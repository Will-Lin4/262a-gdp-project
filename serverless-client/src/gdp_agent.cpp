#include "gdp_agent.h"

#include <iostream>
#include <string>

using json = nlohmann::json;

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

