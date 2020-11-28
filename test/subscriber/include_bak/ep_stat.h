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
**  Extended Status modules
**  	Based on libep, (c) 2012 Eric P. Allman
**
**	Status codes are integers cast to a struct so that they aren't
**	type compatible.  They can return integers on success, but not
**	to the full range of the machine; in particular, they can't
**	return pointers.
**
**	The top few bits encode a severity so the caller can easily
**	determine success or failure with a bit mask.  Failures are
**	further subdivided --- for example, there is a range
**	corresponding to errno codes.
**
**	Status codes are four parts: a severity, a registry, a
**	module, and a detail.  These should be considered opaque
**	by applications.
**
**	The actual implementation (at the moment) looks as follows:
**
**	+-----+---------------------+----------+-------------+
**	| sev |      registry       |  module  |   detail    |
**	+-----+---------------------+----------+-------------+
**	 3 bit        11 bits          8 bits     10/42 bits
**
**	Severities with the top bit being zero indicate success, and
**	the rest of the word can convey arbitary information.  If the
**	top bit of the severity is one there is an error with severity:
**	    EP_STAT_SEV_WARN --- warning or temporary error that is likely
**	    	to recover by itself, e.g., connection refused.
**	    EP_STAT_SEV_ERROR --- ordinary permanent error.
**	    EP_STAT_SEV_SEVERE --- severe error that requires immediate
**	    	recovery, e.g., out of memory.
**	    EP_STAT_SEV_ABORT --- internal error detected, such
**	    	as a software bug, assertion error, etc.  Recovery
**	    	generally not possible.
**
**	The registry is global, and must be assigned through a central
**	authority.  As a special case, the "local" registry is for use
**	within a single program (as opposed to libraries).
**
**	Normally a registry would allocate different modules for
**	different libraries or applications (obviously applications
**	can overlap), and detail is for the use of that module or
**	application.  The module name consisting of all 1 bits
**	(0xff) is reserved for internal use.
**
**	As a special case, a severity code of zero (OK) can also
**	be used to encode an integer return value of up to 2^31
**	(2^63 on 64-bit machines).
*/

#ifndef _EP_STAT_H
# define _EP_STAT_H
# include <ep/ep.h>
//------------------------------------------------------------------------------
//	Added for C++ compability
extern "C"{
//------------------------------------------------------------------------------
__BEGIN_DECLS

# include <limits.h>

typedef struct _ep_stat
{
	uint32_t	code;
} EP_STAT;

#define _EP_STAT_SEVBITS	3
#define _EP_STAT_REGBITS	11
#define _EP_STAT_MODBITS	8

//#if LONG_MAX == INT32_MAX
# define _EP_STAT_DETBITS	10
//#else
//# define _EP_STAT_DETBITS	42
//#endif

#define EP_STAT_MAX_REGISTRIES	((1U << _EP_STAT_REGBITS) - 1)
#define EP_STAT_MAX_MODULES	((1U << _EP_STAT_MODBITS) - 1)
#define EP_STAT_MAX_DETAIL	((1U << _EP_STAT_DETBITS) - 1)

#define _EP_STAT_MODSHIFT	_EP_STAT_DETBITS
#define _EP_STAT_REGSHIFT	(_EP_STAT_MODSHIFT + _EP_STAT_MODBITS)
#define _EP_STAT_SEVSHIFT	(_EP_STAT_REGSHIFT + _EP_STAT_REGBITS)

#define EP_STAT_SEV_OK		(0)	// everything OK (also 1, 2, and 3)
#define EP_STAT_SEV_WARN	(4)	// warning or temp error, may work later
#define EP_STAT_SEV_ERROR	(5)	// normal error
#define EP_STAT_SEV_SEVERE	(6)	// severe error, should back out
#define EP_STAT_SEV_ABORT	(7)	// internal error

// constructors for status code
#define EP_STAT_NEW(s, r, m, d) \
			((EP_STAT) { ((((s) & ((1U << _EP_STAT_SEVBITS) - 1)) << _EP_STAT_SEVSHIFT) | \
				      (((r) & ((1U << _EP_STAT_REGBITS) - 1)) << _EP_STAT_REGSHIFT) | \
				      (((m) & ((1U << _EP_STAT_MODBITS) - 1)) << _EP_STAT_MODSHIFT) | \
				      (((d) & ((1U << _EP_STAT_DETBITS) - 1)))) } )

// routines to extract pieces of error codes
#define EP_STAT_SEVERITY(c)	(((c).code >> _EP_STAT_SEVSHIFT) & ((1U << _EP_STAT_SEVBITS) - 1))
#define EP_STAT_REGISTRY(c)	(((c).code >> _EP_STAT_REGSHIFT) & ((1U << _EP_STAT_REGBITS) - 1))
#define EP_STAT_MODULE(c)	(((c).code >> _EP_STAT_MODSHIFT) & ((1U << _EP_STAT_MODBITS) - 1))
#define EP_STAT_DETAIL(c)	(((c).code                     ) & ((1U << _EP_STAT_DETBITS) - 1))

// predicates to query the status severity
#define EP_STAT_ISOK(c)		((EP_STAT_SEVERITY(c) & 0x4) == EP_STAT_SEV_OK)
#define EP_STAT_ISWARN(c)	(EP_STAT_SEVERITY(c) == EP_STAT_SEV_WARN)
#define EP_STAT_ISERROR(c)	(EP_STAT_SEVERITY(c) == EP_STAT_SEV_ERROR)
#define EP_STAT_ISFAIL(c)	(EP_STAT_SEVERITY(c) >= EP_STAT_SEV_ERROR)
#define EP_STAT_ISSEVERE(c)	(EP_STAT_SEVERITY(c) >= EP_STAT_SEV_SEVERE)
#define EP_STAT_ISABORT(c)	(EP_STAT_SEVERITY(c) >= EP_STAT_SEV_ABORT)

// compare two status codes for equality
#define EP_STAT_IS_SAME(a, b)	((a).code == (b).code)

// casting to and from int
#define EP_STAT_TO_INT(s)	((s).code)
#define EP_STAT_FROM_INT(i)	((EP_STAT) { (uint32_t) (i) })

// error checking quick routine, e.g., EP_STAT_CHECK(stat, break);
#define EP_STAT_CHECK(st, failure) \
		{ \
			if (!EP_STAT_ISOK(st)) \
				{ failure; } \
		}

// functions
extern EP_STAT	ep_stat_from_errno(int uerrno);

/**********************************************************************
**
**  Modules specific to EPLIB registry
*/

#include <ep/ep_registry.h>

#define EP_STAT_MOD_GENERIC	0	// basic multi-use errors
#define EP_STAT_MOD_CRYPTO	1	// cryptographic primitives
#define EP_STAT_MOD_ERRNO	0x0FE	// corresponds to errno codes

// common status code definitions
#define _EP_STAT_INTERNAL(sev, mod, code) \
		EP_STAT_NEW(EP_STAT_SEV_ ## sev, EP_REGISTRY_EPLIB, mod, code)

#include <ep/ep_statcodes.h>

struct ep_stat_to_string
{
	EP_STAT		estat;		// status code
	const char	*estr;		// string representation
};

// register stat code to string mappings
void		ep_stat_reg_strings(
			struct ep_stat_to_string *);

// return string representation of status
char		*ep_stat_tostr(
			EP_STAT estat,
			char *buf,
			size_t bsize);
char		*ep_stat_tostr_terse(
			EP_STAT estat,
			char *buf,
			size_t bsize);

// return string representation of severity (in natural language)
const char	*ep_stat_sev_tostr(
			int sev);

__END_DECLS
//------------------------------------------------------------------------------
//	Added for C++ compability
}
//------------------------------------------------------------------------------
#endif // _EP_STAT_H
