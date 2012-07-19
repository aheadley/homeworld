/*=============================================================================
    Name    : CommandDefs.h
    Purpose : List of commands and command attributes

    Created 4/26/1999 by fpoiker
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef __COMMANDDEFS_H
#define __COMMANDDEFS_H

#define COMMAND_NULL            0
#define COMMAND_MOVE            1
#define COMMAND_ATTACK          2
#define COMMAND_DOCK            3
#define COMMAND_LAUNCHSHIP      4
#define COMMAND_COLLECTRESOURCE 5
#define COMMAND_BUILDINGSHIP    6
#define COMMAND_SPECIAL         7
#define COMMAND_HALT            8
#define COMMAND_MILITARYPARADE  9
#define COMMAND_MP_HYPERSPACEING  10

#define COMMAND_IS_FORMATION    1
#define COMMAND_IS_PROTECTING   2
#define COMMAND_IS_PASSIVEATTACKING 4
#define COMMAND_IS_HOLDINGPATTERN   8
#define COMMAND_IS_ATTACKINGANDMOVING   16

#endif
