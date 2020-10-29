/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	GDP_BUF --- data buffers for the GDP
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

#include "gdp.h"
#include "gdp_buf.h"

#include <ep/ep_dbg.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.buf", "GDP buffer processing");

#define DIAGNOSE(cmd, istat)											\
			do															\
			{															\
				if (istat < 0 && ep_dbg_test(Dbg, 9))					\
				{														\
					char ebuf[40];										\
					(void) (0 == strerror_r(errno, ebuf, sizeof ebuf));	\
					ep_dbg_printf("gdp_buf_%s: stat %d: %s\n",			\
							cmd, istat, ebuf);							\
				}														\
			} while (false)

/*
**  Create a new buffer
*/

gdp_buf_t *
gdp_buf_new(void)
{
	return evbuffer_new();
}

/*
**  Reset the contents of a buffer.
**		Returns 0 on success, -1 on failure.
*/

int
gdp_buf_reset(gdp_buf_t *b)
{
	return evbuffer_drain(b, evbuffer_get_length(b));
}

/*
**  Free a buffer.
*/

void
gdp_buf_free(gdp_buf_t *b)
{
	evbuffer_free(b);
}

/*
**  Set a mutex for a buffer.
*/

void
gdp_buf_setlock(gdp_buf_t *buf, EP_THR_MUTEX *m)
{
	evbuffer_enable_locking(buf, m);
}

/*
**  Lock/unlock a buffer
*/

void
gdp_buf_lock(gdp_buf_t *buf)
{
	evbuffer_lock(buf);
}

void
gdp_buf_unlock(gdp_buf_t *buf)
{
	evbuffer_unlock(buf);
}

/*
**  Get the amount of data in a buffer.
*/

size_t
gdp_buf_getlength(const gdp_buf_t *buf)
{
	return evbuffer_get_length(buf);
}

/*
**  Read data from buffer.
**		Returns number of bytes actually read.
*/

size_t
gdp_buf_read(gdp_buf_t *buf, void *out, size_t sz)
{
	int istat = evbuffer_remove(buf, out, sz);
	DIAGNOSE("read", istat);
	return istat;
}

/*
**  Get a single byte from buffer.
**		Returns EOF if no data is available.
*/

int
gdp_buf_getchar(gdp_buf_t *buf)
{
	uint8_t cbuf[1];
	int istat = evbuffer_remove(buf, cbuf, 1);
	DIAGNOSE("getchar", istat);
	if (istat <= 0)
		return EOF;
	else
		return cbuf[0];
}

/*
**  "Peek" at data in a buffer.
**		Like read, but leaves the buffer intact.
*/

size_t
gdp_buf_peek(gdp_buf_t *buf, void *out, size_t sz)
{
	ssize_t s = evbuffer_copyout(buf, out, sz);
	DIAGNOSE("peek", (int) s);
	if (s < 0)
		return 0;
	return s;
}

/*
**  Remove data from a buffer (and discard it).
**		Returns 0 on success, -1 on failure.
*/

int
gdp_buf_drain(gdp_buf_t *buf, size_t sz)
{
	int istat = evbuffer_drain(buf, sz);
	DIAGNOSE("drain", istat);
	return istat;
}

/*
**  Return a pointer to the data in a buffer (kind of like peek w/o copy).
*/

unsigned char *
gdp_buf_getptr(gdp_buf_t *buf, size_t sz)
{
	return evbuffer_pullup(buf, sz);
}

/*
**  Write data to a buffer.
**		Returns 0 on success, -1 on failure.
*/

int
gdp_buf_write(gdp_buf_t *buf, const void *in, size_t sz)
{
	int istat = evbuffer_add(buf, in, sz);
	DIAGNOSE("write", istat);
	return istat;
}

/*
**  Do a "printf" on to the end of a buffer.
**		Returns the number of bytes added.
*/

int
gdp_buf_printf(gdp_buf_t *buf, const char *fmt, ...)
{
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = evbuffer_add_vprintf(buf, fmt, ap);
	va_end(ap);
	return res;
}

/*
**  Append the contents of one buffer onto another.
**		Returns 0 on success, -1 on failure.
**		This removes the data from ibuf and appends it to obuf.
*/

int
gdp_buf_move(gdp_buf_t *obuf, gdp_buf_t *ibuf, ssize_t sz)
{
	if (sz == -1)
		sz = gdp_buf_getlength(ibuf);
	return evbuffer_remove_buffer(ibuf, obuf, (size_t) sz);
}

/*
**  Duplicate the contents of one buffer into another.
**		Does not change ibuf.
*/

// helper routine to minimize #ifdefs
# ifndef LIBEVENT_USE_EVBUFFER_ADD_BUFFER_REFERENCE
#  define LIBEVENT_USE_EVBUFFER_ADD_BUFFER_REFERENCE	0
# endif

static int
_gdp_buf_raw_copy(gdp_buf_t *obuf, gdp_buf_t *ibuf)
{
	int istat = -1;

#if LIBEVENT_VERSION_NUMBER > 0x02010100	// 2.1.1-alpha
# if LIBEVENT_USE_EVBUFFER_ADD_BUFFER_REFERENCE
	// efficient, but has semantic quirks: "a buffer that has already been
	// the outbuf of one evbuffer_add_buffer_reference call cannot be the
	// inbuf of another"
	istat = evbuffer_add_buffer_reference(ibuf, obuf);
# else
	// Alternative implementation should evbuffer_add_buffer_reference not work.
	// This physically copies the data (twice --- ugh).
	int nleft = evbuffer_get_length(ibuf);
	struct evbuffer_ptr bufpos;

	evbuffer_ptr_set(ibuf, &bufpos, 0, EVBUFFER_PTR_SET);
	if (nleft == 0)
		istat = 0;
	while (nleft > 0)
	{
		char xbuf[4096];
		ev_ssize_t n_copied;

		n_copied = evbuffer_copyout_from(ibuf, &bufpos, xbuf, sizeof xbuf);
		if (n_copied <= 0)
		{
			istat = (int) n_copied;
			break;
		}

		istat = evbuffer_add(obuf, xbuf, n_copied);
		if (istat < 0)
			break;

		evbuffer_ptr_set(ibuf, &bufpos, (size_t) istat, EVBUFFER_PTR_ADD);
		nleft -= n_copied;
	}
# endif // LIBEVENT_USE_EVBUFFER_ADD_BUFFER_REFERENCE
#else // LIBEVENT_VERSION_NUMBER <= 0x02010100	(pre 2.1.1-alpha)
	// this implementation may be expensive if ibuf is large
	ssize_t buflen = evbuffer_get_length(ibuf);
	if (buflen == 0)
		return 0;
	unsigned char *p = evbuffer_pullup(ibuf, buflen);
	if (p != NULL)
		istat = evbuffer_add(obuf, p, buflen);
	else
	{
		ep_dbg_cprintf(Dbg, 1,
				"_gdp_buf_raw_copy: evbuffer_pullup failed, buflen = %zd\n",
				buflen);
		if (errno == 0)
			errno = ENOMEM;
	}
#endif

	return istat;
}

int
gdp_buf_copy(gdp_buf_t *obuf, gdp_buf_t *ibuf)
{
	int istat;

	istat = _gdp_buf_raw_copy(obuf, ibuf);
	DIAGNOSE("copy", istat);
	return istat;
}

/*
**  Create a duplicate of a buffer.
*/

gdp_buf_t *
gdp_buf_dup(gdp_buf_t *ibuf)
{
	gdp_buf_t *nbuf = gdp_buf_new();
	int istat;

	istat = _gdp_buf_raw_copy(nbuf, ibuf);
	DIAGNOSE("dup", istat);
	return nbuf;
}

/*
**  Dump buffer to a file (for debugging).
*/

void
gdp_buf_dump(gdp_buf_t *buf, FILE *fp)
{
	fprintf(fp, "gdp_buf @ %p: len=%zu\n",
			buf, gdp_buf_getlength(buf));
}

/*
**  Get a 16 bit signed int in network byte order from a buffer.
*/

int16_t
gdp_buf_get_int16(gdp_buf_t *buf)
{
	int16_t t;

	evbuffer_remove(buf, &t, sizeof t);
	return ntohl(t);
}

/*
**  Get a 16 bit unsigned int in network byte order from a buffer.
*/

uint16_t
gdp_buf_get_uint16(gdp_buf_t *buf)
{
	uint16_t t;

	evbuffer_remove(buf, &t, sizeof t);
	return ntohl(t);
}

/*
**  Get a 32 bit signed int in network byte order from a buffer.
*/

int32_t
gdp_buf_get_int32(gdp_buf_t *buf)
{
	uint32_t t;

	evbuffer_remove(buf, &t, sizeof t);
	return ntohl(t);
}

/*
**  Get a 32 bit unsigned int in network byte order from a buffer.
*/

uint32_t
gdp_buf_get_uint32(gdp_buf_t *buf)
{
	uint32_t t;

	evbuffer_remove(buf, &t, sizeof t);
	return ntohl(t);
}

/*
**  Get a 48 bit signed int in network byte order from a buffer.
*/

int64_t
gdp_buf_get_int48(gdp_buf_t *buf)
{
	int16_t h;
	uint32_t l;

	evbuffer_remove(buf, &h, sizeof h);
	evbuffer_remove(buf, &l, sizeof l);
	return ((int64_t) ntohs(h) << 32) | ((uint64_t) ntohl(l));
}

/*
**  Get a 48 bit unsigned int in network byte order from a buffer.
*/

uint64_t
gdp_buf_get_uint48(gdp_buf_t *buf)
{
	uint16_t h;
	uint32_t l;

	evbuffer_remove(buf, &h, sizeof h);
	evbuffer_remove(buf, &l, sizeof l);
	return ((uint64_t) ntohs(h) << 32) | ((uint64_t) ntohl(l));
}

/*
**  Get a 64 bit signed int in network byte order from a buffer.
*/

int64_t
gdp_buf_get_int64(gdp_buf_t *buf)
{
	int64_t t;
	static const int32_t num = 42;

	evbuffer_remove(buf, &t, sizeof t);
	if (ntohl(num) == num)
	{
		return t;
	}
	else
	{
		int32_t h = htonl((int32_t) (t >> 32));
		uint32_t l = htonl((uint32_t) (t & UINT64_C(0xffffffff)));

		return ((int64_t) l) << 32 | h;
	}
}

/*
**  Get a 64 bit unsigned int in network byte order from a buffer.
*/

uint64_t
gdp_buf_get_uint64(gdp_buf_t *buf)
{
	uint64_t t;
	static const int32_t num = 42;

	evbuffer_remove(buf, &t, sizeof t);
	if (ntohl(num) == num)
	{
		return t;
	}
	else
	{
		uint32_t h = htonl((uint32_t) (t >> 32));
		uint32_t l = htonl((uint32_t) (t & UINT64_C(0xffffffff)));

		return ((uint64_t) l) << 32 | h;
	}
}

/*
**  Get a time stamp in network byte order from a buffer.
*/

void
gdp_buf_get_timespec(gdp_buf_t *buf, EP_TIME_SPEC *ts)
{
	uint32_t t;

	ts->tv_sec = gdp_buf_get_uint64(buf);
	ts->tv_nsec = gdp_buf_get_uint32(buf);
	t = gdp_buf_get_uint32(buf);
	memcpy(&ts->tv_accuracy, &t, sizeof ts->tv_accuracy);
}


/*
**  Put a 32 bit signed integer to a buffer in network byte order.
*/

void
gdp_buf_put_int32(gdp_buf_t *buf, const int32_t v)
{
	int32_t t = htonl(v);
	int istat = evbuffer_add(buf, &t, sizeof t);
	DIAGNOSE("put_int32", istat);
}


/*
**  Put a 32 bit unsigned integer to a buffer in network byte order.
*/

void
gdp_buf_put_uint32(gdp_buf_t *buf, const uint32_t v)
{
	uint32_t t = htonl(v);
	int istat = evbuffer_add(buf, &t, sizeof t);
	DIAGNOSE("put_uint32", istat);
}


/*
**  Put a 48 bit signed integer to a buffer in network byte order.
*/

void
gdp_buf_put_int48(gdp_buf_t *buf, const int64_t v)
{
	uint16_t h = htons((v >> 32) & 0xffff);
	uint32_t l = htonl((uint32_t) (v & UINT64_C(0xffffffff)));
	int istat;
	istat = evbuffer_add(buf, &h, sizeof h);
	DIAGNOSE("put_int48", istat);
	istat = evbuffer_add(buf, &l, sizeof l);
	DIAGNOSE("put_int48", istat);
}


/*
**  Put a 48 bit unsigned integer to a buffer in network byte order.
*/

void
gdp_buf_put_uint48(gdp_buf_t *buf, const uint64_t v)
{
	uint16_t h = htons((v >> 32) & 0xffff);
	uint32_t l = htonl((uint32_t) (v & UINT64_C(0xffffffff)));
	int istat;
	istat = evbuffer_add(buf, &h, sizeof h);
	DIAGNOSE("put_uint48", istat);
	istat = evbuffer_add(buf, &l, sizeof l);
	DIAGNOSE("put_uint48", istat);
}


/*
**  Put a 64 bit signed integer to a buffer in network byte order.
*/

void
gdp_buf_put_int64(gdp_buf_t *buf, const int64_t v)
{
	static const int32_t num = 42;
	int istat;

	if (htonl(num) == num)
	{
		istat = evbuffer_add(buf, &v, sizeof v);
	}
	else
	{
		int64_t t = htonl(v & INT64_C(0xffffffff));
		t <<= 32;
		t |= (int64_t) htonl((int32_t) ((v >> 32)) & INT64_C(0xffffffff));
		istat = evbuffer_add(buf, &t, sizeof t);
	}
	DIAGNOSE("put_int64", istat);
}


/*
**  Put a 64 bit unsigned integer to a buffer in network byte order.
*/

void
gdp_buf_put_uint64(gdp_buf_t *buf, const uint64_t v)
{
	static const int32_t num = 42;
	int istat;

	if (htonl(num) == num)
	{
		istat = evbuffer_add(buf, &v, sizeof v);
	}
	else
	{
		uint64_t t = htonl(((uint32_t) v & UINT64_C(0xffffffff)));
		t <<= 32;
		t |= (uint64_t) htonl((uint32_t) (v >> 32));
		istat = evbuffer_add(buf, &t, sizeof t);
	}
	DIAGNOSE("put_uint64", istat);
}


/*
**  Put a time stamp to a buffer in network byte order.
*/

void
gdp_buf_put_timespec(gdp_buf_t *buf, EP_TIME_SPEC *ts)
{
	gdp_buf_put_uint64(buf, ts->tv_sec);
	gdp_buf_put_uint32(buf, ts->tv_nsec);
	gdp_buf_put_uint32(buf, *((uint32_t *) &ts->tv_accuracy));
}
