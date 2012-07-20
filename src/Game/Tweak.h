
#ifndef ___TWEAK_H
#define ___TWEAK_H

#include "Types.h"
#include "color.h"
#include "ShipDefs.h"
#include "ClassDefs.h"
#include "ObjTypes.h"

/*=============================================================================
    Insert global tweakables here:
=============================================================================*/
extern bool shipsRetaliate;
extern uword singleMachineNumPlayers;
extern char useMission[50];
extern real32 RETALIATE_ZONE;
extern color R1BulletColor;
extern color R2BulletColor;

extern ubyte AUTOSTABILIZE_FRAME_KICK_IN;

extern real32 BLAST_CONSTANT;
extern real32 BLAST_RADIUS_MULTIPLE_DEFAULT;
extern real32 BLAST_DAMAGE_MAX_PERCENT;

extern real32 BLAST_CONSTANT_SCUTTLE;
extern real32 BLAST_RADIUS_SCUTTLE_BONUS;
extern real32 BLAST_DAMAGE_MAX_PERCENT_SCUTTLE;

extern real32 YAWEXPLOSIONROCKLO;
extern real32 YAWEXPLOSIONROCKHI;
extern real32 PITCHEXPLOSIONROCKLO;
extern real32 PITCHEXPLOSIONROCKHI;
extern real32 ROLLEXPLOSIONROCKLO;
extern real32 ROLLEXPLOSIONROCKHI;

extern real32 YAWROCKLO;
extern real32 YAWROCKHI;
extern real32 PITCHROCKLO;
extern real32 PITCHROCKHI;
extern real32 ROLLROCKLO;
extern real32 ROLLROCKHI;

extern real32 YAWCAREENLO;
extern real32 YAWCAREENHI;
extern real32 PITCHCAREENLO;
extern real32 PITCHCAREENHI;
extern real32 ROLLCAREENLO;
extern real32 ROLLCAREENHI;

extern real32 YAWCRAZYLO;
extern real32 YAWCRAZYHI;
extern real32 PITCHCRAZYLO;
extern real32 PITCHCRAZYHI;
extern real32 ROLLCRAZYLO;
extern real32 ROLLCRAZYHI;

extern real32 RENDER_VIEWABLE_DISTANCE_SQR;
extern real32 RENDER_MAXVIEWABLE_DISTANCE_SQR;
extern real32 RENDER_VIEWABLE_FADE;
extern real32 RENDER_MAXVIEWABLE_FADE;

extern real32 RENDER_LIMIT_MOTHERSHIP;
extern real32 RENDER_FADE_MOTHERSHIP;
extern real32 RENDER_LIMIT_HEAVYCRUISER;
extern real32 RENDER_FADE_HEAVYCRUISER;
extern real32 RENDER_LIMIT_CARRIER;
extern real32 RENDER_FADE_CARRIER;
extern real32 RENDER_LIMIT_DESTROYER;
extern real32 RENDER_FADE_DESTROYER;
extern real32 RENDER_LIMIT_FRIGATE;
extern real32 RENDER_FADE_FRIGATE;
extern real32 RENDER_LIMIT_CORVETTE;
extern real32 RENDER_FADE_CORVETTE;
extern real32 RENDER_LIMIT_FIGHTER;
extern real32 RENDER_FADE_FIGHTER;
extern real32 RENDER_LIMIT_RESOURCE;
extern real32 RENDER_FADE_RESOURCE;
extern real32 RENDER_LIMIT_NONCOMBAT;
extern real32 RENDER_FADE_NONCOMBAT;

extern real32 BG_RADIUS;

extern real32 VERYBIGSHIP_SIZE;

extern sdword REFRESH_RENDERLIST_RATE;

extern sdword REFRESH_COLLBLOB_RATE;

extern real32 OUT_OF_FUEL_FRICTION;
extern real32 SHIP_CRAZY_FRICTION;
extern real32 DONT_DRIFT_FRICTION;
extern real32 CONSIDERED_STILL_LO;
extern real32 CONSIDERED_STILL_HI;

//mainrgn tweak variables
extern color TW_MOVE_HEADING_COLOR;
extern color TW_MOVE_SPOKE_COLOR;
extern color  TW_MOVE_PIZZA_COLOR;
extern color  TW_MOVE_ENEMY_COLOR;
extern color  TW_MOVE_POINT_LINE_COLOR;
extern color  TW_MOVE_ORIGIN_COLOR;

extern color  TW_SELECT_BOX_COLOR;
extern color  TW_SHIP_LINE_COLOR;
extern color  TW_SHIP_LINE_CLOSEST_COLOR;
extern real32 pieCircleSizeMax;
extern real32 pieCircleSizeMin;

extern color  TW_DISTANCE_READOUT_COLOR;
extern char   TW_DISTANCE_READOUT_FONT[];

extern color  TW_CURSORTEXT_COLOR;
extern real32 TW_CURSORTEXT_DELAY_TIME;
extern sdword TW_CURSORTEXT_X;
extern sdword TW_CURSORTEXT_Y;
extern char   TW_CURSORTEXT_FONT[];

extern color  TW_MOVETO_LINE_COLOR;
extern color  TW_MOVETO_PULSE_COLOR;
extern color  TW_ATTMOVETO_LINE_COLOR;
extern color  TW_ATTMOVETO_PULSE_COLOR;
extern real32 TW_MOVETO_CIRCLE_RADIUS;
extern real32 TW_MOVETO_PULSE_SPEED_SCALE;
extern real32 TW_MOVETO_ENDCIRCLE_RADIUS;

//TO special field ships sphere color
extern color TO_SPHERE_COLOR;

//For Special Ships field TO
extern color  TO_PULSE1_COLOR;
extern real32 TO_PULSE1_SPEED;
extern real32 TO_DELAY_PULSE1;

extern color  TO_PULSE2_COLOR;
extern real32 TO_PULSE2_SPEED;
extern real32 TO_DELAY_PULSE2;

extern color  TO_CROSS_COLOR1;
extern color  TO_CROSS_COLOR2;

extern real32 TO_CROSS_HALF_SIZE;
extern real32 TO_PULSE1_SIZE;
extern real32 TO_PULSE2_SIZE;

extern uword TW_MESSAGE_DELAY_TIME;

extern real32 FLASH_TIMER;

extern real32 COLLDAMAGE_VELOCITY_TANGENT_SCALE;
extern real32 COLLDAMAGE_DEFAULTMIN_FRACTION;
extern real32 COLLDAMAGE_DEFAULTMAX_FRACTION;

extern real32 PLAYERLOSE_SHIPDEATHTIMEMIN;
extern real32 PLAYERLOSE_SHIPDEATHTIMEMAX;

extern real32 SCUTTLE_SHIPDEATHTIMEMIN;
extern real32 SCUTTLE_SHIPDEATHTIMEMAX;

extern real32 WAIT_TIME_TO_QUIT;
extern real32 WAIT_TIME_TO_WIN;

//Mission Map tweakables
extern color  MM_SPHERE_COLOR;
extern real32 MM_SELECTED_FLASH_SPEED;
extern real32 MM_CAMERA_DISTANCE;
extern real32 MM_MESH_SCALE_FACTOR;

extern sdword MIN_SHIPS_TO_FIGHT;
extern real32 FACEOFF_DISTANCE;
extern real32 STATFIGHT_TIMEOUT_TIME;

//various front end stuff
extern sdword ferSelectedDotMarginX;
extern sdword ferSelectedDotMarginY;
extern sdword feMenuScreenMarginX, feMenuScreenMarginY;
extern sdword feMenuOverlapMarginX, feMenuOverlapMarginY;

// construction manager tweakables
extern char   cmShipListFontName[];
extern char   svShipStatFontName[];
extern sdword cmSelectTopAdjust;
extern sdword cmSelectBottomAdjust;
extern sdword cmSelectLeftAdjust;
extern sdword cmSelectRightAdjust;
extern color  cmStandardTextColor;
extern color  cmClassHeadingTextColor;
extern color  cmSelectionTextColor;
extern color  cmBuildingTextColor;
extern color  cmProgressShipColor;
extern color  cmProgressTotalColor;
extern color  cmCapsExceededTextColor;

// Launch manager tweakables
extern char   lmShipListFontName[];

extern color  lmShipListTextColor;
extern color  lmShipSelectedTextColor;
extern color  lmUsedColor;
extern color  lmAvailableColor;

// Research Manager tweakables
extern char   rmTechListFontName[];
extern char   rmTechInfoFontName[];
extern color  rmSelectionTextColor;
extern color  rmResearchingTextColor;
extern color  rmCantResearchTextColor;
extern color  rmStandardTextColor;
extern color  rmClassHeadingTextColor;
extern color  rmProgressToGoColor;
extern color  rmProgressDoneColor0;
extern color  rmProgressDoneColor1;
extern color  rmLabActiveColor;
extern color  rmPulseColor;
extern color  rmNoResearchItemColor;
extern color  rmMarqueeOnColor;
extern color  rmMarqueeSemiOnColor;
extern color  rmMarqueeOffColor;

// Info Overlay Tweakables
extern char   ioShipListFontName[];
extern color  ioListTextColor;
extern color  ioSelectedTextColor;


// fleet intelligence

extern char poHeadingFontName[];
extern char poTextFontName[];



//for selections
extern real32 selMinSelectionRadius;

// for gun tuning
extern real32 GUNTUNE_SPEED;

// for tuning of max move distances etc.
extern udword MAX_MOVE_DISTANCE;
extern udword MAX_HARVEST_DISTANCE;

//decloaking attack time variables
extern real32 TW_NOONE_SEE_TIME;
extern real32 TW_FIRST_SEE_TIME;
extern real32 TW_SECOND_SEE_TIME;
extern udword TW_FIRST_SEE_PERCENTAGE;
extern udword TW_SECOND_SEE_PERCENTAGE;

//Multiplayer game screen tweakables, fonts and colors, a whole fricking whack-o-them

extern char  mgListOfGamesFontName[];
extern char  mgChatWindowFontName[];
extern char  mgUserNameFontName[];
extern char  mgCurrentChannelFontName[];
extern char  mgChannelListTitleFontName[];
extern char  mgChannelListFontName[];
extern char  mgGameListTitleFontName[];
extern char  mgGameChatFontName[];
extern char  mgGameUserNameFontName[];
extern char  mgCurrentGameFontName[];
extern char  mgOptionsFontName[];
extern char  mgConnectingFontName[];
extern char  mgMessageBoxFontName[];
extern char  lgProtocalFontName[];

extern color mgGameWhisperedColor;
extern color mgGamePrivateChatColor;
extern color mgGameNormalChatColor;
extern color mgGameMessageChatColor;
extern color mgWhisperedColor;
extern color mgPrivateChatColor;
extern color mgNormalChatColor;
extern color mgMessageChatColor;
extern color mgUserNameColor;
extern color mgCurrentChannelColor;
extern color mgChannelTitleColor;
extern color mgChannelListNormalColor;
extern color mgChannelListSelectedColor;
extern color mgTitleFlashColor;
extern color mgGameListTitleColor;
extern color mgGameListNormalColor;
extern color mgGameListSelectedColor;
extern color mgGameListStartedColor;
extern color mgGameListDiffVersionColor;
extern color mgCurrentGameColor;
extern color mgNumberOfHumansColor;
extern color mgNumberOfComputersColor;
extern color mgConnectingStatusColor;
extern color mgMessageBoxColor;
extern color lgProtocalFontColor;

extern sdword TO_PLAYERLIST_Y;
extern sdword TO_PLAYERLIST_X;
extern color  alliescolor;

extern sdword resourceStartSmall;
extern sdword resourceStartMedium;
extern sdword resourceStartLarge;

extern sdword resourceStartTutorial;

extern color  gcGamePrivateChatColor;
extern color  gcGameWhisperedColor;
extern color  gcGameNormalChatColor;
extern sdword maxlines;

extern real32 horseRaceRenderTime;
extern struct HorseRaceBars horseTotalNumBars;

extern sdword tweakCheckMissileTargetLossRate;
extern sdword tweakNumTimesCheck;
extern real32 tweakMinDistBeforeTargetLoss;

extern real32 tweakRandomNess[NUM_TACTICS_TYPES];
extern real32 tweakBoostIfDodging;
extern real32 tweakBoostIfFlightMan;

extern real32 GC_SCROLL_TIME;

extern real32 attrVELOCITY_TOWARDS;
extern real32 attrVELOCITY_TOWARDS_DEVIATION;
extern real32 attrKillerCollDamage;
extern real32 attrHEADSHOTVELOCITY_TOWARDS;
extern real32 attrHEADSHOTVELOCITY_TOWARDS_DEVIATION;
extern real32 attrHEADSHOTKillerCollDamage;

extern real32 attrRING_WIDTH_SCALE;

extern real32 capitalSideModifiers[6];       // NUM_TRANS_DEGOFFREEDOM

extern real32 friendlyFireModifier;
extern real32 TW_RETIRE_MONEYBACK_RATIO;

extern sdword TW_ProximitySensorSearchRate;
extern real32 TW_ScuttleReconfirmTime;

extern real32 TW_Bounty_ResourcesCollected;
extern real32 TW_Bounty_ResourcesInPossesion;
extern real32 TW_Bounty_CurrentShipWorth;
extern real32 TW_Bounty_AllyStrength;
extern real32 TW_Bounty_ResourcesKilled;

extern real32 TW_HARVESTER_DAMAGE_PER_SECOND;

extern real32 capShipLieFlatInGeneralDistanceTolerance[NUM_CLASSES];
extern real32 capShipLieFlatInGeneralSpeedSqrTolerance[NUM_CLASSES];

extern real32 TW_HOLDING_PATTERN_PATHTIME;
extern real32 TW_HOLDING_PATTERN_PATH_DISTANCE;
extern real32 TW_HOLDING_PATTERN_NUM_TURNS;
extern real32 TW_HOLDING_PATTERN_PATH_MAX_SPEED;

extern color TW_MOVE_ATTACK_POINT_LINE_COLOR_IN_RANGE;
extern color TW_MOVE_ATTACK_POINT_LINE_COLOR_NOT_IN_RANGE;
extern color TW_MOVE_ATTACK_PIE_COLOR;
extern color TW_MOVE_ATTACK_SPOKE_COLOR;

extern real32 MINING_BASE_ROTATION_SPEED;
extern real32 MINING_BASE_VERTICAL_DIST_HACK;

extern real32 TW_HOLDING_PATTERN_HEADING_COEFFICIENT;
extern real32 TW_HOLDING_PATTERN_RIGHT_COEFFICIENT;

extern real32 selShipHealthGreenFactor;
extern real32 selShipHealthYellowFactor;

extern real32 COLLISION_CLEAR_TIME;
extern real32 GLANCE_COLLISION_VEL_RATIO;
extern real32 COLLISION_REPULSE_FACTOR;

extern sdword TW_BountySizes[];

//scarred scaffold rotation tweaks
extern real32 SCAFFOLD_SCARRED_ROTX;
extern real32 SCAFFOLD_SCARRED_ROTY;
extern real32 SCAFFOLD_SCARRED_ROTZ;
extern real32 SCAFFOLDFINGERA_SCARRED_ROTX;
extern real32 SCAFFOLDFINGERA_SCARRED_ROTY;
extern real32 SCAFFOLDFINGERA_SCARRED_ROTZ;
extern real32 SCAFFOLDFINGERB_SCARRED_ROTX;
extern real32 SCAFFOLDFINGERB_SCARRED_ROTY;
extern real32 SCAFFOLDFINGERB_SCARRED_ROTZ;

extern real32 SALVAGE_MIN_RETROFIT_TIME;
extern real32 SALVAGE_MAX_RETROFIT_TIME;
extern real32 TW_R1_MOTHERSHIP_DOOR_OFFSET_MODIFIER;

extern real32 TW_SCUTTLE_CONFIRM_TIME;

extern color TW_MOVE_HYPERSPACE_COLOUR;
extern color TW_MOVE_POINT_HYPERSPACE_LINE_COLOR;
extern color TW_MOVE_SPOKE_HYPERSPACE_COLOR;
extern color TW_MOVE_HEADING_HYPERSPACE_COLOR;

extern real32 TW_MULTIPLAYER_HYPERSPACE_WAIT_IN_ETHER_TIME;

typedef struct
{
    real32 min;
    real32 distanceSlope;
    real32 minDistance;
    real32 max;
    sdword canMpHyperspace;
}HW_Cost;

extern HW_Cost TW_HyperSpaceCostStruct[TOTAL_NUM_SHIPS];
extern color TW_RU_READOUT_COLOR_BAD;
extern color TW_RU_READOUT_COLOR_GOOD;

extern real32 TW_PING_MAX_DISPERSAL;

extern color TW_HW_PING_COLOUR_IN;
extern real32 TW_HW_PING_RATE_IN;
extern real32 TW_HW_PING_MIN_SIZE_IN;
extern real32 TW_HW_PING_MAX_SIZE_IN;

extern color TW_HW_PING_COLOUR_OUT;
extern real32 TW_HW_PING_RATE_OUT;
extern real32 TW_HW_PING_MIN_SIZE_OUT;
extern real32 TW_HW_PING_MAX_SIZE_OUT;

extern real32 TW_HYPERSPACE_TELEGRAPH_WAIT_TIME;

extern real32 TW_SV_MAN_VERY_LOW;      //below this number it will register as VERYLOW
extern real32 TW_SV_MAN_LOW;           //below this, but above previouw == LOW
extern real32 TW_SV_MAN_MEDIUM;        //below this, but above previouw == MEDIUM
extern real32 TW_SV_MAN_HIGH;          //below this, but above previouw == HIGH
                                //Above previous == VERY_HIGH
extern char *selFontName0;
extern char *selFontName1;
extern char *selFontName2;
extern char *selFontName3;

extern real32 TW_R1_DOOR_DOCK_TOLERANCE;

extern color TW_GRAVWELL_SPHERE_COLOUR;
extern color TW_CLOAKGENERATOR_SPHERE_COLOUR;
extern color TW_DFGF_SPHERE_COLOUR;
extern color TW_PROXIMITY_SPHERE_COLOUR;
extern color TW_PROXIMITY_SPHERE_COLOUR_FOUND;
extern color TW_PROXIMITY_RING_COLOUR;

// These variables are used to position the icons for slow computer and internet
// as well the value that lag has to reach before display is also here.
extern udword lagSlowCompX;
extern udword lagSlowCompY;

extern udword lagSlowIntX;
extern udword lagSlowIntY;

extern real32 SLOW_INTERNETPRINTTIME;




extern sdword TUTORIAL_KAS_UPDATE_MASK;
extern sdword TUTORIAL_KAS_UPDATE_FRAME;

extern sdword SP_KAS_UPDATE_FRAME;

extern sdword MINE_DO_SEARCH_MASK;

extern sdword CHECKBOGEY_FRAME;

extern sdword SPHERE_CHECK_TARGET_MASK;
extern sdword SPHERE_CHECK_TARGET_FRAME;

extern sdword CHECK_ATTACKMOVE_TO_MOVE_RATE;
extern sdword CHECK_ATTACKMOVE_TO_MOVE_FRAME;

extern sdword FORMATION_ERROR_CALCULATE_RATE;
extern sdword FORMATION_ERROR_CALCULATE_FRAME;

extern sdword EnemyNearByCheckRate;
extern sdword EnemyNearByCheckFrame;
extern sdword RetreatCheckRate;
extern sdword RetreatCheckFrame;

extern sdword ATTACKMEMORY_CLEANUP_RATE;
extern sdword ATTACKMEMORY_CLEANUP_FRAME;

extern sdword TW_ProximitySensorSearchRate;
extern sdword TW_ProximitySensorSearchFrame;

extern sdword CHECK_PASSIVE_ATTACK_RATE;
extern sdword CHECK_PASSIVE_ATTACK_WHILEMOVING_RATE;

extern sdword REFRESH_RENDERLIST_RATE;
extern sdword REFRESH_RENDERLIST_FRAME;

extern sdword REFRESH_MINORRENDERLIST_RATE;
extern sdword REFRESH_MINORRENDERLIST_FRAME;

extern sdword REFRESH_COLLBLOB_RATE;
extern sdword REFRESH_COLLBLOB_FRAME;

extern sdword REFRESH_COLLBLOB_BATTLEPING_FRAME;

extern sdword REFRESH_RESEARCH_RATE;
extern sdword REFRESH_RESEARCH_FRAME;

extern sdword CHECK_BOUNTIES_RATE;
extern sdword CHECK_BOUNTIES_FRAME;

extern sdword CLOAKGENERATOR_CLOAKCHECK_RATE;

extern real32 TW_GLOBAL_PASSIVE_ATTACK_CANCEL_MODIFIER;

extern real32 twHyperingInDamage;
extern real32 twHyperedIntoDamage;
extern real32 TW_BURST_ATTACK_FORCE_MODIFIER;

extern real32 armourPiercingModifier;
#endif
