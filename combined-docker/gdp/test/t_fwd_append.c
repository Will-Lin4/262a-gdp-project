/* vim: set ai sw=4 sts=4 ts=4 : */

/*  To compile:
cc -I.. t_fwd_append.c -Lep -Lgdp -lgdp -lep -levent -levent_pthreads -pthread -lcrypto -lavahi-client -lavahi-common
*/

#include <gdp/gdp.h>
#include <gdp/gdp_priv.h>

#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <ep/ep_time.h>

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>


/*
**  Test program for FWD_APPEND command (forward append).
**  This function should only be used for replication.
**
**  XXX STILL TO TEST:
**		* Make sure signatures are properly preserved.
**		* Try interspersed appends to multiple logs.
**		* Try interspersed appends to multiple logs from different
**		  threads.
**		* Try getting responses using callbacks.
*/


static EP_DBG	Dbg = EP_DBG_INIT("fwd_append", "GDP forwarded append test");



/*
**  CREATE_DATUM --- create sample datum for "forwarding"
*/

gdp_datum_t *
create_datum(gdp_recno_t recno)
{
	gdp_datum_t *datum = gdp_datum_new();
	char buf[32];

	datum->recno = recno;		// this violates privacy, but meh, what to do?
	snprintf(buf, sizeof buf, "record %" PRIgdp_recno, recno);
	gdp_buf_write(gdp_datum_getbuf(datum), buf, strlen(buf));

	return datum;
}



/*
**  DO_FWD_APPEND --- fake a forwarded append operation
**
**		This does not wait for results: get those using the
**		event interface.
**
**		Arguments to _gdp_gin_fwd_append:
**			gin --- log being written.
**			datum --- source datum to be forwarded.
**			srvname --- name of server to forward to.
**			cbfunc --- callback function for collecting results;
**					if NULL use gdp_event interface.
**			cbarg --- an argument passed to cbfunc.
**			chan --- the I/O channel (may be NULL).
**			reqflags --- starting flags for request structure.
*/

EP_STAT
do_fwd_append(gdp_gin_t *gin,
		gdp_datum_t *datum,
		gdp_name_t svrname,
		void *udata)
{
	EP_STAT estat;

	// start up a fwd_append
	//   ==> this is the API being tested
	estat = _gdp_gin_fwd_append(gin, datum, svrname, NULL, udata, NULL, 0);

	// check to make sure the fwd_append succeeded; if not, bail
	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[200];

		ep_app_fatal("Cannot fwd_append:\n\t%s",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	return estat;
}


void
usage(void)
{
	fprintf(stderr, "Usage: %s [-D dbgspec] [-n numrecs]\n"
			"\tlog_name server_name\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}


int
main(int argc, char **argv)
{
	gdp_gin_t *gin;
	EP_STAT estat;
	char ebuf[60];
	int opt;
	gdp_recno_t recno;
	int nappends = 2;

	while ((opt = getopt(argc, argv, "D:n:")) > 0)
	{
		switch (opt)
		{
		  case 'D':
			ep_dbg_set(optarg);
			break;

		  case 'n':
			nappends = atoi(optarg);
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();
	char *log_xname = argv[0];
	char *svr_xname = argv[1];
	argc -= 2;
	argv += 2;

	ep_app_info("Forwarding append to log %s on server %s",
			log_xname, svr_xname);

	// initialize GDP connection
	estat = gdp_init(NULL);
	if (EP_STAT_ISOK(estat))
		ep_app_info("gdp_init: OK");
	else
		ep_app_fatal("gdp_init: %s", ep_stat_tostr(estat, ebuf, sizeof ebuf));

	// let threads settle (avoid interleaved debug output)
	ep_time_nanosleep(INT64_C(100000000));

	// parse the name of the log to be appended to
	gdp_name_t gdpname;
	estat = gdp_parse_name(log_xname, gdpname);
	if (EP_STAT_ISOK(estat))
		ep_app_info("gdp_parse_name(%s): OK", log_xname);
	else
		ep_app_fatal("gdp_parse_name(%s): %s", log_xname,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));

	// parse the name of the server to receive the append
	gdp_name_t svrname;
	estat = gdp_parse_name(svr_xname, svrname);
	if (EP_STAT_ISOK(estat))
		ep_app_info("gdp_parse_name(%s): OK", svr_xname);
	else
		ep_app_fatal("gdp_parse_name(%s): %s", svr_xname,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));

	// open the log (note: doesn't use svrname)
	estat = gdp_gin_open(gdpname, GDP_MODE_RA, NULL, &gin);
	if (EP_STAT_ISOK(estat))
		ep_app_info("gdp_gin_open: OK");
	else
		ep_app_fatal("gdp_gin_open: %s",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));

	// create datum(s) and send them to the explicit server
	recno = gdp_gin_getnrecs(gin);
	int nresults = 0;
	while (nresults < nappends)
	{
		gdp_datum_t *datum = create_datum(++recno);
		estat = do_fwd_append(gin, datum, svrname, NULL);
		nresults++;
		if (EP_STAT_ISOK(estat))
			ep_app_info("do_fwd_append (%d): OK", nresults);
		else
			ep_app_fatal("do_fwd_append (%d): %s", nresults,
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	// this sleep will allow multiple results to appear before we start reading
	if (ep_dbg_test(Dbg, 100))
		ep_time_nanosleep(500000000);	//DEBUG: one half second

	// collect results
	ep_app_info("waiting for status events, nresults = %d", nresults);
	gdp_event_t *gev;
	while ((gev = gdp_event_next(gin, NULL)) != NULL)
	{
		gdp_event_print(gev, stdout, 3);
		if (gdp_event_gettype(gev) == GDP_EVENT_DONE)
			break;
		if (gdp_event_gettype(gev) == GDP_EVENT_CREATED &&
				--nresults <= 0)
			break;
		gdp_event_free(gev);
	}
	exit(EX_OK);
}
