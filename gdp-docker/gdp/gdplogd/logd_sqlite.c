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


/*
**  Implement GDP logs with SQLite.
**
**		Note: this file is "#include"d by apps/gdp-log-check.c with
**		the LOG_CHECK #define set.  Messy.
*/

#include "logd.h"
#include "logd_sqlite.h"

#include <gdp/gdp_buf.h>
#include <gdp/gdp_md.h>

#include <ep/ep_hexdump.h>
#include <ep/ep_net.h>
#include <ep/ep_string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>


static EP_DBG	Dbg = EP_DBG_INIT("gdplogd.sqlite", "GDP Log Daemon SQLite Physical Log");

#define GOB_PATH_MAX		260			// max length of pathname

static bool			SQLiteInitialized = false;
static int			GOBfilemode;		// the file mode on create
static uint32_t		DefaultLogFlags;	// as indicated
static char			LogDir[GOB_PATH_MAX];	// the gob data directory

#define GETPHYS(gob)	((gob)->x->physinfo)

#define FLAG_TMPFILE		0x00000001	// this is a temporary file


/*
**  The database schema for logs.
**
**		Timestamp usage note: using a 64-bit signed integer nanoseconds
**			centered on 1970/01/01 means that only dates from about 1680
**			through 2360 can be represented; the EP_TIME_SPEC range is
**			much higher.  For this purpose, this is probably OK.  An
**			alternative would be to store dates in microseconds, which
**			is the limit of what most Linux clocks will give you as of
**			this date.
*/

static const char *LogSchema =
				"CREATE TABLE log_entry (\n"
				"	hash BLOB(32) PRIMARY KEY ON CONFLICT IGNORE,\n"
				"	recno INTEGER,\n"
				"	timestamp INTEGER,\n"	// 64 bit, nanoseconds since 1/1/70
				"	accuracy FLOAT,\n"		// in seconds (single precision)
				"	prevhash BLOB(32),\n"
				"	value BLOB,\n"
				"	sig BLOB);\n"
				"CREATE INDEX recno_index\n"
				"	ON log_entry(recno);\n"
				"CREATE INDEX timestamp_index\n"
				"	ON log_entry(timestamp);\n";


/*
**  FSIZEOF --- return the size of a file
*/

#if 0		// currently unused
static off_t
fsizeof(FILE *fp)
{
	struct stat st;

	if (fstat(fileno(fp), &st) < 0)
	{
		char errnobuf[200];

		(void) (0 == strerror_r(errno, errnobuf, sizeof errnobuf));
		ep_dbg_cprintf(Dbg, 1, "fsizeof: fstat failure: %s\n", errnobuf);
		return -1;
	}

	return st.st_size;
}
#endif


/*
**  POSIX_ERROR --- flag error caused by a Posix (Unix) syscall
*/

static EP_STAT EP_TYPE_PRINTFLIKE(2, 3)
posix_error(int _errno, const char *fmt, ...)
{
	va_list ap;
	EP_STAT estat = ep_stat_from_errno(_errno);

	va_start(ap, fmt);
	if (EP_UT_BITSET(LOG_POSIX_ERRORS, DefaultLogFlags))
		ep_logv(estat, fmt, ap);
	else if (!SQLiteInitialized || ep_dbg_test(Dbg, 1))
		ep_app_messagev(estat, fmt, ap);
	va_end(ap);

	return estat;
}


/*
**  SQLITE_RC_SUCCESS --- return TRUE if response code is an OK value
*/

static bool
sqlite_rc_success(int rc)
{
	return rc == SQLITE_OK || rc == SQLITE_ROW || rc == SQLITE_DONE;
}

#define CHECK_RC(rc, action)	if (!sqlite_rc_success(rc)) action


// can we use SQLITE_STATIC for greater efficiency?
#define BLOB_DESTRUCTOR		SQLITE_TRANSIENT

/*
**  SQLITE_ERROR --- flag error caused by an SQLite error
*/

static EP_STAT
ep_stat_from_sqlite_result_code(int rc)
{
	int sev = EP_STAT_SEV_ERROR;
	int registry = EP_REGISTRY_UCB;
	int module = SQLITE_MODULE;

	switch (rc & 0x00ff)
	{
		case SQLITE_OK:
			return EP_STAT_OK;

		case SQLITE_ROW:
		case SQLITE_DONE:
			sev = EP_STAT_SEV_OK;
			break;

		case SQLITE_ABORT:
		case SQLITE_INTERNAL:
			sev = EP_STAT_SEV_ABORT;
			break;

		case SQLITE_WARNING:
			sev = EP_STAT_SEV_WARN;
			break;

		case SQLITE_CANTOPEN:
			return GDP_STAT_NAK_NOTFOUND;
	}

	return EP_STAT_NEW(sev, registry, module, rc & 0x00ff);
}

#define SQLITE_STAT(sev, detail)	\
				EP_STAT_NEW(EP_STAT_SEV_ ## sev,	\
				EP_REGISTRY_UCB, SQLITE_MODULE, detail)

static struct ep_stat_to_string SQLiteStats[] =
{
	// module name
	{ SQLITE_STAT(OK,    0),				"GDP-SQLite"					},

	// individual codes
	{ SQLITE_STAT(OK,    SQLITE_ROW),		"Row available"					},
	{ SQLITE_STAT(OK,    SQLITE_DONE),		"Operation complete"			},
	{ SQLITE_STAT(ABORT, SQLITE_ABORT),		"Operation aborted"				},
	{ SQLITE_STAT(WARN,  SQLITE_WARNING),	"SQLite generic warning"		},
	{ SQLITE_STAT(ERROR, SQLITE_CANTOPEN),	"Cannot open file"				},
	{ SQLITE_STAT(ERROR, SQLITE_ERROR),		"SQLite generic error"			},
	{ SQLITE_STAT(ABORT, SQLITE_INTERNAL),	"SQLite internal error"			},
	{ SQLITE_STAT(ERROR, SQLITE_PERM),		"SQLite access denied"			},
	{ SQLITE_STAT(ERROR, SQLITE_BUSY),		"SQLite busy"					},
	{ SQLITE_STAT(ERROR, SQLITE_LOCKED),	"SQLite write conflict"			},
	{ SQLITE_STAT(ERROR, SQLITE_NOMEM),		"Out of memory"					},
	{ SQLITE_STAT(ERROR, SQLITE_READONLY),	"Cannot write read-only db"		},
	{ SQLITE_STAT(WARN,  SQLITE_INTERRUPT),	"SQLite interrupt"				},
	{ SQLITE_STAT(ERROR, SQLITE_IOERR),		"SQLite I/O error"				},
	{ SQLITE_STAT(SEVERE,SQLITE_CORRUPT),	"Corrupt database file"			},
	{ SQLITE_STAT(ERROR, SQLITE_NOTFOUND),	"sqlite3_file_control error"	},
	{ SQLITE_STAT(ERROR, SQLITE_FULL),		"No space left on disk"			},
	{ SQLITE_STAT(ERROR, SQLITE_CANTOPEN),	"Cannot open file"				},
	{ SQLITE_STAT(ERROR, SQLITE_PROTOCOL),	"File locking error"			},
	{ SQLITE_STAT(ERROR, SQLITE_SCHEMA),	"SQLite schema changed"			},
	{ SQLITE_STAT(ERROR, SQLITE_TOOBIG),	"SQLite blob too big"			},
	{ SQLITE_STAT(ERROR, SQLITE_CONSTRAINT), "SQLite constraint violated"	},
	{ SQLITE_STAT(ERROR, SQLITE_MISMATCH),	"SQLite data type mismatch"		},
	{ SQLITE_STAT(ABORT, SQLITE_MISUSE),	"API misuse, coding error"		},
	{ SQLITE_STAT(ERROR, SQLITE_NOLFS),		"No Large File Support"			},
	{ SQLITE_STAT(ERROR, SQLITE_AUTH),		"SQLite not authorized"			},
	{ SQLITE_STAT(ERROR, SQLITE_RANGE),		"SQLite argument out of range"	},
	{ SQLITE_STAT(ERROR, SQLITE_NOTADB),	"Not an SQLite database"		},
	{ SQLITE_STAT(WARN,  SQLITE_ROW),		"More rows available"			},
	{ SQLITE_STAT(WARN,  SQLITE_DONE),		"Operation complete"			},

	{ EP_STAT_OK,							NULL							},
};

static EP_STAT
sqlite_error(int rc, const char *sqerrmsg, const char *where, const char *phase)
{
	EP_STAT estat = ep_stat_from_sqlite_result_code(rc);
	if (EP_STAT_ISOK(estat))
		estat = GDP_STAT_SQLITE_ERROR;
	if ((rc != SQLITE_OK || sqerrmsg != NULL) && ep_dbg_test(Dbg, 1))
	{
		char ebuf[100];
		if (sqerrmsg == NULL)
			sqerrmsg = sqlite3_errstr(rc);
		ep_dbg_cprintf(Dbg, 1,
					"SQLite status (%s during %s): %s (%d)\n\t%s\n",
					where, phase, sqerrmsg, rc,
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}


static void
sqlite_logger(void *unused, int rc, const char *msg)
{
	ep_dbg_printf("SQLite log %d: %s\n", rc, msg);
}


static struct pragma_name
{
	const char	*pname;
	const char	*pdefault;
}					SqlitePragmaNames[] =
					{
						{ "synchronous",			"OFF",				},
						{ "journal_mode",			"TRUNCATE"			},
						{ "temp_store",				"MEMORY",			},
						{ "locking_mode",			"NORMAL",			},
						{ "cache_size",				NULL,				},
						{ "page_size",				NULL,				},
						{ "journal_size_limit",		NULL,				},
						{ "busy_timeout",			"20",				},
						{ NULL,						NULL				},
					};
static char			*SqlitePragmasSQL;		// SQL to set pragmas

static void
sqlite_init_pragmas(void)
{
	gdp_buf_t *sqlbuf = gdp_buf_new();
	struct pragma_name *pp = SqlitePragmaNames;

	for (; pp->pname != NULL; pp++)
	{
		char pnamebuf[100];

		snprintf(pnamebuf, sizeof pnamebuf,
				"swarm.gdplogd.sqlite.pragma.%s", pp->pname);
		const char *pv = ep_adm_getstrparam(pnamebuf, pp->pdefault);
		if (pv != NULL)
			gdp_buf_printf(sqlbuf, "   PRAGMA %s = %s;\n", pp->pname, pv);
	}
	size_t qlen = gdp_buf_getlength(sqlbuf);
	if (qlen > 0)
	{
		SqlitePragmasSQL = ep_mem_malloc(qlen + 1);
		snprintf(SqlitePragmasSQL, qlen + 1, "%s", gdp_buf_getptr(sqlbuf, qlen));
		ep_dbg_cprintf(Dbg, 10, "SQLite3 Pragmas:\n%s", SqlitePragmasSQL);
	}
}


static EP_STAT
sqlite_set_pragmas(struct sqlite3 *db, const char *logname)
{
	EP_STAT estat = EP_STAT_OK;
	int rc;
	char *sqerrstr = NULL;

	if (SqlitePragmasSQL == NULL)
		return estat;

	rc = sqlite3_exec(db, SqlitePragmasSQL, NULL, NULL, &sqerrstr);
	if (rc != SQLITE_OK || sqerrstr != NULL)
	{
		estat = sqlite_error(rc, sqerrstr, logname, "sqlite_set_pragmas");
		ep_dbg_cprintf(Dbg, 1, "    query = %s", SqlitePragmasSQL);
		if (sqerrstr != NULL)
			sqlite3_free(sqerrstr);
	}
	return estat;
}


/*
**  Initialize the physical I/O module
**
**		Note this is always called before threads have been spawned.
*/

static EP_STAT
sqlite_init(const char *logroot)
{
	EP_STAT estat = EP_STAT_OK;

	// find physical location of GOB directory
	if (logroot != NULL)
		strlcpy(LogDir, logroot, sizeof LogDir);
	else
	{
		estat = _gdp_adm_path_find("swarm.gdp.data.root", GDP_DEFAULT_DATA_ROOT,
							"swarm.gdplogd.log.dir", GDP_DEFAULT_LOG_DIR,
							LogDir, sizeof LogDir);
		if (!EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			ep_dbg_cprintf(Dbg, 1, "sqlite_init: _gdp_adm_path_find => %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			return estat;
		}
	}

	// we will run out of that directory
	if (chdir(LogDir) != 0)
	{
		estat = ep_stat_from_errno(errno);
		ep_app_message(estat, "sqlite_init: chdir(%s)", LogDir);
		return estat;
	}

	// find the file creation mode
	GOBfilemode = ep_adm_getintparam("swarm.gdplogd.gob.mode", 0600);

	if (ep_adm_getboolparam("swarm.gdplogd.sqlite.log-posix-errors", false))
		DefaultLogFlags |= LOG_POSIX_ERRORS;

	// arrange to log SQLite errors
	if (ep_adm_getboolparam("swarm.gdplogd.sqlite.log-sqlite-errors", true))
		sqlite3_config(SQLITE_CONFIG_LOG, sqlite_logger, NULL);

	sqlite3_config(SQLITE_CONFIG_SERIALIZED);	// fully threadable
	sqlite3_initialize();
	ep_stat_reg_strings(SQLiteStats);

	sqlite_init_pragmas();

	SQLiteInitialized = true;
	ep_dbg_cprintf(Dbg, 8, "sqlite_init: log dir = %s, mode = 0%o\n",
			LogDir, GOBfilemode);

	return estat;
}


/*
**  SQL_BIND_* --- GDP-specific type bindings
**
**		Note: sql_bind_timestamp always binds two fields (the time
**			itself and the accuracy), which means the SQL must always
**			list these two fields adjacently.
*/

static int
sql_bind_hash(sqlite3_stmt *stmt, int index, gdp_hash_t *hash)
{
	size_t hashlen;
	void *hashptr = gdp_hash_getptr(hash, &hashlen);
	int rc = sqlite3_bind_blob(stmt, index, hashptr, hashlen, BLOB_DESTRUCTOR);
	return rc;
}

static int
sql_bind_recno(sqlite3_stmt *stmt, int index, gdp_recno_t recno)
{
	int rc = sqlite3_bind_int64(stmt, index, recno);
	return rc;
}

static int
sql_bind_timestamp(sqlite3_stmt *stmt, int index, EP_TIME_SPEC *ts)
{
	int rc = sqlite3_bind_int64(stmt, index, ep_time_to_nsec(ts));
	if (rc != 0)
		return rc;
	// Note the use of adjacent fields
	rc = sqlite3_bind_double(stmt, index + 1, ts->tv_accuracy);
	return rc;
}

static int
sql_bind_signature(sqlite3_stmt *stmt, int index, gdp_datum_t *datum)
{
	size_t siglen;
	void *sigptr = gdp_sig_getptr(datum->sig, &siglen);

	int rc = sqlite3_bind_blob(stmt, index, sigptr, siglen, BLOB_DESTRUCTOR);
	return rc;
}

static int
sql_bind_buf(sqlite3_stmt *stmt, int index, gdp_buf_t *buf)
{
	size_t buflen = gdp_buf_getlength(buf);
	int rc = sqlite3_bind_blob(stmt, index, gdp_buf_getptr(buf, buflen),
							buflen, BLOB_DESTRUCTOR);
	return rc;
}


/*
**	GET_LOG_PATH --- get the pathname to an on-disk version of the gob
*/

static EP_STAT
get_log_path(gdp_gob_t *gob,
		const char *sfx,
		char *pbuf,
		int pbufsiz)
{
	EP_STAT estat = EP_STAT_OK;
	gdp_pname_t pname;
	int i;
	struct stat st;

	EP_ASSERT_POINTER_VALID(gob);

	errno = 0;
	gdp_printable_name(gob->name, pname);

	// find the subdirectory based on the first part of the name
	i = snprintf(pbuf, pbufsiz, "%s/_%02x", LogDir, gob->name[0]);
	if (i >= pbufsiz)
		goto fail1;
	if (stat(pbuf, &st) < 0)
	{
		// doesn't exist; we need to create it
		ep_dbg_cprintf(Dbg, 11, "get_log_path: creating %s\n", pbuf);
		i = mkdir(pbuf, 0775);
		if (i < 0)
			goto fail0;
	}
	else if ((st.st_mode & S_IFMT) != S_IFDIR)
	{
		errno = ENOTDIR;
		goto fail0;
	}

	// now return the final complete name
	i = snprintf(pbuf, pbufsiz, "%s/_%02x/%s%s",
				LogDir, gob->name[0], pname, sfx);
	if (i < pbufsiz)
		return EP_STAT_OK;

fail1:
	estat = EP_STAT_BUF_OVERFLOW;

fail0:
	{
		char ebuf[100];

		if (EP_STAT_ISOK(estat))
		{
			if (errno == 0)
				estat = EP_STAT_ERROR;
			else
				estat = ep_stat_from_errno(errno);
		}

		ep_dbg_cprintf(Dbg, 1, "get_log_path(%s):\n\t%s\n",
				pbuf, ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}


/*
**  Allocate/Free the in-memory version of the physical representation
**		of a GOB.
**
**		XXX Currently allocates space for the first segment.
**		XXX That should probably be deferred until it is actually
**			read off of disk.
*/

static gob_physinfo_t *
physinfo_alloc(gdp_gob_t *gob)
{
	gob_physinfo_t *phys = (gob_physinfo_t *) ep_mem_zalloc(sizeof *phys);

	if (ep_thr_rwlock_init(&phys->lock) != 0)
		goto fail1;

	return phys;

fail1:
	ep_dbg_cprintf(Dbg, 1, "physinfo_alloc: cannot create rwlock: %s\n",
			strerror(errno));
	ep_mem_free(phys);
	return NULL;
}


static void
physinfo_free(gob_physinfo_t *phys)
{
	if (phys == NULL)
		return;

	if (phys->db != NULL)
	{
		int rc;

		ep_dbg_cprintf(Dbg, 41, "physinfo_free: closing db @ %p\n", phys->db);

		// clean up previously prepared statements
		if (phys->read_by_recno_stmt1 != NULL)
			sqlite3_finalize(phys->read_by_recno_stmt1);
		if (phys->read_by_recno_stmt2 != NULL)
			sqlite3_finalize(phys->read_by_recno_stmt2);
		if (phys->read_by_hash_stmt != NULL)
			sqlite3_finalize(phys->read_by_hash_stmt);
		if (phys->read_by_timestamp_stmt != NULL)
			sqlite3_finalize(phys->read_by_timestamp_stmt);
		if (phys->insert_stmt != NULL)
			sqlite3_finalize(phys->insert_stmt);

		// we can now close the database
		rc = sqlite3_close(phys->db);
		if (rc != 0)		//XXX
			(void) sqlite_error(rc, NULL, "physinfo_free", "cannot close db");
		phys->db = NULL;
	}

	if (ep_thr_rwlock_destroy(&phys->lock) != 0)
		(void) posix_error(errno, "physinfo_free: cannot destroy rwlock");

	ep_mem_free(phys);
}


static void
physinfo_dump(gob_physinfo_t *phys, FILE *fp)
{
	fprintf(fp, "physinfo @ %p: min_recno %" PRIgdp_recno
			", max_recno %" PRIgdp_recno "\n",
			phys, phys->min_recno, phys->max_recno);
	fprintf(fp, "\tdb %p, ver %d\n", phys->db, phys->ver);
}


#if GDP_LOG_VIEW
# define SQLITE_OPEN_FLAGS	(SQLITE_OPEN_READONLY)
#else
# define SQLITE_OPEN_FLAGS	(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)
#endif

/*
**  SQLITE_CREATE --- create a brand new GOB on disk
*/

static EP_STAT
sqlite_create(gdp_gob_t *gob, gdp_md_t *gmd)
{
	EP_STAT estat = EP_STAT_OK;
	gob_physinfo_t *phys;
	const char *phase = "init";
	int rc;
	char *sqerrstr = NULL;

	EP_ASSERT_POINTER_VALID(gob);

	// allocate space for the physical information
	phys = physinfo_alloc(gob);
	if (phys == NULL)
		goto fail0;
	gob->x->physinfo = phys;

	// allocate a name
	if (!gdp_name_is_valid(gob->name))
	{
		estat = _gdp_gob_newname(gob);
		EP_STAT_CHECK(estat, goto fail0);
	}

	// create an empty file to hold the log database (this is required
	// to get the file mode right)
	char db_path[GOB_PATH_MAX];
	estat = get_log_path(gob, GLOG_SUFFIX, db_path, sizeof db_path);
	EP_STAT_CHECK(estat, goto fail0);

	ep_dbg_cprintf(Dbg, 20, "sqlite_create: creating %s\n", db_path);
	int fd = open(db_path, O_RDWR | O_CREAT | O_APPEND | O_EXCL,
					GOBfilemode);
	if (fd < 0)
	{
		char nbuf[40];

		estat = ep_stat_from_errno(errno);
		(void) (0 == strerror_r(errno, nbuf, sizeof nbuf));
		ep_log(estat, "sqlite_create(%s): %s", db_path, nbuf);
		goto fail0;
	}
	close(fd);

	// now open (and thus create) the actual database
	phase = "sqlite3_open_v2";
	rc = sqlite3_open_v2(db_path, &phys->db, SQLITE_OPEN_FLAGS, NULL);
	CHECK_RC(rc, goto fail1);

	phase = "sqlite3_extended_result_codes";
	rc = sqlite3_extended_result_codes(phys->db, 1);
	CHECK_RC(rc, goto fail1);

	// tweak database header using PRAGMAs
	//XXX could these be done with sqlite3_exec?
	phase = "pragma prepare";
	{
		// https://www.sqlite.org/pragma.html
		char qbuf[200];
		sqlite3_stmt *stmt;

		// set up application ID (the GDP itself)
		snprintf(qbuf, sizeof qbuf,
				"PRAGMA application_id = %d;\n", GLOG_MAGIC);
		rc = sqlite3_prepare_v2(phys->db, qbuf, -1, &stmt, NULL);
		CHECK_RC(rc, goto fail1);
		phase = "pragma step 1";
		while ((rc = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			ep_dbg_cprintf(Dbg, 7, "create(%s), pragma setting => %s\n",
					db_path, sqlite3_errstr(rc));
		}
		sqlite3_finalize(stmt);

		// set up a version code (we use privately)
		snprintf(qbuf, sizeof qbuf,
				"PRAGMA user_version = %d;\n", GLOG_VERSION);

		rc = sqlite3_prepare_v2(phys->db, qbuf, -1, &stmt, NULL);
		CHECK_RC(rc, goto fail1);
		phase = "pragma step 2";
		while ((rc = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			ep_dbg_cprintf(Dbg, 7, "create(%s), pragma setting => %s\n",
					db_path, sqlite3_errstr(rc));
		}
		sqlite3_finalize(stmt);
		phys->ver = GLOG_VERSION;
	}

	// set performance pragmas
	phase = "set pragmas";
	estat = sqlite_set_pragmas(phys->db, gob->pname);
	if (!EP_STAT_ISOK(estat))
		goto fail0;

	phase = "create schema";
	{
		// create the database schema: primary table and indices
		// https://www.sqlite.org/c3ref/exec.html
		// sqlite3_exec(db, cmd, callback, closure, *errmsg)
		rc = sqlite3_exec(phys->db, LogSchema, NULL, NULL, &sqerrstr);
		CHECK_RC(rc, goto fail1);
	}

	//XXX should probably use Write Ahead Logging
	// http://www.sqlite.org/wal.html

	// write metadata to log
	phase = "metadata prepare";
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(phys->db,
				"INSERT INTO log_entry"
				"	(hash, recno, timestamp, accuracy, value)"
				"	VALUES (?, 0, ?, ?, ?);",
				-1, &stmt, NULL);
	CHECK_RC(rc, goto fail3);

	// bind log name
	phase = "metadata bind 1";
	rc = sqlite3_bind_blob(stmt, 1, gob->name, sizeof gob->name,
						SQLITE_STATIC);
	CHECK_RC(rc, goto fail3);

	// bind creation timestamp
	phase = "metadata bind 2";
	{
		EP_TIME_SPEC ts;

		ep_time_now(&ts);
		rc = sql_bind_timestamp(stmt, 2, &ts);
		CHECK_RC(rc, goto fail3);
	}

	// bind metadata content
	phase = "metadata bind 4";
	{
		uint8_t *obuf;
		size_t mdsize = _gdp_md_serialize(gmd, &obuf);
		if (ep_dbg_test(Dbg, 34))
		{
			ep_dbg_printf("sqlite_create: gmd %p obuf %p mdsize %zd\n",
					gmd, obuf, mdsize);
			ep_hexdump(obuf, mdsize, ep_dbg_getfile(), EP_HEXDUMP_ASCII, 0);
		}
		if (gmd == NULL || mdsize == 0)
		{
			ep_dbg_cprintf(Dbg, 1,
					"sqlite_create: no metadata (gmd %p)\n",
					gmd);
			estat = GDP_STAT_METADATA_REQUIRED;
			goto fail0;
		}
		rc = sqlite3_bind_blob(stmt, 4, obuf, mdsize, SQLITE_TRANSIENT);
		CHECK_RC(rc, goto fail3);
	}

	//FIXME: need values for prevhash

	// do the hard work
	phase = "metadata step";
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
fail3:
		//FIXME: obuf doesn't get freed if one of the earlier binds fails first
		estat = ep_stat_from_sqlite_result_code(rc);
	}
	sqlite3_finalize(stmt);
	EP_STAT_CHECK(estat, goto fail1);

	phys->min_recno = 1;
	phys->max_recno = 0;
	phys->flags |= DefaultLogFlags;
	ep_dbg_cprintf(Dbg, 11, "Created new GDP Log %s\n", gob->pname);
	return estat;

fail1:
	// failure resulted from an SQLite error
	estat = sqlite_error(rc, sqerrstr, "sqlite_create", phase);
	if (sqerrstr != NULL)
		sqlite3_free(sqerrstr);
	sqerrstr = NULL;

fail0:
	// turn OK into an errno-based code
	if (EP_STAT_ISOK(estat))
		estat = ep_stat_from_errno(errno);
	if (EP_STAT_ISOK(estat))
		estat = GDP_STAT_NAK_INTERNAL;

	// turn "file exists" into a meaningful response code
	if (EP_STAT_IS_SAME(estat, ep_stat_from_errno(EEXIST)))
			estat = GDP_STAT_NAK_CONFLICT;

	// free up resources
	if (phys != NULL)
	{
		physinfo_free(phys);
		gob->x->physinfo = phys = NULL;
	}

	if (ep_dbg_test(Dbg, 1))
	{
		char ebuf[100];

		ep_dbg_printf("Could not create GOB during %s: %s\n",
				phase, ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}


/*
**  Check SQLite PRAGMAs for correctness.
*/

static EP_STAT
check_pragma(struct sqlite3 *db,
			const char *name,
			int expected,
			EP_STAT errstat)
{
	EP_STAT estat;
	sqlite3_stmt *stmt;
	int rc;
	char qbuf[50];

	snprintf(qbuf, sizeof qbuf, "PRAGMA %s;", name);
	rc = sqlite3_prepare_v2(db, qbuf, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		estat = sqlite_error(rc, NULL, "sqlite_open pragma prepare", name);
		if (EP_STAT_ISOK(estat))
			estat = GDP_STAT_SQLITE_ERROR;
		return estat;
	}

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW)
	{
		estat = sqlite_error(rc, NULL, "sqlite_open pragma check", name);
		if (EP_STAT_ISOK(estat))
			estat = GDP_STAT_SQLITE_ERROR;
		sqlite3_finalize(stmt);
		return estat;
	}

	// check to make sure this is really a database we understand
	int actual = sqlite3_column_int(stmt, 0);
	if (actual == expected)
	{
		estat = EP_STAT_OK;
	}
	else
	{
		estat = errstat;
		ep_log(estat, "database corruption error: unknown %s %d expected %d",
				name, actual, expected);
	}
	sqlite3_finalize(stmt);
	return estat;
}


/*
**	SQLITE_OPEN --- do physical open of a GOB
*/

static EP_STAT
sqlite_open(gdp_gob_t *gob)
{
	EP_STAT estat = EP_STAT_OK;
	int rc = SQLITE_OK;
	gob_physinfo_t *phys;
	const char *phase;

	ep_dbg_cprintf(Dbg, 20, "sqlite_open(%s)\n", gob->pname);

	// allocate space for physical data
	EP_ASSERT(gob->x != NULL);
	EP_ASSERT(GETPHYS(gob) == NULL);
	phase = "physinfo_alloc";
	errno = 0;
	gob->x->physinfo = phys = physinfo_alloc(gob);
	if (phys == NULL)
		goto fail0;
	phys->flags |= DefaultLogFlags;

	/*
	** Time to open the database!
	*/

	phase = "get db path";
	char db_path[GOB_PATH_MAX];
	estat = get_log_path(gob, GLOG_SUFFIX, db_path, sizeof db_path);
	EP_STAT_CHECK(estat, goto fail1);

	phase = "sqlite3_open_v2";
	rc = sqlite3_open_v2(db_path, &phys->db, SQLITE_OPEN_READWRITE, NULL);
	if (rc != SQLITE_OK)
		goto fail2;

	phase = "sqlite3_extended_result_codes";
	rc = sqlite3_extended_result_codes(phys->db, 1);
	if (rc != SQLITE_OK)
		goto fail2;

	// verify that db is actually one we understand (application_id)
	// and it's the correct version (user_version)
	phase = "database verification";
	{
		estat = check_pragma(phys->db, "application_id", GLOG_MAGIC,
							GDP_STAT_CORRUPT_LOG);
		if (EP_STAT_ISOK(estat))
			estat = check_pragma(phys->db, "user_version", GLOG_VERSION,
							GDP_STAT_LOG_VERSION_MISMATCH);
		if (!EP_STAT_ISOK(estat))
			goto fail1;
	}

	// set performance pragmas
	phase = "set pragmas";
	estat = sqlite_set_pragmas(phys->db, gob->pname);
	if (!EP_STAT_ISOK(estat))
		goto fail1;

	// read metadata
	phase = "metadata read";
	if (gob->gob_md == NULL)
	{
		sqlite3_stmt *stmt;
		rc = sqlite3_prepare_v2(phys->db,
						"SELECT value FROM log_entry"
						"	WHERE recno = 0",
						-1, &stmt, NULL);
		CHECK_RC(rc, goto fail2);
		rc = sqlite3_step(stmt);
		CHECK_RC(rc, goto fail4);
		const uint8_t *md_blob;
		size_t md_size = sqlite3_column_bytes(stmt, 0);
		md_blob = sqlite3_column_blob(stmt, 0);
		if (md_blob != NULL && md_size > 0)
		{
			gob->gob_md = _gdp_md_deserialize(md_blob, md_size);
		}
fail4:
		sqlite3_finalize(stmt);
		CHECK_RC(rc, goto fail2);
	}

	// read stats
	{
		sqlite3_stmt *stmt;
		rc = sqlite3_prepare_v2(phys->db,
						"SELECT min(recno), max(recno) FROM log_entry"
						"    WHERE recno > 0;",
						-1, &stmt, NULL);
		CHECK_RC(rc, goto fail2);
		rc = sqlite3_step(stmt);
		if (rc == SQLITE_ROW)
		{
			phys->min_recno = sqlite3_column_int64(stmt, 0);
			phys->max_recno = gob->nrecs = sqlite3_column_int64(stmt, 1);
		}

		sqlite3_finalize(stmt);
		CHECK_RC(rc, goto fail2);
	}

	if (ep_dbg_test(Dbg, 20))
	{
		ep_dbg_printf("sqlite_open => ");
		physinfo_dump(phys, ep_dbg_getfile());
	}
	return estat;

fail2:
	if (EP_STAT_ISOK(estat))
	{
		if (rc != SQLITE_OK)
			estat = ep_stat_from_sqlite_result_code(rc);
		else if (errno != 0)
			estat = ep_stat_from_errno(errno);
		else
			estat = GDP_STAT_NAK_INTERNAL;
	}
fail1:
	physinfo_free(phys);
	gob->x->physinfo = phys = NULL;

fail0:
	if (ep_dbg_test(Dbg, 9))
	{
		char ebuf[100];

		ep_dbg_printf("sqlite_open(%s): couldn't open GOB %s:\n\t%s\n",
				phase, gob->pname, ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}

/*
**	SQLITE_CLOSE --- physically close an open GOB
*/

static EP_STAT
sqlite_close(gdp_gob_t *gob)
{
	EP_ASSERT_POINTER_VALID(gob);
	ep_dbg_cprintf(Dbg, 20, "sqlite_close(%s)\n", gob->pname);

	if (gob->x == NULL || GETPHYS(gob) == NULL)
	{
		// close as a result of incomplete open; just ignore it
		return EP_STAT_OK;
	}
	physinfo_free(GETPHYS(gob));
	gob->x->physinfo = NULL;

	return EP_STAT_OK;
}


/*
**  SQLITE_REMOVE --- remove a disk-based log
**
**		It is assume that permission has already been granted.
*/

static EP_STAT
sqlite_remove(gdp_gob_t *gob)
{
	EP_STAT estat = EP_STAT_OK;

	if (!EP_ASSERT_POINTER_VALID(gob) || !EP_ASSERT_POINTER_VALID(gob->x))
		return EP_STAT_ASSERT_ABORT;

	ep_dbg_cprintf(Dbg, 18, "sqlite_remove(%s)\n", gob->pname);

	DIR *dir;
	char dbuf[GOB_PATH_MAX];

	snprintf(dbuf, sizeof dbuf, "%s/_%02x", LogDir, gob->name[0]);
	ep_dbg_cprintf(Dbg, 21, "  remove directory %s%s%s\n",
					EpChar->lquote, dbuf, EpChar->rquote);
	dir = opendir(dbuf);
	if (dir == NULL)
	{
		estat = ep_stat_from_errno(errno);
		goto fail0;
	}

	for (;;)
	{
		struct dirent *dent;

		// read the next directory entry
		dent = readdir(dir);
		if (dent == NULL)
			break;

		ep_dbg_cprintf(Dbg, 50, "  remove trial %s%s%s ",
						EpChar->lquote, dent->d_name, EpChar->rquote);
		if (strncmp(gob->pname, dent->d_name, GDP_GOB_PNAME_LEN) == 0)
		{
			char filenamebuf[GOB_PATH_MAX];

			ep_dbg_cprintf(Dbg, 50, "unlinking\n");
			snprintf(filenamebuf, sizeof filenamebuf, "_%02x/%s",
					gob->name[0], dent->d_name);
			if (unlink(filenamebuf) < 0)
				estat = posix_error(errno, "unlink(%s)", filenamebuf);
		}
		else
		{
			ep_dbg_cprintf(Dbg, 50, "skipping\n");
		}
	}
	closedir(dir);

fail0:
	physinfo_free(GETPHYS(gob));
	gob->x->physinfo = NULL;

	return estat;
}


/*
**  Process SQL results and produce a datum
*/

static void
read_blob(sqlite3_stmt *stmt, int index, gdp_buf_t **blobp)
{
	if (*blobp == NULL)
		*blobp = gdp_buf_new();
	else
		gdp_buf_reset(*blobp);

	const void *blob = sqlite3_column_blob(stmt, index);
	int bloblen = sqlite3_column_bytes(stmt, index);
	if (bloblen > 0)
		gdp_buf_write(*blobp, blob, bloblen);
}

static void
read_hash(sqlite3_stmt *stmt, int index, gdp_hash_t **hashp)
{
	int hashalg = EP_CRYPTO_MD_NULL;		//FIXME: should come from GOB
	if (*hashp == NULL)
		*hashp = gdp_hash_new(hashalg, NULL, 0);
	else
		gdp_hash_reset(*hashp);
	gdp_buf_t *buf = _gdp_hash_getbuf(*hashp);
	read_blob(stmt, index, &buf);
}

static void
read_signature(sqlite3_stmt *stmt, int index, gdp_sig_t **sigp)
{
	int hashalg = EP_CRYPTO_MD_NULL;		//FIXME: should come from GOB
	if (*sigp == NULL)
		*sigp = gdp_sig_new(hashalg, NULL, 0);
	else
		gdp_sig_reset(*sigp);
	gdp_buf_t *buf = _gdp_sig_getbuf(*sigp);
	read_blob(stmt, index, &buf);
}

static EP_STAT
process_row(sqlite3_stmt *stmt,
					gdp_result_cb_t *cb,
					gdp_result_ctx_t *cb_ctx)
{
	EP_STAT estat;
	gdp_datum_t *datum = gdp_datum_new();

	// hash value of this record
	//XXX unneeded except to validate the data
//	{
//		read_hash(stmt, 0, &datum->hash);
//	}

	// record number
	{
		datum->recno = sqlite3_column_int64(stmt, 1);
	}

	// timestamp
	{
		int64_t int_ts = sqlite3_column_int64(stmt, 2);
		ep_time_from_nsec(int_ts, &datum->ts);
		datum->ts.tv_accuracy = sqlite3_column_double(stmt, 3);
	}

	// hash of previous record
	{
		read_hash(stmt, 4, &datum->prevhash);
	}

	// the actual value
	{
		read_blob(stmt, 5, &datum->dbuf);
	}

	// signature (if it exists)
	{
		read_signature(stmt, 6, &datum->sig);
	}

	estat = (*cb)(GDP_STAT_ACK_CONTENT, datum, cb_ctx);

	gdp_datum_free(datum);
	return estat;
}

static EP_STAT
process_select_results(sqlite3_stmt *stmt,
					gdp_result_cb_t *cb,
					gdp_result_ctx_t *cb_ctx,
					bool one_only)
{
	int rc;
	EP_STAT estat = EP_STAT_OK;
	int nresults = 0;

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		process_row(stmt, cb, cb_ctx);
		nresults++;
		if (one_only)
			break;
	}
	int reset_rc = sqlite3_reset(stmt);
	if (reset_rc != SQLITE_OK)
	{
		estat = sqlite_error(reset_rc, NULL, "process_select_results", "reset");
	}
	else if (rc == SQLITE_ROW)
	{
		// "one_only" case
		estat = GDP_STAT_RESPONSE_SENT;
	}
	else if (rc == SQLITE_DONE)
	{
		if (nresults <= 0)
		{
			// nothing matched
//			estat = (*cb)(GDP_STAT_NAK_NOTFOUND, NULL, cb_ctx);
			estat = GDP_STAT_NAK_NOTFOUND;
		}
		else
		{
			// we had some results, but now we're done
//			estat = (*cb)(GDP_STAT_ACK_END_OF_RESULTS, NULL, cb_ctx);
			estat = GDP_STAT_ACK_END_OF_RESULTS;
		}
	}
	else if (!sqlite_rc_success(rc))
		estat = sqlite_error(rc, NULL, "process_select_results", "step");
	if (EP_STAT_ISOK(estat))
	{
		EP_ASSERT(nresults >= 0);
		estat = EP_STAT_FROM_INT(nresults);
	}
	return estat;
}


/*
**  SQLITE_READ_BY_HASH --- read record indexed by record hash
*/

static EP_STAT
sqlite_read_by_hash(gdp_gob_t *gob,
		gdp_hash_t *hash,
		gdp_result_cb_t *cb,
		void *cb_ctx)
{
	EP_STAT estat = EP_STAT_OK;
	int rc = SQLITE_OK;
	gob_physinfo_t *phys = GETPHYS(gob);
	const char *phase = "init";
	size_t hashlen;
	const void *hashptr = gdp_hash_getptr(hash, &hashlen);

	EP_ASSERT_POINTER_VALID(gob);

	if (ep_dbg_test(Dbg, 44))
	{
		ep_dbg_printf("sqlite_read_by_hash(%s\n    ", gob->pname);
		ep_hexdump(hashptr, hashlen, ep_dbg_getfile(), EP_HEXDUMP_TERSE, 0);
		ep_dbg_printf("\n");
	}

	ep_thr_rwlock_rdlock(&phys->lock);

	if (phys->read_by_hash_stmt == NULL)
	{
		phase = "prepare";
		rc = sqlite3_prepare_v2(phys->db,
						"SELECT hash, recno, timestamp, accuracy, prevhash, value, sig"
						"	FROM log_entry"
						"	WHERE hash = ?;",
						-1, &phys->read_by_hash_stmt, NULL);
		CHECK_RC(rc, goto fail2);
	}

	phase = "bind";
	rc = sql_bind_hash(phys->read_by_hash_stmt, 1, hash);
	CHECK_RC(rc, goto fail2);

	phase = "process results";
	estat = process_select_results(phys->read_by_hash_stmt, cb, cb_ctx, true);

	if (false)
	{
fail2:
		estat = sqlite_error(rc, NULL, "sqlite_read_by_hash", phase);
	}
	if (phys->read_by_hash_stmt != NULL)
	{
		sqlite3_clear_bindings(phys->read_by_hash_stmt);
		sqlite3_reset(phys->read_by_hash_stmt);
	}
	ep_thr_rwlock_unlock(&phys->lock);

	char ebuf[100];
	ep_dbg_cprintf(Dbg, 44, "sqlite_read_by_hash => %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	return estat;
}


/*
**  SQLITE_READ_BY_RECNO --- read record indexed by record number
**
**		Reads in a message indicated by datum->recno into datum.
*/

static EP_STAT
sqlite_read_by_recno(gdp_gob_t *gob,
		gdp_recno_t startrec,
		uint32_t maxrecs,
		gdp_result_cb_t *cb,
		void *cb_ctx)
{
	EP_STAT estat = EP_STAT_OK;
	int rc = SQLITE_OK;
	gob_physinfo_t *phys = GETPHYS(gob);
	bool one_only = maxrecs == 0;
	const char *phase = "init";
	sqlite3_stmt *stmt = NULL;

	if (!EP_ASSERT_POINTER_VALID(gob))
		return EP_STAT_ASSERT_ABORT;

	ep_dbg_cprintf(Dbg, 44, "sqlite_read_by_recno(%s) rec %" PRIgdp_recno ", n %d\n",
			gob->pname, startrec, maxrecs);
	if (one_only)
		maxrecs = 1;

	ep_thr_rwlock_rdlock(&phys->lock);

	if (phys->read_by_recno_stmt1 == NULL)
	{
		const char *sql = "SELECT hash, recno, timestamp, accuracy, prevhash, value, sig\n"
						"	FROM log_entry\n"
						"	WHERE recno = ?\n"
						"   LIMIT ?;\n";
		phase = "prepare";
		ep_dbg_cprintf(Dbg, 55, "preparing %s", sql);
		rc = sqlite3_prepare_v2(phys->db, sql, -1, &stmt, NULL);
		CHECK_RC(rc, goto fail2);
		phys->read_by_recno_stmt1 = stmt;
	}
	if (phys->read_by_recno_stmt2 == NULL)
	{
		const char *sql = "SELECT hash, recno, timestamp, accuracy, prevhash, value, sig\n"
						"	FROM log_entry\n"
						"	WHERE recno >= ?\n"
						"	ORDER BY recno\n"
						"   LIMIT ?;\n";
		phase = "prepare";
		ep_dbg_cprintf(Dbg, 55, "preparing %s", sql);
		rc = sqlite3_prepare_v2(phys->db, sql, -1, &stmt, NULL);
		CHECK_RC(rc, goto fail2);
		phys->read_by_recno_stmt2 = stmt;
	}

	if (one_only)
		stmt = phys->read_by_recno_stmt1;
	else
		stmt = phys->read_by_recno_stmt2;

	phase = "bind1";
	rc = sqlite3_bind_int64(stmt, 1, startrec);
	CHECK_RC(rc, goto fail2);
	phase = "bind2";
	rc = sqlite3_bind_int(stmt, 2, maxrecs);
	CHECK_RC(rc, goto fail2);

	phase = "process results";
	estat = process_select_results(stmt, cb, cb_ctx, one_only);

	if (false)
	{
fail2:
		estat = sqlite_error(rc, NULL, "sqlite_read_by_recno", phase);
	}
	if (stmt != NULL)
	{
		sqlite3_clear_bindings(stmt);
		sqlite3_reset(stmt);
	}
	ep_thr_rwlock_unlock(&phys->lock);

	char ebuf[100];
	ep_dbg_cprintf(Dbg, 44, "sqlite_read_by_recno => %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	return estat;
}


/*
**  SQLITE_READ_BY_TIMESTAMP --- read record indexed by timestamp
**
**		Reads in a message indicated by datum->ts into datum.
*/

static EP_STAT
sqlite_read_by_timestamp(gdp_gob_t *gob,
		EP_TIME_SPEC *start_time,
		uint32_t maxrecs,
		gdp_result_cb_t *cb,
		void *cb_ctx)
{
	EP_STAT estat = EP_STAT_OK;
	int rc = SQLITE_OK;
	gob_physinfo_t *phys = GETPHYS(gob);
	bool one_only = maxrecs == 0;
	const char *phase = "init";

	EP_ASSERT_POINTER_VALID(gob);

	if (ep_dbg_test(Dbg, 44))
	{
		char time_buf[100];
		ep_time_format(start_time, time_buf, sizeof time_buf,
					EP_TIME_FMT_HUMAN);
		ep_dbg_cprintf(Dbg, 44, "sqlite_read_by_timestamp(%s, %s, %d)\n",
					gob->pname, time_buf, maxrecs);
	}
	if (one_only)
		maxrecs = 1;

	ep_thr_rwlock_rdlock(&phys->lock);

	if (phys->read_by_timestamp_stmt == NULL)
	{
		phase = "prepare";
		rc = sqlite3_prepare_v2(phys->db,
						"SELECT hash, recno, timestamp, accuracy, prevhash, value, sig\n"
						"	FROM log_entry\n"
						"	WHERE timestamp >= ?\n"
						"	ORDER BY timestamp\n"
						"	LIMIT ?;\n",
						-1, &phys->read_by_timestamp_stmt, NULL);
		CHECK_RC(rc, goto fail2);
	}

	phase = "bind1";
	rc = sql_bind_timestamp(phys->read_by_timestamp_stmt, 1, start_time);
	CHECK_RC(rc, goto fail2);
	phase = "bind2";
	rc = sqlite3_bind_int(phys->read_by_timestamp_stmt, 3, maxrecs);
	CHECK_RC(rc, goto fail2);

	phase = "process results";
	estat = process_select_results(phys->read_by_timestamp_stmt, cb, cb_ctx,
							one_only);

	if (false)
	{
fail2:
		estat = sqlite_error(rc, NULL, "sqlite_read_by_timestamp", phase);
	}
	if (phys->read_by_timestamp_stmt != NULL)
	{
		sqlite3_clear_bindings(phys->read_by_timestamp_stmt);
		sqlite3_reset(phys->read_by_timestamp_stmt);
	}
	ep_thr_rwlock_unlock(&phys->lock);

	char ebuf[100];
	ep_dbg_cprintf(Dbg, 44, "sqlite_read_by_timestamp => %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	return estat;
}


/*
**  SQLITE_RECNO_EXISTS --- determine if a record number already exists
*/

#if 0			// currently unused
static int
sqlite_recno_exists(gdp_gob_t *gob, gdp_recno_t recno)
{
	int rc;
	gob_physinfo_t *phys = GETPHYS(gob);
	int rval = 0;
	const char *phase;

	ep_thr_rwlock_rdlock(&phys->lock);

	phase = "prepare";
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(phys->db,
					"SELECT COUNT(hash) WHERE recno = ?;",
					-1, &stmt, NULL);
	CHECK_RC(rc, goto fail1);

	phase = "bind";
	rc = sqlite3_bind_int64(stmt, 1, recno);
	CHECK_RC(rc, goto fail1);

	phase = "step";
	rc = sqlite3_step(stmt);
	CHECK_RC(rc, goto fail1);

	phase = "column";
	rval = sqlite3_column_int(stmt, 0);

	if (false)
	{
fail1:
		sqlite_error(rc, NULL, "sqlite_recno_exists", phase);
	}
	sqlite3_finalize(stmt);

	ep_thr_rwlock_unlock(&phys->lock);
	return rval;
}
#endif


/*
**	SQLITE_APPEND --- append a message to a writable gob
*/

static EP_STAT
sqlite_append(gdp_gob_t *gob,
			gdp_datum_t *datum)
{
	EP_STAT estat = EP_STAT_OK;
	int rc = SQLITE_OK;
	gob_physinfo_t *phys;
	const char *phase;

	if (ep_dbg_test(Dbg, 44))
	{
		ep_dbg_printf("sqlite_append(%s):\n    ", gob->pname);
		gdp_datum_print(datum, ep_dbg_getfile(),
					GDP_DATUM_PRDEBUG |
						(ep_dbg_test(Dbg, 24) ? 0 : GDP_DATUM_PRMETAONLY));
	}

	phys = GETPHYS(gob);
	EP_ASSERT_POINTER_VALID(phys);
	EP_ASSERT_POINTER_VALID(datum);

	ep_thr_rwlock_wrlock(&phys->lock);

	//FIXME: bind datum values into SQL
	gdp_hash_t *hash = NULL;
	phase = "append prepare";
	if (phys->insert_stmt == NULL)
	{
		rc = sqlite3_prepare_v2(phys->db,
					"INSERT INTO log_entry"
					"	(hash, recno, timestamp, accuracy, prevhash, value, sig)"
					"	VALUES(?, ?, ?, ?, ?, ?, ?);",
					-1, &phys->insert_stmt, NULL);
		CHECK_RC(rc, goto fail3);
	}

	phase = "append bind 1";
	hash = _gdp_datum_hash(datum, gob);
	if (hash != NULL)
	{
		rc = sql_bind_hash(phys->insert_stmt, 1, hash);
		CHECK_RC(rc, goto fail3);
	}

	phase = "append bind 2";
	rc = sql_bind_recno(phys->insert_stmt, 2, datum->recno);
	CHECK_RC(rc, goto fail3);

	phase = "append bind 3";
	// actually binds fields 3 and 4
	rc = sql_bind_timestamp(phys->insert_stmt, 3, &datum->ts);
	CHECK_RC(rc, goto fail3);

	if (datum->prevhash != NULL)
	{
		phase = "append bind 5";
		rc = sql_bind_hash(phys->insert_stmt, 5, datum->prevhash);
		CHECK_RC(rc, goto fail3);
	}

	phase = "append bind 6";
	rc = sql_bind_buf(phys->insert_stmt, 6, datum->dbuf);
	CHECK_RC(rc, goto fail3);

	if (datum->sig != NULL)
	{
		phase = "append bind 7";
		rc = sql_bind_signature(phys->insert_stmt, 7, datum);
		CHECK_RC(rc, goto fail3);
	}

	phase = "append step";
	rc = sqlite3_step(phys->insert_stmt);
	if (rc != SQLITE_DONE)
	{
fail3:
		estat = sqlite_error(rc, NULL, "sqlite_append", phase);
		if (hash != NULL)
			gdp_hash_free(hash);
	}

	// done for now
	if (phys->insert_stmt != NULL)
	{
		sqlite3_clear_bindings(phys->insert_stmt);
		sqlite3_reset(phys->insert_stmt);
	}
	ep_thr_rwlock_unlock(&phys->lock);

	return estat;
}


/*
**  GOB_PHYSGETMETADATA --- read metadata from disk
**
**		This is depressingly similar to _gdp_md_deserialize.
*/

#define STDIOCHECK(tag, targ, f)	\
			do	\
			{	\
				int t = f;	\
				if (t != targ)	\
				{	\
					ep_dbg_cprintf(Dbg, 1,	\
							"%s: stdio failure; expected %d got %d (errno=%d)\n"	\
							"\t%s\n",	\
							tag, targ, t, errno, #f)	\
					goto fail_stdio;	\
				}	\
			} while (0);

static EP_STAT
sqlite_getmetadata(gdp_gob_t *gob,
		gdp_md_t **gmdp)
{
//	gdp_md_t *gmd;
//	gob_physinfo_t *phys = GETPHYS(gob);
	EP_STAT estat = EP_STAT_OK;

	//FIXME
	estat = GDP_STAT_NOT_IMPLEMENTED;

	return estat;
}


/*
**  GOB_PHYSFOREACH --- call function for each GOB in directory
**
**		Return the highest severity error code found
*/

static EP_STAT
sqlite_foreach(EP_STAT (*func)(gdp_name_t, void *), void *ctx)
{
	int subdir;
	EP_STAT estat = EP_STAT_OK;

	for (subdir = 0; subdir < 0x100; subdir++)
	{
		DIR *dir;
		char dbuf[400];

		snprintf(dbuf, sizeof dbuf, "%s/_%02x", LogDir, subdir);
		dir = opendir(dbuf);
		if (dir == NULL)
			continue;

		for (;;)
		{
			struct dirent *dent;

			// read the next directory entry
			dent = readdir(dir);
			if (dent == NULL)
				break;

			// we're only interested in .gdpndx files
			char *p = strrchr(dent->d_name, '.');
			if (p == NULL || strcmp(p, GLOG_SUFFIX) != 0)
				continue;

			// strip off the file extension
			*p = '\0';

			// convert the base64-encoded name to internal form
			gdp_name_t gname;
			EP_STAT estat = gdp_internal_name(dent->d_name, gname);
			EP_STAT_CHECK(estat, continue);

			// now call the function
			EP_STAT tstat = (*func)((uint8_t *) gname, ctx);

			// adjust return status only if new one more severe than existing
			if (EP_STAT_SEVERITY(tstat) > EP_STAT_SEVERITY(estat))
				estat = tstat;
		}
		closedir(dir);
	}
	return estat;
}


/*
**  Deliver statistics for management visualization
*/

static void
sqlite_getstats(
		gdp_gob_t *gob,
		struct gob_phys_stats *st)
{
//	unsigned int segno;
//	gob_physinfo_t *phys = GETPHYS(gob);

	st->nrecs = gob->nrecs;
//FIXME:	st->size = fsizeof(x);
}


static EP_STAT
sqlite_xact_begin(gdp_gob_t *gob)
{
	EP_STAT estat = EP_STAT_OK;
	gob_physinfo_t *phys = GETPHYS(gob);
	char *sqerrstr = NULL;

	int rc = sqlite3_exec(phys->db, "BEGIN TRANSACTION;",
						NULL, NULL, &sqerrstr);
	if (!sqlite_rc_success(rc))
	{
		// failure resulted from an SQLite error
		estat = sqlite_error(rc, sqerrstr, "sqlite_xact_begin", "operation");
	}
	if (sqerrstr != NULL)
		sqlite3_free(sqerrstr);
	return estat;
}


static EP_STAT
sqlite_xact_end(gdp_gob_t *gob)
{
	EP_STAT estat = EP_STAT_OK;
	gob_physinfo_t *phys = GETPHYS(gob);
	char *sqerrstr = NULL;

	int rc = sqlite3_exec(phys->db, "END TRANSACTION;",
						NULL, NULL, &sqerrstr);
	if (!sqlite_rc_success(rc))
	{
		// failure resulted from an SQLite error
		estat = sqlite_error(rc, sqerrstr, "sqlite_xact_end", "operation");
	}
	if (sqerrstr != NULL)
		sqlite3_free(sqerrstr);
	return estat;
}


static EP_STAT
sqlite_xact_abort(gdp_gob_t *gob)
{
	EP_STAT estat = EP_STAT_OK;
	gob_physinfo_t *phys = GETPHYS(gob);
	char *sqerrstr = NULL;

	int rc = sqlite3_exec(phys->db, "ROLLBACK TRANSACTION;",
						NULL, NULL, &sqerrstr);
	if (!sqlite_rc_success(rc))
	{
		// failure resulted from an SQLite error
		estat = sqlite_error(rc, sqerrstr, "sqlite_xact_abort", "operation");
	}
	if (sqerrstr != NULL)
		sqlite3_free(sqerrstr);
	return estat;
}


__BEGIN_DECLS
struct gob_phys_impl	GdpSqliteImpl =
{
	.init				= sqlite_init,
	.read_by_hash		= sqlite_read_by_hash,
	.read_by_recno		= sqlite_read_by_recno,
	.read_by_timestamp	= sqlite_read_by_timestamp,
	.create				= sqlite_create,
	.open				= sqlite_open,
	.close				= sqlite_close,
	.append				= sqlite_append,
	.getmetadata		= sqlite_getmetadata,
	.remove				= sqlite_remove,
	.foreach			= sqlite_foreach,
	.getstats			= sqlite_getstats,
	.xact_begin			= sqlite_xact_begin,
	.xact_end			= sqlite_xact_end,
	.xact_abort			= sqlite_xact_abort,
};
__END_DECLS
