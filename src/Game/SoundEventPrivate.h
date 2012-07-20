/*=============================================================================
    Name    : SoundEventPrivate.h
    Purpose : Private Defines for SoundEvent*.c files

    Created 9/8/98 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SOUND_PRIV_H
#define ___SOUND_PRIV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "soundlow.h"
#include "Types.h"
#include "Switches.h"
#include "Memory.h"
#include "File.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "FastMath.h"
#include "Randy.h"
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

#ifdef _WIN32
#define SOUNDFXDIR "SoundFX\\"
#else
#define SOUNDFXDIR "SoundFX/"
#endif

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

extern real32 SFX_VOL_FACTOR;
extern sdword SFX_MAX_ENGINES;
extern sdword SFX_MIN_CAPSHIPS;
extern sdword SFX_AMBIENT_VOLUME;
extern sdword SFX_NIS_MAX_ENGINES;
extern sdword SFX_NIS_MIN_CAPSHIPS;
extern real32 SFX_MAX_ENGINE_RANGE;
extern bool   SFX_CAPSHIPS_ALWAYS_ON;
extern sdword SFX_MAX_AMBIENT;
extern real32 SFX_FLOAT_VELOCITY;
extern real32 SFX_NIS_FLOAT_VELOCITY;
extern real32 SFX_MIN_PERCEPTABLE_VOL;

extern sword  SFX_HYPERSPACE_VOLUME;

extern real32 SFX_DAMAGERATIO_LIGHT;
extern real32 SFX_DAMAGERATIO_MEDIUM;
extern real32 SFX_DAMAGERATIO_HEAVY;
extern bool   SFX_DAMAGERATIO_ENABLE;

extern real32 SFX_CARDIOD_FACTOR;
extern real32 SFX_CARDIOD_MIN;

extern real32 FIGHTER_VELOCITY_LOWPITCH;
extern real32 FIGHTER_VELOCITY_HIGHPITCH;
extern real32 FIGHTER_VELOCITY_SCALE;

extern real32 CORVETTE_VELOCITY_LOWPITCH;
extern real32 CORVETTE_VELOCITY_HIGHPITCH;
extern real32 CORVETTE_VELOCITY_SCALE;

extern real32 FIGHTER_DOPPLER_SCALE;
extern sdword FIGHTER_DOPPLER_LOW;
extern sdword FIGHTER_DOPPLER_HIGH;
extern bool   FIGHTER_DOPPLER_USEVELOCITY;

extern real32 CORVETTE_DOPPLER_SCALE;
extern sdword CORVETTE_DOPPLER_LOW;
extern sdword CORVETTE_DOPPLER_HIGH;
extern bool   CORVETTE_DOPPLER_USEVELOCITY;

extern real32 SPEECH_VOL_FACTOR;
extern sword  SPEECH_VOL_LOW;
extern sword  SPEECH_VOL_MAX;
extern real32 SPEECH_NOISE_FACTOR;
extern real32 SPEECH_NOISE_LOW;
extern real32 SPEECH_NOISE_HIGH;
extern udword SPEECH_FILTER_LOW;
extern udword SPEECH_FILTER_HIGH;
extern real32 SPEECH_BREAK_THRESHOLD;
extern real32 SPEECH_BREAK_RATE_FACTOR;
extern udword SPEECH_BREAK_RATE_LOW;
extern udword SPEECH_BREAK_RATE_HIGH;
extern real32 SPEECH_BREAK_LENGTH_FACTOR;
extern udword SPEECH_BREAK_LENGTH_LOW;
extern udword SPEECH_BREAK_LENGTH_HIGH;
extern real32 SPEECH_CAPSHIP_CHATTER_RANGE;
extern sdword SPEECH_CAPSHIP_CHATTER_TIME;
extern real32 SPEECH_COMBAT_CHATTER_RANGE;
extern sdword SPEECH_COMBAT_CHATTER_TIME;
extern real32 SPEECH_DISOBEY_FORCEDATTACK;

extern real32 SPEECH_MIN_PERCEPTABLE_VOL;
extern real32 SPEECH_AMBIENT_LEVEL;
extern bool   SPEECH_AMBIENT_ENABLE;

extern real32 SPEECH_STIKEDAMAGE_MULT;
extern real32 SPEECH_CAPDAMAGE_MULT;

extern real32 SPEECH_SINGLEPLAYER_RATIO;
extern real32 SPEECH_STATUS_RATIO;
extern real32 SPEECH_CHATTER_RATIO;

extern real32 MUSIC_DISTANCE_SILENT;
extern real32 MUSIC_DISTANCE_MAX;
extern real32 MUSIC_MAXGAME_VOL;
extern real32 MUSIC_MINACTIVE_VOL;
extern real32 MUSIC_MININACTIVE_VOL;
extern real32 MUSIC_SENSORS_VOL;
extern real32 MUSIC_MANAGERS_VOL;
extern real32 MUSIC_TUTORIAL_VOL;
extern real32 MUSIC_FADE_TIME;
extern real32 MUSIC_MAXBATTLE_VOL;
extern real32 MUSIC_MINBATTLE_VOL;

extern real32 RANDOM_AMBIENCE_MINFREQ;
extern sdword RANDOM_AMBIENCE_ADDRANDOM;


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