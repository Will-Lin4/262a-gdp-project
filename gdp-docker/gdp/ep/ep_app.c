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
#include <ep_app.h>
#include <ep_log.h>
#include <ep_string.h>

#include <stdlib.h>
#include <sys/errno.h>


static uint32_t	OperationFlags = EP_APP_FLAG_LOG_ABORT;


////////////////////////////////////////////////////////////////////////
//
//  Application support
//
//	Just to make life easier for app writers
//
////////////////////////////////////////////////////////////////////////

const char *
ep_app_getprogname(void)
{
#if EP_OSCF_HAS_GETPROGNAME
	return getprogname();
#elif __linux__
	extern char *program_invocation_short_name;
	return program_invocation_short_name;
#else
	return NULL;
# endif
}


struct msginfo
{
	const char	*tag;		// message tag (e.g., "ERROR")
	const char	*colors;	// colors (foreground, background)
	uint32_t	flag;		// flag to test for logging
};

#define MSGINFO(tag, colors, logflag)	\
		{ tag, colors, EP_APP_FLAG_LOG_ ## logflag }

static struct msginfo	MsgInfo[10] =
{
	MSGINFO("OK",		"gk",		OK	),
	MSGINFO("OK",		"gk",		OK	),
	MSGINFO("OK",		"gk",		OK	),
	MSGINFO("OK",		"gk",		OK	),
	MSGINFO("WARN",		"yk",		WARNING	),
	MSGINFO("ERROR",	"ry",		ERROR	),
	MSGINFO("SEVERE",	"yr",		SEVERE	),
	MSGINFO("ABORT",	"wr",		ABORT	),
	MSGINFO("FATAL",	"wr",		FATAL	),
	MSGINFO("INFO",		"ck",		INFO	),
};

#define EP_STAT_SEV_FATAL	8	// pseudo-value
#define EP_STAT_SEV_INFO	9	// pseudo-value



////////////////////////////////////////////////////////////////////////
//
//  PRINTMESSAGE -- helper routine for message printing
//

static void
printmessage(struct msginfo *mi,
		const char *emsg,
		const char *fmt,
		va_list av)
{
	const char *progname;
	const char *fg;
	const char *bg;

	switch (mi->colors[0])
	{
	  case 'b': fg = EpVid->vidfgblue;	break;
	  case 'c': fg = EpVid->vidfgcyan;	break;
	  case 'g': fg = EpVid->vidfggreen;	break;
	  case 'k': fg = EpVid->vidfgblack;	break;
	  case 'm': fg = EpVid->vidfgmagenta;	break;
	  case 'r': fg = EpVid->vidfgred;	break;
	  case 'w': fg = EpVid->vidfgwhite;	break;
	  case 'y': fg = EpVid->vidfgyellow;	break;
	  default:  fg = NULL;			break;
	}
	switch (mi->colors[1])
	{
	  case 'b': bg = EpVid->vidbgblue;	break;
	  case 'c': bg = EpVid->vidbgcyan;	break;
	  case 'g': bg = EpVid->vidbggreen;	break;
	  case 'k': bg = EpVid->vidbgblack;	break;
	  case 'm': bg = EpVid->vidbgmagenta;	break;
	  case 'r': bg = EpVid->vidbgred;	break;
	  case 'w': bg = EpVid->vidbgwhite;	break;
	  case 'y': bg = EpVid->vidbgyellow;	break;
	  default:  bg = NULL;			break;
	}

	if (fg == NULL)
		fg = "";
	if (bg == NULL)
		bg = "";

	flockfile(stderr);
	fprintf(stderr, "[%s%s%s%s] ",
			fg, bg, mi->tag, EpVid->vidnorm);
	if ((progname = ep_app_getprogname()) != NULL)
		fprintf(stderr, "%s: ", progname);
	if (fmt != NULL)
		vfprintf(stderr, fmt, av);
	else
		fprintf(stderr, "unknown %s", mi->tag);
	if (emsg != NULL)
		fprintf(stderr, ":\n\t%s", emsg);
	if (errno != 0)
	{
		char nbuf[40];

		(void) (0 == strerror_r(errno, nbuf, sizeof nbuf));
		fprintf(stderr, "\n\t(%s)", nbuf);
	}
	fprintf(stderr, "\n");
	funlockfile(stderr);
}


////////////////////////////////////////////////////////////////////////
//
//  EP_APP_MESSAGE -- print a message (severity from EP_STAT)
//
//	Parameters:
//		estat -- status code (printed)
//		fmt -- format for a message
//		... -- arguments
//

void
ep_app_messagev(
	EP_STAT estat,
	const char *fmt,
	va_list av)
{
	struct msginfo *mi = &MsgInfo[EP_STAT_SEVERITY(estat)];
	char ebuf[100];
	va_list av2;

	ep_stat_tostr(estat, ebuf, sizeof ebuf);
	errno = 0;
	if (EP_UT_BITSET(mi->flag, OperationFlags))
		va_copy(av2, av);
	printmessage(mi, ebuf, fmt, av);
	if (EP_UT_BITSET(mi->flag, OperationFlags))
	{
		ep_logv(estat, fmt, av2);
		va_end(av2);
	}
}

void
ep_app_message(
	EP_STAT estat,
	const char *fmt,
	...)
{
	va_list av;

	errno = 0;
	va_start(av, fmt);
	ep_app_messagev(estat, fmt, av);
	va_end(av);
}


////////////////////////////////////////////////////////////////////////
//
//  EP_APP_INFO -- print an informational message
//
//	Parameters:
//		fmt -- format for a message
//		... -- arguments
//

void
ep_app_info(
	const char *fmt,
	...)
{
	va_list av;

	errno = 0;
	va_start(av, fmt);
	printmessage(&MsgInfo[EP_STAT_SEV_INFO], NULL, fmt, av);
	va_end(av);

	if (EP_UT_BITSET(EP_APP_FLAG_LOG_INFO, OperationFlags))
	{
		va_start(av, fmt);
		ep_logv(EP_STAT_OK, fmt, av);
		va_end(av);
	}
}


////////////////////////////////////////////////////////////////////////
//
//  EP_APP_WARN -- print a warning message
//
//	Parameters:
//		fmt -- format for a message
//		... -- arguments
//
//	Returns:
//		never
//

void
ep_app_warn(
	const char *fmt,
	...)
{
	va_list av;

	va_start(av, fmt);
	printmessage(&MsgInfo[EP_STAT_SEV_WARN], NULL, fmt, av);
	va_end(av);

	if (EP_UT_BITSET(EP_APP_FLAG_LOG_WARNING, OperationFlags))
	{
		va_start(av, fmt);
		ep_logv(EP_STAT_WARN, fmt, av);
		va_end(av);
	}
}


////////////////////////////////////////////////////////////////////////
//
//  EP_APP_ERROR -- print an error message
//
//	Parameters:
//		fmt -- format for a message
//			If NULL it prints errno.
//		... -- arguments
//
//	Returns:
//		never
//

void
ep_app_error(
	const char *fmt,
	...)
{
	va_list av;

	va_start(av, fmt);
	printmessage(&MsgInfo[EP_STAT_SEV_ERROR], NULL, fmt, av);
	va_end(av);

	if (EP_UT_BITSET(EP_APP_FLAG_LOG_ERROR, OperationFlags))
	{
		va_start(av, fmt);
		ep_logv(EP_STAT_ERROR, fmt, av);
		va_end(av);
	}
}


////////////////////////////////////////////////////////////////////////
//
//  EP_APP_SEVERE -- print a severe message
//
//	Parameters:
//		fmt -- format for a message
//			If NULL it prints errno.
//		... -- arguments
//
//	Returns:
//		never
//

void
ep_app_severe(
	const char *fmt,
	...)
{
	va_list av;

	va_start(av, fmt);
	printmessage(&MsgInfo[EP_STAT_SEV_SEVERE], NULL, fmt, av);
	va_end(av);

	if (EP_UT_BITSET(EP_APP_FLAG_LOG_SEVERE, OperationFlags))
	{
		va_start(av, fmt);
		ep_logv(EP_STAT_SEVERE, fmt, av);
		va_end(av);
	}
}


////////////////////////////////////////////////////////////////////////
//
//  EP_APP_FATAL -- print a fatal error message and exit
//
//	Just uses a generic exit status.
//
//	Parameters:
//		fmt -- format for a message
//		... -- arguments
//
//	Returns:
//		never
//

void
ep_app_fatal(
	const char *fmt,
	...)
{
	va_list av;

	va_start(av, fmt);
	printmessage(&MsgInfo[EP_STAT_SEV_FATAL], NULL, fmt, av);
	va_end(av);

	if (EP_UT_BITSET(EP_APP_FLAG_LOG_FATAL, OperationFlags))
	{
		va_start(av, fmt);
		ep_logv(EP_STAT_SEVERE, fmt, av);
		va_end(av);
	}

	fprintf(stderr, "\t(exiting)\n");
	exit(1);
	/*NOTREACHED*/
}


////////////////////////////////////////////////////////////////////////
//
//  EP_APP_ABORT -- print an error message and abort
//
//	Parameters:
//		fmt -- format for a message
//		... -- arguments
//
//	Returns:
//		never
//

void
ep_app_abort(
	const char *fmt,
	...)
{
	va_list av;

	va_start(av, fmt);
	printmessage(&MsgInfo[EP_STAT_SEV_ABORT], NULL, fmt, av);
	va_end(av);

	if (EP_UT_BITSET(EP_APP_FLAG_LOG_ABORT, OperationFlags))
	{
		va_start(av, fmt);
		ep_logv(EP_STAT_ABORT, fmt, av);
		va_end(av);
	}

	fprintf(stderr, "\n\t(exiting)\n");
	abort();
	/*NOTREACHED*/
}
