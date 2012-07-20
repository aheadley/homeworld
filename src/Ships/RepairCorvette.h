/*=============================================================================
    Name    : RepairCorvette.h
    Purpose : Definitions for RepairCorvette

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___REPAIR_CORVETTE_H
#define ___REPAIR_CORVETTE_H

#include "Types.h"
#include "SpaceObj.h"
#include "Attack.h"
#include "ShipSelect.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    AttackSideStep attacksidestep;
    sdword repairState;
    struct Ship *target;
    sdword hyst;
    real32 disengagetime;
} RepairCorvetteSpec;

typedef struct
{
    AttackSideStepParameters sidestepParameters;
    real32 repairApproachDistance;

    real32 approachAndWaitDistance;
    real32 rotationStopDistance;
    real32 stopRotMultiplier;
    real32 sloppyRotThreshold;
    real32 dockWithRotationSpeed;
    real32 targetStartDockDistance;
    real32 startdockTolerance;
    real32 finaldockDistance;
    real32 CapitalDistanceRepairStart;
    real32 capitalShipHealthPerSecond;
    real32 AngleDotProdThreshold;
} RepairCorvetteStatics;

/*=============================================================================
    Public data:
=============================================================================*/

//repair state variables
#define REPAIR_Begin        0
#define REPAIR_Approach     1
#define REPAIR_Nearing      2
#define REPAIR_StopRot      3
#define REPAIR_Dock1        4
#define REPAIR_Dock2        5
#define REPAIR_Repair       6
#define REPAIR_Disengage1   7
#define REPAIR_Disengage2   8
#define REPAIR_Done         9
#define REPAIR_Hackwork     10

extern CustShipHeader RepairCorvetteHeader;

void RepairCorvetteOrderChanged(Ship *ship);
void stopRepairEffect(Ship *ship);

//general repair stuff
bool refuelRepairShips(Ship *ship, SelectAnyCommand *targets,real32 rangetoRefuel);


#endif //___REPAIR_CORVETTE_H

