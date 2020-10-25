/* vim: set ai sw=4 sts=4 ts=4 : */

/*
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

#include <gdp/gdp_chan.h>
#include <gdp/gdp_version.h>

#include <ep/ep_sd.h>
#include <ep/ep_string.h>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/thread.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sysexits.h>
#include <arpa/inet.h>
#include <sys/socket.h>


static EP_DBG	Dbg = EP_DBG_INIT("gdplogd.main", "GDP Log Daemon");


__BEGIN_DECLS
uint32_t		GdplogdForgive = 0;			// treatment of gaps and dups
uint32_t		GdpSignatureStrictness = 0;	// how strongly we enforce signatures
int				LogdExitStat = EX_SOFTWARE;	// exit status

extern const char	GdplogdVersion[];
__END_DECLS


/*
**	LOGD_SOCK_CLOSE_CB --- free resources when we lose a router connection
*/

void
logd_sock_close_cb(gdp_chan_t *chan)
{
	// free any requests tied to this channel
	gdp_req_t *req = LIST_FIRST(&_gdp_chan_get_cdata(chan)->reqs);

	while (req != NULL)
	{
		gdp_req_t *req2 = LIST_NEXT(req, chanlist);
		_gdp_req_free(&req);
		req = req2;
	}
}


/*
**  LOGD_RECLAIM_RESOURCES --- called periodically to prune old resources
*/

static void
logd_reclaim_resources(void *null)
{
	_gdp_reclaim_resources(NULL);
	sub_reclaim_resources(_GdpChannel);
}


// stub for libevent

static void
logd_reclaim_resources_callback(int fd, short what, void *ctx)
{
	ep_dbg_cprintf(Dbg, 69, "logd_reclaim_resources_callback\n");
	if (ep_adm_getboolparam("swarm.gdplogd.reclaim.inthread", false))
		ep_thr_pool_run(logd_reclaim_resources, NULL);
	else
		logd_reclaim_resources(NULL);
}


static void
renew_advertisements_thread(void *_chan)
{
	gdp_chan_t *chan = (gdp_chan_t *) _chan;
	EP_STAT estat = logd_advertise_all(chan, GDP_CMD_ADVERTISE, NULL);

	if (ep_dbg_test(Dbg, EP_STAT_ISOK(estat) ? 30 : 3))
	{
		char ebuf[100];
		ep_dbg_printf("renew_advertisements_thread: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
}


static void
renew_advertisements_callback(int fd, short what, void *_chan)
{
	ep_sd_notifyf("WATCHDOG=1\n");
	ep_thr_pool_run(&renew_advertisements_thread, _chan);

	// schedule next advertisement
	gdp_chan_t *chan = (gdp_chan_t *) _chan;
	gdp_chan_x_t *chanx = _gdp_chan_get_cdata(chan);
	if (chanx->adv_timer != NULL)
	{
		if (chanx->adv_intvl > 0)
		{
			struct timeval tv = { chanx->adv_intvl, 0 };
			event_add(chanx->adv_timer, &tv);
		}
		else
		{
			event_del(chanx->adv_timer);
			chanx->adv_timer = NULL;
		}
	}
}


/*
**  Do shutdown at exit
*/

void
shutdown_req(gdp_req_t *req)
{
	if (ep_dbg_test(Dbg, 49))
	{
		ep_dbg_printf("shutdown_req: ");
		_gdp_req_dump(req, ep_dbg_getfile(), GDP_PR_BASIC, 0);
	}

	EP_ASSERT_ELSE(req != NULL, return);
	EP_ASSERT_ELSE(req->cpdu != NULL, return);
	EP_ASSERT_ELSE(req->cpdu->msg != NULL, return);

	if (EP_UT_BITSET(GDP_REQ_SRV_SUBSCR, req->flags))
	{
		gdp_msg_t *msg = _gdp_msg_new(GDP_NAK_S_LOST_SUBSCR,
									req->cpdu->msg->rid,
									req->cpdu->msg->l5seqno);
		gdp_pdu_t *pdu = _gdp_pdu_new(msg, req->gob->name, req->cpdu->src,
									GDP_SEQNO_NONE);
		gdp_pdu_t *save_pdu = req->rpdu;
		req->rpdu = pdu;

//		EP_STAT estat = _gdp_req_new(
//								GDP_NAK_S_LOST_SUBSCR,
//								req->gob,
//								_GdpChannel,
//								pdu,
//								0,
//								&pubreq);
		sub_send_message_notification(req);
		req->rpdu = save_pdu;
	}
}

void
logd_shutdown(void)
{
	if (ep_adm_getboolparam("gdplogd.shutdown.flushcache", true))
	{
		ep_dbg_cprintf(Dbg, 1, "\n\n*** Shutting down GCL cache ***\n");
		_gdp_gob_cache_shutdown(&shutdown_req);
	}

	ep_dbg_cprintf(Dbg, 1, "\n\n*** Withdrawing all advertisements ***\n");
	logd_advertise_all(_GdpChannel, GDP_CMD_WITHDRAW, NULL);

	ep_dbg_cprintf(Dbg, 1, "\n\n*** Shutting down event loop ***\n");
	_gdp_stop_event_loop();
}


/*
**  SIGTERM --- called on interrupt or kill to do clean shutdown
*/

void
sigterm(int sig)
{
	signal(sig, SIG_DFL);
	ep_sd_notifyf("STOPPING=1\nSTATUS=Terminating on signal %d\n", sig);
	ep_log(EP_STAT_OK, "Terminating on signal %d", sig);
	if (ep_dbg_test(Dbg, 1))
		_gdp_dump_state(GDP_PR_DETAILED);
	_gdp_stop_event_loop();
	LogdExitStat = EX_TEMPFAIL;
	ep_dbg_cprintf(Dbg, 20, "Finishing sigterm\n");
	// event loop will exit, then we'll do final shutdown
}


/*
**  SIGABORT --- called on quit or abort to dump state
*/

void
sigabort(int sig)
{
	signal(sig, SIG_DFL);
	ep_sd_notifyf("STOPPING=1\nSTATUS=Aborting on signal %d\n", sig);
	ep_log(EP_STAT_ABORT, "Aborting on signal %d", sig);
	_gdp_dump_state(GDP_PR_DETAILED);
	_gdp_stop_event_loop();

	// for whatever reason, this doesn't give a core dump on MacOS
	//kill(getpid(), sig);		// this will not do cleanup
	abort();
	char *p = NULL;
	*p = 0;
}


/*
**  Dump state when an assertion fails.
*/

static void
assertion_dump(void)
{
	_gdp_dump_state(GDP_PR_DETAILED);
}


static uint32_t
sig_strictness(const char *s)
{
	uint32_t strictness = 0;

	while (*s != '\0')
	{
		while (isspace(*s) || ispunct(*s))
			s++;
		switch (*s)
		{
			case 'v':
			case 'V':
				strictness |= GDP_SIG_MUSTVERIFY;
				break;

			case 'r':
			case 'R':
				strictness |= GDP_SIG_REQUIRED;
				break;

			case 'p':
			case 'P':
				strictness |= GDP_SIG_PUBKEYREQ;
				break;
		}

		while (isalnum(*++s))
			continue;
	}

	return strictness;
}


static void
usage(const char *err)
{
	fprintf(stderr,
			"Usage error: %s\n"
			"Usage: %s [-D dbgspec] [-F] [-G router_addr] [-n nworkers]\n"
			"\t[-N myname] [-s strictness]\n"
			"    -D  set debugging flags\n"
			"    -F  run in foreground\n"
			"    -G  IP host to contact for gdp router\n"
			"    -n  number of worker threads\n"
			"    -N  set my GDP name (address)\n"
			"    -s  set signature strictness; comma-separated subflags are:\n"
			"\t    verify (if present, signature must verify)\n"
			"\t    required (signature must be included)\n"
			"\t    pubkey (public key must be present)\n",
			err, ep_app_getprogname());
	exit(EX_USAGE);
}


/*
**  MAIN!
**
**		XXX	Currently always runs in foreground.  This will change
**			to run in background unless -D or -F are specified.
**			Running in background should probably also turn off
**			SIGINFO, since it doesn't make sense in that context.
*/

int
main(int argc, char **argv)
{
	int opt;
	bool run_in_foreground = false;
	EP_STAT estat;
	int nworkers = -1;
	const char *router_addr = NULL;
	const char *phase;
	const char *myname = NULL;
	const char *progname;

	while ((opt = getopt(argc, argv, "D:FG:n:N:s:")) > 0)
	{
		switch (opt)
		{
		case 'D':
			run_in_foreground = true;
			ep_dbg_set(optarg);
			break;

		case 'F':
			run_in_foreground = true;
			break;

		case 'G':
			router_addr = optarg;
			break;

		case 'n':
			nworkers = atoi(optarg);
			break;

		case 'N':
			myname = optarg;
			break;

		case 's':
			GdpSignatureStrictness |= sig_strictness(optarg);
			break;

		default:
			usage("unknown flag");
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		usage("no positional arguments allowed");

	/*
	**  Do initialization.  This is very order-sensitive
	*/

	// initialize GDP and the EVENT library
	phase = "gdp library";
	estat = gdp_lib_init("gdplogd", myname, 0);
	EP_STAT_CHECK(estat, goto fail0);

	// initialize physical logs (expand this if multiple log implementations)
	phase = "gcl physlog";
	estat = GdpSqliteImpl.init(NULL);
	EP_STAT_CHECK(estat, goto fail0);

	// initialize the protocol module
	phase = "gdplogd protocol module";
	estat = gdpd_proto_init();
	EP_STAT_CHECK(estat, goto fail0);

	progname = ep_app_getprogname();

	if (myname == NULL)
	{
		if (progname != NULL)
		{
			char argname[100];

			snprintf(argname, sizeof argname, "swarm.%s.gdpname", progname);
			myname = ep_adm_getstrparam(argname, NULL);
		}
	}

	ep_dbg_cprintf(Dbg, 1, "%s\n", GdplogdVersion);

	// print our name as a reminder
	{
		gdp_pname_t pname;

		if (myname == NULL)
			myname = gdp_printable_name(_GdpMyRoutingName, pname);

		fprintf(stdout, "My GDP routing name = %s\n", myname);
	}

	// set up signature strictness
	{
		const char *p;

		p = ep_adm_getstrparam("swarm.gdplogd.crypto.strictness", "v");
		GdpSignatureStrictness |= sig_strictness(p);
	}
	ep_dbg_cprintf(Dbg, 8, "Signature strictness = 0x%x\n",
			GdpSignatureStrictness);

	// check for some options
#if _GDPLOGD_FORGIVING
	if (ep_adm_getboolparam("swarm.gdplogd.sequencing.allowgaps", true))
		GdplogdForgive |= FORGIVE_LOG_GAPS;
	if (ep_adm_getboolparam("swarm.gdplogd.sequencing.allowdups", true))
		GdplogdForgive |= FORGIVE_LOG_DUPS;
#endif

	// go into background mode (before creating any threads!)
	if (!run_in_foreground)
	{
		// do nothing for now
	}

	// initialize the thread pool
	ep_thr_pool_init(nworkers, nworkers, 0);

	// do cleanup on termination
	signal(SIGINT, sigterm);
	signal(SIGTERM, sigterm);
	signal(SIGQUIT, sigabort);
	signal(SIGABRT, sigabort);

	// Ignore SIGPIPE, since this can happen sometimes on writes to the
	// router.  This allows us to catch the error and reconnect.
	if (ep_adm_getboolparam("swarm.gdplogd.ignore.sigpipe",
#ifdef SO_NOSIGPIPE
				false
#else
				true
#endif
				))
		signal(SIGPIPE, SIG_IGN);

	// dump state on assertion failure
	EpAssertInfo = assertion_dump;

	// arrange to clean up resources periodically
	_gdp_reclaim_resources_init(&logd_reclaim_resources_callback);

	// get advertising parameters
	logd_advertise_init();

	// open the channel connection
	phase = "connection to router";

	{
		gdp_chan_x_t *chanx = (gdp_chan_x_t *) ep_mem_zalloc(sizeof *chanx);
		LIST_INIT(&chanx->reqs);

		_GdpChannel = NULL;
		estat = _gdp_chan_open(router_addr,			// IP of router
							NULL,					// qos (unused as yet)
							&_gdp_io_recv,			// receive callback
							NULL,					// send callback
							&_gdp_io_event,			// close/error/eof callback
							&_gdp_router_event,		// router event callback
							&logd_advertise_all,	// advertise callback
							chanx,					// user channel data
							&_GdpChannel);			// output: new channel
		EP_STAT_CHECK(estat, goto fail0);
	}

	// GOBs will be advertised when connection is established

	// arrange to re-advertise on a regular basis
	//XXX This should probably be in the channel code itself, since it
	//XXX is really a router function.
	{
		gdp_chan_x_t *chanx = _gdp_chan_get_cdata(_GdpChannel);
		chanx->adv_intvl = ep_adm_getlongparam(
								"swarm.gdplogd.advertise.interval", 300);
		if (chanx->adv_intvl > 0)
		{
			chanx->adv_timer = event_new(_GdpIoEventBase, -1, 0,
										&renew_advertisements_callback,
										_GdpChannel);
			struct timeval tv = { chanx->adv_intvl, 0 };
			event_add(chanx->adv_timer, &tv);
		}
	}

	/*
	**  At this point we should be running
	*/

	ep_sd_notifyf("READY=1\n");
	_gdp_run_event_loop(NULL);
	ep_dbg_cprintf(Dbg, 11, "Fell out of event loop\n");

	ep_sd_notifyf("STOPPING=1\n");
	logd_shutdown();

	if (false)
	{
		char ebuf[100];

fail0:
		ep_app_fatal("Cannot initialize %s:\n\t%s",
				phase,
		        ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	exit(LogdExitStat);
}
