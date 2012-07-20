/*=============================================================================
    Name    : SoundLow.c
    Purpose : Low level sound routines

    Created 7/24/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.

    CUT CUT CUT CUT CUT OMG WTF HAVE I DONE!?!#@
=============================================================================*/

#include <string.h>
#include "Switches.h"
#include "Debug.h"
#include "soundlow.h"
#include "File.h"
#include "soundcmn.h"
#include "main.h"

#define SOUNDFXDIR "SoundFX\\"

#define EQ_STEP			0.1


typedef struct
{
	void *start;
	void *end;
} BANKPOINTERS;

/* function in speechevent.c that needs to be called when shutting down */
void musicEventUpdateVolume(void);

/* internal functions */
sdword SNDgetchannel(sword patchnum, sdword priority);


/* variables */
CHANNEL channels[SOUND_MAX_VOICES];
sword numpatches = 0;
sdword	lasthandle = 0;
bool soundinited = FALSE;
BANK *bank;
PATCH *patches;
sdword channelsinuse = 0;
bool bDirectSoundCertified = FALSE;
real32 masterEQ[FQ_SIZE];
bool bSoundPaused = FALSE;
bool bSoundDeactivated = FALSE;

sdword numbanks = 0;
sdword numchans[4] = {0,0,0,0};
BANKPOINTERS bankpointers[4];

SOUNDCOMPONENT	mixer;
SOUNDCOMPONENT	streamer;

sdword soundnumvoices=SOUND_DEF_VOICES;

sdword soundvoicemode=SOUND_MODE_NORM;	// voice panic mode, normal by default

//streamprintfunction	debugfunction = NULL;
//char debugtext[256];

extern real32 cardiod[];
extern udword mixerticks;

// Get the min and max number of voices
void soundGetVoiceLimits(sdword *min,sdword *max)
{
	*min=SOUND_MIN_VOICES;
	*max=SOUND_MAX_VOICES;
	return;
}

// Get the current number of voices and mode
void soundGetNumVoices(sdword *num,sdword *mode)
{
	*num=soundnumvoices;
	*mode=soundvoicemode;

	return;
}

// Set the current number of voices and mode
void soundSetNumVoices(sdword num,sdword mode)
{
	if(num < SOUND_MIN_VOICES) num=SOUND_MIN_VOICES;
	if(num > SOUND_MAX_VOICES) num=SOUND_MAX_VOICES;
	soundnumvoices=num;

	soundvoicemode=mode;
	return;
}

// Shut down necessary channels for panic
void soundPanic(void)
{
	CHANNEL *pchan;
	sdword	i;
	sdword	lowchannel;
	real32	lowvolume;
	sdword	lowticks;
	sdword	lowpriority;

	while(channelsinuse > soundnumvoices)
	{
		lowpriority = SOUND_PRIORITY_LOW;
		lowticks = 255;
		lowvolume = (real32)SOUND_VOL_MAX;
		lowchannel = SOUND_DEFAULT;

		/* find the channel to steal */
		for (i = 0; i < SOUND_MAX_VOICES; i++)
		{
			pchan = &channels[i];
			/* don't want one that is already stopping */
			if (pchan->status == SOUND_PLAYING)
			{
				if (pchan->priority < lowpriority)
				{
					if (pchan->volume <= lowvolume)
					{
						if (pchan->volticksleft <= lowticks)
						{
							lowpriority = pchan->priority;
							lowticks = pchan->volticksleft;
							lowvolume = pchan->volume;
							lowchannel = i;
						}
					}
				}
			}
		}

		if (lowchannel > SOUND_DEFAULT)
		{
			/* stop the sound with the lowest priority */
			soundstop(channels[lowchannel].handle, SOUND_FADE_STOPNOW);
		}
		else
		{
			break;
		}
	}

	return;
}

// Called by main.c on before and after[Alt]-[Tab]
void sounddeactivate(bool bDeactivate)
{
	/* set flag */
	if (soundinited)
	{
		bSoundDeactivated=bDeactivate;
	}

	/* reset panic mode */
	soundPanicReset(); // mixer.c
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
real32 soundusage(void)
{
	return ((real32)channelsinuse / SOUND_MAX_VOICES);
}


extern void soundfeedercb(void *userdata, Uint8 *stream, int len);

/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundinit(bool mode)
				{
#ifdef _MACOSX_FIX_ME
	return SOUND_ERR;
#endif

	SDL_AudioSpec aspec;
	sdword i, result;

	// clean up the channels
	for (i = 0; i < soundnumvoices; i++)
	{
		channels[i].status = SOUND_FREE;
	}

	// clean up the masterEQ
	for (i = 0; i < FQ_SIZE; i++)
	{
		masterEQ[i] = 1.0;
	}

	mixer.status = SOUND_FREE;
	mixer.timeout = 0;
	streamer.status = SOUND_FREE;
	streamer.timeout = 0;

		useWaveout = TRUE;
		useDSound = FALSE;
		coopDSound = FALSE;
		bDirectSoundCertified = FALSE;

	// Set up wave format structure.
	aspec.freq = FQ_RATE;
	aspec.format = AUDIO_S16;
	aspec.channels = 2;
	aspec.samples = FQ_SIZE;
	aspec.callback = soundfeedercb;
	aspec.userdata = NULL;

	if (SDL_OpenAudio(&aspec, NULL) < 0) {
	    dbgMessagef("Couldn't open audio: %s\n", SDL_GetError());
	    result = SOUND_ERR;
	}
	else if (isoundmixerinit(&aspec) != SOUND_OK) {
	    dbgMessagef("Unable to init mixer subsystem\n");
	    result = SOUND_ERR;
	} else {
	soundinited = TRUE;
	    result = SOUND_OK;
	}
	return result;
}


sdword soundreinit()
	{
		// couldn't init DSound so use Waveout instead
		useWaveout = TRUE;
		useDSound = FALSE;
		coopDSound = FALSE;
		bDirectSoundCertified = FALSE;
	
	return SOUND_ERR;
}


void soundclose(void)
{
	bSoundPaused = TRUE;
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
void soundrestore(void)
{
	soundpause(TRUE);

	soundinited = FALSE;
	
	while (!((streamer.status == SOUND_FREE) && (mixer.status == SOUND_FREE)))
	{
		SDL_Delay(0);
	}

	isoundmixerrestore();

	return;
}


void soundpause(bool bPause)
{
	if (soundinited)
	{
		if (bPause)
		{
			mixer.timeout = mixerticks + SOUND_FADE_MIXER;
			streamer.timeout = mixerticks + SOUND_FADE_MIXER;
		}
		
		bSoundPaused = bPause;

		if (bPause)
		{
			soundstopall(SOUND_FADE_STOPALL);

			while (!(mixer.status == SOUND_STOPPED))
			{
				musicEventUpdateVolume();
				SDL_Delay(0);
			}

		}
	}
}

void soundstopallSFX(real32 fadetime, bool stopStreams)
{
	sdword i;

	for (i = 0; i < soundnumvoices; i++)
	{
		if (channels[i].handle > SOUND_DEFAULT)
		{
			soundstop(channels[i].handle, fadetime);
		}
	}
	
	if (stopStreams)
	{
		soundstreamstopall(fadetime);
	}
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sword soundloadpatch(char *pszFileName, sword looped)
{
	sword		nRetVal = SOUND_ERR;
#if 0
	HMMIO		hmmioIn;
	MMCKINFO	ckInRiff;
	MMCKINFO	ckIn;
	UINT		cbActualRead;
	PATCH		*newpatch;
	int			ret;
	DWORD		samples;

	if (!soundinited)
	{
		return (nRetVal);
	}

	newpatch = &patches[numpatches];

	newpatch->pbdata = NULL;
	newpatch->pwfx = NULL;
	newpatch->cbSize = 0;
	newpatch->looped = looped;

	if ((ret = WaveLoadFile(pszFileName, &newpatch->cbSize, &samples, &newpatch->pwfx, &newpatch->pbdata)) != 0)
	{
		dbgMessagef("\nWaveLoadFile failed %d", ret);
		dbgMessagef("  File: %s", pszFileName);
	}
	else
		nRetVal = numpatches++;
	
	return (nRetVal);

	
	if ((ret = WaveOpenFile(pszFileName, &hmmioIn, &newpatch->pwfx, &ckInRiff)) != 0)
	{
		dbgMessagef("\nWaveOpenFile failed %d", ret);
		goto ERROR_LOADING;
	}

	if (WaveStartDataRead(&hmmioIn, &ckIn, &ckInRiff) != 0)
	{
		dbgMessagef("\nWaveStartDataRead failed");
		goto ERROR_LOADING;
	}

	// Ok, size of wave data is in ckIn, allocate that buffer.
	if ((newpatch->pbdata = (BYTE *)GlobalAlloc(GMEM_FIXED, ckIn.cksize)) == NULL)
	{
		dbgMessagef("\nglobalalloc failed");
		goto ERROR_LOADING;
	}

	if (WaveReadFile(hmmioIn, ckIn.cksize, newpatch->pbdata, &ckIn, &cbActualRead) != 0)
	{
		dbgMessagef("\nWaveReadFile failed");
		goto ERROR_LOADING;
	}
	
	newpatch->cbSize = cbActualRead;
	nRetVal = numpatches++;
	
	goto DONE_LOADING;

ERROR_LOADING:
	if (newpatch->pbdata != NULL)
	{
		GlobalFree(newpatch->pbdata);
		newpatch->pbdata = NULL;
	}
	if (newpatch->pwfx != NULL)
	{
		GlobalFree(newpatch->pwfx);
		newpatch->pwfx = NULL;
	}
			
DONE_LOADING:
	// Close the wave file.
	if (hmmioIn != NULL)
	{
		mmioClose(hmmioIn, 0);
		hmmioIn = NULL;
	}
#endif
	return(nRetVal);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
udword soundbankadd(void *bankaddress)
{
	sword i;
	sdword numpatches;

	/* point the global bank pointer to the bank header */
	bank = (BANK *)bankaddress;

	/* check bank ID */

	numpatches = bank->numpatches;

	/* point the global patches pointer to the start of the patches */
	patches = (PATCH *)&bank->firstpatch;

	/* figure out where the patch data starts */
	for (i = 0; i < numpatches; i++)
	{
		patches[i].dataoffset = (sdword)bankaddress + patches[i].dataoffset;
		patches[i].loopstart += patches[i].dataoffset;
		patches[i].loopend += patches[i].dataoffset;
		patches[i].datasize += patches[i].dataoffset;
	}

	bankpointers[numbanks].start = bankaddress;
	bankpointers[numbanks++].end = (void *)patches[numpatches - 1].datasize;

	return (bank->checksum);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
bool soundover(sdword handle)
{
	CHANNEL *pchan;

	if (handle < SOUND_OK)
	{
		return (TRUE);
	}

	pchan = &channels[SNDchannel(handle)];

	if (pchan != NULL)
	{
		if (pchan->handle != handle)
		{
			return (TRUE);
		}
	
		if (pchan->status <= SOUND_STOPPED)
		{
			return (TRUE);
		}
		
		return (FALSE);
	}
	
	return (TRUE);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundplayFPRVL(sword patnum, real32 freq, sword pan, sdword priority, sword vol, bool startatloop)
{
	PATCH	*ppatch;
	CHANNEL	*pchan;
	sdword channel;
	sdword handle = SOUND_ERR;
	sdword i;

	return (SOUND_ERR);

	if (!soundinited)
	{
		return (handle);
	}
	
	if (patnum < 0)
	{
		return (handle);
	}

	if (patnum > bank->numpatches)
	{
		return (handle);
	}

	if (vol == SOUND_DEFAULT)
	{
		vol = SOUND_VOL_MAX;
	}

	if ((vol <= SOUND_VOL_MIN) || (vol > SOUND_VOL_MAX))
	{
		return (handle);
	}

	if (freq == SOUND_DEFAULT)
	{
		freq = 1.0f;
	}

#ifdef salfreds
	dbgAssert(patnum >= 0);
#else
	if ((pan < SOUND_PAN_LEFT) || (pan > SOUND_PAN_RIGHT))
	{
		return (handle);
	}
#endif

	ppatch = &patches[patnum];

#if 0
	if (ppatch->dataoffset == NULL)
	{
		return (handle);
	}
#endif

	channel = SNDgetchannel(patnum, priority);

#ifdef salfreds
    dbgAssert(channel >= 0);
#else
	if (channel < SOUND_OK)
	{
		return (handle);
	}
#endif

	/* create handle here */
	handle = SNDcreatehandle(channel);

	pchan = &channels[channel];
	pchan->handle = handle;
	pchan->ppatch = ppatch;
	pchan->volfactorL = (real32)1.0;
	pchan->volfactorR = (real32)1.0;
	pchan->looping = (ppatch->flags & SOUND_FLAGS_LOOPING);
	pchan->pitch = freq;
	pchan->heading = 0;
	pchan->usecardiod = FALSE;
	
	for (i = 0; i < SOUND_EQ_SIZE; i++)
	{
		pchan->filter[i] = 1.0f;
		pchan->cardiodfilter[i] = 1.0f;
	}


	soundvolume(handle, vol);
	
	soundpan(handle, pan);

	SNDcalcvolpan(pchan);

	soundfrequency(handle, freq);
	
// NEWLOOP
	if (startatloop)
	{
		pchan->currentpos = (sbyte *)ppatch->loopstart;
	}
	else
	{
		pchan->currentpos = (sbyte *)ppatch->dataoffset;
	}
	
	if (ppatch->waveformat.frequency < FQ_RATE)
	{
		pchan->fqsize = FQ_QSIZE;
	}
	else
	{
		pchan->fqsize = FQ_HSIZE;
	}

#ifdef DEBUG_SOUNDLOW
	if (DS_OK != hr)
	{
        SNDreleasebuffer(pchan);
		dbgMessagef("\nDirectSound error, could not play patch %d",(udword)patnum);
	}
	else
#endif
		pchan->status = SOUND_PLAYING;

	return (handle);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundstop(sdword handle, real32 fadetime)
{
	CHANNEL *pchan;
	sdword channel;
	sdword fadeblocks = 0;

	if (!soundinited)
	{
		return (SOUND_ERR);
	}
	
	channel = SNDchannel(handle);

	if (channel < SOUND_OK)
	{
		return (SOUND_ERR);
	}
	
	pchan = &channels[channel];

	if (pchan->handle != handle)
	{
		return (SOUND_ERR);
	}

	if (pchan->status == SOUND_FREE)
	{
		return (SOUND_ERR);
	}

	if (pchan != NULL)
	{
		if ((pchan->looping) && (pchan->ppatch->datasize > pchan->ppatch->loopend) && (fadetime > 0.0f))
		{
			pchan->status = SOUND_LOOPEND;
		}
		else
		{
			fadeblocks = (sdword)(fadetime * SOUND_FADE_TIMETOBLOCKS);
			
			if (fadeblocks < NUM_FADE_BLOCKS)
			{
				fadeblocks = NUM_FADE_BLOCKS;
			}
		
			pchan->status = SOUND_STOPPING;
			pchan->voltarget = -1;
			pchan->volticksleft = fadeblocks;
			pchan->volfade = (real32)(pchan->voltarget - pchan->volume) / (real32)pchan->volticksleft;

			if (pchan->volfade == 0.0f)
			{
				pchan->volfade = 0.01f;
				if (pchan->voltarget < pchan->volume)
				{
					pchan->volfade = -0.01f;
				}
			}
		}
		return (SOUND_OK);
	}

	return (SOUND_ERR);
}


/*-----------------------------------------------------------------------------
	Name		: soundrestart
	Description	: If this is a looping sound, it will reset the play pointer
					to the start of the sound.
	Inputs		: handle - the handle to a sound returned by soundplay
	Outputs		:
	Return		: SOUND_OK if successful, SOUND_ERR on error
----------------------------------------------------------------------------*/	
sdword soundrestart(sdword handle)
{
	CHANNEL *pchan;
	sdword channel;

	if (!soundinited)
	{
		channel = SNDchannel(handle);
		if (channel < SOUND_OK)
		{
			pchan = &channels[channel];

			if (pchan != NULL)
			{
				if (pchan->handle != handle)
				{
					return (SOUND_ERR);
				}

				if ((pchan->looping == TRUE) && (pchan->status == SOUND_PLAYING))
				{
					pchan->status = SOUND_RESTART;
					return (SOUND_OK);
				}
			}
		}
	}

	return (SOUND_ERR);
}


/*-----------------------------------------------------------------------------
	Name		: soundvolume
	Description	:
	Inputs		: handle - the handle to a sound returned by soundplay
				  vol - the volume to set this sound to (range of SOUND_MIN_VOL - SOUND_MAX_VOL)
	Outputs		:
	Return		: SOUND_OK if successful, SOUND_ERR on error
----------------------------------------------------------------------------*/	
sdword soundvolumeF(sdword handle, sword vol, real32 fadetime)
{
	CHANNEL *pchan;
	sdword channel;
	sdword fadeblocks = 0;

	if (!soundinited)
	{
		return (SOUND_ERR);
	}
	
	if (vol > SOUND_VOL_MAX)
	{
		vol = SOUND_VOL_MAX;
	}
	else if (vol <= SOUND_VOL_MIN)
	{
		soundstop(handle, TRUE);
		return (SOUND_OK);
	}

	channel = SNDchannel(handle);

	if (channel < SOUND_OK)
	{
		return (SOUND_ERR);
	}

	pchan = &channels[channel];

	if (pchan != NULL)
	{
		if (pchan->handle != handle)
		{
			return (SOUND_ERR);
		}

		if (vol != pchan->voltarget)
		{
			if (vol == (sword)pchan->volume)
			{
				pchan->voltarget = vol;
				pchan->volticksleft = 0;
				pchan->volfade = 0.0f;
			}
			else
			{
				fadeblocks = (sdword)(fadetime * SOUND_FADE_TIMETOBLOCKS);

				if (fadeblocks < NUM_FADE_BLOCKS)
				{
					fadeblocks = NUM_FADE_BLOCKS;
				}

				pchan->voltarget = vol;
				pchan->volticksleft = fadeblocks;
dbgAssert(pchan->volticksleft != 0);
				pchan->volfade = (real32)(pchan->voltarget - pchan->volume) / (real32)pchan->volticksleft;

				if (pchan->volfade == 0.0f)
				{
					pchan->volfade = 0.01f;
					if (pchan->voltarget < pchan->volume)
					{
						pchan->volfade = -0.01f;
					}
				}
			}
		}
	}

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundpanF(sdword handle, sword pan, real32 fadetime)
{
	CHANNEL *pchan;
	sdword channel;
	sdword fadeblocks = 0;

	if (!soundinited)
	{
		return (SOUND_ERR);
	}
	
	channel = SNDchannel(handle);

	if (channel < SOUND_OK)
	{
		return (SOUND_ERR);
	}
	
	pchan = &channels[channel];

	if (pchan != NULL)
	{
		if (pchan->handle != handle)
		{
			return (SOUND_ERR);
		}

		if (pan == pchan->pan)
		{
			pchan->pantarget = pan;
			pchan->panticksleft = 0;
			pchan->panfade = 0;
		}
		else
		{
			fadeblocks = (sdword)(fadetime * SOUND_FADE_TIMETOBLOCKS);

			if (fadeblocks < NUM_FADE_BLOCKS)
			{
				fadeblocks = NUM_FADE_BLOCKS;
			}

			pchan->pantarget = pan;
			pchan->panticksleft = fadeblocks;
			pchan->panfade = (pchan->pantarget - pchan->pan) / pchan->panticksleft;

			if (pchan->panfade > 180)
			{
				pchan->panfade -= (pchan->panfade - 180);
			}
			else if (pchan->panfade < -180)
			{
				pchan->panfade -= (pchan->panfade + 180);
			}

			if (pchan->panfade == 0)
			{
				pchan->panfade = 1;
				if (pchan->pantarget < pchan->pan)
				{
					pchan->panfade = -1;
				}
			}
		}
	}

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		: soundfrequency
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundfrequency(sdword handle, real32 freq)
{
	CHANNEL *pchan;
	sdword channel;

	if (!soundinited)
	{
		return (SOUND_ERR);
	}
	
	channel = SNDchannel(handle);

	if (channel < SOUND_OK)
	{
		return (SOUND_ERR);
	}

	pchan = &channels[channel];

	if (pchan != NULL)
	{
		if (pchan->handle != handle)
		{
			return (SOUND_ERR);
		}

		if (freq != pchan->pitchtarget)
		{
			if (freq == pchan->pitch)
			{
				pchan->pitchtarget = freq;
				pchan->pitchticksleft = 0;
				pchan->pitchfade = 0.0f;
			}
			else
			{
				pchan->pitchtarget = freq;
				pchan->pitchticksleft = NUM_FADE_BLOCKS;
				pchan->pitchfade = (pchan->pitchtarget - pchan->pitch) / pchan->pitchticksleft;
			}
		}
	}

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		: soundequalize
	Description	:
	Inputs		: handle - the handle to a sound returned by soundplay
				  eq - array[SOUND_EQ_SIZE] of floats range of 0.0 to 1.0
	Outputs		:
	Return		: SOUND_OK if successful, SOUND_ERR on error
----------------------------------------------------------------------------*/	
sdword soundequalize(sdword handle, real32 *eq)
{
	CHANNEL *pchan;
	sdword channel, i;

	if (!soundinited)
	{
		return (SOUND_ERR);
	}
	
	if (eq == NULL)
	{
		return (SOUND_ERR);
	}

	channel = SNDchannel(handle);

	if (channel < 0)
	{
		return (SOUND_ERR);
	}

	pchan = &channels[channel];

	if (pchan != NULL)
	{
		if (pchan->handle != handle)
		{
			return (SOUND_ERR);
		}

		for (i = 0; i < SOUND_EQ_SIZE; i++)
		{
			pchan->filter[i] = eq[i];
		}
	}

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundshipheading(sdword handle, sword heading, sdword highband, sdword lowband, real32 velfactor, real32 shipfactor)
{
	CHANNEL *pchan;
	sdword channel;
	real32 factor;
	real32 inversefactor;
	real32 diff;
	sdword i;

	if (!soundinited)
	{
		return (SOUND_ERR);
	}
	
	channel = SNDchannel(handle);

	if (channel < SOUND_OK)
	{
		return (SOUND_ERR);
	}

	pchan = &channels[channel];

	if (pchan != NULL)
	{
		if (pchan->heading != handle)
		{
			return (SOUND_ERR);
		}

//		if (heading != pchan->heading)
		{
			pchan->heading = heading;

			factor = ((cardiod[heading] - 1.0f) * velfactor * shipfactor + 1.0f);
			inversefactor = ((cardiod[180 - heading] - 1.0f) * velfactor * shipfactor + 1.0f);

			for (i = 0; i < lowband; i++)
			{
				pchan->cardiodfilter[i] = factor;
			}

			diff = (inversefactor - factor) / (highband - lowband);

			for (i = lowband; i < highband; i++)
			{
				pchan->cardiodfilter[i] = factor + (diff * (i - lowband));
			}

			for (i = highband; i < SOUND_EQ_SIZE; i++)
			{
				pchan->cardiodfilter[i] = inversefactor;
			}
		}

		pchan->usecardiod = TRUE;
	}

	return (SOUND_OK);
}



/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword SNDresetchannel(CHANNEL *pchan)
{

	memset(pchan, 0, sizeof(CHANNEL));

	pchan->priority = SOUND_PRIORITY_NORMAL;
	pchan->handle = SOUND_DEFAULT;
	pchan->numchannels = SOUND_MONO;
	pchan->volfactorL = 1.0f;
	pchan->volfactorR = 1.0f;
	
	memset(pchan->filter, 1, SOUND_EQ_SIZE);
	memset(pchan->cardiodfilter, 1, SOUND_EQ_SIZE);
	pchan->usecardiod = FALSE;

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword SNDreleasebuffer(CHANNEL *pchan)
{
	sdword i;

	for (i = 0; i < numbanks; i++)
	{
		if ((bankpointers[i].start <= (void *)pchan->ppatch) &&
			(bankpointers[i].end >= (void *)pchan->ppatch))
		{
			numchans[i]--;
			break;
		}
	}
	
	pchan->handle = SOUND_DEFAULT;
	pchan->ppatch = NULL;
	pchan->status = SOUND_FREE;
	pchan->priority = SOUND_PRIORITY_MIN;

	channelsinuse--;

    return(0);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword SNDgetchannel(sword patchnum, sdword priority)
{
	CHANNEL *pchan;
	sdword	i;
	sdword	channel = SOUND_DEFAULT;
	sdword	lowchannel = SOUND_DEFAULT;
	real32	lowvolume = (real32)SOUND_VOL_MAX;
	sdword	lowticks = 255;
	sdword	lowpriority = priority;

	if ((channelsinuse < soundnumvoices-2) || (priority > SOUND_PRIORITY_MAX)) 	// Keep at least 2 voices available
	{

		for (i = 0; i < soundnumvoices; i++)
		{
			if (channels[i].status == SOUND_FREE)
			{
				/* found an unused channel */
				channel = i;
				channels[i].status = SOUND_INUSE;
				channels[i].priority = priority;
				channelsinuse++;
				break;
			}
		}
	}
	
	if (channel == SOUND_DEFAULT)
	{
		if (lowpriority < SOUND_PRIORITY_LOW)
		{
			lowpriority = SOUND_PRIORITY_LOW;
		}

		/* find the channel to steal */
		for (i = 0; i < soundnumvoices; i++)
		{
			pchan = &channels[i];
			/* don't want one that is already stopping */
			if (pchan->status == SOUND_PLAYING)
			{
				if (pchan->priority < lowpriority)
				{
					if (pchan->volume <= lowvolume)
					{
						if (pchan->volticksleft <= lowticks)
						{
							lowpriority = pchan->priority;
							lowticks = pchan->volticksleft;
							lowvolume = pchan->volume;
							lowchannel = i;
						}
					}
				}
			}
		}

		if (lowchannel > SOUND_DEFAULT)
		{
			/* stop the sound with the lowest priority */
			soundstop(channels[lowchannel].handle, SOUND_FADE_STOPNOW);
			
			/* find an empty channel */
			for (i = 0; i < soundnumvoices; i++)
			{
				if (channels[i].status == SOUND_FREE)
				{
					/* found an unused channel */
					channel = i;
					channels[i].status = SOUND_INUSE;
					channels[i].priority = priority;
					channelsinuse++;
					break;
				}
			}
		}
	}
	
	return (channel);
}


sdword SNDcreatehandle(sdword channel)
{
	if ((channel > SOUND_MAX_VOICES) || (channel < SOUND_OK))
	{
		return (SOUND_ERR);
	}

	return ((lasthandle++ << 8) + channel);
}


sdword SNDchannel(sdword handle)
{
	sdword channel;

	if (handle < SOUND_OK)
		return (SOUND_ERR);

	channel = handle & 0xFF;

	if ((channel > SOUND_MAX_VOICES) || (channel < SOUND_OK))
	{
		return (SOUND_ERR);
	}
	
	return (channel);
}


void SNDcalcvolpan(CHANNEL *pchan)
{
	pchan->volfactorL = pchan->volfactorR = pchan->volume / (real32)SOUND_VOL_MAX;
	
	if (pchan->pan < SOUND_PAN_CENTER)
	{
		/* panned left so attenuate right */
		pchan->volfactorR *= (real32)(SOUND_PAN_RIGHT + pchan->pan) / SOUND_PAN_RIGHT;
	}
	else if (pchan->pan > SOUND_PAN_CENTER)
	{
		/* panned right so attenuate left */
		pchan->volfactorL *= (real32)(SOUND_PAN_RIGHT - pchan->pan) / SOUND_PAN_RIGHT;
	}

#if 0
	if (pchan->pan < SOUND_PAN_CENTER)
	{
		pchan->volfactorR *= (real32)((pchan->pan - SOUND_PAN_RIGHT) * -1) / SOUND_PAN_CENTER;
	}
	else if (pchan->pan > SOUND_PAN_CENTER)
	{
		pchan->volfactorL *= (real32)(pchan->pan - SOUND_PAN_LEFT) / SOUND_PAN_CENTER;
	}
#endif

	if (pchan->volfactorL > (real32)1.0)
	{
		pchan->volfactorL = (real32)1.0;
	}
	if (pchan->volfactorL < (real32)-1.0)
	{
		pchan->volfactorL = (real32)-1.0;
	}
	if (pchan->volfactorR > (real32)1.0)
	{
		pchan->volfactorR = (real32)1.0;
	}
	if (pchan->volfactorR < (real32)-1.0)
	{
		pchan->volfactorR = (real32)-1.0;
	}
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword splayFPRVL(void *bankaddress, sdword patnum, real32 *eq, real32 freq, sword pan, sdword priority, sword vol, bool startatloop, bool fadein, bool mute)
{
	PATCH	*ppatch;
	CHANNEL	*pchan;
	sdword channel;
	sdword handle = SOUND_ERR;
	sdword i;

	if (patnum >= SOUND_OK)
	{
		ppatch = SNDgetpatch(bankaddress, patnum);
	}
	else if (patnum == SOUND_FLAGS_PATCHPOINTER)
	{
		/* compare patch ID */
		ppatch = (PATCH *)bankaddress;
	}
	else
	{
		return (handle);
	}

	if (ppatch == NULL)
	{
		return (handle);
	}

	if (vol == SOUND_DEFAULT)
	{
		vol = SOUND_VOL_MAX;
	}

	if ((vol <= SOUND_VOL_MIN) || (vol > SOUND_VOL_MAX))
	{
		return (handle);
	}

	if (freq == SOUND_DEFAULT)
	{
		freq = 1.0f;
	}


#if 0
	if (ppatch->dataoffset == NULL)
	{
		return (handle);
	}
#endif

	channel = SNDgetchannel((sword)patnum, priority);

	if (channel < SOUND_OK)
	{
		return (handle);
	}

	/* create handle here */
	handle = SNDcreatehandle(channel);

	for (i = 0; i < numbanks; i++)
	{
		if ((bankpointers[i].start <= (void *)ppatch) &&
			(bankpointers[i].end >= (void *)ppatch))
		{
			numchans[i]++;
			break;
		}
	}

	pchan = &channels[channel];
	SNDresetchannel(pchan);
	pchan->handle = handle;
	pchan->ppatch = ppatch;
	pchan->looping = ppatch->flags;
	pchan->pitch = freq;
	pchan->mute = mute;

	if (eq != NULL)
	{
		for (i = 0; i < SOUND_EQ_SIZE; i++)
		{
			pchan->filter[i] = eq[i];
		}
	}
	else
	{
		for (i = 0; i < SOUND_EQ_SIZE; i++)
		{
			pchan->filter[i] = (real32)1.0;
		}
	}
	
	for (i = 0; i < SOUND_EQ_SIZE; i++)
	{
		pchan->cardiodfilter[i] = 1.0f;
	}

	pchan->usecardiod = FALSE;

	if (!fadein || !pchan->looping)
	{
		pchan->volume = vol;
	}

	soundvolume(handle, vol);
	
	soundpan(handle, pan);

	SNDcalcvolpan(pchan);

	soundfrequency(handle, freq);
	
// NEWLOOP
	if (startatloop)
	{
		pchan->currentpos = (sbyte *)ppatch->loopstart;
	}
	else
	{
		pchan->currentpos = (sbyte *)ppatch->dataoffset;
	}
	
	if (ppatch->waveformat.frequency < FQ_RATE)
	{
		pchan->fqsize = FQ_QSIZE;
	}
	else
	{
		pchan->fqsize = FQ_HSIZE;
	}

#ifdef DEBUG_SOUNDLOW
	if (DS_OK != hr)
	{
        SNDreleasebuffer(pchan);
		dbgMessagef("\nDirectSound error, could not play patch %d",(udword)patnum);
	}
	else
#endif
		pchan->status = SOUND_PLAYING;

	return (handle);
}


PATCH *SNDgetpatch(void *bankaddress, sdword patnum)
{
	PATCH	*ppatch = NULL;
	BANK	*tempbank;
	PATCH	*temppatches;

	if (soundinited)
	{
		if (patnum >= 0)
		{
			tempbank = (BANK *)bankaddress;

			if (patnum < tempbank->numpatches)
			{
				temppatches = (PATCH *)&tempbank->firstpatch;
				ppatch = &temppatches[patnum];
			}
		}
	}

	return (ppatch);
}
