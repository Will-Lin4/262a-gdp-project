﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/iotanalytics/model/DatastoreSummary.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

namespace Aws
{
namespace IoTAnalytics
{
namespace Model
{

DatastoreSummary::DatastoreSummary() : 
    m_datastoreNameHasBeenSet(false),
    m_datastoreStorageHasBeenSet(false),
    m_status(DatastoreStatus::NOT_SET),
    m_statusHasBeenSet(false),
    m_creationTimeHasBeenSet(false),
    m_lastUpdateTimeHasBeenSet(false)
{
}

DatastoreSummary::DatastoreSummary(JsonView jsonValue) : 
    m_datastoreNameHasBeenSet(false),
    m_datastoreStorageHasBeenSet(false),
    m_status(DatastoreStatus::NOT_SET),
    m_statusHasBeenSet(false),
    m_creationTimeHasBeenSet(false),
    m_lastUpdateTimeHasBeenSet(false)
{
  *this = jsonValue;
}

DatastoreSummary& DatastoreSummary::operator =(JsonView jsonValue)
{
  if(jsonValue.ValueExists("datastoreName"))
  {
    m_datastoreName = jsonValue.GetString("datastoreName");

    m_datastoreNameHasBeenSet = true;
  }

  if(jsonValue.ValueExists("datastoreStorage"))
  {
    m_datastoreStorage = jsonValue.GetObject("datastoreStorage");

    m_datastoreStorageHasBeenSet = true;
  }

  if(jsonValue.ValueExists("status"))
  {
    m_status = DatastoreStatusMapper::GetDatastoreStatusForName(jsonValue.GetString("status"));

    m_statusHasBeenSet = true;
  }

  if(jsonValue.ValueExists("creationTime"))
  {
    m_creationTime = jsonValue.GetDouble("creationTime");

    m_creationTimeHasBeenSet = true;
  }

  if(jsonValue.ValueExists("lastUpdateTime"))
  {
    m_lastUpdateTime = jsonValue.GetDouble("lastUpdateTime");

    m_lastUpdateTimeHasBeenSet = true;
  }

  return *this;
}

JsonValue DatastoreSummary::Jsonize() const
{
  JsonValue payload;

  if(m_datastoreNameHasBeenSet)
  {
   payload.WithString("datastoreName", m_datastoreName);

  }

  if(m_datastoreStorageHasBeenSet)
  {
   payload.WithObject("datastoreStorage", m_datastoreStorage.Jsonize());

  }

  if(m_statusHasBeenSet)
  {
   payload.WithString("status", DatastoreStatusMapper::GetNameForDatastoreStatus(m_status));
  }

  if(m_creationTimeHasBeenSet)
  {
   payload.WithDouble("creationTime", m_creationTime.SecondsWithMSPrecision());
  }

  if(m_lastUpdateTimeHasBeenSet)
  {
   payload.WithDouble("lastUpdateTime", m_lastUpdateTime.SecondsWithMSPrecision());
  }

  return payload;
}

} // namespace Model
} // namespace IoTAnalytics
} // namespace Aws
