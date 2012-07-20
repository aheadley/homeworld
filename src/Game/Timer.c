#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Timer.h"
#include "Memory.h"
#include "SaveGame.h"

//
//  Created 1997/07/01  Darren Stone
//

//
//  support for named, low-resolution timers
//  (specified in seconds, based on universe time)
//
//  the named referencing of these timers make using
//  them a little slower, but friendly for scripting, etc.
//

sdword timersAllocated = 0;
sdword timersUsed = 0;
Timer *timers = NULL;

static Timer *timTimerFind(char *name)
{
    sdword i;

    for (i = 0; i < timersUsed; ++i)
        if (!strncmp(timers[i].name, name, TIMER_NAME_MAX_LENGTH))
            break;

    if (i >= timersUsed)
        return NULL;  // doesn't exist
    else
        return timers+i;
}

void timTimerStart(char *name)
{
    Timer *tp;

    tp = timTimerFind(name);
    if (!tp)
        return;

    if (tp->duration > 0)
    {
        tp->startTime = universe.totaltimeelapsed;
        tp->enabled = 1;
        tp->bPaused = FALSE;
    }
    else
        tp->enabled = 0;
}

void timTimerStop(char *name)
{
    Timer *tp;

    tp = timTimerFind(name);
    if (!tp)
        return;

    tp->duration = (real32)timTimerRemaining(name);
    tp->enabled = 0;
}

sdword timTimerExpired(char *name)
{
    if (!timTimerFind(name))
        return 0;  // non-existent timers haven't "expired"

    return !timTimerRemaining(name);
}

sdword timTimerExpiredDestroy(char *name)
{
    if (!timTimerFind(name))
        return 0;  // non-existent timers haven't "expired"

    if (!timTimerRemaining(name))
    {
       timTimerDestroy(name);
       return 1;
    }
    else
       return 0;
}

sdword timTimerRemaining(char *name)
{
    Timer *tp;
    sdword rem;

    tp = timTimerFind(name);
    if (!tp)
        return 0;
    if (tp->bPaused)
    {
        return(SDWORD_Max);
    }
    if ((!tp->enabled))
        return 0;
    rem = (sdword)(tp->duration - (universe.totaltimeelapsed - tp->startTime));
    if (rem <= 0)
    {
        tp->enabled = 0;
        rem = 0;
    }
    return rem;
}

void timTimerSet(char *name, sdword duration)
{
    Timer *tp;

    tp = timTimerFind(name);
    if (!tp)
        return;

    tp->duration = (real32)duration;
}

void timTimerDestroy(char *name)
{
    sdword i;

    for (i = 0; i < timersUsed; ++i)
        if (!strncmp(timers[i].name, name, TIMER_NAME_MAX_LENGTH))
            break;

    if (i >= timersUsed)
        return;  // doesn't exist

    while (i < timersUsed-1)
    {
        memcpy(timers + i, timers + i+1, sizeof(Timer));
        ++i;
    }

    --timersUsed;
}

void timTimerDestroyAll(void)
{
    if (timers)
    {
        memFree(timers);
        timers = NULL;
    }
    timersAllocated = 0;
    timersUsed = 0;
}

void timTimerCreate(char *name)
{
    if (timTimerFind(name))
        return;  // already exists

    // allocate more if necessary
    if (timersUsed >= timersAllocated)
    {
        timers = memRealloc(timers, sizeof(Timer) * (timersAllocated + TIMER_ALLOC_INCREMENT), "timers", 0);
        timersAllocated += TIMER_ALLOC_INCREMENT;
    }

    memStrncpy(timers[timersUsed].name, name, TIMER_NAME_MAX_LENGTH);
    if (strlen(name) < TIMER_NAME_MAX_LENGTH)
        timers[timersUsed].name[strlen(name)] = 0;
    else
        timers[timersUsed].name[TIMER_NAME_MAX_LENGTH] = 0;
    timers[timersUsed].duration = 0;
    timers[timersUsed].enabled = 0;
    timers[timersUsed].bPaused = 0;

    ++timersUsed;
}

//
//  saves having to do a create, then a set, then a start
//  with separate operations.
//
void timTimerCreateSetStart(char *name, sdword duration)
{
    timTimerCreate(name);
    timTimerSet(name, duration);
    timTimerStart(name);
}

/*-----------------------------------------------------------------------------
    Name        : timTimerPause
    Description : Pause a timer.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void timTimerPause(Timer *tim)
{
    dbgAssert(tim);
    tim->pauseTime = universe.totaltimeelapsed;
    tim->bPaused = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : timTimerPauseAllNotScoped
    Description : Pauses all timers that do not have the specified scope name
    Inputs      : scopeName - name of the FSM that called this function.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void timTimerPauseAllNotScoped(char *scopeName)
{
    Timer *tim = timers;
    sdword index, stringLength = strlen(scopeName);

    for (index = 0; index < timersUsed; index++, tim++)
    {
        if (memcmp(scopeName, tim->name, stringLength))
        {                                                   //if does not match scope name
            timTimerPause(tim);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : timTimerUnpause
    Description : Unpause a timer.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void timTimerUnpause(Timer *tim)
{
    tim->duration += universe.totaltimeelapsed - tim->pauseTime;
    tim->bPaused = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : timTimerUnpauseAll
    Description : Resume all previously paused timers.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void timTimerUnpauseAll(void)
{
    Timer *tim = timers;
    sdword index;

    for (index = 0; index < timersUsed; index++, tim++)
    {
        if (tim->bPaused)
        {                                                   //if this timer is paused
            timTimerUnpause(tim);
        }
    }
}

/*=============================================================================
    Save Game Stuff below:
=============================================================================*/

void timTimerSave(void)
{
    SaveInfoNumber(timersAllocated);
    SaveInfoNumber(timersUsed);

    if (timersAllocated > 0)
    {
        dbgAssert(timers);
        // just number data, no pointers in Timer so we can just save this way
        SaveStructureOfSize(timers,timersAllocated * sizeof(Timer));
    }
}

void timTimerLoad(void)
{
    timersAllocated = LoadInfoNumber();
    timersUsed = LoadInfoNumber();

    if (timersAllocated > 0)
    {
        timers = LoadStructureOfSize(timersAllocated * sizeof(Timer));
    }
    else
    {
        timers = NULL;
    }
}

