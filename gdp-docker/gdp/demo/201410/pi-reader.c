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
**  PI-READER --- reads a GCL and triggers a LED
**
**	Reads a particular log for 0/1 input and tracks that to
**	a visual indicator.
**
**	Assumes a LED attached to GPIO 17 (Pin P1-11 on Pi-B)
**	through a 220 ohm resistor to ground.
*/

#define LOG_NAME	"demo/201410/pi"
#define LEDPIN		17

FILE	*LogFile;
int	LedPin = LEDPIN;

int
main(int argc, char **argv)
{
	gdp_gcl_t *gcl;
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
			LedPin = atoi(optarg);
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
			"Usage: %s [-D dbgspec] [-g gclname] [-p ledpin]\n",
			ep_app_getprogname());
		exit(EX_USAGE);
	}

	// initialize wiringPi library (must be root!)
	printf("Initializing wiringPi library:\n");
	wiringPiSetupGpio();
	pinMode(LedPin, OUTPUT);

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

	// open the GCL for reading
	printf("Opening GCL for reading:\n");
	estat = gdp_gcl_open(gclname, GDP_MODE_RO, &gcl);
	if (!EP_STAT_ISOK(estat))
	{
		char sbuf[100];

		ep_app_error("Cannot open GCL:\n    %s",
			ep_stat_tostr(estat, sbuf, sizeof sbuf));
		exit(EX_NOINPUT);
	}

	// start a subscription
	printf("Subscribing to GCL:\n");
	estat = gdp_gcl_subscribe(gcl, -1, 0, NULL, NULL, NULL);
	if (!EP_STAT_ISOK(estat))
	{
		char sbuf[100];

		ep_app_error("Cannot subscribe:\n\t%s",
			ep_stat_tostr(estat, sbuf, sizeof sbuf));
		exit(EX_SOFTWARE);
	}

	// start reading events
	printf("Starting to read data:\n");
	for (;;)
	{
		gdp_event_t *gev = gdp_event_next(true);
		gdp_buf_t *b;
		json_t *json;
		size_t l;

		printf("Got datum:\n");

		// decode the event
		switch (gdp_event_gettype(gev))
		{
		  case GDP_EVENT_DATA:
		  	// track the input
			if (LogFile != NULL)
			{
				struct timeval tv;
				gettimeofday(&tv, NULL);
				fprintf(LogFile, "R %d.%06d\n",
						tv.tv_sec, tv.tv_usec);
			}

			b = gdp_datum_getbuf(gdp_event_getdatum(gev));
			l = gdp_buf_getlength(b);
			printf("  ... recno %" PRIgdp_recno ", %.*s\n",
				gdp_datum_getrecno(gdp_event_getdatum(gev)),
				(int) l, gdp_buf_getptr(b, l));
			json = json_loadb((const char *)gdp_buf_getptr(b, l),
					l, 0, NULL);
			if (json == NULL || !json_is_object(json))
			{
				ep_app_error("Cannot load JSON data");
				break;
			}

			json_t *jval = json_object_get(json, "value");
			if (jval == NULL || !json_is_boolean(jval))
			{
				ep_app_error("Invalid JSON data");
				break;
			}

			// write the output pin
			printf("Value = %d\n", json_is_true(jval));
			digitalWrite(LedPin, json_is_true(jval));

			json_decref(jval);
			json_decref(json);
			break;

		  case GDP_EVENT_EOS:
		  	// end of subscription (shouldn't happen!)
			break;
			ep_app_error("End of subscription");
			exit(EX_SOFTWARE);

		  default:
			// unknown events are ignored
			printf("Unknown event type %d\n",
					gdp_event_gettype(gev));
			break;
		}

		gdp_event_free(gev);
	}

	// should never get here
	ep_app_abort("Impossible exit");
}
