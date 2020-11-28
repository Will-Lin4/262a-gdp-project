#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <fstream>
#include "gdp_agent.hpp"

void print(std::vector<uint8_t> out, void* udata) {
    std::cout << (char*) out.data() << "\n"; 
    std::cout << "udata:" << udata << "\n";
    std::cout << *((int*) udata) << "\n"; 
}

int main(int argc, char * argv[])
{
    gdp_agent test;

    int a = 123;
    std::cout << "A:" << &a << "\n";
    std::string bucket_name = std::string(argv[1]);
    test.open(bucket_name);
    test.subscribe(bucket_name, &print, (void*) &a);
    
    sleep(3600);
	return 0;
}

