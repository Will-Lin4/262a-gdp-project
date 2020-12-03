/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	GDP.H --- public headers for use of the Swarm Global Data Plane
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

#ifndef _GDP_H_
#define _GDP_H_

#include <gdp/gdp_buf.h>
#include <gdp/gdp_stat.h>

#include <ep/ep.h>
#include <ep/ep_crypto.h>
#include <ep/ep_mem.h>
#include <ep/ep_stat.h>
#include <ep/ep_time.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

#include <inttypes.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <sys/types.h>

//------------------------------------------------------------------------------
//	Added for C++ compability
extern "C"{
//------------------------------------------------------------------------------

/**********************************************************************
**	Opaque structures
*/

// an open GDP Object Instance (GIN) handle
typedef struct gdp_gin		gdp_gin_t;
typedef struct gdp_gin		gdp_gcl_t EP_ATTR_DEPRECATED;	// back compat

// GDP Object metadata
typedef struct gdp_md		gdp_md_t;
typedef uint32_t			gdp_md_id_t;

// hash functions and signatures
typedef struct gdp_buf		gdp_hash_t;		//XXX is this right?
typedef struct gdp_buf		gdp_sig_t;		//XXX is this right?

// additional information when creating/opening logs (e.g., keys, qos, hints)
typedef struct gdp_create_info	gdp_create_info_t;
typedef struct gdp_open_info	gdp_open_info_t;

// quality of service information for subscriptions
typedef struct gdp_sub_qos		gdp_sub_qos_t;

/**********************************************************************
**	Other data types
*/

// the internal name of a GDP Object (256 bits)
typedef uint8_t				gdp_name_t[32];

#define GDP_NAME_SAME(a, b)	(memcmp((a), (b), sizeof (gdp_name_t)) == 0)

// the printable name of a GDP Object
#define GDP_GOB_PNAME_LEN	43			// length of an encoded pname
typedef char				gdp_pname_t[GDP_GOB_PNAME_LEN + 1];

// a GOB record number
typedef int64_t				gdp_recno_t;
#define PRIgdp_recno		PRId64

/*
**	I/O modes
*/

typedef enum
{
	GDP_MODE_ANY =		0x0003,			// no mode specified (= RA)
	GDP_MODE_RO =		0x0001,			// readable
	GDP_MODE_AO =		0x0002,			// appendable
	GDP_MODE_RA =		0x0003,			// read + append
	GDP_MODE_MASK =		0x0003,			// mask for primary mode bits

	// following are bit masks
	_GDP_MODE_PEEK =	0x0100,			// "peek": for stats, no ref counting
										//    in _gdp_gob_cache_get
} gdp_iomode_t;


/*
**  GOB Metadata keys
**
**		Although defined as integers, by convention metadata keys are
**		four ASCII characters, kind of like a file extension.  By further
**		convention, those with fewer than four non-zero characters are
**		system defined and may have semantics built in.  Names that are
**		four characters are reserved for user applications.  Names with
**		zero bytes in the middle are probably a bad idea, albeit legal.
*/

#define GDP_MD_XID			0x00584944	// XID (external id)
#define GDP_MD_PUBKEY		0x00505542	// PUB (public key, deprecated)
#define GDP_MD_OWNERPUBKEY	0x004F504B	// OPK (owner public key)
#define GDP_MD_WRITERPUBKEY	0x0057504B	// WPK (writer public key)
#define GDP_MD_CTIME		0x0043544D	// CTM (creation time)
#define GDP_MD_EXPIRE		0x0058544D	// XTM (expiration date/time)
#define GDP_MD_CREATOR		0x00434944	// CID (creator id)
#define GDP_MD_SYNTAX		0x0053594E	// SYN (data syntax: json, xml, etc.)
#define GDP_MD_LOCATION		0x004C4F43	// LOC (location: lat/long)
#define GDP_MD_NONCE		0x004E4F4E	// NON (unique nonce)


/*
**	 Datums
**		These are the underlying data unit that is passed through a GOB.
*/

typedef struct gdp_datum	gdp_datum_t;


/*
**	Events
**		gdp_event_t encodes an event.  Every event has a type and may
**		optionally have a GIN handle and/or a message.  For example,
**		data (from a subscription) has all three.
*/

typedef struct gdp_event	gdp_event_t;

// event types
#define _GDP_EVENT_FREE		0	// internal use: event is free
#define GDP_EVENT_DATA		1	// 205 returned data
#define GDP_EVENT_DONE		2	// 205 normal end of async read
#define GDP_EVENT_SHUTDOWN	3	// 515 subscription terminating because of shutdown
#define GDP_EVENT_CREATED	4	// 201 successful append, create, or similar
#define GDP_EVENT_SUCCESS	5	// 200 generic asynchronous success status
#define GDP_EVENT_FAILURE	6	//     generic asynchronous failure status
#define GDP_EVENT_MISSING	7	// 430 record is missing

extern gdp_event_t		*gdp_event_next(		// get event (caller must free!)
							gdp_gin_t *gin,			// if set wait for this GIN only
							EP_TIME_SPEC *timeout);

extern EP_STAT			gdp_event_free(			// free event from gdp_event_next
							gdp_event_t *gev);		// event to free

extern void				gdp_event_print(		// print event (user access)
							const gdp_event_t *gev,	// event in question
							FILE *fp);				// output file

extern void				gdp_event_debug(		// print event (for debugging)
							const gdp_event_t *gev,	// event in question
							FILE *fp,				// output file
							int detail,				// how detailed?
							int indent);			// indentation level

extern int				gdp_event_gettype(		// get the type of the event
							gdp_event_t *gev);

extern EP_STAT			gdp_event_getstat(		// get status code
							gdp_event_t *gev);

extern gdp_gin_t		*gdp_event_getgin(		// get the GIN of the event
							gdp_event_t *gev);

extern gdp_datum_t		*gdp_event_getdatum(	// get the datum of the event
							gdp_event_t *gev);

extern void				*gdp_event_getudata(	// get user data (callback only)
							gdp_event_t *gev);

typedef void			(*gdp_event_cbfunc_t)(	// the callback function
							gdp_event_t *ev);		// the event triggering the call

/**********************************************************************
**	Public globals and functions
*/

//extern struct event_base		*GdpIoEventBase;	// the base for GDP I/O events

// initialize the GDP library, inc running event loop, system connect, etc.
extern EP_STAT	gdp_init(
					const char *gdpd_addr);	// address of gdpd

extern EP_STAT	gdp_init2(			// same, but two arguments
					const char *gdpd_addr,	// address of gdpd
					uint32_t flags);		// tweak operations, see below

// pre-initialize the library (gdp_init does this -- rarely needed)
// includes event loop setup (not run), cache setup, signal handling
EP_STAT			gdp_lib_init(
					const char *progname,
					const char *my_routing_name,
					uint32_t flags);

// pre-pre-initialize basic libraries (gdp_lib_init does this -- only
// used by programs that don't actually talk to the GDP itself)
EP_STAT			gdp_init_phase_0(
					const char *progname,
					uint32_t flags);

#define GDP_INIT_NO_HONGDS		0x00000001	// don't initialize HONGDS
#define GDP_INIT_NO_ZEROCONF	0x00000002	// don't use zeroconf
#define GDP_INIT_OPT_HONGDS		0x00000004	// HONGDS is optional

// run event loop (normally run from gdp_init; never returns)
extern void		*gdp_run_accept_event_loop(
					void *);				// unused

// create a new GOB
extern EP_STAT	gdp_gin_create(
					gdp_create_info_t *gci,
					const char *external_name,
					gdp_gin_t **pgin);

// open an existing GOB
extern EP_STAT	gdp_gin_open(
					gdp_name_t name,		// GOB name to open
					gdp_iomode_t rw,		// read/write (append)
					gdp_open_info_t *info,	// additional open info
					gdp_gin_t **gin);		// pointer to result GIN handle

// close an open GIN
extern EP_STAT	gdp_gin_close(
					gdp_gin_t *gin);		// GIN handle to close

// delete and close an open GIN
extern EP_STAT	gdp_gin_delete(
					gdp_gin_t *gin);		// GIN handle to delete

// append to a writable GIN
extern EP_STAT	gdp_gin_append(
					gdp_gin_t *gin,			// writable GIN handle
					gdp_datum_t *datum,		// message to write
					gdp_hash_t *prevhash);	// hash of previous record

// async version
extern EP_STAT gdp_gin_append_async(
					gdp_gin_t *gin,			// writable GIN handle
					int32_t n_datums,		// number of datums being written
					gdp_datum_t **datums,	// list of datums to append
					gdp_hash_t *prevhash,	// hash of previous record
					gdp_event_cbfunc_t,		// callback function
					void *udata);

// synchronous read based on record number
extern EP_STAT gdp_gin_read_by_recno(
					gdp_gin_t *gin,			// readable GIN handle
					gdp_recno_t recno,		// record number
					gdp_datum_t *datum);	// pointer to result

// async read based on record number
extern EP_STAT gdp_gin_read_by_recno_async(
					gdp_gin_t *gin,			// readable GIN handle
					gdp_recno_t recno,		// starting record number
					int32_t nrecs,			// number of records to read
					gdp_event_cbfunc_t cbfunc,	// callback function
					void *cbarg);			// argument to cbfunc

// synchronous read based on timestamp
extern EP_STAT gdp_gin_read_by_ts(
					gdp_gin_t *gin,			// readable GIN handle
					EP_TIME_SPEC *ts,		// timestamp
					gdp_datum_t *datum);	// pointer to result

// async read based on timestamp
extern EP_STAT gdp_gin_read_by_ts_async(
					gdp_gin_t *gin,			// readable GIN handle
					EP_TIME_SPEC *ts,		// starting record number
					int32_t nrecs,			// number of records to read
					gdp_event_cbfunc_t cbfunc,	// callback function
					void *cbarg);			// argument to cbfunc

// synchronous read based on hash
extern EP_STAT gdp_gin_read_by_hash(
					gdp_gin_t *gin,			// readable GIN handle
					gdp_hash_t *hash,		// hash of desired record
					gdp_datum_t *datum);	// pointer to result

// async read based on hash
extern EP_STAT gdp_gin_read_by_hash_async(
					gdp_gin_t *gin,			// readable GIN handle
					uint32_t n_hashes,		// number of records to fetch
					gdp_hash_t **hashes,	// list of hashes to fetch
					gdp_event_cbfunc_t cbfunc,	// callback function
					void *cbarg);			// argument to cbfunc

// subscribe based on record number
extern EP_STAT	gdp_gin_subscribe_by_recno(
					gdp_gin_t *gin,			// readable GIN handle
					gdp_recno_t start,		// starting record number
					int32_t nrecs,			// number of records to retrieve
					gdp_sub_qos_t *qos,		// quality of service info
					gdp_event_cbfunc_t cbfunc,
											// callback function for next datum
					void *cbarg);			// argument passed to callback

// subscribe based on timestamp
extern EP_STAT	gdp_gin_subscribe_by_ts(
					gdp_gin_t *gin,			// readable GIN handle
					EP_TIME_SPEC *ts,		// starting timestamp
					int32_t nrecs,			// number of records to retrieve
					gdp_sub_qos_t *qos,		// quality of service info
					gdp_event_cbfunc_t cbfunc,
											// callback function for next datum
					void *cbarg);			// argument passed to callback

// subscribe based on hash
extern EP_STAT	gdp_gin_subscribe_by_hash(
					gdp_gin_t *gin,			// readable GIN handle
					gdp_hash_t *hash,		// starting record hash
					int32_t nrecs,			// number of records to retrieve
					EP_TIME_SPEC *timeout,	// timeout
					gdp_event_cbfunc_t cbfunc,
											// callback function for next datum
					void *cbarg);			// argument passed to callback


// unsubscribe from a GIN
extern EP_STAT	gdp_gin_unsubscribe(
					gdp_gin_t *gin,			// GIN handle
					gdp_event_cbfunc_t cbfunc,
											// callback func (to make unique)
					void *cbarg);			// callback arg (to make unique)
// read metadata
extern EP_STAT	gdp_gin_getmetadata(
					gdp_gin_t *gin,			// GIN handle
					gdp_md_t **gmdp);		// out-param for metadata

// set append filter
extern EP_STAT	gdp_gin_set_append_filter(
					gdp_gin_t *gin,			// GIN handle
					EP_STAT (*readfilter)(gdp_datum_t *, void *),
					void *filterdata);

// set read filter
extern EP_STAT	gdp_gin_set_read_filter(
					gdp_gin_t *gin,			// GIN handle
					EP_STAT (*readfilter)(gdp_datum_t *, void *),
					void *filterdata);

// return the name of a GIN
//		XXX: should this be in a more generic "getstat" function?
extern const gdp_name_t *gdp_gin_getname(
					const gdp_gin_t *gin);	// open GIN handle

// return the hash algorithm
extern int		gdp_gin_gethashalg(
					const gdp_gin_t *gin);	// open GIN handle

// return the signature algorithm
extern int		gdp_gin_getsigalg(
					const gdp_gin_t *gin);	// open GIN handle

// check to see if a GDP object name is valid
extern bool		gdp_name_is_valid(
					const gdp_name_t);

// print a GIN (for debugging)
extern void		gdp_gin_print(
					const gdp_gin_t *gin,	// GIN handle to print
					FILE *fp);

// make a printable GDP object name from a binary version
char			*gdp_printable_name(
					const gdp_name_t internal,
					gdp_pname_t external);

// print an internal name for human use
void			gdp_print_name(
					const gdp_name_t internal,
					FILE *fp);

// make a binary GDP object name from a printable version
EP_STAT			gdp_internal_name(
					const gdp_pname_t printable,
					gdp_name_t internal);

// parse a (possibly human-friendly) GDP object name
#define GDP_HUMAN_NAME_MAX		255		// maximum length of human-oriented name
EP_STAT			gdp_name_parse(
					const char *hname,		// human text version of name
					gdp_name_t gname,		// output: internal name
					char const **xnamep);	// output: extended version of hname
EP_STAT			gdp_parse_name(			// obsolete: back compat
					const char *hname,
					gdp_name_t gname);

EP_STAT			gdp_name_resolve(
					const char *hname,
					gdp_name_t gname);

EP_STAT			gdp_name_update(
					const char *hname,
					const gdp_name_t gname);

EP_STAT			gdp_name_root_set(
					const char *root);

const char		*gdp_name_root_get(
					void);

// get the number of records in the log
extern gdp_recno_t	gdp_gin_getnrecs(
					const gdp_gin_t *gin);	// open GIN handle

/*
**  GOB Creation Information
**
**		Once the create_info is set it can be passed to the actual
**		creation routine, which will fill in any unspecified but
**		required fields and use that for the final object creation.
**
**		The only non-obvious part of this should be key management.
**		There are multiple keys possible, notably an "admin" (owner)
**		keypair and a "writer" keypair.  If only admin is specified
**		then it is also used as the writer key.
**
**		The "symbolic" creates take a string that encodes the other
**		parameters, e.g., "RSA2048-SHA256" might represent a SHA256
**		hash (digest) algorithm with a 2048-bit RSA signature.  A
**		null parameter would take defaults.
**
**		None of the "create" routines saves the secret key to disk;
**		that's only done when the "save_keys" routine is called.
**		That routine has to specify the on-disk symmetric crypto
**		cipher and associated password, if any.  If unspecified the
**		algorithm uses a site-specific default but the password
**		has to be explicit; if it is missing an interactive input
**		will be solicited.
**
**		An expected calling sequence might be:
**			info = new()
**			set_xxx(info, ...);
**			create_key(info, ...);
**			save_keys(info, ...);
**			create(info, ...);
**			free(info);
**		with (hopefully) obvious status checks.
*/

// get a new creation information structure
gdp_create_info_t	*gdp_create_info_new(void);

// free that structure
void				gdp_create_info_free(
						gdp_create_info_t **pinfo);

// set various parameters (names should hopefully be obvious)
EP_STAT				gdp_create_info_set_creator(	// creator (email)
						gdp_create_info_t *info,
						const char *user,
						const char *domain);
EP_STAT				gdp_create_info_set_expiration(	// expiration date
						gdp_create_info_t *info,
						uint64_t expiration);			// in seconds from now
EP_STAT				gdp_create_info_set_creation_service(
						gdp_create_info_t *info,
						const char *x_service_name);

// create new keypair (in memory, not on disk)
EP_STAT				gdp_create_info_new_owner_key(
						gdp_create_info_t *info,
						const char *dig_alg_name,		// digest algorithm
						const char *key_alg_name,		// signing algorithm
						int bits,
						const char *curve_name,
						const char *key_enc_alg_name);	// on-disk crypto alg
EP_STAT				gdp_create_info_new_writer_key(
						gdp_create_info_t *info,
						const char *dig_alg_name,		// digest algorithm
						const char *key_alg_name,		// signing algorithm
						int bits,
						const char *curve_name,
						const char *key_enc_alg_name);	// on-disk crypto alg

// set already existing keypair
EP_STAT				gdp_create_info_set_owner_key(
						gdp_create_info_t *info,
						EP_CRYPTO_KEY *key,
						const char *dig_alg_name);		// digest algorithm
EP_STAT				gdp_create_info_set_writer_key(
						gdp_create_info_t *info,
						EP_CRYPTO_KEY *key,
						const char *dig_alg_name);		// digest algorithm

// save stored keypairs (both admin and writer) to disk
EP_STAT				gdp_create_info_save_keys(
						gdp_create_info_t *info,
						uint32_t key_ctx,
						const char *key_enc_alg_name,
						const char *key_enc_password);

// add arbitrary user metadata (escape hatch for extensions)
EP_STAT				gdp_create_info_add_metadata(
						gdp_create_info_t *info,
						uint32_t md_name,
						size_t md_len,
						const void *md_val);

/*
**  GOB Open Information
*/

// get a new open information structure
gdp_open_info_t		*gdp_open_info_new(void);

// free that structure
void				gdp_open_info_free(
						gdp_open_info_t *info);

// set the signing key
EP_STAT				gdp_open_info_set_signing_key(
						gdp_open_info_t *info,
						EP_CRYPTO_KEY *skey);

// set the callback function to read a signing key from a user
typedef EP_STAT		gdp_signkey_cb_t(
							gdp_name_t gname,
							void *signkey_udata,
							EP_CRYPTO_KEY **skey);

EP_STAT				gdp_open_info_set_signkey_cb(
						gdp_open_info_t *info,
						gdp_signkey_cb_t *signkey_cb,
						void *signkey_udata);

// set the caching behavior
EP_STAT				gdp_open_info_set_caching(
						gdp_open_info_t *info,
						bool keep_in_cache);

// set data verification behavior
EP_STAT				gdp_open_info_set_vrfy(
						gdp_open_info_t *info,
						bool verify_proof);

// allow open to succeed even if no secret key found
EP_STAT				gdp_open_info_set_no_skey_nonfatal(
						gdp_open_info_t *info,
						bool no_skey_nonfatal);

/*
**  Metadata handling
*/

// create a new metadata set
gdp_md_t		*gdp_md_new(
					int entries);

// free a metadata set
void			gdp_md_free(gdp_md_t *gmd);

// add an entry to a metadata set
EP_STAT			gdp_md_add(
					gdp_md_t *gmd,
					gdp_md_id_t id,
					size_t len,
					const void *data);

// get an entry from a metadata set by index
EP_STAT			gdp_md_get(
					gdp_md_t *gmd,
					int indx,
					gdp_md_id_t *id,
					size_t *len,
					const void **data);

// get an entry from a metadata set by id
EP_STAT			gdp_md_find(
					gdp_md_t *gmd,
					gdp_md_id_t id,
					size_t *len,
					const void **data);

// print metadata set (for debugging)
void			gdp_md_dump(
					const gdp_md_t *gmd,
					FILE *fp,
					int detail,
					int indent);

/*
**  Datum handling
*/

// allocate a new message datum
gdp_datum_t		*gdp_datum_new(void);

// free a message datum
void			gdp_datum_free(gdp_datum_t *);

// reset a datum to clean state
void			gdp_datum_reset(gdp_datum_t *);

// copy contents of one datum into another
extern void		gdp_datum_copy(
					gdp_datum_t *to,
					const gdp_datum_t *from);

// compute hash of a datum for a given log
gdp_hash_t		*gdp_datum_hash(
						gdp_datum_t *datum,
						gdp_gin_t *gin);

// check datum for equality to hash
bool			gdp_datum_hash_equal(
						gdp_datum_t *datum,
						const gdp_gin_t *gin,
						const gdp_hash_t *hash);

// check the signature on a datum
EP_STAT			gdp_datum_vrfy(
						gdp_datum_t *datum,
						gdp_gin_t *gin);

// print out data record
extern void		gdp_datum_print(
					const gdp_datum_t *datum,	// message to print
					FILE *fp,					// file to print it to
					uint32_t flags);			// formatting options

#define GDP_DATUM_PRTEXT		0x00000001		// print data as text
#define GDP_DATUM_PRDEBUG		0x00000002		// print debugging info
#define GDP_DATUM_PRSIG			0x00000004		// print the signature
#define GDP_DATUM_PRQUIET		0x00000008		// don't print any metadata
#define GDP_DATUM_PRMETAONLY	0x00000010		// only print metadata
#define GDP_DATUM_PRBINARY		0x00000020		// output data as binary

// get the record number from a datum
extern gdp_recno_t	gdp_datum_getrecno(
					const gdp_datum_t *datum);

// get the timestamp from a datum
extern void		gdp_datum_getts(
					const gdp_datum_t *datum,
					EP_TIME_SPEC *ts);

// get the data length from a datum
extern size_t	gdp_datum_getdlen(
					const gdp_datum_t *datum);

// get the data buffer from a datum
extern gdp_buf_t *gdp_datum_getbuf(
					const gdp_datum_t *datum);

// get the signature from a datum
extern gdp_sig_t *gdp_datum_getsig(
					const gdp_datum_t *datum);


/*
**  Hashes
*/

// create a new hash structure
extern gdp_hash_t	*gdp_hash_new(
						int alg,				// hash algorithm
						void *hashbytes,		// if set, initial bytes
						size_t hashlen);		// length of hashbytes

// free a hash structure
extern void			gdp_hash_free(
						gdp_hash_t *hash);

// reset a hash structure
extern void			gdp_hash_reset(
						gdp_hash_t *hash);

// set the hash value; implies gdp_hash_reset
extern void			gdp_hash_set(
						gdp_hash_t *hash,		// existing hash
						void *hashbytes,		// actual hash buffer
						size_t hashlen);		// length of hashbytes

// get the length of a hash
extern size_t		gdp_hash_getlength(
						gdp_hash_t *hash);

// get the actual hash value (and optionally length)
extern void			*gdp_hash_getptr(
						gdp_hash_t *hash,
						size_t *hashlen_ptr);

// compare two hash structures
extern bool			gdp_hash_equal(
						const gdp_hash_t *a,
						const gdp_hash_t *b);


/*
**  Signatures
*/

// create a new empty signature structure
extern gdp_sig_t	*gdp_sig_new(
						int alg,				// signature algorithm
						void *sigbytes,			// if set, initial bytes
						size_t siglen);			// length of sigbytes

// free a signature structure
extern void			gdp_sig_free(
						gdp_sig_t *sig);

// reset a signature structure
extern void			gdp_sig_reset(
						gdp_sig_t *sig);

// set a signature; implies gdp_sig_reset
extern void			gdp_sig_set(
						gdp_sig_t *sig,			// existing signature
						void *sigbytes,			// actual signature value
						size_t siglen);			// length of sigbytes

// copy on signature into another
extern void			gdp_sig_copy(
						gdp_sig_t *from,
						gdp_sig_t *to);

// duplicate a signature into a new structure
extern gdp_sig_t	*gdp_sig_dup(
						gdp_sig_t *sig);

// get the length of a signature (not the number of key bits!)
extern size_t		gdp_sig_getlength(
						gdp_sig_t *sig);

// get the actual signature (and length)
extern void			*gdp_sig_getptr(
						gdp_sig_t *sig,
						size_t *siglen_ptr);


//------------------------------------------------------------------------------
//	Added for C++ compability
}
//------------------------------------------------------------------------------


#endif // _GDP_H_
