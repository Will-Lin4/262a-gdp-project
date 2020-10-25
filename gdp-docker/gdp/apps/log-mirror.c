/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  LOG-MIRROR --- mirror a log
**
**		Cheap and dirty replication.
**
**	----- BEGIN LICENSE BLOCK -----
**	Applications for the Global Data Plane
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

#include <gdp/gdp.h>

#include <ep/ep_app.h>
#include <ep/ep_dbg.h>

#include <sysexits.h>

void
usage(void)
{
	fprintf(stderr,
			"Usage: %s [-D dbg_spec] [-G router_addr] source_log target_log\n"
			"    -D  set debugging flags\n"
			"    -G  IP host to contact for gdp_router\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}


int
main(int argc, char **argv)
{
	int opt;
	bool show_usage = false;
	EP_STAT estat;
	gdp_gin_t *igin, *ogin;
	char *gdpd_addr = NULL;
	const char *lname, *lmode;
	gdp_recno_t nextrecno;
	gdp_name_t gdpiname;
	gdp_event_t *gev;

	while ((opt = getopt(argc, argv, "D:G:")) > 0)
	{
		switch (opt)
		{
		 case 'D':
			ep_dbg_set(optarg);
			break;

		 case 'G':
			gdpd_addr = optarg;
			break;

		 default:
			show_usage = true;
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (show_usage || argc != 2)
		usage();

	// initialize GDP library
	estat = gdp_init(gdpd_addr);
	if (!EP_STAT_ISOK(estat))
	{
		ep_app_error("GDP Initialization failed");
		goto fail0;
	}

	// allow thread to settle to avoid interspersed debug output
	ep_time_nanosleep(INT64_C(100000000));

	// open target GCL (must already exist)
	lname = argv[1];
	lmode = "append";
	gdp_parse_name(argv[1], gdpiname);
	estat = gdp_gin_open(gdpiname, GDP_MODE_AO, NULL, &ogin);
	EP_STAT_CHECK(estat, goto fail1);
	nextrecno = gdp_gin_getnrecs(ogin) + 1;

	// open a source GCL with the provided name
	lname = argv[0];
	lmode = "read";
	gdp_parse_name(argv[0], gdpiname);
	estat = gdp_gin_open(gdpiname, GDP_MODE_RO, NULL, &igin);
	EP_STAT_CHECK(estat, goto fail1);

	// subscribe to input starting from the first recno target does not have
	estat = gdp_gin_subscribe_by_recno(igin, nextrecno, 0, NULL, NULL, NULL);
	EP_STAT_CHECK(estat, goto fail2);

	// copy forever
	while ((gev = gdp_event_next(igin, NULL)) != NULL)
	{
		switch (gdp_event_gettype(gev))
		{
		 case GDP_EVENT_DATA:
			// copy the record to the new log
			estat = gdp_gin_append(ogin, gdp_event_getdatum(gev), NULL);
			EP_STAT_CHECK(estat, goto fail3);
			break;

		 case GDP_EVENT_DONE:
		 case GDP_EVENT_SHUTDOWN:
			ep_app_error("unexpected end of subscription");
			estat = EP_STAT_END_OF_FILE;
			goto fail2;

		 default:
			// ignore
			break;
		}
	}

fail3:
	ep_app_error("append failed");

fail2:
	// close GCLs / release resources
	//XXX someday

	if (false)
	{
fail1:
		ep_app_error("could not open %s for %s", lname, lmode);
	}

fail0:
	ep_app_message(estat, "exiting with status");
	return EP_STAT_ISOK(estat) ? EX_OK : EX_UNAVAILABLE;
}
