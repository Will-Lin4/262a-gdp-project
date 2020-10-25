/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	GDP_BUF.H --- data buffers for the GDP
**
**	----- BEGIN LICENSE BLOCK -----
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
**	----- END LICENSE BLOCK -----
*/

#ifndef _GDP_BUF_H_
#define _GDP_BUF_H_

#include <event2/event.h>
#include <event2/buffer.h>
#include <ep/ep_thr.h>

typedef struct evbuffer	gdp_buf_t;
#define GDP_BUF_FROM_EVBUFFER(x)		(x)

extern gdp_buf_t	*gdp_buf_new(void);			// create new buffer

extern int			gdp_buf_reset(				// empty a buffer
						gdp_buf_t *b);

extern void			gdp_buf_free(				// free buffer
						gdp_buf_t *b);

extern void			gdp_buf_setlock(			// associate mutex with buffer
						gdp_buf_t *buf,
						EP_THR_MUTEX *m);

extern void			gdp_buf_lock(				// lock a buffer
						gdp_buf_t *buf);

extern void			gdp_buf_unlock(				// unlock a buffer
						gdp_buf_t *buf);

extern size_t		gdp_buf_getlength(			// get length of buffer
						const gdp_buf_t *buf);

extern size_t		gdp_buf_read(				// read data from buffer
						gdp_buf_t *buf,
						void *out,
						size_t sz);

extern int			gdp_buf_getchar(			// read single byte from buffer
						gdp_buf_t *buf);

extern size_t		gdp_buf_peek(				// peek into buffer
						gdp_buf_t *buf,
						void *out,
						size_t sz);

extern int			gdp_buf_drain(				// remove data from buffer
						gdp_buf_t *buf,
						size_t sz);

extern unsigned char
					*gdp_buf_getptr(			// get pointer to buffer
						gdp_buf_t *buf,
						size_t sz);

extern int			gdp_buf_write(				// write data to buffer
						gdp_buf_t *buf,
						const void *in,
						size_t sz);

extern int			gdp_buf_move(				// move data between buffers
						gdp_buf_t *obuf,
						gdp_buf_t *ibuf,
						ssize_t sz);

extern int			gdp_buf_copy(				// duplicate data
						gdp_buf_t *obuf,
						gdp_buf_t *ibuf);

extern gdp_buf_t	*gdp_buf_dup(				// duplicate a buffer
						gdp_buf_t *buf);

extern int			gdp_buf_printf(
						gdp_buf_t *buf,
						const char *fmt, ...);

extern void			gdp_buf_dump(
						gdp_buf_t *buf,
						FILE *fp);

extern int16_t		gdp_buf_get_int16(
						gdp_buf_t *buf);

extern uint16_t		gdp_buf_get_uint16(
						gdp_buf_t *buf);

extern int32_t		gdp_buf_get_int32(
						gdp_buf_t *buf);

extern uint32_t		gdp_buf_get_uint32(
						gdp_buf_t *buf);

extern void			gdp_buf_put_int32(
						gdp_buf_t *buf,
						const int32_t v);

extern void			gdp_buf_put_uint32(
						gdp_buf_t *buf,
						const uint32_t v);

extern int64_t		gdp_buf_get_int48(
						gdp_buf_t *buf);

extern uint64_t		gdp_buf_get_uint48(
						gdp_buf_t *buf);

extern void			gdp_buf_put_int48(
						gdp_buf_t *buf,
						const int64_t v);

extern void			gdp_buf_put_uint48(
						gdp_buf_t *buf,
						const uint64_t v);

extern int64_t		gdp_buf_get_int64(
						gdp_buf_t *buf);

extern uint64_t		gdp_buf_get_uint64(
						gdp_buf_t *buf);

extern void			gdp_buf_put_int64(
						gdp_buf_t *buf,
						const int64_t v);

extern void			gdp_buf_put_uint64(
						gdp_buf_t *buf,
						const uint64_t v);

extern void			gdp_buf_get_timespec(
						gdp_buf_t *buf,
						EP_TIME_SPEC *ts);

extern void			gdp_buf_put_timespec(
						gdp_buf_t *buf,
						EP_TIME_SPEC *ts);

#endif // _GDP_BUF_H_
