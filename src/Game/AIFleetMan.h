
#ifndef ___AIFLEETCOMMAND_H
#define ___AIFLEETCOMMAND_H

#include "Types.h"
#include "SpaceObj.h"

void aifFleetCommand(void);

void aifInit(struct AIPlayer *aiplayer);
void aifClose(struct AIPlayer *aiplayer);

void aifTeamDied(struct AIPlayer *aiplayer,struct AITeam *team, bool removeAllReferencesToTeam);

bool aifShipDied(struct AIPlayer *aiplayer,ShipPtr ship);

void aifResourceManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority);
void aifAttackManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority);
void aifDefenseManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority);

void aifTeamRequestsShipsCB(ShipType shiptype,sdword number,struct AITeam *team,char *doneSetVar, sdword priority);

void aifHyperspaceInit(AIPlayer *aiplayer);
#endif

