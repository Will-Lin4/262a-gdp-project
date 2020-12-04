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

#ifndef _EP_HASH_H_
#define _EP_HASH_H_
#include <ep/ep.h>
__BEGIN_DECLS

/***********************************************************************
**
**  HASH HANDLING
**
*/

typedef struct EP_HASH		EP_HASH;

typedef void	(*EP_HASH_FORALL_FUNCP)(
			size_t keylen,
			const void *key,
			const void *val,
			va_list av);
typedef int	(*EP_HASH_HASH_FUNCP)(
			const EP_HASH *const hash,
			const size_t keylen,
			const void *key);

extern EP_HASH	*ep_hash_new(
			const char *name,
			EP_HASH_HASH_FUNCP hfunc,
			int tabsize);
extern void	ep_hash_free(
			EP_HASH *hash);
extern const void
		*ep_hash_search(
			EP_HASH *hash,
			size_t keylen,
			const void *key);
extern const void
		*ep_hash_insert(
			EP_HASH *hash,
			size_t keylen,
			const void *key,
			const void *val);
extern const void
		*ep_hash_delete(
			EP_HASH *hash,
			size_t keylen,
			const void *key);
extern void	ep_hash_forall(
			EP_HASH *hash,
			EP_HASH_FORALL_FUNCP func,
			...);

#define EP_HASH_DEFHFUNC	((EP_HASH_HASH_FUNCP) NULL)
#define EP_HASH_DEFHSIZE	0

__END_DECLS
#endif //_EP_HASH_H_
