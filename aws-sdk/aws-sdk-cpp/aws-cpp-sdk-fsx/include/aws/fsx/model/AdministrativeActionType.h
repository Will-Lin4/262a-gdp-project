﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/fsx/FSx_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>

namespace Aws
{
namespace FSx
{
namespace Model
{
  enum class AdministrativeActionType
  {
    NOT_SET,
    FILE_SYSTEM_UPDATE,
    STORAGE_OPTIMIZATION
  };

namespace AdministrativeActionTypeMapper
{
AWS_FSX_API AdministrativeActionType GetAdministrativeActionTypeForName(const Aws::String& name);

AWS_FSX_API Aws::String GetNameForAdministrativeActionType(AdministrativeActionType value);
} // namespace AdministrativeActionTypeMapper
} // namespace Model
} // namespace FSx
} // namespace Aws
