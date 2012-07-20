/*=============================================================================
    Name    : Randy.h
    Purpose : Definitions for generic random-number generator

    Created 11/11/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___RANDY_H
#define ___RANDY_H

#include "Types.h"

/*=============================================================================
    Swithces:
=============================================================================*/
#ifndef HW_Release

#define RAN_ERROR_CHECKING          1           //general error checking
#define RAN_VERBOSE_LEVEL           2           //control specific output code
#define RAN_DEBUG_CALLER            1           //for debugging the calling sequence of random numbers

#else //HW_Debug

#define RAN_ERROR_CHECKING          0           //general error checking
#define RAN_VERBOSE_LEVEL           0           //control specific output code
#define RAN_DEBUG_CALLER            0           //for debugging the calling sequence of random numbers

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define RAN_ParticleStream          0
#define RAN_ETG                     1
#define RAN_Sound                   2
#define RAN_Trails                  3
#define RAN_Clouds                  4
#define RAN_Nebulae                 5
#define RAN_AIPlayer                6
#define RAN_Damage                  7
#define RAN_Battle                  8
#define RAN_Static                  9
#define RAN_Game                    10
#define RAN_SoundGameThread         11
#define RAN_SoundBothThreads        12
#define RAN_Trails0                 13
#define RAN_Trails1                 14
#define RAN_Trails2                 15
#define RAN_Trails3                 16
#define RAN_Trails4                 17
#define RAN_SoundGameThread0        18
#define RAN_SoundGameThread1        19
#define RAN_SoundGameThread2        20
#define RAN_SoundGameThread3        21
#define RAN_SoundGameThread4        22
#define RAN_SoundGameThread5        23
#define RAN_SoundGameThread6        24
#define RAN_NumberStreams           25

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
    udword x, y, z, c, n;
}
ranstream;

/*=============================================================================
    Functions:
=============================================================================*/
void ranRandomize(sdword ranIndex);
#if RAN_DEBUG_CALLER
udword ranRandomFnSimple(sdword ranIndex);
udword ranRandomFn(sdword ranIndex, char *file, sdword line);
#else
#define ranRandomFnSimple ranRandomFn
udword ranRandomFn(sdword ranIndex);
#endif
void ranNumberSet(sdword ranIndex, udword nn);
udword ranNumberGet(sdword ranIndex);
void ranParametersSet(sdword ranIndex, udword xx,udword yy,udword zz,udword cc,udword nn);
void ranParametersGet(sdword ranIndex, udword *xx,udword *yy,udword *zz,udword *cc,udword *nn);
void ranParametersReset(sdword ranIndex);
void ranShutdown(void);
void ranStartup(void);

void ranSave(void);
void ranLoad(void);

/*=============================================================================
    Data:
=============================================================================*/
#if RAN_DEBUG_CALLER
extern bool ranCallerDebug;
#endif

/*=============================================================================
    Macros:
=============================================================================*/
#if RAN_DEBUG_CALLER

#define ranRandom(i)    ranRandomFn(i, NULL, 0)
#define gamerand() ranRandomFn(RAN_Game, __FILE__, __LINE__)
// if you change RAN_Game in randy.h, change 11 as well.  Don't include randy.h because types.h included in other projects
#define randomG(n) (ranRandomFn(RAN_Game, __FILE__, __LINE__) % (n))
// if you change RAN_Game in randy.h, change 11 as well.  Don't include randy.h because types.h included in other projects
#define frandom(n) (ranRandomFn(RAN_Game, __FILE__, __LINE__) * (((real32)(n)) * (1.0f/((real32)UDWORD_Max))))

#else //RAN_DEBUG_CALLER

#define ranRandom(i)    ranRandomFn(i)
#define gamerand() ranRandomFn(RAN_Game)
// if you change RAN_Game in randy.h, change 11 as well.  Don't include randy.h because types.h included in other projects
#define randomG(n) (ranRandomFn(RAN_Game) % (n))
// if you change RAN_Game in randy.h, change 11 as well.  Don't include randy.h because types.h included in other projects
#define frandom(n) (ranRandomFn(RAN_Game) * (((real32)(n)) * (1.0f/((real32)UDWORD_Max))))

#endif //RAN_DEBUG_CALLER

#define randombetween(a,b) ( randomG((b)-(a)+1) + (a) )
#define frandombetween(a,b) (frandom((b)-(a)) + (a))

#endif //___RANDY_H
