/*=============================================================================
    Name    : Undo.c
    Purpose : Logic and data for undoing actions

    Created 8/4/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Debug.h"
#include "Memory.h"
#include "Select.h"
#include "CommandWrap.h"
#include "Universe.h"
#include "Undo.h"

/*=============================================================================
    Data:
=============================================================================*/
undoinfo udUndoInfo;

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : udStartup
    Description : Start undo module
    Inputs      :
    Outputs     : clears out undo structures
    Return      :
----------------------------------------------------------------------------*/
void udStartup(void)
{
    memset(&udUndoInfo, 0, sizeof(udUndoInfo));
}

/*-----------------------------------------------------------------------------
    Name        : udShutdown
    Description : Shut down the undo module
    Inputs      :
    Outputs     : Frees memory if any is allocated to the undo structure
    Return      :
----------------------------------------------------------------------------*/
void udShutdown(void)
{
    udUndoInfo.function = NULL;                             //make sure no more calls
    if (udUndoInfo.userData != NULL)
    {
        memFree(udUndoInfo.userData);
        udUndoInfo.userData = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : udLatestThingPush
    Description : Set the latest thing done so we can undo it later
    Inputs      : function - function to call to perform the undo.
                  userID - arbitrary ID to be passed to callback function
                  userData - memory to duplicate and pass back.  Ignored if
                    length = 0
                  length - length of user memory to duplicate.  If zero, no
                    memory will be duplicated.
    Outputs     : allocates and duplicates userData, plus copies the parameters
                    to global variables
    Return      : ???
----------------------------------------------------------------------------*/
sdword udLatestThingPush(undofunc function, sdword userID, ubyte *userData, sdword length)
{
    if (udUndoInfo.userData != NULL)
    {                                                       //if user data was previously allocated
        memFree(udUndoInfo.userData);                       //free it
        udUndoInfo.userData = NULL;                         //and set pointer to reflect nothing allocated
    }
    udUndoInfo.function = function;                         //save parameters
    udUndoInfo.userID = userID;
    udUndoInfo.length = length;
    if (length != 0)
    {                                                       //if we should duplicate RAM
        udUndoInfo.userData = memAlloc(length, "Undo buffer", 0);
        memcpy(udUndoInfo.userData, userData, length);
    }
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : udLatestThingUndo
    Description : Undo the latest thing pushed onto undo buffer
    Inputs      : void
    Outputs     : calls pushed undo function and then frees allocated user RAM.
    Return      : ??
----------------------------------------------------------------------------*/
sdword udLatestThingUndo(void)
{
    if ((udUndoInfo.function == NULL) || (!udUndoInfo.function(udUndoInfo.userID, udUndoInfo.userData, udUndoInfo.length)))
    {
        // nothing to undo in terms of gui, so if some ships are selected send the halt command
        if (selSelected.numShips > 0)
        {
            clWrapHalt(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
        }
    }
    udUndoInfo.function = NULL;
    if (udUndoInfo.userData != NULL)
    {                                                       //if there is user data to free
        memFree(udUndoInfo.userData);                       //free undo memory
        udUndoInfo.userData = NULL;                         //and set pointer to reflect nothing allocated
    }
    return(OKAY);
}

