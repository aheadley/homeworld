/*=============================================================================
    Name    : LagPrint.c
    Purpose : This file prints the lag icons to the screen when it exceeds
              The specified amounts

    Created 7/14/1999 by Drew Dunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#include "Types.h"
#include "texreg.h"
#include "LagPrint.h"
#include "FEReg.h"
#include "CommandNetwork.h"

//  Data to be added to tweak.script later

real32 SLOW_INTERNETPRINTTIME = 5.0;

udword lagSlowCompX = 10;
udword lagSlowCompY = 10;

udword lagSlowIntX = 10;
udword lagSlowIntY = 43;

real32 lagCalculatedTotal = 0.0;

//void       *LagExchangeInfomutex = NULL;

/*-----------------------------------------------------------------------
   Name        : lagSlowComputerIcon
   Description : Displays the slow computer icon when the lag is to great
   Inputs      : none
   Outputs     : none
   Parameters  : void
   Return      : void
-----------------------------------------------------------------------*/
void lagSlowComputerIcon(void)
{
    lifheader *texture;

    texture = ferTextureRegisterSpecial(SLOW_COMPUTERICON, none, none);
    ferDraw(lagSlowCompX, lagSlowCompY, texture);
}


/*-----------------------------------------------------------------------
   Name        : lagSlowInternetIcon
   Description : Displays the slow Internet icon when the lag is to great
   Inputs      : none
   Outputs     : none
   Parameters  : void
   Return      : void
-----------------------------------------------------------------------*/
void lagSlowInternetIcon(void)
{
    lifheader *texture;

    #ifndef HW_Release
    #ifdef ddunlop
    dbgMessagef("Total Internet Lag :%f\n",lagCalculatedTotal);
    #endif
    #endif

    if (lagCalculatedTotal > SLOW_INTERNETPRINTTIME)
    {
        texture = ferTextureRegisterSpecial(SLOW_INTERNETICON, none, none);
        ferDraw(lagSlowIntX, lagSlowIntY, texture);
    }
}

/*-----------------------------------------------------------------------
   Name        : lagUpdateInternetLag
   Description : This function updates the current internet lag by sending a packet to the captain
   Inputs      : none
   Outputs     : none
   Parameters  : void
   Return      : void
-----------------------------------------------------------------------*/
void lagUpdateInternetLag(void)
{
    LagPacket lagpacket;

    if (!IAmCaptain)
    {
        lagpacket.packetheader.type = PACKETTYPE_LAGCHECK;
        lagpacket.packetheader.from = (uword)sigsPlayerIndex;
        lagpacket.packetheader.frame = 0;
        lagpacket.packetheader.numberOfCommands = 0;

        lagpacket.timestamp = taskTimeElapsed;

        SendLagPacket(captainIndex, (ubyte *)&lagpacket);
    }
}

/*-----------------------------------------------------------------------
   Name        : lagRecievedPacketCB
   Description : This function is called when a lag packet is recieved
                 It is called from another thread so it must be protected by a mutex
   Inputs      : packet and sizeof the packet
   Outputs     : none
   Parameters  : ubyte *packet,udword sizeofPacket
   Return      : void
-----------------------------------------------------------------------*/
void lagRecievedPacketCB(ubyte *packet,udword sizeofPacket)
{
    LagPacket *lagpacket = (LagPacket *)packet;

    if (IAmCaptain)
    {
        SendLagPacket(lagpacket->packetheader.from, (ubyte*)lagpacket);
    }
    else
    {
        dbgAssert(lagpacket->packetheader.from==sigsPlayerIndex);
        // should never reciev lag packets form another player except yourself

        lagCalculatedTotal = taskTimeElapsed - lagpacket->timestamp;
    }
}
