/*=============================================================================
    Name    : AIDefenseMan.h
    Purpose : Definitions for AIDefenseMan

    Created 1998/05/28 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___AIDEFENSEMANAGER_H
#define ___AIDEFENSEMANAGER_H

#include "Types.h"
#include "SpaceObj.h"

void aidDefenseManager(void);
bool aidShipDied(struct AIPlayer *aiplayer, ShipPtr ship);
void aidTeamDied(struct AIPlayer *aiplayer,struct AITeam *team);

void aidInit(struct AIPlayer *aiplayer);
void aidClose(struct AIPlayer *aiplayer);

//void aidAddNewShip(Ship *ship);

void aidSendDistressSignal(SelectCommand *ships);
void aidClearDistressSignal(struct AIPlayer *aiplayer);

#endif

