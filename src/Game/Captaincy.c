/*=============================================================================
    Name    : Captaincy.c
    Purpose : Provides functionality for transfering the captaincy in network games

    Created 98/10/06 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
    #include <sys/time.h>
#endif

#include "SDL.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "Globals.h"
#include "CommandNetwork.h"
#include "Captaincy.h"
#include "utility.h"
#include "CommandWrap.h"
#include "File.h"


/*
    Format for captaincy messages:

#define XFERCAP_GIVEUP_CAPTAINCY_NOTICE     0
    misc - player who is giving up captaincy

#define XFERCAP_CAPTAIN_PROPOSAL            1
    misc - player being proposed as captain

#define XFERCAP_PAUSE_TRANSFERRING_CAPTAIN  2
    misc - player who is candidate for captain

#define XFERCAP_PAUSE_ACK                   3
    misc - last received sync packet
    from - who message is from

#define XFERCAP_PAUSE_YOU_BE_CAPTAIN        4
    misc - who message is from
    custominfo - last received sync packets from all candidates

#define XFERCAP_I_AM_NEW_CAPTAIN            5
    misc - new captain you should acknowledge
*/

extern Uint32 utyTimerDivisor;

typedef struct TimeoutTimer
{
    bool enabled;
    Uint32 timerLast;
    udword timeoutTicks;
} TimeoutTimer;

sdword TimedOutWaitingForPauseAcks = 0;

#define T1                      0
#define T2                      1
#define T3                      2
#define TWAITFORPAUSEACKS       3
#define NUMBER_TIMEOUTTIMERS    4

real32 T1_Timeout = 15.0f;
real32 T2_Timeout = 5.0f;
real32 TWAITFORPAUSEACKS_Timeout = 5.0f;
sdword TimedOutWaitingForPauseAcksGiveUpAfterNumTimes = 2;

#define CAPTAINCYLOGFILE "captaincylog.txt"

TimeoutTimer timeoutTimers[NUMBER_TIMEOUTTIMERS];

bool transferCaptaincyDisabled = FALSE;

udword pausedNextPacketToReceive[MAX_MULTIPLAYER_PLAYERS] = { 0,0,0,0,0,0,0,0 };

bool printCaptainMessage = FALSE;

void AcknowledgeNewCaptain(sdword newcaptainIndex);

void TimeoutTimerDisable(sdword timer);
void TimeoutTimerReset(sdword timer);
void TimeoutTimerStart(sdword timer,real32 timeout);
void TimeoutTimerTimedOut(sdword timer);
void TimeoutTimerUpdateAll(void);
void TimeoutTimerCloseAll(void);
void TimeoutTimerInitAll(void);

void captaincyLogInit(void)
{
    if (logEnable)
    {
        logfileClear(CAPTAINCYLOGFILE);
    }
}

void resetPausedNextPacketToReceive()
{
    sdword i;

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        pausedNextPacketToReceive[i] = 0;
    }
}

void captaincyLog(bool echotoscreen,char *format, ...)
{
    char buffer[200];
    va_list argList;
    va_start(argList, format);                              //get first arg
    vsprintf(buffer, format, argList);                      //prepare output string
    va_end(argList);

    dbgMessage("\n");
    dbgMessage(buffer);
    if (echotoscreen) clCommandMessage(buffer);

    if (logEnable)
    {
#ifdef _WIN32
        SYSTEMTIME systime;
        char buffer2[200];

        GetSystemTime(&systime);
        sprintf(buffer2,"\n%2d:%2d:%2d %s",systime.wHour,systime.wMinute,systime.wSecond,buffer);
#else   /* TEE...VEE...MIND! */
        struct timeval tv;
        long tv_hour, tv_minute, tv_second;
        char buffer2[200];

        gettimeofday(&tv, 0);
        tv_hour   = (tv.tv_sec % 86400) / 3600;
        tv_minute = (tv.tv_sec %  3600) /   60;
        tv_second = (tv.tv_sec %    60);
        sprintf(buffer2, "\n%2ld:%2ld:%2ld %s", tv_hour, tv_minute, tv_second,
            buffer);
#endif
        logfileLog(CAPTAINCYLOGFILE,buffer2);
    }
}

void TransferCaptaincySyncPacketReceivedNotify(void)
{
    if ((transferCaptaincyDisabled) || (sigsNumPlayers < 2))
    {
        return;
    }
    TimeoutTimerReset(T1);
    TransferCaptaincyStateMachine(CAPEVENT_RECEIVED_SYNC_PKT,0,0,NULL);
}

void TransferCaptaincyGameEnded(void)
{
    if (IAmCaptain)
    {
        SendTransferCaptaincyPacket(-1,XFERCAP_GIVEUP_CAPTAINCY_NOTICE,sigsPlayerIndex,NULL);
    }
}

void TimeoutTimerInitAll(void)
{
    sdword i;

    for (i=0;i<NUMBER_TIMEOUTTIMERS;i++)
    {
        timeoutTimers[i].enabled = FALSE;
    }
}

void TimeoutTimerCloseAll(void)
{
    sdword i;

    for (i=0;i<NUMBER_TIMEOUTTIMERS;i++)
    {
        timeoutTimers[i].enabled = FALSE;
    }
}

void TimeoutTimerUpdateAll(void)
{
    sdword i;

    for (i=0;i<NUMBER_TIMEOUTTIMERS;i++)
    {
        if (timeoutTimers[i].enabled)
        {
            Uint32 timerval;
            Uint32 difference;
            udword nTicks;

            timerval = SDL_GetTicks();

            difference = timerval - timeoutTimers[i].timerLast;
            nTicks = (udword)(difference / utyTimerDivisor);

            if (nTicks >= timeoutTimers[i].timeoutTicks)
            {
                TimeoutTimerTimedOut(i);
            }
        }
    }
}

void TimeoutTimerStart(sdword timer,real32 timeout)
{
    Uint32 timerval;

    timerval = SDL_GetTicks();

    timeoutTimers[timer].enabled = TRUE;
    timeoutTimers[timer].timerLast = timerval;
    timeoutTimers[timer].timeoutTicks = (udword) (timeout * UTY_TimerResloutionMax);
}

void TimeoutTimerReset(sdword timer)
{
    Uint32 timerval;

    if (!timeoutTimers[timer].enabled)
    {
        return;
    }

    timerval = SDL_GetTicks();

    timeoutTimers[timer].timerLast = timerval;
}

void TimeoutTimerDisable(sdword timer)
{
    timeoutTimers[timer].enabled = FALSE;
}

void TimeoutTimerTimedOut(sdword timer)
{
    switch (timer)
    {
        case T1:
            TimeoutTimerReset(timer);
            TransferCaptaincyStateMachine(CAPEVENT_TIMEOUT1,0,0,NULL);
            break;

        case T2:
            TimeoutTimerReset(timer);
            TransferCaptaincyStateMachine(CAPEVENT_TIMEOUT2,0,0,NULL);
            break;

        case TWAITFORPAUSEACKS:
            TimeoutTimerReset(timer);
            TransferCaptaincyStateMachine(CAPEVENT_TWAITFORPAUSEACKS,0,0,NULL);
            break;

        default:
            dbgFatalf(DBG_Loc,"Unknown timer %d timed out",timer);
            break;
    }
}

void ProcessTransferCaptaincyPacket(TransferCaptaincyPacket *packet)
{
    CaptaincyCustomInfo *custominfo = NULL;
    udword misc = packet->packetheader.frame;
    uword from = packet->packetheader.from;
    if (packet->subtype & XFERCAP_CUSTOMINFO_PRESENT)
    {
        custominfo = &packet->custominfo;
    }

    switch (packet->subtype & XFERCAP_SUBTYPE_MASK)
    {
        case XFERCAP_GIVEUP_CAPTAINCY_NOTICE:
            if (misc == captainIndex)       // only pass through if active captain is giving up captaincy
            {
                captainIndex = -1;
                receiveSyncPacketsFrom = -1;        // don't accept packets from me
                TransferCaptaincyStateMachine(CAPEVENT_GIVEUP_CAPTAINCY_NOTICE,from,misc,custominfo);
            }
            break;

        case XFERCAP_CAPTAIN_PROPOSAL:
            TransferCaptaincyStateMachine(CAPEVENT_CAPTAIN_PROPOSAL,from,misc,custominfo);
            break;

        case XFERCAP_PAUSE_TRANSFERRING_CAPTAIN:
            TransferCaptaincyStateMachine(CAPEVENT_PAUSE_TRANSFERRING_CAPTAIN,from,misc,custominfo);
            break;

        case XFERCAP_PAUSE_ACK:
            TransferCaptaincyStateMachine(CAPEVENT_PAUSE_ACK,from,misc,custominfo);
            break;

        case XFERCAP_PAUSE_YOU_BE_CAPTAIN:
            TransferCaptaincyStateMachine(CAPEVENT_PAUSE_YOU_BE_CAPTAIN,from,misc,custominfo);
            break;

        case XFERCAP_I_AM_NEW_CAPTAIN:
            AcknowledgeNewCaptain(misc);
            TransferCaptaincyStateMachine(CAPEVENT_I_AM_NEW_CAPTAIN,from,misc,custominfo);
            break;

        default:
            dbgFatalf(DBG_Loc,"Invalid transfer captaincy packet");
            break;
    }
}

void TransferCaptaincyUpdate(void)
{
    udword numPackets;
    udword sizeofPacket;
    ubyte *packet;
    ubyte *copypacket;

    if ((transferCaptaincyDisabled) || (sigsNumPlayers < 2))
    {
        return;
    }

    TimeoutTimerUpdateAll();

    LockQueue(&ProcessCaptaincyPktQ);
    numPackets = queueNumberEntries(ProcessCaptaincyPktQ);

    while (numPackets > 0)
    {
        sizeofPacket = HWDequeue(&ProcessCaptaincyPktQ,&packet);
        dbgAssert(sizeofPacket > 0);
        copypacket = memAlloc(sizeofPacket,"cp(miscpacket)",Pyrophoric);
        memcpy(copypacket,packet,sizeofPacket);
        UnLockQueue(&ProcessCaptaincyPktQ);

        dbgAssert(((HWPacketHeader *)copypacket)->type == PACKETTYPE_TRANSFERCAPTAINCY);
        ProcessTransferCaptaincyPacket((TransferCaptaincyPacket *)copypacket);
        memFree(copypacket);

        LockQueue(&ProcessCaptaincyPktQ);
        numPackets = queueNumberEntries(ProcessCaptaincyPktQ);
    }

    UnLockQueue(&ProcessCaptaincyPktQ);

#if 0
    if (numPackets == 0)
    {
        UnLockQueue(&ProcessCaptaincyPktQ);
    }
    else
    {
        sizeofPacket = HWDequeue(&ProcessCaptaincyPktQ,&packet);
        dbgAssert(sizeofPacket > 0);
        copypacket = memAlloc(sizeofPacket,"cp(miscpacket)",Pyrophoric);
        memcpy(copypacket,packet,sizeofPacket);
        UnLockQueue(&ProcessCaptaincyPktQ);

        dbgAssert(((HWPacketHeader *)copypacket)->type == PACKETTYPE_TRANSFERCAPTAINCY);
        ProcessTransferCaptaincyPacket((TransferCaptaincyPacket *)copypacket);
        memFree(copypacket);
    }
#endif
}

void ProposeNewCaptain(void)
{
    dbgAssert(captainProposal < sigsNumPlayers);
    captaincyLog(FALSE,"Cap:Proposing new captain %d",captainProposal);
    SendTransferCaptaincyPacket(-2,XFERCAP_CAPTAIN_PROPOSAL,captainProposal,NULL);
    captainProposal = (captainProposal + 1) % sigsNumPlayers;
    dbgAssert(captainProposal < sigsNumPlayers);
}

void GiveUpCaptaincy(void)
{
    dbgAssert(IAmCaptain);
    captainIndex = -1;      // I am no longer captain
}

void AcknowledgeNewCaptain(sdword newcaptainIndex)
{
    captaincyLog(FALSE,"Cap:New captain acknowledged %d",newcaptainIndex);
    dbgAssert(newcaptainIndex < sigsNumPlayers);
    captainIndex = newcaptainIndex;
    receiveSyncPacketsFrom = captainIndex;
    explicitlyRequestingPackets = FALSE;        // captaincy transferred, cancel explicitly requested packets
    dbgAssert(!IAmCaptain);
    CaptaincyChangedNotify();
}

void IBecomeCaptain(void)
{
    captainIndex = sigsPlayerIndex;
    receiveSyncPacketsFrom = captainIndex;
    explicitlyRequestingPackets = FALSE;        // captaincy transferred, cancel explicitly requested packets
    captaincyLog(FALSE,"Cap:I become captain %d",sigsPlayerIndex);
    sentPacketNumber = pausedNextPacketToReceive[sigsPlayerIndex];
    SendTransferCaptaincyPacket(-1,XFERCAP_I_AM_NEW_CAPTAIN,sigsPlayerIndex,NULL);
    CaptaincyChangedNotify();
}


#define STATE_NORMAL                0
#define STATE_WAITFOR_PROPOSALS     1
#define STATE_GOT_PROPOSAL_FOR_ME   2
#define STATE_WAIT_FOR_PAUSE_ACKS   3
#define STATE_PAUSED                4

void TransitionToSTATE_NORMAL(bool tellUser)
{
    captaincyLog(tellUser,"Cap:Transition to state normal");
    captainTransferState = STATE_NORMAL;
    TimeoutTimerStart(T1,T1_Timeout);
    TimedOutWaitingForPauseAcks = 0;
    resetPausedNextPacketToReceive();
}

void TransitionToSTATE_WAITFOR_PROPOSALS()
{
    captaincyLog(FALSE,"Cap:Transition to state wait for proposals");
    captainTransferState = STATE_WAITFOR_PROPOSALS;
    TimeoutTimerStart(T2,T2_Timeout);
    ProposeNewCaptain();
}

void TransitionToSTATE_GOT_PROPOSAL_FOR_ME()
{
    captaincyLog(FALSE,"Cap:Transition to state Got proposal for me");
    captainTransferState = STATE_GOT_PROPOSAL_FOR_ME;
}

void TransitionToSTATE_WAIT_FOR_PAUSE_ACKS()
{
    captaincyLog(FALSE,"Cap:Transition to state Wait for pause acks");
    captainTransferState = STATE_WAIT_FOR_PAUSE_ACKS;
    TimeoutTimerStart(TWAITFORPAUSEACKS,TWAITFORPAUSEACKS_Timeout);
}

void TransitionToSTATE_PAUSED()
{
    captaincyLog(FALSE,"Cap:Transition to state Paused");
    captainTransferState = STATE_PAUSED;
}

void PauseAndSendPauseAck(udword to)
{
    receiveSyncPacketsFrom = -1;        // pause
    dbgAssert(to < sigsNumPlayers);   // misc is person to become captain
    SendTransferCaptaincyPacket(to,XFERCAP_PAUSE_ACK,receivedPacketNumber,NULL);
}

void SendSyncPacketsToAllOtherPlayersTillCaughtUp(void)
{
    sdword i;

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        if (i == sigsPlayerIndex)
        {
            continue;       // don't send sync packets to myself
        }

        if (pausedNextPacketToReceive[i] == 0)
        {
            continue;       // this player not involved
        }

        dbgAssert(pausedNextPacketToReceive[i] <= pausedNextPacketToReceive[sigsPlayerIndex]);  // I should have latest sync packets
        if (pausedNextPacketToReceive[i] < pausedNextPacketToReceive[sigsPlayerIndex])
        {
            // send sync packets until caught up for player i:
            sdword j;
            HWPacketHeader *packet;
            udword size;
            bool gotit;

            for (j=pausedNextPacketToReceive[i];j<pausedNextPacketToReceive[sigsPlayerIndex];j++)
            {
                gotit = GetSyncPktFromLastSyncPktsQ(j,&packet,&size);
                if (gotit)
                {
                    captaincyLog(FALSE,"Cap:Sending sync pkt %d to %d",j,i);
                    packet->from = sigsPlayerIndex;     // same sync pkt as before, but from me this time
                    titanAnyoneSendPointMessage(i,(ubyte *)packet,size);
                }
                else
                {
                    captaincyLog(FALSE,"Cap:ERROR sending catch-up sync pkt %d to %d",j,i);
                }
            }
        }
    }
}

bool IHaveEqualOrLatestSyncPacket(void)
{
    sdword i;

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        if (i == sigsPlayerIndex)
        {
            continue;       // don't check against myself.
        }

        if (pausedNextPacketToReceive[i] > pausedNextPacketToReceive[sigsPlayerIndex])
        {
            // someone has a later sync packet than me.
            return FALSE;
        }
    }

    return TRUE;
}

udword NumberPauseAcksReceived(void)
{
    sdword i;
    udword num = 0;

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        if (i == sigsPlayerIndex)
        {
            continue;       // don't check against myself.
        }

        if (pausedNextPacketToReceive[i])
        {
            num++;
        }
    }

    return num;
}

sdword PlayerWithTheLatestSyncPacket(void)
{
    sdword i;
    sdword max = -1;
    udword maxvalue = 0;

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        if (pausedNextPacketToReceive[i] > maxvalue)
        {
            maxvalue = pausedNextPacketToReceive[i];
            max = i;
        }
    }

    return max;
}

void TransferCaptaincyGameStarted(void)
{
    captaincyLogInit();
    captaincyLog(FALSE,"Cap:I Am %d in %d-player game",sigsPlayerIndex,sigsNumPlayers);
    TimeoutTimerInitAll();
    captainProposal = 1;
    TransitionToSTATE_NORMAL(FALSE);
}

void TransferCaptaincyStateMachine(CaptaincyEvent event,uword from,udword misc,CaptaincyCustomInfo *customInfo)
{
    sdword i;

    if ((transferCaptaincyDisabled) || (sigsNumPlayers < 2))
    {
        printCaptainMessage = FALSE;
        return;
    }

    if (event != CAPEVENT_RECEIVED_SYNC_PKT)
    {
        captaincyLog(FALSE,"Cap:Received event %d in state %d from %d Misc=%d",event,captainTransferState,from,misc);
    }

    if ((captainTransferState==STATE_NORMAL) || (event==CAPEVENT_RECEIVED_SYNC_PKT))
        printCaptainMessage = FALSE;
    else
        printCaptainMessage = TRUE;

    switch (captainTransferState)
    {
        case STATE_NORMAL:
        {
            switch (event)
            {
                case CAPEVENT_GIVEUP_CAPTAINCY_NOTICE:
                case CAPEVENT_TIMEOUT1:
                    TimeoutTimerDisable(T1);
                    TransitionToSTATE_WAITFOR_PROPOSALS();
                    break;

                case CAPEVENT_CAPTAIN_PROPOSAL:
                    if (misc == sigsPlayerIndex)
                    {
                        // deliberately do not TimeoutTimerDisable(T1);
                        TransitionToSTATE_GOT_PROPOSAL_FOR_ME();
                    }
                    break;

                case CAPEVENT_RECEIVED_SYNC_PKT:
                    break;      //expected behaviour

                case CAPEVENT_PAUSE_TRANSFERRING_CAPTAIN:
                    TimeoutTimerDisable(T1);
                    PauseAndSendPauseAck(misc);
                    TransitionToSTATE_PAUSED();
                    break;

                default:
                    captaincyLog(FALSE,"Cap:Unexpected event %d in state %d",event,captainTransferState);
                    break;
            }
        }
        break;

        case STATE_WAITFOR_PROPOSALS:
        {
            switch (event)
            {
                case CAPEVENT_CAPTAIN_PROPOSAL:
                    if (misc == sigsPlayerIndex)
                    {
                        // proposal for me to become captain
                        TimeoutTimerDisable(T2);
                        receiveSyncPacketsFrom = -1;        // pause
                        resetPausedNextPacketToReceive();
                        pausedNextPacketToReceive[sigsPlayerIndex] = receivedPacketNumber;
                        SendTransferCaptaincyPacket(-1,XFERCAP_PAUSE_TRANSFERRING_CAPTAIN,sigsPlayerIndex,NULL);    // and tell everyone else to pause too
                        TransitionToSTATE_WAIT_FOR_PAUSE_ACKS();
                    }
                    break;

                case CAPEVENT_TIMEOUT2:
                    TimeoutTimerDisable(T2);
                    TransitionToSTATE_WAITFOR_PROPOSALS();      // transition back to same state
                    break;

                case CAPEVENT_RECEIVED_SYNC_PKT:
                    TimeoutTimerDisable(T2);
                    TransitionToSTATE_NORMAL(FALSE);
                    break;

                case CAPEVENT_PAUSE_TRANSFERRING_CAPTAIN:
                    TimeoutTimerDisable(T2);
                    PauseAndSendPauseAck(misc);
                    TransitionToSTATE_PAUSED();
                    break;

                default:
                    captaincyLog(FALSE,"Cap:Unexpected event %d in state %d",event,captainTransferState);
                    break;

            }
        }
        break;

        case STATE_GOT_PROPOSAL_FOR_ME:
        {
            switch (event)
            {
                case CAPEVENT_GIVEUP_CAPTAINCY_NOTICE:
                case CAPEVENT_TIMEOUT1:
                    TimeoutTimerDisable(T1);
                    receiveSyncPacketsFrom = -1;        // pause
                    resetPausedNextPacketToReceive();
                    pausedNextPacketToReceive[sigsPlayerIndex] = receivedPacketNumber;
                    SendTransferCaptaincyPacket(-1,XFERCAP_PAUSE_TRANSFERRING_CAPTAIN,sigsPlayerIndex,NULL);    // and tell everyone else to pause too
                    TransitionToSTATE_WAIT_FOR_PAUSE_ACKS();
                    break;

                case CAPEVENT_PAUSE_TRANSFERRING_CAPTAIN:
                    TimeoutTimerDisable(T1);
                    PauseAndSendPauseAck(misc);
                    TransitionToSTATE_PAUSED();
                    break;

                default:
                    captaincyLog(FALSE,"Cap:Unexpected event %d in state %d",event,captainTransferState);
                    break;
            }
        }
        break;

        case STATE_WAIT_FOR_PAUSE_ACKS:
        {
            switch (event)
            {
                case CAPEVENT_PAUSE_ACK:
                    dbgAssert(from != sigsPlayerIndex);
                    pausedNextPacketToReceive[from] = misc;
                    break;      // wait for timeout to know if received all pause_ack's

                case CAPEVENT_TWAITFORPAUSEACKS:
                    if ((NumberPauseAcksReceived() == 0) && (sigsNumPlayers > 2) && (TimedOutWaitingForPauseAcks < TimedOutWaitingForPauseAcksGiveUpAfterNumTimes))
                    {
                        TimedOutWaitingForPauseAcks++;
                        TransitionToSTATE_WAIT_FOR_PAUSE_ACKS();
                    }
                    else
                    {
                        TimeoutTimerDisable(TWAITFORPAUSEACKS);
                        captaincyLog(FALSE,"Cap: Sync status: %d %d %d %d %d %d %d %d",pausedNextPacketToReceive[0],pausedNextPacketToReceive[1],pausedNextPacketToReceive[2],pausedNextPacketToReceive[3],
                                                                                      pausedNextPacketToReceive[4],pausedNextPacketToReceive[5],pausedNextPacketToReceive[6],pausedNextPacketToReceive[7]);
                        if (IHaveEqualOrLatestSyncPacket())
                        {
                            IBecomeCaptain();
                            SendSyncPacketsToAllOtherPlayersTillCaughtUp();
                            TransitionToSTATE_NORMAL(FALSE);
                        }
                        else
                        {
                            sdword playerindex = PlayerWithTheLatestSyncPacket();
                            CaptaincyCustomInfo newcustominfo;
                            dbgAssert(playerindex != -1);
                            dbgAssert(playerindex != sigsPlayerIndex);
                            for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
                            {
                                newcustominfo.custom[i] = pausedNextPacketToReceive[i];
                            }
                            SendTransferCaptaincyPacket(playerindex,XFERCAP_PAUSE_YOU_BE_CAPTAIN,sigsPlayerIndex,&newcustominfo);
                            captaincyLog(FALSE,"Telling %d to be captain",playerindex);
                            TransitionToSTATE_PAUSED();
                        }
                    }
                    break;

                default:
                    captaincyLog(FALSE,"Cap:Unexpected event %d in state %d",event,captainTransferState);
                    break;
            }
        }
        break;

        case STATE_PAUSED:
        {
            switch (event)
            {
                case CAPEVENT_I_AM_NEW_CAPTAIN:
                    TransitionToSTATE_NORMAL(FALSE);
                    break;

                case CAPEVENT_PAUSE_YOU_BE_CAPTAIN:
                    captaincyLog(FALSE,"Cap:I'm told to be captain from %d",from);
                    dbgAssert(customInfo);
                    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
                    {
                        pausedNextPacketToReceive[i] = customInfo->custom[i];
                    }
                    captaincyLog(FALSE,"Cap: Sync status: %d %d %d %d %d %d %d %d",pausedNextPacketToReceive[0],pausedNextPacketToReceive[1],pausedNextPacketToReceive[2],pausedNextPacketToReceive[3],
                                                                                  pausedNextPacketToReceive[4],pausedNextPacketToReceive[5],pausedNextPacketToReceive[6],pausedNextPacketToReceive[7]);
                    IBecomeCaptain();       // potential problem in receiving task getting stuff out of order because there
                                            // are two seperate Q's for captaincy and sync packets?  Fix later?
                    SendSyncPacketsToAllOtherPlayersTillCaughtUp();
                    TransitionToSTATE_NORMAL(FALSE);
                    break;

                default:
                    captaincyLog(FALSE,"Cap:Unexpected event %d in state %d",event,captainTransferState);
                    break;
            }
        }
        break;

        default:
            captaincyLog(FALSE,"Cap:Unexpected event %d in unknown state %d",event,captainTransferState);
            break;
    }

}


