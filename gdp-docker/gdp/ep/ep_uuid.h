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

#ifndef _EP_UUID_H_
#define _EP_UUID_H_
__BEGIN_DECLS

// manifest constants
#define EP_UUID_INTERNAL	0	// use internal algorithm
#define EP_UUID_OSSP		1	// use OSSP interface
#define EP_UUID_BSD		2	// use BSD interface
#define EP_UUID_TSO		3	// use Ted T'so interface

#include <ep/ep.h>

#ifdef __FreeBSD__
# define EP_OSCF_UUID_TYPE	EP_UUID_BSD
#endif
#ifdef __APPLE__
# define EP_OSCF_UUID_TYPE	EP_UUID_TSO
#endif
#if defined(__linux__) && defined(__has_include)
# if __has_include("uuid/uuid.h")
#  define EP_OSCF_UUID_TYPE	EP_UUID_TSO
# endif
#endif

#ifndef EP_OSCF_UUID_TYPE
# define EP_OSCF_UUID_TYPE	EP_UUID_INTERNAL
#endif

#if EP_OSCF_UUID_TYPE == EP_UUID_BSD || EP_OSCF_UUID_TYPE == EP_UUID_OSSP
# include <uuid.h>
#elif EP_OSCF_UUID_TYPE == EP_UUID_TSO
# include <uuid/uuid.h>
#else	// internal
# include <ep/ep_crypto.h>
typedef uint8_t		uuid_t[16];
#endif


struct ep_uuid
{
#if EP_OSCF_UUID_TYPE == EP_UUID_OSSP
	uuid_t		*uu;			// system UUID representation
#else
	uuid_t		uu;			// system UUID representation
#endif
};

#define EP_UUID_STR_LEN		37		// external representation len
typedef struct ep_uuid	EP_UUID;		// internal (16 octet) format
typedef char		EP_UUID_STR[EP_UUID_STR_LEN];	// string representation

extern EP_STAT	ep_uuid_generate(		// generate new UUID
			EP_UUID *uu);
extern EP_STAT	ep_uuid_destroy(		// free UUID memory
			EP_UUID *uu);
extern EP_STAT	ep_uuid_tostr(			// make printable UUID
			EP_UUID *uu,
			EP_UUID_STR str);
extern EP_STAT	ep_uuid_parse(			// convert printable to internal
			EP_UUID *uu,
			EP_UUID_STR str);
extern EP_STAT	ep_uuid_clear(			// reset UUID to zero
			EP_UUID *uu);
extern bool	ep_uuid_is_null(		// test if UUID is zero
			EP_UUID *uu);
extern int	ep_uuid_compare(		// compare two UUIDs
			EP_UUID *u1,
			EP_UUID *u2);
extern bool	ep_uuid_equal(			// test if UUIDs are identical
			EP_UUID *u1,
			EP_UUID *u2);

__END_DECLS
#endif //_EP_UUID_H_
