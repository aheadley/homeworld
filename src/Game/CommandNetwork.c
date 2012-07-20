/*=============================================================================
    Name    : CommandNetwork.c
    Purpose : Handles all of the packing/unpacking sending/receiving of
              Command packets and sync packets

    Created 7/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <stdlib.h>
#include "Switches.h"
#include "Types.h"
#include "Debug.h"
#include "Memory.h"
#include "Task.h"
#include "Queue.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "CommandLayer.h"
#include "NetCheck.h"
#include "Globals.h"
#include "utility.h"
#include "CommandNetwork.h"
#include "TitanInterfaceC.h"
#include "HorseRace.h"
#include "Chatting.h"
#include "mainswitches.h"
#include "Captaincy.h"
#include "Titan.h"
#include "TimeoutTimer.h"
#include "TitanNet.h"
#include "MultiplayerGame.h"        // for mutex stuff
#include "AutoDownloadMap.h"
#include "LagPrint.h"
#include "Sensors.h"

/*=============================================================================
    Private Defines:
=============================================================================*/

#define HIGH_WATER_MARK     10

/*=============================================================================
    Private Variables:
=============================================================================*/

udword syncPacketBeforeGameStarted = 0;

udword packetBeforeGameStarted = 0;
udword unknownPackets = 0;
udword printedUnknownPackets = 0;

udword sentPacketNumber = 0;
udword receivedPacketNumber = 0;

udword syncpkts = 0;
udword cmdpkts = 0;
udword syncoverruns = 0;
udword cmdoverruns = 0;

// Keep Alive variables

#define ALIVESTATUS_ALIVE                   0
#define ALIVESTATUS_QUERYINGDROPOUT         1
#define ALIVESTATUS_WAITQUERYDROPOUTLATER   2
#define ALIVESTATUS_DROPPEDOUT              3

#define KILLDROPPEDOUTPLAYER_VERIFY     0xfe389bca

TTimer SendIAmAliveTimer;
TTimer SendAliveStatusTimer;

void *AliveTimeoutStatusesMutex = NULL;
TTimer AliveTimeoutTimers[MAX_MULTIPLAYER_PLAYERS];
ubyte AliveStatuses[MAX_MULTIPLAYER_PLAYERS];
bool8 HaveKilledPlayerDueToDropout[MAX_MULTIPLAYER_PLAYERS];

bool KeepAliveCalledFirstTime = FALSE;

// variables for printing out the message when a player has dropped out
udword numPlayerDropped = 0;
udword playersDropped[MAX_MULTIPLAYER_PLAYERS];
real32 printTimeout = 0.0f;

/*=============================================================================
    Private functions:
=============================================================================*/

void KeepAliveStartTimers();
void receivedIAmAlivePacket(HWPacketHeader *packet,udword sizeofPacket);
void receivedAliveStatusPacket(AliveStatusPacket *packet,udword sizeofPacket);

void BroadCastHorseRacePacketFromCaptain(ubyte *packet, udword sizeofpacket);
void BroadCastChatPacketFromCapatain(ChatPacket *packet, udword sizeofpacket);

void clPlayerDropped(udword playerMask,udword verify);
void clSendPlayerDropped(udword playerDroppedMask);

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : CommandNetworkReset
    Description : resets the CommandNetwork
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CommandNetworkReset(void)
{
    sdword i;

    syncPacketBeforeGameStarted = 0;

    packetBeforeGameStarted = 0;
    unknownPackets = 0;
    printedUnknownPackets = 0;

    sentPacketNumber = 0;
    receivedPacketNumber = 0;

    syncpkts = 0;
    cmdpkts = 0;
    syncoverruns = 0;
    cmdoverruns = 0;

    printCaptainMessage = FALSE;
    numPlayerDropped = 0;
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        playersDropped[i] = 0;
    }
    printTimeout = 0;

    utyPlayerDroppedDisplay = -1;
}

/*-----------------------------------------------------------------------------
    Name        : ReceivedSyncPacketCB
    Description : callback function which is called whenever a sync packet is
                  received.
    Inputs      : packet, sizeofPacket
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ReceivedSyncPacketCB(ubyte *packet,udword sizeofPacket)
{
    dbgAssert(((HWPacketHeader *)packet)->type == PACKETTYPE_SYNC);

    LockQueue(&ProcessSyncPktQ);
    HWEnqueue(&ProcessSyncPktQ,packet,sizeofPacket);
    UnLockQueue(&ProcessSyncPktQ);
}

/*-----------------------------------------------------------------------------
    Name        : ReceivedCmdPacketCB
    Description : callback function which is called whenever a command packet is
                  received.
    Inputs      : packet, sizeofPacket
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ReceivedCmdPacketCB(ubyte *packet,udword sizeofPacket)
{
    dbgAssert(((HWPacketHeader *)packet)->type == PACKETTYPE_COMMAND);

    if (IAmCaptain)
    {
        LockQueue(&ProcessCmdPktQ);
        HWEnqueue(&ProcessCmdPktQ,packet,sizeofPacket);
        UnLockQueue(&ProcessCmdPktQ);
    }
    // else throw packet away
}

/*-----------------------------------------------------------------------------
    Name        : ReceivedSyncPacketCB
    Description : callback function which is called whenever a sync packet is
                  received.
    Inputs      : packet, sizeofPacket
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ReceivedRequestedSyncPacketCB(ubyte *packet,udword sizeofPacket)
{
    dbgAssert(((HWPacketHeader *)packet)->type == PACKETTYPE_REQUESTEDSYNC);

    LockQueue(&ProcessRequestedSyncPktQ);
    HWEnqueue(&ProcessRequestedSyncPktQ,packet,sizeofPacket);
    UnLockQueue(&ProcessRequestedSyncPktQ);
}

void ReceivedRequestSyncPacketsPacketCB(ubyte *packet,udword sizeofPacket)
{
    udword frompacketnum;
    udword topacketnum;
    udword i;
    HWPacketHeader *sendpacket;
    udword sendpacketsize;
    bool gotit;
    uword sendto;

    dbgAssert(((HWPacketHeader *)packet)->type == PACKETTYPE_REQUESTSYNCPKTS);

    frompacketnum = ((RequestSyncPacketsPacket *)packet)->packetheader.frame;
    topacketnum = ((RequestSyncPacketsPacket *)packet)->topacketnum;
    sendto = ((RequestSyncPacketsPacket *)packet)->packetheader.from;

    dbgAssert(frompacketnum <= topacketnum);

    for (i=frompacketnum;i<=topacketnum;i++)
    {
        gotit = GetSyncPktFromLastSyncPktsQ(i,&sendpacket,&sendpacketsize);
        if (gotit)
        {
            sendpacket->from = sigsPlayerIndex;     // same sync pkt as before, but from me this time
            sendpacket->type = PACKETTYPE_REQUESTEDSYNC;    // and this also now a requested sync pkt
            titanSendPointMessage(sendto,(ubyte *)sendpacket,sendpacketsize);
        }
        else
        {
            if (i == topacketnum)   // always have to send the last packet requested for it to continue
            {
                HWPacketHeader fakelastsyncpkt;

                fakelastsyncpkt.type = PACKETTYPE_REQUESTEDSYNC;
                fakelastsyncpkt.from = sigsPlayerIndex;
                fakelastsyncpkt.frame = (udword)-1;
                fakelastsyncpkt.numberOfCommands = 0;

                titanSendPointMessage(sendto,(ubyte *)&fakelastsyncpkt,sizeof(HWPacketHeader));
            }
            titanDebug("Warning: Do not have sync pkt %d to send by request to %d\n",i,sendto);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : ReceivedTransferCaptaincyPacketCB
    Description : callback function which is called whenever a captaincy packet is received.
    Inputs      : packet, sizeofPacket
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ReceivedTransferCaptaincyPacketCB(ubyte *packet,udword sizeofPacket)
{
    dbgAssert(((HWPacketHeader *)packet)->type == PACKETTYPE_TRANSFERCAPTAINCY);

    LockQueue(&ProcessCaptaincyPktQ);
    HWEnqueue(&ProcessCaptaincyPktQ,packet,sizeofPacket);
    UnLockQueue(&ProcessCaptaincyPktQ);
}

bool checkPlayersReady(void)
{
    sdword i;
    bool ready = TRUE;

    dbgAssert((IAmCaptain) && (!multiPlayerGameUnderWay));

    for (i=0;i<sigsNumPlayers;i++)
    {
        if (i != captainIndex)
        {
            if (playersReadyToGo[i] == FALSE)
            {
                ready = FALSE;
            }
        }
    }

    return ready;
}

void PlayerDroppedOut(udword player,bool timedOut)
{
    if (player < sigsNumPlayers)
    {
        if (timedOut)
        {
            playersReadyToGo[player] = PLAYER_DROPPED_OUT;
            captaincyLog(FALSE,"Player %d dropped out",player);
        }
        else
        {
            playersReadyToGo[player] = PLAYER_QUIT;
        }

        if (player == captainIndex)
        {
            // captain just joined out, pick next captain in line:
            captainIndex = (captainIndex + 1) % sigsNumPlayers;
            receiveSyncPacketsFrom = captainIndex;
        }
    }
}

void PlayerInGameQuited(udword player)
{
    if (player < sigsNumPlayers)
    {
        playersReadyToGo[player] = PLAYER_QUIT;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ReceivedPacketCB
    Description : callback function which is called whenever any packet is
                  received.  It dispatches the packet as appropriate.
    Inputs      : packet, sizeofPacket
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ReceivedPacketCB(ubyte *packet,udword sizeofPacket)
{
    if ((!gameIsRunning) && (!startingGame))
    {
        packetBeforeGameStarted++;

        if (((HWPacketHeader *)packet)->type == PACKETTYPE_SYNC)
        {
            syncPacketBeforeGameStarted++;
        }
        else if (((HWPacketHeader *)packet)->type == PACKETTYPE_CHAT)
        {
            if (IAmCaptain)
            {
                BroadCastChatPacketFromCapatain((ChatPacket *)packet, sizeofPacket);
            }
            else
                recievedChatPacketCB(packet, sizeofPacket);
        }
        else if (((HWPacketHeader *)packet)->type == PACKETTYPE_FILE)
        {
            receivedFilePacketCB(packet,sizeofPacket);
        }

        return;
    }

    switch (((HWPacketHeader *)packet)->type)
    {
        case PACKETTYPE_TRANSFERCAPTAINCY:
            ReceivedTransferCaptaincyPacketCB(packet,sizeofPacket);
            break;

        case PACKETTYPE_SYNC:
            ReceivedSyncPacketCB(packet,sizeofPacket);
            break;

        case PACKETTYPE_COMMAND:
            ReceivedCmdPacketCB(packet,sizeofPacket);
            break;

        case PACKETTYPE_REQUESTEDSYNC:
            ReceivedRequestedSyncPacketCB(packet,sizeofPacket);
            break;

        case PACKETTYPE_REQUESTSYNCPKTS:
            ReceivedRequestSyncPacketsPacketCB(packet,sizeofPacket);
            break;

        case PACKETTYPE_DROPPINGOUTOFLOAD:
            if (!multiPlayerGameUnderWay)
            {
                PlayerDroppedOut(((HWPacketHeader *)packet)->frame,FALSE);
            }
            break;

        case PACKETTYPE_INGAMEQUITING:
            PlayerInGameQuited(((HWPacketHeader *)packet)->frame);
            break;

        case PACKETTYPE_NONCAPTAINREADY:
            if (!multiPlayerGameUnderWay)
            {
                sdword player = ((HWPacketHeader *)packet)->frame;       // use frame field as player number

                if (player < sigsNumPlayers)
                {
                    playersReadyToGo[player] = TRUE;
                }
            }
            break;

        case PACKETTYPE_CHAT:       //assumption:   If we revieved the packet, we were meant to see it
            if (IAmCaptain)
            {
                BroadCastChatPacketFromCapatain((ChatPacket *)packet, sizeofPacket);
            }
            else
                recievedChatPacketCB(packet, sizeofPacket);

            break;
        case PACKETTYPE_HORSERACE:
            if(IAmCaptain)
            {
                BroadCastHorseRacePacketFromCaptain(packet,sizeofPacket);
            }
            recievedHorsePacketCB(packet,sizeofPacket);
            break;

        case PACKETTYPE_IAMALIVE:
            if ((!CaptainNotDefined) && (IAmCaptain))
            {
                receivedIAmAlivePacket((HWPacketHeader *)packet,sizeofPacket);
            }
            break;

        case PACKETTYPE_ALIVESTATUS:
            if ((!CaptainNotDefined) && (!IAmCaptain))
            {
                receivedAliveStatusPacket((AliveStatusPacket *)packet,sizeofPacket);
            }
            break;

        case PACKETTYPE_FILE:
            receivedFilePacketCB(packet,sizeofPacket);
            break;

        case PACKETTYPE_LAGCHECK:
            lagRecievedPacketCB(packet,sizeofPacket);
            break;

        case PACKETTYPE_CHEATDETECT:
            {
                if (!blobSyncErr)
                {
                    blobSyncErrFrame = ((HWPacketHeader *)packet)->frame;
                }
                blobSyncErr++;
            }
            break;

        default:
            unknownPackets++;
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : SetTargetID
    Description : sets the TargetID based on the target
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SetTargetID(TargetID *targetID,SpaceObjRotImpTarg *target)
{
    targetID->objtype = (uword)target->objtype;
    switch (target->objtype)
    {
        case OBJ_ShipType:
            targetID->objNumber = ((Ship *)target)->shipID.shipNumber;
            break;

        case OBJ_MissileType:
            targetID->objNumber = ((Missile *)target)->missileID.missileNumber;
            break;

        case OBJ_AsteroidType:
        case OBJ_NebulaType:
        case OBJ_GasType:
        case OBJ_DustType:
            targetID->objNumber = ((Resource *)target)->resourceID.resourceNumber;
            break;

        case OBJ_DerelictType:
            targetID->objNumber = ((Derelict *)target)->derelictID.derelictNumber;
            break;
        default:
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : convertNetSelectionToSelectCommand
    Description : converts a NetSelect type to a SelectCommand type.  It converts
                  all ShipID's to ShipPtr's.  Any dead ShipID's or ships that don't belong
                  to player from are rejected
    Inputs      :
    Outputs     :
    Return      : returns the allocated SelectCommand.  Caller must free it when done.
----------------------------------------------------------------------------*/
SelectCommand *convertNetSelectionToSelectCommand(NetSelection *netselection,bool considerInsideShips,uword from)
{
    udword numShips = netselection->numShips;
    udword selectCommandSize = sizeofSelectCommand(numShips);
    SelectCommand *selectCommand;
    bool someShipsDied = FALSE;
    bool someShipsOrderedByWrongPlayer = FALSE;
    udword i;
    uword shipPlayerIndex;

    dbgAssert(numShips > 0);

    selectCommand = memAlloc(selectCommandSize,"conv. SelCom",0);

    selectCommand->numShips = numShips;

    for (i=0;i<numShips;i++)
    {
        if ((selectCommand->ShipPtr[i] = ShipIDtoShip(netselection->ShipID[i],considerInsideShips)) == NULL)
        {
            someShipsDied = TRUE;
        }
        else
        {
            shipPlayerIndex = selectCommand->ShipPtr[i]->playerowner->playerIndex;
            
            // if it's a human ship
            if ((shipPlayerIndex >= 0) && (shipPlayerIndex < tpGameCreated.numPlayers))
            {
                // do further cheat check - only can control own ship
                if (shipPlayerIndex != from)
            {
                selectCommand->ShipPtr[i] = NULL;       // reject ship
                someShipsDied = TRUE;
                someShipsOrderedByWrongPlayer = TRUE;
            }
        }
    }
    }

    if (someShipsDied)
    {
        if (someShipsOrderedByWrongPlayer)
            dbgMessage("\nWARNING: Some ships ordered by wrong player!");
        clRemoveShipFromSelection(selectCommand,NULL);  // remove NULL ship pointers from selectCommand
    }

    return selectCommand;
}

/*-----------------------------------------------------------------------------
    Name        : convertNetAttackSelectionToAttackCommand
    Description : converts a NetAttackSelection to a AttackCommand type.  It converts
                  all TargetID's to Target's.
    Inputs      :
    Outputs     :
    Return      : returns the allocated AttackCommand.  Caller must free it when done.
----------------------------------------------------------------------------*/
AttackCommand *convertNetAttackSelectionToAttackCommand(NetAttackSelection *netattackselection)
{
    udword numTargets = netattackselection->numTargets;
    udword attackCommandSize = sizeofAttackCommand(numTargets);
    AttackCommand *attackCommand;
    bool someTargetsDied = FALSE;
    udword i;
    TargetID targetID;
    ShipID shipID;
    MissileID missileID;
    ResourceID resourceID;
    DerelictID derelictID;

    dbgAssert(numTargets > 0);

    attackCommand = memAlloc(attackCommandSize,"conv. AttCom",0);

    attackCommand->numTargets = numTargets;

    for (i=0;i<numTargets;i++)
    {
        targetID = netattackselection->TargetID[i];

        switch (targetID.objtype)
        {
            case OBJ_ShipType:
                shipID.shipNumber = targetID.objNumber;
                attackCommand->TargetPtr[i] = (SpaceObjRotImpTarg *)ShipIDtoShip(shipID,FALSE);
                break;

            case OBJ_MissileType:
                missileID.missileNumber = targetID.objNumber;
                attackCommand->TargetPtr[i] = (SpaceObjRotImpTarg *)MissileIDtoMissilePtr(missileID);
                break;

            case OBJ_AsteroidType:
            case OBJ_NebulaType:
            case OBJ_GasType:
            case OBJ_DustType:
                resourceID.resourceNumber = targetID.objNumber;
                attackCommand->TargetPtr[i] = (SpaceObjRotImpTarg *)ResourceIDtoResourcePtr(resourceID);
                break;

            case OBJ_DerelictType:
                derelictID.derelictNumber = targetID.objNumber;
                attackCommand->TargetPtr[i] = (SpaceObjRotImpTarg *)DerelictIDToDerelictPtr(derelictID);
                break;
        }

        if (attackCommand->TargetPtr[i] == NULL)
        {
            someTargetsDied = TRUE;
        }
    }

    if (someTargetsDied)
    {
        dbgMessage("\nWARNING: Some targets died before order executed");
        clRemoveTargetFromSelection(attackCommand,NULL);  // remove NULL ship pointers from attackCommand
    }

    return attackCommand;
}

/*-----------------------------------------------------------------------------
    Name        : packetSendToCaptain
    Description : sends packet to the captain (if not the captain, actually
                  sends to captain across the network.  If is the captain,
                  fake sends it.
    Inputs      :
    Outputs     :
    Return      : TRUE if packet memory should be freed.
----------------------------------------------------------------------------*/
bool packetSendToCaptain(ubyte *packet,udword sizeofPacket)
{
    dbgAssert(((HWPacketHeader *)packet)->type == PACKETTYPE_COMMAND);

    if (playPackets)
    {
        return TRUE;
    }

    if (CaptainNotDefined)
    {
        return TRUE;
    }

    if (recordFakeSendPackets)
    {
        ReceivedCmdPacketCB(packet,sizeofPacket);      // simulate fake transmit
        return TRUE;
    }

    if (IAmCaptain)
    {
        ReceivedCmdPacketCB(packet,sizeofPacket);      // simulate fake transmit to captain
        return TRUE;
    }
    else
    {
        titanSendPointMessage(captainIndex,packet,sizeofPacket);        // send message to captain
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : packetBroadcastSync
    Description : broadcasts the sync packet
    Inputs      :
    Outputs     :
    Return      : TRUE if packet memory should be freed
----------------------------------------------------------------------------*/
bool packetBroadcastSync(ubyte *packet,udword sizeofPacket)
{
    dbgAssert(((HWPacketHeader *)packet)->type == PACKETTYPE_SYNC);
    dbgAssert(IAmCaptain);

    if (playPackets)
    {
        return TRUE;
    }

    titanSendBroadcastMessage(packet,sizeofPacket);     // send to everyone else
    ReceivedSyncPacketCB(packet,sizeofPacket);          // and myself too.
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : clSendMove
    Description : sends the Move command over the network
    Inputs      : selectcom,from,to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendMove(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetMoveCommand *move;
    udword numShips = selectcom->numShips;
    udword i;

    dbgAssert(numShips > 0);

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetMoveCommand(numShips);

    packet = memAlloc(sizeofPacket,"movepacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_MOVE | (sigsPlayerIndex<<8);

    move = (NetMoveCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    move->from = from;
    move->to = to;
    move->selection.numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        move->selection.ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}
/*-----------------------------------------------------------------------------
    Name        : clSendMpHyperspace
    Description : sends the Multiplayer hyperspace command over the network
    Inputs      : selectcom,from,to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetMoveCommand *move;
    udword numShips = selectcom->numShips;
    udword i;

    dbgAssert(numShips > 0);

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetMoveCommand(numShips);

    packet = memAlloc(sizeofPacket,"hyperspacepacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_MP_HYPERSPACE | (sigsPlayerIndex<<8);

    move = (NetMoveCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    move->from = from;
    move->to = to;
    move->selection.numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        move->selection.ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendAttack
    Description : sends the attack command over the network
    Inputs      : selectcom,attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetAttackSelection *attack;
    NetSelection *selection;
    udword numShips = selectcom->numShips;
    udword numShipsToAttack = attackcom->numTargets;
    udword i;

    dbgAssert(numShips > 0);
    dbgAssert(numShipsToAttack > 0);

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetAttackCommand(numShipsToAttack,numShips);

    packet = memAlloc(sizeofPacket,"attackpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_ATTACK | (sigsPlayerIndex<<8);

    attack = (NetAttackSelection *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    attack->numTargets = (uword)numShipsToAttack;
    for (i=0;i<numShipsToAttack;i++)
    {
        SetTargetID(&attack->TargetID[i],attackcom->TargetPtr[i]);
    }

    selection = (NetSelection *)(((ubyte *)attack) + sizeofNetAttackSelection(numShipsToAttack));
    selection->numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        selection->ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendSpecial
    Description : sends the special command over the network
    Inputs      : selectcom,targetcom
    Outputs     :
    Return      :
    Notes       : targetcom may be NULL
----------------------------------------------------------------------------*/
void clSendSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targetcom)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetTargetsSelection *targets;
    NetSelection *selection;
    sdword numShips = selectcom->numShips;
    sdword numShipsToTarget;
    sdword i;

    dbgAssert(numShips > 0);
    if (targetcom == NULL)
    {
        numShipsToTarget = 0;
    }
    else
    {
        numShipsToTarget = targetcom->numTargets;
    }

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetSpecialCommand(numShipsToTarget,numShips);

    packet = memAlloc(sizeofPacket,"targetspacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_SPECIAL | (sigsPlayerIndex<<8);

    targets = (NetTargetsSelection *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    targets->numTargets = (uword)numShipsToTarget;
    for (i=0;i<numShipsToTarget;i++)
    {
        SetTargetID(&targets->TargetID[i],targetcom->TargetPtr[i]);
    }

    selection = (NetSelection *)(((ubyte *)targets) + sizeofNetTargetsSelection(numShipsToTarget));
    selection->numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        selection->ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendFormation
    Description : sends the formation command over the network
    Inputs      : selectcom, formation
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetFormationCommand *formationcom;
    udword numShips = selectcom->numShips;
    udword i;

    if (selectcom->numShips < ABSOLUTE_MIN_SHIPS_IN_FORMATION)
    {
#ifdef DEBUG_FORMATIONS
        dbgMessage("\nNot enough ships to do a formation");
#endif
        return;
    }

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetFormationCommand(numShips);

    packet = memAlloc(sizeofPacket,"formationpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_FORMATION | (sigsPlayerIndex<<8);

    formationcom = (NetFormationCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    formationcom->typeOfFormation = (uword)formation;
    formationcom->selection.numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        formationcom->selection.ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendDock
    Description : sends the Dock command over the network
    Inputs      : selectcom, dockType
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetDockCommand *dock;
    udword numShips = selectcom->numShips;
    udword i;

    dbgAssert(numShips > 0);

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetDockCommand(numShips);

    packet = memAlloc(sizeofPacket,"dockpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_DOCK | (sigsPlayerIndex<<8);

    dock = (NetDockCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    if (dockwith != NULL)
    {
        dock->dockwithID = dockwith->shipID;
    }
    else
    {
        dock->dockwithID.shipNumber = MAX_ID_NUMBER;
    }
    dock->dockType = (uword)dockType;
    dock->selection.numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        dock->selection.ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendLaunchMultipleShips
    Description : sends the Launch multiple ships command over the network
    Inputs      : selectcom, launchFrom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetLaunchMultipleCommand *launch;
    udword numShips = selectcom->numShips;
    udword i;

    dbgAssert(numShips > 0);
    dbgAssert(launchFrom);

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetLaunchMultipleCommand(numShips);

    packet = memAlloc(sizeofPacket,"launchpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_LAUNCHMULTIPLE | (sigsPlayerIndex<<8);

    launch = (NetLaunchMultipleCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    launch->launchfromID = launchFrom->shipID;
    launch->selection.numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        launch->selection.ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendMisc
    Description : sends a miscellaneous command of type miscCommand over the network
    Inputs      : selectcom, miscCommand, miscData
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendMisc(CommandLayer *comlayer,SelectCommand *selectcom,uword miscCommand,udword miscData)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetMiscCommand *misc;
    udword numShips = selectcom->numShips;
    udword i;

    dbgAssert(numShips > 0);

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetMiscCommand(numShips);

    packet = memAlloc(sizeofPacket,"miscpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_MISC | (sigsPlayerIndex<<8);

    misc = (NetMiscCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    misc->miscCommand = miscCommand;
    misc->miscData = miscData;
    misc->selection.numShips = (uword)numShips;

    for (i=0;i<numShips;i++)
    {
        misc->selection.ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}


#ifdef GOD_LIKE_SYNC_CHECKING
void clSendGodSync(GodSyncCheckSums *checksumStruct,sdword playIndex,udword type)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    GodSyncCommand *misc;
    udword numShips = 0;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader)  + sizeof(GodSyncCommand);

    packet = memAlloc(sizeofPacket,"godpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_GODSYNC | (sigsPlayerIndex<<8);

    misc = (GodSyncCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    misc->type = type;
    misc->frame = universe.univUpdateCounter;
    misc->from=playIndex;

    //copy check
    misc->godChecksum=*checksumStruct;

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}
#endif
void clSendRuTransfer(CommandLayer *comlayer,sdword fromIndex,sdword toIndex, sdword resourceUnits,ubyte flags)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetRUTransferferCommand *ruTransfer;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeof(NetRUTransferferCommand);

    packet = memAlloc(sizeofPacket,"rtranspacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_RUTRANSFER | (sigsPlayerIndex<<8);

    ruTransfer = (NetRUTransferferCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));

    ruTransfer->resourceUnits = resourceUnits;
    ruTransfer->to = (ubyte)toIndex;
    ruTransfer->from = (ubyte)fromIndex;
    ruTransfer->flags = flags;

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}


/*-----------------------------------------------------------------------------
    Name        : clSendAutoLaunch
    Description : sends the autolaunch command over the network (for turning autolaunch on/off)
    Inputs      : OnOff, playerIndex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendAutoLaunch(udword OnOff,udword playerIndex)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetAutolaunchCommand *autolaunch;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetAutolaunchCommand;

    packet = memAlloc(sizeofPacket,"alaunchpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_AUTOLAUNCH | (sigsPlayerIndex<<8);

    autolaunch = (NetAutolaunchCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));

    autolaunch->OnOff = (uword)OnOff;
    autolaunch->playerIndex = (uword)playerIndex;

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendSetAlliance
    Description : Sets an alliance based on a player bitmask recieved.
    Inputs      : player bitmask
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void clSendSetAlliance(udword AllianceType, uword playerone, uword playertwo)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetAllianceCommand *allianceptr;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetAllianceCommand;

    packet = memAlloc(sizeofPacket,"aliancepacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_ALLIANCEINFO | (sigsPlayerIndex<<8);

    allianceptr = (NetAllianceCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));

    allianceptr->AllianceMsgType = AllianceType;
    allianceptr->CurAlliance     = playerone;
    allianceptr->NewAlliance     = playertwo;

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

void clSendResearch(udword type, udword playernum, udword labnum, udword tech)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetResearchCommand *researchptr;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetResearchCommand;

    packet = memAlloc(sizeofPacket,"researchpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_RESEARCHINFO | (sigsPlayerIndex<<8);

    researchptr = (NetResearchCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));

    researchptr->ResearchMsgType  = (ubyte) type;
    researchptr->playerindex      = (ubyte) playernum;
    researchptr->labnum           = (ubyte) labnum;
    researchptr->tech             = (ubyte) tech;

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendPlayerDropped(udword playerDroppedMask)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetPlayerDroppedCommand *PlayerDroppedptr;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetPlayerDroppedCommand;

    packet = memAlloc(sizeofPacket,"droppacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_PLAYERDROPPED | (sigsPlayerIndex<<8);

    PlayerDroppedptr = (NetPlayerDroppedCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));

    PlayerDroppedptr->playerDroppedMask = playerDroppedMask;
    if (IAmCaptain) PlayerDroppedptr->verify = playerDroppedMask ^ KILLDROPPEDOUTPLAYER_VERIFY;

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendCreateShip
    Description : sends the create ship command over the network
    Inputs      : shipType, shipRace, playerIndex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetCreateShipCommand *create;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetCreateShipCommand;

    packet = memAlloc(sizeofPacket,"createpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_CREATESHIP | (sigsPlayerIndex<<8);

    create = (NetCreateShipCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));

    create->shipType = (uword)shipType;
    create->shipRace = (uword)shipRace;
    create->playerIndex = playerIndex;
    create->creatorID = creator->shipID;

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

void clSendDeterministicBuild(udword command, CommandLayer* comlayer, sdword numShips,
                              ShipType shipType, ShipRace shipRace,
                              uword playerIndex, ShipPtr creator)
{
    udword sizeofPacket;
    HWPacketHeader* packet;
    HWCommandHeader* commandheader;
    NetDeterministicBuildCommand* create;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetDeterministicBuildCommand;

    packet = memAlloc(sizeofPacket, "dbuildpacket", 0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader*)(((ubyte*)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_DETERMINISTICBUILD | (sigsPlayerIndex<<8);

    create = (NetDeterministicBuildCommand*)(((ubyte*)commandheader) + sizeof(HWCommandHeader));

    create->subCommand = (ubyte)command;
    create->numShips = (uword)numShips;
    create->shipType = (uword)shipType;
    create->shipRace = (uword)shipRace;
    create->playerIndex = playerIndex;
    create->creatorID = creator->shipID;

    if (packetSendToCaptain((ubyte*)packet, sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendBuildShip
    Description : sends the build ship command over the network
    Inputs      : shipType, shipRace, playerIndex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetCreateShipCommand *create;

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetCreateShipCommand;

    packet = memAlloc(sizeofPacket,"buildpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_BUILDSHIP | (sigsPlayerIndex<<8);

    create = (NetCreateShipCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));

    create->shipType = (uword)shipType;
    create->shipRace = (uword)shipRace;
    create->playerIndex = playerIndex;
    create->creatorID = creator->shipID;

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendCollectResource
    Description : sends the CollectResource command over the network
    Inputs      : selectcom, resource
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetCollectResourceCommand *collect;
    udword numShips = selectcom->numShips;
    udword i;

    dbgAssert(numShips > 0);

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetCollectResourceCommand(numShips);

    packet = memAlloc(sizeofPacket,"collectpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_COLLECTRESOURCE | (sigsPlayerIndex<<8);

    collect = (NetCollectResourceCommand *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    if (resource == NULL)
    {
        collect->resourceID.resourceNumber = MAX_ID_NUMBER;
    }
    else
    {
        collect->resourceID = resource->resourceID;
    }
    collect->selection.numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        collect->selection.ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSendProtect
    Description : sends protect command over network
    Inputs      : selectcom, protectcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSendProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom)
{
    udword sizeofPacket;
    HWPacketHeader *packet;
    HWCommandHeader *commandheader;
    NetProtectSelection *protect;
    NetSelection *selection;
    udword numShips = selectcom->numShips;
    udword numShipsToProtect = protectcom->numShips;
    udword i;

    dbgAssert(numShips > 0);
    dbgAssert(numShipsToProtect > 0);

    sizeofPacket = sizeof(HWPacketHeader) + sizeof(HWCommandHeader) + sizeofNetProtectCommand(numShipsToProtect,numShips);

    packet = memAlloc(sizeofPacket,"protectpacket",0);

    packet->type = PACKETTYPE_COMMAND;
    packet->from = (uword)sigsPlayerIndex;
    packet->frame = 0;
    packet->numberOfCommands = 1;

    commandheader = (HWCommandHeader *)(((ubyte *)packet) + sizeof(HWPacketHeader));
    commandheader->commandType = COMMANDTYPE_PROTECT | (sigsPlayerIndex<<8);

    protect = (NetProtectSelection *)(((ubyte *)commandheader) + sizeof(HWCommandHeader));
    protect->numShips = (uword)numShipsToProtect;
    for (i=0;i<numShipsToProtect;i++)
    {
        protect->ShipID[i] = (protectcom->ShipPtr[i])->shipID;
    }

    selection = (NetSelection *)(((ubyte *)protect) + sizeofNetProtectSelection(numShipsToProtect));
    selection->numShips = (uword)numShips;
    for (i=0;i<numShips;i++)
    {
        selection->ShipID[i] = (selectcom->ShipPtr[i])->shipID;
    }

    if (packetSendToCaptain((ubyte *)packet,sizeofPacket))
    {
        memFree(packet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clProcessSyncPacket
    Description : Given the sync packet, it parses it and executes any commands
                  inside it by passing them to the command layer.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clProcessSyncPacket(CommandLayer *comlayer,ubyte *packet,udword sizeofPacket)
{
    ubyte *curpacket;
    udword numberOfCommandsLeft;
    uword commandType;
    uword from;

    dbgAssert(((HWPacketHeader *)packet)->type == PACKETTYPE_SYNC);

    if (!playPackets)
    if (((HWPacketHeader *)packet)->frame != receivedPacketNumber)
    {
        dbgMessagef("\nExpected pkt %d got %d",receivedPacketNumber,((HWPacketHeader *)packet)->frame);
        pktSyncErr++;
    }
    receivedPacketNumber++;

    if (recordPackets)
    {
        if (OrigRecordPacketFileName[0])
        {
            recPackRecordPacketFilename(packet,sizeofPacket,OrigRecordPacketFileName);
        }

        recPackRecordPacket(packet,sizeofPacket);
    }
    netcheckShow((HWPacketHeader *)packet);

#ifdef DEBUG_GAME_STATS
    if(statLogOn)
    {
        gameStatsUpdateLogFile(universe.univUpdateCounter);
    }
#endif

    numberOfCommandsLeft = (udword)((HWPacketHeader *)packet)->numberOfCommands;

    curpacket = packet + sizeof(HWPacketHeader);

    while (numberOfCommandsLeft > 0)
    {
        commandType = ((HWCommandHeader *)curpacket)->commandType;
        from = (commandType >> 8) & 255;
        commandType &= 255;
        curpacket += sizeof(HWCommandHeader);

        switch (commandType)
        {
            case COMMANDTYPE_MOVE:
            {
                NetMoveCommand *movecom = (NetMoveCommand *)curpacket;
                SelectCommand *moveselection = convertNetSelectionToSelectCommand(&movecom->selection,FALSE,from);
                clMove(comlayer,moveselection,movecom->from,movecom->to);
                memFree(moveselection);
                curpacket += sizeofNetMoveCommand(movecom->selection.numShips);
            }
            break;

            case COMMANDTYPE_MP_HYPERSPACE:
            {
                NetMoveCommand *movecom = (NetMoveCommand *)curpacket;
                SelectCommand *moveselection = convertNetSelectionToSelectCommand(&movecom->selection,FALSE,from);
                clMpHyperspace(comlayer,moveselection,movecom->from,movecom->to);
                memFree(moveselection);
                curpacket += sizeofNetMoveCommand(movecom->selection.numShips);
            }
            break;

            case COMMANDTYPE_ATTACK:
            {
                NetAttackSelection *netattackselection = (NetAttackSelection *)curpacket;
                NetSelection *netselection = (NetSelection *)(curpacket + sizeofNetAttackSelection(netattackselection->numTargets));
                AttackCommand *attackattack = convertNetAttackSelectionToAttackCommand(netattackselection);
                SelectCommand *attackselection = convertNetSelectionToSelectCommand(netselection,FALSE,from);
                clAttack(comlayer,attackselection,attackattack);
                memFree(attackattack);
                memFree(attackselection);
                curpacket += sizeofNetAttackCommand(netattackselection->numTargets,netselection->numShips);
            }
            break;

            case COMMANDTYPE_SPECIAL:
            {
                NetTargetsSelection *nettargetsselection = (NetTargetsSelection *)curpacket;
                sdword nettargetsselectionNumShips = nettargetsselection->numTargets;
                NetSelection *netselection = (NetSelection *)(curpacket + sizeofNetTargetsSelection(nettargetsselectionNumShips));
                SelectCommand *selectcom = convertNetSelectionToSelectCommand(netselection,FALSE,from);
                SpecialCommand *targetcom;
                if (nettargetsselectionNumShips > 0)
                {
                    targetcom = convertNetAttackSelectionToAttackCommand(nettargetsselection);
                }
                else
                {
                    targetcom = NULL;
                }
                clSpecial(comlayer,selectcom,targetcom);
                memFree(selectcom);
                if (targetcom != NULL) memFree(targetcom);
                curpacket += sizeofNetSpecialCommand(nettargetsselectionNumShips,netselection->numShips);
            }
            break;

            case COMMANDTYPE_FORMATION:
            {
                NetFormationCommand *formationcom = (NetFormationCommand *)curpacket;
                SelectCommand *formselection = convertNetSelectionToSelectCommand(&formationcom->selection,FALSE,from);
                clFormation(comlayer,formselection,(TypeOfFormation)formationcom->typeOfFormation);
                memFree(formselection);
                curpacket += sizeofNetFormationCommand(formationcom->selection.numShips);
            }
            break;

            case COMMANDTYPE_DOCK:
            {
                NetDockCommand *dockcom = (NetDockCommand *)curpacket;
                SelectCommand *dockselection = convertNetSelectionToSelectCommand(&dockcom->selection,FALSE,from);
                clDock(comlayer,dockselection,(DockType)dockcom->dockType,ShipIDtoShip(dockcom->dockwithID,FALSE));
                memFree(dockselection);
                curpacket += sizeofNetDockCommand(dockcom->selection.numShips);
            }
            break;

            case COMMANDTYPE_LAUNCHMULTIPLE:
            {
                NetLaunchMultipleCommand *launchcom = (NetLaunchMultipleCommand *)curpacket;
                SelectCommand *launchselection = convertNetSelectionToSelectCommand(&launchcom->selection,TRUE,from);
                clLaunchMultipleShips(comlayer,launchselection,ShipIDtoShip(launchcom->launchfromID,FALSE));
                memFree(launchselection);
                curpacket += sizeofNetLaunchMultipleCommand(launchcom->selection.numShips);
            }
            break;

            case COMMANDTYPE_MISC:
            {
                NetMiscCommand *misccom = (NetMiscCommand *)curpacket;
                SelectCommand *miscselection = convertNetSelectionToSelectCommand(&misccom->selection,FALSE,from);
                switch (misccom->miscCommand)
                {
                    case MISCCOMMAND_HALT:
                        clHalt(comlayer,miscselection);
                        break;

                    case MISCCOMMAND_SCUTTLE:
                        clScuttle(comlayer,miscselection);
                        break;

                    case MISCCOMMAND_TACTICS:
                        clSetTactics(comlayer,miscselection,(TacticsType)misccom->miscData);
                        break;

                    case MISCCOMMAND_KAMIKAZE:
                        clSetKamikaze(comlayer,miscselection);
                        break;

                    case MISCCOMMAND_PARADE:
                        clSetMilitaryParade(comlayer,miscselection);
                        break;
                    default:
                        dbgAssert(FALSE);
                        break;
                }
                memFree(miscselection);
                curpacket += sizeofNetMiscCommand(misccom->selection.numShips);
            }
            break;
#ifdef GOD_LIKE_SYNC_CHECKING
            case COMMANDTYPE_GODSYNC:
                // This is a special SYNC debug feature such that when this command is recieved the game will halt
                {
                    GodSyncCommand *gp=(GodSyncCommand *)curpacket;
                    switch(((GodSyncCommand *)curpacket)->type)
                    {
                        case GOD_COMMAND_HALTONSYNC:
                            if(syncDumpOn)
                            {
                                //user wants an int 3
                                syncDebugDump("syncDump.txt",universe.univUpdateCounter,TRUE);
                                //_asm int 3;
                                dbgNonFatal(DBG_Loc,"SYNC Error Detected crash to debuger?");
                            }
                            break;
                        case GOD_COMMAND_GOD_HALTONSYNC:
                            if(syncDumpOn)
                            {
                                netReceivedSyncFromNonCaptain(&gp->godChecksum,gp->frame,gp->from);
                            }
                            break;
                    }
                    curpacket += sizeof(GodSyncCommand);
                }
            break;
#endif
            case COMMANDTYPE_AUTOLAUNCH:
            {
                NetAutolaunchCommand *launchcom = (NetAutolaunchCommand *)curpacket;
                clAutoLaunch((udword)launchcom->OnOff,(udword)launchcom->playerIndex);
                curpacket += sizeofNetAutolaunchCommand;
            }
            break;

            case COMMANDTYPE_ALLIANCEINFO:
            {
                NetAllianceCommand *alliancecom = (NetAllianceCommand *)curpacket;
                clSetAlliance(alliancecom->AllianceMsgType, alliancecom->CurAlliance, alliancecom->NewAlliance);
                curpacket += sizeofNetAllianceCommand;
            }
            break;

            case COMMANDTYPE_PLAYERDROPPED:
            {
                NetPlayerDroppedCommand *droppedcom = (NetPlayerDroppedCommand *)curpacket;
                clPlayerDropped(droppedcom->playerDroppedMask,droppedcom->verify);
                curpacket += sizeofNetPlayerDroppedCommand;
            }
            break;

            case COMMANDTYPE_CREATESHIP:
            {
                NetCreateShipCommand *createcom = (NetCreateShipCommand *)curpacket;
                clCreateShip(comlayer,(ShipType)createcom->shipType,(ShipRace)createcom->shipRace,createcom->playerIndex,ShipIDtoShip(createcom->creatorID,FALSE));
                curpacket += sizeofNetCreateShipCommand;
            }
            break;

            case COMMANDTYPE_DETERMINISTICBUILD:
            {
                NetDeterministicBuildCommand* buildcom = (NetDeterministicBuildCommand*)curpacket;
                clDeterministicBuild(buildcom->subCommand, comlayer,
                                     buildcom->numShips,
                                     (ShipType)buildcom->shipType,
                                     (ShipRace)buildcom->shipRace,
                                     buildcom->playerIndex,
                                     ShipIDtoShip(buildcom->creatorID, FALSE));
                curpacket += sizeofNetDeterministicBuildCommand;
            }
            break;

            case COMMANDTYPE_BUILDSHIP:
            {
                NetCreateShipCommand *createcom = (NetCreateShipCommand *)curpacket;
                clBuildShip(comlayer,(ShipType)createcom->shipType,(ShipRace)createcom->shipRace,createcom->playerIndex,ShipIDtoShip(createcom->creatorID,FALSE));
                curpacket += sizeofNetCreateShipCommand;
            }
            break;

            case COMMANDTYPE_COLLECTRESOURCE:
            {
                NetCollectResourceCommand *collectcom = (NetCollectResourceCommand *)curpacket;
                SelectCommand *collectselection = convertNetSelectionToSelectCommand(&collectcom->selection,FALSE,from);
                clCollectResource(comlayer,collectselection,ResourceIDtoResourcePtr(collectcom->resourceID));
                memFree(collectselection);
                curpacket += sizeofNetCollectResourceCommand(collectcom->selection.numShips);
            }
            break;

            case COMMANDTYPE_PROTECT:
            {
                NetProtectSelection *netprotectselection = (NetProtectSelection *)curpacket;
                NetSelection *netselection = (NetSelection *)(curpacket + sizeofNetProtectSelection(netprotectselection->numShips));
                ProtectCommand *protectprotect = convertNetSelectionToSelectCommand(netprotectselection,FALSE,from);
                SelectCommand *protectselection = convertNetSelectionToSelectCommand(netselection,FALSE,from);
                clProtect(comlayer,protectselection,protectprotect);
                memFree(protectprotect);
                memFree(protectselection);
                curpacket += sizeofNetProtectCommand(netprotectselection->numShips,netselection->numShips);
            }
            break;

            case COMMANDTYPE_RUTRANSFER:
            {
                NetRUTransferferCommand *rucommand = (NetRUTransferferCommand *)curpacket;
                clRUTransfer(comlayer,rucommand->to,rucommand->from,rucommand->resourceUnits,rucommand->flags);
                curpacket+=sizeof(NetRUTransferferCommand);
            }
            break;

            case COMMANDTYPE_RESEARCHINFO:
            {
                NetResearchCommand *researchcommand = (NetResearchCommand *)curpacket;
                clSetResearch((udword)researchcommand->ResearchMsgType, (udword)researchcommand->playerindex, (udword)researchcommand->labnum, (udword)researchcommand->tech);
                curpacket+=sizeofNetResearchCommand;
            }
            break;

            default:
                dbgAssert(FALSE);
                break;
        }

        numberOfCommandsLeft--;
    }
}

#define LastSyncPktsQ_NUMBER 1024
#define LastSyncPktsQ_MASK   1023

typedef struct
{
    udword frame;
    HWPacketHeader *packet;
    udword length;
} CircularQEntry;

typedef struct
{
    udword head;
    CircularQEntry circularQEntry[LastSyncPktsQ_NUMBER];
} LastSyncPktsQ;

LastSyncPktsQ lastSyncPktsQ;

void InitLastSyncPktsQ(void)
{
    memset(&lastSyncPktsQ,0,sizeof(lastSyncPktsQ));
}

void CloseLastSyncPktsQ(void)
{
    sdword i;

    for (i=0;i<LastSyncPktsQ_NUMBER;i++)
    {
        if (lastSyncPktsQ.circularQEntry[i].packet)
        {
            memFree(lastSyncPktsQ.circularQEntry[i].packet);
        }
    }
    memset(&lastSyncPktsQ,0,sizeof(lastSyncPktsQ));
}

void ResetLastSyncPktsQ(void)
{
    CloseLastSyncPktsQ();
    InitLastSyncPktsQ();
}

void EnterIntoLastSyncPktsQ(udword frame,HWPacketHeader *packet,udword length)
{
    CircularQEntry *entry = &lastSyncPktsQ.circularQEntry[lastSyncPktsQ.head];

    if (entry->packet)
    {
        memFree(entry->packet);
    }

    dbgAssert(packet);
    entry->frame = frame;
    entry->packet = packet;
    entry->length = length;

    lastSyncPktsQ.head = (lastSyncPktsQ.head+1) & LastSyncPktsQ_MASK;
}

bool GetSyncPktFromLastSyncPktsQ(udword frame,HWPacketHeader **packet,udword *size)
{
    sdword i;

    dbgAssert(frame != 0);
    for (i=0;i<LastSyncPktsQ_NUMBER;i++)
    {
        if (lastSyncPktsQ.circularQEntry[i].frame == frame)
        {
            *packet = lastSyncPktsQ.circularQEntry[i].packet;
            *size = lastSyncPktsQ.circularQEntry[i].length;
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : clWaitSyncPacket
    Description : Waits for a sync packet.  If one is not present, returns FALSE.
                  If one is present, it processes the sync packet.
    Inputs      :
    Outputs     :
    Return      : TRUE if sync packet received and processed.  FALSE if
                  no sync packet present yet.
----------------------------------------------------------------------------*/
WaitPacketStatus clWaitSyncPacket(CommandLayer *comlayer)
{
    udword numPackets;
    udword sizeofPacket;
    ubyte *packet;
    ubyte *copypacket;

    if (playPackets)
    {
        copypacket = recPackPlayGetNextPacket(&sizeofPacket);
        if (copypacket == NULL)
        {
            return NO_PACKET;
        }
        clProcessSyncPacket(comlayer,copypacket,sizeofPacket);
        memFree(copypacket);
        return PACKET_READY;
    }

    if (receiveSyncPacketsFrom == -1)
    {
        return NO_PACKET;       // we shouldn't receive any packets from anyone till we find out who to receive from
    }

    if (explicitlyRequestingPackets)
    {
        LockQueue(&ProcessRequestedSyncPktQ);
        numPackets = queueNumberEntries(ProcessRequestedSyncPktQ);
        if (numPackets == 0)
        {
            UnLockQueue(&ProcessRequestedSyncPktQ);
            return NO_PACKET;
        }
        else
        {
            sizeofPacket = HWDequeue(&ProcessRequestedSyncPktQ,&packet);       // actually HWDequeue it
            dbgAssert(sizeofPacket > 0);
            copypacket = memAlloc(sizeofPacket,"cp(copypacket)",Pyrophoric);
            memcpy(copypacket,packet,sizeofPacket);
            UnLockQueue(&ProcessRequestedSyncPktQ);

            dbgAssert(((HWPacketHeader *)copypacket)->type == PACKETTYPE_REQUESTEDSYNC);
            ((HWPacketHeader *)copypacket)->type = PACKETTYPE_SYNC;

            if (((HWPacketHeader *)copypacket)->frame == explicitlyRequestingTo)
                explicitlyRequestingPackets = FALSE;
            else if (((HWPacketHeader *)copypacket)->frame == ((udword)-1))
            {
                explicitlyRequestingPackets = FALSE;        // we received fake sync packet because the one we wanted
                                                            // wasn't present.
                memFree(copypacket);
                return NO_PACKET;
            }

            goto gotsyncpkt;
        }
    }

    LockQueue(&ProcessSyncPktQ);
    numPackets = queueNumberEntries(ProcessSyncPktQ);
    if (numPackets == 0)
    {
        UnLockQueue(&ProcessSyncPktQ);
        return NO_PACKET;
    }
    else
    {
        sizeofPacket = Peekqueue(&ProcessSyncPktQ,&packet);
        dbgAssert(sizeofPacket > 0);
        for (;;)
        {
            if (((HWPacketHeader *)packet)->from != receiveSyncPacketsFrom)
            {
                goto getnextpacket;
            }

            if (((HWPacketHeader *)packet)->frame < receivedPacketNumber)
            {
                dbgMessagef("\nDiscarding packet %d already got it",((HWPacketHeader *)packet)->frame);
                goto getnextpacket;
            }

            if (((HWPacketHeader *)packet)->frame == receivedPacketNumber)
            {
                sizeofPacket = HWDequeue(&ProcessSyncPktQ,&packet);       // actually HWDequeue it
                dbgAssert(sizeofPacket > 0);
                break;
            }

            // frame is > receivedPacketNumber, don't HWDequeue it,
            if (!explicitlyRequestingPackets)       // and request packets receivedPacketNumber..frame-1 (but only once)
            {
                explicitlyRequestingFrom = receivedPacketNumber;
                explicitlyRequestingTo = ((HWPacketHeader *)packet)->frame - 1;

                UnLockQueue(&ProcessSyncPktQ);

                SendRequestSyncPkts(explicitlyRequestingFrom,explicitlyRequestingTo);
                explicitlyRequestingPackets = TRUE;
            }
            else
            {
                UnLockQueue(&ProcessSyncPktQ);
            }
            return NO_PACKET;

getnextpacket:
            sizeofPacket = HWDequeue(&ProcessSyncPktQ,&packet);       // actually HWDequeue it
            dbgAssert(sizeofPacket > 0);
            numPackets = queueNumberEntries(ProcessSyncPktQ);
            if (numPackets == 0)
            {
                UnLockQueue(&ProcessSyncPktQ);
                return NO_PACKET;
            }
            sizeofPacket = Peekqueue(&ProcessSyncPktQ,&packet);
            dbgAssert(sizeofPacket > 0);
        }

        copypacket = memAlloc(sizeofPacket,"cp(copypacket)",Pyrophoric);
        memcpy(copypacket,packet,sizeofPacket);
        UnLockQueue(&ProcessSyncPktQ);
gotsyncpkt:
        clProcessSyncPacket(comlayer,copypacket,sizeofPacket);
        EnterIntoLastSyncPktsQ(((HWPacketHeader *)copypacket)->frame,(HWPacketHeader *)copypacket,sizeofPacket);  // last sync packets may be used to recover game if this player takes over as captain
        if (numPackets > HIGH_WATER_MARK)
        {
            return TOO_MANY_PACKETS;
        }
        else
        {
            return PACKET_READY;
        }
    }
}

typedef struct
{
    char *qdata;
    udword qsizeof;
} QInfo;

/*-----------------------------------------------------------------------------
    Name        : captainServerTask
    Description : This is the captain server task, which every so many Hz
                  takes any command packets received, and merges them into
                  a sync packet which it transmits.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void captainServerTask(void)
{
    static QInfo *qinfos;
    static QInfo *curqinfo;
    static udword qTotalNumberEntries;
    static udword numCommands;
    static udword j;
    static HWPacketHeader *packet;
    static HWPacketHeader *thispacket;
    static ubyte *curPacketPtr;
    static udword packetlength;
    static udword datalength;
    static udword totalCommands;

    taskYield(0);

#ifndef C_ONLY
    for(;;)
#endif
    {
        taskStackSaveCond(0);
        if ( (recordFakeSendPackets) ||
            ((multiPlayerGame) && (gameIsRunning) && (IAmCaptain) && (multiPlayerGameUnderWay)) )
        {
            if (playPackets|recordFakeSendPackets)
            {
                if (universePause)
                {
                    goto donecap;
                }
            }

            numCommands = 0;

            LockQueue(&ProcessCmdPktQ);
            qTotalNumberEntries = queueNumberEntries(ProcessCmdPktQ);
            if (qTotalNumberEntries == 0)
            {
                qinfos = NULL;
            }
            else
            {
                qinfos = memAlloc(sizeof(QInfo)*qTotalNumberEntries,"qinfos",0);
            }

            while (queueNumberEntries(ProcessCmdPktQ) > 0)
            {
                curqinfo = &qinfos[numCommands];
                curqinfo->qsizeof = HWDequeue(&ProcessCmdPktQ,(ubyte **)&curqinfo->qdata);
                dbgAssert(curqinfo->qsizeof > 0);
                dbgAssert(curqinfo->qdata != NULL);
                dbgAssert(numCommands < qTotalNumberEntries);
                numCommands++;
            }

            if (numCommands == 0)
            {
                packetlength = sizeof(HWPacketHeader);
                packet = memAlloc(packetlength,"emptysyncpkt",0);

                packet->numberOfCommands = 0;
            }
            else if (numCommands == 1)
            {
                packetlength = qinfos[0].qsizeof;
                packet = memAlloc(packetlength,"onesync",0);
                memcpy(packet,qinfos[0].qdata,packetlength);

                dbgAssert(packet->type == PACKETTYPE_COMMAND);
                packet->numberOfCommands = 1;
            }
            else
            {
                // received more than 1 command packet, so we must concatenate them into one sync packet
                packetlength = sizeof(HWPacketHeader);
                totalCommands = 0;
                for (j=0;j<numCommands;j++)
                {
                    thispacket = (HWPacketHeader *)qinfos[j].qdata;
                    datalength = qinfos[j].qsizeof - sizeof(HWPacketHeader);

                    dbgAssert(thispacket->type == PACKETTYPE_COMMAND);
                    dbgAssert(datalength > 0);
                    dbgAssert(thispacket->numberOfCommands > 0);

                    packetlength += datalength;
                    totalCommands += thispacket->numberOfCommands;
                }

                packet = memAlloc(packetlength,"concatsyncpkt",0);

                packet->type = PACKETTYPE_SYNC;
                packet->numberOfCommands = (uword)totalCommands;

                curPacketPtr = ((ubyte *)packet) + sizeof(HWPacketHeader);

                for (j=0;j<numCommands;j++)
                {
                    thispacket = (HWPacketHeader *)qinfos[j].qdata;
                    datalength = qinfos[j].qsizeof - sizeof(HWPacketHeader);

                    memcpy(curPacketPtr,((ubyte *)thispacket) + sizeof(HWPacketHeader),datalength);
                    curPacketPtr += datalength;
                }
            }
            if (qinfos != NULL)
            {
                memFree(qinfos);
            }
            UnLockQueue(&ProcessCmdPktQ);

            packet->type = PACKETTYPE_SYNC;
            packet->from = (uword)sigsPlayerIndex;
            packet->frame = sentPacketNumber++;
#if SYNC_CHECK
            netcheckFillInChecksum(packet);
#endif

            if (recordFakeSendPackets)
            {
                ReceivedSyncPacketCB((ubyte *)packet,packetlength);          // and myself too.
                memFree(packet);
            }
            else
            {
                if (packetBroadcastSync((ubyte *)packet,packetlength))
                {
                    memFree(packet);
                }
            }
donecap:;
        }
        taskStackRestoreCond();
        taskYield(0);
    }

    taskExit();
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : SendTransferCaptaincyPacket
    Description : sends a TransferCaptaincyPacket of subtype subtype with custominfo
    Inputs      : playerIndex,  >= 0 for player, -1 for broadcast, -2 for broadcast including myself
                  subtype, custominfo
    Outputs     :
    Return      : none
----------------------------------------------------------------------------*/
void SendTransferCaptaincyPacket(sdword playerIndex,uword subtype,udword misc,CaptaincyCustomInfo *custominfo)
{
    TransferCaptaincyPacket packet;
    sdword size;

    if (sigsNumPlayers < 2)
    {
        return;
    }

    packet.packetheader.type = PACKETTYPE_TRANSFERCAPTAINCY;
    packet.packetheader.from = (uword)sigsPlayerIndex;
    packet.packetheader.frame = misc;
    packet.packetheader.numberOfCommands = 0;

    dbgAssert(subtype < XFERCAP_NUM);
    packet.subtype = subtype;
    if (custominfo)
    {
        packet.custominfo = *custominfo;
        packet.subtype |= XFERCAP_CUSTOMINFO_PRESENT;
        size = sizeof(TransferCaptaincyPacket);
    }
    else
    {
        size = sizeof(TransferCaptaincyPacketSmall);
    }

    if (playerIndex >= 0)
    {
        dbgAssert(playerIndex < sigsNumPlayers);
        titanAnyoneSendPointMessage(playerIndex,(ubyte *)&packet,size);
    }
    else
    {
        // -1 or lower
        titanAnyoneSendBroadcastMessage((ubyte *)&packet,size);

        if (playerIndex == -2)
        {
            // even send to myself!
            ReceivedTransferCaptaincyPacketCB((ubyte *)&packet,size);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : SendNonCaptainReadyPacket
    Description : sends the "Ready to play packet (I am not the captain)" packet to
                  indicate user is ready to play.
    Inputs      : playerIndex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SendNonCaptainReadyPacket(udword playerIndex)
{
    HWPacketHeader packet;

    dbgAssert(!IAmCaptain);
    dbgAssert(!CaptainNotDefined);

    packet.type = PACKETTYPE_NONCAPTAINREADY;
    packet.from = (uword)playerIndex;
    packet.frame = playerIndex;
    packet.numberOfCommands = 0;

    titanSendPointMessage(captainIndex,(ubyte *)&packet,sizeof(HWPacketHeader));
}

/*-----------------------------------------------------------------------------
    Name        : SendDroppingOutOfLoad
    Description : sends the "I'm dropping out of loading" packet to
                  everyone else
    Inputs      : my playerIndex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SendDroppingOutOfLoad(udword playerIndex)
{
    HWPacketHeader packet;

    packet.type = PACKETTYPE_DROPPINGOUTOFLOAD;
    packet.from = (uword)playerIndex;
    packet.frame = playerIndex;
    packet.numberOfCommands = 0;

    titanSendBroadcastMessage((ubyte *)&packet,sizeof(HWPacketHeader));
}

/*-----------------------------------------------------------------------------
    Name        : SendCheatDetect
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SendCheatDetect(void)
{
    HWPacketHeader packet;

    packet.type = PACKETTYPE_CHEATDETECT;
    packet.from = (uword)sigsPlayerIndex;
    packet.frame = blobSyncErrFrame;
    packet.numberOfCommands = 0;

    titanSendBroadcastMessage((ubyte *)&packet,sizeof(HWPacketHeader));
}

/*-----------------------------------------------------------------------------
    Name        : SendInGameQuittingPacket
    Description : sends the "In Game I'm quitting" packet to
                  everyone else
    Inputs      : my playerIndex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SendInGameQuittingPacket(udword playerIndex)
{
    HWPacketHeader packet;

    packet.type = PACKETTYPE_INGAMEQUITING;
    packet.from = (uword)playerIndex;
    packet.frame = playerIndex;
    packet.numberOfCommands = 0;

    titanSendBroadcastMessage((ubyte *)&packet,sizeof(HWPacketHeader));
}

void SendRequestSyncPkts(udword frompacketnum,udword topacketnum)
{
    RequestSyncPacketsPacket packet;

    dbgAssert(!IAmCaptain);
    dbgAssert(!CaptainNotDefined);

    dbgAssert(topacketnum >= frompacketnum);

    packet.packetheader.type = PACKETTYPE_REQUESTSYNCPKTS;
    packet.packetheader.from = (uword)sigsPlayerIndex;
    packet.packetheader.frame = frompacketnum;
    packet.packetheader.numberOfCommands = 0;
    packet.topacketnum = topacketnum;

    titanSendPointMessage(captainIndex,(ubyte *)&packet,sizeof(RequestSyncPacketsPacket));
}

void SendLagPacket(udword to, ubyte *packet)
{
    titanSendPointMessage(to, packet, sizeof(LagPacket));
}

/*-----------------------------------------------------------------------------
    Name        : SendHorseRacePacket
    Description : Sends A packet Update Regarding a players horse race status

    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SendHorseRacePacket(ubyte *packet,udword sizeofpacket)
{
    if(!multiPlayerGame) return;
    if (CaptainNotDefined) return;
    if(IAmCaptain)
    {
        titanSendBroadcastMessage(packet,sizeofpacket);         // send to everyone else
        recievedHorsePacketCB(packet,sizeofpacket);     // I receive it too
    }
    else
    {
        titanSendPointMessage(captainIndex,packet,sizeofpacket);
    }
}
/*-----------------------------------------------------------------------------
    Name        : BroadCastHorseRacePacket
    Description : Sends HorseRace status to EVERYONE except captain about EVERYONE

    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void BroadCastHorseRacePacketFromCaptain(ubyte *packet, udword sizeofpacket)
{
    titanSendBroadcastMessage(packet,sizeofpacket);
}

/*-----------------------------------------------------------------------------
    Name        : SendChatPacketPacket
    Description : Sends a packet to all users flagged as TRUE with a packet of CHAT data

    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SendChatPacketPacket(ChatPacket *packet, udword sizeofpacket,udword users)
{
    sdword i;

    if(!multiPlayerGame) return;
    if (CaptainNotDefined)
    {
        return;
    }

    dbgAssert(!(PLAYER_MASK(sigsPlayerIndex) & users));        //shouldn't be sending a message to local player...thats dumb

    packet->bounced = FALSE;

    if(IAmCaptain)
    {
        for (i=0;i<sigsNumPlayers;i++)
        {
            if (i != captainIndex)
            {
                if (bitTest(packet->users,PLAYER_MASK(i)))
                {
                    titanSendPointMessage(i, (ubyte *)packet, sizeofpacket);
                }
            }
        }
    }
    else
    {
        titanSendPointMessage(captainIndex,(ubyte *)packet,sizeofpacket);
    }
}

void BroadCastChatPacketFromCapatain(ChatPacket *packet, udword sizeofpacket)
{
    sdword i;

    if (CaptainNotDefined)
    {
        return;
    }

    dbgAssert(IAmCaptain);

    if (packet->bounced)
    {
        goto justreceiveit;
    }
    packet->bounced = TRUE;

    for (i=0;i<sigsNumPlayers;i++)
    {
        if (i != captainIndex)
        {
            if (bitTest(packet->users,PLAYER_MASK(i)))
            {
                titanSendPointMessage(i, (ubyte *)packet, sizeofpacket);
            }
        }
    }

justreceiveit:
    if (bitTest(packet->users,PLAYER_MASK(captainIndex)))
    {
        recievedChatPacketCB((ubyte *)packet, sizeofpacket);
    }

}

/*-----------------------------------------------------------------------------
    Name        : netCheck
    Description : prints various network statistics
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void netCheck(void)
{
    udword rsyncpkts;
    udword rcmdpkts;
    udword rsyncoverruns;
    udword rcmdoverruns;

    if (unknownPackets != printedUnknownPackets)
    {
        printedUnknownPackets = unknownPackets;
        dbgMessagef("\nUnknown packets %d",printedUnknownPackets);
    }

    rsyncpkts = queueNumberEntries(ProcessSyncPktQ);
    rcmdpkts = queueNumberEntries(ProcessCmdPktQ);
    rsyncoverruns = queueNumberOverruns(ProcessSyncPktQ);
    rcmdoverruns = queueNumberOverruns(ProcessCmdPktQ);

    if (syncpkts != rsyncpkts)
    {
        syncpkts = rsyncpkts;
    }

    if (cmdpkts != rcmdpkts)
    {
        cmdpkts = rcmdpkts;
    }

    if (syncoverruns != rsyncoverruns)
    {
        syncoverruns = rsyncoverruns;
        dbgMessagef("\nSync Q Overruns: %d",syncoverruns);
    }

    if (cmdoverruns != rcmdoverruns)
    {
        cmdoverruns = rcmdoverruns;
        dbgMessagef("\nCmd Q Overruns: %d",cmdoverruns);
    }
}

void CaptaincyChangedNotify(void)
{
    KeepAliveStartTimers();     // restart keepalive timers after captaincy changes
}

/*=============================================================================
    Keep Alive functions
=============================================================================*/

void UpdateDroppedOutStatuses();
void SendKeepAlivePacket();
void BroadcastAliveStatusPacket();

void KeepAliveReset(void)
{
    sdword i;

    TTimerInit(&SendIAmAliveTimer);
    TTimerInit(&SendAliveStatusTimer);

    LockMutex(AliveTimeoutStatusesMutex);
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        TTimerInit(&AliveTimeoutTimers[i]);
        AliveStatuses[i] = ALIVESTATUS_ALIVE;
        HaveKilledPlayerDueToDropout[i] = FALSE;
    }
    UnLockMutex(AliveTimeoutStatusesMutex);

    KeepAliveCalledFirstTime = FALSE;
}

void KeepAliveStartup(void)
{
    AliveTimeoutStatusesMutex = gameCreateMutex();
    KeepAliveReset();
}

void KeepAliveShutdown(void)
{
    gameCloseMutex(AliveTimeoutStatusesMutex);
}

void KeepAliveStartTimers(void)
{
    sdword i;

    TTimerStart(&SendIAmAliveTimer,KEEPALIVE_SEND_IAMALIVE_TIME);
    TTimerStart(&SendAliveStatusTimer,KEEPALIVE_SEND_ALIVESTATUS_TIME);

    LockMutex(AliveTimeoutStatusesMutex);
    for (i=0;i<sigsNumPlayers;i++)
    {
        TTimerStart(&AliveTimeoutTimers[i],KEEPALIVE_IAMALIVE_TIMEOUT);
    }
    UnLockMutex(AliveTimeoutStatusesMutex);
}

void KillAnyDroppedOutPlayers()
{
    sdword i,j;
    udword killmask = 0;

    for (i=0;i<sigsNumPlayers;i++)
    {
        if (playerHasDroppedOutOrQuit(i) && (!HaveKilledPlayerDueToDropout[i]))
        {
            HaveKilledPlayerDueToDropout[i] = TRUE;
            killmask |= (1<<i);

            // are there any computer players on his machine we should kill too?

            for (j=sigsNumPlayers;j<universe.numPlayers;j++)
            {
                if (ComputerPlayerOn[j] == i)
                {
                    killmask |= (1<<j);
                }
            }
        }
    }

    if (!killmask)
    {
        return;
    }

    clSendPlayerDropped(killmask);
}

void KeepAliveDropPlayerCB(sdword playerindex)
{
    LockMutex(AliveTimeoutStatusesMutex);
    if (AliveStatuses[playerindex] != ALIVESTATUS_DROPPEDOUT)
    {
        playersDropped[numPlayerDropped] = playerindex;
        numPlayerDropped++;
        printTimeout = universe.totaltimeelapsed;
        captaincyLog(FALSE,"Player %d was dropped",playerindex);
    }
    AliveStatuses[playerindex] = ALIVESTATUS_DROPPEDOUT;
    UnLockMutex(AliveTimeoutStatusesMutex);
}

void KeepAliveDontDropPlayerCB(sdword playerindex)
{
    LockMutex(AliveTimeoutStatusesMutex);
    TTimerReset(&AliveTimeoutTimers[playerindex]);
    AliveStatuses[playerindex] = ALIVESTATUS_ALIVE;
    UnLockMutex(AliveTimeoutStatusesMutex);
}

void KeepAliveUpdate(void)
{
    if (CaptainNotDefined)
    {
        return;
    }

    if (playPackets)
    {
        return;
    }

    if (!KeepAliveCalledFirstTime)
    {
        KeepAliveStartTimers();
        KeepAliveCalledFirstTime = TRUE;
    }

    if (!IAmCaptain)
    {
        if (TTimerUpdate(&SendIAmAliveTimer))
        {
            TTimerReset(&SendIAmAliveTimer);
            SendKeepAlivePacket();
        }

        UpdateDroppedOutStatuses();
    }
    else
    {
        sdword i;

        LockMutex(AliveTimeoutStatusesMutex);
        for (i=0;i<sigsNumPlayers;i++)
        {
            if (i == sigsPlayerIndex)
            {
                AliveStatuses[i] = ALIVESTATUS_ALIVE;     // I am alive if I'm here
                continue;
            }

            if (playersReadyToGo[i] == PLAYER_QUIT)
            {
                continue;
            }

            if (TTimerUpdate(&AliveTimeoutTimers[i]))
            {
                // timed out, set to
                if (AliveStatuses[i] == ALIVESTATUS_ALIVE)
                {
                    if (universe.players[i].playerState == PLAYER_DEAD)
                    {
                        // player already dead, no need to display drop out box
                        AliveStatuses[i] = ALIVESTATUS_DROPPEDOUT;
                    }
                    else
                    {
                        if ((utyPlayerDroppedDisplay == -1) && (!(smZoomingIn|smZoomingOut)))
                        {
                            utyStartDroppedDialog(i);
                            AliveStatuses[i] = ALIVESTATUS_QUERYINGDROPOUT;
                        }
                        else
                        {
                            AliveStatuses[i] = ALIVESTATUS_WAITQUERYDROPOUTLATER;
                        }
                    }
                }
            }
        }

        if ((utyPlayerDroppedDisplay == -1) && (!(smZoomingIn|smZoomingOut)))
        {
            for (i=0;i<sigsNumPlayers;i++)
            {
                if (i == sigsPlayerIndex)
                {
                    continue;
                }

                if (playersReadyToGo[i] == PLAYER_QUIT)
                {
                    continue;
                }

                if (AliveStatuses[i] == ALIVESTATUS_WAITQUERYDROPOUTLATER)
                {
                    if (universe.players[i].playerState == PLAYER_DEAD)
                    {
                        // player already dead, no need to display drop out box
                        AliveStatuses[i] = ALIVESTATUS_DROPPEDOUT;
                    }
                    else
                    {
                        utyStartDroppedDialog(i);
                        AliveStatuses[i] = ALIVESTATUS_QUERYINGDROPOUT;
                        break;
                    }
                }
            }
        }

        UnLockMutex(AliveTimeoutStatusesMutex);

        if (TTimerUpdate(&SendAliveStatusTimer))
        {
            TTimerReset(&SendAliveStatusTimer);
            BroadcastAliveStatusPacket();
        }

        UpdateDroppedOutStatuses();
        KillAnyDroppedOutPlayers();
    }
}

void SendKeepAlivePacket()
{
    HWPacketHeader packet;

    dbgAssert(!CaptainNotDefined);
    dbgAssert(!IAmCaptain);

    packet.type = PACKETTYPE_IAMALIVE;
    packet.from = (uword)sigsPlayerIndex;
    packet.frame = sigsPlayerIndex;
    packet.numberOfCommands = 0;

    titanSendPointMessage(captainIndex,(ubyte *)&packet,sizeof(HWPacketHeader));
}

void BroadcastAliveStatusPacket()
{
    AliveStatusPacket packet;

    dbgAssert(!CaptainNotDefined);
    dbgAssert(IAmCaptain);

    packet.packetheader.type = PACKETTYPE_ALIVESTATUS;
    packet.packetheader.from = (uword)sigsPlayerIndex;
    packet.packetheader.frame = sigsPlayerIndex;
    packet.packetheader.numberOfCommands = 0;

    memcpy(packet.alivestatus,AliveStatuses,sizeof(AliveStatuses));

    titanSendBroadcastMessage((ubyte *)&packet,sizeof(packet));     // send to everyone else
}

void UpdateDroppedOutStatuses()
{
    sdword i;

    if (playPackets)
    {
        return;
    }

    for (i=0;i<sigsNumPlayers;i++)
    {
        if (AliveStatuses[i] == ALIVESTATUS_DROPPEDOUT)
        {
            if (playersReadyToGo[i] != PLAYER_QUIT)
            {
                playersReadyToGo[i] = PLAYER_DROPPED_OUT;
            }
        }
        else if (AliveStatuses[i] == ALIVESTATUS_ALIVE)
        {
            if (playersReadyToGo[i] != PLAYER_QUIT)
            {
                playersReadyToGo[i] = TRUE;
            }
        }
    }
}

void receivedAliveStatusPacket(AliveStatusPacket *packet,udword sizeofPacket)
{
    dbgAssert(packet->packetheader.type == PACKETTYPE_ALIVESTATUS);

    LockMutex(AliveTimeoutStatusesMutex);
    memcpy(AliveStatuses,packet->alivestatus,sizeof(AliveStatuses));
#if 0       // if captain tells us we're dead, we're dead- and will be receiving a kill player message
    if (AliveStatuses[sigsPlayerIndex] == ALIVESTATUS_DROPPEDOUT)
    {
        sdword i;
        // if they tell me I'm dead then I'm alive but THEY are all dead
        for (i=0;i<sigsNumPlayers;i++)
        {
            if (i == sigsPlayerIndex)
            {
                AliveStatuses[i] = ALIVESTATUS_ALIVE;
            }
            else
            {
                if (AliveStatuses[i] != ALIVESTATUS_DROPPEDOUT)
                {
                    playersDropped[numPlayerDropped] = i;
                    numPlayerDropped++;
                    printTimeout = universe.totaltimeelapsed;
                }
                AliveStatuses[i] = ALIVESTATUS_DROPPEDOUT;
            }
        }
    }
#endif
    UnLockMutex(AliveTimeoutStatusesMutex);
}

void receivedIAmAlivePacket(HWPacketHeader *packet,udword sizeofPacket)
{
    uword aliveplayer;

    dbgAssert(packet->type == PACKETTYPE_IAMALIVE);

    aliveplayer = packet->from;

    if (aliveplayer < sigsNumPlayers)
    {
        LockMutex(AliveTimeoutStatusesMutex);
        if (AliveStatuses[aliveplayer] == ALIVESTATUS_ALIVE)        // keep alive alive players
        {
            TTimerReset(&AliveTimeoutTimers[aliveplayer]);
        }
        UnLockMutex(AliveTimeoutStatusesMutex);
    }
}

void clPlayerDropped(udword playerMask,udword verify)
{
    sdword i;
    bool aPlayerDied = FALSE;

    if ((playerMask ^ verify) != KILLDROPPEDOUTPLAYER_VERIFY)
    {
        return;
    }

    for (i=0;i<universe.numPlayers;i++)
    {
        if (playerMask & (1<<i))
        {
            if (universe.players[i].playerState != PLAYER_DEAD)
            {
                univKillPlayer(i,PLAYERKILLED_DROPPEDOUT);
                aPlayerDied = TRUE;
            }
        }
    }

    if (aPlayerDied)
    {
        CheckPlayerWin();
    }
}

