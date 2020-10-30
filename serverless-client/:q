#include <iostream>
#include <string>

#include "gdp.h"
#include "gdp_buf.h"
#include "ep.h"
#include "ep_app.h"

int main(int argc, char * argv[])
{
	using std::cout; using std::endl;
	EP_STAT estat;
	gdp_init(nullptr);
	gdp_name_t start_iname;
	gdp_parse_name("docker_test", start_iname);
	
	gdp_gin_t * start_gin;
	estat = gdp_gin_open(start_iname, GDP_MODE_AO, NULL, &start_gin);
	if (!EP_STAT_ISOK(estat))
	{
		std::cout << "Failed to open GDP log" << std::endl;
		char ebuf[100];
		ep_app_error("%s", ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	gdp_datum_t * d = gdp_datum_new();
	char buf[] = "it's working";
	gdp_buf_printf(gdp_datum_getbuf(d), "%s", buf);
	estat = gdp_gin_append(start_gin, d, NULL);
	if (!EP_STAT_ISOK(estat))
	{
		std::cout << "Write failed" << std::endl;
		char ebuf[100];
		ep_app_error("%s", ep_stat_tostr(estat, ebuf, sizeof ebuf));
	}
	//gdp_gin_read_by_recno(start_gin, 1, d);
	//gdp_datum_print(d, stdout, GDP_DATUM_PRTEXT);
	
	cout << "here";
	gdp_gin_close(start_gin);


	return 0;
}
