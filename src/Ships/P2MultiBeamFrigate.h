/*=============================================================================
    Name    : P2MultiBeamFrigate.h
    Purpose : Definitions for P2MultiBeamFrigate

    Created 5/07/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___P2_MULTI_BEAM_FRIGATE_H
#define ___P2_MULTI_BEAM_FRIGATE_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    real32 aiattacklast;        // time stamp for spin up and down time count
    real32 aifirestarttime;     // time stamp for start of firing
    bool   spining;
    bool   steady;
} P2MultiBeamFrigateSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader P2MultiBeamFrigateHeader;

#endif //___P2_MULTI_BEAM_FRIGATE_H

