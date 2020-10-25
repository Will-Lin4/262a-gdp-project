﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/elasticloadbalancingv2/ElasticLoadBalancingv2_EXPORTS.h>
#include <aws/elasticloadbalancingv2/ElasticLoadBalancingv2Request.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/elasticloadbalancingv2/model/RuleCondition.h>
#include <aws/elasticloadbalancingv2/model/Action.h>
#include <aws/elasticloadbalancingv2/model/Tag.h>
#include <utility>

namespace Aws
{
namespace ElasticLoadBalancingv2
{
namespace Model
{

  /**
   */
  class AWS_ELASTICLOADBALANCINGV2_API CreateRuleRequest : public ElasticLoadBalancingv2Request
  {
  public:
    CreateRuleRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "CreateRule"; }

    Aws::String SerializePayload() const override;

  protected:
    void DumpBodyToUrl(Aws::Http::URI& uri ) const override;

  public:

    /**
     * <p>The Amazon Resource Name (ARN) of the listener.</p>
     */
    inline const Aws::String& GetListenerArn() const{ return m_listenerArn; }

    /**
     * <p>The Amazon Resource Name (ARN) of the listener.</p>
     */
    inline bool ListenerArnHasBeenSet() const { return m_listenerArnHasBeenSet; }

    /**
     * <p>The Amazon Resource Name (ARN) of the listener.</p>
     */
    inline void SetListenerArn(const Aws::String& value) { m_listenerArnHasBeenSet = true; m_listenerArn = value; }

    /**
     * <p>The Amazon Resource Name (ARN) of the listener.</p>
     */
    inline void SetListenerArn(Aws::String&& value) { m_listenerArnHasBeenSet = true; m_listenerArn = std::move(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the listener.</p>
     */
    inline void SetListenerArn(const char* value) { m_listenerArnHasBeenSet = true; m_listenerArn.assign(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the listener.</p>
     */
    inline CreateRuleRequest& WithListenerArn(const Aws::String& value) { SetListenerArn(value); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the listener.</p>
     */
    inline CreateRuleRequest& WithListenerArn(Aws::String&& value) { SetListenerArn(std::move(value)); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the listener.</p>
     */
    inline CreateRuleRequest& WithListenerArn(const char* value) { SetListenerArn(value); return *this;}


    /**
     * <p>The conditions. Each rule can optionally include up to one of each of the
     * following conditions: <code>http-request-method</code>,
     * <code>host-header</code>, <code>path-pattern</code>, and <code>source-ip</code>.
     * Each rule can also optionally include one or more of each of the following
     * conditions: <code>http-header</code> and <code>query-string</code>.</p>
     */
    inline const Aws::Vector<RuleCondition>& GetConditions() const{ return m_conditions; }

    /**
     * <p>The conditions. Each rule can optionally include up to one of each of the
     * following conditions: <code>http-request-method</code>,
     * <code>host-header</code>, <code>path-pattern</code>, and <code>source-ip</code>.
     * Each rule can also optionally include one or more of each of the following
     * conditions: <code>http-header</code> and <code>query-string</code>.</p>
     */
    inline bool ConditionsHasBeenSet() const { return m_conditionsHasBeenSet; }

    /**
     * <p>The conditions. Each rule can optionally include up to one of each of the
     * following conditions: <code>http-request-method</code>,
     * <code>host-header</code>, <code>path-pattern</code>, and <code>source-ip</code>.
     * Each rule can also optionally include one or more of each of the following
     * conditions: <code>http-header</code> and <code>query-string</code>.</p>
     */
    inline void SetConditions(const Aws::Vector<RuleCondition>& value) { m_conditionsHasBeenSet = true; m_conditions = value; }

    /**
     * <p>The conditions. Each rule can optionally include up to one of each of the
     * following conditions: <code>http-request-method</code>,
     * <code>host-header</code>, <code>path-pattern</code>, and <code>source-ip</code>.
     * Each rule can also optionally include one or more of each of the following
     * conditions: <code>http-header</code> and <code>query-string</code>.</p>
     */
    inline void SetConditions(Aws::Vector<RuleCondition>&& value) { m_conditionsHasBeenSet = true; m_conditions = std::move(value); }

    /**
     * <p>The conditions. Each rule can optionally include up to one of each of the
     * following conditions: <code>http-request-method</code>,
     * <code>host-header</code>, <code>path-pattern</code>, and <code>source-ip</code>.
     * Each rule can also optionally include one or more of each of the following
     * conditions: <code>http-header</code> and <code>query-string</code>.</p>
     */
    inline CreateRuleRequest& WithConditions(const Aws::Vector<RuleCondition>& value) { SetConditions(value); return *this;}

    /**
     * <p>The conditions. Each rule can optionally include up to one of each of the
     * following conditions: <code>http-request-method</code>,
     * <code>host-header</code>, <code>path-pattern</code>, and <code>source-ip</code>.
     * Each rule can also optionally include one or more of each of the following
     * conditions: <code>http-header</code> and <code>query-string</code>.</p>
     */
    inline CreateRuleRequest& WithConditions(Aws::Vector<RuleCondition>&& value) { SetConditions(std::move(value)); return *this;}

    /**
     * <p>The conditions. Each rule can optionally include up to one of each of the
     * following conditions: <code>http-request-method</code>,
     * <code>host-header</code>, <code>path-pattern</code>, and <code>source-ip</code>.
     * Each rule can also optionally include one or more of each of the following
     * conditions: <code>http-header</code> and <code>query-string</code>.</p>
     */
    inline CreateRuleRequest& AddConditions(const RuleCondition& value) { m_conditionsHasBeenSet = true; m_conditions.push_back(value); return *this; }

    /**
     * <p>The conditions. Each rule can optionally include up to one of each of the
     * following conditions: <code>http-request-method</code>,
     * <code>host-header</code>, <code>path-pattern</code>, and <code>source-ip</code>.
     * Each rule can also optionally include one or more of each of the following
     * conditions: <code>http-header</code> and <code>query-string</code>.</p>
     */
    inline CreateRuleRequest& AddConditions(RuleCondition&& value) { m_conditionsHasBeenSet = true; m_conditions.push_back(std::move(value)); return *this; }


    /**
     * <p>The rule priority. A listener can't have multiple rules with the same
     * priority.</p>
     */
    inline int GetPriority() const{ return m_priority; }

    /**
     * <p>The rule priority. A listener can't have multiple rules with the same
     * priority.</p>
     */
    inline bool PriorityHasBeenSet() const { return m_priorityHasBeenSet; }

    /**
     * <p>The rule priority. A listener can't have multiple rules with the same
     * priority.</p>
     */
    inline void SetPriority(int value) { m_priorityHasBeenSet = true; m_priority = value; }

    /**
     * <p>The rule priority. A listener can't have multiple rules with the same
     * priority.</p>
     */
    inline CreateRuleRequest& WithPriority(int value) { SetPriority(value); return *this;}


    /**
     * <p>The actions. Each rule must include exactly one of the following types of
     * actions: <code>forward</code>, <code>fixed-response</code>, or
     * <code>redirect</code>, and it must be the last action to be performed.</p> <p>If
     * the action type is <code>forward</code>, you specify one or more target groups.
     * The protocol of the target group must be HTTP or HTTPS for an Application Load
     * Balancer. The protocol of the target group must be TCP, TLS, UDP, or TCP_UDP for
     * a Network Load Balancer.</p> <p>[HTTPS listeners] If the action type is
     * <code>authenticate-oidc</code>, you authenticate users through an identity
     * provider that is OpenID Connect (OIDC) compliant.</p> <p>[HTTPS listeners] If
     * the action type is <code>authenticate-cognito</code>, you authenticate users
     * through the user pools supported by Amazon Cognito.</p> <p>[Application Load
     * Balancer] If the action type is <code>redirect</code>, you redirect specified
     * client requests from one URL to another.</p> <p>[Application Load Balancer] If
     * the action type is <code>fixed-response</code>, you drop specified client
     * requests and return a custom HTTP response.</p>
     */
    inline const Aws::Vector<Action>& GetActions() const{ return m_actions; }

    /**
     * <p>The actions. Each rule must include exactly one of the following types of
     * actions: <code>forward</code>, <code>fixed-response</code>, or
     * <code>redirect</code>, and it must be the last action to be performed.</p> <p>If
     * the action type is <code>forward</code>, you specify one or more target groups.
     * The protocol of the target group must be HTTP or HTTPS for an Application Load
     * Balancer. The protocol of the target group must be TCP, TLS, UDP, or TCP_UDP for
     * a Network Load Balancer.</p> <p>[HTTPS listeners] If the action type is
     * <code>authenticate-oidc</code>, you authenticate users through an identity
     * provider that is OpenID Connect (OIDC) compliant.</p> <p>[HTTPS listeners] If
     * the action type is <code>authenticate-cognito</code>, you authenticate users
     * through the user pools supported by Amazon Cognito.</p> <p>[Application Load
     * Balancer] If the action type is <code>redirect</code>, you redirect specified
     * client requests from one URL to another.</p> <p>[Application Load Balancer] If
     * the action type is <code>fixed-response</code>, you drop specified client
     * requests and return a custom HTTP response.</p>
     */
    inline bool ActionsHasBeenSet() const { return m_actionsHasBeenSet; }

    /**
     * <p>The actions. Each rule must include exactly one of the following types of
     * actions: <code>forward</code>, <code>fixed-response</code>, or
     * <code>redirect</code>, and it must be the last action to be performed.</p> <p>If
     * the action type is <code>forward</code>, you specify one or more target groups.
     * The protocol of the target group must be HTTP or HTTPS for an Application Load
     * Balancer. The protocol of the target group must be TCP, TLS, UDP, or TCP_UDP for
     * a Network Load Balancer.</p> <p>[HTTPS listeners] If the action type is
     * <code>authenticate-oidc</code>, you authenticate users through an identity
     * provider that is OpenID Connect (OIDC) compliant.</p> <p>[HTTPS listeners] If
     * the action type is <code>authenticate-cognito</code>, you authenticate users
     * through the user pools supported by Amazon Cognito.</p> <p>[Application Load
     * Balancer] If the action type is <code>redirect</code>, you redirect specified
     * client requests from one URL to another.</p> <p>[Application Load Balancer] If
     * the action type is <code>fixed-response</code>, you drop specified client
     * requests and return a custom HTTP response.</p>
     */
    inline void SetActions(const Aws::Vector<Action>& value) { m_actionsHasBeenSet = true; m_actions = value; }

    /**
     * <p>The actions. Each rule must include exactly one of the following types of
     * actions: <code>forward</code>, <code>fixed-response</code>, or
     * <code>redirect</code>, and it must be the last action to be performed.</p> <p>If
     * the action type is <code>forward</code>, you specify one or more target groups.
     * The protocol of the target group must be HTTP or HTTPS for an Application Load
     * Balancer. The protocol of the target group must be TCP, TLS, UDP, or TCP_UDP for
     * a Network Load Balancer.</p> <p>[HTTPS listeners] If the action type is
     * <code>authenticate-oidc</code>, you authenticate users through an identity
     * provider that is OpenID Connect (OIDC) compliant.</p> <p>[HTTPS listeners] If
     * the action type is <code>authenticate-cognito</code>, you authenticate users
     * through the user pools supported by Amazon Cognito.</p> <p>[Application Load
     * Balancer] If the action type is <code>redirect</code>, you redirect specified
     * client requests from one URL to another.</p> <p>[Application Load Balancer] If
     * the action type is <code>fixed-response</code>, you drop specified client
     * requests and return a custom HTTP response.</p>
     */
    inline void SetActions(Aws::Vector<Action>&& value) { m_actionsHasBeenSet = true; m_actions = std::move(value); }

    /**
     * <p>The actions. Each rule must include exactly one of the following types of
     * actions: <code>forward</code>, <code>fixed-response</code>, or
     * <code>redirect</code>, and it must be the last action to be performed.</p> <p>If
     * the action type is <code>forward</code>, you specify one or more target groups.
     * The protocol of the target group must be HTTP or HTTPS for an Application Load
     * Balancer. The protocol of the target group must be TCP, TLS, UDP, or TCP_UDP for
     * a Network Load Balancer.</p> <p>[HTTPS listeners] If the action type is
     * <code>authenticate-oidc</code>, you authenticate users through an identity
     * provider that is OpenID Connect (OIDC) compliant.</p> <p>[HTTPS listeners] If
     * the action type is <code>authenticate-cognito</code>, you authenticate users
     * through the user pools supported by Amazon Cognito.</p> <p>[Application Load
     * Balancer] If the action type is <code>redirect</code>, you redirect specified
     * client requests from one URL to another.</p> <p>[Application Load Balancer] If
     * the action type is <code>fixed-response</code>, you drop specified client
     * requests and return a custom HTTP response.</p>
     */
    inline CreateRuleRequest& WithActions(const Aws::Vector<Action>& value) { SetActions(value); return *this;}

    /**
     * <p>The actions. Each rule must include exactly one of the following types of
     * actions: <code>forward</code>, <code>fixed-response</code>, or
     * <code>redirect</code>, and it must be the last action to be performed.</p> <p>If
     * the action type is <code>forward</code>, you specify one or more target groups.
     * The protocol of the target group must be HTTP or HTTPS for an Application Load
     * Balancer. The protocol of the target group must be TCP, TLS, UDP, or TCP_UDP for
     * a Network Load Balancer.</p> <p>[HTTPS listeners] If the action type is
     * <code>authenticate-oidc</code>, you authenticate users through an identity
     * provider that is OpenID Connect (OIDC) compliant.</p> <p>[HTTPS listeners] If
     * the action type is <code>authenticate-cognito</code>, you authenticate users
     * through the user pools supported by Amazon Cognito.</p> <p>[Application Load
     * Balancer] If the action type is <code>redirect</code>, you redirect specified
     * client requests from one URL to another.</p> <p>[Application Load Balancer] If
     * the action type is <code>fixed-response</code>, you drop specified client
     * requests and return a custom HTTP response.</p>
     */
    inline CreateRuleRequest& WithActions(Aws::Vector<Action>&& value) { SetActions(std::move(value)); return *this;}

    /**
     * <p>The actions. Each rule must include exactly one of the following types of
     * actions: <code>forward</code>, <code>fixed-response</code>, or
     * <code>redirect</code>, and it must be the last action to be performed.</p> <p>If
     * the action type is <code>forward</code>, you specify one or more target groups.
     * The protocol of the target group must be HTTP or HTTPS for an Application Load
     * Balancer. The protocol of the target group must be TCP, TLS, UDP, or TCP_UDP for
     * a Network Load Balancer.</p> <p>[HTTPS listeners] If the action type is
     * <code>authenticate-oidc</code>, you authenticate users through an identity
     * provider that is OpenID Connect (OIDC) compliant.</p> <p>[HTTPS listeners] If
     * the action type is <code>authenticate-cognito</code>, you authenticate users
     * through the user pools supported by Amazon Cognito.</p> <p>[Application Load
     * Balancer] If the action type is <code>redirect</code>, you redirect specified
     * client requests from one URL to another.</p> <p>[Application Load Balancer] If
     * the action type is <code>fixed-response</code>, you drop specified client
     * requests and return a custom HTTP response.</p>
     */
    inline CreateRuleRequest& AddActions(const Action& value) { m_actionsHasBeenSet = true; m_actions.push_back(value); return *this; }

    /**
     * <p>The actions. Each rule must include exactly one of the following types of
     * actions: <code>forward</code>, <code>fixed-response</code>, or
     * <code>redirect</code>, and it must be the last action to be performed.</p> <p>If
     * the action type is <code>forward</code>, you specify one or more target groups.
     * The protocol of the target group must be HTTP or HTTPS for an Application Load
     * Balancer. The protocol of the target group must be TCP, TLS, UDP, or TCP_UDP for
     * a Network Load Balancer.</p> <p>[HTTPS listeners] If the action type is
     * <code>authenticate-oidc</code>, you authenticate users through an identity
     * provider that is OpenID Connect (OIDC) compliant.</p> <p>[HTTPS listeners] If
     * the action type is <code>authenticate-cognito</code>, you authenticate users
     * through the user pools supported by Amazon Cognito.</p> <p>[Application Load
     * Balancer] If the action type is <code>redirect</code>, you redirect specified
     * client requests from one URL to another.</p> <p>[Application Load Balancer] If
     * the action type is <code>fixed-response</code>, you drop specified client
     * requests and return a custom HTTP response.</p>
     */
    inline CreateRuleRequest& AddActions(Action&& value) { m_actionsHasBeenSet = true; m_actions.push_back(std::move(value)); return *this; }


    /**
     * <p>The tags to assign to the rule.</p>
     */
    inline const Aws::Vector<Tag>& GetTags() const{ return m_tags; }

    /**
     * <p>The tags to assign to the rule.</p>
     */
    inline bool TagsHasBeenSet() const { return m_tagsHasBeenSet; }

    /**
     * <p>The tags to assign to the rule.</p>
     */
    inline void SetTags(const Aws::Vector<Tag>& value) { m_tagsHasBeenSet = true; m_tags = value; }

    /**
     * <p>The tags to assign to the rule.</p>
     */
    inline void SetTags(Aws::Vector<Tag>&& value) { m_tagsHasBeenSet = true; m_tags = std::move(value); }

    /**
     * <p>The tags to assign to the rule.</p>
     */
    inline CreateRuleRequest& WithTags(const Aws::Vector<Tag>& value) { SetTags(value); return *this;}

    /**
     * <p>The tags to assign to the rule.</p>
     */
    inline CreateRuleRequest& WithTags(Aws::Vector<Tag>&& value) { SetTags(std::move(value)); return *this;}

    /**
     * <p>The tags to assign to the rule.</p>
     */
    inline CreateRuleRequest& AddTags(const Tag& value) { m_tagsHasBeenSet = true; m_tags.push_back(value); return *this; }

    /**
     * <p>The tags to assign to the rule.</p>
     */
    inline CreateRuleRequest& AddTags(Tag&& value) { m_tagsHasBeenSet = true; m_tags.push_back(std::move(value)); return *this; }

  private:

    Aws::String m_listenerArn;
    bool m_listenerArnHasBeenSet;

    Aws::Vector<RuleCondition> m_conditions;
    bool m_conditionsHasBeenSet;

    int m_priority;
    bool m_priorityHasBeenSet;

    Aws::Vector<Action> m_actions;
    bool m_actionsHasBeenSet;

    Aws::Vector<Tag> m_tags;
    bool m_tagsHasBeenSet;
  };

} // namespace Model
} // namespace ElasticLoadBalancingv2
} // namespace Aws
