/*=============================================================================
    Name    : GameChat.h
    Purpose : This file has all of the prototypes fo the GameChat logic.

    Created 7/23/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef ___GAMECHAT_H
#define ___GAMECHAT_H

#include "Types.h"
#include "MultiplayerGame.h"

#define GC_NORMALMESSAGE        1
#define GC_WHISPEREDMESSAGE     2
#define GC_TEXTMESSAGE          3
#define GC_BUFFERSTART          4
#define GC_BUFFEREND            5
#define GC_WRAPMESSAGE          6

typedef struct chathistory
{
    Node            link;
    char            chatstring[MAX_CHATSTRING_LENGTH];
    char            userName[MAX_USERNAME_LENGTH];
    sdword          playerindex;
    udword          messageType;
    udword          col;
    sdword          indent;
}
chathistory;

extern bool gcRunning;
extern bool ViewingBuffer;

void gcRemoveAmpersands(char *dest, char *source);
void gcChatEntryStart(bool toAllies);
void gcRUTransferStart(uword playertosendto);
void gcProcessGameChatPacket(struct ChatPacket *packet);
void gcProcessGameTextMessage(char *message, udword col);
void gcPollForNewChat(void);

void gcPageDownProcess(void);
void gcPageUpProcess(void);
void gcCancelViewingBuffer(void);

void gcStartup(void);
void gcShutdown(void);


#endif
