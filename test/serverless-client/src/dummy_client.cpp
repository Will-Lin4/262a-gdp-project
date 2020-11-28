#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <fstream>
#include "gdp_agent.hpp"

void print(std::vector<uint8_t> out) {
    std::cout << (char*) out.data(); 
}

int main(int argc, char * argv[])
{
    /*
    gdp_agent test;

    std::string bucket_name = std::string(argv[1]);
    test.open(bucket_name);
    test.subscribe(bucket_name, &print);
    
    sleep(3600);
    */
	return 0;
}

