#include <ep/ep.h>
#include <ep/ep_time.h>
#include "t_common_support.h"


static void
try(char units, const char *s)
{
	EP_TIME_SPEC ts;
	ep_time_parse_interval(s, units, &ts);
	printf("%20s (%c): ", s, units);
	ep_time_print(&ts, stdout, EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG9);
	printf("\n");
}

int
main(int argc, char **argv)
{
	try('s', "10");
	try('s', "10s");
	try('s', "10m");
	try('s', "10ms");
	try('n', "10");
	try('u', "10");
	try('l', "10");
	try('H', "1M30s");
	try('W', "1u999m1m");
	try('d', "10");
	try('s', "-10s");
	try('s', "-10u");
	try('s', "-10s10u");
	try('s', "-10s-10u");
}
