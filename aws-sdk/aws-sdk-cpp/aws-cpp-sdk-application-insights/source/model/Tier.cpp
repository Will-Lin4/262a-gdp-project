﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/application-insights/model/Tier.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Globals.h>
#include <aws/core/utils/EnumParseOverflowContainer.h>

using namespace Aws::Utils;


namespace Aws
{
  namespace ApplicationInsights
  {
    namespace Model
    {
      namespace TierMapper
      {

        static const int DEFAULT_HASH = HashingUtils::HashString("DEFAULT");
        static const int DOT_NET_CORE_HASH = HashingUtils::HashString("DOT_NET_CORE");
        static const int DOT_NET_WORKER_HASH = HashingUtils::HashString("DOT_NET_WORKER");
        static const int DOT_NET_WEB_HASH = HashingUtils::HashString("DOT_NET_WEB");
        static const int SQL_SERVER_HASH = HashingUtils::HashString("SQL_SERVER");


        Tier GetTierForName(const Aws::String& name)
        {
          int hashCode = HashingUtils::HashString(name.c_str());
          if (hashCode == DEFAULT_HASH)
          {
            return Tier::DEFAULT;
          }
          else if (hashCode == DOT_NET_CORE_HASH)
          {
            return Tier::DOT_NET_CORE;
          }
          else if (hashCode == DOT_NET_WORKER_HASH)
          {
            return Tier::DOT_NET_WORKER;
          }
          else if (hashCode == DOT_NET_WEB_HASH)
          {
            return Tier::DOT_NET_WEB;
          }
          else if (hashCode == SQL_SERVER_HASH)
          {
            return Tier::SQL_SERVER;
          }
          EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
          if(overflowContainer)
          {
            overflowContainer->StoreOverflow(hashCode, name);
            return static_cast<Tier>(hashCode);
          }

          return Tier::NOT_SET;
        }

        Aws::String GetNameForTier(Tier enumValue)
        {
          switch(enumValue)
          {
          case Tier::DEFAULT:
            return "DEFAULT";
          case Tier::DOT_NET_CORE:
            return "DOT_NET_CORE";
          case Tier::DOT_NET_WORKER:
            return "DOT_NET_WORKER";
          case Tier::DOT_NET_WEB:
            return "DOT_NET_WEB";
          case Tier::SQL_SERVER:
            return "SQL_SERVER";
          default:
            EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
            if(overflowContainer)
            {
              return overflowContainer->RetrieveOverflow(static_cast<int>(enumValue));
            }

            return {};
          }
        }

      } // namespace TierMapper
    } // namespace Model
  } // namespace ApplicationInsights
} // namespace Aws
