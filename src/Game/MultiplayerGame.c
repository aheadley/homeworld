/*=============================================================================
    Name    : MultiplayerGame.c
    Purpose : Logic for the multiplayergame setup.

    Created 6/22/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define _GNU_SOURCE   /* Get to wcscasecmp() */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#ifndef _MACOSX
    #include <wchar.h>
#endif

#include "MultiplayerGame.h"
#include "MultiplayerLANGame.h"
#include "FEFlow.h"
#include "utility.h"
#include "ScenPick.h"
#include "UIControls.h"
#include "TitanInterfaceC.h"
#include "TitanNet.h"
#include "Region.h"
#include "mouse.h"
#include "FontReg.h"
#include "Scroller.h"
#include "LinkedList.h"
#include "prim2d.h"
#include "Randy.h"
#include "Chatting.h"
#include "CommandNetwork.h"
#include "Globals.h"
#include "msg/ServerStatus.h"
#include "ChannelFSM.h"
#include "ColPick.h"
#include "mainswitches.h"
#include "Chatting.h"
#include "Strings.h"
#include "Queue.h"
#include "Titan.h"
#include "File.h"
#include "StatScript.h"
#include "TitanNet.h"
#include "Universe.h"
#include "GameChat.h"
#include "SaveGame.h" // for ConvertNumToPointerInList

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define wcscasecmp wcsicmp
#endif

#ifdef HW_Debug
#ifdef gshaw
#define NEED_MIN_TWO_HUMAN_PLAYERS  0
#endif
#endif

/*=============================================================================
    Defines:
=============================================================================*/

#ifdef _WIN32
#define MG_FIBFile_SERVERS      "FEMan\\Choose_Server.FIB"
#else
#define MG_FIBFile_SERVERS      "FEMan/Choose_Server.FIB"
#endif

#define MG_FontNameLength       64

#define MG_VertSpacing          (fontHeight(" ") >> 1)
#define MG_HorzSpacing          (fontWidth(" "))

// defines for type of sort for the list of games window
#define MG_SortByGameName       0
#define MG_SortByPingTime       1
#define MG_SortByNumPlayers     2
#define MG_SortByMapName        3

#define MG_ServerSortByName         0
#define MG_ServerSortByDescription  1
#define MG_ServerSortByPing         2
#define MG_ServerSortByLoad         3
#define MG_ServerSortByReliability  4

// defines for the type of WON query
#define MG_WONLogin             1
#define MG_WONNewUser           2
#define MG_WONChangePassword    3

#define PING_INTERVAL_TIME      1.0f

//task parameters
#define MG_TaskStackSize        4096
#define MG_TaskServicePeriod    0.25f

// maximum chat history list window items
#define MG_MaxChatHistory       300

#define MG_QUEUEBUFFERSIZE      50000


#define mg_ArrowInterSpacing    16
#define mg_ArrowMarginTop       4
#define mg_ArrowClickStart      4       //from top of region
#define mgArrowClickSpacing     30      //base to base
#define mgArrowClickHeight      12      //height of button

// defines for which screen is currently active

// enumeration for all of the screens in the multiplayer front end.
/*
enum
{
    MGS_Connection_Method,
    MGS_Internet_Login,
    MGS_Password_Change,
    MGS_LAN_Login,
    MGS_Connecting,
    MGS_New_Account,
    MGS_Channel_Chat,
    MGS_Available_Channels,
    MGS_Player_Options,
    MGS_Create_Channel,
    MGS_Available_Games,
    MGS_Player_Wait,
    MGS_Captain_Wait,
    MGS_Basic_Options,
    MGS_Advanced_Options,
    MGS_Resource_Options,
    MGS_Basic_Options_Change,
    MGS_Advanced_Options_Change,
    MGS_Resource_Options_Change,
    MGS_Basic_Options_View,
    MGS_Advanced_Options_View,
    MGS_Resource_Options_View,
    MGS_Skirmish_Basic,
    MGS_Skirmish_Advanced,
    MGS_Skirmish_Resource,
    MGS_Game_Password,
    MGS_Quit_WON,
    MGS_Message_Box
};
*/

#define MG_JustOneDisappear     10


// This is the maximum number of items that can be transfered between threads
// using the statically allocated arrays.
#define MAX_NUM_ITEMS           40
#define MAX_NEW_CHAT_ITEMS      80



#define MG_WAIT_ACC 0.25f

// These defines are used for the chat command interface

real32 mgAccTime = 0.0f;

/*=============================================================================
    Data:
=============================================================================*/

sdword gameNum;

// multiplayer game base region
regionhandle    mgBaseRegion    = NULL;
fibfileheader  *mgScreensHandle = NULL;
fibfileheader  *mgScreensHandleServers = NULL;

// booleans indicating the status of the user in the screens.
sdword          ConnectionType  = -1;
// is person waiting for a game ?
int             WaitingForGame  = FALSE;
// did the person create the game
int             GameCreator     = FALSE;
// is the person logged into WON
bool            LoggedIn        = FALSE;

// is this a LAN game - shared stuff
int LanProtocalsValid = 0;      // 1 indicates IPX, 2 indicates TCP/IP, 3 indicates both
int LanProtocalButton = 0;      // 0 indicates IPX, 1 indicates TCP/IP (same as IPGame)

int            LANGame         = FALSE;     // use int not bool becuase CPP gets confused with our bool's.
int            IPGame          = 1;  // DO NOT USE TRUE, use 1 or CPP gets confused
                                     // if we are using IP protocal (LAN or Internet)

unsigned long firewallButton = FIREWALL_AUTODETECT;

//bool8 mgLeftArrowActive  = FALSE;
//bool8 mgRightArrowActive = FALSE;
bool8 mgHumanLeftArrowActive  = FALSE;
bool8 mgHumanRightArrowActive = FALSE;
bool8 mgCPULeftArrowActive  = FALSE;
bool8 mgCPURightArrowActive = FALSE;
//sdword mgArrowIndex = -1;
sdword mgHumanArrowIndex = -1;
sdword mgCPUArrowIndex = -1;

// This flag is returned from the refresh task, will be changed when the observation server goes into effect.
bool            donerefreshing  = TRUE;
bool            donepinging     = TRUE;
bool            mgCreatingNetworkGame = FALSE;

// structures and variables for creating a game.
extern CaptainGameInfo tpGameCreated;
// N    P   M   D   u   p      nc  sf  bs  sr   ii    ia     lt    la   ad  ab  p     f
//= {L"", L"", "", "", 0,  {0,0,0},  0,  1,  3,   1, 1440, 2000, 19200, 2000, 50, 50, 0, 22058};

CaptainGameInfo BackupGameCreated;
sdword          spCurrentSelectedBack;
/*char            scenPickBack[]*/
char           *scenPickBack;

// tweakable fonthandles.
fonthandle mgListOfGamesFont=0;
fonthandle mgChatWindowFont=0;
fonthandle mgUserNameFont=0;
fonthandle mgCurrentChannelFont=0;
fonthandle mgChannelListTitleFont=0;
fonthandle mgChannelListFont=0;
fonthandle mgGameListTitleFont=0;
fonthandle mgGameChatFont=0;
fonthandle mgGameUserNameFont=0;
fonthandle mgCurrentGameFont=0;
fonthandle mgOptionsFont=0;
fonthandle mgConnectingFont=0;
fonthandle mgMessageBoxFont=0;
char       mgListOfGamesFontName        [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgChatWindowFontName         [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgUserNameFontName           [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgCurrentChannelFontName     [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgChannelListTitleFontName   [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgChannelListFontName        [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgGameListTitleFontName      [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgGameChatFontName           [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgGameUserNameFontName       [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgCurrentGameFontName        [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgOptionsFontName            [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgConnectingFontName         [MG_FontNameLength] = "Small_Fonts_8.hff";
char       mgMessageBoxFontName         [MG_FontNameLength] = "Small_Fonts_8.hff";

// handles for the text entry boxes
textentryhandle mgNameEntryBox                      = NULL;
textentryhandle mgPasswordEntryBox                  = NULL;
textentryhandle mgOldPasswordChangeEntryBox         = NULL;
textentryhandle mgNewPasswordChangeEntryBox         = NULL;
textentryhandle mgConfirmPasswordChangeEntryBox     = NULL;
textentryhandle mgChatTextEntryBox                  = NULL;
textentryhandle mgChannelNameEntryBox               = NULL;
textentryhandle mgChannelDescriptionEntryBox        = NULL;
textentryhandle mgChannelPasswordEntryBox           = NULL;
textentryhandle mgChannelConfirmEntryBox            = NULL;
textentryhandle mgGameNameTextEntryBox              = NULL;
textentryhandle mgGameChatTextEntryBox              = NULL;
textentryhandle mgNewAccountPasswordEntryBox        = NULL;
textentryhandle mgNewAccountConfirmEntryBox         = NULL;
textentryhandle mgGamePasswordEntryBox              = NULL;
textentryhandle mgGamePasswordConfirmEntryBox       = NULL;
textentryhandle mgResourceInjectionIntervalEntryBox = NULL;
textentryhandle mgResourceInjectionAmountEntryBox   = NULL;
textentryhandle mgResourceLumpSumIntervalEntryBox   = NULL;
textentryhandle mgResourceLumpSumAmountEntryBox     = NULL;
textentryhandle mgGamePasswordEntryEntryBox         = NULL;

// handles for all of the listwindow's
listwindowhandle mgChatHistoryWindow            = NULL;
listwindowhandle mgUserNameWindowWindow         = NULL;
listwindowhandle mgListOfGamesWindow            = NULL;
listwindowhandle mgGameChatWindowWindow         = NULL;
listwindowhandle mgListOfChannelsWindow         = NULL;
listwindowhandle mgListOfServersWindow          = NULL;
listwindowhandle mgGameUserNameWindow           = NULL;
listwindowhandle mgConnectingStatusWindow       = NULL;

// colors for all of the tweakable colors in the screens.
color mgGameWhisperedColor      = colRGB(255,0,255);
color mgGamePrivateChatColor    = colRGB(20,255,50);
color mgGameNormalChatColor     = colRGB(180,180,180);
color mgGameMessageChatColor    = colRGB(255,255,0);
color mgWhisperedColor          = colRGB(255,0,255);
color mgPrivateChatColor        = colRGB(20,255,50);
color mgNormalChatColor         = colRGB(180,180,180);
color mgMessageChatColor        = colRGB(255,255,0);
color mgUserNameColor           = colRGB(180,180,180);
color mgCurrentChannelColor     = colRGB(20,200,20);
color mgChannelTitleColor       = colRGB(50,255,50);
color mgChannelListNormalColor  = colRGB(180,180,180);
color mgChannelListSelectedColor= colRGB(255,255,255);
color mgTitleFlashColor         = colRGB(255,255,50);
color mgGameListTitleColor      = colRGB(50,255,50);
color mgGameListNormalColor     = colRGB(180,180,180);
color mgGameListSelectedColor   = colRGB(255,255,255);
color mgGameListStartedColor    = colRGB(100,100,100);
color mgGameListDiffVersionColor= colRGB(140,140,140);
color mgCurrentGameColor        = colRGB(20,200,20);
color mgNumberOfHumansColor     = colRGB(20,200,20);
color mgNumberOfComputersColor  = colRGB(20,200,20);
color mgConnectingStatusColor   = colRGB(180,180,180);
color mgMessageBoxColor         = colRGB(180,180,180);

// variable for flashing the title when it is clicked on.
real32 mgTitleFlashTimer = 0.0f;

// linked lists with the local copy of info for channels and stuff.

// this linked list holds all of the current games in this channel
LinkedList  listofgames;
// this list has all of the available channels in it
LinkedList  listofchannels;
// this list has all of the servers in it (hwds)
LinkedList  listofservers;
// this list has all of the Room chat history
LinkedList  listofchatinfo;
// this list has all of the users in the Current Room in it
LinkedList  listofusersinfo;
// this is the list of games to ping
LinkedList  listtoping;
// this is the list of topics in the status window listbox
LinkedList  statusitemlist;
// this is the list of players in the current game
LinkedList  listofplayers;
// this is a temporary storage list for the players in a game to compare
// and check if someone has joined or left.
LinkedList  listofplayersold;
// this holds all of the history for the game chat window
LinkedList  listofgamechatinfo;

// These lists are used to hold the list of people ignored or banned that are online
LinkedList  listofIgnoreUsers;
LinkedList  listofBannedUsers;

typedef struct mgUserNameList
{
    Node    link;
    char    name[MAX_PERSONAL_NAME_LEN];
}
mgUserNameList;

void mgProcessIgnore(char *name, char *retmessage);
void mgProcessKick(char *name, char *retmessage);
void mgProcessBan(char *name, char *retmessage);
void mgProcessLimit(char *name, char *retmessage);

typedef void (*mgCommandFunc)(char *name, char *retmessage);

typedef struct mgCommandInfo
{
    char           *Command;
    mgCommandFunc   callback;
    bool            hasnamearg;
    bool            captainonly;
    bool            roomchat;
    bool            pregamechat;
}
mgCommandInfo;

mgCommandInfo  *mgCommandInfoPtr;

mgCommandInfo mgCommandEnglish[] =
{   //                              nam cap roo pre
    {"Ignore",  mgProcessIgnore,    1,  0,  1,  0   },
    {"Kick",    mgProcessKick,      1,  1,  0,  1   },
    {"Ban",     mgProcessBan,       1,  1,  0,  1   },
    {"Limit",   mgProcessLimit,     0,  1,  0,  1   },
    {"",        NULL,               0,  1,  0,  1   }
};

mgCommandInfo mgCommandFrench[] =
{
    {"Ignorer", mgProcessIgnore,    1,  0,  1,  0   },
    {"Exclure", mgProcessKick,      1,  1,  0,  1   },
    {"Bannir",  mgProcessBan,       1,  1,  0,  1   },
    {"Limiter", mgProcessLimit,     0,  1,  0,  1   },
    {"",        NULL,               0,  1,  0,  1   }
};

mgCommandInfo mgCommandGerman[] =
{
    {"Ignorieren",  mgProcessIgnore,    1,  0,  1,  0   },
    {"Entfernen",   mgProcessKick,      1,  1,  0,  1   },
    {"Verbannen",   mgProcessBan,       1,  1,  0,  1   },
    {"Begrenzen",   mgProcessLimit,     0,  1,  0,  1   },
    {"",            NULL,               0,  1,  0,  1   }
};

mgCommandInfo mgCommandSpanish[] =
{
    {"Ignorar",     mgProcessIgnore,    1,  0,  1,  0   },
    {"Acabar",      mgProcessKick,      1,  1,  0,  1   },
    {"Prohibición", mgProcessBan,       1,  1,  0,  1   },
    {"Límite",      mgProcessLimit,     0,  1,  0,  1   },
    {"",            NULL,               0,  1,  0,  1   }
};

mgCommandInfo mgCommandItalian[] =
{
    {"Ignora",  mgProcessIgnore,    1,  0,  1,  0   },
    {"Elimina", mgProcessKick,      1,  1,  0,  1   },
    {"Bando",   mgProcessBan,       1,  1,  0,  1   },
    {"Limite",  mgProcessLimit,     0,  1,  0,  1   },
    {"",        NULL,               0,  1,  0,  1   }
};


bool        gamestarted     = FALSE;
bool        newscreen       = FALSE;

// Queue for transfering packets between threads.
Queue       mgThreadTransfer;

// mutually exclusive handles for locking data between threads.
void       *changescreenmutex;
void       *gamestartedmutex;

// info for spacing in the list of games screen
sdword  mgGameNameWidth     = 150;
sdword  mgPingTimeWidth     = 50;
sdword  mgNumPlayerWidth    = 50;
sdword  mgMapNameWidth      = 110;
sdword  mgGameNotesWidth    = 100;

// info for spacing in the list of channels screen
sdword  mgChannelNameWidth  = 185;
sdword  mgChannelDescriptionWidth = 185;
sdword  mgChannelNumberWidth = 25;
sdword  mgChannelNotesWidth = 100;

// info for spacing in the list of servers screen
sdword  mgServerNameWidth  = 185;
sdword  mgServerDescriptionWidth = 185;
sdword  mgServerPingWidth = 50;
sdword  mgServerLoadWidth = 50;
sdword  mgServerReliabilityWidth = 50;

// for pinging the servers list
BabyCallBack    *pingserversbaby=NULL;
udword pingservernum = 0;

// for refreshing the game list
BabyCallBack    *refreshbaby=NULL;

// for pinging all of the games
BabyCallBack    *pinggamesbaby=NULL;

// is the multiplayer game screens running.
bool mgRunning=FALSE;

// currently active screen
sdword currentScreen=-1;
// last active screen
sdword lastScreen=-1;

// used for starting a screen in a separate thread.
sdword screenaskedfor=-1;
//sdword mgScreensAdded=0;
bool   hideallscreens=TRUE;

// type of query asked for from titan.
sdword mgQueryType=-1;

// handle for the task that transfers the informatoin from titan to the main game thread.
taskhandle ProccessCallback=0;

// pointer to the game that we want to join
tpscenario  *joingame=NULL;

// room we want to join
wchar_t joinchannelname[MAX_CHANNEL_NAME_LEN] = L"";
wchar_t joinchanneldescription[MAX_CHANNEL_DESCRIPTION_LEN] = L"";

// string that is displayed in the message box
char messageBoxString[256];
char messageBoxString2[256];

// userid of this player
udword mgMyChatUserID=0;

// variable that indicates which screen is displayed after a user hits cancel on the conneting screen
sdword mgConnectingScreenGoto=-1;

// pre set games
GameTypes *preSetGames = NULL;

// variables for the hokey color and race picker screen
udword BaseColorSave;
udword StripeColorSave;
uword  RaceSave;

bool refreshChannelsScreenRequest = FALSE;
bool refreshServersScreenRequest = FALSE;

extern color PATCHBARCOLOR;
extern color PATCHBAROUTLINECOLOR;

//bool tempstartship;  //need to update titannet.h for captaingameinfo
// this is the width in pixels of the chat windows
sdword chatwindowwidth = 384-20;

// patch variables
int patchLenSoFar = 0;
int patchTotalLen = 0;
int patchErrorMsg = 0;
int patchAbortRequest = 0;
int patchComplete = 0;

#if PUBLIC_BETA
// public beta variables
int betaVerifying = 0;
int betaVerified = 0;
int betaDoneVerifying = 0;
#endif

// temporary variable for storing name, in case of cancel
char tempname[64];

CaptainGameInfo joinBackuptpGameCreated;
bool joinBackuptpGameCreatedValid = FALSE;

udword mgPlayerLimit = 0;

/*=============================================================================
    Function Callback Prototypes:
=============================================================================*/

//  Callbacks for the connection type screen.
void mgConnectionBack               (char *name, featom *atom);
void mgSkirmish                     (char *name, featom *atom);
void mgLANIPX                       (char *name, featom *atom);
void mgInternetWON                  (char *name, featom *atom);

// Callbacks for the Login screen to Internet WON
void mgPasswordEntry                (char *name, featom *atom);
void mgChangePassword               (char *name, featom *atom);
void mgNewAccount                   (char *name, featom *atom);
void mgLaunchWON                    (char *name, featom *atom);
void mgFirewallButton               (char *name, featom *atom);

// Shared between the internet screen and the lan screen
void mgNameEntry                    (char *name, featom *atom);
void mgBackToConnection             (char *name, featom *atom);

// Callbacks for the Password Changer Screen.
void mgOldPasswordChangeEntry       (char *name, featom *atom);
void mgNewPasswordChangeEntry       (char *name, featom *atom);
void mgConfirmPasswordChangeEntry   (char *name, featom *atom);
void mgChangePasswordBack           (char *name, featom *atom);
void mgChangePasswordNow            (char *name, featom *atom);

// Callbacks from the new account Screen.
void mgNewAccountPassword           (char *name, featom *atom);
void mgNewAccountConfirm            (char *name, featom *atom);
void mgNewAccountBack               (char *name, featom *atom);
void mgCreateNewAccountNow          (char *name, featom *atom);

// Callbacks for the channel chatting screen.
void mgChatTextEntry                (char *name, featom *atom);
void mgViewChannels                 (char *name, featom *atom);
void mgViewGames                    (char *name, featom *atom);
void mgCreateGame                   (char *name, featom *atom);
void mgBackToLogin                  (char *name, featom *atom);
void mgChangeColors                 (char *name, featom *atom);
void mgChatWindowInit               (char *name, featom *atom);
void mgUserNameWindowInit           (char *name, featom *atom);
void mgCurrentChannelDraw           (featom *atom, regionhandle region);

// Callbacks for the Channel Picker screen.
void mgJoinChannel                  (char *name, featom *atom);
void mgCreateChannel                (char *name, featom *atom);
void mgBackToChannelChat            (char *name, featom *atom);
void mgListOfChannelsInit           (char *name, featom *atom);

// Callbacks for the Channel Creation Screen.
void mgChannelNameEntry             (char *name, featom *atom);
void mgChannelDescriptionEntry      (char *name, featom *atom);
void mgChannelProtected             (char *name, featom *atom);
void mgChannelPasswordEntry         (char *name, featom *atom);
void mgChannelConfirmEntry          (char *name, featom *atom);
void mgCreateChannelNow             (char *name, featom *atom);
void mgBackToAvailableChannels      (char *name, featom *atom);

// Callbacks for the Available Games screen.
void mgPingAllGames                 (char *name, featom *atom);
void mgSeeDetails                   (char *name, featom *atom);
void mgJoinGame                     (char *name, featom *atom);
void mgListOfGamesInit              (char *name, featom *atom);
void mgListOfGamesBack              (char *name, featom *atom);
bool Refresh                        (udword num, void *data, struct BabyCallBack *baby);
bool PingAllGames                   (udword num, void *data, struct BabyCallBack *baby);
bool PingAllServers                 (udword num, void *data, struct BabyCallBack *baby);

// Callbacks for the waiting for game screens.
void mgGameChatWindowInit           (char *name, featom *atom);
void mgGameUserNameWindowInit       (char *name, featom *atom);
void mgViewDetails                  (char *name, featom *atom);
void mgLeaveGame                    (char *name, featom *atom);
void mgSetupGame                    (char *name, featom *atom);
void mgStartGame                    (char *name, featom *atom);
void mgGameChatTextEntry            (char *name, featom *atom);
void mgCurrentGameDraw              (featom *atom, regionhandle region);

// Callbacks for the Basic Game Options game screen.
void mgCreateGameNow                (char *name, featom *atom);
void mgGameNameTextEntry            (char *name, featom *atom);
void mgChooseScenario               (char *name, featom *atom);
//void mgAddOpponent                  (char *name, featom *atom);
//void mgRemoveOpponent               (char *name, featom *atom);
void mgAddCPUOpponent               (featom *atom, regionhandle region);
void mgRemoveCPUOpponent            (featom *atom, regionhandle region);
void mgPickGameType                 (char *name, featom *atom);
void mgStartingFleet                (char *name, featom *atom);
void mgStartShip                    (char *name, featom *atom);
void mgGameType                     (char *name, featom *atom);
void mgNumberOfComputers            (featom *atom, regionhandle region);
//void mgLeftArrowDraw(featom *atom, regionhandle region);
//void mgRightArrowDraw(featom *atom, regionhandle region);
//void mgLeftArrowDraw(featom *atom, regionhandle region);
//void mgRightArrowDraw(featom *atom, regionhandle region);

// Callbacks for the Advanced Options game Screen.
void mgHyperspace                   (char *name, featom *atom);
void mgAlliedVictory                (char *name, featom *atom);
void mgLastMotherShip               (char *name, featom *atom);
void mgCaptureCapitalShip           (char *name, featom *atom);
void mgResearchEnabled              (char *name, featom *atom);
//void mgBountiesEnabled              (char *name, featom *atom);
void mgBountySize                   (char *name, featom *atom);
void mgPasswordProtected            (char *name, featom *atom);
void mgGamePassword                 (char *name, featom *atom);
void mgGamePasswordConfirm          (char *name, featom *atom);
void mgUnitCapsEnabled              (char *name, featom *atom);
void mgFuelBurnEnabled              (char *name, featom *atom);
void mgCratesEnabled                (char *name, featom *atom);

// Callbacks for the Resourceing Optinos game Screen.

void mgHarvestingEnabled            (char *name, featom *atom);
void mgStartingResources            (char *name, featom *atom);
void mgResourceInjections           (char *name, featom *atom);
void mgResourceInjectionInterval    (char *name, featom *atom);
void mgResourceInjectionAmount      (char *name, featom *atom);
void mgResourceLumpSum              (char *name, featom *atom);
void mgResourceLumpSumInterval      (char *name, featom *atom);
void mgResourceLumpSumAmount        (char *name, featom *atom);

// Callbacks shared by all of the Game Options screens
void mgBackFromOptions              (char *name, featom *atom);
void mgBasicOptions                 (char *name, featom *atom);
void mgAdvancedOptions              (char *name, featom *atom);
void mgResourceOptions              (char *name, featom *atom);

// Callbacks for the Player options screen.
void mgSetColorsNow                 (char *name, featom *atom);
void mgBackFromPlayerOptions        (char *name, featom *atom);

// Callbacks for the Connecting Status screen.
void mgConnectingStatusInit         (char *name, featom *atom);
void mgConnectingCancel             (char *name, featom *atom);

// Callbacks for the game entry screen.
void mgGamePasswordEntry            (char *name, featom *atom);
void mgBackFromPassword             (char *name, featom *atom);
void mgGoPassword                   (char *name, featom *atom);

void mgBackFromRoomPassword         (char *name, featom *atom);
void mgGoRoomPassword               (char *name, featom *atom);

// Callbacks for the quit WON question screen.
void mgYesQuitWON                   (char *name, featom *atom);
void mgDontQuitWON                  (char *name, featom *atom);

// Callbacks for downloading patch
void mgYesDownloadPatch             (char *name, featom *atom);
void mgNoDownloadPatch              (char *name, featom *atom);
void mgDrawPatchProgress            (featom *atom, regionhandle region);
void mgAbortDownloadPatch           (char *name, featom *atom);

// Callbacks from the message box screen.
void mgMessageOk                    (char *name, featom *atom);
void mgDrawMessageBox               (featom *atom, regionhandle region);

// Callbacks for choose servers

void mgGoChooseServer               (char *name, featom *atom);
void mgCancelChooseServer           (char *name, featom *atom);
void mgListOfServersInit            (char *name, featom *atom);

/*=============================================================================
    Function Prototypes for the draw callbacks:
=============================================================================*/


/*=============================================================================
    List of Callbacks for adding to the region processor:
=============================================================================*/

fecallback      mgCallBack[]=
{
//  Callbacks for the connection type screen.
    {mgConnectionBack               ,   "MG_ConnectionBack"             },
    {mgSkirmish                     ,   "MG_Skirmish"                   },
    {mgLANIPX                       ,   "MG_LANIPX"                     },
    {mgInternetWON                  ,   "MG_InternetWON"                },

// Callbacks for the Login screen to Internet WON
    {mgPasswordEntry                ,   "MG_PasswordEntry"              },
    {mgChangePassword               ,   "MG_ChangePassword"             },
    {mgNewAccount                   ,   "MG_NewAccount"                 },
    {mgLaunchWON                    ,   "MG_LaunchWON"                  },
    {mgFirewallButton               ,   "MG_FirewallButton"             },

// Shared between the internet screen and the lan screen
    {mgNameEntry                    ,   "MG_NameEntry"                  },
    {mgBackToConnection             ,   "MG_BackToConnection"           },

// Callbacks for the Password Changer Screen.
    {mgOldPasswordChangeEntry       ,   "MG_OldPasswordChangeEntry"     },
    {mgNewPasswordChangeEntry       ,   "MG_NewPasswordChangeEntry"     },
    {mgConfirmPasswordChangeEntry   ,   "MG_ConfirmPasswordChangeEntry" },
    {mgChangePasswordBack           ,   "MG_ChangePasswordBack"         },
    {mgChangePasswordNow            ,   "MG_ChangePasswordNow"          },

// Callbacks from the new account Screen.
    {mgNewAccountPassword           ,   "MG_NewAccountPassword"         },
    {mgNewAccountConfirm            ,   "MG_NewAccountConfirm"          },
    {mgNewAccountBack               ,   "MG_NewAccountBack"             },
    {mgCreateNewAccountNow          ,   "MG_CreateNewAccountNow"        },

// Callbacks for the channel chatting screen.
    {mgChatTextEntry                ,   "MG_ChatTextEntry"              },
    {mgViewChannels                 ,   "MG_ViewChannels"               },
    {mgViewGames                    ,   "MG_ViewGames"                  },
    {mgCreateGame                   ,   "MG_CreateGame"                 },
    {mgBackToLogin                  ,   "MG_BackToLogin"                },
    {mgChangeColors                 ,   "MG_ChangeColors"               },
    {mgChatWindowInit               ,   "MG_ChatHistoryWindow"          },
    {mgUserNameWindowInit           ,   "MG_UserNameWindow"             },

// Callbacks for the Channel Picker screen.
    {mgJoinChannel                  ,   "MG_JoinChannel"                },
    {mgCreateChannel                ,   "MG_CreateChannel"              },
    {mgBackToChannelChat            ,   "MG_BackToChannelChat"          },
    {mgListOfChannelsInit           ,   "MG_ListOfChannels"             },

// Callbacks for the Channel Creation Screen.
    {mgChannelNameEntry             ,   "MG_ChannelNameEntry"           },
    {mgChannelDescriptionEntry      ,   "MG_ChannelDescriptionEntry"    },
    {mgChannelProtected             ,   "MG_ChannelProtected"           },
    {mgChannelPasswordEntry         ,   "MG_ChannelPasswordEntry"       },
    {mgChannelConfirmEntry          ,   "MG_ChannelConfirmEntry"        },
    {mgCreateChannelNow             ,   "MG_CreateChannelNow"           },
    {mgBackToAvailableChannels      ,   "MG_BackToAvailableChannels"    },

// Callbacks for the Available Games screen.
    {mgPingAllGames                 ,   "MG_PingAllGames"               },
    {mgSeeDetails                   ,   "MG_SeeDetails"                 },
    {mgJoinGame                     ,   "MG_JoinGame"                   },
    {mgListOfGamesInit              ,   "MG_ListOfGames"                },
    {mgListOfGamesBack              ,   "MG_ListOfGamesBack"            },

// Callbacks for the waiting for game screens.
    {mgGameChatWindowInit           ,   "MG_GameChatWindow"             },
    {mgGameUserNameWindowInit       ,   "MG_GameUserName"               },
    {mgViewDetails                  ,   "MG_ViewDetails"                },
    {mgLeaveGame                    ,   "MG_LeaveGame"                  },
    {mgSetupGame                    ,   "MG_SetupGame"                  },
    {mgStartGame                    ,   "MG_StartGame"                  },
    {mgGameChatTextEntry            ,   "MG_GameChatTextEntry"          },

// Callbacks for the Basic Game Options screen.
    {mgCreateGameNow                ,   "MG_CreateGameNow"              },
    {mgGameNameTextEntry            ,   "MG_GameNameTextEntry"          },
    {mgChooseScenario               ,   "MG_ChooseScenario"             },
    {mgPickGameType                 ,   "MG_PickGameType"               },
    {mgStartingFleet                ,   "MG_StartingFleet"              },
    {mgStartShip                    ,   "MG_StartShip"                   },
    {mgGameType                     ,   "MG_GameType"                   },




// Callbacks for the Advanced Options game Screen.
    {mgHyperspace                   ,   "MG_Hyperspace"                 },
    {mgAlliedVictory                ,   "MG_AlliedVictory"              },
    {mgLastMotherShip               ,   "MG_LastMotherShip"             },
    {mgCaptureCapitalShip           ,   "MG_CaptureCapitalShip"         },
    {mgResearchEnabled              ,   "MG_ResearchEnabled"            },
    //{mgBountiesEnabled              ,   "MG_BountiesEnabled"            },
    {mgBountySize                   ,   "MG_BountySize"                 },
    {mgPasswordProtected            ,   "MG_PasswordProtected"          },
    {mgGamePassword                 ,   "MG_GamePassword"               },
    {mgGamePasswordConfirm          ,   "MG_GamePasswordConfirm"        },
    {mgUnitCapsEnabled              ,   "MG_UnitCapsEnabled"            },
    {mgFuelBurnEnabled              ,   "MG_NoFuelBurn"                 },
    {mgCratesEnabled                ,   "MG_CratesEnabled"              },

// Callbacks for the Resourceing Optinos game Screen.
    {mgHarvestingEnabled            ,   "MG_HarvestingEnabled"          },
    {mgStartingResources            ,   "MG_StartingResources"          },
    {mgResourceInjections           ,   "MG_ResourceInjections"         },
    {mgResourceInjectionInterval    ,   "MG_ResourceInjectionInterval"  },
    {mgResourceInjectionAmount      ,   "MG_ResourceInjectionAmount"    },
    {mgResourceLumpSum              ,   "MG_ResourceLumpSum"            },
    {mgResourceLumpSumInterval      ,   "MG_ResourceLumpSumInterval"    },
    {mgResourceLumpSumAmount        ,   "MG_ResourceLumpSumAmount"      },

// Callbacks shared by all of the Game Options screens
    {mgBackFromOptions              ,   "MG_BackFromOptions"            },
    {mgBasicOptions                 ,   "MG_BasicOptions"               },
    {mgAdvancedOptions              ,   "MG_AdvancedOptions"            },
    {mgResourceOptions              ,   "MG_ResourceOptions"            },

// Callbacks for the Player options screen.
    {mgSetColorsNow                 ,   "MG_SetColorsNow"               },
    {mgBackFromPlayerOptions        ,   "MG_BackFromPlayerOptions"      },

// Callbacks for the scenario picker screen.  In scenpick.h
    {spDonePicking                  ,   "CS_Done"                       },
    {spBackPicking                  ,   "CS_Back"                       },
//    {spScroller                     ,   "CS_Scroller"                   },

// Callbacks for the Connecting Status screen.
    {mgConnectingStatusInit         ,   "MG_ConnectingStatus"           },
    {mgConnectingCancel             ,   "MG_ConnectingCancel"           },

// Callbacks for the password entry screen.
    {mgGamePasswordEntry            ,   "MG_GamePasswordEntry"          },
    {mgBackFromPassword             ,   "MG_BackFromPassword"           },
    {mgGoPassword                   ,   "MG_GoPassword"                 },

// Callbacks for the password entry screen.
    {mgGamePasswordEntry            ,   "MG_RoomPasswordEntry"          },
    {mgBackFromRoomPassword         ,   "MG_BackFromRoomPassword"       },
    {mgGoRoomPassword               ,   "MG_GoRoomPassword"             },

// Callbacks from the are you sure, WON quit screen
    {mgYesQuitWON                   ,   "MG_YesQuitWON"                 },
    {mgDontQuitWON                  ,   "MG_DontQuitWON"                },

// Callbacks from the message box screen.
    {mgMessageOk                    ,   "MG_MessageOk"                  },

    {mgYesDownloadPatch             ,   "MG_YesDownloadPatch"           },
    {mgNoDownloadPatch              ,   "MG_NoDownloadPatch"            },

    {mgAbortDownloadPatch           ,   "MG_AbortDownloadPatch"         },

    {NULL                           ,   NULL                            }
};

fecallback      mgCallBackServers[] =
{
    { mgGoChooseServer              ,   "MG_GoChooseServer"             },
    { mgCancelChooseServer          ,   "MG_CancelChooseServer"         },
    { mgListOfServersInit           ,   "MG_ListOfServers"              },
    {NULL                           ,   NULL                            }
};

fedrawcallback mgDrawCallback[] =
{
    {mgCurrentChannelDraw           ,   "MG_CurrentChannel"             },
    {mgCurrentGameDraw              ,   "MG_CurrentGame"                },
    {mgNumberOfComputers            ,   "MG_NumberOfComputers"          },
    {mgDrawMessageBox               ,   "MG_DrawMessageBox"             },
    {mgAddCPUOpponent               ,   "MG_AddCPUOpponent"           },
    {mgRemoveCPUOpponent            ,   "MG_RemoveCPUOpponent"           },
    {mgDrawPatchProgress            ,   "MG_DrawPatchProgress"          },
    {NULL                           ,   NULL                            }
};

#if 0
fedrawcallback mgDrawCallbackServers[] =
{
    {NULL                           ,   NULL                            }
};
#endif

void mgProcessCallBacksTask(void);

void mgGameTypeScriptInit();
/*=============================================================================
    Functions:
=============================================================================*/



/*-----------------------------------------------------------------------------
    Name        : mgBackuptpGameCreated
    Description : called whenever tpGameCreated will be overwritten due to joining game, etc.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mgBackuptpGameCreated(void)
{
    if (joinBackuptpGameCreatedValid)
    {
        return;     // already backed up
    }
    else if (!GameCreator)
    {
        joinBackuptpGameCreated = tpGameCreated;
        joinBackuptpGameCreatedValid = TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mgRestoretpGameCreated
    Description : will restore tpGameCreated if it was overwritten due to joining game, etc.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mgRestoretpGameCreated(void)
{
    if (joinBackuptpGameCreatedValid)
    {
        tpGameCreated = joinBackuptpGameCreated;
        joinBackuptpGameCreatedValid = FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mgResetNamePassword
    Description : resets the name and password, other attributes which you think should be reset everytime person
                  creates a new game.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mgResetNamePassword(void)
{
    tpGameCreated.Name[0]           = 0;
    tpGameCreated.Password[0]       = 0;
    bitClear(tpGameCreated.flag,MG_PasswordProtected);
}

void mgGameInterestedIn(wchar_t *interested)
{
#ifndef _MACOSX_FIX_ME
    LockMutex(GameWereInterestedInMutex);
    wcscpy(GameWereInterestedIn,interested);
    UnLockMutex(GameWereInterestedInMutex);
#endif
}

void mgGameInterestedOff()
{
    LockMutex(GameWereInterestedInMutex);
    GameWereInterestedIn[0] = 0;
    UnLockMutex(GameWereInterestedInMutex);
}

// this function parses the chat message entered and checks to see if they wanted to send
// a private message.  It has to search through two different lists of users, depending
// on whether it is in game chat or the room chat that they are typing in.

void *mgParseChatEntry(char *messageorig, bool preGameChat)
{
    udword      i,nummatches;
    char        toname[64], ampremoved[64], messagetmp[280], *message;
    bool        done=FALSE;
    Node       *walk=NULL;
    userlist   *user, *matched=NULL;
    gameplayerinfo *guser, *gmatched=NULL;

    message = (char *)&messagetmp[0];

    memset(messagetmp, 0, 280);

    gcRemoveAmpersands(message, messageorig);

    // if not private  at all then return NULL
    if (message[0]!='/') return(NULL);

    message++;

    i=0;

    if (message[0] == 0) done = TRUE;

    while (!done)
    {
        // copy the name over character by character
        toname[i] = message[0];
        i++;
        toname[i]=0;
        message++;

        // do we already know that it is a match?
        if (matched==NULL)
        {
            // no then see if we can exclude all but one name
            nummatches = 0;
            if (!preGameChat)
            {
                walk = listofusersinfo.head;

                while (walk != NULL)
                {
                    user = listGetStructOfNode(walk);
                    gcRemoveAmpersands(ampremoved, user->userName);
                    if (strncasecmp(ampremoved,toname,i)==0)
                    {
                        // found a match so far
                        matched = user;

                        nummatches++;
                    }
                    walk = walk->next;
                }
                if (nummatches!=1)
                {
                    // more than one possible match, then we must search again
                    matched=NULL;
                }
            }
            else
            {
                walk = listofplayers.head;

                while (walk != NULL)
                {
                    guser = listGetStructOfNode(walk);
                    gcRemoveAmpersands(ampremoved, guser->name);
                    if (strncasecmp(ampremoved,toname,i)==0)
                    {
                        // found a match so far
                        gmatched = guser;

                        nummatches++;
                    }
                    walk = walk->next;
                }
                if (nummatches!=1)
                {
                    // more than one possible match, then we must search again
                    gmatched=NULL;
                }
            }
        }

        if (!preGameChat)
        {
            if (matched != NULL)
            {
                return (matched);
            }
        }
        else
        {
            if (gmatched != NULL)
            {
                return (gmatched);
            }
        }
        if (message[0] == 0)
        {
            done = TRUE;
        }
    }

    return NULL;
}

void *mgCompleteRoomPlayerName(char *message, char *compname, bool preGameChat)
{
    udword      i,nummatches;
    char        toname[64], ampremoved[64];
    bool        done=FALSE;
    Node       *walk=NULL;
    userlist   *user, *matched=NULL;
    gameplayerinfo *guser, *gmatched=NULL;

    while (message[0] == ' ') message++;

    i=0;

    if (message[0] == 0) done = TRUE;

    while (!done)
    {
        // copy the name over character by character
        toname[i] = message[0];
        i++;
        toname[i]=0;
        message++;

        // do we already know that it is a match?
        if (matched==NULL)
        {
            // no then see if we can exclude all but one name
            nummatches = 0;
            if (!preGameChat)
            {
                walk = listofusersinfo.head;

                while (walk != NULL)
                {
                    user = listGetStructOfNode(walk);
                    gcRemoveAmpersands(ampremoved, user->userName);
                    if (strncasecmp(ampremoved,toname,i)==0)
                    {
                        // found a match so far
                        matched = user;

                        nummatches++;
                    }
                    walk = walk->next;
                }
                if (nummatches!=1)
                {
                    // more than one possible match, then we must search again
                    matched=NULL;
                }
            }
            else
            {
                walk = listofplayers.head;

                while (walk != NULL)
                {
                    guser = listGetStructOfNode(walk);
                    gcRemoveAmpersands(ampremoved, guser->name);
                    if (strncasecmp(ampremoved,toname,i)==0)
                    {
                        // found a match so far
                        gmatched = guser;

                        nummatches++;
                    }
                    walk = walk->next;
                }
                if (nummatches!=1)
                {
                    // more than one possible match, then we must search again
                    gmatched=NULL;
                }
            }
        }

        if (!preGameChat)
        {
            if (matched != NULL)
            {
                strcpy(compname, ((userlist *)matched)->userName);
                return (matched);
            }
        }
        else
        {
            if (gmatched != NULL)
            {
                strcpy(compname, ((gameplayerinfo *)gmatched)->name);
                return (gmatched);
            }
        }
        if (message[0] == 0)
        {
            done = TRUE;
        }
    }

    return NULL;
}

Node *mgIsUserInList(LinkedList *list, char *name)
{
    Node *search = list->head;
    mgUserNameList *namestruct;

    while (search != NULL)
    {
        namestruct = (mgUserNameList *)listGetStructOfNode(search);

        if (strncasecmp(name, namestruct->name, MAX_PERSONAL_NAME_LEN)==0)
        {
            return(search);
        }

        search = search->next;
    }

    return(NULL);
}

void mgProcessIgnore(char *name, char *retmessage)
{
    mgUserNameList *newname;
    Node           *search;

    if ((search=mgIsUserInList(&listofIgnoreUsers,name))==NULL)
    {
        newname = (mgUserNameList *)memAlloc(sizeof(mgUserNameList),"IgnoreUserList",NonVolatile);

        strcpy(newname->name,name);

        listAddNode(&listofIgnoreUsers, &newname->link, (void *)newname);

        sprintf(retmessage, strGetString(strCommandIgnoreOn), name);
    }
    else
    {
        listDeleteNode(search);

        sprintf(retmessage, strGetString(strCommandIgnoreOff), name);
    }
}

void mgProcessKick(char *name, char *retmessage)
{
    Node *node;
    gameplayerinfo *pinfo;

    retmessage[0] = 0;

    if (!GameCreator)
    {
        return;
    }

    // find player in list listofplayers
    for (node = listofplayers.head; node != NULL; node = node->next)
    {
        pinfo = (gameplayerinfo *)listGetStructOfNode(node);

        if (strcmp(pinfo->name,name) == 0)
        {
            if (titanKickPlayer(pinfo->index))
            {
                sprintf(retmessage, strGetString(strCommandKickCaptain), name);
            }
        }
    }
}

void mgProcessBan(char *name, char *retmessage)
{
    mgUserNameList *newname;
    Node           *search;

    if ((search=mgIsUserInList(&listofBannedUsers,name))==NULL)
    {
        newname = (mgUserNameList *)memAlloc(sizeof(mgUserNameList),"BanUserList",NonVolatile);

        strcpy(newname->name,name);

        listAddNode(&listofBannedUsers, &newname->link, (void *)newname);

        sprintf(retmessage, strGetString(strCommandBanOn), name);
    }
    else
    {
        listDeleteNode(search);

        sprintf(retmessage, strGetString(strCommandBanOff), name);
    }
}

bool mgIsUserBanned(char *name)
{
    if (mgIsUserInList(&listofBannedUsers,name) != NULL)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

void mgProcessLimit(char *name, char *retmessage)
{
    udword limit = 0;

    limit = atoi(name);

    if ((limit > 1) && (limit < MAX_MULTIPLAYER_PLAYERS))
    {
        mgPlayerLimit = limit;
        sprintf(retmessage, strGetString(strCommandLimit), limit);
    }
    else
    {
        mgPlayerLimit = 0;
        sprintf(retmessage, strGetString(strCommandLimitOff));
    }

    return;
}

sdword mgCommandEntered(char *text, bool preGameChat)
{
    char *parse=text, *space=text;
    sdword i=0, command=-1;

    if (parse[0] == '!')
    {
        parse++;

        while ((space[0] != ' ') && (space[0] != 0)) space++;

        while (mgCommandInfoPtr[i].callback != NULL)
        {
            if ((strncasecmp(parse, mgCommandInfoPtr[i].Command, max(strlen(mgCommandInfoPtr[i].Command),space-text-1))==0) &&
                (space[0] == ' ') &&
                (  ((GameCreator) && (mgCommandInfoPtr[i].captainonly)) ||
                   ((preGameChat)&&(mgCommandInfoPtr[i].pregamechat)) ||
                   ((!preGameChat)&&(mgCommandInfoPtr[i].roomchat))) )
            {
                command = i;
                break;
            }

            i++;
        }

        return (command);
    }
    else
    {
        return(-1);
    }
}

sdword mgCommandExecute(char *text, char *retmessage, bool preGameChat)
{
    sdword  command;
    char   *name = text;

    command = mgCommandEntered(text, preGameChat);

    if (command != -1)
    {
        name += 1 + strlen(mgCommandInfoPtr[command].Command);

        while (name[0] == ' ') name++;

        mgCommandInfoPtr[command].callback(name, retmessage);
    }

    return(command);
}

sdword mgCommandComplete(char *text, char *commandstr, bool preGameChat)
{
    sdword      j,i,nummatches, command=-1;
    char        toname[64], compname[64];
    bool        done=FALSE;

    // if not command at all then return NULL
    if (text[0]!='!') return(-1);

    if ((command = mgCommandEntered(text,preGameChat)) == -1)
    {
        text++;

        i=0;

        if (text[0] == 0) done = TRUE;

        while (!done)
        {
            // copy the name over character by character
            toname[i] = text[0];
            i++;
            toname[i]=0;
            text++;

            // do we already know that it is a match?
            if (command==-1)
            {
                // no then see if we can exclude all but one name
                nummatches = 0;

                for (j=0;mgCommandInfoPtr[j].callback != NULL;j++)
                {
                    if ((strncasecmp(toname, mgCommandInfoPtr[j].Command, strlen(toname))==0) &&
                        (  ((GameCreator) && (mgCommandInfoPtr[j].captainonly)) ||
                           ((preGameChat)&&(mgCommandInfoPtr[j].pregamechat)) ||
                           ((!preGameChat)&&(mgCommandInfoPtr[j].roomchat))) )
                    {
                        command = j;

                        nummatches++;
                    }
                }
                if (nummatches != 1)
                {
                    command = -1;
                }
            }

            if (command != -1)
            {
                sprintf(commandstr, "!%s ", mgCommandInfoPtr[command].Command);
                return (command);
            }

            if (text[0] == 0)
            {
                done = TRUE;
            }
        }
    }
    else if (mgCommandInfoPtr[command].hasnamearg)
    {
        text += 1 + strlen(mgCommandInfoPtr[command].Command);

        if (mgCompleteRoomPlayerName(text, compname, FALSE)!=NULL)
        {
            sprintf(commandstr, "!%s %s", mgCommandInfoPtr[command].Command, compname);

            return (command);
        }
    }

    return (-1);
}


/*-----------------------------------------------------------------------------
    Name        : mgScreensDisappear
    Description : a wrapper for front end screens. It makes all screens added dissapear;
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mgScreensDisappear(void)
{
    while (feStackIndex > 0)
    {
        feScreenDisappear(NULL,NULL);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mgDisplayScreen
    Description : a wrapper for front end screens, cleans up the current screen
                  and displays the screen asked for.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mgShowScreen(sdword screennum, bool disappear)
{
    char screenname[64];

    sdword curscreen=currentScreen;

    if (disappear != FALSE)
    {
        if (disappear == MG_JustOneDisappear)
        {
            // just hide one screen
            feScreenDisappear(NULL,NULL);

            currentScreen = lastScreen;
        }
        else
        {
            // hide all of the screens
            mgScreensDisappear();
        }
    }

    if (screennum != -1)
    {
        switch (screennum)
        {
            case MGS_Connection_Method  :
                strcpy(screenname, "Connection_Method");
            break;
            case MGS_Internet_Login     :
                strcpy(screenname, "Internet_Login");
            break;
            case MGS_Password_Change    :
                strcpy(screenname, "Password_Change");
            break;
            case MGS_LAN_Login          :
                strcpy(screenname, "LAN_Login");
            break;
            case MGS_Connecting         :
                strcpy(screenname, "Connecting");
            break;
            case MGS_New_Account        :
                strcpy(screenname, "New_Account");
            break;
            case MGS_Channel_Chat       :
                strcpy(screenname, "Channel_Chat");
            break;
            case MGS_Available_Channels :
                strcpy(screenname, "Available_Channels");
            break;
            case MGS_Player_Options     :
                strcpy(screenname, "Player_Options");
            break;
            case MGS_Create_Channel     :
                strcpy(screenname, "Create_Channel");
            break;
            case MGS_Available_Games    :
                strcpy(screenname, "Available_Games");
            break;
            case MGS_Player_Wait        :
                strcpy(screenname, "Player_Wait");
            break;
            case MGS_Captain_Wait       :
                strcpy(screenname, "Captain_Wait");
            break;
            case MGS_Basic_Options      :
                strcpy(screenname, "Basic_Options");
            break;
            case MGS_Advanced_Options   :
                strcpy(screenname, "Advanced_Options");
            break;
            case MGS_Resource_Options   :
                strcpy(screenname, "Resource_Options");
            break;
            case MGS_Skirmish_Basic     :
                strcpy(screenname, "Skirmish_Basic");
            break;
            case MGS_Skirmish_Advanced  :
                strcpy(screenname, "Skirmish_Advanced");
            break;
            case MGS_Skirmish_Resource  :
                strcpy(screenname, "Skirmish_Resource");
            break;
            case MGS_Game_Password :
                strcpy(screenname, "Game_Password");
            break;
            case MGS_Room_Password :
                strcpy(screenname, "Room_Password");
            break;
            case MGS_Quit_WON :
                strcpy(screenname, "Quit_WON");
            break;
            case MGS_Message_Box :
                strcpy(screenname, "Message_Box");
            break;
            case MGS_Download_Patch:
                strcpy(screenname, "Download_Patch");
            break;
            case MGS_Downloading_Patch:
                strcpy(screenname, "Downloading_Patch");
            break;
            case MGS_Basic_Options_Change :
                strcpy(screenname, "Basic_Options_Change");
            break;
            case MGS_Advanced_Options_Change:
                strcpy(screenname, "Advanced_Options_Change");
            break;
            case MGS_Resource_Options_Change:
                strcpy(screenname, "Resource_Options_Change");
            break;
            case MGS_Basic_Options_View:
                strcpy(screenname, "Basic_Options_View");
            break;
            case MGS_Advanced_Options_View:
                strcpy(screenname, "Advanced_Options_View");
            break;
            case MGS_Resource_Options_View:
                strcpy(screenname, "Resource_Options_View");
            break;

            //LAN screens
            case LGS_LAN_Login          :
                strcpy(screenname, "LLAN_Login");
            break;
            case LGS_Channel_Chat       :
                strcpy(screenname, "LChannel_Chat");
            break;
            case LGS_Player_Options     :
                strcpy(screenname, "Player_Options");
            break;
            case LGS_Available_Games    :
                strcpy(screenname, "LAvailable_Games");
            break;
            case LGS_Player_Wait        :
                strcpy(screenname, "LPlayer_Wait");
            break;
            case LGS_Captain_Wait       :
                strcpy(screenname, "LCaptain_Wait");
            break;
            case LGS_Quit_WON :
                strcpy(screenname, "LQuit_WON");
            break;
            case LGS_Message_Box :
                strcpy(screenname, "LMessage_Box");
            break;
            case LGS_Game_Password :
                strcpy(screenname, "LGame_Password");
            break;

            // Choose Server screens
            case MGS_Choose_Server:
                strcpy(screenname, "ChooseServer");
            break;

            default: dbgFatal(DBG_Loc,"Unknown MultiplayerGame screen to display"); break;
        }

        // start the screen that we want

        mgBaseRegion = feScreenStart(ghMainRegion, screenname);
        currentScreen = screennum;

    }

    lastScreen = curscreen;
}



/*=============================================================================
    Functions for locking and unlocking mutualy exclusive blocks of code.:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : LockMutex
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void LockMutex(void *mutex)
{
    int result = SDL_mutexP(mutex);
    dbgAssert(result != -1);
}

/*-----------------------------------------------------------------------------
    Name        : UnLockMutex
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void UnLockMutex(void *mutex)
{
    int result = SDL_mutexV(mutex);
    dbgAssert(result != -1);
}

/*-----------------------------------------------------------------------------
    Name        : CreateMutex
    Description : creates a mutex
    Inputs      :
    Outputs     :
    Return      : returns mutex
----------------------------------------------------------------------------*/
void *gameCreateMutex()
{
    return SDL_CreateMutex();
}

/*-----------------------------------------------------------------------------
    Name        : CloseMutex
    Description : closes a mutex
    Inputs      : mutex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gameCloseMutex(void *mutex)
{
    SDL_DestroyMutex(mutex);
}

/*=============================================================================
    Miscelanious functions to encapsulate certain repetetive things.:
=============================================================================*/

// this function adds a chat message to the room chat window
void mgAddChatToRoomChat(chatlist *newchat, listwindowhandle listwindow, LinkedList *list)
{
    fonthandle oldfont;
    sdword     width,addwidth,nCharacters,length;
    char       temp[512];
    chatlist  *chatinfo;
    color      col;

    oldfont = fontMakeCurrent(mgChatWindowFont);

    switch (newchat->messagetype)
    {
        case MG_NORMALCHAT:
        {
            sprintf(temp,"<%s>  ",newchat->userName);
            width = MG_HorzSpacing + fontWidth(temp);
            if (list == &listofchatinfo)
                col = mgNormalChatColor;
            else
                col = mgGameNormalChatColor;
        }
        break;
        case MG_WHISPEREDCHAT:
        {
            sprintf(temp,"<%s>%s  ",newchat->userName, strGetString(strWhisperedMessage));
            width = MG_HorzSpacing + fontWidth(temp);
            if (list == &listofchatinfo)
                col = mgPrivateChatColor;
            else
                col = mgGameWhisperedColor;
        }
        break;
        case MG_MESSAGECHAT:
        {
            sprintf(temp,"%s",newchat->userName);
            width = MG_HorzSpacing + fontWidth(temp);
            if (list == &listofchatinfo)
                col = mgMessageChatColor;
            else
                col = mgGameMessageChatColor;
        }
        break;
        case MG_WRAPEDCHAT:
        {
            width = newchat->indent;
            col = newchat->baseColor;
        }
        break;
        case MG_GENERALMESSAGE:
        {
            width = 0;
            col = mgMessageChatColor;
            strcpy(temp, "");
        }
        break;
        default :
            width = 0;
        break;
    }

    if (width+fontWidth(newchat->chatstring) > chatwindowwidth)
    {
        chatinfo = (chatlist *)memAlloc(sizeof(chatlist),"GameChat",NonVolatile);
        strcpy(chatinfo->userName, newchat->userName);
        chatinfo->baseColor = col;
        chatinfo->messagetype = MG_WRAPEDCHAT;
        chatinfo->index = newchat->index;
        chatinfo->indent = 0;

        nCharacters = strlen(newchat->chatstring);
        addwidth = fontWidth(newchat->chatstring);
        while (nCharacters>0 && width + addwidth > chatwindowwidth)
        {
            addwidth = fontWidthN(newchat->chatstring,nCharacters);
            nCharacters--;
        }

        length = nCharacters;

        while ((newchat->chatstring[nCharacters] != ' ') && (nCharacters > 0) )
        {
            nCharacters--;
        }
        if (nCharacters == 0)
        {
            strcpy(chatinfo->chatstring, newchat->chatstring + length);
            newchat->chatstring[length] = 0;
        }
        else
        {
            strcpy(chatinfo->chatstring, newchat->chatstring + nCharacters + 1);
            newchat->chatstring[nCharacters] = 0;
        }

        listAddNode(list, &newchat->link, newchat);
        if (listwindow != NULL)
        {
            newchat->item = uicListAddItem(listwindow,(ubyte *)newchat,0,UICLW_AddToTail);
        }

        mgAddChatToRoomChat(chatinfo, listwindow, list);
    }
    else
    {
        listAddNode(list, &newchat->link, newchat);
        if (listwindow != NULL)
        {
            newchat->item = uicListAddItem(listwindow,(ubyte *)newchat,0,UICLW_AddToTail);
        }
    }

    if (listwindow != NULL)
    {
        while (listofchatinfo.num > MG_MaxChatHistory)
        {
            newchat = listGetStructOfNode(listofchatinfo.head);
            uicListRemoveItem(listwindow, newchat->item);

            listDeleteNode(listofchatinfo.head);
        }
    }
    else
        while (listofchatinfo.num > MG_MaxChatHistory)
        {
            listDeleteNode(listofchatinfo.head);
        }

    fontMakeCurrent(oldfont);
}

/*=============================================================================
    Callbacks for the connection type screen.:
=============================================================================*/

void mgConnectionBack(char *name, featom *atom)
{
    mgBaseRegion = NULL;
    mgScreensDisappear();
    mgShutdownMultiPlayerGameScreens();
    feScreenStart(ghMainRegion, "Main_game_screen");
//    mgScreensAdded=0;

    currentScreen = -1;
}

void mgSkirmish(char *name, featom *atom)
{
#if defined(DLPublicBeta)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        return;
    }
    LANGame = FALSE;
    IPGame = 1; // DO NOT USE TRUE for CPP reasons
    tpGameCreated.numComputers  = 1;
    tpGameCreated.numPlayers    = 1;
    mgShowScreen(MGS_Skirmish_Basic, TRUE);

#ifdef _MACOSX_FIX_ME
	multiPlayerGame = FALSE;
#endif

#endif
}

void mgInternetWON(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        return;
    }
    WaitingForGame  = FALSE;
    GameCreator     = FALSE;
    LANGame = FALSE;
    IPGame = 1; // DO NOT USE TRUE for CPP reasons
    tpGameCreated.numComputers  = 0;

    if (!titanStart(LANGame,IPGame))
    {
        mgPrepareMessageBox(strGetString(strNoInternetTCPIP),NULL);
        mgShowScreen(MGS_Message_Box,FALSE);
        return;
    }
    mgShowScreen(MGS_Internet_Login,TRUE);
#endif
}

void mgLANIPX(char *name, featom *atom)
{
    //mgShowScreen(MGS_LAN_Login,TRUE);

#if defined(DLPublicBeta) || defined(Downloadable)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        return;
    }
    LanProtocalsValid = 0;
    LANGame = TRUE;

    if (titanCheckCanNetwork(LANGame,0))
    {
        LanProtocalsValid |= LANIPX_VALID;
    }
    else
    {
        // disable IPX button
        if (LanProtocalButton == 0)
        {
            LanProtocalButton = 1;
        }
    }

    if (titanCheckCanNetwork(LANGame,1))
    {
        LanProtocalsValid |= LANTCPIP_VALID;
    }
    else
    {
        // disable TCP/IP button
        if (LanProtocalButton == 1)
        {
            LanProtocalButton = 0;
        }
    }

    mgShutdownMultiPlayerGameScreens();
    lgStartMultiPlayerLANGameScreens(ghMainRegion, 0, 0, 0, FALSE);
#endif
}

/*=============================================================================
    Callbacks for the Login screen to Internet WON:
=============================================================================*/

void mgFirewallButton(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, firewallButton);
    }
    else
    {
        firewallButton = (ubyte)(size_t)atom->pData;
    }
}

void mgValidateName(void)
{
    char    temp[64];
    sdword  index,pos;

    strcpy(temp, utyName);

    for (index=0,pos=0;index < strlen(temp); index++)
    {
        if (!( ((temp[index] >= 0x00) && (temp[index] <=0x1F)) ||
                (temp[index] == 0x22) ||
                (temp[index] == 0x27) ||
                (temp[index] == 0x7F)  ) )
        {
            utyName[pos++] = temp[index];
        }

    }

    utyName[pos] = 0x00;
}

// This function will strip all of the invalid characters from peoples names!
void mgVerifyValidNameChars(textentryhandle entry, bool login)
{
    char key = entry->textBuffer[entry->iCharacter-1];

    if (!login)
    {
        if ( ((key >= 0x00) && (key <=0x1F)) ||
              (key == 0x22) ||
              (key == 0x27) ||
              (key == 0x2C) ||
              (key == 0x2E) ||
              (key == 0x2F) ||
              (key == 0x3B) ||
              (key == 0x5C) ||
              (key == 0x60) ||
              (key == 0x7C) ||
              (key == 0x7F)  )
        {
            uicBackspaceCharacter(entry);
        }
    }
    else
    {
        if ( ((key >= 0x00) && (key <=0x1F)) ||
              (key == 0x22) ||
              (key == 0x27) ||
              (key == 0x7F)  )
        {
            uicBackspaceCharacter(entry);
        }
    }
}

void mgNameEntry(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgNameEntryBox = (textentryhandle)atom->pData;
        mgValidateName();
        uicTextEntrySet(mgNameEntryBox,utyName,strlen(utyName)+1);
        uicTextBufferResize(mgNameEntryBox,MAX_USERNAME_LENGTH);
        uicTextEntryInit(mgNameEntryBox,UICTE_UserNameEntry|UICTE_ChatTextEntry);
        // only accept valid characters and call this function for every key press
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            strcpy(utyName,mgNameEntryBox->textBuffer);
        break;
        case CM_GainFocus :
        break;
        case CM_KeyPressed :
            if (currentScreen==MGS_Internet_Login)
            {
                mgVerifyValidNameChars(mgNameEntryBox, TRUE);
            }
            else if (currentScreen==MGS_New_Account)
            {
                mgVerifyValidNameChars(mgNameEntryBox, FALSE);
            }
        break;

    }
}

void mgPasswordEntry(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgPasswordEntryBox = (textentryhandle)atom->pData;
        uicTextEntrySet(mgPasswordEntryBox,utyPassword,strlen(utyPassword)+1);
        uicTextEntryInit(mgPasswordEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgPasswordEntryBox,MAX_PASSWORD_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            strcpy(utyPassword,mgPasswordEntryBox->textBuffer);
        break;
        case CM_GainFocus :
        break;
    }
}

void mgChangePassword(char *name, featom *atom)
{
    mgConnectingScreenGoto = MGS_Internet_Login;

    // verify we have a valid version
    if (!titanCheckValidVersion(versionString))
    {
        // we don't have a valid version, time for patching?
        mgShowScreen(MGS_Download_Patch,FALSE);
        return;
    }

    strcpy(tempname, utyName);

    mgShowScreen(MGS_Password_Change,TRUE);
}

void mgNewAccount(char *name, featom *atom)
{
    mgConnectingScreenGoto = MGS_Internet_Login;

    // verify we have a valid version
    if (!titanCheckValidVersion(versionString))
{
        // we don't have a valid version, time for patching?
        mgShowScreen(MGS_Download_Patch,FALSE);
        return;
    }

    mgShowScreen( MGS_New_Account, TRUE);
}

void mgLaunchWON(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    listDeleteAll(&listofgames);
    listDeleteAll(&listofchannels);
    listDeleteAll(&listofservers);
    listDeleteAll(&listofchatinfo);
    listDeleteAll(&listofusersinfo);
    listDeleteAll(&listtoping);
    listDeleteAll(&statusitemlist);
    listDeleteAll(&listofplayers);
    listDeleteAll(&listofplayersold);
    listDeleteAll(&listofgamechatinfo);
    listDeleteAll(&listofIgnoreUsers);
    listDeleteAll(&listofBannedUsers);

    mgPlayerLimit = 0;

    if (noAuthorization)
    {
        strcpy(utyName,mgNameEntryBox->textBuffer);

        mgShowScreen(MGS_Channel_Chat,TRUE);

        LoggedIn = TRUE;
    }
    else
    {
        // do they at least have a two character name before they try to log on
        if (strlen(mgNameEntryBox->textBuffer) < 2)
            return;

#if 0       // if we don't have valid version information yet, titanCheckValidVersion will assume we have a correct version
            // and return TRUE
        // verify we have valid version information
        if (!HaveValidVersions)
        {
            mgPrepareMessageBox(strGetString(strErrorStillWaitingVersionInfo),NULL);
            mgShowScreen(MGS_Message_Box,FALSE);
            return;
        }
#endif

        mgConnectingScreenGoto = MGS_Internet_Login;

        // verify we have a valid version
        if (!titanCheckValidVersion(versionString))
        {
            // we don't have a valid version, time for patching?
            mgShowScreen(MGS_Download_Patch,FALSE);
            return;
        }

        // we are trying to log on to won
        mgQueryType = MG_WONLogin;

        strcpy(utyName,mgNameEntryBox->textBuffer);

        // call the autenticate function
        authAuthenticate(mgNameEntryBox->textBuffer,mgPasswordEntryBox->textBuffer);

        // bring up the connecting status screen
        mgShowScreen(MGS_Connecting,FALSE);

        mgConnectingScreenGoto = MGS_Internet_Login;

        mgDisplayMessage(strGetString(strSendingLogin));

        //authReceiveReply(StatusCommon_Success);
    }
#endif
}

void mgBackToConnection(char *name, featom *atom)
{
    // closedown the titan engine
    titanShutdown();

#if PUBLIC_BETA
    if (betaVerifying)
        mgConnectionBack(NULL,NULL);
    else
#endif
        mgShowScreen(MGS_Connection_Method,TRUE);

}

/*=============================================================================
    Callbacks for the Password Changer Screen.:
=============================================================================*/


void mgOldPasswordChangeEntry(char*name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgOldPasswordChangeEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgOldPasswordChangeEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgOldPasswordChangeEntryBox,MAX_PASSWORD_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgNewPasswordChangeEntry(char*name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgNewPasswordChangeEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgNewPasswordChangeEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgNewPasswordChangeEntryBox,MAX_PASSWORD_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgConfirmPasswordChangeEntry(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgConfirmPasswordChangeEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgConfirmPasswordChangeEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgConfirmPasswordChangeEntryBox,MAX_PASSWORD_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgChangePasswordBack(char*name,featom *atom)
{
    strcpy(utyName, tempname);

    mgShowScreen(MGS_Internet_Login,TRUE);
}

void mgChangePasswordNow(char *name, featom *atom)
{
    if (strlen(mgNameEntryBox->textBuffer) < 2)
        return;

    if ((strlen(mgNewPasswordChangeEntryBox->textBuffer) < 2) ||
        (strcmp(mgNewPasswordChangeEntryBox->textBuffer,mgConfirmPasswordChangeEntryBox->textBuffer) != 0))
    {
        mgPrepareMessageBox(strGetString(strErrorTypingPassword),strGetString(strMustBeAtLeast2Chars));
        mgShowScreen(MGS_Message_Box,FALSE);
        return;
    }

    // trying to change our password
    mgQueryType = MG_WONChangePassword;

    strcpy(utyName,mgNameEntryBox->textBuffer);
    strcpy(utyPassword, mgNewPasswordChangeEntryBox->textBuffer);

    // call the pasword changer function.
    authChangePassword(mgNameEntryBox->textBuffer,mgOldPasswordChangeEntryBox->textBuffer,mgNewPasswordChangeEntryBox->textBuffer);

    mgShowScreen(MGS_Connecting,FALSE);

    mgConnectingScreenGoto = MGS_Password_Change;

    mgDisplayMessage(strGetString(strChangingPassword));
}

/*=============================================================================
    Callbacks for the New account screen:
=============================================================================*/

void mgNewAccountPassword(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgNewAccountPasswordEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgNewAccountPasswordEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgNewAccountPasswordEntryBox,MAX_PASSWORD_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgNewAccountConfirm(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgNewAccountConfirmEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgNewAccountConfirmEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgNewAccountConfirmEntryBox,MAX_PASSWORD_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgNewAccountBack(char *name, featom *atom)
{
    mgShowScreen(MGS_Internet_Login,TRUE);
}

void mgCreateNewAccountNow(char *name, featom *atom)
{
    if (strlen(mgNameEntryBox->textBuffer) < 2)
        return;

    if ((strlen(mgNewAccountPasswordEntryBox->textBuffer) < 2) ||
        (strcmp(mgNewAccountPasswordEntryBox->textBuffer,mgNewAccountConfirmEntryBox->textBuffer) != 0))
    {
        mgPrepareMessageBox(strGetString(strErrorTypingPassword),strGetString(strMustBeAtLeast2Chars));
        mgShowScreen(MGS_Message_Box,FALSE);
        return;
    }

    // creating new user
    mgQueryType = MG_WONNewUser;

    strcpy(utyName,mgNameEntryBox->textBuffer);
    strcpy(utyPassword, mgNewAccountPasswordEntryBox->textBuffer);

    // create the new user
    authCreateUser(mgNameEntryBox->textBuffer,mgNewAccountPasswordEntryBox->textBuffer);

    mgShowScreen(MGS_Connecting,FALSE);

    mgConnectingScreenGoto = MGS_New_Account;

    mgDisplayMessage(strGetString(strCreatingUser));
}

/*=============================================================================
    Callbacks for the channel chatting screen.:
=============================================================================*/

void mgChatTextEntry(char*name,featom *atom)
{
    chatlist *newchat;
    userlist *user;
    char      temp[512];
    wchar_t aWideStringP[MAX_CHATSTRING_LENGTH];
    size_t  aNumChars;
    sdword  CommandType;

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgChatTextEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgChatTextEntryBox,UICTE_NoLossOfFocus|UICTE_ChatTextEntry|UICTE_SpecialChars);
        uicTextBufferResize(mgChatTextEntryBox,MAX_CHATSTRING_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_AcceptText:
            if (strlen(mgChatTextEntryBox->textBuffer)>0)
            {
                newchat = (chatlist *)memAlloc(sizeof(chatlist),"ListofChannelChat",NonVolatile);
                // check if message is a private message or not
                if ((user=mgParseChatEntry(mgChatTextEntryBox->textBuffer, FALSE))!=NULL)
                {
                    // yes it is
                    if (user->userID == mgMyChatUserID)
                    {
                        // talking to yourself eh!
                        strcpy(newchat->chatstring,mgChatTextEntryBox->textBuffer+strlen(user->userName)+2);
                        strcpy(newchat->userName, user->userName);
                        newchat->messagetype = MG_WHISPEREDCHAT;
                    }
                    else
                    {
                        char *strptr = mgChatTextEntryBox->textBuffer+strlen(user->userName)+2;

                        strcpy(newchat->chatstring,strptr);
                        strcpy(newchat->userName, utyName);
                        newchat->messagetype = MG_WHISPEREDCHAT;

                        // send the message to the intended recipient
                        aNumChars = mbstowcs(aWideStringP, strptr, (uword)strlen(strptr));
                        if (aNumChars != (size_t)-1)
                        {
                            SendPrivateChatMessage((unsigned long*)(&user->userID), 1,
                                                   (uword)(aNumChars*2),aWideStringP);
                        }
                    }
                }
                else if (mgCommandEntered(mgChatTextEntryBox->textBuffer, FALSE) != -1)
                {
                    mgCommandExecute(mgChatTextEntryBox->textBuffer, temp, FALSE);

                    // public chat message
                    strcpy(newchat->chatstring,temp);
                    strcpy(newchat->userName, "");
                    newchat->messagetype = MG_MESSAGECHAT;
                }
                else
                {
                    // public chat message
                    strcpy(newchat->chatstring,mgChatTextEntryBox->textBuffer);
                    strcpy(newchat->userName, utyName);
                    newchat->messagetype = MG_NORMALCHAT;

                    // broadcast it to everyone
                    aNumChars = mbstowcs(aWideStringP, mgChatTextEntryBox->textBuffer, strlen(mgChatTextEntryBox->textBuffer));
                    if (aNumChars != (size_t)-1)
                    {
                        BroadcastChatMessage((uword)(aNumChars*2),aWideStringP);
                        uicTextEntrySet(mgChatTextEntryBox,"",0);
                    }
                }

                mgAddChatToRoomChat(newchat, mgChatHistoryWindow, &listofchatinfo);

                uicTextEntrySet(mgChatTextEntryBox,"",0);
            }
        break;
        case CM_KeyPressed:
        {
            if ((user=mgParseChatEntry(mgChatTextEntryBox->textBuffer, FALSE))!=NULL)
            {
                sprintf(temp, "/%s ", user->userName);
                if (strlen(mgChatTextEntryBox->textBuffer) < strlen(temp))
                    uicTextEntrySet(mgChatTextEntryBox, temp, strlen(temp));
            }
            else if ((CommandType = mgCommandComplete(mgChatTextEntryBox->textBuffer, temp, FALSE)) != -1)
            {
                if (strlen(mgChatTextEntryBox->textBuffer) < strlen(temp))
                    uicTextEntrySet(mgChatTextEntryBox, temp, strlen(temp));
            }
        }
        break;
        case CM_SpecialKey:
        {
#ifdef DEBUG_STOMP
            regVerify((regionhandle)&mgChatHistoryWindow->reg);
#endif
            if (mgChatTextEntryBox->key==PAGEUPKEY)
            {
                uicListWindowPageUp(mgChatHistoryWindow);
                mgChatHistoryWindow->reg.status |= RSF_DrawThisFrame;
            }
            else if (mgChatTextEntryBox->key==PAGEDOWNKEY)
            {
                uicListWindowPageDown(mgChatHistoryWindow);
                mgChatHistoryWindow->reg.status |= RSF_DrawThisFrame;
            }
        }
        break;
        case CM_GainFocus:
        break;

    }
}

void mgViewChannels(char *name,featom *atom)
{
    mgShowScreen(MGS_Available_Channels,TRUE);
}

void mgViewGames(char *name,featom *atom)
{
    mgShowScreen(MGS_Available_Games,TRUE);
}

void mgCreateGame(char *name,featom *atom)
{
    mgCreatingNetworkGame = TRUE;

    mgRestoretpGameCreated();
    mgResetNamePassword();

    mgShowScreen(MGS_Basic_Options,TRUE);
}

void mgBackToLogin(char *name,featom *atom)
{
    mgShowScreen(MGS_Quit_WON, FALSE);
}

void mgChangeColors(char *name, featom *atom)
{
    // save the colors so if they change their minds it is backed up
    BaseColorSave   = utyBaseColor;
    StripeColorSave = utyStripeColor;
    RaceSave = whichRaceSelected;

    cpSetColorsToModify(&utyBaseColor, &utyStripeColor);

    mgShowScreen(MGS_Player_Options,TRUE);
}

void mgDrawChatWindowItem(rectangle *rect, listitemhandle data)
{
    char            temp[512];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    chatlist       *chatinfo = (chatlist *)data->data;

    oldfont = fontMakeCurrent(mgChatWindowFont);

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing/2;

    switch (chatinfo->messagetype)
    {
        case MG_NORMALCHAT:
        {
            sprintf(temp,"<%s>  ",chatinfo->userName);
//            fontPrint(x,y,mgNormalChatColor,temp);
            fontPrint(x,y,colWhite,temp);
            x+=fontWidth(temp);

            c = mgNormalChatColor;
        }
        break;
        case MG_WHISPEREDCHAT:
        {
            sprintf(temp,"<%s>",chatinfo->userName);
            fontPrint(x,y,mgPrivateChatColor,temp);
            x+=fontWidth(temp);

            sprintf(temp, "%s  ", strGetString(strWhisperedMessage));
            fontPrint(x,y,mgWhisperedColor, temp);
            x+=fontWidth(temp);
            c = mgPrivateChatColor;
        }
        break;
        case MG_MESSAGECHAT:
        {
            sprintf(temp,"%s",chatinfo->userName);
            fontPrint(x,y,mgMessageChatColor,temp);
            x+=fontWidth(temp);

            c = mgMessageChatColor;
        }
        break;
        case MG_WRAPEDCHAT:
        {
            c = chatinfo->baseColor;
            x += chatinfo->indent - MG_HorzSpacing;
        }
        break;
        case MG_GENERALMESSAGE:
        {
            c = chatinfo->baseColor;
        }
        break;
    }

    sprintf(temp,"%s",chatinfo->chatstring);    // don't comment out these lines!
    fontPrint(x,y,c,temp);                      // don't comment out these lines!

    fontMakeCurrent(oldfont);
}

void mgChatWindowInit(char *name, featom *atom)
{
    fonthandle oldfont;
    Node      *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(mgChatWindowFont);
        mgChatHistoryWindow = (listwindowhandle)atom->pData;
        uicListWindowInit(mgChatHistoryWindow,
                          NULL,         //  title draw, no title
                          NULL,                             // double click process
                          0,            //  title height, no title
                          mgDrawChatWindowItem,           // item draw funtcion
                          fontHeight(" "), // item height
                          UICLW_AutoScroll);

        chatwindowwidth = atom->width-20;

        if (listofchatinfo.num!=0)
        {
            walk = listofchatinfo.head;

            while (walk!=NULL)
            {
                ((chatlist *)listGetStructOfNode(walk))->item =
                    uicListAddItem(mgChatHistoryWindow,(ubyte *)listGetStructOfNode(walk),0,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        fontMakeCurrent(oldfont);
        return;
    }

    if (FELASTCALL(atom))
    {
        mgChatHistoryWindow = NULL;
        return;
    }
}

void mgDrawUserNameWindowItem(rectangle *rect, listitemhandle data)
{
    char            temp[512];
    sdword          x, y;
    fonthandle      oldfont;
    userlist       *userinfo = (userlist *)data->data;

    oldfont = fontMakeCurrent(mgUserNameFont);

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing/2;

    sprintf(temp,"%s",userinfo->userName);
    fontPrint(x,y,mgUserNameColor,temp);

    fontMakeCurrent(oldfont);
}

void mgUserNameWindowInit(char *name, featom *atom)
{
    fonthandle oldfont;
    Node      *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(mgUserNameFont);
        mgUserNameWindowWindow = (listwindowhandle)atom->pData;
        uicListWindowInit(mgUserNameWindowWindow,
                          NULL,         //  title draw, no title
                          NULL,         //  title click process, no title
                          0,            //  title height, no title
                          mgDrawUserNameWindowItem,       // item draw funtcion
                          fontHeight(" "), // item height
                          0);

        if (listofusersinfo.num!=0)
        {
            walk = listofusersinfo.head;

            while (walk!=NULL)
            {
                ((userlist *)listGetStructOfNode(walk))->item =
                    uicListAddItem(mgUserNameWindowWindow,(ubyte *)listGetStructOfNode(walk),0,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        mgUserNameWindowWindow = NULL;
        return;
    }
}

void mgCurrentChannelDraw(featom *atom, regionhandle region)
{
    fonthandle oldfont;
    wchar_t *channelname;
    static char asciichannelname[MAX_CHANNEL_NAME_LEN];

    oldfont = fontMakeCurrent(mgCurrentChannelFont);

    channelname = GetCurrentChannel();
    if (channelname[0] == 0)
        fontPrint(region->rect.x0,region->rect.y0, mgCurrentChannelColor, strGetString(strNoRoom));
    else
    {
        if (wcstombs(asciichannelname, channelname, MAX_CHANNEL_NAME_LEN) > 0)
            fontPrint(region->rect.x0,region->rect.y0, mgCurrentChannelColor, asciichannelname);
    }

    fontMakeCurrent(oldfont);
}


/*=============================================================================
    Callbacks for the Channel Picker screen.:
=============================================================================*/

void mgJoinChannelNow(wchar_t *channelname,wchar_t *description)
{
    cJoinChannelRequest(channelname,description);

    listDeleteAll(&listofgames);
    listDeleteAll(&listofchatinfo);
    listDeleteAll(&listofusersinfo);

    // clear room ready for use fix later

    mgShowScreen(MGS_Connecting,FALSE);

    mgConnectingScreenGoto = MGS_Available_Channels;
}

void mgJoinChannel(char*name,featom*atom)
{
#ifndef _MACOSX_FIX_ME
    sdword       i;
    channellist *channelinfo;

    if (mgListOfChannelsWindow->CurLineSelected!=NULL)
    {
        channelinfo = (channellist *)mgListOfChannelsWindow->CurLineSelected->data;

        for (i = 0; i < tpChannelList.numChannels; i++)
        {
            if (wcscmp(tpChannelList.tpChannels[i].ChannelName,channelinfo->channelname)==0)
            {
                if (tpChannelList.tpChannels[i].roomflags & ROOMFLAGS_PASSWORDPROTECTED)
                {
                    wcscpy(joinchannelname,channelinfo->channelname);
                    wcscpy(joinchanneldescription,channelinfo->description);
                    mgShowScreen(MGS_Room_Password,FALSE);
                }
                else
                {
                    ChannelPassword[0] = 0;
                    mgJoinChannelNow(channelinfo->channelname,channelinfo->description);
                }
            }
        }
    }
#endif
}

void mgCreateChannel(char*name,featom*atom)
{
    mgShowScreen(MGS_Create_Channel,TRUE);
}

void mgBackToChannelChat(char*name,featom *atom)
{
    donerefreshing=TRUE;

    if (WaitingForGame)
    {
        if (GameCreator)
        {
            mgShowScreen(MGS_Captain_Wait,TRUE);
        }
        else
        {
            mgShowScreen(MGS_Player_Wait,TRUE);
        }
    }
    else
    {
        mgShowScreen(MGS_Channel_Chat,TRUE);
    }
}


void mgDrawListOfChannelsTitle(rectangle *rect)
{
    sdword          x, y;
    fonthandle      oldfont;

    oldfont = fontMakeCurrent(mgChannelListTitleFont);

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing;

    fontPrintf(x,y,mgChannelTitleColor,strGetString(strChannelNameHeading));
    x += mgChannelNameWidth;

    fontPrintf(x,y,mgChannelTitleColor,strGetString(strChannelDescHeading));
    x += mgChannelDescriptionWidth;

    if (strCurLanguage>=3)
    {
        x -= 15;
    }

    fontPrintf(x,y,mgChannelTitleColor,strGetString(strChannelNumberHeading));
    x += mgChannelNumberWidth;

    fontMakeCurrent(oldfont);
}

void mgDrawListOfChannelsItem(rectangle *rect, listitemhandle data)
{
    char            temp[512];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    channellist    *channelinfo = (channellist *)data->data;

    oldfont = fontMakeCurrent(mgChannelListFont);

    if (data->flags&UICLI_Selected)
        c = mgChannelListSelectedColor;
    else
        c = mgChannelListNormalColor;

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing/2;

    wcstombs(temp,channelinfo->channelname,512);
    fontPrint(x,y,c,temp);

    x += mgChannelNameWidth;

    wcstombs(temp,channelinfo->description,512);
    fontPrint(x,y,c,temp);

    x += mgChannelDescriptionWidth;

    if (channelinfo->numpeople >= 0)
    {
        sprintf(temp,"%d",channelinfo->numpeople);
        fontPrint(x,y,c,temp);
    }

    x += mgChannelNumberWidth;

    if (strCurLanguage>=3)
    {
        x -= 15;
    }

    if (channelinfo->roomflags)
    {
        if (channelinfo->roomflags & ROOMFLAGS_PASSWORDPROTECTED)
        {
            fontPrint(x,y,c,strGetString(strPasswordProtected));
        }
    }

    fontMakeCurrent(oldfont);
}

void mgListOfChannelsInit(char *name, featom *atom)
{
    fonthandle  oldfont;
    sdword      titleheight, itemheight;
    Node       *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(mgChannelListTitleFont);
        titleheight = fontHeight(" ")*2;
        fontMakeCurrent(mgChannelListFont);
        itemheight = fontHeight(" ")+MG_VertSpacing;

        mgListOfChannelsWindow = (listwindowhandle)atom->pData;
        uicListWindowInit(mgListOfChannelsWindow,
                          mgDrawListOfChannelsTitle,      // title draw, no title
                          NULL,                           // title click process, no title
                          titleheight,                    // title height, no title
                          mgDrawListOfChannelsItem,       // item draw funtcion
                          itemheight,                     // item height
                          UICLW_CanSelect);

        if (listofchannels.num!=0)
        {
            walk = listofchannels.head;

            while (walk!=NULL)
            {
                ((channellist *)listGetStructOfNode(walk))->item =
                    uicListAddItem(mgListOfChannelsWindow,(ubyte *)listGetStructOfNode(walk),UICLI_CanSelect,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        titanRefreshRequest(NULL);
        //titanQueryRoutingServers();
        refreshbaby = taskCallBackRegister(Refresh, 0, NULL, (real32)TITAN_PICKER_REFRESH_TIME);
        donerefreshing=FALSE;

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        taskCallBackRemove(refreshbaby);
        donerefreshing = TRUE;

        mgListOfChannelsWindow = NULL;
        return;
    }
    else if (mgListOfChannelsWindow->message == CM_DoubleClick)
    {
        mgJoinChannel(NULL, NULL);
    }
}

/*=============================================================================
    Callbacks for the Channel Creation Screen.:
=============================================================================*/

void mgChannelNameEntry(char*name,featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgChannelNameEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgChannelNameEntryBox,0);
        uicTextBufferResize(mgChannelNameEntryBox,MAX_CHANNELNAME_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgChannelDescriptionEntry(char*name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgChannelDescriptionEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgChannelDescriptionEntryBox,0);
        uicTextBufferResize(mgChannelDescriptionEntryBox,MAX_DESCRIPTION_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgChannelProtected(char*name,featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        ChannelProtectedFlag = 0;
        feToggleButtonSet(name, ChannelProtectedFlag);
    }
    else
    {
        if (FECHECKED(atom))
            ChannelProtectedFlag = 1;
        else
            ChannelProtectedFlag = 0;
    }
}

void mgChannelPasswordEntry(char*name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgChannelPasswordEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgChannelPasswordEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgChannelPasswordEntryBox,MAX_PASSWORD_LENGTH-2);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgChannelConfirmEntry(char*name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgChannelConfirmEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgChannelConfirmEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgChannelConfirmEntryBox,MAX_PASSWORD_LENGTH-2);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

bool channelAlreadyExists(wchar_t *channelname)
{
#ifndef _MACOSX_FIX_ME
    sdword i;

    tpLockChannelList();

    for (i = 0; i < tpChannelList.numChannels; i++)
    {
        if (wcscmp(tpChannelList.tpChannels[i].ChannelName,channelname) == 0)
        {
            tpUnLockChannelList();
            return TRUE;
        }
    }
    tpUnLockChannelList();
#endif

    return FALSE;
}

void mgCreateChannelNow(char*name,featom *atom)
{
    static wchar_t widechannelname[MAX_CHANNEL_NAME_LEN];
    static wchar_t widechanneldescription[MAX_CHANNEL_DESCRIPTION_LEN];

    if (strlen(mgChannelNameEntryBox->textBuffer) < 2)
    {
        return;
    }

    mbstowcs(widechannelname, mgChannelNameEntryBox->textBuffer, strlen(mgChannelNameEntryBox->textBuffer)+1);
    if (channelAlreadyExists(widechannelname))
    {
        mgPrepareMessageBox(strGetString(strErrorRoomAlreadyExists),NULL);
        mgShowScreen(MGS_Message_Box,FALSE);
        return;
    }

    if (ChannelProtectedFlag)
    {
        if ((strlen(mgChannelPasswordEntryBox->textBuffer) < 2) ||
            (strcmp(mgChannelPasswordEntryBox->textBuffer,mgChannelConfirmEntryBox->textBuffer) != 0))
        {
            mgPrepareMessageBox(strGetString(strErrorTypingPassword),strGetString(strMustBeAtLeast2Chars));
            mgShowScreen(MGS_Message_Box,FALSE);
            return;
        }

        mbstowcs(ChannelPassword, mgChannelPasswordEntryBox->textBuffer, strlen(mgChannelPasswordEntryBox->textBuffer)+1);
    }
    else
        ChannelPassword[0] = 0;

    mbstowcs(widechanneldescription, mgChannelDescriptionEntryBox->textBuffer, strlen(mgChannelDescriptionEntryBox->textBuffer)+1);

    mgShowScreen(MGS_Connecting,FALSE);

    mgConnectingScreenGoto = MGS_Create_Channel;

    cCreateChannel(widechannelname, widechanneldescription);

    listDeleteAll(&listofgames);
    listDeleteAll(&listofchatinfo);
    listDeleteAll(&listofusersinfo);
}

void mgBackToAvailableChannels(char *name, featom *atom)
{
    mgShowScreen(MGS_Available_Channels,TRUE);
}

/*=============================================================================
    Callbacks for the available games screen:
=============================================================================*/

Node    *pingwalk;
bool     firstping=FALSE;

void mgPingAllGames(char*name,featom *atom)
{
    Node       *walk, *isthere;
    newping    *pinginfo;
    bool        found=FALSE;
    gamelist   *game;
    newping    *ping;

    if (donepinging == FALSE)
    {
        listDeleteAll(&listtoping);

        pingwalk = NULL;
    }

    donepinging = FALSE;

    if (listofgames.num!=0)
    {
        walk = listofgames.head;

        listDeleteAll(&listtoping);

        while (walk!=NULL)
        {
            game = (gamelist *)listGetStructOfNode(walk);
            isthere = listtoping.head;
            found = FALSE;

            dbgAssert(!LANGame);

            while (isthere != NULL)
            {
                ping = (newping *)listGetStructOfNode(isthere);

                if (ping->IP==
                    GetIPFromInternetAddress(game->game.directoryCustomInfo.pingAddress))
                {
                    found = TRUE;
                    break;
                }

                isthere = isthere->next;
            }

            if ((!found) && (! (game->game.directoryCustomInfo.flag & GAME_IN_PROGRESS) ))
            {
                pinginfo  = memAlloc(sizeof(newping),"GamesToPing",NonVolatile);

                pinginfo->IP    = GetIPFromInternetAddress(game->game.directoryCustomInfo.pingAddress);
                pinginfo->Port  = GetPortFromInternetAddress(game->game.directoryCustomInfo.pingAddress);

                listAddNode(&listtoping, &pinginfo->link, pinginfo);
            }

            walk = walk->next;
        }

        firstping = TRUE;
        pinggamesbaby = taskCallBackRegister(PingAllGames, 0, NULL, (real32)PING_INTERVAL_TIME);
    }
}

void mgSeeDetails(char*name,featom*atom)
{

}

void titanJoinGameRequest(tpscenario *gametojoin)
{
    PlayerJoinInfo pInfo;

    mgGameInterestedIn(gametojoin->Name);

    pInfo.baseColor = utyBaseColor;
    pInfo.stripeColor = utyStripeColor;
    pInfo.colorsPicked = cpColorsPicked;
    pInfo.race = whichRaceSelected;
        pInfo.behindFirewall = titanBehindFirewall();
    strcpy(pInfo.PersonalName, utyName);

    if (!LANGame)
    {
        titanSetGameKey(gametojoin->directoryCustomInfo.sessionKey);
    }

    titanConnectToClient(&gametojoin->directoryCustomInfo.captainAddress);
    titanSendPacketTo(&gametojoin->directoryCustomInfo.captainAddress,TITANMSGTYPE_JOINGAMEREQUEST,&pInfo,sizeof(pInfo));
}

void mgRequestJoinGame(tpscenario *game)
{
    mgShowScreen(MGS_Connecting,FALSE);

    mgConnectingScreenGoto = MGS_Available_Games;

    mgDisplayMessage(strGetString(strRequestingToJoin));

    titanJoinGameRequest(game);
    listDeleteAll(&listofgamechatinfo);
    listDeleteAll(&listofplayers);
    listDeleteAll(&listofplayersold);
}

void mgJoinGame(char*name,featom*atom)
{
    gamelist *gameinfo;

    if (mgListOfGamesWindow->CurLineSelected!=NULL)
    {
        gameinfo = (gamelist *)mgListOfGamesWindow->CurLineSelected->data;

        if (AddressesAreEqual(gameinfo->game.directoryCustomInfo.captainAddress,myAddress))
        {
            mgPrepareMessageBox(strGetString(strCantJoinOwnGame),NULL);
            mgShowScreen(MGS_Message_Box,FALSE);
            return;
        }

        if (!forceLAN)      // later change to a more meaningful name fix later
        {
            if (!CheckNetworkVersionCompatibility(gameinfo->game.directoryCustomInfo.versionInfo))
            {
                mgPrepareMessageBox(strGetString(strDifferentVersions),strGetString(strMustUpgradeToSameVersion));
                mgShowScreen(MGS_Message_Box,FALSE);
                return;
            }
        }

#ifndef _MACOSX_FIX_ME
        if (wcslen(gameinfo->game.directoryCustomInfo.stringdata)>1)
        {
            joingame = &gameinfo->game;
            mgShowScreen(MGS_Game_Password,FALSE);
        }
        else
        {
            mgRequestJoinGame(&gameinfo->game);
        }
#endif
    }
}

void mgListOfGamesBack(char *name, featom *atom)
{
    if (WaitingForGame)
    {
        if (GameCreator)
        {
            mgShowScreen(MGS_Captain_Wait,TRUE);
        }
        else
        {
            mgShowScreen(MGS_Player_Wait,TRUE);
        }
    }
    else
    {
        mgShowScreen(MGS_Channel_Chat,TRUE);
    }
}

// the following functions are all callbacks to do with list windows

// callback for sorting the game list window
bool mgListOfGamesCompare(void *firststruct,void *secondstruct)
{
#ifndef _MACOSX_FIX_ME
    sdword i;
#endif

    gamelist *one = (gamelist *)(((listitemhandle)firststruct)->data);
    gamelist *two = (gamelist *)(((listitemhandle)secondstruct)->data);

    if (one->game.directoryCustomInfo.flag & GAME_IN_PROGRESS)
    {
        if (!(two->game.directoryCustomInfo.flag & GAME_IN_PROGRESS))
            return(FALSE);
    }
    else
    {
        if ((two->game.directoryCustomInfo.flag & GAME_IN_PROGRESS))
            return(FALSE);
    }

#ifndef _MACOSX_FIX_ME
    switch (mgListOfGamesWindow->sorttype)
    {
        case MG_SortByGameName:
            if ((i=wcscasecmp( one->game.Name,two->game.Name)) > 0)
                return (mgListOfGamesWindow->sortOrder);
            else
            {
                if (i != 0)
                    return (!mgListOfGamesWindow->sortOrder);
                else
                    return (FALSE);
            }

        case MG_SortByPingTime:
            if (one->pingtime > two->pingtime)
                return (mgListOfGamesWindow->sortOrder);
            else
            {
                if (one->pingtime != two->pingtime)
                    return (!mgListOfGamesWindow->sortOrder);
                else
                    return (FALSE);
            }

        case MG_SortByNumPlayers:
            if (one->game.directoryCustomInfo.numPlayers > two->game.directoryCustomInfo.numPlayers)
                return (mgListOfGamesWindow->sortOrder);
            else
            {
                if (one->game.directoryCustomInfo.numPlayers != two->game.directoryCustomInfo.numPlayers)
                    return (!mgListOfGamesWindow->sortOrder);
                else
                    return(FALSE);
            }

        case MG_SortByMapName:
        {
            wchar_t *onemapname = one->game.directoryCustomInfo.stringdata + 1+wcslen(one->game.directoryCustomInfo.stringdata);
            wchar_t *twomapname = two->game.directoryCustomInfo.stringdata + 1+wcslen(two->game.directoryCustomInfo.stringdata);

            if ((i=wcscasecmp( onemapname,twomapname)) > 0)
                return (mgListOfGamesWindow->sortOrder);
            else
            {
                if (i != 0)
                    return (!mgListOfGamesWindow->sortOrder);
                else
                    return(FALSE);
            }

        }
    }
#endif // _MACOSX_FIX_ME

    return FALSE;
}

// callback if the title is clicked on
void mgListOfGamesTitleClick(struct tagRegion *reg, sdword xClicked)
{
    sdword  x = reg->rect.x0+mgGameNameWidth;

    if (mgListOfGamesWindow->ListTotal==0) return;

    mgTitleFlashTimer = taskTimeElapsed;
#ifdef DEBUG_STOMP
    regVerify(reg);
#endif
    reg->status |= RSF_DrawThisFrame;

    if (xClicked < x)
    {
        if (mgListOfGamesWindow->sorttype == MG_SortByGameName)
            mgListOfGamesWindow->sortOrder = !mgListOfGamesWindow->sortOrder;
        else
            mgListOfGamesWindow->sortOrder = TRUE;
        mgListOfGamesWindow->sorttype = MG_SortByGameName;
        uicListSort(mgListOfGamesWindow,mgListOfGamesCompare);
        return;
    }
    x+=mgPingTimeWidth;

    if (xClicked < x)
    {
        if (mgListOfGamesWindow->sorttype == MG_SortByPingTime)
            mgListOfGamesWindow->sortOrder = !mgListOfGamesWindow->sortOrder;
        else
            mgListOfGamesWindow->sortOrder = TRUE;
        mgListOfGamesWindow->sorttype = MG_SortByPingTime;
        uicListSort(mgListOfGamesWindow,mgListOfGamesCompare);
        return;
    }
    x+=mgNumPlayerWidth;

    if (xClicked < x)
    {
        if (mgListOfGamesWindow->sorttype == MG_SortByNumPlayers)
            mgListOfGamesWindow->sortOrder = !mgListOfGamesWindow->sortOrder;
        else
            mgListOfGamesWindow->sortOrder = TRUE;

        mgListOfGamesWindow->sorttype = MG_SortByNumPlayers;
        uicListSort(mgListOfGamesWindow,mgListOfGamesCompare);
        return;
    }
    else
    {
        if (mgListOfGamesWindow->sorttype == MG_SortByMapName)
            mgListOfGamesWindow->sortOrder = !mgListOfGamesWindow->sortOrder;
        else
            mgListOfGamesWindow->sortOrder = TRUE;

        mgListOfGamesWindow->sorttype = MG_SortByMapName;
        uicListSort(mgListOfGamesWindow,mgListOfGamesCompare);
        return;
    }
}

void mgDrawTriangle(sdword x, color col)
{
    triangle tri;

    if (!mgListOfGamesWindow->sortOrder)
    {
        tri.x0 = -4 + x; tri.y0 = 4 + mgListOfGamesWindow->reg.rect.y0+MG_VertSpacing+4;
        tri.x1 = 4  + x; tri.y1 = 4 + mgListOfGamesWindow->reg.rect.y0+MG_VertSpacing+4;
        tri.x2 = 0  + x; tri.y2 = -4  + mgListOfGamesWindow->reg.rect.y0+MG_VertSpacing+4;
    }
    else
    {
        tri.x0 = -4 + x; tri.y0 = -4  + mgListOfGamesWindow->reg.rect.y0+MG_VertSpacing+4;
        tri.x1 = 0  + x; tri.y1 = 4 + mgListOfGamesWindow->reg.rect.y0+MG_VertSpacing+4;
        tri.x2 = 4  + x; tri.y2 = -4  + mgListOfGamesWindow->reg.rect.y0+MG_VertSpacing+4;
    }

    primTriSolid2(&tri, col);
}

// callback to draw the title bar for the list
void mgListOfGamesTitleDraw(rectangle *rect)
{
    sdword     x, y;
    fonthandle oldfont;
    color      flashcol;

    oldfont = fontMakeCurrent(mgGameListTitleFont);

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing;

    if (mgTitleFlashTimer + FLASH_TIMER > taskTimeElapsed)
    {
        bitSet(mgListOfGamesWindow->windowflags,UICLW_JustRedrawTitle);
#ifdef DEBUG_STOMP
        regVerify(&mgListOfGamesWindow->reg);
#endif
        mgListOfGamesWindow->reg.status |= RSF_DrawThisFrame;
        flashcol = mgTitleFlashColor;
    }
    else
        flashcol = mgGameListTitleColor;

    if (mgListOfGamesWindow->sorttype == MG_SortByGameName)
    {
        fontPrint(x,y,flashcol,strGetString(strGameNameHeading));
        mgDrawTriangle(x+fontWidth(strGetString(strGameNameHeading)), flashcol);
    }
    else
        fontPrint(x,y,mgGameListTitleColor,strGetString(strGameNameHeading));
    x+= mgGameNameWidth;

    if (mgListOfGamesWindow->sorttype == MG_SortByPingTime)
    {
        fontPrint(x,y,flashcol,strGetString(strGamePingHeading));
        mgDrawTriangle(x+fontWidth(strGetString(strGamePingHeading)), flashcol);
    }
    else
        fontPrint(x,y,mgGameListTitleColor,strGetString(strGamePingHeading));
    x+= mgPingTimeWidth;

    if (mgListOfGamesWindow->sorttype == MG_SortByNumPlayers)
    {
        fontPrint(x,y,flashcol,strGetString(strGameNumPlayerHeading));
        mgDrawTriangle(x+fontWidth(strGetString(strGameNumPlayerHeading)), flashcol);
    }
    else
        fontPrint(x,y,mgGameListTitleColor,strGetString(strGameNumPlayerHeading));
    x+= mgNumPlayerWidth;

    if (strCurLanguage >= 3)
    {
        x+=15;
    }

    if (mgListOfGamesWindow->sorttype == MG_SortByMapName)
    {
        fontPrint(x,y,flashcol,strGetString(strGameMapHeading));
        mgDrawTriangle(x+fontWidth(strGetString(strGameMapHeading)), flashcol);
    }
    else
        fontPrint(x,y,mgGameListTitleColor,strGetString(strGameMapHeading));
    x+=mgMapNameWidth;

    fontMakeCurrent(oldfont);
}

// callback to draw each item in the list
void mgListOfGamesItemDraw(rectangle *rect, listitemhandle data)
{
    char        temp[512];
    sdword      x, y;
    color       c;
    fonthandle  oldfont;
    gamelist   *gameinfo = (gamelist *)data->data;
    bool gameinprogress = gameinfo->game.directoryCustomInfo.flag & GAME_IN_PROGRESS;
    bool diffversion = (!CheckNetworkVersionCompatibility(gameinfo->game.directoryCustomInfo.versionInfo));

#ifndef _MACOSX_FIX_ME
    udword passwordlen;
#endif

    oldfont = fontMakeCurrent(mgListOfGamesFont);

    if (data->flags&UICLI_Selected)
        c = mgGameListSelectedColor;
    else
        c = mgGameListNormalColor;

    if (diffversion != 0)
        c = mgGameListDiffVersionColor;

    if (gameinprogress)
        c = mgGameListStartedColor;

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing/2;

    wcstombs(temp,gameinfo->game.Name,512);
    fontPrint(x,y,c,temp);
    x+=mgGameNameWidth;

    x+=mgPingTimeWidth;
    if (gameinfo->pingtime == -1)
        sprintf(temp,"?");
    else
        sprintf(temp,"%i",gameinfo->pingtime);

    fontPrint(x-fontWidth(temp)-fontWidth("W"),y,c,temp);

    x+=mgNumPlayerWidth;
    sprintf(temp,"%i",gameinfo->game.directoryCustomInfo.numPlayers);
    fontPrint(x-fontWidth(temp)-fontWidth("W"),y,c,temp);

#ifndef _MACOSX_FIX_ME
    passwordlen = wcslen(gameinfo->game.directoryCustomInfo.stringdata);

    wcstombs(temp,gameinfo->game.directoryCustomInfo.stringdata + 1+passwordlen,512);
    fontPrint(x,y,c,temp);
    x += mgMapNameWidth;

    if (gameinprogress)
    {
        fontPrint(x,y,c,strGetString(strInProgress));
    }
    else if (diffversion != 0)
    {
        fontPrint(x,y,c,strGetString(strDiffVersion));
    }
    else if (passwordlen > 1)
    {
        fontPrint(x,y,c,strGetString(strPasswordProtected));
    }

    fontMakeCurrent(oldfont);
#endif
}

// initilize the list of games window structure to needed settings
void mgListOfGamesInit(char *name, featom *atom)
{
    fonthandle      oldfont;
    sdword          titleheight, itemheight;
    Node           *walk;
    gamelist       *gameinfo;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(mgGameListTitleFont);
        titleheight = fontHeight(" ")*2;
        fontMakeCurrent(mgListOfGamesFont);
        itemheight  = fontHeight(" ")+MG_VertSpacing;

        mgListOfGamesWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(mgListOfGamesWindow,
                          mgListOfGamesTitleDraw,           // title draw, no title
                          mgListOfGamesTitleClick,          // title click process, no title
                          titleheight,                      // title height, no title
                          mgListOfGamesItemDraw,            // item draw funtcion
                          itemheight,                       // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);
        if (listofgames.num!=0)
        {
            walk = listofgames.head;

            while (walk!=NULL)
            {
                gameinfo = (gamelist *)listGetStructOfNode(walk);

                if (gameinfo->game.directoryCustomInfo.flag & GAME_IN_PROGRESS)
                    gameinfo->item = uicListAddItem(mgListOfGamesWindow, (ubyte *)gameinfo, 0, UICLW_AddToTail);
                else
                    gameinfo->item = uicListAddItem(mgListOfGamesWindow, (ubyte *)gameinfo, UICLI_CanSelect, UICLW_AddToHead);

                walk = walk->next;
            }
        }

        refreshbaby = taskCallBackRegister(Refresh, 0, NULL, (real32)TITAN_PICKER_REFRESH_TIME);
        donerefreshing=FALSE;

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        taskCallBackRemove(refreshbaby);
        donerefreshing = TRUE;

        mgListOfGamesWindow = NULL;
        return;
    }
    else if (mgListOfGamesWindow->message == CM_DoubleClick)
    {
        mgJoinGame(NULL, NULL);
    }
}

// function called to refresh the list of games structure
bool Refresh(udword num, void *data, struct BabyCallBack *baby)
{
    //static sdword refreshcount = 0;

    if (!donerefreshing)
    {
        if ((currentScreen == MGS_Available_Channels) || (currentScreen == MGS_Choose_Server))
        {
            titanRefreshRequest(NULL);
#if 0
            if ((++refreshcount & 1) == 0)
            {
                titanQueryRoutingServers();
            }
#endif
        }
    }

    return (donerefreshing);
}

bool PingAllGames(udword num, void *data, struct BabyCallBack *baby)
{
    Address addr;

    if ( (pingwalk == NULL) && (firstping == TRUE) )
    {
        firstping = FALSE;
        pingwalk = listtoping.head;
        if (pingwalk == NULL)
        {
            donepinging = TRUE;
            return (TRUE);
        }
    }
    else if ( (pingwalk == NULL) && (firstping == FALSE) )
    {
        donepinging = TRUE;
        return (TRUE);
    }

    dbgAssert(!LANGame);
    CreateInternetAddress(addr,((newping *)pingwalk)->IP,((newping *)pingwalk)->Port);

    titanSendPing(&addr,2);

    pingwalk = pingwalk->next;

    return (donepinging);
}

void ConvertUChar6ToAddress(ubyte *uchar6,Address *address)
{
    ubyte *IPPtr = (ubyte *)&address->AddrPart.IP;
    ubyte *PortPtr = (ubyte *)&address->Port;

    PortPtr[0] = uchar6[1];
    PortPtr[1] = uchar6[0];

    IPPtr[0] = uchar6[2];
    IPPtr[1] = uchar6[3];
    IPPtr[2] = uchar6[4];
    IPPtr[3] = uchar6[5];
}

void ConvertAddressToUChar6(ubyte *uchar6,Address *address)
{
    ubyte *IPPtr = (ubyte *)&address->AddrPart.IP;
    ubyte *PortPtr = (ubyte *)&address->Port;

    uchar6[0] = PortPtr[1];
    uchar6[1] = PortPtr[0];

    uchar6[2] = IPPtr[0];
    uchar6[3] = IPPtr[1];
    uchar6[4] = IPPtr[2];
    uchar6[5] = IPPtr[3];
}

bool PingAllServers(udword num, void *data, struct BabyCallBack *baby)
{
    serverlist *servertoping;
    Address addr;

    if (listofservers.num == 0)
    {
        return FALSE;
    }

    if (pingservernum >= listofservers.num)
    {
        pingservernum = 0;
    }

    servertoping = ConvertNumToPointerInList(&listofservers,pingservernum);
    if (servertoping == NULL)
    {
        return FALSE;
    }

    ConvertUChar6ToAddress(servertoping->addressstore,&addr);
    dbgMessage("\nSending ping");
    titanSendPing(&addr,4);

    pingservernum++;

    return FALSE;
}

/*=============================================================================
    Callbacks for the waiting for game screens.:
=============================================================================*/

void mgGameChatItemDraw(rectangle *rect, listitemhandle data)
{
    char            temp[512];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    chatlist       *chatinfo = (chatlist *)data->data;

    oldfont = fontMakeCurrent(mgGameChatFont);

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing/2;

    switch (chatinfo->messagetype)
    {
        case MG_NORMALCHAT:
        {
            sprintf(temp,"<%s>  ",chatinfo->userName);
            fontPrint(x,y,chatinfo->baseColor,temp);
            x+=fontWidth(temp);

            c = mgNormalChatColor;
        }
        break;
        case MG_WHISPEREDCHAT:
        {
            sprintf(temp,"<%s>",chatinfo->userName);
            fontPrint(x,y,chatinfo->baseColor,temp);
            x+=fontWidth(temp);

            sprintf(temp, "%s  ", strGetString(strWhisperedMessage));
            fontPrint(x,y,mgWhisperedColor, temp);
            x+=fontWidth(temp);
            c = mgPrivateChatColor;
        }
        break;
        case MG_MESSAGECHAT:
        {
            sprintf(temp,"%s",chatinfo->userName);
            fontPrint(x,y,mgGameMessageChatColor,temp);
            x+=fontWidth(temp);

            c = mgGameMessageChatColor;
        }
        break;
        case MG_WRAPEDCHAT:
        {
            c = chatinfo->baseColor;
            x += chatinfo->indent - MG_HorzSpacing;
        }
        break;
    }

    sprintf(temp,"%s",chatinfo->chatstring);
    fontPrint(x,y,c,temp);

    fontMakeCurrent(oldfont);
}

void mgGameChatWindowInit(char *name, featom *atom)
{
    fonthandle      oldfont;
    Node           *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(mgGameChatFont);

        mgGameChatWindowWindow = (listwindowhandle)atom->pData;
        uicListWindowInit(mgGameChatWindowWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          mgGameChatItemDraw,               // item draw funtcion
                          fontHeight(" "),   // item height
                          UICLW_AutoScroll);
        fontMakeCurrent(oldfont);

        if (listofgamechatinfo.num!=0)
        {
            walk = listofgamechatinfo.head;

            while (walk!=NULL)
            {
                ((chatlist *)listGetStructOfNode(walk))->item =
                    uicListAddItem(mgGameChatWindowWindow,(ubyte *)listGetStructOfNode(walk),0,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        return;
    }
    if (FELASTCALL(atom))
    {
        mgGameChatWindowWindow = NULL;
        return;
    }
}

void mgGameUserNameItemDraw(rectangle *rect, listitemhandle data)
{
    gameplayerinfo *playerinfo=(gameplayerinfo *)data->data;
    sdword          x, y;
    fonthandle      oldfont;

    oldfont = fontMakeCurrent(mgGameUserNameFont);

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing/2;

    fontPrintf(x,y,playerinfo->baseColor,"%s",playerinfo->name);

    fontMakeCurrent(oldfont);
}

void mgGameUserNameWindowInit(char *name, featom *atom)
{
    fonthandle      oldfont;
    Node           *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(mgGameUserNameFont);

        mgGameUserNameWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(mgGameUserNameWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          mgGameUserNameItemDraw,           // item draw funtcion
                          fontHeight(" "),   // item height
                          0);


        if (listofplayers.num!=0)
        {
            walk = listofplayers.head;

            while (walk!=NULL)
            {
                ((gameplayerinfo *)listGetStructOfNode(walk))->item =
                    uicListAddItem(mgGameUserNameWindow,(ubyte *)listGetStructOfNode(walk),0,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        mgGameUserNameWindow = NULL;
        return;
    }
}

void mgViewDetails(char*name,featom *atom)
{
    mgShowScreen(MGS_Basic_Options_View,TRUE);
}

void mgLeaveGame(char*name,featom*atom)
{
    mgPlayerLimit = 0;
    if (GameCreator)
    {
        titanGameDisolved(FALSE);

        mgShowScreen(MGS_Channel_Chat,TRUE);
        GameCreator = FALSE;
        WaitingForGame = FALSE;
    }
    else
    {
        titanLeaveGame(FALSE);

        GameCreator = FALSE;
        WaitingForGame = FALSE;

        mgShowScreen(MGS_Channel_Chat,TRUE);
    }
}

void mgSetupGame(char*name,featom*atom)
{
    BackupGameCreated = tpGameCreated;
    spCurrentSelectedBack = spCurrentSelected;

    mgShowScreen(MGS_Basic_Options_Change, TRUE);
}

void mgStartGameCB()
{
    sdword i;
    DirectoryCustomInfoMax buildDirectoryCustomInfo;

    tpGameCreated.flag |= GAME_IN_PROGRESS;
    for (i=tpGameCreated.numPlayers; i < tpGameCreated.numPlayers+tpGameCreated.numComputers; i++)
    {
        tpGameCreated.playerInfo[i].race = ranRandom(0)&0x00000001;
    }

    titanBroadcastPacket(TITANMSGTYPE_GAMEISSTARTING,&tpGameCreated,sizeof(tpGameCreated));

    // update directory server with inprogres flag

    generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
    titanReplaceGameInfo(tpGameCreated.Name,(DirectoryCustomInfo *)&buildDirectoryCustomInfo, TRUE);

    sigsPlayerIndex = 0;
    sigsNumPlayers = tpGameCreated.numPlayers;
    sigsPressedStartGame = TRUE;
}

void mgReallyStartGame(unsigned char *routingaddress)
{
    if (titanReadyToStartGame(routingaddress))
    {
        mgStartGameCB();
    }
}

void mgStartGame(char*name,featom*atom)
{
    if (tpGameCreated.numPlayers == 0)
    {
        return;     // if we haven't created a game, don't let us click start.
    }
#if NEED_MIN_TWO_HUMAN_PLAYERS
    if (tpGameCreated.numPlayers < 2)
    {
        char str[200];

        sprintf(str,strGetString(strMinNumPlayersRequired),2);
        mgPrepareMessageBox(str,NULL);
        mgShowScreen(MGS_Message_Box,FALSE);
        return;
    }
#endif

#if NEED_MIN_TWO_HUMAN_PLAYERS
    if (spScenarios[spCurrentSelected].minplayers > (tpGameCreated.numPlayers + tpGameCreated.numComputers))
    {
        char str[200];

        sprintf(str,strGetString(strMinNumPlayersRequired),spScenarios[spCurrentSelected].minplayers);
        mgPrepareMessageBox(str,NULL);
        mgShowScreen(MGS_Message_Box,FALSE);
        return;
    }
#endif

    if (tpGameCreated.userBehindFirewall)
    {
        mgShowScreen(MGS_Choose_Server,TRUE);
    }
    else
    {
        mgReallyStartGame(NULL);
    }
}

void mgGameChatTextEntry(char *name, featom *atom)
{
    chatlist *newchat;
    gameplayerinfo *user;
    char            temp[512];
    sdword          CommandType;

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgGameChatTextEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgGameChatTextEntryBox,UICTE_NoLossOfFocus|UICTE_ChatTextEntry|UICTE_SpecialChars);
        uicTextBufferResize(mgGameChatTextEntryBox,MAX_CHATSTRING_LENGTH-2);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_AcceptText:
            if (strlen(mgGameChatTextEntryBox->textBuffer)>0)
            {
                newchat = (chatlist *)memAlloc(sizeof(chatlist),"GameChat",NonVolatile);

                if ((user=mgParseChatEntry(mgGameChatTextEntryBox->textBuffer, TRUE))!=NULL)
                {
                    if (strcmp(user->name,utyName)==0)
                    {
                        strcpy(newchat->chatstring,mgGameChatTextEntryBox->textBuffer+strlen(user->name)+2);
                        strcpy(newchat->userName, utyName);
                        newchat->baseColor = utyBaseColor;
                        newchat->messagetype   = MG_WHISPEREDCHAT;
                    }
                    else
                    {
                        sendChatMessage(PLAYER_MASK(user->index),mgGameChatTextEntryBox->textBuffer+strlen(user->name)+2,(uword)sigsPlayerIndex);

                        strcpy(newchat->chatstring,mgGameChatTextEntryBox->textBuffer+strlen(user->name)+2);
                        strcpy(newchat->userName, utyName);
                        newchat->baseColor = utyBaseColor;
                        newchat->messagetype   = MG_WHISPEREDCHAT;
                    }
                }
                else if (mgCommandEntered(mgGameChatTextEntryBox->textBuffer, TRUE) != -1)
                {
                    mgCommandExecute(mgGameChatTextEntryBox->textBuffer, temp, TRUE);

                    // public chat message
                    strcpy(newchat->chatstring,temp);
                    strcpy(newchat->userName, "");
                    newchat->messagetype = MG_MESSAGECHAT;
                }
                else
                {
                    strcpy(newchat->chatstring,mgGameChatTextEntryBox->textBuffer);
                    strcpy(newchat->userName, utyName);
                    newchat->baseColor = utyBaseColor;
                    newchat->messagetype   = MG_NORMALCHAT;

                    sendChatMessage(OTHER_PLAYERS_MASK,mgGameChatTextEntryBox->textBuffer,(uword)sigsPlayerIndex);
                    uicTextEntrySet(mgGameChatTextEntryBox,"",0);
                }


                mgAddChatToRoomChat(newchat, mgGameChatWindowWindow, &listofgamechatinfo);

                uicTextEntrySet(mgGameChatTextEntryBox,"",0);
            }
        break;
        case CM_KeyPressed:
        {
            if ((user=mgParseChatEntry(mgGameChatTextEntryBox->textBuffer, TRUE))!=NULL)
            {
                sprintf(temp, "/%s ", user->name);
                if (strlen(mgGameChatTextEntryBox->textBuffer) < strlen(temp))
                    uicTextEntrySet(mgGameChatTextEntryBox, temp, strlen(temp));
            }
            else if ((CommandType = mgCommandComplete(mgGameChatTextEntryBox->textBuffer, temp, TRUE)) != -1)
            {
                if (strlen(mgGameChatTextEntryBox->textBuffer) < strlen(temp))
                    uicTextEntrySet(mgGameChatTextEntryBox, temp, strlen(temp));
            }
        }
        break;
        case CM_SpecialKey:
        {
#ifdef DEBUG_STOMP
            regVerify((regionhandle)&mgGameChatWindowWindow->reg);
#endif
            if (mgGameChatTextEntryBox->key==PAGEUPKEY)
            {
                uicListWindowPageUp(mgGameChatWindowWindow);
                mgGameChatWindowWindow->reg.status |= RSF_DrawThisFrame;
            }
            else if (mgGameChatTextEntryBox->key==PAGEDOWNKEY)
            {
                uicListWindowPageDown(mgGameChatWindowWindow);
                mgGameChatWindowWindow->reg.status |= RSF_DrawThisFrame;
            }
        }
        break;
        case CM_GainFocus:
        break;
    }
}

void mgCurrentGameDraw(featom *atom, regionhandle region)
{
    fonthandle oldfont;
    static char asciigamename[MAX_TITAN_GAME_NAME_LEN];

    oldfont = fontMakeCurrent(mgCurrentGameFont);

    if (wcstombs(asciigamename, tpGameCreated.Name, MAX_TITAN_GAME_NAME_LEN) > 0)
        fontPrint(region->rect.x0,region->rect.y0, mgCurrentGameColor, asciigamename);

    fontMakeCurrent(oldfont);
}


/*=============================================================================
    Callbacks for the create game screen\Options:
=============================================================================*/

/*=============================================================================
    Proccess Callbacks from the Skirmish basic options screen:
=============================================================================*/

regionhandle mgNumCompPlayerReg     = NULL;
regionhandle mgNumHumanPlayerReg    = NULL;

void mgDirtyNumPlayerRegions()
{
    if (mgNumCompPlayerReg!=NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(mgNumCompPlayerReg);
#endif
        mgNumCompPlayerReg->status |= RSF_DrawThisFrame;
    }
    if (mgNumHumanPlayerReg!=NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(mgNumHumanPlayerReg);
#endif
        mgNumHumanPlayerReg->status |= RSF_DrawThisFrame;
    }
}

bool mgInvalidGameName()
{
#ifndef _MACOSX_FIX_ME
    featom  *atomchange;

    if (wcslen(tpGameCreated.Name)<2)
    {
        atomchange = feAtomFindInScreen(feStack[feStackIndex].screen, "MG_GameNameTextEntry");

        if (atomchange == NULL)
        {
            mgShowScreen(MGS_Basic_Options, TRUE);
            atomchange = feAtomFindInScreen(feStack[feStackIndex].screen, "MG_GameNameTextEntry");
        }

        if (atomchange != NULL)
        {
            uicClearCurrent(atomchange->region);
            uicSetCurrent(atomchange->region, FALSE);

        }

        mgPrepareMessageBox(strGetString(strYouMustTypeInGameName),strGetString(strAtLeast2Chars));
        mgShowScreen(MGS_Message_Box,FALSE);

        return TRUE;
    }
#endif

    return FALSE;
}

bool mgInvalidGamePassword()
{
#ifndef _MACOSX_FIX_ME
    if (bitTest(tpGameCreated.flag,MG_PasswordProtected))
    {
        if (wcslen(tpGameCreated.Password) < 2)
        {
            return TRUE;
        }
    }
#endif

    return FALSE;
}

void mgCreateGameNow(char *name, featom *atom)
{
    DirectoryCustomInfoMax buildDirectoryCustomInfo;

    if (LANGame)
    {
        lgCreateGameNow(name, atom);

    }
    else
    {

        if (WaitingForGame)
        {
            if (GameCreator)
            {
                dbgAssert(strlen(spScenarios[spCurrentSelected].fileSpec) <= MAX_MAPNAME_LEN);
                memStrncpy(tpGameCreated.DisplayMapName,spScenarios[spCurrentSelected].title,MAX_MAPNAME_LEN);
                memStrncpy(tpGameCreated.MapName,spScenarios[spCurrentSelected].fileSpec,MAX_MAPNAME_LEN);

                generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
                titanReplaceGameInfo(tpGameCreated.Name,(DirectoryCustomInfo *)&buildDirectoryCustomInfo, FALSE);

                titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));
                mgShowScreen(MGS_Captain_Wait, TRUE);
            }
            else
                mgShowScreen(MGS_Player_Wait, TRUE);
        }
        else
        {
            DirectoryCustomInfoMax buildDirectoryCustomInfo;

            listDeleteAll(&listofgamechatinfo);

            if (mgInvalidGameName())
            {
                return;
            }

#ifndef _MACOSX_FIX_ME
            if (wcslen(GetCurrentChannel()) <= 0)
            {
                mgPrepareMessageBox(strGetString(strMustBeInRoomToCreateGame),strGetString(strMustBeInRoomToCreateGame2));
                mgShowScreen(MGS_Message_Box,FALSE);
                return;
            }
#endif

            if (mgInvalidGamePassword())
            {
                mgPrepareMessageBox(strGetString(strErrorTypingPassword),strGetString(strMustBeAtLeast2Chars));
                mgShowScreen(MGS_Message_Box,FALSE);
                return;
            }

            tpGameCreated.numPlayers = 1;
            tpGameCreated.playerInfo[0].address = myAddress;
            tpGameCreated.playerInfo[0].playerIndex = 0;
            tpGameCreated.playerInfo[0].baseColor = utyBaseColor;
            tpGameCreated.playerInfo[0].stripeColor = utyStripeColor;
            tpGameCreated.playerInfo[0].colorsPicked = cpColorsPicked;
            tpGameCreated.playerInfo[0].race = whichRaceSelected;
            tpGameCreated.playerInfo[0].behindFirewall = titanBehindFirewall();
            strcpy(tpGameCreated.playerInfo[0].PersonalName, utyName);

            dbgAssert(strlen(spScenarios[spCurrentSelected].fileSpec) <= MAX_MAPNAME_LEN);
            memStrncpy(tpGameCreated.DisplayMapName,spScenarios[spCurrentSelected].title,MAX_MAPNAME_LEN);
            memStrncpy(tpGameCreated.MapName,spScenarios[spCurrentSelected].fileSpec,MAX_MAPNAME_LEN);

            generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
            buildDirectoryCustomInfo.captainAddress = myAddress;

            mgGameInterestedIn(tpGameCreated.Name);

            titanCreateGame(tpGameCreated.Name, (DirectoryCustomInfo *)&buildDirectoryCustomInfo);

            donerefreshing=TRUE;

            //WaitingForGame = TRUE;
            //GameCreator    = TRUE;

            // bring up the connecting status screen

            mgShowScreen(MGS_Connecting,FALSE);

            mgConnectingScreenGoto = MGS_Captain_Wait;

            mgDisplayMessage(strGetString(strCreatingGame));

            //mgShowScreen(MGS_Captain_Wait,TRUE);

            //mgUpdateGameInfo();
        }
    }
}

void mgGameNameTextEntry(char *name, featom *atom)
{
    static char asciigamename[MAX_GAMENAME_LENGTH];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgGameNameTextEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(mgGameNameTextEntryBox,0);
        uicTextBufferResize(mgGameNameTextEntryBox,MAX_GAMENAME_LENGTH);
        wcstombs(asciigamename, tpGameCreated.Name, MAX_GAMENAME_LENGTH);
        uicTextEntrySet(mgGameNameTextEntryBox, asciigamename, strlen(asciigamename)+1);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            mbstowcs(tpGameCreated.Name,mgGameNameTextEntryBox->textBuffer,strlen(mgGameNameTextEntryBox->textBuffer)+1);
        break;
        case CM_GainFocus :
        break;
    }
}

void mgChooseScenario(char *name, featom *atom)
{
    spScenarioPick(NULL);
}

void mgAddCPUOpp(void)
{
        if (tpGameCreated.numPlayers + tpGameCreated.numComputers < spScenarios[spCurrentSelected].maxplayers)
        {
            tpGameCreated.numComputers++;
            mgDirtyNumPlayerRegions();

        }
    }

void mgRemoveCPUOpp(void)
{
    sdword minComp;
    if (LoggedIn | LANGame)
        {
        minComp = 0;
    }
    else
    {
        // CPU skirmish
#ifdef fpoiker
        minComp = 0;
#else
        minComp = 1;
#endif
}

    if (tpGameCreated.numComputers > minComp)
        {
        tpGameCreated.numComputers--;
            mgDirtyNumPlayerRegions();
    }
}

bool mgAccelerator(void)
{
    if (taskTimeElapsed >= mgAccTime)
    {
        mgAccTime = taskTimeElapsed + MG_WAIT_ACC;
        return(TRUE);
    }

    return(FALSE);
/*    mgAccCount--;
    if (!mgAccCount)
    {
        mgAccCount = mgAccRate;
        mgAccRate-= MG_RATE_DECREASE;
        if (mgAccRate < MG_MIN_ACC_RATE)
            mgAccRate = MG_MIN_ACC_RATE;
        return TRUE;
    }
    return FALSE;*/
}


#if 0
void mgAcceleratorRelease(void)
{
/*    mgAccRate = MG_START_ACC_RATE;*/
    mgAccCount = 1;
}
#endif

sdword mgSelectOpponentType(regionhandle region, sdword yClicked)
{

    if ((yClicked-region->rect.y0 > mg_ArrowClickStart) &&
      (yClicked-region->rect.y0 < mg_ArrowClickStart + mgArrowClickHeight))
    {
        return 0;
    }

    if ((yClicked-region->rect.y0 > mg_ArrowClickStart + mgArrowClickSpacing) &&
      (yClicked-region->rect.y0 < mg_ArrowClickStart + mgArrowClickHeight + mgArrowClickSpacing))
    {
        return 1;
    }

    return (-1);
}

sdword mgRemoveCPUProcess(regionhandle region, sdword ID, udword event, udword data)
{
#ifdef DEBUG_STOMP
    regVerify(region);
#endif
    if (event == RPE_ReleaseLeft)
    {
        mgCPULeftArrowActive = FALSE;
/*        mgCPUArrowIndex = -1;
//        mgAcceleratorRelease();*/
        region->status |= RSF_DrawThisFrame;
        return (0);
    }
    else if (event == RPE_PressLeft)
    {
        mgAccTime = taskTimeElapsed - MG_WAIT_ACC;
        return(0);
    }
    else if (event == RPE_HoldLeft)
    {
        if (mgAccelerator())
            mgRemoveCPUOpp();
        mgCPULeftArrowActive = TRUE;

#if 0
        if ( TRUE ) // max  )
        {
            mgCPULeftArrowActive = TRUE;
            if (mgAccelerator())
                mgRemoveCPUOpp();
        }
        else
        {
            /*    Put sound effect HERE */
        }
#endif
    }
    region->status |= RSF_DrawThisFrame;
    return(0);
}

sdword mgAddCPUProcess(regionhandle region, sdword ID, udword event, udword data)
{
#ifdef DEBUG_STOMP
    regVerify(region);
#endif
    if (event == RPE_ReleaseLeft)
    {
        mgCPURightArrowActive = FALSE;
//        mgCPUArrowIndex = -1;
//        mgAcceleratorRelease();
        region->status |= RSF_DrawThisFrame;
        return (0);
    }
    else if (event == RPE_PressLeft)
    {
        mgAccTime = taskTimeElapsed - MG_WAIT_ACC;
        return(0);
    }
    else if (event == RPE_HoldLeft)
    {
        if (mgAccelerator())
            mgAddCPUOpp();
        mgCPURightArrowActive = TRUE;
#if 0
        if ( TRUE /* max */ )
        {
            mgCPURightArrowActive = TRUE;
            if (mgAccelerator())
            {
                mgAddCPUOpp();
                dbgMessage(".");
            }
        }
        else
        {
            //    Put sound effect HERE
        }
#endif
    }
    region->status |= RSF_DrawThisFrame;
    return(0);
}



/*-----------------------------------------------------------------------------
    Name        : mgDrawArrow
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mgDrawArrow(regionhandle region, bool leftArrow, bool human)
{
    sdword y;
    rectangle *rect = &region->rect;
    triangle tri;
    sdword width = rect->x1 - rect->x0 - 4;
    bool flag;

    y = rect->y0 + mg_ArrowMarginTop;

    if(leftArrow)
    {
        tri.x0 = rect->x1 - 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }
    else
    {
        tri.x0 = rect->x0 + 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }


    if(leftArrow)
    {
        tri.y0 = y;
        tri.y1 = y + (width / 2);
        tri.y2 = y + width;
    }
    else
    {
        tri.y0 = y;
        tri.y1 = y + width;
        tri.y2 = y + (width / 2);
    }


    if (human)
    {
        flag = ((leftArrow && mgHumanLeftArrowActive) || (!leftArrow && mgHumanRightArrowActive));
        if (flag)
        {
            mgHumanLeftArrowActive  = FALSE;
            mgHumanRightArrowActive = FALSE;
            mgHumanArrowIndex = -1;
            primTriSolid2(&tri, colRGB(200, 0, 0));

        }
        else if (bitTest(region->status, RSF_MouseInside) )
        {
            primTriSolid2(&tri, colRGB(100, 100, 100));
        }
        else
        {
            primTriSolid2(&tri, colRGB(0, 0, 0));
        }



    }
    else //cpu
    {
        flag = ((leftArrow && mgCPULeftArrowActive) || (!leftArrow && mgCPURightArrowActive));
        if( flag )
        {
            mgCPULeftArrowActive  = FALSE;
            mgCPURightArrowActive = FALSE;
            mgCPUArrowIndex = -1;
            primTriSolid2(&tri, colRGB(200, 0, 0));
        }
        else if (bitTest(region->status, RSF_MouseInside) )
        {
            primTriSolid2(&tri, colRGB(100, 100, 100));
        }
        else
        {
            primTriSolid2(&tri, colRGB(0, 0, 0));
        }

    }





    primTriOutline2(&tri, 1, colRGB(200, 200, 0));



}


/*-----------------------------------------------------------------------------
    Name        : mgLeftArrowDraw
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

/*
void mgLeftArrowDraw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_HoldLeft
            | RPE_Enter | RPE_Exit;
                               //receive mouse presses from now on
        regFunctionSet(region, mgLeftArrowProcess);         //set new region handler function
    }
    //mgDrawArrow(region, TRUE);
}
  */
/*-----------------------------------------------------------------------------
    Name        : mgRightArrowDraw
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
void mgRightArrowDraw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_HoldLeft
            | RPE_Enter | RPE_Exit ;
                                //receive mouse presses from now on
        regFunctionSet(region, mgRightArrowProcess);        //set new region handler function
    }
    //mgDrawArrow(region, FALSE);
}

   */


void mgAddCPUOpponent(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_HoldLeft
            | RPE_Enter | RPE_Exit ;
                                //receive mouse presses from now on
        if ((atom->flags & FAF_Disabled) == 0)
            regFunctionSet(region, (regionfunction) mgAddCPUProcess);        //set new region handler function
    }
    mgDrawArrow(region, FALSE, FALSE);
}



void mgRemoveCPUOpponent(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_HoldLeft
            | RPE_Enter | RPE_Exit ;
                                //receive mouse presses from now on
        if ((atom->flags & FAF_Disabled) == 0)
            regFunctionSet(region, (regionfunction) mgRemoveCPUProcess);        //set new region handler function
    }
    mgDrawArrow(region, TRUE, FALSE);
}

void mgPickGameType(char *name, featom *atom)
{
    mgSetGameType("Give Babies A Chance");
}

void mgStartingFleet(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, tpGameCreated.startingFleet);
    }
    else
    {
        tpGameCreated.startingFleet = (ubyte)(size_t)atom->pData;
    }
    mgGameTypesOtherButtonPressed();
}

void mgStartShip(char *name, featom *atom)
{
//#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
//    bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, bitTest(tpGameCreated.flag,MG_CarrierIsMothership) ? 1 : 0);
    }
    else
    {
        if (atom->pData)
            bitSet(tpGameCreated.flag,MG_CarrierIsMothership);
        else
            bitClear(tpGameCreated.flag,MG_CarrierIsMothership);
    }
    mgGameTypesOtherButtonPressed();
//#endif
}



void mgGameType(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, gameNum);
    }
    else
    {
        gameNum = (ubyte)(size_t)atom->pData;
        mgSetGameTypeByNum(gameNum);
        //regRecursiveSetReallyDirty(feStack[feStackIndex].baseRegion);
        feAllCallOnCreate(feStack[feStackIndex].screen);

    }
#endif
}


void mgNumberOfComputers(featom *atom, regionhandle region)
{
    fonthandle oldfont;

    mgNumCompPlayerReg = region;

    primRectSolid2(&region->rect,0);

    oldfont = fontMakeCurrent(mgOptionsFont);

    fontPrintf(region->rect.x0,region->rect.y0, mgNumberOfComputersColor, "%d", tpGameCreated.numComputers);

    fontMakeCurrent(oldfont);
}


/*=============================================================================
    Callbacks for the Advanced Game Options Screen:
=============================================================================*/

void mgLastMotherShip(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_LastMotherShip));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_LastMotherShip);
        else
            bitClear(tpGameCreated.flag,MG_LastMotherShip);

        if ((tpGameCreated.flag & (MG_CaptureCapitalShip|MG_LastMotherShip)) == 0)
        {
            feToggleButtonSet("MG_CaptureCapitalShip",TRUE);
            tpGameCreated.flag |= MG_CaptureCapitalShip;
        }
    }
    mgGameTypesOtherButtonPressed();
}

void mgCaptureCapitalShip(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_CaptureCapitalShip));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_CaptureCapitalShip);
        else
            bitClear(tpGameCreated.flag,MG_CaptureCapitalShip);

        if ((tpGameCreated.flag & (MG_CaptureCapitalShip|MG_LastMotherShip)) == 0)
        {
            feToggleButtonSet("MG_LastMotherShip",TRUE);
            tpGameCreated.flag |= MG_LastMotherShip;
    }
    }
    mgGameTypesOtherButtonPressed();
#endif
}

void mgHyperspace(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag, MG_Hyperspace));
    }
    else
    {
        if (FECHECKED(atom))
        {
            bitSet(tpGameCreated.flag, MG_Hyperspace);
        }
        else
        {
            bitClear(tpGameCreated.flag, MG_Hyperspace);
}
    }
    mgGameTypesOtherButtonPressed();
}

void mgAlliedVictory(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_AlliedVictory));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_AlliedVictory);
        else
            bitClear(tpGameCreated.flag,MG_AlliedVictory);
    }
    mgGameTypesOtherButtonPressed();
}

void mgResearchEnabled(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_ResearchEnabled));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_ResearchEnabled);
        else
            bitClear(tpGameCreated.flag,MG_ResearchEnabled);
    }
    mgGameTypesOtherButtonPressed();
#endif
}
#if 0
void mgBountiesEnabled(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_BountiesEnabled));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_BountiesEnabled);
        else
            bitClear(tpGameCreated.flag,MG_BountiesEnabled);
    }
    mgGameTypesOtherButtonPressed();
#endif
}
#endif

void mgBountySize(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, tpGameCreated.bountySize);
    }
    else
    {
        tpGameCreated.bountySize = (ubyte)(size_t)atom->pData;
    }
    mgGameTypesOtherButtonPressed();
#endif
}

void mgPasswordProtected(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_PasswordProtected));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_PasswordProtected);
        else
            bitClear(tpGameCreated.flag,MG_PasswordProtected);
    }
}

void mgGamePassword(char *name, featom *atom)
{
    static char asciipassword[MAX_PASSWORD_LENGTH];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgGamePasswordEntryBox = (textentryhandle)atom->pData;
        wcstombs(asciipassword, tpGameCreated.Password, MAX_PASSWORD_LENGTH);
        uicTextEntrySet(mgGamePasswordEntryBox,asciipassword,strlen(asciipassword)+1);
        uicTextEntryInit(mgGamePasswordEntryBox,UICTE_PasswordEntry);
        uicTextBufferResize(mgGamePasswordEntryBox,MAX_PASSWORD_LENGTH-2);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
        break;
        case CM_GainFocus :
        break;
    }
}

void mgGamePasswordConfirm(char *name, featom *atom)
{
    static char asciipassword[MAX_PASSWORD_LENGTH];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgGamePasswordConfirmEntryBox = (textentryhandle)atom->pData;
        wcstombs(asciipassword, tpGameCreated.Password, MAX_PASSWORD_LENGTH);
        uicTextEntrySet(mgGamePasswordConfirmEntryBox,asciipassword,strlen(asciipassword)+1);
        uicTextBufferResize(mgGamePasswordConfirmEntryBox,MAX_PASSWORD_LENGTH-2);
        uicTextEntryInit(mgGamePasswordConfirmEntryBox,UICTE_PasswordEntry);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            if (strcmp(mgGamePasswordEntryBox->textBuffer,mgGamePasswordConfirmEntryBox->textBuffer)==0)
            {
                mbstowcs(tpGameCreated.Password,mgGamePasswordEntryBox->textBuffer,strlen(mgGamePasswordEntryBox->textBuffer)+1);
            }
        break;
        case CM_GainFocus :
        break;
    }
}

void mgUnitCapsEnabled(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_UnitCapsEnabled));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_UnitCapsEnabled);
        else
            bitClear(tpGameCreated.flag,MG_UnitCapsEnabled);
    }
    mgGameTypesOtherButtonPressed();
#endif
}

void mgFuelBurnEnabled(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_FuelBurnEnabled));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_FuelBurnEnabled);
        else
            bitClear(tpGameCreated.flag,MG_FuelBurnEnabled);
    }
    mgGameTypesOtherButtonPressed();
#endif
}

void mgCratesEnabled(char *name, featom *atom)
{
#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_CratesEnabled));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_CratesEnabled);
        else
            bitClear(tpGameCreated.flag,MG_CratesEnabled);
    }
    mgGameTypesOtherButtonPressed();
#endif
}

/*=============================================================================
    Callbacks for the Resourceing Optinos Game Screen:
=============================================================================*/

void mgHarvestingEnabled(char *name, featom *atom)
{
//#if defined(CGW) || defined(Downloadable) || defined(OEM)
//    //disable this function in demos
//    bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else

    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, !bitTest(tpGameCreated.flag,MG_HarvestinEnabled));
    }
    else
    {
        if (FECHECKED(atom))
            bitClear(tpGameCreated.flag,MG_HarvestinEnabled);
        else
            bitSet(tpGameCreated.flag,MG_HarvestinEnabled);
    }
    mgGameTypesOtherButtonPressed();
//#endif
}

void mgStartingResources(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, tpGameCreated.startingResources);
    }
    else
    {
        tpGameCreated.startingResources = (ubyte)(size_t)atom->pData;
    }
    mgGameTypesOtherButtonPressed();
}

void mgResourceInjections(char *name, featom *atom)
{
//#if defined(CGW) || defined(Downloadable) || defined(OEM)
//    //disable this function in demos
//    bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_ResourceInjections));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_ResourceInjections);
        else
            bitClear(tpGameCreated.flag,MG_ResourceInjections);
    }
    mgGameTypesOtherButtonPressed();
//#endif
}

void mgResourceInjectionInterval(char *name, featom *atom)
{
//#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
//    bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else
    char    temp[20];
    udword  timemins;

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgResourceInjectionIntervalEntryBox = (textentryhandle)atom->pData;
        timemins = (udword)((tpGameCreated.resourceInjectionInterval)/(60*UNIVERSE_UPDATE_RATE));
        sprintf(temp,"%d",timemins);
        uicTextEntrySet(mgResourceInjectionIntervalEntryBox,temp,strlen(temp)+1);
        uicTextBufferResize(mgResourceInjectionIntervalEntryBox,MAX_NUM_LENGTH-2);
        uicTextEntryInit(mgResourceInjectionIntervalEntryBox,UICTE_NumberEntry);
        mgGameTypesOtherButtonPressed();
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            sscanf(mgResourceInjectionIntervalEntryBox->textBuffer,"%d",&timemins);
            // change the units of minutes into Univupdate slices
            tpGameCreated.resourceInjectionInterval = timemins*60*UNIVERSE_UPDATE_RATE;
            if (uicTextEntryMessage(atom)==CM_AcceptText) feToggleButtonSet("MG_ResourceInjections", TRUE);
        break;
        case CM_GainFocus :
        break;
    }
    mgGameTypesOtherButtonPressed();
//#endif
}

void mgResourceInjectionAmount(char *name, featom *atom)
{
//#if defined(CGW) || defined(Downloadable) || defined(OEM)
//    //disable this function in demos
 ///   bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else
    char    temp[20];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgResourceInjectionAmountEntryBox = (textentryhandle)atom->pData;
        sprintf(temp,"%ld",tpGameCreated.resourceInjectionsAmount);
        uicTextEntrySet(mgResourceInjectionAmountEntryBox,temp,strlen(temp)+1);
        uicTextBufferResize(mgResourceInjectionAmountEntryBox,MAX_NUM_LENGTH-6);
        uicTextEntryInit(mgResourceInjectionAmountEntryBox,UICTE_NumberEntry);
        mgGameTypesOtherButtonPressed();
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            sscanf(mgResourceInjectionAmountEntryBox->textBuffer,"%ld",&tpGameCreated.resourceInjectionsAmount);
            if (uicTextEntryMessage(atom)==CM_AcceptText) feToggleButtonSet("MG_ResourceInjections", TRUE);
        break;
        case CM_GainFocus :
        break;
    }
    mgGameTypesOtherButtonPressed();
//#endif
}

void mgResourceLumpSum(char *name, featom *atom)
{
//#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
//    bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, bitTest(tpGameCreated.flag,MG_ResourceLumpSum));
    }
    else
    {
        if (FECHECKED(atom))
            bitSet(tpGameCreated.flag,MG_ResourceLumpSum);
        else
            bitClear(tpGameCreated.flag,MG_ResourceLumpSum);
    }
    mgGameTypesOtherButtonPressed();
//#endif
}

void mgResourceLumpSumInterval(char *name, featom *atom)
{
//#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
//    bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else
    char    temp[20];
    udword  timemins;

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgResourceLumpSumIntervalEntryBox = (textentryhandle)atom->pData;
        timemins = (udword)(tpGameCreated.resourceLumpSumTime/(60*UNIVERSE_UPDATE_RATE));
        sprintf(temp,"%d",timemins);
        uicTextEntrySet(mgResourceLumpSumIntervalEntryBox,temp,strlen(temp)+1);
        uicTextBufferResize(mgResourceLumpSumIntervalEntryBox,MAX_NUM_LENGTH-2);
        uicTextEntryInit(mgResourceLumpSumIntervalEntryBox,UICTE_NumberEntry);
        mgGameTypesOtherButtonPressed();
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            sscanf(mgResourceLumpSumIntervalEntryBox->textBuffer,"%d",&timemins);
            tpGameCreated.resourceLumpSumTime = timemins*60*UNIVERSE_UPDATE_RATE;
            if (uicTextEntryMessage(atom)==CM_AcceptText) feToggleButtonSet("MG_ResourceLumpSum", TRUE);
        break;
        case CM_GainFocus :
        break;
    }
    mgGameTypesOtherButtonPressed();
//#endif
}

void mgResourceLumpSumAmount(char *name, featom *atom)
{
//#if defined(CGW) || defined(Downloadable) || defined(OEM)
    //disable this function in demos
//    bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else
    char    temp[20];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgResourceLumpSumAmountEntryBox = (textentryhandle)atom->pData;
        sprintf(temp,"%ld",tpGameCreated.resourceLumpSumAmount);
        uicTextEntrySet(mgResourceLumpSumAmountEntryBox,temp,strlen(temp)+1);
        uicTextBufferResize(mgResourceLumpSumAmountEntryBox,MAX_NUM_LENGTH-6);
        uicTextEntryInit(mgResourceLumpSumAmountEntryBox,UICTE_NumberEntry);
        mgGameTypesOtherButtonPressed();
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            sscanf(mgResourceLumpSumAmountEntryBox->textBuffer,"%ld",&tpGameCreated.resourceLumpSumAmount);
            if (uicTextEntryMessage(atom)==CM_AcceptText) feToggleButtonSet("MG_ResourceLumpSum", TRUE);
        break;
        case CM_GainFocus :
        break;
    }
    mgGameTypesOtherButtonPressed();
//#endif
}

void mgBackFromOptions(char *name, featom *atom)
{
    if (LANGame)
    {
        lgBackFromOptions(name, atom);
    }
    else
    {
        if (LoggedIn)
        {
            if (WaitingForGame)
            {
                if (GameCreator)
                {
                    tpGameCreated = BackupGameCreated;
                    spCurrentSelected = spCurrentSelectedBack;
                    mgShowScreen(MGS_Captain_Wait, TRUE);

                }
                else
                {
                    mgShowScreen(MGS_Player_Wait, TRUE);
                }
            }
            else
                mgShowScreen(MGS_Available_Games,TRUE);
        }
        else
        {
            mgShowScreen(MGS_Connection_Method,TRUE);
        }
    }
}

void mgBasicOptions(char *name,featom *atom)
{
    if (LANGame)
    {
        lgBasicOptions(name,atom);
    }
    else
    {
        if (LoggedIn)
        {
            if (currentScreen != MGS_Basic_Options)
            {
                if (WaitingForGame)
                {
                    if (GameCreator)
                        mgShowScreen(MGS_Basic_Options_Change, TRUE);
                    else
                        mgShowScreen(MGS_Basic_Options_View, TRUE);
                }
                else
                    mgShowScreen(MGS_Basic_Options,TRUE);
            }
        }
        else
        {
            if (currentScreen != MGS_Skirmish_Basic)
            {
                mgShowScreen(MGS_Skirmish_Basic,TRUE);
            }
        }

    }
}

void mgAdvancedOptions(char *name,featom *atom)
{
    if (LoggedIn || LANGame)
    {
        if (currentScreen != MGS_Advanced_Options)
        {
            if (WaitingForGame)
            {
                if (GameCreator)
                    mgShowScreen(MGS_Advanced_Options_Change, TRUE);
                else
                    mgShowScreen(MGS_Advanced_Options_View, TRUE);
            }
            else
                mgShowScreen(MGS_Advanced_Options,TRUE);
        }
    }
    else
    {
        if (currentScreen != MGS_Skirmish_Advanced)
        {
            mgShowScreen(MGS_Skirmish_Advanced,TRUE);
        }
    }
}

void mgResourceOptions(char *name,featom *atom)
{
    if (LoggedIn || LANGame)
    {
        if (currentScreen != MGS_Resource_Options)
        {
            if (WaitingForGame)
            {
                if (GameCreator)
                    mgShowScreen(MGS_Resource_Options_Change, TRUE);
                else
                    mgShowScreen(MGS_Resource_Options_View, TRUE);
            }
            else
                mgShowScreen(MGS_Resource_Options,TRUE);
        }
    }
    else
    {
        if (currentScreen != MGS_Skirmish_Resource)
        {
            mgShowScreen(MGS_Skirmish_Resource,TRUE);
        }
    }
}


/*=============================================================================
    Callbacks for the Player options screen:
=============================================================================*/

void mgSetColorsNow(char *name, featom *atom)
{
    cpResetRegions();

    cpAcceptColors();

    if (WaitingForGame)
    {
        if (GameCreator)
        {
            titanUpdatePlayer(TRUE);
        }
        else
        {
            titanUpdatePlayer(FALSE);
        }
    }

    dbgAssert(lastScreen != -1);

    mgShowScreen(lastScreen, TRUE);
}

void mgBackFromPlayerOptions(char *name, featom *atom)
{
    dbgAssert(lastScreen != -1);

    cpResetRegions();

    whichRaceSelected   = RaceSave;
    utyBaseColor    = BaseColorSave;
    utyStripeColor  = StripeColorSave;

    mgShowScreen(lastScreen, TRUE);
}

/*=============================================================================
    Callbacks for the Connecting Status Screen:
=============================================================================*/

void mgConnectingCancel(char *name, featom *atom)
{
    mgQueryType = -1;

    cResetFSM();

    titanConnectingCancelHit();

    mgShowScreen(mgConnectingScreenGoto,TRUE);
}

void mgConnectingStatusItemDraw(rectangle *rect, listitemhandle data)
{
    sdword      x, y;
    fonthandle  oldfont;
    statusline *statinfo = (statusline *)data->data;

    oldfont = fontMakeCurrent(mgConnectingFont);

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing/2;

    fontPrintf(x,y,mgConnectingStatusColor,"%s",statinfo->message);

    fontMakeCurrent(oldfont);
}

void mgConnectingStatusInit(char *name, featom *atom)
{
    fonthandle      oldfont;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(mgConnectingFont);

        mgConnectingStatusWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(mgConnectingStatusWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          mgConnectingStatusItemDraw,       // item draw funtcion
                          fontHeight(" "),                  // item height
                          UICLW_AutoScroll);

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        mgConnectingStatusWindow = NULL;
        return;
    }
}

/*=============================================================================
    Callbacks for the game entry screen.:
=============================================================================*/

void mgGamePasswordEntry(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        mgGamePasswordEntryEntryBox = (textentryhandle)atom->pData;
        uicTextBufferResize(mgGamePasswordEntryEntryBox,MAX_PASSWORD_LENGTH-2);
        uicTextEntryInit(mgGamePasswordEntryEntryBox,UICTE_PasswordEntry);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        break;
        case CM_AcceptText :
            if (!LANGame)
            {
                if (currentScreen == MGS_Game_Password)
                {
                    mgGoPassword(NULL,NULL);
                }
                else if (currentScreen == MGS_Room_Password)
                {
                    mgGoRoomPassword(NULL, NULL);
                }
            }
            else
            {
                lgGoPassword(NULL, NULL);
            }
        break;
        case CM_GainFocus :
        break;
    }
}

void mgBackFromPassword(char *name, featom *atom)
{
    mgShowScreen(MGS_Available_Games,TRUE);
}

void mgGoPassword(char *name, featom *atom)
{
#ifndef _MACOSX_FIX_ME
    static wchar_t widepasswordentryboxtext[MAX_PASSWORD_LENGTH];

    if (joingame!=NULL)
    {
        mbstowcs(widepasswordentryboxtext,mgGamePasswordEntryEntryBox->textBuffer,strlen(mgGamePasswordEntryEntryBox->textBuffer)+1);
        if (wcscmp(widepasswordentryboxtext,joingame->directoryCustomInfo.stringdata)==0)
        {
            mgRequestJoinGame(joingame);
        }
        else
        {
            mgPrepareMessageBox(strGetString(strIncorrectPassword),NULL);
            mgShowScreen(MGS_Message_Box,FALSE);
        }
    }
#endif
}

void mgBackFromRoomPassword(char *name, featom *atom)
{
    mgShowScreen(MGS_Available_Channels,TRUE);
}

void mgGoRoomPassword(char *name, featom *atom)
{
#ifndef _MACOSX_FIX_ME
    if (wcslen(joinchannelname) >= 2)
    {
        mbstowcs(ChannelPassword,mgGamePasswordEntryEntryBox->textBuffer,strlen(mgGamePasswordEntryEntryBox->textBuffer)+1);
        mgJoinChannelNow(joinchannelname,joinchanneldescription);
    }
#endif
}

/*=============================================================================
    callbacks from the Quit WON question box.:
=============================================================================*/

void mgYesQuitWON(char *name, featom *atom)
{
    bool waitforshutdown = FALSE;
    LoggedIn=FALSE;

    if (WaitingForGame)
    {
        if (GameCreator)
        {
            titanGameDisolved(TRUE);
            waitforshutdown = TRUE;
        }
        else
        {
            titanLeaveGame(TRUE);
            waitforshutdown = TRUE;
        }
    }

    if (waitforshutdown)
    {
        titanWaitShutdown();
    }
    else
    {
        titanShutdown();
    }

    mgShowScreen(MGS_Connection_Method, TRUE);
}

void mgDontQuitWON(char *name, featom *atom)
{
    mgShowScreen(-1, MG_JustOneDisappear);
}

/*=============================================================================
    Callbacks from the message box screen.:
=============================================================================*/

void mgMessageOk(char *name, featom *atom)
{
    mgShowScreen(-1, MG_JustOneDisappear);
}


void mgDrawMessageBox(featom *atom, regionhandle region)
{
    sdword      x, y;
    fonthandle  oldfont;

    oldfont = fontMakeCurrent(mgMessageBoxFont);

    x = region->rect.x0 + ((region->rect.x1-region->rect.x0)-fontWidth(messageBoxString))/2;
    y = region->rect.y0+MG_VertSpacing/2;

    fontPrintf(x,y,mgMessageBoxColor,"%s",messageBoxString);

    if (messageBoxString2[0] != 0)
    {
        x = region->rect.x0 + ((region->rect.x1-region->rect.x0)-fontWidth(messageBoxString2))/2;
        y += MG_VertSpacing << 1;

        fontPrintf(x,y,mgMessageBoxColor,"%s",messageBoxString2);
    }

    fontMakeCurrent(oldfont);
}

void mgPrepareMessageBox(char *string1,char *string2)
{
    if (string1 != NULL)
    {
        strcpy(messageBoxString,string1);
    }
    else
    {
        messageBoxString[0] = 0;
    }

    if (string2 != NULL)
    {
        strcpy(messageBoxString2,string2);
    }
    else
    {
        messageBoxString2[0] = 0;
    }
}

void GeneralMessageBox(char *string1,char *string2)     // designed for external use
{
    mgPrepareMessageBox(string1,string2);
    feScreenStart(ghMainRegion, "Message_Box");
}

/*=============================================================================
    Proccess Callbacks from titan stuff.:
=============================================================================*/

void mgProcessNewUserInRoom(mgqueueuserlist *user)
{
    // see if userhere is already in the list

    userlist *userinfo;
    Node     *walk;

    walk = listofusersinfo.head;
    while (walk != NULL)
    {
        userinfo = listGetStructOfNode(walk);

        if (strcmp(userinfo->userName,user->user.userName) == 0)
        {
            return;
        }

        walk = walk->next;
    }

    userinfo = memAlloc(sizeof(userlist), "ListofUsers",NonVolatile);

    strcpy(userinfo->userName, user->user.userName);
    userinfo->userID = user->user.userID;

    listAddNode(&listofusersinfo, &userinfo->link, (void *)userinfo);

    if (mgUserNameWindowWindow!=NULL)
    {
        userinfo->item = uicListAddItem(mgUserNameWindowWindow, (void *)userinfo, 0,UICLW_AddToTail);
    }
}

void mgProcessUserLeftRoom(mgqueueuserlist *user)
{
    userlist *userinfo;
    Node     *walk;

    walk = listofusersinfo.head;
    while (walk != NULL)
    {
        userinfo = listGetStructOfNode(walk);

        if (userinfo->userID==user->user.userID)
        {
            if (mgUserNameWindowWindow!=NULL)
            {
                uicListRemoveItem(mgUserNameWindowWindow, userinfo->item);
            }
            listDeleteNode(&userinfo->link);
            break;
        }

        walk = walk->next;
    }
}

void mgProcessNewRoomChat(mgqueuechatlist *chat)
{
    userlist *userinfo;
    chatlist *chatinfo;
    Node     *walk;

    walk = listofusersinfo.head;
    while (walk != NULL)
    {
        userinfo = listGetStructOfNode(walk);

        if ((userinfo->userID==chat->chat.index) &&
            (!mgIsUserInList(&listofIgnoreUsers,userinfo->userName) ))
        {
            chatinfo = memAlloc(sizeof(chatlist), "ListofChat",NonVolatile);

            strcpy(chatinfo->chatstring, chat->chat.chatstring);
            strcpy(chatinfo->userName, userinfo->userName);
            chatinfo->messagetype = chat->chat.messagetype;

            mgAddChatToRoomChat(chatinfo, mgChatHistoryWindow, &listofchatinfo);

            break;
        }

        walk = walk->next;
    }
}

void mgProcessPingInfo(mgqueuenewping *ping)
{
    Node       *walk;
    gamelist   *gameinfo;
    serverlist *serverinfo;
    bool foundgameping = FALSE;
    Address addr;

    walk = listofgames.head;

    dbgAssert(!LANGame);

    while (walk != NULL)
    {
        gameinfo = (gamelist *)listGetStructOfNode(walk);

        if (GetIPFromInternetAddress(gameinfo->game.directoryCustomInfo.pingAddress) == ping->ping.IP)
        {
            gameinfo->pingtime = ping->ping.pingtime;
            foundgameping = TRUE;
        }

        walk = walk->next;
    }

    walk = listofservers.head;

    while (walk != NULL)
    {
        serverinfo = (serverlist *)listGetStructOfNode(walk);

        ConvertUChar6ToAddress(serverinfo->addressstore,&addr);
        if (GetIPFromInternetAddress(addr) == ping->ping.IP)
        {
            serverinfo->ping = ping->ping.pingtime;
            refreshServersScreenRequest = TRUE;
        }

        walk = walk->next;
    }

    if ((foundgameping) && (mgListOfGamesWindow!=NULL))
    {
#ifdef DEBUG_STOMP
        regVerify(&mgListOfGamesWindow->reg);
#endif
        mgListOfGamesWindow->reg.status |= RSF_DrawThisFrame;
    }
}

void mgProcessGameListGameAdded(mgqueuegamelistgame *added)
{
#ifndef _MACOSX_FIX_ME
    Node       *walk;
    gamelist   *gameinfo;

    // make sure we don't already have it in list
    walk = listofgames.head;

    while (walk != NULL)
    {
        gameinfo = (gamelist *)listGetStructOfNode(walk);

        if (wcscmp(gameinfo->game.Name,added->game.Name) == 0)
        {
            titanDebug("Warning: Game %s already present!\n",added->game.Name);
            return;
        }

        walk = walk->next;
    }

    // we don't have it in list, so let's add it:

    gameinfo = (gamelist *)memAlloc(sizeof(gamelist),"ListofGames",NonVolatile);

    gameinfo->game = added->game;
    gameinfo->pingtime = -1;

    listAddNode(&listofgames,&gameinfo->link, gameinfo);

    if (mgListOfGamesWindow!=NULL)
    {
        if (gameinfo->game.directoryCustomInfo.flag & GAME_IN_PROGRESS)
            gameinfo->item = uicListAddItem(mgListOfGamesWindow, (ubyte *)gameinfo, 0, UICLW_AddToTail);
        else
            gameinfo->item = uicListAddItem(mgListOfGamesWindow, (ubyte *)gameinfo, UICLI_CanSelect, UICLW_AddToHead);
    }
    else
    {
        gameinfo->item = NULL;
    }
#endif
}

void mgProcessGameListGameRemoved(mgqueuegamelistgame *removed)
{
#ifndef _MACOSX_FIX_ME
    Node       *walk;
    gamelist   *gameinfo;

    // make sure we have it
    walk = listofgames.head;

    while (walk != NULL)
    {
        gameinfo = (gamelist *)listGetStructOfNode(walk);

        if (wcscmp(gameinfo->game.Name,removed->game.Name) == 0)
        {
            goto foundit;
        }

        walk = walk->next;
    }

    titanDebug("Warning: Told to remove game %s I can't find!\n",removed->game.Name);
    return;

foundit:

    // walk and gameinfo should be set with the game to remove now

    if (mgListOfGamesWindow!=NULL)
    {
        if (gameinfo->item)
        {
            uicListRemoveItem(mgListOfGamesWindow, gameinfo->item);
        }
    }

    listDeleteNode(walk);
#endif
}

void mgProcessGameListGameChanged(mgqueuegamelistgame *changed)
{
#ifndef _MACOSX_FIX_ME
    Node       *walk;
    gamelist   *gameinfo;

    // make sure we have it
    walk = listofgames.head;

    while (walk != NULL)
    {
        gameinfo = (gamelist *)listGetStructOfNode(walk);

        if (wcscmp(gameinfo->game.Name,changed->game.Name) == 0)
        {
            goto foundit;
        }

        walk = walk->next;
    }

    titanDebug("Warning: Told to change game %s I can't find!\n",changed->game.Name);
    return;

foundit:
    gameinfo->game = changed->game;

    if (mgListOfGamesWindow!=NULL)
    {
        if (gameinfo->game.directoryCustomInfo.flag & GAME_IN_PROGRESS)
            bitClear(gameinfo->item->flags,UICLI_CanSelect);
        else
            bitSet(gameinfo->item->flags,UICLI_CanSelect);
#ifdef DEBUG_STOMP
        regVerify(&mgListOfGamesWindow->reg);
#endif
        mgListOfGamesWindow->reg.status |= RSF_DrawThisFrame;
    }
#endif // _MACOSX_FIX_ME
}

void mgProcessGameListNew(mgqueuegamelistnew *newlist)
{
    // fill in later
}

void mgProcessNewStatusInfo(mgqueuestatusline *status)
{
    statusline *statinfo;

    if (mgConnectingStatusWindow!=NULL)
    {
        statinfo = memAlloc(sizeof(statusline), "StatusWindowList",NonVolatile);

        strcpy(statinfo->message, status->status.message);

        listAddNode(&statusitemlist, &statinfo->link, statinfo);

        uicListAddItem(mgConnectingStatusWindow, (void *)statinfo, 0,UICLW_AddToTail);
    }
}

bool mgIsPlayerInList(LinkedList *list, gameplayerinfo *find)
{
    Node           *walk;
    gameplayerinfo *player;

    walk = list->head;
    while (walk != NULL)
    {
        player = listGetStructOfNode(walk);

        if (strcmp(player->name,find->name) == 0)
        {
            return (TRUE);
        }

        walk = walk->next;
    }
    return (FALSE);
}

void mgProcessGamePlayerInfo(mgqueuegameplayerinfo *player)
{
    gameplayerinfo *playerinfo, *newplayer, *oldplayer;
    Node           *oldlist, *newlist;
    chatlist       *chatinfo;

    if (player->player.index == 0)
    {
        if (mgGameUserNameWindow!=NULL)
        {
            uicListCleanUp(mgGameUserNameWindow);
        }
        listMoveContentsOfList(&listofplayersold, &listofplayers);

        spFindMap(tpGameCreated.DisplayMapName);
    }

    playerinfo = memAlloc(sizeof(gameplayerinfo), "List Of Users", NonVolatile);

    strcpy(playerinfo->name, player->player.name);
    playerinfo->baseColor   = player->player.baseColor;
    playerinfo->stripeColor = player->player.stripeColor;
    playerinfo->colorsPicked = player->player.colorsPicked;
    playerinfo->race = player->player.race;
    playerinfo->index = player->player.index;

    listAddNode(&listofplayers, &playerinfo->link, playerinfo);

    if (mgGameUserNameWindow!=NULL)
    {
        playerinfo->item = uicListAddItem(mgGameUserNameWindow, (void *)playerinfo, 0,UICLW_AddToTail);
    }

    // if last player is added, search through the two lists to see who has left and
    // who has joined the game.
    if (player->player.index == sigsNumPlayers-1)
    {
        // search the oldlist and see if you were in it and not in the new list if so then you have left the game
        oldlist = listofplayersold.head;
        while (oldlist != NULL)
        {
            oldplayer = listGetStructOfNode(oldlist);

            if (!mgIsPlayerInList(&listofplayers, oldplayer))
            {
                chatinfo = memAlloc(sizeof(chatlist), "GameChat",NonVolatile);

                sprintf(chatinfo->chatstring, " %s ", strGetString(strHasLeft));
                wcstombs(chatinfo->chatstring + strlen(chatinfo->chatstring), tpGameCreated.Name, MAX_CHATSTRING_LENGTH - strlen(chatinfo->chatstring));
                strcpy(chatinfo->userName, oldplayer->name);
                chatinfo->messagetype = MG_MESSAGECHAT;
                chatinfo->baseColor   = mgGameMessageChatColor;

                mgAddChatToRoomChat(chatinfo, mgGameChatWindowWindow, &listofgamechatinfo);
            }

            oldlist = oldlist->next;
        }

        // search the new list and see if you were in it and not in the old list then you have joined the game
        newlist = listofplayers.head;
        while (newlist != NULL)
        {
            newplayer = listGetStructOfNode(newlist);

            if (!mgIsPlayerInList(&listofplayersold, newplayer))
            {
                if ( (GameCreator) || (listofplayersold.num!=0) ||
                     ( (listofplayersold.num==0) && (newplayer->index==sigsPlayerIndex)) )
                {
                    chatinfo = memAlloc(sizeof(chatlist), "GameChat",NonVolatile);

                    sprintf(chatinfo->chatstring, " %s ", strGetString(strHasJoined));
                    wcstombs(chatinfo->chatstring + strlen(chatinfo->chatstring), tpGameCreated.Name, MAX_CHATSTRING_LENGTH - strlen(chatinfo->chatstring));
                    strcpy(chatinfo->userName, newplayer->name);
                    chatinfo->messagetype = MG_MESSAGECHAT;

                    mgAddChatToRoomChat(chatinfo, mgGameChatWindowWindow, &listofgamechatinfo);
                }
            }
            newlist = newlist->next;
        }

        listDeleteAll(&listofplayersold);
    }
}

void mgProccessNewGameChat(mgqueuechatlist *chat)
{
    gameplayerinfo *userinfo;
    chatlist       *chatinfo;
    Node           *walk;

    walk = listofplayers.head;
    while (walk != NULL)
    {
        userinfo = listGetStructOfNode(walk);

        if (userinfo->index==chat->chat.index)
        {
            chatinfo = memAlloc(sizeof(chatlist), "GameChat",NonVolatile);

            strcpy(chatinfo->chatstring, chat->chat.chatstring);
            strcpy(chatinfo->userName, userinfo->name);
            chatinfo->baseColor = userinfo->baseColor;
            chatinfo->messagetype = chat->chat.messagetype;

            mgAddChatToRoomChat(chatinfo, mgGameChatWindowWindow, &listofgamechatinfo);

            break;
        }

        walk = walk->next;
    }
}

#if 0
void mgProcessNumPeopleRoom(mgqueuenumpeopleroominfo *numpeopleinfo)
{
    Node *walk = listofchannels.head;
    channellist *channelinfo;

    while (walk!=NULL)
    {
        channelinfo = (channellist *)listGetStructOfNode(walk);
        if (wcscmp(numpeopleinfo->channelname,channelinfo->channelname)==0)
        {
            if (channelinfo->numpeople != numpeopleinfo->numpeople)
            {
                channelinfo->numpeople = numpeopleinfo->numpeople;
                refreshChannelsScreenRequest = TRUE;
            }
            break;
        }
        walk = walk->next;
    }
}
#endif

void mgProcessChatDisconnected(void)
{
    mgPrepareMessageBox(strGetString(strChatDisconnected),NULL);
    mgShowScreen(MGS_Message_Box,FALSE);
}

void mgProcessDisolveGame(void)
{
    mgGameInterestedOff();
    mgShowScreen(MGS_Channel_Chat,TRUE);
    mgPrepareMessageBox(strGetString(strCaptainDisolvedGame),NULL);
    mgShowScreen(MGS_Message_Box,FALSE);
    WaitingForGame  = FALSE;
    GameCreator     = FALSE;
}

void mgProcessKickedOut(void)
{
    mgGameInterestedOff();
    titanLeaveGameNotify();
    mgShowScreen(MGS_Channel_Chat,TRUE);
    mgPrepareMessageBox(strGetString(strCommandKickPlayer),NULL);
    mgShowScreen(MGS_Message_Box,FALSE);
    WaitingForGame  = FALSE;
    GameCreator     = FALSE;
}

#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void mgProcessCallBacksTask(void)
{
#ifndef _MACOSX_FIX_ME
    static Node           *walk;
    static Node           *nextnode;
    static channellist    *channelinfo;
    static serverlist     *serverinfo;
    static bool            found;
    static sdword          sizeofpacket, i;
    static ubyte          *packet;
    static ubyte          *copypacket;

    taskYield(0);

#ifndef C_ONLY
    while (1)
#endif
    {
        taskStackSaveCond(0);
#if defined(OEM) || defined(Downloadable) || defined(CGW)
;
#else
        if (mgListOfChannelsWindow!=NULL)
        {
            tpLockChannelList();
            if (tpChannelList.newDataArrived)
            {
                // remove any channels that aren't there any more
                walk = listofchannels.head;
                while (walk!=NULL)
                {
                    channelinfo = (channellist *)listGetStructOfNode(walk);
                    found = FALSE;
                    for (i = 0;i < tpChannelList.numChannels; i++)
                    {
                        if (wcscmp(tpChannelList.tpChannels[i].ChannelName,channelinfo->channelname)==0)
                        {
                            found = TRUE;
                            break;
                        }
                    }

                    nextnode = walk->next;

                    if (!found)
                    {
                        // this channel isn't here anymore, so remove it:
                        uicListRemoveItem(mgListOfChannelsWindow,channelinfo->item);
                        listDeleteNode(walk);
                    }
                    else
                    {
                        if (channelinfo->numpeople != tpChannelList.tpChannels[i].roomfullness)
                        {
                            channelinfo->numpeople = tpChannelList.tpChannels[i].roomfullness;
                            refreshChannelsScreenRequest = TRUE;
                        }
                    }

                    walk = nextnode;
                }

                // add any new channels to list
                for (i = 0; i < tpChannelList.numChannels; i++)
                {
                    walk = listofchannels.head;
                    found = FALSE;
                    while (walk!=NULL)
                    {
                        channelinfo = (channellist *)listGetStructOfNode(walk);
                        if (wcscmp(tpChannelList.tpChannels[i].ChannelName,channelinfo->channelname)==0)
                        {
                            found = TRUE;
                            break;
                        }
                        walk = walk->next;
                    }
                    if (found==FALSE)
                    {
                        channelinfo = (channellist *)memAlloc(sizeof(channellist),"ListofChannels",NonVolatile);

                        wcscpy(channelinfo->channelname, tpChannelList.tpChannels[i].ChannelName);
                        wcscpy(channelinfo->description, tpChannelList.tpChannels[i].ChannelDescription);
                        channelinfo->roomflags = tpChannelList.tpChannels[i].roomflags;
                        channelinfo->numpeople = tpChannelList.tpChannels[i].roomfullness;

                        listAddNode(&listofchannels,&channelinfo->link, channelinfo);
                        channelinfo->item = uicListAddItem(mgListOfChannelsWindow, (ubyte *)channelinfo, UICLI_CanSelect,UICLW_AddToTail);
                    }
                }
                tpChannelList.newDataArrived = FALSE;
            }
            tpUnLockChannelList();

            if (currentScreen == MGS_Available_Channels)
            {
                if (refreshChannelsScreenRequest)
                {
                    regRecursiveSetReallyDirty(ghMainRegion);
                    refreshChannelsScreenRequest = FALSE;
                }
            }
        }

        if (mgListOfServersWindow != NULL)
        {
            tpLockServerList();
            if (tpServerList.newDataArrived)
            {
                // remove any servers that aren't there any more
                walk = listofservers.head;
                while (walk!=NULL)
                {
                    serverinfo = (serverlist *)listGetStructOfNode(walk);
                    found = FALSE;
                    for (i = 0;i < tpServerList.numServers; i++)
                    {
                        if (wcscmp(tpServerList.tpServers[i].ServerName,serverinfo->ServerName)==0)
                        {
                            found = TRUE;
                            break;
                        }
                    }

                    nextnode = walk->next;

                    if (!found)
                    {
                        // this Server isn't here anymore, so remove it:
                        uicListRemoveItem(mgListOfServersWindow,serverinfo->item);
                        listDeleteNode(walk);
                    }
                    else
                    {
#if 0
                        if (serverinfo->ping != tpServerList.tpServers[i].ping)
                        {
                            serverinfo->ping = tpServerList.tpServers[i].ping;
                            refreshServersScreenRequest = TRUE;
                        }
#endif
                        if (serverinfo->reliability != tpServerList.tpServers[i].reliability)
                        {
                            serverinfo->reliability = tpServerList.tpServers[i].reliability;
                            refreshServersScreenRequest = TRUE;
                        }

                        if (serverinfo->flags != tpServerList.tpServers[i].flags)
                        {
                            serverinfo->flags = tpServerList.tpServers[i].flags;
                            refreshServersScreenRequest = TRUE;
                        }
                    }

                    walk = nextnode;
                }

                // add any new servers to list
                for (i = 0; i < tpServerList.numServers; i++)
                {
                    walk = listofservers.head;
                    found = FALSE;
                    while (walk!=NULL)
                    {
                        serverinfo = (serverlist *)listGetStructOfNode(walk);
                        if (wcscmp(tpServerList.tpServers[i].ServerName,serverinfo->ServerName)==0)
                        {
                            found = TRUE;
                            break;
                }
                        walk = walk->next;
            }
                    if (found==FALSE)
                    {
                        serverinfo = (serverlist *)memAlloc(sizeof(serverlist),"listofservers",NonVolatile);

                        wcscpy(serverinfo->ServerName, tpServerList.tpServers[i].ServerName);
                        wcscpy(serverinfo->ServerDescription, tpServerList.tpServers[i].ServerDescription);
                        serverinfo->flags = tpServerList.tpServers[i].flags;
                        serverinfo->reliability = tpServerList.tpServers[i].reliability;
                        serverinfo->ping = tpServerList.tpServers[i].ping;
                        memcpy(serverinfo->addressstore,tpServerList.tpServers[i].addressstore,6);

                        listAddNode(&listofservers,&serverinfo->link, serverinfo);
                        serverinfo->item = uicListAddItem(mgListOfServersWindow, (ubyte *)serverinfo, UICLI_CanSelect,UICLW_AddToTail);
                    }
                }
                tpServerList.newDataArrived = FALSE;
            }
            tpUnLockServerList();

            if (currentScreen == MGS_Choose_Server)
            {
                if (refreshServersScreenRequest)
                {
                    regRecursiveSetReallyDirty(ghMainRegion);
                    refreshServersScreenRequest = FALSE;
                }
            }
        }

        LockMutex(changescreenmutex);
        if (newscreen==TRUE)
        {
            mgShowScreen(screenaskedfor,hideallscreens);

            newscreen=FALSE;
        }
        UnLockMutex(changescreenmutex);

#if PUBLIC_BETA
        if (betaDoneVerifying)
        {
            betaDoneVerifying = FALSE;
            mgBackToConnection(NULL,NULL);
        }
#endif

        LockMutex(gamestartedmutex);
        if (gamestarted)
        {
            sigsNumPlayers = tpGameCreated.numPlayers;
            sigsPressedStartGame = TRUE;

            mgShutdownMultiPlayerGameScreens();

            gamestarted = FALSE;
        }
        UnLockMutex(gamestartedmutex);

        while (queueNumberEntries(mgThreadTransfer)!=0)
        {
            LockQueue(&mgThreadTransfer);

            sizeofpacket = HWDequeue(&mgThreadTransfer, &packet);
            dbgAssert(sizeofpacket > 0);
            copypacket = memAlloc(sizeofpacket,"mg(mgthreadtransfer)", Pyrophoric);
            memcpy(copypacket, packet, sizeofpacket);

            UnLockQueue(&mgThreadTransfer);

            switch (((mgqueuegeneral *)copypacket)->packettype)
            {
                case MG_QUEUEROOMNEWUSERINFO:
                {
                    mgProcessNewUserInRoom((mgqueueuserlist *)copypacket);
                }
                break;
                case MG_QUEUEROOMGONEUSERINFO:
                {
                    mgProcessUserLeftRoom((mgqueueuserlist *)copypacket);
                }
                break;
                case MG_QUEUEROOMCHATINFO:
                {
                    mgProcessNewRoomChat((mgqueuechatlist *)copypacket);
                }
                break;
                case MG_QUEUEPINGINFO:
                {
                    mgProcessPingInfo((mgqueuenewping *)copypacket);
                }
                break;
                case MG_QUEUEGAMELISTGAMEADDED:
                {
                    mgProcessGameListGameAdded((mgqueuegamelistgame *)copypacket);
                }
                break;
                case MG_QUEUEGAMELISTGAMEREMOVED:
                {
                    mgProcessGameListGameRemoved((mgqueuegamelistgame *)copypacket);
                }
                break;
                case MG_QUEUEGAMELISTGAMECHANGED:
                {
                    mgProcessGameListGameChanged((mgqueuegamelistgame *)copypacket);
                }
                break;
                case MG_QUEUEGAMELISTNEW:
                {
                    mgProcessGameListNew((mgqueuegamelistnew *)copypacket);
                }
                break;
                case MG_QUEUESTATUSINFO:
                {
                    mgProcessNewStatusInfo((mgqueuestatusline *)copypacket);
                }
                break;
                case MG_QUEUEGAMEPLAYER:
                {
                    mgProcessGamePlayerInfo((mgqueuegameplayerinfo *)copypacket);
                }
                break;
                case MG_QUEUEGAMECHATINFO:
                {
                    mgProccessNewGameChat((mgqueuechatlist *)copypacket);
                }
                break;
                case MG_QUEUEGAMEDISOLVED:
                {
                    mgProcessDisolveGame();
                }
                break;
                case MG_QUEUECHATDISCONNECTED:
                {
                    mgProcessChatDisconnected();
                }
                break;
                case MG_QUEUEKICKEDOUT:
                {
                    mgProcessKickedOut();
                }
                break;
#if 0
                case MG_QUEUENUMPEOPLEROOM:
                {
                    mgProcessNumPeopleRoom((mgqueuenumpeopleroominfo *)copypacket);
                }
                break;
#endif
            }

            memFree(copypacket);
        }
#endif //defined(OEM) || defined(Downloadable) || defined(CGW)

        taskStackRestoreCond();
        taskYield(0);
    }
#endif // _MACOSX_FIX_ME
}
#pragma optimize("", on)

/*=============================================================================
    Startup the multiplayer game screens and display the connection screen:
=============================================================================*/

void mgStartMultiPlayerGameScreens(regionhandle region, sdword ID, udword event, udword data, bool AlreadyLoggedIn)
{
#if 0       // relocated to mgStartup
    if (!mgScreensHandle)
    {
        feCallbackAddMultiple(mgCallBack);
        feDrawCallbackAddMultiple(mgDrawCallback);
        mgScreensHandle = feScreensLoad(MG_FIBFile);
    }
#endif
    bitClear(tpGameCreated.flag,GAME_IN_PROGRESS);

    mgValidateName();

    mgRunning = TRUE;

    WaitingForGame  = FALSE;
    GameCreator     = FALSE;

    listDeleteAll(&listofgames);
    listDeleteAll(&listofchannels);
    listDeleteAll(&listofchatinfo);
    listDeleteAll(&listofusersinfo);
    listDeleteAll(&listtoping);
    listDeleteAll(&statusitemlist);
    listDeleteAll(&listofplayers);
    listDeleteAll(&listofplayersold);
    listDeleteAll(&listofgamechatinfo);

    mgPlayerLimit = 0;

    mgResetNamePassword();

    if (AlreadyLoggedIn)
    {
        LoggedIn=TRUE;
        multiPlayerGame = TRUE;

        mgShowScreen(MGS_Connecting,TRUE);
        mgConnectingScreenGoto = MGS_Channel_Chat;
        cJoinChannelRequest(GetCurrentChannel(),GetCurrentChannelDescription());
    }
    else
    {
        LoggedIn=FALSE;

#if PUBLIC_BETA
        if (!betaVerified)
        {
            betaVerifying = TRUE;
            mgInternetWON(NULL,NULL);
        }
        else
#endif
            mgShowScreen(MGS_Connection_Method,TRUE);

    }

    taskResume(ProccessCallback);
}

void mgShutdownMultiPlayerGameScreens(void)
{
    taskPause(ProccessCallback);

    listDeleteAll(&listofgames);
    listDeleteAll(&listofchannels);
    listDeleteAll(&listofchatinfo);
    listDeleteAll(&listofusersinfo);
    listDeleteAll(&listofplayers);
    listDeleteAll(&listofgamechatinfo);

    mgPlayerLimit = 0;

    mgRunning = FALSE;

    WaitingForGame = FALSE;
    GameCreator    = FALSE;

    mgShowScreen(-1,TRUE);
}

void mgStartup(void)
{
    mgListOfGamesFont       = frFontRegister(mgListOfGamesFontName);
    mgChatWindowFont        = frFontRegister(mgChatWindowFontName);
    mgUserNameFont          = frFontRegister(mgUserNameFontName);
    mgCurrentChannelFont    = frFontRegister(mgCurrentChannelFontName);
    mgChannelListTitleFont  = frFontRegister(mgChannelListTitleFontName);
    mgChannelListFont       = frFontRegister(mgChannelListFontName);
    mgGameListTitleFont     = frFontRegister(mgGameListTitleFontName);
    mgGameChatFont          = frFontRegister(mgGameChatFontName);
    mgGameUserNameFont      = frFontRegister(mgGameUserNameFontName);
    mgCurrentGameFont       = frFontRegister(mgCurrentGameFontName);
    mgOptionsFont           = frFontRegister(mgOptionsFontName);
    mgConnectingFont        = frFontRegister(mgConnectingFontName);
    mgMessageBoxFont        = frFontRegister(mgMessageBoxFontName);

    if (!mgScreensHandle)
    {
        feCallbackAddMultiple(mgCallBack);
        feDrawCallbackAddMultiple(mgDrawCallback);
        mgScreensHandle = feScreensLoad(MG_FIBFile);
    }

    if (!mgScreensHandleServers)
    {
        feCallbackAddMultiple(mgCallBackServers);
        //feDrawCallbackAddMultiple(mgDrawCallbackServers);
        mgScreensHandleServers = feScreensLoad(MG_FIBFile_SERVERS);
    }

    InitQueue(&mgThreadTransfer, MG_QUEUEBUFFERSIZE);

    GameWereInterestedIn[0] = 0;

    changescreenmutex       = SDL_CreateMutex();
    gamestartedmutex        = SDL_CreateMutex();
    GameWereInterestedInMutex = SDL_CreateMutex();

    ProccessCallback = taskStart(mgProcessCallBacksTask, MG_TaskServicePeriod, 0);
    taskPause(ProccessCallback);

    titanGameStartup();

    switch (strCurLanguage)
    {
        case languageEnglish :
            mgCommandInfoPtr = mgCommandEnglish;
        break;
        case languageFrench :
            mgCommandInfoPtr = mgCommandFrench;
        break;
        case languageGerman :
            mgCommandInfoPtr = mgCommandGerman;
        break;
        case languageSpanish :
            mgCommandInfoPtr = mgCommandSpanish;
        break;
        case languageItalian :
            mgCommandInfoPtr = mgCommandItalian;
        break;
    }

    listInit(&listofIgnoreUsers);
    listInit(&listofBannedUsers);

    //mgGameTypeScriptInit();
}

void mgShutdown(void)
{
    CloseQueue(&mgThreadTransfer);

    SDL_DestroyMutex(changescreenmutex);
    SDL_DestroyMutex(gamestartedmutex);
    SDL_DestroyMutex(GameWereInterestedInMutex);

    titanGameShutdown();
}


/*=============================================================================
    Callbacks from the chatting system.:
=============================================================================*/

void chatReceiveUserJoinReply(short status, unsigned long userID)
{
    mgqueueuserlist user;
    mgqueuechatlist chat;

    switch (status)
    {
        case StatusRouting_InvalidPassword:
            mgDisplayMessage(strGetString(strInvalidRoomPassword));
            cNotifyChatBadResponse();
            return;

        case StatusRouting_UserAlreadyExists:
        case StatusRouting_ClientAlreadyExists:
            mgDisplayMessage(strGetString(strErrorUserAlreadyExists));
            cNotifyChatBadResponse();
            return;

        case StatusRouting_ServerFull:
            mgDisplayMessage(strGetString(strErrorRoomFull));
            cNotifyChatBadResponse();
            return;

        case StatusCommon_Success:
        case StatusRouting_ClientAlreadyRegistered:
            break;

        default:
            mgDisplayMessage(strGetString(strFailedToChat));
            cNotifyChatBadResponse();
            return;
    }

    cNotifyChatConnected();

    LockQueue(&mgThreadTransfer);

    // Queue up the packet that will add you to the user list
    mgMyChatUserID = userID;

    user.header.packettype = MG_QUEUEROOMNEWUSERINFO;
    strcpy(user.user.userName, utyName);
    user.user.userID = userID;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&user, sizeof(mgqueueuserlist));

    // Queue up the packet that will say, <you> have joined the room.
    chat.header.packettype = MG_QUEUEROOMCHATINFO;
    sprintf(chat.chat.chatstring," %s ", strGetString(strHasJoined));
    wcstombs(chat.chat.chatstring + strlen(chat.chat.chatstring), GetCurrentChannel(), MAX_CHATSTRING_LENGTH - strlen(chat.chat.chatstring));
    chat.chat.index = userID;
    chat.chat.messagetype = MG_MESSAGECHAT;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&chat, sizeof(mgqueuechatlist));

    UnLockQueue(&mgThreadTransfer);

    LockMutex(changescreenmutex);

    newscreen = TRUE;
    screenaskedfor = MGS_Channel_Chat;
    hideallscreens = TRUE;

    UnLockMutex(changescreenmutex);
}

void chatReceiveUsersHere(const char *name, unsigned long userID)
{
    mgqueueuserlist user;

    LockQueue(&mgThreadTransfer);

    user.header.packettype = MG_QUEUEROOMNEWUSERINFO;
    user.user.userID       = userID;
    strcpy(user.user.userName, name);

    HWEnqueue(&mgThreadTransfer, (ubyte *)&user, sizeof(mgqueueuserlist));

    UnLockQueue(&mgThreadTransfer);
}

void chatReceiveUsersJoined(const char *name, unsigned long userID)
{
    mgqueueuserlist user;
    mgqueuechatlist chat;

    LockQueue(&mgThreadTransfer);

    user.header.packettype = MG_QUEUEROOMNEWUSERINFO;
    user.user.userID       = userID;
    strcpy(user.user.userName, name);

    HWEnqueue(&mgThreadTransfer, (ubyte *)&user, sizeof(mgqueueuserlist));

    // Queue up the packet that will say, <they> have joined the room.
    chat.header.packettype = MG_QUEUEROOMCHATINFO;
    sprintf(chat.chat.chatstring," %s ", strGetString(strHasJoined));
    wcstombs(chat.chat.chatstring + strlen(chat.chat.chatstring), GetCurrentChannel(), MAX_CHATSTRING_LENGTH - strlen(chat.chat.chatstring));
    chat.chat.index = userID;
    chat.chat.messagetype = MG_MESSAGECHAT;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&chat, sizeof(mgqueuechatlist));

    UnLockQueue(&mgThreadTransfer);
}

void chatReceiveUserLeft(unsigned long userID)
{
    mgqueueuserlist user;
    mgqueuechatlist chat;

    LockQueue(&mgThreadTransfer);

    // Queue up the packet that will say, <they> have left the room.
    chat.header.packettype = MG_QUEUEROOMCHATINFO;
    sprintf(chat.chat.chatstring," %s ", strGetString(strHasLeft));
    wcstombs(chat.chat.chatstring + strlen(chat.chat.chatstring), GetCurrentChannel(), MAX_CHATSTRING_LENGTH - strlen(chat.chat.chatstring));
    chat.chat.index = userID;
    chat.chat.messagetype = MG_MESSAGECHAT;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&chat, sizeof(mgqueuechatlist));

    user.header.packettype = MG_QUEUEROOMGONEUSERINFO;
    user.user.userID       = userID;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&user, sizeof(mgqueueuserlist));

    UnLockQueue(&mgThreadTransfer);
}

void chatReceiveMessage(unsigned long originUserID, sdword whisper,unsigned long type, unsigned long size, const void* chatData)
{
    mgqueuechatlist chat;
    char aNarrowString[MAX_CHATSTRING_LENGTH];
    unsigned long i;
    unsigned long aNumChars = size >> 1;

    if (originUserID != mgMyChatUserID)
    {
        LockQueue(&mgThreadTransfer);

        chat.header.packettype = MG_QUEUEROOMCHATINFO;
        memset(chat.chat.chatstring,0,MAX_CHATSTRING_LENGTH);
        for (i = 0; i < aNumChars; ++i)
        {
            if (wctomb(aNarrowString+i, *((wchar_t*)chatData+i)) == -1)
            {
                UnLockQueue(&mgThreadTransfer);
                return;
            }
        }

        memcpy(chat.chat.chatstring,aNarrowString,aNumChars);
        if (whisper!=0)
            chat.chat.messagetype = MG_WHISPEREDCHAT;
        else
            chat.chat.messagetype = MG_NORMALCHAT;
        chat.chat.index  = originUserID;

        HWEnqueue(&mgThreadTransfer, (ubyte *)&chat, sizeof(mgqueuechatlist));

        UnLockQueue(&mgThreadTransfer);
    }
}

void patchResetVariables(void)
{
    patchLenSoFar = 0;
    patchTotalLen = 0;
    patchErrorMsg = 0;
    patchAbortRequest = 0;
    patchComplete = 0;
}

// note: called from another thread
void titanPatchProgressCB(int lenSoFar,int totalLen)
{
    patchLenSoFar = lenSoFar;
    patchTotalLen = totalLen;
}

// note: called from another thread
void titanGetPatchCompleteCB(void)
{
    patchComplete = 1;
}

// note: called from another thread
void titanGetPatchFailedCB(int patchFailErrorMsg)
{
    patchErrorMsg = patchFailErrorMsg;
}

void drawBarWithOutline(rectangle *rect,real32 progress,color barcolor,color baroutlinecolor)
{
    rectangle temp;
    //rectangle clear;

    temp.x0 = rect->x0;
    temp.y0 = rect->y0;
    temp.x1 = rect->x0 + (sdword)((rect->x1-rect->x0)*progress);
    temp.y1 = rect->y1;
#if 0
    clear.y0 = temp.y0;
    clear.y1 = temp.y1;
    clear.x0 = temp.x1;
    clear.x1 = rect->x1;

    primRectSolid2(&clear, colBlack);
#endif
    primRectSolid2(&temp, barcolor);
    primRectOutline2(rect, 1, baroutlinecolor);
}

extern fonthandle selGroupFont1;

void mgDrawPatchProgress(featom *atom, regionhandle region)
{
    fonthandle oldfont;
    rectangle pos=region->rect;
    real32 progress;
    sdword fontheight;

    bitSet(region->flags,RPE_DrawEveryFrame);

    oldfont = fontMakeCurrent(selGroupFont1);
    fontheight = fontHeight(" ");

    pos.y0 += fontheight;
    pos.y1 = pos.y0 + fontheight;

    primRectSolid2(&pos,colBlack);

    if (patchErrorMsg)
    {
        switch (patchErrorMsg)
        {
            case PATCHFAIL_UNABLE_TO_CONNECT:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPATCHFAIL_UNABLE_TO_CONNECT));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strTryLaterDownloadManually));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, "%s", PATCHSERVER_IPSTRINGS[0]);
                break;

            case PATCHFAIL_ERROR_SENDING_REQUEST:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPATCHFAIL_ERROR_SENDING_REQUEST));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strTryLaterDownloadManually));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, "%s", PATCHSERVER_IPSTRINGS[0]);
                break;

            case PATCHFAIL_ERROR_RECEIVING_HTTP_HEADER:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPATCHFAIL_ERROR_RECEIVING_HTTP_HEADER));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strTryLaterDownloadManually));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, "%s", PATCHSERVER_IPSTRINGS[0]);
                break;

            case PATCHFAIL_INVALID_FILE_LENGTH:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPATCHFAIL_INVALID_FILE_LENGTH));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strTryLaterDownloadManually));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, "%s", PATCHSERVER_IPSTRINGS[0]);
                break;

            case PATCHFAIL_ERROR_RECEIVING_PATCH:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPATCHFAIL_ERROR_RECEIVING_PATCH));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strTryLaterDownloadManually));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, "%s", PATCHSERVER_IPSTRINGS[0]);
                break;

            case PATCHFAIL_UNABLE_TO_START_DOWNLOAD_THREAD:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPATCHFAIL_UNABLE_TO_START_DOWNLOAD_THREAD));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strTryLaterDownloadManually));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, "%s", PATCHSERVER_IPSTRINGS[0]);
                break;

            case PATCHFAIL_INVALID_STATUS_REPLY:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPATCHFAIL_INVALID_STATUS_REPLY));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strTryLaterDownloadManually));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, "%s", PATCHSERVER_IPSTRINGS[0]);
                break;

            case PATCHFAIL_UNABLE_TO_CREATE_FILE:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPatchUnableCreateFile));
                break;

            case PATCHFAIL_UNABLE_TO_WRITE_FILE:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPatchUnableWriteFile));
                break;

            case PATCHFAIL_USERABORT:
                fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPatchUserAbort));
                break;

            default:
                fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPatchGeneralError),patchErrorMsg);
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strTryLaterDownloadManually));
                pos.y0 += fontheight; fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, "%s", PATCHSERVER_IPSTRINGS[0]);
                break;
        }
    }
    else if (patchTotalLen > 0)
    {
        fontPrintf(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPatchDownloading), patchLenSoFar, patchTotalLen);
        pos.y0 += fontheight;
        pos.y1 += 100;

        progress = (real32)patchLenSoFar / (real32)patchTotalLen;
        if (progress < 0.0f) progress = 0.0f;
        else if (progress > 1.0f) progress = 1.0f;

        drawBarWithOutline(&pos,progress,PATCHBARCOLOR,PATCHBAROUTLINECOLOR);
    }
    else
    {
        fontPrint(pos.x0,pos.y0, mgMessageBoxColor, strGetString(strPatchStartingDownload));
    }

    fontMakeCurrent(oldfont);
}

void mgYesDownloadPatch(char *name, featom *atom)
{
    char patchpath[150];
    mgShowScreen(MGS_Downloading_Patch,FALSE);
    patchResetVariables();
    strcpy(patchpath,languageVersion);
    strcat(patchpath,"/");
    strcat(patchpath,PATCHNAME);
    titanGetPatch(patchpath,PATCHNAME);
}

void mgNoDownloadPatch(char *name, featom *atom)
{
    mgConnectingCancel(NULL,NULL);
}

void mgAbortDownloadPatch(char *name, featom *atom)
{
    patchAbortRequest = 1;
    mgConnectingCancel(NULL,NULL);
}

int authReceiveReply(sword status)
{
    switch (status)
    {
        case StatusCommon_Success :
        if (mgQueryType==MG_WONLogin)
        {
            mgDisplayMessage(strGetString(strConnectedWon));

#if PUBLIC_BETA
            if (betaVerifying)
            {
                betaVerified = TRUE;
                betaVerifying = FALSE;
                betaDoneVerifying = TRUE;
                mgConnectingScreenGoto = MGS_Connection_Method;
                //mgBackToConnection(NULL,NULL);
#if 0
                LockMutex(changescreenmutex);

                newscreen = TRUE;
                screenaskedfor = MGS_Connection_Method;
                hideallscreens = TRUE;

                UnLockMutex(changescreenmutex);
#endif
            }
            else
#endif
            {
                cJoinADefaultRoom();
                LoggedIn = TRUE;

                mgConnectingScreenGoto = MGS_Channel_Chat;
            }

            mgQueryType = -1;
        }
        else if (mgQueryType==MG_WONChangePassword)
        {
            mgDisplayMessage(strGetString(strPasswordChangeSuccess));

            LockMutex(changescreenmutex);

            newscreen = TRUE;
            screenaskedfor = MGS_Internet_Login;
            hideallscreens = TRUE;

            UnLockMutex(changescreenmutex);

            mgQueryType = -1;
        }
        else if (mgQueryType==MG_WONNewUser)
        {
            mgDisplayMessage(strGetString(strNewUserCreated));

            LockMutex(changescreenmutex);

            newscreen = TRUE;
            screenaskedfor = MGS_Internet_Login;
            hideallscreens = TRUE;

            UnLockMutex(changescreenmutex);

            mgQueryType = -1;
        }
        break;

        case StatusCommonFailureStart :
            mgDisplayMessage(strGetString(strLoginFailed));
            mgDisplayMessage(strGetString(strHitCancelAgain));
        return 0;

        case StatusAuth_ExpiredKey:
            mgDisplayMessage(strGetString(strBadKeyExpired));
            mgDisplayMessage(strGetString(strHitCancelAgain));
            break;

        case StatusAuth_VerifyFailed:
            mgDisplayMessage(strGetString(strBadKey));
            mgDisplayMessage(strGetString(strHitCancelAgain));
            break;

        case StatusAuth_LockedOut:
            mgDisplayMessage(strGetString(strBadKeyLockedOut));
            mgDisplayMessage(strGetString(strHitCancelAgain));
        break;

        case StatusAuth_KeyInUse:
            mgDisplayMessage(strGetString(strKeyAlreadyInUse));
            mgDisplayMessage(strGetString(strHitCancelAgain));
            break;

        case StatusAuth_CRCFailed:
            mgDisplayMessage(strGetString(strAuthCRCFailed));
            mgDisplayMessage(strGetString(strHitCancelAgain));
        break;

        case StatusAuth_UserExists:
            mgDisplayMessage(strGetString(strDuplicateUser));
            mgDisplayMessage(strGetString(strHitCancelAgain));
            break;

        case StatusAuth_UserNotFound:
            mgDisplayMessage(strGetString(strUserNotFound));
            mgDisplayMessage(strGetString(strHitCancelAgain));
            break;

        case StatusAuth_BadPassword:
            mgDisplayMessage(strGetString(strIncorrectPassword));
            mgDisplayMessage(strGetString(strHitCancelAgain));
            break;

        case StatusAuth_DirtyWord:
        case StatusAuth_InvalidName:
            mgDisplayMessage(strGetString(strInvalidName));
            mgDisplayMessage(strGetString(strHitCancelAgain));
            break;

        default:
        {
            char errmsg[200];
            sprintf(errmsg,strGetString(strUnknownAuthReply),status);
            mgDisplayMessage(errmsg);
            return 0;
        }
        break;
    }

    return 1;
}

void titanPingReplyReceivedCB(Address *address,unsigned long theLag)
{
    mgqueuenewping ping;

    if (LANGame)
    {
        return;     // ping's not supported in LAN
    }

    LockQueue(&mgThreadTransfer);

    ping.header.packettype = MG_QUEUEPINGINFO;
    ping.ping.IP           = GetIPFromInternetAddress(*address);
    ping.ping.Port         = GetPortFromInternetAddress(*address);
    ping.ping.pingtime     = theLag;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&ping, sizeof(mgqueuenewping));

    UnLockQueue(&mgThreadTransfer);
}

void mgGameListGameAdded(tpscenario *thegame)
{
    mgqueuegamelistgame qgame;

    LockQueue(&mgThreadTransfer);

    qgame.header.packettype = MG_QUEUEGAMELISTGAMEADDED;
    qgame.game = *thegame;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&qgame, sizeof(mgqueuegamelistgame));

    UnLockQueue(&mgThreadTransfer);
}

void mgGameListGameRemoved(tpscenario *thegame)
{
    mgqueuegamelistgame qgame;

    LockQueue(&mgThreadTransfer);

    qgame.header.packettype = MG_QUEUEGAMELISTGAMEREMOVED;
    qgame.game = *thegame;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&qgame, sizeof(mgqueuegamelistgame));

    UnLockQueue(&mgThreadTransfer);
}

void mgGameListGameChanged(tpscenario *thegame)
{
    mgqueuegamelistgame qgame;

    LockQueue(&mgThreadTransfer);

    qgame.header.packettype = MG_QUEUEGAMELISTGAMECHANGED;
    qgame.game = *thegame;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&qgame, sizeof(mgqueuegamelistgame));

    UnLockQueue(&mgThreadTransfer);
}

void mgGameListNew(void)
{
    mgqueuegamelistnew qnew;

    LockQueue(&mgThreadTransfer);

    qnew.header.packettype = MG_QUEUEGAMELISTNEW;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&qnew, sizeof(mgqueuegamelistnew));

    UnLockQueue(&mgThreadTransfer);
}

void mgFailedToConnectToChannel(bool existing)
{
    mgDisplayMessage(strGetString(strFailedToChat));
    mgDisplayMessage(strGetString(strHitCancelContinue));

    mgConnectingScreenGoto = MGS_Channel_Chat;
}

#if 0
void titanGotNumUsersInRoomCB(const wchar_t *theRoomName, int theNumUsers)
{
    mgqueuenumpeopleroominfo qnew;

    LockQueue(&mgThreadTransfer);

    qnew.header.packettype = MG_QUEUENUMPEOPLEROOM;
    wcscpy(qnew.channelname,theRoomName);
    qnew.numpeople = theNumUsers;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&qnew, sizeof(mgqueuenumpeopleroominfo));

    UnLockQueue(&mgThreadTransfer);
}
#endif

void mgDisplayMsgBox(void) {
  LockMutex(changescreenmutex);

  newscreen = TRUE;
  screenaskedfor = MGS_Connecting;
  hideallscreens = TRUE;

  UnLockMutex(changescreenmutex);
}


void mgDisplayMessage(char *message)
{
    mgqueuestatusline status;

    LockQueue(&mgThreadTransfer);

    status.header.packettype = MG_QUEUESTATUSINFO;
    strcpy(status.status.message, message);

    HWEnqueue(&mgThreadTransfer, (ubyte *)&status, sizeof(mgqueuestatusline));

    UnLockQueue(&mgThreadTransfer);
}

void mgNotifyKickedOut(void)
{
    mgqueuegeneral general;

    LockQueue(&mgThreadTransfer);

    general.packettype = MG_QUEUEKICKEDOUT;
    HWEnqueue(&mgThreadTransfer, (ubyte *)&general, sizeof(general));

    UnLockQueue(&mgThreadTransfer);
}

void mgUpdateGameInfo(void)
{
    mgqueuegameplayerinfo player;
    sdword i;

    LockQueue(&mgThreadTransfer);

    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,myAddress))
        {
            sigsPlayerIndex = i;
            utyBaseColor    = tpGameCreated.playerInfo[i].baseColor;
            utyStripeColor  = tpGameCreated.playerInfo[i].stripeColor;
            cpColorsPicked  = tpGameCreated.playerInfo[i].colorsPicked;
        }

        player.header.packettype = MG_QUEUEGAMEPLAYER;
        strcpy(player.player.name, tpGameCreated.playerInfo[i].PersonalName);
        player.player.baseColor   = tpGameCreated.playerInfo[i].baseColor;
        player.player.stripeColor = tpGameCreated.playerInfo[i].stripeColor;
        player.player.colorsPicked = tpGameCreated.playerInfo[i].colorsPicked;
        player.player.race        = tpGameCreated.playerInfo[i].race;
        player.player.index       = i;

        HWEnqueue(&mgThreadTransfer, (ubyte *)&player, sizeof(mgqueuegameplayerinfo));
    }

    sigsNumPlayers = tpGameCreated.numPlayers;

    UnLockQueue(&mgThreadTransfer);
}

void mgGameAlreadyExists(void)
{
    mgDisplayMessage(strGetString(strErrorGameAlreadyExists));
    mgDisplayMessage(strGetString(strHitCancelContinue));
}

void mgCreateGameConfirmed(void)
{
    mgDisplayMessage(strGetString(strCreatedGame));

    LockMutex(changescreenmutex);

    newscreen = TRUE;
    screenaskedfor = MGS_Captain_Wait;
    hideallscreens = TRUE;
    WaitingForGame = TRUE;
    GameCreator    = TRUE;
    captainIndex = 0;
    dbgAssert(IAmCaptain);

    UnLockMutex(changescreenmutex);

    mgUpdateGameInfo();
}

void mgJoinGameConfirmed(void)
{
    mgDisplayMessage(strGetString(strJoinRequestGranted));

    LockMutex(changescreenmutex);

    newscreen = TRUE;
    screenaskedfor = MGS_Player_Wait;
    hideallscreens = TRUE;
    WaitingForGame = TRUE;
    GameCreator    = FALSE;
    captainIndex = 0;
    dbgAssert(!IAmCaptain);

    UnLockMutex(changescreenmutex);
}

void mgJoinGameDenied(void)
{
    mgDisplayMessage(strGetString(strJoinRequestDenied));
}

void mgNotifyAuthRequestFailed(void)
{
     mgDisplayMessage(strGetString(strWonOffline));
     mgDisplayMessage(strGetString(strServerDown));
     mgDisplayMessage(strGetString(strHitCancelContinue));
}

void mgGameStartReceivedCB(const void *blob,unsigned short bloblength)
{
    sdword i;
    bool   found=FALSE;

    dbgAssert(bloblength == sizeof(CaptainGameInfo));
    mgBackuptpGameCreated();
    memcpy(&tpGameCreated,blob,bloblength);

    // scan through to find
    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,myAddress))
        {
            sigsPlayerIndex = i;
            found = TRUE;
        }
    }

    if (found)
    {
        LockMutex(gamestartedmutex);

        gamestarted = TRUE;

        UnLockMutex(gamestartedmutex);
    }
    else
    {
        // my addresss not found.
        char addrstr[50];

        PrintAddressToString(addrstr,&myAddress);

        titanDebug("Fatal: my address not found in game received msg\n");
        titanDebug("Fatal: my address %s",addrstr);
        titanDebug("Fatal: %d Other addresses ",tpGameCreated.numPlayers);

        for (i=0;i<tpGameCreated.numPlayers;i++)
        {
            PrintAddressToString(addrstr,&tpGameCreated.playerInfo[i].address);
            titanDebug("%s\n",addrstr);
        }

        PrintAddressToString(addrstr,&myAddress);
        dbgFatalf(DBG_Loc,"Could not find myaddr %s in game recv msg",addrstr);
    }
}

void mgChatConnectionFailed(void)
{
    mgqueuegeneral connfailed;

    if (LANGame)
    {
        return;
    }

    LockQueue(&mgThreadTransfer);

    connfailed.packettype = MG_QUEUECHATDISCONNECTED;
    HWEnqueue(&mgThreadTransfer, (ubyte *)&connfailed, sizeof(mgqueuegeneral));

    UnLockQueue(&mgThreadTransfer);
}

void mgNotifyGameDisolved(void)
{
    mgqueuegeneral disolved;

    if (LANGame)
    {
        lgNotifyGameDisolved();
        return;
    }

    LockQueue(&mgThreadTransfer);

    disolved.packettype = MG_QUEUEGAMEDISOLVED;
    HWEnqueue(&mgThreadTransfer, (ubyte *)&disolved, sizeof(mgqueuegeneral));

    UnLockQueue(&mgThreadTransfer);
}

void mgProcessGameChatPacket(ChatPacket *packet)
{
    mgqueuechatlist   chat;
    udword    mask;

    LockQueue(&mgThreadTransfer);

    chat.header.packettype = MG_QUEUEGAMECHATINFO;
    chat.chat.index = packet->packetheader.frame;
    memStrncpy(chat.chat.chatstring,packet->message,MAX_CHATSTRING_LENGTH);

    mask = PLAYER_MASK(sigsPlayerIndex);
    mask &= packet->users;

    if (mask==packet->users)
        chat.chat.messagetype = MG_WHISPEREDCHAT;
    else
        chat.chat.messagetype = MG_NORMALCHAT;

    HWEnqueue(&mgThreadTransfer, (ubyte *)&chat, sizeof(mgqueuechatlist));

    UnLockQueue(&mgThreadTransfer);
}

//Game Options setting code

//goes through gametypes.script and sets game options based on gameName.script
void mgSetGameType(char *gameName)
{
    udword i;

    for(i=0;i<preSetGames->numGameTypes;i++)
    {
        if(strcmp(preSetGames->gameType[i].gameName,gameName) == 0)
        {
            //set game flag
            mgSetGameTypeByNum(i);
            gameNum = i;
            //set all on screen options to appropriate states
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mgSetGameTypeByNum
    Description : Set the game settings to match a particular game type
    Inputs      : i game type, corresponds to a button number
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mgSetGameTypeByNum(sdword i)
{
    if ((i >= 0) && (i < preSetGames->numGameTypes))
    {
        mgSetGameTypeByStruct(&preSetGames->gameType[i]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mgSetGameTypeByStruct
    Description : Sets a game settings based on a game settings structure.
    Inputs      : game - game settings.  Flags member will be set according to
                    the flagNeeded mask and all other members will be set if
                    they are not 0xfffffff.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mgSetGameTypeByStruct(GameType *game)
{
    tpGameCreated.flag = (tpGameCreated.flag & (~game->flagNeeded)) | (game->flag & game->flagNeeded);
    if (game->startingFleet != 0xff)
    {
        tpGameCreated.startingFleet = game->startingFleet;
    }
    if (game->bountySize != 0xff)
    {
        tpGameCreated.bountySize = game->bountySize;
    }
    if (game->startingResources != 0xff)
    {
        tpGameCreated.startingResources = game->startingResources;
    }
    if (game->resourceInjectionInterval != 0xffffffff)
    {
        tpGameCreated.resourceInjectionInterval = game->resourceInjectionInterval;
    }
    if (game->resourceInjectionsAmount != 0xffffffff)
    {
        tpGameCreated.resourceInjectionsAmount = game->resourceInjectionsAmount;
    }
    if (game->resourceLumpSumTime != 0xffffffff)
    {
        tpGameCreated.resourceLumpSumTime = game->resourceLumpSumTime;
    }
    if (game->resourceLumpSumAmount != 0xffffffff)
    {
        tpGameCreated.resourceLumpSumAmount = game->resourceLumpSumAmount;
    }
}

void mgGameTypesOtherButtonPressed(void)
{
    sdword i;
    GameType *game;

    game = preSetGames->gameType;
    for(i=0; i < preSetGames->numGameTypes; i++, game++)
    {
        if((tpGameCreated.flag & game->flagNeeded) == game->flag &&
            //tpGameCreated.numComputers == game->numComputers &&
            (game->startingFleet == 0xff || tpGameCreated.startingFleet == game->startingFleet) &&
            (game->bountySize == 0xff || tpGameCreated.bountySize == game->bountySize) &&
            (game->startingResources == 0xff || tpGameCreated.startingResources == game->startingResources) &&
            (game->resourceInjectionInterval == 0xffffffff || tpGameCreated.resourceInjectionInterval == game->resourceInjectionInterval) &&
            (game->resourceInjectionsAmount == 0xffffffff || tpGameCreated.resourceInjectionsAmount == game->resourceInjectionsAmount) &&
            (game->resourceLumpSumTime == 0xffffffff || tpGameCreated.resourceLumpSumTime == game->resourceLumpSumTime) &&
            (game->resourceLumpSumAmount == 0xffffffff || tpGameCreated.resourceLumpSumAmount == game->resourceLumpSumAmount))
        {
            //game types are the same!
            feRadioButtonSet("MG_GameType", i);
            return;
        }
    }
    //no game type is the same!
    //so set the custom button which is numbered 1 above total predefined games
    feRadioButtonSet("MG_GameType", preSetGames->numGameTypes);
    gameNum = preSetGames->numGameTypes;
}

void mgNotifyDirRequestFailed()
{
    mgDisplayMessage(strGetString(strFailedToConnectToDirServer));
}

/*=============================================================================
    Choose Server stuff:
=============================================================================*/

#define mgServerListTitleFont   mgChannelListTitleFont
#define mgServerListFont   mgChannelListFont
#define mgServerTitleColor  mgChannelTitleColor
#define mgServerTitleFlashColor mgTitleFlashColor

#define mgServerListSelectedColor mgChannelListSelectedColor
#define mgServerListNormalColor mgChannelListNormalColor

void mgGoChooseServer(char *name, featom *atom)
{
    serverlist *servlist;

    if (mgListOfServersWindow)
    {
        if (mgListOfServersWindow->CurLineSelected)
        {
            servlist = (serverlist *)mgListOfServersWindow->CurLineSelected->data;
            if (servlist)
            {
                mgReallyStartGame(servlist->addressstore);
            }
        }
        else
        {
            mgReallyStartGame(NULL);
        }
    }
}

void mgStartSpecificFactServerFailedCB(void)
{
    mgDisplayMessage(strGetString(strFailedToStartRoutingServer));
    mgDisplayMessage(strGetString(strHitCancelContinue));
}

void mgCancelChooseServer(char *name, featom *atom)
{
    mgShowScreen(lastScreen, TRUE);
}

void mgDrawTriangleServer(sdword x, color col)
{
    triangle tri;

    if (!mgListOfServersWindow->sortOrder)
    {
        tri.x0 = -4 + x; tri.y0 = 4 + mgListOfServersWindow->reg.rect.y0+MG_VertSpacing+4;
        tri.x1 = 4  + x; tri.y1 = 4 + mgListOfServersWindow->reg.rect.y0+MG_VertSpacing+4;
        tri.x2 = 0  + x; tri.y2 = -4  + mgListOfServersWindow->reg.rect.y0+MG_VertSpacing+4;
    }
    else
    {
        tri.x0 = -4 + x; tri.y0 = -4  + mgListOfServersWindow->reg.rect.y0+MG_VertSpacing+4;
        tri.x1 = 0  + x; tri.y1 = 4 + mgListOfServersWindow->reg.rect.y0+MG_VertSpacing+4;
        tri.x2 = 4  + x; tri.y2 = -4  + mgListOfServersWindow->reg.rect.y0+MG_VertSpacing+4;
    }

    primTriSolid2(&tri, col);
}

void mgDrawListOfServersTitle(rectangle *rect)
{
    sdword     x, y;
    fonthandle oldfont;
    color      flashcol;
    char *string;

    oldfont = fontMakeCurrent(mgServerListTitleFont);

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing;

    if (mgTitleFlashTimer + FLASH_TIMER > taskTimeElapsed)
    {
        bitSet(mgListOfServersWindow->windowflags,UICLW_JustRedrawTitle);
#ifdef DEBUG_STOMP
        regVerify(&mgListOfServersWindow->reg);
#endif
        mgListOfServersWindow->reg.status |= RSF_DrawThisFrame;
        flashcol = mgServerTitleFlashColor;
    }
    else
        flashcol = mgServerTitleColor;

    string = strGetString(strChannelNameHeading);
    if (mgListOfServersWindow->sorttype == MG_ServerSortByName)
    {
        fontPrint(x,y,flashcol,string);
        mgDrawTriangleServer(x+fontWidth(string), flashcol);
    }
    else
        fontPrint(x,y,mgServerTitleColor,string);
    x+= mgServerNameWidth;

    string = strGetString(strChannelDescHeading);
    if (mgListOfServersWindow->sorttype == MG_ServerSortByDescription)
    {
        fontPrint(x,y,flashcol,string);
        mgDrawTriangleServer(x+fontWidth(string), flashcol);
    }
    else
        fontPrint(x,y,mgServerTitleColor,string);
    x+= mgServerDescriptionWidth;

    string = strGetString(strGamePingHeading);
    if (mgListOfServersWindow->sorttype == MG_ServerSortByPing)
    {
        fontPrint(x,y,flashcol,string);
        mgDrawTriangleServer(x+fontWidth(string), flashcol);
    }
    else
        fontPrint(x,y,mgServerTitleColor,string);
    x+= mgServerPingWidth;

    string = strGetString(strServerLoadHeading);
    if (mgListOfServersWindow->sorttype == MG_ServerSortByLoad)
    {
        fontPrint(x,y,flashcol,string);
        mgDrawTriangleServer(x+fontWidth(string), flashcol);
    }
    else
        fontPrint(x,y,mgServerTitleColor,string);
    x+=mgServerLoadWidth;

    string = strGetString(strServerReliabilityHeading);
    if (mgListOfServersWindow->sorttype == MG_ServerSortByReliability)
    {
        fontPrint(x,y,flashcol,string);
        mgDrawTriangleServer(x+fontWidth(string), flashcol);
    }
    else
        fontPrint(x,y,mgServerTitleColor,string);
    x+=mgServerReliabilityWidth;

    fontMakeCurrent(oldfont);
}

// callback for sorting the server list window
bool mgListOfServersCompare(void *firststruct,void *secondstruct)
{
#ifndef _MACOSX_FIX_ME
    sdword i;
    serverlist *one = (serverlist *)(((listitemhandle)firststruct)->data);
    serverlist *two = (serverlist *)(((listitemhandle)secondstruct)->data);

    switch (mgListOfServersWindow->sorttype)
    {
        case MG_ServerSortByName:
            if ((i=wcscasecmp( one->ServerName,two->ServerName)) > 0)
                return (mgListOfServersWindow->sortOrder);
            else
            {
                if (i != 0)
                    return (!mgListOfServersWindow->sortOrder);
                else
                    return (FALSE);
            }

        case MG_ServerSortByDescription:
            if ((i=wcscasecmp( one->ServerDescription,two->ServerDescription)) > 0)
                return (mgListOfServersWindow->sortOrder);
            else
            {
                if (i != 0)
                    return (!mgListOfServersWindow->sortOrder);
                else
                    return (FALSE);
            }

        case MG_ServerSortByPing:
            if (one->ping > two->ping)
                return (mgListOfServersWindow->sortOrder);
            else
            {
                if (one->ping != two->ping)
                    return (!mgListOfServersWindow->sortOrder);
                else
                    return (FALSE);
            }

        case MG_ServerSortByLoad:
            if (one->flags > two->flags)
                return (mgListOfServersWindow->sortOrder);
            else
            {
                if (one->flags != two->flags)
                    return (!mgListOfServersWindow->sortOrder);
                else
                    return (FALSE);
            }

        case MG_ServerSortByReliability:
            if (one->reliability > two->reliability)
                return (mgListOfServersWindow->sortOrder);
            else
            {
                if (one->reliability != two->reliability)
                    return (!mgListOfServersWindow->sortOrder);
                else
                    return (FALSE);
            }
    }
#endif

    return FALSE;
}

// callback if the title is clicked on
void mgListOfServersTitleClick(struct tagRegion *reg, sdword xClicked)
{
    sdword  x = reg->rect.x0+mgServerNameWidth;

    if (mgListOfServersWindow->ListTotal==0) return;

    mgTitleFlashTimer = taskTimeElapsed;
#ifdef DEBUG_STOMP
    regVerify(reg);
#endif
    reg->status |= RSF_DrawThisFrame;

    if (xClicked < x)
    {
        if (mgListOfServersWindow->sorttype == MG_ServerSortByName)
            mgListOfServersWindow->sortOrder = !mgListOfServersWindow->sortOrder;
        else
            mgListOfServersWindow->sortOrder = TRUE;
        mgListOfServersWindow->sorttype = MG_ServerSortByName;
        uicListSort(mgListOfServersWindow,mgListOfServersCompare);
        return;
    }
    x+=mgServerDescriptionWidth;

    if (xClicked < x)
    {
        if (mgListOfServersWindow->sorttype == MG_ServerSortByDescription)
            mgListOfServersWindow->sortOrder = !mgListOfServersWindow->sortOrder;
        else
            mgListOfServersWindow->sortOrder = TRUE;
        mgListOfServersWindow->sorttype = MG_ServerSortByDescription;
        uicListSort(mgListOfServersWindow,mgListOfServersCompare);
        return;
    }
    x+=mgServerPingWidth;

    if (xClicked < x)
    {
        if (mgListOfServersWindow->sorttype == MG_ServerSortByPing)
            mgListOfServersWindow->sortOrder = !mgListOfServersWindow->sortOrder;
        else
            mgListOfServersWindow->sortOrder = TRUE;
        mgListOfServersWindow->sorttype = MG_ServerSortByPing;
        uicListSort(mgListOfServersWindow,mgListOfServersCompare);
        return;
    }
    x += mgServerLoadWidth;

    if (xClicked < x)
    {
        if (mgListOfServersWindow->sorttype == MG_ServerSortByLoad)
            mgListOfServersWindow->sortOrder = !mgListOfServersWindow->sortOrder;
        else
            mgListOfServersWindow->sortOrder = TRUE;
        mgListOfServersWindow->sorttype = MG_ServerSortByLoad;
        uicListSort(mgListOfServersWindow,mgListOfServersCompare);
        return;
    }
    else
    {
        if (mgListOfServersWindow->sorttype == MG_ServerSortByReliability)
            mgListOfServersWindow->sortOrder = !mgListOfServersWindow->sortOrder;
        else
            mgListOfServersWindow->sortOrder = TRUE;
        mgListOfServersWindow->sorttype = MG_ServerSortByReliability;
        uicListSort(mgListOfServersWindow,mgListOfServersCompare);
        return;
    }
}

void mgDrawListOfServersItem(rectangle *rect, listitemhandle data)
{
    char            temp[512];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    serverlist    *serverinfo = (serverlist *)data->data;

    oldfont = fontMakeCurrent(mgServerListFont);

    if (data->flags&UICLI_Selected)
        c = mgServerListSelectedColor;
    else
        c = mgServerListNormalColor;

    x = rect->x0+MG_HorzSpacing;
    y = rect->y0+MG_VertSpacing/2;

    wcstombs(temp,serverinfo->ServerName,512);
    fontPrint(x,y,c,temp);
    x += mgServerNameWidth;

    wcstombs(temp,serverinfo->ServerDescription,512);
    fontPrint(x,y,c,temp);
    x += mgServerDescriptionWidth;

    if (serverinfo->ping <= 0)
        sprintf(temp,"?");
    else
        sprintf(temp,"%i",serverinfo->ping);
    fontPrint(x,y,c,temp);
    x+=mgServerPingWidth;

    sprintf(temp,"%i",serverinfo->flags);
    fontPrint(x,y,c,temp);
    x+=mgServerLoadWidth;

    sprintf(temp,"%i",serverinfo->reliability / 3600);
    fontPrint(x,y,c,temp);
    x+=mgServerReliabilityWidth;

    fontMakeCurrent(oldfont);
}

void mgListOfServersInit(char *name, featom *atom)
{
    fonthandle  oldfont;
    sdword      titleheight, itemheight;
    Node       *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(mgServerListTitleFont);
        titleheight = fontHeight(" ")*2;
        fontMakeCurrent(mgServerListFont);
        itemheight = fontHeight(" ")+MG_VertSpacing;

        mgListOfServersWindow = (listwindowhandle)atom->pData;
        uicListWindowInit(mgListOfServersWindow,
                          mgDrawListOfServersTitle,       // title draw, no title
                          mgListOfServersTitleClick,      // title click process
                          titleheight,                    // title height, no title
                          mgDrawListOfServersItem,        // item draw funtcion
                          itemheight,                     // item height
                          UICLW_CanSelect);

        if (listofservers.num!=0)
        {
            walk = listofservers.head;

            while (walk!=NULL)
            {
                ((serverlist *)listGetStructOfNode(walk))->item =
                    uicListAddItem(mgListOfServersWindow,(ubyte *)listGetStructOfNode(walk),UICLI_CanSelect,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        refreshbaby = taskCallBackRegister(Refresh, 0, NULL, (real32)TITAN_PICKER_REFRESH_TIME);
        donerefreshing=FALSE;

        pingserversbaby = taskCallBackRegister(PingAllServers, 0, NULL, 1.0f);
        pingservernum = 0;

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        taskCallBackRemove(refreshbaby);
        donerefreshing = TRUE;

        taskCallBackRemove(pingserversbaby);
        pingserversbaby = NULL;

        mgListOfServersWindow = NULL;
        return;
    }
    else if (mgListOfServersWindow->message == CM_DoubleClick)
    {
        // add double click on server later    add later
    }
}

