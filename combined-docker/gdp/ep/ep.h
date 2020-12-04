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

/*
**  EP.H -- general definitions for EP library.
**
**  	A lot of this is just to override some of the full library names.
**  	Some should probably be broken out into other headers.
*/

#ifndef _EP_H_
#define _EP_H_

// definitions required on RedHat C++ compilers
#define __STDC_CONSTANT_MACROS	// include <type>_C macros
#define __STDC_FORMAT_MACROS	// include PRI<type> macros

#include <sys/cdefs.h>
#include <ep/ep_conf.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <ep/ep_assert.h>
#include <ep/ep_stat.h>

//XXX hacks for non-libep settings
#define EP_SRC_ID(x)

__BEGIN_DECLS

/*
**  Initialization
*/

EP_STAT		ep_lib_init(uint32_t flags);

#define EP_LIB_USEPTHREADS	0x00000001	// turn on pthreads support

// the versions from ep_mem.h give you a bit more
void	*ep_mem_malloc(size_t);
void	*ep_mem_zalloc(size_t);
char	*ep_mem_strdup(const char *);
void	ep_mem_free(void *);

#define EP_GEN_DEADBEEF		((void *) 0xDEADBEEF)
// ideally this would check valid pointers for this architecture
#define EP_ASSERT_POINTER_VALID(p) \
				EP_ASSERT((p) != NULL && (p) != EP_GEN_DEADBEEF)

#define EP_UT_BITSET(bit, word)	(((bit) & (word)) != 0)

extern void		ep_adm_readparams(	// search for parameter files
				const char *name);	// name to search for
extern int		ep_adm_getintparam(	// get integer param value
				const char *name,	// name of param
				int def);		// default value
extern long		ep_adm_getlongparam(	// get long param value
				const char *name,	// name of param
				long def);		// default value
extern intmax_t		ep_adm_getintmaxparam(	// get intmax_t param value
				const char *name,	// name of param
				intmax_t def);		// default value
extern bool		ep_adm_getboolparam(	// get boolean param value
				const char *name,	// name of param
				bool def);		// default value
extern const char	*ep_adm_getstrparam(	// get string param value
				const char *name,	// name of param
				const char *def);	// default value
extern void		ep_adm_setparam(	// set a single parameter
				const char *name,	// name of param
				const char *val);	// value
extern void		ep_adm_parseline(	// same, but string syntax
				char *lbuf);		// "name=value"

extern FILE		*ep_fopen_smem(		// open a static memory buffer
				void *buf,		// buffer
				size_t bufsz,		// size of buffer
				const char *mode);	// mode, e.g., r, w
extern FILE		*ep_fopen_styled(	// open output with font/colors
				FILE *underlying,	// underlying output
				const char *so,		// "shift out" string
				const char *si);	// "shift in" string
extern FILE		*ep_fopen_syslog(	// open a syslog channel
				int pri);		// message priority
extern size_t		ep_fread_unlocked(	// unlocked version of fread
				void *buf,		// buffer area
				size_t sz,		// size of one item
				size_t n,		// number of items
				FILE *fp);		// file to read
extern size_t		ep_fwrite_unlocked(	// unlocked version of fwrite
				void *buf,		// buffer area
				size_t sz,		// size of one item
				size_t n,		// number of items
				FILE *fp);		// file to write

__END_DECLS

#endif // _EP_H_
