#ifndef LAMBDA_SERVICE_HEADER
#define LAMBDA_SERVICE_HEADER

/*------------------------------------------------------------------------------
  The lambda service determines what serverless computing provider should be 
  used for a given problem.
------------------------------------------------------------------------------*/
#define LAMBDA_DEFAULT		0
#define AWS_LAMBDA			1
#define LOCAL_HOST			2
#define CLOUDBURST			3

#endif
