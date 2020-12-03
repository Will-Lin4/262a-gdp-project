#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <fstream>

extern "C" {
    #include <gdp/gdp.h>
}

EP_STAT open(char* bucket_name, gdp_gin_t** out) {
    EP_STAT estat;

    gdp_name_t name;
    estat = gdp_parse_name(bucket_name, name);
    EP_STAT_CHECK(estat, return estat);

    gdp_open_info* open_info = gdp_open_info_new();

    EP_CRYPTO_KEY* key;
    key = ep_crypto_key_read_file("", EP_CRYPTO_KEYFORM_UNKNOWN,
                                  EP_CRYPTO_F_SECRET);
    estat = gdp_open_info_set_signing_key(open_info, key);
    //EP_STAT_CHECK(estat, return estat);

    gdp_gin_t* gin;
    return gdp_gin_open(name, GDP_MODE_RA, open_info, out);
}

void handle_packet(gdp_event_t *gev) {
    gdp_datum_t* datum = gdp_event_getdatum(gev);
    gdp_gin_t* output = (gdp_gin_t*) gdp_event_getudata(gev);
    gdp_gin_append(output, datum, nullptr);
}

EP_STAT route(char* bucket_name, gdp_gin_t* output) {
    gdp_gin_t* input;
    open(bucket_name, &input);
    return gdp_gin_subscribe_by_recno(input, 0, 0, nullptr,                   
                                      &handle_packet,                  
                                      (void*) output);
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cout << "Output bucket and input bucket required.\n";
        return 0;
    }

    gdp_init(nullptr);

    gdp_gin_t* output;
    open(argv[1], &output);

    EP_STAT estat;
    for (int i = 2; i < argc; i++) {
        estat = route(argv[i], output);
        EP_STAT_CHECK(estat, std::cout << "Fail subscribing to " << argv[i] << "\n");
    }

    std::cout << "Ready to receive\n";
    sleep(3600);
	return 0;
}

