﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/greengrass/model/FunctionExecutionConfig.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

namespace Aws
{
namespace Greengrass
{
namespace Model
{

FunctionExecutionConfig::FunctionExecutionConfig() : 
    m_isolationMode(FunctionIsolationMode::NOT_SET),
    m_isolationModeHasBeenSet(false),
    m_runAsHasBeenSet(false)
{
}

FunctionExecutionConfig::FunctionExecutionConfig(JsonView jsonValue) : 
    m_isolationMode(FunctionIsolationMode::NOT_SET),
    m_isolationModeHasBeenSet(false),
    m_runAsHasBeenSet(false)
{
  *this = jsonValue;
}

FunctionExecutionConfig& FunctionExecutionConfig::operator =(JsonView jsonValue)
{
  if(jsonValue.ValueExists("IsolationMode"))
  {
    m_isolationMode = FunctionIsolationModeMapper::GetFunctionIsolationModeForName(jsonValue.GetString("IsolationMode"));

    m_isolationModeHasBeenSet = true;
  }

  if(jsonValue.ValueExists("RunAs"))
  {
    m_runAs = jsonValue.GetObject("RunAs");

    m_runAsHasBeenSet = true;
  }

  return *this;
}

JsonValue FunctionExecutionConfig::Jsonize() const
{
  JsonValue payload;

  if(m_isolationModeHasBeenSet)
  {
   payload.WithString("IsolationMode", FunctionIsolationModeMapper::GetNameForFunctionIsolationMode(m_isolationMode));
  }

  if(m_runAsHasBeenSet)
  {
   payload.WithObject("RunAs", m_runAs.Jsonize());

  }

  return payload;
}

} // namespace Model
} // namespace Greengrass
} // namespace Aws
