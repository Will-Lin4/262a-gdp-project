/* vim: set ai sw=4 sts=4 ts=4 : */


/*
**  You'll need to get INIH from Github for this:
**		git clone https://github.com/benhoyt/inih
*/



#include <gdp/gdp.h>
#include <gdp/gdp_event.h>

#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <ep/ep_hash.h>
#include <ep/ep_string.h>

#include <sysexits.h>


/*
**  OVERVIEW: A stress test consists of one or more _jobs_.
**
**  Each job represents some number of threads, each of which
**  runs a _batch_.  Batches can be _read batches_ or _write
**  batches_.
**
**  A write batch is a single thread that produces synthetic data
**  using a _datagen_ and writes to an associated log using a
**  method, e.g., synchronous versus asynchronous writes.
**
**  A read batch reads data using some method (e.g., synchronous
**  read, asynchronous read, multiread, or subscribe).  Ideally
**  it would compare the data against the expected data.
**
**  It's possible for each thread to write to a separate log,
**  or to have all threads in a job write to a single log.
**
**  There are many parameters to tweak the process.  For
**  example, if using an asynchronous write method, you can
**  set how often to check for responses (e.g., does each
**  write operation try to collect prior results, or should it
**  write N records before polling for results?).
*/


typedef struct datagen		datagen_t;		// datagen instance

typedef struct batch		batch_t;		// batch instance


/*
**  Data Generator
**
**		Produces a stream of data.  The `next` function does the
**		heavy lifting.
**
**		This keeps track of the number of records returned so far.
**		`max_gen` can be set to put an absolute maximum on the
**		number of records this can generate.
*/

struct datagen
{
	EP_THR_MUTEX	mutex;
	EP_STAT			(*init)(			// initialize data generator
							datagen_t *dg,
							batch_t *bi);
	EP_STAT			(*next)(			// get next datum from generator
							datagen_t *dg,
							batch_t *bi,
							gdp_datum_t *datum);
	long			payload_size;		// size of payload
	long			max_gen;			// max # recs to produce
	long			n_gen;				// # recs already produced
};


/*
**  Log Controller
**
**		This pairs 1::1 with logs.  It is primarily responsible for
**		collecting responses to asynchronous writes.  We can't do
**		this in the Batch Instance since multiple BIs might write a
**		single log.
*/

typedef struct logctl		logctl_t;
struct logctl
{
	EP_THR_MUTEX	mutex;
	const char		*gobxname;			// external name for log
	gdp_gin_t		*gin;				// underlying log
	long			n_out;				// number of records written
	long			n_resp;				// number of responses read
};


/*
**  Batches
**
**		Each batch represents a thread.  For writes, each thread reads
**		from a datagen and writes to a logctl.
**
**		Since these match 1::1 with threads, we don't need locks.
*/

struct batch
{
	const char		*btype;				// printable batch type
	const char		*bname;				// for printing
	EP_STAT			(*run)(				// run the batch
							batch_t *batch);
	long			batch_size;			// number of records in this batch

	datagen_t		*dg;				// data generator for this batch
	logctl_t		*lc;				// log controller
	int				threadno;			// thread number (integer)
	int				results_interval;	// how often to get async results
	int				record_interval;	// time between writing records (msec)
	int				record_ifuzz;		// fuzz factor on record_interval
	bool			verbose:1;			// print out progress status
};



#define STAT_LOST_RESPONSE	EP_STAT_NEW(EP_STAT_SEV_ERROR, EP_REGISTRY_USER, 0, 1)

static struct ep_stat_to_string	UserStats[] =
{
	{ STAT_LOST_RESPONSE,			"lost asynchronous response",			},
	{ EP_STAT_OK,					NULL,									}
};


#define MILLISECONDS	* INT64_C(1000000)

/**********************************************************************
**
**  Batch methods
*/


/*
**  Helper routine for collecting all asynchronous results
*/

void
collect_async_results(logctl_t *lc, long timeout)
{
	int prflags = GDP_DATUM_PRTEXT;
	int n_to_collect;
	gdp_gin_t *gin;
	EP_TIME_SPEC event_timeout;
	ep_time_from_nsec(timeout MILLISECONDS, &event_timeout);

	if (lc == NULL)
	{
		// use some defaults (for final cleanup)
		n_to_collect = 100;
		gin = NULL;
	}
	else
	{
		n_to_collect = lc->n_out - lc->n_resp;
		gin = lc->gin;
	}

	while (n_to_collect > 0)
	{
		gdp_event_t *ev;

		// poll to see if there are any events available
		ev = gdp_event_next(gin, &event_timeout);
		if (ev == NULL)
			break;

		int evtype = gdp_event_gettype(ev);
		const char *bname = (const char *) gdp_event_getudata(ev);
		switch (evtype)
		{
		case GDP_EVENT_DATA:
			if (lc != NULL)
				lc->n_resp++;
			n_to_collect--;
			printf("%s: ", bname);
			gdp_datum_print(gdp_event_getdatum(ev), stdout, prflags);
			break;

		case GDP_EVENT_CREATED:
			if (lc != NULL)
				lc->n_resp++;
			n_to_collect--;
			printf("%s: Data created\n", bname);
			break;

		default:
			printf("%s: ", bname);
			gdp_event_print(ev, stdout, GDP_PR_BASIC + 2, 0);
			break;
		}
	}
}


void
record_delay(int record_interval, int record_ifuzz)
{
	if (record_interval <= 0)
		return;

	int64_t delay;
	int32_t r = random();			// 0 to 2^31-1
	r -= INT32_MAX / 2;				// -2^30 to 2^30-1
	float fuzz = r;
	fuzz /= INT32_MAX / 2;			// -1 to +1

	delay = (record_interval MILLISECONDS) + (record_ifuzz MILLISECONDS) * fuzz;
	ep_time_nanosleep(delay);
}


/*
**  Synchronous writes
*/

EP_STAT
write_batch_synchronous(batch_t *bi)
{
	EP_STAT estat;
	datagen_t *dg = bi->dg;
	logctl_t *lc = bi->lc;
	long n_gen = 0;
	gdp_datum_t *datum = gdp_datum_new();

	while (EP_STAT_ISOK(estat = (*dg->next)(dg, bi, datum)))
	{
		ep_thr_mutex_lock(&lc->mutex);
		estat = gdp_gin_append(lc->gin, datum, NULL);
		if (EP_STAT_ISOK(estat))
		{
			lc->n_out++;
			lc->n_resp++;
		}
		ep_thr_mutex_unlock(&lc->mutex);
		if (!EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			printf("gdp_gin_append error: %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			break;
		}
		if (++n_gen >= bi->batch_size)
			break;

		record_delay(bi->record_interval, bi->record_ifuzz);
	}

	// end of data is not an error
	if (EP_STAT_IS_SAME(estat, EP_STAT_END_OF_FILE))
		estat = EP_STAT_OK;

	gdp_datum_free(datum);
	return estat;
}


/*
**  Asynchronous writes
*/

EP_STAT
write_batch_asynchronous(batch_t *bi)
{
	EP_STAT estat;
	datagen_t *dg = bi->dg;
	logctl_t *lc = bi->lc;
	long n_gen = 0;
	gdp_datum_t *datum = gdp_datum_new();

	while (EP_STAT_ISOK(estat = (*dg->next)(dg, bi, datum)))
	{
		ep_thr_mutex_lock(&lc->mutex);
		estat = gdp_gin_append_async(lc->gin, 1, &datum,
								NULL, (void *) bi->bname, NULL);
		if (EP_STAT_ISOK(estat))
		{
			lc->n_out++;
			if ((lc->n_out - lc->n_resp) > bi->results_interval)
				collect_async_results(lc, 0);
		}
		ep_thr_mutex_unlock(&lc->mutex);

		if (!EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			printf("gdp_gin_append_async error: %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			break;
		}
		if (++n_gen >= bi->batch_size)
			break;

		record_delay(bi->record_interval, bi->record_ifuzz);
	}

	ep_thr_mutex_lock(&lc->mutex);
	if (lc->n_out > lc->n_resp)
	{
		ep_time_nanosleep(200 MILLISECONDS);
		collect_async_results(lc, 0);
	}
	if (lc->n_out > lc->n_resp)
	{
		estat = STAT_LOST_RESPONSE;
		ep_app_message(estat,
				"write_batch_asynchronous: wrote %ld, got %ld results",
				lc->n_out, lc->n_resp);
	}
	ep_thr_mutex_unlock(&lc->mutex);

	gdp_datum_free(datum);
	if (EP_STAT_IS_SAME(estat, EP_STAT_END_OF_FILE))
		estat = EP_STAT_OK;
	return estat;
}


/**********************************************************************
**
**  Modules to read from logs
*/


/*
**  Synchronous reads
*/

EP_STAT
read_batch_synchronous(batch_t *bi)
{
	EP_STAT estat;
	logctl_t *lc = bi->lc;
	gdp_datum_t *datum = gdp_datum_new();
	int n_read = 0;
	int prflags = GDP_DATUM_PRTEXT;

	while (++n_read <= bi->batch_size)
	{
		ep_thr_mutex_lock(&lc->mutex);
		estat = gdp_gin_read_by_recno(lc->gin, n_read, datum);
		ep_thr_mutex_unlock(&lc->mutex);
		if (!EP_STAT_ISOK(estat))
			break;
		printf("%s: ", bi->bname);
		gdp_datum_print(datum, stdout, prflags);
	}

	gdp_datum_free(datum);
	return estat;
}


/*
**  Asynchronous reads
*/

EP_STAT
read_batch_asynchronous(batch_t *bi)
{
	EP_STAT estat = EP_STAT_OK;
	logctl_t *lc = bi->lc;
	int n_read = 0;

	while (++n_read <= bi->batch_size)
	{
		ep_thr_mutex_lock(&lc->mutex);
		estat = gdp_gin_read_by_recno_async(lc->gin, 0, n_read,
								NULL, (void *) bi->bname);
		if (EP_STAT_ISOK(estat))
		{
			lc->n_out++;
			if ((lc->n_out - lc->n_resp) > bi->results_interval)
				collect_async_results(lc, 0);
		}
		ep_thr_mutex_unlock(&lc->mutex);

		if (!EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			printf("gdp_gin_read_by_recno_async error: %s\n",
					ep_stat_tostr(estat, ebuf, sizeof ebuf));
			break;
		}
	}
	ep_thr_mutex_lock(&lc->mutex);
	if (lc->n_out > lc->n_resp)
	{
		ep_time_nanosleep(200 MILLISECONDS);
		collect_async_results(lc, 0);
	}
	if (lc->n_out > lc->n_resp)
	{
		estat = STAT_LOST_RESPONSE;
		ep_app_message(estat,
				"read_batch_asynchronous: asked for %ld, got %ld results",
				lc->n_out, lc->n_resp);
	}
	ep_thr_mutex_unlock(&lc->mutex);

	return estat;
}


/*
**  Multiread
*/

EP_STAT
read_batch_multiread(batch_t *bi)
{
	EP_STAT estat;
	logctl_t *lc = bi->lc;

	ep_thr_mutex_lock(&lc->mutex);
	estat = gdp_gin_multiread(lc->gin, -bi->batch_size, bi->batch_size,
							NULL, (void *) bi->bname);
	lc->n_out += bi->batch_size;
	ep_thr_mutex_unlock(&lc->mutex);

	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[100];
		printf("gdp__multiread error: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		goto fail0;
	}

	ep_time_nanosleep(200 MILLISECONDS);
	ep_thr_mutex_lock(&lc->mutex);
	collect_async_results(lc, 10);

	if (bi->batch_size > lc->n_resp)
	{
		estat = STAT_LOST_RESPONSE;
		ep_app_message(estat,
				"read_batch_multiread: asked for %ld, got %ld results",
				lc->n_out, lc->n_resp);
	}
	ep_thr_mutex_unlock(&lc->mutex);

fail0:
	return estat;
}


/*
**  Subscription
*/

EP_STAT
read_batch_subscribe(batch_t *bi)
{
	EP_STAT estat;
	logctl_t *lc = bi->lc;

	ep_thr_mutex_lock(&lc->mutex);
	estat = gdp_gin_subscribe_by_recno(lc->gin, 0, bi->batch_size,
							NULL, NULL, (void *) bi->bname);
	lc->n_out += bi->batch_size;		// assume we'll get results for all
	ep_thr_mutex_unlock(&lc->mutex);

	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[100];
		printf("gdp_gin_subscribe_by_recno error: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
		goto fail0;
	}

	ep_time_nanosleep(200 MILLISECONDS);
	ep_thr_mutex_lock(&lc->mutex);
	long results_timeout = 1000;
	if (bi->record_interval > 0)
		results_timeout = bi->record_interval * 2;
	collect_async_results(lc, results_timeout);
	//sleep(2);
	//collect_async_results(lc, results_timeout);

	if (bi->batch_size > lc->n_resp)
	{
		estat = STAT_LOST_RESPONSE;
		ep_app_message(estat,
				"read_batch_subscribe: expected %ld results, got %ld",
				bi->batch_size, lc->n_resp);
	}
	ep_thr_mutex_unlock(&lc->mutex);

fail0:
	return estat;
}



/**********************************************************************
**
**  Data generators
*/

EP_STAT
dg_sequential_init(datagen_t *dg, batch_t *bi)
{
	return EP_STAT_OK;
}


EP_STAT
dg_sequential_next(datagen_t *dg, batch_t *bi, gdp_datum_t *datum)
{
	// check to see if we should stop generating records
	ep_thr_mutex_lock(&dg->mutex);
	if (dg->n_gen >= dg->max_gen)
	{
		ep_thr_mutex_unlock(&dg->mutex);
		return EP_STAT_END_OF_FILE;
	}
	long recno = ++dg->n_gen;
	ep_thr_mutex_unlock(&dg->mutex);

	gdp_buf_t *db = gdp_datum_getbuf(datum);
	gdp_buf_reset(db);
	gdp_buf_printf(db, "%8d %3d  ", recno, bi->threadno);
	int nleft = dg->payload_size - gdp_buf_getlength(db);
	while (nleft > 0)
	{
		// filler can be most anything
		const char filler[] = "*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_";
		int chunksize = sizeof filler - 1;
		if (chunksize > nleft)
			chunksize = nleft;

		gdp_buf_printf(db, "%.*s", chunksize, filler);
		nleft -= chunksize;
	}

	return EP_STAT_OK;
}



/**********************************************************************
**
**  Log controller support
*/


/*
**  Create a new name with interpolations.
**
**		Intended for use building log names, but could be more generic.
**
**		The template name is copied as is except for "$" escapes,
**		which are interpolated as follows:
**
**			$$	literal $
**			$T	thread id (as an integer)
**
**		Other escapes silently disappear.
**
**		"%" might be a better escape character if this is used from the
**		command line.
*/

#define ESCAPE_CHAR		'$'

const char *
xlate_name(const char *template, batch_t *bi)
{
	char nbuf[1024];
	char *np = nbuf;
	const char *tp = template;

	while (*tp != '\0' && np < &nbuf[sizeof nbuf - 1])
	{
		char tidbuf[20];
		if (*tp != ESCAPE_CHAR)
		{
			*np++ = *tp++;
			continue;
		}

		// it's some interpolation
		switch (*++tp)
		{
		case ESCAPE_CHAR:
			*np++ = ESCAPE_CHAR;
			break;

		case 'T':
			snprintf(tidbuf, sizeof tidbuf, "%d", bi->threadno);
			char *p = tidbuf;
			while (*p != '\0' && np < &nbuf[sizeof nbuf - 1])
				*np++ = *p++;
			break;
		}
	}
	if (np > &nbuf[sizeof nbuf - 1])
		np = &nbuf[sizeof nbuf - 1];
	*np = '\0';

	return ep_mem_strdup(nbuf);
}


/*
**  Get a log controller for this log
**
**		Given a log name (as a template) and a batch instance,
**		get the corresponding log controller structure with the
**		log already opened.
**		If two log names are the same, we'll get the same
**		log controller.
*/

#define LOGCTL_INIT_ATTEMPTS		5	// 100ms intervals

EP_HASH			*LogControllers;
EP_THR_MUTEX	LogControllersMutex		EP_THR_MUTEX_INITIALIZER;


/*
**  Get the log controller struct (with an open log) for a given log.
**
**		This is a bit tricky since a lot of threads might be
**		active at once, so we have to be careful about locking.
**		There's a global lock around the shared data structure
**		and a lock for each log.
*/

EP_STAT
get_logctl(const char *logtemplate, batch_t *bi, logctl_t **plc)
{
	EP_STAT estat;
	const char *logname = xlate_name(logtemplate, bi);
	gdp_name_t gname;

	// we'll use the internal name since it is canonical
	estat = gdp_parse_name(logname, gname);

	EP_STAT_CHECK(estat, goto fail0);
	// see if we already have this log
	ep_thr_mutex_lock(&LogControllersMutex);

	// initialization
	if (LogControllers == NULL)
		LogControllers = ep_hash_new("LogControllers", NULL, 0);

	logctl_t *lc = ep_hash_search(LogControllers, sizeof gname, gname);
	if (lc == NULL)
	{
		// create a new log controller
		lc = ep_mem_zalloc(sizeof *lc);
		ep_thr_mutex_init(&lc->mutex, EP_THR_MUTEX_DEFAULT);
		lc->gobxname = logname;
		ep_hash_insert(LogControllers, sizeof gname, gname, lc);
	}
	ep_thr_mutex_lock(&lc->mutex);

	// if we are the initializing thread, get on with it
	if (lc->gin == NULL)
	{
		// try to open the log
		estat = gdp_gin_open(gname, GDP_MODE_ANY, NULL, &lc->gin);

		//XXX create it if it doesn't exist?

		EP_STAT_CHECK(estat, goto fail1);
	}

	if (EP_STAT_ISOK(estat))
		*plc = lc;
fail1:
	ep_thr_mutex_unlock(&lc->mutex);
fail0:
	ep_thr_mutex_unlock(&LogControllersMutex);
	return estat;
}


/**********************************************************************
**
**  Stress run startup.
*/

void *
do_run(void *bi_)
{
	batch_t *bi = bi_;
	EP_STAT estat = bi->run(bi);

	collect_async_results(NULL, 2000);
	if (bi->verbose)
		ep_app_message(estat, "batch %s terminated", bi->bname);
	return (void *) (uintptr_t) EP_STAT_TO_INT(estat);
}

EP_STAT
start_run(batch_t *bi_template, int nthreads, EP_THR *threads)
{
	EP_STAT estat = EP_STAT_OK;
	int i;

	for (i = 0; i < nthreads; i++)
	{
		batch_t *bi;
		int istat;
		char bnamebuf[100];

		// create a per-thread version of the batch info
		bi = ep_mem_malloc(sizeof *bi);
		memcpy(bi, bi_template, sizeof *bi);
		bi->threadno = i + 1;
		snprintf(bnamebuf, sizeof bnamebuf, "%s-%d", bi->btype, bi->threadno);
		bi->bname = ep_mem_strdup(bnamebuf);
		istat = ep_thr_spawn(&threads[i], do_run, bi);
		if (istat != 0 && EP_STAT_ISOK(estat))
		{
			estat = ep_stat_from_errno(istat);
			ep_app_message(estat, "could not spawn batch %d", bi->threadno);
		}
	}

	return estat;
}



/**********************************************************************
**
**  Configuration setup.
*/

#if 0
int
batch_config_handler(void *batch_,
		const char *sect,
		const char *name,
		const char *val)
{
	batch_t *batch = batch_;

	if (strcasecmp(name, "class") == 0)
		batch->class = get_batch_class(val);
	else if (strcasecmp(name, "n-recs") == 0)
		batch->n_recs = atol(val);
	else if (strcasecmp(name, "pause") == 0)
		batch->pause = atol(val);
	else if (strcasecmp(name, "results-interval") == 0)
		batch->results_interval = atol(val);
	else if (strcasecmp(name, "log-name") == 0)
		batch->logname = ep_mem_strdup(val);
	else
	{
		ep_app_error("Unknown batch configuration field \"%s\"", name);
		return 0;
	}
	return 1;
}


batch_t *
read_batch_config(const char *bfile)
{
	batch_t *batch = ep_mem_zalloc(sizeof *batch);

	ini_parse(bfile, &batch_file_handler, batch);
	return batch;
}


int
run_config_field_handler(void *cf_,
		const char *sect,
		const char *name,
		const char *val)
{
	config_t *cf = cf_;

	if (strcasecmp(sect, "") == 0)
	{
		// global values
		if (strcasecmp(name, "log-root") == 0)
			cf->log_root = ep_mem_strdup(val);
		else
		{
			ep_app_error("Unknown global run-config field \"%s\"", name);
			return 0;
		}
		return 1;
	}

	job_t *job = get_job(cf, sect);
	if (job == NULL)
		return 0;

	if (strcasecmp(name, "batch") == 0)
	{
		if ((batch = get_batch(cf, val)) == NULL)
			return 0;
		else
			job->batch = batch;
	}
	else if (strcasecmp(name, "instance") == 0)
		job->instance = ep_mem_strdup(val);
	else if (strcasecmp(name, "create-logs") == 0)
		job->create_logs = get_bool(val);		//XXX maybe global?
	else if (strcasecmp(name, "nthreads") == 0)
		job->nthreads = atol(val);
	else
	{
		ep_app_error("Unknown job field \"%s\"", name);
		return 0;
	}
	return 1;
}


config_t *
read_run_config(const char *cfile)
{
	config_t *cf = ep_mem_zalloc(sizeof *cf);

	ini_parse(cfile, &config_field_handler, cf);
	return cf;
}

#endif // 0


/**********************************************************************
**
**  Setup, configuration, etc.
*/


void
usage(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr,
			"Usage: %s [-a] [-A results-collection-interval] [-D dbgspec]\n"
			"       [-G router_addr][-i w-delay] [-m] [-n run-size]\n"
			"       [-p payload-size] [-r n-readers] [-s] [-w n-writers]\n"
			"       log_name_template\n"
			"    -a  read or write asynchronously\n"
			"    -A  set async results collection interval\n"
			"    -d  delay after writing records (msec)\n"
			"    -D  turn on debugging flags\n"
			"    -G  IP host to contact for gdp_router\n"
			"    -i  interval (delay) between writes\n"
			"    -m  use multiread for read batches\n"
			"    -n  set number of records in run\n"
			"    -p  set payload size\n"
			"    -r  set number of reader threads\n"
			"    -s  use subscriptions for reading\n"
			"    -v  operate verbosely\n"
			"    -w  set number of writer threads\n"
			"-m and -s are invalid without at least one reader thread (-r)\n"
			"-m and -s are mutually inconsistent\n"
			"-m and -s override -a for reads\n"
			"\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}


int
main(int argc, char **argv)
{
	bool async = false;
	bool multiread = false;
	bool subscribe = false;
	bool verbose = false;
	long payload_size = 32;
	long async_batch_size = 8;
	long batch_size = 32;
	long n_writers = 0;
	long n_readers = 0;
	long record_interval = 0;
	long record_ifuzz = 0;
	char *router_addr = NULL;
	char *logname_template;
	EP_STAT estat = EP_STAT_OK;
	int opt;
	const char *phase;

	while ((opt = getopt(argc, argv, "aA:D:G:i:I:mn:p:r:svw:")) > 0)
	{
		switch (opt)
		{
		case 'a':
			async = true;
			break;

		case 'A':
			async_batch_size = atol(optarg);
			break;

		case 'D':
			ep_dbg_set(optarg);
			break;

		case 'G':
			router_addr = optarg;
			break;

		case 'i':
			record_interval = atol(optarg);
			break;

		case 'I':
			record_ifuzz = atol(optarg);
			break;

		case 'm':
			multiread = true;
			break;

		case 'n':
			batch_size = atol(optarg);
			break;

		case 'p':
			payload_size = atol(optarg);
			break;

		case 'r':
			n_readers = atol(optarg);
			break;

		case 's':
			subscribe = true;
			break;

		case 'v':
			verbose = true;
			break;

		case 'w':
			n_writers = atol(optarg);
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage("Must specify one log name");
	if (n_readers + n_writers <= 0)
		usage("Must specify at least one reader or writer thread");
	if (multiread && n_readers <= 0)
		usage("-m must be used with -r");
	if (multiread && subscribe)
		usage("-m and -s are incompatible");
	logname_template = argv[0];

	// initialize GDP library
	phase = "gdp_init";
	estat = gdp_init(router_addr);
	if (!EP_STAT_ISOK(estat))
		goto fail0;
	ep_stat_reg_strings(UserStats);

	/*
	**  This should be in some sort of configuration file.
	*/

	batch_t *r_bi = NULL;
	if (n_readers > 0)
	{
		phase = "reader configuration";
		ep_app_info("Doing %s", phase);

		// this is the template for the batch
		r_bi = ep_mem_zalloc(sizeof *r_bi);

		r_bi->results_interval = async_batch_size;
		r_bi->batch_size = batch_size;
		r_bi->verbose = verbose;
		if (subscribe)
		{
			r_bi->btype = "SUB";
			r_bi->run = &read_batch_subscribe;
		}
		else if (multiread)
		{
			r_bi->btype = "RM";
			r_bi->run = &read_batch_multiread;
		}
		else if (async)
		{
			r_bi->btype = "RA";
			r_bi->run = &read_batch_asynchronous;
		}
		else
		{
			r_bi->btype = "RS";
			r_bi->run = read_batch_synchronous;
		}
		estat = get_logctl(logname_template, r_bi, &r_bi->lc);
		EP_STAT_CHECK(estat, goto fail0);
	}

	batch_t *w_bi = NULL;
	if (n_writers > 0)
	{
		phase = "writer configuration";
		ep_app_info("Doing %s", phase);

		// create a single data generator
		datagen_t *dg = ep_mem_zalloc(sizeof *dg);
		dg->max_gen = INT32_MAX;
		dg->payload_size = payload_size;
		dg->init = dg_sequential_init;
		dg->next = dg_sequential_next;
		ep_thr_mutex_init(&dg->mutex, EP_THR_MUTEX_DEFAULT);

		// this is the template for the batch
		w_bi = ep_mem_zalloc(sizeof *w_bi);

		w_bi->results_interval = async_batch_size;
		w_bi->dg = dg;
		w_bi->batch_size = batch_size;
		w_bi->record_interval = record_interval;
		w_bi->record_ifuzz = record_ifuzz;
		w_bi->verbose = verbose;
		if (async)
		{
			w_bi->btype = "WA";
			w_bi->run = &write_batch_asynchronous;
		}
		else
		{
			w_bi->btype = "WS";
			w_bi->run = &write_batch_synchronous;
		}
		estat = get_logctl(logname_template, w_bi, &w_bi->lc);
		EP_STAT_CHECK(estat, goto fail0);
	}

	/*
	**  Run that configuration (note: only runs one batch set)
	**
	**		Do readers first so subscribers will be ready.
	*/

	EP_THR *r_threads = NULL;
	if (n_readers > 0)
	{
		phase = "reader batch run";
		ep_app_info("Running %s", phase);

		r_threads = ep_mem_malloc(n_readers * sizeof *r_threads);
		estat = start_run(r_bi, n_readers, r_threads);
		EP_STAT_CHECK(estat, goto fail0);
	}

	EP_THR *w_threads = NULL;
	if (n_writers > 0)
	{
		phase = "writer batch run";
		ep_app_info("Running %s", phase);

		w_threads = ep_mem_malloc(n_writers * sizeof *w_threads);
		estat = start_run(w_bi, n_writers, w_threads);
		EP_STAT_CHECK(estat, goto fail0);
	}

	// wait for threads to complete
	phase = "wait for thread completion";
	ep_app_info("Waiting for thread completion");
	int i;
	for (i = 0; i < n_writers; i++)
	{
		EP_STAT tstat = EP_STAT_OK;
		int istat;
		istat = pthread_join(w_threads[i], (void **) &tstat);
		if (istat != 0)
		{
			tstat = ep_stat_from_errno(istat);
			ep_app_message(tstat, "pthread_join");
		}
		else
		{
//			if (verbose)
//				ep_app_message(tstat, "batch %s-%d", w_bi->btype, i);
		}
		if (EP_STAT_ISOK(estat))
			estat = tstat;
	}
	for (i = 0; i < n_readers; i++)
	{
		EP_STAT tstat = EP_STAT_OK;
		int istat;
		istat = pthread_join(r_threads[i], (void **) &tstat);
		if (istat != 0)
		{
			tstat = ep_stat_from_errno(istat);
			ep_app_message(tstat, "pthread_join");
			if (EP_STAT_ISOK(estat))
				estat = tstat;
		}
		else
		{
//			if (verbose)
//				ep_app_message(tstat, "batch %s-%d", r_bi->btype, i);
		}
		if (EP_STAT_ISOK(estat))
			estat = tstat;
	}

fail0:
	if (EP_STAT_ISOK(estat))
	{
		ep_app_message(estat, "exiting with status");
		exit(EX_OK);
	}
	else
	{
		ep_app_message(estat, "during %s", phase);
		exit(EX_UNAVAILABLE);
	}
}
