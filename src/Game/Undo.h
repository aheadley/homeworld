/*=============================================================================
    Name    : Undo.h
    Purpose : Definitions for undoing actions

    Created 8/4/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___UNDO_H
#define ___UNDO_H

#include "Types.h"
/*=============================================================================
    Type definitions:
=============================================================================*/
typedef bool (*undofunc)(sdword userID, ubyte *userData, sdword length);
typedef struct
{
    undofunc function;                          //function to call to undo
    sdword userID;                              //arbitrary user ID
    ubyte *userData;                            //allocated user data
    sdword length;                              //length of allocated user data
}
undoinfo;

/*=============================================================================
    Functions:
=============================================================================*/
//startup / shutdown
void udStartup(void);
void udShutdown(void);

//set the lastest thing
sdword udLatestThingPush(undofunc function, sdword userID, ubyte *userData, sdword length);

//undo the latest thing
sdword udLatestThingUndo(void);
#endif //___UNDO_H

