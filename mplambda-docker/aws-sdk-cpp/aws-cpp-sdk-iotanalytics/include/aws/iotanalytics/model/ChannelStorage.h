﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/iotanalytics/IoTAnalytics_EXPORTS.h>
#include <aws/iotanalytics/model/ServiceManagedChannelS3Storage.h>
#include <aws/iotanalytics/model/CustomerManagedChannelS3Storage.h>
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
namespace IoTAnalytics
{
namespace Model
{

  /**
   * <p>Where channel data is stored. You may choose one of "serviceManagedS3" or
   * "customerManagedS3" storage. If not specified, the default is
   * "serviceManagedS3". This cannot be changed after creation of the
   * channel.</p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/iotanalytics-2017-11-27/ChannelStorage">AWS
   * API Reference</a></p>
   */
  class AWS_IOTANALYTICS_API ChannelStorage
  {
  public:
    ChannelStorage();
    ChannelStorage(Aws::Utils::Json::JsonView jsonValue);
    ChannelStorage& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>Use this to store channel data in an S3 bucket managed by the AWS IoT
     * Analytics service. The choice of service-managed or customer-managed S3 storage
     * cannot be changed after creation of the channel.</p>
     */
    inline const ServiceManagedChannelS3Storage& GetServiceManagedS3() const{ return m_serviceManagedS3; }

    /**
     * <p>Use this to store channel data in an S3 bucket managed by the AWS IoT
     * Analytics service. The choice of service-managed or customer-managed S3 storage
     * cannot be changed after creation of the channel.</p>
     */
    inline bool ServiceManagedS3HasBeenSet() const { return m_serviceManagedS3HasBeenSet; }

    /**
     * <p>Use this to store channel data in an S3 bucket managed by the AWS IoT
     * Analytics service. The choice of service-managed or customer-managed S3 storage
     * cannot be changed after creation of the channel.</p>
     */
    inline void SetServiceManagedS3(const ServiceManagedChannelS3Storage& value) { m_serviceManagedS3HasBeenSet = true; m_serviceManagedS3 = value; }

    /**
     * <p>Use this to store channel data in an S3 bucket managed by the AWS IoT
     * Analytics service. The choice of service-managed or customer-managed S3 storage
     * cannot be changed after creation of the channel.</p>
     */
    inline void SetServiceManagedS3(ServiceManagedChannelS3Storage&& value) { m_serviceManagedS3HasBeenSet = true; m_serviceManagedS3 = std::move(value); }

    /**
     * <p>Use this to store channel data in an S3 bucket managed by the AWS IoT
     * Analytics service. The choice of service-managed or customer-managed S3 storage
     * cannot be changed after creation of the channel.</p>
     */
    inline ChannelStorage& WithServiceManagedS3(const ServiceManagedChannelS3Storage& value) { SetServiceManagedS3(value); return *this;}

    /**
     * <p>Use this to store channel data in an S3 bucket managed by the AWS IoT
     * Analytics service. The choice of service-managed or customer-managed S3 storage
     * cannot be changed after creation of the channel.</p>
     */
    inline ChannelStorage& WithServiceManagedS3(ServiceManagedChannelS3Storage&& value) { SetServiceManagedS3(std::move(value)); return *this;}


    /**
     * <p>Use this to store channel data in an S3 bucket that you manage. If customer
     * managed storage is selected, the "retentionPeriod" parameter is ignored. The
     * choice of service-managed or customer-managed S3 storage cannot be changed after
     * creation of the channel.</p>
     */
    inline const CustomerManagedChannelS3Storage& GetCustomerManagedS3() const{ return m_customerManagedS3; }

    /**
     * <p>Use this to store channel data in an S3 bucket that you manage. If customer
     * managed storage is selected, the "retentionPeriod" parameter is ignored. The
     * choice of service-managed or customer-managed S3 storage cannot be changed after
     * creation of the channel.</p>
     */
    inline bool CustomerManagedS3HasBeenSet() const { return m_customerManagedS3HasBeenSet; }

    /**
     * <p>Use this to store channel data in an S3 bucket that you manage. If customer
     * managed storage is selected, the "retentionPeriod" parameter is ignored. The
     * choice of service-managed or customer-managed S3 storage cannot be changed after
     * creation of the channel.</p>
     */
    inline void SetCustomerManagedS3(const CustomerManagedChannelS3Storage& value) { m_customerManagedS3HasBeenSet = true; m_customerManagedS3 = value; }

    /**
     * <p>Use this to store channel data in an S3 bucket that you manage. If customer
     * managed storage is selected, the "retentionPeriod" parameter is ignored. The
     * choice of service-managed or customer-managed S3 storage cannot be changed after
     * creation of the channel.</p>
     */
    inline void SetCustomerManagedS3(CustomerManagedChannelS3Storage&& value) { m_customerManagedS3HasBeenSet = true; m_customerManagedS3 = std::move(value); }

    /**
     * <p>Use this to store channel data in an S3 bucket that you manage. If customer
     * managed storage is selected, the "retentionPeriod" parameter is ignored. The
     * choice of service-managed or customer-managed S3 storage cannot be changed after
     * creation of the channel.</p>
     */
    inline ChannelStorage& WithCustomerManagedS3(const CustomerManagedChannelS3Storage& value) { SetCustomerManagedS3(value); return *this;}

    /**
     * <p>Use this to store channel data in an S3 bucket that you manage. If customer
     * managed storage is selected, the "retentionPeriod" parameter is ignored. The
     * choice of service-managed or customer-managed S3 storage cannot be changed after
     * creation of the channel.</p>
     */
    inline ChannelStorage& WithCustomerManagedS3(CustomerManagedChannelS3Storage&& value) { SetCustomerManagedS3(std::move(value)); return *this;}

  private:

    ServiceManagedChannelS3Storage m_serviceManagedS3;
    bool m_serviceManagedS3HasBeenSet;

    CustomerManagedChannelS3Storage m_customerManagedS3;
    bool m_customerManagedS3HasBeenSet;
  };

} // namespace Model
} // namespace IoTAnalytics
} // namespace Aws
