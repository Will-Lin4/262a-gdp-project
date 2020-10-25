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
**  Definitions for event logging.
*/

#ifndef _EP_LOG_H_
# define _EP_LOG_H_

# include <ep/ep.h>
__BEGIN_DECLS

typedef void	EP_LOG_FUNC(
		void		*ctx,	// function-specific
		EP_STAT		estat,	// status of message
		const char	*fmt,	// format of message
		va_list		ap);	// arguments to fmt

void	ep_log_init(
		const char *tag,	// NULL => use program name
		int logfac,		// -1   => don't use syslog
		FILE *logfile);		// NULL => don't log to open file

void	ep_log_addmethod(
		EP_LOG_FUNC *func,	// function to call
		void *ctx,		// ctx argument to func
		int minsev);		// minimum severity to log

void	ep_log_file(		// utility function to log to a file
		void *fp,		// file to write
		EP_STAT estat,		// status of message
		const char *fmt,	// format of message
		va_list ap);		// arguments to fmt


void	ep_logv(EP_STAT estat, const char *fmt, va_list ap);

void EP_TYPE_PRINTFLIKE(2, 3)
		ep_log(EP_STAT estat, const char *fmt, ...);

__END_DECLS
#endif // _EP_LOG_H_
