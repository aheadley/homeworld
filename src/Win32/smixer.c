/*=============================================================================
    Name    : smixer.c
    Purpose : Low level audio mixer routines

    Created 01/10/1998 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <dsound.h>
#include "fquant.h"
#include "soundcmn.h"
#include "soundlow.h"
#include "debug.h"
#include "soundstructs.h"
#include "randy.h"
#include "fastmath.h"
#include "main.h"
#include "globals.h"

#define MIX_BLOCK_SIZE			FQ_SIZE * sizeof(short) * 2 // 256 samples, 16-bit, stereo = 1024 bytes

#define DS_NUM_BUFFER_BLOCKS	32
#define DS_MIX_BUFFER_SIZE		(32 * MIX_BLOCK_SIZE)
#define DS_MIX_BUFFER_AHEAD		(16 * MIX_BLOCK_SIZE)
#define DS_MIX_SLEEP			0L

#define WO_NUM_BUFFER_BLOCKS	32
#define WO_MIX_BUFFER_SIZE		(WO_NUM_BUFFER_BLOCKS * MIX_BLOCK_SIZE)
#define WO_MIX_BUFFER_AHEAD		(WO_NUM_BUFFER_BLOCKS * MIX_BLOCK_SIZE)
#define WO_MIX_SLEEP			0L

#define MIX_PANIC_THRES			22L // approx 4 fps - 1000/(11.60997732426*fps)
#define MIX_PANIC_DUR			16L	// approx 4 seconds - dur*fps

/* function prototypes */
void isoundmixerthreadWAVEOUT(void *dummy);
void isoundmixerthreadDSOUND(void *dummy);
void isoundmixerqueueWAVEOUT();
sdword isoundmixerprocess(void *pBuf1, udword nSize1, void *pBuf2, udword nSize2);
sdword isoundmixerdecodeEffect(sbyte *readptr, real32 *writeptr1, real32 *writeptr2, ubyte *exponent, sdword size, uword bitrate, EFFECT *effect);
#define isoundmixerdecode(a, b, c, d, e, f)		isoundmixerdecodeEffect(a, b, c, d, e, f, NULL);

/* variables */
byte waveoutbuffer[WO_MIX_BUFFER_SIZE];
WAVEHDR wh;
HWAVEOUT hwo;
MMRESULT mmresult;
MMTIME mmtime;
WAVEFORMATEX woFormat;

LPSTR pWaveBuf[WO_NUM_BUFFER_BLOCKS];
WAVEHDR WaveHead[WO_NUM_BUFFER_BLOCKS];	// pointers to wave header information

sdword nBufRIndex = 0;
sdword nBufWIndex = 0;

udword mixerticks = 0;
udword panicdur = 0L;
udword framecount = 0L;

bool panicflag=FALSE;
sdword numvoices = 0;

udword buffersize;
DWORD dwBlockSize;
DWORD dwWritePos;
DWORD dwMixAhead;

sdword dctmode = FQ_MNORM;	// DCT mode
sdword dctsize = FQ_HSIZE;	// DCT block size

sdword dctpanicmode = SOUND_MODE_NORM;	// DCT panic mode, normal by default

LPDIRECTSOUNDBUFFER lpMixBuffer = NULL;

real32 timebufferL[FQ_DSIZE], timebufferR[FQ_DSIZE], temptimeL[FQ_DSIZE], temptimeR[FQ_DSIZE];
real32 mixbuffer1L[FQ_SIZE], mixbuffer1R[FQ_SIZE], mixbuffer2L[FQ_SIZE], mixbuffer2R[FQ_SIZE];

extern LPDIRECTSOUND lpDirectSound;
extern CHANNEL channels[];
extern bool soundinited;
extern STREAM streams[];
extern CHANNEL speechchannels[];
extern sdword numstreams;
extern bool bDirectSoundCertified;
extern SENTENCELUT *SentenceLUT;
extern real32 MasterEQ[];

extern SOUNDCOMPONENT	mixer;
extern SOUNDCOMPONENT	streamer;

extern bool bSoundPaused;
extern bool bSoundDeactivated;

extern sdword soundnumvoices;
extern sdword soundvoicemode;

extern udword rndFrameCount;

// DCT mode functions
void soundMixerGetMode(sdword *mode)	// mode SOUND_MODE_NORM or SOUND_MODE_AUTO or SOUND_MODE_LOW
{
	// check DCT mode and panic mode
	if((dctmode == FQ_MHALF) && (dctpanicmode == SOUND_MODE_NORM))
	{
		*mode=SOUND_MODE_LOW;

		return;
	}

	// check DCT mode and panic mode
	if((dctmode == FQ_MNORM) && (dctpanicmode == SOUND_MODE_AUTO))
	{
		*mode=SOUND_MODE_AUTO;

		return;
	}

	*mode=SOUND_MODE_NORM;

	return;
}

void soundMixerSetMode(sdword mode)		// mode SOUND_MODE_NORM or SOUND_MODE_AUTO or SOUND_MODE_LOW
{
	if(mode == SOUND_MODE_LOW)
	{
		// set DCT mode
		dctmode=FQ_MHALF;

		// set block size for effects
		dctsize=FQ_QSIZE;
		fqSize(dctsize);

		// set panic mode
		dctpanicmode=SOUND_MODE_NORM;

		return;
	}

	if(mode == SOUND_MODE_AUTO)
	{
		// set DCT mode
		dctmode=FQ_MNORM;

		// set block size for effects
		dctsize=FQ_HSIZE;
		fqSize(dctsize);

		// set panic mode
		dctpanicmode=SOUND_MODE_AUTO;

		return;
	}

	// if mode is SOUND_MODE_NORM
	// set DCT mode
	dctmode=FQ_MNORM;

	// set block size for effects
	dctsize=FQ_HSIZE;
	fqSize(dctsize);

	// set panic mode
	dctpanicmode=SOUND_MODE_NORM;

	return;
}

void soundPanicReset(void)
{
	// reset frame count
	framecount=0;
	panicflag=TRUE;
	return;
}


sdword smixCreateDSoundBuffer(WAVEFORMATEX *pcmwf)
{
	DSBUFFERDESC    dsbd;
	
	// Set up the direct sound buffer.
	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize			= sizeof(DSBUFFERDESC);
	dsbd.dwFlags		= DSBCAPS_STATIC | DSBCAPS_CTRLDEFAULT;
	dsbd.lpwfxFormat	= pcmwf;
	dsbd.dwBufferBytes	= DS_MIX_BUFFER_SIZE;

	if ((lpDirectSound->lpVtbl->CreateSoundBuffer(lpDirectSound, &dsbd,
		&lpMixBuffer, NULL )) != DS_OK)
	{
		lpMixBuffer = NULL;
		return (SOUND_ERR);
	}

	return (SOUND_OK);
}


sdword smixInitMixBuffer(WAVEFORMATEX *pcmwf)
{
	HRESULT hr;
	LPVOID lpvPtr1, lpvPtr2;
	DWORD dwLength1, dwLength2;
	sdword i;
	
	if (bDirectSoundCertified)
	{
		buffersize = DS_MIX_BUFFER_SIZE;
		dwBlockSize = buffersize / DS_NUM_BUFFER_BLOCKS;
		dwMixAhead = DS_MIX_BUFFER_AHEAD;
		
		// write 0's to mixer buffer
		// lock primary buffer
		hr = lpMixBuffer->lpVtbl->Lock(lpMixBuffer, 0, buffersize,
					&lpvPtr1, &dwLength1, &lpvPtr2, &dwLength2, 0L);
		
		memset((byte *)lpvPtr1, 0, buffersize);
		
		hr = lpMixBuffer->lpVtbl->Unlock(lpMixBuffer, lpvPtr1, dwLength1, lpvPtr2, dwLength2);
	
		// start playing some silence
		hr = lpMixBuffer->lpVtbl->SetCurrentPosition(lpMixBuffer, 0);
		hr = lpMixBuffer->lpVtbl->Play(lpMixBuffer, 0, 0, DSBPLAY_LOOPING);
	
		return (SOUND_OK);
	}

	// else this is the waveout thread stuff
	buffersize = WO_MIX_BUFFER_SIZE;
	dwBlockSize = buffersize / WO_NUM_BUFFER_BLOCKS;
	dwMixAhead = WO_MIX_BUFFER_AHEAD;
	
	memcpy(&woFormat, pcmwf, sizeof(WAVEFORMATEX));
	
	mmresult = waveOutOpen(&hwo, WAVE_MAPPER, (LPWAVEFORMATEX)&woFormat, 0, 0, CALLBACK_NULL | WAVE_ALLOWSYNC);
	if (mmresult != MMSYSERR_NOERROR)
	{
		// couldn't initialize the wave device
		return (SOUND_ERR);
	}
	
	for (i = 0; i < WO_NUM_BUFFER_BLOCKS; i++)
	{
		pWaveBuf[i] = (LPSTR)(waveoutbuffer + (dwBlockSize * i));
		WaveHead[i].lpData = pWaveBuf[i];
		WaveHead[i].dwBufferLength = dwBlockSize;
		WaveHead[i].dwFlags = 0;
	
		mmresult = waveOutPrepareHeader(hwo, (LPWAVEHDR)&WaveHead[i], sizeof(WAVEHDR));
		if (mmresult != MMSYSERR_NOERROR)
		{
			// couldn't initialize the wave device
			// close the device...
			isoundmixerrestore();
			return (SOUND_ERR);
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
sdword isoundmixerinit(WAVEFORMATEX *pcmwf)
{
	
	// Initialize codec
	fqInitDequant();
	
	// Set random number generator for effects
	fqRand((int (*)(int))ranRandomFnSimple,RAN_Sound);

	// Set square root function for effects
	fqSqrt((double (*)(double))fmathSqrtDouble);

	// Set block size for effects
	fqSize(dctsize);

	if (bDirectSoundCertified)
	{
		// Create the Direct Sound buffer
		if (smixCreateDSoundBuffer(pcmwf) != SOUND_OK)
		{
			return (SOUND_ERR);
		}
	}
			
	// init the iDCT function
	if (SOUND_OK != fqDecBlock(mixbuffer1L, mixbuffer2L, timebufferL, temptimeL, FQ_MINIT, FQ_MINIT))
	{
		return (SOUND_ERR);
	}

	if (SOUND_OK != fqDecBlock(mixbuffer1R, mixbuffer2R, timebufferR, temptimeR, FQ_MINIT, FQ_MINIT))
	{
		return (SOUND_ERR);
	}
	
	// start the thread
	if (smixInitMixBuffer(pcmwf) != SOUND_OK)
	{
		return (SOUND_ERR);
	}

	if (bDirectSoundCertified)
	{
		_beginthread(isoundmixerthreadDSOUND, 0, NULL);
	}
	else
	{
		_beginthread(isoundmixerthreadWAVEOUT, 0, NULL);
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
void isoundmixerrestore(void)
{
	HRESULT hr;
	sdword i;

	if (bDirectSoundCertified)
	{
		mixer.timeout = 0;
		hr = lpMixBuffer->lpVtbl->Stop(lpMixBuffer);
//		mixer.status = SOUND_FREE;
		lpMixBuffer->lpVtbl->Release(lpMixBuffer);
	}
	else
	{				
		mixer.timeout = 0;
		mmresult = waveOutReset(hwo);
//		mixer.status = SOUND_FREE;
		for (i = 0; i < WO_NUM_BUFFER_BLOCKS; i++)
		{
			mmresult = waveOutUnprepareHeader(hwo, &WaveHead[i], sizeof(WAVEHDR));
		}
		mmresult = waveOutClose(hwo);
	}
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword isoundmixerprocess(void *pBuf1, udword nSize1, void *pBuf2, udword nSize2)
{
	sdword i, amountread;
	CHANNEL	*pchan;
	STREAM *pstream;
	STREAMQUEUE *pqueue;
	real32 scaleLevel;
	sdword chan;

	////////////////////////
	// begin panic mode code
	////////////////////////

	// check game running and sound deactivated flags
	if((gameIsRunning) && (!bSoundDeactivated))
	{
		// check ticks
		if(mixerticks%MIX_PANIC_THRES == 0)
		{
			// compare frame numbers
			if((framecount > 0L) && (framecount == rndFrameCount))
			{
				// check panic flag
				if(panicflag == FALSE)
				{
					// check panic mode
					if(dctpanicmode == SOUND_MODE_AUTO)
					{
						// set DCT mode
						dctmode=FQ_MHALF;

						// set block size for effects
						dctsize=FQ_QSIZE;
						fqSize(dctsize);
					}

					// check voice mode
					if(soundvoicemode == SOUND_MODE_AUTO)
					{
						// set number of voices to min
						soundnumvoices=SOUND_MIN_VOICES;

						// argh... fuck! panic, panic :)
						soundPanic();
					}

					// set panic flag
					panicflag=TRUE;
				}

				// reset duration
				panicdur=0;
			}


			// check duration
			if(panicdur >= MIX_PANIC_DUR)
			{
				// check panic flag
				if(panicflag == TRUE)
				{
					// check panic mode
					if(dctpanicmode == SOUND_MODE_AUTO)
					{
						// set DCT mode
						dctmode=FQ_MNORM;

						// set block size for effects
						dctsize=FQ_HSIZE;
						fqSize(dctsize);
					}

					// check voice mode
					if(soundvoicemode == SOUND_MODE_AUTO)
					{
						// set number of voices
						if(numvoices > 0)
						{
							// set number of voices back
							soundnumvoices=numvoices;
						}
					}

					// set panic flag
					panicflag=FALSE;
				}

				// reset duration
				panicdur=0;
			}
			else
			{
				// increment duration
				panicdur++;
			}

			// check panic flag
			if(panicflag == FALSE)
			{
				// save number of voices
				numvoices=soundnumvoices;
			}
			
			// save frame number
			framecount=rndFrameCount;
		}
	}

	///////////////////////
	// end panic mode code
	///////////////////////
	
	/* clear the mixbuffers */
	memset(mixbuffer1L, 0, FQ_SIZE * sizeof(real32));
	memset(mixbuffer1R, 0, FQ_SIZE * sizeof(real32));
	memset(mixbuffer2L, 0, FQ_SIZE * sizeof(real32));
	memset(mixbuffer2R, 0, FQ_SIZE * sizeof(real32));
	
	/* mix the speech first */
	if (streams != NULL)
	{
		for (i = 0; i < numstreams; i++)
		{
			if(speechchannels[i].status >= SOUND_PLAYING)
			{
				scaleLevel = 1.0f;

				pstream = &streams[i];
				pchan = &speechchannels[i];
				pqueue = &pstream->queue[pstream->playindex];
	
				if (pstream->blockstatus[pstream->readblock] == 0)
				{
					if (pchan->status == SOUND_STOPPING)
					{
						pstream->writepos = 0;
						pstream->playindex = pstream->writeindex;
						pstream->numtoplay = 0;
						pchan->status = SOUND_INUSE;
					}
					continue;
				}
	
				if (pstream->blockstatus[pstream->readblock] == 1)
				{
					pstream->blockstatus[pstream->readblock] = 2;
				}
	
				amountread = isoundmixerdecodeEffect(pchan->currentpos, pchan->mixbuffer1, pchan->mixbuffer2, pchan->exponentblockL,
								pchan->fqsize, pchan->bitrate, pqueue->effect);
				pchan->currentpos += amountread;
				pchan->amountread += amountread;
				
				/* figure out any volume fades, pan fades, etc */
				if (pchan->volticksleft)
				{
					pchan->volume += pchan->volfade;
					pchan->volticksleft--;
					
					if (pchan->volticksleft <= 0)
					{
						pchan->volfade = 0.0f;
						pchan->volume = (real32)pchan->voltarget;
						pchan->volticksleft = 0;
						pchan->voltarget = -1;
					}
	
					if ((sword)pchan->volume > SOUND_VOL_MAX)
					{
						pchan->volume = SOUND_VOL_MAX;
					}
	
					if ((sword)pchan->volume < SOUND_VOL_MIN)
					{
						pchan->volume = SOUND_VOL_MIN;
					}
	
					if ((pchan->status == SOUND_STOPPING) && (pchan->volume == SOUND_VOL_MIN))
					{
						if (pstream->numqueued == 0)
						{
							pstream->status = SOUND_STREAM_INUSE;
							pstream->queueindex = 0;
							pstream->writeindex = 0;
							pstream->playindex = 0;
							pstream->numqueued = 0;
						}
						else
						{
							pstream->status = SOUND_STREAM_STARTING;
							pstream->writeindex = pstream->queueindex - pstream->numqueued;
							if (pstream->writeindex < 0)
							{
								pstream->writeindex += SOUND_MAX_STREAM_QUEUE;
							}
							pstream->playindex = pstream->writeindex;
						}
						pchan->status = SOUND_FREE;
						continue;
					}
					
					SNDcalcvolpan(pchan);
				}
	
				/******************************************************************************/
				/* Shane - pchan->volfactorL BELOW IS PROBABLY NOT THE PROPER VOLUME. - Janik */
				/******************************************************************************/

				if (pchan->numchannels < SOUND_STEREO)	// added this check because of a strange bug that caused the music to
				{										// have a mixHandle of 0, which played a buzzing in the left channel
														// this doesn't fix the bug, but it'll keep the buzzing from happening
					/* mix in SFX */
					if (pqueue->mixHandle >= SOUND_OK)
					{
	//					fqMix(pchan->mixbuffer1,channels[SNDchannel(pqueue->mixHandle)].mixbuffer1, pchan->volfactorL);
	//					fqMix(pchan->mixbuffer2,channels[SNDchannel(pqueue->mixHandle)].mixbuffer2, pchan->volfactorL);
						chan = SNDchannel(pqueue->mixHandle);
						fqMix(pchan->mixbuffer1,channels[chan].mixbuffer1, 1.0f);	//channels[chan].volfactorL);
						fqMix(pchan->mixbuffer2,channels[chan].mixbuffer2, 1.0f);	//channels[chan].volfactorL);
					}
	
					/* do the EQ and Delay or Acoustic Model stuff */
					if (pqueue->eq != NULL)
					{
						if (pqueue->eq->flags && STREAM_FLAGS_EQ)
						{
							/* equalize this sucker */
							fqEqualize(pchan->mixbuffer1, pqueue->eq->eq);
							fqEqualize(pchan->mixbuffer2, pqueue->eq->eq);
						}
					}
		
					if (pqueue->effect != NULL)
					{
						// Add tone
						fqAddToneE(pchan->mixbuffer1, pqueue->effect);
						fqAddToneE(pchan->mixbuffer2, pqueue->effect);
				
						// Generate break
						fqAddBreakE(pchan->mixbuffer1, pqueue->effect);
						fqAddBreakE(pchan->mixbuffer2, pqueue->effect);
						
						// Add noise
						fqAddNoiseE(pchan->mixbuffer1, pqueue->effect);
						fqAddNoiseE(pchan->mixbuffer2, pqueue->effect);
					
						// Limit
						fqLimitE(pchan->mixbuffer1, pqueue->effect);
						fqLimitE(pchan->mixbuffer2, pqueue->effect);
	
						// Filter
						fqFilterE(pchan->mixbuffer1, pqueue->effect);
						fqFilterE(pchan->mixbuffer2, pqueue->effect);
	
						scaleLevel = pqueue->effect->fScaleLev;
					}
		
					if (pqueue->delay != NULL)
					{
						if (pqueue->delay->flags & STREAM_FLAGS_ACMODEL)
						{
							fqAcModel(pchan->mixbuffer1, pqueue->delay->eq, pqueue->delay->duration,
									  pstream->delaybuffer1, DELAY_BUF_SIZE, &(pstream->delaypos1));
							fqAcModel(pchan->mixbuffer2, pqueue->delay->eq, pqueue->delay->duration,
									  pstream->delaybuffer2, DELAY_BUF_SIZE, &(pstream->delaypos2));
						}
						else if (pqueue->delay->flags & STREAM_FLAGS_DELAY)
						{
							fqDelay(pchan->mixbuffer1, pqueue->delay->level, pqueue->delay->duration,
									  pstream->delaybuffer1, DELAY_BUF_SIZE, &(pstream->delaypos1));
							fqDelay(pchan->mixbuffer2, pqueue->delay->level, pqueue->delay->duration,
									  pstream->delaybuffer2, DELAY_BUF_SIZE, &(pstream->delaypos2));
						}
					}
				}
	
				/* add it to mix buffer */
				fqMix(mixbuffer1L,pchan->mixbuffer1,pchan->volfactorL * scaleLevel);
				fqMix(mixbuffer2L,pchan->mixbuffer2,pchan->volfactorL * scaleLevel);

				/* if this is stereo, do the right channel */
				if (pchan->numchannels == SOUND_STEREO)
				{
					amountread = isoundmixerdecode(pchan->currentpos, pchan->mixbuffer1R, pchan->mixbuffer2R, pchan->exponentblockR,
									pchan->fqsize, pchan->bitrate);
					pchan->currentpos += amountread;
					pchan->amountread += amountread;

					/*****************************************************/
					/* Shane - The code below never gets called! - Janik */
					/*****************************************************/
/*
					if (pqueue->effect != NULL)
					{
						// Add tone
						fqAddToneE(pchan->mixbuffer1R, pqueue->effect);
						fqAddToneE(pchan->mixbuffer2R, pqueue->effect);
				
						// Generate break
						fqAddBreakE(pchan->mixbuffer1R, pqueue->effect);
						fqAddBreakE(pchan->mixbuffer2R, pqueue->effect);
						
						// Add noise
						fqAddNoiseE(pchan->mixbuffer1R, pqueue->effect);
						fqAddNoiseE(pchan->mixbuffer2R, pqueue->effect);

						// Limit
						fqLimitE(pchan->mixbuffer1R, pqueue->effect);
						fqLimitE(pchan->mixbuffer2R, pqueue->effect);

						// Filter
						fqFilterE(pchan->mixbuffer1R, pqueue->effect);
						fqFilterE(pchan->mixbuffer2R, pqueue->effect);
					}
*/

					fqMix(mixbuffer1R,pchan->mixbuffer1R,pchan->volfactorR * scaleLevel);
					fqMix(mixbuffer2R,pchan->mixbuffer2R,pchan->volfactorR * scaleLevel);
				}
				else
				{
					fqMix(mixbuffer1R,pchan->mixbuffer1,pchan->volfactorR * scaleLevel);
					fqMix(mixbuffer2R,pchan->mixbuffer2,pchan->volfactorR * scaleLevel);
				}
	
				/* this is a clock for the fequency filtering/effects */
				if (pqueue->effect != NULL)
				{
					pqueue->effect->nClockCount++;
				}
	
				if ((pchan->currentpos == (sbyte *)(pstream->buffer + (pstream->blocksize * (pstream->readblock + 1)))) ||
					(pchan->currentpos == pstream->writepos))
				{
					pstream->blockstatus[pstream->readblock++] = 0;
					if (pstream->readblock >= 2)
					{
						pstream->readblock = 0;
					}
				}
	
				if (pchan->amountread == pqueue->size)
				{
					pchan->amountread = 0;
					if (pstream->numtoplay > 0)
					{
						/* finished this queued stream, go on to the next */
						pstream->playindex++;
						pstream->numtoplay--;
						if (pstream->playindex >= SOUND_MAX_STREAM_QUEUE)
						{
							pstream->playindex = 0;
						}

						if (pqueue->mixHandle >= SOUND_OK)
						{
							if (pqueue->pmixPatch != pstream->queue[pstream->playindex].pmixPatch)
							{
								/* we're mixing in a patch to this stream, shut it down */
								soundstop(pqueue->mixHandle, 0.0f);
								pqueue->mixHandle = SOUND_DEFAULT;
								pqueue->pmixPatch = NULL;
								pqueue->mixLevel = SOUND_VOL_MIN;
							}
							else
							{
								pstream->queue[pstream->playindex].mixHandle = pqueue->mixHandle;
							}
						}			
					}
				}
	
				if (pchan->currentpos >= pchan->endpos)
				{
					/* get the next block */
					pchan->currentpos = pchan->freqdata;
				}
			}
		}
	}

	/* mix the SFX channels */
	for (i = 0; i < soundnumvoices; i++)
	{
		if (channels[i].status >= SOUND_PLAYING)
		{
			pchan = &channels[i];

			if (!pchan->looping)
			{
				if (pchan->currentpos >= (sbyte *)pchan->ppatch->datasize)
				{
					SNDreleasebuffer(pchan);
					continue;
				}
			}
			
			if (pchan->status == SOUND_LOOPEND)
			{
				pchan->status = SOUND_PLAYING;
				pchan->currentpos = (sbyte *)pchan->ppatch->loopend;
				pchan->looping = FALSE;
			}
			else if (pchan->status == SOUND_RESTART)
			{
				pchan->status = SOUND_PLAYING;
				pchan->currentpos = (sbyte *)pchan->ppatch->dataoffset;
			}


			amountread = isoundmixerdecode(pchan->currentpos, pchan->mixbuffer1, pchan->mixbuffer2, pchan->exponentblockL,
							pchan->fqsize, pchan->ppatch->bitrate);
			pchan->currentpos += amountread;
			
			if (pchan->looping)
			{
				if (pchan->currentpos >= (sbyte *)pchan->ppatch->loopend)
				{
					pchan->currentpos = (sbyte *)pchan->ppatch->loopstart;
				}
			}

			/* figure out any volume fades, pan fades, etc */
			if (pchan->volticksleft)
			{
				pchan->volume += pchan->volfade;
				pchan->volticksleft--;
				
				if (pchan->volticksleft <= 0)
				{
					pchan->volfade = 0.0f;
					pchan->volume = (real32)pchan->voltarget;
					pchan->volticksleft = 0;
					pchan->voltarget = -1;
				}

				if ((sword)pchan->volume > SOUND_VOL_MAX)
				{
					pchan->volume = SOUND_VOL_MAX;
				}

				if ((sword)pchan->volume < SOUND_VOL_MIN)
				{
					pchan->volume = SOUND_VOL_MIN;
				}

				if ((pchan->status == SOUND_STOPPING) && ((sword)pchan->volume == SOUND_VOL_MIN))
				{
					SNDreleasebuffer(pchan);
					continue;
				}
				
				SNDcalcvolpan(pchan);
			}

			if (pchan->panticksleft)
			{
				pchan->pan += pchan->panfade;
				pchan->panticksleft--;
				
				if (pchan->panticksleft <= 0)
				{
					pchan->panfade = 0;
					pchan->pan = pchan->pantarget;
					pchan->panticksleft = 0;
				}
			
				if (pchan->pan > SOUND_PAN_MAX)
				{
					pchan->pan =  SOUND_PAN_MIN + (pchan->pan - SOUND_PAN_MAX);
				}
			
				if (pchan->pan < SOUND_PAN_MIN)
				{
					pchan->pan = SOUND_PAN_MAX + (pchan->pan - SOUND_PAN_MIN);
				}
				
				SNDcalcvolpan(pchan);
			}

			if (pchan->pitchticksleft)
			{
				pchan->pitch += pchan->pitchfade;
				pchan->pitchticksleft--;
				
				if (pchan->pitchticksleft <= 0)
				{
					pchan->pitchfade = 0.0f;
					pchan->pitch = pchan->pitchtarget;
					pchan->pitchticksleft = 0;
				}
			}
			
			/******************************************************/
			/* Shane - Try the pitch shift again below... - Janik */
			/******************************************************/
#if 1
			/* do pitch shift */
			fqPitchShift(pchan->mixbuffer1, pchan->pitch);
			fqPitchShift(pchan->mixbuffer2, pchan->pitch);
#endif

			if (pchan->usecardiod)
			{
				/* apply cardiod filter for fake doppler */
				fqEqualize(pchan->mixbuffer1, pchan->cardiodfilter);
				fqEqualize(pchan->mixbuffer2, pchan->cardiodfilter);
			}
			
			/* equalize this sucker */
			fqEqualize(pchan->mixbuffer1, pchan->filter);
			fqEqualize(pchan->mixbuffer2, pchan->filter);

			if (!pchan->mute)
			{
				fqMix(mixbuffer1L,pchan->mixbuffer1,pchan->volfactorL);
				fqMix(mixbuffer2L,pchan->mixbuffer2,pchan->volfactorL);
				fqMix(mixbuffer1R,pchan->mixbuffer1,pchan->volfactorR);
				fqMix(mixbuffer2R,pchan->mixbuffer2,pchan->volfactorR);
			}
		}
		
	}
	
	fqEqualize(mixbuffer1L, MasterEQ);
	fqEqualize(mixbuffer2L, MasterEQ);
	fqEqualize(mixbuffer1R, MasterEQ);
	fqEqualize(mixbuffer2R, MasterEQ);

	fqDecBlock(mixbuffer1L, mixbuffer2L, timebufferL, temptimeL, dctmode, FQ_MNORM);
	fqDecBlock(mixbuffer1R, mixbuffer2R, timebufferR, temptimeR, dctmode, FQ_MNORM);

	if (!reverseStereo)
	{
		/* play normally */
		fqWriteTBlock(timebufferL, timebufferR, 2, pBuf1, nSize1, pBuf2, nSize2);
	}
	else
	{
		/* swap the left and right channels */
		fqWriteTBlock(timebufferR, timebufferL, 2, pBuf1, nSize1, pBuf2, nSize2);
	}

	mixerticks++;

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword isoundmixerdecodeEffect(sbyte *readptr, real32 *writeptr1, real32 *writeptr2, ubyte *exponent, sdword size, uword bitrate, EFFECT *effect)
{
    sbyte tempblock[FQ_LEN];

	memset(tempblock, 0, FQ_LEN);
	memcpy(tempblock, readptr, (bitrate >> 3));

	// Check size
	if(size > dctsize) size=dctsize;
	
	if (effect != NULL)
	{
		fqGenQNoiseE(tempblock, bitrate, effect);
	}

	fqDequantBlock(tempblock, writeptr1, writeptr2, exponent,
					FQ_LEN, bitrate, size);

	return (bitrate >> 3);
}

/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
void isoundmixerthreadDSOUND(void *dummy)
{
	unsigned long i,dataleft;
	short *lpvWritePtr;
	LPVOID lpvPtr1, lpvPtr2;
	DWORD dwLength1, dwLength2;
	HRESULT hr;
	DWORD			dwCurrentPlayCursor;
	DWORD			dwCurrentWriteCursor;
	DWORD	dwReadAhead;
	DWORD	dwPlayAhead;
	bool	missed = FALSE;
	bool	mix = FALSE;
	bool	test = FALSE;

	mixer.status = SOUND_PLAYING;	

	while (soundinited && (mixer.status >= SOUND_STOPPED))
	{
		if (mixer.status >= SOUND_PLAYING)
		{
			/* write block to DSound buffer */					
			hr = lpMixBuffer->lpVtbl->GetCurrentPosition(lpMixBuffer, &dwCurrentPlayCursor, &dwCurrentWriteCursor);
			
			dwReadAhead = dwCurrentPlayCursor + dwMixAhead;
			if (dwReadAhead >= buffersize)
			{
				dwReadAhead -= buffersize;
			}
		
			dwPlayAhead = dwWritePos + (buffersize - dwMixAhead);
			if (dwPlayAhead >= buffersize)
			{
				dwPlayAhead -= buffersize;
			}
	
			if ((dwReadAhead >= dwWritePos) && (dwCurrentPlayCursor >= dwPlayAhead))
			{
				mix = TRUE;
			}
			else if ((dwReadAhead >= dwWritePos) && (dwCurrentPlayCursor + buffersize >= dwPlayAhead)
					 && (dwWritePos > dwCurrentPlayCursor))
			{
				mix = TRUE;
			}
			else if ((dwCurrentPlayCursor >= dwPlayAhead) && (dwReadAhead + buffersize >= dwWritePos)
					 && (dwWritePos > dwCurrentPlayCursor))
			{
				mix = TRUE;
			}
			else
			{
				mix = FALSE;
			}
	
			if (mix)
			{
				/* lock primary buffer */
				hr = lpMixBuffer->lpVtbl->Lock(lpMixBuffer, dwWritePos, dwBlockSize,
						  &lpvPtr1, &dwLength1, &lpvPtr2, &dwLength2, 0L);
		
				/* successful */
				if (hr == DS_OK)
				{
					lpvWritePtr = (short *)lpvPtr1;
					dataleft = (unsigned long)dwLength1;
			
					for (i = 0; i < dwBlockSize; i += MIX_BLOCK_SIZE)
					{
						dataleft -= MIX_BLOCK_SIZE;
		
						if (dataleft > 0)
						{
							// process 256 samples, 16-bit (independent of # of channels)
							isoundmixerprocess(lpvWritePtr, FQ_SIZE * sizeof(short), NULL, 0);
							lpvWritePtr += FQ_SIZE * sizeof(short);
						}
						else
						if (dataleft < 0)
						{
							// process 256 samples, 16-bit (independent of # of channels)
							isoundmixerprocess(lpvWritePtr, (dataleft/2 + (FQ_SIZE * sizeof(short))),lpvPtr2, (0 - dataleft/2));
							lpvWritePtr = (short *)lpvPtr2 + dataleft/2;
							dataleft = (unsigned long)dwLength2 + dataleft;
						}
						else	/* if (dataleft == 0) */
						{
							// process 256 samples, 16-bit (independent of # of channels)
							isoundmixerprocess(lpvWritePtr, FQ_SIZE * sizeof(short), NULL, 0);
							lpvWritePtr = (short *)lpvPtr2;
							dataleft = (unsigned long)dwLength2;
						}
					}
		
					hr = lpMixBuffer->lpVtbl->Unlock(lpMixBuffer, lpvPtr1, dwLength1, lpvPtr2, dwLength2);
					dbgAssert(hr == DS_OK);
	
					dwWritePos += dwBlockSize;
				
					if (dwWritePos >= buffersize)
					{
						dwWritePos -= buffersize;
					}
				}
				else	/* primary buffer not ready yet */
				{
					Sleep(DS_MIX_SLEEP);
				}
			}
			else
			{
				Sleep(DS_MIX_SLEEP);
			}
	
			if (mixer.status == SOUND_STOPPING)
			{
				/* check and see if its done yet */
				if (mixer.timeout <= mixerticks)
				{
					lpMixBuffer->lpVtbl->Stop(lpMixBuffer);
					mixer.timeout = 0;
					mixer.status = SOUND_STOPPED;
				}
			}
			
			if (bSoundPaused && (mixer.status == SOUND_PLAYING))
			{
				mixer.status = SOUND_STOPPING;
			}
			if (bSoundDeactivated && (mixer.status == SOUND_PLAYING))
			{
				lpMixBuffer->lpVtbl->Stop(lpMixBuffer);
				mixer.status = SOUND_STOPPED;
			}
		}
		else if (mixer.status == SOUND_STOPPED)
		{
			/* mixer is paused so don't do nothing */
			if ((!bSoundPaused) && (!bSoundDeactivated))
			{
				/* write 0's to mixer buffer */
				/* lock primary buffer */
				lpMixBuffer->lpVtbl->Lock(lpMixBuffer, 0, buffersize,
							&lpvPtr1, &dwLength1, &lpvPtr2, &dwLength2, 0L);
				
				memset((byte *)lpvPtr1, 0, buffersize);
				
				lpMixBuffer->lpVtbl->Unlock(lpMixBuffer, lpvPtr1, dwLength1, lpvPtr2, dwLength2);
			
				/* start playing some silence */
				lpMixBuffer->lpVtbl->SetCurrentPosition(lpMixBuffer, 0);
				lpMixBuffer->lpVtbl->Play(lpMixBuffer, 0, 0, DSBPLAY_LOOPING);
			
				mixer.status = SOUND_PLAYING;
			}
			else
			{
				Sleep(0L);
			}
		}
	}

	// hr = lpMixBuffer->lpVtbl->Stop(lpMixBuffer);

	/* clean out the buffer */
	mixer.status = SOUND_FREE;

	_endthread();
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
void isoundmixerthreadWAVEOUT(void *dummy)
{
	sdword i;
	
	bool	missed = FALSE;
	bool	mix = FALSE;
	bool	test = FALSE;

	DWORD flags;

	memset(&waveoutbuffer, 0, buffersize);

	nBufRIndex = 0;
	nBufWIndex = 0;
	for(i=0;i<WO_NUM_BUFFER_BLOCKS-1;i++)
	{
		isoundmixerqueueWAVEOUT();
		Sleep(0);
	}

	mixer.status = SOUND_PLAYING;
	
	while (soundinited && (mixer.status >= SOUND_STOPPED))
	{
		if (mixer.status >= SOUND_PLAYING)
		{
			// check flags field
			flags=WaveHead[nBufRIndex].dwFlags&(DWORD)(WHDR_DONE|WHDR_PREPARED|WHDR_BEGINLOOP|WHDR_ENDLOOP|WHDR_INQUEUE);
			if((flags == (DWORD)(WHDR_DONE|WHDR_PREPARED)) || (flags == (DWORD)WHDR_PREPARED))
			{
				isoundmixerqueueWAVEOUT();

				nBufRIndex++;
				if (nBufRIndex >= WO_NUM_BUFFER_BLOCKS)
				{
					nBufRIndex = 0;
				}
			}
			else
			{
				Sleep(WO_MIX_SLEEP);
			}

			if (mixer.status == SOUND_STOPPING)
			{
				/* check and see if its done yet */
				if (mixer.timeout <= mixerticks)
				{
					mixer.timeout = 0;

					mmresult = waveOutReset(hwo);
					// dbgAssert(mmresult == MMSYSERR_NOERROR);

					mixer.status = SOUND_STOPPED;
				}
			}
			
			if (bSoundPaused && (mixer.status == SOUND_PLAYING))
			{
				mixer.status = SOUND_STOPPING;
			}

			if (bSoundDeactivated && (mixer.status == SOUND_PLAYING))
			{
				mmresult = waveOutReset(hwo);
				// dbgAssert(mmresult == MMSYSERR_NOERROR);

				mixer.status = SOUND_STOPPED;
			}
		}
		else if (mixer.status == SOUND_STOPPED)
		{
			/* mixer is paused so don't do anything */
			if ((!bSoundPaused) && (!bSoundDeactivated))
			{
				memset(&waveoutbuffer, 0, buffersize);
			
				nBufRIndex = 0;
				nBufWIndex = 0;
				for(i=0;i<WO_NUM_BUFFER_BLOCKS-1;i++)
				{
					isoundmixerqueueWAVEOUT();
					Sleep(0L);
				}

				mixer.status = SOUND_PLAYING;
			}
			else
			{
				Sleep(0L);
			}
		}
	}

	// mmresult = waveOutReset(hwo);
	// dbgAssert(mmresult == MMSYSERR_NOERROR);

	mixer.status = SOUND_FREE;

	_endthread();
}

/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
void isoundmixerqueueWAVEOUT()
{
	udword i, dataleft;
	short *lpvWritePtr;
	LPVOID lpvPtr2=NULL;
	DWORD dwLength2=0;

	// aet flags field
	WaveHead[nBufWIndex].dwFlags = (DWORD)WHDR_PREPARED;

	lpvWritePtr = (short *)WaveHead[nBufWIndex].lpData;
	dataleft = WaveHead[nBufWIndex].dwBufferLength;

	for (i = 0; i < dwBlockSize; i += MIX_BLOCK_SIZE)
	{
		dataleft -= MIX_BLOCK_SIZE;

		if (dataleft >= 0)
		{
			// process 256 samples, 16-bit (independent of # of channels)
			isoundmixerprocess(lpvWritePtr, FQ_SIZE * sizeof(short), NULL, 0);
			lpvWritePtr += FQ_SIZE * sizeof(short);
		}
		// dbgAssert(dataleft >= 0);
	}
	
	// write data to waveout device
	mmresult = waveOutWrite(hwo, (LPWAVEHDR)&WaveHead[nBufWIndex], sizeof(WAVEHDR));
	// dbgAssert(mmresult == MMSYSERR_NOERROR);

	nBufWIndex++;
	if (nBufWIndex >= WO_NUM_BUFFER_BLOCKS)
	{
		nBufWIndex = 0;
	}

	return;
}
