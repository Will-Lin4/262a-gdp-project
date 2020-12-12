#include <string>
#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <chrono>
#include <time.h>
#include <random>
#include <thread>

#include "client.hpp"
#include "gdp_agent.hpp"
#include "json.hpp"
#include "problem_statement.h"
#include "shared.h"

//#define DURATION 	5
//#define QOS 		MEDIUM

#define EXEC_PROCESS "lambda_images/mpl_robot"

using json = nlohmann::json;
using std::string;
using std::cout;
using std::endl;

void write_output_to_file(std::vector<uint8_t> out, void * udata);

int main(int argc, char ** argv)
{
	if (argc != 6 && argc != 5)
	{
		cout << "./example_client <input json> <problem name> <computation name> <duration> [<service>]" << endl;
		cout << "\tinput json: a JSON parameter file to upload to the GDP" << endl;
		cout << "\tproblem name: a problem name, used in GDP name" << endl;
		cout << "\tcomputation name: name of the lambda to be used for computation" << endl;
		cout << "\tduration: problem duration" << endl;
		cout << "\tservice: service to use [ local | aws ]" << endl;
		exit(1);
	}

	std::cout << time(NULL) << "\tSTART TIMESTAMP" << std::endl;

	// Find the JSON file
	std::ifstream json_file(argv[1]);
	int random = rand() % 100000;
	std::string problem_name = std::string(argv[2]) + std::to_string(random);
	std::string lambda_name = std::string(argv[3]);
	std::string prob_duration = std::string(argv[4]);
	std::string service;
	if (argc == 5)
		service == std::to_string(SERVICE_DEFAULT);
	else
		service == std::string(argv[5]);

    std::string data((std::istreambuf_iterator<char>(json_file)),
                      std::istreambuf_iterator<char>());
    json json_data = json::parse(data);
    std::vector<uint8_t> cbor = json::to_cbor(json_data);
	//nlohmann::json json_object = json::parse(json_file);

	// Create GDP DataCapsule
	gdp_agent agent;
	//string gdp_bucket_name = "example_coordinator_test";
	//string lambda_name     = "test_local"; // mpl_gdp_lambda_aws_test
	//string lambda_name = SOMETHING;

	
	string input_bucket_name = problem_name + "_input";
	string output_bucket_name = problem_name + "_output";

	// Try to create the gdp bucket
	EP_STAT estat;
	estat = agent.create(input_bucket_name);
	estat = agent.create(output_bucket_name);
	/*if (!EP_STAT_ISOK(estat))
	{
		cout << "Could not create GDP DataCapsule: " << gdp_bucket_name;
		cout << " (it may already exist)" << endl;
	}*/
	// Try to open the gdp bucket
	// Note that gdp_agent::open always returns !EP_STAT_ISOK()
	estat = agent.open(input_bucket_name);
	/*if (!EP_STAT_ISOK(estat))
	{
		cout << "Could not open GDP DataCapsule: " << gdp_bucket_name << endl;
	}*/
	
	// Try to write to the gdp bucket
	//estat = gdp.write(gdp_bucket_name, json_object);
    //EP_STAT estat = agent.write_bytes(input_bucket_name, cbor.data(), cbor.size());
	estat = agent.write_bytes(input_bucket_name, cbor.data(), cbor.size());
	if (!EP_STAT_ISOK(estat))
	{
		cout << "Could not write to GDP DataCapsule" << endl;
		cout << "DataCapsule name: " << input_bucket_name << endl;
		cout << "JSON parameter file: " << argv[1] << endl;
	}

	// Create problem statement	
	problem_statement prob;
	prob.problem_id = problem_name;
	prob.computation_id = lambda_name;
	prob.duration = atoi(prob_duration.c_str()); 
	prob.service = atoi(service.c_str());
	//prob.quality_of_service = QOS;
	prob.quality_of_service = LEVEL_MEDIUM;

	// Convert to JSON
	json j = problem_to_json(prob);
	// Create JSON string to send to coordinator
	string json_str = j.dump();
	// Convert to c string and fill message buffer
	int buf_size;
	json_str.length() > BUFFER_SIZE ? buf_size = json_str.length() + 1
									: buf_size = BUFFER_SIZE;
	char buffer[buf_size];
	strcpy(buffer, json_str.c_str());

	// Set up TCP connection
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == -1)
	{
		cout << "Error with socket()" << endl;
		exit(1);
	}
	sockaddr_in client_address;
	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(SERVER_PORT);
	// Test address
	if (inet_pton(AF_INET, "127.0.0.1", &client_address.sin_addr) <= 0)
	{
		cout << "Error with address" << endl;
		exit(1);
	}
	
	// Attempt to connect to the server
	if (connect(client_socket, (sockaddr *)&client_address, 
		sizeof(client_address)) < 0)
	{
		cout << "Error with connect()" << endl;
		exit(1);
	}
	
	// Send JSON string to coordinator server
	send(client_socket, buffer, json_str.length() + 1, 0);
	//cout << "client sent: " << buffer << endl;

	// Wait for response from coordinator server
	int read_size = read(client_socket, buffer, BUFFER_SIZE - 1);
	buffer[read_size] = '\0';
	string coordinator_response(buffer);
	
	if (coordinator_response == PROBLEM_OK)
	{
		std::vector<char *> exec_vector;
		exec_vector.reserve(3);
		exec_vector.push_back(const_cast<char *>(EXEC_PROCESS));
		exec_vector.push_back(const_cast<char *>(output_bucket_name.c_str()));
		exec_vector.push_back(NULL);
		char ** exec_array = exec_vector.data();
		execvp(exec_array[0], &exec_array[0]);
		/*
		cout << "The problem was submitted" << endl;
		bool subscribed = false;
		string output_file_str = "./output/" + prob.problem_id + "_output";
		const char * output_file = output_file_str.c_str();
		// Try to open the output bucket
		string output_bucket = prob.problem_id + "_output";
		estat = gdp.open(output_bucket);
		// Note that gdp_agent::open always returns !EP_STAT_ISOK()
		*/
		/*if (!EP_STAT_ISOK(estat))
		{
			cout << "Could not open output bucket (" << output_bucket 
				 << "), trying to subscribe anyway" << endl;
		}*/
		/*
		// Start timer
		auto start = std::chrono::steady_clock::now();
		// Check output bucket for result
		bool poll_output = true;
		bool timeout = false;
		while (poll_output && !timeout)
		{
			// Check for timeout
			auto current = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>
						   (current - start).count();
			if (elapsed > prob.duration + 5)
			{
				cout << "Timeout: if output exists, it is in " << output_file;
				cout << endl;
				timeout = true;
			}

			// Try to subscribe to the output bucket
			if (!subscribed)
			{
				gdp.subscribe(output_bucket, &write_output_to_file,
							  (void*)output_file);
				subscribed = true;
			}
			else
			{
				sleep(1);
			}
		}
		*/
		
		// Display output
		// print cost path

	}	
	else if (coordinator_response == PROBLEM_ERROR)
	{
		cout << "Coordinator could not submit the problem" << endl;
		exit(1);
	}
	else
	{
		cout << "Unknown error" << endl;
		exit(1);
	}

	
	return 0;
}


void write_output_to_file(std::vector<uint8_t> out, void * udata)
{
	auto received_data = (char *) out.data();
	char * output_file_name{static_cast<char*>(udata)};

	cout << "Client has received data written to output DataCapsule:" << endl;
	cout << "\t" << received_data << endl;

	std::ofstream output_file(output_file_name);
	if (output_file.is_open())
	{
		output_file << received_data << "\n";
		output_file.close();
	}
	else
	{
		cout << "Could not open output file " << output_file_name << endl;
	}
}
