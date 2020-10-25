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
#include <ep_string.h>
#include <ep_pcvt.h>
#include <unistd.h>
#include <stdarg.h>



/***********************************************************************
**
**  STRLCPY, STRLCAT -- string copy/concatenate with buffer size
**
**	Some systems (e.g., Linux) still don't provide these.
*/

#if !EP_OSCF_HAS_STRLCPY

size_t
strlcpy(
	char *dst,
	const char *src,
	size_t n)
{
	size_t i = 0;

	if (n == 0)
		return 0;
	while (--n > 0 && *src != '\0')
	{
		*dst++ = *src++;
		i++;
	}
	*dst++ = '\0';
	if (*src != '\0')
		i += strlen(src);
	return ++i;
}


size_t
strlcat(
	char *dst,
	const char *src,
	size_t n)
{
	size_t i = strlen(dst);

	if (n == 0)
		return 0;
	while (n > 0 && *dst++ != '\0')
		n--;
	--dst; // needed because dst points to the char after the null terminator
	while (*src != '\0' && --n > 0)
	{
		*dst++ = *src++;
		i++;
	}
	if (*src != '\0')
		i += strlen(src);
	*dst++ = '\0';
	return ++i;
}

#endif /* EP_OSCF_HAS_STRLCPY */


/***********************************************************************
**
**  EP_STR_LCPYN -- copy and concatenate N strings
**
**	Parameters:
**		dst -- destination pointer.
**		siz -- size of space available at dst.
**		... -- a NULL terminated list of strings to copy to dst.
**
**	Returns:
**		The number of bytes that would have been copied if dst
**		were large enough.
*/

// helper routine really for internal use only.  Could be exposed if needed.
size_t
ep_str_vlcpyn(
	char *dst,
	size_t siz,
	va_list av)
{
	size_t asize = 0;		// actual size
	char *p = NULL;

	if (siz <= 0)
		return siz;		// no room for anything

	siz--;				// allow for NULL byte at end
	while (siz > 0 && (p = va_arg(av, char *)) != NULL)
	{
		while (siz-- > 0 && (*dst = *p++) != '\0')
			dst++, asize++;
	}
	*dst = '\0';

	// see if we fit
	if (p != NULL)
	{
		// buffer has overflowed; just count the rest

		// first any residual in the current string
		while (*p++ != '\0')
			asize++;

		// then future strings
		while ((p = va_arg(av, char *)) != NULL)
			asize += strlen(p);
	}

	return asize;
}

size_t
ep_str_lcpyn(
	char *dst,
	size_t siz,
	...)
{
	va_list av;
	size_t asize;			// actual size

	va_start(av, siz);
	asize = ep_str_vlcpyn(dst, siz, av);
	va_end(av);

	return asize;
}


/***********************************************************************
**
**  EP_STR_LCATN -- copy and concatenate N strings to existing string
**
**	Parameters:
**		dst -- destination pointer.
**		siz -- size of space available at dst.
**		... -- a NULL terminated list of strings to copy to dst.
**
**	Returns:
**		The number of bytes that would have been copied if dst
**		were large enough.
*/

size_t
ep_str_lcatn(
	char *dst,
	size_t siz,
	...)
{
	va_list av;
	size_t i = strlen(dst);

	va_start(av, siz);
	i += ep_str_vlcpyn(dst + i, siz - i, av);
	va_end(av);

	return i;
}


/***********************************************************************
**
**  Video escape sequences.  Should be setable.
*/

// ANSI X3.64
struct epVidSequences	EpVidANSI_X3_64 =
{
	.vidnorm =	"\033[0m",	// video normal
	.vidbold =	"\033[1m",	// video bold
	.vidfaint =	"\033[2m",	// video faint
	.vidstout =	"\033[3m",	// video standout
	.viduline =	"\033[4m",	// video underline
	.vidblink =	"\033[5m",	// video blink (ugh)
	.vidinv =	"\033[7m",	// video invert
	.vidfgblack =	"\033[30m",	// video foreground black
	.vidfgred =	"\033[31m",	// video foreground red
	.vidfggreen =	"\033[32m",	// video foreground green
	.vidfgyellow =	"\033[33m",	// video foreground yellow
	.vidfgblue =	"\033[34m",	// video foreground blue
	.vidfgmagenta =	"\033[35m",	// video foreground magenta
	.vidfgcyan =	"\033[36m",	// video foreground cyan
	.vidfgwhite =	"\033[37m",	// video foreground white
	.vidbgblack =	"\033[40m",	// video background black
	.vidbgred =	"\033[41m",	// video background red
	.vidbggreen =	"\033[42m",	// video background green
	.vidbgyellow =	"\033[43m",	// video background yellow
	.vidbgblue =	"\033[44m",	// video background blue
	.vidbgmagenta =	"\033[45m",	// video background magenta
	.vidbgcyan =	"\033[46m",	// video background cyan
	.vidbgwhite =	"\033[47m",	// video background white
};

// none
struct epVidSequences	EpVidNull =
{
	.vidnorm =	"",		// video normal
	.vidbold =	"",		// video bold
	.vidfaint =	"",		// video faint
	.vidstout =	"",		// video standout
	.viduline =	"",		// video underline
	.vidblink =	"",		// video blink (ugh)
	.vidinv =	"",		// video invert
	.vidfgblack =	"",		// video foreground black
	.vidfgred =	"",		// video foreground red
	.vidfggreen =	"",		// video foreground green
	.vidfgyellow =	"",		// video foreground yellow
	.vidfgblue =	"",		// video foreground blue
	.vidfgmagenta =	"",		// video foreground magenta
	.vidfgcyan =	"",		// video foreground cyan
	.vidfgwhite =	"",		// video foreground white
	.vidbgblack =	"",		// video background black
	.vidbgred =	"",		// video background red
	.vidbggreen =	"",		// video background green
	.vidbgyellow =	"",		// video background yellow
	.vidbgblue =	"",		// video background blue
	.vidbgmagenta =	"",		// video background magenta
	.vidbgcyan =	"",		// video background cyan
	.vidbgwhite =	"",		// video background white
};

struct epVidSequences	*EpVid = &EpVidNull;


/***********************************************************************
**
**  EP_STR_VID_SET -- set the desired video sequence
**
**	Parameters:
**		type -- the type of sequence to use, either "ansi" or
**			"none"
**
**	Returns:
**		status code
**
**	Side Effects:
**		Updates the binding of 'EpVid'
*/

EP_STAT
ep_str_vid_set(
	const char *type)
{
	if (type == NULL)
		type = getenv("EP_TERM");
	if (type == NULL)
	{
		// try to guess based on environment
		if (isatty(1))
		{
			type = getenv("TERM");
			if (type != NULL &&
			    (strncmp(type, "xterm", 5) == 0 ||
			     strncmp(type, "ansi", 4) == 0))
				EpVid = &EpVidANSI_X3_64;
		}
		return EP_STAT_OK;
	}

	if (strcasecmp(type, "none") == 0)
	{
		EpVid = &EpVidNull;
	}
	else if (strcasecmp(type, "ansi") == 0)
	{
		EpVid = &EpVidANSI_X3_64;
	}
	else
	{
		char e1buf[64];

		fprintf(stderr, "ep_str_vid_set: video type `%s' not valid\n",
			ep_pcvt_str(e1buf, sizeof e1buf, type));
		return EP_STAT_ERROR;
	}
	return EP_STAT_OK;
}



/***********************************************************************
**
**  Special characters used internally that may vary by locale.
**  This is not intended to be complete; it's really for internal
**  use only.
*/

// US-ASCII
struct epCharSequences	EpCharASCII =
{
	.lquote =	"`",		// lquote
	.rquote =	"'",		// rquote
	.copyright =	"(c)",		// copyright
	.degree =	"deg",		// degree
	.micro =	"u",		// micro
	.plusminus =	"+/-",		// plus-or-minus
	.times =	"*",		// times
	.divide =	"/",		// divide
	.null =		"NULL",		// null
	.notequal =	"!=",		// not equal
	.unprintable =	"?",		// sub for unprintable
	.paragraph =	"pp.",
	.section =	"sec.",		// section
	.notsign =	"(not)",	// "not" symbol
	.infinity =	"(inf)",
};

// ISO 8859-1:1987
struct epCharSequences	EpCharISO_8859_1 =
{
	.lquote =	"\xAB",		// << character
	.rquote =	"\xBB",		// >> character
	.copyright =	"\xA9",
	.degree =	"\xB0",
	.micro =	"\xB5",
	.plusminus =	"\xB1",
	.times =	"\xD7",
	.divide =	"\xF7",
	.null =		"\xD8",
	.notequal =	"!=",
	.unprintable =	"\xA4",		// generic currency symbol
	.paragraph =	"\xB6",
	.section =	"\xA7",
	.notsign =	"\xAC",
	.infinity =	"(inf)",
};

// Unicode UTF-8 (US conventions)
struct epCharSequences	EpCharUTF_8 =
{
	.lquote =	"\xC2\xAB",	// '<<' character U+00AB, 0xC2 AB
	.rquote =	"\xC2\xBB",	// '>>' character U+00BB, 0xC2 BB
	.copyright =	"\xC2\xA9",	// U+00A9, 0xC2 A9
	.degree =	"\xC2\xB0",	// U+00B0
	.micro =	"\xC2\xB5",	// U+00B5
	.plusminus =	"\xC2\xB1",	// U+00B1, 0xC2 B1
	.times =	"\xC3\x97",	// U+00D7
	.divide =	"\xC3\xB7",	// U+00F7
	.null =		"\xE2\x88\x85",	// U+2205
	.notequal =	"\xE2\x89\xA0",	// U+2260
	.unprintable =	"\xE2\x8C\xA7",	// U+2327, "X" in a box
//	.unprintable =	"\xE2\x8D\xB0",	// U+2370, question mark in a box
	.paragraph =	"\xC2\xB6",	// U+00B6
	.section =	"\xC2\xA7",	// U+00A7
	.notsign =	"\xC2\xAC",	// U+00AC
	.infinity =	"\xE2\x88\x9E",	// U+221E
};

struct epCharSequences	*EpChar = &EpCharASCII;


/***********************************************************************
**
**  EP_STR_CHAR_SET -- set the desired character set sequences
**
**	Parameters:
**		type -- the type of sequence to use, one of
**			"ascii", "iso-8859-1", or "utf-8"
**
**	Returns:
**		status code
**
**	Side Effects:
**		Updates the binding of 'EpChar'
*/

EP_STAT
ep_str_char_set(
	const char *type)
{
	if (type == NULL)
	{
		// try to guess based on environment
		const char *p;

		if (!isatty(1))
			return EP_STAT_OK;

		type = getenv("LANG");
		if (type == NULL)
			return EP_STAT_OK;
		p = strrchr(type, '.');
		if (p != NULL)
			type = ++p;
	}

	if (strcasecmp(type, "iso-8859-1") == 0 ||
		 strcasecmp(type, "iso-latin-1") == 0)
	{
		EpChar = &EpCharISO_8859_1;
	}
	else if (strcasecmp(type, "utf-8") == 0 ||
		 strcasecmp(type, "utf_8") == 0 ||
		 strcasecmp(type, "utf8") == 0)
	{
		EpChar = &EpCharUTF_8;
	}
	else
	{
		EpChar = &EpCharASCII;
	}
	return EP_STAT_OK;
}
