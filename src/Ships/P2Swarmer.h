/*=============================================================================
    Name    : P2Swarmer.h
    Purpose : Definitions for P2Swarmer

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___P2_SWARMER_H
#define ___P2_SWARMER_H

#include "Types.h"
#include "SpaceObj.h"
#include "Attack.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    AttackSideStep attacksidestep;
} P2SwarmerSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader P2SwarmerHeader;

#endif //___P2_SWARMER_H

