#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <list>
#include <unistd.h>
#include <fstream>
#include <thread>
#include <mutex>

extern "C" {
    #include <gdp/gdp.h>
}

EP_STAT open(const char* bucket_name, gdp_gin_t** out) {
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

EP_STAT route(const char* bucket_name, gdp_gin_t* output) {
    gdp_gin_t* input;
    open(bucket_name, &input);
    return gdp_gin_subscribe_by_recno(input, 0, 0, nullptr,                   
                                      &handle_packet,                  
                                      (void*) output);
}

const size_t total_buckets = 10;
const size_t num_threads = 5;
std::string bucket_name = "mpl-out-";
std::list<int> buckets;
std::mutex buckets_mutex;

void subscriber_thread(gdp_gin_t* output) {
    while (!buckets.empty()) {
        int bucket_num = 0;
        {
            const std::lock_guard<std::mutex> lock(buckets_mutex);
            if (buckets.empty()) return;
            bucket_num = buckets.front();
            buckets.pop_front();
        }

        std::string bucket_i = bucket_name + std::to_string(bucket_num);
        std::cout << "Subscribing to " << bucket_i << "\n";
        if (!EP_STAT_ISOK(route(bucket_i.c_str(), output))) {
            std::cout << "Fail subscribing to " << bucket_i << "\n";
            {
                const std::lock_guard<std::mutex> lock(buckets_mutex);
                buckets.emplace_back(bucket_num);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Output bucket and input bucket required.\n";
        return 0;
    }

    gdp_init(nullptr);

    gdp_gin_t* output;
    open(argv[1], &output);

    EP_STAT estat;

    for (int b = 0; b < total_buckets; b++) {
        buckets.emplace_back(b);
    }

    std::thread threads[num_threads];
    for (int t = 0; t < num_threads; t++) {
        threads[t] = std::thread(subscriber_thread, output);
    }

    for (int t = 0; t < num_threads; t++) {
        threads[t].join();
    }

    std::cout << "Ready to receive\n";
    sleep(3600);
	return 0;
}

