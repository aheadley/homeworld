
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "Types.h"
#include "Tweak.h"
#include "StatScript.h"
#include "Blobs.h"
#include "MultiplayerGame.h"
#include "HorseRace.h"

/*=============================================================================
    Insert global tweakables here:
=============================================================================*/

char  TM_TechListFont[64];
char  TM_Font[64];
color TM_SelectionTextColor;
color TM_SelectionRectColor;
color TM_StandardTextColor;
color TM_CantAffordTextColor;

extern char DefaultScenario[];

uword singleMachineNumPlayers = 2;
char useMission[50];
color R1BulletColor;
color R2BulletColor;

extern real32 cmFloatRefundRatio;

bool shipsRetaliate = TRUE;
real32 RETALIATE_ZONE = 1000.0f;

real32 BLAST_CONSTANT = 100000.0f;
real32 BLAST_RADIUS_MULTIPLE_DEFAULT = 1.5f;
real32 BLAST_DAMAGE_MAX_PERCENT = 0.5f;

real32 BLAST_CONSTANT_SCUTTLE = 150000.0f;
real32 BLAST_RADIUS_SCUTTLE_BONUS = 1.33f;
real32 BLAST_DAMAGE_MAX_PERCENT_SCUTTLE = 0.75f;

real32 YAWEXPLOSIONROCKLO = -100.0f;
real32 YAWEXPLOSIONROCKHI = 100.0f;
real32 PITCHEXPLOSIONROCKLO = -100.0f;
real32 PITCHEXPLOSIONROCKHI = 100.0f;
real32 ROLLEXPLOSIONROCKLO = -100.0f;
real32 ROLLEXPLOSIONROCKHI = 100.0f;

real32 YAWROCKLO = -100.0f;
real32 YAWROCKHI = 100.0f;
real32 PITCHROCKLO = -100.0f;
real32 PITCHROCKHI = 100.0f;
real32 ROLLROCKLO = -100.0f;
real32 ROLLROCKHI = 100.0f;

real32 YAWCAREENLO = -100.0f;
real32 YAWCAREENHI = 100.0f;
real32 PITCHCAREENLO = -100.0f;
real32 PITCHCAREENHI = 100.0f;
real32 ROLLCAREENLO = -100.0f;
real32 ROLLCAREENHI = 100.0f;

real32 YAWCRAZYLO = -10.0f;
real32 YAWCRAZYHI = 10.0f;
real32 PITCHCRAZYLO = -10.0f;
real32 PITCHCRAZYHI = 10.0f;
real32 ROLLCRAZYLO = -10.0f;
real32 ROLLCRAZYHI = 10.0f;

real32 RENDER_VIEWABLE_DISTANCE_SQR = (10000.0f*10000.0f);
real32 RENDER_MAXVIEWABLE_DISTANCE_SQR = (20000.0f*20000.0f);
real32 RENDER_VIEWABLE_FADE = 3500.0f;
real32 RENDER_MAXVIEWABLE_FADE = 4500.0f;
real32 BG_RADIUS = 20000.0f;

real32 RENDER_LIMIT_MOTHERSHIP = (20000.0f*20000.0f);
real32 RENDER_FADE_MOTHERSHIP = (10000.0f*10000.0f);
real32 RENDER_LIMIT_HEAVYCRUISER = (10000.0f*10000.0f);
real32 RENDER_FADE_HEAVYCRUISER = (10000.0f*10000.0f);
real32 RENDER_LIMIT_CARRIER = (10000.0f*10000.0f);
real32 RENDER_FADE_CARRIER = (10000.0f*10000.0f);
real32 RENDER_LIMIT_DESTROYER = (10000.0f*10000.0f);
real32 RENDER_FADE_DESTROYER = (10000.0f*10000.0f);
real32 RENDER_LIMIT_FRIGATE = (10000.0f*10000.0f);
real32 RENDER_FADE_FRIGATE = (10000.0f*10000.0f);
real32 RENDER_LIMIT_CORVETTE = (10000.0f*10000.0f);
real32 RENDER_FADE_CORVETTE = (10000.0f*10000.0f);
real32 RENDER_LIMIT_FIGHTER = (10000.0f*10000.0f);
real32 RENDER_FADE_FIGHTER = (10000.0f*10000.0f);
real32 RENDER_LIMIT_RESOURCE = (10000.0f*10000.0f);
real32 RENDER_FADE_RESOURCE = (10000.0f*10000.0f);
real32 RENDER_LIMIT_NONCOMBAT = (10000.0f*10000.0f);
real32 RENDER_FADE_NONCOMBAT = (10000.0f*10000.0f);

real32 VERYBIGSHIP_SIZE = 1500.0f;

real32 OUT_OF_FUEL_FRICTION = 0.95f;
real32 SHIP_CRAZY_FRICTION = 0.95f;
real32 DONT_DRIFT_FRICTION = 0.99f;
real32 CONSIDERED_STILL_LO = -5.0f;
real32 CONSIDERED_STILL_HI = 5.0f;

// colors for the pie plate movement mechanism (mainrgn.c)
color TW_MOVE_PIZZA_COLOR;
color TW_MOVE_ENEMY_COLOR;
color TW_MOVE_POINT_LINE_COLOR;
color TW_MOVE_ORIGIN_COLOR;
color TW_MOVE_HEADING_COLOR;
color TW_MOVE_SPOKE_COLOR;
extern real32 pieMaxMoveVertical, pieMaxMoveHorizontal;

// other tweakable colors (mainrgn.c)
color TW_SELECT_BOX_COLOR;
color TW_SHIP_LINE_COLOR;
color TW_SHIP_LINE_CLOSEST_COLOR;

// distance readout tweakables
color TW_DISTANCE_READOUT_COLOR;
char  TW_DISTANCE_READOUT_FONT[64];

// Cursor Text tweakables (mainrgn.c)
color TW_CURSORTEXT_COLOR;
real32 TW_CURSORTEXT_DELAY_TIME = 1.0f;
sdword TW_CURSORTEXT_X = 0;
sdword TW_CURSORTEXT_Y = 430;
char   TW_CURSORTEXT_FONT[64];

color  TW_MOVETO_LINE_COLOR;
color  TW_MOVETO_PULSE_COLOR;
color  TW_ATTMOVETO_LINE_COLOR;
color  TW_ATTMOVETO_PULSE_COLOR;
real32 TW_MOVETO_CIRCLE_RADIUS = 1.2f;
real32 TW_MOVETO_PULSE_SPEED_SCALE = 2.0f;
real32 TW_MOVETO_ENDCIRCLE_RADIUS = 160.0f;

color  TO_SPHERE_COLOR;

color  TO_CROSS_COLOR1;
color  TO_CROSS_COLOR2;

real32 TO_CROSS_HALF_SIZE;

color  TO_PULSE1_COLOR;
real32 TO_PULSE1_SPEED;
real32 TO_DELAY_PULSE1;

color  TO_PULSE2_COLOR;
real32 TO_PULSE2_SPEED;
real32 TO_DELAY_PULSE2;
real32 TO_PULSE1_SIZE;
real32 TO_PULSE2_SIZE;

// time for command message feedback to stay on screen
uword TW_MESSAGE_DELAY_TIME;

// time for selection flash on ships
real32 FLASH_TIMER = 0.3f;

// scale factor for determining collision damage.  The bigger, the smaller the damage
real32 COLLDAMAGE_VELOCITY_TANGENT_SCALE = 1000.0f;
real32 COLLDAMAGE_DEFAULTMIN_FRACTION = 0.2f;
real32 COLLDAMAGE_DEFAULTMAX_FRACTION = 1.0f;

real32 PLAYERLOSE_SHIPDEATHTIMEMIN = 5.0f;
real32 PLAYERLOSE_SHIPDEATHTIMEMAX = 10.0f;

real32 SCUTTLE_SHIPDEATHTIMEMIN = 0.0f;
real32 SCUTTLE_SHIPDEATHTIMEMAX = 1.0f;

real32 WAIT_TIME_TO_QUIT = 10.0f;
real32 WAIT_TIME_TO_WIN = 5.0f;

extern BlobProperties collBlobProperties;

/*
// color of on screen map spheres
color  MM_SPHERE_COLOR;
real32 MM_SELECTED_FLASH_SPEED;
real32 MM_CAMERA_DISTANCE;
real32 MM_MESH_SCALE_FACTOR = 0.4f;
*/
// stats gathering stuff
sdword MIN_SHIPS_TO_FIGHT = 5;
real32 FACEOFF_DISTANCE = 5000.0f;
real32 STATFIGHT_TIMEOUT_TIME = 1000.0f;

// gun tuning stuff
real32 GUNTUNE_SPEED = 0.02f;

// max move distance tuning
udword MAX_MOVE_DISTANCE    = 196000000;
udword MAX_HARVEST_DISTANCE = 196000000;

// decloaking attack time variables
real32 TW_NOONE_SEE_TIME;
real32 TW_FIRST_SEE_TIME;
real32 TW_SECOND_SEE_TIME;
udword TW_FIRST_SEE_PERCENTAGE;
udword TW_SECOND_SEE_PERCENTAGE;

ubyte AUTOSTABILIZE_FRAME_KICK_IN = 8;

extern color selShipResourceColor;
extern color selShipDarkResourceColor;
extern color selSelectingColor;
extern color selShipHealthGreen;
extern color selShipHealthYellow;
extern color selShipHealthRed;
extern color selShipHealthDarkGreen;
extern color selShipHealthDarkYellow;
extern color selShipHealthDarkRed;
extern color selShipHealthSolidGreen;
extern color selShipHealthSolidYellow;
extern color selShipHealthSolidRed;
extern color selFuelColorGreen;
extern color selFuelColorRed;
extern color selFuelColorDarkGreen;
extern color selFuelColorDarkRed;
extern real32 selDepthSelectMultiplier;

extern real32 mrNumberDoublePressTime;
extern real32 mrFastMoveShipClickTime;
extern sdword mrFastMoveShipClickX;
extern sdword mrFastMoveShipClickY;

sdword resourceStartSmall   = 400;
sdword resourceStartMedium  = 1000;
sdword resourceStartLarge   = 3000;

sdword resourceStartTutorial = 1700;

real32 horseRaceRenderTime;

struct HorseRaceBars horseTotalNumBars;

real32 TW_RETIRE_MONEYBACK_RATIO;

sdword tweakCheckMissileTargetLossRate;
sdword tweakNumTimesCheck;
real32 tweakMinDistBeforeTargetLoss;
real32 tweakRandomNess[NUM_TACTICS_TYPES];
real32 tweakBoostIfDodging;
real32 tweakBoostIfFlightMan;

real32 attrVELOCITY_TOWARDS = 200.0f;
real32 attrVELOCITY_TOWARDS_DEVIATION = 0.2f;
real32 attrKillerCollDamage = 5000.0f;
real32 attrHEADSHOTVELOCITY_TOWARDS = 400.0f;
real32 attrHEADSHOTVELOCITY_TOWARDS_DEVIATION = 0.0f;
real32 attrHEADSHOTKillerCollDamage = 10000.0f;

real32 attrRING_WIDTH_SCALE = 0.5f;

real32 capitalSideModifiers[NUM_TRANS_DEGOFFREEDOM] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

real32 friendlyFireModifier = 0.5f;

real32 TW_ScuttleReconfirmTime = 3.0f;

real32 TW_Bounty_ResourcesCollected;
real32 TW_Bounty_ResourcesInPossesion;
real32 TW_Bounty_CurrentShipWorth;
real32 TW_Bounty_AllyStrength;
real32 TW_Bounty_ResourcesKilled;

real32 TW_HARVESTER_DAMAGE_PER_SECOND;

real32 capShipLieFlatInGeneralDistanceTolerance[NUM_CLASSES];
real32 capShipLieFlatInGeneralSpeedSqrTolerance[NUM_CLASSES];

real32 TW_HOLDING_PATTERN_PATHTIME;
real32 TW_HOLDING_PATTERN_PATH_DISTANCE;
real32 TW_HOLDING_PATTERN_NUM_TURNS;
real32 TW_HOLDING_PATTERN_PATH_MAX_SPEED;

color TW_MOVE_ATTACK_POINT_LINE_COLOR_IN_RANGE;
color TW_MOVE_ATTACK_POINT_LINE_COLOR_NOT_IN_RANGE;
color TW_MOVE_ATTACK_PIE_COLOR;
color TW_MOVE_ATTACK_SPOKE_COLOR;

real32 MINING_BASE_ROTATION_SPEED = 0.1f;
real32 MINING_BASE_VERTICAL_DIST_HACK = 3400.0f;

real32 TW_HOLDING_PATTERN_HEADING_COEFFICIENT;
real32 TW_HOLDING_PATTERN_RIGHT_COEFFICIENT;

real32 selShipHealthGreenFactor;
real32 selShipHealthYellowFactor;

real32 COLLISION_CLEAR_TIME = 5.0f;
real32 GLANCE_COLLISION_VEL_RATIO = (2.0f*2.0f);
real32 COLLISION_REPULSE_FACTOR = 500.0f;

sdword TW_BountySizes[4] = { 0,0,0,0 };     // last element is for OFF and should always be 0

//scarred scaffold rotation tweaks
real32 SCAFFOLD_SCARRED_ROTX = 0.0;
real32 SCAFFOLD_SCARRED_ROTY = 0.0;
real32 SCAFFOLD_SCARRED_ROTZ = 0.0;
real32 SCAFFOLDFINGERA_SCARRED_ROTX = 0.0;
real32 SCAFFOLDFINGERA_SCARRED_ROTY = 0.0;
real32 SCAFFOLDFINGERA_SCARRED_ROTZ = 0.0;
real32 SCAFFOLDFINGERB_SCARRED_ROTX = 0.0;
real32 SCAFFOLDFINGERB_SCARRED_ROTY = 0.0;
real32 SCAFFOLDFINGERB_SCARRED_ROTZ = 0.0;

extern color versionColor;
real32 SALVAGE_MIN_RETROFIT_TIME;
real32 SALVAGE_MAX_RETROFIT_TIME;

real32 TW_R1_MOTHERSHIP_DOOR_OFFSET_MODIFIER;
real32 TW_SCUTTLE_CONFIRM_TIME;

color TW_MOVE_HYPERSPACE_COLOUR;
color TW_MOVE_POINT_HYPERSPACE_LINE_COLOR;
color TW_MOVE_SPOKE_HYPERSPACE_COLOR;
color TW_MOVE_HEADING_HYPERSPACE_COLOR;

real32 TW_MULTIPLAYER_HYPERSPACE_WAIT_IN_ETHER_TIME;

HW_Cost TW_HyperSpaceCostStruct[TOTAL_NUM_SHIPS];
color TW_RU_READOUT_COLOR_BAD;
color TW_RU_READOUT_COLOR_GOOD;

#if DEM_AUTO_DEMO
extern real32 demAutoDemoWaitTime;
#endif

real32 TW_PING_MAX_DISPERSAL;

color TW_HW_PING_COLOUR_IN;
real32 TW_HW_PING_RATE_IN;
real32 TW_HW_PING_MIN_SIZE_IN;
real32 TW_HW_PING_MAX_SIZE_IN;

color TW_HW_PING_COLOUR_OUT;
real32 TW_HW_PING_RATE_OUT;
real32 TW_HW_PING_MIN_SIZE_OUT;
real32 TW_HW_PING_MAX_SIZE_OUT;

real32 TW_HYPERSPACE_TELEGRAPH_WAIT_TIME;

real32 TW_SV_MAN_VERY_LOW;      //below this number it will register as VERYLOW
real32 TW_SV_MAN_LOW;           //below this, but above previouw == LOW
real32 TW_SV_MAN_MEDIUM;        //below this, but above previouw == MEDIUM
real32 TW_SV_MAN_HIGH;          //below this, but above previouw == HIGH
                                //Above previous == VERY_HIGH

real32 TW_R1_DOOR_DOCK_TOLERANCE;

color TW_GRAVWELL_SPHERE_COLOUR;
color TW_CLOAKGENERATOR_SPHERE_COLOUR;
color TW_DFGF_SPHERE_COLOUR;
color TW_PROXIMITY_SPHERE_COLOUR;
color TW_PROXIMITY_SPHERE_COLOUR_FOUND;
color TW_PROXIMITY_RING_COLOUR;


sdword TUTORIAL_KAS_UPDATE_MASK = 1;
sdword TUTORIAL_KAS_UPDATE_FRAME = 0;

sdword SP_KAS_UPDATE_FRAME = 0;

sdword MINE_DO_SEARCH_MASK = 15;

sdword CHECKBOGEY_FRAME = 1;

sdword SPHERE_CHECK_TARGET_MASK = 31;
sdword SPHERE_CHECK_TARGET_FRAME = 21;

sdword CHECK_ATTACKMOVE_TO_MOVE_RATE = 31;
sdword CHECK_ATTACKMOVE_TO_MOVE_FRAME = 22;

sdword FORMATION_ERROR_CALCULATE_RATE = 7;
sdword FORMATION_ERROR_CALCULATE_FRAME = 2;

sdword EnemyNearByCheckRate = 15;
sdword EnemyNearByCheckFrame = 14;
sdword RetreatCheckRate = 31;
sdword RetreatCheckFrame = 26;

sdword ATTACKMEMORY_CLEANUP_RATE = 255;
sdword ATTACKMEMORY_CLEANUP_FRAME = 254;

sdword TW_ProximitySensorSearchRate = 31;
sdword TW_ProximitySensorSearchFrame = 27;

sdword CHECK_PASSIVE_ATTACK_RATE =  15;
sdword CHECK_PASSIVE_ATTACK_WHILEMOVING_RATE =  31;

sdword REFRESH_RENDERLIST_RATE    =     7;
sdword REFRESH_RENDERLIST_FRAME   =     1;

sdword REFRESH_MINORRENDERLIST_RATE  =  7;
sdword REFRESH_MINORRENDERLIST_FRAME =  3;

sdword REFRESH_COLLBLOB_RATE         =  7;
sdword REFRESH_COLLBLOB_FRAME        =  6;

sdword REFRESH_COLLBLOB_BATTLEPING_FRAME =  4;

sdword REFRESH_RESEARCH_RATE         =  15;
sdword REFRESH_RESEARCH_FRAME        =  13;

sdword CHECK_BOUNTIES_RATE            =     127;
sdword CHECK_BOUNTIES_FRAME           =     101;

sdword CLOAKGENERATOR_CLOAKCHECK_RATE  =    15;

real32 TW_GLOBAL_PASSIVE_ATTACK_CANCEL_MODIFIER=180.0f;

real32 twHyperingInDamage;
real32 twHyperedIntoDamage;
real32 TW_BURST_ATTACK_FORCE_MODIFIER;

real32 armourPiercingModifier = 1.5f;

/*=============================================================================
    Private tweak-specific script set functions
=============================================================================*/

void scriptSetBarsToDo(char *directory,char *field,void *dataToFillIn)
{
    HorseRaceBars *barsToDo = (HorseRaceBars *)dataToFillIn;
    sdword i;
    char *miscPointer;

    barsToDo->numBars = 0;

    miscPointer = strtok(field, " \t,");
    if (miscPointer != NULL)
    {
        sscanf(miscPointer, "%d", &barsToDo->numBars);
    }

    for (i=0;i<barsToDo->numBars;i++)
    {
        if ((miscPointer = strtok(NULL, " \t,")) != NULL)
        {
            sscanf(miscPointer, "%f", &barsToDo->perc[i]);
        }
        else
        {
            dbgFatalf(DBG_Loc,"Invalid specification in tweak.script %s",field);
        }
    }
}

void scriptSetBlobPropertyOverlap(char *directory,char *field,void *dataToFillIn)
{
    sscanf(field,"%f",&((BlobProperties *)dataToFillIn)->bobOverlapFactor);
    ((BlobProperties *)dataToFillIn)->bobSqrtOverlapFactor = sqrt(((BlobProperties *)dataToFillIn)->bobOverlapFactor);
}

void scriptSetBlobBiggestRadius(char *directory,char *field,void *dataToFillIn)
{
    sscanf(field,"%f",&((BlobProperties *)dataToFillIn)->bobBiggestRadius);
    ((BlobProperties *)dataToFillIn)->bobBiggestRadiusDefault = ((BlobProperties *)dataToFillIn)->bobBiggestRadius;
}

/*=============================================================================
    Update table here:
=============================================================================*/

scriptEntry Tweaks[] =
{
    makeEntry(TM_TechListFont, scriptSetStringCB),
    makeEntry(TM_Font, scriptSetStringCB),
    makeEntry(TM_StandardTextColor, scriptSetRGBCB),
    makeEntry(TM_CantAffordTextColor, scriptSetRGBCB),
    makeEntry(TM_SelectionTextColor, scriptSetRGBCB),
    makeEntry(TM_SelectionRectColor, scriptSetRGBCB),
    makeEntry(cmFloatRefundRatio,scriptSetReal32CB),
    makeEntry(shipsRetaliate,scriptSetBool),
    makeEntry(singleMachineNumPlayers,scriptSetUwordCB),
    makeEntry(DefaultScenario,scriptSetStringCB),
    makeEntry(useMission,scriptSetStringCB),
    makeEntry(RETALIATE_ZONE,scriptSetReal32CB),
    makeEntry(R1BulletColor,scriptSetRGBCB),
    makeEntry(R2BulletColor,scriptSetRGBCB),
    makeEntry(BLAST_CONSTANT,scriptSetReal32CB),
    makeEntry(BLAST_RADIUS_MULTIPLE_DEFAULT,scriptSetReal32CB),
    makeEntry(BLAST_DAMAGE_MAX_PERCENT,scriptSetReal32CB),
    makeEntry(BLAST_CONSTANT_SCUTTLE,scriptSetReal32CB),
    makeEntry(BLAST_RADIUS_SCUTTLE_BONUS,scriptSetReal32CB),
    makeEntry(BLAST_DAMAGE_MAX_PERCENT_SCUTTLE,scriptSetReal32CB),
    makeEntry(AUTOSTABILIZE_FRAME_KICK_IN,scriptSetUbyteCB),
    makeEntry(YAWEXPLOSIONROCKLO,scriptSetReal32CB),
    makeEntry(YAWEXPLOSIONROCKHI,scriptSetReal32CB),
    makeEntry(PITCHEXPLOSIONROCKLO,scriptSetReal32CB),
    makeEntry(PITCHEXPLOSIONROCKHI,scriptSetReal32CB),
    makeEntry(ROLLEXPLOSIONROCKLO,scriptSetReal32CB),
    makeEntry(ROLLEXPLOSIONROCKHI,scriptSetReal32CB),
    makeEntry(YAWROCKLO,scriptSetReal32CB),
    makeEntry(YAWROCKHI,scriptSetReal32CB),
    makeEntry(PITCHROCKLO,scriptSetReal32CB),
    makeEntry(PITCHROCKHI,scriptSetReal32CB),
    makeEntry(ROLLROCKLO,scriptSetReal32CB),
    makeEntry(ROLLROCKHI,scriptSetReal32CB),
    makeEntry(YAWCAREENLO,scriptSetReal32CB),
    makeEntry(YAWCAREENHI,scriptSetReal32CB),
    makeEntry(PITCHCAREENLO,scriptSetReal32CB),
    makeEntry(PITCHCAREENHI,scriptSetReal32CB),
    makeEntry(ROLLCAREENLO,scriptSetReal32CB),
    makeEntry(ROLLCAREENHI,scriptSetReal32CB),
    makeEntry(YAWCRAZYLO,scriptSetReal32CB),
    makeEntry(YAWCRAZYHI,scriptSetReal32CB),
    makeEntry(PITCHCRAZYLO,scriptSetReal32CB),
    makeEntry(PITCHCRAZYHI,scriptSetReal32CB),
    makeEntry(ROLLCRAZYLO,scriptSetReal32CB),
    makeEntry(ROLLCRAZYHI,scriptSetReal32CB),
    makeEntry(RENDER_VIEWABLE_DISTANCE_SQR,scriptSetReal32SqrCB),
    makeEntry(RENDER_MAXVIEWABLE_DISTANCE_SQR,scriptSetReal32SqrCB),
    makeEntry(RENDER_VIEWABLE_FADE, scriptSetReal32CB),
    makeEntry(RENDER_MAXVIEWABLE_FADE, scriptSetReal32CB),
    makeEntry(BG_RADIUS,scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_MOTHERSHIP, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_MOTHERSHIP, scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_HEAVYCRUISER, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_HEAVYCRUISER, scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_CARRIER, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_CARRIER, scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_DESTROYER, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_DESTROYER, scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_FRIGATE, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_FRIGATE, scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_CORVETTE, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_CORVETTE, scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_FIGHTER, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_FIGHTER, scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_RESOURCE, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_RESOURCE, scriptSetReal32CB),
    makeEntry(RENDER_LIMIT_NONCOMBAT, scriptSetReal32SqrCB),
    makeEntry(RENDER_FADE_NONCOMBAT, scriptSetReal32CB),
    makeEntry(VERYBIGSHIP_SIZE,scriptSetReal32CB),
    makeEntry(OUT_OF_FUEL_FRICTION,scriptSetReal32CB),
    makeEntry(SHIP_CRAZY_FRICTION,scriptSetReal32CB),
    makeEntry(DONT_DRIFT_FRICTION,scriptSetReal32CB),
    makeEntry(CONSIDERED_STILL_LO,scriptSetReal32CB),
    makeEntry(CONSIDERED_STILL_HI,scriptSetReal32CB),
    makeEntry(TW_MOVE_PIZZA_COLOR       , scriptSetRGBCB),
    makeEntry(TW_MOVE_ENEMY_COLOR       , scriptSetRGBCB),
    makeEntry(TW_MOVE_HEADING_COLOR     , scriptSetRGBCB),
    makeEntry(TW_MOVE_SPOKE_COLOR       , scriptSetRGBCB),
    makeEntry(TW_MOVE_POINT_LINE_COLOR  , scriptSetRGBCB),
    makeEntry(TW_MOVE_ORIGIN_COLOR      , scriptSetRGBCB),
    makeEntry(TW_SELECT_BOX_COLOR       , scriptSetRGBCB),
    makeEntry(TW_SHIP_LINE_COLOR        , scriptSetRGBCB),
    makeEntry(TW_SHIP_LINE_CLOSEST_COLOR, scriptSetRGBCB),
    makeEntry(TW_DISTANCE_READOUT_COLOR , scriptSetRGBCB),
    makeEntry(TW_DISTANCE_READOUT_FONT  , scriptSetStringCB),
    makeEntry(pieMaxMoveHorizontal      , scriptSetReal32CB),
    makeEntry(pieMaxMoveVertical        , scriptSetReal32CB),
    makeEntry(pieCircleSizeMax          , scriptSetReal32CB),
    makeEntry(pieCircleSizeMin          , scriptSetReal32CB),
    //cursor text tweaks
    makeEntry(TW_CURSORTEXT_COLOR       , scriptSetRGBCB),
    makeEntry(TW_CURSORTEXT_DELAY_TIME  , scriptSetReal32CB),
    makeEntry(TW_CURSORTEXT_X           , scriptSetSdwordCB),
    makeEntry(TW_CURSORTEXT_Y           , scriptSetSdwordCB),
    makeEntry(TW_CURSORTEXT_FONT        , scriptSetStringCB),
    //moveto line tweaks
    makeEntry(TW_MOVETO_LINE_COLOR      , scriptSetRGBCB),
    makeEntry(TW_MOVETO_PULSE_COLOR     , scriptSetRGBCB),
    makeEntry(TW_ATTMOVETO_LINE_COLOR   , scriptSetRGBCB),
    makeEntry(TW_ATTMOVETO_PULSE_COLOR  , scriptSetRGBCB),
    makeEntry(TW_MOVETO_CIRCLE_RADIUS   , scriptSetReal32CB),
    makeEntry(TW_MOVETO_PULSE_SPEED_SCALE,scriptSetReal32CB),
    makeEntry(TW_MOVETO_ENDCIRCLE_RADIUS, scriptSetReal32CB),
    makeEntry(TO_SPHERE_COLOR,scriptSetRGBCB),
    makeEntry(TO_PULSE1_COLOR ,scriptSetRGBCB),
    makeEntry(TO_PULSE2_COLOR ,scriptSetRGBCB),
    makeEntry(TO_PULSE1_SPEED ,scriptSetReal32CB),
    makeEntry(TO_PULSE2_SPEED ,scriptSetReal32CB),
    makeEntry(TO_DELAY_PULSE1 ,scriptSetReal32CB),
    makeEntry(TO_DELAY_PULSE2 ,scriptSetReal32CB),
    makeEntry(TO_CROSS_COLOR1 ,scriptSetRGBCB),
    makeEntry(TO_CROSS_COLOR2 ,scriptSetRGBCB),
    makeEntry(TO_CROSS_HALF_SIZE ,scriptSetReal32CB),
    makeEntry(TO_PULSE1_SIZE ,scriptSetReal32CB),
    makeEntry(TO_PULSE2_SIZE ,scriptSetReal32CB),

    makeEntry(TW_MESSAGE_DELAY_TIME, scriptSetUwordCB),
    makeEntry(FLASH_TIMER, scriptSetReal32CB),
    makeEntry(COLLDAMAGE_VELOCITY_TANGENT_SCALE,scriptSetReal32CB),
    makeEntry(COLLDAMAGE_DEFAULTMIN_FRACTION,scriptSetReal32CB),
    makeEntry(COLLDAMAGE_DEFAULTMAX_FRACTION,scriptSetReal32CB),
    makeEntry(PLAYERLOSE_SHIPDEATHTIMEMIN,scriptSetReal32CB),
    makeEntry(PLAYERLOSE_SHIPDEATHTIMEMAX,scriptSetReal32CB),
    makeEntry(WAIT_TIME_TO_QUIT,scriptSetReal32CB),
    makeEntry(WAIT_TIME_TO_WIN,scriptSetReal32CB),
    //on screen map tweaks
/*
    makeEntry(MM_SPHERE_COLOR       , scriptSetRGBCB),
    makeEntry(MM_SELECTED_FLASH_SPEED, scriptSetReal32CB),
    makeEntry(MM_CAMERA_DISTANCE    , scriptSetReal32CB),
    makeEntry(MM_MESH_SCALE_FACTOR  , scriptSetReal32CB),
*/
    //collision blobs
    { "univBobDensityLow",  scriptSetReal32CB, &collBlobProperties.bobDensityLow },
    { "univBobDensityHigh", scriptSetReal32CB, &collBlobProperties.bobDensityHigh },
    { "univBobStartSphereSize",  scriptSetReal32CB, &collBlobProperties.bobStartSphereSize },
    { "univBobRadiusCombineMargin", scriptSetReal32CB, &collBlobProperties.bobRadiusCombineMargin },
    { "univBobOverlapFactor",  scriptSetBlobPropertyOverlap, &collBlobProperties },
    { "univBobSmallestRadius", scriptSetReal32CB, &collBlobProperties.bobSmallestRadius },
    { "univBobBiggestRadius", scriptSetBlobBiggestRadius, &collBlobProperties },
//    { "univBobDoingCollisionBobs", scriptSetBool, &collBlobProperties.bobDoingCollisionBobs },

    //stats gathering tweaks
    makeEntry(MIN_SHIPS_TO_FIGHT    , scriptSetSdwordCB),
    makeEntry(FACEOFF_DISTANCE      , scriptSetReal32CB),
    makeEntry(STATFIGHT_TIMEOUT_TIME, scriptSetReal32CB),
    // gun tuning tweaks
    makeEntry(GUNTUNE_SPEED         , scriptSetReal32CB),
    //various front end tweaks
    makeEntry(ferSelectedDotMarginX , scriptSetSdwordCB),
    makeEntry(ferSelectedDotMarginY , scriptSetSdwordCB),
    makeEntry(feMenuScreenMarginX   , scriptSetSdwordCB),
    makeEntry(feMenuScreenMarginY   , scriptSetSdwordCB),
    makeEntry(feMenuOverlapMarginX  , scriptSetSdwordCB),
    makeEntry(feMenuOverlapMarginY  , scriptSetSdwordCB),
    makeEntry(cmShipListFontName    , scriptSetStringCB),
    makeEntry(cmSelectTopAdjust     , scriptSetSdwordCB),
    makeEntry(cmSelectBottomAdjust  , scriptSetSdwordCB),
    makeEntry(cmSelectLeftAdjust    , scriptSetSdwordCB),
    makeEntry(cmSelectRightAdjust   , scriptSetSdwordCB),
    makeEntry(svShipStatFontName    , scriptSetStringCB),
    makeEntry(cmStandardTextColor   , scriptSetRGBCB),
    makeEntry(cmClassHeadingTextColor,scriptSetRGBCB),
    makeEntry(cmSelectionTextColor  , scriptSetRGBCB),
    makeEntry(cmBuildingTextColor   , scriptSetRGBCB),
    makeEntry(cmProgressShipColor   , scriptSetRGBCB),
    makeEntry(cmProgressTotalColor  , scriptSetRGBCB),
    makeEntry(cmCapsExceededTextColor   , scriptSetRGBCB),

    makeEntry(lmShipListFontName        , scriptSetStringCB),
    makeEntry(lmShipListTextColor       , scriptSetRGBCB),
    makeEntry(lmShipSelectedTextColor   , scriptSetRGBCB),
    makeEntry(lmUsedColor               , scriptSetRGBCB),
    makeEntry(lmAvailableColor          , scriptSetRGBCB),

    makeEntry(rmTechListFontName        , scriptSetStringCB),
    makeEntry(rmTechInfoFontName        , scriptSetStringCB),
    makeEntry(rmSelectionTextColor      , scriptSetRGBCB),
    makeEntry(rmResearchingTextColor    , scriptSetRGBCB),
    makeEntry(rmCantResearchTextColor   , scriptSetRGBCB),
    makeEntry(rmStandardTextColor       , scriptSetRGBCB),
    makeEntry(rmClassHeadingTextColor   , scriptSetRGBCB),
    makeEntry(rmProgressToGoColor       , scriptSetRGBCB),
    makeEntry(rmProgressDoneColor0      , scriptSetRGBCB),
    makeEntry(rmProgressDoneColor1      , scriptSetRGBCB),
    makeEntry(rmLabActiveColor          , scriptSetRGBCB),
    makeEntry(rmPulseColor              , scriptSetRGBCB),
    makeEntry(rmNoResearchItemColor     , scriptSetRGBCB),
    makeEntry(rmMarqueeOnColor          , scriptSetRGBCB),
    makeEntry(rmMarqueeSemiOnColor      , scriptSetRGBCB),
    makeEntry(rmMarqueeOffColor         , scriptSetRGBCB),

    makeEntry(ioShipListFontName        , scriptSetStringCB),
    makeEntry(ioListTextColor           , scriptSetRGBCB),
    makeEntry(ioSelectedTextColor       , scriptSetRGBCB),


    //selection tweaks
    makeEntry(selMinSelectionRadius, scriptSetReal32CB),
    makeEntry(selShipResourceColor    , scriptSetRGBCB),
    makeEntry(selShipDarkResourceColor, scriptSetRGBCB),
    makeEntry(selSelectingColor       , scriptSetRGBCB),
    makeEntry(selShipHealthGreen      , scriptSetRGBCB),
    makeEntry(selShipHealthYellow     , scriptSetRGBCB),
    makeEntry(selShipHealthRed        , scriptSetRGBCB),
    makeEntry(selShipHealthDarkGreen  , scriptSetRGBCB),
    makeEntry(selShipHealthDarkYellow , scriptSetRGBCB),
    makeEntry(selShipHealthDarkRed    , scriptSetRGBCB),
    makeEntry(selShipHealthSolidGreen , scriptSetRGBCB),
    makeEntry(selShipHealthSolidYellow, scriptSetRGBCB),
    makeEntry(selShipHealthSolidRed   , scriptSetRGBCB),
    makeEntry(selFuelColorGreen       , scriptSetRGBCB),
    makeEntry(selFuelColorRed         , scriptSetRGBCB),
    makeEntry(selFuelColorDarkGreen   , scriptSetRGBCB),
    makeEntry(selFuelColorDarkRed     , scriptSetRGBCB),
    makeEntry(selDepthSelectMultiplier, scriptSetReal32CB),
    //distance limitation movement etc.
    makeEntry(MAX_MOVE_DISTANCE    , scriptSetUdwordCB),
    makeEntry(MAX_HARVEST_DISTANCE , scriptSetUdwordCB),
    //decloaking time variables

    makeEntry(TW_NOONE_SEE_TIME , scriptSetReal32CB),
    makeEntry(TW_FIRST_SEE_TIME , scriptSetReal32CB),
    makeEntry(TW_SECOND_SEE_TIME , scriptSetReal32CB),
    makeEntry(TW_FIRST_SEE_PERCENTAGE , scriptSetUdwordCB),
    makeEntry(TW_SECOND_SEE_PERCENTAGE , scriptSetUdwordCB),

    //multiplayer game screen tweakables

    makeEntry(mgListOfGamesFontName,        scriptSetStringCB),
    makeEntry(mgChatWindowFontName,         scriptSetStringCB),
    makeEntry(mgUserNameFontName,           scriptSetStringCB),
    makeEntry(mgCurrentChannelFontName,     scriptSetStringCB),
    makeEntry(mgChannelListTitleFontName,   scriptSetStringCB),
    makeEntry(mgChannelListFontName,        scriptSetStringCB),
    makeEntry(mgGameListTitleFontName,      scriptSetStringCB),
    makeEntry(mgGameChatFontName,           scriptSetStringCB),
    makeEntry(mgGameUserNameFontName,       scriptSetStringCB),
    makeEntry(mgCurrentGameFontName,        scriptSetStringCB),
    makeEntry(mgOptionsFontName,            scriptSetStringCB),
    makeEntry(mgConnectingFontName,         scriptSetStringCB),
    makeEntry(mgMessageBoxFontName,         scriptSetStringCB),
    makeEntry(lgProtocalFontName,           scriptSetStringCB),

    makeEntry(mgGameWhisperedColor,         scriptSetRGBCB),
    makeEntry(mgGamePrivateChatColor,       scriptSetRGBCB),
    makeEntry(mgGameNormalChatColor,        scriptSetRGBCB),
    makeEntry(mgGameMessageChatColor,       scriptSetRGBCB),
    makeEntry(mgWhisperedColor,             scriptSetRGBCB),
    makeEntry(mgPrivateChatColor,           scriptSetRGBCB),
    makeEntry(mgNormalChatColor,            scriptSetRGBCB),
    makeEntry(mgMessageChatColor,           scriptSetRGBCB),
    makeEntry(mgUserNameColor,              scriptSetRGBCB),
    makeEntry(mgCurrentChannelColor,        scriptSetRGBCB),
    makeEntry(mgChannelTitleColor,          scriptSetRGBCB),
    makeEntry(mgChannelListNormalColor,     scriptSetRGBCB),
    makeEntry(mgChannelListSelectedColor,   scriptSetRGBCB),
    makeEntry(mgTitleFlashColor,            scriptSetRGBCB),
    makeEntry(mgGameListTitleColor,         scriptSetRGBCB),
    makeEntry(mgGameListNormalColor,        scriptSetRGBCB),
    makeEntry(mgGameListSelectedColor,      scriptSetRGBCB),
    makeEntry(mgGameListStartedColor,       scriptSetRGBCB),
    makeEntry(mgGameListDiffVersionColor,   scriptSetRGBCB),
    makeEntry(mgCurrentGameColor,           scriptSetRGBCB),
    makeEntry(mgNumberOfHumansColor,        scriptSetRGBCB),
    makeEntry(mgNumberOfComputersColor,     scriptSetRGBCB),
    makeEntry(mgConnectingStatusColor,      scriptSetRGBCB),
    makeEntry(mgMessageBoxColor,            scriptSetRGBCB),
    makeEntry(lgProtocalFontColor,          scriptSetRGBCB),

    // tweakable for the player list in the tactical overlay
    makeEntry(TO_PLAYERLIST_Y,              scriptSetSdwordCB),
    makeEntry(TO_PLAYERLIST_X,              scriptSetSdwordCB),
    makeEntry(alliescolor,                  scriptSetRGBCB),

    //fleet intelligence
    makeEntry(poHeadingFontName,            scriptSetStringCB),
    makeEntry(poTextFontName,               scriptSetStringCB),



    // tweakables for the starting resource amounts small, medium, large.
    makeEntry(resourceStartSmall,           scriptSetSdwordCB),
    makeEntry(resourceStartMedium,          scriptSetSdwordCB),
    makeEntry(resourceStartLarge,           scriptSetSdwordCB),

    // tweakables from the in game chatting system
    makeEntry(gcGamePrivateChatColor,       scriptSetRGBCB),
    makeEntry(gcGameWhisperedColor,         scriptSetRGBCB),
    makeEntry(gcGameNormalChatColor,        scriptSetRGBCB),
    makeEntry(maxlines,                     scriptSetSdwordCB),

    makeEntry(horseRaceRenderTime  , scriptSetReal32CB),
    makeEntry(horseTotalNumBars, scriptSetBarsToDo),

    makeEntry(tweakCheckMissileTargetLossRate  , scriptSetSdwordCB),

    makeEntry(tweakNumTimesCheck  , scriptSetSdwordCB),
    makeEntry(tweakMinDistBeforeTargetLoss  , scriptSetReal32CB),
    makeEntry(tweakRandomNess[Evasive]  , scriptSetReal32CB),
    makeEntry(tweakRandomNess[Neutral]  , scriptSetReal32CB),
    makeEntry(tweakRandomNess[Aggressive]  , scriptSetReal32CB),
    makeEntry(tweakBoostIfDodging  , scriptSetReal32CB),
    makeEntry(tweakBoostIfFlightMan  , scriptSetReal32CB),

    makeEntry(tpGameCreated.flag,                       scriptSetBitUword),
//    makeEntry(tpGameCreated.numComputers,               scriptSetUbyteCB),
//    makeEntry(tpGameCreated.startingFleet,              scriptSetUbyteCB),
//    makeEntry(tpGameCreated.bountySize,                 scriptSetUbyteCB),
//    makeEntry(tpGameCreated.startingResources,          scriptSetUbyteCB),
//    makeEntry(tpGameCreated.resourceInjectionInterval,  scriptSetUdwordCB),
//    makeEntry(tpGameCreated.resourceInjectionsAmount,   scriptSetUdwordCB),
//    makeEntry(tpGameCreated.resourceLumpSumTime,        scriptSetUdwordCB),
//    makeEntry(tpGameCreated.resourceLumpSumAmount,      scriptSetUdwordCB),
    makeEntry(tpGameCreated.aiplayerDifficultyLevel,    scriptSetUbyteCB),
    makeEntry(tpGameCreated.aiplayerBigotry,            scriptSetUbyteCB),

    makeEntry(GC_SCROLL_TIME,                           scriptSetReal32CB),

    makeEntry(attrVELOCITY_TOWARDS,                     scriptSetReal32CB),
    makeEntry(attrVELOCITY_TOWARDS_DEVIATION,           scriptSetReal32CB),
    makeEntry(attrKillerCollDamage,                     scriptSetReal32CB),
    makeEntry(attrHEADSHOTVELOCITY_TOWARDS,             scriptSetReal32CB),
    makeEntry(attrHEADSHOTVELOCITY_TOWARDS_DEVIATION,   scriptSetReal32CB),
    makeEntry(attrHEADSHOTKillerCollDamage,             scriptSetReal32CB),

    makeEntry(attrRING_WIDTH_SCALE,                     scriptSetReal32CB),

    { "capitalSideModifiers[TRANS_UP]",                 scriptSetReal32CB, &capitalSideModifiers[TRANS_UP] },
    { "capitalSideModifiers[TRANS_DOWN]",               scriptSetReal32CB, &capitalSideModifiers[TRANS_DOWN] },
    { "capitalSideModifiers[TRANS_RIGHT]",              scriptSetReal32CB, &capitalSideModifiers[TRANS_RIGHT] },
    { "capitalSideModifiers[TRANS_LEFT]",               scriptSetReal32CB, &capitalSideModifiers[TRANS_LEFT] },
    { "capitalSideModifiers[TRANS_FORWARD]",            scriptSetReal32CB, &capitalSideModifiers[TRANS_FORWARD] },
    { "capitalSideModifiers[TRANS_BACK]",               scriptSetReal32CB, &capitalSideModifiers[TRANS_BACK] },

    makeEntry(friendlyFireModifier,                     scriptSetReal32CB),

    makeEntry(TW_RETIRE_MONEYBACK_RATIO,                scriptSetReal32CB),
    makeEntry(TW_ProximitySensorSearchRate,             scriptSetSdwordCB),

    makeEntry(mrNumberDoublePressTime,                  scriptSetReal32CB),
    makeEntry(mrFastMoveShipClickTime,                  scriptSetReal32CB),
    makeEntry(mrFastMoveShipClickX,                     scriptSetSdwordCB),
    makeEntry(mrFastMoveShipClickY,                     scriptSetSdwordCB),

    makeEntry(TW_ScuttleReconfirmTime,                  scriptSetReal32CB),

    makeEntry(TW_Bounty_ResourcesCollected,                  scriptSetReal32CB),
    makeEntry(TW_Bounty_ResourcesInPossesion,                  scriptSetReal32CB),
    makeEntry(TW_Bounty_CurrentShipWorth,                  scriptSetReal32CB),
    makeEntry(TW_Bounty_AllyStrength,                  scriptSetReal32CB),
    makeEntry(TW_Bounty_ResourcesKilled,                  scriptSetReal32CB),

    makeEntry(TW_HARVESTER_DAMAGE_PER_SECOND,                  scriptSetReal32CB),

    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_Mothership]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_Mothership] },
    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_HeavyCruiser]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_HeavyCruiser] },
    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_Carrier]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_Carrier] },
    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_Destroyer]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_Destroyer] },
    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_Frigate]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_Frigate] },
    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_Corvette]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_Corvette] },
    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_Fighter]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_Fighter] },
    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_Resource]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_Resource] },
    { "capShipLieFlatInGeneralDistanceTolerance[CLASS_NonCombat]",                 scriptSetReal32CB, &capShipLieFlatInGeneralDistanceTolerance[CLASS_NonCombat] },

    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Mothership]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Mothership] },
    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_HeavyCruiser]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_HeavyCruiser] },
    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Carrier]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Carrier] },
    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Destroyer]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Destroyer] },
    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Frigate]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Frigate] },
    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Corvette]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Corvette] },
    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Fighter]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Fighter] },
    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Resource]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_Resource] },
    { "capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_NonCombat]",                 scriptSetReal32SqrCB, &capShipLieFlatInGeneralSpeedSqrTolerance[CLASS_NonCombat] },

    makeEntry(TW_HOLDING_PATTERN_PATHTIME,                  scriptSetReal32CB),
    makeEntry(TW_HOLDING_PATTERN_PATH_DISTANCE,                  scriptSetReal32CB),
    makeEntry(TW_HOLDING_PATTERN_NUM_TURNS,                  scriptSetReal32CB),
    makeEntry(TW_HOLDING_PATTERN_PATH_MAX_SPEED,                  scriptSetReal32CB),

    makeEntry(TW_MOVE_ATTACK_POINT_LINE_COLOR_IN_RANGE,        scriptSetRGBCB),
    makeEntry(TW_MOVE_ATTACK_POINT_LINE_COLOR_NOT_IN_RANGE,        scriptSetRGBCB),
    makeEntry(TW_MOVE_ATTACK_PIE_COLOR,        scriptSetRGBCB),
    makeEntry(TW_MOVE_ATTACK_SPOKE_COLOR,        scriptSetRGBCB),

    makeEntry(MINING_BASE_ROTATION_SPEED,scriptSetReal32CB),
    makeEntry(MINING_BASE_VERTICAL_DIST_HACK,scriptSetReal32CB),
    makeEntry(TW_HOLDING_PATTERN_HEADING_COEFFICIENT,scriptSetReal32CB),
    makeEntry(TW_HOLDING_PATTERN_RIGHT_COEFFICIENT,scriptSetReal32CB),

    makeEntry(selShipHealthGreenFactor,scriptSetReal32CB),
    makeEntry(selShipHealthYellowFactor,scriptSetReal32CB),

    makeEntry(COLLISION_CLEAR_TIME,scriptSetReal32CB),
    makeEntry(GLANCE_COLLISION_VEL_RATIO,scriptSetReal32SqrCB),
    makeEntry(COLLISION_REPULSE_FACTOR,scriptSetReal32CB),

    makeEntry(versionColor,scriptSetRGBCB),

    makeEntry(TW_BountySizes[0],scriptSetSdwordCB),
    makeEntry(TW_BountySizes[1],scriptSetSdwordCB),
    makeEntry(TW_BountySizes[2],scriptSetSdwordCB),

    //scarred scaffold rotation tweaks
    makeEntry(SCAFFOLD_SCARRED_ROTX,scriptSetReal32CB),
    makeEntry(SCAFFOLD_SCARRED_ROTY,scriptSetReal32CB),
    makeEntry(SCAFFOLD_SCARRED_ROTZ,scriptSetReal32CB),
    makeEntry(SCAFFOLDFINGERA_SCARRED_ROTX,scriptSetReal32CB),
    makeEntry(SCAFFOLDFINGERA_SCARRED_ROTY,scriptSetReal32CB),
    makeEntry(SCAFFOLDFINGERA_SCARRED_ROTZ,scriptSetReal32CB),
    makeEntry(SCAFFOLDFINGERB_SCARRED_ROTX,scriptSetReal32CB),
    makeEntry(SCAFFOLDFINGERB_SCARRED_ROTY,scriptSetReal32CB),
    makeEntry(SCAFFOLDFINGERB_SCARRED_ROTZ,scriptSetReal32CB),

    makeEntry(SALVAGE_MIN_RETROFIT_TIME,scriptSetReal32CB),
    makeEntry(SALVAGE_MAX_RETROFIT_TIME,scriptSetReal32CB),
    makeEntry(TW_R1_MOTHERSHIP_DOOR_OFFSET_MODIFIER,scriptSetReal32CB),
    makeEntry(TW_SCUTTLE_CONFIRM_TIME,scriptSetReal32CB),

    makeEntry(TW_MOVE_HYPERSPACE_COLOUR,scriptSetRGBCB),
    makeEntry(TW_MOVE_POINT_HYPERSPACE_LINE_COLOR,scriptSetRGBCB),
    makeEntry(TW_MOVE_SPOKE_HYPERSPACE_COLOR,scriptSetRGBCB),
    makeEntry(TW_MOVE_HEADING_HYPERSPACE_COLOR,scriptSetRGBCB),

    makeEntry(TW_MULTIPLAYER_HYPERSPACE_WAIT_IN_ETHER_TIME,scriptSetReal32CB),

    { "hyperspace", scriptSetHyperspaceCostCB, &TW_HyperSpaceCostStruct },

    makeEntry(TW_RU_READOUT_COLOR_BAD,scriptSetRGBCB),
    makeEntry(TW_RU_READOUT_COLOR_GOOD,scriptSetRGBCB),

#if DEM_AUTO_DEMO
    makeEntry(demAutoDemoWaitTime,scriptSetReal32CB),
#endif

    makeEntry(TW_PING_MAX_DISPERSAL,scriptSetReal32CB),

    makeEntry(TW_HW_PING_COLOUR_IN,scriptSetRGBCB),
    makeEntry(TW_HW_PING_RATE_IN,scriptSetReal32CB),
    makeEntry(TW_HW_PING_MIN_SIZE_IN,scriptSetReal32CB),
    makeEntry(TW_HW_PING_MAX_SIZE_IN,scriptSetReal32CB),

    makeEntry(TW_HW_PING_COLOUR_OUT,scriptSetRGBCB),
    makeEntry(TW_HW_PING_RATE_OUT,scriptSetReal32CB),
    makeEntry(TW_HW_PING_MIN_SIZE_OUT,scriptSetReal32CB),
    makeEntry(TW_HW_PING_MAX_SIZE_OUT,scriptSetReal32CB),

    makeEntry(TW_HYPERSPACE_TELEGRAPH_WAIT_TIME,scriptSetReal32CB),


    makeEntry(TW_SV_MAN_VERY_LOW,scriptSetReal32CB),
    makeEntry(TW_SV_MAN_LOW,scriptSetReal32CB),
    makeEntry(TW_SV_MAN_MEDIUM,scriptSetReal32CB),
    makeEntry(TW_SV_MAN_HIGH,scriptSetReal32CB),

    makeEntry(selFontName0, scriptSetStringPtrCB),
    makeEntry(selFontName1, scriptSetStringPtrCB),
    makeEntry(selFontName2, scriptSetStringPtrCB),
    makeEntry(selFontName3, scriptSetStringPtrCB),

    makeEntry(TW_R1_DOOR_DOCK_TOLERANCE, scriptSetReal32CB),

    makeEntry(TW_GRAVWELL_SPHERE_COLOUR,scriptSetRGBCB),
    makeEntry(TW_CLOAKGENERATOR_SPHERE_COLOUR,scriptSetRGBCB),
    makeEntry(TW_DFGF_SPHERE_COLOUR,scriptSetRGBCB),
    makeEntry(TW_PROXIMITY_SPHERE_COLOUR,scriptSetRGBCB),
    makeEntry(TW_PROXIMITY_SPHERE_COLOUR_FOUND,scriptSetRGBCB),
    makeEntry(TW_PROXIMITY_RING_COLOUR,scriptSetRGBCB),

    makeEntry(lagSlowCompX, scriptSetUdwordCB),
    makeEntry(lagSlowCompY, scriptSetUdwordCB),
    makeEntry(lagSlowIntX,  scriptSetUdwordCB),
    makeEntry(lagSlowIntY,  scriptSetUdwordCB),
    makeEntry(SLOW_INTERNETPRINTTIME, scriptSetReal32CB),

    makeEntry(TUTORIAL_KAS_UPDATE_MASK, scriptSetSdwordCB),
    makeEntry(TUTORIAL_KAS_UPDATE_FRAME, scriptSetSdwordCB),

    makeEntry(SP_KAS_UPDATE_FRAME, scriptSetSdwordCB),

    makeEntry(MINE_DO_SEARCH_MASK, scriptSetSdwordCB),

    makeEntry(CHECKBOGEY_FRAME, scriptSetSdwordCB),

    makeEntry(SPHERE_CHECK_TARGET_MASK, scriptSetSdwordCB),
    makeEntry(SPHERE_CHECK_TARGET_FRAME, scriptSetSdwordCB),

    makeEntry(CHECK_ATTACKMOVE_TO_MOVE_RATE, scriptSetSdwordCB),
    makeEntry(CHECK_ATTACKMOVE_TO_MOVE_FRAME, scriptSetSdwordCB),

    makeEntry(FORMATION_ERROR_CALCULATE_RATE, scriptSetSdwordCB),
    makeEntry(FORMATION_ERROR_CALCULATE_FRAME, scriptSetSdwordCB),

    makeEntry(EnemyNearByCheckRate, scriptSetSdwordCB),
    makeEntry(EnemyNearByCheckFrame, scriptSetSdwordCB),
    makeEntry(RetreatCheckRate, scriptSetSdwordCB),
    makeEntry(RetreatCheckFrame, scriptSetSdwordCB),

    makeEntry(ATTACKMEMORY_CLEANUP_RATE, scriptSetSdwordCB),
    makeEntry(ATTACKMEMORY_CLEANUP_FRAME, scriptSetSdwordCB),

    makeEntry(TW_ProximitySensorSearchRate, scriptSetSdwordCB),
    makeEntry(TW_ProximitySensorSearchFrame, scriptSetSdwordCB),

    makeEntry(CHECK_PASSIVE_ATTACK_RATE, scriptSetSdwordCB),
    makeEntry(CHECK_PASSIVE_ATTACK_WHILEMOVING_RATE, scriptSetSdwordCB),

    makeEntry(REFRESH_RENDERLIST_RATE, scriptSetSdwordCB),
    makeEntry(REFRESH_RENDERLIST_FRAME, scriptSetSdwordCB),

    makeEntry(REFRESH_MINORRENDERLIST_RATE, scriptSetSdwordCB),
    makeEntry(REFRESH_MINORRENDERLIST_FRAME, scriptSetSdwordCB),

    makeEntry(REFRESH_COLLBLOB_RATE, scriptSetSdwordCB),
    makeEntry(REFRESH_COLLBLOB_FRAME, scriptSetSdwordCB),

    makeEntry(REFRESH_COLLBLOB_BATTLEPING_FRAME, scriptSetSdwordCB),

    makeEntry(REFRESH_RESEARCH_RATE, scriptSetSdwordCB),
    makeEntry(REFRESH_RESEARCH_FRAME, scriptSetSdwordCB),

    makeEntry(CHECK_BOUNTIES_RATE, scriptSetSdwordCB),
    makeEntry(CHECK_BOUNTIES_FRAME, scriptSetSdwordCB),

    makeEntry(CLOAKGENERATOR_CLOAKCHECK_RATE, scriptSetSdwordCB),

    makeEntry(TW_GLOBAL_PASSIVE_ATTACK_CANCEL_MODIFIER,scriptSetReal32CB),

    makeEntry(twHyperedIntoDamage,scriptSetReal32CB),
    makeEntry(twHyperingInDamage,scriptSetReal32CB),

    makeEntry(TW_BURST_ATTACK_FORCE_MODIFIER,scriptSetReal32CB),

    makeEntry(armourPiercingModifier,scriptSetReal32CB),

    endEntry
};

