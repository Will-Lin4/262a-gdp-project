#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
#include "gdp_agent.hpp"

//using json = nlohmann::json;

int main(int argc, char * argv[])
{
    /*
    std::ifstream file("Twistycool_env.dae", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);

    if (file.read(buffer.data(), size))
    {
        gdp_agent test;

        std::string bucket_name = "Twistycool_env.dae12";
        test.create(bucket_name);
        test.open(bucket_name);
        test.write_bytes(bucket_name, (uint8_t*) buffer.data(), (size_t) size);
    }
    */
    /*
    gdp_agent test;

    /*
    std::string bucket_name = "test123";
    test.open(bucket_name);
    test.write_bytes(bucket_name, (uint8_t*) "hello!", 7);

    std::vector<uint8_t> output;
    test.read_bytes(bucket_name, -1, &output);
    std::cout << output.size() << "\n";

    test.create("test1234");
    
    */

    gdp_agent test;

    std::string bucket_name = std::string(argv[1]);
    test.create(bucket_name);
    test.open(bucket_name);
    
    std::cout << "Ready to write to " << bucket_name << "\n";
    for (std::string line; std::getline(std::cin, line);) {
        std::cout << "Written" << "\n";
        test.write_bytes(bucket_name, (uint8_t*) line.c_str(), line.length());
    }

	return 0;
}

