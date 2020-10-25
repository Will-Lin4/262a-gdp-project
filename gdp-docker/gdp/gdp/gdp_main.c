/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  GDP Initialization and main event loop.
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

#include "gdp.h"
#include "gdp_chan.h"
#include "gdp_event.h"
#include "gdp_hongd.h"
#include "gdp_priv.h"
#include "gdp_version.h"

#include <ep/ep.h>
#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <ep/ep_funclist.h>
#include <ep/ep_log.h>
#include <ep/ep_syslog.h>

#include <event2/buffer.h>
#include <event2/thread.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.main", "GDP initialization and main loop");
static EP_DBG	DbgEvLock = EP_DBG_INIT("gdp.libevent.locks", "GDP libevent lock debugging");
static EP_DBG	DbgProcResp = EP_DBG_INIT("gdp.response", "GDP response processing");
static EP_DBG	DbgTimers = EP_DBG_INIT("gdp.libevent.timers", "GDP timer events");

struct event_base	*_GdpIoEventBase;	// the base for GDP I/O events
gdp_name_t			_GdpMyRoutingName;	// source name for PDUs
gdp_chan_t			*_GdpChannel;		// our primary app-level protocol port
static bool			_GdpRunCmdInThread = true;		// run commands in threads
static bool			_GdpRunRespInThread = false;	// run responses in threads
bool				_GdpLibInitialized;	// are we initialized?


/*
**  INIT_ERROR --- issue error on initialization problem
*/

static EP_STAT
init_error(const char *datum, const char *where)
{
	EP_STAT estat = ep_stat_from_errno(errno);
	char nbuf[40];

	(void) (0 == strerror_r(errno, nbuf, sizeof nbuf));
	ep_log(estat, "gdp_init: %s: %s", where, datum);
	ep_app_error("gdp_init: %s: %s: %s", where, datum, nbuf);
	return estat;
}


gdp_cmd_t
_gdp_acknak_from_estat(EP_STAT estat, gdp_cmd_t def)
{
	gdp_cmd_t resp = def;

	if (EP_STAT_ISOK(estat))
	{
		if (def < GDP_ACK_MIN || def > GDP_ACK_MAX)
			resp = GDP_ACK_SUCCESS;
	}
	else if (EP_STAT_REGISTRY(estat) == EP_REGISTRY_EPLIB &&
			EP_STAT_MODULE(estat) == EP_STAT_MOD_CRYPTO)
	{
		resp = GDP_NAK_C_UNAUTH;
	}
	else if (EP_STAT_IS_SAME(estat, GDP_STAT_NOT_IMPLEMENTED))
		resp = GDP_NAK_S_NOTIMPL;
	else if (EP_STAT_REGISTRY(estat) == EP_REGISTRY_UCB &&
		EP_STAT_MODULE(estat) == GDP_MODULE)
	{
		// if the estat contains the detail, prefer that
		int d = EP_STAT_DETAIL(estat);

		if (!EP_STAT_ISFAIL(estat))
		{
			if (d >= 200 && d <= (200 + GDP_ACK_MAX - GDP_ACK_MIN))
				resp = (gdp_cmd_t) (d - 200 + GDP_ACK_MIN);
		}
		else if (d >= 400 && d <= (400 + GDP_NAK_C_MAX - GDP_NAK_C_MIN))
			resp = (gdp_cmd_t) (d - 400 + GDP_NAK_C_MIN);
		else if (d >= 500 && d <= (500 + GDP_NAK_S_MAX - GDP_NAK_S_MIN))
				resp = (gdp_cmd_t) (d - 500 + GDP_NAK_S_MIN);
		else if (d >= 600 && d <= (600 + GDP_NAK_R_MAX - GDP_NAK_R_MIN))
			resp = (gdp_cmd_t) (d - 600 + GDP_NAK_R_MIN);
	}
	else if (resp < GDP_NAK_C_MIN || resp > GDP_NAK_R_MAX)
		resp = GDP_NAK_S_INTERNAL;		// default to panic code

	if (ep_dbg_test(Dbg, 41))
	{
		char ebuf[100];

		ep_dbg_printf("_gdp_acknak_from_estat: %s -> %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf),
				_gdp_proto_cmd_name(resp));
	}
	return resp;
}


/*
**  PROCESS_CMD --- process command PDU
**
**		Usually done in a thread since it may be heavy weight.
**		This usually only applies to gdplogd.
*/

static EP_THR_MUTEX		GdpCreateMutex			EP_THR_MUTEX_INITIALIZER;

static void
process_cmd(void *cpdu_)
{
	gdp_pdu_t *cpdu = (gdp_pdu_t *) cpdu_;
	gdp_cmd_t cmd;
	EP_STAT estat;
	gdp_gob_t *gob = NULL;
	gdp_req_t *req = NULL;
	EP_TIME_SPEC starttime;

	ep_time_now(&starttime);
	GDP_MSG_CHECK(cpdu, return);
	cmd = cpdu->msg->cmd;

	ep_dbg_cprintf(Dbg, 40,
			"process_cmd(%s, thread %" EP_THR_PRItid ")\n",
			_gdp_proto_cmd_name(cmd), ep_thr_gettid());

	// create has too many special cases, so we single thread it
	if (cmd == GDP_CMD_CREATE)
		ep_thr_mutex_lock(&GdpCreateMutex);

	estat = _gdp_gob_cache_get(cpdu->dst, GGCF_NOCREATE, &gob);
	if (gob != NULL)
	{
		GDP_GOB_ASSERT_ISLOCKED(gob);
		EP_ASSERT(gob->refcnt > 0);
	}

	ep_dbg_cprintf(Dbg, 43,
			"process_cmd: allocating new req for GOB %p\n", gob);
	estat = _gdp_req_new(cmd, gob, cpdu->chan, cpdu, 0, &req);
	EP_STAT_CHECK(estat, goto fail0);
	EP_THR_MUTEX_ASSERT_ISLOCKED(&req->mutex);
	EP_ASSERT(gob == req->gob);

	ep_dbg_cprintf(Dbg, 40, "process_cmd >>> req=%p\n", req);

	// do the per-command processing
	estat = _gdp_req_dispatch(req, cmd);
	if (ep_dbg_test(Dbg, 59))
	{
		char ebuf[100];
		ep_dbg_printf("process_cmd: dispatch => %s\n    ",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
		_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 1);
	}
	bool response_already_sent = EP_STAT_IS_SAME(estat, GDP_STAT_RESPONSE_SENT);

	// make sure request or GOB haven't gotten fubared
	EP_THR_MUTEX_ASSERT_ISLOCKED(&req->mutex);
	GDP_MSG_CHECK(req->cpdu, goto fail1);
	GDP_MSG_CHECK(req->rpdu, goto fail1);

	// special case: if we have deleted a GOB, it will have disappeared now
	if (req->gob == NULL && gob != NULL &&
			req->cpdu->msg->cmd == GDP_CMD_DELETE &&
			GDP_CMD_IS_ACK(req->rpdu->msg->cmd))
	{
		_gdp_gob_unlock(gob);
		gob = NULL;
	}

	if (gob != NULL)
	{
		GDP_GOB_ASSERT_ISLOCKED(gob);
		if (!EP_ASSERT(gob == req->gob) && ep_dbg_test(Dbg, 1))
		{
			ep_dbg_printf("process_cmd, after dispatch:\n  gob = ");
			_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_BASIC, 0);
			ep_dbg_printf("  req->gob = ");
			_gdp_gob_dump(req->gob, ep_dbg_getfile(), GDP_PR_BASIC, 0);
		}
	}

	// cmd_open and cmd_create can return a new GOB in the req
	if (gob == NULL && req->gob != NULL)
		gob = _gdp_gob_incref(req->gob);

	if (!response_already_sent)
	{
		req->stat = _gdp_pdu_out(req->rpdu, req->chan);
	}

	EP_ASSERT(gob == req->gob);
	if (gob != NULL)
	{
		GDP_GOB_ASSERT_ISLOCKED(gob);
		EP_ASSERT(gob->refcnt > 0);
	}

	// do command post processing
	if (req->postproc)
	{
		ep_dbg_cprintf(Dbg, 44, "process_cmd: doing post processing\n");
		(req->postproc)(req);
		req->postproc = NULL;

		// postproc shouldn't change GOB lock status
		EP_ASSERT(gob == req->gob);
	}

fail1:
	// free up resources
	if (EP_UT_BITSET(GDP_REQ_PERSIST, req->flags))
		_gdp_req_unlock(req);
	else
		_gdp_req_free(&req);		// also decref's req->gob (leaves locked)
	if (gob != NULL)
	{
		if (!GDP_GOB_ASSERT_ISLOCKED(gob) || !EP_ASSERT(gob->refcnt > 0))
			_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_BASIC, 0);
		_gdp_gob_decref(&gob, false);	// ref from _gdp_gob_cache_get
	}

	if (false)
	{
fail0:
		ep_log(estat, "process_cmd: cannot allocate request; dropping PDU");
		if (cpdu != NULL)
			_gdp_pdu_free(&cpdu);
	}

	if (cmd == GDP_CMD_CREATE)
		ep_thr_mutex_unlock(&GdpCreateMutex);

	ep_dbg_cprintf(Dbg, 40, "process_cmd <<< done\n");
	_gdp_show_elapsed("process_cmd", cmd, &starttime);
}


/*
**  Search for a request in a channel list.
**
**		This is a fall-back that should be used only for finding requests
**		sent by FWD_APPEND to a server that isn't accessible.  Since that
**		command links the request to a different GOB than the destination
**		address in the PDU, a NOROUTE response won't find it.
*/

static EP_STAT
find_req_in_channel_list(
				const gdp_pdu_t *rpdu,
				gdp_chan_t *chan,
				gdp_req_t **reqp)
{
	EP_STAT estat = EP_STAT_OK;
	gdp_req_t *req;
	gdp_chan_x_t *chanx;

	if (ep_dbg_test(DbgProcResp, 14))
	{
		gdp_pname_t src_p, dst_p;

		ep_dbg_printf("find_req_in_channel_list: searching for rpdu rid "
						"%" PRIgdp_rid
					"\n    src %s\n    dst %s\n",
				rpdu->msg->rid,
				gdp_printable_name(rpdu->src, src_p),
				gdp_printable_name(rpdu->dst, dst_p));
	}
	_gdp_chan_lock(chan);
	chanx = _gdp_chan_get_cdata(chan);
	if (!EP_ASSERT(chanx != NULL))
	{
		req = NULL;
		estat = EP_STAT_ASSERT_ABORT;
		goto fail0;
	}

	LIST_FOREACH(req, &chanx->reqs, chanlist)
	{
		if (req->cpdu == NULL)
			continue;
		if (ep_dbg_test(Dbg, 48))
		{
			gdp_pname_t src_p, dst_p;
			ep_dbg_printf("    ?cpdu rid %" PRIgdp_rid "\t%s =>\n\t    %s\n",
					req->cpdu->msg->rid,
					gdp_printable_name(req->cpdu->src, src_p),
					gdp_printable_name(req->cpdu->dst, dst_p));
		}
		if ((rpdu->msg->rid == GDP_PDU_ANY_RID ||
					req->cpdu->msg->rid == rpdu->msg->rid) &&
				GDP_NAME_SAME(req->cpdu->src, rpdu->dst) &&
				GDP_NAME_SAME(req->cpdu->dst, rpdu->src))
			break;
	}
	if (ep_dbg_test(DbgProcResp, 40))
	{
		if (req == NULL)
			ep_dbg_printf("    ... not found\n");
		else
			ep_dbg_printf("    ... found req @ %p\n", req);
	}
fail0:
	_gdp_chan_unlock(chan);

	// request should be locked for symmetry with _gdp_req_find
	if (req != NULL)
	{
		char ebuf[100];

		// since GOB has to be locked before req, do it now
		if (req->gob != NULL)
			_gdp_gob_lock(req->gob);
		estat = _gdp_req_lock(req);
		ep_dbg_cprintf(DbgProcResp, 44,
				"find_req_in_channel_list: _gdp_req_lock => %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		if (!EP_STAT_ISOK(estat))
		{
			if (req->gob != NULL)
				_gdp_gob_unlock(req->gob);
			req = NULL;
		}
	}
	if (EP_STAT_ISOK(estat))
		*reqp = req;
	return estat;
}


/*
**  PROCESS_RESP --- process response (ack/nak) PDU
**
**		When this is called, the rpdu passed in will be the actual
**		PDU off the wire and req->cpdu should be the original command
**		PDU that prompted this response.  We save the passed rpdu
**		into req->rpdu for processing in _gdp_req_dispatch.
**
**		XXX This is not tested for running in a thread.
*/

static void
process_resp(void *rpdu_)
{
	gdp_pdu_t *rpdu = (gdp_pdu_t *) rpdu_;
	gdp_chan_t *chan = _GdpChannel;
	int cmd = rpdu->msg->cmd;
	EP_STAT estat;
	gdp_gob_t *gob = NULL;
	gdp_req_t *req = NULL;
	int resp;
	int ocmd;					// original command prompting this response

	estat = _gdp_gob_cache_get(rpdu->src,
						GGCF_NOCREATE | GGCF_GET_PENDING, &gob);
	if (ep_dbg_test(DbgProcResp, 20))
	{
		char ebuf[120];
		gdp_pname_t rpdu_pname;

		ep_dbg_printf("process_resp: cmd %s rpdu %p ->src %s) gob %p stat %s\n",
			_gdp_proto_cmd_name(cmd),
			rpdu, gdp_printable_name(rpdu->src, rpdu_pname), gob,
			ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	// check estat here, or is just checking gob enough?
	if (gob == NULL)
	{
		// gob was not in cache
		char ebuf[200];

		estat = find_req_in_channel_list(rpdu, chan, &req);
		ep_dbg_cprintf(DbgProcResp, 20,
				"    rpdu = %p req = %p stat = %s\n", rpdu, req,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));

		if (!EP_STAT_ISOK(estat) || req == NULL)
		{
			if (ep_dbg_test(DbgProcResp,
						rpdu->msg->cmd == GDP_NAK_R_NOROUTE ? 19 : 1))
			{
				gdp_pname_t pname;
				ep_dbg_printf("process_resp: discarding %d (%s) PDU"
							" for unknown GOB\n",
							rpdu->msg->cmd, _gdp_proto_cmd_name(rpdu->msg->cmd));
				if (ep_dbg_test(DbgProcResp, 24))
					_gdp_pdu_dump(rpdu, ep_dbg_getfile(), 0);
				else
					ep_dbg_printf("    %s\n", gdp_printable_name(rpdu->src, pname));
			}
			_gdp_pdu_free(&rpdu);
			return;
		}

		EP_ASSERT_ELSE(req->state != GDP_REQ_FREE, return);
		if (req->gob != NULL)
		{
			GDP_GOB_ASSERT_ISLOCKED(req->gob);
		}

		// remove the request from the GOB list it is already on
		// req is already locked by find_req_in_channel_list
		if (EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags))
		{
			LIST_REMOVE(req, goblist);
			req->flags &= ~GDP_REQ_ON_GOB_LIST;
			//DEBUG: without this incref, a gdp_gob_create call on a log
			//	that already exists throws the error:
			//	Assertion failed at gob-create:gdp_gob_mgmt.c:435: GDP_GOB_ISGOOD(gob)
			//	because the refcnt has gone to zero prematurely.  But with it,
			//	a successful gdp_gob_create leaves the refcnt one too high
			//	leading to a resource leak.
			_gdp_gob_incref(req->gob);		//DEBUG:

			// code below expects request to remain locked
		}
	}
	else
	{
		// gob was found in cache
		GDP_GOB_ASSERT_ISLOCKED(gob);

		// find the corresponding request
		ep_dbg_cprintf(DbgProcResp, 23,
				"process_resp: searching gob %p for rid %" PRIgdp_rid "\n",
				gob, rpdu->msg->rid);

		// find request to which this PDU applies
		req = _gdp_req_find(gob, rpdu->msg->rid);
		if (ep_dbg_test(DbgProcResp, 51))
		{
			ep_dbg_printf("... found ");
			_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
		}

		// req is already locked by _gdp_req_find
		if (req == NULL)
		{
			// no req for incoming response --- "can't happen"
			if (ep_dbg_test(DbgProcResp, 1))
			{
				ep_dbg_printf("process_resp: no req for incoming response\n");
				_gdp_pdu_dump(rpdu, ep_dbg_getfile(), 0);
				_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_DETAILED, 0);
			}
			_gdp_gob_decref(&gob, false);
			_gdp_pdu_free(&rpdu);
			return;
		}
		else if (!EP_ASSERT(req->state != GDP_REQ_FREE))
		{
			if (ep_dbg_test(DbgProcResp, 1))
			{
				ep_dbg_printf("process_resp: trying to use free ");
				_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_DETAILED, 0);
			}
			_gdp_pdu_free(&rpdu);
			return;
		}
		else if (rpdu == req->rpdu)
		{
			// this could be an assertion
			if (ep_dbg_test(DbgProcResp, 1))
			{
				ep_dbg_printf("process_resp(%d): rpdu == req->rpdu\n",
							rpdu->msg->cmd);
				_gdp_pdu_dump(rpdu, ep_dbg_getfile(), 0);
			}
		}
		EP_ASSERT(gob == req->gob);
	}

	GDP_GOB_ASSERT_ISLOCKED(req->gob);

	if (req->cpdu == NULL)
	{
		ep_dbg_cprintf(DbgProcResp, 1,
				"process_resp(%d): no corresponding command PDU\n",
				rpdu->msg->cmd);
		ocmd = rpdu->msg->cmd;
		//XXX return here?  with req->pdu == NULL, _gdp_req_dispatch
		//XXX will probably die
	}
	else
	{
		ocmd = req->cpdu->msg->cmd;
	}

	// save the response PDU for further processing
	if (req->rpdu != NULL)
	{
		// this can happen in multiread/subscription and async I/O
		if (ep_dbg_test(DbgProcResp, 41))
		{
			ep_dbg_printf("process_resp: req->rpdu already set\n    ");
			_gdp_pdu_dump(req->rpdu, ep_dbg_getfile(), 1);
		}
		_gdp_pdu_free(&req->rpdu);
	}
	req->rpdu = rpdu;

	if (ep_dbg_test(DbgProcResp, 43))
	{
		ep_dbg_printf("process_resp: ");
		_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
	}

	// request is locked, GOB should be too
	if (req->gob != NULL)
		GDP_GOB_ASSERT_ISLOCKED(req->gob);

	// mark this request as active (for subscriptions)
	ep_time_now(&req->act_ts);

	// do ack/nak specific processing
	estat = _gdp_req_dispatch(req, cmd);

	// dispatch should leave it locked
	if (req->gob != NULL)
		GDP_GOB_ASSERT_ISLOCKED(req->gob);

	// figure out potential response code
	// we compute even if unused so we can log server errors
	resp = _gdp_acknak_from_estat(estat, req->rpdu->msg->cmd);

	if (ep_dbg_test(DbgProcResp,
				(resp >= GDP_NAK_S_MIN && resp <= GDP_NAK_S_MAX) ? 1 : 44))
	{
		char ebuf[100];

		ep_dbg_printf("process_resp(%s for %s): %s\n",
				_gdp_proto_cmd_name(cmd),
				_gdp_proto_cmd_name(ocmd),
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		if (ep_dbg_test(DbgProcResp, 55))
			_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
	}

	// ASSERT(all data from chan has been consumed);

	if (EP_UT_BITSET(GDP_REQ_ASYNCIO, req->flags))
	{
		// send the status as an event
		estat = _gdp_event_add_from_req(req);

		// since this is asynchronous we can release the PDU
		_gdp_pdu_free(&req->rpdu);
	}
	else if (req->state == GDP_REQ_WAITING)
	{
		// return our status via the request
		req->stat = estat;
		req->flags |= GDP_REQ_DONE;

		// any further data or status is delivered via event
		req->flags |= GDP_REQ_ASYNCIO | GDP_REQ_PERSIST;	//XXX PERSIST?

		if (ep_dbg_test(DbgProcResp, 40))
		{
			ep_dbg_printf("process_resp: signaling ");
			_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
		}

		// wake up invoker, which will return the status
		ep_thr_cond_signal(&req->cond);

		// give _gdp_invoke a chance to run; not necessary, but
		// avoids having to wait on condition variables
		ep_thr_yield();
	}
	else if (req->rpdu->msg->cmd == GDP_NAK_R_NOROUTE)
	{
		// since this is common and expected, don't sully output
		ep_dbg_cprintf(DbgProcResp, 19,
				"process_resp: discarding GDP_NAK_R_NOROUTE\n");
	}
	else if (ep_dbg_test(DbgProcResp, 1))
	{
		ep_dbg_printf("process_resp: discarding response ");
		_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
	}

	// free up resources
	gob = req->gob;
	if (EP_UT_BITSET(GDP_REQ_PERSIST, req->flags))
		_gdp_req_unlock(req);
	else
		_gdp_req_free(&req);		// also decref's req->gob (leaves locked)
	if (gob != NULL)
	{
		if (!GDP_GOB_ASSERT_ISLOCKED(gob) || !EP_ASSERT(gob->refcnt > 0))
			_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_BASIC, 0);
		_gdp_gob_decref(&gob, false);	// ref from _gdp_gob_cache_get
	}

	ep_dbg_cprintf(DbgProcResp, 40, "process_resp <<< done\n");
}


/*
**  _GDP_PDU_PROCESS --- process a PDU
**
**		This is responsible for the lightweight stuff that can happen
**		in the I/O thread, such as matching an ack/nak PDU with the
**		corresponding req.  It should never block.  The heavy lifting
**		is done in the routine above.
*/

void
_gdp_pdu_process(gdp_pdu_t *pdu, gdp_chan_t *chan)
{
	// use "cheat" field in pdu to pass chan up
	pdu->chan = chan;

	if (GDP_CMD_IS_COMMAND(pdu->msg->cmd))
	{
		if (_GdpRunCmdInThread)
			ep_thr_pool_run(&process_cmd, pdu);
		else
			process_cmd(pdu);
	}
	else
	{
		if (_GdpRunRespInThread)
			ep_thr_pool_run(&process_resp, pdu);
		else
			process_resp(pdu);
	}
}


/*
**  _GDP_RECLAIM_RESOURCES --- find unused GDP resources and reclaim them
**
**		This should really also have a maximum number of GOBs to leave
**		open so we don't run out of file descriptors under high load.
**
**		This implementation locks the GclsByUse list during the
**		entire operation.  That's probably not the best idea.
*/

void
_gdp_reclaim_resources(void *null)
{
	char pbuf[200];
	time_t reclaim_age;		// how long to leave GOBs open before reclaiming

	ep_dbg_cprintf(Dbg, 69, "_gdp_reclaim_resources\n");
	snprintf(pbuf, sizeof pbuf, "swarm.%s.reclaim.age", ep_app_getprogname());
	reclaim_age = ep_adm_getlongparam(pbuf, -1);
	if (reclaim_age == -1)
		reclaim_age = ep_adm_getlongparam("swarm.gdp.reclaim.age",
									GDP_RECLAIM_AGE_DEF);
	_gdp_gob_cache_reclaim(reclaim_age);
}

// stub for libevent

static void
gdp_reclaim_resources_callback(int fd, short what, void *ctx)
{
	ep_dbg_cprintf(Dbg, 69, "gdp_reclaim_resources_callback\n");
	if (ep_adm_getboolparam("swarm.gdp.reclaim.inthread", false))
		ep_thr_pool_run(_gdp_reclaim_resources, NULL);
	else
		_gdp_reclaim_resources(NULL);
}


void
_gdp_reclaim_resources_init(void (*f)(int, short, void *))
{
	static bool running = false;

	if (running)
		return;
	running = true;
	if (f == NULL)
		f = &gdp_reclaim_resources_callback;

	long gc_intvl;
	char pbuf[200];

	snprintf(pbuf, sizeof pbuf, "swarm.%s.reclaim.interval",
			ep_app_getprogname());
	gc_intvl = ep_adm_getlongparam(pbuf, -1);
	if (gc_intvl == -1)
		gc_intvl = ep_adm_getlongparam("swarm.gdp.reclaim.interval", 15L);

	struct timeval tv = { gc_intvl, 0 };
	struct event *evtimer = event_new(_GdpIoEventBase, -1, EV_PERSIST,
								f, NULL);
	event_add(evtimer, &tv);
}


/*
**  Set libevent timer.
**
**		Timeout is in units of microseconds.
**		Not general purpose (assumes *pev never changes type).
*/

void
_gdp_evloop_timer_set(uint32_t timeout,
					libevent_event_t *cbfunc,
					void *cbarg,
					struct event **pev)
{
	struct timeval tv;
	struct event *ev = *pev;

	if (ev != NULL)
		event_free(ev);
	*pev = ev = event_new(_GdpIoEventBase, -1, 0, cbfunc, cbarg);
	ep_dbg_cprintf(DbgTimers, 52,
			"_gdp_evloop_timer_set(%" PRIu32 ") => %p\n", timeout,  ev);
	tv.tv_sec = timeout / 1000000;
	tv.tv_usec = timeout % 1000000;
	event_add(ev, &tv);
}


/*
**  Clear libevent timer.
*/

void
_gdp_evloop_timer_clr(struct event **pev)
{
	ep_dbg_cprintf(DbgTimers, 52, "_gdp_evloop_timer_clr(%p)\n", *pev);
	if (*pev != NULL)
		event_free(*pev);
	*pev = NULL;
}


/*
**	Base loop to be called for event-driven systems.
**	Their events should have already been added to the event base.
**
**		GdpIoEventLoopThread is also used by gdplogd, hence non-static.
*/

EP_THR			_GdpIoEventLoopThread;

// to ensure event loop is running before we proceed
bool					GdpIoEventLoopRunning		= false;
static EP_THR_MUTEX		GdpIoEventLoopRunningMutex	EP_THR_MUTEX_INITIALIZER;
static EP_THR_COND		GdpIoEventLoopRunningCond	EP_THR_COND_INITIALIZER;

static void
event_loop_timeout(int fd, short what, void *unused)
{
	ep_dbg_cprintf(Dbg, 79, "event loop timeout\n");
}

void *
_gdp_run_event_loop(void *eli_)
{
	long evdelay = ep_adm_getlongparam("swarm.gdp.event.loopdelay", 100000L);
									// loopdelay in microseconds

	// keep the loop alive if EVLOOP_NO_EXIT_ON_EMPTY isn't available
	long ev_timeout = ep_adm_getlongparam("swarm.gdp.event.looptimeout", 30L);
									// looptimeout in seconds
	struct timeval tv = { ev_timeout, 0 };
	struct event *evtimer = event_new(_GdpIoEventBase, -1, EV_PERSIST,
			&event_loop_timeout, NULL);
	event_add(evtimer, &tv);

	for (;;)
	{
		if (ep_dbg_test(Dbg, 20))
		{
			ep_dbg_printf("gdp_event_loop: starting up base loop\n");
			event_base_dump_events(_GdpIoEventBase, ep_dbg_getfile());
		}

		ep_thr_mutex_lock(&GdpIoEventLoopRunningMutex);
		GdpIoEventLoopRunning = true;
		ep_thr_cond_broadcast(&GdpIoEventLoopRunningCond);
		ep_thr_mutex_unlock(&GdpIoEventLoopRunningMutex);

#ifdef EVLOOP_NO_EXIT_ON_EMPTY
		event_base_loop(_GdpIoEventBase, EVLOOP_NO_EXIT_ON_EMPTY);
#else
		event_base_loop(_GdpIoEventBase, 0);
#endif

		GdpIoEventLoopRunning = false;

		if (ep_dbg_test(Dbg, 1))
		{
			ep_dbg_printf("gdp_event_loop: event_base_loop returned\n");
			if (event_base_got_break(_GdpIoEventBase))
				ep_dbg_printf(" ... as a result of loopbreak\n");
			if (event_base_got_exit(_GdpIoEventBase))
				ep_dbg_printf(" ... as a result of loopexit\n");
		}
		if (event_base_got_exit(_GdpIoEventBase) ||
				event_base_got_break(_GdpIoEventBase))
		{
			// the GDP daemon went away intentionally
			return NULL;
		}

		if (evdelay > 0)
			ep_time_nanosleep(evdelay * 1000LL);		// avoid CPU hogging
	}

	ep_log(GDP_STAT_DEAD_DAEMON, "lost channel to gdp");
	ep_app_abort("lost channel to gdp");
}

static EP_STAT
_gdp_start_event_loop_thread(EP_THR *thr)
{
	if (ep_thr_spawn(thr, _gdp_run_event_loop, NULL) != 0)
		return init_error("cannot create event loop thread",
						"_gdp_start_event_loop_thread");
	else
		return EP_STAT_OK;
}

void
_gdp_stop_event_loop(void)
{
	event_base_loopbreak(_GdpIoEventBase);
	ep_thr_mutex_lock(&GdpIoEventLoopRunningMutex);
	GdpIoEventLoopRunning = false;
	ep_thr_cond_broadcast(&GdpIoEventLoopRunningCond);
	ep_thr_mutex_unlock(&GdpIoEventLoopRunningMutex);
}


/*
**   Logging callback for event library (for debugging).
*/

static EP_DBG	EvlibDbg = EP_DBG_INIT("gdp.libevent", "GDP Libevent");

static void
evlib_log_cb(int severity, const char *msg)
{
	const char *sev;
	const char *sevstrings[] = { "debug", "msg", "warn", "error" };

	if (severity < 0 || severity > 3)
		sev = "?";
	else
		sev = sevstrings[severity];
	ep_dbg_cprintf(EvlibDbg, ((4 - severity) * 20) + 2, "[%s] %s\n", sev, msg);
}


/*
**  Arrange to call atexit(3) functions on SIGINT and SIGTERM
*/

static void
exit_on_signal(int sig)
{
	ep_app_warn("Exiting on signal %d", sig);
	_gdp_stop_event_loop();
	exit(sig);
}


/*
**  Change user id to something innocuous.
*/

void
_gdp_run_as(const char *runasuser)
{
	if (runasuser != NULL && *runasuser != '\0')
	{
		uid_t uid;
		gid_t gid;
		struct passwd *pw = getpwnam(runasuser);
		if (pw == NULL)
		{
			ep_app_warn("User %s unknown; running as 1:1 (daemon)",
					runasuser);
			gid = 1;
			uid = 1;
		}
		else
		{
			gid = pw->pw_gid;
			uid = pw->pw_uid;
		}
		if (setgid(gid) < 0 || setuid(uid) < 0)
			ep_app_warn("Cannot set user/group id (%d:%d)", uid, gid);
	}
}


/*
**  Print all outstanding requests on a channel
*/

void
_gdp_chan_dumpreqs(gdp_chan_t *chan, FILE *fp)
{
	gdp_req_t *req;
	gdp_chan_x_t *chanx = _gdp_chan_get_cdata(chan);

	if (chanx == NULL)
	{
		fprintf(fp, "\n<<< No Requests >>>\n");
		return;
	}
	fprintf(fp, "\n<<< Active requests >>>\n");
	LIST_FOREACH(req, &chanx->reqs, chanlist)
	{
		_gdp_req_dump(req, fp, GDP_PR_PRETTY, 0);
	}
}


/*
**  SIGINFO --- called to print out internal state (for debugging)
**
**		On BSD and MacOS this is implemented as a SIGINFO (^T from
**		the command line), but since Linux doesn't have that we use
**		SIGUSR1 instead.
*/

extern const char	GdpVersion[];
EP_FUNCLIST			*_GdpDumpFuncs;

void
_gdp_dump_state(int plev)
{
	FILE *fp = stderr;			// should this be the debug file?
	flockfile(fp);
	fprintf(fp, "\n<<< GDP STATE >>>\nVersion: %s\n", GdpVersion);

	_gdp_gob_cache_dump(plev, fp);			// GOB cache contents
	_gdp_chan_dumpreqs(_GdpChannel, fp);	// outstanding requests

	if (_GdpDumpFuncs != NULL)
		ep_funclist_invoke(_GdpDumpFuncs, (void *) fp);

	fprintf(fp, "\n<<< Open file descriptors >>>\n");
	ep_app_dumpfds(fp);
	fprintf(fp, "\n<<< Stack backtrace >>>\n");
	ep_dbg_backtrace(fp);
	fprintf(fp, "\n<<< Statistics >>>\n");
	_gdp_req_pr_stats(fp);
	_gdp_gob_pr_stats(fp);
	funlockfile(fp);
}


static void
siginfo(int sig, short what, void *arg)
{
	if (ep_dbg_test(Dbg, 1))
		_gdp_dump_state(GDP_PR_DETAILED);
	else
		_gdp_dump_state(GDP_PR_PRETTY);
}



/*
**  Initialization, Part 0:
**		Initialize external libraries.
**
**		Used by a few of the utility routines, but unusual in that
**		it doesn't actually start up GDP communications.
*/

EP_STAT
gdp_init_phase_0(const char *progname, uint32_t flags)
{
	ep_dbg_cprintf(Dbg, 4, "gdp_init_phase_0: %s\n", GdpVersion);

	if (_GdpInitState >= GDP_INIT_PHASE_0)
		return EP_STAT_OK;

	// initialize the EP library
	ep_lib_init(EP_LIB_USEPTHREADS);
	_GdpDumpFuncs = ep_funclist_new("GDP debug dump functions");

	// initialize runtime parameters
	_gdp_adm_readparams("gdp");
	if (progname == NULL)
		progname = ep_app_getprogname();
	if (progname != NULL)
		_gdp_adm_readparams(progname);
	ep_crypto_init(0);

	// clear out spurious errors
	errno = 0;

	// we can now re-adjust debugging
	ep_dbg_setfile(NULL);

	// register status strings
	_gdp_stat_init();

	// if not using Zeroconf, disable it
	if (EP_UT_BITSET(GDP_INIT_NO_ZEROCONF, flags))
		ep_adm_setparam("swarm.gdp.zeroconf.enable", "false");

	_GdpInitState = GDP_INIT_PHASE_0;

	return EP_STAT_OK;
}


/*
**  Initialization, Part 1:
**		Initialize the various external libraries.
**		Set up the I/O event loop base.
**		Initialize the GOB cache.
**		Start the event loop.
*/

// locks out multiple calls to gdp_lib_init
static EP_THR_MUTEX		GdpInitMutex	EP_THR_MUTEX_INITIALIZER;
int						_GdpInitState;

EP_STAT
gdp_lib_init(const char *progname, const char *myname, uint32_t flags)
{
	EP_STAT estat = EP_STAT_OK;
	const char *phase = NULL;

	ep_dbg_cprintf(Dbg, 4, "_gdp_lib_init(%s)\n",
			myname == NULL ? "NULL" : myname);

	// need to initialize libep before using mutexes
	phase = "ep_lib_init";
	estat = ep_lib_init(EP_LIB_USEPTHREADS);
	EP_STAT_CHECK(estat, goto fail0);

	ep_thr_mutex_lock(&GdpInitMutex);
	if (_GdpInitState >= GDP_INIT_LIB)
		goto done;

	gdp_init_phase_0(progname, flags);

	// initialize external -> internal name mapping
	if (!EP_UT_BITSET(GDP_INIT_NO_HONGDS, flags))
	{
		if (ep_adm_getboolparam("swarm.gdp.hongd.optional", false))
			flags |= GDP_INIT_OPT_HONGDS;
		phase = "_gdp_name_init";
		estat = _gdp_name_init(NULL);
		if (EP_UT_BITSET(GDP_INIT_OPT_HONGDS, flags))
			estat = EP_STAT_OK;
		EP_STAT_CHECK(estat, goto fail0);
	}

	if (ep_dbg_test(EvlibDbg, 80))
	{
		// according to the book...
		//event_enable_debug_logging(EVENT_DBG_ALL);
		// according to the code...
		event_enable_debug_mode();
	}

	// arrange to call atexit(3) functions on SIGTERM
	if (ep_adm_getboolparam("swarm.gdp.catch.sigint", true))
		(void) signal(SIGINT, exit_on_signal);
	if (ep_adm_getboolparam("swarm.gdp.catch.sigterm", true))
		(void) signal(SIGTERM, exit_on_signal);

	// get assertion behavior information
	// [DEPRECATED: use libep.assert.allabort]
	EpAssertAllAbort = ep_adm_getboolparam("swarm.gdp.debug.assert.allabort",
									EpAssertAllAbort);

	// check to see if commands/responses should be run in threads
	_GdpRunCmdInThread = ep_adm_getboolparam("swarm.gdp.command.runinthread",
									true);
	_GdpRunRespInThread = ep_adm_getboolparam("swarm.gdp.response.runinthread",
									false);

	// figure out or generate our name (for routing)
	if (myname == NULL && progname != NULL)
	{
		char argname[100];

		snprintf(argname, sizeof argname, "swarm.%s.gdpname", progname);
		myname = ep_adm_getstrparam(argname, NULL);
	}

	if (myname != NULL)
	{
		gdp_pname_t pname;

		estat = gdp_parse_name(myname, _GdpMyRoutingName);
		ep_dbg_cprintf(Dbg, 9, "Setting my name:\n\t%s\n\t%s\n",
				myname, gdp_printable_name(_GdpMyRoutingName, pname));
		if (EP_STAT_ISFAIL(estat))
			myname = NULL;
	}

	if (!gdp_name_is_valid(_GdpMyRoutingName))
		_gdp_newname(_GdpMyRoutingName, NULL);

	// avoid running as root if possible (and another user specified)
	if (progname != NULL)
	{
		char argname[100];
		const char *logfac;

		if (getuid() == 0)
		{
			snprintf(argname, sizeof argname, "swarm.%s.runasuser", progname);
			_gdp_run_as(ep_adm_getstrparam(argname, NULL));
		}

		// allow log facilities on a per-app basis
		snprintf(argname, sizeof argname, "swarm.%s.syslog.facility", progname);
		logfac = ep_adm_getstrparam(argname, NULL);
		if (logfac == NULL)
			logfac = ep_adm_getstrparam("swarm.gdp.syslog.facility", "local4");
		ep_log_init(progname, ep_syslog_fac_from_name(logfac), stderr);
	}

	if (getuid() == 0)
		_gdp_run_as(ep_adm_getstrparam("swarm.gdp.runasuser", NULL));

	if (ep_dbg_test(Dbg, 1))
	{
		gdp_pname_t pname;

		ep_dbg_printf("My GDP routing name = %s\n",
				gdp_printable_name(_GdpMyRoutingName, pname));
	}

	// initialize the GOB cache.  In theory this "cannot fail"
	phase = "_gdp_gob_cache_init";
	estat = _gdp_gob_cache_init();
	EP_STAT_CHECK(estat, goto fail0);

	// tell the event library that we're using pthreads
	if (evthread_use_pthreads() < 0)
		return init_error("cannot use pthreads", "gdp_lib_init");
	if (ep_dbg_test(DbgEvLock, 90))
	{
		evthread_enable_lock_debuging();
	}

	// use our debugging printer
	event_set_log_callback(evlib_log_cb);

	// set up the event base
	if (_GdpIoEventBase == NULL)
	{
		// Initialize for I/O events
		struct event_config *ev_cfg = event_config_new();

		phase = "event_base_new_with_config";
		event_config_require_features(ev_cfg, 0);
		_GdpIoEventBase = event_base_new_with_config(ev_cfg);
		if (_GdpIoEventBase == NULL)
			estat = init_error("could not create event base", "gdp_lib_init");
		event_config_free(ev_cfg);
		EP_STAT_CHECK(estat, goto fail0);

		// add a debugging signal to print out some internal data structures
#ifdef SIGINFO
		event_add(evsignal_new(_GdpIoEventBase, SIGINFO, siginfo, NULL), NULL);
#endif
		event_add(evsignal_new(_GdpIoEventBase, SIGUSR1, siginfo, NULL), NULL);
	}

	phase = "_gdp_chan_init";
	estat = _gdp_chan_init(_GdpIoEventBase, NULL);
	EP_STAT_CHECK(estat, goto fail0);

	phase = NULL;

done:
fail0:
	if (ep_dbg_test(Dbg, EP_STAT_ISOK(estat) ? 8 : 1))
	{
		char ebuf[200];
		ep_stat_tostr(estat, ebuf, sizeof ebuf);

		if (phase == NULL)
			ep_dbg_printf("gdp_lib_init: %s\n", ebuf);
		else
			ep_dbg_printf("gdp_lib_init: %s: %s\n", phase, ebuf);
	}

	_GdpInitState = GDP_INIT_LIB;
	ep_thr_mutex_unlock(&GdpInitMutex);
	return estat;
}


EP_STAT
_gdp_evloop_init(void)
{
	EP_STAT estat;

	// set up synchronization for event loop thread startup
	ep_thr_mutex_lock(&GdpIoEventLoopRunningMutex);

	if (!GdpIoEventLoopRunning)
	{
		// create a thread to run the event loop
		estat = _gdp_start_event_loop_thread(&_GdpIoEventLoopThread);
	}

	while (!GdpIoEventLoopRunning)
		ep_thr_cond_wait(&GdpIoEventLoopRunningCond,
						&GdpIoEventLoopRunningMutex, NULL);
	ep_thr_mutex_unlock(&GdpIoEventLoopRunningMutex);

	return estat;
}


/*
*/


/*
**  Data Ready (Receive) callback
**
**		Called whenever there is input from the channel.
**		It is up to this routine to actually read the data from the
**		chan level buffer into active memory.
*/

EP_STAT
_gdp_io_recv(
		gdp_chan_t *chan,
		gdp_name_t src,
		gdp_name_t dst,
		gdp_seqno_t seqno,
		gdp_buf_t *payload_buf,
		size_t payload_len)
{
	EP_STAT estat;

	gdp_pdu_t *pdu = _gdp_pdu_new(NULL, src, dst, seqno);
	estat = _gdp_pdu_in(pdu, payload_buf, payload_len, chan);
	EP_STAT_CHECK(estat, goto fail0);

	_gdp_pdu_process(pdu, chan);
	// _gdp_pdu_process frees pdu, possibly in a thread

fail0:
	{
		char ebuf[100];
		ep_dbg_cprintf(Dbg, EP_STAT_ISOK(estat) ? 21 : 3,
				"_gdp_io_recv: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}


/*
**  Router Event callback
**
**		Called when the router has something to signal to the higher level.
*/

EP_STAT
_gdp_router_event(
		gdp_chan_t *chan,
		gdp_name_t src,
		gdp_name_t dst,
		size_t payload_len,
		EP_STAT estat)
{
	// fake up a PDU for the router event
	gdp_cmd_t cmd = (gdp_cmd_t) 0;

	if (EP_STAT_IS_SAME(estat, GDP_STAT_NAK_NOROUTE))
		cmd = GDP_NAK_R_NOROUTE;
	else
		goto fail0;

	{
		//XXX wildcard seqno?
		GdpMessage *msg = _gdp_msg_new(cmd, GDP_PDU_ANY_RID, GDP_PDU_NO_L5SEQNO);
		gdp_pdu_t *pdu = _gdp_pdu_new(msg, src, dst, GDP_SEQNO_NONE);

		if (msg->cmd != 0)
			_gdp_pdu_process(pdu, chan);
		else
			_gdp_pdu_free(&pdu);
	}

fail0:
	if (ep_dbg_test(Dbg, 23))
	{
		char ebuf[100];
		ep_dbg_printf("_gdp_router_event: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}



/*
**  Data I/O Event callback
**
**		Typically connects, disconnects, and errors
**
**		Following is a list of actions that should be undertaken in
**		response to various events:
**
**		Event					Client Action				Gdplogd Action
**
**		connection established	advertise one				advertise all
**								re-subscribe all
**
**		connection lost [1]		retry open					retry open
**
**		data available			process command/ack			process command/ack
**
**		write complete			anything needed?			anything needed?
**
**		advertise timeout		re-advertise me				re-advertise all
**
**		connection close		withdraw me					withdraw all
**
**		[1] Should be handled automatically by the channel layer, but should
**		generate a "connection established" event.
*/

EP_STAT
_gdp_io_event(
		gdp_chan_t *chan,
		uint32_t what)
{
	EP_STAT estat = EP_STAT_OK;

	if (EP_UT_BITSET(BEV_EVENT_CONNECTED, what))
	{
		// connection up; do advertising and resend subscriptions (if any)
		gdp_chan_x_t *cx = _gdp_chan_get_cdata(chan);
		if (cx->connect_cb != NULL)
			estat = (*cx->connect_cb)(chan);
	}
	return estat;
}
