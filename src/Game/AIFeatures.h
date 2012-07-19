/*=============================================================================
    Name    : AIFeatures.h
    Purpose : Contains all the flags for AIPlayer features

    Created 5/31/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef __AIFeatures_H
#define __AIFeatures_H

// feature types
#define ResourceFeature  0
#define DefenseFeature   1
#define AttackFeature    2
#define TeamFeature      3

//Resource Feature flags
//turned on and off by script or due to difficulty levels
#define AIR_ACTIVE_MOTHERSHIP               0x00000001      //mothership moves
#define AIR_ACTIVE_RESOURCE_COLLECTION      0x00000002      //resource collectors are optimized
#define AIR_ACTIVE_RESOURCE_CONTROLLER      0x00000004      //resource controller moves with resource collectors (NOTE: AIR_RESOURCE_CONTROLLER_REQUESTS should be activated)
#define AIR_ACTIVE_SUPPORT_FRIGATE          0x00000008      //support frigate moves towards harass and guard teams when they are low on fuel (NOTE: AIR_SUPPORT_FRIGATE_REQUESTS should be activated)
#define AIR_RESOURCE_DISTRESS_SIGNALS       0x00000010      //resource ships under fire send distress signals (NOTE: only active guard and temp guard react to distress signals)
#define AIR_SMART_RESEARCH_SHIP_REQUESTS    0x00000100      //research ships are requested depending on need, rather than random number
#define AIR_SMART_COLLECTOR_REQUESTS        0x00000200      //resource collectors are requested depending on need, rather than random number
#define AIR_RESOURCE_CONTROLLER_REQUESTS    0x00000400      //resource controllers are requested
#define AIR_SUPPORT_FRIGATE_REQUESTS        0x00000800      //support frigates are requested
#define AIR_AGGRESSIVE_RESOURCING           0x00001000      //resource ships are put into aggressive mode

//Hypersapce Feature flags
//Uses the Resource feature bitfield
#define AIF_HYPERSPACING                    0x01000000      //turns CPU hyperspacing on

//Defense Feature Flags
//turned on and off by script or due to difficulty levels
#define AID_GUARDING                        0x00000001      //guard teams are created - default to small teams in spherical formation around resource ships
#define AID_ACTIVE_GUARD                    0x00000002      //guard teams are larger and roam around.  React to distress signals and invasion of sphere of influence
#define AID_SPHERE_OF_INFLUENCE_INVADERS    0x00000004      //enemy ships within a certain distance away from the mothership are considered invaders and dealth with (NOTE: only active guard and temp guard react to invaders)
#define AID_MOTHERSHIP_DEFENSE              0x00000010      //mothership defense is activated - this option causes idle ships to defend the mothership
#define AID_MOTHERSHIP_DEFENSE_MEDIUM       0x00000020      //guard and active guard teams also get recalled and defend the mothership
#define AID_MOTHERSHIP_DEFENSE_HARDCORE     0x00000040      //all ships in the computer player's fleet available get recalled and defend the mothership
#define AID_CLOAK_DEFENSE                   0x00000100      //minimal cloak defense - if any cloak capable ship is detected, the computer player builds a few proximity sensors
#define AID_CLOAK_DEFENSE_RED               0x00000200      //more rigorous cloaking defense - if any cloak capable ship is detected within the sphere of influence, more proximity sensors are pumped out and roam around

//Attack Feature Flags
//turned on and off by script or due to difficulty levels
#define AIA_STATIC_RECONAISSANCE            0x00000001      //a probe is sent to the vicinity of the primary enemy's mothership
#define AIA_ACTIVE_RECONAISSANCE            0x00000002      //scouts roam the world increasing the computer player's vision considerably
#define AIA_ATTACK_FLEET_FAST               0x00000010      //cheap frigate attack
#define AIA_ATTACK_FLEET_GUARD              0x00000020      //frigate or destroyer attack with guarding ships
#define AIA_ATTACK_FLEET_BIG                0x00000040      //frigate and destroyer attack
#define AIA_ATTACK_FLEET_HUGE               0x00000080      //heavy cruiser attack
#define AIA_TAKEOUT_TARGET                  0x00000100      //a special team is built intended to eliminate a specific target
#define AIA_FANCY_TAKEOUT_TARGET            0x00000200      //a special team is built intended to eliminate a specific target, and another team is built to defend the first team
#define AIA_FIGHTER_STRIKE                  0x00001000      //an attack team made of a bunch of fighters
#define AIA_CORVETTE_STRIKE                 0x00002000      //an attack team made of a bunch of corvettes
#define AIA_FRIGATE_STRIKE                  0x00004000      //an attack team made of a bunch of frigates
#define AIA_HARASS                          0x00010000      //a special team intended to takeout resource ships, support ships and capital ships vulnerable to fighters
#define AIA_CAPTURE                         0x00020000      //a team of salvage capture corvettes is created - it tries to capture the player's ships
#define AIA_MINE                            0x00040000      //a team of minelayer corvettes is built and they raise havoc whereever they can
#define AIA_KAMIKAZE                        0x00080000      //fighter and corvette kamikaze when their health gets low
#define AIA_CLOAKGEN                        0x00100000      //a Cloak generator is built
#define AIA_GRAVWELL                        0x00200000      //a gravwell generator is built

//Team Feature Flags
//turned on and off by script or due to difficulty levels
#define AIT_ADVANCED_ATTACK                 0x00000001      //attack micromanagement
#define AIT_FLANK_ATTACK                    0x00000002      //attacks come from the side, top or bottom
#define AIT_CLOAKING                        0x00000004      //cloaking fighters and cloak generators cloak at appropriate times
#define AIT_TACTICS                         0x00000008      //in game tactics are used with moves that support them
#define AIT_GRAVWELL                        0x00000010      //gravwell generators do their stuff

#endif
