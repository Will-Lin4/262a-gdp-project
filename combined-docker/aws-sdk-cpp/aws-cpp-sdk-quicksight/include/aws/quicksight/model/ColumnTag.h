﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/quicksight/QuickSight_EXPORTS.h>
#include <aws/quicksight/model/GeoSpatialDataRole.h>
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
namespace QuickSight
{
namespace Model
{

  /**
   * <p>A tag for a column in a <a>TagColumnOperation</a> structure. This is a
   * variant type structure. For this structure to be valid, only one of the
   * attributes can be non-null.</p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/quicksight-2018-04-01/ColumnTag">AWS
   * API Reference</a></p>
   */
  class AWS_QUICKSIGHT_API ColumnTag
  {
  public:
    ColumnTag();
    ColumnTag(Aws::Utils::Json::JsonView jsonValue);
    ColumnTag& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>A geospatial role for a column.</p>
     */
    inline const GeoSpatialDataRole& GetColumnGeographicRole() const{ return m_columnGeographicRole; }

    /**
     * <p>A geospatial role for a column.</p>
     */
    inline bool ColumnGeographicRoleHasBeenSet() const { return m_columnGeographicRoleHasBeenSet; }

    /**
     * <p>A geospatial role for a column.</p>
     */
    inline void SetColumnGeographicRole(const GeoSpatialDataRole& value) { m_columnGeographicRoleHasBeenSet = true; m_columnGeographicRole = value; }

    /**
     * <p>A geospatial role for a column.</p>
     */
    inline void SetColumnGeographicRole(GeoSpatialDataRole&& value) { m_columnGeographicRoleHasBeenSet = true; m_columnGeographicRole = std::move(value); }

    /**
     * <p>A geospatial role for a column.</p>
     */
    inline ColumnTag& WithColumnGeographicRole(const GeoSpatialDataRole& value) { SetColumnGeographicRole(value); return *this;}

    /**
     * <p>A geospatial role for a column.</p>
     */
    inline ColumnTag& WithColumnGeographicRole(GeoSpatialDataRole&& value) { SetColumnGeographicRole(std::move(value)); return *this;}

  private:

    GeoSpatialDataRole m_columnGeographicRole;
    bool m_columnGeographicRoleHasBeenSet;
  };

} // namespace Model
} // namespace QuickSight
} // namespace Aws
