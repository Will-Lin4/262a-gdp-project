/* vim: set ai sw=4 sts=4 ts=4 :*/

/*
**  These are split off from gdp_api.c because they are used by
**  by the router.
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
#include "gdp_hongd.h"
#include "gdp_priv.h"

#include <ep/ep_app.h>
#include <ep/ep_b64.h>
#include <ep/ep_dbg.h>
#include <ep/ep_string.h>

#include <sys/param.h>

static EP_DBG	Dbg = EP_DBG_INIT("gdp.name", "GDP name resolution and processing");


static char			*_GdpNameRoot;		// root of names ("current directory")
static bool			HongDsLive = false;

struct conn
{
	STAILQ_ENTRY(conn)	next;			// next connection
	MYSQL				*rconn;			// actual physical connection
	const char			*phase;			// current operation in progress
	struct hongdargs	*ha;			// connection arguments
};

STAILQ_HEAD(clist, conn);

struct conn_pool
{
	EP_THR_MUTEX		mutex;
	EP_THR_COND			cond;				// an idle connection is available
	int					n_alloc;			// number of connections allocated
	int					max_alloc;			// maximum number of connections
	int					maxtries;			// number of retries
	long				backoff;			// backoff between tries
	EP_STAT				(*open)(			// open a connection
								struct conn *conn,
								void *udata);
	void				(*close)(			// close a connection
								struct conn *conn,
								void *udata);
	void				(*reset)(			// reset to idle state
								struct conn *conn,
								void *udata);
	void				*udata;
	STAILQ_HEAD(cpool, conn)		pool;	// list of idle connections
};

static struct conn_pool		CPool;



/*
**  Manage connection pool.
*/

// get an idle connection
static struct conn *
conn_get(struct conn_pool *cpool, bool failfast)
{
	struct conn *conn = NULL;

	ep_thr_mutex_lock(&cpool->mutex);

	// if there are no connections at all, fail early...
	if (cpool->max_alloc <= 0)
		goto fail0;

	// get a connection from the pool, creating new ones if we can,
	// waiting if necessary.
	for (;;)
	{
		EP_STAT estat;

		if ((conn = STAILQ_FIRST(&cpool->pool)) != NULL)
			STAILQ_REMOVE_HEAD(&cpool->pool, next);
		if (conn != NULL)
			break;

		// if we are at our limit we have to wait
		if (cpool->n_alloc >= cpool->max_alloc)
		{
			// wait for something to be freed
			ep_dbg_cprintf(Dbg, 12, "conn_get: waiting (n_alloc = %d)\n",
					cpool->n_alloc);
			ep_thr_cond_wait(&cpool->cond, &cpool->mutex, NULL);
			ep_dbg_cprintf(Dbg, 12, "conn_get: continuing\n");

			// try again
			continue;
		}

		// allocate and open another connection
		ep_dbg_cprintf(Dbg, 11, "conn_get: allocating new connection\n");
		conn = (struct conn *) ep_mem_zalloc(sizeof *conn);
		conn->phase = "new";
		estat = (*cpool->open)(conn, cpool->udata);
		if (EP_STAT_ISOK(estat))
		{
			cpool->n_alloc++;
			break;
		}
		ep_dbg_cprintf(Dbg, 9, "conn_get: open failure: %s\n",
				mysql_error(conn->rconn));
		ep_mem_free(conn);
		if (failfast)
			return NULL;
		// try again
	}

fail0:
	ep_thr_mutex_unlock(&cpool->mutex);
	return conn;
}


// release a connection to a pool
static void
conn_rls(struct conn *conn, struct conn_pool *cpool)
{
	conn->phase = "idle";
	ep_thr_mutex_lock(&cpool->mutex);
	STAILQ_INSERT_TAIL(&cpool->pool, conn, next);
	ep_thr_cond_signal(&cpool->cond);
	ep_thr_mutex_unlock(&cpool->mutex);
}


// initialize a pool
static void
conn_pool_init(
			struct conn_pool *cpool,
			int maxconns,
			int maxtries,
			long backoff,
			EP_STAT (*open)(struct conn *, void *udata),
			void (*close)(struct conn *, void *udata),
			void (*reset)(struct conn *, void *udata),
			void *udata)
{
	if (maxconns < 1)
		maxconns = 1;
	if (maxtries <= 0)
		maxtries = 1;
	if (backoff <= 0)
		backoff = 1;
	memset(cpool, 0, sizeof *cpool);
	ep_thr_mutex_init(&cpool->mutex, EP_THR_MUTEX_DEFAULT);
	ep_thr_cond_init(&cpool->cond);
	STAILQ_INIT(&cpool->pool);
	cpool->max_alloc = maxconns;
	cpool->maxtries = maxtries;
	cpool->backoff = backoff;
	cpool->open = open;
	cpool->close = close;
	cpool->reset = reset;
	cpool->udata = udata;
}


// free everything in a pool
static void
conn_pool_rls(struct conn_pool *cpool)
{
	struct conn *conn;

	ep_thr_mutex_lock(&cpool->mutex);
	while ((conn = STAILQ_FIRST(&cpool->pool)) != NULL)
	{
		STAILQ_REMOVE_HEAD(&cpool->pool, next);
		(*cpool->close)(conn, cpool->udata);
		ep_mem_free(conn);
		cpool->n_alloc--;
	}
	ep_thr_mutex_unlock(&cpool->mutex);
}


/*
**  HONGDB_OPEN, _RESET, _CLOSE --- open and close database connections
*/

// actually open a connection to MariaDB/MySQL
MYSQL *
_gdp_hongd_conn_open(struct hongdargs *ha)
{
	int istat;
	MYSQL *myconn;

	ep_dbg_cprintf(Dbg, 11, "_gdp_hongd_conn_open:\n");
	myconn = mysql_init(NULL);
	if (myconn == NULL)
	{
		ep_dbg_cprintf(Dbg, 1, "_gdp_hongd_conn_open: mysql_init failure\n");
		return NULL;
	}

	// set up parameters
	if (ha == NULL)
		ha = ep_mem_zalloc(sizeof *ha);
	if (ha->dbhost == NULL)
		ha->dbhost = ep_adm_getstrparam("swarm.gdp.hongdb.host",
											GDP_DEFAULT_HONGD_HOST);
	//TODO: should parse ha->dbhost (if set) to find port number
	if (ha->dbuser == NULL)
		ha->dbuser = ep_adm_getstrparam("swarm.gdp.hongdb.user",
											GDP_DEFAULT_HONGD_USER);
	if (ha->dbpasswd == NULL)
		ha->dbpasswd = strdup(ep_adm_getstrparam("swarm.gdp.hongdb.passwd",
											GDP_DEFAULT_HONGD_PASSWD));
	if (ha->dbname == NULL)
		ha->dbname = ep_adm_getstrparam("swarm.gdp.hongdb.database",
											"gdp_hongd");
	if (ha->dbtable == NULL)
		ha->dbtable = ep_adm_getstrparam("swarm.gdp.hongdb.table",
											"human_to_gdp");
	{
		const char *my_proto = ep_adm_getstrparam("swarm.gdp.hongd.proto",
											GDP_DEFAULT_HONGD_PROTO);

		enum mysql_protocol_type proto_type;
		if (strcasecmp(my_proto, "tcp") == 0)
			proto_type = MYSQL_PROTOCOL_TCP;
		else if (strcasecmp(my_proto, "socket") == 0)
			proto_type = MYSQL_PROTOCOL_SOCKET;
		else if (strcasecmp(my_proto, "pipe") == 0)
			proto_type = MYSQL_PROTOCOL_PIPE;
		else if (strcasecmp(my_proto, "memory") == 0)
			proto_type = MYSQL_PROTOCOL_MEMORY;
		else
			proto_type = MYSQL_PROTOCOL_DEFAULT;
		istat = mysql_optionsv(myconn, MYSQL_OPT_PROTOCOL,
						(void *) &proto_type);
		if (istat != 0 && ep_dbg_test(Dbg, 1))
			ep_dbg_printf("_gdp_hongd_conn_open: set MYSQL_OPT_PROTOCOL => %d\n",
						istat);
	}

	//XXX should these options be parameters?
	my_bool reconnect = ep_adm_getboolparam("swarm.gdp.hongd.reconnect", true);
	istat = mysql_optionsv(myconn, MYSQL_OPT_RECONNECT, (void *) &reconnect);
	if (istat != 0 && ep_dbg_test(Dbg, 1))
		ep_dbg_printf("_gdp_hongd_conn_open: set MYSQL_OPT_RECONNECT => %d\n",
					istat);
	istat = mysql_optionsv(myconn, MYSQL_SET_CHARSET_NAME, (void *) "utf8");
	if (istat != 0 && ep_dbg_test(Dbg, 1))
		ep_dbg_printf("_gdp_hongd_conn_open: set MYSQL_SET_CHARSET_NAME => %d\n",
					istat);

	// open a connection to the external => internal mapping database
	unsigned long db_flags = CLIENT_REMEMBER_OPTIONS;
	if (mysql_real_connect(myconn, ha->dbhost, ha->dbuser, ha->dbpasswd,
							ha->dbname, ha->dbport, NULL, db_flags) == NULL)
	{
		ep_dbg_cprintf(Dbg, 1,
				"_gdp_hongd_conn_open(%s@%s:%d db %s): cannot connect: %s\n",
				ha->dbuser, ha->dbhost, ha->dbport, ha->dbname,
				mysql_error(myconn));
		mysql_close(myconn);
		return NULL;
	}

	return myconn;
}


static EP_STAT
hongdb_open(struct conn *conn, void *udata)
{
	struct hongdargs *ha = (struct hongdargs *) udata;
	conn->phase = "opening";
	conn->ha = ha;
	conn->rconn = _gdp_hongd_conn_open(ha);
	if (conn->rconn == NULL)
	{
		conn->phase = "error";
		return GDP_STAT_MYSQL_ERROR;
	}
	conn->phase = "open";
	return EP_STAT_OK;
}


static void
hongdb_close(struct conn *conn, void *udata)
{
	mysql_close(conn->rconn);
	conn->rconn = NULL;
	conn->phase = "closed";
}

static void
hongdb_reset(struct conn *conn, void *udata)
{
	ep_dbg_cprintf(Dbg, 8, "hongdb_reset(%s): %s\n",
				conn->phase, mysql_error(conn->rconn));
	mysql_reset_connection(conn->rconn);
	conn->phase = "reset";
}


/*
**  _GDP_NAME_INIT, _SHUTDOWN --- initialize/shut down name subsystem
*/

static void
_gdp_name_shutdown(void)
{
	conn_pool_rls(&CPool);
}

EP_STAT
_gdp_name_init(struct hongdargs *ha)
{
	if (ha == NULL)
		ha = ep_mem_zalloc(sizeof *ha);

	// see if the user wants a root (namespace, "working directory", ...)
	if (ha->name_root != NULL)
		gdp_name_root_set(ha->name_root);
	else
		gdp_name_root_set(getenv("GDP_NAME_ROOT"));

	ep_dbg_cprintf(Dbg, 17, "_gdp_name_init: GDP_NAME_ROOT=%s\n",
			_GdpNameRoot == NULL ? "" : _GdpNameRoot);

	// make sure HONGD is configured
	if (ha->dbhost == NULL)
		ha->dbhost = ep_adm_getstrparam("swarm.gdp.hongdb.host",
											GDP_DEFAULT_HONGD_HOST);
	if (ha->dbhost == NULL)
	{
		ep_dbg_cprintf(Dbg, 1, "_gdp_name_init: no name database available\n");
		ep_app_error("Human-Oriented Name to GDPname Directory not configured");
		CPool.max_alloc = -1;
		return GDP_STAT_HONGD_UNAVAILABLE;
	}

	// maximum number of parallel connections
	int maxconn = ep_adm_getintparam("swarm.gdp.hongdb.maxconns", 3);
	int maxtries = ep_adm_getintparam("swarm.gdp.hongd.maxtries", 10);
	long backoff = ep_adm_getlongparam("swarm.gdp.hongd.backoff", 1);

	conn_pool_init(&CPool, maxconn, maxtries, backoff,
				&hongdb_open, &hongdb_close, &hongdb_reset, ha);

	// open a connection to the external => internal mapping database.
	// we do this preemptively to give an error report ASAP.
	struct conn *conn = conn_get(&CPool, true);
	if (conn == NULL)
	{
		ep_dbg_cprintf(Dbg, 1, "_gdp_name_init: cannot open HONGD database\n");
		ep_app_error("Human-Oriented Name to GDPname Directory not available");
		CPool.max_alloc = -1;
		return GDP_STAT_HONGD_UNAVAILABLE;
	}
	conn_rls(conn, &CPool);
	HongDsLive = true;

	atexit(_gdp_name_shutdown);
	return EP_STAT_OK;
}


/*
**  GDP_PRINTABLE_NAME --- make a printable GCL name from an internal name
**
**		Returns the external name buffer for ease-of-use.
*/

char *
gdp_printable_name(const gdp_name_t internal, gdp_pname_t external)
{
	EP_STAT estat = ep_b64_encode(internal, sizeof (gdp_name_t),
							external, sizeof (gdp_pname_t),
							EP_B64_ENC_URL);

	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[100];

		ep_dbg_cprintf(Dbg, 2,
				"gdp_printable_name: ep_b64_encode failure\n"
				"\tstat = %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		strcpy(external, "(unknown)");
	}
	else if (EP_STAT_TO_INT(estat) != GDP_GOB_PNAME_LEN)
	{
		ep_dbg_cprintf(Dbg, 2,
				"gdp_printable_name: ep_b64_encode length failure (%d != %d)\n",
				EP_STAT_TO_INT(estat), GDP_GOB_PNAME_LEN);
	}
	return external;
}


/*
**  GDP_PRINT_NAME --- print a GDP name to a file
*/

void
gdp_print_name(const gdp_name_t name, FILE *fp)
{
	gdp_pname_t pname;

	if (!gdp_name_is_valid(name))
		fprintf(fp, "(none)");
	else
		fprintf(fp, "%s", gdp_printable_name(name, pname));
}


/*
**  Get/Set root of GDP names.
*/

EP_STAT
gdp_name_root_set(const char *root)
{
	if (_GdpNameRoot != NULL)
		ep_mem_free(_GdpNameRoot);
	if (root == NULL || root[0] == '\0')
		_GdpNameRoot = NULL;
	else
		_GdpNameRoot = ep_mem_strdup(root);
	return EP_STAT_OK;
}

const char *
gdp_name_root_get(void)
{
	return _GdpNameRoot;
}


/*
**	GDP_PARSE_NAME --- parse a (possibily human-friendly) GDP object name
**
**		An externally printable version of an internal name must be
**		exactly GDP_GOB_PNAME_LEN (43) characters long and contain only
**		valid URL-Base64 characters.  These are base64 decoded.
**		All other names are considered human-friendly and are
**		looked up in the GDP name directory to get the internal name.
**
**		If the GDP_NAME_ROOT environment variable is set, then the
**		following algorithm is used:
**
**		(1) if the human name `hname` has a dot in it, try that name
**			first without extension.
**		(2) if not found, try GDP_NAME_ROOT.hname (whether or not there
**			is a dot in `hname`).
**		(3) if still not found and `hname` has no dot, try the unextended
**			`hname` anyway.
**
**		For a transition period, the SHA256 of the hname is tried,
**		either with or without extension depending on whether the
**		name has a dot in it.
**
**		A caller may optionally pass in an `xname` buffer pointer to
**		receive the name actually used after `GDP_NAME_ROOT`
**		extension.  It is up to the application to free that memory.
**
**		Returns a warning if the old back compatible algorithm (hash
**		the human name to get the internal name) was used.
*/

EP_STAT
gdp_name_parse(const char *hname, gdp_name_t gname, const char **xnamep)
{
	EP_STAT estat = GDP_STAT_NAME_UNKNOWN;
	char *xname = NULL;
	int xnamelen;
	EP_TIME_SPEC start_time;

	ep_time_now(&start_time);

	if (hname == NULL || hname[0] == '\0')
	{
		// no human name to parse
		estat = GDP_STAT_GDP_NAME_INVALID;
		goto done;
	}

	if (strlen(hname) == GDP_GOB_PNAME_LEN)
	{
		// see if this is a base64-encoded name
		estat = gdp_internal_name(hname, gname);
		if (EP_STAT_ISOK(estat))
		{
			estat = GDP_STAT_OK_NAME_PNAME;
			xname = ep_mem_strdup(hname);
			goto done;
		}
	}

	if (strchr(hname, '.') != NULL || _GdpNameRoot == NULL)
	{
		// have a dot or GDP_NAME_ROOT is not set --- try raw version
		estat = gdp_name_resolve(hname, gname);
		if (EP_STAT_ISOK(estat))
		{
			xname = ep_mem_strdup(hname);
			goto done;
		}
	}

	if (_GdpNameRoot != NULL)
	{
		// try the extended name
		xnamelen = strlen(_GdpNameRoot) + strlen(hname) + 2;
		xname = ep_mem_malloc(xnamelen);
		snprintf(xname, xnamelen, "%s.%s", _GdpNameRoot, hname);
		estat = gdp_name_resolve(xname, gname);
		if (EP_STAT_ISOK(estat))
			goto done;

		// final case, try the unextended, undotted hname
		if (strchr(hname, '.') == NULL)
		{
			estat = gdp_name_resolve(hname, gname);
			if (EP_STAT_ISOK(estat))
				goto done;
		}
	}

#if GDP_COMPAT_OLD_LOG_NAMES
	//XXX temporary: fall back to SHA-256(human_name)
	if (ep_adm_getboolparam("swarm.gdp.compat.lognames", true))
	{
		if (xname == NULL || strchr(hname, '.') != NULL)
			xname = ep_mem_strdup(hname);
		ep_dbg_cprintf(Dbg, 28, "gdp_name_parse: using SHA256(%s)\n", xname);
		ep_crypto_md_sha256(xname, strlen(xname), gname);
		estat = GDP_STAT_NAME_SHA;
	}
#endif

done:
	if (ep_dbg_test(Dbg, 12))
	{
		gdp_pname_t pname;
		char ebuf[100];
		ep_dbg_printf("gdp_name_parse: %s", hname);
		if (xname != NULL)
			ep_dbg_printf("\n       xname => %s", xname);
		if (!EP_STAT_ISFAIL(estat))
			ep_dbg_printf("\n       pname => %s\n",
					gdp_printable_name(gname, pname));
		else
			ep_dbg_printf(":\n               %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	_gdp_show_elapsed("gdp_name_parse", 0, &start_time);
	if (xnamep == NULL || EP_STAT_ISFAIL(estat))
	{
		if (xname != NULL)
			ep_mem_free(xname);
		xname = NULL;
	}
	if (xnamep != NULL)
		*xnamep = xname;

	return estat;
}

EP_STAT
gdp_parse_name(const char *hname, gdp_name_t gname)
{
	// back compat
	EP_STAT estat;
	estat = gdp_name_parse(hname, gname, NULL);

	// GDP_STAT_NAME_SHA is a warning severity:
	if (EP_STAT_ISWARN(estat))
		estat = EP_STAT_OK;
	return estat;
}


/*
**	GDP_NAME_IS_VALID --- test whether a GDP object name is valid
**
**		Unfortunately, since SHA-256 is believed to be surjective
**		(that is, all values are possible), there is a slight
**		risk of a collision.
**
**		Arguably this should also check for `_GdpMyRoutingName`
**		and `RoutingLayerAddr`.
*/

bool
gdp_name_is_valid(const gdp_name_t name)
{
	const uint32_t *up;
	unsigned int i;

	up = (uint32_t *) name;
	for (i = 0; i < sizeof (gdp_name_t) / 4; i++)
		if (*up++ != 0)
			return true;
	return false;
}


/*
**  Convert a base-64 encoded GDP name to internal representation.
*/

EP_STAT
gdp_internal_name(const gdp_pname_t external, gdp_name_t internal)
{
	EP_STAT estat;

	if (strlen(external) != GDP_GOB_PNAME_LEN)
	{
		estat = GDP_STAT_GDP_NAME_INVALID;
	}
	else
	{
		estat = ep_b64_decode(external, sizeof (gdp_pname_t) - 1,
							internal, sizeof (gdp_name_t),
							EP_B64_ENC_URL);
	}

	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[100];

		ep_dbg_cprintf(Dbg, 22,
				"gdp_internal_name: ep_b64_decode failure\n"
				"\tname = %s\n"
				"\tstat = %s\n",
				external,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	else if (EP_STAT_TO_INT(estat) != sizeof (gdp_name_t))
	{
		ep_dbg_cprintf(Dbg, 22,
				"gdp_internal_name: ep_b64_decode length failure (%d != %zd)\n",
				EP_STAT_TO_INT(estat), sizeof (gdp_name_t));
		estat = EP_STAT_ABORT;
	}

	return estat;
}


/*
**  GDP_NAME_RESOLVE --- resolve a human name to an internal name
**
**		This uses the external human-to-internal name database.
*/

static EP_STAT
try_name_query(struct conn *conn, const char *hname, gdp_name_t gname)
{
	EP_STAT estat = GDP_STAT_OK_NAME_HONGD;
	struct hongdargs *ha = conn->ha;

	ep_dbg_cprintf(Dbg, 19, "try_name_query(%s)\n", hname);

	if (!HongDsLive)
	{
		ep_dbg_cprintf(Dbg, 19, "    ... no database\n");
		estat = GDP_STAT_HONGD_UNAVAILABLE;
		goto fail0;
	}

	conn->phase = "query";
	{
		int hname_len = strlen(hname);
		char escaped_hname[2 * hname_len + 1];
		mysql_real_escape_string(conn->rconn, escaped_hname, hname, hname_len);
		int dbtable_len = strlen(ha->dbtable);
		char escaped_dbtable[2 * dbtable_len + 1];
		mysql_real_escape_string(conn->rconn, escaped_dbtable, ha->dbtable,
								dbtable_len);
		const char *q = "SELECT gname FROM `%s` WHERE hname = '%s' LIMIT 1;";
		char qbuf[strlen(q) + sizeof escaped_dbtable + sizeof escaped_hname + 1];
		snprintf(qbuf, sizeof qbuf, q, escaped_dbtable, escaped_hname);
		if (mysql_query(conn->rconn, qbuf) != 0)
		{
			ep_dbg_cprintf(Dbg, 1, "MySQL >>> %s\n", qbuf);
			goto fail1;
		}
	}

	conn->phase = "results";
	MYSQL_RES *res = mysql_store_result(conn->rconn);
	if (res == NULL)
		goto fail1;

	conn->phase = "fetch";
	MYSQL_ROW row = mysql_fetch_row(res);
	if (row == NULL)
	{
		estat = GDP_STAT_NAME_UNKNOWN;
	}
	else
	{
		unsigned long *len = mysql_fetch_lengths(res);
		if (len[0] == sizeof (gdp_name_t))
		{
			memcpy(gname, row[0], sizeof (gdp_name_t));
		}
		else
		{
			ep_dbg_cprintf(Dbg, 1,
					"try_name_query(%s): field len %ld, should be %zd\n",
					hname, len[0], sizeof (gdp_name_t));
			estat = GDP_STAT_NAME_UNKNOWN;
		}
	}

	mysql_free_result(res);

	if (false)
	{
fail1:
		ep_dbg_cprintf(Dbg, 1,
				"try_name_query(%s): %s\n",
				conn->phase, mysql_error(conn->rconn));
		if (EP_STAT_ISOK(estat))
			estat = GDP_STAT_MYSQL_ERROR;
	}
fail0:
	if (ep_dbg_test(Dbg, 19))
	{
		char ebuf[100];
		ep_dbg_printf("try_name_query => %s\n",
			ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return estat;
}

EP_STAT
gdp_name_resolve(const char *hname, gdp_name_t gname)
{
	struct conn_pool *cpool = &CPool;
	struct conn *conn;
	EP_STAT estat = GDP_STAT_NAME_UNKNOWN;
	int tries;

	if (cpool->max_alloc <= 0)
	{
		// couldn't initialize, just give up
		return GDP_STAT_HONGD_UNAVAILABLE;
	}
	conn = conn_get(cpool, false);
	EP_ASSERT_ELSE(conn != NULL, return GDP_STAT_HONGD_UNAVAILABLE);

	// loop around possible failure in database connection
	for (tries = 0; tries < cpool->maxtries; tries++)
	{
		estat = try_name_query(conn, hname, gname);
		if (!EP_STAT_IS_SAME(estat, GDP_STAT_MYSQL_ERROR))
			break;

		// exponential backoff (assumes tries isn't too large)
		ep_time_nanosleep((INT64_C(1) << MIN(tries, 30)) * cpool->backoff MILLISECONDS);

		// try resetting the connection
		(*cpool->reset)(conn, cpool->udata);
	}

	conn_rls(conn, cpool);
	return estat;
}


/*
**  Get the human name from the internal name in HONGD
*/

EP_STAT
_gdp_name_gethname(gdp_name_t gname, char *hnbuf, size_t hnblen)
{
	struct conn_pool *cpool = &CPool;
	struct conn *conn = NULL;
	EP_STAT estat = EP_STAT_OK;

	if (!HongDsLive)
	{
		ep_dbg_cprintf(Dbg, 19, "_gdp_name_gethname: no database\n");
		estat = GDP_STAT_HONGD_UNAVAILABLE;
		goto fail0;
	}
	if (cpool->max_alloc <= 0)
	{
		// couldn't initialize, just give up
		ep_dbg_cprintf(Dbg, 19, "_gdp_name_gethname: no connections in pool\n");
		estat = GDP_STAT_HONGD_UNAVAILABLE;
		goto fail0;
	}
	conn = conn_get(cpool, false);
	EP_ASSERT_ELSE(conn != NULL, return GDP_STAT_HONGD_UNAVAILABLE);
	struct hongdargs *ha = conn->ha;

	conn->phase = "query";
	{
		char hexbuf[2 * sizeof (gdp_name_t) + 1];
		int i;
		for (i = 0; i < sizeof (gdp_name_t); i++)
			snprintf(&hexbuf[i * 2], sizeof hexbuf, "%02x", gname[i]);

		int dbtable_len = strlen(ha->dbtable);
		char escaped_dbtable[2 * dbtable_len + 1];
		mysql_real_escape_string(conn->rconn, escaped_dbtable, ha->dbtable,
								dbtable_len);
		const char *q = "SELECT hname FROM `%s`\n"
						" WHERE gname = CAST(X'%s' AS BINARY(32))\n"
						" LIMIT 1;";
		char qbuf[strlen(q) + sizeof escaped_dbtable + sizeof hexbuf + 1];
		snprintf(qbuf, sizeof qbuf, q, escaped_dbtable, hexbuf);
		ep_dbg_cprintf(Dbg, 30, "_gdp_name_gethname: %s\n", qbuf);
		if (mysql_query(conn->rconn, qbuf) != 0)
		{
			ep_dbg_cprintf(Dbg, 1, "_gdp_name_gethname: mysql(%s)\n", qbuf);
			goto fail1;
		}
	}

	conn->phase = "results";
	{
		MYSQL_RES *res = mysql_store_result(conn->rconn);
		if (res == NULL)
		{
			ep_dbg_cprintf(Dbg, 19, "_gdp_name_gethname: no results\n");
			goto fail1;
		}
		conn->phase = "fetch";
		MYSQL_ROW row = mysql_fetch_row(res);
		if (row == NULL)
		{
			ep_dbg_cprintf(Dbg, 19, "_gdp_name_gethname: cannot find name: %s\n",
							mysql_error(conn->rconn));
			estat = GDP_STAT_NAME_UNKNOWN;
		}
		else
		{
			unsigned long *len = mysql_fetch_lengths(res);
			snprintf(hnbuf, hnblen, "%.*s", (int) len[0], row[0]);
		}

		mysql_free_result(res);
	}

	if (false)
	{
fail1:
		ep_dbg_cprintf(Dbg, 1,
				"_gdp_name_gethname(%s): %s\n",
				conn->phase, mysql_error(conn->rconn));
		if (EP_STAT_ISOK(estat))
			estat = GDP_STAT_MYSQL_ERROR;
	}
fail0:
	if (ep_dbg_test(Dbg, 19))
	{
		char ebuf[100];
		ep_dbg_printf("_gdp_name_gethname => %s\n",
			ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}

	if (conn != NULL)
		conn_rls(conn, cpool);
	return estat;
}
