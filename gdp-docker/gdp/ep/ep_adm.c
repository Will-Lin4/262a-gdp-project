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
**  Administration routines
**
**	The defaults database (global parameters database?  surely
**	a better name can be found) stored various tunable
**	parameters.  The namespace is just strings, structured as
**	system.subsystem...parameter (i.e., it can be a variable
**	number of components, kind of like a MIB).
**
**	My thought is to use this something like the X Resources
**	database -- i.e., there is a search path.  For now, they
**	always return exact matches only.  They should also allow
**	multiple data files so that different subsystems can
**	easily include their own defaults.
**
**	This may be a candidate for osdep implementation -- e.g.,
**	using the registry on Windows-based systems.
**
***********************************************************************/

#include <ep.h>
#include <ep_dbg.h>
#include <ep_hash.h>
#include <ep_string.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/stat.h>

// allow environment to override admin parameters
//	There are probably security considerations here, but we try
//	to limit checks for variables that might be in the local
//	ep_adm_params directory.  Should probably be turned off for
//	production.
#ifndef EP_CONF_ADM_ENV_OVERRIDE
# define EP_CONF_ADM_ENV_OVERRIDE	1
#endif

static EP_DBG	Dbg = EP_DBG_INIT("libep.dbg", "Administrative routines");
static EP_HASH	*ParamHash;
static const char	*ParamFile;	// not thread safe


/*
**  READ_PARAM_FILE --- parses a single file based on absolute path name
*/

static void
read_param_file(char *path)
{
	FILE *fp = NULL;
	char lbuf[200];

	ep_dbg_cprintf(Dbg, 34, "read_param_file(%s)\n", path);
	if (path == NULL || (fp = fopen(path, "r")) == NULL)
	{
		errno = 0;		// avoid misleading error messages
		return;
	}

	ParamFile = path;
	while (fgets(lbuf, sizeof lbuf, fp) != NULL)
	{
		if (lbuf[0] == '#')
			continue;	// comment
		ep_adm_parseline(lbuf);
	}
	ParamFile = NULL;
	fclose(fp);
}


/*
**  EP_ADM_PARSELINE --- parse one line from a parameter file.
**	Can also be called by user programs.
*/

void
ep_adm_parseline(char *lbuf)
{
	char *p = lbuf;
	char *np;		// pointer to name
	char *vp;		// pointer to value

	p += strspn(p, " \t\n");	// trim leading wsp
	np = p;
	p += strcspn(p, " \t\n=");	// find end of name
	if (*p == '\0')
		return;			// syntax: no value
	else if (*p == '=')
		*p++ = '\0';	// no wsp at end of name
	else
	{
		// trim white space from end of name
		*p++ = '\0';
		p += strspn(p, " \t\n");
		if (*p++ != '=')
			return;		// syntax: bad name
	}
	p += strspn(p, " \t\n");	// skip wsp before val
	vp = p;
	p += strcspn(p, "\n");
	*p = '\0';
	ep_adm_setparam(np, vp);
}


/*
**  EP_ADM_SETPARAM --- set a single parameter.
**	Useful for having command line flags override parameters.
**	Note: must be called after the parameter files are read.
*/

void
ep_adm_setparam(const char *np, const char *vp)
{
	char *op;

	ep_dbg_cprintf(Dbg, 8, "param %s=%s (%s)\n",
				np, vp,
				ParamFile == NULL ? "internal" : ParamFile);
	op = (char *) ep_hash_insert(ParamHash, strlen(np), np, strdup(vp));
	if (op != NULL)
		ep_mem_free(op);
}


/*
**  GET_PARAM_PATH --- create the search path
**
**	Instances of "~" are converted to refer to the home directory.
**	Searches for directories named ".?gdp/params" even though that's
**		a layering violation (libep is not libgdp-specific).
**	But also searches ep_adm_params for historic reasons.
*/

static char *
get_param_path(void)
{
	static char *path = NULL;
	static char pathbuf[200] = "";

	if (path != NULL)
		return path;

	// initialize the hash table
	ParamHash = ep_hash_new("ep_adm hash", NULL, 97);

	if (getuid() == geteuid())
		path = getenv("EP_PARAM_PATH");
	if (path == NULL)
	{
		if (getuid() == geteuid())
		{
			strlcpy(pathbuf, ".gdp/params:.ep_adm_params:",
					sizeof pathbuf);
			path = getenv("HOME");
			if (path != NULL)
			{
				strlcat(pathbuf, path, sizeof pathbuf);
				strlcat(pathbuf, "/.gdp/params:", sizeof pathbuf);
				strlcat(pathbuf, path, sizeof pathbuf);
				strlcat(pathbuf, "/.ep_adm_params:", sizeof pathbuf);
			}
		}
		strlcat(pathbuf,
			"/usr/local/etc/gdp/params"
			":/etc/gdp/params"
			":/usr/local/etc/ep_adm_params"
			":/etc/ep_adm_params",
			sizeof pathbuf);
	}
	else
	{
		int i = sizeof pathbuf;
		char *p = pathbuf;

		while (*path != '\0' && --i > 0)
		{
			if (*path == '~')
			{
				const char *home = getenv("HOME");
				int j;

				if (home == NULL)
					home = ".";
				j = strlen(home);
				strlcpy(p, home, j);
				p += j;
				i -= j;
				path++;
			}
			else
			{
				*p++ = *path++;
			}
		}
		*p = '\0';
	}
	path = pathbuf;
	return path;
}


/*
**  EP_ADM_READPARAMS --- read one or more files based on a search path
**
**	Actually reads them in reverse order so that the first ones
**	override.
*/

void
ep_adm_readparams(const char *name)
{
	char fnbuf[200];
	char pbuf[400];
	char *path = get_param_path();
	char *p;

	if (name == NULL)
		return;

	strlcpy(pbuf, get_param_path(), sizeof pbuf);
	while ((p = strrchr(pbuf, ':')) != NULL)
	{
		*p++ = '\0';
		if (*p != '\0')
		{
			snprintf(fnbuf, sizeof fnbuf, "%s/%s", p, name);
			read_param_file(fnbuf);
		}
	}
	if (*path != '\0')
	{
		snprintf(fnbuf, sizeof fnbuf, "%s/%s", pbuf, name);
		read_param_file(fnbuf);
	}
}


static const char *
getparamval(
	const char *pname)
{
	static bool recurse = false;
	extern bool _EpLibInitialized;

#if EP_CONF_ADM_ENV_OVERRIDE
	// allow environment to override admin parameters
	if (getuid() == geteuid() && getuid() != 0)
	{
		int l = strlen(pname) + 1;
		char evname[l];
		strlcpy(evname, pname, l);
		char *p = evname;
		while ((p = strchr(p, '.')) != NULL)
			*p = '_';
		const char *val = getenv(evname);
		if (val != NULL)
			return val;
	}
#endif

	// don't allow recursive calls or use before initialization
	if (recurse | !_EpLibInitialized)
		return NULL;

	// if we have no data, read it in
	if (ParamHash == NULL)
	{
		recurse = true;
		ep_adm_readparams("defaults");
		recurse = false;
	}

	return (const char *) ep_hash_search(ParamHash, strlen(pname), pname);
}



/***********************************************************************
**
**  EP_ADM_GETINTPARAM -- get integer parameter from defaults database
**
**	Parameters:
**		pname -- the name of the parameter
**		def -- the default value if not specified in the
**			database
**
**	Returns:
**		The value of the parameter or def
*/

int
ep_adm_getintparam(
	const char *pname,
	int def)
{
	const char *p = getparamval(pname);

	if (p == NULL)
		return def;
	else
		return (int) strtol(p, NULL, 0);
}


/***********************************************************************
**
**  EP_ADM_GETLONGPARAM -- get long int parameter from defaults database
**
**	Parameters:
**		pname -- the name of the parameter
**		def -- the default value if not specified in the
**			database
**
**	Returns:
**		The value of the parameter or def
*/

long
ep_adm_getlongparam(
	const char *pname,
	long def)
{
	const char *p = getparamval(pname);

	if (p == NULL)
		return def;
	else
		return strtol(p, NULL, 0);
}


/***********************************************************************
**
**  EP_ADM_GETINTMAXPARAM -- get long long parameter from defaults database
**
**	Note that according to the C spec, intmax_t might be larger than
**	long long.  Or maybe not.
**
**	Parameters:
**		pname -- the name of the parameter
**		def -- the default value if not specified in the
**			database
**
**	Returns:
**		The value of the parameter or def
*/

intmax_t
ep_adm_getintmaxparam(
	const char *pname,
	intmax_t def)
{
	const char *p = getparamval(pname);

	if (p == NULL)
		return def;
	else
		return strtoimax(p, NULL, 0);
}


/***********************************************************************
**
**  EP_ADM_GETBOOLPARAM -- get Boolean parameter from defaults database
**
**	Parameters:
**		pname -- the name of the parameter
**		def -- the default value if not specified in the
**			database
**
**	Returns:
**		The value of the parameter or def
*/

bool
ep_adm_getboolparam(
	const char *pname,
	bool def)
{
	const char *p = getparamval(pname);

	if (p == NULL)
		return def;
	if (strchr("1tTyY", *p) != NULL)
		return true;
	else
		return false;
}


/***********************************************************************
**
**  EP_ADM_GETSTRPARAM -- get string parameter from defaults database
**
**	Parameters:
**		pname -- the name of the parameter
**		def -- the default value if not specified in the
**			database
**
**	Returns:
**		The value of the parameter or def
*/

const char *
ep_adm_getstrparam(
	const char *pname,
	const char *def)
{
	const char *p = getparamval(pname);

	if (p == NULL)
		return def;
	else
		return p;
}
