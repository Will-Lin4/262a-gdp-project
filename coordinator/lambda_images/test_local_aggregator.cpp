#include <stdio.h>
#include <string>
#include <unistd.h>

#include "gdp_agent.hpp"

int main(int argc, char ** argv)
{
	if (argc != 3)
	{
		printf("Too few arguments to aggregator\n");
		return 1;
	}
	printf("test_local_aggregator - problem_id: %s\n", argv[1]);
	std::string message = "this is from the aggregator";
	const char * char_message = message.c_str();
	uint8_t * byte_message = (uint8_t *) char_message;
	unsigned int size = message.size();

	std::string input_bucket(argv[1]);
	std::string output_bucket(input_bucket + "_output");

	int duration = atoi(argv[2]);
	
	//printf("Sending message to %s", output_bucket.c_str());

	sleep(duration*2/3);

	gdp_agent gdp;
	gdp.open(output_bucket);
	gdp.write_bytes(output_bucket, byte_message, size);

	return 0;
}
