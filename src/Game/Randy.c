/*=============================================================================
    Name    : Randy.c
    Purpose : Generic random-number generators with multiple streams

    Created 11/11/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "Randy.h"
#include "SaveGame.h"

#if RAN_DEBUG_CALLER
#include "Universe.h"
#include "File.h"
#include <stdio.h>
#endif

/*=============================================================================
    Data:
=============================================================================*/
ranstream ranStream[RAN_NumberStreams];

#if RAN_DEBUG_CALLER
#define RAN_LogFile             "Randylog.txt"
#define RAN_LogBufferLength     512
#define RAN_LogStringLength     24
bool ranCallerDebug = FALSE;
bool ranLogCleared = FALSE;
typedef struct
{
    char file[RAN_LogStringLength];
    sdword line;
    udword univCounter;
}
ranlogentry;
ranlogentry *ranLogBuffer = NULL;
sdword ranLogIndex = 0;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : ranRandom
    Description : Get a random number from a random number sequence.
    Inputs      : ranIndex - index of random number sequence.
                  file, line - where it was called from
    Outputs     :
    Return      : New random number.
----------------------------------------------------------------------------*/
#if RAN_DEBUG_CALLER
udword ranRandomFn(sdword ranIndex, char *file, sdword line)
#else
udword ranRandomFn(sdword ranIndex)
#endif
{
    long int s;
    ranstream *stream;

#if RAN_DEBUG_CALLER
    char *fileNameFull;
    char *truncatedFile;
    FILE *fp;

    if ((ranCallerDebug || logEnable == LOG_ON || logEnable == LOG_VERBOSE) && ranIndex == RAN_Game && file != NULL)
    {
        if (ranLogBuffer == NULL)
        {
            ranLogBuffer = memAlloc(sizeof(ranlogentry) * RAN_LogBufferLength, "ranLogBuffer", NonVolatile);
        }
        if (ranLogIndex >= RAN_LogBufferLength)
        {
            fileNameFull = filePathPrepend(RAN_LogFile, FF_UserSettingsPath);

            if (fileMakeDestinationDirectory(fileNameFull))
            {
                fp = fopen(fileNameFull, ranLogCleared ? "at" : "wt");
            if (fp != NULL)
            {
                ranLogCleared = TRUE;
                for (ranLogIndex = 0; ranLogIndex < RAN_LogBufferLength; ranLogIndex++)
                {
                    fprintf(fp, "%s(%d): %d\n", ranLogBuffer[ranLogIndex].file, ranLogBuffer[ranLogIndex].line, ranLogBuffer[ranLogIndex].univCounter);
                }
                fclose(fp);
            }
            }
            ranLogIndex = 0;
        }
        truncatedFile = file + strlen(file) - (RAN_LogStringLength - 1);
        strcpy(ranLogBuffer[ranLogIndex].file, max(file, truncatedFile));
        ranLogBuffer[ranLogIndex].line = line;
        ranLogBuffer[ranLogIndex].univCounter = universe.univUpdateCounter;
        ranLogIndex++;
    }
#endif
    dbgAssert(ranIndex < RAN_NumberStreams);
    stream = &ranStream[ranIndex];
    if(stream->y>stream->x+stream->c){s=stream->y-(stream->x+stream->c);stream->c=0;}
    else {s=(stream->x+stream->c)-stream->y-18;stream->c=1;}
    stream->x=stream->y; stream->y=stream->z;
    return ((stream->z=s) + (stream->n=69069*stream->n+1013904243));
}

/*-----------------------------------------------------------------------------
    Name        : ranRandomFnSimple
    Description : A special version of the random number generator with no file
                    or line parmeters
    Inputs      :ranIndex - index of random number sequence.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RAN_DEBUG_CALLER
udword ranRandomFnSimple(sdword ranIndex)
{
    return(ranRandomFn(ranIndex, NULL, 0));
}
#endif
/*-----------------------------------------------------------------------------
    Name        : ranNumberGet
    Description : Get position in random-number sequence.
    Inputs      : ranIndex - index of random number sequence.
    Outputs     :
    Return      : Current random stream position for this stream.
----------------------------------------------------------------------------*/
udword ranNumberGet(sdword ranIndex)
{
    dbgAssert(ranIndex < RAN_NumberStreams);
    return(ranStream[ranIndex].n);
}

/*-----------------------------------------------------------------------------
    Name        : ranNumberSet
    Description : Set position in random-number sequence.
    Inputs      : ranIndex - index of random number sequence.
                  nn - random number to start at.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ranNumberSet(sdword ranIndex, udword nn)
{
    dbgAssert(ranIndex < RAN_NumberStreams);
    ranStream[ranIndex].n=nn;
}

/*-----------------------------------------------------------------------------
    Name        : ranParametersSet
    Description : Set the parameters used to define a random-number sequence.
                    These will have defaults.
    Inputs      : ranIndex - index of random number sequence.
                  xx, yy, zz, cc, nn - parameters for the random stream
    Outputs     : stores parameters for later usage
    Return      : void
----------------------------------------------------------------------------*/
void ranParametersSet(sdword ranIndex, udword xx,udword yy,udword zz,udword cc,udword nn)
{
    dbgAssert(ranIndex < RAN_NumberStreams);
    ranStream[ranIndex].x=xx; ranStream[ranIndex].y=yy; ranStream[ranIndex].z=zz; ranStream[ranIndex].c=cc; ranStream[ranIndex].n=nn;
}

/*-----------------------------------------------------------------------------
    Name        : ranParametersGet
    Description : Get the parameters used to define a random-number sequence.
    Inputs      : ranIndex - index of random number sequence.
    Outputs     : xx, yy, zz, cc, nn - parameters for the random stream
    Return      : void
----------------------------------------------------------------------------*/
void ranParametersGet(sdword ranIndex, udword *xx,udword *yy,udword *zz,udword *cc,udword *nn)
{
    dbgAssert(ranIndex < RAN_NumberStreams);
    *xx = ranStream[ranIndex].x; *yy = ranStream[ranIndex].y; *zz = ranStream[ranIndex].z; *cc = ranStream[ranIndex].c; *nn = ranStream[ranIndex].n;
}

/*-----------------------------------------------------------------------------
    Name        : ranParametersReset
    Description : Reset a stream's parameters to default values.
    Inputs      : ranIndex - index of random number sequence.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ranParametersReset(sdword ranIndex)
{
    dbgAssert(ranIndex < RAN_NumberStreams);
    ranStream[ranIndex].x=521288629;
    ranStream[ranIndex].y=362436069;
    ranStream[ranIndex].z=1613801;
    ranStream[ranIndex].c=1;
    ranStream[ranIndex].n=1131199209;
}

/*-----------------------------------------------------------------------------
    Name        : ranRandomize
    Description : randomizes the number stream based on system time
    Inputs      : ranIndex - index of random number sequence.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ranRandomize(sdword ranIndex)
{
    Uint32 curr_ticks;
    udword iter;
    udword i;

    curr_ticks = SDL_GetTicks();

    iter = curr_ticks & 255;

    for (i=0;i<iter;i++)
    {
        (void)ranRandom(ranIndex);      // call a "random" amount of times based on current time
    }
}

/*-----------------------------------------------------------------------------
    Name        : ranStartup
    Description : Startup random module
    Inputs      :
    Outputs     : inits all the random streams to default values
    Return      :
----------------------------------------------------------------------------*/
void ranStartup(void)
{
    sdword ranIndex;

    for (ranIndex = 0; ranIndex < RAN_NumberStreams; ranIndex++)
    {
        ranParametersReset(ranIndex);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ranShutdown
    Description : Shuts down the random module, currently does nothing
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ranShutdown(void)
{
#if RAN_DEBUG_CALLER
    char *fileNameFull;
    FILE *fp;
    sdword index;

    if (ranLogBuffer != NULL)
    {
        fileNameFull = filePathPrepend(RAN_LogFile, FF_UserSettingsPath);

        if (fileMakeDestinationDirectory(fileNameFull))
        {
            fp = fopen(fileNameFull, ranLogCleared ? "at" : "wt");
        if (fp != NULL)
        {
            ranLogCleared = TRUE;
            for (index = 0; index < ranLogIndex; index++)
            {
                fprintf(fp, "%s(%d): %d\n", ranLogBuffer[index].file, ranLogBuffer[index].line, ranLogBuffer[index].univCounter);
            }
            fclose(fp);
        }
        }
        ranLogIndex = 0;
        memFree(ranLogBuffer);
    }

#endif
    ;
}

/*=============================================================================
    Save Game stuff
=============================================================================*/

void ranSave(void)
{
    sdword ranIndex;

    for (ranIndex = 0; ranIndex < RAN_NumberStreams; ranIndex++)
    {
        SaveStructureOfSize(&ranStream[ranIndex],sizeof(ranstream));
    }
}

void ranLoad(void)
{
    sdword ranIndex;

    for (ranIndex = 0; ranIndex < RAN_NumberStreams; ranIndex++)
    {
        LoadStructureOfSizeToAddress(&ranStream[ranIndex],sizeof(ranstream));
    }
}

