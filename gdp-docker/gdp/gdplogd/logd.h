/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**	Headers for the GDP Log Daemon
**
**	----- BEGIN LICENSE BLOCK -----
**	GDPLOGD: Log Daemon for the Global Data Plane
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

#ifndef _GDPLOGD_LOGD_H_
#define _GDPLOGD_LOGD_H_		1

#include <ep/ep.h>
#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <ep/ep_log.h>
#include <ep/ep_stat.h>

#include <gdp/gdp.h>
#include <gdp/gdp_priv.h>
#include <gdp/gdp_pdu.h>
#include <gdp/gdp_stat.h>

#include <unistd.h>
#include <string.h>
#include <sys/queue.h>

__BEGIN_DECLS

/*
**  "Forgive" decisions.  Which problems will we forgive and repair?
**
**		Probably lowers security, but raises resilience.
*/

#ifndef _GDPLOGD_FORGIVING
# define _GDPLOGD_FORGIVING	1
#endif

#if _GDPLOGD_FORGIVING
extern uint32_t		GdplogdForgive;
#define FORGIVE_LOG_GAPS			0x00000001
#define FORGIVE_LOG_DUPS			0x00000002
#endif //_GDPLOGD_FORGIVING

// how strongly we enforce signatures
extern uint32_t		GdpSignatureStrictness;		// how strongly we enforce signatures

#define GDP_SIG_MUSTVERIFY	0x01		// sig must verify if it exists
#define GDP_SIG_REQUIRED	0x02		// sig must exist if pub key exists
#define GDP_SIG_PUBKEYREQ	0x04		// public key must exist

/*
**  Private GOB definitions for gdplogd only
**
**		The gob field is because the LIST macros don't understand
**		having the links in a substructure (i.e., I can't link a
**		gdp_gob_xtra to a gdp_gob).
*/

typedef struct physinfo	gob_physinfo_t;
struct gdp_gob_xtra
{
	// declarations relating to semantics
	gdp_gob_t				*gob;			// enclosing GOB
	uint16_t				n_md_entries;	// number of metadata entries
	uint16_t				log_type;		// from log header

	// physical implementation declarations
	struct gob_phys_impl	*physimpl;		// physical implementation
	gob_physinfo_t			*physinfo;		// info needed by physical module
};


/*
**  Definitions for the gdpd-specific GOB handling
*/

extern EP_STAT	gob_alloc(				// allocate a new GOB
					gdp_name_t gob_name,
					gdp_iomode_t iomode,
					gdp_gob_t **pgob);

extern EP_STAT	gob_open(				// open an existing physical GOB
					gdp_name_t gob_name,
					gdp_iomode_t iomode,
					gdp_gob_t **pgob);

extern void		gob_close(				// close an open GOB
					gdp_gob_t *gob);

extern void		gob_delete(				// close and delete a GCL
					gdp_gob_t *gob);

extern void		gob_touch(				// make a GOB recently used
					gdp_gob_t *gob);

extern void		gob_showusage(			// show GOB LRU list
					FILE *fp);

extern EP_STAT	get_open_handle(		// get open handle (pref from cache)
					gdp_req_t *req);

extern void		gob_reclaim_resources(	// reclaim old GOBs
					void *null);			// parameter unused


/*
**  Definitions for the protocol module
*/

extern EP_STAT	gdpd_proto_init(void);	// initialize protocol module

extern EP_STAT	dispatch_cmd(			// dispatch a request
					gdp_req_t *req);


/*
**  Advertisements
*/

extern void		logd_advertise_init(void);

extern EP_STAT	logd_advertise_all(
						gdp_chan_t *chan,
						int cmd,
						void *adata);

extern void		logd_advertise_one(
						gdp_chan_t *chan,
						gdp_name_t name,
						int cmd);

extern EP_STAT	sub_send_message_notification(
						gdp_req_t *req);

/*
**  Physical Implementation --- these are the routines that implement the
**			on-disk (or in-memory) structure.
*/

// status structure (for administrative use in gdplogd)
struct gob_phys_stats
{
	gdp_recno_t		nrecs;			// number of records
	int64_t			size;			// size in bytes
};

typedef struct gdp_result_ctx	gdp_result_ctx_t;

// callback for dispatching results of reads
typedef EP_STAT		gdp_result_cb_t(
							EP_STAT estat,
							gdp_datum_t *datum,
							gdp_result_ctx_t *cb_ctx);

// the service switch entry
struct gob_phys_impl
{
	EP_STAT		(*init)(
						const char *log_dir);
	EP_STAT		(*read_by_hash)(
						gdp_gob_t *gob,
						gdp_hash_t *hash,
						gdp_result_cb_t *cb,
						void *cb_ctx);
	EP_STAT		(*read_by_recno)(
						gdp_gob_t *gob,
						gdp_recno_t startrec,
						uint32_t maxrecs,
						gdp_result_cb_t *cb,
						void *cb_ctx);
	EP_STAT		(*read_by_timestamp)(
						gdp_gob_t *gob,
						EP_TIME_SPEC *start_time,
						uint32_t maxrecs,
						gdp_result_cb_t *cb,
						void *cb_ctx);
	EP_STAT		(*create)(
						gdp_gob_t *pgob,
						gdp_md_t *gmd);
	EP_STAT		(*open)(
						gdp_gob_t *gob);
	EP_STAT		(*close)(
						gdp_gob_t *gob);
	EP_STAT		(*append)(
						gdp_gob_t *gob,
						gdp_datum_t *datum);
	EP_STAT		(*getmetadata)(
						gdp_gob_t *gob,
						gdp_md_t **gmdp);
	EP_STAT		(*remove)(
						gdp_gob_t *gob);
	EP_STAT		(*foreach)(
						EP_STAT (*func)(
							gdp_name_t name,
							void *ctx),
						void *ctx);
	void		(*getstats)(
						gdp_gob_t *gob,
						struct gob_phys_stats *stats);
	bool		(*recno_exists)(
						gdp_gob_t *gob,
						gdp_recno_t recno);
	EP_STAT		(*xact_begin)(
						gdp_gob_t *gob);
	EP_STAT		(*xact_end)(
						gdp_gob_t *gob);
	EP_STAT		(*xact_abort)(
						gdp_gob_t *gob);
};

// known implementations
extern struct gob_phys_impl		GdpSqliteImpl;


__END_DECLS
#endif //_GDPLOG_LOGD_H_
