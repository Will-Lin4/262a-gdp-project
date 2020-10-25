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
  class AWS_ROBOMAKER_API DescribeRobotRequest : public RoboMakerRequest
  {
  public:
    DescribeRobotRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "DescribeRobot"; }

    Aws::String SerializePayload() const override;


    /**
     * <p>The Amazon Resource Name (ARN) of the robot to be described.</p>
     */
    inline const Aws::String& GetRobot() const{ return m_robot; }

    /**
     * <p>The Amazon Resource Name (ARN) of the robot to be described.</p>
     */
    inline bool RobotHasBeenSet() const { return m_robotHasBeenSet; }

    /**
     * <p>The Amazon Resource Name (ARN) of the robot to be described.</p>
     */
    inline void SetRobot(const Aws::String& value) { m_robotHasBeenSet = true; m_robot = value; }

    /**
     * <p>The Amazon Resource Name (ARN) of the robot to be described.</p>
     */
    inline void SetRobot(Aws::String&& value) { m_robotHasBeenSet = true; m_robot = std::move(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the robot to be described.</p>
     */
    inline void SetRobot(const char* value) { m_robotHasBeenSet = true; m_robot.assign(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the robot to be described.</p>
     */
    inline DescribeRobotRequest& WithRobot(const Aws::String& value) { SetRobot(value); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the robot to be described.</p>
     */
    inline DescribeRobotRequest& WithRobot(Aws::String&& value) { SetRobot(std::move(value)); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the robot to be described.</p>
     */
    inline DescribeRobotRequest& WithRobot(const char* value) { SetRobot(value); return *this;}

  private:

    Aws::String m_robot;
    bool m_robotHasBeenSet;
  };

} // namespace Model
} // namespace RoboMaker
} // namespace Aws
