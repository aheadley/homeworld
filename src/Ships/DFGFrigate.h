/*=============================================================================
    Name    : DFGFrigate.h
    Purpose : Definitions for DFGFrigate

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___DFG_FRIGATE_H
#define ___DFG_FRIGATE_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

/*=============================================================================
    Function Prototypes:            
=============================================================================*/
         
void univDFGFieldEffect(Ship *ship, Bullet *bullet, real32 totaltimeelapsed);

         
/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    udword dummy;
} DFGFrigateSpec;

typedef struct
{
    real32 DFGFrigateFieldRadius;     //DFG's Effectie Field Radius
    real32 DFGFrigateFieldRadiusSqr;  //DFG's Effectie Field Radius Squared
    real32 BulletDamageMultiplier;    //Bullets damage is multiplied by this factor
    real32 BulletLifeExtension;       //Bullets Life is Extended by this ammount (Set high to ensure collision)
    udword percentageThatGetThrough;  //percetage of bullets that make it through field
    udword angleVariance;
} DFGFrigateStatics;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader DFGFrigateHeader;

#endif //___DDD_FRIGATE_H

