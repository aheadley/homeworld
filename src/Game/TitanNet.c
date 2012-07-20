/*=============================================================================
    Name    : titannet.c
    Purpose : All of the logic for the titan game interface stuff.

    Created 6/23/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "TitanNet.h"
#include "Types.h"
#include "Debug.h"
#include "StatScript.h"
#include "TitanInterfaceC.h"
#include "CommandNetwork.h"
#include "MultiplayerGame.h"
#include "MultiplayerLANGame.h"
#include "utility.h"
#include "Teams.h"
#include "ScenPick.h"
#include "ColPick.h"
#include "Captaincy.h"
#include "color.h"
#include "File.h"
#include "Titan.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#define DEBUG_GAME_BW

/*=============================================================================
    Data:
=============================================================================*/

unsigned long TitanActive = FALSE;
unsigned long TitanReadyToShutdown = FALSE;

extern TPChannelList tpChannelList;
extern TPServerList  tpServerList;
extern CaptainGameInfo tpGameCreated;
wchar_t CurrentChannel[MAX_CHANNEL_NAME_LEN] = L"";
wchar_t CurrentChannelDescription[MAX_CHANNEL_DESCRIPTION_LEN] = L"";
unsigned long myIP;
unsigned long myPort;

int ChannelProtectedFlag = 0;
extern wchar_t ChannelPassword[MAX_PASSWORD_LEN];

wchar_t RemoveGameRequest[MAX_TITAN_GAME_NAME_LEN] = L"";

long CONNECT_TIMEOUT = 8000;

#define VALIDVERSIONS_MAXLEN 2000

bool HaveValidVersions = FALSE;
char ValidVersions[VALIDVERSIONS_MAXLEN] = "";

/*=============================================================================
    Tweakables:
=============================================================================*/

real32 TITAN_PICKER_REFRESH_TIME = 1.0f;
unsigned short TITAN_GAME_EXPIRE_TIME = 3600;
unsigned long TITAN_CHANNEL_EXPIRE_TIME = 300;


unsigned long AUTHSERVER_NUM = 0;       // get from dirserver now
unsigned long DIRSERVER_NUM   = 1;
unsigned long PATCHSERVER_NUM = 1;

unsigned long AUTHSERVER_PORTS[MAX_PORTS];
extern unsigned long DIRSERVER_PORTS[MAX_PORTS];
extern unsigned long PATCHSERVER_PORTS[MAX_PORTS];

ipString AUTHSERVER_IPSTRINGS[MAX_IPS];
extern ipString DIRSERVER_IPSTRINGS[MAX_IPS];
extern ipString PATCHSERVER_IPSTRINGS[MAX_IPS];

unsigned long GAME_PORT = 2500;
unsigned long AD_PORT = 2501;

char PATCHNAME[50] = "HomeworldPatch.exe";

char ROUTING_SERVER_NAME[30] = "routingserv";

real32 LAN_ADVERTISE_USER_TIME = 1.0f;
real32 LAN_ADVERTISE_USER_TIMEOUT = 5.0f;
real32 LAN_ADVERTISE_GAME_TIME = 1.0f;
real32 LAN_ADVERTISE_GAME_TIMEOUT = 5.0f;

real32 KEEPALIVE_SEND_IAMALIVE_TIME = 10.0f;
real32 KEEPALIVE_IAMALIVE_TIMEOUT = 25.0f;
real32 KEEPALIVE_SEND_ALIVESTATUS_TIME = 30.0f;

color PATCHBARCOLOR = colRGB(255, 0, 0);
color PATCHBAROUTLINECOLOR = colWhite;

udword PRINTLAG_IFGREATERTHAN =        10;
udword PRINTLAG_MINFRAMES =            20;

udword ROOM_MIN_THRESHOLD = 1;
udword ROOM_MAX_THRESHOLD = 50;

unsigned long WAIT_SHUTDOWN_MS = 1000;

void scriptSetIPStrings(char *directory,char *field,void *dataToFillIn);
void scriptSetPortNumbers(char *directory,char *field,void *dataToFillIn);

extern sdword TimedOutWaitingForPauseAcksGiveUpAfterNumTimes;

extern real32 HorseRacePlayerDropoutTime;
extern color HorseRaceDropoutColor;

scriptEntry NetTweaks[] =
{
    makeEntry(PATCHBARCOLOR,scriptSetRGBCB),
    makeEntry(PATCHBAROUTLINECOLOR,scriptSetRGBCB),
    makeEntry(TITAN_PICKER_REFRESH_TIME,scriptSetReal32CB),
    makeEntry(TITAN_GAME_EXPIRE_TIME,scriptSetUwordCB),
    makeEntry(TITAN_CHANNEL_EXPIRE_TIME,scriptSetUdwordCB),
    makeEntry(DIRSERVER_NUM, scriptSetUdwordCB),
    makeEntry(PATCHSERVER_NUM, scriptSetUdwordCB),
    makeEntry(DIRSERVER_PORTS,scriptSetPortNumbers),
    makeEntry(PATCHSERVER_PORTS,scriptSetPortNumbers),
    makeEntry(DIRSERVER_IPSTRINGS,scriptSetIPStrings),
    makeEntry(PATCHSERVER_IPSTRINGS,scriptSetIPStrings),
    makeEntry(GAME_PORT,scriptSetUdwordCB),
    makeEntry(T1_Timeout,scriptSetReal32CB),
    makeEntry(T2_Timeout,scriptSetReal32CB),
    makeEntry(TWAITFORPAUSEACKS_Timeout,scriptSetReal32CB),
    makeEntry(TimedOutWaitingForPauseAcksGiveUpAfterNumTimes,scriptSetSdwordCB),
    makeEntry(HorseRacePlayerDropoutTime,scriptSetReal32CB),
    makeEntry(HorseRaceDropoutColor,scriptSetRGBCB),
    makeEntry(ROUTING_SERVER_NAME,scriptSetStringCB),
    makeEntry(PATCHNAME,scriptSetStringCB),
    makeEntry(LAN_ADVERTISE_USER_TIME,scriptSetReal32CB),
    makeEntry(LAN_ADVERTISE_USER_TIMEOUT,scriptSetReal32CB),
    makeEntry(LAN_ADVERTISE_GAME_TIME,scriptSetReal32CB),
    makeEntry(LAN_ADVERTISE_GAME_TIMEOUT,scriptSetReal32CB),
    makeEntry(AD_PORT,scriptSetUdwordCB),
    makeEntry(CONNECT_TIMEOUT,scriptSetSdwordCB),
    makeEntry(KEEPALIVE_SEND_IAMALIVE_TIME,scriptSetReal32CB),
    makeEntry(KEEPALIVE_IAMALIVE_TIMEOUT,scriptSetReal32CB),
    makeEntry(KEEPALIVE_SEND_ALIVESTATUS_TIME,scriptSetReal32CB),
    makeEntry(PRINTLAG_MINFRAMES,scriptSetUdwordCB),
    makeEntry(PRINTLAG_IFGREATERTHAN,scriptSetUdwordCB),
    makeEntry(ROOM_MIN_THRESHOLD,scriptSetUdwordCB),
    makeEntry(ROOM_MAX_THRESHOLD,scriptSetUdwordCB),
    makeEntry(WAIT_SHUTDOWN_MS,scriptSetUdwordCB),
    endEntry
};

/*=============================================================================
    Functions:
=============================================================================*/

void scriptSetPortNumbers(char *directory,char *field,void *dataToFillIn)
{
    char *param;
    sdword i;
    udword *ptr = (udword *)dataToFillIn;

    for (i=0;i<MAX_PORTS;i++)
    {
        param = strtok(field, ", \t");
        field = NULL;
        if (param)
        {
            sscanf(param,"%d",&ptr[i]);
        }
        else
        {
            break;
        }
    }
}

void scriptSetIPStrings(char *directory,char *field,void *dataToFillIn)
{
    char *param;
    sdword i;
    ipString *ipStrings = (ipString *)dataToFillIn;

    for (i=0;i<MAX_IPS;i++)
    {
        param = strtok(field, ", \t");
        field = NULL;
        if (param)
        {
            sscanf(param,"%s",ipStrings[i]);
        }
        else
        {
            break;
        }
    }
}

void ResetChannel(void)
{
    CurrentChannel[0] = 0;
    CurrentChannelDescription[0] = 0;
}

void SetChannel(wchar_t *channel, wchar_t *description)
{
#ifndef _MACOSX_FIX_ME
    dbgAssert(wcslen(channel) <= MAX_CHANNEL_NAME_LEN);
    wcscpy(CurrentChannel,channel);
    dbgAssert(wcslen(description) <= MAX_CHANNEL_DESCRIPTION_LEN);
    wcscpy(CurrentChannelDescription,description);
#endif
}

wchar_t *GetCurrentChannel(void)
{
    return CurrentChannel;
}

wchar_t *GetCurrentChannelDescription(void)
{
    return CurrentChannelDescription;
}

/*-----------------------------------------------------------------------------
    Name        : tpLockChannelList
    Description : Locks the tpChannelList for exclusive access
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tpLockChannelList()
{
    int result = SDL_mutexP(tpChannelList.mutex);
    dbgAssert(result != -1);
}

/*-----------------------------------------------------------------------------
    Name        : tpUnLockChannelList
    Description : Unlocks the tpChannelList, so other tasks can access it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tpUnLockChannelList()
{
    int result = SDL_mutexV(tpChannelList.mutex);
    dbgAssert(result != -1);
}

/*-----------------------------------------------------------------------------
    Name        : tpLockServerList
    Description : Locks the tpServerList for exclusive access
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tpLockServerList()
{
    int result = SDL_mutexP(tpServerList.mutex);
    dbgAssert(result != -1);
}

/*-----------------------------------------------------------------------------
    Name        : tpUnLockServerList
    Description : Unlocks the tpServerList, so other tasks can access it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tpUnLockServerList()
{
    int result = SDL_mutexV(tpServerList.mutex);
    dbgAssert(result != -1);
}

/*-----------------------------------------------------------------------------
    Name        : titanGameStartup
    Description : initializes the titan game networking interface variables.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanGameStartup(void)
{
    tpChannelList.numChannels = 0;
    tpChannelList.newDataArrived = FALSE;
    tpChannelList.mutex = SDL_CreateMutex();
    dbgAssert(tpChannelList.mutex != NULL);
    tpServerList.numServers = 0;
    tpServerList.newDataArrived = FALSE;
    tpServerList.mutex = SDL_CreateMutex();
    dbgAssert(tpServerList.mutex != NULL);

    titanResetGameCreated();
}

/*-----------------------------------------------------------------------------
    Name        : titanResetGameCreated
    Description : resets the tpGameCreated Structures
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanResetGameCreated(void)
{
    mgResetNamePassword();
    tpGameCreated.MapName[0]        = 0;
    tpGameCreated.DisplayMapName[0] = 0;
    tpGameCreated.numPlayers        = 1;
    tpGameCreated.userBehindFirewall = 0;
}

/*-----------------------------------------------------------------------------
    Name        : titanGameShutdown
    Description : shuts down the titat game interface code and data.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanGameShutdown(void)
{
    SDL_DestroyMutex(tpChannelList.mutex);
    SDL_DestroyMutex(tpServerList.mutex);
    tpChannelList.mutex = NULL;
    tpServerList.mutex = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : titanGameEnded
    Description : Function to be called when titan game ended
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void titanGameEnded(void)
{
#ifndef _MACOSX_FIX_ME
    signed int IWasCaptain = IAmCaptain;

    mgGameInterestedOff();
    titanLeaveGameNotify();

    if (IWasCaptain)
    {
        if (!LANGame)
        {
            wcscpy(RemoveGameRequest,tpGameCreated.Name);   // don't call titanRemoveGame directly because you must be connected
                                                            // to the room before you call it
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : titanGameDisolved
    Description : Disolves a game created by a captain.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanGameDisolved(bool shutdown)
{
    if (shutdown)
    {
        if (!LANGame)
        {
            titanRemoveGame(tpGameCreated.Name);
        }

        titanStartShutdown(TITANMSGTYPE_GAMEDISOLVED, NULL, 0);
    }
    else
    {
        if (!LANGame)
        {
            titanRemoveGame(tpGameCreated.Name);
        }

        titanBroadcastPacket(TITANMSGTYPE_GAMEDISOLVED, NULL, 0);
    }

    mgGameInterestedOff();
    titanLeaveGameNotify();
}

/*-----------------------------------------------------------------------------
    Name        : titanLeaveGame
    Description : Sends a leave game request to the captain.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanLeaveGame(int shutdown)
{
    if (shutdown)
    {
        titanStartShutdown(TITANMSGTYPE_LEAVEGAMEREQUEST,NULL,0);
    }
    else
    {
        titanSendPacketTo(&tpGameCreated.playerInfo[0].address,TITANMSGTYPE_LEAVEGAMEREQUEST,NULL,0);
    }

    mgGameInterestedOff();
    titanLeaveGameNotify();
}

/*-----------------------------------------------------------------------------
    Name        : titanNoClientsCB
    Description : Called when all quees are closed.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanNoClientsCB(void)
{
    TitanReadyToShutdown = TRUE;
}

unsigned long titanConfirmReceivedCB(Address *address,const void *blob,unsigned short bloblen)
{
    dbgAssert(bloblen == 0);

    if (LANGame)
        lgJoinGameConfirmed();
    else
        mgJoinGameConfirmed();

    return TRUE;
}

unsigned long titanRejectReceivedCB(Address *address,const void *blob,unsigned short bloblen)
{
    dbgAssert(bloblen == 0);

    if (LANGame)
        lgJoinGameDenied();
    else
        mgJoinGameDenied();

    return TRUE;
}

void titanUpdateGameDataCB(const void *blob,unsigned short bloblen)
{
    dbgAssert(bloblen == sizeof(CaptainGameInfo));

    mgBackuptpGameCreated();
    memcpy(&tpGameCreated,blob,bloblen);

    if (LANGame)
        lgUpdateGameInfo();
    else
        mgUpdateGameInfo();
}

bool titanKickPlayer(udword i)
{
#ifndef _MACOSX_FIX_ME
    unsigned long j;
    DirectoryCustomInfoMax buildDirectoryCustomInfo;

    if (!GameCreator)
    {
        return FALSE;
    }

    if (i < tpGameCreated.numPlayers)
    {
        titanSendPacketTo(&(tpGameCreated.playerInfo[i].address), TITANMSGTYPE_KICKPLAYER, NULL, 0);

        if (i+1>=tpGameCreated.numPlayers)
        {
            tpGameCreated.numPlayers--;
        }
        else
        {
            tpGameCreated.numPlayers--;
            for (j=i;j<tpGameCreated.numPlayers;j++)
            {
                tpGameCreated.playerInfo[j] = tpGameCreated.playerInfo[j+1];
            }
        }

        if (!LANGame)
        {
            generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
            titanReplaceGameInfo(tpGameCreated.Name,(DirectoryCustomInfo *)&buildDirectoryCustomInfo, FALSE);
        }
        else
        {
            generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
            wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);
        }

        sigsPlayerIndex = 0;
        sigsNumPlayers = tpGameCreated.numPlayers;

        titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));

        if (LANGame)
            lgUpdateGameInfo();
        else
            mgUpdateGameInfo();

        return TRUE;

    }
    else
    {
        return FALSE;
    }
#endif
	return FALSE;
}

unsigned long titanLeaveGameReceivedCB(Address *address,const void *blob,unsigned short bloblen)
{
#ifndef _MACOSX_FIX_ME
    unsigned long i,j;
    bool       found=FALSE;
    DirectoryCustomInfoMax buildDirectoryCustomInfo;

    if (!GameCreator)
    {
        return FALSE;
    }

    dbgAssert(bloblen == 0);

    // search for the player who has left the game
    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,*address))
        {
            found = TRUE;
            break;
        }
    }

    if (found)
    {
        if (i+1>=tpGameCreated.numPlayers)
        {
            tpGameCreated.numPlayers--;
        }
        else
        {
            tpGameCreated.numPlayers--;
            for (j=i;j<tpGameCreated.numPlayers;j++)
            {
                tpGameCreated.playerInfo[j] = tpGameCreated.playerInfo[j+1];
            }
        }

        if (!LANGame)
        {
            generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
            titanReplaceGameInfo(tpGameCreated.Name,(DirectoryCustomInfo *)&buildDirectoryCustomInfo, FALSE);
        }
        else
        {
            generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
            wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);
        }

        sigsPlayerIndex = 0;
        sigsNumPlayers = tpGameCreated.numPlayers;

        titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));

        if (LANGame)
            lgUpdateGameInfo();
        else
            mgUpdateGameInfo();

        return TRUE;
    }
#endif // _MACOSX_FIX_ME
    return FALSE;
}

void generateDirectoryCustomInfo(DirectoryCustomInfoMax *buildDirectoryCustomInfo)
{
    sdword passwordnamelen;
    sdword mapnamelen = strlen(tpGameCreated.DisplayMapName) + 1;
    int n;

    dbgAssert(mapnamelen <= MAX_MAPNAME_LEN);
#ifndef _MACOSX_FIX_ME
    if (bitTest(tpGameCreated.flag,MG_PasswordProtected))
    {
        wcscpy(buildDirectoryCustomInfo->stringdata,tpGameCreated.Password);
        passwordnamelen = wcslen(tpGameCreated.Password) + 1;
    }
    else
    {
        wcscpy(buildDirectoryCustomInfo->stringdata,L"");
        passwordnamelen = wcslen(L"") + 1;
    }
#endif
    dbgAssert(passwordnamelen <= MAX_PASSWORD_LEN);

    mbstowcs(buildDirectoryCustomInfo->stringdata + passwordnamelen, tpGameCreated.DisplayMapName, mapnamelen);

    buildDirectoryCustomInfo->stringdatalength = (mapnamelen + passwordnamelen) * 2;

    dbgAssert(tpGameCreated.numPlayers > 0);

    buildDirectoryCustomInfo->userBehindFirewall = 0;
    for(n=0; n<tpGameCreated.numPlayers; n++) {
        if(tpGameCreated.playerInfo[n].behindFirewall) {
            buildDirectoryCustomInfo->userBehindFirewall = 1;
            break;
        }
    }

    tpGameCreated.userBehindFirewall = buildDirectoryCustomInfo->userBehindFirewall;

    buildDirectoryCustomInfo->numPlayers = tpGameCreated.numPlayers;
    buildDirectoryCustomInfo->flag = tpGameCreated.flag;

    buildDirectoryCustomInfo->captainAddress = myAddress;
    buildDirectoryCustomInfo->pingAddress = titanGetMyPingAddress();
    memcpy(buildDirectoryCustomInfo->sessionKey,titanGetGameKey(),GAMEKEY_SIZE);
    memcpy(buildDirectoryCustomInfo->versionInfo,networkVersion,MAX_NETWORKVERSION_STRING_LEN);
}

signed long titanRequestReceivedCB(Address *address,const void *blob,unsigned short bloblen)
{
#ifndef _MACOSX_FIX_ME
    sdword i;
    DirectoryCustomInfoMax buildDirectoryCustomInfo;
    const PlayerJoinInfo* pInfo;

    if (AddressesAreEqual(*address,myAddress))
    {
        return REQUEST_RECV_CB_JUSTDENY; // can't join your own game
    }

    // reject if got multiple join requests by the same person
    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,*address))
        {
            return REQUEST_RECV_CB_JUSTDENY;       // already received this one, so return
        }
    }

    if ((tpGameCreated.numPlayers >= MAX_MULTIPLAYER_PLAYERS) || (tpGameCreated.flag & GAME_IN_PROGRESS))
    {
        return REQUEST_RECV_CB_DENYANDCLOSE;
    }

    if ((mgPlayerLimit) && (tpGameCreated.numPlayers >= mgPlayerLimit))
    {
        return REQUEST_RECV_CB_DENYANDCLOSE;
    }

    if (tpGameCreated.numPlayers + tpGameCreated.numComputers >= spScenarios[spCurrentSelected].maxplayers)
    {
        if (tpGameCreated.numPlayers >= spScenarios[spCurrentSelected].maxplayers)
        {
            return REQUEST_RECV_CB_DENYANDCLOSE; // too many people for this map
        }
        else
        {
            // let's bump computers out for humans to join
            tpGameCreated.numComputers = spScenarios[spCurrentSelected].maxplayers - tpGameCreated.numPlayers - 1;
            dbgAssert(tpGameCreated.numComputers >= 0);
            dbgAssert(tpGameCreated.numPlayers + tpGameCreated.numComputers < spScenarios[spCurrentSelected].maxplayers);
        }
    }

    pInfo = (PlayerJoinInfo*)blob;

    if (mgIsUserBanned((char *)pInfo->PersonalName))
    {
        return REQUEST_RECV_CB_JUSTDENY;
    }

    // update directory server
    dbgAssert(bloblen == sizeof(PlayerJoinInfo));

    tpGameCreated.playerInfo[tpGameCreated.numPlayers].address = *address;
    tpGameCreated.playerInfo[tpGameCreated.numPlayers].playerIndex = tpGameCreated.numPlayers;
    tpGameCreated.playerInfo[tpGameCreated.numPlayers].colorsPicked = pInfo->colorsPicked;
    if (!pInfo->colorsPicked)
    {
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].baseColor = teColorSchemes[tpGameCreated.numPlayers].textureColor.base;
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].stripeColor = teColorSchemes[tpGameCreated.numPlayers].textureColor.detail;
    }
    else
    {
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].baseColor = pInfo->baseColor;
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].stripeColor = pInfo->stripeColor;
    }
    tpGameCreated.playerInfo[tpGameCreated.numPlayers].race = pInfo->race;
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].behindFirewall = pInfo->behindFirewall;
    strcpy(tpGameCreated.playerInfo[tpGameCreated.numPlayers].PersonalName, pInfo->PersonalName);
    tpGameCreated.numPlayers++;

    if (!LANGame)
    {
        generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
        titanReplaceGameInfo(tpGameCreated.Name,(DirectoryCustomInfo *)&buildDirectoryCustomInfo, FALSE);
        mgUpdateGameInfo();
    }
    else
    {
        generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
        wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);
        lgUpdateGameInfo();
    }
#endif // _MACOSX_FIX_ME
    return REQUEST_RECV_CB_ACCEPT;
}

void titanGameMsgReceivedCB(const void *blob,unsigned short bloblen)
{
    ReceivedPacketCB((ubyte *)blob,(udword)bloblen);
}

void titanSendBroadcastMessage(ubyte *packet,udword sizeofPacket)
{
#ifdef DEBUG_GAME_BW
    titanDebug("bw: broadcast gamepkt %x size %d",((HWPacketHeader *)packet)->type,sizeofPacket);
#endif
    titanBroadcastPacket(TITANMSGTYPE_GAME,packet,(uword)sizeofPacket);     // Fix later for non-captain broadcasting stuff
}

void titanSendPointMessage(int playerIndex,ubyte *packet,udword sizeofPacket)
{
#ifdef DEBUG_GAME_BW
    titanDebug("bw: pointmsg gamepkt %x size %d",((HWPacketHeader *)packet)->type,sizeofPacket);
#endif
    titanSendPacketTo(&tpGameCreated.playerInfo[playerIndex].address,TITANMSGTYPE_GAME,packet,(uword)sizeofPacket);
}

void titanAnyoneSendBroadcastMessage(ubyte *packet,udword sizeofPacket)
{
#ifdef DEBUG_GAME_BW
    titanDebug("bw: broadcastany gamepkt %x size %d",((HWPacketHeader *)packet)->type,sizeofPacket);
#endif
    titanAnyoneBroadcastPacket(TITANMSGTYPE_GAME,packet,(uword)sizeofPacket);     // Fix later for non-captain broadcasting stuff
}

void titanAnyoneSendPointMessage(int playerIndex,ubyte *packet,udword sizeofPacket)
{
#ifdef DEBUG_GAME_BW
    titanDebug("bw: pointmsgany gamepkt %x size %d",((HWPacketHeader *)packet)->type,sizeofPacket);
#endif
    titanAnyoneSendPacketTo(&tpGameCreated.playerInfo[playerIndex].address,TITANMSGTYPE_GAME,packet,(uword)sizeofPacket);
}

void captainGameStartCB(void)
{
    sigsPlayerIndex = 0;
    sigsNumPlayers = tpGameCreated.numPlayers;
    sigsPressedStartGame = TRUE;

    if (LANGame)
        lgShutdownMultiPlayerGameScreens();
    else
        mgShutdownMultiPlayerGameScreens();
}

void titanUpdatePlayer(bool captain)
{
    PlayerJoinInfo pInfo;

    if (captain)
    {
        tpGameCreated.playerInfo[0].baseColor = utyBaseColor;
        tpGameCreated.playerInfo[0].stripeColor = utyStripeColor;
        tpGameCreated.playerInfo[0].colorsPicked = cpColorsPicked;
        tpGameCreated.playerInfo[0].race = whichRaceSelected;

        titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));

        if (LANGame)
            lgUpdateGameInfo();
        else
            mgUpdateGameInfo();
    }
    else
    {
        pInfo.baseColor = utyBaseColor;
        pInfo.stripeColor = utyStripeColor;
        pInfo.colorsPicked = cpColorsPicked;
        pInfo.race = whichRaceSelected;

        titanSendPacketTo(&tpGameCreated.playerInfo[0].address,TITANMSGTYPE_UPDATEPLAYER,&pInfo,sizeof(pInfo));
    }
}

void titanUpdatePlayerCB(Address *address, const void *blob, unsigned short bloblen)
{
    udword i;
    PlayerJoinInfo *pinfo=(PlayerJoinInfo *)blob;

    dbgAssert(bloblen == sizeof(PlayerJoinInfo));

    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,*address))
        {
            tpGameCreated.playerInfo[i].baseColor   = pinfo->baseColor;
            tpGameCreated.playerInfo[i].stripeColor = pinfo->stripeColor;
            tpGameCreated.playerInfo[i].colorsPicked = pinfo->colorsPicked;
            tpGameCreated.playerInfo[i].race        = pinfo->race;

            titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));

            if (LANGame)
                lgUpdateGameInfo();
            else
                mgUpdateGameInfo();
        }
    }
}

void PrintAddressToString(char *buffer,Address *address)
{
    if (!IPGame)
    {
        sprintf(buffer,"(Ether:%x,%x,%x,%x,%x,%x Port:%x)",address->AddrPart.etherAddr[0],address->AddrPart.etherAddr[1],
                address->AddrPart.etherAddr[2],address->AddrPart.etherAddr[3],address->AddrPart.etherAddr[4],address->AddrPart.etherAddr[5],
                address->Port);
    }
    else
    {
        sprintf(buffer,"(IP:%lx Port:%d)",address->AddrPart.IP,address->Port);
    }
}

int titanLoadPublicKey(char *filename,unsigned char** buffer)
{
    sdword fileLength = fileSizeGet(filename, 0);
    (*buffer) = (unsigned char*)malloc(fileLength);
    return fileLoad(filename, *buffer, 0);
}

void titanResetValidVersions()
{
    HaveValidVersions = FALSE;
    ValidVersions[0] = 0;
}

/*-----------------------------------------------------------------------------
    Name        : titanGotValidVersionStrings
    Description : callback for when titan gets valid version strings (one string, multiple version strings seperated by tabs)
    Inputs      : validversions
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void titanGotValidVersionStrings(char *vversions)
{
    strcpy(ValidVersions,vversions);
    HaveValidVersions = TRUE;
}

bool VersionFoundInValidVersions(char *myversion)
{
    char copyValidVersions[VALIDVERSIONS_MAXLEN];
    char *strtokptr;
    char *ptr;

    strcpy(copyValidVersions,ValidVersions);        // have to make a copy since strtok converts delimiters to NULLs
    strtokptr = copyValidVersions;

    for (;;)
    {
        ptr = strtok(strtokptr, "\r\n");      // only use "\n", "\t" is contained in the version strings to sepearte language, etc.
        strtokptr = NULL;
        if (!ptr)
            return FALSE;

        if (strcasecmp(ptr,myversion) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

bool titanCheckValidVersion(char *myversion)
{
    if (ShortCircuitWON) return(TRUE);

    if (!HaveValidVersions)
    {
        return TRUE;            // assume we have valid version if we don't have any information yet
    }

    if (VersionFoundInValidVersions(myversion))
    {
        return TRUE;
    }

    return FALSE;
}

bool CheckNetworkVersionCompatibility(char *netversion)
{
    if (strcasecmp(netversion,networkVersion) == 0)
        return TRUE;
    else
        return FALSE;
}

#if 0
void titanCDKeyNotFoundCB(void)
{
    mgDisplayMessage(strGetString(strNoCDKey));
    return;
}

void titanInvalidCDKeyCB(void)
{
    mgDisplayMessage(strGetString(strLightweightBadKey));
    return;
}
#endif

