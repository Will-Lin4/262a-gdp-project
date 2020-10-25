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

#ifndef _EP_APP_H_
# define _EP_APP_H_
# include <ep/ep.h>
# include <stdlib.h>

__BEGIN_DECLS

extern void EP_TYPE_PRINTFLIKE(1, 2)
			ep_app_info(const char *fmt, ...);
extern void EP_TYPE_PRINTFLIKE(1, 2)
			ep_app_warn(const char *fmt, ...);
extern void EP_TYPE_PRINTFLIKE(1, 2)
			ep_app_error(const char *fmt, ...);
extern void EP_TYPE_PRINTFLIKE(1, 2)
			ep_app_severe(const char *fmt, ...);
extern void EP_TYPE_PRINTFLIKE(1, 2)
			ep_app_fatal(const char *fmt, ...)
				EP_ATTR_NORETURN;
extern void EP_TYPE_PRINTFLIKE(1, 2)
			ep_app_abort(const char *fmt, ...)
				EP_ATTR_NORETURN;
extern void EP_TYPE_PRINTFLIKE(2, 3)
			ep_app_message(EP_STAT estat, const char *fmt, ...);
extern void		ep_app_messagev(EP_STAT estat, const char *fmt, va_list av);

extern const char	*ep_app_getprogname(void);

extern void		ep_app_dumpfds(FILE *fd);
extern int		ep_app_numfds(int *maxfds);

extern void		ep_app_setflags(uint32_t flags);

#define EP_APP_FLAG_LOG_ABORT		0x00000001	// log ep_app_abort
#define EP_APP_FLAG_LOG_FATAL		0x00000002	// log ep_app_fatal
#define EP_APP_FLAG_LOG_SEVERE		0x00000004	// log ep_app_severe
#define EP_APP_FLAG_LOG_ERROR		0x00000008	// log ep_app_error
#define EP_APP_FLAG_LOG_WARNING		0x00000010	// log ep_app_warn
#define EP_APP_FLAG_LOG_OK		0x00000040	// log OK messages
	// pseudo (doesn't have an associated log severity)
#define EP_APP_FLAG_LOG_INFO		0x00000040	// log ep_app_info

__END_DECLS

#endif //_EP_APP_H_
