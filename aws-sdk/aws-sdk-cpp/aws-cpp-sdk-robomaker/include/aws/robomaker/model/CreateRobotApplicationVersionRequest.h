﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/robomaker/RoboMaker_EXPORTS.h>
#include <aws/robomaker/RoboMakerRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
namespace RoboMaker
{
namespace Model
{

  /**
   */
  class AWS_ROBOMAKER_API CreateRobotApplicationVersionRequest : public RoboMakerRequest
  {
  public:
    CreateRobotApplicationVersionRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "CreateRobotApplicationVersion"; }

    Aws::String SerializePayload() const override;


    /**
     * <p>The application information for the robot application.</p>
     */
    inline const Aws::String& GetApplication() const{ return m_application; }

    /**
     * <p>The application information for the robot application.</p>
     */
    inline bool ApplicationHasBeenSet() const { return m_applicationHasBeenSet; }

    /**
     * <p>The application information for the robot application.</p>
     */
    inline void SetApplication(const Aws::String& value) { m_applicationHasBeenSet = true; m_application = value; }

    /**
     * <p>The application information for the robot application.</p>
     */
    inline void SetApplication(Aws::String&& value) { m_applicationHasBeenSet = true; m_application = std::move(value); }

    /**
     * <p>The application information for the robot application.</p>
     */
    inline void SetApplication(const char* value) { m_applicationHasBeenSet = true; m_application.assign(value); }

    /**
     * <p>The application information for the robot application.</p>
     */
    inline CreateRobotApplicationVersionRequest& WithApplication(const Aws::String& value) { SetApplication(value); return *this;}

    /**
     * <p>The application information for the robot application.</p>
     */
    inline CreateRobotApplicationVersionRequest& WithApplication(Aws::String&& value) { SetApplication(std::move(value)); return *this;}

    /**
     * <p>The application information for the robot application.</p>
     */
    inline CreateRobotApplicationVersionRequest& WithApplication(const char* value) { SetApplication(value); return *this;}


    /**
     * <p>The current revision id for the robot application. If you provide a value and
     * it matches the latest revision ID, a new version will be created.</p>
     */
    inline const Aws::String& GetCurrentRevisionId() const{ return m_currentRevisionId; }

    /**
     * <p>The current revision id for the robot application. If you provide a value and
     * it matches the latest revision ID, a new version will be created.</p>
     */
    inline bool CurrentRevisionIdHasBeenSet() const { return m_currentRevisionIdHasBeenSet; }

    /**
     * <p>The current revision id for the robot application. If you provide a value and
     * it matches the latest revision ID, a new version will be created.</p>
     */
    inline void SetCurrentRevisionId(const Aws::String& value) { m_currentRevisionIdHasBeenSet = true; m_currentRevisionId = value; }

    /**
     * <p>The current revision id for the robot application. If you provide a value and
     * it matches the latest revision ID, a new version will be created.</p>
     */
    inline void SetCurrentRevisionId(Aws::String&& value) { m_currentRevisionIdHasBeenSet = true; m_currentRevisionId = std::move(value); }

    /**
     * <p>The current revision id for the robot application. If you provide a value and
     * it matches the latest revision ID, a new version will be created.</p>
     */
    inline void SetCurrentRevisionId(const char* value) { m_currentRevisionIdHasBeenSet = true; m_currentRevisionId.assign(value); }

    /**
     * <p>The current revision id for the robot application. If you provide a value and
     * it matches the latest revision ID, a new version will be created.</p>
     */
    inline CreateRobotApplicationVersionRequest& WithCurrentRevisionId(const Aws::String& value) { SetCurrentRevisionId(value); return *this;}

    /**
     * <p>The current revision id for the robot application. If you provide a value and
     * it matches the latest revision ID, a new version will be created.</p>
     */
    inline CreateRobotApplicationVersionRequest& WithCurrentRevisionId(Aws::String&& value) { SetCurrentRevisionId(std::move(value)); return *this;}

    /**
     * <p>The current revision id for the robot application. If you provide a value and
     * it matches the latest revision ID, a new version will be created.</p>
     */
    inline CreateRobotApplicationVersionRequest& WithCurrentRevisionId(const char* value) { SetCurrentRevisionId(value); return *this;}

  private:

    Aws::String m_application;
    bool m_applicationHasBeenSet;

    Aws::String m_currentRevisionId;
    bool m_currentRevisionIdHasBeenSet;
  };

} // namespace Model
} // namespace RoboMaker
} // namespace Aws
