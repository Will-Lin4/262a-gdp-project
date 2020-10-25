/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  GDP Request handling
**
**	----- BEGIN LICENSE BLOCK -----
**	GDP: Global Data Plane Support Library
**	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
**
**	Copyright (c) 2015-2019, Regents of the University of California.
**	All rights reserved.
**
**	Permission is hereby granted, without written agreement and without
**	license or royalty fees, to use, copy, modify, and distribute this
**	software and its documentation for any purpose, provided that the above
**	copyright notice and the following two paragraphs appear in all copies
**	of this software.
**
**	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
**	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
**	PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
**	EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
**	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
**	FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION,
**	IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO
**	OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
**	OR MODIFICATIONS.
**	----- END LICENSE BLOCK -----
*/

#include <string.h>
#include <sys/queue.h>

#include "gdp.h"
#include "gdp_chan.h"
#include "gdp_event.h"
#include "gdp_priv.h"

#include <ep/ep_dbg.h>
#include <ep/ep_log.h>
#include <ep/ep_prflags.h>
#include <ep/ep_thr.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.req", "GDP request processing");

static struct req_head	ReqFreeList = LIST_HEAD_INITIALIZER(ReqFreeList);
static EP_THR_MUTEX		ReqFreeListMutex
							EP_THR_MUTEX_INITIALIZER2(GDP_MUTEX_LORDER_LEAF);
static int				NReqsAllocated = 0;

static const char *ReqStates[] =
{
	"FREE",			// 0
	"ACTIVE",		// 1
	"WAITING",		// 2
	"IDLE",			// 3
};


/*
**  Show string version of state (for debugging output)
**
**		Not thread safe, but only for impossible states.
*/

static const char *
statestr(const gdp_req_t *req)
{
	static char sbuf[20];
	int state;

	if (req == NULL)
		return "(NONE)";

	state = req->state;
	if (state >= 0 && (unsigned) state < sizeof ReqStates)
	{
		return ReqStates[state];
	}
	else
	{
		snprintf(sbuf, sizeof sbuf, "IMPOSSIBLE(%d)", state);
		return sbuf;
	}
}

/*
**  _GDP_REQ_NEW --- allocate a new request
**
**	Parameters:
**		cmd --- the command to be issued
**		gob --- the associated GOB handle, if any.  Must be locked
**				on entry.
**		chan --- the channel associated with the request
**		pdu --- the existing PDU; if none, one will be allocated.
**				This is only used by gdp_pdu_proc_cmd.
**		flags --- modifier flags
**		reqp --- a pointer to the output area
**
**	Returns:
**		status
**		The request has been allocated an id (possibly unique to gob),
**			but the request has not been linked onto the GOB's request list.
**			This allows the caller to adjust the request without locking it.
**		The request is always returned locked.
*/

EP_STAT
_gdp_req_new(gdp_cmd_t cmd,
		gdp_gob_t *gob,
		gdp_chan_t *chan,
		gdp_pdu_t *pdu,
		uint32_t flags,
		gdp_req_t **reqp)
{
	EP_STAT estat = EP_STAT_OK;
	gdp_req_t *req;

	// if the caller wants asynchronous results, tweak flags accordingly
	if (EP_UT_BITSET(GDP_REQ_ASYNCIO, flags))
		flags |= GDP_REQ_ALLOC_RID;

	// if assertion fails, may be working with an unallocated GOB
	if (gob != NULL && !GDP_GOB_ASSERT_ISLOCKED(gob))
			return EP_STAT_ASSERT_ABORT;

	// rationalize parameters
	if (chan == NULL)
		chan = _GdpChannel;

	// get memory, off free list if possible (and do stats now)
	ep_thr_mutex_lock(&ReqFreeListMutex);
	NReqsAllocated++;
	req = LIST_FIRST(&ReqFreeList);
	if (req != NULL)
		LIST_REMOVE(req, goblist);
	ep_thr_mutex_unlock(&ReqFreeListMutex);

	// sanity: make sure "free" request isn't on a live list
	if (req != NULL)
	{
		if (!EP_ASSERT(req->state == GDP_REQ_FREE) ||
			!EP_ASSERT(!EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags)) ||
			!EP_ASSERT(!EP_UT_BITSET(GDP_REQ_ON_CHAN_LIST, req->flags)))
		{
			// just abandon the bogus request on free list
			// (we'll create a new one below)
			req = NULL;
		}
	}

	// if no free reqs available, create and initialize a new one
	if (req == NULL)
	{
		// nothing on free list; allocate another
		req = (gdp_req_t *) ep_mem_zalloc(sizeof *req);
		ep_thr_mutex_init(&req->mutex, EP_THR_MUTEX_DEFAULT);
		ep_thr_mutex_setorder(&req->mutex, GDP_MUTEX_LORDER_REQ);
		ep_thr_cond_init(&req->cond);
		TAILQ_INIT(&req->events);
	}

	// initialize request
	req->state = GDP_REQ_ACTIVE;		// must be before _gdp_req_lock
	VALGRIND_HG_CLEAN_MEMORY(req, sizeof *req);
	(void) _gdp_req_lock(req);
	req->stat = EP_STAT_OK;
	req->flags = flags;
	req->chan = chan;
	req->gob = gob;
	if (gob != NULL)
		_gdp_gob_incref(gob);		// request has a new reference
	req->r_results = 0;				// no results received yet
	req->s_results = -1;			// unknown number of results sent

	// if we're not passing in a PDU, create and initialize the new one
	if (pdu == NULL)
	{
		// This is the common case
		// gdp_pdu_proc_cmd is the only case where PDU already exists

		gdp_rid_t rid;
		gdp_seqno_t seqno;
		gdp_name_t dst;

		if (gob != NULL)
		{
			GDP_GOB_ASSERT_ISLOCKED(gob);
			memcpy(dst, gob->name, sizeof dst);
		}
		else
		{
			ep_dbg_cprintf(Dbg, 1, "_gdp_req_new: no destination address\n");
			memset(dst, 0, sizeof dst);
		}

		if ((gob == NULL || !EP_UT_BITSET(GDP_REQ_PERSIST, flags)) &&
				!EP_UT_BITSET(GDP_REQ_ALLOC_RID, flags))
		{
			// just use constant zero; any value would be fine
			rid = GDP_PDU_NO_RID;
		}
		else
		{
			// allocate a new unique request id
			rid = _gdp_rid_new(gob, chan);
		}

		seqno = GDP_PDU_NO_L5SEQNO;		// for now...

		gdp_msg_t *msg = _gdp_msg_new(cmd, rid, seqno);
		pdu = _gdp_pdu_new(msg, _GdpMyRoutingName, dst, GDP_SEQNO_NONE);

		ep_dbg_cprintf(Dbg, 11, "_gdp_req_new: allocated new pdu @ %p\n",
					pdu);
	}
	if (GDP_CMD_IS_COMMAND(cmd))
		req->cpdu = pdu;
	else
		req->rpdu = pdu;

	// keep track of all outstanding requests on a channel
	//DEBUG: shouldn't this be in _gdp_req_send???
	if (chan != NULL)
	{
		gdp_chan_x_t *chanx;

		_gdp_chan_lock(chan);
		chanx = _gdp_chan_get_cdata(chan);
		if (!EP_ASSERT(chanx != NULL))
		{
			estat = EP_STAT_ASSERT_ABORT;
			goto fail0;
		}
		IF_LIST_CHECK_OK(&chanx->reqs, req, chanlist, gdp_req_t)
		{
			LIST_INSERT_HEAD(&chanx->reqs, req, chanlist);
			req->flags |= GDP_REQ_ON_CHAN_LIST;
		}
		else
		{
			estat = EP_STAT_ASSERT_ABORT;
		}
fail0:
		_gdp_chan_unlock(chan);
	}

	// success
	*reqp = req;
	ep_dbg_cprintf(Dbg, 48, "_gdp_req_new(gob=%p, cmd=%s) => %p (rid=%d)\n",
			req->gob, _gdp_proto_cmd_name(cmd), req, pdu->msg->rid);
	return estat;
}


/*
**  _GDP_REQ_FREE --- return a request to the free list
**
**		Note that we grab the GOB linked list as the free list, since
**		it's impossible for a free request to be attached to a GOB.
**
**		The request must be locked on entry.
**		A GOB in the request must be unlocked on entry.
**		It will still be locked on return.   XXX why?
*/

void
_gdp_req_free(gdp_req_t **reqp)
{
	gdp_req_t *req = *reqp;

	if (req == NULL)
		return;

	// make sure the original pointer is invalid
	*reqp = NULL;

	ep_dbg_cprintf(Dbg, 48, "_gdp_req_free(%p)  state=%d, gob=%p\n",
			req, req->state, req->gob);

	EP_THR_MUTEX_ASSERT_ISLOCKED(&req->mutex);
	if (req->state == GDP_REQ_FREE)
	{
		// req was freed after a reference was taken
		ep_dbg_cprintf(Dbg, 1, "_gdp_req_free(%p): already free\n", req);
		return;
	}
	if (req->gob != NULL)
		EP_ASSERT(req->gob->refcnt > 0);

	// remove any timeout associated with this req
	_gdp_evloop_timer_clr(&req->ev_to);

	// flush any saved events for this req
	_gdp_event_trigger_pending(req, true);

	// remove the request from the channel subscription list
	if (EP_UT_BITSET(GDP_REQ_ON_CHAN_LIST, req->flags))
	{
		_gdp_chan_lock(req->chan);
		LIST_REMOVE(req, chanlist);
		req->flags &= ~GDP_REQ_ON_CHAN_LIST;
		_gdp_chan_unlock(req->chan);
	}

	// remove the request from the GOB list
	if (EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags))
	{
		EP_ASSERT_ELSE(req->gob != NULL, return);
		GDP_GOB_ASSERT_ISLOCKED(req->gob);
		LIST_REMOVE(req, goblist);
		req->flags &= ~GDP_REQ_ON_GOB_LIST;
	}

	// remove any pending events from the request
	{
		gdp_event_t *gev;
		while ((gev = TAILQ_FIRST(&req->events)) != NULL)
			TAILQ_REMOVE(&req->events, gev, queue);
	}

	// free the associated PDU(s)
	if (req->rpdu != NULL && req->rpdu != req->cpdu)
		_gdp_pdu_free(&req->rpdu);
	if (req->cpdu != NULL)
		_gdp_pdu_free(&req->cpdu);

	if (req->gob != NULL)
		_gdp_gob_decref(&req->gob, true);
	req->state = GDP_REQ_FREE;
	req->flags = 0;
	req->sub_cbarg = NULL;

	// add the empty request to the free list
	ep_thr_mutex_lock(&ReqFreeListMutex);
	_gdp_req_unlock(req);
#if GDP_DEBUG_NO_FREE_LISTS		// avoid helgrind complaints
	ep_thr_mutex_destroy(&req->mutex);
	ep_thr_cond_destroy(&req->cond);
	ep_mem_free(req);
#else
	LIST_INSERT_HEAD(&ReqFreeList, req, goblist);
#endif
	NReqsAllocated--;
	ep_thr_mutex_unlock(&ReqFreeListMutex);
}


/*
**  _GDP_REQ_FREEALL --- free all requests for a given GOB
**
**		The data structure that reqlist is in should be locked.
**		This will normally be a GOB (and in fact, the loop does not
**		handle failures properly if _gdp_req_free doesn't remove
**		req from the list).
**
**		If gin is set it only frees requests associated with this gin.
*/

void
_gdp_req_freeall(gdp_gob_t *gob,
				gdp_gin_t *gin,
				void (*shutdownfunc)(gdp_req_t *))
{
	EP_STAT rstat = EP_STAT_OK;
	gdp_req_t *req;
	gdp_req_t *nextreq;
	bool initlist = true;

	ep_dbg_cprintf(Dbg, 49, ">>> _gdp_req_freeall(%p)\n", gob);

	for (req = LIST_FIRST(&gob->reqs); req != NULL; req = nextreq)
	{
		EP_STAT estat = _gdp_req_lock(req);
		nextreq = LIST_NEXT(req, goblist);
		if (!EP_STAT_ISOK(estat))
		{
			// couldn't lock the request, so skip it
			ep_log(estat, "_gdp_req_freeall: couldn't acquire req lock");
			LIST_REMOVE(req, goblist);
			req->flags &= ~GDP_REQ_ON_GOB_LIST;
			rstat = estat;
		}
		else if (gin == NULL || req->gin == gin)
		{
			if (shutdownfunc != NULL)
				(*shutdownfunc)(req);

			// this will remove req from the GOB reqlist
			_gdp_req_free(&req);
		}
		else
		{
			// leave it (and don't initialize the list)
			initlist = false;
		}
	}

	if (initlist)
	{
		// if there were errors, it's possible that there are still some
		// items on reqlist.  Abandon those to avoid cascading errors.
		LIST_INIT(&gob->reqs);
	}

	if (ep_dbg_test(Dbg, EP_STAT_ISOK(rstat) ? 49 : 1))
	{
		char ebuf[100];
		ep_dbg_printf("<<< _gdp_req_freeall(%p): %s (%d)\n",
					gob, ep_stat_tostr(rstat, ebuf, sizeof ebuf), initlist);
	}
}


/*
**  Lock/unlock a request
*/

EP_STAT
_gdp_req_lock(gdp_req_t *req)
{
	ep_dbg_cprintf(Dbg, 60, "_gdp_req_lock: req @ %p\n", req);
	ep_thr_mutex_lock(&req->mutex);

	// if this request was being freed, the reference might be dead now
	if (!EP_ASSERT(req->state != GDP_REQ_FREE))
	{
		// oops, unlock it and return failure
		if (ep_dbg_test(Dbg, 1))
		{
			ep_dbg_printf("_gdp_req_lock: req @ %p is free\n", req);
			_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_DETAILED, 0);
		}
		ep_thr_mutex_unlock(&req->mutex);
		return GDP_STAT_USING_FREE_REQ;
	}
	return EP_STAT_OK;
}

void
_gdp_req_unlock(gdp_req_t *req)
{
	ep_dbg_cprintf(Dbg, 60, "_gdp_req_unlock: req @ %p\n", req);
	ep_thr_mutex_unlock(&req->mutex);
}


/*
**   _GDP_REQ_SEND --- send a request to the GDP daemon
**
**		This makes no attempt to read results.
**
**		This routine also links the request onto the GOB list (if any)
**		so that the matching response PDU can find the request (the
**		PDU contains the GOB and the RID, which are enough to find
**		the corresponding request).  If it's already on a GOB list we
**		work on the assumption that it is this one.  We might want to
**		verify that for debugging purposes.
**
**		The request must be locked.
*/

EP_STAT
_gdp_req_send(gdp_req_t *req)
{
	EP_STAT estat;
	gdp_gob_t *gob = req->gob;

	if (ep_dbg_test(Dbg, 45))
	{
		flockfile(ep_dbg_getfile());
		ep_dbg_printf("_gdp_req_send: ");
		_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_DETAILED, 0);
		funlockfile(ep_dbg_getfile());
	}
	EP_ASSERT_ELSE(req->state != GDP_REQ_FREE, return EP_STAT_ASSERT_ABORT);

	req->flags &= ~GDP_REQ_DONE;
	if (gob != NULL && !EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags))
	{
		// link the request to the GOB
		ep_dbg_cprintf(Dbg, 49, "_gdp_req_send(%p) gob=%p\n", req, gob);
		GDP_GOB_ASSERT_ISLOCKED(gob);
		LIST_INSERT_HEAD(&gob->reqs, req, goblist);
		req->flags |= GDP_REQ_ON_GOB_LIST;
	}

	// write the message out
	estat = _gdp_pdu_out(req->cpdu, req->chan);

	// done
	return estat;
}


/*
**  _GDP_REQ_UNSEND --- pull a request off a GOB list
**
**		Used when the attempt to do an invocation fails.
*/

EP_STAT
_gdp_req_unsend(gdp_req_t *req)
{
	gdp_gob_t *gob = req->gob;

	if (ep_dbg_test(Dbg, 17))
	{
		ep_dbg_printf("_gdp_req_unsend: ");
		_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
	}
	EP_ASSERT_ELSE(req->state != GDP_REQ_FREE, return EP_STAT_ASSERT_ABORT);

	if (gob == NULL)
	{
		ep_dbg_cprintf(Dbg, 4, "_gdp_req_unsend: req %p has NULL GOB\n",
				req);
		return GDP_STAT_NULL_GOB;
	}
	if (!EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags))
	{
		ep_dbg_cprintf(Dbg, 4, "_gdp_req_unsend: req %p not on GOB list\n",
				req);
	}
	GDP_GOB_ASSERT_ISLOCKED(gob);
	LIST_REMOVE(req, goblist);
	req->flags &= ~GDP_REQ_ON_GOB_LIST;

	return EP_STAT_OK;
}


/*
**  _GDP_REQ_FIND --- find a request in a GOB
**
**		The state must show that the req is not currently active; if it
**		is we would clobber one another.  Note that we can't just keep
**		the req locked because that would require passing a lock between
**		threads, which is a non-starter.  To get around that the req
**		has a state; if it is currently in active use by another thread
**		we have to wait.  However, this does return the req pre-locked.
**
**		This may be the wrong place to do this, since this blocks the
**		I/O thread.  Arguably the I/O thread should read PDUs, put them
**		on a service list, and let another thread handle it.  This is
**		more-or-less what gdplogd does now, so this problem only shows
**		up in clients that may be working with many GOBs at the same
**		time.  Tomorrow is another day.
*/

gdp_req_t *
_gdp_req_find(gdp_gob_t *gob, gdp_rid_t rid)
{
	gdp_req_t *req;

	ep_dbg_cprintf(Dbg, 50, "_gdp_req_find(gob=%p, rid=%" PRIgdp_rid")\n",
			gob, rid);
	if (!EP_ASSERT(GDP_GOB_ISGOOD(gob)))
		return NULL;
	GDP_GOB_ASSERT_ISLOCKED(gob);

	for (;;)
	{
		EP_STAT estat;
		gdp_req_t *nextreq;

		do
		{
			estat = EP_STAT_OK;
			req = LIST_FIRST(&gob->reqs);
			for (; req != NULL; req = nextreq)
			{
				estat = _gdp_req_lock(req);
				EP_STAT_CHECK(estat, break);
				nextreq = LIST_NEXT(req, goblist);
				if (req->cpdu != NULL && req->cpdu->msg != NULL &&
						(rid == GDP_PDU_ANY_RID || req->cpdu->msg->rid == rid))
					break;
				_gdp_req_unlock(req);
			}
		} while (!EP_STAT_ISOK(estat));
		if (req == NULL)
			break;				// nothing to find

		// if we find a free request (we shouldn't), just ignore it
		EP_ASSERT_ELSE(req->state != GDP_REQ_FREE,
						continue);
		if (req->state != GDP_REQ_ACTIVE)
			break;				// this is what we are looking for!

		// it's in the wrong state; wait for a change and then try again
		ep_dbg_cprintf(Dbg, 20, "_gdp_req_find: wrong state: %s\n",
				statestr(req));
		//XXX should have a timeout here
		ep_thr_cond_wait(&req->cond, &req->mutex, NULL);
	}
	if (req != NULL)
	{
		if (!EP_UT_BITSET(GDP_REQ_PERSIST|GDP_REQ_ASYNCIO, req->flags))
		{
			EP_ASSERT(EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags));
			LIST_REMOVE(req, goblist);
			req->flags &= ~GDP_REQ_ON_GOB_LIST;
		}
	}

	ep_dbg_cprintf(Dbg, 48,
			"_gdp_req_find(gob=%p, rid=%" PRIgdp_rid ") => %p, state %s\n",
			gob, rid, req, statestr(req));
	return req;
}


/*
**  Print a request (for debugging)
**
**		This potentially references the req while it is unlocked,
**		which isn't good, but since this is only for debugging and
**		is read-only we'll take the risk.
*/

static EP_PRFLAGS_DESC	ReqFlags[] =
{
	{ GDP_REQ_ASYNCIO,		GDP_REQ_ASYNCIO,		"ASYNCIO"		},
	{ GDP_REQ_DONE,			GDP_REQ_DONE,			"DONE"			},
	{ GDP_REQ_CLT_SUBSCR,	GDP_REQ_CLT_SUBSCR,		"CLT_SUBSCR"	},
	{ GDP_REQ_SRV_SUBSCR,	GDP_REQ_SRV_SUBSCR,		"SRV_SUBSCR"	},
	{ GDP_REQ_PERSIST,		GDP_REQ_PERSIST,		"PERSIST"		},
	{ GDP_REQ_SUBUPGRADE,	GDP_REQ_SUBUPGRADE,		"SUBUPGRADE"	},
	{ GDP_REQ_ALLOC_RID,	GDP_REQ_ALLOC_RID,		"ALLOC_RID"		},
	{ GDP_REQ_ON_GOB_LIST,	GDP_REQ_ON_GOB_LIST,	"ON_GOB_LIST"	},
	{ GDP_REQ_ON_CHAN_LIST,	GDP_REQ_ON_CHAN_LIST,	"ON_CHAN_LIST"	},
	{ GDP_REQ_VRFY_CONTENT,	GDP_REQ_VRFY_CONTENT,	"VRFY_CONTENT"	},
	{ GDP_REQ_ROUTEFAIL,	GDP_REQ_ROUTEFAIL,		"ROUTEFAIL"		},
	{ 0,					0,						NULL			}
};

void
_gdp_req_dump(const gdp_req_t *req, FILE *fp, int detail, int indent)
{
	char ebuf[200];

	if (fp == NULL)
		fp = ep_dbg_getfile();
	if (req == NULL)
	{
		fprintf(fp, "req@%p: null\n", req);
		return;
	}
	indent++;
	VALGRIND_HG_DISABLE_CHECKING(req, sizeof *req);
	flockfile(fp);
	fprintf(fp, "req@%p:\n", req);
	fprintf(fp, "%snextrec=%" PRIgdp_recno ", numrecs=%" PRIu32
				", r_results=%" PRId64 ", s_results=%" PRId64 ", chan=%p\n"
			"%spostproc=%p, sub_cbfunc=%p, sub_cbarg=%p\n"
			"%sgin=%p, state=%s, stat=%s\n",
			_gdp_pr_indent(indent), req->nextrec, req->numrecs,
				req->r_results, req->s_results, req->chan,
			_gdp_pr_indent(indent), req->postproc, req->sub_cbfunc, req->sub_cbarg,
			_gdp_pr_indent(indent), req->gin, statestr(req),
			ep_stat_tostr(req->stat, ebuf, sizeof ebuf));
	fprintf(fp, "%sact_ts=", _gdp_pr_indent(indent));
	ep_time_print(&req->act_ts, fp, EP_TIME_FMT_HUMAN);
	fprintf(fp, "\n%ssub_ts=", _gdp_pr_indent(indent));
	ep_time_print(&req->sub_ts, fp, EP_TIME_FMT_HUMAN);
	fprintf(fp, "\n%sflags=", _gdp_pr_indent(indent));
	ep_prflags(req->flags, ReqFlags, fp);
	fprintf(fp, "\n%s", _gdp_pr_indent(indent));
	_gdp_gob_dump(req->gob, fp, detail, indent);
	if (req->cpdu != NULL)
	{
		fprintf(fp, "%sc", _gdp_pr_indent(indent));
		_gdp_pdu_dump(req->cpdu, fp, indent);
	}
	if (req->rpdu != NULL)
	{
		fprintf(fp, "%sr", _gdp_pr_indent(indent));
		_gdp_pdu_dump(req->rpdu, fp, indent);
	}
	funlockfile(fp);
	VALGRIND_HG_ENABLE_CHECKING(req, sizeof *req);
}


/*
**  Print statistics (for debugging)
*/

void
_gdp_req_pr_stats(FILE *fp)
{
	VALGRIND_HG_DISABLE_CHECKING(&NReqsAllocated, sizeof NReqsAllocated);
	fprintf(fp, "Reqs Allocated: %d\n", NReqsAllocated);
	VALGRIND_HG_ENABLE_CHECKING(&NReqsAllocated, sizeof NReqsAllocated);
}


/*
**  _GDP_REQ_ACK_RESP --- helper routine for returning ACK responses
*/

EP_STAT
_gdp_req_ack_resp(
			gdp_req_t *req,
			gdp_cmd_t ack_type)
{
	gdp_msg_t *msg;

	if (req->rpdu != NULL)
	{
		ep_dbg_cprintf(Dbg, 61,
				"_gdp_req_ack_resp: flushing old rpdu %p for %s\n",
				req->rpdu, _gdp_proto_cmd_name(ack_type));
		_gdp_pdu_free(&req->rpdu);
	}
	msg = _gdp_msg_new(ack_type, req->cpdu->msg->rid, req->cpdu->msg->l5seqno);
	req->rpdu = _gdp_pdu_new(msg, req->cpdu->dst, req->cpdu->src, GDP_SEQNO_NONE);
	return EP_STAT_OK;
}


/*
**	_GDP_REQ_NAK_RESP --- helper routine for returning NAK responses
*/

EP_STAT
_gdp_req_nak_resp(gdp_req_t *req,
			gdp_cmd_t nak_type,
			const char *detail,
			EP_STAT estat)
{
	gdp_pname_t pname;
	char text_message[250];
	gdp_msg_t *msg;
	char ebuf[80];

	// a bit of a hack
	if (nak_type == 0)
		nak_type = _gdp_acknak_from_estat(estat, GDP_NAK_S_INTERNAL);

	gdp_printable_name(req->cpdu->dst, pname);
	snprintf(text_message, sizeof text_message, "%s: %s: %s",
			detail, pname, ep_stat_tostr(estat, ebuf, sizeof ebuf));
	if (nak_type == GDP_NAK_S_INTERNAL)
	{
		// server error (rather than client error)
		ep_log(estat, "%s: %s", detail, pname);
	}
	else
	{
		ep_dbg_cprintf(Dbg, 1, "%s\n", text_message);
	}
	if (req->rpdu != NULL)
	{
		ep_dbg_cprintf(Dbg, 61, "gdpd_nak_resp: flushing old rpdu %p\n",
				req->rpdu);
		_gdp_pdu_free(&req->rpdu);
	}

	ep_dbg_cprintf(Dbg, 10,
				"gdpd_nak_resp: sending %s (%s)\n",
				_gdp_proto_cmd_name(nak_type),
				text_message);
	msg = _gdp_msg_new(nak_type, req->cpdu->msg->rid, req->cpdu->msg->l5seqno);
	req->rpdu = _gdp_pdu_new(msg, req->cpdu->dst, req->cpdu->src,
							GDP_SEQNO_NONE);
	GdpMessage__NakGeneric *nak = msg->nak;
	nak->ep_stat = EP_STAT_TO_INT(estat);
	nak->description = ep_mem_strdup(text_message);

	return estat;
}


/***********************************************************************
**
**	Request ID handling
**
**		Very simplistic for now.  RIDs really only need to be unique
**		within a given GOB/channel tuple.
**
**		Assumes that GDP_PDU_ANY_RID == UINT32_MAX; otherwise the
**		domain of MaxRid is reduced.
*/

static gdp_rid_t	MaxRid = 0;

gdp_rid_t
_gdp_rid_new(gdp_gob_t *gob, gdp_chan_t *chan)
{
	if (++MaxRid == GDP_PDU_ANY_RID)
	{
		// overflow!!!  at least log something
		ep_log(EP_STAT_SEVERE, "Request ID overflow");
		MaxRid = 100;			// XXX wild guess; should check for rids in use
	}
	return MaxRid;
}

char *
gdp_rid_tostr(gdp_rid_t rid, char *buf, size_t len)
{
	snprintf(buf, len, "%d", rid);
	return buf;
}
