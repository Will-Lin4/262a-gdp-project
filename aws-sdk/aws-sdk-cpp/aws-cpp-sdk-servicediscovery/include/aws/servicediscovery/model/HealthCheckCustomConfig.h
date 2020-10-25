﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/servicediscovery/ServiceDiscovery_EXPORTS.h>

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
namespace ServiceDiscovery
{
namespace Model
{

  /**
   * <p>A complex type that contains information about an optional custom health
   * check. A custom health check, which requires that you use a third-party health
   * checker to evaluate the health of your resources, is useful in the following
   * circumstances:</p> <ul> <li> <p>You can't use a health check that is defined by
   * <code>HealthCheckConfig</code> because the resource isn't available over the
   * internet. For example, you can use a custom health check when the instance is in
   * an Amazon VPC. (To check the health of resources in a VPC, the health checker
   * must also be in the VPC.)</p> </li> <li> <p>You want to use a third-party health
   * checker regardless of where your resources are.</p> </li> </ul> 
   * <p>If you specify a health check configuration, you can specify either
   * <code>HealthCheckCustomConfig</code> or <code>HealthCheckConfig</code> but not
   * both.</p>  <p>To change the status of a custom health check, submit
   * an <code>UpdateInstanceCustomHealthStatus</code> request. AWS Cloud Map doesn't
   * monitor the status of the resource, it just keeps a record of the status
   * specified in the most recent <code>UpdateInstanceCustomHealthStatus</code>
   * request.</p> <p>Here's how custom health checks work:</p> <ol> <li> <p>You
   * create a service and specify a value for <code>FailureThreshold</code>. </p>
   * <p>The failure threshold indicates the number of 30-second intervals you want
   * AWS Cloud Map to wait between the time that your application sends an <a
   * href="https://docs.aws.amazon.com/cloud-map/latest/api/API_UpdateInstanceCustomHealthStatus.html">UpdateInstanceCustomHealthStatus</a>
   * request and the time that AWS Cloud Map stops routing internet traffic to the
   * corresponding resource.</p> </li> <li> <p>You register an instance.</p> </li>
   * <li> <p>You configure a third-party health checker to monitor the resource that
   * is associated with the new instance. </p>  <p>AWS Cloud Map doesn't check
   * the health of the resource directly. </p>  </li> <li> <p>The third-party
   * health-checker determines that the resource is unhealthy and notifies your
   * application.</p> </li> <li> <p>Your application submits an
   * <code>UpdateInstanceCustomHealthStatus</code> request.</p> </li> <li> <p>AWS
   * Cloud Map waits for (<code>FailureThreshold</code> x 30) seconds.</p> </li> <li>
   * <p>If another <code>UpdateInstanceCustomHealthStatus</code> request doesn't
   * arrive during that time to change the status back to healthy, AWS Cloud Map
   * stops routing traffic to the resource.</p> </li> </ol><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/servicediscovery-2017-03-14/HealthCheckCustomConfig">AWS
   * API Reference</a></p>
   */
  class AWS_SERVICEDISCOVERY_API HealthCheckCustomConfig
  {
  public:
    HealthCheckCustomConfig();
    HealthCheckCustomConfig(Aws::Utils::Json::JsonView jsonValue);
    HealthCheckCustomConfig& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     *  <p>This parameter has been deprecated and is always set to 1. AWS
     * Cloud Map waits for approximately 30 seconds after receiving an
     * <code>UpdateInstanceCustomHealthStatus</code> request before changing the status
     * of the service instance.</p>  <p>The number of 30-second intervals
     * that you want AWS Cloud Map to wait after receiving an
     * <code>UpdateInstanceCustomHealthStatus</code> request before it changes the
     * health status of a service instance.</p> <p>Sending a second or subsequent
     * <code>UpdateInstanceCustomHealthStatus</code> request with the same value before
     * 30 seconds has passed doesn't accelerate the change. AWS Cloud Map still waits
     * <code>30</code> seconds after the first request to make the change.</p>
     */
    inline int GetFailureThreshold() const{ return m_failureThreshold; }

    /**
     *  <p>This parameter has been deprecated and is always set to 1. AWS
     * Cloud Map waits for approximately 30 seconds after receiving an
     * <code>UpdateInstanceCustomHealthStatus</code> request before changing the status
     * of the service instance.</p>  <p>The number of 30-second intervals
     * that you want AWS Cloud Map to wait after receiving an
     * <code>UpdateInstanceCustomHealthStatus</code> request before it changes the
     * health status of a service instance.</p> <p>Sending a second or subsequent
     * <code>UpdateInstanceCustomHealthStatus</code> request with the same value before
     * 30 seconds has passed doesn't accelerate the change. AWS Cloud Map still waits
     * <code>30</code> seconds after the first request to make the change.</p>
     */
    inline bool FailureThresholdHasBeenSet() const { return m_failureThresholdHasBeenSet; }

    /**
     *  <p>This parameter has been deprecated and is always set to 1. AWS
     * Cloud Map waits for approximately 30 seconds after receiving an
     * <code>UpdateInstanceCustomHealthStatus</code> request before changing the status
     * of the service instance.</p>  <p>The number of 30-second intervals
     * that you want AWS Cloud Map to wait after receiving an
     * <code>UpdateInstanceCustomHealthStatus</code> request before it changes the
     * health status of a service instance.</p> <p>Sending a second or subsequent
     * <code>UpdateInstanceCustomHealthStatus</code> request with the same value before
     * 30 seconds has passed doesn't accelerate the change. AWS Cloud Map still waits
     * <code>30</code> seconds after the first request to make the change.</p>
     */
    inline void SetFailureThreshold(int value) { m_failureThresholdHasBeenSet = true; m_failureThreshold = value; }

    /**
     *  <p>This parameter has been deprecated and is always set to 1. AWS
     * Cloud Map waits for approximately 30 seconds after receiving an
     * <code>UpdateInstanceCustomHealthStatus</code> request before changing the status
     * of the service instance.</p>  <p>The number of 30-second intervals
     * that you want AWS Cloud Map to wait after receiving an
     * <code>UpdateInstanceCustomHealthStatus</code> request before it changes the
     * health status of a service instance.</p> <p>Sending a second or subsequent
     * <code>UpdateInstanceCustomHealthStatus</code> request with the same value before
     * 30 seconds has passed doesn't accelerate the change. AWS Cloud Map still waits
     * <code>30</code> seconds after the first request to make the change.</p>
     */
    inline HealthCheckCustomConfig& WithFailureThreshold(int value) { SetFailureThreshold(value); return *this;}

  private:

    int m_failureThreshold;
    bool m_failureThresholdHasBeenSet;
  };

} // namespace Model
} // namespace ServiceDiscovery
} // namespace Aws
