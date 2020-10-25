/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  Handle publish/subscribe requests
**
**	----- BEGIN LICENSE BLOCK -----
**	GDPLOGD: Log Daemon for the Global Data Plane
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

#include "logd.h"
#include "logd_pubsub.h"

#include <gdp/gdp_priv.h>
#include <gdp/gdp_chan.h>
#include <ep/ep.h>
#include <ep/ep_dbg.h>
#include <ep/ep_hash.h>

#include <sys/queue.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdplogd.pubsub",
								"GDP Log Daemon pub/sub handling");

extern EP_HASH	*_OpenGOBCache;		// associative cache


/*
**  SUB_SEND_MESSAGE_NOTIFICATION --- inform a subscriber of a new message
**
**		Assumes req is locked.
*/

EP_STAT
sub_send_message_notification(gdp_req_t *req)
{
	EP_STAT estat;

	if (ep_dbg_test(Dbg, 33))
	{
		ep_dbg_printf("sub_send_message_notification: ");
		_gdp_req_dump(req, NULL, GDP_PR_BASIC, 0);
	}

	// sanity checks
	EP_ASSERT(req->rpdu != NULL);
	EP_ASSERT(req->cpdu != NULL);
	EP_ASSERT(req->rpdu->msg != NULL);

	memcpy(req->rpdu->dst, req->cpdu->src, sizeof req->rpdu->dst);
	req->rpdu->msg->rid = req->cpdu->msg->rid;
	req->rpdu->msg->l5seqno = req->cpdu->msg->l5seqno;
	estat = _gdp_pdu_out(req->rpdu, req->chan);

	if (!EP_STAT_ISOK(estat))
	{
		ep_dbg_cprintf(Dbg, 1,
				"sub_send_message_notification: couldn't write PDU!\n");
	}

	return estat;
}


/*
**  SUB_NOTIFY_ALL_SUBSCRIBERS --- send something to all interested parties
**
**		pubreq should be locked when this is called.
*/

void
sub_notify_all_subscribers(gdp_req_t *pubreq)
{
	gdp_req_t *req;
	gdp_req_t *nextreq;
	long timeout;
	EP_TIME_SPEC sub_timeout;

	EP_THR_MUTEX_ASSERT_ISLOCKED(&pubreq->mutex);
	GDP_GOB_ASSERT_ISLOCKED(pubreq->gob);
	EP_ASSERT_ELSE(pubreq->rpdu != NULL, return);
	EP_ASSERT_ELSE(pubreq->rpdu->msg != NULL, return);

	// set up for subscription timeout
	{
		EP_TIME_SPEC sub_delta;
		timeout = ep_adm_getlongparam("swarm.gdplogd.subscr.timeout", 0);

		if (timeout == 0)
			timeout = ep_adm_getlongparam("swarm.gdp.subscr.timeout",
									GDP_SUBSCR_TIMEOUT_DEF);
		ep_time_from_nsec(-timeout SECONDS, &sub_delta);
		ep_time_deltanow(&sub_delta, &sub_timeout);
	}

	if (ep_dbg_test(Dbg, 32))
	{
		EP_TIME_SPEC now;
		char tbuf[100];

		ep_time_now(&now);
		ep_dbg_printf("sub_notify_all_subscribers(timeout=%ld, now=%s)\n",
					timeout,
					ep_time_format(&now, tbuf, sizeof tbuf, EP_TIME_FMT_HUMAN));
		ep_dbg_printf("%spub", _gdp_pr_indent(1));
		_gdp_req_dump(pubreq, ep_dbg_getfile(), GDP_PR_BASIC, 1);
	}

	pubreq->gob->flags |= GOBF_KEEPLOCKED;
	for (req = LIST_FIRST(&pubreq->gob->reqs); req != NULL; req = nextreq)
	{
		_gdp_req_lock(req);
		nextreq = LIST_NEXT(req, goblist);
		EP_ASSERT_ELSE(req != nextreq, break);

		// make sure we don't tell ourselves
		if (req == pubreq)
		{
			_gdp_req_unlock(req);
			continue;
		}

		if (ep_dbg_test(Dbg, 59))
		{
			ep_dbg_printf("sub_notify_all_subscribers: checking ");
			_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
		}

		// notify subscribers
		if (!EP_UT_BITSET(GDP_REQ_SRV_SUBSCR, req->flags))
		{
			ep_dbg_cprintf(Dbg, 59,
					"   ... not a subscription (flags = 0x%x)\n",
					req->flags);
		}
		else if (!ep_time_before(&req->sub_ts, &sub_timeout))
		{
			EP_STAT estat;
			EP_ASSERT_ELSE(req->cpdu != NULL, continue);
			EP_ASSERT_ELSE(req->cpdu->msg != NULL, continue);
			gdp_pdu_t *save_pdu = req->rpdu;
			req->rpdu = pubreq->rpdu;
			estat = sub_send_message_notification(req);
			req->rpdu = save_pdu;
			if (EP_STAT_ISOK(estat))
			{
				// XXX: This won't really work in case of holes.
				req->nextrec++;

				if (req->numrecs > 0 && --req->numrecs <= 0)
					sub_end_subscription(req);
			}
		}
		else
		{
			// this subscription seems to be dead
			if (ep_dbg_test(Dbg, 18))
			{
				char tbuf[100];
				ep_time_format(&sub_timeout, tbuf, sizeof tbuf,
							EP_TIME_FMT_HUMAN);
				ep_dbg_printf("sub_notify_all_subscribers: "
						"subscription timeout (%s):\n%s",
						tbuf, _gdp_pr_indent(1));
				_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 1);
				ep_dbg_printf("%s", _gdp_pr_indent(1));
				_gdp_gob_dump(req->gob, ep_dbg_getfile(), GDP_PR_BASIC, 1);
			}

			// actually remove the subscription
			//XXX isn't this done by _gdp_req_free???
			//LIST_REMOVE(req, goblist);

			EP_ASSERT(req->gob != NULL);
			EP_ASSERT(EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags));
			_gdp_req_free(&req);
		}
		if (req != NULL)
			_gdp_req_unlock(req);
	}
	pubreq->gob->flags &= ~GOBF_KEEPLOCKED;
}


/*
**  SUB_END_SUBSCRIPTION --- terminate a subscription
**
**		req and req->gob should be locked when this is called.
*/

void
sub_end_subscription(gdp_req_t *req)
{

	EP_THR_MUTEX_ASSERT_ISLOCKED(&req->mutex);
	GDP_GOB_ASSERT_ISLOCKED(req->gob);

	// make it not persistent and not a subscription
	req->flags &= ~(GDP_REQ_PERSIST | GDP_REQ_SRV_SUBSCR);

	// remove the request from the work list
	if (EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags))
	{
		gdp_gob_t *gob = req->gob;
		LIST_REMOVE(req, goblist);
		req->flags &= ~GDP_REQ_ON_GOB_LIST;
		EP_ASSERT(gob->refcnt > 1);
		_gdp_gob_decref(&gob, true);
	}

	// make sure we have a response message available
	if (req->rpdu == NULL || req->rpdu->msg == NULL)
	{
		GdpMessage *msg = ep_mem_malloc(sizeof *msg);
		gdp_message__init(msg);
		if (req->rpdu == NULL)
			req->rpdu = _gdp_pdu_new(msg, req->cpdu->dst, req->cpdu->src,
									GDP_SEQNO_NONE);
		req->rpdu->msg = msg;
	}

	// send an "end of subscription" event
	req->rpdu->msg->rid = req->cpdu->msg->rid;
	req->rpdu->msg->cmd = GDP_ACK_END_OF_RESULTS;

	if (ep_dbg_test(Dbg, 39))
	{
		ep_dbg_printf("sub_end_subscription removing:\n  ");
		_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
	}

	(void) _gdp_pdu_out(req->rpdu, req->chan);
}


/*
**  Unsubscribe all requests for a given gob and destination.
**  Can also optionally select a particular request id.
*/

EP_STAT
sub_end_all_subscriptions(
		gdp_gob_t *gob,
		gdp_name_t dest,
		gdp_rid_t rid)
{
	EP_STAT estat;
	gdp_req_t *req;
	gdp_req_t *nextreq;

	if (ep_dbg_test(Dbg, 29))
	{
		gdp_pname_t dst_p;
		ep_dbg_printf("sub_end_all_subscriptions: rid %" PRIgdp_rid " dst %s\n    ",
				rid, gdp_printable_name(dest, dst_p));
		_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_BASIC, 1);
	}

	GDP_GOB_ASSERT_ISLOCKED(gob);
	if (EP_UT_BITSET(GOBF_KEEPLOCKED, gob->flags) && ep_dbg_test(Dbg, 1))
		ep_dbg_printf("sub_end_all_subscriptions: GOBF_KEEPLOCKED on entry\n");
	gob->flags |= GOBF_KEEPLOCKED;

	do
	{
		estat = EP_STAT_OK;
		for (req = LIST_FIRST(&gob->reqs); req != NULL; req = nextreq)
		{
			estat = _gdp_req_lock(req);
			EP_STAT_CHECK(estat, break);
			nextreq = LIST_NEXT(req, goblist);
			if (!GDP_NAME_SAME(req->cpdu->dst, dest) ||
					(rid != GDP_PDU_NO_RID && rid != req->cpdu->msg->rid) ||
					!EP_ASSERT(req->gob == gob))
			{
				_gdp_req_unlock(req);
				continue;
			}

			// remove subscription for this destination (but keep GOB locked)
			if (ep_dbg_test(Dbg, 39))
			{
				ep_dbg_printf("sub_end_all_subscriptions removing ");
				_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
			}
			LIST_REMOVE(req, goblist);
			req->flags &= ~GDP_REQ_ON_GOB_LIST;
			_gdp_gob_decref(&req->gob, false);
			_gdp_req_free(&req);
		}
	} while (!EP_STAT_ISOK(estat));
	gob->flags &= ~GOBF_KEEPLOCKED;
	return estat;
}


/*
**  SUB_RECLAIM_RESOURCES --- remove any expired subscriptions
**
**		This is a bit tricky to get lock ordering correct.  The
**		obvious implementation is to loop through the channel
**		list, but when you try to lock a GOB or a request you
**		have a lock ordering problem (the channel is quite low
**		in the locking hierarchy).  Instead you run through
**		the GOB hash table.
*/

// helper (does most of the work)
static void
gob_reclaim_subscriptions(gdp_gob_t *gob)
{
	int istat;
	gdp_req_t *req;
	gdp_req_t *nextreq;
	EP_TIME_SPEC sub_timeout;

	// just in case
	if (gob == NULL)
		return;

	{
		EP_TIME_SPEC sub_delta;
		long timeout = ep_adm_getlongparam("swarm.gdp.subscr.timeout",
								GDP_SUBSCR_TIMEOUT_DEF);

		ep_time_from_nsec(-timeout SECONDS, &sub_delta);
		ep_time_deltanow(&sub_delta, &sub_timeout);
		ep_dbg_cprintf(Dbg, 39,
				"gob_reclaim_subscriptions: GOB = %p, refcnt = %d, timeout = %ld\n",
				gob, gob->refcnt, timeout);
	}

	// don't even try locked GOBs
	// first check is to avoid extraneous errors
	if (EP_UT_BITSET(GOBF_ISLOCKED, gob->flags))
	{
		ep_dbg_cprintf(Dbg, 39, " ... skipping locked GOB\n");
		return;
	}
	istat = ep_thr_mutex_trylock(&gob->mutex);
	if (istat != 0)
	{
		if (ep_dbg_test(Dbg, 21))
		{
			ep_dbg_printf("gob_reclaim_subscriptions: gob already locked:\n    ");
			_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_BASIC, 0);
		}
		return;
	}
	gob->flags |= GOBF_ISLOCKED;	// if trylock succeeded

	nextreq = LIST_FIRST(&gob->reqs);
	while ((req = nextreq) != NULL)
	{
		if (ep_dbg_test(Dbg, 59))
		{
			ep_dbg_printf("gob_reclaim_subscriptions: checking ");
			_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
		}

		// now that GOB is locked, we lock the request
		istat = ep_thr_mutex_trylock(&req->mutex);
		if (istat != 0)		// checking on status of req lock attempt
		{
			// already locked
			if (ep_dbg_test(Dbg, 41))
			{
				ep_dbg_printf("gob_reclaim_subscriptions: req already locked:\n    ");
				_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
			}
			_gdp_gob_unlock(req->gob);
			continue;
		}

		// get next request while locked and do sanity checks
		nextreq = LIST_NEXT(req, goblist);
		if (!EP_ASSERT(req != nextreq) || !EP_ASSERT(req->gob == gob))
		{
			_gdp_gob_unlock(req->gob);
			break;
		}


		if (!EP_UT_BITSET(GDP_REQ_SRV_SUBSCR, req->flags))
		{
			ep_dbg_cprintf(Dbg, 59, "   ... not a subscription (flags = 0x%x)\n",
					req->flags);
		}
		else if (ep_time_before(&req->sub_ts, &sub_timeout))
		{
			// this subscription seems to be dead
			if (ep_dbg_test(Dbg, 18))
			{
				ep_dbg_printf("    ...  subscription timeout: ");
				_gdp_gob_dump(req->gob, ep_dbg_getfile(), GDP_PR_BASIC, 0);
			}

			// have to manually remove req from lists to avoid lock inversion
			if (EP_UT_BITSET(GDP_REQ_ON_GOB_LIST, req->flags))
			{
				// gob is already locked
				LIST_REMOVE(req, goblist);
			}
			if (EP_UT_BITSET(GDP_REQ_ON_CHAN_LIST, req->flags))
			{
				LIST_REMOVE(req, chanlist);			// chan already locked
			}
			req->flags &= ~(GDP_REQ_ON_GOB_LIST | GDP_REQ_ON_CHAN_LIST);
			_gdp_gob_decref(&req->gob, true);
			_gdp_req_free(&req);
		}
		else if (ep_dbg_test(Dbg, 59))
		{
			ep_dbg_printf("    ... not yet time\n");
		}

		if (req != NULL)
			_gdp_req_unlock(req);
	}

	if (gob != NULL)
		_gdp_gob_unlock(gob);
}

void
sub_reclaim_resources(gdp_chan_t *chan)
{
	_gdp_gob_cache_foreach(gob_reclaim_subscriptions);
}
