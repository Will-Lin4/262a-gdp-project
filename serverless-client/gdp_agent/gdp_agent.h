#include <vector>

//#include <nlohmann/json.hpp>
#include "json.hpp"
#include "gdp.h"

#ifndef GDP_AGENT_HEADER
#define GDP_AGENT_HEADER

/*------------------------------------------------------------------------------
  encryption_settings
	This struct holds all information needed to figure out how a gdp_gin_t is
	encrypted. Note that no defaults are set externally from the gdp_agent.

  fields
	const char * dig_alg_name
		Specifies message digest algorithm for signing
	const char * key_alg_name
		Specifies parameters for signing algorithm
	int key_bits
	const char * curve name
	const char * key_curve
	const char * key_enc_alg_name
		Specifies algorithm for encrypting the private key when it is written 
		to disk
------------------------------------------------------------------------------*/
struct encryption_settings
{
	const char * dig_alg_name;	
	const char * key_alg_name;	
	int key_bits;				 
	const char * curve_name;	
	const char * key_curve;		
	const char * key_enc_alg_name;
};

/*------------------------------------------------------------------------------
  gdp_bucket
	This struct forms the basis of the vector<gdp_bucket *> vector_bucket in
	the gdp_agent class. It collects necessary information pertaining to a
	given GDP DataCapsule.

  fields
	gdp_gin_t * object
		An instance of GDP DataCapsule
	gdp_name_t * internal_name
		GDP internal name of this DataCapsule
	const char * external_name
		human oriented name of this DataCapsule
	struct encryption_settings encryption
		Associated encryption settings
------------------------------------------------------------------------------*/
struct gdp_bucket
{
	gdp_gin_t * object;		
	gdp_name_t * internal_name;
	const char * external_name;
	struct encryption_settings encryption;
} typedef gdp_bucket;

/*------------------------------------------------------------------------------
  gdp_agent
	A gdp_agent is a class that handles the creation of and writing to GDP
	DataCapsules.

  fields
	std::vector<gdp_bucket *> buckets
		A vector that stores gdp_bucket structs that the gdp_agent has created
		or wishes to read from. Note, that because the gdp_agent stores a 
		pointer to it's own gdp_gin_t, multiple gdp_agents can have the same
		gdp_gin_t open at the same time.
		
  public methods
	create_bucket
		Create a new bucket based on the parameters passed. Check to see if the
		DataCapsule exists before creating a new one. If the DataCapsule exists,
		do not create a new one, but add the bucket to the buckets vector so 
		that the gdp_agent can still access the DataCapsule. Do not add buckets
		if the supplied external name is already in the buckets vector.
	write_to_bucket
		Write to a bucket in the buckets vector. The data to be written is 
		passed as an nlohmann::json object.
	read_from_bucket
		Read data from the indicated bucket and store it into the passed json
		object.
	find_bucket
		Return a pointer to the bucket with the associated external name.

  private methods
	add_new_bucket
		Called by the create_bucket method. Goes through the creation process
		for a new GDP DataCapsule, and adds the resulting bucket to the buckets
		vector.
	add_existing_bucket
		If a DataCapsule is known to exist already, create a bucket and put it 
		into the buckets vector, but open up a gdp_gin_t instance. The gdp_gin_t
		should be able to be found by the HONGDS through the external name alone.
------------------------------------------------------------------------------*/
class gdp_agent
{
private:
	void add_new_bucket(const char * ext_name, gdp_create_info_t * info, 
						struct encryption_settings enc);
	void add_existing_bucket(const char * ext_name);

public:
	// Fields
	std::vector<gdp_bucket *> buckets;

	// Methods
	gdp_agent();
	~gdp_agent();
	
	EP_STAT create_bucket(const char * ext_name, gdp_create_info_t * info,
						  struct encryption_settings enc);
	EP_STAT write_to_bucket(gdp_bucket * bucket,  nlohmann::json input_data);
	EP_STAT read_from_bucket(gdp_bucket * bucket, nlohmann::json & output_data);

	gdp_bucket * find_bucket(const char * ext_name);
};
#endif
