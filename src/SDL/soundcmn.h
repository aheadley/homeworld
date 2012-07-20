#ifndef ___SOUNDCMN_H
#define ___SOUNDCMN_H

#include <SDL.h>

#include "fqcodec.h"
#include "fqeffect.h"
#include "Types.h"
#include "soundlow.h"
#include "File.h"

#define NUM_FADE_BLOCKS		20

#define DELAY_BUF_SIZE		(18 * FQ_SIZE) // approx 200 msec delay buffer
#define SND_BLOCK_TIME      ((float)FQ_SLICE / 1000.0F) // approx 11.6 msec (in secs)

#define VCE_BACKWARDS_COMPATIBLE    1   //backwards compatible with older version with the "DATA" before the "INFO"

/* structures */

/* Basic replacement for WAVEFORMATEX. */
typedef struct
{
	Uint16 format;
        Uint16 channels;
        Uint32 frequency;
        Uint32 avgBytesPerSecond;
        Uint16 blockAlign;
/*        Unit16 bitsPerSample; */
} SDLWAVEFORMAT;

typedef struct
{
	udword			id;
	udword			priority;
	sdword			pitch;
	sdword			dataoffset;
	sdword			datasize;
	sdword			loopstart;
	sdword			loopend;
	
	sword			bitrate;
	sword			flags;
	
	ubyte			volume;
	sbyte			pan;
	sbyte			pad[2];

	SDLWAVEFORMAT	waveformat;
	sword			wavepad;
} PATCH;

typedef struct
{
	udword			id;
	udword			checksum;
	sdword			numpatches;
	sdword			firstpatch;
} BANK;

typedef struct
{
	sdword			priority;
	sdword			status;
	sdword			handle;

	bool			mute;

	PATCH			*ppatch;
	
	real32			volume;
	real32			volfade;
	
	sword			voltarget;
	sword			volticksleft;
	
	sword			pan;
	sword			pantarget;
	sword			panfade;
	sword			panticksleft;

	real32			pitch;
	real32			pitchtarget;
	real32			pitchfade;
	sdword			pitchticksleft;
	
	sword			heading;
	sword			bitrate;

	sdword			fqsize;
	sdword			looping;

	sdword			numchannels;	//new

	sbyte			*freqdata;
	sbyte			*currentpos;
	sbyte			*endpos;
	
	sdword			amountread;

	real32			shift;
	
	real32			volfactorL;
	real32			volfactorR;

	ubyte			exponentblockL[FQ_SIZE];
	ubyte			exponentblockR[FQ_SIZE];

	real32			mixbuffer1[FQ_SIZE];
	real32			mixbuffer2[FQ_SIZE];

	real32			mixbuffer1R[FQ_SIZE];
	real32			mixbuffer2R[FQ_SIZE];

	real32			filter[SOUND_EQ_SIZE];
	real32			cardiodfilter[SOUND_EQ_SIZE];
	bool			usecardiod;
} CHANNEL;		/* 80 bytes + mixbuffers */

typedef struct
{
	filehandle		fhandle;	/* this is the stream file handle */
	sdword			offset;		/* this is the offset within that file */
	sdword			flags;		/* looping? */
	sdword			size;
	
	sword			vol;
	sword			pan;
	
	sword			numchannels;
	sword			bitrate;
	
	real32			fadetime;
	real32			volfactorL;
	real32			volfactorR;

	real32			silencetime;

	sbyte			*nextplay;

	PATCH			*pmixPatch;
	sdword			mixHandle;
	sword			mixLevel;
	sword			pad;

	STREAMDELAY		*delay;
	STREAMEQ		*eq;
	EFFECT			*effect;

    sdword          actornum;   //needed for subtitles
    sdword          speechEvent;//ditto
} STREAMQUEUE;

typedef struct
{
	sdword			ID;			/* this is the ID to check that this is a valid stream */
	sdword			size;		/* this is the size of this stream */
} STREAMHEADER;

typedef struct
{
	STREAMHEADER	header;

	ubyte			*buffer;	/* this is a pointer to the buffer to read data from the stream file */
	sdword			buffersize;	/* this is the size of the stream buffer / SOUND_STREAM_NUM_BUFFERS */
	sdword			blocksize;	/* this is the size of the stream buffer / SOUND_STREAM_NUM_BUFFERS */

	ubyte			*writepos;
	ubyte			*bufferend;

	sdword			blockstatus[2];

//	sdword			bufferstatus[SOUND_STREAM_NUM_BUFFERS];
//	sdword			readbuffer;	/* this is which buffer is being read to the DirectSound buffer */
//	sdword			writebuffer;/* this is which buffer is being written into from the CD */
	sdword			readblock;	/* this is which buffer is being read to the DirectSound buffer */
	sdword			writeblock;	/* this is which buffer is being written into from the CD */

	sdword			dataleft;	/* amount of data left to read in this stream */
	sdword			lastpos;

//	sdword			writepos;	/* location in the DirectSound buffer to write at */

	sdword			status;

	sdword			filepos;

	sdword			playing;

	sdword			handle;		/* this is the sound handle that this stream is being played on */
	
	sdword			queueindex;	/* this is which queue to play next */
	sdword			writeindex;	/* this is which queue to play next */
	sdword			numqueued;
	sdword			numtoplay;

	sdword			playindex;

	STREAMQUEUE		queue[SOUND_MAX_STREAM_QUEUE];

	sdword			delaypos1;
	sdword			delaypos2;
	real32			delaybuffer1[DELAY_BUF_SIZE];
	real32			delaybuffer2[DELAY_BUF_SIZE];

    real32          dataPeriod; //inverse of data rate, seconds per byte
} STREAM;

typedef struct
{
	FILE*			fp;	        /* this is the O/S handle to the file */
	sdword			handle;		/* this handle is used for indexing into the streamfiles array */
} STREAMFILE;


typedef struct
{
	sdword status;
	udword timeout;
} SOUNDCOMPONENT;

/* functions */
sdword isoundmixerinit(SDL_AudioSpec *aspec);
void isoundmixerrestore(void);
void isoundstreamupdate(void *dummy);

sdword SNDreleasebuffer(CHANNEL *pchan);
sdword SNDchannel(sdword handle);
void SNDcalcvolpan(CHANNEL *pchan);

sdword SNDcreatehandle(sdword channel);
PATCH *SNDgetpatch(void *bankaddress, sdword patnum);

sdword smixCreateDSoundBuffer(SDLWAVEFORMAT *pcmwf);
sdword smixInitMixBuffer(SDL_AudioSpec *aspec);

sdword streamStartThread(void);

#endif
