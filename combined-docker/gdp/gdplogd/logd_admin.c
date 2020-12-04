/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	----- BEGIN LICENSE BLOCK -----
**	GDPLOGD: Log Daemon for the Global Data Plane
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


/*
**  This file has support for GDP log server administration
*/

#include "logd_admin.h"
#include "logd_sqlite.h"

#include <gdp/gdp.h>
#include <gdp/gdp_priv.h>

#include <ep/ep_dbg.h>
#include <ep/ep_xlate.h>

#include <stdarg.h>
#include <string.h>
#include <syslog.h>

#include <sys/errno.h>
#include <sys/stat.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdplogd.admin", "GDP administrative logging");

/*
**	ADMIN_POST --- post statistics for system visualization
**
**		This always produces parsable output.
*/


static const char	*AdminStatsFileName = NULL;
static ino_t		AdminStatsIno = -1;
static FILE			*AdminStatsFp;
static uint32_t		AdminRunMask = 0xffffffff;
static const char	*AdminPrefix = "";
static bool			AdminInitialized = false;
static EP_THR_MUTEX	AdminProbeMutex		EP_THR_MUTEX_INITIALIZER;

// a prefix to indicate that this is an admin message (stdout & stderr only)
#define INDICATOR		">#<"


/*
**  (Re-)Open an output file
*/

static void
reopen(const char *fname, FILE **fpp, ino_t *inop)
{
	FILE *fp;
	struct stat st;

	// open the new version of the file
	fp = fopen(fname, "a");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open output file \"%s\": %s\n",
				fname, strerror(errno));
		return;
	}
	setlinebuf(fp);

	// close the old file
	if (*fpp != NULL)
		fclose(*fpp);

	// save info for new version of the file
	*inop = -1;
	if (fstat(fileno(fp), &st) == 0)
		*inop = st.st_ino;
	*fpp = fp;
}


/*
**  ADMIN_INIT
*/

extern void			admin_probe(int, short, void *);

static void
admin_init(void)
{
	// figure out where to put the output
	const char *logdest =
			ep_adm_getstrparam("swarm.gdplogd.admin.output", NULL);
	int fd;
	char *endp;

	// first, avoid calling this multiple times
	AdminInitialized = true;

	if (logdest == NULL || logdest[0] == '\0' ||
			strcasecmp(logdest, "none") == 0)
	{
		AdminRunMask = 0;
		return;
	}

	fd = strtol(logdest, &endp, 10);
	if (fd > 0 && *endp == '\0')
	{
		// pure integer; use as fd
		AdminStatsFp = fdopen(fd, "a");
	}
	else if (strcmp(logdest, "stdout") == 0)
	{
		AdminStatsFp = stdout;
		AdminPrefix = INDICATOR;
	}
	else if (strcmp(logdest, "stderr") == 0)
	{
		AdminStatsFp = stderr;
		AdminPrefix = INDICATOR;
	}
	else if (strcmp(logdest, "syslog") == 0)
	{
		AdminStatsFp = ep_fopen_syslog(LOG_INFO);
	}
	else
	{
		reopen(logdest, &AdminStatsFp, &AdminStatsIno);
		AdminStatsFileName = logdest;
	}

	// if we couldn't open the file, skip this output
	if (AdminStatsFp == NULL)
	{
		AdminRunMask = 0;
		AdminStatsFp = stderr;			// anything non-NULL
		return;
	}

	// arrange to probe logs periodically
	long stats_intvl = ep_adm_getlongparam("swarm.gdplogd.admin.probeintvl", 0);

	if (stats_intvl > 0)
	{
		struct timeval tv = { stats_intvl, 0 };
		struct event *evtimer = event_new(_GdpIoEventBase, -1,
									EV_PERSIST, &admin_probe, NULL);
		event_add(evtimer, &tv);
	}
}


/*
**  _ADMIN_POST_STATS, _ADMIN_POST_STATSV --- post statistics
**
**		Parameters:
**			mask --- to allow classification
**			msgid --- to label the statistic
**			..., av --- a list of name, value pairs.  The name may
**				be NULL to output an unnamed (positional) value.
**				A value of NULL terminates the list.
*/

void
admin_post_stats(
		uint32_t mask,
		const char *msgid,
		...)
{
	va_list av;

	va_start(av, msgid);
	admin_post_statsv(mask, msgid, av);
	va_end(av);
}

void
admin_post_statsv(
		uint32_t mask,
		const char *msgid,
		va_list av)
{
	int argno = 0;
	bool firstparam = true;
	int xlatemode = EP_XLATE_PLUS | EP_XLATE_NPRINT;
	static const char *forbidchars = NULL;
	FILE *fp;

	if (!AdminInitialized)
		admin_init();
	if ((mask & AdminRunMask) == 0)
		return;
	if (forbidchars == NULL)
		forbidchars = ep_adm_getstrparam("gdplogd.admin.forbidchars", "=;");

	// check to see if we need to re-open the output
	if (AdminStatsFileName != NULL)
	{
		struct stat st;

		if (stat(AdminStatsFileName, &st) != 0 || st.st_ino != AdminStatsIno)
		{
			reopen(AdminStatsFileName, &AdminStatsFp, &AdminStatsIno);
		}
	}
	fp = AdminStatsFp;

	// make sure this message is atomic
	flockfile(fp);

	// output an initial indicator to make this easy to find
	fprintf(fp, "%s", AdminPrefix);

	// add a timestamp and the message id
	{
		EP_TIME_SPEC now;
		char tbuf[60];

		ep_time_now(&now);
		ep_time_format(&now, tbuf, sizeof tbuf, EP_TIME_FMT_NOFUZZ);
		fprintf(fp, "%s ", tbuf);
	}

	(void) ep_xlate_out(msgid,
			strlen(msgid),
			fp,
			forbidchars,
			xlatemode);

	// scan the arguments
	for (;;)
	{
		const char *apn;
		const char *apv;

		argno++;
		apn = va_arg(av, const char *);
		if ((apv = va_arg(av, const char *)) == NULL)
			break;

		if (!firstparam)
			putc(';', fp);
		putc(' ', fp);
		firstparam = false;

		if (apn != NULL)
		{
			(void) ep_xlate_out(apn,
					strlen(apn),
					fp,
					forbidchars,
					xlatemode);

			putc('=', fp);
		}

		(void) ep_xlate_out(apv,
				strlen(apv),
				fp,
				forbidchars,
				xlatemode);
	}
	putc('\n', fp);
	fflush(fp);
	funlockfile(fp);
}


/*
**  Periodic probe of system status (should probably be in thread)
*/

static EP_STAT
post_one_log(gdp_name_t gdpname, void *ctx)
{
	gdp_gob_t *gob;
	gdp_pname_t gdppname;
	EP_STAT estat;

	gdp_printable_name(gdpname, gdppname);

	estat = _gdp_gob_cache_get(gdpname, GGCF_NOCREATE | GGCF_PEEK, &gob);
	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[100];

		ep_dbg_cprintf(Dbg, 1, "post_one_log: _gdp_gob_cache_get: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		return estat;
	}
	if (gob != NULL)
	{
		if (gob->x->physimpl->getstats != NULL)
		{
			char nrecsbuf[40];
			char logsizebuf[40];
			struct gob_phys_stats stats;

			gob->x->physimpl->getstats(gob, &stats);
			snprintf(nrecsbuf, sizeof nrecsbuf, "%" PRIgdp_recno, stats.nrecs);
			snprintf(logsizebuf, sizeof logsizebuf, "%" PRId64, stats.size);
			admin_post_stats(ADMIN_LOG_SNAPSHOT, "log-snapshot",
					"name", gdppname,
					"in-cache", "true",
					"nrecs", nrecsbuf,
					"size", logsizebuf,
					NULL, NULL);
		}
		else
		{
			admin_post_stats(ADMIN_LOG_SNAPSHOT, "log-snapshot",
					"name", gdppname,
					"in-cache", "true",
					NULL, NULL);
		}

		_gdp_gob_unlock(gob);
	}
	else
	{
		admin_post_stats(ADMIN_LOG_SNAPSHOT, "log-snapshot",
				"name", gdppname,
				"in-cache", "false",
				NULL, NULL);
	}
	return EP_STAT_OK;
}


static void
admin_probe_thread(void *ctx)
{
	if (ep_thr_mutex_trylock(&AdminProbeMutex) != 0)
	{
		ep_dbg_cprintf(Dbg, 7, "admin_probe_thread: locked\n");
		return;
	}
	GdpSqliteImpl.foreach(post_one_log, ctx);
	ep_thr_mutex_unlock(&AdminProbeMutex);
}

void
admin_probe(int fd, short what, void *ctx)
{
	ep_thr_pool_run(admin_probe_thread, ctx);
}
