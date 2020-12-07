﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/autoscaling/AutoScaling_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSStreamFwd.h>
#include <aws/autoscaling/model/LaunchTemplate.h>
#include <aws/autoscaling/model/InstancesDistribution.h>
#include <utility>

namespace Aws
{
namespace Utils
{
namespace Xml
{
  class XmlNode;
} // namespace Xml
} // namespace Utils
namespace AutoScaling
{
namespace Model
{

  /**
   * <p>Describes a mixed instances policy for an Auto Scaling group. With mixed
   * instances, your Auto Scaling group can provision a combination of On-Demand
   * Instances and Spot Instances across multiple instance types. For more
   * information, see <a
   * href="https://docs.aws.amazon.com/autoscaling/ec2/userguide/asg-purchase-options.html">Auto
   * Scaling Groups with Multiple Instance Types and Purchase Options</a> in the
   * <i>Amazon EC2 Auto Scaling User Guide</i>.</p> <p>You can create a mixed
   * instances policy for a new Auto Scaling group, or you can create it for an
   * existing group by updating the group to specify
   * <code>MixedInstancesPolicy</code> as the top-level parameter instead of a launch
   * configuration or launch template. For more information, see
   * <a>CreateAutoScalingGroup</a> and <a>UpdateAutoScalingGroup</a>.</p><p><h3>See
   * Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/autoscaling-2011-01-01/MixedInstancesPolicy">AWS
   * API Reference</a></p>
   */
  class AWS_AUTOSCALING_API MixedInstancesPolicy
  {
  public:
    MixedInstancesPolicy();
    MixedInstancesPolicy(const Aws::Utils::Xml::XmlNode& xmlNode);
    MixedInstancesPolicy& operator=(const Aws::Utils::Xml::XmlNode& xmlNode);

    void OutputToStream(Aws::OStream& ostream, const char* location, unsigned index, const char* locationValue) const;
    void OutputToStream(Aws::OStream& oStream, const char* location) const;


    /**
     * <p>The launch template and instance types (overrides).</p> <p>Required when
     * creating a mixed instances policy.</p>
     */
    inline const LaunchTemplate& GetLaunchTemplate() const{ return m_launchTemplate; }

    /**
     * <p>The launch template and instance types (overrides).</p> <p>Required when
     * creating a mixed instances policy.</p>
     */
    inline bool LaunchTemplateHasBeenSet() const { return m_launchTemplateHasBeenSet; }

    /**
     * <p>The launch template and instance types (overrides).</p> <p>Required when
     * creating a mixed instances policy.</p>
     */
    inline void SetLaunchTemplate(const LaunchTemplate& value) { m_launchTemplateHasBeenSet = true; m_launchTemplate = value; }

    /**
     * <p>The launch template and instance types (overrides).</p> <p>Required when
     * creating a mixed instances policy.</p>
     */
    inline void SetLaunchTemplate(LaunchTemplate&& value) { m_launchTemplateHasBeenSet = true; m_launchTemplate = std::move(value); }

    /**
     * <p>The launch template and instance types (overrides).</p> <p>Required when
     * creating a mixed instances policy.</p>
     */
    inline MixedInstancesPolicy& WithLaunchTemplate(const LaunchTemplate& value) { SetLaunchTemplate(value); return *this;}

    /**
     * <p>The launch template and instance types (overrides).</p> <p>Required when
     * creating a mixed instances policy.</p>
     */
    inline MixedInstancesPolicy& WithLaunchTemplate(LaunchTemplate&& value) { SetLaunchTemplate(std::move(value)); return *this;}


    /**
     * <p>The instances distribution to use.</p> <p>If you leave this parameter
     * unspecified, the value for each parameter in <code>InstancesDistribution</code>
     * uses a default value.</p>
     */
    inline const InstancesDistribution& GetInstancesDistribution() const{ return m_instancesDistribution; }

    /**
     * <p>The instances distribution to use.</p> <p>If you leave this parameter
     * unspecified, the value for each parameter in <code>InstancesDistribution</code>
     * uses a default value.</p>
     */
    inline bool InstancesDistributionHasBeenSet() const { return m_instancesDistributionHasBeenSet; }

    /**
     * <p>The instances distribution to use.</p> <p>If you leave this parameter
     * unspecified, the value for each parameter in <code>InstancesDistribution</code>
     * uses a default value.</p>
     */
    inline void SetInstancesDistribution(const InstancesDistribution& value) { m_instancesDistributionHasBeenSet = true; m_instancesDistribution = value; }

    /**
     * <p>The instances distribution to use.</p> <p>If you leave this parameter
     * unspecified, the value for each parameter in <code>InstancesDistribution</code>
     * uses a default value.</p>
     */
    inline void SetInstancesDistribution(InstancesDistribution&& value) { m_instancesDistributionHasBeenSet = true; m_instancesDistribution = std::move(value); }

    /**
     * <p>The instances distribution to use.</p> <p>If you leave this parameter
     * unspecified, the value for each parameter in <code>InstancesDistribution</code>
     * uses a default value.</p>
     */
    inline MixedInstancesPolicy& WithInstancesDistribution(const InstancesDistribution& value) { SetInstancesDistribution(value); return *this;}

    /**
     * <p>The instances distribution to use.</p> <p>If you leave this parameter
     * unspecified, the value for each parameter in <code>InstancesDistribution</code>
     * uses a default value.</p>
     */
    inline MixedInstancesPolicy& WithInstancesDistribution(InstancesDistribution&& value) { SetInstancesDistribution(std::move(value)); return *this;}

  private:

    LaunchTemplate m_launchTemplate;
    bool m_launchTemplateHasBeenSet;

    InstancesDistribution m_instancesDistribution;
    bool m_instancesDistributionHasBeenSet;
  };

} // namespace Model
} // namespace AutoScaling
} // namespace Aws
