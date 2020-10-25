#include <gdp/gdp.h>
#include <ep/ep_app.h>
#include <ep/ep_dbg.h>
#include <wiringPi.h>
#include <jansson.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <sys/time.h>

/*
**  PI-WRITER --- watches a button and generates GDP updates accordingly
**
**	Changes in the physical sensor are sent to the log.
**
**	Assumes a button attached to GPIO 18 (Pin P1-12 on Pi-B)
**	to ground.
*/

#define LOG_NAME	"demo/201410/pi"
#define BUTTONPIN	18

gdp_gcl_t *PiGcl;
FILE	*LogFile;
int	ButtonPin = BUTTONPIN;

void
changefunc(void)
{
	static int oldval = -1;
	int v = digitalRead(ButtonPin);
	EP_STAT estat;
	gdp_datum_t *datum = gdp_datum_new();
	gdp_buf_t *buf = gdp_datum_getbuf(datum);

	if (v == oldval)
		return;
	oldval = v;

	if (LogFile != NULL)
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		fprintf(LogFile, "W %d.%06d\n",
				tv.tv_sec, tv.tv_usec);
	}


	gdp_buf_printf(buf, "{ \"value\": %s }", v ? "true" : "false");
	estat = gdp_gcl_publish(PiGcl, datum);
	if (!EP_STAT_ISOK(estat))
	{
		char ebuf[100];

		printf("publish failed: %s\n",
				ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	gdp_datum_free(datum);
}


int
main(int argc, char **argv)
{
	EP_STAT estat;
	gcl_name_t gclname;
	char *gclpname = LOG_NAME;
	bool show_usage = false;
	char *log_file_name = NULL;
	int opt;

	while ((opt = getopt(argc, argv, "D:g:L:p:")) > 0)
	{
		switch (opt)
		{
		  case 'D':
			ep_dbg_set(optarg);
			break;

		  case 'g':
			gclpname = optarg;
			break;

		  case 'L':
			log_file_name = optarg;
			break;

		  case 'p':
			ButtonPin = atoi(optarg);
			break;

		  default:
			show_usage = true;
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (show_usage || argc > 0)
	{
		fprintf(stderr,
			"Usage: %s [-D dbgspec] [-g gclname] [-p buttonpin]\n",
			ep_app_getprogname());
		exit(EX_USAGE);
	}

	// initialize wiringPi library (must be root!)
	printf("Initializing wiringPi library:\n");
	wiringPiSetupGpio();
	pinMode(ButtonPin, INPUT);
	pullUpDnControl(ButtonPin, PUD_UP);

	//XXX should probably give up root privileges here

	if (log_file_name != NULL)
	{
		// open a log file (for timing measurements)
		printf("Opening log file:\n");
		LogFile = fopen(log_file_name, "a");
		if (LogFile == NULL)
			printf("Cannot open log file: %s\n", strerror(errno));
		setlinebuf(LogFile);
	}

	// initialize the GDP library
	printf("Initializing GDP library:\n");
	estat = gdp_init(NULL);
	if (!EP_STAT_ISOK(estat))
	{
		ep_app_error("GDP Initialization failed");
		exit(EX_UNAVAILABLE);
	}

	// convert the name to internal form
	printf("Converting name %s to internal form:\n", gclpname);
	estat = gdp_gcl_parse_name(gclpname, gclname);
	if (!EP_STAT_ISOK(estat))
	{
		ep_app_error("Illegal GCL name syntax:\n\t%s",
			gclpname);
		exit(EX_NOINPUT);
	}

	// open the GCL for writing
	printf("Opening GCL for writing:\n");
	estat = gdp_gcl_open(gclname, GDP_MODE_AO, &PiGcl);
	if (!EP_STAT_ISOK(estat))
	{
		char sbuf[100];

		ep_app_error("Cannot open GCL:\n    %s",
			ep_stat_tostr(estat, sbuf, sizeof sbuf));
		exit(EX_NOINPUT);
	}

	// arrange to call a function on edge triggers
	(void) wiringPiISR(ButtonPin, INT_EDGE_BOTH, &changefunc);

	while (true)
		sleep(3600);

	ep_app_abort("Impossible exit");
}
