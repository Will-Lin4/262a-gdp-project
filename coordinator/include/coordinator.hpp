#ifndef COORDINATOR_HEADER
#define COORDINATOR_HEADER

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>

#include <mpl/packet.hpp>
#include "lambda_service.h"
#include "problem_statement.h"
#include "json.hpp"

#if HAS_AWS_SDK
#include <aws/lambda-runtime/runtime.h>
#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/InvokeRequest.h>
#include <aws/core/utils/json/JsonSerializer.h>
#endif

#define LOW_QOS_LAMBDAS		1
#define MEDIUM_QOS_LAMBDAS	8
#define HIGH_QOS_LAMBDAS	16

#define COMPUTE_LAMBDA_PROG		"./compute_lambda"
#define AGGREGATOR_LAMBDA_PROG	"./aggregator_lambda"

/*------------------------------------------------------------------------------
  The coordinator acts as a liason between a GDP client and the lambda service
  which actually provides serverless computing capabilities. In our model, the
  service provider should provide coordinator capabilities. It is also assumed
  that the coordinator/service provider is untrusted, and thus lambdas must be
  capable of verifying the code they are running.
------------------------------------------------------------------------------*/
class coordinator
{
private:
	/*--------------------------------------------------------------------------
	  The problem_set map keeps track of all currently running problems.
	--------------------------------------------------------------------------*/
	std::unordered_map<std::string, problem_statement> problem_set;

	/*--------------------------------------------------------------------------
	  Structure used to keep track of computation_id's and their associated
	  compute and aggregator lambda paths.
	--------------------------------------------------------------------------*/
	typedef struct lambda_paths
	{
		std::string compute_path;
		std::string aggregator_path;
	} lambda_paths;

	/*--------------------------------------------------------------------------
	  The lambda_set keeps track of the allowed lambdas which may be launched.
	--------------------------------------------------------------------------*/
	std::unordered_map<std::string, lambda_paths> lambda_set;


public:
	/*--------------------------------------------------------------------------
	  The constructor for the coordinator must setup variables needed by
	  the TCP server, and must fill the lambda_set with lambda_id:lambda_file
	  pairs.
	--------------------------------------------------------------------------*/
	coordinator();

	/*--------------------------------------------------------------------------
	  This method checks to see if any problem_statement fields have been left
	  blank. If so, the coordinator will fill them in. If the coordinator finds
	  any issues with the problem_statement, it will reject the problem and 
	  return false. If the problem_statement is deemed okay, the function will
	  return true.
	--------------------------------------------------------------------------*/
	bool check_problem(problem_statement & problem);

	/*--------------------------------------------------------------------------
	  The submit_problem method takes a problem from the problem_set and 
	  submits it to the specified lambda_service. If this function fails, it 
	  returns false. Otherwise it returns true.
	--------------------------------------------------------------------------*/
	bool submit_problem(problem_statement & problem);

	/*--------------------------------------------------------------------------
	  The remove_problem method removes a problem in the problem_set. If the 
	  problem is not in the problem_set, then this function returns false.
	  Otherwise this function will return true, and the specified problem will
	  be removed.
	--------------------------------------------------------------------------*/
	bool remove_problem(std::string & problem_id);

private:
	/*--------------------------------------------------------------------------
	  launch_lambdas is a private method that is called by the submit_problem
	  method. It calls the function necessary to submit some lambda_image_file
	  along with a specified parameter file to the indicated lambda_service.
	--------------------------------------------------------------------------*/
	bool launch_lambdas(std::string & problem_id, int lambda_service,
						int num_lambdas);
	void launch_aws_lambda(std::vector<std::string> & lambda_args);
	void launch_local_lambda(std::vector<std::string> & lambda_args);
};

coordinator::coordinator()
{
	using std::string; using std::pair;
	// Look for Lambda images to load into the lambda_set
	string path = "./lambda_images/lambda_table";
	string line;
	std::ifstream lambda_table("./lambda_images/lambda_table", std::ifstream::in); 
	
	if (lambda_table.is_open())
	{
		while (std::getline(lambda_table, line))
		{
			string computation_id, compute, aggregator;
			std::istringstream iss(line);
			if (iss >> computation_id >> compute >> aggregator)
			{
				lambda_paths paths;
				paths.compute_path = compute;
				paths.aggregator_path = aggregator;
				pair<string,lambda_paths> set_entry(computation_id, paths);
				lambda_set.insert(set_entry);
			}
		}
	}
}

bool coordinator::check_problem(problem_statement & problem)
{
	bool valid_problem = true;
	// Check to see if problem_id is valid

	// Check to see if computation_id is valid
	//std::unordered_map<std::string,lambda_paths>::const_iterator lambda_iter
	//	= lambda_set.find(problem.computation_id);
	auto lambda_iter = lambda_set.find(problem.computation_id);
	if (lambda_iter == lambda_set.end())
	{
		valid_problem = false;
	}
	// If duration is <= 0, then set it to the DEFAULT_DURATION
	if (problem.duration <= 0)
	{
		std::cout << "Setting duration to DEFAULT_DURATION" << std::endl;
		problem.duration = DEFAULT_DURATION;
	}
	// If the quality of service is not set, set it to LOW
	level qos = check_qos(problem);
	if (qos != problem.quality_of_service)
	{
		std::cout << "Setting quality of service to " << qos << std::endl;
		problem.quality_of_service = qos;
	}
	return valid_problem;
}

bool coordinator::submit_problem(problem_statement & prob)
{
	// Assumes that the problem has passed checks
	// Insert the problem into the problem_set
	std::pair<std::string,problem_statement>prob_pair(prob.problem_id,prob);
	problem_set.insert(prob_pair);
	// Select lambda service to use
	int service = LOCAL_HOST;
	// Use QOS to determine how many compute lambdas to launch
	int num_lambdas = LOW_QOS_LAMBDAS;
	level qos = prob.quality_of_service;
	if (qos == HIGH)
	{
		num_lambdas = HIGH_QOS_LAMBDAS;
	}
	else if (qos == MEDIUM)
	{
		num_lambdas = MEDIUM_QOS_LAMBDAS;
	}
	else
	{
		// Low quality, keep num_lambdas = 1
	}
	return launch_lambdas(prob.problem_id, service, num_lambdas);
}

bool coordinator::remove_problem(std::string & problem_id)
{
	// Remove problem from problem set after client indicates to do so
	return problem_set.erase(problem_id) != 0;
}

bool coordinator::launch_lambdas(std::string & problem_id, int lambda_service,
								 int num_lambdas)
{
	using std::string; using std::vector;
	// Set up some mplambda specific variables
	vector<string> compute_vector;
	vector<string> aggregator_vector;
	compute_vector.reserve(3);
	aggregator_vector.reserve(3);
	// Find the lambdas to execute
	string comp_id = problem_set.find(problem_id)->second.computation_id;
	string compute_lambda = lambda_set.find(comp_id)->second.compute_path;
	string aggregator_lambda = lambda_set.find(comp_id)->second.aggregator_path;
	string time_limit = std::to_string(problem_set.find(problem_id)->second.duration);

	compute_vector.push_back(compute_lambda);
	compute_vector.push_back(problem_id);
	compute_vector.push_back(time_limit);
	aggregator_vector.push_back(aggregator_lambda);
	aggregator_vector.push_back(problem_id);
	aggregator_vector.push_back(time_limit);

	// Choose subfunction to launch
	switch (lambda_service)
	{
	// Launch AWS lambdas
	case AWS_LAMBDA:
		// Launch the computation lambdas
		for (int i = 0; i < num_lambdas; ++i)
		{
			launch_aws_lambda(compute_vector);
		}
		// Launch the aggregator lambda
		launch_aws_lambda(aggregator_vector);
		break;

	// Launch local host lambdas
	default:
		// Launch the computation lambdas
		for (int i = 0; i < num_lambdas; ++i)
		{
			launch_local_lambda(compute_vector);
		}
		// Launch the aggregator lambda
		launch_local_lambda(aggregator_vector);
	}
	return true;
}

void coordinator::launch_aws_lambda(std::vector<std::string> & lambda_args)
{

}

void coordinator::launch_local_lambda(std::vector<std::string> & lambda_args)
{
	// Convert to char array
	std::vector<char *> cmd_vector;
	cmd_vector.reserve(lambda_args.size() + 1);
	for (unsigned int i = 0; i < lambda_args.size(); ++i)
		cmd_vector.push_back(const_cast<char *>(lambda_args[i].c_str()));
	cmd_vector.push_back(NULL);
	char ** cmd_array = cmd_vector.data();

	// Call exec to launch a process that executes the indicated lambda func
	if (fork() == 0)
		execvp(cmd_array[0], &cmd_array[0]);
}


#endif
