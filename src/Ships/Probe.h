/*=============================================================================
    Name    : Probe.h
    Purpose : Definitions for the Probe

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PROBE_H
#define ___PROBE_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    sdword HaveMoved;
    real32 moveTime;
} ProbeSpec;

typedef struct
{
    real32 ProbeDispatchMaxVelocity;
} ProbeStatics;

/*=============================================================================
    Public data:
=============================================================================*/

real32 ProbeGetMaxVelocity(Ship *ship);

extern CustShipHeader ProbeHeader;

#endif //___PROBE_H

