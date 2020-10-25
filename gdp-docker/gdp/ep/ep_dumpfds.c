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
#include <ep/ep_app.h>
#include <sys/stat.h>
#include <sys/errno.h>

/*
**  EP_APP_DUMPFDS --- show all open file descriptors
**
**	Right now doesn't show much....
*/

void
ep_app_dumpfds(FILE *fp)
{
	long maxfds = sysconf(_SC_OPEN_MAX);
	int i;
	int j;

	for (i = j = 0; i < maxfds; i++)
	{
		struct stat sbuf;

		if (fstat(i, &sbuf) < 0)
			continue;
		if (++j > 10)
		{
			fprintf(fp, "\n");
			j = 1;
		}
		fprintf(fp, " %3d", i);
		if ((sbuf.st_mode & S_IFMT) == S_IFSOCK)
			fprintf(fp, "s");
		else
			fprintf(fp, " ");
	}
	fprintf(fp, "\n");
	errno = 0;
}


/*
**  EP_APP_NUMFDS --- return number of open file descriptors
*/

int
ep_app_numfds(int *maxfdsp)
{
	int nfds = 0;
	int maxfds = sysconf(_SC_OPEN_MAX);
	int fd;

	for (fd = 0; fd < maxfds; fd++)
	{
		struct stat sbuf;

		if (fstat(fd, &sbuf) == 0)
			nfds++;
	}

	if (maxfdsp != NULL)
		*maxfdsp = maxfds;
	return nfds;
}
