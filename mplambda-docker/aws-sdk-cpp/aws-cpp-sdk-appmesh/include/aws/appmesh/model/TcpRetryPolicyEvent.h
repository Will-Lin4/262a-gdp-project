﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/appmesh/AppMesh_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>

namespace Aws
{
namespace AppMesh
{
namespace Model
{
  enum class TcpRetryPolicyEvent
  {
    NOT_SET,
    connection_error
  };

namespace TcpRetryPolicyEventMapper
{
AWS_APPMESH_API TcpRetryPolicyEvent GetTcpRetryPolicyEventForName(const Aws::String& name);

AWS_APPMESH_API Aws::String GetNameForTcpRetryPolicyEvent(TcpRetryPolicyEvent value);
} // namespace TcpRetryPolicyEventMapper
} // namespace Model
} // namespace AppMesh
} // namespace Aws
