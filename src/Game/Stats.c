/*=============================================================================
    Name    : Stats.c
    Purpose : Stats deals with collecting fighting stats for the various ships

    Created 4/1/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <stdarg.h>
#include "Types.h"
#include "SpaceObj.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "File.h"
#include "Tweak.h"
#include "Stats.h"
#include "FastMath.h"
#include "SinglePlayer.h"
#include "CRC32.h"
#include "ResearchAPI.h"
#include "NetCheck.h"

#define STATLOG_FILENAME "statlog.txt"

#define CONSIDER_SHIPS_EVEN_PERC_HP_LEFT 0.15f

#define REFRESH     5

real32 statsOverkillfactor = 1.0f;
real32 statsOverkillfactorSqr = 1.0f;

TypeOfFormation showStatsFightF[2];
TacticsType showStatsFightT[2];
bool ShowFancyFights = FALSE;

static CommandToDo *attackcommands[2];

FightStats FightStatsTable[NUM_SHIPS_TO_GATHER_STATS_FOR][NUM_SHIPS_TO_GATHER_STATS_FOR];

FightStatsSum FightStatsColumnSum[NUM_SHIPS_TO_GATHER_STATS_FOR];
FightStatsSum FightStatsRowSum[NUM_SHIPS_TO_GATHER_STATS_FOR];

// R1,R2,P1-3
#define NUM_RACES_TO_GATHER_STATS_FOR     5

sdword StatIndexRaceOffsets[NUM_RACES_TO_GATHER_STATS_FOR] =
{
    0,
    TOTAL_STD_SHIPS,
    TOTAL_STD_SHIPS+TOTAL_STD_SHIPS,
    TOTAL_STD_SHIPS+TOTAL_STD_SHIPS+TOTAL_P1_SHIPS,
    TOTAL_STD_SHIPS+TOTAL_STD_SHIPS+TOTAL_P1_SHIPS+TOTAL_P2_SHIPS
};

static sbyte CalcFightStatsForRaces[NUM_RACES_TO_GATHER_STATS_FOR][NUM_RACES_TO_GATHER_STATS_FOR] =
{   //  R1   R2    P1    P2    P3
    { TRUE, TRUE, TRUE, TRUE, TRUE },    // R1
    { TRUE, TRUE, TRUE, TRUE, TRUE },    // R2
    { TRUE, TRUE, TRUE, FALSE,FALSE},    // P1
    { TRUE, TRUE, FALSE,TRUE, FALSE},    // P2
    { TRUE, TRUE, FALSE,FALSE,TRUE },    // P3
};

static sbyte R1CalcFightStatsFor[TOTAL_STD_SHIPS] =
{
    /* R1 */
    TRUE,   //AdvanceSupportFrigate
    TRUE,   //AttackBomber
    TRUE,   //Carrier
    TRUE,   //CloakedFighter
    FALSE,  //CloakGenerator
    TRUE,   //DDDFrigate
    FALSE,  //DefenseFighter
    FALSE,  //DFGFrigate
    FALSE,  //GravWellGenerator
    TRUE,   //HeavyCorvette
    TRUE,   //HeavyCruiser
    TRUE,   //HeavyDefender
    TRUE,   //HeavyInterceptor
    TRUE,   //IonCannonFrigate
    TRUE,   //LightCorvette
    FALSE,  //LightDefender
    TRUE,   //LightInterceptor
    TRUE,   //MinelayerCorvette
    TRUE,   //MissileDestroyer
    TRUE,   //Mothership
    TRUE,   //MultiGunCorvette
    FALSE,  //Probe
    FALSE,  //ProximitySensor
    TRUE,   //RepairCorvette
    FALSE,  //ResearchShip
    FALSE,  //ResourceCollector
    FALSE,  //ResourceController
    FALSE,  //SalCapCorvette
    FALSE,  //SensorArray
    TRUE,   //StandardDestroyer
    TRUE,   //StandardFrigate
    FALSE,  //Drone
    FALSE,  //TargetDrone
    FALSE,  //HeadShotAsteroid
    FALSE   //CryoTray
};

static sbyte R2CalcFightStatsFor[TOTAL_STD_SHIPS] =
{
    /* R2 */
    TRUE,   //AdvanceSupportFrigate
    TRUE,   //AttackBomber
    TRUE,   //Carrier
    FALSE,  //CloakedFighter
    FALSE,  //CloakGenerator
    FALSE,  //DDDFrigate
    FALSE,  //DefenseFighter
    FALSE,  //DFGFrigate
    FALSE,  //GravWellGenerator
    TRUE,   //HeavyCorvette
    TRUE,   //HeavyCruiser
    TRUE,   //HeavyDefender
    TRUE,   //HeavyInterceptor
    TRUE,   //IonCannonFrigate
    TRUE,   //LightCorvette
    FALSE,  //LightDefender
    TRUE,   //LightInterceptor
    TRUE,   //MinelayerCorvette
    TRUE,   //MissileDestroyer
    TRUE,   //Mothership
    TRUE,   //MultiGunCorvette
    FALSE,  //Probe
    FALSE,  //ProximitySensor
    TRUE,   //RepairCorvette
    FALSE,  //ResearchShip
    FALSE,  //ResourceCollector
    FALSE,  //ResourceController
    FALSE,  //SalCapCorvette
    FALSE,  //SensorArray
    TRUE,   //StandardDestroyer
    TRUE,   //StandardFrigate
    FALSE,  //Drone
    FALSE,  //TargetDrone
    FALSE,  //HeadShotAsteroid
    FALSE
};

static sbyte P1CalcFightStatsFor[TOTAL_P1_SHIPS] =
{
    /* P1 */
    TRUE,  //P1Fighter
    TRUE,  //P1IonArrayFrigate
    TRUE,  //P1MissileCorvette
    TRUE,  //P1Mothership
    TRUE   //P1StandardCorvette
};

static sbyte P2CalcFightStatsFor[TOTAL_P2_SHIPS] =
{
    /* P2 */
    TRUE,   //P2AdvanceSwarmer
    FALSE,  //P2FuelPod
    TRUE,   //P2Mothership
    TRUE,   //P2MultiBeamFrigate
    TRUE,   //P2Swarmer
};

static sbyte P3CalcFightStatsFor[TOTAL_P3_SHIPS] =
{
    /* P3 */
    TRUE,   //P3Destroyer
    TRUE,   //P3Frigate
    TRUE,   //P3Megaship
};

static sbyte *RaceCalcFightStatsFor[NUM_RACES_TO_GATHER_STATS_FOR] =
{
    R1CalcFightStatsFor,
    R2CalcFightStatsFor,
    P1CalcFightStatsFor,
    P2CalcFightStatsFor,
    P3CalcFightStatsFor
};

bool ForceTotalRefresh = FALSE;

static void statsTest(void);
static void StatsForRaceCB(char *directory,char *field,void *dataToFillIn);
static void StatsForCB(char *directory,char *field,void *dataToFillIn);

scriptEntry GatherStatsScriptTable[] =
{
    makeEntry(ForceTotalRefresh,scriptSetBool),
    {"StatsForRace", StatsForRaceCB, NULL},
    {"StatsFor",     StatsForCB, NULL},
    endEntry
};

static void StatsForRaceCB(char *directory,char *field,void *dataToFillIn)
{
    char *param;
    ShipRace race;
    sbyte val;
    sdword i;

    param = strtok(field, ", \t");
    dbgAssert(param);
    race = StrToShipRace(param);
    dbgAssert(race >= 0 && race < NUM_RACES_TO_GATHER_STATS_FOR);

    for (i=0;i<NUM_RACES_TO_GATHER_STATS_FOR;i++)
    {
        param = strtok(NULL, ", \t");
        val = FALSE;
        if (param)
        {
            if (strncmp(param,"TRUE",4) == 0)
            {
                val = TRUE;
            }
            else if (strncmp(param,"REFRESH",7) == 0)
            {
                val = REFRESH;
            }
        }

        CalcFightStatsForRaces[race][i] = val;
    }
}

static void StatsForCB(char *directory,char *field,void *dataToFillIn)
{
    char racestr[15];
    char shipstr[50];
    char truestr[15];
    ShipRace race;
    ShipType shiptype;
    sbyte val;

    RemoveCommasFromString(field);

    sscanf(field,"%s %s %s",racestr,shipstr,truestr);

    race = StrToShipRace(racestr);
    dbgAssert(race >= 0 && race < NUM_RACES_TO_GATHER_STATS_FOR);

    shiptype = StrToShipType(shipstr);
    dbgAssert(shiptype >= 0 && shiptype < TOTAL_NUM_SHIPS);

    val = FALSE;
    if (strncmp(truestr,"TRUE",4) == 0)
    {
        val = TRUE;
    }
    else if (strncmp(truestr,"REFRESH",7) == 0)
    {
        val = REFRESH;
    }

    RaceCalcFightStatsFor[race][shiptype - FirstShipTypeOfRace[race]] = val;
}

void statLog(char *format, ...)
{
    char buffer[200];
    va_list argList;
    va_start(argList, format);                              //get first arg
    vsprintf(buffer, format, argList);                      //prepare output string
    va_end(argList);

    dbgMessage("\nSTATLOG:");
    logfileLog(STATLOG_FILENAME,buffer);
    dbgMessage(buffer);
}


sdword ConvertShipRaceTypeToStatIndex(ShipType shiptype,ShipRace shiprace)
{
    sdword offset;

    if ((shiprace < 0) || (shiprace >= NUM_RACES_TO_GATHER_STATS_FOR))
    {
        return -1;
    }

    if ((shiptype < 0) || (shiptype >= TOTAL_NUM_SHIPS))
    {
        return -1;
    }

    offset = shiptype - FirstShipTypeOfRace[shiprace];

    if ((offset < 0) || (offset >= NumShipTypesInRace[shiprace]))
    {
        return -1;
    }

    return (offset + StatIndexRaceOffsets[shiprace]);
}

void ConvertStatIndexToShipRaceType(sdword index,ShipType *shiptype,ShipRace *shiprace)
{
    sdword j;
    sdword offset;

    for (j=NUM_RACES_TO_GATHER_STATS_FOR-1;j>=0;j--)
    {
        if (index >= StatIndexRaceOffsets[j])
        {
            offset = index - StatIndexRaceOffsets[j];
            dbgAssert(offset >= 0);
            dbgAssert(offset < NumShipTypesInRace[j]);

            *shiprace = j;
            *shiptype = (ShipType) (offset + FirstShipTypeOfRace[j]);
            return;
        }
    }

    dbgFatalf(DBG_Loc,"Bad stats index %d",index);
}

ShipStaticInfo *ConvertStatIndexToShipStatic(sdword i)
{
    ShipType shiptype;
    ShipRace shiprace;
    ShipStaticInfo *shipstatic;

    ConvertStatIndexToShipRaceType(i,&shiptype,&shiprace);
    shipstatic = GetShipStaticInfo(shiptype,shiprace);
    if (bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded))
    {
        return shipstatic;
    }
    else
    {
        return NULL;
    }
}

#if 0
void GetPrecombatStats(FightStats *fightStats)
{
    ShipStaticInfo *shipstatic0 = GetShipStaticInfoValidate(fightStats->shiptype[0],fightStats->shiprace[0]);
    ShipStaticInfo *shipstatic1 = GetShipStaticInfoValidate(fightStats->shiptype[1],fightStats->shiprace[1]);
    sdword RU0;
    sdword RU1;

    dbgAssert(shipstatic0->shiptype == fightStats->shiptype[0]);
    dbgAssert(shipstatic1->shiptype == fightStats->shiptype[1]);

    // Get # of ships for players 0 and 1 to fight

    RU0 = shipstatic0->buildCost;
    RU1 = shipstatic1->buildCost;

    if (RU0 == RU1)
    {
        fightStats->numShips[0] = (sword)MIN_SHIPS_TO_FIGHT;
        fightStats->numShips[1] = (sword)MIN_SHIPS_TO_FIGHT;
    }
    else
    {
        sdword maxRU;
        sdword minRU;
        sdword maxRUplayer;
        sdword numMinRUShips;
        sdword numMaxRUShips;

        if (RU0 > RU1)
        {
            maxRU = RU0;
            minRU = RU1;
            maxRUplayer = 0;
        }
        else
        {
            maxRU = RU1;
            minRU = RU0;
            maxRUplayer = 1;
        }

        numMinRUShips = maxRU / minRU;
        numMaxRUShips = 1;
        if (numMinRUShips < MIN_SHIPS_TO_FIGHT)
        {
            numMinRUShips = MIN_SHIPS_TO_FIGHT;

            numMaxRUShips = (numMinRUShips * minRU) / maxRU;
        }

        if (maxRUplayer == 0)
        {
            fightStats->numShips[0] = (sword)numMaxRUShips;
            fightStats->numShips[1] = (sword)numMinRUShips;
        }
        else
        {
            fightStats->numShips[0] = (sword)numMinRUShips;
            fightStats->numShips[1] = (sword)numMaxRUShips;
        }
    }

    if (fightStats->numShips[0] > cdLimitCaps[fightStats->shiptype[0]])
    {
        fightStats->numShips[0] = cdLimitCaps[fightStats->shiptype[0]];
    }

    if (fightStats->numShips[1] > cdLimitCaps[fightStats->shiptype[1]])
    {
        fightStats->numShips[1] = cdLimitCaps[fightStats->shiptype[1]];
    }

    dbgAssert(fightStats->numShips[0] > 0);
    dbgAssert(fightStats->numShips[1] > 0);

    dbgMessagef("\n%s %s Vs %s %s\n",ShipRaceToStr(fightStats->shiprace[0]),ShipTypeToStr(fightStats->shiptype[0]),ShipRaceToStr(fightStats->shiprace[1]),ShipTypeToStr(fightStats->shiptype[1]));
    dbgMessagef("\n%d %d",fightStats->numShips[0],fightStats->numShips[1]);

#if 0
    fightStats->numRUs[0] = fightStats->numShips[0] * RU0;
    fightStats->numRUs[1] = fightStats->numShips[1] * RU1;

    fightStats->totalHP[0] = fightStats->numShips[0] * shipstatic0->maxhealth;
    fightStats->totalHP[1] = fightStats->numShips[1] * shipstatic1->maxhealth;
#endif
}
#endif

void SetupShipsForFight(FightStats *fightStats)
{
    sdword i,j;
    SelectCommand *selection[2];
    sdword numShips;
    vector position[2];
    real32 facerotation[2] = { DEG_TO_RAD(0.0f), DEG_TO_RAD(180.0f) };
    Ship *ship;

    vecSet(position[0],0.0f,0.0f,0.0f);
    vecSet(position[1],FACEOFF_DISTANCE,0.0f,0.0f);

    unitCapDisable();

    for (i=0;i<2;i++)
    {
        numShips = (sdword)fightStats->numShips[i];
        dbgAssert(numShips > 0);
        selection[i] = memAlloc(sizeofSelectCommand(numShips),"fightselection",0);

        selection[i]->numShips = numShips;

        for (j=0;j<numShips;j++)
        {
            selection[i]->ShipPtr[j] = ship = univAddShip(fightStats->shiptype[i],fightStats->shiprace[i],&position[i],&universe.players[i],0);
            univRotateObjYaw((SpaceObjRot *)ship,facerotation[i]);
        }

        if (showStatsFancyFight)
        {
            if ((showStatsFightF[i] >= 0) && (showStatsFightF[i] < NO_FORMATION)) clFormation(&universe.mainCommandLayer,selection[i],showStatsFightF[i]);
            clSetTactics(&universe.mainCommandLayer,selection[i],showStatsFightT[i]);
        }
        else
        {
            clFormation(&universe.mainCommandLayer,selection[i],BROAD_FORMATION);
        }
    }

    clPresetShipsToPosition(&universe.mainCommandLayer);

    universeSwitchToPlayer(0);

    if (AreAllShipsAttackCapable(selection[0]))
    {
        attackcommands[0] = clAttackThese(&universe.mainCommandLayer,selection[0],(AttackCommand *)selection[1]);
        dbgAssert(attackcommands[0] != NULL);
    }
    else
    {
        statLog("Warning: Ships %s %s could not attack",ShipRaceToStr(fightStats->shiprace[0]),ShipTypeToStr(fightStats->shiptype[0]));
    }

    if (AreAllShipsAttackCapable(selection[1]))
    {
        attackcommands[1] = clAttackThese(&universe.mainCommandLayer,selection[1],(AttackCommand *)selection[0]);
        dbgAssert(attackcommands[1] != NULL);
    }
    else
    {
        statLog("Warning: Ships %s %s could not attack",ShipRaceToStr(fightStats->shiprace[1]),ShipTypeToStr(fightStats->shiptype[1]));
    }

    memFree(selection[0]);
    memFree(selection[1]);
}

bool AreShipsDoneFighting()
{
    // keep going until an attack is finished
    if (!CommandInCommandLayer(&universe.mainCommandLayer,attackcommands[0]))
    {
        return TRUE;
    }

    if (!CommandInCommandLayer(&universe.mainCommandLayer,attackcommands[1]))
    {
        return TRUE;
    }

    // or the time expires
    if (universe.totaltimeelapsed >= STATFIGHT_TIMEOUT_TIME)
    {
        return TRUE;
    }

    return FALSE;
}

void HaveShipsFight()
{
    universe.totaltimeelapsed = 0.0f;
    universe.univUpdateCounter = 0;

    for (;;)
    {
        univUpdate(UNIVERSE_UPDATE_PERIOD);

        if (AreShipsDoneFighting())
        {
            break;
        }
    }
}

void CalculateRUKillRatios(FightStats *fightStats)
{
    sdword i;
    ShipStaticInfo *shipstatic[2];

    for (i=0;i<2;i++)
    {
        shipstatic[i] = GetShipStaticInfoValidate(fightStats->shiptype[i],fightStats->shiprace[i]);
    }

    fightStats->fracKillratio = ((real32)fightStats->numShips[1]) / ((real32)fightStats->numShips[0]);
    fightStats->fracRUratio = ((real32)(fightStats->numShips[1] * shipstatic[1]->buildCost)) /
                              ((real32)(fightStats->numShips[0] * shipstatic[0]->buildCost));

    statLog(" >%d Vs %d RUratio Killratio %3.3f %3.3f\n",fightStats->numShips[0],fightStats->numShips[1],fightStats->fracRUratio,fightStats->fracKillratio);
}

#if 0
void CalculateRatios(FightStats *fightStats)
{
    sdword i;
    ShipStaticInfo *shipstatic[2];
    sdword numShipsDied[2];
    real32 numFracShipsDied[2];

    for (i=0;i<2;i++)
    {
        shipstatic[i] = GetShipStaticInfoValidate(fightStats->shiptype[i],fightStats->shiprace[i]);
        fightStats->numFracShipsAfter[i] = ((real32)fightStats->totalHPAfter[i]) / shipstatic[i]->maxhealth;
        numShipsDied[i] = fightStats->numShips[i] - fightStats->numShipsAfter[i];
        numFracShipsDied[i] = ((real32)fightStats->numShips[i]) - fightStats->numFracShipsAfter[i];
    }

#if 0
    fightStats->RUratio = ((real32)(numShipsDied[1] * shipstatic[1]->buildCost)) /
                          ((real32)(numShipsDied[0] * shipstatic[0]->buildCost));
    fightStats->Killratio = ((real32)numShipsDied[1]) / ((real32)numShipsDied[0]);
#endif

    if (numFracShipsDied[0] == 0.0f)
    {
        fightStats->fracRUratio = MAX_RU_RATIO;
        fightStats->fracKillratio = MAX_KILL_RATIO;
    }
    else
    {
        fightStats->fracRUratio = ((real32)(numFracShipsDied[1] * shipstatic[1]->buildCost)) /
                                  ((real32)(numFracShipsDied[0] * shipstatic[0]->buildCost));
        fightStats->fracKillratio = numFracShipsDied[1] / numFracShipsDied[0];
    }

    if (fightStats->fracRUratio > MAX_RU_RATIO)
    {
        fightStats->fracRUratio = MAX_RU_RATIO;
    }

    if (fightStats->fracKillratio > MAX_KILL_RATIO)
    {
        fightStats->fracKillratio = MAX_KILL_RATIO;
    }
}
#endif

#if 0
void PrintFightStats(FightStats *fightStats)
{
    filehandle statlogfileFH = fileOpen(STATLOG_FILENAME, FF_AppendMode | FF_TextMode);
    FILE *statlogfile;

    dbgAssert(!fileUsingBigfile(statlogfileFH));
    statlogfile = fileStream(statlogfileFH);

    if (statlogfile)
    {
        fprintf(statlogfile,"STATS for %s %s Vs %s %s\n",ShipRaceToStr(fightStats->shiprace[0]),ShipTypeToStr(fightStats->shiptype[0]),ShipRaceToStr(fightStats->shiprace[1]),ShipTypeToStr(fightStats->shiptype[1]));
        fprintf(statlogfile,"Before: %3d %3d\n",fightStats->numShips[0],fightStats->numShips[1]);
//        fprintf(statlogfile,"After : %3d %3d\n",fightStats->numShipsAfter[0],fightStats->numShipsAfter[1]);
        fprintf(statlogfile,"After: %3.3f %3.3f\n",fightStats->numFracShipsAfter[0],fightStats->numFracShipsAfter[1]);
//        fprintf(statlogfile,"RU,Kill: %3.3f %3.3f\n",fightStats->RUratio,fightStats->Killratio);
        fprintf(statlogfile,"RU,Kill: %3.3f %3.3f %3.3f\n",fightStats->fracRUratio,fightStats->fracKillratio,fightStats->battleTime);

        fileClose(statlogfileFH);
    }
}
#endif

void GetPostCombatStats(FightStats *fightStats)
{
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    SelectCommand *selection;
    sdword numShips;
    sdword i;
    uword playerindex;

    fightStats->numShipsAfter[0] = 0;
    fightStats->numShipsAfter[1] = 0;
    fightStats->totalHPAfter[0] = 0;
    fightStats->totalHPAfter[1] = 0;

    // Go through command layer, and see what ships are left:

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        if ((command == attackcommands[0]) || (command == attackcommands[1]))
        {
            selection = command->selection;
            numShips = selection->numShips;
            dbgAssert(numShips > 0);
            playerindex = selection->ShipPtr[0]->playerowner->playerIndex;
            dbgAssert(playerindex < 2);
            fightStats->numShipsAfter[playerindex] = (uword)numShips;
            for (i=0;i<numShips;i++)
            {
                fightStats->totalHPAfter[playerindex] += selection->ShipPtr[i]->health;
            }
        }

        curnode = curnode->next;
    }

    fightStats->battleTime = universe.totaltimeelapsed;

#if 0
    CalculateRatios(fightStats);
    PrintFightStats(fightStats);
#endif

    universeReset();
    univupdateReset();
    singlePlayerInit();     // for hyperspace
//    clReset(&universe.mainCommandLayer);
//    ccReset(&universe.mainCameraCommand);
//    univupdateCloseAllObjectsAndMissionSpheres();

//    universe.totaltimeelapsed = 0.0f;
//    universe.univUpdateCounter = 0;
}

void GatherFightStatsFor(sdword i,sdword j,bool actuallyDoFight)
{
    FightStats *fightStats = &FightStatsTable[i][j];
    sdword *binsearchtable = NULL;
    sdword bottom,top;
    sdword trynum;
    sdword search,found,verytop;
    sdword powerfulship,nonpowerfulship;
    ShipType shiptype;
    ShipRace shiprace;

    ConvertStatIndexToShipRaceType(i,&shiptype,&shiprace);
    fightStats->shiptype[0] = shiptype;
    fightStats->shiprace[0] = shiprace;
    ConvertStatIndexToShipRaceType(j,&shiptype,&shiprace);
    fightStats->shiptype[1] = shiptype;
    fightStats->shiprace[1] = shiprace;

    if (!actuallyDoFight)
    {
        // later handle this case
    }

    statLog("STATS for %s %s Vs %s %s\n",ShipRaceToStr(fightStats->shiprace[0]),ShipTypeToStr(fightStats->shiptype[0]),ShipRaceToStr(fightStats->shiprace[1]),ShipTypeToStr(fightStats->shiptype[1]));

    // Have first fight 1 on 1, to determine who is more powerful:
    fightStats->numShips[0] = 1;
    fightStats->numShips[1] = 1;

    if (i == j)
    {
        CalculateRUKillRatios(fightStats);
        return;
    }

    SetupShipsForFight(fightStats);
    HaveShipsFight();
    GetPostCombatStats(fightStats);

    if ((fightStats->numShipsAfter[0] | fightStats->numShipsAfter[1]) == 0)
    {
        // simultaneous kill:
        CalculateRUKillRatios(fightStats);
        statLog(" Simultaneous kill\n");
        return;
    }

    if ((fightStats->numShipsAfter[0] == 1) && (fightStats->numShipsAfter[1] == 1))
    {
        // neither ship could kill the other ship:
        fightStats->fracRUratio = 0.0f;
        fightStats->fracKillratio = 0.0f;
        statLog(" Stalemate\n");
        return;
    }

    if (fightStats->numShipsAfter[0] == 1)
    {
        // ship 0 is more powerful

        real32 hpleftperc = ((real32)fightStats->totalHPAfter[0]) /
                            ((real32)GetShipStaticInfoValidate(fightStats->shiptype[0],fightStats->shiprace[0])->maxhealth);

        if (hpleftperc < CONSIDER_SHIPS_EVEN_PERC_HP_LEFT)
        {
            // ship 0 isn't that much more powerful.  Consider these ships even
            CalculateRUKillRatios(fightStats);
            statLog(" Ships roughly equal 0\n");
            return;
        }

        powerfulship = 0;

    }
    else
    {
        // ship 1 is more powerful

        real32 hpleftperc = ((real32)fightStats->totalHPAfter[1]) /
                            ((real32)GetShipStaticInfoValidate(fightStats->shiptype[1],fightStats->shiprace[1])->maxhealth);

        if (hpleftperc < CONSIDER_SHIPS_EVEN_PERC_HP_LEFT)
        {
            // ship 1 isn't that much more powerful.  Consider these ships even
            CalculateRUKillRatios(fightStats);
            statLog(" Ships roughly equal 1\n");
            return;
        }

        powerfulship = 1;
    }
    nonpowerfulship = 1-powerfulship;

    bottom = 1;
    top = cdLimitCaps[fightStats->shiptype[nonpowerfulship]];
    if (top == -1)
    {
        top = cdClassCaps[GetShipStaticInfoValidate(fightStats->shiptype[nonpowerfulship],fightStats->shiprace[nonpowerfulship])->shipclass];
    }
    if (top >= MAX_SHIPS_TO_EVER_CONSIDER)
    {
        top = MAX_SHIPS_TO_EVER_CONSIDER;
    }
    dbgAssert(top >= 1);
    verytop = top;

    binsearchtable = memAlloc(sizeof(udword)*(top+1),"binsearchtable",0);
    memset(binsearchtable,0,sizeof(udword)*(top+1));

    while (top >= bottom)
    {
        trynum = (bottom+top)>>1;

        if (binsearchtable[trynum])
        {
            statLog(" Cache hit\n");

            // decision in cache
            if (binsearchtable[trynum] < 0)
            {
                // trynum is too small
                bottom = trynum+1;
            }
            else
            {
                // trynum is too big
                top = trynum-1;
            }
        }
        else
        {
            // decision not in cache, so execute fight
            fightStats->numShips[powerfulship] = 1;
            fightStats->numShips[nonpowerfulship] = (uword)trynum;

            statLog(" Binsearch %d %d\n",fightStats->numShips[0],fightStats->numShips[1]);

            SetupShipsForFight(fightStats);
            HaveShipsFight();
            GetPostCombatStats(fightStats);

            if (fightStats->numShipsAfter[powerfulship] == 1)
            {
                // powerful ship hasn't been defeated.  Need more ships.
                binsearchtable[trynum] = -1;
                // trynum is too small
                bottom = trynum+1;
            }
            else
            {
                binsearchtable[trynum] = 1;
                // trynum is too big
                top = trynum-1;
            }
        }
    }

    for (found=-1,search=0;search<=verytop;search++)
    {
        if (binsearchtable[search] > 0)
        {
            // lowest number of ships needed to kill
            found = search;
            break;
        }
    }

    memFree(binsearchtable);

    // at this point trynum contains # of ships needed to kill powerfulship

    fightStats->numShips[powerfulship] = 1;
    fightStats->numShips[nonpowerfulship] = (found >= 0) ? found : trynum;

    CalculateRUKillRatios(fightStats);
}

#define tswap(a,b,abtype) \
{                         \
    abtype typetemp;      \
    typetemp = a;         \
    a = b;                \
    b = typetemp;         \
}

void ReciprocateFightStats(FightStats *fightStats)
{
    tswap(fightStats->shiptype[0],fightStats->shiptype[1],ShipType)
    tswap(fightStats->shiprace[0],fightStats->shiprace[1],ShipRace)
    tswap(fightStats->numShips[0],fightStats->numShips[1],sword)
    tswap(fightStats->numShipsAfter[0],fightStats->numShipsAfter[1],sword)
    tswap(fightStats->totalHPAfter[0],fightStats->totalHPAfter[1],real32)
//    tswap(fightStats->numFracShipsAfter[0],fightStats->numFracShipsAfter[1],sword)
    if (fightStats->fracRUratio != 0.0f)
    {
        fightStats->fracRUratio = 1.0f / fightStats->fracRUratio;
    }
    if (fightStats->fracKillratio != 0.0f)
    {
        fightStats->fracKillratio = 1.0f / fightStats->fracKillratio;
    }
}

/*=============================================================================
    FancyFight Stuff
=============================================================================*/

typedef struct {
    sdword repeat;
    sbyte statindex[2];
    sbyte num[2];
    TypeOfFormation formation[2];
    TacticsType tactics[2];
} FancyFightEntry;

#define MAX_NUM_FANCYFIGHTENTRYS 100

sdword numFancyFightEntrys = 0;
FancyFightEntry fancyFightEntry[MAX_NUM_FANCYFIGHTENTRYS];

sdword currentFancyFightEntry = 0;
sdword currentFancyFightRepeatFight = 0;
bool ShowFancyFightsShipsFighting = FALSE;

void SetFlightEntrysCB(char *directory,char *field,void *dataToFillIn);
void SetFlightEntrysPreLoadCB(char *directory,char *field,void *dataToFillIn);

void SetInfoNeededForShipAndRelatedStaticInfo(ShipType type,ShipRace race,bool8 dataToFillIn);
void universeFlagNothingNeeded(void);

scriptEntry FancyFightScriptTable[] =
{
    makeEntry(ShowFancyFights,scriptSetBool),
    { "DoStatsFor", SetFlightEntrysCB, NULL },
    endEntry
};

scriptEntry FancyFightPreLoadScriptTable[] =
{
    { "DoStatsFor", SetFlightEntrysPreLoadCB, NULL },
    endEntry
};

void SetFlightEntrysCB(char *directory,char *field,void *dataToFillIn)
{
    sdword repeat;
    sdword num[2];
    char racestr[2][20];
    char shipstr[2][50];
    char formationstr[2][50];
    char tacticsstr[2][50];

    RemoveCommasFromString(field);

    sscanf(field,"%d %d %s %s %s %s %d %s %s %s %s",&repeat,&num[0],racestr[0],shipstr[0],formationstr[0],tacticsstr[0],
                                                            &num[1],racestr[1],shipstr[1],formationstr[1],tacticsstr[1]);

    dbgAssert(numFancyFightEntrys < MAX_NUM_FANCYFIGHTENTRYS);

    fancyFightEntry[numFancyFightEntrys].repeat = repeat;
    fancyFightEntry[numFancyFightEntrys].statindex[0] = ConvertShipRaceTypeToStatIndex(StrToShipType(shipstr[0]),StrToShipRace(racestr[0]));
    fancyFightEntry[numFancyFightEntrys].statindex[1] = ConvertShipRaceTypeToStatIndex(StrToShipType(shipstr[1]),StrToShipRace(racestr[1]));
    fancyFightEntry[numFancyFightEntrys].num[0] = num[0];
    fancyFightEntry[numFancyFightEntrys].num[1] = num[1];
    fancyFightEntry[numFancyFightEntrys].formation[0] = StrToTypeOfFormation(formationstr[0]);
    fancyFightEntry[numFancyFightEntrys].formation[1] = StrToTypeOfFormation(formationstr[1]);
    fancyFightEntry[numFancyFightEntrys].tactics[0] = StrToTacticsType(tacticsstr[0]);
    fancyFightEntry[numFancyFightEntrys].tactics[1] = StrToTacticsType(tacticsstr[1]);
    numFancyFightEntrys++;
}

void SetFlightEntrysPreLoadCB(char *directory,char *field,void *dataToFillIn)
{
    sdword repeat;
    sdword num[2];
    char racestr[2][20];
    char shipstr[2][50];
    char formationstr[2][50];
    char tacticsstr[2][50];

    RemoveCommasFromString(field);

    sscanf(field,"%d %d %s %s %s %s %d %s %s %s %s",&repeat,&num[0],racestr[0],shipstr[0],formationstr[0],tacticsstr[0],
                                                            &num[1],racestr[1],shipstr[1],formationstr[1],tacticsstr[1]);

    SetInfoNeededForShipAndRelatedStaticInfo(StrToShipType(shipstr[0]),StrToShipRace(racestr[0]),TRUE);
    SetInfoNeededForShipAndRelatedStaticInfo(StrToShipType(shipstr[1]),StrToShipRace(racestr[1]),TRUE);
}

void FancyFightPreLoad(void)
{
    universeFlagNothingNeeded();
    scriptSet(NULL,showStatsFancyFightScriptFile,FancyFightPreLoadScriptTable);
}

void FancyFightPrepareFightStats(FightStats *fightStats)
{
    FancyFightEntry *ffe = &fancyFightEntry[currentFancyFightEntry];
    ShipType shiptype;
    ShipRace shiprace;
    sdword i = ffe->statindex[0];
    sdword j = ffe->statindex[1];
    dbgAssert(i >= 0);
    dbgAssert(i < NUM_SHIPS_TO_GATHER_STATS_FOR);
    dbgAssert(j >= 0);
    dbgAssert(j < NUM_SHIPS_TO_GATHER_STATS_FOR);

    ConvertStatIndexToShipRaceType(i,&shiptype,&shiprace);
    fightStats->shiptype[0] = shiptype;
    fightStats->shiprace[0] = shiprace;
    ConvertStatIndexToShipRaceType(j,&shiptype,&shiprace);
    fightStats->shiptype[1] = shiptype;
    fightStats->shiprace[1] = shiprace;

    fightStats->numShips[0] = ffe->num[0];
    fightStats->numShips[1] = ffe->num[1];

    showStatsFightF[0] = ffe->formation[0];
    showStatsFightF[1] = ffe->formation[1];
    showStatsFightT[0] = ffe->tactics[0];
    showStatsFightT[1] = ffe->tactics[1];
}

void PrintFancyFightStats(FightStats *fightStats)
{
    statLog("STATS for %s %s Vs %s %s\n",ShipRaceToStr(fightStats->shiprace[0]),ShipTypeToStr(fightStats->shiptype[0]),ShipRaceToStr(fightStats->shiprace[1]),ShipTypeToStr(fightStats->shiptype[1]));
    statLog("Before: %3d %3d\n",fightStats->numShips[0],fightStats->numShips[1]);
    statLog("After : %3d %3d\n",fightStats->numShipsAfter[0],fightStats->numShipsAfter[1]);
}

void statsShowFancyFight(sdword i,sdword j)
{
    FightStats fightStats;

    // initialize fancyfight stats
    ShowFancyFights = FALSE;        // set defaults
    numFancyFightEntrys = 0;
    currentFancyFightEntry = 0;
    currentFancyFightRepeatFight = 0;
    scriptSet(NULL,showStatsFancyFightScriptFile,FancyFightScriptTable);

    logfileClear(STATLOG_FILENAME);
    statLog("GATHERING FANCYFIGHT STATS\n");

    dbgAssert(universe.numPlayers >= 2);    // need at least 2 players to gather stats

    if (!ShowFancyFights)
    {
        for (currentFancyFightEntry=0;currentFancyFightEntry<numFancyFightEntrys;currentFancyFightEntry++)
        {
            sdword numRepeat = fancyFightEntry[currentFancyFightEntry].repeat;
            for (currentFancyFightRepeatFight=0;currentFancyFightRepeatFight<numRepeat;currentFancyFightRepeatFight++)
            {
                FancyFightPrepareFightStats(&fightStats);
                SetupShipsForFight(&fightStats);
                HaveShipsFight();
                GetPostCombatStats(&fightStats);
                PrintFancyFightStats(&fightStats);
            }
        }
    }
    else
    {
        currentFancyFightEntry = 0;
        currentFancyFightRepeatFight = 0;
        dbgAssert(currentFancyFightEntry < numFancyFightEntrys);
        dbgAssert(currentFancyFightRepeatFight < fancyFightEntry[currentFancyFightEntry].repeat);
        ShowFancyFightsShipsFighting = FALSE;
    }
}

void statsShowFancyFightUpdate(void)
{
    static FightStats tempFightStats;

    dbgAssert(ShowFancyFights);

    if (!ShowFancyFightsShipsFighting)
    {
        FancyFightPrepareFightStats(&tempFightStats);
        SetupShipsForFight(&tempFightStats);

        universe.totaltimeelapsed = 0.0f;
        universe.univUpdateCounter = 0;

        ShowFancyFightsShipsFighting = TRUE;
    }
    else
    {
        if (AreShipsDoneFighting())
        {
            GetPostCombatStats(&tempFightStats);
            PrintFancyFightStats(&tempFightStats);
            ShowFancyFightsShipsFighting = FALSE;

            currentFancyFightRepeatFight++;
            if (currentFancyFightRepeatFight >= fancyFightEntry[currentFancyFightEntry].repeat)
            {
                currentFancyFightRepeatFight = 0;

                currentFancyFightEntry++;
                if (currentFancyFightEntry >= numFancyFightEntrys)
                {
                    // all done!
                    ShowFancyFights = FALSE;
                }
            }
        }
    }
}

void statsShowFight(sdword i,sdword j)
{
    FightStats *fightStats;
    dbgAssert(i >= 0);
    dbgAssert(i < NUM_SHIPS_TO_GATHER_STATS_FOR);
    dbgAssert(j >= 0);
    dbgAssert(j < NUM_SHIPS_TO_GATHER_STATS_FOR);

    dbgAssert(universe.numPlayers >= 2);    // need at least 2 players to gather stats

    fightStats = &FightStatsTable[i][j];
    if ((fightStats->numShips[0] > 0) && (fightStats->numShips[1] > 0))
    {
        SetupShipsForFight(fightStats);
    }
    else
    {
        fightStats->numShips[0] = 1;
        fightStats->numShips[1] = 1;
        SetupShipsForFight(fightStats);
    }
}

void CalculateOverallSums(void)
{
    sdword i,j;

    for (i=0;i<NUM_SHIPS_TO_GATHER_STATS_FOR;i++)
    {
        FightStatsColumnSum[i].RUratio = 0.0f;
        FightStatsColumnSum[i].Killratio = 0.0f;

        for (j=0;j<NUM_SHIPS_TO_GATHER_STATS_FOR;j++)
        {
            if (FightStatsCalculated(FightStatsTable[i][j]))
            {
                FightStatsColumnSum[i].RUratio += FightStatsTable[i][j].fracRUratio;
                FightStatsColumnSum[i].Killratio += FightStatsTable[i][j].fracKillratio;
            }
        }
    }

    for (j=0;j<NUM_SHIPS_TO_GATHER_STATS_FOR;j++)
    {
        FightStatsRowSum[j].RUratio = 0.0f;
        FightStatsRowSum[j].Killratio = 0.0f;

        for (i=0;i<NUM_SHIPS_TO_GATHER_STATS_FOR;i++)
        {
            if (FightStatsCalculated(FightStatsTable[i][j]))
            {
                FightStatsRowSum[j].RUratio += FightStatsTable[i][j].fracRUratio;
                FightStatsRowSum[j].Killratio += FightStatsTable[i][j].fracKillratio;
            }
        }
    }
}

void GatherFightStatsForRaces(ShipRace racei,ShipRace racej)
{
    sdword i,j;
    sdword indexi,indexj;
    sdword raceiNumShips = NumShipTypesInRace[racei];
    sbyte *raceiCalcFightStatsFor = RaceCalcFightStatsFor[racei];
    sdword raceiStatIndexOffset = StatIndexRaceOffsets[racei];
    sdword racejNumShips = NumShipTypesInRace[racej];
    sbyte *racejCalcFightStatsFor = RaceCalcFightStatsFor[racej];
    sdword racejStatIndexOffset = StatIndexRaceOffsets[racej];
#if MEM_ERROR_CHECKING
    sdword memoryFree = memFreeMemGet(&memMainPool);
    sdword memoryLeak;
#endif

    for (i=0;i<raceiNumShips;i++)
    {
        if (raceiCalcFightStatsFor[i])
        {
            for (j=0;j<racejNumShips;j++)
            {
                if (racejCalcFightStatsFor[j])
                {
                    indexi = i+raceiStatIndexOffset;
                    indexj = j+racejStatIndexOffset;
                    if (!FightStatsTable[indexi][indexj].numShips[0])       // check to make sure havent already calculated
                    {
                        GatherFightStatsFor(indexi,indexj,TRUE);
                        dbgAssert(FightStatsTable[indexi][indexj].numShips[0]);
#if MEM_ERROR_CHECKING
                        if (memoryFree != memFreeMemGet(&memMainPool))
                        {
                            memoryLeak = memFreeMemGet(&memMainPool) - memoryFree;
                            memAnalysisCreate();
                            dbgAssert(FALSE);
                        }
#endif
                        if (indexi != indexj)
                        {
                            FightStatsTable[indexj][indexi] = FightStatsTable[indexi][indexj];
                            ReciprocateFightStats(&FightStatsTable[indexj][indexi]);
                        }
                    }
                }
            }
        }
    }
}

void RefreshClearFightStatsTable(void)
{
    ShipRace racei,racej;
    sdword i,j;
    sdword indexi,indexj;

    // for any race vs race sets to refresh, clear them so they get refreshed:

    for (racei=0;racei<NUM_RACES_TO_GATHER_STATS_FOR;racei++)
    {
        for (racej=0;racej<NUM_RACES_TO_GATHER_STATS_FOR;racej++)
        {
            sdword raceiNumShips = NumShipTypesInRace[racei];
            sbyte *raceiCalcFightStatsFor = RaceCalcFightStatsFor[racei];
            sdword raceiStatIndexOffset = StatIndexRaceOffsets[racei];

            sdword racejNumShips = NumShipTypesInRace[racej];
            sbyte *racejCalcFightStatsFor = RaceCalcFightStatsFor[racej];
            sdword racejStatIndexOffset = StatIndexRaceOffsets[racej];

            if (CalcFightStatsForRaces[racei][racej] == REFRESH)
            {
                // force all occurances of racei/racej to 0
                for (i=0;i<raceiNumShips;i++)
                {
                    for (j=0;j<racejNumShips;j++)
                    {
                        indexi = i+raceiStatIndexOffset;
                        indexj = j+racejStatIndexOffset;
                        memset(&FightStatsTable[indexi][indexj],0,sizeof(FightStats));
                    }
                }
            }
            else
            {
                // just force ships with refresh set to 0
                for (i=0;i<raceiNumShips;i++)
                {
                    if (raceiCalcFightStatsFor[i] == REFRESH)
                    {
                        for (j=0;j<racejNumShips;j++)           // force all intersections with it to 0
                        {
                            if (racejCalcFightStatsFor[j])
                            {
                                indexi = i+raceiStatIndexOffset;
                                indexj = j+racejStatIndexOffset;
                                memset(&FightStatsTable[indexi][indexj],0,sizeof(FightStats));
                            }
                        }
                    }
                }

                for (j=0;j<racejNumShips;j++)
                {
                    if (racejCalcFightStatsFor[j] == REFRESH)
                    {
                        for (i=0;i<raceiNumShips;i++)           // force all intersections with it to 0
                        {
                            if (raceiCalcFightStatsFor[i])
                            {
                                indexi = i+raceiStatIndexOffset;
                                indexj = j+racejStatIndexOffset;
                                memset(&FightStatsTable[indexi][indexj],0,sizeof(FightStats));
                            }
                        }
                    }
                }
            }
        }
    }
}

void statsGatherFightStats(void)
{
    ShipRace racei,racej;
    sdword statfilesize = NUM_SHIPS_TO_GATHER_STATS_FOR * NUM_SHIPS_TO_GATHER_STATS_FOR * sizeof(FightStats);

    dbgAssert(universe.numPlayers >= 2);    // need at least 2 players to gather stats

    logfileClear(STATLOG_FILENAME);

    if ((ForceTotalRefresh) || (statfilesize != fileSizeGet("statfile.bin",0)))
    {
        memset(&FightStatsTable[0][0],0,statfilesize);
    }
    else
    {
        fileLoad("statfile.bin",&FightStatsTable[0][0],0);
        RefreshClearFightStatsTable();
    }

    for (racei=0;racei<NUM_RACES_TO_GATHER_STATS_FOR;racei++)
    {
        for (racej=0;racej<NUM_RACES_TO_GATHER_STATS_FOR;racej++)
        {
            if (CalcFightStatsForRaces[racei][racej])
            {
                statLog("GATHERING STATS FOR RACES %s %s\n",ShipRaceToStr(racei),ShipRaceToStr(racej));
                GatherFightStatsForRaces(racei,racej);
                statLog("-------------------------------------\n");

                fileSave("statfile.bin",&FightStatsTable[0][0],statfilesize);
                CalculateOverallSums();
                statsPrintTable();
            }
        }
    }
}

void statsLoadFightStats(void)
{
    sdword correctsize = sizeof(FightStats) * (NUM_SHIPS_TO_GATHER_STATS_FOR * NUM_SHIPS_TO_GATHER_STATS_FOR);
    sdword statfilesize = fileSizeGet("statfile.bin",0);

    if (correctsize != statfilesize)
    {
        dbgMessage("\nWARNING: Stat file incorrect version");
        memset(&FightStatsTable[0][0],0,correctsize);
    }
    else
    {
        fileLoad("statfile.bin",&FightStatsTable[0][0],0);
    }

    CalculateOverallSums();
    statsTest();
}

void statsPrintTableRaceXVsRaceY(FILE *tablefile,ShipRace racei,ShipRace racej)
{
    sdword i,j;
    sdword indexi,indexj;
    sdword raceiNumShips = NumShipTypesInRace[racei];
    sbyte *raceiCalcFightStatsFor = RaceCalcFightStatsFor[racei];
    sdword raceiStatIndexOffset = StatIndexRaceOffsets[racei];
    sdword racejNumShips = NumShipTypesInRace[racej];
    sbyte *racejCalcFightStatsFor = RaceCalcFightStatsFor[racej];
    sdword racejStatIndexOffset = StatIndexRaceOffsets[racej];
    FightStats *fightStats;

    fprintf(tablefile,"\t");
    for (j=0;j<racejNumShips;j++)
    {
        if (racejCalcFightStatsFor[j])
        {
            indexj = j+racejStatIndexOffset;
            fprintf(tablefile,"%dRU\t%dKill\t",indexj,indexj);
        }
    }

    for (i=0;i<raceiNumShips;i++)
    {
        if (raceiCalcFightStatsFor[i])
        {
            indexi = i+raceiStatIndexOffset;
            fprintf(tablefile,"\n%2d %2s %20s\t",indexi,ShipRaceToStr(racei),ShipTypeToStr(i+FirstShipTypeOfRace[racei]));
            for (j=0;j<racejNumShips;j++)
            {
                if (racejCalcFightStatsFor[j])
                {
                    indexj = j+racejStatIndexOffset;
                    fightStats = &FightStatsTable[indexi][indexj];
                    fprintf(tablefile,"%4.3f\t%4.3f\t",fightStats->fracRUratio,fightStats->fracKillratio);
                }
            }
        }
    }
}

void statsPrintTable(void)
{
    ShipRace racei,racej;
    filehandle tablefileFH = fileOpen("stattable.txt", FF_IgnoreBIG | FF_WriteMode | FF_TextMode | FF_UserSettingsPath);
    FILE *tablefile;

    dbgAssert(!fileUsingBigfile(tablefileFH));
    tablefile = fileStream(tablefileFH);

    dbgAssert(tablefile);

    for (racei=0;racei<NUM_RACES_TO_GATHER_STATS_FOR;racei++)
    {
        for (racej=0;racej<NUM_RACES_TO_GATHER_STATS_FOR;racej++)
        {
            if (CalcFightStatsForRaces[racei][racej])
            {
                fprintf(tablefile,"Statistics for %s VS %s\n",ShipRaceToStr(racei),ShipRaceToStr(racej));
                statsPrintTableRaceXVsRaceY(tablefile,racei,racej);
                fprintf(tablefile,"\n\n");
            }
        }
    }

    fileClose(tablefileFH);
}



FightStats *getFightStatsFromShipStatics(ShipStaticInfo *thisShip,ShipStaticInfo *againstShip)
{
    sdword i = ConvertShipRaceTypeToStatIndex(thisShip->shiptype,thisShip->shiprace);
    sdword j = ConvertShipRaceTypeToStatIndex(againstShip->shiptype,againstShip->shiprace);

    if ((i < 0) || (j < 0))
    {
        return NULL;
    }

    return &FightStatsTable[i][j];
}

real32 statsGetShipRURatingAgainstShip(ShipStaticInfo *thisShip,ShipStaticInfo *againstShip)
{
    FightStats *fightStats = getFightStatsFromShipStatics(thisShip,againstShip);

    if (fightStats == NULL)
    {
        return 0.0f;
    }

    return fightStats->fracRUratio;
}

real32 statsGetShipKillRatingAgainstShip(ShipStaticInfo *thisShip,ShipStaticInfo *againstShip)
{
    FightStats *fightStats = getFightStatsFromShipStatics(thisShip,againstShip);

    if (fightStats == NULL)
    {
        return 0.0f;
    }

    return fightStats->fracKillratio;
}

void statsSetOverkillfactor(real32 factor)
{
    statsOverkillfactor = factor;
    statsOverkillfactorSqr = factor*factor;
}

real32 statsGetOverallRURating(ShipStaticInfo *shipstatic)
{
    sdword i = ConvertShipRaceTypeToStatIndex(shipstatic->shiptype,shipstatic->shiprace);

    if (i < 0)
    {
        return 0.0f;
    }

    return FightStatsColumnSum[i].RUratio;
}

real32 statsGetOverallKillRating(ShipStaticInfo *shipstatic)
{
    sdword i = ConvertShipRaceTypeToStatIndex(shipstatic->shiptype,shipstatic->shiprace);

    if (i < 0)
    {
        return 0.0f;
    }

    return FightStatsColumnSum[i].Killratio;
}

bool ShipConstraintsNoneCB(Ship *ship)
{
    return TRUE;
}

bool statShipConstraintsNoneCB(ShipStaticInfo *shipstatic)
{
    return TRUE;
}

bool statShipConstraintsFrigatesOrWorseCB(ShipStaticInfo *shipstatic)
{
    if (shipstatic->shipclass >= CLASS_Frigate)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool statShipConstraintsFrigatesOrBetterCB(ShipStaticInfo *shipstatic)
{
    if (shipstatic->shipclass <= CLASS_Frigate)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : statShipConstraintsFightingShipsCB
    Description : Returns true if the staticinfo passed is a ship that can be
                  considered a "fighting ship" (i.e. has guns, can attack
                  effectively, etc.)
    Inputs      : shipstatic - the staticinfo to check
    Outputs     :
    Return      : TRUE if the ship is a fighting ship
----------------------------------------------------------------------------*/
bool statShipConstraintsFightingShipsCB(ShipStaticInfo *shipstatic)
{
    switch (shipstatic->shipclass)
    {
        case CLASS_Mothership:
        case CLASS_Carrier:
            return FALSE;
        case CLASS_HeavyCruiser:
        case CLASS_Destroyer:
            return TRUE;
            break;
        case CLASS_Frigate:
            if ((shipstatic->shiptype == ResourceController) ||
                (shipstatic->shiptype == DFGFrigate) ||
                (shipstatic->shiptype == AdvanceSupportFrigate))
            {
                return FALSE;
            }
            else
            {
                return TRUE;    // all other Frigates are considered fighting ships
            }
            break;
        case CLASS_Corvette:
            if ((shipstatic->shiptype == RepairCorvette) ||
                (shipstatic->shiptype == SalCapCorvette))
            {
                return FALSE;   //salcap corvette doesn't attack in a conventional manner
            }
            else
            {
                return TRUE;
            }
        case CLASS_Fighter:
            if (shipstatic->shiptype == DefenseFighter)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        case CLASS_Resource:
        case CLASS_NonCombat:
        default:
            return FALSE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : statShipConstraintsCarrierFightingShipsCB
    Description : Returns true if the staticinfo passed is a ship that can be
                  considered a "fighting ship" (i.e. has guns, can attack effectively, etc.)
                  and that can be built by a Carrier
    Inputs      : shipstatic - the staticinfo to check
    Outputs     :
    Return      : TRUE if the ship is a fighting ship
----------------------------------------------------------------------------*/
bool statShipConstraintsCarrierFightingShipsCB(ShipStaticInfo *shipstatic)
{
    switch (shipstatic->shipclass)
    {
        case CLASS_Mothership:
        case CLASS_Carrier:
        case CLASS_HeavyCruiser:
        case CLASS_Destroyer:
            return FALSE;
            break;
        case CLASS_Frigate:
            if ((shipstatic->shiptype == ResourceController) ||
                (shipstatic->shiptype == DFGFrigate) ||
                (shipstatic->shiptype == AdvanceSupportFrigate))
            {
                return FALSE;
            }
            else
            {
                return TRUE;    // all other Frigates are considered fighting ships
            }
            break;
        case CLASS_Corvette:
            if ((shipstatic->shiptype == RepairCorvette) ||
                (shipstatic->shiptype == SalCapCorvette))
            {
                return FALSE;   //salcap corvette doesn't attack in a conventional manner
            }
            else
            {
                return TRUE;
            }
        case CLASS_Fighter:
            if (shipstatic->shiptype == DefenseFighter)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        case CLASS_Resource:
        case CLASS_NonCombat:
        default:
            return FALSE;
    }
    return FALSE;
}

real32 statsGetKillRatingAgainstFleet(ShipStaticInfo *shipstatic,SelectCommand *fleet)
{
    sdword numShips = fleet->numShips;
    sdword i;
    real32 totalkills = 0.0f;

    dbgAssert(numShips > 0);

    // kill rating against fleet is reciprical of (sum of recipricals of kill ratings)

    for (i=0;i<numShips;i++)
    {
        totalkills += statsGetShipKillRatingAgainstShip(fleet->ShipPtr[i]->staticinfo,shipstatic);
    }

    return (1.0f / totalkills);
}

real32 statsGetKillRatingAgainstFleetStatic(ShipStaticInfo *shipstatic,SelectCommandStatic *fleetStatic)
{
    sdword numShips = fleetStatic->numShips;
    sdword i;
    real32 totalkills = 0;

    dbgAssert(numShips > 0);

    for (i=0;i<numShips;i++)
    {
        totalkills += statsGetShipKillRatingAgainstShip(fleetStatic->ShipStaticPtr[i],shipstatic);
    }

    return (1.0f / totalkills);
}

real32 statsGetRURatingAgainstFleet(ShipStaticInfo *shipstatic,SelectCommand *fleet)
{
    sdword numShips = fleet->numShips;
    sdword i;
    real32 totalrurating = 0;

    dbgAssert(numShips > 0);

    for (i=0;i<numShips;i++)
    {
        totalrurating += statsGetShipRURatingAgainstShip(fleet->ShipPtr[i]->staticinfo,shipstatic);
    }

    return (1.0f / totalrurating);
}

real32 statsGetRURatingAgainstFleetStatic(ShipStaticInfo *shipstatic,SelectCommandStatic *fleetStatic)
{
    sdword numShips = fleetStatic->numShips;
    sdword i;
    real32 totalrurating = 0;

    dbgAssert(numShips > 0);

    for (i=0;i<numShips;i++)
    {
        totalrurating += statsGetShipRURatingAgainstShip(fleetStatic->ShipStaticPtr[i],shipstatic);
    }

    return (1.0f / totalrurating);
}

Ship *statsGetMostDangerousShipNonStatConstraints(SelectCommand *selection,ShipConstraintsCB constraintsCB)
{
    sdword numShips = selection->numShips;
    sdword i;
    Ship *ship;
    Ship *maxship = NULL;
    real32 maxKillratio = -1.0f;
    real32 Killratio;

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (constraintsCB(ship))
        {
            Killratio = statsGetOverallKillRating(ship->staticinfo);

            if (Killratio > maxKillratio)
            {
                maxKillratio = Killratio;
                maxship = ship;
            }
        }
    }

    return maxship;
}

ShipStaticInfo *statsGetMostDangerousShipStaticConstraints(SelectCommandStatic *selection,statShipConstraintsCB constraintsCB)
{
    sdword numShips = selection->numShips;
    sdword i;
    ShipStaticInfo *shipstatic;
    ShipStaticInfo *maxshipstatic = NULL;
    real32 maxKillratio = -1.0f;
    real32 Killratio;

    for (i=0;i<numShips;i++)
    {
        shipstatic = selection->ShipStaticPtr[i];

        if (constraintsCB(shipstatic))
        {
            Killratio = statsGetOverallKillRating(shipstatic);

            if (Killratio > maxKillratio)
            {
                maxKillratio = Killratio;
                maxshipstatic = shipstatic;
            }
        }
    }

    return maxshipstatic;
}

ShipStaticInfo *statsBestShipToBuyToKillShip(ShipRace shipRace,statShipConstraintsCB constraintsCB,ShipStaticInfo *targetstatic)
{
    sdword i;
    ShipStaticInfo *shipstatic;
    ShipStaticInfo *maxshipstatic = NULL;
    real32 maxRUratio = -1.0f;
    real32 RUratio;

    sdword raceiNumShips = NumShipTypesInRace[shipRace];
    sbyte *raceiCalcFightStatsFor = RaceCalcFightStatsFor[shipRace];
    sdword raceiStatIndexOffset = StatIndexRaceOffsets[shipRace];
    sdword indexi;

    for (i=0;i<raceiNumShips;i++)
    {
        if (raceiCalcFightStatsFor[i])
        {
            indexi = i + raceiStatIndexOffset;
            shipstatic = ConvertStatIndexToShipStatic(indexi);
            if ((shipstatic) && constraintsCB(shipstatic))
            {
                dbgAssert(shipstatic->shiprace == shipRace);
                RUratio = statsGetShipRURatingAgainstShip(shipstatic,targetstatic);
                if (RUratio > maxRUratio)
                {
                    maxRUratio = RUratio;
                    maxshipstatic = shipstatic;
                }
            }
        }
    }

    return maxshipstatic;
}

ShipStaticInfo *statsBestShipToBuyToKillFleet(ShipRace shipRace,statShipConstraintsCB constraintsCB,SelectCommand *targetFleet)
{
    sdword i;
    sdword indexi;
    ShipStaticInfo *shipstatic;
    ShipStaticInfo *maxshipstatic = NULL;
    real32 maxRUratio = -1.0f;
    real32 RUratio;

    sdword raceiNumShips = NumShipTypesInRace[shipRace];
    sbyte *raceiCalcFightStatsFor = RaceCalcFightStatsFor[shipRace];
    sdword raceiStatIndexOffset = StatIndexRaceOffsets[shipRace];

    for (i=0;i<raceiNumShips;i++)
    {
        if (raceiCalcFightStatsFor[i])
        {
            indexi = i + raceiStatIndexOffset;
            shipstatic = ConvertStatIndexToShipStatic(indexi);
            if ((shipstatic) && constraintsCB(shipstatic))
            {
                dbgAssert(shipstatic->shiprace == shipRace);
                RUratio = statsGetRURatingAgainstFleet(shipstatic,targetFleet);
                if (RUratio > maxRUratio)
                {
                    maxRUratio = RUratio;
                    maxshipstatic = shipstatic;
                }
            }
        }
    }

    return maxshipstatic;
}

ShipStaticInfo *statsBestShipToBuyToKillFleetStatic(ShipRace shipRace,statShipConstraintsCB constraintsCB,SelectCommandStatic *targetFleetStatic)
{
    sdword i;
    ShipStaticInfo *shipstatic;
    ShipStaticInfo *maxshipstatic = NULL;
    real32 maxRUratio = -1.0f;
    real32 RUratio;

    sdword raceiNumShips = NumShipTypesInRace[shipRace];
    sbyte *raceiCalcFightStatsFor = RaceCalcFightStatsFor[shipRace];
    sdword raceiStatIndexOffset = StatIndexRaceOffsets[shipRace];
    sdword indexi;

    for (i=0;i<raceiNumShips;i++)
    {
        if (raceiCalcFightStatsFor[i])
        {
            indexi = i + raceiStatIndexOffset;
            shipstatic = ConvertStatIndexToShipStatic(indexi);
            if ((shipstatic) && constraintsCB(shipstatic))
            {
                RUratio = statsGetRURatingAgainstFleetStatic(shipstatic,targetFleetStatic);
                if (RUratio > maxRUratio)
                {
                    maxRUratio = RUratio;
                    maxshipstatic = shipstatic;
                }
            }
        }
    }

    return maxshipstatic;
}

Ship *statsBestShipToUseToKillShip(SelectCommand *freeShips,ShipStaticInfo *targetstatic)
{
    sdword i;
    Ship *ship;
    Ship *maxship = NULL;
    real32 maxKillratio = -1.0f;
    real32 Killratio;
    sdword numShips = freeShips->numShips;

    for (i=0;i<numShips;i++)
    {
        ship = freeShips->ShipPtr[i];
        dbgAssert(ship->objtype == OBJ_ShipType);

        Killratio = statsGetShipKillRatingAgainstShip(ship->staticinfo,targetstatic);

        if (Killratio > maxKillratio)
        {
            maxKillratio = Killratio;
            maxship = ship;
        }
    }

    return maxship;
}

Ship *statsBestShipToUseToKillFleet(SelectCommand *freeShips,SelectCommand *targetFleet)
{
    sdword i;
    Ship *ship;
    Ship *maxship = NULL;
    real32 maxKillratio = -1.0f;
    real32 Killratio;
    sdword numShips = freeShips->numShips;

    for (i=0;i<numShips;i++)
    {
        ship = freeShips->ShipPtr[i];
        dbgAssert(ship->objtype == OBJ_ShipType);

        Killratio = statsGetKillRatingAgainstFleet(ship->staticinfo,targetFleet);

        if (Killratio > maxKillratio)
        {
            maxKillratio = Killratio;
            maxship = ship;
        }
    }

    return maxship;
}

sdword statsNumShipsNeededToKillTarget(ShipStaticInfo *shipstatic,ShipStaticInfo *targetstatic)
{
    real32 killratio = statsGetShipKillRatingAgainstShip(shipstatic,targetstatic);
    real32 num;

    if (killratio >= 1)
    {
        num = 1.0f;     // can kill >= 1 target with just 1 ship, so we only need 1
    }
    else
    {
        num = 1.0f / killratio;
    }

    return ((sdword)(num * statsOverkillfactor));
}

sdword statsNumShipsNeededToKillFleet(ShipStaticInfo *shipstatic,SelectCommand *targetFleet)
{
    real32 killratio = statsGetKillRatingAgainstFleet(shipstatic,targetFleet);
    real32 num;

    if (killratio == 0.0f)
    {
        return 0;
    }
    else if (killratio >= 1)
    {
        num = 1.0f;     // can kill >= 1 target with just 1 ship, so we only need 1
    }
    else
    {
        num = 1.0f / killratio;
    }

    return (((sdword)(num * statsOverkillfactor)));
}

sdword statsNumShipsNeededToKillFleetStatic(ShipStaticInfo *shipstatic,SelectCommandStatic *targetFleetStatic)
{
    real32 killratio = statsGetKillRatingAgainstFleetStatic(shipstatic,targetFleetStatic);
    real32 num;

    if (killratio == 0.0f)
    {
        return 0;
    }
    else if (killratio >= 1)
    {
        num = 1.0f;     // can kill >= 1 target with just 1 ship, so we only need 1
    }
    else
    {
        num = 1.0f / killratio;
    }

    return (((sdword)(num * statsOverkillfactor)));
}

// strength of fleet2 against fleet1, e.g. fleet2/fleet1
real32 statsGetRelativeFleetStrengths(SelectCommand *fleet1,SelectCommand *fleet2)
{
    //sdword numShips1 = fleet1->numShips;
    sdword numShips2 = fleet2->numShips;
    sdword j;
    //real32 totalstr1 = 0.0f;
    real32 totalstr2 = 0.0f;

    //dbgAssert(numShips1 > 0);
    dbgAssert(numShips2 > 0);
#if 0
    for (i=0;i<numShips1;i++)
    {
        totalstr1 += statsGetKillRatingAgainstFleet(fleet1->ShipPtr[i]->staticinfo,fleet2);
    }
#endif
    for (j=0;j<numShips2;j++)
    {
        totalstr2 += statsGetKillRatingAgainstFleet(fleet2->ShipPtr[j]->staticinfo,fleet1);
    }

    return totalstr2;
}

// strength of ship against fleet strength, e.g. targetstatic strength / fleet1 strength
real32 statsGetRelativeFleetStrengthAgainstShip(SelectCommand *fleet1,ShipStaticInfo *targetstatic)
{
    //sdword numShips1 = fleet1->numShips;
    //real32 totalstr1 = 0.0f;
    real32 totalstr2;

    //dbgAssert(numShips1 > 0);
#if 0
    for (i=0;i<numShips1;i++)
    {
        totalstr1 += statsGetShipKillRatingAgainstShip(fleet1->ShipPtr[i]->staticinfo,targetstatic);
    }
#endif
    totalstr2 = statsGetKillRatingAgainstFleet(targetstatic,fleet1);

    return totalstr2;
}

SelectCommand *statsBestShipsToUseToKillTarget(SelectCommand *freeships,ShipStaticInfo *targetstatic,bool *goodEnough)
{
    SelectCommand *freeshipsleft;
    SelectCommand *useships;
    Ship *bestship;
    Ship *ship;
    sdword num;
    real32 fleetstr;
    sdword i;

    if (freeships->numShips == 0)
    {
        *goodEnough = FALSE;
        return NULL;
    }

    freeshipsleft = selectDupSelection(freeships);

    useships = memAlloc(sizeofSelectCommand(freeships->numShips),"statsuseships",0);
    useships->numShips = 0;

repeat:

    bestship = statsBestShipToUseToKillShip(freeshipsleft,targetstatic);
    num = statsNumShipsNeededToKillTarget(bestship->staticinfo,targetstatic);

    clRemoveShipFromSelection(freeshipsleft,bestship);
    useships->ShipPtr[useships->numShips++] = bestship;

    num--;
    while (num > 0)
    {
        // find another ship of type bestship->staticinfo
        for (i=0;i<freeshipsleft->numShips;i++)
        {
            ship = freeshipsleft->ShipPtr[i];
            if (ship->staticinfo == bestship->staticinfo)
            {
                // found it, remove it from freeshipsleft
                freeshipsleft->numShips--;
                freeshipsleft->ShipPtr[i] = freeshipsleft->ShipPtr[freeshipsleft->numShips];

                // add it to useships
                useships->ShipPtr[useships->numShips++] = ship;
                goto foundit;
            }
        }
        // didn't find any more ships, so break out of loop
        break;
foundit:
        num--;
    }

    // now check fleet strengths:

    // want useships strength relative to targetstatic
    fleetstr = 1.0f / statsGetRelativeFleetStrengthAgainstShip(useships,targetstatic);

    if (fleetstr < statsOverkillfactor)
    {
        // not good enough, so lets exit and not repeat if we don't have any free ships
        if (freeshipsleft->numShips == 0)
        {
            // use all ships
            dbgAssert(useships->numShips == freeships->numShips);
            memFree(freeshipsleft);
            *goodEnough = FALSE;
            return useships;
        }
        goto repeat;
    }

    memFree(freeshipsleft);
    *goodEnough = TRUE;
    return useships;
}

SelectCommand *statsBestShipsToUseToKillFleet(SelectCommand *freeships,SelectCommand *targetFleet,bool *goodEnough)
{
    SelectCommand *freeshipsleft;
    SelectCommand *useships;
    Ship *bestship;
    Ship *ship;
    sdword num;
    real32 fleetstr;
    sdword i;

    if (freeships->numShips == 0)
    {
        *goodEnough = FALSE;
        return NULL;
    }

    freeshipsleft = selectDupSelection(freeships);

    useships = memAlloc(sizeofSelectCommand(freeships->numShips),"statsuseships",0);
    useships->numShips = 0;

repeat:

    bestship = statsBestShipToUseToKillFleet(freeshipsleft,targetFleet);
    num = statsNumShipsNeededToKillFleet(bestship->staticinfo,targetFleet);

    clRemoveShipFromSelection(freeshipsleft,bestship);
    useships->ShipPtr[useships->numShips++] = bestship;

    num--;
    while (num > 0)
    {
        // find another ship of type bestship->staticinfo
        for (i=0;i<freeshipsleft->numShips;i++)
        {
            ship = freeshipsleft->ShipPtr[i];
            if (ship->staticinfo == bestship->staticinfo)
            {
                // found it, remove it from freeshipsleft
                freeshipsleft->numShips--;
                freeshipsleft->ShipPtr[i] = freeshipsleft->ShipPtr[freeshipsleft->numShips];

                // add it to useships
                useships->ShipPtr[useships->numShips++] = ship;
                goto foundit;
            }
        }
        // didn't find any more ships, so break out of loop
        break;
foundit:
        num--;
    }

    // now check fleet strengths:

    // want useships strength relative to targetFleet
    fleetstr = 1.0f / statsGetRelativeFleetStrengths(useships,targetFleet);

    if (fleetstr < statsOverkillfactor)
    {
        // not good enough, so lets exit and not repeat if we don't have any free ships
        if (freeshipsleft->numShips == 0)
        {
            // use all ships
            dbgAssert(useships->numShips == freeships->numShips);
            memFree(freeshipsleft);
            *goodEnough = FALSE;
            return useships;
        }
        goto repeat;
    }

    memFree(freeshipsleft);
    *goodEnough = TRUE;
    return useships;
}

SelectCommand *statsGetMostEfficientShipsToKill(ShipStaticInfo *killer,SelectCommand *fleet,real32 efficiencyFactor)
{
    SelectCommand *returnsel;
    sdword i;

    if (fleet->numShips <= 0)
    {
        return NULL;
    }

    returnsel = selectDupSelection(fleet);

    for (i=0;i<returnsel->numShips; )
    {
        if (statsGetShipRURatingAgainstShip(killer,returnsel->ShipPtr[i]->staticinfo) <= efficiencyFactor)
        {
            // killer ship sucks against this ship, so let's remove it

            returnsel->numShips--;
            returnsel->ShipPtr[i] = returnsel->ShipPtr[returnsel->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }

    if (returnsel->numShips == 0)
    {
        memFree(returnsel);
        return NULL;
    }

    return returnsel;
}

SelectCommand *CreateTestSelectionOfShips(ShipRace shiprace,ShipType shiptype,sdword numShips)
{
    SelectCommand *selection;
    vector position = { 0.0f, 0.0f, 0.0f };
    sdword i;

    selection = memAlloc(sizeofSelectCommand(numShips),"createshipsel",0);

    selection->numShips = numShips;
    for (i=0;i<numShips;i++)
    {
        selection->ShipPtr[i] = univCreateShip(shiptype,shiprace,&position,&universe.players[0],0);
    }

    return selection;
}

void FreeTestSelectionOfShips(SelectCommand *selection)
{
    sdword numShips = selection->numShips;
    sdword i;

    for (i=0;i<numShips;i++)
    {
        univFreeShipContents(selection->ShipPtr[i]);
        memFree(selection->ShipPtr[i]);
    }

    memFree(selection);
}

static void statsTest(void)
{
#ifndef HW_Release
#ifdef gshaw
#if 0
    ShipType shiptype1,shiptype2;
    ShipRace shiprace1,shiprace2;
    ShipStaticInfo *shipstatic1,*shipstatic2;
    real32 killratio;
    real32 ruratio;
    sdword num;
    SelectCommand *selection1,*selection2,*selection3,*selection4,*selection5;
    SelectCommand *fleet1,*fleet2,*fleet3;
    real32 ratio1,ratio2,ratio3,ratio4;
    SelectCommand *bestsel;
    sdword numneeded;
    Ship *ship;
    bool goodEnough;

    statsSetOverkillfactor(1.2f);

    shiptype2 = IonCannonFrigate;
    shiprace2 = R1;
    shipstatic2 = GetShipStaticInfoValidate(shiptype2,shiprace2);

    shiptype1 = StandardFrigate;
    shiprace1 = R1;
    shipstatic1 = GetShipStaticInfoValidate(shiptype1,shiprace1);

    killratio = statsGetShipRURatingAgainstShip(shipstatic1,shipstatic2);
    ruratio = statsGetShipKillRatingAgainstShip(shipstatic1,shipstatic2);

    shiptype1 = StandardFrigate;
    shiprace1 = R1;
    shipstatic1 = GetShipStaticInfoValidate(shiptype1,shiprace1);

    killratio = statsGetOverallKillRating(shipstatic1);
    ruratio = statsGetOverallRURating(shipstatic1);

    shiptype1 = StandardFrigate;
    shiprace1 = R1;
    shipstatic1 = GetShipStaticInfoValidate(shiptype1,shiprace1);

    shipstatic2 = statsBestShipToBuyToKillShip(R1,statShipConstraintsNoneCB,shipstatic1);
    shipstatic2 = statsBestShipToBuyToKillShip(R1,statShipConstraintsFrigatesOrWorseCB,shipstatic1);

    num = statsNumShipsNeededToKillTarget(shipstatic2,shipstatic1);

    if (!multiPlayerGame)       // game will get out of sync due to test ship creation
    {
        selection1 = CreateTestSelectionOfShips(R1,HeavyDefender,10);
        selection2 = CreateTestSelectionOfShips(R1,LightCorvette,5);
        selection3 = CreateTestSelectionOfShips(R1,IonCannonFrigate,1);
        selection4 = CreateTestSelectionOfShips(R1,HeavyDefender,10);
        selection5 = CreateTestSelectionOfShips(R1,LightInterceptor,22);

        fleet1 = selectMergeTwoSelections(selection1,selection2, NO_DEALLOC);
        fleet2 = selectMergeTwoSelections(selection3,selection4, NO_DEALLOC);

        ratio2 = statsGetRelativeFleetStrengths(selection2,selection3);
        ratio3 = statsGetRelativeFleetStrengths(selection2,selection4);
        ratio4 = statsGetRelativeFleetStrengths(selection1,selection4);
        ratio1 = statsGetRelativeFleetStrengths(fleet1,fleet2);

        bestsel = statsBestShipsToUseToKillTarget(fleet1,shipstatic1,&goodEnough);
        memFree(bestsel);

        shipstatic1 = statsBestShipToBuyToKillFleet(R1,statShipConstraintsFrigatesOrWorseCB,fleet2);
        numneeded = statsNumShipsNeededToKillFleet(shipstatic1,fleet2);

        ratio1 = statsGetRelativeFleetStrengths(fleet2,selection5);

        ship = statsGetMostDangerousShip(fleet2);

        fleet3 = selectMergeTwoSelections(fleet1,selection5, NO_DEALLOC);
        bestsel = statsBestShipsToUseToKillFleet(fleet3,fleet2,&goodEnough);
        memFree(bestsel);

        ratio1 = statsGetRelativeFleetStrengths(fleet3,fleet2);

        FreeTestSelectionOfShips(selection1);
        FreeTestSelectionOfShips(selection2);
        FreeTestSelectionOfShips(selection3);
        FreeTestSelectionOfShips(selection4);
        FreeTestSelectionOfShips(selection5);

        memFree(fleet1);
        memFree(fleet2);
        memFree(fleet3);
    }
#endif
#endif
#endif
}


/*=============================================================================
    Cheat detection:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : statsGetStatChecksum
    Description : Returns a checksum of the game statistics
    Inputs      :
    Outputs     :
    Return      : The checksum of the game statistics
----------------------------------------------------------------------------*/
udword statsGetStatChecksum(void)
{
    udword i, intCheck;
    udword shiptotals,classtotals;

    for (i=0, intCheck = 0;i<tpGameCreated.numPlayers;i++)
    {
        // add the player's shipcount, resourcecount and sensorlevel
        intCheck += universe.players[i].totalships;
        intCheck += universe.players[i].resourceUnits;
        //intCheck += universe.players[i].sensorLevel;

        // add the player's shipcount for each shiptype
        shiptotals = crc32Compute((ubyte *)&universe.players[i].shiptotals, TOTAL_NUM_SHIPS*sizeof(sdword));
        intCheck += shiptotals;

        // add the player's shipcount for each shipclass
        classtotals = crc32Compute((ubyte *)&universe.players[i].shiptotals, NUM_CLASSES*sizeof(sdword));
        intCheck += classtotals;

        // add the player's research information
        //intCheck += crc32Compute(&universe.players[i].researchinfo, sizeof(PlayerResearchInfo));
        intCheck += universe.players[i].researchinfo.HasTechnology;
        intCheck += universe.players[i].researchinfo.listoftopics.num;

        if ((netlogfile) && (logEnable == LOG_VERBOSE))
        {
#if BINNETLOG
            binnetlogCheatInfo bnc;

            bnc.header = makenetcheckHeader('C','D','E','T');
            bnc.totalships = universe.players[i].totalships;
            bnc.resourceunits = universe.players[i].resourceUnits;
            bnc.shiptotals = shiptotals;
            bnc.classtotals = classtotals;
            bnc.hastechnology = universe.players[i].researchinfo.HasTechnology;
            bnc.listoftopicsnum = universe.players[i].researchinfo.listoftopics.num;
            fwrite(&bnc,sizeof(bnc),1,netlogfile);
#else
            fprintf(netlogfile,"  CDET:%d %d %d %d %d %d\n",universe.players[i].totalships,universe.players[i].resourceUnits,
                                shiptotals,classtotals,universe.players[i].researchinfo.HasTechnology,universe.players[i].researchinfo.listoftopics.num);
#endif
        }
    }

    // bounty checking (all bounty values must be deterministic)
    {
#if BINNETLOG
        binnetlogBountyInfo bnb;

        bnb.header = makenetcheckHeader('B','O','U','N');
        for (i=0;i<8;i++)
        {
            bnb.bounties[i] = 0;
        }
#endif

        for (i=0;i<universe.numPlayers;i++)
        {
            intCheck += universe.players[i].bounty;
#if BINNETLOG
            bnb.bounties[i] = universe.players[i].bounty;
#endif
        }

        if ((netlogfile) && (logEnable == LOG_VERBOSE))
        {
#if BINNETLOG
            fwrite(&bnb,sizeof(bnb),1,netlogfile);
#else
            fprintf(netlogfile,"BOUNT:");
            for (i=0;i<universe.numPlayers;i++)
            {
                fprintf(netlogfile," %d",universe.players[i].bounty);
            }
            fprintf(netlogfile,"\n");
#endif
        }

    }

    return intCheck;
}

