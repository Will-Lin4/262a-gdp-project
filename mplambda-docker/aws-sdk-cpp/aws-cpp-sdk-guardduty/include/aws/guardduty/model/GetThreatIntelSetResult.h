﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/guardduty/GuardDuty_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/guardduty/model/ThreatIntelSetFormat.h>
#include <aws/guardduty/model/ThreatIntelSetStatus.h>
#include <aws/core/utils/memory/stl/AWSMap.h>
#include <utility>

namespace Aws
{
template<typename RESULT_TYPE>
class AmazonWebServiceResult;

namespace Utils
{
namespace Json
{
  class JsonValue;
} // namespace Json
} // namespace Utils
namespace GuardDuty
{
namespace Model
{
  class AWS_GUARDDUTY_API GetThreatIntelSetResult
  {
  public:
    GetThreatIntelSetResult();
    GetThreatIntelSetResult(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    GetThreatIntelSetResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);


    /**
     * <p>A user-friendly ThreatIntelSet name displayed in all findings that are
     * generated by activity that involves IP addresses included in this
     * ThreatIntelSet.</p>
     */
    inline const Aws::String& GetName() const{ return m_name; }

    /**
     * <p>A user-friendly ThreatIntelSet name displayed in all findings that are
     * generated by activity that involves IP addresses included in this
     * ThreatIntelSet.</p>
     */
    inline void SetName(const Aws::String& value) { m_name = value; }

    /**
     * <p>A user-friendly ThreatIntelSet name displayed in all findings that are
     * generated by activity that involves IP addresses included in this
     * ThreatIntelSet.</p>
     */
    inline void SetName(Aws::String&& value) { m_name = std::move(value); }

    /**
     * <p>A user-friendly ThreatIntelSet name displayed in all findings that are
     * generated by activity that involves IP addresses included in this
     * ThreatIntelSet.</p>
     */
    inline void SetName(const char* value) { m_name.assign(value); }

    /**
     * <p>A user-friendly ThreatIntelSet name displayed in all findings that are
     * generated by activity that involves IP addresses included in this
     * ThreatIntelSet.</p>
     */
    inline GetThreatIntelSetResult& WithName(const Aws::String& value) { SetName(value); return *this;}

    /**
     * <p>A user-friendly ThreatIntelSet name displayed in all findings that are
     * generated by activity that involves IP addresses included in this
     * ThreatIntelSet.</p>
     */
    inline GetThreatIntelSetResult& WithName(Aws::String&& value) { SetName(std::move(value)); return *this;}

    /**
     * <p>A user-friendly ThreatIntelSet name displayed in all findings that are
     * generated by activity that involves IP addresses included in this
     * ThreatIntelSet.</p>
     */
    inline GetThreatIntelSetResult& WithName(const char* value) { SetName(value); return *this;}


    /**
     * <p>The format of the threatIntelSet.</p>
     */
    inline const ThreatIntelSetFormat& GetFormat() const{ return m_format; }

    /**
     * <p>The format of the threatIntelSet.</p>
     */
    inline void SetFormat(const ThreatIntelSetFormat& value) { m_format = value; }

    /**
     * <p>The format of the threatIntelSet.</p>
     */
    inline void SetFormat(ThreatIntelSetFormat&& value) { m_format = std::move(value); }

    /**
     * <p>The format of the threatIntelSet.</p>
     */
    inline GetThreatIntelSetResult& WithFormat(const ThreatIntelSetFormat& value) { SetFormat(value); return *this;}

    /**
     * <p>The format of the threatIntelSet.</p>
     */
    inline GetThreatIntelSetResult& WithFormat(ThreatIntelSetFormat&& value) { SetFormat(std::move(value)); return *this;}


    /**
     * <p>The URI of the file that contains the ThreatIntelSet. For example:
     * https://s3.us-west-2.amazonaws.com/my-bucket/my-object-key.</p>
     */
    inline const Aws::String& GetLocation() const{ return m_location; }

    /**
     * <p>The URI of the file that contains the ThreatIntelSet. For example:
     * https://s3.us-west-2.amazonaws.com/my-bucket/my-object-key.</p>
     */
    inline void SetLocation(const Aws::String& value) { m_location = value; }

    /**
     * <p>The URI of the file that contains the ThreatIntelSet. For example:
     * https://s3.us-west-2.amazonaws.com/my-bucket/my-object-key.</p>
     */
    inline void SetLocation(Aws::String&& value) { m_location = std::move(value); }

    /**
     * <p>The URI of the file that contains the ThreatIntelSet. For example:
     * https://s3.us-west-2.amazonaws.com/my-bucket/my-object-key.</p>
     */
    inline void SetLocation(const char* value) { m_location.assign(value); }

    /**
     * <p>The URI of the file that contains the ThreatIntelSet. For example:
     * https://s3.us-west-2.amazonaws.com/my-bucket/my-object-key.</p>
     */
    inline GetThreatIntelSetResult& WithLocation(const Aws::String& value) { SetLocation(value); return *this;}

    /**
     * <p>The URI of the file that contains the ThreatIntelSet. For example:
     * https://s3.us-west-2.amazonaws.com/my-bucket/my-object-key.</p>
     */
    inline GetThreatIntelSetResult& WithLocation(Aws::String&& value) { SetLocation(std::move(value)); return *this;}

    /**
     * <p>The URI of the file that contains the ThreatIntelSet. For example:
     * https://s3.us-west-2.amazonaws.com/my-bucket/my-object-key.</p>
     */
    inline GetThreatIntelSetResult& WithLocation(const char* value) { SetLocation(value); return *this;}


    /**
     * <p>The status of threatIntelSet file uploaded.</p>
     */
    inline const ThreatIntelSetStatus& GetStatus() const{ return m_status; }

    /**
     * <p>The status of threatIntelSet file uploaded.</p>
     */
    inline void SetStatus(const ThreatIntelSetStatus& value) { m_status = value; }

    /**
     * <p>The status of threatIntelSet file uploaded.</p>
     */
    inline void SetStatus(ThreatIntelSetStatus&& value) { m_status = std::move(value); }

    /**
     * <p>The status of threatIntelSet file uploaded.</p>
     */
    inline GetThreatIntelSetResult& WithStatus(const ThreatIntelSetStatus& value) { SetStatus(value); return *this;}

    /**
     * <p>The status of threatIntelSet file uploaded.</p>
     */
    inline GetThreatIntelSetResult& WithStatus(ThreatIntelSetStatus&& value) { SetStatus(std::move(value)); return *this;}


    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline const Aws::Map<Aws::String, Aws::String>& GetTags() const{ return m_tags; }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline void SetTags(const Aws::Map<Aws::String, Aws::String>& value) { m_tags = value; }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline void SetTags(Aws::Map<Aws::String, Aws::String>&& value) { m_tags = std::move(value); }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& WithTags(const Aws::Map<Aws::String, Aws::String>& value) { SetTags(value); return *this;}

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& WithTags(Aws::Map<Aws::String, Aws::String>&& value) { SetTags(std::move(value)); return *this;}

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& AddTags(const Aws::String& key, const Aws::String& value) { m_tags.emplace(key, value); return *this; }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& AddTags(Aws::String&& key, const Aws::String& value) { m_tags.emplace(std::move(key), value); return *this; }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& AddTags(const Aws::String& key, Aws::String&& value) { m_tags.emplace(key, std::move(value)); return *this; }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& AddTags(Aws::String&& key, Aws::String&& value) { m_tags.emplace(std::move(key), std::move(value)); return *this; }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& AddTags(const char* key, Aws::String&& value) { m_tags.emplace(key, std::move(value)); return *this; }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& AddTags(Aws::String&& key, const char* value) { m_tags.emplace(std::move(key), value); return *this; }

    /**
     * <p>The tags of the threat list resource.</p>
     */
    inline GetThreatIntelSetResult& AddTags(const char* key, const char* value) { m_tags.emplace(key, value); return *this; }

  private:

    Aws::String m_name;

    ThreatIntelSetFormat m_format;

    Aws::String m_location;

    ThreatIntelSetStatus m_status;

    Aws::Map<Aws::String, Aws::String> m_tags;
  };

} // namespace Model
} // namespace GuardDuty
} // namespace Aws
