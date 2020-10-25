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

#ifndef _EP_XLATE_H_
#define _EP_XLATE_H_

#include <ep/ep.h>
__BEGIN_DECLS

#define EP_XLATE_PERCENT	0x00000001	// translate %xx like SMTP/URLs
#define EP_XLATE_BSLASH		0x00000002	// translate \ like C
#define EP_XLATE_AMPER		0x00000004	// translate &name; like HTML
#define EP_XLATE_PLUS		0x00000008	// translate +xx like DNSs
#define EP_XLATE_EQUAL		0x00000010	// translate =xx like Q-P
#define EP_XLATE_8BIT		0x00000100	// encode 8-bit chars
#define EP_XLATE_NPRINT		0x00000200	// encode non-printable chars

extern int	ep_xlate_in(
			const void *in,
			unsigned char *out,
			size_t olen,
			char stopchar,
			uint32_t how);

extern int	ep_xlate_out(
			const void *in,
			size_t ilen,
			FILE *osp,
			const char *forbid,
			uint32_t how);
__END_DECLS
#endif // _EP_XLATE_H_
