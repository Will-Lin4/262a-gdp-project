#ifndef SERVERLESS_CLIENT_HEADER
#define SERVERLESS_CLIENT_HEADER

#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <random>

#include "gdp.h"


/*---------------------------------------------------------------------
  Usage:
    Create a ServerlessClient object
    Call get_parameters(<vector>) to determine what initial params are
    Call start_timer to begin timing
    Call create_buckets to start DataCapsules
    Call subscribe_to_final
    Call write_params

    Wait for agent to write to final_bucket
    Print contents of final_bucket and call print_timer
---------------------------------------------------------------------*/
class ServerlessClient
{
private:
	/*---------------------------------------------------------------------
	  start_bucket
	    name of DataCapsule that holds the initial parameters needed for 
	    motion planning
	  final_bucket
	    name of DataCapsule where the final calculated path will be stored
	  start_time
	    time of DataCapsule creation
	  current_time
	    current time measured by timer
	  params
	    vector of strings for parameters for the start_bucket
	---------------------------------------------------------------------*/
	std::string start_bucket_name;
	std::string final_bucket_name;
	std::string client_name;
	std::clock_t start_time;
	std::clock_t current_time;
	std::vector<std::string> params;
	bool verbose_mode;
public:
	/*---------------------------------------------------------------------
	  Constructor
	    just assign start_bucket and final_bucket names
	---------------------------------------------------------------------*/
	ServerlessClient(
		std::string & initbucket, 
		std::string & finalbucket,
		std::string & name_of_client,	
		bool verbose = false, 
		int png_seed_val = 0)
	{
		client_name = name_of_client;

		verbose_mode = verbose;

		// Initialize
		EP_STAT estat;
		estat = gdp_init(nullptr);
		if (!EP_STAT_ISOK(estat))
		{
			ep_app_error("GDP Initialization failed");
		}
		// Create bucket names
		if (png_seed_val != 0)
			srand(png_seed_val);
		start_bucket_name = initbucket + std::to_string(rand());
		final_bucket_name = finalbucket + std::to_string(rand());
		start_bucket_name.resize(GDP_HUMAN_NAME_MAX);
		final_bucket_name.resize(GDP_HUMAN_NAME_MAX);
		const char * sname_c = start_bucket_name.c_str();
		const char * fname_c = final_bucket_name.c_str();
		if (verbose_mode)
		{
			std::cout << "Start name: " << sname_c << std::endl;
			std::cout << "Final name: " << fname_c << std::endl;
		}
		// Check names
		gdp_name_t start_gdpname;
		estat = gdp_parse_name(sname_c, start_gdpname);
		if (verbose_mode && !EP_STAT_ISOK(estat))
		{
			std::cout << "Error with name: " << start_gdpname << std::endl;
		}
		gdp_name_t final_gdpname;
		estat = gdp_parse_name(fname_c, final_gdpname);
		if (verbose_mode && !EP_STAT_ISOK(estat))
		{
			std::cout << "Error with name: " << final_gdpname << std::endl;
		}
	}

	~ServerlessClient()
	{

	}

	/*---------------------------------------------------------------------
	  start_timer
	    starts timer to measure time elapsed
	---------------------------------------------------------------------*/
	void start_timer()
	{
		start_time = std::clock();
	}

	/*---------------------------------------------------------------------
	  measure_timer
	    assigns time to current_time variable
	---------------------------------------------------------------------*/
	void measure_timer()
	{
		current_time = std::clock();
	}

	/*---------------------------------------------------------------------
	  print_timer
	    print the elapsed current value in microseconds
	---------------------------------------------------------------------*/
	void print_timer()
	{
		measure_timer();
		std::cout << "Time Elapsed: " << current_time - start_time 
			<< std::endl;
	}

	/*---------------------------------------------------------------------
	  get_parameters
	    get parameters that can be written into the start_bucket
	---------------------------------------------------------------------*/
	bool get_parameters(const std::string & input_file)
	{
		std::ifstream param_file(input_file);
		if (!param_file.good())
			return false;
		std::string current_line;
		while (std::getline(param_file, current_line))
		{
			if (current_line.size() > 0)
				params.push_back(current_line);
		}
		return true;
	}

	/*---------------------------------------------------------------------
	  create_capsules
	    method that creates the start_bucket and final_bucket DataCapsules
	    calls on gdp-creator app
	---------------------------------------------------------------------*/
	EP_STAT create_capsules(
		gdp_gin_t * start_instance, 
		gdp_gin_t * final_instance,
		gdp_create_info_t * start_info = nullptr, 
		gdp_create_info_t * final_info = nullptr)
	{
		// Create the GDP Capsule names
		EP_STAT estat;

		// Create the metadata objects if needed
		if (start_info == nullptr)
		{
			start_info = gdp_create_info_new();
			gdp_create_info_set_creator(start_info, client_name.c_str(), NULL);
		}
		if (final_info == nullptr)
		{
			final_info = gdp_create_info_new();
			gdp_create_info_set_creator(final_info, client_name.c_str(), NULL);
		}
		// Turn off encryption
		const char * dig_alg_name = "def";
		const char * key_alg_name = "def";
		int key_bits = 0;
		const char * key_curve = NULL;
		const char * key_enc_alg_name = "def";
		// gdp-create line 367
		estat = gdp_create_info_new_owner_key(start_info, dig_alg_name, 
			key_alg_name, key_bits, key_curve, key_enc_alg_name);
		estat = gdp_create_info_new_owner_key(final_info, dig_alg_name, 
			key_alg_name, key_bits, key_curve, key_enc_alg_name);
		

		// Set the creation service (TODO: automate this)
		gdp_create_info_set_creation_service(start_info,
			"edu.berkeley.eecs.gdp.service.creation");
		gdp_create_info_set_creation_service(final_info, 
			"edu.berkeley.eecs.gdp.service.creation");

		const char * start_gdpxname = start_bucket_name.c_str();
		const char * final_gdpxname = final_bucket_name.c_str();

		// Create the DataCapsules
		estat = gdp_gin_create(start_info, start_gdpxname, &start_instance);
		if (verbose_mode && !EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			std::cout << "Creation failed for start bucket" << std::endl;
			ep_app_error("%s", ep_stat_tostr(estat, ebuf, sizeof ebuf));
			//return estat;
		}
		estat = gdp_gin_create(final_info, final_gdpxname, &final_instance);
		if (verbose_mode && !EP_STAT_ISOK(estat))
		{
			char ebuf[100];
			std::cout << "Creation failed for final bucket" << std::endl;
			ep_app_error("%s", ep_stat_tostr(estat, ebuf, sizeof ebuf));
			//return estat;
		}

		return estat;
	}
	
	std::string get_start_name() { return start_bucket_name; }
	std::string get_final_name() { return final_bucket_name; }

	/*---------------------------------------------------------------------
	  subscribe_to_final
	    subscribes the client to the final_bucket DataCapsule
	    calls on gdp-reader app
	---------------------------------------------------------------------*/

	/*---------------------------------------------------------------------
	  write_to_capsule
		Write the given datum to the given capsule instance
	---------------------------------------------------------------------*/
	EP_STAT write_to_capsule(gdp_datum_t * datum, gdp_gin_t * gin)
	{
		EP_STAT estat;

		estat = gdp_gin_append(gin, datum, NULL);
		if (verbose_mode && !EP_STAT_ISOK(estat))
		{
			std::cout << "Write failed" << std::endl;
			char ebuf[100];
			ep_app_error("%s", ep_stat_tostr(estat, ebuf, sizeof ebuf));
		}
		
		return estat;
	}
};

#endif
