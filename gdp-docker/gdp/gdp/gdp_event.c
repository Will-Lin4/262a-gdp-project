/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  GDP_EVENT.C --- event handling
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

#include <ep/ep_thr.h>
#include <ep/ep_dbg.h>
#include <ep/ep_funclist.h>
#include <ep/ep_log.h>

#include "gdp.h"
#include "gdp_event.h"
#include "gdp_chan.h"
#include "gdp_priv.h"

#include <string.h>
#include <sys/errno.h>


static EP_DBG	Dbg = EP_DBG_INIT("gdp.event", "GDP event handling");

#define DATA_TO_DEFAULT	100000		// "content" timeout (in usec)
#define DONE_TO_DEFAULT	250000		// "no more results" timeout (in usec)


// free (unused) events
static EP_THR_MUTEX		FreeListMutex	EP_THR_MUTEX_INITIALIZER2(GDP_MUTEX_LORDER_LEAF);
static struct gev_list	FreeList		= TAILQ_HEAD_INITIALIZER(FreeList);

// active events (synchronous, ready for gdp_event_next)
static EP_THR_MUTEX		ActiveListMutex	EP_THR_MUTEX_INITIALIZER2(GDP_MUTEX_LORDER_LEAF);
static EP_THR_COND		ActiveListSig	EP_THR_COND_INITIALIZER;
static struct gev_list	ActiveList		= TAILQ_HEAD_INITIALIZER(ActiveList);

// callback events (asynchronous, ready for delivery in callback thread)
static EP_THR_MUTEX		CallbackListMutex	EP_THR_MUTEX_INITIALIZER2(GDP_MUTEX_LORDER_LEAF);
static EP_THR_COND		CallbackListSig		EP_THR_COND_INITIALIZER;
static struct gev_list	CallbackList		= TAILQ_HEAD_INITIALIZER(CallbackList);
static EP_THR			CallbackThread;
static bool				CallbackThreadStarted	= false;


/*
**  Create a new event.
*/

static EP_STAT
_gdp_event_new(gdp_event_t **gevp)
{
	gdp_event_t *gev = NULL;

	for (;;)
	{
		ep_thr_mutex_lock(&FreeListMutex);
		if ((gev = TAILQ_FIRST(&FreeList)) != NULL)
			TAILQ_REMOVE(&FreeList, gev, queue);
		ep_thr_mutex_unlock(&FreeListMutex);
		if (gev == NULL || gev->type == _GDP_EVENT_FREE)
			break;

		// error: abandon this event
		EP_ASSERT_PRINT("_gdp_event_new: allocated event %p on free list", gev);
	}
	if (gev == NULL)
	{
		gev = (gdp_event_t *) ep_mem_zalloc(sizeof *gev);
	}
	VALGRIND_HG_CLEAN_MEMORY(gev, sizeof *gev);
	*gevp = gev;
	ep_dbg_cprintf(Dbg, 48, "_gdp_event_new => %p\n", gev);
	return EP_STAT_OK;
}


/*
**  Free an event.
**		The client using the event interface must call this.
**		Clients using callbacks do not need this and MUST NOT
**			attempt to do so.
*/

EP_STAT
gdp_event_free(gdp_event_t *gev)
{
	if (!EP_ASSERT_POINTER_VALID(gev))
		return EP_STAT_NULL_POINTER;

	ep_dbg_cprintf(Dbg, 48, "gdp_event_free(%p)\n", gev);
	if (!EP_ASSERT(gev->type != _GDP_EVENT_FREE))
	{
		// apparently this is already free
		return EP_STAT_ASSERT_ABORT;
	}

	gev->type = _GDP_EVENT_FREE;
	if (gev->datum != NULL)
		gdp_datum_free(gev->datum);
	gev->datum = NULL;
#if GDP_DEBUG_NO_FREE_LISTS		// avoid helgrind complaints
	ep_mem_free(gev);
#else
	ep_thr_mutex_lock(&FreeListMutex);
	TAILQ_INSERT_HEAD(&FreeList, gev, queue);
	ep_thr_mutex_unlock(&FreeListMutex);
#endif
	return EP_STAT_OK;
}


/*
**  Return next event.
**		Optionally, specify a GCL that must match and/or a timeout.
**		If the timeout is zero this acts like a poll.
*/

gdp_event_t *
gdp_event_next(gdp_gin_t *gin, EP_TIME_SPEC *timeout)
{
	gdp_event_t *gev = NULL;
	EP_TIME_SPEC *abs_to = NULL;
	EP_TIME_SPEC tv;

	ep_dbg_cprintf(Dbg, 59, "gdp_event_next: gin %p\n", gin);

	if (timeout != NULL)
	{
		ep_time_deltanow(timeout, &tv);
		abs_to = &tv;
	}

	ep_thr_mutex_lock(&ActiveListMutex);
restart:
	for (;;)
	{
		// scan the in-memory list to see if we can go with no delay
		for (gev = TAILQ_FIRST(&ActiveList); gev != NULL;
									gev = TAILQ_NEXT(gev, queue))
		{
			// if this is the GCL we want, return it
			if (gin == NULL || gev->gin == gin)
				break;
		}
		if (gev != NULL)
			break;

		// no match yet --- wait for new active event and try again
		int err;
		ep_dbg_cprintf(Dbg, 58, "gdp_event_next: empty ActiveList; waiting\n");
		err = ep_thr_cond_wait(&ActiveListSig, &ActiveListMutex, abs_to);
		ep_dbg_cprintf(Dbg, 58, "gdp_event_next: ep_thr_cond_wait => %d\n",
				err);
		if (err != 0)
		{
			if (err != ETIMEDOUT)
			{
				char errno_buf[40];

				(void) (0 == strerror_r(err, errno_buf, sizeof errno_buf));
				EP_ASSERT_PRINT("gdp_event_next: ep_thr_cond_wait => %s",
						errno_buf);
			}
			break;
		}
	}

	if (gev != NULL)
	{
		TAILQ_REMOVE(&ActiveList, gev, queue);
		if (!EP_ASSERT(gev->type != _GDP_EVENT_FREE))
		{
			// bad news, this event is on two lists (Active and Free)
			goto restart;
		}
	}
	ep_thr_mutex_unlock(&ActiveListMutex);

	// the callback must call gdp_event_free(gev)
	ep_dbg_cprintf(Dbg, 52, "gdp_event_next => %p\n", gev);
	return gev;
}


/*
**  Free all active events associated with a given GIN.
**		Used when closing a log.
**		Pending events are associated with reqs and hence are
**		deleted when the reqs are freed from the GOB.
*/

EP_STAT
_gdp_event_free_all(gdp_gin_t *gin)
{
	gdp_event_t *gev, *next_gev;

	GDP_GIN_CHECK_RETURN_STAT(gin);

	ep_thr_mutex_lock(&CallbackListMutex);
	for (gev = TAILQ_FIRST(&CallbackList); gev != NULL; gev = next_gev)
	{
		next_gev = TAILQ_NEXT(gev, queue);
		if (gev->gin != gin)
			continue;
		else if (gev->type == _GDP_EVENT_FREE)
			TAILQ_REMOVE(&CallbackList, gev, queue);
		else
			gdp_event_free(gev);
	}
	ep_thr_mutex_unlock(&CallbackListMutex);

	ep_thr_mutex_lock(&ActiveListMutex);
	for (gev = TAILQ_FIRST(&ActiveList); gev != NULL; gev = next_gev)
	{
		next_gev = TAILQ_NEXT(gev, queue);
		if (gev->gin != gin)
			continue;
		else if (gev->type == _GDP_EVENT_FREE)
			TAILQ_REMOVE(&ActiveList, gev, queue);
		else
			gdp_event_free(gev);
	}
	ep_thr_mutex_unlock(&ActiveListMutex);

	return EP_STAT_OK;
}

/*
**	Same, but for a pending event.
**		This is essentially an insertion sort based on sequence number.
**		The list (or other enclosing data structure) must be locked
**			before this is called.
**	Since the sequence number wraps, we need to do some magic to make
**		modulo compares work.
**	We start from the tail of the list on the expectation that usually
**		events appear in the correct order.
*/

static bool
modulo_gt(gdp_seqno_t a, gdp_seqno_t b)
{
	// return true if a > b
	if (a < b && (b - a) > (GDP_SEQNO_BASE / 2))
		return true;
	if (a > b && (a - b) > (GDP_SEQNO_BASE / 2))
		return false;
	return a > b;
}

static void
insert_pending_event(gdp_event_t *gev,
		gdp_req_t *req)
{
	EP_ASSERT_POINTER_VALID(gev);

	if (gev->type == _GDP_EVENT_FREE)
	{
		ep_dbg_cprintf(Dbg, 1,
				"insert_pending_event(%p): event is free, req %p\n",
				gev, req);
		return;
	}
	ep_dbg_cprintf(Dbg, 48,
			"insert_pending_event %p (%d) to req %p\n",
			gev, gev->type, req);

	// sort the event into the correct place in the list
	if (gev->type == GDP_EVENT_DONE)
	{
		static EP_TIME_SPEC done_to = { EP_TIME_NOTIME };
		if (!EP_TIME_IS_VALID(&done_to))
		{
			uint64_t to = ep_adm_getintmaxparam("swarm.gdp.event.timeout.done",
									DONE_TO_DEFAULT);
			ep_time_from_usec(to, &done_to);
			EP_ASSERT(EP_TIME_IS_VALID(&done_to));
		}

		// done ("no more results") always goes at end of list
		ep_dbg_cprintf(Dbg, 41,
				"insert_pending_event(DONE): seq %5" PRIgdp_seqno
				", rseq %5" PRIgdp_seqno "\n",
				gev->seqno, req->seqnext);
		ep_time_deltanow(&done_to, &gev->timeout);
		TAILQ_INSERT_TAIL(&req->events, gev, queue);
	}
	else
	{
		// compute the normal (data) timeout
		static EP_TIME_SPEC data_to = { EP_TIME_NOTIME };
		if (!EP_TIME_IS_VALID(&data_to))
		{
			uint64_t to = ep_adm_getintmaxparam("swarm.gdp.event.timeout.data",
									DATA_TO_DEFAULT);
			ep_time_from_usec(to, &data_to);
			EP_ASSERT(EP_TIME_IS_VALID(&data_to));
		}
		ep_time_deltanow(&data_to, &gev->timeout);

		// search backwards through the list to find where this event fits
		gdp_event_t *next_ev = TAILQ_LAST(&req->events, gev_list);
		if (next_ev == NULL ||
				gev->type == GDP_EVENT_DONE ||
				(modulo_gt(gev->seqno, next_ev->seqno) &&
				 next_ev->type != GDP_EVENT_DONE))
		{
			// event belongs at the end of the list
			if (ep_dbg_test(Dbg, 46))
			{
				const char *tag = (gev->seqno == req->seqnext) ? "" : " -";
				if (next_ev != NULL)
				{
					ep_dbg_printf("insert_pending_event(%p): seq %5" PRIgdp_seqno
								", next %5" PRIgdp_seqno
								", rseq %5" PRIgdp_seqno "%s\n",
							gev, gev->seqno, next_ev->seqno, req->seqnext, tag);
				}
				else
				{
					ep_dbg_printf("insert_pending_event(%p): seq %5" PRIgdp_seqno
								", next -----"
								", rseq %5" PRIgdp_seqno "%s\n",
							gev, gev->seqno, req->seqnext, tag);
				}
			}
			TAILQ_INSERT_TAIL(&req->events, gev, queue);
		}
		else
		{
			if (ep_dbg_test(Dbg, 42))
			{
				const char *tag = (gev->seqno == req->seqnext) ? "+" : "*";
				ep_dbg_printf("insert_pending_event(%p): seq %5" PRIgdp_seqno
						", next %5" PRIgdp_seqno
						", rseq %5" PRIgdp_seqno " %s\n",
						gev, gev->seqno, next_ev->seqno, req->seqnext, tag);
			}

			// find correct position in list
			if (next_ev->type == GDP_EVENT_DONE)
				next_ev = TAILQ_PREV(next_ev, gev_list, queue);

			while (next_ev != NULL && modulo_gt(next_ev->seqno, gev->seqno))
				next_ev = TAILQ_PREV(next_ev, gev_list, queue);
			if (next_ev == NULL)
				TAILQ_INSERT_HEAD(&req->events, gev, queue);
			else
				TAILQ_INSERT_BEFORE(next_ev, gev, queue);
		}
	}

	if (ep_dbg_test(Dbg, 58))
	{
		for (gev = TAILQ_FIRST(&req->events); gev != NULL;
						gev = TAILQ_NEXT(gev, queue))
			ep_dbg_printf(" %d", gev->seqno);
	}

	// now figure out what should be delivered to the application
	_gdp_event_trigger_pending(req, false);
}


/*
**	Handle timeout for content events.
**
**		Since the content PDUs may arrive out of order, we may an
**		attempt to order them for applications that want them.  But
**		to avoid unbounded latency, we include a "maturity time"
**		for each event, which gets deferred until that time.  This
**		adds latency but improves ordering.  It gets triggered
**		when the head of the list (and maybe others) reaches maturity.
**
**		As a short path, if the sequence numbers indicate that there
**		return stream is up to date, events can be posted immediately.
**		That's not implemented yet, but is obviously a Good Thing.
*/

static void
pending_timeout(int unused, short what, void *req_)
{
	gdp_req_t *req = req_;

	// activate any pending items that should be delivered now
	ep_dbg_cprintf(Dbg, 51, "pending_timeout\n");
	EP_ASSERT(req->state != GDP_REQ_FREE);
	_gdp_event_trigger_pending(req, false);
}


/*
**  Handle timeout for GDP_EVENT_DONE events.
**		If a DONE PDU comes in before all data has been received,
**		the event is saved.  But if nothing else happens before this
**		timeout, assume that nothing else will come.  This is
**		generally a longer timeout than the maturity timeout, since
**		it is only triggered if a PDU is lost during transmission.
*/

static void
done_timeout(int unused, short what, void *req_)
{
	gdp_req_t *req = req_;

	// activate any pending items that should be delivered now
	ep_dbg_cprintf(Dbg, 51, "done_timeout\n");
	EP_ASSERT(req->state != GDP_REQ_FREE);
	_gdp_event_trigger_pending(req, true);
}


/*
**  Trigger an event (i.e., add to an active event queue).
**		There are two lists, depending on whether a callback was specified.
**
**	This is a bit tricky because of asynchronous results, which may
**	appear out of order.  In particular, we want to avoid returning the
**	"done" event since it really means "no more results" if there are
**	actually going to be more results appearing.  When we do get the
**	"done" event, it should contain the number of results that
**	preceded it.  If we haven't seen that many results yet we'll need
**	to tuck it away for a while.
*/

static void
_gdp_event_trigger(gdp_event_t *gev, gdp_req_t *req)
{
	EP_ASSERT_POINTER_VALID(gev);

	if (ep_dbg_test(Dbg, 48))
	{
		ep_dbg_printf("_gdp_event_trigger: gev %p req %p seqno %" PRIgdp_seqno
					" %" PRIgdp_seqno "\n",
					gev, req, gev->seqno, req->seqnext);
		char tbuf[60];
		ep_time_format(&gev->timeout, tbuf, sizeof tbuf,
					EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG6);
		ep_dbg_printf("    timeout %s\n", tbuf);
	}
	if (gev->type == _GDP_EVENT_FREE)
	{
		EP_ASSERT_PRINT("_gdp_event_trigger(%p): event is free", gev);
		return;
	}
	req->seqnext = (gev->seqno + 1) % GDP_SEQNO_BASE;

	if (ep_dbg_test(Dbg, 48))
	{
		const char *where;

		if (gev->cb == NULL)
			where = "active";
		else
			where = "callback";
		ep_dbg_printf("_gdp_event_trigger: adding event %p (%d) to %s list\n",
				gev, gev->type, where);
	}
	if (gev->type == _GDP_EVENT_FREE)
	{
		ep_dbg_cprintf(Dbg, 1, "_gdp_event_trigger(%p): event is free\n", gev);
		return;
	}

	if (gev->cb == NULL)
	{
		ep_thr_mutex_lock(&ActiveListMutex);
		TAILQ_INSERT_TAIL(&ActiveList, gev, queue);
		ep_thr_cond_broadcast(&ActiveListSig);
		ep_thr_mutex_unlock(&ActiveListMutex);
	}
	else
	{
		ep_thr_mutex_lock(&CallbackListMutex);
		TAILQ_INSERT_TAIL(&CallbackList, gev, queue);
		ep_thr_cond_signal(&CallbackListSig);
		ep_thr_mutex_unlock(&CallbackListMutex);
	}
}


/*
**	Trigger any pending events that should be active.
**		If the event is the next one we expect (based on the sequence
**			number), go ahead and trigger it and bump the expectation.
**		If the event comes before what we have already triggered, go
**			ahead and trigger it: better late than never???
**		If the event has been sitting more than the maturity timeout,
**			go ahead and trigger it.
**		Otherwise, set a new timeout corresponding to the next
**			maturity date in the (sorted) list.
*/

void
_gdp_event_trigger_pending(gdp_req_t *req, bool flush)
{
	gdp_event_t *gev;
	EP_TIME_SPEC now;

	ep_time_now(&now);
	if (ep_dbg_test(Dbg, 48))
	{
		char tbuf[60];

		ep_time_format(&now, tbuf, sizeof tbuf,
					EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG6);
		ep_dbg_printf("_gdp_event_trigger_pending: req %p seqnext %d flush %d\n"
					"    now %s first %p\n",
					req, req->seqnext, flush,
					tbuf, TAILQ_FIRST(&req->events));
	}

	while ((gev = TAILQ_FIRST(&req->events)) != NULL)
	{
		/*
		**  We want to trigger an event if:
		**		* we are flushing everything, or
		**		* the event is the next one we expect, or
		**		* the event has passed its timeout
		**	Otherwise we leave it in the list and add a timeout.
		*/

		ep_dbg_cprintf(Dbg, 56, "  gseq %d rseq %d  ",
				gev->seqno, req->seqnext);
		if (!flush &&
				gev->seqno != req->seqnext &&
				ep_time_before(&now, &gev->timeout))
		{
			ep_dbg_cprintf(Dbg, 56, "break\n");
			break;
		}

		ep_dbg_cprintf(Dbg, 56, "trigger %s\n",
				gev->type == GDP_EVENT_DONE ? "DONE" : "DATA");
		// we need to trigger this event
		TAILQ_REMOVE(&req->events, gev, queue);
		_gdp_event_trigger(gev, req);
	}

	// if anything left in the list, set the next timeout
	if (gev != NULL)
	{
		long to = ep_time_diff_usec(&now, &gev->timeout);
		ep_dbg_cprintf(Dbg, 49, "  ... setting %ld usec event timeout\n", to);
		if (gev->type == GDP_EVENT_DONE)
			_gdp_evloop_timer_set(to, &done_timeout, req, &req->ev_to);
		else
			_gdp_evloop_timer_set(to, &pending_timeout, req, &req->ev_to);
	}
	else
		ep_dbg_cprintf(Dbg, 49, "   ... no pending events\n");
}


/*
**  This is the thread that processes callbacks.
**		The event is freed, so the callback should NOT call
**			gdp_event_free.
*/

static void *
_gdp_event_thread(void *ctx)
{
	for (;;)
	{
		gdp_event_t *gev;

		// get the next event off the list
		ep_thr_mutex_lock(&CallbackListMutex);
		while ((gev = TAILQ_FIRST(&CallbackList)) == NULL)
		{
			ep_thr_cond_wait(&CallbackListSig, &CallbackListMutex, NULL);
		}
		TAILQ_REMOVE(&CallbackList, gev, queue);
		ep_thr_mutex_unlock(&CallbackListMutex);

		// sanity checks...
		EP_ASSERT(gev->cb != NULL);
		EP_ASSERT(gev->type != _GDP_EVENT_FREE);

		// now invoke it
		if (gev->cb != NULL)
			(*gev->cb)(gev);

		// don't forget to clean up (unless it's already free)
		if (gev->type != _GDP_EVENT_FREE)
			gdp_event_free(gev);
	}

	// not reached, but make gcc happy
	return NULL;
}


/*
**  _GDP_EVENT_SETCB --- set the callback function & start thread if needed
*/

void
_gdp_event_setcb(
			gdp_req_t *req,
			gdp_event_cbfunc_t cbfunc,
			void *cbarg)
{
	req->sub_cbfunc = cbfunc;
	req->sub_cbarg = cbarg;

	// if using callbacks, make sure we have a callback thread running
	if (cbfunc != NULL && !CallbackThreadStarted)
	{
		int err = ep_thr_spawn(&CallbackThread, &_gdp_event_thread, NULL);
		if (err != 0 && ep_dbg_test(Dbg, 1))
			ep_log(ep_stat_from_errno(err),
					"_gdp_gcl_setcb: cannot start callback thread");
		CallbackThreadStarted = true;
	}
}


/*
**  Create an event and link it into the queue based on a acknak req.
*/

EP_STAT
_gdp_event_add_from_req(gdp_req_t *req)
{
	EP_STAT estat = EP_STAT_OK;
	int evtype;
	GdpMessage *msg = req->rpdu->msg;
	gdp_seqno_t seqno = GDP_SEQNO_NONE;

	GDP_MSG_CHECK(req->rpdu, return EP_STAT_ASSERT_ABORT);

	// make note that we've seen activity for this subscription
	ep_time_now(&req->act_ts);

	// for the moment we only understand data responses (for subscribe)
	switch (msg->cmd)
	{
	  case GDP_ACK_SUCCESS:
		// success with no further information (many commands)
		evtype = GDP_EVENT_SUCCESS;
		break;

	  case GDP_ACK_CONTENT:
		evtype = GDP_EVENT_DATA;
		seqno = req->rpdu->seqno;
		break;

	  case GDP_ACK_END_OF_RESULTS:
		// end of subscription
		evtype = GDP_EVENT_DONE;
		seqno = req->rpdu->seqno;
		break;

	  case GDP_ACK_CREATED:
		// response to APPEND
		evtype = GDP_EVENT_CREATED;
		break;

	  case GDP_NAK_S_LOST_SUBSCR:
		evtype = GDP_EVENT_SHUTDOWN;
		req->flags &= ~GDP_REQ_PERSIST;
		break;

	  case GDP_NAK_C_REC_MISSING:
		evtype = GDP_EVENT_MISSING;
		break;

	  default:
		if (msg->cmd >= GDP_ACK_MIN && msg->cmd <= GDP_ACK_MAX)
		{
			// some sort of success
			evtype = GDP_EVENT_SUCCESS;
			req->stat = _gdp_stat_from_acknak(msg->cmd);
			break;
		}
		if (msg->cmd >= GDP_NAK_C_MIN && msg->cmd <= GDP_NAK_R_MAX)
		{
			// some sort of failure
			evtype = GDP_EVENT_FAILURE;
			req->stat = _gdp_stat_from_acknak(msg->cmd);
			break;
		}
		ep_dbg_cprintf(Dbg, 1,
				"_gdp_event_add_from_req: unexpected ack/nak %d\n",
				msg->cmd);
		estat = GDP_STAT_PROTOCOL_FAIL;
		return estat;
	}

	gdp_event_t *gev;
	estat = _gdp_event_new(&gev);
	EP_STAT_CHECK(estat, return estat);

	gev->type = evtype;
	gev->gin = req->gin;
	gev->stat = req->stat;
	gev->udata = req->sub_cbarg;
	gev->cb = req->sub_cbfunc;
	gev->datum = gdp_datum_new();
	gev->seqno = seqno;
	EP_TIME_INVALIDATE(&gev->timeout);
	if (msg->cmd == GDP_ACK_CONTENT)
	{
		EP_ASSERT(msg->ack_content->dl->n_d == 1);		//FIXME: should handle multiples
		_gdp_datum_from_pb(gev->datum, msg->ack_content->dl->d[0], msg->sig);
	}

	// schedule the event for delivery
	insert_pending_event(gev, req);
	return estat;
}


/*
**  Print an event (for debugging)
*/

void
gdp_event_print(const gdp_event_t *gev, FILE *fp)
{
	_gdp_event_dump(gev, fp, GDP_PR_PRETTY, 0);
}

void
_gdp_event_dump(const gdp_event_t *gev, FILE *fp, int detail, int indent)
{
	gdp_recno_t recno = -1;
	char ebuf[100];

	if (fp == NULL)
		fp = ep_dbg_getfile();

	if (detail >= GDP_PR_BASIC + 1)
	{
		char tbuf[100];
		ep_time_format(&gev->timeout, tbuf, sizeof tbuf,
					EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG6 | EP_TIME_FMT_NOFUZZ);
		fprintf(fp, "Event type %d, seqno %d, cbarg %p, timeout %s\n"
				"    stat %s\n",
				gev->type, gev->seqno, gev->udata, tbuf,
				ep_stat_tostr(gev->stat, ebuf, sizeof ebuf));
		indent++;
	}

	if (gev->datum != NULL)
		recno = gev->datum->recno;

	fprintf(fp, "%s", _gdp_pr_indent(indent));

	switch (gev->type)
	{
	  case GDP_EVENT_DATA:
		gdp_datum_print(gev->datum, fp, GDP_DATUM_PRTEXT);
		break;

	  case GDP_EVENT_CREATED:
		fprintf(fp, "Data created\n");
		break;

	  case GDP_EVENT_DONE:
		fprintf(fp, "End of data\n");
		break;

	  case GDP_EVENT_SHUTDOWN:
		fprintf(fp, "Log daemon shutdown\n");
		break;

	  case GDP_EVENT_SUCCESS:
		fprintf(fp, "Success: %s\n",
					ep_stat_tostr(gev->stat, ebuf, sizeof ebuf));
		break;

	  case GDP_EVENT_FAILURE:
		fprintf(fp, "Failure: %s\n",
					ep_stat_tostr(gev->stat, ebuf, sizeof ebuf));
		break;

	  case GDP_EVENT_MISSING:
		fprintf(fp, "Record %" PRIgdp_recno " missing\n", recno);
		break;

	  default:
		fprintf(fp, "Unknown event type %d: %s\n",
					gev->type, ep_stat_tostr(gev->stat, ebuf, sizeof ebuf));
		break;
	}
}


/*
**  Dump all events (debugging)
**
**		Intentionally doesn't lock the queues.
*/

void
_gdp_event_dump_all(void *unused_, void *fp_)
{
	gdp_event_t *gev;
	FILE *fp = fp_;
	int detail = GDP_PR_DETAILED;

	if (fp == NULL)
		fp = ep_dbg_getfile();

	fprintf(fp, "\n<<< Active Events >>>\n");
	TAILQ_FOREACH(gev, &ActiveList, queue)
	{
		_gdp_event_dump(gev, fp, detail, 0);
	}

	fprintf(fp, "\n<<< Callback Events >>>\n");
	TAILQ_FOREACH(gev, &CallbackList, queue)
	{
		_gdp_event_dump(gev, fp, detail, 0);
	}
}


/*
**  Getter functions for various event fields.
*/

int
gdp_event_gettype(gdp_event_t *gev)
{
	EP_ASSERT_POINTER_VALID(gev);
	return gev->type;
}


gdp_gin_t *
gdp_event_getgin(gdp_event_t *gev)
{
	EP_ASSERT_POINTER_VALID(gev);
	return gev->gin;
}


gdp_datum_t *
gdp_event_getdatum(gdp_event_t *gev)
{
	EP_ASSERT_POINTER_VALID(gev);
	return gev->datum;
}


void *
gdp_event_getudata(gdp_event_t *gev)
{
	EP_ASSERT_POINTER_VALID(gev);
	return gev->udata;
}


EP_STAT
gdp_event_getstat(gdp_event_t *gev)
{
	EP_ASSERT_POINTER_VALID(gev);
	return gev->stat;
}
