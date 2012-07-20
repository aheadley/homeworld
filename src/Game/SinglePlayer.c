
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
/*#include "bink.h"*/
#include "Types.h"
#include "Vector.h"
#include "LinkedList.h"
#include "ObjTypes.h"
#include "SpaceObj.h"
#include "StatScript.h"
#include "font.h"
#include "FEFlow.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "Physics.h"
#include "SinglePlayer.h"
#include "DDDFrigate.h"
#include "TaskBar.h"
#include "utility.h"
#include "mainrgn.h"
#include "AITrack.h"
#include "AIShip.h"
#include "LevelLoad.h"
#include "Sensors.h"
#include "NIS.h"
#include "mouse.h"
#include "Select.h"
#include "SoundEvent.h"
#include "Collision.h"
#include "ResearchShip.h"
#include "InfoOverlay.h"
#include "AIPlayer.h"
#include "KAS.h"
#include "HS.h"
#include "AIVar.h"
#include "Objectives.h"
#include "ResearchGUI.h"
#include "LaunchMgr.h"
#include "TradeMgr.h"
#include "File.h"
#include "ETG.h"
#include "HorseRace.h"
#include "Attributes.h"
#include "SaveGame.h"
#include "Damage.h"
#include "TradeMgr.h"
#include "Ping.h"
#include "Tutor.h"
#include "Randy.h"
#include "../Generated/Mission01.h"
#include "../Generated/Mission02.h"
#include "../Generated/Mission03.h"
#include "../Generated/Mission04.h"
#ifdef OEM
#include "../Generated/Mission05_OEM.h"
#else
#include "../Generated/Mission05.h"
#include "../Generated/Mission06.h"
#include "../Generated/Mission07.h"
#include "../Generated/Mission08.h"
#include "../Generated/Mission09.h"
#include "../Generated/Mission10.h"
#include "../Generated/Mission11.h"
#include "../Generated/Mission12.h"
#include "../Generated/Mission13.h"
#include "../Generated/Mission14.h"
#include "../Generated/Mission15.h"
#include "../Generated/Mission16.h"
#endif
#include "../Generated/Tutorial1.h"
#include "Strings.h"
#include "Animatic.h"
#include "GravWellGenerator.h"
#include "CommandWrap.h"
#include "Mothership.h"
#include "Alliance.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#define NUMBER_SINGLEPLAYER_MISSIONS 19

#define ShipCanMakeHyperspace(ship) (ShipCanHyperspace(ship) && ((ship)->specialFlags & SPECIAL_SinglePlayerWithinHyperSpaceRange))
#define WARP_FILENAME "Warp.script"

//stores the current mission filename
char CurrentLevelName[256] = "";

bool singlePlayerHyperspacingInto = FALSE;

bool spHoldHyperspaceWindow = FALSE;

bool hyperspaceOverride = FALSE;

real32 spFleetModifier = 0.0f;
real32 spFleetStr = 0.0f;

real32 BaseFleetStrengthForLevel[NUMBER_SINGLEPLAYER_MISSIONS+1];
real32 AdvancedFleetStrengthForLevel[NUMBER_SINGLEPLAYER_MISSIONS+1];

bool triggerNIS = FALSE;

sdword warpLevel = 0;

bool spBinkPlay;

extern fonthandle selGroupFont0;

// state of kas debug overlays
#if SP_DEBUGLEVEL2
extern sdword mrKASDebugDrawStates;
extern sdword mrKASDebugDrawTimers;
extern sdword mrKASDebugDrawVars;
extern sdword mrKASDebugDrawVolumes;
extern sdword mrKASSkipFactor;
#endif

bool singlePlayerGameLoadNewLevelFlag;
SinglePlayerGameInfo singlePlayerGameInfo;

real32 HYPERSPACE_SLICE_RATE = 0.075f;

bool HYPERSPACE_MOTHERSHIP_ALWAYS_ARRIVES_FIRST = TRUE;
real32 HYPERSPACE_WAIT_FOR_MOTHERSHIPFOCUS_TIME = 5.0f;
real32 HYPERSPACE_BASE_WAITTIME = 3.0f;
real32 HYPERSPACE_BASE_ARRIVETIME = 2.0f;
real32 HYPERSPACE_BASE_MOTHERSHIPARRIVETIME = 1.0f;
real32 HYPERSPACE_RANDOM_WAITTIME = 0.2f;
real32 HYPERSPACE_WAITTIME_PER_DISTANCE = 0.0005f;
real32 HYPERSPACE_MINWAITTIME_DISTANCE = -1.0f;
real32 HYPERSPACE_MAXWAITTIME_DISTANCE = 1.0f;
real32 HYPERSPACE_ACCELERATE_TIME = 3.0f;
real32 HYPERSPACE_ACCELERATION = 1000.0f;
real32 HYPERSPACE_WHIP_VELOCITY = 50000.0f;
real32 HYPERSPACE_HEADINGACCURACY = 0.98f;
real32 HYPERSPACE_FARCLIPPLANE = 100000.0f;
real32 HYPERSPACE_WHITEOUTTIME = 0.2f;
real32 HYPERSPACE_FUNNEL_WIDTH = 5000.0f;
real32 HYPERSPACE_FUNNEL_WIDTH_TO_BACK_FACTOR = 0.5f;

real32 HYPERSPACE_FARCLIPPLANESQR;

real32 HYPERSPACE_DECELERATE_TIME = 3.0f;
real32 HYPERSPACE_DECELERATION = 10000.0f;

real32 HYPERSPACE_DECELERATE_DIST;
real32 HYPERSPACE_DECELERATE_INITIAL_VEL;
real32 HYPERSPACE_WHIPTOWARDSDIST;

real32 HYPERSPACE_TRAVELDIST_PER_RU_SPENT = 10000.0f;

sdword SINGLEPLAYER_STARTINGRUS = 200;
sdword SINGLEPLAYER_DEBUGLEVEL = 0;

real32 HS_CLIPT_NEG_THRESH = -0.4f;
real32 HS_CLIPT_NEG_SCALAR = 0.33f;
real32 HS_CLIPT_NEG_FINISH = 0.9f;
real32 HS_CLIPT_POS_THRESH = 0.5f;
real32 HS_CLIPT_POS_SCALAR = 0.5f;
real32 HS_CLIPT_POS_START  = 1.0f;

real32 HYPERSPACEGATE_HEALTH = 20000.0f;
real32 HYPERSPACEGATE_WIDTH = 1000.0f;
real32 HYPERSPACEGATE_HEIGHT = 1000.0f;
real32 HYPERSPACEGATE_WAYPOINTDIST = 800.0f;

real32 HYPERSPACE_RANGE = (3000.0f*3000.0f);
real32 spHyperspaceDelay = 0.0f;

real32 SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE = 50000.0f;

real32 SUBMESSAGE_SAFETY_TIMEOUT = 15.0f;

real32 SINGLEPLAYER_BOBBIGGESTRADIUS_LEVEL6 = 40000.0f;

void tmTechInit(void);

static void SetBaseFleetStrCB(char *directory,char *field,void *dataToFillIn);

scriptEntry SinglePlayerTweaks[] =
{
    makeEntry(HS_CLIPT_NEG_THRESH, scriptSetReal32CB),
    makeEntry(HS_CLIPT_NEG_SCALAR, scriptSetReal32CB),
    makeEntry(HS_CLIPT_NEG_FINISH, scriptSetReal32CB),
    makeEntry(HS_CLIPT_POS_THRESH, scriptSetReal32CB),
    makeEntry(HS_CLIPT_POS_SCALAR, scriptSetReal32CB),
    makeEntry(HS_CLIPT_POS_START,  scriptSetReal32CB),
    makeEntry(HYPERSPACEGATE_HEALTH, scriptSetReal32CB),
    makeEntry(HYPERSPACEGATE_WIDTH , scriptSetReal32CB),
    makeEntry(HYPERSPACEGATE_HEIGHT, scriptSetReal32CB),
    makeEntry(HYPERSPACEGATE_WAYPOINTDIST, scriptSetReal32CB),
    makeEntry(HYPERSPACE_SLICE_RATE, scriptSetReal32CB),
    makeEntry(HYPERSPACE_WAIT_FOR_MOTHERSHIPFOCUS_TIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_MOTHERSHIP_ALWAYS_ARRIVES_FIRST,scriptSetBool),
    makeEntry(HYPERSPACE_BASE_WAITTIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_BASE_ARRIVETIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_BASE_MOTHERSHIPARRIVETIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_RANDOM_WAITTIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_WAITTIME_PER_DISTANCE,scriptSetReal32CB),
    makeEntry(HYPERSPACE_MINWAITTIME_DISTANCE,scriptSetReal32CB),
    makeEntry(HYPERSPACE_MAXWAITTIME_DISTANCE,scriptSetReal32CB),
    makeEntry(HYPERSPACE_ACCELERATE_TIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_ACCELERATION,scriptSetReal32CB),
    makeEntry(HYPERSPACE_WHIP_VELOCITY,scriptSetReal32CB),
    makeEntry(HYPERSPACE_FARCLIPPLANE,scriptSetReal32CB),
    makeEntry(HYPERSPACE_HEADINGACCURACY,scriptSetCosAngCB),
    makeEntry(HYPERSPACE_WHITEOUTTIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_DECELERATE_TIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_DECELERATION,scriptSetReal32CB),
    makeEntry(HYPERSPACE_TRAVELDIST_PER_RU_SPENT,scriptSetReal32CB),
    makeEntry(SINGLEPLAYER_STARTINGRUS,scriptSetSdwordCB),
    makeEntry(SINGLEPLAYER_DEBUGLEVEL,scriptSetSdwordCB),
    makeEntry(HYPERSPACE_RANGE,scriptSetReal32SqrCB),
    makeEntry(SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE,scriptSetReal32CB),
    makeEntry(SUBMESSAGE_SAFETY_TIMEOUT,scriptSetReal32CB),
    makeEntry(SINGLEPLAYER_BOBBIGGESTRADIUS_LEVEL6,scriptSetReal32CB),
    { "BaseFleetStrengthForLevel",SetBaseFleetStrCB,NULL },
    endEntry
};

static void SetWarpFleetCB(char *directory,char *field,void *dataToFillIn);
static void WarpFleetStrengthCB(char *directory,char *field,void *dataToFillIn);
static void WarpFleetRUStrengthCB(char *directory,char *field,void *dataToFillIn);
static void WarpPreLoadCB(char *directory,char *field,void *dataToFillIn);

scriptEntry WarpScriptTable[] =
{
    { "WarpStartRU", scriptSetSdwordCB, &universe.players[0].resourceUnits},
    { "WarpFleet", SetWarpFleetCB, NULL },
    endEntry
};

scriptEntry WarpCalcFleetStrengthTable[] =
{
    { "WarpStartRU", WarpFleetRUStrengthCB, NULL},
    { "WarpFleet", WarpFleetStrengthCB, NULL },
    endEntry
};

scriptEntry WarpPreLoadTable[] =
{
    { "WarpFleet", WarpPreLoadCB, NULL },
    endEntry
};

static sdword WarpToLevelEnabled();
static void SetOnMissionCompleteInfo(char *directory,char *field,void *dataToFillIn);
static void singlePlayerKasMissionStart(sdword missionnum);
static void singlePlayerStartNis(char *nis, char *script, bool centre, hvector *positionAndAngle);
void singlePlayerNISNamesGet(char **nisname, char **scriptname, bool *centreMothership, sdword nisNumber);

//static bool spSavedCameraYet;
//static Camera spSavedCamera;

scriptEntry SinglePlayerScriptTable[] =
{
    { "onMissionComplete", SetOnMissionCompleteInfo, &singlePlayerGameInfo },
    { "onMissionLoad", scriptSetSdwordCB, &singlePlayerGameInfo.onLoadPlayNIS},
    { "MissionStartEnemyRUs", scriptSetSdwordCB, &universe.players[1].resourceUnits },
    { "MissionStartEnemyRace", scriptSetShipRaceCB, &universe.players[1].race },
    { "Asteroid0sCanMove", scriptSetBool, &singlePlayerGameInfo.asteroid0sCanMove },
    endEntry
};

fibfileheader *spHyperspaceRollCallHandle = NULL;
regionhandle spHyperspaceRollCallScreen = NULL;

//taskbutton *hyperspacetbbutton = NULL;

void spAbortHyperspaceCB(char *string, featom *atom);
void spGoNowHyperspaceCB(char *string, featom *atom);

void spShipsRemainingDrawCB(featom *atom, regionhandle region);
void spShipsDockedDrawCB(featom *atom, regionhandle region);

void singlePlayerLoadNewLevel(void);

fecallback spHyperspaceCallback[] =
{
    {spAbortHyperspaceCB,  "AbortHyperspace"},
    {spGoNowHyperspaceCB,  "GoNowHyperspace"},
    {NULL, NULL}
};

fedrawcallback spHyperspaceDrawCallback[] =
{
    {spShipsRemainingDrawCB, "ShipsRemainingDraw"},
    {spShipsDockedDrawCB,    "ShipsDockedDraw"},
    {NULL, NULL}
};

char spMissionsDir[32];
char spMissionsFile[32];

void GetMissionsDirAndFile(sdword mission)
{
#ifdef OEM
    if (mission == 5)
    {
#ifdef _WIN32
        sprintf(spMissionsDir, "SinglePlayer\\Mission05_OEM\\");
#else
        sprintf(spMissionsDir, "SinglePlayer/Mission05_OEM/");
#endif
        sprintf(spMissionsFile, "Mission05_OEM.mission");
        return;
    }
#endif

#ifdef _WIN32
    sprintf(spMissionsDir, "SinglePlayer\\Mission%02d\\", mission);
#else
    sprintf(spMissionsDir, "SinglePlayer/Mission%02d/", mission);
#endif
    sprintf(spMissionsFile,"Mission%02d.mission", mission);
}

bool GetPointOfName(hvector *point,char *name)
{
    hvector *vectptr = kasVectorPtr(name);

    if (vectptr != NULL)
    {
        *point = *vectptr;
        return TRUE;
    }
    else
    {
        point->x = 0.0f;   point->y = 0.0f;   point->z = 0.0f;   point->w = 0.0f;
        return FALSE;
    }
}

bool GetStartPointPlayer(hvector *startpoint)
{
    return GetPointOfName(startpoint,"StartPointPlayer");
}

void SetBaseFleetStrCB(char *directory,char *field,void *dataToFillIn)
{
    sdword level;
    real32 basestrength;
    real32 advancedstrength;

    RemoveCommasFromString(field);

    sscanf(field,"%d %f %f",&level,&basestrength,&advancedstrength);

    dbgAssert(level <= NUMBER_SINGLEPLAYER_MISSIONS);

    BaseFleetStrengthForLevel[level] = basestrength;
    AdvancedFleetStrengthForLevel[level] = advancedstrength;
}

real32 GetStrengthOfShip(Ship *ship)
{
    // for now, just return RU cost of ship:
    if (ship->shiptype == Mothership)
    {
        return 0.0f;
    }
    return (real32)ship->staticinfo->buildCost;
}

real32 GetStrengthOfShipStatic(ShipStaticInfo *shipstatic)
{
    // for now, just return RU cost of ship:
    if (shipstatic->shiptype == Mothership)
    {
        return 0.0f;
    }
    return (real32)shipstatic->buildCost;
}

real32 GetStrengthOfRUs(sdword RUs)
{
    return (real32)RUs;
}

typedef void (*GetStrengthOfFleet)(Player *player);

/*-----------------------------------------------------------------------------
    Name        : GetFleetStrengthHyperspace
    Description : Returns fleet strength of ships in hyperspace of player
    Inputs      :
    Outputs     :
    Return      : Returns fleet strength of ships in hyperspace of player
----------------------------------------------------------------------------*/
void GetFleetStrengthHyperspace(Player *player)
{
    // find total strength of fleet in hyperspace
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    InsideShip *insideShip;
    Ship *ship;

    spFleetStr = 0.0f;
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            spFleetStr += GetStrengthOfShip(ship);
            // also check ships inside it:

            if ((ship->shipsInsideMe != NULL) && (ship->shiptype != DDDFrigate))
            {
                // check ships inside too
                Node *insidenode = ship->shipsInsideMe->insideList.head;
                InsideShip *insideship2;

                while (insidenode != NULL)
                {
                    insideship2 = (InsideShip *)listGetStructOfNode(insidenode);
                    dbgAssert(insideship2->ship->objtype == OBJ_ShipType);

                    spFleetStr += GetStrengthOfShip(insideship2->ship);

                    insidenode = insidenode->next;
                }
            }
        }

        node = node->next;
    }

    spFleetStr += GetStrengthOfRUs(player->resourceUnits);
}


/*-----------------------------------------------------------------------------
    Name        : GetFleetStrengthInUniverseOfPlayer
    Description : Returns fleet strength of ships of player in universe
    Inputs      :
    Outputs     :
    Return      : Returns fleet strength of ships of player in universe
----------------------------------------------------------------------------*/
void GetFleetStrengthInUniverseOfPlayer(Player *player)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    spFleetStr = 0.0f;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            spFleetStr += GetStrengthOfShip(ship);

            // also check ships inside it:

            if ((ship->shipsInsideMe != NULL) && (ship->shiptype != DDDFrigate))
            {
                // check ships inside too
                Node *insidenode = ship->shipsInsideMe->insideList.head;
                InsideShip *insideship2;

                while (insidenode != NULL)
                {
                    insideship2 = (InsideShip *)listGetStructOfNode(insidenode);
                    dbgAssert(insideship2->ship->objtype == OBJ_ShipType);

                    spFleetStr += GetStrengthOfShip(insideship2->ship);

                    insidenode = insidenode->next;
                }
            }
        }

        objnode = objnode->next;
    }

    spFleetStr += GetStrengthOfRUs(player->resourceUnits);
}

void GetFleetStrengthWarpScript(Player *player)
{
    spFleetStr = 0.0f;
    scriptSet(NULL,WARP_FILENAME,WarpCalcFleetStrengthTable);
}

void singlePlayerPreLoadCheck(void)
{
    if (warpLevel)
    {
        scriptSet(NULL,WARP_FILENAME,WarpPreLoadTable);
    }
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SetFleetModifier(sdword level,GetStrengthOfFleet getStrengthOfFleetFunc)
{
    real32 comparestrength,comparestrength2;

    comparestrength = BaseFleetStrengthForLevel[level];
    comparestrength2 = AdvancedFleetStrengthForLevel[level];

    if ((comparestrength == 0.0f) || (comparestrength2 <= comparestrength))
    {
        spFleetModifier = 0.0f;
        return;
    }

    getStrengthOfFleetFunc(&universe.players[0]);

    // set spFleetModifier to 0 to 1 based on spFleetStr commpared to comparestrength and comparestrength2

    if (spFleetStr <= comparestrength)
    {
        spFleetModifier = 0.0f;
        return;
    }
    else if (spFleetStr >= comparestrength2)
    {
        spFleetModifier = 1.0f;
        return;
    }
    else
    {
        spFleetModifier = (spFleetStr - comparestrength) / (comparestrength2 - comparestrength);
        if (spFleetModifier > 1.0f)
        {
            spFleetModifier = 1.0f;
        }
        else if (spFleetModifier < 0.0f)
        {
            spFleetModifier = 0.0f;
        }
    }
}

// this function doesn't need to be a function, its only called in one place
// and it changes the state variable of the case statement its called from, very confusing
// and its only 4 lines - 07/02/99 SA
//void spHyperspaceCommence()
//{
    //FocusCommand focusone;

//    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_SHIPSLEAVING;
//    singlePlayerGameInfo.hyperspaceSubState = FOCUSING_ON_MOTHERSHIP;
//    dbgAssert(universe.curPlayerPtr->PlayerMothership != NULL);
//    focusone.numShips = 1;
//    focusone.ShipPtr[0] = universe.curPlayerPtr->PlayerMothership;
//    ccFocus(&universe.mainCameraCommand,&focusone);
//    singlePlayerGameInfo.hyperspaceTimeStamp = universe.totaltimeelapsed; // + HYPERSPACE_WAIT_FOR_MOTHERSHIPFOCUS_TIME;

//    speechEventFleet(STAT_F_Hyper_TakingOff, 0, universe.curPlayerIndex);
//}

void TellShipsToAbortDockForHyperspace()
{
    SelectCommand *selectAll;

    selectAll = selectAllCurrentPlayersNonHyperspacingShips();
    clHalt(&universe.mainCommandLayer,selectAll);
    memFree(selectAll);
}

void spAbortHyperspaceCB(char *string, featom *atom)
{
    if (singlePlayerGameInfo.hyperspaceState == HYPERSPACE_WAITINGROLLCALL)
    {
        // Let's abort hyperspace
        speechEventFleet(COMM_F_Hyper_Abort, 0, universe.curPlayerIndex);

        // give player refund on RU's
//        universe.curPlayerPtr->resourceUnits += singlePlayerGameInfo.RUNeededForNextHyperSpaceJump;
        clAutoLaunch(singlePlayerGameInfo.oldAutoLaunch,universe.curPlayerIndex);
        singlePlayerGameInfo.hyperspaceState = NO_HYPERSPACE;

        TellShipsToAbortDockForHyperspace();

        feScreenDelete(spHyperspaceRollCallScreen);
        spHyperspaceRollCallScreen = NULL;
    }
}

void spGoNowHyperspaceCB(char *string, featom *atom)
{
    if (singlePlayerGameInfo.hyperspaceState == HYPERSPACE_WAITINGROLLCALL)
    {
        SelectCommand *selectAll;
        CommandToDo *militaryGroup;
        sdword i;
        Ship *ship;
        Ship *mothership = universe.curPlayerPtr->PlayerMothership;

        dbgAssert(mothership);

        selectAll = selectAllCurrentPlayersNonHyperspacingShips();
        clDock(&universe.mainCommandLayer,selectAll,DOCK_PERMANENTLY|DOCK_INSTANTANEOUSLY,NULL);
        memFree(selectAll);

        selectAll = selectAllCurrentPlayersHyperspacingShips();
        if(universe.curPlayerPtr->PlayerMothership->shiprace == R1)
        {
            mothershipCleanDoorForHSInstant(universe.curPlayerPtr->PlayerMothership);
        }
        for (i=0;i<selectAll->numShips;i++)
        {
            ship = selectAll->ShipPtr[i];
            if (ship->shiptype != Mothership)
            {
                SelectCommand selectone;
//                if (!(ship->specialFlags & SPECIAL_SinglePlayerWithinHyperSpaceRange))
//                {
//putinparade:
                    selectone.ShipPtr[0] = ship;
                    selectone.numShips = 1;
                    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,&selectone);
                    aitrackForceSteadyShip(ship);
                    GroupShipIntoMilitaryParade(&universe.mainCommandLayer,ship,mothership);
                    bitSet(ship->specialFlags,SPECIAL_SinglePlayerWithinHyperSpaceRange|SPECIAL_SinglePlayerInParade);
/*                }
                else
                {
                    if (!(ship->specialFlags & SPECIAL_SinglePlayerInParade))
                    {
                        goto putinparade;
#if 0
                        // steady ship and make sure it's pointing the right way.
                        aitrackForceSteadyShip(ship);
                        // face same direction as mothership
                        ship->rotinfo.coordsys = mothership->rotinfo.coordsys;
                        univUpdateObjRotInfo((SpaceObjRot *)ship);
#endif
                    }
                }
*/
            }
            if (ship->shiptype == DDDFrigate)
            {
                DDDFrigateDockAllDronesInstantly(ship);
            }
        }
        memFree(selectAll);
        militaryGroup = GetMilitaryGroupAroundShip(&universe.mainCommandLayer,universe.curPlayerPtr->PlayerMothership);
        if (militaryGroup) setMilitaryParade(militaryGroup);

        feScreenDelete(spHyperspaceRollCallScreen);
        spHyperspaceRollCallScreen = NULL;
    }
}

void spShipsRemainingDrawCB(featom *atom, regionhandle region)
{
    fonthandle fhSave;
    char numstr[10];

    sprintf(numstr,"%d",singlePlayerGameInfo.rollcallShipsRemaining);

    fhSave = fontCurrentGet();                              //save the current font
    fontMakeCurrent(selGroupFont2);             //select the appropriate font
    fontPrintCentreCentreRectangle(&region->rect,colWhite,numstr);
    fontMakeCurrent(fhSave);
}

void spShipsDockedDrawCB(featom *atom, regionhandle region)
{
    fonthandle fhSave;
    char numstr[10];

    sprintf(numstr,"%d",singlePlayerGameInfo.rollcallShipsDocked);

    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);             //select the appropriate font
    fontPrintCentreCentreRectangle(&region->rect,colWhite,numstr);
    fontMakeCurrent(fhSave);
}

void CalculateRollCall()
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    singlePlayerGameInfo.rollcallShipsDocked = 0;
    singlePlayerGameInfo.rollcallShipsRemaining = 0;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && (ship->shiptype != CryoTray) && (ship->shiptype != Probe) && !(ship->flags & SOF_Disabled))
        {
            if (ship->shipsInsideMe)
            {
                if (ship->shiptype != DDDFrigate)
                {
                    singlePlayerGameInfo.rollcallShipsDocked += ship->shipsInsideMe->insideList.num;
                }
            }

            if (!ShipCanHyperspace(ship))
            {
                singlePlayerGameInfo.rollcallShipsRemaining++;
            }
            else if (ship->shiptype != Mothership)  // non-mothership hyperspacecapable ships must get in range
            {
                // is ship in range?
                if (ship->specialFlags & SPECIAL_SinglePlayerWithinHyperSpaceRange)
                {
                    singlePlayerGameInfo.rollcallShipsDocked++;
                }
                else
                {
                    singlePlayerGameInfo.rollcallShipsRemaining++;
                }
            }

        }
        objnode = objnode->next;
    }


}

void TellShipsToDockForHyperspace()
{
    Node *objnode;
    Ship *ship;
    SelectCommand *selectAll;

    selectAll = selectAllCurrentPlayersNonHyperspacingShips();
    //cancel all the launch orders so we can then tell all ships to dock permanently.
    clCancelAllLaunchOrdersFromPlayer(universe.curPlayerPtr);
    if (clDock(&universe.mainCommandLayer,selectAll,DOCK_PERMANENTLY,NULL) > 0)
    {
        // moved this here cause I don't want it said if no ships are docking
        speechEventFleet(STAT_F_Hyper_Autodock, 0, universe.curPlayerIndex);
    }
    memFree(selectAll);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ship->shiptype == DDDFrigate)
            {
                DDDFrigateMakeReadyForHyperspace(ship);
            }
            if(ship->shiptype == ResearchShip)
            {
                ResearchShipMakeReadyForHyperspace(ship);
            }
        }

        objnode = objnode->next;
    }
}

bool AreNotInParadeConstraint(Ship *ship)
{
    if (ship->specialFlags & SPECIAL_SinglePlayerInParade)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
void MakeShipsNotIncludeLaunchingShips(SelectCommand *selection)
{
    sdword i;
    CommandToDo *shipcommand;
    for(i=0;i<selection->numShips;)
    {
        if(ShipCanHyperspace(selection->ShipPtr[i]))
        {
            //ship can hyperspace
            shipcommand = getShipAndItsCommand(&universe.mainCommandLayer,selection->ShipPtr[i]);
            if(shipcommand != NULL)
            {
                if(shipcommand->ordertype.order == COMMAND_LAUNCHSHIP)
                {
                    //launching ship can hyperspace...lets not cancel its
                    //orders
                    selection->numShips--;
                    selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
                    continue;
                }
            }
        }
        i++;
    }

}
void TellHyperspacingShipsToReady()
{
    SelectCommand *selectAll;
    sdword i;
    Ship *ship;

    selectAll = selectAllCurrentPlayersHyperspacingShips();
    for (i=0;i<selectAll->numShips;i++)
    {
        ship = selectAll->ShipPtr[i];
        if (ship->shiptype == Mothership)
        {
            bitSet(ship->specialFlags,SPECIAL_SinglePlayerWithinHyperSpaceRange);
        }
        else
        {
            if (shipInMilitaryParade(ship))
            {
                bitSet(ship->specialFlags,SPECIAL_SinglePlayerWithinHyperSpaceRange|SPECIAL_SinglePlayerInParade);
            }
            else
            {
                bitClear(ship->specialFlags,SPECIAL_SinglePlayerWithinHyperSpaceRange);
            }
        }
    }

    //MakeShipsOnlyFollowConstraints(selectAll,AreNotInParadeConstraint);
    MakeShipsNotIncludeLaunchingShips(selectAll);
    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,selectAll);
    clSetMilitaryParade(&universe.mainCommandLayer,selectAll);
    memFree(selectAll);
}

sdword HyperspaceRollCallBegin(regionhandle region, sdword ID, udword event, udword data)
{
    if (!spHyperspaceRollCallHandle)
    {
        feCallbackAddMultiple(spHyperspaceCallback);      //add in the callbacks
        feDrawCallbackAddMultiple(spHyperspaceDrawCallback);
#ifdef _WIN32
        spHyperspaceRollCallHandle = feScreensLoad("FEMan\\Hyperspace_Roll_Call.fib");   //load in the screen
#else
        spHyperspaceRollCallHandle = feScreensLoad("FEMan/Hyperspace_Roll_Call.fib");   //load in the screen
#endif
    }

    CalculateRollCall();
    spHyperspaceRollCallScreen = feScreenStart(region, "HyperspaceRollCall");

    return 0;
}

ShipSinglePlayerGameInfo *spNewShipSinglePlayerGameInfo()
{
    ShipSinglePlayerGameInfo *newone = memAlloc(sizeof(ShipSinglePlayerGameInfo),"shipsingleGame",0);
    memset(newone,0,sizeof(*newone));
    newone->shipHyperspaceState = SHIPHYPERSPACE_NONE;
    newone->midlevelHyperspaceIn = FALSE;
    newone->midlevelHyperspaceOut = FALSE;
    newone->staticHyperspaceGate = FALSE;
    return newone;
}

void CalculateHyperspaceTimeForShip(Ship *ship,Ship *mothership)
{
    ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
    vector mothershipheading;
    vector mothershipright;
    vector mothershipup;
    vector mothershipposition;
    vector mothershipToShip;

    if (mothership != NULL)
    {
        matGetVectFromMatrixCol3(mothershipheading,mothership->rotinfo.coordsys);
        matGetVectFromMatrixCol2(mothershipright,mothership->rotinfo.coordsys);
        matGetVectFromMatrixCol1(mothershipup,mothership->rotinfo.coordsys);
        mothershipposition = mothership->posinfo.position;
    }

    if ((mothership != NULL) && (ship != mothership))
    {
        vecSub(mothershipToShip,ship->posinfo.position,mothershipposition);
        shipSinglePlayerGameInfo->forbehind = vecDotProduct(mothershipheading,mothershipToShip);
        shipSinglePlayerGameInfo->rightleft = vecDotProduct(mothershipright,mothershipToShip);
        shipSinglePlayerGameInfo->updown = vecDotProduct(mothershipup,mothershipToShip);
    }
    else
    {
        shipSinglePlayerGameInfo->forbehind = 0.0f;
        shipSinglePlayerGameInfo->rightleft = 0.0f;
        shipSinglePlayerGameInfo->updown = 0.0f;
    }

    shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_WAITING;
    shipSinglePlayerGameInfo->hsState = HS_INACTIVE;
    shipSinglePlayerGameInfo->hsFlags = 0;

    if ((mothership == NULL) || (ship == mothership))
    {
        shipSinglePlayerGameInfo->timeToDelayHyperspace = 0.0f;
    }
    else
    {
        shipSinglePlayerGameInfo->timeToDelayHyperspace =
            HYPERSPACE_BASE_WAITTIME + frandom(HYPERSPACE_RANDOM_WAITTIME);
    }
    shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + shipSinglePlayerGameInfo->timeToDelayHyperspace;

    bitSet(ship->specialFlags,SPECIAL_Hyperspacing);
    shipHasJustDisappeared(ship);
}

void CalculateHyperspaceTimesForShips(void)
{
    Node *objnode;
    Ship *ship;
    Ship *mothership = universe.curPlayerPtr->PlayerMothership;

    dbgAssert(mothership != NULL);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ShipCanMakeHyperspace(ship))
            {
                CalculateHyperspaceTimeForShip(ship,mothership);
            }
        }

        objnode = objnode->next;
    }
}

void AddShipToHyperspace(Ship *ship)
{
    InsideShip *insideShip;

    insideShip = memAlloc(sizeof(InsideShip),"InsideHyper",0);
    insideShip->ship = ship;
    listAddNode(&singlePlayerGameInfo.ShipsInHyperspace,&insideShip->node,insideShip);
    bitSet(ship->flags,SOF_Hyperspace);

    if(singlePlayerGame)
    {
        // ships in hyperspace are instantly healed/refueled:
        //ONLY in single player game!
        ship->health = ship->staticinfo->maxhealth;
        ship->fuel = ship->staticinfo->maxfuel;
    }

    // and any ships inside as well:
    if (ship->shipsInsideMe != NULL)
    {
        // check ships inside too
        Node *insidenode = ship->shipsInsideMe->insideList.head;
        InsideShip *insideship2;
        Ship *iship;

        while (insidenode != NULL)
        {
            insideship2 = (InsideShip *)listGetStructOfNode(insidenode);
            iship = insideship2->ship;
            dbgAssert(iship->objtype == OBJ_ShipType);

            iship->health = iship->staticinfo->maxhealth;
            iship->fuel = iship->staticinfo->maxfuel;

            insidenode = insidenode->next;
        }
    }
}

bool NoneDoing(udword state)
{
    Node* objnode;
    Ship* ship;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship*)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
            ShipCanMakeHyperspace(ship) && !(ship->flags & SOF_Disabled))
        {
            if (ship->shipSinglePlayerGameInfo->hsState == state)
            {
                return FALSE;
            }
        }

        objnode = objnode->next;
    }

    return TRUE;
}

bool AllDoingExceptMe(udword state, Ship* me)
{
    Node* objnode;
    Ship* ship;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship*)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
            ShipCanMakeHyperspace(ship) && !(ship->flags & SOF_Disabled))
        {
            if (ship != me)
            {
                if (ship->shipSinglePlayerGameInfo->hsState != state)
                {
                    return FALSE;
                }
            }
        }

        objnode = objnode->next;
    }

    return TRUE;
}

bool NoneDoingHS(udword state)
{
    InsideShip* insideship;
    Node* objnode;
    Ship* ship;

    objnode = singlePlayerGameInfo.ShipsInHyperspace.head;
    while (objnode != NULL)
    {
        insideship = (InsideShip*)listGetStructOfNode(objnode);
        ship = insideship->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
            ShipCanMakeHyperspace(ship))
        {
            if (ship->shipSinglePlayerGameInfo->hsState == state)
            {
                return FALSE;
            }
        }

        objnode = objnode->next;
    }

    return TRUE;
}

bool AllDoingExceptMeHS(udword state, Ship* me)
{
    InsideShip* insideship;
    Node* objnode;
    Ship* ship;

    objnode = singlePlayerGameInfo.ShipsInHyperspace.head;
    while (objnode != NULL)
    {
        insideship = (InsideShip*)listGetStructOfNode(objnode);
        ship = insideship->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
            ShipCanMakeHyperspace(ship))
        {
            if (ship != me)
            {
                if (ship->shipSinglePlayerGameInfo->hsState != state)
                {
                    return FALSE;
                }
            }
        }

        objnode = objnode->next;
    }

    return TRUE;
}

void InstantlyHyperspaceOut(Ship *ship,Ship *mothership)
{
    CalculateHyperspaceTimeForShip(ship,mothership);

    //disappear ...
    ship->shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_GONE;
    if ((ship->flags & SOF_Hyperspace) == 0)        // extra safety for hyperspacing ships already hyperspacing
    {
        univRemoveShipFromOutside(ship);
        // ... into the hyperspace ether
        AddShipToHyperspace(ship);
    }
}

bool UpdateHyperspacingShip(Ship *ship,bool midLevel)
{
    bool hyperspacedout = FALSE;
    ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
    static sdword hsSFXHandle = -1;

    ship->autostabilizeship = FALSE;        // ship is doing something, don't want to autostabilize
    ship->shipidle = FALSE;

    shipSinglePlayerGameInfo->midlevelHyperspaceOut = midLevel;

    dbgAssert(shipSinglePlayerGameInfo);

    if ((singlePlayerGameInfo.hyperspaceFails) && (!midLevel))
    {
        // do effect state machine for hyperspace fizzling here
        switch (shipSinglePlayerGameInfo->hsState)
        {
            case HS_INACTIVE:
                if (universe.totaltimeelapsed >= shipSinglePlayerGameInfo->timeToHyperspace)
                {
                    shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_ACCELERATINGAWAY;
                    shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + HYPERSPACE_ACCELERATE_TIME;
                    clRemoveShipFromSelection((SelectAnyCommand *)&selSelected,ship);
                    clRemoveShipFromSelection((SelectAnyCommand *)&selSelecting,ship);
//                    hyperspacedout = TRUE;
//                    shipSinglePlayerGameInfo->hsState = HS_FINISHED;
                    hsStart(ship, HYPERSPACE_SLICE_RATE, TRUE, hsShouldDisplayEffect(ship));
                    if (ship == universe.curPlayerPtr->PlayerMothership)
                    {
                        hsSFXHandle = soundEvent(ship, Ship_SinglePlayerHyperspace);
                    }
                }
                break;
            case HS_SLICING_INTO:
                if ( midLevel || (NoneDoing(HS_POPUP_INTO) && NoneDoing(HS_INACTIVE)) )
                {
                    shipSinglePlayerGameInfo->hsState = HS_COLLAPSE_INTO;
                }
                break;
            case HS_COLLAPSE_INTO:
                if (hsSFXHandle != SOUND_NOTINITED)
                {
                    if (ship == universe.curPlayerPtr->PlayerMothership)
                    {
                        soundEventStopSound(hsSFXHandle, 1);
                        hsSFXHandle = SOUND_NOTINITED;
                    }
                }
                hsUpdate(ship);
                break;
            case HS_FINISHED:
                hyperspacedout = TRUE;
                break;
            default:
                hsUpdate(ship);
                break;
        }
    }
    else
    switch (shipSinglePlayerGameInfo->hsState)
    {
    case HS_INACTIVE:
        if (universe.totaltimeelapsed >= shipSinglePlayerGameInfo->timeToHyperspace)
        {
            shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_ACCELERATINGAWAY;
            shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + HYPERSPACE_ACCELERATE_TIME;
            clRemoveShipFromSelection((SelectAnyCommand *)&selSelected,ship);
            clRemoveShipFromSelection((SelectAnyCommand *)&selSelecting,ship);
            hsStart(ship, HYPERSPACE_SLICE_RATE, TRUE, hsShouldDisplayEffect(ship));
            if (!singlePlayerGame || (singlePlayerGame && (ship->playerowner != universe.curPlayerPtr)))
            {
                soundEvent(ship, Ship_Hyperdrive);
            }
        }
        break;
    case HS_SLICING_INTO:
        if ( midLevel || (NoneDoing(HS_POPUP_INTO) && NoneDoing(HS_INACTIVE)) )
        {
            hsUpdate(ship);
        }
        break;
    case HS_COLLAPSE_INTO:
        if (ship->soundevent.hyperspaceHandle != SOUND_NOTINITED)
        {
            soundEventStopSound(ship->soundevent.hyperspaceHandle, 1);
            ship->soundevent.hyperspaceHandle = SOUND_NOTINITED;
        }
        if ( (!midLevel) && (ship == universe.curPlayerPtr->PlayerMothership))
        {
            if (mrWhiteOut || AllDoingExceptMe(HS_FINISHED, ship))
            {
                mrWhiteOut = TRUE;
                mrWhiteOutT = 1.0f - shipSinglePlayerGameInfo->clipt;
                hsUpdate(ship);
            }
        }
        else
        {
            hsUpdate(ship);
        }
        break;
    case HS_FINISHED:
        //disappear ...
        shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_GONE;
        if ((ship->flags & SOF_Hyperspace) == 0)        // extra safety for hyperspacing ships already hyperspacing
        {
            univRemoveShipFromOutside(ship);

            if ((!midLevel) && (ship->playerowner != universe.curPlayerPtr))
            {
                ship->howDidIDie = DEATH_Left_Behind_At_Hyperspace;
                univDeleteWipeInsideShipOutOfExistence(ship);   // kill non player ships that went along for the ride
            }
            else
            {
                // ... into the hyperspace ether
                AddShipToHyperspace(ship);
            }
        }
        hyperspacedout = TRUE;
        break;
    default:
        hsUpdate(ship);
    }

    return hyperspacedout;
}

bool UpdateHyperspacingShips(void)
{
    Node *objnode;
    Ship *ship;
    bool shipsAllHyperspacedAway = TRUE;
    bool result;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        objnode = objnode->next;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ShipCanMakeHyperspace(ship))
            {
                result = UpdateHyperspacingShip(ship,FALSE);
                if (singlePlayerGameInfo.hyperspaceFails)
                {
                    if (!result)
                    {
                        shipsAllHyperspacedAway = FALSE;
                    }
                }
                else
                {
                    shipsAllHyperspacedAway = FALSE;
                }
            }
        }
    }

    return shipsAllHyperspacedAway;
}

bool OrientHyperspacingShips(void)
{
    Node *objnode;
    Ship *ship;
    vector mothershipheading;
    bool shipsoriented = TRUE;
    Ship *mothership = universe.curPlayerPtr->PlayerMothership;
    udword shipsNotInHyperspaceRange = 0;

    dbgAssert(mothership != NULL);

    matGetVectFromMatrixCol3(mothershipheading,mothership->rotinfo.coordsys);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ShipCanHyperspace(ship))
            {
                ship->autostabilizeship = FALSE;        // ship is doing something, don't want to autostabilize
                ship->shipidle = FALSE;

                if (ship->shiptype != Mothership)
                {
                    if (ship->specialFlags & SPECIAL_SinglePlayerWithinHyperSpaceRange)
                    {
                        //if (!(ship->specialFlags & SPECIAL_SinglePlayerInParade))
                        {
                            sdword shipready=FALSE;
                            shipready = aitrackHeadingWithBank(ship,&mothershipheading,HYPERSPACE_HEADINGACCURACY,ship->staticinfo->sinbank);
                            if(!shipready)
                            {
                                singlePlayerGameInfo.rollcallShipsRemaining++;  //add to count because we really aren't ready
                                shipsNotInHyperspaceRange++;
                            }
                            shipsoriented &= shipready;

                            //don't track zero velocity
                            //if in parade...which is everyone...lets check though

                            //aitrackSteadyShipDriftOnly(ship);
                        }
                    }
                    else
                    {
                        shipsNotInHyperspaceRange++;
                        if (aiuFindDistanceSquared(ship->posinfo.position,mothership->posinfo.position) < HYPERSPACE_RANGE)
                        {
                            CommandToDo *shipcommand = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                            if(shipcommand !=NULL)
                            {
                                if(shipcommand->ordertype.order == COMMAND_LAUNCHSHIP)
                                {
                                    goto notreadyhscrud;
                                }
                            }
                            bitSet(ship->specialFlags,SPECIAL_SinglePlayerWithinHyperSpaceRange);
notreadyhscrud:;
                        }
                        else
                        {
                            aishipFlyToShipAvoidingObjs(ship,mothership,AISHIP_PointInDirectionFlying|AISHIP_FirstPointInDirectionFlying|
                                                                        AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,0.0f);
                        }
                    }
                }
            }
        }

        objnode = objnode->next;
    }

    return shipsoriented;
}

void PutShipOutsideAt(Ship *ship,vector *createat,vector *createvelocity,real32 angle)
{
    sdword i;

    collAddSpaceObjToCollBlobs((SpaceObj *)ship);
    listAddNode(&universe.SpaceObjList,&(ship->objlink),ship);
    listAddNode(&universe.ShipList,&(ship->shiplink),ship);
    listAddNode(&universe.ImpactableList,&(ship->impactablelink),ship);
    bitClear(ship->flags, SOF_Hide);
    ship->posinfo.position = *createat;
    ship->posinfo.velocity = *createvelocity;
    ship->rotinfo.coordsys = defaultshipmatrix;

    univAddObjToRenderList((SpaceObj *)ship);

    univRotateObjYaw((SpaceObjRot *)ship,angle);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (ship->trail[i])
        {
            trailZeroLength(ship->trail[i]);
        }
    }
}

void CalculateArrivingTimesForShips()
{
    InsideShip *insideShip;
    Ship *ship;
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    struct ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo;

    dbgAssert(node != NULL);
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if ((ship->shiptype == Mothership) && (HYPERSPACE_MOTHERSHIP_ALWAYS_ARRIVES_FIRST))
        {
            shipSinglePlayerGameInfo->timeToDelayHyperspace = universe.totaltimeelapsed + HYPERSPACE_BASE_MOTHERSHIPARRIVETIME;
        }
        else
        {
            shipSinglePlayerGameInfo->timeToDelayHyperspace = universe.totaltimeelapsed + HYPERSPACE_BASE_ARRIVETIME + shipSinglePlayerGameInfo->timeToDelayHyperspace;
        }

        node = node->next;
    }
}

bool UpdateArrivingShip(Ship *ship,hvector *topoint,bool midLevel)
{
    ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
    bool thisShipArrived = FALSE;

    dbgAssert(shipSinglePlayerGameInfo);

    ship->autostabilizeship = FALSE;        // ship is doing something, don't want to autostabilize
    ship->shipidle = FALSE;

    switch (shipSinglePlayerGameInfo->shipHyperspaceState)
    {
        case SHIPHYPERSPACE_GONE:
            {
                real32 forbehind;
                real32 rightleft;
                real32 updown;
                vector tempvec;
                vector createposition;
                vector createvelocity;
                real32 angle;
                matrix rotzmat;

                // calculate position based on forbehind, rightleft
                forbehind = shipSinglePlayerGameInfo->forbehind;
                rightleft = shipSinglePlayerGameInfo->rightleft;
                updown = shipSinglePlayerGameInfo->updown;

                vecSet(tempvec,forbehind,-rightleft,updown);
                vecSet(createvelocity,0.0f,0.0f,0.0f);

                angle = DEG_TO_RAD(topoint->w);

                matMakeRotAboutZ(&rotzmat,(real32)cos(angle),(real32)sin(angle));
                matMultiplyMatByVec(&createposition,&rotzmat,&tempvec);

                vecAddTo(createposition,*topoint);

                PutShipOutsideAt(ship,&createposition,&createvelocity,angle);
                bitClear(ship->flags,SOF_Hyperspace);

                shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_DECELERATE;
                {
                    if (midLevel)
                    {
                        shipSinglePlayerGameInfo->timeToHyperspace = 0.0f;
                    }
                    else
                    {
                        real32 waittime;
                        if (ship == universe.curPlayerPtr->PlayerMothership)
                        {
                            waittime = 2.0f * HYPERSPACE_BASE_WAITTIME;
                        }
                        else
                        {
                            waittime = 3.0f * HYPERSPACE_BASE_WAITTIME + frandom(HYPERSPACE_RANDOM_WAITTIME);
                        }
                        shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + waittime;
                    }
                }

                if ((ship == universe.curPlayerPtr->PlayerMothership) && (!midLevel))
                {
                    //FocusCommand focusone;
                    //TransitionType trans;

                    //focusone.numShips = 1;
                    //focusone.ShipPtr[0] = ship;
                    //trans = universe.mainCameraCommand.transition;
                    //universe.mainCameraCommand.transition = CAMERA_SET;

                    //dbgAssert(spSavedCameraYet);
                    //ccFocus(&universe.mainCameraCommand, &focusone);
                    //universe.mainCameraCommand.currentCameraStack->remembercam = spSavedCamera;

                    //universe.mainCameraCommand.transition = trans;
                    ccFocusOnPlayersMothership(&universe.mainCameraCommand,0);
                }
            }
            if (!midLevel)
            {
                break;
            }
            // if midLevel, deliberately fall through

        case SHIPHYPERSPACE_DECELERATE:
            if (universe.totaltimeelapsed > shipSinglePlayerGameInfo->timeToHyperspace)
            {
                vecZeroVector(ship->posinfo.velocity);
                shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_DONE;
                shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + HYPERSPACE_DECELERATE_TIME;
                hsStart(ship, HYPERSPACE_SLICE_RATE, FALSE, hsShouldDisplayEffect(ship));
                if (!singlePlayerGame || (singlePlayerGame && (ship->playerowner != universe.curPlayerPtr)))
                {
                    soundEvent(ship, Ship_HyperdriveOff);
                }
            }
            break;

        case SHIPHYPERSPACE_DONE:
            mrWhiteOutT -= 3.0f * HYPERSPACE_SLICE_RATE;
            if (mrWhiteOutT < 0.0f)
            {
                mrWhiteOut = FALSE;
            }
            switch (shipSinglePlayerGameInfo->hsState)
            {
            case HS_SLICING_OUTOF:
                if ( midLevel || (NoneDoingHS(HS_POPUP_OUTOF) && NoneDoingHS(HS_FINISHED)) )
                {
                    if(!singlePlayerGame)
                    {
                        CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                        if(command != NULL)
                        {
                            if(command->ordertype.order == COMMAND_MP_HYPERSPACEING)
                            {
                                //if its a capital ship (docked ships won't have this command)
                                if(universe.totaltimeelapsed - command->hyperSpaceingTime < TW_HYPERSPACE_TELEGRAPH_WAIT_TIME)
                                {
                                    //ship hasn't waited long enough for incomming hs...
                                    //so don't
                                    goto noupdateonincomminghs;
                                }
                            }
                        }
                    }
                    hsUpdate(ship);
noupdateonincomminghs:;
                }
                break;
                    
            case HS_COLLAPSE_OUTOF:
                if (ship->soundevent.hyperspaceHandle != SOUND_NOTINITED)
                {
                    soundEventStopSound(ship->soundevent.hyperspaceHandle, 1);
                    ship->soundevent.hyperspaceHandle = SOUND_NOTINITED;
                }
                if ((!midLevel) && (ship == universe.curPlayerPtr->PlayerMothership))
                {
                    if (AllDoingExceptMe(HS_INACTIVE, ship))
                    {
                        hsUpdate(ship);
                    }
                }
                else
                {
                    if(!singlePlayerGame)
                    {
                        if(gravwellIsShipStuckForHyperspaceing(ship))
                        {
                            //ship is stuck here because of a gravwell gen somewhere
                            goto noupdateHSMP;
                        }
                    }
                    hsUpdate(ship);
noupdateHSMP:;
                }
                break;
                    
            case HS_INACTIVE:
                if (universe.totaltimeelapsed > shipSinglePlayerGameInfo->timeToHyperspace)
                {
                    thisShipArrived = TRUE;
                }
                break;
                    
            default:
                hsUpdate(ship);
                    break;
            }

        default:
            break;
    }

    if (thisShipArrived)
    {
        bitClear(ship->specialFlags,SPECIAL_Hyperspacing);
    }

    return thisShipArrived;
}

bool UpdateArrivingShips()
{
    InsideShip *insideShip;
    Ship *ship;
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    bool allShipsArrived = TRUE;
    hvector startpoint;

    GetStartPointPlayer(&startpoint);

    dbgAssert(node != NULL);
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr))
        {
            allShipsArrived &= UpdateArrivingShip(ship,&startpoint,FALSE);
        }

        node = node->next;
    }

    return allShipsArrived;
}

void GetShipsClampedToInHS(Ship *clampedto,GrowSelection *clampedships)
{
    InsideShip *insideShip;
    Ship *ship;
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;

//    dbgAssert(node != NULL);
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->clampInfo)
        {
            if ((ship->clampInfo->host == (SpaceObjRotImpTarg *)clampedto)
                && ship->shipSinglePlayerGameInfo->midlevelHyperspaceIn != TRUE)
            {
                //add ship to the ships to hs in
                growSelectAddShip(clampedships,ship);
            }
        }
        node = node->next;
    }
}

void spHyperspaceSelectionIn(SelectCommand *selection,hvector *destination)
{
    ShipSinglePlayerGameInfo *ssinfo;
    sdword i;
    sdword numShips = selection->numShips;
    GrowSelection clampedShips;

    if (numShips < 1)
    {
        return;
    }

    for (i=0;i<numShips;i++)
    {
        growSelectInit(&clampedShips);
        GetShipsClampedToInHS(selection->ShipPtr[i],&clampedShips);
        if (clampedShips.selection->numShips > 0)
        {
            spHyperspaceSelectionIn(clampedShips.selection,destination);
        }
        growSelectClose(&clampedShips);

        ssinfo = selection->ShipPtr[i]->shipSinglePlayerGameInfo;
        ssinfo->midlevelHyperspaceIn = TRUE;
        ssinfo->staticHyperspaceGate = FALSE;
        ssinfo->midlevelHyperspaceTo = *destination;
        ssinfo->timeToHyperspace += spHyperspaceDelay;
    }
}

void spHyperspaceSelectionInStatic(SelectCommand *selection,hvector *destination)
{
    ShipSinglePlayerGameInfo *ssinfo;
    sdword i;
    sdword numShips = selection->numShips;

    if (numShips < 1)
    {
        return;
    }

    for (i=0;i<numShips;i++)
    {
        ssinfo = selection->ShipPtr[i]->shipSinglePlayerGameInfo;
        ssinfo->midlevelHyperspaceIn = TRUE;
        ssinfo->staticHyperspaceGate = TRUE;
        ssinfo->midlevelHyperspaceTo = *destination;
    }
}

void GetShipsClampedTo(Ship *clampedto,GrowSelection *clampedships)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->clampInfo)
        {
            if ((ship->clampInfo->host == (SpaceObjRotImpTarg *)clampedto) && (!ShipInSelection(singlePlayerGameInfo.ShipsToHyperspace.selection,ship)))
            {
                growSelectAddShip(clampedships,ship);
            }
        }

        objnode = objnode->next;
    }
}

bool ShipAlreadyHyperspaceOut(Ship *ship)
{
    if ((ship->flags & SOF_Hyperspace) ||
        ShipInSelection(singlePlayerGameInfo.ShipsToHyperspace.selection,ship))
    {
        return TRUE;
    }

    return FALSE;
}

void spHyperspaceSelectionOut(SelectCommand *selection)
{
    Ship *ship;
    Ship *defactoMothership;
    sdword i;
    sdword numShips = selection->numShips;
    GrowSelection clampedShips;

    if (numShips < 1)
    {
        return;
    }

    defactoMothership = selection->ShipPtr[0];
    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if (!ShipAlreadyHyperspaceOut(ship))
        {
            if (ship->attributes & ATTRIBUTES_StartInHS)
            {
                InstantlyHyperspaceOut(ship,defactoMothership);
                bitClear(ship->attributes, ATTRIBUTES_StartInHS);
            }
            else
            {
                CalculateHyperspaceTimeForShip(ship,defactoMothership);
                ship->shipSinglePlayerGameInfo->timeToHyperspace += spHyperspaceDelay;
                if(ship->clampInfo != NULL)
                {
                    //ship is clamped to another ship...
                    if(ship->clampInfo->host->objtype == OBJ_ShipType)
                    {
                        if( ((Ship *)ship->clampInfo->host)->specialFlags & SPECIAL_Hyperspacing)
                        {
                            //host has been told to hyperspace
                            //so lets set the clamped ships hyperspace time, to the same time as its hose
                            ship->shipSinglePlayerGameInfo->timeToHyperspace =
                                ((Ship *)ship->clampInfo->host)->shipSinglePlayerGameInfo->timeToHyperspace;
                        }
                    }
                }
                growSelectAddShip(&singlePlayerGameInfo.ShipsToHyperspace,ship);
            }
            growSelectInit(&clampedShips);
            GetShipsClampedTo(ship,&clampedShips);
            if (clampedShips.selection->numShips > 0)
            {
                spHyperspaceSelectionOut(clampedShips.selection);
            }
            growSelectClose(&clampedShips);
        }
#ifndef HW_Release
        else
        {
            char warningmsg[200];
            sprintf(warningmsg,"Hyperspaced %s tried to hyperspace",ShipTypeToStr(ship->shiptype));
            clCommandMessage(warningmsg);
        }
#endif
    }
}

void spHyperspaceSelectionOutStatic(SelectCommand *selection)
{
    Ship *ship;
    Ship *defactoMothership;
    sdword i;
    sdword numShips = selection->numShips;
    GrowSelection clampedShips;

    if (numShips < 1)
    {
        return;
    }

    defactoMothership = selection->ShipPtr[0];
    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if (!ShipAlreadyHyperspaceOut(ship))
        {
            growSelectInit(&clampedShips);
            GetShipsClampedTo(ship,&clampedShips);
            if (clampedShips.selection->numShips > 0)
            {
                spHyperspaceSelectionOutStatic(clampedShips.selection);
            }
            growSelectClose(&clampedShips);
            if (ship->attributes & ATTRIBUTES_StartInHS)
            {
                InstantlyHyperspaceOut(ship,defactoMothership);
            }
            else
            {
                ShipSinglePlayerGameInfo* ssinfo = ship->shipSinglePlayerGameInfo;
                if (ssinfo != NULL)
                {
                    ssinfo->staticHyperspaceGate = TRUE;
                }
                CalculateHyperspaceTimeForShip(ship,defactoMothership);
                growSelectAddShip(&singlePlayerGameInfo.ShipsToHyperspace,ship);
            }
        }
#ifndef HW_Release
        else
        {
            char warningmsg[200];
            sprintf(warningmsg,"Hyperspaced %s tried to hyperspace",ShipTypeToStr(ship->shiptype));
            clCommandMessage(warningmsg);
        }
#endif
    }
}

void singlePlayerInit(void)
{
    if (singlePlayerGame)
    {
        warpLevel = WarpToLevelEnabled();

        if (warpLevel)
        {
            singlePlayerGameInfo.currentMission = warpLevel;
        }
        else
        {
            singlePlayerGameInfo.currentMission = 1;
        }
    }

    singlePlayerGameInfo.hyperspaceState = NO_HYPERSPACE;
    singlePlayerGameInfo.resourceUnitsCollected = 0;
    singlePlayerGameInfo.giveComputerFleetControl = FALSE;
    singlePlayerGameInfo.asteroid0sCanMove = FALSE;
    singlePlayerGameInfo.hyperspaceFails = FALSE;
    singlePlayerGameLoadNewLevelFlag = FALSE;

    singlePlayerGameInfo.playerCanHyperspace = FALSE;
    smUpdateHyperspaceStatus(FALSE);

    listInit(&singlePlayerGameInfo.ShipsInHyperspace);
    growSelectInit(&singlePlayerGameInfo.ShipsToHyperspace);

    if (singlePlayerGame)
    {
        objectiveStartup();
        kasInit();
    }

    HYPERSPACE_FARCLIPPLANESQR = HYPERSPACE_FARCLIPPLANE * HYPERSPACE_FARCLIPPLANE;

    HYPERSPACE_DECELERATE_DIST = 0.5f * HYPERSPACE_DECELERATION * HYPERSPACE_DECELERATE_TIME*HYPERSPACE_DECELERATE_TIME;
    HYPERSPACE_DECELERATE_INITIAL_VEL = HYPERSPACE_DECELERATION * HYPERSPACE_DECELERATE_TIME;
    dbgAssert(HYPERSPACE_DECELERATE_DIST < HYPERSPACE_FARCLIPPLANE);
    HYPERSPACE_WHIPTOWARDSDIST = HYPERSPACE_FARCLIPPLANE - HYPERSPACE_DECELERATE_DIST;
    dbgAssert(HYPERSPACE_DECELERATE_INITIAL_VEL < HYPERSPACE_WHIP_VELOCITY);
}

static udword spLockoutFlags = 0;

void spMainScreen()
{
    if (smSensorsActive)
    {
        smSensorsCloseForGood();
    }
    cmCloseIfOpen();
    rmCloseIfOpen();
    lmCloseIfOpen();
    feAllMenusDelete();                                     //make sure no menus stay up during NIS
    tbForceTaskbar(FALSE);                                  //make sure taskbar is down during NIS
}

void spLockout(udword flags)
{
    spLockoutFlags = flags;
//comment out the rest of this function to use NIS debugging keys
#if NIS_PRINT_INFO
    if (nisNoLockout)
    {
        return;
    }
#endif
    if (flags & SPLOCKOUT_DESELECT)
    {
        selSelectNone();
    }

    smSensorsDisable = TRUE;
    tbDisable = TRUE;
    if (flags & SPLOCKOUT_MR)
    {
        mrDisable();
    }
    if (flags & SPLOCKOUT_MOUSE)
    {
        mouseDisable();
    }
}

void spMainScreenAndLockout(udword flags)
{
    spMainScreen();
    spLockout(flags);
}

void spUnlockout()
{
//comment out the rest of this function to use NIS debugging keys
    smSensorsDisable = FALSE;
    tbDisable = FALSE;
    if (spLockoutFlags & SPLOCKOUT_DESELECT)
    {
        bitClear(spLockoutFlags,SPLOCKOUT_DESELECT);
    }
    if (spLockoutFlags & SPLOCKOUT_MR)
    {
        mrEnable();
        bitClear(spLockoutFlags,SPLOCKOUT_MR);
    }
    if (spLockoutFlags & SPLOCKOUT_MOUSE)
    {
        mouseEnable();
        bitClear(spLockoutFlags,SPLOCKOUT_MOUSE);
    }
}

void spHyperspaceButtonPushed(void)
{
    dbgAssert(singlePlayerGame);

    if (!singlePlayerGameInfo.playerCanHyperspace && !hyperspaceOverride)
    {
        return;
    }

    hyperspaceOverride = FALSE;

    singlePlayerGameInfo.playerCanHyperspace = FALSE;
    smUpdateHyperspaceStatus(FALSE);

    if (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE)
    {
        return;
    }

    if (universe.curPlayerPtr->PlayerMothership == NULL)
    {
        return;
    }

    if (universePause)
    {
        universePause = FALSE;      // unpause game
    }

    ccFocusOnMyMothership(&universe.mainCameraCommand);

    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_WAITINGROLLCALL;
    singlePlayerGameInfo.oldAutoLaunch = universe.curPlayerPtr->autoLaunch;     // restore autolaunch flag after hyperspace
    clAutoLaunch(FALSE,universe.curPlayerIndex);
    TellShipsToDockForHyperspace();
    TellHyperspacingShipsToReady();
    spMainScreenAndLockout(SPLOCKOUT_MR);

    HyperspaceRollCallBegin(ghMainRegion,0,0,0);
}

void spTaskBarHyperspaceCB(struct taskbutton *button, ubyte *userData)
{
    spHyperspaceButtonPushed();
}

void singlePlayerPostInit(bool loadingSaveGame)
{
    if (!loadingSaveGame)
    {
        if (singlePlayerGameInfo.currentMission == 1)
        {
            animBinkPlay(0, 1);
        }
    }
    //hyperspacetbbutton = tbButtonCreate("Hyperspace",spTaskBarHyperspaceCB,(ubyte *)0x00,0);
    //tbButtonListRefresh();
}

void singlePlayerClose(void)
{
    //dbgAssert(hyperspacetbbutton);
    //tbButtonDelete(hyperspacetbbutton);
    tbButtonListRefresh();

    kasClose();

    if (spHyperspaceRollCallHandle)
    {
        feScreensDelete(spHyperspaceRollCallHandle);
        spHyperspaceRollCallHandle = NULL;
    }
    smSensorWeirdness = FALSE;
    // turns on Fleet command again, if it's been turned off
    soundEventSetActorFlag(ACTOR_FLEETCOMMAND_FLAG, TRUE);
#if SP_DEBUGLEVEL2
    mrKASDebugDrawStates = FALSE;
    mrKASDebugDrawTimers = FALSE;
    mrKASDebugDrawVars = FALSE;
    mrKASDebugDrawVolumes = FALSE;
    mrKASSkipFactor = 0;
#endif
}

void singlePlayerSetMissionAttributes(char *directory,char *filename)
{
    singlePlayerGameInfo.onCompleteHyperspace = TRUE;     // default to TRUE
    singlePlayerGameInfo.onLoadPlayNIS = 0;
    singlePlayerGameInfo.onCompletePlayNIS = 0;
    singlePlayerGameInfo.resourceUnitsCollected = 0;
    if (universe.players[0].race == R1)
    {
        universe.players[1].race = R2;              // default to enemy race
    }
    else
    {
        universe.players[1].race = R1;              // default to enemy race
    }
    scriptSet(directory,filename,SinglePlayerScriptTable);

    //reset nebulae
    nebReset();
}

void singlePlayerLevelLoaded(void)
{
    if (singlePlayerGameInfo.currentMission == 10)
    {
        Node *objnode;
        Ship *ship;

        objnode = universe.ShipList.head;
        while (objnode != NULL)
        {
            ship = (Ship *)listGetStructOfNode(objnode);
            objnode = objnode->next;
            dbgAssert(ship->objtype == OBJ_ShipType);

            if ((ship->playerowner->playerIndex == 1) && (ship->shiptype == Carrier))
            {
                bitSet(ship->specialFlags2,SPECIAL_2_DontPlowThroughEnemyShips);
            }
        }
    }
}

void CleanupDamageEffects(void)
{
    InsideShip* insideShip;
    Ship* ship;
    Node* node = singlePlayerGameInfo.ShipsInHyperspace.head;

    while (node != NULL)
    {
        insideShip = (InsideShip*)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);
        node = node->next;

        dmgForgetEffects(ship);
    }
}

void CleanupHyperspacingShip(Ship *ship)
{
    dbgAssert(ship->shipSinglePlayerGameInfo);
    ship->shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_NONE;
}

void CleanupHyperspacingShipsThatDidntHyperspace()
{
    Node *objnode;
    Ship *ship;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        objnode = objnode->next;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ShipCanMakeHyperspace(ship))
            {
                CleanupHyperspacingShip(ship);
            }
        }
    }
}

void CleanupHyperspacingShips(bool FreeShips,bool cleanupAllPlayersShips)
{
    InsideShip *insideShip;
    Ship *ship;
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    Node *deletenode;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);
        deletenode = node;
        node = node->next;

        if (cleanupAllPlayersShips || allianceIsShipAlly(ship,universe.curPlayerPtr))
        {
            listDeleteNode(deletenode);     // put before so univFreeShipContents doesn't try to delete this twice
            if ((FreeShips) && (ship->shipSinglePlayerGameInfo->shipHyperspaceState == SHIPHYPERSPACE_GONE))
            {
                univFreeShipContents(ship);
                memFree(ship);
            }
            else
            {
                CleanupHyperspacingShip(ship);
            }
        }
    }

    if (singlePlayerGameInfo.hyperspaceFails)
    {
        CleanupHyperspacingShipsThatDidntHyperspace();
    }
}

void singlePlayerShipDied(Ship *ship)
{
    growSelectRemoveShip(&singlePlayerGameInfo.ShipsToHyperspace,ship);

    if (singlePlayerGameInfo.ShipsInHyperspace.num)
    {
        InsideShip *insideShip;
        Ship *checkship;
        Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;

        while (node != NULL)
        {
            insideShip = (InsideShip *)listGetStructOfNode(node);
            checkship = insideShip->ship;
            dbgAssert(checkship->objtype == OBJ_ShipType);

            if (checkship == ship)
            {
                listDeleteNode(node);
                break;
            }

            node = node->next;
        }
    }
}

void singlePlayerGameCleanup(void)
{
    if (!singlePlayerGame)
    {
        CleanupHyperspacingShips(TRUE,TRUE);
        return;
    }

    // delete all objectives
    objectiveShutdown();

    // remove ship lists, paths, volumes, etc.
    kasLabelledEntitiesDestroy();

    pingAllPingsDelete();

    if (singlePlayerGameLoadNewLevelFlag)
    {
        return;
    }

    spUnlockout();

    CleanupHyperspacingShips(TRUE,TRUE);

    if (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE)
    {
        if (spHyperspaceRollCallScreen != NULL)
        {
            feScreenDelete(spHyperspaceRollCallScreen);
            spHyperspaceRollCallScreen = NULL;
        }

        ccClearModeFlag(&universe.mainCameraCommand,
            CCMODE_LOCK_OUT_USER_INPUT |
            CCMODE_FOCUS_CHANGES_LOOKATPOINT_ONLY |
            CCMODE_FOCUS_CHANGES_NOTHING);

        mrWhiteOut = FALSE;
        mrWhiteOutT = 0.0f;
    }
}

void UpdateMidLevelHyperspacingShips()
{
    bool addedShipToHyperspace = FALSE;
    sdword i;

    // update any mid-level hyperspacing away ships
    for (i=0;i<singlePlayerGameInfo.ShipsToHyperspace.selection->numShips; )
    {
        if (UpdateHyperspacingShip(singlePlayerGameInfo.ShipsToHyperspace.selection->ShipPtr[i],TRUE))
        {
            addedShipToHyperspace = TRUE;
            growSelectRemoveShipIndex(&singlePlayerGameInfo.ShipsToHyperspace,i);   // deliberate don't increment i
        }
        else
        {
            i++;
        }
    }

    // update any mid-level hyperspacing in ships
    {
        Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
        Node *nextnode;
        InsideShip *insideShip;
        Ship *ship;

        while (node != NULL)
        {
            insideShip = (InsideShip *)listGetStructOfNode(node);
            ship = insideShip->ship;
            dbgAssert(ship->objtype == OBJ_ShipType);
            nextnode = node->next;

            if ((ship->attributes & ATTRIBUTES_DeleteAfterHSOut) && (ship->flags & SOF_Hyperspace))
            {
                ship->howDidIDie = DEATH_Left_Behind_At_Hyperspace;
                univDeleteWipeInsideShipOutOfExistence(ship);   // will wipe out of ShipsInHyperspace list
            }
            else if (ship->shipSinglePlayerGameInfo->midlevelHyperspaceIn)
            {
                if (UpdateArrivingShip(ship,&ship->shipSinglePlayerGameInfo->midlevelHyperspaceTo,TRUE))
                {
                    ship->shipSinglePlayerGameInfo->midlevelHyperspaceIn = FALSE;
                    CleanupHyperspacingShip(ship);
                    listDeleteNode(node);
                }
            }

            node = nextnode;
        }
    }
}

void singlePlayerGameUpdate()
{
    AIVar *tgui;
    AIVar *nisVariable = aivarFindAnyFSM("PlayNis");
    AIVar *nisletVariable = aivarFindAnyFSM("PlayNislet");
    sdword value;
    static real32 hyperspaceStartTime;

#ifdef gshaw
    if (triggerNIS)
    {
        hvector startpoint;
        char *nis, *script;
        bool centre;

        GetStartPointPlayer(&startpoint);
        singlePlayerNISNamesGet(&nis, &script, &centre, 2);
        singlePlayerStartNis(nis, script, centre, &startpoint);
        triggerNIS = FALSE;
    }
#endif

    //play an nis when the time is right
    if (nisVariable)
    {
        value = aivarValueGet(nisVariable);
        if (value != 0)
        {
            hvector startpoint;
            char *nis, *script;
            bool centre;

            GetStartPointPlayer(&startpoint);
            singlePlayerNISNamesGet(&nis, &script, &centre, value);
            singlePlayerStartNis(nis, script, centre, &startpoint);
        }
        aivarDestroy(nisVariable);
    }

    //play an nislet when the time is right
    if (nisletVariable)
    {
        value = aivarValueGet(nisletVariable);
        if (value != 0)
        {
            hvector startpoint;
            char *nis, *script;

            GetStartPointPlayer(&startpoint);
            singlePlayerNISletNamesGet(&nis, &script, value);
            singlePlayerStartNis(nis, script, FALSE, &startpoint);
        }
        aivarDestroy(nisletVariable);
    }

    //pause or unpause the NIS task.  This has to be called from within a task,
    //not the loading code, in case the player alt-tabs during loading.
    //Can you think of a better spot for it?
    nisTaskPauseOrResume();

    UpdateMidLevelHyperspacingShips();

    if (universe.curPlayerPtr->PlayerMothership == NULL)
    {
        if (singlePlayerGameInfo.hyperspaceState == HYPERSPACE_WAITINGROLLCALL)
        {
            spAbortHyperspaceCB(NULL,NULL);
        }
        return;
    }


    tgui = aivarFind("Traders.PopUpGUI");
    if (tgui)
    {
        if (tgui->value)
        {
            tgui->value = 0;
            tmTradeBegin(ghMainRegion, 0, 0, 0);
        }
    }

    switch (singlePlayerGameInfo.hyperspaceState)
    {
        case HYPERSPACE_WAITINGROLLCALL:
            CalculateRollCall();
            OrientHyperspacingShips();
            if (singlePlayerGameInfo.rollcallShipsRemaining == 0)
            {
                if (spHyperspaceRollCallScreen != NULL)
                {
                    feScreenDelete(spHyperspaceRollCallScreen);
                    spHyperspaceRollCallScreen = NULL;
                }

                //spHyperspaceCommence();   // moved code from this function to here
//              singlePlayerGameInfo.hyperspaceState = HYPERSPACE_SHIPSLEAVING;
                singlePlayerGameInfo.hyperspaceState = HYPERSPACE_WAITINGFORFLEETCOMMAND;
                singlePlayerGameInfo.hyperspaceSubState = FOCUSING_ON_MOTHERSHIP;
                singlePlayerGameInfo.hyperspaceTimeStamp = universe.totaltimeelapsed; // + HYPERSPACE_WAIT_FOR_MOTHERSHIPFOCUS_TIME;
                speechEventFleet(STAT_F_Hyper_TakingOff, 0, universe.curPlayerIndex);
                // this isn't the nicest way to do this, but it works.  I'd rather write a callback for this
                hyperspaceStartTime = universe.totaltimeelapsed + SPEECH_HYPERSPACE_DELAY;
//                soundEventStopTrack(SongNumber, SPEECH_HYPERSPACE_DELAY + 1.5f);
            }
            break;

        case HYPERSPACE_WAITINGFORFLEETCOMMAND:
            // this isn't the nicest way to do this, but it works.  I'd rather write a callback for this
            if (universe.totaltimeelapsed >= hyperspaceStartTime)
            {
                singlePlayerGameInfo.hyperspaceState = HYPERSPACE_SHIPSLEAVING;
            }
            break;

        case HYPERSPACE_SHIPSLEAVING:
            if (singlePlayerGameInfo.hyperspaceSubState == FOCUSING_ON_MOTHERSHIP)
            {
                bool shipsoriented = OrientHyperspacingShips();
                if ((universe.totaltimeelapsed >= singlePlayerGameInfo.hyperspaceTimeStamp) && (shipsoriented))
                {
                    CalculateHyperspaceTimesForShips();
                    singlePlayerGameInfo.hyperspaceSubState = LOCKED_FOCUS_FOR_HOP;
                    tutGameMessage("Game_BeginHyperspace");
//                    ccSetModeFlag(&universe.mainCameraCommand, CCMODE_FOCUS_CHANGES_NOTHING);
//                    spSavedCameraYet = FALSE;
                    if (!singlePlayerGameInfo.hyperspaceFails)
                    {
                        soundEvent(universe.curPlayerPtr->PlayerMothership, Ship_SinglePlayerHyperspace);
                        soundEventStopTrack(musicEventCurrentTrack(), 1.5f);
                    }
                }

            }
            else if (singlePlayerGameInfo.hyperspaceSubState == LOCKED_FOCUS_FOR_HOP)
            {
                if (UpdateHyperspacingShips())
                {
                    if (singlePlayerGameInfo.hyperspaceFails)
                    {
                        singlePlayerGameInfo.hyperspaceState = HYPERSPACE_FAILED;
                        goto hyperspacefailed;
                    }

                    // all ships have hyperspaced out.
                    spBinkPlay = TRUE;
                    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_WHITEOUT;
                    singlePlayerGameInfo.hyperspaceTimeStamp = universe.totaltimeelapsed + HYPERSPACE_WHITEOUTTIME;

                    speechEventStopAll(0.5f);

                    // screen should fade out
                    mrWhiteOut = TRUE;
                    mrWhiteOutT = 1.0f;

                    // save camera
                    //if (!spSavedCameraYet)
                    //{
                    //    Ship* mo = universe.curPlayerPtr->PlayerMothership;
//                        spSavedCamera = universe.mainCameraCommand.currentCameraStack->remembercam;
  //                      vecSubFrom(spSavedCamera.eyeposition, mo->posinfo.position);
    //                    vecSubFrom(spSavedCamera.lookatpoint, mo->posinfo.position);
                        //spSavedCameraYet = TRUE;
                    //}
                }
            }
            break;

        case HYPERSPACE_WHITEOUT:
            if (universe.totaltimeelapsed >= singlePlayerGameInfo.hyperspaceTimeStamp)
            {
                if (spBinkPlay)
                {
#if defined(OEM)
//no animatic between OEM Missions 4 and 5 and thereafter
                    if (singlePlayerGameInfo.currentMission < 4)
#endif
                    //playback level transition animatic
                    animBinkPlay(singlePlayerGameInfo.currentMission, singlePlayerGameInfo.currentMission + 1);

#if defined(CGW)
                    if (singlePlayerGameInfo.currentMission == 2)
                    {
                        universe.quittime = universe.totaltimeelapsed;
                        utyPlugScreens = TRUE;
                    }
#elif defined(OEM)
                    if (singlePlayerGameInfo.currentMission == 5)
                    {
                        universe.quittime = universe.totaltimeelapsed;
                        utyPlugScreens = TRUE;
                    }
#else
                    if (singlePlayerGameInfo.currentMission == 16)
                    {
                        universe.quittime = universe.totaltimeelapsed;
                        utyCreditsSequence = TRUE;
                    }
#endif
                    spBinkPlay = FALSE;
                    mrWhiteOut = FALSE;
                }
                else
                {
#if 0   /* Bink...the VOICES...MAKE IT STOP!#@$1 */
                    if (!binkDonePlaying)
                    {
                        break;
                    }
#endif

                    // clear damage effects from ships
                    CleanupDamageEffects();

                    // rebuild level now
                    singlePlayerGameLoadNewLevelFlag = TRUE;
                    singlePlayerHyperspacingInto = TRUE;
//                    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_SHIPSARRIVING;
                    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_LOADDONE;
                }
            }
            break;

        case HYPERSPACE_LOADDONE:
            soundEvent(universe.curPlayerPtr->PlayerMothership, Ship_SinglePlayerHyperspace);
//            speechEventFleet(STAT_F_HyperspaceInterrupted, 0, universe.curPlayerIndex);
            singlePlayerGameInfo.hyperspaceState = HYPERSPACE_SHIPSARRIVING;
            break;

        case HYPERSPACE_SHIPSARRIVING:
            if (UpdateArrivingShips())
            {
                // all ships have arrived and are in starting position
                singlePlayerHyperspacingInto = FALSE;
                singlePlayerGameInfo.hyperspaceState = NO_HYPERSPACE;
                CleanupHyperspacingShips(FALSE,FALSE);

                //ccClearModeFlag(&universe.mainCameraCommand,CCMODE_LOCK_OUT_USER_INPUT);

//                ccFocusOnFleetSmooth(&universe.mainCameraCommand);
                spUnlockout();

                //launch all necessary ships first
                LaunchAllInternalShipsOfPlayerThatMustBeLaunched(universe.curPlayerPtr);

                clAutoLaunch(singlePlayerGameInfo.oldAutoLaunch,universe.curPlayerIndex);
                /*
                if (!singlePlayerGameInfo.oldAutoLaunch)
                {
                    // auto launch not set to true, therefore manually launch all ships that must be:
                    LaunchAllInternalShipsOfPlayerThatMustBeLaunched(universe.curPlayerPtr);
                }
                */

                if (singlePlayerGameInfo.onLoadPlayNIS <= 0)
                {
                    soundEventPlayMusic(SongNumber);
                }

                singlePlayerKasMissionStart(singlePlayerGameInfo.currentMission);

                aiplayerGameStart(universe.players[1].aiPlayer);

                if (singlePlayerGameInfo.onLoadPlayNIS > 0)
                {
                    hvector startpoint;
                    char *nis, *script;
                    bool centre;

                    GetStartPointPlayer(&startpoint);
                    singlePlayerNISNamesGet(&nis, &script, &centre, singlePlayerGameInfo.onLoadPlayNIS);
                    singlePlayerStartNis(nis, script, centre, &startpoint);
                }
            }
            break;
hyperspacefailed:
        case HYPERSPACE_FAILED:
            {
                // all ships have arrived and are in starting position
                singlePlayerGameInfo.hyperspaceState = NO_HYPERSPACE;
                CleanupHyperspacingShips(FALSE,FALSE);

                spUnlockout();

                //launch all necessary ships first
                LaunchAllInternalShipsOfPlayerThatMustBeLaunched(universe.curPlayerPtr);

                clAutoLaunch(singlePlayerGameInfo.oldAutoLaunch,universe.curPlayerIndex);

                tutGameMessage("Game_HyperspaceFailed");
                singlePlayerGameInfo.hyperspaceFails = FALSE;
            }
            break;
            
        default:
            break;
    }
}

static void SetOnMissionCompleteInfo(char *directory,char *field,void *dataToFillIn)
{
    char str[30];
    SinglePlayerGameInfo *info = (SinglePlayerGameInfo *)dataToFillIn;

    RemoveCommasFromString(field);

    sscanf(field,"%d %s",&info->onCompletePlayNIS,str);

    if (strcmp(str,"NOHYPERSPACE") == 0)
    {
        info->onCompleteHyperspace = FALSE;
    }
    else
    {
        info->onCompleteHyperspace = TRUE;
    }
}

void DeleteAllRemainingShips()
{
    Ship *ship;
    Node *node = universe.ShipList.head;

    while (node != NULL)
    {
        ship = listGetStructOfNode(node);
        ship->howDidIDie = DEATH_Left_Behind_At_Hyperspace;
        univReallyDeleteThisShipRightNow(ship);

        node = universe.ShipList.head;  // walk the list like this because univReallyDeleteThisShipRightNow is always
                                        // modifying the list of ships as it deletes stuff
    }
}

void DeleteAnyMidLevelHyperspacingShips()
{
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    Node *nextnode;
    InsideShip *insideShip;
    Ship *ship;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);
        nextnode = node->next;

        if (ship->shipSinglePlayerGameInfo->midlevelHyperspaceOut)
        {
            ship->howDidIDie = DEATH_Left_Behind_At_Hyperspace;
            univDeleteWipeInsideShipOutOfExistence(ship);   // will wipe out of ShipsInHyperspace list
        }

        node = nextnode;
    }
}

void singlePlayerLoadNewLevel(void)
{
    real32 savephystime = universe.phystimeelapsed;
    real32 savetottime= universe.totaltimeelapsed;
    Ship *mothership = universe.players[0].PlayerMothership;

    DeleteAllRemainingShips();
    DeleteAnyMidLevelHyperspacingShips();

    rmResetStaticInfo();
    universeReset();
    univupdateReset();

    universe.phystimeelapsed = savephystime;
    universe.totaltimeelapsed = savetottime;
    universe.players[0].PlayerMothership = mothership;

    // restart computer AI player
    dbgAssert(universe.players[1].aiPlayer == NULL);
    universe.players[1].aiPlayer = aiCurrentAIPlayer = aiplayerInit(&universe.players[1],AI_ADV);
    aiCurrentAIPlayer->primaryEnemyPlayer = &universe.players[0];

    // remove sensor weirdness if it's on
    kasfSensorsWeirdness(FALSE);

#ifndef HW_Release
    if (SINGLEPLAYER_DEBUGLEVEL)
    {
        singlePlayerGameInfo.currentMission = SINGLEPLAYER_DEBUGLEVEL;
        SINGLEPLAYER_DEBUGLEVEL = 0;
    }
    else
#endif
    singlePlayerGameInfo.currentMission++;                      // go to next level

    singlePlayerGameInfo.giveComputerFleetControl = FALSE;      // reset any computer control
    singlePlayerGameInfo.asteroid0sCanMove = FALSE;

    GetMissionsDirAndFile(singlePlayerGameInfo.currentMission); //what mission file to load?
//... start of crazy starting code from utility.c
    //reset the enemy player's race to the 'opposite' (R1/R2 race)
    universe.players[1].race = (universe.players[0].race == R1) ? R2 : R1;
    //!!! should we reset the other things like RU's and OnMissionLoad stuff?
    horseRaceInit();
    levelPreInit(spMissionsDir,spMissionsFile);                 //see what ships need to be loaded and what ships we can toss

    // pause sound engine
    soundEventPause(TRUE);
    //load in all the ships, register all the textures
    universeStaticInit();

    if (hrAbortLoadingGame)
    {
        goto abortload;
    }

    trailsRecolorize();
    mistrailsRecolorize();

    SetFleetModifier(singlePlayerGameInfo.currentMission, GetFleetStrengthHyperspace);
    levelStartNext(spMissionsDir,spMissionsFile);
    clPresetShipsToPosition(&universe.mainCommandLayer);
    singlePlayerLevelLoaded();

    etgReloadReset();

    trRegistryRefresh();                                    //refresh registry - load in all textures
    //if (hrAbortLoadingGame) goto abortload;

abortload:
    //start the sound engine playing again
    soundEventPause(FALSE);

    horseRaceShutdown();
    //... end of crazy starting code from utility.c

    if (!hrAbortLoadingGame)
    {
        //ccSetModeFlag(&universe.mainCameraCommand,CCMODE_LOCK_OUT_USER_INPUT);

        univUpdateRenderList();

        CalculateArrivingTimesForShips();
    }

    bobInitProperties();

    singlePlayerGameLoadNewLevelFlag = FALSE;
}

Ship *FindPlayersMothership(Player *player)
{
    Node *objnode;
    Ship *ship;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            if (ship->shiptype == Mothership)
            {
                return ship;
            }
        }

        objnode = objnode->next;
    }

    return NULL;
}

SelectCommand *GetAllPlayersShipsExceptMothership(Player *player)
{
    Node *objnode;
    Ship *ship;
    SelectCommand *selection;
    sdword numShips = 0;

    selection = memAlloc(sizeofSelectCommand(universe.ShipList.num),"ToDoSelection",0);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            if (ship->shiptype != Mothership)
            {
                selection->ShipPtr[numShips++] = ship;
            }
        }

        objnode = objnode->next;
    }

    selection->numShips = numShips;

    return selection;
}

#define NUMBER_SINGLEPLAYER_NIS 12

#ifdef _WIN32
#define NIS_PATH "nis\\"
#else
#define NIS_PATH "nis/"
#endif

//Note: single player order has been changed.  The Previous mission 3
//      where the player meets the traders and fights the Turanic raiders
//      has been moved to mission 4.  Previous mission 4 (capture the
//      enemy ship, see the destruction of your homeworld) now happens
//      in mission 3 with a new background (you fly back to your homeworld).
//      This explains the weird ordering of the NISs
//Note2:In addition, the previous mission 15 (mining base) has been moved
//      to Mission 14, with the Headshot asteroid level (old mission 14)
//      is now Mission 15.  The only change this effects is the nislet names
char *nisR1Names[NUMBER_SINGLEPLAYER_NIS][2] =
{
    { NIS_PATH"n01r1.nis",    NIS_PATH"n01r1.script" },
#if defined (Downloadable)
    { NIS_PATH"n02.nis",      NIS_PATH"n02-demo.script" },
#else
    { NIS_PATH"n02.nis",      NIS_PATH"n02.script" },
#endif
    { NIS_PATH"n04.nis",      NIS_PATH"n04.script" },
    { NIS_PATH"n03r1.nis",    NIS_PATH"n03r1.script" },
    { NIS_PATH"n05r1.nis",    NIS_PATH"n05r1.script" },
    { NIS_PATH"n06.nis",      NIS_PATH"n06.script" },
    { NIS_PATH"n07.nis",      NIS_PATH"n07.script" },
    { NIS_PATH"n08r1a.nis",   NIS_PATH"n08r1a.script" },
    { NIS_PATH"n09.nis",      NIS_PATH"n09.script" },
    { NIS_PATH"n10r1.nis",    NIS_PATH"n10r1.script" },
    { NIS_PATH"n11r1.nis",    NIS_PATH"n11r1.script" },
    { NIS_PATH"n12r1.nis",    NIS_PATH"n12r1.script" },
};
char *singlePlayerNISName = NULL;

//See note above regarding the weird NIS ordering
char *nisR2Names[NUMBER_SINGLEPLAYER_NIS][2] =
{
    { NIS_PATH"n01r2.nis",    NIS_PATH"n01r2.script" },
    { NIS_PATH"n02.nis",      NIS_PATH"n02.script" },
    { NIS_PATH"n04.nis",      NIS_PATH"n04.script" },
    { NIS_PATH"n03r2.nis",    NIS_PATH"n03r2.script" },
    { NIS_PATH"n05r2.nis",    NIS_PATH"n05r2.script" },
    { NIS_PATH"n06.nis",      NIS_PATH"n06.script" },
    { NIS_PATH"n07.nis",      NIS_PATH"n07.script" },
    { NIS_PATH"n08r2a.nis",   NIS_PATH"n08r2a.script" },
    { NIS_PATH"n09.nis",      NIS_PATH"n09.script" },
    { NIS_PATH"n10r2.nis",    NIS_PATH"n10r2.script" },
    { NIS_PATH"n11r2.nis",    NIS_PATH"n11r2.script" },
    { NIS_PATH"n12r2.nis",    NIS_PATH"n12r2.script" },
};

#define NISLETS_PER_LEVEL       3
typedef struct
{
    char *name[NISLETS_PER_LEVEL];
    bool8 raceSpecific[NISLETS_PER_LEVEL];
}
nisletnames;

//see note above nisR1Names regarding nis\m03 being in mission 4
//in addition, Missions 14 and 15 were swapped
nisletnames nisletNames[NUMBER_SINGLEPLAYER_MISSIONS] =
{
    {{NULL, NULL, NULL},                     {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m02a", NULL, NULL},                    {FALSE,FALSE,FALSE}},
    {{NULL, NULL, NULL},                     {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m03", NULL, NULL},                     {FALSE,FALSE,FALSE}},
    {{NULL, NULL, NULL},                     {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m06a", NULL, NULL},                    {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m07a", NULL, NULL},                    {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m08a", NULL, NULL},                    {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m09a", NIS_PATH"m09b", NIS_PATH"m06a"},{FALSE,FALSE,FALSE}},
    {{NIS_PATH"m10a", NULL, NULL},                    {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m11a", NULL, NULL},                    {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m12a", NIS_PATH"m12b", NULL},          {FALSE,TRUE, FALSE}},
    {{NIS_PATH"m13a", NULL, NIS_PATH"m13c"},          {FALSE,FALSE,TRUE }},
    {{NIS_PATH"m15a", NIS_PATH"m15b", NULL},          {FALSE,FALSE,FALSE}},
    {{NIS_PATH"m14a", NULL, NULL},                    {TRUE, FALSE,FALSE}},
    {{NIS_PATH"m16a", NIS_PATH"m16a", NIS_PATH"m16a"},{FALSE,FALSE,TRUE }},
    {{NULL, NULL, NULL},                     {FALSE,FALSE,FALSE}},
};

bool8 nisPlayRelativeToMothership[NUMBER_SINGLEPLAYER_NIS] =
{
    FALSE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    TRUE
};

void *WatchFunctionAddress(sdword i)
{
    switch (i)
    {
        case 0:     return &Watch_Mission01;
        case 1:     return &Watch_Mission02;
        case 2:     return &Watch_Mission03;
        case 3:     return &Watch_Mission04;
#ifdef OEM
        case 4:     return &Watch_Mission05_OEM;
#else
        case 4:     return &Watch_Mission05;
        case 5:     return &Watch_Mission06;
        case 6:     return &Watch_Mission07;
        case 7:     return &Watch_Mission08;
        case 8:     return &Watch_Mission09;
        case 9:     return &Watch_Mission10;
        case 10:     return &Watch_Mission11;
        case 11:     return &Watch_Mission12;
        case 12:     return &Watch_Mission13;
        case 13:     return &Watch_Mission14;
        case 14:     return &Watch_Mission15;
        case 15:     return &Watch_Mission16;
#endif
        case 16:     return &Watch_Tutorial1;
    }

    return NULL;
}

sdword WatchFunctionToIndex(KASWatchFunction watchFunction)
{
    sdword i;

    for (i=0;i<NUMBER_SINGLEPLAYER_MISSIONS;i++)
    {
        if (watchFunction == WatchFunctionAddress(i))
        {
            return i;
        }
    }
    return -1;
}

KASWatchFunction IndexToWatchFunction(sdword index)
{
    if (index == -1)
    {
        return NULL;
    }

    dbgAssert(index >= 0);
    dbgAssert(index < NUMBER_SINGLEPLAYER_MISSIONS);
    return WatchFunctionAddress(index);
}

/* Get the proper FSM function list for the given mission index. */
const void **FunctionListAddress(sdword i)
{
    switch (i)
    {
        case 0:     return Mission01_FunctionPointers;
        case 1:     return Mission02_FunctionPointers;
        case 2:     return Mission03_FunctionPointers;
        case 3:     return Mission04_FunctionPointers;
#ifdef OEM
        case 4:     return Mission05_OEM_FunctionPointers;
#else
        case 4:     return Mission05_FunctionPointers;
        case 5:     return Mission06_FunctionPointers;
        case 6:     return Mission07_FunctionPointers;
        case 7:     return Mission08_FunctionPointers;
        case 8:     return Mission09_FunctionPointers;
        case 9:     return Mission10_FunctionPointers;
        case 10:     return Mission11_FunctionPointers;
        case 11:     return Mission12_FunctionPointers;
        case 12:     return Mission13_FunctionPointers;
        case 13:     return Mission14_FunctionPointers;
        case 14:     return Mission15_FunctionPointers;
        case 15:     return Mission16_FunctionPointers;
#endif
        case 16:     return Tutorial1_FunctionPointers;
    }

    return NULL;
}

/* Get the size of an FSM function list for the given mission index. */
udword FunctionListSize(sdword i)
{
    switch (i)
    {
        case 0:     return Mission01_FunctionPointerCount;
        case 1:     return Mission02_FunctionPointerCount;
        case 2:     return Mission03_FunctionPointerCount;
        case 3:     return Mission04_FunctionPointerCount;
#ifdef OEM
        case 4:     return Mission05_OEM_FunctionPointerCount;
#else
        case 4:     return Mission05_FunctionPointerCount;
        case 5:     return Mission06_FunctionPointerCount;
        case 6:     return Mission07_FunctionPointerCount;
        case 7:     return Mission08_FunctionPointerCount;
        case 8:     return Mission09_FunctionPointerCount;
        case 9:     return Mission10_FunctionPointerCount;
        case 10:     return Mission11_FunctionPointerCount;
        case 11:     return Mission12_FunctionPointerCount;
        case 12:     return Mission13_FunctionPointerCount;
        case 13:     return Mission14_FunctionPointerCount;
        case 14:     return Mission15_FunctionPointerCount;
        case 15:     return Mission16_FunctionPointerCount;
#endif
        case 16:     return Tutorial1_FunctionPointerCount;
    }

    return 0;
}

/* Get the the FSM function list for the given level index. */
const void** IndexToFunctionList(sdword index)
{
    if (index == -1)
    {
        return NULL;
    }

    dbgAssert(index >= 0);
    dbgAssert(index < NUMBER_SINGLEPLAYER_MISSIONS);
    return FunctionListAddress(index);
}

//nisplaying *singlePlayerNISPlaying = NULL;

sdword singlePlayerNisNumber = -1;
sdword singlePlayerNisletNumber = -1;


void singlePlayerKasMissionStart(sdword missionnum)
{
    switch (missionnum)
    {
        case 1: kasMissionStart("mission01", Init_Mission01, Watch_Mission01);  break;
        case 2: kasMissionStart("mission02", Init_Mission02, Watch_Mission02);  break;
        case 3: kasMissionStart("mission03", Init_Mission03, Watch_Mission03);  break;
        case 4: kasMissionStart("mission04", Init_Mission04, Watch_Mission04);  break;
#ifdef OEM
        case 5: kasMissionStart("mission05_OEM", Init_Mission05_OEM, Watch_Mission05_OEM);  break;
#else
        case 5: kasMissionStart("mission05", Init_Mission05, Watch_Mission05);  break;
        case 6: kasMissionStart("mission06", Init_Mission06, Watch_Mission06);  break;
        case 7: kasMissionStart("mission07", Init_Mission07, Watch_Mission07);  break;
        case 8: kasMissionStart("mission08", Init_Mission08, Watch_Mission08);  break;
        case 9: kasMissionStart("mission09", Init_Mission09, Watch_Mission09);  break;
        case 10: kasMissionStart("mission10", Init_Mission10, Watch_Mission10);  break;
        case 11: kasMissionStart("mission11", Init_Mission11, Watch_Mission11);  break;
        case 12: kasMissionStart("mission12", Init_Mission12, Watch_Mission12);  break;
        case 13: kasMissionStart("mission13", Init_Mission13, Watch_Mission13);  break;
        case 14: kasMissionStart("mission14", Init_Mission14, Watch_Mission14);  break;
        case 15: kasMissionStart("mission15", Init_Mission15, Watch_Mission15);  break;
        case 16: kasMissionStart("mission16", Init_Mission16, Watch_Mission16);  break;
#endif
        default:
            dbgFatalf(DBG_Loc,"\nUnsupported mission #%d",missionnum);
            break;
    }
}

bool hyperspaceAtEndOfSinglePlayerNIS = FALSE;

void RotatePlayersFleetAbout(Player *player,vector about,real32 deg)
{
    Node *objnode;
    Ship *ship;
    vector position;
    vector rotatedposition;
    matrix rotzmat;
    real32 rot = DEG_TO_RAD(deg);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            vecSub(position,ship->posinfo.position,about);

            matMakeRotAboutZ(&rotzmat,(real32)cos(rot),(real32)sin(rot));

            matMultiplyMatByVec(&rotatedposition,&rotzmat,&position);
            vecAdd(ship->posinfo.position,about,rotatedposition);

            univRotateObjYaw((SpaceObjRot *)ship,rot);
        }

        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : FindShipsOfShipTypeOfPlayer
    Description : finds ships of shiptype and owned by player, puts them in growselect
    Inputs      : growselect, shiptype, player
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FindShipsOfShipTypeOfPlayer(GrowSelection *growselect,ShipType shiptype,Player *player)
{
    Node *node;
    Ship *ship;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        ship = (Ship *)listGetStructOfNode(node);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if ((ship->playerowner == player) && (ship->shiptype == shiptype))
        {
            growSelectAddShip(growselect,ship);
        }
    }
}

void singlePlayerNisStoppedCB(void)
{
    bool savefirstlevel = FALSE;
    AIVar *var;
    char bufferstr[30];

    switch (singlePlayerNisNumber)
    {
        case 1:
        {
            GrowSelection growselect;

            universe.curPlayerPtr->PlayerMothership = FindPlayersMothership(universe.curPlayerPtr);
            dbgAssert(universe.curPlayerPtr->PlayerMothership != NULL);

            growSelectInit(&growselect);
            FindShipsOfShipTypeOfPlayer(&growselect,LightInterceptor,universe.curPlayerPtr);
            if (growselect.selection->numShips > 0)
            {
                clFormation(&universe.mainCommandLayer,growselect.selection,CUSTOM_FORMATION);
            }
            growSelectClose(&growselect);

            savefirstlevel = TRUE;
        }
        break;
    }

    soundEventPlayMusic(SongNumber);

    //set the KAS global variable when an NIS has stopped
    if (singlePlayerNisletNumber != -1)
    {                                                       //if it's an nislet
        sprintf(bufferstr,"Nislet%dComplete",singlePlayerNisletNumber);
        var = aivarCreate(bufferstr);
        aivarValueSet(var, TRUE);
    }
    else
    {
        dbgAssert(toupper(singlePlayerNISName[0]) == 'N');
        sprintf(bufferstr,"Nis%dComplete",singlePlayerNisNumber);
        var = aivarCreate(bufferstr);
        aivarValueSet(var, TRUE);
    }

    spUnlockout();

    if (hyperspaceAtEndOfSinglePlayerNIS)
    {
        hyperspaceAtEndOfSinglePlayerNIS = FALSE;
        spHyperspaceButtonPushed();
    }

    if (savefirstlevel)         // this should be at end of function
    {
        kasfSaveLevel(1, strGetString(strSaveSinglePlayerLevel1));
    }
    singlePlayerNISName = NULL;
    singlePlayerNisletNumber = -1;
    singlePlayerNisNumber = -1;
}

/*-----------------------------------------------------------------------------
    Name        : singlePlayerNISNamesGet
    Description : Figure out the name of the nis and script file for a given
                    nis number.
    Inputs      : nisNumber - number of NIS to run
    Outputs     : nisname - name of nis file
                  scriptname - name of nis script file
                  centreMothership - TRUE if the NIS is to be centred about player's mothership
    Return      : void
----------------------------------------------------------------------------*/
void singlePlayerNISNamesGet(char **nisname, char **scriptname, bool *centreMothership, sdword nisNumber)
{
    singlePlayerNisNumber = nisNumber;
    nisNumber--;
    dbgAssert(nisNumber >= 0);
    dbgAssert(nisNumber < NUMBER_SINGLEPLAYER_NIS);

    if (universe.players[0].race == R1)
    {
        *nisname = nisR1Names[nisNumber][0];
        *scriptname = nisR1Names[nisNumber][1];
    }
    else
    {
        *nisname = nisR2Names[nisNumber][0];
        *scriptname = nisR2Names[nisNumber][1];
    }
    singlePlayerNISName = *nisname;
    *centreMothership = (bool)nisPlayRelativeToMothership[nisNumber];
}

/*-----------------------------------------------------------------------------
    Name        : singlePlayerNISletNamesGet
    Description : Figure out the name of the nis and script file for a given
                    nis number.
    Inputs      : nisletNumber - number of NISlet to run
    Outputs     : nisname - name of nis file
                  scriptname - name of nis script file
    Return      : TRUE if the NIS can be played properly
----------------------------------------------------------------------------*/
bool singlePlayerNISletNamesGet(char **nisname, char **scriptname, sdword nisletNumber)
{
    sdword missionNumber, insertNumber;
    static char nisFileName[16];
    static char nisScriptName[16];

    singlePlayerNisletNumber = nisletNumber;

    missionNumber = nisletNumber / 10;
    insertNumber = nisletNumber % 10;
    missionNumber--;
    dbgAssert(missionNumber >= 0);
    dbgAssert(missionNumber < NUMBER_SINGLEPLAYER_MISSIONS);
    dbgAssert(insertNumber >= 0);
    dbgAssert(insertNumber < NISLETS_PER_LEVEL);
#if NIS_PRINT_INFO
    if (nisletNames[missionNumber].name[insertNumber] == NULL)
    {
        dbgMessagef("\nMission %d insert (%c) is not defined", missionNumber + 1, insertNumber + 'A');
        return(FALSE);
    }
#endif
    strcpy(nisFileName, nisletNames[missionNumber].name[insertNumber]);

    if (nisletNames[missionNumber].raceSpecific[insertNumber])
    {
        if (universe.curPlayerPtr->race == R1)
        {
            strcat(nisFileName, "r1");
        }
        else
        {
            strcat(nisFileName, "r2");
        }
    }
    strcpy(nisScriptName, nisFileName);
    strcat(nisFileName, ".nis");
    strcat(nisScriptName, ".script");
#if NIS_PRINT_INFO
    if (!fileExists(nisFileName, 0) || !fileExists(nisScriptName, 0))
    {
        dbgMessagef("\n'%s' and/or '%s' do not exist.", nisFileName, nisScriptName);
        return(0);
    }
#endif
    *nisname = nisFileName;
    *scriptname = nisScriptName;
    singlePlayerNISName = *nisname;
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : singlePlayerStartNis
    Description : Start a single player NIS or NISlet
    Inputs      : nisname - name of NIS file
                  scriptname - name of NIS script file
                  centreMothership - TRUE if this NIS plays relative to the
                    player's mothership (does not apply to NISlets).
                  positionAndHeading - ?
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void singlePlayerStartNis(char *nisname, char *scriptname, bool centreMothership, hvector *positionAndAngle)
{
    matrix thisIdentityMatrix;
    vector position;
    real32 angle;
    matrix rotz;
    matrix nisMatrix;
    vector motherVector;
    matrix motherMatrix;
    vector tempvec;
    Ship *nisCentreShip = NULL;
    bool centreShipFound = FALSE;
    GrowSelection *nisSelection;
    ShipRace raceOfCentreShip = (ShipRace)-1;
    ShipType typeOfCentreShip;
    sdword instanceOfCentreShip = -1;
    AIVar *nisVar;
    Node *node;
    Derelict *derelict;

    vecGrabVecFromHVec(position,*positionAndAngle);
    angle = DEG_TO_RAD(positionAndAngle->w);

    spMainScreenAndLockout(SPLOCKOUT_EVERYTHING);

    thisNisDoneCB = singlePlayerNisStoppedCB;
    thisNisHeader = nisLoad(nisname, scriptname);

    //see if we should centre NIS around a ship
    nisSelection = kasGetGrowSelectionPtrIfExists("NisCentreShip");
    if ((nisSelection) && (nisSelection->selection->numShips > 0))
    {
        nisCentreShip = nisSelection->selection->ShipPtr[0];
        kasGrowSelectionClear(nisSelection);
        typeOfCentreShip = nisCentreShip->shiptype;
    }

    //see if we should centre NIS around a derelict
    nisVar = aivarFind("NisCentreDerelict");
    if (nisVar != NULL)
    {
        raceOfCentreShip = NSR_Derelict;
        typeOfCentreShip = aivarValueGet(nisVar);
        nisCentreShip = NULL;
        //find a derelict of the specified type
        for (node = universe.DerelictList.head; node != NULL; node = node->next)
        {
            derelict = (Derelict *)listGetStructOfNode(node);
            if (derelict->derelicttype == typeOfCentreShip)
            {                                               //if this is the derelict we're looking for
                nisCentreShip = (Ship *)derelict;
                break;
            }
        }
    }
    //see if a ship instance was specified
    nisVar = aivarFind("NisCentreInstance");
    if (nisVar != NULL)
    {
        instanceOfCentreShip = aivarValueGet(nisVar);
    }

    if (nisCentreShip)
    {
        vector heading;
        vector normal = {1.0, 0.0, 0.0};
        real32 cross;

        centreShipFound = nisShipStartPosition(&motherVector, &motherMatrix, thisNisHeader, raceOfCentreShip, typeOfCentreShip, instanceOfCentreShip);
        position = nisCentreShip->posinfo.position;
        //to get the ships rotation abouts Z, get it's heading on the XY plane,
        //cross it with a 'normal' (zero degree) vector and check sign of Y to determine what to do with cross product
        matGetVectFromMatrixCol3(heading, nisCentreShip->rotinfo.coordsys);
        heading.z = 0;
        vecNormalize(&heading);
        cross = vecDotProduct(heading, normal);
        if (heading.y < 0.0f)
        {
            angle = TWOPI - (real32)acos((double)cross);
        }
        else
        {
            angle = (real32)acos((double)cross);
        }

    }
    else
    {
        if (centreMothership)
        {
            centreShipFound = nisShipStartPosition(&motherVector, &motherMatrix, thisNisHeader, (ShipRace)-1, Mothership, -1);
            dbgAssert(universe.curPlayerPtr->PlayerMothership != NULL);
            nisCentreShip = universe.curPlayerPtr->PlayerMothership;
        }
    }

    if (centreShipFound)
    {
        // we found the mothership or other centre ship, so let's play the NIS relative to it:

//        matMultiplyMatByMat(&thisIdentityMatrix,&motherMatrix,&IdentityMatrix);
//        matTranspose(&thisIdentityMatrix);
        matCopyAndTranspose(&motherMatrix, &thisIdentityMatrix);
        matMultiplyMatByMat(&nisMatrix,&nisCentreShip->rotinfo.coordsys,&thisIdentityMatrix);

        matMultiplyMatByVec(&tempvec,&nisMatrix,&motherVector);
        vecSubFrom(position,tempvec);
    }
    else
    {                                                       //play about 0,0,0
        thisIdentityMatrix = IdentityMatrix;
        matMakeRotAboutZ(&rotz,(real32)cos(angle),(real32)sin(angle));
        matMultiplyMatByMat(&nisMatrix,&thisIdentityMatrix,&rotz);
    }

    thisNisPlaying = nisStart(thisNisHeader, &position, &nisMatrix);
    nisTaskResume();
}

#define MAXLINENEED 100

sdword WarpToLevelEnabled()
{
    filehandle fh;
    char line[MAXLINENEED];
    char tempbuf[30];
    sdword status;
    sdword warpLevel;

    if (fileExists(WARP_FILENAME,0))
    {
        fh = fileOpen(WARP_FILENAME,FF_TextMode);

        status = fileLineRead(fh,line,MAXLINENEED);

        fileClose(fh);

        if (status == FR_EndOfFile)
        {
            return 0;
        }

        if (sscanf(line,"%s %d",tempbuf,&warpLevel) == 2)
        {
            if (strcasecmp("WarpTo",tempbuf) == 0)
            {
                if (warpLevel > 1)
                {
                    return warpLevel;
                }
            }
            else
            {
                return 0;
            }
        }
    }

    return 0;
}

void WarpFleetStrengthCB(char *directory,char *field,void *dataToFillIn)
{
    ShipType shiptype;
    ShipRace shiprace;
    char shiptypestr[50];
    sdword num = 0;
    ShipStaticInfo *shipstatic;

    RemoveCommasFromString(field);

    sscanf(field,"%s %d",shiptypestr,&num);

    shiptype = StrToShipType(shiptypestr);
    if (!(shiptype < TOTAL_NUM_SHIPS))
    {
        dbgFatalf(DBG_Loc,"Bad shiptype in %s",field);
    }

    if (num <= 0)
    {
        return;
    }

    if (shiptype > STD_LAST_SHIP)
    {
        shiprace = GetValidRaceForShipType(shiptype);
    }
    else
    {
        shiptype = GetAppropriateShipTypeForRace(shiptype,universe.players[0].race);
        dbgAssert(shiptype != -1);
        shiprace = universe.players[0].race;
    }

    shipstatic = GetShipStaticInfo(shiptype,shiprace);
    dbgAssert(bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded));

    spFleetStr += (GetStrengthOfShipStatic(shipstatic)*num);
}

void WarpPreLoadCB(char *directory,char *field,void *dataToFillIn)
{
    ShipType shiptype;
    ShipRace shiprace;
    char shiptypestr[50];
    sdword num = 0;

    RemoveCommasFromString(field);

    sscanf(field,"%s %d",shiptypestr,&num);

    shiptype = StrToShipType(shiptypestr);
    if (!(shiptype < TOTAL_NUM_SHIPS))
    {
        dbgFatalf(DBG_Loc,"Bad shiptype in %s",field);
    }

    if (num <= 0)
    {
        return;
    }

    if (shiptype > STD_LAST_SHIP)
    {
        shiprace = GetValidRaceForShipType(shiptype);
    }
    else
    {
        shiptype = GetAppropriateShipTypeForRace(shiptype,universe.players[0].race);
        dbgAssert(shiptype != -1);
        shiprace = universe.players[0].race;
    }

    SetInfoNeededForShipAndRelatedStaticInfo(shiptype,shiprace,TRUE);
}

void WarpFleetRUStrengthCB(char *directory,char *field,void *dataToFillIn)
{
    sdword num = 0;

    sscanf(field,"%d",&num);
    spFleetStr += GetStrengthOfRUs(num);
}

void SetWarpFleetCB(char *directory,char *field,void *dataToFillIn)
{
    Ship *ship;
    ShipType shiptype;
    ShipRace shiprace;
    char shiptypestr[50];
    sdword num = 0;
    sdword i;
    vector zeroVector = { 0.0f, 0.0f, 0.0f };
    Player *player = &universe.players[0];

    RemoveCommasFromString(field);

    sscanf(field,"%s %d",shiptypestr,&num);

    shiptype = StrToShipType(shiptypestr);
    if (!(shiptype < TOTAL_NUM_SHIPS))
    {
        dbgFatalf(DBG_Loc,"Bad shiptype in %s",field);
    }

    if (num <= 0)
    {
        return;
    }

    if (shiptype > STD_LAST_SHIP)
    {
        shiprace = GetValidRaceForShipType(shiptype);
    }
    else
    {
        shiptype = GetAppropriateShipTypeForRace(shiptype,universe.players[0].race);
        dbgAssert(shiptype != -1);
        shiprace = universe.players[0].race;
    }

    dbgAssert(universe.players[0].PlayerMothership);

    for (i=0;i<num;i++)
    {
        ship = univAddShip(shiptype,shiprace,&zeroVector,&universe.players[0],0);
        gameStatsAddShip(ship,0);
        // Update ship totals for player
        unitCapCreateShip(ship,player);

        // Update research manager ship status
        if (shiptype==ResearchShip)
        {
            if ( (player==universe.curPlayerPtr) ||
                 (player->aiPlayer!=NULL) )
            {
                rmActivateFreeLab(player);
            }
        }

        GroupShipIntoMilitaryParade(&universe.mainCommandLayer,ship,universe.players[0].PlayerMothership);
    }
}

void WarpInFleet(hvector *startPoint)
{
    // Create Mothership
    Ship *ship;
    Player *player = &universe.players[0];

    ship = univAddShip(Mothership,universe.players[0].race,(vector *)startPoint,&universe.players[0],0);
    gameStatsAddShip(ship,0);
    univRotateObjYaw((SpaceObjRot *)ship,DEG_TO_RAD(startPoint->w));

    // Update ship totals for player
    unitCapCreateShip(ship,player);

    universe.players[0].PlayerMothership = ship;

    // Now add rest of fleet in parade formation
    scriptSet(NULL,WARP_FILENAME,WarpScriptTable);

    clPresetShipsToPosition(&universe.mainCommandLayer);
}

/*-----------------------------------------------------------------------------
    Name        : singleHackResearchWarp
    Description : A function that hard codes the research a player is has already
                  had based on the settings daly told me they would be in each level
                  ugly stuff... replace with an elegant solution later
    Inputs      :
    Outputs     : Allows the player to research certain topics
    Return      : void
----------------------------------------------------------------------------*/
void singleHackResearchWarp(void)
{
    switch (warpLevel)
    {
        case 16:
        case 15:
            PlayerAcquiredTechnology("HeavyGuns");
        case 14:
            AllowPlayerToResearch("HeavyGuns");
            PlayerAcquiredTechnology("CloakDefenseFighter");
        case 13:
            PlayerAcquiredTechnology("MissileWeapons");
            AllowPlayerToResearch("CloakDefenseFighter");
        case 12:
            PlayerAcquiredTechnology("GravityWellGeneratorTech");
        case 11:
            AllowPlayerToResearch("GravityWellGeneratorTech");
            PlayerAcquiredTechnology("ProximityDetector");
            PlayerAcquiredTechnology("RepairTech");
        case 10:
            AllowPlayerToResearch("RepairTech");
            AllowPlayerToResearch("MissileWeapons");
        case 9:
            PlayerAcquiredTechnology("DDDFDFGFTech");
            PlayerAcquiredTechnology("TargetingSystems");
        case 8:
            AllowPlayerToResearch("TargetingSystems");
        case 7:
            AllowPlayerToResearch("DDDFDFGFTech");
            PlayerAcquiredTechnology("MassDrive1Mt");
            PlayerAcquiredTechnology("FireControl");
        case 6:                                       // Mission 06: GIVE: Plasma Weapons
            PlayerAcquiredTechnology("PlasmaWeapons");
            AllowPlayerToResearch("MassDrive1Mt");
            AllowPlayerToResearch("FireControl");
        case 5:                                       // Mission 05: GIVE: Capital Ship Chassis, Ion Weapons, ALLOW: Plasma Weapons
            AllowPlayerToResearch("PlasmaWeapons");
            PlayerAcquiredTechnology("Chassis3");
            PlayerAcquiredTechnology("IonWeapons");
        case 4:                                       // Mission 04: GIVE: Capital Ship Drive, ALLOW: Capital Ship Chassis
            PlayerAcquiredTechnology("MassDrive100Kt");
            AllowPlayerToResearch("Chassis3");
        case 3:                                       // Mission 03: GIVE: Corvette Chassis, Heavy Corvette
            PlayerAcquiredTechnology("Chassis2");
            PlayerAcquiredTechnology("MediumGuns");
        case 2:                                       // Mission 02: GIVE: Fighter Chassis, Corvette Drive
            PlayerAcquiredTechnology("Chassis1");
            PlayerAcquiredTechnology("MassDrive10Kt");
        case 1:                                       // Mission 01, GIVE: Fighter Drive, ALLOW: Fighter Chassis
            PlayerAcquiredTechnology("MassDrive1Kt");
            AllowPlayerToResearch("Chassis1");
    }
}


void singlePlayerStartGame(void)
{
    hvector startPoint;
    char *nis, *script;
    bool centre;

    universe.players[1].resourceUnits = 0;

    if (warpLevel)
    {
        dbgAssert(singlePlayerGameInfo.currentMission == warpLevel);

        universeSwitchToPlayer(0);

        GetMissionsDirAndFile(singlePlayerGameInfo.currentMission);

        SetFleetModifier(singlePlayerGameInfo.currentMission, GetFleetStrengthWarpScript);
        levelStartNext(spMissionsDir,spMissionsFile);

        GetStartPointPlayer(&startPoint);
        WarpInFleet(&startPoint);

        singlePlayerLevelLoaded();

        singleHackResearchWarp();

        ccFocusOnPlayersMothership(&universe.mainCameraCommand,0);

        soundEventPlayMusic(SongNumber);

        singlePlayerKasMissionStart(singlePlayerGameInfo.currentMission);

        if (singlePlayerGameInfo.onLoadPlayNIS > 0)
        {
            hvector startpoint;

            GetStartPointPlayer(&startpoint);
            singlePlayerNISNamesGet(&nis, &script, &centre, singlePlayerGameInfo.onLoadPlayNIS);
            singlePlayerStartNis(nis, script, centre, &startpoint);
        }

        return;
    }

    tmTechInit();
    spFleetModifier = 0.0f;
    GetMissionsDirAndFile(1);
    levelInit(spMissionsDir,spMissionsFile);
    singlePlayerLevelLoaded();
    singlePlayerKasMissionStart(1);
    GetStartPointPlayer(&startPoint);
    singlePlayerNISNamesGet(&nis, &script, &centre, 1);
    singlePlayerStartNis(nis, script, centre, &startPoint);
}

void singlePlayerNextLevel(void)
{
    hvector startnisAt = { 0.0f, 0.0f, 0.0f, 0.0f };
    char *nis, *script;
    bool centre;

    if (singlePlayerGameInfo.currentMission == 16)
    {
        singlePlayerMissionCompleteCB();
        return;
    }

    if (singlePlayerGameInfo.onCompletePlayNIS > 0)
    {
        singlePlayerNISNamesGet(&nis, &script, &centre, singlePlayerGameInfo.onCompletePlayNIS);
        singlePlayerStartNis(nis, script, centre, &startnisAt);
        if (singlePlayerGameInfo.onCompleteHyperspace)
        {
            hyperspaceAtEndOfSinglePlayerNIS = TRUE;
        }
        else
        {
            dbgFatal(DBG_Loc,"\nNot hyperspacing at end of level not supported yet");
        }
        return;
    }

    if (singlePlayerGameInfo.onCompleteHyperspace)
    {
        if (hyperspaceOverride)
        {
            spHyperspaceButtonPushed();
        }
    }
}

void singlePlayerMissionCompleteCB(void)
{
    selSelected.numShips = 0;
    ioUpdateShipTotals();
    mrRenderMainScreen = FALSE;
    universe.quittime = universe.totaltimeelapsed;
    // this will prevent the shiplist from being displayed!
    utyCreditsSequence = TRUE;
}

void singlePlayerMissionFailedCB(void)
{
    univKillPlayer(0,PLAYERKILLED_LOSTMISSION);
    CheckPlayerWin();
}

#if SP_DEBUGKEYS
void singlePlayerCheckDebugKeys(sdword ID)
{
#if SP_DEBUGLEVEL2
    sdword state;
#endif

    switch (ID)                 // general stuff
    {
        case RKEY:
            {
                sdword total;
                sdword numHarvestables;
                sdword numAsteroid0s;
                char totalstr[50];
                univGetResourceStatistics(&total,&numHarvestables,&numAsteroid0s);
                sprintf(totalstr,"RV=%d #R=%d #A0=%d",total,numHarvestables,numAsteroid0s);
                clCommandMessage(totalstr);
            }
            return;
    }

    if (singlePlayerGame)       // singlePlayerGame specific stuff
    switch (ID)
    {
        case CKEY:
            hyperspaceOverride = TRUE;
            singlePlayerNextLevel();
            break;

        case FKEY:
            singlePlayerMissionFailedCB();
            break;

        case EKEY:
            {
                char totalstr[50];
                GetFleetStrengthInUniverseOfPlayer(&universe.players[0]);
                sprintf(totalstr,"Fleet Str = %f",spFleetStr);
                clCommandMessage(totalstr);
            }
            break;
        case DKEY:
            kasTakeADump();
            break;

#if SP_DEBUGLEVEL2
        // debug overlays
        case SKEY: // states
            mrKASDebugDrawStates ^= TRUE;
            break;
        case TKEY: // timers
            mrKASDebugDrawTimers ^= TRUE;
            break;
        case VKEY: // vars
            mrKASDebugDrawVars ^= TRUE;
            break;
        case ZKEY: // volumes
            mrKASDebugDrawVolumes ^= TRUE;
            break;
        case AKEY: // all
            state = !(mrKASDebugDrawStates && mrKASDebugDrawTimers && mrKASDebugDrawVars && mrKASDebugDrawVolumes);
            mrKASDebugDrawStates = state;
            mrKASDebugDrawTimers = state;
            mrKASDebugDrawVars = state;
            mrKASDebugDrawVolumes = state;
            break;
        case PAGEDOWNKEY:
            mrKASSkipFactor++;
            break;
#endif //SP_DEBUGLEVEL2
    }
}
#endif //SP_DEBUGKEYS

/*-----------------------------------------------------------------------------
    Name        : spNISletTestAttempt
    Description : Attempt to test an nislet for the current level
    Inputs      : index - number of NISlet for the current level (0..2)
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#if SP_NISLET_TEST
void spNISletTestAttempt(sdword index)
{
    hvector startpoint;
    char *nis, *script;

    if (index < 0 || index >= NUMBER_SINGLEPLAYER_MISSIONS)
    {
        dbgMessage("\nindex < 0 || index >= NUMBER_SINGLEPLAYER_MISSIONS");
        return;
    }
    if (nisletNames[singlePlayerGameInfo.currentMission - 1].name[index] == NULL)
    {
        dbgMessagef("\nnisletNames[%d].name[%d] == NULL", singlePlayerGameInfo.currentMission - 1, index);
        return;
    }
    GetStartPointPlayer(&startpoint);
    singlePlayerNISletNamesGet(&nis, &script, singlePlayerGameInfo.currentMission * 10 + index);
    if (!fileExists(nis, 0))
    {
        dbgMessagef("\n'%s' not found.", nis);
        return;
    }
    if (!fileExists(script, 0))
    {
        dbgMessagef("\n'%s' not found.", script);
        return;
    }
    singlePlayerStartNis(nis, script, FALSE, &startpoint);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : spFindCameraAttitude
    Description : Find the level's default camera poistion, if any.
    Inputs      :
    Outputs     : position - position for the camera's eye point
    Return      : TRUE if the camera position could be computed, FALSE otherwise
----------------------------------------------------------------------------*/
bool spFindCameraAttitude(vector *position)
{
    hvector *hPosition = kasVectorPtrIfExists(SP_CAMERA_POINT_NAME);

    if (hPosition == NULL)
    {
        return(FALSE);
    }
    position->x = hPosition->x;
    position->y = hPosition->y;
    position->z = hPosition->z;
    return(TRUE);
}

/*=============================================================================
    Save Game Stuff
=============================================================================*/

void SaveSinglePlayerGame(void)
{
    dbgAssert(singlePlayerGameLoadNewLevelFlag == FALSE);

    SaveStructureOfSize(&singlePlayerGameInfo,sizeof(SinglePlayerGameInfo));

    SaveLinkedListOfInsideShips(&singlePlayerGameInfo.ShipsInHyperspace);
    SaveGrowSelection(&singlePlayerGameInfo.ShipsToHyperspace);
}

void LoadSinglePlayerGame(void)
{
    singlePlayerGameLoadNewLevelFlag = FALSE;

    LoadStructureOfSizeToAddress(&singlePlayerGameInfo,sizeof(SinglePlayerGameInfo));

    LoadLinkedListOfInsideShips(&singlePlayerGameInfo.ShipsInHyperspace);
    FixLinkedListOfInsideShips(&singlePlayerGameInfo.ShipsInHyperspace);

    LoadGrowSelectionAndFix(&singlePlayerGameInfo.ShipsToHyperspace);
}

