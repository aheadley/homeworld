/*=============================================================================
    Name    : madLinkInDefs.h
    Purpose : contains #defines for madAnimationFlags in ship structure 
        
    Created 31/08/1998 by bryce pasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

//madAnimationFlags Vars
#define MAD_ANIMATION_NEED_PROC             0x00000001
#define MAD_NEED_TO_START_NEW_ANIMATION     0x00000002

//Animation Type Vars
#define MAD_ANIMATION_NOTHING                   0
#define MAD_ANIMATION_GUN_OPENING               1
#define MAD_ANIMATION_GUN_CLOSING               2
#define MAD_ANIMATION_WINGS_OPENING             3
#define MAD_ANIMATION_WINGS_CLOSING             4
#define MAD_ANIMATION_DOOR_OPENING              5
#define MAD_ANIMATION_DOOR_CLOSING              6
#define MAD_ANIMATION_SPECIAL_OPENING           7
#define MAD_ANIMATION_SPECIAL_CLOSING           8       

//gun Animation Statuse
#define MAD_STATUS_NOSTATUS                            0

#define MAD_STATUS_GUNS_OPEN                           1
#define MAD_STATUS_GUNS_CLOSED                         2
#define MAD_STATUS_GUNS_OPENING                        3
#define MAD_STATUS_GUNS_CLOSING                        4

#define MAD_STATUS_WINGS_OPEN                          5
#define MAD_STATUS_WINGS_CLOSED                        6
#define MAD_STATUS_WINGS_OPENING                       7
#define MAD_STATUS_WINGS_CLOSING                       8

#define MAD_STATUS_DOOR_OPEN                           9
#define MAD_STATUS_DOOR_CLOSED                         10
#define MAD_STATUS_DOOR_OPENING                        11
#define MAD_STATUS_DOOR_CLOSING                        12

#define MAD_STATUS_SPECIAL_OPEN                        13
#define MAD_STATUS_SPECIAL_CLOSED                      14
#define MAD_STATUS_SPECIAL_OPENING                     15
#define MAD_STATUS_SPECIAL_CLOSING                     16


