/*=============================================================================
    Name    : Soundlow.h
    Purpose : Defines for SoundLow.c

    Created 7/24/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SOUNDLOW_H
#define ___SOUNDLOW_H

#include "Types.h"
#include "fqcodec.h"
#include "fqeffect.h"

#define SND_SYNC            0x0000  /* play synchronously (default) */
#define SND_ASYNC           0x0001  /* play asynchronously */
#define SND_NODEFAULT       0x0002  /* silence (!default) if sound not found */
#define SND_MEMORY          0x0004  /* pszSound points to a memory file */
#define SND_LOOP            0x0008  /* loop the sound until next sndPlaySound */
#define SND_NOSTOP          0x0010  /* don't stop any currently playing sound */

#define SND_NOWAIT	0x00002000L /* don't wait if the driver is busy */
#define SND_ALIAS       0x00010000L /* name is a registry alias */
#define SND_ALIAS_ID	0x00110000L /* alias is a predefined ID */

#define SOUND_MAX_VOICES	32		// Maximum number of voices
#define SOUND_DEF_VOICES	16		// Default number of voices
#define SOUND_MIN_VOICES	8		// Minimum number of voices

#define SOUND_MODE_NORM		0
#define SOUND_MODE_AUTO		1
#define SOUND_MODE_LOW		2

#define SOUND_MAX_PATCHES	128
#define SOUND_MAX_STREAM_BUFFERS	7
#define SOUND_MAX_STREAM_FILES		1
#define SOUND_MAX_STREAM_QUEUE		10

#define SOUND_EQ_SIZE	 	(FQ_EQNUM - 1)

#define SOUND_FADE_TIMETOBLOCKS		(1000.0f / (float)FQ_SLICE)

#define SOUND_FADE_STOPTIME			0.5f		// 1/2 a second to fade when stopping a sound
#define SOUND_FADE_STOPALL			1.0f		// 1 second to fade when stopping all sound
#define SOUND_FADE_STOPNOW			0.0f
#define SOUND_FADE_MIXER			(udword)(1.2f * SOUND_FADE_TIMETOBLOCKS)// (udword)(1.0f * SOUND_FADE_TIMETOBLOCKS)

#define SOUND_ERR		-1
#define SOUND_OK		0
#define SOUND_DEFAULT	-1
#define SOUND_VOLFACTOR	-39

#define SOUND_FLAGS_LOOPING			0x1
#define SOUND_FLAGS_STRETCH			0x2
#define SOUND_FLAGS_QUEUESTREAM		0x4
#define SOUND_FLAGS_QUEUESILENCE	0x8
#define SOUND_FLAGS_QUEUEPATCH		0x10

#define SOUND_FLAGS_PATCHPOINTER	-2

#define SOUND_MONO				1
#define SOUND_STEREO			2

#define SOUND_PAN_CENTER		0
#define SOUND_PAN_LEFT			-90
#define SOUND_PAN_RIGHT			90
#define SOUND_PAN_BEHIND		179
#define SOUND_PAN_MAX			179
#define SOUND_PAN_MIN			-179

#define SOUND_PRIORITY_STREAM	101
#define SOUND_PRIORITY_MAX		100
#define SOUND_PRIORITY_HIGH		75
#define SOUND_PRIORITY_NORMAL	50
#define SOUND_PRIORITY_LOW		25
#define SOUND_PRIORITY_MIN		-1

#define SOUND_VOL_MAX			255
#define SOUND_VOL_MID			127
#define SOUND_VOL_MIN			0
#define SOUND_VOL_AUTOSTOP		-1

#define SOUND_STREAM_BUFFER_SIZE	(8 * 1024)		// this seems to be some magic number, acts poorly if increased to 16
#define SOUND_STREAM_DSBUFFER_SIZE	(SOUND_STREAM_BUFFER_SIZE * 2)
#define SOUND_STREAM_SLEEP			0L

#define SOUND_STREAM_FREE		0
#define SOUND_STREAM_INUSE		1
#define SOUND_STREAM_STARTING	2
#define SOUND_STREAM_WRITING	3
#define SOUND_STREAM_PLAYING	4
#define SOUND_STREAM_WAITING	5
#define SOUND_STREAM_SILENCE	6

#define ID_STREAM_DATA			0x41544144
#define ID_STREAM_INFO			0x4f464e49

#define SOUND_MIXER_CUSTOM		0
#define SOUND_MIXER_DSOUND		1
#define SOUND_MIXER_WAVEOUT		2

#define SOUND_FREE				0
#define SOUND_STOPPED			1
#define SOUND_INUSE				1
#define SOUND_STARTING			2
#define SOUND_WRITING			3
#define SOUND_PLAYING			4
#define SOUND_WAITING			5
#define SOUND_SILENCE			6
#define SOUND_STOPPING			7
#define SOUND_FADING			8
#define SOUND_LOOPEND			9
#define SOUND_RESTART			10
#define SOUND_PAUSED			11

#define STREAM_FLAGS_NOEFFECT	0
#define STREAM_FLAGS_DELAY		0x1
#define STREAM_FLAGS_ACMODEL	0x2
#define STREAM_FLAGS_EQ			0x4


/* structures */
typedef struct
{
	sdword			flags;
	real32			level;
	udword			duration;
	real32			eq[SOUND_EQ_SIZE];
} STREAMDELAY;

typedef struct
{
	sdword			flags;
	real32			eq[SOUND_EQ_SIZE];
} STREAMEQ;


typedef void (*streamprintfunction)(char *pszInformation);

// channel functions
void soundGetVoiceLimits(sdword *min,sdword *max);
void soundGetNumVoices(sdword *num,sdword *mode);	// mode SOUND_MODE_NORM or SOUND_MODE_AUTO
void soundSetNumVoices(sdword num,sdword mode);		// mode SOUND_MODE_NORM or SOUND_MODE_AUTO

// DCT mode functions (mixer.c)
void soundMixerGetMode(sdword *mode);	// mode SOUND_MODE_NORM or SOUND_MODE_AUTO or SOUND_MODE_LOW
void soundMixerSetMode(sdword mode);	// mode SOUND_MODE_NORM or SOUND_MODE_AUTO or SOUND_MODE_LOW

// Panic mode functions
void soundPanic(void);
void soundPanicReset(void);	// mixer.c

/* functions */
//sdword soundinit(HWND hWnd, sdword mode);
sdword soundinit(bool mode);
//sdword soundreinit(HWND hWnd);
sdword soundreinit();
void soundrestore(void);
void soundclose(void);
void soundupdate(void);

void soundpause(bool bPause);
void sounddeactivate(bool bDeactivate);

sword soundloadpatch(char *pszFileName, sword looped);
udword soundbankadd(void *bankaddress);

sdword soundplayFPRVL(sword patnum, real32 freq, sword pan, sdword priority, sword vol, bool startatloop);

sdword soundvolumeF(sdword handle, sword vol, real32 fadetime);
#define soundvolume(a, b)		soundvolumeF(a, b, 0)

sdword soundpanF(sdword handle, sword pan, real32 fadetime);
#define soundpan(a, b)			soundpanF(a, b, 0)

sdword soundfrequency(sdword handle, real32 freq);
sdword soundequalize(sdword handle, real32 *eq);

void soundstopallSFX(real32 fadetime, bool stopStreams);
#define soundstopall(a)		soundstopallSFX(a, TRUE)
sdword soundstop(sdword handle, real32 fade);

sdword soundrestart(sdword handle);

sdword soundshipheading(sdword handle, sword heading, sdword highband, sdword lowband, real32 velfactor, real32 shipfactor);

bool soundover(sdword handle);

void soundSetMasterEQ(sdword startband, sdword endband, bool increment);
void soundResetMasterEQ(void);


udword soundgettick(void);


#define soundplay(a)			soundplayFPRVL(a, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_DEFAULT, SOUND_DEFAULT, FALSE)
#define soundplayV(a, b)		soundplayFPRVL(a, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_DEFAULT, b, FALSE)
#define soundplayP(a, b)		soundplayFPRVL(a, (real32)SOUND_DEFAULT, b, SOUND_DEFAULT, SOUND_DEFAULT, FALSE)
#define soundplayF(a, b)		soundplayFPRVL(a, b, SOUND_PAN_CENTER, SOUND_DEFAULT, SOUND_DEFAULT, FALSE)
#define soundplayPV(a, b, c)	soundplayFPRVL(a, (real32)SOUND_DEFAULT, b, SOUND_DEFAULT, c, FALSE)
#define soundplayPRV(a, b, c, d)	soundplayFPRVL(a, (real32)SOUND_DEFAULT, b, c, d, FALSE)
#define soundplayFPRV(a, b, c, d, e)	soundplayFPRVL(a, b, c, d, e, FALSE)
#define soundplayPRVL(a, b, c, d, e)	soundplayFPRVL(a, (real32)SOUND_DEFAULT, b, c, d, e)


sdword splayFPRVL(void *bankaddress, sdword patnum, real32 *eq, real32 freq, sword pan, sdword priority, sword vol, bool startatloop, bool fadein, bool mute);

#define splay(f, a)			splayFPRVL(f, a, NULL, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_DEFAULT, SOUND_DEFAULT, FALSE, TRUE, FALSE)
#define splayV(f, a, b)		splayFPRVL(f, a, NULL, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_DEFAULT, b, FALSE, TRUE, FALSE)
#define splayP(f, a, b)		splayFPRVL(f, a, NULL, (real32)SOUND_DEFAULT, b, SOUND_DEFAULT, SOUND_DEFAULT, FALSE, TRUE, FALSE)
#define splayF(f, a, b)		splayFPRVL(f, a, NULL, b, SOUND_PAN_CENTER, SOUND_DEFAULT, SOUND_DEFAULT, FALSE, TRUE, FALSE)
#define splayPV(f, a, b, c)	splayFPRVL(f, a, NULL, (real32)SOUND_DEFAULT, b, SOUND_DEFAULT, c, FALSE, TRUE, FALSE)
#define splayPRV(f, a, b, c, d)	splayFPRVL(f, a, NULL, (real32)SOUND_DEFAULT, b, c, d, FALSE, TRUE, FALSE)
#define splayEPRV(f, a, q, b, c, d)	splayFPRVL(f, a, q, (real32)SOUND_DEFAULT, b, c, d, FALSE, TRUE, FALSE)
#define splayFPRV(f, a, b, c, d, e)	splayFPRVL(f, a, NULL, b, c, d, e, FALSE, TRUE, FALSE)
#define splayPRVL(f, a, b, c, d, e)	splayFPRVL(f, a, NULL, (real32)SOUND_DEFAULT, b, c, d, e, TRUE, FALSE)
#define splayEPRVL(f, a, q, b, c, d, e)	splayFPRVL(f, a, q, (real32)SOUND_DEFAULT, b, c, d, e, TRUE, FALSE)
#define splayNOFADE(f, a)	splayFPRVL(f, a, NULL, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_DEFAULT, SOUND_DEFAULT, FALSE, FALSE, FALSE)
#define splayMUTE(f, a, b, c, d)	splayFPRVL(f, a, NULL, (real32)SOUND_DEFAULT, b, c, d, FALSE, TRUE, TRUE)

void soundstreamquery(sdword maxstreams, sdword *pbuffersize, sdword *pstreamersize);
sdword soundstreaminit(void *pstreamer, sdword size, sdword numstreams, streamprintfunction printfunction);
udword soundstreamopenfile(char *pszStreamFile, sdword *handle);
sdword soundstreamcreatebuffer(void *pstreambuffer, sdword size, uword bitrate);

sdword soundstreamqueuePatch(sdword streamhandle, sdword filehandle, sdword offset, udword flags, sword vol, sword pan, sword numchannels, sword bitrate, EFFECT *peffect, STREAMEQ *pEQ, STREAMDELAY *pdelay, void *pmixpatch, sdword level, real32 silence, real32 fadetime, sdword actornum, sdword speechEvent, bool bWait);
#define soundstreamqueue(a, b, c, d, e, f, g, h, i, j, k, l)	soundstreamqueuePatch(a, b, c, d, e, f, g, h, i, j, k, NULL, SOUND_VOL_MIN, 0.0, 0.0, -1, l, FALSE)
#define soundstreamqueuefade(a, b, c, d, e, f, g, h, i, j, k, l)	soundstreamqueuePatch(a, b, c, d, e, f, g, h, i, j, k, NULL, SOUND_VOL_MIN, 0.0, l, -1, -1, FALSE)
#define soundstreamqueueSilence(a, b, c, d, e, f, g, h, i, j)	soundstreamqueuePatch(a, SOUND_DEFAULT, SOUND_DEFAULT, b, c, d, e, f, g, h, i, NULL, SOUND_DEFAULT, j, 0.0, -1, -1, FALSE)
#define soundstreamqueuewait(a, b, c, d, e, f, g, h, i, j, k, l)	soundstreamqueuePatch(a, b, c, d, e, f, g, h, i, j, k, NULL, SOUND_VOL_MIN, 0.0, 0.0, -1, l, TRUE)

sdword soundstreamrestorebuffer(sdword streamhandle);
sdword soundstreamclosefile(sdword filehandle);
sdword soundstreamrestore(void);
sdword soundstreamnumqueued(sdword streamhandle);
void soundstreamstopall(real32 fadetime);
sdword soundstreamover(sdword streamhandle);
sdword soundstreamvolume(sdword handle, sword vol, real32 fadetime);
sword soundstreamgetvol(sdword handle);
real32 soundusage(void);
sdword soundstreamfading(sdword streamhandle);

#endif

