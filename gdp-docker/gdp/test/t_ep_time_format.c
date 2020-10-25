#include <ep/ep.h>
#include <ep/ep_time.h>
#include "t_common_support.h"

int
main(int argc, char **argv)
{
	EP_TIME_SPEC now;
	EP_STAT estat;

	estat = ep_time_now(&now);
	test_message(estat, "ep_time_now");

	printf("Default: ");
	ep_time_print(&now, stdout, EP_TIME_FMT_HUMAN);
	printf("\nSIGFIG0: ");
	ep_time_print(&now, stdout, EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG0);
	printf("\nSIGFIG3: ");
	ep_time_print(&now, stdout, EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG3);
	printf("\nSIGFIG6: ");
	ep_time_print(&now, stdout, EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG6);
	printf("\nSIGFIG9: ");
	ep_time_print(&now, stdout, EP_TIME_FMT_HUMAN | EP_TIME_FMT_SIGFIG9);
	printf("\n");
	exit(0);
}
