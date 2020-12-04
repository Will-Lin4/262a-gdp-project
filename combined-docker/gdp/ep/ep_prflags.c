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
#include <ep_dbg.h>
#include <ep_prflags.h>
#include <stdio.h>


/*
**  EP_PRFLAGS -- print out a flags word in a user-friendly manner
**
**	Parameters:
**		flagword -- the word of flags to print
**		flagdesc -- a vector of descriptors for those flags
**		out -- the output file on which to print them
**
**	Returns:
**		none.
*/

void
ep_prflags(
	uint32_t flagword,
	const EP_PRFLAGS_DESC *flagdesc,
	FILE *out)
{
	bool firsttime = true;

	if (out == NULL)
		out = ep_dbg_getfile();
	if (flagword == 0)
	{
		fprintf(out, "0");
		return;
	}
	flockfile(out);			// thread magic
	fprintf(out, "0x%x<", flagword);
	for (; flagdesc->name != NULL; flagdesc++)
	{
		if (((flagword ^ flagdesc->bits) & flagdesc->mask) != 0)
			continue;
		fprintf(out, "%s%s",
			firsttime ? "" : ",",
			flagdesc->name);
		firsttime = false;
	}
	fprintf(out, ">");
	funlockfile(out);
}
