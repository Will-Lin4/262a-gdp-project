/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**
**		The definitions in this file represent the API between the
**		Session Layer (dealing with commands, requests, and so forth)
**		and the Transport Layer (dealing with network reliability,
**		fragmentation/reassembly, etc.)
**
**		Anything private to the Transport Layer should not be in
**		this file.
**
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

/*
**	These headers describe the API between the network (in OSI terms,
**	"layer 4", i.e., the Transport layer) and the GDP library ("layer
**	5" or Session layer).  Details of the actual implementation
**	(including the on-the-wire format) are limited to gdp_chan.c.
*/

#ifndef _GDP_CHAN_H_
#define _GDP_CHAN_H_

#include "gdp.h"
#include "gdp_priv.h"

#define GDP_PORT_DEFAULT		8009	// default IP port
#define GDP_TTL_DEFAULT			15		// hops left



/*
**  Prototypes for type declarations.  These are for the most part
**  opaque, so the structure itself is defined where the datatype
**  is actually implemented.
*/

//typedef struct gdp_chan		gdp_chan_t;			// defined in gdp_priv.h
typedef struct gdp_chan_x		gdp_chan_x_t;		// for chan private "udata"
typedef struct gdp_target		gdp_target_t;		//XXX as yet undefined
typedef struct gdp_adcert		gdp_adcert_t;		// advertising cert


/*
**  On-the-Wire PDU Format
**
**		Portions of this packet are not used for the client-to-router
**		TCP interface, but are used in the router-to-router UDP
**		interface, and will be needed when the client-to-router
**		path switches to UDP.
**
**		All multi-byte fields are in network byte order (big-
**		endian).
**
**		off	len	meaning
**		---	---	-------
**		0	1	version (must be 4) [1]
**		1	1	reserved / header length in units of 32 bits (= H / 4) [2]
**		2	1	type / flags / address format [3]
**		3	1	time to live [4]
**		4	4	sequence number, "more fragments" bit, frag offset [5]
**		8	2	fragment length (= F) [6]
**		10	2	payload (SDU) length (= P) [6]
**				--- following depend on address format in octet 2 ---
**		12	4	flow id (optional) [7]
**		12	32	destination address (optional) [7]
**		44	32	source address (optional) [7]
**		?	?	for future use (probably options)
**		H	P	payload (SDU) (starts at offset given in octet 1)
**
**		[1] If the high order bit of the version is set, this is
**			reserved for router-to-router communication.  When the
**			client generates or sees a PDU, the high order bit must
**			be zero.  The remainder of a router-to-router PDU is not
**			defined here.
**
**		[2]	Only the bottom six bits are used to encode H.  The
**			top two bits are reserved.  They must be ignored when
**			reading the PDU and set to zero when generating it.
**
**		[3] The low-order three bits define the address fields.  If
**			zero, there are two 32-byte (256-bit) addresses for
**			destination and source respectively.  Other values are
**			reserved.
**
**			The next two bits are flag bits, as defined in the
**			#defines below.
**
**			The top three bits represent the PDU type:
**				0: regular traffic
**				1: forward this PDU to the destination, strip off
**					the header, and re-interpret the payload as
**					a new PDU (i.e., do source routing).
**				2: name advertisement
**				3: name withdrawal
**				4: "no route" NAK
**				5: reserved
**				6: "transmission NAK"
**				7: "transmission ACK"
**
**			Types 1, 2, and 3 are client to routing layer only.
**			Type 4 is routing layer to client only.
**			Types 6 and 7 are reserved to the routing layer.
**
**		[4]	TTL is only bottom six bits; the remainder of the octet
**			is reserved.  Any code changing or reading the TTL should
**			mask off the reserved bits; when creating a PDU, these
**			bits should be zero.
**
**		[5]	The top bit indicates that there are more fragments to
**			this PDU.  The next 15 bits are the sequence number, which
**			should start as a random number which can wrap back to
**			zero; after that it should be incremented on each PDU.
**			The bottom 16 bits are the fragment offset; if the PDU is
**			not fragmented it will be zero.
**
**			This field is unused in the TCP interface.
**
**		[6]	The SDU length (P) is the length of the entire reassembled
**			opaque payload.  The fragment length (F) is the size of this
**			fragment.  If transmitting over TCP, F is ignored.
**
**		[7]	The addresses included in the PDU are defined by the
**			the bottom three bits of octet 2.  The intent is that
**			once a flow is initialized, only that four octet field
**			will be sent.  However, the flowid compression is not
**			implemented yet.
**
**		Any additional octets in the header are interpreted as options.
*/

/*** values for header length field (octet 1) ***/
#define GDP_PKT_HDR_LEN_MASK		0x3f	// mask for header length

// no flags defined in this octet yet
#define GDP_PKT_HDR_LEN_UNUSED		0xc0	// reserved

/*** values for flags / router control / type of service field (octet 2) ***/
// address format
#define GDP_PKT_ADDR_TYPE_MASK		0x07	// indicates structure of addresses
#define GDP_PKT_ADDR_TYPE_2FULL		0x00	// two 256-bit addresses

// flag bits
#define GDP_PKT_TYPE_RELIABLE		0x08	// do extra work to ensure delivery
#define GDP_PKT_TYPE_SSEQ			0x10	// reset sequence number

// type field
#define GDP_PKT_TYPE_MASK			0xe0	// mask for the router command
// (client-to-router commands)
#define GDP_PKT_TYPE_REGULAR		0x00	// regular delivery
#define GDP_PKT_TYPE_FORWARD		0x20	// forward to another address
#define GDP_PKT_TYPE_ADVERTISE		0x40	// name advertisement
#define GDP_PKT_TYPE_WITHDRAW		0x60	// name withdrawal
// (router-to-client responses)
#define GDP_PKT_TYPE_NAK_NOROUTE	0x80	// no route / name unknown
// (internal (router-to-router only) responses)
#define GDP_PKT_TYPE_SEQ_PACKET		0xa0	// sequence notification
#define GDP_PKT_TYPE_NAK_PACKET		0xc0	// transmission nak
#define GDP_PKT_TYPE_ACK_PACKET		0xe0	// transmission ack

/*** values for time to live field (octet 3) ***/
#define GDP_PKT_TTL_MASK			0x3f	// mask for time to live
#define GDP_PKT_TTL_UNUSED			0xc0	// reserved

// no flags defined in this octet yet

/*** values for sequence_number, frag offset field (octets 4-7) ***/
#define GDP_PKT_SEQNO_SHIFT			17		// the number of bits to shift
#define GDP_PKT_SEQNO_MASK			0x7fff	// the sequence number mask
#define GDP_PKT_SEQNO_MF			0x10000	// more fragments bit
#define GDP_PKT_SEQNO_FOFF_MASK		0xffff	// fragment offset mask

// L4 sequence number
//typedef uint16_t					gdp_seqno_t;	// defined in gdp_priv.h
#define PRIgdp_seqno				PRIu16
#define GDP_SEQNO_BASE				(1 << 15)		// modulus base
#define GDP_SEQNO_NONE				((uint16_t) GDP_SEQNO_BASE)


/*
**  Callback function declarations to simplify later declarations.
*/

// called to issue advertisements (initial open or reopen)
typedef EP_STAT		gdp_chan_advert_func_t(
							gdp_chan_t *chan,
							int action,				// to be defined
							void *adata);			// passed in on open

// if router challenges, call this function
typedef EP_STAT		gdp_chan_advert_cr_t(
							gdp_chan_t *chan,
							gdp_name_t gname,
							int action,
							void *cdata,
							void *adata);

// called when data is received
typedef EP_STAT		gdp_chan_recv_cb_t(
							gdp_chan_t *chan,
							gdp_name_t src,
							gdp_name_t dst,
							gdp_seqno_t seqno,
							gdp_buf_t *payload,
							size_t payload_len);

// called when data can be sent (unused at this time; will probably change)
typedef EP_STAT		gdp_chan_send_cb_t(
							gdp_chan_t *chan,
							gdp_buf_t *payload);

// called on other channel events, e.g., close, error, or eof
typedef EP_STAT		gdp_chan_ioevent_cb_t(
							gdp_chan_t *chan,
							uint32_t flags);

// called on router events, e.g., NOROUTE
typedef EP_STAT		gdp_chan_router_cb_t(
							gdp_chan_t *chan,
							gdp_name_t src,
							gdp_name_t dst,
							size_t payload_len,
							EP_STAT estat);

// gdp_chan_ioevent_cb flags:
#define GDP_IOEVENT_USER_CLOSE	0					// user close
#define GDP_IOEVENT_CONNECTED	BEV_EVENT_CONNECTED	// connection established
#define GDP_IOEVENT_EOF			BEV_EVENT_EOF		// end of file on channel
#define GDP_IOEVENT_ERROR		BEV_EVENT_ERROR		// error on channel


/*
**  Channel operations.
*/

EP_STAT			_gdp_chan_init(				// initialize channel subsystem
						struct event_base *evbase,
						void *unused);

EP_STAT			_gdp_chan_open(				// open channel to routing layer
						const char *gdpd_addr,
						void *qos,
						gdp_chan_recv_cb_t *recv_cb,
						gdp_chan_send_cb_t *send_cb,
						gdp_chan_ioevent_cb_t *ioevent_cb,
						gdp_chan_router_cb_t *router_cb,
						gdp_chan_advert_func_t *advert_func,
						gdp_chan_x_t *cdata,
						gdp_chan_t **pchan);

EP_STAT			_gdp_chan_close(			// close channel
						gdp_chan_t *chan);

EP_STAT			_gdp_chan_send(				// send data to channel
						gdp_chan_t *chan,
						gdp_target_t *target,
						gdp_name_t src,
						gdp_name_t dst,
						gdp_buf_t *payload,
						int tos);

gdp_chan_x_t	*_gdp_chan_get_cdata(		// get user data from channel
						gdp_chan_t *chan);

EP_STAT			_gdp_chan_advertise(		// advertise name
						gdp_chan_t *chan,
						gdp_name_t gname,
						gdp_adcert_t *adcert,
						gdp_chan_advert_cr_t *challenge_cb,
						void *adata);

EP_STAT			_gdp_chan_withdraw(			// withdraw advertisement
						gdp_chan_t *chan,
						gdp_name_t gname,
						void *adata);

EP_STAT			_gdp_chan_advert_flush(		// flush advertisments/withdrawals
						gdp_chan_t *chan);
#define			_gdp_chan_advert_commit		_gdp_chan_advert_flush // back compat

EP_STAT			_gdp_chan_flush(			// flush buffers to network
						gdp_chan_t *chan);

void			_gdp_chan_lock(				// lock the channel
						gdp_chan_t *chan);

void			_gdp_chan_unlock(			// unlock the channel
						gdp_chan_t *chan);

void			_gdp_chan_drain_input(		// drain all input from channel
						gdp_chan_t *chan);

void			_gdp_chan_dump_reqs(		// print all outstanding requests
						gdp_chan_t *chan,
						FILE *fp);

#endif // _GDP_CHAN_H_
