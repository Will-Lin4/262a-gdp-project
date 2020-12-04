/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	I/O CHANNEL HANDLING
**		This communicates between the client and the routing layer.
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
#include "gdp_chan.h"
#include "gdp_priv.h"
#include "gdp_zc_client.h"

#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <ep/ep_hexdump.h>
#include <ep/ep_log.h>
#include <ep/ep_prflags.h>
#include <ep/ep_string.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/signal.h>
#include <sys/socket.h>

#include <netinet/tcp.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.chan", "GDP channel processing");
#if GDP_DEBUG_EXTREME_TESTS
static EP_DBG	DbgT1 = EP_DBG_INIT(".test.gdp.chan.seqno", "TEST: GDP channel seqno randomization");
#endif

// protocol version number in layer 4 (transport) PDU
#define GDP_CHAN_PROTO_VERSION	4

static struct event_base	*EventBase;
static EP_STAT				chan_reopen(gdp_chan_t *);

/*
**  Channel internal structure
*/

struct gdp_chan
{
	EP_THR_MUTEX			mutex;			// data structure lock
	EP_THR_COND				cond;			// wake up after state change
	int16_t					state;			// current state of channel
	uint16_t				flags;			// status flags
	struct bufferevent		*bev;			// associated bufferevent (socket)
	char					*router_addr;	// text version of router address
	gdp_name_t				gdpname;		// GDPname of the router
	gdp_chan_x_t			*cdata;			// arbitrary user data

	// callbacks
	gdp_chan_recv_cb_t		*recv_cb;		// receive callback
	gdp_chan_send_cb_t		*send_cb;		// send callback
	gdp_chan_ioevent_cb_t	*ioevent_cb;	// close/error/eof callback
	gdp_chan_router_cb_t	*router_cb;		// router event callback
	gdp_chan_advert_func_t	*advert_cb;		// advertising function
};

/* Channel states */
#define GDP_CHAN_UNCONNECTED	0		// channel is not connected yet
#define GDP_CHAN_CONNECTING		1		// connection being initiated
#define GDP_CHAN_CONNECTED		2		// channel is connected and active
#define GDP_CHAN_ERROR			3		// channel has had error
#define GDP_CHAN_CLOSING		4		// channel is closing


//XXX following needs to be changed if ADDR_FMT != 0
// magic, hdrlen, type, ttl, seq_mf_foff, fraglen, paylen, dst, src
#define MIN_HEADER_LENGTH	(1 + 1 + 1 + 1 + 4 + 2 + 2 + 32 + 32)
#define MAX_HEADER_LENGTH	(255 * 4)


/*
**  _GDP_CHAN_INIT --- initialize channel subsystem
*/

EP_STAT
_gdp_chan_init(
		struct event_base *evbase,
		void *options_unused)
{
	if (evbase == NULL)
		return EP_STAT_ABORT;
	EventBase = evbase;
	return EP_STAT_OK;
}


/*
**  Lock and Unlock a channel
*/

void
_gdp_chan_lock(gdp_chan_t *chan)
{
	ep_thr_mutex_lock(&chan->mutex);
}

void
_gdp_chan_unlock(gdp_chan_t *chan)
{
	ep_thr_mutex_unlock(&chan->mutex);
}


/*
**  Read and decode fixed PDU header
**		On return, the header has been consumed from the input but
**			the complete payload is still in the input buffer.
**			The payload length is returned through plenp.
**		Returns GDP_STAT_KEEP_READING and leaves the header in the
**			input buffer if the entire payload is not yet in memory.
**		Returns GDP_STAT_NAK_NOROUTE if the router cannot find a
**			path to the destination.
*/

static EP_PRFLAGS_DESC	L4Flags[] =
{
	// address type portion
	{ GDP_PKT_ADDR_TYPE_2FULL,	GDP_PKT_ADDR_TYPE_MASK,	"2FULL"			},

	// packet type portion
	{ GDP_PKT_TYPE_REGULAR,		GDP_PKT_TYPE_MASK,		"REGULAR"		},
	{ GDP_PKT_TYPE_FORWARD,		GDP_PKT_TYPE_MASK,		"FORWARD"		},
	{ GDP_PKT_TYPE_ADVERTISE,	GDP_PKT_TYPE_MASK,		"ADVERTISE"		},
	{ GDP_PKT_TYPE_WITHDRAW,	GDP_PKT_TYPE_MASK,		"WITHDRAW"		},
	{ GDP_PKT_TYPE_NAK_NOROUTE,	GDP_PKT_TYPE_MASK,		"NAK_NOROUTE"	},
	{ GDP_PKT_TYPE_SEQ_PACKET,	GDP_PKT_TYPE_MASK,		"SEQ_PACKET"	},
	{ GDP_PKT_TYPE_NAK_PACKET,	GDP_PKT_TYPE_MASK,		"NAK_PACKET"	},
	{ GDP_PKT_TYPE_ACK_PACKET,	GDP_PKT_TYPE_MASK,		"ACK_PACKET"	},

	// flags portion
	{ GDP_PKT_TYPE_RELIABLE,	GDP_PKT_TYPE_RELIABLE,	"RELIABLE"		},
	{ GDP_PKT_TYPE_SSEQ,		GDP_PKT_TYPE_SSEQ,		"SSEQ"			},
	{ 0,						0,						NULL			}
};

static EP_STAT
read_header(gdp_chan_t *chan,
		gdp_buf_t *ibuf,
		gdp_name_t *src,
		gdp_name_t *dst,
		gdp_seqno_t *seqnop,
		size_t *plenp)
{
	uint8_t *pbp = gdp_buf_getptr(ibuf, MIN_HEADER_LENGTH);
	size_t hdr_len = 0;
	size_t payload_len = 0;
	EP_STAT estat = EP_STAT_OK;

	if (pbp == NULL)
	{
		// fewer than MIN_HEADER_LENGTH bytes in buffer
		ep_dbg_cprintf(Dbg, 11, "read_header: pbp == NULL\n");
		estat = GDP_STAT_KEEP_READING;
		goto done;
	}

	if (ep_dbg_test(Dbg, 66))
	{
		ep_dbg_printf("read_header: initial header:\n");
		ep_hexdump(pbp, MIN_HEADER_LENGTH, ep_dbg_getfile(), EP_HEXDUMP_HEX, 0);
	}

	int ver;
	GET8(ver);				// PDU version number (offset 0)
	if (ver != GDP_CHAN_PROTO_VERSION)
	{
		ep_dbg_cprintf(Dbg, 1, "wrong protocol version %d (%d expected)\n",
				ver, GDP_CHAN_PROTO_VERSION);
		estat = GDP_STAT_PDU_VERSION_MISMATCH;

		// for lack of anything better, flush the entire input buffer
		gdp_buf_drain(ibuf, gdp_buf_getlength(ibuf));
		goto fail0;
	}
	GET8(hdr_len);			// header length / 4
	hdr_len &= 0x3F;		// top two bits reserved
	hdr_len *= 4;
	if (hdr_len < MIN_HEADER_LENGTH)
	{
		ep_dbg_cprintf(Dbg, 1,
				"read_header: short header, need %d got %zd\n",
				MIN_HEADER_LENGTH, hdr_len);
		estat = GDP_STAT_PDU_CORRUPT;
		goto done;
	}

	// if we don't yet have the whole header, wait until we do
	if (gdp_buf_getlength(ibuf) < hdr_len)
		return GDP_STAT_KEEP_READING;

	int flags;
	GET8(flags);			// type of service/flags/address format
	int ttl;
	GET8(ttl);				// time to live (ignored by endpoints)
	ttl &= 0x3f;
	uint32_t seq_mf_foff;
	GET32(seq_mf_foff);		// seqno, more frags bit, and frag offset
	uint16_t seqno = (seq_mf_foff >> GDP_PKT_SEQNO_SHIFT) & GDP_PKT_SEQNO_MASK;
	uint16_t frag_off = seq_mf_foff & GDP_PKT_SEQNO_FOFF_MASK;
	uint16_t frag_len;
	GET16(frag_len);		// fragment length
	GET16(payload_len);		// length of opaque payload (reassembled)
	if ((flags & GDP_PKT_ADDR_TYPE_MASK) == 0)
	{
		memcpy(dst, pbp, sizeof (gdp_name_t));
		pbp += sizeof (gdp_name_t);
		memcpy(src, pbp, sizeof (gdp_name_t));
		pbp += sizeof (gdp_name_t);
	}
	else
	{
		ep_dbg_cprintf(Dbg, 1,
				"read_header: unknown address format 0x%02x\n",
				flags & GDP_PKT_ADDR_TYPE_MASK);
		estat = GDP_STAT_PDU_CORRUPT;
		goto done;
	}

#if GDP_DEBUG_EXTREME_TESTS
	if (ep_dbg_test(DbgT1, 102))
	{
		// randomize sequence number to test recovery
		// (no need to be cryptographically secure, hence trivial algorithm)
		seqno = random() % GDP_SEQNO_BASE;
	}
#endif

	if (ep_dbg_test(Dbg, 55))
	{
		gdp_pname_t src_p, dst_p;
		ep_dbg_printf("read_header(%zd): ver %d ttl %d seqno %d off %d paylen %zd\n"
					"    src %s\n"
					"    dst %s\n"
					"    flags ",
				hdr_len, ver, ttl, seqno, frag_off, payload_len,
				gdp_printable_name(*src, src_p),
				gdp_printable_name(*dst, dst_p));
			ep_prflags(flags, L4Flags, NULL);
			ep_dbg_printf("\n");
	}

	// check for router meta-commands (type)
	if ((flags & GDP_PKT_TYPE_MASK) == GDP_PKT_TYPE_NAK_NOROUTE)
	{
		estat = GDP_STAT_NAK_NOROUTE;
		goto done;
	}
	else if ((flags & GDP_PKT_TYPE_MASK) != GDP_PKT_TYPE_REGULAR)
	{
		if (ep_dbg_test(Dbg, 1))
		{
			gdp_pname_t src_p, dst_p;
			ep_dbg_printf("read_header(%zd): unxpected PDU router type\n"
						"    ver %d ttl %d seqno %d off %d paylen %zd\n"
						"    src %s\n"
						"    dst %s\n"
						"    flags ",
					hdr_len, ver, ttl, seqno, frag_off, payload_len,
					gdp_printable_name(*src, src_p),
					gdp_printable_name(*dst, dst_p));
				ep_prflags(flags, L4Flags, NULL);
				ep_dbg_printf("\n");
		}
		estat = GDP_STAT_PDU_CORRUPT;
		goto done;
	}

	// XXX check for rational payload_len here? XXX

	// make sure entire PDU is in memory
	if (gdp_buf_getlength(ibuf) < hdr_len + payload_len)
		return GDP_STAT_KEEP_READING;

	// consume the header, but leave the payload
	gdp_buf_drain(ibuf, hdr_len);

done:
	if (EP_STAT_ISOK(estat))
	{
		estat = EP_STAT_FROM_INT(payload_len);
		*seqnop = seqno;
	}
	else
	{
		ep_dbg_cprintf(Dbg, 19, "read_header: draining %zd on error\n",
						hdr_len + payload_len);
		gdp_buf_drain(ibuf, hdr_len + payload_len);
		payload_len = 0;
	}

fail0:
	{
		char ebuf[100];
		ep_dbg_cprintf(Dbg, 32, "read_header: hdr %zd pay %zd stat %s\n",
				hdr_len, payload_len,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	*plenp = payload_len;
	return estat;
}


/*
**	CHAN_READ_CB --- data is available for reading from network socket
**
**		Minimal implementation: read in PDU and hand it to
**		processing routine.  If that processing is going to be
**		lengthy it should use a thread.
**
**		We insist that the entire PDU be in memory before passing
**		the cursor up.  To fix that we would need to associate the
**		cursor with a {src, dst, seqno} tuple, but our naive
**		implementation will never intersperse portions of messages,
**		so this is safe.
*/

static void
chan_read_cb(struct bufferevent *bev, void *ctx)
{
	EP_STAT estat;
	gdp_buf_t *ibuf = GDP_BUF_FROM_EVBUFFER(bufferevent_get_input(bev));
	gdp_chan_t *chan = (gdp_chan_t *) ctx;
	gdp_name_t src, dst;
	gdp_seqno_t seqno = 0;

	ep_dbg_cprintf(Dbg, 50, "chan_read_cb: fd %d, %zd bytes\n",
			bufferevent_getfd(bev), gdp_buf_getlength(ibuf));

	EP_ASSERT(bev == chan->bev);

	while (gdp_buf_getlength(ibuf) >= MIN_HEADER_LENGTH)
	{
		// get the transport layer header
		size_t payload_len;
		estat = read_header(chan, ibuf, &src, &dst, &seqno, &payload_len);

		// if we don't have enough input, wait for more (we'll be called again)
		if (EP_STAT_IS_SAME(estat, GDP_STAT_KEEP_READING))
			break;

		if (!EP_STAT_ISOK(estat))
		{
			// deliver routing error to upper level
			ep_dbg_cprintf(Dbg, 27, "chan_read_cb: sending to router_cb %p\n",
						chan->router_cb);
			if (chan->router_cb != NULL)
			{
				estat = (*chan->router_cb)(chan, src, dst, payload_len, estat);
			}
			else
			{
				ep_dbg_cprintf(Dbg, 1, "chan_read_cb: NULL router_cb\n");
				estat = GDP_STAT_NOT_IMPLEMENTED;
				gdp_buf_drain(ibuf, payload_len);
			}
		}

		// pass it to the L5 callback
		// note that if the callback is not set, the PDU is thrown away
		if (EP_STAT_ISOK(estat))
		{
			if (chan->recv_cb != NULL)
			{
				// call upper level processing
				estat = (*chan->recv_cb)(chan, src, dst, seqno,
									ibuf, payload_len);
			}
			else
			{
				// discard input
				ep_dbg_cprintf(Dbg, 1, "chan_read_cb: NULL recv_cb\n");
				estat = GDP_STAT_NOT_IMPLEMENTED;
				gdp_buf_drain(ibuf, payload_len);
			}
		}
		char ebuf[100];
		ep_dbg_cprintf(Dbg, 32, "chan_read_cb: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
}


/*
**	CHAN_EVENT_CB --- events or errors occur on network socket
*/

static EP_PRFLAGS_DESC	EventWhatFlags[] =
{
	{ BEV_EVENT_READING,	BEV_EVENT_READING,		"READING"			},
	{ BEV_EVENT_WRITING,	BEV_EVENT_WRITING,		"WRITING"			},
	{ BEV_EVENT_EOF,		BEV_EVENT_EOF,			"EOF"				},
	{ BEV_EVENT_ERROR,		BEV_EVENT_ERROR,		"ERROR"				},
	{ BEV_EVENT_TIMEOUT,	BEV_EVENT_TIMEOUT,		"TIMEOUT"			},
	{ BEV_EVENT_CONNECTED,	BEV_EVENT_CONNECTED,	"CONNECTED"			},
	{ GDP_IOEVENT_USER_CLOSE,	0xffff,				"USER_CLOSE"		},
	{ 0, 0, NULL }
};

static void
chan_event_cb(struct bufferevent *bev, short events, void *ctx)
{
	bool restart_connection = false;
	gdp_chan_t *chan = (gdp_chan_t *) ctx;
	uint32_t cbflags = 0;

	if (ep_dbg_test(Dbg, 10))
	{
		int sockerr = EVUTIL_SOCKET_ERROR();
		ep_dbg_printf("chan_event_cb[%d]: ", getpid());
		ep_prflags(events, EventWhatFlags, ep_dbg_getfile());
		ep_dbg_printf(", fd=%d , errno=%d %s\n",
				bufferevent_getfd(bev),
				sockerr, evutil_socket_error_to_string(sockerr));
	}

	EP_ASSERT(bev == chan->bev);

	if (EP_UT_BITSET(BEV_EVENT_CONNECTED, events))
	{
		// sometimes libevent says we're connected when we're not
		if (EVUTIL_SOCKET_ERROR() == ECONNREFUSED)
		{
			chan->state = GDP_CHAN_ERROR;
			cbflags |= GDP_IOEVENT_ERROR;
		}
		else
		{
			chan->state = GDP_CHAN_CONNECTED;
			cbflags |= GDP_IOEVENT_CONNECTED;
		}
		ep_thr_cond_broadcast(&chan->cond);
	}
	if (EP_UT_BITSET(BEV_EVENT_EOF, events))
	{
		gdp_buf_t *ibuf = GDP_BUF_FROM_EVBUFFER(bufferevent_get_input(bev));
		size_t l = gdp_buf_getlength(ibuf);

		ep_dbg_cprintf(Dbg, 1, "chan_event_cb[%d]: got EOF, %zu bytes left\n",
					getpid(), l);
		cbflags |= GDP_IOEVENT_EOF;
		restart_connection = true;
	}
	if (EP_UT_BITSET(BEV_EVENT_ERROR, events))
	{
		int sockerr = EVUTIL_SOCKET_ERROR();

		ep_dbg_cprintf(Dbg, 1, "chan_event_cb[%d]: error: %s\n",
				getpid(), evutil_socket_error_to_string(sockerr));
		cbflags |= GDP_IOEVENT_ERROR;
		restart_connection = true;
	}

	if (chan->ioevent_cb != NULL)
		(*chan->ioevent_cb)(chan, cbflags);

	// if we need to restart, let it run
	if (restart_connection)
	{
		EP_STAT estat;

		chan->state = GDP_CHAN_ERROR;
		ep_thr_cond_broadcast(&chan->cond);

		do
		{
			long delay = ep_adm_getlongparam("swarm.gdp.reconnect.delay", 1000L);
			if (delay > 0)
				ep_time_nanosleep(delay * INT64_C(1000000));
			estat = chan_reopen(chan);
		} while (!EP_STAT_ISOK(estat));
	}

	if (chan->state == GDP_CHAN_CONNECTED)
		(*chan->advert_cb)(chan, GDP_CMD_ADVERTISE, ctx);
}


/*
**  Helper for close, error, and eof handlers
*/

static EP_STAT
chan_do_close(gdp_chan_t *chan, int what)
{
	if (ep_dbg_test(Dbg, 7))
	{
		ep_dbg_printf("chan_do_close: chan %p what ", chan);
		ep_prflags(what, EventWhatFlags, NULL);
	}
	if (chan == NULL)
		return EP_STAT_ERROR;

	chan->state = GDP_CHAN_CLOSING;
	ep_thr_cond_broadcast(&chan->cond);
	_gdp_chan_flush(chan);
	if (chan->ioevent_cb != NULL)
		(*chan->ioevent_cb)(chan, what);
	if (chan->bev != NULL)
		bufferevent_free(chan->bev);
	chan->bev = NULL;
	if (chan->router_addr != NULL)
		ep_mem_free(chan->router_addr);
	ep_thr_cond_destroy(&chan->cond);
	ep_thr_mutex_destroy(&chan->mutex);
	ep_mem_free(chan);
	return EP_STAT_OK;
}


/*
**	_GDP_CHAN_OPEN_HELPER --- open channel to the routing layer
*/

static EP_STAT
chan_open_helper(
		gdp_chan_t *chan,
		void *adata)
{
	EP_STAT estat = EP_STAT_OK;
	char abuf[500] = "";
	char *host;
	char *port = NULL;

	// get the host:port info into abuf
	if (chan->router_addr != NULL && chan->router_addr[0] != '\0')
	{
		strlcpy(abuf, chan->router_addr, sizeof abuf);
	}
	else
	{
#if GDP_OSCF_USE_ZEROCONF
		if (ep_adm_getboolparam("swarm.gdp.zeroconf.enable", true))
		{
			ep_dbg_cprintf(Dbg, 1, "Trying Zeroconf:\n");

			if (gdp_zc_scan())
			{
				ep_dbg_cprintf(Dbg, 20, "... after gdp_zc_scan\n");
				zcinfo_t **list = gdp_zc_get_infolist();
				ep_dbg_cprintf(Dbg, 20, "... after gdp_zc_get_infolist: %p\n",
						list);
				if (list != NULL)
				{
					char *info = gdp_zc_addr_str(list);
					ep_dbg_cprintf(Dbg, 20, "... after gdp_zc_addr_str: %p\n",
							info);
					gdp_zc_free_infolist(list);
					ep_dbg_cprintf(Dbg, 20, "... after gdp_zc_free_infolist\n");
					if (info != NULL)
					{
						if (info[0] != '\0')
						{
							ep_dbg_cprintf(Dbg, 1, "Zeroconf found %s\n",
									info);
							strlcpy(abuf, info, sizeof abuf);
							strlcat(abuf, ";", sizeof abuf);
						}
						free(info);
					}
				}
			}
			else
				ep_dbg_cprintf(Dbg, 20, "gdp_zc_scan failed\n");
		}
#endif // GDP_OSCF_USE_ZEROCONF
		strlcat(abuf,
				ep_adm_getstrparam("swarm.gdp.routers", "127.0.0.1"),
				sizeof abuf);
	}

	ep_dbg_cprintf(Dbg, 28, "chan_open_helper(%s)\n", abuf);

	/*
	**  Sort-of ABNF syntax is:
	**
	**  addr-list =		addr-spec *( ";" addr-spec )
	**  addr-spec =		host-ip [ ":" port ] [ "/" gdpame ]
	**  host-ip =		dns-name | v4-ip | "[" v6-ip "]"
	**	gdpname =		43*b64-char
	**	b64-char =		( "a" - "z" | "A" - "Z" | "0" - "9" | "-" | "_" )
	**
	**	Hopefully the missing non-terminals (port, etc.) will be obvious.
	*/

	// strip off addresses and try them
	estat = GDP_STAT_NOTFOUND;				// anything that is not OK
	{
		char *delim = abuf;
		do
		{
			char pbuf[10];
			char *gdppname = NULL;

			host = delim;						// beginning of address spec
			delim = strchr(delim, ';');			// end of address spec
			if (delim != NULL)
				*delim++ = '\0';

			host = &host[strspn(host, " \t\n\f\f\r")];	// strip whitespace
			if (*host == '\0')
				continue;						// empty spec

			ep_dbg_cprintf(Dbg, 1, "Trying %s\n", host);

			// if host is an IPv6 literal it may have colons
			char *sep = host;
			if (*host == '[')
			{
				// IPv6 literal --- strip [] to satisfy getaddrinfo
				host++;
				sep = &host[strcspn(host, "]/")];
				if (*sep == ']')
					*sep++ = '\0';
			}

			// extract port and gdpname
			sep = &sep[strcspn(sep, ":/")];
			if (*sep == ':')
			{
				*sep++ = '\0';
				port = sep;
				sep = &sep[strcspn(sep, "/")];
			}
			if (*sep == '/')
			{
				*sep++ = '\0';
				gdppname = sep;
				sep = &sep[strcspn(sep, " \t\n\f\f\r")];
			}

			// allow trailing white space
			if (isspace(*sep))
				*sep++ = '\0';

			// extract gdpname if it exists
			if (gdppname != NULL)
			{
				estat = gdp_internal_name(gdppname, chan->gdpname);
				if (!EP_STAT_ISOK(estat))
				{
					char ebuf[100];
					ep_dbg_cprintf(Dbg, 2,
							"chan_open_helper: bad gdpname in router spec:\n"
							"    %s\n    %s\n",
							gdppname, ep_stat_tostr(estat, ebuf, sizeof ebuf));
					continue;
				}
			}
			else
			{
				ep_dbg_cprintf(Dbg, 20,
						"chan_open_helper: no gdpname in router spec\n");
				memset(chan->gdpname, 0, sizeof chan->gdpname);
			}

			// if no explicit port number, use a default
			if (port == NULL || *port == '\0')
			{
				int portno;

				portno = ep_adm_getintparam("swarm.gdp.router.port",
								GDP_PORT_DEFAULT);
				snprintf(pbuf, sizeof pbuf, "%d", portno);
				port = pbuf;
			}

			ep_dbg_cprintf(Dbg, 20, "chan_open_helper: trying host %s port %s\n",
					host, port);

			// parsing done....  let's try the lookup
			struct addrinfo *res, *a;
			struct addrinfo hints;
			int r;

			memset(&hints, '\0', sizeof hints);
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			r = getaddrinfo(host, port, &hints, &res);
			if (r != 0)
			{
				// address resolution failed; try the next one
				switch (r)
				{
				case EAI_SYSTEM:
					estat = ep_stat_from_errno(errno);
					if (!EP_STAT_ISOK(estat))
						break;
					// ... fall through

				case EAI_NONAME:
					estat = EP_STAT_DNS_NOTFOUND;
					break;

				default:
					estat = EP_STAT_DNS_FAILURE;
				}
				ep_dbg_cprintf(Dbg, 1,
						"chan_open_helper: getaddrinfo(%s, %s) =>\n"
						"    %s\n",
						host, port, gai_strerror(r));
				continue;
			}

			// attempt connects on all available addresses
			_gdp_chan_lock(chan);
			for (a = res; a != NULL; a = a->ai_next)
			{
				// make the actual connection
				// it would be nice to have a private timeout here...
				evutil_socket_t sock = socket(a->ai_family, SOCK_STREAM, 0);
				if (sock < 0)
				{
					// bad news, but keep trying
					estat = ep_stat_from_errno(errno);
					ep_log(estat, "chan_open_helper: cannot create socket");
					continue;
				}

				// shall we disable Nagle algorithm?
				if (ep_adm_getboolparam("swarm.gdp.tcp.nodelay", false))
				{
					int enable = 1;
					if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
								(void *) &enable, sizeof enable) != 0)
					{
						estat = ep_stat_from_errno(errno);
						ep_log(estat, "chan_open_helper: cannot set TCP_NODELAY");
						// error not fatal, let's just go on
					}
				}
				if (connect(sock, a->ai_addr, a->ai_addrlen) < 0)
				{
					// connection failure
					estat = ep_stat_from_errno(errno);
					ep_dbg_cprintf(Dbg, 38,
							"chan_open_helper[%d]: connect failed: %s\n",
							getpid(), strerror(errno));
					close(sock);
					continue;
				}

				// success!  Make it non-blocking and associate with bufferevent
				ep_dbg_cprintf(Dbg, 39, "successful connect\n");
				estat = EP_STAT_OK;

				evutil_make_socket_nonblocking(sock);
				chan->bev = bufferevent_socket_new(EventBase, sock,
								BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE |
								BEV_OPT_DEFER_CALLBACKS |
								BEV_OPT_UNLOCK_CALLBACKS);
				bufferevent_setcb(chan->bev,
								chan_read_cb, NULL, chan_event_cb, chan);
				bufferevent_setwatermark(chan->bev,
								EV_READ, MIN_HEADER_LENGTH, 0);
				bufferevent_enable(chan->bev, EV_READ | EV_WRITE);

				// disable SIGPIPE so that we'll get an error instead of death
#ifdef SO_NOSIGPIPE
				int sockopt_set = 1;
				setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE,
						(void *) &sockopt_set, sizeof sockopt_set);
#else
				if (ep_adm_getboolparam("swarm.gdp.ignore.sigpipe", false))
				{
					// It isn't clear you want to do this, since this may
					// cause stdout writes to dead pipes to keep going in
					// programs that are not fastidious about error checking.
					// (Most programs do not check e.g., printf for error.)
					signal(SIGPIPE, SIG_IGN);
				}
#endif
				break;
			}

			_gdp_chan_unlock(chan);
			freeaddrinfo(res);

			if (EP_STAT_ISOK(estat))
			{
				// success
				break;
			}
		} while (delim != NULL);
	}

	// error cleanup and return
	if (!EP_STAT_ISOK(estat))
	{
		if (ep_dbg_test(Dbg, 2))
		{
			char ebuf[80];
			ep_dbg_printf("chan_open_helper[%d]: could not open channel: %s\n",
					getpid(), ep_stat_tostr(estat, ebuf, sizeof ebuf));
			//ep_log(estat, "chan_open_helper: could not open channel");
		}
	}
	else
	{
		if (ep_dbg_test(Dbg, 1))
		{
			ep_dbg_printf("chan_open_helper[%d]: talking to router at %s:%s\n    (",
					getpid(), host, port);
			if (gdp_name_is_valid(chan->gdpname))
				gdp_print_name(chan->gdpname, ep_dbg_getfile());
			else
				ep_dbg_printf("no gdpname");
			ep_dbg_printf(")\n");
		}
		(*chan->advert_cb)(chan, GDP_CMD_ADVERTISE, adata);
	}
	return estat;
}


/*
**  _GDP_CHAN_OPEN --- open a channel
*/

EP_STAT
_gdp_chan_open(
		const char *router_addr,
		void *qos,
		gdp_chan_recv_cb_t *recv_cb,
		gdp_chan_send_cb_t *send_cb,
		gdp_chan_ioevent_cb_t *ioevent_cb,
		gdp_chan_router_cb_t *router_cb,
		gdp_chan_advert_func_t *advert_func,
		gdp_chan_x_t *cdata,
		gdp_chan_t **pchan)
{
	EP_STAT estat;
	gdp_chan_t *chan;

	ep_dbg_cprintf(Dbg, 11, "_gdp_chan_open(%s)\n", router_addr);

	// allocate a new channel structure
	chan = (gdp_chan_t *) ep_mem_zalloc(sizeof *chan);
	ep_thr_mutex_init(&chan->mutex, EP_THR_MUTEX_DEFAULT);
	ep_thr_mutex_setorder(&chan->mutex, GDP_MUTEX_LORDER_CHAN);
	ep_thr_cond_init(&chan->cond);
	chan->state = GDP_CHAN_CONNECTING;
	chan->recv_cb = recv_cb;
	chan->send_cb = send_cb;			//XXX unused at this time
	chan->ioevent_cb = ioevent_cb;
	chan->router_cb = router_cb;
	chan->advert_cb = advert_func;
	chan->cdata = cdata;
	if (router_addr != NULL)
		chan->router_addr = ep_mem_strdup(router_addr);

	estat = chan_open_helper(chan, NULL);

	if (EP_STAT_ISOK(estat))
		*pchan = chan;
	else
	{
		ep_app_message(estat, "Cannot open connection to GDP");
		chan_do_close(chan, BEV_EVENT_ERROR);
	}
	return estat;
}


/*
**  CHAN_REOPEN --- re-open a channel (e.g., on router failure)
*/

static EP_STAT
chan_reopen(gdp_chan_t *chan)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 12, "chan_reopen: %p\n	 advert_cb = %p\n",
			chan, chan->advert_cb);

	// close the (now dead) bufferevent
	if (chan->bev != NULL)
		bufferevent_free(chan->bev);
	chan->bev = NULL;
	estat = chan_open_helper(chan, NULL);
	return estat;
}


/*
**	_GDP_CHAN_CLOSE --- close a channel (user-driven)
*/

EP_STAT
_gdp_chan_close(gdp_chan_t *chan)
{
	return chan_do_close(chan, GDP_IOEVENT_USER_CLOSE);
}


/*
**  _GDP_CHAN_SEND --- send a message to a channel
*/

static EP_STAT
send_helper(gdp_chan_t *chan,
			gdp_target_t *target,
			gdp_name_t src,
			gdp_name_t dst,
			gdp_buf_t *payload,
			int tos)
{
	EP_STAT estat = EP_STAT_OK;
	int i;
	size_t payload_len = 0;
	uint16_t seqno = 0;			//FIXME: should be useful

	if (payload != NULL)
		payload_len = gdp_buf_getlength(payload);

	if (ep_dbg_test(Dbg, 51))
	{
		gdp_pname_t src_printable;
		gdp_pname_t dst_printable;
		ep_dbg_printf("send_helper:\n\tsrc %s\n\tdst %s\n\tflags ",
				gdp_printable_name(src, src_printable),
				gdp_printable_name(dst, dst_printable));
		ep_prflags(tos, L4Flags, NULL);
		ep_dbg_printf("\n\tpayload %p ", payload);
		if (payload == NULL)
			ep_dbg_printf("(no payload)\n");
		else
		{
			ep_dbg_printf("len %zd\n", payload_len);
			ep_hexdump(gdp_buf_getptr(payload, payload_len), payload_len,
					ep_dbg_getfile(), EP_HEXDUMP_ASCII, 0);
		}
	}

	if (chan->bev == NULL)
	{
		ep_dbg_cprintf(Dbg, 1, "send_helper: no channel\n");
		return GDP_STAT_DEAD_DAEMON;
	}

	if (payload_len > UINT16_MAX)
	{
		ep_dbg_cprintf(Dbg, 1, "send_helper: payload_len = %zd, max %d\n",
				payload_len, UINT16_MAX);
		return GDP_STAT_PDU_TOO_LONG;
	}

	// build the header in memory
	char pb[MAX_HEADER_LENGTH];
	char *pbp = pb;

	PUT8(GDP_CHAN_PROTO_VERSION);		// version number
	PUT8(MIN_HEADER_LENGTH / 4);		// header length (= 72 / 4)
	PUT8(tos);							// flags / type of service
	PUT8(GDP_TTL_DEFAULT);				// time to live
	uint32_t seq_mf_foff = (seqno & GDP_PKT_SEQNO_MASK) << GDP_PKT_SEQNO_SHIFT;
	PUT32(seq_mf_foff);					// more frag bit, seqno, frag offset
	uint16_t frag_len = 0;
	PUT16(frag_len);					// length of this fragment
	PUT16(payload_len);					// length of opaque payload
	memcpy(pbp, dst, sizeof (gdp_name_t));	// destination address
	pbp += sizeof (gdp_name_t);
	memcpy(pbp, src, sizeof (gdp_name_t));	// source address
	pbp += sizeof (gdp_name_t);

	// now write header to the socket
	bufferevent_lock(chan->bev);
	EP_ASSERT((pbp - pb) == MIN_HEADER_LENGTH);
	if (ep_dbg_test(Dbg, 42))
	{
		ep_dbg_printf("send_helper: sending %zd octets:\n",
					payload_len + (pbp - pb));
		ep_hexdump(pb, pbp - pb, ep_dbg_getfile(), 0, 0);
		if (payload_len > 0)
		{
			ep_hexdump(gdp_buf_getptr(payload, payload_len), payload_len,
					ep_dbg_getfile(), EP_HEXDUMP_ASCII, pbp - pb);
		}
	}

	i = bufferevent_write(chan->bev, pb, pbp - pb);
	if (i < 0)
	{
		estat = GDP_STAT_PDU_WRITE_FAIL;
		goto fail0;
	}

	// and the payload
	if (payload_len > 0)
	{
		i = bufferevent_write_buffer(chan->bev, payload);
		if (i < 0)
		{
			estat = GDP_STAT_PDU_WRITE_FAIL;
			goto fail0;
		}
	}

fail0:
	if (!EP_STAT_ISOK(estat) && ep_dbg_test(Dbg, 4))
	{
		char ebuf[100];
		ep_dbg_printf("send_helper failure: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	bufferevent_unlock(chan->bev);
	return estat;
}

EP_STAT
_gdp_chan_send(gdp_chan_t *chan,
			gdp_target_t *target,
			gdp_name_t src,
			gdp_name_t dst,
			gdp_buf_t *payload,
			int tos)
{
	if (ep_dbg_test(Dbg, 42))
	{
		size_t l = evbuffer_get_length(payload);
		uint8_t *p = evbuffer_pullup(payload, l);
		ep_dbg_printf("_gdp_chan_send: sending PDU:\n");
		ep_hexdump(p, l, ep_dbg_getfile(), EP_HEXDUMP_ASCII, 0);
	}
	return send_helper(chan, target, src, dst, payload, tos);
}


EP_STAT
_gdp_chan_flush(gdp_chan_t *chan)
{
	// if we aren't running an event loop, do one pass to flush any
	// buffered output.
	extern bool GdpIoEventLoopRunning;

	if (!GdpIoEventLoopRunning)
	{
		int i = event_base_loop(_GdpIoEventBase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
		if (i < 0)
			return GDP_STAT_PDU_WRITE_FAIL;
	}
	return EP_STAT_OK;
}


/*
**  Advertising primitives
*/

EP_STAT
_gdp_chan_advertise(
			gdp_chan_t *chan,
			gdp_name_t gname,
			gdp_adcert_t *adcert,
			gdp_chan_advert_cr_t *challenge_cb,
			void *adata)
{
	EP_STAT estat = EP_STAT_OK;
	gdp_pname_t pname;

	ep_dbg_cprintf(Dbg, 39, "_gdp_chan_advertise(%s):\n",
			gdp_printable_name(gname, pname));

	estat = send_helper(chan, NULL, _GdpMyRoutingName, gname,
						NULL, GDP_PKT_TYPE_ADVERTISE);

	if (ep_dbg_test(Dbg, 21))
	{
		char ebuf[100];

		ep_dbg_printf("_gdp_chan_advertise => %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	return estat;
}


EP_STAT
_gdp_chan_withdraw(
			gdp_chan_t *chan,
			gdp_name_t gname,
			void *adata)
{
	EP_STAT estat;
	gdp_pname_t pname;

	ep_dbg_cprintf(Dbg, 39, "_gdp_chan_withdraw(%s)\n",
			gdp_printable_name(gname, pname));
//	gdp_buf_t *payload = gdp_buf_new();
//	gdp_buf_write(payload, gname, sizeof (gdp_name_t));
	estat = send_helper(chan, NULL, _GdpMyRoutingName, gname,
						NULL, GDP_PKT_TYPE_WITHDRAW);

	if (ep_dbg_test(Dbg, 21))
	{
		char ebuf[100];

		ep_dbg_printf("_gdp_chan_withdraw => %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	return estat;
}


EP_STAT
_gdp_chan_advert_flush(
			gdp_chan_t *chan)
{
	// eventually, flush out any pending piggybacked advertisements
	return EP_STAT_OK;
}


/*
**  _GDP_CHAN_GET_UDATA --- get user data from channel
*/

gdp_chan_x_t *
_gdp_chan_get_cdata(gdp_chan_t *chan)
{
	return chan->cdata;
}


/* vim: set noexpandtab : */
