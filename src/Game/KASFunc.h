#ifndef __KASFUNC_H
#define __KASFUNC_H

#include "Types.h"
#include "AIUtilities.h"
#include "Volume.h"

//
//  --------------------------------
//  these are for the placeholder for a real fleet intelligence message window
//
//
//  PETER:  we can nuke all this now, right?
//

// seconds
#define POPUPTEXT_DURATION_PER_LINE  4

// chars
#define POPUPTEXT_MAX_LINE_LENGTH  128
#define POPUPTEXT_MAX_LINES  16

// pixels
#define POPUPTEXT_BOX_WIDTH  200

void kasfPopupTextDraw(void);
void kasfPopupText(char *line);
void kasfPopupTextClear(void);

//
//  --------------------------------
//


//
//  new function prototypes also need to go into KAS2C.C so the
//  translator will recognize them
//

void kasfMissionCompleted(void);
void kasfMissionFailed(void);

void kasfFadeToWhite(void);

void kasfVarCreate(char *name);
void kasfVarValueSet(char *name, sdword value);
void kasfVarCreateSet(char *name, sdword value);
void kasfVarValueInc(char *name);
void kasfVarValueDec(char *name);
sdword kasfVarValueGet(char *name);
void kasfVarDestroy(char *name);

void kasfCommandMessage(char *message);

void kasfTimerCreate(char *name);
void kasfTimerSet(char *name, sdword duration);
void kasfTimerStart(char *name);
void kasfTimerCreateSetStart(char *name, sdword duration);
void kasfTimerStop(char *name);
void kasfHarvest(void);
sdword kasfTimerRemaining(char *name);
sdword kasfTimerExpired(char *name);
sdword kasfTimerExpiredDestroy(char *name);
void kasfTimerDestroy(char *name);

sdword kasfMsgReceived(char *msg);

void kasfAttackMothership(void);
void kasfAttack(GrowSelection *targets);
void kasfAttackFlank(GrowSelection *targets);
void kasfAttackHarass(void);
void kasfAttackSpecial(GrowSelection *targets);
void kasfMoveAttack(GrowSelection *targets);
sdword kasfBulgeAttack(GrowSelection *targets, GrowSelection *bulgetargets, GrowSelection *attackers, sdword radius);
void kasfIntercept(GrowSelection *targets);
void kasfTargetDrop(void);
void kasfSetSwarmerTargets(GrowSelection *targets);
void kasfSwarmMoveTo(GrowSelection *targets);
void kasfShipsAttack(GrowSelection *targets, GrowSelection *attackers);

void kasfFormationDelta(void);
void kasfFormationBroad(void);
void kasfFormationDelta3D(void);
void kasfFormationClaw(void);
void kasfFormationWall(void);
void kasfFormationSphere(void);
void kasfFormationCustom(GrowSelection *shipsg);

sdword kasfRandom(sdword lowestNum, sdword highestNum);

void kasfGuardMothership(void);
void kasfGuardShips(GrowSelection *ships);

void kasfPatrolPath(Path *path);
void kasfPatrolActive(void);

void kasfLog(char *string);
void kasfLogInteger(char *string, sdword integer);

sdword kasfTeamMemberHandle(sdword Index);
sdword kasfTeamHealthAverage(void);
sdword kasfTeamHealthLowest(void);
sdword kasfTeamFuelAverage(void);
sdword kasfTeamFuelLowest(void);
sdword kasfTeamCount(void);
sdword kasfTeamCountOriginal(void);
sdword kasfNewShipsAdded(void);

void kasfGrowSelectionClear(GrowSelection *ships);

void kasfTeamAttributesBitSet(sdword attributes);
void kasfTeamAttributesBitClear(sdword attributes);
void kasfTeamAttributesSet(sdword attributes);

void kasfShipsAttributesBitSet(GrowSelection *ships,sdword attributes);
void kasfShipsAttributesBitClear(GrowSelection *ships,sdword attributes);
void kasfShipsAttributesSet(GrowSelection *ships,sdword attributes);

sdword kasfShipsDisabled(GrowSelection *ships);

sdword kasfShipsCount(GrowSelection *ships);

sdword kasfPointInside(Volume *volume, hvector *location);

sdword kasfFindShipsInside(Volume *volume, GrowSelection *ships);
sdword kasfFindEnemiesInside(Volume *volume, GrowSelection *ships, sdword neighborRadius);
sdword kasfFindEnemiesNearby(GrowSelection *ships, sdword radius);
sdword kasfFindEnemiesNearTeam(GrowSelection *ships, sdword radius);
sdword kasfFindEnemyShipsOfType(GrowSelection *ships, char *shipType);
sdword kasfFindFriendlyShipsOfType(GrowSelection *ships, char *shipType);
sdword kasfFindEnemyShipsOfClass(GrowSelection *ships, char *shipClass);
sdword kasfFindFriendlyShipsOfClass(GrowSelection *ships, char *shipClass);
sdword kasfFindShipsNearPoint(GrowSelection *ships, hvector *location, sdword radius);

void kasfTeamSkillSet(sdword skillLevel);
sdword kasfTeamSkillGet(void);

void kasfDisablePlayerHyperspace(void);
void kasfHoldHyperspaceWindow(bool hold);
void kasfTeamHyperspaceIn(hvector *destination);
void kasfTeamHyperspaceInNear(hvector *destination, sdword distance);
void kasfTeamHyperspaceOut(void);
void kasfHyperspaceDelay(sdword milliseconds);

void kasfGateDestroy(hvector *gatePoint);
void kasfGateShipsIn(GrowSelection *ships, hvector *gatePoint);
void kasfGateShipsOut(GrowSelection *ships, hvector *gatePoint);
void kasfGateMoveToNearest(void);
sdword kasfGateShipsOutNearest(GrowSelection *ships);

void kasfMissionSkillSet(sdword skillLevel);
sdword kasfMissionSkillGet(void);

void kasfRequestShips(char *shipType, sdword numShips);
void kasfRequestShipsOriginal(sdword percentOriginal);

void kasfReinforce(struct AITeam *team);

void kasfTeamGiveToAI(void);

void kasfDisableAIFeature(sdword feature, sdword type);
void kasfEnableAIFeature(sdword feature, sdword type);
void kasfDisableAllAIFeatures(void);
void kasfEnableAllAIFeatures(void);

//void kasfTeamGiveToPlayer(void);
void kasfTeamSwitchPlayerOwner(void);
void kasfShipsSwitchPlayerOwner(GrowSelection *ships);

void kasfReinforceTeamWithShips(struct AITeam *teamtoreinforce,GrowSelection *shipstoadd);
void kasfTeamMakeCrazy(sdword makecrazy);

void kasfForceCombatStatus(GrowSelection *ships, sdword on);

sdword kasfThisTeamIs(struct AITeam *team);

void kasfMoveTo(hvector *destination);
void kasfShipsMoveTo(GrowSelection *ships, hvector *destination);

void kasfTacticsAggressive(void);
void kasfTacticsNeutral(void);
void kasfTacticsEvasive(void);

sdword kasfNearby(hvector *location, sdword distance);
sdword kasfFindDistance(hvector *location1, hvector *location2);

sdword kasfUnderAttack(GrowSelection *attackers);
sdword kasfUnderAttackElsewhere(struct AITeam *otherTeam, GrowSelection *attackers);

sdword kasfShipsCountType(GrowSelection *ships, char *shipType);

void kasfDock(struct AITeam *withTeam);
void kasfDockSupport();
void kasfDockSupportWith(struct AITeam * withTeam);
void kasfShipsDockSupportWith(GrowSelection *ships, GrowSelection *withShips);
void kasfDockStay(struct AITeam * withTeam);
void kasfShipsDockStay(GrowSelection *ships, GrowSelection *withShips);
void kasfDockStayMothership(void);
void kasfDockInstant(struct AITeam * withTeam);
void kasfLaunch();

sdword kasfTeamDocking(void);
sdword kasfTeamDockedReadyForLaunch(void);
sdword kasfTeamFinishedLaunching(void);

void kasfMsgSend(struct AITeam *team, char *msg);
void kasfMsgSendAll(char *msg);

sdword kasfRUsEnemyCollected(void);

sdword kasfPathNextPoint(void);

sdword kasfNISRunning(void);

void kasfTeamSetMaxVelocity(sdword maxVelocity);
void kasfTeamClearMaxVelocity(void);
void kasfShipsSetMaxVelocity(GrowSelection *ships, sdword maxVelocity);
void kasfShipsClearMaxVelocity(GrowSelection *ships, sdword maxVelocity);
void kasfShipsSetDamageFactor(GrowSelection *ships, sdword damagePercent);
void kasfShipsClearDamageFactor(GrowSelection *ships);
void kasfTeamSetPercentDamaged(sdword percentDamaged);
void kasfTeamSetPercentFueled(sdword percentFueled);

sdword kasfShipsSelectEnemy(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectFriendly(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectClass(GrowSelection *newShips, GrowSelection *originalShips, char *shipClass);
sdword kasfShipsSelectType(GrowSelection *newShips, GrowSelection *originalShips, char *shipType);
sdword kasfShipsSelectDamaged(GrowSelection *newShips, GrowSelection *originalShips, sdword maxHealthPercent);
sdword kasfShipsSelectMoving(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectCapital(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectNonCapital(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectStrikeCraft(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectNotDocked(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectIndex(GrowSelection *newShips, GrowSelection *originalShips, sdword Index);
sdword kasfShipsSelectNearby(GrowSelection *newShips, GrowSelection *originalShips, hvector *location, sdword distance);

sdword kasfShipsOrder(GrowSelection *ships);
sdword kasfShipsOrderAttributes(GrowSelection *ships);

#define kasSpecial_ReturningTechnology  1  //ships are salcapcorvettes returning technology
#define kasSpecial_ShipsDisabled        2  //ships have been disabled (salcapcorvette has locked on)
#define kasSpecial_ShipsAreArmed        3  //ships have arms on them - includes GravWellGenerator as an armed ship
#define kasSpecial_ShipsDefected        4  //ships have defected to the other side
sdword kasfShipsSelectSpecial(GrowSelection *newShips, GrowSelection *originalShips, sdword SpecialFlag);


sdword kasfShipsAdd(GrowSelection *shipsA, GrowSelection *shipsB);
sdword kasfShipsRemove(GrowSelection *shipsA, GrowSelection *shipsB);

void kasfShipsSetNonRetaliation(GrowSelection *ships);
void kasfShipsSetRetaliation(GrowSelection *ships);

void kasfPingAddSingleShip(GrowSelection *ships, char *label);
void kasfPingAddShips(GrowSelection *ships, char *label);
void kasfPingAddPoint(hvector *point, char *label);
void kasfPingRemove(char *label);

void kasfBuildControl(sdword on);
void kasfBuildingTeam(struct AITeam *builder);

void kasfStop(void);

void kasfKamikaze(GrowSelection *targets);
void kasfKamikazeEveryone(GrowSelection *targets);

void kasfPopup(char *text);
void kasfPopupInteger(char *text, sdword integer);

void kasfObjectiveCreate(char *label, char *briefText, char *fullText);
void kasfObjectiveCreateSecondary(char *label, char* briefText, char* fullText);
void kasfObjectiveSet(char *label, sdword status);
//void kasfObjectivePopupAll(void);
sdword kasfObjectiveGet(char *label);
sdword kasfObjectiveGetAll(void);
void kasfObjectiveDestroy(char *label);
void kasfObjectiveDestroyAll(void);

void kasfSpecialToggle(void);

void kasfShipsDamage(GrowSelection *ships, sdword points);

void kasfForceTaskbar(void);

// Tutorial helper functions for kas ------------
void kasfBuilderRestrictShipTypes(char *shipTypes);
void kasfBuilderUnrestrictShipTypes(char *shipTypes);
void kasfBuilderRestrictAll(void);
void kasfBuilderRestrictNone(void);
void kasfBuilderCloseIfOpen(void);
void kasfForceBuildShipType(char *shipType);

sdword kasfCameraGetAngleDeg(void);
sdword kasfCameraGetDeclinationDeg(void);
sdword kasfCameraGetDistance(void);

sdword kasfSelectNumSelected(void);
sdword kasfSelectIsSelectionShipType(sdword Index, char *shipType);
sdword kasfSelectContainsShipTypes(char *shipTypes);
sdword kasfSelectedShipsInFormation(sdword formation);
sdword kasfShipsInFormation(GrowSelection *ships, sdword formation);

void kasfTutSetPointerTargetXY(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetXYRight(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetXYBottomRight(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetXYTaskbar(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetXYFE(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetShip(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetShipSelection(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetShipHealth(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetShipGroup(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetFERegion(char *name, char *pAtomName);
void kasfTutSetPointerTargetRect(char *name, sdword x0, sdword y0, sdword x1, sdword y1);
void kasfTutSetPointerTargetAIVolume(char *name, Volume *volume);
void kasfTutRemovePointer(char *name);
void kasfTutRemoveAllPointers(void);

void kasfTutSetTextDisplayBoxGame(sdword x, sdword y, sdword width, sdword height);
void kasfTutSetTextDisplayBoxFE(sdword x, sdword y, sdword width, sdword height);
void kasfTutSetTextDisplayBoxToSubtitleRegion(void);
void kasfTutShowText(char *szText);
void kasfTutHideText(void);
void kasfTutShowNextButton(void);
void kasfTutHideNextButton(void);
sdword kasfTutNextButtonClicked(void);
void kasfTutShowBackButton(void);
void kasfTutHideBackButton(void);
void kasfTutShowPrevButton(void);
void kasfTutSaveLesson(sdword Num, char *pName);

void kasfTutShowImages(char *szImages);
void kasfTutHideImages(void);

void kasfTutEnableEverything(void);
void kasfTutDisableEverything(void);
void kasfTutEnableFlags(char *pFlags);
void kasfTutDisableFlags(char *pFlags);
void kasfTutForceUnpaused(void);

sdword kasfTutGameSentMessage(char *commandNames);
void kasfTutResetGameMessageQueue(void);
sdword kasfTutContextMenuDisplayedForShipType(char *shipType);
void  kasfTutResetContextMenuShipTypeTest(void);
void kasfTutRedrawEverything(void);

sdword kasfBuildManagerShipTypeInBatchQueue(char *shipType);
sdword kasfBuildManagerShipTypeInBuildQueue(char *shipType);
sdword kasfBuildManagerShipTypeSelected(char *shipType);

sdword kasfTutCameraFocusedOnShipType(char *shipTypes);
void kasfTutCameraFocus(GrowSelection *ships);
void kasfTutCameraFocusDerelictType(char *derelictType);
void kasfTutCameraFocusFar(GrowSelection *ships);
void kasfTutCameraFocusCancel(void);

void kasfDisablePlayer(bool toggle);

sdword kasfTutShipsInView(GrowSelection *ships);
sdword kasfTutShipsTactics(GrowSelection *ships);

sdword kasfTutPieDistance(void);
sdword kasfTutPieHeight(void);

void kasfForceFISensors(void);
void kasfOpenSensors(sdword flag);
void kasfCloseSensors(sdword flag);
sdword kasfSensorsIsOpen(sdword flag);
void kasfSensorsWeirdness(sdword flag);

void kasfAllowPlayerToResearch(char *name);
void kasfAllowPlayerToPurchase(char *name);
void kasfPlayerAcquiredTechnology(char *name);
sdword kasfCanPlayerResearch(char *name);
sdword kasfCanPlayerPurchase(char *name);
sdword kasfDoesPlayerHave(char *name);
void kasfSetBaseTechnologyCost(char *name, sdword cost);
sdword kasfGetBaseTechnologyCost(char *name);
sdword kasfTechIsResearching(void);

void kasfEnableTraderGUI(void);
sdword kasfTraderGUIActive(void);
void kasfSetTraderDialog(sdword dialogNum, char *text);
void kasfSetTraderPriceScale(sdword percent);
sdword kasfGetTraderPriceScale(void);
void kasfSetTraderDisabled(sdword disable);

sdword kasfRUsGet(sdword player);
void kasfRUsSet(sdword player, sdword RUs);

sdword kasfGetWorldResources(void);

void kasfSoundEvent(sdword event);
void kasfSoundEventShips(GrowSelection *ships, sdword event);

void kasfSpeechEvent(sdword event, sdword variable);
void kasfSpeechEventShips(GrowSelection *ships, sdword event, sdword variable);

void kasfToggleActor(sdword Actor, sdword on);

void kasfMusicPlay(sdword trackNum);
void kasfMusicStop(sdword fadeTime);

sdword kasfRenderedShips(GrowSelection *ships, sdword LOD);
void kasfResetShipRenderFlags(GrowSelection *ships);
sdword kasfRenderedDerelicts(char *derelictType, sdword LOD);
void kasfResetDerelictRenderFlags(char *derelicttype);

udword kasfFindSelectedShips(GrowSelection *ships);

void kasfSaveLevel(sdword num, char *name);

void kasfClearScreen(void);

void kasfWideScreenIn(sdword frames);
void kasfWideScreenOut(sdword frames);
void kasfSubtitleSimulate(sdword actor, sdword milliseconds, char *speech);
void kasfLocationCard(sdword milliseconds, char *location);
void kasfHideShips(GrowSelection *ships);
void kasfUnhideShips(GrowSelection *ships);

void kasfDeleteShips(GrowSelection *ships);

void kasfRotateDerelictType(char *derelictType, sdword rot_x, sdword rot_y, sdword rot_z, sdword variation);

sdword kasfRaceOfHuman(void);
void kasfUniversePause(void);
void kasfUniverseUnpause(void);
void kasfPauseBuildShips(void);
void kasfUnpauseBuildShips(void);
void kasfOtherKASPause(void);
void kasfOtherKASUnpause(void);

sdword kasfIntelEventEnded(void);
void kasfIntelEventNotEnded(void);
void kasfForceIntelEventEnded(void);

//turn sensors manager static on/off
void kasfSensorsStaticOn(void);
void kasfSensorsStaticOff(void);

//end the game right now and start plugscreens
//#if defined(GCW) || defined (Downloadable) || defined (OEM)
void kasfGameEnd(void);
//#endif

void kasfSpawnEffect(GrowSelection *ships, char *effectName, sdword parameter);

#endif
