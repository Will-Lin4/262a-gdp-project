#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <app_options.hpp>
#include "gdp_agent.hpp"

using json = nlohmann::json;

int main(int argc, char * argv[]) try {
    
    std::ifstream t(argv[1]);
    std::string bucket_name = std::string(argv[2]);

    std::string data((std::istreambuf_iterator<char>(t)),
                      std::istreambuf_iterator<char>());
    json json_data = json::parse(data);
    std::vector<uint8_t> cbor = json::to_cbor(json_data);

    gdp_agent agent;
    agent.create(bucket_name);
    agent.open(bucket_name);

    EP_STAT estat = agent.write_bytes(bucket_name, cbor.data(), cbor.size());

    if (EP_STAT_ISOK(estat)) {
        std::cout << "Success\n";
    } else {
        std::cout << "Fail\n";
    }

	return 0;
} catch (const std::invalid_argument& ex) {
    std::cerr << "Invalid argument: " << ex.what() << std::endl;
    return 1;
}
