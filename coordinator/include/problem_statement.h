#ifndef PROBLEM_STATEMENT_HEADER
#define PROBLEM_STATEMENT_HEADER

#include <string>

#include "json.hpp"

#define DEFAULT_DURATION 100

/*------------------------------------------------------------------------------
  A problem can have 3 levels of urgency/quality of service. These levels are
  used to determine how many compute nodes are launched by the coordinator.
------------------------------------------------------------------------------*/
enum level
{
	LOW    = 0,
	MEDIUM = 1,
	HIGH   = 2
};

/*------------------------------------------------------------------------------
  A problem_statement contains all data that the coordinator needs from the 
  client in order to submit a serverless computing request to a service 
  provider. If parameters other than problem_id and lambda_id are left as 
  defaults, then the coordiantor's check_problem method will assign values.
------------------------------------------------------------------------------*/
typedef struct problem_statement
{
	/*--------------------------------------------------------------------------
	  The problem_id is also the GDP name of the DataCapsule containing the
	  problem specific data
	--------------------------------------------------------------------------*/
	std::string problem_id;

	/*--------------------------------------------------------------------------
	  ID for the lambda image to be launched
	--------------------------------------------------------------------------*/
	std::string computation_id;

	/*--------------------------------------------------------------------------
	  Determines how long the problem should be allowed to run in seconds
	--------------------------------------------------------------------------*/
	int duration;

	/*--------------------------------------------------------------------------
	  Determines how many compute lambdas should be launched
	--------------------------------------------------------------------------*/
	level quality_of_service;

} problem_statement;

nlohmann::json problem_to_json(problem_statement & prob)
{
	nlohmann::json j;
	j["problem_id"] = prob.problem_id;
	j["computation_id"] = prob.computation_id;
	j["duration"] = prob.duration;
	j["quality_of_service"] = prob.quality_of_service;
	return j;
}

problem_statement json_to_problem(nlohmann::json & jobj)
{
	problem_statement prob;
	prob.problem_id         = jobj["problem_id"];
	prob.computation_id     = jobj["computation_id"];
	prob.duration           = jobj["duration"];
	prob.quality_of_service = jobj["quality_of_service"];
	return prob;
}

level check_qos(problem_statement prob)
{
	switch (prob.quality_of_service)
	{
	case 1:
		return MEDIUM;
	case 2:
		return HIGH;
	default:
		return LOW;
	}
}

#endif
