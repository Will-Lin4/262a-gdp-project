/* vim: set ai sw=8 sts=8 ts=8 :*/

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

#include <ep.h>
#include <ep_app.h>
#include <ep_dbg.h>
#include <ep_string.h>
#include <ep_thr.h>
#include <ep_time.h>
#include <errno.h>
#include <inttypes.h>

static EP_DBG	Dbg = EP_DBG_INIT("libep.time", "Time operations");


/*
**  EP_TIME_ACCURACY --- return the accuracy of the clock
**
**	Some systems may be able to determine this more precisely; we
**	just take it as an administrative parameter.
*/

static float		ClockAccuracy = -1;	// in seconds
static EP_THR_MUTEX	ClockAccuracyMutex	EP_THR_MUTEX_INITIALIZER;

#define ONESECOND	INT64_C(1000000000)	// one second in nanoseconds
#define ONEMINUTE	(60)			// one minute in seconds
#define ONEHOUR		(60 * ONEMINUTE)	// one hour in seconds
#define ONEDAY		(24 * ONEHOUR)		// one day in seconds

float
ep_time_accuracy(void)
{
	ep_thr_mutex_lock(&ClockAccuracyMutex);
	if (ClockAccuracy < 0)
	{
#ifdef CLOCK_REALTIME
		// this assumes that the resolution is at least one second
		struct timespec ts;
		clock_getres(CLOCK_REALTIME, &ts);
		ClockAccuracy = (float) ts.tv_nsec / 1.0e9;
#else
		// default implementation that uses a static parameter
		const char *p = ep_adm_getstrparam("libep.time.accuracy", NULL);

		if (p == NULL)
			ClockAccuracy = 0.0;
		else
			ClockAccuracy = strtof(p, NULL);
#endif
	}
	ep_thr_mutex_unlock(&ClockAccuracyMutex);
	return ClockAccuracy;
}


/*
**  EP_TIME_NOW --- return current time of day
**
**	Ideally this will have resolution as good as a nanosecond,
**	but it can be much less on most hardware.
**
**	Everything gets copied because some platforms *still* use
**	32-bit time_t.  Beware of January 18, 2038!
*/

#undef CLOCK_REALTIME		// issues on Linux; punt for now

EP_STAT
ep_time_now(EP_TIME_SPEC *tv)
{
	EP_STAT estat = EP_STAT_OK;

#ifdef CLOCK_REALTIME
	struct timespec tvs;

	if (clock_gettime(CLOCK_REALTIME, &tvs) < 0)
	{
		estat = ep_stat_from_errno(errno);
		tv->tv_sec = EP_TIME_NOTIME;
	}
	else
	{
		tv->tv_sec = tvs.tv_sec;
		tv->tv_nsec = tvs.tv_nsec;
	}
#else
	struct timeval tvu;

	if (gettimeofday(&tvu, NULL) < 0)
	{
		estat = ep_stat_from_errno(errno);
		tv->tv_sec = EP_TIME_NOTIME;
	}
	else
	{
		tv->tv_sec = tvu.tv_sec;
		tv->tv_nsec = tvu.tv_usec * 1000;
	}
#endif

	tv->tv_accuracy = ep_time_accuracy();

	return estat;
}


/*
**  EP_TIME_ZERO --- zero out a time structure
*/

void
ep_time_zero(EP_TIME_SPEC *tv)
{
	tv->tv_sec = 0;
	tv->tv_nsec = 0;
	tv->tv_accuracy = 0.0;
}


/*
**  EP_TIME_DELTANOW --- return current time of day plus a delta
**
**	This assumes that a negative delta is represented as both
**	a non-positive number of seconds and a non-positive number
**	of nanoseconds; for example, { -5, +500000 } wouldn't make
**	sense.
*/

EP_STAT
ep_time_deltanow(EP_TIME_SPEC *delta, EP_TIME_SPEC *tv)
{
	EP_STAT estat;

	estat = ep_time_now(tv);
	EP_STAT_CHECK(estat, return estat);

	ep_time_add_delta(delta, tv);
	return EP_STAT_OK;
}


/*
**  EP_TIME_ADD_DELTA --- add a delta to a time
*/

void
ep_time_add_delta(EP_TIME_SPEC *delta, EP_TIME_SPEC *tv)
{
	tv->tv_sec += delta->tv_sec;
	tv->tv_nsec += delta->tv_nsec;
	if (tv->tv_nsec > ONESECOND)
	{
		tv->tv_sec++;
		tv->tv_nsec -= ONESECOND;
	}
	else if (tv->tv_nsec < 0)
	{
		tv->tv_sec--;
		tv->tv_nsec += ONESECOND;
	}
}


/*
**  EP_TIME_SUBTRACT --- return difference of two times (producing interval)
**
**	Subtracts a from b and leaves the result in b.
*/

void
ep_time_subtract(EP_TIME_SPEC *a, EP_TIME_SPEC *b)
{
	b->tv_sec -= a->tv_sec;
	b->tv_nsec -= a->tv_nsec;
	if (b->tv_sec < 0)
	{
		// a > b; adjust nsec appropriately
		if (b->tv_nsec > 0)
		{
			b->tv_sec += 1;
			b->tv_nsec -= ONESECOND;
		}
	}
	else if (b->tv_nsec < 0)
	{
		b->tv_sec -= 1;
		b->tv_nsec += ONESECOND;
	}
}


/*
**  EP_TIME_DIFF_USEC --- return delta between two times
**
**	Returns b - a as interpreted in microseconds.
*/

int64_t
ep_time_diff_usec(EP_TIME_SPEC *a, EP_TIME_SPEC *b)
{
	return (b->tv_sec * 1000000L + (b->tv_nsec / 1000)) -
		(a->tv_sec * 1000000L + (a->tv_nsec / 1000));
}


/*
**  EP_TIME_BEFORE --- true if A occurred before B
**
**	This doesn't allow for clock precision; it should really have
**	a "maybe" return.
*/

bool
ep_time_before(EP_TIME_SPEC *a, EP_TIME_SPEC *b)
{
	if (a->tv_sec < b->tv_sec)
		return true;
	else if (a->tv_sec > b->tv_sec)
		return false;
	else
		return (a->tv_nsec < b->tv_nsec);
}


/*
**  EP_TIME_TO_NSEC --- convert EP_TIME_SPEC to integer nanoseconds
*/

int64_t
ep_time_to_nsec(EP_TIME_SPEC *tv)
{
	return (tv->tv_sec * ONESECOND) + tv->tv_nsec;
}


/*
**  EP_TIME_FROM_[NU]SEC --- convert nanoseconds/microseconds to a EP_TIME_SPEC
*/

void
ep_time_from_nsec(int64_t nsec, EP_TIME_SPEC *tv)
{
	tv->tv_sec = nsec / ONESECOND;
	tv->tv_nsec = nsec % ONESECOND;
	tv->tv_accuracy = 0;
}

void
ep_time_from_usec(int64_t usec, EP_TIME_SPEC *tv)
{
	tv->tv_sec = usec / 1000000L;
	tv->tv_nsec = (usec % 1000000L) * 1000L;
	tv->tv_accuracy = 0;
}


/*
**  EP_TIME_FROM_SEC --- create an EP_TIME_SPEC from seconds
*/

void
ep_time_from_sec(int64_t sec, EP_TIME_SPEC *tv)
{
	tv->tv_sec = sec;
	tv->tv_nsec = 0;
	tv->tv_accuracy = 0.0;
}


/*
**  EP_TIME_SLEEP --- sleep for a designated interval
*/

EP_STAT
ep_time_sleep(EP_TIME_SPEC *tv)
{
	struct timespec ts;

	ts.tv_sec = tv->tv_sec;
	ts.tv_nsec = tv->tv_nsec;
	if (nanosleep(&ts, NULL) < 0)
		return ep_stat_from_errno(errno);
	return EP_STAT_OK;
}


/*
**  EP_TIME_NANOSLEEP --- sleep for designated number of nanoseconds
*/

EP_STAT
ep_time_nanosleep(int64_t nsec)
{
	struct timespec ts;

	ts.tv_sec = nsec / ONESECOND;
	ts.tv_nsec = nsec % ONESECOND;
	if (nanosleep(&ts, NULL) < 0)
		return ep_stat_from_errno(errno);
	return EP_STAT_OK;
}


/*
**  EP_TIME_FORMAT --- format a time/date for printing
**
**	Uses ISO 8601 format (except for the "fuzz").
**	Always prints in Zulu (GMT); should take a time zone or at least
**		a "EP_TIME_FMT_LOCALTIME" bit.
*/

char *
ep_time_format(const EP_TIME_SPEC *tv, char *tbuf, size_t tbsiz, uint32_t flags)
{
	bool human = EP_UT_BITSET(EP_TIME_FMT_HUMAN, flags);
	bool showfuzz = !EP_UT_BITSET(EP_TIME_FMT_NOFUZZ, flags);
	int sigfigs;
	uint32_t scale;

	if (!EP_TIME_IS_VALID(tv))
	{
		snprintf(tbuf, tbsiz, "%s", human ? "(none)" : "-");
		return tbuf;
	}

	sigfigs = (flags & _EP_TIME_FMT_SIGFIGMASK) >> _EP_TIME_FMT_SIGFIGSHIFT;
	if (sigfigs == 0)
		sigfigs = 3;		// default
	else if (sigfigs > 9)
		sigfigs = 0;

	scale = 1;
	int i;
	for (i = 9; i > sigfigs; i--)
		scale *= 10;

	struct tm tm;
	time_t tvsec;
	char xbuf[40];
	char ybuf[40];

	tvsec = tv->tv_sec;	// may overflow if time_t is 32 bits!
	gmtime_r(&tvsec, &tm);
	strftime(xbuf, sizeof xbuf, "%Y-%m-%dT%H:%M:%S", &tm);
	if (human)
		xbuf[10] = ' ';		// change "T" to " " for humans
	if (tv->tv_accuracy != 0.0 && showfuzz)
	{
		if (human)
		{
			snprintf(ybuf, sizeof ybuf, " %s %#f",
				EpChar->plusminus, tv->tv_accuracy);
		}
		else
		{
			snprintf(ybuf, sizeof ybuf, "/%f", tv->tv_accuracy);
		}
	}
	else
	{
		ybuf[0] = '\0';
	}
	if (sigfigs == 0)
		snprintf(tbuf, tbsiz, "%sZ%s", xbuf, ybuf);
	else
		snprintf(tbuf, tbsiz, "%s.%0*" PRIu32 "Z%s",
				xbuf, sigfigs, tv->tv_nsec / scale, ybuf);
	return tbuf;
}


/*
**  EP_TIME_PRINT --- print a time/date spec to a file
*/

void
ep_time_print(const EP_TIME_SPEC *tv, FILE *fp, uint32_t flags)
{
	char tbuf[100];

	ep_time_format(tv, tbuf, sizeof tbuf, flags);
	fprintf(fp, "%s", tbuf);
}


/*
**  EP_TIME_FORMAT_INTERVAL --- print the interval between two times
**
**	If the start time is not set, assume 0.
*/

void
ep_time_format_interval(
		EP_TIME_SPEC *start,		// starting time
		EP_TIME_SPEC *end,		// ending time
		char *buf,			// output buffer
		size_t bufsize)			// size of output buffer
{
	EP_TIME_SPEC interval;
	bool force = false;
	EP_TIME_SPEC zero = { 0, 0 };

	if (start == NULL)
		start = &zero;

	interval.tv_sec = end->tv_sec - start->tv_sec;
	// ASSERT(interval.tv_sec >= 0)
	interval.tv_nsec = end->tv_nsec - start->tv_nsec;
	if (interval.tv_nsec < 0)
	{
		interval.tv_nsec += ONESECOND;
		interval.tv_sec -= 1;
		// ASSERT(interval.tv_sec >= 0)
	}

	if (interval.tv_sec >= ONEDAY)
	{
		snprintf(buf, bufsize, "%ld+", (long) interval.tv_sec / ONEDAY);
		interval.tv_sec %= ONEDAY;
		bufsize -= strlen(buf);
		buf += strlen(buf);
		force = true;
	}
	if (force || interval.tv_sec >= ONEHOUR)
	{
		snprintf(buf, bufsize, "%d:", (int) interval.tv_sec / ONEHOUR);
		interval.tv_sec %= ONEHOUR;
		bufsize -= strlen(buf);
		buf += strlen(buf);
		force = true;
	}
	if (force || interval.tv_sec >= ONEMINUTE)
	{
		snprintf(buf, bufsize, "%02d:", (int) interval.tv_sec / ONEMINUTE);
		interval.tv_sec %= ONEMINUTE;
		bufsize -= strlen(buf);
		buf += strlen(buf);
		force = true;
	}
	char *fmt = force ? "%02d.%06d" : "%d.%06d";
	snprintf(buf, bufsize, fmt,
			(int) interval.tv_sec, interval.tv_nsec / 1000);
}


/*
**  EP_TIME_PARSE --- convert external to internal format
**
**	Only works for a very specific format.
*/


/*
**  EP_TIME_PARSE --- parse a human date into internal form
**
**	Uses up to three algorithms.  The first, built in, is to parse
**	an ISO 8601 date (including nanoseconds) in an ad-hoc way.
**	If the string ends in "Z" it will be GMT, otherwise it will
**	be local time.  This is the only algorithm that handles
**	nanoseconds, which is why it is first.
**
**	The second algorithm is to use strptime(3) with a set of
**	built-in strings.  This should work on most systems.
**
**	The third algorithm is to use getdate(3) (a GNU extension) if
**	available.  It's not on BSD, but it is on Darwin/MacOS.
**	Oddly, it has a manpage on Ubuntu but doesn't seem to be
**	implemented in the library, so it's not clear if this
**	algorithm is broadly useful.  Also, getdate is hard to use,
**	requiring that the caller build their own pattern file.
*/

EP_STAT
ep_time_parse(const char *dtstr, EP_TIME_SPEC *ts, uint32_t flags)
{
	EP_STAT estat;
	const char *dsp = dtstr;
	int nbytes = 0;
	struct tm tmb;
	struct tm *tm = &tmb;
	bool zulu = !EP_UT_BITSET(EP_TIME_USE_LOCALTIME, flags);

	memset(ts, 0, sizeof *ts);
	ts->tv_accuracy = 0.0;

	estat = EP_STAT_TIME_PARSE;		// assume failure

	// start with ISO format dates (including timezone & nsecs)
	time_t (*cvtfunc)(struct tm *) = zulu ? &timegm : &mktime;
	int i;
	char sepbuf[10];

	memset(tm, 0, sizeof *tm);
	i = sscanf(dsp, "%d%n-%d%n-%d%n%[ tT_@]%n%d%n:%d%n:%d%n.%uld%n",
			&tm->tm_year, &nbytes,
			&tm->tm_mon, &nbytes,
			&tm->tm_mday, &nbytes,
			sepbuf, &nbytes,
			&tm->tm_hour, &nbytes,
			&tm->tm_min, &nbytes,
			&tm->tm_sec, &nbytes,
			&ts->tv_nsec, &nbytes);
	ep_dbg_cprintf(Dbg, 32,
			"ep_time_parse (%d fields): "
			"%04d-%02d-%02dT%02d:%02d:%02d.%09" PRIi32 "\n",
			i,
			tm->tm_year, tm->tm_mon, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			ts->tv_nsec);
	dsp += nbytes;
	if (*dsp == 'Z')
		cvtfunc = timegm;
	if (i >= 1)
		tm->tm_year -= 1900;
	if (i >= 2)
		tm->tm_mon -= 1;
	if (i > 0)
		estat = EP_STAT_OK;

#if EP_OSPF_HAS_STRPTIME
	// if that failed, try strptime with some home-grown formats
	static const char *fmts[] =
	{
		// full date/time conversions
		"%Y-%m-%dT%H:%M:%S",		// 2016-09-02T13:20:30	(ISO)
		"%Y-%m-%dt%H:%M:%S",		// 2016-09-02t13:20:30
		"%Y-%m-%d_%H:%M:%S",		// 2016-09-02_13:20:30
		"%Y-%m-%d %H:%M:%S",		// 2016-09-02 13:20:30
		"%c",				// locale's date time format

		// date-only conversions
		"%x",				// locale's date format
		"%Y-%m-%d",			// 2016-09-02
#		"%m/%d/%Y",			// 09/02/2016	(USA-centric)
#		"%m/%d/%y",			// 09/02/16	(USA-centric)
		"%B %e",			// September 2
		"%b %e",			// Sep 2
		"%A",				// Monday
		"%a",				// Mon

		// time-only conversions
		"%X",				// locale's time format
		"%H:%M:%S",			// 13:20:30
		"%I:%M%t%p",			// 1:20 pm
		"%H:%M",			// 13:20
		"%I%t%p",			// 1 pm, 1pm

		NULL
	};
	if (!EP_STAT_ISOK(estat))
	{
		const char **f = fmts;
		EP_TIME_SPEC now1;
		time_t now;

		estat = ep_time_now(&now1);
		EP_STAT_CHECK(estat, goto fail0);
		now = now1.tv_sec;

		while (*f != NULL)
		{
			// initialize tm to have current values
			(*(zulu ? gmtime_r : localtime_r))(&now, tm);

			// strptime may not set all fields
			if (strptime(dsp, f, tm) != NULL)
			{
				estat = EP_STAT_OK;
				break;
			}
		}
	}
fail0:
#endif // EP_OSCF_HAS_STRPTIME

#if EP_OSCF_USE_GETDATE
	// if that failed, try getdate (if available)
	static bool initialized = false;
	static bool use_getdate = false;
	static const char *gd_errs[] =
	{
		NULL,
		"The DATEMSK environment variable is null or undefined",
		"The template file cannot be opened for reading",
		"Failed to get file status information",
		"The template file is not a regular file",
		"An error is encountered while reading the template file",
		"The malloc(3) function failed (not enough memory is available)",
		"There is no line in the template that matches the input",
		"The input specification is invalid (for example, February 31)",
	};

	if (!initialized)
	{
		const char *p = getenv("DATEMSK");

		if (p == NULL)
			p = ep_adm_getstrparam("libep.time.datemsk", NULL);

		if (p != NULL && *p != '\0')
		{
			(void) setenv("DATEMSK", p, 0);
			use_getdate = true;
		}
		initialized = true;
	}
	if (use_getdate)
	{
		// try and use the getdate function (Linux and Darwin only)
		int gd_err;

#if EP_OSCF_HAS_GETDATE_R
		gd_err = getdate_r(dsp, tm);
#else
		tm = getdate(dsp);
		gd_err = getdate_err;
#endif
		if (tm != NULL)
		{
			estat = EP_STAT_OK;
		}
		else
		{
			// getdate failed, fall back to internal version
			if (gd_err > 0 && ep_dbg_test(Dbg, 28))
			{
				if ((unsigned) gd_err <
					(sizeof gd_errs / sizeof gd_errs[0]))
					ep_dbg_printf("Cannot convert date; getdate says:\n"
							"    %s\n",
							gd_errs[gd_err]);
				else
					ep_dbg_printf("Cannot convert date, getdate code %d\n",
								gd_err);
			}
		}
	}
#endif // EP_OSCF_USE_GETDATE

	// collect the results from whichever algorithm worked (if any)
	if (EP_STAT_ISOK(estat))
	{
		ts->tv_sec = (*cvtfunc)(tm);

		// see if we have /accuracy
		dsp = strchr(dsp, '/');
		if (dsp != NULL)
		{
			dsp++;
			if (sscanf(dsp, "%f%n", &ts->tv_accuracy, &nbytes) > 0)
				dsp += nbytes;
			nbytes = dsp - dtstr;
		}

		if (ep_dbg_test(Dbg, 17))
		{
			char tbuf[60];

			ep_dbg_printf("ep_time_parse: from %s\n\tto %s\n",
					dtstr,
					ep_time_format(ts, tbuf, sizeof tbuf,
						EP_TIME_FMT_HUMAN));
		}
		EP_ASSERT(nbytes >= 0);
		estat = EP_STAT_FROM_INT(nbytes);
	}

	return estat;
}


/***********************************************************************
**
**  EP_TIME_PARSE_INTERVAL -- parse a string representation of an interval
**
**	The caller is allowed to indicate units with a suffix:
**	    n   -- nanoseconds (scale × 1)
**	    u   -- microseconds (scale × 10³)
**	    l   -- milliseconds (scale × 10⁶)
**	    ms  -- milliseconds (scale × 10⁶) (alternate form, str only)
**	    s   -- seconds (scale × 10⁹)
**	    m/M -- minutes (scale × 60 × 10⁹)
**	    h/H -- hours
**	    d/D -- days
**	    w/W -- weeks
**	These can be combined, e.g., "2h30s" would mean two minutes
**	thirty seconds (that is, two and one half minutes).  The
**	floating point representation (2.5m) is not supported at
**	this time.
**
**	Parameters:
**		str -- the default value, represented as a string.
**		units -- if parameter has no units, assume this value.
**		tsp -- pointer to EP_TIME_SPEC to be filled in.
**
**	Returns:
**		none (return value comes through *tsp).
**
**	Bugs:
**		There should be some way to specify the accuracy.
*/

#define MICROSECONDS	* INT64_C(1000)
#define MILLISECONDS	* INT64_C(1000000)
#define SECONDS		* INT64_C(1000000000)

void
ep_time_parse_interval(
		const char *str,
		const char units,
		EP_TIME_SPEC *tsp)
{
	int64_t nsec = 0;		// 64-bit to allow for overflow
	tsp->tv_sec = 0;
	while (*str != '\0')
	{
		char *ep;
		long long ll = strtoll(str, &ep, 0);
		// if no input consumed, avoid infinite loop
		if (ep == str)
			break;
		if (*(str = ep) != '\0')
			str++;

		// decode units
		char u = *ep;
		if (u == '\0')
			u = units;
		switch (u)
		{
		  case 'n':
			nsec += ll;
			break;

		  case 'u':
			nsec += ll MICROSECONDS;
			break;

		  case 'l':
		  case '\0':	// no units?  use this as default
			nsec += ll MILLISECONDS;
			break;

		  case 's':
			tsp->tv_sec += ll;
			break;

		  case 'm':
			if (*str == 's')
			{
				// input is "ms" (milliseconds)
				nsec += ll MILLISECONDS;
				break;
			}
			// fall through to minutes

		  case 'M':
			tsp->tv_sec += ll * 60;
			break;

		  case 'H':
		  case 'h':
			tsp->tv_sec += ll * 60 * 60;
			break;

		  case 'D':
		  case 'd':
			tsp->tv_sec += ll * 60 * 60 * 24;
			break;

		  case 'W':
		  case 'w':
			tsp->tv_sec += ll * 60 * 60 * 24 * 7;
			break;

		  default:
			// give an error here?
			break;
		}
	}

	// deal with negative nanoseconds
	while (nsec < 0)
		nsec += 1 SECONDS, tsp->tv_sec--;

	// enough nanoseconds turn into seconds
	tsp->tv_sec += nsec / (1 SECONDS);
	nsec %= 1 SECONDS;

	// finish up the result
	tsp->tv_nsec = nsec;
	tsp->tv_accuracy = 0.0;
}

void
ep_time_parse_param(
		const char *param,
		const char *def,
		const char units,
		EP_TIME_SPEC *tsp)
{
	const char *p = ep_adm_getstrparam(param, def);
	if (p == NULL)
		p = "";
	ep_time_parse_interval(p, units, tsp);
}
