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

#ifndef _EP_NET_H_
#define _EP_NET_H_
#include <ep/ep.h>
__BEGIN_DECLS

#include <arpa/inet.h>

#include <ep/ep_time.h>

#define ep_net_hton16(v)	htons(v)
#define ep_net_ntoh16(v)	ntohs(v)
#define ep_net_hton32(v)	htonl(v)
#define ep_net_ntoh32(v)	ntohl(v)
#define ep_net_hton64(v)	((htonl(1) == 1) ? v : _ep_net_swap64(v))
#define ep_net_ntoh64(v)	((htonl(1) == 1) ? v : _ep_net_swap64(v))

#define ep_net_hton_timespec(v)	((htonl(1) == 1) ? 0 : _ep_net_swap_timespec(v))
#define ep_net_ntoh_timespec(v)	((htonl(1) == 1) ? 0 : _ep_net_swap_timespec(v))


extern uint64_t		_ep_net_swap64(uint64_t v);
extern int		_ep_net_swap_timespec(EP_TIME_SPEC *v);

__END_DECLS
#endif //_EP_NET_H_
