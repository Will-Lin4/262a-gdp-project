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

#ifndef _EP_HEXDUMP_H_
#define _EP_HEXDUMP_H_
#include <ep/ep.h>
__BEGIN_DECLS

/*
**  Print binary area to file for human consumption.
**
**	The offset is just added to the printed offset.  For example,
**	if you are printing starting at offset 16 in a larger buffer,
**	this will cause the labels to start at 16 rather than zero.
*/

void	ep_hexdump(
		const void *bufp,	// buffer to print
		size_t buflen,		// length of buffer
		FILE *fp,		// file to print to
		int format,		// format (see below)
		size_t offset);		// starting offset of bufp

// format flags
#define EP_HEXDUMP_HEX		0	// no special formatting
#define EP_HEXDUMP_ASCII	0x0001	// show ASCII equivalent
#define EP_HEXDUMP_TERSE	0x0002	// minimal formating

__END_DECLS
#endif //_EP_HEXDUMP_H_
