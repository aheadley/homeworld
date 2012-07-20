/*=============================================================================
    Name    : AIAttackMan.h
    Purpose : Definitions for AIAttackMan

    Created 1998/05/28 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___AIATTACKMANAGER_H
#define ___AIATTACKMANAGER_H

#include "Types.h"
#include "SpaceObj.h"
#include "Select.h"

typedef struct
{
    ubyte          numTeams;        //how many teams are needed to go for an armada run
    SelectCommand *targets;
    sdword         visibility;
    struct AITeam *recon_team;
} ArmadaType;


struct AITeam *aiaSendRecon(SelectCommand *ships);
void aiaAttackManager(void);
void aiaProcessSpecialTeams(void);
bool aiaShipDied(struct AIPlayer *aiplayer, ShipPtr ship);
void aiaTeamDied(struct AIPlayer *aiplayer,struct AITeam *team);

void aiaP2AttackManager(void);

void aiaInit(struct AIPlayer *aiplayer);
void aiaClose(struct AIPlayer *aiplayer);

//void aiaAddNewShip(Ship *ship);

#endif

