/*=============================================================================
    Name    : bink.h
    Purpose : routines for playback of Bink files

    Created 6/8/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _BINK_H
#define _BINK_H

#include "radbink.h"
#include "types.h"

#define S_RGB555    0
#define S_ARGB8888  1
#define S_RGB888    2
#define S_BGR888    3
#define S_RGB565    4
#define S_RGBA8888  5
#define S_PAL8      6

extern bool binkDonePlaying;

typedef void (*binkDecodeCallback_proc)(sdword frame);
typedef void (*binkDisplayCallback_proc)(void* psurf, sdword pitch, sdword x, sdword y);
typedef void (*binkDisplay_proc)(binkDisplayCallback_proc callback);
typedef void (*binkEndCallback_proc)(void);

bool binkInit(sdword rgltype);
bool binkPlay(char* filename,
              binkDisplay_proc displayProc,
              binkDecodeCallback_proc decodeProc,
              sdword surfType, bool rev, sdword tracknum);
bool binkStop(void);
bool binkPause(bool pause);
bool binkCleanup(void);
uword* binkGetSurface(void);

HBINK binkGetBink(void);

extern real32 binkFrameRate;
extern bool   binkPlaying;

#endif
