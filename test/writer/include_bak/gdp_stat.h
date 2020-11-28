/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**	GDP_STAT.H --- status codes specific to the Global Data Plane
**
**		If you add codes here, be sure to add a string description
**		in gdp_stat.c.
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

#include <ep/ep_stat.h>
//------------------------------------------------------------------------------
//	Added for C++ compability
extern "C"{
//------------------------------------------------------------------------------

extern void		_gdp_stat_init(void);

// XXX	should really be in an include shared with other projects
//		to avoid conflicts in the future
#define GDP_MODULE			1
#define SQLITE_MODULE		2		// SQLite-specific error codes

#define GDP_STAT_NEW(sev, det)		EP_STAT_NEW(EP_STAT_SEV_ ## sev, \
										EP_REGISTRY_UCB, GDP_MODULE, det)
#define GDP_STAT_IS_GDP(estat)		(EP_STAT_REGISTRY(estat) == EP_REGISTRY_UCB && \
									 EP_STAT_MODULE(estat) == GDP_MODULE)

#define GDP_STAT_MSGFMT					GDP_STAT_NEW(ERROR, 1)
#define GDP_STAT_SHORTMSG				GDP_STAT_NEW(ERROR, 2)
#define GDP_STAT_READ_OVERFLOW			GDP_STAT_NEW(WARN, 3)
#define GDP_STAT_NOT_IMPLEMENTED		GDP_STAT_NEW(SEVERE, 4)
#define GDP_STAT_PDU_WRITE_FAIL			GDP_STAT_NEW(ERROR, 5)
#define GDP_STAT_PDU_READ_FAIL			GDP_STAT_NEW(ERROR, 6)
#define GDP_STAT_PDU_VERSION_MISMATCH	GDP_STAT_NEW(SEVERE, 7)
#define GDP_STAT_PDU_NO_SEQ				GDP_STAT_NEW(ERROR, 8)
#define GDP_STAT_KEEP_READING			GDP_STAT_NEW(WARN, 9)
#define GDP_STAT_LOG_NOT_OPEN			GDP_STAT_NEW(ERROR, 10)
#define GDP_STAT_UNKNOWN_RID			GDP_STAT_NEW(SEVERE, 11)
#define GDP_STAT_INTERNAL_ERROR			GDP_STAT_NEW(ABORT, 12)
#define GDP_STAT_BAD_IOMODE				GDP_STAT_NEW(ERROR, 13)
#define GDP_STAT_GDP_NAME_INVALID		GDP_STAT_NEW(ERROR, 14)
#define GDP_STAT_BUFFER_FAILURE			GDP_STAT_NEW(ABORT, 15)
#define GDP_STAT_NULL_GOB				GDP_STAT_NEW(ERROR, 16)
#define GDP_STAT_NULL_GIN				GDP_STAT_NEW(ERROR, 17)
#define GDP_STAT_PROTOCOL_FAIL			GDP_STAT_NEW(SEVERE, 18)
#define GDP_STAT_CORRUPT_LOG			GDP_STAT_NEW(SEVERE, 19)
#define GDP_STAT_DEAD_DAEMON			GDP_STAT_NEW(ABORT, 20)
#define GDP_STAT_LOG_VERSION_MISMATCH	GDP_STAT_NEW(SEVERE, 21)
#define GDP_STAT_READONLY				GDP_STAT_NEW(ERROR, 22)
#define GDP_STAT_NOTFOUND				GDP_STAT_NEW(ERROR, 23)
#define GDP_STAT_PDU_CORRUPT			GDP_STAT_NEW(ABORT, 24)
#define GDP_STAT_CRYPTO_SKEY_REQUIRED	GDP_STAT_NEW(ERROR, 25)
#define GDP_STAT_CRYPTO_SKEY_MISSING	GDP_STAT_NEW(WARN, 25)
#define GDP_STAT_LOG_READ_ERROR			GDP_STAT_NEW(ERROR, 26)
#define GDP_STAT_RECNO_SEQ_ERROR		GDP_STAT_NEW(ERROR, 27)
#define GDP_STAT_CRYPTO_VRFY_FAIL		GDP_STAT_NEW(ERROR, 28)
#define GDP_STAT_PHYSIO_ERROR			GDP_STAT_NEW(ERROR, 29)
#define GDP_STAT_RECORD_EXPIRED			GDP_STAT_NEW(WARN, 30)
#define GDP_STAT_USING_FREE_REQ			GDP_STAT_NEW(ABORT, 31)
#define GDP_STAT_BAD_REFCNT				GDP_STAT_NEW(ABORT, 32)
#define GDP_STAT_RECORD_MISSING			GDP_STAT_NEW(WARN, 33)
#define GDP_STAT_RECORD_DUPLICATED		GDP_STAT_NEW(WARN, 34)
#define GDP_STAT_DATUM_REQUIRED			GDP_STAT_NEW(ERROR, 35)
#define GDP_STAT_SQLITE_ERROR			GDP_STAT_NEW(SEVERE, 36)
#define GDP_STAT_INVOKE_TIMEOUT			GDP_STAT_NEW(SEVERE, 37)
#define GDP_STAT_PDU_TOO_LONG			GDP_STAT_NEW(SEVERE, 38)
#define GDP_STAT_CHAN_NOT_CONNECTED		GDP_STAT_NEW(ABORT, 39)
#define GDP_STAT_METADATA_REQUIRED		GDP_STAT_NEW(SEVERE, 40)
#define GDP_STAT_RESPONSE_SENT			GDP_STAT_NEW(WARN, 41)
#define GDP_STAT_NO_METADATA			GDP_STAT_NEW(ABORT, 42)
#define GDP_STAT_NAME_UNKNOWN			GDP_STAT_NEW(ERROR, 43)
#define GDP_STAT_MYSQL_ERROR			GDP_STAT_NEW(SEVERE, 44)
#define GDP_STAT_CRYPTO_SIG_MISSING		GDP_STAT_NEW(WARN, 45)
#define GDP_STAT_CRYPTO_NO_PUB_KEY		GDP_STAT_NEW(WARN, 46)
#define GDP_STAT_CRYPTO_NO_SIG			GDP_STAT_NEW(ERROR, 47)
#define GDP_STAT_SVC_NAME_REQ			GDP_STAT_NEW(SEVERE, 48)
#define GDP_STAT_NAME_SHA				GDP_STAT_NEW(WARN, 49)
#define GDP_STAT_OK_NAME_HONGD			GDP_STAT_NEW(OK, 50)
#define GDP_STAT_OK_NAME_PNAME			GDP_STAT_NEW(OK, 51)
#define GDP_STAT_OK_NAME_HEX			GDP_STAT_NEW(OK, 52)
#define GDP_STAT_HONGD_UNAVAILABLE		GDP_STAT_NEW(ERROR, 53)
#define GDP_STAT_CRYPTO_ERROR			GDP_STAT_NEW(ERROR, 54)


/*
**  Both GDP ACK/NAK commands and status codes are based on status
**  codes from CoAP and HTTP, which are themselves related.  In most
**  (but not all) cases the CoAP codes are more appropriate.
**  There are also some codes that have no CoAP or HTTP equivalents.
**
**  This list uses the HTTP-style (three digit) representation, and
**  will be massaged later into both status codes and PDU commands.
**  I've tried to show the equivalent HTTP/CoAP codes (with CoAP
**  scaled from x.yz to xyz).
**
**  CoAP is based on RFC7252.  HTTP codes are based on version 1.1.
*/

#define _GDP_CCODE_SUCCESS			200		// HTTP/CoAP Success
#define _GDP_CCODE_CREATED			201		// HTTP/CoAP Created (201)
#define _GDP_CCODE_DELETED			202		// CoAP Deleted
											// (HTTP 202 Accepted)
#define _GDP_CCODE_VALID			203		// CoAP Valid
											// (HTTP 203 Non-Authoritative Info)
#define _GDP_CCODE_CHANGED			204		// CoAP Changed
											// (HTTP 204 No Content)
#define _GDP_CCODE_CONTENT			205		// Content (~200, GET only)
											// (HTTP 205 Reset Content)
											// (HTTP 206 Partial Content)
#define _GDP_CCODE_END_OF_RESULTS	263		// end of results (subscr/mread)

#define _GDP_CCODE_BADREQ			400		// HTTP/CoAP Bad Request
#define _GDP_CCODE_UNAUTH			401		// HTTP/CoAP Unauthorized
#define _GDP_CCODE_BADOPT			402		// CoAP Bad Option
											// (HTTP 402 Payment Required)
#define _GDP_CCODE_FORBIDDEN		403		// HTTP/CoAP Forbidden
#define _GDP_CCODE_NOTFOUND			404		// HTTP/CoAP Not Found
#define _GDP_CCODE_METHNOTALLOWED	405		// HTTP/CoAP Method Not Allowed
#define _GDP_CCODE_NOTACCEPTABLE	406		// HTTP/CoAP Not Acceptable
											// (HTTP 407 Proxy Auth Required)
											// (HTTP 408 Request Timeout)
#define _GDP_CCODE_CONFLICT			409		// HTTP Conflict
#define _GDP_CCODE_GONE				410		// HTTP Gone
											// (HTTP 411 Length Required)
#define _GDP_CCODE_PRECONFAILED		412		// HTTP/CoAP Precondition Failed
#define _GDP_CCODE_TOOLARGE			413		// HTTP/CoAP Request Entity Too Large
											// (HTTP 414 Request-URI Too Long)
#define _GDP_CCODE_UNSUPMEDIA		415		// CoAP Unsupported Content-Format
											// (HTTP 415 Request URI Too Long)
											// (HTTP 416 Requested Range Not Satisficable)
											// (HTTP 417 Expectation Failed)
#define _GDP_CCODE_REC_MISSING		430		// GDP missing record (gap)
#define _GDP_CCODE_REC_DUP			431		// GDP Duplicate Record

#define _GDP_CCODE_INTERNAL			500		// HTTP/CoAP Internal Server Error
#define _GDP_CCODE_NOTIMPL			501		// HTTP/CoAP Not Implemented
#define _GDP_CCODE_BADGATEWAY		502		// HTTP/CoAP Bad Gateway
#define _GDP_CCODE_SVCUNAVAIL		503		// HTTP/CoAP Service Unavailable
#define _GDP_CCODE_GWTIMEOUT		504		// HTTP/CoAP Gateway Timeout
#define _GDP_CCODE_PROXYNOTSUP		505		// CoAP Proxying Not Supported
											// (HTTP 505 HTTP Version Not Supported)
#define _GDP_CCODE_REPLICATE_FAIL	514		// couldn't replicate
#define _GDP_CCODE_LOST_SUBSCR		515		// GDP Lost Subscription

#define _GDP_CCODE_NOROUTE			600		// no advertisement found for name

//		values from 200 up are reserved for this

#define GDP_STAT_ACK_SUCCESS		GDP_STAT_NEW(OK, _GDP_CCODE_SUCCESS)
#define GDP_STAT_ACK_CREATED		GDP_STAT_NEW(OK, _GDP_CCODE_CREATED)
#define GDP_STAT_ACK_DELETED		GDP_STAT_NEW(OK, _GDP_CCODE_DELETED)
#define GDP_STAT_ACK_VALID			GDP_STAT_NEW(OK, _GDP_CCODE_VALID)
#define GDP_STAT_ACK_CHANGED		GDP_STAT_NEW(OK, _GDP_CCODE_CHANGED)
#define GDP_STAT_ACK_CONTENT		GDP_STAT_NEW(OK, _GDP_CCODE_CONTENT)
#define GDP_STAT_ACK_END_OF_RESULTS	GDP_STAT_NEW(WARN, _GDP_CCODE_END_OF_RESULTS)

#define GDP_STAT_NAK_BADREQ			GDP_STAT_NEW(ERROR, _GDP_CCODE_BADREQ)
#define GDP_STAT_NAK_UNAUTH			GDP_STAT_NEW(ERROR, _GDP_CCODE_UNAUTH)
#define GDP_STAT_NAK_BADOPT			GDP_STAT_NEW(ERROR, _GDP_CCODE_BADOPT)
#define GDP_STAT_NAK_FORBIDDEN		GDP_STAT_NEW(ERROR, _GDP_CCODE_FORBIDDEN)
#define GDP_STAT_NAK_NOTFOUND		GDP_STAT_NEW(ERROR, _GDP_CCODE_NOTFOUND)
#define GDP_STAT_NAK_METHNOTALLOWED	GDP_STAT_NEW(ERROR, _GDP_CCODE_METHNOTALLOWED)
#define GDP_STAT_NAK_NOTACCEPTABLE	GDP_STAT_NEW(ERROR, _GDP_CCODE_NOTACCEPTABLE)
#define GDP_STAT_NAK_CONFLICT		GDP_STAT_NEW(ERROR, _GDP_CCODE_CONFLICT)
#define GDP_STAT_NAK_GONE			GDP_STAT_NEW(WARN,  _GDP_CCODE_GONE)
#define GDP_STAT_NAK_PRECONFAILED	GDP_STAT_NEW(ERROR, _GDP_CCODE_PRECONFAILED)
#define GDP_STAT_NAK_TOOLARGE		GDP_STAT_NEW(ERROR, _GDP_CCODE_TOOLARGE)
#define GDP_STAT_NAK_UNSUPMEDIA		GDP_STAT_NEW(ERROR, _GDP_CCODE_UNSUPMEDIA)
#define GDP_STAT_NAK_REC_MISSING	GDP_STAT_NEW(WARN, _GDP_CCODE_REC_MISSING)
#define GDP_STAT_NAK_REC_DUP		GDP_STAT_NEW(WARN, _GDP_CCODE_REC_DUP)

#define GDP_STAT_NAK_INTERNAL		GDP_STAT_NEW(SEVERE, _GDP_CCODE_INTERNAL)
#define GDP_STAT_NAK_NOTIMPL		GDP_STAT_NEW(SEVERE, _GDP_CCODE_NOTIMPL)
#define GDP_STAT_NAK_BADGATEWAY		GDP_STAT_NEW(SEVERE, _GDP_CCODE_BADGATEWAY)
#define GDP_STAT_NAK_SVCUNAVAIL		GDP_STAT_NEW(SEVERE, _GDP_CCODE_SVCUNAVAIL)
#define GDP_STAT_NAK_GWTIMEOUT		GDP_STAT_NEW(SEVERE, _GDP_CCODE_GWTIMEOUT)
#define GDP_STAT_NAK_PROXYNOTSUP	GDP_STAT_NEW(SEVERE, _GDP_CCODE_PROXYNOTSUP)
#define GDP_STAT_NAK_REPLICATE_FAIL	GDP_STAT_NEW(ERROR, _GDP_CCODE_REPLICATE_FAIL)
#define GDP_STAT_NAK_LOST_SUBSCR	GDP_STAT_NEW(WARN, _GDP_CCODE_LOST_SUBSCR)

#define GDP_STAT_NAK_NOROUTE		GDP_STAT_NEW(ERROR, _GDP_CCODE_NOROUTE)


/*
**  Create EP_STAT from protocol command codes for acks and naks
*/

#define GDP_STAT_FROM_ACK(c)		EP_STAT_NEW(EP_STAT_SEV_OK, 0, 0, (c) - GDP_ACK_MIN + 200)
#define GDP_STAT_FROM_C_NAK(c)		GDP_STAT_NEW(ERROR, (c) - GDP_NAK_C_MIN + 400)
#define GDP_STAT_FROM_S_NAK(c)		GDP_STAT_NEW(SEVERE, (c) - GDP_NAK_S_MIN + 500)
#define GDP_STAT_FROM_R_NAK(c)		GDP_STAT_NEW(ERROR, (c) - GDP_NAK_R_MIN + 600)

EP_STAT					_gdp_stat_from_acknak(int acknak);


/*
**  Tests to see if an EP_STAT corresponds directly to a protocol command code
**
**		Detail values from 200 through 699 are reserved for this.
**		Note that these don't do serious error checking.
*/

#define GDP_STAT_IS_ACKNAK(estat)	(GDP_STAT_IS_GDP(estat) && \
									 EP_STAT_DETAIL(estat) >= 200 && \
									 EP_STAT_DETAIL(estat) <= 615)

#define GDP_STAT_IS_ACK(estat)		(GDP_STAT_IS_GDP(estat) && \
									 EP_STAT_DETAIL(estat) >= 200 && \
									 EP_STAT_DETAIL(estat) <= 263)
#define GDP_STAT_IS_C_NAK(estat)	(GDP_STAT_IS_GDP(estat) && \
									 EP_STAT_DETAIL(estat) >= 400 && \
									 EP_STAT_DETAIL(estat) <= 431)
#define GDP_STAT_IS_S_NAK(estat)	(GDP_STAT_IS_GDP(estat) && \
									 EP_STAT_DETAIL(estat) >= 500 && \
									 EP_STAT_DETAIL(estat) <= 531)
#define GDP_STAT_IS_R_NAK(estat)	(GDP_STAT_IS_GDP(estat) && \
									 EP_STAT_DETAIL(estat) >= 600 && \
									 EP_STAT_DETAIL(estat) <= 699)

//------------------------------------------------------------------------------
//	Added for C++ compability
}
//------------------------------------------------------------------------------
