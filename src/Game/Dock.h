/*=============================================================================
    Name    : Dock.h
    Purpose : Definitions for dock.c

    Created 11/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___DOCK_H
#define ___DOCK_H

#include "Types.h"
#include "SpaceObj.h"
#include "ShipSelect.h"
#include "render.h"

#define DOCK_FOR_BOTHREFUELANDREPAIR    1
#define DOCK_FOR_REFUEL                 2
#define DOCK_FOR_REPAIR                 4
#define DOCK_TO_DEPOSIT_RESOURCES       8       // for DockType
#define DOCK_PERMANENTLY                16
#define DOCK_FOR_RESEARCH               32      //special docking for Research Ships
#define DOCK_AT_SPECIFIC_SHIP           64      //dock at a ship specified by user, which is parameter dockwith of clDock
#define DOCK_FOR_REFUELREPAIR           (DOCK_FOR_REFUEL|DOCK_FOR_REPAIR)
#define DOCK_STAY_TILL_EXPLICITLAUNCH   128
#define DOCK_INSTANTANEOUSLY            256
#define DOCK_FOR_RETIRE                 512
typedef udword DockType;

typedef struct
{
    DockType dockType;
    udword formationToGoIntoAfterDockingID;
    bool8 wasHarvesting;
    bool8 allDockingWithSameShip;
    bool8 allNearShip;
    bool8 haveFlyedFarEnoughAway;
} DockCommand;

#if RND_VISUALIZATION
void dockDrawDockInfo(Ship *ship);
#endif

void dockInitializeCustomFunctions(ShipStaticInfo *statinfo,ShipType type,ShipRace race);

sdword dockFindDockIndex(char *name,DockStaticInfo *dockstaticinfo);
/*static void dockReserveDockPoint(Ship *ship,Ship *dockwith,sdword dockpointindex);*/
/*void dockReserveDockPoint(Ship *ship,Ship *dockwith,sdword dockpointindex);*/

void dockPutShipInside(Ship *ship,Ship *dockwith);
void dockInitShipForLaunch(Ship *ship);
void dockRemoveShipFromInside(Ship *ship,Ship *dockwith);
void dockPutShipOutside(Ship *ship,Ship *creator,vector *createat,udword headingdirection,udword updirection);

void RemoveShipFromLaunching(Ship *ship);
void RemoveShipFromDocking(Ship *ship);

void dockPrepareSingleShipForLaunch(Ship *ship,Ship *dockship);
void dockChangeSingleShipToDock(struct CommandToDo *command,Ship *ship,Ship *dock,bool wasHarvesting,DockType dockType);
void DockCleanup(struct CommandToDo *docktodo);
void LaunchCleanup(struct CommandToDo *launchtodo);
bool processDockToDo(struct CommandToDo *docktodo);

bool ShipWithinDockRange(Ship *ship,Ship *target);
Ship *FindNearestShipToDockAt(Ship *ship,DockType dockType);

// specific docking for certain ships
bool LaunchShipFromDDDF(Ship *ship,Ship *dockwith);
bool DroneDocksAtDDDF(struct Ship *ship,struct Ship *dockwith);
void dockPrepareDroneForDocking(Ship *ship,Ship *dockship);

bool ShipIsRefuelingAtCarrierMother(struct Ship *ship);
bool ShipIsWaitingForSoftLaunch(struct Ship *ship);
Ship *FindAnotherResearchShiptoDockWith(Ship *ship);
void dockMakeMaster(Ship *master);
void dockAddSlave(Ship *master, Ship *slave);
void bitClearAllSlaves(Ship *ship);

sdword **GetLaunchPoints(ShipStaticInfo *shipstatic,ShipStaticInfo *dockwithstatic);

DockType dockGetAppropriateTypeOfDocking(SelectCommand *selectcom);

//slaveables...
void dockDealWithDeadSlaveable(Ship *ship);
void dockUpdateSlaves(Ship *master);
void dockCrushMaster(Ship *master);

typedef struct
{
    sdword size;
    real32 dockmisc;
    sdword docktoindex;
    sdword dockingindex;
} AtResearchShipDockInfo;

/*  Defines needed in another file */
#define ASF_WAITLATCHPOINT     1
#define ASF_FLYTOINSIDECONE    2
#define ASF_FLYTOCONEORIGIN    3
#define ASF_REFUEL             4
#define ASF_BACKUP             5
#define ASF_WAIT               6

#endif

