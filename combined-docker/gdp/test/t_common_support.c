/* vim: set ai sw=4 sts=4 ts=4 : */


#include "t_common_support.h"

//static EP_DBG	Dbg = EP_DBG_INIT("t_common_support", "Support routines for tests");


void
print_event(gdp_event_t *gev)
{
	printf("%s>>> Event type %d, udata %p%s\n",
			EpVid->vidbgblue,
			gdp_event_gettype(gev), gdp_event_getudata(gev),
			EpVid->vidnorm);

	// decode it
	switch (gdp_event_gettype(gev))
	{
	  case GDP_EVENT_DATA:
		// this event contains a data return
		gdp_datum_print(gdp_event_getdatum(gev), stdout, GDP_DATUM_PRTEXT);
		break;

	  case GDP_EVENT_DONE:
		// "end of subscription": no more data will be returned
		fprintf(stderr, "End of multiread/subscription\n");
		break;

	  case GDP_EVENT_SHUTDOWN:
		// log daemon has shut down, meaning we lose our subscription
		fprintf(stderr, "log daemon shutdown\n");
		break;

	  default:
		// should be ignored, but we print it since this is a test program
		fprintf(stderr, "Unknown event type %d\n", gdp_event_gettype(gev));

		// just in case we get into some crazy loop.....
		sleep(1);
		break;
	}
}



void
test_message(EP_STAT estat, char *fmt, ...)
{
	va_list av;
	char ebuf[80];

	va_start(av, fmt);
	printf("%s%s",
			EP_STAT_ISOK(estat) ? EpVid->vidfggreen : EpVid->vidfgred,
			EpVid->vidbgblack);
	vprintf(fmt, av);
	printf(": %s%s\n", ep_stat_tostr(estat, ebuf, sizeof ebuf),
			EpVid->vidnorm);
	if (!EP_STAT_ISOK(estat))
	{
		printf("%sExiting%s\n", EpVid->vidfgred, EpVid->vidnorm);
		exit(1);
	}
}
