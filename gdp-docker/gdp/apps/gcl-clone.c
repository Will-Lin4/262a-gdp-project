/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  LOG-CLONE --- clone a log --- essentially the same as doing
**		a gcl-create with the same metadata but on a different
**		server.  We can't actually do that because the meta
**		differs (notably creation time).
**
**		Note: it does NOT copy any data.  This program is only
**		intended for research purposes.
**
**		This is a temporary interface.  Ultimately we need a log
**		creation service that does reasonable log placement rather
**		than having to name a specific log server.
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

#include <ep/ep.h>
#include <ep/ep_app.h>
#include <ep/ep_crypto.h>
#include <ep/ep_dbg.h>
#include <ep/ep_string.h>
#include <gdp/gdp.h>

#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>

#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <pwd.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <sys/stat.h>


// minimum secure key length
#ifndef GDP_MIN_KEY_LEN
# define GDP_MIN_KEY_LEN		1024
#endif // GDP_MIN_KEY_LEN

static EP_DBG	Dbg = EP_DBG_INIT("gdp.gcl-clone", "Clone log app");

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-D dbgspec] [-G gdpd_addr]\n"
			"\tgcl_name logd_name ...\n"
			"    -D  set debugging flags\n"
			"    -G  IP host to contact for GDP router\n"
			"    gcl_name is the name of the GCL to be cloned\n"
			"    logd_name is the name of the log server(s) to host this log\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}


int
main(int argc, char **argv)
{
	gdp_name_t gcliname;			// internal name of GCL
	const char *gclxname = NULL;	// external name of GCL
	gdp_gcl_t *gcl = NULL;
	gdp_gob_md_t *gmd = NULL;
	int opt;
	EP_STAT estat;
	char *gdpd_addr = NULL;
	char ebuf[100];
	bool show_usage = false;
	bool quiet = false;

	// collect command-line arguments
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

	if (show_usage || argc-- < 2)
		usage();

	gclxname = *argv++;

	// initialize the GDP library
	estat = gdp_init(gdpd_addr);
	if (!EP_STAT_ISOK(estat))
	{
		ep_app_error("GDP Initialization failed");
		goto fail0;
	}

	// allow thread to settle to avoid interspersed debug output
	ep_time_nanosleep(INT64_C(100000000));

	// open the existing log to get the metadata
	estat = gdp_parse_name(gclxname, gcliname);
	if (EP_STAT_ISFAIL(estat))
	{
		ep_app_error("Cannot parse source name %s: %s", gclxname,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		exit(EX_NOINPUT);
	}

	estat = gdp_gcl_open(gcliname, GDP_MODE_RO, NULL, &gcl);
	if (!EP_STAT_ISOK(estat))
	{
		ep_app_error("Cannot find source log %s: %s", gclxname,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		exit(EX_NOINPUT);
	}

	// get the metadata from the source log
	estat = gdp_gcl_getmetadata(gcl, &gmd);
	if (!EP_STAT_ISOK(estat))
	{
		ep_app_error("Cannot fetch metadata for log %s: %s", gclxname,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		exit(EX_NOINPUT);
	}

	// we no longer need the source log
	gdp_gcl_close(gcl);

	for (; argc-- > 0; ++argv)
	{
		gdp_name_t logdiname;
		estat = gdp_parse_name(*argv, logdiname);
		if (EP_STAT_ISFAIL(estat))
		{
			ep_app_error("Cannot parse log name %s: %s", *argv++,
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			continue;
		}

		// do the actual create
		estat = gdp_gcl_create(gcliname, logdiname, gmd, &gcl);
		if (!EP_STAT_ISOK(estat))
		{
			ep_app_error("Cannot create clone on %s: %s", *argv++,
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			continue;
		}

		// we can now close the new log as well
		gdp_gcl_close(gcl);
	}

	// free metadata, if set
	if (gmd != NULL)
		gdp_gob_md_free(gmd);

fail0:
	// OK status can have values; hide that from the user
	int xstat = 0;
	if (EP_STAT_ISOK(estat))
		estat = EP_STAT_OK;
	else
		xstat = EX_CANTCREAT;
	if (!quiet)
		ep_app_message(estat, exiting with status");
	exit(xstat);
}
