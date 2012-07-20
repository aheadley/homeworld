/*=============================================================================
    Name    : LagPrint.c
    Purpose : This file contains all of the defines and headers for the lag
              system.

    Created 7/14/1999 by Drew Dunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___LAGPRINT_H
#define ___LAGPRINT_H

#ifdef _WIN32
#define LAG_PATH                    "FeMan\\LagIcon\\"
#else
#define LAG_PATH                    "FeMan/LagIcon/"
#endif

#define SLOW_COMPUTERICON           LAG_PATH"SlowComputerIcon.lif"
#define SLOW_INTERNETICON           LAG_PATH"SlowInternetIcon.lif"
#define INTERNET_LAG_UPDATERATE     31

void lagSlowComputerIcon(void);
void lagSlowInternetIcon(void);

void lagUpdateInternetLag(void);

void lagRecievedPacketCB(ubyte *packet,udword sizeofPacket);

#endif
