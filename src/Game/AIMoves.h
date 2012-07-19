#ifndef __AIMOVES_H
#define __AIMOVES_H

#include "vector.h"
#include "volume.h"

/*=============================================================================
    Move Definitions:
=============================================================================*/
//moveto types
#define TIME_LIMITED        0
#define DISTANCE_LIMITED    1

/*=============================================================================
    Utility Functions:
=============================================================================*/
void aimInsertMove(AITeam *team, struct AITeamMove *move);

/*=============================================================================
    Move Process Functions:
=============================================================================*/
sdword aimProcessVarSet(AITeam *team);
sdword aimProcessVarInc(AITeam *team);
sdword aimProcessVarDec(AITeam *team);
sdword aimProcessVarWait(AITeam *team);
sdword aimProcessVarDestroy(AITeam *team);
sdword aimProcessGuardShips(AITeam *team);
sdword aimProcessFormation(AITeam *team);
sdword aimProcessGetShips(AITeam *team);
sdword aimProcessMoveDone(AITeam *team);
sdword aimProcessSpecial(AITeam *team);
sdword aimProcessAttack(AITeam *team);
sdword aimProcessAdvancedAttack(AITeam *team);
sdword aimProcessMoveAttack(AITeam *team);
sdword aimProcessMoveTeam(AITeam *team);
sdword aimProcessIntercept(AITeam *team);
sdword aimProcessMoveTo(AITeam *team);
sdword aimProcessMoveSplit(AITeam *team);
sdword aimProcessCountShips(AITeam *team);
sdword aimProcessHarassAttack(AITeam *team);
sdword aimProcessFancyGetShips(AITeam *team);
sdword aimProcessGuardCooperatingTeam(AITeam *team);
sdword aimProcessDefendMothership(AITeam *team);
sdword aimProcessPatrolMove(AITeam *team);
sdword aimProcessActivePatrol(AITeam *team);
sdword aimProcessTempGuard(AITeam *team);
sdword aimProcessSupport(AITeam *team);
sdword aimProcessSwarmAttack(AITeam *team);
sdword aimProcessLaunch(AITeam *team);
sdword aimProcessResourceVolume(AITeam *team);
sdword aimProcessCapture(AITeam *team);
sdword aimProcessActiveCapture(AITeam *team);
sdword aimProcessActiveMine(AITeam *team);
sdword aimProcessMineVolume(AITeam *team);
sdword aimProcessSpecialDefense(AITeam *team);
sdword aimProcessKamikaze(AITeam *team);

/*=============================================================================
    Move Creation Functions:
=============================================================================*/
AITeamMove *aimCreateVarSet(AITeam *team, char *varName, sdword value, bool8 wait, bool8 remove);
AITeamMove *aimCreateVarInc(AITeam *team, char *varName, bool8 wait, bool8 remove);
AITeamMove *aimCreateVarDec(AITeam *team, char *varName, bool8 wait, bool8 remove);
AITeamMove *aimCreateVarWait(AITeam *team, char *varName, sdword value, bool8 wait, bool8 remove);
AITeamMove *aimCreateVarDestroy(AITeam *team, char *varName, bool8 wait, bool8 remove);
AITeamMove *aimCreateGuardShips(AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove);
AITeamMove *aimCreateFormation(AITeam *team, TypeOfFormation formationtype, bool8 wait, bool8 remove);
AITeamMove *aimCreateGetShips(AITeam *team, ShipType shiptype, sbyte num_ships, sdword priority, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveDone(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateSpecial(AITeam *team, SelectCommand *targets, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateAttack(AITeam *team, SelectCommand *targets, TypeOfFormation formation, bool8 wait, bool8 remove);
AITeamMove *aimCreateAdvancedAttack(AITeam *team, SelectCommand *targets, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateFlankAttack(AITeam *team, SelectCommand *targets, bool8 hyperspace, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveAttack(AITeam *team, SelectCommand *targets, bool Advanced, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveTeam(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveTeamIndex(AITeam *team, vector destination, udword index, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveTeamSplit(AITeam *team, SelectCommand *ships, Path *destinations, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateHyperspace(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove);
AITeamMove *aimCreateIntercept(AITeam *team, ShipPtr ship, real32 interval, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveTo(AITeam *team, vector destination, real32 limiter, udword type, TypeOfFormation formation, TacticsType tactics, bool wait, bool remove);
AITeamMove *aimCreateCountShips(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateHarassAttack(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateFancyGetShips(AITeam *team, ShipType shiptype, sbyte num_ships, AlternativeShips *alternatives, sdword priority, bool8 wait, bool8 remove);
AITeamMove *aimCreateDock(AITeam *team, sdword dockmoveFlags, ShipPtr dockAt, bool8 wait, bool8 remove);
AITeamMove *aimCreateLaunch(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateGuardCooperatingTeam(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateDefendMothership(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreatePatrolMove(AITeam *team, Path *path, udword startIndex, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateActivePatrol(AITeam *team, udword patroltype, bool8 wait, bool8 remove);
AITeamMove *aimCreateTempGuard(AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateReinforce(AITeam *team, AITeam *reinforceteam, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateSupport(AITeam *team, SelectCommand *ships, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateActiveRecon(AITeam *team, bool EnemyRecon, TypeOfFormation formation,TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateShipRecon(AITeam *team, SelectCommand *ships, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateArmada(AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateControlResources(AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove);
AITeamMove *aimCreateSwarmAttack(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateSwarmDefense(AITeam *team, SelectCommand *pods, bool8 wait, bool8 remove);
AITeamMove *aimCreateSwarmPod(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateResourceVolume(AITeam *team, Volume volume, bool8 strictVolume, bool8 wait, bool8 remove);
AITeamMove *aimCreateActiveResource(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateMothershipMove(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateCapture(AITeam *team, ShipPtr ship, bool8 wait, bool8 remove);
AITeamMove *aimCreateActiveCapture(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateActiveMine(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateMineVolume(AITeam *team, Volume volume, bool8 wait, bool8 remove);
AITeamMove *aimCreateSpecialDefense(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateDeleteTeam(AITeam *team);
AITeamMove *aimCreateKamikaze(AITeam *team, SelectCommand *targets,TypeOfFormation formation, bool8 wait, bool8 remove);

/*=============================================================================
    Move Creation Without Linked List Add:
=============================================================================*/
AITeamMove *aimCreateFormationNoAdd(AITeam *team, TypeOfFormation formationtype, bool8 wait, bool8 remove);
AITeamMove *aimCreateFancyGetShipsNoAdd(AITeam *team, ShipType shiptype, sbyte num_ships, AlternativeShips *alternatives, sdword priority, bool8 wait, bool8 remove);
AITeamMove *aimCreateSpecialNoAdd(AITeam *team, SelectCommand *targets, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateAttackNoAdd(AITeam *team, SelectCommand *targets, TypeOfFormation formation, bool8 wait, bool8 remove);
AITeamMove *aimCreateAdvancedAttackNoAdd(AITeam *team, SelectCommand *target, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateFlankAttackNoAdd(AITeam *team, SelectCommand *targets, bool8 hyperspace, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveAttackNoAdd(AITeam *team, SelectCommand *targets, bool Advanced, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveTeamNoAdd(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveTeamIndexNoAdd(AITeam *team, vector destination, udword index, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveTeamSplitNoAdd(AITeam *team, SelectCommand *ships, Path *destinations, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateHyperspaceNoAdd(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove);
AITeamMove *aimCreateInterceptNoAdd(AITeam *team, ShipPtr ship, real32 interval, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateMoveToNoAdd(AITeam *team, vector destination, real32 limiter, udword type, TypeOfFormation formation, TacticsType tactics, bool wait, bool remove);
AITeamMove *aimCreateCountShipsNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateDockNoAdd(AITeam *team, sdword dockmoveFlags, ShipPtr dockAt, bool8 wait, bool8 remove);
AITeamMove *aimCreateLaunchNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateGuardCooperatingTeamNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateDefendMothershipNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreatePatrolMoveNoAdd(AITeam *team, Path *path, udword startIndex, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateActivePatrolNoAdd(AITeam *team, udword patroltype, bool8 wait, bool8 remove);
AITeamMove *aimCreateGetShipsNoAdd(AITeam *team, ShipType shiptype, sbyte num_ships, sdword priority, bool8 wait, bool8 remove);
AITeamMove *aimCreateTempGuardNoAdd(AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateReinforceNoAdd(AITeam *team, AITeam *reinforceteam, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateSupportNoAdd(AITeam *team, SelectCommand *ships, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateActiveReconNoAdd(AITeam *team, bool EnemyRecon, TypeOfFormation formation,TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateShipReconNoAdd(AITeam *team, SelectCommand *ships, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateArmadaNoAdd(AITeam *team, TypeOfFormation formation, TacticsType tactics, bool8 wait, bool8 remove);
AITeamMove *aimCreateControlResourcesNoAdd(AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove);
AITeamMove *aimCreateSwarmAttackNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateSwarmDefenseNoAdd(AITeam *team, SelectCommand *pods, bool8 wait, bool8 remove);
AITeamMove *aimCreateResourceVolumeNoAdd(AITeam *team, Volume volume, bool8 strictVolume, bool8 wait, bool8 remove);
AITeamMove *aimCreateActiveResourceNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateMothershipMove(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateCaptureNoAdd(AITeam *team, ShipPtr ship, bool8 wait, bool8 remove);
AITeamMove *aimCreateActiveCaptureNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateActiveMineNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateMineVolumeNoAdd(AITeam *team, Volume vol, bool8 wait, bool8 remove);
AITeamMove *aimCreateSpecialDefenseNoAdd(AITeam *team, bool8 wait, bool8 remove);
AITeamMove *aimCreateDeleteTeamNoAdd(AITeam *team);
AITeamMove *aimCreateKamikazeNoAdd(AITeam *team, SelectCommand *targets,TypeOfFormation formation, bool8 wait, bool8 remove);

/*=============================================================================
    Save Move Functions
=============================================================================*/

void aimFix_MoveDone(AITeamMove *move);
void aimFix_GuardShips(AITeamMove *move);
void aimFix_DefMoship(AITeamMove *move);
void aimFix_TempGuard(AITeamMove *move);
void aimFix_GetShips(AITeamMove *move);
void aimFix_Formation(AITeamMove *move);
void aimFix_MoveTeam(AITeamMove *move);
void aimFix_MoveTeamIndex(AITeamMove *move);
void aimFix_MoveTeamSplit(AITeamMove *move);
void aimFix_Hyperspace(AITeamMove *move);
void aimFix_Intercept(AITeamMove *move);
void aimFix_MoveTo(AITeamMove *move);
void aimFix_PatrolMove(AITeamMove *move);
void aimFix_ActivePatrol(AITeamMove *move);
void aimFix_ActiveRecon(AITeamMove *move);
void aimFix_ShipRecon(AITeamMove *move);
void aimFix_CountShips(AITeamMove *move);
void aimFix_Special(AITeamMove *move);
void aimFix_Attack(AITeamMove *move);
void aimFix_AdvancedAttack(AITeamMove *move);
void aimFix_FlankAttack(AITeamMove *move);
void aimFix_MoveAttack(AITeamMove *move);
void aimFix_HarassAttack(AITeamMove *move);
void aimFix_FancyGetShips(AITeamMove *move);
void aimFix_Dock(AITeamMove *move);
void aimFix_Launch(AITeamMove *move);
void aimFix_Reinforce(AITeamMove *move);
void aimFix_VarSet(AITeamMove *move);
void aimFix_VarInc(AITeamMove *move);
void aimFix_VarDec(AITeamMove *move);
void aimFix_VarWait(AITeamMove *move);
void aimFix_VarDestroy(AITeamMove *move);
void aimFix_GuardCoopTeam(AITeamMove *move);
void aimFix_Support(AITeamMove *move);
void aimFix_Armada(AITeamMove *move);
void aimFix_ControlResources(AITeamMove *move);
void aimFix_SwarmAttack(AITeamMove *move);
void aimFix_SwarmDefense(AITeamMove *move);
void aimFix_SwarmPod(AITeamMove *move);
void aimFix_ResourceVolume(AITeamMove *move);
void aimFix_DeleteTeam(AITeamMove *move);
void aimFix_Capture(AITeamMove *move);
void aimFix_ActiveCapture(AITeamMove *move);
void aimFix_ActiveMine(AITeamMove *move);
void aimFix_MineVolume(AITeamMove *move);
void aimFix_SpecialDefense(AITeamMove *move);
void aimFix_ActiveResource(AITeamMove *move);
void aimFix_MothershipMove(AITeamMove *move);
void aimFix_Kamikaze(AITeamMove *move);

void aimLoad_GuardShips(AITeamMove *move);
void aimLoad_DefMoship(AITeamMove *move);
void aimLoad_PatrolMove(AITeamMove *move);
void aimLoad_MoveTeamSplit(AITeamMove *move);
void aimLoad_Special(AITeamMove *move);
void aimLoad_Attack(AITeamMove *move);
void aimLoad_AdvancedAttack(AITeamMove *move);
void aimLoad_FlankAttack(AITeamMove *move);
void aimLoad_MoveAttack(AITeamMove *move);
void aimLoad_Dock(AITeamMove *move);
void aimLoad_Support(AITeamMove *move);
void aimLoad_ShipRecon(AITeamMove *move);
void aimLoad_ControlResources(AITeamMove *move);
void aimLoad_SwarmAttack(AITeamMove *move);
void aimLoad_SwarmDefense(AITeamMove *move);
void aimLoad_ResourceVolume(AITeamMove *move);
void aimLoad_Kamikaze(AITeamMove *move);

void aimSave_GuardShips(AITeamMove *move);
void aimSave_DefMoship(AITeamMove *move);
void aimSave_PatrolMove(AITeamMove *move);
void aimSave_MoveTeamSplit(AITeamMove *move);
void aimSave_Special(AITeamMove *move);
void aimSave_Attack(AITeamMove *move);
void aimSave_AdvancedAttack(AITeamMove *move);
void aimSave_FlankAttack(AITeamMove *move);
void aimSave_MoveAttack(AITeamMove *move);
void aimSave_Dock(AITeamMove *move);
void aimSave_Support(AITeamMove *move);
void aimSave_ShipRecon(AITeamMove *move);
void aimSave_ControlResources(AITeamMove *move);
void aimSave_SwarmAttack(AITeamMove *move);
void aimSave_SwarmDefense(AITeamMove *move);
void aimSave_ResourceVolume(AITeamMove *move);
void aimSave_Kamikaze(AITeamMove *move);

void aimPreFix_Dock(AITeamMove *move);
void aimPreFix_GetShips(AITeamMove *move);
void aimPreFix_Intercept(AITeamMove *move);
void aimPreFix_PatrolMove(AITeamMove *move);
void aimPreFix_AdvancedAttack(AITeamMove *move);
void aimPreFix_MoveAttack(AITeamMove *move);
void aimPreFix_HarassAttack(AITeamMove *move);
void aimPreFix_FancyGetShips(AITeamMove *move);
void aimPreFix_Reinforce(AITeamMove *move);

#endif
