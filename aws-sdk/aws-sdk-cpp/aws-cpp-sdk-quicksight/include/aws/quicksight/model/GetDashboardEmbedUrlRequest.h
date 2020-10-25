﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/quicksight/QuickSight_EXPORTS.h>
#include <aws/quicksight/QuickSightRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/quicksight/model/IdentityType.h>
#include <utility>

namespace Aws
{
namespace Http
{
    class URI;
} //namespace Http
namespace QuickSight
{
namespace Model
{

  /**
   */
  class AWS_QUICKSIGHT_API GetDashboardEmbedUrlRequest : public QuickSightRequest
  {
  public:
    GetDashboardEmbedUrlRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "GetDashboardEmbedUrl"; }

    Aws::String SerializePayload() const override;

    void AddQueryStringParameters(Aws::Http::URI& uri) const override;


    /**
     * <p>The ID for the AWS account that contains the dashboard that you're
     * embedding.</p>
     */
    inline const Aws::String& GetAwsAccountId() const{ return m_awsAccountId; }

    /**
     * <p>The ID for the AWS account that contains the dashboard that you're
     * embedding.</p>
     */
    inline bool AwsAccountIdHasBeenSet() const { return m_awsAccountIdHasBeenSet; }

    /**
     * <p>The ID for the AWS account that contains the dashboard that you're
     * embedding.</p>
     */
    inline void SetAwsAccountId(const Aws::String& value) { m_awsAccountIdHasBeenSet = true; m_awsAccountId = value; }

    /**
     * <p>The ID for the AWS account that contains the dashboard that you're
     * embedding.</p>
     */
    inline void SetAwsAccountId(Aws::String&& value) { m_awsAccountIdHasBeenSet = true; m_awsAccountId = std::move(value); }

    /**
     * <p>The ID for the AWS account that contains the dashboard that you're
     * embedding.</p>
     */
    inline void SetAwsAccountId(const char* value) { m_awsAccountIdHasBeenSet = true; m_awsAccountId.assign(value); }

    /**
     * <p>The ID for the AWS account that contains the dashboard that you're
     * embedding.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithAwsAccountId(const Aws::String& value) { SetAwsAccountId(value); return *this;}

    /**
     * <p>The ID for the AWS account that contains the dashboard that you're
     * embedding.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithAwsAccountId(Aws::String&& value) { SetAwsAccountId(std::move(value)); return *this;}

    /**
     * <p>The ID for the AWS account that contains the dashboard that you're
     * embedding.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithAwsAccountId(const char* value) { SetAwsAccountId(value); return *this;}


    /**
     * <p>The ID for the dashboard, also added to the IAM policy.</p>
     */
    inline const Aws::String& GetDashboardId() const{ return m_dashboardId; }

    /**
     * <p>The ID for the dashboard, also added to the IAM policy.</p>
     */
    inline bool DashboardIdHasBeenSet() const { return m_dashboardIdHasBeenSet; }

    /**
     * <p>The ID for the dashboard, also added to the IAM policy.</p>
     */
    inline void SetDashboardId(const Aws::String& value) { m_dashboardIdHasBeenSet = true; m_dashboardId = value; }

    /**
     * <p>The ID for the dashboard, also added to the IAM policy.</p>
     */
    inline void SetDashboardId(Aws::String&& value) { m_dashboardIdHasBeenSet = true; m_dashboardId = std::move(value); }

    /**
     * <p>The ID for the dashboard, also added to the IAM policy.</p>
     */
    inline void SetDashboardId(const char* value) { m_dashboardIdHasBeenSet = true; m_dashboardId.assign(value); }

    /**
     * <p>The ID for the dashboard, also added to the IAM policy.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithDashboardId(const Aws::String& value) { SetDashboardId(value); return *this;}

    /**
     * <p>The ID for the dashboard, also added to the IAM policy.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithDashboardId(Aws::String&& value) { SetDashboardId(std::move(value)); return *this;}

    /**
     * <p>The ID for the dashboard, also added to the IAM policy.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithDashboardId(const char* value) { SetDashboardId(value); return *this;}


    /**
     * <p>The authentication method that the user uses to sign in.</p>
     */
    inline const IdentityType& GetIdentityType() const{ return m_identityType; }

    /**
     * <p>The authentication method that the user uses to sign in.</p>
     */
    inline bool IdentityTypeHasBeenSet() const { return m_identityTypeHasBeenSet; }

    /**
     * <p>The authentication method that the user uses to sign in.</p>
     */
    inline void SetIdentityType(const IdentityType& value) { m_identityTypeHasBeenSet = true; m_identityType = value; }

    /**
     * <p>The authentication method that the user uses to sign in.</p>
     */
    inline void SetIdentityType(IdentityType&& value) { m_identityTypeHasBeenSet = true; m_identityType = std::move(value); }

    /**
     * <p>The authentication method that the user uses to sign in.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithIdentityType(const IdentityType& value) { SetIdentityType(value); return *this;}

    /**
     * <p>The authentication method that the user uses to sign in.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithIdentityType(IdentityType&& value) { SetIdentityType(std::move(value)); return *this;}


    /**
     * <p>How many minutes the session is valid. The session lifetime must be 15-600
     * minutes.</p>
     */
    inline long long GetSessionLifetimeInMinutes() const{ return m_sessionLifetimeInMinutes; }

    /**
     * <p>How many minutes the session is valid. The session lifetime must be 15-600
     * minutes.</p>
     */
    inline bool SessionLifetimeInMinutesHasBeenSet() const { return m_sessionLifetimeInMinutesHasBeenSet; }

    /**
     * <p>How many minutes the session is valid. The session lifetime must be 15-600
     * minutes.</p>
     */
    inline void SetSessionLifetimeInMinutes(long long value) { m_sessionLifetimeInMinutesHasBeenSet = true; m_sessionLifetimeInMinutes = value; }

    /**
     * <p>How many minutes the session is valid. The session lifetime must be 15-600
     * minutes.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithSessionLifetimeInMinutes(long long value) { SetSessionLifetimeInMinutes(value); return *this;}


    /**
     * <p>Remove the undo/redo button on the embedded dashboard. The default is FALSE,
     * which enables the undo/redo button.</p>
     */
    inline bool GetUndoRedoDisabled() const{ return m_undoRedoDisabled; }

    /**
     * <p>Remove the undo/redo button on the embedded dashboard. The default is FALSE,
     * which enables the undo/redo button.</p>
     */
    inline bool UndoRedoDisabledHasBeenSet() const { return m_undoRedoDisabledHasBeenSet; }

    /**
     * <p>Remove the undo/redo button on the embedded dashboard. The default is FALSE,
     * which enables the undo/redo button.</p>
     */
    inline void SetUndoRedoDisabled(bool value) { m_undoRedoDisabledHasBeenSet = true; m_undoRedoDisabled = value; }

    /**
     * <p>Remove the undo/redo button on the embedded dashboard. The default is FALSE,
     * which enables the undo/redo button.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithUndoRedoDisabled(bool value) { SetUndoRedoDisabled(value); return *this;}


    /**
     * <p>Remove the reset button on the embedded dashboard. The default is FALSE,
     * which enables the reset button.</p>
     */
    inline bool GetResetDisabled() const{ return m_resetDisabled; }

    /**
     * <p>Remove the reset button on the embedded dashboard. The default is FALSE,
     * which enables the reset button.</p>
     */
    inline bool ResetDisabledHasBeenSet() const { return m_resetDisabledHasBeenSet; }

    /**
     * <p>Remove the reset button on the embedded dashboard. The default is FALSE,
     * which enables the reset button.</p>
     */
    inline void SetResetDisabled(bool value) { m_resetDisabledHasBeenSet = true; m_resetDisabled = value; }

    /**
     * <p>Remove the reset button on the embedded dashboard. The default is FALSE,
     * which enables the reset button.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithResetDisabled(bool value) { SetResetDisabled(value); return *this;}


    /**
     * <p>The Amazon QuickSight user's Amazon Resource Name (ARN), for use with
     * <code>QUICKSIGHT</code> identity type. You can use this for any Amazon
     * QuickSight users in your account (readers, authors, or admins) authenticated as
     * one of the following:</p> <ul> <li> <p>Active Directory (AD) users or group
     * members</p> </li> <li> <p>Invited nonfederated users</p> </li> <li> <p>IAM users
     * and IAM role-based sessions authenticated through Federated Single Sign-On using
     * SAML, OpenID Connect, or IAM federation.</p> </li> </ul> <p>Omit this parameter
     * for users in the third group – IAM users and IAM role-based sessions.</p>
     */
    inline const Aws::String& GetUserArn() const{ return m_userArn; }

    /**
     * <p>The Amazon QuickSight user's Amazon Resource Name (ARN), for use with
     * <code>QUICKSIGHT</code> identity type. You can use this for any Amazon
     * QuickSight users in your account (readers, authors, or admins) authenticated as
     * one of the following:</p> <ul> <li> <p>Active Directory (AD) users or group
     * members</p> </li> <li> <p>Invited nonfederated users</p> </li> <li> <p>IAM users
     * and IAM role-based sessions authenticated through Federated Single Sign-On using
     * SAML, OpenID Connect, or IAM federation.</p> </li> </ul> <p>Omit this parameter
     * for users in the third group – IAM users and IAM role-based sessions.</p>
     */
    inline bool UserArnHasBeenSet() const { return m_userArnHasBeenSet; }

    /**
     * <p>The Amazon QuickSight user's Amazon Resource Name (ARN), for use with
     * <code>QUICKSIGHT</code> identity type. You can use this for any Amazon
     * QuickSight users in your account (readers, authors, or admins) authenticated as
     * one of the following:</p> <ul> <li> <p>Active Directory (AD) users or group
     * members</p> </li> <li> <p>Invited nonfederated users</p> </li> <li> <p>IAM users
     * and IAM role-based sessions authenticated through Federated Single Sign-On using
     * SAML, OpenID Connect, or IAM federation.</p> </li> </ul> <p>Omit this parameter
     * for users in the third group – IAM users and IAM role-based sessions.</p>
     */
    inline void SetUserArn(const Aws::String& value) { m_userArnHasBeenSet = true; m_userArn = value; }

    /**
     * <p>The Amazon QuickSight user's Amazon Resource Name (ARN), for use with
     * <code>QUICKSIGHT</code> identity type. You can use this for any Amazon
     * QuickSight users in your account (readers, authors, or admins) authenticated as
     * one of the following:</p> <ul> <li> <p>Active Directory (AD) users or group
     * members</p> </li> <li> <p>Invited nonfederated users</p> </li> <li> <p>IAM users
     * and IAM role-based sessions authenticated through Federated Single Sign-On using
     * SAML, OpenID Connect, or IAM federation.</p> </li> </ul> <p>Omit this parameter
     * for users in the third group – IAM users and IAM role-based sessions.</p>
     */
    inline void SetUserArn(Aws::String&& value) { m_userArnHasBeenSet = true; m_userArn = std::move(value); }

    /**
     * <p>The Amazon QuickSight user's Amazon Resource Name (ARN), for use with
     * <code>QUICKSIGHT</code> identity type. You can use this for any Amazon
     * QuickSight users in your account (readers, authors, or admins) authenticated as
     * one of the following:</p> <ul> <li> <p>Active Directory (AD) users or group
     * members</p> </li> <li> <p>Invited nonfederated users</p> </li> <li> <p>IAM users
     * and IAM role-based sessions authenticated through Federated Single Sign-On using
     * SAML, OpenID Connect, or IAM federation.</p> </li> </ul> <p>Omit this parameter
     * for users in the third group – IAM users and IAM role-based sessions.</p>
     */
    inline void SetUserArn(const char* value) { m_userArnHasBeenSet = true; m_userArn.assign(value); }

    /**
     * <p>The Amazon QuickSight user's Amazon Resource Name (ARN), for use with
     * <code>QUICKSIGHT</code> identity type. You can use this for any Amazon
     * QuickSight users in your account (readers, authors, or admins) authenticated as
     * one of the following:</p> <ul> <li> <p>Active Directory (AD) users or group
     * members</p> </li> <li> <p>Invited nonfederated users</p> </li> <li> <p>IAM users
     * and IAM role-based sessions authenticated through Federated Single Sign-On using
     * SAML, OpenID Connect, or IAM federation.</p> </li> </ul> <p>Omit this parameter
     * for users in the third group – IAM users and IAM role-based sessions.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithUserArn(const Aws::String& value) { SetUserArn(value); return *this;}

    /**
     * <p>The Amazon QuickSight user's Amazon Resource Name (ARN), for use with
     * <code>QUICKSIGHT</code> identity type. You can use this for any Amazon
     * QuickSight users in your account (readers, authors, or admins) authenticated as
     * one of the following:</p> <ul> <li> <p>Active Directory (AD) users or group
     * members</p> </li> <li> <p>Invited nonfederated users</p> </li> <li> <p>IAM users
     * and IAM role-based sessions authenticated through Federated Single Sign-On using
     * SAML, OpenID Connect, or IAM federation.</p> </li> </ul> <p>Omit this parameter
     * for users in the third group – IAM users and IAM role-based sessions.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithUserArn(Aws::String&& value) { SetUserArn(std::move(value)); return *this;}

    /**
     * <p>The Amazon QuickSight user's Amazon Resource Name (ARN), for use with
     * <code>QUICKSIGHT</code> identity type. You can use this for any Amazon
     * QuickSight users in your account (readers, authors, or admins) authenticated as
     * one of the following:</p> <ul> <li> <p>Active Directory (AD) users or group
     * members</p> </li> <li> <p>Invited nonfederated users</p> </li> <li> <p>IAM users
     * and IAM role-based sessions authenticated through Federated Single Sign-On using
     * SAML, OpenID Connect, or IAM federation.</p> </li> </ul> <p>Omit this parameter
     * for users in the third group – IAM users and IAM role-based sessions.</p>
     */
    inline GetDashboardEmbedUrlRequest& WithUserArn(const char* value) { SetUserArn(value); return *this;}

  private:

    Aws::String m_awsAccountId;
    bool m_awsAccountIdHasBeenSet;

    Aws::String m_dashboardId;
    bool m_dashboardIdHasBeenSet;

    IdentityType m_identityType;
    bool m_identityTypeHasBeenSet;

    long long m_sessionLifetimeInMinutes;
    bool m_sessionLifetimeInMinutesHasBeenSet;

    bool m_undoRedoDisabled;
    bool m_undoRedoDisabledHasBeenSet;

    bool m_resetDisabled;
    bool m_resetDisabledHasBeenSet;

    Aws::String m_userArn;
    bool m_userArnHasBeenSet;
  };

} // namespace Model
} // namespace QuickSight
} // namespace Aws
