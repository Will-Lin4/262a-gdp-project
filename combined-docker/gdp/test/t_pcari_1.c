/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  C version of PCARI "test_rw_gdp.py".
*/

#include <gdp/gdp.h>

#include <ep/ep_app.h>
#include <ep/ep_crypto.h>
#include <ep/ep_dbg.h>
#include <ep/ep_time.h>

#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>

EP_TIME_SPEC		WriteInterval =		{ 1, 0, 0.0 };
bool				Errors =			false;

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-D dbgspec] [-G router-addr]\n"
			"    [-s final-sleep-time] [-w write-interval] log1 log2\n"
			"Defaults:\n"
			"    final-sleep-time = 10s\n"
			"    write-interval = 1s\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}

static void *
writer(void *_)
{
	gdp_gin_t *gin = _;
	static int counter = 0;
	EP_STAT estat;
	gdp_hash_t *prevhash = NULL;
	gdp_datum_t *datum = gdp_datum_new();

	for (;;)
	{
		char data[30];
		snprintf(data, sizeof data, "Hi %d", counter++);
		ep_app_info("writer: %s", data);

		gdp_datum_reset(datum);
		gdp_buf_t *dbuf = gdp_datum_getbuf(datum);
		gdp_buf_printf(dbuf, "%s", data);
		estat = gdp_gin_append(gin, datum, prevhash);
		if (!EP_STAT_ISOK(estat))
			ep_app_message(estat, "writer append");
		ep_time_sleep(&WriteInterval);
	}
	return NULL;
}

static void *
reader(void *_)
{
	gdp_gin_t *gin = _;
	EP_STAT estat;

	estat = gdp_gin_subscribe_by_recno(gin, 0, 0, NULL, NULL, NULL);
	if (!EP_STAT_ISOK(estat))
		ep_app_message(estat, "reader subscribe");
	for (;;)
	{
		gdp_event_t *gev = gdp_event_next(gin, NULL);
		gdp_event_print(gev, stdout);
		gdp_event_free(gev);
	}
	return NULL;
}

static void
stat_check(EP_STAT estat, const char *where)
{
	if (EP_STAT_ISOK(estat))
		return;
	ep_app_message(estat, "%s", where);
	Errors = true;
}

int main(int argc, char **argv)
{
	EP_STAT estat;
	char *router_addr = NULL;
	int opt;
	EP_TIME_SPEC final_sleep_time = { 10, 0, 0.0 };

	while ((opt = getopt(argc, argv, "D:G:s:w:")) > 0)
	{
		switch (opt)
		{
		  case 'D':
			ep_dbg_set(optarg);
			break;

		  case 'G':
			router_addr = optarg;
			break;

		  case 's':
			ep_time_parse_interval(optarg, 's', &final_sleep_time);
			break;

		  case 'w':
			ep_time_parse_interval(optarg, 's', &WriteInterval);
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 2)
		usage();

	// initialize the GDP library
	estat = gdp_init(router_addr);
	if (!EP_STAT_ISOK(estat))
		ep_app_fatal("GDP Initialization failed");

	// open log1 for writing and spawn a writer thread
	ep_app_info("Opening log1w");
	gdp_name_t log1name;
	estat = gdp_parse_name(argv[0], log1name);
	stat_check(estat, "gdp_parse_name(log1)");
	gdp_gin_t *log1w;
	estat = gdp_gin_open(log1name, GDP_MODE_AO, NULL, &log1w);
	stat_check(estat, "gdp_gin_open(log1w)");
	if (Errors)
		exit(EX_UNAVAILABLE);

	ep_app_info("Spawning thread w1");
	EP_THR thr_w1;
	ep_thr_spawn(&thr_w1, &writer, log1w);

	// open log1 and log2 for reading
	ep_app_info("Opening log1r");
	gdp_gin_t *log1r;
	estat = gdp_gin_open(log1name, GDP_MODE_RO, NULL, &log1r);
	stat_check(estat, "gdp_gin_open(log1r)");
	ep_app_info("Opening log2r");
	gdp_name_t log2name;
	estat = gdp_parse_name(argv[1], log2name);
	stat_check(estat, "gdp_parse_name(log2)");
	gdp_gin_t *log2r;
	estat = gdp_gin_open(log2name, GDP_MODE_RO, NULL, &log2r);
	stat_check(estat, "gdp_gin_open(log2r)");
	if (Errors)
		exit(EX_UNAVAILABLE);

	// spawn two subscriber threads, one on each log
	ep_app_info("Spawning thread r2");
	EP_THR thr_r2;
	ep_thr_spawn(&thr_r2, &reader, log2r);
	ep_app_info("Spawning thread r1");
	EP_THR thr_r1;
	ep_thr_spawn(&thr_r1, &reader, log1r);

	// run threads for a while to collect results
	ep_app_info("Waiting for status");
	ep_time_sleep(&final_sleep_time);

	// exiting will kill off the threads
	ep_app_info("Exiting");
	exit(EX_OK);
}
