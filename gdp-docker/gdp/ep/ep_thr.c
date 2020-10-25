/* vim: set ai sw=8 sts=8 ts=8 :*/

/***********************************************************************
**  ----- BEGIN LICENSE BLOCK -----
**	LIBEP: Enhanced Portability Library (Reduced Edition)
**
**	Copyright (c) 2008-2019, Eric P. Allman.  All rights reserved.
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
***********************************************************************/

#include <ep.h>
#include <ep_dbg.h>
#include <ep_thr.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/errno.h>

#if EP_OSCF_USE_PTHREADS

static EP_DBG	Dbg = EP_DBG_INIT("libep.thr", "Threading support");

bool	_EpThrUsePthreads = false;	// also used by ep_dbg_*

#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
# include <ep_string.h>
#endif

#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
pthread_key_t	lorder_key;
pthread_once_t	lorder_once = PTHREAD_ONCE_INIT;
struct lorder
{
	uint64_t	lorder_used;
};

#  define GETMTX(m)							\
	pthread_mutex_t *pmtx;						\
	int mtxorder EP_ATTR_UNUSED;					\
	if (m->magic != _EP_THR_MUTEX_MAGIC)				\
	{								\
	    ep_assert_print(file, line,					\
		    "passing non-debug mutex %p to debug library", m);	\
	    pmtx = (pthread_mutex_t *) m;				\
	    mtxorder = 0;						\
	}								\
	else								\
	{								\
	    pmtx = (pthread_mutex_t *) &m->pthr_mtx;			\
	    mtxorder = m->order;					\
	}

// This extern is needed because GNU insists that _GNU_SOURCE be defined
// to get the declaration.  But that pulls in a non-Posix strerror_r.
// Damned if you do, damned if you don't.
extern int	ffsl(long);

#else
#  define GETMTX(m)							\
	pthread_mutex_t *pmtx = m;
#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x02

#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x10
#  define CHECKMTX(m, e) \
    do	\
    {								\
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__lock,		\
			sizeof pmtx->__data.__lock);		\
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__owner,		\
			sizeof pmtx->__data.__owner);		\
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__nusers,	\
			sizeof pmtx->__data.__nusers);		\
	if (ep_dbg_test(Dbg, 98) &&				\
	    ((m)->__data.__lock > 1 ||				\
	     (m)->__data.__nusers > 1))				\
	{							\
		fprintf(stderr,					\
		    "%smutex_%s(%p): __lock=%x, __owner=%d, __nusers=%u%s\n", \
		    EpVid->vidfgred, e, m,			\
		    (m)->__data.__lock, (m)->__data.__owner,	\
		    (m)->__data.__nusers, EpVid->vidnorm);	\
	}							\
    } while (false)
#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x10

#ifndef CHECKMTX
# define CHECKMTX(m, e)
#endif

#ifndef CHECKCOND
# define CHECKCOND(c, e)
#endif

/*
**  Helper routines
*/

static void
diagnose_thr_err(int err,
		const char *where,
		const char *file,
		int line,
		const char *name,
		void *p)
{
	// timed out is not unexpected, so put it at a high debug level
	if (ep_dbg_test(Dbg, err == ETIMEDOUT ? 90 : 4))
	{
		char nbuf[40];

		(void) (0 == strerror_r(err, nbuf, sizeof nbuf));
		if (name == NULL)
			name = "???";
		ep_dbg_printf("ep_thr_%-13s: %s:%d %s (%p): %s\n",
				where, file, line, name, p, nbuf);
		ep_dbg_backtrace(NULL);
	}
	if (ep_dbg_test(Dbg, 101))
		ep_assert_failure(file, line,
				"exiting on thread error in %s",
				where);
}

# if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
static void
mtx_printtrace(EP_THR_MUTEX *m, const char *where,
		const char *file, int line, const char *name)
{
	EP_THR_ID my_tid = ep_thr_gettid();

	GETMTX(m);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__lock, sizeof pmtx->__data.__lock);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__owner, sizeof pmtx->__data.__owner);
	ep_dbg_printf("ep_thr_%-13s %s:%d %p (%s) [%d] __lock=%x __owner=%d%s\n",
			where, file, line, m, name, my_tid,
			pmtx->__data.__lock, pmtx->__data.__owner,
			_EpThrUsePthreads ? "" : " (ignored)");
}

static void
lock_printtrace(void *lock, const char *where,
		const char *file, int line, const char *name)
{
	EP_THR_ID my_tid = ep_thr_gettid();

	ep_dbg_printf("ep_thr_%-13s %s:%d %p (%s) [%" EP_THR_PRItid "]%s\n",
			where, file, line, lock, name, my_tid,
			_EpThrUsePthreads ? "" : " (ignored)");
}

#define TRACEMTX(m, where)	\
		if (ep_dbg_test(Dbg, 99))	\
			mtx_printtrace(m, where, file, line, name)

#else

static void
lock_printtrace(void *lock, const char *where,
		const char *file, int line, const char *name)
{
	pthread_t self = pthread_self();

	ep_dbg_printf("ep_thr_%-13s %s:%d %p (%s) [%p]%s\n",
			where, file, line, lock, name, (void *) self,
			_EpThrUsePthreads ? "" : " (ignored)");
}
#define TRACEMTX	TRACE

#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x01

#define TRACE(lock, where)	\
		if (ep_dbg_test(Dbg, 99))	\
			lock_printtrace(lock, where, file, line, name)

void
_ep_thr_init(void)
{
	_EpThrUsePthreads = true;
}


/*
**  Basics
*/

int
_ep_thr_spawn(EP_THR *thidp,
		void *(*thfunc)(void *),
		void *arg,
		const char *file,
		int line)
{
int r;

	// to make the TRACE call compile
	const char *name = NULL;

	TRACE(NULL, "spawn");
	if (!_EpThrUsePthreads)
		return EPERM;
	r = pthread_create(thidp, NULL, thfunc, arg);
	if (r != 0)
		diagnose_thr_err(errno, "spawn", file, line, NULL, NULL);
	return r;
}


void
_ep_thr_yield(const char *file, int line)
{
	// to make the TRACE call compile
	const char *name = NULL;

	TRACE(NULL, "yield");
	if (!_EpThrUsePthreads)
		return;
	if (sched_yield() < 0)
		diagnose_thr_err(errno, "yield", file, line, NULL, NULL);
}


EP_THR
ep_thr_getself(void)
{
	return pthread_self();
}


EP_THR_ID
ep_thr_gettid(void)
{
#if EP_OSCF_HAS_SYS_GETTID
	return syscall(SYS_gettid);
#else
	return pthread_self();
#endif
}


/*
**  Mutex implementation
*/

int
_ep_thr_mutex_init(EP_THR_MUTEX *mtx, int type,
		const char *file, int line, const char *name)
{
	int err;
	pthread_mutexattr_t attr;

	if (!_EpThrUsePthreads)
		return 0;
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	mtx->magic = _EP_THR_MUTEX_MAGIC;
	mtx->locker = 0;
	mtx->order = 0;
	mtx->l_file = NULL;
	mtx->l_line = 0;
	pthread_mutex_t *pmtx = &mtx->pthr_mtx;
#else
	pthread_mutex_t *pmtx = mtx;
#endif
	pthread_mutexattr_init(&attr);
	if (type == EP_THR_MUTEX_DEFAULT)
	{
		const char *mtype;

		mtype = ep_adm_getstrparam("libep.thr.mutex.type", "default");
		if (strcasecmp(mtype, "normal") == 0)
			type = PTHREAD_MUTEX_NORMAL;
		else if (strcasecmp(mtype, "errorcheck") == 0)
			type = PTHREAD_MUTEX_ERRORCHECK;
		else if (strcasecmp(mtype, "recursive") == 0)
			type = PTHREAD_MUTEX_RECURSIVE;
		else
			type = PTHREAD_MUTEX_DEFAULT;
	}
	pthread_mutexattr_settype(&attr, type);
	if ((err = pthread_mutex_init(pmtx, &attr)) != 0)
		diagnose_thr_err(err, "mutex_init", file, line, name, mtx);
	pthread_mutexattr_destroy(&attr);
	TRACEMTX(mtx, "mutex_init");
	CHECKMTX(mtx, "init <<<");
	return err;
}

int
_ep_thr_mutex_destroy(EP_THR_MUTEX *mtx,
		const char *file, int line, const char *name)
{
	int err;

	TRACEMTX(mtx, "mutex_destroy");
	if (!_EpThrUsePthreads)
		return 0;
	GETMTX(mtx);
	CHECKMTX(mtx, "destroy >>>");
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__lock, sizeof pmtx->__data.__lock);
	if (pmtx->__data.__lock != 0)
	{
		if (pmtx->__data.__lock == ep_thr_gettid())
			ep_assert_print(file, line,
				"_ep_thr_mutex_destroy: destroying self-locked"
				" mutex %p (%s)",
				pmtx, name);
		else
			ep_assert_print(file, line,
				"_ep_thr_mutex_destroy: destroying mutex "
				"%p (%s) locked by %d "
				"(I am %" EP_THR_PRItid ")",
				pmtx, name, pmtx->__data.__lock,
				ep_thr_gettid());
	}
#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (mtx->magic != _EP_THR_MUTEX_MAGIC)
		ep_assert_print(file, line, "mutex %s bad magic %x",
				name, mtx->magic);
	mtx->magic = 0xDEADBEEF;
#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if ((err = pthread_mutex_destroy(pmtx)) != 0)
		diagnose_thr_err(err, "mutex_destroy", file, line, name, mtx);
	return err;
}


#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
static void
lorder_free(void *lorder_)
{
	ep_mem_free(lorder_);
}

static void
lorder_init(void)
{
	int istat;
	struct lorder *lorder =
			(struct lorder *) ep_mem_zalloc(sizeof *lorder);

	lorder->lorder_used = 0;
	istat = pthread_key_create(&lorder_key, lorder_free);
	if (istat != 0)
		diagnose_thr_err(istat, "lorder_init", __FILE__, __LINE__,
				"pthread_key_create", NULL);
	istat = pthread_setspecific(lorder_key, lorder);
	if (istat != 0)
		diagnose_thr_err(istat, "lorder_init", __FILE__, __LINE__,
				"pthread_setspecific", NULL);
}

void
_ep_thr_mutex_setorder(EP_THR_MUTEX *mtx, int order,
		const char *file, int line, const char *name)
{
	if (mtx->order != 0 && mtx->order != order)
	{
		ep_dbg_cprintf(Dbg, 1,
				"_ep_thr_mutex_setorder: changing order from %d to %d\n",
				mtx->order, order);
	}
	mtx->order = order;
}

#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x02

int
_ep_thr_mutex_lock(EP_THR_MUTEX *mtx,
		const char *file, int line, const char *name)
{
	int err;

	TRACEMTX(mtx, "mutex_lock");
	if (!_EpThrUsePthreads)
		return 0;
	GETMTX(mtx);
	CHECKMTX(mtx, "lock >>>");
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__lock, sizeof pmtx->__data.__lock);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__owner, sizeof pmtx->__data.__owner);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__kind, sizeof pmtx->__data.__kind);
	if (pmtx->__data.__lock != 0 &&
	    pmtx->__data.__owner == ep_thr_gettid() &&
	    pmtx->__data.__kind != PTHREAD_MUTEX_RECURSIVE_NP)
	{
		ep_assert_print(file, line,
			"ep_thr_mutex_lock: mutex %p (%s) already self-locked",
			mtx, name);
	}
#elif EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (mtx->magic != _EP_THR_MUTEX_MAGIC)
		ep_assert_print(file, line, "mutex %s bad magic %x",
				name, mtx->magic);
	if (mtx->locker == ep_thr_gettid())
	{
		ep_dbg_cprintf(Dbg, 1,
			"ep_thr_mutex_lock at %s:%d: mutex %p (%s) already self-locked"
			" (%s:%d)\n",
			file, line,
			mtx, name, mtx->l_file, mtx->l_line);
	}
#endif
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	struct lorder *lorder;
	if (mtxorder > 0)
	{
		uint64_t mask;

		mask = ~((1 << (mtxorder - 1)) - 1) << 1;
		pthread_once(&lorder_once, lorder_init);
		lorder = (struct lorder *) pthread_getspecific(lorder_key);
		if (lorder != NULL)
		{
			int llu = ffsl(lorder->lorder_used & mask);
			if (llu > mtxorder)
			{
				ep_dbg_cprintf(Dbg, 1,
					"ep_thr_mutex_lock at %s:%d:"
					" mutex %p (%s) has order %d,"
					" but %d is already locked\n",
					file, line,
					mtx, name, mtxorder, llu);
			}
		}
	}
#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if ((err = pthread_mutex_lock(pmtx)) != 0)
		diagnose_thr_err(err, "mutex_lock", file, line, name, mtx);
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (err == 0)
	{
		if (mtxorder > 0 && lorder != NULL)
			lorder->lorder_used |= 1 << (mtxorder - 1);
		mtx->locker = ep_thr_gettid();
		mtx->l_file = file;
		mtx->l_line = line;
	}
#endif
	CHECKMTX(mtx, "lock <<<");
	return err;
}

int
_ep_thr_mutex_trylock(EP_THR_MUTEX *mtx,
		const char *file, int line, const char *name)
{
	int err;

	TRACEMTX(mtx, "mutex_trylock");
	if (!_EpThrUsePthreads)
		return 0;
	GETMTX(mtx);
	CHECKMTX(pmtx, "trylock >>>");
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__lock, sizeof pmtx->__data.__lock);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__owner, sizeof pmtx->__data.__owner);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__kind, sizeof pmtx->__data.__kind);
	if (pmtx->__data.__lock != 0 &&
	    pmtx->__data.__owner == ep_thr_gettid() &&
	    pmtx->__data.__kind != PTHREAD_MUTEX_RECURSIVE_NP)
	{
		// this is not necessarily an error
		ep_dbg_cprintf(Dbg, 1,
			"_ep_thr_mutex_lock: mutex %p (%s) "
			"already self-locked (%s:%d)\n",
			mtx, name, file, line);
	}
#elif EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (mtx->magic != _EP_THR_MUTEX_MAGIC)
		ep_assert_print(file, line, "mutex %s bad magic %x",
				name, mtx->magic);
	if (mtx->locker == ep_thr_gettid())
	{
		// this is not necessarily an error
		ep_dbg_cprintf(Dbg, 1,
			"ep_thr_mutex_lock: mutex %p (%s) already self-locked\n"
			"    (error at %s:%d, previous lock %s:%d)\n",
			mtx, name, file, line, mtx->l_file, mtx->l_line);
	}
#endif
	// EBUSY => mutex was already locked
	if ((err = pthread_mutex_trylock(pmtx)) != 0 && err != EBUSY)
		diagnose_thr_err(err, "mutex_trylock", file, line, name, mtx);
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	struct lorder *lorder;
	pthread_once(&lorder_once, lorder_init);
	lorder = (struct lorder *) pthread_getspecific(lorder_key);
	if (err == 0)
	{
		if (mtxorder > 0 && lorder != NULL)
			lorder->lorder_used |= 1 << (mtxorder - 1);
		mtx->locker = ep_thr_gettid();
		mtx->l_file = file;
		mtx->l_line = line;
	}
#endif
	CHECKMTX(mtx, "trylock <<<");
	return err;
}

int
_ep_thr_mutex_unlock(EP_THR_MUTEX *mtx,
		const char *file, int line, const char *name)
{
	int err;

	TRACEMTX(mtx, "mutex_unlock");
	if (!_EpThrUsePthreads)
		return 0;
	GETMTX(mtx);
	CHECKMTX(pmtx, "unlock >>>");
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__owner, sizeof pmtx->__data.__owner);
	if (pmtx->__data.__owner != ep_thr_gettid())
		ep_dbg_cprintf(Dbg, 1,
				"ep_thr_mutex_unlock at %s:%d:"
				" mtx owner = %d, I am %"EP_THR_PRItid "\n",
				file, line,
				pmtx->__data.__owner, ep_thr_gettid());
#elif EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (mtx->magic != _EP_THR_MUTEX_MAGIC)
		ep_assert_print(file, line, "mutex %s bad magic %x",
				name, mtx->magic);
	if (mtx->locker != ep_thr_gettid())
	{
		ep_dbg_cprintf(Dbg, 1,
				"ep_thr_mutex_unlock at %s:%d:"
				" mtx owner = %"EP_THR_PRItid " at %s:%d;"
				" I am %"EP_THR_PRItid "\n",
				file, line,
				mtx->locker, mtx->l_file, mtx->l_line,
				ep_thr_gettid());
	}
	else
	{
		mtx->locker = 0;	// presumptuous
	}
#endif
	if ((err = pthread_mutex_unlock(pmtx)) != 0)
		diagnose_thr_err(err, "mutex_unlock", file, line, name, mtx);
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (err == 0 && mtxorder > 0)
	{
		struct lorder *lorder;
		pthread_once(&lorder_once, lorder_init);
		lorder = (struct lorder *) pthread_getspecific(lorder_key);
		if (lorder != NULL)
			lorder->lorder_used &= ~(1 << (mtxorder - 1));
	}
#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	CHECKMTX(pmtx, "unlock <<<");
	return err;
}

int
_ep_thr_mutex_tryunlock(EP_THR_MUTEX *mtx,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(mtx, "mutex_tryunlock");
	if (!_EpThrUsePthreads)
		return 0;
	GETMTX(mtx);
	CHECKMTX(pmtx, "tryunlock >>>");
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__owner, sizeof pmtx->__data.__owner);
	if (pmtx->__data.__owner != ep_thr_gettid())
		ep_dbg_cprintf(Dbg, 1,
				"_ep_thr_mutex_unlock at %s:%d:"
				" mtx owner = %"EP_THR_PRItid ","
				" I am %" EP_THR_PRItid "\n",
				file, line,
				pmtx->__data.__owner, ep_thr_gettid());
#elif EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (mtx->locker != ep_thr_gettid())
	{
		ep_dbg_cprintf(Dbg, 1,
				"_ep_thr_mutex_unlock at %s:%d:"
				" mtx owner = %"EP_THR_PRItid " at %s:%d,"
				" I am %" EP_THR_PRItid "\n",
				file, line,
				mtx->locker, mtx->l_file, mtx->l_line,
				ep_thr_gettid());
	}
	else
	{
		mtx->locker = 0;	// presumptuous
	}
#endif
	// EAGAIN => mutex was not locked
	// EPERM  => mutex held by a different thread
	if ((err = pthread_mutex_unlock(pmtx)) != 0 &&
			err != EAGAIN && err != EPERM)
		diagnose_thr_err(err, "mutex_unlock", file, line, name, mtx);
	CHECKMTX(pmtx, "tryunlock <<<");
	return err;
}


int
_ep_thr_mutex_check(
		EP_THR_MUTEX *mtx,
		const char *file,
		int line,
		const char *mstr)
{
	CHECKMTX(mtx, "check ===");
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (mtx->magic != _EP_THR_MUTEX_MAGIC)
		ep_assert_print(file, line, "mutex %s bad magic %x",
				mstr, mtx->magic);
#endif
	return 0;
}


#undef ep_thr_mutex_assert_islocked

bool
ep_thr_mutex_assert_islocked(
			EP_THR_MUTEX *m,
			const char *mstr,
			const char *file,
			int line)
{
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
	GETMTX(m);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__lock, sizeof pmtx->__data.__lock);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__owner, sizeof pmtx->__data.__owner);
	if (pmtx->__data.__lock != 0 && pmtx->__data.__owner == ep_thr_gettid())
	{
		// OK, this is locked (by me)
		return true;
	}

	// oops, not locked or not locked by me
	if (pmtx->__data.__lock == 0)
		ep_assert_print(file, line,
				"mutex %s (%p) is not locked "
				"(should be %" EP_THR_PRItid ")",
				mstr, m, ep_thr_gettid());
	else
		ep_assert_print(file, line,
				"mutex %s (%p) locked by %d "
				"(should be %" EP_THR_PRItid ")",
				mstr, m, pmtx->__data.__owner, ep_thr_gettid());
	return false;
#elif EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (m->magic != _EP_THR_MUTEX_MAGIC)
		ep_assert_print(file, line, "mutex %s bad magic %x",
				mstr, m->magic);
	if (m->locker == 0)
		ep_assert_print(file, line, "mutex %s (%p) is not locked "
				"(should be %" EP_THR_PRItid ")",
				mstr, m, ep_thr_gettid());
	else if (m->locker != ep_thr_gettid())
		ep_assert_print(file, line,
				"mutex %s (%p) is locked by %" EP_THR_PRItid
				" (%s:%d), should be %" EP_THR_PRItid,
				mstr, m, m->locker, m->l_file, m->l_line,
				ep_thr_gettid());
	else
		return true;
	return false;
#else
	return true;
#endif
}


#undef ep_thr_mutex_assert_isunlocked

bool
ep_thr_mutex_assert_isunlocked(
			EP_THR_MUTEX *m,
			const char *mstr,
			const char *file,
			int line)
{
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
	GETMTX(m);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__lock, sizeof pmtx->__data.__lock);
	if (pmtx->__data.__lock == 0)
	{
		return true;
	}
	ep_assert_print(file, line,
			"mutex %s (%p) is locked by %d "
			"(should be unlocked; I am %" EP_THR_PRItid ")",
			mstr, m, pmtx->__data.__owner, ep_thr_gettid());
	return false;
#elif EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (m->magic != _EP_THR_MUTEX_MAGIC)
		ep_assert_print(file, line, "mutex %s bad magic %x",
				mstr, m->magic);
	if (m->locker == 0)
		return true;
	ep_assert_print(file, line,
			"mutex %s (%p) is locked by %" EP_THR_PRItid
			" (%s:%d), should be %" EP_THR_PRItid,
			mstr, m, m->locker, m->l_file, m->l_line,
			ep_thr_gettid());
	return false;
#else
	return true;
#endif
}


#undef ep_thr_mutex_assert_i_own

bool
ep_thr_mutex_assert_i_own(
			EP_THR_MUTEX *m,
			const char *mstr,
			const char *file,
			int line)
{
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x01
	GETMTX(m);
	VALGRIND_HG_CLEAN_MEMORY(&pmtx->__data.__owner, sizeof pmtx->__data.__owner);
	if (pmtx->__data.__owner == ep_thr_gettid())
	{
		return true;
	}
	ep_assert_print(file, line,
			"mutex %s (%p) is locked by %d "
			"(should be %" EP_THR_PRItid ")",
			mstr, m, pmtx->__data.__owner, ep_thr_gettid());
	return false;
#elif EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	if (m->magic != _EP_THR_MUTEX_MAGIC)
		ep_assert_print(file, line, "mutex %s bad magic %x",
				mstr, m->magic);
	if (m->locker == ep_thr_gettid())
		return true;
	ep_assert_print(file, line,
			"mutex %s (%p) is locked by %" EP_THR_PRItid
			" at %s:%d, should be %" EP_THR_PRItid,
			mstr, m, m->locker, m->l_file, m->l_line,
			ep_thr_gettid());
	return false;
#else
	return true;
#endif
}


/*
**  Condition Variable implementation
*/

int
_ep_thr_cond_init(EP_THR_COND *cv,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(cv, "cond_init");
	if (!_EpThrUsePthreads)
		return 0;
	if ((err = pthread_cond_init(cv, NULL)) != 0)
		diagnose_thr_err(err, "cond_init", file, line, name, cv);
	CHECKCOND(cv, "init <<<");
	return err;
}

int
_ep_thr_cond_destroy(EP_THR_COND *cv,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(cv, "cond_destroy");
	if (!_EpThrUsePthreads)
		return 0;
	CHECKCOND(cv, "destroy >>>");
	if ((err = pthread_cond_destroy(cv)) != 0)
		diagnose_thr_err(err, "cond_destroy", file, line, name, cv);
	return err;
}

int
_ep_thr_cond_signal(EP_THR_COND *cv,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(cv, "cond_signal");
	if (!_EpThrUsePthreads)
		return 0;
	CHECKCOND(cv, "signal >>>");
	if ((err = pthread_cond_signal(cv)) != 0)
		diagnose_thr_err(err, "cond_signal", file, line, name, cv);
	CHECKCOND(cv, "signal <<<");
	return err;
}

int
_ep_thr_cond_wait(EP_THR_COND *cv, EP_THR_MUTEX *mtx, EP_TIME_SPEC *timeout,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(cv, "cond_wait-cv");
	TRACEMTX(mtx, "cond-wait-mtx");
	if (!_EpThrUsePthreads)
		return 0;
	GETMTX(mtx);
	CHECKMTX(mtx, "wait >>>");
	CHECKCOND(cv, "wait >>>");
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	EP_THR_ID save_locker = mtx->locker;
	const char *save_l_file = mtx->l_file;
	int save_l_line = mtx->l_line;
#endif
	if (timeout == NULL)
	{
		err = pthread_cond_wait(cv, pmtx);
	}
	else
	{
		struct timespec ts;
		ts.tv_sec = timeout->tv_sec;
		ts.tv_nsec = timeout->tv_nsec;
		err = pthread_cond_timedwait(cv, pmtx, &ts);
	}
	if (err != 0)
		diagnose_thr_err(err, "cond_wait", file, line, name, cv);
#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
	mtx->locker = save_locker;
	mtx->l_file = save_l_file;
	mtx->l_line = save_l_line;
#endif
	CHECKMTX(mtx, "wait <<<");
	CHECKCOND(cv, "wait <<<");
	return err;
}

int
_ep_thr_cond_broadcast(EP_THR_COND *cv,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(cv, "cond_broadcast");
	if (!_EpThrUsePthreads)
		return 0;
	CHECKCOND(cv, "broadcast >>>");
	if ((err = pthread_cond_broadcast(cv)) != 0)
		diagnose_thr_err(err, "cond_broadcast", file, line, name, cv);
	CHECKCOND(cv, "broadcast <<<");
	return err;
}


/*
**  Read/Write Lock implementation
*/

int
_ep_thr_rwlock_init(EP_THR_RWLOCK *rwl,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(rwl, "rwlock_init");
	if (!_EpThrUsePthreads)
		return 0;
	if ((err = pthread_rwlock_init(rwl, NULL)) != 0)
		diagnose_thr_err(err, "rwlock_init", file, line, name, rwl);
	return err;
}

int
_ep_thr_rwlock_destroy(EP_THR_RWLOCK *rwl,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(rwl, "rwlock_destroy");
	if (!_EpThrUsePthreads)
		return 0;
	if ((err = pthread_rwlock_destroy(rwl)) != 0)
		diagnose_thr_err(err, "rwlock_destroy", file, line, name, rwl);
	return err;
}

int
_ep_thr_rwlock_rdlock(EP_THR_RWLOCK *rwl,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(rwl, "rwlock_rdlock");
	if (!_EpThrUsePthreads)
		return 0;
	if ((err = pthread_rwlock_rdlock(rwl)) != 0)
		diagnose_thr_err(err, "rwlock_rdlock", file, line, name, rwl);
	return err;
}

int
_ep_thr_rwlock_tryrdlock(EP_THR_RWLOCK *rwl,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(rwl, "rwlock_tryrdlock");
	if (!_EpThrUsePthreads)
		return 0;
	if ((err = pthread_rwlock_tryrdlock(rwl)) != 0)
		diagnose_thr_err(err, "rwlock_tryrdlock", file, line, name, rwl);
	return err;
}

int
_ep_thr_rwlock_wrlock(EP_THR_RWLOCK *rwl,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(rwl, "rwlock_wrlock");
	if (!_EpThrUsePthreads)
		return 0;
	if ((err = pthread_rwlock_wrlock(rwl)) != 0)
		diagnose_thr_err(err, "rwlock_wrlock", file, line, name, rwl);
	return err;
}

int
_ep_thr_rwlock_trywrlock(EP_THR_RWLOCK *rwl,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(rwl, "rwlock_tryrwlock");
	if (!_EpThrUsePthreads)
		return 0;
	if ((err = pthread_rwlock_trywrlock(rwl)) != 0)
		diagnose_thr_err(err, "rwlock_trywrlock", file, line, name, rwl);
	return err;
}

int
_ep_thr_rwlock_unlock(EP_THR_RWLOCK *rwl,
		const char *file, int line, const char *name)
{
	int err;

	TRACE(rwl, "rwlock_unlock");
	if (!_EpThrUsePthreads)
		return 0;
	if ((err = pthread_rwlock_unlock(rwl)) != 0)
		diagnose_thr_err(err, "rwlock_unlock", file, line, name, rwl);
	return err;
}

#else // !EP_OSCF_USE_PTHREADS

void
_ep_thr_init(void)
{
	ep_dbg_printf("WARNING: initializing pthreads, "
			"but pthreads compiled out\n");
}

#endif // EP_OSCF_USE_PTHREADS
