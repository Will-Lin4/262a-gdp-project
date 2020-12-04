/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	GDP_COMPAT_V0.H --- compatibility with v0 of the Global Data Plane
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

#ifndef _GDP_COMPAT_V0_H_
#define _GDP_COMPAT_V0_H_

/*
**  Simple substitutions (name changes)
*/

// type names
#define gdp_gcl_t				gdp_gin_t
#define gdp_gclmd_t				gdp_md_t

// simple functions
#define gdp_gcl_create			gdp_gin_create
#define	gdp_gcl_open			gdp_gin_open
#define gdp_gcl_close			gdp_gin_close
#define gdp_gcl_read			gdp_gin_read_by_recno
#define gdp_gcl_read_ts			gdp_gin_read_by_ts
#define gdp_gcl_unsubscribe		gdp_gin_unsubscribe
#define gdp_gcl_multiread		gdp_gin_read_by_recno_async
#define gdp_gcl_multiread_ts	gdp_gin_read_by_ts_async
#define gdp_gcl_getmetadata		gdp_gin_getmetadata
#define gdp_gcl_getname			gdp_gin_getname
#define gdp_gcl_getnrecs		gdp_gin_getnrecs
#define gdp_gcl_print			gdp_gin_print
#define gdp_gclmd_id_t			gdp_md_id_t
#define gdp_gclmd_new			gdp_md_new
#define gdp_gclmd_free			gdp_md_free
#define gdp_gclmd_add			gdp_md_add
#define gdp_gclmd_get			gdp_md_get
#define gdp_gclmd_find			gdp_md_find

#define	gdp_gcl_open_info_t					gdp_open_info_t
#define	gdp_gcl_open_info_new				gdp_open_info_new
#define	gdp_gcl_open_info_free				gdp_open_info_free
#define gdp_gcl_open_info_set_signing_key	gdp_open_info_set_signing_key
#define gdp_gcl_open_info_set_signkey_cb	gdp_open_info_set_signkey_cb
#define gdp_gcl_open_info_set_caching		gdp_open_info_set_caching
#define gdp_gcl_open_info_free				gdp_open_info_free
#define gdp_gcl_set_append_filter			gdp_gin_set_append_filter
#define gdp_gcl_set_read_filter				gdp_gin_set_read_filter

// manifest constants
#define GDP_EVENT_EOS			GDP_EVENT_DONE

/*
**  Functions with parameter changes
*/

#define gdp_gcl_append(gcl, datum)											\
				gdp_gin_append(gcl, datum, NULL)
#define gdp_gcl_append_async(gcl, datum, cbfunc, cbarg)						\
				gdp_gin_append_async(gcl, 1, &datum, NULL, cbfunc, cbarg)
#define gdp_gcl_read_async(gcl, recno, cbfunc, cbarg)						\
				gdp_gin_read_by_recno_async(gcl, recno, 1, cbfunc, cbarg)
#define gdp_gclmd_print(gmd, fp, detail)									\
				gdp_md_dump(gmd, fp, detail, 0)

/*
**  Functions that can't be made 100% compatible
*/

// timeout argument is now a qos argument
#define gdp_gcl_subscribe(gcl, start, nrecs, timeout, cbfunc, cbarg)		\
				gdp_gin_subscribe_by_recno(gcl, start, nrecs, NULL, cbfunc, cbarg)
// timeout argument is now a qos argument
#define gdp_gcl_subscribe_ts(gcl, start, nrecs, timeout, cbfunc, cbarg)		\
				gdp_gin_subscribe_by_rs(gcl, start, nrecs, NULL, cbfunc, cbarg)


#endif // _GDP_COMPAT_V0_H_
