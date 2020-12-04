/* vim: set ai sw=4 sts=4 ts=4 :*/

#include <gdp/gdp.h>
#include <gdp/gdp_priv.h>
#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <ep/ep_thr.h>

#include <getopt.h>
#include <sysexits.h>


#define MAXTHREADS		20

int			NLookups = 4;

void *
test_thread(void *tdata)
{
	int tno = (int) tdata;
	int i;
	char nbuf[200];
	EP_STAT estat;

	for (i = 0; i < NLookups; i++)
	{
		gdp_name_t gname;

		snprintf(nbuf, sizeof nbuf, "test.%x%02x", tno, i);
		estat = gdp_name_resolve(nbuf, gname);
		char ebuf[100];
		printf("test %x%02x  %s\n", tno, i,
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	return NULL;
}

void
usage(void)
{
	fprintf(stderr,
			"Usage: %s [-D dbgspec] [-n nlookups] [-t nthreads]\n",
			ep_app_getprogname());
	exit(EX_USAGE);
}

int
main(int argc, char **argv)
{
	int tno;
	int nThreads = 4;
	int opt;
	bool show_usage = false;
	EP_THR thr_id[MAXTHREADS];

	gdp_init_phase_0(NULL, 0);
	_gdp_name_init(NULL);

	while ((opt = getopt(argc, argv, "D:n:t:")) > 0)
	{
		switch (opt)
		{
			case 'D':
				ep_dbg_set(optarg);
				break;

			case 'n':
				NLookups = atoi(optarg);
				break;

			case 't':
				nThreads = atoi(optarg);
				if (nThreads > MAXTHREADS)
					nThreads = MAXTHREADS;
				break;

			default:
				show_usage = true;
				break;
		}
	}
	argc -= optind;
	argv += optind;

	if (show_usage)
		usage();

	for (tno = 0; tno < nThreads; tno++)
		ep_thr_spawn(&thr_id[tno], test_thread, (void *) tno);
	for (tno = 0; tno < nThreads; tno++)
		pthread_join(thr_id[tno], NULL);
	return 0;
}
