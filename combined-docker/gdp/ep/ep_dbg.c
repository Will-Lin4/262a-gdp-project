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
#include <ep_dbg.h>
#include <ep_string.h>
#include <ep_assert.h>

#include <ctype.h>
#include <fnmatch.h>
#include <pthread.h>


typedef struct FLAGPAT	FLAGPAT;

struct FLAGPAT
{
	const char	*pat;		// flag pattern (text)
	int		lev;		// level to set the pattern
	FLAGPAT		*next;		// next in chain
};

/*
**  This intentionally uses pthread primitives instead of ep_thr_*
**  to avoid recursive locks when calling ep_dbg_test.
*/

#if EP_OSCF_USE_PTHREADS
extern bool	_EpThrUsePthreads;
static pthread_rwlock_t	FlagListRwlock	= PTHREAD_RWLOCK_INITIALIZER;
#endif
static FLAGPAT	*FlagList;
FILE		*EpDebugFileP;
int		__EpDbgCurGen;		// current generation number
static bool	FlushDebugFile;		// flush output on each printf?


/*
**  EP_DBG_INIT -- initialize debug subsystem
*/

void
ep_dbg_init(void)
{
	if (EpDebugFileP == NULL)
		ep_dbg_setfile(NULL);
	EP_ASSERT(EpDebugFileP != NULL);
}

/*
**  EP_DBG_SET -- set a debugging flag
**
**	Should probably do more syntax checking here.
*/

void
ep_dbg_set(const char *fspec)
{
	const char *f = fspec;

	ep_dbg_init();

	if (f == NULL)
		return;
	while (*f != '\0')
	{
		unsigned int i = 0;
		char pbuf[200];

		if (strchr(",; ", *f) != NULL)
		{
			f++;
			continue;
		}

		while (*f != '\0' && strchr("=,; ", *f) == NULL)
		{
			if (i <= sizeof pbuf - 1)
				pbuf[i++] = *f;
			f++;
		}
		pbuf[i] = '\0';
		while (*f == ' ')
			f++;
		if (isdigit(pbuf[0]) && *f != '=')
		{
			// -D999, with no "flag=" part
			ep_dbg_setto("*", strtol(pbuf, NULL, 10));
		}
		else if (*f != '=')
		{
			// -Dflag, with no level indicator: default to 1
			ep_dbg_setto(pbuf, 1);
		}
		else
		{
			// -Dflag=999
			ep_dbg_setto(pbuf, strtol(++f, NULL, 10));
		}
		f += strcspn(f, ",;");
	}
}

/*
**  EP_DBG_SETTO -- set a debugging flag to a particular value
*/

void
ep_dbg_setto(const char *fpat,
	int lev)
{
	FLAGPAT *fp;
	int patlen = strlen(fpat) + 1;
	char *tpat;

	fp = (FLAGPAT *) ep_mem_malloc(sizeof *fp + patlen);
	tpat = ((char *) fp) + sizeof *fp;
	(void) strncpy(tpat, fpat, patlen);
	fp->pat = tpat;
	fp->lev = lev;

	// link to front of chain
#if EP_OSCF_USE_PTHREADS
	if (_EpThrUsePthreads)
		pthread_rwlock_wrlock(&FlagListRwlock);
#endif
	fp->next = FlagList;
	FlagList = fp;
	__EpDbgCurGen++;
#if EP_OSCF_USE_PTHREADS
	if (_EpThrUsePthreads)
		pthread_rwlock_unlock(&FlagListRwlock);
#endif
}


/*
**  EP_DBG_FLAGLEVEL -- return flag level, initializing if necessary
**
**	This checks to see if the flag is specified; in any case it
**	updates the local value indicator.
**
**	Parameters:
**		flag -- the flag to be tested
**
**	Returns:
**		The current flag level
*/

int
ep_dbg_flaglevel(EP_DBG *flag)
{
	FLAGPAT *fp;

#if EP_OSCF_USE_PTHREADS
	if (_EpThrUsePthreads)
		pthread_rwlock_rdlock(&FlagListRwlock);
#endif
	flag->gen = __EpDbgCurGen;
	for (fp = FlagList; fp != NULL; fp = fp->next)
	{
		if (fnmatch(fp->pat, flag->name, FNM_PERIOD) == 0)
			break;
	}
	if (fp == NULL)
		flag->level = 0;
	else
		flag->level = fp->lev;
#if EP_OSCF_USE_PTHREADS
	if (_EpThrUsePthreads)
		pthread_rwlock_unlock(&FlagListRwlock);
#endif

	return flag->level;
}


/*
**  EP_DBG_[V]PRINTF -- print debug information
**
**	Parameters:
**		fmt -- the message to print
**		... -- parameters to that fmt
**
**	Returns:
**		none.
*/


void
ep_dbg_vprintf(const char *fmt, va_list av)
{
	if (EpDebugFileP == NULL)
		ep_dbg_init();

	flockfile(EpDebugFileP);
	//fprintf(EpDebugFileP, "%s", EpVid->vidfgyellow);	// OLD
	vfprintf(EpDebugFileP, fmt, av);
	//fprintf(EpDebugFileP, "%s", EpVid->vidnorm);		// OLD
	if (FlushDebugFile)
		fflush(EpDebugFileP);		// XXX ??? performance issues?
	funlockfile(EpDebugFileP);
}


void
ep_dbg_printf(const char *fmt, ...)
{
	va_list av;

	va_start(av, fmt);
	ep_dbg_vprintf(fmt, av);
	va_end(av);
}


/*
**  EP_DBG_SETFILE -- set output file for debugging
**
**	Parameters:
**		fp -- the file to use for debugging; if NULL it
**			uses the default (stderr).
**
**	Returns:
**		none
*/

void
ep_dbg_setfile(
	FILE *fp)
{
	if (fp != NULL && fp == EpDebugFileP)
		return;

	// close the old stream
	if (EpDebugFileP != NULL &&
	    EpDebugFileP != stdout && EpDebugFileP != stderr)
		(void) fclose(EpDebugFileP);

	// if fp is NULL, switch to the default
	if (fp == NULL)
	{
		const char *dfile;

		dfile = ep_adm_getstrparam("libep.dbg.file", "stderr");
		if (strcmp(dfile, "stderr") == 0)
			fp = stderr;
		else if (strcmp(dfile, "stdout") == 0)
			fp = stdout;
		else
		{
			fp = fopen(dfile, "a");
			if (fp == NULL)
			{
				ep_app_warn("Cannot open debug file %s",
						dfile);
				fp = stderr;
			}
		}
		setlinebuf(fp);
	}

	// this looks like the wrong heuristic...
	FlushDebugFile = fp == stderr;
	EpDebugFileP = ep_fopen_styled(fp, EpVid->vidfgyellow, EpVid->vidnorm);
	//EpDebugFileP = fp;		// OLD
}


FILE *
ep_dbg_getfile(void)
{
	if (EpDebugFileP == NULL)
		ep_dbg_init();
	return EpDebugFileP;
}


EP_STAT
ep_cvt_txt_to_debug(
	const char *vp,
	void *rp)
{
	ep_dbg_set(vp);

	return EP_STAT_OK;
}


#if EP_OSCF_HAS_BACKTRACE
#include <execinfo.h>
#endif

#define NFRAMES		128		// maximum stack depth

void
ep_dbg_backtrace(FILE *fp)
{
	if (fp == NULL)
		fp = EpDebugFileP;
#if EP_OSCF_HAS_BACKTRACE
	void *frames[NFRAMES];
	int nframes;
	int fd = fileno(fp);

	if (fd < 0)
		fd = 2;
	nframes = backtrace(frames, NFRAMES);
	backtrace_symbols_fd(frames, nframes, fd);
#else
	fprintf(fp, "No stack backtrace available\n");
#endif
}
