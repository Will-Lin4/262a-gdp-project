﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/ec2/EC2_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSStreamFwd.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/ec2/model/EphemeralNvmeSupport.h>
#include <aws/ec2/model/DiskInfo.h>
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
   * <p>Describes the disks that are available for the instance type.</p><p><h3>See
   * Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/ec2-2016-11-15/InstanceStorageInfo">AWS
   * API Reference</a></p>
   */
  class AWS_EC2_API InstanceStorageInfo
  {
  public:
    InstanceStorageInfo();
    InstanceStorageInfo(const Aws::Utils::Xml::XmlNode& xmlNode);
    InstanceStorageInfo& operator=(const Aws::Utils::Xml::XmlNode& xmlNode);

    void OutputToStream(Aws::OStream& ostream, const char* location, unsigned index, const char* locationValue) const;
    void OutputToStream(Aws::OStream& oStream, const char* location) const;


    /**
     * <p>The total size of the disks, in GB.</p>
     */
    inline long long GetTotalSizeInGB() const{ return m_totalSizeInGB; }

    /**
     * <p>The total size of the disks, in GB.</p>
     */
    inline bool TotalSizeInGBHasBeenSet() const { return m_totalSizeInGBHasBeenSet; }

    /**
     * <p>The total size of the disks, in GB.</p>
     */
    inline void SetTotalSizeInGB(long long value) { m_totalSizeInGBHasBeenSet = true; m_totalSizeInGB = value; }

    /**
     * <p>The total size of the disks, in GB.</p>
     */
    inline InstanceStorageInfo& WithTotalSizeInGB(long long value) { SetTotalSizeInGB(value); return *this;}


    /**
     * <p>Array describing the disks that are available for the instance type.</p>
     */
    inline const Aws::Vector<DiskInfo>& GetDisks() const{ return m_disks; }

    /**
     * <p>Array describing the disks that are available for the instance type.</p>
     */
    inline bool DisksHasBeenSet() const { return m_disksHasBeenSet; }

    /**
     * <p>Array describing the disks that are available for the instance type.</p>
     */
    inline void SetDisks(const Aws::Vector<DiskInfo>& value) { m_disksHasBeenSet = true; m_disks = value; }

    /**
     * <p>Array describing the disks that are available for the instance type.</p>
     */
    inline void SetDisks(Aws::Vector<DiskInfo>&& value) { m_disksHasBeenSet = true; m_disks = std::move(value); }

    /**
     * <p>Array describing the disks that are available for the instance type.</p>
     */
    inline InstanceStorageInfo& WithDisks(const Aws::Vector<DiskInfo>& value) { SetDisks(value); return *this;}

    /**
     * <p>Array describing the disks that are available for the instance type.</p>
     */
    inline InstanceStorageInfo& WithDisks(Aws::Vector<DiskInfo>&& value) { SetDisks(std::move(value)); return *this;}

    /**
     * <p>Array describing the disks that are available for the instance type.</p>
     */
    inline InstanceStorageInfo& AddDisks(const DiskInfo& value) { m_disksHasBeenSet = true; m_disks.push_back(value); return *this; }

    /**
     * <p>Array describing the disks that are available for the instance type.</p>
     */
    inline InstanceStorageInfo& AddDisks(DiskInfo&& value) { m_disksHasBeenSet = true; m_disks.push_back(std::move(value)); return *this; }


    /**
     * <p>Indicates whether non-volatile memory express (NVMe) is supported for
     * instance store.</p>
     */
    inline const EphemeralNvmeSupport& GetNvmeSupport() const{ return m_nvmeSupport; }

    /**
     * <p>Indicates whether non-volatile memory express (NVMe) is supported for
     * instance store.</p>
     */
    inline bool NvmeSupportHasBeenSet() const { return m_nvmeSupportHasBeenSet; }

    /**
     * <p>Indicates whether non-volatile memory express (NVMe) is supported for
     * instance store.</p>
     */
    inline void SetNvmeSupport(const EphemeralNvmeSupport& value) { m_nvmeSupportHasBeenSet = true; m_nvmeSupport = value; }

    /**
     * <p>Indicates whether non-volatile memory express (NVMe) is supported for
     * instance store.</p>
     */
    inline void SetNvmeSupport(EphemeralNvmeSupport&& value) { m_nvmeSupportHasBeenSet = true; m_nvmeSupport = std::move(value); }

    /**
     * <p>Indicates whether non-volatile memory express (NVMe) is supported for
     * instance store.</p>
     */
    inline InstanceStorageInfo& WithNvmeSupport(const EphemeralNvmeSupport& value) { SetNvmeSupport(value); return *this;}

    /**
     * <p>Indicates whether non-volatile memory express (NVMe) is supported for
     * instance store.</p>
     */
    inline InstanceStorageInfo& WithNvmeSupport(EphemeralNvmeSupport&& value) { SetNvmeSupport(std::move(value)); return *this;}

  private:

    long long m_totalSizeInGB;
    bool m_totalSizeInGBHasBeenSet;

    Aws::Vector<DiskInfo> m_disks;
    bool m_disksHasBeenSet;

    EphemeralNvmeSupport m_nvmeSupport;
    bool m_nvmeSupportHasBeenSet;
  };

} // namespace Model
} // namespace EC2
} // namespace Aws
