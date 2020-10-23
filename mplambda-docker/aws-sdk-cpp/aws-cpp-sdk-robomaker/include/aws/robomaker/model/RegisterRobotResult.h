﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/robomaker/RoboMaker_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
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
namespace RoboMaker
{
namespace Model
{
  class AWS_ROBOMAKER_API RegisterRobotResult
  {
  public:
    RegisterRobotResult();
    RegisterRobotResult(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);
    RegisterRobotResult& operator=(const Aws::AmazonWebServiceResult<Aws::Utils::Json::JsonValue>& result);


    /**
     * <p>The Amazon Resource Name (ARN) of the fleet that the robot will join.</p>
     */
    inline const Aws::String& GetFleet() const{ return m_fleet; }

    /**
     * <p>The Amazon Resource Name (ARN) of the fleet that the robot will join.</p>
     */
    inline void SetFleet(const Aws::String& value) { m_fleet = value; }

    /**
     * <p>The Amazon Resource Name (ARN) of the fleet that the robot will join.</p>
     */
    inline void SetFleet(Aws::String&& value) { m_fleet = std::move(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the fleet that the robot will join.</p>
     */
    inline void SetFleet(const char* value) { m_fleet.assign(value); }

    /**
     * <p>The Amazon Resource Name (ARN) of the fleet that the robot will join.</p>
     */
    inline RegisterRobotResult& WithFleet(const Aws::String& value) { SetFleet(value); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the fleet that the robot will join.</p>
     */
    inline RegisterRobotResult& WithFleet(Aws::String&& value) { SetFleet(std::move(value)); return *this;}

    /**
     * <p>The Amazon Resource Name (ARN) of the fleet that the robot will join.</p>
     */
    inline RegisterRobotResult& WithFleet(const char* value) { SetFleet(value); return *this;}


    /**
     * <p>Information about the robot registration.</p>
     */
    inline const Aws::String& GetRobot() const{ return m_robot; }

    /**
     * <p>Information about the robot registration.</p>
     */
    inline void SetRobot(const Aws::String& value) { m_robot = value; }

    /**
     * <p>Information about the robot registration.</p>
     */
    inline void SetRobot(Aws::String&& value) { m_robot = std::move(value); }

    /**
     * <p>Information about the robot registration.</p>
     */
    inline void SetRobot(const char* value) { m_robot.assign(value); }

    /**
     * <p>Information about the robot registration.</p>
     */
    inline RegisterRobotResult& WithRobot(const Aws::String& value) { SetRobot(value); return *this;}

    /**
     * <p>Information about the robot registration.</p>
     */
    inline RegisterRobotResult& WithRobot(Aws::String&& value) { SetRobot(std::move(value)); return *this;}

    /**
     * <p>Information about the robot registration.</p>
     */
    inline RegisterRobotResult& WithRobot(const char* value) { SetRobot(value); return *this;}

  private:

    Aws::String m_fleet;

    Aws::String m_robot;
  };

} // namespace Model
} // namespace RoboMaker
} // namespace Aws
