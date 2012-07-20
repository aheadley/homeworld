/*=============================================================================
    Name    : SoundEvent.h
    Purpose : Defines for SoundEvent.c

    Created 7/24/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SOUND_EVENT_H
#define ___SOUND_EVENT_H

#include "Types.h"
#include "SpeechEvent.h"
#include "SoundStructs.h"
#include "SpaceObj.h"
#include "SoundMusic.h"
#include "SoundEventDefs.h"

/*=============================================================================
    Switches:
=============================================================================*/

#ifndef HW_Release

#define SE_ERROR_CHECKING          1           //general error checking
#define SE_VERBOSE_LEVEL           1           //control specific output code
#define SE_DEBUG                   1           //switch for command line options

#else //HW_Debug

#define SE_ERROR_CHECKING          0           //general error checking
#define SE_VERBOSE_LEVEL           0           //control specific output code
#define SE_DEBUG                   0           //switch for command line options

#endif //HW_Debug


#define SOUND_NOTINITED             -1

#define SOUND_EVENT_DEFAULT			-1

#define SOUND_FRONTEND_TRACK		AMB12_FrontEnd	// defined in soundmusic.h

#define SPEECH_HYPERSPACE_DELAY		2.5f

/*=============================================================================
    Tweaks
=============================================================================*/

extern real32	SPEECH_MOSHIP_WARNING_TIME;
extern real32	SPEECH_WARNING_TIME;

extern Ship *lastshiptospeak;
extern sdword lastgrouptospeak;


/* generic functions */
void soundEventInit(void);				/* Initializes audio mixer, stream and DirectSound/Waveout */
void soundEventClose(void);				/* Shuts down the mixer and frees resources, any soundevent calls will fail */
void soundEventReset(void);				/* Shuts down the mixer but does not free resources, soundevent calls will continue to work */
void soundEventUpdate(void);			/* Update function that does panning, volume, etc for objects in the universe */
void soundEventInitStruct(SOUNDEVENT *pseStruct);	/* initializes a SOUNDEVENT structure for use, all ships have a SOUNDEVENT structure */
void soundEventPause(bool bPause);		/* Shuts down or restore the mixer */
void soundEventReloadVolumes(void);
void soundEventShutdown(void);
void soundEventRestart(void);

/* sound FX functions */
sdword soundEventPlay(void *object, sdword event, Gun *gun);
#define soundEvent(a,b)					soundEventPlay(a, b, NULL)
#define soundEventMisc(a)				soundEventPlay(NULL, a, NULL)
#define soundEventStartEngine(a)		soundEventPlay(a, ShipCmn_Engine, NULL)
#define soundEventBurstFire(a,b)		soundEventPlay(a, Gun_WeaponFireLooped, b)

sdword soundEffect(void *effect, sdword event);
sdword soundEffectType(Effect *effect, sdword event, sdword objecttype);

sdword soundEventStopSound(sdword soundhandle, sdword numblocks);
#define soundEventStop(a)				soundEventStopSound(a, 0)

void soundEventBurstStop(Ship *ship, Gun *gun);

void soundEventShipDied(Ship *deadship);
void soundEventShipRemove(Ship *ship);
void soundEventDerelictRemove(Derelict *pDerelict);

void soundEventStopSFX(real32 fadetime);


/* speech functions */
void speechEventUpdate(void);			/* Update function so that speech continues to stream */
//sdword speechEventPlay(void *object, sdword event, sdword var, sdword playernum, sdword variation);
//#define speechEventFleet(a,b,c)			speechEventPlay(NULL, a, b, c, SOUND_NOTINITED)
//#define speechEvent(a,b,c)				speechEventPlay(a, b, c, SOUND_NOTINITED, SOUND_NOTINITED)
//#define speechEventVar(a, b, c, d)		speechEventPlay(a, b, c, SOUND_NOTINITED, d)

bool speechEventAttack(void);
void speechEventUnderAttack(Ship *target);

sdword speechEventQueue(void *object, sdword event, sdword var, sdword variation, sdword actornum, sdword playernum, sdword linkto, real32 timeout, sword volume);
#define speechEventFleet(a,b,c)		speechEventQueue(NULL, a, b, -1, 0, c, -1, 5.0f, -1)
#define speechEventFleetSpec(a,b,c,d)	speechEventQueue(a, b, c, -1, 0, d, -1, 5.0f, -1)
#define speechEventFleetVar(a,b,c,d)	speechEventQueue(NULL, a, b, c, 0, d, -1, 5.0f, -1)
#define speechEvent(a,b,c)			speechEventQueue(a, b, c, -1, -1, -1, -1, 0.0f, -1)
#define speechEventPlay(a, b, c, d, e)	speechEventQueue(a, b, c, e, -1, e, -1, 0.0f, -1)
#define speechEventVar(a, b, c, d)	speechEventQueue(a, b, c, d, -1, -1, -1, 0.0f, -1)

sdword speechEventRemoveShip(Ship *pShip);
sdword speechEventStop(sdword handle, real32 fadetime, bool addstatic);
sdword speechEventStopAllSpecific(real32 fadetime, sdword speechType);
#define speechEventStopAll(ft)		speechEventStopAllSpecific(ft, SPEECH_SINGLEPLAYER_FLAG)
sdword speechEventActorStop(udword actorMask, real32 fadetime);
#define speechEventFleetIntelStop(ft)       speechEventActorStop(ACTOR_FLEETINTEL_FLAG, (ft))
#define speechEventFleetCommandStop(ft)     speechEventActorStop(ACTOR_FLEETCOMMAND_FLAG, (ft))
#define ACTOR_ALL_ACTORS    (ACTOR_FLEETCOMMAND_FLAG | ACTOR_FLEETINTEL_FLAG | ACTOR_TRADERS_FLAG | ACTOR_PIRATES2_FLAG | ACTOR_ALLSHIPSENEMY_FLAG | ACTOR_AMBASSADOR_FLAG)
sdword speechEventCleanup(void);


/* Redbook audio functions */
void soundEventPlayCD(uword tracknum);
void soundEventStopCD(void);

/* Streamed Music functions */
void soundEventPlayMusic(sdword tracknum);
void soundEventStopMusic(real32 fadetime);
void soundEventStopTrack(sdword tracknum, real32 fadetime);
void musicEventNextTrack(void);
void musicEventPrevTrack(void);
sdword musicEventCurrentTrack(void);

/* Setup and Control functions */
void soundEventSFXVol(real32);
void soundEventSFXMasterVol(real32);
void soundEventSpeechVol(real32);
void soundEventSpeechMasterVol(real32);
void soundEventMusicVol(real32);
void soundEventMusicMasterVol(real32);
void soundEventMusicVolNOW(real32 level);
void soundEventMusicMasterVolNOW(real32 level);
void soundEventMasterEQ(real32 *pmasterEQ);
void soundEventSetActor(sdword actornum, bool bOn);
void soundEventVocalSettings(bool bCommands, bool bStatus, bool bChatter);
void soundEventGetVolume(real32 *sfxvol, real32 *speechvol, real32 *musicvol);
void soundEventHearActor(sdword actornum);
void soundEventSetActorFlag(sdword actorflag, bool bOn);

#endif

