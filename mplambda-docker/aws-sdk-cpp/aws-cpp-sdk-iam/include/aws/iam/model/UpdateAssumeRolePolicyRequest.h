﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/iam/IAM_EXPORTS.h>
#include <aws/iam/IAMRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
namespace IAM
{
namespace Model
{

  /**
   */
  class AWS_IAM_API UpdateAssumeRolePolicyRequest : public IAMRequest
  {
  public:
    UpdateAssumeRolePolicyRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "UpdateAssumeRolePolicy"; }

    Aws::String SerializePayload() const override;

  protected:
    void DumpBodyToUrl(Aws::Http::URI& uri ) const override;

  public:

    /**
     * <p>The name of the role to update with the new policy.</p> <p>This parameter
     * allows (through its <a href="http://wikipedia.org/wiki/regex">regex pattern</a>)
     * a string of characters consisting of upper and lowercase alphanumeric characters
     * with no spaces. You can also include any of the following characters:
     * _+=,.@-</p>
     */
    inline const Aws::String& GetRoleName() const{ return m_roleName; }

    /**
     * <p>The name of the role to update with the new policy.</p> <p>This parameter
     * allows (through its <a href="http://wikipedia.org/wiki/regex">regex pattern</a>)
     * a string of characters consisting of upper and lowercase alphanumeric characters
     * with no spaces. You can also include any of the following characters:
     * _+=,.@-</p>
     */
    inline bool RoleNameHasBeenSet() const { return m_roleNameHasBeenSet; }

    /**
     * <p>The name of the role to update with the new policy.</p> <p>This parameter
     * allows (through its <a href="http://wikipedia.org/wiki/regex">regex pattern</a>)
     * a string of characters consisting of upper and lowercase alphanumeric characters
     * with no spaces. You can also include any of the following characters:
     * _+=,.@-</p>
     */
    inline void SetRoleName(const Aws::String& value) { m_roleNameHasBeenSet = true; m_roleName = value; }

    /**
     * <p>The name of the role to update with the new policy.</p> <p>This parameter
     * allows (through its <a href="http://wikipedia.org/wiki/regex">regex pattern</a>)
     * a string of characters consisting of upper and lowercase alphanumeric characters
     * with no spaces. You can also include any of the following characters:
     * _+=,.@-</p>
     */
    inline void SetRoleName(Aws::String&& value) { m_roleNameHasBeenSet = true; m_roleName = std::move(value); }

    /**
     * <p>The name of the role to update with the new policy.</p> <p>This parameter
     * allows (through its <a href="http://wikipedia.org/wiki/regex">regex pattern</a>)
     * a string of characters consisting of upper and lowercase alphanumeric characters
     * with no spaces. You can also include any of the following characters:
     * _+=,.@-</p>
     */
    inline void SetRoleName(const char* value) { m_roleNameHasBeenSet = true; m_roleName.assign(value); }

    /**
     * <p>The name of the role to update with the new policy.</p> <p>This parameter
     * allows (through its <a href="http://wikipedia.org/wiki/regex">regex pattern</a>)
     * a string of characters consisting of upper and lowercase alphanumeric characters
     * with no spaces. You can also include any of the following characters:
     * _+=,.@-</p>
     */
    inline UpdateAssumeRolePolicyRequest& WithRoleName(const Aws::String& value) { SetRoleName(value); return *this;}

    /**
     * <p>The name of the role to update with the new policy.</p> <p>This parameter
     * allows (through its <a href="http://wikipedia.org/wiki/regex">regex pattern</a>)
     * a string of characters consisting of upper and lowercase alphanumeric characters
     * with no spaces. You can also include any of the following characters:
     * _+=,.@-</p>
     */
    inline UpdateAssumeRolePolicyRequest& WithRoleName(Aws::String&& value) { SetRoleName(std::move(value)); return *this;}

    /**
     * <p>The name of the role to update with the new policy.</p> <p>This parameter
     * allows (through its <a href="http://wikipedia.org/wiki/regex">regex pattern</a>)
     * a string of characters consisting of upper and lowercase alphanumeric characters
     * with no spaces. You can also include any of the following characters:
     * _+=,.@-</p>
     */
    inline UpdateAssumeRolePolicyRequest& WithRoleName(const char* value) { SetRoleName(value); return *this;}


    /**
     * <p>The policy that grants an entity permission to assume the role.</p> <p>You
     * must provide policies in JSON format in IAM. However, for AWS CloudFormation
     * templates formatted in YAML, you can provide the policy in JSON or YAML format.
     * AWS CloudFormation always converts a YAML policy to JSON format before
     * submitting it to IAM.</p> <p>The <a href="http://wikipedia.org/wiki/regex">regex
     * pattern</a> used to validate this parameter is a string of characters consisting
     * of the following:</p> <ul> <li> <p>Any printable ASCII character ranging from
     * the space character (<code>\u0020</code>) through the end of the ASCII character
     * range</p> </li> <li> <p>The printable characters in the Basic Latin and Latin-1
     * Supplement character set (through <code>\u00FF</code>)</p> </li> <li> <p>The
     * special characters tab (<code>\u0009</code>), line feed (<code>\u000A</code>),
     * and carriage return (<code>\u000D</code>)</p> </li> </ul>
     */
    inline const Aws::String& GetPolicyDocument() const{ return m_policyDocument; }

    /**
     * <p>The policy that grants an entity permission to assume the role.</p> <p>You
     * must provide policies in JSON format in IAM. However, for AWS CloudFormation
     * templates formatted in YAML, you can provide the policy in JSON or YAML format.
     * AWS CloudFormation always converts a YAML policy to JSON format before
     * submitting it to IAM.</p> <p>The <a href="http://wikipedia.org/wiki/regex">regex
     * pattern</a> used to validate this parameter is a string of characters consisting
     * of the following:</p> <ul> <li> <p>Any printable ASCII character ranging from
     * the space character (<code>\u0020</code>) through the end of the ASCII character
     * range</p> </li> <li> <p>The printable characters in the Basic Latin and Latin-1
     * Supplement character set (through <code>\u00FF</code>)</p> </li> <li> <p>The
     * special characters tab (<code>\u0009</code>), line feed (<code>\u000A</code>),
     * and carriage return (<code>\u000D</code>)</p> </li> </ul>
     */
    inline bool PolicyDocumentHasBeenSet() const { return m_policyDocumentHasBeenSet; }

    /**
     * <p>The policy that grants an entity permission to assume the role.</p> <p>You
     * must provide policies in JSON format in IAM. However, for AWS CloudFormation
     * templates formatted in YAML, you can provide the policy in JSON or YAML format.
     * AWS CloudFormation always converts a YAML policy to JSON format before
     * submitting it to IAM.</p> <p>The <a href="http://wikipedia.org/wiki/regex">regex
     * pattern</a> used to validate this parameter is a string of characters consisting
     * of the following:</p> <ul> <li> <p>Any printable ASCII character ranging from
     * the space character (<code>\u0020</code>) through the end of the ASCII character
     * range</p> </li> <li> <p>The printable characters in the Basic Latin and Latin-1
     * Supplement character set (through <code>\u00FF</code>)</p> </li> <li> <p>The
     * special characters tab (<code>\u0009</code>), line feed (<code>\u000A</code>),
     * and carriage return (<code>\u000D</code>)</p> </li> </ul>
     */
    inline void SetPolicyDocument(const Aws::String& value) { m_policyDocumentHasBeenSet = true; m_policyDocument = value; }

    /**
     * <p>The policy that grants an entity permission to assume the role.</p> <p>You
     * must provide policies in JSON format in IAM. However, for AWS CloudFormation
     * templates formatted in YAML, you can provide the policy in JSON or YAML format.
     * AWS CloudFormation always converts a YAML policy to JSON format before
     * submitting it to IAM.</p> <p>The <a href="http://wikipedia.org/wiki/regex">regex
     * pattern</a> used to validate this parameter is a string of characters consisting
     * of the following:</p> <ul> <li> <p>Any printable ASCII character ranging from
     * the space character (<code>\u0020</code>) through the end of the ASCII character
     * range</p> </li> <li> <p>The printable characters in the Basic Latin and Latin-1
     * Supplement character set (through <code>\u00FF</code>)</p> </li> <li> <p>The
     * special characters tab (<code>\u0009</code>), line feed (<code>\u000A</code>),
     * and carriage return (<code>\u000D</code>)</p> </li> </ul>
     */
    inline void SetPolicyDocument(Aws::String&& value) { m_policyDocumentHasBeenSet = true; m_policyDocument = std::move(value); }

    /**
     * <p>The policy that grants an entity permission to assume the role.</p> <p>You
     * must provide policies in JSON format in IAM. However, for AWS CloudFormation
     * templates formatted in YAML, you can provide the policy in JSON or YAML format.
     * AWS CloudFormation always converts a YAML policy to JSON format before
     * submitting it to IAM.</p> <p>The <a href="http://wikipedia.org/wiki/regex">regex
     * pattern</a> used to validate this parameter is a string of characters consisting
     * of the following:</p> <ul> <li> <p>Any printable ASCII character ranging from
     * the space character (<code>\u0020</code>) through the end of the ASCII character
     * range</p> </li> <li> <p>The printable characters in the Basic Latin and Latin-1
     * Supplement character set (through <code>\u00FF</code>)</p> </li> <li> <p>The
     * special characters tab (<code>\u0009</code>), line feed (<code>\u000A</code>),
     * and carriage return (<code>\u000D</code>)</p> </li> </ul>
     */
    inline void SetPolicyDocument(const char* value) { m_policyDocumentHasBeenSet = true; m_policyDocument.assign(value); }

    /**
     * <p>The policy that grants an entity permission to assume the role.</p> <p>You
     * must provide policies in JSON format in IAM. However, for AWS CloudFormation
     * templates formatted in YAML, you can provide the policy in JSON or YAML format.
     * AWS CloudFormation always converts a YAML policy to JSON format before
     * submitting it to IAM.</p> <p>The <a href="http://wikipedia.org/wiki/regex">regex
     * pattern</a> used to validate this parameter is a string of characters consisting
     * of the following:</p> <ul> <li> <p>Any printable ASCII character ranging from
     * the space character (<code>\u0020</code>) through the end of the ASCII character
     * range</p> </li> <li> <p>The printable characters in the Basic Latin and Latin-1
     * Supplement character set (through <code>\u00FF</code>)</p> </li> <li> <p>The
     * special characters tab (<code>\u0009</code>), line feed (<code>\u000A</code>),
     * and carriage return (<code>\u000D</code>)</p> </li> </ul>
     */
    inline UpdateAssumeRolePolicyRequest& WithPolicyDocument(const Aws::String& value) { SetPolicyDocument(value); return *this;}

    /**
     * <p>The policy that grants an entity permission to assume the role.</p> <p>You
     * must provide policies in JSON format in IAM. However, for AWS CloudFormation
     * templates formatted in YAML, you can provide the policy in JSON or YAML format.
     * AWS CloudFormation always converts a YAML policy to JSON format before
     * submitting it to IAM.</p> <p>The <a href="http://wikipedia.org/wiki/regex">regex
     * pattern</a> used to validate this parameter is a string of characters consisting
     * of the following:</p> <ul> <li> <p>Any printable ASCII character ranging from
     * the space character (<code>\u0020</code>) through the end of the ASCII character
     * range</p> </li> <li> <p>The printable characters in the Basic Latin and Latin-1
     * Supplement character set (through <code>\u00FF</code>)</p> </li> <li> <p>The
     * special characters tab (<code>\u0009</code>), line feed (<code>\u000A</code>),
     * and carriage return (<code>\u000D</code>)</p> </li> </ul>
     */
    inline UpdateAssumeRolePolicyRequest& WithPolicyDocument(Aws::String&& value) { SetPolicyDocument(std::move(value)); return *this;}

    /**
     * <p>The policy that grants an entity permission to assume the role.</p> <p>You
     * must provide policies in JSON format in IAM. However, for AWS CloudFormation
     * templates formatted in YAML, you can provide the policy in JSON or YAML format.
     * AWS CloudFormation always converts a YAML policy to JSON format before
     * submitting it to IAM.</p> <p>The <a href="http://wikipedia.org/wiki/regex">regex
     * pattern</a> used to validate this parameter is a string of characters consisting
     * of the following:</p> <ul> <li> <p>Any printable ASCII character ranging from
     * the space character (<code>\u0020</code>) through the end of the ASCII character
     * range</p> </li> <li> <p>The printable characters in the Basic Latin and Latin-1
     * Supplement character set (through <code>\u00FF</code>)</p> </li> <li> <p>The
     * special characters tab (<code>\u0009</code>), line feed (<code>\u000A</code>),
     * and carriage return (<code>\u000D</code>)</p> </li> </ul>
     */
    inline UpdateAssumeRolePolicyRequest& WithPolicyDocument(const char* value) { SetPolicyDocument(value); return *this;}

  private:

    Aws::String m_roleName;
    bool m_roleNameHasBeenSet;

    Aws::String m_policyDocument;
    bool m_policyDocumentHasBeenSet;
  };

} // namespace Model
} // namespace IAM
} // namespace Aws
