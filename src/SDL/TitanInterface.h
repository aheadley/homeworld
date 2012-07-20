#ifndef _TitanInterface_H
#define _TitanInterface_H

#include "DEQUE"
#include "MAP"
#include "LIST"
#include "STRING"
#include "VECTOR"
#include "common/CriticalSection.h"
#include "common/ThreadBase.h"
#include "msg/comm/SMsgCommRegisterRequest.h"
#include "msg/fact/SMsgFactStartProcessUnicode.h"
#include "msg/routing/MMsgRoutingRegisterClient.h"
#include "EasySocket/EasySocket.h"
#include "EasySocket/PipeCmd.h"
#include "crypt/EGPrivateKey.h"
#include "crypt/EGPublicKey.h"
#include "crypt/BFSymmetricKey.h"
#include "auth/Auth1PublicKeyBlock.h"
#include "auth/Auth1Certificate.h"
typedef int BOOL;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef unsigned char BYTE;
#include "MD52.h"

extern "C"
{
#include "TitanInterfaceC.h"
}

namespace WONMisc {
    class EasySocketEngine;
    class SocketPipe;
    class EasySocket;
};

namespace WONMsg {
    class TRawMsg;
    class TMessage;
};

enum IpType {
    ip,
    ipx
};

typedef union {
    WONMisc::SocketPipe *pipe;
    WONMsg::ClientId id;
} ClientMapUnion;


typedef std::pair<unsigned long, unsigned long> ClientNetAddr;
typedef std::map<ClientNetAddr, ClientMapUnion> ClientToPipe;

class TitanInterface : public WONCommon::ThreadBase
{
public:
    TitanInterface(unsigned long isLan, unsigned long isIP);
    ~TitanInterface();

    // Overridden from ThreadBase
    void startThread();
    void stopThread();

    void loadVerifierKey();

    //Creates a socket to verify that networking with current protocol is possible
    unsigned long CanNetwork(void);

    //--MikeN
    // Begin shutdown procesing.  Sends shutdown msg to all connected clients and
    // closes connection after msg has been sent.
    void StartShutdown(unsigned char titanMsgType, const void* thePacket, unsigned short theLen);

    // Request the list of valid Homeworld client versions from a Directory Server
    void RequestValidVersions();

    // Request directory contents, theDir is subdir of /Homeworld dir
    // Set theDir to NULL to get contents of /Homeworld
    void RequestDirectory();

    // Game data maintenance (create, delete, replace)
    void RequestCreateGame(const wchar_t* theGame, DirectoryCustomInfo* myInfo);
    void RequestDeleteGame(const wchar_t* theGame);
    void RequestReplaceGame(const wchar_t* theGame, DirectoryCustomInfo* myInfo, unsigned long replaceTimeout);
    void RequestRenewGameLifespan(const wchar_t* theGame, unsigned long newLifespan);

    // Connect to a client
    void ConnectToClient(Address* theAddressP);

    // Ping a game
    void SendPing(Address* theAddressP,unsigned int pingsizebytes);

    // Send a Lan broadcast
    void SendLanBroadcast(const void* thePacket, unsigned short theLen);
    void BroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen);

    // Send a packet to a client
    void SendPacketTo(Address* theAddressP, unsigned char titanMsgType,
                      const void* thePacket, unsigned short theLen,
                      bool appendSeqNum = false, int theSeqNum = 0);

    // Authentication stuff
    void Authenticate(const string &loginName, const string &password, const string &theNewPassword, bool CreateAccount);


    // Routing Server stuff
    unsigned long StartRoutingServer(const wchar_t* theChannelName, const wchar_t* theChannelDescription, const wchar_t* thePassword, bool isGameServer,unsigned char *routingaddress);
    void RegisterRoutingServer(void);
    void HandleRoutingRegisterReply(WONMisc::SocketPipe* thePipeP, const WONMsg::SmallMessage& theMsgR);
    void ConnectToRoutingServer(wstring theUserName, const wchar_t* thePassword, int theServer, bool reconnect = false);
    void CloseRoutingServerConnection(int theServer);
    void RoutingSendChatBroadcast(unsigned short theSize, const unsigned char* theDataP, int theServer = 0, bool appendSeqNum = false, int theSeqNum = 0);
    void RoutingSendChatWhisper(unsigned long* theIds, unsigned short theNumIds, unsigned short theSize, const unsigned char* theDataP, bool addSeqNum =false, int theSeqNum =0);
    void RoutingSendDataBroadcast(unsigned short theSize, const unsigned char* theDataP, int theServer = 0, bool appendSeqNum = false, int theSeqNum = 0);
    void RoutingSendData(WONMsg::ClientId theId, unsigned short theSize, const unsigned char* theDataP, int theServer = 0, bool appendSeqNum = false, int theSeqNum = 0);

    void SetGameKey(unsigned char *key);
    const unsigned char *GetGameKey(void);

    // Patch stuff
    int GetPatch(const char *theFilename,const char *saveFileName);
    static void GetPatch(void *theArgs);

    bool CheckStartingGame(unsigned char *routingaddress);
    bool BehindFirewall(void);
    void LeaveGameNotify(void);
    Address GetMyPingAddress(void);

    void ConnectingCancelHit();

//  void QueryRoutingServers(void);

    void PumpEngine();

    void OnInitialLobbyEnter(void);
    void OnFinalLobbyExit(void);
    void OnCaptainStartedGame(void);
    void CreateMediaMetrixEditControl(void);
    static bool TitanInterface::SaveWonstuff();
private:
    bool mUseRoutingServer; // Are we using a routing server for game communication?
    bool mUseOldScheme; // Old
    bool mLaunched;
    bool mBehindFirewall;
    bool mIsGameServer; // is the Routing Server that we're starting a game server (true) or a chat server (false)
    bool mAmCaptain;
    bool mRoutingReconnect[2];
    int mRoutingReconnectNum[2];
    bool mLoggedInToRoutingServer[2];
    bool mHaveConnectedToAChatServer;
    bool mGameDisconnectWasVoluntary;
    bool mFailFactOverDirectly;
    int mCaptainReconnectNum;
    HWND mMediaMetrixHWND; // MediaMetrix edit control HANDLE

    std::vector<std::wstring> mRoutingQueryMap;
//    std::vector<SOCKADDR_IN> mRoutingAddrMap;
    unsigned short mRoutingQueryOffset;

    void InitPacketList(void);
    void AddPacketToList(const std::basic_string<unsigned char> &thePacket, unsigned char theType);
    std::list<std::basic_string<unsigned char> > mPacketList;
    std::list<unsigned char> mPacketTypeList;
    unsigned long mFirstPacket;

    unsigned long mSeqNum;

    CaptainGameInfo mOldtpGameCreated;

    enum { GAME_NOT_STARTED, GAME_STARTING, GAME_STARTED } mGameCreationState;

    struct LoginKeyStruct {
        unsigned long authAddr;
        unsigned short authPort;
        char loginKey[8];
        char newLoginKey[8];
    };

    char mOldNewLoginKey[8];

    int mNumPlayersJoined;

    Address mMyIPAddress; // Remember this for later
    Address mCaptainAddress;

    WONMisc::EasySocketEngine* mEngine;        // Socket engine
    WONMisc::SocketPipe*       mAuthPipe;      // AuthServer connection
    WONMisc::SocketPipe*       mDirPipe;       // DirServer connection
    WONMisc::SocketPipe*       mEventPipe;     // EventServer "connection" (UDP)
    WONMisc::SocketPipe*       mFactPipe;      // FactoryServer connection
    WONMisc::SocketPipe*       mRoutePipe[2];  // RoutingServer connection
    WONMisc::SocketPipe*       mLanAdPipe;     // For advertising games, people, LAN chat
    WONMisc::SocketPipe*       mTimerPipe;     // For refreshing certificate
    WONMisc::SocketPipe*       mFirewallPipe;
    WONMisc::SocketPipe*       mFactPingPipe;
    WONMisc::SocketPipe*       mRoutingInfoPipe;
    WONMisc::SocketPipe*       mCaptainPipe;


    long mCurFactPing;
    unsigned long mMinPingTime;
    int mNumPingTrials;

    ClientToPipe               mClientMap;     // Client connections
    bool                       mCloseRequest;  // Shutdown requested?
    IpType                     mIpType;        // ip or ipx
    bool                       mIsLan;         // is this a lan game
    WONMisc::EasySocket::SocketType     mDatagramType;  // IPX or UDP
    WONMisc::EasySocket::SocketType     mStreamType;    // SPX or TCP

// Auth stuff
    WONCrypt::EGPublicKey *mVerifierKey;                                // Auth verifier key
    WONCrypt::EGPrivateKey *mPrivateKey;                                // Private key for certificate
    WONAuth::Auth1PublicKeyBlock *mPublicKeyBlock;          // Auth public key block
    WONAuth::Auth1Certificate *mCertificate;                        // Auth certificate
    time_t mAuthDeltaTime;                                                      // Difference from auth server clock
    bool mNeedToAuthenticateAfterGettingAuthDirectory;

    WONCrypt::BFSymmetricKey *mAuthSessionKey;
    WONCrypt::BFSymmetricKey *mDirSessionKey;
    WONCrypt::BFSymmetricKey *mDirClientSecret;
    WONCrypt::BFSymmetricKey *mFactSessionKey;
    WONCrypt::BFSymmetricKey *mFactClientSecret;
    WONCrypt::BFSymmetricKey *mRouteSessionKey[2];
    WONCrypt::BFSymmetricKey *mRouteClientSecret[2];

    unsigned short mDirInSeqNum;
    unsigned short mDirOutSeqNum;
    unsigned short mDirSessionId;

    unsigned short mFactSessionId;

    unsigned long mNumAuthServersTried;
    unsigned long mCurAuthServer;

    unsigned long mNumDirServersTried;
    unsigned long mCurDirServer;

    unsigned long mNumFactServersTried;
    unsigned long mCurFactServer;

    unsigned long mNumFirewallServersTried;
    unsigned long mCurFirewallServer;

    std::wstring mChannelName;
    std::wstring mChannelDescription;
    std::wstring mRoomPassword;

    std::string mLoginName;
    std::string mPassword;
    std::string mNewPassword;
    bool mCreateAccount;

    WONMsg::SMsgFactStartProcessUnicode mStartProcessMsg;
    WONMsg::MMsgRoutingRegisterClient   mRouteRegisterMsg;
    WONMsg::SMsgCommRegisterRequest     mRegisterRoutingServerMsg;
    bool                                mNeedToRegisterRoutingServer;

    unsigned long FACTSERVER_NUM;
    unsigned long AUTHSERVER_NUM;
    unsigned long FIREWALLSERVER_NUM;
    unsigned long EVENTSERVER_NUM;
    SOCKADDR_IN FACTSERVER_ADDRESSES[MAX_IPS];
    SOCKADDR_IN AUTHSERVER_ADDRESSES[MAX_IPS];
    SOCKADDR_IN FIREWALLSERVER_ADDRESSES[MAX_IPS];
    SOCKADDR_IN EVENTSERVER_ADDRESSES[MAX_IPS];

    // For Routing Server
    WONMsg::ClientId mMyClientId[2];                   // client id of the local user
    bool           mHaveReceivedInitialUserList;  // Initial list of users here?
    SOCKADDR_IN    mRoutingAddress[2];               // Routing server address
    std::deque<unsigned char> mWaitingRequestQueue[2]; // queue of messages (message types) waiting for StatusReply messages
    WONCrypt::BFSymmetricKey mGameKey; // stored on Routing Server when you create a game

    // Critical sections
    WONCommon::CriticalSection mPipeCrit;
    WONCommon::CriticalSection mClientCrit;
    WONCommon::CriticalSection mRoutingCrit;
    WONCommon::CriticalSection mPacketCrit;
    WONCommon::CriticalSection mStartRoutingCrit;

    int threadProcess();
    void ChangeAddress(Address *theOldAddress, Address *theNewAddress);

    void HandleWaitCmd(WONMisc::PipeCmd* theCmdP);
    void HandleAcceptCmd(WONMisc::SocketPipe* thePipeP, WONMisc::PipeCmd* theCmdP);
    void HandleCloseCmd(WONMisc::SocketPipe* thePipeP);
    void HandleRecvCmd(WONMisc::SocketPipe* thePipeP, WONMisc::PipeCmd* theCmdP, bool pipeClosed);
    void HandleTitanMsg(WONMisc::SocketPipe* thePipeP, const char* theBufP, unsigned long theLen);
    void HandleSmallMsg(WONMisc::SocketPipe* thePipeP, const char* theBufP, unsigned long theLen);
    void HandleMiniMsg(WONMisc::SocketPipe* thePipeP, const char* theBufP, unsigned long theLen);
    void HandleLanBroadcastMsg(WONMisc::SocketPipe* thePipeP, const char* theBufP, unsigned long theLen);

    // Directory server stuff
    void DirFailOver(void);
    void ResetDirFailOver(void);

    void DirHandleGetTitanServers(void);
    void DirHandleGetHWDirectory(void);
    void DirStartPeerLogin(void);
    void HandleDirStatusReply(const WONMsg::TMessage& theMsgR);
    void HandleGetDirReply(const WONMsg::TMessage& theMsgR);
    void HandleDirMultiEntityReply(const WONMsg::SmallMessage& theMsgR);

    // Factory Server Stuff
    void FactFailOver(void);
    void ResetFactFailOver(void);

    void PingFactServer(void);
    void PingHandleReply(const WONMsg::MiniMessage& theMsgR);

    void HandleStartRoutingReply(WONMisc::SocketPipe* thePipeP, const WONMsg::SmallMessage& theMsgR);
    void FactHandleStartProcess(void);

    // Firewall Server stuff
    void FirewallFailOver(void);
    void FirewallDetect(void);
    void HandleFirewallResponse(WONMisc::SocketPipe *thePipeP, const WONMsg::SmallMessage& theMsgR);

// Authentication server stuff
    void AuthFailOver(void);
    void ResetAuthFailOver(void);

    bool ReadLoginKey(char *theKey);
    void WriteLoginKey(char *theKey, bool useOldNewLoginKey = false);

    void AuthHandleGetPubKeysReply(const WONMsg::TMessage &theMsg);
    void AuthGetPubKeyBlock(void);
    void AuthHandleLogin(void);
    void AuthHandleChallenge(const WONMsg::TMessage &theMsg);
    void AuthHandleLoginReply(const WONMsg::TMessage &theMsg);
    void AuthHandleRefresh(void);

    void PeerHandleChallenge(WONMisc::SocketPipe** thePipeP, const WONMsg::TMessage& theMsgR);
    void PeerHandleComplete(WONMisc::SocketPipe** thePipeP, const WONMsg::TMessage& theMsgR);
    void PeerHandleMiniChallenge(WONMisc::SocketPipe** thePipeP, const WONMsg::MiniMessage& theMsgR);
    void PeerHandleMiniComplete(WONMisc::SocketPipe** thePipeP, const WONMsg::MiniMessage& theMsgR);

    bool EncryptMessage(const WONMsg::BaseMessage &theInMsg, WONMsg::BaseMessage &theOutMsg, const WONCrypt::BFSymmetricKey &theKey, unsigned short theSessionId, unsigned short *theSeqNum);
//  bool EncryptTMessage(const WONMsg::BaseMessage &theInMsg, WONMsg::BaseMessage &theOutMsg, const WONCrypt::BFSymmetricKey &theKey, unsigned short theSessionId, unsigned short *theSeqNum);
    bool EncryptNonTMessage(const WONMsg::BaseMessage &theInMsg, WONMsg::BaseMessage &theOutMsg, const WONCrypt::BFSymmetricKey &theKey, unsigned short theSessionId, unsigned short *theSeqNum);

    bool DecryptMessage(const char *theBuf, unsigned long theLen, WONMsg::BaseMessage &theOutMsg, WONMisc::SocketPipe **thePipePP);
//  bool DecryptTMessage(const char *theBuf, unsigned long theLen, WONMsg::BaseMessage &theOutMsg, WONMisc::SocketPipe **thePipePP);
    bool DecryptNonTMessage(const char *theBuf, unsigned long theLen, WONMsg::BaseMessage &theOutMsg, WONMisc::SocketPipe **thePipePP);

//  void HandleGetNumUsersReply(WONMisc::SocketPipe *thePipeP, const WONMsg::MiniMessage& theMsgR);

    // Routing Server methods
    void HandleRoutingGroupChange(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR, int theServer);
    void HandleRoutingGroupChangeEx(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR, int theServer);
    void HandleRoutingGetClientListReply(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR, int theServer);
    void HandleRoutingRegisterClientReply(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR, int theServer);

    void HandleRoutingCreateDataObject(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR);
    void HandleRoutingDeleteDataObject(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR);

    void HandleRoutingPeerChat(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR, int theServer);
    void HandleRoutingPeerData(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR, int theServer);
    void HandleRoutingPeerData(WONMisc::SocketPipe* thePipeP, const WONCommon::RawBuffer &theData,WONMsg::ClientId theId, int theServer);
    void HandleRoutingPeerDataMultiple(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR, int theServer);

    void HandleRoutingReadDataObjectReply(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR);
    void HandleRoutingReplaceDataObject(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR);
    void HandleRoutingStatusReply(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR, int theServer);

    void HandleRoutingRegister(int theServer);
    void OnNewRoutingServerClient(WONMsg::ClientId theClientId, const std::wstring& theUserNameR, unsigned long theIPAddress, int theServer);

    // Event Server stuff
    unsigned long mEventTag;
    bool          mHasLobbyEnterEventBeenSent;
    time_t        mLobbyEnterTime;
    time_t        mGameStartTime;
    void RecordEvent(unsigned short theEventType);

    // Homeworld stuff
    void HandlePeerMsg(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR);

    void HandleJoinGame(WONMisc::SocketPipe* thePipeP, const TitanPacketMsg& theMsgR);
    void HandleJoinConfirm(WONMisc::SocketPipe* thePipeP, const TitanPacketMsg& theMsgR);
    void HandleLeaveGame(WONMisc::SocketPipe* thePipeP, const TitanPacketMsg& theMsgR);
    void HandleJoinReject(WONMisc::SocketPipe* thePipeP, const TitanPacketMsg& theMsgR);
    void HandleGameData(const TitanPacketMsg& theMsgR);
    void HandleGameStart(const TitanPacketMsg& theMsgR);
    void HandleGameMsg(const TitanPacketMsg& theMsgR);
    void HandleGameDisolved(const TitanPacketMsg& theMsgR);
    void HandleKickedOutOfGame(const TitanPacketMsg& theMsgR);
    void HandleUpdatePlayer(WONMisc::SocketPipe* thePipeP, const TitanPacketMsg& theMsgR);
    void HandleChannelList(WONMsg::DirEntityList& dirEntities);
    void HandleTitanServerList(WONMsg::DirEntityList& dirEntities);
    void HandlePing(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR);
    void HandlePingReply(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR);
    void HandleClientReconnect(WONMisc::SocketPipe* thePipeP, const WONMsg::MiniMessage& theMsgR);

    void HandleBeginStartGame(WONMisc::SocketPipe* thePipeP, const TitanPacketMsg& theMsgR);
    void HandleChangeAddress(WONMisc::SocketPipe* thePipeP, const TitanPacketMsg& theMsgR);
    void HandleRequestPackets(WONMisc::SocketPipe* thePipeP, const TitanPacketMsg& theMsgR);

    // Utility
    ClientNetAddr TitanInterface::MakeClientNetAddr(Address* theAddressP);
    void TitanInterface::InitDestAddress(Address* theAddressP, WONMisc::SocketPipe* thePipeP);
    void BuildAddress(SOCKADDR_IN& theAddrR, long theIP, unsigned short thePort);
    void BuildAddress(SOCKADDR_IN& theAddrR, const WONCommon::RawBuffer& theSixBytes);
    void BuildAddress(SOCKADDR_IN& theAddrR, unsigned char buffer[]);
    const char* PrintAddress(SOCKADDR_IN& theAddrR);
    const char* PrintAddress(const WONCommon::RawBuffer& theSixBytes);
    bool SendMsg(WONMisc::SocketPipe* thePipeP, const WONMsg::BaseMessage& theMsgR, unsigned char theLengthFieldSize = 0);
    unsigned long GetLengthFieldSize(const WONMsg::BaseMessage& theMsgR);

    unsigned long GetLocalIPAddress(void);

    bool EncryptAndSendRoutingMsg(const WONMsg::BaseMessage &theMsgR, int theServer);

    WONMisc::SocketPipe* ConnectTo(const SOCKADDR& theDest, WONMisc::EasySocket::SocketType theType=WONMisc::EasySocket::TCP, WONMisc::RecvLengthPrefixType thePrefixType=WONMisc::ptUnsignedLong);
    WONMisc::SocketPipe* ConnectTo(const Address& theDest, WONMisc::EasySocket::SocketType theType=WONMisc::EasySocket::TCP, WONMisc::RecvLengthPrefixType thePrefixType=WONMisc::ptUnsignedLong);
    WONMisc::SocketPipe* ConnectAndSend(const SOCKADDR& theDest, const WONMsg::BaseMessage& theMsgR, WONMisc::EasySocket::SocketType theType=WONMisc::EasySocket::TCP, WONMisc::RecvLengthPrefixType thePrefixType=WONMisc::ptUnsignedLong);
    WONMisc::SocketPipe* ConnectAndSend(const SOCKADDR_IN& theDest, const WONMsg::BaseMessage& theMsgR, WONMisc::EasySocket::SocketType theType=WONMisc::EasySocket::TCP, WONMisc::RecvLengthPrefixType thePrefixType=WONMisc::ptUnsignedLong);
    WONMisc::SocketPipe* ConnectAndSend(const Address& theDest, const WONMsg::BaseMessage& theMsgR, WONMisc::EasySocket::SocketType theType=WONMisc::EasySocket::TCP, WONMisc::RecvLengthPrefixType thePrefixType=WONMisc::ptUnsignedLong);

    static unsigned long GetHashSection(bool restart, unsigned char** theUnhashedBufP, unsigned char digest[MD5_HASH_SIZE]);
    static void TitanInterface::ShortCircuitChallengeResponse(unsigned char* theSeed, unsigned char* theChallengeResponseP);
    static bool TitanInterface::ReadFromWonstuff(bool restart, unsigned char* theBufferP);
};

#endif // _TitanInterface_H
