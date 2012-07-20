#ifndef ___CAPTAINCY_H
#define ___CAPTAINCY_H
/*=============================================================================
    Name    : Captaincy.h
    Purpose : Definitions for Captaincy.c - transfering captaincy

    Created 98/10/06 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
//#include "CommandNetwork.h"

typedef enum CaptaincyEvent {
    CAPEVENT_STARTGAME,
    CAPEVENT_TIMEOUT1,
    CAPEVENT_TIMEOUT2,
    CAPEVENT_TWAITFORPAUSEACKS,
    CAPEVENT_GIVEUP_CAPTAINCY_NOTICE,
    CAPEVENT_CAPTAIN_PROPOSAL,
    CAPEVENT_PAUSE_TRANSFERRING_CAPTAIN,
    CAPEVENT_PAUSE_ACK,
    CAPEVENT_PAUSE_YOU_BE_CAPTAIN,
    CAPEVENT_I_AM_NEW_CAPTAIN,
    CAPEVENT_RECEIVED_SYNC_PKT
} CaptaincyEvent;

struct CaptaincyCustomInfo;

void TransferCaptaincyGameEnded(void);
void TransferCaptaincyGameStarted(void);

void TransferCaptaincyStateMachine(CaptaincyEvent event,uword from,udword misc,struct CaptaincyCustomInfo *customInfo);
void TransferCaptaincySyncPacketReceivedNotify(void);

void TransferCaptaincyUpdate(void);

void TimeoutTimerInitAll(void);
void TimeoutTimerCloseAll(void);
void TimeoutTimerUpdateAll(void);

void GiveUpCaptaincy(void);

void captaincyLogInit(void);
void captaincyLog(bool echotoscreen,char *format, ...);

/*=============================================================================
    Variables:
=============================================================================*/

extern bool transferCaptaincyDisabled;
extern bool captaincyLogEnable;

extern bool printCaptainMessage;

extern real32 T1_Timeout;
extern real32 T2_Timeout;
extern real32 TWAITFORPAUSEACKS_Timeout;

#endif

