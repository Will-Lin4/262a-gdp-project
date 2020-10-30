/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  ----- BEGIN LICENSE BLOCK -----
**	GDP: Global Data Plane Support Library
**	From the Ubiquitous Swarm Lab, 490 Cory Hall, U.C. Berkeley.
**
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
*/

//------------------------------------------------------------------------------
//	Added for C++ compability
extern "C"{
//------------------------------------------------------------------------------
#define EP_OSCF_USE_ZEROCONF		0	// force zeroconf off for now

#ifndef EP_OSCF_USE_ZEROCONF
# ifdef __has_include
#  if __has_include("avahi-common/simple-watch.h")
#   define EP_OSCF_USE_ZEROCONF		1	// include Avahi headers
#  else
#   define EP_OSCF_USE_ZEROCONF		0	// no Avahi headers
#  endif
# endif
#endif
#ifndef GDP_OSCF_USE_ZEROCONF
# define GDP_OSCF_USE_ZEROCONF		1	// default to zeroconf on
#endif

#if GDP_OSCF_USE_ZEROCONF
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>
#include <avahi-common/llist.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

typedef struct zcinfo
{
	char			*address;		// text version of IP/DNS address
	uint16_t		port;			// port number
	char			*xinfo;			// extended information (GDPname of router)
	struct zcinfo	*info_next;
} zcinfo_t;

int gdp_zc_scan();
zcinfo_t **gdp_zc_get_infolist();
char *gdp_zc_addr_str(zcinfo_t **list);
int gdp_zc_free_infolist(zcinfo_t **list);

#endif // GDP_OSCF_USE_ZEROCONF
//------------------------------------------------------------------------------
//	Added for C++ compability
}
//------------------------------------------------------------------------------

/* vim: set noexpandtab : */
