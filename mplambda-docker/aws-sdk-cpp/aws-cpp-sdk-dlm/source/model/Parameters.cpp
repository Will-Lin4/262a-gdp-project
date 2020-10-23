﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/dlm/model/Parameters.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

namespace Aws
{
namespace DLM
{
namespace Model
{

Parameters::Parameters() : 
    m_excludeBootVolume(false),
    m_excludeBootVolumeHasBeenSet(false)
{
}

Parameters::Parameters(JsonView jsonValue) : 
    m_excludeBootVolume(false),
    m_excludeBootVolumeHasBeenSet(false)
{
  *this = jsonValue;
}

Parameters& Parameters::operator =(JsonView jsonValue)
{
  if(jsonValue.ValueExists("ExcludeBootVolume"))
  {
    m_excludeBootVolume = jsonValue.GetBool("ExcludeBootVolume");

    m_excludeBootVolumeHasBeenSet = true;
  }

  return *this;
}

JsonValue Parameters::Jsonize() const
{
  JsonValue payload;

  if(m_excludeBootVolumeHasBeenSet)
  {
   payload.WithBool("ExcludeBootVolume", m_excludeBootVolume);

  }

  return payload;
}

} // namespace Model
} // namespace DLM
} // namespace Aws
