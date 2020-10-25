/* vim: set ai sw=4 sts=4 ts=4 : */

/*
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

#include "logd.h"
#include "logd_pubsub.h"

#if !LOG_CHECK
static EP_DBG	Dbg = EP_DBG_INIT("gdplogd.gob", "GDP Log Daemon GOB handling");
#endif


/*
**  GOB_ALLOC --- allocate a new GOB handle in memory
*/

EP_STAT
gob_alloc(gdp_name_t gob_name, gdp_iomode_t iomode, gdp_gob_t **pgob)
{
	EP_STAT estat;
	gdp_gob_t *gob;
	extern void gob_close(gdp_gob_t *gob);

	// get the standard handle
	estat = _gdp_gob_new(gob_name, &gob);
	EP_STAT_CHECK(estat, goto fail0);

	// add the gdpd-specific information
	gob->x = (struct gdp_gob_xtra *) ep_mem_zalloc(sizeof *gob->x);
	if (gob->x == NULL)
	{
		estat = EP_STAT_OUT_OF_MEMORY;
		goto fail0;
	}
	gob->x->gob = gob;

	//XXX for now, assume all GOBs are on disk
	gob->x->physimpl = &GdpSqliteImpl;

	// make sure that if this is freed it gets removed from GclsByUse
	gob->freefunc = gob_close;

	// OK, return the value
	*pgob = gob;

fail0:
	return estat;
}


/*
**  GOB_CLOSE --- close a GDP version of a GOB handle
**
**		Called from _gdp_gob_freehandle, generally when the reference
**		count drops to zero and the GOB is reclaimed.
*/

void
gob_close(gdp_gob_t *gob)
{
	if (gob->x == NULL)
		return;

	// close the underlying files and free memory as needed
	if (gob->x->physimpl->close != NULL)
		gob->x->physimpl->close(gob);

	ep_mem_free(gob->x);
	gob->x = NULL;
}

#if !LOG_CHECK

/*
**   GOB_DELETE --- delete and close a GOB
*/

void
gob_delete(gdp_gob_t *gob)
{
	// arrange to remove the GOB from the cache
	gob->flags &= ~GOBF_DEFER_FREE;

	if (gob->x == NULL)
		return;

	// close the underlying files and free memory as needed
	if (gob->x->physimpl->close != NULL)
		gob->x->physimpl->close(gob);

	// now delete the files
	if (gob->x->physimpl->remove != NULL)
		gob->x->physimpl->remove(gob);

	ep_mem_free(gob->x);
	gob->x = NULL;
}


/*
**  Get an open instance of the GOB in the request.
**
**		This maps the GOB name to the internal GOB instance.
**		That open instance is returned in the request passed in.
**		The GOB will have it's reference count bumped, so the
**		caller must call _gdp_gob_decref when done with it.
**
**		GOB is returned locked and with its reference count
**		incremented.
*/

static EP_STAT
do_physical_open(gdp_gob_t *gob, void *open_info_)
{
	EP_STAT estat;

	if (ep_dbg_test(Dbg, 11))
		ep_dbg_printf("do_physical_open: %s\n", gob->pname);

	gob->x = (struct gdp_gob_xtra *) ep_mem_zalloc(sizeof *gob->x);
	gob->x->gob = gob;

	//XXX for now, assume all GOBs are on disk
	gob->x->physimpl = &GdpSqliteImpl;

	// make sure that if this is freed it gets removed from GclsByUse
	gob->freefunc = gob_close;

	// open the physical disk files
	estat = gob->x->physimpl->open(gob);
	if (EP_STAT_ISOK(estat))
	{
		gob->flags |= GOBF_DEFER_FREE;
		gob->flags &= ~GOBF_PENDING;
	}
	else
	{
		// if this isn't a "not found" error, mark it as an internal error
		if (!EP_STAT_IS_SAME(estat, GDP_STAT_NAK_NOTFOUND))
			estat = GDP_STAT_NAK_INTERNAL;
	}

	return estat;
}


/*
**  Make sure GOB in req->gob is filled in.
**
**		It must correspond to the address in req->cpdu->dst.
**		If req->gob is filled in (i.e., it was NULL on entry), the GOB
**			reference count will be bumped; otherwise it is untouched..
*/

EP_STAT
get_open_handle(gdp_req_t *req)
{
	EP_STAT estat;

	// sanity check
	EP_THR_MUTEX_ASSERT_ISLOCKED(&req->mutex);

	// if we already got this (e.g., in _gdp_pdu_process or in cache),
	//		just let it be
	if (req->gob != NULL)
	{
		estat = EP_STAT_OK;
		if (ep_dbg_test(Dbg, 40))
		{
			gdp_pname_t pname;

			gdp_printable_name(req->cpdu->dst, pname);
			ep_dbg_printf("get_open_handle: using existing GOB:\n\t%s => %p\n",
					pname, req->gob);
		}
	}
	else
	{
		if (ep_dbg_test(Dbg, 40))
		{
			gdp_pname_t pname;

			gdp_printable_name(req->cpdu->dst, pname);
			ep_dbg_printf("get_open_handle: finding %s in cache\n", pname);
		}

		// it should be safe to unlock the req here, since we should hold the
		// only reference, and we need to get the lock ordering right
		_gdp_req_unlock(req);
		estat = _gdp_gob_cache_get(req->cpdu->dst, GGCF_CREATE, &req->gob);
		_gdp_req_lock(req);
		if (EP_STAT_ISOK(estat) && EP_UT_BITSET(GOBF_PENDING, req->gob->flags))
		{
			estat = do_physical_open(req->gob, NULL);
			if (EP_STAT_IS_SAME(estat, GDP_STAT_NAK_NOTFOUND))
			{
				// router sent us a reference to a log that we don't own
				_gdp_gob_free(&req->gob);
			}
		}
		if (!EP_STAT_ISOK(estat) && req->gob != NULL)
			_gdp_gob_decref(&req->gob, false);
		if (ep_dbg_test(Dbg, 40))
		{
			char ebuf[60];
			gdp_pname_t pname;

			gdp_printable_name(req->cpdu->dst, pname);
			ep_stat_tostr(estat, ebuf, sizeof ebuf);
			ep_dbg_printf("get_open_handle: %s => %p: %s\n",
					pname, req->gob, ebuf);
		}
	}
	return estat;
}


# endif // LOG_CHECK
