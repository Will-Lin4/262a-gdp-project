﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/signer/Signer_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/signer/model/SigningMaterial.h>
#include <aws/signer/model/SigningPlatformOverrides.h>
#include <aws/core/utils/memory/stl/AWSMap.h>
#include <aws/signer/model/SigningProfileStatus.h>
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
namespace signer
{
namespace Model
{
  class AWS_SIGNER_API GetSigningProfileResult
  {
  public:
    GetSigningProfileResult();
    GetSigningProfileResult(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    GetSigningProfileResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);


    /**
     * <p>The name of the target signing profile.</p>
     */
    inline const Aws::String& GetProfileName() const{ return m_profileName; }

    /**
     * <p>The name of the target signing profile.</p>
     */
    inline void SetProfileName(const Aws::String& value) { m_profileName = value; }

    /**
     * <p>The name of the target signing profile.</p>
     */
    inline void SetProfileName(Aws::String&& value) { m_profileName = std::move(value); }

    /**
     * <p>The name of the target signing profile.</p>
     */
    inline void SetProfileName(const char* value) { m_profileName.assign(value); }

    /**
     * <p>The name of the target signing profile.</p>
     */
    inline GetSigningProfileResult& WithProfileName(const Aws::String& value) { SetProfileName(value); return *this;}

    /**
     * <p>The name of the target signing profile.</p>
     */
    inline GetSigningProfileResult& WithProfileName(Aws::String&& value) { SetProfileName(std::move(value)); return *this;}

    /**
     * <p>The name of the target signing profile.</p>
     */
    inline GetSigningProfileResult& WithProfileName(const char* value) { SetProfileName(value); return *this;}


    /**
     * <p>The ARN of the certificate that the target profile uses for signing
     * operations.</p>
     */
    inline const SigningMaterial& GetSigningMaterial() const{ return m_signingMaterial; }

    /**
     * <p>The ARN of the certificate that the target profile uses for signing
     * operations.</p>
     */
    inline void SetSigningMaterial(const SigningMaterial& value) { m_signingMaterial = value; }

    /**
     * <p>The ARN of the certificate that the target profile uses for signing
     * operations.</p>
     */
    inline void SetSigningMaterial(SigningMaterial&& value) { m_signingMaterial = std::move(value); }

    /**
     * <p>The ARN of the certificate that the target profile uses for signing
     * operations.</p>
     */
    inline GetSigningProfileResult& WithSigningMaterial(const SigningMaterial& value) { SetSigningMaterial(value); return *this;}

    /**
     * <p>The ARN of the certificate that the target profile uses for signing
     * operations.</p>
     */
    inline GetSigningProfileResult& WithSigningMaterial(SigningMaterial&& value) { SetSigningMaterial(std::move(value)); return *this;}


    /**
     * <p>The ID of the platform that is used by the target signing profile.</p>
     */
    inline const Aws::String& GetPlatformId() const{ return m_platformId; }

    /**
     * <p>The ID of the platform that is used by the target signing profile.</p>
     */
    inline void SetPlatformId(const Aws::String& value) { m_platformId = value; }

    /**
     * <p>The ID of the platform that is used by the target signing profile.</p>
     */
    inline void SetPlatformId(Aws::String&& value) { m_platformId = std::move(value); }

    /**
     * <p>The ID of the platform that is used by the target signing profile.</p>
     */
    inline void SetPlatformId(const char* value) { m_platformId.assign(value); }

    /**
     * <p>The ID of the platform that is used by the target signing profile.</p>
     */
    inline GetSigningProfileResult& WithPlatformId(const Aws::String& value) { SetPlatformId(value); return *this;}

    /**
     * <p>The ID of the platform that is used by the target signing profile.</p>
     */
    inline GetSigningProfileResult& WithPlatformId(Aws::String&& value) { SetPlatformId(std::move(value)); return *this;}

    /**
     * <p>The ID of the platform that is used by the target signing profile.</p>
     */
    inline GetSigningProfileResult& WithPlatformId(const char* value) { SetPlatformId(value); return *this;}


    /**
     * <p>A list of overrides applied by the target signing profile for signing
     * operations.</p>
     */
    inline const SigningPlatformOverrides& GetOverrides() const{ return m_overrides; }

    /**
     * <p>A list of overrides applied by the target signing profile for signing
     * operations.</p>
     */
    inline void SetOverrides(const SigningPlatformOverrides& value) { m_overrides = value; }

    /**
     * <p>A list of overrides applied by the target signing profile for signing
     * operations.</p>
     */
    inline void SetOverrides(SigningPlatformOverrides&& value) { m_overrides = std::move(value); }

    /**
     * <p>A list of overrides applied by the target signing profile for signing
     * operations.</p>
     */
    inline GetSigningProfileResult& WithOverrides(const SigningPlatformOverrides& value) { SetOverrides(value); return *this;}

    /**
     * <p>A list of overrides applied by the target signing profile for signing
     * operations.</p>
     */
    inline GetSigningProfileResult& WithOverrides(SigningPlatformOverrides&& value) { SetOverrides(std::move(value)); return *this;}


    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline const Aws::Map<Aws::String, Aws::String>& GetSigningParameters() const{ return m_signingParameters; }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline void SetSigningParameters(const Aws::Map<Aws::String, Aws::String>& value) { m_signingParameters = value; }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline void SetSigningParameters(Aws::Map<Aws::String, Aws::String>&& value) { m_signingParameters = std::move(value); }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& WithSigningParameters(const Aws::Map<Aws::String, Aws::String>& value) { SetSigningParameters(value); return *this;}

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& WithSigningParameters(Aws::Map<Aws::String, Aws::String>&& value) { SetSigningParameters(std::move(value)); return *this;}

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& AddSigningParameters(const Aws::String& key, const Aws::String& value) { m_signingParameters.emplace(key, value); return *this; }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& AddSigningParameters(Aws::String&& key, const Aws::String& value) { m_signingParameters.emplace(std::move(key), value); return *this; }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& AddSigningParameters(const Aws::String& key, Aws::String&& value) { m_signingParameters.emplace(key, std::move(value)); return *this; }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& AddSigningParameters(Aws::String&& key, Aws::String&& value) { m_signingParameters.emplace(std::move(key), std::move(value)); return *this; }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& AddSigningParameters(const char* key, Aws::String&& value) { m_signingParameters.emplace(key, std::move(value)); return *this; }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& AddSigningParameters(Aws::String&& key, const char* value) { m_signingParameters.emplace(std::move(key), value); return *this; }

    /**
     * <p>A map of key-value pairs for signing operations that is attached to the
     * target signing profile.</p>
     */
    inline GetSigningProfileResult& AddSigningParameters(const char* key, const char* value) { m_signingParameters.emplace(key, value); return *this; }


    /**
     * <p>The status of the target signing profile.</p>
     */
    inline const SigningProfileStatus& GetStatus() const{ return m_status; }

    /**
     * <p>The status of the target signing profile.</p>
     */
    inline void SetStatus(const SigningProfileStatus& value) { m_status = value; }

    /**
     * <p>The status of the target signing profile.</p>
     */
    inline void SetStatus(SigningProfileStatus&& value) { m_status = std::move(value); }

    /**
     * <p>The status of the target signing profile.</p>
     */
    inline GetSigningProfileResult& WithStatus(const SigningProfileStatus& value) { SetStatus(value); return *this;}

    /**
     * <p>The status of the target signing profile.</p>
     */
    inline GetSigningProfileResult& WithStatus(SigningProfileStatus&& value) { SetStatus(std::move(value)); return *this;}


    /**
     * <p>The Amazon Resource Name (ARN) for the signing profile.</p>
     */
    inline const Aws::String& GetArn() const{ return m_arn; }

    /**
     * <p>The Amazon Resource Name (ARN) for the signing profile.</p>
     */
    inline void SetArn(const Aws::String& value) { m_arn = value; }

    /**
     * <p>The Amazon Resource Name (ARN) for the signing profile.</p>
     */
    inline void SetArn(Aws::String&& value) { m_arn = std::move(value); }

    /**
     * <p>The Amazon Resource Name (ARN) for the signing profile.</p>
     */
    inline void SetArn(const char* value) { m_arn.assign(value); }

    /**
     * <p>The Amazon Resource Name (ARN) for the signing profile.</p>
     */
    inline GetSigningProfileResult& WithArn(const Aws::String& value) { SetArn(value); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) for the signing profile.</p>
     */
    inline GetSigningProfileResult& WithArn(Aws::String&& value) { SetArn(std::move(value)); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) for the signing profile.</p>
     */
    inline GetSigningProfileResult& WithArn(const char* value) { SetArn(value); return *this;}


    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline const Aws::Map<Aws::String, Aws::String>& GetTags() const{ return m_tags; }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline void SetTags(const Aws::Map<Aws::String, Aws::String>& value) { m_tags = value; }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline void SetTags(Aws::Map<Aws::String, Aws::String>&& value) { m_tags = std::move(value); }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& WithTags(const Aws::Map<Aws::String, Aws::String>& value) { SetTags(value); return *this;}

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& WithTags(Aws::Map<Aws::String, Aws::String>&& value) { SetTags(std::move(value)); return *this;}

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& AddTags(const Aws::String& key, const Aws::String& value) { m_tags.emplace(key, value); return *this; }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& AddTags(Aws::String&& key, const Aws::String& value) { m_tags.emplace(std::move(key), value); return *this; }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& AddTags(const Aws::String& key, Aws::String&& value) { m_tags.emplace(key, std::move(value)); return *this; }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& AddTags(Aws::String&& key, Aws::String&& value) { m_tags.emplace(std::move(key), std::move(value)); return *this; }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& AddTags(const char* key, Aws::String&& value) { m_tags.emplace(key, std::move(value)); return *this; }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& AddTags(Aws::String&& key, const char* value) { m_tags.emplace(std::move(key), value); return *this; }

    /**
     * <p>A list of tags associated with the signing profile.</p>
     */
    inline GetSigningProfileResult& AddTags(const char* key, const char* value) { m_tags.emplace(key, value); return *this; }

  private:

    Aws::String m_profileName;

    SigningMaterial m_signingMaterial;

    Aws::String m_platformId;

    SigningPlatformOverrides m_overrides;

    Aws::Map<Aws::String, Aws::String> m_signingParameters;

    SigningProfileStatus m_status;

    Aws::String m_arn;

    Aws::Map<Aws::String, Aws::String> m_tags;
  };

} // namespace Model
} // namespace signer
} // namespace Aws
