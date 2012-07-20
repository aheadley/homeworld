/*=============================================================================
        Name    : hs.h
        Purpose : hyperspace rendering & management routines

Created 7/10/1998 by khent
Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _HS_H
#define _HS_H

#include "SpaceObj.h"

void hsStart(Ship* ship, real32 cliptDelta, bool into, bool displayEffect);
void hsContinue(Ship* ship, bool displayEffect);
void hsEnd(Ship* ship, bool displayEffect);
void hsUpdate(Ship* ship);
void hsFinish(Ship* ship);
bool hsShouldDisplayEffect(Ship* ship);

#define HS_STATIC_ACTIVE        1
#define HS_STATIC_COLLAPSING    2
#define HS_STATIC_EXPANDING     3

#define HSGATE_LABEL_LEN        16

typedef struct hsStaticGate
{
    bool   active;
    vector position;
    real32 rotation;
    sdword state;
    sdword counter;
    Derelict *derelict;
    char label[HSGATE_LABEL_LEN];
} hsStaticGate;

// for hyperspace gates
void hsDerelictDied(Derelict *derelict);
void hsDerelictTakesDamage(Derelict *derelict);

void hsStaticInit(sdword nVectors); //level start (init structs)
void hsStaticReset(void);   //level reset (clear structs)
void hsStaticDestroy(hvector*); //destroy a gate
sdword hsSizeofStaticData(void);    //return size of static hyperspace gate data
void hsGetStaticData(sdword* size, ubyte* data); //retrieve static gate data (for saving)
void hsSetStaticData(sdword size, ubyte* data); //set static gate data (for loading)
void hsStaticRender(void);  //render all static gates
void hsNoGate(bool state);

void SaveHyperspaceGates(void);
void LoadHyperspaceGates(void);

Derelict *GetHyperspaceGateFromVector(vector *compare);

#endif
