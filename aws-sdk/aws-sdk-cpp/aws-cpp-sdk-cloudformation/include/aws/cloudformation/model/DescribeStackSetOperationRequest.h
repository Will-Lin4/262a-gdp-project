﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/cloudformation/CloudFormation_EXPORTS.h>
#include <aws/cloudformation/CloudFormationRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
namespace CloudFormation
{
namespace Model
{

  /**
   */
  class AWS_CLOUDFORMATION_API DescribeStackSetOperationRequest : public CloudFormationRequest
  {
  public:
    DescribeStackSetOperationRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "DescribeStackSetOperation"; }

    Aws::String SerializePayload() const override;

  protected:
    void DumpBodyToUrl(Aws::Http::URI& uri ) const override;

  public:

    /**
     * <p>The name or the unique stack ID of the stack set for the stack operation.</p>
     */
    inline const Aws::String& GetStackSetName() const{ return m_stackSetName; }

    /**
     * <p>The name or the unique stack ID of the stack set for the stack operation.</p>
     */
    inline bool StackSetNameHasBeenSet() const { return m_stackSetNameHasBeenSet; }

    /**
     * <p>The name or the unique stack ID of the stack set for the stack operation.</p>
     */
    inline void SetStackSetName(const Aws::String& value) { m_stackSetNameHasBeenSet = true; m_stackSetName = value; }

    /**
     * <p>The name or the unique stack ID of the stack set for the stack operation.</p>
     */
    inline void SetStackSetName(Aws::String&& value) { m_stackSetNameHasBeenSet = true; m_stackSetName = std::move(value); }

    /**
     * <p>The name or the unique stack ID of the stack set for the stack operation.</p>
     */
    inline void SetStackSetName(const char* value) { m_stackSetNameHasBeenSet = true; m_stackSetName.assign(value); }

    /**
     * <p>The name or the unique stack ID of the stack set for the stack operation.</p>
     */
    inline DescribeStackSetOperationRequest& WithStackSetName(const Aws::String& value) { SetStackSetName(value); return *this;}

    /**
     * <p>The name or the unique stack ID of the stack set for the stack operation.</p>
     */
    inline DescribeStackSetOperationRequest& WithStackSetName(Aws::String&& value) { SetStackSetName(std::move(value)); return *this;}

    /**
     * <p>The name or the unique stack ID of the stack set for the stack operation.</p>
     */
    inline DescribeStackSetOperationRequest& WithStackSetName(const char* value) { SetStackSetName(value); return *this;}


    /**
     * <p>The unique ID of the stack set operation. </p>
     */
    inline const Aws::String& GetOperationId() const{ return m_operationId; }

    /**
     * <p>The unique ID of the stack set operation. </p>
     */
    inline bool OperationIdHasBeenSet() const { return m_operationIdHasBeenSet; }

    /**
     * <p>The unique ID of the stack set operation. </p>
     */
    inline void SetOperationId(const Aws::String& value) { m_operationIdHasBeenSet = true; m_operationId = value; }

    /**
     * <p>The unique ID of the stack set operation. </p>
     */
    inline void SetOperationId(Aws::String&& value) { m_operationIdHasBeenSet = true; m_operationId = std::move(value); }

    /**
     * <p>The unique ID of the stack set operation. </p>
     */
    inline void SetOperationId(const char* value) { m_operationIdHasBeenSet = true; m_operationId.assign(value); }

    /**
     * <p>The unique ID of the stack set operation. </p>
     */
    inline DescribeStackSetOperationRequest& WithOperationId(const Aws::String& value) { SetOperationId(value); return *this;}

    /**
     * <p>The unique ID of the stack set operation. </p>
     */
    inline DescribeStackSetOperationRequest& WithOperationId(Aws::String&& value) { SetOperationId(std::move(value)); return *this;}

    /**
     * <p>The unique ID of the stack set operation. </p>
     */
    inline DescribeStackSetOperationRequest& WithOperationId(const char* value) { SetOperationId(value); return *this;}

  private:

    Aws::String m_stackSetName;
    bool m_stackSetNameHasBeenSet;

    Aws::String m_operationId;
    bool m_operationIdHasBeenSet;
  };

} // namespace Model
} // namespace CloudFormation
} // namespace Aws
