/*
 * SNTypes.h
 *
 * Author:        John W. Fawcett
 * Date Created:  08/06/96
 *
 * Purpose:
 *  Public type declarations for SIGS
 *
 * Copyright (c) 1996 by Sierra On-Line, Inc.
 * All Rights Reserved.
 *
 * $Header: /export/home/rcs/snlib1.0//include/SNTypes.h,v 1.9 1996/09/10 19:02:05 abakken Exp $
 *
 * $Log: SNTypes.h,v $
 * Revision 1.9  1996/09/10  19:02:05  abakken
 * Added the JoinInProgressFlag for games
 *
 * Revision 1.8  1996/09/09  18:28:49  jepson
 * Added const defintions from SNGen.h
 * Changed Invite Permissions options
 *
 * Revision 1.7  1996/09/04  00:01:25  abakken
 * dont use UINT
 *
 * Revision 1.6  1996/09/03  23:07:38  abakken
 * Use LOGICAL instead of BOOL
 *
 * Revision 1.5  1996/08/26  16:35:31  abakken
 * sync with the PC side
 *
 * Revision 1.4  1996/08/07  20:08:50  abakken
 * expanded the game state variables
 *
 * Revision 1.3  1996/08/07  14:53:41  fawcett
 * Added struct for passing PublicInfo back to the application
 *
 * Revision 1.2  1996/08/06  22:14:55  abakken
 * fixed the header
 *
 *
 */

#ifndef _SN_TYPES_H_
#define _SN_TYPES_H_

// this is to allow for one function decleration
// DO NOT!! set the MAKE_DLL parameter in your mak file.
#ifdef MAKE_DLL
	#define SIGSAPI __declspec( dllexport )
	#define SIGSCALL __cdecl
	#define DllExport __declspec( dllexport )
#else
	#define SIGSAPI __declspec( dllimport )
	#define SIGSCALL __cdecl
#endif

#define HEADER_DATATYPE unsigned short

// Data types
typedef short           LOGICAL;
typedef LOGICAL         SN_FLAG;
typedef char*           PSTR;
typedef const char*     PCSTR;
typedef long            CUSTID;
typedef CUSTID          PLAYERID;
typedef CUSTID          USERID;
typedef unsigned char   SN_CODE;
typedef short           GPNUM;
typedef long            CYBERCASH;
typedef long            ROOMNUM;
typedef long            GAMENUM;
typedef GAMENUM         SN_SERVICE;
typedef long            PORTNUM;
typedef short           SERVERSITE;
typedef short           PREQUALNUM;

// String lengths
#define ADDRESS_LEN                60
#define CITY_LEN                   50
#define COUNTRY_LEN                30
#define STATE_LEN                  4
#define USER_NAME_LEN              25


// Logical values
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef  min
#define min(a, b)       ((a) < (b) ? (a) : (b))
#endif

#ifndef  max
#define max(a, b)       ((a) < (b) ? (b) : (a))
#endif

// Status values
#define SN_OK  0
#define SN_ERR  -1
#define SN_TIMEOUT  -2
#define RETURN_OK  SN_OK
#define RETURN_ERR  SN_ERR
#define RETURN_TIMEOUT  SN_TIMEOUT

// Age Codes
#define AC_LESS_7  1
#define AC_7_12  2
#define AC_13_18  3
#define AC_19_25  4
#define AC_26_40  5
#define AC_41_55  6
#define AC_55_GREATER  7
#define AC_NA  99

// Gender codes
#define GC_MALE  1
#define GC_FEMALE  2
#define GC_NA  9

// Internet service types
#define IS_OTHER  0x01
#define IS_MSN  0x02
#define IS_AOL  0x04
#define IS_WOW  0x08
#define IS_COMPUSERVE  0x10
#define IS_PRODIGY  0x20
#define IS_GENIE  0x40

// Modem speed codes
#define MS_2400  1
#define MS_9600  2
#define MS_14400  3
#define MS_28800  4
#define MS_HIGH  5
#define MS_NA  9

// Processor codes
#define PC_386  1
#define PC_486  2
#define PC_PENTIUM  3
#define PC_PENTIUM_PRO  4
#define PC_OTHER  9

// RAM amount codes
#define RA_LESS_8  1
#define RA_8_12  2
#define RA_12_GREATER  3

// Monitor codes
#define MC_14  1
#define MC_15  2
#define MC_17  3
#define MC_LAPTOP  4

// Join and Watch Permissions
#define PM_OFF  1
#define PM_NONE  2
#define PM_ONE  3
#define PM_CAPTAIN  4
#define PM_GAME_OWNER  5
#define PM_MAJORITY  6
#define PM_ALL  7

// Invite Permissions
#define IP_NONE  1
#define IP_GAME_OWNER  2
#define IP_CAPTAIN  3
#define IP_PLAYER  4
#define IP_ALL  5

// Join In Progress
#define JIP_OFF  0
#define JIP_ON   1

// Game Groups
#define GG_ADVENTURE  1
#define GG_ACTION  2
#define GG_SIMULATION  3
#define GG_STRATEGY  4
#define GG_ROLE_PLAYING  5
#define GG_SPORTS  6
#define GG_EDUCATION  7

typedef struct tPublicInfo {
    char UserName[USER_NAME_LEN];
    char City[CITY_LEN];
    char State[STATE_LEN];
    char Country[COUNTRY_LEN];
    LOGICAL HasCD;
    LOGICAL HasMike;
    SN_CODE AgeGroup;
    SN_CODE GenderGroup;
    SN_CODE ModemSpeedGroup;
    SN_CODE MonitorGroup;
    SN_CODE RamCodeGroup;
    SN_CODE ProcessorGroup;
} sPublicInfo;

// Game state parameters
#define PARAM_JOIN_PERMISSION    1
#define PARAM_INVITE_PERMISSION  2
#define PARAM_WATCH_PERMISSION   3
#define PARAM_JOIN_IN_PROGRESS   4

// API callback function type
#define SIGSCALLBACK __cdecl

// Possible values for CheckPlayerConnectStatus()
#define _GAME_STAT_NEWPLAYER  2
#define _GAME_STAT_SUCCESS   1
#define _GAME_STAT_PENDING   0
#define _GAME_STAT_FAILURE  -1
#define _NO_NEW_PLAYER_FOUND  -1

typedef enum PingMethod
{
	PING_LOBBY_SERVER = 0,
	PING_ROOM_SERVER,
	PING_GAME_SERVER,
	PING_PLAYER
} PING_METHOD;

// Information for "TellMe" functions:
// Define your callback functions as SIGSCALLBACK and include SNTypes.h

//typedef of enum used to flag notification method in "TellMe" functions
typedef enum NotifyMethod
{
	SIGS_DEFAULT_QUERY = 0,
	SIGS_CALLBACK_FUNCTION,
	SIGS_POST_MESSAGE,
	SIGS_EVENT_HANDLE
} NOTIFY_METHOD;

//typedef for TellMeGameConnected() callback function
typedef void (SIGSCALL *GAME_CONNECT_PROC)(LOGICAL);

//typedef for TellMeUserDisconnected() & TellMePlayerLeftGame() callback functions
typedef void (SIGSCALL *PLAYER_STATUS_PROC)(unsigned int);

//typedef for TellMePlayerJoined() callback function
typedef void (SIGSCALL *NEW_PLAYER_PROC)(void);

//typedef for TellMeUserPublicInfo callback function
typedef void (SIGSCALL *PUBLIC_INFO_PROC)(struct tPublicInfo *PublicInfo);

//a pointer to this structure will be the third parameter when a message is
//posted (WPARAM) and the structure will be the second parameter when a callback
// is used
//for TellMeGameMessReceived and TellMeChatMessReceived functions
typedef struct tMessInfo
{
	int iPlayerIndex;
	int iLength;
} sMessInfo;

typedef void (SIGSCALL *MESS_RECEIVED_PROC)(char *, sMessInfo);

typedef enum SpecialChatType {
	SYSOP_AD = 0,
	SYSOP_BROAD,
	PRIVATE,
	SYSOP_PRIVATE
} SP_CHAT_TYPE;

typedef struct tSpecialChat
{
	char Sender[USER_NAME_LEN];
	int  MsgColor;
	SP_CHAT_TYPE MsgType;
} sSpecialChat;

typedef void (SIGSCALL *SPECIAL_CHAT_PROC)(char *, sSpecialChat);

//a pointer to this struct will be passed by the game to GetUDPPlayerData
//and will be filled in by SIGS
typedef struct tUDPPlayer {
	unsigned long  ipAddr;
	int  idPort;
	int  iNameLength;
	char cName[USER_NAME_LEN];
} sUDPPlayer;

//for use with game messages
struct tMessageObject
{
	int iTeamNum;
	int iTeamPos;
	int iBufLen;
	char *MessageBuffer;
};

#endif // _SN_TYPES_H_

