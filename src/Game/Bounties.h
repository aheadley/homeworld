/*=============================================================================
    Name    : bounties.h
    Purpose : bounty defines

    Created oct 6th, bpasechn
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef BOUNTY_H
#define BOUNTY_H


#include "Types.h"
#include "Universe.h"
#include "SpaceObj.h"

void BountyInit(sdword bountySettingSize);
sdword getPlayerBountyRender(Player *player);
void calculatePlayerBounties();                   //updates all players 'bounty' variable
real32 getPlayerBountyWorth(sdword playerIndex,bool CPUPlayer);     //returns a meaningless number with great meaning

sdword bountyCurrentRusForPlayer(sdword playerIndex);
sdword getBountyOnShip(Ship *ship);
void bountyShipWasKilled(Ship *ship);

#endif //BOUNTY_H
