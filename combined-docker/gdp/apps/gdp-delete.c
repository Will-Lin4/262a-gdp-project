/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  GDP-DELETE --- delete a GCL
**
**		This is only for debugging.  The gdp_gin_delete call is intended
**		for use by an expiration service.
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
			"Usage: %s [-D dbg_spec] [-G router_addr] target_log\n"
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
	gdp_gin_t *gin;
	char *gdpd_addr = NULL;
	const char *lname;
	gdp_name_t gdpiname;
	const char *phase;
	int exitstat = EX_OK;

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

	if (show_usage || argc != 1)
		usage();

	// initialize GDP library
	estat = gdp_init(gdpd_addr);
	if (!EP_STAT_ISOK(estat))
	{
		exitstat = EX_UNAVAILABLE;
		ep_app_error("GDP Initialization failed");
		goto fail0;
	}

	// allow thread to settle to avoid interspersed debug output
	ep_time_nanosleep(INT64_C(100000000));

	// open target GCL (must already exist)
	lname = argv[0];
	gdp_parse_name(lname, gdpiname);
	phase = "open";
	estat = gdp_gin_open(gdpiname, GDP_MODE_AO, NULL, &gin);
	if (!EP_STAT_ISOK(estat))
	{
		exitstat = EX_NOINPUT;
		goto fail1;
	}

	// delete and close log
	phase = "delete";
	estat = gdp_gin_delete(gin);

	if (!EP_STAT_ISOK(estat))
	{
		exitstat = EX_NOPERM;
fail1:
		ep_app_error("could not %s %s", phase, lname);
	}

fail0:
	ep_app_message(estat, "exiting with status");
	if (exitstat == EX_OK && !EP_STAT_ISOK(estat))
		exitstat = EX_UNAVAILABLE;
	return exitstat;
}
