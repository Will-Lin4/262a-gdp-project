﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/ssm/SSM_EXPORTS.h>

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
namespace SSM
{
namespace Model
{
  class AWS_SSM_API DescribePatchGroupStateResult
  {
  public:
    DescribePatchGroupStateResult();
    DescribePatchGroupStateResult(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    DescribePatchGroupStateResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);


    /**
     * <p>The number of instances in the patch group.</p>
     */
    inline int GetInstances() const{ return m_instances; }

    /**
     * <p>The number of instances in the patch group.</p>
     */
    inline void SetInstances(int value) { m_instances = value; }

    /**
     * <p>The number of instances in the patch group.</p>
     */
    inline DescribePatchGroupStateResult& WithInstances(int value) { SetInstances(value); return *this;}


    /**
     * <p>The number of instances with installed patches.</p>
     */
    inline int GetInstancesWithInstalledPatches() const{ return m_instancesWithInstalledPatches; }

    /**
     * <p>The number of instances with installed patches.</p>
     */
    inline void SetInstancesWithInstalledPatches(int value) { m_instancesWithInstalledPatches = value; }

    /**
     * <p>The number of instances with installed patches.</p>
     */
    inline DescribePatchGroupStateResult& WithInstancesWithInstalledPatches(int value) { SetInstancesWithInstalledPatches(value); return *this;}


    /**
     * <p>The number of instances with patches installed that aren't defined in the
     * patch baseline.</p>
     */
    inline int GetInstancesWithInstalledOtherPatches() const{ return m_instancesWithInstalledOtherPatches; }

    /**
     * <p>The number of instances with patches installed that aren't defined in the
     * patch baseline.</p>
     */
    inline void SetInstancesWithInstalledOtherPatches(int value) { m_instancesWithInstalledOtherPatches = value; }

    /**
     * <p>The number of instances with patches installed that aren't defined in the
     * patch baseline.</p>
     */
    inline DescribePatchGroupStateResult& WithInstancesWithInstalledOtherPatches(int value) { SetInstancesWithInstalledOtherPatches(value); return *this;}


    /**
     * <p>The number of instances with patches installed by Patch Manager that have not
     * been rebooted after the patch installation. The status of these instances is
     * NON_COMPLIANT.</p>
     */
    inline int GetInstancesWithInstalledPendingRebootPatches() const{ return m_instancesWithInstalledPendingRebootPatches; }

    /**
     * <p>The number of instances with patches installed by Patch Manager that have not
     * been rebooted after the patch installation. The status of these instances is
     * NON_COMPLIANT.</p>
     */
    inline void SetInstancesWithInstalledPendingRebootPatches(int value) { m_instancesWithInstalledPendingRebootPatches = value; }

    /**
     * <p>The number of instances with patches installed by Patch Manager that have not
     * been rebooted after the patch installation. The status of these instances is
     * NON_COMPLIANT.</p>
     */
    inline DescribePatchGroupStateResult& WithInstancesWithInstalledPendingRebootPatches(int value) { SetInstancesWithInstalledPendingRebootPatches(value); return *this;}


    /**
     * <p>The number of instances with patches installed that are specified in a
     * RejectedPatches list. Patches with a status of <i>INSTALLED_REJECTED</i> were
     * typically installed before they were added to a RejectedPatches list.</p> 
     * <p>If ALLOW_AS_DEPENDENCY is the specified option for RejectedPatchesAction, the
     * value of InstancesWithInstalledRejectedPatches will always be 0 (zero).</p>
     * 
     */
    inline int GetInstancesWithInstalledRejectedPatches() const{ return m_instancesWithInstalledRejectedPatches; }

    /**
     * <p>The number of instances with patches installed that are specified in a
     * RejectedPatches list. Patches with a status of <i>INSTALLED_REJECTED</i> were
     * typically installed before they were added to a RejectedPatches list.</p> 
     * <p>If ALLOW_AS_DEPENDENCY is the specified option for RejectedPatchesAction, the
     * value of InstancesWithInstalledRejectedPatches will always be 0 (zero).</p>
     * 
     */
    inline void SetInstancesWithInstalledRejectedPatches(int value) { m_instancesWithInstalledRejectedPatches = value; }

    /**
     * <p>The number of instances with patches installed that are specified in a
     * RejectedPatches list. Patches with a status of <i>INSTALLED_REJECTED</i> were
     * typically installed before they were added to a RejectedPatches list.</p> 
     * <p>If ALLOW_AS_DEPENDENCY is the specified option for RejectedPatchesAction, the
     * value of InstancesWithInstalledRejectedPatches will always be 0 (zero).</p>
     * 
     */
    inline DescribePatchGroupStateResult& WithInstancesWithInstalledRejectedPatches(int value) { SetInstancesWithInstalledRejectedPatches(value); return *this;}


    /**
     * <p>The number of instances with missing patches from the patch baseline.</p>
     */
    inline int GetInstancesWithMissingPatches() const{ return m_instancesWithMissingPatches; }

    /**
     * <p>The number of instances with missing patches from the patch baseline.</p>
     */
    inline void SetInstancesWithMissingPatches(int value) { m_instancesWithMissingPatches = value; }

    /**
     * <p>The number of instances with missing patches from the patch baseline.</p>
     */
    inline DescribePatchGroupStateResult& WithInstancesWithMissingPatches(int value) { SetInstancesWithMissingPatches(value); return *this;}


    /**
     * <p>The number of instances with patches from the patch baseline that failed to
     * install.</p>
     */
    inline int GetInstancesWithFailedPatches() const{ return m_instancesWithFailedPatches; }

    /**
     * <p>The number of instances with patches from the patch baseline that failed to
     * install.</p>
     */
    inline void SetInstancesWithFailedPatches(int value) { m_instancesWithFailedPatches = value; }

    /**
     * <p>The number of instances with patches from the patch baseline that failed to
     * install.</p>
     */
    inline DescribePatchGroupStateResult& WithInstancesWithFailedPatches(int value) { SetInstancesWithFailedPatches(value); return *this;}


    /**
     * <p>The number of instances with patches that aren't applicable.</p>
     */
    inline int GetInstancesWithNotApplicablePatches() const{ return m_instancesWithNotApplicablePatches; }

    /**
     * <p>The number of instances with patches that aren't applicable.</p>
     */
    inline void SetInstancesWithNotApplicablePatches(int value) { m_instancesWithNotApplicablePatches = value; }

    /**
     * <p>The number of instances with patches that aren't applicable.</p>
     */
    inline DescribePatchGroupStateResult& WithInstancesWithNotApplicablePatches(int value) { SetInstancesWithNotApplicablePatches(value); return *this;}


    /**
     * <p>The number of instances with <code>NotApplicable</code> patches beyond the
     * supported limit, which are not reported by name to Systems Manager
     * Inventory.</p>
     */
    inline int GetInstancesWithUnreportedNotApplicablePatches() const{ return m_instancesWithUnreportedNotApplicablePatches; }

    /**
     * <p>The number of instances with <code>NotApplicable</code> patches beyond the
     * supported limit, which are not reported by name to Systems Manager
     * Inventory.</p>
     */
    inline void SetInstancesWithUnreportedNotApplicablePatches(int value) { m_instancesWithUnreportedNotApplicablePatches = value; }

    /**
     * <p>The number of instances with <code>NotApplicable</code> patches beyond the
     * supported limit, which are not reported by name to Systems Manager
     * Inventory.</p>
     */
    inline DescribePatchGroupStateResult& WithInstancesWithUnreportedNotApplicablePatches(int value) { SetInstancesWithUnreportedNotApplicablePatches(value); return *this;}

  private:

    int m_instances;

    int m_instancesWithInstalledPatches;

    int m_instancesWithInstalledOtherPatches;

    int m_instancesWithInstalledPendingRebootPatches;

    int m_instancesWithInstalledRejectedPatches;

    int m_instancesWithMissingPatches;

    int m_instancesWithFailedPatches;

    int m_instancesWithNotApplicablePatches;

    int m_instancesWithUnreportedNotApplicablePatches;
  };

} // namespace Model
} // namespace SSM
} // namespace Aws
