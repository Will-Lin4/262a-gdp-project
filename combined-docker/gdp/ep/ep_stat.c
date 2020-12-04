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
**  EP_STAT handling
**
***********************************************************************/

#include <ep.h>
#include <ep_app.h>
#include <ep_stat.h>
#include <ep_pcvt.h>
#include <ep_dbg.h>
#include <ep_registry.h>
#include <ep_hash.h>
#include <ep_thr.h>
#include <ep_string.h>


/***********************************************************************
**
**  EP_STAT_FROM_ERRNO -- create a status encoding errno
**
**	Assumes errnos are positive integers < 2^10 (the size of
**		the detail part of a status code).
**	Should probably turn some errnos into temporary failures.
**
**	Parameters:
**		uerrno -- the UNIX errno code
**
**	Returns:
**		An appropriately encoded status code
*/

EP_STAT
ep_stat_from_errno(int uerrno)
{
	if (uerrno == 0)
		return EP_STAT_OK;
	return EP_STAT_NEW(EP_STAT_SEV_ERROR, EP_REGISTRY_EPLIB,
			EP_STAT_MOD_ERRNO, (uint32_t) uerrno);
}


/***********************************************************************
**
**  EP_STAT_REG_STRINGS -- register status code strings (for printing)
*/

static EP_HASH	*EpStatStrings;

void
ep_stat_reg_strings(struct ep_stat_to_string *r)
{
	if (EpStatStrings == NULL)
	{
		EpStatStrings = ep_hash_new("EP_STAT to string", NULL, 173);
		if (EpStatStrings == NULL)
			return;
	}

	while (r->estr != NULL)
	{
		(void) ep_hash_insert(EpStatStrings,
			sizeof r->estat, &r->estat, (const void *) r->estr);
		r++;
	}
}



/***********************************************************************
**
**  EP_STAT_TOSTR -- return printable version of a status code
**
**	Currently has a few registries compiled in; these should be
**	driving in some other (extensible) way.
**
**	Parameters:
**		stat -- the status to convert
**		blen -- length of the output buffer
**		buf -- output buffer
**
**	Returns:
**		A string representation of the status.
**		(Will always be "buf")
*/

static const char	*GenericErrors[] =
{
	"generic error",		// 0
	"out of memory",		// 1
	"arg out of range",		// 2
	"end of file",			// 3
};

char *
ep_stat_tostr(EP_STAT stat,
		char *buf,
		size_t blen)
{
	int reg = EP_STAT_REGISTRY(stat);
	const char *pfx;
	const char *detail = NULL;
	const char *module = NULL;
	const char *rname;
	char rbuf[50];
	char mbuf[30];

	// dispose of OK status quickly
	if (EP_STAT_ISOK(stat))
	{
		if (EP_STAT_TO_INT(stat) == 0)
			snprintf(buf, blen, "OK");
		else
			snprintf(buf, blen, "OK [%d = 0x%x]",
				EP_STAT_TO_INT(stat),
				EP_STAT_TO_INT(stat));
		return buf;
	}

	// @@@ this really needs to be made configurable somehow
	switch (reg)
	{
	  case 0:
		rname = "GENERIC";
		break;

	  case EP_REGISTRY_NEOPHILIC:
		rname = "Neophilic";
		break;

	  case EP_REGISTRY_UCB:
		rname = "Berkeley";
		break;

	  case EP_REGISTRY_EPLIB:
		rname = "EPLIB";
		switch (EP_STAT_MODULE(stat))
		{
		  case EP_STAT_MOD_ERRNO:
			module = "errno";
			(void) (0 == strerror_r(EP_STAT_DETAIL(stat),
					rbuf, sizeof rbuf));
			detail = rbuf;
			break;

		  case EP_STAT_MOD_GENERIC:
			module = "generic";
			if (EP_STAT_DETAIL(stat) <
			    (sizeof GenericErrors / sizeof *GenericErrors))
				detail = GenericErrors[EP_STAT_DETAIL(stat)];
			break;
		}
		break;

	  case EP_REGISTRY_USER:
		rname = ep_app_getprogname();
		break;

	  default:
		if (reg <= 0x07f)
			pfx = "LOCAL";
		else if (reg <= 0x0ff)
			pfx = "ENTERPRISE";
		else if (reg <= 0x6ff)
			pfx = "VND";
		else
			pfx = "RESERVED";
		snprintf(rbuf, sizeof rbuf, "%s-%d", pfx, reg);
		rname = rbuf;
		break;
	}

	// check to see if there is a string already
	if (module == NULL && EpStatStrings != NULL && !EP_STAT_ISOK(stat))
	{
		EP_STAT xstat;

		xstat = EP_STAT_NEW(EP_STAT_SEV_OK,
				EP_STAT_REGISTRY(stat),
				EP_STAT_MODULE(stat),
				0);
		module = (const char *) ep_hash_search(EpStatStrings,
						sizeof xstat, &xstat);
	}
	if (module == NULL)
	{
		snprintf(mbuf, sizeof mbuf, "%d", EP_STAT_MODULE(stat));
		module = mbuf;
	}

	// check for detail in hash table
	if (EpStatStrings != NULL)
	{
		const char *s;

		s = (const char *) ep_hash_search(
					EpStatStrings, sizeof stat, &stat);
		if (s != NULL)
			detail = s;
	}

	if (detail != NULL)
	{
		snprintf(buf, blen, "%s: %s [%s:%s:%d]",
				ep_stat_sev_tostr(EP_STAT_SEVERITY(stat)),
				detail,
				rname,
				module,
				EP_STAT_DETAIL(stat));
	}
	else
	{
		snprintf(buf, blen, "%s: [%s:%s:%d]",
				ep_stat_sev_tostr(EP_STAT_SEVERITY(stat)),
				rname,
				module,
				EP_STAT_DETAIL(stat));
	}
	return buf;
}


/*
**  Same, but gives more detail on "OK" and only shows text.
*/

char *
ep_stat_tostr_terse(EP_STAT stat,
		char *buf,
		size_t blen)
{
	if (EpStatStrings != NULL)
	{
		const char *s = (const char *) ep_hash_search(
					EpStatStrings, sizeof stat, &stat);
		if (s != NULL)
		{
			strlcpy(buf, s, blen);
			return buf;
		}
	}

	return ep_stat_tostr(stat, buf, blen);
}


/***********************************************************************
**
**  EP_STAT_SEV_TOSTR -- return printable version of a status severity
**
**	Parameters:
**		sev -- the severity to convert
**
**	Returns:
**		A string representation of the severity.
**
**	To Do:
**		Should be internationalized.
*/

const char *
ep_stat_sev_tostr(int sev)
{
	const char *const sevnames[8] =
	{
		"OK",			// 0
		"OK",			// 1
		"OK",			// 2
		"OK",			// 3
		"WARNING",		// 4
		"ERROR",		// 5
		"SEVERE",		// 6
		"ABORT",		// 7
	};

	return sevnames[sev & 0x07];
}
