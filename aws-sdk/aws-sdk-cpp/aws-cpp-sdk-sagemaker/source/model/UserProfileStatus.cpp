﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/sagemaker/model/UserProfileStatus.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Globals.h>
#include <aws/core/utils/EnumParseOverflowContainer.h>

using namespace Aws::Utils;


namespace Aws
{
  namespace SageMaker
  {
    namespace Model
    {
      namespace UserProfileStatusMapper
      {

        static const int Deleting_HASH = HashingUtils::HashString("Deleting");
        static const int Failed_HASH = HashingUtils::HashString("Failed");
        static const int InService_HASH = HashingUtils::HashString("InService");
        static const int Pending_HASH = HashingUtils::HashString("Pending");


        UserProfileStatus GetUserProfileStatusForName(const Aws::String& name)
        {
          int hashCode = HashingUtils::HashString(name.c_str());
          if (hashCode == Deleting_HASH)
          {
            return UserProfileStatus::Deleting;
          }
          else if (hashCode == Failed_HASH)
          {
            return UserProfileStatus::Failed;
          }
          else if (hashCode == InService_HASH)
          {
            return UserProfileStatus::InService;
          }
          else if (hashCode == Pending_HASH)
          {
            return UserProfileStatus::Pending;
          }
          EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
          if(overflowContainer)
          {
            overflowContainer->StoreOverflow(hashCode, name);
            return static_cast<UserProfileStatus>(hashCode);
          }

          return UserProfileStatus::NOT_SET;
        }

        Aws::String GetNameForUserProfileStatus(UserProfileStatus enumValue)
        {
          switch(enumValue)
          {
          case UserProfileStatus::Deleting:
            return "Deleting";
          case UserProfileStatus::Failed:
            return "Failed";
          case UserProfileStatus::InService:
            return "InService";
          case UserProfileStatus::Pending:
            return "Pending";
          default:
            EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
            if(overflowContainer)
            {
              return overflowContainer->RetrieveOverflow(static_cast<int>(enumValue));
            }

            return {};
          }
        }

      } // namespace UserProfileStatusMapper
    } // namespace Model
  } // namespace SageMaker
} // namespace Aws
