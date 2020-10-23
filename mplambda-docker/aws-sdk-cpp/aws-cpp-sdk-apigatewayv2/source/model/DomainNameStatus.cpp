﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/apigatewayv2/model/DomainNameStatus.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Globals.h>
#include <aws/core/utils/EnumParseOverflowContainer.h>

using namespace Aws::Utils;


namespace Aws
{
  namespace ApiGatewayV2
  {
    namespace Model
    {
      namespace DomainNameStatusMapper
      {

        static const int AVAILABLE_HASH = HashingUtils::HashString("AVAILABLE");
        static const int UPDATING_HASH = HashingUtils::HashString("UPDATING");


        DomainNameStatus GetDomainNameStatusForName(const Aws::String& name)
        {
          int hashCode = HashingUtils::HashString(name.c_str());
          if (hashCode == AVAILABLE_HASH)
          {
            return DomainNameStatus::AVAILABLE;
          }
          else if (hashCode == UPDATING_HASH)
          {
            return DomainNameStatus::UPDATING;
          }
          EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
          if(overflowContainer)
          {
            overflowContainer->StoreOverflow(hashCode, name);
            return static_cast<DomainNameStatus>(hashCode);
          }

          return DomainNameStatus::NOT_SET;
        }

        Aws::String GetNameForDomainNameStatus(DomainNameStatus enumValue)
        {
          switch(enumValue)
          {
          case DomainNameStatus::AVAILABLE:
            return "AVAILABLE";
          case DomainNameStatus::UPDATING:
            return "UPDATING";
          default:
            EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
            if(overflowContainer)
            {
              return overflowContainer->RetrieveOverflow(static_cast<int>(enumValue));
            }

            return {};
          }
        }

      } // namespace DomainNameStatusMapper
    } // namespace Model
  } // namespace ApiGatewayV2
} // namespace Aws
