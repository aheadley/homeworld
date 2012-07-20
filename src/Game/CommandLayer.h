/*=============================================================================
    Name    : CommandLayer
    Purpose : Definitions for the ship command layer,
              and camera commands too.

    Created 7/2/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___COMMAND_LAYER_H
#define ___COMMAND_LAYER_H

#include "Types.h"
#include "Vector.h"
#include "SpaceObj.h"
#include "ShipSelect.h"
#include "Formation.h"
#include "Dock.h"
#include "ResCollect.h"
#include "CommandDefs.h"

#ifndef HW_Release

#define CL_TEXTFEEDBACK     1

#else

#define CL_TEXTFEEDBACK     0

#endif
/*=============================================================================
    Types:
=============================================================================*/

//Ru Transfer types for one person
#define RUTRANS_BUILDCANCEL     101
#define RUTRANS_BUILTSHIP       102
#define RUTRANS_GENERATEDRUS    103

//defines for hyperspaceing states
#define COM_HYPERSPACE_START    0
#define COM_HYPERSPACE_OUT      1
#define COM_HYPERSPACE_WAIT     2
#define COM_HYPERSPACE_IN       3

typedef struct
{
    vector heading;
    vector destination;
} MoveCommand;

typedef SelectAnyCommand AttackCommand;
typedef SelectAnyCommandMax6 AttackCommandMax6;
typedef SelectAnyCommand SpecialCommand;

#define PROTECTFLAGS_JUST_FOLLOW     1
typedef SelectCommand ProtectCommand;
typedef SelectCommandMax6 ProtectCommandMax6;

#define IAmAWingman(attackvar) ((attackvar)->myLeaderIs)
#define IAmALeader(attackvar) ((attackvar)->myWingmanIs)

typedef struct
{
    ShipPtr receiverShip;
} LaunchShipCommand;

typedef struct
{
    ShipType shipType;
    ShipRace shipRace;
    ShipPtr creator;
    udword frameAtWhichToCreate;
    udword playerIndex;
} BuildingShipCommand;

typedef struct
{
    uword order;
    uword attributes;
} CommandOrder;

typedef struct CommandToDo
{
    Node todonode;
    SelectCommand *selection;
    CommandOrder ordertype;
    MoveCommand move;
    AttackCommand *attack;
    FormationCommand formation;
    DockCommand dock;
    LaunchShipCommand launchship;
    CollectResourceCommand collect;
    ProtectCommand *protect;
    udword protectFlags;
    SpecialCommand *specialtargets;
    MilitaryParadeCommand *militaryParade;
    BuildingShipCommand buildingship;
    real32 tacticalupdatetime;                 //needed for optimal scanning of ship list in enemy detection for tactics
    real32 holdingStartTime;                //time that a command entered a holding pattern
    real32 hyperSpaceingTime;
    sdword hyperspaceState;
    sdword turnAroundTimes;
    vector holdingPatternVec;
    real32 pingUpdateTime;                  //last time this command was processed by pings
    void *updatedPing;                      //what ping it was last updated for
} CommandToDo;

typedef struct CommandLayer
{
    LinkedList todolist;
} CommandLayer;

/*=============================================================================
    Macros:
=============================================================================*/

#define sizeofAttackCommand sizeofSelectCommand
#define sizeofProtectCommand sizeofSelectCommand

// any variables you want to reset in CommandToDo upon initialization, put here
#define InitCommandToDo(command) memset(command,0,sizeof(CommandToDo))

/*=============================================================================
    Data:
=============================================================================*/
#if CL_TEXTFEEDBACK
extern bool enableTextFeedback;
#endif

/*=============================================================================
    Functions:
=============================================================================*/

// Initializing, Closing, Processing
void clInit(CommandLayer *comlayer);
void clClose(CommandLayer *comlayer);
void clReset(CommandLayer *comlayer);
void clProcess(CommandLayer *comlayer);
void clPostProcess(CommandLayer *comlayer);

void clChecksum(void);

// Ship Command Layer
void clMove(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
CommandToDo *clAttackThese(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clPassiveAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation);
sdword clDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith);
Ship *clCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);
void clBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);
void clCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource);
CommandToDo *clProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom);
void clSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targets);
void clHalt(CommandLayer *comlayer,SelectCommand *selectcom);
void clHaltThese(CommandLayer *comlayer,SelectCommand *selectcom);
void clScuttle(CommandLayer *comlayer,SelectCommand *selectcom);
void clAutoLaunch(udword OnOff,udword playerIndex);
void clSetAlliance(udword AllianceType, uword curalliance, uword newalliance);
void clLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom);
void clSetTactics(CommandLayer *comlayer,SelectCommand *selectcom,TacticsType tacticstype);
void clSetKamikaze(CommandLayer *comlayer,SelectCommand *selectcom);
void clSetMilitaryParade(CommandLayer *comlayer,SelectCommand *selectcom);
void clRUTransfer(CommandLayer *comlayer, sdword toIndex, sdword fromIndex, sdword resourceUnits,ubyte flags);
void clMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
void clSetResearch(udword type, udword playernum, udword labnum, udword tech);
void clDeterministicBuild(udword command, CommandLayer* comlayer, sdword numShips, ShipType shipType, ShipRace shipRace, uword playerIndex, ShipPtr creator);

// does ship have to launch
bool ShipHasToLaunch(Ship *InsideShip, Ship *ship);

// launch functions
void LaunchAllInternalShipsOfPlayerThatMustBeLaunched(struct Player *player);
sdword LaunchAllInternalShipsOfPlayer(struct Player *player, udword carriermask);

// checks if can ChangeOrderToPassiveAttack
bool canChangeOrderToPassiveAttack(CommandToDo *alreadycommand,AttackCommand *attack);

// changes order to passive attack
// WARNING: Must call canChangeOrderToPassiveAttack to see if you can call this
void ChangeOrderToPassiveAttack(CommandToDo *alreadycommand,AttackCommand *attackcom);

// changes order to attack
void ChangeOrderToAttack(CommandToDo *alreadycommand,AttackCommand *attackcom);

// changes order to move
void ChangeOrderToMove(CommandToDo *alreadycommand,vector from,vector to);

// Returns allocated command containing the ship
CommandToDo *getShipAndItsCommand(CommandLayer *comlayer,ShipPtr ship);
SelectCommand *getShipAndItsCommandSelection(CommandLayer *comlayer,ShipPtr ship,bool *parade);

// Returns allocated command containing the ship and its formation
CommandToDo *getShipAndItsFormationCommand(CommandLayer *comlayer,ShipPtr ship);

// Returns allocated selectcommand containing the ship and its formation
SelectCommand *getShipAndItsFormation(CommandLayer *comlayer,ShipPtr ship);

// returns TRUE if searchfor is in comlayer
bool CommandInCommandLayer(CommandLayer *comlayer,CommandToDo *searchfor);

// Call this function to play appropriate formation sound
void PlayFormationSound(TypeOfFormation formation);

// Call this function to see if this selection is already in formation
TypeOfFormation clSelectionAlreadyInFormation(CommandLayer *comlayer,SelectCommand *selectcom);

// Call this function to update ship command layer when ship dies
void clShipDied(CommandLayer *comlayer,ShipPtr deadship);

// Call this function to update ship command layer when resource dies
void clResourceDied(CommandLayer *comlayer,ResourcePtr resource);

// Call this function to update ship command layer when derelict dies
void clDerelictDied(CommandLayer *comlayer,DerelictPtr derelict);

// Call this function to update ship command layer when missile dies
void clMissileDied(CommandLayer *comlayer,MissilePtr missile);

// Tells if selectcom is already doing something in the command layer
CommandToDo *IsSelectionAlreadyDoingSomething(CommandLayer *comlayer,SelectCommand *selectcom);

// Removes ships in selectcom from doing stuff in the command layer
void RemoveShipsFromDoingStuff(CommandLayer *comlayer,SelectCommand *selectcom);

// values for removeFlag for RemoveShipFromBeingTargeted
#define REMOVE_PROTECT          1
#define REMOVE_CLOAKED          2
#define REMOVE_DISABLED         4
#define REMOVE_DISAPPEARED      8
#define REMOVE_HYPERSPACING     16

// Removes ship from being targeted in the command layer
void RemoveShipFromBeingTargeted(CommandLayer *comlayer,ShipPtr shiptoremove,udword removeFlag);

//ccleans up game when a ship goes out of view..
void shipHasJustCloaked(Ship *ship);

// cleans up game when a ship becomes disabled
void shipHasJustBeenDisabled(Ship *ship);

// when ship just disappeared (SOF_Hide) e.g. univRemoveShipFromOutside
void shipHasJustDisappeared(Ship *ship);

//REmoves target info from command regarding shiptoremove
void RemoveShipReferencesFromExtraAttackInfo(Ship *shiptoremove,CommandToDo *todo);
void RemoveAttackTargetFromExtraAttackInfo(SpaceObjRotImpTarg *targettoremove,CommandToDo *todo);
void removeShipsFromDockingWithThisShip(Ship *ship);

void RemoveShipFromAttacking(Ship *ship);

// Initializes ship's AI state variables
void InitShipAI(Ship *ship,bool fresh);

// Initializes selection ships AI state variables
void InitShipsForAI(SelectCommand *selection,bool fresh);

// clears any protecting in command
void ClearProtecting(CommandToDo *command);

// clears any passive attack command
void ClearPassiveAttacking(CommandToDo *command);

// frees last order of command, so new order can be put in
void FreeLastOrder(CommandToDo *command);

// adds a ship to a formation group
void AddShipToFormationGroup(ShipPtr ship,CommandToDo *group);

// adds a ship to a command group
void AddShipToGroup(ShipPtr ship,CommandToDo *group);

// Adds a ship to the proper group around aroundShip
void GroupShip(CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip);

// groups ship into military parade around aroundShip
void GroupShipIntoMilitaryParade(CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip);

// sets ships to correct position according to command layer
void clPresetShipsToPosition(CommandLayer *comlayer);

// cleans up formation ship stuff
void RemoveShipFromFormation(Ship *ship);

void fixFlipTurnSelection(SelectCommand *selection,SelectCommand *global);

CommandToDo *GetMilitaryGroupAroundShip(CommandLayer *comlayer,Ship *aroundShip);

// call whenever creating a new command in command layer or ships changing their command
void PrepareShipsForCommand(CommandToDo *command,bool rowClear);
void PrepareOneShipForCommand(Ship *ship,CommandToDo *command,bool rowClear);

// call whenever removing command
void RemoveShipsFromCommand(CommandToDo *command);
// call whenever removing a ship from command (e.g. removing ship from command->selection)
void RemoveShipFromCommand(Ship *ship);

//returns cost of selSelected to hyperspace to distance
real32 hyperspaceCost(real32 distance,SelectCommand *selection);

void FillInCarrierMothershipInfo(struct Player *player,Ship **mothership,Ship *carrierX[]);

//cancels all current players launching ship(single player game only)
void clCancelAllLaunchOrdersFromPlayer(struct Player *player);

// tells ship to stay nearby protectThisShip - actual logic for attack if protectThisShip being attacked not in this routine
void protectShip(Ship *ship,Ship *protectThisShip,bool passiveAttacked);

/*=============================================================================
    Data:
=============================================================================*/

#endif //___COMMAND_LAYER_H

