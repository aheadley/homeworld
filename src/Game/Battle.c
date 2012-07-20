/*=============================================================================
    Name    : Battle.c
    Purpose : Code to evaluate and update battle chatter.

    Created 12/14/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <math.h>
#include "Types.h"
#include "Debug.h"
#include "Memory.h"
#include "SpaceObj.h"
#include "Vector.h"
#include "Matrix.h"
#include "prim2d.h"
#include "Camera.h"
#include "SoundEvent.h"
#include "Randy.h"
#include "mainrgn.h"
#include "FastMath.h"
#include "Alliance.h"
#include "Universe.h"
#include "FlightMan.h"
#include "Ping.h"
#include "Gun.h"
#include "Select.h"
#include "Blobs.h"
#include "Battle.h"

/*=============================================================================
    Data:
=============================================================================*/
#if BATTLE_TEXT
#if 0
char *battleString_Fighter_ManeuverGrunts[] =
{
    "...oh boy...",
    "...go...go...go...",
    "...come on...come on...",
    "...that's it, that's it...",
    "...yeah!...",
    "...[stomach punch grunt]...",
    "...[stomach twisting groan]...",
    "...[steady heavy breathing]...",
    "...[shaky fast breathing]...",
    "...[hyperventilating rapid breathing]...",
    "...[G-force induced moan - short]...",
    "...[G-force induced moan - long]...",
    NULL
};
#endif
char *battleString_WingmanChased[] =
{
    "There's one on your tail. Check six. Check six.",
    "You're being followed.",
    "You've got one closing behind.",
    "You've gotta tail.",
    "You've picked one up.",
    "There's one on you.",
    "Break! Break!",
    NULL
};
char *battleString_WingmanChased_Rsp[] =
{
    "He's on tight.",
    "...Can't break it...",
    "...I see 'em. Cover me.",
    "Roger that trailer, I see him.",
    "Get him off me!",
    "Gotcha.",
    NULL
};
char *battleString_WingmanHit[] =
{
    "Wing hit!",
    "...Cover me dammit.",
    "I've been hit.",
    "I'm hit, but not too bad.",
    "Okay, I'm still with you.",
    "(to self) Damn! Okay, alright. I'm alright...",
    NULL
};
char *battleString_WingmanHit_Rsp[] =
{
    "Alright, hang in there.",
    "Looks OK.",
    "Hold formation. Let's keep it together now...",
    "You're looking fine.",
    "Hang on ... I'm coming in.",
    "On my way.",
    NULL
};
char *battleString_WingmanLethal[] =
{
    "Can't stabilize... I'm... <static>",
    "I'm losing the... <static>",
    "(to self) (a big scream cut short)... <static>",
    "(to self) (breathes in hard and fast)... <static>",
    "aaargh (effort) ... <static>",
    "ahhhhhhhh (burning) ... <static>",
    "hunngh! (depressurization) .. <static>",
    "I'm folding! ...<static>",
    "Mayday, I got no...<static>",
    NULL
};
char *battleString_WingmanDies[] =
{
    "Wingman down",
    "I've lost my wingman",
    "Wing down.",
    "Lost the wing.",
    NULL
};
char *battleString_LeaderChased[] =
{
    "Group leader, you've got a tail.",
    "Group lead, you've picked up a trailer.",
    "Hold on group leader, I'm coming.",
    NULL
};
char *battleString_Wingman[] =
{
    "Wing, watch your course.",
    "You're sliding off-line. Correct.",
    "You're off target vector... compensate now.",
    "Stay within targeting threshold.",
    "Counter that angle, wing.",
    "We're losing wing lock... reset.",
    "You're too close, watch the gap.",
    "Close the gap your too far!",
    "Wingman, tighten it up.",
    "Stay on target.",
    NULL
};
char *battleString_PositiveBattle[] =
{
    "Nice move.",
    "I saw it, yeah, very smooth.",
    "OK, looking fine over here.",
    "Cover me.",
    "I got you.",
    "I'm with you.",
    "That's affirmative, I have it here.",
    "We're on it.",
    "Confirmed visual. Locked.",
    "Target assignments confirmed.",
    "Formation secured.",
    "Coordinates entered.",
    "Got it.",
    "Once again, the way it's done.",
    "Boom. Right there.",
    "Oh yeah.",
    "We're in. Yes.",
    NULL
};
char *battleString_NegativeBattle[] =
{
    "Redirect fire! Redirect fire!",
    "What's happening?",
    "I can't see! Scopes are dead!",
    "Negative! Left! Go left!",
    "Say again?",
    "Behind you!",
    "What's the position?",
    "Can't lock in!",
    "Alarms! I got alarms!",
    "Sound off!",
    "Dammit!",
    "Fall back... ",
    "Respond! ",
    "Talk to me...",
    "Hold on...",
    "What are you doing?",
    "Come on!",
    "Let's move!",
    "Go around!",
    "No! Negative!",
    "Regroup! Regroup!",
    "This ain't happening, man.",
    NULL
};
char *battleString_NeutralBattle[] =
{
    "Tactical even. It's flat.",
    "We're this far, push it home!",
    "Lay it on! Over the top!",
    "Lead your targets...",
    "Collision avoidance... lock it up.",
    "It's pitched. Nudge it! Nudge it our way!",
    "Battle stats level.",
    "Who has it?",
    "Watch the targets, not tactical.",
    "Pick them up fast.",
    "Plug in! Plug in!",
    "We're holding positions...",
    "Flanking ships stay close.",
    NULL
};
char *battleString_FriendlyFire[] =
{
    "Check your fire!  Check your fire!",
    "Check your targets!  Friendly fire!",
    "Watch it!",
    "Watch that friendly fire!",
    NULL
};
char *battleString_Kamikaze[] =
{

    "[increasing pitch] ...mmmmmmmnnahh!!!",
    "....nnnnn....aaahh!",
    "..aaaarrrgh!",
    "FOR THE HOMEWORLD!!!!",
    "...uh....understood...  (hesitant)",
    "EAT THIS!!!",

    NULL
};
char *battleString_StartBattleDisadvantaged[] =
{

    "We're outnumbered.",
    "I have a bad feeling about this...",
    "This isn't lookin' good Fleet...",
    "(to self quietly) ..ohh man...",

    NULL
};
char *battleString_StartBattleDisadvantaged_Rsp[] =
{
    "Yeah, but we can do it.",
    "Just keep it tight.",
    "We've got our orders.",
    "Shut it, we're goin' in.",
    NULL
};
char *battleString_StartBattleFair[] =
{

    "Watch your six.",
    "Pick your targets.",
    "It looks even, let's do it.",

    NULL
};
char *battleString_StartBattleFair_Rsp[] =
{

    "Maintain positions.",
    "Right behind you.",
    "Acknowledged. On your wing.",
    "Coming in at <NumberList>.",
    "Look for the trap.",

    NULL
};
char *battleString_StartBattleAdvantaged[] =
{

    "We caught 'em napping.",
    "I almost feel bad about this.",
    "It's in our favor.",
    "Keep one eye on the clock.",
    "Looks like we got the upper hand.",
    "Let's make this quick and painless.",

    NULL
};
char *battleString_StartBattleAdvantaged_Rsp[] =
{

    "Stay on top of them.",
    "Don't let them regroup.",
    "It's not over yet.",
    "Get on their tails, and stay there.",
    "Copy that ... let's take 'em down one by one.",
    "They're up against the wall.",

    NULL
};
char *battleString_Retreat[] =
{

    "All units retreat, repeat, all units retreat.",
    "Get out of there now! Abort. Abort.",
    "Abort mission, repeat, abort mission.",
    "Retreat. Retreat. Abort all offensive action.",
    "Begin abort sequence.",
    "Abort code <NumberList>.",
    "Break! Break!",
    "Disengage immediately.",

    NULL
};
char *battleString_Retreat_Rsp[] =
{

    "<Roger>. On our way.",
    "<Roger>. We're outta here!",
    "<Roger>. It's about time!",
    "Abort code received and confirmed.",
    "<Roger>. Disengaging.",
    "Let's get the hell outta Dodge!",
    "Wing leaders... head count, report all losses.",

    NULL
};
char *battleString_WinningBattle[] =
{

    "We're on top of it.",
    "This is looking good. Stay tight.",
    "It's ours so far. Watch the counter.",
    "Spreading at <Degrees>. Good work.",
    "Just a little more and it's over.",
    "Tactical full, looks good.",
    "Let's wrap this up.",

    NULL
};
char *battleString_WinningBattle_Rsp[] =
{

    "Yup.",
    "Roger that sweep.",
    "Holding attack pattern <Letter>.",
    "That's it. There we go.",
    "They're going down.",

    NULL
};
char *battleString_KickingButt[] =
{

    "Let's end this.",
    "It's over. Let's clean it up.",
    "We have them.",
    "This is finished.",
    "Pick off the stragglers.",
    "OK that'll do it.",
    "The enemy is failing.",

    NULL
};
char *battleString_KickingButt_Rsp[] =
{
    "Roger that. Cleaning it up.",
    "Affirmative. It'll be no problem.",
    "No contest.",
    "They're almost down.",
    "Watch for debris.",
    "Reset targeting and inertials.",
    "Let's do them a favour.",
    "They're routed.",
    NULL
};
char *battleString_BattleWon[] =
{

    "The enemy is defeated.",
    "We've won. We've done it.",
    "Stand down from combat stations.",
    "The area is secure.",
    "Mission accomplished. Standing by.",
    "Enemy neutralized, awaiting further instructions.",
    "OK. That's it.  Nice work.",

    NULL
};
char *battleString_LosingBattle[] =
{
    "We have a situation developing here.",
    "Requesting back up... repeat... requesting back up.",
    "Switch to battle code <NumberList>.",
    "The enemy has it.",
    "We're taking heavy losses.",
    "We are being overwhelmed.",
    "We're losing grip here.",
    "Head count, head count! Report in!",
    "Links failing, we have losses.",
    "Tactical is thinning out here, we've got a problem.",
    "... ah ... this is not looking good Fleet.",
    NULL
};
char *battleString_LosingBattle_Rsp[] =
{

    "OK, keep it together now.",
    "Return the fight to them.",
    "Roger that contact, we see it.",
    "Hang in there. Hold your positions.",
    "Losses confirmed.",
    "Get it together and fight back.",
    "Assume defense lock <Letter>.",
    "Open up gaps. Look for the gaps.",
    "<Roger>. Stay close. Keep it close.",

    NULL
};
char *battleString_LosingBadly[] =
{

    "Situation critical Fleet.",
    "We're getting our asses kicked here Fleet!",
    "We're down to <Number> units.",
    "We're pinned. We need relief.",
    "Hits! We got hits all over here!",
    "Contact everywhere! Heavy contact!",
    "Is anyone responding here?",
    "Revise head counts! Updates!",
    "Where are the links?",
    "Tactical is almost clear. It's getting a bit lonely out here...",
    "Now it's really not looking good Fleet...",
    "Check your fire! Check your fire!",

    NULL
};
char *battleString_LosingBadly_Rsp[] =
{
    "Emergency procedures.",
    "Rally! Rally!",
    "Get us out of here!",
    "Maintain positions!",
    "We're running out of time.",
    "Can't ... shake 'em... Can't...",
    "It's too much! I can't.... damn!",
    "Come ON!  Let's GO!",
    "If you're gonna act, act now.",
    "This is murder out here.",
    NULL
};
#endif //BATTLE_TEXT

//master list of chatter events
battleevent battleChatterEvent[] =
{
/*BCE_Wingman,                  */ newEvent("CHAT_Fighter_Wingman",          CHAT_Fighter_Wingman,           battleString_Wingman),
/*BCE_PositiveBattle            */ newEvent("CHAT_Grp_PositiveBattle",       CHAT_Grp_PositiveBattle,        battleString_PositiveBattle),
/*BCE_PositiveBattle_More       */ newEvent("CHAT_Grp_PositiveBattle_More",  CHAT_Grp_PositiveBattle_More,   battleString_PositiveBattle),
/*BCE_NegativeBattle            */ newEvent("CHAT_Grp_NegativeBattle",       CHAT_Grp_NegativeBattle,        battleString_NegativeBattle),
/*BCE_NegativeBattle_More       */ newEvent("CHAT_Grp_NegativeBattle_More",  CHAT_Grp_NegativeBattle_More,   battleString_NegativeBattle),
/*BCE_NegativeBattle_More2      */ newEvent("CHAT_Grp_NegativeBattle_More2", CHAT_Grp_NegativeBattle_More2,  battleString_NegativeBattle),
/*BCE_NeutralBattle,            */ newEvent("CHAT_Grp_NeutralBattle",        CHAT_Grp_NeutralBattle,         battleString_NeutralBattle),
/*BCE_NeutralBattle_More,       */ newEvent("CHAT_Grp_NeutralBattle_More",   CHAT_Grp_NeutralBattle_More,    battleString_NeutralBattle),
/*BCE_FriendlyFire,             */ newEvent("CHAT_Grp_FriendlyFire",         CHAT_Grp_FriendlyFire,          battleString_FriendlyFire),
/*BCE_GroupFriendlyFire,        */ newEvent("STAT_AssGrp_FriendlyFire",      STAT_AssGrp_FriendlyFire,       battleString_FriendlyFire),
/*BCE_WingmanChased,            */ newEvent("STAT_Fighter_WingmanChased",    STAT_Fighter_WingmanChased,     battleString_WingmanChased),
/*BCE_WingmanChasedR            */ newEvent("STAT_Fighter_WingmanChased_Rsp",STAT_Fighter_WingmanChased_Rsp, battleString_WingmanChased_Rsp),
/*BCE_WingmanHit,               */ newEvent("STAT_Fighter_WingmanHit",       STAT_Fighter_WingmanHit,        battleString_WingmanHit),
/*BCE_WingmanHitRsp             */ newEvent("STAT_Fighter_WingmanHit_Rsp",   STAT_Fighter_WingmanHit_Rsp,    battleString_WingmanHit_Rsp),
/*BCE_WingmanLethal,            */ newEvent("STAT_Fighter_WingmanLethal",    STAT_Fighter_WingmanLethal,     battleString_WingmanLethal),
/*BCE_WingmanDies,              */ newEvent("STAT_Fighter_WingmanDies",      STAT_Fighter_WingmanDies,       battleString_WingmanDies),
/*BCE_LeaderChased,             */ newEvent("STAT_Fighter_LeaderChased",     STAT_Fighter_LeaderChased,      battleString_LeaderChased),
/*BCE_Kamikaze,                 */ newEvent("COMM_LInt_Kamikaze",                COMM_LInt_Kamikaze                , battleString_Kamikaze),
/*BCE_StartBattleDisadvant,     */ newEvent("STAT_Grp_StartBattleDisadvant",     STAT_Grp_StartBattleDisadvant     , battleString_StartBattleDisadvantaged),
/*BCE_StartBattleDisadvantRsp,  */ newEvent("STAT_Grp_StartBattleDisadvant_Rsp", STAT_Grp_StartBattleDisadvant_Rsp , battleString_StartBattleDisadvantaged_Rsp),
/*BCE_StartBattleFair,          */ newEvent("STAT_Grp_StartBattleFair",          STAT_Grp_StartBattleFair          , battleString_StartBattleFair),
/*BCE_StartBattleFairRsp,       */ newEvent("STAT_Grp_StartBattleFair_Rsp",      STAT_Grp_StartBattleFair_Rsp      , battleString_StartBattleFair_Rsp),
/*BCE_StartBattleAdvantaged,    */ newEvent("STAT_Grp_StartBattleAdvantaged",    STAT_Grp_StartBattleAdvantaged    , battleString_StartBattleAdvantaged),
/*BCE_StartBattleAdvantagedRsp, */ newEvent("STAT_Grp_StartBattleAdvantaged_Rsp",STAT_Grp_StartBattleAdvantaged_Rsp, battleString_StartBattleAdvantaged_Rsp),
/*BCE_Retreat,                  */ newEvent("COMM_Group_Retreat",                COMM_Group_Retreat                , battleString_Retreat),
/*BCE_RetreatRsp,               */ newEvent("COMM_Group_Retreat_Rsp",            COMM_Group_Retreat_Rsp            , battleString_Retreat_Rsp),
/*BCE_WinningBattle,            */ newEvent("STAT_Grp_WinningBattle",            STAT_Grp_WinningBattle            , battleString_WinningBattle),
/*BCE_WinningBattleRsp,         */ newEvent("STAT_Grp_WinningBattle_Rsp",        STAT_Grp_WinningBattle_Rsp        , battleString_WinningBattle_Rsp),
/*BCE_KickingButt,              */ newEvent("STAT_Grp_KickingButt",              STAT_Grp_KickingButt              , battleString_KickingButt),
/*BCE_KickingButtRsp,           */ newEvent("STAT_Grp_KickingButt_Rsp",          STAT_Grp_KickingButt_Rsp          , battleString_KickingButt_Rsp),
/*BCE_BattleWon,                */ newEvent("STAT_Grp_BattleWon",                STAT_Grp_BattleWon                , battleString_BattleWon),
/*BCE_GroupVictory,             */ newEvent("STAT_F_AssGrp_Victory",             STAT_F_AssGrp_Victory             , battleString_BattleWon),
/*BCE_LosingBattle,             */ newEvent("STAT_Grp_LosingBattle",             STAT_Grp_LosingBattle             , battleString_LosingBattle),
/*BCE_LosingBattle_More,        */ newEvent("STAT_Grp_LosingBattle_More",        STAT_Grp_LosingBattle_More        , battleString_LosingBattle),
/*BCE_LosingBattleRsp,          */ newEvent("STAT_Grp_LosingBattle_Rsp",         STAT_Grp_LosingBattle_Rsp         , battleString_LosingBattle_Rsp),
/*BCE_LosingBadly,              */ newEvent("STAT_Grp_LosingBadly",              STAT_Grp_LosingBadly              , battleString_LosingBadly),
/*BCE_LosingBadly_More,         */ newEvent("STAT_Grp_LosingBadly_More",         STAT_Grp_LosingBadly_More         , battleString_LosingBadly),
/*BCE_LosingBadlyRsp,           */ newEvent("STAT_Grp_LosingBadly_Rsp",          STAT_Grp_LosingBadly_Rsp          , battleString_LosingBadly_Rsp),
/*COMM_CannotComply,            */ newEvent("COMM_CannotComply", COMM_CannotComply , NULL),
/*STAT_Strike_StuckInGrav,      */ newEvent("STAT_Strike_StuckInGrav", STAT_Strike_StuckInGrav , NULL),
/*COMM_Grav_Off,                */ newEvent("COMM_Grav_Off", COMM_Grav_Off , NULL),
/*STAT_Strike_AttackComplete,   */ newEvent("STAT_Strike_AttackComplete", STAT_Strike_AttackComplete , NULL),
/*COMM_Kamikaze_NoTargets,      */ newEvent("COMM_Kamikaze_NoTargets", COMM_Kamikaze_NoTargets , NULL),
/*COMM_HVette_BurstAttack,      */ newEvent("COMM_HVette_BurstAttack", COMM_HVette_BurstAttack , NULL),
/*COMM_MLVette_ForceDrop,       */ newEvent("COMM_MLVette_ForceDrop", COMM_MLVette_ForceDrop , NULL),
/*COMM_MissleDest_VolleyAttack, */ newEvent("COMM_MissleDest_VolleyAttack", COMM_MissleDest_VolleyAttack , NULL),
/*STAT_Grp_RetreatSuccessful,   */ newEvent("STAT_Grp_RetreatSuccessful", STAT_Grp_RetreatSuccessful , NULL),
/*STAT_AssGrp_RetreatSuccessful,*/ newEvent("STAT_AssGrp_RetreatSuccessful", STAT_AssGrp_RetreatSuccessful , NULL),
/*STAT_Grp_EnemyRetreated,      */ newEvent("STAT_Grp_EnemyRetreated", STAT_Grp_EnemyRetreated , NULL),
/*STAT_AssGrp_EnemyRetreated,   */ newEvent("STAT_AssGrp_EnemyRetreated", STAT_AssGrp_EnemyRetreated , NULL),
/*STAT_SCVette_DroppingOff,     */ newEvent("STAT_SCVette_DroppingOff", STAT_SCVette_DroppingOff , NULL),
/*STAT_F_ShipStolen,            */ newEvent("STAT_F_ShipStolen", STAT_F_ShipStolen , NULL),
/*STAT_Grp_InMineField,         */ newEvent("STAT_Grp_InMineField", STAT_Grp_InMineField , NULL),
/*STAT_AssGrp_InMineField,      */ newEvent("STAT_AssGrp_InMineField", STAT_AssGrp_InMineField , NULL),
/*BCE_STAT_F_CloakedShipsDete   */ newEvent("STAT_F_CloakedShipsDetected", STAT_F_CloakedShipsDetected , NULL),
/*BCE_STAT_Grp_AttackRetaliat   */ newEvent("STAT_Grp_AttackRetaliate", STAT_Grp_AttackRetaliate , NULL),
/*BCE_STAT_Grp_AttackRetaliat,  */ newEvent("STAT_Grp_AttackRetaliate_Repeat", STAT_Grp_AttackRetaliate_Repeat , NULL),
/*BCE_STAT_AssGrp_AttackRetal   */ newEvent("STAT_AssGrp_AttackRetaliate", STAT_AssGrp_AttackRetaliate , NULL),
/*BCE_STAT_AssGrp_AttackRetaleat*/ newEvent("STAT_AssGrp_AttackRetaliate_Repeat", STAT_AssGrp_AttackRetaliate_Repeat , NULL),
/* BCE_AttackFromAlly,          */ newEvent("STAT_F_UnderAttackFromAlly", STAT_F_UnderAttackFromAlly, NULL),
/* BCE_GroupAttackFromAlly,     */ newEvent("STAT_Grp_AttackFromAlly", STAT_Grp_AttackFromAlly, NULL),
/* BCE_ProbeDetected,           */ newEvent("STAT_F_EnemyProbe_Detected", STAT_F_EnemyProbe_Detected, NULL),
/* BCE_GravwellDetected,        */ newEvent("STAT_F_GravWellDetected", STAT_F_GravWellDetected, NULL),
/* BCE_CapitalShipDamaged,      */ newEvent("STAT_Cap_Damaged", STAT_Cap_Damaged     , NULL),
/* BCE_CapitalShipDies,         */ newEvent("STAT_Cap_Dies", STAT_Cap_Dies        , NULL),
/* BCE_CapitalShipDiesReport,   */ newEvent("STAT_Cap_Dies_Report", STAT_Cap_Dies_Report , NULL),
/* BCE_HyperspaceAbandoned      */ newEvent("STAT_Hyper_Abandoned", STAT_Hyper_Abandoned, NULL),
/* BCE_MothershipLargeAttack,   */ newEvent("STAT_MoShip_LargeAttack", STAT_MoShip_LargeAttack, NULL),
/* BCE_MothershipLargeAttackResp*/ newEvent("STAT_MoShip_LargeAttack_Resp", STAT_MoShip_LargeAttack_Resp, NULL),
/* BCE_CapitalShipLaunched,     */ newEvent("STAT_Cap_Launched", STAT_Cap_Launched, NULL),
/* BCE_CapitalShipWelcomed,     */ newEvent("STAT_Cap_Welcomed", STAT_Cap_Welcomed, NULL),
/* BCE_CapitalDamagedSpecific,  */ newEvent("STAT_Cap_Damaged_Specific", STAT_Cap_Damaged_Specific, NULL),
/* BCE_CapitalShipDocked,       */ newEvent("STAT_Cap_ShipDocked", STAT_Cap_ShipDocked, NULL),
/* BCE_DockingChatter,          */ newEvent("CHAT_Docking", CHAT_Docking, NULL),
/* BCE_DockingChatterMore,      */ newEvent("CHAT_Docking_More", CHAT_Docking_More, NULL),
/* BCE_CloakingOn,              */ newEvent("COMM_Cloak_CloakingOn", COMM_Cloak_CloakingOn, NULL),
/* BCE_CloakingOnResponse,      */ newEvent("STAT_Cloak_CloakingOn_Resp", STAT_Cloak_CloakingOn_Resp, NULL),
/* BCE_CloakingInsufficientPower*/ newEvent("COMM_Cloak_InsufficientPower", COMM_Cloak_InsufficientPower, NULL),
/* BCE_CloakingPowerLow,        */ newEvent("STAT_Cloak_CloakPowerLow", STAT_Cloak_CloakPowerLow, NULL),
/* BCE_Decloaking,              */ newEvent("COMM_Cloak_Decloak", COMM_Cloak_Decloak, NULL),
/* BCE_SalvageTargetAcquired,   */ newEvent("STAT_SCVette_TargetAcquired", STAT_SCVette_TargetAcquired, NULL),
/* BCE_RepairStarted,           */ newEvent("STAT_RepVette_StartedRepairs", STAT_RepVette_StartedRepairs, NULL),
/* BCE_RepairFinished,          */ newEvent("STAT_RepVette_FinishedRepairs", STAT_RepVette_FinishedRepairs, NULL),
/* BCE_FuelLow,                 */ newEvent("STAT_Strike_LowOnFuel", STAT_Strike_LowOnFuel, NULL),
/* BCE_OutOfFuel,               */ newEvent("STAT_Strike_OutOfFuel", STAT_Strike_OutOfFuel, NULL),
/* BCE_COMM_F_ScuttleReaffirm   */ newEvent("COMM_F_ScuttleReaffirm", COMM_F_ScuttleReaffirm, NULL),
/* BCE_COMM_F_HyperspaceDetected*/ newEvent("COMM_F_HyperspaceDetected", COMM_F_HyperspaceDetected, NULL),
/* BCE_STAT_F_ProxSensor_Detection*/ newEvent("STAT_F_ProxSensor_Detection", STAT_F_ProxSensor_Detection, NULL),

/*COMM_F_Grp_Enemy_Fighters_Decl*/ //newEvent("COMM_F_Grp_Enemy_Fighters_Decloaking", COMM_F_Grp_Enemy_Fighters_Decloaking , NULL),
/*COMM_F_Cloakgen_Decloaking,   */ //newEvent("COMM_F_Cloakgen_Decloaking", COMM_F_Cloakgen_Decloaking , NULL),
#if 0
/**/newEvent(),
#endif
};

//some chatter is class-specific
#define BCD_LaunchWelcome       0x01
#define BCD_Damaged             0x02
ubyte battleClassDependencies[NUM_CLASSES] =
{
    /* CLASS_Mothership   */ 0,
    /* CLASS_HeavyCruiser */ BCD_Damaged | BCD_LaunchWelcome,
    /* CLASS_Carrier      */ BCD_Damaged | BCD_LaunchWelcome,
    /* CLASS_Destroyer    */ BCD_Damaged | BCD_LaunchWelcome,
    /* CLASS_Frigate      */ BCD_Damaged | BCD_LaunchWelcome,
    /* CLASS_Corvette     */ 0,
    /* CLASS_Fighter      */ 0,
    /* CLASS_Resource     */ BCD_Damaged,
    /* CLASS_NonCombat    */ 0
};

/*-----------------------------------------------------------------------------
    Battle chatter tweaks and script parsing table
-----------------------------------------------------------------------------*/
//number of times to try to find a ship to do the talking
sdword batMouthpieceTries = BAT_MouthpieceTries;

//coefficients for computing the current battle standing number (BSN)
real32 batStandingFriendlyPower =   BAT_StandingFriendlyPower;
real32 batStandingFriendlyValue =   BAT_StandingFriendlyValue;
real32 batStandingEnemyPower    =   BAT_StandingEnemyPower;
real32 batStandingEnemyValue    =   BAT_StandingEnemyValue;
/*
real32 batFriendlyLossPower     =   BAT_FriendlyLossPower;
real32 batFriendlyLossValue     =   BAT_FriendlyLossValue;
real32 batEnemyLossPower        =   BAT_EnemyLossPower;
real32 batEnemyLossValue        =   BAT_EnemyLossValue;
*/
real32 batExpLossDevalue        =   BAT_ExpLossDevalue;

//values of BSN for various statuses (BSN > x ?)
real32 batBSNKickingButt        =   BAT_BSNKickingButt;
real32 batBSNWinning            =   BAT_BSNWinning;
real32 batBSNNeutral            =   BAT_BSNNeutral;
real32 batBSNLosing             =   BAT_BSNLosing;

//coefficients for determining if a ship is really on your tail
real32 batBogeyAspect           =   BAT_BogeyAspect;

//retreat parameters
real32 batRetreatModifier       =   BAT_RetreatModifier;

//global parameter for controling how much chatter there is
udword batGlobalFrequencyModifier = BAT_GlobalFrequencyModifier;
real32 batGlobalFrequencyMin     = BAT_GlobalFrequencyMin;  //min/max in log base 2 space
real32 batGlobalFrequencyMax     = BAT_GlobalFrequencyMax;

//battle chatter tuning pointer
#if BATTLE_TUNING
battleevent *batChatterToTune   = NULL;
#endif

//script-parsing table and function prototypes
void batMaxDistanceSetCB(char *directory,char *field,void *dataToFillIn);
void batDistanceExponentSetCB(char *directory,char *field,void *dataToFillIn);
void batRandomWeightSetCB(char *directory,char *field,void *dataToFillIn);
void batMinWavelengthSetCB(char *directory,char *field,void *dataToFillIn);
void batWavelengthExponentSetCB(char *directory,char *field,void *dataToFillIn);
void batMinRepeatProximitySetCB(char *directory,char *field,void *dataToFillIn);
void batRepeatProximityExponentSetCB(char *directory,char *field,void *dataToFillIn);
void batTuneEventSet(char *directory,char *field,void *dataToFillIn);
void batGlobalFrequencyModifierSet(char *directory,char *field,void *dataToFillIn);
scriptEntry battleTweaks[] =
{
    {"randomWeight",    batRandomWeightSetCB, NULL},
    {"maxDistance",     batMaxDistanceSetCB, NULL},
    {"expDistance",     batDistanceExponentSetCB, NULL},
    {"minWavelength",   batMinWavelengthSetCB, NULL},
    {"expWavelength",   batWavelengthExponentSetCB, NULL},
    {"minRepeatProximity",batMinRepeatProximitySetCB, NULL},
    {"expRepeatProximity",batRepeatProximityExponentSetCB, NULL},
#if BATTLE_TUNING
    {"TuneOnlyThisEvent",batTuneEventSet, NULL},
#endif
    {"batGlobalFrequencyModifier", batGlobalFrequencyModifierSet,NULL},
    makeEntry(batMouthpieceTries , scriptSetSdwordCB),
    makeEntry(batStandingFriendlyPower , scriptSetReal32CB),
    makeEntry(batStandingFriendlyValue , scriptSetReal32CB),
    makeEntry(batStandingEnemyPower    , scriptSetReal32CB),
    makeEntry(batStandingEnemyValue    , scriptSetReal32CB),
    makeEntry(batBSNKickingButt , scriptSetReal32CB),
    makeEntry(batBSNWinning     , scriptSetReal32CB),
    makeEntry(batBSNNeutral     , scriptSetReal32CB),
    makeEntry(batBSNLosing      , scriptSetReal32CB),
    makeEntry(batBogeyAspect    , scriptSetReal32CB),
    makeEntry(batRetreatModifier, scriptSetReal32CB),
    makeEntry(batGlobalFrequencyMin, scriptSetReal32CB),
    makeEntry(batGlobalFrequencyMax, scriptSetReal32CB),
    endEntry
};

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : battleChatterEventFromName
    Description : Find a battle chatter event by name reference
    Inputs      :
    Outputs     : value - value to read from the string after the name
    Return      :
----------------------------------------------------------------------------*/
battleevent *battleChatterEventFromName(char *name, double *value)
{
    sdword index, nScanned;
    char *string;
    real32 floatValue;

    string = strtok(name, BAT_Delimiters);
    for (index = 0; index < BCE_LastBCE; index++)
    {
        if (!strcmp(name, battleChatterEvent[index].eventName))
        {
            //return(&battleChatterEvent[index]);
            goto foundName;
        }
    }
#if BATTLE_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "Unable to find battle chatter event '%s'.", name);
#endif
foundName:
    string = strtok(NULL, BAT_Delimiters);
    nScanned = sscanf(string, "%f", &floatValue);
    dbgAssert(nScanned == 1);
    *value = (double)floatValue;
    return(&battleChatterEvent[index]);
}

/*-----------------------------------------------------------------------------
    Script-parsing callback functions
-----------------------------------------------------------------------------*/
void batRandomWeightSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value >= 0.0f);
    chat->randomWeight = (udword)(value * (real32)BAT_RandomTotal / (real32)PNG_TaskPeriod);
}
void batMaxDistanceSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->maxDistance = (double)value;
}
void batDistanceExponentSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value >= 0.0f);
    chat->expDistance = (double)value;
}
void batMinWavelengthSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->minWavelength = (double)value;
}
void batWavelengthExponentSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->expWavelength = (double)value;
}
void batMinRepeatProximitySetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->minRepeatProximity = (double)value;
}
void batRepeatProximityExponentSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->expRepeatProximity = (double)value;
}
#if BATTLE_TUNING
void batTuneEventSet(char *directory,char *field,void *dataToFillIn)
{
    double value;
    batChatterToTune = battleChatterEventFromName(field, &value);
}
#endif
void batGlobalFrequencyModifierSet(char *directory,char *field,void *dataToFillIn)
{
    real32 value;
    sdword nScanned;

    nScanned = sscanf(field, "%f", &value);
    dbgAssert(nScanned == 1);
    batGlobalFrequencyModifier = (udword)(value * BIT8);
}

/*-----------------------------------------------------------------------------
    Name        : battleChatterStartup
    Description : Startup the battle chatter module by reading in BattleChatter.script
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleChatterStartup(void)
{
    scriptSet(NULL, "BattleChatter.script", battleTweaks);  //scan in all the chatter parameters
}

/*-----------------------------------------------------------------------------
    Name        : battleChatterShutdown
    Description : Shut down the battle chatter module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleChatterShutdown(void)
{
}

/*-----------------------------------------------------------------------------
    Name        : battleSpeechText
    Description : Print out some text of what the battle chatter _might_ say,
                    not what it actually says.
    Inputs      : eventNum - what event is being called
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if BATTLE_TEXT
void battleSpeechText(sdword eventNum, char **texts)
{
    sdword nTexts, index;

    for (nTexts = 0; texts[nTexts] != NULL; nTexts++)
    {                                                       //count number of text strings
        ;
    }
    index = batRandom() % nTexts;
    dbgMessagef("\nBattle chatter event %d - '%s'.", eventNum, texts[index]);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : battleChatterAttempt
    Description : Attempt to make a given chatter event chat
    Inputs      : linkTo - handle of the previously queued event that this one is to follow
                  event - event to attempt to speak
                  ship - ship that'll do the speaking
    Outputs     :
    Return      : handle of queued event, or ERROR (-1) if none queued
----------------------------------------------------------------------------*/
sdword battleChatterAttempt(sdword linkTo, sdword event, Ship *ship, sdword variable)
{
    vector dist;
    double distance, result;
    battleevent *chat;
    udword randomWeight;

    dbgAssert(event >= 0 && event < BCE_LastBCE);
    chat = &battleChatterEvent[event];
//	if (variable == SOUND_EVENT_DEFAULT)
//		variable = 0;
#if BATTLE_TUNING
    if (batChatterToTune != NULL && batChatterToTune != chat)
    {                                                       //if we're tuning only chatter event
        return(ERROR);                                      //no jouez-vous
    }
#endif
    randomWeight = chat->randomWeight * batGlobalFrequencyModifier / BIT8;
    if (batRandom() < randomWeight)
    {                                                       //if it passes random weight test
        vecSub(dist, mrCamera->eyeposition, ship->posinfo.position);
        distance = (double)vecMagnitudeSquared(dist);       //get length of vector
        distance = fmathSqrtDouble(distance);
        if (distance < chat->maxDistance)
        {                                                   //if it's not too far away from camera to hear
            result = 1.0 - distance / chat->maxDistance;
            dbgAssert(result >= 0.0f && result <= 1.0f);
            if (chat->expDistance != 1.0)
            {
                result = pow(result, chat->expDistance);    //compute point on probability curve
            }
            if ((double)batRandom() < result * (double)BAT_RandomTotal)
            {                                               //if passes camera proximity test
#if BATTLE_TEXT
                if(chat->texts != NULL)
                {
                    battleSpeechText(chat->eventNumber, chat->texts);
                }
#endif
                chat->lastTimeSpoken = universe.totaltimeelapsed;
                chat->lastPlaceSpoken = ship->posinfo.position;
                return(speechEventQueue(ship, chat->eventNumber,
                    variable, SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT,
                    universe.curPlayerIndex, linkTo,
                    (real32)SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT));
                return(ERROR);
                //... actual speech event
            }
#if BATTLE_VERBOSE_LEVEL >= 2
            else
            {
                dbgMessagef("\n...Event %d above distance curve.", chat->eventNumber);
            }
#endif
        }
#if BATTLE_VERBOSE_LEVEL >= 2
        else
        {
            dbgMessagef("\n...Event %d too far away.", chat->eventNumber);
        }
#endif
    }
#if BATTLE_VERBOSE_LEVEL >= 2
    else
    {
        dbgMessagef("\n...Event %d failed randomWeight check.", chat->eventNumber);
    }
#endif
    return(ERROR);
}

/*-----------------------------------------------------------------------------
    Name        : battleCanChatterAtThisTime
    Description : See if a particular event will work at this time (i.e. it
                    has not been used around here recently)
    Inputs      : event - what event to consider
                  ship - what ship we would like to play it on.  If NULL,
                  camera's position will be used for location information if needed.
    Outputs     :
    Return      : TRUE if it's OK to chatter at this time.
----------------------------------------------------------------------------*/
bool battleCanChatterAtThisTime(sdword event, Ship *ship)
{
    battleevent *chat;
    double timeSinceLastSpoken, result, distance;
    vector diff;

    dbgAssert(event >= 0 && event < BCE_LastBCE);
    chat = &battleChatterEvent[event];
#if BATTLE_TUNING
    if (batChatterToTune != NULL && batChatterToTune != chat)
    {                                                       //if we're tuning only one chatter event
        return(ERROR);                                      //no jouez-vous
    }
#endif
    timeSinceLastSpoken = (double)(universe.totaltimeelapsed - chat->lastTimeSpoken);

    dbgAssert(chat->minWavelength > 0.0);
    if (timeSinceLastSpoken > chat->minWavelength)
    {                                                       //if it hasn't been spoken in a coon's age
        return(TRUE);
    }
    result = timeSinceLastSpoken / chat->minWavelength;
    if (chat->expWavelength != 1.0f)
    {                                                       //find proper spot on probability curve
        result = pow(result, chat->expWavelength);
    }
    if ((double)batRandom() >= result * (double)BAT_RandomTotal)
    {
#if BATTLE_VERBOSE_LEVEL >= 2
        dbgMessagef("\n...Event %d above wavelength curve.", chat->eventNumber);
#endif
        return(FALSE);
    }
    if (chat->minRepeatProximity > 0.0f)
    {                                                       //if we should consider location of last time this event was spoken
        if (ship != NULL)
        {
            vecSub(diff, ship->posinfo.position, chat->lastPlaceSpoken);
        }
        else
        {
            vecSub(diff, mrCamera->eyeposition, chat->lastPlaceSpoken);
        }
        distance = fmathSqrtDouble((double)vecMagnitudeSquared(diff));
        if (distance < chat->minRepeatProximity)
        {
            result = distance / chat->minRepeatProximity;
            if (chat->expRepeatProximity != 1.0f)
            {
                result = pow(result, chat->expRepeatProximity);
            }
            if ((double)batRandom() >= result * (double)BAT_RandomTotal)
            {
#if BATTLE_VERBOSE_LEVEL >= 2
                dbgMessagef("\n...Event %d above proximity curve.", chat->eventNumber);
#endif
                return(FALSE);
            }
        }
    }
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : battleChatterFleetAttempt
    Description : Tries to play a fleet command battle chatter event.
    Inputs      : linkTo - what speech event to link to, or SOUND_EVENT_DEFAULT
                  event - what event to play
                  variable - variable for speech event or SOUND_EVENT_DEFAULT
                  where - position info so that we can use proximity tuning.
                    May be NULL
    Outputs     :
    Return      : handle of queued speech event, or ERROR
    Note        : This function should only be called for the correct player as
                    there is no ship parameter to determine what player this
                    event belongs to.
----------------------------------------------------------------------------*/
sdword battleChatterFleetAttempt(sdword linkTo, sdword event, sdword variable, vector *where)
{
    battleevent *chat;
    double timeSinceLastSpoken, result;
    udword randomWeight;
    vector diff;
    real32 distance;

    dbgAssert(event >= 0 && event < BCE_LastBCE);
    chat = &battleChatterEvent[event];
#if BATTLE_TUNING
    if (batChatterToTune != NULL && batChatterToTune != chat)
    {                                                       //if we're tuning only one chatter event
        return(ERROR);                                      //no jouez-vous
    }
#endif
    timeSinceLastSpoken = (double)(universe.totaltimeelapsed - chat->lastTimeSpoken);

    dbgAssert(chat->minWavelength > 0.0);
    if (timeSinceLastSpoken > chat->minWavelength)
    {                                                       //if it hasn't been spoken in a coon's age
        goto readyToPlay;
    }
    result = timeSinceLastSpoken / chat->minWavelength;
    if (chat->expWavelength != 1.0f)
    {                                                       //find proper spot on probability curve
        result = pow(result, chat->expWavelength);
    }
    if ((double)batRandom() >= result * (double)BAT_RandomTotal)
    {
#if BATTLE_VERBOSE_LEVEL >= 2
        dbgMessagef("\n...Event %d above wavelength curve.", chat->eventNumber);
#endif
        return(ERROR);
    }
readyToPlay:
    if (chat->minRepeatProximity > 0.0f && where != NULL)
    {                                                       //if we should consider location of last time this event was spoken
        vecSub(diff, *where, chat->lastPlaceSpoken);
        distance = fmathSqrtDouble((double)vecMagnitudeSquared(diff));
        if (distance < chat->minRepeatProximity)
        {
            result = distance / chat->minRepeatProximity;
            if (chat->expRepeatProximity != 1.0f)
            {
                result = pow(result, chat->expRepeatProximity);
            }
            if ((double)batRandom() >= result * (double)BAT_RandomTotal)
            {
#if BATTLE_VERBOSE_LEVEL >= 2
                dbgMessagef("\n...Event %d above proximity curve.", chat->eventNumber);
#endif
                return(ERROR);
            }
        }
    }

    randomWeight = chat->randomWeight * batGlobalFrequencyModifier / BIT8;
    if (batRandom() < randomWeight)
    {                                                       //if it passes random weight test
#if BATTLE_TEXT
        if(chat->texts != NULL)
        {
            battleSpeechText(chat->eventNumber, chat->texts);
        }
#endif
        if (where)
        {                                                   //remember where it was spoken
            chat->lastPlaceSpoken = *where;
        }
        chat->lastTimeSpoken = universe.totaltimeelapsed;
        return(speechEventQueue(NULL, chat->eventNumber,
            variable, SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT,
            universe.curPlayerIndex, linkTo,
            (real32)SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT));
        return(ERROR);
    }
#if BATTLE_VERBOSE_LEVEL >= 2
    else
    {
        dbgMessagef("\n...Event %d failed randomWeight check.", chat->eventNumber);
    }
#endif
    return(ERROR);

}

/*-----------------------------------------------------------------------------
    Name        : battleMouthPieceFind
    Description : Find a ship to do some talking.  The ship needs to be one of
                    the player's ships.
    Inputs      : ships - list of ships to look through
                  numShips - number of ships in list
    Outputs     :
    Return      : Pointer to one of the players' ships, or NULL if one could not be found.
----------------------------------------------------------------------------*/
Ship *battleMouthPieceFind(Ship **ships, sdword numShips)
{
    Ship *mouthPiece;
    sdword tries = 0;

    do
    {
        tries++;
        if (tries > batMouthpieceTries)
        {
            return(NULL);
        }
        mouthPiece = ships[batRandom() % numShips];
    }
    while ( mouthPiece->playerowner != universe.curPlayerPtr);

    dbgAssert(mouthPiece->objtype == OBJ_ShipType);
    return(mouthPiece);
}

/*-----------------------------------------------------------------------------
    Name        : battleMouthPieceFindAnother
    Description : Same as above, but it will exclude the specified ship from
                    the search.
    Inputs      : firstMouthPiece - the ship that we don't want to be talking
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Ship *battleMouthPieceFindAnother(Ship *firstMouthPiece, Ship **ships, sdword numShips)
{
    Ship *mouthPiece;
    sdword tries = 0;

    if (numShips == 0)
    {
        return(NULL);
    }
    do
    {
        mouthPiece = ships[batRandom() % numShips];
        tries++;
        if (tries > batMouthpieceTries)
        {
            return(NULL);
        }
    }
    while ( mouthPiece->playerowner != universe.curPlayerPtr && mouthPiece != firstMouthPiece);

    dbgAssert(mouthPiece->objtype == OBJ_ShipType);
    return(mouthPiece);
}

/*-----------------------------------------------------------------------------
    Name        : battleShipOnMyTail
    Description : See if a ship is on the tail of another ship
    Inputs      : meShip - ship being followed
                  badGuyShip - ship [possibly] following us
    Outputs     :
    Return      : TRUE if the ship is on the tail
----------------------------------------------------------------------------*/
bool battleShipOnMyTail(Ship *meShip, Ship *badGuyShip)
{
    vector temp, bogeyPosition;
    matrix transpose;

    matCopyAndTranspose(&meShip->rotinfo.coordsys, &transpose);//transpose of coordsys
    vecSub(temp, badGuyShip->posinfo.position, meShip->posinfo.position);//vector from us to them
    matMultiplyMatByVec(&bogeyPosition, &transpose, &temp); //badguy's position in our local space
    if (bogeyPosition.z < 0.0f)
    {                                           //if they're behind us
        //sort of a "Manhattan bogey attitude" check
        if (ABS(bogeyPosition.x) + ABS(bogeyPosition.y) < -bogeyPosition.z * batBogeyAspect)
        {                                       //and they're on our 6
            vecNegate(temp);                    //vector from them to us
            matCopyAndTranspose(&badGuyShip->rotinfo.coordsys, &transpose);//transpose of coordsys
            matMultiplyMatByVec(&bogeyPosition, &transpose, &temp); //our position in badguy's local space
            if (bogeyPosition.z > 0.0f)
            {                                   //if they're ahead of us
                if (ABS(bogeyPosition.x) + ABS(bogeyPosition.y) < bogeyPosition.z * batBogeyAspect)
                {                               //and we're in their sights
                    return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : battleAccountForLosses
    Description : Account for the loss of a ship on the compution of future BSN's
    Inputs      : ship - what ship was lost
                  battlePing - what ping it was lost from
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void battleAccountForLosses(Ship *ship, battleping *battlePing)
{
    real32 lossPower, lossValue;
    real32 timeSinceLoss;

    lossPower = gunShipFirePower(ship->staticinfo, ship->tacticstype);
    lossValue = (real32)ship->staticinfo->buildCost;

    if (ship->playerowner == universe.curPlayerPtr)
    {                                                       //if it's one of our ships we're losing
        if (battlePing->friendlyValueRecentlyLost != 0.0f)
        {
            timeSinceLoss = universe.totaltimeelapsed - battlePing->whenFriendliesLost;
            if (timeSinceLoss != 0.0f)
            {
                battlePing->friendlyFirepowerRecentlyLost /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
                battlePing->friendlyValueRecentlyLost /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
            }
        }
        battlePing->friendlyFirepowerRecentlyLost += lossPower;
        battlePing->friendlyValueRecentlyLost += lossValue;
        battlePing->whenFriendliesLost = universe.totaltimeelapsed;
    }
    else
    {                                                       //else it's an enemy that was lost
        if (battlePing->enemyValueRecentlyLost != 0.0f)
        {
            timeSinceLoss = universe.totaltimeelapsed - battlePing->whenEnemiesLost;
            if (timeSinceLoss != 0.0f)
            {
                battlePing->enemyFirepowerRecentlyLost /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
                battlePing->enemyValueRecentlyLost /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
            }
        }
        battlePing->enemyFirepowerRecentlyLost += lossPower;
        battlePing->enemyValueRecentlyLost += lossValue;
        battlePing->whenEnemiesLost = universe.totaltimeelapsed;
    }
}

#define thisPing ((ping *)voidPing)
/*-----------------------------------------------------------------------------
    Name        : battlePingEvaluateNoFriendlies
    Description : Handle a battle ping with no friendlies.
    Inputs      : thisPing - the ping who's evaluation function we are being
                    called from.
                  battlePing - the battle ping we are evaluating
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battlePingEvaluateNoFriendlies(void *voidPing, battleping *battlePing)
{
    ;//nothing to play in this case
}

/*-----------------------------------------------------------------------------
    Name        : battlePingEvaluateNoEnemies
    Description : Same as regular evaluation, except there are no enemies.
                    We've probably just won a battle, or maybe it's friendly
                    or allied fire.
    Inputs      : thisPing - the ping who's evaluation function we are being
                    called from.
                  battlePing - the battle ping we are evaluating
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battlePingEvaluateNoEnemies(void *voidPing, battleping *battlePing)
{
    Ship *mouthPiece = NULL;
    sdword handle;
    sdword index;
    Ship *ship;
    bool bFriendlyFire = FALSE;

    //do a per-ship scan for some events
    for (index = 0; index < battlePing->nShips; index++)
    {
        ship = battlePing->ship[index];
        dbgAssert(ship->objtype == OBJ_ShipType);
        dbgAssert(ship->playerowner != NULL);
        if (ship->shiptype == Mothership)
        {                                                   //don't even have the mothership talking
            continue;
        }
        if (/*ship->shiprace > R2 || */ship->playerowner == universe.curPlayerPtr)
        {                                                   //if this is a friendly ship (thus we can hear it speak)
            //Are my guys someone shooting at me?
            if (bitTest(ship->chatterBits, BCB_FriendlyFire))
            {
                if (selHotKeyGroupNumberTest(ship))
                {
                    battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_GroupFriendlyFire, selHotKeyGroupNumberTest(ship), NULL);
                }
                else
                {
                    if (battleCanChatterAtThisTime(BCE_FriendlyFire, ship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_FriendlyFire, ship, SOUND_EVENT_DEFAULT);
                    }
                }
                bitClear(ship->chatterBits, BCB_FriendlyFire);
                bFriendlyFire = TRUE;
            }
            //Are my allies someone shooting at me?
            if (bitTest(ship->chatterBits, BCB_AlliedFire))
            {
                if (selHotKeyGroupNumberTest(ship))
                {                                           //if allies are attacking a selected group
                    if (battleCanChatterAtThisTime(BCE_GroupAttackFromAlly, ship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_GroupAttackFromAlly, ship, selHotKeyGroupNumberTest(ship));
                    }
                }
                else
                {                                           //not a group number; report from fleet command
                    battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_AttackFromAlly, SOUND_EVENT_DEFAULT, &ship->posinfo.position);
                }
                bitClear(ship->chatterBits, BCB_AlliedFire);
                bFriendlyFire = TRUE;
            }
        }
    }
    //if there was no friendly fire, it must mean we've just won a battle
    if (!bFriendlyFire && battlePing->enemyValueRecentlyLost)
    {
        mouthPiece = battleMouthPieceFind(battlePing->ship, battlePing->nShips);
        if (battleCanChatterAtThisTime(BCE_BattleWon, mouthPiece))
        {
            handle = ERROR;
            if (mouthPiece != NULL)
            {
                if (battleCanChatterAtThisTime(BCE_BattleWon, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_BattleWon, mouthPiece, SOUND_EVENT_DEFAULT);
                }
                if (handle == ERROR && selHotKeyGroupNumberTest(mouthPiece))
                {                                               //if could not play the ship's victory dance
                    battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_GroupVictory, selHotKeyGroupNumberTest(mouthPiece), &mouthPiece->posinfo.position);
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : battlePingEvaluate
    Description : Called from the corresponding ping's evaluation function,
                    evaluate the battle ping and maybe make it say something.
    Inputs      : thisPing - the ping who's evaluation function we are being
                    called from.
                  battlePing - the battle ping we are evaluating
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battlePingEvaluate(void *voidPing, battleping *battlePing)
{
    sdword index;
    Ship *ship;
    real32 enemyPower = 0.0f, friendlyPower = 0.0f;
    real32 enemyValue = 0.0f, friendlyValue = 0.0f;
    real32 enemyStanding, friendlyStanding, battleStanding;
    real32 timeSinceLoss, lossPower, lossValue;
    Ship *mouthPiece = NULL, *mothership;
    sdword handle;
    CommandToDo *command;
    //SelectCommand *ships;
    vector diff;
    udword chances, saveChance[4];

#if BATTLE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nEvaluating ping 0x%x", battlePing);
#endif
    //do a per-ship scan for some events
    for (index = 0; index < battlePing->nShips; index++)
    {
        ship = battlePing->ship[index];
        dbgAssert(ship->objtype == OBJ_ShipType);
        dbgAssert(ship->playerowner != NULL);
        if (ship->flags & SOF_Dead)
        {
            continue;   // dead ships shouldn't interfere
        }
        if (ship->shiptype == Mothership)
        {                                                   //don't even have the mothership talking
            continue;
        }
        if (/*ship->shiprace > R2 || */ship->playerowner == universe.curPlayerPtr)
        {                                                   //if this is a friendly ship (thus we can hear it speak)
            //count up the power and cost of your ships
            friendlyPower += gunShipFirePower(ship->staticinfo, ship->tacticstype);
//            friendlyValue += (real32)ship->staticinfo->buildCost * ship->health / ship->staticinfo->maxhealth;
            friendlyValue += (real32)ship->staticinfo->buildCost;

            // do some high-g grunting if flight maneuvers are going on
//            if (ship->flightman > FLIGHTMAN_DONOTHING)      //!!! is this the proper flightman check?
//            {
//                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_ManeuverGrunts, ship, SOUND_EVENT_DEFAULT);
//            }
#if BATTLE_VERBOSE_LEVEL >= 2
//            else
//            {
//                dbgMessagef("\n...No flightman");
//            }
#endif
            //leaders may make snaps at their wingmen
            if (ship->tacticstype == Evasive)
            {                                               //if in evasive tactics
                if (ship->attackvars.myWingmanIs != NULL)
                {                                           //if ship has a wingman
                    if (battleCanChatterAtThisTime(BCE_Wingman, ship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Wingman, ship, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            //wingman (or another party in the battle) may tell this ship that they have a tail
            if (ship->recentlyFiredUpon)
            {                                               //if someone has been attacking
                dbgAssert(ship->firingAtUs != NULL);
                if (battleShipOnMyTail(ship, ship->firingAtUs))
                {
                    handle = ERROR;
                    if (ship->tacticstype == Evasive && ship->attackvars.myWingmanIs != NULL)
                    {                                       //the wingman should say it if possible
                        mouthPiece = ship->attackvars.myWingmanIs;
                        if (mouthPiece != NULL && battleCanChatterAtThisTime(BCE_LeaderChased, mouthPiece))
                        {
                            handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LeaderChased, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                    else
                    {                                       //otherwise anybody else could say it
                        mouthPiece = battleMouthPieceFindAnother(ship, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL && battleCanChatterAtThisTime(BCE_WingmanChased, mouthPiece))
                        {                                       //if there is someone to say there is a bogey
                            handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WingmanChased, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                    if (handle != ERROR)
                    {
                        if (battleCanChatterAtThisTime(BCE_WingmanChasedRsp, ship))
                        {
                            battleChatterAttempt(handle, BCE_WingmanChasedRsp, ship, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
            }
            //say that a ship is recently hit, someone may respond
            if (ship->recentlyAttacked > BAT_ReallyRecentAttack)
            {                                               //if attacked quite recently
                handle = ERROR;
                if (battleCanChatterAtThisTime(BCE_WingmanHit, ship))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WingmanHit, ship, SOUND_EVENT_DEFAULT);
                }
                if (handle != ERROR)
                {
                    if (ship->tacticstype == Evasive && ship->attackvars.myWingmanIs != NULL)
                    {                                       //the wingman should say it if possible
                        mouthPiece = ship->attackvars.myWingmanIs;
                    }
                    else
                    {                                       //otherwise anybody else could say it
                        mouthPiece = battleMouthPieceFindAnother(ship, battlePing->ship, battlePing->nShips);
                    }
                    if (mouthPiece != NULL && battleCanChatterAtThisTime(BCE_WingmanHitRsp, mouthPiece))
                    {
                        battleChatterAttempt(handle, BCE_WingmanHitRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            //I've lost my wingman... bummer
            if (bitTest(ship->chatterBits, BCB_WingmanLost))
            {
                if (battleCanChatterAtThisTime(BCE_WingmanDies, ship))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WingmanDies, ship, SOUND_EVENT_DEFAULT);
                }
                bitClear(ship->chatterBits, BCB_WingmanLost);
            }
            //am I really hurtin'?
            if (bitTest(ship->chatterBits, BCB_DamageInTheRed))
            {
                handle = ERROR;
                if (bitTest(battleClassDependencies[ship->staticinfo->shipclass], BCD_Damaged))
                {                                           //if this class is allowed to complain about damage
                    if (battleCanChatterAtThisTime(BCE_CapitalShipDamaged, ship))
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipDamaged, ship, ship->staticinfo->shipclass);
                    }
                    if (handle == ERROR && battleCanChatterAtThisTime(BCE_CapitalDamagedSpecific, ship))
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalDamagedSpecific, ship, ship->shiptype);
                    }
                }
                bitClear(ship->chatterBits, BCB_DamageInTheRed);
            }
            //see if we're leaving the battle area
            command = ship->command;                        //get the ship's command
            if (command != NULL)
            {
                switch (command->ordertype.order)
                {
                    case COMMAND_MOVE:
                        //... see if he's moving out of the ping
                        vecSub(diff, thisPing->centre, command->move.destination);
                        if (fsqrt(vecMagnitudeSquared(diff)) > battlePing->radius * batRetreatModifier)
                        {                                   //if this ship is moving out of the ping
                            if (battleCanChatterAtThisTime(BCE_Retreat, ship))
                            {                               //if we can speak of retreat
                                handle = ERROR;
                                if (battleCanChatterAtThisTime(BCE_Retreat, ship))
                                {
                                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Retreat, ship, SOUND_EVENT_DEFAULT);
                                }
                                if (handle != ERROR)
                                {
                                    mouthPiece = battleMouthPieceFindAnother(ship, battlePing->ship, battlePing->nShips);
                                    if (mouthPiece != NULL)
                                    {
                                        battleChatterAttempt(handle, BCE_RetreatRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                                    }
                                }
                            }
                        }
                        break;
/*
                    case COMMAND_DOCK:
                        //... see if the dock ship is far enough away
                        break;
*/
                }
            }
            //... other allied-ships tests
        }
        else
        {                                                   //not a player ship
            if (allianceIsShipAlly(ship, universe.curPlayerPtr))
            {                                               //!!! consider your allies in the battle
                friendlyPower += gunShipFirePower(ship->staticinfo, ship->tacticstype);
//                friendlyValue += (real32)ship->staticinfo->buildCost * ship->health / ship->staticinfo->maxhealth;
                friendlyValue += (real32)ship->staticinfo->buildCost;
            }
            else
            {
                enemyPower += gunShipFirePower(ship->staticinfo, ship->tacticstype);
//                enemyValue += (real32)ship->staticinfo->buildCost * ship->health / ship->staticinfo->maxhealth;
                enemyValue += (real32)ship->staticinfo->buildCost;
            }
        }
        //... other non-allied ship tests
    }

    //compute the battle status number (BSN)
    friendlyStanding = friendlyPower * batStandingFriendlyPower + friendlyValue * batStandingFriendlyValue;
    enemyStanding = enemyPower * batStandingEnemyPower + enemyValue * batStandingEnemyValue;
    //... adjust these standings by the recent losses
    //save the chances of some events because they will have their randomWeight's modified
    saveChance[0] = battleChatterEvent[BCE_LosingBattle].randomWeight;
    saveChance[1] = battleChatterEvent[BCE_LosingBadly].randomWeight;
    saveChance[2] = battleChatterEvent[BCE_WinningBattle].randomWeight;
    saveChance[3] = battleChatterEvent[BCE_KickingButt].randomWeight;
    if (battlePing->friendlyValueRecentlyLost)
    {
        timeSinceLoss = universe.totaltimeelapsed - battlePing->whenFriendliesLost;
        lossPower = battlePing->friendlyFirepowerRecentlyLost;
        lossValue = battlePing->friendlyValueRecentlyLost;
        if (timeSinceLoss != 0.0f)
        {
            lossPower /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
            lossValue /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
        }
        if (friendlyValue != 0.0f)
        {
            chances = (udword)max(((lossValue / friendlyValue * batStandingFriendlyValue +
                         lossPower / friendlyPower * batStandingFriendlyValue) * (real32)BAT_RandomTotal), (real32)BAT_RandomTotal);
        }
        else
        {
            chances = BAT_RandomTotal;
        }
        battleChatterEvent[BCE_LosingBattle].randomWeight = chances;
        battleChatterEvent[BCE_LosingBadly].randomWeight = chances;
    }
    if (battlePing->enemyValueRecentlyLost)
    {
        timeSinceLoss = universe.totaltimeelapsed - battlePing->whenEnemiesLost;
        lossPower = battlePing->enemyFirepowerRecentlyLost;
        lossValue = battlePing->enemyValueRecentlyLost;
        if (timeSinceLoss != 0.0f)
        {
            lossPower /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
            lossValue /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
        }
        if (enemyValue != 0.0f)
        {
            chances = (udword)max(((lossValue / enemyValue * batStandingEnemyValue +
                         lossPower / enemyPower * batStandingEnemyValue) * (real32)BAT_RandomTotal), (real32)BAT_RandomTotal);
        }
        else
        {
            chances = BAT_RandomTotal;
        }
        battleChatterEvent[BCE_WinningBattle].randomWeight = chances;
        battleChatterEvent[BCE_KickingButt].randomWeight = chances;
    }

    if (enemyStanding > 0.0f)
    {
        battleStanding = friendlyStanding / enemyStanding;
    }
    else
    {
        battleStanding = -1.0f;
        //... play the battle won event here
    }

    mouthPiece = battleMouthPieceFind(battlePing->ship, battlePing->nShips);
    if (mouthPiece != NULL)
    {                                                       //found someone to do the talking; now make him say something
        if (battleStanding > batBSNKickingButt)
        {                                                   //> kickingbutt
            //see if the battle is just starting, and assess it if it is
            handle = ERROR;
            if (battleCanChatterAtThisTime(BCE_StartBattleAdvantaged, mouthPiece))
            {
                if (battleCanChatterAtThisTime(BCE_StartBattleAdvantaged, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleAdvantaged, mouthPiece, SOUND_EVENT_DEFAULT);
                }
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleAdvantagedRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                //...kicking butt speech event
                if (battleCanChatterAtThisTime(BCE_KickingButt, mouthPiece))
                {
                    handle = ERROR;
                    if (battleCanChatterAtThisTime(BCE_KickingButt, mouthPiece))
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_KickingButt, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                    if (handle != ERROR)
                    {
                        mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL)
                        {
                            battleChatterAttempt(handle, BCE_KickingButtRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
            }
        }
        else if (battleStanding > batBSNWinning)
        {                                                   //winning..kickingButt
            //see if the battle is just starting, and assess it if it is
            handle = ERROR;
            if (battleCanChatterAtThisTime(BCE_StartBattleAdvantaged, mouthPiece))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleAdvantaged, mouthPiece, SOUND_EVENT_DEFAULT);
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleAdvantagedRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                handle = ERROR;
                if (battleCanChatterAtThisTime(BCE_WinningBattle, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WinningBattle, mouthPiece, SOUND_EVENT_DEFAULT);
                    if (handle != ERROR)
                    {
                        mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL)
                        {
                            battleChatterAttempt(handle, BCE_WinningBattleRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
                if (handle == ERROR)
                {
                    if (battleCanChatterAtThisTime(BCE_PositiveBattle, mouthPiece))
                    {
                        if (battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_PositiveBattle, mouthPiece, SOUND_EVENT_DEFAULT) == ERROR)
                        {
                            battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_PositiveBattle_More, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
            }
        }
        else if (battleStanding > batBSNNeutral)
        {                                                   //neutral..winning
            //see if the battle is just starting, and assess it if it is
            handle = ERROR;
            if (battleCanChatterAtThisTime(BCE_StartBattleFair, mouthPiece))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleFair, mouthPiece, SOUND_EVENT_DEFAULT);
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleFairRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                if (battleCanChatterAtThisTime(BCE_NeutralBattle, mouthPiece))
                {
                    if (battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NeutralBattle, mouthPiece, SOUND_EVENT_DEFAULT) == ERROR)
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NeutralBattle_More, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
        }
        else if (battleStanding > batBSNLosing)
        {                                                   //losing..neutral
            handle = ERROR;
            mothership = universe.curPlayerPtr->PlayerMothership;
            if (mothership != NULL && selShipInSelection(battlePing->ship, battlePing->nShips, mothership))
            {
                //ships = mothership->collMyBlob->blobShips;
                //dbgAssert(ships != NULL);
                //if (selShipInSelection(ships->ShipPtr, ships->numShips, mothership))
                //{                                               //if the mothership is involved in this battle
                    if (battleCanChatterAtThisTime(BCE_MothershipLargeAttack, mothership))
                    {                                           //say the mothership is under heavy fire
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_MothershipLargeAttack, mothership, SOUND_EVENT_DEFAULT);
                        if (handle != ERROR && battleCanChatterAtThisTime(BCE_MothershipLargeAttackResp, mouthPiece))
                        {
                            battleChatterAttempt(handle, BCE_MothershipLargeAttackResp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                //}
            }
            //see if the battle is just starting, and assess it if it is
            if (handle == ERROR && battleCanChatterAtThisTime(BCE_StartBattleDisadvant, mouthPiece))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleDisadvant, mouthPiece, SOUND_EVENT_DEFAULT);
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleDisadvantRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                handle = ERROR;
                if (battleCanChatterAtThisTime(BCE_LosingBattle, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LosingBattle, mouthPiece, SOUND_EVENT_DEFAULT);
                    if (handle == ERROR)
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LosingBattle_More, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                    if (handle != ERROR)
                    {
                        mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL)
                        {
                            battleChatterAttempt(handle, BCE_LosingBattleRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
                if (handle == ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL && battleCanChatterAtThisTime(BCE_NegativeBattle, mouthPiece))
                    {
                        if (battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NegativeBattle, mouthPiece, SOUND_EVENT_DEFAULT) == ERROR)
                        {
                            if (battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NegativeBattle_More, mouthPiece, SOUND_EVENT_DEFAULT) == ERROR)
                            {
                                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NegativeBattle_More2, mouthPiece, SOUND_EVENT_DEFAULT);
                            }
                        }
                    }
                }
            }
        }
        else if (battleStanding > 0.0f)
        {                                                   //< losing (losing badly)
            //see if the battle is just starting, and assess it if it is
            handle = ERROR;
            mothership = universe.curPlayerPtr->PlayerMothership;
            if (mothership != NULL && selShipInSelection(battlePing->ship, battlePing->nShips, mothership))
            {
                //ships = mothership->collMyBlob->blobShips;
                //dbgAssert(ships != NULL);
                //if (selShipInSelection(ships->ShipPtr, ships->numShips, mothership))
                //{                                               //if the mothership is involved in this battle
                    if (battleCanChatterAtThisTime(BCE_MothershipLargeAttack, mothership))
                    {                                           //say the mothership is under heavy fire
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_MothershipLargeAttack, mothership, SOUND_EVENT_DEFAULT);
                        if (handle != ERROR && battleCanChatterAtThisTime(BCE_MothershipLargeAttackResp, mouthPiece))
                        {
                            battleChatterAttempt(handle, BCE_MothershipLargeAttackResp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                //}
            }
            if (handle == ERROR && battleCanChatterAtThisTime(BCE_StartBattleDisadvant, mouthPiece))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleDisadvant, mouthPiece, SOUND_EVENT_DEFAULT);
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleDisadvantRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                //...losing badly speech events
                if (battleCanChatterAtThisTime(BCE_LosingBadly, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LosingBadly, mouthPiece, SOUND_EVENT_DEFAULT);
                    if (handle == ERROR)
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LosingBadly_More, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                    if (handle != ERROR)
                    {
                        mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL)
                        {
                            battleChatterAttempt(handle, BCE_LosingBadlyRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
            }
        }
    }
    //restore these previously saved randomWeight values
    battleChatterEvent[BCE_LosingBattle].randomWeight  = saveChance[0];
    battleChatterEvent[BCE_LosingBadly].randomWeight   = saveChance[1];
    battleChatterEvent[BCE_WinningBattle].randomWeight = saveChance[2];
    battleChatterEvent[BCE_KickingButt].randomWeight   = saveChance[3];
}
#undef thisPing

/*-----------------------------------------------------------------------------
    Name        : battleChatterFrequencySet
    Description : Set the battle chatter frequency
    Inputs      : freq100 - value from 0 to 100 (compatible with FE sliders)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleChatterFrequencySet(sdword freq100)
{
    real32 value;
    value = ((real32)freq100 / 100.0f) * (batGlobalFrequencyMax - batGlobalFrequencyMin) + batGlobalFrequencyMin;
    batGlobalFrequencyModifier = (udword)((real32)pow(2.0, (double)value) * BIT8);
}

/*-----------------------------------------------------------------------------
    Name        : battleShipDyingWithTimeToScream
    Description : Attempt to play one or more death speech events for a given ship's death.
    Inputs      : ship - ship that is dying
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleShipDyingWithTimeToScream(Ship *ship)
{
    ShipClass shipclass = ship->staticinfo->shipclass;
    sdword handle = ERROR;
    Ship *mouthpiece;
    SelectCommand *ships;

    switch (shipclass)
    {
        case CLASS_Mothership:
            break;                                          //there's plenty enough death songs for the mothership
        case CLASS_HeavyCruiser:
        case CLASS_Carrier:
        case CLASS_Destroyer:
        case CLASS_Frigate:                                 //capital ships have special death songs
sameAsCapitalShips:
            if (battleCanChatterAtThisTime(BCE_CapitalShipDies, ship))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipDies, ship, SOUND_EVENT_DEFAULT);
            }
            if (handle != ERROR/* && ship->collMyBlob*/)        // blob ptr is always NULL here
            {
                //ships = ship->collMyBlob->blobShips;
                ships = (SelectCommand *)&selSelected;
                dbgAssert(ships != NULL);
                mouthpiece = battleMouthPieceFindAnother(ship, ships->ShipPtr, ships->numShips);
                if (mouthpiece != NULL && battleCanChatterAtThisTime(BCE_CapitalShipDiesReport, mouthpiece))
                {
                    battleChatterAttempt(handle, BCE_CapitalShipDiesReport, mouthpiece, shipclass);
                }
            }
            break;
        case CLASS_Corvette:
        case CLASS_Fighter:                                 //strike craft have a number of yells.
            if (battleCanChatterAtThisTime(BCE_WingmanLethal, ship))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WingmanLethal, ship, SOUND_EVENT_DEFAULT);
            }
            if (handle == ERROR)
            {
                if (battleCanChatterAtThisTime(BCE_HyperspaceAbandoned, ship))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_HyperspaceAbandoned, ship, SOUND_EVENT_DEFAULT);
                }
            }
            break;
        case CLASS_Resource:                                //resourcers: same as capital ships
            goto sameAsCapitalShips;
            break;
        case CLASS_NonCombat:                               //nothing for non-combat
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : battleLaunchWelcomeExchange
    Description : Play the lauch/welcome speech event exchange on launch of a
                    new super-capital ship.
    Inputs      : ship - ship that is just launched
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleLaunchWelcomeExchange(Ship *ship)
{
    sdword handle = ERROR;
    Ship *mouthpiece;
    SelectCommand *ships;

    if (!bitTest(battleClassDependencies[ship->staticinfo->shipclass], BCD_LaunchWelcome))
    {
         return;
    }
    if (battleCanChatterAtThisTime(BCE_CapitalShipLaunched, ship))
    {
        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipLaunched, ship, ship->shiptype);
    }
    if (handle != ERROR && ship->collMyBlob != NULL)
    {
        ships = ship->collMyBlob->blobShips;
        dbgAssert(ships != NULL);
        mouthpiece = battleMouthPieceFindAnother(ship, ships->ShipPtr, ships->numShips);
        if (mouthpiece != NULL)
        {
            battleChatterAttempt(handle, BCE_CapitalShipWelcomed, mouthpiece, mouthpiece->shiptype);
        }
    }
}

