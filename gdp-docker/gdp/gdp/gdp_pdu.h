/* vim: set ai sw=4 sts=4 ts=4 : */

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

#ifndef _GDP_PDU_H_
#define _GDP_PDU_H_

#include <stdio.h>
#include <netinet/in.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include "gdp_priv.h"

#define GDP_PROTO_CUR_VERSION	3		// current protocol version
#define GDP_PROTO_MIN_VERSION	2		// min version we can accept




/*
**	Header for a GDP Protocol Data Unit (PDU).
**
**		This struct is not the "on the wire" format, which has to be
**		put into network byte order and packed.	However, this does
**		show the order in which fields are written.
**
**		Commands are eight bits with the top three bits encoding
**		additional semantics.  Those bits are:
**
**		00x		"Blind" (unacknowledged) command
**		01x		Acknowledged command
**		10x		Positive acknowledgement
**		110		Negative acknowledgement, client side problem
**		111		Negative acknowledgement, server side problem
**
**		These roughly correspond to the "Type" and "Code" class
**		field in the CoAP header.
**
**		XXX We may still want to play with these allocations,
**			depending on how dense the various spaces become.  I
**			suspect that "acknowledged command" will have the
**			most values and the ack/naks won't have very many.
**			Remember in particular that the commands have to include
**			the commands between gdpds for things like migration,
**			resource negotiation, etc.
**
**		XXX CoAP has two "sequence numbers": a message-id which
**			relates ack/naks to commands and a "token" which is
**			a higher level construct relating (for example)
**			subscribe requests to results.	The "rid" represents
**			a shorter version of the "token".  We don't include
**			"seq" since this is a lower-level concept that is
**			subsumed by TCP.
**
**		Since we're using ProtoBufs most of the complication is there.
**		The gdp_pdu struct is now a Layer 5 ("Session Layer") concept
**		and now only contains in-core administrative stuff and the
**		src/dst info, which is inherited from Layer 4 ("Transport
**		Layer").  All the juicy stuff is in the message itself.
**
**		When sending a ProtoBuf messages you can do static memory
**		allocation, but when receiving you have no choice: the
**		result is malloced.  For that reason the message itself
**		appears both as part of this structure and as a pointer.
**		When sending, the pointer will point at the structure.
**		When receiving, the pointer will be malloced and must
**		be explicitly freed.
*/

// Layer 5 definitions; L4 are in gdp_chan.h
typedef uint32_t		gdp_l5seqno_t;	// protocol sequence number
#define	PRIgdp_l5seqno	PRIu32

typedef uint32_t		gdp_rid_t;		// request id (uniquifies request)
#define PRIgdp_rid		PRIu32


struct gdp_pdu
{
	// metadata
	TAILQ_ENTRY(gdp_pdu)	list;		// free list
	uint32_t				flags;
	gdp_chan_t				*chan;		// only used in _gdp_pdu_process

	// copied from L4 (lower layer)
	gdp_name_t				dst;		// destination address
	gdp_name_t				src;		// source address
	gdp_seqno_t				seqno;		// sequence number

	// Layer 5 info
	GdpMessage				*msg;		// on-the-wire message (not NULL)
};

// flag bits
#define GDP_PDU_INUSE		0x00000001	// PDU is allocated and in use


/***** dummy values for other fields *****/
#define GDP_PDU_NO_RID		UINT32_C(0)		// no request id
#define GDP_PDU_NO_RECNO	INT64_C(0)		// no record number
#define GDP_PDU_NO_L5SEQNO	UINT32_C(0)		// no L5 sequence number
#define GDP_PDU_ANY_RID		UINT32_MAX		// any request id

/***** manifest constants *****/

// maximum size of an on-wire header (excluding data and signature)
#define _GDP_PDU_MAXHDRSZ		(_GDP_PDU_FIXEDHDRSZ + _GDP_PDU_MAXOPTSZ)


/***** commands *****/

// functions to determine characteristics of command/ack/nak
#define GDP_CMD_NEEDS_ACK(c)	(((c) & 0xc0) == 0x40)	// expect ACK/NAK
#define GDP_CMD_IS_COMMAND(c)	(((c) & 0x80) != 0x80)	// is a command
#define GDP_CMD_IS_ACK(c)		(((c) & 0xc0) == 0x80)	// is a positive ack
#define GDP_CMD_IS_NAK(c)		(((c) & 0xc0) == 0xc0)	// is a negative nak
#define GDP_CMD_IS_C_NAK(c)		(((c) & 0xe0) == 0xc0)	// is a client side nak
#define GDP_CMD_IS_S_NAK(c)		(((c) & 0xe0) == 0xe0)	// is a server side nak


/*
**  Protocol command values
**
**		The ACK and NAK values are tightly coupled with EP_STAT codes
**		and with COAP status codes, hence the somewhat baroque approach
**		here.
*/


/*
**  Short names for ugly Protobuf names.
**  See gdp.proto for numeric values.
*/

//		0-63			Blind commands
#define GDP_CMD_KEEPALIVE			GDP_MSG_CODE__CMD_KEEPALIVE
#define GDP_CMD_ADVERTISE			GDP_MSG_CODE__CMD_ADVERTISE
#define GDP_CMD_WITHDRAW			GDP_MSG_CODE__CMD_WITHDRAW
#define GDP_CMD_ROUTER_META			GDP_MSG_CODE__CMD_ROUTER_META

//		64-127			Acknowledged commands
#define GDP_CMD_PING				GDP_MSG_CODE__CMD_PING
#define GDP_CMD_HELLO				GDP_MSG_CODE__CMD_HELLO
#define GDP_CMD_CREATE				GDP_MSG_CODE__CMD_CREATE
#define GDP_CMD_OPEN_AO				GDP_MSG_CODE__CMD_OPEN_AO
#define GDP_CMD_OPEN_RO				GDP_MSG_CODE__CMD_OPEN_RO
#define GDP_CMD_OPEN_RA				GDP_MSG_CODE__CMD_OPEN_RA
#define GDP_CMD_CLOSE				GDP_MSG_CODE__CMD_CLOSE
#define GDP_CMD_APPEND				GDP_MSG_CODE__CMD_APPEND
#define GDP_CMD_READ_BY_RECNO		GDP_MSG_CODE__CMD_READ_BY_RECNO
#define GDP_CMD_READ_BY_TS			GDP_MSG_CODE__CMD_READ_BY_TS
#define GDP_CMD_READ_BY_HASH		GDP_MSG_CODE__CMD_READ_BY_HASH
#define GDP_CMD_SUBSCRIBE_BY_RECNO	GDP_MSG_CODE__CMD_SUBSCRIBE_BY_RECNO
#define GDP_CMD_SUBSCRIBE_BY_TS		GDP_MSG_CODE__CMD_SUBSCRIBE_BY_TS
#define GDP_CMD_SUBSCRIBE_BY_HASH	GDP_MSG_CODE__CMD_SUBSCRIBE_BY_HASH
#define GDP_CMD_UNSUBSCRIBE			GDP_MSG_CODE__CMD_UNSUBSCRIBE
#define GDP_CMD_GETMETADATA			GDP_MSG_CODE__CMD_GETMETADATA
#define GDP_CMD_NEWSEGMENT			GDP_MSG_CODE__CMD_NEWSEGMENT
#define GDP_CMD_DELETE				GDP_MSG_CODE__CMD_DELETE
#define GDP_CMD_FWD_APPEND			GDP_MSG_CODE__CMD_FWD_APPEND

//		128-191			Positive acks (HTTP 200-263)
#define GDP_ACK_MIN			128			// minimum ack code
#define GDP_ACK_SUCCESS				GDP_MSG_CODE__ACK_SUCCESS
#define GDP_ACK_CREATED				GDP_MSG_CODE__ACK_CREATED
#define GDP_ACK_DELETED				GDP_MSG_CODE__ACK_DELETED
#define GDP_ACK_VALID				GDP_MSG_CODE__ACK_VALID
#define GDP_ACK_CHANGED				GDP_MSG_CODE__ACK_CHANGED
#define GDP_ACK_CONTENT				GDP_MSG_CODE__ACK_CONTENT
#define GDP_ACK_END_OF_RESULTS		GDP_MSG_CODE__ACK_END_OF_RESULTS
#define GDP_ACK_MAX			191			// maximum ack code

//		192-223			Negative acks, client side (CoAP, HTTP 400-431)
#define GDP_NAK_C_MIN		192			// minimum client-side nak code
#define GDP_NAK_C_BADREQ			GDP_MSG_CODE__NAK_C_BADREQ
#define GDP_NAK_C_UNAUTH			GDP_MSG_CODE__NAK_C_UNAUTH
#define GDP_NAK_C_BADOPT			GDP_MSG_CODE__NAK_C_BADOPT
#define GDP_NAK_C_FORBIDDEN			GDP_MSG_CODE__NAK_C_FORBIDDEN
#define GDP_NAK_C_NOTFOUND			GDP_MSG_CODE__NAK_C_NOTFOUND
#define GDP_NAK_C_METHNOTALLOWED	GDP_MSG_CODE__NAK_C_METHNOTALLOWED
#define GDP_NAK_C_NOTACCEPTABLE		GDP_MSG_CODE__NAK_C_NOTACCEPTABLE
#define GDP_NAK_C_CONFLICT			GDP_MSG_CODE__NAK_C_CONFLICT
#define GDP_NAK_C_GONE				GDP_MSG_CODE__NAK_C_GONE
#define GDP_NAK_C_PRECONFAILED		GDP_MSG_CODE__NAK_C_PRECONFAILED
#define GDP_NAK_C_TOOLARGE			GDP_MSG_CODE__NAK_C_TOOLARGE
#define GDP_NAK_C_UNSUPMEDIA		GDP_MSG_CODE__NAK_C_UNSUPMEDIA
#define GDP_NAK_C_REC_MISSING		GDP_MSG_CODE__NAK_C_REC_MISSING
#define GDP_NAK_C_REC_DUP			GDP_MSG_CODE__NAK_C_REC_DUP
#define GDP_NAK_C_MAX		223			// maximum client-side nak code

//		224-239			Negative acks, server side (CoAP, HTTP 500-515)
#define GDP_NAK_S_MIN		224			// minimum server-side nak code
#define GDP_NAK_S_INTERNAL			GDP_MSG_CODE__NAK_S_INTERNAL
#define GDP_NAK_S_NOTIMPL			GDP_MSG_CODE__NAK_S_NOTIMPL
#define GDP_NAK_S_BADGATEWAY		GDP_MSG_CODE__NAK_S_BADGATEWAY
#define GDP_NAK_S_SVCUNAVAIL		GDP_MSG_CODE__NAK_S_SVCUNAVAIL
#define GDP_NAK_S_GWTIMEOUT			GDP_MSG_CODE__NAK_S_GWTIMEOUT
#define GDP_NAK_S_PROXYNOTSUP		GDP_MSG_CODE__NAK_S_PROXYNOTSUP
#define GDP_NAK_S_REPLICATE_FAIL	GDP_MSG_CODE__NAK_S_REPLICATE_FAIL
#define GDP_NAK_S_LOST_SUBSCR		GDP_MSG_CODE__NAK_S_LOST_SUBSCR
#define GDP_NAK_S_MAX		239			// maximum server-side nak code

//		240-254			Negative acks, routing layer
#define GDP_NAK_R_MIN		240			// minimum routing layer nak code
#define GDP_NAK_R_NOROUTE			GDP_MSG_CODE__NAK_R_NOROUTE
#define GDP_NAK_R_MAX		254			// maximum routing layer nak code

//		255				Reserved


gdp_pdu_t	*_gdp_pdu_new(			// allocate a new PDU
				GdpMessage *msg,		// the initial message (may be NULL)
				gdp_name_t src,			// source address
				gdp_name_t dst,			// destination address
				gdp_seqno_t seqno);		// L4 sequence number

void		_gdp_pdu_free(			// free a PDU
				gdp_pdu_t **);

EP_STAT		_gdp_pdu_out(			// send a PDU to a network buffer
				gdp_pdu_t *,			// the PDU information
				gdp_chan_t *);			// the network channel

EP_STAT		_gdp_pdu_in(			// read a PDU from a network buffer
				gdp_pdu_t *,			// the buffer to store the result
				gdp_buf_t *pbuf,		// the payload (input) buffer
				size_t plen,			// the payload length
				gdp_chan_t *);			// the input channel

void		_gdp_pdu_dump(
				const gdp_pdu_t *pdu,
				FILE *fp,
				int indent);

void		_gdp_pdu_process(
				gdp_pdu_t *pdu,
				gdp_chan_t *chan);

// generic sockaddr union	XXX does this belong in this header file?
union sockaddr_xx
{
	struct sockaddr		sa;
	struct sockaddr_in	sin;
	struct sockaddr_in6 sin6;
};

#endif // _GDP_PDU_H_
