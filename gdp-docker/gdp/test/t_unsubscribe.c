/* vim: set ai sw=4 sts=4 ts=4 : */

/*  To compile:
cc -I. t_unsubscribe.c -Lep -Lgdp -lgdp -lep -levent -levent_pthreads -pthread -lcrypto -lavahi-client -lavahi-common
*/

#include "t_common_support.h"

#include <getopt.h>

//static EP_DBG	Dbg = EP_DBG_INIT("t_unsubscribe", "Unsubscribe test");


int
main(int argc, char **argv)
{
	gdp_gin_t *gin;
	gdp_name_t gobname;
	gdp_datum_t *d;
	EP_STAT estat;
	int opt;
	char *gobxname = "x00";
	bool test_wildcard = false;
	int pausesec = 1;

	while ((opt = getopt(argc, argv, "D:p:w")) > 0)
	{
		switch (opt)
		{
		  case 'D':
			ep_dbg_set(optarg);
			break;

		  case 'p':
			pausesec = atoi(optarg);
			break;

		  case 'w':
			test_wildcard = true;
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		gobxname = argv[0];

	estat = gdp_init(NULL);
	test_message(estat, "gdp_init");

	ep_time_nanosleep(100 MILLISECONDS);

	estat = gdp_parse_name(gobxname, gobname);
	test_message(estat, "gdp_parse_name");

	estat = gdp_gin_open(gobname, GDP_MODE_RA, NULL, &gin);
	test_message(estat, "gdp_gin_open");

	estat = gdp_gin_subscribe_by_recno(gin, 0, 0, NULL, print_event, (void *) 1);
	test_message(estat, "gdp_ubscribe");
	ep_app_info("You should see subscription results");

	d = gdp_datum_new();
	gdp_buf_printf(gdp_datum_getbuf(d), "one");
	estat = gdp_gin_append(gin, d, NULL);
	test_message(estat, "gdp_gin_append1");
	gdp_buf_reset(gdp_datum_getbuf(d));
	ep_app_info("sleeping1");
	ep_time_nanosleep(pausesec SECONDS);


	// use different cbarg: subscription should persist
	estat = gdp_gin_unsubscribe(gin, NULL, (void *) 2);
	test_message(estat, "gdp_gin_unsubscribe1");

	gdp_buf_printf(gdp_datum_getbuf(d), "two");
	estat = gdp_gin_append(gin, d, NULL);
	test_message(estat, "gdp_gin_append2");
	ep_app_info("sleeping2");
	ep_time_nanosleep(pausesec SECONDS);

	if (test_wildcard)
	{
		// use wildcard cbarg: subscription should go away
		estat = gdp_gin_unsubscribe(gin, NULL, NULL);
		test_message(estat, "gdp_gin_unsubscribe3");
	}
	else
	{
		// use correct cbarg: subscription should go away
		estat = gdp_gin_unsubscribe(gin, NULL, (void *) 1);
		test_message(estat, "gdp_gin_unsubscribe2");
	}

	ep_app_info("Subscription should be gone");

	gdp_buf_printf(gdp_datum_getbuf(d), "three");
	estat = gdp_gin_append(gin, d, NULL);
	test_message(estat, "gdp_gin_append3");
	ep_app_info("sleeping3");
	ep_time_nanosleep(pausesec SECONDS);

	ep_app_info("closing");
	estat = gdp_gin_close(gin);
	test_message(estat, "gdp_gin_close");

	return 0;
}
