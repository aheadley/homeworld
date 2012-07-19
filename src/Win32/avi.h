/*=============================================================================
    Name    : avi.h
    Purpose : routines for playing AVI files

    Created 1/9/1999 by jdorie, khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _AVI_H
#define _AVI_H

#ifdef __cplusplus
extern "C" {
#endif

int aviInit(void);
int aviPlay(char* filename);
int aviStop(void);
int aviCleanup(void);

int aviGetSamples(void* pBuf, long* pNumSamples, long nBufSize);

extern int aviIsPlaying;

#ifdef __cplusplus
}
#endif

#endif

