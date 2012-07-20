/*=============================================================================
    Name    : Shipselect.h
    Purpose : Includes type definitions for selecting ships

    Created 10/11/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SHIPSELECT_H
#define ___SHIPSELECT_H

#include "Types.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

#define COMMAND_MAX_SHIPS 1000

/*=============================================================================
    Types:
=============================================================================*/

// select ships
typedef struct
{
    sdword numShips;
    real32 timeLastStatus;
    ShipPtr ShipPtr[1];
} SelectCommand;

typedef struct
{
    sdword numShips;
    real32 timeLastStatus;
    ShipPtr ShipPtr[6];
} SelectCommandMax6;

typedef struct
{
    sdword numShips;
    real32 timeLastStatus;
    ShipPtr ShipPtr[COMMAND_MAX_SHIPS];
} MaxSelection;

// select ships or targets
typedef struct
{
    sdword numTargets;
    real32 timeLastStatus;
    TargetPtr TargetPtr[1];
} SelectAnyCommand;

typedef struct
{
    sdword numTargets;
    real32 timeLastStatus;
    TargetPtr TargetPtr[6];
} SelectAnyCommandMax6;

typedef struct
{
    sdword numTargets;
    real32 timeLastStatus;
    TargetPtr TargetPtr[COMMAND_MAX_SHIPS];
} MaxAnySelection;

typedef struct
{
    sdword numResources;
    real32 timeLastStatus;
    ResourcePtr ResourcePtr[1];
} ResourceSelection;

typedef struct
{
    sdword numDerelicts;
    real32 timeLastStatus;
    DerelictPtr DerelictPtr[1];
} DerelictSelection;

typedef struct
{
    sdword numBullets;
    real32 timeLastStatus;
    BulletPtr BulletPtr[1];
} BulletSelection;

typedef struct
{
    sdword numMissiles;
    real32 timeLastStatus;
    MissilePtr MissilePtr[1];
} MissileSelection;

typedef struct
{
    sdword numSpaceObjs;
    real32 timeLastStatus;
    SpaceObjPtr SpaceObjPtr[1];
} SpaceObjSelection;

typedef struct
{
    sdword numShips;
    real32 timeLastStatus;
    ShipStaticInfo *ShipStaticPtr[1];
} SelectCommandStatic;

#define GROWSELECT_INITIALBATCH     16
#define GROWSELECT_ADDBATCH         8

typedef struct
{
    sdword maxNumShips;
    SelectCommand *selection;
} GrowSelection;

/*=============================================================================
    Macros:
=============================================================================*/

#define sizeofSelectCommand(n) (sizeof(SelectCommand) + sizeof(ShipPtr)*((n)-1))

/*=============================================================================
    Functions:
=============================================================================*/

// Adds a ship to a growable selection of ships.
void growSelectAddShip(GrowSelection *growSelect,Ship *ship);

// Returns TRUE if obj was removed
bool growSelectRemoveShip(GrowSelection *growSelect,Ship *ship);

// Returns TRUE if obj was removed
bool growSelectRemoveShipBySettingNULL(GrowSelection *growSelect,Ship *ship);

// Removes ship at index index from growSelect
void growSelectRemoveShipIndex(GrowSelection *growSelect,sdword index);

// Initializes a growable selection of ships
void growSelectInit(GrowSelection *growSelect);

// Closes a growable selection of ships
void growSelectClose(GrowSelection *growSelect);

// Resets a growable selection (sets numShips to 0)
void growSelectReset(GrowSelection *growSelect);

// Add a ship to a growable selection without duplicating an existing ship pointer
void growSelectAddShipNoDuplication(GrowSelection *growSelect, Ship *ship);

// Adds a SpaceObj to selection before index
void AddSpaceObjToSelectionBeforeIndex(SpaceObj *obj,SpaceObjSelection *selection,sdword index);

// Adds a SpaceObj to selection after index
void AddSpaceObjToSelectionAfterIndex(SpaceObj *obj,SpaceObjSelection *selection,sdword index);

// Returns TRUE if obj was removed
bool RemoveSpaceObjFromSelectionPreserveOrder(SpaceObjSelection *selection,SpaceObj *obj);

// Returns TRUE if ship is in selection
bool ShipInSelection(SelectCommand *selection,Ship *ship);

// Returns number if ShipType "type"  in selection
udword ShiptypeInSelection(SelectCommand *selection, ShipType type);

// Returns the first instance of the specified type in the selection
ShipPtr FindFirstInstanceOfShipType(SelectCommand *selection, ShipType type);

// Does not delete entry - just removes reference to it.
// Returns TRUE if any ship was removed
bool clRemoveTargetFromSelection(SelectAnyCommand *selection,TargetPtr removeTargetPtr);

#define clRemoveShipFromSelection(sel,ship) clRemoveTargetFromSelection((SelectAnyCommand *)(sel),(TargetPtr)ship)

// returns true if any ships in theseShips are in selection
bool AnyOfTheseShipsAreInSelection(SelectCommand *theseShips,SelectCommand *selection);

// returns true if all of these ships are in selection
bool TheseShipsAreInSelection(SelectCommand *theseShips,SelectCommand *selection);

// returns true if selections 1 and 2 are equivalent (ignores order)
bool SelectionsAreEquivalent(SelectCommand *selection1,SelectCommand *selection2);

// returns true if selections 1 and 2 are equivalent (including order)
bool SelectionsAreTotallyEquivalent(SelectCommand *selection1,SelectCommand *selection2);

// Makes sure that ships in selection do not include theseships
void MakeShipsNotIncludeTheseShips(SelectCommand *selection,SelectCommand *theseships);

// Makes sure that the targets in selection are not missiles.
void MakeTargetsNotIncludeMissiles(SelectAnyCommand *selection);

// Returns true if ship can harvest
bool ShipCanHarvest(ShipStaticInfo *shipstatic);

// Makes targets all be within range of comparewith, but don't include comparewith
void MakeTargetsOnlyBeWithinRangeAndNotIncludeMe(SelectAnyCommand *selection,SpaceObjRotImpTarg *comparewith,real32 range);

// Makes sure that targets in selection are ships and enemies
void MakeTargetsOnlyEnemyShips(SelectAnyCommand *selection,struct Player *curplayer);

// Same as MakeTargetsOnlyEnemyShips except includes stuff like hyperspace gates
void MakeTargetsOnlyNonForceAttackTargets(SelectAnyCommand *selection,struct Player *curplayer);

// Makes targets only be salvageable ships/derelicts
void MakeTargetsSalvageable(SelectAnyCommand *selection,struct Player *curplayer);

typedef bool (*ShipConstraintsCB)(Ship *ship);
// Makes sure that all ships in selection follow shipConstraintsCB
void MakeShipsOnlyFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB);
// Returns TRUE if any ships in selection follow shipConstraintsCB
bool DoAnyShipsFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB);
// Returns TRUE if all ships in selection follow shipConstraintsCB
bool DoAllShipsFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB);

// Makes sure that ships in selection are friendly ships (are curplayer's ships)
void MakeShipsFriendlyShips(SelectCommand *selection,struct Player *curplayer);

// Makes sure that ships in the selection are friendly, i.e. allies and curplayer's ships
void MakeShipsFriendlyAndAlliesShips(SelectCommand *selection,struct Player *curplayer);

// Makes sure that ships are harvest capable
sdword MakeShipsHarvestCapable(SelectCommand *dest, SelectCommand *source);

// Makes sure that ships are attack capable
sdword MakeShipsAttackCapable(SelectCommand *dest, SelectCommand *source);
//void MakeShipsAttackCapable(SelectCommand *selection);

// Returns TRUE if all ships are attack capable
bool AreAllShipsAttackCapable(SelectCommand *selection);

// returns TRUE if any ships are attack capable
bool AreAnyShipsAttackCapable(SelectCommand *selection);

// Returns TRUE if ship is attack capable
bool isShipAttackCapable(Ship *ship);

// Returns TRUE if all ships are attack capable
bool AreAllShipsPassiveAttackCapable(SelectCommand *selection);

// returns TRUE if any ships are attack capable
bool AreAnyShipsPassiveAttackCapable(SelectCommand *selection);

// Returns TRUE if ship is attack capable
bool isShipPassiveAttackCapable(Ship *ship);

// Makes sure that ships are guard capable
void MakeShipsGuardCapable(SelectCommand *selection);

// Makes sure the ships have a special activate function
void MakeShipsSpecialActivateCapable(SelectCommand *selection);

// Makes sure the ships have special targeting capability
void MakeShipsSpecialTargetCapable(SelectCommand *selection, bool bFriendlies);

// Makes sure that ships are mobile
void MakeShipsMobile(SelectCommand *selection);
bool AreShipsMobile(SelectCommand *selection);

// Makes sure ships aren't capital ships
void MakeShipsNonCapital(SelectCommand *selection);

// Makes sure ships are only capital ships
void MakeShipsOnlyCapital(SelectCommand *selection);

// Makes sure selection don't have cloaked enemy ships
void MakeSelectionNotHaveCloakedEnemyShips(SelectCommand *selection, struct Player *curplayer);

// Makes sure selection doesn't have non-visible cloaked enemy ships
// Note: takes into account proximity sensors detecting cloaked ships
void MakeSelectionNotHaveNonVisibleEnemyShips(SelectCommand *selection, struct Player *curplayer);

// Adds slaves to a selection list if the master is in it
void MakeShipMastersIncludeSlaves(SelectCommand *selection);

// Removes Research Ships from Selection
void MakeSelectionNonResearchShips(SelectCommand *selection);

// removes ships that can't dock from the selection
void makeShipsDockCapable(SelectCommand *selection);
void makeShipsRetireable(SelectCommand *selection);

// rmakes list contain no docking ships <ships in docking cones>
void makeShipsControllable(SelectCommand *selection,sdword newCommand);

// makes list contains ships that can gointo formation..probe is removed
void makeShipsFormationCapable(SelectCommand *selection);

// removes the player's motership from selection
void makeShipsNotIncludeSinglePlayerMotherships(SelectCommand *selection);

//Removes non Kamikazeable ships
bool MakeSelectionKamikazeCapable(SelectCommand *selection);

//remove any ships not allowed to hyperspace
void makeSelectionHyperspaceCapable(SelectCommand *selection);

//removes certain ships from an attack selection so that guarding ships
//isn't wacky
void makeShipsNotHaveNonCombatShipsForGuardAttack(SelectCommand *selection);

// Selects all of the current player's non-hyperspacing ships
SelectCommand *selectAllCurrentPlayersNonHyperspacingShips(void);

// Selects all of the current player's hyperspacing ships
SelectCommand *selectAllCurrentPlayersHyperspacingShips(void);

// Selects all of a player's ships
SelectCommand *selectAllPlayersShips(struct Player *player);

// returns a new selection consisting of the ships in both selection 1 and selection 2
// Note: the new selection created with a memAlloc and therefore must be freed later
SelectCommand *selectSelectionIntersection(SelectCommand *selection1, SelectCommand *selection2);

//select all ships with the canSingleClickSpecialActivate flag set
sdword MakeShipsSingleClickSpecialCapable(SelectCommand *dest, SelectCommand *source);

//select all visible ships
sdword MakeShipsNotHidden(SelectCommand *dest, SelectCommand *source);

sdword MothershipOrCarrierIndexInSelection(SelectCommand *selection);

// Merge two selections
#define NO_DEALLOC      0   //no selections are deallocated
#define DEALLOC1        1   //the first selection is deallocated
#define DEALLOC2        2   //the second selection is deallocated
#define DEALLOC_BOTH    3   //both selections are deallocated
SelectCommand *selectMergeTwoSelections(SelectCommand *selection1, SelectCommand *selection2, udword dealloc);

// Duplicate a selection
SelectCommand *selectMemDupSelection(SelectCommand *selection, char *str, udword memflag);
#define selectDupSelection(sel)     selectMemDupSelection(sel, "selDup", 0)

#define ShipCanHyperspace(ship) (isCapitalShip(ship) && (ship->shiptype != CryoTray) && (ship->shiptype != Probe))

#endif

