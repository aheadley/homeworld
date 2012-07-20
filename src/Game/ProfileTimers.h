
#include "Types.h"

#ifndef HW_Release
#define PROFILE_TIMERS
#endif

#define NUM_PROFILE_TIMERS      8
#define PROFILE_TIMER_LABLEN    16

#define RECORD_SAMPLE_BATCH     10000

typedef struct RecordProfTimer
{
    sdword numSamplesAllocated;
    sdword numSamples;
    sdword *samples;
} RecordProfTimer;

#define PROFTIMER_TYPE_NORMAL               0
#define PROFTIMER_TYPE_ADDUPLITTLETIMES     1

typedef struct ProfileTimers
{
    sqword timeStart[NUM_PROFILE_TIMERS];
    sqword timeStop[NUM_PROFILE_TIMERS];
    sqword timeDuration[NUM_PROFILE_TIMERS];
    sqword timeTotalDuration[NUM_PROFILE_TIMERS];
    char timeLabel[NUM_PROFILE_TIMERS][PROFILE_TIMER_LABLEN];
    RecordProfTimer recordTimer[NUM_PROFILE_TIMERS];
    udword profileTimerType[NUM_PROFILE_TIMERS];
    bool recordTimersOn;
} ProfileTimers;

#ifdef PROFILE_TIMERS

extern ProfileTimers profileTimers;

void profInitFunc(void);
void profCloseFunc(void);
void profResetFunc(void);
void profTimerStartFunc(sdword timer);
void profTimerStopFunc(sdword timer);
void profTimerStatsPrintFunc(sdword *y);
void profTimerLabelFunc(sdword timer,char *label);
void profTimerStartLabelFunc(sdword timer,char *label);
void profTimerOutputRecordingsFunc();

void profTimerStartLittleLabelFunc(sdword timer,char *label);
void profTimerStopLittleFunc(sdword timer);

#endif

#ifdef PROFILE_TIMERS

#define profInit() profInitFunc()
#define profClose() profCloseFunc()
#define profReset() profResetFunc()

#define profTimerStart(t) profTimerStartFunc(t)
#define PTSTART(t) profTimerStartFunc(t)

#define profTimerStop(t) profTimerStopFunc(t)
#define PTEND(t) profTimerStopFunc(t)

#define PTENDLITTLE(t) profTimerStopLittleFunc(t)

#define profTimerStatsPrint(y) profTimerStatsPrintFunc(y)

#define profTimerLabel(t,lab) profTimerLabelFunc(t,lab)
#define PTLABEL(t,lab) profTimerLabelFunc(t,lab)

#define profTimerStartLabel(t,lab) profTimerStartLabelFunc(t,lab)
#define PTSLAB(t,lab) profTimerStartLabelFunc(t,lab)

#define PTSLABLITTLE(t,lab) profTimerStartLittleLabelFunc(t,lab)

#define profTimerRecordOn()      profileTimers.recordTimersOn = TRUE
#define profTimerRecordOff()     profileTimers.recordTimersOn = FALSE

#define profTimerOutputRecordings() profTimerOutputRecordingsFunc()

#else

#define profInit()
#define profClose()
#define profReset()

#define profTimerStart(t)
#define PTSTART(t)

#define profTimerStop(t)
#define PTEND(t)
#define PTENDLITTLE(t)

#define profTimerStatsPrint(y)

#define profTimerLabel(t,lab)
#define PTLABEL(t,lab)

#define profTimerStartLabel(t,lab)
#define PTSLAB(t,lab)
#define PTSLABLITTLE(t,lab)

#define profTimerRecordOn()
#define profTimerRecordOff()

#define profTimerOutputRecordings()

#endif

