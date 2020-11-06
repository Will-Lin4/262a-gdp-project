#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <random>

#include "gdp.h"
#include "gdp_buf.h"
#include "ep.h"
#include "ep_app.h"

#include "ServerlessClient.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;


int main(int argc, char * argv[])
{
	bool client_waiting = true;

	// Catch formatting error
	if (argc != 4)
	{
		cout << "Error in usage: wrong number of arguments" << endl;
		cout << "./dummy_client <param_bucket> <final_bucket> <parameter_file>"
			 << endl;
	}
	else
	{
		// Create the dummyclient
		string param_bucket_str(argv[1]);
		string final_bucket_str(argv[2]);
		string client_name = "dummyclient";
		string param_file(argv[3]);
		ServerlessClient client(param_bucket_str, 
			final_bucket_str, client_name, true, 100);

		// Get parameters from final
		client.get_parameters(param_file);


		EP_STAT estat;
		estat = gdp_init("gdp-03.eecs.berkeley.edu:8007");
		if (!EP_STAT_ISOK(estat))
		{
			ep_app_error("GDP Initialization failed");
			return -1;
		}

		gdp_gin_t * start_gin = nullptr;	// GDP object instance
		gdp_gin_t * final_gin = nullptr;	// GDP object instance

		client.create_capsules(start_gin, final_gin);

		// Create datacapsule to write to
		gdp_datum_t * datum = gdp_datum_new();
		//char buf[8 * 1024];
		string message = "hello world to start capsule";
		char buf[128]; 
		strcpy(buf, message.c_str());
		int l = strlen(buf);
		gdp_datum_reset(datum);
		gdp_buf_t * dbuf = gdp_datum_getbuf(datum);
		//gdp_buf_write(dbuf, buf, l);
		gdp_buf_printf(dbuf, "%s", buf);

		string start_name = client.get_start_name();
		gdp_name_t start_iname;
		gdp_parse_name(start_name.c_str(), start_iname);
		string final_name = client.get_final_name();
		gdp_name_t final_iname;
		gdp_parse_name(final_name.c_str(), final_iname);
		
		// Open GIN for writing
		estat = gdp_gin_open(start_iname, GDP_MODE_AO, NULL, &start_gin);
		if (!EP_STAT_ISOK(estat))
		{
			std::cout << "Failed to open GDP log" << std::endl;
			char ebuf[100];
			ep_app_error("%s", ep_stat_tostr(estat, ebuf, sizeof ebuf));
			exit -1;
		}

		//estat = write_record(datum, start_gin);
		estat = gdp_gin_append(start_gin, datum, NULL);
		if (!EP_STAT_ISOK(estat))
		{
			std::cout << "Write failed" << std::endl;
			char ebuf[100];
			ep_app_error("%s", ep_stat_tostr(estat, ebuf, sizeof ebuf));
		}
		//client.write_to_capsule(datum, start_gin);

		//client.write
		gdp_datum_free(datum);
		
		// subscribe to final_bucket
		// write parameters
		// Start timer
		
		// Wait for agent to write to final_bucket
		//while (client_waiting)
		{
			
		}
		// Print final contents and timer
	}
	return 0;
}

