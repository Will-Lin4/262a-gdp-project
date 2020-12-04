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
#include <ep_file_search.h>

#include <string.h>
#include <sys/stat.h>

/*
**  EP_FILE_SEARCH --- locate a file or directory in a search path
**
**	If the file name ends with a slash, search for a directory,
**	otherwise search for a non-directory.
**
**	Should probably allow "~" expansion (not for root!).
*/

EP_STAT
ep_file_search(
	const char *search_path,	// directory path to search
	const char *fname,		// file or dir in that path
	size_t fnbuf_len,		// size of output buffer
	char *fnbuf)			// output buffer
{
	const char *dp;			// directory name pointer
	const char *ep;			// end of directory name pointer
	size_t flen = 0;
	bool find_dir = true;		// assume fname == NULL

	if (fname != NULL)
	{
		// if fname is an absolute path, just use it
		//XXX should really do directory check
		if (fname[0] == '/')
		{
			snprintf(fnbuf, fnbuf_len, "%s", fname);
			return EP_STAT_OK;
		}

		// not absolute: let's search
		flen = strlen(fname);
		// if fname ends in slash, we are looking for a directory
		find_dir = fname[flen - 1] == '/';
		if (find_dir)
			flen--;
	}
	if (flen != 0)
		flen++;			// include space for `/`
	for (dp = search_path; dp != NULL; dp = ep)
	{
		size_t dlen;

		ep = strchr(dp, ':');
		if (ep == NULL)
			dlen = strlen(dp);
		else
			dlen = ep++ - dp;
		if (dlen == 0)
		{
			// handle the "::" or ":path" or "path:" case
			dp = ".";
			dlen++;
		}

		if (flen > 0)
			snprintf(fnbuf, fnbuf_len, "%.*s/%.*s",
					(int) dlen, dp, (int) flen, fname);
		else
			snprintf(fnbuf, fnbuf_len, "%.*s",
					(int) dlen, dp);
		struct stat st;
		if (stat(fnbuf, &st) != 0)
			continue;		// not found
		if (find_dir			// see if compatible type
			? ((st.st_mode & S_IFMT) == S_IFDIR)
			: ((st.st_mode & S_IFMT) != S_IFDIR))
		{
			// found it
			return EP_STAT_OK;
		}
	}

	// not found
	return EP_STAT_NOT_FOUND;
}
