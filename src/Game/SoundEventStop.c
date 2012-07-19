/*=============================================================================
    Name    : SoundEventStop.c
    Purpose : Sound Event Stop functions

    Created 5/8/98 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "SoundEventPrivate.h"



/*-----------------------------------------------------------------------------
    Name        :	soundEventStopSound
    Description :
    Inputs      :	soundhandle - handle of the sound to stop
					numblocks - number of DCT blocks to fade over
    Outputs     :
    Return      :	SOUND_OK if successful, error ( < 0) if fails
----------------------------------------------------------------------------*/
sdword soundEventStopSound(sdword soundhandle, sdword numblocks)
{
	if (!enableSFX)
	{
		return (SOUND_OK);
	}

	if (numblocks < SOUND_OK)
		return (soundstop(soundhandle, SOUND_FADE_STOPNOW));

	return (soundstop(soundhandle, SOUND_FADE_STOPTIME));
}


/*-----------------------------------------------------------------------------
    Name        :	soundEventBustStop
    Description :
    Inputs      :	ship - the ship that is firing
					gun - the gun that is firing
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventBurstStop(Ship *ship, Gun *gun)
{
	if (!enableSFX)
	{
		return;
	}

	if (ship->soundevent.burstfiring)
	{
		ship->soundevent.burstfiring = FALSE;

		if (ship->soundevent.burstHandle > SOUND_NOTINITED)
		{
			if (ship->staticinfo->shiptype == DefenseFighter)
			{
				soundstop(ship->soundevent.burstHandle, SOUND_FADE_STOPNOW);
			}
			else
			{
				soundstop(ship->soundevent.burstHandle, SOUND_FADE_STOPTIME);
			}
		    ship->soundevent.burstHandle = SOUND_NOTINITED;
		}
	}
}

