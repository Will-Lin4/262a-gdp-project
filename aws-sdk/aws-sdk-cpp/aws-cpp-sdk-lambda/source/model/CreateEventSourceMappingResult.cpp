﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/lambda/model/CreateEventSourceMappingResult.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/AmazonWebServiceResult.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/UnreferencedParam.h>

#include <utility>

using namespace Aws::Lambda::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;
using namespace Aws;

CreateEventSourceMappingResult::CreateEventSourceMappingResult() : 
    m_batchSize(0),
    m_maximumBatchingWindowInSeconds(0),
    m_parallelizationFactor(0),
    m_maximumRecordAgeInSeconds(0),
    m_bisectBatchOnFunctionError(false),
    m_maximumRetryAttempts(0)
{
}

CreateEventSourceMappingResult::CreateEventSourceMappingResult(const Aws::AmazonWebServiceResult<JsonValue>& result) : 
    m_batchSize(0),
    m_maximumBatchingWindowInSeconds(0),
    m_parallelizationFactor(0),
    m_maximumRecordAgeInSeconds(0),
    m_bisectBatchOnFunctionError(false),
    m_maximumRetryAttempts(0)
{
  *this = result;
}

CreateEventSourceMappingResult& CreateEventSourceMappingResult::operator =(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  JsonView jsonValue = result.GetPayload().View();
  if(jsonValue.ValueExists("UUID"))
  {
    m_uUID = jsonValue.GetString("UUID");

  }

  if(jsonValue.ValueExists("BatchSize"))
  {
    m_batchSize = jsonValue.GetInteger("BatchSize");

  }

  if(jsonValue.ValueExists("MaximumBatchingWindowInSeconds"))
  {
    m_maximumBatchingWindowInSeconds = jsonValue.GetInteger("MaximumBatchingWindowInSeconds");

  }

  if(jsonValue.ValueExists("ParallelizationFactor"))
  {
    m_parallelizationFactor = jsonValue.GetInteger("ParallelizationFactor");

  }

  if(jsonValue.ValueExists("EventSourceArn"))
  {
    m_eventSourceArn = jsonValue.GetString("EventSourceArn");

  }

  if(jsonValue.ValueExists("FunctionArn"))
  {
    m_functionArn = jsonValue.GetString("FunctionArn");

  }

  if(jsonValue.ValueExists("LastModified"))
  {
    m_lastModified = jsonValue.GetDouble("LastModified");

  }

  if(jsonValue.ValueExists("LastProcessingResult"))
  {
    m_lastProcessingResult = jsonValue.GetString("LastProcessingResult");

  }

  if(jsonValue.ValueExists("State"))
  {
    m_state = jsonValue.GetString("State");

  }

  if(jsonValue.ValueExists("StateTransitionReason"))
  {
    m_stateTransitionReason = jsonValue.GetString("StateTransitionReason");

  }

  if(jsonValue.ValueExists("DestinationConfig"))
  {
    m_destinationConfig = jsonValue.GetObject("DestinationConfig");

  }

  if(jsonValue.ValueExists("Topics"))
  {
    Array<JsonView> topicsJsonList = jsonValue.GetArray("Topics");
    for(unsigned topicsIndex = 0; topicsIndex < topicsJsonList.GetLength(); ++topicsIndex)
    {
      m_topics.push_back(topicsJsonList[topicsIndex].AsString());
    }
  }

  if(jsonValue.ValueExists("MaximumRecordAgeInSeconds"))
  {
    m_maximumRecordAgeInSeconds = jsonValue.GetInteger("MaximumRecordAgeInSeconds");

  }

  if(jsonValue.ValueExists("BisectBatchOnFunctionError"))
  {
    m_bisectBatchOnFunctionError = jsonValue.GetBool("BisectBatchOnFunctionError");

  }

  if(jsonValue.ValueExists("MaximumRetryAttempts"))
  {
    m_maximumRetryAttempts = jsonValue.GetInteger("MaximumRetryAttempts");

  }



  return *this;
}
