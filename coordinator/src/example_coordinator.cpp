#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>

#include "coordinator.hpp"
#include "problem_statement.h"
#include "lambda_service.h"
#include "shared.h"


int main(int argc, char**argv) 
{
	using std::cout; using std::endl; using std::string;
	using json = nlohmann::json;

	// TCP Server setup
	int server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		cout << "Error with socket()" << endl;
		exit(1);
	}
	sockaddr_in server_address;
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) 
		== -1)
	{
		cout << "Error with bind()" << endl;
		close(server_socket);
		exit(1);
	}
	if (listen(server_socket, BACKLOG_SIZE) == -1)
	{
		cout << "Error with listen()" << endl;
		close(server_socket);
		exit(1);
	}

	// Coordinator handles launching of lambdas indicated in problem_statements
	coordinator coord;

	// Main while loop
	bool running = true;
	while (running)
	{
		auto address_size = sizeof(server_address);
		int connection = accept(server_socket, (sockaddr*)&server_address, 
								(socklen_t*)&address_size);
		if (connection < 0)
		{
			cout << "Error with accept()" << endl;
			close(connection);
			close(server_socket);
			exit(1);
		}
		// Read message into buffer
		char buffer[BUFFER_SIZE];
		ssize_t read_size = read(connection, buffer, BUFFER_SIZE - 1);
		buffer[read_size] = '\0';

		// Parse message, try to fill out fields for problem statement
		json j = json::parse(std::string(buffer));
		
		// If message could not be parsed, close connection and continue
		if (!(j.contains("problem_id") && j.contains("computation_id") 
			&& j.contains("duration") && j.contains("quality_of_service")))
		{
			cout << "Error parsing JSON" << endl;
			close(connection);
			continue;
		}
		// If message could be parsed
		else
		{
			string response;

			// Check the problem
			problem_statement problem = json_to_problem(j);
			if (coord.check_problem(problem))
			{
				// Send response to the client
				cout << "Problem is valid" << endl;

				// Add the problem to the problem set and submit the problem
				if (coord.submit_problem(problem))
				{
					response = PROBLEM_OK;
				}
				else
				{
					response = PROBLEM_ERROR;
				}
			}
			else
			{
				// Send response to the client
				cout << "Problem is not valid" << endl;
				response = PROBLEM_ERROR;
			}
			strcpy(buffer, response.c_str());
			send(connection, buffer, response.length() + 1, 0);
		}
		// Remove the problem from the problem set
		// TODO: Add multithreading support so coordinator can wait for client
		// to respond with OK before removing the problem from the problem set 
		//coord.remove_problem(problem);
		close(connection);
	}
	close(server_socket);
	return 0;
}
