﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/glacier/model/ListPartsRequest.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/http/URI.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>

#include <utility>

using namespace Aws::Glacier::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;
using namespace Aws::Http;

ListPartsRequest::ListPartsRequest() : 
    m_accountIdHasBeenSet(false),
    m_vaultNameHasBeenSet(false),
    m_uploadIdHasBeenSet(false),
    m_markerHasBeenSet(false),
    m_limitHasBeenSet(false)
{
}

Aws::String ListPartsRequest::SerializePayload() const
{
  return {};
}

void ListPartsRequest::AddQueryStringParameters(URI& uri) const
{
    Aws::StringStream ss;
    if(m_markerHasBeenSet)
    {
      ss << m_marker;
      uri.AddQueryStringParameter("marker", ss.str());
      ss.str("");
    }

    if(m_limitHasBeenSet)
    {
      ss << m_limit;
      uri.AddQueryStringParameter("limit", ss.str());
      ss.str("");
    }

}



