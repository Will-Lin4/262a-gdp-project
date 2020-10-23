﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/glue/model/GetColumnStatisticsForTableResult.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/AmazonWebServiceResult.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/UnreferencedParam.h>

#include <utility>

using namespace Aws::Glue::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;
using namespace Aws;

GetColumnStatisticsForTableResult::GetColumnStatisticsForTableResult()
{
}

GetColumnStatisticsForTableResult::GetColumnStatisticsForTableResult(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  *this = result;
}

GetColumnStatisticsForTableResult& GetColumnStatisticsForTableResult::operator =(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  JsonView jsonValue = result.GetPayload().View();
  if(jsonValue.ValueExists("ColumnStatisticsList"))
  {
    Array<JsonView> columnStatisticsListJsonList = jsonValue.GetArray("ColumnStatisticsList");
    for(unsigned columnStatisticsListIndex = 0; columnStatisticsListIndex < columnStatisticsListJsonList.GetLength(); ++columnStatisticsListIndex)
    {
      m_columnStatisticsList.push_back(columnStatisticsListJsonList[columnStatisticsListIndex].AsObject());
    }
  }

  if(jsonValue.ValueExists("Errors"))
  {
    Array<JsonView> errorsJsonList = jsonValue.GetArray("Errors");
    for(unsigned errorsIndex = 0; errorsIndex < errorsJsonList.GetLength(); ++errorsIndex)
    {
      m_errors.push_back(errorsJsonList[errorsIndex].AsObject());
    }
  }



  return *this;
}
