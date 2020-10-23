﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/fms/model/SecurityServiceType.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Globals.h>
#include <aws/core/utils/EnumParseOverflowContainer.h>

using namespace Aws::Utils;


namespace Aws
{
  namespace FMS
  {
    namespace Model
    {
      namespace SecurityServiceTypeMapper
      {

        static const int WAF_HASH = HashingUtils::HashString("WAF");
        static const int WAFV2_HASH = HashingUtils::HashString("WAFV2");
        static const int SHIELD_ADVANCED_HASH = HashingUtils::HashString("SHIELD_ADVANCED");
        static const int SECURITY_GROUPS_COMMON_HASH = HashingUtils::HashString("SECURITY_GROUPS_COMMON");
        static const int SECURITY_GROUPS_CONTENT_AUDIT_HASH = HashingUtils::HashString("SECURITY_GROUPS_CONTENT_AUDIT");
        static const int SECURITY_GROUPS_USAGE_AUDIT_HASH = HashingUtils::HashString("SECURITY_GROUPS_USAGE_AUDIT");


        SecurityServiceType GetSecurityServiceTypeForName(const Aws::String& name)
        {
          int hashCode = HashingUtils::HashString(name.c_str());
          if (hashCode == WAF_HASH)
          {
            return SecurityServiceType::WAF;
          }
          else if (hashCode == WAFV2_HASH)
          {
            return SecurityServiceType::WAFV2;
          }
          else if (hashCode == SHIELD_ADVANCED_HASH)
          {
            return SecurityServiceType::SHIELD_ADVANCED;
          }
          else if (hashCode == SECURITY_GROUPS_COMMON_HASH)
          {
            return SecurityServiceType::SECURITY_GROUPS_COMMON;
          }
          else if (hashCode == SECURITY_GROUPS_CONTENT_AUDIT_HASH)
          {
            return SecurityServiceType::SECURITY_GROUPS_CONTENT_AUDIT;
          }
          else if (hashCode == SECURITY_GROUPS_USAGE_AUDIT_HASH)
          {
            return SecurityServiceType::SECURITY_GROUPS_USAGE_AUDIT;
          }
          EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
          if(overflowContainer)
          {
            overflowContainer->StoreOverflow(hashCode, name);
            return static_cast<SecurityServiceType>(hashCode);
          }

          return SecurityServiceType::NOT_SET;
        }

        Aws::String GetNameForSecurityServiceType(SecurityServiceType enumValue)
        {
          switch(enumValue)
          {
          case SecurityServiceType::WAF:
            return "WAF";
          case SecurityServiceType::WAFV2:
            return "WAFV2";
          case SecurityServiceType::SHIELD_ADVANCED:
            return "SHIELD_ADVANCED";
          case SecurityServiceType::SECURITY_GROUPS_COMMON:
            return "SECURITY_GROUPS_COMMON";
          case SecurityServiceType::SECURITY_GROUPS_CONTENT_AUDIT:
            return "SECURITY_GROUPS_CONTENT_AUDIT";
          case SecurityServiceType::SECURITY_GROUPS_USAGE_AUDIT:
            return "SECURITY_GROUPS_USAGE_AUDIT";
          default:
            EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
            if(overflowContainer)
            {
              return overflowContainer->RetrieveOverflow(static_cast<int>(enumValue));
            }

            return {};
          }
        }

      } // namespace SecurityServiceTypeMapper
    } // namespace Model
  } // namespace FMS
} // namespace Aws
