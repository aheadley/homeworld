/*=============================================================================
    Name    : Battle.h
    Purpose : Defenitions for battle chatter evaluation and updating.

    Created 12/14/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___BATTLE_H
#define ___BATTLE_H     "Oh, the humanity!"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define BATTLE_VERBOSE_LEVEL    1               //control verbose output
#define BATTLE_ERROR_CHECKING   1               //basic error checking
#define BATTLE_TEXT             1               //print the text of the battle chatter
#define BATTLE_TUNING           1               //enable special tuning code

#else //HW_Debug

#define BATTLE_VERBOSE_LEVEL    0               //control verbose output
#define BATTLE_ERROR_CHECKING   0               //no error ckecking in retail
#define BATTLE_TEXT             0               //print the text of the battle chatter
#define BATTLE_TUNING           0               //enable special tuning code

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define BAT_RandomMask              (BIT15 - 1)
#define BAT_RandomTotal             BIT15
#define BAT_Delimiters              " ,\t"

//defaults for events
#define BAD_RandomWeight            (BAT_RandomTotal / 35)
#define BAD_MaxDistance             1000.0
#define BAD_ExpDistance             1.0
#define BAD_MinWavelength           0.0
#define BAD_ExpWavelength           1.0
#define BAD_MinProximity            0.0
#define BAD_ExpProximity            1.0

#define BAT_MouthpieceTries         4

#define BAT_StandingFriendlyPower   0.5f
#define BAT_StandingFriendlyValue   0.5f
#define BAT_StandingEnemyPower      0.5f
#define BAT_StandingEnemyValue      0.5f
/*
#define BAT_FriendlyLossPower       0.5f
#define BAT_FriendlyLossValue       0.5f
#define BAT_EnemyLossPower          0.5f
#define BAT_EnemyLossValue          0.5f
*/
#define BAT_ExpLossDevalue          2.0f

#define BAT_BSNKickingButt          4.0f
#define BAT_BSNWinning              1.5f
#define BAT_BSNNeutral              0.75f
#define BAT_BSNLosing               0.25f

#define BAT_BogeyAspect             0.5f

//chatter bits in the ship structure
#define BCB_FriendlyFire            0x01
#define BCB_WingmanLost             0x02
#define BCB_AlliedFire              0x04
#define BCB_DamageInTheRed          0x08

#define BAT_InTheRedFactor          0.25f       //at 1/4 damage, it's 'in the red'

#define BAT_ReallyRecentAttack      (RECENT_ATTACK_DURATION - UNIVERSE_UPDATE_RATE / 2)

#define BAT_RetreatModifier         1.5

#define BAT_GlobalFrequencyModifier 128
#define BAT_GlobalFrequencyMin      -10
#define BAT_GlobalFrequencyMax      1.5

/*=============================================================================
    Type definitions:
=============================================================================*/
//before being discarded, battle blobs are converted into this structure:
typedef struct
{
    //Note: because players' alliance status can change during the life of a ping,
    //it is possible that either of these numbers, but not both, could
    //become negative.  Please make all battle ping code tolerant of this.
    sdword nEnemyShips;                         //enemy ships in battle
    sdword nFriendlyShips;                      //friendly ships in battle; includes allies
    real32 radius;                              //radius of the battle blob
    sdword nShips;                              //total number of ships in battle

    real32 enemyFirepowerRecentlyLost;          //when, where and how much enemy juice was lost
    real32 enemyValueRecentlyLost;
    real32 whenEnemiesLost;

    real32 friendlyFirepowerRecentlyLost;       //when, where and how much friendly juice was lost
    real32 friendlyValueRecentlyLost;
    real32 whenFriendliesLost;

    Ship *ship[1];                              //list of ships in the battle
}
battleping;

//structure for a battle chatter event and it's heuristics
typedef struct
{
    char *eventName;                            //name of event for parsing
    sdword eventNumber;                         //event number
    udword randomWeight;                        //liklihood of this event being called ( / BAT_RandomTotal)
    double maxDistance;                         //max distance from camera event can be heard
    double expDistance;                         //exponent of camera distance liklihood curve
    double minWavelength;                       //how often this event can be said (1 / frequency) (0 = ignore)
    double expWavelength;                       //exponent of repeat time liklihood curve
    double minRepeatProximity;                  //min proximity for a repeat within minWavelength (0 = ignore)
    double expRepeatProximity;                  //exponent of the proximity probability curve
#if BATTLE_TEXT
    char **texts;                               //strings of what the ship might say (for debug printing)
#endif
    //end of static stuff; dynamic stuff follows
    real32 lastTimeSpoken;                      //universe time when this event was last actually queued up
    vector lastPlaceSpoken;                     //where the event was last said
}
battleevent;

//enumeration of all battle chatter speech events
typedef enum
{
    BCE_Wingman,                                //67. CHATTER_Fighter-Wingman[Report] (ALL SHIPS)
    BCE_PositiveBattle,                         //90. CHATTER_GenericGroup-PositiveBattle[Report] (ALL SHIPS)
    BCE_PositiveBattle_More,                    //90. CHATTER_GenericGroup-PositiveBattle[Report] (ALL SHIPS)
    BCE_NegativeBattle,                         //91. CHATTER_GenericGroup-NegativeBattle[Report] (ALL SHIPS)
    BCE_NegativeBattle_More,                    //91. CHATTER_GenericGroup-NegativeBattle[Report] (ALL SHIPS)
    BCE_NegativeBattle_More2,                   //91. CHATTER_GenericGroup-NegativeBattle[Report] (ALL SHIPS)
    BCE_NeutralBattle,                          //92. CHATTER_GenericGroup-NeutralBattle[Report] (ALL SHIPS)
    BCE_NeutralBattle_More,                     //92. CHATTER_GenericGroup-NeutralBattle[Report] (ALL SHIPS)
    BCE_FriendlyFire,                           //93. CHATTER_GenericGroup-FriendlyFire (ALL SHIPS)
    BCE_GroupFriendlyFire,                      // STAT_AssGrp_FriendlyFire
    BCE_WingmanChased,                          //60. STATUS_Fighter-WingmanChased[Report] (ALL SHIPS)
    BCE_WingmanChasedRsp,                       //61. STATUS_Fighter-WingmanChased[Response] (ALL SHIPS)
    BCE_WingmanHit,                             //62. STATUS_Fighter-WingmanHit[Report] (ALL SHIPS)
    BCE_WingmanHitRsp,                          //63. STATUS_Fighter-WingmanHit[Response] (ALL SHIPS)
    BCE_WingmanLethal,                          //64. STATUS_Fighter-WingmanLethallyDamaged[Report] (ALL SHIPS)
    BCE_WingmanDies,                            //66. STATUS_Fighter-WingmanDies[Report] (ALL SHIPS)
    BCE_LeaderChased,                           //68. STATUS_Fighter-LeaderBeingChased[Report] (ALL SHIPS)
    BCE_Kamikaze,                               //69. COMMAND-LightInterceptor-Kamikaze (ALL SHIPS)
    BCE_StartBattleDisadvant,                   //73. STATUS_GenericGroup-StartBattleDisadvantaged[Report] (ALL SHIPS)
    BCE_StartBattleDisadvantRsp,                //74. STATUS_GenericGroup-StartBattleDisadvantaged[Response] (ALL SHIPS)
    BCE_StartBattleFair,                        //75. STATUS_GenericGroup-StartBattleFair[Report] (ALL SHIPS)
    BCE_StartBattleFairRsp,                     //76. STATUS_GenericGroup-StartBattleFair[Response] (ALL SHIPS)
    BCE_StartBattleAdvantaged,                  //77. STATUS_GenericGroup-StartBattleAdvantaged[Report] (ALL SHIPS)
    BCE_StartBattleAdvantagedRsp,               //78. STATUS_GenericGroup-StartBattleAdvantaged[Response] (ALL SHIPS)
    BCE_Retreat,                                //79. COMMAND_GenericGroup-Retreat (ALL SHIPS)
    BCE_RetreatRsp,                             //80. COMMAND_GenericGroup-Retreat[Response] (ALL SHIPS)
    BCE_WinningBattle,                          //81. STATUS_GenericGroup-WinningBattle[Report] (ALL SHIPS)
    BCE_WinningBattleRsp,                       //82. STATUS_GenericGroup-WinningBattle[Response] (ALL SHIPS)
    BCE_KickingButt,                            //83. STATUS_GenericGroup-KickingButt[Report] (ALL SHIPS)
    BCE_KickingButtRsp,                         //84. STATUS_GenericGroup-KickingButt[Response] (ALL SHIPS)
    BCE_BattleWon,                              //85. STATUS_GenericGroup-BattleWon[Report] (ALL SHIPS)
    BCE_GroupVictory,                           // STAT_F_AssGrp_Victory
    BCE_LosingBattle,                           //86. STATUS_GenericGroup-LosingBattle[Report] (ALL SHIPS)
    BCE_LosingBattle_More,                      //86. STATUS_GenericGroup-LosingBattle[Report] (ALL SHIPS)
    BCE_LosingBattleRsp,                        //87. STATUS_GenericGroup-LosingBattle[Response] (ALL SHIPS)
    BCE_LosingBadly,                            //88. STATUS_GenericGroup-LosingBadly[Report] (ALL SHIPS)
    BCE_LosingBadly_More,                       //88. STATUS_GenericGroup-LosingBadly[Report] (ALL SHIPS)
    BCE_LosingBadlyRsp,                         //89. STATUS_GenericGroup-LosingBadly[Response] (ALL SHIPS)
    BCE_CannotComply,                           //  COMM_CannotComply
    BCE_STAT_Strike_StuckInGrav,                //  STAT_Strike_StuckInGrav
    BCE_COMM_Grav_Off,                          //  COMM_Grav_Off
    BCE_STAT_Strike_AttackComplete,             //  STAT_Strike_AttackComplete
    BCE_COMM_Kamikaze_NoTargets,                //  COMM_Kamikaze_NoTargets
    BCE_COMM_HVette_BurstAttack,                //  COMM_HVette_BurstAttack
    BCE_COMM_MLVette_ForceDrop,                 //  COMM_MLVette_ForceDrop
    BCE_COMM_MissleDest_VolleyAttack,           //  COMM_MissleDest_VolleyAttack
    BCE_STAT_Grp_RetreatSuccessful,             //  STAT_Grp_RetreatSuccessful
    BCE_STAT_AssGrp_RetreatSuccessful,          //  STAT_AssGrp_RetreatSuccessful
    BCE_STAT_Grp_EnemyRetreated,                //  STAT_Grp_EnemyRetreated
    BCE_STAT_AssGrp_EnemyRetreated,             //  STAT_AssGrp_EnemyRetreated
    BCE_STAT_SCVette_DroppingOff,               //STAT_SCVette_DroppingOff
    BCE_STAT_F_ShipStolen,                      //STAT_F_ShipStolen
    BCE_STAT_Grp_InMineField,                   //STAT_Grp_InMineField
    BCE_STAT_AssGrp_InMineField,                //STAT_AssGrp_InMineField
    BCE_STAT_F_CloakedShipsDetected,            //STAT_F_CloakedShipsDetected
    BCE_STAT_Grp_AttackRetaliate,
    BCE_STAT_Grp_AttackRetaliate_Repeat,
    BCE_STAT_AssGrp_AttackRetaliate,
    BCE_STAT_AssGrp_AttackRetaliate_Repeat,
    BCE_AttackFromAlly,                         //Fleet command's notification
    BCE_GroupAttackFromAlly,                    //allship's notification for numbered groups
    BCE_ProbeDetected,                          // STAT_F_EnemyProbe_Detected
    BCE_GravwellDetected,                       // STAT_F_GravWellDetected
    BCE_CapitalShipDamaged,                     // STAT_Cap_Damaged
    BCE_CapitalShipDies,                        // STAT_Cap_Dies
    BCE_CapitalShipDiesReport,                  // STAT_Cap_Dies_Report
    BCE_HyperspaceAbandoned,                    // STAT_Hyper_Abandoned
    BCE_MothershipLargeAttack,                  // STAT_MoShip_LargeAttack
    BCE_MothershipLargeAttackResp,              // STAT_MoShip_LargeAttack_Resp
    BCE_CapitalShipLaunched,                    // STAT_Cap_Launched
    BCE_CapitalShipWelcomed,                    // STAT_Cap_Welcomed
    BCE_CapitalDamagedSpecific,                 // STAT_Cap_Damaged_Specific
    BCE_CapitalShipDocked,						// STAT_Cap_ShipDocked
    BCE_DockingChatter,							// CHAT_Docking
	BCE_DockingChatterMore,						// CHAT_Docking_More
	BCE_CloakingOn,								// COMM_Cloak_CloakingOn
	BCE_CloakingOnResponse,						// STAT_Cloak_CloakingOn_Resp
	BCE_CloakingInsufficientPower,				// COMM_Cloak_InsufficientPower
	BCE_CloakingPowerLow,						// STAT_Cloak_CloakPowerLow
	BCE_Decloaking,								// COMM_Cloak_Decloak
	BCE_SalvageTargetAcquired,					// STAT_SCVette_TargetAcquired
	BCE_RepairStarted,							// STAT_RepVette_StartedRepairs
	BCE_RepairFinished,							// STAT_RepVette_FinishedRepairs
	BCE_FuelLow,								// STAT_Strike_LowOnFuel
	BCE_OutOfFuel,								// STAT_Strike_OutOfFuel
    BCE_COMM_F_ScuttleReaffirm,                 // COMM_F_ScuttleReaffirm
    BCE_COMM_F_HyperspaceDetected,              //COMM_F_HyperspaceDetected
    BCE_STAT_F_ProxSensor_Detection,            //STAT_F_ProxSensor_Detection
    //BCE_COMM_F_Grp_Enemy_Fighters_Decloaking, // STAT_F_CloakedShipsDetected
    //BCE_COMM_F_Cloakgen_Decloaking,           // STAT_F_CloakedShipsDetected
    BCE_LastBCE
}
battlechatterevent;

/*=============================================================================
    Data:
=============================================================================*/
extern udword batGlobalFrequencyModifier;       //exported for demo playback

/*=============================================================================
    Macros:
=============================================================================*/
#define battlePingSize(nShips)  (sizeof(battleping) + sizeof(Ship *) * ((nShips) - 1))

//for initializing arrays if battle chatter events
#if BATTLE_TEXT
#define newEvent(name, num, text)             { name, num, BAD_RandomWeight, BAD_MaxDistance, BAD_ExpDistance, BAD_MinWavelength, BAD_ExpWavelength, BAD_MinProximity, BAD_ExpProximity, text, 0.0f, {0.0f, 0.0f, 0.0f}}
#else //BATTLE_TEXT
#define newEvent(name, num, text)             { name, num, BAD_RandomWeight, BAD_MaxDistance, BAD_ExpDistance, BAD_MinWavelength, BAD_ExpWavelength, BAD_MinProximity, BAD_ExpProximity, 0.0f, {0.0f, 0.0f, 0.0f}}
#endif //BATTLE_TEXT

#define batRandom()                     (ranRandom(RAN_Battle) & BAT_RandomMask)

/*=============================================================================
    Functions:
=============================================================================*/
void battleChatterStartup(void);
void battleChatterShutdown(void);
void battlePingEvaluate(void *thisPing, battleping *battlePing);
void battlePingEvaluateNoFriendlies(void *voidPing, battleping *battlePing);
void battlePingEvaluateNoEnemies(void *voidPing, battleping *battlePing);

bool battleCanChatterAtThisTime(sdword event, Ship *ship);
sdword battleChatterAttempt(sdword linkTo, sdword event, Ship *ship, sdword variable);
sdword battleChatterFleetAttempt(sdword linkTo, sdword event, sdword variable, vector *where);
Ship *battleMouthPieceFindAnother(Ship *firstMouthPiece, Ship **ships, sdword numShips);
Ship *battleMouthPieceFind(Ship **ships, sdword numShips);
bool battleShipOnMyTail(Ship *meShip, Ship *badGuyShip);
void battleAccountForLosses(Ship *ship, battleping *battlePing);
void battleChatterFrequencySet(sdword freq100);
void battleShipDyingWithTimeToScream(Ship *ship);
void battleLaunchWelcomeExchange(Ship *ship);

#endif //___BATTLE_H

