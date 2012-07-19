/*=============================================================================
    Name    : sstream.c
    Purpose : Low level audio streamer routines

    Created 01/10/1998 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <mmsystem.h>
#include <dsound.h>
#include <process.h>

#include "types.h"
#include "switches.h"
#include "debug.h"
#include "soundlow.h"
#include "soundcmn.h"
#include "file.h"
#include "fqeffect.h"
#include "soundstructs.h"
#include "speechevent.h"
#include "subtitle.h"

/* functions */
sdword isoundstreamreadheader(STREAM *pstream);


/* variables */
streamprintfunction	debugfunction = NULL;
char debugtext[256];

//filehandle streamfile;
CHANNEL speechchannels[SOUND_MAX_STREAM_BUFFERS];
STREAM streams[SOUND_MAX_STREAM_BUFFERS];
sdword numstreams;

#if VCE_BACKWARDS_COMPATIBLE
bool ssOldFormatVCE = FALSE;
#endif

extern LPDIRECTSOUND lpDirectSound;
extern CHANNEL channels[];
extern SENTENCELUT *SentenceLUT;
extern bool soundinited;
extern SOUNDCOMPONENT	streamer;
extern udword mixerticks;

extern bool bSoundPaused;
extern bool bSoundDeactivated;


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
void soundstreamquery(sdword maxstreams, sdword *pbuffersize, sdword *pstreamersize)
{
	if (maxstreams > SOUND_MAX_STREAM_BUFFERS)
	{
		*pstreamersize = 0;
		*pbuffersize = 0;
		return;
	}

	numstreams = maxstreams;

	/* figure out how much memory we need for the array of stream structures */
	*pstreamersize = (sdword)(maxstreams * sizeof(STREAM));

	/* figure out how big each stream buffer needs to be */
	*pbuffersize = SOUND_STREAM_BUFFER_SIZE + 4;		// needs 4 extra bytes for dequantizing.
}



sdword streamStartThread(void)
{
	_beginthread(isoundstreamupdate, 0, NULL);
			
	streamer.status = SOUND_PLAYING;

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundstreaminit(void *pstreamer, sdword size, sdword nostreams, streamprintfunction printfunction)
{
	sdword i, j;
	STREAM *pstream;

	memset(pstreamer, 0, size);

	dbgAssert(streamer.status == SOUND_FREE);

	/* go through stream structures and init */
	for (i = 0; i < numstreams; i++)
	{
		pstream = &streams[i];

		pstream->buffer = NULL;
		for (j = 0; j < SOUND_MAX_STREAM_QUEUE; j++)
		{
			pstream->queue[j].fhandle = SOUND_ERR;
			pstream->queue[j].offset = SOUND_ERR;
		}
		pstream->numqueued = 0;
		pstream->numtoplay = 0;
        if (i < SentenceLUT->numactors)
		{
			pstream->dataPeriod = SND_BLOCK_TIME / (SentenceLUT->compbitrate[i] / 8);
		}
		else
		{
			pstream->dataPeriod = SND_BLOCK_TIME / (SentenceLUT->compbitrate[0] / 8);
		}
		
		speechchannels[i].status = SOUND_FREE;
	}

	if (printfunction != NULL)
	{
		debugfunction = printfunction;
	}
	
	streamStartThread();

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
udword soundstreamopenfile(char *pszStreamFile, sdword *handle)
{
	filehandle streamfile;
    udword identifier;
	udword checksum;
	udword flags;

	/* Check Data dir on HD... */
	flags = 0;
    if (!fileExists(pszStreamFile, flags))
    {
		/* Check first CDROM drive... */
		flags = FF_CDROM;
		if (!fileExists(pszStreamFile, flags))
		{
			/* Death, destruction and general chaos! Should never happen (taken care of in utyGameSystemsInit()...*/
			dbgFatalf(DBG_Loc, "Homeworld CD not detected.");
		}
    }

	streamfile = fileOpen(pszStreamFile, flags);
	*handle = streamfile;

#if VCE_BACKWARDS_COMPATIBLE
    fileBlockRead(streamfile, &identifier, sizeof(identifier));//read in an identifier
    if (identifier == ID_STREAM_DATA)
    {                                                       //if it's 'DATA'
        ssOldFormatVCE = TRUE;
    }
#else
	fileSeek(streamfile, 4, FS_Start);
#endif

	fileBlockRead(streamfile, &checksum, sizeof(checksum));

	fileSeek(streamfile, 0, FS_Start);

	return (checksum);
}


/*-----------------------------------------------------------------------------
	Name		: soundstreamcreatebuffer
	Description	:
	Inputs		: pstreambuffer: pointer to a buffer allocated by the game
				  size: the size of streambuffer
	Outputs		:
	Return		: the handle to the stream
----------------------------------------------------------------------------*/	
sdword soundstreamcreatebuffer(void *pstreambuffer, sdword size, uword bitrate)
{
	sdword			channel = 0;
	CHANNEL			*pchan;
	STREAM			*pstream;
	sdword			i;

    for (i = 0; i < numstreams; i++)
	{
		if (streams[i].status == SOUND_STREAM_FREE)
		{
			pchan = &speechchannels[i];
			pstream = &streams[i];
			channel = i;
			pstream->status = SOUND_STREAM_INUSE;
			pstream->buffersize = SOUND_STREAM_BUFFER_SIZE;
			pstream->buffer = pstreambuffer;
			pstream->playing = FALSE;
			memset(pstream->buffer, 0, pstream->buffersize);
			
			fqAcModel(NULL, NULL, 0, pstream->delaybuffer1, DELAY_BUF_SIZE, &(pstream->delaypos1));
			fqAcModel(NULL, NULL, 0, pstream->delaybuffer2, DELAY_BUF_SIZE, &(pstream->delaypos2));
			break;
		}
	}

	if (pstream == NULL)
	{
		SNDreleasebuffer(pchan);
	}

	if (pstream->buffersize % ((bitrate >> 3) * 4))
	{
		pstream->buffersize /= ((bitrate >> 3) * 4);
		pstream->buffersize *= ((bitrate >> 3) * 4);
	}
	pchan->currentpos = pchan->freqdata = (sbyte *)pstream->buffer;
	pchan->endpos = (sbyte *)(pstream->buffer + pstream->buffersize);
	pchan->fqsize = FQ_HSIZE;
	pstream->writepos = pstream->buffer;
	pstream->readblock = 0;
	pstream->writeblock = 0;
	pstream->blocksize = pstream->buffersize / 2;

	pchan->handle = SNDcreatehandle(channel);
	pstream->handle = pchan->handle;

    return (pchan->handle);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundstreamnumqueued(sdword streamhandle)
{
#if 1
	if (speechchannels[SNDchannel(streamhandle)].status <= SOUND_INUSE)
	{
		return (0);
	}
	else
	{
		return (1);
	}
#endif
	return (streams[SNDchannel(streamhandle)].numqueued);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
void soundstreamstopall(real32 fadetime)
{
	sdword i;

	for (i = 0; i < numstreams; i++)
	{
		soundstreamvolume(speechchannels[i].handle, SOUND_VOL_AUTOSTOP, fadetime);
//		streams[i].status = SOUND_STREAM_INUSE;
//		streams[i].queueindex = 0;
//		streams[i].writeindex = 0;
//		streams[i].playindex = 0;
//		speechchannels[i].status = SOUND_FREE;
	}
}


sdword soundstreamover(sdword streamhandle)
{
	sdword channel;

	if (!soundinited)
	{
		return (TRUE);
	}
	
	if (streamer.status != SOUND_PLAYING)
	{
		return (TRUE);
	}
	
	channel = SNDchannel(streamhandle);

	if (channel >= SOUND_OK)
	{
		if (speechchannels[SNDchannel(streamhandle)].status <= SOUND_STOPPED)
		{
			return (TRUE);
		}
	}
	return (FALSE);
}


sdword soundstreamfading(sdword streamhandle)
{
	sdword channel;

	if (!soundinited)
	{
		return (FALSE);
	}
	
	if (streamer.status != SOUND_PLAYING)
	{
		return (FALSE);
	}
	
	channel = SNDchannel(streamhandle);

	if (channel >= SOUND_OK)
	{
		if (speechchannels[channel].volfade != 0.0f)
		{
			return (TRUE);
		}
	}
	return (FALSE);
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		: actornum - actor index of event we're queueing up
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
sdword soundstreamqueuePatch(sdword streamhandle, sdword filehandle, sdword offset, udword flags, sword vol, sword pan, sword numchannels, sword bitrate, EFFECT *peffect, STREAMEQ *pEQ, STREAMDELAY *pdelay, void *pmixpatch, sdword level, real32 silence, real32 fadetime, sdword actornum, sdword speechEvent, bool bWait)
{
	sdword chan;
	STREAM	*pstream;
	PATCH *ppatch;
	STREAMQUEUE *pqueue;

	// make sure the damn offset is valid.
#ifdef salfreds
	if (!(flags & SOUND_FLAGS_QUEUESILENCE))
	{
		dbgAssert(offset >= 0);
	}
#endif

	if (streamer.status == SOUND_STOPPING)
	{
		return (SOUND_ERR);
	}

	chan = SNDchannel(streamhandle);

	dbgAssert(chan >= 0);
	pstream = &streams[chan];
	if (pstream->handle != streamhandle)
	{
		/* hmmm, bad.  do something */
		return (SOUND_ERR);
	}

	if ((pstream->queueindex == pstream->playindex) && (pstream->status > SOUND_STREAM_INUSE))
	{
		return (SOUND_ERR);
	}

	if ((flags & SOUND_FLAGS_QUEUESTREAM) || (flags & SOUND_FLAGS_QUEUESILENCE))
	{
		if (pstream->numqueued < (SOUND_MAX_STREAM_QUEUE - 1))
		{
			pqueue = &pstream->queue[pstream->queueindex];
			pqueue->offset = offset;
			pqueue->flags = flags;
			pqueue->vol = vol;
			pqueue->pan = pan;
			pqueue->numchannels = numchannels;
			pqueue->bitrate = bitrate;
			pqueue->fadetime = fadetime;
			pqueue->volfactorL = (real32)vol / SOUND_VOL_MAX;
			pqueue->volfactorR = (real32)vol / SOUND_VOL_MAX;
			pqueue->silencetime = silence;
            pqueue->actornum = actornum;
            pqueue->speechEvent = speechEvent;

			if (pmixpatch != NULL)
			{
				pqueue->pmixPatch = (PATCH *)pmixpatch;
				pqueue->mixLevel = level;
			}
			else
			{
				pqueue->pmixPatch = NULL;
				pqueue->mixLevel = SOUND_VOL_MIN;
			}
			pqueue->mixHandle = SOUND_DEFAULT;

			if (pan < SOUND_PAN_CENTER)
			{
				/* panned left so attenuate right */
				pqueue->volfactorR *= (real32)(SOUND_PAN_RIGHT + pan) / SOUND_PAN_RIGHT;
			}
			else if (pan > SOUND_PAN_CENTER)
			{
				/* panned right so attenuate left */
				pqueue->volfactorL *= (real32)(SOUND_PAN_RIGHT - pan) / SOUND_PAN_RIGHT;
			}
	
			if (pEQ != NULL)
			{
				pqueue->eq = pEQ;
			}
			else
			{
				pqueue->eq = NULL;
			}
	
			if (pdelay != NULL)
			{
				pqueue->delay = pdelay;
			}
			else
			{
				pqueue->delay = NULL;
			}
	
			if (peffect != NULL)
			{
				pqueue->effect = peffect;
			}
			else
			{
				pqueue->effect = NULL;
			}

			pstream->numqueued++;
			pstream->numtoplay++;
			pstream->queueindex++;
			if (pstream->queueindex >= SOUND_MAX_STREAM_QUEUE)
			{
				pstream->queueindex = 0;
			}
			pqueue->fhandle = filehandle;

			if ((pstream->status == SOUND_STREAM_INUSE) && !bWait)
			{
				pstream->blockstatus[0] = 0;
				pstream->blockstatus[1] = 0;
				pstream->readblock = 0;
				pstream->writeblock = 0;
				pstream->writepos = 0;
				pstream->status = SOUND_STREAM_STARTING;
			}
		}
	}
	else if (flags & SOUND_FLAGS_QUEUEPATCH)
	{
		if (pstream->numqueued < (SOUND_MAX_STREAM_QUEUE - 1))
		{
			ppatch = SNDgetpatch((void *)filehandle, offset);

			if (ppatch != NULL)
			{
				pqueue = &pstream->queue[pstream->queueindex];
				pqueue->offset = (sdword)ppatch;
				pqueue->flags = flags;
				pqueue->vol = vol;
				pqueue->pan = pan;
				pqueue->numchannels = numchannels;
				pqueue->bitrate = ppatch->bitrate;
				pqueue->fadetime = 0.0f;
				pqueue->volfactorL = (real32)vol / SOUND_VOL_MAX;
				pqueue->volfactorR = (real32)vol / SOUND_VOL_MAX;
				pqueue->pmixPatch = NULL;
				pqueue->mixLevel = SOUND_VOL_MIN;
				pqueue->mixHandle = SOUND_DEFAULT;
                pqueue->actornum = actornum;
				
				if (pan < SOUND_PAN_CENTER)
				{
					/* panned left so attenuate right */
					pqueue->volfactorR *= (real32)(SOUND_PAN_RIGHT + pan) / SOUND_PAN_RIGHT;
				}
				else if (pan > SOUND_PAN_CENTER)
				{
					/* panned right so attenuate left */
					pqueue->volfactorL *= (real32)(SOUND_PAN_RIGHT - pan) / SOUND_PAN_RIGHT;
				}
		
				if (pEQ != NULL)
				{
					pqueue->eq = pEQ;
				}
				else
				{
					pqueue->eq = NULL;
				}
		
				if (pdelay != NULL)
				{
					pqueue->delay = pdelay;
				}
				else
				{
					pqueue->delay = NULL;
				}
		
				if (peffect != NULL)
				{
					pqueue->effect = peffect;
				}
				else
				{
					pqueue->effect = NULL;
				}

				pstream->numqueued++;
				pstream->numtoplay++;
				pstream->queueindex++;
				if (pstream->queueindex >= SOUND_MAX_STREAM_QUEUE)
				{
					pstream->queueindex = 0;
				}
				pqueue->fhandle = filehandle;
				
				if ((pstream->status == SOUND_STREAM_INUSE) && !bWait)
				{
					pstream->blockstatus[0] = 0;
					pstream->blockstatus[1] = 0;
					pstream->readblock = 0;
					pstream->writeblock = 0;
					pstream->writepos = 0;
					pstream->status = SOUND_STREAM_STARTING;
				}
			}
		}
	}

	return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
	Name		: soundstreamvolume
	Description	:
	Inputs		: handle - the handle to a sound returned by soundplay
				  vol - the volume to set this sound to (range of SOUND_MIN_VOL - SOUND_MAX_VOL)
	Outputs		:
	Return		: SOUND_OK if successful, SOUND_ERR on error
----------------------------------------------------------------------------*/	
sdword soundstreamvolume(sdword handle, sword vol, real32 fadetime)
{
    HRESULT hr = SOUND_ERR;
	CHANNEL *pchan;
	sdword channel;
	bool stop = FALSE;
	sdword fadeblocks = -1;
	STREAM *pstream;
	sdword i;

	if (!soundinited)
	{
		return (SOUND_ERR);
	}
	
	if (streamer.status != SOUND_PLAYING)
	{
		return (SOUND_ERR);
	}

	if (vol > SOUND_VOL_MAX)
	{
		vol = SOUND_VOL_MAX;
	}
	else if (vol == -1)
	{
		vol = SOUND_VOL_MIN;
		stop = TRUE;
	}
	else if (vol <= SOUND_VOL_MIN)
	{
		vol = SOUND_VOL_MIN;
	}

	channel = SNDchannel(handle);

	if (channel < 0)
	{
		return (SOUND_ERR);
	}

	pchan = &speechchannels[channel];

	if (pchan != NULL)
	{
		fadeblocks = (sdword)(fadetime * SOUND_FADE_TIMETOBLOCKS);

		if (fadeblocks < NUM_FADE_BLOCKS)
		{
			fadeblocks = NUM_FADE_BLOCKS;
		}

		if ((vol != pchan->voltarget) || stop)
		{
			if (vol == (sword)pchan->volume)
			{
				pchan->voltarget = vol;
				pchan->volticksleft = 1;
				pchan->volfade = 0.0f;
			}
			else if (fadeblocks > 0)
			{
				pchan->voltarget = vol;
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
			else if (fadeblocks == -1)
			{
				pchan->voltarget = vol;
				pchan->volfade = 0.01f;
				pchan->volticksleft = ((sword)pchan->volume - vol) * 100;
				if (pchan->volticksleft < 0)
				{
					pchan->volticksleft *= -1;
				}
			}
			else
			{
				pchan->volume = vol;
				pchan->voltarget = vol;
				pchan->volticksleft = 1;
				pchan->volfade = 0.0f;
			}
			
			if (stop)
			{
				pchan->status = SOUND_STOPPING;
				/* need to clean up the queue */
				pstream = &streams[channel];
				pstream->writeindex = pstream->playindex;
                if (pstream->status == SOUND_STREAM_INUSE)
				{
					pstream->queueindex = pstream->writeindex;
				}
				else
				{
					pstream->queueindex = pstream->writeindex + 1;
					if (pstream->queueindex >= SOUND_MAX_STREAM_QUEUE)
					{
						pstream->queueindex = 0;
					}
				}
//				pstream->queue[pstream->writeindex].fhandle = SOUND_ERR;
				pstream->numqueued = 0;
				for (i = 0; i < SOUND_MAX_STREAM_QUEUE; i++)
				{
					if (i != pstream->playindex)
					{
						pstream->queue[i].fhandle = SOUND_ERR;
					}
				}

			}
		}
	}

	return (SOUND_OK);
}

/*-----------------------------------------------------------------------------
    Name        : ssSubtitleRead
    Description : Read in a subtitle and pass it on to the subtitle code.
    Inputs      : handle - handle of the file to read from, queued up to the
                    start of the subtitle
                  actornum - index of actor or -1 if there is no actor
                  speechEvent - what speechEvent this was
                  dataPeriod - inverse of data rate, in seconds/byte
    Outputs     : header - header for the data of the speech phrase.  We need
                    to read this here to determine the length of the phrase.
    Return      : length of subtitle info read or -1 if there is no subtitle
----------------------------------------------------------------------------*/
sdword ssSubtitleRead(STREAMHEADER *header, filehandle handle, sdword actornum, sdword speechEvent, real32 dataPeriod)
{
    sdword length, length2;
    char subTitle[SUB_SubtitleLength];
    real32 time;
#if VCE_BACKWARDS_COMPATIBLE
    STREAMHEADER header2, headerTemp;
    udword currentOffset;
#endif //VCE_BACKWARDS_COMPATIBLE

    length = fileBlockRead(handle, header, sizeof(STREAMHEADER));//read in the "INFO" and length
    dbgAssert(length == sizeof(STREAMHEADER));
    if (header->ID != ID_STREAM_INFO)
    {                                                       //if we didn't read "INFO"
        //!!!dbgAssert(header->ID == ID_STREAM_DATA);            //it must have been "DATA"
        if (header->ID != ID_STREAM_DATA)
        {
            return(sizeof(STREAMHEADER));                   //skip the subtitle
        }
#if VCE_BACKWARDS_COMPATIBLE
        if (ssOldFormatVCE)
        {                                                   //if it's an old-format file
            currentOffset = fileSeek(handle, 0, FS_Current);//get current location in file
            fileSeek(handle, header->size, FS_Current);     //seek to end of data to read in the info
            length2 = fileBlockRead(handle, &header2, sizeof(STREAMHEADER));//read in the "INFO" and length
            if (header2.ID != ID_STREAM_INFO)
            {                                               //if didn't find data
                fileSeek(handle, currentOffset, FS_Start);  //seek back to start of data
            }
            else
            {                                               //it did find the info
                headerTemp = *header;
                *header = header2;
                goto foundInfo;
            }
        }
#endif //VCE_BACKWARDS_COMPATIBLE
        return(sizeof(STREAMHEADER));                       //skip the subtitle
    }
#if VCE_BACKWARDS_COMPATIBLE
foundInfo:;
#endif
    dbgAssert(header->size >= 0);
    dbgAssert(header->size < SUB_SubtitleLength);

    length = fileBlockRead(handle, subTitle, header->size);  //read in the actual subtitle
    dbgAssert(length == header->size);
    subTitle[header->size] = 0;                              //NULL-terminate the string (it's not terminated in the file)

    //now read in the "DATA" header
#if VCE_BACKWARDS_COMPATIBLE
    if (ssOldFormatVCE)
    {                                                       //if it's old format
        fileSeek(handle, currentOffset, FS_Start);          //seek back to start of data
        *header = headerTemp;                               //restore the header
        length = -length2;                                  //so neither of these will be reported in the return length
    }
    else
    {
        length2 = fileBlockRead(handle, header, sizeof(STREAMHEADER));
    }
#endif
    dbgAssert(length2 == sizeof(STREAMHEADER));
    //!!!dbgAssert(header->ID == ID_STREAM_DATA);

    time = (real32)header->size * dataPeriod;
    if (actornum >= 0 && length > 0)
    {
        if (actornum == 0)
        {                                                   //all single-player voices are on the same channel
            if (bitTest(speechEvent, ACTOR_FLEETCOMMAND_FLAG))
            {
                actornum = STA_FleetCommand;
            }
            else if (bitTest(speechEvent, ACTOR_FLEETINTEL_FLAG))
            {
                actornum = STA_FleetIntel;
            }
            else if (bitTest(speechEvent, ACTOR_TRADERS_FLAG))
            {
                actornum = STA_Traders;
            }
            else if (bitTest(speechEvent, ACTOR_PIRATES2_FLAG))
            {
                actornum = STA_Pirates2;
            }
            else if (bitTest(speechEvent, ACTOR_ALLSHIPSENEMY_FLAG))
            {
                actornum = STA_AllEnemyShips;
            }
            else if (bitTest(speechEvent, ACTOR_AMBASSADOR_FLAG))
            {
                actornum = STA_Ambassador;
            }
            else if (bitTest(speechEvent, ACTOR_NARRATOR_FLAG))
            {
                actornum = STA_Narrator;
            }
            else if (bitTest(speechEvent, ACTOR_DEFECTOR_FLAG))
            {
                actornum = STA_Defector;
            }
            else if (bitTest(speechEvent, ACTOR_EMPEROR_FLAG))
            {
                actornum = STA_Emperor;
            }
            else if (bitTest(speechEvent, ACTOR_KHARSELIM_FLAG))
            {
                actornum = STA_KharSelim;
            }
            else
            {
                //dbgFatalf(DBG_Loc, "Invalid actor 0x%x with speech event 0x%x", actornum, speechEvent);
                ;//just assume fleet command!!!is this valid?
            }
        }
        subTitleAdd(actornum, speechEvent, subTitle, length, time);
    }
    return(length + length2 + sizeof(STREAMHEADER));
}

sword soundstreamgetvol(sdword handle)
{
	sdword channel;
	
	if (!soundinited)
	{
		return (SOUND_ERR);
	}
	
	if (streamer.status != SOUND_PLAYING)
	{
		return (SOUND_ERR);
	}

	channel = SNDchannel(handle);

	if (channel >= SOUND_OK)
	{
		return ((sword)speechchannels[channel].volume);
	}
	return (SOUND_ERR);
}


sdword isoundstreamreadheader(STREAM *pstream)
{
	sdword ret, length, i;
	STREAMQUEUE *pqueue;
	PATCH *ppatch;

	pqueue = &pstream->queue[pstream->writeindex];

	if (pqueue->fhandle == SOUND_ERR)
	{
		if (pstream->numqueued == 1)
		{
			for (i = 0; i < SOUND_MAX_STREAM_QUEUE; i++)
			{
				if (pstream->queue[i].fhandle > 0)
				{
					pstream->writeindex = i;
					pstream->playindex = i;
					pstream->queueindex = i + 1;
					if (pstream->queueindex >= SOUND_MAX_STREAM_QUEUE)
					{
						pstream->queueindex = 0;
					}
					pqueue = &pstream->queue[pstream->writeindex];
					break;
				}
			}
			if (i == SOUND_MAX_STREAM_QUEUE)
			{
				pstream->dataleft = 0;
				pstream->numqueued = 0;
				return (-4);	//(SOUND_ERR);
			}
		}
		else
		{
			pstream->dataleft = 0;
			pstream->numqueued--;
			if (pstream->numqueued < 0)
			{
				pstream->numqueued = 0;
			}
			return (-1);	//(SOUND_ERR);
		}
	}

	if (pqueue->flags & SOUND_FLAGS_QUEUESTREAM)
	{
		ret = fileSeek(pqueue->fhandle, pqueue->offset, FS_Start);	//pstream->lastpos);
		if (ret != pqueue->offset)
		{
			/* yuck, bad */
			return (-2);	//(SOUND_ERR);
		}
        //read in the subTitle, if there is any
        length = ssSubtitleRead(&pstream->header, pqueue->fhandle, pqueue->actornum, pqueue->speechEvent, pstream->dataPeriod); //read the subtitle

        pstream->lastpos = ret + length;
		
		if (pstream->header.ID != ID_STREAM_DATA)
		{
			/* this is not stream data */
			dbgMessage("\nsoundstreamreadheader: bad ID");
            return (-3);	//(SOUND_ERR);
		}

		// need to assign a size to this item
		pqueue->size = pstream->header.size;

		if (pqueue->pmixPatch != NULL)
		{
			pqueue->mixHandle = splayMUTE((void *)pqueue->pmixPatch, SOUND_FLAGS_PATCHPOINTER, pqueue->pan, SOUND_PRIORITY_STREAM, pqueue->mixLevel);
		}
	}
	else if (pqueue->flags & SOUND_FLAGS_QUEUEPATCH)
	{
		ppatch = (PATCH *)pqueue->offset;
		pstream->header.ID = ppatch->id;
		pstream->header.size = ppatch->datasize - ppatch->dataoffset;
		pstream->lastpos = ppatch->dataoffset;
		
		// need to assign a size to this item
		pqueue->size = pstream->header.size;
	}
	else if (pqueue->flags & SOUND_FLAGS_QUEUESILENCE)
	{
		pstream->header.ID = 0;
		pstream->header.size = (sdword)(pqueue->silencetime * SOUND_FADE_TIMETOBLOCKS * (pqueue->bitrate >> 3));  // * bitrate?
		pstream->lastpos = 0;
	
		if (pqueue->pmixPatch != NULL)
		{
			pqueue->mixHandle = splayMUTE((void *)pqueue->pmixPatch, SOUND_FLAGS_PATCHPOINTER, pqueue->pan, SOUND_PRIORITY_STREAM, pqueue->mixLevel);
		}
	}
		
	pstream->dataleft = pstream->header.size;
	pstream->numqueued--;
	
	return (SOUND_OK);
}


sdword isoundstreamreadblock(STREAMQUEUE *pqueue, void *buffer, sdword position, sdword size)
{
	sdword ret = SOUND_ERR;

	if (pqueue->fhandle == -1)
	{
		return (-1);
	}

	if (pqueue->flags & SOUND_FLAGS_QUEUESTREAM)
	{
		/* read a block of data */
		ret = fileSeek(pqueue->fhandle, position, FS_Start);
		if (ret != position)
		{
			/* yuck, bad */
			return (-2);
		}
		
		ret = fileBlockRead(pqueue->fhandle, buffer, size);
		if (ret != size)
		{
			/* yuck, bad */
			dbgMessagef("\nsoundstreamupdate95: couldn't read file block.");
			return (-3);
		}
	}
	else if (pqueue->flags & SOUND_FLAGS_QUEUEPATCH)
	{
		memcpy((sbyte *)buffer, (sbyte *)position, size);
		ret = size;
	}
	else if (pqueue->flags & SOUND_FLAGS_QUEUESILENCE)
	{
//		memset((sbyte *)buffer, 0, size);
		ret = size;
	}

	return (ret);
}


void isoundstreamcleanup(void)
{
	sdword i;

	/* clean up all the streams */
	for (i = 0; i < numstreams; i++)
	{
		streams[i].status = SOUND_STREAM_INUSE;
		streams[i].buffersize = SOUND_STREAM_BUFFER_SIZE;
		memset(streams[i].buffer, 0, SOUND_STREAM_BUFFER_SIZE);
		speechchannels[i].currentpos = speechchannels[i].freqdata = (sbyte *)streams[i].buffer;
		streams[i].writepos = streams[i].buffer;
		streams[i].readblock = 0;
		streams[i].writeblock = 0;
	}
}


/*-----------------------------------------------------------------------------
	Name		:
	Description	:
	Inputs		:
	Outputs		:
	Return		:
----------------------------------------------------------------------------*/	
void isoundstreamupdate(void *dummy)
{
	sdword i;
	STREAM *pstream;
	CHANNEL *pchan;
	sdword ret;
	sdword readsize;
	BYTE *bufferpos;
	sdword bufsize;
	STREAMQUEUE *pqueue;
	
	if (streams == NULL)
	{
		/* not inited yet */
		return;
	}

	while (soundinited && (streamer.status >= SOUND_STOPPED))
	{
		if (streamer.status == SOUND_STOPPING)
		{
			/* check and see if its done yet */
			if (streamer.timeout <= mixerticks)
			{
				isoundstreamcleanup();
				streamer.timeout = 0;
				streamer.status = SOUND_STOPPED;
			}
		}
		
		if (bSoundDeactivated && (streamer.status == SOUND_PLAYING)  && !bSoundPaused)
		{
//			isoundstreamcleanup();
//			streamer.status = SOUND_STOPPED;
			streamer.status = SOUND_PAUSED;
		}
		
		if ((streamer.status >= SOUND_PLAYING) && (streamer.status < SOUND_PAUSED))
		{
			if (bSoundPaused && (streamer.status == SOUND_PLAYING))
			{
				streamer.status = SOUND_STOPPING;
			}
			
			for (i = 0; i < numstreams; i++)
			{
				pstream = &streams[i];
				pchan = &speechchannels[i];
				pqueue = &pstream->queue[pstream->writeindex];
		
				if (pstream->status == SOUND_STREAM_STARTING)
				{
					pstream->playindex = pstream->writeindex;

					ret = isoundstreamreadheader(pstream);
					if (ret != SOUND_OK)
					{
						// bad
						/* dbgMessagef("\nsoundstreamupdate95: couldn't read stream header.");
						pqueue->fhandle = SOUND_ERR;
						dbgMessagef("\nisoundstreamreadheader returned %d\n", ret);
						dbgAssert(FALSE); */
						goto Recover;
					}
					else
					{
						if ((pstream->readblock != pstream->writeblock) && (pstream->blockstatus[pstream->readblock] == 0))
						{
							pstream->writeblock = pstream->readblock;
						}
						pstream->status = SOUND_STREAM_WRITING;
						pchan->volfactorL = pqueue->volfactorL;
						pchan->volfactorR = pqueue->volfactorR;
						
						pchan->voltarget = pqueue->vol;
						if (pqueue->fadetime != 0.0f)
						{
							pchan->volume = 0.0f;
							pchan->volticksleft = (sdword)(pqueue->fadetime * SOUND_FADE_TIMETOBLOCKS);
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
						else
						{
							pchan->volume = (real32)pqueue->vol;
							pchan->volticksleft = 0;
							pchan->volfade = 0.0f;
						}
	
						pchan->pan = pqueue->pan;
						pchan->numchannels = pqueue->numchannels;
						pchan->bitrate = pqueue->bitrate;
						pchan->amountread = 0;
						
						bufsize = SOUND_STREAM_BUFFER_SIZE;
						
						if (bufsize % ((pchan->bitrate >> 3) * 8))
						{
							bufsize /= ((pchan->bitrate >> 3) * 8);
							bufsize *= ((pchan->bitrate >> 3) * 8);
						}
						pstream->buffersize = bufsize;
						pstream->blocksize = pstream->buffersize / 2;
						pchan->endpos = (sbyte *)(pstream->buffer + pstream->buffersize);
						pchan->currentpos = (sbyte *)pstream->buffer;
						
						pstream->numtoplay--;
						pchan->status = SOUND_PLAYING;
					}
				}
		
				if (pstream->status == SOUND_STREAM_WRITING)
				{
					if (pstream->blockstatus[pstream->writeblock] == 0)
					{
						if (pstream->dataleft == 0)
						{	/* HMMM, WE'RE OUT OF DATA HERE */
							if (pqueue->flags & SOUND_FLAGS_LOOPING)
							{
								/* LOOKS LIKE A LOOPER, BACK TO THE START YOU GO */
								isoundstreamreadheader(pstream);
							}
							else
							{
								/* IS THERE ANOTHER STREAM IN THE HOUSE? */
								pqueue->fhandle = SOUND_ERR;
								
								pqueue->nextplay = pstream->buffer + (pstream->blocksize * pstream->writeblock);
								
								pstream->writeindex++;
					
								if (pstream->writeindex >= SOUND_MAX_STREAM_QUEUE)
								{
									pstream->writeindex = 0;
								}
								
								pqueue = &pstream->queue[pstream->writeindex];
		
								if (pqueue->fhandle != SOUND_ERR)
								{
									isoundstreamreadheader(pstream);
								}
								else
								{
									pstream->status = SOUND_STREAM_INUSE;
									pstream->queueindex = 0;
									pstream->writeindex = 0;
									pstream->numqueued = 0;
									pchan->status = SOUND_STOPPING;
									continue;
								}
							}
						}
		
						if (pstream->dataleft >= pstream->blocksize)
						{
							/* WE HAVE LOTS OF DATA HERE */
							ret = isoundstreamreadblock(pqueue, (void *)(pstream->buffer + (pstream->blocksize * pstream->writeblock)), pstream->lastpos, pstream->blocksize);
							
							if (ret != pstream->blocksize)
							{
								/* yuck, bad */
								/* dbgMessagef("\nsoundstreamupdate95: couldn't read file block.");*/
								goto Recover;
							}
				
							pstream->lastpos += ret;
							pstream->dataleft -= pstream->blocksize;
						}
						else
						{
							/* HMMM, GETTING KINDA LOW, NEED A TOP UP */
							ret = isoundstreamreadblock(pqueue, (void *)(pstream->buffer + (pstream->blocksize * pstream->writeblock)), pstream->lastpos, pstream->dataleft);
							
							if (ret != pstream->dataleft)
							{
								/* yuck, bad */
								/* dbgMessagef("\nsoundstreamupdate95: couldn't read file block."); */
								goto Recover;
							}
							pstream->lastpos += ret;
		
							if (!(pqueue->flags & SOUND_FLAGS_LOOPING))
							{
								/* IS THERE ANOTHER STREAM IN THE HOUSE? */
								pqueue->fhandle = SOUND_ERR;
								
								pqueue->nextplay = pstream->buffer + (pstream->blocksize * pstream->writeblock) + pstream->dataleft;
								
								pstream->writeindex++;
					
								if (pstream->writeindex >= SOUND_MAX_STREAM_QUEUE)
								{
									pstream->writeindex = 0;
								}
								
								pqueue = &pstream->queue[pstream->writeindex];
							}
							
							readsize = pstream->blocksize - pstream->dataleft;
							bufferpos = pstream->buffer + (pstream->blocksize * pstream->writeblock) + pstream->dataleft;
							pstream->dataleft = 0;
				
							if (pqueue->fhandle != SOUND_ERR)
							{
								/* LOOKS LIKE WE HAVE ANOTHER STREAM HERE */
								isoundstreamreadheader(pstream);
		
								if (pstream->dataleft >= readsize)
								{
									/* read a block of data */
									ret = isoundstreamreadblock(pqueue, (void *)(bufferpos), pstream->lastpos, readsize);
									
									if (ret != readsize)
									{
										/* yuck, bad */
									}
									pstream->lastpos += ret;
									pstream->dataleft -= readsize;
								}
								else
								{
									ret = isoundstreamreadblock(pqueue, (void *)(bufferpos), pstream->lastpos, pstream->dataleft);
									
									if (ret != pstream->dataleft)
									{
										/*bad*/
										/* dbgAssert(FALSE); */
										goto Recover;
									}
		
									if (pstream->writeindex + 1 >= SOUND_MAX_STREAM_QUEUE)
									{
										if (pstream->queue[0].fhandle != SOUND_ERR)
										{
											memset(bufferpos + pstream->dataleft, 0, readsize - pstream->dataleft);
										}
										else
										{
											pstream->writepos = bufferpos + pstream->dataleft;
										}
									}
									else
									{
										if (pstream->queue[pstream->writeindex+1].fhandle != SOUND_ERR)
										{
											memset(bufferpos + pstream->dataleft, 0, readsize - pstream->dataleft);
										}
										else
										{
											pstream->writepos = bufferpos + pstream->dataleft;
										}
									}
									pstream->lastpos += ret;
									pstream->dataleft = 0;
								}
							}
							else
							{
								/* NO MORE STREAMS FOR NOW, BYE, BYE */
								pstream->writepos = bufferpos;
								pstream->status = SOUND_STREAM_INUSE;
								pstream->queueindex = 0;
								pstream->writeindex = 0;
								pstream->numqueued = 0;
								pchan->status = SOUND_STOPPING;
							}
						}
						pstream->blockstatus[pstream->writeblock++] = 1;
						if (pstream->writeblock >= 2)
						{
							pstream->writeblock = 0;
						}
					}
				}

				continue;

Recover:
				{
					/* IS THERE ANOTHER STREAM IN THE HOUSE? */
					pqueue->fhandle = SOUND_ERR;
					
					pqueue->nextplay = pstream->buffer + (pstream->blocksize * pstream->writeblock);
					
					pstream->writeindex++;
		
					if (pstream->writeindex >= SOUND_MAX_STREAM_QUEUE)
					{
						pstream->writeindex = 0;
					}
					
					pqueue = &pstream->queue[pstream->writeindex];

					if (pqueue->fhandle != SOUND_ERR)
					{
						isoundstreamreadheader(pstream);
					}
					else
					{
						pstream->status = SOUND_STREAM_INUSE;
						pstream->queueindex = 0;
						pstream->writeindex = 0;
						pstream->numqueued = 0;
						pchan->status = SOUND_STOPPING;
						continue;
					}
				}
			}
		}
		else if (streamer.status == SOUND_STOPPED)
		{
			/* mixer is paused so don't do nothing */
			if (!bSoundPaused)	// && (!bSoundDeactivated))
			{
				streamer.status = SOUND_PLAYING;
			}
		}
		else if (streamer.status == SOUND_PAUSED)
		{
			if (!bSoundDeactivated)
			{
				streamer.status = SOUND_PLAYING;
			}
		}
		
		Sleep(SOUND_STREAM_SLEEP);
	}
	
	/* clean up all the streams */
    for (i = 0; i < numstreams; i++)
	{
		streams[i].status = SOUND_STREAM_INUSE;
		streams[i].buffersize = SOUND_STREAM_BUFFER_SIZE;
		memset(streams[i].buffer, 0, SOUND_STREAM_BUFFER_SIZE);
		speechchannels[i].currentpos = speechchannels[i].freqdata = (sbyte *)streams[i].buffer;
		streams[i].writepos = streams[i].buffer;
		streams[i].readblock = 0;
		streams[i].writeblock = 0;
	}

	streamer.status = SOUND_FREE;

	_endthread();
}

