/* vim: set ai sw=4 sts=4 ts=4 : */

#include <gdp/gdp.h>

#include <ep/ep_app.h>
#include <ep/ep_crypto.h>
#include <ep/ep_dbg.h>

#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-D dbgspec] [-l datalen] [-n numrecs]\n"
			"  logname\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}

int main(int argc, char **argv)
{
	EP_STAT estat;
	char *router_addr = NULL;
	gdp_gin_t *handle;
	gdp_name_t name;
	gdp_datum_t *datum;
	gdp_buf_t *buf;
	gdp_event_t *e;
	int i, numrecs=100;
	int dlen = 500;
	uint8_t *random_data;
	int opt;

	while ((opt = getopt(argc, argv, "D:G:l:n:")) > 0)
	{
		switch (opt)
		{
		  case 'D':
			ep_dbg_set(optarg);
			break;

		  case 'G':
			router_addr = optarg;
			break;

		  case 'l':
			dlen = atoi(optarg);
			break;

		  case 'n':
			numrecs = atoi(optarg);
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();
	if (dlen <= 0)
		ep_app_fatal("must write some data (-l must be > 0, is %d)", dlen);

	// initialize the GDP library
	estat = gdp_init(router_addr);
	if (!EP_STAT_ISOK(estat))
		ep_app_fatal("GDP Initialization failed");

	random_data = ep_mem_malloc(dlen);

	gdp_parse_name(argv[0], name);
	gdp_gin_open(name, GDP_MODE_RA, NULL, &handle);

	datum = gdp_datum_new();
	buf = gdp_datum_getbuf(datum);
	for (i = 0; i < numrecs; i++)
	{
		// be sure we're really writing random data
		ep_crypto_random_buf(random_data, dlen);
		gdp_buf_reset(buf);
		gdp_buf_write(buf, random_data, sizeof random_data);
		gdp_gin_append_async(handle, 1, &datum, NULL, NULL, NULL);
	}

	printf("Done appending. Now reading\n");

	for (i = 0; i < numrecs; i++)
	{
		e = gdp_event_next(NULL, NULL);
		if (gdp_event_gettype(e) != GDP_EVENT_SUCCESS)
			gdp_event_print(e, stdout);
		gdp_event_free(e);
	}

	printf("Done fetching status.\n");
	return 0;
}
