/*=============================================================================
    Name    : Demo.c
    Purpose : Code to record and play demos.

    Created 5/21/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Types.h"
#include "File.h"
#include "Memory.h"
#include "Debug.h"
#include "Key.h"
#include "Region.h"
#include "mouse.h"
#include "UnivUpdate.h"
#include "Randy.h"
#include "Camera.h"
#include "mainrgn.h"
#include "Select.h"
#include "render.h"
#include "NetCheck.h"
#include "Demo.h"

/*=============================================================================
    Data:
=============================================================================*/
bool demDemoRecording = FALSE;
bool demDemoPlaying = FALSE;
bool wasDemoPlaying = FALSE;
char demDemoFilename[128];
taskhandle demDemoTask;
char demFileSaveName[128] = "";

demoheader *demLoadFile = NULL;
sdword demFileSize;
ubyte *demFilePointer;
demplayfinished demFinishFunction = NULL;

sdword nDemoPlays = 0, nUnivUpdates = 0;

#if DEM_RANDOM_VERIFY
bool8 demCheckStream[DEM_NumberStreamsToSave] =
{
    0,                          // RAN_ParticleStream
    0,                          // RAN_ETG
    0,                          // RAN_Sound
    1,                          // RAN_Trails
    1,                          // RAN_Clouds
    1,                          // RAN_Nebulae
    1,                          // RAN_AIPlayer
    0,                          // RAN_Damage
    1,                          // RAN_Battle
    0,                          // RAN_Static
    1,                          // RAN_Game
    1,                          // RAN_SoundGameThread
    0,                          // RAN_SoundBothThreads
    1,                          // RAN_Trails0
    1,                          // RAN_Trails1
    1,                          // RAN_Trails2
    1,                          // RAN_Trails3
    1,                          // RAN_Trails4
    1,                          // RAN_SoundGameThread0
    1,                          // RAN_SoundGameThread1
    1,                          // RAN_SoundGameThread2
    1,                          // RAN_SoundGameThread3
    1,                          // RAN_SoundGameThread4
    1,                          // RAN_SoundGameThread5
    1,                          // RAN_SoundGameThread6
};
#endif

extern udword utyNFrameTicks;

//auto-demo stuff
#if DEM_AUTO_DEMO
bool demAutoDemo = TRUE;
real32 demAutoDemoWaitTime = DEM_AUTO_DELAY;
#endif

#if DEM_CHECKSUM
bool demChecksumError = FALSE;
char demChecksumString[DEM_ChecksumLength];
#endif

#if DEM_INTERRUPTABLE
bool demPlaybackInterrupted = FALSE;
#endif

//stuff for keeping track of fake vs. real renders
udword demFakeRenderCount;                      //number of times we have to do fake renders before we're allowed to render again.
udword demRecordedTicks;                        //accumulated number of ticks from recording
udword demPlaybackTicks;                        //accumulated number of ticks from playback, including the remainder of last division
udword demPacketNumber;                         //what demo 'packet' we're on
#if DEM_FAKE_RENDER_SWITCH
bool demFakeRenders = TRUE;                     //fake renders on by default
#endif

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : demBlockWrite
    Description : Write a block to the demo file.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demBlockWrite(void *data, sdword length)
{
    FILE *appendfile;

    if (*demFileSaveName)
    {
        if (!fileMakeDestinationDirectory(demFileSaveName))
            return;

        appendfile = fopen(demFileSaveName, "ab");
        if (appendfile != NULL)
        {
            fwrite(data, length, 1, appendfile);
            fclose(appendfile);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : demRecordStart
    Description : Start recording a demo.
    Inputs      : fileName - name of file to save to.
                  saveFunction - user function to call to save user state info.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void demRecordStart(char *fileName, demstatesave saveFunction)
{
    char *fullFileName;
    demoheader header;
    ubyte *stateBuffer;
#if DEM_RANDY_SAVE
    sdword index;
#endif

    dbgAssert(demDemoPlaying != TRUE);
    //create a header structure
    strcpy(header.ident, DEM_HeaderString);
    header.version = DEM_VersionNumber;
    header.startTime = taskTimeElapsed;
#if DEM_RANDY_SAVE
    for (index = 0; index < DEM_NumberStreamsToSave; index++)
    {                                                       //save the state of all random number streams
         ranParametersGet(index, &header.randyState[index].x, &header.randyState[index].y, &header.randyState[index].z, &header.randyState[index].c, &header.randyState[index].n);
    }
#endif

    if (saveFunction != NULL)
    {                                                       //save initial configuration, if applicable
        saveFunction(&stateBuffer, &header.initialStateSize);
        dbgAssert(header.initialStateSize > 0);
    }
    else
    {
        header.initialStateSize = 0;
    }

    //open the file         (actually, delete it, and every time we will open it in append write mode)
    fullFileName = filePathPrepend(fileName, FF_UserSettingsPath);
    strcpy(demFileSaveName, fullFileName);
    remove(fullFileName);

    demBlockWrite(&header, sizeof(header));
    if (header.initialStateSize > 0)
    {                                                       //if there is state info to save
        demBlockWrite(stateBuffer, header.initialStateSize);//save it
        memFree(stateBuffer);                               //and free the state buffer
    }
#if DEM_CHECKSUM
    if (logEnable == LOG_VERBOSE)
    {
        char *demLogFileNameFull = filePathPrepend(
            DEM_LogFileName, FF_UserSettingsPath);
        if (fileMakeDestinationDirectory(demLogFileNameFull))
        {
            netlogfile = fopen(demLogFileNameFull, "wb");
            if (netlogfile)
        fclose(netlogfile);
        netlogfile = NULL;
    }
    }
#endif
    demPacketNumber = 0;
    demStateSave();

    demDemoRecording = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : demStateSave
    Description : Save the state of the mouse and keyboard at a given instant.
    Inputs      : void
    Outputs     : Creates and saves a state structure.
    Return      : void
----------------------------------------------------------------------------*/
void demStateSave(void)
{
    demomaxstate state;
    sdword index, size;
#if DEM_CHECKSUM
    sdword numShips;
#endif

    dbgAssert(demDemoRecording);
    state.time = taskTimeElapsed;                           //basic state stuff
#if DEM_CHECKSUM
    if (gameIsRunning)
    {
        if (logEnable == LOG_VERBOSE)
        {
            char *demLogFileNameFull = filePathPrepend(
                DEM_LogFileName, FF_UserSettingsPath);
            if (fileMakeDestinationDirectory(demLogFileNameFull))
            {
                netlogfile = fopen(demLogFileNameFull, "at");
            if (netlogfile)
            {
                fprintf(netlogfile, "************* Demo packet #%d   Task time %.4f\n", demPacketNumber, taskTimeElapsed);
            }
        }
        }
        state.univCheckSum = univGetChecksum(&numShips);
        if (netlogfile && logEnable == LOG_VERBOSE)
        {
            fclose(netlogfile);
            netlogfile = NULL;
        }
#if DEM_CAMERA_CHECKSUM
        state.cameraCRC = cameraChecksum(mrCamera);
#endif //DEM_CAMERA_CHECKSUM
#if DEM_SELECT_CHECKSUM
        state.nSelected = selSelected.numShips;
        state.nSelecting = selSelecting.numTargets;
#endif //DEM_SELECT_CHECKSUM
    }
    else
    {
        state.univCheckSum = DEM_NoChecksum;                    //no checksum when game not running
    }
#if DEM_RANDOM_VERIFY
        for (index = 0; index < DEM_NumberStreamsToSave; index++)
        {
            if (demCheckStream[index])
            {
                state.randyStream[index] = ranRandom(index);
            }
        }
#endif //DEM_RANDOM_VERIFY
#endif //DEM_CHECKSUM
    state.mouseX = (sword)mouseCursorX();                   //save mouse state
    state.mouseY = (sword)mouseCursorY();
    state.mouseButtons = (ubyte)mouseButtons;
    dbgAssert(taskNumberCalls >= 0 && taskNumberCalls < 256);
    state.taskNumber = (ubyte)taskNumberCalls;              //save the value of the for loop in task.c where this is called from
    state.frameTicks = utyNFrameTicks;

    for (index = state.nKeys = 0; index < KEY_TOTAL_KEYS; index++)
    {                                                       //for all keys
        if (*((ubyte *)&keyScanCode[index]) != 0)
        {                                                   //if this scan code has some activity
            state.key[state.nKeys].index = (ubyte)index;    //save it
            state.key[state.nKeys].key = keyScanCode[index];
            state.nKeys++;                                  //and increase the number of keys
        }
    }
    size = demStateSize(state.nKeys);
    demBlockWrite(&state, size);

    if (demoEndKeysHit())
    {
        demRecordEnd();
    }
    demPacketNumber++;
}

/*-----------------------------------------------------------------------------
    Name        : demNumberTicksSave
    Description : Store the number of task ticks before any tasks are called
    Inputs      : nTicks - number of task ticks for this time through
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demNumberTicksSave(sdword nTicks)
{
    dbgAssert(demDemoRecording);
    demBlockWrite(&nTicks, sizeof(sdword));
}

/*-----------------------------------------------------------------------------
    Name        : demRecordEnd
    Description : End a demo recording session, closing the file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demRecordEnd(void)
{
    dbgAssert(demDemoRecording);
    demFileSaveName[0] = 0;
    demDemoRecording = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : demBlockRead
    Description : Read in a block of data from the demo file in memory.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool demBlockRead(void *dest, sdword size)
{
    if (demFilePointer + size <= (ubyte *)demLoadFile + demFileSize)
    {
        memcpy(dest, demFilePointer, size);
        demFilePointer += size;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : demPlayStart
    Description : Start playing a demo
    Inputs      : fileName - name of the file to start play
                  loadfunction - function to call with state information
                  finishFunction - function to call whent the demo playback is completed
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demPlayStart(char *fileName, demstateload loadFunction, demplayfinished finishFunction)
{
#if DEM_RANDY_SAVE
    sdword index;
#endif

    dbgAssert(demDemoPlaying);
    dbgAssert(demLoadFile == NULL);

    demPlaybackInterrupted = FALSE;

    demFileSize = fileLoadAlloc(fileName, (void **)&demLoadFile, NonVolatile);//read in the whole file
    demFilePointer = (ubyte *)demLoadFile + sizeof(demoheader);//compute initial file pointer
#if DEM_RANDY_SAVE
    for (index = 0; index < DEM_NumberStreamsToSave; index++)
    {                                                       //load the state of all random number streams
         ranParametersSet(index, demLoadFile->randyState[index].x, demLoadFile->randyState[index].y, demLoadFile->randyState[index].z, demLoadFile->randyState[index].c, demLoadFile->randyState[index].n);
    }
#endif
    if (loadFunction != NULL && demLoadFile->initialStateSize > 0)//call the function to restore the configuration
    {
        loadFunction(demFilePointer, demLoadFile->initialStateSize);
    }
    demFilePointer += demLoadFile->initialStateSize;

#if DEM_ERROR_CHECKING
    if (strcmp(demLoadFile->ident, DEM_HeaderString))
    {
        dbgFatalf(DBG_Loc, "Invalid demo file '%s'", fileName);
    }
    if (demLoadFile->version != DEM_VersionNumber)
    {
        dbgFatalf(DBG_Loc, "Invalid version of file '%s'.  Expected version 0x%x, found 0x%x.", fileName, DEM_VersionNumber, demLoadFile->version);
    }
#endif
#if DEM_CHECKSUM
    if (logEnable == LOG_VERBOSE)
    {
        char *demLogFileNameFull = filePathPrepend(
            DEM_LogFileName, FF_UserSettingsPath);
        if (fileMakeDestinationDirectory(demLogFileNameFull))
        {
            netlogfile = fopen(demLogFileNameFull, "wb");
            if (netlogfile)
        fclose(netlogfile);
        netlogfile = NULL;
    }
    }
#endif
    demFinishFunction = finishFunction;
    demFakeRenderCount = 0;
    demPlaybackTicks = 0;
    demRecordedTicks = 0;
    demPacketNumber = 0;
    demStateLoad();
}

/*-----------------------------------------------------------------------------
    Name        : demPlayEnd
    Description : Stop playing a demo
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demPlayEnd(void)
{
    rndMainViewRender = rndMainViewRenderFunction;
    dbgAssert(demDemoPlaying);
    memFree(demLoadFile);
    demLoadFile = NULL;
    demDemoPlaying = FALSE;
#if DEM_CHECKSUM
    demChecksumError = FALSE;
#endif
    if (demFinishFunction != NULL)
    {
        demFinishFunction();
        demFinishFunction = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : demStateLoad
    Description : Load in the state of the mouse and keyboard
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void demStateLoad(void)
{
    demomaxstate state;
    sdword index, size;
#if DEM_CHECKSUM
    sdword numShips;
    real32 univCheckSum;
#endif
    ubyte mouseLButton = keyScanCode[LMOUSE_BUTTON].keypressed;

    if (!demBlockRead(&state, demStateSize(0)))
    {                                                       //if there is no more file to read
        demPlayEnd();                                       //end of demo - hand control back to user
        return;
    }

#if DEM_CHECKSUM
    strcpy(demChecksumString, "Demo Sync: ");
    demChecksumError = FALSE;

    if (gameIsRunning)
    {
        nDemoPlays++;
        if (logEnable == LOG_VERBOSE)
        {
            char *demLogFileNameFull = filePathPrepend(
                DEM_LogFileName, FF_UserSettingsPath);
            if (fileMakeDestinationDirectory(demLogFileNameFull))
            {
                netlogfile = fopen(demLogFileNameFull, "at");
            if (netlogfile)
            {
                fprintf(netlogfile, "************* Demo packet #%d   Task time %.4f\n", demPacketNumber, taskTimeElapsed);
            }
        }
        }
        univCheckSum = univGetChecksum(&numShips);
        if (netlogfile && logEnable == LOG_VERBOSE)
        {
            fclose(netlogfile);
            netlogfile = NULL;
        }
        if (state.univCheckSum != univCheckSum)
        {
            demChecksumError = TRUE;
            strcat(demChecksumString, "Univ ");
        }
        else
        {
            strcat(demChecksumString, "     ");
        }
#if DEM_CAMERA_CHECKSUM
        if (state.cameraCRC != cameraChecksum(mrCamera))
        {
            demChecksumError = TRUE;
            strcat(demChecksumString, "Cam ");
        }
        else
        {
            strcat(demChecksumString, "    ");
        }
#endif //DEM_CAMERA_CHECKSUM
#if DEM_SELECT_CHECKSUM
        if (state.nSelected != selSelected.numShips)
        {
            demChecksumError = TRUE;
            strcat(demChecksumString, "Sd ");
        }
        else
        {
            strcat(demChecksumString, "   ");
        }
        if (state.nSelecting != selSelecting.numTargets)
        {
            demChecksumError = TRUE;
            strcat(demChecksumString, "Sg ");
        }
        else
        {
            strcat(demChecksumString, "   ");
        }
#endif //DEM_SELECT_CHECKSUM
    }
#if DEM_RANDOM_VERIFY
    for (index = 0; index < DEM_NumberStreamsToSave; index++)
    {
        if (demCheckStream[index])
        {
            if (ranRandom(index) != state.randyStream[index])
            {
                demChecksumError = TRUE;
                sprintf(demChecksumString + strlen(demChecksumString), "R%c%c ", index >= 10 ? index/10 + '0' : '0', index%10 + '0');
            }
            else
            {
                strcat(demChecksumString, "    ");
            }
        }
    }
#endif //DEM_RANDOM_VERIFY
    if (demChecksumError)
    {
        dbgMessagef("\n%s", demChecksumString);
    }
#endif // DEM_CHECKSUM
    size = sizeof(demokey) * state.nKeys;
    if (!demBlockRead(&state.key, size))
    {
        demPlayEnd();
        return;
    }
    memset((ubyte *)&keyScanCode[0], 0, sizeof(keyScanCode));
    for (index = 0; index < state.nKeys; index++)
    {                                                       //set state of all keys
        keyScanCode[state.key[index].index] = state.key[index].key;
    }
    mousePositionSet((sdword)state.mouseX, (sdword)state.mouseY);//set state of mouse
    mouseButtons = state.mouseButtons;
    taskNumberCalls = (udword)state.taskNumber;             //make sure this one gets called just like when it was saved
    utyNFrameTicks = state.frameTicks;

    //tricky solution to get double-clicking to work in demo mode
    if (!mouseLButton && keyScanCode[LMOUSE_BUTTON].keypressed)
    {                                                       //if mouse button just hit
        mouseLClick();
    }
#if DEM_INTERRUPTABLE
    if (demoInterruptKeysHit())
    {
        demPlaybackInterrupted = TRUE;
        demRecordEnd();
    }
#endif
    demPacketNumber++;
}

/*-----------------------------------------------------------------------------
    Name        : demNumberTicksLoad
    Description : Load in the number of ticks for processing tasks, before any
                    tasks get called.
    Inputs      : initial - default number of ticks
    Outputs     :
    Return      : number of ticks for this process pass
----------------------------------------------------------------------------*/
sdword demNumberTicksLoad(sdword initial)
{
    sdword nTicks;

    if (demLoadFile == NULL)
    {
        return(initial);
    }
    dbgAssert(demDemoPlaying);
    if (!demBlockRead(&nTicks, sizeof(sdword)))
    {
        demPlayEnd();
        return(initial);
    }

    demRecordedTicks += nTicks;                             //update record/playback tick comparisons
    demPlaybackTicks += initial;

    if (demFakeRenderCount == 0)
    {                                                       //if we've rendered all the fake renders we were supposed to
        rndMainViewRender = rndMainViewRenderFunction;      //do one regular render this time around
#if DEM_FAKE_RENDER_SWITCH
        if (demRecordedTicks < demPlaybackTicks && (demFakeRenders))
#else
        if (demRecordedTicks < demPlaybackTicks)
#endif
        {                                                   //if the recording was running faster than playback
            demFakeRenderCount = min(demPlaybackTicks / demRecordedTicks, DEM_MaxFakeRenders);//compute number of fake renders we'll need
            demPlaybackTicks = demPlaybackTicks % demRecordedTicks;//keep the remainder
            demRecordedTicks = 0;                           //reset the recording accumulator
        }
        else
        {                                                   //else playback is running fast enough!
            demFakeRenderCount = 1;
            demPlaybackTicks = 0;
            demRecordedTicks = 0;
        }
    }
    else
    {                                                       //already rendered once, set to render nothing
        if (gameIsRunning)
        {
            rndMainViewRender = rndMainViewAllButRenderFunction;//use the fake renderer next time around
        }
    }
    demFakeRenderCount--;                                   //one less render, fake or real

    //because the last round of tasks may have set the render function pointer to something
    //else, let's set it back to the actual view rendering function.
    return(nTicks);
}

