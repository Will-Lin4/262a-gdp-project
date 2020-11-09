#include <iostream>
#include <map>
#include <string>

#include <vector>

#include "json.hpp"
#include "gdp.h"

#ifndef GDP_AGENT_HEADER
#define GDP_AGENT_HEADER

using json = nlohmann::json;

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
/*
struct encryption_settings
{
	const char * dig_alg_name;	
	const char * key_alg_name;	
	int key_bits;				 
	const char * curve_name;	
	const char * key_curve;		
	const char * key_enc_alg_name;
};
*/

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
/*
struct gdp_bucket
{
	const std::string external_name;
	const gdp_gin_t * object;		
};

*/

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

public:
	gdp_agent();
	~gdp_agent();
	
	EP_STAT create(const std::string& ext_name);
	EP_STAT open(const std::string& ext_name);
	EP_STAT write(const std::string& ext_name, const json& input_data);
	EP_STAT read(const std::string& ext_name, gdp_recno_t recno,
                 json* output_data);

private:
    std::map<std::string, gdp_gin_t*> buckets_;

};


/*------------------------------------------------------------------------------
  Implementation of gdp_agent
------------------------------------------------------------------------------*/

// TODO: Add better error handling
//
// TODO: Implement better key management
const std::string KEY_FILE = "";                                                               

// Public Methods
gdp_agent::gdp_agent()
{
    gdp_init(nullptr);
}


gdp_agent::~gdp_agent()
{
    for (auto &it : buckets_)
        gdp_gin_close(it.second);
}


EP_STAT gdp_agent::create(const std::string& ext_name)
{
	EP_STAT estat;

    auto it = buckets_.find(ext_name);
    if (it != buckets_.end())
        return EP_STAT {EP_STAT_SEV_OK};

    gdp_create_info* create_info = gdp_create_info_new();
    estat = gdp_create_info_set_creation_service(
            create_info, "edu.berkeley.eecs.gdp.service.creation");
    //EP_STAT_CHECK(estat, goto fail0);

    EP_CRYPTO_KEY* key;
    key = ep_crypto_key_read_file(KEY_FILE.c_str(), EP_CRYPTO_KEYFORM_UNKNOWN,
                                  EP_CRYPTO_F_SECRET);
    estat = gdp_create_info_set_owner_key(create_info, key, "def");
    EP_STAT_CHECK(estat, goto fail1);

    gdp_gin_t* gin;
    estat = gdp_gin_create(create_info, ext_name.c_str(), &gin);
    EP_STAT_CHECK(estat, goto fail1);

    buckets_.emplace(ext_name, gin);

fail1:
    ep_crypto_key_free(key);
fail0:
    gdp_create_info_free(&create_info);

	return estat;
}

EP_STAT gdp_agent::open(const std::string& ext_name)
{
	EP_STAT estat;

    auto it = buckets_.find(ext_name);
    if (it != buckets_.end())
        return EP_STAT {EP_STAT_SEV_OK};

    gdp_name_t name;
    estat = gdp_parse_name(ext_name.c_str(), name);
    EP_STAT_CHECK(estat, return estat);

    gdp_open_info* open_info;
    open_info = gdp_open_info_new();

    EP_CRYPTO_KEY* key;
    key = ep_crypto_key_read_file(KEY_FILE.c_str(), EP_CRYPTO_KEYFORM_UNKNOWN,
                                  EP_CRYPTO_F_SECRET);
    estat = gdp_open_info_set_signing_key(open_info, key);

    if (EP_STAT_ISOK(estat))
    {
        gdp_gin_t* gin;
        estat = gdp_gin_open(name, GDP_MODE_RA, open_info, &gin);
        //if (EP_STAT_ISOK(estat))
            buckets_.emplace(ext_name, gin);

    }

    ep_crypto_key_free(key);
    gdp_open_info_free(open_info);

	return estat;
}


EP_STAT gdp_agent::write(const std::string& ext_name, const json& input_data)
{
	EP_STAT estat;

    auto it = buckets_.find(ext_name);
    if (it == buckets_.end())
        return EP_STAT {EP_STAT_SEV_WARN};
    gdp_gin_t* gin = it->second;

    std::vector<uint8_t> cbor = json::to_cbor(input_data);
    gdp_datum_t* datum = gdp_datum_new();
    gdp_buf_t* datum_buf = gdp_datum_getbuf(datum);

    int buf_write = gdp_buf_write(datum_buf, (void*) cbor.data(), cbor.size());
    if (buf_write == 0)
    {
        // TODO: Change with hash later
        estat = gdp_gin_append(gin, datum, nullptr); 
    }
    else 
    {
        estat = EP_STAT {EP_STAT_SEV_WARN};
    }

    gdp_datum_free(datum);
	return estat;
}


EP_STAT gdp_agent::read(const std::string& ext_name, gdp_recno_t recno,
                        json* output_data)
{
	EP_STAT estat;

    auto it = buckets_.find(ext_name);
    if (it == buckets_.end())
        return EP_STAT {EP_STAT_SEV_WARN};
    gdp_gin_t* gin = it->second;

    gdp_datum_t* datum = gdp_datum_new();
    estat = gdp_gin_read_by_recno(gin, recno, datum);
    if (EP_STAT_ISOK(estat))
    {
        gdp_buf_t* datum_buf = gdp_datum_getbuf(datum);
        size_t datum_size = gdp_datum_getdlen(datum);

        char cbor[datum_size + 1];
        cbor[datum_size] = '\0';
        gdp_buf_read(datum_buf, (void*) cbor, datum_size);

        *output_data = json::from_cbor((char*) cbor);
    }

    gdp_datum_free(datum);

	return estat;
}

#endif //GDP_AGENT_HEADER
