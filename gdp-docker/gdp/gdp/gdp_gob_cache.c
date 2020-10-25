/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	This implements GDP Connection Log Cache
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

#include <ep/ep.h>
#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <ep/ep_hash.h>
#include <ep/ep_log.h>
#include <ep/ep_thr.h>

#include "gdp.h"
#include "gdp_stat.h"
#include "gdp_priv.h"

#include <event2/event.h>
#include <openssl/md4.h>

#include <errno.h>
#include <string.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.gob.cache", "GOB cache");



/***********************************************************************
**
**	GOB Caching
**		Return the internal representation of the GOB from
**		the name.  These are not really intended for public use,
**		but they are shared with gdplogd.
**
**		Getting lock ordering right here is a pain.
**
***********************************************************************/

static EP_THR_MUTEX		GobCacheMutex;		// locks _OpenGOBCache and GobsByUse
EP_HASH					*_OpenGOBCache;		// associative cache
LIST_HEAD(gob_use_head, gdp_gob)			// LRU cache
						GobsByUse		= LIST_HEAD_INITIALIZER(GobByUse);


/*
**  Initialize the GOB cache
*/

EP_STAT
_gdp_gob_cache_init(void)
{
	EP_STAT estat = EP_STAT_OK;
	int istat;
	const char *err;

	if (_OpenGOBCache == NULL)
	{
		istat = ep_thr_mutex_init(&GobCacheMutex, EP_THR_MUTEX_DEFAULT);
		if (istat != 0)
		{
			estat = ep_stat_from_errno(istat);
			err = "could not initialize GobCacheMutex";
			goto fail0;
		}
		ep_thr_mutex_setorder(&GobCacheMutex, GDP_MUTEX_LORDER_GOBCACHE);

		_OpenGOBCache = ep_hash_new("_OpenGOBCache", NULL, 0);
		if (_OpenGOBCache == NULL)
		{
			estat = ep_stat_from_errno(errno);
			err = "could not create OpenGOBCache";
			goto fail0;
		}
	}

#if EP_OSCF_USE_VALGRIND
	{
		gdp_gob_t *gob;

		// establish lock ordering for valgrind
		ep_thr_mutex_lock(&GobCacheMutex);
		estat = _gdp_gob_new(NULL, &gob);
		if (EP_STAT_ISOK(estat))
		{
			_gdp_gob_lock(gob);
			_gdp_gob_free(&gob);
		}
		ep_thr_mutex_unlock(&GobCacheMutex);
	}
#endif

	// Nothing to do for LRU cache

	if (false)
	{
fail0:
		if (EP_STAT_ISOK(estat))
			estat = EP_STAT_ERROR;
		ep_log(estat, "gdp_gob_cache_init: %s", err);
		ep_app_fatal("gdp_gob_cache_init: %s", err);
	}

	return estat;
}

#define LOG2_BLOOM_SIZE		10			// log base 2 of bloom filter size


/*
**  Helper routine checking that associative cache matches the LRU cache.
**
**		Unfortunately, this is O(n^2).  The bloom filter is less
**		useful here.
**		This is a helper routine called from ep_hash_forall.
*/

static void
check_cache_helper(size_t klen, const void *key, const void *val, va_list av)
{
	const gdp_gob_t *gob;
	const gdp_gob_t *g2;

	if (val == NULL)
		return;

	gob = (const gdp_gob_t *) val;
	LIST_FOREACH(g2, &GobsByUse, ulist)
	{
		if (g2 == gob)
			return;
	}

	FILE *fp = va_arg(av, FILE *);
	fprintf(fp, "    ===> WARNING: %s not in usage list\n", gob->pname);
}


/*
** Check cache consistency.
**		Returns true if is OK, false if is not.
*/

static bool
check_cache_consistency(const char *where)
{
	bool rval = true;
	gdp_gob_t *gob;
	uint32_t bloom[1 << (LOG2_BLOOM_SIZE - 1)];		// bloom filter
	union
	{
		uint8_t		md4out[MD4_DIGEST_LENGTH];		// output md4
		uint32_t	md4eq[MD4_DIGEST_LENGTH / 4];
	} md4buf;

	memset(&md4buf, 0, sizeof md4buf);
	memset(&bloom, 0, sizeof bloom);

	EP_THR_MUTEX_ASSERT_ISLOCKED(&GobCacheMutex);
	LIST_FOREACH(gob, &GobsByUse, ulist)
	{
		// check for loops, starting with quick bloom filter on address
		uint32_t bmask;
		int bindex;

		VALGRIND_HG_DISABLE_CHECKING(gob, sizeof *gob);

		MD4((unsigned char *) &gob, sizeof gob, md4buf.md4out);
		bindex = md4buf.md4eq[0];			// just use first 32-bit word
		bmask = 1 << (bindex & 0x1f);		// mask on single word
		bindex = (bindex >> 4) & ((1 << (LOG2_BLOOM_SIZE - 1)) - 1);
		if (EP_UT_BITSET(bmask, bloom[bindex]))
		{
			// may have a conflict --- do explicit check
			gdp_gob_t *g2 = LIST_NEXT(gob, ulist);
			while (g2 != NULL && g2 != gob)
				g2 = LIST_NEXT(g2, ulist);
			if (g2 != NULL)
			{
				EP_ASSERT_PRINT("Loop in GobsByUse on %p at %s",
						gob, where);
				_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_DETAILED, 0);
				rval = false;
				VALGRIND_HG_ENABLE_CHECKING(gob, sizeof *gob);
				break;
			}
		}
		bloom[bindex] |= bmask;
		VALGRIND_HG_ENABLE_CHECKING(gob, sizeof *gob);
	}

	ep_hash_forall(_OpenGOBCache, check_cache_helper, ep_dbg_getfile());
	return rval;
}


/*
**  Rebuild GOB LRU list from the associative hash table.
**		Which one is the definitive version?
**
**		Should "never happen", but if it does all hell breaks loose.
**		Yes, I know, this is O(n^2).  But it should never happen.
**		So sue me.
*/

// helper routine
static void
sorted_insert(size_t klen, const void *key, const void *gob_, va_list av)
{
	gdp_gob_t *gob = (gdp_gob_t *) gob_;

	// if this is being dropped, just skip this GOB
	if (EP_UT_BITSET(GOBF_DROPPING, gob->flags))
		return;

	gdp_gob_t *g2 = LIST_FIRST(&GobsByUse);
	if (g2 == NULL || gob->utime > g2->utime)
	{
		// new entry is younger than first entry
		LIST_INSERT_HEAD(&GobsByUse, gob, ulist);
		return;
	}
	LIST_FOREACH(g2, &GobsByUse, ulist)
	{
		gdp_gob_t *g3 = LIST_NEXT(g2, ulist);
		if (g3 == NULL || gob->utime > g3->utime)
		{
			LIST_INSERT_AFTER(g2, gob, ulist);
			return;
		}
	}

	// shouldn't happen
	EP_ASSERT_PRINT("sorted_insert: ran off end of GobsByUse, gob = %p", gob);
}

static void
rebuild_lru_list(void)
{
	ep_dbg_cprintf(Dbg, 2, "rebuild_lru_list: rebuilding\n");
	ep_thr_mutex_lock(&GobCacheMutex);
	LIST_INIT(&GobsByUse);
	ep_hash_forall(_OpenGOBCache, sorted_insert);
	ep_thr_mutex_unlock(&GobCacheMutex);
}




/*
**  Add a GOB to both the associative and the LRU caches.
**  The "unlocked" refers to GobCacheMutex, which should already
**  be locked on entry.  The GOB is also expected to be locked.
*/

static void
add_cache_unlocked(gdp_gob_t *gob)
{
	ep_dbg_cprintf(Dbg, 49, "_gdp_gob_cache_add(%p): adding\n", gob);

	// sanity checks
	EP_ASSERT_ELSE(GDP_GOB_ISGOOD(gob), return);
	if (!GDP_GOB_ASSERT_ISLOCKED(gob))
		return;
	EP_THR_MUTEX_ASSERT_ISLOCKED(&GobCacheMutex);

	check_cache_consistency("add_cache_unlocked");

	if (EP_UT_BITSET(GOBF_INCACHE, gob->flags))
	{
		ep_dbg_cprintf(Dbg, 9,
				"_gdp_gob_cache_add(%p): already cached\n",
				gob);
		return;
	}

	// save it in the associative cache
	gdp_gob_t *g2;
	ep_dbg_cprintf(Dbg, 49,
			"_gdp_gob_cache_add(%p): insert into _OpenGOBCache\n",
			gob);
	g2 = (gdp_gob_t *) ep_hash_insert(_OpenGOBCache,
								sizeof (gdp_name_t), gob->name, gob);
	if (g2 != NULL)
	{
		EP_ASSERT_PRINT("duplicate GOB cache entry, gob=%p (%s)",
				gob, gob->pname);
		fprintf(stderr, "New ");
		_gdp_gob_dump(gob, stderr, GDP_PR_DETAILED, 0);
		fprintf(stderr, "Existing ");
		_gdp_gob_dump(g2, stderr, GDP_PR_DETAILED, 0);
		// we don't free g2 in case someone else has the pointer
	}

	// ... and the LRU list
	{
		struct timeval tv;

		gettimeofday(&tv, NULL);
		gob->utime = tv.tv_sec;

		ep_dbg_cprintf(Dbg, 49, "_gdp_gob_cache_add(%p): insert into LRU list\n",
				gob);
		IF_LIST_CHECK_OK(&GobsByUse, gob, ulist, gdp_gob_t)
		{
			LIST_INSERT_HEAD(&GobsByUse, gob, ulist);
		}
	}

	gob->flags |= GOBF_INCACHE;
	ep_dbg_cprintf(Dbg, 40, "_gdp_gob_cache_add: %s => %p\n",
			gob->pname, gob);
}


/*
**	Wrapper for add_cache_unlocked that takes care of locking
**	GobCacheMutex.  Since that *must* be locked before the GOB,
**	we have to unlock the GOB before we lock GobCache.
**	This should be OK since presumably the GOB is not in the
**	cache and hence not accessible to other threads.
*/

void
_gdp_gob_cache_add(gdp_gob_t *gob)
{
	EP_THR_MUTEX_ASSERT_ISLOCKED(&gob->mutex);
	_gdp_gob_unlock(gob);
	ep_thr_mutex_lock(&GobCacheMutex);
	_gdp_gob_lock(gob);
	add_cache_unlocked(gob);
	ep_thr_mutex_unlock(&GobCacheMutex);
}


/*
**  Update the name of a GOB in the cache
**		This is used when creating a new GOB.  The original GOB
**		is a dummy that uses the name of the log server.  After
**		the log actually exists it has to be updated with the
**		real name.
*/

void
_gdp_gob_cache_changename(gdp_gob_t *gob, gdp_name_t newname)
{
	// sanity checks
	EP_ASSERT_ELSE(GDP_GOB_ISGOOD(gob), return);
	EP_ASSERT_ELSE(gdp_name_is_valid(gob->name), return);
	EP_ASSERT_ELSE(gdp_name_is_valid(newname), return);
	EP_ASSERT_ELSE(EP_UT_BITSET(GOBF_INCACHE, gob->flags), return);

	ep_thr_mutex_lock(&GobCacheMutex);
	check_cache_consistency("_gdp_gob_cache_changename");
	(void) ep_hash_delete(_OpenGOBCache, sizeof (gdp_name_t), gob->name);
	(void) memcpy(gob->name, newname, sizeof (gdp_name_t));
	(void) ep_hash_insert(_OpenGOBCache, sizeof (gdp_name_t), newname, gob);
	ep_thr_mutex_unlock(&GobCacheMutex);

	ep_dbg_cprintf(Dbg, 40, "_gdp_gob_cache_changename: %s => %p\n",
					gob->pname, gob);
}


/*
**  _GDP_GOB_CACHE_GET --- get a GOB from the cache, if it exists
**
**		Searches for a specific GOB.  If found it is returned; if not,
**		it returns null unless the GGCF_CREATE flag is set, in which
**		case it is created and returned.  This allows the Cache Mutex
**		to be locked before the GOB is locked.  Newly created GOBs
**		are marked GOBF_PENDING unless the open routine clears that bit.
**		In particular, gdp_gob_open needs to do additional opening
**		_after_ _gdp_gob_cache_get returns so that the cache is
**		unlocked while sending protocol to the server.
**
**		The GOB is returned locked and with its reference count
**		incremented.  It is up to the caller to call _gdp_gob_decref
**		on the GOB when it is done with it.
*/

EP_STAT
_gdp_gob_cache_get(
			gdp_name_t gob_name,
			uint32_t flags,
			gdp_gob_t **pgob)
{
	gdp_gob_t *gob;
	EP_STAT estat = EP_STAT_OK;

	ep_thr_mutex_lock(&GobCacheMutex);
	if (!check_cache_consistency("_gdp_gob_cache_get"))
		rebuild_lru_list();

	// see if we have a pointer to this GOB in the cache
	gob = (gdp_gob_t *) ep_hash_search(_OpenGOBCache,
							sizeof (gdp_name_t), (void *) gob_name);
	if (gob != NULL)
	{
		_gdp_gob_lock(gob);

		// sanity checking:
		// someone may have snuck in before we acquired the lock
		if (!EP_UT_BITSET(GOBF_INUSE, gob->flags) ||
				!EP_UT_BITSET(GOBF_INCACHE, gob->flags) ||
				EP_UT_BITSET(GOBF_DROPPING, gob->flags) ||
				(EP_UT_BITSET(GOBF_PENDING, gob->flags) &&
				 !EP_UT_BITSET(GGCF_GET_PENDING, flags)))
		{
			// someone deallocated this in the brief window above
			if (ep_dbg_test(Dbg, 10))
			{
				ep_dbg_printf("_gdp_gob_cache_get: abandoning ");
				_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_BASIC, 0);
			}
			_gdp_gob_unlock(gob);
			gob = NULL;
		}
		else if (!EP_UT_BITSET(GGCF_PEEK, flags))
		{
			_gdp_gob_incref(gob);
			_gdp_gob_touch(gob);
		}
	}
	if (gob == NULL && EP_UT_BITSET(GGCF_CREATE, flags))
	{
		// create a new one
		estat = _gdp_gob_new(gob_name, &gob);
		EP_STAT_CHECK(estat, goto fail0);
		_gdp_gob_lock(gob);
		add_cache_unlocked(gob);
	}

	if (ep_dbg_test(Dbg, 42))
	{
		gdp_pname_t pname;

		gdp_printable_name(gob_name, pname);
		if (gob == NULL)
			ep_dbg_printf("_gdp_gob_cache_get: %s => NULL\n", pname);
		else
			ep_dbg_printf("_gdp_gob_cache_get: %s =>\n\t%p refcnt %d\n",
						pname, gob, gob->refcnt);
	}
fail0:
	if (EP_STAT_ISOK(estat))
		*pgob = gob;
	ep_thr_mutex_unlock(&GobCacheMutex);
	return estat;
}


/*
** Drop a GOB from both the associative and the LRU caches
**
*/

void
_gdp_gob_cache_drop(gdp_gob_t *gob, bool cleanup)
{
	EP_ASSERT_ELSE(gob != NULL, return);
	if (!EP_ASSERT(GDP_GOB_ISGOOD(gob)))
	{
		// GOB is in some random state --- we need the name at least
		// (this may crash)
		EP_ASSERT_ELSE(gdp_name_is_valid(gob->name), return);
	}

	GDP_GOB_ASSERT_ISLOCKED(gob);
	if (!EP_ASSERT(EP_UT_BITSET(GOBF_INCACHE, gob->flags)))
		return;
	if (cleanup)
	{
		EP_THR_MUTEX_ASSERT_ISLOCKED(&GobCacheMutex);
		check_cache_consistency("_gdp_gob_cache_drop");
	}

	// error if we're dropping something that's referenced from the cache
	if (gob->refcnt != 0)
	{
		ep_log(GDP_STAT_BAD_REFCNT, "_gdp_gob_cache_drop: ref count %d != 0",
				gob->refcnt);
		if (ep_dbg_test(Dbg, 1))
			_gdp_gob_dump(gob, ep_dbg_getfile(), GDP_PR_BASIC, 0);
	}

	// mark it as being dropped to detect race condition
	gob->flags |= GOBF_DROPPING;

	// if we're not cleanup up (GobCacheMutex unlocked) we have to
	// get the lock ordering right
	if (!cleanup)
	{
		// now lock the cache and then re-lock the GOB
		_gdp_gob_unlock(gob);
		ep_thr_mutex_lock(&GobCacheMutex);
		check_cache_consistency("_gdp_gob_cache_drop");
		_gdp_gob_lock(gob);

		// sanity checks (XXX should these be assertions? XXX)
		//XXX need to check that nothing bad happened while GOB was unlocked
		EP_ASSERT(EP_UT_BITSET(GOBF_INCACHE, gob->flags));
	}

	// remove it from the associative cache
	(void) ep_hash_delete(_OpenGOBCache, sizeof (gdp_name_t), gob->name);

	// ... and the LRU list
	LIST_REMOVE(gob, ulist);
	gob->flags &= ~GOBF_INCACHE;

	if (!cleanup)
	{
		// now we can unlock the cache, but leave the GOB locked
		ep_thr_mutex_unlock(&GobCacheMutex);
	}

	ep_dbg_cprintf(Dbg, 40, "_gdp_gob_cache_drop: %s => %p\n",
			gob->pname, gob);
}


/*
**  _GDP_GOB_TOUCH --- move GOB to the front of the LRU list
**
**		GOB must be locked when we enter.
*/

void
_gdp_gob_touch(gdp_gob_t *gob)
{
	struct timeval tv;

	EP_ASSERT_ELSE(GDP_GOB_ISGOOD(gob), return);

	ep_dbg_cprintf(Dbg, 46, "_gdp_gob_touch(%p)\n", gob);

	gettimeofday(&tv, NULL);
	gob->utime = tv.tv_sec;

	if (!GDP_GOB_ASSERT_ISLOCKED(gob))
	{
		// GOB isn't locked: do nothing
	}
	else if (!EP_UT_BITSET(GOBF_INCACHE, gob->flags))
	{
		// GOB isn't in cache: do nothing
	}
	else
	{
		// both locked and in cache
		EP_THR_MUTEX_ASSERT_ISLOCKED(&GobCacheMutex);
		check_cache_consistency("_gdp_gob_touch");
		LIST_REMOVE(gob, ulist);
		IF_LIST_CHECK_OK(&GobsByUse, gob, ulist, gdp_gob_t)
			LIST_INSERT_HEAD(&GobsByUse, gob, ulist);
	}
}


/*
**  Reclaim cache entries older than a specified age
**
**		If we can't get the number of file descriptors down far enough
**		we keep trying with increasingly stringent constraints, so maxage
**		is really more advice than a requirement.
**
**		XXX	Currently the GOB is not reclaimed if the reference count
**		XXX	is greater than zero, i.e., someone is subscribed to it.
**		XXX But the file descriptors associated with it _could_ be
**		XXX	reclaimed, especially since they are a scarce resource.
*/

#define CACHE_BLOOM_SIZE		(16 * 1024)
void
_gdp_gob_cache_reclaim(time_t maxage)
{
	static int headroom = 0;
	static long maxgobs;				// maximum number of GOBs in one pass

	ep_dbg_cprintf(Dbg, 68, "_gdp_gob_cache_reclaim(maxage = %ld)\n", maxage);

	// collect some parameters (once only)
	if (headroom == 0)
	{
		headroom = ep_adm_getintparam("swarm.gdp.cache.fd.headroom", 0);
		if (headroom <= 0)
		{
			int maxfds;
			(void) ep_app_numfds(&maxfds);
			headroom = maxfds - ((maxfds * 2) / 3);
			if (headroom == 0)
				headroom = 8;
		}
	}

	if (maxgobs <= 0)
	{
		maxgobs = ep_adm_getlongparam("swarm.gdp.cache.reclaim.maxgobs", 100000);
	}

	for (;;)
	{
		struct timeval tv;
		gdp_gob_t *g1, *g2;
		time_t mintime;
		long loopcount = 0;

		gettimeofday(&tv, NULL);
		mintime = tv.tv_sec - maxage;

		ep_thr_mutex_lock(&GobCacheMutex);
		if (!check_cache_consistency("_gdp_gob_cache_reclaim"))
			rebuild_lru_list();
		for (g1 = LIST_FIRST(&GobsByUse); g1 != NULL; g1 = g2)
		{
			if (loopcount++ > maxgobs)
			{
				EP_ASSERT_PRINT("_gdp_gob_cache_reclaim: processed %ld "
								"GOBS (giving up)",
							maxgobs);
				break;
			}
			g2 = LIST_NEXT(g1, ulist);		// do as early as possible

			if (ep_thr_mutex_trylock(&g1->mutex) != 0)
			{
				// couldn't get the lock: previous g2 setting may be wrong
				continue;
			}
			g1->flags |= GOBF_ISLOCKED;
			g2 = LIST_NEXT(g1, ulist);		// get g2 again with the lock
			if (g1->utime > mintime)
			{
				_gdp_gob_unlock(g1);
				continue;
			}
			if (EP_UT_BITSET(GOBF_DROPPING, g1->flags) || g1->refcnt > 0)
			{
				if (ep_dbg_test(Dbg, 19))
				{
					ep_dbg_printf("_gdp_gob_cache_reclaim: skipping %s:\n   ",
							EP_UT_BITSET(GOBF_DROPPING, g1->flags) ?
								"dropping" : "referenced");
					_gdp_gob_dump(g1, ep_dbg_getfile(), GDP_PR_DETAILED, 0);
				}
				_gdp_gob_unlock(g1);
				continue;
			}

			// OK, we really want to drop this GOB
			if (ep_dbg_test(Dbg, 32))
			{
				ep_dbg_printf("_gdp_gob_cache_reclaim: reclaiming:\n   ");
				_gdp_gob_dump(g1, ep_dbg_getfile(), GDP_PR_DETAILED, 0);
			}

			// remove from the LRU list and the name->handle cache
			_gdp_gob_cache_drop(g1, true);

			// release memory (this will also unlock the corpse)
			_gdp_gob_free(&g1);
		}
		ep_thr_mutex_unlock(&GobCacheMutex);

		// check to see if we have enough headroom
		int maxfds;
		int nfds = ep_app_numfds(&maxfds);

		if (nfds < maxfds - headroom)
			return;

		// try again, shortening timeout
		maxage -= maxage < 4 ? 1 : maxage / 4;
		if (maxage <= 0)
		{
			ep_log(EP_STAT_WARN,
					"_gdp_gob_cache_reclaim: cannot reach headroom %d, nfds %d",
					headroom, nfds);
			return;
		}
	}
}


/*
**  Shut down GOB cache --- immediately!
**
**		Informs subscribers of imminent shutdown.
**		Only used when the entire daemon is shutting down.
*/

void
_gdp_gob_cache_shutdown(void (*shutdownfunc)(gdp_req_t *))
{
	gdp_gob_t *g1, *g2;

	ep_dbg_cprintf(Dbg, 30, "\n_gdp_gob_cache_shutdown\n");

	// free all GOBs and all reqs linked to them
	// can give locking errors in some circumstances
	for (g1 = LIST_FIRST(&GobsByUse); g1 != NULL; g1 = g2)
	{
		ep_thr_mutex_trylock(&g1->mutex);
		g1->flags |= GOBF_ISLOCKED;
		g2 = LIST_NEXT(g1, ulist);
		_gdp_req_freeall(g1, NULL, shutdownfunc);
		_gdp_gob_free(&g1);	// also removes from cache and usage list
	}
}


void
_gdp_gob_cache_dump(int plev, FILE *fp)
{
	gdp_gob_t *gob;
	int ngobs = 0;			// actual number of GOBs in cache
	int maxprint = 30;		//XXX should be a parameter
	int maxgobs = 200;		//XXX ditto (this parameter is a loop breaker)

	if (fp == NULL)
		fp = ep_dbg_getfile();

	fprintf(fp, "\n<<< Showing cached GOBs by usage >>>\n");
	LIST_FOREACH(gob, &GobsByUse, ulist)
	{
		if (++ngobs > maxgobs)
			break;
		if (ngobs > maxprint)
			continue;
		VALGRIND_HG_DISABLE_CHECKING(gob, sizeof *gob);

		if (plev > GDP_PR_PRETTY)
		{
			_gdp_gob_dump(gob, fp, plev, 0);
		}
		else
		{
			struct tm *tm;
			char tbuf[40];

			if ((tm = localtime(&gob->utime)) != NULL)
				strftime(tbuf, sizeof tbuf, "%Y%m%d-%H%M%S", tm);
			else
				snprintf(tbuf, sizeof tbuf, "%"PRIu64, (int64_t) gob->utime);
			fprintf(fp, "%s %p %s %d\n", tbuf, gob, gob->pname, gob->refcnt);
		}
		if (ep_hash_search(_OpenGOBCache, sizeof gob->name, (void *) gob->name) == NULL)
			fprintf(fp, "    ===> WARNING: %s not in primary cache\n",
					gob->pname);
		VALGRIND_HG_ENABLE_CHECKING(gob, sizeof *gob);
	}
	if (ngobs <= maxprint)
		fprintf(fp, "\n<<< End of cached GOB list >>>\n");
	else
		fprintf(fp, "\n<<< End of cached GOB list (only %d of %d printed) >>>\n",
				maxprint, ngobs);
}


/*
**  Do a pass over all known GOBs.  Used for reclamation.
*/

void
_gdp_gob_cache_foreach(void (*f)(gdp_gob_t *))
{
	gdp_gob_t *g1;
	gdp_gob_t *g2;

	ep_thr_mutex_lock(&GobCacheMutex);
	for (g1 = LIST_FIRST(&GobsByUse); g1 != NULL; g1 = g2)
	{
		g2 = LIST_NEXT(g1, ulist);
		(*f)(g1);
	}
	ep_thr_mutex_unlock(&GobCacheMutex);
}
