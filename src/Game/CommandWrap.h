/*=============================================================================
    Name    : CommandWrap.h
    Purpose : Definitions for CommandWrap.c

    Created 7/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___COMMAND_WRAP_H
#define ___COMMNAD_WRAP_H

#include "Types.h"
#include "CommandLayer.h"

void clCommandMessage(char CommandMessage[MAX_MESSAGE_LENGTH]);

void clWrapMove(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
void clWrapAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clWrapFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation);
void clWrapDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith);
void clWrapCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);
void clWrapBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);

void clWrapDeterministicBuild(udword command,
                              CommandLayer* comlayer, sdword numShips,
                              ShipType shipType, ShipRace shipRace,
                              uword playerIndex, ShipPtr creator);

void clWrapCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource);
void clWrapProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom);
void clWrapSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targets);
void clWrapHalt(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapScuttle(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapAutoLaunch(udword OnOff,udword playerIndex);
void clWrapSetAlliance(udword AllianceType, uword curalliance, uword newalliance);
void clWrapLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom);
void clWrapSetTactics(CommandLayer *comlayer,SelectCommand *selectcom,TacticsType tacticstype);
void clWrapSetKamikaze(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapSetMilitaryParade(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapRUTransfer(CommandLayer *comlayer,sdword fromIndex,sdword toIndex, sdword resourceUnits,ubyte flags);
void clWrapMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
void clWrapResearch(udword type, udword playernum, udword labnum, udword tech);
#endif
