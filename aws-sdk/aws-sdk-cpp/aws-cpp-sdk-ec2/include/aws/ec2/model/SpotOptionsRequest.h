﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/ec2/EC2_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSStreamFwd.h>
#include <aws/ec2/model/SpotAllocationStrategy.h>
#include <aws/ec2/model/SpotInstanceInterruptionBehavior.h>
#include <aws/core/utils/memory/stl/AWSString.h>
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
namespace EC2
{
namespace Model
{

  /**
   * <p>Describes the configuration of Spot Instances in an EC2 Fleet
   * request.</p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/ec2-2016-11-15/SpotOptionsRequest">AWS
   * API Reference</a></p>
   */
  class AWS_EC2_API SpotOptionsRequest
  {
  public:
    SpotOptionsRequest();
    SpotOptionsRequest(const Aws::Utils::Xml::XmlNode& xmlNode);
    SpotOptionsRequest& operator=(const Aws::Utils::Xml::XmlNode& xmlNode);

    void OutputToStream(Aws::OStream& ostream, const char* location, unsigned index, const char* locationValue) const;
    void OutputToStream(Aws::OStream& oStream, const char* location) const;


    /**
     * <p>Indicates how to allocate the target Spot Instance capacity across the Spot
     * Instance pools specified by the EC2 Fleet.</p> <p>If the allocation strategy is
     * <code>lowest-price</code>, EC2 Fleet launches instances from the Spot Instance
     * pools with the lowest price. This is the default allocation strategy.</p> <p>If
     * the allocation strategy is <code>diversified</code>, EC2 Fleet launches
     * instances from all of the Spot Instance pools that you specify.</p> <p>If the
     * allocation strategy is <code>capacity-optimized</code>, EC2 Fleet launches
     * instances from Spot Instance pools with optimal capacity for the number of
     * instances that are launching.</p>
     */
    inline const SpotAllocationStrategy& GetAllocationStrategy() const{ return m_allocationStrategy; }

    /**
     * <p>Indicates how to allocate the target Spot Instance capacity across the Spot
     * Instance pools specified by the EC2 Fleet.</p> <p>If the allocation strategy is
     * <code>lowest-price</code>, EC2 Fleet launches instances from the Spot Instance
     * pools with the lowest price. This is the default allocation strategy.</p> <p>If
     * the allocation strategy is <code>diversified</code>, EC2 Fleet launches
     * instances from all of the Spot Instance pools that you specify.</p> <p>If the
     * allocation strategy is <code>capacity-optimized</code>, EC2 Fleet launches
     * instances from Spot Instance pools with optimal capacity for the number of
     * instances that are launching.</p>
     */
    inline bool AllocationStrategyHasBeenSet() const { return m_allocationStrategyHasBeenSet; }

    /**
     * <p>Indicates how to allocate the target Spot Instance capacity across the Spot
     * Instance pools specified by the EC2 Fleet.</p> <p>If the allocation strategy is
     * <code>lowest-price</code>, EC2 Fleet launches instances from the Spot Instance
     * pools with the lowest price. This is the default allocation strategy.</p> <p>If
     * the allocation strategy is <code>diversified</code>, EC2 Fleet launches
     * instances from all of the Spot Instance pools that you specify.</p> <p>If the
     * allocation strategy is <code>capacity-optimized</code>, EC2 Fleet launches
     * instances from Spot Instance pools with optimal capacity for the number of
     * instances that are launching.</p>
     */
    inline void SetAllocationStrategy(const SpotAllocationStrategy& value) { m_allocationStrategyHasBeenSet = true; m_allocationStrategy = value; }

    /**
     * <p>Indicates how to allocate the target Spot Instance capacity across the Spot
     * Instance pools specified by the EC2 Fleet.</p> <p>If the allocation strategy is
     * <code>lowest-price</code>, EC2 Fleet launches instances from the Spot Instance
     * pools with the lowest price. This is the default allocation strategy.</p> <p>If
     * the allocation strategy is <code>diversified</code>, EC2 Fleet launches
     * instances from all of the Spot Instance pools that you specify.</p> <p>If the
     * allocation strategy is <code>capacity-optimized</code>, EC2 Fleet launches
     * instances from Spot Instance pools with optimal capacity for the number of
     * instances that are launching.</p>
     */
    inline void SetAllocationStrategy(SpotAllocationStrategy&& value) { m_allocationStrategyHasBeenSet = true; m_allocationStrategy = std::move(value); }

    /**
     * <p>Indicates how to allocate the target Spot Instance capacity across the Spot
     * Instance pools specified by the EC2 Fleet.</p> <p>If the allocation strategy is
     * <code>lowest-price</code>, EC2 Fleet launches instances from the Spot Instance
     * pools with the lowest price. This is the default allocation strategy.</p> <p>If
     * the allocation strategy is <code>diversified</code>, EC2 Fleet launches
     * instances from all of the Spot Instance pools that you specify.</p> <p>If the
     * allocation strategy is <code>capacity-optimized</code>, EC2 Fleet launches
     * instances from Spot Instance pools with optimal capacity for the number of
     * instances that are launching.</p>
     */
    inline SpotOptionsRequest& WithAllocationStrategy(const SpotAllocationStrategy& value) { SetAllocationStrategy(value); return *this;}

    /**
     * <p>Indicates how to allocate the target Spot Instance capacity across the Spot
     * Instance pools specified by the EC2 Fleet.</p> <p>If the allocation strategy is
     * <code>lowest-price</code>, EC2 Fleet launches instances from the Spot Instance
     * pools with the lowest price. This is the default allocation strategy.</p> <p>If
     * the allocation strategy is <code>diversified</code>, EC2 Fleet launches
     * instances from all of the Spot Instance pools that you specify.</p> <p>If the
     * allocation strategy is <code>capacity-optimized</code>, EC2 Fleet launches
     * instances from Spot Instance pools with optimal capacity for the number of
     * instances that are launching.</p>
     */
    inline SpotOptionsRequest& WithAllocationStrategy(SpotAllocationStrategy&& value) { SetAllocationStrategy(std::move(value)); return *this;}


    /**
     * <p>The behavior when a Spot Instance is interrupted. The default is
     * <code>terminate</code>.</p>
     */
    inline const SpotInstanceInterruptionBehavior& GetInstanceInterruptionBehavior() const{ return m_instanceInterruptionBehavior; }

    /**
     * <p>The behavior when a Spot Instance is interrupted. The default is
     * <code>terminate</code>.</p>
     */
    inline bool InstanceInterruptionBehaviorHasBeenSet() const { return m_instanceInterruptionBehaviorHasBeenSet; }

    /**
     * <p>The behavior when a Spot Instance is interrupted. The default is
     * <code>terminate</code>.</p>
     */
    inline void SetInstanceInterruptionBehavior(const SpotInstanceInterruptionBehavior& value) { m_instanceInterruptionBehaviorHasBeenSet = true; m_instanceInterruptionBehavior = value; }

    /**
     * <p>The behavior when a Spot Instance is interrupted. The default is
     * <code>terminate</code>.</p>
     */
    inline void SetInstanceInterruptionBehavior(SpotInstanceInterruptionBehavior&& value) { m_instanceInterruptionBehaviorHasBeenSet = true; m_instanceInterruptionBehavior = std::move(value); }

    /**
     * <p>The behavior when a Spot Instance is interrupted. The default is
     * <code>terminate</code>.</p>
     */
    inline SpotOptionsRequest& WithInstanceInterruptionBehavior(const SpotInstanceInterruptionBehavior& value) { SetInstanceInterruptionBehavior(value); return *this;}

    /**
     * <p>The behavior when a Spot Instance is interrupted. The default is
     * <code>terminate</code>.</p>
     */
    inline SpotOptionsRequest& WithInstanceInterruptionBehavior(SpotInstanceInterruptionBehavior&& value) { SetInstanceInterruptionBehavior(std::move(value)); return *this;}


    /**
     * <p>The number of Spot pools across which to allocate your target Spot capacity.
     * Valid only when Spot <b>AllocationStrategy</b> is set to
     * <code>lowest-price</code>. EC2 Fleet selects the cheapest Spot pools and evenly
     * allocates your target Spot capacity across the number of Spot pools that you
     * specify.</p>
     */
    inline int GetInstancePoolsToUseCount() const{ return m_instancePoolsToUseCount; }

    /**
     * <p>The number of Spot pools across which to allocate your target Spot capacity.
     * Valid only when Spot <b>AllocationStrategy</b> is set to
     * <code>lowest-price</code>. EC2 Fleet selects the cheapest Spot pools and evenly
     * allocates your target Spot capacity across the number of Spot pools that you
     * specify.</p>
     */
    inline bool InstancePoolsToUseCountHasBeenSet() const { return m_instancePoolsToUseCountHasBeenSet; }

    /**
     * <p>The number of Spot pools across which to allocate your target Spot capacity.
     * Valid only when Spot <b>AllocationStrategy</b> is set to
     * <code>lowest-price</code>. EC2 Fleet selects the cheapest Spot pools and evenly
     * allocates your target Spot capacity across the number of Spot pools that you
     * specify.</p>
     */
    inline void SetInstancePoolsToUseCount(int value) { m_instancePoolsToUseCountHasBeenSet = true; m_instancePoolsToUseCount = value; }

    /**
     * <p>The number of Spot pools across which to allocate your target Spot capacity.
     * Valid only when Spot <b>AllocationStrategy</b> is set to
     * <code>lowest-price</code>. EC2 Fleet selects the cheapest Spot pools and evenly
     * allocates your target Spot capacity across the number of Spot pools that you
     * specify.</p>
     */
    inline SpotOptionsRequest& WithInstancePoolsToUseCount(int value) { SetInstancePoolsToUseCount(value); return *this;}


    /**
     * <p>Indicates that the fleet uses a single instance type to launch all Spot
     * Instances in the fleet. Supported only for fleets of type
     * <code>instant</code>.</p>
     */
    inline bool GetSingleInstanceType() const{ return m_singleInstanceType; }

    /**
     * <p>Indicates that the fleet uses a single instance type to launch all Spot
     * Instances in the fleet. Supported only for fleets of type
     * <code>instant</code>.</p>
     */
    inline bool SingleInstanceTypeHasBeenSet() const { return m_singleInstanceTypeHasBeenSet; }

    /**
     * <p>Indicates that the fleet uses a single instance type to launch all Spot
     * Instances in the fleet. Supported only for fleets of type
     * <code>instant</code>.</p>
     */
    inline void SetSingleInstanceType(bool value) { m_singleInstanceTypeHasBeenSet = true; m_singleInstanceType = value; }

    /**
     * <p>Indicates that the fleet uses a single instance type to launch all Spot
     * Instances in the fleet. Supported only for fleets of type
     * <code>instant</code>.</p>
     */
    inline SpotOptionsRequest& WithSingleInstanceType(bool value) { SetSingleInstanceType(value); return *this;}


    /**
     * <p>Indicates that the fleet launches all Spot Instances into a single
     * Availability Zone. Supported only for fleets of type <code>instant</code>.</p>
     */
    inline bool GetSingleAvailabilityZone() const{ return m_singleAvailabilityZone; }

    /**
     * <p>Indicates that the fleet launches all Spot Instances into a single
     * Availability Zone. Supported only for fleets of type <code>instant</code>.</p>
     */
    inline bool SingleAvailabilityZoneHasBeenSet() const { return m_singleAvailabilityZoneHasBeenSet; }

    /**
     * <p>Indicates that the fleet launches all Spot Instances into a single
     * Availability Zone. Supported only for fleets of type <code>instant</code>.</p>
     */
    inline void SetSingleAvailabilityZone(bool value) { m_singleAvailabilityZoneHasBeenSet = true; m_singleAvailabilityZone = value; }

    /**
     * <p>Indicates that the fleet launches all Spot Instances into a single
     * Availability Zone. Supported only for fleets of type <code>instant</code>.</p>
     */
    inline SpotOptionsRequest& WithSingleAvailabilityZone(bool value) { SetSingleAvailabilityZone(value); return *this;}


    /**
     * <p>The minimum target capacity for Spot Instances in the fleet. If the minimum
     * target capacity is not reached, the fleet launches no instances.</p>
     */
    inline int GetMinTargetCapacity() const{ return m_minTargetCapacity; }

    /**
     * <p>The minimum target capacity for Spot Instances in the fleet. If the minimum
     * target capacity is not reached, the fleet launches no instances.</p>
     */
    inline bool MinTargetCapacityHasBeenSet() const { return m_minTargetCapacityHasBeenSet; }

    /**
     * <p>The minimum target capacity for Spot Instances in the fleet. If the minimum
     * target capacity is not reached, the fleet launches no instances.</p>
     */
    inline void SetMinTargetCapacity(int value) { m_minTargetCapacityHasBeenSet = true; m_minTargetCapacity = value; }

    /**
     * <p>The minimum target capacity for Spot Instances in the fleet. If the minimum
     * target capacity is not reached, the fleet launches no instances.</p>
     */
    inline SpotOptionsRequest& WithMinTargetCapacity(int value) { SetMinTargetCapacity(value); return *this;}


    /**
     * <p>The maximum amount per hour for Spot Instances that you're willing to
     * pay.</p>
     */
    inline const Aws::String& GetMaxTotalPrice() const{ return m_maxTotalPrice; }

    /**
     * <p>The maximum amount per hour for Spot Instances that you're willing to
     * pay.</p>
     */
    inline bool MaxTotalPriceHasBeenSet() const { return m_maxTotalPriceHasBeenSet; }

    /**
     * <p>The maximum amount per hour for Spot Instances that you're willing to
     * pay.</p>
     */
    inline void SetMaxTotalPrice(const Aws::String& value) { m_maxTotalPriceHasBeenSet = true; m_maxTotalPrice = value; }

    /**
     * <p>The maximum amount per hour for Spot Instances that you're willing to
     * pay.</p>
     */
    inline void SetMaxTotalPrice(Aws::String&& value) { m_maxTotalPriceHasBeenSet = true; m_maxTotalPrice = std::move(value); }

    /**
     * <p>The maximum amount per hour for Spot Instances that you're willing to
     * pay.</p>
     */
    inline void SetMaxTotalPrice(const char* value) { m_maxTotalPriceHasBeenSet = true; m_maxTotalPrice.assign(value); }

    /**
     * <p>The maximum amount per hour for Spot Instances that you're willing to
     * pay.</p>
     */
    inline SpotOptionsRequest& WithMaxTotalPrice(const Aws::String& value) { SetMaxTotalPrice(value); return *this;}

    /**
     * <p>The maximum amount per hour for Spot Instances that you're willing to
     * pay.</p>
     */
    inline SpotOptionsRequest& WithMaxTotalPrice(Aws::String&& value) { SetMaxTotalPrice(std::move(value)); return *this;}

    /**
     * <p>The maximum amount per hour for Spot Instances that you're willing to
     * pay.</p>
     */
    inline SpotOptionsRequest& WithMaxTotalPrice(const char* value) { SetMaxTotalPrice(value); return *this;}

  private:

    SpotAllocationStrategy m_allocationStrategy;
    bool m_allocationStrategyHasBeenSet;

    SpotInstanceInterruptionBehavior m_instanceInterruptionBehavior;
    bool m_instanceInterruptionBehaviorHasBeenSet;

    int m_instancePoolsToUseCount;
    bool m_instancePoolsToUseCountHasBeenSet;

    bool m_singleInstanceType;
    bool m_singleInstanceTypeHasBeenSet;

    bool m_singleAvailabilityZone;
    bool m_singleAvailabilityZoneHasBeenSet;

    int m_minTargetCapacity;
    bool m_minTargetCapacityHasBeenSet;

    Aws::String m_maxTotalPrice;
    bool m_maxTotalPriceHasBeenSet;
  };

} // namespace Model
} // namespace EC2
} // namespace Aws
