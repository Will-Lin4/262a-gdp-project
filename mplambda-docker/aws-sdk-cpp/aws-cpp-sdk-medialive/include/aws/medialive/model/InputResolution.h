﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/medialive/MediaLive_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>

namespace Aws
{
namespace MediaLive
{
namespace Model
{
  enum class InputResolution
  {
    NOT_SET,
    SD,
    HD,
    UHD
  };

namespace InputResolutionMapper
{
AWS_MEDIALIVE_API InputResolution GetInputResolutionForName(const Aws::String& name);

AWS_MEDIALIVE_API Aws::String GetNameForInputResolution(InputResolution value);
} // namespace InputResolutionMapper
} // namespace Model
} // namespace MediaLive
} // namespace Aws
