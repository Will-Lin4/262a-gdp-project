/* vim: set ai sw=4 sts=4 ts=4 : */

/*  To compile:
cc -I. t_multimultiread.c -Lep -Lgdp -lgdp -lep -levent -levent_pthreads -pthread -lcrypto -lavahi-client -lavahi-common
*/

#include "t_common_support.h"

#include <getopt.h>
#include <sysexits.h>

static EP_DBG	Dbg = EP_DBG_INIT("t_multimultiread", "GDP multiple multireader test");



/*
**  DO_MULTIREAD --- subscribe or multiread
**
**		This routine handles calls that return multiple values via the
**		event interface.  They might include subscriptions.
*/

EP_STAT
do_multiread(gdp_gin_t *gin,
		gdp_recno_t firstrec,
		int32_t numrecs,
		void *udata)
{
	EP_STAT estat;
	void (*cbfunc)(gdp_event_t *) = NULL;

	cbfunc = print_event;

	// make the flags more user-friendly
	if (firstrec == 0)
		firstrec = 1;

	// start up a multiread
	estat = gdp_gin_multiread(gin, firstrec, numrecs, cbfunc, udata);

	// check to make sure the subscribe/multiread succeeded; if not, bail
	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[200];

		ep_app_fatal("Cannot multiread:\n\t%s",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	// this sleep will allow multiple results to appear before we start reading
	if (ep_dbg_test(Dbg, 100))
		ep_time_nanosleep(500000000);	//DEBUG: one half second

	return estat;
}


void
usage(void)
{
	fprintf(stderr,
			"Usage: %s [-D dbgspec] [-n maxrecs] [-r nreaders] [log_name]\n"
			"    -D  set debugging flags\n"
			"    -n  set maximum number of records returned\n"
			"    -r  set number of readers (default 2)\n"
			"log_name (default \"x00\") must already exist\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}

int
main(int argc, char **argv)
{
	gdp_gin_t *gin;
	gdp_name_t gdpname;
	EP_STAT estat;
	int opt;
	char *gdpxname = "x00";
	int nreaders = 2;
	int maxrecs = 0;
	bool show_usage = false;

	while ((opt = getopt(argc, argv, "D:n:r:")) > 0)
	{
		switch (opt)
		{
		  case 'D':
			ep_dbg_set(optarg);
			break;

		  case 'n':
			maxrecs = atoi(optarg);
			break;

		  case 'r':
			nreaders = atoi(optarg);
			break;

		  case '?':
			show_usage = true;
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (show_usage || argc > 1)
		usage();

	if (argc > 0)
		gdpxname = argv[0];

	estat = gdp_init(NULL);
	test_message(estat, "gdp_init");

	ep_time_nanosleep(INT64_C(100000000));

	estat = gdp_parse_name(gdpxname, gdpname);
	test_message(estat, "gdp_parse_name");

	estat = gdp_gin_open(gdpname, GDP_MODE_RO, NULL, &gin);
	test_message(estat, "gdp_gin_open");

	long i;
	for (i = 1; i <= nreaders; i++)
	{
		estat = do_multiread(gin, 1, maxrecs, (void *) i);
		test_message(estat, "reader %ld", i);
	}

	// hang for 5 seconds waiting for events
	ep_app_info("sleeping");
	sleep(5);

	return 0;
}
