
/*=============================================================================
    Name    : chatting.h
    Purpose : chatting prototypes and defines

    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef CHATTING_H
#define CHATTING_H

#include "Types.h"

// masks for players
#define PLAYER_MASK(player)         (1<<player)
#define ALL_PLAYER_MASK             (0x000000FF)
#define OTHER_PLAYERS_MASK          ((0x000000FF)^(1<<sigsPlayerIndex))
#define MAX_CHAT_MESSAGE_LENGTH     255

// packet type, either a chat packet or an alliance packet.
#define CHAT_PACKET                 1
#define ALLIANCE_PACKET             2

void recievedChatPacketCB(ubyte *packet,udword sizeofPacket);
void sendChatMessage(udword users, char *message, uword player_index);
void sendAllianceRequest(udword user, uword player_index, uword type, ubyte data);
void sendRUTransfer(udword user, sdword RUAmount);

#endif