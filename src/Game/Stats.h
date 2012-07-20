/*=============================================================================
    Name    : Stats.h
    Purpose : Definitions for Stats.c

    Created 4/1/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "ShipDefs.h"
#include "SpaceObj.h"
#include "ShipSelect.h"

#define NUM_SHIPS_TO_GATHER_STATS_FOR   (TOTAL_STD_SHIPS+TOTAL_STD_SHIPS+TOTAL_P1_SHIPS+TOTAL_P2_SHIPS+TOTAL_P3_SHIPS)

typedef struct FightStats
{
    ubyte shiptype[2];
    ubyte shiprace[2];

    ubyte numShips[2];
    ubyte numShipsAfter[2];
    real32 totalHPAfter[2];

//    real32 numFracShipsAfter[2];
//    real32 RUratio;        // numRUsDead[1] / numRUsDead[0]
//    real32 Killratio;      // numShipsDied[1] / numShipsDied[0]
    real32 fracRUratio;    // same as above, but uses fractional ships
    real32 fracKillratio;  // same as above, but uses fractional ships
    real32 battleTime;     // length in seconds of battle

} FightStats;

typedef struct FightStatsSum
{
    real32 RUratio;
    real32 Killratio;
} FightStatsSum;

#define FightStatsCalculated(fightstats) (fightstats.numShips[0])
#define FightStatsPtrCalculated(fightstats) (fightstats->numShips[0])

#define MAX_SHIPS_TO_EVER_CONSIDER      30

extern FightStats FightStatsTable[NUM_SHIPS_TO_GATHER_STATS_FOR][NUM_SHIPS_TO_GATHER_STATS_FOR];

extern FightStatsSum FightStatsColumnSum[NUM_SHIPS_TO_GATHER_STATS_FOR];
extern FightStatsSum FightStatsRowSum[NUM_SHIPS_TO_GATHER_STATS_FOR];

extern bool ShowFancyFights;

/*=============================================================================
    Functions:
=============================================================================*/

void statsGatherFightStats(void);
void statsLoadFightStats(void);
void statsPrintTable(void);
void statsShowFight(sdword i,sdword j);
void statsShowFancyFight(sdword i,sdword j);
void statsShowFancyFightUpdate(void);

void statsSetOverkillfactor(real32 factor);

real32 statsGetShipRURatingAgainstShip(ShipStaticInfo *thisShip,ShipStaticInfo *againstShip);
real32 statsGetShipKillRatingAgainstShip(ShipStaticInfo *thisShip,ShipStaticInfo *againstShip);
real32 statsGetOverallRURating(ShipStaticInfo *shipstatic);
real32 statsGetOverallKillRating(ShipStaticInfo *shipstatic);

typedef bool (*statShipConstraintsCB)(ShipStaticInfo *shipstatic);
//typedef bool (*ShipConstraintsCB)(Ship *ship);
bool ShipConstraintsNoneCB(Ship *ship);
bool statShipConstraintsNoneCB(ShipStaticInfo *shipstatic);
bool statShipConstraintsFrigatesOrWorseCB(ShipStaticInfo *shipstatic);
bool statShipConstraintsFrigatesOrBetterCB(ShipStaticInfo *shipstatic);
bool statShipConstraintsFightingShipsCB(ShipStaticInfo *shipstatic);
bool statShipConstraintsCarrierFightingShipsCB(ShipStaticInfo *shipstatic);

real32 statsGetKillRatingAgainstFleet(ShipStaticInfo *shipstatic,SelectCommand *fleet);
real32 statsGetRURatingAgainstFleet(ShipStaticInfo *shipstatic,SelectCommand *fleet);
Ship *statsGetMostDangerousShipNonStatConstraints(SelectCommand *selection,ShipConstraintsCB constraintsCB);
#define statsGetMostDangerousShip(sel) statsGetMostDangerousShipNonStatConstraints(sel,ShipConstraintsNoneCB)

real32 statsGetKillRatingAgainstFleetStatic(ShipStaticInfo *shipstatic,SelectCommandStatic *fleetStatic);
real32 statsGetRURatingAgainstFleetStatic(ShipStaticInfo *shipstatic,SelectCommandStatic *fleetStatic);
ShipStaticInfo *statsGetMostDangerousShipStaticConstraints(SelectCommandStatic *selection,statShipConstraintsCB constraintsCB);
#define statsGetMostDangerousShipStatic(sel) statsGetMostDangerousShipStaticConstraints(sel,statShipConstraintsNoneCB)

ShipStaticInfo *statsBestShipToBuyToKillShip(ShipRace shipRace,statShipConstraintsCB constraintsCB,ShipStaticInfo *targetstatic);
ShipStaticInfo *statsBestShipToBuyToKillFleet(ShipRace shipRace,statShipConstraintsCB constraintsCB,SelectCommand *targetFleet);
ShipStaticInfo *statsBestShipToBuyToKillFleetStatic(ShipRace shipRace,statShipConstraintsCB constraintsCB,SelectCommandStatic *targetFleetStatic);
Ship *statsBestShipToUseToKillShip(SelectCommand *freeShips,ShipStaticInfo *targetstatic);
Ship *statsBestShipToUseToKillFleet(SelectCommand *freeShips,SelectCommand *targetFleet);

sdword statsNumShipsNeededToKillTarget(ShipStaticInfo *shipstatic,ShipStaticInfo *targetstatic);
sdword statsNumShipsNeededToKillFleet(ShipStaticInfo *shipstatic,SelectCommand *targetFleet);
sdword statsNumShipsNeededToKillFleetStatic(ShipStaticInfo *shipstatic,SelectCommandStatic *targetFleetStatic);
// strength of fleet2 against fleet1, e.g. fleet2/fleet1
real32 statsGetRelativeFleetStrengths(SelectCommand *fleet1,SelectCommand *fleet2);
// strength of ship against fleet strength, e.g. targetstatic strength / fleet1 strength
real32 statsGetRelativeFleetStrengthAgainstShip(SelectCommand *fleet1,ShipStaticInfo *targetstatic);

SelectCommand *statsBestShipsToUseToKillTarget(SelectCommand *freeships,ShipStaticInfo *targetstatic,bool *goodEnough);
SelectCommand *statsBestShipsToUseToKillFleet(SelectCommand *freeships,SelectCommand *targetFleet,bool *goodEnough);

// returns a selection of ships which are efficient for killer to kill
// efficiencyFactor should be < 1, and the lower it is, the more ships killer will think it can take out
SelectCommand *statsGetMostEfficientShipsToKill(ShipStaticInfo *killer,SelectCommand *fleet,real32 efficiencyFactor);



/*=============================================================================
    Cheat detection:
=============================================================================*/
// returns a checksum of the statistics gathered by the game
/*real32 statsGetStatsChecksum();*/
udword statsGetStatChecksum();

