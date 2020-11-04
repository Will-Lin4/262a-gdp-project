#include "gdp_agent.h"


// Public Methods
gdp_agent::gdp_agent()
{

}


gdp_agent::~gdp_agent()
{

}


EP_STAT gdp_agent::create_bucket(const char * ext_name, gdp_create_info_t * info,
					  struct encryption_settings enc)
{
	EP_STAT estat;

	return estat;
}


EP_STAT gdp_agent::write_to_bucket(gdp_bucket * bucket,  nlohmann::json input_data)
{
	EP_STAT estat;

	return estat;
}


EP_STAT gdp_agent::read_from_bucket(gdp_bucket * bucket, nlohmann::json & output_data)
{
	EP_STAT estat;

	return estat;
}


gdp_bucket * gdp_agent::find_bucket(const char * ext_name)
{
	
	return nullptr;
}



// Private Methods
void add_new_bucket(const char * ext_name, gdp_create_info_t * info, 
					struct encryption_settings enc)
{

}


void add_existing_bucket(const char * ext_name)
{

}

