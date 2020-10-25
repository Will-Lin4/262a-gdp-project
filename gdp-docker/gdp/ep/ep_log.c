/* vim: set ai sw=8 sts=8 ts=8 : */

/***********************************************************************
**  ----- BEGIN LICENSE BLOCK -----
**	LIBEP: Enhanced Portability Library (Reduced Edition)
**
**	Copyright (c) 2008-2019, Eric P. Allman.  All rights reserved.
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
**  ----- END LICENSE BLOCK -----
***********************************************************************/

/*
**  Message logging.
*/

#include <ep/ep.h>
#include <ep/ep_app.h>
#include <ep/ep_log.h>
#include <ep/ep_stat.h>
#include <ep/ep_string.h>
#include <ep/ep_syslog.h>
#include <ep/ep_time.h>

#include <inttypes.h>
#include <time.h>
#include <sys/cdefs.h>
#include <sys/time.h>

struct logfunc
{
	struct logfunc	*next;		// next in chain
	EP_LOG_FUNC	*func;		// function to call
	void		*ctx;		// external context
	unsigned int	minsev;		// minimum severity to log
};

static const char	*LogTag = NULL;
static int		LogFac = -1;
static struct logfunc	*LogFuncList = NULL;
static bool		LogInitialized = false;


void
ep_log_file(void *_fp,
	EP_STAT estat,
	const char *fmt,
	va_list ap)
{
	char tbuf[40];
	struct tm *tm;
	time_t tvsec;
	EP_TIME_SPEC tv;
	FILE *fp = (FILE *) _fp;

	flockfile(fp);
	if (fp == stderr || fp == stdout)
		fprintf(fp, "%s", EpVid->vidfgcyan);
	ep_time_now(&tv);
	tvsec = tv.tv_sec;		//XXX may overflow if time_t is 32 bits!
	if ((tm = localtime(&tvsec)) == NULL)
		snprintf(tbuf, sizeof tbuf, "%"PRIu64".%06lu",
				tv.tv_sec, tv.tv_nsec / 1000L);
	else
	{
		char lbuf[40];

		snprintf(lbuf, sizeof lbuf, "%%Y-%%m-%%d %%H:%%M:%%S.%06lu %%z",
				tv.tv_nsec / 1000L);
		strftime(tbuf, sizeof tbuf, lbuf, tm);
	}

	fprintf(fp, "%s %s: ", tbuf, LogTag);
	vfprintf(fp, fmt, ap);
	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[100];

		ep_stat_tostr(estat, ebuf, sizeof ebuf);
		fprintf(fp, ": %s", ebuf);
	}
	fprintf(fp, "\n");
	if (fp == stderr || fp == stdout)
		fprintf(fp, "%s", EpVid->vidnorm);
	fprintf(fp, "\n");
	funlockfile(fp);
}


static void
ep_log_syslog(void *unused, EP_STAT estat, const char *fmt, va_list ap)
{
	char ebuf[100];
	char mbuf[500];
	int sev = EP_STAT_SEVERITY(estat);
	int logsev;
	static bool inited = false;

	// initialize log if necessary
	if (!inited)
	{
		openlog(LogTag, LOG_PID, LogFac);
		inited = true;
	}

	// map estat severity to syslog priority
	switch (sev)
	{
	  case EP_STAT_SEV_OK:
		logsev = LOG_INFO;
		break;

	  case EP_STAT_SEV_WARN:
		logsev = LOG_WARNING;
		break;

	  case EP_STAT_SEV_ERROR:
		logsev = LOG_ERR;
		break;

	  case EP_STAT_SEV_SEVERE:
		logsev = LOG_CRIT;
		break;

	  case EP_STAT_SEV_ABORT:
		logsev = LOG_ALERT;
		break;

	  default:
		// %%% for lack of anything better
		logsev = LOG_ERR;
		break;

	}

	ep_stat_tostr(estat, ebuf, sizeof ebuf);
	vsnprintf(mbuf, sizeof mbuf, fmt, ap);
	syslog(logsev, "%s: %s", mbuf, ebuf);
}


void
ep_log_init(const char *tag,	// NULL => use program name
	int logfac,		// -1 => don't use syslog
	FILE *logfile)		// NULL => don't print to open file
{

	if (tag == NULL)
		tag = ep_app_getprogname();
	LogTag = tag;

	if (logfac >= 0)
		ep_log_addmethod(&ep_log_syslog, NULL, EP_STAT_SEV_OK);
	LogFac = logfac;

	if (logfile != NULL)
		ep_log_addmethod(&ep_log_file, logfile, EP_STAT_SEV_OK);

	LogInitialized = true;
}


void
ep_log_addmethod(EP_LOG_FUNC *func, void *ctx, int minsev)
{
	struct logfunc *lf;
	struct logfunc **lfh;

	for (lfh = &LogFuncList; (lf = *lfh) != NULL; lfh = &lf->next)
		continue;
	*lfh = lf = (struct logfunc *) ep_mem_zalloc(sizeof *lf);
	lf->func = func;
	lf->ctx = ctx;
	lf->minsev = minsev;
}

void
ep_logv(EP_STAT estat, const char *fmt, va_list _ap)
{
	struct logfunc *lf;

	if (!LogInitialized)
	{
		const char *p;
		int logfac = -1;
		FILE *logfp = NULL;

		p = ep_adm_getstrparam("libep.log.facility", "user");
		if (p != NULL)
			logfac = ep_syslog_fac_from_name(p);

		p = ep_adm_getstrparam("libep.log.fileout", "stderr");
		if (strcasecmp(p, "stderr") == 0)
			logfp = stderr;
		else if (strcmp(p, "stdout") == 0)
			logfp = stdout;

		ep_log_init(NULL, logfac, logfp);
	}

	// call other log functions
	for (lf = LogFuncList; lf != NULL; lf = lf->next)
	{
		va_list ap;

		if (EP_STAT_SEVERITY(estat) < lf->minsev)
			continue;
		va_copy(ap, _ap);
		(*lf->func)(lf->ctx, estat, fmt, ap);
	}
}


void
ep_log(EP_STAT estat, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	ep_logv(estat, fmt, ap);
	va_end(ap);
}
