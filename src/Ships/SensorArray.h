/*=============================================================================
    Name    : SensorArray.h
    Purpose : Definitions for the Sensor Array

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SENSOR_ARRAY_H
#define ___SENSOR_ARRAY_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    udword dummy;
} SensorArraySpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader SensorArrayHeader;

//void SensorArrayDied(Ship *ship);

#endif //___SENSOR_ARRAY_H


