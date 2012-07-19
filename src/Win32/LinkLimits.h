/*=============================================================================
    Name    : LinkLimits.h
    Purpose : Data and functions from LinkStart and LinkEnd.c

    Created 11/30/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___LINKLIMITS_H
#define ___LINKLIMITS_H

/*=============================================================================
    Data:
=============================================================================*/
extern char *linkStartData;
extern char *linkEndData;

/*=============================================================================
    Functions:
=============================================================================*/
udword linkEndCode(udword var);
udword linkStartCode(udword var);

#endif //___LINKLIMITS_H

