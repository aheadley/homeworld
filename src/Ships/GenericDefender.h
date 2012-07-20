/*=============================================================================
    Name    : GenericDefender.h
    Purpose : Definitions for Generic Defender

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___GENERIC_DEFENDER_H
#define ___GENERIC_DEFENDER_H

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
    udword dummy;
} GenericDefenderSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader GenericDefenderHeader;

void doKamikazeAttack(Ship *ship,SpaceObjRotImpTarg *target);

#endif //___GENERIC_DEFENDER_H

