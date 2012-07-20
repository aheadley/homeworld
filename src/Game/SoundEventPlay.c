/*=============================================================================
    Name    : SoundEventPlay.c
    Purpose : Sound Event Play functions

    Created 5/8/98 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "SoundEventPrivate.h"
#include "SoundEvent.h"
#include "Gun.h"
#include "Matrix.h"

#define OBJ_None    -1
#define OBJ_Gun     -2

#define EFFECT_SIZE	500.0f

extern real32 volSFX;
typedef struct
{
	sdword max;
	sdword handle[2];
} effectHandles;

effectHandles effectHandle[NUM_HIT_EVENTS];

void SEinitHandles(void)
{
	sdword i;

	for (i = 0; i < NUM_HIT_EVENTS; i++)
	{
		if ((i == Hit_Nick) || (i == Hit_Ricochet))
		{
			effectHandle[i].max = 1;
		}
		else
		{
			effectHandle[i].max = 2;
		}
		effectHandle[i].handle[0] = SOUND_NOTINITED;
		effectHandle[i].handle[1] = SOUND_NOTINITED;
	}
}

/*-----------------------------------------------------------------------------
    Name        :   soundEventPlay
    Description :
    Inputs      :   object - pointer to a spaceobj, ie a Ship or an Effect
                    event - the sound event to play
                    gun - pointer to a gun if this is a gun event
    Outputs     :
    Return      :   handle to the sound, this can be used to turn off the sound later
----------------------------------------------------------------------------*/
sdword soundEventPlay(void *object, sdword event, Gun *gun)
{
    sdword handle = SOUND_NOTINITED;
#if SOUND
    Ship *ship;
    sword vol;
    sword pan;
	sdword i;
    real32 dist;
    sdword priority = SOUND_PRIORITY_MIN;
    SpaceObj *spaceobject;
    Effect *effect;
    ObjType objtype;
    real32 tempEQ[SOUND_EQ_SIZE];
	sdword eventflag;
    ShipClass shipclass;


    if (!enableSFX)
    {
        return (SOUND_OK);
    }

    if (object != NULL)
    {
        spaceobject = (SpaceObj *)object;
        if (!univSpaceObjInRenderList(spaceobject) && (event != Ship_Hyperdrive) && (event != Ship_HyperdriveOff) && (event != Ship_SinglePlayerHyperspace))
        {
            return (handle);        // only do sound events for viewable objects
        }

        if (gun != NULL)
        {
            objtype = OBJ_Gun;
        }
        else
        {
            objtype = spaceobject->objtype;
        }

        dist = fsqrt(spaceobject->cameraDistanceSquared);
    }
    else
    {
        objtype = OBJ_None;
    }

	if (event & SFX_Flag_Mask)
	{
		eventflag = event & SFX_Flag_Mask;
	
		// new stuff
		switch (eventflag)
		{
			case UI_Flag:
				if (event <= UI_Trading)
				{
					// play the ambient sound
					soundstop(ambienthandle, SOUND_FADE_STOPTIME);
					ambienthandle = splayPRV(UIBank, UIEventsLUT->lookup[GetPatch(UIEventsLUT, 0, event)],
												 SOUND_PAN_CENTER, SOUND_PRIORITY_STREAM,
												 (sword)(SFX_AMBIENT_VOLUME * volSFX));
				}
				else
				{
					handle = splayFPRVL(UIBank, UIEventsLUT->lookup[GetPatch(UIEventsLUT, 0, event)],
										NULL, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_PRIORITY_MAX,
										(sword)(SOUND_VOL_MAX * volSFX), FALSE, FALSE, FALSE);
				}
	
				break;
			
			case Gun_Flag:
				if (smSensorsActive || !mrRenderMainScreen)
				{
					// don't want to play this sound from the manager screens
					break;
				}
				
				ship = (Ship *)object;
	
				dbgAssert(gun != NULL);
				dbgAssert(gun->gunstatic != NULL);
				if (gun->gunstatic->gunsoundtype >= GunEventsLUT->numobjects)
				{
					break;
				}
				
				if (event == Gun_WeaponFireLooped)
				{
					if ((!SEinrange(gun->gunstatic->gunsoundtype + GUNSHOT_OFFSET, dist)) || // is it in range
                        (ship->soundevent.burstHandle != SOUND_NOTINITED) || // does the sound engine think its already playing
						!soundover(ship->soundevent.burstHandle)) // is it actually still playing
					{
						break;
					}
					
					ship->soundevent.burstfiring = TRUE;
					priority = SOUND_PRIORITY_MAX;
					vol = (sdword)(SEequalize(gun->gunstatic->gunsoundtype + GUNSHOT_OFFSET, dist, tempEQ)
								   * (1.0f - (ship->soundevent.coverage * 0.5)));
					pan = getPanAngle(ship->posinfo.position, ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);
					
					handle = splayEPRV(GunBank, GunEventsLUT->lookup[GetPatch(GunEventsLUT, gun->gunstatic->gunsoundtype, event)], tempEQ, pan, priority, vol);
					ship->soundevent.burstHandle = handle;
				}
				else if (event == Gun_WeaponShot)
				{
					if (!SEinrange(gun->gunstatic->gunsoundtype + GUNSHOT_OFFSET, dist) ||
						!soundover(ship->soundevent.gunHandle))
					{
						break;
					}

					priority = SOUND_PRIORITY_HIGH;
					vol = (sdword)(SEequalize(gun->gunstatic->gunsoundtype + GUNSHOT_OFFSET, dist, tempEQ)
								   * (1.0f - ship->soundevent.coverage));
					pan = getPanAngle(ship->posinfo.position, ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);
					
					handle = splayEPRV(GunBank, GunEventsLUT->lookup[GetPatch(GunEventsLUT, gun->gunstatic->gunsoundtype, event)], tempEQ, pan, priority, vol);
					ship->soundevent.gunHandle = handle;
				}
				else	// event = Gun_WeaponMove
				{
					if (!SEinrange(gun->gunstatic->gunsoundtype + GUNMOVE_OFFSET, dist) ||
						!soundover(ship->soundevent.gunHandle))
					{
						break;
					}
					priority = SOUND_PRIORITY_MIN;
					vol = (sdword)(SEequalize(gun->gunstatic->gunsoundtype + GUNMOVE_OFFSET, dist, tempEQ)
								   * (1.0f - ship->soundevent.coverage));
					pan = getPanAngle(ship->posinfo.position, ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);
					handle = splayEPRV(GunBank, GunEventsLUT->lookup[GetPatch(GunEventsLUT, gun->gunstatic->gunsoundtype, event)], tempEQ, pan, priority, vol);
				}
				break;
	
			case ShipCmn_Flag:
				if (smSensorsActive || !mrRenderMainScreen)
				{
					// don't want to play this sound from the manager screens
					break;
				}
				if (objtype != OBJ_ShipType)
				{
					break;
				}
				
				ship = (Ship *)object;
				shipclass = ship->staticinfo->shipclass;

				if (SEinrange(shipclass, dist))
				{
					if (event == ShipCmn_Engine)
					{
						if (ship->soundevent.engineHandle != SOUND_NOTINITED)
						{
							break;
						}
	
						ship->soundevent.engineState = SOUND_STARTING;
						if (shipclass == CLASS_Corvette)
						{
							priority = SOUND_PRIORITY_MAX - 1;
						}
						else if (shipclass == CLASS_Fighter)
						{
							priority = SOUND_PRIORITY_MAX - 2;
						}
						else
						{
							priority = SOUND_PRIORITY_MAX;
						}
						
						vol = SEequalize(shipclass, dist, tempEQ);
					
						if ((shipclass == CLASS_Fighter) ||
							(shipclass == CLASS_Corvette))
						{
							vol = (sword)(vol * (1.0f - ship->soundevent.coverage));
						}
					}
					else
					{
						vol = SEequalize(shipclass + AMBIENT_OFFSET, dist, tempEQ);
						vol = (sword)(vol * (1.0f - ship->soundevent.coverage));
					}
	
					pan = getPanAngle(ship->posinfo.position, ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);
	
					if (ship->shiptype < ShipCmnEventsLUT->numobjects)
					{
						handle = splayEPRV(ShipBank, ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, ship->shiptype, event)], tempEQ, pan, priority, vol);
					}
					
					if (event == ShipCmn_Engine)
					{
						ship->soundevent.engineHandle = handle;
						if (handle == SOUND_ERR)
						{
							ship->soundevent.engineState = SOUND_NOTINITED;
						}
					}
				}
					
				break;
	
			case Ship_Flag:
				if (smSensorsActive || !mrRenderMainScreen)
				{
					// don't want to play this sound from the manager screens
					break;
				}
				if (objtype != OBJ_ShipType)
				{
					// can't do ship events if its not a ship
					break;
				}
				
				ship = (Ship *)object;
	
				if (!SEinrange(ship->staticinfo->shipclass, dist) && (event != Ship_SelectProbe) && (event != Ship_SinglePlayerHyperspace))
				{
					// the Ship_SelectProbe event doesn't have a range limit on it,
					// neither does the Ship_SinglePlayerHyperspace
					// so only break if its out of range and its not that event.
					break;
				}
				
				if ((event != Ship_SelectProbe) && (event != Ship_SinglePlayerHyperspace))
				{
					// calculate the volume and EQ
					vol = SEequalize(ship->staticinfo->shipclass, dist, tempEQ);
				}
				else
				{
					// this is played at a constant volume with no EQ
					vol = (sword)(SFX_AMBIENT_VOLUME * volSFX);
					for (i = 0; i < SOUND_EQ_SIZE; i++)
					{
						tempEQ[i] = 1.0f;
					}
				}
				
				pan = getPanAngle(ship->posinfo.position, ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);
				priority = SOUND_PRIORITY_NORMAL;

				if ((event == Ship_Hyperdrive) || (event == Ship_HyperdriveOff))
				{
					handle = splayPRV(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, event)], SOUND_PAN_CENTER, SOUND_PRIORITY_MAX, vol);
					ship->soundevent.hyperspaceHandle = handle;
				}
				else if (event == Ship_SinglePlayerHyperspace)
				{
					handle = splayPRV(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, event)], SOUND_PAN_CENTER, SOUND_PRIORITY_MAX, (sword)(SFX_HYPERSPACE_VOLUME * volSFX));
				}
				else
				{
					handle = splayEPRV(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, event)], tempEQ, pan, SOUND_PRIORITY_NORMAL, vol);
				}
				break;
			
			case Derelict_Flag:
				dbgMessagef("SoundEventPlay: Derelict event %d\n", event);
				break;
	
			case Exp_Flag:
				if (smSensorsActive || !mrRenderMainScreen)
				{
					// don't want to play this sound from the manager screens
					break;
				}
				dbgMessagef("SoundEventPlay: Explosion event %d\n", event);
				effect = (Effect *)object;
				dist = (real32)fsqrt(effect->cameraDistanceSquared);
				if (SEinrange((event - Exp_Flag) + EXPLOSION_OFFSET, dist))
				{
					vol = SEequalize((event - Exp_Flag) + EXPLOSION_OFFSET, dist, tempEQ);
					pan = getPanAngle(effect->posinfo.position, EFFECT_SIZE, dist);
	
					handle = splayEPRV(SpecialEffectBank, SpecExpEventsLUT->lookup[GetPatch(SpecExpEventsLUT, 0, event)], tempEQ, pan, SOUND_PRIORITY_MAX + 1, vol);
				}
				break;
	
			case Hit_Flag:
				if (smSensorsActive || !mrRenderMainScreen)
				{
					// don't want to play this sound from the manager screens
					break;
				}
				effect = (Effect *)object;
				if (SEinrangeSqr((event - Hit_Flag) + HIT_OFFSET, effect->cameraDistanceSquared)) // &&
//					soundover(effectHandle))
				{
					for (i = 0; i < effectHandle[event & SFX_Event_Mask].max; i++)
					{
						if (soundover(effectHandle[event & SFX_Event_Mask].handle[i]))
						{
							dist = (real32)fsqrt(effect->cameraDistanceSquared);
							vol = SEequalize((event - Hit_Flag) + HIT_OFFSET, dist, tempEQ);
							pan = getPanAngle(effect->posinfo.position, EFFECT_SIZE, dist);
		
							handle = splayEPRV(SpecialEffectBank, SpecHitEventsLUT->lookup[GetPatch(SpecHitEventsLUT, 0, event)], tempEQ, pan, SOUND_PRIORITY_HIGH, vol);
							effectHandle[event & SFX_Event_Mask].handle[i] = handle;
							break;
						}
					}
				}
				break;
			
			default:
				dbgMessagef("\nsoundEventPlaySound: %d event", event);
				break;
		}
	}
#endif

    return (handle);
}


/*-----------------------------------------------------------------------------
    Name        :   soundEffect
    Description :   This is just a wrapper for soundEventPlay since the ETG
                    doesn't like it when I use a #define for soundEffect.
    Inputs      :   effect - pointer to an Effect
                    event - the sound event to play
    Outputs     :
    Return      :   handle to the sound, this can be used to turn off the sound later
----------------------------------------------------------------------------*/
sdword soundEffect(void *effect, sdword event)
{
    return (soundEventPlay(effect, event, NULL));
}


/*-----------------------------------------------------------------------------
    Name        :   soundEffectType
    Description :
    Inputs      :   effect - pointer to an Effect
                    event - the sound event to play
                    objecttype - the type of object playing the event (gun sound type)
    Outputs     :
    Return      :   handle to the sound, this can be used to turn off the sound later
----------------------------------------------------------------------------*/
sdword soundEffectType(Effect *effect, sdword event, sdword objecttype)
{
    sdword handle = SOUND_NOTINITED;
    sword vol;
    sword pan;
    real32 dist;
    real32 tempEQ[SOUND_EQ_SIZE];
    sdword eventflag;

    if (!enableSFX)
    {
        return (SOUND_OK);
    }

    if (event & SFX_Flag_Mask)
    {
        eventflag = event & SFX_Flag_Mask;

        switch (eventflag)
        {
            case Hit_Flag:
				if (smSensorsActive || !mrRenderMainScreen)
				{
					// don't want to play this sound from the manager screens
					break;
				}
                if (SEinrangeSqr((event - Hit_Flag) + HIT_OFFSET, effect->cameraDistanceSquared))
                {
                    dist = (real32)fsqrt(effect->cameraDistanceSquared);
                    vol = SEequalize((event - Hit_Flag) + HIT_OFFSET, dist, tempEQ);
                    pan = getPanAngle(effect->posinfo.position, EFFECT_SIZE, dist);

                    handle = splayEPRV(SpecialEffectBank, SpecHitEventsLUT->lookup[GetPatch(SpecHitEventsLUT, objecttype, event)], tempEQ, pan, SOUND_PRIORITY_NORMAL, vol);
                }
                break;

            default:
                dbgMessagef("\nShould use soundEffect for %d event instead.", event);
                break;
        }
    }

    return (handle);
}
