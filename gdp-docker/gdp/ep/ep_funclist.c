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

/***********************************************************************
**
**  FUNCTION LISTS
**
**	Used anywhere that you might need a dynamic list of functions
**
***********************************************************************/

#include <ep.h>
#include <ep_funclist.h>
#include <ep_rpool.h>
#include <ep_stat.h>
#include <ep_thr.h>
#include <ep_assert.h>


/*
**  Function lists
**
**	Functions take two arguments.  The first is set when the
**	call is entered into the list.  It acts as a closure.
**	The second is specified when the list is actually invoked.
*/


/**************************  BEGIN PRIVATE  **************************/

// per-entry structure
struct fseg
{
	struct fseg		*next;		// next in list
	EP_FUNCLIST_FUNC	*func;		// the function to call
	void			*closure;	// first argument to that func
};

// per-list structure
struct EP_FUNCLIST
{
	// administrative....
	const char	*name;			// for debugging
	uint32_t	flags;			// flag bits, see below
	EP_RPOOL	*rpool;			// resource pool
	EP_THR_MUTEX	mutex;			// locking node

	// memory for actual functions
	struct fseg	*list;			// the actual list
};

// flags bits
//  none at this time

/***************************  END PRIVATE  ***************************/



EP_FUNCLIST *
ep_funclist_new(
	const char *name)
{
	uint32_t flags = 0;
	EP_FUNCLIST *flp;
	EP_RPOOL *rp;

	if (name == NULL)
		name = "<funclist>";

	rp = ep_rpool_new(name, (size_t) 0);
	flp = (EP_FUNCLIST *) ep_rpool_zalloc(rp, sizeof *flp);

	flp->flags = flags;
	flp->rpool = rp;
	ep_thr_mutex_init(&flp->mutex, EP_THR_MUTEX_DEFAULT);

	return flp;
}

void
ep_funclist_free(EP_FUNCLIST *flp)
{
	EP_ASSERT_POINTER_VALID(flp);
	ep_thr_mutex_destroy(&flp->mutex);
	ep_rpool_free(flp->rpool);	// also frees flp
}

void
ep_funclist_push(EP_FUNCLIST *flp,
	EP_FUNCLIST_FUNC *func,
	void *closure)
{
	struct fseg *fsp;

	EP_ASSERT_POINTER_VALID(flp);
	ep_thr_mutex_lock(&flp->mutex);

	// allocate a new function block and fill it
	fsp = (struct fseg *) ep_rpool_malloc(flp->rpool, sizeof *fsp);
	fsp->func = func;
	fsp->closure = closure;

	// link it onto the list
	fsp->next = flp->list;
	flp->list = fsp;

	ep_thr_mutex_unlock(&flp->mutex);
}


void
ep_funclist_invoke(EP_FUNCLIST *flp,
	void *arg)
{
	struct fseg *fsp;

	EP_ASSERT_POINTER_VALID(flp);

	//XXX possible deadlock if a called function tries to modify the list
	ep_thr_mutex_lock(&flp->mutex);
	for (fsp = flp->list; fsp != NULL; fsp = fsp->next)
	{
		(*fsp->func)(fsp->closure, arg);
	}
	ep_thr_mutex_unlock(&flp->mutex);
}
