/*=============================================================================
    Name    : CommandNetwork.h
    Purpose : Definitions for CommandNetwork.c

    Created 7/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___COMMAND_NETWORK_H
#define ___COMMAND_NETWORK_H

#include "Switches.h"
#include "Types.h"
#include "Globals.h"
#include "CommandLayer.h"
#include "Chatting.h"



/*=============================================================================

Data format for sending command packets (individual commands to captain server)
-------------------------------------------------------------------------------

Each command packet will contain:

Type of Packet      -   uword
Frame number        -   uword
#if SYNC_CHECK
Checksums field     -   type NetSyncChecksums
#endif
Number of commands  -   uword
Commands

where each Command is:

CommandType     -   uword
CommandBody     -   contents vary

where the CommandBody is determined by the CommandType.

For Move,

vector from
vector to
NetSelection        (variable)

For Attack,

NetAttackSelection  (variable)
NetSelection        (variable)

For Special,

NetTargetsSelection (variable)
NetSelection        (variable)

For Formation,

TypeOfFormation -   uword
NetSelection        (variable)

For Dock

dockwith            ShipID
dockType            uword
NetSelection        (variable)

For Launch Multiple
launchfrom          ShipID
NetSelection        (variable)

For Misc
miscData            udword
miscCommand         uword
NetSelection        (variable)

For Autolaunch
OnOff               uword
PlayerIndex         uword

For CreateShip

ShipType            uword
ShipRace            uword
PlayerIndex         uword
CreatorID           ShipID

For BuildShip

ShipType            uword
ShipRace            uword
PlayerIndex         uword
CreatorID           ShipID

For CollectResource
resource            ResourceID
NetSelection        (variable)

Data format for sending sync packets (packets containing commands to execute, from captain server).
---------------------------------------------------------------------------------------------------

Each sync packet will contain:

Type of packet          -   uword
Frame number            -   uword
#if SYNC_CHECK
Checksums field         -   type NetSyncChecksums
#endif
Number of Commands      -   uword
Commands

where Commands is a bunch of commands, one after the other, as defined above.

=============================================================================*/



/*=============================================================================
    Defines:
=============================================================================*/

#define CAPTAINSERVER_PERIOD            1.0f/8.0f


#define COMMANDTYPE_MOVE                1
#define COMMANDTYPE_ATTACK              2
#define COMMANDTYPE_FORMATION           3
#define COMMANDTYPE_DOCK                4
#define COMMANDTYPE_CREATESHIP          5
#define COMMANDTYPE_COLLECTRESOURCE     6
#define COMMANDTYPE_PROTECT             7
#define COMMANDTYPE_BUILDSHIP           8
#define COMMANDTYPE_SPECIAL             9
#define COMMANDTYPE_MISC                10
#define COMMANDTYPE_AUTOLAUNCH          11
#define COMMANDTYPE_LAUNCHMULTIPLE      12
#define COMMANDTYPE_ALLIANCEINFO        13
#define COMMANDTYPE_RUTRANSFER          14
#define COMMANDTYPE_MP_HYPERSPACE       15

#ifdef GOD_LIKE_SYNC_CHECKING
    #define COMMANDTYPE_GODSYNC         16

//defines for command
    #define GOD_COMMAND_GOD_HALTONSYNC  1
    #define GOD_COMMAND_HALTONSYNC      2
#endif

#define COMMANDTYPE_PLAYERDROPPED       17

#define COMMANDTYPE_DETERMINISTICBUILD  18

#define COMMANDTYPE_RESEARCHINFO        19
    #define RESEARCH_SUBCOMMAND_START   0
    #define RESEARCH_SUBCOMMAND_STOP    1

#define MISCCOMMAND_HALT                0
#define MISCCOMMAND_SCUTTLE             1
#define MISCCOMMAND_TACTICS             2
#define MISCCOMMAND_KAMIKAZE            3
#define MISCCOMMAND_PARADE              4
#define PACKETTYPE_COMMAND              0xcccc
#define PACKETTYPE_SYNC                 0x5555
#define PACKETTYPE_REQUESTEDSYNC        0xe555
#define PACKETTYPE_NONCAPTAINREADY      0xaaaa

//#define PACKETTYPE_INFOFROMCAPTAIN      0x8888
//#define PACKETTYPE_NONCAPTAININFO       0x9999

#define PACKETTYPE_HORSERACE            0x0911

#define PACKETTYPE_CHAT                 0xb1ab      //packet type 'blab'er

#define PACKETTYPE_TRANSFERCAPTAINCY    0xca00

#define PACKETTYPE_DROPPINGOUTOFLOAD    0x10ad

#define PACKETTYPE_REQUESTSYNCPKTS      0xe155  //for explicitly requesting sync pkts (due to temporary loss of connection)

#define PACKETTYPE_IAMALIVE             0xa1a1
#define PACKETTYPE_ALIVESTATUS          0xa155

#define PACKETTYPE_FILE                 0xf11e

#define PACKETTYPE_INGAMEQUITING        0x15ed

#define PACKETTYPE_LAGCHECK             0x1A5C

#define PACKETTYPE_CHEATDETECT          0xcea1

// subtypes for PACKETTYPE_TRANSFERCAPTAINCY
#define XFERCAP_GIVEUP_CAPTAINCY_NOTICE     0
#define XFERCAP_CAPTAIN_PROPOSAL            1
#define XFERCAP_PAUSE_TRANSFERRING_CAPTAIN  2
#define XFERCAP_PAUSE_ACK                   3
#define XFERCAP_PAUSE_YOU_BE_CAPTAIN        4
#define XFERCAP_I_AM_NEW_CAPTAIN            5
#define XFERCAP_NUM                         6

#define XFERCAP_SUBTYPE_MASK                127
#define XFERCAP_CUSTOMINFO_PRESENT          128

/*=============================================================================
    Types:
=============================================================================*/

#if SYNC_CHECK
typedef struct
{
    uword packetnum;    // packetnum this checksum is for   (should this be a udword? fix later)
    uword randcheck;    // random # generated, but if == 0, indicates all checksums should be ignored
    real32 univcheck;   // universe checksum
    udword blobcheck;   // blob checksum
} NetSyncChecksums;
#endif

typedef struct CaptaincyCustomInfo
{
    udword custom[MAX_MULTIPLAYER_PLAYERS];
} CaptaincyCustomInfo;

typedef struct
{
    uword type;
    uword from;
    udword frame;
#if SYNC_CHECK
    NetSyncChecksums checksums;
#endif
    uword numberOfCommands;
    // Commands go here
} HWPacketHeader;

typedef struct
{
    HWPacketHeader packetheader;
    uword playerindex;
    uword barnum;
    real32 percent;
} HorsePacket;

//if we change chatpacket we need to change the way the size of the packet is determined in
//chatting.c
typedef struct ChatPacket
{
    HWPacketHeader packetheader;
    //chat text string goes here...we'll allow modular text strings, capping
    //will be performed in text retrieval
    uword  bounced;
    uword  messageType;
    uword  users;
    udword data;
    char   message[MAX_CHAT_MESSAGE_LENGTH*sizeof(wchar_t)];
} ChatPacket;

typedef struct TransferCaptaincyPacketSmall
{
    HWPacketHeader packetheader;
    uword subtype;
} TransferCaptaincyPacketSmall;
typedef struct TransferCaptaincyPacket      // same as TransferCaptaincyPacketSmall except with custominfo
{
    HWPacketHeader packetheader;
    uword subtype;
    CaptaincyCustomInfo custominfo;         // optional, only here if XFERCAP_CUSTOMINFO_PRESENT set in subtype
} TransferCaptaincyPacket;

typedef struct RequestSyncPacketsPacket
{
    HWPacketHeader packetheader;
    udword topacketnum;                     // frompacketnum is stored in frame of packetheader
} RequestSyncPacketsPacket;

typedef struct AliveStatusPacket
{
    HWPacketHeader packetheader;
    ubyte alivestatus[MAX_MULTIPLAYER_PLAYERS];
} AliveStatusPacket;

typedef struct FilePacket
{
    HWPacketHeader packetheader;            // frame contains filesize, numberOfCommands contains filename size
    uword totalNumFiles;                    // total number of files that will eventually be sent
    char filename[1];                       // variable size, null terminated string
    //char filecontents[1];                 // variable size, data, commented out since we don't know its
                                            // position in the structure.  Use filecontentsAddress to find out.
} FilePacket;

typedef struct LagPacket
{
    HWPacketHeader packetheader;
    real32         timestamp;
} LagPacket;

#define filecontentsAddress(pkt,fns) (((ubyte *)&(pkt)->filename[0]) + (fns))

#define sizeofFilePacket(n,d) (sizeof(FilePacket) + ((n)-1) + ((d)))
#define sizeofFilePacketGivenPacket(pkt) (sizeof(FilePacket) + ((pkt)->packetheader.frame-1) + ((pkt)->packetheader.numberOfCommands))

typedef struct
{
    uword commandType;
    // Command body goes here
} HWCommandHeader;

typedef struct
{
    uword numShips;
    ShipID ShipID[1];   // variable number of ShipID's
} NetSelection;

typedef struct
{
    uword numTargets;
    TargetID TargetID[1];   // variable number of TargetID's
} NetAttackSelection;

typedef NetSelection NetProtectSelection;

typedef NetAttackSelection NetTargetsSelection;

typedef struct
{
    vector from;
    vector to;
    NetSelection selection;
} NetMoveCommand;

typedef struct
{
    NetAttackSelection attack;
    NetSelection selection;
} NetAttackCommand;

typedef struct
{
    NetTargetsSelection targets;
    NetSelection selection;
} NetSpecialCommand;

typedef struct
{
    uword typeOfFormation;
    NetSelection selection;
} NetFormationCommand;

typedef struct
{
    ShipID dockwithID;
    uword dockType;
    NetSelection selection;
} NetDockCommand;

typedef struct
{
    ShipID launchfromID;
    NetSelection selection;
} NetLaunchMultipleCommand;

typedef struct
{
    udword miscData;
    uword miscCommand;
    NetSelection selection;
} NetMiscCommand;


#ifdef GOD_LIKE_SYNC_CHECKING
typedef struct
{
    udword univframe;   // univupdateframe for this checksum.
    uword  randcheck;   // random # generated, but if == 0, indicates all checksums should be ignored
    uword  pad;
    real32 univcheck;   // universe checksum
    udword blobcheck;   // blob checksum
    udword shipcheck;   // ship super checksum
} GodSyncCheckSums;

typedef struct
{
    uword type;
    uword from;
    udword frame;
    GodSyncCheckSums godChecksum;
}GodSyncCommand;

#endif

typedef struct
{
    ubyte to;
    ubyte from;
    ubyte flags;
    sdword resourceUnits;
} NetRUTransferferCommand;

typedef struct
{
    uword OnOff;
    uword playerIndex;
} NetAutolaunchCommand;

typedef struct
{
    udword AllianceMsgType;
    uword  CurAlliance;
    uword  NewAlliance;
} NetAllianceCommand;

typedef struct
{
    udword playerDroppedMask;
    udword verify;
} NetPlayerDroppedCommand;

typedef struct
{
    uword shipType;
    uword shipRace;
    uword playerIndex;
    ShipID creatorID;
} NetCreateShipCommand;

typedef struct
{
    ResourceID resourceID;
    NetSelection selection;
} NetCollectResourceCommand;

typedef struct
{
    NetProtectSelection protect;
    NetSelection selection;
} NetProtectCommand;

typedef enum
{
    NO_PACKET,
    PACKET_READY,
    TOO_MANY_PACKETS
} WaitPacketStatus;

typedef struct
{
    ubyte subCommand;
    uword numShips;
    uword shipType;
    uword shipRace;
    uword playerIndex;
    ShipID creatorID;
} NetDeterministicBuildCommand;

typedef struct
{
    ubyte  ResearchMsgType;
    ubyte  playerindex;
    ubyte  labnum;
    ubyte  tech;
} NetResearchCommand;


/*=============================================================================
    Macros:
=============================================================================*/

#define sizeofNetSelection(n) (sizeof(NetSelection) + sizeof(ShipID)*((n)-1))
#define sizeofNetProtectSelection sizeofNetSelection

#define sizeofNetAttackSelection(n) (sizeof(NetAttackSelection) + sizeof(TargetID)*((n)-1))
#define sizeofNetTargetsSelection sizeofNetAttackSelection

#define sizeofNetMoveCommand(n) (sizeof(NetMoveCommand) + sizeof(ShipID)*((n)-1))
#define sizeofNetAttackCommand(a,s) (sizeof(NetAttackCommand) + sizeof(TargetID)*((a)-1) + sizeof(ShipID)*((s)-1))
#define sizeofNetSpecialCommand(t,s) (sizeof(NetSpecialCommand) + sizeof(TargetID)*((t)-1) + sizeof(ShipID)*((s)-1))
#define sizeofNetFormationCommand(n) (sizeof(NetFormationCommand) + sizeof(ShipID)*((n)-1))
#define sizeofNetDockCommand(n) (sizeof(NetDockCommand) + sizeof(ShipID)*((n)-1))
#define sizeofNetAutolaunchCommand (sizeof(NetAutolaunchCommand))
#define sizeofNetAllianceCommand (sizeof(NetAllianceCommand))
#define sizeofNetPlayerDroppedCommand (sizeof(NetPlayerDroppedCommand))
#define sizeofNetCreateShipCommand (sizeof(NetCreateShipCommand))
#define sizeofNetCollectResourceCommand(n) (sizeof(NetCollectResourceCommand) + sizeof(ShipID)*((n)-1))
#define sizeofNetProtectCommand(p,s) (sizeof(NetProtectCommand) + sizeof(ShipID)*((p)-1) + sizeof(ShipID)*((s)-1))
#define sizeofNetMiscCommand(n) (sizeof(NetMiscCommand) + sizeof(ShipID)*((n)-1))
#define sizeofNetLaunchMultipleCommand(n) (sizeof(NetLaunchMultipleCommand) + sizeof(ShipID)*((n)-1))
#define sizeofNetDeterministicBuildCommand (sizeof(NetDeterministicBuildCommand))
#define sizeofNetResearchCommand (sizeof(NetResearchCommand))

/*=============================================================================
    Externs:
=============================================================================*/

//  For printing of the currently dropped out players;
extern udword numPlayerDropped;
extern udword playersDropped[MAX_MULTIPLAYER_PLAYERS];
extern real32 printTimeout;

/*=============================================================================
    Functions:
=============================================================================*/

void CommandNetworkReset(void);

void SendTransferCaptaincyPacket(sdword playerIndex,uword subtype,udword misc,CaptaincyCustomInfo *custominfo);
void SendRequestSyncPkts(udword frompacketnum,udword topacketnum);

void clSendMove(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
void clSendAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clSendFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation);
void clSendDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith);
void clSendCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);
void clSendBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);

void clSendDeterministicBuild(udword command, CommandLayer* comlayer, sdword numShips, ShipType shipType, ShipRace shipRace, uword playerIndex, ShipPtr creator);

void clSendCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource);
void clSendProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom);
void clSendSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targetcom);
void clSendMisc(CommandLayer *comlayer,SelectCommand *selectcom,uword miscCommand,udword miscData);
void clSendAutoLaunch(udword OnOff,udword playerIndex);
void clSendSetAlliance(udword AllianceType, uword curalliance, uword newalliance);
void clSendLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom);
void clProcessSyncPacket(CommandLayer *comlayer,ubyte *packet,udword sizeofPacket);
void clSendRuTransfer(CommandLayer *comlayer,sdword fromIndex,sdword toIndex, sdword resourceUnits,ubyte flags);
void clSendMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
#ifdef GOD_LIKE_SYNC_CHECKING
void clSendGodSync(GodSyncCheckSums *checksumStruct,sdword playIndex,udword type);
#endif
WaitPacketStatus clWaitSyncPacket(CommandLayer *comlayer);
void captainServerTask(void);
void ReceivedPacketCB(ubyte *packet,udword sizeofPacket);
void netCheck(void);

void SendNonCaptainReadyPacket(udword playerIndex);
void SendNonCaptainInfoPacket(udword playerIndex);
void SendDroppingOutOfLoad(udword playerIndex);
void SendInGameQuittingPacket(udword playerIndex);

void SendLagPacket(udword to, ubyte *packet);
void SendCheatDetect(void);

void clSendResearch(udword type, udword playernum, udword labnum, udword tech);

//prototype for horse race communication
void SendHorseRacePacket(ubyte *packet,udword sizeofpacket);

//prototype for ingame chat packets
void SendChatPacketPacket(ChatPacket *packet, udword sizeofpacket,udword users);

void InitLastSyncPktsQ(void);
void CloseLastSyncPktsQ(void);
void ResetLastSyncPktsQ(void);
bool GetSyncPktFromLastSyncPktsQ(udword frame,HWPacketHeader **packet,udword *size);

bool checkPlayersReady(void);

void PlayerDroppedOut(udword player,bool timedOut);
void PlayerInGameQuited(udword player);

void CaptaincyChangedNotify(void);

// Keep alive stuff:

void KeepAliveReset(void);
void KeepAliveStartup(void);
void KeepAliveShutdown(void);
void KeepAliveUpdate(void);
void KeepAliveDropPlayerCB(sdword playerindex);
void KeepAliveDontDropPlayerCB(sdword playerindex);

/*=============================================================================
    Variables:
=============================================================================*/

extern udword sentPacketNumber;
extern udword receivedPacketNumber;

#endif

