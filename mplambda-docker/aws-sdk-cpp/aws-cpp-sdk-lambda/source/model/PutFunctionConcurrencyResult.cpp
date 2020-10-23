﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/lambda/model/PutFunctionConcurrencyResult.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/AmazonWebServiceResult.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/UnreferencedParam.h>

#include <utility>

using namespace Aws::Lambda::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;
using namespace Aws;

PutFunctionConcurrencyResult::PutFunctionConcurrencyResult() : 
    m_reservedConcurrentExecutions(0)
{
}

PutFunctionConcurrencyResult::PutFunctionConcurrencyResult(const Aws::AmazonWebServiceResult<JsonValue>& result) : 
    m_reservedConcurrentExecutions(0)
{
  *this = result;
}

PutFunctionConcurrencyResult& PutFunctionConcurrencyResult::operator =(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  JsonView jsonValue = result.GetPayload().View();
  if(jsonValue.ValueExists("ReservedConcurrentExecutions"))
  {
    m_reservedConcurrentExecutions = jsonValue.GetInteger("ReservedConcurrentExecutions");

  }



  return *this;
}
