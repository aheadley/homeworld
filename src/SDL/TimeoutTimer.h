/*=============================================================================
    Name    : TimeoutTimer.h
    Purpose : Definitions for TimeoutTimers

    Created 98/11/13 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___TIMEOUT_TIMER_H
#define ___TIMEOUT_TIMER_H

#include "Types.h"

typedef struct TTimer
{
    bool enabled;
    bool timedOut;
    sqword timerLast;
    udword timeoutTicks;
} TTimer;

void TTimerInit(TTimer *timer);
void TTimerClose(TTimer *timer);
void TTimerDisable(TTimer *timer);
bool TTimerUpdate(TTimer *timer);
void TTimerReset(TTimer *timer);
void TTimerStart(TTimer *timer,real32 timeout);
bool TTimerIsTimedOut(TTimer *timer);

void GetRawTime(sqword *time);

#endif

