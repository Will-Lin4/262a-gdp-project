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

////////////////////////////////////////////////////////////////////////
//
//  VERSION NUMBER
//
//	Patches shouldn't include new functionality, just bug fixes.
//
//	Used by ep_gen.h to make string and numeric versions.
//
////////////////////////////////////////////////////////////////////////

#ifndef _EP_VERSION_H_
#define _EP_VERSION_H_

#include <ep/ep.h>
__BEGIN_DECLS

// change these as necessary
#define EP_VERSION_MAJOR	3
#define EP_VERSION_MINOR	0
#define EP_VERSION_PATCH	1

#define __EP_STRING(x)		#x
#define __EP_VER_CONCAT(a, b, c)					\
		__EP_STRING(a) "." __EP_STRING(b) "." __EP_STRING(c)
#define EP_VER_STRING							\
		__EP_VER_CONCAT(EP_VERSION_MAJOR,			\
				EP_VERSION_MINOR,			\
				EP_VERSION_PATCH)

#define EP_VERSION	((EP_VERSION_MAJOR << 24) |			\
			 (EP_VERSION_MINOR << 16) |			\
			 (EP_VERSION_PATCH      ))

extern const char	EpVersion[];

__END_DECLS
#endif
