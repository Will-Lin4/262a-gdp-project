﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/gamelift/GameLift_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
namespace Utils
{
namespace Json
{
  class JsonValue;
  class JsonView;
} // namespace Json
} // namespace Utils
namespace GameLift
{
namespace Model
{

  /**
   * <p>Current status of fleet utilization, including the number of game and player
   * sessions being hosted.</p> <ul> <li> <p> <a>CreateFleet</a> </p> </li> <li> <p>
   * <a>ListFleets</a> </p> </li> <li> <p> <a>DeleteFleet</a> </p> </li> <li> <p>
   * <a>DescribeFleetAttributes</a> </p> </li> <li> <p> <a>UpdateFleetAttributes</a>
   * </p> </li> <li> <p> <a>StartFleetActions</a> or <a>StopFleetActions</a> </p>
   * </li> </ul><p><h3>See Also:</h3>   <a
   * href="http://docs.aws.amazon.com/goto/WebAPI/gamelift-2015-10-01/FleetUtilization">AWS
   * API Reference</a></p>
   */
  class AWS_GAMELIFT_API FleetUtilization
  {
  public:
    FleetUtilization();
    FleetUtilization(Aws::Utils::Json::JsonView jsonValue);
    FleetUtilization& operator=(Aws::Utils::Json::JsonView jsonValue);
    Aws::Utils::Json::JsonValue Jsonize() const;


    /**
     * <p>A unique identifier for a fleet.</p>
     */
    inline const Aws::String& GetFleetId() const{ return m_fleetId; }

    /**
     * <p>A unique identifier for a fleet.</p>
     */
    inline bool FleetIdHasBeenSet() const { return m_fleetIdHasBeenSet; }

    /**
     * <p>A unique identifier for a fleet.</p>
     */
    inline void SetFleetId(const Aws::String& value) { m_fleetIdHasBeenSet = true; m_fleetId = value; }

    /**
     * <p>A unique identifier for a fleet.</p>
     */
    inline void SetFleetId(Aws::String&& value) { m_fleetIdHasBeenSet = true; m_fleetId = std::move(value); }

    /**
     * <p>A unique identifier for a fleet.</p>
     */
    inline void SetFleetId(const char* value) { m_fleetIdHasBeenSet = true; m_fleetId.assign(value); }

    /**
     * <p>A unique identifier for a fleet.</p>
     */
    inline FleetUtilization& WithFleetId(const Aws::String& value) { SetFleetId(value); return *this;}

    /**
     * <p>A unique identifier for a fleet.</p>
     */
    inline FleetUtilization& WithFleetId(Aws::String&& value) { SetFleetId(std::move(value)); return *this;}

    /**
     * <p>A unique identifier for a fleet.</p>
     */
    inline FleetUtilization& WithFleetId(const char* value) { SetFleetId(value); return *this;}


    /**
     * <p>Number of server processes in an <code>ACTIVE</code> status currently running
     * across all instances in the fleet</p>
     */
    inline int GetActiveServerProcessCount() const{ return m_activeServerProcessCount; }

    /**
     * <p>Number of server processes in an <code>ACTIVE</code> status currently running
     * across all instances in the fleet</p>
     */
    inline bool ActiveServerProcessCountHasBeenSet() const { return m_activeServerProcessCountHasBeenSet; }

    /**
     * <p>Number of server processes in an <code>ACTIVE</code> status currently running
     * across all instances in the fleet</p>
     */
    inline void SetActiveServerProcessCount(int value) { m_activeServerProcessCountHasBeenSet = true; m_activeServerProcessCount = value; }

    /**
     * <p>Number of server processes in an <code>ACTIVE</code> status currently running
     * across all instances in the fleet</p>
     */
    inline FleetUtilization& WithActiveServerProcessCount(int value) { SetActiveServerProcessCount(value); return *this;}


    /**
     * <p>Number of active game sessions currently being hosted on all instances in the
     * fleet.</p>
     */
    inline int GetActiveGameSessionCount() const{ return m_activeGameSessionCount; }

    /**
     * <p>Number of active game sessions currently being hosted on all instances in the
     * fleet.</p>
     */
    inline bool ActiveGameSessionCountHasBeenSet() const { return m_activeGameSessionCountHasBeenSet; }

    /**
     * <p>Number of active game sessions currently being hosted on all instances in the
     * fleet.</p>
     */
    inline void SetActiveGameSessionCount(int value) { m_activeGameSessionCountHasBeenSet = true; m_activeGameSessionCount = value; }

    /**
     * <p>Number of active game sessions currently being hosted on all instances in the
     * fleet.</p>
     */
    inline FleetUtilization& WithActiveGameSessionCount(int value) { SetActiveGameSessionCount(value); return *this;}


    /**
     * <p>Number of active player sessions currently being hosted on all instances in
     * the fleet.</p>
     */
    inline int GetCurrentPlayerSessionCount() const{ return m_currentPlayerSessionCount; }

    /**
     * <p>Number of active player sessions currently being hosted on all instances in
     * the fleet.</p>
     */
    inline bool CurrentPlayerSessionCountHasBeenSet() const { return m_currentPlayerSessionCountHasBeenSet; }

    /**
     * <p>Number of active player sessions currently being hosted on all instances in
     * the fleet.</p>
     */
    inline void SetCurrentPlayerSessionCount(int value) { m_currentPlayerSessionCountHasBeenSet = true; m_currentPlayerSessionCount = value; }

    /**
     * <p>Number of active player sessions currently being hosted on all instances in
     * the fleet.</p>
     */
    inline FleetUtilization& WithCurrentPlayerSessionCount(int value) { SetCurrentPlayerSessionCount(value); return *this;}


    /**
     * <p>The maximum number of players allowed across all game sessions currently
     * being hosted on all instances in the fleet.</p>
     */
    inline int GetMaximumPlayerSessionCount() const{ return m_maximumPlayerSessionCount; }

    /**
     * <p>The maximum number of players allowed across all game sessions currently
     * being hosted on all instances in the fleet.</p>
     */
    inline bool MaximumPlayerSessionCountHasBeenSet() const { return m_maximumPlayerSessionCountHasBeenSet; }

    /**
     * <p>The maximum number of players allowed across all game sessions currently
     * being hosted on all instances in the fleet.</p>
     */
    inline void SetMaximumPlayerSessionCount(int value) { m_maximumPlayerSessionCountHasBeenSet = true; m_maximumPlayerSessionCount = value; }

    /**
     * <p>The maximum number of players allowed across all game sessions currently
     * being hosted on all instances in the fleet.</p>
     */
    inline FleetUtilization& WithMaximumPlayerSessionCount(int value) { SetMaximumPlayerSessionCount(value); return *this;}

  private:

    Aws::String m_fleetId;
    bool m_fleetIdHasBeenSet;

    int m_activeServerProcessCount;
    bool m_activeServerProcessCountHasBeenSet;

    int m_activeGameSessionCount;
    bool m_activeGameSessionCountHasBeenSet;

    int m_currentPlayerSessionCount;
    bool m_currentPlayerSessionCountHasBeenSet;

    int m_maximumPlayerSessionCount;
    bool m_maximumPlayerSessionCountHasBeenSet;
  };

} // namespace Model
} // namespace GameLift
} // namespace Aws
