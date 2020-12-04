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
#include <ep_hexdump.h>
#include <ep_string.h>

#include <ctype.h>
#include <stdio.h>

void
ep_hexdump(const void *bufp, size_t buflen, FILE *fp,
		int format, size_t offset)
{
	size_t bufleft = buflen;
	const uint8_t *b = (const uint8_t *) bufp;
	const size_t width = 16;
	bool terse = EP_UT_BITSET(EP_HEXDUMP_TERSE, format);
	bool ascii = EP_UT_BITSET(EP_HEXDUMP_ASCII, format);

	flockfile(fp);			// to make threads happy
	while (bufleft > 0)
	{
		size_t lim = bufleft;
		unsigned int i;
		int shift = 0;

		if (!terse)
		{
			shift = offset % width;
			if (lim > width)
				lim = width;
			if (lim > (width - shift))
				lim = width - shift;
			fprintf(fp, "%08zx", offset);
			if (shift != 0)
				fprintf(fp, "%*s", shift * 3, "");
		}
		for (i = 0; i < lim; i++)
			fprintf(fp, "%s%02x", terse ? "" : " ", b[i]);
		if (ascii)
		{
			fprintf(fp, "\n        ");
			if (shift != 0)
				fprintf(fp, "%*s", shift * 3, "");
			for (i = 0; i < lim; i++)
			{
				if (isprint(b[i]))
					fprintf(fp, " %c ", b[i]);
				else
					fprintf(fp, " %s ", EpChar->unprintable);
			}
		}
		fprintf(fp, "\n");
		b += lim;
		bufleft -= lim;
		offset += lim;
	}
	funlockfile(fp);
}
