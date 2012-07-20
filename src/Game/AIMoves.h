#ifndef __AIMOVES_H
#define __AIMOVES_H

#include "Vector.h"
#include "Volume.h"
#include "Formation.h"

struct AITeam;
struct AITeamMove;
struct Path;
struct AlternativeShips;

/*=============================================================================
    Move Definitions:
=============================================================================*/
//moveto types
#define TIME_LIMITED        0
#define DISTANCE_LIMITED    1

/*=============================================================================
    Utility Functions:
=============================================================================*/
void aimInsertMove(struct AITeam *team, struct AITeamMove *move);

/*=============================================================================
    Move Process Functions:
=============================================================================*/
sdword aimProcessVarSet(struct AITeam *team);
sdword aimProcessVarInc(struct AITeam *team);
sdword aimProcessVarDec(struct AITeam *team);
sdword aimProcessVarWait(struct AITeam *team);
sdword aimProcessVarDestroy(struct AITeam *team);
sdword aimProcessGuardShips(struct AITeam *team);
sdword aimProcessFormation(struct AITeam *team);
sdword aimProcessGetShips(struct AITeam *team);
sdword aimProcessMoveDone(struct AITeam *team);
sdword aimProcessSpecial(struct AITeam *team);
sdword aimProcessAttack(struct AITeam *team);
sdword aimProcessAdvancedAttack(struct AITeam *team);
sdword aimProcessMoveAttack(struct AITeam *team);
sdword aimProcessMoveTeam(struct AITeam *team);
sdword aimProcessIntercept(struct AITeam *team);
sdword aimProcessMoveTo(struct AITeam *team);
sdword aimProcessMoveSplit(struct AITeam *team);
sdword aimProcessCountShips(struct AITeam *team);
sdword aimProcessHarassAttack(struct AITeam *team);
sdword aimProcessFancyGetShips(struct AITeam *team);
sdword aimProcessGuardCooperatingTeam(struct AITeam *team);
sdword aimProcessDefendMothership(struct AITeam *team);
sdword aimProcessPatrolMove(struct AITeam *team);
sdword aimProcessActivePatrol(struct AITeam *team);
sdword aimProcessTempGuard(struct AITeam *team);
sdword aimProcessSupport(struct AITeam *team);
sdword aimProcessSwarmAttack(struct AITeam *team);
sdword aimProcessLaunch(struct AITeam *team);
sdword aimProcessResourceVolume(struct AITeam *team);
sdword aimProcessCapture(struct AITeam *team);
sdword aimProcessActiveCapture(struct AITeam *team);
sdword aimProcessActiveMine(struct AITeam *team);
sdword aimProcessMineVolume(struct AITeam *team);
sdword aimProcessSpecialDefense(struct AITeam *team);
sdword aimProcessKamikaze(struct AITeam *team);

/*=============================================================================
    Move Creation Functions:
=============================================================================*/
struct AITeamMove *aimCreateVarSet(struct AITeam *team, char *varName, sdword value, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateVarInc(struct AITeam *team, char *varName, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateVarDec(struct AITeam *team, char *varName, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateVarWait(struct AITeam *team, char *varName, sdword value, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateVarDestroy(struct AITeam *team, char *varName, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateGuardShips(struct AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateFormation(struct AITeam *team, TypeOfFormation formationtype, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateGetShips(struct AITeam *team, ShipType shiptype, sbyte num_ships, sdword priority, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveDone(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSpecial(struct AITeam *team, SelectCommand *targets, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateAttack(struct AITeam *team, SelectCommand *targets, TypeOfFormation formation, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateAdvancedAttack(struct AITeam *team, SelectCommand *targets, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateFlankAttack(struct AITeam *team, SelectCommand *targets, bool8 hyperspace, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveAttack(struct AITeam *team, SelectCommand *targets, bool Advanced, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveTeam(struct AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveTeamIndex(struct AITeam *team, vector destination, udword index, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveTeamSplit(struct AITeam *team, SelectCommand *ships, struct Path *destinations, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateHyperspace(struct AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateIntercept(struct AITeam *team, ShipPtr ship, real32 interval, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveTo(struct AITeam *team, vector destination, real32 limiter, udword type, TypeOfFormation formation, TacticsType tactics, bool wait, bool remove);
struct AITeamMove *aimCreateCountShips(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateHarassAttack(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateFancyGetShips(struct AITeam *team, ShipType shiptype, sbyte num_ships, struct AlternativeShips *alternatives, sdword priority, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateDock(struct AITeam *team, sdword dockmoveFlags, ShipPtr dockAt, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateLaunch(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateGuardCooperatingTeam(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateDefendMothership(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreatePatrolMove(struct AITeam *team, struct Path *path, udword startIndex, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActivePatrol(struct AITeam *team, udword patroltype, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateTempGuard(struct AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateReinforce(struct AITeam *team, struct AITeam *reinforceteam, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSupport(struct AITeam *team, SelectCommand *ships, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActiveRecon(struct AITeam *team, bool EnemyRecon, TypeOfFormation formation,TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateShipRecon(struct AITeam *team, SelectCommand *ships, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateArmada(struct AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateControlResources(struct AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSwarmAttack(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSwarmDefense(struct AITeam *team, SelectCommand *pods, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSwarmPod(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateResourceVolume(struct AITeam *team, Volume volume, bool8 strictVolume, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActiveResource(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMothershipMove(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateCapture(struct AITeam *team, ShipPtr ship, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActiveCapture(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActiveMine(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMineVolume(struct AITeam *team, Volume volume, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSpecialDefense(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateDeleteTeam(struct AITeam *team);
struct AITeamMove *aimCreateKamikaze(struct AITeam *team, SelectCommand *targets,TypeOfFormation formation, bool8 wait, bool8 remove);

/*=============================================================================
    Move Creation Without Linked List Add:
=============================================================================*/
struct AITeamMove *aimCreateFormationNoAdd(struct AITeam *team, TypeOfFormation formationtype, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateFancyGetShipsNoAdd(struct AITeam *team, ShipType shiptype, sbyte num_ships, struct AlternativeShips *alternatives, sdword priority, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSpecialNoAdd(struct AITeam *team, SelectCommand *targets, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateAttackNoAdd(struct AITeam *team, SelectCommand *targets, TypeOfFormation formation, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateAdvancedAttackNoAdd(struct AITeam *team, SelectCommand *target, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateFlankAttackNoAdd(struct AITeam *team, SelectCommand *targets, bool8 hyperspace, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveAttackNoAdd(struct AITeam *team, SelectCommand *targets, bool Advanced, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveTeamNoAdd(struct AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveTeamIndexNoAdd(struct AITeam *team, vector destination, udword index, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveTeamSplitNoAdd(struct AITeam *team, SelectCommand *ships, struct Path *destinations, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateHyperspaceNoAdd(struct AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateInterceptNoAdd(struct AITeam *team, ShipPtr ship, real32 interval, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMoveToNoAdd(struct AITeam *team, vector destination, real32 limiter, udword type, TypeOfFormation formation, TacticsType tactics, bool wait, bool remove);
struct AITeamMove *aimCreateCountShipsNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateDockNoAdd(struct AITeam *team, sdword dockmoveFlags, ShipPtr dockAt, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateLaunchNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateGuardCooperatingTeamNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateDefendMothershipNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreatePatrolMoveNoAdd(struct AITeam *team, struct Path *path, udword startIndex, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActivePatrolNoAdd(struct AITeam *team, udword patroltype, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateGetShipsNoAdd(struct AITeam *team, ShipType shiptype, sbyte num_ships, sdword priority, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateTempGuardNoAdd(struct AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateReinforceNoAdd(struct AITeam *team, struct AITeam *reinforceteam, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSupportNoAdd(struct AITeam *team, SelectCommand *ships, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActiveReconNoAdd(struct AITeam *team, bool EnemyRecon, TypeOfFormation formation,TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateShipReconNoAdd(struct AITeam *team, SelectCommand *ships, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateArmadaNoAdd(struct AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateControlResourcesNoAdd(struct AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSwarmAttackNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSwarmDefenseNoAdd(struct AITeam *team, SelectCommand *pods, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateResourceVolumeNoAdd(struct AITeam *team, Volume volume, bool8 strictVolume, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActiveResourceNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMothershipMove(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateCaptureNoAdd(struct AITeam *team, ShipPtr ship, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActiveCaptureNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateActiveMineNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateMineVolumeNoAdd(struct AITeam *team, Volume vol, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateSpecialDefenseNoAdd(struct AITeam *team, bool8 wait, bool8 remove);
struct AITeamMove *aimCreateDeleteTeamNoAdd(struct AITeam *team);
struct AITeamMove *aimCreateKamikazeNoAdd(struct AITeam *team, SelectCommand *targets,TypeOfFormation formation, bool8 wait, bool8 remove);

/*=============================================================================
    Save Move Functions
=============================================================================*/

void aimFix_MoveDone(struct AITeamMove *move);
void aimFix_GuardShips(struct AITeamMove *move);
void aimFix_DefMoship(struct AITeamMove *move);
void aimFix_TempGuard(struct AITeamMove *move);
void aimFix_GetShips(struct AITeamMove *move);
void aimFix_Formation(struct AITeamMove *move);
void aimFix_MoveTeam(struct AITeamMove *move);
void aimFix_MoveTeamIndex(struct AITeamMove *move);
void aimFix_MoveTeamSplit(struct AITeamMove *move);
void aimFix_Hyperspace(struct AITeamMove *move);
void aimFix_Intercept(struct AITeamMove *move);
void aimFix_MoveTo(struct AITeamMove *move);
void aimFix_PatrolMove(struct AITeamMove *move);
void aimFix_ActivePatrol(struct AITeamMove *move);
void aimFix_ActiveRecon(struct AITeamMove *move);
void aimFix_ShipRecon(struct AITeamMove *move);
void aimFix_CountShips(struct AITeamMove *move);
void aimFix_Special(struct AITeamMove *move);
void aimFix_Attack(struct AITeamMove *move);
void aimFix_AdvancedAttack(struct AITeamMove *move);
void aimFix_FlankAttack(struct AITeamMove *move);
void aimFix_MoveAttack(struct AITeamMove *move);
void aimFix_HarassAttack(struct AITeamMove *move);
void aimFix_FancyGetShips(struct AITeamMove *move);
void aimFix_Dock(struct AITeamMove *move);
void aimFix_Launch(struct AITeamMove *move);
void aimFix_Reinforce(struct AITeamMove *move);
void aimFix_VarSet(struct AITeamMove *move);
void aimFix_VarInc(struct AITeamMove *move);
void aimFix_VarDec(struct AITeamMove *move);
void aimFix_VarWait(struct AITeamMove *move);
void aimFix_VarDestroy(struct AITeamMove *move);
void aimFix_GuardCoopTeam(struct AITeamMove *move);
void aimFix_Support(struct AITeamMove *move);
void aimFix_Armada(struct AITeamMove *move);
void aimFix_ControlResources(struct AITeamMove *move);
void aimFix_SwarmAttack(struct AITeamMove *move);
void aimFix_SwarmDefense(struct AITeamMove *move);
void aimFix_SwarmPod(struct AITeamMove *move);
void aimFix_ResourceVolume(struct AITeamMove *move);
void aimFix_DeleteTeam(struct AITeamMove *move);
void aimFix_Capture(struct AITeamMove *move);
void aimFix_ActiveCapture(struct AITeamMove *move);
void aimFix_ActiveMine(struct AITeamMove *move);
void aimFix_MineVolume(struct AITeamMove *move);
void aimFix_SpecialDefense(struct AITeamMove *move);
void aimFix_ActiveResource(struct AITeamMove *move);
void aimFix_MothershipMove(struct AITeamMove *move);
void aimFix_Kamikaze(struct AITeamMove *move);

void aimLoad_GuardShips(struct AITeamMove *move);
void aimLoad_DefMoship(struct AITeamMove *move);
void aimLoad_PatrolMove(struct AITeamMove *move);
void aimLoad_MoveTeamSplit(struct AITeamMove *move);
void aimLoad_Special(struct AITeamMove *move);
void aimLoad_Attack(struct AITeamMove *move);
void aimLoad_AdvancedAttack(struct AITeamMove *move);
void aimLoad_FlankAttack(struct AITeamMove *move);
void aimLoad_MoveAttack(struct AITeamMove *move);
void aimLoad_Dock(struct AITeamMove *move);
void aimLoad_Support(struct AITeamMove *move);
void aimLoad_ShipRecon(struct AITeamMove *move);
void aimLoad_ControlResources(struct AITeamMove *move);
void aimLoad_SwarmAttack(struct AITeamMove *move);
void aimLoad_SwarmDefense(struct AITeamMove *move);
void aimLoad_ResourceVolume(struct AITeamMove *move);
void aimLoad_Kamikaze(struct AITeamMove *move);

void aimSave_GuardShips(struct AITeamMove *move);
void aimSave_DefMoship(struct AITeamMove *move);
void aimSave_PatrolMove(struct AITeamMove *move);
void aimSave_MoveTeamSplit(struct AITeamMove *move);
void aimSave_Special(struct AITeamMove *move);
void aimSave_Attack(struct AITeamMove *move);
void aimSave_AdvancedAttack(struct AITeamMove *move);
void aimSave_FlankAttack(struct AITeamMove *move);
void aimSave_MoveAttack(struct AITeamMove *move);
void aimSave_Dock(struct AITeamMove *move);
void aimSave_Support(struct AITeamMove *move);
void aimSave_ShipRecon(struct AITeamMove *move);
void aimSave_ControlResources(struct AITeamMove *move);
void aimSave_SwarmAttack(struct AITeamMove *move);
void aimSave_SwarmDefense(struct AITeamMove *move);
void aimSave_ResourceVolume(struct AITeamMove *move);
void aimSave_Kamikaze(struct AITeamMove *move);

void aimPreFix_Dock(struct AITeamMove *move);
void aimPreFix_GetShips(struct AITeamMove *move);
void aimPreFix_Intercept(struct AITeamMove *move);
void aimPreFix_PatrolMove(struct AITeamMove *move);
void aimPreFix_AdvancedAttack(struct AITeamMove *move);
void aimPreFix_MoveAttack(struct AITeamMove *move);
void aimPreFix_HarassAttack(struct AITeamMove *move);
void aimPreFix_FancyGetShips(struct AITeamMove *move);
void aimPreFix_Reinforce(struct AITeamMove *move);

#endif
