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

#ifndef _EP_STRING_H_
#define _EP_STRING_H_
#include <ep/ep.h>
__BEGIN_DECLS

#include <string.h>

// these really don't belong here -- maybe ep_video.h

// video sequences for terminals
extern struct epVidSequences
{
	const char	*vidnorm;	// set video to normal
	const char	*vidbold;	// set video to bold
	const char	*vidfaint;	// set video to faint
	const char	*vidstout;	// set video to "standout"
	const char	*viduline;	// set video to underline
	const char	*vidblink;	// set video to blink
	const char	*vidinv;	// set video to invert
	const char	*vidfgblack;	// set foreground black
	const char	*vidfgred;	// set foreground red
	const char	*vidfggreen;	// set foreground green
	const char	*vidfgyellow;	// set foreground yellow
	const char	*vidfgblue;	// set foreground blue
	const char	*vidfgmagenta;	// set foreground magenta
	const char	*vidfgcyan;	// set foreground cyan
	const char	*vidfgwhite;	// set foreground white
	const char	*vidbgblack;	// set background black
	const char	*vidbgred;	// set background red
	const char	*vidbggreen;	// set background green
	const char	*vidbgyellow;	// set background yellow
	const char	*vidbgblue;	// set background blue
	const char	*vidbgmagenta;	// set background magenta
	const char	*vidbgcyan;	// set background cyan
	const char	*vidbgwhite;	// set background white
} *EpVid;

// target video treatment, corresponding to above
//  (currently used only in ep_st_printf for %#c and %#s printing)
#define EP_VID_NORM	0
#define EP_VID_BOLD	0x00000001
#define EP_VID_FAINT	0x00000002
#define EP_VID_STOUT	0x00000004
#define EP_VID_ULINE	0x00000008
#define EP_VID_BLINK	0x00000010
#define EP_VID_INV	0x00000020

extern EP_STAT	ep_str_vid_set(const char *);


// character sequences (e.g., ASCII, ISO-8859-1, UTF-8)
extern struct epCharSequences
{
	const char	*lquote;	// left quote or '<<' character
	const char	*rquote;	// right quote or '>>' character
	const char	*copyright;	// copyright symbol
	const char	*degree;	// degree symbol
	const char	*micro;		// micro symbol
	const char	*plusminus;	// +/- symbol
	const char	*times;		// times symbol
	const char	*divide;	// divide symbol
	const char	*null;		// null symbol
	const char	*notequal;	// not equal symbol
	const char	*unprintable;	// non-printable character
	const char	*paragraph;	// paragraph symbol
	const char	*section;	// section symbol
	const char	*notsign;	// "not" symbol
	const char	*infinity;	// infinity symbol
} *EpChar;

extern EP_STAT	ep_str_char_set(const char *);

// function to adjust alternate printing
extern uint32_t	(*EpStPrintfAltprFunc)(int *, char *, char *);


#ifndef EP_OSCF_HAS_STRLCPY
# define EP_OSCF_HAS_STRLCPY	1
#endif

#if !EP_OSCF_HAS_STRLCPY
extern size_t	strlcpy(char *dst, const char *src, size_t n);
extern size_t	strlcat(char *dst, const char *src, size_t n);
#endif

#define ep_str_lcpy(dst, src, siz)	strlcpy(dst, src, siz)
#define ep_str_lcat(dst, src, siz)	strlcat(dst, src, siz)
#define ep_str_casecmp(s1, s2)		strcasecmp(s1, s2)
#define ep_str_ncasecmp(s1, s2, l)	strncasecmp(s1, s2, l)

extern size_t	ep_str_lcpyn(char *dst, size_t siz, ...);
extern size_t	ep_str_lcatn(char *dst, size_t siz, ...);

__END_DECLS
#endif /* _EP_STRING_H_ */
