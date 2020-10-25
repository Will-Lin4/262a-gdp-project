﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/elasticloadbalancingv2/ElasticLoadBalancingv2_EXPORTS.h>
#include <aws/elasticloadbalancingv2/ElasticLoadBalancingv2Request.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/elasticloadbalancingv2/model/SubnetMapping.h>
#include <utility>

namespace Aws
{
namespace ElasticLoadBalancingv2
{
namespace Model
{

  /**
   */
  class AWS_ELASTICLOADBALANCINGV2_API SetSubnetsRequest : public ElasticLoadBalancingv2Request
  {
  public:
    SetSubnetsRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "SetSubnets"; }

    Aws::String SerializePayload() const override;

  protected:
    void DumpBodyToUrl(Aws::Http::URI& uri ) const override;

  public:

    /**
     * <p>The Amazon Resource Name (ARN) of the load balancer.</p>
     */
    inline const Aws::String& GetLoadBalancerArn() const{ return m_loadBalancerArn; }

    /**
     * <p>The Amazon Resource Name (ARN) of the load balancer.</p>
     */
    inline bool LoadBalancerArnHasBeenSet() const { return m_loadBalancerArnHasBeenSet; }

    /**
     * <p>The Amazon Resource Name (ARN) of the load balancer.</p>
     */
    inline void SetLoadBalancerArn(const Aws::String& value) { m_loadBalancerArnHasBeenSet = true; m_loadBalancerArn = value; }

    /**
     * <p>The Amazon Resource Name (ARN) of the load balancer.</p>
     */
    inline void SetLoadBalancerArn(Aws::String&& value) { m_loadBalancerArnHasBeenSet = true; m_loadBalancerArn = std::move(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the load balancer.</p>
     */
    inline void SetLoadBalancerArn(const char* value) { m_loadBalancerArnHasBeenSet = true; m_loadBalancerArn.assign(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the load balancer.</p>
     */
    inline SetSubnetsRequest& WithLoadBalancerArn(const Aws::String& value) { SetLoadBalancerArn(value); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the load balancer.</p>
     */
    inline SetSubnetsRequest& WithLoadBalancerArn(Aws::String&& value) { SetLoadBalancerArn(std::move(value)); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the load balancer.</p>
     */
    inline SetSubnetsRequest& WithLoadBalancerArn(const char* value) { SetLoadBalancerArn(value); return *this;}


    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline const Aws::Vector<Aws::String>& GetSubnets() const{ return m_subnets; }

    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline bool SubnetsHasBeenSet() const { return m_subnetsHasBeenSet; }

    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline void SetSubnets(const Aws::Vector<Aws::String>& value) { m_subnetsHasBeenSet = true; m_subnets = value; }

    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline void SetSubnets(Aws::Vector<Aws::String>&& value) { m_subnetsHasBeenSet = true; m_subnets = std::move(value); }

    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline SetSubnetsRequest& WithSubnets(const Aws::Vector<Aws::String>& value) { SetSubnets(value); return *this;}

    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline SetSubnetsRequest& WithSubnets(Aws::Vector<Aws::String>&& value) { SetSubnets(std::move(value)); return *this;}

    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline SetSubnetsRequest& AddSubnets(const Aws::String& value) { m_subnetsHasBeenSet = true; m_subnets.push_back(value); return *this; }

    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline SetSubnetsRequest& AddSubnets(Aws::String&& value) { m_subnetsHasBeenSet = true; m_subnets.push_back(std::move(value)); return *this; }

    /**
     * <p>The IDs of the public subnets. You must specify subnets from at least two
     * Availability Zones. You can specify only one subnet per Availability Zone. You
     * must specify either subnets or subnet mappings.</p>
     */
    inline SetSubnetsRequest& AddSubnets(const char* value) { m_subnetsHasBeenSet = true; m_subnets.push_back(value); return *this; }


    /**
     * <p>The IDs of the public subnets. You can specify only one subnet per
     * Availability Zone. You must specify either subnets or subnet mappings.</p>
     * <p>[Application Load Balancers] You must specify subnets from at least two
     * Availability Zones. You cannot specify Elastic IP addresses for your
     * subnets.</p> <p>[Network Load Balancers] You can specify subnets from one or
     * more Availability Zones. If you need static IP addresses for your
     * internet-facing load balancer, you can specify one Elastic IP address per
     * subnet. For internal load balancers, you can specify one private IP address per
     * subnet from the IPv4 range of the subnet.</p>
     */
    inline const Aws::Vector<SubnetMapping>& GetSubnetMappings() const{ return m_subnetMappings; }

    /**
     * <p>The IDs of the public subnets. You can specify only one subnet per
     * Availability Zone. You must specify either subnets or subnet mappings.</p>
     * <p>[Application Load Balancers] You must specify subnets from at least two
     * Availability Zones. You cannot specify Elastic IP addresses for your
     * subnets.</p> <p>[Network Load Balancers] You can specify subnets from one or
     * more Availability Zones. If you need static IP addresses for your
     * internet-facing load balancer, you can specify one Elastic IP address per
     * subnet. For internal load balancers, you can specify one private IP address per
     * subnet from the IPv4 range of the subnet.</p>
     */
    inline bool SubnetMappingsHasBeenSet() const { return m_subnetMappingsHasBeenSet; }

    /**
     * <p>The IDs of the public subnets. You can specify only one subnet per
     * Availability Zone. You must specify either subnets or subnet mappings.</p>
     * <p>[Application Load Balancers] You must specify subnets from at least two
     * Availability Zones. You cannot specify Elastic IP addresses for your
     * subnets.</p> <p>[Network Load Balancers] You can specify subnets from one or
     * more Availability Zones. If you need static IP addresses for your
     * internet-facing load balancer, you can specify one Elastic IP address per
     * subnet. For internal load balancers, you can specify one private IP address per
     * subnet from the IPv4 range of the subnet.</p>
     */
    inline void SetSubnetMappings(const Aws::Vector<SubnetMapping>& value) { m_subnetMappingsHasBeenSet = true; m_subnetMappings = value; }

    /**
     * <p>The IDs of the public subnets. You can specify only one subnet per
     * Availability Zone. You must specify either subnets or subnet mappings.</p>
     * <p>[Application Load Balancers] You must specify subnets from at least two
     * Availability Zones. You cannot specify Elastic IP addresses for your
     * subnets.</p> <p>[Network Load Balancers] You can specify subnets from one or
     * more Availability Zones. If you need static IP addresses for your
     * internet-facing load balancer, you can specify one Elastic IP address per
     * subnet. For internal load balancers, you can specify one private IP address per
     * subnet from the IPv4 range of the subnet.</p>
     */
    inline void SetSubnetMappings(Aws::Vector<SubnetMapping>&& value) { m_subnetMappingsHasBeenSet = true; m_subnetMappings = std::move(value); }

    /**
     * <p>The IDs of the public subnets. You can specify only one subnet per
     * Availability Zone. You must specify either subnets or subnet mappings.</p>
     * <p>[Application Load Balancers] You must specify subnets from at least two
     * Availability Zones. You cannot specify Elastic IP addresses for your
     * subnets.</p> <p>[Network Load Balancers] You can specify subnets from one or
     * more Availability Zones. If you need static IP addresses for your
     * internet-facing load balancer, you can specify one Elastic IP address per
     * subnet. For internal load balancers, you can specify one private IP address per
     * subnet from the IPv4 range of the subnet.</p>
     */
    inline SetSubnetsRequest& WithSubnetMappings(const Aws::Vector<SubnetMapping>& value) { SetSubnetMappings(value); return *this;}

    /**
     * <p>The IDs of the public subnets. You can specify only one subnet per
     * Availability Zone. You must specify either subnets or subnet mappings.</p>
     * <p>[Application Load Balancers] You must specify subnets from at least two
     * Availability Zones. You cannot specify Elastic IP addresses for your
     * subnets.</p> <p>[Network Load Balancers] You can specify subnets from one or
     * more Availability Zones. If you need static IP addresses for your
     * internet-facing load balancer, you can specify one Elastic IP address per
     * subnet. For internal load balancers, you can specify one private IP address per
     * subnet from the IPv4 range of the subnet.</p>
     */
    inline SetSubnetsRequest& WithSubnetMappings(Aws::Vector<SubnetMapping>&& value) { SetSubnetMappings(std::move(value)); return *this;}

    /**
     * <p>The IDs of the public subnets. You can specify only one subnet per
     * Availability Zone. You must specify either subnets or subnet mappings.</p>
     * <p>[Application Load Balancers] You must specify subnets from at least two
     * Availability Zones. You cannot specify Elastic IP addresses for your
     * subnets.</p> <p>[Network Load Balancers] You can specify subnets from one or
     * more Availability Zones. If you need static IP addresses for your
     * internet-facing load balancer, you can specify one Elastic IP address per
     * subnet. For internal load balancers, you can specify one private IP address per
     * subnet from the IPv4 range of the subnet.</p>
     */
    inline SetSubnetsRequest& AddSubnetMappings(const SubnetMapping& value) { m_subnetMappingsHasBeenSet = true; m_subnetMappings.push_back(value); return *this; }

    /**
     * <p>The IDs of the public subnets. You can specify only one subnet per
     * Availability Zone. You must specify either subnets or subnet mappings.</p>
     * <p>[Application Load Balancers] You must specify subnets from at least two
     * Availability Zones. You cannot specify Elastic IP addresses for your
     * subnets.</p> <p>[Network Load Balancers] You can specify subnets from one or
     * more Availability Zones. If you need static IP addresses for your
     * internet-facing load balancer, you can specify one Elastic IP address per
     * subnet. For internal load balancers, you can specify one private IP address per
     * subnet from the IPv4 range of the subnet.</p>
     */
    inline SetSubnetsRequest& AddSubnetMappings(SubnetMapping&& value) { m_subnetMappingsHasBeenSet = true; m_subnetMappings.push_back(std::move(value)); return *this; }

  private:

    Aws::String m_loadBalancerArn;
    bool m_loadBalancerArnHasBeenSet;

    Aws::Vector<Aws::String> m_subnets;
    bool m_subnetsHasBeenSet;

    Aws::Vector<SubnetMapping> m_subnetMappings;
    bool m_subnetMappingsHasBeenSet;
  };

} // namespace Model
} // namespace ElasticLoadBalancingv2
} // namespace Aws
