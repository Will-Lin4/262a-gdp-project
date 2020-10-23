﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/ec2/EC2_EXPORTS.h>
#include <aws/ec2/EC2Request.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/ec2/model/ConnectionLogOptions.h>
#include <aws/ec2/model/DnsServersOptionsModifyStructure.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <utility>

namespace Aws
{
namespace EC2
{
namespace Model
{

  /**
   */
  class AWS_EC2_API ModifyClientVpnEndpointRequest : public EC2Request
  {
  public:
    ModifyClientVpnEndpointRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "ModifyClientVpnEndpoint"; }

    Aws::String SerializePayload() const override;

  protected:
    void DumpBodyToUrl(Aws::Http::URI& uri ) const override;

  public:

    /**
     * <p>The ID of the Client VPN endpoint to modify.</p>
     */
    inline const Aws::String& GetClientVpnEndpointId() const{ return m_clientVpnEndpointId; }

    /**
     * <p>The ID of the Client VPN endpoint to modify.</p>
     */
    inline bool ClientVpnEndpointIdHasBeenSet() const { return m_clientVpnEndpointIdHasBeenSet; }

    /**
     * <p>The ID of the Client VPN endpoint to modify.</p>
     */
    inline void SetClientVpnEndpointId(const Aws::String& value) { m_clientVpnEndpointIdHasBeenSet = true; m_clientVpnEndpointId = value; }

    /**
     * <p>The ID of the Client VPN endpoint to modify.</p>
     */
    inline void SetClientVpnEndpointId(Aws::String&& value) { m_clientVpnEndpointIdHasBeenSet = true; m_clientVpnEndpointId = std::move(value); }

    /**
     * <p>The ID of the Client VPN endpoint to modify.</p>
     */
    inline void SetClientVpnEndpointId(const char* value) { m_clientVpnEndpointIdHasBeenSet = true; m_clientVpnEndpointId.assign(value); }

    /**
     * <p>The ID of the Client VPN endpoint to modify.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithClientVpnEndpointId(const Aws::String& value) { SetClientVpnEndpointId(value); return *this;}

    /**
     * <p>The ID of the Client VPN endpoint to modify.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithClientVpnEndpointId(Aws::String&& value) { SetClientVpnEndpointId(std::move(value)); return *this;}

    /**
     * <p>The ID of the Client VPN endpoint to modify.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithClientVpnEndpointId(const char* value) { SetClientVpnEndpointId(value); return *this;}


    /**
     * <p>The ARN of the server certificate to be used. The server certificate must be
     * provisioned in AWS Certificate Manager (ACM).</p>
     */
    inline const Aws::String& GetServerCertificateArn() const{ return m_serverCertificateArn; }

    /**
     * <p>The ARN of the server certificate to be used. The server certificate must be
     * provisioned in AWS Certificate Manager (ACM).</p>
     */
    inline bool ServerCertificateArnHasBeenSet() const { return m_serverCertificateArnHasBeenSet; }

    /**
     * <p>The ARN of the server certificate to be used. The server certificate must be
     * provisioned in AWS Certificate Manager (ACM).</p>
     */
    inline void SetServerCertificateArn(const Aws::String& value) { m_serverCertificateArnHasBeenSet = true; m_serverCertificateArn = value; }

    /**
     * <p>The ARN of the server certificate to be used. The server certificate must be
     * provisioned in AWS Certificate Manager (ACM).</p>
     */
    inline void SetServerCertificateArn(Aws::String&& value) { m_serverCertificateArnHasBeenSet = true; m_serverCertificateArn = std::move(value); }

    /**
     * <p>The ARN of the server certificate to be used. The server certificate must be
     * provisioned in AWS Certificate Manager (ACM).</p>
     */
    inline void SetServerCertificateArn(const char* value) { m_serverCertificateArnHasBeenSet = true; m_serverCertificateArn.assign(value); }

    /**
     * <p>The ARN of the server certificate to be used. The server certificate must be
     * provisioned in AWS Certificate Manager (ACM).</p>
     */
    inline ModifyClientVpnEndpointRequest& WithServerCertificateArn(const Aws::String& value) { SetServerCertificateArn(value); return *this;}

    /**
     * <p>The ARN of the server certificate to be used. The server certificate must be
     * provisioned in AWS Certificate Manager (ACM).</p>
     */
    inline ModifyClientVpnEndpointRequest& WithServerCertificateArn(Aws::String&& value) { SetServerCertificateArn(std::move(value)); return *this;}

    /**
     * <p>The ARN of the server certificate to be used. The server certificate must be
     * provisioned in AWS Certificate Manager (ACM).</p>
     */
    inline ModifyClientVpnEndpointRequest& WithServerCertificateArn(const char* value) { SetServerCertificateArn(value); return *this;}


    /**
     * <p>Information about the client connection logging options.</p> <p>If you enable
     * client connection logging, data about client connections is sent to a Cloudwatch
     * Logs log stream. The following information is logged:</p> <ul> <li> <p>Client
     * connection requests</p> </li> <li> <p>Client connection results (successful and
     * unsuccessful)</p> </li> <li> <p>Reasons for unsuccessful client connection
     * requests</p> </li> <li> <p>Client connection termination time</p> </li> </ul>
     */
    inline const ConnectionLogOptions& GetConnectionLogOptions() const{ return m_connectionLogOptions; }

    /**
     * <p>Information about the client connection logging options.</p> <p>If you enable
     * client connection logging, data about client connections is sent to a Cloudwatch
     * Logs log stream. The following information is logged:</p> <ul> <li> <p>Client
     * connection requests</p> </li> <li> <p>Client connection results (successful and
     * unsuccessful)</p> </li> <li> <p>Reasons for unsuccessful client connection
     * requests</p> </li> <li> <p>Client connection termination time</p> </li> </ul>
     */
    inline bool ConnectionLogOptionsHasBeenSet() const { return m_connectionLogOptionsHasBeenSet; }

    /**
     * <p>Information about the client connection logging options.</p> <p>If you enable
     * client connection logging, data about client connections is sent to a Cloudwatch
     * Logs log stream. The following information is logged:</p> <ul> <li> <p>Client
     * connection requests</p> </li> <li> <p>Client connection results (successful and
     * unsuccessful)</p> </li> <li> <p>Reasons for unsuccessful client connection
     * requests</p> </li> <li> <p>Client connection termination time</p> </li> </ul>
     */
    inline void SetConnectionLogOptions(const ConnectionLogOptions& value) { m_connectionLogOptionsHasBeenSet = true; m_connectionLogOptions = value; }

    /**
     * <p>Information about the client connection logging options.</p> <p>If you enable
     * client connection logging, data about client connections is sent to a Cloudwatch
     * Logs log stream. The following information is logged:</p> <ul> <li> <p>Client
     * connection requests</p> </li> <li> <p>Client connection results (successful and
     * unsuccessful)</p> </li> <li> <p>Reasons for unsuccessful client connection
     * requests</p> </li> <li> <p>Client connection termination time</p> </li> </ul>
     */
    inline void SetConnectionLogOptions(ConnectionLogOptions&& value) { m_connectionLogOptionsHasBeenSet = true; m_connectionLogOptions = std::move(value); }

    /**
     * <p>Information about the client connection logging options.</p> <p>If you enable
     * client connection logging, data about client connections is sent to a Cloudwatch
     * Logs log stream. The following information is logged:</p> <ul> <li> <p>Client
     * connection requests</p> </li> <li> <p>Client connection results (successful and
     * unsuccessful)</p> </li> <li> <p>Reasons for unsuccessful client connection
     * requests</p> </li> <li> <p>Client connection termination time</p> </li> </ul>
     */
    inline ModifyClientVpnEndpointRequest& WithConnectionLogOptions(const ConnectionLogOptions& value) { SetConnectionLogOptions(value); return *this;}

    /**
     * <p>Information about the client connection logging options.</p> <p>If you enable
     * client connection logging, data about client connections is sent to a Cloudwatch
     * Logs log stream. The following information is logged:</p> <ul> <li> <p>Client
     * connection requests</p> </li> <li> <p>Client connection results (successful and
     * unsuccessful)</p> </li> <li> <p>Reasons for unsuccessful client connection
     * requests</p> </li> <li> <p>Client connection termination time</p> </li> </ul>
     */
    inline ModifyClientVpnEndpointRequest& WithConnectionLogOptions(ConnectionLogOptions&& value) { SetConnectionLogOptions(std::move(value)); return *this;}


    /**
     * <p>Information about the DNS servers to be used by Client VPN connections. A
     * Client VPN endpoint can have up to two DNS servers.</p>
     */
    inline const DnsServersOptionsModifyStructure& GetDnsServers() const{ return m_dnsServers; }

    /**
     * <p>Information about the DNS servers to be used by Client VPN connections. A
     * Client VPN endpoint can have up to two DNS servers.</p>
     */
    inline bool DnsServersHasBeenSet() const { return m_dnsServersHasBeenSet; }

    /**
     * <p>Information about the DNS servers to be used by Client VPN connections. A
     * Client VPN endpoint can have up to two DNS servers.</p>
     */
    inline void SetDnsServers(const DnsServersOptionsModifyStructure& value) { m_dnsServersHasBeenSet = true; m_dnsServers = value; }

    /**
     * <p>Information about the DNS servers to be used by Client VPN connections. A
     * Client VPN endpoint can have up to two DNS servers.</p>
     */
    inline void SetDnsServers(DnsServersOptionsModifyStructure&& value) { m_dnsServersHasBeenSet = true; m_dnsServers = std::move(value); }

    /**
     * <p>Information about the DNS servers to be used by Client VPN connections. A
     * Client VPN endpoint can have up to two DNS servers.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithDnsServers(const DnsServersOptionsModifyStructure& value) { SetDnsServers(value); return *this;}

    /**
     * <p>Information about the DNS servers to be used by Client VPN connections. A
     * Client VPN endpoint can have up to two DNS servers.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithDnsServers(DnsServersOptionsModifyStructure&& value) { SetDnsServers(std::move(value)); return *this;}


    /**
     * <p>The port number to assign to the Client VPN endpoint for TCP and UDP
     * traffic.</p> <p>Valid Values: <code>443</code> | <code>1194</code> </p>
     * <p>Default Value: <code>443</code> </p>
     */
    inline int GetVpnPort() const{ return m_vpnPort; }

    /**
     * <p>The port number to assign to the Client VPN endpoint for TCP and UDP
     * traffic.</p> <p>Valid Values: <code>443</code> | <code>1194</code> </p>
     * <p>Default Value: <code>443</code> </p>
     */
    inline bool VpnPortHasBeenSet() const { return m_vpnPortHasBeenSet; }

    /**
     * <p>The port number to assign to the Client VPN endpoint for TCP and UDP
     * traffic.</p> <p>Valid Values: <code>443</code> | <code>1194</code> </p>
     * <p>Default Value: <code>443</code> </p>
     */
    inline void SetVpnPort(int value) { m_vpnPortHasBeenSet = true; m_vpnPort = value; }

    /**
     * <p>The port number to assign to the Client VPN endpoint for TCP and UDP
     * traffic.</p> <p>Valid Values: <code>443</code> | <code>1194</code> </p>
     * <p>Default Value: <code>443</code> </p>
     */
    inline ModifyClientVpnEndpointRequest& WithVpnPort(int value) { SetVpnPort(value); return *this;}


    /**
     * <p>A brief description of the Client VPN endpoint.</p>
     */
    inline const Aws::String& GetDescription() const{ return m_description; }

    /**
     * <p>A brief description of the Client VPN endpoint.</p>
     */
    inline bool DescriptionHasBeenSet() const { return m_descriptionHasBeenSet; }

    /**
     * <p>A brief description of the Client VPN endpoint.</p>
     */
    inline void SetDescription(const Aws::String& value) { m_descriptionHasBeenSet = true; m_description = value; }

    /**
     * <p>A brief description of the Client VPN endpoint.</p>
     */
    inline void SetDescription(Aws::String&& value) { m_descriptionHasBeenSet = true; m_description = std::move(value); }

    /**
     * <p>A brief description of the Client VPN endpoint.</p>
     */
    inline void SetDescription(const char* value) { m_descriptionHasBeenSet = true; m_description.assign(value); }

    /**
     * <p>A brief description of the Client VPN endpoint.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithDescription(const Aws::String& value) { SetDescription(value); return *this;}

    /**
     * <p>A brief description of the Client VPN endpoint.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithDescription(Aws::String&& value) { SetDescription(std::move(value)); return *this;}

    /**
     * <p>A brief description of the Client VPN endpoint.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithDescription(const char* value) { SetDescription(value); return *this;}


    /**
     * <p>Indicates whether the VPN is split-tunnel.</p> <p>For information about
     * split-tunnel VPN endpoints, see <a
     * href="https://docs.aws.amazon.com/vpn/latest/clientvpn-admin/split-tunnel-vpn.html">Split-Tunnel
     * AWS Client VPN Endpoint</a> in the <i>AWS Client VPN Administrator
     * Guide</i>.</p>
     */
    inline bool GetSplitTunnel() const{ return m_splitTunnel; }

    /**
     * <p>Indicates whether the VPN is split-tunnel.</p> <p>For information about
     * split-tunnel VPN endpoints, see <a
     * href="https://docs.aws.amazon.com/vpn/latest/clientvpn-admin/split-tunnel-vpn.html">Split-Tunnel
     * AWS Client VPN Endpoint</a> in the <i>AWS Client VPN Administrator
     * Guide</i>.</p>
     */
    inline bool SplitTunnelHasBeenSet() const { return m_splitTunnelHasBeenSet; }

    /**
     * <p>Indicates whether the VPN is split-tunnel.</p> <p>For information about
     * split-tunnel VPN endpoints, see <a
     * href="https://docs.aws.amazon.com/vpn/latest/clientvpn-admin/split-tunnel-vpn.html">Split-Tunnel
     * AWS Client VPN Endpoint</a> in the <i>AWS Client VPN Administrator
     * Guide</i>.</p>
     */
    inline void SetSplitTunnel(bool value) { m_splitTunnelHasBeenSet = true; m_splitTunnel = value; }

    /**
     * <p>Indicates whether the VPN is split-tunnel.</p> <p>For information about
     * split-tunnel VPN endpoints, see <a
     * href="https://docs.aws.amazon.com/vpn/latest/clientvpn-admin/split-tunnel-vpn.html">Split-Tunnel
     * AWS Client VPN Endpoint</a> in the <i>AWS Client VPN Administrator
     * Guide</i>.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithSplitTunnel(bool value) { SetSplitTunnel(value); return *this;}


    /**
     * <p>Checks whether you have the required permissions for the action, without
     * actually making the request, and provides an error response. If you have the
     * required permissions, the error response is <code>DryRunOperation</code>.
     * Otherwise, it is <code>UnauthorizedOperation</code>.</p>
     */
    inline bool GetDryRun() const{ return m_dryRun; }

    /**
     * <p>Checks whether you have the required permissions for the action, without
     * actually making the request, and provides an error response. If you have the
     * required permissions, the error response is <code>DryRunOperation</code>.
     * Otherwise, it is <code>UnauthorizedOperation</code>.</p>
     */
    inline bool DryRunHasBeenSet() const { return m_dryRunHasBeenSet; }

    /**
     * <p>Checks whether you have the required permissions for the action, without
     * actually making the request, and provides an error response. If you have the
     * required permissions, the error response is <code>DryRunOperation</code>.
     * Otherwise, it is <code>UnauthorizedOperation</code>.</p>
     */
    inline void SetDryRun(bool value) { m_dryRunHasBeenSet = true; m_dryRun = value; }

    /**
     * <p>Checks whether you have the required permissions for the action, without
     * actually making the request, and provides an error response. If you have the
     * required permissions, the error response is <code>DryRunOperation</code>.
     * Otherwise, it is <code>UnauthorizedOperation</code>.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithDryRun(bool value) { SetDryRun(value); return *this;}


    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline const Aws::Vector<Aws::String>& GetSecurityGroupIds() const{ return m_securityGroupIds; }

    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline bool SecurityGroupIdsHasBeenSet() const { return m_securityGroupIdsHasBeenSet; }

    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline void SetSecurityGroupIds(const Aws::Vector<Aws::String>& value) { m_securityGroupIdsHasBeenSet = true; m_securityGroupIds = value; }

    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline void SetSecurityGroupIds(Aws::Vector<Aws::String>&& value) { m_securityGroupIdsHasBeenSet = true; m_securityGroupIds = std::move(value); }

    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithSecurityGroupIds(const Aws::Vector<Aws::String>& value) { SetSecurityGroupIds(value); return *this;}

    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithSecurityGroupIds(Aws::Vector<Aws::String>&& value) { SetSecurityGroupIds(std::move(value)); return *this;}

    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline ModifyClientVpnEndpointRequest& AddSecurityGroupIds(const Aws::String& value) { m_securityGroupIdsHasBeenSet = true; m_securityGroupIds.push_back(value); return *this; }

    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline ModifyClientVpnEndpointRequest& AddSecurityGroupIds(Aws::String&& value) { m_securityGroupIdsHasBeenSet = true; m_securityGroupIds.push_back(std::move(value)); return *this; }

    /**
     * <p>The IDs of one or more security groups to apply to the target network.</p>
     */
    inline ModifyClientVpnEndpointRequest& AddSecurityGroupIds(const char* value) { m_securityGroupIdsHasBeenSet = true; m_securityGroupIds.push_back(value); return *this; }


    /**
     * <p>The ID of the VPC to associate with the Client VPN endpoint.</p>
     */
    inline const Aws::String& GetVpcId() const{ return m_vpcId; }

    /**
     * <p>The ID of the VPC to associate with the Client VPN endpoint.</p>
     */
    inline bool VpcIdHasBeenSet() const { return m_vpcIdHasBeenSet; }

    /**
     * <p>The ID of the VPC to associate with the Client VPN endpoint.</p>
     */
    inline void SetVpcId(const Aws::String& value) { m_vpcIdHasBeenSet = true; m_vpcId = value; }

    /**
     * <p>The ID of the VPC to associate with the Client VPN endpoint.</p>
     */
    inline void SetVpcId(Aws::String&& value) { m_vpcIdHasBeenSet = true; m_vpcId = std::move(value); }

    /**
     * <p>The ID of the VPC to associate with the Client VPN endpoint.</p>
     */
    inline void SetVpcId(const char* value) { m_vpcIdHasBeenSet = true; m_vpcId.assign(value); }

    /**
     * <p>The ID of the VPC to associate with the Client VPN endpoint.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithVpcId(const Aws::String& value) { SetVpcId(value); return *this;}

    /**
     * <p>The ID of the VPC to associate with the Client VPN endpoint.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithVpcId(Aws::String&& value) { SetVpcId(std::move(value)); return *this;}

    /**
     * <p>The ID of the VPC to associate with the Client VPN endpoint.</p>
     */
    inline ModifyClientVpnEndpointRequest& WithVpcId(const char* value) { SetVpcId(value); return *this;}

  private:

    Aws::String m_clientVpnEndpointId;
    bool m_clientVpnEndpointIdHasBeenSet;

    Aws::String m_serverCertificateArn;
    bool m_serverCertificateArnHasBeenSet;

    ConnectionLogOptions m_connectionLogOptions;
    bool m_connectionLogOptionsHasBeenSet;

    DnsServersOptionsModifyStructure m_dnsServers;
    bool m_dnsServersHasBeenSet;

    int m_vpnPort;
    bool m_vpnPortHasBeenSet;

    Aws::String m_description;
    bool m_descriptionHasBeenSet;

    bool m_splitTunnel;
    bool m_splitTunnelHasBeenSet;

    bool m_dryRun;
    bool m_dryRunHasBeenSet;

    Aws::Vector<Aws::String> m_securityGroupIds;
    bool m_securityGroupIdsHasBeenSet;

    Aws::String m_vpcId;
    bool m_vpcIdHasBeenSet;
  };

} // namespace Model
} // namespace EC2
} // namespace Aws
