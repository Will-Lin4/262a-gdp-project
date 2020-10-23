﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/ssm/model/RegisterTaskWithMaintenanceWindowResult.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/AmazonWebServiceResult.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/UnreferencedParam.h>

#include <utility>

using namespace Aws::SSM::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;
using namespace Aws;

RegisterTaskWithMaintenanceWindowResult::RegisterTaskWithMaintenanceWindowResult()
{
}

RegisterTaskWithMaintenanceWindowResult::RegisterTaskWithMaintenanceWindowResult(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  *this = result;
}

RegisterTaskWithMaintenanceWindowResult& RegisterTaskWithMaintenanceWindowResult::operator =(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  JsonView jsonValue = result.GetPayload().View();
  if(jsonValue.ValueExists("WindowTaskId"))
  {
    m_windowTaskId = jsonValue.GetString("WindowTaskId");

  }



  return *this;
}
