/*=============================================================================
    Name    : SinglePlayer.h
    Purpose : Definitions for SinglePlayer.c

    Created 3/3/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "Vector.h"
#include "Region.h"
#include "SpaceObj.h"
#include "KAS.h"
#include "ETG.h"

/*=============================================================================
    Switches:
=============================================================================*/

#ifndef HW_Release

#define SP_ERROR_CHECKING           1           //general error checking
#define SP_VERBOSE_LEVEL            3           //control specific output code
#define SP_NISLET_TEST              1           //test NISlets
#define SP_DEBUGKEYS                1           //single player debugging keys
#define SP_DEBUGLEVEL2              1           //more debug keys

#else //HW_Debug

#define SP_ERROR_CHECKING           0           //general error checking
#define SP_VERBOSE_LEVEL            0           //control specific output code
#define SP_NISLET_TEST              0
#define SP_DEBUGKEYS                0           //single player debugging keys
#define SP_DEBUGLEVEL2              0           //more debug keys

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
typedef enum
{
    NO_HYPERSPACE,
    HYPERSPACE_PREWAIT,
    HYPERSPACE_WAITINGROLLCALL,
    HYPERSPACE_WAITINGFORFLEETCOMMAND,
    HYPERSPACE_SHIPSLEAVING,
    HYPERSPACE_WHITEOUT,
    HYPERSPACE_LOADDONE,
    HYPERSPACE_SHIPSARRIVING,
    HYPERSPACE_FAILED
} HyperspaceState;

#define HS_INACTIVE         0
#define HS_POPUP_INTO       1
#define HS_POPUP_OUTOF      2
#define HS_SLICING_INTO     3
#define HS_SLICING_OUTOF    4
#define HS_COLLAPSE_INTO    5
#define HS_COLLAPSE_OUTOF   6
#define HS_FINISHED         7

#define HSF_WAIT            1   //wait for bit to clear before changing state

#define SP_CAMERA_POINT_NAME        "StartCameraPosition"

/*=============================================================================
    Type definitions:
=============================================================================*/
// for HYPERSPACE_SHIPSLEAVING, here are some substates:
typedef enum
{
    FOCUSING_ON_MOTHERSHIP,
    LOCKED_FOCUS_FOR_HOP
} HyperspaceSubState;

typedef struct
{
    udword currentMission;
    sdword onCompletePlayNIS;
    bool onCompleteHyperspace;

    sdword onLoadPlayNIS;

    HyperspaceState hyperspaceState;
    udword oldAutoLaunch;    //bits 0,1,2 = mothership, carrier1, carrier2
    real32 oldClipPlane;
    sdword rollcallShipsDocked;
    sdword rollcallShipsRemaining;
    HyperspaceSubState hyperspaceSubState;
    real32 hyperspaceTimeStamp;
    LinkedList ShipsInHyperspace;   // linked list of ships in hyperspace ether (uses InsideShip structure)
    sdword resourceUnitsCollected;

    GrowSelection ShipsToHyperspace;
    bool giveComputerFleetControl;
    bool asteroid0sCanMove;

    bool playerCanHyperspace;
    bool hyperspaceFails;

} SinglePlayerGameInfo;             // later save this data structure

typedef enum
{
    SHIPHYPERSPACE_NONE,
    SHIPHYPERSPACE_WAITING,
    SHIPHYPERSPACE_ACCELERATINGAWAY,
    SHIPHYPERSPACE_WHIPAWAY,
    SHIPHYPERSPACE_GONE,
    SHIPHYPERSPACE_WHIPTOWARDS,
    SHIPHYPERSPACE_DECELERATE,
    SHIPHYPERSPACE_DONE
} ShipHyperspaceState;

typedef struct ShipSinglePlayerGameInfo
{
    bool midlevelHyperspaceIn;
    bool midlevelHyperspaceOut;
    bool staticHyperspaceGate;
    hvector midlevelHyperspaceTo;
    struct Effect* hyperspaceEffect;

    ShipHyperspaceState shipHyperspaceState;
    real32 timeToHyperspace;
    real32 timeToDelayHyperspace;
    real32 forbehind;       // how far ship is forward or behind mothership
    real32 rightleft;       // how far ship is right or left of mothership
    real32 updown;          // how far ship is up or down of mothership

    real32 clipt;                   //[0..1] or -1, hyperspace clip parameter
    real32 cliptDelta;              //clipt incrementer
    udword hsState;                 //hyperspace effect state
    udword hsFlags;
} ShipSinglePlayerGameInfo;

extern SinglePlayerGameInfo singlePlayerGameInfo;
extern bool singlePlayerGameLoadNewLevelFlag;
extern bool hyperspaceFails;
extern bool singlePlayerHyperspacingInto;

void spHyperspaceButtonPushed(void);

void spHyperspaceSelectionIn(SelectCommand *selection,hvector *destination);
void spHyperspaceSelectionOut(SelectCommand *selection);
void spHyperspaceSelectionInStatic(SelectCommand *selection,hvector *destination);
void spHyperspaceSelectionOutStatic(SelectCommand *selection);

void singlePlayerInit(void);
void GetMissionsDirAndFile(sdword mission);
void singlePlayerPostInit(bool loadingSaveGame);
void singlePlayerClose(void);
void singlePlayerSetMissionAttributes(char *directory,char *filename);

void singlePlayerGameCleanup(void);

void singlePlayerGameUpdate();
void singlePlayerLoadNewLevel(void);
void singlePlayerStartGame(void);

void singlePlayerShipDied(Ship *ship);

void singlePlayerMissionCompleteCB(void);
void singlePlayerMissionFailedCB(void);

#if SP_DEBUGKEYS
void singlePlayerCheckDebugKeys(sdword ID);
#endif
void GetMissionsDirAndFile(sdword mission);

void spMainScreen();

bool GetStartPointPlayer(hvector *startpoint);
bool GetPointOfName(hvector *point,char *name);

ShipSinglePlayerGameInfo *spNewShipSinglePlayerGameInfo();

// flags for spLockout
#define SPLOCKOUT_MOUSE     1
#define SPLOCKOUT_MR        2
#define SPLOCKOUT_DESELECT  4
#define SPLOCKOUT_EVERYTHING    (SPLOCKOUT_MOUSE+SPLOCKOUT_MR+SPLOCKOUT_DESELECT)

void spMainScreen();
void spLockout(udword flags);
void spMainScreenAndLockout(udword flags);
void spUnlockout();

void LoadSinglePlayerGame(void);
void SaveSinglePlayerGame(void);

sdword WatchFunctionToIndex(KASWatchFunction watchFunction);
KASWatchFunction IndexToWatchFunction(sdword index);

udword FunctionListSize(sdword i);
const void** IndexToFunctionList(sdword index);

#if SP_NISLET_TEST
void spNISletTestAttempt(sdword index);
#endif

bool singlePlayerNISletNamesGet(char **nisname, char **scriptname, sdword nisletNumber);
bool spFindCameraAttitude(vector *position);

void UpdateMidLevelHyperspacingShips();

void singlePlayerPreLoadCheck(void);
bool isShipSinglePlayerHyperspaceable(Ship *ship);

//stores the current mission filename
extern char CurrentLevelName[];

/*=============================================================================
    Data:
=============================================================================*/

extern sdword SINGLEPLAYER_STARTINGRUS;
extern char spMissionsDir[];
extern char spMissionsFile[];

extern real32 spFleetModifier;

extern real32 spHyperspaceDelay;
extern bool   spHoldHyperspaceWindow;

extern real32 HYPERSPACEGATE_HEALTH;
extern real32 HYPERSPACEGATE_WIDTH;
extern real32 HYPERSPACEGATE_HEIGHT;
extern real32 HYPERSPACEGATE_WAYPOINTDIST;

extern real32 SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE;

