/*=============================================================================
    Name    : formation.c
    Purpose : Implementation of formations

    Created 10/20/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "FastMath.h"
#include "SpaceObj.h"
#include "AIShip.h"
#include "AITrack.h"
#include "ShipSelect.h"
#include "Formation.h"
#include "CommandLayer.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "StatScript.h"
#include "ProximitySensor.h"
#include "DDDFrigate.h"
#include "Tactics.h"
#include "Strings.h"
#include "SalCapCorvette.h"
#include "SinglePlayer.h"

//real32 DELTA_FORMATION_PADDING = 50.0f;
//real32 BROAD_FORMATION_PADDING = 50.0f;
real32 DELTA_FORMATION_PADDING_SCALE = 1.5f;
real32 BROAD_FORMATION_PADDING_SCALE = 1.5f;
real32 CLAW_FORMATION_PADDING_SCALE = 1.5f;
real32 DELTA3D_FORMATION_PADDING_SCALE = 1.5f;
real32 WALL_FORMATION_PADDING_SCALE = 2.0f;
real32 WALL_FORMATION_PADDING = 20.0f;
real32 SPHERE_FORMATION_PADDING_SCALE = 4.0f;
real32 CLAW_FORMATION_CURVATURE = 1.1f;
real32 SPHERE_MINIMUM_RADIUS = 500.0f;
real32 SPHERE_MAXIMUM_RADIUS = 3500.0f;
real32 SPHERESIZE_IGNORE_NUMDECLINATIONS = 600.0f;

real32 STRIKECRAFT_PADDING_MODIFIER = 1.2f;

real32 FORMATION_TRAVELVEL_MAXSCALE = 1.0f;
real32 FORMATION_TRAVELVEL_MINSCALE = 0.7f;
real32 FORMATION_ERROR_BIG = 10000.0f;
real32 FORMATION_ERROR_NEGLIGIBLE = 100.0f;

real32 PARADE_CONSIDERED_STEADY_VELSQR = 2500.0f;

bool8 FORMATION_TRAVEL_SLOWEST_SHIP = TRUE;

real32 TARGETDRONE_FORMATION_SIZE = 1000.0f;

static NumStrXlate formationtypeinfo[] =
{
    { (uword)DELTA_FORMATION,  str$(DELTA_FORMATION) },
    { (uword)BROAD_FORMATION, str$(BROAD_FORMATION) },
    { (uword)DELTA3D_FORMATION, str$(DELTA3D_FORMATION) },
    { (uword)CLAW_FORMATION, str$(CLAW_FORMATION) },
    { (uword)WALL_FORMATION, str$(WALL_FORMATION) },
    { (uword)SPHERE_FORMATION, str$(SPHERE_FORMATION) },
    { (uword)CUSTOM_FORMATION, str$(CUSTOM_FORMATION) },
    { (uword)NO_FORMATION, str$(NO_FORMATION) },
    { (uword)SAME_FORMATION, str$(SAME_FORMATION) },
    { 0,NULL }
};

static NumStrXlate formationtypeniceinfo[] =
{
    { (uword)DELTA_FORMATION,  "Delta Wing" },
    { (uword)BROAD_FORMATION, "Broad Attack" },
    { (uword)DELTA3D_FORMATION, "Formation X" },
    { (uword)CLAW_FORMATION, "The Claw" },
    { (uword)WALL_FORMATION, "The Wall" },
    { (uword)SPHERE_FORMATION, "Sphere" },
    { (uword)CUSTOM_FORMATION, "Custom" },
    { 0,NULL }
};

scriptEntry FormationTweaks[] =
{
    makeEntry(FORMATION_TRAVEL_SLOWEST_SHIP,scriptSetBool8),
    makeEntry(DELTA_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(BROAD_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(DELTA3D_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(CLAW_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(CLAW_FORMATION_CURVATURE,scriptSetReal32CB),
    makeEntry(WALL_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(WALL_FORMATION_PADDING,scriptSetReal32CB),
    makeEntry(SPHERE_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(SPHERE_MINIMUM_RADIUS,scriptSetReal32CB),
    makeEntry(SPHERE_MAXIMUM_RADIUS,scriptSetReal32CB),
    makeEntry(STRIKECRAFT_PADDING_MODIFIER,scriptSetReal32CB),
    makeEntry(FORMATION_ERROR_CALCULATE_RATE,scriptSetSdwordCB),
    makeEntry(FORMATION_TRAVELVEL_MAXSCALE,scriptSetReal32CB),
    makeEntry(FORMATION_TRAVELVEL_MINSCALE,scriptSetReal32CB),
    makeEntry(FORMATION_ERROR_BIG,scriptSetReal32CB),
    makeEntry(FORMATION_ERROR_NEGLIGIBLE,scriptSetReal32CB),
    makeEntry(TARGETDRONE_FORMATION_SIZE,scriptSetReal32CB),
    makeEntry(SPHERESIZE_IGNORE_NUMDECLINATIONS,scriptSetReal32CB),
    endEntry
};

udword formationSortType[NO_FORMATION] =
{
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //DELTA_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //BROAD_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //DELTA3D_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //CLAW_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //WALL_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,       //SPHERE_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST       //SPHERE_FORMATION


};

//prototypes!
void formationArrageCrazyOptimum(CommandToDo *formationcommand);

bool isCapitalShipStaticOrBig(ShipStaticInfo *shipstatic)
{
    if (isCapitalShipStatic(shipstatic))
    {
        return TRUE;
    }

    switch (shipstatic->shiptype)   // other cases that are not necessarily covered by isCapitalShipStatic
    {
        case SensorArray:
        case CloakGenerator:
        case GravWellGenerator:
        case ResourceController:
        case ResearchShip:
            return TRUE;
        default:
            return FALSE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : TypeOfFormationToStr
    Description : converts TypeOfFormation to its string equivalent
    Inputs      : formation
    Outputs     :
    Return      : string representing formation
----------------------------------------------------------------------------*/
char *TypeOfFormationToStr(TypeOfFormation formation)
{
    return NumToStr(formationtypeinfo,(uword)formation);
}

/*-----------------------------------------------------------------------------
    Name        : StrToTypeOfFormation
    Description : converts from string to TypeOfFormation
    Inputs      : str
    Outputs     :
    Return      : returns TypeOfFormation enumeration based on string
----------------------------------------------------------------------------*/
TypeOfFormation NiceStrToTypeOfFormation(char *str)
{
    return StrToNum(formationtypeniceinfo,str);
}

/*-----------------------------------------------------------------------------
    Name        : TypeOfFormationToStr
    Description : converts TypeOfFormation to its string equivalent
    Inputs      : formation
    Outputs     :
    Return      : string representing formation
----------------------------------------------------------------------------*/
char *TypeOfFormationToNiceStr(TypeOfFormation formation)
{
    if (formation == -1)
        return strGetString(strPARADE_FORMATION);
    else
        return (strGetString(formation+strFormationOffset));
}

/*-----------------------------------------------------------------------------
    Name        : StrToTypeOfFormation
    Description : converts from string to TypeOfFormation
    Inputs      : str
    Outputs     :
    Return      : returns TypeOfFormation enumeration based on string
----------------------------------------------------------------------------*/
TypeOfFormation StrToTypeOfFormation(char *str)
{
    return StrToNum(formationtypeinfo,str);
}

typedef struct
{
    real32 distance;
    ShipPtr ShipPtr;
} ShipDistInfo;

int compareSizeThenDist(const void *arg1,const void *arg2)
{
    ShipDistInfo *a1 = (ShipDistInfo *)arg1;
    ShipDistInfo *a2 = (ShipDistInfo *)arg2;
    real32 size1 = a1->ShipPtr->staticinfo->staticheader.staticCollInfo.collspheresize;
    real32 size2 = a2->ShipPtr->staticinfo->staticheader.staticCollInfo.collspheresize;

    if (size1 == size2)
    {
        if (a1->distance < a2->distance)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        if (size1 > size2)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
}

void formationBiggestThenClosestOptimum(SelectCommand *selection,TypeOfFormation formationtype)
{
    sdword numShips = selection->numShips;
    vector avgpos = { 0.0f,0.0f,0.0f };

    real32 contender;
    real32 biggestship = 0.0f;
    real32 smallestship = 1000000.0f;
    sdword biggestshipindex = -1;
    real32 temp;

    ShipDistInfo *shipdist;
    Ship *tempship;
    sdword i,cloakinsel,cloakindex;
    vector diff;

    for (i=0;i<numShips;i++)
    {
        contender = (selection->ShipPtr[i])->staticinfo->staticheader.staticCollInfo.collspheresize;
        if (contender > biggestship)
        {
            biggestship = contender;
            biggestshipindex = i;
        }
        if (contender < smallestship)
        {
            smallestship = contender;
        }
    }

    dbgAssert(biggestshipindex >= 0);

    if (smallestship == biggestship)        // all ships are the same size
    {
        for (i=0;i<numShips;i++)            // so find average position of all ships
        {
            vecAddTo(avgpos,(selection->ShipPtr[i])->posinfo.position);
        }
        vecDivideByScalar(avgpos,(real32)numShips,temp);
    }
    else                                    // ships are not same size
    {
        // so let average position be position of biggest ship
        avgpos = (selection->ShipPtr[biggestshipindex])->posinfo.position;
    }

    shipdist = memAlloc(sizeof(ShipDistInfo)*selection->numShips,"TmpShipDistArray",0);

    for (i=0;i<selection->numShips;i++)
    {
        vecSub(diff,(selection->ShipPtr[i])->posinfo.position,avgpos);
        shipdist[i].distance = vecMagnitudeSquared(diff);
        shipdist[i].ShipPtr = selection->ShipPtr[i];
    }

    qsort(shipdist,selection->numShips,sizeof(ShipDistInfo),compareSizeThenDist);

    for (i=0;i<selection->numShips;i++)
    {
        selection->ShipPtr[i] = shipdist[i].ShipPtr;
    }

    cloakinsel = FALSE;
    for (i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[i]->shiptype == CloakGenerator)
        {
            cloakinsel = TRUE;
            cloakindex = i;
        }
    }

    if(cloakinsel)
    {
        //go through selection, and move any cloak generator to the
        //leader position, that way we cover other ships with cloaking
        //field more effectivly
        tempship = selection->ShipPtr[0];
        selection->ShipPtr[0] = selection->ShipPtr[cloakindex];
        selection->ShipPtr[cloakindex] = tempship;
    }

    memFree(shipdist);

}

void formationArrangeOptimum(struct CommandToDo *formationtodo)
{
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;

    dbgAssert(numShips >= ABSOLUTE_MIN_SHIPS_IN_FORMATION);

    if (selection->numShips == 1)
    {
        return;     // only 1 item, so don't sort
    }

    switch (formationSortType[formationtodo->formation.formationtype])
    {
        case FORMATION_SORT_BIGGEST_THEN_CLOSEST:
            formationBiggestThenClosestOptimum(selection,formationtodo->formation.formationtype);
            formationtodo->formation.sortorder = FORMATION_SORT_BIGGEST_THEN_CLOSEST;
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

#define NUM_SPHERE_TABLE_ENTRYS     50

struct SphereStaticInfo *createSphereStaticInfo(void)
{
    sdword sizeofspherestaticinfo;
    SphereStaticInfo *sphereStaticInfo;
    sdword tableindex;
    SphereTableEntry *tableentry;
    sdword sizeofSphereTableEntry;
    sdword numDeclinations;
    sdword numCircleDivisions;
    real32 angleDivision;
    real32 angle;
    real32 rotang;
    sdword totalships;
    sdword numships;
    sdword i;
    SphereDeclination *sphereDeclination;

    sizeofspherestaticinfo = sizeofSphereStaticInfo(NUM_SPHERE_TABLE_ENTRYS);

    sphereStaticInfo = memAlloc(sizeofspherestaticinfo,"spherestatinfo",NonVolatile);
    memset(sphereStaticInfo,0,sizeofspherestaticinfo);

    sphereStaticInfo->numTableEntries = NUM_SPHERE_TABLE_ENTRYS;

    for (tableindex=0;tableindex<NUM_SPHERE_TABLE_ENTRYS;tableindex++)
    {
        numDeclinations = tableindex+1;
        numCircleDivisions = numDeclinations + numDeclinations + 2;
        angleDivision = TWOPI / ((real32)numCircleDivisions);

        sizeofSphereTableEntry = sizeofSphereTableEntry(numDeclinations);
        tableentry = memAlloc(sizeofSphereTableEntry,"spheretabentry",NonVolatile);
        memset(tableentry,0,sizeofSphereTableEntry);

        sphereStaticInfo->sphereTableEntryPtrs[tableindex] = tableentry;

        totalships = 0;
        tableentry->numDeclinations = numDeclinations;
        angle = PI / 2.0f;     // 90 degrees

        for (i=0,sphereDeclination=&tableentry->sphereDeclinations[0];i<numDeclinations;i++,sphereDeclination++)
        {
            angle -= angleDivision;
            numships = (sdword) (cos(angle) * (real32)numCircleDivisions);
            totalships += numships;

            sphereDeclination->cosdeclination = (real32)cos(angle);
            sphereDeclination->sindeclination = (real32)sin(angle);
            sphereDeclination->numAnglePoints = numships;
            rotang = TWOPI / (real32)numships;
            sphereDeclination->cosrotang = (real32)cos(rotang);
            sphereDeclination->sinrotang = (real32)sin(rotang);
        }

        tableentry->numShipsCanHandle = totalships + 3;   // leader goes in center, two ships at poles (+3)

    }

    return sphereStaticInfo;
}

/*-----------------------------------------------------------------------------
    Name        : freeSphereStaticInfo
    Description : frees sphereStaticInfo
    Inputs      : sphereStaticInfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void freeSphereStaticInfo(struct SphereStaticInfo *sphereStaticInfo)
{
    sdword numTableEntrys = sphereStaticInfo->numTableEntries;
    sdword i;

    for (i=0;i<numTableEntrys;i++)
    {
        memFree(sphereStaticInfo->sphereTableEntryPtrs[i]);
    }

    memFree(sphereStaticInfo);
}

SphereTableEntry *findSphereTableEntry(sdword numShips)
{
    sdword tableindex;
    sdword numTables = sphereStaticInfo->numTableEntries;
    SphereTableEntry *tableentry;

    for (tableindex=0;tableindex<numTables;tableindex++)
    {
        tableentry = sphereStaticInfo->sphereTableEntryPtrs[tableindex];

        if (numShips <= tableentry->numShipsCanHandle)
        {
            return tableentry;
        }
    }
    
    dbgFatal(DBG_Loc,"\nSphere formation too big");
    
    // never going to get here because of the dbgFatal above; this is here just to keep the compiler happy
    return tableentry;
}

/*-----------------------------------------------------------------------------
    Name        : sphere_special_proximity_solution(selection);
    Description : retruns true if we can creativly change the sphere formation for proximity sensors
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

bool sphere_special_proximity_solution(Ship *centreship, SelectCommand *selection)
{
   sdword i;
   for(i=1;i<selection->numShips;i++)
   {
      if(selection->ShipPtr[i]->shiptype != ProximitySensor)
      {
         //non-proximity sensor detected, return false
         return(FALSE);
      }
   }
   //Entire selection is proximity sensors, except maybe centreship

   if(centreship == selection->ShipPtr[0])
      return(TRUE);                          //not protecting anything
   else if(selection->ShipPtr[0]->shiptype == ProximitySensor)
      return(TRUE);                          //all proximity sensors

   return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : formationSpecificsSphere
    Description : calculates and returns custom information for the specifics of a sphere formation command
    Inputs      : command
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void formationSpecificsSphere(CommandToDo *command)
{
    SelectCommand *selection = command->selection;
    sdword numShips = selection->numShips;
    sdword shipWorkingOn;
    real32 sphereRadius;
    real32 shipRadius,minrange;
    sdword i,j,k;
    sdword numDeclinations;
//    sdword totalDeclinations;
    SphereDeclination *sphereDeclination;
    vector declination;
    vector tempvec;
    matrix rotaboutz;
    Ship *centreship;
    sdword numShipsIncludingCentre;
    Ship *ship;

    SphereTableEntry *tableentry;
    sdword prox_sol;
    dbgAssert(numShips > 0);

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        // We should form around protected ship
//        command->formation.formAroundProtectedShip = TRUE;
        dbgAssert(command->protect->numShips > 0);
        centreship = command->protect->ShipPtr[0];
        shipWorkingOn = 0;
        numShipsIncludingCentre = numShips + 1;
    }
    else if(command->ordertype.order == COMMAND_ATTACK && command->attack->numTargets)
    {
        if(command->formation.enders == TRUE)
        {
            //centreship = command->formation.endership;

            //if this case never happens, we can remove the check..but the check is
            if(command->selection->ShipPtr[0]->attackvars.attacktarget == NULL)
            {
                //don't know if this will happen ever...we'll find out
                //
                //
                //  Tell Bryce about this!
                //
                //
#ifdef DEBUG_TACTICS
    #ifdef bpasechn
                _asm int 3
    #endif
#endif
                centreship = (Ship*)command->attack->TargetPtr[0];      //this can't possibly be invalid!
            }
            else
            {
                centreship = (Ship *)command->selection->ShipPtr[0]->attackvars.attacktarget;
            }
            shipWorkingOn = 0;
            numShipsIncludingCentre = numShips + 1;
        }
        else
        {
            centreship = selection->ShipPtr[0];
            shipWorkingOn = 1;
            numShipsIncludingCentre = numShips;
        }
    }
    else
    {
        centreship = selection->ShipPtr[0];
        shipWorkingOn = 1;
        numShipsIncludingCentre = numShips;
    }

    prox_sol = sphere_special_proximity_solution(centreship, selection);

    if (numShipsIncludingCentre == 1)
    {
        return;             // if only leader is present, don't need to calculate anything
    }

    // calculate sphereRadius

    if ((prox_sol) && (centreship->shiptype == ProximitySensor))
    {
        shipRadius = ((ProximitySensorStatics *) ((ShipStaticInfo *)(centreship->staticinfo))->custstatinfo)->SearchRadius;
    }
    else
    {
        shipRadius = centreship->staticinfo->staticheader.staticCollInfo.collspheresize;

    }
//    totalDeclinations = numDeclinations + numDeclinations + 2;

    tableentry = findSphereTableEntry(numShipsIncludingCentre);
    numDeclinations = tableentry->numDeclinations;

    if ((centreship->shiptype == DDDFrigate) && (selection->ShipPtr[0]->shiptype == Drone))
    {   //if centre ship is DDDF and the 1st ship is a drone, the assumption is all are drones
        sphereRadius = ((DDDFrigateStatics *) ((ShipStaticInfo *)(centreship->staticinfo))->custstatinfo)->droneDeploymentRange;
    }
    else if(prox_sol)
    {
        sphereRadius = shipRadius + ((ProximitySensorStatics *) ((ShipStaticInfo *)(selection->ShipPtr[1]->staticinfo))->custstatinfo)->SearchRadius;
    }
    else if (singlePlayerGame && singlePlayerGameInfo.currentMission == 14 && /*numShips > 20*/(selection->ShipPtr[0]->playerowner->playerIndex) && selection->ShipPtr[0]->shiptype == IonCannonFrigate)
    {                                                                         //^^^^^^^^^^^^^  special case changed to be more general
        sphereRadius = SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE;
    }
    else if (command->formation.enders == TRUE)
    {
        minrange = REALlyBig;
        for(k=1;k<numShips;k++)
        {
            if(command->selection->ShipPtr[k]->staticinfo->bulletRange[command->selection->ShipPtr[k]->tacticstype] < minrange)
                minrange = command->selection->ShipPtr[k]->staticinfo->bulletRange[command->selection->ShipPtr[k]->tacticstype];
        }
        sphereRadius = minrange*0.9f;
    }
    else
    {
        if (shipRadius > SPHERESIZE_IGNORE_NUMDECLINATIONS)
            sphereRadius = shipRadius * SPHERE_FORMATION_PADDING_SCALE * 2;
        else
            sphereRadius = shipRadius * SPHERE_FORMATION_PADDING_SCALE * (real32)((numDeclinations < 2) ? 2 : numDeclinations);
    }

    if (sphereRadius < SPHERE_MINIMUM_RADIUS)
    {
        sphereRadius = SPHERE_MINIMUM_RADIUS;
    }
    else if ((sphereRadius > SPHERE_MAXIMUM_RADIUS) && (sphereRadius != SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE))
    {
        sphereRadius = SPHERE_MAXIMUM_RADIUS;
    }

    // position ship at top pole
    ship = selection->ShipPtr[shipWorkingOn];

    vecSet(ship->formationOffset,0.0f,0.0f,sphereRadius);
    vecSet(ship->formationHeading,0.0f,0.0f,1.0f);

    shipWorkingOn++;
    if (numShips == shipWorkingOn)
    {
        return;
    }
    ship = selection->ShipPtr[shipWorkingOn];

    // position ship at bottom pole
    vecSet(ship->formationOffset,0.0f,0.0f,-sphereRadius);
    vecSet(ship->formationHeading,0.0f,0.0f,-1.0f);

    shipWorkingOn++;
    if (numShips == shipWorkingOn)
    {
        return;
    }
    ship = selection->ShipPtr[shipWorkingOn];

    // now position rest of ships.  Look up the table we need for the number of ships we have

    dbgAssert(shipWorkingOn < numShips);
    dbgAssert(numShips <= tableentry->numShipsCanHandle);

    for (i=0,sphereDeclination=&tableentry->sphereDeclinations[0];i<numDeclinations;i++,sphereDeclination++)
    {
        // get the declination vector

        vecSet(declination,sphereDeclination->cosdeclination,0.0f,sphereDeclination->sindeclination);

        matMakeRotAboutZ(&rotaboutz,sphereDeclination->cosrotang,sphereDeclination->sinrotang);

        for (j=0;j<sphereDeclination->numAnglePoints;j++)
        {
            // place the position

            ship->formationHeading = declination;
            vecScalarMultiply(ship->formationOffset,declination,sphereRadius);

            shipWorkingOn++;
            if (numShips == shipWorkingOn)
            {
                return;
            }
            ship = selection->ShipPtr[shipWorkingOn];

            tempvec = declination;
            matMultiplyMatByVec(&declination,&rotaboutz,&tempvec);      // later optimize by just having a table of unit
                                                                        // vectors describing the sphere?
        }
    }
    dbgAssert(FALSE);       // should return before getting here
}

// define for a ship moving into formation, without changing heading, and trying to steady formation
#define FormationShipJustMoveSteady(desiredpos) \
    if (MoveReachedDestinationVariable(ship,&desiredpos,100.0f))                                                                                                                                                                \
    {                                                                                                                                                                                                                           \
        ship->flags |= SOF_DontDrawTrails;                                                                                                                                                                                      \
    }                                                                                                                                                                                                                           \
    if (!MoveReachedDestinationVariable(ship,&desiredpos,10.0f))                                                                                                                                                                \
    {                                                                                                                                                                                                                           \
        if (aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints|AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)             \
        {                                                                                                                                                                                                                       \
            aitrackSteadyShipDriftOnly(ship);                                                                                                           \
        }                                                                                                                                                                                                                       \
    }                                                                                                                                                   \
    else                                                                                                                                                \
    {                                                                                                                                                   \
        aitrackSteadyShipDriftOnly(ship);                                                                                                               \
    }

// define for a ship moving into formation, without changing heading
#define FormationShipJustMove(desiredpos) \
    if (!MoveReachedDestinationVariable(ship,&desiredpos,10.0f))                                                                                                                                                                \
    {                                                                                                                                                                                                                           \
        aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity);             \
    }

// define for a ship moving into formation, without changing heading, and not trying to fly to obscured points
#define FormationShipJustMoveNotToObscuredPoints(desiredpos) \
    if (!MoveReachedDestinationVariable(ship,&desiredpos,10.0f))                                                                                                                                                                \
    {                                                                                                                                                                                                                           \
        if (aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints|AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)             \
        {                                                                                                                                                                                                                                                            \
            aitrackSteadyShipDriftOnly(ship);                                                                                                           \
        }                                                                                                                                                                                                                                                            \
    }

// define for a ship moving into formation, including heading, and trying to steady formation
#define FormationShipMoveToSteady(desiredpos,head) \
    {                                                                                                                                                       \
        real32 tmpdistaway = MoveLeftToGo(ship,&desiredpos);                                                                                                \
        udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;        \
                                                                                                                                                            \
        if (isCapitalShipStaticOrBig(shipstatic))                                                                                                                \
        {                                                                                                                                                   \
            if (tmpdistaway <= 500.0f)                                                                                                                      \
            {                                                                                                                                               \
                aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                           \
                tmpuseflags = AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;  \
                if (tmpdistaway <= 100.0f) ship->flags |= SOF_DontDrawTrails;                                                                               \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        else if (tmpdistaway <= 100.0f)                                                                                                                     \
        {                                                                                                                                                   \
            aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                               \
            tmpuseflags = AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;      \
            ship->flags |= SOF_DontDrawTrails;                                                                                                              \
        }                                                                                                                                                   \
                                                                                                                                                            \
        if (tmpdistaway >= 10.0f)                                                                                                                           \
        {                                                                                                                                                   \
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,tmpuseflags,0.0f,&leader->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)                \
            {                                                                                                                                               \
                if (tmpuseflags & AISHIP_PointInDirectionFlying)                                                                                            \
                {                                                                                                                                           \
                    aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                       \
                }                                                                                                                                           \
                aitrackSteadyShipDriftOnly(ship);                                                                                                           \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        else                                                                                                                                                \
        {                                                                                                                                                   \
            aitrackSteadyShipDriftOnly(ship);                                                                                                               \
        }                                                                                                                                                   \
    }

// define for a ship moving into formation, including heading, and not trying to fly to obscured points
#define FormationShipMoveToNotToObscuredPoints(desiredpos,head) \
    {                                                                                                                                                       \
        real32 tmpdistaway = MoveLeftToGo(ship,&desiredpos);                                                                                                \
        udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;        \
                                                                                                                                                            \
        if (isCapitalShipStaticOrBig(shipstatic))                                                                                                                \
        {                                                                                                                                                   \
            if (tmpdistaway <= 500.0f)                                                                                                                      \
            {                                                                                                                                               \
                aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                           \
                tmpuseflags = AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;  \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        else if (tmpdistaway <= 100.0f)                                                                                                                     \
        {                                                                                                                                                   \
            aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                               \
            tmpuseflags = AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;      \
        }                                                                                                                                                   \
                                                                                                                                                            \
        if (tmpdistaway >= 10.0f)                                                                                                                           \
        {                                                                                                                                                   \
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,tmpuseflags,0.0f,&leader->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)                \
            {                                                                                                                                               \
                if (tmpuseflags & AISHIP_PointInDirectionFlying)                                                                                            \
                {                                                                                                                                           \
                    aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                       \
                }                                                                                                                                           \
                aitrackSteadyShipDriftOnly(ship);                                                                                                           \
            }                                                                                                                                               \
        }                                                                                                                                                   \
    }

// define for a ship moving into formation, including heading
#define FormationShipMoveTo(desiredpos,head) \
    {                                                                                                                                                       \
        real32 tmpdistaway = MoveLeftToGo(ship,&desiredpos);                                                                                                \
        udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible;                                                                         \
                                                                                                                                                            \
        if (isCapitalShipStaticOrBig(shipstatic))                                                                                                                \
        {                                                                                                                                                   \
            if (tmpdistaway <= 500.0f)                                                                                                                      \
            {                                                                                                                                               \
                aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                           \
                tmpuseflags = AISHIP_FastAsPossible;                                                                                                        \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        else if (tmpdistaway <= 100.0f)                                                                                                                     \
        {                                                                                                                                                   \
            aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                               \
            tmpuseflags = AISHIP_FastAsPossible;                                                                                                            \
        }                                                                                                                                                   \
                                                                                                                                                            \
        if (tmpdistaway >= 10.0f)                                                                                                                           \
        {                                                                                                                                                   \
            aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,tmpuseflags,0.0f,&leader->posinfo.velocity);                                               \
        }                                                                                                                                                   \
    }

void formationWingmanTrackLeader(struct Ship *ship,struct Ship *leader,bool rotate)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    vector heading;
    vector right;
    vector rightback;
    vector desiredpos;
    real32 sepdist;

    matGetVectFromMatrixCol2(right,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);

    sepdist = leader->staticinfo->staticheader.staticCollInfo.collspheresize;
    sepdist += ship->staticinfo->staticheader.staticCollInfo.collspheresize;
    sepdist *= DELTA_FORMATION_PADDING_SCALE;

    vecSub(rightback,right,heading);
    vecMultiplyByScalar(rightback,sepdist);

    vecAdd(desiredpos,leader->posinfo.position,rightback);

//    FormationShipMoveTo(desiredpos,heading);
    if (rotate)
    {
        real32 tmpdistaway = MoveLeftToGo(ship,&desiredpos);
        udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible;

        if (tmpdistaway <= 100.0f)
        {
            aitrackHeadingWithBank(ship,&(heading),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);
            tmpuseflags = AISHIP_FastAsPossible;
        }

        if (tmpdistaway >= 10.0f)
        {
            aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,tmpuseflags,0.0f,&leader->posinfo.velocity);
        }
    }
    else
    {
        if (!MoveReachedDestinationVariable(ship,&desiredpos,10.0f))
        {
            aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity);
        }
    }
}

//function to determine if selection contains nothing but proximity sensors
bool need_Proximity_Sensor_Solution(SelectCommand *selection)
{
   sdword i;
   for(i=0;i<selection->numShips;i++)
   {
      if(selection->ShipPtr[i]->shiptype != ProximitySensor)
      {
         //non-proximity sensor detected, return false
         return(FALSE);
      }
   }

   //Entire selection is proximity sensors
   return(TRUE);
}

//function to determine if selection contains nothing but Target Drones
bool need_TargetDrone_Solution(SelectCommand *selection)
{
   sdword i;
   for(i=0;i<selection->numShips;i++)
   {
      if(selection->ShipPtr[i]->shiptype != TargetDrone)
      {
         return(FALSE);
      }
   }
   return(TRUE);
}


#define oneOverRootTwo 0.7071068f

void setFormationToDo(struct CommandToDo *formationtodo)
{
    sdword i;
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader = selection->ShipPtr[0];
    matrix *coordsys;
    vector position;
    vector desiredposition;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    vector heading;

    formationtodo->formation.percentmaxspeed = FORMATION_TRAVELVEL_MAXSCALE;

    dbgAssert(numShips > 0);
    if (numShips == 1)
    {
        return;
    }

    position = leader->posinfo.position;

    /*  Setup formations coordinate system */
    if(formationtodo->formation.formationLocked)
    {
        coordsys = &formationtodo->formation.coordsys;
        matGetVectFromMatrixCol3(heading,formationtodo->formation.coordsys);
    }
    else
    {
        coordsys = &leader->rotinfo.coordsys;
        matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);
    }

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case CUSTOM_FORMATION:
            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                matMultiplyMatByVec(&desiredposition,coordsys,&ship->formationOffset);
                vecAddTo(desiredposition,position);

                ship->posinfo.position = desiredposition;
                ship->rotinfo.coordsys = *coordsys;
                univUpdateObjRotInfo((SpaceObjRot *)ship);
            }
            break;

        case SPHERE_FORMATION:
            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                vecAdd(ship->posinfo.position,position,ship->formationOffset);
                ship->rotinfo.coordsys = *coordsys;
                univUpdateObjRotInfo((SpaceObjRot *)ship);
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

void processFormationToDo(struct CommandToDo *formationtodo,bool steadyFormation,bool passiveAttacked)
{
    sdword i;
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader = selection->ShipPtr[0];
    matrix *coordsys;
    vector position;
    vector desiredposition;
    bool dontrotate;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    vector heading;
    real32 error = 0;
    vector errorvect;
    bool calcError = ((universe.univUpdateCounter & FORMATION_ERROR_CALCULATE_RATE) == FORMATION_ERROR_CALCULATE_FRAME);
    // write to leader->formationcommand instead of formationtodo - formationtodo might be a fakeCommand from delegateCommand
    CommandToDo *formationtomodify = leader->formationcommand;

    dbgAssert(numShips > 0);
    if (numShips == 1)
    {
        formationtomodify->formation.percentmaxspeed = FORMATION_TRAVELVEL_MAXSCALE;
        setFormationTravelVelocity(formationtomodify);
        return;
    }

    position = leader->posinfo.position;

    /*  Setup formations coordinate system */
    if(formationtodo->formation.formationLocked)
    {
        coordsys = &formationtodo->formation.coordsys;
        matGetVectFromMatrixCol3(heading,formationtodo->formation.coordsys);
    }
    else
    {
        coordsys = &leader->rotinfo.coordsys;
        matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);
    }

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case CUSTOM_FORMATION:
            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;
                dontrotate = (passiveAttacked & (bool)shipstatic->rotateToRetaliate);
                ship->shipidle = FALSE;

                matMultiplyMatByVec(&desiredposition,coordsys,&ship->formationOffset);
                vecAddTo(desiredposition,position);

                if (calcError)
                {
                    vecSub(errorvect,desiredposition,ship->posinfo.position);
                    error += vecMagnitudeSquared(errorvect);
                }

                if(!bitTest(ship->specialFlags,SPECIAL_BrokenFormation))
                if (dontrotate)
                {
                    if (steadyFormation)
                    {
                        FormationShipJustMoveSteady(desiredposition)
                    }
                    else
                    {
                        FormationShipJustMove(desiredposition)
                    }
                }
                else
                {
                    if (steadyFormation)
                    {
                        FormationShipMoveToSteady(desiredposition,heading)
                    }
                    else
                    {
                        FormationShipMoveTo(desiredposition,heading)
                    }
                }
            }
            break;

        case SPHERE_FORMATION:
        {
            bool protectsurround = FALSE;

            if (leader->shiptype == Drone)
            {
                aishipTempDisableAvoiding = TRUE;
            }

            if (formationtodo->ordertype.attributes & COMMAND_IS_PROTECTING)
            {
                dbgAssert(formationtodo->protect->numShips > 0);
                leader = formationtodo->protect->ShipPtr[0];
                position = leader->posinfo.position;       // override position
                matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);
                protectsurround = TRUE;
                i = 0;
            }
            else if (formationtodo->ordertype.order == COMMAND_ATTACK)
            {
                if(formationtodo->formation.enders == TRUE)
                {
                    Ship *protectship;
                    //protectship = formationtodo->formation.endership;
                    if(formationtodo->selection->ShipPtr[0]->attackvars.attacktarget == NULL)
                    {
                        //don't know if this will happen ever...we'll find out
                        //
                        //
                        //  Tell Bryce about this!
                        //
                        //
#ifdef DEBUG_TACTICS
                        _asm int 3
#endif
                        protectship = (Ship*)formationtodo->attack->TargetPtr[0];       //this can't possibly be invalid!
                    }
                    else
                    {
                        protectship = (Ship *)formationtodo->selection->ShipPtr[0]->attackvars.attacktarget;
                    }
                    //protectship = formationtodo->selection->ShipPtr[0]->attackvars.attacktarget;
                    leader = protectship;
                    position = leader->posinfo.position;       // override position
                    protectsurround = TRUE;
                    i = 0;
                }
                else
                    i=1;
            }
            else
            {
                i = 1;
            }

            for (;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;
                dontrotate = (passiveAttacked & (bool)shipstatic->rotateToRetaliate);
                if ((i & 7) != 0)
                {
                    ship->shipidle = FALSE;     // don't see shipidle for every 8th ship, and never set the 0th ship's shipidle
                }

                vecAdd(desiredposition,position,ship->formationOffset);

                if (calcError)
                {
                    vecSub(errorvect,desiredposition,ship->posinfo.position);
                    error += vecMagnitudeSquared(errorvect);
                }

                if(!bitTest(ship->specialFlags,SPECIAL_BrokenFormation))
                if (dontrotate)
                {
                    if (steadyFormation)
                    {
                        FormationShipJustMoveSteady(desiredposition)
                    }
                    else
                    {
                        if (protectsurround)
                        {
                            FormationShipJustMoveNotToObscuredPoints(desiredposition)
                        }
                        else
                        {
                            FormationShipJustMove(desiredposition)
                        }
                    }
                }
                else
                {
                    if (steadyFormation)
                    {
                        FormationShipMoveToSteady(desiredposition,ship->formationHeading)
                    }
                    else
                    {
                        if (protectsurround)
                        {
                            if (leader->posinfo.isMoving & ISMOVING_MOVING)
                            {
                                FormationShipMoveToNotToObscuredPoints(desiredposition,heading);
                            }
                            else
                            {
                                FormationShipMoveToNotToObscuredPoints(desiredposition,ship->formationHeading);
                            }
                        }
                        else
                        {
                            FormationShipMoveTo(desiredposition,heading)
                        }
                    }
                }
            }

            aishipTempDisableAvoiding = FALSE;
        }
        break;

        default:
            dbgAssert(FALSE);
            break;
    }

    if (calcError)
    {
        error /= numShips;
#if 0
        dbgMessagef("\nFormation Error %f  ",error);
#endif
        if (error > FORMATION_ERROR_BIG)
        {
            formationtomodify->formation.percentmaxspeed = FORMATION_TRAVELVEL_MINSCALE;
        }
        else
        {
            if (error < FORMATION_ERROR_NEGLIGIBLE)
            {
                formationtomodify->formation.percentmaxspeed = FORMATION_TRAVELVEL_MAXSCALE;
            }
            else
            {
                formationtomodify->formation.percentmaxspeed = FORMATION_TRAVELVEL_MAXSCALE - (error / FORMATION_ERROR_BIG) * (FORMATION_TRAVELVEL_MAXSCALE - FORMATION_TRAVELVEL_MINSCALE);
            }
        }
#if 0
        dbgMessagef("Perc Max speed %f",formationtomodify->formation.percentmaxspeed);
#endif

        setFormationTravelVelocity(formationtomodify);
    }
}

void formationTypeHasChanged(struct CommandToDo *command)
{
    TypeOfFormation formationtype = command->formation.formationtype;

    if (formationSortType[formationtype] != command->formation.sortorder)
    {
        formationArrangeOptimum(command);
    }

    FormationCalculateOffsets(command);
}

void formationContentHasChanged(struct CommandToDo *command)
{
    FormationCalculateOffsets(command);
}

/*-----------------------------------------------------------------------------
    Name        : clSelectionAlreadyInFormation
    Description : Call this function to see if this selection is already in formation
    Inputs      :
    Outputs     :
    Return      : returns NO_FORMATION if this selection is not already in formation, otherwise
                  returns the type of formation the selection is in.
----------------------------------------------------------------------------*/
TypeOfFormation clSelectionAlreadyInFormation(CommandLayer *comlayer,SelectCommand *selectcom)
{
    CommandToDo *alreadyformation;

    if ((alreadyformation = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (alreadyformation->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            return alreadyformation->formation.formationtype;
        }
    }
    return NO_FORMATION;
}

/*-----------------------------------------------------------------------------
    Name        : FillInShipFormationStuff
    Description : Fills in formation stuff in ship
    Inputs      : ship, formationcommand
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FillInShipFormationStuff(Ship *ship,struct CommandToDo *formationcommand)
{
    ship->formationcommand = formationcommand;
}

/*-----------------------------------------------------------------------------
    Name        : GetShipsTravelVel
    Description : returns the selection's velocity at which it can all travel, scaled by scalevel
    Inputs      : selection, scalevel
    Outputs     :
    Return      : returns the selection's velocity at which it can all travel, scaled by scalevel
----------------------------------------------------------------------------*/
real32 GetShipsTravelVel(SelectCommand *selection,real32 scalevel)
{
    real32 minvel = REALlyBig;
    real32 testvel;
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        // cap velocity vector
        //Get ships max velocity
        testvel = tacticsGetShipsMaxVelocity(ship);

        if (testvel < minvel)
        {
            minvel = testvel;
        }
    }

    dbgAssert(minvel >= 0.0f);
    dbgAssert(minvel != REALlyBig);

    minvel *= scalevel;

    return minvel;
}

void  setFormationTravelVelocity(CommandToDo *formationcommand)
{
    //won't hurt if we were to set the travel velocity and ships aren't
    //in formation, but lets not let it happen anyways.
    dbgAssert(formationcommand->ordertype.attributes & COMMAND_IS_FORMATION);

    formationcommand->formation.travelvel = GetShipsTravelVel(formationcommand->selection,formationcommand->formation.percentmaxspeed);
}


/*-----------------------------------------------------------------------------
    Name        : formationRemoveShipFromSelection
    Description : removes ship from selection of formation
    Inputs      :
    Outputs     :
    Return      : -1 if not found, shipindex if found
----------------------------------------------------------------------------*/
sdword formationRemoveShipFromSelection(struct CommandToDo *formationtodo,Ship *removeship)
{
    SelectCommand *selection = formationtodo->selection;
    sdword i,j;
    sdword returnval;

    dbgAssert(formationtodo->ordertype.attributes & COMMAND_IS_FORMATION);

    for (i=0;i<selection->numShips; )
    {
        if (removeship == selection->ShipPtr[i])
        {
            goto foundshiptoremove;
        }
        i++;
    }

    return -1;

foundshiptoremove:
    returnval = i;

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
            if (selection->numShips <= 2)
            {
                goto defaultremove;
            }

            // move one wing forward
            for (j=i;j<selection->numShips;j+=2)
            {
                if ((j+2) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+2];
                }
                else if ((j+1) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+1];
                }
            }
            selection->numShips--;
            break;

        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
            // move one wing forward
            for (j=i;j<selection->numShips;j+=4)
            {
                if ((j+4) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+4];
                }
                else if ((j+3) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+3];
                }
                else if ((j+2) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+2];
                }
                else if ((j+1) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+1];
                }
            }
            selection->numShips--;
            break;

defaultremove:
        default:
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            break;
    }

    return returnval;
}

void FormationCalculateOffsets(struct CommandToDo *formationtodo)
{
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader;
    ShipStaticInfo *leaderstatic;
    real32 tacticsFormationPadding;
    sdword i;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    sdword prox_sol;
    real32 searchRadius;

    dbgAssert(numShips > 0);

    if (formationtodo->formation.formationtype == SPHERE_FORMATION)
    {
        formationSpecificsSphere(formationtodo);
        return;
    }

    leader = selection->ShipPtr[0];
    leaderstatic = leader->staticinfo;
    tacticsFormationPadding = tacticsInfo.formationPadding[formationtodo->formation.tacticalState];

    prox_sol = need_Proximity_Sensor_Solution(selection);
    if (prox_sol)
    {
        searchRadius = ((ProximitySensorStatics *)leaderstatic->custstatinfo)->SearchRadius;
    }
    else
    {
        if ((formationtodo->formation.formationtype == WALL_FORMATION) && (need_TargetDrone_Solution(selection)))
        {
            prox_sol = TRUE;
            searchRadius = TARGETDRONE_FORMATION_SIZE;
        }
    }

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            // rightback is (0,1,-1)
            // leftback is (0,-1,-1)
            real32 rightsepdist;
            real32 leftsepdist;
            real32 thissepdist;
            vector desiredposright = { 0.0f, 0.0f, 0.0f };
            vector desiredposleft = { 0.0f, 0.0f, 0.0f };

            if (prox_sol)
            {
                leftsepdist = searchRadius;
            }
            else
            {
                leftsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }
            rightsepdist = leftsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                if (i & 1)
                {
                    rightsepdist += thissepdist;
                    if (!prox_sol) rightsepdist *= DELTA_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposright.y -= rightsepdist;
                    else
                        desiredposright.y += rightsepdist;

                    desiredposright.z -= rightsepdist;

                    ship->formationOffset = desiredposright;

                    rightsepdist = thissepdist;
                }
                else
                {
                    leftsepdist += thissepdist;
                    if (!prox_sol) leftsepdist *= DELTA_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposleft.y += leftsepdist;
                    else
                        desiredposleft.y -= leftsepdist;

                    desiredposleft.z -= leftsepdist;

                    ship->formationOffset = desiredposleft;

                    leftsepdist = thissepdist;
                }
            }
        }
        break;

        case BROAD_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            // right is (0,1,0)
            // left is (0,-1,0)
            real32 rightsepdist;
            real32 leftsepdist;
            real32 thissepdist;
            vector desiredposright = { 0.0f, 0.0f, 0.0f };
            vector desiredposleft = { 0.0f, 0.0f, 0.0f };

            if (prox_sol)
            {
                leftsepdist = searchRadius;
            }
            else
            {
                leftsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }
            rightsepdist = leftsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                if (i & 1)
                {
                    rightsepdist += thissepdist;
                    if (!prox_sol) rightsepdist *= BROAD_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposright.y -= rightsepdist;
                    else
                        desiredposright.y += rightsepdist;

                    ship->formationOffset = desiredposright;

                    rightsepdist = thissepdist;
                }
                else
                {
                    leftsepdist += thissepdist;
                    if (!prox_sol) leftsepdist *= BROAD_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposleft.y += leftsepdist;
                    else
                        desiredposleft.y -= leftsepdist;



                    ship->formationOffset = desiredposleft;

                    leftsepdist = thissepdist;
                }
            }
        }
        break;

        case DELTA3D_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            vector desiredposupright = { 0.0f, 0.0f, 0.0f };
            vector desiredposupleft = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnright = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnleft = { 0.0f, 0.0f, 0.0f };
            real32 uprightsepdist;
            real32 upleftsepdist;
            real32 dnrightsepdist;
            real32 dnleftsepdist;
            real32 thissepdist;

            if (prox_sol)
            {
                uprightsepdist = searchRadius;
            }
            else
            {
                uprightsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }

            upleftsepdist = uprightsepdist;
            dnrightsepdist = uprightsepdist;
            dnleftsepdist = uprightsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                switch (i & 3)
                {
                    case 1:
                        uprightsepdist += thissepdist;
                        if (!prox_sol) uprightsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // uprightback is (1,1,-1)
                        desiredposupright.x += uprightsepdist;

                        if(formationtodo->formation.flipState)
                            desiredposupright.y -= uprightsepdist;
                        else
                            desiredposupright.y += uprightsepdist;
//                        desiredposupright.z -= uprightsepdist;

                        ship->formationOffset = desiredposupright;

                        uprightsepdist = thissepdist;
                        break;

                    case 2:
                        upleftsepdist += thissepdist;
                        if (!prox_sol) upleftsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // upleftback is (1,-1,-1)
                        desiredposupleft.x += upleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupleft.y += upleftsepdist;
                        else
                            desiredposupleft.y -= upleftsepdist;
//                        desiredposupleft.z -= upleftsepdist;

                        ship->formationOffset = desiredposupleft;

                        upleftsepdist = thissepdist;
                        break;

                    case 0:
                        dnleftsepdist += thissepdist;
                        if (!prox_sol) dnleftsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // dnleftback is (-1,-1,-1)
                        desiredposdnleft.x -= dnleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnleft.y += dnleftsepdist;
                        else
                            desiredposdnleft.y -= dnleftsepdist;
  //                      desiredposdnleft.z -= dnleftsepdist;

                        ship->formationOffset = desiredposdnleft;

                        dnleftsepdist = thissepdist;
                        break;

                    case 3:
                        dnrightsepdist += thissepdist;
                        if (!prox_sol) dnrightsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // dnrightback is (-1,1,-1)
                        desiredposdnright.x -= dnrightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnright.y -= dnrightsepdist;
                        else
                            desiredposdnright.y += dnrightsepdist;
//                        desiredposdnright.z -= dnrightsepdist;

                        ship->formationOffset = desiredposdnright;

                        dnrightsepdist = thissepdist;
                        break;
                }
            }
        }
        break;

        case CLAW_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            vector desiredposupright = { 0.0f, 0.0f, 0.0f };
            vector desiredposupleft = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnright = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnleft = { 0.0f, 0.0f, 0.0f };
            real32 uprightsepdist;
            real32 upleftsepdist;
            real32 dnrightsepdist;
            real32 dnleftsepdist;
            real32 thissepdist;

            real32 forwardmodifier = 1.0f;
            real32 forsepdist;

            if (prox_sol)
            {
                uprightsepdist = searchRadius;
            }
            else
            {
                uprightsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }

            upleftsepdist = uprightsepdist;
            dnrightsepdist = uprightsepdist;
            dnleftsepdist = uprightsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                switch (i & 3)
                {
                    case 1:
                        uprightsepdist += thissepdist;
                        if (!prox_sol) uprightsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = uprightsepdist * forwardmodifier;

                        // uprightforward is (1,1,1)
                        desiredposupright.x += uprightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupright.y -= uprightsepdist;
                        else
                            desiredposupright.y += uprightsepdist;
                        desiredposupright.z += forsepdist;

                        ship->formationOffset = desiredposupright;

                        uprightsepdist = thissepdist;
                        break;

                    case 2:
                        upleftsepdist += thissepdist;
                        if (!prox_sol) upleftsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = upleftsepdist * forwardmodifier;

                        // upleftforward is (1,-1,1)

                        desiredposupleft.x += upleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupleft.y += upleftsepdist;
                        else
                            desiredposupleft.y -= upleftsepdist;
                        desiredposupleft.z += forsepdist;

                        ship->formationOffset = desiredposupleft;

                        upleftsepdist = thissepdist;
                        break;

                    case 0:
                        dnleftsepdist += thissepdist;
                        if (!prox_sol) dnleftsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = dnleftsepdist * forwardmodifier;

                        // dnleftforward is (-1,-1,1)
                        desiredposdnleft.x -= dnleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnleft.y += dnleftsepdist;
                        else
                            desiredposdnleft.y -= dnleftsepdist;
                        desiredposdnleft.z += forsepdist;

                        ship->formationOffset = desiredposdnleft;

                        dnleftsepdist = thissepdist;

                        forwardmodifier *= CLAW_FORMATION_CURVATURE;
                        break;

                    case 3:
                        dnrightsepdist += thissepdist;
                        if (!prox_sol) dnrightsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = dnrightsepdist * forwardmodifier;

                        // dnrightforward is (-1,1,1)
                        desiredposdnright.x -= dnrightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnright.y -= dnrightsepdist;
                        else
                            desiredposdnright.y += dnrightsepdist;
                        desiredposdnright.z += forsepdist;

                        ship->formationOffset = desiredposdnright;

                        dnrightsepdist = thissepdist;
                        break;
                }
            }
        }
        break;

        case WALL_FORMATION:
        {
            sdword squaresize;
            sdword curpos,currow;
            real32 sepdist;
            vector desiredpos = { 0.0f, 0.0f, 0.0f };
            vector desiredv = { 0.0f, 0.0f, 0.0f };
            Ship *ship;
            ShipStaticInfo *shipstatic;

            squaresize = (udword)fsqrt((real32)numShips);
            if ((squaresize*squaresize) < numShips)
            {
                squaresize++;
            }

            if (prox_sol)
            {
                sepdist = searchRadius*2.0f;
            }
            else
            {
                sepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*tacticsFormationPadding*
                            leaderstatic->formationPaddingModifier*WALL_FORMATION_PADDING_SCALE + WALL_FORMATION_PADDING;
            }

            // rightspacing is (0,1,0)
            // downspacing is (-1,0,0)
            if(formationtodo->formation.flipState)
                desiredpos.y -= sepdist;
            else
                desiredpos.y += sepdist;

            for (curpos=1,currow=0,i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                ship->formationOffset = desiredpos;

                curpos++;
                if (curpos >= squaresize)
                {
                    curpos = 0;
                    currow++;
                    desiredv.x -= sepdist;
                    desiredpos = desiredv;
                }
                else
                {
                    if(formationtodo->formation.flipState)
                        desiredpos.y -= sepdist;
                    else
                        desiredpos.y += sepdist;
                }
            }
        }
        break;

        case CUSTOM_FORMATION:
            {
                vector desiredpos = { 0.0f, 0.0f, 0.0f };
                vector averagepos = { 0.0f, 0.0f, 0.0f };
                vector error = { 0.0f, 0.0f, 0.0f };
                Ship *ship;
                real32 minError,totalError;
                sdword minErrorIndex;


                //get center point
                for(i=0;i<numShips;i++)
                {
                    vecAddTo(averagepos,selection->ShipPtr[i]->posinfo.position);
                }
                vecScalarMultiply(averagepos,averagepos,1.0f/numShips);

                //find ship closest to centerpoint to use for leader
                minError = REALlyBig;
                minErrorIndex = 0;
                for(i=0;i<numShips;i++)
                {
                    vecSub(error,selection->ShipPtr[i]->posinfo.position,averagepos);
                    error.x = ABS(error.x);
                    error.y = ABS(error.y);
                    error.z = ABS(error.z);
                    totalError = error.x+error.y+error.z;
                    if(totalError < minError)
                    {
                        //new closest ship!
                        minError = totalError;
                        minErrorIndex = i;
                    }
                }

                //swap center ship with leader position
                ship = selection->ShipPtr[0];
                selection->ShipPtr[0] = selection->ShipPtr[minErrorIndex];
                selection->ShipPtr[minErrorIndex] = ship;

                leader = selection->ShipPtr[0];

                //convert all ships current positions to relatavistic offsets
                for(i=1;i<numShips;i++)
                {
                    ship = (Ship *) selection->ShipPtr[i];

                    vecSub(desiredpos,ship->posinfo.position,leader->posinfo.position);
                    matMultiplyVecByMat(&ship->formationOffset,&desiredpos,&leader->rotinfo.coordsys);
                }
            }
        break;
        default:
            dbgAssert(FALSE);
            break;
    }
}

void FormationCalculateSpecialOffsets(struct CommandToDo *formationtodo)
{
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader;
    ShipStaticInfo *leaderstatic;
    real32 tacticsFormationPadding;
    sdword i;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    sdword prox_sol;
    real32 searchRadius;

    dbgAssert(numShips > 0);

    if (formationtodo->formation.formationtype == SPHERE_FORMATION)
    {
        formationSpecificsSphere(formationtodo);
        return;
    }

    leader = selection->ShipPtr[0];
    leaderstatic = leader->staticinfo;
    tacticsFormationPadding = tacticsInfo.formationPadding[formationtodo->formation.tacticalState];

    prox_sol = need_Proximity_Sensor_Solution(selection);
    if (prox_sol)
    {
        searchRadius = ((ProximitySensorStatics *)leaderstatic->custstatinfo)->SearchRadius;
    }
    else
    {
        if ((formationtodo->formation.formationtype == WALL_FORMATION) && (need_TargetDrone_Solution(selection)))
        {
            prox_sol = TRUE;
            searchRadius = TARGETDRONE_FORMATION_SIZE;
        }
    }

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            // rightback is (0,1,-1)
            // leftback is (0,-1,-1)
            real32 rightsepdist;
            real32 leftsepdist;
            real32 thissepdist;
            vector desiredposright = { 0.0f, 0.0f, 0.0f };
            vector desiredposleft = { 0.0f, 0.0f, 0.0f };

            if (prox_sol)
            {
                leftsepdist = searchRadius;
            }
            else
            {
                leftsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }
            rightsepdist = leftsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                if (i & 1)
                {
                    rightsepdist += thissepdist;
                    if (!prox_sol) rightsepdist *= DELTA_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposright.y -= rightsepdist;
                    else
                        desiredposright.y += rightsepdist;

                    desiredposright.z -= rightsepdist;

                    ship->formationOffset = desiredposright;

                    rightsepdist = thissepdist;
                }
                else
                {
                    leftsepdist += thissepdist;
                    if (!prox_sol) leftsepdist *= DELTA_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposleft.y += leftsepdist;
                    else
                        desiredposleft.y -= leftsepdist;

                    desiredposleft.z -= leftsepdist;

                    ship->formationOffset = desiredposleft;

                    leftsepdist = thissepdist;
                }
            }
        }
        break;

        case BROAD_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            // right is (0,1,0)
            // left is (0,-1,0)
            real32 rightsepdist;
            real32 leftsepdist;
            real32 thissepdist;
            vector desiredposright = { 0.0f, 0.0f, 0.0f };
            vector desiredposleft = { 0.0f, 0.0f, 0.0f };

            if (prox_sol)
            {
                leftsepdist = searchRadius;
            }
            else
            {
                leftsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }
            rightsepdist = leftsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                if (i & 1)
                {
                    rightsepdist += thissepdist;
                    if (!prox_sol) rightsepdist *= BROAD_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposright.y -= rightsepdist;
                    else
                        desiredposright.y += rightsepdist;

                    ship->formationOffset = desiredposright;

                    rightsepdist = thissepdist;
                }
                else
                {
                    leftsepdist += thissepdist;
                    if (!prox_sol) leftsepdist *= BROAD_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposleft.y += leftsepdist;
                    else
                        desiredposleft.y -= leftsepdist;



                    ship->formationOffset = desiredposleft;

                    leftsepdist = thissepdist;
                }
            }
        }
        break;

        case DELTA3D_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            vector desiredposupright = { 0.0f, 0.0f, 0.0f };
            vector desiredposupleft = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnright = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnleft = { 0.0f, 0.0f, 0.0f };
            real32 uprightsepdist;
            real32 upleftsepdist;
            real32 dnrightsepdist;
            real32 dnleftsepdist;
            real32 thissepdist;

            if (prox_sol)
            {
                uprightsepdist = searchRadius;
            }
            else
            {
                uprightsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }

            upleftsepdist = uprightsepdist;
            dnrightsepdist = uprightsepdist;
            dnleftsepdist = uprightsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                switch (i & 3)
                {
                    case 1:
                        uprightsepdist += thissepdist;
                        if (!prox_sol) uprightsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // uprightback is (1,1,-1)
                        desiredposupright.x += uprightsepdist;

                        if(formationtodo->formation.flipState)
                            desiredposupright.y -= uprightsepdist;
                        else
                            desiredposupright.y += uprightsepdist;
//                        desiredposupright.z -= uprightsepdist;

                        ship->formationOffset = desiredposupright;

                        uprightsepdist = thissepdist;
                        break;

                    case 2:
                        upleftsepdist += thissepdist;
                        if (!prox_sol) upleftsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // upleftback is (1,-1,-1)
                        desiredposupleft.x += upleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupleft.y += upleftsepdist;
                        else
                            desiredposupleft.y -= upleftsepdist;
//                        desiredposupleft.z -= upleftsepdist;

                        ship->formationOffset = desiredposupleft;

                        upleftsepdist = thissepdist;
                        break;

                    case 0:
                        dnleftsepdist += thissepdist;
                        if (!prox_sol) dnleftsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // dnleftback is (-1,-1,-1)
                        desiredposdnleft.x -= dnleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnleft.y += dnleftsepdist;
                        else
                            desiredposdnleft.y -= dnleftsepdist;
  //                      desiredposdnleft.z -= dnleftsepdist;

                        ship->formationOffset = desiredposdnleft;

                        dnleftsepdist = thissepdist;
                        break;

                    case 3:
                        dnrightsepdist += thissepdist;
                        if (!prox_sol) dnrightsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // dnrightback is (-1,1,-1)
                        desiredposdnright.x -= dnrightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnright.y -= dnrightsepdist;
                        else
                            desiredposdnright.y += dnrightsepdist;
//                        desiredposdnright.z -= dnrightsepdist;

                        ship->formationOffset = desiredposdnright;

                        dnrightsepdist = thissepdist;
                        break;
                }
            }
        }
        break;

        case CLAW_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            vector desiredposupright = { 0.0f, 0.0f, 0.0f };
            vector desiredposupleft = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnright = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnleft = { 0.0f, 0.0f, 0.0f };
            real32 uprightsepdist;
            real32 upleftsepdist;
            real32 dnrightsepdist;
            real32 dnleftsepdist;
            real32 thissepdist;

            real32 forwardmodifier = 1.0f;
            real32 forsepdist;

            if (prox_sol)
            {
                uprightsepdist = searchRadius;
            }
            else
            {
                uprightsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }

            upleftsepdist = uprightsepdist;
            dnrightsepdist = uprightsepdist;
            dnleftsepdist = uprightsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                switch (i & 3)
                {
                    case 1:
                        uprightsepdist += thissepdist;
                        if (!prox_sol) uprightsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = uprightsepdist * forwardmodifier;

                        // uprightforward is (1,1,1)
                        desiredposupright.x += uprightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupright.y -= uprightsepdist;
                        else
                            desiredposupright.y += uprightsepdist;
                        desiredposupright.z += forsepdist;

                        ship->formationOffset = desiredposupright;

                        uprightsepdist = thissepdist;
                        break;

                    case 2:
                        upleftsepdist += thissepdist;
                        if (!prox_sol) upleftsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = upleftsepdist * forwardmodifier;

                        // upleftforward is (1,-1,1)

                        desiredposupleft.x += upleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupleft.y += upleftsepdist;
                        else
                            desiredposupleft.y -= upleftsepdist;
                        desiredposupleft.z += forsepdist;

                        ship->formationOffset = desiredposupleft;

                        upleftsepdist = thissepdist;
                        break;

                    case 0:
                        dnleftsepdist += thissepdist;
                        if (!prox_sol) dnleftsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = dnleftsepdist * forwardmodifier;

                        // dnleftforward is (-1,-1,1)
                        desiredposdnleft.x -= dnleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnleft.y += dnleftsepdist;
                        else
                            desiredposdnleft.y -= dnleftsepdist;
                        desiredposdnleft.z += forsepdist;

                        ship->formationOffset = desiredposdnleft;

                        dnleftsepdist = thissepdist;

                        forwardmodifier *= CLAW_FORMATION_CURVATURE;
                        break;

                    case 3:
                        dnrightsepdist += thissepdist;
                        if (!prox_sol) dnrightsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = dnrightsepdist * forwardmodifier;

                        // dnrightforward is (-1,1,1)
                        desiredposdnright.x -= dnrightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnright.y -= dnrightsepdist;
                        else
                            desiredposdnright.y += dnrightsepdist;
                        desiredposdnright.z += forsepdist;

                        ship->formationOffset = desiredposdnright;

                        dnrightsepdist = thissepdist;
                        break;
                }
            }
        }
        break;

        case WALL_FORMATION:
        {
            sdword squaresize;
            sdword curpos,currow;
            real32 sepdist;
            vector desiredpos = { 0.0f, 0.0f, 0.0f };
            vector desiredv = { 0.0f, 0.0f, 0.0f };
            Ship *ship;
            ShipStaticInfo *shipstatic;

            squaresize = (udword)fsqrt((real32)numShips);
            if ((squaresize*squaresize) < numShips)
            {
                squaresize++;
            }

            if (prox_sol)
            {
                sepdist = searchRadius*2.0f;
            }
            else
            {
                sepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*tacticsFormationPadding*
                            leaderstatic->formationPaddingModifier*WALL_FORMATION_PADDING_SCALE + WALL_FORMATION_PADDING;
            }

            // rightspacing is (0,1,0)
            // downspacing is (-1,0,0)
            if(formationtodo->formation.flipState)
                desiredpos.y -= sepdist;
            else
                desiredpos.y += sepdist;

            for (curpos=1,currow=0,i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                ship->formationOffset = desiredpos;

                curpos++;
                if (curpos >= squaresize)
                {
                    curpos = 0;
                    currow++;
                    desiredv.x -= sepdist;
                    desiredpos = desiredv;
                }
                else
                {
                    if(formationtodo->formation.flipState)
                        desiredpos.y -= sepdist;
                    else
                        desiredpos.y += sepdist;
                }
            }
        }
        break;

        case CUSTOM_FORMATION:
            {
                vector desiredpos = { 0.0f, 0.0f, 0.0f };
                vector averagepos = { 0.0f, 0.0f, 0.0f };
                vector error = { 0.0f, 0.0f, 0.0f };
                Ship *ship;
                real32 minError,totalError;
                sdword minErrorIndex;


                //get center point
                for(i=0;i<numShips;i++)
                {
                    vecAddTo(averagepos,selection->ShipPtr[i]->posinfo.position);
                }
                vecScalarMultiply(averagepos,averagepos,1.0f/numShips);

                //find ship closest to centerpoint to use for leader
                minError = REALlyBig;
                minErrorIndex = 0;
                for(i=0;i<numShips;i++)
                {
                    vecSub(error,selection->ShipPtr[i]->posinfo.position,averagepos);
                    error.x = ABS(error.x);
                    error.y = ABS(error.y);
                    error.z = ABS(error.z);
                    totalError = error.x+error.y+error.z;
                    if(totalError < minError)
                    {
                        //new closest ship!
                        minError = totalError;
                        minErrorIndex = i;
                    }
                }

                //swap center ship with leader position
                ship = selection->ShipPtr[0];
                selection->ShipPtr[0] = selection->ShipPtr[minErrorIndex];
                selection->ShipPtr[minErrorIndex] = ship;

                leader = selection->ShipPtr[0];

                //convert all ships current positions to relatavistic offsets
                for(i=1;i<numShips;i++)
                {
                    ship = (Ship *) selection->ShipPtr[i];

                    vecSub(desiredpos,ship->posinfo.position,leader->posinfo.position);
                    matMultiplyVecByMat(&ship->formationOffset,&desiredpos,&leader->rotinfo.coordsys);
                }
            }
        break;
        default:
            dbgAssert(FALSE);
            break;
    }
}

real32 calculateTravelDistance(CommandToDo *formationtodo)
{
    sdword i;
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader = selection->ShipPtr[0];
    matrix *coordsys;
    vector position;
    vector desiredposition;
    Ship *ship;
    real32  distanceSqr=0.0f;
    vector distvec;

    position = leader->posinfo.position;
    coordsys = &leader->rotinfo.coordsys;

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case CUSTOM_FORMATION:
            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];

                //calculate realspace position
                matMultiplyMatByVec(&desiredposition,coordsys,&ship->formationOffset);
                vecAddTo(desiredposition,position);

                //calculate distance ship must go
                vecSub(distvec,ship->posinfo.position,desiredposition);
                distanceSqr += vecMagnitudeSquared(distvec);
            }
        case SPHERE_FORMATION:
            break;
    }
    return distanceSqr;
}

//if smallest ship radius/biggest ship radius is bigger than this, then
//we ignore leader size in choosing a leader and find the center leader!
#define BIG_SMALL_RATIO_TO_BE_EQUAL_FOR_LEADER      0.7f

void formationArrageCrazyOptimum(CommandToDo *formationcommand)
{
    //algorithm...go through each ship...selecting each to be the leader
    //then go through each iteration of that to calculate the minimum distance
    //then select the minimum distance for actual use!
    sdword i,j,a,p;
    sdword numShips = formationcommand->selection->numShips;
    SelectCommand *selection = formationcommand->selection;
    SelectCommand *optimumSel;
    SelectCommand *originalSel,*testSel;
    real32 tempdist,biggestToSmallestRatio;
    sdword numWhoWant,poswanted;
    real32 minDist = REALlyBig;
    real32 mindistleader,tempdist3;

    vector positionsMax[COMMAND_MAX_SHIPS];
    sdword shipWantsPositionMax[COMMAND_MAX_SHIPS];
    sdword shipGetsPositionMax[COMMAND_MAX_SHIPS];
    sdword positionTakenMax[COMMAND_MAX_SHIPS];

    vector *positions;
    sdword *shipWantsPosition;
    sdword *shipGetsPosition;
    sdword *positionTaken;
    bool redo = FALSE;

    matrix *coordsys;
    //vector center = {0.0f,0.0f,0.0f};
    sdword shipwhogetsit;
    vector tempvec;
    Ship *ship;
    //could possible select 'middle' ship as leader...would be faster...
    //will test effect on efficiency

    //allocation enough memory for
    optimumSel = memAlloc(sizeofSelectCommand(selection->numShips),"FormationOpt2",0);
    originalSel = memAlloc(sizeofSelectCommand(selection->numShips),"FormationOpt1",0);
    testSel = memAlloc(sizeofSelectCommand(selection->numShips),"FormationOpt1",0);


    if (numShips > COMMAND_MAX_SHIPS-1)
    {
        positions=memAlloc(sizeof(vector)*numShips,"forma1",0);
        shipWantsPosition=memAlloc(sizeof(sdword)*numShips,"forma2",0);
        shipGetsPosition=memAlloc(sizeof(sdword)*numShips,"forma3",0);
        positionTaken=memAlloc(sizeof(sdword)*numShips,"forma4",0);
    }
    else
    {
        positions=positionsMax;
        shipWantsPosition=shipWantsPositionMax;
        shipGetsPosition=shipGetsPositionMax;
        positionTaken=positionTakenMax;
    }

    optimumSel->numShips = numShips;

    formationArrangeOptimum(formationcommand);
    biggestToSmallestRatio = selection->ShipPtr[numShips-1]->staticinfo->staticheader.staticCollInfo.collspheresize/selection->ShipPtr[0]->staticinfo->staticheader.staticCollInfo.collspheresize;
    if(biggestToSmallestRatio > BIG_SMALL_RATIO_TO_BE_EQUAL_FOR_LEADER)
    {
        redo=TRUE;
/*
        for(i=0;i<numShips;i++)
        {
            vecAddTo(center,selection->ShipPtr[i]->posinfo.position);
        }
        tempdist = 1.0f/numShips;
        vecScalarMultiply(center,center,tempdist);
        minDist = REALlyBig;
        for(i=0;i<numShips;i++)
        {
            vecSub(tempvec,selection->ShipPtr[i]->posinfo.position,center);
            tempdist = vecMagnitudeSquared(tempvec);
            if(tempdist < minDist)
            {
                minDist = tempdist;
                shipwhogetsit = i;
            }
        }

        ship = selection->ShipPtr[0];
        selection->ShipPtr[0] = selection->ShipPtr[shipwhogetsit];
        selection->ShipPtr[shipwhogetsit] = ship;
*/
    }

    //find center ship...i.e. find leader!

    //assume ships are in the correct order of sizeness
    //and the leader has been picked!  I.E. position 0 is secure!

    //go through all leaders and use optimal version IF redo is true

    originalSel->numShips = numShips;
    for(a=0;a<numShips;a++)
    {
        originalSel->ShipPtr[a] = selection->ShipPtr[a];
    }

    mindistleader = REALlyBig;

    for(a=0;a<numShips;a++)
    {
        //reassemble original list
        for(p=0;p<numShips;p++)
        {
            selection->ShipPtr[p] = originalSel->ShipPtr[p];
        }
        //use a as leader
        selection->ShipPtr[0] = selection->ShipPtr[a];
        selection->ShipPtr[a] = originalSel->ShipPtr[0];

        FormationCalculateOffsets(formationcommand);
        coordsys = &selection->ShipPtr[0]->rotinfo.coordsys;
        for(i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            matMultiplyMatByVec(&positions[i],coordsys,&ship->formationOffset);
            vecAddTo(positions[i],selection->ShipPtr[0]->posinfo.position);

            // This line doesn't do anything but it was probably intended to do something...
            // I've left it here in case someone debugging this code needs this as part of the solution!
            //selection->ShipPtr[i]->formationOffset;

            shipWantsPosition[i]=0;
            shipGetsPosition[i]=0;
            positionTaken[i] = FALSE;
        }

        //positions,posiitiontaken,shipwantsposiiton initialized
        //find position that ship WANTS
        for(i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            minDist = REALlyBig;
            for(j=1;j<numShips;j++)
            {
                vecSub(tempvec,ship->posinfo.position,positions[j]);
                tempdist = vecMagnitudeSquared(tempvec);
                if(tempdist < minDist)
                {
                    //new best point
                    minDist = tempdist;
                    shipWantsPosition[i] = j;
                }
            }
        }

        //shipWantsPosition full of points that each ship WANTS to go to
        //now go through list and assign positions in
        for(i=1;i<numShips;i++)
        {
            poswanted = shipWantsPosition[i];
            if(poswanted == 0)
                continue;
            numWhoWant = 1;
            //see if someone else wants this position too...
            for(j=1;j<numShips;j++)
            {
                if(j==i)
                    continue;
                if(shipWantsPosition[j] == poswanted)
                {
                    //contention exists for point poswanted
                    numWhoWant++;
                }
            }
            if(numWhoWant == 1)
            {
                //no contention
                shipGetsPosition[i] = poswanted;
                positionTaken[poswanted] = TRUE;
            }
            else
            {
                //numWhoWant > 1 hence contention
                //must resolve conflict
                minDist = REALlyBig;
                for(j=1;j<numShips;j++)
                {
                    ship = selection->ShipPtr[j];
                    if(shipWantsPosition[j] == poswanted)
                    {
                        vecSub(tempvec,ship->posinfo.position,positions[poswanted]);
                        tempdist = vecMagnitudeSquared(tempvec);
                        if(tempdist < minDist)
                        {
                            minDist = tempdist;
                            shipwhogetsit = j;
                        }
                    }
                }
                //shipwhogetsit is the shipindex of the ship that won the point
                for(j=1;j<numShips;j++)
                {
                    if(shipWantsPosition[j] == poswanted &&
                       j != shipwhogetsit)
                    {
                        shipWantsPosition[j] = 0;   //set so we know to fix it up later
                        shipGetsPosition[j] = 0;
                    }
                }
                shipGetsPosition[shipwhogetsit] = poswanted;
                positionTaken[poswanted] = TRUE;
            }
        }
        //now match up free ships with closest free slots
        for(i=1;i<numShips;i++)
        {
            if(shipWantsPosition[i] == 0)
            {
                //ship needs a position
                minDist = REALlyBig;
                ship = selection->ShipPtr[i];
                for(j=1;j<numShips;j++)
                {
                    if(positionTaken[j] == FALSE)
                    {
                        vecSub(tempvec,ship->posinfo.position,positions[j]);
                        tempdist = vecMagnitudeSquared(tempvec);
                        if(tempdist < minDist)
                        {
                            minDist = tempdist;
                            shipwhogetsit = j;      //use as tempvar
                        }
                    }
                }
                shipWantsPosition[i] = shipwhogetsit;   //set so we know to fix it up later
                shipGetsPosition[i] = shipwhogetsit;
                positionTaken[shipwhogetsit] = TRUE;
            }
        }

        testSel->ShipPtr[0] = selection->ShipPtr[0];
        for(i=1;i<numShips;i++)
        {
        #ifdef HW_DEBUG
            dbgAssert(shipGetsPosition[i] != 0);
        #endif
            testSel->ShipPtr[shipGetsPosition[i]] = selection->ShipPtr[i];
        }
        for(i=0;i<numShips;i++)
        {
            selection->ShipPtr[i]=testSel->ShipPtr[i];
        }
        //selection has been re-organized correctly...so now lets
        //recalculate the proper offsets and start drinking
        FormationCalculateOffsets(formationcommand);

        tempdist3 = calculateTravelDistance(formationcommand);
        if(tempdist3 < mindistleader)
        {
            mindistleader = tempdist3;
            for(i=0;i<numShips;i++)
            {
                optimumSel->ShipPtr[i] = selection->ShipPtr[i];
            }
        }

        if (!redo)
        {
            break;
        }
    }

/*    optimumSel->ShipPtr[0] = selection->ShipPtr[0];
    for(i=1;i<numShips;i++)
    {
#ifdef HW_DEBUG
        dbgAssert(shipGetsPosition[i] != 0);
#endif
        optimumSel->ShipPtr[shipGetsPosition[i]] = selection->ShipPtr[i];
    }

  */
    for(i=0;i<numShips;i++)
    {
        selection->ShipPtr[i]=optimumSel->ShipPtr[i];
    }
    //selection has been re-organized correctly...so now lets
    //recalculate the proper offsets and start drinking
    FormationCalculateOffsets(formationcommand);

    //MAYBE make the big/small arangement here...then do this?

    memFree(optimumSel);
    memFree(testSel);
    memFree(originalSel);

    if (numShips > COMMAND_MAX_SHIPS-1)
    {
        memFree(positions);
        memFree(shipWantsPosition);
        memFree(shipGetsPosition);
        memFree(positionTaken);
    }

}

/*-----------------------------------------------------------------------------
    Name        : FillInFormationSpecifics
    Description : fills in the formation details
    Inputs      : formationcommand
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FillInFormationSpecifics(CommandToDo *formationcommand,TypeOfFormation formation,sdword newformation)
{
    sdword i;
    SelectCommand *selection = formationcommand->selection;
    sdword numShips = selection->numShips;

    // fill in formation specifics:
    formationcommand->formation.formationtype = formation;
    formationcommand->formation.tacticalState = TightFormation;       //default behavior
    formationcommand->formation.formationLocked = FALSE;                //initialize a formation as unlocked
    formationcommand->formation.tacticsUpdate = 0.0f;                 //this formation needs updating
    formationcommand->formation.flipselectionfixed = FALSE;
    formationcommand->formation.needFix = FALSE;
    formationcommand->formation.enders = FALSE;
    formationcommand->formation.percentmaxspeed = FORMATION_TRAVELVEL_MINSCALE;
    setFormationTravelVelocity(formationcommand);
    formationcommand->formation.flagTravelSlowestShip = FORMATION_TRAVEL_SLOWEST_SHIP;  // obsolete flag, but set anyway
    formationcommand->formation.doneInitialAttack = FALSE;
    formationcommand->formation.sortorder = 0;
    formationcommand->formation.pulse = 0;

    if(newformation)
    {
        //formation group is new so set this properly
        formationcommand->formation.flipState = 0;
    }

    for (i=0;i<numShips;i++)
    {
        FillInShipFormationStuff(selection->ShipPtr[i],formationcommand);
    }

//    formationArrangeOptimum(formationcommand);
//    FormationCalculateOffsets(formationcommand);

    formationArrageCrazyOptimum(formationcommand);

    switch (formation)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case SPHERE_FORMATION:
        case CUSTOM_FORMATION:
            formationcommand->formation.formationSpecificInfo = NULL;
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

typedef struct UniqueObjAndFreq
{
    void *obj;
    sdword freq;
} UniqueObjAndFreq;

typedef struct CountFreqObjs
{
    sdword numUniqueObjs;
    UniqueObjAndFreq *uniqueObjFreqs;
} CountFreqObjs;

void uofInit(CountFreqObjs *countFreqObjs,sdword max)
{
    countFreqObjs->numUniqueObjs = 0;
    countFreqObjs->uniqueObjFreqs = memAlloc(sizeof(UniqueObjAndFreq)*max,"uniqueobjfreq",Pyrophoric);
}

void uofClose(CountFreqObjs *countFreqObjs)
{
    memFree(countFreqObjs->uniqueObjFreqs);
}

void uofAddObj(CountFreqObjs *countFreqObjs,void *obj)
{
    sdword i;

    for (i=0;i<countFreqObjs->numUniqueObjs;i++)
    {
        if (obj == countFreqObjs->uniqueObjFreqs[i].obj)
        {
             countFreqObjs->uniqueObjFreqs[i].freq++;
             return;
        }
    }

    countFreqObjs->uniqueObjFreqs[countFreqObjs->numUniqueObjs].obj = obj;
    countFreqObjs->uniqueObjFreqs[countFreqObjs->numUniqueObjs].freq = 1;
    countFreqObjs->numUniqueObjs++;
}

void *uofFindObjWithMaxFreq(CountFreqObjs *countFreqObjs)
{
    sdword i;
    sdword max = -1;
    sdword maxindex = -1;

    for (i=0;i<countFreqObjs->numUniqueObjs;i++)
    {
        if (countFreqObjs->uniqueObjFreqs[i].freq > max)
        {
            maxindex = i;
            max = countFreqObjs->uniqueObjFreqs[i].freq;
        }
    }

    if (maxindex != -1)
    {
        return countFreqObjs->uniqueObjFreqs[maxindex].obj;
    }

    return NULL;
}


/*-----------------------------------------------------------------------------
    Name        : MajorityShipsAreDoing
    Description : returns what majority of ships are doing (attacking or moving)
    Inputs      : selectcom
    Outputs     : if COMMAND_MOVE returned, outputs movingto
                  if COMMAND_ATTACK returned, outputs attacking.  In this case you must
                    do a growSelectClose on attacking.
    Return      : returns what majority of ships are doing (attacking or moving)
----------------------------------------------------------------------------*/
sdword MajorityShipsAreDoing(SelectCommand *selectcom,vector *movingto,GrowSelection *attacking)
{
    sdword numShips = selectcom->numShips;
    sdword i;
    Ship *ship;
    sdword nummoving = 0;
    sdword numattacking = 0;

    // first pass: are any ships moving or attacking?
    for (i=0;i<numShips;i++)
    {
        ship = selectcom->ShipPtr[i];
        if (ship->command)
        {
            if (ship->command->ordertype.order == COMMAND_ATTACK)
            {
                numattacking++;
            }
            else if (ship->command->ordertype.order == COMMAND_MOVE)
            {
                nummoving++;
            }
        }
    }

    if (numattacking && nummoving)
    {
        return 0;       // ambiguous - some attacking, some moving
    }

    if (numattacking > (numShips - numattacking))
    {
        if (AreAllShipsAttackCapable(selectcom))
        {
            // amalgamate a list of targets everyone is going to attack in formation
            growSelectInit(attacking);

            for (i=0;i<numShips;i++)
            {
                ship = selectcom->ShipPtr[i];
                if ((ship->command) && (ship->command->ordertype.order == COMMAND_ATTACK))
                {
                    SelectCommand *attacksel = (SelectCommand *)ship->command->attack;
                    sdword numTargets = attacksel->numShips;
                    sdword j;
                    for (j=0;j<numTargets;j++)
                    {
                        if (!ShipInSelection(selectcom,attacksel->ShipPtr[j]))      // in case force attacking
                        {
                            growSelectAddShipNoDuplication(attacking,attacksel->ShipPtr[j]);
                        }
                    }
                }
            }

            if (attacking->selection->numShips == 0)
            {
                growSelectClose(attacking);
                return 0;
            }

            return COMMAND_ATTACK;
        }
        else
        {
            return 0;       // not all ships attack capable - therefore do not attack
        }
    }
    else if (nummoving > (numShips - nummoving))
    {
        if (AreShipsMobile(selectcom))
        {
            // find a tovector the majority of ships are moving to
            CountFreqObjs countFreqObjs;
            CommandToDo *mostfreqmoveto;

            uofInit(&countFreqObjs,nummoving);

            for (i=0;i<numShips;i++)
            {
                ship = selectcom->ShipPtr[i];
                if ((ship->command) && (ship->command->ordertype.order == COMMAND_MOVE))
                {
                    uofAddObj(&countFreqObjs,ship->command);
                }
            }

            mostfreqmoveto = uofFindObjWithMaxFreq(&countFreqObjs);
            uofClose(&countFreqObjs);

            if (!mostfreqmoveto)
            {
                return 0;
            }

            *movingto = mostfreqmoveto->move.destination;

            return COMMAND_MOVE;
        }
        else
        {
            return 0;       // not all ships are mobile - therefore do not move
        }
    }
    else
    {
        return 0;           // no one attacking or moving
    }
}

/*-----------------------------------------------------------------------------
    Name        : clFormation
    Description : Command to form a formation
    Inputs      : selectcom, formation
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation)
{
    CommandToDo *newcommand;
    CommandToDo *alreadyformation;
    SelectCommand *selection;
    udword sizeofselection;
    sdword majorityshipsare;
    vector majorityshipsmoveto;
    GrowSelection majorityshipsattacking;

    if (!((formation >= 0) && (formation < NO_FORMATION)))
    {
        dbgFatalf(DBG_Loc,"Invalid formation %d",formation);
    }

    if (selectcom->numShips < ABSOLUTE_MIN_SHIPS_IN_FORMATION)
    {
#ifdef DEBUG_FORMATIONS
        dbgMessage("\nNot enough ships to do a formation");
#endif
        return;
    }

    //treat cleanup like a halt!
    salCapExtraSpecialOrderCleanUp(selectcom,COMMAND_HALT,NULL,NULL);



    if ((alreadyformation = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (alreadyformation->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            if (alreadyformation->formation.formationtype == formation)
            {
#ifdef DEBUG_FORMATIONS
        dbgMessage("\nAlready gave that formation order");
#endif
                return;
            }
            else
            {
                if (alreadyformation->formation.formationSpecificInfo != NULL)
                {
                    memFree(alreadyformation->formation.formationSpecificInfo);
                    alreadyformation->formation.formationSpecificInfo = NULL;
                }

                alreadyformation->formation.formationtype = formation;
                formationTypeHasChanged(alreadyformation);

                PrepareShipsForCommand(alreadyformation,TRUE);
#ifdef DEBUG_FORMATIONS
        dbgMessage("\nChanging formation");
#endif
                return;
            }
        }
        else if (alreadyformation->ordertype.order == COMMAND_MOVE)
        {
            // Change from normal move command to move in formation
            alreadyformation->ordertype.attributes |= COMMAND_IS_FORMATION;
            FillInFormationSpecifics(alreadyformation,formation,TRUE);
            PrepareShipsForCommand(alreadyformation,TRUE);
            return;
        }
        else if (alreadyformation->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            alreadyformation->ordertype.attributes |= COMMAND_IS_FORMATION;
            FillInFormationSpecifics(alreadyformation,formation,TRUE);
            PrepareShipsForCommand(alreadyformation,TRUE);
            return;
        }
        else if (alreadyformation->ordertype.order == COMMAND_ATTACK)
        {
            alreadyformation->ordertype.attributes |= COMMAND_IS_FORMATION;
            FillInFormationSpecifics(alreadyformation,formation,TRUE);
            PrepareShipsForCommand(alreadyformation,TRUE);
            return;
        }
    }

    majorityshipsare = MajorityShipsAreDoing(selectcom,&majorityshipsmoveto,&majorityshipsattacking);

#ifdef DEBUG_FORMATIONS
    dbgMessage("\nReceived Order to do formation");
#endif

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(selectcom->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_NULL;
    newcommand->ordertype.attributes = COMMAND_IS_FORMATION;

    FillInFormationSpecifics(newcommand,formation,TRUE);

    InitShipsForAI(selection,TRUE);

    PrepareShipsForCommand(newcommand,TRUE);
    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);

    if (majorityshipsare == COMMAND_MOVE)
    {
        ChangeOrderToMove(newcommand,selectcom->ShipPtr[0]->posinfo.position,majorityshipsmoveto);
    }
    else if (majorityshipsare == COMMAND_ATTACK)
    {
        ChangeOrderToAttack(newcommand,(SelectAnyCommand *)majorityshipsattacking.selection);
        growSelectClose(&majorityshipsattacking);
    }
}

/*=============================================================================
    The following routines are for the military parades ships form when they are launched from a ship
=============================================================================*/

typedef struct
{
    real32 upoffset;
    real32 rightoffset;
    real32 headingoffset;
    real32 upspacing;
    real32 rightspacing;
    real32 headingspacing;
    udword facedirection;
} SlotInfo;

typedef struct
{
    SlotInfo slotinfos[MAX_MILITARY_SLOTS];
} ParadeTypeInfo;

#define PARADE_R1MOTHERSHIP     0
#define PARADE_R2MOTHERSHIP     1
#define PARADE_R1CARRIER        2
#define PARADE_R2CARRIER        3
#define PARADE_P1MOTHERSHIP     4
#define PARADE_P2MOTHERSHIP     5
#define NUMBER_PARADE_TYPES     6

#define SUBSLOT_BATCH      10      // subslots are allocated in batches of this many

static ParadeTypeInfo paradeTypeInfos[NUMBER_PARADE_TYPES];

void scriptSetSlotInfoCB(char *directory,char *field,void *dataToFillIn);

scriptStructEntry ParadeInfoScriptTable[] =
{
    { "SlotPositionInfo[SLOT_Fighter0]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter0], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter1]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter1], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter2]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter2], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter3]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter3], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter4]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter4], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter5]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter5], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Corvette0]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Corvette0], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Corvette1]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Corvette1], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Corvette2]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Corvette2], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Corvette3]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Corvette3], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Frigate]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Frigate], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_ResCollector]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_ResCollector], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_ResourceController]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_ResourceController], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Destroyer]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Destroyer], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_HeavyCruiser]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_HeavyCruiser], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_NonCombat]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_NonCombat], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Carrier]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Carrier], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SensorArray]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SensorArray], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_ResearchShip]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_ResearchShip], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SupportCorvette]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SupportCorvette], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P1Fighter]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P1Fighter], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P1IonArrayFrigate]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P1IonArrayFrigate], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P1MissileCorvette]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P1MissileCorvette], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P1StandardCorvette]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P1StandardCorvette], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2Swarmer0]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2Swarmer0], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2Swarmer1]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2Swarmer1], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2Swarmer2]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2Swarmer2], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2Swarmer3]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2Swarmer3], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2AdvanceSwarmer0]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2AdvanceSwarmer0], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2AdvanceSwarmer1]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2AdvanceSwarmer1], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2AdvanceSwarmer2]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2AdvanceSwarmer2], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2AdvanceSwarmer3]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2AdvanceSwarmer3], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2FuelPod0]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2FuelPod0], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2FuelPod1]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2FuelPod1], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2FuelPod2]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2FuelPod2], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2FuelPod3]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2FuelPod3], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2MultiBeamFrigate0]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2MultiBeamFrigate0], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2MultiBeamFrigate1]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2MultiBeamFrigate1], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2MultiBeamFrigate2]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2MultiBeamFrigate2], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P2MultiBeamFrigate3]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P2MultiBeamFrigate3], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Misc]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Misc], (udword)&paradeTypeInfos[0] },
    { NULL,NULL,0,0 }
};

void scriptSetSlotInfoCB(char *directory,char *field,void *dataToFillIn)
{
    SlotInfo *slotInfo = (SlotInfo *)dataToFillIn;

    RemoveCommasFromString(field);

    slotInfo->facedirection = 0;
    sscanf(field,"%f %f %f %f %f %f %d",&slotInfo->upoffset,&slotInfo->rightoffset,&slotInfo->headingoffset,&slotInfo->upspacing,&slotInfo->rightspacing,&slotInfo->headingspacing,&slotInfo->facedirection);
}

void paradeSetTweakables()
{
    memset(&paradeTypeInfos[PARADE_R1MOTHERSHIP],0,sizeof(ParadeTypeInfo));
    memset(&paradeTypeInfos[PARADE_R2MOTHERSHIP],0,sizeof(ParadeTypeInfo));
    memset(&paradeTypeInfos[PARADE_R1CARRIER],0,sizeof(ParadeTypeInfo));
    memset(&paradeTypeInfos[PARADE_R2CARRIER],0,sizeof(ParadeTypeInfo));
    memset(&paradeTypeInfos[PARADE_P1MOTHERSHIP],0,sizeof(ParadeTypeInfo));
    memset(&paradeTypeInfos[PARADE_P2MOTHERSHIP],0,sizeof(ParadeTypeInfo));
    scriptSetStruct(NULL,"Parade_R1Mothership.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_R1MOTHERSHIP]);
    scriptSetStruct(NULL,"Parade_R2Mothership.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_R2MOTHERSHIP]);
    scriptSetStruct(NULL,"Parade_Carrier.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_R1CARRIER]);
    scriptSetStruct(NULL,"Parade_R2Carrier.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_R2CARRIER]);
    scriptSetStruct(NULL,"Parade_P1Mothership.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_P1MOTHERSHIP]);
    scriptSetStruct(NULL,"Parade_P2Mothership.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_P2MOTHERSHIP]);
}

/*-----------------------------------------------------------------------------
    Name        : processMilitaryParadeToDo
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void processMilitaryParadeToDo(struct CommandToDo *command,bool passiveAttacked)
{
    MilitaryParadeCommand *militaryParade = command->militaryParade;
    Ship *leader = militaryParade->aroundShip;
    sdword paradeType = militaryParade->paradeType;
    ParadeTypeInfo *paradeTypeInfo = &paradeTypeInfos[paradeType];
    vector up,right,heading;
    vector faceheading;
    sdword slot;
    sdword subslot;
    MilitarySlot *thisslot;
    SlotInfo *thisslotinfo;
    vector positionat;
    vector positionchange;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    bool dontrotate;
    bool firstShipOfSlot;
    bool steadyFormation = TRUE;

    dbgAssert(paradeType >= 0);
    dbgAssert(paradeType < NUMBER_PARADE_TYPES);

    matGetVectFromMatrixCol1(up,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol2(right,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);

    if (vecMagnitudeSquared(leader->posinfo.velocity) >= PARADE_CONSIDERED_STEADY_VELSQR)
    {
        steadyFormation = FALSE;
    }

    // for each slot, position the ship correctly

    for (slot=0;slot<MAX_MILITARY_SLOTS;slot++)
    {
        thisslot = militaryParade->militarySlots[slot];

        if (thisslot->highestSlotUsed < 0)
        {
            continue;
        }

        thisslotinfo = &paradeTypeInfo->slotinfos[slot];

        // calculate starting positonat of this slot
        positionat = leader->posinfo.position;
        vecAddToScalarMultiply(positionat,up,thisslotinfo->upoffset);
        vecAddToScalarMultiply(positionat,right,thisslotinfo->rightoffset);
        vecAddToScalarMultiply(positionat,heading,thisslotinfo->headingoffset);

        vecScalarMultiply(positionchange,right,thisslotinfo->rightspacing);
        vecAddToScalarMultiply(positionchange,up,thisslotinfo->upspacing);              // added by MG July 23
        vecAddToScalarMultiply(positionchange,heading,thisslotinfo->headingspacing);    //

        switch (thisslotinfo->facedirection)
        {
            case 0:
                faceheading = heading;
                break;
            case 1:
                vecCopyAndNegate(faceheading,heading);
                break;
            case 2:
                faceheading = right;
                break;
            case 3:
                vecCopyAndNegate(faceheading,right);
                break;
            default:
                dbgAssert(FALSE);
                break;
        }

        dbgAssert(thisslot->highestSlotUsed < thisslot->numSubslots);

        firstShipOfSlot = TRUE;

        for (subslot=0;subslot<=thisslot->highestSlotUsed;subslot++)
        {
            ship = thisslot->subslots[subslot].ship;
            if (ship != NULL)
            {
                shipstatic = (ShipStaticInfo *)ship->staticinfo;
                dontrotate = (passiveAttacked & (bool)shipstatic->rotateToRetaliate);
                if(ship->shiptype == ResearchShip)
                {
                    //special case for research ship
                    CommandToDo *leaderCommand = getShipAndItsCommand(&universe.mainCommandLayer,leader);
                    dontrotate = TRUE;  //default to NOT ROTATING in parade
                    if(leaderCommand != NULL)
                    {
                        if(leaderCommand->ordertype.order == COMMAND_MOVE)
                        {
                            //let it rotate here.
                            dontrotate = FALSE;
                        }
                    }
                }
                if (firstShipOfSlot)
                {
                    firstShipOfSlot = FALSE;
                }
                else
                {
                    ship->shipidle = FALSE;       // only set non-first-ship-of-slot's shipidle to FALSE
                }

                if (dontrotate)
                {
                    if (steadyFormation && MoveReachedDestinationVariable(ship,&positionat,100.0f))
                    {
                        ship->flags |= SOF_DontDrawTrails;
                    }
                    if (!MoveReachedDestinationVariable(ship,&positionat,10.0f))
                    {
                        if(ship->shiptype == ResearchShip)
                        {
                            if(ship->flags & SOF_Slaveable)
                            {
                                real32 tmpdistaway = MoveLeftToGo(ship,&positionat);
                                if(tmpdistaway < 50.0 || (tmpdistaway <300.0f && !bitTest(ship->specialFlags,SPECIAL_ParadeNeedTomoveCloser)))
                                {
                                    //less that 50, or less than 300, but we are have been doing this already
                                    bitClear(ship->specialFlags,SPECIAL_ParadeNeedTomoveCloser);
                                    aitrackSteadyShipDriftOnly(ship);
                                    goto dontmovecloser;
                                }
                                else
                                {
                                    //greater than 50.0f && we're driving for 50, set flag, and then drive in
                                    bitSet(ship->specialFlags,SPECIAL_ParadeNeedTomoveCloser);
                                }
                            }
                        }
                        aishipFlyToPointAvoidingObjsWithVel(ship,&positionat,AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity);
                    }
                    else if (steadyFormation)
                    {
                        aitrackSteadyShipDriftOnly(ship);
                    }
                }
                else
                {
                    real32 tmpdistaway = MoveLeftToGo(ship,&positionat);
                    udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible;
                    bool trackheading = FALSE;

                    if (thisslotinfo->facedirection)
                    {
                        trackheading = TRUE;        // if facing nonstandard direction, always track heading
                    }

                    if (isCapitalShipStaticOrBig(shipstatic))
                    {
                        if (tmpdistaway <= 500.0f)
                        {
                            if ((tmpdistaway <= 100.0f) && (steadyFormation)) ship->flags |= SOF_DontDrawTrails;
                            trackheading = TRUE;
                        }
                    }
                    else if (tmpdistaway <= 100.0f)
                    {
                        if (steadyFormation)
                        {
                            ship->flags |= SOF_DontDrawTrails;
                            //no choice special case ResearchShip code
                            if(ship->shiptype == ResearchShip)
                            {
                                //researchship, and we're in a 'steady' state
                                //lets skip the adjustment of our heading
                                if(ship->flags & SOF_Slaveable)
                                {
                                    goto dontadjustheading;
                                }
                            }
                        }

                        trackheading = TRUE;
dontadjustheading:;
                    }

                    if (trackheading)
                    {
                        aitrackHeadingWithBank(ship,&(faceheading),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);
                        tmpuseflags = AISHIP_FastAsPossible;
                    }

                    if (tmpdistaway >= 10.0f)
                    {
                        if(ship->shiptype == ResearchShip)
                        {
                            if(ship->flags & SOF_Slaveable)
                            {
                                if(tmpdistaway < 50.0 || (tmpdistaway <300.0f && !bitTest(ship->specialFlags,SPECIAL_ParadeNeedTomoveCloser)))
                                {
                                    //less that 50, or less than 300, but we are have been doing this already
                                    bitClear(ship->specialFlags,SPECIAL_ParadeNeedTomoveCloser);
                                    aitrackSteadyShipDriftOnly(ship);
                                    goto dontmovecloser;
                                }
                                else
                                {
                                    //greater than 50.0f && we're driving for 50, set flag, and then drive in
                                    bitSet(ship->specialFlags,SPECIAL_ParadeNeedTomoveCloser);
                                }
                            }
                        }
                        aishipFlyToPointAvoidingObjsWithVel(ship,&positionat,tmpuseflags,0.0f,&leader->posinfo.velocity);
dontmovecloser:;
                    }
                    else if (steadyFormation)
                    {
                        aitrackSteadyShipDriftOnly(ship);
                    }
                }
            }

            vecAddTo(positionat,positionchange);
        }
    }
}

bool shipInMilitaryParade(ShipPtr ship)
{
    CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
    if (command == NULL)
    {
        return FALSE;
    }
    if (command->ordertype.order == COMMAND_MILITARYPARADE)
    {
        return TRUE;
    }
    return FALSE;
}

void setMilitaryParade(struct CommandToDo *command)
{
    MilitaryParadeCommand *militaryParade = command->militaryParade;
    Ship *leader = militaryParade->aroundShip;
    sdword paradeType = militaryParade->paradeType;
    ParadeTypeInfo *paradeTypeInfo = &paradeTypeInfos[paradeType];
    vector up,right,heading;
    sdword slot;
    sdword subslot;
    MilitarySlot *thisslot;
    SlotInfo *thisslotinfo;
    vector positionat;
    vector positionchange;
    Ship *ship;

    dbgAssert(paradeType >= 0);
    dbgAssert(paradeType < NUMBER_PARADE_TYPES);

    matGetVectFromMatrixCol1(up,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol2(right,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);

    // for each slot, position the ship correctly

    for (slot=0;slot<MAX_MILITARY_SLOTS;slot++)
    {
        thisslot = militaryParade->militarySlots[slot];

        if (thisslot->highestSlotUsed < 0)
        {
            continue;
        }

        thisslotinfo = &paradeTypeInfo->slotinfos[slot];

        // calculate starting positonat of this slot
        positionat = leader->posinfo.position;
        vecAddToScalarMultiply(positionat,up,thisslotinfo->upoffset);
        vecAddToScalarMultiply(positionat,right,thisslotinfo->rightoffset);
        vecAddToScalarMultiply(positionat,heading,thisslotinfo->headingoffset);

        vecScalarMultiply(positionchange,right,thisslotinfo->rightspacing);
        vecAddToScalarMultiply(positionchange,up,thisslotinfo->upspacing);              // added by MG July 23
        vecAddToScalarMultiply(positionchange,heading,thisslotinfo->headingspacing);    //

        dbgAssert(thisslot->highestSlotUsed < thisslot->numSubslots);

        for (subslot=0;subslot<=thisslot->highestSlotUsed;subslot++)
        {
            ship = thisslot->subslots[subslot].ship;
            if (ship != NULL)
            {
                ship->posinfo.position = positionat;
                switch (thisslotinfo->facedirection)
                {
                    vector temp1,temp2;

                    case 0:     // forward
                        ship->rotinfo.coordsys = leader->rotinfo.coordsys;
                        break;

                    case 1:     // back
                        vecCopyAndNegate(temp1,heading);
                        vecCopyAndNegate(temp2,right);
                        matCreateMatFromVecs(&ship->rotinfo.coordsys,&up,&temp2,&temp1);
                        break;

                    case 2:     // right
                        vecCopyAndNegate(temp1,heading);
                        matCreateMatFromVecs(&ship->rotinfo.coordsys,&up,&temp1,&right);
                        break;

                    case 3:     // left
                        vecCopyAndNegate(temp2,right);
                        matCreateMatFromVecs(&ship->rotinfo.coordsys,&up,&heading,&temp2);
                        break;

                    default:
                        dbgAssert(FALSE);
                        break;
                }
                univUpdateObjRotInfo((SpaceObjRot *)ship);
            }

            vecAddTo(positionat,positionchange);
        }
    }
}

void FreeMilitaryParadeContents(MilitaryParadeCommand *militaryParade)
{
    sdword i;

    for (i=0;i<MAX_MILITARY_SLOTS;i++)
    {
        dbgAssert(militaryParade->militarySlots[i]);
        memFree(militaryParade->militarySlots[i]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : GetShipSlot
    Description : returns the correct slot a ship belongs to
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword GetShipSlot(Ship *ship)
{
    switch(ship->staticinfo->shiptype)
    {
      case LightDefender:
            return SLOT_Fighter0;
        case HeavyDefender:
            return SLOT_Fighter1;
        case LightInterceptor:
            return SLOT_Fighter2;
        case HeavyInterceptor:
            return SLOT_Fighter3;
        case AttackBomber:
            return SLOT_Fighter4;
        case CloakedFighter:
        case DefenseFighter:
            return SLOT_Fighter5;
       case LightCorvette:
            return SLOT_Corvette0;
        case HeavyCorvette:
            return SLOT_Corvette1;
        case MultiGunCorvette:
            return SLOT_Corvette2;
        case MinelayerCorvette:
            return SLOT_Corvette3;

        case RepairCorvette:
        case SalCapCorvette:
            return SLOT_SupportCorvette;
        case ResourceController:
            return SLOT_ResourceController;

        case CloakGenerator:
        case GravWellGenerator:
            return SLOT_NonCombat;

        case ResourceCollector:
            return SLOT_ResCollector;

        case StandardDestroyer:
            return SLOT_Destroyer;

        case HeavyCruiser:
            return SLOT_HeavyCruiser;

        case SensorArray:
            return SLOT_SensorArray;
        case ResearchShip:
            return SLOT_ResearchShip;
        case P1Fighter:
            return SLOT_P1Fighter;
        case P1IonArrayFrigate:
            return SLOT_P1IonArrayFrigate;
        case P1MissileCorvette:
            return SLOT_P1MissileCorvette;
        case P1StandardCorvette:
            return SLOT_P1StandardCorvette;
        case P2Swarmer:
            return (SLOT_P2Swarmer0 + (ship->shipID.shipNumber & 3));
        case P2AdvanceSwarmer:
            return (SLOT_P2AdvanceSwarmer0 + (ship->shipID.shipNumber & 3));
        case P2FuelPod:
            return (SLOT_P2FuelPod0 + (ship->shipID.shipNumber & 3));
        case P2MultiBeamFrigate:
            return (SLOT_P2MultiBeamFrigate0 + (ship->shipID.shipNumber & 3));
        default:
            switch(ship->shiprace)
            {
            case R1:
            case R2:
                switch(ship->staticinfo->shipclass)
                {
                case CLASS_Fighter:
                    return SLOT_Fighter5;
                case CLASS_Corvette:
                    return SLOT_Corvette3;
                case CLASS_Frigate:
                    return SLOT_Frigate;
                case CLASS_Resource:
                    return SLOT_ResCollector;
                case CLASS_Destroyer:
                    return SLOT_Destroyer;
                case CLASS_HeavyCruiser:
                    return SLOT_HeavyCruiser;
                case CLASS_NonCombat:
                    return SLOT_NonCombat;
                case CLASS_Carrier:
                    return SLOT_Carrier;
                }
            }
        return SLOT_Misc;
    }

/*

    switch(ship->staticinfo->shiprace)
    {
        case R1:
        case R2:
            switch (ship->staticinfo->shipclass)
            {
                case CLASS_Fighter:
                    switch (ship->shiptype)
                    {
                        case LightDefender:
                            return SLOT_Fighter0;
                        case HeavyDefender:
                            return SLOT_Fighter1;
                        case LightInterceptor:
                            return SLOT_Fighter2;
                        case HeavyInterceptor:
                            return SLOT_Fighter3;
                        case AttackBomber:
                            return SLOT_Fighter4;
                        case CloakedFighter:
                        case DefenseFighter:
                            return SLOT_Fighter5;

                        default:
                            return SLOT_Fighter5;
                    }

                case CLASS_Corvette:
                    switch (ship->shiptype)
                    {
                        case LightCorvette:
                            return SLOT_Corvette0;
                        case HeavyCorvette:
                            return SLOT_Corvette1;
                        case MultiGunCorvette:
                            return SLOT_Corvette2;
                        case MinelayerCorvette:
                            return SLOT_Corvette3;

                        case RepairCorvette:
                        case SalCapCorvette:
                            return SLOT_SupportCorvette;

                        default:
                            return SLOT_Corvette3;
                    }

                case CLASS_Frigate:
                    switch (ship->shiptype)
                    {
                        case ResourceController:
                            return SLOT_ResourceController;

                        case CloakGenerator:
                        case GravWellGenerator:
                            return SLOT_NonCombat;

                        default:
                            return SLOT_Frigate;
                    }

                case CLASS_Resource:
                    return SLOT_ResCollector;

                case CLASS_Destroyer:
                    return SLOT_Destroyer;

                case CLASS_HeavyCruiser:
                    return SLOT_HeavyCruiser;

                case CLASS_NonCombat:
                    if (ship->shiptype == SensorArray)
                    {
                        return SLOT_SensorArray;
                    }
                    else if (ship->shiptype == ResearchShip)
                    {
                        return SLOT_ResearchShip;
                    }
                    else
                    {
                        return SLOT_NonCombat;
                    }

                case CLASS_Carrier:
                    return SLOT_Carrier;
            }
            return SLOT_Misc;

        case P1:
            switch(ship->shiptype)
            {
                case P1Fighter:
                    return SLOT_P1Fighter;
                case P1IonArrayFrigate:
                    return SLOT_P1IonArrayFrigate;
                case P1MissileCorvette:
                    return SLOT_P1MissileCorvette;
                case P1StandardCorvette:
                    return SLOT_P1StandardCorvette;
            }
            return SLOT_Misc;
        case P2:
            switch(ship->shiptype)
            {
                case P2Swarmer:
                    return (SLOT_P2Swarmer0 + (ship->shipID.shipNumber & 3));
                case P2AdvanceSwarmer:
                    return (SLOT_P2AdvanceSwarmer0 + (ship->shipID.shipNumber & 3));
                case P2FuelPod:
                    return (SLOT_P2FuelPod0 + (ship->shipID.shipNumber & 3));
                case P2MultiBeamFrigate:
                    return (SLOT_P2MultiBeamFrigate0 + (ship->shipID.shipNumber & 3));
            }
            return SLOT_Misc;
        default:
            return SLOT_Misc;
    }
    */
}

void CalculateNewHighestUsedSlot(MilitarySlot *militarySlot)
{
    sdword i;

    for (i=militarySlot->numSubslots-1;i>=0;i--)
    {
        if (militarySlot->subslots[i].ship != NULL)
        {
            militarySlot->highestSlotUsed = i;
            return;
        }
    }
    militarySlot->highestSlotUsed = -1;
}

void RemoveShipFromMilitaryParade(Ship *shiptoremove,MilitaryParadeCommand *militaryParade)
{
    sdword slot;
    sdword subslot;
    MilitarySlot *militarySlot;

    for (slot=0;slot<MAX_MILITARY_SLOTS;slot++)
    {
        militarySlot = militaryParade->militarySlots[slot];

        dbgAssert(militarySlot->highestSlotUsed < militarySlot->numSubslots);

        for (subslot=0;subslot<=militarySlot->highestSlotUsed;subslot++)
        {
            if (militarySlot->subslots[subslot].ship == shiptoremove)
            {
                // found shiptoremove
                militarySlot->subslots[subslot].ship = NULL;
                CalculateNewHighestUsedSlot(militarySlot);
                return;
            }
        }
    }

    dbgAssert(FALSE);       // should never reach here
}

/*-----------------------------------------------------------------------------
    Name        : PutShipInMilitaryParade
    Description : puts a ship into the correct position in a militaryParade
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PutShipInMilitaryParade(Ship *ship,MilitaryParadeCommand *militaryParade)
{
    sdword slot;
    MilitarySlot *militarySlot;
    sdword i;
    sdword paradeType = militaryParade->paradeType;
    ParadeTypeInfo *paradeTypeInfo = &paradeTypeInfos[paradeType];
    SlotInfo *slotinfo;

    sdword numSubslotsToAllocate;
    sdword sizeofNewSlot;
    MilitarySlot *newMilitarySlot;

    slot = GetShipSlot(ship);
    dbgAssert(slot >= 0);
    dbgAssert(slot < MAX_MILITARY_SLOTS);
    slotinfo = &paradeTypeInfo->slotinfos[slot];
    if ((slotinfo->upoffset == 0.0f) && (slotinfo->rightoffset == 0.0f) && (slotinfo->headingoffset == 0.0f))
    {
        // this slot isn't defined well, perhaps a captured ship in another race's parade?  Put in MiscSlot
        slot = SLOT_Misc;
    }
    militarySlot = militaryParade->militarySlots[slot];

    // find first empty subslot
    for (i=0;i<militarySlot->numSubslots;i++)
    {
        if (militarySlot->subslots[i].ship == NULL)
        {
            // found a empty slot
            militarySlot->subslots[i].ship = ship;
            CalculateNewHighestUsedSlot(militarySlot);
            return;
        }
    }

    // did not find a empty slot.  All full.  We should allocate more slots.

    numSubslotsToAllocate = militarySlot->numSubslots + SUBSLOT_BATCH;
    sizeofNewSlot = sizeofMilitarySlot(numSubslotsToAllocate);
    newMilitarySlot = memAlloc(sizeofNewSlot,"newmilslot",0);

    newMilitarySlot->numSubslots = numSubslotsToAllocate;
    for (i=0;i<militarySlot->numSubslots;i++)
    {
        newMilitarySlot->subslots[i] = militarySlot->subslots[i];
    }
    dbgAssert(i == militarySlot->numSubslots);
    for ( ;i<numSubslotsToAllocate;i++)
    {
        newMilitarySlot->subslots[i].ship = NULL;
    }

    // put ship in:
    newMilitarySlot->subslots[militarySlot->numSubslots].ship = ship;
    newMilitarySlot->highestSlotUsed = militarySlot->numSubslots;

    memFree(militarySlot);
    militaryParade->militarySlots[slot] = newMilitarySlot;
}

/*-----------------------------------------------------------------------------
    Name        : AddShipToMilitaryGroup
    Description : adds a ship to a military group command
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddShipToMilitaryGroup(ShipPtr ship,struct CommandToDo *militaryGroup)
{
    AddShipToGroup(ship,militaryGroup);

    dbgAssert(militaryGroup->ordertype.order == COMMAND_MILITARYPARADE);

    PutShipInMilitaryParade(ship,militaryGroup->militaryParade);
}

/*-----------------------------------------------------------------------------
    Name        : CreateMilitaryGroupAroundShip
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
struct CommandToDo *CreateMilitaryGroupAroundShip(struct CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip)
{
    CommandToDo *newcommand;
    sdword i,j;
    MilitarySlot *militarySlot;
    SelectCommand *selection;
    MilitaryParadeCommand *militaryParade;

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    selection = memAlloc(sizeofSelectCommand(1),"ToDoSelection",NonVolatile);
    selection->numShips = 1;
    selection->ShipPtr[0] = ship;

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_MILITARYPARADE;
    newcommand->ordertype.attributes = 0;

    newcommand->militaryParade = militaryParade = memAlloc(sizeof(MilitaryParadeCommand),"milparadecom",0);
    militaryParade->aroundShip = aroundShip;
    for (i=0;i<MAX_MILITARY_SLOTS;i++)
    {
        militarySlot = memAlloc(sizeofMilitarySlot(SUBSLOT_BATCH),"milslot",0);
        militarySlot->numSubslots = SUBSLOT_BATCH;
        militarySlot->highestSlotUsed = -1;
        for (j=0;j<SUBSLOT_BATCH;j++)
        {
            militarySlot->subslots[j].ship = NULL;
        }
        militaryParade->militarySlots[i] = militarySlot;
    }

    militaryParade->paradeType = -1;
    if (aroundShip->shiprace == R1)
    {
        if (aroundShip->shiptype == Mothership)
        {
            militaryParade->paradeType = PARADE_R1MOTHERSHIP;
        }
        else if (aroundShip->shiptype == Carrier)
        {
            militaryParade->paradeType = PARADE_R1CARRIER;
        }
    }
    else if (aroundShip->shiprace == R2)
    {
        if (aroundShip->shiptype == Mothership)
        {
            militaryParade->paradeType = PARADE_R2MOTHERSHIP;
        }
        else if (aroundShip->shiptype == Carrier)
        {
            militaryParade->paradeType = PARADE_R2CARRIER;
        }
    }
    else if (aroundShip->shiprace == P1)
    {
        militaryParade->paradeType = PARADE_P1MOTHERSHIP;
    }
    else if (aroundShip->shiprace == P2)
    {
        militaryParade->paradeType = PARADE_P2MOTHERSHIP;
    }

    dbgAssert(militaryParade->paradeType != -1);

    PutShipInMilitaryParade(ship,militaryParade);

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);

    return newcommand;
}

void lockFormation(CommandToDo *formationcommand,udword specialEffect)
{
    //copy leader coordinate system...
    //maybe...but probably don't want to relock formations over and over
    vector heading;
    //dbgAssert(formationcommand->formation.formationLocked == FALSE);
    formationcommand->formation.coordsys = formationcommand->selection->ShipPtr[0]->rotinfo.coordsys;
    if(specialEffect == 1)
    {
        //reverse Heading
        matGetVectFromMatrixCol3(heading,formationcommand->formation.coordsys);
        vecScalarMultiply(heading,heading,-1.0f);
        matPutVectIntoMatrixCol3(heading,formationcommand->formation.coordsys);
    }
    formationcommand->formation.formationLocked = TRUE;
}

void unlockFormation(CommandToDo *formationcommand)
{
    //copy leader coordinate system...
    //maybe...but probably don't want to unlock formations over and over
    //it shows we don't know what we're doing :)
    //dbgAssert(formationcommand->formation.formationLocked == TRUE);
    formationcommand->formation.formationLocked = FALSE;
}


