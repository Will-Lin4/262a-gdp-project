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
**  EP_FREAD_UNLOCKED --- unlocked file read
**
**	This is needed only because there is no Posix standard
**	fread_unlocked.
**
**	Be sure you use flockfile before calling this if you will
**	ever use this in a threaded environment!
*/

#include <ep.h>
#include <stdio.h>

size_t
ep_fread_unlocked(void *ptr, size_t size, size_t n, FILE *fp)
{
	size_t nbytes = size * n;
	char *b = (char *) ptr;
	unsigned int i;
	int c;

	if (nbytes == 0)
		return 0;
	for (i = 0; i < nbytes; i++)
	{
		c = getc_unlocked(fp);
		if (c == EOF)
			break;
		*b++ = c;
	}
	return i / size;
}


/*
**  EP_FWRITE_UNLOCKED --- unlocked file write
**
**	This is only needed because there is no Posix standard
**	fwrite_unlocked.
**
**	Be sure you use flockfile before calling this if you will
**	ever use this in a threaded environment!
*/

size_t
ep_fwrite_unlocked(void *ptr, size_t size, size_t n, FILE *fp)
{
	size_t nbytes = size * n;
	char *b = (char *) ptr;
	unsigned int i;

	if (nbytes == 0)
		return 0;
	for (i = 0; i < nbytes; i++)
	{
		if (putc_unlocked(*b++, fp) == EOF)
			break;
	}
	return i / size;
}
