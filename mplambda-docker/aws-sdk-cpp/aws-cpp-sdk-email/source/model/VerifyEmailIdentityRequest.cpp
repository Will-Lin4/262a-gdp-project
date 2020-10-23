﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/email/model/VerifyEmailIdentityRequest.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>

using namespace Aws::SES::Model;
using namespace Aws::Utils;

VerifyEmailIdentityRequest::VerifyEmailIdentityRequest() : 
    m_emailAddressHasBeenSet(false)
{
}

Aws::String VerifyEmailIdentityRequest::SerializePayload() const
{
  Aws::StringStream ss;
  ss << "Action=VerifyEmailIdentity&";
  if(m_emailAddressHasBeenSet)
  {
    ss << "EmailAddress=" << StringUtils::URLEncode(m_emailAddress.c_str()) << "&";
  }

  ss << "Version=2010-12-01";
  return ss.str();
}


void  VerifyEmailIdentityRequest::DumpBodyToUrl(Aws::Http::URI& uri ) const
{
  uri.SetQueryString(SerializePayload());
}
