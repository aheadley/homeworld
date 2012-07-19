/*=============================================================================
    Name    : ConsMgr.h
    Purpose : Definitions for the contruction manager.

    Created 7/18/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___CONSMGR_H
#define ___CONSMGR_H

#include "types.h"
#include "region.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define CM_ERROR_CHECKING       1               //general error checking
#define CM_VERBOSE_LEVEL        2               //print extra info
#define CM_CAP_JOBS_PER_CLASS   0
#define CM_CHEAP_SHIPS          1               //enable the 'cheapShips' flag

#else //HW_Debug

#define CM_ERROR_CHECKING       0               //general error checking
#define CM_VERBOSE_LEVEL        0               //print extra info
#define CM_CAP_JOBS_PER_CLASS   0

//personal addition of cheapships so I can build special crud in release build
#ifndef HW_Release
    #ifdef bpasechnik
        #undef  CM_CHEAP_SHIPS
        #define CM_CHEAP_SHIPS      1           //enable typing in cheats on the keyboard
    #endif
#endif

#ifdef ddunlop
    #ifndef HW_Release
        #undef  CM_CHEAP_SHIPS
        #define CM_CHEAP_SHIPS      1           //enable typing in cheats on the keyboard
    #endif
#endif

#endif //HW_Debug

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure for a ship under construction
typedef struct
{
    sdword nJobs;
    sdword nJobsTotal;
    sdword timeStart;
    sdword timeLeft;
    sdword timeFraction; // Simulates floating point so that fraction is accounted for in decrement
    sdword selected;
    sdword paused;
    sdword costSoFar;
    ShipStaticInfo *info;
}
shipinprogress;
//structure for all ships under construction at a certain site
typedef struct
{
    Node node;
    struct Ship *ship;
    bool canBuildBigShips;
    shipinprogress progress[TOTAL_STD_SHIPS];
}
shipsinprogress;

//ships available
typedef struct
{
    sdword nJobs;
    sbyte  itemtype;
    sbyte  itemstat;
    ShipType type;
    ShipStaticInfo *info;
}
shipavailable;



typedef struct
{
    sword       key;
    ShipType    ship;
}
KeysToShips;


typedef struct
{
    struct Ship   *ship;
    struct Player *owner;
}
CarrierInfo;


/*=============================================================================
    Data:
=============================================================================*/

extern CarrierInfo *cmCarriers;
extern sdword cmNumCarriers;
extern sdword cmNumPlayersCarriers;


#if CM_CHEAP_SHIPS
extern bool cmCheapShips;
#endif
#define CM_SHIFT 256
extern KeysToShips cmShipTypes[];
extern bool cmPrintHotKey;

extern sdword shiplagtotals[TOTAL_NUM_SHIPS];

extern bool cmActive;

extern sdword cmMaxJobsPerClass[NUM_CLASSES];

/*=============================================================================
    Functions:
=============================================================================*/
void cmStartup(void);
void cmShutdown(void);
void cmLoadTextures(void);
void cmCloseTextures(void);

// cancels all jobs of a certain shiptype
void cmForceBuildShipType(ShipType type);

void cmReset(void); // Free carrier ship pointers used by construction manager

//start the construction manager.  It will kill itself when you hit the quit button.
sdword cmConstructionBegin(regionhandle region, sdword ID, udword event, udword data);

//start a new build job
void cmBuildJobsAdd(shipsinprogress *factory, ShipStaticInfo *info, sdword index);

void cmAddFactory(struct Ship *ship,bool canBuildBigShips);
void cmRemoveFactory(struct Ship *ship);

void cmCloseIfOpen(void);

bool cmCanBuildShipType(Ship *factoryship,ShipType shiptype,bool checkResearch);

bool cmBuildHotKey(keyindex key, bool shift);
ShipType cmKeyToShipType(uword key);
uword cmShipTypeToKey(ShipType ship);

void cmUpdateShipsAvailable(void);

void cmPauseAllJbos(void);
void cmUnPauseAllJobs(void);

#define CMD_START   1
#define CMD_PAUSE   2
#define CMD_UNPAUSE 3
#define CMD_CANCEL  4

void cmDeterministicBuild(udword command, sdword numShips,
                          ShipType shipType, ShipRace shipRace,
                          uword playerIndex, ShipPtr creator);
void cmDeterministicBuildProcess(void);
void cmDeterministicBuildDisplay(void);
void cmDeterministicReset(void);
void cmBuildTaskFunction(void);

void LoadConsMgrDetermOptional();
void SaveConsMgrDetermOptional();

#endif //___CONSMGR_H
