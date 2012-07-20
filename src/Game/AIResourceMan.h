/*=============================================================================
    Name    : AIResourceMan.h
    Purpose : Definitions for AIResourceMan.c

    Created 5/27/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef ___AIRESOURCEMAN_H
#define ___AIRESOURCEMAN_H

#include "Types.h"
#include "ShipSelect.h"

struct AITeam;

void airResourceManager(void);

void airInit(struct AIPlayer *aiplayer);
void airClose(struct AIPlayer *aiplayer);

bool airShipDied(struct AIPlayer *aiplayer,ShipPtr ship);
void airTeamDied(struct AIPlayer *aiplayer, struct AITeam *team);

sdword NumRUDockPointsOnThisShip(Ship *ship);

void airAddNewShip(Ship *ship);
void airProcessSpecialTeams(void);

real32 BlobInbetweenMothershipAndEnemyRating(void *thisBlob);

#endif

