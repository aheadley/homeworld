
/*=============================================================================
    Name    : Strings.h
    Purpose : Header for Strings.c

    Created 5/7/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <string.h>
#include "Types.h"
#include "Memory.h"
#include "StatScript.h"
#include "Strings.h"
#include "Debug.h"

#define strEntry(var)  {#var,strSetStringCB,&MessageStrings[var]}

udword strCurLanguage=0;
udword strCurKeyboardLanguage=0;
bool8  strInitialized=FALSE;
char  *MessageStrings[NumStrings];

char   crapMessage[]="*NO_STRING*";

scriptEntry LanguageStrings[] =
{
    strEntry(strAdvanceSupportFrigate),
    strEntry(strAttackBomber),
    strEntry(strCarrier),
    strEntry(strCloakedFighter),
    strEntry(strCloakGenerator),
    strEntry(strDDDFrigate),
    strEntry(strDefenseFighter),
    strEntry(strDFGFrigate),
    strEntry(strGravWellGenerator),
    strEntry(strHeavyCorvette),
    strEntry(strHeavyCruiser),
    strEntry(strHeavyDefender),
    strEntry(strHeavyInterceptor),
    strEntry(strIonCannonFrigate),
    strEntry(strLightCorvette),
    strEntry(strLightDefender),
    strEntry(strLightInterceptor),
    strEntry(strMinelayerCorvette),
    strEntry(strMissileDestroyer),
    strEntry(strMothership),
    strEntry(strMultiGunCorvette),
    strEntry(strProbe),
    strEntry(strProximitySensor),
    strEntry(strRepairCorvette),
    strEntry(strResearchShip),
    strEntry(strResourceCollector),
    strEntry(strResourceController),
    strEntry(strSalCapCorvette),
    strEntry(strSensorArray),
    strEntry(strStandardDestroyer),
    strEntry(strStandardFrigate),
    strEntry(strDrone),
    strEntry(strTargetDrone),
    strEntry(strHeadShotAsteroid),
    strEntry(strCryoTray),
    strEntry(strP1Fighter),
    strEntry(strP1IonArrayFrigate),
    strEntry(strP1MissileCorvette),
    strEntry(strP1Mothership),
    strEntry(strP1StandardCorvette),
    strEntry(strP2AdvanceSwarmer),
    strEntry(strP2FuelPod),
    strEntry(strP2Mothership),
    strEntry(strP2MultiBeamFrigate),
    strEntry(strP2Swarmer),
    strEntry(strP3Destroyer),
    strEntry(strP3Frigate),
    strEntry(strP3Megaship),
    strEntry(strFloatingCity),
    strEntry(strCargoBarge),
    strEntry(strMiningBase),
    strEntry(strResearchStation),
    strEntry(strJunkYardDawg),
    strEntry(strJunkYardHQ),
    strEntry(strGhostship),
    strEntry(strJunk_LGun),
    strEntry(strJunk_SGun),
    strEntry(strResearchStationBridge),
    strEntry(strResearchStationTower),

    strEntry(strCLASS_Mothership),
    strEntry(strCLASS_HeavyCruiser),
    strEntry(strCLASS_Carrier),
    strEntry(strCLASS_Destroyer),
    strEntry(strCLASS_Frigate),
    strEntry(strCLASS_Corvette),
    strEntry(strCLASS_Fighter),
    strEntry(strCLASS_Resource),
    strEntry(strCLASS_NonCombat),
    strEntry(strCLASS_Class),
    strEntry(strCLASS_SuperCapital),

    strEntry(strPlayerUnits),
    strEntry(strEnemyUnits),
    strEntry(strAlliedUnits),
    strEntry(strResources),
    strEntry(strDerelictShip),
    strEntry(strUnexplored),
    strEntry(strAsteroids),
    strEntry(strNebula),
    strEntry(strGasClouds),
    strEntry(strDustClouds),

    strEntry(strLoadingShips),
    strEntry(strTexturePreload),
    strEntry(strLoadingTextures),

    strEntry(strEnemyShip),
    strEntry(strAlliedShip),
    strEntry(strEnemy),
    strEntry(strAllied),

    strEntry(strHaveBeenDefeated),
    strEntry(strQuestIsLost),
    strEntry(strAllPlayersDead),
    strEntry(strPlayerHasWon),
    strEntry(strPlayerHasDied),
    strEntry(strPlayersHaveWon),
    strEntry(strHaveBeenDroppedOut),
    strEntry(strAllOtherPlayersDroppedOut),
    strEntry(strSyncPktQOverrun),

    strEntry(strNewAlloys),
    strEntry(strMassDrive1Kt),
    strEntry(strCoolingSystems),
    strEntry(strCloakFighter),
    strEntry(strDefenseFighterTech),
    strEntry(strTargetingSystems),
    strEntry(strPlasmaWeapons),
    strEntry(strChassis1),
    strEntry(strMassDrive10Kt),
    strEntry(strMediumGuns),
    strEntry(strMineLayerTech),
    strEntry(strChassis2),
    strEntry(strAdvancedCoolingSystems),
    strEntry(strMassDrive100Kt),
    strEntry(strFireControl),
    strEntry(strSupportRefuelTech),
    strEntry(strAdvanceTacticalSupport),
    strEntry(strIonWeapons),
    strEntry(strDDDFTech),
    strEntry(strDFGFTech),
    strEntry(strChassis3),
    strEntry(strMassDrive1Mt),
    strEntry(strAdvancedFireControl),
    strEntry(strMissileWeapons),
    strEntry(strConstructionTech),
    strEntry(strHeavyGuns),
    strEntry(strProximityDetector),
    strEntry(strSensorsArrayTech),
    strEntry(strGravityWellGeneratorTech),
    strEntry(strCloakGeneratorTech),
    strEntry(strRepairTech),
    strEntry(strSalvageTech),

    strEntry(strWeaponHeading),
    strEntry(strConstuctionHeading),
    strEntry(strInterdictionHeading),
    strEntry(strSupportHeading),

    strEntry(strR1NewAlloysinfo),
    strEntry(strR1MassDrive1Ktinfo),
    strEntry(strR1CoolingSystemsinfo),
    strEntry(strR1CloakFighterinfo),
    strEntry(strR1TargetingSystemsinfo),
    strEntry(strR1PlasmaWeaponsinfo),
    strEntry(strR1Chassis1info),
    strEntry(strR1MassDrive10Ktinfo),
    strEntry(strR1MediumGunsinfo),
    strEntry(strR1MineLayerTechinfo),
    strEntry(strR1Chassis2info),
    strEntry(strR1AdvancedCoolingSystemsinfo),
    strEntry(strR1MassDrive100Ktinfo),
    strEntry(strR1FireControlinfo),
    strEntry(strR1SupportRefuelTechinfo),
    strEntry(strR1AdvanceTacticalSupportinfo),
    strEntry(strR1IonWeaponsinfo),
    strEntry(strR1DDDFTechinfo),
    strEntry(strR1Chassis3info),
    strEntry(strR1MassDrive1Mtinfo),
    strEntry(strR1AdvancedFireControlinfo),
    strEntry(strR1MissileWeaponsinfo),
    strEntry(strR1ConstructionTechinfo),
    strEntry(strR1HeavyGunsinfo),
    strEntry(strR1ProximityDetectorinfo),
    strEntry(strR1SensorsArrayTechinfo),
    strEntry(strR1GravityWellGeneratorTechinfo),
    strEntry(strR1CloakGeneratorTechinfo),
    strEntry(strR1RepairTechinfo),
    strEntry(strR1SalvageTechinfo),
    strEntry(strR1DFGFTechinfo),
    strEntry(strR1DefenseFighterTechinfo),

    strEntry(strR2NewAlloysinfo),
    strEntry(strR2MassDrive1Ktinfo),
    strEntry(strR2CoolingSystemsinfo),
    strEntry(strR2CloakFighterinfo),
    strEntry(strR2TargetingSystemsinfo),
    strEntry(strR2PlasmaWeaponsinfo),
    strEntry(strR2Chassis1info),
    strEntry(strR2MassDrive10Ktinfo),
    strEntry(strR2MediumGunsinfo),
    strEntry(strR2MineLayerTechinfo),
    strEntry(strR2Chassis2info),
    strEntry(strR2AdvancedCoolingSystemsinfo),
    strEntry(strR2MassDrive100Ktinfo),
    strEntry(strR2FireControlinfo),
    strEntry(strR2SupportRefuelTechinfo),
    strEntry(strR2AdvanceTacticalSupportinfo),
    strEntry(strR2IonWeaponsinfo),
    strEntry(strR2DDDFTechinfo),
    strEntry(strR2Chassis3info),
    strEntry(strR2MassDrive1Mtinfo),
    strEntry(strR2AdvancedFireControlinfo),
    strEntry(strR2MissileWeaponsinfo),
    strEntry(strR2ConstructionTechinfo),
    strEntry(strR2HeavyGunsinfo),
    strEntry(strR2ProximityDetectorinfo),
    strEntry(strR2SensorsArrayTechinfo),
    strEntry(strR2GravityWellGeneratorTechinfo),
    strEntry(strR2CloakGeneratorTechinfo),
    strEntry(strR2RepairTechinfo),
    strEntry(strR2SalvageTechinfo),
    strEntry(strR2DFGFTechinfo),
    strEntry(strR2DefenseFighterTechinfo),

    strEntry(strAbrevAdvanceSupportFrigate),
    strEntry(strAbrevAttackBomber),
    strEntry(strAbrevCarrier),
    strEntry(strAbrevCloakedFighter),
    strEntry(strAbrevCloakGenerator),
    strEntry(strAbrevDDDFrigate),
    strEntry(strAbrevDefenseFighter),
    strEntry(strAbrevDFGFrigate),
    strEntry(strAbrevGravWellGenerator),
    strEntry(strAbrevHeavyCorvette),
    strEntry(strAbrevHeavyCruiser),
    strEntry(strAbrevHeavyDefender),
    strEntry(strAbrevHeavyInterceptor),
    strEntry(strAbrevIonCannonFrigate),
    strEntry(strAbrevLightCorvette),
    strEntry(strAbrevLightDefender),
    strEntry(strAbrevLightInterceptor),
    strEntry(strAbrevMinelayerCorvette),
    strEntry(strAbrevMissileDestroyer),
    strEntry(strAbrevMothership),
    strEntry(strAbrevMultiGunCorvette),
    strEntry(strAbrevProbe),
    strEntry(strAbrevProximitySensor),
    strEntry(strAbrevRepairCorvette),
    strEntry(strAbrevResearchShip),
    strEntry(strAbrevResourceCollector),
    strEntry(strAbrevResourceController),
    strEntry(strAbrevSalCapCorvette),
    strEntry(strAbrevSensorArray),
    strEntry(strAbrevStandardDestroyer),
    strEntry(strAbrevStandardFrigate),
    strEntry(strAbrevDrone),
    strEntry(strAbrevTargetDrone),
    strEntry(strAbrevHeadShotAsteroid),
    strEntry(strAbrevCryoTray),
    strEntry(strAbrevP1Fighter),
    strEntry(strAbrevP1IonArrayFrigate),
    strEntry(strAbrevP1MissileCorvette),
    strEntry(strAbrevP1Mothership),
    strEntry(strAbrevP1StandardCorvette),
    strEntry(strAbrevP2AdvanceSwarmer),
    strEntry(strAbrevP2FuelPod),
    strEntry(strAbrevP2Mothership),
    strEntry(strAbrevP2MultiBeamFrigate),
    strEntry(strAbrevP2Swarmer),
    strEntry(strAbrevP3Destroyer),
    strEntry(strAbrevP3Frigate),
    strEntry(strAbrevP3Megaship),
    strEntry(strAbrevFloatingCity),
    strEntry(strAbrevCargoBarge),
    strEntry(strAbrevMiningBase),
    strEntry(strAbrevResearchStation),
    strEntry(strAbrevJunkYardDawg),
    strEntry(strAbrevJunkYardHQ),
    strEntry(strAbrevGhostship),
    strEntry(strAbrevJunk_LGun),
    strEntry(strAbrevJunk_SGun),
    strEntry(strAbrevResearchStationBridge),
    strEntry(strAbrevResearchStationTower),

    strEntry(strCanBuild),

    strEntry(strDELTA_FORMATION),
    strEntry(strBROAD_FORMATION),
    strEntry(strDELTA3D_FORMATION),
    strEntry(strCLAW_FORMATION),
    strEntry(strWALL_FORMATION),
    strEntry(strSPHERE_FORMATION),
    strEntry(strPICKET_FORMATION),

    strEntry(strPARADE_FORMATION),

    strEntry(strCreatingRoom),
    strEntry(strRoomCreated),
    strEntry(strQueryingChat),
    strEntry(strConnectingToChat),
    strEntry(strConnectedToChat),
    strEntry(strStartingChat),
    strEntry(strCreatedChat),
    strEntry(strSendingLogin),
    strEntry(strChangingPassword),
    strEntry(strCreatingUser),
    strEntry(strRequestingToJoin),
    strEntry(strConnectedWon),
    strEntry(strLoginFailed),
    strEntry(strHitCancelAgain),
    strEntry(strPasswordChangeSuccess),
    strEntry(strPasswordChangeFailed),
    strEntry(strNewUserCreated),
    strEntry(strDuplicateUser),
    strEntry(strUserNotFound),
    strEntry(strInvalidName),
    strEntry(strUnknownAuthReply),
    strEntry(strFailedToCreateUser),
    strEntry(strFailedToChat),
    strEntry(strHitCancelContinue),
    strEntry(strJoinRequestGranted),
    strEntry(strJoinRequestDenied),
    strEntry(strWonOffline),
    strEntry(strServerDown),

    strEntry(strKeyAlreadyInUse),
    strEntry(strBadKey),
    strEntry(strLightweightBadKey),
    strEntry(strBadKeyExpired),
    strEntry(strBadKeyLockedOut),
    strEntry(strBadKeyBetaKeyRequired),
    strEntry(strBadKeyBetaKeyNotAllowed),
    strEntry(strNoCDKey),

    strEntry(strFailedToCreateChat),
    strEntry(strFailedToConnectToDirServer),

    strEntry(strCantJoinOwnGame),
    strEntry(strErrorCantFindGame),
    strEntry(strIncorrectPassword),
    strEntry(strCaptainDisolvedGame),
    strEntry(strNoInternetTCPIP),
    strEntry(strNoLanIPXorTCPIP),
    strEntry(strProtocalIPXLAN),
    strEntry(strProtocalTCPIPLAN),
    strEntry(strAuthCRCFailed),
    strEntry(strErrorStillWaitingVersionInfo),
    strEntry(strMinNumPlayersRequired),
    strEntry(strMustBeInRoomToCreateGame),
    strEntry(strMustBeInRoomToCreateGame2),

    strEntry(strCreatingGame),
    strEntry(strInvalidRoomPassword),
    strEntry(strErrorUserAlreadyExists),
    strEntry(strErrorRoomFull),
    strEntry(strNeedLatestVersion),
    strEntry(strDownloadingPatch),
    strEntry(strErrorGameAlreadyExists),
    strEntry(strCreatedGame),
    strEntry(strDroppedOut),
    strEntry(strQuit),
    strEntry(strErrorTypingPassword),
    strEntry(strPasswordProtected),
    strEntry(strInProgress),
    strEntry(strDiffVersion),
    strEntry(strErrorRoomAlreadyExists),
    strEntry(strNoRoom),
    strEntry(strDifferentVersions),
    strEntry(strMustUpgradeToSameVersion),
    strEntry(strChatDisconnected),

    strEntry(strYouMustTypeInGameName),
    strEntry(strAtLeast2Chars),
    strEntry(strMustBeAtLeast2Chars),
    strEntry(strErrorInSaveGameName),
    strEntry(strErrorInvalidSaveGameFile),
    strEntry(strErrorInvalidSaveGameFileVersion),

    strEntry(strStartingGame),
    strEntry(strDetectedUserBehindFirewall),
    strEntry(strStartingRoutingServer),
    strEntry(strTellingPlayersToConnectToMe),
    strEntry(strConnectingToRoutingServer),
    strEntry(strPlayerJoined),
    strEntry(strConnectingToCaptain),
    strEntry(strTellingPlayersConnectRoutServ),

    strEntry(strPatchStartingDownload),
    strEntry(strPatchDownloading),
    strEntry(strPatchGeneralError),
    strEntry(strPatchUnableCreateFile),
    strEntry(strPatchUnableWriteFile),
    strEntry(strPatchUserAbort),
    strEntry(strPATCHFAIL_UNABLE_TO_CONNECT),
    strEntry(strPATCHFAIL_ERROR_SENDING_REQUEST),
    strEntry(strPATCHFAIL_ERROR_RECEIVING_HTTP_HEADER),
    strEntry(strPATCHFAIL_INVALID_FILE_LENGTH),
    strEntry(strPATCHFAIL_ERROR_RECEIVING_PATCH),
    strEntry(strPATCHFAIL_UNABLE_TO_START_DOWNLOAD_THREAD),
    strEntry(strPATCHFAIL_INVALID_STATUS_REPLY),
    strEntry(strTryLaterDownloadManually),

    strEntry(strWhisperedMessage),
    strEntry(strHasJoined),
    strEntry(strHasLeft),

    strEntry(strChannelNameHeading),
    strEntry(strChannelDescHeading),
    strEntry(strChannelNumberHeading),

    strEntry(strGameNameHeading),
    strEntry(strGamePingHeading),
    strEntry(strGameNumPlayerHeading),
    strEntry(strGameMapHeading),

    strEntry(strCustomMapAutoupload),
    strEntry(strCustomMapAutodownload),

    strEntry(strVersion),

    strEntry(strAsksToFormAlliance),
    strEntry(strWants),
    strEntry(strToJoin),
    strEntry(strAnd),
    strEntry(strAllianceFormed),
    strEntry(strHasBrokenAlliance),
    strEntry(strAllianceBroken),
    strEntry(strAllianceRequest),
    strEntry(strAllianceConfirm),
    strEntry(strAskForPermision),

    strEntry(strComputerName),

    strEntry(strFirepower),
    strEntry(strCoverage),
    strEntry(strManeuver),
    strEntry(strArmor),
    strEntry(strTopSpeed),
    strEntry(strMass),

    strEntry(strVeryLow),
    strEntry(strLow),
    strEntry(strMedium),
    strEntry(strHigh),
    strEntry(strVeryHigh),
    strEntry(strCoverageUnits),

    strEntry(strFightersCorvettesDocked),
    strEntry(strSay),
    strEntry(strToAllies),
    strEntry(strRUAmount),
    strEntry(strTutorialTip),

    strEntry(strPlayersAllied),
    strEntry(strResourcesRes),
    strEntry(strGas),
    strEntry(strDust),
    strEntry(strRock),
    strEntry(strNonCombatShips),
    strEntry(strStrikeCraft),
    strEntry(strCapitalShips),
    strEntry(strPingTO0),
    strEntry(strPingTO1),
    strEntry(strPingTO2),
    strEntry(strPingTO3),
    strEntry(strPingTO4),
    strEntry(strPingTORC),

    strEntry(strSaveSinglePlayerLevel1),
    strEntry(strSavedGame),
    strEntry(strRecordingGame),
    strEntry(strQuickSave),

    strEntry(strCaptainTransfering),

    strEntry(strEndOfBuffer),
    strEntry(strStartOfBuffer),

    strEntry(strObjComplete),
    strEntry(strObjIncomplete),
    strEntry(strobjSecondary),

    strEntry(strkbNEXT_FORMATION),
    strEntry(strkbBUILD_MANAGER),
    strEntry(strkbPREVIOUS_FOCUS),
    strEntry(strkbNEXT_FOCUS),
    strEntry(strkbDOCK),
    strEntry(strkbSELECT_ALL_VISIBLE),
    strEntry(strkbFOCUS),
    strEntry(strkbRESEARCH_MANAGER),
    strEntry(strkbHARVEST),
    strEntry(strkbMOVE),
    strEntry(strkbNEXT_TACTIC),
    strEntry(strkbPREVIOUS_TACTIC),
    strEntry(strkbSCUTTLE),
    strEntry(strkbSHIP_SPECIAL),
    strEntry(strkbTACTICAL_OVERLAY),
    strEntry(strkbMOTHERSHIP),
    strEntry(strkbKAMIKAZE),
    strEntry(strkbCANCEL_ORDERS),
    strEntry(strkbLAUNCH_MANAGER),
    strEntry(strkbTOTAL_COMMANDS),

    strEntry(strAKEY),
    strEntry(strBKEY),
    strEntry(strCKEY),
    strEntry(strDKEY),
    strEntry(strEKEY),
    strEntry(strFKEY),
    strEntry(strGKEY),
    strEntry(strHKEY),
    strEntry(strIKEY),
    strEntry(strJKEY),
    strEntry(strKKEY),
    strEntry(strLKEY),
    strEntry(strMKEY),
    strEntry(strNKEY),
    strEntry(strOKEY),
    strEntry(strPKEY),
    strEntry(strQKEY),
    strEntry(strRKEY),
    strEntry(strSKEY),
    strEntry(strTKEY),
    strEntry(strUKEY),
    strEntry(strVKEY),
    strEntry(strWKEY),
    strEntry(strXKEY),
    strEntry(strYKEY),
    strEntry(strZKEY),
    strEntry(strBACKSPACEKEY),
    strEntry(strTABKEY),
    strEntry(strARRLEFT),
    strEntry(strARRRIGHT),
    strEntry(strARRUP),
    strEntry(strARRDOWN),
    strEntry(strENDKEY),
    strEntry(strLBRACK),
    strEntry(strRBRACK),
    strEntry(strCAPSLOCKKEY),
    strEntry(strSPACEKEY),
    strEntry(strENTERKEY),
    strEntry(strHOMEKEY),
    strEntry(strPAGEDOWNKEY),
    strEntry(strPAGEUPKEY),
    strEntry(strBACKSLASHKEY),
    strEntry(strPAUSEKEY),
    strEntry(strSCROLLKEY),
    strEntry(strPRINTKEY),
    strEntry(strINSERTKEY),
    strEntry(strDELETEKEY),
    strEntry(strLESSTHAN),
    strEntry(strGREATERTHAN),
    strEntry(strTILDEKEY),
    strEntry(strNUMPAD0),
    strEntry(strNUMPAD1),
    strEntry(strNUMPAD2),
    strEntry(strNUMPAD3),
    strEntry(strNUMPAD4),
    strEntry(strNUMPAD5),
    strEntry(strNUMPAD6),
    strEntry(strNUMPAD7),
    strEntry(strNUMPAD8),
    strEntry(strNUMPAD9),
    strEntry(strNUMMINUSKEY),
    strEntry(strNUMPLUSKEY),
    strEntry(strNUMSTARKEY),
    strEntry(strNUMSLASHKEY),
    strEntry(strNUMDOTKEY),
    strEntry(strMINUSKEY),
    strEntry(strPLUSKEY),
    strEntry(strF1KEY),
    strEntry(strF2KEY),
    strEntry(strF3KEY),
    strEntry(strF4KEY),
    strEntry(strF5KEY),
    strEntry(strF6KEY),
    strEntry(strF7KEY),
    strEntry(strF8KEY),
    strEntry(strF9KEY),
    strEntry(strF10KEY),
    strEntry(strF11KEY),
    strEntry(strF12KEY),

    strEntry(strCommandToBind),
    strEntry(strKeyBound),
    strEntry(strNoKeyBound),

    strEntry(strMissingCD),
    strEntry(strInvalidCD),

    strEntry(strGDIGeneric0),
    strEntry(strGDIGeneric1),
    strEntry(strGDIGenericRenderer),

    strEntry(strOPCountdown0),
    strEntry(strOPCountdown1),
    strEntry(strOPCountdown2),
    strEntry(strOPFailed0),
    strEntry(strOPFailed1),

    strEntry(strDefaultPlayerName),

    strEntry(strStatsDiedStats),
    strEntry(strStatsGameEndStats),
    strEntry(strStatsPlayerName),
    strEntry(strStatsPlayerRace),
    strEntry(strStatsTimeOfDeath),
    strEntry(strStatsDeathByDropout),
    strEntry(strStatsDeathByShipCapture),
    strEntry(strStatsDeathByLossOfMission),
    strEntry(strStatsDeathStillAlive),
    strEntry(strStatsRUsStart),
    strEntry(strStatsRUsCollected),
    strEntry(strStatsRUsSpent),
    strEntry(strStatsRUsGiven),
    strEntry(strStatsRUsReceived),
    strEntry(strStatsRUsInjected),
    strEntry(strStatsRUsByBounties),
    strEntry(strStatsRUsGenerated),
    strEntry(strStatsCurrentRUs),
    strEntry(strStatsTotalDmgAgainstPlayer),
    strEntry(strStatsTotalAcquiredShips),
    strEntry(strStatsTotalAcqShipsByType),
    strEntry(strStatsTotalAcqShipsByClass),
    strEntry(strStatsTotalKills),
    strEntry(strStatsKilled),
    strEntry(strStatsTotalLosses),
    strEntry(strStatsLost),

    strEntry(strSVCost),
    strEntry(strSVRUs),

    strEntry(strFailedToStartRoutingServer),
    strEntry(strServerLoadHeading),
    strEntry(strServerReliabilityHeading),

    strEntry(strStatsBadResUnitsBounds),
    strEntry(strStatsBadResUnitTotals),
    strEntry(strStatsBadShipBounds),
    strEntry(strStatsBadShipCostTotals),
    strEntry(strStatsBadShipsAcquiredTotals),
    strEntry(strStatsBadShipsKilledTotals),
    strEntry(strStatsBadShipsLostTotals),
    strEntry(strStatsBadShipsAcquiredClassTotals),
    strEntry(strStatsBadShipsKilledClassTotals),
    strEntry(strStatsBadShipsLostClassTotals),

    strEntry(strCommandIgnoreOn),
    strEntry(strCommandIgnoreOff),
    strEntry(strCommandKickCaptain),
    strEntry(strCommandKickPlayer),
    strEntry(strCommandBanOn),
    strEntry(strCommandBanOff),
    strEntry(strCommandLimit),
    strEntry(strCommandLimitOff),

    strEntry(strCheatDetect),

    endEntry
};


static char * strParseString(char *str);


/*=============================================================================
    Public Functions for string access:
=============================================================================*/

void strSetCurKeyboard(void)
{
#ifdef _WIN32
    udword keyboard;

    if (keyboard = GetKeyboardLayout(0))
    {
        if (PRIMARYLANGID(keyboard)==LANG_ENGLISH)
        {
            strCurKeyboardLanguage = languageEnglish;
        }
        else if (PRIMARYLANGID(keyboard)==LANG_FRENCH)
        {
            strCurKeyboardLanguage = languageFrench;
        }
        else if (PRIMARYLANGID(keyboard)==LANG_GERMAN)
        {
            strCurKeyboardLanguage = languageGerman;
        }
        else if (PRIMARYLANGID(keyboard)==LANG_SPANISH)
        {
            strCurKeyboardLanguage = languageSpanish;
        }
        else if (PRIMARYLANGID(keyboard)==LANG_ITALIAN)
        {
            strCurKeyboardLanguage = languageItalian;
        }
    }
    else
#endif
        strCurKeyboardLanguage = languageEnglish;
}


/*-----------------------------------------------------------------------------
    Name        : strLoadLanguage
    Description : Load a particular language script and activate it
    Inputs      : language to load
    Outputs     : whether the language was loaded or not
    Return      : bool8
----------------------------------------------------------------------------*/
bool8 strLoadLanguage(strLanguageType language)
{
    udword i;
#ifdef _WIN32
    udword keyboard;
#endif

    for (i=0;i<NumStrings;i++)
    {
        MessageStrings[i]=crapMessage;
    }

#ifdef _WIN32
    if (keyboard = GetKeyboardLayout(0))
    {
        if (PRIMARYLANGID(keyboard)==LANG_ENGLISH)
        {
            strCurKeyboardLanguage = languageEnglish;
        }
        else if (PRIMARYLANGID(keyboard)==LANG_FRENCH)
        {
            strCurKeyboardLanguage = languageFrench;
        }
        else if (PRIMARYLANGID(keyboard)==LANG_GERMAN)
        {
            strCurKeyboardLanguage = languageGerman;
        }
        else if (PRIMARYLANGID(keyboard)==LANG_SPANISH)
        {
            strCurKeyboardLanguage = languageSpanish;
        }
        else if (PRIMARYLANGID(keyboard)==LANG_ITALIAN)
        {
            strCurKeyboardLanguage = languageItalian;
        }
    }
    else
#endif
        strCurKeyboardLanguage = languageEnglish;

    if (strInitialized==TRUE)
    {
        strFreeLanguage();
    }
    switch (language)
    {
        case languageEnglish :
            scriptSet("","English.script",LanguageStrings);
            strCurLanguage = (udword)language;
            strInitialized = TRUE;
        return(TRUE);
        case languageFrench :
            scriptSet("","French.script",LanguageStrings);
            strCurLanguage = (udword)language;
            strInitialized = TRUE;
        return(TRUE);
        case languageGerman :
            scriptSet("","German.script",LanguageStrings);
            strCurLanguage = (udword)language;
            strInitialized = TRUE;
        return(TRUE);
        case languageSpanish :
            scriptSet("","Spanish.script",LanguageStrings);
            strCurLanguage = (udword)language;
            strInitialized = TRUE;
        return(TRUE);
        case languageItalian :
            scriptSet("","Italian.script",LanguageStrings);
            strCurLanguage = (udword)language;
            strInitialized = TRUE;
        return(TRUE);
    }

    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : strFreeLanguage
    Description : Free's all global data from memory
    Inputs      : none
    Outputs     : if success
    Return      : boolean
----------------------------------------------------------------------------*/
bool8 strFreeLanguage(void)
{
    uword i;

    for (i=0;i<NumStrings;i++)
    {
        if ((MessageStrings[i]!=NULL)&&(MessageStrings[i] != crapMessage)) memFree(MessageStrings[i]);
    }

    return(TRUE);
}



/*-----------------------------------------------------------------------------
    Name        : strSetStringCB
    Description : Set String call back for setscript
    Inputs      : directory, field, where to stick it
    Outputs     : none
    Return      : none
----------------------------------------------------------------------------*/
void strSetStringCB(char *directory,char *field,void *dataToFillIn)
{
    *(void**)dataToFillIn = (void *)memStringDupe(field);
    *(void**)dataToFillIn = strParseString(*(void**)dataToFillIn);
}


/*-----------------------------------------------------------------------------
    Name        : strParseString
    Description : Expands special carachters into linefeed etc.
    Inputs      : string to process
    Outputs     : processed string
    Return      : void
----------------------------------------------------------------------------*/
char * strParseString(char *str)
{
    uword i,j, numfound=0;

    for (i=0;i<strlen(str);i++)
    {
        if (str[i]=='\\')
        {
            for (j=(uword)(i+1);j<strlen(str);j++)
            {
                str[j-1]=str[j];
            }
            if (str[i]=='n')
            {
                str[i]='\n';
                numfound++;
            }
        }
    }
    str[i-numfound] = 0;
    return(str);
}

/*-----------------------------------------------------------------------------
    Name        : strGamesMessages
    Description : Convert a string to a string enumeration.
    Inputs      : String - string to convert
    Outputs     :
    Return      : enumeration of string or NumStrings if not found.
----------------------------------------------------------------------------*/
strGamesMessages strNameToEnum(char *string)
{
    sdword index;

    for (index = 0; index < NumStrings; index++)
    {                                                       //search through all language strings
        if (strcmp(string, LanguageStrings[index].name) == 0)
        {                                                   //see if this one matches.
            return(index);
        }
    }
    return(index);
}

