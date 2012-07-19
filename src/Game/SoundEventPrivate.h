/*=============================================================================
    Name    : SoundEventPrivate.h
    Purpose : Private Defines for SoundEvent*.c files

    Created 9/8/98 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SOUND_PRIV_H
#define ___SOUND_PRIV_H

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "soundlow.h"
#include "types.h"
#include "switches.h"
#include "memory.h"
#include "file.h"
#include "universe.h"
#include "univupdate.h"
#include "fastmath.h"
#include "randy.h"
#include "main.h"
#include "fqeffect.h"

#include "mainrgn.h"

#include "CloakGenerator.h"
#include "GravWellGenerator.h"

#include "SoundEventDefs.h"
#include "VolTweakDefs.h"

/*=============================================================================
    Private Defines:
=============================================================================*/

#define AMBIENT_OFFSET		9
#define GUNSHOT_OFFSET		18
#define GUNMOVE_OFFSET		36
#define SPEC_OFFSET			54
#define EXPLOSION_OFFSET	58
#define HIT_OFFSET			65

#define SOUNDFXDIR "SoundFX\\"

#define CGW1 0
#if defined(Downloadable) || defined(DLPublicBeta)
#define SE_NUM_ACTORS		4
#else
#define SE_NUM_ACTORS		5
#endif
#define SE_NUM_STREAMS      (SE_NUM_ACTORS + 2)

#define ACTOR_FLEETCOMMAND  0

#define MAX_CHATTERS        2

#define CARDIOD_POINTS		180

#define PI_UNDER_180  57.29577f

#define SOUND_NOTINITED             -1
#define SOUND_SHIP_EXPLODED         -2


/*=============================================================================
    Tweaks
=============================================================================*/

real32 SFX_VOL_FACTOR;
sdword SFX_MAX_ENGINES;
sdword SFX_MIN_CAPSHIPS;
sdword SFX_AMBIENT_VOLUME;
sdword SFX_NIS_MAX_ENGINES;
sdword SFX_NIS_MIN_CAPSHIPS;
real32 SFX_MAX_ENGINE_RANGE;
bool   SFX_CAPSHIPS_ALWAYS_ON;
sdword SFX_MAX_AMBIENT;
real32 SFX_FLOAT_VELOCITY;
real32 SFX_NIS_FLOAT_VELOCITY;
real32 SFX_MIN_PERCEPTABLE_VOL;

sword  SFX_HYPERSPACE_VOLUME;

real32 SFX_DAMAGERATIO_LIGHT;
real32 SFX_DAMAGERATIO_MEDIUM;
real32 SFX_DAMAGERATIO_HEAVY;
bool   SFX_DAMAGERATIO_ENABLE;

real32 SFX_CARDIOD_FACTOR;
real32 SFX_CARDIOD_MIN;

real32 FIGHTER_VELOCITY_LOWPITCH;
real32 FIGHTER_VELOCITY_HIGHPITCH;
real32 FIGHTER_VELOCITY_SCALE;

real32 CORVETTE_VELOCITY_LOWPITCH;
real32 CORVETTE_VELOCITY_HIGHPITCH;
real32 CORVETTE_VELOCITY_SCALE;

real32 FIGHTER_DOPPLER_SCALE;
sdword FIGHTER_DOPPLER_LOW;
sdword FIGHTER_DOPPLER_HIGH;
bool   FIGHTER_DOPPLER_USEVELOCITY;

real32 CORVETTE_DOPPLER_SCALE;
sdword CORVETTE_DOPPLER_LOW;
sdword CORVETTE_DOPPLER_HIGH;
bool   CORVETTE_DOPPLER_USEVELOCITY;

real32 SPEECH_VOL_FACTOR;
sword  SPEECH_VOL_LOW;
sword  SPEECH_VOL_MAX;
real32 SPEECH_NOISE_FACTOR;
real32 SPEECH_NOISE_LOW;
real32 SPEECH_NOISE_HIGH;
udword SPEECH_FILTER_LOW;
udword SPEECH_FILTER_HIGH;
real32 SPEECH_BREAK_THRESHOLD;
real32 SPEECH_BREAK_RATE_FACTOR;
udword SPEECH_BREAK_RATE_LOW;
udword SPEECH_BREAK_RATE_HIGH;
real32 SPEECH_BREAK_LENGTH_FACTOR;
udword SPEECH_BREAK_LENGTH_LOW;
udword SPEECH_BREAK_LENGTH_HIGH;
real32 SPEECH_CAPSHIP_CHATTER_RANGE;
sdword SPEECH_CAPSHIP_CHATTER_TIME;
real32 SPEECH_COMBAT_CHATTER_RANGE;
sdword SPEECH_COMBAT_CHATTER_TIME;
real32 SPEECH_DISOBEY_FORCEDATTACK;

real32 SPEECH_MIN_PERCEPTABLE_VOL;
real32 SPEECH_AMBIENT_LEVEL;
bool   SPEECH_AMBIENT_ENABLE;

real32 SPEECH_STIKEDAMAGE_MULT;
real32 SPEECH_CAPDAMAGE_MULT;

real32 SPEECH_SINGLEPLAYER_RATIO;
real32 SPEECH_STATUS_RATIO;
real32 SPEECH_CHATTER_RATIO;

real32 MUSIC_DISTANCE_SILENT;
real32 MUSIC_DISTANCE_MAX;
real32 MUSIC_MAXGAME_VOL;
real32 MUSIC_MINACTIVE_VOL;
real32 MUSIC_MININACTIVE_VOL;
real32 MUSIC_SENSORS_VOL;
real32 MUSIC_MANAGERS_VOL;
real32 MUSIC_TUTORIAL_VOL;
real32 MUSIC_FADE_TIME;
real32 MUSIC_MAXBATTLE_VOL;
real32 MUSIC_MINBATTLE_VOL;

real32 RANDOM_AMBIENCE_MINFREQ;
sdword RANDOM_AMBIENCE_ADDRANDOM;


/*=============================================================================
    Functions
=============================================================================*/
void SEupdateShipRange(void);

sword SEcalcvolumeold(double distance);
sword SEcalcvolume(real32 distance, real32 maxdist);
sword SEcalcenginevol(sdword shipclass, real32 distance);
sword SEcalcvol(sdword shipclass, real32 distance);
bool SEinrange(sdword shipclass, real32 distance);
bool SEinrangeSqr(sdword shipclass, real32 distancesqr);
sword SEequalize(sdword objtype, real32 distance, real32 *eq);
void SEeq(sdword objtype, real32 distance, real32 *eq);
void SEstopengine(Ship *ship, bool stopnow);
void SEloadbank(void);
sdword SEselectactor(void);
sdword SEspeechevent(sdword stream, sdword actor, sdword event, sdword var, sword vol, sword pan, double dist, float damageratio, sdword setVariation, bool bookend, ShipClass shipclass);
sword SEgetAngleToCamera(Ship *ship);
sdword GetPatch(SFXLUT *lut, sdword object, sdword event);
real32 SEsilence(void);

sword getPanAngle(vector WorldVector, real32 objsize, real32 distance);	/* Returns an angle to the object from the camera: 0 ahead, 90 right, -90 left, 179/-179 behind */

void SEstopsoundhandle(sdword *shandle, real32 fadetime);

sdword speechEventInit(void);
void speechEventClose(void);
sdword speechEventCleanup(void);

sdword musicEventPlay(sdword tracknum);
sdword musicEventUpdateVolume(void);
sdword musicEventStop(sdword tracknum, real32 fadetime);


/* SFX lookup tables */
extern SFXLUT  *MiscLUT;
extern SFXLUT  *R1GunsLUT;
extern SFXLUT  *R1ShipsLUT;
extern SFXLUT  *R2GunsLUT;
extern SFXLUT  *R2ShipsLUT;
extern SFXLUT  *SpecialLUT;
/* new */
extern SFXLUT	*GunEventsLUT;
extern SFXLUT	*ShipCmnEventsLUT;
extern SFXLUT	*ShipEventsLUT;
extern SFXLUT	*SpecEffectEventsLUT;
extern SFXLUT	*SpecExpEventsLUT;
extern SFXLUT	*SpecHitEventsLUT;
extern SFXLUT	*UIEventsLUT;

/* new banks */
extern ubyte	*GunBank;
extern ubyte	*ShipBank;
extern ubyte	*SpecialEffectBank;
extern ubyte	*UIBank;


extern sdword  ambienthandle;

#endif