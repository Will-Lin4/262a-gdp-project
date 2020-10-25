﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/service-quotas/ServiceQuotas_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/service-quotas/model/ServiceInfo.h>
#include <utility>

namespace Aws
{
template<typename RESULT_TYPE>
class AmazonWebServiceResult;

namespace Utils
{
namespace Json
{
  class JsonValue;
} // namespace Json
} // namespace Utils
namespace ServiceQuotas
{
namespace Model
{
  class AWS_SERVICEQUOTAS_API ListServicesResult
  {
  public:
    ListServicesResult();
    ListServicesResult(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    ListServicesResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);


    /**
     * <p>If present in the response, this value indicates there's more output
     * available that what's included in the current response. This can occur even when
     * the response includes no values at all, such as when you ask for a filtered view
     * of a very long list. Use this value in the <code>NextToken</code> request
     * parameter in a subsequent call to the operation to continue processing and get
     * the next part of the output. You should repeat this until the
     * <code>NextToken</code> response element comes back empty (as
     * <code>null</code>).</p>
     */
    inline const Aws::String& GetNextToken() const{ return m_nextToken; }

    /**
     * <p>If present in the response, this value indicates there's more output
     * available that what's included in the current response. This can occur even when
     * the response includes no values at all, such as when you ask for a filtered view
     * of a very long list. Use this value in the <code>NextToken</code> request
     * parameter in a subsequent call to the operation to continue processing and get
     * the next part of the output. You should repeat this until the
     * <code>NextToken</code> response element comes back empty (as
     * <code>null</code>).</p>
     */
    inline void SetNextToken(const Aws::String& value) { m_nextToken = value; }

    /**
     * <p>If present in the response, this value indicates there's more output
     * available that what's included in the current response. This can occur even when
     * the response includes no values at all, such as when you ask for a filtered view
     * of a very long list. Use this value in the <code>NextToken</code> request
     * parameter in a subsequent call to the operation to continue processing and get
     * the next part of the output. You should repeat this until the
     * <code>NextToken</code> response element comes back empty (as
     * <code>null</code>).</p>
     */
    inline void SetNextToken(Aws::String&& value) { m_nextToken = std::move(value); }

    /**
     * <p>If present in the response, this value indicates there's more output
     * available that what's included in the current response. This can occur even when
     * the response includes no values at all, such as when you ask for a filtered view
     * of a very long list. Use this value in the <code>NextToken</code> request
     * parameter in a subsequent call to the operation to continue processing and get
     * the next part of the output. You should repeat this until the
     * <code>NextToken</code> response element comes back empty (as
     * <code>null</code>).</p>
     */
    inline void SetNextToken(const char* value) { m_nextToken.assign(value); }

    /**
     * <p>If present in the response, this value indicates there's more output
     * available that what's included in the current response. This can occur even when
     * the response includes no values at all, such as when you ask for a filtered view
     * of a very long list. Use this value in the <code>NextToken</code> request
     * parameter in a subsequent call to the operation to continue processing and get
     * the next part of the output. You should repeat this until the
     * <code>NextToken</code> response element comes back empty (as
     * <code>null</code>).</p>
     */
    inline ListServicesResult& WithNextToken(const Aws::String& value) { SetNextToken(value); return *this;}

    /**
     * <p>If present in the response, this value indicates there's more output
     * available that what's included in the current response. This can occur even when
     * the response includes no values at all, such as when you ask for a filtered view
     * of a very long list. Use this value in the <code>NextToken</code> request
     * parameter in a subsequent call to the operation to continue processing and get
     * the next part of the output. You should repeat this until the
     * <code>NextToken</code> response element comes back empty (as
     * <code>null</code>).</p>
     */
    inline ListServicesResult& WithNextToken(Aws::String&& value) { SetNextToken(std::move(value)); return *this;}

    /**
     * <p>If present in the response, this value indicates there's more output
     * available that what's included in the current response. This can occur even when
     * the response includes no values at all, such as when you ask for a filtered view
     * of a very long list. Use this value in the <code>NextToken</code> request
     * parameter in a subsequent call to the operation to continue processing and get
     * the next part of the output. You should repeat this until the
     * <code>NextToken</code> response element comes back empty (as
     * <code>null</code>).</p>
     */
    inline ListServicesResult& WithNextToken(const char* value) { SetNextToken(value); return *this;}


    /**
     * <p>Returns a list of services. </p>
     */
    inline const Aws::Vector<ServiceInfo>& GetServices() const{ return m_services; }

    /**
     * <p>Returns a list of services. </p>
     */
    inline void SetServices(const Aws::Vector<ServiceInfo>& value) { m_services = value; }

    /**
     * <p>Returns a list of services. </p>
     */
    inline void SetServices(Aws::Vector<ServiceInfo>&& value) { m_services = std::move(value); }

    /**
     * <p>Returns a list of services. </p>
     */
    inline ListServicesResult& WithServices(const Aws::Vector<ServiceInfo>& value) { SetServices(value); return *this;}

    /**
     * <p>Returns a list of services. </p>
     */
    inline ListServicesResult& WithServices(Aws::Vector<ServiceInfo>&& value) { SetServices(std::move(value)); return *this;}

    /**
     * <p>Returns a list of services. </p>
     */
    inline ListServicesResult& AddServices(const ServiceInfo& value) { m_services.push_back(value); return *this; }

    /**
     * <p>Returns a list of services. </p>
     */
    inline ListServicesResult& AddServices(ServiceInfo&& value) { m_services.push_back(std::move(value)); return *this; }

  private:

    Aws::String m_nextToken;

    Aws::Vector<ServiceInfo> m_services;
  };

} // namespace Model
} // namespace ServiceQuotas
} // namespace Aws
