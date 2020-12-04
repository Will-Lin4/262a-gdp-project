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
#include <ep_string.h>

static EP_DBG	Dbg = EP_DBG_INIT("libep.init", "initialization");

/*
**  EP_LIB_INIT --- initialize the library
*/

extern void	_ep_stat_init(void);
extern void	_ep_thr_init(void);

bool	_EpLibInitialized = false;

EP_STAT
ep_lib_init(uint32_t flags)
{
	if (_EpLibInitialized)
		return EP_STAT_OK;
	if (EP_UT_BITSET(EP_LIB_USEPTHREADS, flags))
		_ep_thr_init();
	_ep_stat_init();
	ep_adm_readparams("defaults");
	ep_str_vid_set(NULL);
	ep_str_char_set(NULL);
	ep_dbg_init();
	_EpLibInitialized = true;

	// set default debug flags, useful for debugging initialization
	const char *defdbg = ep_adm_getstrparam("libep.dbg.default", NULL);
	if (defdbg != NULL)
		ep_dbg_set(defdbg);
	ep_dbg_cprintf(Dbg, 1, "ep_lib_init:\n");

	return EP_STAT_OK;
}
