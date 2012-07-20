/*=============================================================================
    Name    : ProximitySensor.h
    Purpose : Definitions for the ProximitySensor

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PROXIMITYSENSOR_H
#define ___PROXIMITYSENSOR_H

#include "Types.h"
#include "SpaceObj.h"
#include "Select.h"
#ifndef STATVIEWER_PROGRAM
#include "Universe.h"
#endif


/*=============================================================================
    Types:
=============================================================================*/
typedef struct
{
    udword SearchRate;
    udword SearchRateAfterFound;
    real32 SearchRadius;
    real32 SearchRadiusSqr;
    sdword SensorCircleRadius;
    real32 SensorBlinkRate;
    real32 TriggerSpeed;
    real32 TriggerSpeedSqr;
    udword blipColor;
    sdword blipThickness;
} ProximitySensorStatics;

typedef struct
{
    udword sensorState;
    sdword blipState;
    real32 blipRadius;
    sdword TAGGED;
} ProximitySensorSpec;

/*=============================================================================
    Public data:
=============================================================================*/

//sensor states
#define SENSOR_BEGIN    0
#define SENSOR_SENSE    1
#define SENSOR_SENSED   2
#define SENSOR_SENSED2  3
extern color ProximitySensorBlipColor;

/*=============================================================================
    Functions:
=============================================================================*/

extern CustShipHeader ProximitySensorHeader;

sdword proxGetBlipRadius(Ship *ship);
void proxInFocusSelection(SelectCommand *focus);
bool proxShouldDrawOverlay(Ship *ship);
udword proxBlipColor(Ship *ship);
sdword proxBlipThickness(Ship *ship);
#ifndef STATVIEWER_PROGRAM
bool proximityCanPlayerSeeShip(Player *player,Ship *shipInQuestion);
#endif
#endif //___PROXIMITYSENSOR_H

