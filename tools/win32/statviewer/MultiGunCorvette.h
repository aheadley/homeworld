/*=============================================================================
    Name    : MultiGunCorvette.h
    Purpose : Definitions for MultiGunCorvette

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MULTIGUN_CORVETTE_H
#define ___MULTIGUN_CORVETTE_H

#include "types.h"
#include "spaceobj.h"
#include "attack.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    AttackSideStep attacksidestep;
} MultiGunCorvetteSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader MultiGunCorvetteHeader;

#endif //___MULTIGUN_CORVETTE_H

