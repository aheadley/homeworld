
#ifndef ___TITAN_INTERFACEC_H
#define ___TITAN_INTERFACEC_H

#include <wchar.h>

// Homeworld Message Types
#define TITANMSGTYPE_JOINGAMEREQUEST   0x10
#define TITANMSGTYPE_JOINGAMECONFIRM   0x11
#define TITANMSGTYPE_JOINGAMEREJECT    0x12
#define TITANMSGTYPE_UPDATEGAMEDATA    0x20
#define TITANMSGTYPE_LEAVEGAMEREQUEST  0x30
#define TITANMSGTYPE_GAMEISSTARTING    0x40
#define TITANMSGTYPE_PING              0x50
#define TITANMSGTYPE_PINGREPLY         0x51
#define TITANMSGTYPE_GAME              0x60
#define TITANMSGTYPE_GAMEDISOLVED      0x70
#define TITANMSGTYPE_UPDATEPLAYER      0x80
#define TITANMSGTYPE_BEGINSTARTGAME    0x90
#define TITANMSGTYPE_CHANGEADDRESS     0xa0
#define TITANMSGTYPE_REQUESTPACKETS    0xb0
#define TITANMSGTYPE_RECONNECT         0xc0
#define TITANMSGTYPE_KICKPLAYER        0xd0

#define MAX_VERSION_STRING_LEN      64
#define MAX_NETWORKVERSION_STRING_LEN   16

#define MAX_CHANNEL_NAME_LEN        32
#define MAX_CHANNEL_DESCRIPTION_LEN 32
#define MAX_SERVER_NAME_LEN         32
#define MAX_SERVER_DESCRIPTION_LEN  32

#define MAX_TITAN_GAME_NAME_LEN     32
#define MAX_PASSWORD_LEN            20
#define MAX_MAPNAME_LEN             32
#define MAX_PERSONAL_NAME_LEN       18      // if change change in globals.h to match
#define TP_ScenarioListLength       200     // fix later
#define TP_ChannelListLength        200     // fix later
#define TP_ServerListLength         200     // fix later
#define MAX_LAN_ADVERT_LEN          1500

#define GAME_IN_PROGRESS            0x01

typedef struct Address
{
    union {
        unsigned long IP;
        unsigned char etherAddr[6];
    } AddrPart;
    unsigned short Port;
} Address;

#define CreateInternetAddress(addr,ip,port)     \
    addr.AddrPart.IP = ip;                      \
    addr.Port = port;

#define CreateEthernetAddress(addr,etheraddr,port)   \
    addr.AddrPart.etherAddr = etheraddr;        \
    addr.Port = port;

#define GetIPFromInternetAddress(addr) ((addr).AddrPart.IP)
#define GetPortFromInternetAddress(addr) ((addr).Port)

#define InternetAddressesAreEqual(addr1,addr2) ((addr1).AddrPart.IP == (addr2).AddrPart.IP)
#define InternetAddressesAndPortsAreEqual(addr1,addr2) (((addr1).AddrPart.IP == (addr2).AddrPart.IP) && ((addr1).Port == (addr2).Port))
#define EthernetAddressesAreEqual(addr1,addr2) ( (*((unsigned long *)&((addr1).AddrPart.etherAddr[0])) == *((unsigned long *)&((addr2).AddrPart.etherAddr[0]))) && ((addr1).AddrPart.etherAddr[4] == (addr2).AddrPart.etherAddr[4]) && ((addr1).AddrPart.etherAddr[5] == (addr2).AddrPart.etherAddr[5]) )
#define EthernetAddressesAndPortsAreEqual(addr1,addr2) (EthernetAddressesAreEqual(addr1,addr2) && ((addr1).Port == (addr2).Port))

#define AddressesAreEqual(addr1,addr2) ((!IPGame) ? (EthernetAddressesAreEqual((addr1),(addr2))) : (InternetAddressesAreEqual((addr1),(addr2))))
#define AddressesAndPortsAreEqual(addr1,addr2) ((!IPGame) ? (EthernetAddressesAndPortsAreEqual((addr1),(addr2))) : (InternetAddressesAndPortsAreEqual((addr1),(addr2))))

#define GAMEKEY_SIZE 8

void PrintAddressToString(char *buffer,Address *address);

typedef struct
{
    Address routingAddress; // Address of routing mechanism

    Address oldCaptainAddress; // Old address of captain
    Address newCaptainAddress; // New address of captain
} BeginStartGamePacket;


typedef struct
{
    Address oldAddress;
    Address newAddress;
} ChangeAddressPacket;

typedef struct
{
    unsigned long firstPacket;
    unsigned long lastPacket;
} RequestPacketsPacket;

typedef struct
{
    Address captainAddress;
    Address pingAddress;
    char versionInfo[MAX_NETWORKVERSION_STRING_LEN];
    char userBehindFirewall;
    unsigned char pad[3];
    unsigned char sessionKey[GAMEKEY_SIZE];     // filled in by TitanInterface.cpp when we create a internet game
    unsigned short flag;
    unsigned char numPlayers;
    unsigned char stringdatalength;     // contains Password and Mapnam
    wchar_t stringdata[1];
} DirectoryCustomInfo;

typedef struct
{
    Address captainAddress;
    Address pingAddress;
    char versionInfo[MAX_NETWORKVERSION_STRING_LEN];
    char userBehindFirewall;
    unsigned char pad[3];
    unsigned char sessionKey[GAMEKEY_SIZE];     // filled in by TitanInterface.cpp when we create a internet game
    unsigned short flag;
    unsigned char numPlayers;
    unsigned char stringdatalength;
    wchar_t stringdata[MAX_PASSWORD_LEN+MAX_MAPNAME_LEN];
} DirectoryCustomInfoMax;

typedef struct
{
    Address address;
    unsigned short playerIndex;
    unsigned long  baseColor;
    unsigned long  stripeColor;
    unsigned long  colorsPicked;
    unsigned short race;
    char behindFirewall;
    unsigned char pad[3];
    char PersonalName[MAX_PERSONAL_NAME_LEN];
} PlayerInfo;

typedef struct
{
    unsigned long  baseColor;
    unsigned long  stripeColor;
    unsigned long  colorsPicked;
    unsigned short race;
    char behindFirewall;
    unsigned char pad[3];
    char PersonalName[MAX_PERSONAL_NAME_LEN];
} PlayerJoinInfo;

typedef struct CaptainGameInfo
{
    wchar_t Name[MAX_TITAN_GAME_NAME_LEN];

    wchar_t Password[MAX_PASSWORD_LEN];
    char MapName[MAX_MAPNAME_LEN];
    char DisplayMapName[MAX_MAPNAME_LEN];

    char userBehindFirewall;
    unsigned char pad[3];

    unsigned char numComputers;
    unsigned char startingFleet;
    unsigned char bountySize;
    unsigned char startingResources;

    unsigned long resourceInjectionInterval;
    unsigned long resourceInjectionsAmount;

    unsigned long resourceLumpSumTime;
    unsigned long resourceLumpSumAmount;

    unsigned char aiplayerDifficultyLevel;
    unsigned char aiplayerBigotry;
    unsigned short pad2;

    unsigned short flag;

    unsigned short numPlayers;

    PlayerInfo playerInfo[8];        // MAX_MULTIPLAYER_PLAYERS

    // all new options here
} CaptainGameInfo;

typedef struct
{
    wchar_t Name[MAX_TITAN_GAME_NAME_LEN];
    DirectoryCustomInfoMax directoryCustomInfo;
}
tpscenario;

// flags for roomflags of tpchannel
#define ROOMFLAGS_LANGUAGE_MASK         0x0ff
#define ROOMFLAGS_PASSWORDPROTECTED     0x100
#define ROOMFLAGS_ADEFAULTROOM          0x200

typedef struct
{
    wchar_t ChannelName[MAX_CHANNEL_NAME_LEN];
    wchar_t ChannelDescription[MAX_CHANNEL_DESCRIPTION_LEN];
    unsigned int roomflags;
    unsigned int roomfullness;
    unsigned char addressstore[6];
}
tpchannel;

typedef struct TPChannelList
{
    unsigned long numChannels;
    tpchannel tpChannels[TP_ChannelListLength];
    void *mutex;
    unsigned long newDataArrived;
} TPChannelList;

typedef struct
{
    wchar_t ServerName[MAX_SERVER_NAME_LEN];
    wchar_t ServerDescription[MAX_SERVER_DESCRIPTION_LEN];
    unsigned int flags;
    unsigned int reliability;
    unsigned int ping;
    unsigned char addressstore[6];
}
tpserver;

typedef struct TPServerList
{
    unsigned long numServers;
    tpserver tpServers[TP_ServerListLength];
    void *mutex;
    unsigned newDataArrived;
} TPServerList;

#define sizeofDirectoryCustomInfo(n) (sizeof(DirectoryCustomInfo) + (n).stringdatalength - 1)

void SetChannel(wchar_t *channel,wchar_t *description);
void ResetChannel(void);
wchar_t *GetCurrentChannel(void);
wchar_t *GetCurrentChannelDescription(void);

extern int ChannelProtectedFlag;
extern wchar_t ChannelPassword[MAX_PASSWORD_LEN];

void titanGameMsgReceivedCB(const void *blob,unsigned short bloblen);
//void titanQueryRoutingServers(void);
void titanGotNumUsersInRoomCB(const wchar_t *theRoomName, int theNumUsers);

unsigned long titanStart(unsigned long isLan, unsigned long isIP); // returns 1 if succesful, 0 if failed
unsigned long titanCheckCanNetwork(unsigned long isLan, unsigned long isIP); // returns 1 if can network, 0 if can't.  Assumes titanInterface not initialized

// --MikeN
// Call this method to begin shutdown of titan.  Parameters specify packet to send
// to connected client(s) (a shutdown message).  The callback titanNoClientsCB() will
// be invoked when complete.
void titanStartShutdown(unsigned long titanMsgType, const void* thePacket,
                        unsigned short theLen);

void titanLeaveGameNotify(void);

void titanShutdown(void);
void titanRefreshRequest(char* theDir);

unsigned long titanReadyToStartGame(unsigned char *routingaddress);
unsigned long titanBehindFirewall(void);

void mgGameStartReceivedCB(const void *blob,unsigned short bloblength);

void titanCreateGame(wchar_t *str, DirectoryCustomInfo* myInfo);
void titanRemoveGame(wchar_t *str);

void titanCreateDirectory(char *str, char* desc);

void titanSendLanBroadcast(const void* thePacket, unsigned short theLen);
void titanReceivedLanBroadcastCB(const void* thePacket, unsigned short theLen);

void titanSendPacketTo(Address *address, unsigned char titanMsgType,
                       const void* thePacket, unsigned short theLen);

void titanBroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen);

void titanAnyoneSendPacketTo(Address *address, unsigned char titanMsgType,
                       const void* thePacket, unsigned short theLen);

void titanAnyoneBroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen);

void titanConnectToClient(Address *address);

int titanStartChatServer(wchar_t *password);

void titanSendPing(Address *address,unsigned int pingsizebytes);
void titanSendPointMessage(int playerIndex,unsigned char *packet,unsigned long sizeofPacket);
void titanSendBroadcastMessage(unsigned char *packet,unsigned long sizeofPacket);
void titanAnyoneSendPointMessage(int playerIndex,unsigned char *packet,unsigned long sizeofPacket);
void titanAnyoneSendBroadcastMessage(unsigned char *packet,unsigned long sizeofPacket);
void titanPumpEngine();

void titanSetGameKey(unsigned char *key);
const unsigned char *titanGetGameKey(void);

// callback for when titan gets valid version strings (one string, multiple version strings seperated by tabs)
void titanGotValidVersionStrings(char *validversions);
void titanResetValidVersions();     // clears ValidVersions string

Address titanGetMyPingAddress(void);

//--MikeN
// This method is invoked when the last client connection is closed.  Note that this
// may happen at times other than shutdown.
void titanNoClientsCB(void);

unsigned long titanConfirmReceivedCB(Address *address,const void *blob,unsigned short bloblen);
unsigned long titanRejectReceivedCB(Address *address,const void *blob,unsigned short bloblen);
void titanStartChatReplyReceivedCB(short theStatus);
void titanUpdateGameDataCB(const void *blob,unsigned short bloblen);
void titanPingReplyReceivedCB(Address *address,unsigned long theLag);
unsigned long titanLeaveGameReceivedCB(Address *address,const void *blob,unsigned short bloblen);
void titanLeaveGame(int shutdown);

#define REQUEST_RECV_CB_DENYANDCLOSE    -1
#define REQUEST_RECV_CB_JUSTDENY        0
#define REQUEST_RECV_CB_ACCEPT          1
signed long titanRequestReceivedCB(Address *address,const void *blob,unsigned short bloblen);

void tpLockChannelList();
void tpUnLockChannelList();
void tpLockServerList();
void tpUnLockServerList();

//void titanGetMyAddress(Address *address);

int titanGetPatch(char *filename,char *saveFileName);

void titanPatchProgressCB(int lenSoFar,int totalLen);
void titanGetPatchCompleteCB(void);
void titanGetPatchFailedCB(int patchFailErrorMsg);
#define PATCHFAIL_UNABLE_TO_CONNECT             1
#define PATCHFAIL_ERROR_SENDING_REQUEST         2
#define PATCHFAIL_ERROR_RECEIVING_HTTP_HEADER   3
#define PATCHFAIL_INVALID_FILE_LENGTH           4
#define PATCHFAIL_UNABLE_TO_CREATE_FILE         5
#define PATCHFAIL_UNABLE_TO_WRITE_FILE          6
#define PATCHFAIL_ERROR_RECEIVING_PATCH         7
#define PATCHFAIL_UNABLE_TO_START_DOWNLOAD_THREAD   8
#define PATCHFAIL_USERABORT                     9
#define PATCHFAIL_INVALID_STATUS_REPLY          10

extern int patchAbortRequest;

void titanReplaceGameInfo(wchar_t *str, DirectoryCustomInfo* myInfo, unsigned long replaceTimeout);

void titanUpdatePlayerCB(Address *address, const void *blob, unsigned short bloblen);

// Callbacks for chatting, user joined user left, etc..

void chatConnect(wchar_t *password);
void chatClose(void);

void BroadcastChatMessage(unsigned short size, const void* chatData);
void SendPrivateChatMessage(unsigned long* userIDList, unsigned short numUsersInList,
                            unsigned short size, const void* chatData);

void chatReceiveUserJoinReply(short status, unsigned long userID);
void chatReceiveUsersHere(const char *name, unsigned long userID);
void chatReceiveUsersJoined(const char *name, unsigned long userID);
void chatReceiveMessage(unsigned long originUserID, signed long whisper,unsigned long type, unsigned long size, const void* chatData);
void chatReceiveUserLeft(unsigned long userID);

int authReceiveReply(short status);

void authAuthenticate(char *loginName, char *password);
void authCreateUser(char *loginName, char *password);
void authChangePassword(char *loginName, char *oldpassword, char *newpassword);

void mgNotifyAuthRequestFailed(void);

void mgNotifyGameDisolved(void);

void mgNotifyKickedOut(void);

// Functions to call from the titan code for the connection state machine
void cNotifyChatInfo(void);

void cNotifyDirStatus(short theStatus);
void mgNotifyDirRequestFailed();

void cNotifyGameCreationStatus(short theStatus);

void cNotifyChatConnectFailed(void);

void cNotifyOffline(const signed long theStatus);

void cNotifyCurrentRoomPresent(int present);
void cNotifyRoomListPresent(void);

extern TPChannelList tpChannelList;
extern TPServerList tpServerList;
extern CaptainGameInfo tpGameCreated;

void mgGameListGameAdded(tpscenario *thegame);
void mgGameListGameRemoved(tpscenario *thegame);
void mgGameListGameChanged(tpscenario *thegame);
void mgGameListNew(void);

int titanLoadPublicKey(char *filename,unsigned char **buffer);

int titanSaveWonstuff();

extern Address myAddress;

extern unsigned long TitanActive;

extern int LANGame;
extern int IPGame;

#define MAX_PORTS      10
#define MAX_IPS        (MAX_PORTS)

#define MAXIP_STRLEN        50

extern char PATCHNAME[];

extern unsigned long DIRSERVER_NUM;
extern unsigned long PATCHSERVER_NUM;

extern unsigned long DIRSERVER_PORTS[MAX_PORTS];
extern unsigned long PATCHSERVER_PORTS[MAX_PORTS];

typedef char ipString[MAXIP_STRLEN];

extern ipString DIRSERVER_IPSTRINGS[MAX_IPS];
extern ipString PATCHSERVER_IPSTRINGS[MAX_IPS];

extern unsigned long HomeworldCRC[4];

#define FIREWALL_NOTBEHIND      0
#define FIREWALL_BEHIND         1
#define FIREWALL_AUTODETECT     2
extern unsigned long firewallButton;

extern long CONNECT_TIMEOUT;

extern int sigsNumPlayers;
extern int sigsPlayerIndex;

extern int sigsPressedStartGame;

// is person waiting for a game ?
extern int            WaitingForGame;
// did the person create the game
extern int            GameCreator;

extern signed int captainIndex;
#define IAmCaptain  (sigsPressedStartGame ? (sigsPlayerIndex == captainIndex) : (GameCreator))
#define CaptainNotDefined (captainIndex == -1)

extern wchar_t GameWereInterestedIn[MAX_TITAN_GAME_NAME_LEN];
extern void *GameWereInterestedInMutex;

void mgGameInterestedIn(wchar_t *interested);
void mgGameInterestedOff();

extern unsigned long TitanReadyToShutdown;
extern unsigned long WAIT_SHUTDOWN_MS;
void titanWaitShutdown(void);

void titanConnectingCancelHit(void);

#endif

