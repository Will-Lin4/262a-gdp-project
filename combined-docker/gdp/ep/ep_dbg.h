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

#ifndef _EP_DEBUG_H_
#define _EP_DEBUG_H_

# include <ep/ep.h>
# include <stdarg.h>

__BEGIN_DECLS

/**************************  BEGIN PRIVATE  **************************/

// if the current generation is greater than that in the flag then some
// debug flag has been changed and the whole thing should be re-inited.

typedef struct EP_DBG	EP_DBG;

struct EP_DBG
{
	const char	*name;	// debug flag name
	int		level;	// current debug level
	const char	*desc;	// description
	int		gen;	// flag initialization generation
	EP_DBG		*next;	// initted flags, in case values change
};

extern int	__EpDbgCurGen;		// current generation

/***************************  END PRIVATE  ***************************/

// macros for use in applications
#define ep_dbg_level(f)		((f).gen == __EpDbgCurGen ?		\
				 (f).level :				\
				 ep_dbg_flaglevel(&f))
#define ep_dbg_test(f, l)	(ep_dbg_level(f) >= (l))

// support functions
extern int	ep_dbg_flaglevel(EP_DBG *f);

// creating a flag
#define EP_DBG_INIT(name, desc)						\
		{ name, -1, "@(#)$Debug: " name " = " desc " $", -1, NULL }

// initialization
extern void	ep_dbg_init(void);

// setting/fetching debug output file
extern void	ep_dbg_setfile(FILE *fp);
extern FILE	*ep_dbg_getfile(void);

// setting debug flags
extern void	ep_dbg_set(const char *s);
extern void	ep_dbg_setto(const char *pat, int lev);

// printing debug output (uses stddbg)
extern void EP_TYPE_PRINTFLIKE(1, 2)
		ep_dbg_printf(const char *fmt, ...);
extern void	ep_dbg_vprintf(const char *fmt, va_list av);

// print only if flag set
#define ep_dbg_cprintf(f, l, ...)	{if (ep_dbg_test(f, l)) \
						ep_dbg_printf(__VA_ARGS__);}

// stack backtrace to debug output
extern void	ep_dbg_backtrace(FILE *fp);

// crackarg parsing
extern EP_STAT	epCavDebug(const char *vp, void *rp);

__END_DECLS

#endif /*_EP_DEBUG_H_*/
