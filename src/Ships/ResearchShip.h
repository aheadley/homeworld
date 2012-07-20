                                   /*=============================================================================
    Name    : ResearchShip.h
    Purpose : Definitions for Research Ship

    Created 01/06/98 Bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___RESEARCH_SHIP_H
#define ___RESERACH_SHIP_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    sdword seed;
    sdword done;
    udword dockers;
    sdword busy_docking;
    udword pie_plate_num;
    sdword prepshipforanother;
    sdword master;
    sdword rotate_state;
    real32 theta;
    Ship *masterptr;
    Ship *dockwith;
    sdword have_removed_from_parade;
    sdword dockordernumber;
    vector rotate_point;
    real32 rotate_distance;
} ResearchShipSpec;
typedef struct
{
    real32 R1final_dock_distance;
    real32 R1parallel_dock_distance;
    real32 R1VerticalDockDistance;
    real32 max_rotate;
    real32 rotate_acelleration;
    real32 rotate_slow;
    real32 RotationAngle;
    real32 R2DockFinalDistance;
} ResearchShipStatics;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader ResearchShipHeader;


/*-----------------------------------------------------------------------------
    Name        : toFakeOneship
    Description : Used to 'correct' movement circles for a Slaved ship (which should be a master)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
//to OPTIMIZE THIS:  ONLY CALCULATE SLAVED SHIPS AVERAGE POSITION when slaves are added!
void toFakeOneShip(Ship *ship, vector *oldpos, real32 *oldradius);
void toUnFakeOneShip(Ship *ship, vector *oldpos,real32 *oldradius);
void ResearchShipMakeReadyForHyperspace(Ship *ship);
void addMonkeyResearchShip(Ship *ship);
void addMonkeyResearchShipChangePosition(Ship *dockwith, Ship *ship,sdword dockindex);

#endif //___RESEARCH_SHIP_H


