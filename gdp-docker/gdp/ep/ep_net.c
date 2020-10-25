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

#include "ep.h"
#include "ep_net.h"

/*
**  Network support routines.
**
**	Right now these mostly have to do with endianness.
**
**	I assume that 64 bits are transmitted in pure big-endian format;
**	for example 0x000102030405060708090A0B0C0D0E0F is transmitted as
**	00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F.  On 32-bit
**	machines we might have a NUXI problem that would transmit as
**	08 09 0A 0B 0C 0D 0E 0F 00 01 02 03 04 05 06 07.
**
**	XXX THIS NEEDS MORE RESEARCH
*/

#ifndef EP_HWCF_64_BIT_NUXI_PROBLEM
# define EP_HWCF_64_BIT_NUXI_PROBLEM	0
#endif

uint64_t
_ep_net_swap64(uint64_t v)
{
	union
	{
		uint64_t	i64;
		uint32_t	i32[2];
	} a, b;

	a.i64 = v;

#if EP_HWCF_64_BIT_NUXI_PROBLEM
	b.i32[0] = ep_net_hton32(a.i32[0]);
	b.i32[1] = ep_net_hton32(a.i32[1]);
#else
	b.i32[0] = ep_net_hton32(a.i32[1]);
	b.i32[1] = ep_net_hton32(a.i32[0]);
#endif
	return b.i64;
}


int
_ep_net_swap_timespec(EP_TIME_SPEC *ts)
{
	uint32_t *p32;

	ts->tv_sec = ep_net_hton64(ts->tv_sec);
	ts->tv_nsec = ep_net_hton32(ts->tv_nsec);
	p32 = (uint32_t *) &ts->tv_accuracy;
	*p32 = ep_net_hton32(*p32);
	return 0;
}
