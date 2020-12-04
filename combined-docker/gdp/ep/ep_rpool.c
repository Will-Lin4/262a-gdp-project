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
#include <ep_rpool.h>
#include <ep_mem.h>
#include <ep_stat.h>
#include <ep_assert.h>
#include <ep_pcvt.h>
#include <ep_dbg.h>
#include <ep_funclist.h>
#include <ep_thr.h>
#include <unistd.h>
#include <string.h>


/***********************************************************************
**
**  RESOURCE POOLS
**
***********************************************************************/

/**************************  BEGIN PRIVATE  **************************/

// make memory _after_ MEMALIGN(s) bytes be on proper generic alignment
#ifndef EP_OSCF_MEM_ALIGNMENT
# define EP_OSCF_MEM_ALIGNMENT	4
#endif

// physical memory page size (OS dependent)
#ifndef EP_OSCF_MEM_PAGESIZE
# define EP_OSCF_MEM_PAGESIZE	getpagesize()
#endif

// return the address rounded up to the next EP_OSCF_MEM_ALIGNMENT boundary
#define MEMALIGN(s)		(((s) + (EP_OSCF_MEM_ALIGNMENT - 1)) & ~(EP_OSCF_MEM_ALIGNMENT - 1))

// system malloc and free functions
//	A valloc function can also be defined if available.
//	Note that memory allocated by valloc can't always be realloc'ed.
#ifndef EP_OSCF_SYSTEM_MALLOC
# define EP_OSCF_SYSTEM_MALLOC(s)	(malloc(s))
#endif

#ifndef EP_OSCF_SYSTEM_VALLOC
//# define EP_OSCF_SYSTEM_VALLOC(s)	(valloc(s))
//# define EP_OSCF_SYSTEM_VALLOC(s)	(memalign(s, EP_OSCF_MEM_PAGESIZE))
#endif

#ifndef EP_OSCF_SYSTEM_REALLOC
# define EP_OSCF_SYSTEM_REALLOC(p, s)	(realloc(p, s))
#endif

#ifndef EP_OSCF_SYSTEM_MFREE
# define EP_OSCF_SYSTEM_MFREE(p)	(free(p))
#endif

// number of tries to get more memory (XXX runtime?)
#ifndef N_MALLOC_TRIES
# define N_MALLOC_TRIES		3
#endif


/*
**  A segment in the memory part of a resource pool.
*/

struct rpseg
{
	// administrative pointers
	struct rpseg	*next;		// next segment in list

	// the actual memory
	size_t		segsize;	// size of this segment
	uint8_t *	segfree;	// free pointer
	uint8_t *	segbase;	// base of memory
};


/*
**  The user handle on resource pool (opaque)
*/

struct EP_RPOOL
{
	// administrative information
	const char	*name;		// for debugging
	size_t		qsize;		// quantum size
	uint32_t	flags;		// various flags, see below
	EP_THR_MUTEX	mutex;		// to avoid being tromped upon

	// pointers to the segment list
	struct rpseg	*head;		// pointer to allocatable memory

	// other resources
	EP_FUNCLIST	*ffuncs;	// list of free functions
};

// flag bits
#define BEING_FREED	0x00000001	// currently being freed
#define PRE_LOCKED	0x00000002	// we've already locked the struct



/***********************************************************************
**
**  RPOOL IMPLEMENTATION
**
***********************************************************************/


/*
**  EP_RPOOL_NEW -- create new resource pool
**
**	We tweak with the quantum size to get allocations that will be
**	approximately aligned, but then we don't use an explicitly aligned
**	malloc to get them.  Does this make any sense?
**
**	The default quantum size is the system I/O page size.  On some
**	architectures this might be large (65k and up), which could be a
**	waste.  Perhaps the default should be something like 2k?
**
**	Parameters:
**		name -- a pool name (optional, for debugging)
**		qsize -- quantum size, that is, suggested minimum
**			amount of memory to allocate in each segment.
**			This is a "hint" and can be ignored.
**
**	Returns:
**		A pointer to the new pool
*/

EP_RPOOL *
ep_rpool_new(const char *name,
	size_t qsize)
{
	EP_RPOOL *rp;
	int pagesize = EP_OSCF_MEM_PAGESIZE;	// XXX too large on some machines?

	if (name == NULL)
		name = "anonymous rpool";
	rp = (EP_RPOOL *) ep_mem_zalloc(sizeof *rp);
	rp->name = name;
	ep_thr_mutex_init(&rp->mutex, EP_THR_MUTEX_DEFAULT);

	if (qsize == 0)
	{
		// get a default quantum that seems reasonable
		qsize = ep_adm_getintparam("libep.rpool.quantum", 0);
	}

	// tweak qsize to allow for malloc and segment headers,
	// so the physical memory actually fits on even pages
	qsize = ((qsize + sizeof (struct rpseg) + pagesize - 1) &
			~(pagesize - 1)) - sizeof (struct rpseg);

	rp->qsize = qsize;

	return rp;
}


/*
**  EP_RPOOL_FREE -- free a resource pool
**
**	Parameters:
**		rp -- the pool to free
**
**	Returns:
**		void
*/

void
ep_rpool_free(EP_RPOOL *rp)
{
	struct rpseg *sp;
	struct rpseg *newsp;

	EP_ASSERT_POINTER_VALID(rp);

	// if already in process of freeing, ignore this request
	if (EP_UT_BITSET(BEING_FREED, rp->flags))
		return;

	// mark this as in process of being freed
	rp->flags |= BEING_FREED;

	// free any related resources
	if (rp->ffuncs != NULL)
	{
		ep_funclist_invoke(rp->ffuncs, rp);
		ep_funclist_free(rp->ffuncs);
	}

	// free up all memory segments associated with this pool
	for (sp = rp->head; sp != NULL; sp = newsp)
	{
		newsp = sp->next;
		if (sp->segbase != ((uint8_t *) sp) + sizeof *sp)
			ep_mem_free(sp->segbase);
		ep_mem_free(sp);
	}

	// now free the memory pool itself
	ep_thr_mutex_destroy(&rp->mutex);
	ep_mem_free(rp);
}


/*
**  EP_RPOOL_[MZ]ALLOC -- allocate memory from a resource pool
**
**	Allocate memory from an rpool.  If compile-time memory
**	debugging is turned on, these are implemented as macros
**	that call ep_rpool_ialloc, but these still exist in the
**	event that a module that is compiled without debugging
**	is linked into another application that does have
**	memory debugging compiled in.
**
**	Parameters:
**		rp -- the resource pool to allocate from.  If
**			NULL, allocates from the heap
**		nbytes -- how much memory to return
**
**	Returns:
**		The allocated memory
*/

#undef ep_rpool_malloc

void *
ep_rpool_malloc(EP_RPOOL *rp,
	size_t nbytes)
{
	return ep_rpool_ialloc(rp, nbytes, 0, NULL, 0);
}

#undef ep_rpool_zalloc

void *
ep_rpool_zalloc(EP_RPOOL *rp,
	size_t nbytes)
{
	return ep_rpool_ialloc(rp, nbytes, EP_MEM_F_ZERO, NULL, 0);
}

void *
ep_rpool_ralloc(EP_RPOOL *rp,
	size_t nbytes)
{
	return ep_rpool_ialloc(rp, nbytes, EP_MEM_F_TRASH, NULL, 0);
}


/*
**  EP_RPOOL_IALLOC -- allocate memory from a resource pool (internal)
**
**	Allocate memory from an rpool.  This is the underlying
**	routine that everyone else uses.
**
**	Parameters:
**		rp -- the resource pool to allocate from.  If
**			NULL, allocates from the heap
**		nbytes -- how much memory to return
**		flags -- flags updating semantics
**		grp -- an arbitrary group number (for debugging)
**		file -- file name where we were called (may be null)
**		line -- line number from file
**
**	Returns:
**		The allocated memory
*/

static EP_DBG	Dbg = EP_DBG_INIT("libep.rpool", "Resource pools");

void *
ep_rpool_ialloc(EP_RPOOL *rp,
	size_t nbytes,
	uint32_t flags,
	const char *file,
	int line)
{
	struct rpseg *sp;
	size_t spaceleft;
	uint8_t *p;
	bool aligned;

	if (ep_dbg_test(Dbg, 90))
	{
		ep_dbg_printf("ep_rpool_ialloc(%p, %zu)", rp, nbytes);
		if (rp != NULL)
			ep_dbg_printf(" [%s]", rp->name);
		ep_dbg_printf("\n");
	}

	// if no pool, treat this as a heap malloc
	if (rp == NULL)
		return ep_mem_ialloc(nbytes, NULL, flags, file, line);

	EP_ASSERT_POINTER_VALID(rp);
	if (!EP_UT_BITSET(PRE_LOCKED, rp->flags))
		ep_thr_mutex_lock(&rp->mutex);

	// round the size up so allocations will be even
	nbytes = MEMALIGN(nbytes);

	// see if this request wants page alignment
	aligned = (nbytes & (EP_OSCF_MEM_PAGESIZE - 1)) == 0;

	// see if there is enough space in the current segment
	sp = rp->head;
	if (sp == NULL)
		spaceleft = 0;
	else
		spaceleft = sp->segsize - (sp->segfree - sp->segbase);

	ep_dbg_cprintf(Dbg, 91,
			"rpool=%p, sp=%p, spaceleft=%zu, nbytes=%zu, aligned=%d\n",
			rp, sp, spaceleft, nbytes, aligned);
	if (sp == NULL)
	{
		ep_dbg_cprintf(Dbg, 92,
			"    (no current segment)\n");
	}
	else
	{
		ep_dbg_cprintf(Dbg, 92,
			"    segfree=%p, segbase=%p, segfree=%zu\n",
			sp->segfree, sp->segbase, sp->segsize);
	}

	// if there is no room, allocate a new segment
	if (sp == NULL || spaceleft < nbytes ||
	    (aligned && (((uintptr_t) sp->segfree & (EP_OSCF_MEM_PAGESIZE - 1)) != 0)))
	{
		size_t segsize;

		if (aligned)
		{
			/*
			**  This space is a multiple of a page size.
			**
			**	To keep it page aligned we allocate it
			**	separately from the segment descriptor.
			**	Unfortunately this may mean that the
			**	segment descriptors fragment memory.
			**	This might be fixed by keeping a free list
			**	of segment descriptors (ugh).
			**
			**	Note the assumption that ep_mem_ialloc
			**	will keep large blocks page aligned.  Our
			**	system_malloc will try to use valloc
			**	if available to ensure this.  Also,
			**	modern BSD systems do this automatically
			**	in malloc.
			*/

			sp = (struct rpseg *) ep_mem_ialloc(sizeof *sp,
					NULL,
					0,
					file,
					line);
			sp->segbase = (uint8_t *) ep_mem_ialloc(nbytes,
					NULL,
					0,
					file,
					line);
			segsize = nbytes;
		}
		else
		{
			// figure out how big this needs to be
			segsize = nbytes;
			if (segsize < rp->qsize)
				segsize = rp->qsize;

			// do the physical allocation
			p = (uint8_t *) ep_mem_ialloc(segsize + sizeof *sp,
					NULL,
					0,
					file,
					line);
			sp = (struct rpseg *) p;
			sp->segbase = p + sizeof *sp;
		}
		sp->segsize = segsize;

		/*
		**  Keep the segment with more free space at the head of the
		**  list.
		**
		**	This prevents alternate allocations of short and long
		**	buffers from wasting huge amounts of physical memory
		**	by clustering the short allocates together.
		*/

		if (rp->head != NULL && spaceleft > (segsize - nbytes))
		{
			// more space in old segment
			sp->next = rp->head->next;
			rp->head->next = sp;
		}
		else
		{
			// no, just put new block on head of list
			sp->next = rp->head;
			rp->head = sp;
		}

		// fill in other good info
		sp->segfree = sp->segbase;
	}

	// we have room -- now allocate and return the memory
	p = sp->segfree;
	sp->segfree += nbytes;

	if (!EP_UT_BITSET(PRE_LOCKED, rp->flags))
		ep_thr_mutex_unlock(&rp->mutex);

	// zero or trash memory as requested
	if (EP_UT_BITSET(EP_MEM_F_ZERO, flags))
		memset(p, 0, nbytes);
	else if (EP_UT_BITSET(EP_MEM_F_TRASH, flags))
		ep_mem_trash(p, nbytes);

	return p;
}


/*
**  EP_RPOOL_REALLOC -- expand a buffer located in an rpool
**
**	Can only extend the most recently allocated memory in the rpool.
**	Anything else will cause a memory leak (but only until the rpool
**		is freed).
*/

void *
ep_rpool_realloc(EP_RPOOL *rp,
	void *_emem,
	size_t oldsize,
	size_t newsize)
{
	uint8_t *emem = (uint8_t *) _emem;
	void *p;
	struct rpseg *sp;
	ssize_t spaceleft;

	if (ep_dbg_test(Dbg, 90))
	{
		ep_dbg_printf("ep_rpool_realloc(%p, %p, %zu, %zu)",
			rp, emem, oldsize, newsize);
		if (rp != NULL)
			ep_dbg_printf(" [%s]", rp->name);
		ep_dbg_printf("\n");
	}

	// easy case --- no rpool, so we can treat it as a heap
	if (rp == NULL)
	{
		p = ep_mem_realloc(emem, newsize);
		return p;
	}

	// dup code from ep_rpool_ialloc
	EP_ASSERT_POINTER_VALID(rp);
	if (!EP_UT_BITSET(PRE_LOCKED, rp->flags))
		ep_thr_mutex_lock(&rp->mutex);

	// round the size up so allocations will be even
	oldsize = MEMALIGN(oldsize);
	newsize = MEMALIGN(newsize);

	// see if there is enough space in the current segment
	sp = rp->head;
	if (sp == NULL)
		spaceleft = 0;
	else
		spaceleft = sp->segsize - (sp->segfree - sp->segbase);

	ep_dbg_cprintf(Dbg, 91,
			"rpool=%p, sp=%p, spaceleft=%zu\n",
			rp, sp, spaceleft);

	/*
	**  See if (a) we know where the old memory is coming from;
	**  (b) the old block is contained in the last segment;
	**  (c) the old block is the last one in the segment; and
	**  (d) there is enough room in the segment to hold the new
	**  allocation.
	*/

	if (emem != NULL && sp != NULL && spaceleft > 0 &&
	    (emem > sp->segbase && emem < sp->segfree) &&
	    emem + oldsize == sp->segfree &&
	    spaceleft >= 0 &&
	    newsize - oldsize <= (size_t) spaceleft)
	{
		// excellent; just increase the allocation
		sp->segfree += newsize - oldsize;
		p = emem;

		if (!EP_UT_BITSET(PRE_LOCKED, rp->flags))
			ep_thr_mutex_unlock(&rp->mutex);
	}
	else
	{
		// can't do it the easy way, so just find new space
		if (!EP_UT_BITSET(PRE_LOCKED, rp->flags))
			ep_thr_mutex_unlock(&rp->mutex);

		p = ep_rpool_malloc(rp, newsize);
		if (emem != NULL)
		{
			memcpy(p, emem, oldsize);
			ep_rpool_mfree(rp, emem);
		}
	}
	return p;
}


/*
**  EP_RPOOL_STRDUP, EP_RPOOL_TSTRDUP -- save a string into a resource pool
*/

#undef ep_rpool_strdup

char *
ep_rpool_strdup(
	EP_RPOOL *rp,
	const char *s)
{
	return ep_rpool_istrdup(rp, s, -1, 0, NULL, 0);
}

char *
ep_rpool_istrdup(
	EP_RPOOL *rp,
	const char *s,
	ssize_t slen,
	uint32_t flags,
	const char *file,
	int line)
{
	ssize_t l;
	char *p;

	if (s == NULL)
		return NULL;
	l = strlen(s);
	if (slen >= 0 && l > slen)
		l = slen;
	p = (char *) ep_rpool_ialloc(rp, l + 1, flags, file, line);
	memcpy(p, s, l);
	p[l] = '\0';
	return p;
}


/*
**
**  EP_RPOOL_MFREE -- free memory from a resource pool
**
**	Generally, this will only release memory if it is the most
**	recent memory allocated.  Can be ignored.
**
**	Long term: if all the allocations in a given segment are
**	freed, the segment can be freed back to the heap (or added
**	to the free list).  That requires more bookkeeping, which
**	might not be worth the cost.
**
**	Parameters:
**		rp -- pool from which this memory was allocated.
**			If NULL, free this to the heap.
**		p -- pointer to the actual memory
**
**	Returns:
**		void
*/

void
ep_rpool_mfree(EP_RPOOL *rp,
	void *p)
{
	// if no pool, treat this as a heap free
	if (rp == NULL)
	{
		ep_mem_free(p);
		return;
	}

	EP_ASSERT_POINTER_VALID(rp);

	// throw in some garbage at the beginning in case of reuse
	(*(void **)p) = EP_GEN_DEADBEEF;

	// not necessary to reclaim space, at least right now
	// (might someday prune from end)
}



/***********************************************************************
**
** EP_RPOOL_ATTACH -- attach a resource to a pool
**
**	Parameters:
**		rp -- the pool to attach the resource to.
**		freefunc -- the function to call when the pool is
**			freed
**		arg -- an argument to freefunc
**
**	Returns:
**		void
*/

void
ep_rpool_attach(EP_RPOOL *rp,
	EP_RPOOL_FREEFUNC *freefunc,
	void *arg)
{
	EP_ASSERT_POINTER_VALID(rp);

	ep_thr_mutex_lock(&rp->mutex);

	// if no function list associated with this pool, create one
	if (rp->ffuncs == NULL)
	{
		EP_FUNCLIST *fl;

		rp->flags |= PRE_LOCKED;
		ep_thr_mutex_unlock(&rp->mutex);
		fl = ep_funclist_new(rp->name);
		ep_thr_mutex_lock(&rp->mutex);
		rp->ffuncs = fl;
		rp->flags &= ~PRE_LOCKED;
	}
	ep_thr_mutex_unlock(&rp->mutex);

	// add this function to the function list
	ep_funclist_push(rp->ffuncs, freefunc, arg);
}


/***********************************************************************/
