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

/*
**  Open up a stream backed by syslog.
**
**	This is a wrapper around funopen (on BSD and MacOS) or
**	fopencookie (on Linux).  These two routimes do substantially
**	the same thing.
*/

#if __FreeBSD__ || __APPLE__
#   define IORESULT_T	int
#   define IOBLOCK_T	int
#elif __linux__
#   define IORESULT_T	ssize_t
#   define IOBLOCK_T	size_t
#   define _GNU_SOURCE	1	// required to get fopencookie
#else
#   error Cannot determine use of funopen vs fopencookie
#endif

#include <ep.h>

#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/errno.h>

struct syslog_info
{
	int	pri;		// log priority
};

static IORESULT_T
syslog_write(void *cookie, const char *buf, IOBLOCK_T size)
{
	struct syslog_info *slinf = (struct syslog_info *) cookie;
	int size_as_int = size;			// avoid gcc warnings

	syslog(slinf->pri, "%.*s", size_as_int, buf);

	return size;
}

static int
syslog_close(void *cookie)
{
	ep_mem_free(cookie);
	return 0;
}


FILE *
ep_fopen_syslog(
	int pri)
{
	struct syslog_info *slinf;

	slinf = (struct syslog_info *) ep_mem_zalloc(sizeof *slinf);
	if (slinf == NULL)
		return NULL;
	slinf->pri = pri;

#if __FreeBSD__ || __APPLE__
	{
		// BSD/MacOS
		return funopen(slinf, NULL, &syslog_write, NULL, &syslog_close);
	}
#elif __linux__
	{
		// Linux
		cookie_io_functions_t iof =
		{
			NULL,
			&syslog_write,
			NULL,
			&syslog_close
		};
		return fopencookie(slinf, "a", iof);
	}
#endif
}
