﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/kinesis/model/MergeShardsRequest.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>

using namespace Aws::Kinesis::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;

MergeShardsRequest::MergeShardsRequest() : 
    m_streamNameHasBeenSet(false),
    m_shardToMergeHasBeenSet(false),
    m_adjacentShardToMergeHasBeenSet(false)
{
}

Aws::String MergeShardsRequest::SerializePayload() const
{
  JsonValue payload;

  if(m_streamNameHasBeenSet)
  {
   payload.WithString("StreamName", m_streamName);

  }

  if(m_shardToMergeHasBeenSet)
  {
   payload.WithString("ShardToMerge", m_shardToMerge);

  }

  if(m_adjacentShardToMergeHasBeenSet)
  {
   payload.WithString("AdjacentShardToMerge", m_adjacentShardToMerge);

  }

  return payload.View().WriteReadable();
}

Aws::Http::HeaderValueCollection MergeShardsRequest::GetRequestSpecificHeaders() const
{
  Aws::Http::HeaderValueCollection headers;
  headers.insert(Aws::Http::HeaderValuePair("X-Amz-Target", "Kinesis_20131202.MergeShards"));
  return headers;

}




