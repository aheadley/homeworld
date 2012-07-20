/*=============================================================================
    Name    : LinkStart.c
    Purpose : Linked at the start of the project.

    Created 11/30/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"

/*=============================================================================
    Data:
=============================================================================*/

char LinkStartData[] = "0";
char *linkStartData = LinkStartData;

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : linkStartCode
    Description : Function to label the start of code.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
udword linkStartCode(udword var)
{
    return(var + 1);
}

