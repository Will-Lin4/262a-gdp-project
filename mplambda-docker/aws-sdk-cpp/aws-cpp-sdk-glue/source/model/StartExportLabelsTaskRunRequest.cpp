﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/glue/model/StartExportLabelsTaskRunRequest.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>

using namespace Aws::Glue::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;

StartExportLabelsTaskRunRequest::StartExportLabelsTaskRunRequest() : 
    m_transformIdHasBeenSet(false),
    m_outputS3PathHasBeenSet(false)
{
}

Aws::String StartExportLabelsTaskRunRequest::SerializePayload() const
{
  JsonValue payload;

  if(m_transformIdHasBeenSet)
  {
   payload.WithString("TransformId", m_transformId);

  }

  if(m_outputS3PathHasBeenSet)
  {
   payload.WithString("OutputS3Path", m_outputS3Path);

  }

  return payload.View().WriteReadable();
}

Aws::Http::HeaderValueCollection StartExportLabelsTaskRunRequest::GetRequestSpecificHeaders() const
{
  Aws::Http::HeaderValueCollection headers;
  headers.insert(Aws::Http::HeaderValuePair("X-Amz-Target", "AWSGlue.StartExportLabelsTaskRun"));
  return headers;

}




