/*=============================================================================
    Name    : Mothership.h
    Purpose : Definitions for Mothership

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MOTHERSHIP_H
#define ___MOTHERSHIP_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    sdword doorFlags;
    SpaceObjRotImpTargGuidanceShipDerelict *doorCargo;
    SpaceObjRotImpTargGuidanceShipDerelict *droptarget[MAX_NUM_DROP];
    sdword dropstate[MAX_NUM_DROP];
    sdword dockindex[MAX_NUM_DROP];
    sdword CAP_NumInBuildCue;
    sdword CAPshiptype[SALVAGE_MAX_CAPTURABLE];
    sdword CAPshiprace[SALVAGE_MAX_CAPTURABLE];
    real32 CAPTimeToBuildShip[SALVAGE_MAX_CAPTURABLE];
    ubyte CAPcolorScheme[SALVAGE_MAX_CAPTURABLE];
    ubyte freedocktimer[3];
} MothershipSpec;

typedef struct
{
    vector specialDoorOffset[NUM_RACES][TOTAL_NUM_SHIPS];
    sdword specialDoorOrientationHeading[NUM_RACES][TOTAL_NUM_SHIPS];
    sdword specialDoorOrientationUp[NUM_RACES][TOTAL_NUM_SHIPS];
    real32 specialDoorInterpolationPerSecond;
} MothershipStatics;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader MothershipHeader;


// Public Prototypes
void MothershipAttachObjectToDoor(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *object);
void MothershipDettachObjectFromDoor(Ship *ship);
void MothershipDoorUpKeep(Ship *ship);
void mothershipGetCargoPosition(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *cargo, vector *position, matrix *coordsys, vector *heading, vector *up,vector *right);
void mothershipCleanDoorForHSInstant(Ship *ship);
#endif //___MOTHERSHIP_H

