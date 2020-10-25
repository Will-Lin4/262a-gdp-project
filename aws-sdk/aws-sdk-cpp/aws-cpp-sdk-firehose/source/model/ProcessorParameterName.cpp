﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/firehose/model/ProcessorParameterName.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/Globals.h>
#include <aws/core/utils/EnumParseOverflowContainer.h>

using namespace Aws::Utils;


namespace Aws
{
  namespace Firehose
  {
    namespace Model
    {
      namespace ProcessorParameterNameMapper
      {

        static const int LambdaArn_HASH = HashingUtils::HashString("LambdaArn");
        static const int NumberOfRetries_HASH = HashingUtils::HashString("NumberOfRetries");
        static const int RoleArn_HASH = HashingUtils::HashString("RoleArn");
        static const int BufferSizeInMBs_HASH = HashingUtils::HashString("BufferSizeInMBs");
        static const int BufferIntervalInSeconds_HASH = HashingUtils::HashString("BufferIntervalInSeconds");


        ProcessorParameterName GetProcessorParameterNameForName(const Aws::String& name)
        {
          int hashCode = HashingUtils::HashString(name.c_str());
          if (hashCode == LambdaArn_HASH)
          {
            return ProcessorParameterName::LambdaArn;
          }
          else if (hashCode == NumberOfRetries_HASH)
          {
            return ProcessorParameterName::NumberOfRetries;
          }
          else if (hashCode == RoleArn_HASH)
          {
            return ProcessorParameterName::RoleArn;
          }
          else if (hashCode == BufferSizeInMBs_HASH)
          {
            return ProcessorParameterName::BufferSizeInMBs;
          }
          else if (hashCode == BufferIntervalInSeconds_HASH)
          {
            return ProcessorParameterName::BufferIntervalInSeconds;
          }
          EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
          if(overflowContainer)
          {
            overflowContainer->StoreOverflow(hashCode, name);
            return static_cast<ProcessorParameterName>(hashCode);
          }

          return ProcessorParameterName::NOT_SET;
        }

        Aws::String GetNameForProcessorParameterName(ProcessorParameterName enumValue)
        {
          switch(enumValue)
          {
          case ProcessorParameterName::LambdaArn:
            return "LambdaArn";
          case ProcessorParameterName::NumberOfRetries:
            return "NumberOfRetries";
          case ProcessorParameterName::RoleArn:
            return "RoleArn";
          case ProcessorParameterName::BufferSizeInMBs:
            return "BufferSizeInMBs";
          case ProcessorParameterName::BufferIntervalInSeconds:
            return "BufferIntervalInSeconds";
          default:
            EnumParseOverflowContainer* overflowContainer = Aws::GetEnumOverflowContainer();
            if(overflowContainer)
            {
              return overflowContainer->RetrieveOverflow(static_cast<int>(enumValue));
            }

            return {};
          }
        }

      } // namespace ProcessorParameterNameMapper
    } // namespace Model
  } // namespace Firehose
} // namespace Aws
