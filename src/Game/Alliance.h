/*=============================================================================
    Name    : Alliance.h
    Purpose : This file contains all of the definitions for forming and breaking alliances.

    Created 7/31/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#ifndef ___ALLIANCE_H
#define ___ALLIANCE_H

#include "Types.h"

#define ALLIANCE_FORMNEWALLIANCE    1
#define ALLIANCE_BREAKALLIANCE      2

struct ChatPacket;
struct Ship;
struct Player;

void allianceFormWith(udword playerindex);
void allianceBreakWith(udword playerindex);
void allianceFormRequestRecievedCB(struct ChatPacket *packet);
void allianceSetAlliance(udword AllianceType, uword playerone, uword playertwo);

bool allianceIsShipAlly(struct Ship *ship, struct Player *player);
bool allianceArePlayersAllied(struct Player *playerone, struct Player *playertwo);

void allianceBreakAll(void);

#endif
