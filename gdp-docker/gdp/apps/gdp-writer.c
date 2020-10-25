/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  GDP-WRITER --- writes records to a log
**
**		This reads the records one line at a time from standard input
**		and assumes they are text, but there is no text requirement
**		implied by the GDP.
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
#include <ep/ep_dbg.h>
#include <ep/ep_hexdump.h>
#include <ep/ep_string.h>
#include <gdp/gdp.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <sysexits.h>
#include <sys/stat.h>

bool	AsyncIo = false;		// use asynchronous I/O
bool	Quiet = false;			// be silent (no chatty messages)
bool	Hexdump = false;		// echo input in hex instead of ASCII
bool	KeepGoing = false;		// keep going on append errors

static EP_DBG	Dbg = EP_DBG_INIT("gdp-writer", "gdp-writer");

/*
**  DO_LOG --- log a timestamp (for performance checking).
*/

FILE	*LogFile;

void
do_log(const char *tag)
{
	struct timeval tv;

	if (LogFile == NULL)
		return;
	gettimeofday(&tv, NULL);
	fprintf(LogFile, "%s %ld.%06ld\n", tag, tv.tv_sec, (long) tv.tv_usec);
}

#define LOG(tag)	{ if (LogFile != NULL) do_log(tag); }


static const char	*EventTypes[] =
{
	"Free (internal use)",
	"Data",
	"End of Subscription",
	"Shutdown",
	"Asynchronous Status",
};

void
showstat(gdp_event_t *gev)
{
	unsigned int evtype = gdp_event_gettype(gev);
	EP_STAT estat = gdp_event_getstat(gev);
	gdp_datum_t *d = gdp_event_getdatum(gev);
	char ebuf[100];
	char tbuf[20];
	const char *evname;

	if (evtype >= sizeof EventTypes / sizeof EventTypes[0])
	{
		snprintf(tbuf, sizeof tbuf, "%u", evtype);
		evname = tbuf;
	}
	else
	{
		evname = EventTypes[evtype];
	}

	printf("Asynchronous event type %s:\n"
			"\trecno %" PRIgdp_recno ", stat %s\n",
			evname,
			gdp_datum_getrecno(d),
			ep_stat_tostr(estat, ebuf, sizeof ebuf));

	gdp_event_free(gev);
}


EP_STAT
write_record(gdp_datum_t *datum, gdp_gin_t *gin)
{
	EP_STAT estat;

	// echo the input for that warm fuzzy feeling
	if (!Quiet)
	{
		gdp_buf_t *dbuf = gdp_datum_getbuf(datum);
		int l = gdp_buf_getlength(dbuf);
		unsigned char *buf = gdp_buf_getptr(dbuf, l);

		if (!Hexdump)
			fprintf(stdout, "Got input %s%.*s%s\n",
					EpChar->lquote, l, buf, EpChar->rquote);
		else
			ep_hexdump(buf, l, stdout, EP_HEXDUMP_ASCII, 0);
	}

	if (ep_dbg_test(Dbg, 60))
		gdp_datum_print(datum, ep_dbg_getfile(), GDP_DATUM_PRDEBUG);

	// then send the buffer to the GDP
	LOG("W");
	if (AsyncIo)
	{
		estat = gdp_gin_append_async(gin, 1, &datum, NULL, showstat, NULL);
		EP_STAT_CHECK(estat, return estat);

		// return value will be printed asynchronously
	}
	else
	{
		estat = gdp_gin_append(gin, datum, NULL);

		if (EP_STAT_ISOK(estat))
		{
			// print the return value (shows the record number assigned)
			if (!Quiet)
				gdp_datum_print(datum, stdout, GDP_DATUM_PRMETAONLY);
		}
		else if (!Quiet)
		{
			char ebuf[100];
			ep_app_error("Append error: %s",
						ep_stat_tostr(estat, ebuf, sizeof ebuf));
		}
	}
	return estat;
}


EP_STAT
signkey_cb(
		gdp_name_t gname,
		void *udata,
		EP_CRYPTO_KEY **skeyp)
{
	FILE *fp;
	EP_CRYPTO_KEY *skey;
	const char *signing_key_file = (const char *) udata;

	ep_dbg_cprintf(Dbg, 1, "signkey_cb(%s)\n", signing_key_file);

	fp = fopen(signing_key_file, "r");
	if (fp == NULL)
	{
		ep_app_error("cannot open signing key file %s", signing_key_file);
		return ep_stat_from_errno(errno);
	}

	skey = ep_crypto_key_read_fp(fp, signing_key_file,
			EP_CRYPTO_KEYFORM_PEM, EP_CRYPTO_F_SECRET);
	if (skey == NULL)
	{
		ep_app_error("cannot read signing key file %s", signing_key_file);
		return ep_stat_from_errno(errno);
	}

	*skeyp = skey;
	return EP_STAT_OK;
}


void
usage(void)
{
	fprintf(stderr,
			"Usage: %s [-1] [-a] [-D dbgspec] [-G router_addr] [-K key_file]\n"
			"\t[-L log_file] [-q] [-S] log_name\n"
			"    -1  write all input as one record\n"
			"    -a  use asynchronous I/O\n"
			"    -D  set debugging flags\n"
			"    -G  IP host to contact for gdp_router\n"
			"    -i  ignore append errors\n"
			"    -K  signing key file\n"
			"    -L  set logging file name (for debugging)\n"
			"    -q  run quietly (no non-error output)\n"
			"    -S  continue even if signing key cannot be found\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}


int
main(int argc, char **argv)
{
	gdp_gin_t *gin;
	gdp_name_t gdpiname;
	int opt;
	EP_STAT estat;
	char *gdpd_addr = NULL;
	bool show_usage = false;
	bool one_record = false;
	bool allow_no_signing_key = false;
	char *log_file_name = NULL;
	char *signing_key_file = NULL;
	gdp_open_info_t *info;

	// collect command-line arguments
	while ((opt = getopt(argc, argv, "1aD:G:iK:L:qS")) > 0)
	{
		switch (opt)
		{
		 case '1':
			one_record = true;
			Hexdump = true;
			break;

		 case 'a':
			AsyncIo = true;
			break;

		 case 'D':
			ep_dbg_set(optarg);
			break;

		 case 'G':
			gdpd_addr = optarg;
			break;

		 case 'i':
			KeepGoing = true;
			break;

		 case 'K':
			signing_key_file = optarg;
			break;

		 case 'L':
			log_file_name = optarg;
			break;

		 case 'q':
			Quiet = true;
			break;

		 case 'S':
			allow_no_signing_key = true;
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

	if (log_file_name != NULL)
	{
		// open a log file (for timing measurements)
		LogFile = fopen(log_file_name, "a");
		if (LogFile == NULL)
			ep_app_error("Cannot open log file %s: %s",
					log_file_name, strerror(errno));
		else
			setlinebuf(LogFile);
	}

	// initialize the GDP library
	estat = gdp_init(gdpd_addr);
	if (!EP_STAT_ISOK(estat))
	{
		ep_app_error("GDP Initialization failed");
		goto fail0;
	}

	// allow thread to settle to avoid interspersed debug output
	ep_time_nanosleep(INT64_C(100000000));

	// set up any open information
	info = gdp_open_info_new();

	if (signing_key_file != NULL)
	{
		gdp_open_info_set_signkey_cb(info, signkey_cb, signing_key_file);

#if 0	// old code: keep as an example of gdp_open_info_set_signing_key
		FILE *fp;
		EP_CRYPTO_KEY *skey;

		fp = fopen(signing_key_file, "r");
		if (fp == NULL)
		{
			ep_app_error("cannot open signing key file %s", signing_key_file);
			goto fail1;
		}

		skey = ep_crypto_key_read_fp(fp, signing_key_file,
				EP_CRYPTO_KEYFORM_PEM, EP_CRYPTO_F_SECRET);
		if (skey == NULL)
		{
			ep_app_error("cannot read signing key file %s", signing_key_file);
			goto fail1;
		}

		estat = gdp_open_info_set_signing_key(info, skey);
		EP_STAT_CHECK(estat, goto fail1);
#endif
	}

	if (allow_no_signing_key)
		estat = gdp_open_info_set_no_skey_nonfatal(info, true);

	// open a GDP object with the provided name
	estat = gdp_parse_name(argv[0], gdpiname);
	if (EP_STAT_ISFAIL(estat))
		goto fail1;
	else
	{
		estat = gdp_gin_open(gdpiname, GDP_MODE_AO, info, &gin);
		if (EP_STAT_ISFAIL(estat))
			goto fail1;
	}

	if (!Quiet)
	{
		gdp_pname_t pname;

		// dump the internal version of the GDP object to facilitate testing
		printf("GDPname: %s (%" PRIu64 " recs)\n",
				gdp_printable_name(*gdp_gin_getname(gin), pname),
				gdp_gin_getnrecs(gin));

		// OK, ready to go!
		fprintf(stdout, "\nStarting to read input\n");
	}

	{
		// we need a place to buffer the input
		gdp_datum_t *datum = gdp_datum_new();

		if (one_record)
		{
			// read the entire stdin into a single datum
			char buf[8 * 1024];
			int l;

			while ((l = fread(buf, 1, sizeof buf, stdin)) > 0)
				gdp_buf_write(gdp_datum_getbuf(datum), buf, l);

			estat = write_record(datum, gin);
		}
		else
		{
			// write lines into multiple datums
			char buf[200];

			while (fgets(buf, sizeof buf, stdin) != NULL)
			{
				// strip off newlines
				char *p = strchr(buf, '\n');
				if (p != NULL)
					*p++ = '\0';

				// first copy the text buffer into the datum buffer
				gdp_datum_reset(datum);
				gdp_buf_write(gdp_datum_getbuf(datum), buf, strlen(buf));

				// write the record to the log
				estat = write_record(datum, gin);
				if (!EP_STAT_ISOK(estat) && !KeepGoing)
					break;
			}
		}

		// OK, all done.  Free our resources and exit
		gdp_datum_free(datum);
	}

	// give a chance to collect async results
	if (AsyncIo)
		sleep(1);

	// tell the GDP that we are done
	gdp_gin_close(gin);

fail1:
	if (info != NULL)
		gdp_open_info_free(info);

fail0:
	;			// avoid compiler error
	int exitstat;

	if (EP_STAT_ISOK(estat))
		exitstat = EX_OK;
	else if (EP_STAT_IS_SAME(estat, GDP_STAT_NAK_NOROUTE))
		exitstat = EX_CANTCREAT;
	else if (EP_STAT_ISABORT(estat))
		exitstat = EX_SOFTWARE;
	else
		exitstat = EX_UNAVAILABLE;

	// OK status can have values; hide that from the user
	if (EP_STAT_ISOK(estat))
		estat = EP_STAT_OK;
	if (!EP_STAT_ISOK(estat))
		ep_app_message(estat, "exiting with status");
	else if (!Quiet)
		fprintf(stderr, "Exiting with status OK\n");
	return exitstat;
}
