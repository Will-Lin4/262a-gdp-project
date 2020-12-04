#include <ep/ep.h>
#include <ep/ep_uuid.h>
#include "t_common_support.h"

int
main(int argc, char **argv)
{
	EP_UUID uuid;
	EP_UUID_STR uustr;
	EP_STAT estat;

	estat = ep_uuid_generate(&uuid);
	test_message(estat, "ep_uuid_create");

	estat = ep_uuid_tostr(&uuid, uustr);
	test_message(estat, "ep_uuid_tostr");
	printf("New: %s\n", uustr);
	printf("Null?: %d\n", ep_uuid_is_null(&uuid));

	estat = ep_uuid_clear(&uuid);
	test_message(estat, "ep_uuid_clear");
	estat = ep_uuid_tostr(&uuid, uustr);
	printf("Clear: %s\n", uustr);
	printf("Null?: %d\n", ep_uuid_is_null(&uuid));

	exit(0);
}
