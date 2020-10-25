﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/sns/SNS_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSMap.h>
#include <aws/sns/model/ResponseMetadata.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
template<typename RESULT_TYPE>
class AmazonWebServiceResult;

namespace Utils
{
namespace Xml
{
  class XmlDocument;
} // namespace Xml
} // namespace Utils
namespace SNS
{
namespace Model
{
  /**
   * <p>Response for GetTopicAttributes action.</p><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/sns-2010-03-31/GetTopicAttributesResponse">AWS
   * API Reference</a></p>
   */
  class AWS_SNS_API GetTopicAttributesResult
  {
  public:
    GetTopicAttributesResult();
    GetTopicAttributesResult(const Aws::AmazonWebServiceResult<Aws::Utils::Xml::XmlDocument>& result);
    GetTopicAttributesResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Xml::XmlDocument>& result);


    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline const Aws::Map<Aws::String, Aws::String>& GetAttributes() const{ return m_attributes; }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline void SetAttributes(const Aws::Map<Aws::String, Aws::String>& value) { m_attributes = value; }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline void SetAttributes(Aws::Map<Aws::String, Aws::String>&& value) { m_attributes = std::move(value); }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& WithAttributes(const Aws::Map<Aws::String, Aws::String>& value) { SetAttributes(value); return *this;}

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& WithAttributes(Aws::Map<Aws::String, Aws::String>&& value) { SetAttributes(std::move(value)); return *this;}

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& AddAttributes(const Aws::String& key, const Aws::String& value) { m_attributes.emplace(key, value); return *this; }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& AddAttributes(Aws::String&& key, const Aws::String& value) { m_attributes.emplace(std::move(key), value); return *this; }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& AddAttributes(const Aws::String& key, Aws::String&& value) { m_attributes.emplace(key, std::move(value)); return *this; }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& AddAttributes(Aws::String&& key, Aws::String&& value) { m_attributes.emplace(std::move(key), std::move(value)); return *this; }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& AddAttributes(const char* key, Aws::String&& value) { m_attributes.emplace(key, std::move(value)); return *this; }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& AddAttributes(Aws::String&& key, const char* value) { m_attributes.emplace(std::move(key), value); return *this; }

    /**
     * <p>A map of the topic's attributes. Attributes in this map include the
     * following:</p> <ul> <li> <p> <code>DeliveryPolicy</code> – The JSON
     * serialization of the topic's delivery policy.</p> </li> <li> <p>
     * <code>DisplayName</code> – The human-readable name used in the <code>From</code>
     * field for notifications to <code>email</code> and <code>email-json</code>
     * endpoints.</p> </li> <li> <p> <code>Owner</code> – The AWS account ID of the
     * topic's owner.</p> </li> <li> <p> <code>Policy</code> – The JSON serialization
     * of the topic's access control policy.</p> </li> <li> <p>
     * <code>SubscriptionsConfirmed</code> – The number of confirmed subscriptions for
     * the topic.</p> </li> <li> <p> <code>SubscriptionsDeleted</code> – The number of
     * deleted subscriptions for the topic.</p> </li> <li> <p>
     * <code>SubscriptionsPending</code> – The number of subscriptions pending
     * confirmation for the topic.</p> </li> <li> <p> <code>TopicArn</code> – The
     * topic's ARN.</p> </li> <li> <p> <code>EffectiveDeliveryPolicy</code> – The JSON
     * serialization of the effective delivery policy, taking system defaults into
     * account.</p> </li> </ul> <p>The following attribute applies only to <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html">server-side-encryption</a>:</p>
     * <ul> <li> <p> <code>KmsMasterKeyId</code> - The ID of an AWS-managed customer
     * master key (CMK) for Amazon SNS or a custom CMK. For more information, see <a
     * href="https://docs.aws.amazon.com/sns/latest/dg/sns-server-side-encryption.html#sse-key-terms">Key
     * Terms</a>. For more examples, see <a
     * href="https://docs.aws.amazon.com/kms/latest/APIReference/API_DescribeKey.html#API_DescribeKey_RequestParameters">KeyId</a>
     * in the <i>AWS Key Management Service API Reference</i>.</p> </li> </ul>
     */
    inline GetTopicAttributesResult& AddAttributes(const char* key, const char* value) { m_attributes.emplace(key, value); return *this; }


    
    inline const ResponseMetadata& GetResponseMetadata() const{ return m_responseMetadata; }

    
    inline void SetResponseMetadata(const ResponseMetadata& value) { m_responseMetadata = value; }

    
    inline void SetResponseMetadata(ResponseMetadata&& value) { m_responseMetadata = std::move(value); }

    
    inline GetTopicAttributesResult& WithResponseMetadata(const ResponseMetadata& value) { SetResponseMetadata(value); return *this;}

    
    inline GetTopicAttributesResult& WithResponseMetadata(ResponseMetadata&& value) { SetResponseMetadata(std::move(value)); return *this;}

  private:

    Aws::Map<Aws::String, Aws::String> m_attributes;

    ResponseMetadata m_responseMetadata;
  };

} // namespace Model
} // namespace SNS
} // namespace Aws
