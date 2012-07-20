/*=============================================================================
    Name    : Drone.h
    Purpose : Definitions for Drone

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___DRONE_H
#define ___DRONE_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

// for droneState
#define DRONESTATE_DORMANT      0
#define DRONESTATE_LAUNCHING    1
#define DRONESTATE_LAUNCHED     2
#define DRONESTATE_DOCKING      3

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    udword droneNumber;
    udword droneState;
} DroneSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader DroneHeader;

#endif //___DRONE_H

