/*=============================================================================
    Name    : Objtypes.c
    Purpose : Provides conversions for datatypes found in objtypes.h

    Created 6/24/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <strings.h>
#include "Types.h"
#include "StatScript.h"
#include "Strings.h"
#include "ObjTypes.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

/*=============================================================================
    Public Data:
=============================================================================*/

ShipType FirstShipTypeOfRace[NUM_RACES] =
{
    STD_FIRST_SHIP,
    STD_FIRST_SHIP,
    P1_FIRST_SHIP,
    P2_FIRST_SHIP,
    P3_FIRST_SHIP,
    TRADERS_FIRST_SHIP
};

ShipType LastShipTypeOfRace[NUM_RACES] =
{
    STD_LAST_SHIP,
    STD_LAST_SHIP,
    P1_LAST_SHIP,
    P2_LAST_SHIP,
    P3_LAST_SHIP,
    TRADERS_LAST_SHIP
};

uword NumShipTypesInRace[NUM_RACES] =
{
    TOTAL_STD_SHIPS,
    TOTAL_STD_SHIPS,
    TOTAL_P1_SHIPS,
    TOTAL_P2_SHIPS,
    TOTAL_P3_SHIPS,
    TOTAL_TRADERS_SHIPS
};

/*=============================================================================
    Private Data:
=============================================================================*/

static NumStrXlate shiptypeinfo[] =
{
    { (uword)AdvanceSupportFrigate, str$(AdvanceSupportFrigate) },
    { (uword)AttackBomber, str$(AttackBomber) },
    { (uword)Carrier, str$(Carrier) },
    { (uword)CloakedFighter, str$(CloakedFighter) },
    { (uword)CloakGenerator, str$(CloakGenerator) },
    { (uword)DDDFrigate, str$(DDDFrigate) },
    { (uword)DefenseFighter, str$(DefenseFighter) },
    { (uword)DFGFrigate, str$(DFGFrigate) },
    { (uword)GravWellGenerator, str$(GravWellGenerator) },
    { (uword)HeavyCorvette, str$(HeavyCorvette) },
    { (uword)HeavyCruiser, str$(HeavyCruiser) },
    { (uword)HeavyDefender, str$(HeavyDefender) },
    { (uword)HeavyInterceptor, str$(HeavyInterceptor) },
    { (uword)IonCannonFrigate, str$(IonCannonFrigate) },
    { (uword)LightCorvette, str$(LightCorvette) },
    { (uword)LightDefender, str$(LightDefender) },
    { (uword)LightInterceptor, str$(LightInterceptor) },
    { (uword)MinelayerCorvette, str$(MinelayerCorvette) },
    { (uword)MissileDestroyer, str$(MissileDestroyer) },
    { (uword)Mothership, str$(Mothership) },
    { (uword)MultiGunCorvette, str$(MultiGunCorvette) },
    { (uword)Probe, str$(Probe) },
    { (uword)ProximitySensor, str$(ProximitySensor) },
    { (uword)RepairCorvette, str$(RepairCorvette) },
    { (uword)ResearchShip, str$(ResearchShip) },
    { (uword)ResourceCollector, str$(ResourceCollector) },
    { (uword)ResourceController, str$(ResourceController) },
    { (uword)SalCapCorvette, str$(SalCapCorvette) },
    { (uword)SensorArray, str$(SensorArray) },
    { (uword)StandardDestroyer, str$(StandardDestroyer) },
    { (uword)StandardFrigate, str$(StandardFrigate) },
    { (uword)Drone, str$(Drone) },
    { (uword)TargetDrone, str$(TargetDrone) },
    { (uword)HeadShotAsteroid, str$(HeadShotAsteroid) },
    { (uword)CryoTray, str$(CryoTray) },
    { (uword)P1Fighter, str$(P1Fighter) },
    { (uword)P1IonArrayFrigate, str$(P1IonArrayFrigate) },
    { (uword)P1MissileCorvette, str$(P1MissileCorvette) },
    { (uword)P1Mothership, str$(P1Mothership) },
    { (uword)P1StandardCorvette, str$(P1StandardCorvette) },
    { (uword)P2AdvanceSwarmer, str$(P2AdvanceSwarmer) },
    { (uword)P2FuelPod, str$(P2FuelPod) },
    { (uword)P2Mothership, str$(P2Mothership) },
    { (uword)P2MultiBeamFrigate, str$(P2MultiBeamFrigate) },
    { (uword)P2Swarmer, str$(P2Swarmer) },
    { (uword)P3Destroyer, str$(P3Destroyer) },
    { (uword)P3Frigate, str$(P3Frigate) },
    { (uword)P3Megaship, str$(P3Megaship) },
    { (uword)FloatingCity, str$(FloatingCity) },
    { (uword)CargoBarge, str$(CargoBarge) },
    { (uword)MiningBase, str$(MiningBase) },
    { (uword)ResearchStation, str$(ResearchStation) },
    { (uword)JunkYardDawg, str$(JunkYardDawg) },
    { (uword)JunkYardHQ, str$(JunkYardHQ) },
    { (uword)Ghostship,  str$(Ghostship)},
    { (uword)Junk_LGun,  str$(Junk_LGun)},
    { (uword)Junk_SGun,  str$(Junk_SGun)},
    { (uword)ResearchStationBridge,  str$(ResearchStationBridge)},
    { (uword)ResearchStationTower,  str$(ResearchStationTower)},
    { 0,NULL }
};

static NumStrXlate shipraceinfo[] =
{
    { (uword)R1, str$(R1) },
    { (uword)R2, str$(R2) },
    { (uword)P1, str$(P1) },
    { (uword)P2, str$(P2) },
    { (uword)P3, str$(P3) },
    { (uword)Traders, str$(Traders) },
    { 0,NULL }
};

static NumStrXlate nisraceinfo[] =
{
    { (uword)R1, str$(R1) },
    { (uword)R2, str$(R2) },
    { (uword)P1, str$(P1) },
    { (uword)P2, str$(P2) },
    { (uword)P3, str$(P3) },
    { (uword)Traders, str$(Traders) },
    { (uword)NSR_Asteroid , "Asteroids" },
    { (uword)NSR_DustCloud, "DustClouds" },
    { (uword)NSR_GasCloud , "GasClouds" },
    { (uword)NSR_Nebula   , "Nebulae" },
    { (uword)NSR_Derelict , "Derelicts" },
    { (uword)NSR_Effect   , "ETG" },
    { (uword)NSR_Generic  , "Misc" },
    { (uword)NSR_Missile  , "Missile" },
    { 0,NULL }
};
static NumStrXlate shipclassinfo[] =
{
    { (uword)CLASS_Mothership,     str$(CLASS_Mothership) },
    { (uword)CLASS_HeavyCruiser,   str$(CLASS_HeavyCruiser) },
    { (uword)CLASS_Carrier,        str$(CLASS_Carrier) },
    { (uword)CLASS_Destroyer,      str$(CLASS_Destroyer) },
    { (uword)CLASS_Frigate,        str$(CLASS_Frigate) },
    { (uword)CLASS_Corvette,       str$(CLASS_Corvette) },
    { (uword)CLASS_Fighter,        str$(CLASS_Fighter) },
    { (uword)CLASS_Resource,       str$(CLASS_Resource) },
    { (uword)CLASS_NonCombat,      str$(CLASS_NonCombat) },
    { 0,NULL }
};

/*static NumStrXlate shipclassniceinfo[] =
{
    { (uword)CLASS_Mothership,     "MOTHERSHIP"},
    { (uword)CLASS_HeavyCruiser,   "CRUISER"},
    { (uword)CLASS_Carrier,        "CARRIER"},
    { (uword)CLASS_Destroyer,      "DESTROYER"},
    { (uword)CLASS_Frigate,        "FRIGATE"},
    { (uword)CLASS_Corvette,       "CORVETTE"},
    { (uword)CLASS_Fighter,        "FIGHTER"},
    { (uword)CLASS_Resource,       "RESOURCE"},
    { (uword)CLASS_NonCombat,      "NON-COMBAT"},
    { 0,NULL }
};*/

static NumStrXlate asteroidtypeinfo[] =
{
    { (uword)Asteroid0, str$(Asteroid0) },
    { (uword)Asteroid1, str$(Asteroid1) },
    { (uword)Asteroid2, str$(Asteroid2) },
    { (uword)Asteroid3, str$(Asteroid3) },
    { (uword)Asteroid4, str$(Asteroid4) },
    { 0,NULL }
};

static NumStrXlate dustcloudtypeinfo[] =
{
    { (uword)DustCloud0, str$(DustCloud0) },
    { (uword)DustCloud1, str$(DustCloud1) },
    { (uword)DustCloud2, str$(DustCloud2) },
    { (uword)DustCloud3, str$(DustCloud3) },
    { 0,NULL }
};

static NumStrXlate gascloudtypeinfo[] =
{
    { (uword)GasCloud0, str$(GasCloud0) },
    { (uword)GasCloud1, str$(GasCloud1) },
    { 0,NULL }
};

static NumStrXlate nebulatypeinfo[] =
{
    { (uword)Nebula0, str$(Nebula0) },
    { 0, NULL }
};

static NumStrXlate derelicttypeinfo[] =
{
    { (uword)AngelMoon,           str$(AngelMoon)},
    { (uword)AngelMoon_clean,     str$(AngelMoon_clean)},
    { (uword)Crate,               str$(Crate)},
    { (uword)FragmentPanel0a,     str$(FragmentPanel0a)},
    { (uword)FragmentPanel0b,     str$(FragmentPanel0b)},
    { (uword)FragmentPanel0c,     str$(FragmentPanel0c)},
    { (uword)FragmentPanel1,      str$(FragmentPanel1)},
    { (uword)FragmentPanel2,      str$(FragmentPanel2)},
    { (uword)FragmentPanel3,      str$(FragmentPanel3)},
    { (uword)FragmentStrut,       str$(FragmentStrut)},
    { (uword)Homeworld,           str$(Homeworld)},
    { (uword)LifeBoat,            str$(LifeBoat)},
    { (uword)PlanetOfOrigin,      str$(PlanetOfOrigin)},
    { (uword)PlanetOfOrigin_scarred, str$(PlanetOfOrigin_scarred)},
    { (uword)PrisonShipOld,       str$(PrisonShipOld)},
    { (uword)PrisonShipNew,       str$(PrisonShipNew)},
    { (uword)Scaffold,            str$(Scaffold)},
    { (uword)Scaffold_scarred,    str$(Scaffold_scarred)},
    { (uword)ScaffoldFingerA_scarred, str$(ScaffoldFingerA_scarred)},
    { (uword)ScaffoldFingerB_scarred, str$(ScaffoldFingerB_scarred)},
    { (uword)Shipwreck,           str$(Shipwreck)},
    //pre-revision ships as junkyard derelicts:
    { (uword)JunkAdvanceSupportFrigate,     "AdvanceSupportFrigate"},
    { (uword)JunkCarrier,                   "Carrier"},
    { (uword)JunkDDDFrigate,                "DDDFrigate"},
    { (uword)JunkHeavyCorvette,             "HeavyCorvette"},
    { (uword)JunkHeavyCruiser,              "HeavyCruiser"},
    { (uword)JunkIonCannonFrigate,          "IonCannonFrigate"},
    { (uword)JunkLightCorvette,             "LightCorvette"},
    { (uword)JunkMinelayerCorvette,         "MinelayerCorvette"},
    { (uword)JunkMultiGunCorvette,          "MultiGunCorvette"},
    { (uword)JunkRepairCorvette,            "RepairCorvette"},
    { (uword)JunkResourceController,        "ResourceController"},
    { (uword)JunkSalCapCorvette,            "SalCapCorvette"},
    { (uword)JunkStandardFrigate,           "StandardFrigate"},
    //junk derelicts
    { (uword)Junk0_antenna,                 "Junk0_antenna"},
    { (uword)Junk0_fin1,                    "Junk0_fin1"},
    { (uword)Junk0_fin2,                    "Junk0_fin2"},
    { (uword)Junk0_GunAmmo,                 "Junk0_GunAmmo"},
    { (uword)Junk0_panel,                   "Junk0_panel"},
    { (uword)Junk0_sensors,                 "Junk0_sensors"},
    { (uword)Junk1_partA,                   "Junk1_partA"},
    { (uword)Junk1_partB,                   "Junk1_partB"},
    { (uword)Junk1_shell,                   "Junk1_shell"},
    { (uword)Junk1_strut,                   "Junk1_strut"},
    { (uword)Junk2_panelA,                  "Junk2_panelA"},
    { (uword)Junk2_panelB,                  "Junk2_panelB"},
    { (uword)Junk2_panelC,                  "Junk2_panelC"},
    { (uword)Junk2_panelD,                  "Junk2_panelD"},
    { (uword)Junk2_shipwreck,               "Junk2_shipwreck"},
    { (uword)Junk3_Boiler,                  "Junk3_Boiler"},
    { (uword)Junk3_BoilerCasing,            "Junk3_BoilerCasing"},
    { (uword)M13PanelA,                     "M13PanelA"},
    { (uword)M13PanelB,                     "M13PanelB"},
    { (uword)M13PanelC,                     "M13PanelC"},
    { (uword)HyperspaceGate,                "HyperspaceGate"},
    { 0,NULL }
};

static NumStrXlate explosiontypeinfo[] =
{
    { ET_Explosion,           "ET_Explosion"           },

    { ET_LightLaserHit,       "ET_LightLaserHit"       },
    { ET_MedLaserHit,         "ET_MedLaserHit"         },
    { ET_HeavyLaserHit,       "ET_HeavyLaserHit"       },
    { ET_PlasmaBombHit,       "ET_PlasmaBombHit"       },

    { ET_InterceptorExplosion,"ET_InterceptorExplosion"},
    { ET_CorvetteExplosion,   "ET_CorvetteExplosion"   },
    { ET_FrigateExplosion,    "ET_FrigateExplosion"    },
    { ET_CarrierExplosion,    "ET_CarrierExplosion"    },
    { ET_ResourcerExplosion,  "ET_ResourcerExplosion"  },

    { 0,NULL }
};

static NumStrXlate guntypeinfo[] =
{
    { (uword)GUN_Fixed,  str$(GUN_Fixed) },
    { (uword)GUN_Gimble, str$(GUN_Gimble) },
    { (uword)GUN_NewGimble, str$(GUN_NewGimble) },
    { (uword)GUN_MissileLauncher, str$(GUN_MissileLauncher) },
    { (uword)GUN_MineLauncher, str$(GUN_MineLauncher) },
    { 0,NULL }
};

static NumStrXlate dockpointtypeinfo[] =
{
    { (uword)DockingCone, str$(DockingCone) },
    { (uword)LatchPoint, str$(LatchPoint) },
    { (uword)LaunchPoint, str$(LaunchPoint) },

    { 0,NULL }
};

static NumStrXlate salvagepointtypeinfo[] =
{
    { (uword)Heading, str$(Heading) },
    { (uword)Up, str$(Up) },
    { (uword)AttachPoint, str$(AttachPoint) },

    { 0,NULL }
};

static NumStrXlate navlighttypeinfo[] =
{
    { (uword)NAVLIGHT_Default, str$(NAVLIGHT_Default) },
    { 0,NULL }
};

static NumStrXlate gunsoundtypeinfo[] =
{
    { (uword)GS_LargeEnergyCannon, str$(GS_LargeEnergyCannon) },
    { (uword)GS_LargeIonCannon, str$(GS_LargeIonCannon) },
    { (uword)GS_LargePlasmaBomb, str$(GS_LargePlasmaBomb) },
    { (uword)GS_LargeProjectile, str$(GS_LargeProjectile) },
    { (uword)GS_MediumEnergyCannon, str$(GS_MediumEnergyCannon) },
    { (uword)GS_MediumIonCannon, str$(GS_MediumIonCannon) },
    { (uword)GS_MediumPlasmaBomb, str$(GS_MediumPlasmaBomb) },
    { (uword)GS_MediumProjectile, str$(GS_MediumProjectile) },
    { (uword)GS_MineLauncher, str$(GS_MineLauncher) },
    { (uword)GS_MissleLauncher, str$(GS_MissleLauncher) },
    { (uword)GS_SmallEnergyCannon, str$(GS_SmallEnergyCannon) },
    { (uword)GS_SmallIonCannon, str$(GS_SmallIonCannon) },
    { (uword)GS_SmallPlasmaBomb, str$(GS_SmallPlasmaBomb) },
    { (uword)GS_SmallProjectile, str$(GS_SmallProjectile) },
    { (uword)GS_VeryLargeEnergyCannon, str$(GS_VeryLargeEnergyCannon) },
    { (uword)GS_VeryLargeIonCannon, str$(GS_VeryLargeIonCannon) },
    { (uword)GS_VeryLargePlasmaBomb, str$(GS_VeryLargePlasmaBomb) },
    { (uword)GS_Laser, str$(GS_Laser) },
    { 0,NULL }
};

static NumStrXlate bullettypeinfo[] =
{
    { (uword)BULLET_Projectile, str$(BULLET_Projectile) },
    { (uword)BULLET_PlasmaBomb, str$(BULLET_PlasmaBomb) },
    { (uword)BULLET_Beam, str$(BULLET_Beam) },
    { 0,NULL }
};

static NumStrXlate missiletypeinfo[] =
{
    { (uword)MISSILE_Regular, str$(MISSILE_Regular) },
    { (uword)MISSILE_Mine, str$(MISSILE_Mine) },
    { 0,NULL }
};

static NumStrXlate objtypeinfo[] =
{
    { (uword)OBJ_ShipType, "Ship" },
    { (uword)OBJ_BulletType, "Bullet" },
    { (uword)OBJ_AsteroidType, "Asteroid" },
    { (uword)OBJ_NebulaType, "Nebula" },
    { (uword)OBJ_GasType, "GasCloud" },
    { (uword)OBJ_DustType, "DustCloud" },
    { (uword)OBJ_DerelictType, "Derelict" },
    { (uword)OBJ_EffectType, "Effect" },
    { (uword)OBJ_MissileType, "Missile" },
    { 0,NULL }
};

static NumStrXlate SpecialEffectinfo[] =
{
    { (uword)EGT_MINE_WALL_EFFECT, "EGT_MINE_WALL_EFFECT"},
    { (uword)EGT_BURST_CHARING_EFFECT, "EGT_BURST_CHARING_EFFECT"},
    { (uword)EGT_BURST_MUZZLE_EFFECT, "EGT_BURST_MUZZLE_EFFECT"},
    { (uword)EGT_BURST_BULLET_EFFECT, "EGT_BURST_BULLET_EFFECT"},
    { (uword)EGT_BURST_HIT_EFFECT, "EGT_BURST_HIT_EFFECT"},
    { (uword)EGT_BURST_EXPLOSION_EFFECT, "EGT_BURST_EXPLOSION_EFFECT"},
    { (uword)EGT_CLOAK_ON      , "EGT_CLOAK_ON"},
    { (uword)EGT_CLOAK_OFF     , "EGT_CLOAK_OFF"},
    { (uword)EGT_GRAVWELL_ON   , "EGT_GRAVWELL_ON"},
    { (uword)EGT_GRAVWELL_OFF  , "EGT_GRAVWELL_OFF"},
    { (uword)EGT_LIGHTNING_GLOW, "EGT_LIGHTNING_GLOW"},
    { (uword)EGT_BULLET_DEFLECT, "EGT_BULLET_DEFLECT"},
    { (uword)EGT_CRATE_GENERATED, "EGT_CRATE_GENERATED"},
    { (uword)EGT_CRATE_IS_FOUND_SHIP, "EGT_CRATE_IS_FOUND_SHIP"},
    { (uword)EGT_CRATE_IS_FOUND_MONEY, "EGT_CRATE_IS_FOUND_MONEY"},
    { (uword)EGT_CRATE_IS_FOUND_RESEARCH, "EGT_CRATE_IS_FOUND_RESEARCH"},
    { (uword)EGT_CRATE_TIME_OUT, "EGT_CRATE_TIME_OUT"},
    { (uword)EGT_REPAIR_BEAM, "EGT_REPAIR_BEAM"},
    { (uword)EGT_CAUGHT_GRAVWELL, "EGT_CAUGHT_GRAVWELL"},

    { 0,NULL }
};

static NumStrXlate tacticsinfo[] =
{
    { (uword)Evasive, str$(Evasive) },
    { (uword)Neutral, str$(Neutral) },
    { (uword)Aggressive, str$(Aggressive) },
    { 0,NULL }
};

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : RaceBitsToRace
    Description : Converts bit positions representing race (e.g. RACE1_VALID)
                  to race enumeration (e.g. R1)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
ShipRace RaceBitsToRace(uword raceBits)
{
    if (raceBits & R1_VALID)
    {
        return R1;
    }
    else if (raceBits & R2_VALID)
    {
        return R2;
    }
    else if (raceBits & P1_VALID)
    {
        return P1;
    }
    else if (raceBits & P2_VALID)
    {
        return P2;
    }
    else if (raceBits & P3_VALID)
    {
        return P3;
    }
    else if (raceBits & Traders_VALID)
    {
        return Traders;
    }

    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : NumToStr
    Description : Converts a DEFINE to a string
    Inputs      : numstrtab - table cross-referencing numbers and strings
                  num - number to convert
    Outputs     :
    Return      : pointer to converted string if found, NULL otherwise
----------------------------------------------------------------------------*/
char *NumToStr(NumStrXlate numstrtab[],uword num)
{
    uword i;
    NumStrXlate *curnumstr;

    for (i=0,curnumstr=numstrtab;curnumstr->str != NULL;i++,curnumstr++)
    {
        if (num == curnumstr->number)
        {
            return curnumstr->str;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : StrToNum
    Description : Converts a string to a DEFINE
    Inputs      : numstrtab - table cross-referencing numbers and strings
                  str - string to convert
    Outputs     :
    Return      : number if converted correctly, -1 otherwise
----------------------------------------------------------------------------*/
sdword StrToNum(NumStrXlate numstrtab[],char *str)
{
    sdword i;
    NumStrXlate *curnumstr;

    for (i=0,curnumstr=numstrtab;curnumstr->str != NULL;i++,curnumstr++)
    {
        if (strcasecmp(str,curnumstr->str) == 0)
        {
            return curnumstr->number;
        }
    }
    return -1;
}

/*=============================================================================
    The following functions convert between ShipClass, ShipRace, ShipType
    and the strings representing them and vice-versa.
=============================================================================*/

char *TacticsTypeToStr(TacticsType tactics)
{
    return NumToStr(tacticsinfo,(uword)tactics);
}

TacticsType StrToTacticsType(char *str)
{
    return StrToNum(tacticsinfo,str);
}

char *ShipClassToStr(ShipClass shipclass)
{
    return NumToStr(shipclassinfo,(uword)shipclass);
}

char *ShipClassToNiceStr(ShipClass shipclass)
{
    return(strGetString((uword) shipclass+strClassOffset));
}

char *ShipRaceToStr(ShipRace shiprace)
{
    return NumToStr(shipraceinfo,(uword)shiprace);
}

char *ShipTypeToStr(ShipType shiptype)
{
    return NumToStr(shiptypeinfo,(uword)shiptype);
}

char *ShipStatToNiceStr(ShipStatsType stattype)
{
    // There are 6 stat types for each ship type in the ship view stat window
    return(strGetString((uword) (stattype + strShipStatNameOffset)));
}

/*
char *ShipTypeStatToNiceStr(ShipType shiptype, ShipStatsType stattype)
{
    // There are 6 stat types for each ship type in the ship view stat window
    return(strGetString((uword) ((shiptype * 6 + stattype) + strShipStatOffset)));
}
*/

ShipRace StrToNisRace(char *string)
{
    return((ShipRace)StrToNum(nisraceinfo, string));
}

char *NisRaceToStr(ShipRace race)
{
    return(NumToStr(nisraceinfo, (uword)race));
}

char *ShipTypeToNiceStr(ShipType shiptype)
{
    return(strGetString((uword) shiptype + strShipOffset));
}

char *GunTypeToStr(GunType guntype)
{
    return NumToStr(guntypeinfo,(uword)guntype);
}

char *GunSoundTypeToStr(GunSoundType gunsoundtype)
{
    return NumToStr(gunsoundtypeinfo,(uword)gunsoundtype);
}

char *DockPointTypeToStr(DockPointType dockpointtype)
{
    return NumToStr(dockpointtypeinfo,(uword)dockpointtype);
}

char *SalvagePointTypeToStr(SalvagePointType salvagepointtype)
{
    return NumToStr(salvagepointtypeinfo,(uword)salvagepointtype);
}

char *ExplosionTypeToStr(EffectType type)
{
    return NumToStr(explosiontypeinfo,(uword)type);
}

char *AsteroidTypeToStr(AsteroidType asteroidtype)
{
    return NumToStr(asteroidtypeinfo,(uword)asteroidtype);
}

char *DustCloudTypeToStr(DustCloudType dustcloudtype)
{
    return NumToStr(dustcloudtypeinfo,(uword)dustcloudtype);
}

char *GasCloudTypeToStr(GasCloudType gascloudtype)
{
    return NumToStr(gascloudtypeinfo,(uword)gascloudtype);
}

char *NebulaTypeToStr(NebulaType nebulatype)
{
    return NumToStr(nebulatypeinfo, (uword)nebulatype);
}

char *DerelictTypeToStr(DerelictType derelicttype)
{
    return NumToStr(derelicttypeinfo,(uword)derelicttype);
}

char *BulletTypeToStr(BulletType bulletType)
{
    return NumToStr(bullettypeinfo,(uword)bulletType);
}

char *MissileTypeToStr(MissileType missileType)
{
    return NumToStr(missiletypeinfo,(uword)missileType);
}

char *NAVLightTypeToStr(NAVLightType navlightType)
{
    return(NumToStr(navlighttypeinfo, (uword)navlightType));
}

char *ObjTypeToStr(ObjType objtype)
{
    return(NumToStr(objtypeinfo,(uword)objtype));
}

ShipClass StrToShipClass(char *str)
{
    return StrToNum(shipclassinfo,str);
}

ShipRace StrToShipRace(char *str)
{
    return StrToNum(shipraceinfo,str);
}

SpecialEffectType StrToEffectNum(char *str)
{
    return StrToNum(SpecialEffectinfo,str);
}

ShipType StrToShipType(char *str)
{
    return StrToNum(shiptypeinfo,str);
}

AsteroidType StrToAsteroidType(char *str)
{
    return StrToNum(asteroidtypeinfo,str);
}

DustCloudType StrToDustCloudType(char *str)
{
    return StrToNum(dustcloudtypeinfo, str);
}

GasCloudType StrToGasCloudType(char *str)
{
    return StrToNum(gascloudtypeinfo, str);
}

NebulaType StrToNebulaType(char *str)
{
    return StrToNum(nebulatypeinfo, str);
}

DerelictType StrToDerelictType(char *str)
{
    return StrToNum(derelicttypeinfo,str);
}

EffectType StrToExplosionType(char *str)
{
    return StrToNum(explosiontypeinfo,str);
}

GunType StrToGunType(char *str)
{
    return StrToNum(guntypeinfo,str);
}

GunSoundType StrToGunSoundType(char *str)
{
    return StrToNum(gunsoundtypeinfo,str);
}

DockPointType StrToDockPointType(char *str)
{
    return StrToNum(dockpointtypeinfo,str);
}

SalvagePointType StrToSalvagePointType(char *str)
{
    return StrToNum(salvagepointtypeinfo,str);
}

BulletType StrToBulletType(char *str)
{
    return StrToNum(bullettypeinfo,str);
}

MissileType StrToMissileType(char *str)
{
    return StrToNum(missiletypeinfo,str);
}

NAVLightType StrToNAVLightType(char *str)
{
    return StrToNum(navlighttypeinfo, str);
}

ObjType StrToObjType(char *str)
{
    return StrToNum(objtypeinfo, str);
}

