/*=============================================================================
    Name    : MultiplayerGame.h
    Purpose : definitions for MultiplayerGame.c

    Created 6/22/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/



#ifndef ___MULTIPLAYERLANGAME_H
#define ___MULTIPLAYERLANGAME_H


#include "Types.h"
#include "Region.h"
#include "LinkedList.h"
#include "UIControls.h"
#include "TitanInterfaceC.h"
#include "MultiplayerGame.h"

/*=============================================================================
    defines:
=============================================================================*/

#ifdef _WIN32
#define LG_FIBFile          "FEMan\\Multiplayer_LAN_Game.FIB"
#else
#define LG_FIBFile          "FEMan/Multiplayer_LAN_Game.FIB"
#endif
#define MAX_GAMENAME_LENGTH     MAX_TITAN_GAME_NAME_LEN
#define MAX_MAPNAME_LENGTH      MAX_MAPNAME_LEN
#define MAX_CHANNELNAME_LENGTH  MAX_CHANNEL_NAME_LEN
#define MAX_DESCRIPTION_LENGTH  MAX_CHANNEL_DESCRIPTION_LEN
#define MAX_USERNAME_LENGTH     MAX_PERSONAL_NAME_LEN
#define MAX_PASSWORD_LENGTH     MAX_PASSWORD_LEN
#define MAX_CHATSTRING_LENGTH   256
#define MAX_STATUSSTRING_LENGTH 128
#define MAX_NUM_LENGTH          12

// defines for the multiplayer game options
#define LG_Small    0
#define LG_Medium   1
#define LG_Large    2

#define LG_LastMotherShip       BIT1
#define LG_CaptureCapitalShip   BIT2
#define LG_ResearchEnabled      BIT3
#define LG_BountiesEnabled      BIT4
#define LG_HarvestinEnabled     BIT5
#define LG_ResourceInjections   BIT6
#define LG_ResourceLumpSum      BIT7
#define LG_PasswordProtected    BIT8
#define LG_UnitCapsEnabled      BIT9
#define LG_FuelBurnEnabled      BIT10

// defines for the type of packet being transfered between threads
//#define LG_QUEUEROOMNEWUSERINFO     1
//#define LG_QUEUEROOMGONEUSERINFO    2
//#define LG_QUEUEROOMCHATINFO        3
//#define LG_QUEUEPINGINFO            4
#define LG_QUEUEUSERHERE            1
#define LG_QUEUEGAMEHERE            2
#define LG_QUEUECHATMSG             3

#define LG_QUEUESTATUSINFO          5
#define LG_QUEUEGAMEPLAYER          6
#define LG_QUEUEGAMECHATINFO        7
#define LG_QUEUEGAMEDISOLVED        8

// defines for the type of message the chat is
#define LG_NORMALCHAT               1
#define LG_WHISPEREDCHAT            2
#define LG_MESSAGECHAT              3
#define LG_WRAPEDCHAT               4

/*=============================================================================
    Types:
=============================================================================*/

// type definitions for maintaining lists internal to the multiplayer game structure

typedef enum LANAdvertType {
    LANAdvertType_UserHere,
    LANAdvertType_GameHere,
    LANAdvertType_ChatMsg,
    LANAdvertType_NUMBER
} LANAdvertType;

typedef struct LANAdvertHeader {
    Address from;
    LANAdvertType type;
} LANAdvertHeader;

typedef struct LANAdvert {
    LANAdvertHeader header;
} LANAdvert;

typedef struct LANAdvert_UserHere {
    LANAdvertHeader header;
    char PersonalName[MAX_PERSONAL_NAME_LEN];
} LANAdvert_UserHere;

typedef struct LANAdvert_GameHere {
    LANAdvertHeader header;
    tpscenario game;
    CaptainGameInfo captainGameInfo;
} LANAdvert_GameHere;

typedef struct LANAdvert_ChatMsg {
    LANAdvertHeader header;
    bool whispered;
    char chatFrom[MAX_PERSONAL_NAME_LEN];
    char whisperToWhoPersonalName[MAX_PERSONAL_NAME_LEN];
    char chatstring[MAX_CHATSTRING_LENGTH];
} LANAdvert_ChatMsg;


typedef struct lgqueuegeneral
{
    uword           packettype;
}
lgqueuegeneral;

typedef struct lgqueueuserhere
{
    lgqueuegeneral  header;
    LANAdvert_UserHere userhere;
}
lgqueueuserhere;

typedef struct lgqueuegamehere
{
    lgqueuegeneral  header;
    LANAdvert_GameHere gamehere;
}
lgqueuegamehere;

typedef struct lgqueuechatmsg
{
    lgqueuegeneral  header;
    LANAdvert_ChatMsg chatmsg;
}
lgqueuechatmsg;

typedef struct lgqueuechatlist
{
    lgqueuegeneral  header;
    chatlist        chat;
}
lgqueuechatlist;

#if 0
typedef struct lgqueueuserlist
{
    lgqueuegeneral  header;
    userlist        user;
}

lgqueueuserlist;

typedef struct lgqueuenewping
{
    lgqueuegeneral  header;
    newping         ping;
}
lgqueuenewping;
#endif

typedef struct lgqueuestatusline
{
    lgqueuegeneral  header;
    statusline      status;
}
lgqueuestatusline;

typedef struct lgqueuegameplayerinfo
{
    lgqueuegeneral  header;
    gameplayerinfo  player;
}
lgqueuegameplayerinfo;


#define MAX_GAME_NAME_CHAR 42       //pad to make struct below alligned by 4




/*=============================================================================
    Function Prototypes:
=============================================================================*/

void lgStartMultiPlayerLANGameScreens(regionhandle region, sdword ID, udword event, udword data, bool AlreadyLoggedIn);
void lgShutdownMultiPlayerGameScreens(void);

void lgConnectedToChannel(bool existing);
void lgFailedToConnectToChannel(bool existing);

void lgDisplayMessage(char *message);

void lgProcessGameChatPacket(struct ChatPacket *packet);

void lgJoinGameConfirmed(void);
void lgJoinGameDenied(void);
void lgUpdateGameInfo(void);

void lgNotifyGameDisolved(void);

void lgStartup(void);
void lgShutdown(void);

//sets the game options based on gameName
void lgSetGameType(char *gameName);

void mgShowScreen(sdword screennum, bool disappear);

//misc stuff MultiPlayerGame.c needs
void lgCreateGameNow(char *name, featom *atom);
void lgBackFromOptions(char *name, featom *atom);
void lgBasicOptions(char *name,featom *atom);

// called from MG.c
void lgGoPassword(char *name, featom *atom);


/*=============================================================================
    Data:
=============================================================================*/

extern taskhandle lgProccessCallback;

extern bool lgRunning;

extern CaptainGameInfo tpGameCreated;

extern taskhandle ProccessCallback;

extern tpscenario lgMyGameInfo;

extern GameTypes *preSetGames;

#endif
