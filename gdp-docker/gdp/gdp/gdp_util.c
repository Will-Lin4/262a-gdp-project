/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**	GDP Utility routines
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
#include "gdp_md.h"
#include "gdp_priv.h"
#include "gdp_version.h"

#include <event2/event.h>

#include <ep/ep_crypto.h>
#include <ep/ep_dbg.h>
#include <ep/ep_string.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.util", "GDP utility routines");


/*
**   _GDP_NEWNAME --- create a new GDP name
**
**		Really just creates a random number (for now).
*/

void
_gdp_newname(gdp_name_t gname, gdp_md_t *gmd)
{
	if (gmd == NULL)
	{
		// last resort: use random bytes
		evutil_secure_rng_get_bytes(gname, sizeof (gdp_name_t));
	}
	else
	{
		uint8_t *mdbuf;
		size_t mdlen = _gdp_md_serialize(gmd, &mdbuf);

		ep_crypto_md_sha256(mdbuf, mdlen, gname);
		ep_mem_free(mdbuf);
	}
}


/*
**  _GDP_PR_INDENT --- output indent string for debug output
*/

#define INDENT_MAX		6			// max depth of indentation
#define INDENT_PER		4			// spaces per indent level

const char *
_gdp_pr_indent(int indent)
{
	static char spaces[INDENT_MAX * INDENT_PER + 1];

	// initialize the array with just spaces
	if (spaces[0] == 0)
		snprintf(spaces, sizeof spaces, "%*s", INDENT_MAX * INDENT_PER, "");

	if (indent < 0)
		indent = 0;
	if (indent > INDENT_MAX)
		indent = INDENT_MAX;
	return &spaces[(INDENT_MAX - indent) * INDENT_PER];
}


/*
**  _GDP_ADM_READPARAMS --- like ep_adm_readparams with versioning
**
**		For a given NAME, tries param files named
**			NAME
**			NAME$GDP_VERSION_MAJOR
**			NAME$GDP_VERSION_MAJOR.$GDP_VERSION_MINOR
**		Later values overwrite earlier values.
*/

void
_gdp_adm_readparams(const char *name)
{
	int nlen = strlen(name);
	char nbuf[nlen + 20];

	// if program name already has major version, strip it off
	// (this is primarily to simplify gdplogd2 special case)
	snprintf(nbuf, sizeof nbuf, "%d", GDP_VERSION_MAJOR);
	if (strcmp(nbuf, &name[nlen - strlen(nbuf)]) == 0)
		nlen -= strlen(nbuf);

	// try name, name<MAJOR>, name<MAJOR>.<MINOR>
	snprintf(nbuf, sizeof nbuf, "%.*s", nlen, name);
	ep_adm_readparams(nbuf);
	snprintf(nbuf, sizeof nbuf, "%.*s%d", nlen, name,
				GDP_VERSION_MAJOR);
	ep_adm_readparams(nbuf);
	snprintf(nbuf, sizeof nbuf, "%.*s%d.%d", nlen, name,
				GDP_VERSION_MAJOR, GDP_VERSION_MINOR);
	ep_adm_readparams(nbuf);
}


/*
**  _GDP_ADM_PATH_FIND --- find a parametrically-defined path
**
**		This indirects through the ep_adm_*param interface to find
**		path names.
**
**		This _might_ be generic enough to move into libep.
*/

EP_STAT
_gdp_adm_path_find(
			const char *dir_param,
			const char *dir_def,
			const char *file_param,
			const char *file_def,
			char *path_buf,
			size_t path_buf_len)
{
	const char *dir;
	const char *file;

	if (file_param == NULL)
		file = file_def;
	else
		file = ep_adm_getstrparam(file_param, file_def);
	if (file == NULL)
		file = "";
	else if (file[0] == '/')
	{
		snprintf(path_buf, path_buf_len, "%s", file);
		return EP_STAT_OK;
	}

	if (dir_param == NULL)
		dir = dir_def;
	else
		dir = ep_adm_getstrparam(dir_param, dir_def);
	if (dir == NULL)
		dir = "";

	snprintf(path_buf, path_buf_len, "%s/%s", dir, file);
	return EP_STAT_OK;
}


/*
**  _GDP_SHOW_ELAPSED --- show amount of wall clock time spent in a function
*/

void
_gdp_show_elapsed(const char *func, gdp_cmd_t cmd, EP_TIME_SPEC *start)
{
	if (ep_dbg_test(Dbg, 11))
	{
		char tbuf[30];
		EP_TIME_SPEC end;

		ep_time_now(&end);
		ep_time_format_interval(start, &end, tbuf, sizeof tbuf);
		ep_dbg_printf("=== %s", func);
		if (cmd != 0)
			ep_dbg_printf("(%s)", _gdp_proto_cmd_name(cmd));
		ep_dbg_printf(" elapsed: %s\n", tbuf);
	}
}
