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
//  FUNCTION LISTS
//
////////////////////////////////////////////////////////////////////////

#ifndef _EP_FUNCLIST_H_
#define _EP_FUNCLIST_H_

#include <ep/ep.h>
__BEGIN_DECLS

// the function list itself (contents are private)
typedef struct EP_FUNCLIST	EP_FUNCLIST;

// the type of a function passed to the funclist
typedef void		(EP_FUNCLIST_FUNC)(void *closure, void *arg);

// the funclist primitives
extern EP_FUNCLIST	*ep_funclist_new(
				const char *name);
extern void		ep_funclist_free(
				EP_FUNCLIST *flp);
extern void		ep_funclist_push(
				EP_FUNCLIST *flp,
				EP_FUNCLIST_FUNC *func,
				void *closure);
extern void		ep_funclist_invoke(
				EP_FUNCLIST *flp,
				void *arg);

__END_DECLS
#endif //_EP_FUNCLIST_H_
