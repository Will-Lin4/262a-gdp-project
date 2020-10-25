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
**  Open up a stream that has styling (e.g., colors or fonts)
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

struct styleinfo
{
	FILE	*underlying;	// underlying file
	char	*so;		// "shift out" string
	char	*si;		// "shift in" string
};

static IORESULT_T
styled_write(void *cookie, const char *buf, IOBLOCK_T size)
{
	struct styleinfo *sinf = (struct styleinfo *) cookie;
	IORESULT_T result = 0;

	flockfile(sinf->underlying);
	while (size > 0)
	{
		const char *p;
		IOBLOCK_T tlen;

		tlen = size;
		p = memchr(buf, '\n', tlen);
		if (p != NULL)
			tlen = p - buf;
		if (tlen > 0)
		{
			IORESULT_T tres;
			if (sinf->so != NULL)
				fputs(sinf->so, sinf->underlying);
			tres = fwrite(buf, 1, tlen, sinf->underlying);
			if (sinf->si != NULL)
				fputs(sinf->si, sinf->underlying);
			buf += tlen;
			size -= tlen;
			if (tres > 0)
				result += tres;
		}
		if (size > 0 && *buf == '\n')
		{
			fputc(*buf++, sinf->underlying);
			result++;
			size--;
		}
	}
	funlockfile(sinf->underlying);
	fflush(sinf->underlying);
	return result;
}

static int
styled_close(void *cookie)
{
	struct styleinfo *sinf = (struct styleinfo *) cookie;

	if (sinf->so != NULL)
		ep_mem_free(sinf->so);
	if (sinf->si != NULL)
		ep_mem_free(sinf->si);
	ep_mem_free(cookie);
	return 0;
}


FILE *
ep_fopen_styled(FILE *underlying,
		const char *so,
		const char *si)
{
	FILE *fp;
	struct styleinfo *sinf = (struct styleinfo *) ep_mem_zalloc(sizeof *sinf);
	if (sinf == NULL)
		return NULL;
	sinf->underlying = underlying;
	if (so != NULL)
		sinf->so = strdup(so);
	if (si != NULL)
		sinf->si = strdup(si);

#if __FreeBSD__ || __APPLE__
	{
		// BSD/MacOS
		fp = funopen(sinf, NULL, &styled_write, NULL, &styled_close);
	}
#elif __linux__
	{
		// Linux
		cookie_io_functions_t iof =
		{
			NULL,
			&styled_write,
			NULL,
			&styled_close
		};
		fp = fopencookie(sinf, "w", iof);
	}
#endif
	setlinebuf(fp);
	return fp;
}
