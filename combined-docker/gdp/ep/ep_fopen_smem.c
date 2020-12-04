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
**  Open up a stream backed by (static) memory.
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
#include <sys/errno.h>

struct meminfo
{
	char *bufb;		// base of buffer
	size_t bufs;		// size of buffer
	size_t bufx;		// index of current read/write pointer
};

static IORESULT_T
memread(void *cookie, char *buf, IOBLOCK_T size)
{
	struct meminfo *minf = (struct meminfo *) cookie;
	size_t l = minf->bufs - minf->bufx;

	if (l > size)
		l = size;
	if (l > 0)
		memcpy(buf, minf->bufb + minf->bufx, l);
	minf->bufx += l;
	return l;
}

static IORESULT_T
memwrite(void *cookie, const char *buf, IOBLOCK_T size)
{
	struct meminfo *minf = (struct meminfo *) cookie;
	size_t l = minf->bufs - minf->bufx;

	if (l > size)
		l = size;
	if (l < size)
		errno = ENOSPC;
	if (l > 0)
	{
		memcpy(minf->bufb + minf->bufx, buf, l);
		minf->bufx += l;
		return l;
	}
	else
	{
		return -1;
	}
}

static int
memclose(void *cookie)
{
	struct meminfo *minf = (struct meminfo *) cookie;

	if (minf->bufs > minf->bufx)
		minf->bufb[minf->bufx] = '\0';
	ep_mem_free(cookie);
	return 0;
}


FILE *
ep_fopen_smem(void *buf,
	size_t size,
	const char *mode)
{
	struct meminfo *minf = (struct meminfo *) ep_mem_zalloc(sizeof *minf);
	if (minf == NULL)
		return NULL;
	minf->bufb = (char *) buf;
	minf->bufs = size;
	minf->bufx = 0;

#if __FreeBSD__ || __APPLE__
	{
		// BSD/MacOS
		return funopen(minf, &memread, &memwrite, NULL, &memclose);
	}
#elif __linux__
	{
		// Linux
		cookie_io_functions_t iof =
		{
			&memread,
			&memwrite,
			NULL,
			&memclose
		};
		return fopencookie(minf, mode, iof);
	}
#endif
}
