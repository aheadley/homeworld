/*=============================================================================
    Name    : MultiplayerLANGame.c
    Purpose : Logic for the multiplayer LAN game setup.

    Created 10/23/1998 by yo
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#include <windows.h>
#else
#define _GNU_SOURCE   /* Get to wcscasecmp() */
#endif

#include <stdio.h>
#include <strings.h>

#ifndef _MACOSX
    #include <wchar.h>
#endif

#include <stdlib.h>
#include "MultiplayerLANGame.h"
//#include "MultiplayerGame.h"
#include "FEFlow.h"
#include "utility.h"
#include "ScenPick.h"
#include "UIControls.h"
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
#include "File.h"
#include "StatScript.h"
#include "TimeoutTimer.h"
#include "Titan.h"
#include "TitanNet.h"
#include "GameChat.h"

#ifdef _WIN32
#define strncasecmp _strnicmp
#define wcscasecmp  _wcsicmp
#endif

/*=============================================================================
    Defines:
=============================================================================*/

#define LG_FontNameLength       64

#define LG_VertSpacing          (fontHeight(" ") >> 1)
#define LG_HorzSpacing          (fontWidth(" "))

// defines for type of sort for the list of games window
#define LG_SortByGameName       0
#define LG_SortByPingTime       1
#define LG_SortByNumPlayers     2
#define LG_SortByMapName        3



//task parameters
#define LG_TaskStackSize        4096
#define LG_TaskServicePeriod    0.25f

// maximum chat history list window items
#define LG_MaxChatHistory       300

#define LG_QUEUEBUFFERSIZE      50000
#define LG_QUEUEHIGHWATER_MARK  100         // if we have over this # messages start losing lan advertisements.

// defines for which screen is currently active


#define LG_JustOneDisappear     10


// This is the maximum number of items that can be transfered between threads
// using the statically allocated arrays.
#define MAX_NUM_ITEMS           40
#define MAX_NEW_CHAT_ITEMS      80

/*=============================================================================
    Data:
=============================================================================*/

TTimer lgAdvertiseMyselfTimer;
TTimer lgAdvertiseMyGameTimer;

tpscenario lgMyGameInfo;

// multiplayer game base region
fibfileheader  *lgScreensHandle = NULL;

// This flag is returned from the refresh task, will be changed when the observation server goes into effect.
bool            lgdonerefreshing  = TRUE;
bool            lgCreatingNetworkGame = FALSE;

// structures and variables for creating a game.
extern CaptainGameInfo tpGameCreated;
// = {"","","","",0,0,0,0,0,0,0,0,0,0};

// tweakable fonthandles.
fonthandle lgListOfGamesFont=0;
fonthandle lgChatWindowFont=0;
fonthandle lgUserNameFont=0;
fonthandle lgCurrentChannelFont=0;
fonthandle lgChannelListTitleFont=0;
fonthandle lgChannelListFont=0;
fonthandle lgGameListTitleFont=0;
fonthandle lgGameChatFont=0;
fonthandle lgGameUserNameFont=0;
fonthandle lgCurrentGameFont=0;
fonthandle lgOptionsFont=0;
fonthandle lgConnectingFont=0;
fonthandle lgMessageBoxFont=0;
fonthandle lgProtocalFont=0;

char       lgListOfGamesFontName        [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgChatWindowFontName         [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgUserNameFontName           [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgCurrentChannelFontName     [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgChannelListTitleFontName   [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgChannelListFontName        [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgGameListTitleFontName      [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgGameChatFontName           [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgGameUserNameFontName       [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgCurrentGameFontName        [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgOptionsFontName            [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgConnectingFontName         [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgMessageBoxFontName         [LG_FontNameLength] = "Small_Fonts_8.hff";
char       lgProtocalFontName           [LG_FontNameLength] = "EyeChartDisplayCapsSSK_bi11.hff";

// handles for the text entry boxes
textentryhandle lgNameEntryBox                      = NULL;
textentryhandle lgChatTextEntryBox                  = NULL;
textentryhandle lgGameNameTextEntryBox              = NULL;
textentryhandle lgGameChatTextEntryBox              = NULL;
textentryhandle lgResourceInjectionIntervalEntryBox = NULL;
textentryhandle lgResourceInjectionAmountEntryBox   = NULL;
textentryhandle lgResourceLumpSumIntervalEntryBox   = NULL;
textentryhandle lgResourceLumpSumAmountEntryBox     = NULL;
extern textentryhandle mgGamePasswordEntryEntryBox;

// handles for all of the listwindow's
listwindowhandle lgChatHistoryWindow            = NULL;
listwindowhandle lgUserNameWindowWindow         = NULL;
listwindowhandle lgListOfGamesWindow            = NULL;
listwindowhandle lgGameChatWindowWindow         = NULL;
listwindowhandle lgGameUserNameWindow           = NULL;
extern listwindowhandle mgConnectingStatusWindow;

// colors for all of the tweakable colors in the screens.
color lgGameWhisperedColor      = colRGB(255,0,255);
color lgGamePrivateChatColor    = colRGB(20,255,50);
color lgGameNormalChatColor     = colRGB(180,180,180);
color lgGameMessageChatColor    = colRGB(255,255,0);
color lgWhisperedColor          = colRGB(255,0,255);
color lgPrivateChatColor        = colRGB(20,255,50);
color lgNormalChatColor         = colRGB(180,180,180);
color lgMessageChatColor        = colRGB(255,255,0);
color lgUserNameColor           = colRGB(180,180,180);
color lgCurrentChannelColor     = colRGB(20,200,20);
color lgChannelTitleColor       = colRGB(50,255,50);
color lgChannelListNormalColor  = colRGB(180,180,180);
color lgChannelListSelectedColor= colRGB(255,255,255);
color lgTitleFlashColor         = colRGB(255,255,50);
color lgCurrentGameColor        = colRGB(20,200,20);
color lgConnectingStatusColor   = colRGB(180,180,180);
color lgMessageBoxColor         = colRGB(180,180,180);
color lgProtocalFontColor       = colRGB(255,200,0);

// variable for flashing the title when it is clicked on.
real32 lgTitleFlashTimer = 0.0f;

// linked lists with the local copy of info for channels and stuff.

// this linked list holds all of the current games in this channel
extern LinkedList  listofgames;
// this list has all of the Room chat history
extern LinkedList  listofchatinfo;
// this list has all of the users in the Current Room in it
extern LinkedList  listofusersinfo;
// this is the list of topics in the status window listbox
extern LinkedList  statusitemlist;
// this is the list of players in the current game
extern LinkedList  listofplayers;
// this is a temporary storage list for the players in a game to compare
// and check if someone has joined or left.
extern LinkedList  listofplayersold;
// this holds all of the history for the game chat window
extern LinkedList  listofgamechatinfo;

extern bool gamestarted;
bool        lgnewscreen       = FALSE;

// Queue for transfering packets between threads.
Queue       lgThreadTransfer;

// mutually exclusive handles for locking data between threads.
void       *lgchangescreenmutex;
extern void *gamestartedmutex;

// info for spacing in the list of games screen
sdword  lgGameNameWidth     = 150;
sdword  lgNumPlayerWidth    = 50;
sdword  lgMapNameWidth      = 110;

// info for spacing in the list of channels screen
sdword  lgChannelNameWidth  = 258;

// for refreshing the game list
//BabyCallBack    *lgrefreshbaby=NULL;


// is the multiplayer game screens running.
bool lgRunning=FALSE;

// used for starting a screen in a separate thread.
extern sdword screenaskedfor;
//sdword lgScreensAdded=0;
bool   lghideallscreens=TRUE;

// type of query asked for from titan.
sdword lgQueryType=-1;

// handle for the task that transfers the informatoin from titan to the main game thread.
taskhandle lgProccessCallback=0;

// pointer to the game that we want to join
extern tpscenario *joingame;

// string that is displayed in the message box
extern char messageBoxString[256];
extern char messageBoxString2[256];

// variable that indicates which screen is displayed after a user hits cancel on the conneting screen
sdword lgConnectingScreenGoto=-1;

// variables for the hokey color and race picker screen
extern udword BaseColorSave;
extern udword StripeColorSave;
extern uword  RaceSave;

// this is the width in pixels of the chat windows
sdword lgchatwindowwidth = 384-20;

extern real32 LAN_ADVERTISE_USER_TIME;
extern real32 LAN_ADVERTISE_USER_TIMEOUT;

extern real32 LAN_ADVERTISE_GAME_TIME;
extern real32 LAN_ADVERTISE_GAME_TIMEOUT;

static wchar_t SeeingDetailsForGameName[MAX_TITAN_GAME_NAME_LEN] = L"";
static wchar_t JustDeletedGameFromGameList[MAX_TITAN_GAME_NAME_LEN] = L"";

/*=============================================================================
    Function Callback Prototypes:
=============================================================================*/




// Shared between the internet screen and the lan screen
void lgNameEntry                    (char *name, featom *atom);
void lgBackToConnection             (char *name, featom *atom);

// Callbacks for the LAN launch screen
void lgLaunchLAN                    (char *name, featom *atom);
void lgLanProtocalButton            (char *name, featom *atom);

// Callbacks for the channel chatting screen.
void lgChatTextEntry                (char *name, featom *atom);
void lgCreateGame                   (char *name, featom *atom);
void lgBackToLogin                  (char *name, featom *atom);
void lgChangeColors                 (char *name, featom *atom);
void lgChatWindowInit               (char *name, featom *atom);
void lgUserNameWindowInit           (char *name, featom *atom);
void lgCurrentChannelDraw           (featom *atom, regionhandle region);

// Callbacks for the game entry screen.
extern void mgGamePasswordEntry            (char *name, featom *atom);
void lgBackFromPassword             (char *name, featom *atom);
//void lgGoPassword                   (char *name, featom *atom);

// Callbacks for the Available Games screen.
void lgSeeDetails                   (char *name, featom *atom);
void lgJoinGame                     (char *name, featom *atom);
void lgListOfGamesInit              (char *name, featom *atom);
//bool lgRefresh                        (udword num, void *data, struct BabyCallBack *baby);

// Callbacks for the waiting for game screens.
void lgGameChatWindowInit           (char *name, featom *atom);
void lgGameUserNameWindowInit       (char *name, featom *atom);
void lgViewDetails                  (char *name, featom *atom);
void lgLeaveGame                    (char *name, featom *atom);
void lgSetupGame                    (char *name, featom *atom);
void lgStartGame                    (char *name, featom *atom);
void lgGameChatTextEntry            (char *name, featom *atom);
void lgCurrentGameDraw              (featom *atom, regionhandle region);

// Callbacks for the Basic Game Options game screen.
void lgCreateGameNow                (char *name, featom *atom);
void lgGameNameTextEntry            (char *name, featom *atom);
void lgChooseScenario               (char *name, featom *atom);


// Callbacks for buttons linking LAN and internet FE screens
void lgAdvanced                     (char *name, featom *atom);
void lgResource                     (char *name, featom *atom);


// Callbacks shared by all of the Game Options screens
void lgBackFromOptions              (char *name, featom *atom);
void lgBasicOptions                 (char *name, featom *atom);

// Callbacks for the Player options screen.
void lgSetColorsNow                 (char *name, featom *atom);
void lgBackFromPlayerOptions        (char *name, featom *atom);


// Callbacks for the quit WON question screen.
void lgYesQuitWON                   (char *name, featom *atom);
void lgDontQuitWON                  (char *name, featom *atom);

// Callbacks from the message box screen.
void lgMessageOk                    (char *name, featom *atom);
extern void mgDrawMessageBox               (featom *atom, regionhandle region);

void lgDrawProtocal                 (featom *atom, regionhandle region);

void mgStartingFleet(char *name, featom *atom);

void lgSendChatMessage(char *towho,char *message);

static void lgExplicitlyDeleteGameFromGameList(wchar_t *name);

/*=============================================================================
    Function Prototypes for the draw callbacks:
=============================================================================*/


/*=============================================================================
    List of Callbacks for adding to the region processor:
=============================================================================*/

fecallback      lgCallBack[]=
{


// Shared between the internet screen and the lan screen
    {lgNameEntry                    ,   "LG_NameEntry"                  },
    {lgBackToConnection             ,   "LG_BackToConnection"           },

// Callbacks for the LAN launch screen
    {lgLaunchLAN                    ,   "LG_LaunchLAN"                  },
    {lgLanProtocalButton            ,   "LG_LanProtocalButton"          },

// Callbacks for the channel chatting screen.
    {lgChatTextEntry                ,   "LG_ChatTextEntry"              },
    {lgCreateGame                   ,   "LG_CreateGame"                 },
    {lgBackToLogin                  ,   "LG_BackToLogin"                },
    {lgChangeColors                 ,   "LG_ChangeColors"               },
    {lgChatWindowInit               ,   "LG_ChatHistoryWindow"          },
    {lgUserNameWindowInit           ,   "LG_UserNameWindow"             },
    {lgSeeDetails                   ,   "LG_SeeDetails"                 },
    {lgJoinGame                     ,   "LG_JoinGame"                   },
    {lgListOfGamesInit              ,   "LG_ListOfGames"                },

// Callbacks for the waiting for game screens.
    {lgGameChatWindowInit           ,   "LG_GameChatWindow"             },
    {lgGameUserNameWindowInit       ,   "LG_GameUserName"               },
    {lgViewDetails                  ,   "LG_ViewDetails"                },
    {lgLeaveGame                    ,   "LG_LeaveGame"                  },
    {lgSetupGame                    ,   "LG_SetupGame"                  },
    {lgStartGame                    ,   "LG_StartGame"                  },
    {lgGameChatTextEntry            ,   "LG_GameChatTextEntry"          },

// Callbacks for the password entry screen.
    {mgGamePasswordEntry            ,   "LG_GamePasswordEntry"          },
    {lgBackFromPassword             ,   "LG_BackFromPassword"           },
    {lgGoPassword                   ,   "LG_GoPassword"                 },

// Callbacks for the Basic Game Options screen.
    {lgCreateGameNow                ,   "LG_CreateGameNow"              },
    {lgGameNameTextEntry            ,   "LG_GameNameTextEntry"          },
    {lgChooseScenario               ,   "LG_ChooseScenario"             },
    {mgStartingFleet                ,   "LG_StartingFleet"              },

// Callbacks for buttons linking LAN and internet FE screens
    {lgResource                     ,   "LG_Resource"                   },
    {lgAdvanced                     ,   "LG_Advanced"                   },


// Callbacks shared by all of the Game Options screens
    {lgBackFromOptions              ,   "LG_BackFromOptions"            },
    {lgBasicOptions                 ,   "LG_BasicOptions"               },

// Callbacks for the Player options screen.
    {lgSetColorsNow                 ,   "LG_SetColorsNow"               },
    {lgBackFromPlayerOptions        ,   "LG_BackFromPlayerOptions"      },

// Callbacks for the scenario picker screen.  In scenpick.h
    {spDonePicking                  ,   "CS_Done"                       },
    {spBackPicking                  ,   "CS_Back"                       },
//    {spScroller                     ,   "CS_Scroller"                   },


// Callbacks from the are you sure, WON quit screen
    {lgYesQuitWON                   ,   "LG_YesQuitWON"                 },
    {lgDontQuitWON                  ,   "LG_DontQuitWON"                },

// Callbacks from the message box screen.
    {lgMessageOk                    ,   "LG_MessageOk"                  },

    {NULL                           ,   NULL                            }
};


fedrawcallback lgDrawCallback[] =
{
    {lgCurrentGameDraw              ,   "LG_CurrentGame"                },
    {mgDrawMessageBox               ,   "LG_DrawMessageBox"             },
    {lgDrawProtocal                 ,   "LG_Protocal"                   },
    {NULL                           ,   NULL                            }
};

void lgProcessCallBacksTask(void);

void mgGameTypeScriptInit();
  // use mgGameTypeScriptInit() instead of making a brand
  // new copy called lgGameTypeScriptInit()

/*=============================================================================
    Functions:
=============================================================================*/

extern fonthandle selGroupFont1;

void lgDrawProtocal(featom *atom, regionhandle region)
{
    rectangle pos=region->rect;
    fonthandle oldfont;
    char *str = (IPGame) ? strGetString(strProtocalTCPIPLAN) : strGetString(strProtocalIPXLAN);
    sdword width;

    oldfont = fontMakeCurrent(lgProtocalFont);
    width = fontWidth(str);

    fontPrint(pos.x1 - width,pos.y0, lgProtocalFontColor, str);

    fontMakeCurrent(oldfont);
}

// this function parses the chat message entered and checks to see if they wanted to send
// a private message.  It has to search through two different lists of users, depending
// on whether it is in game chat or the room chat that they are typing in.

void *lgParseChatEntry(char *messageorig, bool preGameChat)
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

/*=============================================================================
    Miscelanious functions to encapsulate certain repetetive things.:
=============================================================================*/

// this function adds a chat message to the room chat window
void lgAddChatToRoomChat(chatlist *newchat, listwindowhandle listwindow, LinkedList *list)
{
    fonthandle oldfont;
    sdword     width,addwidth,nCharacters,length;
    char       temp[512];
    chatlist  *chatinfo;
    color      col;

    oldfont = fontMakeCurrent(lgChatWindowFont);

    switch (newchat->messagetype)
    {
        case LG_NORMALCHAT:
        {
            sprintf(temp,"<%s>  ",newchat->userName);
            width = LG_HorzSpacing + fontWidth(temp);
            if (list == &listofchatinfo)
                col = lgNormalChatColor;
            else
                col = lgGameNormalChatColor;
        }
        break;
        case LG_WHISPEREDCHAT:
        {
            sprintf(temp,"<%s>%s  ",newchat->userName, strGetString(strWhisperedMessage));
            width = LG_HorzSpacing + fontWidth(temp);
            if (list == &listofchatinfo)
                col = lgPrivateChatColor;
            else
                col = lgGameWhisperedColor;
        }
        break;
        case LG_MESSAGECHAT:
        {
            sprintf(temp,"%s",newchat->userName);
            width = LG_HorzSpacing + fontWidth(temp);
            if (list == &listofchatinfo)
                col = lgMessageChatColor;
            else
                col = lgGameMessageChatColor;
        }
        break;
        case LG_WRAPEDCHAT:
        {
            width = newchat->indent;
            col = newchat->baseColor;
        }
        break;
        default :
            width = 0;
        break;
    }

    if (width+fontWidth(newchat->chatstring) > lgchatwindowwidth)
    {
        chatinfo = (chatlist *)memAlloc(sizeof(chatlist),"GameChat",NonVolatile);
        strcpy(chatinfo->userName, newchat->userName);
        chatinfo->baseColor = col;
        chatinfo->messagetype = LG_WRAPEDCHAT;
        chatinfo->index = newchat->index;
        chatinfo->indent = 0;

        nCharacters = strlen(newchat->chatstring);
        addwidth = fontWidth(newchat->chatstring);
        while (nCharacters>0 && width + addwidth > lgchatwindowwidth)
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

        lgAddChatToRoomChat(chatinfo, listwindow, list);
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
        while (listofchatinfo.num > LG_MaxChatHistory)
        {
            newchat = listGetStructOfNode(listofchatinfo.head);
            uicListRemoveItem(listwindow, newchat->item);

            listDeleteNode(listofchatinfo.head);
        }
    }
    else
        while (listofchatinfo.num > LG_MaxChatHistory)
        {
            listDeleteNode(listofchatinfo.head);
        }

    fontMakeCurrent(oldfont);
}


/*=============================================================================
    Callbacks for the LAN launch screen:
=============================================================================*/

void lgPrepareLanLoginScreen(void)
{
    if (LanProtocalsValid != LANIPXANDTCPIPVALID)
    {
        fescreen *screen = feScreenFind("LLAN_Login");
        featom *atom = feAtomFindInScreen(screen,"LG_LanProtocalButton");
        featom *atom2 = feAtomFindNextInScreen(screen,atom,"LG_LanProtocalButton");

        atom->flags |= FAF_Disabled;
        atom2->flags |= FAF_Disabled;
    }
}

void lgLanProtocalButton(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, LanProtocalButton);
    }
    else
    {
        LanProtocalButton = (ubyte)(size_t)atom->pData;
    }
}

void lgNameEntry(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        lgNameEntryBox = (textentryhandle)atom->pData;
        uicTextEntrySet(lgNameEntryBox,utyName,strlen(utyName)+1);
        uicTextBufferResize(lgNameEntryBox,MAX_USERNAME_LENGTH);
        uicTextEntryInit(lgNameEntryBox,UICTE_UserNameEntry);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            strcpy(utyName,lgNameEntryBox->textBuffer);
        break;
        case CM_GainFocus :
        break;
    }
}

void lgLaunchLAN(char *name, featom *atom)
{
    LANGame = TRUE;
    IPGame = LanProtocalButton; // DO NOT USE TRUE for CPP reasons
    tpGameCreated.numComputers  = 0;

    dbgAssert((IPGame == 0) || (IPGame == 1));

    if (strlen(utyName) < 2)
    {
        GeneralMessageBox(utyName, strGetString(strMustBeAtLeast2Chars));
        return;
    }

    if (!titanStart(LANGame,IPGame))        // first try protocal specified by LanProtocalButton
    {
        IPGame ^= 1;     // didn't work, let's try other protocal
        if (!titanStart(LANGame,IPGame))
        {
            mgPrepareMessageBox(strGetString(strNoLanIPXorTCPIP),NULL);
            mgShowScreen(MGS_Message_Box,FALSE);

            LANGame = FALSE;
            IPGame = 1; // DO NOT USE TRUE for CPP reasons
            return;
        }
    }

    lgRunning = TRUE;

    WaitingForGame  = FALSE;
    GameCreator     = FALSE;

    strcpy(utyName,lgNameEntryBox->textBuffer);

    TTimerStart(&lgAdvertiseMyselfTimer,LAN_ADVERTISE_USER_TIME);

    mgShowScreen(LGS_Channel_Chat,TRUE);

    taskResume(lgProccessCallback);
}

/*=============================================================================
    Callbacks for the channel chatting screen.:
=============================================================================*/

void lgChatTextEntry(char*name,featom *atom)
{
    chatlist *newchat = NULL;
    userlist *user;
    char      temp[512];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        lgChatTextEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(lgChatTextEntryBox,UICTE_NoLossOfFocus|UICTE_ChatTextEntry|UICTE_SpecialChars);
        uicTextBufferResize(lgChatTextEntryBox,MAX_CHATSTRING_LENGTH);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_AcceptText:
            if (strlen(lgChatTextEntryBox->textBuffer)>0)
            {
                newchat = (chatlist *)memAlloc(sizeof(chatlist),"ListofChannelChat",NonVolatile);
                // check if message is a private message or not
                if ((user=lgParseChatEntry(lgChatTextEntryBox->textBuffer, FALSE))!=NULL)
                {
                    char *strtosend = lgChatTextEntryBox->textBuffer+strlen(user->userName)+2;
                    // yes it is
                    if ((strcmp(user->userName,utyName) == 0) && (AddressesAreEqual(user->userAddress,myAddress)))
                    {
                        // talking to yourself eh!
                        strcpy(newchat->chatstring,strtosend);
                        strcpy(newchat->userName, user->userName);
                        newchat->messagetype = LG_WHISPEREDCHAT;
                    }
                    else
                    {
                        // send the message to the intended recipient
                        lgSendChatMessage(user->userName,strtosend);

                        strcpy(newchat->chatstring,strtosend);
                        strcpy(newchat->userName, utyName);
                        newchat->messagetype = LG_WHISPEREDCHAT;
                    }
                }
                else
                {
                    // public chat message
#if 0
                    strcpy(newchat->chatstring,lgChatTextEntryBox->textBuffer);
                    strcpy(newchat->userName, utyName);
                    newchat->messagetype = LG_NORMALCHAT;
#endif
                    memFree(newchat);    // don't print out own chat because we receive our own broadcasts
                    newchat = NULL;

                    // broadcast it to everyone
                    lgSendChatMessage(NULL,lgChatTextEntryBox->textBuffer);
                    uicTextEntrySet(lgChatTextEntryBox,"",0);
                }

                if (newchat) lgAddChatToRoomChat(newchat, lgChatHistoryWindow, &listofchatinfo);

                uicTextEntrySet(lgChatTextEntryBox,"",0);
            }
        break;
        case CM_KeyPressed:
        {
            if ((user=lgParseChatEntry(lgChatTextEntryBox->textBuffer, FALSE))!=NULL)
            {
                sprintf(temp, "/%s ", user->userName);
                if (strlen(lgChatTextEntryBox->textBuffer) < strlen(temp))
                    uicTextEntrySet(lgChatTextEntryBox, temp, strlen(temp));
            }
        }
        break;
        case CM_SpecialKey:
        {
#ifdef DEBUG_STOMP
            regVerify((regionhandle)&lgChatHistoryWindow->reg);
#endif
            if (lgChatTextEntryBox->key==PAGEUPKEY)
            {
                uicListWindowPageUp(lgChatHistoryWindow);
                lgChatHistoryWindow->reg.status |= RSF_DrawThisFrame;
            }
            else if (lgChatTextEntryBox->key==PAGEDOWNKEY)
            {
                uicListWindowPageDown(lgChatHistoryWindow);
                lgChatHistoryWindow->reg.status |= RSF_DrawThisFrame;
            }
        }
        break;
        case CM_GainFocus:
        break;

    }
}

void lgCreateGame(char *name,featom *atom)
{
    lgCreatingNetworkGame = TRUE;

    mgRestoretpGameCreated();
    mgResetNamePassword();

    mgShowScreen(MGS_Basic_Options,TRUE);
}

void lgBackToLogin(char *name,featom *atom)
{
    mgShowScreen(LGS_Quit_WON, FALSE);
}

void lgChangeColors(char *name, featom *atom)
{
    // save the colors so if they change their minds it is backed up
    BaseColorSave   = utyBaseColor;
    StripeColorSave = utyStripeColor;
    RaceSave = whichRaceSelected;

    cpSetColorsToModify(&utyBaseColor, &utyStripeColor);

    mgShowScreen(LGS_Player_Options,TRUE);
}

void lgDrawChatWindowItem(rectangle *rect, listitemhandle data)
{
    char            temp[512];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    chatlist       *chatinfo = (chatlist *)data->data;

    oldfont = fontMakeCurrent(lgChatWindowFont);

    x = rect->x0+LG_HorzSpacing;
    y = rect->y0+LG_VertSpacing/2;

    switch (chatinfo->messagetype)
    {
        case LG_NORMALCHAT:
        {
            sprintf(temp,"<%s>  ",chatinfo->userName);
//            fontPrint(x,y,lgNormalChatColor,temp);
            fontPrint(x,y,colWhite,temp);
            x+=fontWidth(temp);

            c = lgNormalChatColor;
        }
        break;
        case LG_WHISPEREDCHAT:
        {
            sprintf(temp,"<%s>",chatinfo->userName);
            fontPrint(x,y,lgPrivateChatColor,temp);
            x+=fontWidth(temp);

            sprintf(temp, "%s  ", strGetString(strWhisperedMessage));
            fontPrint(x,y,lgWhisperedColor, temp);
            x+=fontWidth(temp);
            c = lgPrivateChatColor;
        }
        break;
        case LG_MESSAGECHAT:
        {
            sprintf(temp,"%s",chatinfo->userName);
            fontPrint(x,y,lgMessageChatColor,temp);
            x+=fontWidth(temp);

            c = lgMessageChatColor;
        }
        break;
        case LG_WRAPEDCHAT:
        {
            c = chatinfo->baseColor;
            x += chatinfo->indent - LG_HorzSpacing;
        }
        break;
    }

    sprintf(temp,"%s",chatinfo->chatstring);
    fontPrint(x,y,c,temp);

    fontMakeCurrent(oldfont);
}

void lgChatWindowInit(char *name, featom *atom)
{
    fonthandle oldfont;
    Node      *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(lgChatWindowFont);
        lgChatHistoryWindow = (listwindowhandle)atom->pData;
        uicListWindowInit(lgChatHistoryWindow,
                          NULL,         //  title draw, no title
                          NULL,         //  title click process, no title
                          0,            //  title height, no title
                          lgDrawChatWindowItem,           // item draw funtcion
                          fontHeight(" "), // item height
                          UICLW_AutoScroll);

        lgchatwindowwidth = atom->width-20;

        if (listofchatinfo.num!=0)
        {
            walk = listofchatinfo.head;

            while (walk!=NULL)
            {
                ((chatlist *)listGetStructOfNode(walk))->item =
                    uicListAddItem(lgChatHistoryWindow,(ubyte *)listGetStructOfNode(walk),0,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        fontMakeCurrent(oldfont);
        return;
    }

    if (FELASTCALL(atom))
    {
        lgChatHistoryWindow = NULL;
        return;
    }
}

void lgDrawUserNameWindowItem(rectangle *rect, listitemhandle data)
{
    char            temp[512];
    sdword          x, y;
    fonthandle      oldfont;
    userlist       *userinfo = (userlist *)data->data;

    oldfont = fontMakeCurrent(lgUserNameFont);

    x = rect->x0+LG_HorzSpacing;
    y = rect->y0+LG_VertSpacing/2;

    sprintf(temp,"%s",userinfo->userName);
    fontPrint(x,y,lgUserNameColor,temp);

    fontMakeCurrent(oldfont);
}

void lgUserNameWindowInit(char *name, featom *atom)
{
    fonthandle oldfont;
    Node      *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(lgUserNameFont);
        lgUserNameWindowWindow = (listwindowhandle)atom->pData;
        uicListWindowInit(lgUserNameWindowWindow,
                          NULL,         //  title draw, no title
                          NULL,         //  title click process, no title
                          0,            //  title height, no title
                          lgDrawUserNameWindowItem,       // item draw funtcion
                          fontHeight(" "), // item height
                          0);

        if (listofusersinfo.num!=0)
        {
            walk = listofusersinfo.head;

            while (walk!=NULL)
            {
                ((userlist *)listGetStructOfNode(walk))->item =
                    uicListAddItem(lgUserNameWindowWindow,(ubyte *)listGetStructOfNode(walk),0,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        lgUserNameWindowWindow = NULL;
        return;
    }
}

/*=============================================================================
    Callbacks for the available games screen:
=============================================================================*/



void lgSeeDetails(char*name,featom*atom)
{
#ifndef _MACOSX_FIX_ME
    dbgAssert(LANGame);

    if (lgListOfGamesWindow->CurLineSelected!=NULL)
    {
        lggamelist *gameinfo = (lggamelist *)lgListOfGamesWindow->CurLineSelected->data;
        wcscpy(SeeingDetailsForGameName,gameinfo->game.Name);
        wcscpy(tpGameCreated.Name,gameinfo->game.Name);
        dbgAssert(SeeingDetailsForGameName[0] != 0);
        mgShowScreen(MGS_Basic_Options_View,TRUE);
    }
#endif
}

void lgRequestJoinGame(tpscenario *game)
{
    mgShowScreen(MGS_Connecting,FALSE);

    mgConnectingScreenGoto = LGS_Channel_Chat;

    lgDisplayMessage(strGetString(strRequestingToJoin));

    titanJoinGameRequest(game);
    listDeleteAll(&listofgamechatinfo);
    listDeleteAll(&listofplayers);
    listDeleteAll(&listofplayersold);
}

void lgJoinGame(char*name,featom*atom)
{
    lggamelist *gameinfo;

    dbgAssert(LANGame);

    if (lgListOfGamesWindow->CurLineSelected!=NULL)
    {
        gameinfo = (lggamelist *)lgListOfGamesWindow->CurLineSelected->data;

        if (AddressesAreEqual(gameinfo->game.directoryCustomInfo.captainAddress,myAddress))
        {
            mgPrepareMessageBox(strGetString(strCantJoinOwnGame),NULL);
            mgShowScreen(LGS_Message_Box,FALSE);
            return;
        }

        if (!forceLAN)
        {
            if (!CheckNetworkVersionCompatibility(gameinfo->game.directoryCustomInfo.versionInfo))
            {
                mgPrepareMessageBox(strGetString(strDifferentVersions),strGetString(strMustUpgradeToSameVersion));
                mgShowScreen(LGS_Message_Box,FALSE);
                return;
            }
        }

#ifndef _MACOSX_FIX_ME
        if (wcslen(gameinfo->game.directoryCustomInfo.stringdata)>1)
        {
            joingame = &gameinfo->game;
            mgShowScreen(LGS_Game_Password,FALSE);
        }
        else
        {
            lgRequestJoinGame(&gameinfo->game);
        }
#endif
    }
}

// the following functions are all callbacks to do with list windows

// callback for sorting the game list window
bool lgListOfGamesCompare(void *firststruct,void *secondstruct)
{
#ifndef _MACOSX_FIX_ME
    sdword i;
#endif

    lggamelist *one = (lggamelist *)(((listitemhandle)firststruct)->data);
    lggamelist *two = (lggamelist *)(((listitemhandle)secondstruct)->data);

    if (one->game.directoryCustomInfo.flag & GAME_IN_PROGRESS)
    {
        if (!(two->game.directoryCustomInfo.flag & GAME_IN_PROGRESS))
            return(FALSE);
    }
    else
    {
        if (two->game.directoryCustomInfo.flag & GAME_IN_PROGRESS)
            return(FALSE);
    }

#ifndef _MACOSX_FIX_ME
    switch (lgListOfGamesWindow->sorttype)
    {
        case LG_SortByGameName:
            if ((i=wcscasecmp( one->game.Name,two->game.Name)) > 0)
                return (lgListOfGamesWindow->sortOrder);
            else
            {
                if (i != 0)
                    return (!lgListOfGamesWindow->sortOrder);
                else
                    return (FALSE);
            }
        case LG_SortByNumPlayers:
            if (one->game.directoryCustomInfo.numPlayers > two->game.directoryCustomInfo.numPlayers)
                return (lgListOfGamesWindow->sortOrder);
            else
            {
                if (one->game.directoryCustomInfo.numPlayers != two->game.directoryCustomInfo.numPlayers)
                    return (!lgListOfGamesWindow->sortOrder);
                else
                    return(FALSE);
            }

        case LG_SortByMapName:
        {
            wchar_t *onemapname = one->game.directoryCustomInfo.stringdata + 1+wcslen(one->game.directoryCustomInfo.stringdata);
            wchar_t *twomapname = two->game.directoryCustomInfo.stringdata + 1+wcslen(two->game.directoryCustomInfo.stringdata);

            if ((i=wcscasecmp( onemapname,twomapname)) > 0)
                return (lgListOfGamesWindow->sortOrder);
            else
            {
                if (i != 0)
                    return (!lgListOfGamesWindow->sortOrder);
                else
                    return(FALSE);
            }
        }
    }
#endif

    return FALSE;
}

// callback if the title is clicked on
void lgListOfGamesTitleClick(struct tagRegion *reg, sdword xClicked)
{
    sdword  x = reg->rect.x0+lgGameNameWidth;

    if (lgListOfGamesWindow->ListTotal==0) return;

    lgTitleFlashTimer = taskTimeElapsed;
#ifdef DEBUG_STOMP
    regVerify(reg);
#endif
    reg->status |= RSF_DrawThisFrame;

    if (xClicked < x)
    {
        if (lgListOfGamesWindow->sorttype == LG_SortByGameName)
            lgListOfGamesWindow->sortOrder = !lgListOfGamesWindow->sortOrder;
        else
            lgListOfGamesWindow->sortOrder = TRUE;
        lgListOfGamesWindow->sorttype = LG_SortByGameName;
        uicListSort(lgListOfGamesWindow,lgListOfGamesCompare);
        return;
    }
    /*
    x+=lgPingTimeWidth;

    if (xClicked < x)
    {
        if (lgListOfGamesWindow->sorttype == LG_SortByPingTime)
            lgListOfGamesWindow->sortOrder = !lgListOfGamesWindow->sortOrder;
        else
            lgListOfGamesWindow->sortOrder = TRUE;
        lgListOfGamesWindow->sorttype = LG_SortByPingTime;
        uicListSort(lgListOfGamesWindow,lgListOfGamesCompare);
        return;
    }
    */

    x+=lgNumPlayerWidth;

    if (xClicked < x)
    {
        if (lgListOfGamesWindow->sorttype == LG_SortByNumPlayers)
            lgListOfGamesWindow->sortOrder = !lgListOfGamesWindow->sortOrder;
        else
            lgListOfGamesWindow->sortOrder = TRUE;

        lgListOfGamesWindow->sorttype = LG_SortByNumPlayers;
        uicListSort(lgListOfGamesWindow,lgListOfGamesCompare);
        return;
    }
    else
    {
        if (lgListOfGamesWindow->sorttype == LG_SortByMapName)
            lgListOfGamesWindow->sortOrder = !lgListOfGamesWindow->sortOrder;
        else
            lgListOfGamesWindow->sortOrder = TRUE;

        lgListOfGamesWindow->sorttype = LG_SortByMapName;
        uicListSort(lgListOfGamesWindow,lgListOfGamesCompare);
        return;
    }
}

void lgDrawTriangle(sdword x, color col)
{
    triangle tri;

    if (!lgListOfGamesWindow->sortOrder)
    {
        tri.x0 = -4 + x; tri.y0 = 4 + lgListOfGamesWindow->reg.rect.y0+LG_VertSpacing+4;
        tri.x1 = 4  + x; tri.y1 = 4 + lgListOfGamesWindow->reg.rect.y0+LG_VertSpacing+4;
        tri.x2 = 0  + x; tri.y2 = -4  + lgListOfGamesWindow->reg.rect.y0+LG_VertSpacing+4;
    }
    else
    {
        tri.x0 = -4 + x; tri.y0 = -4  + lgListOfGamesWindow->reg.rect.y0+LG_VertSpacing+4;
        tri.x1 = 0  + x; tri.y1 = 4 + lgListOfGamesWindow->reg.rect.y0+LG_VertSpacing+4;
        tri.x2 = 4  + x; tri.y2 = -4  + lgListOfGamesWindow->reg.rect.y0+LG_VertSpacing+4;
    }

    primTriSolid2(&tri, col);
}

// callback to draw the title bar for the list
void lgListOfGamesTitleDraw(rectangle *rect)
{
    sdword     x, y;
    fonthandle oldfont;
    color      flashcol;

    oldfont = fontMakeCurrent(lgGameListTitleFont);

    x = rect->x0+LG_HorzSpacing;
    y = rect->y0+LG_VertSpacing;

    if (lgTitleFlashTimer + FLASH_TIMER > taskTimeElapsed)
    {
        bitSet(lgListOfGamesWindow->windowflags,UICLW_JustRedrawTitle);
#ifdef DEBUG_STOMP
        regVerify(&lgListOfGamesWindow->reg);
#endif
        lgListOfGamesWindow->reg.status |= RSF_DrawThisFrame;
        flashcol = lgTitleFlashColor;
    }
    else
        flashcol = mgGameListTitleColor;

    if (lgListOfGamesWindow->sorttype == LG_SortByGameName)
    {
        fontPrint(x,y,flashcol,strGetString(strGameNameHeading));
        lgDrawTriangle(x+fontWidth(strGetString(strGameNameHeading)), flashcol);
    }
    else
        fontPrint(x,y,mgGameListTitleColor,strGetString(strGameNameHeading));

    x+= lgGameNameWidth;

    if (lgListOfGamesWindow->sorttype == LG_SortByNumPlayers)
    {
        fontPrint(x,y,flashcol,strGetString(strGameNumPlayerHeading));
        lgDrawTriangle(x+fontWidth(strGetString(strGameNumPlayerHeading)), flashcol);
    }
    else
        fontPrint(x,y,mgGameListTitleColor,strGetString(strGameNumPlayerHeading));
    x+= lgNumPlayerWidth;

    if (strCurLanguage >= 3)
    {
        x+=15;
    }

    if (lgListOfGamesWindow->sorttype == LG_SortByMapName)
    {
        fontPrint(x,y,flashcol,strGetString(strGameMapHeading));
        lgDrawTriangle(x+fontWidth(strGetString(strGameMapHeading)), flashcol);
    }
    else
        fontPrint(x,y,mgGameListTitleColor,strGetString(strGameMapHeading));
    x+=lgMapNameWidth;

    fontMakeCurrent(oldfont);
}

// callback to draw each item in the list
void lgListOfGamesItemDraw(rectangle *rect, listitemhandle data)
{
    char        temp[512];
    sdword      x, y;
    color       c;
    fonthandle  oldfont;
    lggamelist   *gameinfo = (lggamelist *)data->data;
    bool gameinprogress = gameinfo->game.directoryCustomInfo.flag & GAME_IN_PROGRESS;
    bool diffversion = (!CheckNetworkVersionCompatibility(gameinfo->game.directoryCustomInfo.versionInfo));

#ifndef _MACOSX_FIX_ME
    udword passwordlen;
#endif

    oldfont = fontMakeCurrent(lgListOfGamesFont);

    if (data->flags&UICLI_Selected)
        c = mgGameListSelectedColor;
    else
        c = mgGameListNormalColor;

    if (diffversion != 0)
        c = mgGameListDiffVersionColor;

    if (gameinprogress)
        c = mgGameListStartedColor;

    x = rect->x0+LG_HorzSpacing;
    y = rect->y0+LG_VertSpacing/2;

    wcstombs(temp,gameinfo->game.Name,512);
    fontPrint(x,y,c,temp);
    x+=lgGameNameWidth;

    x+=lgNumPlayerWidth;
    if (strCurLanguage >= 3)
    {
        x+=15;
    }

    sprintf(temp,"%i",gameinfo->game.directoryCustomInfo.numPlayers);
    fontPrint(x-fontWidth(temp)-fontWidth("W"),y,c,temp);

#ifndef _MACOSX_FIX_ME
    passwordlen = wcslen(gameinfo->game.directoryCustomInfo.stringdata);

    wcstombs(temp,gameinfo->game.directoryCustomInfo.stringdata + 1+passwordlen,512);
    fontPrint(x,y,c,temp);
    x += lgMapNameWidth;

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
void lgListOfGamesInit(char *name, featom *atom)
{
    fonthandle      oldfont;
    sdword          titleheight, itemheight;
    Node           *walk;
    lggamelist       *gameinfo;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(lgGameListTitleFont);
        titleheight = fontHeight(" ")*2;
        fontMakeCurrent(lgListOfGamesFont);
        itemheight  = fontHeight(" ")+LG_VertSpacing;

        lgListOfGamesWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(lgListOfGamesWindow,
                          lgListOfGamesTitleDraw,           // title draw, no title
                          lgListOfGamesTitleClick,          // title click process, no title
                          titleheight,                      // title height, no title
                          lgListOfGamesItemDraw,            // item draw funtcion
                          itemheight,                       // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);
        if (listofgames.num!=0)
        {
            walk = listofgames.head;

            while (walk!=NULL)
            {
                gameinfo = (lggamelist *)listGetStructOfNode(walk);

                if (gameinfo->game.directoryCustomInfo.flag & GAME_IN_PROGRESS)
                    gameinfo->item = uicListAddItem(lgListOfGamesWindow, (ubyte *)gameinfo, 0, UICLW_AddToTail);
                else
                    gameinfo->item = uicListAddItem(lgListOfGamesWindow, (ubyte *)gameinfo, UICLI_CanSelect, UICLW_AddToHead);

                walk = walk->next;
            }
        }

        //RefreshRequest(GetCurrentChannel());
        //lgrefreshbaby = taskCallBackRegister(Refresh, 0, NULL, (real32)TITAN_PICKER_REFRESH_TIME);
        lgdonerefreshing=FALSE;

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        //taskCallBackRemove(lgrefreshbaby);
        lgdonerefreshing = TRUE;

        lgListOfGamesWindow = NULL;
        return;
    }
    else if (lgListOfGamesWindow->message == CM_DoubleClick)
    {
        lgJoinGame(NULL, NULL);
    }
}

/*=============================================================================
    Callbacks for the waiting for game screens.:
=============================================================================*/

void lgGameChatItemDraw(rectangle *rect, listitemhandle data)
{
    char            temp[512];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    chatlist       *chatinfo = (chatlist *)data->data;

    oldfont = fontMakeCurrent(lgGameChatFont);

    x = rect->x0+LG_HorzSpacing;
    y = rect->y0+LG_VertSpacing/2;

    switch (chatinfo->messagetype)
    {
        case LG_NORMALCHAT:
        {
            sprintf(temp,"<%s>  ",chatinfo->userName);
            fontPrint(x,y,chatinfo->baseColor,temp);
            x+=fontWidth(temp);

            c = lgNormalChatColor;
        }
        break;
        case LG_WHISPEREDCHAT:
        {
            sprintf(temp,"<%s>",chatinfo->userName);
            fontPrint(x,y,chatinfo->baseColor,temp);
            x+=fontWidth(temp);

            sprintf(temp, "%s  ", strGetString(strWhisperedMessage));
            fontPrint(x,y,lgWhisperedColor, temp);
            x+=fontWidth(temp);
            c = lgPrivateChatColor;
        }
        break;
        case LG_MESSAGECHAT:
        {
            sprintf(temp,"%s",chatinfo->userName);
            fontPrint(x,y,lgGameMessageChatColor,temp);
            x+=fontWidth(temp);

            c = lgGameMessageChatColor;
        }
        break;
        case LG_WRAPEDCHAT:
        {
            c = chatinfo->baseColor;
            x += chatinfo->indent - LG_HorzSpacing;
        }
        break;
    }

    sprintf(temp,"%s",chatinfo->chatstring);
    fontPrint(x,y,c,temp);

    fontMakeCurrent(oldfont);
}

void lgGameChatWindowInit(char *name, featom *atom)
{
    fonthandle      oldfont;
    Node           *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(lgGameChatFont);

        lgGameChatWindowWindow = (listwindowhandle)atom->pData;
        uicListWindowInit(lgGameChatWindowWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          lgGameChatItemDraw,               // item draw funtcion
                          fontHeight(" "),   // item height
                          UICLW_AutoScroll);
        fontMakeCurrent(oldfont);

        if (listofgamechatinfo.num!=0)
        {
            walk = listofgamechatinfo.head;

            while (walk!=NULL)
            {
                ((chatlist *)listGetStructOfNode(walk))->item =
                    uicListAddItem(lgGameChatWindowWindow,(ubyte *)listGetStructOfNode(walk),0,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        return;
    }
    if (FELASTCALL(atom))
    {
        lgGameChatWindowWindow = NULL;
        return;
    }
}

void lgGameUserNameItemDraw(rectangle *rect, listitemhandle data)
{
    gameplayerinfo *playerinfo=(gameplayerinfo *)data->data;
    sdword          x, y;
    fonthandle      oldfont;

    oldfont = fontMakeCurrent(lgGameUserNameFont);

    x = rect->x0+LG_HorzSpacing;
    y = rect->y0+LG_VertSpacing/2;

    fontPrintf(x,y,playerinfo->baseColor,"%s",playerinfo->name);

    fontMakeCurrent(oldfont);
}

void lgGameUserNameWindowInit(char *name, featom *atom)
{
    fonthandle      oldfont;
    Node           *walk;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(lgGameUserNameFont);

        lgGameUserNameWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(lgGameUserNameWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          lgGameUserNameItemDraw,           // item draw funtcion
                          fontHeight(" "),   // item height
                          0);


        if (listofplayers.num!=0)
        {
            walk = listofplayers.head;

            while (walk!=NULL)
            {
                ((gameplayerinfo *)listGetStructOfNode(walk))->item =
                    uicListAddItem(lgGameUserNameWindow,(ubyte *)listGetStructOfNode(walk),0,UICLW_AddToTail);

                walk = walk->next;
            }
        }

        fontMakeCurrent(oldfont);
        return;
    }
    if (FELASTCALL(atom))
    {
        lgGameUserNameWindow = NULL;
        return;
    }
}

void lgViewDetails(char*name,featom *atom)
{
    mgShowScreen(MGS_Basic_Options_View,TRUE);
}

void lgLeaveGame(char*name,featom*atom)
{
    if (GameCreator)
    {
        lgExplicitlyDeleteGameFromGameList(tpGameCreated.Name);
        titanGameDisolved(FALSE);

        mgShowScreen(LGS_Channel_Chat,TRUE);
        GameCreator = FALSE;
        WaitingForGame = FALSE;
    }
    else
    {
        titanLeaveGame(FALSE);

        GameCreator = FALSE;
        WaitingForGame = FALSE;

        mgShowScreen(LGS_Channel_Chat,TRUE);
    }
}

void lgSetupGame(char*name,featom*atom)
{
    BackupGameCreated = tpGameCreated;
    spCurrentSelectedBack = spCurrentSelected;

    mgShowScreen(MGS_Basic_Options_Change, TRUE);
}

void lgStartGame(char*name,featom*atom)
{
#ifndef _MACOSX_FIX_ME
    sdword i;

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
        mgShowScreen(LGS_Message_Box,FALSE);
        return;
    }
#endif

    if (spScenarios[spCurrentSelected].minplayers > (tpGameCreated.numPlayers + tpGameCreated.numComputers))
    {
        char str[200];

        sprintf(str,strGetString(strMinNumPlayersRequired),spScenarios[spCurrentSelected].minplayers);
        mgPrepareMessageBox(str,NULL);
        mgShowScreen(LGS_Message_Box,FALSE);
        return;
    }



    tpGameCreated.flag |= GAME_IN_PROGRESS;
    for (i=tpGameCreated.numPlayers; i < tpGameCreated.numPlayers+tpGameCreated.numComputers; i++)
    {
        tpGameCreated.playerInfo[i].race = ranRandom(0)&0x00000001;
    }

    titanBroadcastPacket(TITANMSGTYPE_GAMEISSTARTING,&tpGameCreated,sizeof(tpGameCreated));

    // update directory server with inprogres flag

    generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
    wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);

    sigsPlayerIndex = 0;
    sigsNumPlayers = tpGameCreated.numPlayers;
    sigsPressedStartGame = TRUE;

    lgShutdownMultiPlayerGameScreens();
#endif // _MACOSX_FIX_ME
}

void lgGameChatTextEntry(char *name, featom *atom)
{
    chatlist *newchat;
    gameplayerinfo *user;
    char            temp[512];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        lgGameChatTextEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(lgGameChatTextEntryBox,UICTE_NoLossOfFocus|UICTE_ChatTextEntry|UICTE_SpecialChars);
        uicTextBufferResize(lgGameChatTextEntryBox,MAX_CHATSTRING_LENGTH-2);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_AcceptText:
            if (strlen(lgGameChatTextEntryBox->textBuffer)>0)
            {
                newchat = (chatlist *)memAlloc(sizeof(chatlist),"GameChat",NonVolatile);

                if ((user=lgParseChatEntry(lgGameChatTextEntryBox->textBuffer, TRUE))!=NULL)
                {
                    if (strcmp(user->name,utyName)==0)
                    {
                        strcpy(newchat->chatstring,lgGameChatTextEntryBox->textBuffer+strlen(user->name)+2);
                        strcpy(newchat->userName, utyName);
                        newchat->baseColor = utyBaseColor;
                        newchat->messagetype   = LG_WHISPEREDCHAT;
                    }
                    else
                    {
                        sendChatMessage(PLAYER_MASK(user->index),lgGameChatTextEntryBox->textBuffer+strlen(user->name)+2,(uword)sigsPlayerIndex);

                        strcpy(newchat->chatstring,lgGameChatTextEntryBox->textBuffer+strlen(user->name)+2);
                        strcpy(newchat->userName, utyName);
                        newchat->baseColor = utyBaseColor;
                        newchat->messagetype   = LG_WHISPEREDCHAT;
                    }
                }
                else
                {
                    strcpy(newchat->chatstring,lgGameChatTextEntryBox->textBuffer);
                    strcpy(newchat->userName, utyName);
                    newchat->baseColor = utyBaseColor;
                    newchat->messagetype   = LG_NORMALCHAT;

                    sendChatMessage(OTHER_PLAYERS_MASK,lgGameChatTextEntryBox->textBuffer,(uword)sigsPlayerIndex);
                    uicTextEntrySet(lgGameChatTextEntryBox,"",0);
}


                lgAddChatToRoomChat(newchat, lgGameChatWindowWindow, &listofgamechatinfo);

                uicTextEntrySet(lgGameChatTextEntryBox,"",0);
            }
        break;
        case CM_KeyPressed:
        {
            if ((user=lgParseChatEntry(lgGameChatTextEntryBox->textBuffer, TRUE))!=NULL)
            {
                sprintf(temp, "/%s ", user->name);
                if (strlen(lgGameChatTextEntryBox->textBuffer) < strlen(temp))
                    uicTextEntrySet(lgGameChatTextEntryBox, temp, strlen(temp));
            }
        }
        break;
        case CM_SpecialKey:
        {
#ifdef DEBUG_STOMP
            regVerify((regionhandle)&lgGameChatWindowWindow->reg);
#endif
            if (lgGameChatTextEntryBox->key==PAGEUPKEY)
            {
                uicListWindowPageUp(lgGameChatWindowWindow);
                lgGameChatWindowWindow->reg.status |= RSF_DrawThisFrame;
            }
            else if (lgGameChatTextEntryBox->key==PAGEDOWNKEY)
            {
                uicListWindowPageDown(lgGameChatWindowWindow);
                lgGameChatWindowWindow->reg.status |= RSF_DrawThisFrame;
            }
        }
        break;
        case CM_GainFocus:
        break;
    }
}

void lgCurrentGameDraw(featom *atom, regionhandle region)
{
    fonthandle oldfont;
    static char asciigamename[MAX_GAMENAME_LENGTH];

    oldfont = fontMakeCurrent(lgCurrentGameFont);

    wcstombs(asciigamename, tpGameCreated.Name, MAX_GAMENAME_LENGTH);
    fontPrint(region->rect.x0,region->rect.y0, lgCurrentGameColor, asciigamename);

    fontMakeCurrent(oldfont);
}


/*=============================================================================
    Callbacks for the create game screen\Options:
=============================================================================*/

/*=============================================================================

=============================================================================*/

regionhandle lgNumCompPlayerReg     = NULL;

void lgDirtyNumPlayerRegions()
{
    if (lgNumCompPlayerReg!=NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(lgNumCompPlayerReg);
#endif
        lgNumCompPlayerReg->status |= RSF_DrawThisFrame;
    }
}

void lgCreateGameNow(char *name, featom *atom)
{
#ifndef _MACOSX_FIX_ME
    if (SeeingDetailsForGameName[0])
    {
        SeeingDetailsForGameName[0] = 0;
        mgShowScreen(LGS_Channel_Chat,TRUE);
        return;
    }

    if (WaitingForGame)
    {
        if (GameCreator)
        {
            dbgAssert(strlen(spScenarios[spCurrentSelected].fileSpec) <= MAX_MAPNAME_LEN);
            strncpy(tpGameCreated.DisplayMapName,spScenarios[spCurrentSelected].title,MAX_MAPNAME_LEN);
            strncpy(tpGameCreated.MapName,spScenarios[spCurrentSelected].fileSpec,MAX_MAPNAME_LEN);

            generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
            wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);

            mgShowScreen(LGS_Captain_Wait, TRUE);
        }
        else
            mgShowScreen(LGS_Player_Wait, TRUE);
    }
    else
    {
        listDeleteAll(&listofgamechatinfo);

        if (mgInvalidGameName())
        {
            return;
        }

        if (mgInvalidGamePassword())
        {
            mgPrepareMessageBox(strGetString(strErrorTypingPassword),strGetString(strMustBeAtLeast2Chars));
            mgShowScreen(LGS_Message_Box,FALSE);
            return;
        }

        tpGameCreated.numPlayers = 1;
        tpGameCreated.playerInfo[0].address = myAddress;
        tpGameCreated.playerInfo[0].playerIndex = 0;
        tpGameCreated.playerInfo[0].baseColor = utyBaseColor;
        tpGameCreated.playerInfo[0].stripeColor = utyStripeColor;
        tpGameCreated.playerInfo[0].colorsPicked = cpColorsPicked;
        tpGameCreated.playerInfo[0].race = whichRaceSelected;
        strcpy(tpGameCreated.playerInfo[0].PersonalName, utyName);

        dbgAssert(strlen(spScenarios[spCurrentSelected].fileSpec) <= MAX_MAPNAME_LEN);
        strncpy(tpGameCreated.DisplayMapName,spScenarios[spCurrentSelected].title,MAX_MAPNAME_LEN);
        strncpy(tpGameCreated.MapName,spScenarios[spCurrentSelected].fileSpec,MAX_MAPNAME_LEN);

        generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
        wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);
        lgMyGameInfo.directoryCustomInfo.captainAddress = myAddress;

        mgGameInterestedIn(tpGameCreated.Name);

        lgdonerefreshing=TRUE;

        WaitingForGame = TRUE;
        GameCreator    = TRUE;
        captainIndex = 0;
        dbgAssert(IAmCaptain);
        TTimerStart(&lgAdvertiseMyGameTimer,LAN_ADVERTISE_GAME_TIME);

        mgShowScreen(LGS_Captain_Wait,TRUE);

        lgUpdateGameInfo();
    }
#endif // _MACOSX_FIX_ME
}

void lgGameNameTextEntry(char *name, featom *atom)
{
    static char asciigamename[MAX_GAMENAME_LENGTH];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        lgGameNameTextEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(lgGameNameTextEntryBox,0);
        uicTextBufferResize(lgGameNameTextEntryBox,MAX_GAMENAME_LENGTH);
        wcstombs(asciigamename, tpGameCreated.Name, MAX_GAMENAME_LENGTH);
        uicTextEntrySet(lgGameNameTextEntryBox,asciigamename, strlen(asciigamename)+1);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            mbstowcs(tpGameCreated.Name,lgGameNameTextEntryBox->textBuffer,strlen(lgGameNameTextEntryBox->textBuffer)+1);
        break;
        case CM_GainFocus :
        break;
    }
}

void lgChooseScenario(char *name, featom *atom)
{
    spScenarioPick(NULL);
}

/*=============================================================================

=============================================================================*/


void lgBackFromOptions(char *name, featom *atom)
{
    if (WaitingForGame)
    {
        if (GameCreator)
        {
            tpGameCreated = BackupGameCreated;
            spCurrentSelected = spCurrentSelectedBack;
            mgShowScreen(LGS_Captain_Wait, TRUE);
        }
        else
        {
            mgShowScreen(LGS_Player_Wait, TRUE);
        }
    }
    else
    {
        mgShowScreen(LGS_Channel_Chat,TRUE);
        SeeingDetailsForGameName[0] = 0;
    }
}

void lgBasicOptions(char *name,featom *atom)
{
    mgShowScreen(MGS_Basic_Options,TRUE);
}


void lgAdvanced(char *name,featom *atom)
{
    mgShowScreen(MGS_Advanced_Options,TRUE);
}

void lgResource(char *name,featom *atom)
{
    mgShowScreen(MGS_Resource_Options,TRUE);
}


/*=============================================================================
    Callbacks for the Player options screen:
=============================================================================*/

void lgSetColorsNow(char *name, featom *atom)
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

void lgBackFromPlayerOptions(char *name, featom *atom)
{
    dbgAssert(lastScreen != -1);

    cpResetRegions();

    whichRaceSelected   = RaceSave;
    utyBaseColor    = BaseColorSave;
    utyStripeColor  = StripeColorSave;

    mgShowScreen(lastScreen, TRUE);
}

/*=============================================================================
    Callbacks for the game entry screen.:
=============================================================================*/

void lgBackFromPassword(char *name, featom *atom)
{
    mgShowScreen(LGS_Channel_Chat,TRUE);
}

void lgGoPassword(char *name, featom *atom)
{
#ifndef _MACOSX_FIX_ME
    static wchar_t widepasswordentryboxtext[MAX_PASSWORD_LENGTH];

    if (joingame!=NULL)
    {
        mbstowcs(widepasswordentryboxtext,mgGamePasswordEntryEntryBox->textBuffer,strlen(mgGamePasswordEntryEntryBox->textBuffer)+1);
        if (wcscmp(widepasswordentryboxtext,joingame->directoryCustomInfo.stringdata)==0)
        {
            lgRequestJoinGame(joingame);
        }
        else
        {
            mgPrepareMessageBox(strGetString(strIncorrectPassword),NULL);
            mgShowScreen(LGS_Message_Box,FALSE);
        }
    }
#endif
}


/*=============================================================================
    callbacks from the Quit WON question box.:
=============================================================================*/

void lgBackToConnection(char *name, featom *atom)
{
    // closedown the titan engine
    titanShutdown();

    mgScreensDisappear();
    lgShutdownMultiPlayerGameScreens();
    LANGame = FALSE;
    IPGame = 1; // DO NOT USE TRUE for CPP reasons
    feScreenStart(ghMainRegion, "Connection_Method");
    mgStartMultiPlayerGameScreens(ghMainRegion,0,0,0, FALSE);       // fix properly later
}

void lgYesQuitWON(char *name, featom *atom)
{
    bool waitforshutdown = FALSE;
    //LoggedIn=FALSE;

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

    //mgShowScreen(MGS_Connection_Method, TRUE);
    mgScreensDisappear();
    lgShutdownMultiPlayerGameScreens();
    LANGame = FALSE;
    IPGame = 1; // DO NOT USE TRUE for CPP reasons
    feScreenStart(ghMainRegion, "Connection_Method");
    mgStartMultiPlayerGameScreens(ghMainRegion,0,0,0, FALSE);       // fix properly later
}

void lgDontQuitWON(char *name, featom *atom)
{
    mgShowScreen(-1, LG_JustOneDisappear);
}

/*=============================================================================
    Callbacks from the message box screen.:
=============================================================================*/

void lgMessageOk(char *name, featom *atom)
{
    mgShowScreen(-1, LG_JustOneDisappear);
}

/*=============================================================================
    Proccess Callbacks from titan stuff.:
=============================================================================*/

void lgProcessUserHere(lgqueueuserhere *user)
{
    LANAdvert_UserHere *userhere = &user->userhere;

    // see if userhere is already in the list

    userlist *userinfo;
    Node     *walk;

    walk = listofusersinfo.head;
    while (walk != NULL)
    {
        userinfo = listGetStructOfNode(walk);

        if ((strcmp(userinfo->userName,userhere->PersonalName) == 0) && (AddressesAreEqual(userinfo->userAddress,userhere->header.from)))
        {
            TTimerReset(&userinfo->userTimeout);        // don't let the timeout timer timeout on us, we're still here!
            return;
        }

        walk = walk->next;
    }

    // we aren't in the list of users, so we should be added:

    userinfo = memAlloc(sizeof(userlist), "ListofUsers",NonVolatile);

    dbgAssert(strlen(userhere->PersonalName) < MAX_PERSONAL_NAME_LEN);
    strcpy(userinfo->userName, userhere->PersonalName);
    TTimerStart(&userinfo->userTimeout,LAN_ADVERTISE_USER_TIMEOUT);
    userinfo->userAddress = userhere->header.from;

    listAddNode(&listofusersinfo, &userinfo->link, (void *)userinfo);

    if (lgUserNameWindowWindow!=NULL)
    {
        userinfo->item = uicListAddItem(lgUserNameWindowWindow, (void *)userinfo, 0,UICLW_AddToTail);
    }
}

int CompareTpGameCreatedExceptPlayersColors(CaptainGameInfo *newcaptaingameinfo)
{
    return memcmp(&tpGameCreated,newcaptaingameinfo,sizeof(tpGameCreated) - sizeof(tpGameCreated.playerInfo));
}

void CopyToTpGameCreatedExceptPlayerColors(CaptainGameInfo *newcaptaingameinfo)
{
    mgBackuptpGameCreated();
    memcpy(&tpGameCreated,newcaptaingameinfo,sizeof(tpGameCreated) - sizeof(tpGameCreated.playerInfo));
}

void lgProcessGameHere(lgqueuegamehere *game)
{
#ifndef _MACOSX_FIX_ME
    LANAdvert_GameHere *gamehere = &game->gamehere;

    // see if gamehere is already in the list

    lggamelist *gameinfo;
    Node     *walk;

    if ((JustDeletedGameFromGameList[0] != 0) && (wcscmp(JustDeletedGameFromGameList,gamehere->game.Name) == 0))
    {
        return;     // ignore games just deleted in case it was left over in Q for a while.
    }

    if ( ((WaitingForGame) && (!GameCreator) && (wcscmp(tpGameCreated.Name,gamehere->captainGameInfo.Name) == 0)) ||
         ((SeeingDetailsForGameName[0] != 0) && (wcscmp(SeeingDetailsForGameName,gamehere->captainGameInfo.Name) == 0))
       )
    {
        if (CompareTpGameCreatedExceptPlayersColors(&gamehere->captainGameInfo))
        {
            //tpGameCreated = gamehere->captainGameInfo;
            CopyToTpGameCreatedExceptPlayerColors(&gamehere->captainGameInfo);
            switch (currentScreen)
            {
                case MGS_Resource_Options_View:
                case MGS_Advanced_Options_View:
                case MGS_Basic_Options_View:
                    regRecursiveSetReallyDirty(ghMainRegion);
                    break;

                default:
                    break;
            }
        }
    }

    walk = listofgames.head;
    while (walk != NULL)
    {
        gameinfo = listGetStructOfNode(walk);

        if (wcscmp(gameinfo->game.Name,gamehere->game.Name) == 0)
        {
            TTimerReset(&gameinfo->gameTimeout);        // don't let the timeout timer timeout on us, we're still here!
            if (memcmp(&gameinfo->game,&gamehere->game,sizeof(gamehere->game)) != 0)
            {
                // different, so update with new one
                gameinfo->game = gamehere->game;
                if (lgListOfGamesWindow!=NULL)
                {
#ifdef DEBUG_STOMP
                    regVerify(&lgListOfGamesWindow->reg);
#endif
                    lgListOfGamesWindow->reg.status |= RSF_DrawThisFrame;
                }
            }
            gameinfo->captainGameInfo = gamehere->captainGameInfo;
            return;
        }

        walk = walk->next;
    }

    // we aren't in the list of games, so we should be added:

    gameinfo = (lggamelist *)memAlloc(sizeof(lggamelist),"ListofGames",NonVolatile);

    gameinfo->game = gamehere->game;
    gameinfo->captainGameInfo = gamehere->captainGameInfo;
    TTimerStart(&gameinfo->gameTimeout,LAN_ADVERTISE_GAME_TIMEOUT);

    listAddNode(&listofgames,&gameinfo->link, gameinfo);

    if (lgListOfGamesWindow!=NULL)
    {
        if (gameinfo->game.directoryCustomInfo.flag & GAME_IN_PROGRESS)
            gameinfo->item = uicListAddItem(lgListOfGamesWindow, (ubyte *)gameinfo, 0, UICLW_AddToTail);
        else
            gameinfo->item = uicListAddItem(lgListOfGamesWindow, (ubyte *)gameinfo, UICLI_CanSelect, UICLW_AddToHead);
    }
    else
    {
        gameinfo->item = NULL;
    }
#endif // _MACOSX_FIX_ME
}

void lgProcessChatMsg(lgqueuechatmsg *chat)
{
    LANAdvert_ChatMsg *chatmsg = &chat->chatmsg;
    chatlist *chatinfo;

    if (chatmsg->whispered)
    {
        if (strcmp(utyName,chatmsg->whisperToWhoPersonalName))
        {
            return;     // whispered message wasn't for us
        }
    }

    chatinfo = memAlloc(sizeof(chatlist), "ListofChat",NonVolatile);

    strcpy(chatinfo->chatstring, chatmsg->chatstring);
    strcpy(chatinfo->userName, chatmsg->chatFrom);
    chatinfo->messagetype = (chatmsg->whispered) ? LG_WHISPEREDCHAT : LG_NORMALCHAT;

    lgAddChatToRoomChat(chatinfo, lgChatHistoryWindow, &listofchatinfo);
}

void lgProcessNewStatusInfo(lgqueuestatusline *status)
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

void lgProcessGamePlayerInfo(lgqueuegameplayerinfo *player)
{
    gameplayerinfo *playerinfo, *newplayer, *oldplayer;
    Node           *oldlist, *newlist;
    chatlist       *chatinfo;

    if (player->player.index == 0)
    {
        if (lgGameUserNameWindow!=NULL)
        {
            uicListCleanUp(lgGameUserNameWindow);
        }
        listMoveContentsOfList(&listofplayersold, &listofplayers);

        spFindMap(tpGameCreated.DisplayMapName);
    }

    playerinfo = memAlloc(sizeof(gameplayerinfo), "List Of Users", NonVolatile);

    strcpy(playerinfo->name, player->player.name);
    playerinfo->baseColor   = player->player.baseColor;
    playerinfo->stripeColor = player->player.stripeColor;
    playerinfo->race = player->player.race;
    playerinfo->index = player->player.index;

    listAddNode(&listofplayers, &playerinfo->link, playerinfo);

    if (lgGameUserNameWindow!=NULL)
    {
        playerinfo->item = uicListAddItem(lgGameUserNameWindow, (void *)playerinfo, 0,UICLW_AddToTail);
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
                chatinfo->messagetype = LG_MESSAGECHAT;
                chatinfo->baseColor   = lgGameMessageChatColor;

                lgAddChatToRoomChat(chatinfo, lgGameChatWindowWindow, &listofgamechatinfo);
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
                    chatinfo->messagetype = LG_MESSAGECHAT;

                    lgAddChatToRoomChat(chatinfo, lgGameChatWindowWindow, &listofgamechatinfo);
                }
            }
            newlist = newlist->next;
        }

        listDeleteAll(&listofplayersold);
    }
}

void lgProccessNewGameChat(lgqueuechatlist *chat)
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

            lgAddChatToRoomChat(chatinfo, lgGameChatWindowWindow, &listofgamechatinfo);

            break;
        }

        walk = walk->next;
    }
}

void lgProcessDisolveGame(void)
{
    lgExplicitlyDeleteGameFromGameList(tpGameCreated.Name);
    mgShowScreen(LGS_Channel_Chat,TRUE);
    mgPrepareMessageBox(strGetString(strCaptainDisolvedGame),NULL);
    mgShowScreen(LGS_Message_Box,FALSE);
    WaitingForGame  = FALSE;
    GameCreator     = FALSE;
}

void lgFillInLANAdvertHeader(LANAdvertHeader *header,LANAdvertType type)
{
    header->from = myAddress;
    header->type = type;
}

void lgAdvertiseMyGame(void)
{
    LANAdvert_GameHere gamehere;

    gamehere.game = lgMyGameInfo;
    gamehere.captainGameInfo = tpGameCreated;

    lgFillInLANAdvertHeader(&gamehere.header,LANAdvertType_GameHere);

    titanSendLanBroadcast(&gamehere,sizeof(gamehere));
}

void lgAdvertiseMyself(void)
{
    LANAdvert_UserHere userhere;

    dbgAssert(strlen(utyName) < MAX_PERSONAL_NAME_LEN);
    strcpy(userhere.PersonalName,utyName);

    lgFillInLANAdvertHeader(&userhere.header,LANAdvertType_UserHere);

    titanSendLanBroadcast(&userhere,sizeof(userhere));
}

/*-----------------------------------------------------------------------------
    Name        : goodstrncpy
    Description : strncpy as one would expect it to work
                  copies up to maxstringlength characters from src to dst, and
                  always puts the null terminator at the end of the copied string!
    Inputs      : dst, src
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
char *goodstrncpy(char *dst,char *src,sdword maxstringlength)
{
    sdword len = strlen(src);
    if (len <= maxstringlength)
    {
        strcpy(dst,src);
    }
    else
    {
        strncpy(dst,src,maxstringlength);
        dst[maxstringlength] = 0;
    }

    return dst;
}

void lgSendChatMessage(char *towho,char *message)
{
    LANAdvert_ChatMsg chatmsg;

    chatmsg.whispered = FALSE;
    if (towho)
    {
        chatmsg.whispered = TRUE;
        dbgAssert(strlen(towho) < MAX_PERSONAL_NAME_LEN);
        strcpy(chatmsg.whisperToWhoPersonalName,towho);
    }

    dbgAssert(strlen(utyName) < MAX_PERSONAL_NAME_LEN);
    strcpy(chatmsg.chatFrom,utyName);

    goodstrncpy(chatmsg.chatstring,message,MAX_CHATSTRING_LENGTH-1);

    lgFillInLANAdvertHeader(&chatmsg.header,LANAdvertType_ChatMsg);

    titanSendLanBroadcast(&chatmsg,sizeof(chatmsg));
}

static void lgExplicitlyDeleteGameFromGameList(wchar_t *name)
{
#ifndef _MACOSX_FIX_ME
    lggamelist *gameinfo;
    Node     *walk2;

    if ((name == NULL) || (name[0] == 0))
    {
        return;
    }

    wcscpy(JustDeletedGameFromGameList,name);

    walk2 = listofgames.head;
    while (walk2 != NULL)
    {
        gameinfo = listGetStructOfNode(walk2);

        if (wcscmp(gameinfo->game.Name,name) == 0)
        {
            // should remove ourselves
            if (lgListOfGamesWindow!=NULL)
            {
                if (gameinfo->item)
                {
                    uicListRemoveItem(lgListOfGamesWindow, gameinfo->item);
                }
            }
            listDeleteNode(&gameinfo->link);

            return;
        }

        walk2 = walk2->next;
    }
#endif
}

#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void lgProcessCallBacksTask(void)
{
    static sdword          sizeofpacket;
    static ubyte          *packet;
    static ubyte          *copypacket;

    taskYield(0);

#ifndef C_ONLY
    while (1)
#endif
    {
        taskStackSaveCond(0);

#if defined(DLPublicBeta) || defined(Downloadable)
        ;
#else
        if (TTimerUpdate(&lgAdvertiseMyselfTimer))
        {
            TTimerReset(&lgAdvertiseMyselfTimer);       // advertise myself
            lgAdvertiseMyself();
        }

        if (GameCreator)
        {
            if (TTimerUpdate(&lgAdvertiseMyGameTimer))
            {
                TTimerReset(&lgAdvertiseMyGameTimer);
                lgAdvertiseMyGame();
            }
        }

        // check for any timeouts in list of users (and update timers)
        {
            userlist *userinfo;
            Node     *walk2;
            Node     *nextwalk2;

            walk2 = listofusersinfo.head;
            while (walk2 != NULL)
            {
                userinfo = listGetStructOfNode(walk2);
                nextwalk2 = walk2->next;

                if (TTimerUpdate(&userinfo->userTimeout))
                {
                    // we timed out, should remove ourselves
                    if (lgUserNameWindowWindow!=NULL)
                    {
                        uicListRemoveItem(lgUserNameWindowWindow, userinfo->item);
                    }
                    listDeleteNode(&userinfo->link);
                }

                walk2 = nextwalk2;
            }
        }

        // check for any timeouts in list of games (and update timers)
        {
            lggamelist *gameinfo;
            Node     *walk2;
            Node     *nextwalk2;

            walk2 = listofgames.head;
            while (walk2 != NULL)
            {
                gameinfo = listGetStructOfNode(walk2);
                nextwalk2 = walk2->next;

                if (TTimerUpdate(&gameinfo->gameTimeout))
                {
                    if (lgListOfGamesWindow!=NULL)
                    {
                        if (gameinfo->item)
                        {
                            uicListRemoveItem(lgListOfGamesWindow, gameinfo->item);
                        }
                    }
                    listDeleteNode(&gameinfo->link);
                }

                walk2 = nextwalk2;
            }
        }

        LockMutex(lgchangescreenmutex);
        if (lgnewscreen==TRUE)
        {
            mgShowScreen(screenaskedfor,lghideallscreens);

            lgnewscreen=FALSE;
        }
        UnLockMutex(lgchangescreenmutex);

        LockMutex(gamestartedmutex);
        if (gamestarted)
        {
            sigsNumPlayers = tpGameCreated.numPlayers;
            sigsPressedStartGame = TRUE;

            lgShutdownMultiPlayerGameScreens();

            gamestarted = FALSE;
        }
        UnLockMutex(gamestartedmutex);

        while (queueNumberEntries(lgThreadTransfer)!=0)
        {
            LockQueue(&lgThreadTransfer);

            sizeofpacket = HWDequeue(&lgThreadTransfer, &packet);
            dbgAssert(sizeofpacket > 0);
            copypacket = memAlloc(sizeofpacket,"lg(lgthreadtransfer)", Pyrophoric);
            memcpy(copypacket, packet, sizeofpacket);

            UnLockQueue(&lgThreadTransfer);

            switch (((lgqueuegeneral *)copypacket)->packettype)
            {
                case LG_QUEUEUSERHERE:
                {
                    lgProcessUserHere((lgqueueuserhere *)copypacket);
                }
                break;
                case LG_QUEUEGAMEHERE:
                {
                    lgProcessGameHere((lgqueuegamehere *)copypacket);
                }
                break;
                case LG_QUEUECHATMSG:
                {
                    lgProcessChatMsg((lgqueuechatmsg *)copypacket);
                }
                break;
                case LG_QUEUESTATUSINFO:
                {
                    lgProcessNewStatusInfo((lgqueuestatusline *)copypacket);
                }
                break;
                case LG_QUEUEGAMEPLAYER:
                {
                    lgProcessGamePlayerInfo((lgqueuegameplayerinfo *)copypacket);
                }
                break;
                case LG_QUEUEGAMECHATINFO:
                {
                    lgProccessNewGameChat((lgqueuechatlist *)copypacket);
                }
                break;
                case LG_QUEUEGAMEDISOLVED:
                {
                    lgProcessDisolveGame();
                }
                break;
            }

            memFree(copypacket);
        }

        JustDeletedGameFromGameList[0] = 0;
#endif //defined(CGW) || defined(Downloadable) || defined(DLPublicBeta)  || defined(OEM)
        taskStackRestoreCond();
        taskYield(0);
    }
}
#pragma optimize("", on)

/*=============================================================================
    Startup the multiplayer game screens and display the connection screen:
=============================================================================*/

void lgStartMultiPlayerLANGameScreens(regionhandle region, sdword ID, udword event, udword data, bool AlreadylgLoggedIn)
{
#if defined(DLPublicBeta)
    //disable this function in demos
    bitSet(((featom *)ID)->flags, FAF_Disabled);
    bitSet(region->status, RSF_RegionDisabled);
#else
    if (FEFIRSTCALL(((featom *)ID)))
    {
        return;
    }
    if (!lgScreensHandle)
    {
        feCallbackAddMultiple(lgCallBack);
        feDrawCallbackAddMultiple(lgDrawCallback);
        lgScreensHandle = feScreensLoad(LG_FIBFile);
    }

    dbgAssert(LANGame);

    bitClear(tpGameCreated.flag,GAME_IN_PROGRESS);

    WaitingForGame  = FALSE;
    GameCreator     = FALSE;

    listDeleteAll(&listofgames);
    listDeleteAll(&listofchatinfo);
    listDeleteAll(&listofusersinfo);
    listDeleteAll(&statusitemlist);
    listDeleteAll(&listofplayers);
    listDeleteAll(&listofplayersold);
    listDeleteAll(&listofgamechatinfo);

    LockQueue(&lgThreadTransfer);
    ResetQueue(&lgThreadTransfer);
    UnLockQueue(&lgThreadTransfer);

    multiPlayerGame = TRUE;

    mgResetNamePassword();

    if (AlreadylgLoggedIn)
    {
        lgRunning = TRUE;
        TTimerStart(&lgAdvertiseMyselfTimer,LAN_ADVERTISE_USER_TIME);
        mgShowScreen(LGS_Channel_Chat,TRUE);
        taskResume(lgProccessCallback);
    }
    else
    {
        lgPrepareLanLoginScreen();
        mgShowScreen(LGS_LAN_Login,TRUE);
    }
#endif //defined(CGW) || defined(Downloadable) || defined(DLPublicBeta) || defined(OEM)
}

void lgShutdownMultiPlayerGameScreens(void)
{
    taskPause(lgProccessCallback);

    listDeleteAll(&listofgames);
    listDeleteAll(&listofchatinfo);
    listDeleteAll(&listofusersinfo);
    listDeleteAll(&listofplayers);
    listDeleteAll(&listofgamechatinfo);

    TTimerDisable(&lgAdvertiseMyselfTimer);

    lgRunning = FALSE;

    WaitingForGame = FALSE;
    GameCreator    = FALSE;

    mgShowScreen(-1,TRUE);
}

void lgStartup(void)
{
    lgListOfGamesFont       = frFontRegister(lgListOfGamesFontName);
    lgChatWindowFont        = frFontRegister(lgChatWindowFontName);
    lgUserNameFont          = frFontRegister(lgUserNameFontName);
    lgCurrentChannelFont    = frFontRegister(lgCurrentChannelFontName);
    lgChannelListTitleFont  = frFontRegister(lgChannelListTitleFontName);
    lgChannelListFont       = frFontRegister(lgChannelListFontName);
    lgGameListTitleFont     = frFontRegister(lgGameListTitleFontName);
    lgGameChatFont          = frFontRegister(lgGameChatFontName);
    lgGameUserNameFont      = frFontRegister(lgGameUserNameFontName);
    lgCurrentGameFont       = frFontRegister(lgCurrentGameFontName);
    lgOptionsFont           = frFontRegister(lgOptionsFontName);
    lgConnectingFont        = frFontRegister(lgConnectingFontName);
    lgMessageBoxFont        = frFontRegister(lgMessageBoxFontName);
    lgProtocalFont          = frFontRegister(lgProtocalFontName);

    InitQueue(&lgThreadTransfer, LG_QUEUEBUFFERSIZE);

    TTimerDisable(&lgAdvertiseMyselfTimer);

    lgchangescreenmutex = SDL_CreateMutex();
    gamestartedmutex    = SDL_CreateMutex();

    lgProccessCallback = taskStart(lgProcessCallBacksTask, LG_TaskServicePeriod, 0);
    taskPause(lgProccessCallback);

    titanGameStartup();

}

void lgShutdown(void)
{
    CloseQueue(&lgThreadTransfer);

    SDL_DestroyMutex(lgchangescreenmutex);
}


/*=============================================================================
    Callbacks from the chatting system.:
=============================================================================*/

void titanReceivedLanBroadcastCB(const void* thePacket, unsigned short theLen)
{
    if (gameIsRunning || startingGame)
    {
        return;
    }

#define lanAdvert ((LANAdvert *)thePacket)

    LockQueue(&lgThreadTransfer);

    if (queueNumberEntries(lgThreadTransfer) <= LG_QUEUEHIGHWATER_MARK)
    {
        switch (lanAdvert->header.type)
        {
            case LANAdvertType_UserHere:
            {
                lgqueueuserhere userhere;

                userhere.header.packettype = LG_QUEUEUSERHERE;

                if (theLen == sizeof(LANAdvert_UserHere))
                {
                    memcpy(&userhere.userhere,thePacket,sizeof(LANAdvert_UserHere));

                    HWEnqueue(&lgThreadTransfer, (ubyte *)&userhere, sizeof(userhere));
                }
                else
                {
                    titanDebug("Warning: Invalid size of lan advertisement...discarding\n");
                }
            }
            break;

            case LANAdvertType_GameHere:
            {
                lgqueuegamehere gamehere;

                gamehere.header.packettype = LG_QUEUEGAMEHERE;

                if (theLen == sizeof(LANAdvert_GameHere))
                {
                    memcpy(&gamehere.gamehere,thePacket,sizeof(LANAdvert_GameHere));

                    HWEnqueue(&lgThreadTransfer, (ubyte *)&gamehere,sizeof(gamehere));
                }
                else
                {
                    titanDebug("Warning: Invalid size of lan advertisement...discarding\n");
                }
            }
            break;

            case LANAdvertType_ChatMsg:
            {
                lgqueuechatmsg chatmsg;

                chatmsg.header.packettype = LG_QUEUECHATMSG;

                if (theLen == sizeof(LANAdvert_ChatMsg))
                {
                    memcpy(&chatmsg.chatmsg,thePacket,sizeof(LANAdvert_ChatMsg));

                    HWEnqueue(&lgThreadTransfer, (ubyte *)&chatmsg,sizeof(chatmsg));
                }
                else
                {
                    titanDebug("Warning: Invalid size of lan advertisement...discarding\n");
                }
            }
            break;

            default:
                titanDebug("Warning: Unknown lan advertisement received...discarding\n");
                break;
        }
    }
    else
    {
        titanDebug("Warning: Lan Thread Transfer Q over high water mark %d...discarding\n",LG_QUEUEHIGHWATER_MARK);
    }

    UnLockQueue(&lgThreadTransfer);

#undef lanAdvert
}

void lgDisplayMessage(char *message)
{
    lgqueuestatusline status;

    LockQueue(&lgThreadTransfer);

    status.header.packettype = LG_QUEUESTATUSINFO;
    strcpy(status.status.message, message);

    HWEnqueue(&lgThreadTransfer, (ubyte *)&status, sizeof(lgqueuestatusline));

    UnLockQueue(&lgThreadTransfer);
}

void lgUpdateGameInfo(void)
{
    lgqueuegameplayerinfo player;
    sdword i;

    dbgAssert(LANGame);

    LockQueue(&lgThreadTransfer);

    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,myAddress))
        {
            sigsPlayerIndex = i;
            utyBaseColor    = tpGameCreated.playerInfo[i].baseColor;
            utyStripeColor  = tpGameCreated.playerInfo[i].stripeColor;
            cpColorsPicked = tpGameCreated.playerInfo[i].colorsPicked;
        }

        player.header.packettype = LG_QUEUEGAMEPLAYER;
        strcpy(player.player.name, tpGameCreated.playerInfo[i].PersonalName);
        player.player.baseColor   = tpGameCreated.playerInfo[i].baseColor;
        player.player.stripeColor = tpGameCreated.playerInfo[i].stripeColor;
        player.player.colorsPicked = tpGameCreated.playerInfo[i].colorsPicked;
        player.player.race        = tpGameCreated.playerInfo[i].race;
        player.player.index       = i;

        HWEnqueue(&lgThreadTransfer, (ubyte *)&player, sizeof(lgqueuegameplayerinfo));
    }

    sigsNumPlayers = tpGameCreated.numPlayers;

    UnLockQueue(&lgThreadTransfer);
}

void lgJoinGameConfirmed(void)
{
    lgDisplayMessage(strGetString(strJoinRequestGranted));

    LockMutex(lgchangescreenmutex);

    lgnewscreen = TRUE;
    screenaskedfor = LGS_Player_Wait;
    lghideallscreens = TRUE;
    WaitingForGame = TRUE;
    GameCreator    = FALSE;
    captainIndex = 0;
    dbgAssert(!IAmCaptain);

    UnLockMutex(lgchangescreenmutex);
}

void lgJoinGameDenied(void)
{
    lgDisplayMessage(strGetString(strJoinRequestDenied));
}

void lgNotifyGameDisolved(void)
{
    lgqueuegeneral disolved;

    LockQueue(&lgThreadTransfer);

    disolved.packettype = LG_QUEUEGAMEDISOLVED;
    HWEnqueue(&lgThreadTransfer, (ubyte *)&disolved, sizeof(lgqueuegeneral));

    UnLockQueue(&lgThreadTransfer);
}

void lgProcessGameChatPacket(ChatPacket *packet)
{
    lgqueuechatlist   chat;
    udword    mask;

    LockQueue(&lgThreadTransfer);

    chat.header.packettype = LG_QUEUEGAMECHATINFO;
    chat.chat.index = packet->packetheader.frame;
    strncpy(chat.chat.chatstring,packet->message,MAX_CHATSTRING_LENGTH);

    mask = PLAYER_MASK(sigsPlayerIndex);
    mask &= packet->users;

    if (mask==packet->users)
        chat.chat.messagetype = LG_WHISPEREDCHAT;
    else
        chat.chat.messagetype = LG_NORMALCHAT;

    HWEnqueue(&lgThreadTransfer, (ubyte *)&chat, sizeof(lgqueuechatlist));

    UnLockQueue(&lgThreadTransfer);
}

