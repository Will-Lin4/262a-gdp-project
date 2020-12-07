﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/autoscaling/AutoScaling_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSStreamFwd.h>
#include <aws/autoscaling/model/LaunchTemplateSpecification.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/autoscaling/model/LaunchTemplateOverrides.h>
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
   * <p>Describes a launch template and overrides.</p> <p>The overrides are used to
   * override the instance type specified by the launch template with multiple
   * instance types that can be used to launch On-Demand Instances and Spot
   * Instances.</p> <p>When you update the launch template or overrides, existing
   * Amazon EC2 instances continue to run. When scale out occurs, Amazon EC2 Auto
   * Scaling launches instances to match the new settings. When scale in occurs,
   * Amazon EC2 Auto Scaling terminates instances according to the group's
   * termination policies.</p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/autoscaling-2011-01-01/LaunchTemplate">AWS
   * API Reference</a></p>
   */
  class AWS_AUTOSCALING_API LaunchTemplate
  {
  public:
    LaunchTemplate();
    LaunchTemplate(const Aws::Utils::Xml::XmlNode& xmlNode);
    LaunchTemplate& operator=(const Aws::Utils::Xml::XmlNode& xmlNode);

    void OutputToStream(Aws::OStream& ostream, const char* location, unsigned index, const char* locationValue) const;
    void OutputToStream(Aws::OStream& oStream, const char* location) const;


    /**
     * <p>The launch template to use. You must specify either the launch template ID or
     * launch template name in the request.</p>
     */
    inline const LaunchTemplateSpecification& GetLaunchTemplateSpecification() const{ return m_launchTemplateSpecification; }

    /**
     * <p>The launch template to use. You must specify either the launch template ID or
     * launch template name in the request.</p>
     */
    inline bool LaunchTemplateSpecificationHasBeenSet() const { return m_launchTemplateSpecificationHasBeenSet; }

    /**
     * <p>The launch template to use. You must specify either the launch template ID or
     * launch template name in the request.</p>
     */
    inline void SetLaunchTemplateSpecification(const LaunchTemplateSpecification& value) { m_launchTemplateSpecificationHasBeenSet = true; m_launchTemplateSpecification = value; }

    /**
     * <p>The launch template to use. You must specify either the launch template ID or
     * launch template name in the request.</p>
     */
    inline void SetLaunchTemplateSpecification(LaunchTemplateSpecification&& value) { m_launchTemplateSpecificationHasBeenSet = true; m_launchTemplateSpecification = std::move(value); }

    /**
     * <p>The launch template to use. You must specify either the launch template ID or
     * launch template name in the request.</p>
     */
    inline LaunchTemplate& WithLaunchTemplateSpecification(const LaunchTemplateSpecification& value) { SetLaunchTemplateSpecification(value); return *this;}

    /**
     * <p>The launch template to use. You must specify either the launch template ID or
     * launch template name in the request.</p>
     */
    inline LaunchTemplate& WithLaunchTemplateSpecification(LaunchTemplateSpecification&& value) { SetLaunchTemplateSpecification(std::move(value)); return *this;}


    /**
     * <p>Any parameters that you specify override the same parameters in the launch
     * template. Currently, the only supported override is instance type. You can
     * specify between 1 and 20 instance types.</p> <p>If not provided, Amazon EC2 Auto
     * Scaling will use the instance type specified in the launch template to launch
     * instances. </p>
     */
    inline const Aws::Vector<LaunchTemplateOverrides>& GetOverrides() const{ return m_overrides; }

    /**
     * <p>Any parameters that you specify override the same parameters in the launch
     * template. Currently, the only supported override is instance type. You can
     * specify between 1 and 20 instance types.</p> <p>If not provided, Amazon EC2 Auto
     * Scaling will use the instance type specified in the launch template to launch
     * instances. </p>
     */
    inline bool OverridesHasBeenSet() const { return m_overridesHasBeenSet; }

    /**
     * <p>Any parameters that you specify override the same parameters in the launch
     * template. Currently, the only supported override is instance type. You can
     * specify between 1 and 20 instance types.</p> <p>If not provided, Amazon EC2 Auto
     * Scaling will use the instance type specified in the launch template to launch
     * instances. </p>
     */
    inline void SetOverrides(const Aws::Vector<LaunchTemplateOverrides>& value) { m_overridesHasBeenSet = true; m_overrides = value; }

    /**
     * <p>Any parameters that you specify override the same parameters in the launch
     * template. Currently, the only supported override is instance type. You can
     * specify between 1 and 20 instance types.</p> <p>If not provided, Amazon EC2 Auto
     * Scaling will use the instance type specified in the launch template to launch
     * instances. </p>
     */
    inline void SetOverrides(Aws::Vector<LaunchTemplateOverrides>&& value) { m_overridesHasBeenSet = true; m_overrides = std::move(value); }

    /**
     * <p>Any parameters that you specify override the same parameters in the launch
     * template. Currently, the only supported override is instance type. You can
     * specify between 1 and 20 instance types.</p> <p>If not provided, Amazon EC2 Auto
     * Scaling will use the instance type specified in the launch template to launch
     * instances. </p>
     */
    inline LaunchTemplate& WithOverrides(const Aws::Vector<LaunchTemplateOverrides>& value) { SetOverrides(value); return *this;}

    /**
     * <p>Any parameters that you specify override the same parameters in the launch
     * template. Currently, the only supported override is instance type. You can
     * specify between 1 and 20 instance types.</p> <p>If not provided, Amazon EC2 Auto
     * Scaling will use the instance type specified in the launch template to launch
     * instances. </p>
     */
    inline LaunchTemplate& WithOverrides(Aws::Vector<LaunchTemplateOverrides>&& value) { SetOverrides(std::move(value)); return *this;}

    /**
     * <p>Any parameters that you specify override the same parameters in the launch
     * template. Currently, the only supported override is instance type. You can
     * specify between 1 and 20 instance types.</p> <p>If not provided, Amazon EC2 Auto
     * Scaling will use the instance type specified in the launch template to launch
     * instances. </p>
     */
    inline LaunchTemplate& AddOverrides(const LaunchTemplateOverrides& value) { m_overridesHasBeenSet = true; m_overrides.push_back(value); return *this; }

    /**
     * <p>Any parameters that you specify override the same parameters in the launch
     * template. Currently, the only supported override is instance type. You can
     * specify between 1 and 20 instance types.</p> <p>If not provided, Amazon EC2 Auto
     * Scaling will use the instance type specified in the launch template to launch
     * instances. </p>
     */
    inline LaunchTemplate& AddOverrides(LaunchTemplateOverrides&& value) { m_overridesHasBeenSet = true; m_overrides.push_back(std::move(value)); return *this; }

  private:

    LaunchTemplateSpecification m_launchTemplateSpecification;
    bool m_launchTemplateSpecificationHasBeenSet;

    Aws::Vector<LaunchTemplateOverrides> m_overrides;
    bool m_overridesHasBeenSet;
  };

} // namespace Model
} // namespace AutoScaling
} // namespace Aws
