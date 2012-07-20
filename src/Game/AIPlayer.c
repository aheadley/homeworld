/*=============================================================================
    Name    : AIPlayer.c
    Purpose : All Computer Player (AIPlayer) interfaces with game happen here

    Created 5/31/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdarg.h>
#include <string.h>
#include "Types.h"
#include "ObjTypes.h"
#include "AIPlayer.h"
#include "File.h"
#include "Universe.h"
#include "Select.h"
#include "AIFleetMan.h"
#include "AIAttackMan.h"
#include "AIDefenseMan.h"
#include "AIUtilities.h"
#include "AITeam.h"
#include "Stats.h"
#include "Randy.h"
#include "NIS.h"
#include "SaveGame.h"
#include "SinglePlayer.h"

#define AIPLAYER_LOG_FILE_NAME "aiplayerlog.txt"

bool aiplayerLogEnable = FALSE;
AIPlayer *aiCurrentAIPlayer;
uword aiIndex;

udword AIPLAYER_UPDATE_RATE[AI_NUM_LEVELS] = { 63, 31, 15 };
real32 ATTMAN_BUILDPRIORITY_RATIO = 0.3f;
real32 DEFMAN_BUILDPRIORITY_RATIO = 0.3f;
real32 RESMAN_BUILDPRIORITY_RATIO = 0.4f;        // these 3 numbers must add to 1.0

real32 AIPLAYER_OVERKILL_FACTOR = 1.4f;
udword AIPLAYER_BIGOTRY[MAX_MULTIPLAYER_PLAYERS] = {0,0,0,0,0,0,0,0};

ubyte  AIF_RESEARCH_DELAY[AI_NUM_LEVELS] = { 2, 1, 0};

//attackman tweak variables
//out of 255
udword AIA_ATTACK_FLEET_FAST_PROB[AI_NUM_LEVELS]     = { 130, 130, 130};
udword AIA_ATTACK_FLEET_FAST_RANGE[AI_NUM_LEVELS]    = { 50, 50, 50};
udword AIA_ATTACK_FLEET_GUARD_PROB[AI_NUM_LEVELS]    = { 120, 120, 120};
udword AIA_ATTACK_FLEET_GUARD_RANGE[AI_NUM_LEVELS]   = { 50, 50, 50};
udword AIA_ATTACK_FLEET_BIG_PROB[AI_NUM_LEVELS]      = { 100, 100, 100};
udword AIA_ATTACK_FLEET_BIG_RANGE[AI_NUM_LEVELS]     = { 50, 50, 50};
udword AIA_ATTACK_FLEET_HUGE_PROB[AI_NUM_LEVELS]     = { 0, 0, 70};
udword AIA_ATTACK_FLEET_HUGE_RANGE[AI_NUM_LEVELS]    = { 0, 0, 40};
udword AIA_TAKEOUT_TARGET_PROB[AI_NUM_LEVELS]        = { 100, 100, 100};
udword AIA_TAKEOUT_TARGET_RANGE[AI_NUM_LEVELS]       = { 50, 50, 50};
udword AIA_FANCY_TAKEOUT_TARGET_PROB[AI_NUM_LEVELS]  = { 50, 50, 50};
udword AIA_FANCY_TAKEOUT_TARGET_RANGE[AI_NUM_LEVELS] = { 40, 40, 40};
udword AIA_FIGHTER_STRIKE_PROB[AI_NUM_LEVELS]        = { 15, 15, 15};
udword AIA_FIGHTER_STRIKE_RANGE[AI_NUM_LEVELS]       = { 15, 15, 15};
udword AIA_CORVETTE_STRIKE_PROB[AI_NUM_LEVELS]       = { 100, 100, 100};
udword AIA_CORVETTE_STRIKE_RANGE[AI_NUM_LEVELS]      = { 60, 60, 60};
udword AIA_FRIGATE_STRIKE_PROB[AI_NUM_LEVELS]        = { 115, 115, 115};
udword AIA_FRIGATE_STRIKE_RANGE[AI_NUM_LEVELS]       = { 70, 70, 70};
udword AIA_HARASS_BIG_PROB[AI_NUM_LEVELS]            = { 60, 60, 60};
udword AIA_HARASS_BIG_RANGE[AI_NUM_LEVELS]           = { 30, 30, 30};
udword AIA_HARASS_SMALL_PROB[AI_NUM_LEVELS]          = { 0, 0, 0};
udword AIA_HARASS_SMALL_RANGE[AI_NUM_LEVELS]         = { 0, 0, 0};
udword AIA_CAPTURE_PROB[AI_NUM_LEVELS]               = { 80, 80, 80};
udword AIA_CAPTURE_RANGE[AI_NUM_LEVELS]              = { 40, 40, 40};
udword AIA_MINE_PROB[AI_NUM_LEVELS]                  = { 0, 0, 55};
udword AIA_MINE_RANGE[AI_NUM_LEVELS]                 = { 0, 0, 45};
udword AIA_CLOAKGEN_PROB[AI_NUM_LEVELS]              = { 0, 0, 55};
udword AIA_CLOAKGEN_RANGE[AI_NUM_LEVELS]             = { 0, 0, 35};
udword AIA_GRAVWELL_PROB[AI_NUM_LEVELS]              = { 0, 0, 55};
udword AIA_GRAVWELL_RANGE[AI_NUM_LEVELS]             = { 0, 0, 35};

//out of 100
udword AIA_KAMIKAZE_PROB[AI_NUM_LEVELS]   = { 0, 0, 25};
udword AIA_KAMIKAZE_RANGE[AI_NUM_LEVELS]  = { 0, 0, 20};

ubyte  AIA_NUM_ARMADA_TEAMS[AI_NUM_LEVELS] = { 0, 1, 2};

udword AIA_ENEMYBLOBS_PER_RECON      = 5;

//defenseman tweak variables
real32 AID_MOTHERSHIP_DEFENSE_FLEET_STRENGTH    = 1.4f;
udword AID_RESCON_ASF_DEFENDABLE_MULTIPLIER     = 2;
udword AID_SPHERE_OF_INFLUENCE_INTERVAL         = 15;
udword AID_ROVING_DEFENSE_LOW_COUNT_CUTOFF      = 3;
udword AID_ROVING_DEFENSE_LOW_COUNT_DIVISOR     = 4;
udword AID_ROVING_DEFENSE_HIGH_COUNT_DIVISOR    = 2;
udword AID_DEFEND_MOTHERSHIP_FUEL_LOW           = 5;
real32 AID_MOTHERSHIP_ATTACKER_FLEET_RADIUS     = 1000.0f;

//team tweaks
udword           AIT_TEAM_MOVE_DELAY[NUM_TEAMLEVEL_TYPES] = {2,1,1,0,0};
TypeOfFormation  AIT_DEFAULT_FORMATION           = BROAD_FORMATION;
real32           AIT_DEFTEAM_NEEDS_HELP_STRENGTH = 1.2f;

//order tweaks
udword           AIO_GUARD_SHIPS_NUM_CAPITAL        = 12;
udword           AIO_GUARD_SHIPS_NUM_NONCAPITAL     = 6;
udword           AIO_GUARD_SHIPS_FUEL_LOW           = 15;
TypeOfFormation  AIO_HARASS_INITIAL_MOVE_FORMATION  = DELTA_FORMATION;
udword           AIO_HARASS_FUEL_LOW                = 15;
udword           AIO_HARASS_NUMBERS_LOW             = 40;
udword           AIO_DEFMOTHERSHIP_FUEL_LOW         = 5;
udword           AIO_PATROL_FUEL_LOW                = 15;
udword           AIO_PATROL_NUMBERS_LOW             = 50;
udword           AIO_FASTROVING_FUEL_LOW            = 15;
udword           AIO_FASTROVING_NUMBERS_LOW         = 80;
udword           AIO_SLOWROVING_FUEL_LOW            = 15;
udword           AIO_SLOWROVING_NUMBERS_LOW         = 50;
TypeOfFormation  AIO_TOUT_MSHIP_FAST_TGRD_FORMATION = BROAD_FORMATION;
TacticsType      AIO_TOUT_MSHIP_FAST_TGRD_TACTICS   = Aggressive;
TypeOfFormation  AIO_TOUT_MSHIP_BIG_TGRD_FORMATION  = BROAD_FORMATION;
TacticsType      AIO_TOUT_MSHIP_BIG_TGRD_TACTICS    = Aggressive;
TypeOfFormation  AIO_TOUT_MSHIP_GUARD_TGRD_FORMATION= BROAD_FORMATION;
TacticsType      AIO_TOUT_MSHIP_GUARD_TGRD_TACTICS  = Aggressive;
TypeOfFormation  AIO_TOUT_TARG_WCUR_FORMATION       = WALL_FORMATION;
TacticsType      AIO_TOUT_TARG_WCUR_TACTICS         = Aggressive;
TypeOfFormation  AIO_TOUT_TARG_TGUARD_FORMATION     = BROAD_FORMATION;
TacticsType      AIO_TOUT_TARG_TGUARD_TACTICS       = Aggressive;
TypeOfFormation  AIO_TOUT_TARG_FANCY_TGRD_FORMATION = BROAD_FORMATION;
TacticsType      AIO_TOUT_TARG_FANCY_TGRD_TACTICS   = Aggressive;
udword           AIO_FIGHTER_KAMIKAZE_HEALTH        = 45;
udword           AIO_CORVETTE_KAMIKAZE_HEALTH       = 25;

//move and handler tweak variables
real32          AIH_GENERIC_GETTINGROCKED_STRENGTH  = 1.5f;
TypeOfFormation AIH_GENERIC_GETTINGROCKED_FORMATION = BROAD_FORMATION;
real32          AIM_DIST_MOVE_END                   = 1300.0f;
udword          AIM_SQUADRON_NUM_SHIPS              = 5;
real32          AIM_INTERCEPT_FINISH_DISTSQ         = 4000000.0f;
real32          AIM_INTERCEPT_RECALC_DISTSQ         = 2000000.0f;
//harass move tweaks
real32          AIM_MIN_HARASS_ATTACK_DIST_SQ       = 4000000.0f;
real32          AIM_HARASS_STANDOFF_DIST            = 16000.0f;
real32          AIM_HARASS_PROTECTION_RADIUS        = 2500.0f;
TypeOfFormation AIM_HARASS_MOVE_FORMATION           = DELTA3D_FORMATION;
TypeOfFormation AIM_HARASS_SINGLETARGET_FORMATION   = SPHERE_FORMATION;
TypeOfFormation AIM_HARASS_MULTITARGET_FORMATION    = CLAW_FORMATION;
TypeOfFormation AIH_HARASS_SINGLEATTACK_FORMATION   = SPHERE_FORMATION;
real32          AIH_HARASS_NUMLOW_STANDOFF_DIST     = 7500.0f;
TypeOfFormation AIH_HARASS_NUMLOW_FORMATION         = BROAD_FORMATION;
//flank move tweaks
real32          AIM_FLANK_ATTACK_RADIUS             = 16000.0f;
TypeOfFormation AIM_FLANK_MOVE_FORMATION            = WALL_FORMATION;
TypeOfFormation AIM_FLANK_ATTACK_FORMATION          = BROAD_FORMATION;
//dock move tweaks
udword          AIM_DOCK_REPAIRHEALTH_TRIGGER       = 80;
real32          AIM_DOCK_FINISH_FUEL_THRESHOLD      = 0.95f;
TypeOfFormation AIM_DOCK_DEFAULT_FORMATION          = SPHERE_FORMATION;
//defend mothership move tweaks
TypeOfFormation AIM_DEF_MOTHERSHIP_ATTACK_FORMATION = WALL_FORMATION;
//active patrol move tweaks
TypeOfFormation AIM_PATROL_FIRST_FORMATION          = DELTA_FORMATION;
TypeOfFormation AIM_PATROL_GENERAL_FORMATION        = DELTA3D_FORMATION;
real32          AIH_PATROL_ENEMYNEARBY_STRENGTH     = 1.5f;
TypeOfFormation AIH_PATROL_ENEMYNEARBY_FORMATION    = BROAD_FORMATION;
real32          AIH_FASTDEF_NUMLOW_STANDOFF_DIST    = 7500.0f;
TypeOfFormation AIH_FASTDEF_NUMLOW_FORMATION        = BROAD_FORMATION;
real32          AIH_FASTDEF_INVADER_ENEMYDIST       = 1500.0f;
udword          AIH_FASTDEF_INVADER_LOTSFEW_LIMIT   = 1;
TypeOfFormation AIH_FASTDEF_INVADER_LOTS_FORMATION  = CLAW_FORMATION;
TypeOfFormation AIH_FASTDEF_INVADER_FEW_FORMATION   = SPHERE_FORMATION;
real32          AIH_SLOWDEF_NUMLOW_STANDOFF_DIST    = 7000.0f;
TypeOfFormation AIH_SLOWDEF_NUMLOW_FORMATION        = BROAD_FORMATION;
real32          AIH_SLOWDEF_RESPONSE_MAXDISTSQ      = 100000000.0f;
real32          AIH_SLOWDEF_INVADER_ENEMYDIST       = 1500.0f;
udword          AIH_SLOWDEF_INVADER_LOTSFEW_LIMIT   = 1;
TypeOfFormation AIH_SLOWDEF_INVADER_LOTS_FORMATION  = BROAD_FORMATION;
TypeOfFormation AIH_SLOWDEF_INVADER_FEW_FORMATION   = SPHERE_FORMATION;
//temp guard move tweaks
real32          AIM_TEMP_GUARD_MOVE_LOCATION_MULT   = 0.6666f;
TypeOfFormation AIM_TEMP_GUARD_MOVE_FORMATION       = BROAD_FORMATION;
TacticsType     AIM_TEMP_GUARD_MOVE_TACTICS         = Aggressive;
TypeOfFormation AIM_TEMP_GUARD_INVADER_FORMATION    = BROAD_FORMATION;
//support move tweaks
real32          AIM_SUPPORT_SAFE_DISTANCE           = 16000;

//utilities tweak variables
real32          AIU_STANDOFF_NEG_ERROR_ANGLE        = -10.0f;
real32          AIU_STANDOFF_POS_ERROR_ANGLE        = 10.0f;
real32          AIU_STANDOFF_DIST_ERROR_LOW         = 0.95f;
real32          AIU_STANDOFF_DIST_ERROR_HIGH        = 1.05f;
udword          AIU_RESCUE_MULTIPLE_MINSHIPS        = 4;
udword          AIU_RESCUE_ENEMYMULTIPLE_MINSHIPS   = 2;
TypeOfFormation AIU_RESCUE_NOMULTIPLE_FORMATION     = BROAD_FORMATION;
TypeOfFormation AIU_RESCUE_TEAMMULTIPLE_FORMATION   = SPHERE_FORMATION;
TypeOfFormation AIU_RESCUE_BOTHMULTIPLE_FORMATION   = WALL_FORMATION;
real32          AIU_TAKEOUT_ENEMYFLEET_RADIUS       = 1500.0f;
udword          AIU_TAKEOUT_MULTIPLE_MINSHIPS       = 4;
udword          AIU_TAKEOUT_ENEMYMULTIPLE_MINSHIPS  = 2;
TypeOfFormation AIU_TAKEOUT_NOMULTIPLE_FORMATION    = BROAD_FORMATION;
TypeOfFormation AIU_TAKEOUT_TEAMMULTIPLE_FORMATION  = SPHERE_FORMATION;
TypeOfFormation AIU_TAKEOUT_BOTHMULTIPLE_FORMATION  = WALL_FORMATION;
real32          AIU_SAFEST_STANDOFF_DIST_MULT       = 2.2f;
real32          AIU_FINDUNARMED_PROTECTION_STRENGTH = 2.0f;
real32          AIU_FINDATTACKING_ENEMYFLEET_RADIUS = 1000.0f;
real32          AIU_HARASS_PROTECTION_RADIUS        = 1500.0f;
real32          AIU_UNARMED_UNDEFENDED_MODIFIER     = 1.0f;
real32          AIU_UNARMED_MODIFIER                = 1.2f;
real32          AIU_FIGHTER_VULNERABLE_MODIFIER     = 0.8f;

scriptEntry AIPlayerTweaks[] =
{
    makeEntry(AIPLAYER_UPDATE_RATE[AI_BEG], scriptSetUdwordCB),
    makeEntry(AIPLAYER_UPDATE_RATE[AI_INT], scriptSetUdwordCB),
    makeEntry(AIPLAYER_UPDATE_RATE[AI_ADV], scriptSetUdwordCB),
    makeEntry(ATTMAN_BUILDPRIORITY_RATIO,   scriptSetReal32CB),
    makeEntry(DEFMAN_BUILDPRIORITY_RATIO,   scriptSetReal32CB),
    makeEntry(RESMAN_BUILDPRIORITY_RATIO,   scriptSetReal32CB),
    makeEntry(AIPLAYER_OVERKILL_FACTOR,     scriptSetReal32CB),
    makeEntry(AIPLAYER_BIGOTRY[0],          scriptSetUdwordCB),
    makeEntry(AIPLAYER_BIGOTRY[1],          scriptSetUdwordCB),
    makeEntry(AIPLAYER_BIGOTRY[2],          scriptSetUdwordCB),
    makeEntry(AIPLAYER_BIGOTRY[3],          scriptSetUdwordCB),
    makeEntry(AIPLAYER_BIGOTRY[4],          scriptSetUdwordCB),
    makeEntry(AIPLAYER_BIGOTRY[5],          scriptSetUdwordCB),
    makeEntry(AIPLAYER_BIGOTRY[6],          scriptSetUdwordCB),
    makeEntry(AIPLAYER_BIGOTRY[7],          scriptSetUdwordCB),
    makeEntry(AIF_RESEARCH_DELAY[AI_BEG],   scriptSetUbyteCB),
    makeEntry(AIF_RESEARCH_DELAY[AI_INT],   scriptSetUbyteCB),
    makeEntry(AIF_RESEARCH_DELAY[AI_ADV],   scriptSetUbyteCB),

    //attackman tweaks
    makeEntry(AIA_ATTACK_FLEET_FAST_PROB[AI_BEG],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_FAST_RANGE[AI_BEG],   scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_FAST_PROB[AI_INT],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_FAST_RANGE[AI_INT],   scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_FAST_PROB[AI_ADV],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_FAST_RANGE[AI_ADV],   scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_GUARD_PROB[AI_BEG],   scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_GUARD_RANGE[AI_BEG],  scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_GUARD_PROB[AI_INT],   scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_GUARD_RANGE[AI_INT],  scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_GUARD_PROB[AI_ADV],   scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_GUARD_RANGE[AI_ADV],  scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_BIG_PROB[AI_BEG],     scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_BIG_RANGE[AI_BEG],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_BIG_PROB[AI_INT],     scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_BIG_RANGE[AI_INT],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_BIG_PROB[AI_ADV],     scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_BIG_RANGE[AI_ADV],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_HUGE_PROB[AI_BEG],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_HUGE_RANGE[AI_BEG],   scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_HUGE_PROB[AI_INT],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_HUGE_RANGE[AI_INT],   scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_HUGE_PROB[AI_ADV],    scriptSetUdwordCB),
    makeEntry(AIA_ATTACK_FLEET_HUGE_RANGE[AI_ADV],   scriptSetUdwordCB),
    makeEntry(AIA_TAKEOUT_TARGET_PROB[AI_BEG],       scriptSetUdwordCB),
    makeEntry(AIA_TAKEOUT_TARGET_RANGE[AI_BEG],      scriptSetUdwordCB),
    makeEntry(AIA_TAKEOUT_TARGET_PROB[AI_INT],       scriptSetUdwordCB),
    makeEntry(AIA_TAKEOUT_TARGET_RANGE[AI_INT],      scriptSetUdwordCB),
    makeEntry(AIA_TAKEOUT_TARGET_PROB[AI_ADV],       scriptSetUdwordCB),
    makeEntry(AIA_TAKEOUT_TARGET_RANGE[AI_ADV],      scriptSetUdwordCB),
    makeEntry(AIA_FANCY_TAKEOUT_TARGET_PROB[AI_BEG], scriptSetUdwordCB),
    makeEntry(AIA_FANCY_TAKEOUT_TARGET_RANGE[AI_BEG],scriptSetUdwordCB),
    makeEntry(AIA_FANCY_TAKEOUT_TARGET_PROB[AI_INT], scriptSetUdwordCB),
    makeEntry(AIA_FANCY_TAKEOUT_TARGET_RANGE[AI_INT],scriptSetUdwordCB),
    makeEntry(AIA_FANCY_TAKEOUT_TARGET_PROB[AI_ADV], scriptSetUdwordCB),
    makeEntry(AIA_FANCY_TAKEOUT_TARGET_RANGE[AI_ADV],scriptSetUdwordCB),
    makeEntry(AIA_FIGHTER_STRIKE_PROB[AI_BEG],       scriptSetUdwordCB),
    makeEntry(AIA_FIGHTER_STRIKE_RANGE[AI_BEG],      scriptSetUdwordCB),
    makeEntry(AIA_FIGHTER_STRIKE_PROB[AI_INT],       scriptSetUdwordCB),
    makeEntry(AIA_FIGHTER_STRIKE_RANGE[AI_INT],      scriptSetUdwordCB),
    makeEntry(AIA_FIGHTER_STRIKE_PROB[AI_ADV],       scriptSetUdwordCB),
    makeEntry(AIA_FIGHTER_STRIKE_RANGE[AI_ADV],      scriptSetUdwordCB),
    makeEntry(AIA_CORVETTE_STRIKE_PROB[AI_BEG],      scriptSetUdwordCB),
    makeEntry(AIA_CORVETTE_STRIKE_RANGE[AI_BEG],     scriptSetUdwordCB),
    makeEntry(AIA_CORVETTE_STRIKE_PROB[AI_INT],      scriptSetUdwordCB),
    makeEntry(AIA_CORVETTE_STRIKE_RANGE[AI_INT],     scriptSetUdwordCB),
    makeEntry(AIA_CORVETTE_STRIKE_PROB[AI_ADV],      scriptSetUdwordCB),
    makeEntry(AIA_CORVETTE_STRIKE_RANGE[AI_ADV],     scriptSetUdwordCB),
    makeEntry(AIA_FRIGATE_STRIKE_PROB[AI_BEG],       scriptSetUdwordCB),
    makeEntry(AIA_FRIGATE_STRIKE_RANGE[AI_BEG],      scriptSetUdwordCB),
    makeEntry(AIA_FRIGATE_STRIKE_PROB[AI_INT],       scriptSetUdwordCB),
    makeEntry(AIA_FRIGATE_STRIKE_RANGE[AI_INT],      scriptSetUdwordCB),
    makeEntry(AIA_FRIGATE_STRIKE_PROB[AI_ADV],       scriptSetUdwordCB),
    makeEntry(AIA_FRIGATE_STRIKE_RANGE[AI_ADV],      scriptSetUdwordCB),
    makeEntry(AIA_HARASS_BIG_PROB[AI_BEG],           scriptSetUdwordCB),
    makeEntry(AIA_HARASS_BIG_RANGE[AI_BEG],          scriptSetUdwordCB),
    makeEntry(AIA_HARASS_BIG_PROB[AI_INT],           scriptSetUdwordCB),
    makeEntry(AIA_HARASS_BIG_RANGE[AI_INT],          scriptSetUdwordCB),
    makeEntry(AIA_HARASS_BIG_PROB[AI_ADV],           scriptSetUdwordCB),
    makeEntry(AIA_HARASS_BIG_RANGE[AI_ADV],          scriptSetUdwordCB),
    makeEntry(AIA_HARASS_SMALL_PROB[AI_BEG],         scriptSetUdwordCB),
    makeEntry(AIA_HARASS_SMALL_RANGE[AI_BEG],        scriptSetUdwordCB),
    makeEntry(AIA_HARASS_SMALL_PROB[AI_INT],         scriptSetUdwordCB),
    makeEntry(AIA_HARASS_SMALL_RANGE[AI_INT],        scriptSetUdwordCB),
    makeEntry(AIA_HARASS_SMALL_PROB[AI_ADV],         scriptSetUdwordCB),
    makeEntry(AIA_HARASS_SMALL_RANGE[AI_ADV],        scriptSetUdwordCB),
    makeEntry(AIA_CAPTURE_PROB[AI_BEG],              scriptSetUdwordCB),
    makeEntry(AIA_CAPTURE_RANGE[AI_BEG],             scriptSetUdwordCB),
    makeEntry(AIA_CAPTURE_PROB[AI_INT],              scriptSetUdwordCB),
    makeEntry(AIA_CAPTURE_RANGE[AI_INT],             scriptSetUdwordCB),
    makeEntry(AIA_CAPTURE_PROB[AI_ADV],              scriptSetUdwordCB),
    makeEntry(AIA_CAPTURE_RANGE[AI_ADV],             scriptSetUdwordCB),
    makeEntry(AIA_MINE_PROB[AI_BEG],                 scriptSetUdwordCB),
    makeEntry(AIA_MINE_RANGE[AI_BEG],                scriptSetUdwordCB),
    makeEntry(AIA_MINE_PROB[AI_INT],                 scriptSetUdwordCB),
    makeEntry(AIA_MINE_RANGE[AI_INT],                scriptSetUdwordCB),
    makeEntry(AIA_MINE_PROB[AI_ADV],                 scriptSetUdwordCB),
    makeEntry(AIA_MINE_RANGE[AI_ADV],                scriptSetUdwordCB),
    makeEntry(AIA_CLOAKGEN_PROB[AI_BEG],             scriptSetUdwordCB),
    makeEntry(AIA_CLOAKGEN_RANGE[AI_BEG],            scriptSetUdwordCB),
    makeEntry(AIA_CLOAKGEN_PROB[AI_INT],             scriptSetUdwordCB),
    makeEntry(AIA_CLOAKGEN_RANGE[AI_INT],            scriptSetUdwordCB),
    makeEntry(AIA_CLOAKGEN_PROB[AI_ADV],             scriptSetUdwordCB),
    makeEntry(AIA_CLOAKGEN_RANGE[AI_ADV],            scriptSetUdwordCB),
    makeEntry(AIA_GRAVWELL_PROB[AI_BEG],             scriptSetUdwordCB),
    makeEntry(AIA_GRAVWELL_RANGE[AI_BEG],            scriptSetUdwordCB),
    makeEntry(AIA_GRAVWELL_PROB[AI_INT],             scriptSetUdwordCB),
    makeEntry(AIA_GRAVWELL_RANGE[AI_INT],            scriptSetUdwordCB),
    makeEntry(AIA_GRAVWELL_PROB[AI_ADV],             scriptSetUdwordCB),
    makeEntry(AIA_GRAVWELL_RANGE[AI_ADV],            scriptSetUdwordCB),

    makeEntry(AIA_KAMIKAZE_PROB[AI_BEG],    scriptSetUdwordCB),
    makeEntry(AIA_KAMIKAZE_RANGE[AI_BEG],   scriptSetUdwordCB),
    makeEntry(AIA_KAMIKAZE_PROB[AI_INT],    scriptSetUdwordCB),
    makeEntry(AIA_KAMIKAZE_RANGE[AI_INT],   scriptSetUdwordCB),
    makeEntry(AIA_KAMIKAZE_PROB[AI_ADV],    scriptSetUdwordCB),
    makeEntry(AIA_KAMIKAZE_RANGE[AI_ADV],   scriptSetUdwordCB),

    makeEntry(AIA_NUM_ARMADA_TEAMS[AI_BEG], scriptSetUbyteCB),
    makeEntry(AIA_NUM_ARMADA_TEAMS[AI_INT], scriptSetUbyteCB),
    makeEntry(AIA_NUM_ARMADA_TEAMS[AI_ADV], scriptSetUbyteCB),

    makeEntry(AIA_ENEMYBLOBS_PER_RECON,      scriptSetUdwordCB),

    //defenseman tweaks
    makeEntry(AID_MOTHERSHIP_DEFENSE_FLEET_STRENGTH,scriptSetReal32CB),
    makeEntry(AID_RESCON_ASF_DEFENDABLE_MULTIPLIER, scriptSetUdwordCB),
    makeEntry(AID_SPHERE_OF_INFLUENCE_INTERVAL,     scriptSetUdwordCB),
    makeEntry(AID_ROVING_DEFENSE_LOW_COUNT_CUTOFF,  scriptSetUdwordCB),
    makeEntry(AID_ROVING_DEFENSE_LOW_COUNT_DIVISOR, scriptSetUdwordCB),
    makeEntry(AID_ROVING_DEFENSE_HIGH_COUNT_DIVISOR,scriptSetUdwordCB),
    makeEntry(AID_DEFEND_MOTHERSHIP_FUEL_LOW,       scriptSetUdwordCB),
    makeEntry(AID_MOTHERSHIP_ATTACKER_FLEET_RADIUS, scriptSetReal32CB),

    //team tweaks
    makeEntry(AIT_TEAM_MOVE_DELAY[TEAM_BEGINNER],              scriptSetUdwordCB),
    makeEntry(AIT_TEAM_MOVE_DELAY[TEAM_BEGINNER_INTERMEDIATE], scriptSetUdwordCB),
    makeEntry(AIT_TEAM_MOVE_DELAY[TEAM_INTERMEDIATE],          scriptSetUdwordCB),
    makeEntry(AIT_TEAM_MOVE_DELAY[TEAM_INTERMEDIATE_ADVANCED], scriptSetUdwordCB),
    makeEntry(AIT_TEAM_MOVE_DELAY[TEAM_ADVANCED],              scriptSetUdwordCB),
    makeEntry(AIT_DEFAULT_FORMATION,           scriptSetFormationCB),
    makeEntry(AIT_DEFTEAM_NEEDS_HELP_STRENGTH, scriptSetReal32CB),

    //order tweaks
    makeEntry(AIO_GUARD_SHIPS_NUM_CAPITAL,        scriptSetUdwordCB),
    makeEntry(AIO_GUARD_SHIPS_NUM_NONCAPITAL,     scriptSetUdwordCB),
    makeEntry(AIO_GUARD_SHIPS_FUEL_LOW,           scriptSetUdwordCB),
    makeEntry(AIO_HARASS_INITIAL_MOVE_FORMATION,  scriptSetFormationCB),
    makeEntry(AIO_HARASS_FUEL_LOW,                scriptSetUdwordCB),
    makeEntry(AIO_HARASS_NUMBERS_LOW,             scriptSetUdwordCB),
    makeEntry(AIO_DEFMOTHERSHIP_FUEL_LOW,         scriptSetUdwordCB),
    makeEntry(AIO_PATROL_FUEL_LOW,                scriptSetUdwordCB),
    makeEntry(AIO_PATROL_NUMBERS_LOW,             scriptSetUdwordCB),
    makeEntry(AIO_FASTROVING_FUEL_LOW,            scriptSetUdwordCB),
    makeEntry(AIO_FASTROVING_NUMBERS_LOW,         scriptSetUdwordCB),
    makeEntry(AIO_SLOWROVING_FUEL_LOW,            scriptSetUdwordCB),
    makeEntry(AIO_SLOWROVING_NUMBERS_LOW,         scriptSetUdwordCB),
    makeEntry(AIO_TOUT_MSHIP_FAST_TGRD_FORMATION, scriptSetFormationCB),
    makeEntry(AIO_TOUT_MSHIP_FAST_TGRD_TACTICS,   scriptSetTacticsCB),
    makeEntry(AIO_TOUT_MSHIP_BIG_TGRD_FORMATION,  scriptSetFormationCB),
    makeEntry(AIO_TOUT_MSHIP_BIG_TGRD_TACTICS,    scriptSetTacticsCB),
    makeEntry(AIO_TOUT_MSHIP_GUARD_TGRD_FORMATION,scriptSetFormationCB),
    makeEntry(AIO_TOUT_MSHIP_GUARD_TGRD_TACTICS,  scriptSetTacticsCB),
    makeEntry(AIO_TOUT_TARG_WCUR_FORMATION,       scriptSetFormationCB),
    makeEntry(AIO_TOUT_TARG_WCUR_TACTICS,         scriptSetTacticsCB),
    makeEntry(AIO_TOUT_TARG_TGUARD_FORMATION,     scriptSetFormationCB),
    makeEntry(AIO_TOUT_TARG_TGUARD_TACTICS,       scriptSetTacticsCB),
    makeEntry(AIO_TOUT_TARG_FANCY_TGRD_FORMATION, scriptSetFormationCB),
    makeEntry(AIO_TOUT_TARG_FANCY_TGRD_TACTICS,   scriptSetTacticsCB),
    makeEntry(AIO_FIGHTER_KAMIKAZE_HEALTH,        scriptSetUdwordCB),
    makeEntry(AIO_CORVETTE_KAMIKAZE_HEALTH,       scriptSetUdwordCB),

    //move tweaks
    makeEntry(AIH_GENERIC_GETTINGROCKED_STRENGTH, scriptSetReal32CB),
    makeEntry(AIH_GENERIC_GETTINGROCKED_FORMATION,scriptSetFormationCB),
    makeEntry(AIM_DIST_MOVE_END,                  scriptSetReal32CB),
    makeEntry(AIM_SQUADRON_NUM_SHIPS,             scriptSetUdwordCB),
    makeEntry(AIM_INTERCEPT_FINISH_DISTSQ,        scriptSetReal32CB),
    makeEntry(AIM_INTERCEPT_RECALC_DISTSQ,        scriptSetReal32CB),
    //harass move tweaks
    makeEntry(AIM_MIN_HARASS_ATTACK_DIST_SQ,      scriptSetReal32CB),
    makeEntry(AIM_HARASS_STANDOFF_DIST,           scriptSetReal32CB),
    makeEntry(AIM_HARASS_PROTECTION_RADIUS,       scriptSetReal32CB),
    makeEntry(AIM_HARASS_MOVE_FORMATION,          scriptSetFormationCB),
    makeEntry(AIM_HARASS_SINGLETARGET_FORMATION,  scriptSetFormationCB),
    makeEntry(AIM_HARASS_MULTITARGET_FORMATION,   scriptSetFormationCB),
    makeEntry(AIH_HARASS_SINGLEATTACK_FORMATION,  scriptSetFormationCB),
    makeEntry(AIH_HARASS_NUMLOW_STANDOFF_DIST,    scriptSetReal32CB),
    makeEntry(AIH_HARASS_NUMLOW_FORMATION,        scriptSetFormationCB),
    //flank attack move tweaks
    makeEntry(AIM_FLANK_ATTACK_RADIUS,            scriptSetReal32CB),
    makeEntry(AIM_FLANK_MOVE_FORMATION,           scriptSetFormationCB),
    makeEntry(AIM_FLANK_ATTACK_FORMATION,         scriptSetFormationCB),
    //dock move tweaks
    makeEntry(AIM_DOCK_REPAIRHEALTH_TRIGGER,      scriptSetUdwordCB),
    makeEntry(AIM_DOCK_FINISH_FUEL_THRESHOLD,     scriptSetReal32CB),
    makeEntry(AIM_DOCK_DEFAULT_FORMATION,         scriptSetFormationCB),
    //defend mothership move tweaks
    makeEntry(AIM_DEF_MOTHERSHIP_ATTACK_FORMATION,scriptSetFormationCB),
    //active patrol move tweaks
    makeEntry(AIM_PATROL_FIRST_FORMATION,         scriptSetFormationCB),
    makeEntry(AIM_PATROL_GENERAL_FORMATION,       scriptSetFormationCB),
    makeEntry(AIH_PATROL_ENEMYNEARBY_STRENGTH,    scriptSetReal32CB),
    makeEntry(AIH_PATROL_ENEMYNEARBY_FORMATION,   scriptSetFormationCB),
    makeEntry(AIH_FASTDEF_NUMLOW_STANDOFF_DIST,   scriptSetReal32CB),
    makeEntry(AIH_FASTDEF_NUMLOW_FORMATION,       scriptSetFormationCB),
    makeEntry(AIH_FASTDEF_INVADER_ENEMYDIST,      scriptSetReal32CB),
    makeEntry(AIH_FASTDEF_INVADER_LOTSFEW_LIMIT,  scriptSetUdwordCB),
    makeEntry(AIH_FASTDEF_INVADER_LOTS_FORMATION, scriptSetFormationCB),
    makeEntry(AIH_FASTDEF_INVADER_FEW_FORMATION,  scriptSetFormationCB),
    makeEntry(AIH_SLOWDEF_NUMLOW_STANDOFF_DIST,   scriptSetReal32CB),
    makeEntry(AIH_SLOWDEF_NUMLOW_FORMATION,       scriptSetFormationCB),
    makeEntry(AIH_SLOWDEF_RESPONSE_MAXDISTSQ,     scriptSetReal32CB),
    makeEntry(AIH_SLOWDEF_INVADER_ENEMYDIST,      scriptSetReal32CB),
    makeEntry(AIH_SLOWDEF_INVADER_LOTSFEW_LIMIT,  scriptSetUdwordCB),
    makeEntry(AIH_SLOWDEF_INVADER_LOTS_FORMATION, scriptSetFormationCB),
    makeEntry(AIH_SLOWDEF_INVADER_FEW_FORMATION,  scriptSetFormationCB),
    //temp guard move tweaks
    makeEntry(AIM_TEMP_GUARD_MOVE_LOCATION_MULT,  scriptSetReal32CB),
    makeEntry(AIM_TEMP_GUARD_MOVE_FORMATION,      scriptSetFormationCB),
    makeEntry(AIM_TEMP_GUARD_MOVE_TACTICS,        scriptSetTacticsCB),
    makeEntry(AIM_TEMP_GUARD_INVADER_FORMATION,   scriptSetFormationCB),
    //support move tweaks
    makeEntry(AIM_SUPPORT_SAFE_DISTANCE,          scriptSetReal32CB),

    //utility tweak variables
    makeEntry(AIU_STANDOFF_NEG_ERROR_ANGLE,       scriptSetReal32CB),
    makeEntry(AIU_STANDOFF_POS_ERROR_ANGLE,       scriptSetReal32CB),
    makeEntry(AIU_STANDOFF_DIST_ERROR_LOW,        scriptSetReal32CB),
    makeEntry(AIU_STANDOFF_DIST_ERROR_HIGH,       scriptSetReal32CB),
    makeEntry(AIU_RESCUE_MULTIPLE_MINSHIPS,       scriptSetUdwordCB),
    makeEntry(AIU_RESCUE_ENEMYMULTIPLE_MINSHIPS,  scriptSetUdwordCB),
    makeEntry(AIU_RESCUE_NOMULTIPLE_FORMATION,    scriptSetFormationCB),
    makeEntry(AIU_RESCUE_TEAMMULTIPLE_FORMATION,  scriptSetFormationCB),
    makeEntry(AIU_RESCUE_BOTHMULTIPLE_FORMATION,  scriptSetFormationCB),
    makeEntry(AIU_TAKEOUT_ENEMYFLEET_RADIUS,      scriptSetReal32CB),
    makeEntry(AIU_TAKEOUT_MULTIPLE_MINSHIPS,      scriptSetUdwordCB),
    makeEntry(AIU_TAKEOUT_ENEMYMULTIPLE_MINSHIPS, scriptSetUdwordCB),
    makeEntry(AIU_TAKEOUT_NOMULTIPLE_FORMATION,   scriptSetFormationCB),
    makeEntry(AIU_TAKEOUT_TEAMMULTIPLE_FORMATION, scriptSetFormationCB),
    makeEntry(AIU_TAKEOUT_BOTHMULTIPLE_FORMATION, scriptSetFormationCB),
    makeEntry(AIU_SAFEST_STANDOFF_DIST_MULT,      scriptSetReal32CB),
    makeEntry(AIU_FINDUNARMED_PROTECTION_STRENGTH,scriptSetReal32CB),
    makeEntry(AIU_FINDATTACKING_ENEMYFLEET_RADIUS,scriptSetReal32CB),
    makeEntry(AIU_HARASS_PROTECTION_RADIUS,       scriptSetReal32CB),
    makeEntry(AIU_UNARMED_UNDEFENDED_MODIFIER,    scriptSetReal32CB),
    makeEntry(AIU_UNARMED_MODIFIER,               scriptSetReal32CB),
    makeEntry(AIU_FIGHTER_VULNERABLE_MODIFIER,    scriptSetReal32CB),

    endEntry
};




/*-----------------------------------------------------------------------------
    Name        : aiplayerChooseEnemies
    Description : Fills in the universe.aiplayerEnemy array so that each computer player
                  has a primary enemy.  Sorry about the sheer massiveness of this
                  function.  It just got out of hand.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aiplayerChooseEnemies(udword num_players, udword num_human, udword num_comp)
{
    sdword players[MAX_MULTIPLAYER_PLAYERS][MAX_MULTIPLAYER_PLAYERS],
           bigots[MAX_MULTIPLAYER_PLAYERS],
           normcomp[MAX_MULTIPLAYER_PLAYERS];
    udword bigotcnt = 0, normcnt = 0;
    udword numbigotsperhuman, numbigotsleftover, numloops, numcompenemies;
    udword slot, i, j;
    sdword giveup;

    if ((!num_comp) || (noDefaultComputerPlayer))
    {
        universe.aiplayerEnemy[0] = 1;
        universe.aiplayerEnemy[1] = 0;
        return;
    }

    dbgAssert(num_players == num_human+num_comp);

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        for (j=0;j<MAX_MULTIPLAYER_PLAYERS;j++)
        {
            players[i][j] = -1;
        }
        bigots[i]   = -1;
        normcomp[i] = -1;
    }

    //i denotes the computer player's playerIndex
    for (i=num_human; i<num_players;i++)
    {
        udword bigotchance = randyrandombetween(RAN_AIPlayer, 0, 100);

        if ((num_human) && (bigotchance < AIPLAYER_BIGOTRY[i-num_human]))
        {
            bigots[bigotcnt++] = i;
        }
        else
        {
            normcomp[normcnt++] = i;
        }
    }

    //find all the enemies for normal computer players
    for (i=0;i<normcnt;i++)
    {
        //choose a random player as enemy
        udword enemy = randyrandombetween(RAN_AIPlayer, 0, (num_players-1));
        bool done    = FALSE;
        giveup = 0;

        while (!done)
        {
            if (giveup++ > 100) { aiplayerLog((0,"Warning: giving up on loop 1")); break; }
            //if the player has no enemy
            if ((enemy != normcomp[i]) && (players[enemy][0] == -1))
            {
                //the player becomes that player's enemy
                players[enemy][0] = normcomp[i];
                done = TRUE;
            }
            else
            {
                //go to the next player
                enemy++;

                //loop around if at end of list
                if (enemy == num_players)
                {
                    enemy = 0;
                }
            }
        }

        //Note: each (human or computer) player should have only 1 normal computer player as enemy
    }

    //get some stats on the bigots
//    num_comp_per_human = num_comp/num_human;
//    num_comp_leftover  = num_comp%num_human;

    if (!num_human)
    {
        //want to avoid divide by zero
        //if there aren't any human players, then bigotry is irrelevant
        goto transfer_aiplayerEnemy;
    }

    numbigotsperhuman = bigotcnt/num_human;
    numbigotsleftover = bigotcnt%num_human;
    numloops = bigotcnt-numbigotsleftover;

    //determine how many enemy computer players each human player can have
    if (normcnt)
    {
        numcompenemies = numbigotsperhuman+1;
    }
    else
    {
        numcompenemies = numbigotsperhuman;
    }


    //distribute the bigot computers among available human players
    //numloops is the amount of bigot computer players per human
    for (i=0;i<numloops;i++)
    {
        udword enemy = randyrandombetween(RAN_AIPlayer, 0, (num_human-1));
        bool done = FALSE;
        giveup = 0;

        slot  = 0;

        while (!done)
        {
            if (giveup++ > 100) { aiplayerLog((0,"Warning: giving up on loop 2")); break; }
            if (players[enemy][slot] == -1)
            {
                players[enemy][slot] = bigots[i];
                done = TRUE;
            }
            else
            {
                slot++;

                if (slot == numcompenemies)
                {
                    enemy++;
                    slot = 0;

                    if (enemy == num_human)
                    {
                        enemy = 0;
                    }
                }
            }
        }
    }

    numcompenemies++;

    //distribute extra bigot computer players among human players
    for (i=0;i<numbigotsleftover;i++)
    {
        udword enemy = randyrandombetween(RAN_AIPlayer, 0, (num_human-1));
        udword slot = 0;
        bool done = FALSE;
        giveup = 0;

        while (!done)
        {
            if (giveup++ > 100) { aiplayerLog((0,"Warning: giving up on loop 3")); break; }
            if (players[enemy][slot] == -1)
            {
                players[enemy][slot] = bigots[i+(num_human*numloops)];
                done = TRUE;
            }
            else
            {
                slot++;

                if (slot == numcompenemies)
                {
                    enemy++;
                    slot = 0;

                    if (enemy == num_human)
                    {
                        enemy = 0;
                    }
                }
            }
        }
    }

transfer_aiplayerEnemy:

    //initialize enemyplayer array
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        universe.aiplayerEnemy[i] = -1;
    }

    //give everyone an enemy
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        slot = 0;

        while (players[i][slot] != -1)
        {
            universe.aiplayerEnemy[players[i][slot]] = i;
            slot++;
        }
    }

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        aiplayerLog(((uword)i, "My aiplayer Enemy = %i", universe.aiplayerEnemy[i]));
    }
}



void aiplayerStartup(udword num_players, udword num_human_players, udword num_comp_players)
{
    if (aiplayerLogEnable)
    {
        logfileClear(AIPLAYER_LOG_FILE_NAME);
    }
    aivarStartup();

    if (!determCompPlayer)
        ranRandomize(RAN_AIPlayer);     // randomize the AIPlayer random number stream
    else
        ranParametersReset(RAN_AIPlayer);

    aiplayerChooseEnemies(num_players, num_human_players, num_comp_players);
}

void aiplayerShutdown()
{
    // ONLY close global module stuff such as log files here
    aivarShutdown();
}

void aiplayerDebugLog(uword playerIndex, char *format, ...)
{
    if (aiplayerLogEnable)
    {
        char buffer[200];
        char buffer2[200];
        va_list argList;
        va_start(argList, format);                              //get first arg
        vsprintf(buffer, format, argList);                      //prepare output string
        va_end(argList);

        //find a way to put playernum in as well
        sprintf(buffer2,"\nAI%i: %0.1f %s", playerIndex, universe.totaltimeelapsed, buffer);

        logfileLog(AIPLAYER_LOG_FILE_NAME,buffer2);
        dbgMessage(buffer2);
    }
}

AIPlayer *aiplayerInit(Player *player,AIPlayerLevel aiplayerLevel)
{
    udword i;
    AIPlayer *aiplayer = memAlloc(sizeof(AIPlayer),"AIPlayer",0);
    memset(aiplayer,0,sizeof(AIPlayer));

    aiplayer->player = player;
    dbgAssert(aiplayerLevel < AI_NUM_LEVELS);
    aiplayer->aiplayerDifficultyLevel = aiplayerLevel;

    aiplayer->aiplayerUpdateRate = AIPLAYER_UPDATE_RATE[aiplayerLevel];

    // initialize other stuff here
    aiplayer->aiplayerFrameCount = 0;
    aiplayer->enemyPlayerCount   = 0;
    aiplayer->primaryEnemyPlayer = NULL;
    growSelectInit(&aiplayer->newships);

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        growSelectInit(&aiplayer->enemyShipsIAmAwareOf[i]);
        growSelectInit(&aiplayer->primaryEnemyShipsIAmAwareOf[i]);
    }

    //initialize hyperspacing stuff
    aiplayer->aifHyperSavings  = 0;
    aiplayer->aifHyperSkimming = 0;
    aiplayer->aifLastRUCount   = aiplayer->player->resourceUnits;

    aiplayer->NumRUDockPoints            = 0;
    aiplayer->NumRCollectorsBeingBuilt   = 0;
    aiplayer->NumRControllersBeingBuilt  = 0;
    aiplayer->NumASFBeingBuilt           = 0;
    aiplayer->NumResearchShipsBeingBuilt = 0;

    aiplayer->NumRUsSpentOnAttman      = 0;
    aiplayer->NumRUsSpentOnDefman      = 0;
    aiplayer->NumRUsSpentOnResourceman = 0;
    aiplayer->NumRUsSpentOnScriptman   = 0;

    listInit(&aiplayer->AttackManRequestShipsQ);
    listInit(&aiplayer->DefenseManRequestShipsQ);
    listInit(&aiplayer->ScriptManRequestShipsQ);
    listInit(&aiplayer->AttackManTeamsWaitingForShipsQ);
    listInit(&aiplayer->DefenseManTeamsWaitingForShipsQ);
    listInit(&aiplayer->ScriptManTeamsWaitingForShipsQ);
    memset(&aiplayer->AttackManShipsBeingBuilt,0,sizeof(aiplayer->AttackManShipsBeingBuilt));
    memset(&aiplayer->DefenseManShipsBeingBuilt,0,sizeof(aiplayer->DefenseManShipsBeingBuilt));
    memset(&aiplayer->ScriptManShipsBeingBuilt,0,sizeof(aiplayer->ScriptManShipsBeingBuilt));

    aiplayer->numLeaders = 0;

    aifInit(aiplayer);
    airInit(aiplayer);
    aiaInit(aiplayer);
    aidInit(aiplayer);
    aitInit(aiplayer);

    return aiplayer;
}

void aiplayerClose(AIPlayer *aiplayer)
{
    udword i;

    aiCurrentAIPlayer = aiplayer;

    aiplayerLog((aiplayer->player->playerIndex,"Deleting AIPlayer %x",aiplayer));

    growSelectClose(&aiplayer->newships);

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        growSelectClose(&aiplayer->enemyShipsIAmAwareOf[i]);
        growSelectClose(&aiplayer->primaryEnemyShipsIAmAwareOf[i]);
    }

    listDeleteAll(&aiplayer->AttackManRequestShipsQ);
    listDeleteAll(&aiplayer->DefenseManRequestShipsQ);
    listDeleteAll(&aiplayer->ScriptManRequestShipsQ);
    listDeleteAll(&aiplayer->AttackManTeamsWaitingForShipsQ);
    listDeleteAll(&aiplayer->DefenseManTeamsWaitingForShipsQ);
    listDeleteAll(&aiplayer->ScriptManTeamsWaitingForShipsQ);

    aifClose(aiplayer);
    airClose(aiplayer);
    aiaClose(aiplayer);
    aidClose(aiplayer);
    aitClose(aiplayer);

    memFree(aiplayer);
}

void aiplayerPlay(AIPlayer *aiplayer)
{
//    if (!singlePlayerGame)
        aiCurrentAIPlayer = aiplayer;

    universe.aiplayerProcessing = TRUE;

    if (!nisIsRunning)
    {
        statsSetOverkillfactor(AIPLAYER_OVERKILL_FACTOR);
        aifFleetCommand();
        aiplayer->aiplayerFrameCount++;
    }

    if (!singlePlayerGame)
        aiCurrentAIPlayer = NULL;

    universe.aiplayerProcessing = FALSE;
}

extern bool mrNoAI;

void aiplayerUpdateAll(void)
{
    uword i;
    AIPlayer *aiplayer;

    if (gameIsRunning)
    {
        if (singlePlayerGame)
        {
            if (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE)
            {
                return;
            }

            if (tutorial!=TUTORIAL_ONLY)
            {
                aiCurrentAIPlayer = universe.players[1].aiPlayer;
                dbgAssert(aiCurrentAIPlayer);
                if ((universe.univUpdateCounter & aiCurrentAIPlayer->aiplayerUpdateRate) == (SP_KAS_UPDATE_FRAME & aiCurrentAIPlayer->aiplayerUpdateRate))
                {
                    universe.aiplayerProcessing = TRUE;
                    kasExecute();
                    universe.aiplayerProcessing = FALSE;
                }
            }
        }

        if (!playPackets)
        {
            for (i=0;i<universe.numPlayers;i++)
            {
                if ((aiplayer = universe.players[i].aiPlayer) != NULL)
                {
                    if (((universe.univUpdateCounter & aiplayer->aiplayerUpdateRate) == (i & aiplayer->aiplayerUpdateRate)) &&
                        (aiplayer->player->playerState != PLAYER_DEAD) &&
                        (!mrNoAI))
                    {
                        aiplayerPlay(aiplayer);
                    }
                }
            }
        }

        if(tutorial==TUTORIAL_ONLY)
        {
            // In the tutorial, execute Kas script every other frame - we want REALTIME Kas.
            if( (universe.univUpdateCounter & TUTORIAL_KAS_UPDATE_MASK) == TUTORIAL_KAS_UPDATE_FRAME)
                kasExecute();
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ThisAIPlayerShipDiedCB(AIPlayer *aiplayer,Ship *ship)
{
    if (ship->staticinfo->canReceiveResources)
    {
        aiplayer->NumRUDockPoints -= NumRUDockPointsOnThisShip(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ThisAIPlayerShipCreatedCB(AIPlayer *aiplayer,Ship *ship)
{
    if (ship->staticinfo->canReceiveResources)
    {
        aiplayer->NumRUDockPoints += NumRUDockPointsOnThisShip(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : aiplayerShipLaunchedCallback
    Description : calls the aiplayer module to indicate a new ship has been built and launched
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aiplayerShipLaunchedCallback(Ship *ship)
{
    AIPlayer *aiplayer;

    if ((gameIsRunning) && (!nisIsRunning))
    {
        aiplayer = ship->playerowner->aiPlayer;
        if (aiplayer != NULL)
        {
            if (bitTest(ship->specialFlags, SPECIAL_LaunchedFromKas))
            {
                bitClear(ship->specialFlags, SPECIAL_LaunchedFromKas);
            }
            else
            {
                unitCapCreateShip(ship, aiplayer->player);
                growSelectAddShip(&aiplayer->newships, ship);
                ThisAIPlayerShipCreatedCB(aiplayer,ship);
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aiplayerPlayerDied
    Description : Checks to see if the player that died is the aiplayer's primary enemy,
                  and changes the primary enemy if needed
    Inputs      : player - the player that died
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aiplayerPlayerDied(Player *player)
{
//    AIPlayer *aiplayer;
//    udword DeadPlayerIndex = player->playerIndex, tempIndex,loop,
//           DeadPlayersEnemy = universe.aiplayerEnemy[DeadPlayerIndex], i,j;
    sdword players[MAX_MULTIPLAYER_PLAYERS][MAX_MULTIPLAYER_PLAYERS];
    Node *shipnode = universe.ShipList.head;
    Ship *ship;
    udword DeadPlayerIndex = player->playerIndex, num_living_humans=0, num_humans=0, maxopponents=0, tempcnt=0, num_remaining_players=0;
    udword slot, newslot, i, j;
    bool done;
    sdword giveup;

    if (DeadPlayerIndex == MAX_MULTIPLAYER_PLAYERS)
    {
        // this is the hack player for autoguns and stuff
        return;
    }

    //count the number of remaining players
    for (i=0;i<universe.numPlayers;i++)
    {
        if (universe.players[i].playerState == PLAYER_ALIVE)
        {
            num_remaining_players++;
        }
    }

    if (num_remaining_players == 1)
    {
        //only 1 player left, so it's the end anyway
        universe.aiplayerEnemy[DeadPlayerIndex] = DeadPlayerIndex;
        goto shipdiedstuff;
    }

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        for (j=0;j<MAX_MULTIPLAYER_PLAYERS;j++)
        {
            players[i][j] = -1;
        }
    }

    //fill in the players enemy array
    for (i=0;i<universe.numPlayers;i++)
    {
        if (universe.aiplayerEnemy[i] != -1)
        {
            if (universe.players[i].playerState == PLAYER_ALIVE)
            {
                bool done = FALSE;
                giveup = 0;
                slot = 0;

                while (!done)
                {
                    if (giveup++ > 100) { aiplayerLog((0,"Warning: giving up on loop 4")); break; }
                    if (players[universe.aiplayerEnemy[i]][slot] == -1)
                    {
                        players[universe.aiplayerEnemy[i]][slot] = i;
                        done = TRUE;
                    }
                    else
                    {
                        slot++;
                    }
                }
            }
        }
        else
        {
            //humans have -1 as their enemies
            num_humans++;

            if (universe.players[i].playerState == PLAYER_ALIVE)
            {
                num_living_humans++;
            }
        }
    }

    //find out how many opponents human players have
    maxopponents = 0;

    for (i=0;i<num_humans;i++)
    {
        slot = 0;

        while (players[i][slot]!=-1)
        {
            tempcnt++;
            slot++;
        }

        //this human player has tempcnt opponents
        //is it the minimum number of opponents?
        if (tempcnt > maxopponents)
        {
            maxopponents = tempcnt;
        }
    }

    //now find new opponents for the players who's enemy died
    slot=0;

    while (players[DeadPlayerIndex][slot] != -1)
    {
        udword bigotchance = randyrandombetween(RAN_AIPlayer, 0, 100);

        //there're still living humans, and the computer player is a bigot
        //so he'll have to kill the human, won't he?
        if ((num_living_humans) && (bigotchance < AIPLAYER_BIGOTRY[players[DeadPlayerIndex][slot]-num_humans]))
        {
            udword enemy = randyrandombetween(RAN_AIPlayer, 0, num_humans);
            done = FALSE;
            giveup = 0;
            newslot = 0;

            //the computer player is a bigot
            while (!done)
            {
                if (giveup++ > 100) { aiplayerLog((0,"Warning: giving up on loop 5")); break; }
                if ((universe.players[enemy].playerState == PLAYER_ALIVE) &&
                    (players[enemy][newslot] == -1))
                {
                    players[enemy][newslot] = players[DeadPlayerIndex][slot];
                    done = TRUE;

                    if (universe.players[players[DeadPlayerIndex][slot]].aiPlayer)
                    {
                        universe.players[players[DeadPlayerIndex][slot]].aiPlayer->primaryEnemyPlayer = &universe.players[enemy];
                    }
                }
                else
                {
                    newslot++;

                    if ((universe.players[enemy].playerState != PLAYER_ALIVE) ||
                        (newslot == maxopponents))
                    {
                        enemy++;
                        newslot = 0;

                        if (enemy == num_humans)
                        {
                            enemy = 0;
                        }
                    }
                }
            }
        }
        else
        {
            //find the enemy for a non-bigot
            udword enemy = randyrandombetween(RAN_AIPlayer, 0, (universe.numPlayers-1));
            bool done    = FALSE, loop = FALSE;
            giveup = 0;

            while (!done)
            {
                if (giveup++ > 100) { aiplayerLog((0,"Warning: giving up on loop 6")); break; }
                //if the player has no enemy
                if ((universe.players[enemy].playerState == PLAYER_ALIVE) &&
                    (players[enemy][0] == -1) && (enemy != players[DeadPlayerIndex][slot]))
                {
                    //the player becomes that player's enemy
                    players[enemy][0] = players[DeadPlayerIndex][slot];
                    done = TRUE;

                    if (universe.players[players[DeadPlayerIndex][slot]].aiPlayer)
                    {
                        universe.players[players[DeadPlayerIndex][slot]].aiPlayer->primaryEnemyPlayer = &universe.players[enemy];
                    }
                }
                else
                {
                    //go to the next player
                    enemy++;

                    //loop around if at end of list
                    if (enemy == universe.numPlayers)
                    {

                        //if this while loop happens more than once, then
                        //the random enemies chosen ended up giving the last
                        //computer player to have itself as enemy
                        if (!loop)
                        {
                            loop = TRUE;
                        }
                        else
                        {
                            //loop done twice, switch previous enemy with current enemy
                            enemy--;
                            players[enemy][0] = players[enemy-1][0];
                            players[enemy-1][0] = players[DeadPlayerIndex][slot];
                            done = TRUE;
                        }

                        enemy = 0;
                    }
                }
            }
        }
        slot++;
    }


    //initialize enemyplayer array
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        universe.aiplayerEnemy[i] = -1;
    }

    //give everyone an enemy
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        slot = 0;

        while (players[i][slot] != -1)
        {
            universe.aiplayerEnemy[players[i][slot]] = i;
            slot++;
        }
    }

    for (i=0;i<universe.numPlayers;i++)
    {
        if ((universe.players[i].aiPlayer) && (universe.players[i].playerState == PLAYER_ALIVE))
        {
            if ((universe.aiplayerEnemy[i] == -1) ||
                (universe.players[universe.aiplayerEnemy[i]].playerState != PLAYER_ALIVE))
            {
                //on rare occasions, the super solid, well written, elegant code above
                //accidently assigns a dead player as the CPU's primary enemy.  If this
                //is the case, throw up your arms, give up completely and assign the
                //first living player as the CPU's primary enemy.
                for (j=0;j<universe.numPlayers;j++)
                {
                    //if this is a different player and it's alive
                    if ((j != i) &&
                        (universe.players[j].playerState == PLAYER_ALIVE))
                    {
                        universe.players[i].aiPlayer->primaryEnemyPlayer = &universe.players[j];
                        aiplayerLog(((uword)i, "Giving Up on this function... assigning the first living player %i as primary enemy", j));
                    }
                }
            }
            else
            {
                //yay!  it worked properly
                universe.players[i].aiPlayer->primaryEnemyPlayer = &universe.players[universe.aiplayerEnemy[i]];
                aiplayerLog(((uword)i, "New Primary Enemy %i", universe.aiplayerEnemy[i]));
            }

            aiuChangePrimaryEnemy(universe.players[i].aiPlayer);
            universe.players[i].aiPlayer->recalculateAllies = TRUE;
        }
    }

shipdiedstuff:
    //go through the entire shiplist and run the ship died
    //function for every ship belonging to the dead player
    while (shipnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(shipnode);

        if (ship->playerowner == player)
        {
            aiplayerShipDied(ship);
        }
        shipnode = shipnode->next;
    }

    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        aiplayerLog(((uword)i, "My aiplayer Enemy = %i", universe.aiplayerEnemy[i]));
    }

}


//
//  return 1 if the ship isn't on any team of the aiplayer
//  return 0 otherwise
//
static int ShipNotOnTeam(AIPlayer *aiplayer, Ship *ship)
{
    sdword i, j;
    AITeam *teamp;

    for (i = 0; i < aiplayer->teamsUsed; ++i)
    {
        teamp = aiplayer->teams[i];
        for (j = 0; j < teamp->shipList.selection->numShips; ++j)
            if (teamp->shipList.selection->ShipPtr[j] == ship)
                return 0;
    }
    return 1;
}

/*-----------------------------------------------------------------------------
    Name        : BuildShipList
    Description : Goes through the universe shiplist and pulls out the aiplayer's ships
    Inputs      : aiplayer - the current computer player
    Outputs     : fills in the computer players newships shiplist
    Return      : void
----------------------------------------------------------------------------*/
void BuildShipList(AIPlayer *aiplayer)
{
    Node *shipnode = universe.ShipList.head;
    Ship *ship;

    while (shipnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(shipnode);

        if (((ship->flags & SOF_NISShip) == 0) &&
            (ship->playerowner == aiplayer->player) && ShipNotOnTeam(aiplayer, ship))
        {
            growSelectAddShip(&aiplayer->newships, ship);
            ThisAIPlayerShipCreatedCB(aiplayer,ship);
        }
        shipnode = shipnode->next;
    }
}


/*-----------------------------------------------------------------------------
    Name        : FindEnemies
    Description : Counts the number of enemy players and decides who the primary enemy will be
    Inputs      : aiplayer - the current aiplayer
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void FindEnemies(AIPlayer *aiplayer)
{
    udword primaryEnemyIndex = 0;

    aiplayer->enemyPlayerCount = universe.numPlayers - 1;

    primaryEnemyIndex = universe.aiplayerEnemy[aiplayer->player->playerIndex];

    //randomize the primary Enemy Index if there is more than one opponent
/*    if (aiplayer->enemyPlayerCount > 1)
    {

        primaryEnemyIndex = ranRandom(RAN_AIPlayer) % (aiplayer->enemyPlayerCount);

        //check to make sure the primaryEnemy isn't the aiplayer itself
        while (&universe.players[primaryEnemyIndex] == aiplayer->player)
        {
            primaryEnemyIndex = ranRandom(RAN_AIPlayer) % (aiplayer->enemyPlayerCount);
        }

    }
    else
    {
        if (&universe.players[primaryEnemyIndex] == aiplayer->player)
        {
            primaryEnemyIndex = 1;
        }
    }
*/
#ifndef HW_Release
    if (primaryEnemyIndex == aiplayer->player->playerIndex)
    {
        aiplayerLog((aiplayer->player->playerIndex, "Super Heap Big Warning!!!  CPU chose itself as primary enemy"));
    }
#endif

    if (primaryEnemyIndex == -1)
    {
        aiplayerLog((aiplayer->player->playerIndex, "Error: CPU Player chose -1th player, setting to 0"));
        primaryEnemyIndex++;
    }
    aiplayer->primaryEnemyPlayer = &universe.players[primaryEnemyIndex];

}


/*-----------------------------------------------------------------------------
    Name        : aiplayerGameStart
    Description : Starting code for aiplayer
    Inputs      : aiplayer - the player to start
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aiplayerGameStart(AIPlayer *aiplayer)
{
    //put startup code in here

    //build currentshiplist
    BuildShipList(aiplayer);

    //find other players and decide who the primary enemy is
    FindEnemies(aiplayer);

    //initializes hyperspacing code
    aifHyperspaceInit(aiplayer);
}




/*-----------------------------------------------------------------------------
    Name        : aiplayerShipDied
    Description : Clears the pointers to a ship that just died from the entire computer player tree
    Inputs      : ship - the ship that just died
    Outputs     : various structures get changed
    Return      : void
----------------------------------------------------------------------------*/
void aiplayerShipDied(ShipPtr ship)
{
    udword i;
    AIPlayer *aiplayer;
    uword playerIndex;

    kasShipDied(ship);      // kas runs even when NIS running, so always report ship deaths.

    if (ship->flags & SOF_NISShip)
    {
        return;
    }

    aiplayerLog((ship->playerowner->playerIndex,"%s Ship died, ", ShipTypeToStr(ship->shiptype)));

    for (i = 0; i < universe.numPlayers;i++)
    {
        if ((aiplayer = universe.players[i].aiPlayer) != NULL)
        {
            aiCurrentAIPlayer = aiplayer;
            playerIndex = aiplayer->player->playerIndex;

            if ((ship->playerowner == aiplayer->player) || (ship->attributes & ATTRIBUTES_Defector)) // do all checks for defector
            {
                if (ship->playerowner == aiplayer->player) ThisAIPlayerShipDiedCB(aiplayer,ship);

                if (growSelectRemoveShip(&aiplayer->newships, ship))
                {
                    if ((aiplayer->numLeaders) && (bitTest(ship->attributes, ATTRIBUTES_TeamLeader)))
                    {
                        aiplayer->numLeaders--;
                    }

                    aiplayerLog((playerIndex,"removed from aiplayer newships"));
                }
            }

            // do these checks all the time for safety (salcap's capturing ships, defectors, etc.)
            if (growSelectRemoveShip(&aiplayer->enemyShipsIAmAwareOf[ship->shiptype], ship))
            {
//                    aiplayerLog((playerIndex,"removed from aiplayer enemyShipsIAmAwareOf"));
            }

            if (growSelectRemoveShip(&aiplayer->primaryEnemyShipsIAmAwareOf[ship->shiptype], ship))
            {
//                    aiplayerLog((playerIndex,"removed from aiplayer primaryEnemyShipsIAmAwareOf"));
            }

            if (airShipDied(aiplayer,ship))
            {
                aiplayerLog((playerIndex,"removed from Resource Manager of aiplayer %d",aiplayer->player->playerIndex));
            }

            if (aiaShipDied(aiplayer,ship))
            {
                aiplayerLog((playerIndex,"removed from Attack Manager of aiplayer %d",aiplayer->player->playerIndex));
            }

            if (aidShipDied(aiplayer,ship))
            {
                aiplayerLog((playerIndex,"removed from Defense Manager of aiplayer %d",aiplayer->player->playerIndex));
            }

            aitShipDied(aiplayer,ship);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aiplayerResourceDied
    Description : Clears all pointers in the computer player tree to a resource that died
    Inputs      : resource - the resource that died
    Outputs     : Clears some pointers
    Return      : void
----------------------------------------------------------------------------*/
void aiplayerResourceDied(Resource *resource)
{
    udword i;
    AIPlayer *aiplayer;

    for (i=0;i<universe.numPlayers;i++)
    {
        if ((aiplayer = universe.players[i].aiPlayer) != NULL)
        {
            aitResourceDied(aiplayer, resource);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aiplayerChangeBigotry
    Description : Changes the bigotry value of the computer player
    Inputs      : playernum - the computer player number
                  newvalue  - the new bigotry value
    Outputs     : changes the global bigotry number of the computer player
    Return      : void
----------------------------------------------------------------------------*/
void aiplayerChangeBigotry(udword newvalue)
{
    udword i;

    dbgAssert(newvalue <= 100);

    for (i=0; i < MAX_MULTIPLAYER_PLAYERS;i++)
    {
        AIPLAYER_BIGOTRY[i] = newvalue+10;
    }

    /*    if ((playernum < MAX_MULTIPLAYER_PLAYERS) && (newvalue <= 100))
    {
        AIPLAYER_BIGOTRY[playernum] = newvalue;
    }
    else
    {
        dbgAssert(FALSE);
    }*/
}

/*-----------------------------------------------------------------------------
    Name        : aiplayerAddLeader
    Description : Adds a leader ship to the computer player's records
    Inputs      : player - the player to add the ship to
    Outputs     : increments the numLeaders variable
    Return      : void
----------------------------------------------------------------------------*/
void aiplayerAddLeader(AIPlayer *aiplayer, ShipPtr ship)
{
    aiplayer->numLeaders++;
    bitSet(ship->attributes, ATTRIBUTES_TeamLeader);
    aiplayerLog((aiplayer->player->playerIndex, "Added Leader to aiplayer"));
}




/*=============================================================================
    Save Game Stuff
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

AIPlayer *fixingThisAIPlayer;

void SavePath(struct Path *path)
{
    SaveChunk *chunk;
    sdword size;

    size = sizeof_Path(path->numPoints);

    chunk = CreateChunk(BASIC_STRUCTURE|PATH,size,path);
    SaveThisChunk(chunk);
    memFree(chunk);
}

struct Path *LoadPath(void)
{
    SaveChunk *chunk;
    sdword size;
    Path *path;
    Path *loadpath;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|PATH);
    path = (Path *)chunkContents(chunk);

    size = sizeof_Path(path->numPoints);
    dbgAssert(size == chunk->contentsSize);

    loadpath = (Path *)memAlloc(size, "newPath", NonVolatile);
    memcpy(loadpath,path,size);

    memFree(chunk);

    return loadpath;
}

sdword AITeamToTeamIndex(struct AITeam *team)
{
    sdword i;
    AIPlayer *aiplayer;

    if (team == NULL)
    {
        return -1;
    }

    aiplayer = team->aiplayerowner;

    for (i=0;i<aiplayer->teamsUsed;i++)
    {
        if (aiplayer->teams[i] == team)
        {
            return i;
        }
    }

    dbgAssert(FALSE);
    return -1;
}

struct AITeam *AITeamIndexToTeam(AIPlayer *aiplayer,sdword index)
{
    if (index == -1)
    {
        return NULL;
    }

    dbgAssert(index < aiplayer->teamsUsed);
    return aiplayer->teams[index];
}

void SaveRequestShipsCB(void *stuff)
{
    RequestShips copyOfRequest;

    memcpy(&copyOfRequest,stuff,sizeof(RequestShips));
    copyOfRequest.creator = (ShipPtr)SpaceObjRegistryGetID((SpaceObj *)copyOfRequest.creator);

    SaveStructureOfSize(&copyOfRequest,sizeof(RequestShips));
}

void SaveTeamWaitingCB(void *stuff)
{
    SaveChunk *chunk;
    TeamWaitingForTheseShips *sc;

    chunk = CreateChunk(BASIC_STRUCTURE,sizeof(TeamWaitingForTheseShips),stuff);
    sc = (TeamWaitingForTheseShips *)chunkContents(chunk);

    sc->team = (AITeam*)AITeamToTeamIndex(sc->team);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadRequestShipsCB(LinkedList *list)
{
    RequestShips *requestShips = LoadStructureOfSize(sizeof(RequestShips));
    requestShips->creator = SpaceObjRegistryGetShip((sdword)requestShips->creator);

    listAddNode(list,&requestShips->node,requestShips);
}

void LoadTeamWaitingCB(LinkedList *list)
{
    TeamWaitingForTheseShips *teamWaiting = LoadStructureOfSize(sizeof(TeamWaitingForTheseShips));

    listAddNode(list,&teamWaiting->node,teamWaiting);
}

void FixTeamWaitingCB(void *stuff)
{
#define teamWaiting ((TeamWaitingForTheseShips *)stuff)
    teamWaiting->team = AITeamIndexToTeam(fixingThisAIPlayer,(sdword)teamWaiting->team);
#undef teamWaiting
}

void SaveThisAIPlayer(AIPlayer *aiplayer)
{
    SaveChunk *chunk;
    AIPlayer *sc;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|AIPLAYER,sizeof(AIPlayer),aiplayer);
    sc = chunkContents(chunk);

    sc->player = (Player*)SavePlayerToPlayerIndex(aiplayer->player);
    sc->primaryEnemyPlayer = (Player*)SavePlayerToPlayerIndex(aiplayer->primaryEnemyPlayer);

    for (i=0;i<aiplayer->numSupportTeams;i++)
    {
        sc->supportTeam[i] = (AITeam*)AITeamToTeamIndex(aiplayer->supportTeam[i]);
    }

    for (i=0;i<AIPLAYER_NUM_RECONTEAMS;i++)
    {
        sc->reconTeam[i] = (AITeam*)AITeamToTeamIndex(aiplayer->reconTeam[i]);
    }

    sc->harassTeam = (AITeam*)AITeamToTeamIndex(aiplayer->harassTeam);

    for (i=0;i<AIPLAYER_NUM_ATTACKTEAMS;i++)
    {
        sc->attackTeam[i] = (AITeam*)AITeamToTeamIndex(aiplayer->attackTeam[i]);
    }

    for (i=0;i<aiplayer->numGuardTeams;i++)
    {
        sc->guardTeams[i] = (AITeam*)AITeamToTeamIndex(aiplayer->guardTeams[i]);
    }

    sc->ScriptCreator = (ShipPtr)SpaceObjRegistryGetID((SpaceObj *)aiplayer->ScriptCreator);
    sc->AICreator     = (ShipPtr)SpaceObjRegistryGetID((SpaceObj *)aiplayer->AICreator);

    SaveThisChunk(chunk);
    memFree(chunk);
    sc = NULL;

    SaveGrowSelection(&aiplayer->newships);

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        SaveGrowSelection(&aiplayer->enemyShipsIAmAwareOf[i]);
        SaveGrowSelection(&aiplayer->primaryEnemyShipsIAmAwareOf[i]);
    }

    SaveLinkedListOfStuff(&aiplayer->AttackManRequestShipsQ,SaveRequestShipsCB);
    SaveLinkedListOfStuff(&aiplayer->DefenseManRequestShipsQ,SaveRequestShipsCB);
    SaveLinkedListOfStuff(&aiplayer->ScriptManRequestShipsQ,SaveRequestShipsCB);

    SaveLinkedListOfStuff(&aiplayer->AttackManTeamsWaitingForShipsQ,SaveTeamWaitingCB);
    SaveLinkedListOfStuff(&aiplayer->DefenseManTeamsWaitingForShipsQ,SaveTeamWaitingCB);
    SaveLinkedListOfStuff(&aiplayer->ScriptManTeamsWaitingForShipsQ,SaveTeamWaitingCB);

    SaveGrowSelection(&aiplayer->airResourceReserves);
    SaveGrowSelection(&aiplayer->airResourceCollectors);

    if (aiplayer->shipsattackingmothership) SaveSelection((SpaceObjSelection *)aiplayer->shipsattackingmothership);
    if (aiplayer->aidProximitySensors)      SaveSelection((SpaceObjSelection *)aiplayer->aidProximitySensors);
    if (aiplayer->aidDefenseTargets)        SaveSelection((SpaceObjSelection *)aiplayer->aidDefenseTargets);
    if (aiplayer->aidInvadingShips)         SaveSelection((SpaceObjSelection *)aiplayer->aidInvadingShips);
    if (aiplayer->aidDistressShips)         SaveSelection((SpaceObjSelection *)aiplayer->aidDistressShips);
    if (aiplayer->aiaArmada.targets)        SaveSelection((SpaceObjSelection *)aiplayer->aiaArmada.targets);
    if (aiplayer->Targets)                  SaveSelection((SpaceObjSelection *)aiplayer->Targets);

    // Save all team stuff

    aitSave(aiplayer);
}

void FixThisAIPlayer(AIPlayer *aiplayer)
{
    sdword i;

    aiplayer->player = SavePlayerIndexToPlayer((sdword)aiplayer->player);
    aiplayer->primaryEnemyPlayer = SavePlayerIndexToPlayer((sdword)aiplayer->primaryEnemyPlayer);

    for (i=0;i<aiplayer->numSupportTeams;i++)
    {
        aiplayer->supportTeam[i] = AITeamIndexToTeam(aiplayer,(sdword)aiplayer->supportTeam[i]);
    }

    for (i=0;i<AIPLAYER_NUM_RECONTEAMS;i++)
    {
        aiplayer->reconTeam[i] = AITeamIndexToTeam(aiplayer,(sdword)aiplayer->reconTeam[i]);
    }

    aiplayer->harassTeam = AITeamIndexToTeam(aiplayer,(sdword)aiplayer->harassTeam);

    for (i=0;i<AIPLAYER_NUM_ATTACKTEAMS;i++)
    {
        aiplayer->attackTeam[i] = AITeamIndexToTeam(aiplayer,(sdword)aiplayer->attackTeam[i]);
    }

    for (i=0;i<aiplayer->numGuardTeams;i++)
    {
        aiplayer->guardTeams[i] = AITeamIndexToTeam(aiplayer,(sdword)aiplayer->guardTeams[i]);
    }

    //FixLinkedListOfStuff(&aiplayer->AttackManRequestShipsQ,FixRequestShipsCB);        not needed
    //FixLinkedListOfStuff(&aiplayer->DefenseManRequestShipsQ,FixRequestShipsCB);       not needed
    //FixLinkedListOfStuff(&aiplayer->ScriptManRequestShipsQ,FixRequestShipsCB);        not needed

    FixLinkedListOfStuff(&aiplayer->AttackManTeamsWaitingForShipsQ,FixTeamWaitingCB);
    FixLinkedListOfStuff(&aiplayer->DefenseManTeamsWaitingForShipsQ,FixTeamWaitingCB);
    FixLinkedListOfStuff(&aiplayer->ScriptManTeamsWaitingForShipsQ,FixTeamWaitingCB);

    // Fix all team stuff

    aitFix(aiplayer);
}

AIPlayer *LoadThisAIPlayer(void)
{
    SaveChunk *chunk;
    AIPlayer *aiplayer;
    udword i;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|AIPLAYER,sizeof(AIPlayer));

    aiplayer = memAlloc(sizeof(AIPlayer),"AIPlayer",0);
    memcpy(aiplayer,chunkContents(chunk),sizeof(AIPlayer));
    memFree(chunk);

    LoadGrowSelectionAndFix(&aiplayer->newships);

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        LoadGrowSelectionAndFix(&aiplayer->enemyShipsIAmAwareOf[i]);
        LoadGrowSelectionAndFix(&aiplayer->primaryEnemyShipsIAmAwareOf[i]);
    }

    LoadLinkedListOfStuff(&aiplayer->AttackManRequestShipsQ,LoadRequestShipsCB);
    LoadLinkedListOfStuff(&aiplayer->DefenseManRequestShipsQ,LoadRequestShipsCB);
    LoadLinkedListOfStuff(&aiplayer->ScriptManRequestShipsQ,LoadRequestShipsCB);

    LoadLinkedListOfStuff(&aiplayer->AttackManTeamsWaitingForShipsQ,LoadTeamWaitingCB);
    LoadLinkedListOfStuff(&aiplayer->DefenseManTeamsWaitingForShipsQ,LoadTeamWaitingCB);
    LoadLinkedListOfStuff(&aiplayer->ScriptManTeamsWaitingForShipsQ,LoadTeamWaitingCB);

    aiplayer->ScriptCreator = SpaceObjRegistryGetShip((sdword)aiplayer->ScriptCreator);
    aiplayer->AICreator     = SpaceObjRegistryGetShip((sdword)aiplayer->AICreator);

    LoadGrowSelectionAndFix(&aiplayer->airResourceReserves);
    LoadGrowSelectionAndFix(&aiplayer->airResourceCollectors);

    if (aiplayer->shipsattackingmothership) aiplayer->shipsattackingmothership = (SelectCommand *)LoadSelectionAndFix();
    if (aiplayer->aidProximitySensors)      aiplayer->aidProximitySensors      = (SelectCommand *)LoadSelectionAndFix();
    if (aiplayer->aidDefenseTargets)        aiplayer->aidDefenseTargets        = (SelectCommand *)LoadSelectionAndFix();
    if (aiplayer->aidInvadingShips)         aiplayer->aidInvadingShips         = (SelectCommand *)LoadSelectionAndFix();
    if (aiplayer->aidDistressShips)         aiplayer->aidDistressShips         = (SelectCommand *)LoadSelectionAndFix();
    if (aiplayer->aiaArmada.targets)        aiplayer->aiaArmada.targets        = (SelectCommand *)LoadSelectionAndFix();
    if (aiplayer->Targets)                  aiplayer->Targets                  = (SelectCommand *)LoadSelectionAndFix();

    // Load all team stuff

    aitLoad(aiplayer);

    return aiplayer;
}

sdword AIPlayerToNumber(AIPlayer *aiplayer)
{
    sdword  number;

    if (aiplayer == NULL)
    {
        number = -1;
    }
    else
    {
        number = aiplayer->player->playerIndex;
    }

    return number;
}

AIPlayer *NumberToAIPlayer(sdword number)
{
    if (number == -1)
    {
        return NULL;
    }
    else
    {
        Player *tmpplayer = SavePlayerIndexToPlayer(number);
        dbgAssert(tmpplayer->aiPlayer);
        return tmpplayer->aiPlayer;
    }
}

void aiplayerSave(void)
{
    sdword i;
    AIPlayer *aiplayer;

    // Save Global Variables

    SaveInfoNumber(AIPlayerToNumber(aiCurrentAIPlayer));

    aivarSave();

    // Save each instance of aiplayer structure

    for (i=0;i<universe.numPlayers;i++)
    {
        aiplayer = universe.players[i].aiPlayer;
        if (aiplayer != NULL)
        {
            fixingThisAIPlayer = aiplayer;
            SaveThisAIPlayer(aiplayer);
            fixingThisAIPlayer = NULL;
        }
    }
}

void aiplayerLoad(void)
{
    // Load Global Variables

    sdword number;
    sdword i;
    AIPlayer *aiplayer;

    number = LoadInfoNumber();

    aivarLoad();

    // Load each instance of aiplayer structure

    for (i=0;i<universe.numPlayers;i++)
    {
        aiplayer = universe.players[i].aiPlayer;
        if (aiplayer != NULL)
        {
            universe.players[i].aiPlayer = fixingThisAIPlayer = LoadThisAIPlayer();
            FixThisAIPlayer(fixingThisAIPlayer);
            fixingThisAIPlayer = NULL;
        }
    }

    aiCurrentAIPlayer = NumberToAIPlayer(number);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"


