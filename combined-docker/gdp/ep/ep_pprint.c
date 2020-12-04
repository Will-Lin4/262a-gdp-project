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
#include <ep_stat.h>
#include <ctype.h>


/***********************************************************************
**
**  EP_ST_PPRINT -- positional print
**
**	Interpret %N in fmt (where N is an number) as interpolating
**	the Nth argument.
**
**	Parameters:
**		fp -- the stream to print to
**		fmt -- the format string
**		argc -- the number of arguments
**		argv -- the actual arguments
**
**	Returns:
**		none
*/

void
ep_pprint(FILE *fp,
	const char *fmt,
	int argc,
	const char *const *argv)
{
	const char *ap;
	char c;
	int i;

	while ((c = *fmt++) != '\0')
	{
		// if it's not a percent, just copy through
		if (c != '%')
		{
			putc(c, fp);
			continue;
		}

		// if it is, look at the next byte
		if ((c = *fmt++) == '\0')
		{
			// hack for percent at end of string
			putc('%', fp);
			break;
		}

		if (!isdigit(c))
		{
			// bogus character, unless %%
			putc('%', fp);
			if (c != '%')
				putc(c, fp);
			continue;
		}

		// interpolate Nth argument
		i = c - '0';
		if (i <= 0 || i > argc)
			ap = "(unknown)";
		else if (argv[i - 1] == NULL)
			ap = "(null)";
		else
			ap = argv[i - 1];

		while ((c = *ap++) != '\0')
			putc(c, fp);
	}
}
