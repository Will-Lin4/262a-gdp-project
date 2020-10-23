﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/autoscaling-plans/AutoScalingPlans_EXPORTS.h>
#include <aws/autoscaling-plans/model/LoadMetricType.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
namespace Utils
{
namespace Json
{
  class JsonValue;
  class JsonView;
} // namespace Json
} // namespace Utils
namespace AutoScalingPlans
{
namespace Model
{

  /**
   * <p>Represents a predefined metric that can be used for predictive scaling.
   * </p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/autoscaling-plans-2018-01-06/PredefinedLoadMetricSpecification">AWS
   * API Reference</a></p>
   */
  class AWS_AUTOSCALINGPLANS_API PredefinedLoadMetricSpecification
  {
  public:
    PredefinedLoadMetricSpecification();
    PredefinedLoadMetricSpecification(Aws::Utils::Json::JsonView jsonValue);
    PredefinedLoadMetricSpecification& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>The metric type.</p>
     */
    inline const LoadMetricType& GetPredefinedLoadMetricType() const{ return m_predefinedLoadMetricType; }

    /**
     * <p>The metric type.</p>
     */
    inline bool PredefinedLoadMetricTypeHasBeenSet() const { return m_predefinedLoadMetricTypeHasBeenSet; }

    /**
     * <p>The metric type.</p>
     */
    inline void SetPredefinedLoadMetricType(const LoadMetricType& value) { m_predefinedLoadMetricTypeHasBeenSet = true; m_predefinedLoadMetricType = value; }

    /**
     * <p>The metric type.</p>
     */
    inline void SetPredefinedLoadMetricType(LoadMetricType&& value) { m_predefinedLoadMetricTypeHasBeenSet = true; m_predefinedLoadMetricType = std::move(value); }

    /**
     * <p>The metric type.</p>
     */
    inline PredefinedLoadMetricSpecification& WithPredefinedLoadMetricType(const LoadMetricType& value) { SetPredefinedLoadMetricType(value); return *this;}

    /**
     * <p>The metric type.</p>
     */
    inline PredefinedLoadMetricSpecification& WithPredefinedLoadMetricType(LoadMetricType&& value) { SetPredefinedLoadMetricType(std::move(value)); return *this;}


    /**
     * <p>Identifies the resource associated with the metric type. You can't specify a
     * resource label unless the metric type is <code>ALBRequestCountPerTarget</code>
     * and there is a target group for an Application Load Balancer attached to the
     * Auto Scaling group.</p> <p>The format is
     * app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;/targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt;,
     * where:</p> <ul> <li> <p>app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;
     * is the final portion of the load balancer ARN.</p> </li> <li>
     * <p>targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt; is the final
     * portion of the target group ARN.</p> </li> </ul>
     */
    inline const Aws::String& GetResourceLabel() const{ return m_resourceLabel; }

    /**
     * <p>Identifies the resource associated with the metric type. You can't specify a
     * resource label unless the metric type is <code>ALBRequestCountPerTarget</code>
     * and there is a target group for an Application Load Balancer attached to the
     * Auto Scaling group.</p> <p>The format is
     * app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;/targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt;,
     * where:</p> <ul> <li> <p>app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;
     * is the final portion of the load balancer ARN.</p> </li> <li>
     * <p>targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt; is the final
     * portion of the target group ARN.</p> </li> </ul>
     */
    inline bool ResourceLabelHasBeenSet() const { return m_resourceLabelHasBeenSet; }

    /**
     * <p>Identifies the resource associated with the metric type. You can't specify a
     * resource label unless the metric type is <code>ALBRequestCountPerTarget</code>
     * and there is a target group for an Application Load Balancer attached to the
     * Auto Scaling group.</p> <p>The format is
     * app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;/targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt;,
     * where:</p> <ul> <li> <p>app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;
     * is the final portion of the load balancer ARN.</p> </li> <li>
     * <p>targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt; is the final
     * portion of the target group ARN.</p> </li> </ul>
     */
    inline void SetResourceLabel(const Aws::String& value) { m_resourceLabelHasBeenSet = true; m_resourceLabel = value; }

    /**
     * <p>Identifies the resource associated with the metric type. You can't specify a
     * resource label unless the metric type is <code>ALBRequestCountPerTarget</code>
     * and there is a target group for an Application Load Balancer attached to the
     * Auto Scaling group.</p> <p>The format is
     * app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;/targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt;,
     * where:</p> <ul> <li> <p>app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;
     * is the final portion of the load balancer ARN.</p> </li> <li>
     * <p>targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt; is the final
     * portion of the target group ARN.</p> </li> </ul>
     */
    inline void SetResourceLabel(Aws::String&& value) { m_resourceLabelHasBeenSet = true; m_resourceLabel = std::move(value); }

    /**
     * <p>Identifies the resource associated with the metric type. You can't specify a
     * resource label unless the metric type is <code>ALBRequestCountPerTarget</code>
     * and there is a target group for an Application Load Balancer attached to the
     * Auto Scaling group.</p> <p>The format is
     * app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;/targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt;,
     * where:</p> <ul> <li> <p>app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;
     * is the final portion of the load balancer ARN.</p> </li> <li>
     * <p>targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt; is the final
     * portion of the target group ARN.</p> </li> </ul>
     */
    inline void SetResourceLabel(const char* value) { m_resourceLabelHasBeenSet = true; m_resourceLabel.assign(value); }

    /**
     * <p>Identifies the resource associated with the metric type. You can't specify a
     * resource label unless the metric type is <code>ALBRequestCountPerTarget</code>
     * and there is a target group for an Application Load Balancer attached to the
     * Auto Scaling group.</p> <p>The format is
     * app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;/targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt;,
     * where:</p> <ul> <li> <p>app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;
     * is the final portion of the load balancer ARN.</p> </li> <li>
     * <p>targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt; is the final
     * portion of the target group ARN.</p> </li> </ul>
     */
    inline PredefinedLoadMetricSpecification& WithResourceLabel(const Aws::String& value) { SetResourceLabel(value); return *this;}

    /**
     * <p>Identifies the resource associated with the metric type. You can't specify a
     * resource label unless the metric type is <code>ALBRequestCountPerTarget</code>
     * and there is a target group for an Application Load Balancer attached to the
     * Auto Scaling group.</p> <p>The format is
     * app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;/targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt;,
     * where:</p> <ul> <li> <p>app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;
     * is the final portion of the load balancer ARN.</p> </li> <li>
     * <p>targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt; is the final
     * portion of the target group ARN.</p> </li> </ul>
     */
    inline PredefinedLoadMetricSpecification& WithResourceLabel(Aws::String&& value) { SetResourceLabel(std::move(value)); return *this;}

    /**
     * <p>Identifies the resource associated with the metric type. You can't specify a
     * resource label unless the metric type is <code>ALBRequestCountPerTarget</code>
     * and there is a target group for an Application Load Balancer attached to the
     * Auto Scaling group.</p> <p>The format is
     * app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;/targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt;,
     * where:</p> <ul> <li> <p>app/&lt;load-balancer-name&gt;/&lt;load-balancer-id&gt;
     * is the final portion of the load balancer ARN.</p> </li> <li>
     * <p>targetgroup/&lt;target-group-name&gt;/&lt;target-group-id&gt; is the final
     * portion of the target group ARN.</p> </li> </ul>
     */
    inline PredefinedLoadMetricSpecification& WithResourceLabel(const char* value) { SetResourceLabel(value); return *this;}

  private:

    LoadMetricType m_predefinedLoadMetricType;
    bool m_predefinedLoadMetricTypeHasBeenSet;

    Aws::String m_resourceLabel;
    bool m_resourceLabelHasBeenSet;
  };

} // namespace Model
} // namespace AutoScalingPlans
} // namespace Aws
