﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/ec2/model/NetworkInfo.h>
#include <aws/core/utils/xml/XmlSerializer.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>

#include <utility>

using namespace Aws::Utils::Xml;
using namespace Aws::Utils;

namespace Aws
{
namespace EC2
{
namespace Model
{

NetworkInfo::NetworkInfo() : 
    m_networkPerformanceHasBeenSet(false),
    m_maximumNetworkInterfaces(0),
    m_maximumNetworkInterfacesHasBeenSet(false),
    m_ipv4AddressesPerInterface(0),
    m_ipv4AddressesPerInterfaceHasBeenSet(false),
    m_ipv6AddressesPerInterface(0),
    m_ipv6AddressesPerInterfaceHasBeenSet(false),
    m_ipv6Supported(false),
    m_ipv6SupportedHasBeenSet(false),
    m_enaSupport(EnaSupport::NOT_SET),
    m_enaSupportHasBeenSet(false),
    m_efaSupported(false),
    m_efaSupportedHasBeenSet(false)
{
}

NetworkInfo::NetworkInfo(const XmlNode& xmlNode) : 
    m_networkPerformanceHasBeenSet(false),
    m_maximumNetworkInterfaces(0),
    m_maximumNetworkInterfacesHasBeenSet(false),
    m_ipv4AddressesPerInterface(0),
    m_ipv4AddressesPerInterfaceHasBeenSet(false),
    m_ipv6AddressesPerInterface(0),
    m_ipv6AddressesPerInterfaceHasBeenSet(false),
    m_ipv6Supported(false),
    m_ipv6SupportedHasBeenSet(false),
    m_enaSupport(EnaSupport::NOT_SET),
    m_enaSupportHasBeenSet(false),
    m_efaSupported(false),
    m_efaSupportedHasBeenSet(false)
{
  *this = xmlNode;
}

NetworkInfo& NetworkInfo::operator =(const XmlNode& xmlNode)
{
  XmlNode resultNode = xmlNode;

  if(!resultNode.IsNull())
  {
    XmlNode networkPerformanceNode = resultNode.FirstChild("networkPerformance");
    if(!networkPerformanceNode.IsNull())
    {
      m_networkPerformance = Aws::Utils::Xml::DecodeEscapedXmlText(networkPerformanceNode.GetText());
      m_networkPerformanceHasBeenSet = true;
    }
    XmlNode maximumNetworkInterfacesNode = resultNode.FirstChild("maximumNetworkInterfaces");
    if(!maximumNetworkInterfacesNode.IsNull())
    {
      m_maximumNetworkInterfaces = StringUtils::ConvertToInt32(StringUtils::Trim(Aws::Utils::Xml::DecodeEscapedXmlText(maximumNetworkInterfacesNode.GetText()).c_str()).c_str());
      m_maximumNetworkInterfacesHasBeenSet = true;
    }
    XmlNode ipv4AddressesPerInterfaceNode = resultNode.FirstChild("ipv4AddressesPerInterface");
    if(!ipv4AddressesPerInterfaceNode.IsNull())
    {
      m_ipv4AddressesPerInterface = StringUtils::ConvertToInt32(StringUtils::Trim(Aws::Utils::Xml::DecodeEscapedXmlText(ipv4AddressesPerInterfaceNode.GetText()).c_str()).c_str());
      m_ipv4AddressesPerInterfaceHasBeenSet = true;
    }
    XmlNode ipv6AddressesPerInterfaceNode = resultNode.FirstChild("ipv6AddressesPerInterface");
    if(!ipv6AddressesPerInterfaceNode.IsNull())
    {
      m_ipv6AddressesPerInterface = StringUtils::ConvertToInt32(StringUtils::Trim(Aws::Utils::Xml::DecodeEscapedXmlText(ipv6AddressesPerInterfaceNode.GetText()).c_str()).c_str());
      m_ipv6AddressesPerInterfaceHasBeenSet = true;
    }
    XmlNode ipv6SupportedNode = resultNode.FirstChild("ipv6Supported");
    if(!ipv6SupportedNode.IsNull())
    {
      m_ipv6Supported = StringUtils::ConvertToBool(StringUtils::Trim(Aws::Utils::Xml::DecodeEscapedXmlText(ipv6SupportedNode.GetText()).c_str()).c_str());
      m_ipv6SupportedHasBeenSet = true;
    }
    XmlNode enaSupportNode = resultNode.FirstChild("enaSupport");
    if(!enaSupportNode.IsNull())
    {
      m_enaSupport = EnaSupportMapper::GetEnaSupportForName(StringUtils::Trim(Aws::Utils::Xml::DecodeEscapedXmlText(enaSupportNode.GetText()).c_str()).c_str());
      m_enaSupportHasBeenSet = true;
    }
    XmlNode efaSupportedNode = resultNode.FirstChild("efaSupported");
    if(!efaSupportedNode.IsNull())
    {
      m_efaSupported = StringUtils::ConvertToBool(StringUtils::Trim(Aws::Utils::Xml::DecodeEscapedXmlText(efaSupportedNode.GetText()).c_str()).c_str());
      m_efaSupportedHasBeenSet = true;
    }
  }

  return *this;
}

void NetworkInfo::OutputToStream(Aws::OStream& oStream, const char* location, unsigned index, const char* locationValue) const
{
  if(m_networkPerformanceHasBeenSet)
  {
      oStream << location << index << locationValue << ".NetworkPerformance=" << StringUtils::URLEncode(m_networkPerformance.c_str()) << "&";
  }

  if(m_maximumNetworkInterfacesHasBeenSet)
  {
      oStream << location << index << locationValue << ".MaximumNetworkInterfaces=" << m_maximumNetworkInterfaces << "&";
  }

  if(m_ipv4AddressesPerInterfaceHasBeenSet)
  {
      oStream << location << index << locationValue << ".Ipv4AddressesPerInterface=" << m_ipv4AddressesPerInterface << "&";
  }

  if(m_ipv6AddressesPerInterfaceHasBeenSet)
  {
      oStream << location << index << locationValue << ".Ipv6AddressesPerInterface=" << m_ipv6AddressesPerInterface << "&";
  }

  if(m_ipv6SupportedHasBeenSet)
  {
      oStream << location << index << locationValue << ".Ipv6Supported=" << std::boolalpha << m_ipv6Supported << "&";
  }

  if(m_enaSupportHasBeenSet)
  {
      oStream << location << index << locationValue << ".EnaSupport=" << EnaSupportMapper::GetNameForEnaSupport(m_enaSupport) << "&";
  }

  if(m_efaSupportedHasBeenSet)
  {
      oStream << location << index << locationValue << ".EfaSupported=" << std::boolalpha << m_efaSupported << "&";
  }

}

void NetworkInfo::OutputToStream(Aws::OStream& oStream, const char* location) const
{
  if(m_networkPerformanceHasBeenSet)
  {
      oStream << location << ".NetworkPerformance=" << StringUtils::URLEncode(m_networkPerformance.c_str()) << "&";
  }
  if(m_maximumNetworkInterfacesHasBeenSet)
  {
      oStream << location << ".MaximumNetworkInterfaces=" << m_maximumNetworkInterfaces << "&";
  }
  if(m_ipv4AddressesPerInterfaceHasBeenSet)
  {
      oStream << location << ".Ipv4AddressesPerInterface=" << m_ipv4AddressesPerInterface << "&";
  }
  if(m_ipv6AddressesPerInterfaceHasBeenSet)
  {
      oStream << location << ".Ipv6AddressesPerInterface=" << m_ipv6AddressesPerInterface << "&";
  }
  if(m_ipv6SupportedHasBeenSet)
  {
      oStream << location << ".Ipv6Supported=" << std::boolalpha << m_ipv6Supported << "&";
  }
  if(m_enaSupportHasBeenSet)
  {
      oStream << location << ".EnaSupport=" << EnaSupportMapper::GetNameForEnaSupport(m_enaSupport) << "&";
  }
  if(m_efaSupportedHasBeenSet)
  {
      oStream << location << ".EfaSupported=" << std::boolalpha << m_efaSupported << "&";
  }
}

} // namespace Model
} // namespace EC2
} // namespace Aws
