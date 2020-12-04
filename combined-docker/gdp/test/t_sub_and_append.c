/* vim: set ai sw=4 sts=4 ts=4 : */

/*  To compile:
cc -I. t_sub_and_append.c -Lep -Lgdp -lgdp -lep -levent -levent_pthreads -pthread -lcrypto -lavahi-client -lavahi-common
*/

#include "t_common_support.h"

#include <getopt.h>

//static EP_DBG	Dbg = EP_DBG_INIT("t_sub_and_append", "Subscribe and Append in one process test");


int
main(int argc, char **argv)
{
	gdp_gin_t *gin;
	gdp_name_t gdpname;
	gdp_datum_t *d;
	EP_STAT estat;
	int opt;
	char *gdpxname = "x00";

	while ((opt = getopt(argc, argv, "D:")) > 0)
	{
		switch (opt)
		{
		  case 'D':
			ep_dbg_set(optarg);
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		gdpxname = argv[0];

	estat = gdp_init(NULL);
	test_message(estat, "gdp_init");

	ep_time_nanosleep(INT64_C(100000000));

	estat = gdp_parse_name(gdpxname, gdpname);
	test_message(estat, "gdp_parse_name");

	estat = gdp_gin_open(gdpname, GDP_MODE_RA, NULL, &gin);
	test_message(estat, "gdp_gin_open");

	estat = gdp_gin_subscribe_by_recno(gin, 0, 0, NULL, print_event, NULL);
	test_message(estat, "gdp_gin_subscribe");

	d = gdp_datum_new();
	gdp_buf_printf(gdp_datum_getbuf(d), "one");
	estat = gdp_gin_append(gin, d, NULL);
	test_message(estat, "gdp_gin_append1");
	gdp_buf_reset(gdp_datum_getbuf(d));

	gdp_buf_printf(gdp_datum_getbuf(d), "two");
	estat = gdp_gin_append(gin, d, NULL);
	test_message(estat, "gdp_gin_append2");

	// hang for 5 seconds waiting for events
	ep_app_info("sleeping");
	sleep(5);

	ep_app_info("closing");
	estat = gdp_gin_close(gin);
	test_message(estat, "gdp_gin_close");

	return 0;
}
