﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/timestream-write/TimestreamWrite_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/timestream-write/model/MeasureValueType.h>
#include <aws/timestream-write/model/TimeUnit.h>
#include <aws/timestream-write/model/Dimension.h>
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
namespace TimestreamWrite
{
namespace Model
{

  /**
   * <p>Record represents a time series data point being written into Timestream.
   * Each record contains an array of dimensions. Dimensions represent the meta data
   * attributes of a time series data point such as the instance name or availability
   * zone of an EC2 instance. A record also contains the measure name which is the
   * name of the measure being collected for example the CPU utilization of an EC2
   * instance. A record also contains the measure value and the value type which is
   * the data type of the measure value. In addition, the record contains the
   * timestamp when the measure was collected that the timestamp unit which
   * represents the granularity of the timestamp. </p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/timestream-write-2018-11-01/Record">AWS
   * API Reference</a></p>
   */
  class AWS_TIMESTREAMWRITE_API Record
  {
  public:
    Record();
    Record(Aws::Utils::Json::JsonView jsonValue);
    Record& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>Contains the list of dimensions for time series data points.</p>
     */
    inline const Aws::Vector<Dimension>& GetDimensions() const{ return m_dimensions; }

    /**
     * <p>Contains the list of dimensions for time series data points.</p>
     */
    inline bool DimensionsHasBeenSet() const { return m_dimensionsHasBeenSet; }

    /**
     * <p>Contains the list of dimensions for time series data points.</p>
     */
    inline void SetDimensions(const Aws::Vector<Dimension>& value) { m_dimensionsHasBeenSet = true; m_dimensions = value; }

    /**
     * <p>Contains the list of dimensions for time series data points.</p>
     */
    inline void SetDimensions(Aws::Vector<Dimension>&& value) { m_dimensionsHasBeenSet = true; m_dimensions = std::move(value); }

    /**
     * <p>Contains the list of dimensions for time series data points.</p>
     */
    inline Record& WithDimensions(const Aws::Vector<Dimension>& value) { SetDimensions(value); return *this;}

    /**
     * <p>Contains the list of dimensions for time series data points.</p>
     */
    inline Record& WithDimensions(Aws::Vector<Dimension>&& value) { SetDimensions(std::move(value)); return *this;}

    /**
     * <p>Contains the list of dimensions for time series data points.</p>
     */
    inline Record& AddDimensions(const Dimension& value) { m_dimensionsHasBeenSet = true; m_dimensions.push_back(value); return *this; }

    /**
     * <p>Contains the list of dimensions for time series data points.</p>
     */
    inline Record& AddDimensions(Dimension&& value) { m_dimensionsHasBeenSet = true; m_dimensions.push_back(std::move(value)); return *this; }


    /**
     * <p>Measure represents the data attribute of the time series. For example, the
     * CPU utilization of an EC2 instance or the RPM of a wind turbine are measures.
     * </p>
     */
    inline const Aws::String& GetMeasureName() const{ return m_measureName; }

    /**
     * <p>Measure represents the data attribute of the time series. For example, the
     * CPU utilization of an EC2 instance or the RPM of a wind turbine are measures.
     * </p>
     */
    inline bool MeasureNameHasBeenSet() const { return m_measureNameHasBeenSet; }

    /**
     * <p>Measure represents the data attribute of the time series. For example, the
     * CPU utilization of an EC2 instance or the RPM of a wind turbine are measures.
     * </p>
     */
    inline void SetMeasureName(const Aws::String& value) { m_measureNameHasBeenSet = true; m_measureName = value; }

    /**
     * <p>Measure represents the data attribute of the time series. For example, the
     * CPU utilization of an EC2 instance or the RPM of a wind turbine are measures.
     * </p>
     */
    inline void SetMeasureName(Aws::String&& value) { m_measureNameHasBeenSet = true; m_measureName = std::move(value); }

    /**
     * <p>Measure represents the data attribute of the time series. For example, the
     * CPU utilization of an EC2 instance or the RPM of a wind turbine are measures.
     * </p>
     */
    inline void SetMeasureName(const char* value) { m_measureNameHasBeenSet = true; m_measureName.assign(value); }

    /**
     * <p>Measure represents the data attribute of the time series. For example, the
     * CPU utilization of an EC2 instance or the RPM of a wind turbine are measures.
     * </p>
     */
    inline Record& WithMeasureName(const Aws::String& value) { SetMeasureName(value); return *this;}

    /**
     * <p>Measure represents the data attribute of the time series. For example, the
     * CPU utilization of an EC2 instance or the RPM of a wind turbine are measures.
     * </p>
     */
    inline Record& WithMeasureName(Aws::String&& value) { SetMeasureName(std::move(value)); return *this;}

    /**
     * <p>Measure represents the data attribute of the time series. For example, the
     * CPU utilization of an EC2 instance or the RPM of a wind turbine are measures.
     * </p>
     */
    inline Record& WithMeasureName(const char* value) { SetMeasureName(value); return *this;}


    /**
     * <p> Contains the measure value for the time series data point. </p>
     */
    inline const Aws::String& GetMeasureValue() const{ return m_measureValue; }

    /**
     * <p> Contains the measure value for the time series data point. </p>
     */
    inline bool MeasureValueHasBeenSet() const { return m_measureValueHasBeenSet; }

    /**
     * <p> Contains the measure value for the time series data point. </p>
     */
    inline void SetMeasureValue(const Aws::String& value) { m_measureValueHasBeenSet = true; m_measureValue = value; }

    /**
     * <p> Contains the measure value for the time series data point. </p>
     */
    inline void SetMeasureValue(Aws::String&& value) { m_measureValueHasBeenSet = true; m_measureValue = std::move(value); }

    /**
     * <p> Contains the measure value for the time series data point. </p>
     */
    inline void SetMeasureValue(const char* value) { m_measureValueHasBeenSet = true; m_measureValue.assign(value); }

    /**
     * <p> Contains the measure value for the time series data point. </p>
     */
    inline Record& WithMeasureValue(const Aws::String& value) { SetMeasureValue(value); return *this;}

    /**
     * <p> Contains the measure value for the time series data point. </p>
     */
    inline Record& WithMeasureValue(Aws::String&& value) { SetMeasureValue(std::move(value)); return *this;}

    /**
     * <p> Contains the measure value for the time series data point. </p>
     */
    inline Record& WithMeasureValue(const char* value) { SetMeasureValue(value); return *this;}


    /**
     * <p> Contains the data type of the measure value for the time series data point.
     * </p>
     */
    inline const MeasureValueType& GetMeasureValueType() const{ return m_measureValueType; }

    /**
     * <p> Contains the data type of the measure value for the time series data point.
     * </p>
     */
    inline bool MeasureValueTypeHasBeenSet() const { return m_measureValueTypeHasBeenSet; }

    /**
     * <p> Contains the data type of the measure value for the time series data point.
     * </p>
     */
    inline void SetMeasureValueType(const MeasureValueType& value) { m_measureValueTypeHasBeenSet = true; m_measureValueType = value; }

    /**
     * <p> Contains the data type of the measure value for the time series data point.
     * </p>
     */
    inline void SetMeasureValueType(MeasureValueType&& value) { m_measureValueTypeHasBeenSet = true; m_measureValueType = std::move(value); }

    /**
     * <p> Contains the data type of the measure value for the time series data point.
     * </p>
     */
    inline Record& WithMeasureValueType(const MeasureValueType& value) { SetMeasureValueType(value); return *this;}

    /**
     * <p> Contains the data type of the measure value for the time series data point.
     * </p>
     */
    inline Record& WithMeasureValueType(MeasureValueType&& value) { SetMeasureValueType(std::move(value)); return *this;}


    /**
     * <p> Contains the time at which the measure value for the data point was
     * collected. </p>
     */
    inline const Aws::String& GetTime() const{ return m_time; }

    /**
     * <p> Contains the time at which the measure value for the data point was
     * collected. </p>
     */
    inline bool TimeHasBeenSet() const { return m_timeHasBeenSet; }

    /**
     * <p> Contains the time at which the measure value for the data point was
     * collected. </p>
     */
    inline void SetTime(const Aws::String& value) { m_timeHasBeenSet = true; m_time = value; }

    /**
     * <p> Contains the time at which the measure value for the data point was
     * collected. </p>
     */
    inline void SetTime(Aws::String&& value) { m_timeHasBeenSet = true; m_time = std::move(value); }

    /**
     * <p> Contains the time at which the measure value for the data point was
     * collected. </p>
     */
    inline void SetTime(const char* value) { m_timeHasBeenSet = true; m_time.assign(value); }

    /**
     * <p> Contains the time at which the measure value for the data point was
     * collected. </p>
     */
    inline Record& WithTime(const Aws::String& value) { SetTime(value); return *this;}

    /**
     * <p> Contains the time at which the measure value for the data point was
     * collected. </p>
     */
    inline Record& WithTime(Aws::String&& value) { SetTime(std::move(value)); return *this;}

    /**
     * <p> Contains the time at which the measure value for the data point was
     * collected. </p>
     */
    inline Record& WithTime(const char* value) { SetTime(value); return *this;}


    /**
     * <p> The granularity of the timestamp unit. It indicates if the time value is in
     * seconds, milliseconds, nanoseconds or other supported values. </p>
     */
    inline const TimeUnit& GetTimeUnit() const{ return m_timeUnit; }

    /**
     * <p> The granularity of the timestamp unit. It indicates if the time value is in
     * seconds, milliseconds, nanoseconds or other supported values. </p>
     */
    inline bool TimeUnitHasBeenSet() const { return m_timeUnitHasBeenSet; }

    /**
     * <p> The granularity of the timestamp unit. It indicates if the time value is in
     * seconds, milliseconds, nanoseconds or other supported values. </p>
     */
    inline void SetTimeUnit(const TimeUnit& value) { m_timeUnitHasBeenSet = true; m_timeUnit = value; }

    /**
     * <p> The granularity of the timestamp unit. It indicates if the time value is in
     * seconds, milliseconds, nanoseconds or other supported values. </p>
     */
    inline void SetTimeUnit(TimeUnit&& value) { m_timeUnitHasBeenSet = true; m_timeUnit = std::move(value); }

    /**
     * <p> The granularity of the timestamp unit. It indicates if the time value is in
     * seconds, milliseconds, nanoseconds or other supported values. </p>
     */
    inline Record& WithTimeUnit(const TimeUnit& value) { SetTimeUnit(value); return *this;}

    /**
     * <p> The granularity of the timestamp unit. It indicates if the time value is in
     * seconds, milliseconds, nanoseconds or other supported values. </p>
     */
    inline Record& WithTimeUnit(TimeUnit&& value) { SetTimeUnit(std::move(value)); return *this;}

  private:

    Aws::Vector<Dimension> m_dimensions;
    bool m_dimensionsHasBeenSet;

    Aws::String m_measureName;
    bool m_measureNameHasBeenSet;

    Aws::String m_measureValue;
    bool m_measureValueHasBeenSet;

    MeasureValueType m_measureValueType;
    bool m_measureValueTypeHasBeenSet;

    Aws::String m_time;
    bool m_timeHasBeenSet;

    TimeUnit m_timeUnit;
    bool m_timeUnitHasBeenSet;
  };

} // namespace Model
} // namespace TimestreamWrite
} // namespace Aws
