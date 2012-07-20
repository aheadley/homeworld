/*=============================================================================
    Name    : Objtypes.h
    Purpose : Enumerations of object types, ships, race, class, etc.

    Created 7/8/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___OBJTYPES_H
#define ___OBJTYPES_H

#include "Types.h"
#include "ClassDefs.h"
#include "RaceDefs.h"
#include "ShipDefs.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef enum
{
    DockingCone,
    LatchPoint,
    LaunchPoint,
} DockPointType;

typedef enum
{
    Heading,
    Up,
    AttachPoint,
} SalvagePointType;

typedef enum
{
    GUN_Fixed,
    GUN_Gimble,
    GUN_NewGimble,
    GUN_MissileLauncher,
    GUN_MineLauncher
} GunType;

typedef enum
{
    NAVLIGHT_Default,
} NAVLightType;

typedef enum
{
    GS_LargeEnergyCannon,
    GS_LargeIonCannon,
    GS_LargePlasmaBomb,
    GS_LargeProjectile,
    GS_MediumEnergyCannon,
    GS_MediumIonCannon,
    GS_MediumPlasmaBomb,
    GS_MediumProjectile,
    GS_MineLauncher,
    GS_MissleLauncher,
    GS_SmallEnergyCannon,
    GS_SmallIonCannon,
    GS_SmallPlasmaBomb,
    GS_SmallProjectile,
    GS_VeryLargeEnergyCannon,
    GS_VeryLargeIonCannon,
    GS_VeryLargePlasmaBomb,
    GS_Laser,
    NUM_GUN_SOUND_TYPES
} GunSoundType;

typedef enum
{
    BULLET_Projectile,
    BULLET_PlasmaBomb,
    BULLET_Beam,
    BULLET_Laser,
    BULLET_SpecialBurst
} BulletType;

typedef enum
{
    MISSILE_Regular,
    MISSILE_Mine
} MissileType;

typedef enum
{
    OBJ_ShipType,
    OBJ_BulletType,
    OBJ_AsteroidType,
    OBJ_NebulaType,
    OBJ_GasType,
    OBJ_DustType,
    OBJ_DerelictType,
    OBJ_EffectType,
    OBJ_MissileType
} ObjType;

// CLASS enumerations   (changed to defines for speech tool in ClassDefs.h)
typedef udword ShipClass;

// RACE enumerations    (changed to defines for speech tool in RaceDefs.h)
typedef udword ShipRace;

#define RaceToRaceBits(race) (1<<(race))

// RACE bits
#define R1_VALID            (1<<R1)
#define R2_VALID            (1<<R2)
#define P1_VALID            (1<<P1)
#define P2_VALID            (1<<P2)
#define P3_VALID            (1<<P3)
#define Traders_VALID       (1<<Traders)

//special 'races' for different types of objects besides ships
#define NSR_Asteroid                (NUM_RACES + 0)
#define NSR_DustCloud               (NUM_RACES + 1)
#define NSR_GasCloud                (NUM_RACES + 2)
#define NSR_Nebula                  (NUM_RACES + 3)
#define NSR_Derelict                (NUM_RACES + 4)
#define NSR_Effect                  (NUM_RACES + 5)
#define NSR_Generic                 (NUM_RACES + 6)
#define NSR_Missile                 (NUM_RACES + 7)
#define NSR_LastNSR                 (NSR_Missile + 1)

typedef udword ShipType;

// moved ship defines to ShipDefs.h

typedef enum
{
    Asteroid0,
    Asteroid1,
    Asteroid2,
    Asteroid3,
    Asteroid4,
    NUM_ASTEROIDTYPES
} AsteroidType;

typedef enum
{
    DustCloud0,
    DustCloud1,
    DustCloud2,
    DustCloud3,
    NUM_DUSTCLOUDTYPES
} DustCloudType;

typedef enum
{
    GasCloud0,
    GasCloud1,
    NUM_GASCLOUDTYPES
} GasCloudType;

typedef enum
{
    Nebula0,
    NUM_NEBULATYPES
} NebulaType;

typedef enum
{
    AngelMoon,
    AngelMoon_clean,
    Crate,
    FragmentPanel0a,
    FragmentPanel0b,
    FragmentPanel0c,
    FragmentPanel1,
    FragmentPanel2,
    FragmentPanel3,
    FragmentStrut,
    Homeworld,
    LifeBoat,
    PlanetOfOrigin,
    PlanetOfOrigin_scarred,
    PrisonShipOld,
    PrisonShipNew,
    Scaffold,
    Scaffold_scarred,
    ScaffoldFingerA_scarred,
    ScaffoldFingerB_scarred,
    Shipwreck,
    //pre-revision ships as junkyard derelicts:
    JunkAdvanceSupportFrigate,
    JunkCarrier,
    JunkDDDFrigate,
    JunkHeavyCorvette,
    JunkHeavyCruiser,
    JunkIonCannonFrigate,
    JunkLightCorvette,
    JunkMinelayerCorvette,
    JunkMultiGunCorvette,
    JunkRepairCorvette,
    JunkResourceController,
    JunkSalCapCorvette,
    JunkStandardFrigate,
    //junk derelicts
    Junk0_antenna,
    Junk0_fin1,
    Junk0_fin2,
    Junk0_GunAmmo,
    Junk0_panel,
    Junk0_sensors,
    Junk1_partA,
    Junk1_partB,
    Junk1_shell,
    Junk1_strut,
    Junk2_panelA,
    Junk2_panelB,
    Junk2_panelC,
    Junk2_panelD,
    Junk2_shipwreck,
    Junk3_Boiler,
    Junk3_BoilerCasing,
    M13PanelA,
    M13PanelB,
    M13PanelC,
    //hyperspace gate dummy derelict
    HyperspaceGate,
    NUM_DERELICTTYPES
} DerelictType;

typedef enum
{
    ET_Explosion = 0,           //generic explosion
    ET_LightLaserHit,           //weapon impacts
    ET_MedLaserHit,
    ET_HeavyLaserHit,
    ET_PlasmaBombHit,

    ET_InterceptorExplosion,    //ship explosions
    ET_CorvetteExplosion,
    ET_FrigateExplosion,
    ET_CarrierExplosion,
    ET_ResourcerExplosion,

    NUM_EFFECT_TYPES            // must be last
}
EffectType;

typedef enum
{
    Evasive,
    Neutral,
    Aggressive,

    NUM_TACTICS_TYPES
}
TacticsType;

typedef enum
{
    Firepower = 0,
    Coverage,
    Maneuver,
    Armor,
    TopSpeed,
    Mass,
    VeryLow,
    Low,
    Medium,
    High,
    VeryHigh,
    CoverageUnits,

    NUM_SHIP_STATS
}
ShipStatsType;

typedef struct
{
    uword number;
    char *str;
} NumStrXlate;

typedef udword SpecialEffectType;
//special effect var defines
#define EGT_MINE_WALL_EFFECT        0
#define EGT_BURST_CHARING_EFFECT    1
#define EGT_BURST_MUZZLE_EFFECT     2
#define EGT_BURST_BULLET_EFFECT     3
#define EGT_BURST_HIT_EFFECT        4
#define EGT_BURST_EXPLOSION_EFFECT  5   //bullet explosion
#define EGT_CLOAK_ON                6
#define EGT_CLOAK_OFF               7
#define EGT_GRAVWELL_ON             8
#define EGT_GRAVWELL_OFF            9
#define EGT_LIGHTNING_GLOW         10
#define EGT_BULLET_DEFLECT         12
#define EGT_CRATE_GENERATED        13
#define EGT_CRATE_IS_FOUND_SHIP    14
#define EGT_CRATE_IS_FOUND_MONEY   15
#define EGT_CRATE_IS_FOUND_RESEARCH 16
#define EGT_CRATE_TIME_OUT         17
#define EGT_REPAIR_BEAM            18
#define EGT_CAUGHT_GRAVWELL        19
#define EGT_NumberOfSpecialEffects 20

/*=============================================================================
    Data:
=============================================================================*/

extern ShipType FirstShipTypeOfRace[NUM_RACES];
extern ShipType LastShipTypeOfRace[NUM_RACES];
extern uword NumShipTypesInRace[NUM_RACES];

/*=============================================================================
    Support Functions:
=============================================================================*/

char *NumToStr(NumStrXlate numstrtab[],uword num);
sdword StrToNum(NumStrXlate numstrtab[],char *str);

ShipRace RaceBitsToRace(uword raceBits);

char *ShipClassToStr(ShipClass shipclass);
char *ShipClassToNiceStr(ShipClass shipclass);
ShipClass StrToShipClass(char *str);

char *ShipRaceToStr(ShipRace shiprace);
ShipRace StrToShipRace(char *str);

char *TacticsTypeToStr(TacticsType tactics);
TacticsType StrToTacticsType(char *str);

SpecialEffectType StrToEffectNum(char *str);

char *ShipTypeToStr(ShipType shiptype);
char *ShipTypeToNiceStr(ShipType shiptype);
ShipType StrToShipType(char *str);

char *ShipStatToNiceStr(ShipStatsType stattype);
char *ShipTypeStatToNiceStr(ShipType shiptype, ShipStatsType stattype);

char *AsteroidTypeToStr(AsteroidType asteroidtype);
AsteroidType StrToAsteroidType(char *str);

char *DustCloudTypeToStr(DustCloudType dustcloudtype);
DustCloudType StrToDustCloudType(char *str);

char *GasCloudTypeToStr(GasCloudType gascloudtype);
GasCloudType StrToGasCloudType(char *str);

char *NebulaTypeToStr(NebulaType nebulatype);
NebulaType StrToNebulaType(char *str);

char *DerelictTypeToStr(DerelictType derelicttype);
DerelictType StrToDerelictType(char *str);

char *ExplosionTypeToStr(EffectType type);
EffectType StrToExplosionType(char *str);

char *GunTypeToStr(GunType guntype);
GunType StrToGunType(char *str);

DockPointType StrToDockPointType(char *str);
char *DockPointTypeToStr(DockPointType dockpointtype);

SalvagePointType StrToSalvagePointType(char *str);
char *SalvagePointTypeToStr(SalvagePointType salvagepointtype);

char *GunSoundTypeToStr(GunSoundType gunsoundtype);
GunSoundType StrToGunSoundType(char *str);

char *BulletTypeToStr(BulletType bulletType);
BulletType StrToBulletType(char *str);

char *MissileTypeToStr(MissileType missileType);
MissileType StrToMissileType(char *str);

char *NAVLightTypeToStr(NAVLightType navlightType);
NAVLightType StrToNAVLightType(char *str);

char *ObjTypeToStr(ObjType objtype);
ObjType StrToObjType(char *str);

char *NisRaceToStr(ShipRace race);
ShipRace StrToNisRace(char *string);

#endif

