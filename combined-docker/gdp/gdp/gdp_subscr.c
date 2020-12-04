/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  ----- BEGIN LICENSE BLOCK -----
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
**  ----- END LICENSE BLOCK -----
*/

#include "gdp.h"
#include "gdp_chan.h"
#include "gdp_event.h"
#include "gdp_priv.h"
#include "gdp.pb-c.h"

#include <ep/ep.h>
#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <ep/ep_log.h>

#include <string.h>
#include <sys/errno.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.subscr", "GDP subscriptions");


static bool			SubscriptionThreadRunning;
static EP_THR		SubscriptionThreadId;
EP_THR_MUTEX		_GdpSubscriptionMutex	EP_THR_MUTEX_INITIALIZER;
struct req_head		_GdpSubscriptionRequests;


/*
**  Re-subscribe to a GCL
*/

static EP_STAT
subscr_resub(gdp_req_t *req)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "subscr_resub: refreshing req@%p\n", req);
	EP_ASSERT_ELSE(req != NULL, return EP_STAT_ASSERT_ABORT);
	EP_THR_MUTEX_ASSERT_ISLOCKED(&req->mutex);
	EP_ASSERT_ELSE(req->gin != NULL, return EP_STAT_ASSERT_ABORT);
	EP_ASSERT_ELSE(req->cpdu != NULL, return EP_STAT_ASSERT_ABORT);

	req->state = GDP_REQ_ACTIVE;

	// payload should already be set up
	memcpy(req->cpdu->dst, req->gob->name, sizeof req->cpdu->dst);
	memcpy(req->cpdu->src, _GdpMyRoutingName, sizeof req->cpdu->src);
	{
		GDP_MSG_CHECK(req->cpdu, return EP_STAT_ASSERT_ABORT);
		gdp_msg_t *msg = req->cpdu->msg;
		EP_ASSERT_ELSE(msg->cmd == GDP_CMD_SUBSCRIBE_BY_RECNO,
						return EP_STAT_ASSERT_ABORT);
		GdpMessage__CmdSubscribeByRecno *payload =
						msg->cmd_subscribe_by_recno;
		payload->has_start = true;
		payload->start = req->gob->nrecs + 1;
		payload->has_nrecs = true;
		payload->nrecs = req->numrecs;
	}

	estat = _gdp_invoke(req);

	if (ep_dbg_test(Dbg, EP_STAT_ISOK(estat) ? 20 : 1))
	{
		char ebuf[200];

		ep_dbg_printf("subscr_resub(%s) ->\n\t%s\n",
				req->gob == NULL ? "(no gob)" : req->gob->pname,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	req->state = GDP_REQ_IDLE;
	// req->rpdu might be NULL if _gdp_invoke failed
	if (req->rpdu != NULL)
	{
		gdp_message__free_unpacked(req->rpdu->msg, NULL);
		req->rpdu->msg = NULL;
	}
	if (EP_STAT_ISOK(estat))
		ep_time_now(&req->sub_ts);

	return estat;
}


/*
**  Periodically ping all open subscriptions to make sure they are
**  still happy.
*/

static void *
subscr_poker_thread(void *chan_)
{
	gdp_chan_t *chan = chan_;
	gdp_chan_x_t *chanx = _gdp_chan_get_cdata(chan);
	long timeout = ep_adm_getlongparam("swarm.gdp.subscr.timeout",
							GDP_SUBSCR_TIMEOUT_DEF);
	long delta_poke = ep_adm_getlongparam("swarm.gdp.subscr.refresh",
							timeout / 3);

	if (timeout < delta_poke)
		ep_app_warn("swarm.gdp.subscr.timeout < swarm.gdp.subscr.refresh"
					" (%ld < %ld)",
					timeout, delta_poke);
	ep_dbg_cprintf(Dbg, 10,
			"Starting subscription poker thread, delta_poke = %ld\n",
			delta_poke);

	// loop forever poking subscriptions
	for (;;)
	{
		EP_STAT estat;
		gdp_req_t *req;
		gdp_req_t *nextreq;
		EP_TIME_SPEC now;
		EP_TIME_SPEC t_poke;	// poke if older than this

		// wait for a while to avoid hogging CPU
		ep_time_nanosleep(delta_poke SECONDS / 10);
		ep_dbg_cprintf(Dbg, 40, "\nsubscr_poker_thread: poking\n");

		ep_time_now(&now);
		ep_time_from_nsec(-delta_poke SECONDS, &t_poke);
		ep_time_add_delta(&now, &t_poke);

		// do loop is in case _gdp_req_lock fails
		do
		{
			estat = EP_STAT_OK;
			for (req = LIST_FIRST(&chanx->reqs); req != NULL; req = nextreq)
			{
				estat = _gdp_req_lock(req);
				EP_STAT_CHECK(estat, break);

				nextreq = LIST_NEXT(req, chanlist);
				if (ep_dbg_test(Dbg, 51))
				{
					char tbuf[60];

					ep_time_format(&now, tbuf, sizeof tbuf, EP_TIME_FMT_HUMAN);
					ep_dbg_printf("subscr_poker_thread: at %s checking ", tbuf);
					_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
				}

				gdp_gob_t *gob = req->gob;
				_gdp_req_unlock(req);	// GOBs need to be locked before reqs

				// lock GOB, then req, then validate req
				if (ep_thr_mutex_trylock(&gob->mutex) != 0)
				{
					// not an error ... we'll get this one next time through
					ep_dbg_cprintf(Dbg, 51,
							"   ... gob->mutex trylock failed (%s)\n",
							strerror(errno));
					continue;
				}
				gob->flags |= GOBF_ISLOCKED;
				_gdp_req_lock(req);

				if (!EP_UT_BITSET(GDP_REQ_CLT_SUBSCR, req->flags))
				{
					// not a subscription: skip this entry
					ep_dbg_cprintf(Dbg, 51, "   ... not client subscription\n");
				}
				else if (ep_time_before(&t_poke, &req->sub_ts))
				{
					// we've seen activity recently, no need to poke
					ep_dbg_cprintf(Dbg, 51, "   ... not yet\n");
				}
				else
				{
					// sub_ts <= t_poke: refresh this subscription
					ep_dbg_cprintf(Dbg, 51, "   ... subscr_resub\n");
					(void) subscr_resub(req);
				}

				// if _gdp_invoke failed, try again at the next poke interval
				_gdp_req_unlock(req);
				_gdp_gob_unlock(gob);
			}
		} while (!EP_STAT_ISOK(estat));
	}

	// not reached; keep gcc happy
	ep_log(EP_STAT_SEVERE, "subscr_poker_thread: fell out of loop");
	return NULL;
}


/*
**	_GDP_GIN_SUBSCRIBE_BY_RECNO --- subscribe to a GCL
**
**		This also implements multiread.
*/

EP_STAT
_gdp_gin_subscribe(gdp_gin_t *gin,
		gdp_cmd_t cmd,
		gdp_recno_t start,
		int32_t numrecs,
		gdp_sub_qos_t *qos,
		gdp_event_cbfunc_t cbfunc,
		void *cbarg)
{
	EP_STAT estat = EP_STAT_OK;
	gdp_req_t *req;

	// create the subscribe request
	estat = _gdp_req_new(cmd, gin->gob, _GdpChannel, NULL,
			GDP_REQ_PERSIST | GDP_REQ_CLT_SUBSCR | GDP_REQ_ALLOC_RID,
			&req);
	EP_STAT_CHECK(estat, goto fail0);

	// add start and stop parameters to PDU
	req->gin = gin;

	EP_ASSERT_ELSE(req != NULL, return EP_STAT_ASSERT_ABORT);
	EP_THR_MUTEX_ASSERT_ISLOCKED(&req->mutex);
	EP_ASSERT_ELSE(req->gin != NULL, return EP_STAT_ASSERT_ABORT);
	EP_ASSERT_ELSE(req->cpdu != NULL, return EP_STAT_ASSERT_ABORT);

	errno = 0;				// avoid spurious messages

	{
		gdp_msg_t *msg = req->cpdu->msg;
		EP_ASSERT_ELSE(msg != NULL, return EP_STAT_ASSERT_ABORT);
		GdpMessage__CmdSubscribeByRecno *payload =
						msg->cmd_subscribe_by_recno;
		if (start != GDP_PDU_NO_RECNO)
		{
			payload->has_start = true;
			payload->start = start;
		}
		if (numrecs > 0)
		{
			payload->has_nrecs = true;
			payload->nrecs = numrecs;
		}
	}

	// arrange for responses to appear as events or callbacks
	_gdp_event_setcb(req, cbfunc, cbarg);

	// issue the subscription --- no data returned
	estat = _gdp_invoke(req);
	EP_ASSERT(req->state == GDP_REQ_ACTIVE);

	if (!EP_STAT_ISOK(estat))
	{
		_gdp_req_free(&req);
		goto fail0;
	}

	// at this point remaining results will be asynchronous
	req->flags |= GDP_REQ_ASYNCIO;

	// now waiting for other events; go ahead and unlock
	ep_time_now(&req->sub_ts);
	req->state = GDP_REQ_IDLE;
	if (req->rpdu != NULL)
		_gdp_pdu_free(&req->rpdu);
	ep_thr_cond_signal(&req->cond);
	_gdp_req_unlock(req);

	// the req is still on the channel list

	// start a subscription poker thread if needed (not for multiread)
	if (cmd == GDP_CMD_SUBSCRIBE_BY_RECNO)
	{
		long poke = ep_adm_getlongparam("swarm.gdp.subscr.pokeintvl", 60L);

		ep_thr_mutex_lock(&_GdpSubscriptionMutex);
		if (poke > 0 && !SubscriptionThreadRunning)
		{
			SubscriptionThreadRunning = true;
			int istat = ep_thr_spawn(&SubscriptionThreadId,
								subscr_poker_thread, req->chan);
			if (istat != 0)
			{
				EP_STAT spawn_stat = ep_stat_from_errno(istat);
				ep_log(spawn_stat, "_gdp_gin_subscribe: thread spawn failure");
			}
		}
		ep_thr_mutex_unlock(&_GdpSubscriptionMutex);
	}

fail0:
	return estat;
}

EP_STAT
_gdp_gin_unsubscribe(gdp_gin_t *gin,
		gdp_event_cbfunc_t cbfunc,
		void *cbarg,
		uint32_t reqflags)
{
	EP_STAT estat;
	gdp_req_t *req;
	gdp_req_t *sub, *next_sub;

	if (!GDP_GIN_ASSERT_ISLOCKED(gin))
		return EP_STAT_ASSERT_ABORT;

	ep_dbg_cprintf(Dbg, 1, "_gdp_gin_unsubscribe(%s) cbfunc=%p cbarg=%p\n",
			gin->gob->pname, cbfunc, cbarg);

	estat = _gdp_req_new(GDP_CMD_UNSUBSCRIBE, gin->gob, _GdpChannel, NULL,
						reqflags, &req);
	EP_STAT_CHECK(estat, goto fail0);
	req->gin = gin;

	GDP_MSG_CHECK(req->cpdu, return EP_STAT_ASSERT_ABORT);

	for (sub = LIST_FIRST(&gin->gob->reqs); sub != NULL; sub = next_sub)
	{
		_gdp_req_lock(sub);
		ep_dbg_cprintf(Dbg, 1, "... comparing to cbfunc=%p cbarg=%p\n",
				sub->sub_cbfunc, sub->sub_cbarg);

		next_sub = LIST_NEXT(req, goblist);
		if (req->gin != gin ||
				(cbfunc != NULL && cbfunc != sub->sub_cbfunc) ||
				(cbarg != NULL && cbarg != sub->sub_cbarg))
		{
			// this is not the subscription you are looking for
			ep_dbg_cprintf(Dbg, 1, "... no match\n");
			_gdp_req_unlock(sub);
			continue;
		}

		GDP_MSG_CHECK(sub->cpdu, continue);
		ep_dbg_cprintf(Dbg, 1, "... deleting rid %" PRIgdp_rid "\n",
					sub->cpdu->msg->rid);
		req->cpdu->msg->rid = sub->cpdu->msg->rid;

		estat = _gdp_invoke(req);
		EP_STAT_CHECK(estat, continue);

		GDP_MSG_CHECK(req->rpdu, return EP_STAT_ASSERT_ABORT);

		_gdp_req_free(&sub);
	}

	_gdp_req_free(&req);

fail0:
	return estat;
}
