#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "json.hpp"
#include "gdp_agent.h"

using json = nlohmann::json;

int main(int argc, char * argv[])
{
    gdp_agent test;

    json j;
    j["hello"] = "hiicao";
    std::string bucket_name = "new0_3123234";
    test.create(bucket_name);
    test.write(bucket_name, j);

    json b;
    test.read(bucket_name, -1, &b);
    std::cout << b;

	return 0;
}

