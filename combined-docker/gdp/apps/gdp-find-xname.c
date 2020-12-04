/* vim: set ai sw=4 sts=4 ts=4 : */


/*
**  GDP-FIND-XNAME --- find the human name of a log
**
**		... as long as it is listed in the metadata.
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

/*
**  Given an internal (base64-encoded) log name, check the metadata
**  and print the external (human) name.
*/

void
usage(void)
{
	fprintf(stderr,
			"Usage: %s [-D dbg_spec] [-G router_addr] log-name\n"
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
	char buf[200];
	gdp_name_t gdpiname;

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
		ep_app_error("GDP Initialization failed");
		goto fail0;
	}

	// allow thread to settle to avoid interspersed debug output
	ep_time_nanosleep(INT64_C(100000000));

	estat = gdp_parse_name(argv[0], gdpiname);
	if (EP_STAT_ISFAIL(estat))
		ep_app_fatal("Cannot parse log name %s", argv[0]);
	estat = gdp_gin_open(gdpiname, GDP_MODE_RO, NULL, &gin);
	EP_STAT_CHECK(estat, goto fail1);

	gdp_md_t *md;
	estat = gdp_gin_getmetadata(gin, &md);
	EP_STAT_CHECK(estat, goto fail2);

	int indx;
	for (indx = 0; EP_STAT_ISOK(estat); indx++)
	{
		gdp_md_id_t id;
		size_t len;
		const void *data;

		estat = gdp_md_get(md, indx, &id, &len, &data);
		if (EP_STAT_ISOK(estat) && id == GDP_MD_XID)
		{
			// found the name
			printf("%.*s\n", (int) len, (const char *) data);
			break;
		}
	}

	if (!EP_STAT_ISOK(estat))
		printf("No name found\n");

fail2:
	// close logs / release resources
	//XXX someday

	if (false)
	{
fail1:
		ep_app_error("Could not open log %s: %s",
				argv[0], ep_stat_tostr(estat, buf, sizeof buf));
	}

fail0:
	ep_app_message(estat, "exiting with status");
	return EP_STAT_ISOK(estat) ? EX_OK : EX_UNAVAILABLE;
}
