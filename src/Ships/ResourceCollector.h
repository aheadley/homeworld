/*=============================================================================
    Name    : ResourceCollector.h
    Purpose : Definitions for ResourceCollector

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___RESOURCE_COLLECTOR_H
#define ___RESOURCE_COLLECTOR_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    sdword resourcesCollected;
} ResourceCollectorSpec;

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader ResourceCollectorHeader;

#endif //___RESOURCE_COLLECTOR_H

