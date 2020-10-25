/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**  ----- BEGIN LICENSE BLOCK -----
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
**  ----- END LICENSE BLOCK -----
*/

/*
**	These headers are not intended for external use.
*/

#ifndef _GDPD_ADMIN_H_
#define _GDPD_ADMIN_H_

#include <stdarg.h>
#include <stdint.h>

/*
**  Statistics gathering
*/

extern void		admin_post_stats(
						uint32_t mask,
						const char *,
						...);
extern void		admin_post_statsv(
						uint32_t mask,
						const char *,
						va_list);

#define ADMIN_LOG_EXIST		0x00000001	// existential (create/destroy) events
#define ADMIN_LOG_OPEN		0x00000002	// initial log open
#define ADMIN_LOG_READ		0x00000004	// read operations
#define ADMIN_LOG_WRITE		0x00000008	// write/append operations
#define ADMIN_LOG_SNAPSHOT	0x00000010	// periodic log summary (size, etc.)

#endif // _GDPD_ADMIN_H_
