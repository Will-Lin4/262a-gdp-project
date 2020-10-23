﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/timestream-write/model/RejectedRecord.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

namespace Aws
{
namespace TimestreamWrite
{
namespace Model
{

RejectedRecord::RejectedRecord() : 
    m_recordIndex(0),
    m_recordIndexHasBeenSet(false),
    m_reasonHasBeenSet(false)
{
}

RejectedRecord::RejectedRecord(JsonView jsonValue) : 
    m_recordIndex(0),
    m_recordIndexHasBeenSet(false),
    m_reasonHasBeenSet(false)
{
  *this = jsonValue;
}

RejectedRecord& RejectedRecord::operator =(JsonView jsonValue)
{
  if(jsonValue.ValueExists("RecordIndex"))
  {
    m_recordIndex = jsonValue.GetInteger("RecordIndex");

    m_recordIndexHasBeenSet = true;
  }

  if(jsonValue.ValueExists("Reason"))
  {
    m_reason = jsonValue.GetString("Reason");

    m_reasonHasBeenSet = true;
  }

  return *this;
}

JsonValue RejectedRecord::Jsonize() const
{
  JsonValue payload;

  if(m_recordIndexHasBeenSet)
  {
   payload.WithInteger("RecordIndex", m_recordIndex);

  }

  if(m_reasonHasBeenSet)
  {
   payload.WithString("Reason", m_reason);

  }

  return payload;
}

} // namespace Model
} // namespace TimestreamWrite
} // namespace Aws
