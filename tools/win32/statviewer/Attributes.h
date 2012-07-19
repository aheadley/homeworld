/*=============================================================================
    Name    : Attributes.h
    Purpose : Definitions for attributes (properties) set for any object in Mission Editor

    Created 98/09/09 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___ATTRIBUTES
#define ___ATTRIBUTES

#define ATTRIBUTES_StripTechnology          0x0001
#define ATTRIBUTES_ShellOfResources         0x0002
#define ATTRIBUTES_VelToMothership          0x0004
#define ATTRIBUTES_DontApplyFriction        0x0008
#define ATTRIBUTES_KillerCollDamage         0x0010
#define ATTRIBUTES_Anomaly                  0x0020
#define ATTRIBUTES_DeleteAfterHSOut         0x0040
#define ATTRIBUTES_StartInHS                0x0080
#define ATTRIBUTES_Defector                 0x0100
#define ATTRIBUTES_HeadShotVelToMothership  0x0200
#define ATTRIBUTES_HeadShotKillerCollDamage 0x0400
#define ATTRIBUTES_Regrow                   0x0800

#define ATTRIBUTES_SMColor1                 0x1000      // 0 indicates no change, 1 indicates invisible, 2 indicates yellow, 3 indicates green
#define ATTRIBUTES_SMColor2                 0x2000
#define ATTRIBUTES_SMColorField             0x3000
#define ATTRIBUTES_SMColorInvisible         0x1000
#define ATTRIBUTES_SMColorYellow            0x2000
#define ATTRIBUTES_SMColorGreen             0x3000

#define ATTRIBUTES_ScaleResources           0x4000

#define ATTRIBUTES_TeamLeader               0x8000          // this ship is considered the leader of a team

// warning, attributes is only a word and cannot hold any more bits!

// defines for attributesParam
#define REGROWRATE_Max       7


#endif

