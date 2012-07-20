/*=============================================================================
    Name    : AIUtilities.h
    Purpose : Utility functions, macros and defines for the Homeworld Computer Player

    Created 6/1/1998 by fpoiker
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef ___AIUTILITIES_H
#define ___AIUTILITIES_H

#include "Types.h"
#include "SpaceObj.h"
#include "Blobs.h"
#include "Memory.h"
#include "Vector.h"
#include "Universe.h"
#include "Volume.h"

bool ShipAlreadyHyperspaceOut(Ship *ship);      // don't include singleplayer.h, just this

/*=============================================================================
    Macros:
=============================================================================*/
#define aiumemFree(x)                   if ((x) != NULL) {memFree((x));(x) = NULL;}
#define sizeof_blob_array(x)            sizeof(blob_array) + ((x) - 1) * sizeof(blob *)
#define sizeof_aiblob_array(x)          sizeof(aiblob_array) + ((x) - 1) * sizeof(aiblob *)
#define sizeof_Path(x)                  sizeof(Path) + ((x) - 1) * sizeof(vector)
#define aiuShipIsHidden(x)              bitTest((x)->flags, SOF_Hide)
#define aiuShipIsntSelectable(x)        (bitTest((x)->flags, SOF_Hide|SOF_Disabled|SOF_Crazy) || ShipAlreadyHyperspaceOut(x))
#define aiuShipIsntTargetable(x,player) (bitTest((x)->flags, SOF_Hide|SOF_Hyperspace) || \
                                         ((bitTest((x)->flags, SOF_Cloaked) && (!proximityCanPlayerSeeShip((player), (x))))))

#define aiuShipIsWorthCapturing(x)      ((!isShipOfClass(x, CLASS_Fighter)) && (!isShipOfClass(x, CLASS_Corvette)))

#define num_is_odd(x)                   ((x) & 0x1)

#define blob_to_aiblob(blob, aiblob)    (aiblob)->centre    = (blob)->centre; \
                                        (aiblob)->radius    = (blob)->radius; \
                                        (aiblob)->blobShips = (blob)->blobShips;\
                                        (aiblob)->primaryenemystrength = 0; \
                                        (aiblob)->primaryenemyvalue    = 0; \
                                        (aiblob)->otherenemystrength   = 0; \
                                        (aiblob)->otherenemyvalue      = 0; \
                                        (aiblob)->goodGuystrength      = 0; \
                                        (aiblob)->goodGuyvalue         = 0;

#define aiuFuelAbove(ship, level)       ((100.0 * ((ship)->fuel/(ship)->staticinfo->maxfuel)) > (level))
#define aiuHealthAbove(ship, level)     ((100.0 * ((ship)->health*(ship)->staticinfo->oneOverMaxHealth)) > (level))

#define aiuNewSelection(sel, size, string)  \
                                        (sel) = (SelectCommand *)memAlloc(sizeofSelectCommand((size)), (string), 0);\
                                        (sel)->numShips = 0;
#define aiuNewPyroSelection(sel, size, string)  \
                                        (sel) = (SelectCommand *)memAlloc(sizeofSelectCommand((size)), (string), Pyrophoric);\
                                        (sel)->numShips = 0;


#define aiuRandomRange(prob, range)     randyrandombetween(RAN_AIPlayer, ((prob)-(range)), ((prob)+(range)))

#define aiuGameTimePassed(time)         (universe.totaltimeelapsed > (time))

#define aiuEnableResourceFeature(x)     (bitSet(aiplayer->ResourceFeatures, (x)))
#define aiuEnableAttackFeature(x)       (bitSet(aiplayer->AttackFeatures, (x)))
#define aiuEnableDefenseFeature(x)      (bitSet(aiplayer->DefenseFeatures, (x)))
#define aiuEnableTeamFeature(team, x)   (bitSet((team)->TeamFeatures, (x)))
#define aiuDisableResourceFeature(x)     (bitClear(aiplayer->ResourceFeatures, (x)))
#define aiuDisableAttackFeature(x)       (bitClear(aiplayer->AttackFeatures, (x)))
#define aiuDisableDefenseFeature(x)      (bitClear(aiplayer->DefenseFeatures, (x)))
#define aiuDisableTeamFeature(team, x)   (bitSet((team)->TeamFeatures, (x)))
#define aiuResourceFeatureEnabled(x)    (bitTest(aiCurrentAIPlayer->ResourceFeatures, (x)))
#define aiuAttackFeatureEnabled(x)      (bitTest(aiCurrentAIPlayer->AttackFeatures, (x)))
#define aiuDefenseFeatureEnabled(x)     (bitTest(aiCurrentAIPlayer->DefenseFeatures, (x)))
#define aiuTeamFeatureEnabled(team, x)  (bitTest((team)->TeamFeatures, (x)))

/*=============================================================================
    Constant Definitions:
=============================================================================*/
//#define NUM_FAST_PATROL_PATHPOINTS  4
#define FASTPATROL_STANDOFFDIST     2500

//flags for random standoff points
#define RSP_NORMAL      0
#define RSP_NEAR        1
#define RSP_FAR         2

//vulnerable ships flags
#define ALL_VULNERABLE_SHIPS    0
#define LARGE_VULNERABLE_SHIPS  1


#define ORIGIN_VECTOR {0,0,0}

/*=============================================================================
    Structures:
=============================================================================*/
typedef struct Path
{
    udword numPoints;
    bool   closed;
    vector point[1];
} Path;


/*=============================================================================
    Utility Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Coordinate and Vector Utility Functions:
-----------------------------------------------------------------------------*/
//generates a random vector
vector aiuGenerateRandomVector(void);

//generates a random standoff point
vector aiuGenerateRandomStandoffPoint(vector center, real32 radius, vector origin, udword flags);

//generates coordinates for a flank attack
vector aiuGenerateFlankCoordinates(vector center, vector origin, vector referencevec, real32 radius);

//finds the square of the distance between two points
real32 aiuFindDistanceSquared(vector point1, vector point2);

//finds a point that is within a range of a given destination
vector aiuFindRangeStandoffPoint(vector destination, vector location, real32 range);

//creates a path struct with every point at (-REALlyBig, -REALlyBig, -REALlyBig)
Path *aiuCreatePathStruct(udword numPoints, bool closed);

// allocates memory for and returns a new path (a perfect copy of the existing path)
Path *aiuPathDupe(Path *existing);

//adds a point to a path
void aiuAddPointToPath(vector point, udword pointnum, Path *path);

Path *aiuGenerateCircularPath(udword num_points, vector center, real32 radius, bool closed);

/*-----------------------------------------------------------------------------
    General Utility Functions:
-----------------------------------------------------------------------------*/
void aiuMoveShipSelection(MaxSelection *dest, MaxSelection *source, udword num);

/*-----------------------------------------------------------------------------
    Ship Related Utility Functions:
-----------------------------------------------------------------------------*/
struct AITeam;

//rescues (reinforces) a certain ship
bool aiuRescueShip(ShipPtr ship, struct AITeam *team);

//rescues (reinforces) a certain shiptype within the selection
ShipPtr aiuRescueShipType(SelectCommand *selection, struct AITeam *team, ShipType type);

//takes out a specific ship
bool aiuTakeoutShip(ShipPtr ship, struct AITeam *team);

//takes out a shiptype within a selection
ShipPtr aiuTakeoutShipType(SelectCommand *selection, struct AITeam *team, ShipType type);

//finds out if the ships are no longer attacking
bool aiuShipsNoLongerAttacking(SelectCommand *selection);

//finds out if the ship is an enemy mothership
bool aiuShipIsAnEnemyMothership(Ship *ship);

//finds out if a ship is not an enemy mothership
bool aiuShipIsntAnEnemyMothership(Ship *ship);

//finds out if any of the selection of ships is a capital ship
bool aiuAnyShipsAreCapitalShips(SelectCommand *ships);

//finds out if the ship is a fighter
bool aiuShipIsFighter(Ship *ship);

//finds out if the ship are not good against fighters
bool aiuShipNotGoodAgainstFighters(Ship *ship);

//function name pretty self explanitory
bool aiuSelectionNotGoodAtKillingTheseTargets(SelectCommand *selection, SelectCommand *targets, real32 strengthratio);

//finds out if any of the selection of ships is hidden
bool aiuShipsAreHidden(SelectCommand *selection);

//filters out any ship that is disabled
udword aiuFilterDisabledShips(SelectCommand *selection, MaxSelection *filtered);

//filters out any ship that isn't selectable
udword aiuFilterSelectableShips(SelectCommand *selection, MaxSelection *filtered);

//tells whether any of the selection of ships isn't targetable
bool aiuShipsArentTargetable(SelectCommand *selection);

//returns TRUE if ship is a dangerous ship
bool aiuIsShipDangerous(Ship *ship);

//returns TRUE if the ship is a primary enemy
bool aiuIsShipPrimaryEnemy(Ship *ship);

void aiuRateShip(udword *strength, udword *value, ShipPtr ship);

//Returns the enemy ships found in the mothership blob
SelectCommand *aiuEnemyShipsInMothershipBlob(void);

//finds the nearest enemy ship in the mothership blob
ShipPtr aiuFindNearestMothershipAttacker(SelectCommand *teamShips);

//returns the player's mothership coordinates
vector aiuPlayerMothershipCoords(Player *player);

// Returns the enemy mothership of player
Ship *aiuFindEnemyMothership(Player *player);

// eliminates ships that aren't dangerous to the mothership from the selection
void aiuMakeShipsOnlyDangerousToMothership(SelectCommand *enemyships);

//finds the coordinates of the enemy mothership
vector aiuFindEnemyMothershipCoords(Player *player);

//returns TRUE if ships are less than dist away from the current players' enemy mothership
bool aiuShipsCloseToEnemyMothership(Player *player, SelectCommand *ships, real32 dist);

//finds the safest point a given distance away from the center of the
//selected ships
vector aiuFindSafestStandoffPoint(SelectCommand *ships, real32 distance);

//generates a patrol path for fast ships
Path *aiuGeneratePatrolPath(udword patroltype);

//finds all the unarmed undefended enemy ships
SelectCommand *aiuFindUnarmedUndefendedEnemyShips(void);

//finds all the unarmed enemy ships
SelectCommand *aiuFindUnarmedEnemyShips(void);

//finds the best volume to mine
Volume *aiuFindBestVolumeToMine(SelectCommand *selection);

//finds all of a player's resource collectors
SelectCommand *aiuFindResourceCollectors(void);

//finds an ideal place for the resource controller to go
vector aiuFindResourceControllerDestination(void);

//makes sure no other mothership is moving towards or near this destination
bool aiuDestinationNotNearOtherMothership(vector destination, real32 radiussq);

//finds the slowest ship in the selection
real32 aiuFindSlowestShipMaxSpeed(SelectCommand *ships);

//returns the ship in the selection that is closest to "ship"
ShipPtr aiuGetClosestShip(SelectCommand *selection, ShipPtr ship);

//finds a small selection of unarmed or lightly armed good guy ships
SelectCommand *aiuFindClosestVulnerableGoodGuyShips(ShipPtr ship, udword type);

//weeds out ships that are farther away from a reference point than a specified point
SelectCommand *aiuRejectShipsPastLocation(SelectCommand *selection, vector reference, vector location);

//finds the closest unarmed undefended enemy ship
ShipPtr aiuFindClosestUnarmedUndefendedEnemyShip(ShipPtr ship);

//finds the best unarmed enemy ship
ShipPtr aiuFindBestUnarmedEnemyShip(SelectCommand *teamShips, real32 range);

//finds dangerous enemy ships close to primarytarget
SelectCommand *aiuFindNearbyDangerousEnemyShips(Ship *primarytarget,real32 range);

//makes targets only be primary enemy ships
void aiuMakeShipsOnlyPrimaryEnemyShips(SelectCommand *selection);

//finds the ships attacking "ship"
SelectCommand *aiuFindAttackingShips(ShipPtr ship);

//finds enemy ships close to primarytarget
SelectCommand *aiuFindNearbyEnemyShips(Ship *primarytarget,real32 range);

//finds the most likely enemy target to harass
ShipPtr aiuFindHarassTarget(SelectCommand *teamShips);

//finds the juciest target for capturing
//note: SelectCommand structure not used as normal, see function comments
SelectCommand *aiuFindCaptureTarget(SelectCommand *capturingShips);

//finds the nearest target from the aiCurrentAIPlayer->Targets list
SelectCommand *aiuFindSwarmerTarget(struct AITeam *team);

//finds targets for the P2 swarmers
SelectCommand *aiuFindSwarmTargets(SelectCommand *teamShips, udword numTargets, bool harass);

//finds out if a point is within the sphere of influence
bool aiuPointWithinSphereOfInfluence(vector point);

//finds one ship in a group of ships within the computer player's sphere of influence
SelectCommand *aiuFindLeadShipInSphereOfInfluence(vector centre, real32 radiussq);

//finds standoff points to the next nearest blob (going outwards from or inwards towards the source from the holder)
vector aiuFindStandoffOfNextNearestBlobOut(ShipPtr source, ShipPtr holder);
vector aiuFindStandoffOfNextNearestBlobIn(ShipPtr source, ShipPtr holder);
vector aiuFindStandoffOfNextNearestEnemyBlobOut(ShipPtr source, ShipPtr holder);
vector aiuFindStandoffOfNextNearestEnemyBlobIn(ShipPtr source, ShipPtr holder);

//returns all the resources in a particular volume
ResourceSelection *aiuFindResourcesInVolume(Volume volume);

//returns true if resourcer is busy
bool aiuShipIsResourcingOrSomething(ShipPtr ship);

//returns the best resource for the ship to harvest
Resource *aiuFindBestResource(Resource **biggetsResource, ShipPtr ship, ResourceSelection *resources);

//returns a rating for a blob w.r.t resourcing
real32 aiuRateBlobForResourcing(blob *thisBlob);

//finds the best blob to collect resources
bool aiuFindBestResourceBlob(vector *destination);

//finds the nearest resource blob
blob *aiuFindNearestResourceBlob(SelectCommand *ships);

//find the aiblob the ship is in
//returns NULL if the ships are not in the same blob if retentive == TRUE
struct aiblob *aiuFindShipsAIBlob(SelectCommand *ships, bool retentive);

//same as above, but for one ship (doesn't return NULL)
struct aiblob *aiuFindShipAIBlob(ShipPtr ship);

//returns TRUE if the enemy ship is visible
bool aiuEnemyShipIsVisible(ShipPtr ship);

//returns the smallest gun range distance among the armed ships in "ships"
//    Note: This function scales the minimum gun ranges to work better
//          with the move attack command
real32 aiuFindMinimumGunRangeSquared(SelectCommand *ships);

//returns TRUE if the ships in selection 1 are in gun range of their attack targets
bool aiuShipsInGunRangeOfTargets(SelectCommand *selection1);

//returns the number of ships an aiplayer can build according to unit caps
udword aiuUnitCapCanBuildShip(struct AIPlayer *aiplayer, ShipType shiptype, sdword numShips);

/*-----------------------------------------------------------------------------
    Armada Functions:
-----------------------------------------------------------------------------*/
bool aiuFindArmadaTarget(vector *dest, SelectCommand **sel_target, SelectCommand *team_ships);

/*-----------------------------------------------------------------------------
    Wrap Functions:
-----------------------------------------------------------------------------*/
//computer player wrap of the attack command
SelectCommand *aiuAttack(struct AITeam *team, SelectCommand *targets);

//splits the attackers into smaller groups.  Recommended for strikecraft vs strikecraft battles only
void aiuSplitAttack(SelectCommand *attackers, SelectCommand *targets);

//special swarm dock command
void aiuSwarmDock(SelectCommand *ships, SelectCommand *Pods);

bool aiuWrapAttack(SelectCommand *attackers, SelectCommand *targets);
bool aiuWrapMove(SelectCommand *ships, vector destination);
bool aiuWrapDock(SelectCommand *ships, DockType docktype, ShipPtr dockwith);
bool aiuWrapFormation(SelectCommand *ships, TypeOfFormation formation);
bool aiuWrapProtect(SelectCommand *ships, SelectCommand *shipstoguard);
bool aiuWrapSpecial(SelectCommand *ships, SelectCommand *targets);
bool aiuWrapCollectResource(SelectCommand *ships, ResourcePtr resource);
bool aiuWrapSetTactics(SelectCommand *ships, TacticsType tactics);
bool aiuWrapHalt(SelectCommand *ships);
bool aiuWrapScuttle(SelectCommand *ships);
bool aiuWrapSetKamikaze(SelectCommand *ships);

// like aiuWrapFormation, except doesn't filter out hidden ships
bool aiuKasWrapFormation(SelectCommand *ships, TypeOfFormation formation);

//gets the collision blob of the selection of ships
blob *aiuWrapGetCollBlob(SelectCommand *ships);

/*-----------------------------------------------------------------------------
    Enemy Stats Utility Functions:
-----------------------------------------------------------------------------*/
//makes changes in the aiplayer structure if the primary enemy changes
void aiuChangePrimaryEnemy(struct AIPlayer *aiplayer);

//finds cloak capable ships in the gathered enemy statistics
bool aiuFindCloakersInEnemyShipsIAmAwareOf(bool Red_Alert);


/*-----------------------------------------------------------------------------
    Blob Utility Functions:
-----------------------------------------------------------------------------*/
//returns the number of enemy blobs
udword aiuGetNumEnemyBlobs();

//fills the EnemyShipsIAmAwareOf arrays in the aiplayer struct
void aiuUpdateKnowledgeOfEnemyShips(struct AIPlayer *aiplayer);

//creates two arrays of pointers to the blobs containing enemy ship
//and the good guy ships
void aiuCreateBlobArrays(Player *player);

//deletes the two blob arrays
void aiuDeleteBlobArrays(void);

// Returns TRUE if current AI player can build ships of shiptype
bool aiuCanBuildShipType(ShipType shiptype,bool scriptteam);

// Returns TRUE if current AI player already has num ships of shiptype
bool aiuAlreadyHasShipType(ShipType shiptype, udword num);

#endif
