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


/*
**  ASSERTIONS
**
**	Normally assertions are _not_ fatal.  This can be changed either
**	by the swarm.gdp.debug.assert.allabort runtime parameter or
**	by setting the EpAssertAllAbort global variable.  If either
**	is set, then all assertions (actually all calls to
**	_ep_assert_printv) cause immediate death.  This is to simplify
**	debugging, where ignoring an assertion could result in cascading
**	failures.
**
**	There are multiple ways to test assertions.
**
**		EP_ASSERT(condition)
**
**	If condition evaluates false, a message is printed and the
**	macro returns false, unless one of the previously mentioned
**	parameters indicate that abort should be immediate, in which
**	case the process exits.
**
**	Since the EP_ASSERT returns the boolean result of the condition
**	(equivalent to returning !!condition), recovery can be
**	performed by testing the result, e.g.,
**
**		if (!EP_ASSERT(condition))
**			do recovery;
**
**	A bit of syntactic sugar permits us to use:
**
**		EP_ASSERT_ELSE(condition, do recovery)
**
**	If the condition test is unneeded, just the error action
**	can be performed using EP_ASSERT_FAILURE or EP_ASSERT_PRINT;
**	the first aborts the process (unconditionally) and the second
**	behaves like EP_ASSERT with a false condition.
**
**	If the EpAssertInfo function pointer is set, that function
**	will be called as part of the process of printing the error.
**	It can be used to dump global state.  It is called before
**	EpAssertAllAbort is tested, so it can set that variable true
**	if it wants the process to exit immediately.
**
**	If the process does abort and EpAssertAbort is set, that
**	function is called immediately before the process exits.
**	If it returns, abort(3) is called.  It could, for example,
**	kill the current thread as opposed to the current process.
**
**	If _EP_CCCF_ASSERT_NONE is set during compilation, then all
**	assertions are compiled out.
*/


#ifndef _EP_ASSERT_H_
#define _EP_ASSERT_H_

#include "ep.h"

__BEGIN_DECLS

#if !_EP_CCCF_ASSERT_NONE

// assert that an expression must be true
#define EP_ASSERT(e)							\
		((e)							\
			? (true)					\
			: (ep_assert_print(__FILE__, __LINE__,		\
					"%s", #e), false))

// test for condition (a bit odd because the macro is true if the
// expression is false)  [DEPRECATED]
#define EP_ASSERT_TEST(e)						\
		((e)							\
			? (false)					\
			: (ep_assert_print(__FILE__, __LINE__,		\
					"%s", #e), true))

// assert condition with recovery (note: doesn't use do ... while (false)
// paradigm to allow break & continue in recovery actions)
#define EP_ASSERT_ELSE(e, r)						\
		if (!(e))						\
		{							\
			ep_assert_print(__FILE__, __LINE__,		\
					"%s", #e);			\
			r;						\
		}


// force assertion failure and abort
#define EP_ASSERT_FAILURE(...)						\
		ep_assert_failure(__FILE__, __LINE__, __VA_ARGS__)

// print assertion failure and optionally continue
#define EP_ASSERT_PRINT(...)						\
		ep_assert_print(__FILE__, __LINE__, __VA_ARGS__)

// force abort due to assertion failure (should be called ep_assert_abort)
extern void EP_TYPE_PRINTFLIKE(3, 4)
		ep_assert_failure(
			const char *file,
			int line,
			const char *msg,
			...)
			EP_ATTR_NORETURN;

// print an assertion failure, but generally won't cause an abort
extern void	ep_assert_printv(
			const char *file,
			int line,
			const char *msg,
			va_list av);

extern void EP_TYPE_PRINTFLIKE(3, 4)
		ep_assert_print(
			const char *file,
			int line,
			const char *msg,
			...);

#else // _EP_CCCF_ASSERT_NONE

#define EP_ASSERT(e)			true
#define EP_ASSERT_TEST(e)		false
#define EP_ASSERT_ELSE(e, r)
#define EP_ASSERT_FAILURE(...)
#define EP_ASSERT_PRINT(...)
#define ep_assert_failure(f, l, m, ...)
#define ep_assert_printv(f, l, m, av)
#define ep_assert_print(f, l, m, ...)

#endif // _EP_CCCF_ASSERT_NONE

// callback functions
extern void	(*EpAssertInfo)(void);		// show additional info
extern void	(*EpAssertAbort)(void);		// abort the thread/process

// control flags
extern bool	EpAssertAllAbort;		// abort on all assertions

// back compatibility --- these are deprecated
#define EP_ASSERT_REQUIRE(e)		EP_ASSERT(e)
#define EP_ASSERT_ENSURE(e)		EP_ASSERT(e)
#define EP_ASSERT_INSIST(e)		EP_ASSERT(e)
#define EP_ASSERT_INVARIANT(e)		EP_ASSERT(e)

__END_DECLS

#endif /*_EP_ASSERT_H_*/
