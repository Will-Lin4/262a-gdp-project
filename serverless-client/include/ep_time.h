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

#ifndef _EP_TIME_H_
#define _EP_TIME_H_

#include <ep/ep.h>
//------------------------------------------------------------------------------
//	Added for C++ compability
extern "C"{
//------------------------------------------------------------------------------
__BEGIN_DECLS

#include <time.h>
#include <sys/time.h>

#pragma pack(push, 1)
typedef struct
{
	int64_t		tv_sec;		// seconds since Jan 1, 1970
	int32_t		tv_nsec;	// nanoseconds
	float		tv_accuracy;	// clock accuracy in seconds
} EP_TIME_SPEC;
#pragma pack(pop)

// doesn't use INT64_MIN because protobuf chokes on that as default
// this is INT64_MIN + 1
#define EP_TIME_NOTIME		(-INT64_MAX)
#define EP_TIME_MAXTIME		INT64_MAX

// return current time
extern EP_STAT	ep_time_now(EP_TIME_SPEC *tv);

// zero out a time
extern void	ep_time_zero(EP_TIME_SPEC *tv);

// return current time plus an offset
extern EP_STAT	ep_time_deltanow(
				EP_TIME_SPEC *delta,
				EP_TIME_SPEC *tv);

// add a delta to a time (delta may be negative)
extern void	ep_time_add_delta(
				EP_TIME_SPEC *delta,
				EP_TIME_SPEC *tv);

// determine if A occurred before B
extern bool	ep_time_before(
				EP_TIME_SPEC *a,
				EP_TIME_SPEC *b);

// subtract time A from time B, leaving result in B
extern void	ep_time_subtract(
				EP_TIME_SPEC *a,
				EP_TIME_SPEC *b);

// return the difference between two times in microseconds
extern int64_t	ep_time_diff_usec(
				EP_TIME_SPEC *a,
				EP_TIME_SPEC *b);

// create a scalar number of nanoseconds and microseconds from a time
// These should be used only for time intervals because of overflow problems.
extern int64_t	ep_time_to_nsec(
				EP_TIME_SPEC *tv);
extern int64_t	ep_time_to_usec(
				EP_TIME_SPEC *tv);

// create a time from a scalar number of nanoseconds, microseconds, seconds
// The nsec and usec versions should only be used for time intervals
// because of integer overflow problems.
extern void	ep_time_from_nsec(
				int64_t nsec,
				EP_TIME_SPEC *tv);
extern void	ep_time_from_usec(
				int64_t usec,
				EP_TIME_SPEC *tv);
extern void	ep_time_from_sec(
				int64_t sec,
				EP_TIME_SPEC *tv);

// return putative clock accuracy
extern float	ep_time_accuracy(void);

// set the clock accuracy (may not be available)
extern void	ep_time_setaccuracy(float acc);

// format a time string into a buffer
extern char	*ep_time_format(const EP_TIME_SPEC *tv,
				char *buf,
				size_t bz,
				uint32_t flags);

// format a time string to a file
extern void	ep_time_print(const EP_TIME_SPEC *tv,
				FILE *fp,
				uint32_t);

// format a time interval to a buffer
extern void	ep_time_format_interval(
				EP_TIME_SPEC *start,	// starting time
				EP_TIME_SPEC *end,	// ending time
				char *buf,		// output buffer
				size_t bufsize);	// size of output buffer

// values for ep_time_format and ep_time_print flags
#define EP_TIME_FMT_DEFAULT	0		// pseudo-flag
#define EP_TIME_FMT_HUMAN	0x00000001	// format for humans
#define EP_TIME_FMT_NOFUZZ	0x00000002	// suppress accuracy printing
#define _EP_TIME_FMT_SIGFIGMASK	0x000F0000	// number of sig digits
#define EP_TIME_FMT_SIGFIG9	    0x00090000		// 9 digits (nsec)
#define EP_TIME_FMT_SIGFIG6	    0x00060000		// 6 digits (usec)
#define EP_TIME_FMT_SIGFIG3	    0x00030000		// 3 digits (msec)
#define EP_TIME_FMT_SIGFIG0	    0x000F0000		// 0 digits (sec)
#define _EP_TIME_FMT_SIGFIGSHIFT	16

// parse a time string
extern EP_STAT	ep_time_parse(const char *timestr,
				EP_TIME_SPEC *tv,
				uint32_t flags);

#define EP_TIME_USE_UTC		0x00000000	// assume UTC (default)
#define EP_TIME_USE_LOCALTIME	0x00000001	// assume times in local zone

// parse a time interval from string or administrative parameter
extern void		ep_time_parse_interval(
				const char *str,	// string to parse
				char def_units,		// default units
				EP_TIME_SPEC *tsp);	// output parameter
extern void		ep_time_parse_param(
				const char *param,	// name of parameter
				const char *def,	// default
				char def_units,		// default units
				EP_TIME_SPEC *tsp);	// output parameter

// sleep for a given interval
extern EP_STAT	ep_time_sleep(EP_TIME_SPEC *tv);

// sleep for the indicated number of nanoseconds
extern EP_STAT	ep_time_nanosleep(int64_t);

// test to see if a timestamp is valid
#define EP_TIME_IS_VALID(ts)	((ts)->tv_sec != EP_TIME_NOTIME)

// invalidate a timestamp
#define EP_TIME_INVALIDATE(ts)	((ts)->tv_sec = EP_TIME_NOTIME)

__END_DECLS
//------------------------------------------------------------------------------
//	Added for C++ compability
}
//------------------------------------------------------------------------------
#endif //_EP_TIME_H_
