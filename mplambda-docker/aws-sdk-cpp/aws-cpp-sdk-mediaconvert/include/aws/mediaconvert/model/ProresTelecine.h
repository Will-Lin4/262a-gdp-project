﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/mediaconvert/MediaConvert_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>

namespace Aws
{
namespace MediaConvert
{
namespace Model
{
  enum class ProresTelecine
  {
    NOT_SET,
    NONE,
    HARD
  };

namespace ProresTelecineMapper
{
AWS_MEDIACONVERT_API ProresTelecine GetProresTelecineForName(const Aws::String& name);

AWS_MEDIACONVERT_API Aws::String GetNameForProresTelecine(ProresTelecine value);
} // namespace ProresTelecineMapper
} // namespace Model
} // namespace MediaConvert
} // namespace Aws
