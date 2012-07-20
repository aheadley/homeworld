/*=============================================================================
    Name    : Globals.h
    Purpose : Definitions for Globals.c

    Created 7/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___GLOBALS_H
#define ___GLOBALS_H

#include "Types.h"
#include "Tweak.h"
#include "color.h"
#include "ObjTypes.h"
#include "TitanInterfaceC.h"
#include "Queue.h"
#include "MaxMultiplayer.h"

//define to include a number of checks to see if regions are valid
#ifndef HW_Release
    #define DEBUG_STOMP
#endif

#ifndef HW_Release
#define GOD_LIKE_SYNC_CHECKING
//remove if we don't want this logging capability anymore
#define DEBUG_GAME_STATS
#endif

#define OPTIMIZE_VERBOSE            //certain optimizing features to help optimizing



/*=============================================================================
    Defines:
=============================================================================*/

#define MAX_RACES 2

// defines for startingGameState
#define CHECK_AUTODOWNLOAD_MAP          1
#define AUTODOWNLOAD_MAP                2
#define PROCESS_TEXTURES                4
#define GAME_STARTED                    5

#define MAX_SCENARIOSTR_LENGTH          50

#define MAX_MESSAGE_LENGTH              100
#define MAX_MESSAGES                    5

#define MAX_BIGMESSAGE_LENGTH           100
#define MAX_BIGMESSAGES                 2

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    real32 MessageExpire;
    char   message[MAX_MESSAGE_LENGTH];
} gMessageType;

typedef struct
{
    bool messageOn;
    char message[MAX_BIGMESSAGE_LENGTH];
} bMessageType;

/*=============================================================================
    Globals
=============================================================================*/

#ifdef OPTIMIZE_VERBOSE
extern sdword vecNormalizeCounter;
#endif

extern udword startingGameState;
extern bool HaveSentNonCaptainReadyPacket;

extern bool gameIsRunning;
extern bool multiPlayerGame;
extern bool multiPlayerGameUnderWay;

extern bool singlePlayerGame;
extern bool objectivesShown;

#define TUTORIAL_ONLY           1
#define TUTORIAL_SINGLEPLAYER   2
extern sdword tutorial;
extern bool tutorialdone;

#define PLAYER_QUIT             6
#define PLAYER_DROPPED_OUT      7
extern bool playersReadyToGo[MAX_MULTIPLAYER_PLAYERS];

#define playerHasDroppedOutOrQuit(index) ((playersReadyToGo[index] == PLAYER_DROPPED_OUT) || (playersReadyToGo[index] == PLAYER_QUIT))

extern uword numPlayers;
extern uword curPlayer;

extern bool startingGame;
//extern bool nisEnabled;

extern void *gMessageMutex;
extern gMessageType gMessage[MAX_MESSAGES+1];

extern bMessageType bMessage[MAX_BIGMESSAGES];

extern bool smSensorsActive;

extern udword logEnable;
extern char logFilePath[];


#ifdef GOD_LIKE_SYNC_CHECKING
extern bool syncDumpOn;
extern sdword syncDumpWindowSize;
extern sdword syncDumpGranularity;
extern sdword syncDumpWindowPos;
extern sdword syncDumpGranTrack;
#endif

#ifdef DEBUG_GAME_STATS
extern bool statLogOn;
#endif

extern bool smGhostMode;

extern char playerNames[MAX_MULTIPLAYER_PLAYERS][MAX_PERSONAL_NAME_LEN];

extern Queue ProcessSyncPktQ;
extern Queue ProcessRequestedSyncPktQ;
extern Queue ProcessCmdPktQ;
extern Queue ProcessCaptaincyPktQ;

extern sdword captainProposal;
extern sdword captainTransferState;
extern sdword receiveSyncPacketsFrom;

extern bool hrAbortLoadingGame;

extern bool explicitlyRequestingPackets;
extern udword explicitlyRequestingFrom;
extern udword explicitlyRequestingTo;

extern bool8 ComputerPlayerOn[MAX_MULTIPLAYER_PLAYERS];

extern bool startRecordingGameWhenSafe;

/*=============================================================================
    Public Macros:
=============================================================================*/

#define LOG_OFF     0
#define LOG_ON      1
#define LOG_VERBOSE 2

/*=============================================================================
    Functions:
=============================================================================*/

void globalsResetFunc(bool firstTime);
#define globalsReset()  globalsResetFunc(0)
#define globalsInit() globalsResetFunc(1)

void globalsClose();

#endif

