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
#include <ep_pcvt.h>
#include <ep_string.h>


/*
**  PRINT CONVERSION ROUTINES
**
**	These are conversions from various formats to string, intended
**	for short-term use.  In all cases, the caller passes in a
**	buffer in which to store the results.  The value will be
**	trimmed as necessary to fit that area.
*/

const char *
ep_pcvt_str(
	char *obuf,
	size_t osize,
	const char *val)
{
	size_t vl;

	if (val == NULL)
	{
		strlcpy(obuf, "<NULL>", osize);
		return obuf;
	}

	vl = strlen(val);
	if (vl < osize)
		return val;
	strncpy(obuf, val, osize - 7);
	strlcat(obuf, "...", osize);
	strlcat(obuf, &val[vl - 3], osize);
	return obuf;
}


const char *
ep_pcvt_int(
	char *obuf,
	size_t osize,
	int base,
	long val)
{
	const char *fmtstr;

	switch (base)
	{
	  case 8:
	  case -8:
		fmtstr = "%o";
		break;

	  case 16:
	  case -16:
		fmtstr = "%X";
		break;

	  case 10:
	  default:
		fmtstr = "%l";
		break;

	  case -10:
		fmtstr = "%ul";
		break;
	}

	snprintf(obuf, osize, fmtstr, val);
	return obuf;
}
