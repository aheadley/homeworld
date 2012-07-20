/*=============================================================================
    Name    : Universe.c
    Purpose : The universe represents the entire game world, including any
              mission spheres contained within.

    Created 6/19/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "Types.h"
#include "Debug.h"
#include "Task.h"
#include "File.h"
#include "LinkedList.h"
#include "SpaceObj.h"
#include "Collision.h"
#include "UnivUpdate.h"
#include "Stats.h"
#include "Camera.h"
#include "CameraCommand.h"
#include "render.h"
#include "StatScript.h"
#include "Ships.h"
#include "Formation.h"
#include "CommandLayer.h"
#include "MEX.h"
#include "Tweak.h"
#include "Globals.h"
#include "CommandNetwork.h"
#include "utility.h"
#include "LevelLoad.h"
#include "AIShip.h"
#include "Gun.h"
#include "ETG.h"
#include "Dock.h"
#include "Universe.h"
#include "AIPlayer.h"
#include "ConsMgr.h"
#include "Strings.h"
#include "SoundEvent.h"
#include "HorseRace.h"
#include "MeshAnim.h"
#include "MultiplayerGame.h"
#include "GameChat.h"
#include "ResearchAPI.h"
#include "SinglePlayer.h"
#include "NavLights.h"
#include "Teams.h"
#include "Captaincy.h"
#include "Bounties.h"
#include "Tutor.h"
#include "Ping.h"
#include "SaveGame.h"
#include "AutoDownloadMap.h"
#include "LagPrint.h"
#include "ScenPick.h"
#include "FastMath.h"
#include "NetCheck.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

extern bool cheapShips;

extern real32 GLOBAL_SHIP_HEALTH_MODIFIER;
extern real32 GLOBAL_SHIP_SPEED_MODIFIER;

/*=============================================================================
    Public Data:
=============================================================================*/

Universe universe;
AsteroidStaticInfo asteroidStaticInfos[NUM_ASTEROIDTYPES];
DustCloudStaticInfo dustcloudStaticInfos[NUM_DUSTCLOUDTYPES];
GasCloudStaticInfo gascloudStaticInfos[NUM_GASCLOUDTYPES];
NebulaStaticInfo nebulaStaticInfos[NUM_NEBULATYPES];

DerelictStaticInfo derelictStaticInfos[NUM_DERELICTTYPES];

MissileStaticInfo missileStaticInfos[NUM_RACES];
MissileStaticInfo mineStaticInfos[2];               //only race 1 and race 2 have mines

GrowSelection ClampedShipList;       // special GrowSelect which can have NULL's in it

struct SphereStaticInfo *sphereStaticInfo = NULL;

Camera defaultCamera;

real32 maxMineCollSphereSize;
//real32 maxShipCollSphereSize;
//real32 maxResourceCollSphereSize;
//real32 maxDerelictCollSphereSize;
//real32 maxCollSphereSize;

meshdata *defaultmesh = NULL;

#if UNIVERSE_TURBOPAUSE_DEBUG
#define UNIVERSE_TURBO_REPEAT       19
bool universeTurbo = FALSE;
#endif

bool universePause = FALSE;

sdword cdMaxShipsAllowed;            // max number of ships allowed
sdword cdLimitCaps[TOTAL_NUM_SHIPS]; // max number of ships per player allowed
sdword cdClassCaps[NUM_CLASSES];     // max number of ships per class allowed
bool   cdEnabled=TRUE;               // flag specifying whether unit caps enabled

uword RacesAllowedForGivenShip[TOTAL_NUM_SHIPS] =
{
    R1_VALID|R2_VALID,                   //AdvanceSupportFrigate
    R1_VALID|R2_VALID,                   //AttackBomber
    R1_VALID|R2_VALID,                   //Carrier
    R1_VALID         ,                   //CloakedFighter
    R1_VALID|R2_VALID,                   //CloakGenerator
    R1_VALID         ,                   //DDDFrigate
             R2_VALID,                   //DefenseFighter
             R2_VALID,                   //DFGFrigate
    R1_VALID|R2_VALID,                   //GravWellGenerator
    R1_VALID|R2_VALID,                   //HeavyCorvette
    R1_VALID|R2_VALID,                   //HeavyCruiser
    R1_VALID|R2_VALID,                   //HeavyDefender
    R1_VALID|R2_VALID,                   //HeavyInterceptor
    R1_VALID|R2_VALID,                   //IonCannonFrigate
    R1_VALID|R2_VALID,                   //LightCorvette
    R1_VALID|R2_VALID,                   //LightDefender
    R1_VALID|R2_VALID,                   //LightInterceptor
    R1_VALID|R2_VALID,                   //MinelayerCorvette
    R1_VALID|R2_VALID,                   //MissileDestroyer
    R1_VALID|R2_VALID,                   //Mothership
    R1_VALID|R2_VALID,                   //MultiGunCorvette
    R1_VALID|R2_VALID,                   //Probe
    R1_VALID|R2_VALID,                   //ProximitySensor
    R1_VALID|R2_VALID,                   //RepairCorvette
    R1_VALID|R2_VALID,                   //ResearchShip
    R1_VALID|R2_VALID,                   //ResourceCollector
    R1_VALID|R2_VALID,                   //ResourceController
    R1_VALID|R2_VALID,                   //SalCapCorvette
    R1_VALID|R2_VALID,                   //SensorArray
    R1_VALID|R2_VALID,                   //StandardDestroyer
    R1_VALID|R2_VALID,                   //StandardFrigate
    R1_VALID         ,                   //Drone
    R1_VALID|R2_VALID,                   //TargetDrone
    R1_VALID|R2_VALID,                   //HeadShotAsteroid
    R1_VALID|R2_VALID,                   //CryoTray
    P1_VALID         ,                   //P1Fighter
    P1_VALID         ,                   //P1IonArrayFrigate
    P1_VALID         ,                   //P1MissileCorvette
    P1_VALID         ,                   //P1Mothership
    P1_VALID         ,                   //P1StandardCorvette
    P2_VALID         ,                   //P2AdvanceSwarmer
    P2_VALID         ,                   //P2FuelPod
    P2_VALID         ,                   //P2Mothership
    P2_VALID         ,                   //P2MultiBeamFrigate
    P2_VALID         ,                   //P2Swarmer
    P3_VALID         ,                   //P3Destroyer
    P3_VALID         ,                   //P3Frigate
    P3_VALID         ,                   //P3Megaship
    Traders_VALID    ,                   //FloatingCity
    Traders_VALID    ,                   //CargoBarge
    Traders_VALID    ,                   //MiningBase
    Traders_VALID    ,                   //ResearchStation
    Traders_VALID    ,                   //JunkYardDawg
    Traders_VALID                        //JunkYardHQ
};

bool universeForceDefaultShip = FALSE;   //flag for forcing ships to be loaded as rave borgs

#if UNIV_SHIP_LOADFREE_LOG
bool univLoadFreeLog = FALSE;
#endif

/*=============================================================================
    Private Function prototypes:
=============================================================================*/

static void loadMexDataCB(char *directory,char *field,void *dataToFillIn);
//static void setRaceCB(char *directory,char *field,void *dataToFillIn);
static void setClassCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetFramesCB(char *directory,char *field,void *dataToFillIn);
static void LODScriptLoad(char *directory,char *field,void *dataToFillIn);
static void setGunBindInfo(char *directory,char *field,void *dataToFillIn);
static void setAnimationBindInfo(char *directory,char *field,void *dataToFillIn);
static void setMadStartInfo(char *directory,char *field,void *dataToFillIn);
static void setGunAnimInfo(char *directory,char *field,void *dataToFillIn);
static void setDockAnimInfo(char *directory,char *field,void *dataToFillIn);
static void setMothershipDoorAnimInfo(char *directory,char *field,void *dataToFillIn);
static void setSpecialCaseAnimInfo(char *directory,char *field,void *dataToFillIn);

/*=============================================================================
    Private Data:
=============================================================================*/

lodinfo *defaultlod = NULL;         // remove later when all stubs are taken out
void *defaultmex = NULL;            // remove later when all stubs are taken out

udword universeUpdateCounter = 0;

ShipStaticInfo ShipStaticInfoR1[TOTAL_STD_SHIPS];
ShipStaticInfo ShipStaticInfoR2[TOTAL_STD_SHIPS];
ShipStaticInfo ShipStaticInfoP1[TOTAL_P1_SHIPS];
ShipStaticInfo ShipStaticInfoP2[TOTAL_P2_SHIPS];
ShipStaticInfo ShipStaticInfoP3[TOTAL_P3_SHIPS];
ShipStaticInfo ShipStaticInfoTraders[TOTAL_TRADERS_SHIPS];

ShipStaticInfo *RaceShipStaticInfos[NUM_RACES] =
{
    ShipStaticInfoR1,
    ShipStaticInfoR2,
    ShipStaticInfoP1,
    ShipStaticInfoP2,
    ShipStaticInfoP3,
    ShipStaticInfoTraders
};

CustShipHeader *shipHeaders[TOTAL_NUM_SHIPS] =
{
    &AdvanceSupportFrigateHeader,   //AdvanceSupportFrigate
    &AttackBomberHeader,            //AttackBomber
    &CarrierHeader,                 //Carrier
    &CloakedFighterHeader,          //CloakedFighter
    &CloakGeneratorHeader,          //CloakGenerator
    &DDDFrigateHeader,              //DDDFrigate
    &DefenseFighterHeader,          //DefenseFighter
    &DFGFrigateHeader,              //DFGFrigate
    &GravWellGeneratorHeader,       //GravWellGenerator
    &HeavyCorvetteHeader,           //HeavyCorvette
    &HeavyCruiserHeader,            //HeavyCruiser
    &GenericDefenderHeader,         //HeavyDefender
    &GenericInterceptorHeader,      //HeavyInterceptor
    &IonCannonFrigateHeader,        //IonCannonFrigate
    &LightCorvetteHeader,           //LightCorvette
    &GenericDefenderHeader,         //LightDefender
    &GenericInterceptorHeader,      //LightInterceptor
    &MinelayerCorvetteHeader,       //MinelayerCorvette
    &MissileDestroyerHeader,        //MissileDestroyer
    &MothershipHeader,              //Mothership
    &MultiGunCorvetteHeader,        //MultiGunCorvette
    &ProbeHeader,                   //Probe
    &ProximitySensorHeader,         //ProximitySensor
    &RepairCorvetteHeader,          //RepairCorvette
    &ResearchShipHeader,            //ResearchShip
    &ResourceCollectorHeader,       //ResourceCollector
    &ResourceControllerHeader,      //ResourceController
    &SalCapCorvetteHeader,          //SalCapCorvette
    &SensorArrayHeader,             //SensorArray
    &StandardDestroyerHeader,       //StandardDestroyer
    &StandardFrigateHeader,         //StandardFrigate
    &DroneHeader,                   //Drone
    &TargetDroneHeader,             //TargetDrone (header located in DefaultShip.c)
    &DoNothingShipHeader,           //HeadShotAsteroid
    &DoNothingShipHeader,           //CryoTray
    &GenericInterceptorHeader,      //P1Fighter
    &P1IonArrayFrigateHeader,       //P1IonArrayFrigate
    &P1MissileCorvetteHeader,       //P1MissileCorvette
    &P1MothershipHeader,            //P1Mothership
    &P1StandardCorvetteHeader,      //P1StandardCorvette
    &P2AdvanceSwarmerHeader,        //P2AdvanceSwarmer
    &P2FuelPodHeader,               //P2FuelPod
    &P2MothershipHeader,            //P2Mothership
    &P2MultiBeamFrigateHeader,      //P2MultiBeamFrigate
    &P2SwarmerHeader,               //P2Swarmer
    &P3StandardShipHeader,          //P3Destroyer
    &P3StandardShipHeader,          //P3Frigate
    &P3StandardShipHeader,          //P3Megaship
    &FloatingCityHeader,            //FloatingCity
    &DefaultShipHeader,             //CargoBarge
    &MiningBaseHeader,              //MiningBase
    &DefaultShipHeader,             //ResearchStation
    &JunkYardDawgHeader,            //JunkYardDawg
    &DoNothingShipHeader,           //JunkYardHQ
    &DoNothingShipHeader,           //GhostShip
    &StationaryGunHeader,           //Junk_LGun
    &StationaryGunHeader,           //Junk_SGun
    &DoNothingShipHeader,           //ResearchStationBridge
    &DoNothingShipHeader           //ResearchStationTower
};

real32 rowAvoidBy[TOTAL_NUM_SHIPS] =
{
    1000.0,         //AdvanceSupportFrigate
    0.0,            //AttackBomber
    1600.0,         //Carrier
    0.0,            //CloakedFighter
    1000.0,         //CloakGenerator
    1000.0,         //DDDFrigate
    0.0,            //DefenseFighter
    1000.0,         //DFGFrigate
    1000.0,         //GravWellGenerator
    0.0,            //HeavyCorvette
    1500.0,         //HeavyCruiser
    0.0,            //HeavyDefender
    0.0,            //HeavyInterceptor
    1000.0,         //IonCannonFrigate
    0.0,            //LightCorvette
    0.0,            //LightDefender
    0.0,            //LightInterceptor
    0.0,            //MinelayerCorvette
    1500.0,         //MissileDestroyer
    2000.0,         //Mothership
    0.0,            //MultiGunCorvette
    0.0,            //Probe
    0.0,            //ProximitySensor
    0.0,            //RepairCorvette
    1000.0,         //ResearchShip
    1000.0,         //ResourceCollector
    1000.0,         //ResourceController
    0.0,            //SalCapCorvette
    1000.0,         //SensorArray
    1500.0,         //StandardDestroyer
    1000.0,         //StandardFrigate
    0.0,            //Drone
    0.0,            //TargetDrone
    2000.0,         //HeadShotAsteroid
    2000.0,         //CryoTray
    0.0,            //P1Fighter
    1000.0,         //P1IonArrayFrigate
    0.0,            //P1MissileCorvette
    1700.0,         //P1Mothership
    0.0,            //P1StandardCorvette
    0.0,            //P2AdvanceSwarmer
    1000.0,         //P2FuelPod
    1700.0,         //P2Mothership
    1000.0,         //P2MultiBeamFrigate
    0.0,            //P2Swarmer
    1500.0,         //P3Destroyer
    1700.0,         //P3Frigate
    2000.0,         //P3Megaship
    2000.0,         //FloatingCity
    1000.0,         //CargoBarge
    1700.0,         //MiningBase
    1700.0,         //ResearchStation
    1000.0,         //JunkYardDawg
    1000.0,         //JunkYardHQ
    2000.0,         //Junk_LGun
    1000.0,         //Junk_SGun
    2000.0,         //ResearchStationBridge
    2000.0          //ResearchStationTower
};

// rightOfWay for ships.
// -1 indicates lowest priority of the low (e.g. pebbles in space)
// 0 indicates strikecraft, probes, etc.
// 1 indicates frigates
// 2 indicates cruisers/destroyers
// 3 indicates carriers
// 4 indicates mothership

sbyte rightOfWays[TOTAL_NUM_SHIPS] =
{
    1,              //AdvanceSupportFrigate
    0,              //AttackBomber
    3,              //Carrier
    0,              //CloakedFighter
    1,              //CloakGenerator
    1,              //DDDFrigate
    0,              //DefenseFighter
    1,              //DFGFrigate
    1,              //GravWellGenerator
    0,              //HeavyCorvette
    2,              //HeavyCruiser
    0,              //HeavyDefender
    0,              //HeavyInterceptor
    1,              //IonCannonFrigate
    0,              //LightCorvette
    0,              //LightDefender
    0,              //LightInterceptor
    0,              //MinelayerCorvette
    2,              //MissileDestroyer
    4,              //Mothership
    0,              //MultiGunCorvette
    0,              //Probe
    0,              //ProximitySensor
    0,              //RepairCorvette
    2,              //ResearchShip
    1,              //ResourceCollector
    2,              //ResourceController
    0,              //SalCapCorvette
    1,              //SensorArray
    2,              //StandardDestroyer
    1,              //StandardFrigate
    0,              //Drone
    0,              //TargetDrone
    4,              //HeadShotAsteroid
    4,              //CryoTray
    0,              //P1Fighter
    1,              //P1IonArrayFrigate
    0,              //P1MissileCorvette
    4,              //P1Mothership
    0,              //P1StandardCorvette
    0,              //P2AdvanceSwarmer
    1,              //P2FuelPod
    4,              //P2Mothership
    1,              //P2MultiBeamFrigate
    0,              //P2Swarmer
    2,              //P3Destroyer
    3,              //P3Frigate
    4,              //P3Megaship
    4,              //FloatingCity
    1,              //CargoBarge
    4,              //MiningBase
    4,              //ResearchStation
    1,              //JunkYardDawg
    2,              //JunkYardHQ
    1,              //Junk_LGun
    0,               //Junk_SGun
    4,               //ResearchStationBridge
    4                //ResearchStationTower
};

scriptEntry ShipStaticCapLimits[] =
{
    { "ShipTotalCaps",              scriptSetSdwordCB, &cdMaxShipsAllowed },
    { "CapAdvanceSupportFrigate",   scriptSetSdwordCB, &(cdLimitCaps[0]) },
    { "CapAttackBomber",            scriptSetSdwordCB, &cdLimitCaps[1] },
    { "CapCarrier",                 scriptSetSdwordCB, &cdLimitCaps[2] },
    { "CapCloakedFighter",          scriptSetSdwordCB, &cdLimitCaps[3] },
    { "CapCloakGenerator",          scriptSetSdwordCB, &cdLimitCaps[4] },
    { "CapDDDFrigate",              scriptSetSdwordCB, &cdLimitCaps[5] },
    { "CapDefenseFighter",          scriptSetSdwordCB, &cdLimitCaps[6] },
    { "CapDFGFrigate",              scriptSetSdwordCB, &cdLimitCaps[7] },
    { "CapGravWellGenerator",       scriptSetSdwordCB, &cdLimitCaps[8] },
    { "CapHeavyCorvette",           scriptSetSdwordCB, &cdLimitCaps[9] },
    { "CapHeavyCruiser",            scriptSetSdwordCB, &cdLimitCaps[10] },
    { "CapHeavyDefender",           scriptSetSdwordCB, &cdLimitCaps[11] },
    { "CapHeavyInterceptor",        scriptSetSdwordCB, &cdLimitCaps[12] },
    { "CapIonCannonFrigate",        scriptSetSdwordCB, &cdLimitCaps[13] },
    { "CapLightCorvette",           scriptSetSdwordCB, &cdLimitCaps[14] },
    { "CapLightDefender",           scriptSetSdwordCB, &cdLimitCaps[15] },
    { "CapLightInterceptor",        scriptSetSdwordCB, &cdLimitCaps[16] },
    { "CapMinelayerCorvette",       scriptSetSdwordCB, &cdLimitCaps[17] },
    { "CapMissileDestroyer",        scriptSetSdwordCB, &cdLimitCaps[18] },
    { "CapMothership",              scriptSetSdwordCB, &cdLimitCaps[19] },
    { "CapMultiGunCorvette",        scriptSetSdwordCB, &cdLimitCaps[20] },
    { "CapProbe",                   scriptSetSdwordCB, &cdLimitCaps[21] },
    { "CapProximitySensor",         scriptSetSdwordCB, &cdLimitCaps[22] },
    { "CapRepairCorvette",          scriptSetSdwordCB, &cdLimitCaps[23] },
    { "CapResearchShip",            scriptSetSdwordCB, &cdLimitCaps[24] },
    { "CapResourceCollector",       scriptSetSdwordCB, &cdLimitCaps[25] },
    { "CapResourceController",      scriptSetSdwordCB, &cdLimitCaps[26] },
    { "CapSalCapCorvette",          scriptSetSdwordCB, &cdLimitCaps[27] },
    { "CapSensorArray",             scriptSetSdwordCB, &cdLimitCaps[28] },
    { "CapStandardDestroyer",       scriptSetSdwordCB, &cdLimitCaps[29] },
    { "CapStandardFrigate",         scriptSetSdwordCB, &cdLimitCaps[30] },
    { "CapDrone",                   scriptSetSdwordCB, &cdLimitCaps[31] },
    { "CapTargetDrone",             scriptSetSdwordCB, &cdLimitCaps[32] },
    { "CapHeadShotAsteroid",        scriptSetSdwordCB, &cdLimitCaps[33] },
    { "CapCryoTray",                scriptSetSdwordCB, &cdLimitCaps[34] },
    { "CapP1Fighter",               scriptSetSdwordCB, &cdLimitCaps[35] },
    { "CapP1IonArrayFrigate",       scriptSetSdwordCB, &cdLimitCaps[36] },
    { "CapP1MissileCorvette",       scriptSetSdwordCB, &cdLimitCaps[37] },
    { "CapP1Mothership",            scriptSetSdwordCB, &cdLimitCaps[38] },
    { "CapP1StandardCorvette",      scriptSetSdwordCB, &cdLimitCaps[39] },
    { "CapP2AdvanceSwarmer",        scriptSetSdwordCB, &cdLimitCaps[40] },
    { "CapP2FuelPod",               scriptSetSdwordCB, &cdLimitCaps[41] },
    { "CapP2Mothership",            scriptSetSdwordCB, &cdLimitCaps[42] },
    { "CapP2MultiBeamFrigate",      scriptSetSdwordCB, &cdLimitCaps[43] },
    { "CapP2Swarmer",               scriptSetSdwordCB, &cdLimitCaps[44] },
    { "CapP3Destroyer",             scriptSetSdwordCB, &cdLimitCaps[45] },
    { "CapP3Frigate",               scriptSetSdwordCB, &cdLimitCaps[46] },
    { "CapP3Megaship",              scriptSetSdwordCB, &cdLimitCaps[47] },
    { "CapFloatingCity",            scriptSetSdwordCB, &cdLimitCaps[48] },
    { "CapCargoBarge",              scriptSetSdwordCB, &cdLimitCaps[49] },
    { "CapMiningBase",              scriptSetSdwordCB, &cdLimitCaps[50] },
    { "CapResearchStation",         scriptSetSdwordCB, &cdLimitCaps[51] },
    { "CapJunkYardDawg",            scriptSetSdwordCB, &cdLimitCaps[52] },
    { "CapJunkYardHQ",              scriptSetSdwordCB, &cdLimitCaps[53] },
    { "CapGhostShip",               scriptSetSdwordCB, &cdLimitCaps[54] },
    { "CapJunk_LGun",               scriptSetSdwordCB, &cdLimitCaps[55] },
    { "CapJunk_SGun",               scriptSetSdwordCB, &cdLimitCaps[56] },
    { "CapResearchStationBridge",               scriptSetSdwordCB, &cdLimitCaps[57] },
    { "CapResearchStationTower",               scriptSetSdwordCB, &cdLimitCaps[58] },
    { "CapCLASS_Mothership",        scriptSetSdwordCB, &cdClassCaps[0]  },
    { "CapCLASS_HeavyCruiser",      scriptSetSdwordCB, &cdClassCaps[1]  },
    { "CapCLASS_Carrier",           scriptSetSdwordCB, &cdClassCaps[2]  },
    { "CapCLASS_Destroyer",         scriptSetSdwordCB, &cdClassCaps[3]  },
    { "CapCLASS_Frigate",           scriptSetSdwordCB, &cdClassCaps[4]  },
    { "CapCLASS_Corvette",          scriptSetSdwordCB, &cdClassCaps[5]  },
    { "CapCLASS_Fighter",           scriptSetSdwordCB, &cdClassCaps[6]  },
    { "CapCLASS_Resource",          scriptSetSdwordCB, &cdClassCaps[7]  },
    { "CapCLASS_NonCombat",         scriptSetSdwordCB, &cdClassCaps[8]  },

    { "MaxJobsPerClass[Mothership]",    scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Mothership]  },
    { "MaxJobsPerClass[HeavyCruiser]",  scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_HeavyCruiser]},
    { "MaxJobsPerClass[Carrier]",       scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Carrier]},
    { "MaxJobsPerClass[Destroyer]",     scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Destroyer]},
    { "MaxJobsPerClass[Frigate]",       scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Frigate]},
    { "MaxJobsPerClass[Corvette]",      scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Corvette]},
    { "MaxJobsPerClass[Fighter]",       scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Fighter]},
    { "MaxJobsPerClass[Resource]",      scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Resource]},
    { "MaxJobsPerClass[NonCombat]",     scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_NonCombat]},
    endEntry
};

scriptStructEntry ShipStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(ShipStaticInfoR1[0].staticheader.LOD), (udword) &(ShipStaticInfoR1[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(ShipStaticInfoR1[0].staticheader.pMexData), (udword) &(ShipStaticInfoR1[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].staticheader.mass),      (udword) &(ShipStaticInfoR1[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.momentOfInertiaX), (udword) &(ShipStaticInfoR1[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.momentOfInertiaY), (udword) &(ShipStaticInfoR1[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.momentOfInertiaZ), (udword) &(ShipStaticInfoR1[0]) },

    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(ShipStaticInfoR1[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(ShipStaticInfoR1[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(ShipStaticInfoR1[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(ShipStaticInfoR1[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(ShipStaticInfoR1[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(ShipStaticInfoR1[0]) },



    { "immobile",        scriptSetBool8,    (udword) &(ShipStaticInfoR1[0].staticheader.immobile), (udword) &(ShipStaticInfoR1[0]) },
    { "rightOfWay",      scriptSetSbyteCB,  (udword) &(ShipStaticInfoR1[0].staticheader.rightOfWay), (udword) &(ShipStaticInfoR1[0]) },
    { "collspheresize", scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.collspheresize),  (udword) &(ShipStaticInfoR1[0]) },
    { "approxcollmodifier", scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(ShipStaticInfoR1[0]) },
    { "avoidcollmodifier", scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(ShipStaticInfoR1[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].staticheader.maxvelocity),  (udword) &(ShipStaticInfoR1[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].staticheader.maxrot),  (udword) &(ShipStaticInfoR1[0]) },
    { "bulletRange[0]", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].bulletRange[0]),  (udword) &(ShipStaticInfoR1[0]) },
    { "bulletRange[1]", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].bulletRange[1]),  (udword) &(ShipStaticInfoR1[0]) },
    { "bulletRange[2]", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].bulletRange[2]),  (udword) &(ShipStaticInfoR1[0]) },
    { "minBulletRange[0]", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].minBulletRange[0]),  (udword) &(ShipStaticInfoR1[0]) },
    { "minBulletRange[1]", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].minBulletRange[1]),  (udword) &(ShipStaticInfoR1[0]) },
    { "minBulletRange[2]", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].minBulletRange[2]),  (udword) &(ShipStaticInfoR1[0]) },
    { "blastRadiusShockWave", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].blastRadiusShockWave),  (udword) &(ShipStaticInfoR1[0]) },
    { "blastRadiusDamage", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].blastRadiusDamage),  (udword) &(ShipStaticInfoR1[0]) },
    { "passiveRetaliateZone", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].passiveRetaliateZone),  (udword) &(ShipStaticInfoR1[0]) },
    { "shipclass",      setClassCB,         (udword) &(ShipStaticInfoR1[0].shipclass), (udword) &(ShipStaticInfoR1[0]) },
    { "rotateToRetaliate",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].rotateToRetaliate), (udword) &(ShipStaticInfoR1[0]) },
    { "canTargetMultipleTargets",scriptSetBool8,(udword) &(ShipStaticInfoR1[0].canTargetMultipleTargets), (udword) &(ShipStaticInfoR1[0]) },
    { "canBuildShips",  scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canBuildShips), (udword) &(ShipStaticInfoR1[0]) },
    { "canReceiveShips",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canReceiveShips), (udword) &(ShipStaticInfoR1[0]) },
    { "canReceiveResources",scriptSetBool8, (udword) &(ShipStaticInfoR1[0].canReceiveResources), (udword) &(ShipStaticInfoR1[0]) },
    { "canReceiveShipsPermanently",scriptSetBool8, (udword) &(ShipStaticInfoR1[0].canReceiveShipsPermanently), (udword) &(ShipStaticInfoR1[0]) },
    { "canReceiveShipsForRetire",scriptSetBool8, (udword) &(ShipStaticInfoR1[0].canReceiveShipsForRetire), (udword) &(ShipStaticInfoR1[0]) },
    { "canBuildBigShips",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canBuildBigShips), (udword) &(ShipStaticInfoR1[0]) },
    { "canReceiveFighters",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canReceiveTheseShips[THISSHIPIS_FIGHTER]), (udword) &(ShipStaticInfoR1[0]) },
    { "canReceiveCorvettes",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canReceiveTheseShips[THISSHIPIS_CORVETTE]), (udword) &(ShipStaticInfoR1[0]) },
    { "canReceiveResourceCollectors",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canReceiveTheseShips[THISSHIPIS_RESOURCER]), (udword) &(ShipStaticInfoR1[0]) },
    { "canReceiveOtherNonCapitalShips",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canReceiveTheseShips[THISSHIPIS_OTHERNONCAPITALSHIP]), (udword) &(ShipStaticInfoR1[0]) },
    { "canHandleNumShipsDocking",scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].canHandleNumShipsDocking), (udword) &(ShipStaticInfoR1[0]) },
    { "canHandleNumShipsDepositingRU",scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].canHandleNumShipsDepositingRU), (udword) &(ShipStaticInfoR1[0]) },
    { "svManeuverability",scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].svManeuverability), (udword) &(ShipStaticInfoR1[0]) },
    { "svCoverage",scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].svCoverage), (udword) &(ShipStaticInfoR1[0]) },
    { "svFirePower",scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].svFirePower), (udword) &(ShipStaticInfoR1[0]) },

    { "maxDockableFighters" ,scriptSetSwordCB, (udword) &(ShipStaticInfoR1[0].maxDockableFighters),(udword) &(ShipStaticInfoR1[0])},
    { "maxDockableCorvettes",scriptSetSwordCB, (udword) &(ShipStaticInfoR1[0].maxDockableCorvettes),(udword) &(ShipStaticInfoR1[0])},
    { "specialActivateIsContinuous",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].specialActivateIsContinuous), (udword) &(ShipStaticInfoR1[0]) },
    { "canSpecialBandBoxFriendlies",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canSpecialBandBoxFriendlies), (udword) &(ShipStaticInfoR1[0]) },
    { "canSingleClickSpecialActivate",scriptSetBool8,     (udword) &(ShipStaticInfoR1[0].canSingleClickSpecialActivate), (udword) &(ShipStaticInfoR1[0]) },
    { "buildCost",      scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].buildCost), (udword) &(ShipStaticInfoR1[0]) },
    { "buildTime",      scriptSetFramesCB,  (udword) &(ShipStaticInfoR1[0].buildTime), (udword) &(ShipStaticInfoR1[0]) },
    { "resourcesAtOneTime",   scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].resourcesAtOneTime), (udword) &(ShipStaticInfoR1[0]) },
    { "maxresources",   scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].maxresources), (udword) &(ShipStaticInfoR1[0]) },
    { "harvestAmount",  scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].harvestAmount), (udword) &(ShipStaticInfoR1[0]) },
    { "harvestRate",    scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].harvestRate), (udword) &(ShipStaticInfoR1[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].maxhealth), (udword) &(ShipStaticInfoR1[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].minCollDamage), (udword) &(ShipStaticInfoR1[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].maxCollDamage), (udword) &(ShipStaticInfoR1[0]) },
    { "groupSize",      scriptSetSdwordCB,  (udword) &(ShipStaticInfoR1[0].groupSize), (udword) &(ShipStaticInfoR1[0]) },
    { "maxfuel",        scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].maxfuel),   (udword) &(ShipStaticInfoR1[0]) },
    { "lowfuelpoint",   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].lowfuelpoint),   (udword) &(ShipStaticInfoR1[0]) },
    { "fuelburnrate",   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].fuelburnrate), (udword) &(ShipStaticInfoR1[0]) },
    { "attackfuelburnrate", scriptSetReal32CB, (udword) &(ShipStaticInfoR1[0].attackfuelburnrate), (udword) &(ShipStaticInfoR1[0]) },
    { "repairTime",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].repairTime), (udword) &(ShipStaticInfoR1[0]) },
    { "repairDamage",   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].repairDamage), (udword) &(ShipStaticInfoR1[0]) },
    { "repairCombatTime",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].repairCombatTime), (udword) &(ShipStaticInfoR1[0]) },
    { "repairCombatDamage",   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].repairCombatDamage), (udword) &(ShipStaticInfoR1[0]) },
    { "pumpFuelRate",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].pumpFuelRate), (udword) &(ShipStaticInfoR1[0]) },
    { "repairOtherShipRate",   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].repairOtherShipRate), (udword) &(ShipStaticInfoR1[0]) },
    { "repairResourceCollectorRate",   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].repairResourceCollectorRate), (udword) &(ShipStaticInfoR1[0]) },
    { "dockShipRange",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].dockShipRange), (udword) &(ShipStaticInfoR1[0]) },
    { "clearanceDistance",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].clearanceDistance), (udword) &(ShipStaticInfoR1[0]) },
    { "clearanceDirection",   scriptSetUdwordCB,  (udword) &(ShipStaticInfoR1[0].clearanceDirection), (udword) &(ShipStaticInfoR1[0]) },
    { "formationPaddingModifier", scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].formationPaddingModifier), (udword) &(ShipStaticInfoR1[0]) },
    { "cannotForceAttackIfOwnShip",scriptSetBool,     (udword) &(ShipStaticInfoR1[0].cannotForceAttackIfOwnShip), (udword) &(ShipStaticInfoR1[0]) },
    { "passiveAttackPenaltyExempt",scriptSetBool,     (udword) &(ShipStaticInfoR1[0].passiveAttackPenaltyExempt), (udword) &(ShipStaticInfoR1[0]) },
    { "isCapitalShip",scriptSetBool,     (udword) &(ShipStaticInfoR1[0].shipIsCapital), (udword) &(ShipStaticInfoR1[0]) },
    { "cantMoveAndAttack",scriptSetBool,     (udword) &(ShipStaticInfoR1[0].cantMoveAndAttack), (udword) &(ShipStaticInfoR1[0]) },

    { "repairBeamCapable",scriptSetBool,     (udword) &(ShipStaticInfoR1[0].repairBeamCapable), (udword) &(ShipStaticInfoR1[0]) },
    { "healthPerSecond", scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].healthPerSecond), (udword) &(ShipStaticInfoR1[0]) },
    { "CapitalDistanceRepairStart2", scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].CapitalDistanceRepairStart2), (udword) &(ShipStaticInfoR1[0]) },
    { "CapitalDistanceRepairStart", scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].CapitalDistanceRepairStart), (udword) &(ShipStaticInfoR1[0]) },
    { "AngleDotProdThreshold", scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].AngleDotProdThreshold), (udword) &(ShipStaticInfoR1[0]) },

    { "collSideModifiers[TRANS_UP]",       scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].collSideModifiers[TRANS_UP]), (udword) &(ShipStaticInfoR1[0]) },
    { "collSideModifiers[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].collSideModifiers[TRANS_DOWN]), (udword) &(ShipStaticInfoR1[0]) },
    { "collSideModifiers[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].collSideModifiers[TRANS_RIGHT]), (udword) &(ShipStaticInfoR1[0]) },
    { "collSideModifiers[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].collSideModifiers[TRANS_LEFT]), (udword) &(ShipStaticInfoR1[0]) },
    { "collSideModifiers[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].collSideModifiers[TRANS_FORWARD]), (udword) &(ShipStaticInfoR1[0]) },
    { "collSideModifiers[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].collSideModifiers[TRANS_BACKWARD]), (udword) &(ShipStaticInfoR1[0]) },
    { "thruststrength[TRANS_UP]",       scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].thruststrengthstat[TRANS_UP]), (udword) &(ShipStaticInfoR1[0]) },
    { "thruststrength[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].thruststrengthstat[TRANS_DOWN]), (udword) &(ShipStaticInfoR1[0]) },
    { "thruststrength[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].thruststrengthstat[TRANS_RIGHT]), (udword) &(ShipStaticInfoR1[0]) },
    { "thruststrength[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].thruststrengthstat[TRANS_LEFT]), (udword) &(ShipStaticInfoR1[0]) },
    { "thruststrength[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].thruststrengthstat[TRANS_FORWARD]), (udword) &(ShipStaticInfoR1[0]) },
    { "thruststrength[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].thruststrengthstat[TRANS_BACKWARD]), (udword) &(ShipStaticInfoR1[0]) },
    { "rotstrength[ROT_YAWLEFT]",       scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].rotstrengthstat[ROT_YAWLEFT]), (udword) &(ShipStaticInfoR1[0]) },
    { "rotstrength[ROT_YAWRIGHT]",      scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].rotstrengthstat[ROT_YAWRIGHT]), (udword) &(ShipStaticInfoR1[0]) },
    { "rotstrength[ROT_PITCHUP]",       scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].rotstrengthstat[ROT_PITCHUP]), (udword) &(ShipStaticInfoR1[0]) },
    { "rotstrength[ROT_PITCHDOWN]",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].rotstrengthstat[ROT_PITCHDOWN]), (udword) &(ShipStaticInfoR1[0]) },
    { "rotstrength[ROT_ROLLRIGHT]",     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].rotstrengthstat[ROT_ROLLRIGHT]), (udword) &(ShipStaticInfoR1[0]) },
    { "rotstrength[ROT_ROLLLEFT]",      scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].rotstrengthstat[ROT_ROLLLEFT]), (udword) &(ShipStaticInfoR1[0]) },
    { "turnspeed[TURN_YAW]",            scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].turnspeedstat[TURN_YAW]), (udword) &(ShipStaticInfoR1[0]) },
    { "turnspeed[TURN_PITCH]",          scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].turnspeedstat[TURN_PITCH]), (udword) &(ShipStaticInfoR1[0]) },
    { "turnspeed[TURN_ROLL]",           scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].turnspeedstat[TURN_ROLL]), (udword) &(ShipStaticInfoR1[0]) },
    { "N-LIPS",                         scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].scaleCap), (udword) &(ShipStaticInfoR1[0]) },
//    { "scaleCap",                       scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].scaleCap), (udword) &(ShipStaticInfoR1[0]) },
#if SO_CLOOGE_SCALE
    { "scaleFactor",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].scaleFactor), (udword) &(ShipStaticInfoR1[0]) },
#endif
//    { "NumRepairDroids",                scriptSetUdwordCB,  (udword) &(ShipStaticInfoR1[0].nRepairDroids), (udword) &(ShipStaticInfoR1[0]) },
    { "trailStyle",                     scriptSetUdwordCB,  (udword) &(ShipStaticInfoR1[0].trailStyle[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailWidth",                     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailWidth[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailHeight",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailHeight[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailAngle",                     scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailAngle[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailRibbonAdjust",              scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailRibbonAdjust[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailLength",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailLength[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailScaleCap",                  scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailScaleCap[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailStyle2",                    scriptSetUdwordCB,  (udword) &(ShipStaticInfoR1[0].trailStyle[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailWidth2",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailWidth[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailHeight2",                   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailHeight[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailAngle2",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailAngle[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailRibbonAdjust2",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailRibbonAdjust[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailLength2",                   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailLength[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailScaleCap2",                 scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailScaleCap[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailStyle3",                    scriptSetUdwordCB,  (udword) &(ShipStaticInfoR1[0].trailStyle[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailWidth3",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailWidth[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailHeight3",                   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailHeight[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailAngle3",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailAngle[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailRibbonAdjust3",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailRibbonAdjust[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailLength3",                   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailLength[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailScaleCap3",                 scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailScaleCap[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailStyle4",                    scriptSetUdwordCB,  (udword) &(ShipStaticInfoR1[0].trailStyle[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailWidth4",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailWidth[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailHeight4",                   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailHeight[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailAngle4",                    scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailAngle[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailRibbonAdjust4",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailRibbonAdjust[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailLength4",                   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailLength[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailScaleCap4",                 scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailScaleCap[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailSpriteRadius",              scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailSpriteRadius[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailSpriteRadius2",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailSpriteRadius[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailSpriteRadius3",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailSpriteRadius[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailSpriteRadius4",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailSpriteRadius[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailSpriteOffset",              scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailSpriteOffset[0]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailSpriteOffset2",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailSpriteOffset[1]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailSpriteOffset3",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailSpriteOffset[2]), (udword) &(ShipStaticInfoR1[0]) },
    { "trailSpriteOffset4",             scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].trailSpriteOffset[3]), (udword) &(ShipStaticInfoR1[0]) },
    { "minimumZoomDistance",            scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].minimumZoomDistance), (udword) &(ShipStaticInfoR1[0]) },
    { "renderlistFade",                 scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].renderlistFade), (udword) &(ShipStaticInfoR1[0]) },
    { "renderlistLimit",                scriptSetReal32SqrCB, (udword) &(ShipStaticInfoR1[0].renderlistLimitSqr), (udword) &(ShipStaticInfoR1[0]) },
    { "dockLightNear",                  scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].dockLightNear), (udword) &(ShipStaticInfoR1[0]) },
    { "dockLightFar",                   scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].dockLightFar), (udword) &(ShipStaticInfoR1[0]) },
    { "dockLightColor",                 scriptSetRGBCB,     (udword) &(ShipStaticInfoR1[0].dockLightColor), (udword) &(ShipStaticInfoR1[0]) },
    { "hyperspaceColor",                scriptSetRGBCB,     (udword) &(ShipStaticInfoR1[0].hyperspaceColor), (udword) &(ShipStaticInfoR1[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.preciseSelection), (udword) &(ShipStaticInfoR1[0]) },
    { "madGunOpenDamagedHealthThreshold",  scriptSetReal32CB,(udword) &(ShipStaticInfoR1[0].madGunOpenDamagedHealthThreshold), (udword) &(ShipStaticInfoR1[0]) },
    { "madWingOpenDamagedHealthThreshold", scriptSetReal32CB,(udword) &(ShipStaticInfoR1[0].madWingOpenDamagedHealthThreshold), (udword) &(ShipStaticInfoR1[0]) },
    { "NUMBER_OF_SALVAGE_POINTS", scriptSetSalvageStatCB,(udword) &(ShipStaticInfoR1[0]),(udword) &(ShipStaticInfoR1[0]) },
    { "Overlay_Selection_Scalar",       scriptSetReal32CB,  (udword) &(ShipStaticInfoR1[0].tacticalSelectionScale), (udword) &(ShipStaticInfoR1[0]) },

    { NULL,NULL,0,0 }
};

scriptStructEntry HierarchyBindingTable[] =
{
    { "BindGun",                        setGunBindInfo,     0, 0 },
    { "BindAnimations",                 setAnimationBindInfo,0, 0 },
    { NULL,NULL,0,0 }
};

scriptStructEntry MadMaxMadInfoLoad[] =
{
    { "MadStartInfo",                   setMadStartInfo,0, 0 },
    { "MadGunAnims",                    setGunAnimInfo,0, 0 },
    { "MadDockAnims",                   setDockAnimInfo,0, 0 },
    { "MadMothershipDockingAnims",      setMothershipDoorAnimInfo,0,0 },
    { "MadSpecialCaseAnims",      setSpecialCaseAnimInfo,0,0 },

    { NULL,NULL,0,0 }
};

scriptStructEntry AsteroidStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(asteroidStaticInfos[0].staticheader.LOD), (udword) &(asteroidStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(asteroidStaticInfos[0].staticheader.pMexData), (udword) &(asteroidStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.mass),      (udword) &(asteroidStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(asteroidStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(asteroidStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(asteroidStaticInfos[0]) },


    { "immobile",        scriptSetBool8,    (udword) &(asteroidStaticInfos[0].staticheader.immobile), (udword) &(asteroidStaticInfos[0]) },
    { "rightOfWay",      scriptSetSbyteCB,  (udword) &(asteroidStaticInfos[0].staticheader.rightOfWay), (udword) &(asteroidStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(asteroidStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(asteroidStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.maxvelocity),  (udword) &(asteroidStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.maxrot),  (udword) &(asteroidStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].maxhealth), (udword) &(asteroidStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].minCollDamage), (udword) &(asteroidStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].maxCollDamage), (udword) &(asteroidStaticInfos[0]) },
    { "resourcevalue",  scriptSetSdwordCB,  (udword) &(asteroidStaticInfos[0].resourcevalue), (udword) &(asteroidStaticInfos[0]) },
    { "harvestableByMultipleShips",  scriptSetUdwordCB,  (udword) &(asteroidStaticInfos[0].harvestableByMultipleShips), (udword) &(asteroidStaticInfos[0]) },

    { "asteroidCanShrink",scriptSetBool8, (udword) &(asteroidStaticInfos[0].asteroidCanShrink), (udword) &(asteroidStaticInfos[0]) },
    { "asteroidCanBreak", scriptSetBool8, (udword) &(asteroidStaticInfos[0].asteroidCanBreak),  (udword) &(asteroidStaticInfos[0]) },
    { "asteroidMinShrink",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].asteroidMinShrink), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid0]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid0]), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid1]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid1]), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid2]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid2]), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid3]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid3]), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid4]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid4]), (udword) &(asteroidStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.preciseSelection), (udword) &(ShipStaticInfoR1[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry DustCloudStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(dustcloudStaticInfos[0].staticheader.LOD), (udword) &(dustcloudStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(dustcloudStaticInfos[0].staticheader.pMexData), (udword) &(dustcloudStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].staticheader.mass),     (udword) &(dustcloudStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(dustcloudStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(dustcloudStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(dustcloudStaticInfos[0]) },

    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(dustcloudStaticInfos[0]) },

    { "immobile",        scriptSetBool8,    (udword) &(dustcloudStaticInfos[0].staticheader.immobile), (udword) &(dustcloudStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(dustcloudStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(dustcloudStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].staticheader.maxvelocity),  (udword) &(dustcloudStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].staticheader.maxrot),  (udword) &(dustcloudStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].maxhealth), (udword) &(dustcloudStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].minCollDamage), (udword) &(dustcloudStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].maxCollDamage), (udword) &(dustcloudStaticInfos[0]) },
    { "resourcevalue",  scriptSetSdwordCB,  (udword) &(dustcloudStaticInfos[0].resourcevalue), (udword) &(dustcloudStaticInfos[0]) },
    { "harvestableByMultipleShips",  scriptSetUdwordCB, (udword) &(dustcloudStaticInfos[0].harvestableByMultipleShips), (udword) &(dustcloudStaticInfos[0]) },

    { "dustcloudCanShrink",scriptSetBool8, (udword) &(dustcloudStaticInfos[0].dustcloudCanShrink), (udword) &(dustcloudStaticInfos[0]) },
    { "dustcloudMinShrink",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].dustcloudMinShrink), (udword) &(dustcloudStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.preciseSelection), (udword) &(ShipStaticInfoR1[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry NebulaStaticScriptTable[] =
{
    { "LODFile", LODScriptLoad, (udword)&(nebulaStaticInfos[0].staticheader.LOD), (udword)&(nebulaStaticInfos[0]) },
    { "pMexData", loadMexDataCB, (udword)&(nebulaStaticInfos[0].staticheader.pMexData), (udword)&(nebulaStaticInfos[0]) },
    { "mass", scriptSetReal32CB, (udword)&(nebulaStaticInfos[0].staticheader.mass), (udword)&(nebulaStaticInfos[0]) },
    { "momentOfInertiaX", scriptSetReal32CB, (udword)&(nebulaStaticInfos[0].staticheader.momentOfInertiaX), (udword)&(nebulaStaticInfos[0]) },
    { "momentOfInertiaY", scriptSetReal32CB, (udword)&(nebulaStaticInfos[0].staticheader.momentOfInertiaY), (udword)&(nebulaStaticInfos[0]) },
    { "momentOfInertiaZ", scriptSetReal32CB, (udword)&(nebulaStaticInfos[0].staticheader.momentOfInertiaZ), (udword)&(nebulaStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(nebulaStaticInfos[0]) },
    { "immobile", scriptSetBool8, (udword)&(nebulaStaticInfos[0].staticheader.immobile), (udword)&(nebulaStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(nebulaStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(nebulaStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].staticheader.maxvelocity),  (udword) &(nebulaStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].staticheader.maxrot),  (udword) &(nebulaStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].maxhealth), (udword) &(nebulaStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].minCollDamage), (udword) &(nebulaStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].maxCollDamage), (udword) &(nebulaStaticInfos[0]) },
    { "resourcevalue",  scriptSetSdwordCB,  (udword) &(nebulaStaticInfos[0].resourcevalue), (udword) &(nebulaStaticInfos[0]) },
    { "harvestableByMultipleShips",  scriptSetUdwordCB, (udword) &(nebulaStaticInfos[0].harvestableByMultipleShips), (udword) &(nebulaStaticInfos[0]) },

    { "nebulaCanShrink",scriptSetBool8, (udword) &(nebulaStaticInfos[0].nebulaCanShrink), (udword) &(nebulaStaticInfos[0]) },
    { "nebulaMinShrink",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].nebulaMinShrink), (udword) &(nebulaStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.preciseSelection), (udword) &(ShipStaticInfoR1[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry GasCloudStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(gascloudStaticInfos[0].staticheader.LOD), (udword) &(gascloudStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(gascloudStaticInfos[0].staticheader.pMexData), (udword) &(gascloudStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].staticheader.mass),     (udword) &(gascloudStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(gascloudStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(gascloudStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(gascloudStaticInfos[0]) },

    { "immobile",        scriptSetBool8,    (udword) &(gascloudStaticInfos[0].staticheader.immobile), (udword) &(gascloudStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(gascloudStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(gascloudStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].staticheader.maxvelocity),  (udword) &(gascloudStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].staticheader.maxrot),  (udword) &(gascloudStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].maxhealth), (udword) &(gascloudStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].minCollDamage), (udword) &(gascloudStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].maxCollDamage), (udword) &(gascloudStaticInfos[0]) },
    { "resourcevalue",  scriptSetSdwordCB,  (udword) &(gascloudStaticInfos[0].resourcevalue), (udword) &(gascloudStaticInfos[0]) },
    { "harvestableByMultipleShips",  scriptSetUdwordCB, (udword) &(gascloudStaticInfos[0].harvestableByMultipleShips), (udword) &(gascloudStaticInfos[0]) },

    { "gascloudCanShrink",scriptSetBool8, (udword) &(gascloudStaticInfos[0].gascloudCanShrink), (udword) &(gascloudStaticInfos[0]) },
    { "gascloudMinShrink",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].gascloudMinShrink), (udword) &(gascloudStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.preciseSelection), (udword) &(ShipStaticInfoR1[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry DerelictStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(derelictStaticInfos[0].staticheader.LOD), (udword) &(derelictStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(derelictStaticInfos[0].staticheader.pMexData), (udword) &(derelictStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.mass),      (udword) &(derelictStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(derelictStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(derelictStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(derelictStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(derelictStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(derelictStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(derelictStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(derelictStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(derelictStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(derelictStaticInfos[0]) },
    { "rightOfWay",      scriptSetSbyteCB,  (udword) &(derelictStaticInfos[0].staticheader.rightOfWay), (udword) &(derelictStaticInfos[0]) },
    { "immobile",        scriptSetBool8,    (udword) &(derelictStaticInfos[0].staticheader.immobile), (udword) &(derelictStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(derelictStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(derelictStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.maxvelocity),  (udword) &(derelictStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.maxrot),  (udword) &(derelictStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].maxhealth), (udword) &(derelictStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].minCollDamage), (udword) &(derelictStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].maxCollDamage), (udword) &(derelictStaticInfos[0]) },

    { "thruststrength[TRANS_UP]",       scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_UP]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_DOWN]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_RIGHT]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_LEFT]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_FORWARD]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_BACKWARD]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_YAWLEFT]",       scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_YAWLEFT]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_YAWRIGHT]",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_YAWRIGHT]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_PITCHUP]",       scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_PITCHUP]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_PITCHDOWN]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_PITCHDOWN]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_ROLLRIGHT]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_ROLLRIGHT]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_ROLLLEFT]",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_ROLLLEFT]), (udword) &(derelictStaticInfos[0]) },
    { "turnspeed[TURN_YAW]",            scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].turnspeedstat[TURN_YAW]), (udword) &(derelictStaticInfos[0]) },
    { "turnspeed[TURN_PITCH]",          scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].turnspeedstat[TURN_PITCH]), (udword) &(derelictStaticInfos[0]) },
    { "turnspeed[TURN_ROLL]",           scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].turnspeedstat[TURN_ROLL]), (udword) &(derelictStaticInfos[0]) },

    { "pitchdescend",               scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].pitchdescend), (udword) &(derelictStaticInfos[0]) },
    { "sinbank",                    scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].sinbank), (udword) &(derelictStaticInfos[0]) },
    { "pitchturn",                  scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].pitchturn), (udword) &(derelictStaticInfos[0]) },

    { "preciseSelection",           scriptSetUdwordCB,(udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.preciseSelection), (udword) &(ShipStaticInfoR1[0]) },
    { "NUMBER_OF_SALVAGE_POINTS",   scriptSetSalvageStatCB,(udword) &(derelictStaticInfos[0]),(udword) &(derelictStaticInfos[0]) },

    { "scaleFactor",          scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].scaleFactor), (udword) &(derelictStaticInfos[0]) },
    { "worldRender",                scriptSetBool,  (udword) &(derelictStaticInfos[0].worldRender), (udword) &(derelictStaticInfos[0]) },

    { "salvageable",                scriptSetBool,  (udword) &(derelictStaticInfos[0].salvageable), (udword) &(derelictStaticInfos[0]) },

    { "minimumZoomDistance",  scriptSetReal32CB,    (udword) &(derelictStaticInfos[0].minimumZoomDistance), (udword) &(derelictStaticInfos[0]) },
    { "renderlistFade",       scriptSetReal32CB,    (udword) &(derelictStaticInfos[0].renderlistFade),      (udword) &(derelictStaticInfos[0]) },
    { "renderlistLimit",      scriptSetReal32SqrCB, (udword) &(derelictStaticInfos[0].renderlistLimitSqr),  (udword) &(derelictStaticInfos[0]) },

    { NULL,NULL,0,0 }
};

scriptStructEntry MissileStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(missileStaticInfos[0].staticheader.LOD), (udword) &(missileStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(missileStaticInfos[0].staticheader.pMexData), (udword) &(missileStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(missileStaticInfos[0].staticheader.mass),      (udword) &(missileStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(missileStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(missileStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(missileStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(missileStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(missileStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(missileStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(missileStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(missileStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(missileStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(missileStaticInfos[0].staticheader.maxvelocity),  (udword) &(missileStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(missileStaticInfos[0].staticheader.maxrot),  (udword) &(missileStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].maxhealth), (udword) &(missileStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].minCollDamage), (udword) &(missileStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].maxCollDamage), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_UP]",       scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_UP]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_DOWN]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_RIGHT]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_LEFT]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_FORWARD]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_BACKWARD]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_YAWLEFT]",       scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_YAWLEFT]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_YAWRIGHT]",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_YAWRIGHT]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_PITCHUP]",       scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_PITCHUP]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_PITCHDOWN]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_PITCHDOWN]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_ROLLRIGHT]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_ROLLRIGHT]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_ROLLLEFT]",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_ROLLLEFT]), (udword) &(missileStaticInfos[0]) },
    { "turnspeed[TURN_YAW]",            scriptSetReal32CB,  (udword) &(missileStaticInfos[0].turnspeedstat[TURN_YAW]), (udword) &(missileStaticInfos[0]) },
    { "turnspeed[TURN_PITCH]",          scriptSetReal32CB,  (udword) &(missileStaticInfos[0].turnspeedstat[TURN_PITCH]), (udword) &(missileStaticInfos[0]) },
    { "turnspeed[TURN_ROLL]",           scriptSetReal32CB,  (udword) &(missileStaticInfos[0].turnspeedstat[TURN_ROLL]), (udword) &(missileStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.preciseSelection), (udword) &(ShipStaticInfoR1[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry MineStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(mineStaticInfos[0].staticheader.LOD), (udword) &(mineStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(mineStaticInfos[0].staticheader.pMexData), (udword) &(mineStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(mineStaticInfos[0].staticheader.mass),      (udword) &(mineStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(mineStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(mineStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(mineStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(asteroidStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(mineStaticInfos[0].staticheader.maxvelocity),  (udword) &(mineStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(mineStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(mineStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(mineStaticInfos[0]) },
    { "maxvelocity_FORCED",    scriptSetReal32CB,  (udword) &(mineStaticInfos[0].maxvelocity_FORCED),  (udword) &(mineStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(mineStaticInfos[0].staticheader.maxrot),  (udword) &(mineStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].maxhealth), (udword) &(mineStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].minCollDamage), (udword) &(mineStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].maxCollDamage), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_UP]",       scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_UP]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_DOWN]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_RIGHT]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_LEFT]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_FORWARD]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_BACKWARD]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_YAWLEFT]",       scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_YAWLEFT]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_YAWRIGHT]",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_YAWRIGHT]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_PITCHUP]",       scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_PITCHUP]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_PITCHDOWN]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_PITCHDOWN]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_ROLLRIGHT]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_ROLLRIGHT]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_ROLLLEFT]",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_ROLLLEFT]), (udword) &(mineStaticInfos[0]) },
    { "turnspeed[TURN_YAW]",            scriptSetReal32CB,  (udword) &(mineStaticInfos[0].turnspeedstat[TURN_YAW]), (udword) &(mineStaticInfos[0]) },
    { "turnspeed[TURN_PITCH]",          scriptSetReal32CB,  (udword) &(mineStaticInfos[0].turnspeedstat[TURN_PITCH]), (udword) &(mineStaticInfos[0]) },
    { "turnspeed[TURN_ROLL]",           scriptSetReal32CB,  (udword) &(mineStaticInfos[0].turnspeedstat[TURN_ROLL]), (udword) &(mineStaticInfos[0]) },
    { "MINE_STOP_FRICTION",           scriptSetReal32CB,  (udword) &(mineStaticInfos[0].MINE_STOP_FRICTION), (udword) &(mineStaticInfos[0]) },
    { "MINE_RANGESQR",           scriptSetReal32SqrCB,  (udword) &(mineStaticInfos[0].MINE_RANGESQR), (udword) &(mineStaticInfos[0]) },
    { "MINE_RANGESQR_FORCED",           scriptSetReal32SqrCB,  (udword) &(mineStaticInfos[0].MINE_RANGESQR_FORCED), (udword) &(mineStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(ShipStaticInfoR1[0].staticheader.staticCollInfo.preciseSelection), (udword) &(ShipStaticInfoR1[0]) },

    { NULL,NULL,0,0 }
};

/*=============================================================================
    Private Functions:
=============================================================================*/

/*=============================================================================
    The following functions are callback functions which fill in
    information in the static structures.  They are called by scriptSetStruct
=============================================================================*/
//bind a hierarchy element to a gun and set the hierarchy size
static void setGunBindInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    char *hString, *string, *gString;
    sdword gunIndex, nScanned, index, nBound;
    mhbinding *binding;
    lod *level;
    udword flags = 0;
    sdword frequency;

    //first string is always name of hierarchy member.
    hString = strtok(field, " ,");
    //second string is the gun mex name
    gString = strtok(NULL, " ,");
    nScanned = sscanf(gString, "Gun%d", &gunIndex);         //get gun #
    dbgAssert(nScanned == 1);                               //verify it
    dbgAssert(gunIndex >= 0 && gunIndex < info->gunStaticInfo->numGuns);

    //parse the modifiers
    while ((string = strtok(NULL, " ,")) != NULL)
    {
        if (strcasecmp(string, "Azimuth") == 0)
        {
            flags |= HBF_Azimuth;
        }
        else if (strcasecmp(string, "Declination") == 0)
        {
            flags |= HBF_Declination;
        }
        else if (strcasecmp(string, "Recoil") == 0)
        {
            flags |= HBF_Recoil;
        }
        else if (strcasecmp(string, "Frequency") == 0)
        {
            string = strtok(NULL, " ,");
            dbgAssert(string);
            nScanned = sscanf(string, "%d", &frequency);
            dbgAssert(nScanned == 1);
            dbgAssert(frequency >= 1 && frequency < HB_UpdateFrequencyMax);
            flags |= frequency;
        }
    }
    info->hierarchySize = lodHierarchySizeCompute(info->staticheader.LOD);
    nBound = 0;
    for (index = 0; index < info->staticheader.LOD->nLevels; index++)
    {                                                       //for all levels of detail
        level = &info->staticheader.LOD->level[index];
        binding = NULL;
        if ((level->flags & LM_LODType) != LT_Mesh)
        {
            continue;
        }
        while ((binding = meshBindingFindByName(binding, level->hBindings, (meshdata *)level->pData, hString)) != NULL)
        {
            binding->function = gunMatrixUpdate;            //bind all named members to this gun
            binding->userData = &info->gunStaticInfo->gunstatics[gunIndex];
            binding->userID = gunIndex;
            binding->flags = flags;
            nBound++;
        }
    }
#if MESH_VERBOSE_LEVEL >= 1
    if (nBound == 0)
    {
        dbgWarningf(DBG_Loc, "Could not bind '%s' to '%s' because object not found.", hString, gString);
    }
#endif
#if MESH_VERBOSE_LEVEL >= 2
    dbgMessagef("\nsetGunBindInfo: bound '%s' to '%s' %d times", hString, gString, nBound++);
#endif
}
static void setAnimationBindInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    char *shipName;
    char fileName[256];

    shipName = ShipTypeToStr(info->shiptype);
    sprintf(fileName, "%s%s%s", directory, shipName, MAD_Extension);//make path to file
    info->madStatic = memAlloc(sizeof(madstatic), "madBindings", NonVolatile);
    memset(info->madStatic,0,sizeof(madstatic));
    info->madStatic->header = madFileLoad(fileName);
    info->hierarchySize = lodHierarchySizeCompute(info->staticheader.LOD);
}

static void setMadStartInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    char madAnimName[256];
    udword startFlag;
    char madAnimName2[256];
    udword startFlag2;
    sscanf(field,"%d , %s , %d , %s",&startFlag,madAnimName,&startFlag2,madAnimName2);

    if(startFlag == 1)
    {
        info->madStatic->needStartPlacedAnimation = TRUE;
        info->madStatic->startPlacedAnimationIndex = madAnimIndexFindByName(info->madStatic->header, madAnimName);
    }
    else
    {
        //no need to start and then pause an animation on creation
        info->madStatic->needStartPlacedAnimation = FALSE;
    }

    if(startFlag2 == 1)
    {
        info->madStatic->needStartBuiltAnimation = TRUE;
        info->madStatic->startBuiltAnimationIndex = madAnimIndexFindByName(info->madStatic->header, madAnimName2);
    }
    else
    {
        //no need to start and then pause an animation on creation
        info->madStatic->needStartBuiltAnimation = FALSE;
    }


}
 /*
    sdword *gunOpenIndexs;                      //arrays for gunopening/closing animation indexes both damaged and undamaged.  So we can support a number of different animationsfor the same ship
    sdword *gunOpenDamagedIndexes;
    sdword *gunCloseIndexes;
    sdword *gunCloseDamagedIndexes;
 */
static void setGunAnimInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    sdword i;
    char madAnimName[256];   //maximum of 4 animations so far
    udword numAnims;
    char *token;
    char seps[] = ", ";

    token = strtok(field,seps);

    //do normal gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numGunOpenIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->gunOpenIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->gunOpenIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

    token = strtok(NULL,seps);
    //do normal gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numGunCloseIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->gunCloseIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->gunCloseIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


    token = strtok(NULL,seps);
    //do damaged gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numGunOpenDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->gunOpenDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->gunOpenDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


    token = strtok(NULL,seps);
    //do damaged gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numGunCloseDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->gunCloseDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->gunCloseDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }
}

static void setDockAnimInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    sdword i;
    char madAnimName[256];   //maximum of 4 animations so far
    udword numAnims;
    char *token;
    char seps[] = ", ";

    token = strtok(field,seps);

    //do normal gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numDockIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->PostDockIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->PostDockIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

    token = strtok(NULL,seps);
    //do normal gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numPostDockIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->DockIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
           info->madStatic->DockIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


       token = strtok(NULL,seps);
    //do damaged gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numPostDockDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->PostDockDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->PostDockDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

    token = strtok(NULL,seps);
    //do damaged gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numDockDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->DockDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->DockDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

}
void setMothershipDoorAnimInfo(char *directory,char *field,void *dataToFillIn)
{
     ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
     sdword i;
     char madAnimName[256];   //maximum of 4 animations so far
     udword numAnims;
     char *token;
     char seps[] = ", ";

     token = strtok(field,seps);

     //do normal gun opening animations
     sscanf(token,"%d",&numAnims);
     info->madStatic->numDoorOpenIndexes = numAnims;
     if(numAnims > 0)
     {
         info->madStatic->DoorOpenIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"DoorOpenMadAnimIndex",NonVolatile);
         for(i=0;i<numAnims;i++)
         {
             token = strtok(NULL,seps);
             sscanf(token,"%s",madAnimName);
             info->madStatic->DoorOpenIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
         }
     }

     token = strtok(NULL,seps);
     //do normal gun closing animations
     sscanf(token,"%d",&numAnims);
     info->madStatic->numDoorCloseIndexes = numAnims;
     if(numAnims > 0)
     {
         info->madStatic->DoorCloseIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"DoorCloseMadAnimIndex",NonVolatile);
         for(i=0;i<numAnims;i++)
         {
             token = strtok(NULL,seps);
             sscanf(token,"%s",madAnimName);
            info->madStatic->DoorCloseIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
         }
     }
}
void setSpecialCaseAnimInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    sdword i;
    char madAnimName[256];   //maximum of 4 animations so far
    udword numAnims;
    char *token;
    char seps[] = ", ";

    token = strtok(field,seps);

    //do normal gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numSpecialOpenIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->specialOpenIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"SpecialMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->specialOpenIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

    token = strtok(NULL,seps);
    //do normal gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numSpecialCloseIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->specialCloseIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"SpecialMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->specialCloseIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


    token = strtok(NULL,seps);
    //do damaged gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numSpecialOpenDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->specialOpenDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"SpecialMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->specialOpenDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


    token = strtok(NULL,seps);
    //do damaged gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numSpecialCloseDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->specialCloseDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"SpecialMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->specialCloseDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

}
static void LODScriptLoad(char *directory,char *field,void *dataToFillIn)
{
    if (strcmp(field,"DefaultShip") == 0)
    {
        *((lodinfo **)dataToFillIn) = defaultlod;
    }
    else
    {
        *((lodinfo **)dataToFillIn) = lodTableReadScript(directory, field);
    }
}
static void loadMexDataCB(char *directory,char *field,void *dataToFillIn)
{
    char fullfilename[80];

    if (strcmp(field,"DefaultShip") == 0)
    {
        *((void **)dataToFillIn) = defaultmex;
    }
    else
    {
        if (directory != NULL)
        {
            strcpy(fullfilename,directory);
            strcat(fullfilename,field);
        }
        else
        {
            strcpy(fullfilename,field);
        }

        if (fileExists(fullfilename,0))
        {
            *((void **)dataToFillIn) = mexLoad(fullfilename);
        }
        else
        {
            dbgMessagef("\n WARNING: Could not locate mex %s",fullfilename);
            *((void **)dataToFillIn) = defaultmex;
        }
    }
}

#if 0
static void setRaceCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipRace *)dataToFillIn) = StrToShipRace(field);
}
#endif

static void setClassCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipClass *)dataToFillIn) = StrToShipClass(field);
}

static void scriptSetFramesCB(char *directory,char *field,void *dataToFillIn)
{
    sscanf(field,"%d",(sdword *)dataToFillIn);
    *((sdword *)dataToFillIn) *= UNIVERSE_UPDATE_RATE;
}

void gunGetDefaultOrientationFromBindings(GunStatic *gunstatic,ShipStaticInfo *statinfo)
{
    lod *level = &statinfo->staticheader.LOD->level[0];
    //udword numHierarchys = ((meshdata *)level->pData)->nPolygonObjects;
    hmatrix concatenated, nonConcatenated;
//    mhbinding *binding;

    if (meshFindHierarchyMatrixByUserData(&concatenated, &nonConcatenated, level->hBindings, gunstatic) != FALSE)
    {
        matGetMatFromHMat(&gunstatic->defaultgunorientation,&concatenated);
        matGetMatFromHMat(&gunstatic->defaultGunOrientationNonConcat,&nonConcatenated);
        if (memcmp(&gunstatic->defaultGunOrientationNonConcat,
                   &gunstatic->defaultgunorientation, sizeof(matrix)))
        {                                                   //if these guys are different
            bitSet(gunstatic->flags, GF_MultiLevelMatrix);
        }
    }
    else
    {
        dbgMessagef("\nWARNING: could not set gunbindings for ship %d",statinfo->shiptype);
    }

/*
    binding = level->hBindings;

    // later, take into account multiple hierarchies for a single gun
    while (numHierarchys > 0)
    {
        if (binding->userData == gunstatic)
        {
            matGetMatFromHMat(&gunstatic->defaultgunorientation,&binding->object->localMatrix);
            return;
        }
        binding++;
        numHierarchys--;
    }
*/
}

void gunSetStaticGunInfoFromBindings(ShipStaticInfo *statinfo)
{
    sdword i;
    GunStaticInfo *gunStaticInfo = statinfo->gunStaticInfo;
    sdword numGuns = gunStaticInfo->numGuns;
    GunStatic *gunstatic;

    for (i=0;i<numGuns;i++)
    {
        gunstatic = &gunStaticInfo->gunstatics[i];

        switch (gunstatic->guntype)
        {
            case GUN_NewGimble:
                gunGetDefaultOrientationFromBindings(gunstatic,statinfo);
                break;

            default:
                // ignore
                break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : precalcStaticHeader
    Description : precalculates certain useful information in the static header
                  so these calculations don't have to be done every frame
    Inputs      : *staticheader
    Outputs     : *staticheader updated appropriately
    Return      :
----------------------------------------------------------------------------*/
void precalcStaticHeader(StaticHeader *staticheader)
{
    real32 avoidcollsize;

    mexSetCollisionInfo(&staticheader->staticCollInfo,staticheader->pMexData,
                        staticheader->staticCollInfo.collBoxScaleForward,
                        staticheader->staticCollInfo.collBoxScaleUp,
                        staticheader->staticCollInfo.collBoxScaleRight,
                        staticheader->staticCollInfo.collBoxOffsetX,
                        staticheader->staticCollInfo.collBoxOffsetY,
                        staticheader->staticCollInfo.collBoxOffsetZ);

    if (staticheader->staticCollInfo.approxcollmodifier != 0.0f)
    {
        staticheader->staticCollInfo.approxcollspheresize *= staticheader->staticCollInfo.approxcollmodifier;
    }

    if (staticheader->staticCollInfo.avoidcollmodifier != 0.0f)
    {
        staticheader->staticCollInfo.avoidcollspheresize *= staticheader->staticCollInfo.avoidcollmodifier;
    }

    avoidcollsize = staticheader->staticCollInfo.avoidcollspheresize;
    staticheader->staticCollInfo.avoidcollspherepad = avoidcollsize * ((avoidcollsize > AISHIP_BIGSHIP) ? AVOID_OBJ_PADDING_SCALE_BIG : AVOID_OBJ_PADDING_SCALE);

    staticheader->oneOverMass =             1.0f /      staticheader->mass;
    staticheader->oneOverMomentOfInertiaX = 1.0f /      staticheader->momentOfInertiaX;
    staticheader->oneOverMomentOfInertiaY = 1.0f /      staticheader->momentOfInertiaY;
    staticheader->oneOverMomentOfInertiaZ = 1.0f /      staticheader->momentOfInertiaZ;
}

void precalcStaticInfoHealth(StaticInfoHealth *staticinfo)
{
    if ((staticinfo->minCollDamage == 0.0f) && (staticinfo->maxCollDamage == 0.0f))
    {
        // put in defaults:
        staticinfo->minCollDamage = ((staticinfo->maxhealth) * COLLDAMAGE_DEFAULTMIN_FRACTION);
        staticinfo->maxCollDamage = ((staticinfo->maxhealth) * COLLDAMAGE_DEFAULTMAX_FRACTION);
    }
    if(staticinfo->maxhealth != 0.0f)
    {
        //precalc 1/maxhealth..duh
        staticinfo->oneOverMaxHealth = 1.0f/staticinfo->maxhealth;
    }
}

void precalcStaticInfoGuidance(StaticInfoHealthGuidance *staticinfo)
{
    sdword i;

    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        staticinfo->thruststrengthstat[i] *= staticinfo->staticheader.mass * GLOBAL_SHIP_SPEED_MODIFIER;
    }

    staticinfo->staticheader.maxvelocity *= GLOBAL_SHIP_SPEED_MODIFIER;

    staticinfo->rotstrengthstat[ROT_ABOUTXCCW] *= staticinfo->staticheader.momentOfInertiaX;
    staticinfo->rotstrengthstat[ROT_ABOUTXCW]  *= staticinfo->staticheader.momentOfInertiaX;
    staticinfo->rotstrengthstat[ROT_ABOUTYCCW] *= staticinfo->staticheader.momentOfInertiaY;
    staticinfo->rotstrengthstat[ROT_ABOUTYCW]  *= staticinfo->staticheader.momentOfInertiaY;
    staticinfo->rotstrengthstat[ROT_ABOUTZCCW] *= staticinfo->staticheader.momentOfInertiaZ;
    staticinfo->rotstrengthstat[ROT_ABOUTZCW]  *= staticinfo->staticheader.momentOfInertiaZ;
}

/*-----------------------------------------------------------------------------
    Name        : InitStatAsteroidInfo
    Description : initializes static asteroid information
    Inputs      : asteroidType
    Outputs     : asteroidStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatAsteroidInfo(AsteroidStaticInfo *asteroidStatInfo,AsteroidType asteroidtype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *asteroidtypestr;

    asteroidStatInfo->asteroidtype = asteroidtype;

    asteroidtypestr = AsteroidTypeToStr(asteroidtype);

#ifdef _WIN32
    strcpy(directory,"Resources\\Asteroids\\");
#else
    strcpy(directory,"Resources/Asteroids/");
#endif
//    strcat(directory,asteroidtypestr);
//    strcat(directory,"\\");

    strcpy(shipname,asteroidtypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (fileExists(fullshipname,0))
    {
        scriptSetStruct(directory,shipname,AsteroidStaticScriptTable,(ubyte *)asteroidStatInfo);

        precalcStaticHeader(&asteroidStatInfo->staticheader);
        precalcStaticInfoHealth((StaticInfoHealth *)asteroidStatInfo);

        // optimization for Asteroid0
        if (asteroidtype == Asteroid0) collZeroRectInfo(&asteroidStatInfo->staticheader.staticCollInfo);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatDustCloudInfo
    Description : initializes static dustcloud information
    Inputs      : dustcloudType
    Outputs     : dustcloudStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatDustCloudInfo(DustCloudStaticInfo *dustcloudStatInfo, DustCloudType dustcloudtype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *dustcloudtypestr;

    dbgAssert(dustcloudtype < NUM_DUSTCLOUDTYPES);

    dustcloudStatInfo->dustcloudtype = dustcloudtype;

    dustcloudtypestr = DustCloudTypeToStr(dustcloudtype);

#ifdef _WIN32
    strcpy(directory,"Resources\\DustClouds\\");
#else
    strcpy(directory,"Resources/DustClouds/");
#endif
//    strcat(directory,dustcloudtypestr);
//    strcat(directory,"\\");

    strcpy(shipname,dustcloudtypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (fileExists(fullshipname,0))
    {
        StaticCollInfo *coll;
        real32 r = DUSTCLOUD_RADII[(sdword)dustcloudtype] * DUSTCLOUD_RADIAL_SCALE;

        scriptSetStruct(directory,shipname,DustCloudStaticScriptTable,(ubyte *)dustcloudStatInfo);

        precalcStaticHeader(&dustcloudStatInfo->staticheader);
        precalcStaticInfoHealth((StaticInfoHealth *)dustcloudStatInfo);

        coll = &dustcloudStatInfo->staticheader.staticCollInfo;
        collZeroRectInfo(coll);

        coll->originalcollspheresize = r;
        coll->approxcollspheresize = r;
        coll->avoidcollspheresize = r;
        coll->avoidcollspherepad = r;
        coll->collspheresize = r;
        coll->collspheresizeSqr = r * r;
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatGasCloudInfo
    Description : initializes static gascloud information
    Inputs      : gascloudType
    Outputs     : gascloudStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatGasCloudInfo(GasCloudStaticInfo *gascloudStatInfo, GasCloudType gascloudtype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *gascloudtypestr;

    gascloudStatInfo->gascloudtype = gascloudtype;

    gascloudtypestr = GasCloudTypeToStr(gascloudtype);

#ifdef _WIN32
    strcpy(directory,"Resources\\GasClouds\\");
#else
    strcpy(directory,"Resources/GasClouds/");
#endif
//    strcat(directory,gascloudtypestr);
//    strcat(directory,"\\");

    strcpy(shipname,gascloudtypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (fileExists(fullshipname,0))
    {
        scriptSetStruct(directory,shipname,GasCloudStaticScriptTable,(ubyte *)gascloudStatInfo);

        precalcStaticHeader(&gascloudStatInfo->staticheader);
        precalcStaticInfoHealth((StaticInfoHealth *)gascloudStatInfo);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatNebulaInfo
    Description : initializes static nebula information
    Inputs      : nebulaType
    Outputs     : nebulaStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatNebulaInfo(NebulaStaticInfo* nebulaStatInfo, NebulaType nebulatype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char* nebulatypestr;

    dbgAssert(nebulatype < NUM_NEBULATYPES);

    nebulaStatInfo->nebulatype = nebulatype;

    nebulatypestr = NebulaTypeToStr(nebulatype);

#ifdef _WIN32
    strcpy(directory,"Resources\\Nebulae\\");
#else
    strcpy(directory,"Resources/Nebulae/");
#endif
//    strcat(directory,nebulatypestr);
//    strcat(directory,"\\");

    strcpy(shipname,nebulatypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (fileExists(fullshipname,0))
    {
        StaticCollInfo *coll;

        scriptSetStruct(directory, shipname, NebulaStaticScriptTable, (ubyte*)nebulaStatInfo);

        precalcStaticHeader(&nebulaStatInfo->staticheader);
        precalcStaticInfoHealth((StaticInfoHealth*)nebulaStatInfo);

        coll = &nebulaStatInfo->staticheader.staticCollInfo;
        collZeroRectInfo(coll);

        coll->originalcollspheresize = NEB_RADIUS;
        coll->approxcollspheresize = NEB_RADIUS;
        coll->avoidcollspheresize = NEB_RADIUS;
        coll->avoidcollspherepad = NEB_RADIUS;
        coll->collspheresize = NEB_RADIUS;
        coll->collspheresizeSqr = NEB_RADIUS * NEB_RADIUS;
    }
}

void InitStatHyperspaceGateInfo(DerelictStaticInfo *derelictStatInfo)
{
    StaticCollInfo *sci;
    real32 r;

    memset(derelictStatInfo, 0, sizeof(DerelictStaticInfo));

    derelictStatInfo->maxhealth = HYPERSPACEGATE_HEALTH;
    derelictStatInfo->oneOverMaxHealth = (1.0f / derelictStatInfo->maxhealth);

    derelictStatInfo->renderlistLimitSqr = 5.0e8f;

    sci = &derelictStatInfo->staticheader.staticCollInfo;

    sci->uplength = HYPERSPACEGATE_HEIGHT;
    sci->rightlength = HYPERSPACEGATE_WIDTH;
    sci->forwardlength = 50.0f;     // give it a small thickness for collision's sake

    vecSet(sci->collrectoffset,-HYPERSPACEGATE_HEIGHT/2,-HYPERSPACEGATE_WIDTH/2,-50.0/2);

    r = max(HYPERSPACEGATE_HEIGHT,HYPERSPACEGATE_WIDTH);

    sci->approxcollspheresize = r;
    sci->avoidcollspheresize = r;
    sci->avoidcollspherepad = r;
    sci->avoidcollmodifier = 1.0f;
    sci->approxcollmodifier = 1.0f;

    sci->originalcollspheresize = r;
    sci->collspheresize = r;
    sci->collspheresizeSqr = r*r;

    vecSet(sci->collsphereoffset,0.0f,0.0f,0.0f);
}

/*-----------------------------------------------------------------------------
    Name        : InitStatDerelictInfo
    Description : initializes static derelict information
    Inputs      : derelicttype
    Outputs     : derelictStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatDerelictInfo(DerelictStaticInfo *derelictStatInfo, DerelictType derelicttype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *derelicttypestr;

    if (derelicttype == HyperspaceGate)
    {
        InitStatHyperspaceGateInfo(derelictStatInfo);
        return;
    }

    derelicttypestr = DerelictTypeToStr(derelicttype);

#ifdef _WIN32
    strcpy(directory,"Derelicts\\");
#else
    strcpy(directory,"Derelicts/");
#endif
//    strcat(directory,derelicttypestr);
//    strcat(directory,"\\");

    strcpy(shipname,derelicttypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    derelictStatInfo->minimumZoomDistance = 0.0f;
    derelictStatInfo->renderlistFade = 0.0f;
    derelictStatInfo->renderlistLimitSqr = 0.0f;

    if (fileExists(fullshipname,0))
    {
        InitStatDerelictInfoByPath(derelictStatInfo, derelicttype, directory,shipname);
    }
    scriptSetNAVLightStatics(directory, shipname, (ShipStaticInfo *)derelictStatInfo);
    if (derelictStatInfo->navlightStaticInfo != NULL)
    {
        mexGetNAVLightStaticInfo(derelictStatInfo->navlightStaticInfo,derelictStatInfo->staticheader.pMexData);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatDerelictInfoByPath
    Description : Init the static info for a derelict based on a type AND a path.
                  Mostly used for creating generic objects for NIS's.
    Inputs      : derelictStatInfo - where to load the data to.
                  derelictType - type of derelict.  If only used in NIS, this
                    field is not used.
                  directory - what directory the .shp file is in.
                  shipFile - what .shp file to load in.
    Outputs     : reads in the representative .shp file into derelictStatInfo.
    Return      : void
----------------------------------------------------------------------------*/
void InitStatDerelictInfoByPath(DerelictStaticInfo *derelictStatInfo, DerelictType derelicttype, char *directory, char *shipFile)
{
    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];

    memcpy(teamColor, derelictStatInfo->teamColor, sizeof(teamColor));//save the default team colors
    derelictStatInfo->derelicttype = derelicttype;
    memset(derelictStatInfo, 0, sizeof(DerelictStaticInfo));
    memcpy(derelictStatInfo->teamColor, teamColor, sizeof(teamColor));
    derelictStatInfo->staticheader.rightOfWay = 1;      // default to derelicts having a right of way same as frigates
    derelictStatInfo->salvageable = FALSE;              //default salvageability to FALSE!
    scriptSetStruct(directory,shipFile,DerelictStaticScriptTable,(ubyte *)derelictStatInfo);

    precalcStaticHeader(&derelictStatInfo->staticheader);
    precalcStaticInfoHealth((StaticInfoHealth *)derelictStatInfo);
    precalcStaticInfoGuidance((StaticInfoHealthGuidance *)derelictStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : InitStatMissileInfo
    Description : initializes static missile information
    Inputs      : race
    Outputs     : missileStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatMissileInfo(MissileStaticInfo *missileStatInfo,ShipRace race)
{
    char directory[80];
    char fullshipname[100];
    char *shipname = "Missile.shp";

    strcpy(directory,ShipRaceToStr(race));
//    strcat(directory,"\\Missile\\");
#ifdef _WIN32
    strcat(directory, "\\");
#else
    strcat(directory, "/");
#endif

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (!fileExists(fullshipname,0))
    {
        dbgMessagef("\nWarning: Could not find Missile for %s",ShipRaceToStr(race));
        return;
    }

    missileStatInfo->turnspeedstat[TURN_YAW]   = 1.0f;
    missileStatInfo->turnspeedstat[TURN_PITCH] = 1.0f;
    missileStatInfo->turnspeedstat[TURN_ROLL]  = 1.0f;

    missileStatInfo->staticheader.rightOfWay = -1;

    scriptSetStruct(directory,shipname,MissileStaticScriptTable,(ubyte *)missileStatInfo);

    precalcStaticHeader(&missileStatInfo->staticheader);
    precalcStaticInfoHealth((StaticInfoHealth *)missileStatInfo);
    precalcStaticInfoGuidance((StaticInfoHealthGuidance *)missileStatInfo);

    //missile trail stuff
    missileStatInfo->trailWidth = -1.0f;
    missileStatInfo->trailHeight = -1.0f;
    missileStatInfo->trailStatic = trailStaticInfoParse(directory, shipname);
}

/*-----------------------------------------------------------------------------
    Name        : InitStatMineInfo
    Description : initializes static missile-mine information
    Inputs      : race
    Outputs     : missileStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatMineInfo(MissileStaticInfo *mineStatInfo,ShipRace race)
{
    char directory[80];
    char fullshipname[100];
    char *shipname = "Mine.shp";

    strcpy(directory,ShipRaceToStr(race));
//    strcat(directory,"\\Mine\\");
#ifdef _WIN32
    strcat(directory, "\\");
#else
    strcat(directory, "/");
#endif

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (!fileExists(fullshipname,0))
    {
        dbgMessagef("\nWarning: Could not find Mine for %s",ShipRaceToStr(race));
        return;
    }

    mineStatInfo->turnspeedstat[TURN_YAW]   = 1.0f;
    mineStatInfo->turnspeedstat[TURN_PITCH] = 1.0f;
    mineStatInfo->turnspeedstat[TURN_ROLL]  = 1.0f;

    mineStatInfo->staticheader.rightOfWay = -1;

    scriptSetStruct(directory,shipname,MineStaticScriptTable,(ubyte *)mineStatInfo);

    mineStatInfo->mineRange = fsqrt(mineStatInfo->MINE_RANGESQR);
    mineStatInfo->mineRangeForced = fsqrt(mineStatInfo->MINE_RANGESQR_FORCED);

    precalcStaticHeader(&mineStatInfo->staticheader);
    precalcStaticInfoHealth((StaticInfoHealth *)mineStatInfo);
    precalcStaticInfoGuidance((StaticInfoHealthGuidance *)mineStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : isStrikeCraft
    Description : returns TRUE if ship is a strikecraft
    Inputs      : shiptype
    Outputs     :
    Return      : returns TRUE if ship is a strikecraft
----------------------------------------------------------------------------*/
bool isStrikeCraft(ShipType shiptype)
{
    switch (shiptype)
    {
        case AttackBomber:
        case CloakedFighter:
        case DefenseFighter:
        case HeavyInterceptor:
        case LightInterceptor:
        case HeavyDefender:
        case LightCorvette:
        case HeavyCorvette:
        case MultiGunCorvette:
        case MinelayerCorvette:
        case SalCapCorvette:
        case RepairCorvette:
        case P1Fighter:
        case P1MissileCorvette:
        case P1StandardCorvette:
        case P2Swarmer:
        case P2AdvanceSwarmer:
            return TRUE;

        default:
            return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatShipInfo
    Description : Initializes static ship information for all ship types.
    Inputs      : type, race
    Outputs     : statinfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatShipInfo(ShipStaticInfo *statinfo,ShipType type,ShipRace race)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *shiptypestr;
    sdword i;
    bool stubbedOut;

    statinfo->shiptype = type;
    statinfo->shiprace = race;

    statinfo->custshipheader = *(shipHeaders[type]);
    dbgAssert((statinfo->custshipheader.shiptype == (ShipType)-1) || (type == statinfo->custshipheader.shiptype));

    shiptypestr = ShipTypeToStr(type);

    strcpy(directory,ShipRaceToStr(race));
//    strcat(directory,"\\");
//    strcat(directory,shiptypestr);
#ifdef _WIN32
    strcat(directory,"\\");
#else
    strcat(directory,"/");
#endif

    strcpy(shipname,shiptypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (!fileExists(fullshipname,0) || universeForceDefaultShip)
    {
        //we now disable this ship if we can't find it...
        SetInfoNeededForShipAndRelatedStaticInfo(type,race,FALSE);
        rmEnableShip(race, type, FALSE);
        return;
/*

#ifndef HW_Release
        dbgMessagef("\nStubbing out %s\\%s", ShipRaceToStr(race), ShipTypeToStr(type));
#endif
        strcpy(directory,"DefaultShip\\");
        strcpy(shipname,"DefaultShip.shp");

        strcpy(fullshipname,directory);
        strcat(fullshipname,shipname);
        stubbedOut = TRUE;
*/
    }
    else
    {
        stubbedOut = FALSE;
    }

#if SO_CLOOGE_SCALE
    statinfo->scaleFactor = 1.0f;
#endif

    statinfo->turnspeedstat[TURN_YAW]   = 1.0f;
    statinfo->turnspeedstat[TURN_PITCH] = 1.0f;
    statinfo->turnspeedstat[TURN_ROLL]  = 1.0f;

    statinfo->minimumZoomDistance = 0.0f;

    statinfo->renderlistFade = 0.0f;
    statinfo->renderlistLimitSqr = 0.0f;

    statinfo->dockLightNear = 0.0f;
    statinfo->dockLightFar  = 0.1f;
    statinfo->dockLightColor = colRGB(120, 194, 255);   //default blue glow

    statinfo->hyperspaceColor = colRGB(0,0,0);  //default is to use the effect's default

    statinfo->tacticalSelectionScale = 1.0f;  // This is used to scale the R1 probes tactiacl overlay size, and the defenders selection box size

    if (isStrikeCraft(type))
        statinfo->formationPaddingModifier = STRIKECRAFT_PADDING_MODIFIER;          // default formation padding modifier
    else
        statinfo->formationPaddingModifier = 1.0f;          // default formation padding modifier

    //default to no explosions at all
//    statinfo->smallExplosion = statinfo->bigExplosion = statinfo->specialExplosion = NULL;
//    statinfo->bigExplosionFactor = 1.0f;

    //engine trail defaults
    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        statinfo->trailStyle[i] = 1;
        statinfo->trailWidth[i] = -1.0f;
        statinfo->trailHeight[i] = -1.0f;
        statinfo->trailAngle[i] = -1.0f;
        statinfo->trailRibbonAdjust[i] = -1.0f;
        statinfo->trailLength[i] = -1.0f;
        statinfo->trailScaleCap[i] = -1.0f;
        statinfo->trailSpriteRadius[i] = -1.0f;
        statinfo->trailSpriteOffset[i] = 0.0f;
    }
    statinfo->staticheader.staticCollInfo.preciseSelection = 0xff;//default is no precise selection

    // collSideModifiers reset
    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        statinfo->collSideModifiers[i] = 0.0f;
    }

    statinfo->staticheader.rightOfWay = rightOfWays[type];      // defaults for right of way

    scriptSetStruct(directory,shipname,ShipStaticScriptTable,(ubyte *)statinfo);

    if ((statinfo->repairTime) && (statinfo->repairCombatTime == 0.0f))
    {
        statinfo->repairCombatTime = statinfo->repairTime;
    }

    if ((statinfo->repairDamage) && (statinfo->repairCombatDamage == 0.0f))
    {
        statinfo->repairCombatDamage = statinfo->repairDamage;
    }

    if (statinfo->collSideModifiers[0] == 0.0f)
    {
        // collSideModifiers haven't been explicitly set, so set them:
        if (isCapitalShipStatic(statinfo))
        {
            for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
            {
                statinfo->collSideModifiers[i] = capitalSideModifiers[i];
            }
        }
        else
        {
            for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
            {
                statinfo->collSideModifiers[i] = 1.0f;
            }
        }
    }
/*
#ifndef HW_Release
    if ((cheapShips) && (!multiPlayerGame))
    {
        statinfo->buildTime = 1;
        statinfo->buildCost = 1;
    }
#endif
*/
    if (statinfo->blastRadiusShockWave == 0.0f)
    {
        statinfo->blastRadiusShockWave = BLAST_RADIUS_MULTIPLE_DEFAULT;
    }



    if (statinfo->passiveRetaliateZone == 0.0f)
    {
        statinfo->passiveRetaliateZone = RETALIATE_ZONE;
    }
    else if (statinfo->passiveRetaliateZone < 0.0f)
    {
        statinfo->passiveRetaliateZone = 0.0f;       // turn of retaliation zone feature if -ve
    }

    if (statinfo->attackfuelburnrate == 0.0f)
    {
        statinfo->attackfuelburnrate = statinfo->fuelburnrate;
    }

    statinfo->canReceiveSomething = (bool8)(statinfo->canReceiveResources | statinfo->canReceiveShips | statinfo->canReceiveShipsPermanently | statinfo->canReceiveShipsForRetire);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        statinfo->trailStatic[i] = trailStaticInfoParse(directory, shipname);
    }
    if (mexGetChunk(statinfo->staticheader.pMexData, "Eng", "Eng1") != NULL)
    {
        statinfo->multipleEngines = (udword)TRUE;
    }
    else
    {
        statinfo->multipleEngines = FALSE;
    }

    statinfo->lowfuelpoint *= statinfo->maxfuel;

    precalcStaticHeader(&statinfo->staticheader);
#if 1  // comment out when MiningBase properly moved hack,  fix later
    if (type == MiningBase)
    {
        StaticCollInfo *staticCollInfo = &statinfo->staticheader.staticCollInfo;
        staticCollInfo->collsphereoffset.x += MINING_BASE_VERTICAL_DIST_HACK;
        staticCollInfo->collrectoffset.x += MINING_BASE_VERTICAL_DIST_HACK;
    }
#endif
    precalcStaticInfoHealth((StaticInfoHealth *)statinfo);
    precalcStaticInfoGuidance((StaticInfoHealthGuidance *)statinfo);

    if (statinfo->dockShipRange != 0.0f)
    {
        statinfo->dockShipRange += statinfo->staticheader.staticCollInfo.approxcollspheresize;
        statinfo->dockShipRange *= statinfo->dockShipRange;
    }

    aishipPrecalcInfo(statinfo);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        mexSetEngineNozzle(statinfo->engineNozzleOffset + i, statinfo->staticheader.pMexData, i);
    }

    if (stubbedOut == FALSE)
    {
        scriptSetDockStatics(directory,shipname,statinfo);
        scriptSetDockOverideStatics(directory,shipname,statinfo);
    }

    if (statinfo->dockStaticInfo != NULL)
    {
        mexGetDockStaticInfo(statinfo->dockStaticInfo,statinfo->staticheader.pMexData);
    }

    if (stubbedOut == FALSE)
    {
        dockInitializeCustomFunctions(statinfo,type,race);
    }

    if (ShipCanHarvest(statinfo))
    {
        // find resource collection nozzle
        statinfo->resNozzleStatic = mexGetResNozzleStatic(statinfo->staticheader.pMexData);
    }

    if(statinfo->repairBeamCapable)
    {
        //find repair beam nozzle
        statinfo->repairNozzleStatic = mexGetRepairNozzleStatic(statinfo->staticheader.pMexData);
    }

    if(type == SalCapCorvette)
    {
        //find tractor beam emmitter
        statinfo->tractorEmitterStatic = mexGetTractorBeamStatic(statinfo->staticheader.pMexData);
    }

    scriptSetNAVLightStatics(directory, shipname, statinfo);
    if (statinfo->navlightStaticInfo != NULL)
    {
        mexGetNAVLightStaticInfo(statinfo->navlightStaticInfo,statinfo->staticheader.pMexData);
    }

    scriptSetGunStatics(directory,shipname,statinfo);
    if (statinfo->gunStaticInfo != NULL)
    {
        GunStaticInfo *gunstaticinfo = statinfo->gunStaticInfo;
        for (i=0;i<gunstaticinfo->numGuns;i++)
        {
            gunstaticinfo->gunstatics[i].gunindex = i;
        }
        mexGetGunStaticInfo(gunstaticinfo,statinfo->staticheader.pMexData);
    }

    if (statinfo->bulletRange[0] != 0.0f)
    {
        for(i=0;i<NUM_TACTICS_TYPES;i++)
        {
            statinfo->bulletRangeSquared[i] = statinfo->bulletRange[i] * statinfo->bulletRange[i];
        }
    }


    //load in the hierarchy binding information
    scriptSetStruct(directory,shipname,HierarchyBindingTable,(ubyte *)statinfo);

    //load in More Mesh Animation info
    //must be AFTER we've loaded in all other MAD stuff, otherwise
    //I'll get MAD!
    if(statinfo->madStatic != NULL)
    {
        //default these to 0
        statinfo->madStatic->needStartPlacedAnimation = 0;
        statinfo->madStatic->needStartBuiltAnimation = 0;
        statinfo->madStatic->numGunOpenIndexes=0;
        statinfo->madStatic->numGunOpenDamagedIndexes=0;
        statinfo->madStatic->numGunCloseIndexes=0;
        statinfo->madStatic->numGunCloseDamagedIndexes=0;
        statinfo->madStatic->numDockIndexes=0;                   //number of below indices
        statinfo->madStatic->numDockDamagedIndexes=0;                   //number of below indices
        statinfo->madStatic->numPostDockIndexes=0;                   //number of below indices
        statinfo->madStatic->numPostDockDamagedIndexes=0;                   //number of below indices
        statinfo->madStatic->numDoorOpenIndexes=0;
        statinfo->madStatic->numDoorCloseIndexes=0;
        statinfo->madStatic->numSpecialOpenIndexes=0;
        statinfo->madStatic->numSpecialCloseIndexes=0;
        statinfo->madStatic->numSpecialOpenDamagedIndexes=0;
        statinfo->madStatic->numSpecialCloseDamagedIndexes=0;

    }

    scriptSetStruct(directory,shipname,MadMaxMadInfoLoad,(ubyte *)statinfo);

    if (statinfo->gunStaticInfo != NULL && stubbedOut == FALSE)
    {
        gunSetStaticGunInfoFromBindings(statinfo);

    }

    if (statinfo->custshipheader.CustShipStaticInit != NULL)
    {
        statinfo->custshipheader.CustShipStaticInit(directory,shipname,statinfo);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatShipInfoPost
    Description : Initializes static ship information for all ship types. Post operation
                  uses information from previous pass to complete the initialization.
    Inputs      : type, race
    Outputs     : statinfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatShipInfoPost(ShipStaticInfo *statinfo)
{
    if (GLOBAL_SHIP_HEALTH_MODIFIER != 1.0f)
    {
        statinfo->maxhealth = ((statinfo->maxhealth) * GLOBAL_SHIP_HEALTH_MODIFIER);
        if (statinfo->maxhealth == 0.0f)
        {
            statinfo->maxhealth = 1.0f;
        }
        statinfo->oneOverMaxHealth = 1.0f/statinfo->maxhealth;
    }

    if (statinfo->custshipheader.CustShipStaticInitPost != NULL)
    {
        statinfo->custshipheader.CustShipStaticInitPost(statinfo);
    }
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatInfo
    Description : Close base static info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatInfo(StaticInfo *statinfo)
{
    if ((statinfo->staticheader.pMexData != defaultmex) && (statinfo->staticheader.pMexData != NULL))
    {
        mexFree(statinfo->staticheader.pMexData);
        statinfo->staticheader.pMexData = NULL;
    }

    if ((statinfo->staticheader.LOD != defaultlod) && (statinfo->staticheader.LOD != NULL))
    {
        lodFree(statinfo->staticheader.LOD);
        statinfo->staticheader.LOD = NULL;
    }
    //statinfo->staticheader.infoLoaded = statinfo->staticheader.infoNeeded = FALSE;
    statinfo->staticheader.infoFlags = 0;
    memset(statinfo, 0, sizeof(StaticInfo));
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatShipInfo
    Description : Closes static ship info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatShipInfo(ShipStaticInfo *statinfo)
{
    sdword i;

    CloseStatInfo((StaticInfo *)statinfo);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (statinfo->trailStatic[i] != NULL)
        {
            trailStaticDelete(statinfo->trailStatic[i]);
            statinfo->trailStatic[i] = NULL;
        }
    }

    if (statinfo->madStatic != NULL)
    {
        if(statinfo->madStatic->gunOpenIndexes != NULL)
        {
            memFree(statinfo->madStatic->gunOpenIndexes);
        }
        if(statinfo->madStatic->gunCloseIndexes != NULL)
        {
            memFree(statinfo->madStatic->gunCloseIndexes);
        }
        if(statinfo->madStatic->gunOpenDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->gunOpenDamagedIndexes);
        }
        if(statinfo->madStatic->gunCloseDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->gunCloseDamagedIndexes);
        }
        //dock anims
        if(statinfo->madStatic->PostDockIndexes != NULL)
        {
            memFree(statinfo->madStatic->PostDockIndexes);
        }
        if(statinfo->madStatic->DockIndexes != NULL)
        {
            memFree(statinfo->madStatic->DockIndexes);
        }
        if(statinfo->madStatic->PostDockDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->PostDockDamagedIndexes);
        }
        if(statinfo->madStatic->DockDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->DockDamagedIndexes);
        }
        //mothership door anims
        if(statinfo->madStatic->DoorOpenIndexes != NULL)
        {
            memFree(statinfo->madStatic->DoorOpenIndexes);
        }
        if(statinfo->madStatic->DoorCloseIndexes != NULL)
        {
            memFree(statinfo->madStatic->DoorCloseIndexes);
        }
        //special case
        if(statinfo->madStatic->specialOpenIndexes != NULL)
        {
            memFree(statinfo->madStatic->specialOpenIndexes);
        }
        if(statinfo->madStatic->specialCloseIndexes != NULL)
        {
            memFree(statinfo->madStatic->specialCloseIndexes);
        }
        if(statinfo->madStatic->specialOpenDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->specialOpenDamagedIndexes);
        }
        if(statinfo->madStatic->specialCloseDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->specialCloseDamagedIndexes);
        }


        madHeaderDelete(statinfo->madStatic->header);
        //... other mesh binding freeing stuffs
        memFree(statinfo->madStatic);

    }
    if(statinfo->dockOverideInfo != NULL)
    {
        memFree(statinfo->dockOverideInfo);
    }
    if (statinfo->resNozzleStatic != NULL)
    {
        memFree(statinfo->resNozzleStatic);
        statinfo->resNozzleStatic = NULL;
    }
    if(statinfo->repairNozzleStatic != NULL)
    {
        memFree(statinfo->repairNozzleStatic);
        statinfo->repairNozzleStatic = NULL;
    }

    if (statinfo->salvageStaticInfo != NULL)
    {
        memFree(statinfo->salvageStaticInfo);
        statinfo->salvageStaticInfo = NULL;
    }


    if (statinfo->tractorEmitterStatic != NULL)
    {
        memFree(statinfo->tractorEmitterStatic);
        statinfo->tractorEmitterStatic = NULL;
    }


    if (statinfo->dockStaticInfo != NULL)
    {
        memFree(statinfo->dockStaticInfo);
        statinfo->dockStaticInfo = NULL;
    }

    if (statinfo->navlightStaticInfo != NULL)
    {
        navLightStaticInfoDelete(statinfo->navlightStaticInfo);
        statinfo->navlightStaticInfo = NULL;
    }

    if (statinfo->gunStaticInfo != NULL)
    {
        memFree(statinfo->gunStaticInfo);
        statinfo->gunStaticInfo = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatAsteroidInfo
    Description : Closes static asteroid info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatAsteroidInfo(AsteroidStaticInfo *asteroidStatInfo)
{
    CloseStatInfo((StaticInfo *)asteroidStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatDustCloudInfo
    Description : Closes static dustcloud info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatDustCloudInfo(DustCloudStaticInfo *dustcloudStatInfo)
{
    CloseStatInfo((StaticInfo *)dustcloudStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatGasCloudInfo
    Description : Closes static gascloud info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatGasCloudInfo(GasCloudStaticInfo *gascloudStatInfo)
{
    CloseStatInfo((StaticInfo *)gascloudStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatNebulaInfo
    Description : Closes static nebula info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatNebulaInfo(NebulaStaticInfo *nebulaStatInfo)
{
//    CloseStatInfo((StaticInfo *)nebulaStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatDerelictInfo
    Description : Closes static derelict info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatDerelictInfo(DerelictStaticInfo *derelictStatInfo)
{
    if (derelictStatInfo->salvageStaticInfo != NULL)
    {
        memFree(derelictStatInfo->salvageStaticInfo);
        derelictStatInfo->salvageStaticInfo = NULL;
    }
    CloseStatInfo((StaticInfo *)derelictStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatMissileInfo
    Description : Closes static missile info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatMissileInfo(MissileStaticInfo *missileStatInfo)
{
    CloseStatInfo((StaticInfo *)missileStatInfo);
}
/*-----------------------------------------------------------------------------
    Name        : CloseStatMineInfo
    Description : Closes static mine info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatMineInfo(MissileStaticInfo *mineStatInfo)
{
    CloseStatInfo((StaticInfo *)mineStatInfo);
}

void CloseStatEffectInfo(etgeffectstatic *effectStatInfo)
{
//    CloseStatInfo((EffectStaticInfo *)effectStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : universeInfoNeededSetCB
    Description : Static info scan callback to flag a static info as needed or
                    not needed.
    Inputs      : info - static info to look at
                  objType - type of object to work with
    Outputs     : modifies the IF_InfoNeeded bit of infoFlags to bNeededParam
    Return      : void
----------------------------------------------------------------------------*/
static sdword bNeededParam;
void universeInfoNeededSetCB(StaticInfo *info, ObjType objType)
{
    bitSetTo(info->staticheader.infoFlags, IF_InfoNeeded, bNeededParam);
}

/*=============================================================================
    Public Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : universeFlagRaceNeeded
    Description : Marks an entire race of staticinfo's as needed or not
    Inputs      : shiprace - what race to mark
                  bNeeded - TRUE if we need this static info loaded this level
    Outputs     :
    Return      : void
    Note        : Missiles/mines will be turned on/off with the associated ships.
----------------------------------------------------------------------------*/
void universeFlagRaceNeeded(ShipRace shiprace, bool8 bNeeded)
{
    bNeededParam = bNeeded;
    universeRaceStaticsScan(shiprace, universeInfoNeededSetCB, universeInfoNeededSetCB);
}

/*-----------------------------------------------------------------------------
    Name        : universeDefaultTeamColors
    Description : Sets the defualts for what colors the ships can be in -
                    whatever players are that race.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void universeDefaultTeamColors(void)
{
    bool8 racePlayerTable[NUM_RACES][MAX_MULTIPLAYER_PLAYERS];
    sdword index;
    ShipRace shiprace;
    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    DerelictType derelicttype;
    ShipStaticInfo *shipstaticinfo;
    MissileStaticInfo *missileInfo;

    //make a boolean table of color schemes by race
    memset(racePlayerTable, 0, sizeof(racePlayerTable));
    for (index = 0; index < numPlayers; index++)
    {
        racePlayerTable[universe.players[index].race][index] = TRUE;
    }
    //!!! somewhat oversimple assignment of default color schemes for the non-player races
    racePlayerTable[Traders][7] = TRUE;
    racePlayerTable[P3][6] = TRUE;
    racePlayerTable[P2][5] = TRUE;
    racePlayerTable[P1][4] = TRUE;
    //now plug that default table into all the ship static infos
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
            memcpy(shipstaticinfo->teamColor, racePlayerTable[shiprace], sizeof(shipstaticinfo->teamColor));
        }
        missileInfo = &missileStaticInfos[shiprace];
        memcpy(missileInfo->teamColor, racePlayerTable[shiprace], sizeof(missileInfo->teamColor));
        if (shiprace <= R2)
        {
            missileInfo = &mineStaticInfos[shiprace];
            memcpy(missileInfo->teamColor, racePlayerTable[shiprace], sizeof(missileInfo->teamColor));
        }
    }
    //plug default team colors into derelicts (last team color scheme, like traders)
    for (derelicttype = 0; derelicttype < NUM_DERELICTTYPES; derelicttype++)
    {
        memcpy(derelictStaticInfos[derelicttype].teamColor, racePlayerTable[Traders], sizeof(racePlayerTable[Traders]));
    }
}

/*-----------------------------------------------------------------------------
    Name        : universeFlagEverythingNeeded
    Description : Flags all static infos to be reloaded during the next level
    Inputs      : void
    Outputs     : sets the infoNeeded flag true for all static infos
    Return      : void
----------------------------------------------------------------------------*/
void universeFlagEverythingNeeded(void)
{
    bNeededParam = TRUE;
    universeAllStaticsScan(universeInfoNeededSetCB, universeInfoNeededSetCB, universeInfoNeededSetCB, universeInfoNeededSetCB);
}

/*-----------------------------------------------------------------------------
    Name        : universeRaceStaticsScan
    Description : Scans all static infos needed for a race and calls appropriate
                    callbacks.
    Inputs      : shiprace - what race to scan
                  forShips - called for ship static infos
                  forMisc - called for other statics, like mines and missiles
    Outputs     :
    Return      : void
    Note        : Missiles/mines will be turned on/off with the associated ships.
----------------------------------------------------------------------------*/
void universeRaceStaticsScan(ShipRace shiprace, univstatcallback forShips, univstatcallback forMisc)
{
    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    ShipStaticInfo *shipstaticinfo;

    firstshiptype = FirstShipTypeOfRace[shiprace];
    lastshiptype = LastShipTypeOfRace[shiprace];
    for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
    {
        shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
        if (forShips)
        {
            forShips((StaticInfo *)shipstaticinfo, OBJ_ShipType);
        }
        if (forMisc)
        {
            if (shiptype == MissileDestroyer)
            {
                forMisc((StaticInfo *)&missileStaticInfos[shiprace], OBJ_MissileType);
            }
            else if (shiptype == P1MissileCorvette && shiprace == P1)
            {
                forMisc((StaticInfo *)&missileStaticInfos[P1], OBJ_MissileType);
            }
            else if (shiprace < 2 && shiptype == MinelayerCorvette)
            {
                forMisc((StaticInfo *)&mineStaticInfos[shiprace], OBJ_MissileType);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : universeAllStaticsScan
    Description : Scans through the static info structures for all spaceobj
                    types and calls a callback with that static info.
    Inputs      : forShips - called for ship static infos
                  forDerelicts, forResources, forMisc - same, but for derelicts,
                  resources and whatnot.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void universeAllStaticsScan(univstatcallback forShips, univstatcallback forDerelicts, univstatcallback forResources, univstatcallback forMisc)
{
    ShipRace shiprace;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        universeRaceStaticsScan(shiprace, forShips, forMisc);
    }
    if (forResources != NULL)
    {
        for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
        {
            forResources((StaticInfo *)&asteroidStaticInfos[asteroidtype], OBJ_AsteroidType);
        }
        for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
        {
            forResources((StaticInfo *)&dustcloudStaticInfos[dustcloudtype], OBJ_DustType);
        }
        for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
        {
            forResources((StaticInfo *)&gascloudStaticInfos[gascloudtype], OBJ_GasType);
        }
        for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
        {
            forResources((StaticInfo *)&nebulaStaticInfos[nebulatype], OBJ_NebulaType);
        }
    }
    if (forDerelicts != NULL)
    {
        for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
        {
            forDerelicts((StaticInfo *)&derelictStaticInfos[derelicttype], OBJ_DerelictType);
        }
    }
    if (forMisc != NULL)
    {
        for (shiprace=0;shiprace<NUM_RACES;shiprace++)
        {
            forMisc((StaticInfo *)&missileStaticInfos[shiprace], OBJ_MissileType);
        }
/*
    for (shiprace=0;shiprace<2;shiprace++)
    {
        bitSet(mineStaticInfos[shiprace].staticheader.infoFlags, IF_InfoNeeded);
    }
*/
    }
}

/*-----------------------------------------------------------------------------
    Name        : universeFlagEverythingNeeded
    Description : Flags all static infos to be reloaded during the next level
    Inputs      : void
    Outputs     : sets the infoNeeded flag FALSE for all static infos
    Return      : void
----------------------------------------------------------------------------*/
void universeFlagNothingNeeded(void)
{
    bNeededParam = FALSE;
    universeAllStaticsScan(universeInfoNeededSetCB, universeInfoNeededSetCB, universeInfoNeededSetCB, universeInfoNeededSetCB);
}

ubyte bitpackTeamColor(bool8 teamColor[])
{
    sdword i;
    ubyte bitmask = 0;

    for (i=0;i<8;i++)
    {
        bitmask <<= 1;
        if (teamColor[i])
        {
            bitmask |= 1;
        }
    }

    return bitmask;
}

void unbitpackTeamColor(bool8 teamColor[],ubyte pack)
{
    sdword i;

    for (i=0;i<8;i++)
    {
        if (pack & 128)
        {
            teamColor[i] = TRUE;
        }
        else
        {
            teamColor[i] = FALSE;
        }
        pack <<= 1;
    }
}

#define INFO_NEEDED_FLAGS_DELIMITER         0xf1a5deed

void universeSaveEverythingNeeded(void)
{
    ShipRace shiprace;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;

    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    ShipStaticInfo *shipstatic;

    ubyte data[4];

    SaveInfoNumber(INFO_NEEDED_FLAGS_DELIMITER);

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            shipstatic = GetShipStaticInfo(shiptype,shiprace);
            data[0] = bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded);      // anything loaded is needed
            data[1] = bitpackTeamColor(shipstatic->teamColor);
            data[2] = 0;
            data[3] = 0;
            SaveInfoNumber(TreatAsUdword(data[0]));
        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        SaveInfoNumber(bitTest(asteroidStaticInfos[asteroidtype].staticheader.infoFlags, IF_InfoLoaded));
    }
    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        SaveInfoNumber(bitTest(dustcloudStaticInfos[dustcloudtype].staticheader.infoFlags, IF_InfoLoaded));
    }
    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        SaveInfoNumber(bitTest(gascloudStaticInfos[gascloudtype].staticheader.infoFlags, IF_InfoLoaded));
    }
    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        SaveInfoNumber(bitTest(nebulaStaticInfos[nebulatype].staticheader.infoFlags, IF_InfoLoaded));
    }
    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        DerelictStaticInfo *dstaticinfo = &derelictStaticInfos[derelicttype];
        data[0] = bitTest(dstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
        data[1] = bitpackTeamColor(dstaticinfo->teamColor);
        data[2] = 0;
        data[3] = 0;
        SaveInfoNumber(TreatAsUdword(data[0]));
    }
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        MissileStaticInfo *misstaticinfo = &missileStaticInfos[shiprace];
        data[0] = bitTest(misstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
        data[1] = bitpackTeamColor(misstaticinfo->teamColor);
        data[2] = 0;
        data[3] = 0;
        SaveInfoNumber(TreatAsUdword(data[0]));
    }
    for (shiprace=0;shiprace<2;shiprace++)
    {
        MissileStaticInfo *misstaticinfo = &mineStaticInfos[shiprace];
        data[0] = bitTest(misstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
        data[1] = bitpackTeamColor(misstaticinfo->teamColor);
        data[2] = 0;
        data[3] = 0;
        SaveInfoNumber(TreatAsUdword(data[0]));
    }

    SaveInfoNumber(INFO_NEEDED_FLAGS_DELIMITER);
}

void universeLoadEverythingNeeded(void)
{
    ShipRace shiprace;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;

    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    ShipStaticInfo *shipstatic;

    udword data;
    ubyte *datab;

    datab = (ubyte *)&data;

    data = LoadInfoNumber();                        // sanity check
    dbgAssert(data == INFO_NEEDED_FLAGS_DELIMITER);

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            shipstatic = GetShipStaticInfo(shiptype,shiprace);
            data = LoadInfoNumber();
            bitSetTo(shipstatic->staticheader.infoFlags, IF_InfoNeeded, datab[0]);
            rmEnableShip(shiprace, shiptype, datab[0]);
            unbitpackTeamColor(shipstatic->teamColor,datab[1]);
        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        bitSetTo(asteroidStaticInfos[asteroidtype].staticheader.infoFlags, IF_InfoNeeded, LoadInfoNumber());
    }
    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        bitSetTo(dustcloudStaticInfos[dustcloudtype].staticheader.infoFlags, IF_InfoNeeded, LoadInfoNumber());
    }
    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        bitSetTo(gascloudStaticInfos[gascloudtype].staticheader.infoFlags, IF_InfoNeeded, LoadInfoNumber());
    }
    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        bitSetTo(nebulaStaticInfos[nebulatype].staticheader.infoFlags, IF_InfoNeeded, LoadInfoNumber());
    }
    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        DerelictStaticInfo *dstaticinfo = &derelictStaticInfos[derelicttype];
        data = LoadInfoNumber();
        bitSetTo(dstaticinfo->staticheader.infoFlags, IF_InfoNeeded, datab[0]);
        unbitpackTeamColor(dstaticinfo->teamColor,datab[1]);
    }
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        MissileStaticInfo *misstaticinfo = &missileStaticInfos[shiprace];
        data = LoadInfoNumber();
        bitSetTo(misstaticinfo->staticheader.infoFlags, IF_InfoNeeded, datab[0]);
        unbitpackTeamColor(misstaticinfo->teamColor,datab[1]);
    }
    for (shiprace=0;shiprace<2;shiprace++)
    {
        MissileStaticInfo *misstaticinfo = &mineStaticInfos[shiprace];
        data = LoadInfoNumber();
        bitSetTo(misstaticinfo->staticheader.infoFlags, IF_InfoNeeded, datab[0]);
        unbitpackTeamColor(misstaticinfo->teamColor,datab[1]);
    }

    data = LoadInfoNumber();                        // sanity check
    dbgAssert(data == INFO_NEEDED_FLAGS_DELIMITER);
}

/*-----------------------------------------------------------------------------
    Name        : universeStaticInit
    Description : Initializes static data for the Universe.  Only the ships,
                    resources, missiles etc. that are needed will be loaded.
                    Anything that was previously loaded and is no longer
                    needed will be deleted.
    Inputs      : none
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
extern void *utyTeaserHeader;
void universeStaticInit(void)
{
    ShipRace shiprace;
    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;
    ShipStaticInfo *shipstaticinfo;
    StaticInfo *staticinfo;
    udword max,count;
#if UNIV_SHIP_LOADFREE_LOG
    char *fileNameFull;
    FILE *logFile = NULL;

    if (univLoadFreeLog)
    {
        fileNameFull = filePathPrepend(
            UNIV_LOAD_FREE_FILENAME,
            FF_UserSettingsPath);

        if (fileMakeDestinationDirectory(fileNameFull))
            logFile = fopen(fileNameFull, "at");
    }
    if (logFile)
    {
        if (singlePlayerGame)
        {
            fprintf(logFile, "\n-----------\nCurrent level = %d\n", singlePlayerGameInfo.currentMission);
        }
    }
#endif

#ifdef _WIN32
#define TMP_DEFSHIP_PATH "DefaultShip\\"
#else
#define TMP_DEFSHIP_PATH "DefaultShip/"
#endif

    //load some things shared by everyone
    if (sphereStaticInfo == NULL)
    {
        sphereStaticInfo = createSphereStaticInfo();
    }
    if (defaultmesh == NULL)
    {
        defaultmesh = meshLoad(TMP_DEFSHIP_PATH "DefaultShip.geo");
    }
    if (defaultlod == NULL)
    {
        defaultlod = lodTableReadScript(TMP_DEFSHIP_PATH, "DefaultShip.lod");
    }
    if (defaultmex == NULL)
    {
        defaultmex = mexLoad(TMP_DEFSHIP_PATH "DefaultShip.mex");
    }

#undef TMP_DEFSHIP_PATH

    max = 0;
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        max += (LastShipTypeOfRace[shiprace] - FirstShipTypeOfRace[shiprace]);
    }
    max += NUM_ASTEROIDTYPES + NUM_DUSTCLOUDTYPES + NUM_NEBULATYPES + NUM_DERELICTTYPES + NUM_RACES + 2;

    HorseRaceBeginBar(UNIVERSE_BAR);
    count = 0;
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            HorseRaceNext(((real32)count)/((real32)max));
            if (hrAbortLoadingGame)
            {
                goto abortloadinggame;
            }
            count++;
            shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
            if (bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoNeeded))
            {                                               //if this info needed
                if (!bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded))
                {                                           //if this info not loaded
                    InitStatShipInfo(shipstaticinfo,shiptype,shiprace);
                    bitSet(shipstaticinfo->staticheader.infoFlags, IF_NotPostLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                    if (logFile)
                    {
                        fprintf(logFile, "Loading ship: %8s %s\n", ShipRaceToStr(shiprace), ShipTypeToStr(shiptype));
                    }
#endif
                }
                else
                {
                    if (shipstaticinfo->staticheader.LOD != NULL)
                    {
                        lodAllMeshesRecolorize(shipstaticinfo->staticheader.LOD);
                    }
                }
            }
            else
            {                                               //static info not needed this round
                if (bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded))
                {                                           //if it's been loaded
                    CloseStatShipInfo(shipstaticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                    if (logFile)
                    {
                        fprintf(logFile, "Freeing ship: %8s %s\n", ShipRaceToStr(shiprace), ShipTypeToStr(shiptype));
                    }
#endif
                }
            }

        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&asteroidStaticInfos[asteroidtype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatAsteroidInfo((AsteroidStaticInfo *)staticinfo,asteroidtype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading asteroid: %s\n", AsteroidTypeToStr(asteroidtype));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatAsteroidInfo((AsteroidStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing asteroid: %s\n", AsteroidTypeToStr(asteroidtype));
                }
#endif
            }
        }
        count++;
    }

    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&dustcloudStaticInfos[dustcloudtype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatDustCloudInfo((DustCloudStaticInfo *)staticinfo, dustcloudtype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading ship: %s\n", DustCloudTypeToStr(dustcloudtype));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatDustCloudInfo((DustCloudStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing dustcloud: %s\n", DustCloudTypeToStr(dustcloudtype));
                }
#endif
            }
        }
        count++;
    }

#if 0
    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        staticinfo = (StaticInfo *)&gascloudStaticInfos[gascloudtype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatGasCloudInfo((GasCloudStaticInfo *)staticinfo, gascloudtype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading gas cloud: %s\n", GasCloudTypeToStr(gascloudtype));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatGasCloudInfo((GasCloudStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing gascloud: %s\n", GasCloudTypeToStr(gascloudtype));
                }
#endif
            }
        }
    }
#endif

    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&nebulaStaticInfos[nebulatype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatNebulaInfo((NebulaStaticInfo *)staticinfo, nebulatype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading nebula: %s\n", NebulaTypeToStr(nebulatype));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatNebulaInfo((NebulaStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing nebula: %s\n", NebulaTypeToStr(nebulatype));
                }
#endif
            }
        }
        count++;
    }

    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&derelictStaticInfos[derelicttype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatDerelictInfo((DerelictStaticInfo *)staticinfo,derelicttype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading derelict: %s\n", DerelictTypeToStr(derelicttype));
                }
#endif
            }
            else
            {
                if (staticinfo->staticheader.LOD != NULL)
                {
                    lodAllMeshesRecolorize(staticinfo->staticheader.LOD);
                }
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatDerelictInfo((DerelictStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing derelict: %s\n", DerelictTypeToStr(derelicttype));
                }
#endif
            }
        }
        count++;
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&missileStaticInfos[shiprace];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatMissileInfo((MissileStaticInfo *)staticinfo,shiprace);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading missile: %8s\n", ShipRaceToStr(shiprace));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatMissileInfo((MissileStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing missile: %8s\n", ShipRaceToStr(shiprace));
                }
#endif
            }
        }
        count++;
    }

    maxMineCollSphereSize = 0.0f;
    for (shiprace=0;shiprace<2;shiprace++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&mineStaticInfos[shiprace];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatMineInfo((MissileStaticInfo *)staticinfo,shiprace);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
                if (mineStaticInfos[shiprace].staticheader.staticCollInfo.collspheresize > maxMineCollSphereSize)
                {
                    maxMineCollSphereSize = mineStaticInfos[shiprace].staticheader.staticCollInfo.collspheresize;
                }
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading mine: %8s\n", ShipRaceToStr(shiprace));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatMineInfo((MissileStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing mine: %8s\n", ShipRaceToStr(shiprace));
                }
#endif
            }
        }
        count++;
    }

    maxMineCollSphereSize *= 2.0f;

//    InitStatEffectInfo(&effectStaticInfo);

abortloadinggame:

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
            if (bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoNeeded))
            {
                if (bitTest(shipstaticinfo->staticheader.infoFlags, IF_NotPostLoaded))
                {                                           //if ship just now loaded
                    InitStatShipInfoPost(shipstaticinfo);
                    //shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded) = TRUE;
                    bitClear(shipstaticinfo->staticheader.infoFlags, IF_NotPostLoaded);
                    bitSet(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
                }
            }
        }
    }


#if UNIV_SHIP_LOADFREE_LOG
    if (logFile)
    {
        fclose(logFile);
    }
#endif

    if (hrAbortLoadingGame)
    {
        ;
    }
    else
    {
        if (singlePlayerGame)
        {
            GetShipStaticInfo(Mothership,R1)->staticheader.immobile = TRUE;
            GetShipStaticInfo(Mothership,R2)->staticheader.immobile = TRUE;
        }
        else
        {
            GetShipStaticInfo(Mothership,R1)->staticheader.immobile = FALSE;
            GetShipStaticInfo(Mothership,R2)->staticheader.immobile = FALSE;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : universeStaticInit
    Description : Reinitializes some static data:
                                                    *.Shp variables
    Inputs      : none
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void universeRealTimeTweak(SelectCommand *selection)
{
    ShipRace shiprace;
    ShipType shiptype;
    ShipStaticInfo *shipstaticinfo;
    udword shiparray[100];
    udword shiptypedone;
    udword shipdoneptr=0;
    udword index;
    sdword flag = FALSE;

    for(shiptypedone = 0;shiptypedone < 100; shiptypedone++)
    {
        shiparray[shiptypedone] = 99999;    //fill with garbage ships
    }
    for (index = 0;index < selection->numShips;index++)
    {
        shiprace = selection->ShipPtr[index]->staticinfo->shiprace;
        shiptype = selection->ShipPtr[index]->shiptype;

        for(shiptypedone = 0;shiptypedone <=shipdoneptr; shiptypedone++)
        {
            if(shiparray[shiptypedone] == shiptype)
            {
                //shiptype already loaded, so don't load it
                flag = TRUE;
                break;
            }
        }
        if(flag == TRUE)
        {
            flag = FALSE;
            continue;
        }

        shiparray[shipdoneptr] = shiptype;
        shipdoneptr++;
        shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
        InitStatShipInfo(shipstaticinfo,shiptype,shiprace);
    }
}
void universeSetZeroPlayers(void)
{
    universe.numPlayers = 0;
    cameraInit(&defaultCamera,5000.0f);
}

/*-----------------------------------------------------------------------------
    Name        : universeNoStaticLoadedOrNeeded
    Description : Marks all static static infos as not loaded and not needed
                  in fact, it sets the entire static infos to NULL
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void universeNoStaticLoadedOrNeeded(void)
{
    ShipRace shiprace;
    ShipType shiptype;
    ShipStaticInfo *shipStaticInfo;
    AsteroidStaticInfo *asteroidStaticInfo;
    DustCloudStaticInfo *dustcloudStaticInfo;
    GasCloudStaticInfo *gascloudStaticInfo;
    NebulaStaticInfo *nebulaStaticInfo;
    DerelictStaticInfo *derelictStaticInfo;
    MissileStaticInfo *missileStaticInfo;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;
    ShipType firstshiptype;
    ShipType lastshiptype;

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            shipStaticInfo = GetShipStaticInfo(shiptype,shiprace);
            memset(shipStaticInfo,0,sizeof(ShipStaticInfo));
        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        asteroidStaticInfo = &asteroidStaticInfos[asteroidtype];
        memset(asteroidStaticInfo,0,sizeof(*asteroidStaticInfo));
    }

    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        dustcloudStaticInfo = &dustcloudStaticInfos[dustcloudtype];
        memset(dustcloudStaticInfo,0,sizeof(*dustcloudStaticInfo));
    }

    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        gascloudStaticInfo = &gascloudStaticInfos[gascloudtype];
        memset(gascloudStaticInfo,0,sizeof(*gascloudStaticInfo));
    }

    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        nebulaStaticInfo = &nebulaStaticInfos[nebulatype];
    }

    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        derelictStaticInfo = &derelictStaticInfos[derelicttype];
        memset(derelictStaticInfo,0,sizeof(*derelictStaticInfo));
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        missileStaticInfo = &missileStaticInfos[shiprace];
        memset(missileStaticInfo,0,sizeof(*missileStaticInfo));
        if (shiprace <= R2)
        {
            missileStaticInfo = &mineStaticInfos[shiprace];
            memset(missileStaticInfo,0,sizeof(*missileStaticInfo));
        }
    }
}

void universeInit(void)
{
    sdword index;

    univupdateInit();

    clInit(&universe.mainCommandLayer);
    ccInit(&universe.mainCameraCommand);

    universeSetZeroPlayers();

    universeNoStaticLoadedOrNeeded();

    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        universe.world[index] = NULL;
    }
}

void universeClosePlayers(void)
{
    udword i;

    for (i=0;i<universe.numPlayers;i++)
    {
        if (universe.players[i].aiPlayer != NULL)
        {
            aiplayerClose(universe.players[i].aiPlayer);
            universe.players[i].aiPlayer = NULL;
        }
        universe.players[i].PlayerMothership = NULL;
    }
}

void universeReset(void)
{
    sdword index;

    clReset(&universe.mainCommandLayer);
    ccReset(&universe.mainCameraCommand);

    pingAllPingsDelete();

    cmReset(); // Free carrier factory pointers in construction manager

    universeClosePlayers();

    univupdateCloseAllObjectsAndMissionSpheres();
    // don't do univupdateReset() because it is done in gameStart

    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        universe.world[index] = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : universeStaticCloseAllStatics
    Description : Closes all universe static structures for good.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void universeStaticCloseAllStatics(void)
{
    ShipRace shiprace;
    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    DerelictType derelicttype;
    StaticInfo *staticInfo;

//    CloseStatEffectInfo(&effectStaticInfo);

    if (sphereStaticInfo != NULL)
    {
        freeSphereStaticInfo(sphereStaticInfo);
        sphereStaticInfo = NULL;
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        staticInfo = (StaticInfo *)&missileStaticInfos[shiprace];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatMissileInfo((MissileStaticInfo *)staticInfo);
        }
    }

    for (shiprace=0;shiprace<2;shiprace++)        //only two races have mines
    {
        staticInfo = (StaticInfo *)&mineStaticInfos[shiprace];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatMineInfo((MissileStaticInfo *)staticInfo);
        }
    }

    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        staticInfo = (StaticInfo *)&derelictStaticInfos[derelicttype];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatDerelictInfo((DerelictStaticInfo *)staticInfo);
        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        staticInfo = (StaticInfo *)&asteroidStaticInfos[asteroidtype];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatAsteroidInfo((AsteroidStaticInfo *)staticInfo);
        }
    }

    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        staticInfo = (StaticInfo *)&dustcloudStaticInfos[dustcloudtype];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatDustCloudInfo((DustCloudStaticInfo *)staticInfo);
        }
    }

    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        staticInfo = (StaticInfo *)&gascloudStaticInfos[gascloudtype];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatGasCloudInfo((GasCloudStaticInfo *)staticInfo);
        }
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            staticInfo = (StaticInfo *)GetShipStaticInfo(shiptype,shiprace);
            if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
            {
                CloseStatShipInfo((ShipStaticInfo *)staticInfo);
            }
        }
    }

    if (defaultlod != NULL)
    {
        lodFree(defaultlod);
        defaultlod = NULL;
    }

    if (defaultmex != NULL)
    {
        mexFree(defaultmex);
        defaultmex = NULL;
    }

    if (defaultmesh != NULL)
    {
        meshFree(defaultmesh);
        defaultmesh = NULL;
    }
}

void universeClose(void)
{
    clClose(&universe.mainCommandLayer);
    ccClose(&universe.mainCameraCommand);

    universeClosePlayers();

    univupdateClose();

    universeStaticCloseAllStatics();
}

/*-----------------------------------------------------------------------------
    Name        : universeStaticClose
    Description : Closes static data of the Universe
    Inputs      : none
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void universeStaticClose(void)
{
}

/*-----------------------------------------------------------------------------
    Name        : universeUpdateTask
    Description : the main loop of the task which updates the universe (all
                  mission spheres) and executes all of the physics stuff
                  and moves all of the ships.  This task runs independent of
                  the screen refresh rate.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void universeUpdateTask(void)
{
    static WaitPacketStatus waitpacketstatus;
    static sdword repeatuniv;
    static sdword repeattimes;

    taskYield(0);

#ifndef C_ONLY
    for(;;)
#endif
    {
        if ((multiPlayerGame) && (startingGame) && (gameIsRunning) && ((IAmCaptain) && (!multiPlayerGameUnderWay)))
        {
            if (checkPlayersReady())
            {
                multiPlayerGameUnderWay = TRUE;
            }
        }

        if (startingGameState == CHECK_AUTODOWNLOAD_MAP)
        {
            taskStackSaveCond(0);
            universeUpdateCounter++;

            if (!LANGame) chatClose();
            if (sigsPlayerIndex == 0)
                autodownloadmapGotMapName(spScenarios[spCurrentSelected].fileSpec,numPlayers,spScenarios[spCurrentSelected].minplayers,spScenarios[spCurrentSelected].maxplayers);
            else
                autodownloadmapGotMapName(tpGameCreated.MapName,numPlayers,0,0);
            horseRaceInit();
            if (autodownloadmapRequired())
            {
                HorseRaceBeginBar(DOWNLOADMAP_BAR);

                if (sigsPlayerIndex == 0)
                {
                    autodownloadmapGetFilesOfMap();
                }

                autodownloadmapPrintStatusStart();
                startingGameState = AUTODOWNLOAD_MAP;
            }
            else
            {
                startingGameState = PROCESS_TEXTURES;
                goto processtextures;
            }
        }
        else if (startingGameState == AUTODOWNLOAD_MAP)
        {
            taskStackSaveCond(0);
repeatagainwithoutyielding:         // for horse race hack we need this silly hack
            universeUpdateCounter++;

            autodownloadmapPrintStatus();
            if (sigsPlayerIndex == 0)
            {
                // for now, lets just bang everything through:
                if (autodownloadmapSendAFile())
                {
                    HorseRaceNext(1.0f);
                    startingGameState = PROCESS_TEXTURES;
                    goto processtextures;
                }
                else
                {
                    HorseRaceNext(autodownloadmapPercentSentFiles());
                    // don't check for aborting - will mess up everyone since files must be sent!
                }
            }
            else
            {
                if (autodownloadmapReceivedAllFiles())
                {
                    HorseRaceNext(1.0f);
                    startingGameState = PROCESS_TEXTURES;
                    goto processtextures;
                }
                else
                {
                    HorseRaceNext(autodownloadmapPercentReceivedFiles());
                    if (hrAbortLoadingGame)
                    {
                        startingGameState = PROCESS_TEXTURES;
                        goto processtextures;
                    }
                }
            }

            goto repeatagainwithoutyielding;
        }
        else if (startingGameState == PROCESS_TEXTURES)
        {
            taskStackSaveCond(0);
            universeUpdateCounter++;
processtextures:
            if (!hrAbortLoadingGame)
            {
                gameStart(NULL);
            }
            horseRaceShutdown();

            startingGameState = GAME_STARTED;

            if (hrAbortLoadingGame)
            {
                utyGameQuitToMain(NULL,NULL);
                hrAbortLoadingGame = FALSE;
            }
            else if (multiPlayerGame)
            {
                TransferCaptaincyGameStarted();
            }

            taskStackRestoreCond();
        }
        else if (gameIsRunning)
        {
            taskStackSaveCond(0);
            universeUpdateCounter++;

            if ((multiPlayerGame) || (playPackets|recordFakeSendPackets))
            {
                if (!universe.dontUpdateRenderList)
                {
                    if ((universe.univUpdateCounter == 1) || ((universeUpdateCounter & REFRESH_RENDERLIST_RATE) == REFRESH_RENDERLIST_FRAME))
                    {
                        univUpdateRenderList();
                    }

                    if ((universe.univUpdateCounter == 1) || ((universeUpdateCounter & REFRESH_MINORRENDERLIST_RATE) == REFRESH_MINORRENDERLIST_FRAME))        // make it 3 to mix things up a bit
                    {
                        univUpdateMinorRenderList();
                    }
                }

                if (!(playPackets|recordFakeSendPackets))
                {
                    if ((!multiPlayerGameUnderWay) && (!IAmCaptain))
                    {
                        if (!HaveSentNonCaptainReadyPacket)
                        {
                            SendNonCaptainReadyPacket(sigsPlayerIndex);
                            HaveSentNonCaptainReadyPacket = TRUE;
                        }
                    }
                }

                if (multiPlayerGame)
                {
                    gcPollForNewChat();
                    TransferCaptaincyUpdate();
                    KeepAliveUpdate();

                    if ((universeUpdateCounter & INTERNET_LAG_UPDATERATE) == 0)
                    {
                        lagUpdateInternetLag();
                    }
                }

                if (playPackets|recordFakeSendPackets)
                {
                    if (universePause)
                    {
                        goto alldone;
                    }
                }

#if UNIVERSE_TURBOPAUSE_DEBUG
                if ((playPackets|recordFakeSendPackets) && universeTurbo)
                {
                    repeattimes = UNIVERSE_TURBO_REPEAT>>1;
                    dbgAssert(repeattimes >= 1);
                }
                else
#endif
                    repeattimes = 1;



                for (repeatuniv=0;repeatuniv<repeattimes;repeatuniv++)
                {
                    waitpacketstatus = clWaitSyncPacket(&universe.mainCommandLayer);
                    if (waitpacketstatus != NO_PACKET)
                    {
                        if (multiPlayerGame)
                        {
                            multiPlayerGameUnderWay = TRUE;
                            TransferCaptaincySyncPacketReceivedNotify();
                        }

                        netCheck();

                        if (univUpdate(UNIVERSE_UPDATE_PERIOD))
                        {
                            goto alldone;
                        }
                        if (! ((waitpacketstatus == TOO_MANY_PACKETS) ||        // if TOO_MANY_PACKETS, don't yield, and have throughput at twice normal to catch up
                              (repeattimes > 1)) )
                        {
                            taskStackRestoreCond();
                            taskYield(0);
                            taskStackSaveCond(0);
                        }
                        if (!gameIsRunning)
                        {
                            goto alldone;
                        }
                        if (univUpdate(UNIVERSE_UPDATE_PERIOD))
                        {
                            goto alldone;
                        }
                    }
                }
    alldone:;

            }
            else
            {
                if (!universe.dontUpdateRenderList)
                {
                    if ((universe.univUpdateCounter == 1) || ((universeUpdateCounter & REFRESH_RENDERLIST_RATE) == REFRESH_RENDERLIST_FRAME))
                    {
                        univUpdateRenderList();
                    }

                    if ((universe.univUpdateCounter == 1) || ((universeUpdateCounter & REFRESH_MINORRENDERLIST_RATE) == REFRESH_MINORRENDERLIST_FRAME))        // make it 3 to mix things up a bit
                    {
                        univUpdateMinorRenderList();
                    }
                }

                if (universePause)
                {
                    goto alldone2;
                }

                if((tutorial==TUTORIAL_ONLY) && !tutEnable.bGameRunning)
                {
                    tutEnable.bKASFrame ^= 1;
                    if(tutEnable.bKASFrame)
                        kasExecute();       // Make sure KAS gets called

                    goto alldone2;
                }

#if (!UNIVERSE_TURBORECORD_ONLY)
#if UNIVERSE_TURBOPAUSE_DEBUG
                if (universeTurbo)
                {
                    for (repeatuniv=0;repeatuniv<UNIVERSE_TURBO_REPEAT;repeatuniv++)
                    {
                        if (univUpdate(UNIVERSE_UPDATE_PERIOD))
                        {
                            goto alldone2;
                        }
#ifndef HW_Release
                        if (ShowFancyFights) statsShowFancyFightUpdate();
#endif
                    }
                }
#endif
#endif
                univUpdate(UNIVERSE_UPDATE_PERIOD);
alldone2:;
#ifndef HW_Release
                if (ShowFancyFights) statsShowFancyFightUpdate();
#endif

            }
            taskStackRestoreCond();
        }
        else if (!startingGame)
        {
            if (sigsPressedStartGame)
            {
                utyNewGameStart(NULL, NULL);
            }
        }

        if (startRecordingGameWhenSafe)
        {
            recPackInGameStartCBSafeToStart();
            startRecordingGameWhenSafe = FALSE;
        }

        taskYield(0);
    }

    taskExit();
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : universeSwitchToNextPlayer
    Description : switches current player to the next player (wraps around)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void universeSwitchToNextPlayer(void)
{
    universe.curPlayerIndex++;
    if (universe.curPlayerIndex >= universe.numPlayers)
    {
        universe.curPlayerIndex = 0;
    }
    universe.curPlayerPtr = &universe.players[universe.curPlayerIndex];

    universe.collUpdateAllBlobs = TRUE;                              //next update will update all blobs

    dbgMessagef("\nCurrent player: %d %x",universe.curPlayerIndex,universe.curPlayerPtr);
}

/*-----------------------------------------------------------------------------
    Name        : universeSwitchToPlayer
    Description : switches current player to player indicated by playerIndex
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void universeSwitchToPlayer(uword playerIndex)
{
    dbgAssert(playerIndex < universe.numPlayers);
    universe.curPlayerIndex = playerIndex;
    universe.curPlayerPtr = &universe.players[universe.curPlayerIndex];
}

/*-----------------------------------------------------------------------------
    Name        : GetShipStaticInfo
    Description : returns ShipStaticInfo of shiptype, shiprace
    Inputs      : shiptype, shiprace
    Outputs     :
    Return      : returns ShipStaticInfo of shiptype, shiprace
----------------------------------------------------------------------------*/
ShipStaticInfo *GetShipStaticInfo(ShipType shiptype,ShipRace shiprace)
{
    ShipStaticInfo *raceInfo = RaceShipStaticInfos[shiprace];
    sdword offset;

    dbgAssert(shiprace < NUM_RACES);
    dbgAssert(shiptype < TOTAL_NUM_SHIPS);

    offset = shiptype - FirstShipTypeOfRace[shiprace];
    dbgAssert(offset >= 0);
    dbgAssert(offset < NumShipTypesInRace[shiprace]);

    return &raceInfo[offset];
}

/*-----------------------------------------------------------------------------
    Name        : GetShipStaticInfoSafe
    Description : returns ShipStaticInfo of shiptype, shiprace, NULL if not valid
    Inputs      : shiptype, shiprace
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
ShipStaticInfo *GetShipStaticInfoSafe(ShipType shiptype,ShipRace shiprace)
{
    ShipStaticInfo *raceInfo = RaceShipStaticInfos[shiprace];
    ShipStaticInfo *shipstatic;
    sdword offset;

    if ((shiprace >= NUM_RACES))
    {
        return NULL;
    }

    if ((shiptype >= TOTAL_NUM_SHIPS))
    {
        return NULL;
    }

    offset = shiptype - FirstShipTypeOfRace[shiprace];

    if ((offset < 0) || (offset >= NumShipTypesInRace[shiprace]))
    {
        return NULL;
    }

    shipstatic = &raceInfo[offset];
    if (!bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded))
    {
        return NULL;
    }

    return shipstatic;
}

/*-----------------------------------------------------------------------------
    Name        : GetValidRaceForShipType
    Description : returns valid race for shiptype, -1 if none found
    Inputs      : shiptype
    Outputs     :
    Return      : returns valid race for shiptype, -1 if none found
----------------------------------------------------------------------------*/
ShipRace GetValidRaceForShipType(ShipType shiptype)
{
    dbgAssert(shiptype < TOTAL_NUM_SHIPS);

    if (RacesAllowedForGivenShip[shiptype] & R1_VALID)
    {
        return R1;
    }
    if (RacesAllowedForGivenShip[shiptype] & R2_VALID)
    {
        return R2;
    }
    if (RacesAllowedForGivenShip[shiptype] & P1_VALID)
    {
        return P1;
    }
    if (RacesAllowedForGivenShip[shiptype] & P2_VALID)
    {
        return P2;
    }
    if (RacesAllowedForGivenShip[shiptype] & P3_VALID)
    {
        return P3;
    }
    if (RacesAllowedForGivenShip[shiptype] & Traders_VALID)
    {
        return Traders;
    }

    dbgAssert(FALSE);
    return R1;
}

/*-----------------------------------------------------------------------------
    Name        : GetShipStaticInfoValidate
    Description : returns ShipStaticInfo of shiptype, shiprace.  Makes sure data loaded
    Inputs      : shiptype, shiprace
    Outputs     :
    Return      : returns ShipStaticInfo of shiptype, shiprace
----------------------------------------------------------------------------*/
ShipStaticInfo *GetShipStaticInfoValidate(ShipType shiptype,ShipRace shiprace)
{
    ShipStaticInfo *shipstatic = GetShipStaticInfo(shiptype,shiprace);
    if (!bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded))
    {
        dbgFatalf(DBG_Loc,"Ship %s %s not loaded",ShipRaceToStr(shiprace),ShipTypeToStr(shiptype));
    }
    return shipstatic;
}

/*  Unit Capping functions */

/*-----------------------------------------------------------------------------
    Name        : unitCapCreateShip
    Description : Updates ship totals for player
    Inputs      : uword shiptype, Player *player
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void unitCapCreateShip(Ship *ship, Player *player)
{
//    if (player->aiPlayer)
//    {
//        return;
//    }
    // don't keep tabs on these ship types
    if ((cdEnabled) &&
        (ship->shiptype != Drone) &&
        (ship->shiptype != TargetDrone) &&
        (ship->shiptype != HeadShotAsteroid) &&
        (ship->shiptype != CryoTray) &&
        (! (bitTest(ship->specialFlags2,SPECIAL_2_InUnitCaps)) ) )
    {
        player->totalships++;
        player->shiptotals[ship->shiptype]++;
        player->classtotals[ship->staticinfo->shipclass]++;
        bitSet(ship->specialFlags2,SPECIAL_2_InUnitCaps);
    }
}

/*-----------------------------------------------------------------------------
    Name        : unitCapDeleteShip
    Description : Updates ship totals for player
    Inputs      : uword shiptype, Player *player
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void unitCapDeleteShip(Ship *ship, Player *player)
{

    // don't keep tabs on these ship types
//    if (player->aiPlayer)
//    {
//        return;
//    }
    if ((cdEnabled) &&
        (ship->shiptype != Drone) &&
        (ship->shiptype != TargetDrone) &&
        (ship->shiptype != HeadShotAsteroid) &&
        (ship->shiptype != CryoTray) &&
        ( (bitTest(ship->specialFlags2,SPECIAL_2_InUnitCaps)) ))
    {
        player->totalships--;

//        dbgAssert(player->totalships >= 0);
        if (player->totalships < 0)
        {
            player->totalships = 0;
            dbgMessagef("\nWarning: total shipcount in unit caps went below 0");
        }

        player->shiptotals[ship->shiptype]--;

//        dbgAssert(player->shiptotals[ship->shiptype] >= 0);
        if (player->shiptotals[ship->shiptype] < 0)
        {
            player->shiptotals[ship->shiptype] = 0;
            dbgMessagef("\nWarning: shipcount of shiptype %d went below 0");
        }

        player->classtotals[ship->staticinfo->shipclass]--;

//        dbgAssert(player->classtotals[ship->staticinfo->shipclass] >= 0);
        if (player->classtotals[ship->staticinfo->shipclass] < 0)
        {
            player->classtotals[ship->staticinfo->shipclass] = 0;
            dbgMessagef("\nWarning: classtotals of class %d went below 0",ship->staticinfo->shipclass);
        }
        bitClear(ship->specialFlags2,SPECIAL_2_InUnitCaps);
    }
}

/*-----------------------------------------------------------------------------
    Name        : unitCapCanCreateShip
    Description : Can we build this ship ??
    Inputs      : uword shiptype, Player *player
    Outputs     : TRUE if ship is within limit caps
                  FLASE if ship will exceed limit caps
    Return      : bool8
-----------------------------------------------------------------------------*/
bool unitCapCanCreateShip(ShipType ship, shipsinprogress *factory, shipavailable *cmShipsAvail)
{
    Player *player=factory->ship->playerowner;
    shipsinprogress *allfactories=(shipsinprogress *)factory->node.belongto->head;
    udword   totalships=1, typeships=1, classships=1,index;
    ShipStaticInfo *statinfo, *shipstatic;

    if ((!bitTest(tpGameCreated.flag,MG_UnitCapsEnabled)) &&
        (ship!=ResearchShip)&&(!singlePlayerGame))
    {
        return(TRUE);
    }

    shipstatic = GetShipStaticInfo(ship, player->race);

    // total up all ships in the queue but not yet started
    for (index=0;cmShipsAvail[index].nJobs != -1; index++)
    {
        statinfo = GetShipStaticInfo(cmShipsAvail[index].type, player->race);
        totalships+=cmShipsAvail[index].nJobs;
        if (cmShipsAvail[index].type == shipstatic->shiptype) typeships+=cmShipsAvail[index].nJobs;
        if (statinfo->shipclass == shipstatic->shipclass) classships+=cmShipsAvail[index].nJobs;
    }

    // walk through list of factories
    do
    {
        // if factory is owned by player then total all ships being built
        if (allfactories->ship->playerowner==player)
        {
            for (index=0;index<TOTAL_STD_SHIPS;index++)
            {
                statinfo = GetShipStaticInfo(index, player->race);
                totalships+=allfactories->progress[index].nJobs + shiplagtotals[index];
                if (index==shipstatic->shiptype) typeships+=allfactories->progress[index].nJobs + shiplagtotals[index];
                if (statinfo->shipclass == shipstatic->shipclass) classships+=allfactories->progress[index].nJobs + shiplagtotals[index];
            }

        }
        // next factory
        allfactories=(shipsinprogress *)allfactories->node.next;
    }
    while (allfactories != NULL);

    if (player->totalships+totalships > cdMaxShipsAllowed)
    {
        speechEventFleet(STAT_F_Const_TotalLimit, 0, player->playerIndex);
        // player already has too many ships cannot build anymore
        return(FALSE);
    }

    if (cdLimitCaps[shipstatic->shiptype] != -1)
    {
        if (player->shiptotals[shipstatic->shiptype]+typeships > cdLimitCaps[shipstatic->shiptype])
        {
            speechEventFleet(STAT_F_Const_UnitLimit, shipstatic->shiptype, player->playerIndex);
            // player has to many of this type of ship
            return(FALSE);
        }
    }

    if (player->classtotals[shipstatic->shipclass]+classships > cdClassCaps[shipstatic->shipclass])
    {
        speechEventFleet(STAT_F_Const_UnitLimit, shipstatic->shiptype, player->playerIndex);
        // player has to many of this class of ship
        return(FALSE);
    }

    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : unitCapInitialize
    Description : Initialize the unit totals to zero
    Inputs      : Player *player
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void unitCapInitialize(Player *player)
{
    player->totalships=0;
    memset(player->shiptotals, 0,sizeof(sdword)*TOTAL_NUM_SHIPS);
    memset(player->classtotals,0,sizeof(sdword)*NUM_CLASSES);
}

/*-----------------------------------------------------------------------------
    Name        : unitCapEnable
    Description : Enables unit cap tracking
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void unitCapEnable(void)
{
    cdEnabled = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : unitCapDisable
    Description : disables unit cap tracking
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void unitCapDisable(void)
{
    cdEnabled = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : resourceInjectionTask
    Description : this is setup to be called at a specified time if it was enabled.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void unitCapInitStatics(udword numplayers)
{
    char temp[128];

    sprintf(temp,"UnitLimitCaps%i.script", numplayers);
    scriptSet(NULL,temp,ShipStaticCapLimits);
}

/*-----------------------------------------------------------------------------
    Name        : resourceInjectionTask
    Description : this is setup to be called at a specified time if it was enabled.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void resourceInjectionTask(void)
{
    sdword index;

    for (index=0;index<universe.numPlayers;index++)
    {
        if (universe.players[index].playerState != PLAYER_DEAD)
        {
            universe.players[index].resourceUnits += tpGameCreated.resourceInjectionsAmount;
            universe.gameStats.playerStats[index].totalResourceUnits += tpGameCreated.resourceInjectionsAmount;
            universe.gameStats.playerStats[index].totalInjectedResources += tpGameCreated.resourceInjectionsAmount;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : resourceLumpSum
    Description : this will give each player a lump sum amount of resources, once during the game
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void resourceLumpSum(void)
{
    sdword index;

    for (index=0;index<universe.numPlayers;index++)
    {
        if (universe.players[index].playerState != PLAYER_DEAD)
        {
            universe.players[index].resourceUnits += tpGameCreated.resourceLumpSumAmount;
            universe.gameStats.playerStats[index].totalResourceUnits += tpGameCreated.resourceLumpSumAmount;
            universe.gameStats.playerStats[index].totalInjectedResources += tpGameCreated.resourceLumpSumAmount;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : initPlayerStats
    Description : zeros the players inital multiplayer statistics
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void initGameStats(Universe *universe)
{
    sdword j,i;

    universe->gameStats.totalResourcesInGame = 0;
    universe->gameStats.totalShipsInGame = 0;
    universe->gameStats.totalKills = 0;
    universe->gameStats.totalRUsInAllShips = 0;
    universe->gameStats.totalRUsKilled = 0;
    universe->gameStats.totalResourceUnitsCollected = 0;
    universe->gameStats.totalRUsLost = 0;
    universe->gameStats.updatedRUValuesTime = 0.0f;
    universe->gameStats.universeRUWorth = 0;


    for(i=0;i<universe->numPlayers;i++)
    {
        universe->gameStats.playerStats[i].timeOfDeath = 0;
        universe->gameStats.playerStats[i].typeOfPlayerDeath = 0;
        universe->gameStats.playerStats[i].totalResourceUnits = 0;
        universe->gameStats.playerStats[i].totalResourceUnitsCollected = 0;                 //[]
        universe->gameStats.playerStats[i].totalResourceUnitsSpent = 0;                     //[]
        universe->gameStats.playerStats[i].totalResourceUnitsGiven = 0;                     //[]
        universe->gameStats.playerStats[i].totalResourceUnitsRecieved = 0;
        universe->gameStats.playerStats[i].totalRegeneratedResourceUnits = 0;               //[x]
        universe->gameStats.playerStats[i].totalInjectedResources = 0;               //[x]
        universe->gameStats.playerStats[i].totalResourceUnitsViaBounties = 0;               //[x]



        universe->gameStats.playerStats[i].totalRUsInCurrentShips = 0;
        universe->gameStats.playerStats[i].totalShips = 0;

        universe->gameStats.playerStats[i].totalKills = 0;
        universe->gameStats.playerStats[i].totalRUsKilled = 0;

        universe->gameStats.playerStats[i].totalLosses = 0;
        universe->gameStats.playerStats[i].totalRUsLost = 0;


        for(j=0;j<universe->numPlayers;j++)
        {
            universe->gameStats.playerStats[i].pointsOfDamageDoneToWho[j] = 0.0f;
        }

        for(j=0;j<TOTAL_NUM_SHIPS;j++)
        {
            universe->gameStats.playerStats[i].totalForEachShip[j]=0;
            universe->gameStats.playerStats[i].totalKillsForEachShip[j]=0;
            universe->gameStats.playerStats[i].totalLossesForEachShip[j]=0;
        }
        for(j=0;j<NUM_CLASSES;j++)
        {
            universe->gameStats.playerStats[i].totalForEachClass[j]=0;
            universe->gameStats.playerStats[i].totalKillsForEachClass[j]=0;
            universe->gameStats.playerStats[i].totalLossesForEachClass[j]=0;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : gameStatsAddShip
    Description : Updates stats for ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void gameStatsAddShip(Ship *ship,sdword playerIndex)
{
    universe.gameStats.playerStats[playerIndex].totalShips++;
    universe.gameStats.playerStats[playerIndex].totalForEachShip[ship->shiptype]++;
    universe.gameStats.playerStats[playerIndex].totalForEachClass[ship->staticinfo->shipclass]++;
    universe.gameStats.totalRUsInAllShips += ship->staticinfo->buildCost;
    universe.gameStats.playerStats[playerIndex].totalRUsInCurrentShips += ship->staticinfo->buildCost;
    universe.gameStats.totalShipsInGame++;
}


void gameStatsShipDied(Ship *ship)
{
    sdword shipownerindex,killerPlayerIndex;

    if(ship->playerowner != NULL)
        shipownerindex = ship->playerowner->playerIndex;
    else
        dbgFatalf(DBG_Loc,"\nShip player pointer NULL...shouldn't be");
    if(ship->shiptype == Drone)
    {
        //we don't want to count drones as kills..
        //because they aren't really a critical kill
        //andthey have no real RU value
        return;
    }
    switch(ship->howDidIDie)
    {
    case DEATH_Killed_By_Player:
    case DEATH_Killed_By_Player_Explosion:
    case DEATH_I_DIDNT_DIE_I_WAS_SALAVAGED:
    case DEATH_Killed_By_Kamikaze:
        killerPlayerIndex = ship->whoKilledMe;
        universe.gameStats.totalRUsKilled += ship->staticinfo->buildCost;
        universe.gameStats.totalKills++;
        break;
    case DEATH_BY_GAME_END:
        return;

    //non player depenedant:  Only register losses, not kills
    case DEATH_Killed_By_Collision:
    case DEATH_Killed_By_Cloud:
    case DEATH_Killed_By_Dead_Player:
    case DEATH_BY_RETIREMENT:
    case DEATH_BY_AFTER_DEAD:
    case DEATH_Left_Behind_At_Hyperspace:
    default:
        killerPlayerIndex = 99;
        break;
    }

    if(killerPlayerIndex != ship->playerowner->playerIndex && killerPlayerIndex != 99)
    {
        //Didn't kill own ships...and we need to register the kills
        universe.gameStats.playerStats[killerPlayerIndex].totalRUsKilled += ship->staticinfo->buildCost;
        universe.gameStats.playerStats[killerPlayerIndex].totalKills++;
        universe.gameStats.playerStats[killerPlayerIndex].totalKillsForEachShip[ship->shiptype]++;
        universe.gameStats.playerStats[killerPlayerIndex].totalKillsForEachClass[ship->staticinfo->shipclass]++;
    }

    universe.gameStats.playerStats[ship->playerowner->playerIndex].totalLosses++;
    universe.gameStats.playerStats[ship->playerowner->playerIndex].totalLossesForEachShip[ship->shiptype]++;
    universe.gameStats.playerStats[ship->playerowner->playerIndex].totalLossesForEachClass[ship->staticinfo->shipclass]++;
    universe.gameStats.playerStats[ship->playerowner->playerIndex].totalRUsLost += ship->staticinfo->buildCost;
    universe.gameStats.playerStats[ship->playerowner->playerIndex].totalRUsInCurrentShips -= ship->staticinfo->buildCost;

    universe.gameStats.totalRUsLost += ship->staticinfo->buildCost;
    universe.gameStats.totalRUsInAllShips -= ship->staticinfo->buildCost;
}
//DEBUG FUNCTION

#ifdef DEBUG_GAME_STATS

char gamestatsfilename[] = "gamestats.log";
sdword needToInit=FALSE;

void gameStatsInitLog()
{
    needToInit=TRUE;
}
void gameStatsInitForReal()
{
    filehandle fh;
    FILE *fp;
    sdword i;
    GameStatsDebugHeader header;
    fileDelete(gamestatsfilename);

    fh = fileOpen(gamestatsfilename, FF_AppendMode | FF_UserSettingsPath);
    dbgAssert(!fileUsingBigfile(fh));
    fp = fileStream(fh);

    header.numPlayers = universe.numPlayers;
    for(i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        strcpy(header.names[i],playerNames[i]);
    }
    //now create and save header file
    fwrite(&header,sizeof(GameStatsDebugHeader),1,fp);

    fileClose(fh);
}
void gameStatsUpdateLogFile(sdword framenum)
{
    filehandle fh;
    FILE *fp;
    //sdword i;

    if(needToInit)
    {
        gameStatsInitForReal();
        needToInit=FALSE;
    }
    fh = fileOpen(gamestatsfilename, FF_AppendMode | FF_UserSettingsPath);
    dbgAssert(!fileUsingBigfile(fh));
    fp = fileStream(fh);

    //write frame number
    fwrite(&framenum,sizeof(sdword),1,fp);
    //write global game stats
    fwrite(&universe.gameStats,sizeof(GameStats)-sizeof(PlayerStats)*(MAX_MULTIPLAYER_PLAYERS+1-universe.numPlayers),1,fp);
    //write individual player stats
    //for(i=0;i<universe.numPlayers;i++)
    //{
    //    fwrite(&universe.gameStats.playerStats[i],sizeof(PlayerStats),1,fp);
    //}
    fileClose(fh);
}

#endif

// this function calculates the total ship cost for each player (typically at game start)
void gameStatsCalcShipCostTotals(void)
{
    ShipStaticInfo *pInfo;

    sdword i,j;

    // check for single player game
    if(singlePlayerGame) return;

    for(i=0;i<universe.numPlayers;i++)
    {
        // reset cost
        universe.players[i].initialShipCost=0;

        // calculate cost
        for(j=0;j<TOTAL_NUM_SHIPS;j++)
        {
            if(universe.gameStats.playerStats[i].totalForEachShip[j] > 0)
            {
                pInfo=GetShipStaticInfoSafe(j,universe.players[i].race);
                if(pInfo != NULL)
                {
                    universe.players[i].initialShipCost+=universe.gameStats.playerStats[i].totalForEachShip[j]*
                        pInfo->buildCost;
                }
            }
        }
    }

    return;
}

// this function returns the actual ship class based on ship type, -1 otherwise
sdword gameStatsGetShipClass(sdword nShipType)
{
    switch(nShipType)
    {
        case Carrier:
        case JunkYardHQ:
            return(CLASS_Carrier);

        case HeavyCorvette:
        case LightCorvette:
        case MinelayerCorvette:
        case MultiGunCorvette:
        case RepairCorvette:
        case SalCapCorvette:
        case P1MissileCorvette:
        case P1StandardCorvette:
        case JunkYardDawg:
            return(CLASS_Corvette);

        case MissileDestroyer:
        case StandardDestroyer:
        case P3Destroyer:
        case MiningBase:
            return(CLASS_Destroyer);

        case AttackBomber:
        case CloakedFighter:
        case DefenseFighter:
        case HeavyDefender:
        case HeavyInterceptor:
        case LightDefender:
        case LightInterceptor:
        case Drone:
        case P1Fighter:
        case P2AdvanceSwarmer:
        case P2Swarmer:
            return(CLASS_Fighter);

        case AdvanceSupportFrigate:
        case DDDFrigate:
        case DFGFrigate:
        case IonCannonFrigate:
        case StandardFrigate:
        case P1IonArrayFrigate:
        case P2FuelPod:
        case P2MultiBeamFrigate:
        case P3Frigate:
        case Junk_LGun:
        case Junk_SGun:
            return(CLASS_Frigate);

        case HeavyCruiser:
            return(CLASS_HeavyCruiser);

        case Mothership:
        case HeadShotAsteroid:
        case P1Mothership:
        case P2Mothership:
        case P3Megaship:
        case FloatingCity:
            return(CLASS_Mothership);

        case CloakGenerator:
        case GravWellGenerator:
        case Probe:
        case ProximitySensor:
        case ResearchShip:
        case ResourceController:
        case SensorArray:
        case TargetDrone:
        case CryoTray:
        case CargoBarge:
        case ResearchStation:
        case ResearchStationBridge:
        case ResearchStationTower:
        case Ghostship:
            return(CLASS_NonCombat);

        case ResourceCollector:
            return(CLASS_Resource);

        default:
            return(-1);
    }
}

#define isPlayerIndexCmptPlayer(i)  ((i) >= tpGameCreated.numPlayers)

// this function checks the bounds for RUs
bool verifyGameStatsResUnitsBounds(FILE *pStatsFile,sdword nPlayerNum,sdword nResourceUnits)
{
    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    if((nResourceUnits < 0) || (nResourceUnits >= 100000))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadResUnitsBounds));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the totals for RUs
bool verifyGameStatsResUnitsTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    sdword nStartingResources=0;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    switch(tpGameCreated.startingResources)
    {
        case MG_Small :
            nStartingResources=resourceStartSmall;
            break;
        case MG_Medium :
            nStartingResources=resourceStartMedium;
            break;
        case MG_Large :
            nStartingResources=resourceStartLarge;
            break;
    }

    if(universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsSpent > (
        universe.gameStats.playerStats[nPlayerNum].totalInjectedResources+
        universe.gameStats.playerStats[nPlayerNum].totalRegeneratedResourceUnits+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsCollected+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsGiven+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsRecieved+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsViaBounties+
        nStartingResources
        ))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadResUnitTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the bounds for the number of ships (based on the number of players)
bool verifyGameStatsShipsBounds(FILE *pStatsFile,sdword nPlayerNum,sdword nNumShips,sdword nNumPlayers)
{
    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    if((nNumShips < 0) || (nNumShips >= 1000*nNumPlayers))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipBounds));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the ship cost totals
bool verifyGameStatsShipCostTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    ShipStaticInfo *pInfo;

    sdword n,nShipTotal,nStartingResources=0;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    switch(tpGameCreated.startingResources)
    {
        case MG_Small :
            nStartingResources=resourceStartSmall;
            break;
        case MG_Medium :
            nStartingResources=resourceStartMedium;
            break;
        case MG_Large :
            nStartingResources=resourceStartLarge;
            break;
    }

    // reset total
    nShipTotal=-universe.players[nPlayerNum].initialShipCost;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n] > 0)
        {
            pInfo=GetShipStaticInfoSafe(n,universe.players[nPlayerNum].race);
            if(pInfo != NULL)
            {
                nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n]*
                    pInfo->buildCost;
            }
        }
    }

    // compare totals
    if(((nShipTotal - nShipTotal/50 ) > (
        universe.gameStats.playerStats[nPlayerNum].totalInjectedResources+
        universe.gameStats.playerStats[nPlayerNum].totalRegeneratedResourceUnits+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsCollected+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsGiven+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsRecieved+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsViaBounties+
        nStartingResources
        )) || ((nShipTotal - nShipTotal/50 ) > universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsSpent))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipCostTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the total number of ships acquired
bool verifyGameStatsShipsAcquiredTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    sdword n,nShipTotal,nClassTotal;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n] > 0)
        {
            nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n];
        }
    }

    // reset total
    nClassTotal=0;

    // calculate total
    for(n=0;n<NUM_CLASSES;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalForEachClass[n] > 0)
        {
            nClassTotal+=universe.gameStats.playerStats[nPlayerNum].totalForEachClass[n];
        }
    }

    // compare total
    if((universe.gameStats.playerStats[nPlayerNum].totalShips != nShipTotal) ||
        (universe.gameStats.playerStats[nPlayerNum].totalShips != nClassTotal))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsAcquiredTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the total number of ships killed
bool verifyGameStatsShipsKilledTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    sdword n,nShipTotal,nClassTotal;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalKillsForEachShip[n] > 0)
        {
            nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalKillsForEachShip[n];
        }
    }

    // reset total
    nClassTotal=0;

    // calculate total
    for(n=0;n<NUM_CLASSES;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalKillsForEachClass[n] > 0)
        {
            nClassTotal+=universe.gameStats.playerStats[nPlayerNum].totalKillsForEachClass[n];
        }
    }

    // compare total
    if((universe.gameStats.playerStats[nPlayerNum].totalKills != nShipTotal) ||
        (universe.gameStats.playerStats[nPlayerNum].totalKills != nClassTotal))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsKilledTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the total number of ships lost
bool verifyGameStatsShipsLostTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    sdword n,nShipTotal,nClassTotal;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalLossesForEachShip[n] > 0)
        {
            nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalLossesForEachShip[n];
        }
    }

    // reset total
    nClassTotal=0;

    // calculate total
    for(n=0;n<NUM_CLASSES;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalLossesForEachClass[n] > 0)
        {
            nClassTotal+=universe.gameStats.playerStats[nPlayerNum].totalLossesForEachClass[n];
        }
    }

    // compare total
    if((universe.gameStats.playerStats[nPlayerNum].totalLosses != nShipTotal) ||
        (universe.gameStats.playerStats[nPlayerNum].totalLosses != nClassTotal))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsLostTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the number of ships acquired per class
bool verifyGameStatsShipsAcquiredClassTotals(FILE *pStatsFile,sdword nPlayerNum,sdword nShipClass)
{
    sdword n,nShipTotal,nActualClass;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n] > 0)
        {
            // get actual ship class
            nActualClass=gameStatsGetShipClass(n);
            if((nActualClass >= 0) && (nActualClass == nShipClass))
            {
                nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n];
            }
        }
    }

    // compare totals
    if(nShipTotal != universe.gameStats.playerStats[nPlayerNum].totalForEachClass[nShipClass])
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsAcquiredClassTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the number of ships killed per class
bool verifyGameStatsShipsKilledClassTotals(FILE *pStatsFile,sdword nPlayerNum,sdword nShipClass)
{
    sdword n,nShipTotal,nActualClass;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalKillsForEachShip[n] > 0)
        {
            // get actual ship class
            nActualClass=gameStatsGetShipClass(n);
            if((nActualClass >= 0) && (nActualClass == nShipClass))
            {
                nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalKillsForEachShip[n];
            }
        }
    }

    // compare totals
    if(nShipTotal != universe.gameStats.playerStats[nPlayerNum].totalKillsForEachClass[nShipClass])
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsKilledClassTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the number of ships lost per class
bool verifyGameStatsShipsLostClassTotals(FILE *pStatsFile,sdword nPlayerNum,sdword nShipClass)
{
    sdword n,nShipTotal,nActualClass;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalLossesForEachShip[n] > 0)
        {
            // get actual ship class
            nActualClass=gameStatsGetShipClass(n);
            if((nActualClass >= 0) && (nActualClass == nShipClass))
            {
                nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalLossesForEachShip[n];
            }
        }
    }

    // compare totals
    if(nShipTotal != universe.gameStats.playerStats[nPlayerNum].totalLossesForEachClass[nShipClass])
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsLostClassTotals));
        return(FALSE);
    }

    return(TRUE);
}

// why are there no comments below?
void writeGameStatsToFile(char *filename)
{
    sdword i,j,startingresources=0;
    filehandle statsFH;
    FILE *statsFile = NULL;

    if (singlePlayerGame)
    {
        return;
    }

    statsFH = fileOpen(filename, FF_WriteMode | FF_TextMode | FF_ReturnNULLOnFail | FF_UserSettingsPath);
    if (!statsFH)
    {
        return;
    }
    dbgAssert(!fileUsingBigfile(statsFH));
    statsFile = fileStream(statsFH);

    if (statsFile)
    {
        if (randSyncErr)
        {
            fprintf(statsFile,"\nRAND SYNC ERR: Frame %d",randSyncErrFrame);
        }
        if (univSyncErr)
        {
            fprintf(statsFile,"\nUNIV SYNC ERR: Frame %d",univSyncErrFrame);
        }
        if (multiPlayerGame)        // only print cheat detect in multiplayer game
        {
            if (blobSyncErr)
            {
                fprintf(statsFile,"\n");
                fprintf(statsFile,strGetString(strCheatDetect),blobSyncErrFrame);
            }
        }
        if (pktSyncErr)
        {
            fprintf(statsFile,"\nPKT SYNC ERR");
        }

        for(i=0;i<universe.numPlayers;i++)
        {
            fprintf(statsFile,"\n*************");
            fprintf(statsFile,"\n*************");
            fprintf(statsFile,"\n");
            fprintf(statsFile,strGetString(strStatsPlayerName),playerNames[i]);
            fprintf(statsFile,"\n");
            fprintf(statsFile,strGetString(strStatsPlayerRace),ShipRaceToStr(universe.players[i].race));
            fprintf(statsFile,"\n___________________\n");

            if (universe.gameStats.playerStats[i].timeOfDeath)
            {
                fprintf(statsFile,"\n");
                fprintf(statsFile,strGetString(strStatsTimeOfDeath), universe.gameStats.playerStats[i].timeOfDeath);
                if (universe.gameStats.playerStats[i].typeOfPlayerDeath)
                {
                    fprintf(statsFile,"\n");
                    switch (universe.gameStats.playerStats[i].typeOfPlayerDeath)
                    {
                        case PLAYERKILLED_DROPPEDOUT    : fprintf(statsFile,strGetString(strStatsDeathByDropout));                  break;
                        case PLAYERKILLED_CAPTUREDSHIP  : fprintf(statsFile,strGetString(strStatsDeathByShipCapture));              break;
                        case PLAYERKILLED_LOSTMISSION   : fprintf(statsFile,strGetString(strStatsDeathByLossOfMission));            break;
                    }
                }
            }
            else
            {
                fprintf(statsFile,"\n");
                fprintf(statsFile,strGetString(strStatsDeathStillAlive));
            }

            switch(tpGameCreated.startingResources)
            {
                case MG_Small:
                    startingresources=resourceStartSmall;
                    break;
                case MG_Medium:
                    startingresources=resourceStartMedium;
                    break;
                case MG_Large:
                    startingresources=resourceStartLarge;
                    break;
            }

            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsStart)    ,startingresources);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,startingresources);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsCollected),universe.gameStats.playerStats[i].totalResourceUnitsCollected);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsCollected);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsSpent)    ,universe.gameStats.playerStats[i].totalResourceUnitsSpent);
                // check stats
                if(verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsSpent) == TRUE)
                    verifyGameStatsResUnitsTotals(statsFile,i);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsGiven)    ,universe.gameStats.playerStats[i].totalResourceUnitsGiven);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsGiven);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsReceived) ,universe.gameStats.playerStats[i].totalResourceUnitsRecieved);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsRecieved);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsInjected) ,universe.gameStats.playerStats[i].totalInjectedResources);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalInjectedResources);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsByBounties),universe.gameStats.playerStats[i].totalResourceUnitsViaBounties);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsViaBounties);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsGenerated),universe.gameStats.playerStats[i].totalRegeneratedResourceUnits);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalRegeneratedResourceUnits);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsCurrentRUs),universe.players[i].resourceUnits);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.players[i].resourceUnits);
            fprintf(statsFile,"\n___________________\n");

            for (j=0;j<universe.numPlayers;j++)
            {
                if (j!=i)
                {
                    fprintf(statsFile,"\n");
                    fprintf(statsFile, strGetString(strStatsTotalDmgAgainstPlayer), j, universe.gameStats.playerStats[i].pointsOfDamageDoneToWho[j]);
                }
            }

            fprintf(statsFile,"\n\n");
            fprintf(statsFile,strGetString(strStatsTotalAcquiredShips),universe.gameStats.playerStats[i].totalShips);
                // check stats
                if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalShips,1) == TRUE)
                    verifyGameStatsShipsAcquiredTotals(statsFile,i);
            fprintf(statsFile,"\n\n");
            fprintf(statsFile,strGetString(strStatsTotalAcqShipsByType));
            fprintf(statsFile,"\n");

            for(j=0;j<TOTAL_NUM_SHIPS;j++)
            {
                if(universe.gameStats.playerStats[i].totalForEachShip[j] > 0)
                {
                    fprintf(statsFile,"\n%-40s:%d",ShipTypeToNiceStr(j),universe.gameStats.playerStats[i].totalForEachShip[j]);
                        // check stats
                        verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalForEachShip[j],1);
                }
            }
            fprintf(statsFile,"\n___________________\n\n\n");
            fprintf(statsFile,strGetString(strStatsTotalAcqShipsByClass));
            for(j=0;j<NUM_CLASSES;j++)
            {
                if(universe.gameStats.playerStats[i].totalForEachClass[j] > 0)
                {
                    fprintf(statsFile,"\n%-40s:%d",ShipClassToNiceStr(j),universe.gameStats.playerStats[i].totalForEachClass[j]);
                        // check stats
                        if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalForEachClass[j],1) == TRUE)
                            verifyGameStatsShipsAcquiredClassTotals(statsFile,i,j);
                }
            }
            fprintf(statsFile,"\n___________________\n\n");

            fprintf(statsFile,strGetString(strStatsTotalKills),universe.gameStats.playerStats[i].totalKills);
                // check stats
                if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalKills,universe.numPlayers) == TRUE)
                    verifyGameStatsShipsKilledTotals(statsFile,i);
            for(j=0;j<TOTAL_NUM_SHIPS;j++)
            {
                if(universe.gameStats.playerStats[i].totalKillsForEachShip[j] > 0)
                {
                    fprintf(statsFile,"\n%s %s\t\t%d",ShipTypeToNiceStr(j),strGetString(strStatsKilled),universe.gameStats.playerStats[i].totalKillsForEachShip[j]);
                        // check stats
                        verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalKillsForEachShip[j],universe.numPlayers);
                }
            }
            for(j=0;j<NUM_CLASSES;j++)
            {
                if(universe.gameStats.playerStats[i].totalKillsForEachClass[j] > 0)
                {
                    fprintf(statsFile,"\n%s %s\t\t%d",ShipClassToNiceStr(j),strGetString(strStatsKilled),universe.gameStats.playerStats[i].totalKillsForEachClass[j]);
                        // check stats
                        if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalKillsForEachClass[j],universe.numPlayers) == TRUE)
                            verifyGameStatsShipsKilledClassTotals(statsFile,i,j);
                }
            }
            fprintf(statsFile,"\n___________________\n\n");

            fprintf(statsFile,strGetString(strStatsTotalLosses),universe.gameStats.playerStats[i].totalLosses);
                // check stats
                if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalLosses,universe.numPlayers) == TRUE)
                    verifyGameStatsShipsLostTotals(statsFile,i);
            for(j=0;j<TOTAL_NUM_SHIPS;j++)
            {
                if(universe.gameStats.playerStats[i].totalLossesForEachShip[j] > 0)
                {
                    fprintf(statsFile,"\n%s %s\t\t %d",ShipTypeToNiceStr(j),strGetString(strStatsLost),universe.gameStats.playerStats[i].totalLossesForEachShip[j]);
                        // check stats
                        verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalLossesForEachShip[j],universe.numPlayers);
                }
            }
            for(j=0;j<NUM_CLASSES;j++)
            {
                if(universe.gameStats.playerStats[i].totalLossesForEachClass[j] > 0)
                {
                    fprintf(statsFile,"\n%s %s\t\t %d",ShipClassToNiceStr(j),strGetString(strStatsLost),universe.gameStats.playerStats[i].totalLossesForEachClass[j]);
                        // check stats
                        if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalLossesForEachClass[j],universe.numPlayers) == TRUE)
                            verifyGameStatsShipsLostClassTotals(statsFile,i,j);
                }
            }
            fprintf(statsFile,"\n___________________\n");
            fprintf(statsFile,"\n___________________\n");
        }
    }

    fileClose(statsFH);
}
