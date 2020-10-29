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

/*
**	These headers are not intended for external use.
*/

#ifndef _GDP_PRIV_H_
#define _GDP_PRIV_H_

#include <ep/ep.h>
#include <ep/ep_assert.h>
#include <ep/ep_crypto.h>
#include <ep/ep_thr.h>

#include <event2/buffer.h>

#include "gdp.pb-c.h"

#if EP_OSCF_USE_VALGRIND
# include <valgrind/helgrind.h>
#else
# define VALGRIND_HG_CLEAN_MEMORY(a, b)
#endif

#ifndef GDP_COMPAT_OLD_LOG_NAMES
# define GDP_COMPAT_OLD_LOG_NAMES		1	// assume old style SHA(human) names
#endif
#ifndef GDP_COMPAT_OLD_PUBKEYS
# define GDP_COMPAT_OLD_PUBKEYS			1	// try old public key metadata name
#endif
#ifndef GDP_DEFAULT_CREATION_SERVICE
//# define GDP_DEFAULT_CREATION_SERVICE	"edu.berkeley.eecs.gdp.service.creation"
#endif
#ifndef GDP_DEFAULT_DATA_ROOT
# define GDP_DEFAULT_DATA_ROOT			"/var/swarm/gdp"
#endif

typedef struct gdp_chan		gdp_chan_t;		// should be in gdp_chan.h?
typedef uint16_t			gdp_seqno_t;	// should be in gdp_chan.h?
typedef struct gdp_req		gdp_req_t;
typedef struct gdp_gob		gdp_gob_t;
typedef struct gdp_gin		gdp_gin_t;
typedef struct gdp_pdu		gdp_pdu_t;		// should be in gdp_pdu.h?
typedef GdpMessage			gdp_msg_t;
typedef GdpMsgCode			gdp_cmd_t;
typedef struct event_base	event_base_t;
TAILQ_HEAD(gev_list, gdp_event);

#include "gdp_pdu.h"

extern EP_THR		_GdpIoEventLoopThread;
extern event_base_t	*_GdpIoEventBase;	// for all I/O events
extern gdp_chan_t	*_GdpChannel;		// our primary app-level protocol port
extern gdp_name_t	_GdpMyRoutingName;	// source name for PDUs
extern int			_GdpInitState;		// initialization state, see below

#define GDP_INIT_NONE		0	// uninitialized
#define GDP_INIT_PHASE_0	10	// phase 0 complete
#define GDP_INIT_LIB		20	// gdp_init_lib done
#define GDP_INIT_COMPLETE	99	// all initialization complete

#define GDP_CHECK_INITIALIZED											\
					((_GdpInitState >= GDP_INIT_COMPLETE)				\
										? EP_STAT_OK					\
										: gdp_init(NULL))

#ifndef GDP_OPT_EXTENDED_CACHE_CHECK		//XXX DEBUG TEMPORARY
# define GDP_OPT_EXTENDED_CACHE_CHECK	1	//XXX DEBUG TEMPORARY
#endif										//XXX DEBUG TEMPORARY
#if GDP_OPT_EXTENDED_CACHE_CHECK
#define IF_LIST_CHECK_OK(list, item, chain, type)						\
	type *_x_ ## item;													\
	if (ep_dbg_test(Dbg, 10))											\
	{																	\
		LIST_FOREACH(_x_ ## item, list, chain)							\
		{																\
			EP_ASSERT_ELSE(_x_ ## item != item, break);					\
		}																\
	}																	\
	else																\
		_x_ ## item = NULL;												\
	if (_x_ ## item == NULL)
#else
#define IF_LIST_CHECK_OK(list, item, chain, type)						\
	if (true)
#endif

// helper to do sanity checks
#define GDP_MSG_CHECK(pdu, recovery)										\
			EP_ASSERT_ELSE(pdu != NULL, recovery);							\
			EP_ASSERT_ELSE(pdu->msg != NULL, recovery);

// declare the type of the gdp_req linked list (used multiple places)
LIST_HEAD(req_head, gdp_req);


/*
**  Some generic constants
*/

// "dump" routine detail parameters (XXX should these be public?)
#define GDP_PR_PRETTY		0		// suitable for end users
#define GDP_PR_BASIC		8		// basic debug information
#define GDP_PR_DETAILED		16		// detailed information
#define GDP_PR_RECURSE		32		// recurse into substructures
									// add N to recurse N+1 levels deep


/*
**  Basic data types
*/

// hashes
//struct gdp_hash
//{
//	int					alg;		// hash algorithm to use
//	gdp_buf_t			*buf;		// the hash value itself
//};
gdp_buf_t		*_gdp_hash_getbuf(gdp_hash_t *hash);


// signatures
//struct gdp_sig
//{
//	XXX;
//};
gdp_buf_t		*_gdp_sig_getbuf(gdp_sig_t *sig);


/*
**	 Datums
**		These are the underlying data unit that is passed through a GOB.
**
**		The timestamp here is the database commit timestamp; any sample
**		timestamp must be added by the sensor itself as part of the data.
**
**		dhash = H(dbuf)
**		datum hash = H(recno || ts || prevhash || dhash)
**		prevhash = hash of previous datum
*/

struct gdp_datum
{
	EP_THR_MUTEX		mutex;			// locking mutex (mostly for dbuf)
	struct gdp_datum	*next;			// next in free list
	uint32_t			flags;			// see below
	gdp_recno_t			recno;			// the record number
	EP_TIME_SPEC		ts;				// commit timestamp
	gdp_buf_t			*dbuf;			// data buffer
	gdp_sig_t			*sig;			// signature (may be NULL)
	gdp_hash_t			*prevhash;		// hash of previous datum
};

#define GDP_DF_INUSE		0x00000001	// datum is in use
#define GDP_DF_GOODSIG		0x00000002	// signature is good

#define GDP_DATUM_ISGOOD(datum)											\
				((datum) != NULL &&										\
				 (datum)->dbuf != NULL &&									\
				 EP_UT_BITSET(GDP_DF_INUSE, (datum)->flags))

gdp_datum_t		*_gdp_datum_new_gob(	// generate new datum from GOB
						void);

gdp_datum_t		*gdp_datum_dup(			// duplicate a datum
						const gdp_datum_t *datum);

void			_gdp_datum_dump(		// dump data record (for debugging)
						const gdp_datum_t *datum,	// message to print
						FILE *fp);					// file to print it to

gdp_hash_t		*_gdp_datum_hash(		// compute hash of datum
						gdp_datum_t *datum,
						gdp_gob_t *gob);			// enclosing GOB

bool			_gdp_datum_hash_equal(	// check that a hash matches the datum
						gdp_datum_t *datum,			// the datum to check
						gdp_gob_t *gob,				// enclosing GOB
						const gdp_hash_t *hash);	// the hash to check against

EP_STAT			_gdp_datum_digest(		// add datum to existing digest
						gdp_datum_t *datum,			// the datum to include
						EP_CRYPTO_MD *md);			// the existing digest

void			_gdp_datum_to_pb(		// convert datum to protobuf form
						const gdp_datum_t *datum,
						GdpMessage *msg,
						GdpDatum *pb);

void			_gdp_datum_from_pb(		// convert protobuf form to datum
						gdp_datum_t *datum,
						const GdpDatum *pb,
						const GdpSignature *sig);

EP_STAT			_gdp_datum_sign(		// sign a datum
						gdp_datum_t *datum,			// the datum to sign
						gdp_gob_t *gob);			// the object storing it

EP_STAT			_gdp_datum_vrfy_gob(	// verify a datum signature
						gdp_datum_t *datum,
						gdp_gob_t *gob);

void			_gdp_timestamp_from_pb(	// convert protobuf form to EP_TIME_SPEC
						EP_TIME_SPEC *ts,
						const GdpTimestamp *pbd);



/*
**  GDP Objects
**
**		There are two data structures around GOBs.
**
**		The gdp_gin is an open handle on a GDP Object Instance.
**		Client-side subscriptions are associated with this so
**		that gdp_event_next can deliver the correct information.
**		Read and append filters are also considered "external".
**		It is not used at all by gdplogd.
**		Mostly implemented in gdp_api.c.
**
**		The gdp_gob is the internal representation of a potentially
**		shared GOB.  This is what is in the cache.
**		It has the request list representing
**		server-side subscriptions.  It is used both by the
**		GDP library and by gdplogd.  Mostly implemented in
**		gdp_gob_ops.c.
*/

SLIST_HEAD(gcl_head, gdp_gcl);


// application per-open-instance information (unused in gdplogd)
struct gdp_gin
{
	EP_THR_MUTEX		mutex;			// lock on this data structure
	gdp_gob_t			*gob;			// internal GDP object
	SLIST_ENTRY(gdp_gin)
						next;			// chain for freelist
	uint16_t			flags;			// see below
	gdp_iomode_t		iomode;			// read only or append only
	void				(*closefunc)(gdp_gin_t *);
										// called when this is closed
	EP_STAT				(*apndfilter)(	// append filter function
							gdp_datum_t *,
							void *);
	void				*apndfpriv;		// private data for apndfilter
	EP_STAT				(*readfilter)(	// read filter function
							gdp_datum_t *,
							void *);
	void				*readfpriv;		// private data for readfilter
};

#define GINF_INUSE			0x0001		// GIN is allocated
#define GINF_ISLOCKED		0x0002		// GIN is locked
#define GINF_SIG_VRFY		0x0004		// verify returned data if possible
#define GINF_SIG_VRFY_REQ	0x0008		// signature verification required

// internal GDP object, shared between open instances, used in gdplogd
struct gdp_gob
{
	EP_THR_MUTEX		mutex;			// lock on this data structure
	time_t				utime;			// last time used (seconds only)
	LIST_ENTRY(gdp_gob)	ulist;			// list sorted by use time
	struct req_head		reqs;			// list of outstanding requests
	gdp_name_t			name;			// the internal name
	gdp_pname_t			pname;			// printable name (for debugging)
	uint16_t			flags;			// flag bits, see below
	uint16_t			hashalg;		// hash algorithm
	int					refcnt;			// reference counter
	void				(*freefunc)(gdp_gob_t *);
										// called when this is freed
	gdp_recno_t			nrecs;			// # of records (actually last recno)
	gdp_md_t			*gob_md;		// metadata
	EP_CRYPTO_MD		*sign_ctx;		// base digest for signature
	EP_CRYPTO_MD		*vrfy_ctx;		// base digest for verification
	struct gdp_gob_xtra	*x;				// for use by gdplogd, gdp-rest
};

// flags for GDP objects
#define GOBF_INUSE			0x0001		// handle is allocated
#define GOBF_ISLOCKED		0x0002		// GOB is locked
#define GOBF_DROPPING		0x0004		// handle is being deallocated
#define GOBF_INCACHE		0x0008		// handle is in cache
#define GOBF_DEFER_FREE		0x0010		// defer actual free until reclaim
#define GOBF_KEEPLOCKED		0x0020		// don't unlock in _gdp_gob_decref
#define GOBF_PENDING		0x0040		// not yet fully open
#define GOBF_SIGNING		0x0080		// we are signing records
#define GOBF_VERIFYING		0x0100		// we are verifying records
#define GOBF_VRFY_WARN		0x0200		// verification fail is a warning only


/*
**  GOB functions
*/

// Used to avoid compiler warnings on some versions of gcc ("value computed is
// not used").  If "inline" doesn't work, use #define _gdp_bool_null(b) (b).
static inline bool _gdp_bool_null(bool b) { return b; }

#define GDP_GOB_ISGOOD(gob)												\
				((gob) != NULL &&										\
				 EP_UT_BITSET(GOBF_INUSE, (gob)->flags))
#define GDP_GOB_ASSERT_ISLOCKED(gob)									\
			_gdp_bool_null(												\
				EP_ASSERT(GDP_GOB_ISGOOD(gob)) &&						\
				EP_ASSERT(EP_UT_BITSET(GOBF_ISLOCKED, (gob)->flags)) &&	\
				EP_THR_MUTEX_ASSERT_ISLOCKED(&(gob)->mutex)				\
			)


EP_STAT			_gdp_gob_new(				// create new in-mem handle
						gdp_name_t name,
						gdp_gob_t **gobhp);

void			_gdp_gob_free(				// free in-memory handle
						gdp_gob_t **gob);		// GOB to free

#define _gdp_gob_lock(g)		_gdp_gob_lock_trace(g, __FILE__, __LINE__, #g)
#define _gdp_gob_unlock(g)		_gdp_gob_unlock_trace(g, __FILE__, __LINE__, #g)
#define _gdp_gob_incref(g)		_gdp_gob_incref_trace(g, __FILE__, __LINE__, #g)
#define _gdp_gob_decref(g, k)	_gdp_gob_decref_trace(g, k, __FILE__, __LINE__, #g)

void			_gdp_gob_lock_trace(		// lock the GOB mutex
						gdp_gob_t *gob,
						const char *file,
						int line,
						const char *id);

void			_gdp_gob_unlock_trace(		// unlock the GOB mutex
						gdp_gob_t *gob,
						const char *file,
						int line,
						const char *id);

gdp_gob_t		*_gdp_gob_incref_trace(		// increase reference count (trace)
						gdp_gob_t *gob,
						const char *file,
						int line,
						const char *id);

void			_gdp_gob_decref_trace(		// decrease reference count (trace)
						gdp_gob_t **gobp,
						bool keeplocked,
						const char *file,
						int line,
						const char *id);

EP_STAT			_gdp_gob_newname(			// create new name based on metadata
						gdp_gob_t *gob);

void			_gdp_gob_dump(				// dump for debugging
						const gdp_gob_t *gob,	// GOB to print
						FILE *fp,				// where to print it
						int detail,				// how much to print
						int indent);			// unused at this time

EP_STAT			_gdp_gob_init_vrfy_ctx(		// initialize for proof verification
						gdp_gob_t *gob);

EP_STAT			_gdp_gob_create(			// create a new GDP object
						gdp_md_t *gmd,
						gdp_name_t logdname,
						gdp_gob_t **pgob);

EP_STAT			_gdp_gob_open(				// open a GOB
						gdp_gob_t *gob,
						gdp_cmd_t cmd,
						gdp_open_info_t *open_info,
						gdp_chan_t *chan,
						uint32_t reqflags);

EP_STAT			_gdp_gob_close(				// close a GOB (handle)
						gdp_gob_t *gob,
						gdp_chan_t *chan,
						uint32_t reqflags);

EP_STAT			_gdp_gob_delete(			// delete and close a GOB (handle)
						gdp_gob_t *gob,
						gdp_chan_t *chan,
						uint32_t reqflags);

EP_STAT			_gdp_gob_read_by_recno(			// read GOB record based on datum
						gdp_gob_t *gob,
						gdp_recno_t recno,
						gdp_chan_t *chan,
						uint32_t reqflags,
						gdp_datum_t *datum);

EP_STAT			_gdp_gob_read_by_recno_async(	// read asynchronously
						gdp_gob_t *gob,
						gdp_gin_t *gin,
						gdp_recno_t recno,
						uint32_t nrecs,
						gdp_event_cbfunc_t cbfunc,
						void *cbarg,
						gdp_chan_t *chan);

EP_STAT			_gdp_gob_append_sync(		// append a record (gdpd shared)
						gdp_gob_t *gob,
						int n_datums,
						gdp_datum_t **datums,
						gdp_hash_t *prevhash,
						gdp_chan_t *chan,
						uint32_t reqflags);

EP_STAT			_gdp_gob_append_async(		// append asynchronously
						gdp_gob_t *gob,
						gdp_gin_t *gin,
						int n_datums,
						gdp_datum_t **datums,
						gdp_hash_t *prevhash,
						gdp_event_cbfunc_t cbfunc,
						void *cbarg,
						gdp_chan_t *chan,
						uint32_t reqflags);

EP_STAT			_gdp_gin_subscribe(			// subscribe to data
						gdp_gin_t *gin,
						gdp_cmd_t cmd,
						gdp_recno_t start,
						int32_t numrecs,
						gdp_sub_qos_t *qos,
						gdp_event_cbfunc_t cbfunc,
						void *cbarg);

EP_STAT			_gdp_gin_unsubscribe(		// delete subscriptions
						gdp_gin_t *gin,
						gdp_event_cbfunc_t cbfunc,
						void *cbarg,
						uint32_t reqflags);

EP_STAT			_gdp_gob_getmetadata(		// retrieve metadata
						gdp_gob_t *gob,
						gdp_md_t **gmdp,
						gdp_chan_t *chan,
						uint32_t reqflags);

EP_STAT			_gdp_gob_newsegment(		// create a new physical segment
						gdp_gob_t *gob,
						gdp_chan_t *chan,
						uint32_t reqflags);

EP_STAT			_gdp_gob_fwd_append(		// forward APPEND (replication)
						gdp_gob_t *gob,
						gdp_datum_t *datum,
						gdp_name_t to_server,
						gdp_event_cbfunc_t cbfunc,
						void *cbarg,
						gdp_chan_t *chan,
						uint32_t reqflags);

/*
**  GIN functions
*/

#define GDP_GIN_ISGOOD(gin)												\
				((gin) != NULL &&										\
				 EP_UT_BITSET(GOBF_INUSE, (gin)->flags) &&				\
				 GDP_GOB_ISGOOD((gin)->gob))
#define GDP_GIN_ASSERT_ISLOCKED(gin)									\
			_gdp_bool_null(												\
				EP_ASSERT(GDP_GIN_ISGOOD(gin)) &&						\
				EP_ASSERT(EP_UT_BITSET(GOBF_ISLOCKED, (gin)->flags)) &&	\
				EP_THR_MUTEX_ASSERT_ISLOCKED(&(gin)->mutex) &&			\
				GDP_GOB_ASSERT_ISLOCKED((gin)->gob)						\
			)
#define GDP_GIN_CHECK_RETURN_STAT(gin)									\
			do															\
			{															\
				if (!EP_ASSERT((gin) != NULL))							\
						return GDP_STAT_NULL_GOB;						\
				if (!EP_ASSERT(EP_UT_BITSET(GOBF_INUSE, (gin)->flags)))	\
						return GDP_STAT_LOG_NOT_OPEN;					\
			} while (false)
#define GDP_GIN_CHECK_RETURN_NULL(gin)									\
			do															\
			{															\
				if (!EP_ASSERT((gin) != NULL))							\
						return NULL;									\
				if (!EP_ASSERT(EP_UT_BITSET(GOBF_INUSE, (gin)->flags)))	\
						return NULL;									\
			} while (false)

gdp_gin_t		*_gdp_gin_new(				// create new GIN from GOB
						gdp_gob_t *gob);

void			_gdp_gin_free(				// free a GIN
						gdp_gin_t *gin);

void			_gdp_gin_lock_trace(		// lock the GIN mutex
						gdp_gin_t *gin,
						const char *file,
						int line,
						const char *id);
#define _gdp_gin_lock(g)		_gdp_gin_lock_trace(g, __FILE__, __LINE__, #g)

void			_gdp_gin_unlock_trace(		// unlock the GIN mutex
						gdp_gin_t *gin,
						const char *file,
						int line,
						const char *id);
#define _gdp_gin_unlock(g)		_gdp_gin_unlock_trace(g, __FILE__, __LINE__, #g)


/*
**  GOB cache.
**
**		Implemented in gdp/gdp_gob_cache.c.
*/

EP_STAT			_gdp_gob_cache_init(void);	// initialize cache

void			_gdp_gob_cache_dump(		// print cache (for debugging)
						int plev,
						FILE *fp);

typedef EP_STAT	gcl_open_func(
						gdp_gin_t *gcl,
						void *open_info);

EP_STAT			_gdp_gob_cache_get(		// get entry from cache
						gdp_name_t gcl_name,
						uint32_t flags,
						gdp_gob_t **pgob);

#define GGCF_NOCREATE		0			// dummy
#define GGCF_CREATE			0x00000001	// create cache entry if non existent
#define GGCF_GET_PENDING	0x00000002	// return "pending" entries
#define GGCF_PEEK			0x00000004	// don't update cache usage time

void			_gdp_gob_cache_add(			// add entry to cache
						gdp_gob_t *gob);

void			_gdp_gob_cache_changename(	// update the name of a cached GOB
						gdp_gob_t *gob,
						gdp_name_t newname);

void			_gdp_gob_cache_drop(		// drop entry from cache
						gdp_gob_t *gob,			// GOB to drop
						bool cleanup);			// set if doing cache cleanup

void			_gdp_gob_cache_reclaim(		// flush old entries
						time_t maxage);

void			_gdp_gob_cache_shutdown(	// immediately shut down cache
						void (*shutdownfunc)(gdp_req_t *));

void			_gdp_gob_touch(				// move to front of LRU list
						gdp_gob_t *gob);

void			_gdp_gob_cache_foreach(		// run over all cached GOBs
						void (*f)(gdp_gob_t *));

void			_gdp_gob_pr_stats(			// print (debug) GOB statistics
						FILE *fp);



/*
**  GOB Open Information
**
**		Passed from application into library to allow for all the
**		parameters we didn't think of when we first defined the API.
*/

struct gdp_open_info
{
	EP_CRYPTO_KEY		*signkey;			// signing key
	gdp_signkey_cb_t	*signkey_cb;		// callback to get signing key
	void				*signkey_udata;		// passed to signkey_cb
	uint32_t			flags;				// see below
};

// flags values
#define GOIF_KEEP_IN_CACHE		0x00000001	// defer GOB free
#define GOIF_VERIFY_PROOF		0x00000002	// when reading, verify datum
#define GOIF_NO_SKEY_NONFATAL	0x00000004	// missing secret key not fatal


/*
**  A Work Request (and associated Response)
**
**		A GDP request is packaged up in one of these things and
**		submitted.  Responses are returned in the same structure.
**
**		There are two PDU pointers:
**		* cpdu is the PDU with the command.  Generally this is
**			kept around until the response is read in case you
**			need to retransmit the command PDU.
**		* rpdu is the PDU with the response.
**
**		PDUs have an associated gdp_buf_t to store the actual
**		data.  That buffer does not have a write callback, so
**		it can be used without having any side effects.
**
**		The PDU includes the command/response code, the rid,
**		the record number, the timestamp, the data buffer,
**		and an optional signature buffer.
**
**		There can be mulitple requests active on a single GOB at
**		any time, but they should have unique rids.  Rids can be
**		reused if desired once an operation is complete.  Note:
**		some operations (e.g., subscriptions) can return multiple
**		results, but they will have the same rid.
**
**		Requests are potentially linked on lists.  Every request
**		that is active on a channel is linked to that channel
**		(with the GDP_REQ_ON_CHAN_LIST flag set); this is so that
**		requests can be cleaned up if the channel goes away.  At
**		this point we try to recover the channel, so this should
**		be rare, but that list is also used to find requests that
**		need to be timed out.
**
**		For active requests --- that is, requests that are either
**		waiting for a response (in _gdp_invoke) or represent
**		potential points for subscriptions --- are also linked to
**		the corresponding GOB, and will have the GDP_REQ_ON_GOB_LIST
**		flag set.  Subscription listeners also have the
**		GDP_REQ_CLT_SUBSCR flag set.  GDP_REQ_SRV_SUBSCR is used
**		by gdplogd to find the other end of the subscription, i.e,
**		subscription data producers.
**
**		In both the case of applications and gdplogd, requests may
**		get passed between threads.  To prevent someone from finding
**		a request on one of these lists and using it at the same time
**		someone else has it in use, you would like to lock the data
**		structure while it is active.  But you can't pass a mutex
**		between threads.  This is a particular problem if subscription
**		or multiread data comes in faster than it can be processed;
**		since the I/O thread is separate from the processing thread
**		things can clobber each other.
**
**		We solve this by assigning a state to each request:
**
**		FREE means that this request is on the free list.  It
**			should never appear in any other context.
**		ACTIVE means that there is currently an operation taking
**			place on the request, and no one other than the owner
**			should use it.  If you need it, you can wait on the
**			condition variable.
**		WAITING means that the request has been sent from a client
**			to a server but hasn't gotten the response yet.  It
**			shouldn't be possible for a WAITING request to also
**			have an active subscription, but it will be in the GOB
**			list.
**		IDLE means that the request is not free, but there is no
**			operation in process on it.  This will generally be
**			because it is a subscription that does not have any
**			currently active data.
**
**		If you want to deliver data to a subscription, you have to
**		first make sure the req is in IDLE state, turn it to ACTIVE
**		state, and then process it.  If it is not in IDLE state you
**		sleep on the condition variable and try again.
**
**		Passing a request to another thread is basically the same.
**		The invariant is that any req being passed between threads
**		should always be ACTIVE.
**
**		In some cases requests may have pending events.  This
**		occurs for commands such as SUBSCRIBE or MULTIREAD when
**		the first data return appears before the ack for the
**		initial command has finished processing.  To avoid confusing
**		applications you have to defer these events until the app
**		knows that the command succeeded.  This list is sorted by
**		seqno (that is, the sequence number of the PDU as issued
**		by the sender).  The next expected seqno is stored in
**		seqnext.
**
**		Implemented in gdp_req.c.
*/

struct gdp_req
{
	EP_THR_MUTEX		mutex;		// lock on this data structure
	EP_THR_COND			cond;		// pthread wakeup condition variable
	uint16_t			state;		// see below
	LIST_ENTRY(gdp_req)	goblist;	// linked list for cache management
	LIST_ENTRY(gdp_req)	chanlist;	// reqs associated with a given channel
	gdp_gob_t			*gob;		// associated GDP Object handle
	gdp_pdu_t			*cpdu;		// PDU for commands
	gdp_pdu_t			*rpdu;		// PDU for ack/nak responses
	gdp_chan_t			*chan;		// the network channel for this req
	EP_STAT				stat;		// status code from last operation
	gdp_recno_t			nextrec;	// next record to return (subscriptions)
	int64_t				s_results;	// number of results sent
	int32_t				numrecs;	// remaining number of records to return
	uint32_t			flags;		// see below
	void				(*postproc)(struct gdp_req *);
									// do post processing after ack sent
	EP_TIME_SPEC		act_ts;		// timestamp of last successful activity
	EP_TIME_SPEC		sub_ts;		// time of current subscription lease start
	gdp_event_cbfunc_t	sub_cbfunc;	// callback function (subscribe & async I/O)
	void				*sub_cbarg;	// user-supplied opaque data to cb

	// these are only of interest in clients, never in gdplogd
	gdp_gin_t			*gin;		// GIN handle (client only, may be NULL)
	int64_t				r_results;	// number of results received so far
	struct gev_list		events;		// pending events (see above)
	gdp_seqno_t			seqnext;	// next expected seqno
	struct event		*ev_to;		// event timeout (to scan pending events)
};

// states
#define GDP_REQ_FREE			0			// request is free
#define GDP_REQ_ACTIVE			1			// currently being processed
#define GDP_REQ_WAITING			2			// waiting on cond variable
#define GDP_REQ_IDLE			3			// subscription waiting for data

// flags
#define GDP_REQ_ASYNCIO			0x00000001	// async I/O operation
#define GDP_REQ_DONE			0x00000002	// operation complete
#define GDP_REQ_CLT_SUBSCR		0x00000004	// client-side subscription
#define GDP_REQ_SRV_SUBSCR		0x00000008	// server-side subscription
#define GDP_REQ_PERSIST			0x00000010	// request persists after response
#define GDP_REQ_SUBUPGRADE		0x00000020	// can upgrade to subscription
#define GDP_REQ_ALLOC_RID		0x00000040	// force allocation of new rid
#define GDP_REQ_ON_GOB_LIST		0x00000080	// this is on a GOB list
#define GDP_REQ_ON_CHAN_LIST	0x00000100	// this is on a channel list
#define GDP_REQ_VRFY_CONTENT	0x00000200	// verify content proof
#define GDP_REQ_ROUTEFAIL		0x00000400	// fail immediately on route failure

EP_STAT			_gdp_req_new(				// create new request
						gdp_cmd_t cmd,
						gdp_gob_t *gob,
						gdp_chan_t *chan,
						gdp_pdu_t *pdu,
						uint32_t flags,
						gdp_req_t **reqp);

void			_gdp_req_free(				// free old request
						gdp_req_t **reqp);

EP_STAT			_gdp_req_lock(				// lock a request mutex
						gdp_req_t *);

void			_gdp_req_unlock(			// unlock a request mutex
						gdp_req_t *);

gdp_req_t		*_gdp_req_find(				// find a request in a GOB
						gdp_gob_t *gob, gdp_rid_t rid);

gdp_rid_t		_gdp_rid_new(				// create new request id
						gdp_gob_t *gob, gdp_chan_t *chan);

EP_STAT			_gdp_req_send(				// send request to daemon (async)
						gdp_req_t *req);

EP_STAT			_gdp_req_unsend(			// pull failed request off GOB list
						gdp_req_t *req);

EP_STAT			_gdp_req_dispatch(			// do local req processing
						gdp_req_t *req,
						int cmd);

EP_STAT			_gdp_invoke(				// send request to daemon (sync)
						gdp_req_t *req);

void			_gdp_req_freeall(			// free all requests in GOB list
						gdp_gob_t *gob,
						gdp_gin_t *gin,
						void (*shutdownfunc)(gdp_req_t *));

void			_gdp_req_dump(				// print (debug) request
						const gdp_req_t *req,
						FILE *fp,
						int detail,
						int indent);

void			_gdp_req_pr_stats(			// print (debug) statistics
						FILE *fp);

EP_STAT			_gdp_req_ack_resp(			// helper: create ACK response
						gdp_req_t *req,			// active request
						gdp_cmd_t ack_type);	// actual ACK command

EP_STAT			_gdp_req_nak_resp(			// helper: create NAK response
						gdp_req_t *req,			// active request
						gdp_cmd_t nak_type,		// actual NAK command
						const char *detail,		// text detail
						EP_STAT estat);			// status code detail


/*
**  Channel and I/O Event support
*/

// extended channel information (passed as channel "cdata")
struct gdp_chan_x
{
	struct req_head		reqs;			// reqs associated with this channel
	EP_STAT				(*connect_cb)(	// called on connection established
							gdp_chan_t *chan);
	long				adv_intvl;		// advertising interval
	struct event		*adv_timer;		// re-advertise event (gdplogd only)
};

// functions used internally related to channel I/O
EP_STAT			_gdp_io_recv(
						gdp_chan_t *chan,
						gdp_name_t src,
						gdp_name_t dst,
						gdp_seqno_t seqno,
						gdp_buf_t *payload_buf,
						size_t payload_len);

EP_STAT			_gdp_io_event(
						gdp_chan_t *chan,
						uint32_t flags);

// router control information
EP_STAT			_gdp_router_event(
						gdp_chan_t *chan,
						gdp_name_t src,
						gdp_name_t dst,
						size_t payload_len,
						EP_STAT estat);

// I/O event handling
struct event_loop_info
{
	const char		*where;
};

void			*_gdp_run_event_loop(
						void *eli_);

void			_gdp_stop_event_loop(void);

typedef void	libevent_event_t(
						evutil_socket_t sock,
						short what,
						void *cbarg);

void			_gdp_evloop_timer_set(
						uint32_t timeout,			// in timeout usec
						libevent_event_t *cbfunc,	// ... call cbfunc
						void *cbarg,				// ... with this arg
						struct event **pev);		// ... stored here

void			_gdp_evloop_timer_clr(
						struct event **pev);


/*
**  Protobuf message handling.  These are only for the unpacked
**  (in memory, unserialized) version; the rest is hidden in
**  gdp_pdu.[ch].
*/

gdp_msg_t		*_gdp_msg_new(				// create new message
					gdp_cmd_t cmd,
					gdp_rid_t rid,
					gdp_l5seqno_t l5seqno);

void			_gdp_msg_free(				// free a message
					gdp_msg_t **pmsg);

void			_gdp_msg_dump(				// print a message for debugging
					const gdp_msg_t *msg,
					FILE *fp,
					int indent);


/*
**  Structure used for registering command functions
**
**		The names are already known to the GDP library, so this is just
**		to bind functions that implement the individual commands.
*/

typedef EP_STAT	cmdfunc_t(			// per command dispatch entry
					gdp_req_t *req);	// the request to be processed

struct cmdfuncs
{
	int			cmd;				// command number
	cmdfunc_t	*func;				// pointer to implementing function
};

void			_gdp_register_cmdfuncs(
						struct cmdfuncs *);

const char		*_gdp_proto_cmd_name(		// return printable cmd name
						uint8_t cmd);

#define GDP_RECLAIM_AGE_DEF		300L		// default reclaim age (sec)


/*
**  Advertising.
*/

EP_STAT			_gdp_advertise_me(			// advertise me only
						gdp_chan_t *chan,
						int cmd,
						void *ctx);

/*
**  Subscriptions.
*/

#define GDP_SUBSCR_REFRESH_DEF	60L			// default refresh interval (sec)
#define GDP_SUBSCR_TIMEOUT_DEF	180L		// default timeout (sec)

extern EP_THR_MUTEX		_GdpSubscriptionMutex;
extern struct req_head	_GdpSubscriptionRequests;

void			_gdp_subscr_lost(			// subscription disappeared
						gdp_req_t *req);

void			_gdp_subscr_poke(			// test subscriptions still alive
						gdp_chan_t *chan);

/*
**  Initialization and Maintenance.
*/

void			_gdp_newname(gdp_name_t gname,
						gdp_md_t *gmd);

void			_gdp_reclaim_resources(		// reclaim system resources
						void *);				// unused

void			_gdp_reclaim_resources_init(
						void (*f)(int, short, void *));

void			_gdp_dump_state(int plev);

gdp_cmd_t		_gdp_acknak_from_estat(		// produce acknak code from status
						EP_STAT estat,			// status to evaluate
						gdp_cmd_t def);			// use this if nothing better...

/*
**  Cryptography support
*/

EP_CRYPTO_KEY	*_gdp_crypto_skey_read(		// read a secret key
						const char *searchpath,
						const char *filename);
void			_gdp_sign_metadata(			// sign the metadata
						gdp_gin_t *gin);


/*
**  Orders for lock acquisition
**		Lower numbered locks should be acquired before higher numbered locks.
*/

#define GDP_MUTEX_LORDER_GIN		6
#define GDP_MUTEX_LORDER_GOBCACHE	8
#define GDP_MUTEX_LORDER_GOB		10
#define GDP_MUTEX_LORDER_REQ		12
#define GDP_MUTEX_LORDER_CHAN		14
#define GDP_MUTEX_LORDER_DATUM		18
#define GDP_MUTEX_LORDER_LEAF		31	// freelists, etc.


/*
**  Utility routines
*/

const char		*_gdp_pr_indent(			// return indenting for debug output
						int indent);

void			_gdp_adm_readparams(		// read admin params w/ versioning
						const char *name);		// base of config name

EP_STAT			_gdp_adm_path_find(			// find file in path via getstrparam
						const char *dir_param,
						const char *dir_def,
						const char *file_param,
						const char *file_def,
						char *path_buf,
						size_t path_buf_len);

void			_gdp_show_elapsed(			// show elapsed time
						const char *func,		// function name
						gdp_cmd_t cmd,			// associated command
						EP_TIME_SPEC *start);	// starting time


/*
**  Convenience macros
*/

#define MICROSECONDS	* INT64_C(1000)
#define MILLISECONDS	* INT64_C(1000000)
#define SECONDS			* INT64_C(1000000000)


/*
**  Low level support for cracking protocol, computing hashes, etc.
*/

#define PUT8(v) \
		{ \
			*pbp++ = ((v) & 0xff); \
		}
#define PUT16(v) \
		{ \
			*pbp++ = ((v) >> 8) & 0xff; \
			*pbp++ = ((v) & 0xff); \
		}
#define PUT24(v) \
		{ \
			*pbp++ = ((v) >> 16) & 0xff; \
			*pbp++ = ((v) >> 8) & 0xff; \
			*pbp++ = ((v) & 0xff); \
		}
#define PUT32(v) \
		{ \
			*pbp++ = ((v) >> 24) & 0xff; \
			*pbp++ = ((v) >> 16) & 0xff; \
			*pbp++ = ((v) >> 8) & 0xff; \
			*pbp++ = ((v) & 0xff); \
		}
#define PUT48(v) \
		{ \
			*pbp++ = ((v) >> 40) & 0xff; \
			*pbp++ = ((v) >> 32) & 0xff; \
			*pbp++ = ((v) >> 24) & 0xff; \
			*pbp++ = ((v) >> 16) & 0xff; \
			*pbp++ = ((v) >> 8) & 0xff; \
			*pbp++ = ((v) & 0xff); \
		}
#define PUT64(v) \
		{ \
			*pbp++ = ((v) >> 56) & 0xff; \
			*pbp++ = ((v) >> 48) & 0xff; \
			*pbp++ = ((v) >> 40) & 0xff; \
			*pbp++ = ((v) >> 32) & 0xff; \
			*pbp++ = ((v) >> 24) & 0xff; \
			*pbp++ = ((v) >> 16) & 0xff; \
			*pbp++ = ((v) >> 8) & 0xff; \
			*pbp++ = ((v) & 0xff); \
		}

#define GET8(v) \
		{ \
				v  = *pbp++; \
		}
#define GET16(v) \
		{ \
				v  = *pbp++ << 8; \
				v |= *pbp++; \
		}
#define GET24(v) \
		{ \
				v  = *pbp++ << 16; \
				v |= *pbp++ << 8; \
				v |= *pbp++; \
		}
#define GET32(v) \
		{ \
				v  = *pbp++ << 24; \
				v |= *pbp++ << 16; \
				v |= *pbp++ << 8; \
				v |= *pbp++; \
		}
#define GET48(v) \
		{ \
				v  = ((uint64_t) *pbp++) << 40; \
				v |= ((uint64_t) *pbp++) << 32; \
				v |= ((uint64_t) *pbp++) << 24; \
				v |= ((uint64_t) *pbp++) << 16; \
				v |= ((uint64_t) *pbp++) << 8; \
				v |= ((uint64_t) *pbp++); \
		}
#define GET64(v) \
		{ \
				v  = ((uint64_t) *pbp++) << 56; \
				v |= ((uint64_t) *pbp++) << 48; \
				v |= ((uint64_t) *pbp++) << 40; \
				v |= ((uint64_t) *pbp++) << 32; \
				v |= ((uint64_t) *pbp++) << 24; \
				v |= ((uint64_t) *pbp++) << 16; \
				v |= ((uint64_t) *pbp++) << 8; \
				v |= ((uint64_t) *pbp++); \
		}


#endif // _GDP_PRIV_H_
