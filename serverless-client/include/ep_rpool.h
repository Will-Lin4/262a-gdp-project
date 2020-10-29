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
//  MEMORY ALLOCATION: RESOURCE POOLS
//
////////////////////////////////////////////////////////////////////////

#ifndef _EP_RPOOL_H_
#define _EP_RPOOL_H_
#include <ep/ep.h>
__BEGIN_DECLS

# include <ep/ep_mem.h>

typedef struct EP_RPOOL		EP_RPOOL;

typedef void			(EP_RPOOL_FREEFUNC)(void *, void *);

extern EP_RPOOL *ep_rpool_new(			// create new pool
			const char *name,		// name (debugging)
			const size_t quantum);		// allocation size
extern void	ep_rpool_free(			// free everything in pool
			EP_RPOOL *rp);			// rpool to free
extern void	*ep_rpool_malloc(		// allocate memory
			EP_RPOOL *rp,			// source rpool
			size_t nbytes);			// bytes to get
extern void	*ep_rpool_zalloc(		// allocate zeroed memory
			EP_RPOOL *rp,			// source rpool
			size_t nbytes);			// bytes to get
extern void	*ep_rpool_realloc(		// change allocation size
			EP_RPOOL *rp,			// source rpool
			void *emem,			// old memory
			size_t oldsize,			// size of old memory
			size_t newsize);		// size of new memory
extern char	*ep_rpool_istrdup(		// save string in rpool
			EP_RPOOL *rp,			// rpool to use
			const char *s,			// str to copy
			ssize_t slen,			// max length of s
			uint32_t flags,			// action modifiers
			const char *file,		// dbg: file name
			int line);			// dbg: line number
extern void	ep_rpool_mfree(			// free block of memory
			EP_RPOOL *rp,			// owner rpool
			void *p);			// space to free
//extern void	ep_rpool_attach(		// attach free func to rpool
//			EP_RPOOL *rp,			// target rpool
//			EP_RPOOL_FREEFUNC *freefunc,	// free function
//			void *arg);			// arg to free func
extern void	*ep_rpool_ialloc(		// allocate memory from pool
			EP_RPOOL *rp,			// source pool
			size_t nbytes,			// bytes to get
			uint32_t flags,			// action modifiers
			const char *file,		// dbg: file name
			int line);			// dbg: file number

# define ep_rpool_malloc(rp, size) \
				ep_rpool_ialloc(rp, size, 0, \
					_EP_MEM_FILE_LINE_)
# define ep_rpool_strdup(rp, s)	ep_rpool_istrdup(rp, s, -1, 0, \
					_EP_MEM_FILE_LINE_)
# define ep_rpool_strndup(rp, s) \
				ep_rpool_istrdup(rp, s, l, 0, \
					_EP_MEM_FILE_LINE_)

__END_DECLS
#endif //_EP_RPOOL_H_
