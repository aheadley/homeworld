/*=============================================================================
    Name    : flightman.h
    Purpose : Definitions for flightman.c

    Created 11/4/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___FLIGHTMAN_H
#define ___FLIGHTMAN_H

#include "Types.h"
#include "SpaceObj.h"
#include "FlightManDefs.h"

#define FLIGHTMAN_TYPE_TURNAROUND   0
#define FLIGHTMAN_TYPE_AIP          1
#define FLIGHTMAN_TYPE_EVASIVE      2
#define FLIGHTMAN_TYPE_DUAL         3
#define FLIGHTMAN_NUM_MANEUVERTYPES 4

// subtypes
#define FLIGHTMAN_EVASIVE_FRONT     1
#define FLIGHTMAN_EVASIVE_BEHIND    2
#define FLIGHTMAN_EVASIVE_PURE      3
#define FLIGHTMAN_TURNAROUND        4
#define FLIGHTMAN_AIP               5

/*=============================================================================
    Defines for specific flight maneuvers
=============================================================================*/

// values for flightmanState2 for Whip Strafe
#define WHIPSTRAFE_ABOVE    1       // indicates coming from above

#define IMMELMAN_INVERT     1
#define IMMELMAN_ROLLLEFT   2
#define SOFTBANK_LEFT       1

#define SANDWICH_LEFT       0
#define SANDWICH_RIGHT      1

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    udword flightprobTurnaround[FLIGHTMAN_TYPE_TURNAROUND_NUM];
    udword flightcumTurnaround[FLIGHTMAN_TYPE_TURNAROUND_NUM];
    udword sumtotalTurnaround;

    udword flightprobAIP[FLIGHTMAN_TYPE_AIP_NUM];
    udword flightcumAIP[FLIGHTMAN_TYPE_AIP_NUM];
    udword sumtotalAIP;

    udword flightprobEvasiveBehind[FLIGHTMAN_TYPE_EVASIVE_NUM];
    udword flightcumEvasiveBehind[FLIGHTMAN_TYPE_EVASIVE_NUM];
    udword sumtotalEvasiveBehind;

    udword flightprobEvasiveFront[FLIGHTMAN_TYPE_EVASIVE_NUM];
    udword flightcumEvasiveFront[FLIGHTMAN_TYPE_EVASIVE_NUM];
    udword sumtotalEvasiveFront;

    udword flightprobEvasivePure[FLIGHTMAN_TYPE_EVASIVE_NUM];
    udword flightcumEvasivePure[FLIGHTMAN_TYPE_EVASIVE_NUM];
    udword sumtotalEvasivePure;

    udword valid;
} FlightManProb;

/*=============================================================================
    Functions:
=============================================================================*/

void scriptSetFlightManTurnaroundCB(char *directory,char *field,FlightManProb *dataToFillIn);
void scriptSetFlightManAIPCB(char *directory,char *field,FlightManProb *dataToFillIn);
void scriptSetFlightManEvasiveBehindCB(char *directory,char *field,FlightManProb *dataToFillIn);
void scriptSetFlightManEvasiveFrontCB(char *directory,char *field,FlightManProb *dataToFillIn);
void scriptSetFlightManEvasivePureCB(char *directory,char *field,FlightManProb *dataToFillIn);

udword flightmanGetRandom(FlightManProb *prob,udword flightmanSubtype);
bool flightmanTestRandom(FlightManProb *prob,udword flightmanSubtype,udword flightman);

void flightmanInitFunc(Ship *ship,udword flightman,sdword flags);

bool flightmanExecute(Ship *ship);
void flightmanClose(Ship *ship);

#define flightmanInit(ship,flightman) flightmanInitFunc(ship,flightman,-1)
#define flightmanInitWithFlags(ship,flightman,flags) flightmanInitFunc(ship,flightman,flags)

/*=============================================================================
    Data:
=============================================================================*/

extern real32 WHIPSTRAFE_ESCAPEDIST_Z;
extern real32 WHIPSTRAFE_BULLETRANGEOPTIMUM;
extern real32 WHIPSTRAFE_MAXANGCOMPAREFACTOR;
extern real32 WHIPSTRAFE_FLYBYOVERSHOOT;
extern real32 WHIPSTRAFE_MINVELOCITY;
extern real32 WHIPSTRAFE_REACHDESTINATIONRANGE;

extern sdword FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKRATE;
extern sdword FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKFRAME;
extern sdword FLIGHTMAN_TOKEN_EVASIVEPURE_PROB;

extern sdword FLIGHTMAN_EVASIVEPURE_PROB[NUM_TACTICS_TYPES];
extern sdword FLIGHTMAN_SPLIT_PROB[NUM_TACTICS_TYPES];
extern sdword FLIGHTMAN_SANDWICH_PROB[NUM_TACTICS_TYPES];

extern uword FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[NUM_TACTICS_TYPES];

/*=============================================================================
    Debug features:
=============================================================================*/

#ifndef HW_Release
extern Ship *testflightmanship;
void flightmanTest();
#endif

#endif

