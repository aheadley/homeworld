// TitanInterface

#ifdef _WIN32
#include <process.h>
#include <winsock.h>
#include <crtdbg.h>
#endif

#include <sys/stat.h>
#include "common/won.h"
#include "common/DataObject.h"
#include "common/Threadbase.h"
#include "common/WONException.h"
#include "db/dbconstants.h"
#include "msg/TMessage.h"
#include "msg/MServiceTypes.h"
#include "msg/SServiceTypes.h"
#include "msg/TServiceTypes.h"
#include "msg/comm/MMsgTypesComm.h"
#include "msg/comm/MMsgCommPing.h"
#include "msg/comm/MMsgCommGetNumUsers.h"
#include "msg/comm/SMsgTypesComm.h"
#include "msg/comm/SMsgCommStatusReply.h"
#include "msg/comm/TMsgTypesComm.h"
#include "msg/comm/TMsgCommStatusReply.h"
#include "msg/dir/DirG2Flags.h"
#include "msg/dir/SMsgTypesDir.h"
#include "msg/dir/SMsgDirG2GetEntity.h"
#include "msg/dir/SMsgDirG2MultiEntityReply.h"
#include "msg/event/SMsgEventTaggedRecordEvent.h"
#include "msg/fact/SMsgTypesFact.h"
#include "msg/fact/SMsgFactStatusReply.h"
#include "msg/fact/SMsgFactStartProcessUnicode.h"
#include "msg/firewall/SMsgTypesFirewall.h"
#include "msg/firewall/SMsgFirewallDetect.h"
#include "msg/firewall/SMsgFirewallStatusReply.h"
#include "msg/auth/TMsgTypesAuth.h"
#include "msg/auth/TMsgAuth1GetPubKeys.h"
#include "msg/auth/TMsgAuth1LoginHW.h"
#include "msg/auth/TMsgAuth1LoginReply.h"
#include "msg/auth/TMsgAuth1PeerToPeer.h"
#include "msg/routing/MMsgTypesRouting.h"
#include "msg/routing/MMsgRoutingCreateDataObject.h"
#include "msg/routing/MMsgRoutingDeleteDataObject.h"
#include "msg/routing/MMsgRoutingDisconnectClient.h"
#include "msg/routing/MMsgRoutingGetClientList.h"
#include "msg/routing/MMsgRoutingGroupChange.h"
#include "msg/routing/MMsgRoutingPeerChat.h"
#include "msg/routing/MMsgRoutingPeerData.h"
#include "msg/routing/MMsgRoutingPeerDataMultiple.h"
#include "msg/routing/MMsgRoutingRegisterClient.h"
#include "msg/routing/MMsgRoutingReadDataObjectReply.h"
#include "msg/routing/MMsgRoutingRenewDataObject.h"
#include "msg/routing/MMsgRoutingReplaceDataObject.h"
#include "msg/routing/MMsgRoutingStatusReply.h"
#include "msg/routing/MMsgRoutingSendChat.h"
#include "msg/routing/MMsgRoutingSendData.h"
#include "msg/routing/MMsgRoutingSendDataBroadcast.h"
#include "msg/routing/MMsgRoutingSubscribeDataObject.h"
#include "msg/routing/MMsgRoutingReconnectClient.h"
#include "TitanPacketMsg.h"
#include "EasySocket/EasySocketEngine.h"
#include "EasySocket/EasySocket.h"
#include "EasySocket/SocketPipe.h"
#include "EasySocket/PipeCmd.h"
#include "TitanInterface.h"
#include "ClientCDKey.h"
#include "wassert.h"

#define DEBUG_DISCONNECT    0
#define CLOCK_COMMANDS_THRU_ENGINE
#define LOG_MESSAGE_SIZE

extern "C"
{
    #include "TitanInterfaceC.h"
    #include "Titan.h"
    #include "StringsOnly.h"
    extern char filePrependPath[];
    void mgStartSpecificFactServerFailedCB(void);
    void mgStartGame(char*name,void*atom);
    void mgStartGameCB(void);
    void mgDisplayMsgBox(void);
    void mgDisplayMessage(char *message);
    void clCommandMessage(char *msg);
    void LockMutex(void *mutex);
    void UnLockMutex(void *mutex);
}



// Private namespace for using and constants
namespace {
    using namespace WONDatabase;
    using WONCommon::AutoCrit;
    using WONCommon::StringToWString;
    using WONCommon::WStringToString;
    using WONCommon::WONException;
    using WONCrypt::BFSymmetricKey;
    using WONCrypt::EGPublicKey;
    using WONCrypt::EGPrivateKey;
    using WONCrypt::CryptKeyBase;
    using WONAuth::Auth1PublicKeyBlock;
    using WONAuth::Auth1Certificate;
    using WONMsg::TRawMsg;
    using WONMsg::TMessage;
    using WONMsg::BaseMessage;
    using WONMsg::MiniMessage;
    using WONMsg::SmallMessage;
    using WONMsg::MMsgRoutingPeerDataMultiple::PeerDataMessage;
    using WONMsg::MMsgRoutingReadDataObjectReply::DataObjectWithIds;
    using WONMisc::EasySocketEngine;
    using WONMisc::EasySocket;
    using WONMisc::ES_ErrorType;
    using WONMisc::SocketPipe;
    using WONMisc::PipeCmd;
    using WONMisc::OpenCmd;
    using WONMisc::BindCmd;
    using WONMisc::CloseCmd;
    using WONMisc::CloseNowCmd;
    using WONMisc::SmartConnectCmd;
    using WONMisc::ListenCmd;
    using WONMisc::AcceptCmd;
    using WONMisc::RecvPrefCmd;
    using WONMisc::RecvFromCmd;
    using WONMisc::SendCmd;
    using WONMisc::SendToCmd;
    using WONMisc::BroadcastCmd;
    using WONMisc::SetEventCmd;
    using WONMisc::WaitForEventCmd;
    using WONMisc::TimerCmd;
    using WONMisc::NoOpPayloadCmd;

    // Constants
    const wchar_t* HOMEWORLD_DIR   = L"/Homeworld"; // Dir Server directory for Homeworld rooms
    const wchar_t* HWDS_DIR        = L"HWDS"; // sub-dir of Homeworld where HWDS's register
    const wchar_t* TITANSERVER_DIR = L"/TitanServers"; // Parent directory of dirs holding auth, firewall, and event servers.
    const wchar_t* AUTH_SERV     = L"AuthServer"; // Service name for auth servers
    const wchar_t* CHATROOM_SERV = L"TitanRoutingServer"; // Service name for chat rooms (chat Routing Servers)
    const wchar_t* FACTORY_SERV  = L"TitanFactoryServer"; // Service name for factory servers
    const wchar_t* FIREWALL_SERV = L"TitanFirewallDetector"; // Service name for firewall servers
    const wchar_t* EVENT_SERV    = L"TitanEventServer"; // Service name for event servers
    const WONCommon::RawBuffer VALIDVERSIONS_OBJ(reinterpret_cast<unsigned char*>("HomeworldValidVersions")); // Data object that contains valid Homeworld version strings    (attached to TitanServers dir)
    const WONCommon::RawBuffer DESCRIPTION_OBJ(reinterpret_cast<unsigned char*>("Description")); // Data object that contains the HWDS server/chat room's description         (attached to Routing Server entries in Homeworld dir & Factory Server entries in the HWDS dir)
    const WONCommon::RawBuffer ROOM_FLAGS_OBJ(reinterpret_cast<unsigned char*>("RoomFlags")); // Data object that contains the chat room flags                                (attached to Routing Server entries in Homeworld dir)
    const WONCommon::RawBuffer ROOM_CLIENTCOUNT_OBJ(reinterpret_cast<unsigned char*>("__RSClientCount")); // Data object that contains some indication of a chat room's fullness (attached to Routing Server entries in Homeworld dir)
    const WONCommon::RawBuffer FACT_CUR_SERVER_COUNT_OBJ(reinterpret_cast<unsigned char*>("__FactCur_RoutingServHWGame")); // Data object that contains the current number of game servers currently running on a particular machine/Factory Server (attached to Factory Server entries in HWDS dir)
    const WONCommon::RawBuffer FACT_TOTAL_SERVER_COUNT_OBJ(reinterpret_cast<unsigned char*>("__FactTotal_RoutingServHWGame")); // Data object that contains the total number of game servers that have been run by a particular machine/Factory Server (attached to Factory Server entries in HWDS dir)
    const WONCommon::RawBuffer SERVER_UPTIME_OBJ(reinterpret_cast<unsigned char*>("__ServerUptime")); // Data object that contains the number of seconds that a particular server has been up (attached to Factory Server entries in HWDS dir)
    const unsigned char* gameTag = reinterpret_cast<unsigned char*>("HW"); // tag prepended to game names when creating Routing Server data types (game objects)
    const unsigned char* keyTag  = reinterpret_cast<unsigned char*>("HK"); // tag prepended to game names when creating Routing Server data types (symmetric keys)
    const char* ROUTINGSERV_CHAT = "RoutingServHWChat";
    const char* ROUTINGSERV_GAME = "RoutingServHWGame";
    const char* MEDIAMETRIX_URL = "http://homeworld.won.net";

    const int   PREPOST_MAX = 10;

    const unsigned short CHAT_GROUP = 4;

    char *VERIFIER_KEY_FILE_NAME = "kver.kp";       // took out const so C won't bitch
    const char *LOGIN_KEY_FILE_NAME = "login.ks";

    const unsigned char  LAN_GAMEKEY[GAMEKEY_SIZE] = {
        0x23, 0xFD, 0x77, 0xAB, 0x69, 0x17, 0x99, 0xCE
    };

    const unsigned long NUM_EVENTS_TO_SEND = 3; // number of TaggedRecordEvent messages to send each time we try to record an event
    const unsigned long TIME_BETWEEN_EVENTS = 20; // milliseconds
};

// Global Instance of TitanInterface (allocated by titanStart)
TitanInterface* titanInterface = NULL;

extern "C"
{
    extern unsigned long multiPlayerGame;
    extern unsigned long TITAN_GAME_EXPIRE_TIME;
    extern unsigned long TITAN_CHANNEL_EXPIRE_TIME;
    extern unsigned long GAME_PORT;
    extern unsigned long AD_PORT;
    extern char ROUTING_SERVER_NAME[];
    extern char utyName[];
    extern HWND ghMainWindow;
    extern char versionString[];
    extern unsigned long strCurLanguage;
    extern unsigned long ShortCircuitWON;
    extern signed long numPendingIn;
    extern signed long numPendingOut;
}


/*=============================================================================
    Titan C code wrapper
=============================================================================*/

Address TitanInterface::GetMyPingAddress(void)
{
    Address myPingAddress;

    if (mBehindFirewall)
    {
        long IP = mRoutingAddress[0].sin_addr.s_addr;
        short int Port = ntohs(mRoutingAddress[0].sin_port);
        CreateInternetAddress(myPingAddress,IP,Port);
    }
    else
    {
        myPingAddress = mMyIPAddress;
    }

    return myPingAddress;
}

Address titanGetMyPingAddress(void)
{
    return titanInterface->GetMyPingAddress();
}
/*
void titanQueryRoutingServers(void) {
    titanInterface->QueryRoutingServers();
}
*/
void titanLeaveGameNotify(void) {
    captainIndex = -1;
    if (IPGame)
    {
    titanInterface->LeaveGameNotify();
    }
}

unsigned long titanReadyToStartGame(unsigned char *routingaddress) {
    return titanInterface->CheckStartingGame(routingaddress) ? 1 : 0;
}


void TitanInterface::LeaveGameNotify(void)
{
    captainIndex = -1;

    // If the player is leaving a game that has started, record an event indicating
    // that the player has exitted the game
    if (mGameCreationState == GAME_STARTED)
    {
        mGameDisconnectWasVoluntary = true;
        RecordEvent(ACTIVITYTYPE_HOMEWORLD_GAME_EXIT);
    }

    if(!mUseOldScheme) {
        mUseRoutingServer = true;
        mRoutingReconnectNum[0] = 0;
        mRoutingReconnectNum[1] = 0;

        CloseRoutingServerConnection(1);
        mGameCreationState = GAME_NOT_STARTED;

        myAddress.AddrPart.IP = mMyClientId[0];
        myAddress.Port = mMyClientId[0];
    }

    if (IPGame)     // don't close old client connections if IPX Lan
    {
    // Close all old client connections.
    AutoCrit aCrit(mClientCrit);
    ClientToPipe::iterator anItr = mClientMap.begin();
    while(anItr!=mClientMap.end())
    {
        SocketPipe *aPipe = anItr->second.pipe;
        if(aPipe!=NULL)
            aPipe->AddOutgoingCmd(new CloseCmd(false, false, 0, false));
        anItr++;
    }

    mClientMap.clear();
    }
}

void TitanInterface::ConnectingCancelHit()
{
    if (mGameCreationState == GAME_STARTING)
    {
        mGameCreationState = GAME_NOT_STARTED;
    }
}

void titanConnectingCancelHit(void)
{
    titanInterface->ConnectingCancelHit();
}

bool TitanInterface::CheckStartingGame(unsigned char *routingaddress) {
    if(mIsLan || mUseOldScheme || tpGameCreated.numPlayers==1) {
        mGameCreationState = GAME_STARTED;
        InitPacketList();
        //CloseRoutingServerConnection(0);
        return true;
    }


    if(mGameCreationState==GAME_NOT_STARTED) {
        mgDisplayMsgBox();
        mgDisplayMessage(strGetString(strStartingGame));

        mGameCreationState = GAME_STARTING;
        mOldtpGameCreated = tpGameCreated;

        mNumPlayersJoined = 1;

        if(tpGameCreated.userBehindFirewall) {
            mgDisplayMessage(strGetString(strDetectedUserBehindFirewall));
            mgDisplayMessage(strGetString(strStartingRoutingServer));

            mUseRoutingServer = true;
            StartRoutingServer(L"",L"",L"",true,routingaddress);
        }
        else {
            mgDisplayMessage(strGetString(strTellingPlayersToConnectToMe));

            BeginStartGamePacket aPacket;
            aPacket.routingAddress = mMyIPAddress;
            aPacket.oldCaptainAddress = myAddress;
            aPacket.newCaptainAddress = mMyIPAddress;

            titanBroadcastPacket(TITANMSGTYPE_BEGINSTARTGAME, &aPacket, sizeof(aPacket));
            OnCaptainStartedGame();

            ChangeAddress(&(aPacket.oldCaptainAddress), &(aPacket.newCaptainAddress));
            myAddress = mMyIPAddress;

            mUseRoutingServer = false;
        }

        return false;
    }
    else if(mGameCreationState==GAME_STARTING)
        return false;
    else
        return true;
}

unsigned long titanBehindFirewall(void) {
    return titanInterface->BehindFirewall() ? 1 : 0;
}

bool TitanInterface::BehindFirewall() {
    return mBehindFirewall;
}

void titanRefreshRequest(char* theDir)
{
    if (theDir != NULL)
    {
        return;
    }
    titanInterface->RequestDirectory();
}

void titanCreateGame(wchar_t *str, DirectoryCustomInfo* myInfo)
{
    titanInterface->RequestCreateGame(str, myInfo);
}

void titanRemoveGame(wchar_t *str)
{
    titanInterface->RequestDeleteGame(str);
}

void titanReplaceGameInfo(wchar_t *str, DirectoryCustomInfo* myInfo, unsigned long replaceTimeout)
{
    titanInterface->RequestReplaceGame(str, myInfo, replaceTimeout);
}

void titanConnectToClient(Address* address)
{
    titanInterface->ConnectToClient(address);
}

//void titanConnectToClient(unsigned long theIP, unsigned long thePort)
//{
//    titanInterface->ConnectToClient(theIP, thePort);
//}

void titanSendPing(Address* address,unsigned int pingsizebytes)
{
    titanInterface->SendPing(address,pingsizebytes);
}

//void titanSendPing(unsigned long theIP, unsigned long thePort)
//{
//    titanInterface->SendPing(theIP, thePort);
//}

int titanStartChatServer(wchar_t* thePassword)
{
    return titanInterface->StartRoutingServer(GetCurrentChannel(), GetCurrentChannelDescription(), thePassword, false, NULL);
}

void
titanSendLanBroadcast(const void* thePacket, unsigned short theLen)
{
    titanInterface->SendLanBroadcast(thePacket, theLen);
}

void
titanSendPacketTo(Address *theAddressP, unsigned char titanMsgType,
                  const void* thePacket, unsigned short theLen)
{
    titanInterface->SendPacketTo(theAddressP, titanMsgType, thePacket, theLen);
}

//void
//titanSendPacketTo(unsigned long theIP, unsigned long thePort, unsigned char titanMsgType,
//                  const void* thePacket, unsigned short theLen)
//{
//    titanInterface->SendPacketTo(theIP, thePort, titanMsgType, thePacket, theLen);
//}

void
titanBroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen)
{
    titanInterface->BroadcastPacket(titanMsgType, thePacket, theLen);
}

void
titanAnyoneSendPacketTo(Address *address, unsigned char titanMsgType,
                        const void* thePacket, unsigned short theLen)
{
    titanInterface->ConnectToClient(address);
    titanInterface->SendPacketTo(address, titanMsgType, thePacket, theLen);
}

//void
//titanAnyoneSendPacketTo(unsigned long theIP, unsigned long thePort, unsigned char titanMsgType,
//                  const void* thePacket, unsigned short theLen)
//{
//    titanInterface->ConnectToClient(theIP, thePort);
//    titanInterface->SendPacketTo(theIP, thePort, titanMsgType, thePacket, theLen);
//}

void
titanAnyoneBroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen)
{
    int i;

    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (!AddressesAreEqual(tpGameCreated.playerInfo[i].address,myAddress))
        {
            titanInterface->ConnectToClient(&(tpGameCreated.playerInfo[i].address));
            titanInterface->SendPacketTo(&(tpGameCreated.playerInfo[i].address), titanMsgType, thePacket, theLen);
        }
    }
}

// provided for temporary backward compatability
void titanStartOld(signed long isLan)
{
if ( isLan )
    titanStart(1,0);
else
    titanStart(0,1);
}

// note: assumes titanInterface not intialized
unsigned long titanCheckCanNetwork(unsigned long isLan, unsigned long isIP)
{
    if (!titanInterface)
    {
        titanInterface = new TitanInterface(isLan,isIP);
        if ( titanInterface->CanNetwork() == 0 )
        {
            delete titanInterface;
            titanInterface = NULL;
            return 0;
        }
        else
        {
            delete titanInterface;
            titanInterface = NULL;
            return 1;
        }
    }

    return 0;
}

unsigned long titanStart(unsigned long isLan, unsigned long isIP)
{
    if (!titanInterface)
    {
        titanLogFileOpen();
        titanInterface = new TitanInterface(isLan,isIP);
        if ( titanInterface->CanNetwork() == 0 )
        {
            delete titanInterface;
            titanInterface = NULL;
            return 0;
        }

        if (!isLan)
            titanInterface->loadVerifierKey();

        titanInterface->startThread();

        if (!isLan)
            // Get Auth Server list and valid versions string
            titanInterface->RequestDirectory();
    }

    multiPlayerGame = TRUE;
    TitanActive = TRUE;
    TitanReadyToShutdown = FALSE;

    ResetChannel();
    //SetChannel(L"Default",L"Default");    // room now intelligently picked

    return 1;
}

// --MikeN
// Call this method to begin shutdown of titan.  Parameters specify packet to send
// to connected client(s) (a shutdown message).  The callback titanNoClientsCB() will
// be invoked when complete.
void titanStartShutdown(unsigned long titanMsgType, const void* thePacket,
                        unsigned short theLen)
{
    if (titanInterface)
        titanInterface->StartShutdown(titanMsgType,thePacket,theLen);
}

void titanShutdown(void)
{
    if (titanInterface)
    {
        titanLogFileClose();
        titanInterface->stopThread();
        delete titanInterface;
        titanInterface = NULL;
    }

    multiPlayerGame = FALSE;
    TitanActive = FALSE;
    TitanReadyToShutdown = FALSE;
}

void titanPumpEngine()
{
    titanInterface->PumpEngine();
}

#define WAIT_SHUTDOWN_MS        1000

void titanWaitShutdown(void)
{
    long waitfor = 0;

    for (;;)
    {
        if (TitanReadyToShutdown)
        {
            titanDebug("Titan ready to shut down...shutting down");
            break;
        }

        if (TitanActive)
        {
            titanPumpEngine();
}
        Sleep(10);
        waitfor += 10;

        if (waitfor >= WAIT_SHUTDOWN_MS)
        {
            titanDebug("Titan timed out waiting for shutdown ready...shutting down");
            break;
        }
    }

    titanShutdown();
}

/*=============================================================================
    C code wrapper for chat functions:
=============================================================================*/

void chatConnect(wchar_t* thePassword)
{
    titanInterface->ConnectToRoutingServer(StringToWString(utyName), thePassword, 0);
}


void chatClose(void)
{
    if (titanInterface)
        titanInterface->CloseRoutingServerConnection(0);
}

void BroadcastChatMessage(unsigned short size, const void* chatData)
{
    titanInterface->RoutingSendChatBroadcast(size, (const unsigned char*)chatData);
}


void SendPrivateChatMessage(unsigned long* userIDList, unsigned short numUsersInList, unsigned short size, const void* chatData)
{
    titanInterface->RoutingSendChatWhisper(userIDList, numUsersInList, size, (const unsigned char*)chatData);
}

void titanSetGameKey(unsigned char *key)
{
    titanInterface->SetGameKey(key);
}

const unsigned char *titanGetGameKey(void)
{
    return titanInterface->GetGameKey();
}

int titanGetPatch(char *filename,char *saveFileName)
{
    return titanInterface->GetPatch(filename,saveFileName);
}

/*=============================================================================
    Authentication server wrapper functions.:
=============================================================================*/

void authAuthenticate(char *loginName, char *password)
{
    titanInterface->Authenticate(string(loginName), string(password), "", false);
}

void authCreateUser(char *loginName, char *password)
{
    titanInterface->Authenticate(string(loginName), string(password), "", true);
}

void authChangePassword(char *loginName, char *oldpassword, char *newpassword)
{
    titanInterface->Authenticate(string(loginName), string(oldpassword), string(newpassword), false);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

TitanInterface::TitanInterface(unsigned long isLan, unsigned long isIP) :
    ThreadBase("TitanInterface"),
    mEngine(NULL),
    mAuthPipe(NULL),
    mDirPipe(NULL),
    mFactPipe(NULL),
    mLanAdPipe(NULL),
    mTimerPipe(NULL),
    mFirewallPipe(NULL),
    mCaptainPipe(NULL),
    mClientMap(),
    mCloseRequest(false),
    mHaveReceivedInitialUserList(false),
    mClientCrit(),
    mPipeCrit(),
    mRoutingCrit(),
    mGameKey(),
    mVerifierKey(NULL),
    mPrivateKey(NULL),
    mPublicKeyBlock(NULL),
    mCertificate(NULL),
    mAuthSessionKey(NULL),
    mDirSessionKey(NULL),
    mDirClientSecret(NULL),
    mFactSessionKey(NULL),
    mFactClientSecret(NULL),
    mRoutingInfoPipe(NULL),
    mEventPipe(NULL),
    mNumAuthServersTried(0),
    mCurAuthServer(0),
    mNumDirServersTried(0),
    mCurDirServer(0),
    mNumFactServersTried(0),
    mCurFactServer(0),
    mNumFirewallServersTried(0),
    mCurFirewallServer(0),
    FACTSERVER_NUM(0),
    AUTHSERVER_NUM(0),
    FIREWALLSERVER_NUM(0),
    EVENTSERVER_NUM(0),
    mNeedToAuthenticateAfterGettingAuthDirectory(false),
    mUseRoutingServer(true),
    mUseOldScheme(false),
    mBehindFirewall(false),
    mLaunched(false),
    mGameCreationState(GAME_NOT_STARTED),
    mNumPlayersJoined(0),
    mFactPingPipe(NULL),
    mCurFactPing(-1),
    mMinPingTime(-1),
    mNumPingTrials(0),
    mSeqNum(0),
    mRoutingQueryOffset(0),
    mCaptainReconnectNum(0),
    mEventTag(0),
    mHasLobbyEnterEventBeenSent(false),
    mMediaMetrixHWND(NULL),
    mRegisterRoutingServerMsg(true /* extended version */),
    mNeedToRegisterRoutingServer(false),
    mLobbyEnterTime(0),
    mGameStartTime(0),
    mHaveConnectedToAChatServer(false),
    mIsGameServer(false),
    mGameDisconnectWasVoluntary(false),
    mFailFactOverDirectly(false)
{
    // Array initialization
    for(int n=0; n<2; n++) {
        mRoutePipe[n] = NULL;
        mRouteSessionKey[n] = NULL;
        mRouteClientSecret[n] = NULL;
        mMyClientId[n] = 0;
        mRoutingReconnect[n] = false;
        mRoutingReconnectNum[n] = 0;
        mLoggedInToRoutingServer[n] = false;
    }

    if ( isIP )
    {
        mIpType       = ip;
        mDatagramType = EasySocket::UDP;
        mStreamType   = EasySocket::TCP;
    }
    else
    {
        mIpType       = ipx;
        mDatagramType = EasySocket::IPX;
        mStreamType   = EasySocket::SPX;
    }

    if ( isLan )
    {
        mUseRoutingServer = false;
        mUseOldScheme = true;
        mIsLan = true;
        mGameKey.Create(GAMEKEY_SIZE, LAN_GAMEKEY);
    }
    else
    {
        mIsLan = false;
        mGameKey.Create(GAMEKEY_SIZE);
        mCurFactServer = time(NULL) % MAX_IPS;
        mCurFirewallServer = time(NULL) % MAX_IPS;
        mCurAuthServer = time(NULL) % MAX_IPS;
    }

    CreateMediaMetrixEditControl();

    // initialize winsock
    EasySocketEngine::startWinsock();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

TitanInterface::~TitanInterface()
{
    EasySocketEngine::stopWinsock();

    delete mVerifierKey;
    delete mPrivateKey;
    delete mPublicKeyBlock;
    delete mCertificate;
    delete mAuthSessionKey;
    delete mDirSessionKey;
    delete mDirClientSecret;
    delete mFactSessionKey;
    delete mFactClientSecret;

    for(int n=0; n<2; n++) {
        delete mRouteSessionKey[n];
        delete mRouteClientSecret[n];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::ChangeAddress(Address *theOldAddress, Address *theNewAddress)
{
    int i;

    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(mOldtpGameCreated.playerInfo[i].address,*theOldAddress))
        {
            tpGameCreated.playerInfo[i].address = *theNewAddress;
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

unsigned long TitanInterface::CanNetwork(void)
{
    SOCKET aSocket;
    bool success = false;
    int bindSuccess = false;

    if ( mIpType == ip )
        {
        aSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( aSocket != INVALID_SOCKET )
            {
            SOCKADDR_IN aSockAddrIn;
            memset((char *)&aSockAddrIn, 0, sizeof(aSockAddrIn));
            aSockAddrIn.sin_family      = AF_INET;
            aSockAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
            aSockAddrIn.sin_port        = htons(0);
            bindSuccess = bind( aSocket, (SOCKADDR*)(&aSockAddrIn), sizeof(SOCKADDR_IN) );
            if ( bindSuccess == 0 )
                success = true;
            closesocket(aSocket);
            }
        }
    else
        {
        aSocket = socket( AF_IPX, SOCK_STREAM, NSPROTO_SPX);
        if ( aSocket != INVALID_SOCKET )
            {
            SOCKADDR_IPX aSockAddrIpx;
            memset((char *)&aSockAddrIpx, 0, sizeof(aSockAddrIpx));
            aSockAddrIpx.sa_family = AF_IPX;
            aSockAddrIpx.sa_socket = htons(0); // sa_socket is the IPX equivalent of port
            bindSuccess = bind( aSocket, (SOCKADDR*)(&aSockAddrIpx), sizeof(aSockAddrIpx) );
            if ( bindSuccess == 0 )
                success = true;
            closesocket(aSocket);
            }
        }
    if ( success == true )
        return 1;
    else
        return 0;
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

int TitanInterface::GetPatch(const char *theFilename,const char *saveFileName) {
    string *anArgP;
    anArgP = new string[2];
    anArgP[0] = theFilename;
    anArgP[1] = saveFileName;

    int aHandle;
    aHandle = _beginthread(GetPatch,0,anArgP);
    if(aHandle==-1)
        titanGetPatchFailedCB(PATCHFAIL_UNABLE_TO_START_DOWNLOAD_THREAD);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::GetPatch(void *theArgs)
{
    EasySocket aSocket(EasySocket::TCP);
    ES_ErrorType anError;

        // Read in the Parameters
    string theFilename = ((string*)theArgs)[0];
    string saveFileName = ((string*)theArgs)[1];
    delete [] (string*)theArgs;


    // Construct the HTTP request
    string aRequest;
    aRequest += string("GET /") + theFilename + string(" HTTP/1.1\r\n");
    aRequest += "Host: " + string(PATCHSERVER_IPSTRINGS[0]) + "\r\n";

    char aBuf[1000];
    struct _stat anInfo;

    // Check for old file and when it was last modified
    char anOldDate[255], aNewDate[255];
    anOldDate[0] = aNewDate[0] = aNewDate[254] = '\0';
    FILE *aModFile = fopen("modified.txt","r");
    if(aModFile!=NULL) {
        fread(anOldDate,1,255,aModFile);
        fclose(aModFile);

        int aResult;
        aResult = _stat(saveFileName.c_str(),&anInfo);


        // If an old file exists and we know when it was last modified then
        // do a conditional range get
        if(aResult==0) {
            aRequest+= "If-Range: " + string(anOldDate) + "\r\n";
            aRequest+= "Range: bytes="+string(ltoa(anInfo.st_size,aBuf,10))+"-\r\n";
        }
        else {
            anInfo.st_size = 0;
            anOldDate[0] = '\0';
        }
    }
    aRequest+="\r\n";

    // Connect to Web Server
    anError = aSocket.connect(string(PATCHSERVER_IPSTRINGS[0]),PATCHSERVER_PORTS[0],0);
    if(anError!=WONMisc::ES_NO_ERROR) {
        titanGetPatchFailedCB(PATCHFAIL_UNABLE_TO_CONNECT);
        titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_UNABLE_TO_CONNECT (1) Error: %d", anError);
        return;
    }

    do {
        anError = aSocket.checkAsynchConnect(1000);
        if (anError == WONMisc::ES_NO_ERROR)
            break;
        else if (anError != WONMisc::ES_TIMED_OUT)
        {
            titanGetPatchFailedCB(PATCHFAIL_UNABLE_TO_CONNECT);
            titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_UNABLE_TO_CONNECT (2) Error: %d", anError);
            return;
        }

        if (patchAbortRequest)
        {
            titanGetPatchFailedCB(PATCHFAIL_USERABORT);
            titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_USERABORT");
            return;
        }
    } while (anError == WONMisc::ES_TIMED_OUT);

    // Send the HTTP request
    do {
        anError = aSocket.sendBuffer((void*)aRequest.c_str(),aRequest.length());
        if (patchAbortRequest)
        {
            titanGetPatchFailedCB(PATCHFAIL_USERABORT);
            titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_USERABORT");
            return;
        }
    } while(anError==WONMisc::ES_TIMED_OUT);

    if(anError!=WONMisc::ES_NO_ERROR) {
        titanGetPatchFailedCB(PATCHFAIL_ERROR_SENDING_REQUEST);
        titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_ERROR_SENDING_REQUEST Error: %d", anError);
        return;
    }

    char ch = ' ';
    string aLine;
    int aFileLen = -1;

    bool append = false;
    bool readFirstLine = false;

    // Read HTTP header
    while(true)
    {
        aLine.erase();
        // Read a line of the Header
        while(true)
        {
            if (patchAbortRequest)
            {
                titanGetPatchFailedCB(PATCHFAIL_USERABORT);
                titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_USERABORT");
                return;
            }

            anError = aSocket.recvBuffer(&ch,1);
            if(anError!=WONMisc::ES_NO_ERROR) {
                if(anError==WONMisc::ES_TIMED_OUT)
                    continue;

                titanGetPatchFailedCB(PATCHFAIL_ERROR_RECEIVING_HTTP_HEADER);
                titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_ERROR_RECEIVING_HTTP_HEADER (1) Error: %d", anError);
                return;
            }

            if(ch!='\r')
                aLine+=ch;
            else {
                do {
                    anError = aSocket.recvBuffer(&ch,1);
                    if (patchAbortRequest)
                    {
                        titanGetPatchFailedCB(PATCHFAIL_USERABORT);
                        titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_USERABORT");
                        return;
                    }
                } while(anError==WONMisc::ES_TIMED_OUT);

                if(anError!=WONMisc::ES_NO_ERROR) {
                    titanGetPatchFailedCB(PATCHFAIL_ERROR_RECEIVING_HTTP_HEADER);
                    titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_ERROR_RECEIVING_HTTP_HEADER (2) Error: %d", anError);
                    return;
                }

                if(ch!='\n') {
                    aLine+='\r';
                    aLine+=ch;
                }
                else
                    break;
            }
        }

        // Inspect the line
        if(aLine.length()==0) // Nothing -> finished reading header
            break;

        if(readFirstLine==false) {
            readFirstLine = true;
            if(strstr(aLine.c_str(),"200")!=NULL) // 200 -> Get entire body
                append = false; // Full get
            else if(strstr(aLine.c_str(),"206")!=NULL) // 206 -> Get partial body
                append = true; // Partial get
            else
            {
                titanGetPatchFailedCB(PATCHFAIL_INVALID_STATUS_REPLY);
                titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_INVALID_STATUS_REPLY HTTP status line = \"%s\"", aLine.c_str());
                return;
            }
        }

        if(aFileLen==-1) // Check for Content-Length
            sscanf(aLine.c_str(),"Content-Length: %d",&aFileLen);

        if(aNewDate[0]=='\0') { // Check for Last-Modified
            const char *aPtr = strstr(aLine.c_str(),"Last-Modified: ");
            if(aPtr!=NULL) {
                aPtr+=strlen("Last-Modified: ");
                strncpy(aNewDate, aPtr, 254);
                aModFile = fopen("modified.txt","w");
                if(aModFile!=NULL) {
                    fwrite(aNewDate,1,strlen(aNewDate)+1,aModFile);
                    fclose(aModFile);
                }
            }
        }
    }

    if(!append && !strcmp(anOldDate, aNewDate)) {
        // The web server returns 200 (Full Get) if you request zero bytes of the file
        titanGetPatchCompleteCB();
        return;
    }

    if(aFileLen<=0) {
        titanGetPatchFailedCB(PATCHFAIL_INVALID_FILE_LENGTH);
        titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_INVALID_FILE_LENGTH File-length = %d", aFileLen);
        return;
    }

    int aLen, aTotalLen = 0;

    if(append) {
        aTotalLen = anInfo.st_size;
        aFileLen+=aTotalLen;
    }

    FILE *aFile;
    if(append)  // Continue download
        aFile = fopen(saveFileName.c_str(),"a+b");
    else  // start download
        aFile = fopen(saveFileName.c_str(),"wb");

    if(aFile==NULL) {
        titanGetPatchFailedCB(PATCHFAIL_UNABLE_TO_CREATE_FILE);
        titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_UNABLE_TO_CREATE_FILE errno = %d", errno);
        return;
    }

    while(true) {
        aLen = 0;
        anError = aSocket.recvBuffer(aBuf,1000,&aLen);
        aTotalLen+=aLen;

        if(anError==WONMisc::ES_NO_ERROR || anError==WONMisc::ES_INCOMPLETE_RECV)
        {
            if(fwrite(aBuf,1,aLen,aFile)!=aLen) {
                titanGetPatchFailedCB(PATCHFAIL_UNABLE_TO_WRITE_FILE);
                titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_UNABLE_TO_WRITE_FILE (1) ferror = %d, errno = %d", ferror(aFile), errno);
                fclose(aFile);
                return;
            }
        }
        else if(anError!=WONMisc::ES_TIMED_OUT) {
            titanGetPatchFailedCB(PATCHFAIL_ERROR_RECEIVING_PATCH);
            titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_ERROR_RECEIVING_PATCH Error: %d", anError);
            fclose(aFile);
            return;
        }

        titanPatchProgressCB(aTotalLen,aFileLen);

        if(aTotalLen>=aFileLen)
            break;

        if (patchAbortRequest)
        {
            titanGetPatchFailedCB(PATCHFAIL_USERABORT);
            titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_USERABORT");
            fclose(aFile);
            return;
        }
    }

    if(fclose(aFile)==EOF) {
        titanGetPatchFailedCB(PATCHFAIL_UNABLE_TO_WRITE_FILE);
        titanDebug("FAIL TitanInterface::GetPatch PATCHFAIL_UNABLE_TO_WRITE_FILE (2) ferror = %d, errno = %d", ferror(aFile), errno);
        return;
    }

    titanGetPatchCompleteCB();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::SetGameKey(unsigned char *key)
{
    mGameKey.Create(GAMEKEY_SIZE,key);
}

const unsigned char *TitanInterface::GetGameKey(void)
{
    return mGameKey.GetKey();
}

void TitanInterface::loadVerifierKey()
{
    unsigned char* aBuf = NULL; // buffer is allocated by titanLoadPublicKey
    unsigned short aLen;

    aLen = titanLoadPublicKey(VERIFIER_KEY_FILE_NAME,&aBuf);

    if(aLen==0) {
        titanDebug("FAIL: Unable to read verifier key.");
        return;
    }

    try {
        mVerifierKey = new EGPublicKey(aLen,aBuf);
    }
    catch(WONException&) {
        titanDebug("EXCEPTION: Failed to create verifier key.");
        mVerifierKey = NULL;
    }

    free(aBuf);
}

void
TitanInterface::PumpEngine()
{
    EasySocketEngine::Pump(mEngine);
}

void
TitanInterface::startThread()
{
    // Create engine and start (launches the engine thread)
    mEngine = new EasySocketEngine(3, 0, false);

    // Add the timer pipe to the engine for certificate refreshes
    mTimerPipe = new SocketPipe;
    mEngine->AddPipe(mTimerPipe);

    // Start up the message receive thread
    ThreadBase::startThread();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::stopThread()
{
    if (!mIsLan)
        OnFinalLobbyExit();

    // Stop the message receive thread
    SetEvent(getStopEvent());
    if (mEngine)
        mEngine->AbortGetCompletedPipe();

    ThreadBase::stopThread();

    // Shutdown the engine if needed
    if (mEngine)
    {
        delete mEngine;  mEngine = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::BuildAddress(SOCKADDR_IN& theAddrR, long theIP, unsigned short thePort)
{
    theAddrR.sin_family      = AF_INET;
    theAddrR.sin_addr.s_addr = theIP;
    theAddrR.sin_port        = htons(thePort);
}

void TitanInterface::BuildAddress(SOCKADDR_IN& theAddrR, const WONCommon::RawBuffer& theSixBytes)
{
    theAddrR.sin_family      = AF_INET;
    memcpy(&theAddrR.sin_port, theSixBytes.data(), 6);
}

void TitanInterface::BuildAddress(SOCKADDR_IN& theAddrR, unsigned char buffer[])
{
    theAddrR.sin_family      = AF_INET;
    memcpy(&theAddrR.sin_port, buffer, 6);
}

const char* TitanInterface::PrintAddress(SOCKADDR_IN& theAddrR)
{
    static char aAddrString[22];
    sprintf(aAddrString, "%d.%d.%d.%d:%d", theAddrR.sin_addr.S_un.S_un_b.s_b1, theAddrR.sin_addr.S_un.S_un_b.s_b2, theAddrR.sin_addr.S_un.S_un_b.s_b3, theAddrR.sin_addr.S_un.S_un_b.s_b4, ntohs(theAddrR.sin_port));
    return aAddrString;
}
const char* TitanInterface::PrintAddress(const WONCommon::RawBuffer& theSixBytes)
{
    SOCKADDR_IN aSockAddrIn;
    BuildAddress(aSockAddrIn, theSixBytes);
    return PrintAddress(aSockAddrIn);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

unsigned long
TitanInterface::GetLocalIPAddress(void)
{
    SOCKET aSocket;
    long aLong;
    long anotherLong;
    int aRetValue;
    int anAddrSize;
    BOOL aBool = 1;
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD NumberOfFreeClusters;
    DWORD NumberOfClusters;
    BOOL success;

    success = GetDiskFreeSpace(NULL,&SectorsPerCluster,&BytesPerSector,&NumberOfFreeClusters,&NumberOfClusters);
    if ( success == FALSE )
        return 0;
    aLong = GetTickCount();
    aLong += (SectorsPerCluster * BytesPerSector * NumberOfFreeClusters );
    anotherLong = aLong;

    aSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if ( aSocket == INVALID_SOCKET )
        return 0;
    aRetValue = setsockopt(aSocket,SOL_SOCKET,SO_BROADCAST, (char*)(&aBool), sizeof(BOOL) );
    if ( aRetValue != 0 )
    {
        closesocket(aSocket);
        return 0;
    }
    SOCKADDR_IN aSockAddrIn;
    memset((char *)&aSockAddrIn, 0, sizeof(aSockAddrIn));
    aSockAddrIn.sin_family      = AF_INET;
    aSockAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
    aSockAddrIn.sin_port        = htons(48357);
    aRetValue = bind( aSocket, (SOCKADDR*)(&aSockAddrIn), sizeof(SOCKADDR_IN) );
    if ( aRetValue != 0 )
    {
        closesocket(aSocket);
        return 0;
    }
    aSockAddrIn.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    aRetValue = sendto(aSocket,(char*)&aLong,sizeof(long),0,(SOCKADDR*)(&aSockAddrIn), sizeof(SOCKADDR_IN) );
    if ( aRetValue != sizeof(long) )
    {
        closesocket(aSocket);
        return 0;
    }
    do {
        anAddrSize = sizeof(SOCKADDR_IN);
        aRetValue = recvfrom(aSocket,(char*)&aLong,sizeof(long),0,(SOCKADDR*)(&aSockAddrIn), &anAddrSize );
    } while (aRetValue == sizeof(long) && anotherLong != aLong );
    closesocket(aSocket);
    if ( aRetValue != sizeof(long) )
        return 0;

    return aSockAddrIn.sin_addr.s_addr;
//  return ntohl(aSockAddrIn.sin_addr.s_addr);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

unsigned long TitanInterface::GetLengthFieldSize(const BaseMessage& theMsgR)
{
    switch (theMsgR.GetMessageClass())
    {
        case WONMsg::eTMessage:
        case WONMsg::eSmallMessage:
            return 4;
        case WONMsg::eMiniMessage:
            return 2;
        default:
            return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool
TitanInterface::SendMsg(SocketPipe* thePipeP, const BaseMessage& theMsgR, unsigned char theLengthFieldSize)
{
    if(thePipeP==NULL)
        return false;

    // Build send buffer
    unsigned long  aMsgLen = theMsgR.GetDataLen();

    if (theLengthFieldSize==0)
        theLengthFieldSize = GetLengthFieldSize(theMsgR);
    if (theLengthFieldSize==0)
    {
        titanDebug("TitanInterface::SendMsg Invalid message class type.");
        return false;
    }

    aMsgLen+=theLengthFieldSize;
    char *aBuf = new char[aMsgLen];

    if(theLengthFieldSize==2) *(unsigned short*)aBuf = aMsgLen;
    else if(theLengthFieldSize==4) *(unsigned long*)aBuf = aMsgLen;
    else _ASSERT(0);

#ifdef LOG_MESSAGE_SIZE
    titanDebug("TitanInterface::SendMsg Sending %d bytes", aMsgLen + theLengthFieldSize);
#endif // LOG_MESSAGE_SIZE

    memcpy(aBuf+theLengthFieldSize,theMsgR.GetDataPtr(),theMsgR.GetDataLen());
    // Send the buffer

    thePipeP->AddOutgoingCmd(new SendCmd(aBuf, aMsgLen, true, true));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

SocketPipe* TitanInterface::ConnectTo(const Address& theDest, EasySocket::SocketType theType, WONMisc::RecvLengthPrefixType thePrefixType)
{
    SOCKADDR anAddr;
    if ( mIpType == ip )
        BuildAddress(reinterpret_cast<SOCKADDR_IN&>(anAddr), theDest.AddrPart.IP, theDest.Port);
    else
        EasySocket::getSockAddrIpx(reinterpret_cast<SOCKADDR_IPX&>(anAddr), theDest.AddrPart.etherAddr, theDest.Port);
    return ConnectTo(anAddr, theType, thePrefixType);
}

SocketPipe* TitanInterface::ConnectTo(const SOCKADDR& theDest, EasySocket::SocketType theType, WONMisc::RecvLengthPrefixType thePrefixType)
{
    // Create a TCP pipe
    SocketPipe* aPipeP = new SocketPipe;
    aPipeP->AddOutgoingCmd(new OpenCmd(theType));

    // Connect to destination, set event when done
    HANDLE anEvent = CreateEvent(NULL, false, false, NULL);
    aPipeP->AddOutgoingCmd(new SmartConnectCmd(theDest, CONNECT_TIMEOUT, 1, true));
    aPipeP->AddOutgoingCmd(new SetEventCmd(anEvent));

    // Add a bunch of RecvCmds
    aPipeP->AddIncomingCmd(new WaitForEventCmd(anEvent, true, false));
    for (int i=0; i < PREPOST_MAX; i++)
        aPipeP->AddIncomingCmd(new RecvPrefCmd(false, thePrefixType, false));

    return aPipeP;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

SocketPipe* TitanInterface::ConnectAndSend(const Address& theDest, const BaseMessage& theMsgR, EasySocket::SocketType theType, WONMisc::RecvLengthPrefixType thePrefixType)
{
    SOCKADDR anAddr;
    if ( mIpType == ip )
        BuildAddress(reinterpret_cast<SOCKADDR_IN&>(anAddr), theDest.AddrPart.IP, theDest.Port);
    else
        EasySocket::getSockAddrIpx(reinterpret_cast<SOCKADDR_IPX&>(anAddr), theDest.AddrPart.etherAddr, theDest.Port);
    return ConnectAndSend(anAddr, theMsgR, theType, thePrefixType);
}
SocketPipe* TitanInterface::ConnectAndSend(const SOCKADDR_IN& theDest, const BaseMessage& theMsgR, EasySocket::SocketType theType, WONMisc::RecvLengthPrefixType thePrefixType)
{ return ConnectAndSend(reinterpret_cast<const SOCKADDR&>(theDest), theMsgR, theType, thePrefixType); }
SocketPipe* TitanInterface::ConnectAndSend(const SOCKADDR& theDest, const BaseMessage& theMsgR, EasySocket::SocketType theType, WONMisc::RecvLengthPrefixType thePrefixType)
{
    // Create a TCP pipe
    SocketPipe* aPipeP = ConnectTo(theDest, theType, thePrefixType);
    if (aPipeP)
    {
        int aLengthFieldSize = 4;
        switch(thePrefixType) {
            case WONMisc::ptByte:
            case WONMisc::ptUnsignedByte: aLengthFieldSize = 1; break;
            case WONMisc::ptShort:
            case WONMisc::ptUnsignedShort: aLengthFieldSize = 2; break;
            case WONMisc::ptLong:
            case WONMisc::ptUnsignedLong: aLengthFieldSize = 4; break;
        }

        // Send message
        if (! SendMsg(aPipeP, theMsgR, aLengthFieldSize))
        {
            titanDebug("FAIL: TitanInterface::SendMsg!");
            delete aPipeP;  aPipeP = NULL;
        }
        // Hand off pipe to the engine
        else
            mEngine->AddPipe(aPipeP);
    }

    return aPipeP;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::AuthFailOver(void)
{
    titanDebug("TitanInterface::AuthFailOver");

    mNumAuthServersTried++; // Trying another auth server
    if(mNumAuthServersTried>=AUTHSERVER_NUM)
    {
        titanDebug("TitanInterface::AuthFailOver FAILED all %d authservers.", AUTHSERVER_NUM);
        mgNotifyAuthRequestFailed();
        ResetAuthFailOver();
        return;
    }

    mCurAuthServer = (mCurAuthServer + 1) % AUTHSERVER_NUM;
    titanDebug("TitanInterface::AuthFailOver Trying %s", PrintAddress(AUTHSERVER_ADDRESSES[mCurAuthServer]));

    Authenticate(mLoginName,mPassword,"",false);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::ResetAuthFailOver(void)
{
    titanDebug("TitanInterface::ResetAuthFailOver");

    mNumAuthServersTried = 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::DirFailOver(void)
{
    titanDebug("TitanInterface::DirFailOver");

    delete mDirSessionKey;
    mDirSessionKey = NULL;

    mNumDirServersTried++; // Trying another auth server
    if(mNumDirServersTried>=DIRSERVER_NUM)
    {
        titanDebug("TitanInterface::DirFailOver FAILED all %d dir servers.", DIRSERVER_NUM);
        mgNotifyDirRequestFailed();
        ResetDirFailOver();
        return;
    }

    mCurDirServer = (mCurDirServer + 1) % DIRSERVER_NUM;
    titanDebug("TitanInterface::DirFailOver Trying %s:%d", DIRSERVER_IPSTRINGS[mCurDirServer], DIRSERVER_PORTS[mCurDirServer]);

    RequestDirectory();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::ResetDirFailOver(void)
{
    titanDebug("TitanInterface::ResetDirFailOver");

    mNumDirServersTried = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::FactFailOver(void)
{
    titanDebug("TitanInterface::FactFailOver");

    delete mFactSessionKey;
    mFactSessionKey = NULL;

    if (mFailFactOverDirectly)
    {
        mgStartSpecificFactServerFailedCB();

        ResetFactFailOver();
        return;
    }

    mNumFactServersTried++; // Trying another factory server

    if(mNumFactServersTried >= FACTSERVER_NUM) {
        titanDebug("TitanInterface::FactFailOver FactFailOver FAILED all %d factory servers.", FACTSERVER_NUM);
        if (mGameCreationState == GAME_NOT_STARTED)
            titanStartChatReplyReceivedCB(WONMsg::StatusCommon_Failure);
        else
        {
            mgDisplayMessage(strGetString(strServerDown));
            mgDisplayMessage(strGetString(strHitCancelAgain));
        }

        ResetFactFailOver();
        return;
    }

    mCurFactServer = (mCurFactServer + 1) % FACTSERVER_NUM;
    titanDebug("TitanInterface::FactFailOver Trying %s", PrintAddress(FACTSERVER_ADDRESSES[mCurFactServer]));

    AutoCrit aCrit(mStartRoutingCrit);
    if(mRoomPassword.length()!=0)
        StartRoutingServer(mChannelName.c_str(),mChannelDescription.c_str(),mRoomPassword.c_str(),mIsGameServer,NULL);
    else
        StartRoutingServer(mChannelName.c_str(),mChannelDescription.c_str(),NULL,mIsGameServer,NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::ResetFactFailOver(void)
{
    titanDebug("TitanInterface::ResetFactFailOver");

    mNumFactServersTried = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::FirewallFailOver(void)
{
    titanDebug("TitanInterface::FirewallFailOver");

    mNumFirewallServersTried++; // Trying another auth server
    if(mNumFirewallServersTried>=FIREWALLSERVER_NUM)
    {
        titanDebug("TitanInterface::FirewallFailOver FAILED all %d firewall servers.", FIREWALLSERVER_NUM);
        return;
    }

    mCurFirewallServer = (mCurFirewallServer + 1) % FIREWALLSERVER_NUM;
    titanDebug("TitanInterface::FirewallFailOver Trying %s", PrintAddress(FIREWALLSERVER_ADDRESSES[mCurFirewallServer]));

    FirewallDetect();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::FirewallDetect(void)
{
    titanDebug("TitanInterface::FirewallDetect");

    AutoCrit aCrit(mPipeCrit);
    // Toss old connection if needed
    if (mFirewallPipe)
    {
        titanDebug("NOTE: TitanInterface::FirewallDetect Close existing FirewallPipe");
        mFirewallPipe->AddOutgoingCmd(new CloseNowCmd(false, false, 0, false));
        mFirewallPipe = NULL;
    }

    aCrit.Leave();
    WONMsg::SMsgFirewallDetect aMsg;
    aMsg.SetListenPort(GAME_PORT);

    if(mCurFirewallServer>=FIREWALLSERVER_NUM) {
        if(FIREWALLSERVER_NUM>0)
            mCurFirewallServer%=FIREWALLSERVER_NUM;
        else
            return;
    }

    try
    {
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::FirewallDetect: %s", (char*)anEx.what());
        return;
    }

    mFirewallPipe = ConnectAndSend(FIREWALLSERVER_ADDRESSES[mCurFirewallServer], aMsg, EasySocket::TCP, WONMisc::ptUnsignedShort);

    if (!mFirewallPipe)
    {
        titanDebug("FAIL: TitanInterface::FirewallDetect!");
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleFirewallResponse(SocketPipe *thePipeP, const WONMsg::SmallMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleFirewallResponse");

    try {
        WONMsg::SMsgFirewallStatusReply aReply(theMsgR);

        if(thePipeP==mFirewallPipe) {
            if(aReply.GetStatus()==WONMsg::StatusCommon_Success) {
                mFirewallPipe = NULL;
                return; // Success, now just wait for the connect
            }
            else {
                titanDebug("TitanInterface::HandleFirewallResponse Status = %d", aReply.GetStatus());
                FirewallFailOver();
                return;
            }
        }
        else { // The firewall server must have successfully connected
            mBehindFirewall = false;
        }
    }
    catch(WONException&) {
        if(thePipeP==mFirewallPipe)
            FirewallFailOver();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::PingFactServer() {
    titanDebug("TitanInterface::PingFactServer");

    AutoCrit aCrit(mPipeCrit);

    // Toss old connection if needed
    if (mFactPingPipe)
    {
        mFactPingPipe->AddOutgoingCmd(new CloseNowCmd(false, false, 0, false));
        mFactPingPipe = NULL;
    }
    aCrit.Leave();


    mCurFactPing++;
    if(mCurFactPing>=FACTSERVER_NUM) {
        mCurFactPing = 0;
        mNumPingTrials++;
        if(mNumPingTrials>=3)
            return;
    }

    WONMsg::MMsgCommPing aMsg;
    try
    {
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::PingFactServer: %s", (char*)anEx.what());
        return;
    }

    mFactPingPipe = ConnectAndSend(FACTSERVER_ADDRESSES[mCurFactPing], aMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::PingHandleReply(const WONMsg::MiniMessage& theMsgR) {
    titanDebug("TitanInterface::PingHandleReply");

    // Toss ping pipe
    mPipeCrit.Enter();
    mFactPingPipe = NULL;
    mPipeCrit.Leave();

    try
    {
        WONMsg::MMsgCommPingReply aMsg(theMsgR);

        if(aMsg.GetLag() < mMinPingTime) {
            mCurFactServer = mCurFactPing;
            mMinPingTime = aMsg.GetLag();
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::PingHandleReply: %s", (char*)anEx.what());
    }

    PingFactServer();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::AuthGetPubKeyBlock(void) {
    titanDebug("TitanInterface::AuthGetPubKeyBlock");

    AutoCrit aCrit(mPipeCrit);
    // Toss old connection if needed
    if (mAuthPipe)
    {
        titanDebug("NOTE: TitanInterface::AuthGetPubKeyBlock Close existing AuthPipe!");
        mAuthPipe->AddOutgoingCmd(new CloseNowCmd(false, false, 0, false));
        mAuthPipe = NULL;
    }
    aCrit.Leave();

    WONMsg::TMsgAuth1GetPubKeys aMsg(WONMsg::Auth1Login);


    if(AUTHSERVER_NUM==0) {
        titanDebug("FAIL: No auth server to talk to.");
        return;
    }
    else if(mCurAuthServer>=AUTHSERVER_NUM)
        mCurAuthServer%=AUTHSERVER_NUM;

    // Connect and send request
    delete mPublicKeyBlock;
    mPublicKeyBlock = NULL;

    try
    {
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::AuthGetPubKeyBlock: %s", (char*)anEx.what());
        return;
    }

    mAuthPipe = ConnectAndSend(AUTHSERVER_ADDRESSES[mCurAuthServer], aMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


void TitanInterface::AuthHandleGetPubKeysReply(const WONMsg::TMessage& theMsgR)
{
    titanDebug("TitanInterface::AuthHandleGetPubKeysReply Got PubKeysReply message");

    // Toss auth pipe
    mPipeCrit.Enter();
    mAuthPipe = NULL;
    mPipeCrit.Leave();

    if(mVerifierKey==NULL) {
        titanDebug("TitanInterface::AuthHandleGetPubKeysReply FAIL: Don't have verifier key.");
        return;
    }

    try
    {
        WONMsg::TMsgAuth1GetPubKeysReply msg(theMsgR);
        short status = msg.GetStatus();
        if(status<0) {
            titanDebug("TitanInterface::AuthHandleGetPubKeysReply Status = %d", msg.GetStatus());
            if (!authReceiveReply(status))
                AuthFailOver();
            else
                ResetAuthFailOver();
            return;
        }

        mPublicKeyBlock = new Auth1PublicKeyBlock(msg.GetRawBuf(),msg.GetRawBufLen());
        if(!mPublicKeyBlock->Verify(*mVerifierKey)) {
            delete mPublicKeyBlock;
            mPublicKeyBlock = NULL;
            AuthFailOver();
            return;
        }

        ResetAuthFailOver();
        AuthHandleLogin();
    }
    catch (WONException& anEx)
    {
        AuthFailOver();
        titanDebug("EXCEPTION: TitanInterface::AuthHandleGetPubKeysReply: %s", (char*)anEx.what());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool TitanInterface::ReadLoginKey(char *theKey) {
    FILE *aFile = fopen(LOGIN_KEY_FILE_NAME,"rb");
    if(aFile==NULL)
        return false;

    unsigned char aNum = fgetc(aFile);
    LoginKeyStruct *aKeyStruct = new LoginKeyStruct[aNum];
    aNum = fread(aKeyStruct,sizeof(LoginKeyStruct),aNum,aFile);
    fclose(aFile);

    for(int n=0; n<aNum; n++) {
        if((AUTHSERVER_ADDRESSES[mCurAuthServer].sin_addr.s_addr == aKeyStruct[n].authAddr) && (AUTHSERVER_ADDRESSES[mCurAuthServer].sin_port == aKeyStruct[n].authPort))
        {
            memcpy(theKey, aKeyStruct[n].loginKey, 8);
            break;
        }
    }

    delete [] aKeyStruct;
    return n<aNum;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::WriteLoginKey(char *theKey, bool useOldNewLoginKey) {
    if(AUTHSERVER_NUM==0)
        return;

    FILE *aFile = fopen(LOGIN_KEY_FILE_NAME,"rb");
    unsigned char aNum = 0;
    int n;

    LoginKeyStruct *aKeyStruct = NULL;
    LoginKeyStruct *aNewKeyStruct = new LoginKeyStruct[AUTHSERVER_NUM];

    map<ClientNetAddr,unsigned char> anAddrMap;
    map<ClientNetAddr,unsigned char>::iterator anItr;

    if(aFile!=NULL) {
        aNum = fgetc(aFile);
        aKeyStruct = new LoginKeyStruct[aNum];

        aNum = fread(aKeyStruct,sizeof(LoginKeyStruct),aNum,aFile);

        for(n=0; n<aNum; n++)
            anAddrMap[ClientNetAddr(aKeyStruct[n].authAddr,aKeyStruct[n].authPort)] = n;

        fclose(aFile);
    }

    for(n=0; n<AUTHSERVER_NUM; n++) {
        anItr = anAddrMap.find(ClientNetAddr(AUTHSERVER_ADDRESSES[n].sin_addr.s_addr, AUTHSERVER_ADDRESSES[n].sin_port));

        if(anItr!=anAddrMap.end())
            memcpy(&aNewKeyStruct[n], &aKeyStruct[anItr->second], sizeof(LoginKeyStruct));
        else {
            aNewKeyStruct[n].authAddr = AUTHSERVER_ADDRESSES[n].sin_addr.s_addr;
            aNewKeyStruct[n].authPort = AUTHSERVER_ADDRESSES[n].sin_port;
        }

        if(n==mCurAuthServer) {
            if(theKey!=NULL) {
                memcpy(mOldNewLoginKey, aNewKeyStruct[n].newLoginKey, 8);
                memcpy(aNewKeyStruct[n].newLoginKey,theKey,8);
            }
            else {
                if(useOldNewLoginKey)
                    memcpy(aNewKeyStruct[n].newLoginKey, mOldNewLoginKey, 8);

                memcpy(aNewKeyStruct[n].loginKey,aNewKeyStruct[n].newLoginKey,8);
            }
        }
    }

    aFile = fopen(LOGIN_KEY_FILE_NAME,"wb");
    if(aFile!=NULL) {
        fwrite(&AUTHSERVER_NUM,1,1,aFile);
        fwrite(aNewKeyStruct,sizeof(LoginKeyStruct),AUTHSERVER_NUM,aFile);
        fclose(aFile);
    }

    delete [] aKeyStruct;
    delete [] aNewKeyStruct;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


void TitanInterface::AuthHandleLogin(void) {
    titanDebug("TitanInterface::AuthHandleLogin");

    if(AUTHSERVER_NUM==0) {
        titanDebug("FAIL: No auth server to talk to.");
        return;
    }
    else if(mCurAuthServer>=AUTHSERVER_NUM)
        mCurAuthServer%=AUTHSERVER_NUM;


    AutoCrit aCrit(mPipeCrit);
    // Toss old connection if needed
    if (mAuthPipe)
    {
        titanDebug("NOTE: TitanInterface::AuthHandleLogin Close existing AuthPipe!");
        mAuthPipe->AddOutgoingCmd(new CloseNowCmd(false, false, 0, false));
        mAuthPipe = NULL;
    }
    aCrit.Leave();


    if(mPublicKeyBlock==NULL) {
        titanDebug("FAIL: AuthHandleLogin - Don't have public key block.");
        return;
    }

    WONMsg::TMsgAuth1LoginRequestHW aMsg;
    aMsg.SetKeyBlockId(mPublicKeyBlock->GetBlockId());
    aMsg.SetNeedKeyFlg(mPrivateKey==NULL);
    aMsg.SetCreateAcctFlg(mCreateAccount);
    aMsg.SetUserName(StringToWString(mLoginName));
#if defined(CGW) || defined(Downloadable)
    aMsg.SetCommunityName(StringToWString("HomeworldDemo"));
#else // Normal release/beta
    aMsg.SetCommunityName(StringToWString("Homeworld"));
#endif
    aMsg.SetPassword(StringToWString(mPassword));
    aMsg.SetNewPassword(StringToWString(mNewPassword));

#if !defined(CGW) && !defined(Downloadable)
    WONCDKey::ClientCDKey aCDKey("Homeworld");
    if (!aCDKey.Load()) {
        titanDebug("FAIL: AuthHandleLogin - Failed to load CD key from registry.");
        mgDisplayMessage(strGetString(strNoCDKey));
        return;
    }
    if (!aCDKey.IsValid()) {
        titanDebug("FAIL: AuthHandleLogin - CD key failed lightweight check.");
        mgDisplayMessage(strGetString(strLightweightBadKey));
        return;
    }

#ifdef HW_Release
    // If this is a beta version, only beta keys are allowed.  If it's a release
    // version (not beta, not demo), only non-beta keys are allowed.
#ifdef DLPublicBeta
    if (!aCDKey.IsBeta()) {
        titanDebug("FAIL: AuthHandleLogin - CD key is NOT a beta key.");
        mgDisplayMessage(strGetString(strBadKeyBetaKeyRequired));
        return;
    }
#else
    if (aCDKey.IsBeta()) {
        titanDebug("FAIL: AuthHandleLogin - CD key IS a beta key.");
        mgDisplayMessage(strGetString(strBadKeyBetaKeyNotAllowed));
        return;
    }
#endif // DLPublicBeta
#endif // HW_Release

    __int64 aRawCDKey = aCDKey.AsRaw(); // message is encrypted, so key doesn't need to be
    aMsg.SetCDKey((const unsigned char*)&aRawCDKey,sizeof(aRawCDKey));
#endif // Not a CGW or Downloadable demo

    char aBuf[8];
    ReadLoginKey(aBuf);

    aMsg.SetLoginKey((const unsigned char*)aBuf,8);  // This is the secret

    delete mAuthSessionKey; mAuthSessionKey = NULL;
    try {
        // mAuthSessionKey = new BFSymmetricKey(8,(const unsigned char*)aBuf);
        mAuthSessionKey = new BFSymmetricKey(8);
    }
    catch(WONException&) {
        titanDebug("EXCEPTION: AuthHandleLogin unable to create session key.");
        return;
    }

    try {
        aMsg.BuildBuffer(*mPublicKeyBlock, *mAuthSessionKey);
    }
    catch(WONException&) {
        titanDebug("EXCEPTION: Unable to build AuthLogin message.");
        delete mAuthSessionKey; mAuthSessionKey = NULL;
    }

    try
    {
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::AuthHanleLogin: %s", (char*)anEx.what());
        return;
    }

    mAuthPipe = ConnectAndSend(AUTHSERVER_ADDRESSES[mCurAuthServer], aMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::AuthHandleChallenge(const WONMsg::TMessage &theMsgR) {
    titanDebug("TitanInterface::AuthHandleChallenge");

    AutoCrit aCrit(mPipeCrit);

    if(mAuthSessionKey==NULL) {
        titanDebug("FAIL: Don't have auth session key.");
        return;
    }

    try {
        WONMsg::TMsgAuth1ChallengeHW aMsg(theMsgR);

        CryptKeyBase::CryptReturn aCryptRet(NULL,0);
        aCryptRet = mAuthSessionKey->Decrypt(aMsg.GetRawBuf(),aMsg.GetRawBufLen());

        if(aCryptRet.first==NULL) {
            titanDebug("FAIL: Unable to decrypt challenge seed.");
            AuthFailOver();
            return;
        }

        auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

        if(aCryptRet.second!=16) {
            titanDebug("FAIL: Challenge seed length != 16 bytes.");
            AuthFailOver();
            return;
        }

        WriteLoginKey((char*)(aCryptRet.first));

        // respond to challenge from Auth Server
        unsigned int aChallengeResponse[8];

        if (ShortCircuitWON)
            ShortCircuitChallengeResponse((BYTE*)aCryptRet.first, reinterpret_cast<unsigned char*>(aChallengeResponse));
        else
        {
            struct MD5Context ContextEntireFile; // hash without seed
            struct MD5Context ContextKeyedHash;  // hash with seed
            MD5Init2(&ContextEntireFile); MD5Init2(&ContextKeyedHash);

            MD5Update2(&ContextKeyedHash, (BYTE*)aCryptRet.first, aCryptRet.second);

            bool firsttime = true;
            unsigned long aBytesRead = 0;
            unsigned char* aUnhashedBuf = NULL;
            while ((aBytesRead = GetHashSection(firsttime, &aUnhashedBuf, reinterpret_cast<unsigned char*>(aChallengeResponse) + MD5_HASH_SIZE)) != 0)
            {
                MD5Update2(&ContextEntireFile, aUnhashedBuf, aBytesRead);
                MD5Update2(&ContextKeyedHash, reinterpret_cast<unsigned char*>(aChallengeResponse) + MD5_HASH_SIZE, MD5_HASH_SIZE);
                firsttime = false;
            }

            MD5Final2(reinterpret_cast<unsigned char*>(aChallengeResponse), &ContextEntireFile);
            MD5Final2(reinterpret_cast<unsigned char*>(aChallengeResponse) + MD5_HASH_SIZE, &ContextKeyedHash);

            delete aUnhashedBuf; aUnhashedBuf = NULL;
        }

        CryptKeyBase::CryptReturn aReplyCrypt(NULL,0);
        aReplyCrypt = mAuthSessionKey->Encrypt((const unsigned char*)aChallengeResponse, MD5_HASH_SIZE*2);

        if(aReplyCrypt.first==NULL) {
            titanDebug("FAIL: Unable to encrypt challenge reply.");
            return;
        }

        auto_ptr<unsigned char> aDeleteReplyCrypt(aReplyCrypt.first);

        WONMsg::TMsgAuth1ConfirmHW aReply;
        aReply.SetRawBuf(aReplyCrypt.first,aReplyCrypt.second,true);
        aReply.Pack();


        if(!SendMsg(mAuthPipe,aReply)) {
            titanDebug("FAIL: Unable to send Auth1LoginConfirm message.");
            return;
        }

    }
    catch(WONException &anEx) {
        titanDebug("EXCEPTION: TitanInterface::AuthHandleChallenge: %s", (char*)anEx.what());
        AuthFailOver();
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::AuthHandleLoginReply(const WONMsg::TMessage &theMsg)
{
    titanDebug("TitanInterface::AuthHandleLoginReply.");

    // Toss auth pipe
    mPipeCrit.Enter();
    mAuthPipe = NULL;
    mPipeCrit.Leave();

    if(mPublicKeyBlock==NULL) {
        titanDebug("FAIL: AuthHandleLogin - Don't have public key block.");
        return;
    }

    if(mAuthSessionKey==NULL) {
        titanDebug("FAIL: Don't have auth session key.");
        return;
    }


    try {
        WONMsg::TMsgAuth1LoginReply aMsg(theMsg,mAuthSessionKey);
        WONMsg::TMsgAuth1LoginReply::RawBlock aRawBlock;


        if(aMsg.GetStatus()<0) {
            titanDebug("TitanInterface::AuthHandleLoginReply Status = %d", aMsg.GetStatus());
            if(aMsg.GetStatus()==WONMsg::StatusAuth_KeyInUse) {
                // Perhaps we succeeded previously but never got the reply
                // If this is the case, then trying to authenticate again
                // will succeed.
                WriteLoginKey(NULL, true);
            }

            if (!authReceiveReply(aMsg.GetStatus()))
                AuthFailOver();
            else
            ResetAuthFailOver();
            return;
        }

        // Check for new public key block
        aRawBlock = aMsg.GetRawBlock(WONMsg::TMsgAuth1LoginReply::ALPublicKeyBlock);
        if(aRawBlock.first!=NULL) {
            delete mPublicKeyBlock; mPublicKeyBlock = NULL;
            mPublicKeyBlock = new Auth1PublicKeyBlock(aRawBlock.first,aRawBlock.second);
            if(!mPublicKeyBlock->Verify(*mVerifierKey)) {
                titanDebug("FAIL: Failed to verify new public key block.");
                delete mPublicKeyBlock; mPublicKeyBlock = NULL;
                AuthFailOver();
                return;
            }
        }

        // Check for certificate
        delete mCertificate; mCertificate = NULL;
        aRawBlock = aMsg.GetRawBlock(WONMsg::TMsgAuth1LoginReply::ALCertificate);
        if(aRawBlock.first==NULL) {
            titanDebug("FAIL: Didn't get certificate.");
            AuthFailOver();
            return;
        }

        mCertificate = new Auth1Certificate(aRawBlock.first,aRawBlock.second);
        if(!mPublicKeyBlock->VerifyFamilyBuffer(*mCertificate)) {
            titanDebug("FAIL: Couldn't verify certificate from auth server.");
            delete mCertificate; mCertificate = NULL;
            AuthFailOver();
            return;
        }

        mAuthDeltaTime = mCertificate->GetIssueTime() - time(NULL);

        // Check for private key
        aRawBlock = aMsg.GetRawBlock(WONMsg::TMsgAuth1LoginReply::ALClientPrivateKey);
        if(aRawBlock.first!=NULL) {
            delete mPrivateKey; mPrivateKey = NULL;
            mPrivateKey = new EGPrivateKey(aRawBlock.second, aRawBlock.first);
        }
        else if(mPrivateKey==NULL) {
            titanDebug("FAIL: Didn't get private key from auth server.");
            delete mCertificate; mCertificate = NULL;
            AuthFailOver();
            return;
        }

        ResetAuthFailOver();

        time_t aRefreshWaitTime = 1000*(mCertificate->GetExpireTime() - mCertificate->GetIssueTime() - 120);
        mTimerPipe->AddIncomingCmd(new TimerCmd(aRefreshWaitTime,false));

        if(mNewPassword!="") {
            mPassword = mNewPassword;
            mNewPassword = "";
        }
        mCreateAccount = false;


        WriteLoginKey(NULL);
        authReceiveReply(aMsg.GetStatus());
    }
    catch(WONException &anEx) {
        titanDebug("EXCEPTION: TitanInterface::AuthHandleLoginReply: %s", (char*)anEx.what());
        AuthFailOver();
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::AuthHandleRefresh(void)
{
    titanDebug("TitanInterface::AuthHandleRefresh");

    if(mPublicKeyBlock==NULL) {
        titanDebug("FAIL: Don't have public key block,");
        return;
    }

    char aBuf[8];
    if(!ReadLoginKey(aBuf)) {
        titanDebug("AuthHandleRefresh: Login key not found.");
        return;
    }

    WONMsg::TRawMsg anEncryptBuf;

    anEncryptBuf.AppendShort((short)time(NULL));
    anEncryptBuf.AppendBytes(8, aBuf);

    CryptKeyBase::CryptReturn aCryptRet(NULL,0);
    aCryptRet = mPublicKeyBlock->EncryptRawBuffer((const unsigned char*)anEncryptBuf.GetDataPtr(),anEncryptBuf.GetDataLen());

    auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

    if(aCryptRet.first==NULL) {
        titanDebug("FAIL: Couldn't encrypt block with auth public key.");
        return;
    }

    WONMsg::TMsgAuth1RefreshHW aMsg;

    aMsg.SetKeyBlockId(mPublicKeyBlock->GetBlockId());
    aMsg.SetRawKeyBuf(aCryptRet.first,aCryptRet.second,true);
    aMsg.SetRawDataBuf(NULL,0,false);

    try
    {
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::AuthHanleRefresh: %s", (char*)anEx.what());
        return;
    }

    mAuthPipe = ConnectAndSend(AUTHSERVER_ADDRESSES[mCurAuthServer], aMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::Authenticate(const string &loginName, const string &password, const string &theNewPassword,bool CreateAccount)
{
    titanDebug("TitanInterface::Authenticate");

    AutoCrit aCrit(mPipeCrit);

    // Toss old connection if needed
    if (mAuthPipe)
    {
        titanDebug("NOTE: TitanInterface::Authenticate Close existing AuthPipe!");
        mAuthPipe->AddOutgoingCmd(new CloseNowCmd(false, false, 0, false));
        mAuthPipe = NULL;
    }
    aCrit.Leave();

    if(mVerifierKey==NULL) {
        titanDebug("FAIL: Can't authenticate without verifier key.");
        return;
    }

    mLoginName = loginName;
    mPassword = password;
    mNewPassword = theNewPassword;
    mCreateAccount = CreateAccount;


    if(!mLaunched) {
        if(firewallButton==FIREWALL_NOTBEHIND) {
            mBehindFirewall = false;
            mUseRoutingServer = true;
        }
        else if(firewallButton==FIREWALL_BEHIND) {
            mBehindFirewall = true;
            mUseRoutingServer = true;
        }
        else { // auto-detect
            mBehindFirewall = true;
            mUseRoutingServer = true;
            FirewallDetect();
        }

        mLaunched = true;
    }

    if(AUTHSERVER_NUM==0) {
        mNeedToAuthenticateAfterGettingAuthDirectory = true;

        RequestDirectory();
        return;
    }

    if(mPublicKeyBlock==NULL)
        AuthGetPubKeyBlock();
    else
        AuthHandleLogin();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
void TitanInterface::PeerHandleChallenge(SocketPipe** thePipeP, const TMessage& theMsgR)
{
    titanDebug("TitanInterface::PeerHandleChallenge");

    AutoCrit aCrit(mPipeCrit);

    bool dirStuff = false, factStuff = false, routeStuff = false;
    if(*thePipeP==mDirPipe)
        dirStuff = true;
    else if(*thePipeP==mFactPipe)
        factStuff = true;
        else {
            for(int n=0; n<2; n++)
                if(*thePipeP==mRoutePipe[n])
                    routeStuff = true;
        }

    if(mCertificate==NULL) {
        titanDebug("FAIL: Don't have certificate.");
        *thePipeP = NULL;
        return;
    }

    try {
        WONMsg::TMsgAuth1Challenge1 aMsg(theMsgR);
        CryptKeyBase::CryptReturn aCryptRet(NULL,0);
        BFSymmetricKey *aSessionKey;
        BFSymmetricKey *aClientSecret;

        aCryptRet = mPrivateKey->Decrypt(aMsg.GetSecretB(),aMsg.GetSecretBLen());

        auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

        if(aCryptRet.second < 2 ||
            (*(unsigned short*)aCryptRet.first)!=aCryptRet.second - 2)
        {
            titanDebug("Invalid SecretB received.");
            *thePipeP = NULL;

            if(dirStuff)
                DirFailOver();
            else if(factStuff)
                FactFailOver();

            return;
        }

        if(*thePipeP==mDirPipe) {
            delete mDirSessionKey; delete mDirClientSecret;
            mDirSessionKey = new BFSymmetricKey(aCryptRet.second-2,aCryptRet.first+2);
            mDirClientSecret = new BFSymmetricKey(8);
            aSessionKey = mDirSessionKey;
            aClientSecret = mDirClientSecret;
        }
        else if(*thePipeP==mFactPipe) {
            delete mFactSessionKey; delete mFactClientSecret;
            mFactSessionKey = new BFSymmetricKey(aCryptRet.second-2,aCryptRet.first+2);
            mFactClientSecret = new BFSymmetricKey(8);
            aSessionKey = mFactSessionKey;
            aClientSecret = mFactClientSecret;
        }
        else {
            for(int n=0; n<2; n++) {
                if(*thePipeP==mRoutePipe[n]) {
                    delete mRouteSessionKey[n]; delete mRouteClientSecret[n];
                    mRouteSessionKey[n] = new BFSymmetricKey(aCryptRet.second-2,aCryptRet.first+2);
                    mRouteClientSecret[n] = new BFSymmetricKey(8);
                    aSessionKey = mRouteSessionKey[n];
                    aClientSecret = mRouteClientSecret[n];
                    break;
                }
            }

            if(n==2) {
                titanDebug("FAIL: Pipe is neither dir nor fact nor route pipe.");
                return;
            }
        }

        Auth1Certificate aCertificate;
        if(!aCertificate.Unpack(aMsg.GetRawBuf(), aMsg.GetRawBufLen()) ||
            !mPublicKeyBlock->VerifyFamilyBuffer(aCertificate))
        {
            titanDebug("FAIL: Server certificate is invalid.");
            if(dirStuff)
                DirFailOver();
            else if(factStuff)
                FactFailOver();

            return;
        }

        TRawMsg aBuf;
        aBuf.AppendShort(aSessionKey->GetKeyLen());
        aBuf.AppendBytes(aSessionKey->GetKeyLen(),aSessionKey->GetKey());
        aBuf.AppendBytes(aClientSecret->GetKeyLen(), aClientSecret->GetKey());

        CryptKeyBase::CryptReturn aSecretCrypt(NULL, 0);
        aSecretCrypt = aCertificate.GetPubKey().Encrypt(aBuf.GetDataPtr(), aBuf.GetDataLen());

        auto_ptr<unsigned char> aDeleteSecretCrypt(aSecretCrypt.first);
        if(aSecretCrypt.first==NULL) {
            titanDebug("FAIL: Unable to encrypt challenge2.");

            if(dirStuff)
                DirFailOver();
            else if(factStuff)
                FactFailOver();

            return;
        }

        WONMsg::TMsgAuth1Challenge2 aChallenge2;
        aChallenge2.SetRawBuf(aSecretCrypt.first,aSecretCrypt.second,true);
        aChallenge2.Pack();

        if(!SendMsg(*thePipeP,aChallenge2, routeStuff?2:4)) {
            titanDebug("FAIL: Unable to send Challenge2.");

            if(dirStuff)
                DirFailOver();
            else if(factStuff)
                FactFailOver();

            return;
        }
    }
    catch(WONException &anEx) {
        titanDebug("EXCEPTION: TitanInterface::PeerHandleChallenge: %s", (char*)anEx.what());
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
void TitanInterface::PeerHandleComplete(SocketPipe** thePipeP, const TMessage& theMsgR)
{
    titanDebug("TitanInterface::PeerHandleComplete");

    AutoCrit aCrit(mPipeCrit);

    bool dirStuff = false, factStuff = false, routeStuff = false;
    if(*thePipeP==mDirPipe)
        dirStuff = true;
    else if(*thePipeP==mFactPipe)
        factStuff = true;

    int aRouteNum;

    if(mPrivateKey==NULL) {
        titanDebug("FAIL: Don't have private key.");
        return;
    }

    try {
        WONMsg::TMsgAuth1Complete aMsg(theMsgR);
        BFSymmetricKey **aClientSecret, **aSessionKey;

        if(*thePipeP==mDirPipe) {
            aClientSecret = &mDirClientSecret;
            aSessionKey = &mDirSessionKey;
        }
        else if(*thePipeP==mFactPipe) {
            aClientSecret = &mFactClientSecret;
            aSessionKey = &mFactSessionKey;
        }
        else {
            for(int n=0; n<2; n++) {
                if(*thePipeP==mRoutePipe[n]) {
                    aClientSecret = &(mRouteClientSecret[n]);
                    aSessionKey = &(mRouteSessionKey[n]);
                    routeStuff = true;
                    aRouteNum = n;
                    break;
                }
            }

            if(!routeStuff) {
                titanDebug("FAIL: Invalid pipe received in PeerHandleComplete.");
                return;
            }
        }

        if(aMsg.GetStatus() < 0) {
            titanDebug("FAIL: Failure status on Auth1Complete. %d", aMsg.GetStatus());
            delete *aClientSecret; *aClientSecret=NULL;
            delete *aSessionKey; *aSessionKey=NULL;
            return;
        }

        if(*aClientSecret==NULL) {
            titanDebug("FAIL: Don't have ClientSecret.");
            delete *aSessionKey; *aSessionKey = NULL;
            return;
        }

        if(*aSessionKey==NULL) {
            titanDebug("FAIL: Don't have session key.");
            delete *aClientSecret; *aClientSecret = NULL;
            return;
        }

        CryptKeyBase::CryptReturn aCryptRet(NULL,0);
        aCryptRet = mPrivateKey->Decrypt(aMsg.GetRawBuf(),aMsg.GetRawBufLen());

        auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

        if(aCryptRet.second<2 ||
            (*(unsigned short*)aCryptRet.first != aCryptRet.second-2) ||
            aCryptRet.second-2 != (*aClientSecret)->GetKeyLen() ||
            !memcmp(aCryptRet.first,(*aClientSecret)->GetKey(),aCryptRet.second-2))
        {
            titanDebug("FAIL: Invalid client secret received.");
            delete *aClientSecret; *aClientSecret = NULL;
            delete *aSessionKey; *aSessionKey = NULL;

            if(dirStuff)
                DirFailOver();
            else if(factStuff)
                FactFailOver();

            return;
        }

        delete *aClientSecret; *aClientSecret = NULL;

        if(*thePipeP==mDirPipe) {
            mDirInSeqNum = 1;
            mDirOutSeqNum = 1;
            mDirSessionId = aMsg.GetSessionId();
            DirHandleGetHWDirectory();
        }
        else if(*thePipeP==mFactPipe)
        {
            mFactSessionId = aMsg.GetSessionId();
            FactHandleStartProcess();
        }
        else if(routeStuff)
        {
            if (mNeedToRegisterRoutingServer)
                RegisterRoutingServer();
            else
                HandleRoutingRegister(aRouteNum);
        }
    }
    catch(WONException &anEx) {
        titanDebug("EXCEPTION: TitanInterface::PeerHandleComplete: %s", (char*)anEx.what());
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool TitanInterface::EncryptAndSendRoutingMsg(const WONMsg::BaseMessage &theMsgR, int theServer)
{
    titanDebug("TitanInterface::EncryptAndSendRoutingMsg");

    if(mRouteSessionKey[theServer]==NULL) {
        titanDebug("FAIL: Don't have routing server session key");
        return false;
    }

    // Use a MiniMessage to hold the encrypted output.  This has the added benefit of
    // signalling SendMsg to use a 2-byte length...
    MiniMessage anEncryptMsg;
    if(!EncryptMessage(theMsgR,anEncryptMsg,*(mRouteSessionKey[theServer]),0,NULL)) {
        titanDebug("FAIL: Failed to encrypt routing message.");
        return false;
    }

    if(!SendMsg(mRoutePipe[theServer], anEncryptMsg)) {
        titanDebug("Couldn't send msg even though this is impossible.");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool TitanInterface::EncryptMessage(const BaseMessage &theInMsg, BaseMessage &theOutMsg,
            const BFSymmetricKey &theKey, unsigned short theSessionId,
            unsigned short *theSeqNum)
{
    //titanDebug("TitanInterface::EncryptMessage.");

    if (theInMsg.GetMessageClass() == WONMsg::eTMessage)
    {
        titanDebug("EncryptTMessage FAIL: We no longer use encrypted TMessages!");
        return false;
    }
    else
        return EncryptNonTMessage(theInMsg, theOutMsg, theKey, theSessionId, theSeqNum);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*
bool TitanInterface::EncryptTMessage(const BaseMessage &theInMsg, BaseMessage &theOutMsg,
            const BFSymmetricKey &theKey, unsigned short theSessionId,
            unsigned short *theSeqNum)
{
    titanDebug("TitanInterface::EncryptTMessage.");

    if(theInMsg.GetDataLen() < theInMsg.GetHeaderLength())
        return false;

    const char *aDataPtr = (const char*)theInMsg.GetDataPtr();
    char *aBuffer = NULL;
    unsigned long anEncryptSize = theInMsg.GetDataLen();

    if(theSeqNum!=NULL) {
        aBuffer = new char[theInMsg.GetDataLen() + 2];
        memcpy(aBuffer+2,theInMsg.GetDataPtr(),theInMsg.GetDataLen());
        aDataPtr = aBuffer;
        *(unsigned short*)aDataPtr = *theSeqNum;
        anEncryptSize+=2;
    }

    auto_ptr<char> aDelBuf(aBuffer);

    theOutMsg.ResetBuffer();

    theOutMsg.AppendByte(WONMsg::EncryptedService); // Encrypt Flag
    if(theSessionId!=0)
        theOutMsg.AppendShort(theSessionId);

    CryptKeyBase::CryptReturn aCryptRet(NULL,0);
    aCryptRet = theKey.Encrypt(aDataPtr,anEncryptSize);

    auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

    if(aCryptRet.first==NULL)
        return false;

    theOutMsg.AppendBytes(aCryptRet.second, aCryptRet.first);

    if(theSeqNum!=NULL)
        (*theSeqNum)++;

    return true;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool TitanInterface::EncryptNonTMessage(const BaseMessage &theInMsg, BaseMessage &theOutMsg,
            const BFSymmetricKey &theKey, unsigned short theSessionId,
            unsigned short *theSeqNum)
{
    //titanDebug("TitanInterface::EncryptNonTMessage.");

    if(theInMsg.GetDataLen() < theInMsg.GetHeaderLength())
        return false;

    const char *aDataPtr = ((const char*)theInMsg.GetDataPtr()) + 1;
    char *aBuffer = NULL;
    unsigned long anEncryptSize = theInMsg.GetDataLen() - 1;

    if(theSeqNum!=NULL) {
        aBuffer = new char[theInMsg.GetDataLen() + 1];
        memcpy(aBuffer+2,((char*)theInMsg.GetDataPtr())+1,theInMsg.GetDataLen()-1);
        aDataPtr = aBuffer;
        *(unsigned short*)aDataPtr = *theSeqNum;
        anEncryptSize+=2;
    }

    auto_ptr<char> aDelBuf(aBuffer);

    theOutMsg.ResetBuffer();

    // Encrypt Flag
    switch (theInMsg.GetMessageClass())
    {
        case WONMsg::eMiniMessage:
            theOutMsg.AppendByte(WONMsg::MiniEncryptedService); break;
        case WONMsg::eSmallMessage:
            theOutMsg.AppendByte(WONMsg::SmallEncryptedService); break;
        case WONMsg::eLargeMessage:
            theOutMsg.AppendByte(WONMsg::LargeEncryptedService); break;
    }
    if(theSessionId!=0)
        theOutMsg.AppendShort(theSessionId);

    CryptKeyBase::CryptReturn aCryptRet(NULL,0);
    aCryptRet = theKey.Encrypt(aDataPtr,anEncryptSize);

    auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

    if(aCryptRet.first==NULL)
        return false;

    theOutMsg.AppendBytes(aCryptRet.second, aCryptRet.first);

    if(theSeqNum!=NULL)
        (*theSeqNum)++;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool TitanInterface::DecryptMessage(const char *theBuf, unsigned long theLen, BaseMessage &theOutMsg, SocketPipe **thePipePP)
{
    //titanDebug("TitanInterface::DecryptMessage");

    if (theOutMsg.GetMessageClass() == WONMsg::eTMessage)
    {
        titanDebug("DecryptTMessage FAIL: We no longer use encrypted TMessages!");
        return false;
    }
    else
        return DecryptNonTMessage(theBuf, theLen, theOutMsg, thePipePP);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*
bool TitanInterface::DecryptTMessage(const char *theBuf, unsigned long theLen, BaseMessage &theOutMsg, SocketPipe **thePipePP)
{
    titanDebug("TitanInterface::DecryptTMessage");

    BFSymmetricKey **aSessionKey;
    unsigned short aSessionId = 0, *aSeqNum = NULL;
    const char *aDataPtr = theBuf + 1;

    if(aSessionId!=0) {
        if(*(unsigned short*)aDataPtr!=aSessionId) {
            titanDebug("DecryptTMessage FAIL: Incorrect session id.");
            *thePipePP = NULL;
            delete *aSessionKey; *aSessionKey = NULL;

            FactFailOver();
            return false;
        }

        aDataPtr+=2;
    }

    CryptKeyBase::CryptReturn aCryptRet(NULL,0);
    aCryptRet = (*aSessionKey)->Decrypt((const unsigned char*)aDataPtr,theLen - (aDataPtr - theBuf));

    auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

    if(aCryptRet.first==NULL) {
        titanDebug("DecryptTMessage FAIL: Unable to decrypt message.");
        *thePipePP = NULL;
        delete *aSessionKey; *aSessionKey = NULL;

        FactFailOver();
        return false;
    }

    aDataPtr = (const char *)aCryptRet.first;
    if(aSeqNum!=NULL) {
        if(aCryptRet.second<2 || *aSeqNum!=*(unsigned short*)aDataPtr) {
            titanDebug("DecryptTMessage FAIL: Invalid sequence number received.");
            *thePipePP = NULL;
            delete *aSessionKey; *aSessionKey = NULL;

            FactFailOver();
            return false;
        }
        (*aSeqNum)++;
        aDataPtr+=2;
    }


    theOutMsg.ResetBuffer();

    theOutMsg.AppendBytes(aCryptRet.second - (aDataPtr - (const char*)aCryptRet.first), aDataPtr);

    if(theOutMsg.GetDataLen() < theOutMsg.GetHeaderLength()) {
        titanDebug("DecryptTMessage FAIL: Message too small.");

        FactFailOver();
        return false;
    }

    return true;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool TitanInterface::DecryptNonTMessage(const char *theBuf, unsigned long theLen, BaseMessage &theOutMsg, SocketPipe **thePipePP)
{
    //titanDebug("TitanInterface::DecryptNonTMessage");


    BFSymmetricKey **aSessionKey;
    BFSymmetricKey *aDummyKeyPtr = NULL;
    unsigned short aSessionId = 0, *aSeqNum = NULL;
    const char *aDataPtr = theBuf + 1;

    if (*thePipePP == mRoutePipe[0])
        aSessionKey = &(mRouteSessionKey[0]);
    else if (*thePipePP == mRoutePipe[1])
        aSessionKey = &(mRouteSessionKey[1]);
    else if (*thePipePP == mDirPipe)
    {
        aSessionKey = &mDirSessionKey;
        aSessionId = mDirSessionId;
        aSeqNum = &mDirInSeqNum;
    }
    else if(*thePipePP==mFactPipe)
    {
        aSessionKey = &mFactSessionKey;
        aSessionId = mFactSessionId;
    }
    else
        aSessionKey = &aDummyKeyPtr; // if it's not a server pipe, we assume that it is a game connection (i.e. use mGameKey)

    if(aSessionId!=0) {
        if(*(unsigned short*)aDataPtr!=aSessionId) {
            titanDebug("FAIL: Incorrect session id in non-Tmessage.");
            *thePipePP = NULL;
            delete *aSessionKey; *aSessionKey = NULL;
            return false;
        }

        aDataPtr+=2;
    }

    CryptKeyBase::CryptReturn aCryptRet(NULL,0);
    if (*aSessionKey)
        aCryptRet = (*aSessionKey)->Decrypt((const unsigned char*)aDataPtr,theLen - (aDataPtr - theBuf));
    else
        aCryptRet = mGameKey.Decrypt((const unsigned char*)aDataPtr,theLen - (aDataPtr - theBuf));

    auto_ptr<unsigned char> aDeleteCryptRet(aCryptRet.first);

    if(aCryptRet.first==NULL) {
        titanDebug("FAIL: Unable to decrypt non-Tmessage.");
        *thePipePP = NULL;
        delete *aSessionKey; *aSessionKey = NULL;
        return false;
    }

    aDataPtr = (const char *)aCryptRet.first;
    if(aSeqNum!=NULL) {
        if(aCryptRet.second<2 || *aSeqNum!=*(unsigned short*)aDataPtr) {
            titanDebug("FAIL: Invalid sequence number received on non-Tmessage.");
            *thePipePP = NULL;
            delete *aSessionKey; *aSessionKey = NULL;
            return false;
        }
        (*aSeqNum)++;
        aDataPtr+=2;
    }


    theOutMsg.ResetBuffer();

    theOutMsg.AppendByte(WONMsg::HeaderService1Message1);
    theOutMsg.AppendBytes(aCryptRet.second - (aDataPtr - (const char*)aCryptRet.first), aDataPtr);


    if(theOutMsg.GetDataLen() < theOutMsg.GetHeaderLength()) {
        titanDebug("FAIL: non-Tmessage too small.");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::FactHandleStartProcess(void)
{
    titanDebug("TitanInterface::FactHandleStartProcess");
    AutoCrit aCrit(mPipeCrit);

    if(mFactSessionKey==NULL) {
        titanDebug("FAIL: Don't have session key.");
        return;
    }

    TMessage anEncryptMsg;

    if(!EncryptMessage(mStartProcessMsg,anEncryptMsg,*mFactSessionKey,mFactSessionId,NULL)) {
        titanDebug("FAIL: Failed to encrypt start process message.");
        return;
    }

    if(!SendMsg(mFactPipe, anEncryptMsg))
    {
        titanDebug("FAIL: TitanInterface::FactHandleStartProcess!");
        FactFailOver();
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::RecordEvent(unsigned short theEventType)
{
    titanDebug("TitanInterface::RecordEvent");

    // Don't log events for LAN games.  Can't log events if no Event Servers.
    if (mIsLan || EVENTSERVER_NUM == 0)
        return;

    // If we don't have a certificate, we don't have a user-id...
    if (!mCertificate)
    {
        titanDebug("TitanInterface::RecordEvent mCertificate is NULL!");
        return;
    }

    WONMsg::SMsgEventTaggedRecordEvent aRecordEventMsg;

    try
    {
        aRecordEventMsg.SetHasDateTime(false);
        aRecordEventMsg.SetHasRelatedServer(false);
        aRecordEventMsg.SetHasRelatedClient(false);
        aRecordEventMsg.SetHasRelatedUser(true);
        aRecordEventMsg.SetUserAuthenticationMethod(USERAUTHENTICATIONMETHOD_AUTH1);
        aRecordEventMsg.SetUserId(mCertificate->GetUserId());
        aRecordEventMsg.SetActivityType(theEventType);
        aRecordEventMsg.SetTag(mEventTag++);

        // Add data that is specific to this particular type of event.
        //  LOBBY_ENTER (WON user ID, date/time, time zone, client type (version & language))
        //  LOBBY_EXIT  (WON user ID, date/time, duration of stay, behind firewall or not)
        //  GAME_CREATE (WON user ID, date/time, num players, routing-server or peer-to-peer)
        //  GAME_ENTER  (WON user ID, date/time)
        //  GAME_EXIT   (WON user ID, date/time, duration of game session)
        //  SYNC_ERROR  (WON user ID, date/time)
        //  CHEAT_DETECT(WON user ID, date/time)
        switch (theEventType)
        {
            case ACTIVITYTYPE_HOMEWORLD_LOBBY_ENTER:
            {
                // add timezone detail
                long aRealTimezone = _timezone;
                if (_daylight != 0)
                    aRealTimezone += 3600;
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_TIMEZONE, aRealTimezone);
                // add version detail
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_VERSION_STRING, StringToWString(versionString));
                // add language detail
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_HOMEWORLD_LANGUAGE, strCurLanguage);
                break;
            }
            case ACTIVITYTYPE_HOMEWORLD_LOBBY_EXIT:
                // add duration of stay detail
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_SESSION_LENGTH, time(NULL) - mLobbyEnterTime);
                // add firewall-presence detail
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_BEHIND_FIREWALL, mBehindFirewall);
                break;
            case ACTIVITYTYPE_HOMEWORLD_GAME_CREATE:
            {
                // add player count detail
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_PLAYER_COUNT, tpGameCreated.numPlayers);
                // add Internet game type detail (Routing Server or peer-to-peer)
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_USING_ROUTING_SERVER, tpGameCreated.userBehindFirewall);
                break;
            }
            case ACTIVITYTYPE_HOMEWORLD_GAME_EXIT:
                // add duration of game session detail
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_SESSION_LENGTH, time(NULL) - mGameStartTime);
                // add the disconnect code (voluntary/involuntary)
                aRecordEventMsg.AddDetail(ACTIVITYHISTORYDETAILTYPE_DISCONNECT_CODE, mGameDisconnectWasVoluntary);
                break;
        }

        // set/reset time variables
        switch (theEventType)
        {
            case ACTIVITYTYPE_HOMEWORLD_LOBBY_ENTER:
                mLobbyEnterTime = time(NULL); break;
            case ACTIVITYTYPE_HOMEWORLD_LOBBY_EXIT:
                mLobbyEnterTime = 0; // not really necessary, but what the heck...
            case ACTIVITYTYPE_HOMEWORLD_GAME_ENTER:
                mGameStartTime = time(NULL); break;
            case ACTIVITYTYPE_HOMEWORLD_GAME_EXIT:
                mGameStartTime = 0; break;
        }

        aRecordEventMsg.Pack();

        char *aBuf = new char[aRecordEventMsg.GetDataLen()];
        memcpy(aBuf,aRecordEventMsg.GetDataPtr(),aRecordEventMsg.GetDataLen());

        // Send request
        mEventPipe = new SocketPipe;
        mEventPipe->AddOutgoingCmd(new OpenCmd(EasySocket::UDP));
        for (int i = 0; i < NUM_EVENTS_TO_SEND; ++i)
        {
            mEventPipe->AddOutgoingCmd(new SendToCmd(reinterpret_cast<SOCKADDR&>(EVENTSERVER_ADDRESSES[0]), aBuf, aRecordEventMsg.GetDataLen(), (i == NUM_EVENTS_TO_SEND - 1), true));
            mEventPipe->AddOutgoingCmd(new TimerCmd(TIME_BETWEEN_EVENTS, true));
        }
        mEngine->AddPipe(mEventPipe);
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RecordEvent: %s", (char*)anEx.what());
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::DirHandleGetTitanServers(void)
{
    titanDebug("TitanInterface::DirHandleGetTitanServers");
    AutoCrit aCrit(mPipeCrit);

    // Build get dir msg
    WONMsg::SMsgDirG2GetEntity aMsg(WONMsg::SMsgDirG2KeyedBase::KT_DIRECTORY, true /* Ex version*/);
    try
    {
        aMsg.SetPath(TITANSERVER_DIR);
        aMsg.SetFlags(WONMsg::GF_DECOMPROOT | WONMsg::GF_DECOMPRECURSIVE | WONMsg::GF_DECOMPSERVICES | WONMsg::GF_ADDTYPE | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | WONMsg::GF_ADDDOTYPE | WONMsg::GF_ADDDODATA);
        aMsg.AddGetType(VALIDVERSIONS_OBJ.c_str(), VALIDVERSIONS_OBJ.size());
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::DirHandleGetTitanServers: %s", (char*)anEx.what());
        return;
    }

    // Connect and send request
    SOCKADDR_IN aDirAddr;
    EasySocket::getSockAddrIn(aDirAddr, DIRSERVER_IPSTRINGS[mCurDirServer], DIRSERVER_PORTS[mCurDirServer]);

    mDirPipe = ConnectAndSend(reinterpret_cast<SOCKADDR&>(aDirAddr), aMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::DirHandleGetHWDirectory(void)
{
    titanDebug("TitanInterface::DirHandleGetHWDirectory");
    AutoCrit aCrit(mPipeCrit);

    // Build get dir msg
    WONMsg::SMsgDirG2GetEntity aMsg(WONMsg::SMsgDirG2KeyedBase::KT_DIRECTORY, true /* Ex version*/);
    try
    {
        aMsg.SetPath(HOMEWORLD_DIR);
        aMsg.SetFlags(WONMsg::GF_DECOMPRECURSIVE | WONMsg::GF_DECOMPSERVICES | WONMsg::GF_DECOMPSUBDIRS | WONMsg::GF_ADDTYPE | WONMsg::GF_ADDDISPLAYNAME | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | WONMsg::GF_DIRADDNAME | WONMsg::GF_ADDDOTYPE | WONMsg::GF_ADDDODATA);
        aMsg.AddGetType(DESCRIPTION_OBJ.c_str(),             DESCRIPTION_OBJ.size());
        aMsg.AddGetType(ROOM_FLAGS_OBJ.c_str(),              ROOM_FLAGS_OBJ.size());
        aMsg.AddGetType(ROOM_CLIENTCOUNT_OBJ.c_str(),        ROOM_CLIENTCOUNT_OBJ.size());
        aMsg.AddGetType(FACT_CUR_SERVER_COUNT_OBJ.c_str(),   FACT_CUR_SERVER_COUNT_OBJ.size());
        aMsg.AddGetType(FACT_TOTAL_SERVER_COUNT_OBJ.c_str(), FACT_TOTAL_SERVER_COUNT_OBJ.size());
        aMsg.AddGetType(SERVER_UPTIME_OBJ.c_str(),           SERVER_UPTIME_OBJ.size());
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::DirHandleGetHWDirectory: %s", (char*)anEx.what());
        return;
    }

    // Connect and send request
    SOCKADDR_IN aDirAddr;
    EasySocket::getSockAddrIn(aDirAddr, DIRSERVER_IPSTRINGS[mCurDirServer], DIRSERVER_PORTS[mCurDirServer]);

    TMessage anEncryptMsg;

    if(mDirSessionKey==NULL) {
        titanDebug("FAIL: Don't have session key.");
        return;
    }

    if(!EncryptMessage(aMsg,anEncryptMsg,*mDirSessionKey,mDirSessionId,&mDirOutSeqNum)) {
        titanDebug("FAIL: Failed to encrypt get dir message.");
        return;
    }

    if(mDirPipe!=NULL)
    {
        if(!SendMsg(mDirPipe, anEncryptMsg))
        {
            titanDebug("FAIL: TitanInterface::DirHandleGetHWDirectory!");
            return;
        }
    }
    else
        mDirPipe = ConnectAndSend(reinterpret_cast<SOCKADDR&>(aDirAddr), anEncryptMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::DirStartPeerLogin(void)
{
    titanDebug("TitanInterface::StartPeerLogin");

    if(mCertificate==NULL) {
        titanDebug("FAIL: Don't have certificate.");
        return;
    }

    WONMsg::TMsgAuth1Request aMsg;

    try
    {
        aMsg.SetAuthMode(WONAuth::AUTH_SESSION);
        aMsg.SetEncryptMode(WONAuth::ENCRYPT_BLOWFISH);
        aMsg.SetEncryptFlags(WONAuth::EFLAGS_NONE);
        aMsg.SetRawBuf(mCertificate->GetRaw(),mCertificate->GetRawLen());
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::DirStartPeerLogin: %s", (char*)anEx.what());
        return;
    }

    // Connect and send request
    SOCKADDR_IN aDirAddr;
    EasySocket::getSockAddrIn(aDirAddr, DIRSERVER_IPSTRINGS[mCurDirServer], DIRSERVER_PORTS[mCurDirServer]);
    mDirPipe = ConnectAndSend(reinterpret_cast<SOCKADDR&>(aDirAddr), aMsg);
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::RequestDirectory()
{
    // Toss old connection if needed
    titanDebug("TitanInterface::RequestDirectory");
    AutoCrit aCrit(mPipeCrit);

    if (mDirPipe)
    {
        titanDebug("NOTE: TitanInterface::RequestDirectory already in progress.");
        return;
    }
    aCrit.Leave();

    if (AUTHSERVER_NUM==0)
        DirHandleGetTitanServers();
    else if(mDirSessionKey!=NULL)
        DirHandleGetHWDirectory();
    else
        DirStartPeerLogin();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::RequestCreateGame(const wchar_t* theGame, DirectoryCustomInfo* myInfo)
{
    titanDebug("TitanInterface::RequestCreateGame");

    if (! mRoutePipe[0])
    {
        titanDebug("FAIL: TitanInterface::RequestCreateGame, no RoutingPipe!");
        return;
    }

    mGameKey.Create(GAMEKEY_SIZE);
    memcpy(myInfo->sessionKey,mGameKey.GetKey(),GAMEKEY_SIZE);

    AutoCrit aCrit(mRoutingCrit);
    WONMsg::MMsgRoutingCreateDataObject aCreateDataObjectMsg;
    try
    {
        aCreateDataObjectMsg.SetLinkId(0 /* ALL_USERS */);
        aCreateDataObjectMsg.SetOwnerId(mMyClientId[0]);
        aCreateDataObjectMsg.SetLifespan(0);
        aCreateDataObjectMsg.SetDataType(gameTag);
        aCreateDataObjectMsg.AppendToDataType(WONCommon::RawBuffer((unsigned char*)theGame, wcslen(theGame)*2));
        aCreateDataObjectMsg.SetData(WONCommon::RawBuffer((unsigned char*)myInfo, sizeof(*myInfo) + myInfo->stringdatalength - 1));
        aCreateDataObjectMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RequestCreateGame: %s", (char*)anEx.what());
        return;
    }

    if (! EncryptAndSendRoutingMsg(aCreateDataObjectMsg,0))
    {
        titanDebug("FAIL: TitanInterface::SendMsg, send CreateGame");
    }
    else
    {
        mWaitingRequestQueue[0].push_back(WONMsg::RoutingCreateDataObject);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::RequestDeleteGame(const wchar_t* theGame)
{
    titanDebug("TitanInterface::RequestDeleteGame");

    if (! mRoutePipe[0])
    {
        titanDebug("FAIL: TitanInterface::RequestDeleteGame, no RoutingPipe!");
        return;
    }

    AutoCrit aCrit(mRoutingCrit);
    WONMsg::MMsgRoutingDeleteDataObject aDeleteDataObjectMsg;
    try
    {
        aDeleteDataObjectMsg.SetLinkId(0 /* ALL_USERS */);
        aDeleteDataObjectMsg.SetDataType(gameTag);
        aDeleteDataObjectMsg.AppendToDataType(WONCommon::RawBuffer((unsigned char*)theGame, wcslen(theGame)*2));
        aDeleteDataObjectMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RequestDeleteGame: %s", (char*)anEx.what());
        return;
    }

    if (! EncryptAndSendRoutingMsg(aDeleteDataObjectMsg,0))
    {
        titanDebug("FAIL: TitanInterface::SendMsg, send DeleteGame");
    }
    mWaitingRequestQueue[0].push_back(WONMsg::RoutingDeleteDataObject);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::RequestReplaceGame(const wchar_t* theGame, DirectoryCustomInfo* myInfo, unsigned long replaceTimeout)
{
    titanDebug("TitanInterface::RequestReplaceGame");

    if (! mRoutePipe[0])
    {
        titanDebug("FAIL: TitanInterface::RequestReplaceGame, no RoutingPipe!");
        return;
    }

    AutoCrit aCrit(mRoutingCrit);
    WONMsg::MMsgRoutingReplaceDataObject aReplaceDataObjectMsg;
    try
    {
        aReplaceDataObjectMsg.SetLinkId(0 /* ALL_USERS */);
        aReplaceDataObjectMsg.SetDataType(gameTag);
        aReplaceDataObjectMsg.AppendToDataType(WONCommon::RawBuffer((unsigned char*)theGame, wcslen(theGame)*2));
        aReplaceDataObjectMsg.SetData(WONCommon::RawBuffer((unsigned char*)myInfo, sizeof(*myInfo) + myInfo->stringdatalength - 1));
        aReplaceDataObjectMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RequestReplaceGame: %s", (char*)anEx.what());
        return;
    }

    // send the replace request
    if (! EncryptAndSendRoutingMsg(aReplaceDataObjectMsg,0))
    {
        titanDebug("FAIL: TitanInterface::SendMsg, send ReplaceGame");
    }

    mWaitingRequestQueue[0].push_back(WONMsg::RoutingReplaceDataObject);

    // send a renew request if needed
    if (replaceTimeout != 0)
        RequestRenewGameLifespan(theGame, TITAN_GAME_EXPIRE_TIME);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::RequestRenewGameLifespan(const wchar_t* theGame, unsigned long newLifespan)
{
    titanDebug("TitanInterface::RequestRenewGameLifespan");

    if (! mRoutePipe[0])
    {
        titanDebug("FAIL: TitanInterface::RequestRenewGameLifespan, no RoutingPipe!");
        return;
    }

    AutoCrit aCrit(mRoutingCrit);
    WONMsg::MMsgRoutingRenewDataObject aRenewDataObjectMsg;
    try
    {
        aRenewDataObjectMsg.SetLinkId(0 /* ALL_USERS */);
        aRenewDataObjectMsg.SetDataType(gameTag);
        aRenewDataObjectMsg.AppendToDataType(WONCommon::RawBuffer((unsigned char*)theGame, wcslen(theGame)*2));
        aRenewDataObjectMsg.SetNewLifespan(newLifespan);
        aRenewDataObjectMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RequestReplaceGame: %s", (char*)anEx.what());
        return;
    }

    // send the renew request
    if (! EncryptAndSendRoutingMsg(aRenewDataObjectMsg,0))
    {
        titanDebug("FAIL: TitanInterface::SendMsg, send Renew(lifespan)");
    }

    mWaitingRequestQueue[0].push_back(WONMsg::RoutingRenewDataObject);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::ConnectToClient(Address* theAddressP)
{
    if(mUseRoutingServer)
        return;

    ClientNetAddr aCAddr = MakeClientNetAddr(theAddressP);

    AutoCrit aCrit(mClientCrit);
    ClientToPipe::iterator anItr = mClientMap.find(aCAddr);

    // If no existing connection...
    if (anItr == mClientMap.end())
    {
        SocketPipe* aPipeP = ConnectTo(*theAddressP, mStreamType, WONMisc::ptUnsignedShort);
        if (! aPipeP)
        {
            titanDebug("FAIL: TitanInterface::ConnectToClient, fail ConnectTo");
            return;
        }

        // Add pipe to engine and update client map
        mEngine->AddPipe(aPipeP);
        mClientMap[aCAddr].pipe = aPipeP;

        aCrit.Leave();


        if(mGameCreationState==GAME_STARTED && AddressesAndPortsAreEqual(*theAddressP,mCaptainAddress))
            mCaptainPipe = aPipeP;
    }
    else if(mGameCreationState==GAME_STARTED && AddressesAndPortsAreEqual(*theAddressP,mCaptainAddress))
        mCaptainPipe = anItr->second.pipe;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

ClientNetAddr
TitanInterface::MakeClientNetAddr(Address* theAddressP)
{
    ClientNetAddr aCAddr;

    if ( mIpType == ip )
    {
        aCAddr.first = theAddressP->AddrPart.IP;
        aCAddr.second = theAddressP->Port;
    }
    else
    {
        unsigned long aFirstLong;
        unsigned long aSecondLong;
        memcpy( &aFirstLong, theAddressP->AddrPart.etherAddr, 4 );
        ((char*)(&aSecondLong))[0] = theAddressP->AddrPart.etherAddr[4];
        ((char*)(&aSecondLong))[1] = theAddressP->AddrPart.etherAddr[5];
        ((char*)(&aSecondLong))[2] = ((char*)(&(theAddressP->Port)))[0];
        ((char*)(&aSecondLong))[3] = ((char*)(&(theAddressP->Port)))[1];
        aCAddr.first = aFirstLong;
        aCAddr.second = aSecondLong;
    }
    return aCAddr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::SendPing(Address* theAddressP,unsigned int pingsizebytes)
{
    // Build change user msg
    WONMsg::MMsgCommPing aMsg;
    try
    {
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::SendPing: %s", (char*)anEx.what());
        return;
    }

    // Connect and send request
    ConnectAndSend(*theAddressP, aMsg, mStreamType, (pingsizebytes == 4) ? WONMisc::ptUnsignedLong : WONMisc::ptUnsignedShort);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long
TitanInterface::StartRoutingServer(const wchar_t* theChannelName, const wchar_t* theChannelDescription, const wchar_t* thePassword, bool isGameServer,unsigned char *routingaddress)
{
    titanDebug("TitanInterface::StartRoutingServer");
    AutoCrit aPipeCrit(mPipeCrit);
    unsigned int roomflags = 0;

    if(mCertificate==NULL) {
        titanDebug("FAIL: Don't have certificate.");
        return 0;
    }

    // Toss old connection if needed
    if (mFactPipe)
    {
        titanDebug("NOTE: TitanInterface::StartRoutingServer Close existing FactPipe!");
        mFactPipe->AddOutgoingCmd(new CloseNowCmd(false, false, 0, false));
        mFactPipe = NULL;
    }
    aPipeCrit.Leave();

    try
    {
        // Build the command line
        wstring aCmdLine;
        if (thePassword && thePassword[0] != '\0')
        {
            aCmdLine += L" -Password ";
            aCmdLine += thePassword;
            roomflags |= ROOMFLAGS_PASSWORDPROTECTED;
        }

        mStartProcessMsg.SetAddCmdLine(true);
        mStartProcessMsg.SetCmdLine(aCmdLine);

        // Build start process msg
        if(isGameServer)
            mStartProcessMsg.SetProcessName(ROUTINGSERV_GAME);
        else
            mStartProcessMsg.SetProcessName(ROUTINGSERV_CHAT);

        if(theChannelName[0]!='\0') {
            WONCommon::DataObjectTypeSet aDOSet;
            aDOSet.insert(WONCommon::DataObject(DESCRIPTION_OBJ, WONCommon::RawBuffer(reinterpret_cast<const unsigned char*>(theChannelDescription),wcslen(theChannelDescription) * 2)));
            aDOSet.insert(WONCommon::DataObject(ROOM_FLAGS_OBJ, WONCommon::RawBuffer((unsigned char*)&roomflags,4)));
            mRegisterRoutingServerMsg.SetDataObjects(aDOSet);
            mRegisterRoutingServerMsg.SetDisplayName(theChannelName);
        }

        mStartProcessMsg.SetTotalPorts(1);
        mStartProcessMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::StartRoutingServer: %s", (char*)anEx.what());
        return 0;
    }

    WONMsg::TMsgAuth1Request aMsg;

    try
    {
        aMsg.SetAuthMode(WONAuth::AUTH_SESSION);
        aMsg.SetEncryptMode(WONAuth::ENCRYPT_BLOWFISH);
        aMsg.SetEncryptFlags(WONAuth::EFLAGS_NOTSEQUENCED);
        aMsg.SetRawBuf(mCertificate->GetRaw(),mCertificate->GetRawLen());
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::StartRoutingServer: %s", (char*)anEx.what());
        return 0;
    }

    if (!routingaddress)
    {
        if(FACTSERVER_NUM==0) {
            return 0;
        }

        if(mCurFactServer>=FACTSERVER_NUM)
            mCurFactServer%=FACTSERVER_NUM;

        mFailFactOverDirectly = false;
    }
    else
    {
        mFailFactOverDirectly = true;
    }

    // Connect and send request
    AutoCrit aStartRoutingCrit(mStartRoutingCrit);
    mChannelName = theChannelName;
    mChannelDescription = theChannelDescription;
    mRoomPassword = (thePassword==NULL) ? L"" : thePassword;
    mIsGameServer = isGameServer;

    if (!routingaddress)
    {
        mFactPipe = ConnectAndSend(FACTSERVER_ADDRESSES[mCurFactServer], aMsg);
    }
    else
    {
        SOCKADDR_IN buildaddr;
        BuildAddress(buildaddr, routingaddress);

        mFactPipe = ConnectAndSend(buildaddr, aMsg);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::SendLanBroadcast(const void* thePacket, unsigned short theLen)
{
    if ( mLanAdPipe != NULL )
    {
        char* aBufferP = new char[theLen];
        memcpy(aBufferP,thePacket,theLen);
        mLanAdPipe->AddOutgoingCmd(new BroadcastCmd(AD_PORT,aBufferP,theLen,true,true));
    }
    else
    {
        titanDebug("TitanInterface::SendLanBroadcast called when mLadAdSocket == NULL");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::SendPacketTo(Address* theAddressP, unsigned char titanMsgType,
                             const void* thePacket, unsigned short theLen,
                                                        bool addSeqNum, int theSeqNum)
{

    TitanPacketMsg aMsg(titanMsgType, !mUseRoutingServer);
    try
    {
        // insert the data into the packet
        if (theLen > 0)
            aMsg.SetBlob(thePacket, theLen);

        // insert the game name into the packet
        LockMutex(GameWereInterestedInMutex);
        aMsg.SetGameName(GameWereInterestedIn);
        UnLockMutex(GameWereInterestedInMutex);

        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::SendPacketTo: %s", (char*)anEx.what());
        return;
    }

    if(mUseRoutingServer)
    {
        int aRouteNum = 0;

        if(mGameCreationState==GAME_STARTED) aRouteNum = 1;
        if(theAddressP!=NULL)
            RoutingSendData(theAddressP->Port, aMsg.GetDataLen(), (const unsigned char*)aMsg.GetDataPtr(),aRouteNum,addSeqNum,theSeqNum);
        else
            RoutingSendDataBroadcast(aMsg.GetDataLen(), (const unsigned char*)aMsg.GetDataPtr(),aRouteNum,addSeqNum,theSeqNum);
    }
    else
    {
        AutoCrit aCrit(mClientCrit);  // Enter client crit sec
        ClientToPipe::iterator anItr = mClientMap.find(MakeClientNetAddr(theAddressP));
//      ClientToPipe::iterator anItr = mClientMap.find(ClientNetAddr(theIP, thePort));
        if (anItr != mClientMap.end())
        {
#if DEBUG_DISCONNECT
            string aStr((const char*)thePacket,theLen);
            if(aStr.find("briandis")!=string::npos)
            {
                anItr->second.pipe->AddOutgoingCmd(new CloseCmd(false, false, 0, false));
                return;
            }
#endif // DEBUG_DISCONNECT

            MiniMessage anEncryptMsg;
            if(!EncryptMessage(aMsg, anEncryptMsg, mGameKey, 0, NULL)) {
                titanDebug("FAIL: Failed to encrypt packet message.");
                return;
            }

            if (! SendMsg(anItr->second.pipe, anEncryptMsg))
            {
                titanDebug("FAILED: TitanInterface:SendPacketTo, send packet to client!");
                return;
            }

#ifdef CLOCK_COMMANDS_THRU_ENGINE
            anItr->second.pipe->AddOutgoingCmd(new NoOpPayloadCmd(GetTickCount(), false));
#endif // CLOCK_COMMANDS_THRU_ENGINE
        }
        else
        {
            char addressstring[50];
            PrintAddressToString(addressstring,theAddressP);
            titanDebug("TitanInterface::SendPacketTo unknown client %s",addressstring);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

TitanInterface::threadProcess()
{
    titanDebug("TitanInterface::threadProcess Starting up.");

    // Listen for connections
    SocketPipe* aListenP = new SocketPipe;
    aListenP->AddOutgoingCmd(new OpenCmd(mStreamType));
    aListenP->AddOutgoingCmd(new BindCmd(GAME_PORT));
    aListenP->AddIncomingCmd(new ListenCmd);

    // Stuff in a bunch of accept commands
    for (int i=0; i < PREPOST_MAX; i++)
        aListenP->AddIncomingCmd(new AcceptCmd(mEngine, true, false));

    // Add listen pipe to engine
    mEngine->AddPipe(aListenP);

//    if ( mIpType == ipx )
    if ( mIsLan == true )
    {
        // LAN Ad Pipe
        mLanAdPipe = new WONMisc::SocketPipe;
        mLanAdPipe->AddOutgoingCmd(new OpenCmd(mDatagramType));
        mLanAdPipe->AddOutgoingCmd(new BindCmd(AD_PORT, true));

#if 0
        // make LAN Ad Pipe socket linger
        linger aLingerStruct;
        aLingerStruct.l_onoff = 1;
        aLingerStruct.l_linger = 120;
        mLanAdPipe->mEasySocketP->setOption(SOL_SOCKET, SO_LINGER, reinterpret_cast<const char*>(&aLingerStruct), sizeof(aLingerStruct));
#endif
        // Stuff in a bunch of recvfrom commands
        for (int i=0; i < PREPOST_MAX; i++)
            mLanAdPipe->AddIncomingCmd(new RecvFromCmd(new char[MAX_LAN_ADVERT_LEN],MAX_LAN_ADVERT_LEN,true,false));

        // Add listen pipe to engine
        mEngine->AddPipe(mLanAdPipe);

        // get the local MAC address
        if ( mIpType == ipx )
            mLanAdPipe->mEasySocketP->getLocalAddr(myAddress.AddrPart.etherAddr);
        else
            myAddress.AddrPart.IP = GetLocalIPAddress();
        myAddress.Port = GAME_PORT;

        char addrstr[100];
        PrintAddressToString(addrstr,&myAddress);
        titanDebug("TitanInterface::LAN myAddress %s",addrstr);
    }

    // Main loop
    titanDebug("TitanInterface::threadProcess Staring Main Loop.");
    for (;;)
    {
        if (WaitForSingleObject(getStopEvent(), 0) == WAIT_OBJECT_0)
        {
            titanDebug("TitanInterface::threadProcess Stop requested, shutting down.");
            break;
        }

        SocketPipe* aPipeP = mEngine->GetCompletedPipe(INFINITE);
        if (aPipeP)
        {
            if (aPipeP->mInErrorState)
            {
                titanDebug("TitanInterface::threadProcess Pipe in ERROR state, closing!");
                HandleCloseCmd(aPipeP);
            }
            else
            {
                numPendingIn = aPipeP->GetNumPendingIncomingCmds();
                if (numPendingIn > 20)
                {
                    titanDebug("TitanInterface::In Queue too high at %d",numPendingIn);
                }
                numPendingOut = aPipeP->GetNumPendingOutgoingCmds();
                if (numPendingOut > 20)
            {
                    titanDebug("TitanInterface::Out Queue too high at %d",numPendingOut);
                }

                auto_ptr<PipeCmd> aCmdP(aPipeP->RemoveCompletedCmd());
                if (aCmdP.get())
                {
                    switch (aCmdP->GetType())
                    {
                        case WONMisc::pctWaitForEvent:
                            HandleWaitCmd(aCmdP.get());
                            break;
                        case WONMisc::pctAccept:
                            HandleAcceptCmd(aPipeP, aCmdP.get());
                            break;
                        case WONMisc::pctClose:
                        case WONMisc::pctCloseNow:
                            HandleCloseCmd(aPipeP);
                            break;
                        case WONMisc::pctRecvPrefixed:
                            if(aPipeP==mCaptainPipe)
                                mCaptainReconnectNum = 0;
                            HandleRecvCmd(aPipeP, aCmdP.get(), false);
                            break;
                        case WONMisc::pctRecvFrom:
                            {
                                if(aPipeP==mCaptainPipe)
                                    mCaptainReconnectNum = 0;

                                RecvFromCmd* aRecvFromCmdP = dynamic_cast<RecvFromCmd*>(aCmdP.get());
                                if(aPipeP==mRoutingInfoPipe) {
                                    aPipeP->AddIncomingCmd(new RecvFromCmd(new char[32],32,true,false));
                                    HandleMiniMsg(aPipeP, aRecvFromCmdP->mBufferP, aRecvFromCmdP->mBytesReceived);
                                }
                                else
                                    HandleLanBroadcastMsg(aPipeP, aRecvFromCmdP->mBufferP, aRecvFromCmdP->mBytesReceived );
                            }
                            break;
                        case WONMisc::pctTimer:
                            if(aPipeP==mTimerPipe)
                                AuthHandleRefresh();
                            break;
#ifdef CLOCK_COMMANDS_THRU_ENGINE
                        case WONMisc::pctNoOpPayload:
                            {
                            NoOpPayloadCmd* aPayloadCmdP = reinterpret_cast<NoOpPayloadCmd*>(aCmdP.get());
                            titanDebug("TitanInterface::threadProcess Payload delay = %d ms", GetTickCount() - aPayloadCmdP->GetPayload());
                            break;
                            }
#endif // CLOCK_COMMANDS_THRU_ENGINE
                        default:
                            titanDebug("TitanInterface::threadProcess Got unknown command type.");
                            break;
                        }
                }
            }
        }
    }

    titanDebug("TitanInterface::threadProcess Shutting down.");
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleWaitCmd(PipeCmd* theCmdP)
{
    titanDebug("TitanInterface::HandleWaitCommand Got wait command");
    WaitForEventCmd* aWaitCmdP = dynamic_cast<WaitForEventCmd*>(theCmdP);
    if (aWaitCmdP)
        CloseHandle(aWaitCmdP->mEvent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleAcceptCmd(SocketPipe* thePipeP, PipeCmd* theCmdP)
{
    titanDebug("TitanInterface::HandleAcceptCmd Got accept command");
    AcceptCmd* anAcceptCmdP = dynamic_cast<AcceptCmd*>(theCmdP);

    // Add recv commands to new pipe
    if (anAcceptCmdP)
    {
        for (int i=0; i < PREPOST_MAX; i++)
            anAcceptCmdP->mAcceptedPipeP->AddIncomingCmd(new RecvPrefCmd(false, WONMisc::ptUnsignedShort, false));
    }

    // Add another accept command to pipe
    thePipeP->AddIncomingCmd(new AcceptCmd(mEngine, true, false));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleCloseCmd(SocketPipe* thePipeP)
{
    titanDebug("TitanInterface::HandleCloseCmd Got closed pipe with error %s (%s:%d)", WONMisc::ES_ErrorTypeToString(thePipeP->mError).c_str(), thePipeP->mEasySocketP->getDestAddrString().c_str(), thePipeP->mEasySocketP->getDestPort());

    // Flush the pipe of any incoming recv commands
    while (thePipeP->HasCompletedIncomingCmds())
    {
        auto_ptr<PipeCmd> aCmdP(thePipeP->RemoveCompletedIncomingCmd());
        if (aCmdP->GetType() == WONMisc::pctRecvPrefixed)
            HandleRecvCmd(thePipeP, aCmdP.get(), true);
    }

    // Check unpected closes on pipes we're waiting on
    AutoCrit aPipeCrit(mPipeCrit);
    if (thePipeP == mAuthPipe)
    {
        titanDebug("TitanInterface::HandleCloseCmd AuthPipe closed unexpectedly!");
        mAuthPipe = NULL;
        aPipeCrit.Leave();
        AuthFailOver();
    }
    else if (thePipeP == mDirPipe)
    {
        titanDebug("TitanInterface::HandleCloseCmd DirPipe closed unexpectedly!");
        mDirPipe = NULL;
        aPipeCrit.Leave();
        DirFailOver();
    }
    else if (thePipeP == mEventPipe)
    {
        titanDebug("TitanInterface::HandleCloseCmd EventPipe closed unexpectedly!");
        mEventPipe = NULL;
        aPipeCrit.Leave();
    }
    else if (thePipeP == mFactPipe)
    {
        titanDebug("TitanInterface::HandleCloseCmd FactPipe closed unexpectedly!");
        mFactPipe = NULL;
        aPipeCrit.Leave();
        FactFailOver();
    }
    else if (thePipeP == mRoutePipe[0])
    {
        titanDebug("TitanInterface::HandleCloseCmd RoutePipe closed unexpectedly!");
        mRoutePipe[0] = NULL;
        mNeedToRegisterRoutingServer = false;
        mWaitingRequestQueue[0].clear();
        aPipeCrit.Leave();
        cNotifyChatConnectFailed();
    }
    else if (thePipeP == mRoutePipe[1])
    {
        titanDebug("TitanInterface::HandleCloseCmd Game RoutePipe closed unexpectedly!");
        clCommandMessage("Lost connection...reconnecting");
        mRoutePipe[1] = NULL;
        mWaitingRequestQueue[1].clear();
        aPipeCrit.Leave();
        if(mRoutingReconnectNum[1]++ < 3)
            ConnectToRoutingServer(L"SuckMe",L"MoFo",1,true);
        else
        {
            mGameDisconnectWasVoluntary = false;
            RecordEvent(ACTIVITYTYPE_HOMEWORLD_GAME_EXIT);
        }
    }
    else if(thePipeP == mFirewallPipe)
    {
        titanDebug("TitanInterface::HandleCloseCmd FirewallPipe closed unexpectedly!");
        mFirewallPipe = NULL;
        aPipeCrit.Leave();
        FirewallFailOver();
    }
    else if(thePipeP == mFactPingPipe)
    {
        titanDebug("TitanInterface::HandleCloseCmd FactPingPipe closed unexpectedly!");
        aPipeCrit.Leave();
        PingFactServer();
    }
    else if(thePipeP == mRoutingInfoPipe || thePipeP == mLanAdPipe)
    {
        if(thePipeP->mInErrorState && thePipeP->mError==WONMisc::ES_WSAEMSGSIZE)
        {
            thePipeP->mInErrorState = false;
            thePipeP->mError = WONMisc::ES_NO_ERROR;
            return;
        }
        else
        {
            titanDebug("TitanInterface::HandleCloseCmd RoutingInfoPipe closed! (Error: %d)", thePipeP->mError);
            if (thePipeP == mRoutingInfoPipe)
                mRoutingInfoPipe = NULL;
            else
                mLanAdPipe = NULL;
            aPipeCrit.Leave();
        }
    }
    else
    {
        aPipeCrit.Leave();

        // Check for offline clients
        AutoCrit aCrit(mClientCrit);  // Enter client crit sec
        ClientToPipe::iterator anItr = mClientMap.begin();
        for (; anItr != mClientMap.end(); anItr++)
            if (anItr->second.pipe == thePipeP)
                break;

        if (anItr != mClientMap.end())
        {
            titanDebug("TitanInterface::HandleCloseCmd Client pipe closed!");
            if(IAmCaptain && mGameCreationState!=GAME_STARTED)
            {
                Address anAddr;
                if(!mIsLan)
                {
                    anAddr.AddrPart.IP = anItr->first.first;
                    anAddr.Port = anItr->first.second;
                }
                else
                {
                    memcpy(anAddr.AddrPart.etherAddr,&(anItr->first.first),4);
                    memcpy(anAddr.AddrPart.etherAddr + 4, &(anItr->first.second),2);
                    memcpy(&anAddr.Port,((char*)(&(anItr->first.second)))+2,2);
                }
                titanLeaveGameReceivedCB(&anAddr, NULL, 0);
            }

            mClientMap.erase(anItr);

            if(thePipeP == mCaptainPipe && mGameCreationState==GAME_STARTED)
            {
                if (mCaptainReconnectNum < 3)
                {
                    clCommandMessage("Lost captain conn...reconnecting");
                    mCaptainPipe = NULL;
                    mCaptainReconnectNum++;
                    ConnectToClient(&mCaptainAddress);
                    SendPacketTo(&mCaptainAddress,TITANMSGTYPE_RECONNECT,NULL,0);
                }
                else
                {
                    mGameDisconnectWasVoluntary = false;
                    RecordEvent(ACTIVITYTYPE_HOMEWORLD_GAME_EXIT);
                }
            }

            if (mClientMap.empty() && mCloseRequest)
            {
                titanNoClientsCB();
                mCloseRequest = false;
            }
        }
    }

    // Kill the Pipe
    mEngine->KillPipe(thePipeP);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleRecvCmd(SocketPipe* thePipeP, PipeCmd* theCmdP, bool pipeClosed)
{
    // Cast to real command type
    RecvPrefCmd* aRecvCmdP = dynamic_cast<RecvPrefCmd*>(theCmdP);
    if (! aRecvCmdP)
    {
        titanDebug("TitanInterface::HandleRecvCmd Corrupt command discarded, bad conversion!");
        return;
    }

    // Check for closed connection (empty buffer)
    if (aRecvCmdP->mBytesReceived == 0)
    {
        titanDebug("TitanInterface::HandleRecvCmd Empty buffer, pipe closed!");
        if (! pipeClosed)
            HandleCloseCmd(thePipeP);
        return;
    }

    // Post a new recv command
    if (! pipeClosed)
        thePipeP->AddIncomingCmd(new RecvPrefCmd(false, aRecvCmdP->mPrefixType, false));

    // Dispatch message
    switch (aRecvCmdP->mBufferP[0])
    {
        case WONMsg::HeaderService2Message2:
        case WONMsg::SmallEncryptedService:
            HandleSmallMsg(thePipeP, aRecvCmdP->mBufferP, aRecvCmdP->mBytesReceived); break;
        case WONMsg::HeaderService1Message1:
        case WONMsg::MiniEncryptedService:
            HandleMiniMsg(thePipeP, aRecvCmdP->mBufferP, aRecvCmdP->mBytesReceived); break;
        case WONMsg::HeaderService4Message4:
        case WONMsg::LargeEncryptedService:
            titanDebug("TitanInterface::HandleRecvCmd Command discarded.  Message received with unused header type."); break;
        // NOTE: The default case handles some messages that you might not expect.  For example,
        //       the Routing pipes are set up to use 2-byte prefixed recv commands, but peer-to-
        //       peer authentication ALWAYS uses 4-byte lengths.  Since the peer-to-peer messages
        //       are expected to be less than 2^16 bytes long and the lengths are little endian,
        //       we can receive them on a 2-byte prefixed pipe.  Then when we look at the first
        //       byte (normally the header type byte), it will be 0 and fall into the default
        //       case:
        default:
            HandleTitanMsg(thePipeP, aRecvCmdP->mBufferP, aRecvCmdP->mBytesReceived); break;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


void
TitanInterface::HandleLanBroadcastMsg(WONMisc::SocketPipe* thePipeP, const char* theBufP, unsigned long theLen)
{
// put a new recvfrom on the pipe
thePipeP->AddIncomingCmd(new RecvFromCmd(new char[MAX_LAN_ADVERT_LEN],MAX_LAN_ADVERT_LEN,true,false));

// do the callback
titanReceivedLanBroadcastCB(theBufP,theLen);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleTitanMsg(SocketPipe* thePipeP, const char* theBufP, unsigned long theLen)
{
    TMessage aMsg;
    if (theLen < aMsg.GetHeaderLength())
    {
        titanDebug("TitanInterface::HandleTitanMsg Corrupt Titan Message discarded, bad length!");
        return;
    }

    if(theBufP[0]==WONMsg::EncryptedService) { // Encrypted Message
        if(!DecryptMessage(theBufP,theLen,aMsg,&thePipeP))
            return; // Failed to decrypt
    }
    else {
        // Fill in TMsg header
        aMsg.ResetBuffer();
        aMsg.AllocateHeaderBuffer();
        aMsg.SetServiceType(*(reinterpret_cast<const unsigned long*>(theBufP)));
        theBufP += sizeof(unsigned long);
        aMsg.SetMessageType(*(reinterpret_cast<const unsigned long*>(theBufP)));
        theBufP += sizeof(unsigned long);
        aMsg.Pack();

        // Add msg data if any
        unsigned long aDataLen = theLen - aMsg.GetHeaderLength();
        if (aDataLen > 0)
            aMsg.AppendBytes(aDataLen, theBufP);
    }

    AutoCrit aCrit(mPipeCrit);

    // Dispatch message
    switch (aMsg.GetServiceType())
    {
        case WONMsg::CommonService:
            if (aMsg.GetMessageType() != WONMsg::MiniCommPing && aMsg.GetMessageType() != WONMsg::MiniCommPingReply && thePipeP != mDirPipe && thePipeP !=mFactPingPipe)
            {
                titanDebug("Titaninterface::HandleTitanMsg unexpected CommonService message discarded.");
            }
            else
            {
                switch(aMsg.GetMessageType())
                {
                    case WONMsg::CommStatusReply:
                       //Dir Server Session has expired
                       delete mDirSessionKey;
                       mDirSessionKey = NULL;
                       mDirPipe = NULL;
                       RequestDirectory();
                       break;
                    default:
                        titanDebug("TitanInterface::HandleTitanMsg Got Unknown titan message, bad CommonService message type %d!", aMsg.GetMessageType());
                        break;
                }
            }
            break;
        case WONMsg::Auth1Login:
            if (thePipeP != mAuthPipe)
            {
                titanDebug("TitanInterface::HandleTitanMsg AuthServer msg on non-auth pipe discared!");
            }
            else
            {
                switch (aMsg.GetMessageType())
                {
                    case WONMsg::Auth1GetPubKeysReply:
                        AuthHandleGetPubKeysReply(aMsg); break;
                    case WONMsg::Auth1LoginReply:
                        AuthHandleLoginReply(aMsg);      break;
                    case WONMsg::Auth1LoginChallengeHW:
                        AuthHandleChallenge(aMsg);       break;
                    default:
                        titanDebug("TitanInterface::HandleTitanMsg Got Unknown titan message, bad Auth message type %d!", aMsg.GetMessageType());
                        break;
                }
            }
            break;
        case WONMsg::Auth1PeerToPeer:
            if(thePipeP!=mDirPipe && thePipeP!=mFactPipe && thePipeP!=mRoutePipe[0] && thePipeP!=mRoutePipe[1])
            {
                titanDebug("TitanInterface::HandleTitanMsg Auth1PeerToPeer msg on non dir/fact/route pipe discarded.");
            }
            else
            {
                switch(aMsg.GetMessageType())
                {
                    case WONMsg::Auth1Challenge1:
                        PeerHandleChallenge(&thePipeP, aMsg); break;
                    case WONMsg::Auth1Complete:
                        PeerHandleComplete(&thePipeP, aMsg);  break;
                    default:
                        titanDebug("TitanInterface::HandleTitanMsg got unkonw Auth1PeerToPeer message.");
                        break;
                }
            }
            break;
        default:
            titanDebug("TitanInterface::HandleTitanMsg Got Unknown titan message, bad service type!");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleSmallMsg(SocketPipe* thePipeP, const char* theBufP, unsigned long theLen)
{
    SmallMessage aMsg;

    if (theLen < aMsg.GetHeaderLength())
    {
        titanDebug("TitanInterface::HandleSmallMsg Corrupt MiniMessage discarded, bad length!");
        return;
    }

    if(theBufP[0]==WONMsg::SmallEncryptedService) {
        if(!DecryptMessage(theBufP,theLen,aMsg,&thePipeP))
            return; // Failed to decrypt
    }
    else {
        // Fill in SMsg header
        aMsg.ResetBuffer();
        aMsg.AllocateHeaderBuffer();
        theBufP += sizeof(unsigned char); // skip header type
        aMsg.SetServiceType(*(reinterpret_cast<const unsigned short*>(theBufP)));
        theBufP += sizeof(unsigned short);
        aMsg.SetMessageType(*(reinterpret_cast<const unsigned short*>(theBufP)));
        theBufP += sizeof(unsigned short);
        aMsg.Pack();

        unsigned long aDataLen = theLen - aMsg.GetHeaderLength();
        if (aDataLen > 0)
            aMsg.AppendBytes(aDataLen, theBufP);
    }

    switch (aMsg.GetServiceType())
    {
        case WONMsg::SmallCommonService:
            switch (aMsg.GetMessageType())
            {
                case WONMsg::SmallCommStatusReply:
                    HandleRoutingRegisterReply(thePipeP, aMsg); break;
                default:
                    titanDebug("TitanInterface::HandleSmallMsg Got Unknown small message, bad Common message type %d!", aMsg.GetMessageType());
                    break;
            }
            break;
        case WONMsg::SmallDirServerG2:
            if (thePipeP != mDirPipe)
            {
                titanDebug("TitanInterface::HandleSmallMsg DirServer msg on non-dir pipe discarded!");
            }
            else
            {
                switch (aMsg.GetMessageType())
                {
                case WONMsg::DirG2MultiEntityReply:
                    HandleDirMultiEntityReply(aMsg); break;
                default:
                    titanDebug("TitanInterface::HandleSmallMsg Got Unknown small message, bad DirServer message type %d!", aMsg.GetMessageType());
                    break;
                }
            }
            break;
        case WONMsg::SmallFactoryServer:
            if (thePipeP != mFactPipe)
            {
                titanDebug("TitanInterface::HandleSmallMsg FactoryServer msg on non-fact pipe discarded!");
            }
            else
            {
                switch (aMsg.GetMessageType())
                {
                    case WONMsg::SmallFactStatusReply:
                        HandleStartRoutingReply(thePipeP, aMsg); break;
                    default:
                        titanDebug("TitanInterface::HandleSmallMsg Got Unknown small message, bad FactoryServer message type %d!", aMsg.GetMessageType());
                        break;
                }
            }
            break;
        case WONMsg::SmallFirewallDetector:
            switch (aMsg.GetMessageType())
            {
                case WONMsg::FirewallStatusReply:
                    HandleFirewallResponse(thePipeP, aMsg); break;
                default:
                    titanDebug("TitanInterface::HandleSmallMsg Got Unknown small message, bad Firewall message type %d!", aMsg.GetMessageType());
                    break;
            }
            break;
        default:
            titanDebug("TitanInterface::HandleSmallMsg Got Unknown small message!  Service type %d, Message type %d", aMsg.GetServiceType(), aMsg.GetMessageType());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleMiniMsg(SocketPipe* thePipeP, const char* theBufP, unsigned long theLen)
{
    MiniMessage aMsg;
    int aRouteNum;

    if (theLen < aMsg.GetHeaderLength())
    {
        titanDebug("TitanInterface::HandleMiniMsg Corrupt MiniMessage discarded, bad length!");
        return;
    }

    if(theBufP[0]==WONMsg::MiniEncryptedService) {
        if(!DecryptMessage(theBufP,theLen,aMsg,&thePipeP))
            return; // Failed to decrypt
    }
    else {
        // Fill in MMsg header
        const char* p = theBufP;
        aMsg.ResetBuffer();
        aMsg.AllocateHeaderBuffer();
        p += sizeof(unsigned char); // skip header type
        aMsg.SetServiceType(*(reinterpret_cast<const unsigned char*>(p)));
        p += sizeof(unsigned char);
        aMsg.SetMessageType(*(reinterpret_cast<const unsigned char*>(p)));
        p += sizeof(unsigned char);
        aMsg.Pack();

        // Discard non-encrypted game msgs other than pings
        if ((aMsg.GetServiceType() == TitanPacketMsg::MyServiceType) &&
            (aMsg.GetMessageType() != TITANMSGTYPE_PING) &&
            (aMsg.GetMessageType() != TITANMSGTYPE_PINGREPLY))
        {
            titanDebug("TitanInterface::HandleMiniMsg Unencrypted game msg dropped!");
            // Could add a callback here if game wants to know
            return;
        }

        unsigned long aDataLen = theLen - aMsg.GetHeaderLength();
        if (aDataLen > 0)
            aMsg.AppendBytes(aDataLen, p);
    }

    switch (aMsg.GetServiceType())
    {
        case WONMsg::MiniRoutingServer:
            if(thePipeP==mRoutePipe[0]) aRouteNum = 0;
            else if(thePipeP==mRoutePipe[1]) aRouteNum = 1;
            else
            {
                titanDebug("TitanInterface::HandleMiniMsg RoutingServer msg on non-route pipe discarded!");
                return;
            }

            switch (aMsg.GetMessageType())
            {
                case WONMsg::RoutingCreateDataObject:
                    HandleRoutingCreateDataObject(thePipeP, aMsg); break;
                case WONMsg::RoutingDeleteDataObject:
                    HandleRoutingDeleteDataObject(thePipeP, aMsg); break;
                case WONMsg::RoutingGetClientListReply:
                    HandleRoutingGetClientListReply(thePipeP, aMsg, aRouteNum); break;
                case WONMsg::RoutingClientChange:
                case WONMsg::RoutingClientChangeEx:
                    break; // we expect these messages, but don't process them at all
                case WONMsg::RoutingGroupChange:
                    HandleRoutingGroupChange(thePipeP, aMsg, aRouteNum); break;
                case WONMsg::RoutingGroupChangeEx:
                    HandleRoutingGroupChangeEx(thePipeP, aMsg, aRouteNum); break;
                case WONMsg::RoutingPeerChat:
                    HandleRoutingPeerChat(thePipeP, aMsg, aRouteNum); break;
                case WONMsg::RoutingPeerData:
                    HandleRoutingPeerData(thePipeP, aMsg, aRouteNum); break;
                case WONMsg::RoutingPeerDataMultiple:
                    HandleRoutingPeerDataMultiple(thePipeP, aMsg, aRouteNum); break;
                case WONMsg::RoutingReadDataObjectReply:
                    HandleRoutingReadDataObjectReply(thePipeP, aMsg); break;
                case WONMsg::RoutingReplaceDataObject:
                    HandleRoutingReplaceDataObject(thePipeP, aMsg); break;
                case WONMsg::RoutingRegisterClientReply:
                    HandleRoutingRegisterClientReply(thePipeP, aMsg, aRouteNum); break;
                case WONMsg::RoutingStatusReply:
                    HandleRoutingStatusReply(thePipeP, aMsg, aRouteNum); break;
                default:
                    titanDebug("TitanInterface::HandleMiniMsg Got Unknown mini message, bad RoutingServer message type %d!", aMsg.GetMessageType());
                    break;
            }
            break;
        case TitanPacketMsg::MyServiceType:
        case TitanPacketMsg::MyEncryptServiceType:
             HandlePeerMsg(thePipeP, aMsg);
             break;
        case WONMsg::MiniCommonService:
            switch(aMsg.GetMessageType())
            {
                case WONMsg::MiniCommPing:
                    HandlePing(thePipeP, aMsg); break;
                case WONMsg::MiniCommPingReply:
                    if(thePipeP==mFactPingPipe)
                        PingHandleReply(aMsg);
                    else
                        HandlePingReply(thePipeP, aMsg);
                    break;
//              case WONMsg::MiniCommGetNumUsersReply:
//                  HandleGetNumUsersReply(thePipeP, aMsg); break;
                default:
                    titanDebug("TitanInterface::HandleMiniMsg Got Unknown mini message, bad Common message type %d!", aMsg.GetMessageType());
                    break;
            }
            break;
        default:
            titanDebug("TitanInterface::HandleMiniMsg Got Unknown mini message! (service %d, message %d)", aMsg.GetServiceType(), aMsg.GetMessageType());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandlePeerMsg(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR)
{
    // All packets other than game data packets are prefaced with the game name
    // so unwanted messages can be ignored.  Game data packets are the exception
    // because we don't want to waste bandwidth during the game...
    TitanPacketMsg aTitanPacketMsg(theMsgR);
    if (theMsgR.GetMessageType() == TITANMSGTYPE_GAME)
        HandleGameMsg(aTitanPacketMsg);
    else
    {
        LockMutex(GameWereInterestedInMutex);
        bool ignore = (aTitanPacketMsg.GetGameName() != GameWereInterestedIn);
        UnLockMutex(GameWereInterestedInMutex);
        if (!ignore)
        {
            titanDebug("TitanInterface::HandlePeerMsg Got message type %d",theMsgR.GetMessageType());
            switch (theMsgR.GetMessageType())
            {
                case TITANMSGTYPE_JOINGAMEREQUEST:
                    HandleJoinGame(thePipeP, aTitanPacketMsg);  break;
                case TITANMSGTYPE_JOINGAMECONFIRM:
                    HandleJoinConfirm(thePipeP, aTitanPacketMsg);  break;
                case TITANMSGTYPE_JOINGAMEREJECT:
                    HandleJoinReject(thePipeP, aTitanPacketMsg);  break;
                case TITANMSGTYPE_LEAVEGAMEREQUEST:
                    HandleLeaveGame(thePipeP, aTitanPacketMsg);  break;
                case TITANMSGTYPE_UPDATEGAMEDATA:
                    HandleGameData(aTitanPacketMsg);  break;
                case TITANMSGTYPE_GAMEISSTARTING:
                    HandleGameStart(aTitanPacketMsg);  break;
                case TITANMSGTYPE_GAMEDISOLVED:
                    HandleGameDisolved(aTitanPacketMsg); break;
                case TITANMSGTYPE_UPDATEPLAYER:
                    HandleUpdatePlayer(thePipeP, aTitanPacketMsg); break;
                case TITANMSGTYPE_BEGINSTARTGAME:
                    HandleBeginStartGame(thePipeP, aTitanPacketMsg); break;
                case TITANMSGTYPE_CHANGEADDRESS:
                    HandleChangeAddress(thePipeP, aTitanPacketMsg); break;
                case TITANMSGTYPE_REQUESTPACKETS:
                    HandleRequestPackets(thePipeP, aTitanPacketMsg); break;
                case TITANMSGTYPE_RECONNECT:
                    HandleClientReconnect(thePipeP, aTitanPacketMsg); break;
                case TITANMSGTYPE_KICKPLAYER:
                    HandleKickedOutOfGame(aTitanPacketMsg); break;
                default:
                    titanDebug("TitanInterface::HandlePeerMsg Got Unknown titan message, bad Homeworld message type!");
                    break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


void
TitanInterface::HandleStartRoutingReply(SocketPipe* thePipeP, const SmallMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleStartRoutingReply Got FactoryStatusReply message");

    // Toss dir pipe
    mPipeCrit.Enter();
    mFactPipe = NULL;
    mPipeCrit.Leave();

    try
    {
        WONMsg::SMsgFactStatusReply aReply(theMsgR);

        titanDebug("TitanInterface::HandleStartRoutingReply StartProcess returned status %d", aReply.GetProcessStatus());
        if(aReply.GetProcessStatus()<0) {
            FactFailOver();
            return;
        }

        int aRouteNum = 0;

        if(mGameCreationState==GAME_STARTING) aRouteNum = 1;

        thePipeP->mEasySocketP->getDestAddr(reinterpret_cast<SOCKADDR*>(&(mRoutingAddress[aRouteNum])));

        WONMsg::FACT_SERV_PORT_RANGE_SET::iterator anItr = aReply.GetProcessPortIDSet().begin();
        mRoutingAddress[aRouteNum].sin_port = htons((anItr != aReply.GetProcessPortIDSet().end() ? *anItr : 0));

        if(mGameCreationState==GAME_NOT_STARTED)
        {
            mNeedToRegisterRoutingServer = true;
            ConnectToRoutingServer(StringToWString(utyName),mRoomPassword.c_str(),0);
        }
        else {
            mgDisplayMessage(strGetString(strConnectingToRoutingServer));
            ConnectToRoutingServer(StringToWString(utyName),L"",1);
        }

        ResetFactFailOver();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::FactoryStatusReply: %s", (char*)anEx.what());
        FactFailOver();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingRegisterReply(SocketPipe* thePipeP, const WONMsg::SmallMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleRoutingRegisterReply");

    short aStatus;

    try
    {
        WONMsg::SMsgCommStatusReply aReply(theMsgR);
        aStatus = aReply.GetStatus();
        if (aStatus < 0)
            titanDebug("TitanInterface::HandleRoutingRegisterReply Status = %d", aReply.GetStatus());
    }
    catch(WONException&)
    {
        titanDebug("TitanInterface::HandleRoutingRegisterReply EXCEPTION: Invalid SMsgCommStatusReply received.");
        return;
    }

    // tell Homeworld whether chat startup succeeded
    titanStartChatReplyReceivedCB(aStatus);

    // shutdown server if necessary
    if (aStatus != WONMsg::StatusCommon_Success)
        titanDebug("TitanInterface::HandleRoutingRegisterReply Failed to register chat server with error %d.  Server will shutdown on its own.", aStatus);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void
TitanInterface::HandleGetNumUsersReply(WONMisc::SocketPipe *thePipeP, const WONMsg::MiniMessage& theMsgR)
{
    if(thePipeP!=mRoutingInfoPipe)
        return;

    tpLockChannelList();

    try
    {
        WONMsg::MMsgCommGetNumUsersReply aMsg(theMsgR);

        unsigned short anOffset = aMsg.GetTag() - mRoutingQueryOffset;
        if(anOffset<mRoutingQueryMap.size()) {
            titanGotNumUsersInRoomCB(mRoutingQueryMap[anOffset].c_str(),aMsg.GetNumActiveUsers());
        }
    }
    catch(WONException&)
    {
        titanDebug("Invalid get num users reply received.");
    }

    tpUnLockChannelList();
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void
TitanInterface::QueryRoutingServers(void)
{
    AutoCrit aPipeCrit(mPipeCrit);
    if(mRoutingInfoPipe==NULL) {
        mRoutingInfoPipe=new SocketPipe;
        mRoutingInfoPipe->AddOutgoingCmd(new OpenCmd(EasySocket::UDP));
        for (int i=0; i < PREPOST_MAX; i++)
            mRoutingInfoPipe->AddIncomingCmd(new RecvFromCmd(new char[32],32,true,false));

        mEngine->AddPipe(mRoutingInfoPipe);
    }
    aPipeCrit.Leave();

    int n;

    tpLockChannelList();

    int numChannels = tpChannelList.numChannels;

    mRoutingQueryMap.resize(numChannels);
    for(n=0; n<numChannels; n++)
        mRoutingQueryMap[n] = tpChannelList.tpChannels[n].ChannelName;

    tpUnLockChannelList();

    mRoutingQueryOffset+=1000;
    if(mRoutingQueryOffset>60000) mRoutingQueryOffset = 0;

    WONMsg::MMsgCommGetNumUsers aMsg;

    aPipeCrit.Enter(); // QueryRoutingServers is called from Homeworld's thread, so it's possible (improbable) that the pipe is in an error state now.
    if (mRoutingInfoPipe == NULL)
        return;

    try {
        for(n=0; n<numChannels; n++) {
            aMsg.SetTag(mRoutingQueryOffset+n);
            aMsg.Pack();

            char *aBuf = new char[aMsg.GetDataLen()];
            memcpy(aBuf,aMsg.GetDataPtr(),aMsg.GetDataLen());

            if(n<mRoutingAddrMap.size())
                mRoutingInfoPipe->AddOutgoingCmd(new SendToCmd(*(SOCKADDR*)&(mRoutingAddrMap[n]), aBuf, aMsg.GetDataLen(), true, true));
        }
    }
    catch(WONException&) {
        titanDebug("Threw fucking exception while packing nothing message in TitanInterface::PingChatServers!");
    }
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleChannelList(WONMsg::DirEntityList& dirEntities)
{
    tpLockChannelList();

    bool readFactServers = (FACTSERVER_NUM==0);
    int foundCurrentChannel = false;
    bool isHWDS = false;

//    mRoutingAddrMap.clear();

    WONMsg::DirEntityList::iterator aEntityItr = dirEntities.begin();
    for (int i=0; aEntityItr != dirEntities.end(); ++aEntityItr)
    {
        if (aEntityItr->mType == WONMsg::DirEntity::ET_SERVICE)
        {
            if (!isHWDS)
            {
                if (readFactServers && aEntityItr->mName == FACTORY_SERV)
                {
                    if (FACTSERVER_NUM < MAX_IPS)
                        BuildAddress(FACTSERVER_ADDRESSES[FACTSERVER_NUM++], aEntityItr->mNetAddress);
                    else
                        titanDebug("Too many factory servers.");
                }
                else if (i < TP_ChannelListLength && aEntityItr->mName == CHATROOM_SERV)
                {
                    // save room name
                    wcsncpy(tpChannelList.tpChannels[i].ChannelName, aEntityItr->mDisplayName.c_str(), MAX_CHANNEL_NAME_LEN-1);
                    tpChannelList.tpChannels[i].ChannelName[MAX_CHANNEL_NAME_LEN-1] = L'\0';

                    // unpack data objects (room flags & description)
                    tpChannelList.tpChannels[i].roomflags = 0;
                    tpChannelList.tpChannels[i].roomfullness = 0;
                    WONCommon::DataObjectTypeSet::iterator aObjItr = aEntityItr->mDataObjects.begin();
                    for (; aObjItr != aEntityItr->mDataObjects.end(); ++aObjItr)
                    {
                        if (aObjItr->GetDataType() == ROOM_FLAGS_OBJ && aObjItr->GetData().size() == sizeof(unsigned int))
                            tpChannelList.tpChannels[i].roomflags = *(unsigned int*)aObjItr->GetData().c_str();
                        else if (aObjItr->GetDataType() == DESCRIPTION_OBJ)
                        {
                            wcsncpy(tpChannelList.tpChannels[i].ChannelDescription, reinterpret_cast<const wchar_t*>(aObjItr->GetData().data()), MAX_CHANNEL_DESCRIPTION_LEN-1);
                            unsigned int aStringEnd = aObjItr->GetData().size() / 2;
                            if (aStringEnd > MAX_CHANNEL_DESCRIPTION_LEN-1)
                                aStringEnd = MAX_CHANNEL_DESCRIPTION_LEN-1;
                            tpChannelList.tpChannels[i].ChannelDescription[aStringEnd] = L'\0';
                        }
                        else if (aObjItr->GetDataType() == ROOM_CLIENTCOUNT_OBJ)
                            tpChannelList.tpChannels[i].roomfullness = *(unsigned long*)aObjItr->GetData().data();
                    }

                    memcpy(tpChannelList.tpChannels[i].addressstore,aEntityItr->mNetAddress.data(),6);

                    i++;
                }
            }
            else if (aEntityItr->mName == FACTORY_SERV)
            {
                // save server name
                wcsncpy(tpServerList.tpServers[i].ServerName, aEntityItr->mDisplayName.c_str(), MAX_SERVER_NAME_LEN-1);
                tpServerList.tpServers[i].ServerName[MAX_SERVER_NAME_LEN-1] = L'\0';

                // unpack data objects (flags & description)
                tpServerList.tpServers[i].flags = 0;
                tpServerList.tpServers[i].reliability = 0;
                tpServerList.tpServers[i].ping = 0;
                WONCommon::DataObjectTypeSet::iterator aObjItr = aEntityItr->mDataObjects.begin();
                for (; aObjItr != aEntityItr->mDataObjects.end(); ++aObjItr)
                {
                    if (aObjItr->GetDataType() == DESCRIPTION_OBJ)
                    {
                        wcsncpy(tpServerList.tpServers[i].ServerDescription, reinterpret_cast<const wchar_t*>(aObjItr->GetData().data()), MAX_SERVER_DESCRIPTION_LEN-1);
                        unsigned int aStringEnd = aObjItr->GetData().size() / 2;
                        if (aStringEnd > MAX_SERVER_DESCRIPTION_LEN-1)
                            aStringEnd = MAX_SERVER_DESCRIPTION_LEN-1;
                        tpServerList.tpServers[i].ServerDescription[aStringEnd] = L'\0';
                    }
                    else if (aObjItr->GetDataType() == SERVER_UPTIME_OBJ)
                        tpServerList.tpServers[i].reliability = *(unsigned long*)aObjItr->GetData().data();
                    else if (aObjItr->GetDataType() == FACT_CUR_SERVER_COUNT_OBJ)
                        tpServerList.tpServers[i].flags += *(unsigned long*)aObjItr->GetData().data();
                    //else if (aObjItr->GetDataType() == FACT_TOTAL_SERVER_COUNT_OBJ)
                        //tpServerList.tpServers[i].flags += *(unsigned long*)aObjItr->GetData().data();
                }

                memcpy(tpServerList.tpServers[i].addressstore,aEntityItr->mNetAddress.data(),6);

                i++;
            }

            titanDebug("  Ser: %s - %s", WStringToString(aEntityItr->mName).c_str(), PrintAddress(aEntityItr->mNetAddress));
        }
        else if (aEntityItr->mType == WONMsg::DirEntity::ET_DIRECTORY)
        {
            if (aEntityItr->mName == HWDS_DIR)
            {
                // switch from expecting WON Factory Servers and Chat Rooms to expecting HWDS Factory Servers.
                // (aka switch from ChannelList to ServerList)
                tpChannelList.numChannels = i;
                tpChannelList.newDataArrived = TRUE;
                tpUnLockChannelList();
                i = 0;
                isHWDS = true;
                tpLockServerList();
            }
            titanDebug("  Dir: %s", WStringToString(aEntityItr->mName).c_str());
        }
    }

    if (isHWDS)
    {
        tpServerList.numServers = i;
        tpServerList.newDataArrived = TRUE;
        tpUnLockServerList();
    }
    else
    {
        tpChannelList.numChannels = i;
        tpChannelList.newDataArrived = TRUE;
        tpUnLockChannelList();
    }

    cNotifyRoomListPresent();

    tpLockChannelList();
    for (i=0;i<tpChannelList.numChannels;i++)
    {
        if (wcscmp(tpChannelList.tpChannels[i].ChannelName, GetCurrentChannel()) == 0)
        {
            foundCurrentChannel = true;

            SOCKADDR_IN aSockAddr;
            BuildAddress(aSockAddr, tpChannelList.tpChannels[i].addressstore);

            mRoutingAddress[0] = aSockAddr;
            break;
        }
    }
    tpUnLockChannelList();

    cNotifyCurrentRoomPresent(foundCurrentChannel);

    if(mCurFactPing==-1)
        PingFactServer();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleTitanServerList(WONMsg::DirEntityList& dirEntities)
{
    WONMsg::DirEntityList::iterator aEntityItr = dirEntities.begin();
    for ( ; aEntityItr != dirEntities.end(); ++aEntityItr)
    {
        if (aEntityItr->mType == WONMsg::DirEntity::ET_SERVICE)
        {
            if (aEntityItr->mName == AUTH_SERV)
            {
                if (AUTHSERVER_NUM < MAX_IPS)
                    BuildAddress(AUTHSERVER_ADDRESSES[AUTHSERVER_NUM++], aEntityItr->mNetAddress);
                else
                    titanDebug("Too many auth servers.");
            }
            else if (aEntityItr->mName == EVENT_SERV)
            {
                if (EVENTSERVER_NUM < MAX_IPS)
                    BuildAddress(EVENTSERVER_ADDRESSES[EVENTSERVER_NUM++], aEntityItr->mNetAddress);
                else
                    titanDebug("Too many event servers.");
            }
            else if (aEntityItr->mName == FIREWALL_SERV)
            {
                if (FIREWALLSERVER_NUM < MAX_IPS)
                    BuildAddress(FIREWALLSERVER_ADDRESSES[FIREWALLSERVER_NUM++], aEntityItr->mNetAddress);
                else
                    titanDebug("Too many firewall servers.");
            }

            titanDebug("  Ser: %s - %s", WStringToString(aEntityItr->mName).c_str(), PrintAddress(aEntityItr->mNetAddress));
        }
        else
            titanDebug("  Dir: %s", WStringToString(aEntityItr->mName).c_str());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleDirMultiEntityReply(const SmallMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleDirMultiEntityReply");

    // Toss dir pipe
    mPipeCrit.Enter();
    mDirPipe = NULL;
    mPipeCrit.Leave();

    try
    {
        WONMsg::SMsgDirG2MultiEntityReply aReply(theMsgR);
        WONMsg::DirEntityList& dirEntities = aReply.Entities();

        if (aReply.GetStatus() >= 0)
        {
            titanDebug("TitanInterface::HandleDirMultiEntityReply DirG2MultiEntityReply message - Request OK.");

            // If we haven't gotten the Auth Server list yet, this reply should be
            // in response to our request for the ValidVersions object.  Otherwise,
            // it should be in response to a general query of the Homeworld directory.
            if (AUTHSERVER_NUM == 0)
            {
                if (dirEntities.size() > 0)
                {
                    titanDebug("TitanInterface::HandleDirMultiEntityReply Got auth server list and valid version info.");

                    // find directory entity and grab valid versions object from it
                    WONMsg::DirEntityList::iterator aEntityItr = dirEntities.begin();
                    for ( ; aEntityItr != dirEntities.end(); ++aEntityItr)
                    {
                        if (aEntityItr->mType == WONMsg::DirEntity::ET_DIRECTORY)
                        {
                            WONCommon::DataObjectTypeSet::iterator aObjItr = aEntityItr->mDataObjects.find(WONCommon::DataObject(VALIDVERSIONS_OBJ));
                            if (aObjItr != aEntityItr->mDataObjects.end())
                            {
                                titanGotValidVersionStrings(const_cast<char*>(reinterpret_cast<const char*>(aObjItr->GetData().c_str())));
                                break;
                            }
                        }
                    }
                    // handle the auth server services
                    HandleTitanServerList(dirEntities);
                    if(mNeedToAuthenticateAfterGettingAuthDirectory && AUTHSERVER_NUM>0) {
                        mNeedToAuthenticateAfterGettingAuthDirectory = false;
                        Authenticate(mLoginName,mPassword,mNewPassword,mCreateAccount);
                    }
                }
            }
            else
            {
                titanDebug("TitanInterface::HandleDirMultiEntityReply Dumping Channel Reply: %d Entities",dirEntities.size());
                HandleChannelList(dirEntities);
            }
        }
        else
        {
            titanDebug("TitanInterface::HandleDirMultiEntityReply message - Request FAILED!  Status = %d", aReply.GetStatus());
        }

        ResetDirFailOver();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleDirMultiEntityReply: %s", (char*)anEx.what());
        DirFailOver();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleJoinGame(SocketPipe* thePipeP, const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleJoinGame");
    try
    {
        Address anAddress;
        InitDestAddress(&anAddress, thePipeP);

        long requestResult;

        if(mGameCreationState==GAME_NOT_STARTED)
            requestResult = titanRequestReceivedCB(&anAddress,theMsgR.GetBlob(), theMsgR.GetBlobLen());
        else
            requestResult = REQUEST_RECV_CB_JUSTDENY;

        if (requestResult == REQUEST_RECV_CB_ACCEPT)
        {
            if(!mUseRoutingServer) {
                ClientNetAddr anAddr = MakeClientNetAddr(&anAddress);
                AutoCrit aCrit(mClientCrit);
                mClientMap[anAddr].pipe = thePipeP;
                aCrit.Leave();
            }

            titanDebug("TitanInterface::HandleJoinGame Sent join confirm");

            titanSendPacketTo(&anAddress, TITANMSGTYPE_JOINGAMECONFIRM, NULL, 0);
            titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));
        }
        else
        {
            // send reject message
            titanDebug("TitanInterface::HandleJoinGame Sent join reject");

            if(!mUseRoutingServer) {
                ClientNetAddr anAddr = MakeClientNetAddr(&anAddress);
                AutoCrit aCrit(mClientCrit);
                mClientMap[anAddr].pipe = thePipeP;
                aCrit.Leave();
            }
            titanSendPacketTo(&anAddress, TITANMSGTYPE_JOINGAMEREJECT, NULL, 0);
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleJoinGame: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleClientReconnect(SocketPipe* thePipeP, const MiniMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleClientReconnect");
    try
    {
        Address anAddress;
        InitDestAddress(&anAddress, thePipeP);

        ClientNetAddr anAddr = MakeClientNetAddr(&anAddress);
        AutoCrit aCrit(mClientCrit);
        mClientMap[anAddr].pipe = thePipeP;
        aCrit.Leave();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleClientReconnect: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::InitDestAddress(Address* theAddressP, SocketPipe* thePipeP)
{

    if(mUseRoutingServer) {
        theAddressP->AddrPart.IP = thePipeP->GetLabel();
        theAddressP->Port = thePipeP->GetLabel();
        return;
    }
    else
        theAddressP->Port = GAME_PORT;

    if ( mIpType == ip )
    {
        SOCKADDR_IN aClientAddr;
        thePipeP->mEasySocketP->getDestAddr(reinterpret_cast<SOCKADDR*>(&aClientAddr));
        theAddressP->AddrPart.IP = aClientAddr.sin_addr.s_addr;
    }
    else
    {
        SOCKADDR_IPX aClientAddr;
        thePipeP->mEasySocketP->getDestAddr(reinterpret_cast<SOCKADDR*>(&aClientAddr));
        memcpy( theAddressP->AddrPart.etherAddr, aClientAddr.sa_nodenum, 6 );
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleJoinConfirm(SocketPipe* thePipeP, const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleJoinConfirm");
    try
    {
        Address anAddress;

        InitDestAddress(&anAddress, thePipeP);

        if(!mUseRoutingServer) {
            ClientNetAddr anAddr = MakeClientNetAddr(&anAddress);
            AutoCrit aCrit(mClientCrit);
            mClientMap[anAddr].pipe = thePipeP;
            aCrit.Leave();
        }

        titanConfirmReceivedCB(&anAddress, theMsgR.GetBlob(), theMsgR.GetBlobLen());
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleJoinConfirm: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleJoinReject(SocketPipe* thePipeP, const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleJoinReject");
    try
    {
        Address anAddress;
        InitDestAddress(&anAddress, thePipeP);

        if(!mUseRoutingServer) {
            ClientNetAddr anAddr = MakeClientNetAddr(&anAddress);
            AutoCrit aCrit(mClientCrit);
            mClientMap[anAddr].pipe = thePipeP;
            aCrit.Leave();
        }

        titanRejectReceivedCB(&anAddress, theMsgR.GetBlob(), theMsgR.GetBlobLen());
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleJoinReject: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleLeaveGame(SocketPipe* thePipeP, const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleLeaveGame");
    try
    {
        Address anAddress;
        InitDestAddress(&anAddress, thePipeP);

        ClientNetAddr anAddr = MakeClientNetAddr(&anAddress);
        AutoCrit aCrit(mClientCrit);

        if (IPGame) // don't close old client connections if IPX Lan
        {
        ClientToPipe::iterator anItr = mClientMap.find(anAddr);
        if(anItr!=mClientMap.end())
        {
            if(anItr->second.pipe!=NULL)
                anItr->second.pipe->AddOutgoingCmd(new CloseCmd(false, false, 0, false));

            mClientMap.erase(anItr);
            }
        }
        aCrit.Leave();

        titanLeaveGameReceivedCB(&anAddress, theMsgR.GetBlob(), theMsgR.GetBlobLen());
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleLeaveGame: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandlePing(SocketPipe* thePipeP, const MiniMessage& theMsgR)
{
    titanDebug("TitanInterface::HandlePing");
    try
    {
        WONMsg::MMsgCommPing aMsg(theMsgR);
        WONMsg::MMsgCommPingReply aReply;
        aReply.SetStartTick(aMsg.GetStartTick());

        aReply.Pack();

        if (! SendMsg(thePipeP, aReply, 2))
        {
            titanDebug("FAIL: TitanInterface::HandlePing to send ping reply.");
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandlePing: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandlePingReply(SocketPipe* thePipeP, const MiniMessage& theMsgR)
{
    titanDebug("TitanInterface::HandlePingReply");
    try
    {
        WONMsg::MMsgCommPingReply aMsg(theMsgR);

        SOCKADDR_IN aClientAddr;

        thePipeP->mEasySocketP->getDestAddr(reinterpret_cast<SOCKADDR*>(&aClientAddr));
        Address anAddress;
        anAddress.AddrPart.IP = aClientAddr.sin_addr.s_addr;
        anAddress.Port = ntohs(aClientAddr.sin_port);

        unsigned long aLag = aMsg.GetLag() / 5;
        titanPingReplyReceivedCB(&anAddress, aLag < 50 ? 50 : aLag);
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandlePingReply: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleGameStart(const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleGameStart");
    try
    {
        //CloseRoutingServerConnection(0);
        mgGameStartReceivedCB(theMsgR.GetBlob(), theMsgR.GetBlobLen());

        // Record an event indicating that the player has joined the game
        RecordEvent(ACTIVITYTYPE_HOMEWORLD_GAME_ENTER);
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleGameStart: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleGameData(const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleGameData");
    try
    {
        titanUpdateGameDataCB(theMsgR.GetBlob(), theMsgR.GetBlobLen());
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleGameData: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleGameMsg(const TitanPacketMsg& theMsgR)
{
    try
    {
        titanGameMsgReceivedCB(theMsgR.GetBlob(), theMsgR.GetBlobLen());
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleGameMsg: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleGameDisolved(const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleGameDisolved");
    try
    {
        mgNotifyGameDisolved();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleGameDisolved: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleKickedOutOfGame(const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleKickedOutOfGame");
    try
    {
        mgNotifyKickedOut();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleKickedOutOfGame: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleUpdatePlayer(SocketPipe* thePipeP, const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleUpdatePlayer");
    try
    {
        Address anAddress;
        InitDestAddress(&anAddress, thePipeP);

        titanUpdatePlayerCB(&anAddress, theMsgR.GetBlob(), theMsgR.GetBlobLen());
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleUpdatePlayer: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleChangeAddress(SocketPipe* thePipeP, const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleChangeAddress");
    try
    {
        Address anAddress;

        if(theMsgR.GetBlobLen()!=sizeof(ChangeAddressPacket))
            return;

        ChangeAddressPacket *aPacket = (ChangeAddressPacket*)theMsgR.GetBlob();
        ChangeAddress(&(aPacket->oldAddress),&(aPacket->newAddress));

        if(!mUseRoutingServer) {
            ClientNetAddr anAddr = MakeClientNetAddr(&(aPacket->newAddress));
            AutoCrit aCrit(mClientCrit);
            mClientMap[anAddr].pipe = thePipeP;
            aCrit.Leave();
        }

        if(mGameCreationState == GAME_STARTING && IAmCaptain) {
            mgDisplayMessage(strGetString(strPlayerJoined));

            mNumPlayersJoined++;
            if(mNumPlayersJoined==tpGameCreated.numPlayers) {
                mGameCreationState = GAME_STARTED;
                InitPacketList();

                titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));
                mgStartGameCB();

                //CloseRoutingServerConnection(0);

                return;
            }
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleUpdatePlayer: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleRequestPackets(SocketPipe* thePipeP, const TitanPacketMsg& theMsgR)
{
    titanDebug("TitanInterface::HandleRequestPackets");

    try
    {
        Address anAddress;

        if(theMsgR.GetBlobLen()!=sizeof(RequestPacketsPacket))
            return;

        RequestPacketsPacket *aPacket = (RequestPacketsPacket*)theMsgR.GetBlob();
        InitDestAddress(&anAddress, thePipeP);

        AutoCrit aCrit(mPacketCrit);

        if(mPacketList.size()==0)
            return;

        unsigned long aBeginPacket = mSeqNum - mPacketList.size();

        if(mSeqNum<=aPacket->lastPacket || aBeginPacket>aPacket->firstPacket)
            return;

        std::list<basic_string<unsigned char> >::iterator anItr = mPacketList.begin();
        std::list<unsigned char>::iterator aTypeItr = mPacketTypeList.begin();

        while(aBeginPacket!=aPacket->firstPacket) {
            anItr++;
            aTypeItr++;
            aBeginPacket++;
        }

        while(aBeginPacket<=aPacket->lastPacket) {
            SendPacketTo(&anAddress,*aTypeItr,anItr->data(),anItr->size(),true,aBeginPacket);
            anItr++;
            aTypeItr++;
            aBeginPacket++;
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleUpdatePlayer: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::HandleBeginStartGame(SocketPipe* thePipeP, const TitanPacketMsg& theMsgR) {
    titanDebug("TitanInterface::HandleBeginStartGame");
    try
    {
        mgDisplayMsgBox();
        mgDisplayMessage(strGetString(strStartingGame));

        if(theMsgR.GetBlobLen()!=sizeof(BeginStartGamePacket))
            return;

        BeginStartGamePacket *aPacket = (BeginStartGamePacket*)theMsgR.GetBlob();

        mGameCreationState = GAME_STARTING;
        ChangeAddress(&(aPacket->oldCaptainAddress),&(aPacket->newCaptainAddress));
        mCaptainAddress = aPacket->newCaptainAddress;

        if(tpGameCreated.userBehindFirewall) {
            mgDisplayMessage(strGetString(strDetectedUserBehindFirewall));
            mgDisplayMessage(strGetString(strConnectingToRoutingServer));

            mUseRoutingServer = true;
            BuildAddress(mRoutingAddress[1], aPacket->routingAddress.AddrPart.IP, aPacket->routingAddress.Port);
            ConnectToRoutingServer(StringToWString(utyName),L"",1);
            return;
        }
        else {
            mgDisplayMessage(strGetString(strConnectingToCaptain));

            mUseRoutingServer = false;
            ChangeAddressPacket aChangePacket;
            aChangePacket.oldAddress = myAddress;
            aChangePacket.newAddress = mMyIPAddress;

            myAddress = mMyIPAddress;
            mGameCreationState = GAME_STARTED;
            InitPacketList();

            titanConnectToClient(&(aPacket->newCaptainAddress));
            titanSendPacketTo(&(aPacket->newCaptainAddress),TITANMSGTYPE_CHANGEADDRESS, &aChangePacket, sizeof(aChangePacket));
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleBeginStartGame: %s", (char*)anEx.what());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


//--MikeN
// Adding StartShutdown method
void
TitanInterface::StartShutdown(unsigned char titanMsgType, const void* thePacket,
                              unsigned short theLen)
{
    if(tpGameCreated.numPlayers>0) { // Send final message
        if(IAmCaptain) BroadcastPacket(titanMsgType, thePacket, theLen);
        else SendPacketTo(&tpGameCreated.playerInfo[0].address, titanMsgType, thePacket, theLen);
    }

    if(!mUseRoutingServer) {
        if (IPGame) // don't close old client connections if IPX Lan
        {
        AutoCrit aCrit(mClientCrit);  // Enter client crit sec
        ClientToPipe::iterator anItr = mClientMap.begin();
        for (; anItr != mClientMap.end(); anItr++)
        {
            SocketPipe* aPipeP = anItr->second.pipe;
            aPipeP->AddOutgoingCmd(new CloseCmd(false, false, 0, false));
            }
        }
    }

    // Close connection to routing server if needed
    CloseRoutingServerConnection(0);
    CloseRoutingServerConnection(1);

    // Mark as closing
    mCloseRequest = true;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


// Routing Server stuff
void TitanInterface::ConnectToRoutingServer(wstring theUserName, const wchar_t* thePassword, int theServer, bool reconnect)
{
    titanDebug("TitanInterface::ConnectToRoutingServer");
    AutoCrit aPipeCrit(mPipeCrit);

    if (mRoutePipe[theServer])
    {
        titanDebug("NOTE: TitanInterface::ConnectToRoutingServer Close existing RoutePipe!");
        mRoutePipe[theServer]->AddOutgoingCmd(new CloseNowCmd(false, false, 0, false));
        mRoutePipe[theServer] = NULL;
    }
    aPipeCrit.Leave();

    mLoggedInToRoutingServer[theServer] = false;

    AutoCrit aCrit(mRoutingCrit);
    // Prepare for the connection
    mHaveReceivedInitialUserList = false;
    mWaitingRequestQueue[theServer].clear();
    mRoutingReconnect[theServer] = reconnect;

    WONMsg::TMsgAuth1Request aMsg;

    try
    {
        if (mNeedToRegisterRoutingServer) // Register server with Directory Server
        {
            static char aPortStr[17];
            for (int iDirServer = 0; iDirServer < DIRSERVER_NUM; ++iDirServer)
            {
                string aDirServerAddress(DIRSERVER_IPSTRINGS[iDirServer]);
                aDirServerAddress += ":";
                aDirServerAddress += itoa(DIRSERVER_PORTS[iDirServer], aPortStr, 10);
                mRegisterRoutingServerMsg.AddDirServerAddress(aDirServerAddress);
            }
            mRegisterRoutingServerMsg.SetPath(HOMEWORLD_DIR);
            mRegisterRoutingServerMsg.SetRequireUniqueDisplayName(true);

            mRegisterRoutingServerMsg.Pack();
        }
        else // Login to the Routing Server
        {
            mRouteRegisterMsg.SetClientName(WONCommon::RawBuffer((unsigned char*)theUserName.c_str(), theUserName.size() * 2));
            if (theServer == 0)
                mRouteRegisterMsg.SetSetupChat(true); // join chat group
            if (thePassword && thePassword[0] != '\0')
                mRouteRegisterMsg.SetPassword(thePassword);
            else
                mRouteRegisterMsg.SetPassword(wstring());

            mRouteRegisterMsg.Pack();
        }

        aMsg.SetAuthMode(WONAuth::AUTH_PERSISTENT);
        aMsg.SetEncryptMode(WONAuth::ENCRYPT_BLOWFISH);
        aMsg.SetEncryptFlags(WONAuth::EFLAGS_NOTSEQUENCED);
        aMsg.SetRawBuf(mCertificate->GetRaw(),mCertificate->GetRawLen());
        aMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::ConnectToRoutingServer: %s", (char*)anEx.what());
        return;
    }

    titanDebug("TitanInterface::ConnectToRoutingServer Connecting to %s.", PrintAddress(mRoutingAddress[theServer]));
    mRoutePipe[theServer] = ConnectAndSend(reinterpret_cast<SOCKADDR&>(mRoutingAddress[theServer]), aMsg, mStreamType, WONMisc::ptUnsignedShort);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::RegisterRoutingServer()
{
    titanDebug("TitanInterface::RegisterRoutingServer");

    if(!mRoutePipe[0]) {
        titanDebug("FAIL: No routing pipe with which to register. ;)");
        return;
    }

    // If we just started a chat server, tell it to register with a directory server
    mNeedToRegisterRoutingServer = false;
    if(!EncryptAndSendRoutingMsg(mRegisterRoutingServerMsg, 0)) {
        titanDebug("Couldn't send RegisterRequest msg even though this is improbable.");
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingRegister(int theServer)
{
    titanDebug("TitanInterface::HandleRoutingRegister");

    if(!mRoutePipe[theServer]) {
        titanDebug("FAIL: No routing pipe with which to register. ;)");
        return;
    }

    if(mRoutingReconnect[theServer]) {
        WONMsg::MMsgRoutingReconnectClient aReconnectMsg;
        aReconnectMsg.SetClientId(mMyClientId[theServer]);
        aReconnectMsg.SetWantMissedMessages(true);

        try {
            aReconnectMsg.Pack();
        }
        catch(WONException&) {
            titanDebug("Error packing reconnect msg.");
            return;
        }

        mRoutingReconnect[theServer] = false;
        AutoCrit aCrit(mRoutingCrit);
        mWaitingRequestQueue[theServer].push_back(WONMsg::RoutingReconnectClient);
        aCrit.Leave();

        if(!EncryptAndSendRoutingMsg(aReconnectMsg, theServer)) {
            titanDebug("Couldn't send reconnect msg even though this is improbable.");
            return;
        }
    }
    else
    {
        // Login to the Routing Server
        if(!EncryptAndSendRoutingMsg(mRouteRegisterMsg, theServer)) {
            titanDebug("Couldn't send RegisterClient msg even though this is improbable.");
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::CloseRoutingServerConnection(int theServer)
{
    titanDebug("TitanInterface::CloseRoutingServerConnection");
    AutoCrit aCrit(mRoutingCrit);

    if (! mRoutePipe[theServer])
    {
        titanDebug("TitanInterface::CloseRoutingServerConnection No connection.");
        return;
    }

    WONMsg::MMsgRoutingDisconnectClient aDisconnectClientMsg;
    try
    {
        aDisconnectClientMsg.SetIsPermanent(true);
        aDisconnectClientMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::CloseRoutingServerConnection: %s", (char*)anEx.what());
        return;
    }

    // Send disconnect and close connection
    aCrit.Leave();

    mPipeCrit.Enter();

    EncryptAndSendRoutingMsg(aDisconnectClientMsg, theServer);
    mRoutePipe[theServer]->AddOutgoingCmd(new CloseCmd(false, false, 0, false));


    mRoutePipe[theServer] = NULL;
    mPipeCrit.Leave();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::InitPacketList(void) {
    mPacketList.clear();
    mPacketTypeList.clear();
    mSeqNum = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::AddPacketToList(const std::basic_string<unsigned char>&thePacket, unsigned char theType)
{
    AutoCrit aCrit(mPacketCrit);

    if(mPacketList.size()>5000) {
        mPacketList.pop_front();
        mPacketTypeList.pop_front();
    }

    mPacketList.push_back(thePacket);
    mPacketTypeList.push_back(theType);

    mSeqNum++;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void
TitanInterface::BroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen) {
    int i;

    if(mGameCreationState==GAME_NOT_STARTED) {
        for (i=0;i<tpGameCreated.numPlayers;i++)
        {
            if (!AddressesAreEqual(tpGameCreated.playerInfo[i].address,myAddress))
            {
                SendPacketTo(&(tpGameCreated.playerInfo[i].address), titanMsgType, thePacket, theLen);
            }
        }
    }
    else if(mGameCreationState==GAME_STARTING) {
        for (i=0;i<mOldtpGameCreated.numPlayers;i++)
        {
            if (!AddressesAreEqual(mOldtpGameCreated.playerInfo[i].address,myAddress))
            {
                SendPacketTo(&(tpGameCreated.playerInfo[i].address), titanMsgType, thePacket, theLen);
            }
        }
    }
    else {
        AutoCrit aCrit(mPacketCrit);

        if(!mUseRoutingServer) {
            for (i=0;i<tpGameCreated.numPlayers;i++)
            {
                if (!AddressesAreEqual(tpGameCreated.playerInfo[i].address,myAddress))
                {
//                  SendPacketTo(&(tpGameCreated.playerInfo[i].address), titanMsgType,(const unsigned char*)thePacket,theLen,IAmCaptain,mSeqNum);
                    SendPacketTo(&(tpGameCreated.playerInfo[i].address), titanMsgType,(const unsigned char*)thePacket,theLen);
                }
            }
        }
        else
            //  SendPacketTo(NULL, titanMsgType,(const unsigned char*)thePacket,theLen,IAmCaptain,mSeqNum);
            SendPacketTo(NULL, titanMsgType,(const unsigned char*)thePacket,theLen);

/*
            if(IAmCaptain) {
                WONCommon::RawBuffer aPacket((const unsigned char*)thePacket,theLen);
                AddPacketToList(aPacket,titanMsgType);
            }*/
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::RoutingSendChatBroadcast(unsigned short theSize, const unsigned char* theDataP, int theServer, bool addSeqNum, int theSeqNum)
{
    titanDebug("TitanInterface::RoutingSendChatBroadcast");
    AutoCrit aCrit(mRoutingCrit);

#if DEBUG_DISCONNECT
    string aStr((const char*)theDataP,theSize);
    if(aStr.find("briandis")!=string::npos) {
        mRoutePipe[theServer]->AddOutgoingCmd(new CloseCmd(false, false, 0, false));
        return;
    }
#endif // DEBUG_DISCONNECT

    if (! mRoutePipe[theServer])
    {
        titanDebug("TitanInterface::RoutingSendChatBroadcast No connection.");
        return;
    }

    if (! mLoggedInToRoutingServer[theServer])
    {
        titanDebug("TitanInterface::RoutingSendChatBroadcast Not logged in yet.");
        return;
    }


    WONMsg::MMsgRoutingSendChat aSendChatMsg;
    try
    {
        if(addSeqNum) {
            aSendChatMsg.AppendToData(WONCommon::RawBuffer((unsigned char*)&theSeqNum,4));
        }

        aSendChatMsg.AppendToData(WONCommon::RawBuffer(theDataP,theSize));
        aSendChatMsg.SetChatType(WONMsg::CHATTYPE_UNICODE);
        aSendChatMsg.SetShouldSendReply(false);
        aSendChatMsg.SetIncludeExcludeFlag(false); // exclude no-one (aka broadcast)
        aSendChatMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RoutingSendChatBroadcast: %s", (char*)anEx.what());
        return;
    }

    if (!EncryptAndSendRoutingMsg(aSendChatMsg, theServer))
    {
        titanDebug("FAIL: TitanInterface::RoutingSendChatBroadcast fail send!");
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::RoutingSendDataBroadcast(unsigned short theSize, const unsigned char* theDataP, int theServer, bool addSeqNum, int theSeqNum)
{
    titanDebug("TitanInterface::RoutingSendDataBroadcast");
    AutoCrit aCrit(mRoutingCrit);

#if DEBUG_DISCONNECT
    string aStr((const char*)theDataP,theSize);
    if(aStr.find("briandis")!=string::npos) {
        mRoutePipe[theServer]->AddOutgoingCmd(new CloseCmd(false, false, 0, false));
        return;
    }
#endif // DEBUG_DISCONNECT

    if (! mRoutePipe[theServer])
    {
        titanDebug("TitanInterface::RoutingSendDataBroadcast No connection.");
        return;
    }

    if (! mLoggedInToRoutingServer[theServer])
    {
        titanDebug("TitanInterface::RoutingSendDataBroadcast Not logged in yet.");
        return;
    }


    WONMsg::MMsgRoutingSendDataBroadcast aSendDataBroadcastMsg;
    try
    {
        if(addSeqNum) {
            aSendDataBroadcastMsg.AppendToData(WONCommon::RawBuffer((unsigned char*)&theSeqNum,4));
        }

        aSendDataBroadcastMsg.AppendToData(WONCommon::RawBuffer(theDataP,theSize));
        aSendDataBroadcastMsg.SetShouldSendReply(false);
        aSendDataBroadcastMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RoutingSendDataBroadcast: %s", (char*)anEx.what());
        return;
    }

    if (!EncryptAndSendRoutingMsg(aSendDataBroadcastMsg, theServer))
    {
        titanDebug("FAIL: TitanInterface::RoutingSendDataBroadcast fail send!");
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::RoutingSendChatWhisper(unsigned long* theIds, unsigned short theNumIds, unsigned short theSize, const unsigned char* theDataP, bool addSeqNum, int theSeqNum)
{
    titanDebug("TitanInterface::RoutingSendChatWhisper");
    AutoCrit aCrit(mRoutingCrit);

#if DEBUG_DISCONNECT
    string aStr((const char*)theDataP,theSize);
    if(aStr.find("dis")!=string::npos) {
        mRoutePipe[theServer]->AddOutgoingCmd(new CloseCmd(false, false, 0, false));
        return;
    }
#endif // DEBUG_DISCONNECT

    if (! mRoutePipe[0])
    {
        titanDebug("TitanInterface::RoutingSendChatWhisper No connection.");
        return;
    }

    if (! mLoggedInToRoutingServer[0])
    {
        titanDebug("TitanInterface::RoutingSendChatWhisper Not logged in yet.");
        return;
    }


    WONMsg::MMsgRoutingSendChat aSendChatMsg;
    try
    {
        aSendChatMsg.SetIncludeExcludeFlag(true);

        if(addSeqNum) {
            aSendChatMsg.AppendToData(WONCommon::RawBuffer((unsigned char*)&theSeqNum,4));
        }

        for (int iId = 0; iId < theNumIds; ++iId)
            aSendChatMsg.AddAddressee(theIds[iId]);
        aSendChatMsg.AppendToData(WONCommon::RawBuffer(theDataP, theSize));
        aSendChatMsg.SetChatType(WONMsg::CHATTYPE_UNICODE);
        aSendChatMsg.SetShouldSendReply(false);
        aSendChatMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RoutingSendChatWhisper: %s", (char*)anEx.what());
        return;
    }

    if (!EncryptAndSendRoutingMsg(aSendChatMsg, 0))
    {
        titanDebug("FAIL: TitanInterface::RoutingSendChatWhisper fail send!");
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::RoutingSendData(WONMsg::ClientId theId, unsigned short theSize, const unsigned char* theDataP, int theServer, bool addSeqNum, int theSeqNum)
{
    titanDebug("TitanInterface::RoutingSendData");
    AutoCrit aCrit(mRoutingCrit);

#if DEBUG_DISCONNECT
    string aStr((const char*)theDataP,theSize);
    if(aStr.find("dis")!=string::npos) {
        mRoutePipe[theServer]->AddOutgoingCmd(new CloseCmd(false, false, 0, false));
        return;
    }
#endif // DEBUG_DISCONNECT

    if (! mRoutePipe[theServer])
    {
        titanDebug("TitanInterface::RoutingSendData No connection.");
        return;
    }

    if (! mLoggedInToRoutingServer[theServer])
    {
        titanDebug("TitanInterface::RoutingSendData Not logged in yet.");
        return;
    }


    WONMsg::MMsgRoutingSendData aSendDataMsg;
    try
    {
        aSendDataMsg.SetIncludeExcludeFlag(true);

        if(addSeqNum) {
            aSendDataMsg.AppendToData(WONCommon::RawBuffer((unsigned char*)&theSeqNum,4));
        }

        aSendDataMsg.AddAddressee(theId);
        aSendDataMsg.AppendToData(WONCommon::RawBuffer(theDataP, theSize));
        aSendDataMsg.SetShouldSendReply(false);
        aSendDataMsg.Pack();
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::RoutingSendData: %s", (char*)anEx.what());
        return;
    }

    if (!EncryptAndSendRoutingMsg(aSendDataMsg, theServer))
    {
        titanDebug("FAIL: TitanInterface::RoutingSendData fail send!");
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingGroupChange(SocketPipe* thePipeP, const MiniMessage& theMsgR, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingGroupChange");
    AutoCrit aCrit(mRoutingCrit);

    if(theServer!=0) // captain figures out when people are gone (want to allow reconnects)
        return;

    try
    {
        WONMsg::MMsgRoutingGroupChange aGroupChangeMsg(theMsgR);

        // only care about chat group
        if (aGroupChangeMsg.GetGroupId() != CHAT_GROUP)
            return;

        // Check if user joined, request user info if so
        if (aGroupChangeMsg.GetReason() & 0x80)
        {
            // ask the server for more info on the client
            WONMsg::MMsgRoutingGetClientInfo aGetClientInfoMsg;
            aGetClientInfoMsg.SetClientId(aGroupChangeMsg.GetClientId());
            aGetClientInfoMsg.SetAuthInfoRequested(false);
            aGetClientInfoMsg.Pack();

            if (!EncryptAndSendRoutingMsg(aGetClientInfoMsg, theServer))
            {
                titanDebug("FAIL: TitanInterface::HandleRoutingGroupChange fail send!");
                return;
            }
        }
        else {
            if(mUseRoutingServer && IAmCaptain && mGameCreationState!=GAME_STARTED) {
                Address anAddr;
                anAddr.AddrPart.IP = aGroupChangeMsg.GetClientId();
                anAddr.Port = aGroupChangeMsg.GetClientId();
                titanLeaveGameReceivedCB(&anAddr, NULL, 0);
            }

            chatReceiveUserLeft(aGroupChangeMsg.GetClientId());
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingGroupChange: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingGroupChangeEx(SocketPipe* thePipeP, const MiniMessage& theMsgR, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingGroupChangeEx");
    AutoCrit aCrit(mRoutingCrit);

    if(theServer!=0)  // captain figures out when people are gone (want to allow reconnects)
        return;

    try
    {
        WONMsg::MMsgRoutingGroupChangeEx aGroupChangeExMsg(theMsgR);

        // only care about chat group
        if (aGroupChangeExMsg.GetGroupId() != CHAT_GROUP)
            return;

        // Check if user joined, request user info if so
        if (aGroupChangeExMsg.GetReason() & 0x80)
        {
            OnNewRoutingServerClient(aGroupChangeExMsg.GetClientId(),
                                     wstring(reinterpret_cast<const wchar_t*>(aGroupChangeExMsg.GetClientName().data()), aGroupChangeExMsg.GetClientName().size() / 2),
                                     aGroupChangeExMsg.GetIPAddress(),
                                     theServer);
        }
        else
        {
            WONMsg::MMsgRoutingGroupChange aDummyGroupChangeMsg;
            aDummyGroupChangeMsg.SetClientId(aGroupChangeExMsg.GetClientId());
            aDummyGroupChangeMsg.SetReason(aGroupChangeExMsg.GetReason());
            aDummyGroupChangeMsg.Pack();
            HandleRoutingGroupChange(thePipeP, aDummyGroupChangeMsg, theServer);
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingGroupChangeEx: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingCreateDataObject(SocketPipe* thePipeP, const MiniMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleRoutingCreateDataObject.");

    try
    {
        WONMsg::MMsgRoutingCreateDataObject aCreateDataObjectMsg(theMsgR);

        tpscenario aGame;
        memcpy(aGame.Name,
               aCreateDataObjectMsg.GetDataType().c_str() + strlen((char*)gameTag),
               aCreateDataObjectMsg.GetDataType().size() - strlen((char*)gameTag));
        aGame.Name[(aCreateDataObjectMsg.GetDataType().size() - strlen((char*)gameTag))/2] = '\0';
        memcpy(&aGame.directoryCustomInfo,
               aCreateDataObjectMsg.GetData().c_str(),
               aCreateDataObjectMsg.GetData().size());
        mgGameListGameAdded(&aGame);
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingCreateDataObject: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingDeleteDataObject(SocketPipe* thePipeP, const MiniMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleRoutingDeleteDataObject.");

    try
    {
        WONMsg::MMsgRoutingDeleteDataObject aDeleteDataObjectMsg(theMsgR);

        tpscenario aGame;
        memcpy(aGame.Name,
               aDeleteDataObjectMsg.GetDataType().c_str() + strlen((char*)gameTag),
               aDeleteDataObjectMsg.GetDataType().size() - strlen((char*)gameTag));
        aGame.Name[(aDeleteDataObjectMsg.GetDataType().size() - strlen((char*)gameTag))/2] = '\0';
        memset(&aGame.directoryCustomInfo, 0, sizeof(aGame.directoryCustomInfo));
        mgGameListGameRemoved(&aGame);
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingDeleteDataObject: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingGetClientListReply(SocketPipe* thePipeP, const MiniMessage& theMsgR, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingGetClientListReply.");

    try
    {
        WONMsg::MMsgRoutingGetClientListReply aGetClientListReply(theMsgR);

        std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData>::iterator itr = aGetClientListReply.GetClientList().begin();
        for (; itr != aGetClientListReply.GetClientList().end(); itr++)
            OnNewRoutingServerClient(itr->mClientId, wstring(reinterpret_cast<const wchar_t*>(itr->mClientName.data()), itr->mClientName.size() / 2), itr->mIPAddress, theServer);

        if(theServer==0)
            mHaveReceivedInitialUserList = true;
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingGetClientListReply: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::OnNewRoutingServerClient(WONMsg::ClientId theClientId, const wstring& theUserNameR, unsigned long theIPAddress, int theServer)
{
    if (theClientId == mMyClientId[theServer])
    {
        mMyIPAddress.AddrPart.IP = theIPAddress;
        mMyIPAddress.Port = GAME_PORT;

        if(!mUseRoutingServer) {
            myAddress.AddrPart.IP = theIPAddress;
            myAddress.Port = GAME_PORT;
        }
    }
    else if(theServer==0)
    {
        if (mHaveReceivedInitialUserList)
            chatReceiveUsersJoined(WStringToString(theUserNameR).c_str(), theClientId);
        else
            chatReceiveUsersHere(WStringToString(theUserNameR).c_str(), theClientId);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingPeerChat(SocketPipe* thePipeP, const MiniMessage& theMsgR, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingPeerChat");

   try
    {
        WONMsg::MMsgRoutingPeerChat aPeerChatMsg(theMsgR);
        if(aPeerChatMsg.GetData().size()<=0)
            return;

        if(aPeerChatMsg.GetClientId() != mMyClientId[0])
            chatReceiveMessage(aPeerChatMsg.GetClientId(), (aPeerChatMsg.GetAddresseeList().size() > 0), 0, aPeerChatMsg.GetData().size(), aPeerChatMsg.GetData().c_str());
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingPeerData: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingPeerData(SocketPipe* thePipeP, const WONCommon::RawBuffer &theData, WONMsg::ClientId theId, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingPeerData");

    MiniMessage aMsg(theData.size(), theData.c_str());
    thePipeP->SetLabel(theId);
    HandlePeerMsg(thePipeP, aMsg);
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingPeerData(SocketPipe* thePipeP, const MiniMessage& theMsgR, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingPeerData1.");

    try
    {
        WONMsg::MMsgRoutingPeerData aPeerDataMsg(theMsgR);
        HandleRoutingPeerData(thePipeP, aPeerDataMsg.GetData(), aPeerDataMsg.GetClientId(), theServer);
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingPeerData: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingPeerDataMultiple(SocketPipe* thePipeP, const MiniMessage& theMsgR, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingPeerDataMultiple.");

    try
    {
        WONMsg::MMsgRoutingPeerDataMultiple aPeerDataMultipleMsg(theMsgR);

        std::list<PeerDataMessage>::iterator itr = aPeerDataMultipleMsg.GetMessageList().begin();
        for (; itr != aPeerDataMultipleMsg.GetMessageList().end(); itr++)
            HandleRoutingPeerData(thePipeP, itr->mData, itr->mClientId, theServer);
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingPeerDataMultiple: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingRegisterClientReply(SocketPipe* thePipeP, const MiniMessage& theMsgR, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingRegisterClientReply.");
    AutoCrit aCrit(mRoutingCrit);

    try
    {
        WONMsg::MMsgRoutingRegisterClientReply aRegisterClientReply(theMsgR);

        mMyClientId[theServer] = aRegisterClientReply.GetClientId();
        mLoggedInToRoutingServer[theServer] = true;
        mHaveConnectedToAChatServer = true;

        if (theServer == 0)
            chatReceiveUserJoinReply(aRegisterClientReply.GetStatus(), mMyClientId[theServer]);

        if (aRegisterClientReply.GetStatus() != WONMsg::StatusCommon_Success)
        {
            titanDebug("TitanInterface::HandleRoutingRegisterClientReply Failed! Status = %d", aRegisterClientReply.GetStatus());
            return;
        }

        if (theServer == 1) // we registered with a game server
        {
            if(IAmCaptain) {
                mgDisplayMessage(strGetString(strTellingPlayersConnectRoutServ));

                BeginStartGamePacket aPacket;

                aPacket.routingAddress.AddrPart.IP = mRoutingAddress[1].sin_addr.S_un.S_addr;
                aPacket.routingAddress.Port = ntohs(mRoutingAddress[1].sin_port);

                aPacket.oldCaptainAddress.AddrPart.IP = mMyClientId[0];
                aPacket.oldCaptainAddress.Port = mMyClientId[0];

                aPacket.newCaptainAddress.AddrPart.IP = mMyClientId[1];
                aPacket.newCaptainAddress.Port = mMyClientId[1];

                titanBroadcastPacket(TITANMSGTYPE_BEGINSTARTGAME, &aPacket, sizeof(aPacket));
                ChangeAddress(&(aPacket.oldCaptainAddress), &(aPacket.newCaptainAddress));

                OnCaptainStartedGame();
            }
            else {
                mgDisplayMessage(strGetString(strConnectingToRoutingServer));

                mGameCreationState = GAME_STARTED;
                InitPacketList();

                ChangeAddressPacket aChangePacket;
                aChangePacket.oldAddress.AddrPart.IP = mMyClientId[0];
                aChangePacket.oldAddress.Port = mMyClientId[0];

                aChangePacket.newAddress.AddrPart.IP = mMyClientId[1];
                aChangePacket.newAddress.Port = mMyClientId[1];

                titanSendPacketTo(&mCaptainAddress,TITANMSGTYPE_CHANGEADDRESS, &aChangePacket, sizeof(aChangePacket));
            }

            myAddress.AddrPart.IP = mMyClientId[theServer];
            myAddress.Port = mMyClientId[theServer];
        }
        else // we registered with a chat server
        {
            if(mUseRoutingServer) {
                myAddress.AddrPart.IP = mMyClientId[0];
                myAddress.Port = mMyClientId[0];
            }

            // now that we're registered, there are a few things to do
            if (mRoutePipe[0])
            {
                // if we've just entered the lobby for the first time (during this session), record the event
                if (!mHasLobbyEnterEventBeenSent)
                    OnInitialLobbyEnter();

                // get the user list
                WONMsg::MMsgRoutingGetClientList aGetClientListMsg;
                aGetClientListMsg.SetAuthInfoRequested(false);
                aGetClientListMsg.Pack();

                if (!EncryptAndSendRoutingMsg(aGetClientListMsg, theServer))
                {
                    titanDebug("FAIL: TitanInterface::HandleRoutingRegisterClientReply fail send RoutingGetClientList!");
                    return;
                }

                // subscribe to the Homeworld game data
                WONMsg::MMsgRoutingSubscribeDataObject aSubscribeDataObjectMsg;
                aSubscribeDataObjectMsg.SetLinkId(0 /* ALL_USERS */);
                aSubscribeDataObjectMsg.SetDataType(gameTag);
                aSubscribeDataObjectMsg.SetExactOrRecursiveFlag(false);
                aSubscribeDataObjectMsg.SetGroupOrMembersFlag(true);
                aSubscribeDataObjectMsg.Pack();

                if (!EncryptAndSendRoutingMsg(aSubscribeDataObjectMsg, theServer))
                {
                    titanDebug("FAIL: TitanInterface::HandleRoutingRegisterClientReply fail send RoutingSubscribeDataObject!");
                    return;
                }
            }
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingRegisterClientReply: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingReadDataObjectReply(SocketPipe* thePipeP, const MiniMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleRoutingReadDataObjectReply.");

    try
    {
        WONMsg::MMsgRoutingReadDataObjectReply aReadDataObjectReplyMsg(theMsgR);

        tpscenario aGame;

        WONMsg::MMsgRoutingReadDataObjectReply::DataObjectSet::iterator itr = aReadDataObjectReplyMsg.GetDataObjects().begin();
        for (; itr != aReadDataObjectReplyMsg.GetDataObjects().end(); itr++)
        {
            memcpy(aGame.Name,
                   itr->GetDataType().c_str() + strlen((char*)gameTag),
                   itr->GetDataType().size() - strlen((char*)gameTag));
            aGame.Name[(itr->GetDataType().size() - strlen((char*)gameTag))/2] = '\0';
            memcpy(&aGame.directoryCustomInfo, itr->GetData().c_str(), itr->GetData().size());
            mgGameListGameAdded(&aGame);
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingReadDataObjectReply: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingReplaceDataObject(SocketPipe* thePipeP, const MiniMessage& theMsgR)
{
    titanDebug("TitanInterface::HandleRoutingReplaceDataObject.");

    try
    {
        WONMsg::MMsgRoutingReplaceDataObject aReplaceDataObjectMsg(theMsgR);

        tpscenario aGame;
        memcpy(aGame.Name,
               aReplaceDataObjectMsg.GetDataType().c_str() + strlen((char*)gameTag),
               aReplaceDataObjectMsg.GetDataType().size() - strlen((char*)gameTag));
        aGame.Name[(aReplaceDataObjectMsg.GetDataType().size() - strlen((char*)gameTag))/2] = '\0';
        memcpy(&aGame.directoryCustomInfo,
               aReplaceDataObjectMsg.GetData().c_str(),
               aReplaceDataObjectMsg.GetData().size());
        mgGameListGameChanged(&aGame);
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingReplaceDataObject: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::HandleRoutingStatusReply(SocketPipe* thePipeP, const MiniMessage& theMsgR, int theServer)
{
    titanDebug("TitanInterface::HandleRoutingStatusReply.");

    try
    {
        WONMsg::MMsgRoutingStatusReply aStatusReplyMsg(theMsgR);
        short aStatus = aStatusReplyMsg.GetStatus();

        // were we expecting a StatusReply (can't really tell unless queue is empty)
        if (mWaitingRequestQueue[theServer].size() == 0)
        {
            titanDebug("TitanInterface::HandleRoutingStatusReply: **SOFTWARE ERROR** Received unexpected StatusReply with status %d.", aStatus);
            _ASSERT(mWaitingRequestQueue[theServer].size() != 0); // we're slightly hosed...requests and replies out of sync
        }

        // what type of request elicited this RoutingStatusReply?
        unsigned char aRequestType = mWaitingRequestQueue[theServer].front();
        mWaitingRequestQueue[theServer].pop_front();

        // take action based on the reply
        if (aRequestType == WONMsg::RoutingCreateDataObject)
        {
            titanDebug("TitanInterface::HandleRoutingStatusReply: Got status (%d) when creating game object", aStatus);
            cNotifyGameCreationStatus(aStatus);
        }
        else if(aRequestType == WONMsg::RoutingReconnectClient)
        {
            if(aStatus!=WONMsg::StatusCommon_Success) {
                // What now? mgNotifyReconnectFailed();
                CloseRoutingServerConnection(theServer);
                return;
            }

            clCommandMessage("Reconnected.");

            mLoggedInToRoutingServer[theServer] = true;
            mRoutingReconnectNum[theServer] = 0;
        }
    }
    catch (WONException& anEx)
    {
        titanDebug("EXCEPTION: TitanInterface::HandleRoutingStatusReply: %s", (char*)anEx.what());
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::OnInitialLobbyEnter()
{
    RecordEvent(ACTIVITYTYPE_HOMEWORLD_LOBBY_ENTER);
    mHasLobbyEnterEventBeenSent = true;

    // enter the Homeworld URL into the MediaMetrix edit box
    if (mMediaMetrixHWND) SendMessage(mMediaMetrixHWND, WM_SETTEXT, 0, (LPARAM)MEDIAMETRIX_URL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::OnFinalLobbyExit()
{
    RecordEvent(ACTIVITYTYPE_HOMEWORLD_LOBBY_EXIT);

    // clear the Homeworld URL from the MediaMetrix edit box
    if (mMediaMetrixHWND) SendMessage(mMediaMetrixHWND, WM_SETTEXT, 0, (LPARAM) "");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::OnCaptainStartedGame()
{
    // Record an event indicating that a new game has been created and that we joined it
    RecordEvent(ACTIVITYTYPE_HOMEWORLD_GAME_CREATE);
    RecordEvent(ACTIVITYTYPE_HOMEWORLD_GAME_ENTER);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void TitanInterface::CreateMediaMetrixEditControl()
{
    if (!mIsLan)
        mMediaMetrixHWND = CreateWindow("EDIT",         // predefined class
                                        NULL,           // no window title
                                        WS_CHILD,       // not visible. not tabstop
                                        0, 0, 100, 50,  // arbitrary size
                                        ghMainWindow,   // parent window
                                        (HMENU) NULL,   // edit control ID
                                        (HINSTANCE) GetWindowLong(ghMainWindow, GWL_HINSTANCE),
                                        NULL);          // pointer not needed
}

unsigned long TitanInterface::GetHashSection(bool restart, unsigned char** theUnhashedBufP, unsigned char digest[MD5_HASH_SIZE])
{
    const unsigned int HASH_CHUNK_SIZE = 16384; // number of writes read from the exe per GetHashSection call
    static HANDLE aFileH = INVALID_HANDLE_VALUE;

    if (restart)
    {
        char aFileName[MAX_PATH];
        if (GetModuleFileName(GetModuleHandle(NULL), aFileName, MAX_PATH) == 0)
            return 0;

        aFileH = CreateFile(aFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);

        if (*theUnhashedBufP == NULL)
            *theUnhashedBufP = new unsigned char[HASH_CHUNK_SIZE];
    }

    DWORD dwBytesRead;
    if (ReadFile(aFileH, *theUnhashedBufP, HASH_CHUNK_SIZE, &dwBytesRead, NULL) == 0 || dwBytesRead == 0)
        return 0;

    struct MD5Context Context;
    MD5Init2(&Context);
    MD5Update2(&Context, *theUnhashedBufP, dwBytesRead);
    MD5Final2(digest, &Context);

    return dwBytesRead;
}

void TitanInterface::ShortCircuitChallengeResponse(unsigned char* theSeed, unsigned char* theChallengeResponseP)
{
    ReadFromWonstuff(true, theChallengeResponseP);

    struct MD5Context Context;
    MD5Init2(&Context);
    MD5Update2(&Context, theSeed, MD5_HASH_SIZE);
    while (ReadFromWonstuff(false, theChallengeResponseP + MD5_HASH_SIZE))
        MD5Update2(&Context, theChallengeResponseP + MD5_HASH_SIZE, MD5_HASH_SIZE);
    MD5Final2(theChallengeResponseP + MD5_HASH_SIZE, &Context);
}

bool TitanInterface::SaveWonstuff()
{
    unsigned char digest[MD5_HASH_SIZE];
    DWORD dwBytesWritten;

    char readfile[500];

    strcpy(readfile,filePrependPath);
    strcat(readfile,"WONstuff.txt");

    // open the wonstuff.txt file
    HANDLE aFileH = CreateFile(readfile,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               CREATE_ALWAYS,
                               0,
                               NULL);

    if (aFileH == INVALID_HANDLE_VALUE)
    {
        DWORD a = GetLastError();
        return false;
    }

    // save space for the whole-file hash
    SetFilePointer(aFileH, MD5_HASH_SIZE, NULL, FILE_BEGIN);

    struct MD5Context Context;
    MD5Init2(&Context);

    // write out the partial-file digests
    bool firsttime = true;
    DWORD dwBytesRead;
    unsigned char* aUnhashedBuf = NULL;
    while ((dwBytesRead = GetHashSection(firsttime, &aUnhashedBuf, digest)) != 0)
    {
        firsttime = false;

        // keep track of the whole-file hash
        MD5Update2(&Context, aUnhashedBuf, dwBytesRead);

        // write out the partial-file hash
        if (WriteFile(aFileH, digest, MD5_HASH_SIZE, &dwBytesWritten, NULL) == 0 || dwBytesWritten != MD5_HASH_SIZE)
            return false;
    }

    // write out the whole-file hash
    SetFilePointer(aFileH, 0, NULL, FILE_BEGIN);
    MD5Final2(digest, &Context);
    if (WriteFile(aFileH, digest, MD5_HASH_SIZE, &dwBytesWritten, NULL) == 0 || dwBytesWritten != MD5_HASH_SIZE)
        return false;

    CloseHandle(aFileH);
    return true;
}

int titanSaveWonstuff()
{
    return TitanInterface::SaveWonstuff();
}

bool TitanInterface::ReadFromWonstuff(bool restart, unsigned char* theBufferP)
{
    static HANDLE aFileH = INVALID_HANDLE_VALUE;
    char readfile[500];

    strcpy(readfile,filePrependPath);
    strcat(readfile,"WONstuff.txt");

    if (restart)
    {
        aFileH = CreateFile(readfile,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);

        if (aFileH == INVALID_HANDLE_VALUE)
            return false;
    }

    DWORD dwBytesRead;
    if (ReadFile(aFileH, theBufferP, MD5_HASH_SIZE, &dwBytesRead, NULL) == 0 || dwBytesRead == 0)
    {
        CloseHandle(aFileH);
        aFileH = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}
