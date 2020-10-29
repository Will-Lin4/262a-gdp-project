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

#ifndef _EP_THR_H_
#define _EP_THR_H_

# include <ep/ep.h>
# include <ep/ep_time.h>

# if EP_OSCF_USE_PTHREADS
#  include <pthread.h>

__BEGIN_DECLS

/*
**  THREADS LOCKING SUPPORT
**
**  These are mostly one-for-one replacements for pthreads.
**  The main differences are:
**
**  (a)	They can be compiled out if you don't have pthreads available.
**	By default they are compiled in, but "-DEP_OSCF_USE_PTHREADS=0"
**	will undo this.
**  (b) The routines themselves have debugging output in the event of
**	errors.
**  (c)	These definitions only cover locking.  The assumption is that
**	if you need threads then you'll do all the thread creation/
**	destruction/control using native pthreads primitives.  In fact,
**	these routines are probably only useful for libraries.
**  (d) It is essential that the application call
**		ep_lib_init(EP_LIB_USEPTHREADS);
**	Otherwise these routines are all null.
**
**  This could be made slightly more efficient by changing all calls
**  to "ep_thr_XYZ" to (_EpThrUsePthreads ? _ep_thr_XYZ : 0).  It's
**  not clear that it's worth the complexity, but it's easy to do in
**  the future if it is.
**
**  XXX	Should these return an EP_STAT instead of an int?
**	PRO:	matches other routines, allows for possible non-errno
**		values in the future.
**	CON:	makes the ep_thr_* routines not drop-in replacements
**		for pthreads_* routines.
**	NOTE:	There's not much to do if these routines fail (kind
**		of like malloc), so unfortunately the return values
**		are usually ignored.  Just live with it?
**
**  XXX	I started implementing the mutex routines as recursive locks,
**	mostly so ep_funclist_invoke would work.  This is how the
**	stdio libraries deal with the problem.  Problem is, that
**	wouldn't fix the problem (because of local variables), so I
**	went for simplicity.
**
**  EP_OPT_EXTENDED_MUTEX_CHECK adds special mutex debugging.  It can
**	slow things down, so it doesn't default on.  It is a bitmap:
**
**	0x01	Do rationality checks on locks, notably checking for
**		self-locking non-recursive mutexes, unlocking a mutex
**		we don't own, etc.  It also adds assertions that
**		mutexes must be locked (by me) or unlocked.
**		This peeks into the pthread mutex implementation, and
**		hence only runs on Linux systems using NPTL.
**	0x02	Add primitive lock ordering checks.  This changes the
**		ABI!!!  As a result, it should not be used in production.
**	0x10	Do deeper lock checks (every time one is used).
**		Probably inefficient.
*/

#ifndef EP_OPT_EXTENDED_MUTEX_CHECK				//XXX DEBUG TEMP
# ifdef __linux							//XXX DEBUG TEMP
#  define EP_OPT_EXTENDED_MUTEX_CHECK	0x03			//XXX DEBUG TEMP
# else								//XXX DEBUG TEMP
#  define EP_OPT_EXTENDED_MUTEX_CHECK	0x02			//XXX DEBUG TEMP
# endif								//XXX DEBUG TEMP
#endif								//XXX DEBUG TEMP
#if (EP_OPT_EXTENDED_MUTEX_CHECK & 0x11) && !defined(__linux__)
# warning EP_OPT_EXTENDED_MUTEX_CHECK only works on Linux
# undef EP_OPT_EXTENDED_MUTEX_CHECK
#endif

#if EP_OSCF_USE_VALGRIND
# include <valgrind/helgrind.h>
#else
# define VALGRIND_HG_CLEAN_MEMORY(a, b)
# define VALGRIND_HG_DISABLE_CHECKING(p, s)
# define VALGRIND_HG_ENABLE_CHECKING(p, s)
#endif


typedef pthread_t		EP_THR;

#if EP_OSCF_HAS_SYS_GETTID
typedef int			EP_THR_ID;
# include <sys/syscall.h>
# define EP_THR_PRItid		"d"
#else
typedef void			*EP_THR_ID;
# define EP_THR_PRItid		"p"
#endif


extern EP_THR_ID	ep_thr_gettid(void);
extern EP_THR		ep_thr_getself(void);

extern int		_ep_thr_spawn(EP_THR *th,
					void *(*thfunc)(void *),
					void *arg,
					const char *file,
					int line);
#define			ep_thr_spawn(thr, thfunc, arg) \
					_ep_thr_spawn(thr, thfunc, arg, \
					__FILE__, __LINE__)
extern void		_ep_thr_yield(const char *file,
					int line);
#define			ep_thr_yield() \
					_ep_thr_yield(__FILE__, __LINE__)

#if EP_OPT_EXTENDED_MUTEX_CHECK & 0x02
#  define _EP_THR_MUTEX_MAGIC	0x454d5458	// EMTX

struct ep_mutex
{
	uint32_t		magic;		// _EP_THR_MUTEX_MAGIC
	pthread_mutex_t		pthr_mtx;	// the actual mutex
	EP_THR_ID		locker;		// identity of locker
	const char		*l_file;	// file of locker
	int			l_line;		// line of locker
	uint8_t			order;		// lock order indicator
};
typedef struct ep_mutex		EP_THR_MUTEX;
#  define	EP_THR_MUTEX_INITIALIZER	=			\
			{						\
				_EP_THR_MUTEX_MAGIC,			\
				PTHREAD_MUTEX_INITIALIZER,		\
				0,					\
				NULL, 0,				\
				0,					\
			}
#  define	EP_THR_MUTEX_INITIALIZER2(o)	=			\
			{						\
				_EP_THR_MUTEX_MAGIC,			\
				PTHREAD_MUTEX_INITIALIZER,		\
				0,					\
				NULL, 0,				\
				o,					\
			}

#  define ep_thr_mutex_setorder(mtx, order)				\
		_ep_thr_mutex_setorder(mtx, order,			\
				__FILE__, __LINE__, #mtx)

extern void	_ep_thr_mutex_setorder(EP_THR_MUTEX *mtx, int order,
				const char *file, int line, const char *name);

#else
typedef pthread_mutex_t		EP_THR_MUTEX;
#  define	EP_THR_MUTEX_INITIALIZER	= PTHREAD_MUTEX_INITIALIZER
#  define	EP_THR_MUTEX_INITIALIZER2(o)	= PTHREAD_MUTEX_INITIALIZER
#  define	ep_thr_mutex_setorder(mtx, order)
#endif // EP_OPT_EXTENDED_MUTEX_CHECK & 0x02

#define		EP_THR_MUTEX_NORMAL		PTHREAD_MUTEX_NORMAL
#define		EP_THR_MUTEX_ERRORCHECK		PTHREAD_MUTEX_ERRORCHECK
#define		EP_THR_MUTEX_RECURSIVE		PTHREAD_MUTEX_RECURSIVE
#define		EP_THR_MUTEX_DEFAULT		PTHREAD_MUTEX_DEFAULT

extern int	_ep_thr_mutex_init(EP_THR_MUTEX *mtx, int type,
				const char *file, int line, const char *name);
extern int	_ep_thr_mutex_destroy(EP_THR_MUTEX *mtx,
				const char *file, int line, const char *name);
extern int	_ep_thr_mutex_lock(EP_THR_MUTEX *mtx,
				const char *file, int line, const char *name);
extern int	_ep_thr_mutex_trylock(EP_THR_MUTEX *mtx,
				const char *file, int line, const char *name);
extern int	_ep_thr_mutex_unlock(EP_THR_MUTEX *mtx,
				const char *file, int line, const char *name);
extern int	_ep_thr_mutex_tryunlock(EP_THR_MUTEX *mtx,
				const char *file, int line, const char *name);
extern int	_ep_thr_mutex_check(EP_THR_MUTEX *mtx,
				const char *file, int line, const char *name);
#define		ep_thr_mutex_init(mtx, type)	_ep_thr_mutex_init(mtx, type, \
				__FILE__, __LINE__, #mtx)
#define		ep_thr_mutex_destroy(mtx)	_ep_thr_mutex_destroy(mtx, \
				__FILE__, __LINE__, #mtx)
#define		ep_thr_mutex_lock(mtx)		_ep_thr_mutex_lock(mtx, \
				__FILE__, __LINE__, #mtx)
#define		ep_thr_mutex_trylock(mtx)	_ep_thr_mutex_trylock(mtx, \
				__FILE__, __LINE__, #mtx)
#define		ep_thr_mutex_unlock(mtx)	_ep_thr_mutex_unlock(mtx, \
				__FILE__, __LINE__, #mtx)
#define		ep_thr_mutex_tryunlock(mtx)	_ep_thr_mutex_tryunlock(mtx, \
				__FILE__, __LINE__, #mtx)
#define		ep_thr_mutex_check(mtx)		_ep_thr_mutex_check(mtx, \
				__FILE__, __LINE__, #mtx)

#if EP_OPT_EXTENDED_MUTEX_CHECK
extern bool	ep_thr_mutex_assert_islocked(
			EP_THR_MUTEX *, const char *, const char *, int);
extern bool	ep_thr_mutex_assert_isunlocked(
			EP_THR_MUTEX *, const char *, const char *, int);
extern bool	ep_thr_mutex_assert_i_own(
			EP_THR_MUTEX *, const char *, const char *, int);

# define	EP_THR_MUTEX_ASSERT_ISLOCKED(m)				\
			ep_thr_mutex_assert_islocked(m, #m, __FILE__, __LINE__)
# define	EP_THR_MUTEX_ASSERT_ISUNLOCKED(m)			\
			ep_thr_mutex_assert_isunlocked(m, #m, __FILE__, __LINE__)
#else
# define	ep_thr_mutex_assert_islocked(m, i, f, l)	true
# define	ep_thr_mutex_assert_isunlocked(m, i, f, l)	true
# define	ep_thr_mutex_assert_i_own(m, i, f, l)		true
# define	EP_THR_MUTEX_ASSERT_ISLOCKED(m)
# define	EP_THR_MUTEX_ASSERT_ISUNLOCKED(m)
#endif

typedef pthread_cond_t		EP_THR_COND;
#  define	EP_THR_COND_INITIALIZER		= PTHREAD_COND_INITIALIZER
extern int	_ep_thr_cond_init(EP_THR_COND *cv,
				const char *file, int line, const char *name);
extern int	_ep_thr_cond_destroy(EP_THR_COND *cv,
				const char *file, int line, const char *name);
extern int	_ep_thr_cond_signal(EP_THR_COND *cv,
				const char *file, int line, const char *name);
extern int	_ep_thr_cond_wait(EP_THR_COND *cv, EP_THR_MUTEX *mtx,
				EP_TIME_SPEC *timeout,
				const char *file, int line, const char *name);
extern int	_ep_thr_cond_broadcast(EP_THR_COND *cv,
				const char *file, int line, const char *name);
#define		ep_thr_cond_init(cv)		_ep_thr_cond_init(cv, \
				__FILE__, __LINE__, #cv)
#define		ep_thr_cond_destroy(cv)		_ep_thr_cond_destroy(cv, \
				__FILE__, __LINE__, #cv)
#define		ep_thr_cond_signal(cv)		_ep_thr_cond_signal(cv, \
				__FILE__, __LINE__, #cv)
#define		ep_thr_cond_wait(cv, mtx, tout)	_ep_thr_cond_wait(cv, mtx, tout, \
				__FILE__, __LINE__, #cv)
#define		ep_thr_cond_broadcast(cv)	_ep_thr_cond_broadcast(cv, \
				__FILE__, __LINE__, #cv)

typedef pthread_rwlock_t	EP_THR_RWLOCK;
#  define	EP_THR_RWLOCK_INITIALIZER	= PTHREAD_RWLOCK_INITIALIZER
extern int	_ep_thr_rwlock_init(EP_THR_RWLOCK *rwl,
				const char *file, int line, const char *name);
extern int	_ep_thr_rwlock_destroy(EP_THR_RWLOCK *rwl,
				const char *file, int line, const char *name);
extern int	_ep_thr_rwlock_rdlock(EP_THR_RWLOCK *rwl,
				const char *file, int line, const char *name);
extern int	_ep_thr_rwlock_tryrdlock(EP_THR_RWLOCK *rwl,
				const char *file, int line, const char *name);
extern int	_ep_thr_rwlock_wrlock(EP_THR_RWLOCK *rwl,
				const char *file, int line, const char *name);
extern int	_ep_thr_rwlock_trywrlock(EP_THR_RWLOCK *rwl,
				const char *file, int line, const char *name);
extern int	_ep_thr_rwlock_unlock(EP_THR_RWLOCK *rwl,
				const char *file, int line, const char *name);
#define		ep_thr_rwlock_init(rwl)		_ep_thr_rwlock_init(rwl, \
				__FILE__, __LINE__, #rwl)
#define		ep_thr_rwlock_destroy(rwl)	_ep_thr_rwlock_destroy(rwl, \
				__FILE__, __LINE__, #rwl)
#define		ep_thr_rwlock_rdlock(rwl)	_ep_thr_rwlock_rdlock(rwl, \
				__FILE__, __LINE__, #rwl)
#define		ep_thr_rwlock_tryrdlock(rwl)	_ep_thr_rwlock_tryrdlock(rwl, \
				__FILE__, __LINE__, #rwl)
#define		ep_thr_rwlock_wrlock(rwl)	_ep_thr_rwlock_wrlock(rwl, \
				__FILE__, __LINE__, #rwl)
#define		ep_thr_rwlock_trywrlock(rwl)	_ep_thr_rwlock_trywrlock(rwl, \
				__FILE__, __LINE__, #rwl)
#define		ep_thr_rwlock_unlock(rwl)	_ep_thr_rwlock_unlock(rwl, \
				__FILE__, __LINE__, #rwl)

/*
**  Thread pool declarations
*/

// initialize a thread pool
void		ep_thr_pool_init(
			int min_threads,	// min number of threads
			int max_threads,	// max number of threads
			uint32_t flags);	// flag bits (none yet)

// run a function in a worker thread from the pool
void		ep_thr_pool_run(
			void (*func)(void *),	// the function
			void *arg);		// passed to func

# else // ! EP_OSCF_USE_PTHREADS

# define	ep_thr_yield()

typedef int	EP_THR_MUTEX;
#  define	EP_THR_MUTEX_INITIALIZER
#  define	ep_thr_mutex_init(mtx, type)	0
#  define	ep_thr_mutex_destroy(mtx)	0
#  define	ep_thr_mutex_lock(mtx)		0
#  define	ep_thr_mutex_trylock(mtx)	0
#  define	ep_thr_mutex_unlock(mtx)	0
#  define	ep_thr_mutex_check(mtx)		0
#define		EP_THR_MUTEX_NORMAL		0
#define		EP_THR_MUTEX_ERRORCHECK		0
#define		EP_THR_MUTEX_RECURSIVE		0

typedef int	EP_THR_COND;
#  define	EP_THR_COND_INITIALIZER
#  define	ep_thr_cond_init(cv)		0
#  define	ep_thr_cond_destroy(cv)		0
#  define	ep_thr_cond_signal(cv)		0
#  define	ep_thr_cond_wait(cv, mtx, to)	0
#  define	ep_thr_cond_broadcast(cv)	0

typedef int	EP_THR_RWLOCK;
#  define	EP_THR_RWLOCK_INITIALIZER
#  define	ep_thr_rwlock_init(rwl)		0
#  define	ep_thr_rwlock_destroy(rwl)	0
#  define	ep_thr_rwlock_rdlock(rwl)	0
#  define	ep_thr_rwlock_tryrdlock(rwl)	0
#  define	ep_thr_rwlock_wrlock(rwl)	0
#  define	ep_thr_rwlock_trywrlock(rwl)	0
#  define	ep_thr_rwlock_unlock(rwl)	0

# endif // EP_OSCF_USE_PTHREADS

__END_DECLS
#endif // _EP_THR_H_
