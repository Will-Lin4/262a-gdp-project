/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	This implements the GDP API for C-based applications.
**
**  With the exception of the name manipulation (parsing,
**  printing, etc.) most of these are basically just translation
**  routines, converting the API calls into requests and handing
**  them on; the hard work is done in gdp_gob_ops.c and gdp_proto.c.
**
**	TODO In the future this may need to be extended to have knowledge
**		 of TSN/AVB, but for now we don't worry about that.
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
#include "gdp_event.h"
#include "gdp_md.h"
#include "gdp_stat.h"
#include "gdp_priv.h"

#include <ep/ep_app.h>
#include <ep/ep_b64.h>
#include <ep/ep_dbg.h>
#include <ep/ep_funclist.h>
#include <ep/ep_prflags.h>
#include <ep/ep_string.h>

#include <openssl/sha.h>

#include <errno.h>
#include <string.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.api", "C API for GDP");

static EP_THR_MUTEX		GinFreeListMutex	EP_THR_MUTEX_INITIALIZER;
SLIST_HEAD(gin_list_head, gdp_gin)
						GinFreeList			= SLIST_HEAD_INITIALIZER(GinFreeList);


/*
**  Mutex around open operations
*/

static EP_THR_MUTEX		OpenMutex		EP_THR_MUTEX_INITIALIZER;


/*
**  Simplify debugging
*/

static void
prstat(EP_STAT estat, const gdp_gin_t *gin, const char *where)
{
	int dbglev = 2;
	char ebuf[100];

	if (EP_STAT_ISOK(estat))
		dbglev = 39;
	else if (EP_STAT_ISWARN(estat))
		dbglev = 11;
	if (gin == NULL || gin->gob == NULL)
	{
		ep_dbg_cprintf(Dbg, dbglev, "<<< %s: %s\n",
				where, ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	else
	{
		ep_dbg_cprintf(Dbg, dbglev, "<<< %s(%s): %s\n",
				where, gin->gob->pname,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
}


/*
**  Generic GDP Object Instance (GIN) code.
*/

static EP_STAT
bad_gin(const gdp_gin_t *gin, const char *where, const char *xtra)
{
	EP_STAT estat;
	char wbuf[256];

	if (where != NULL && xtra != NULL)
	{
		snprintf(wbuf, sizeof wbuf, "%s/%s", where, xtra);
		where = wbuf;
	}
	if (gin == NULL)
		estat = GDP_STAT_NULL_GIN;
	else
		estat = GDP_STAT_LOG_NOT_OPEN;
	prstat(estat, gin, where);
	return estat;
}

void
_gdp_gin_lock_trace(gdp_gin_t *gin,
					const char *file,
					int line,
					const char *id)
{
	_ep_thr_mutex_lock(&gin->mutex, file, line, id);
	gin->flags |= GINF_ISLOCKED;
}

void
_gdp_gin_unlock_trace(gdp_gin_t *gin,
					const char *file,
					int line,
					const char *id)
{
	gin->flags &= ~GINF_ISLOCKED;
	_ep_thr_mutex_unlock(&gin->mutex, file, line, id);
}


static EP_PRFLAGS_DESC	_GdpGinFlags[] =
{
	{ GINF_INUSE,				GINF_INUSE,			"INUSE"				},
	{ GINF_ISLOCKED,			GINF_ISLOCKED,		"ISLOCKED"			},
	{ GINF_SIG_VRFY,			GINF_SIG_VRFY,		"SIG_VRFY"			},
	{ GINF_SIG_VRFY_REQ,		GINF_SIG_VRFY_REQ,	"SIG_VRFY_REQ"		},
	{ 0, 0, NULL }
};

void
_gdp_gin_dump(
		const gdp_gin_t *gin,
		FILE *fp,
		int detail)
{
	int indent = 1;

	if (fp == NULL)
		fp = ep_dbg_getfile();
	if (detail >= GDP_PR_BASIC)
		fprintf(fp, "GIN@%p: ", gin);
	VALGRIND_HG_DISABLE_CHECKING(gin, sizeof *gin);
	if (gin == NULL)
	{
		fprintf(fp, "NULL\n");
	}
	else
	{
		gdp_gob_t *gob = gin->gob;

		if (gob == NULL)
			fprintf(fp, "No GOB\n");
		else
		{
			VALGRIND_HG_DISABLE_CHECKING(gob, sizeof *gob);
			if (!gdp_name_is_valid(gob->name))
				fprintf(fp, "no name\n");
			else
				fprintf(fp, "%s\n", gob->pname);
			VALGRIND_HG_ENABLE_CHECKING(gob, sizeof *gob);
		}
		fprintf(fp, "%sflags = ", _gdp_pr_indent(indent));
		ep_prflags(gin->flags, _GdpGinFlags, fp);

		if (detail >= GDP_PR_BASIC)
		{
			fprintf(fp, "\n%sgob = %p, iomode = %d",
					_gdp_pr_indent(indent), gob,  gin->iomode);
			if (detail >= GDP_PR_DETAILED)
			{
				fprintf(fp, ", closefunc = %p\n"
						"%sapndfilter = %p, apndfpriv = %p,"
						" readfilter = %p, readfpriv = %p",
						gin->closefunc,
						_gdp_pr_indent(indent),
						gin->apndfilter, gin->apndfpriv,
						gin->readfilter, gin->readfpriv);
			}
		}
		fprintf(fp, "\n");
	}
	VALGRIND_HG_ENABLE_CHECKING(gin, sizeof *gin);
}

static void
unlock_gin_and_gob(gdp_gin_t *gin, const char *where)
{
	_gdp_gob_unlock(gin->gob);
	_gdp_gin_unlock(gin);
}

static EP_STAT
check_and_lock_gin(gdp_gin_t *gin, const char *where)
{
	if (gin == NULL)
		return bad_gin(gin, where, "null gin");
	_gdp_gin_lock(gin);
	if (!EP_UT_BITSET(GINF_INUSE, gin->flags))
	{
		_gdp_gin_unlock(gin);
		return bad_gin(gin, where, "gin not inuse");
	}
	if (gin->gob == NULL)
	{
		_gdp_gin_unlock(gin);
		return bad_gin(gin, where, "null gob");
	}
	return EP_STAT_OK;
}

static EP_STAT
check_and_lock_gin_and_gob(gdp_gin_t *gin, const char *where)
{
	EP_STAT estat;

	estat = check_and_lock_gin(gin, where);
	EP_STAT_CHECK(estat, return estat);
	_gdp_gob_lock(gin->gob);
	if (!EP_UT_BITSET(GOBF_INUSE, gin->gob->flags))
		estat = bad_gin(gin, where, "gob not inuse");
	else if (EP_UT_BITSET(GOBF_DROPPING, gin->gob->flags))
		estat = bad_gin(gin, where, "gob not open");
	else if (gin->gob->refcnt <= 0)
		estat = bad_gin(gin, where, "gob bad refcnt");
	if (!EP_STAT_ISOK(estat))
		unlock_gin_and_gob(gin, "check_and_lock_gin_and_gob");
	return estat;
}


/*
**  Allocate and Free GDP Object Instance handles.
**
**		Note that the reference count on the input GOB is not
**		increased; it is assumed that the GIN "takes over" the
**		reference that is passed in.
*/

gdp_gin_t *
_gdp_gin_new(gdp_gob_t *gob)
{
	gdp_gin_t *gin = NULL;

	for (;;)
	{
		ep_thr_mutex_lock(&GinFreeListMutex);
		if ((gin = SLIST_FIRST(&GinFreeList)) != NULL)
			SLIST_REMOVE_HEAD(&GinFreeList, next);
		ep_thr_mutex_unlock(&GinFreeListMutex);
		if (gin == NULL || !EP_UT_BITSET(GINF_INUSE, gin->flags))
			break;

		// gin from freelist is allocated --- abandon it
		EP_ASSERT_PRINT("_gdp_gin_new: allocated gin %p on free list", gin);
	}

	if (gin == NULL)
	{
		gin = (gdp_gin_t *) ep_mem_zalloc(sizeof *gin);
		ep_thr_mutex_init(&gin->mutex, EP_THR_MUTEX_DEFAULT);
		ep_thr_mutex_setorder(&gin->mutex, GDP_MUTEX_LORDER_GIN);
	}

	gin->flags = GINF_INUSE;
	gin->iomode = GDP_MODE_ANY;
	gin->gob = gob;

	VALGRIND_HG_CLEAN_MEMORY(gin, sizeof gin);
	ep_dbg_cprintf(Dbg, 48, "_gdp_gin_new => %p\n", gin);
	return gin;
}

void
_gdp_gin_free(gdp_gin_t *gin)
{
	ep_dbg_cprintf(Dbg, 28, "_gdp_gin_free(%p)\n", gin);
	if (gin == NULL)
		return;
	if (!EP_ASSERT(EP_UT_BITSET(GINF_INUSE, gin->flags)))
		return;
	GDP_GIN_ASSERT_ISLOCKED(gin);

	// release resources: requests (subscriptions) and events
	if (gin->gob != NULL)
	{
		GDP_GOB_ASSERT_ISLOCKED(gin->gob);
		_gdp_req_freeall(gin->gob, gin, NULL);
		_gdp_gob_decref(&gin->gob, false);
	}
	_gdp_event_free_all(gin);

	// put gin handle on freelist
	ep_thr_mutex_lock(&GinFreeListMutex);
	gin->flags = 0;
	SLIST_INSERT_HEAD(&GinFreeList, gin, next);
	_gdp_gin_unlock(gin);
	ep_thr_mutex_unlock(&GinFreeListMutex);
}


/*
**	GDP_GIN_GETNAME --- get the name of a GDP log
*/

const gdp_name_t *
gdp_gin_getname(const gdp_gin_t *gin)
{
	if (!GDP_GIN_ISGOOD(gin))
	{
		(void) bad_gin(gin, "gdp_gin_getname", NULL);
		return NULL;
	}
	return &gin->gob->name;
}


/*
**  GDP_GIN_GETNRECS --- get the number of records in a GOB
*/

gdp_recno_t
gdp_gin_getnrecs(const gdp_gin_t *gin)
{
	if (!GDP_GIN_ISGOOD(gin))
	{
		(void) bad_gin(gin, "gdp_gin_getnrecs", NULL);
		return GDP_PDU_NO_RECNO;
	}
	return gin->gob->nrecs;
}


/*
**  GDP_GIN_PRINT --- print a GOB (for debugging)
*/

void
gdp_gin_print(
		const gdp_gin_t *gin,
		FILE *fp)
{
	if (!GDP_GIN_ISGOOD(gin))
	{
		(void) bad_gin(gin, "gdp_gin_print", NULL);
	}
	else
	{
		// _gdp_gob_dump handles null gob properly
		_gdp_gob_dump(gin->gob, fp, GDP_PR_PRETTY, 0);
	}
}



/*
**	GDP_INIT --- initialize this library
**
**		This is the normal startup for a client process.  Servers
**		may need to do additional steps early on, and may choose
**		to advertise more than their own name.
*/

static void
gdp_exit_debug(void)
{
	if (ep_dbg_test(Dbg, 10))
	{
		_gdp_req_pr_stats(ep_dbg_getfile());
		_gdp_gob_pr_stats(ep_dbg_getfile());
	}
}


EP_STAT
gdp_init(const char *router_addr)
{
	return gdp_init2(router_addr, 0);
}


EP_STAT
gdp_init2(const char *router_addr, uint32_t flags)
{
	gdp_chan_x_t *chanx = NULL;
	EP_STAT estat = EP_STAT_OK;
	extern EP_FUNCLIST *_GdpDumpFuncs;

	ep_dbg_cprintf(Dbg, 9, "gdp_init, state = %d\n", _GdpInitState);
	if (_GdpInitState >= GDP_INIT_COMPLETE)
		goto done;

	// set up global state, event loop, etc. (shared with gdplogd)
	if (_GdpInitState < GDP_INIT_LIB)
	{
		estat = gdp_lib_init(NULL, NULL, flags);
		EP_STAT_CHECK(estat, goto fail0);
	}

	ep_funclist_push(_GdpDumpFuncs, _gdp_event_dump_all, NULL);

	chanx = (gdp_chan_x_t *) ep_mem_zalloc(sizeof *chanx);
	LIST_INIT(&chanx->reqs);

	// open at least one channel to the routing subsystem
	_GdpChannel = NULL;
	estat = _gdp_chan_open(router_addr,			// IP of router
						NULL,					// qos (unused as yet)
						&_gdp_io_recv,			// receive callback
						NULL,					// send callback
						&_gdp_io_event,			// close/error/eof callback
						&_gdp_router_event,		// router event callback
						&_gdp_advertise_me,		// advertise callback
						chanx,					// user channel data
						&_GdpChannel);			// output: new channel
	EP_STAT_CHECK(estat, goto fail0);

	if (ep_thr_spawn(&_GdpIoEventLoopThread, &_gdp_run_event_loop, NULL) != 0)
	{
		char ebuf[100];

		estat = ep_stat_from_errno(errno);
		ep_app_severe("cannot spawn event i/o thread: %s",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	// do some optional status printing on exit
	atexit(gdp_exit_debug);

	_GdpInitState = GDP_INIT_COMPLETE;

fail0:
done:
	if (ep_dbg_test(Dbg, 4))
	{
		char ebuf[200];

		ep_dbg_printf("gdp_init: %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}


/*
**	GDP_GIN_OPEN --- open a GOB for reading or further appending
*/

EP_STAT
gdp_gin_open(gdp_name_t name,
			gdp_iomode_t iomode,
			gdp_open_info_t *open_info,
			gdp_gin_t **pgin)
{
	EP_STAT estat;
	gdp_gob_t *gob = NULL;
	gdp_gin_t *gin = NULL;
	gdp_cmd_t cmd;

	if (ep_dbg_test(Dbg, 19))
	{
		gdp_pname_t pname;
		ep_dbg_printf("\n>>> gdp_gin_open(%s)\n",
					gdp_printable_name(name, pname));
	}
	estat = GDP_CHECK_INITIALIZED;		// make sure gdp_init is done
	EP_STAT_CHECK(estat, return estat);

	if (iomode == GDP_MODE_RO)
		cmd = GDP_CMD_OPEN_RO;
	else if (iomode == GDP_MODE_AO)
		cmd = GDP_CMD_OPEN_AO;
	else if (iomode == GDP_MODE_RA)
		cmd = GDP_CMD_OPEN_RA;
	else
	{
		// illegal I/O mode
		ep_app_error("gdp_gin_open: illegal mode %d", iomode);
		return GDP_STAT_BAD_IOMODE;
	}

	if (!gdp_name_is_valid(name))
	{
		// illegal GOB name
		ep_dbg_cprintf(Dbg, 6, "gdp_gin_open: null GOB name\n");
		return GDP_STAT_GDP_NAME_INVALID;
	}

	// lock this operation to keep the GOB cache consistent
	ep_thr_mutex_lock(&OpenMutex);

	// see if we already have this open (and initiate open if not)
	estat = _gdp_gob_cache_get(name, GGCF_CREATE, &gob);
	EP_STAT_CHECK(estat, goto fail0);
	EP_ASSERT(gob != NULL);
	GDP_GOB_ASSERT_ISLOCKED(gob);

	// if open is partially complete, finish the job
	if (EP_UT_BITSET(GOBF_PENDING, gob->flags))
	{
		estat = _gdp_gob_open(gob, cmd, open_info, _GdpChannel, 0);
		if (EP_STAT_ISFAIL(estat))
			goto fail0;
	}
	gob->flags &= ~GOBF_PENDING;
	gin = _gdp_gin_new(gob);

	if (open_info != NULL)
	{
		if (EP_UT_BITSET(GOIF_VERIFY_PROOF, open_info->flags))
		{
			gin->flags |= GINF_SIG_VRFY;
		}
	}
	_gdp_gob_unlock(gob);

fail0:
	// note that warnings are treated like success
	if (EP_STAT_ISFAIL(estat) && gob != NULL)
		_gdp_gob_free(&gob);
	*pgin = gin;		// might be NULL

	ep_thr_mutex_unlock(&OpenMutex);
	prstat(estat, gin, "gdp_gin_open");
	if (ep_dbg_test(Dbg, 10))
	{
		_gdp_gin_dump(gin, NULL, GDP_PR_DETAILED);
		if (ep_dbg_test(Dbg, 14))
			_gdp_gob_dump(gob, NULL, GDP_PR_BASIC, 0);
	}
	return estat;
}


/*
**	GDP_GIN_CLOSE --- close an open GOB
*/

EP_STAT
gdp_gin_close(gdp_gin_t *gin)
{
	EP_STAT estat;

	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_close");
	EP_STAT_CHECK(estat, return estat);
	ep_dbg_cprintf(Dbg, 19, "\n>>> gdp_gin_close(%s)\n", gin->gob->pname);
	estat = _gdp_gob_close(gin->gob, _GdpChannel, 0);
	_gdp_gin_free(gin);
	prstat(estat, gin, "gdp_gin_close");
	return estat;
}


/*
**  GDP_GIN_DELETE --- delete and close an open GOB
**
**		This is not intended to be an end-user API.  Deletion should
**		only be done by a system service on the basis of expiration
**		criteria.  This API is intended for testing.
*/

EP_STAT
gdp_gin_delete(gdp_gin_t *gin)
{
	EP_STAT estat;

	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_delete");
	EP_STAT_CHECK(estat, return estat);
	ep_dbg_cprintf(Dbg, 19, "\n>>> gdp_gin_delete(%s)\n", gin->gob->pname);
	estat = _gdp_gob_delete(gin->gob, _GdpChannel, 0);
	_gdp_gin_free(gin);
	prstat(estat, gin, "gdp_gin_delete");
	return estat;
}


/*
**	GDP_GIN_APPEND --- append a message to a writable GOB
*/

EP_STAT
gdp_gin_append(gdp_gin_t *gin, gdp_datum_t *datum, gdp_hash_t *prevhash)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_append\n");
	if (!GDP_DATUM_ISGOOD(datum))
		return GDP_STAT_DATUM_REQUIRED;
	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_append");
	EP_STAT_CHECK(estat, return estat);
	if (gin->apndfilter != NULL)
		estat = gin->apndfilter(datum, gin->apndfpriv);
	if (EP_STAT_ISOK(estat))
		estat = _gdp_gob_append_sync(gin->gob, 1, &datum, prevhash,
								_GdpChannel, 0);
	unlock_gin_and_gob(gin, "gdp_gin_append");
	prstat(estat, gin, "gdp_gin_append");
	return estat;
}


/*
**  GDP_GIN_APPEND_ASYNC --- asynchronously append to a writable GOB
*/

EP_STAT
gdp_gin_append_async(gdp_gin_t *gin,
			int ndatums,
			gdp_datum_t **datums,
			gdp_hash_t *prevhash,
			gdp_event_cbfunc_t cbfunc,
			void *udata)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_append_async\n");
	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_append_async");
	EP_STAT_CHECK(estat, return estat);
	estat = _gdp_gob_append_async(gin->gob, gin, ndatums, datums, prevhash,
							cbfunc, udata, _GdpChannel, 0);
	unlock_gin_and_gob(gin, "gdp_gin_append_async");
	prstat(estat, gin, "gdp_gin_append_async");
	return estat;
}


/*
**	GDP_GIN_READ_BY_RECNO --- read a message from a GOB based on recno
**
**	The data is returned through the passed-in datum.
**
**		Parameters:
**			gin --- the GDP instance from which to read
**			recno --- the record number to read
**			datum --- the message header (to avoid dynamic memory)
*/

EP_STAT
gdp_gin_read_by_recno(gdp_gin_t *gin,
			gdp_recno_t recno,
			gdp_datum_t *datum)
{
	EP_STAT estat;
	uint32_t reqflags = 0;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_read_by_recno (%"PRIgdp_recno ")\n",
			recno);
	EP_ASSERT_POINTER_VALID(datum);
	gdp_datum_reset(datum);

	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_read_by_recno");
	EP_STAT_CHECK(estat, return estat);

	if (EP_UT_BITSET(GINF_SIG_VRFY, gin->flags))
		reqflags |= GDP_REQ_VRFY_CONTENT;
	//XXX somehow have to convey gin->readfilter to _gdp_gob_read
	//XXX is there any reason not to just do it here?
	//XXX Answer: read_async and subscriptions
	estat = _gdp_gob_read_by_recno(gin->gob, recno, _GdpChannel,
							reqflags, datum);
	unlock_gin_and_gob(gin, "gdp_gin_read_by_recno");
	prstat(estat, gin, "gdp_gin_read_by_recno");
	return estat;
}


/*
**	GDP_GIN_READ_BY_TS --- read a message from a GOB based on timestamp
**
**	The data is returned through the passed-in datum.
**
**		Parameters:
**			gin --- the GDP instance from which to read
**			ts --- the lowest timestamp we are interested in.  The
**				result will be the lowest timestamp that is greater than
**				or equal to this value.
**			datum --- the message header (to avoid dynamic memory)
*/

EP_STAT
gdp_gin_read_by_ts(gdp_gin_t *gin,
			EP_TIME_SPEC *ts,
			gdp_datum_t *datum)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_read_by_ts\n");
	EP_ASSERT_POINTER_VALID(datum);
#if 0 //TODO
	memcpy(&datum->d->ts, ts, sizeof datum->d->ts);
	datum->d->recno = GDP_PDU_NO_RECNO;

	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_read_by_ts");
	EP_STAT_CHECK(estat, return estat);
	//XXX somehow have to convey gin->readfilter to _gdp_gob_read
	//XXX is there any reason not to just do it here?
	//XXX Answer: read_async and subscriptions
	estat = _gdp_gob_read(gin->gob, datum, _GdpChannel, 0);
	unlock_gin_and_gob(gin, "gdp_gin_read_by_ts");
	prstat(estat, gin, "gdp_gin_read_by_ts");
#else //TODO
	estat = GDP_STAT_NOT_IMPLEMENTED;
#endif //TODO
	return estat;
}


/*
**	GDP_GIN_READ_BY_HASH --- read a message from a GOB based on record hash
**
**	The data is returned through the passed-in datum.
**
**		Parameters:
**			gin --- the GDP instance from which to read
**			hash --- the starting record hash
**			nrecs --- the number of records to read
**			datum --- the message header (to avoid dynamic memory)
*/

#if 0 //TODO
EP_STAT
gdp_gin_read_by_hash(gdp_gin_t *gin,
			ep_hash_t *hash,
			gdp_datum_t *datum)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_read_by_hash\n");
	EP_ASSERT_POINTER_VALID(datum);
	memcpy(&datum->d->ts, ts, sizeof datum->d->ts);
	datum->d->recno = GDP_PDU_NO_RECNO;

	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_read_by_hash");
	EP_STAT_CHECK(estat, return estat);
	//XXX somehow have to convey gin->readfilter to _gdp_gob_read
	//XXX is there any reason not to just do it here?
	//XXX Answer: read_async and subscriptions
	estat = _gdp_gob_read(gin->gob, datum, _GdpChannel, 0);
	unlock_gin_and_gob(gin, "gdp_gin_read_by_hash");
	prstat(estat, gin, "gdp_gin_read_by_hash");
	return estat;
}
#endif //TODO


/*
**  GDP_GIN_READ_BY_xxx_ASYNC --- read asynchronously
**
**  Data and status are delivered as events.  These subsume the
**  old multiread command.
*/

EP_STAT
gdp_gin_read_by_recno_async(
			gdp_gin_t *gin,
			gdp_recno_t recno,
			int32_t nrecs,
			gdp_event_cbfunc_t cbfunc,
			void *cbarg)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_read_by_recno_async\n");
	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_read_by_recno_async");
	EP_STAT_CHECK(estat, return estat);
	estat = _gdp_gob_read_by_recno_async(gin->gob, gin, recno, nrecs,
							cbfunc, cbarg, _GdpChannel);
	unlock_gin_and_gob(gin, "gdp_gin_read_by_recno_async");
	prstat(estat, gin, "gdp_gin_read_by_recno_async");
	return estat;
}


EP_STAT
gdp_gin_read_by_ts_async(
			gdp_gin_t *gin,
			EP_TIME_SPEC *ts,
			int32_t nrecs,
			gdp_event_cbfunc_t cbfunc,
			void *cbarg)
{
	return GDP_STAT_NOT_IMPLEMENTED;
}


EP_STAT
gdp_gin_read_by_hash_async(
			gdp_gin_t *gin,
			uint32_t n_hashes,
			gdp_hash_t **hashes,
			gdp_event_cbfunc_t cbfunc,
			void *cbarg)
{
	return GDP_STAT_NOT_IMPLEMENTED;
}


#if 0
// back compat
EP_STAT
gdp_gcl_read_async(
			gdp_gin_t *gin,
			gdp_recno_t recno,
			int32_t nrecs,
			gdp_event_cbfunc_t cbfunc,
			void *cbarg)
{
	return gdp_gcl_read_by_recno_async(gin, recno, nrecs, cbfunc, cbarg);
}
#endif




/*
**	GDP_GIN_SUBSCRIBE_BY_RECNO --- subscribe starting from a record number
*/

EP_STAT
gdp_gin_subscribe_by_recno(gdp_gin_t *gin,
		gdp_recno_t start,
		int32_t numrecs,
		gdp_sub_qos_t *qos,
		gdp_event_cbfunc_t cbfunc,
		void *cbarg)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_subscribe_by_recno\n");
	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_subscribe_by_recno");
	EP_STAT_CHECK(estat, return estat);

	estat = _gdp_gin_subscribe(gin, GDP_CMD_SUBSCRIBE_BY_RECNO, start, numrecs,
							qos, cbfunc, cbarg);

	unlock_gin_and_gob(gin, "gdp_gin_subscribe_by_recno");
	prstat(estat, gin, "gdp_gin_subscribe_by_recno");
	return estat;
}


/*
**	GDP_GIN_SUBSCRIBE_BY_TS --- subscribe to a GOB starting from a timestamp
*/

EP_STAT
gdp_gin_subscribe_by_ts(gdp_gin_t *gin,
		EP_TIME_SPEC *start,
		int32_t numrecs,
		gdp_sub_qos_t *qos,
		gdp_event_cbfunc_t cbfunc,
		void *cbarg)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_subscribe_by_ts\n");
#if 0 //TODO
	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_subscribe_by_ts");
	EP_STAT_CHECK(estat, return estat);

	// create the subscribe request
	gdp_req_t *req;
	estat = _gdp_req_new(GDP_CMD_SUBSCRIBE_BY_TS, gin->gob, _GdpChannel, NULL,
			GDP_REQ_PERSIST | GDP_REQ_CLT_SUBSCR | GDP_REQ_ALLOC_RID,
			&req);
	EP_STAT_CHECK(estat, goto fail0);

	// add start and stop parameters to PDU
	req->gin = gin;
	memcpy(&req->cpdu->m->b->datum->d->ts, start, sizeof req->cpdu->m->b->datum->d->ts);
	req->numrecs = numrecs;

	// now do the hard work
	estat = _gdp_gin_subscribe(req, numrecs, timeout, cbfunc, cbarg);
fail0:
	unlock_gin_and_gob(gin, "gdp_gin_subscribe_by_ts");
#else	//TODO
	estat = GDP_STAT_NOT_IMPLEMENTED;
#endif	//TODO
	prstat(estat, gin, "gdp_gin_subscribe_by_ts");
	return estat;
}


/*
**  GDP_GIN_UNSUBSCRIBE --- delete subscriptions to a named GOB
*/

EP_STAT
gdp_gin_unsubscribe(gdp_gin_t *gin,
		gdp_event_cbfunc_t cbfunc,
		void *cbarg)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_unsubscribe\n");

	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_unsubscribe");
	EP_STAT_CHECK(estat, return estat);
	estat = _gdp_gin_unsubscribe(gin, cbfunc, cbarg, 0);
	unlock_gin_and_gob(gin, "gdp_gin_unsubscribe");

	prstat(estat, gin, "gdp_gin_unsubscribe");
	return estat;
}


/*
**  GDP_GIN_GETMETADATA --- return the metadata associated with a GOB
*/

EP_STAT
gdp_gin_getmetadata(gdp_gin_t *gin,
		gdp_md_t **gmdp)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_getmetadata\n");
	estat = check_and_lock_gin_and_gob(gin, "gdp_gin_getmetadata");
	EP_STAT_CHECK(estat, return estat);
	estat = _gdp_gob_getmetadata(gin->gob, gmdp, _GdpChannel, 0);
	unlock_gin_and_gob(gin, "gdp_gin_getmetadata");
	prstat(estat, gin, "gdp_gin_getmetadata");
	return estat;
}


/*
**  GDP_GIN_SET_APPEND_FILTER --- set the append filter function
*/

EP_STAT
gdp_gin_set_append_filter(gdp_gin_t *gin,
		EP_STAT (*appendfilter)(gdp_datum_t *, void *),
		void *filterdata)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_set_append_filter\n");
	estat = check_and_lock_gin(gin, "gdp_gin_set_append_filter");
	EP_STAT_CHECK(estat, return estat);
	gin->apndfilter = appendfilter;
	gin->apndfpriv = filterdata;
	_gdp_gin_unlock(gin);
	return EP_STAT_OK;
}


/*
**  GDP_GIN_SET_READ_FILTER --- set the read filter function
*/

EP_STAT
gdp_gin_set_read_filter(gdp_gin_t *gin,
		EP_STAT (*readfilter)(gdp_datum_t *, void *),
		void *filterdata)
{
	EP_STAT estat;

	ep_dbg_cprintf(Dbg, 39, "\n>>> gdp_gin_set_read_filter\n");
	estat = check_and_lock_gin(gin, "gdp_gin_set_read_filter");
	EP_STAT_CHECK(estat, return estat);
	gin->readfilter = readfilter;
	gin->readfpriv = filterdata;
	_gdp_gin_unlock(gin);
	return EP_STAT_OK;
}


/*
**  GDP Open Information handling
*/

gdp_open_info_t *
gdp_open_info_new(void)
{
	gdp_open_info_t *info;

	info = (gdp_open_info_t *) ep_mem_zalloc(sizeof*info);
	return info;
}

void
gdp_open_info_free(gdp_open_info_t *info)
{
	//XXX should check for info == NULL here
	if (info->signkey != NULL)
		ep_crypto_key_free(info->signkey);
	ep_mem_free(info);
}

EP_STAT
gdp_open_info_set_signing_key(gdp_open_info_t *info,
		EP_CRYPTO_KEY *skey)
{
	//XXX should check for info == NULL here
	info->signkey = skey;
	return EP_STAT_OK;
}

EP_STAT
gdp_open_info_set_signkey_cb(
				gdp_open_info_t *info,
				EP_STAT (*signkey_cb)(
					gdp_name_t gname,
					void *signkey_udata,
					EP_CRYPTO_KEY **skey),
				void *signkey_udata)
{
	//XXX should check for info == NULL here
	info->signkey_cb = signkey_cb;
	info->signkey_udata = signkey_udata;
	return EP_STAT_OK;
}

EP_STAT
gdp_open_info_set_caching(
		gdp_open_info_t *info,
		bool keep_in_cache)
{
	//XXX should check for info == NULL here
	if (keep_in_cache)
		info->flags |= GOIF_KEEP_IN_CACHE;
	else
		info->flags &= ~GOIF_KEEP_IN_CACHE;
	return EP_STAT_OK;
}

EP_STAT
gdp_open_info_set_vrfy(
		gdp_open_info_t *info,
		bool verify_proof)
{
	//XXX should check for info == NULL here
	//XXX ideally this would check to make sure we have a public key,
	//XXX	but that isn't possible yet.
	if (verify_proof)
		info->flags |= GOIF_VERIFY_PROOF;
	else
		info->flags &= ~GOIF_VERIFY_PROOF;
	return EP_STAT_OK;
}

EP_STAT
gdp_open_info_set_no_skey_nonfatal(
		gdp_open_info_t *info,
		bool nonfatal)
{
	if (nonfatal)
		info->flags |= GOIF_NO_SKEY_NONFATAL;
	else
		info->flags &= ~GOIF_NO_SKEY_NONFATAL;
	return EP_STAT_OK;
}
