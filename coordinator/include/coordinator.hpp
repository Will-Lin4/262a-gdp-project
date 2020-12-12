#ifndef COORDINATOR_HEADER
#define COORDINATOR_HEADER

#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

#include "problem_statement.h"

#include <aws/lambda-runtime/runtime.h>
#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/InvokeRequest.h>
#include <aws/core/utils/json/JsonSerializer.h>

#define LOW_QOS_LAMBDAS		1
#define MEDIUM_QOS_LAMBDAS	8
#define HIGH_QOS_LAMBDAS	16

#define ALLOCATION_TAG "mplLambdaAWS"
 

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
	  Used for running lambdas on AWS.
	--------------------------------------------------------------------------*/
	std::shared_ptr<Aws::Lambda::LambdaClient> m_client;
	Aws::SDKOptions options;

public:
	/*--------------------------------------------------------------------------
	  The constructor for the coordinator must setup variables needed by
	  the TCP server, and must fill the lambda_set with lambda_id:lambda_file
	  pairs.
	--------------------------------------------------------------------------*/
	coordinator();
	~coordinator();

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
	bool remove_problem(problem_statement & problem);

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

	// Start up AWS API
	//ALLOCATION_TAG = "mplLambdaAWS";
	Aws::InitAPI(options);
	Aws::Client::ClientConfiguration clientConfig;
	m_client = Aws::MakeShared<Aws::Lambda::LambdaClient>
			   (ALLOCATION_TAG, clientConfig);
}

coordinator::~coordinator()
{
	// Empty the problem_set and lambda_set
	problem_set.clear();
	lambda_set.clear();

	// Shut down the AWS API
	Aws::ShutdownAPI(options);
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
	//level qos = check_qos(problem);
	int qos = check_qos(problem);
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
	//int service = LOCAL_HOST;
	int service = prob.service;
	// Use QOS to determine how many compute lambdas to launch
	int num_lambdas = LOW_QOS_LAMBDAS;
	//level qos = prob.quality_of_service;
	int qos = prob.quality_of_service;
	if (qos == LEVEL_HIGH)
	{
		num_lambdas = HIGH_QOS_LAMBDAS;
	}
	else if (qos == LEVEL_MEDIUM)
	{
		num_lambdas = MEDIUM_QOS_LAMBDAS;
	}
	else
	{
		// Low quality, keep num_lambdas = 1
	}
	return launch_lambdas(prob.problem_id, service, num_lambdas);
}

bool coordinator::remove_problem(problem_statement & problem)
{
	// Remove problem from problem set after done responding
	return problem_set.erase(problem.problem_id) != 0;
}

bool coordinator::launch_lambdas(std::string & problem_id, int lambda_service,
								 int num_lambdas)
{
	using std::string; using std::vector;
	// Find the lambdas to execute
	string comp_id = problem_set.find(problem_id)->second.computation_id;
	string compute_lambda = lambda_set.find(comp_id)->second.compute_path;
	string aggregator_lambda = lambda_set.find(comp_id)->second.aggregator_path;
	int duration = problem_set.find(problem_id)->second.duration;
	string time_limit = std::to_string(duration);

	// Set the deadline
//	auto deadline = std::chrono::system_clock::now();
//	deadline += std::chrono::seconds(duration);
//	std::time_t t = std::chrono::system_clock::to_time_t(deadline);	
//	string time_limit = std::ctime(&t);		// Uses UTC

	// Set up parameter vectors
	vector<string> compute_vector;
	vector<string> aggregator_vector;
	compute_vector.reserve(3);
	aggregator_vector.reserve(3);

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
	case SERVICE_AWS:
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
	// Code from mplambda/src/mpl_lambda_invoke.cpp
	Aws::Lambda::Model::InvokeRequest invokeRequest;
	invokeRequest.SetFunctionName(lambda_args[0].c_str());
	invokeRequest.SetInvocationType(Aws::Lambda::Model::InvocationType::Event);
	std::shared_ptr<Aws::IOStream> payload = 
		Aws::MakeShared<Aws::StringStream>("PayloadData");
	Aws::Utils::Json::JsonValue jsonPayload;
	jsonPayload.WithString("problem_id", lambda_args[1].c_str());
	jsonPayload.WithString("duration", lambda_args[2].c_str());
	*payload << jsonPayload.View().WriteReadable();
	invokeRequest.SetBody(payload);
	invokeRequest.SetContentType("application/json");

	auto outcome = m_client->Invoke(invokeRequest);
	if (outcome.IsSuccess())
	{
		auto &result = outcome.GetResult();
		Aws::IOStream & payload = result.GetPayload();
		Aws::String functionResult;
		std::getline(payload, functionResult);
		std::cout << "Lambda result:\n" << functionResult << "\n\n";
	}
	else
	{
		auto &error = outcome.GetError();
		std::cout << "Error: " << error.GetExceptionName() << "\nMessage: " 
				  << error.GetMessage() << "\n\n";
	}
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
