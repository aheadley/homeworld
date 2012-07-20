/*=============================================================================
    Name    : GravWellGenerator.h
    Purpose : Definitions for the Gravity Well Generator

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___GRAV_WELL_GENERATOR_H
#define ___GRAV_WELL_GENERATOR_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Types:
=============================================================================*/
typedef struct
{
    real32 GravWellRadius;
    real32 GravWellRadiusSqr;
    real32 OperationTime;
    real32 EffectConstant;
    udword scanrate;
    real32 repulseForce;
    real32 warmupdowntime;
    real32 xrot;
    real32 yrot;
    real32 zrot;
} GravWellGeneratorStatics;

typedef struct
{
    Node objnode;
    Ship *ship;
    sdword stoppingstate;
    real32 xangle;
    real32 yangle;
    real32 zangle;
} GravStruct;

typedef struct
{
    sdword GravFieldOn;
    sdword GravFired;
    LinkedList  GravList;
    real32 TimeOn;
    real32 powertime;
    sdword ready;
    sdword GravDerelict;
    Effect *gravityEffect;
} GravWellGeneratorSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader GravWellGeneratorHeader;
/*=============================================================================
    Prototypes:
=============================================================================*/
//void GravWellGeneratorRemoveShipReferences(Ship *ship, SpaceObj *spaceobj);
//void GravWellGeneratorDied(Ship *ship);

void GravWellGeneratorJustDisabled(Ship *ship);
bool gravwellIsShipStuckForHyperspaceing(Ship *ship);
void turnoffGravwell(Ship *ship);


#endif //___GRAV_WELL_GENERATOR_H


