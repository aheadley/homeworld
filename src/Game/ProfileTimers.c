
#include "ProfileTimers.h"
#include "TimeoutTimer.h"
#include "color.h"
#include "font.h"
#include <string.h>
#include "Memory.h"
#include "Debug.h"
#include "File.h"

#ifdef PROFILE_TIMERS

#define PROFILE_OUTPUTFILE "ProfTimers.txt"

ProfileTimers profileTimers;

void profTimerRecord(sdword timer);

void profInitFunc(void)
{
    memset(&profileTimers,0,sizeof(profileTimers));
    //profTimerRecordOn();          uncomment if you want profile timer recording on
}

void profCloseFunc(void)
{
    sdword i;

    for (i=0;i<NUM_PROFILE_TIMERS;i++)
    {
        if (profileTimers.recordTimer[i].samples)
        {
            memFree(profileTimers.recordTimer[i].samples);
            profileTimers.recordTimer[i].samples = NULL;
        }
    }
}

void profResetFunc(void)
{
    //profTimerOutputRecordingsFunc();     uncomment if you want output from profile timers
    profCloseFunc();
    profInitFunc();
}

void profTimerLabelFunc(sdword timer,char *label)
{
    if ((timer < 0) || (timer >= NUM_PROFILE_TIMERS)) return;
    memStrncpy(profileTimers.timeLabel[timer],label,PROFILE_TIMER_LABLEN-1);
}

void profTimerStartLittleLabelFunc(sdword timer,char *label)
{
    if ((timer < 0) || (timer >= NUM_PROFILE_TIMERS)) return;
    profileTimers.profileTimerType[timer] = PROFTIMER_TYPE_ADDUPLITTLETIMES;
    if (profileTimers.timeLabel[timer][0] == 0) memStrncpy(profileTimers.timeLabel[timer],label,PROFILE_TIMER_LABLEN-1);
    profileTimers.timeTotalDuration[timer] = 0;
}

void profTimerStartLabelFunc(sdword timer,char *label)
{
    if ((timer < 0) || (timer >= NUM_PROFILE_TIMERS)) return;
    if (profileTimers.timeLabel[timer][0] == 0) memStrncpy(profileTimers.timeLabel[timer],label,PROFILE_TIMER_LABLEN-1);
    GetRawTime(&profileTimers.timeStart[timer]);
}

void profTimerStartFunc(sdword timer)
{
    if ((timer < 0) || (timer >= NUM_PROFILE_TIMERS)) return;
    GetRawTime(&profileTimers.timeStart[timer]);
}

void profTimerStopFunc(sdword timer)
{
    if ((timer < 0) || (timer >= NUM_PROFILE_TIMERS)) return;
    GetRawTime(&profileTimers.timeStop[timer]);
    profileTimers.timeDuration[timer] = profileTimers.timeStop[timer] - profileTimers.timeStart[timer];
    if (profileTimers.profileTimerType[timer] == PROFTIMER_TYPE_ADDUPLITTLETIMES)
    {
        profileTimers.timeTotalDuration[timer] += profileTimers.timeDuration[timer];
    }
    else
    if (profileTimers.recordTimersOn)
    {
        profTimerRecord(timer);
    }
}

void profTimerStopLittleFunc(sdword timer)
{
    if (profileTimers.recordTimersOn)
    {
        profTimerRecord(timer);
    }
}

void profTimerStatsPrintFunc(sdword *y)
{
    sdword i;

    for (i=0;i<NUM_PROFILE_TIMERS;i++)
    {
        if (profileTimers.timeDuration[i])
        {
            sdword timeDuration;
            if (profileTimers.profileTimerType[i] == PROFTIMER_TYPE_ADDUPLITTLETIMES)
                timeDuration = profileTimers.timeTotalDuration[i] / (1000L);
            else
                timeDuration = profileTimers.timeDuration[i] / (1000L);
            if (profileTimers.timeLabel[i][0])
                fontPrintf(0,*y += 15,colWhite, "Timer %s : %d",profileTimers.timeLabel[i],timeDuration);
            else
                fontPrintf(0,*y += 15,colWhite, "Timer %d : %d",i,timeDuration);
        }
    }
}

void profTimerRecord(sdword timer)
{
    sdword duration = profileTimers.timeDuration[timer] / (1000L);
    RecordProfTimer *rec = &profileTimers.recordTimer[timer];

    if (profileTimers.profileTimerType[timer] == PROFTIMER_TYPE_ADDUPLITTLETIMES)
        duration = profileTimers.timeTotalDuration[timer] / (1000L);
    else
        duration = profileTimers.timeDuration[timer] / (1000L);

    if (rec->numSamples >= rec->numSamplesAllocated)
    {
        rec->numSamplesAllocated += RECORD_SAMPLE_BATCH;
        rec->samples = memRealloc(rec->samples,sizeof(sdword)*rec->numSamplesAllocated,"recproftimers",0);
    }

    dbgAssert(rec->numSamples < rec->numSamplesAllocated);
    rec->samples[rec->numSamples++] = duration;
}

void profTimerOutputRecordingsFunc()
{
    sdword i,j;
    bool haverecordings = FALSE;

    for (i=0;i<NUM_PROFILE_TIMERS;i++)
    {
        if (profileTimers.recordTimer[i].samples)
        {
            haverecordings = TRUE;
            break;
        }
    }

    if (!haverecordings)
    {
        return;
    }

    logfileClear(PROFILE_OUTPUTFILE);

    for (i=0;i<NUM_PROFILE_TIMERS;i++)
    {
        if (profileTimers.recordTimer[i].samples)
        {
            RecordProfTimer *rec = &profileTimers.recordTimer[i];

            if (profileTimers.timeLabel[i][0])
                logfileLogf(PROFILE_OUTPUTFILE,"\nTimer %20s:",profileTimers.timeLabel[i]);
            else
                logfileLogf(PROFILE_OUTPUTFILE,"\nTimer %20d:",i);

            for (j=0;j<rec->numSamples;j++)
            {
                logfileLogf(PROFILE_OUTPUTFILE,"\t%d",rec->samples[j]);
            }
        }
    }
}

#endif

