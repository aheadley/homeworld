/*============================================================================
 * TitanInterfaceC.stub.c
 * Dummy functions to simulate a WONnet connection that never works.
 *
 * Author:  Ted Cipicchio <ted@thereisnospork.com>
 * Created: Sat Oct 4 2003
 *==========================================================================*/
#include "TitanInterfaceC.h"

/*----------------------------------------------------------------------------
 * Global Variables
 *--------------------------------------------------------------------------*/

wchar_t ChannelPassword[MAX_PASSWORD_LEN];
TPChannelList tpChannelList;
TPServerList tpServerList;
CaptainGameInfo tpGameCreated;
Address myAddress;
unsigned long DIRSERVER_PORTS[MAX_PORTS];
unsigned long PATCHSERVER_PORTS[MAX_PORTS];
ipString DIRSERVER_IPSTRINGS[MAX_IPS];
ipString PATCHSERVER_IPSTRINGS[MAX_IPS];
unsigned long HomeworldCRC[4];
wchar_t GameWereInterestedIn[MAX_TITAN_GAME_NAME_LEN];
void *GameWereInterestedInMutex = 0;


/*----------------------------------------------------------------------------
 * Functions
 *--------------------------------------------------------------------------*/

void titanGotNumUsersInRoomCB(const wchar_t *theRoomName, int theNumUsers)
{ }


unsigned long titanStart(unsigned long isLan, unsigned long isIP)
{ return 0; }


unsigned long titanCheckCanNetwork(unsigned long isLan, unsigned long isIP)
{ return 0; }


// --MikeN
// Call this method to begin shutdown of titan.  Parameters specify packet to send
// to connected client(s) (a shutdown message).  The callback titanNoClientsCB() will
// be invoked when complete.
void titanStartShutdown(unsigned long titanMsgType, const void* thePacket,
                        unsigned short theLen)
{
	/* Probably won't ever be called, but we'll be consistent anyway... */
	titanNoClientsCB();
}


void titanLeaveGameNotify(void)
{ }


void titanShutdown(void)
{ }


void titanRefreshRequest(char* theDir)
{ }


unsigned long titanReadyToStartGame(unsigned char *routingaddress)
{ return 0; }


unsigned long titanBehindFirewall(void)
{ return 0; }


void titanCreateGame(wchar_t *str, DirectoryCustomInfo* myInfo)
{ }


void titanRemoveGame(wchar_t *str)
{ }


void titanCreateDirectory(char *str, char* desc)
{ }


void titanSendLanBroadcast(const void* thePacket, unsigned short theLen)
{ }


void titanSendPacketTo(Address *address, unsigned char titanMsgType,
                       const void* thePacket, unsigned short theLen)
{ }


void titanBroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen)
{ }


void titanAnyoneSendPacketTo(Address *address, unsigned char titanMsgType,
                       const void* thePacket, unsigned short theLen)
{ }


void titanAnyoneBroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen)
{ }


void titanConnectToClient(Address *address)
{ }


int titanStartChatServer(wchar_t *password)
{ return 0; }


void titanSendPing(Address *address,unsigned int pingsizebytes)
{ }


void titanPumpEngine()
{ }


void titanSetGameKey(unsigned char *key)
{ }


const unsigned char *titanGetGameKey(void)
{ return 0; }


Address titanGetMyPingAddress(void)
{ return myAddress; }


int titanGetPatch(char *filename,char *saveFileName)
{
	titanGetPatchFailedCB(PATCHFAIL_UNABLE_TO_CONNECT);
	return PATCHFAIL_UNABLE_TO_CONNECT;
}


void titanReplaceGameInfo(wchar_t *str, DirectoryCustomInfo* myInfo, unsigned long replaceTimeout)
{ }


void chatConnect(wchar_t *password)
{ }


void chatClose(void)
{ }


void BroadcastChatMessage(unsigned short size, const void* chatData)
{ }


void SendPrivateChatMessage(unsigned long* userIDList, unsigned short numUsersInList,
                            unsigned short size, const void* chatData)
{ }


void authAuthenticate(char *loginName, char *password)
{ }


void authCreateUser(char *loginName, char *password)
{ }


void authChangePassword(char *loginName, char *oldpassword, char *newpassword)
{ }


int titanSaveWonstuff()
{ return 0; }


void titanWaitShutdown(void)
{ }


void titanConnectingCancelHit(void)
{ }
