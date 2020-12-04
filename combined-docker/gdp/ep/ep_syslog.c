/* vim: set ai sw=8 sts=8 ts=8 : */

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
#include <ep_syslog.h>

#include <strings.h>

struct code
{
	const char	*c_name;
	const int	c_val;
};

static struct code	PriNames[] =
{
	{ "alert",	LOG_ALERT	},
	{ "crit",	LOG_CRIT	},
	{ "debug",	LOG_DEBUG	},
	{ "emerg",	LOG_EMERG	},
	{ "err",	LOG_ERR		},
	{ "error",	LOG_ERR		},		/* DEPRECATED */
	{ "info",	LOG_INFO	},
	{ "notice",	LOG_NOTICE	},
	{ "panic",	LOG_EMERG	},		/* DEPRECATED */
	{ "warn",	LOG_WARNING	},		/* DEPRECATED */
	{ "warning",	LOG_WARNING	},
	{ NULL,		-1		}
};

struct code     FacNames[] =
{
	{ "auth",	LOG_AUTH	},
#ifdef LOG_AUTHPRIV
	{ "authpriv",	LOG_AUTHPRIV	},
#endif
#ifdef LOG_CONSOLE
	{ "console",	LOG_CONSOLE	},
#endif
	{ "cron",	LOG_CRON	},
	{ "daemon",	LOG_DAEMON	},
#ifdef LOG_FTP
	{ "ftp",	LOG_FTP		},
#endif
#ifdef LOG_INSTALL
	{ "install",	LOG_INSTALL	},		// MacOS
#endif
	{ "kern",	LOG_KERN	},
#ifdef LOG_LAUNCHD
	{ "launchd",	LOG_LAUNCHD	},		// MacOS
#endif
	{ "lpr",	LOG_LPR		},
	{ "mail",	LOG_MAIL	},
#ifdef LOG_NETINFO
	{ "netinfo",	LOG_NETINFO	},		// MacOS
#endif
	{ "news",	LOG_NEWS	},
#ifdef LOG_NTP
	{ "ntp",	LOG_NTP		},
#endif
#ifdef LOG_RAS
	{ "ras",	LOG_RAS		},		// MacOS
#endif
#ifdef LOG_REMOTEAUTH
	{ "remoteauth",	LOG_REMOTEAUTH	},		// MacOS
#endif
	{ "security",	LOG_AUTH	},		/* DEPRECATED */
	{ "syslog",	LOG_SYSLOG	},
	{ "user",	LOG_USER	},
	{ "uucp",	LOG_UUCP	},
	{ "local0",	LOG_LOCAL0	},
	{ "local1",	LOG_LOCAL1	},
	{ "local2",	LOG_LOCAL2	},
	{ "local3",	LOG_LOCAL3	},
	{ "local4",	LOG_LOCAL4	},
	{ "local5",	LOG_LOCAL5	},
	{ "local6",	LOG_LOCAL6	},
	{ "local7",	LOG_LOCAL7	},
	{ NULL,		-1		}
};


/*
**  EP_SYSLOG_PRI_FROM_NAME --- translate string name to log priority
*/

int
ep_syslog_pri_from_name(const char *name)
{
	struct code *c;

	for (c = PriNames; c->c_name != NULL; c++)
	{
		if (strcasecmp(c->c_name, name) == 0)
			return c->c_val;
	}
	return -1;
}



const char *
ep_syslog_name_from_pri(int pri)
{
	struct code *c;

	for (c = PriNames; c->c_name != NULL; c++)
	{
		if (c->c_val == pri)
			return c->c_name;
	}
	return "unknown";
}



int
ep_syslog_fac_from_name(const char *name)
{
	struct code *c;

	for (c = FacNames; c->c_name != NULL; c++)
	{
		if (strcasecmp(c->c_name, name) == 0)
			return c->c_val;
	}
	return -1;
}



const char *
ep_syslog_name_from_fac(int fac)
{
	struct code *c;

	for (c = FacNames; c->c_name != NULL; c++)
	{
		if (c->c_val == fac)
			return c->c_name;
	}
	return "unknown";
}
