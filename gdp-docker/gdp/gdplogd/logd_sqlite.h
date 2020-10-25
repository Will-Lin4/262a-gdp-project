/* vim: set ai sw=4 sts=4 ts=4 : */

/*
**	----- BEGIN LICENSE BLOCK -----
**	GDPLOGD: Log Daemon for the Global Data Plane
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

#ifndef _GDPLOGD_SQLITE_H_
#define _GDPLOGD_SQLITE_H_		1

#include "logd.h"
#include <sqlite3.h>

/*
**	Headers for the physical log implementation.
**		This is how bytes are actually laid out on the disk.
**		This module is private to the physical layer, but it is
**			used by apps/log-view since that needs to crack the
**			physical layout.
*/

// default directory for GDP Log storage (relative to GDP_DEFAULT_DATA_ROOT)
#define GDP_DEFAULT_LOG_DIR	"glogs"

// magic numbers and versions for on-disk database
#define GLOG_MAGIC			UINT32_C(0x47434C30)	// 'GCL0'
#define GLOG_VERSION		UINT32_C(20180428)		// current version
#define GLOG_MINVERS		UINT32_C(20180428)		// lowest readable version
#define GLOG_MAXVERS		UINT32_C(20180428)		// highest readable version
#define GLOG_SUFFIX			".glog"

#define GLOG_READ_BUFFER_SIZE	4096			// size of I/O buffers


/*
**  Per-log info.
**
**		There is no single instantiation of a log, so this is really
**		a representation of an abstraction.  It includes information
**		about all segments and the index.
*/

struct physinfo
{
	// reading and writing to the log requires holding this lock
	EP_THR_RWLOCK		lock;

	// info regarding the entire log (not segment)
	gdp_recno_t			min_recno;				// first recno in log
	gdp_recno_t			max_recno;				// last recno in log (dynamic)
	uint32_t			flags;					// see below
	int32_t				ver;					// database version

	// the underlying SQLite database
	struct sqlite3		*db;					// database handle

	// cache of prepared statements
	struct sqlite3_stmt	*insert_stmt;
	struct sqlite3_stmt	*read_by_hash_stmt;
	struct sqlite3_stmt	*read_by_recno_stmt1;
	struct sqlite3_stmt	*read_by_recno_stmt2;
	struct sqlite3_stmt	*read_by_timestamp_stmt;
};

// values for physinfo:flags
#define LOG_POSIX_ERRORS		0x00000002	// send posix errors to syslog

#endif //_GDPLOGD_SQLITE_H_
