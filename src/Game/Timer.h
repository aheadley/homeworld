#ifndef __TIMER_H
#define __TIMER_H

#include "Types.h"
#include "Universe.h"

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

#define TIMER_NAME_MAX_LENGTH 47

#define TIMER_ALLOC_INCREMENT 16

typedef struct {
    char name[TIMER_NAME_MAX_LENGTH+1];
    real32 startTime;
    real32 duration;
    real32 pauseTime;
    bool8 enabled;
    bool8 bPaused;
} Timer;

void timTimerCreate(char *name);
void timTimerStart(char *name);
void timTimerStop(char *name);
sdword timTimerExpired(char *name);
sdword timTimerExpiredDestroy(char *name);
sdword timTimerRemaining(char *name);
void timTimerSet(char *name, sdword duration);
void timTimerCreateSetStart(char *name, sdword duration);
void timTimerDestroy(char *name);
void timTimerDestroyAll(void);
void timTimerPause(Timer *tim);
void timTimerPauseAllNotScoped(char *scopeName);
void timTimerUnpause(Timer *tim);
void timTimerUnpauseAll(void);

void timTimerSave(void);
void timTimerLoad(void);

#endif
