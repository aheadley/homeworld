/*=============================================================================
    Name    : SpeechEvent.c
    Purpose : Speech Event processor

    Created 10/01/1998 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <strings.h>
#include "SoundEventPrivate.h"
#include "SoundEvent.h"
#include "Select.h"
#include "Blobs.h"
#include "soundcmn.h"
#include "NIS.h"
#include "SoundMusic.h"
#include "fquant.h"
#include "Chatting.h"
#include "SinglePlayer.h"
/*#include "bink.h"*/
#include "utility.h"

#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif

#define STATE_BREAK     3

#define MAX_BATTLES     20

#define Asteroids       0
#define Nebulae         1
#define GasClouds       2
#define DustClouds      3

#define NISSTREAM       0
#define AMBIENTSTREAM   1

#define STINGER_FADE_TIME   0.5f
#define MUSIC_FADE_FAST     0.5f
#define MUSIC_FADE_SLOW     1.0f

#define MUSIC_MIN_FADE  80

#define SPEECH_FLEET        0
#define SPEECH_STRIKECRAFT  1
#define SPEECH_CAPSHIPS     2
#define SPEECH_CONSTRUCTION 3
#define SPEECH_INTEL        4
#define SPEECH_PILOT1       1
#define SPEECH_PILOT2       2
#define SPEECH_PILOT3       3

#define NUM_SPEAKERS        5

#define SE_MAX_QUEUE        10

#define SPEECH_PAUSE        1.5f

#define MAX_RELIC_NAMES     33

#define SPEECH_SUPERNOVA_LEVEL  10

/*=============================================================================
    Structures
=============================================================================*/
typedef struct
{
    sdword      offset;
    sword       flags;
    sword       bitrate;
    ubyte       volume;
    ubyte       numchannels;
    sword       pad;
} MUSICSTREAM;

typedef struct
{
    sdword      ID;
    udword      checksum;
    sdword      numstreams;
    MUSICSTREAM mstreams[];
} MUSICHEADER;

#if 1
typedef struct
{
    sdword      handle;     // this is the handle for this event (might not be unique if linked)
    sdword      index;      // this is the index of linked events
    sdword      status;

    sdword      actornum;   // need this for linked speech events

    sdword      numlinks;


    Ship        *pShip;     // ship pointer associated with this event
    sdword      event;      // actual event, listed in speechevent.h
    sdword      variable;   // optional variable for this event
    sdword      variation;  // overriding variation to play
    uword       priority;   // priority of this event
    sword       volume;     // overriding volume

    real32      timein;     // at what time was this event put in the queue?
    real32      timeout;    // at what point is this going to timeout at?
} QUEUEEVENT;

typedef struct
{
    QUEUEEVENT  queue[SE_MAX_QUEUE];    // these are the queued events
    QUEUEEVENT  current;    // the event that is currently playing

    sdword      status;     // status of this speech channel
    sdword      nextevent;  // index into the queue of the next event

    bool        locked;     // set when deleting a bunch of stuff so another event doesn't play

    real32      timeover;   // what time will this event be over (approx)

    sdword      numqueued;

    sdword      lastpriority;
    sdword      laststarttick;
    sdword      lastendtick;
    sdword      lastevent;
    Ship        *lastship;
} SPEECHQUEUE;

typedef struct
{
    sdword      handle;
    sdword      status;
    sdword      tracknum;
    sdword      nexttrack;
    sword       targetvol;
    sword       pad;
} MUSICINFO;
#endif


/*=============================================================================
    Prototypes
=============================================================================*/

void speechPlayQueue(SPEECHQUEUE *pSQueue, sdword streamchannel);
sdword SEselectsentence(sdword actor, sdword event, sdword variable, sdword setvariation, sdword **pOffsets, udword *pDuration);
void speechQueueUpdate(void);
sdword SEreorderqueue(SPEECHQUEUE *pSQueue);
sdword SEcleanqueue(SPEECHQUEUE *pSQueue);


/*=============================================================================
    Private Data
=============================================================================*/

SPEECHQUEUE speechqueue[SE_NUM_ACTORS];
sdword  streamhandle[SE_NUM_ACTORS];
sdword  speechfilehandle;

sdword  numSinglePlayerEvents;

bool    MoShipMoving[MAX_MULTIPLAYER_PLAYERS] = {FALSE};

ubyte   *pspeechstream;

ubyte   *pspeechbuffer0;
ubyte   *pspeechbuffer1;
ubyte   *pspeechbuffer2;
ubyte   *pspeechbuffer3;
ubyte   *pspeechbuffer4;

Ship    *lastshiptospeak = NULL;
sdword  lastgrouptospeak = -1;

/* speech lookup tables */
SENTENCELUT *SentenceLUT;
PHRASELUT   *PhraseLUT;

/* special fx vars */
STREAMDELAY streamdelay[NUM_SPEAKERS];
STREAMEQ    streamEQ[NUM_SPEAKERS];
EFFECT      cleaneffect[NUM_SPEAKERS];
EFFECT      mixedeffect[NUM_SPEAKERS];
EFFECT      damageeffect;


/* music variables */
MUSICINFO   musicinfo[2];
sdword  musicstreamhandle[2];
sdword  musicfilehandle;
ubyte   *pmusicbuffer0;
ubyte   *pmusicbuffer1;
MUSICHEADER *musicheader;
sdword  levelTrack = -1;

char relicPlayerNames[MAX_RELIC_NAMES][MAX_PERSONAL_NAME_LEN] = {
    "Rupract",
    "SpankMaster3000",
    "SM3K",
    "Plaz",
    "dav",
    "Briscola",
    "EFMagnet",
    "Midnight",
    "Quinn_Deadly",
    "YourMammaStinks",
    "Wintermute",
    "Chimpden",
    "Lunarboy",
    "Unspacey",
    "Antonalogue",
    "=A=",
    "B1FF",
    "Hootchi",
    "rhithyn",
    "Psyco-Kitty",
    "Rasolf",
    "GargleTroth",
    "Flako",
    "Yonderboy",
    "DrakeBird",
    "TheDoctor",
    "Tender",
    "Killroy",
    "Rocketboy",
    "TheEmporersLover",
    "DrMagneto",
    "DrTongue",
    "NoWhere2Camp"
};

/* name of music header file */
#if defined(CGW)
char musicHeaderName[] = "CGW_Music.wxh";
#elif defined(Downloadable)
char musicHeaderName[] = "DL_Music.wxh";
#elif defined(DLPublicBeta)
char musicHeaderName[] = "PB_Music.wxh";
#elif defined(OEM)
char musicHeaderName[] = "OEM_Music.wxh";
#else
char musicHeaderName[] = "HW_Music.wxh";
#endif

/* externs */
extern bool soundpaused;

/* global volume settings */
extern real32 volSpeech;
extern real32 volMusic;
extern real32 volNIS;
extern bool bVolMusicNoFade;

/* global speech settings */
extern bool bActorOn[];
extern bool bCommandsOn;
extern bool bStatusOn;
extern bool bChatterOn;

extern sdword actorFlagsEnabled;

extern real32 nearestShipDistance;
extern real32 musicVolFactor;
extern real32 battleMusicVolFactor;
extern real32 musicInactiveFactor;
extern bool nearestShipMoving;

extern bool FalkosFuckedUpTutorialFlag;

/*=============================================================================
    Scripts
=============================================================================*/

scriptStructEntry streamEQScriptTable[] =
{
    { "flags", scriptSetSdwordCB, (udword)&streamEQ[0].flags, (udword)&streamEQ[0] },
    { "eq0", scriptSetReal32CB, (udword)&streamEQ[0].eq[0], (udword)&streamEQ[0] },
    { "eq1", scriptSetReal32CB, (udword)&streamEQ[0].eq[1], (udword)&streamEQ[0] },
    { "eq2", scriptSetReal32CB, (udword)&streamEQ[0].eq[2], (udword)&streamEQ[0] },
    { "eq3", scriptSetReal32CB, (udword)&streamEQ[0].eq[3], (udword)&streamEQ[0] },
    { "eq4", scriptSetReal32CB, (udword)&streamEQ[0].eq[4], (udword)&streamEQ[0] },
    { "eq5", scriptSetReal32CB, (udword)&streamEQ[0].eq[5], (udword)&streamEQ[0] },
    { "eq6", scriptSetReal32CB, (udword)&streamEQ[0].eq[6], (udword)&streamEQ[0] },
    { "eq7", scriptSetReal32CB, (udword)&streamEQ[0].eq[7], (udword)&streamEQ[0] },
    endEntry
};

scriptStructEntry streamDelaySciptTable[] =
{
    { "flags", scriptSetSdwordCB, (udword)&streamdelay[0].flags, (udword)&streamdelay[0] },
    { "level", scriptSetReal32CB, (udword)&streamdelay[0].level, (udword)&streamdelay[0] },
    { "duration", scriptSetUdwordCB, (udword)&streamdelay[0].duration, (udword)&streamdelay[0] },
    { "eq0", scriptSetReal32CB, (udword)&streamdelay[0].eq[0], (udword)&streamdelay[0] },
    { "eq1", scriptSetReal32CB, (udword)&streamdelay[0].eq[1], (udword)&streamdelay[0] },
    { "eq2", scriptSetReal32CB, (udword)&streamdelay[0].eq[2], (udword)&streamdelay[0] },
    { "eq3", scriptSetReal32CB, (udword)&streamdelay[0].eq[3], (udword)&streamdelay[0] },
    { "eq4", scriptSetReal32CB, (udword)&streamdelay[0].eq[4], (udword)&streamdelay[0] },
    { "eq5", scriptSetReal32CB, (udword)&streamdelay[0].eq[5], (udword)&streamdelay[0] },
    { "eq6", scriptSetReal32CB, (udword)&streamdelay[0].eq[6], (udword)&streamdelay[0] },
    { "eq7", scriptSetReal32CB, (udword)&streamdelay[0].eq[7], (udword)&streamdelay[0] },
    endEntry
};

scriptStructEntry streamEffectScriptTable[] =
{
    { "ClockCount", scriptSetUdwordCB, (udword)&cleaneffect[0].nClockCount, (udword)&cleaneffect[0] },
    { "FilterMinFreq", scriptSetUdwordCB, (udword)&cleaneffect[0].nFiltMinFreq, (udword)&cleaneffect[0] },
    { "FilterMaxFreq", scriptSetUdwordCB, (udword)&cleaneffect[0].nFiltMaxFreq, (udword)&cleaneffect[0] },
    { "ToneMinFreq", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneMinFreq, (udword)&cleaneffect[0] },
    { "ToneMaxFreq", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneMaxFreq, (udword)&cleaneffect[0] },
    { "ToneDuration", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneDur, (udword)&cleaneffect[0] },
    { "ToneMute", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneMute, (udword)&cleaneffect[0] },
    { "ToneCount", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneCount, (udword)&cleaneffect[0] },
    { "BreakMaxRate", scriptSetUdwordCB, (udword)&cleaneffect[0].nBreakMaxRate, (udword)&cleaneffect[0] },
    { "BreakMaxDur", scriptSetUdwordCB, (udword)&cleaneffect[0].nBreakMaxDur, (udword)&cleaneffect[0] },
    { "QNoiseMaxRate", scriptSetUdwordCB, (udword)&cleaneffect[0].nQNoiseMaxRate, (udword)&cleaneffect[0] },
    { "QNoiseMaxDur", scriptSetUdwordCB, (udword)&cleaneffect[0].nQNoiseMaxDur, (udword)&cleaneffect[0] },
    { "ScaleLevel", scriptSetReal32CB, (udword)&cleaneffect[0].fScaleLev, (udword)&cleaneffect[0] },
    { "NoiseLevel", scriptSetReal32CB, (udword)&cleaneffect[0].fNoiseLev, (udword)&cleaneffect[0] },
    { "ToneLevel", scriptSetReal32CB, (udword)&cleaneffect[0].fToneLev, (udword)&cleaneffect[0] },
    { "LimitLevel", scriptSetReal32CB, (udword)&cleaneffect[0].fLimitLev, (udword)&cleaneffect[0] },
    { "PitchShift", scriptSetReal32CB, (udword)&cleaneffect[0].fPitchShift, (udword)&cleaneffect[0] },
    endEntry
};


/*=============================================================================
    Functions
=============================================================================*/
void SEloadscripts(void)
{
    sdword i;

#ifdef _WIN32
#define SFX_SCRIPT_PATH "SoundFX\\Scripts\\"
#else
#define SFX_SCRIPT_PATH "SoundFX/Scripts/"
#endif

    scriptSetStruct(SFX_SCRIPT_PATH, "eq_fleet.script", streamEQScriptTable, (ubyte *)&streamEQ[SPEECH_FLEET]);
    scriptSetStruct(SFX_SCRIPT_PATH, "eq_intel.script", streamEQScriptTable, (ubyte *)&streamEQ[SPEECH_INTEL]);
    scriptSetStruct(SFX_SCRIPT_PATH, "eq_pilot1.script", streamEQScriptTable, (ubyte *)&streamEQ[SPEECH_PILOT1]);
    scriptSetStruct(SFX_SCRIPT_PATH, "eq_pilot2.script", streamEQScriptTable, (ubyte *)&streamEQ[SPEECH_PILOT2]);
    scriptSetStruct(SFX_SCRIPT_PATH, "eq_pilot3.script", streamEQScriptTable, (ubyte *)&streamEQ[SPEECH_PILOT3]);

    // set last unsused eq band to 1
    streamEQ[SPEECH_FLEET].eq[8]=1.0F;
    streamEQ[SPEECH_INTEL].eq[8]=1.0F;
    streamEQ[SPEECH_PILOT1].eq[8]=1.0F;
    streamEQ[SPEECH_PILOT2].eq[8]=1.0F;
    streamEQ[SPEECH_PILOT3].eq[8]=1.0F;

    scriptSetStruct(SFX_SCRIPT_PATH, "delay_fleet.script", streamDelaySciptTable, (ubyte *)&streamdelay[SPEECH_FLEET]);
    scriptSetStruct(SFX_SCRIPT_PATH, "delay_intel.script", streamDelaySciptTable, (ubyte *)&streamdelay[SPEECH_INTEL]);
    scriptSetStruct(SFX_SCRIPT_PATH, "delay_strikecraft.script", streamDelaySciptTable, (ubyte *)&streamdelay[SPEECH_STRIKECRAFT]);
    scriptSetStruct(SFX_SCRIPT_PATH, "delay_capships.script", streamDelaySciptTable, (ubyte *)&streamdelay[SPEECH_CAPSHIPS]);
    scriptSetStruct(SFX_SCRIPT_PATH, "delay_construction.script", streamDelaySciptTable, (ubyte *)&streamdelay[SPEECH_CONSTRUCTION]);

    // set last unsused eq band to 1
    streamdelay[SPEECH_FLEET].eq[8]=1.0F;
    streamdelay[SPEECH_INTEL].eq[8]=1.0F;
    streamdelay[SPEECH_STRIKECRAFT].eq[8]=1.0F;
    streamdelay[SPEECH_CAPSHIPS].eq[8]=1.0F;
    streamdelay[SPEECH_CONSTRUCTION].eq[8]=1.0F;

    scriptSetStruct(SFX_SCRIPT_PATH, "effect_fleet.script", streamEffectScriptTable, (ubyte *)&cleaneffect[SPEECH_FLEET]);
    scriptSetStruct(SFX_SCRIPT_PATH, "effect_intel.script", streamEffectScriptTable, (ubyte *)&cleaneffect[SPEECH_INTEL]);
    scriptSetStruct(SFX_SCRIPT_PATH, "effect_strikecraft.script", streamEffectScriptTable, (ubyte *)&cleaneffect[SPEECH_STRIKECRAFT]);
    scriptSetStruct(SFX_SCRIPT_PATH, "effect_capships.script", streamEffectScriptTable, (ubyte *)&cleaneffect[SPEECH_CAPSHIPS]);

    scriptSetStruct(SFX_SCRIPT_PATH, "effect_damage.script", streamEffectScriptTable, (ubyte *)&damageeffect);

    /* copy the unchanging effects to the mixed effect */
    for (i = 0;  i < SE_NUM_ACTORS; i++)
    {
        mixedeffect[i].nFiltMinFreq = cleaneffect[i].nFiltMinFreq;
        mixedeffect[i].nFiltMaxFreq = cleaneffect[i].nFiltMaxFreq;
        mixedeffect[i].fScaleLev = cleaneffect[i].fScaleLev;
        mixedeffect[i].fLimitLev = cleaneffect[i].fLimitLev;
    }

#undef SFX_SCRIPT_PATH
}


void SEinitdelayEQ(void)
{
    sdword i;

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        streamdelay[i].flags = STREAM_FLAGS_NOEFFECT;
        streamdelay[i].level = 0.0f;
        streamdelay[i].duration = 0;
        memset(streamdelay[i].eq, 0, SOUND_EQ_SIZE * sizeof(real32));
//      streamEQ[i].flags = STREAM_FLAGS_NOEFFECT;
//      memset(streamEQ[i].eq, 0, SOUND_EQ_SIZE * sizeof(real32));
#if 0  /* fq*() functions not yet implemented... */
        fqInitE(&cleaneffect[i]);
        fqInitE(&mixedeffect[i]);
#endif
    }
#if 0  /* fq*() functions not yet implemented... */
    fqInitE(&damageeffect);
#endif
}


void soundEventDebugPrint(char *pszInformation)
{
#if SE_VERBOSE_LEVEL >= 1
    dbgMessagef("\n%s", pszInformation);
#endif
}


sdword speechEventInit(void)
{
    sdword buffersize;
    sdword streamersize;
    char loadfile[100];
    sdword i, j;
    
#ifdef ENDIAN_BIG
    sdword p, z;
#endif

#if 0
    sdword numoffsets;
    sdword index;
#endif

    /* load the speech lookup tables */
    strcpy(loadfile, SOUNDFXDIR);
#ifdef CGW
    strcat(loadfile, "CGWsentence.lut");
#elif defined(Downloadable) || defined(DLPublicBeta)
    strcat(loadfile, "DLsentence.lut");
#else
    strcat(loadfile, "speechsentence_comp.lut");
#endif
    fileLoadAlloc(loadfile, (void**)&SentenceLUT, NonVolatile);

#ifdef ENDIAN_BIG
	SentenceLUT->ID         = LittleLong( SentenceLUT->ID );
	SentenceLUT->checksum   = LittleLong( SentenceLUT->checksum );
	SentenceLUT->numcolumns = LittleShort( SentenceLUT->numcolumns );
	SentenceLUT->numevents  = LittleShort( SentenceLUT->numevents );
	SentenceLUT->numactors  = LittleShort( SentenceLUT->numactors );
#if defined(Downloadable) || defined(DLPublicBeta)
	SentenceLUT->compbitrate[3] = LittleShort( SentenceLUT->compbitrate[3] );
#else
	SentenceLUT->compbitrate[0] = LittleShort( SentenceLUT->compbitrate[0] );
	SentenceLUT->compbitrate[1] = LittleShort( SentenceLUT->compbitrate[1] );
	SentenceLUT->compbitrate[2] = LittleShort( SentenceLUT->compbitrate[2] );
	SentenceLUT->compbitrate[3] = LittleShort( SentenceLUT->compbitrate[3] );
#endif // Downloadable
	for ( z=0; z< (SentenceLUT->numcolumns*(SentenceLUT->numactors*SentenceLUT->numevents)); z++){
		SentenceLUT->lookup[z] = LittleShort( SentenceLUT->lookup[z] );
	}
#endif // ENDIAN_BIG

    strcpy(loadfile, SOUNDFXDIR);
#ifdef CGW
    strcat(loadfile, "CGWphrase.lut");
#elif defined(Downloadable) || defined(DLPublicBeta)
    strcat(loadfile, "DLphrase.lut");
#else
    strcat(loadfile, "speechphrase_comp.lut");
#endif
    fileLoadAlloc(loadfile, (void**)&PhraseLUT, NonVolatile);

#ifdef ENDIAN_BIG
	PhraseLUT->ID = LittleLong( PhraseLUT->ID );
	PhraseLUT->checksum = LittleLong( PhraseLUT->checksum );
	PhraseLUT->numcolumns = LittleShort( PhraseLUT->numcolumns );
	PhraseLUT->numsentences = LittleShort( PhraseLUT->numsentences );

	for ( p=0; p< (PhraseLUT->numcolumns*PhraseLUT->numsentences); p++){
		PhraseLUT->lookupsy[p] = LittleShort( PhraseLUT->lookupsy[p] );
	}
#endif

    /* verify sentence and phrase luts */
    if (SentenceLUT->checksum != PhraseLUT->checksum)
    {
        dbgMessage("Sentence and Phrase lookup tables do not match.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

#if 0
    for (i = 0; i < PhraseLUT->numsentences; i++)
    {
        numoffsets = PhraseLUT->lookup[(i * PhraseLUT->numcolumns) + 2];

        if (numoffsets > 20)
        {
            index = i;
#if SE_VERBOSE_LEVEL >= 1
            dbgMessagef("\nnumoffsets %d", numoffsets);
#endif
            break;
        }
    }

    for (i = 0; i < SentenceLUT->prioritylevels * SentenceLUT->numevents * SentenceLUT->numactors; i++)
    {
        if (index == SentenceLUT->lookup[(i*3)+2])
        {
#if SE_VERBOSE_LEVEL >= 1
            dbgMessagef("\nfound it at %d", i);
#endif
        }
    }
#endif
    soundstreamquery(SE_NUM_STREAMS, &buffersize, &streamersize);

    if (buffersize == 0)
    {
        memFree(SentenceLUT);
        memFree(PhraseLUT);
        /* soundstreamquery failed */
        return (SOUND_ERR);
    }

    pspeechstream = memAlloc(streamersize, "StreamStructures", NonVolatile);

    soundstreaminit(pspeechstream, streamersize, SE_NUM_STREAMS, soundEventDebugPrint);

    /* moved voice file from sfxdir to root */
    //  strcpy(loadfile, SOUNDFXDIR);

    /* open speech file and compare checksums */
    strcpy(loadfile,utyVoiceFilename);
    if (soundstreamopenfile(loadfile, &speechfilehandle) != SentenceLUT->checksum)
    {
        dbgMessage("Voice file does not match lookup tables.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    pspeechbuffer0 = memAlloc(buffersize, "SpeechBuffer0", NonVolatile);
        streamhandle[0] = soundstreamcreatebuffer(pspeechbuffer0, buffersize, SentenceLUT->compbitrate[0]);
    pspeechbuffer1 = memAlloc(buffersize, "SpeechBuffer1", NonVolatile);
        streamhandle[1] = soundstreamcreatebuffer(pspeechbuffer1, buffersize, SentenceLUT->compbitrate[1]);
    pspeechbuffer2 = memAlloc(buffersize, "SpeechBuffer2", NonVolatile);
        streamhandle[2] = soundstreamcreatebuffer(pspeechbuffer2, buffersize, SentenceLUT->compbitrate[2]);
#if defined(Downloadable) || defined(DLPublicBeta)
    pspeechbuffer4 = memAlloc(buffersize, "SpeechBuffer4", NonVolatile);
        streamhandle[4] = soundstreamcreatebuffer(pspeechbuffer4, buffersize, SentenceLUT->compbitrate[0]);
#else
    pspeechbuffer3 = memAlloc(buffersize, "SpeechBuffer3", NonVolatile);
        streamhandle[3] = soundstreamcreatebuffer(pspeechbuffer3, buffersize, SentenceLUT->compbitrate[3]);
    pspeechbuffer4 = memAlloc(buffersize, "SpeechBuffer4", NonVolatile);
        streamhandle[4] = soundstreamcreatebuffer(pspeechbuffer4, buffersize, SentenceLUT->compbitrate[0]);
#endif

    /* music stuff */
    /* load the music header file */
    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile,musicHeaderName);
    fileLoadAlloc(loadfile, (void**)&musicheader, NonVolatile);

#ifdef ENDIAN_BIG
	musicheader->ID = LittleLong( musicheader->ID );
	musicheader->checksum = LittleLong( musicheader->checksum );
	musicheader->numstreams = LittleLong( musicheader->numstreams );
#endif

    /* open music file and compare checksums */
    strcpy(loadfile,utyMusicFilename);
    if (soundstreamopenfile(loadfile, &musicfilehandle) != musicheader->checksum)
    {
        dbgMessage("Music data file does not match lookup table.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    pmusicbuffer0 = memAlloc(buffersize, "MusicBuffer", NonVolatile);
        musicinfo[NISSTREAM].handle = soundstreamcreatebuffer(pmusicbuffer0, buffersize, (uword)FQ_BR88);
    pmusicbuffer1 = memAlloc(buffersize, "MusicBuffer", NonVolatile);
        musicinfo[AMBIENTSTREAM].handle = soundstreamcreatebuffer(pmusicbuffer1, buffersize, (uword)FQ_BR88);

    for (i = NISSTREAM; i <= AMBIENTSTREAM; i++)
    {
        musicinfo[i].status = SOUND_FREE;
        musicinfo[i].tracknum = SOUND_DEFAULT;
        musicinfo[i].targetvol = SOUND_DEFAULT;
    }

    SEinitdelayEQ();

    SEloadscripts();

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        memset(&speechqueue[i], 0, sizeof(SPEECHQUEUE));
        speechqueue[i].current.handle = SOUND_NOTINITED;
        speechqueue[i].status = SOUND_STOPPED;
        speechqueue[i].nextevent = SOUND_NOTINITED;
        speechqueue[i].locked = FALSE;
        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            speechqueue[i].queue[j].handle = SOUND_NOTINITED;
            speechqueue[i].queue[j].variation = SOUND_NOTINITED;
        }
    }

    numSinglePlayerEvents = 0;

    return (SOUND_OK);
}


void speechEventClose(void)
{
    memFree(pmusicbuffer0);
    memFree(pmusicbuffer1);
    memFree(musicheader);

    memFree(pspeechstream);
    memFree(pspeechbuffer0);
    memFree(pspeechbuffer1);
    memFree(pspeechbuffer2);
#if defined(Downloadable) || defined(DLPublicBeta)
    memFree(pspeechbuffer4);
#else
    memFree(pspeechbuffer3);
    memFree(pspeechbuffer4);
#endif
    memFree(SentenceLUT);
    memFree(PhraseLUT);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void speechEventUpdate(void)
{
    /* must call this so the streamer gets updated */
    speechQueueUpdate();
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :   this function updates the queues, starts playing new events,
                    reorders and cleans up the queue
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if 1
void speechQueueUpdate(void)
{
    sdword  i;
    SPEECHQUEUE *pSQueue;

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        pSQueue = &speechqueue[i];

        /* is this channel playing something? */
        if (pSQueue->status >= SOUND_PLAYING)
        {
            /* is it over yet? */
            if (soundstreamover(streamhandle[i]))
            {
#if SE_VERBOSE_LEVEL >= 2
                dbgMessagef("\nQueue %d, stopped", i);
#endif
                /* was it a single player event? */
                if (pSQueue->current.event & SPEECH_SINGLEPLAYER_FLAG)
                {
                    numSinglePlayerEvents--;
                }
                if (pSQueue->nextevent >= SOUND_OK)
                {
                    if ((pSQueue->current.event == pSQueue->queue[pSQueue->nextevent].event) &&
                        (pSQueue->current.variable == pSQueue->queue[pSQueue->nextevent].variable))
                    {
                        memset(&pSQueue->queue[pSQueue->nextevent], 0, sizeof(QUEUEEVENT));
                        pSQueue->queue[pSQueue->nextevent].handle = SOUND_NOTINITED;
                        pSQueue->queue[pSQueue->nextevent].timeout = -1.0f;
                    }
                }
                pSQueue->current.event = SOUND_DEFAULT;
                pSQueue->status = SOUND_STOPPED;
                pSQueue->timeover = universe.totaltimeelapsed;
            }
            else
            {
                /* should it be shut off? */
            }
        }

        /* are we ready to play something? */
#if 0   /* For reasons of Bink... */
        if ((pSQueue->status == SOUND_STOPPED) && (pSQueue->nextevent >= SOUND_OK) && (!pSQueue->locked) &&
            (((universe.totaltimeelapsed - pSQueue->timeover) >= SPEECH_PAUSE) || !gameIsRunning || !binkDonePlaying || FalkosFuckedUpTutorialFlag))
#else
        if ((pSQueue->status == SOUND_STOPPED) && (pSQueue->nextevent >= SOUND_OK) && (!pSQueue->locked) &&
            (((universe.totaltimeelapsed - pSQueue->timeover) >= SPEECH_PAUSE) || !gameIsRunning || FalkosFuckedUpTutorialFlag))
#endif
        {
            /* find the event that we should play */

            /* remove it from the queue */
            memcpy(&pSQueue->current, &pSQueue->queue[pSQueue->nextevent], sizeof(QUEUEEVENT));

            memset(&pSQueue->queue[pSQueue->nextevent], 0, sizeof(QUEUEEVENT));
            pSQueue->queue[pSQueue->nextevent].handle = SOUND_NOTINITED;
            pSQueue->queue[pSQueue->nextevent].timeout = -1.0f;

            /* find the next event */

            /* play this one */
            speechPlayQueue(pSQueue, i);
            if (pSQueue->current.event & SPEECH_SINGLEPLAYER_FLAG)
            {
                numSinglePlayerEvents++;
            }
            SEreorderqueue(pSQueue);

            pSQueue->status = SOUND_PLAYING;
        }

        /* find the event that we should play */
        /* remove it from the queue */

        /* reorder the queue */
        /* clean up the queue */

    }

}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void speechPlayQueue(SPEECHQUEUE *pSQueue, sdword streamchannel)
{
    sdword i;
    sdword numoffsets;
    udword duration;
    sdword *phraseoffsets;

    EFFECT *fqeffect = NULL;
    STREAMEQ *pEQ = NULL;
    STREAMDELAY *pdelay = NULL;
    void *ppatch = NULL;
    sword level = 0;
    bool bookend = FALSE;
    ShipClass shipclass = 0;

    real32 vol = (real32)SOUND_VOL_MAX;
    sword pan = SOUND_PAN_CENTER;
    uword bitrate;
    sdword actor;
    sdword event;
    real32 damageratio = 0.0f;
    real32 mult;
    sdword actornum;

    event = pSQueue->current.event;
    actornum = pSQueue->current.actornum;

    /* figure out all the filtering etc */
    if ((event & ACTOR_PILOT_FLAG) && (!nisIsRunning))//gameIsRunning))
    {
        /* this is one of the Pilot actors */

        /* figure out the ship class */
        if (nisIsRunning)
        {
            shipclass = CLASS_Fighter;
        }
        else if (pSQueue->current.pShip != NULL)
        {
            shipclass = pSQueue->current.pShip->staticinfo->shipclass;

            damageratio = 1.0f - ((real32)pSQueue->current.pShip->health * (real32)pSQueue->current.pShip->staticinfo->oneOverMaxHealth);
        }

        /* get the appropriate EQ */
        pEQ = &streamEQ[actornum];

        if (((event & SPEECH_EVENT_MASK) >= (CHAT_Const_DuringBattle & SPEECH_EVENT_MASK)) &&
            ((event & SPEECH_EVENT_MASK) <= (CHAT_Const_More & SPEECH_EVENT_MASK)))
        {
            /* construction has a special delay */
            pdelay = &streamdelay[SPEECH_CONSTRUCTION];
            fqeffect = &cleaneffect[SPEECH_CAPSHIPS];
        }

        if ((shipclass == CLASS_Corvette) || (shipclass == CLASS_Fighter))
        {
            /* get the strike craft effects */
            pdelay = &streamdelay[SPEECH_STRIKECRAFT];

            /* figure out the damage */
            mult = damageratio * SPEECH_STIKEDAMAGE_MULT;

            mixedeffect[SPEECH_STRIKECRAFT].fScaleLev = cleaneffect[SPEECH_STRIKECRAFT].fScaleLev;
            mixedeffect[SPEECH_STRIKECRAFT].fNoiseLev = cleaneffect[SPEECH_STRIKECRAFT].fNoiseLev +
                            ((damageeffect.fNoiseLev - cleaneffect[SPEECH_STRIKECRAFT].fNoiseLev) * mult);
            mixedeffect[SPEECH_STRIKECRAFT].nBreakMaxRate = damageeffect.nBreakMaxRate;     //(udword)((real32)damageeffect.nBreakMaxRate * mult);
            mixedeffect[SPEECH_STRIKECRAFT].nBreakMaxDur = (udword)((real32)damageeffect.nBreakMaxDur - ((real32)damageeffect.nBreakMaxDur * mult));

            if (singlePlayerGameInfo.currentMission == SPEECH_SUPERNOVA_LEVEL)
            {
                /* use the quantize noise */
                mixedeffect[SPEECH_STRIKECRAFT].nQNoiseMaxRate = (udword)(damageeffect.nQNoiseMaxRate * SPEECH_STIKEDAMAGE_MULT);
                mixedeffect[SPEECH_STRIKECRAFT].nQNoiseMaxDur = (udword)(damageeffect.nQNoiseMaxDur * SPEECH_STIKEDAMAGE_MULT);
            }
            else
            {
                mixedeffect[SPEECH_STRIKECRAFT].nQNoiseMaxRate = 0;
                mixedeffect[SPEECH_STRIKECRAFT].nQNoiseMaxDur = 0;
            }

            fqeffect = &mixedeffect[SPEECH_STRIKECRAFT];
        }
        else    // if (shipclass != CLASS_Mothership)
        {
#if 0
            if (SPEECH_AMBIENT_ENABLE)
            {
                /* if we are adding background ambients to speech */
                if ((shipclass >= CLASS_HeavyCruiser) && (shipclass <= CLASS_Frigate))
                {
                    /* need to check if we should play calm or excited */
                    ppatch = SNDgetpatch(SpecialEffectBank, SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_IntCapShipCalm)]);
                }
                else if (shipclass == CLASS_Resource)
                {
                    /* resources play their bg ambient */
                    ppatch = SNDgetpatch(ShipBank, ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, ResourceCollector, ShipCmn_Ambient)]);
                }
            }
#endif

            /* get the capital ship effects */
            pdelay = &streamdelay[SPEECH_CAPSHIPS];

            mult = damageratio * SPEECH_CAPDAMAGE_MULT;

            mixedeffect[SPEECH_CAPSHIPS].fScaleLev = cleaneffect[SPEECH_CAPSHIPS].fScaleLev;
            mixedeffect[SPEECH_CAPSHIPS].fNoiseLev = cleaneffect[SPEECH_CAPSHIPS].fNoiseLev +
                            ((damageeffect.fNoiseLev - cleaneffect[SPEECH_CAPSHIPS].fNoiseLev) * mult);
            mixedeffect[SPEECH_CAPSHIPS].nBreakMaxRate = damageeffect.nBreakMaxRate;    //(udword)((real32)damageeffect.nBreakMaxRate * mult);
            mixedeffect[SPEECH_CAPSHIPS].nBreakMaxDur = (udword)((real32)damageeffect.nBreakMaxDur - ((real32)damageeffect.nBreakMaxDur * mult));   //(udword)(damageeffect.nBreakMaxDur * mult);

            if (singlePlayerGameInfo.currentMission == SPEECH_SUPERNOVA_LEVEL)
            {
                /* use the quantize noise */
                mixedeffect[SPEECH_CAPSHIPS].nQNoiseMaxRate = (udword)(damageeffect.nQNoiseMaxRate * SPEECH_CAPDAMAGE_MULT);
                mixedeffect[SPEECH_CAPSHIPS].nQNoiseMaxDur = (udword)(damageeffect.nQNoiseMaxDur * SPEECH_CAPDAMAGE_MULT);
            }
            else
            {
                mixedeffect[SPEECH_CAPSHIPS].nQNoiseMaxRate = 0;
                mixedeffect[SPEECH_CAPSHIPS].nQNoiseMaxDur = 0;
            }

            /* sound the alarm */
            mixedeffect[SPEECH_CAPSHIPS].nToneMinFreq = damageeffect.nToneMinFreq;
            mixedeffect[SPEECH_CAPSHIPS].nToneMaxFreq = damageeffect.nToneMaxFreq;
            mixedeffect[SPEECH_CAPSHIPS].nToneDur = damageeffect.nToneDur;
            mixedeffect[SPEECH_CAPSHIPS].nToneMute = damageeffect.nToneMute;
            mixedeffect[SPEECH_CAPSHIPS].nToneCount = damageeffect.nToneCount;
            mixedeffect[SPEECH_CAPSHIPS].fToneLev = damageeffect.fToneLev * mult;

            fqeffect = &mixedeffect[SPEECH_CAPSHIPS];
        }

        if (fqeffect != NULL)
        {
            if (event & SPEECH_STATUS_FLAG)
            {
                fqeffect->fScaleLev *= SPEECH_STATUS_RATIO;
            }
            else if (event & SPEECH_CHATTER_FLAG)
            {
                fqeffect->fScaleLev *= SPEECH_CHATTER_RATIO;
            }
        }

        /* play radio bookends */
        bookend = TRUE;
    }
    else
    {
        /* this is a non-pilot actor */

        actor = (event & SPEECH_FLAG_MASK);

        switch (actor)
        {
            case ACTOR_FLEETCOMMAND_FLAG:
#if 0
                if (SPEECH_AMBIENT_ENABLE)
                {
                    ppatch = SNDgetpatch(SpecialEffectBank, SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_FleetBG)]);
                }
#endif
                pEQ = &streamEQ[SPEECH_FLEET];
                pdelay = &streamdelay[SPEECH_FLEET];
                fqeffect = &cleaneffect[SPEECH_FLEET];
                break;

            case ACTOR_FLEETINTEL_FLAG:
#if 0
                if (SPEECH_AMBIENT_ENABLE)
                {
                    ppatch = SNDgetpatch(SpecialEffectBank, SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_IntelBG)]);
                }
#endif
                pEQ = &streamEQ[SPEECH_INTEL];
                pdelay = &streamdelay[SPEECH_INTEL];
                fqeffect = &cleaneffect[SPEECH_INTEL];
                break;

            case ACTOR_ALLSHIPSENEMY_FLAG:
                bookend = TRUE;
                pdelay = &streamdelay[SPEECH_STRIKECRAFT];
                fqeffect = &cleaneffect[SPEECH_STRIKECRAFT];
                fqeffect->fScaleLev = 1.3f;
                break;

            case  ACTOR_PILOT_FLAG:
                if ((streamchannel > 0)  && (streamchannel < SentenceLUT->numactors))
                {
                    bookend = TRUE;
                }
                pdelay = &streamdelay[SPEECH_STRIKECRAFT];
                fqeffect = &cleaneffect[SPEECH_STRIKECRAFT];
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("Pilot\n");
#endif
                break;
            case ACTOR_AMBASSADOR_FLAG:
                pdelay = &streamdelay[SPEECH_STRIKECRAFT];
                fqeffect = &cleaneffect[SPEECH_STRIKECRAFT];
                fqeffect->fScaleLev = 2.5f;
                bookend = TRUE;
                pSQueue->current.actornum = 0;
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("Ambassador\n");
#endif
                break;
            case ACTOR_TRADERS_FLAG:
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("Traders\n");
#endif
                break;
            case ACTOR_PIRATES2_FLAG:
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("Pirates2\n");
#endif
                break;
            case ACTOR_NARRATOR_FLAG:
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("Narrator\n");
#endif
                break;
            case ACTOR_DEFECTOR_FLAG:
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("Defector\n");
#endif
                break;
            case ACTOR_EMPEROR_FLAG:
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("Emperor\n");
#endif
                break;
            case ACTOR_KHARSELIM_FLAG:
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("KharSelim\n");
#endif
                break;

            default:
                if ((event == N01_Chatter_Mid) || (event == N01_Chatter_BG))
                {
                    vol *= 1.5f;
                }
#if SE_VERBOSE_LEVEL >= 2
                dbgMessage("default\n");
#endif
                break;
        }
    }

    /* get the bitrate for this actor */
    if (streamchannel < SentenceLUT->numactors)
    {
        bitrate = SentenceLUT->compbitrate[streamchannel];
    }
    else
    {
        /* the extra channel always plays from actor 0 */
        bitrate = SentenceLUT->compbitrate[0];
    }

    /* get the sentence */
    numoffsets = SEselectsentence(actornum, (event & SPEECH_EVENT_MASK),
                                  pSQueue->current.variable, pSQueue->current.variation, &phraseoffsets, &duration);

    /* scale by the front end volume settings */
    vol *= volSpeech;

    /* if this is not a single player event and there is a single player event playing then turn it down */
    if ((numSinglePlayerEvents > 0) && !(event & SPEECH_SINGLEPLAYER_FLAG))
    {
        vol *= SPEECH_SINGLEPLAYER_RATIO;
    }

    /* scale the background ambience thats mixed into the speech */
    level = (sword)(vol * SPEECH_AMBIENT_LEVEL);

    /* queue the offsets */
    if ((numoffsets > 0) && (vol >= SPEECH_MIN_PERCEPTABLE_VOL))
    {
#if 0
        if (fqeffect != NULL)
        {
            dbgMessagef("\nspeechEvent: %d, channel: %d, volume: %f, scale: %f\n", event & SPEECH_EVENT_MASK, streamchannel, vol, fqeffect->fScaleLev);
        }
        else
        {
            dbgMessagef("\nspeechEvent: %d, channel: %d, volume: %f\n", event & SPEECH_EVENT_MASK, streamchannel, vol);
        }
#endif
        if (bookend && enableSFX)
        {
            soundstreamqueuewait(streamhandle[streamchannel], (sdword)UIBank,
                             (sdword)UIEventsLUT->lookup[GetPatch(UIEventsLUT, 0, UI_RadioBeepStart)],
                             SOUND_FLAGS_QUEUEPATCH, (sword)vol, pan, SOUND_MONO, SOUND_DEFAULT, NULL, NULL, NULL, event);

//          soundstreamqueueSilence(streamhandle[stream], SOUND_FLAGS_QUEUESILENCE, vol, pan, SOUND_MONO,
//                                  SentenceLUT->compbitrate, fqeffect, pEQ, pdelay, SEsilence());
        }

#if SE_VERBOSE_LEVEL >= 2
        dbgMessagef("\nspeechEvent: %d, channel: %d, volume: %d",
                event, streamchannel, vol);
#endif

        for (i = 0; i < numoffsets - 1; i++)
        {
            soundstreamqueuePatch(streamhandle[streamchannel], speechfilehandle, *phraseoffsets++, SOUND_FLAGS_QUEUESTREAM, (sword)vol, pan,
                                  SOUND_MONO, bitrate, fqeffect, pEQ, pdelay, ppatch, level, 0.0f, 0.0f, pSQueue->current.actornum, event, TRUE);
        }

        if (bookend && enableSFX)
        {
            soundstreamqueuePatch(streamhandle[streamchannel], speechfilehandle, *phraseoffsets++, SOUND_FLAGS_QUEUESTREAM, (sword)vol, pan,
                                  SOUND_MONO, bitrate, fqeffect, pEQ, pdelay, ppatch, level, 0.0f, 0.0f, pSQueue->current.actornum, event, TRUE);

//          soundstreamqueueSilence(streamhandle[stream], SOUND_FLAGS_QUEUESILENCE, vol, pan, SOUND_MONO,
//                                  SentenceLUT->compbitrate, fqeffect, pEQ, pdelay, SEsilence());

            soundstreamqueue(streamhandle[streamchannel], (sdword)UIBank,
                             (sdword)UIEventsLUT->lookup[GetPatch(UIEventsLUT, 0, UI_RadioBeepEnd)],
                             SOUND_FLAGS_QUEUEPATCH, (sword)vol, pan, SOUND_MONO, SOUND_DEFAULT, NULL, NULL, NULL, event);
        }
        else
        {
            soundstreamqueuePatch(streamhandle[streamchannel], speechfilehandle, *phraseoffsets++, SOUND_FLAGS_QUEUESTREAM, (sword)vol, pan,
                                  SOUND_MONO, bitrate, fqeffect, pEQ, pdelay, ppatch, level, 0.0f, 0.0f, pSQueue->current.actornum, event, FALSE);
        }

        /* figure out the last ship to speak var */
        if (pSQueue->current.pShip != NULL)
        {
            /* straight forward, this was just a ship */
            lastshiptospeak = pSQueue->current.pShip;
        }
        else if (event & SPEECH_GROUP_FLAG)
        {
            lastshiptospeak = (Ship *)-1;
            lastgrouptospeak = pSQueue->current.variable;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : SERandomSequenceCreate
    Description : Creates a number which is a random BCD sequence numVariations
                    in length containing all numbers between 0 and numVarations
                    once and only once.
    Inputs      : numVariations - number of variations to encode.
                  lookupsy - the lookup table which contains probabilities
                  width - width of the lookup table
    Outputs     :
    Return      : BCD-encoded random series
----------------------------------------------------------------------------*/
udword SERandomSequenceCreate(sdword numVariations, sdword *lookupsy, sdword width)
{
    sdword index, whichIndex;
    udword sequence[16];
    ubyte probs[16], accumProb = 0;
    udword finalSequence;

    dbgAssert(numVariations <= 16);                         //can only fit 16 BCD's in a DWORD
    //make an order 0, 1, 2, 3, ... and a list of matching probabilities
    for (index = 0; index < numVariations; index++)
    {
        sequence[index] = index;
        accumProb += probs[index] = ((lookupinterp *)(lookupsy))->probability - accumProb;
        lookupsy += width;
    }
    //partially bubble sort this list (this will favor the most common variations first)
    for (index = 0; index < numVariations - 1; index++)     //!!! might want to bubble sort a different number of times
    {
        whichIndex = ranRandom(RAN_SoundBothThreads) % (numVariations - 1);
        if (probs[whichIndex] <= probs[whichIndex + 1])
        {
            swap(sequence[whichIndex], sequence[whichIndex + 1], finalSequence);
            swap(probs[whichIndex], probs[whichIndex + 1], accumProb);
        }
    }
    //create the final sequence DWORD
    finalSequence = 0;
    for (index = numVariations - 1; index >= 0; index--)
    {
        finalSequence <<= 4;                                //free up a new BCD digit
        finalSequence = (finalSequence & 0xfffffff0) | (sequence[index] + 1);//stick in the new digit
    }
    return(finalSequence);                                  //done
}

/*-----------------------------------------------------------------------------
    Name        : SENextVariationInSeries
    Description : Select a variation of a speech event based on a random sequence
    Inputs      : numVariations - how many variations of this event
                  lookupsy - the lookup table
                  width - width of the lookup table
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword SENextVariationInSeries(sdword numVariations, sdword *lookupsy, sdword width)
{
    lookupinterp *interp = (lookupinterp *)(lookupsy);
    udword sequence;
    udword iVariation;

#if defined(Downloadable) || defined(DLPublicBeta)
    if (numVariations > 0xf)
    {                                                       //could be the case if this actor is disabled
        return(0);
    }
#endif
    dbgAssert(numVariations < 0xf);
    //get the random sequence into a dword
    sequence = (interp->high[1] << 24) + (interp->high[0] << 16) + (interp->low[1] << 8) + (interp->low[0]);

    if (sequence == 0)
    {                                                       //if no numbers left in sequence (tried all variations)
        sequence = SERandomSequenceCreate(numVariations, lookupsy, width);//start a new sequence
    }
    iVariation = (sequence & 0xf) - 1;                      //get a new variation
    sequence >>= 4;                                         //shift off those 4 bits
    dbgAssert(iVariation < numVariations);
    interp->high[1] = sequence >> 24;                       //put the sequence back in the high words
    interp->high[0] = (sequence >> 16) & 0xff;
    interp->low[1] = (sequence >> 8) & 0xff;
    interp->low[0] = (sequence) & 0xff;
#ifdef lmoloney
#if SE_VERBOSE_LEVEL >= 1
    dbgMessagef("\nSelected variation %d of %d for event at 0x%x of width %d", iVariation, numVariations, lookupsy, width);
#endif
#endif
    return(iVariation);                                     //return the variation we've found
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :   actor - which actor?
                    event - which event?
                    variable - value of the variable for this event
                    setvariation - select a particular variation, -1 is random
    Outputs     :   pOffsets - sets a pointer to the first offset for the sentence, NULL if there is an error
                    pDuration - sets the approximate duration of this sentence
    Return      :   the number of offsets in this sentence, -1 if there is an error
----------------------------------------------------------------------------*/
sdword SEselectsentence(sdword actor, sdword event, sdword variable, sdword setvariation, sdword **pOffsets, udword *pDuration)
{
    uword numvariations;
    uword maxvariable;
    uword index;
    udword numoffsets;
    udword i;
    udword phrase;
    sword *pSentence;
    sdword *pPhrase;
    udword variation;
    static sdword lastBGvariation = 0;
    static sdword lastMIDvariation = 0;

    pSentence = &SentenceLUT->lookup[(actor * SentenceLUT->numevents + event) * SentenceLUT->numcolumns + 1];   // + 1 is to get past the priority

    numvariations = *pSentence++;

    if (numvariations == 0)
    {
        pOffsets = NULL;
        return (SOUND_ERR);
    }

    maxvariable = *pSentence++;
    index = *pSentence;

    if (variable < SOUND_OK)
    {                                                       //no variable specified
        variable = ranRandom(RAN_SoundBothThreads) % maxvariable;//!!! no heuristic on this one
    }
    else if (variable >= maxvariable)
    {                                                       //variable out of range
        pOffsets = NULL;
        return (SOUND_ERR);
    }

    if ((setvariation != SOUND_NOTINITED) && (setvariation < numvariations))
    {                                                       //if variation specified
        variation = setvariation;

        phrase = index + (variation * maxvariable) + variable;
    }
    else if (numvariations == 1)
    {                                                       //not spec'd but only 1 choice
        phrase = index + variable;
    }
    else
    {                                                       //no variation spec'd, choose one intelligently
        if (event == (N01_Chatter_Mid & SPEECH_EVENT_MASK))
        {                                                   //increment the variation if it's mid-ground chatter
            if (++lastMIDvariation >= numvariations)
            {
                lastMIDvariation = 0;
            }
            i = lastMIDvariation;
        }
        else if (event == (N01_Chatter_BG & SPEECH_EVENT_MASK))
        {                                                   //inclrement the variation if it's background chatter
            if (++lastBGvariation >= numvariations)
            {
                lastBGvariation = 0;
            }
            i = lastBGvariation;
        }
        else
        {                                                   //else it's a regular speech event; choose a random variation somehow
            i = SENextVariationInSeries(numvariations, &PhraseLUT->lookupsy[(index + variable) * PhraseLUT->numcolumns], maxvariable * PhraseLUT->numcolumns);
           /*
            for (i = 0; i < numvariations; i++)
            {
                if (PhraseLUT->lookupsy[(index + (i * maxvariable) + variable) * PhraseLUT->numcolumns + 1] >= variation)
                {
                    break;
                }
            }
            if (i >= numvariations)
            {
                i = numvariations - 1;
            }
            */
        }

        phrase = index + (i * maxvariable) + variable;
    }

    pPhrase = &PhraseLUT->lookupsy[phrase * PhraseLUT->numcolumns];

    *pDuration = *pPhrase++;
    pPhrase++;  // skip  the probability
    numoffsets = (*pPhrase++) & 0xff;

    *pOffsets = pPhrase;

    if (numoffsets > 0)
    {
        for (i = 0; i < numoffsets; i++)
        {
            if (*(pPhrase + i) == SOUND_NOTINITED)
            {
                // bad, this won't work
                pOffsets = NULL;
                numoffsets = 0;
                break;
            }
        }
    }

    return (numoffsets);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :   this function updates the actual speech that is playing,
                    decides when to fade volumes or cut off particular events
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void speechUpdateSpeech(void)
{
    /* what is the most important speech? */
    /* should something else be shut off? */
    /* are we past the max # of events that should be playing? */


}


/*-----------------------------------------------------------------------------
    Name        :   speechEventQueue
    Description :   Adds speech events to the queue if there is space for them.  Does trivial rejection on
                    low priority speech events.
    Inputs      :   object - pointer to a ship object that this speech event is attached to (can be NULL for Fleet Command, etc)
                    event - event # to play, define in speechevent.h
                    var - variable that may be attached to an event (ie a build ship event would pass the ship variable #)
                    variation - only used if you want a specific variation of an event to play
                    actornum - used if you want to override the actor that the speech code has selected
                    playernum - the playerindex in the array player in the universe structure, only want to hear your own speech events
                    linkto - handle of the speech event that you want to tie this one to, will play this event directly after that event
                    timeout - amount of time this event can wait in the queue, 0 means play now, < 0 means wait indefinatly and don't drop this from the queue
                    volume - volume at which to play this event, if you want to override the code that figures that out
    Outputs     :
    Return      :   handle to this speech event, can be used to stop the event or link another event to this one
                    or SOUND_ERR (-1) if there is an error or the event can't be queued
----------------------------------------------------------------------------*/
sdword speechEventQueue(void *object, sdword event, sdword var, sdword variation, sdword actornum, sdword playernum, sdword linkto, real32 timeout, sword volume)
{
    sdword actor;
    sdword i, j;
    Ship *pShip = NULL;
    sdword handle = SOUND_ERR;
    SPEECHQUEUE *pSQueue;
    QUEUEEVENT *pQEvent;
    SpaceObj *spaceobject;
    sdword channel = 0;
    sdword ally = SOUND_ERR;

    if (!enableSpeech || soundpaused)
    {
        return (SOUND_ERR);
    }

    if ((event & SPEECH_EVENT_MASK) > SPEECH_LAST_EVENT)
    {
#if SE_VERBOSE_LEVEL >= 1
        dbgMessagef("\nspeechEventQueue: Tutorial speeech event %d, not implemented", event);
#endif
        return (SOUND_ERR);
    }

    /* make sure they're not asking for an actor that doesn't exist */
    if (actornum >= (SentenceLUT->numactors))
    {
        return (handle);
    }

    /* if an NIS is playing, only want to hear NIS events */
//  if (nisIsRunning && (event < SPEECH_FIRST_SP_EVENT))
    if (nisIsRunning && !(event & (SPEECH_NIS_FLAG + SPEECH_ANIMATIC_FLAG)))
    {
        return (handle);
    }

    /* if single player events are happening, don't want any chatter */
    if ((numSinglePlayerEvents > 0) && (event & SPEECH_CHATTER_FLAG))
    {
        return (handle);
    }

    /* toss out events the user doesn't want to hear */
    if ((event & SPEECH_CHATTER_FLAG) && !bChatterOn)
    {
        return (handle);
    }
    else if ((event & SPEECH_STATUS_FLAG) && !bStatusOn)
    {
        return (handle);
    }
    else if ((event & SPEECH_COMMAND_FLAG) && !bCommandsOn)
    {
        return (handle);
    }

#if 1       // enable when new speech is put in game
    if (!((event & SPEECH_FLAG_MASK) & actorFlagsEnabled))
    {
        return (handle);
    }
#endif

    if ((object != NULL) && !(event & ACTOR_FLEETCOMMAND_FLAG))
    {
        /* this is probably a ship, so this is one of the ALL SHIPS voices */
        spaceobject = (SpaceObj *)object;

        if (!univSpaceObjInRenderList(spaceobject) && ((event & SPEECH_STATUS_FLAG) || (event & SPEECH_CHATTER_FLAG)))
        {
            if (!(event & SPEECH_ALWAYSPLAY_FLAG))
            {
                return (SOUND_ERR);        // only do sound events for objects in current mission sphere we are viewing
            }
        }

        if (spaceobject->objtype != OBJ_ShipType)
        {
            return (SOUND_ERR);
        }

        pShip = (Ship *)object;

        if ((universe.curPlayerIndex != pShip->playerowner->playerIndex) && (!nisIsRunning))
        {
            return (SOUND_ERR);
        }

        if ((event == COMM_Selection) || (event == COMM_AssGrp_Select))
        {
            /* Probes and Proximity Sensors are unmanned and shouldn't speak when selected */
            if ((pShip->staticinfo->shiptype == Probe) || (pShip->staticinfo->shiptype == ProximitySensor))
            {
                /* play this sound event instead */
                soundEvent(pShip, Ship_SelectProbe);

                return (SOUND_OK);
            }
            else if ((pShip->staticinfo->maxfuel > 0.0f) && (pShip->fuel <= 0.0f))
            {
                if (event == COMM_Selection)
                {
                    var = SOUND_DEFAULT;
                }
                event = STAT_Strike_OutOfFuel;
            }
        }

        if (actornum < SOUND_OK)
        {
            if ((pShip->soundevent.actorNum <= SOUND_NOTINITED) || (pShip->soundevent.actorNum >= SentenceLUT->numactors))
            {
                actor = pShip->soundevent.actorNum = SEselectactor();
            }
            else
            {
                actor = pShip->soundevent.actorNum;
            }
        }
        else
        {
            actor = actornum;
        }
        channel = actor;
    }
    else
    {
        if (object != NULL)
        {
            spaceobject = (SpaceObj *)object;
            if (spaceobject->objtype == OBJ_ShipType)
            {
                pShip = (Ship *)object;
            }
        }

        if (((udword)playernum != universe.curPlayerIndex) && (!nisIsRunning) && (playernum != SOUND_EVENT_DEFAULT))
        {
            return (SOUND_ERR);
        }

        /* this is either Fleet Command, Fleet Intel, Traders, Pirates or NIS speech*/
//      if ()
//      {
//      }

        if (actornum < SOUND_OK)
        {
            actor = ACTOR_FLEETCOMMAND;
        }
        else
        {
            actor = actornum;
        }

        if (event == N01_Chatter_BG)
        {
            channel = 4;
        }
        else
        {
            channel = actor;
        }
    }

    /* figure out which speech channel to use */
    pSQueue = &speechqueue[channel];

    /* is the queue full? */
    if (pSQueue->numqueued == SE_MAX_QUEUE)
    {
        /* should see if I can bump one of these? */
        return (SOUND_ERR);
    }

    /* is this going to time out before it'll play? */
    if ((timeout > 0.0f) && (pSQueue->timeover > universe.totaltimeelapsed + timeout))
    {
        return (SOUND_ERR);
    }

    if (actor == SOUND_NOTINITED)
    {
        return (handle);
    }

    if ((event == STAT_Strike_OutOfFuel) && (var > SOUND_DEFAULT))
    {
        event = STAT_Group_OutOfFuel;
    }
    else if ((event == STAT_Strike_LowOnFuel) && (var > SOUND_DEFAULT))
    {
        event = STAT_Group_LowOnFuel;
    }

    if (var == SOUND_DEFAULT)
    {
        var = 0;
    }

    if ((pSQueue->current.event == event) && (pSQueue->current.variable == var))
    {
        return (SOUND_ERR);
    }

    /* special relic users events */
    if ((event == COMM_F_AllianceFormed) || (event == COMM_F_AllianceBroken))
    {
        ally = var;
        var = 0;

        for (i = 0; i < MAX_RELIC_NAMES; i++)
        {
            if (strncasecmp(playerNames[ally], relicPlayerNames[i], MAX_PERSONAL_NAME_LEN) == 0)
            {
                if (event == COMM_F_AllianceFormed)
                {
                    event = COMM_F_RelicAllianceFormed;
                }
                else
                {
                    event = COMM_F_RelicAllianceBroken;
                }
                var = i;
                break;
            }
        }
    }
    else if (event == STAT_F_RelicPlayerDies)
    {
        for (i = 0; i < MAX_RELIC_NAMES; i++)
        {
            if (strncasecmp(playerNames[var], relicPlayerNames[i], MAX_PERSONAL_NAME_LEN) == 0)
            {
                var = i;
                break;
            }
        }
        if ((i == MAX_RELIC_NAMES) || (i == universe.curPlayerIndex))
        {
            return (handle);
        }
    }

    if (gameIsRunning)
    {
        if (universe.curPlayerPtr->race == R2)
        {
            if (event == A09_Intel_Kushan)
            {
                event = A09_Intel_Taiidan;
            }
            else if (event == A10_Intel_Kushan)
            {
                event = A10_Intel_Taiidan;
            }
            else if (event == A12_Defector_Kushan)
            {
                event = A12_Defector_Taiidan;
            }
        }
    }

    for (i = 0; i < SE_MAX_QUEUE; i++)
    {
        if (pSQueue->queue[i].handle == SOUND_NOTINITED)
        {
            pQEvent = &pSQueue->queue[i];

            /* put it in this space */
            if (linkto < SOUND_OK)
            {
                handle = SNDcreatehandle(channel);
                pQEvent->index = 0;
            }
            else
            {
                handle = linkto;
                for (j = 0; j < SE_MAX_QUEUE; j++)
                {
                    if ((pSQueue->queue[j].handle == linkto) && (pSQueue->queue[j].index == 0))
                    {
                        break;
                    }
                }
                if (j < SE_MAX_QUEUE)
                {
                    pQEvent->index = pSQueue->queue[j].numlinks++;
                }
                else
                {
                    /* the one you wanted to link to is gone */
                    return (SOUND_ERR);
                }
            }

#if SE_VERBOSE_LEVEL >= 2
            dbgMessagef("Speech Event Queued.  Actor: %d, Event: %d\n", actor, event);
#endif
            pQEvent->status = SOUND_OK;
            pQEvent->actornum = actor;

            if (event & SPEECH_GROUP_FLAG)
            {
                pQEvent->pShip = NULL;
            }
            else
            {
                pQEvent->pShip = pShip;
            }
            pQEvent->event = event;
            pQEvent->variable = var;
            pQEvent->variation = variation;
            pQEvent->priority = SentenceLUT->lookup[(actor * SentenceLUT->numevents + (event & SPEECH_EVENT_MASK)) * SentenceLUT->numcolumns];
            pQEvent->volume = volume;

            pQEvent->timein = universe.totaltimeelapsed;
            if (timeout >= 0.0f)
            {
                pQEvent->timeout = universe.totaltimeelapsed + timeout;
            }
            else
            {
                pQEvent->timeout = timeout;
            }

            pQEvent->handle = handle;

            SEreorderqueue(pSQueue);
            break;
        }
    }

    return (handle);
}


/*-----------------------------------------------------------------------------
    Name        :   speechEventStop
    Description :   stops a speech event that is currently playing and deletes any
                    linked speech events from the queue.
    Inputs      :   handle - the handle for this event that was return by speechEventQueue
                    fadetime - the amount of time to fade out this speech event (in seconds)
                    addstatic - add static noise ass the speech fades out
    Outputs     :
    Return      :   SOUND_OK (0) if successful, SOUND_ERR (-1) if not
----------------------------------------------------------------------------*/
sdword speechEventStop(sdword handle, real32 fadetime, bool addstatic)
{
    /* is this event still playing? */
    /* are there any events linked to this one in the queue? */
    return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
    Name        : speechEventActorStop
    Description : Stops all speech events from a particular actor.  Queued
                    events are discarded; playing events faded out.
    Inputs      : actorMask - bit fields for what actors' speech events to
                    delete.  Same as bit fields added to speech events.
                  fadeTime - amount of time to fade out over.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword speechEventActorStop(udword actorMask, real32 fadetime)
{
    sdword i;

    dbgAssert((actorMask & (~ACTOR_ALL_ACTORS)) == 0);

    for (i = 0; i < SE_MAX_QUEUE; i++)
    {
        if (speechqueue[0].queue[i].event & actorMask)
        {
            memset(&speechqueue[0].queue[i], 0, sizeof(QUEUEEVENT));
            speechqueue[0].queue[i].handle = SOUND_NOTINITED;
            speechqueue[0].queue[i].timeout = -1.0f;
        }
    }

    if (speechqueue[0].current.event & actorMask)
    {
        soundstreamvolume(speechqueue[0].current.handle, -1, fadetime);
    }

    return (SOUND_OK);
}


sdword speechEventStopAllSpecific(real32 fadetime, sdword speechType)
{
    sdword i, j;

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        speechqueue[i].locked = TRUE;
        speechqueue[i].timeover = 0.0f;

        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            if (speechqueue[i].queue[j].event & speechType)
            {
                memset(&speechqueue[i].queue[j], 0, sizeof(QUEUEEVENT));
                speechqueue[i].queue[j].handle = SOUND_NOTINITED;
                speechqueue[i].queue[j].timeout = -1.0f;
            }
        }

        /* need to stop the currently playing one */
        if (speechqueue[i].current.event & speechType)
        {
            soundstreamvolume(speechqueue[i].current.handle, -1, fadetime);
        }

        speechqueue[i].locked = FALSE;
    }

    return (SOUND_OK);
}


sdword speechEventRemoveShip(Ship *pShip)
{
    sdword i, j;

    if (pShip == lastshiptospeak)
    {
        lastshiptospeak = NULL;
    }

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        speechqueue[i].locked = TRUE;

        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            if (speechqueue[i].queue[j].pShip == pShip)
            {
                memset(&speechqueue[i].queue[j], 0, sizeof(QUEUEEVENT));
                speechqueue[i].queue[j].handle = SOUND_NOTINITED;
                speechqueue[i].queue[j].timeout = -1.0f;
                speechqueue[i].queue[j].pShip = NULL;
            }
        }

        /* need to stop the currently playing one */
        if (speechqueue[i].current.pShip == pShip)
        {
            soundstreamvolume(speechqueue[i].current.handle, -1, 100);
            speechqueue[i].current.pShip = NULL;
        }

        speechqueue[i].locked = FALSE;
    }

    return (SOUND_OK);
}


sdword SEreorderqueue(SPEECHQUEUE *pSQueue)
{
    sdword i;
    sdword highpriority = 0;
    bool bSomethingQueued = FALSE;
    /* need to get the next event */

    SEcleanqueue(pSQueue);

    for (i = 0; i < SE_MAX_QUEUE; i++)
    {
        if (pSQueue->queue[i].handle >= SOUND_OK)
        {
            bSomethingQueued = TRUE;
            if (pSQueue->queue[highpriority].handle < SOUND_OK)
            {
                highpriority = i;
            }
            else if ((pSQueue->queue[i].event & SPEECH_TYPE_MASK) > (pSQueue->queue[highpriority].event & SPEECH_TYPE_MASK))
            {
                highpriority = i;
            }
            else if ((pSQueue->queue[i].event & SPEECH_TYPE_MASK) == (pSQueue->queue[highpriority].event & SPEECH_TYPE_MASK))
            {
                if (pSQueue->queue[i].priority > pSQueue->queue[highpriority].priority)
                {
                    highpriority = i;
                }
                else if (pSQueue->queue[i].priority == pSQueue->queue[highpriority].priority)
                {
                    if (pSQueue->queue[i].timein < pSQueue->queue[highpriority].timein)
                    {
                        highpriority = i;
                    }
                }
            }
            else if (pSQueue->queue[i].priority > pSQueue->queue[highpriority].priority)
            {
                highpriority = i;
            }
            else if (pSQueue->queue[i].priority == pSQueue->queue[highpriority].priority)
            {
                if (pSQueue->queue[i].timein < pSQueue->queue[highpriority].timein)
                {
                    highpriority = i;
                }
            }
        }
    }

    if (bSomethingQueued)
    {
        pSQueue->nextevent = highpriority;
    }
    else
    {
        pSQueue->nextevent = SOUND_NOTINITED;
    }

    return (SOUND_OK);
}


sdword SEcleanqueue(SPEECHQUEUE *pSQueue)
{
    sdword i;

    for (i = 0; i < SE_MAX_QUEUE; i++)
    {
        if (pSQueue->queue[i].timeout >= 0.0f)
        {
            if ((pSQueue->queue[i].timeout < universe.totaltimeelapsed)
                && ((pSQueue->queue[i].event & SPEECH_TYPE_MASK) < SPEECH_TUTORIAL_FLAG))   // don't throw out Animatic, NIS, Single Player
            {                                                                               // or Tutorial speech events.
                /* get ride of this one */
                /* IS IT LINKED? */
                memset(&pSQueue->queue[i], 0, sizeof(QUEUEEVENT));
                pSQueue->queue[i].handle = SOUND_NOTINITED;
                pSQueue->queue[i].timeout = -1.0f;
            }
        }
    }
    return (SOUND_OK);
}


sdword speechEventCleanup(void)
{
    sdword i, j;

    numSinglePlayerEvents = 0;
    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            memset(&speechqueue[i].queue[j], 0, sizeof(QUEUEEVENT));
            speechqueue[i].queue[j].handle = SOUND_NOTINITED;
            speechqueue[i].queue[j].timeout = -1.0f;
        }

        if (speechqueue[i].current.handle > SOUND_NOTINITED)
        {
            soundstreamvolume(speechqueue[i].current.handle, -1, 0.5f);
        }

        memset(&speechqueue[i].current, 0, sizeof(QUEUEEVENT));
        speechqueue[i].current.handle = SOUND_NOTINITED;
        speechqueue[i].status = SOUND_STOPPED;
        speechqueue[i].nextevent = SOUND_NOTINITED;
        speechqueue[i].timeover = 0.0f;
        speechqueue[i].numqueued = 0;
    }
    return (SOUND_OK);
}

#else

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword speechEventPlay(void *object, sdword event, sdword var, sdword playernum, sdword variation)
{
#if SPEECH
//    udword i;
    sdword actor = ACTOR_FLEETCOMMAND;
    sword vol = SOUND_VOL_MAX;
    sword pan = SOUND_PAN_CENTER;
    double dist;
    float damageratio = 0.0f;
    float distratio = (float)1.0;
    Ship *ship;
    SpaceObj *spaceobject;
    ShipStaticInfo *shipstatic;
    sdword curstream = SOUND_ERR;
    bool bookend = FALSE;
    ShipClass shipclass = 0;

    if (!enableSpeech)
    {
        return (SOUND_ERR);
    }

    if (event == STAT_Cap_ShipDocked)
    {
        return (SOUND_ERR);
    }

    /* figure out if there is a stream free to be played on */
//    for (i = 0; i < SE_NUM_STREAMS; i++)
//    {
//        if (soundstreamnumqueued(streamhandle[i]) == 0)
//        {
//            curstream = i;
//            break;
//        }
//    }
    if (soundstreamnumqueued(streamhandle[0]) == 0)
    {
        curstream = 0;
    }

    if (curstream == SOUND_ERR)
    {
        if ((event != COMM_Const_BuildCapShip) || (event != STAT_F_Research_CompletedShip))
        {
            return (SOUND_ERR);
        }

        curstream = 0;
    }

    if (object != NULL)
    {
        spaceobject = (SpaceObj *)object;

        if (spaceobject->objtype != OBJ_ShipType)
        {
            return (SOUND_ERR);
        }

        ship = (Ship *)object;
        shipstatic = (ShipStaticInfo *)ship->staticinfo;

        if (universe.curPlayerIndex != ship->playerowner->playerIndex)
        {
            return (SOUND_ERR);
        }

        if (!univSpaceObjInRenderList(spaceobject))
        {
            return (SOUND_ERR);        // only do sound events for objects in current mission sphere we are viewing
        }

//      if (event >= SPEECH_FIRST_SP_EVENT)
        if (event & SPEECH_SINGLEPLAYER_FLAG)
        {
            shipclass = CLASS_Fighter;
        }
        else
        {
            shipclass = ship->staticinfo->shipclass;
        }

        if (shipclass == CLASS_Mothership)
        {
            if (event == COMM_Selection)
            {
                event = COMM_F_MoShip_Selected;
            }
            playernum = universe.curPlayerIndex;
        }
        else
        {
            if ((ship->soundevent.actorNum <= SOUND_NOTINITED) || (ship->soundevent.actorNum >= SentenceLUT->numactors))
            {
                ship->soundevent.actorNum = actor = SEselectactor();
            }
            else
            {
                actor = ship->soundevent.actorNum;
            }

            damageratio = 1.0f - ((real32)ship->health * (real32)shipstatic->oneOverMaxHealth);      // (real32)shipstatic->maxhealth);

            dist = fsqrt(spaceobject->cameraDistanceSquared);

            vol = SOUND_VOL_MAX - (sword)(SPEECH_VOL_FACTOR * dist);
            if (vol < SPEECH_VOL_LOW)
            {
                vol = SPEECH_VOL_LOW;
            }
            else if (vol > SPEECH_VOL_MAX)
            {
                vol = SPEECH_VOL_MAX;
            }

            /* add the digital bleeps */
            bookend = TRUE;
        }
    }

    if (actor == ACTOR_FLEETCOMMAND)
    {
        if ((udword)playernum != universe.curPlayerIndex)
        {
            return (SOUND_ERR);
        }

        if (event == COMM_F_MoShip_Arrived)
        {
            if (MoShipMoving[universe.curPlayerIndex])
            {
                MoShipMoving[universe.curPlayerIndex] = FALSE;
            }
            else
            {
                return (SOUND_ERR);
            }
        }
        vol = SPEECH_VOL_MAX;
    }

    if (SEspeechevent(curstream, actor, event, var, vol, pan, 0.0f, 0.0f, variation, bookend, shipclass) == SOUND_OK)
    {
        /* set the last ship to speak variable so the hot key can be used later */
        if (actor == ACTOR_FLEETCOMMAND)
        {
            /* MAKE LAST SHIP BE THE MOTHERSHIP */
            if (universe.curPlayerPtr != NULL)
            {
                lastshiptospeak = universe.curPlayerPtr->PlayerMothership;
            }
            else
            {
                lastshiptospeak = NULL;
            }
        }
        else
        {
            lastshiptospeak = ship;
        }

    }


    if (event == COMM_Cloak_CloakingOn)
    {
        if (++actor >= SentenceLUT->numactors - 1)
        {
            actor = 1;
        }

        SEspeechevent(curstream, actor, STAT_Cloak_CloakingOn_Resp, var, vol, pan, 0.0f, 0.0f, variation, bookend, shipclass);
    }
    if ((event == STAT_AssGrp_UnderAttack) || (event == STAT_Grp_UnderAttack))
    {
        if (++actor >= SentenceLUT->numactors - 1)
        {
            actor = 1;
        }

        SEspeechevent(curstream, actor, STAT_Grp_UnderAttack_Rsp, var, vol, pan, 0.0f, 0.0f, variation, bookend, shipclass);
    }

    if (event == COMM_F_MoShip_Move)
    {
        MoShipMoving[universe.curPlayerIndex] = TRUE;
    }

    return (streamhandle[curstream]);
#endif
}


sdword SEspeechevent(sdword stream, sdword actor, sdword event, sdword var, sword vol, sword pan, double dist, float damageratio, sdword setVariation, bool bookend, ShipClass shipclass)
{
    udword duration = 0;
    udword variation = 0;
#if SPEECH
    uword numvariations;
    uword maxvar;
    uword index;
    udword numoffsets;
    udword i;
    udword phrase;
    EFFECT *fqeffect = NULL;
    sword priority = 0;
    sword *pSentence;
    sdword *pPhrase;
    STREAMEQ *pEQ = NULL;
    STREAMDELAY *pdelay = NULL;
    void *ppatch = NULL;
    sword level = 0;

// NEW SPEECH
    udword probability;

    pSentence = &SentenceLUT->lookup[(actor * SentenceLUT->numevents + event) * SentenceLUT->numcolumns];

    priority = *pSentence++;
    numvariations = *pSentence++;

#if 0
    for (i = 0; i < SentenceLUT->prioritylevels; i++)
    {
        pSentence = &SentenceLUT->lookup[((actor * SentenceLUT->numevents + event) * SentenceLUT->prioritylevels + i) * 3];
        if (*pSentence > 0)
        {
            numvariations = *pSentence++;
            break;
        }
    }
#endif
    if (numvariations == 0)
    {
        return (SOUND_ERR);
    }

    if (var < SOUND_OK)
    {
        var = 0;
//      return (SOUND_ERR);
    }

    maxvar = *pSentence++;
    index = *pSentence;

    if ((setVariation != SOUND_NOTINITED) && (setVariation < numvariations))
    {
        variation = setVariation;
    }
    else
    {
        variation = ranRandom(RAN_SoundGameThread) % numvariations;
    }

    if (event == COMM_F_Hyper_Engage)
    {
        variation = 1;
    }
    else if (event == STAT_Grav_Collapse)
    {
        variation = 1;
    }

    phrase = index + (variation * maxvar) + var;

    pPhrase = &PhraseLUT->lookup[phrase * PhraseLUT->numcolumns];

    duration = *pPhrase++;
// NEW SPEECH
    probability = *pPhrase++;
    numoffsets = *pPhrase++;

    if (numoffsets > 0)
    {
        for (i = 0; i < numoffsets; i++)
        {
            if (*(pPhrase + i) == SOUND_NOTINITED)
            {
                // bad, this won't work
                numoffsets = 0;
                break;
            }
        }
    }

    /* set up the effect structure */
//    fqInitE(&fqeffect);

    if (actor != ACTOR_FLEETCOMMAND)
    {
        pEQ = &streamEQ[SPEECH_PILOT1];

        if ((event >= CHAT_Const_DuringBattle) && (event <= CHAT_Const_More))
        {
            pdelay = &streamdelay[SPEECH_CONSTRUCTION];
        }

        if ((shipclass == CLASS_Corvette) || (shipclass == CLASS_Fighter))
        {
            /* filter the fighters */
            pdelay = &streamdelay[SPEECH_STRIKECRAFT];
            fqeffect = &cleaneffect[SPEECH_STRIKECRAFT];
//          vol = (sword)(vol * 1.5f);
//          if (vol > SOUND_VOL_MAX)
//          {
//              vol = SOUND_VOL_MAX;
//          }
        }
        if ((shipclass >= CLASS_HeavyCruiser) && (shipclass <= CLASS_Frigate))
        {
            ppatch = SNDgetpatch(SpecialEffectBank, SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_IntCapShipCalm)]);
            level = 128;
        }
        if (shipclass == CLASS_Resource)
        {
            ppatch = SNDgetpatch(ShipBank, ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, ResourceCollector, ShipCmn_Ambient)]);
            level = 128;
        }
#if 0
        /* adjust frequency effects here */
        fqeffect.nFiltMinFreq = SPEECH_FILTER_LOW;
        fqeffect.nFiltMaxFreq = SPEECH_FILTER_HIGH;

        fqeffect.fNoiseLev = SPEECH_NOISE_LOW + ((float)dist * SPEECH_NOISE_FACTOR);    /// (float)150.0;
        if (fqeffect.fNoiseLev > SPEECH_NOISE_HIGH)
        {
            fqeffect.fNoiseLev = SPEECH_NOISE_HIGH;
        }
        fqeffect.fNoiseLev *= FQ_FNOISE;

        if (damageratio > SPEECH_BREAK_THRESHOLD)
        {
            fqeffect.nBreakMaxRate = SPEECH_BREAK_RATE_LOW + (udword)(SPEECH_BREAK_RATE_FACTOR * damageratio);
            if (fqeffect.nBreakMaxRate > SPEECH_BREAK_RATE_HIGH)
            {
                fqeffect.nBreakMaxRate = SPEECH_BREAK_RATE_HIGH;
            }
            fqeffect.nBreakMaxDur = SPEECH_BREAK_LENGTH_LOW + (udword)(SPEECH_BREAK_LENGTH_FACTOR * damageratio);
            if (fqeffect.nBreakMaxDur < SPEECH_BREAK_LENGTH_HIGH)
            {
                fqeffect.nBreakMaxDur = SPEECH_BREAK_LENGTH_HIGH;
            }
        }
#endif
    }
    else
    {
        pEQ = &streamEQ[SPEECH_FLEET];
    }

    if (numoffsets > 0)
    {
        if (bookend && enableSFX)
        {
            soundstreamqueue(streamhandle[stream], (sdword)SpecialEffectBank,
                             (sdword)SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_RadioBeepStart)],
                             SOUND_FLAGS_QUEUEPATCH, vol, pan, SOUND_MONO, SOUND_DEFAULT, NULL, NULL, NULL, pSQueue->current.event);

//          soundstreamqueueSilence(streamhandle[stream], SOUND_FLAGS_QUEUESILENCE, vol, pan, SOUND_MONO,
//                                  SentenceLUT->compbitrate, fqeffect, pEQ, pdelay, SEsilence());
        }

        for (i = 0; i < numoffsets; i++)
        {
//            soundstreamqueue(streamhandle[stream], speechfilehandle, *pPhrase++, 0, vol, pan, SOUND_MONO, SentenceLUT->compbitrate, &fqeffect, pEQ, pdelay);
//            soundstreamqueue(streamhandle[stream], speechfilehandle, *pPhrase++, SOUND_FLAGS_QUEUESTREAM, vol, pan, SOUND_MONO, SentenceLUT->compbitrate, fqeffect, pEQ, pdelay);
            soundstreamqueuePatch(streamhandle[stream], speechfilehandle, *pPhrase++, SOUND_FLAGS_QUEUESTREAM, vol, pan,
                                  SOUND_MONO, SentenceLUT->compbitrate, fqeffect, pEQ, pdelay, ppatch, level, 0.0f, actor);
        }

        if (bookend && enableSFX)
        {
//          soundstreamqueueSilence(streamhandle[stream], SOUND_FLAGS_QUEUESILENCE, vol, pan, SOUND_MONO,
//                                  SentenceLUT->compbitrate, fqeffect, pEQ, pdelay, SEsilence());

            soundstreamqueue(streamhandle[stream], (sdword)SpecialEffectBank,
                             (sdword)SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_RadioBeepEnd)],
                             SOUND_FLAGS_QUEUEPATCH, vol, pan, SOUND_MONO, SOUND_DEFAULT, NULL, NULL, NULL, pSQueue->current.event);
        }
    }
#endif

//    dbgMessagef("\nspeechEvent: %d, variable: %d, variation: %d, duration %d msec, text: %s",
//                event, var, variation, duration, aSpeechEventDebug[event]);

    return (SOUND_OK);
}

#endif

real32 SEsilence(void)
{
    return (0.5f);
}


sdword SEselectactor(void)
{
    sdword actor;

#if defined(Downloadable) || defined(DLPublicBeta)
    actor = (ranRandom(RAN_SoundGameThread) % 2) + 1;
#else
    actor = (ranRandom(RAN_SoundGameThread) % 3) + 1;
#endif

    if (!bActorOn[actor])
    {
        actor++;

#if defined(Downloadable) || defined(DLPublicBeta)
            if (actor > 2)
#else
            if (actor > 3)
#endif
        {
            actor = 1;
        }

        if (!bActorOn[actor])
        {
            actor++;

#if defined(Downloadable) || defined(DLPublicBeta)
            if (actor > 2)
#else
            if (actor > 3)
#endif
            {
                actor = 1;
            }

            if (!bActorOn[actor])
            {
                actor = SOUND_NOTINITED;
            }
        }
    }

    return (actor);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool speechEventAttack(void)
{
    sdword objtype,
            i;
    Ship *ship;
    sdword  numFriendlies = 0,
            numEnemies = 0,
            resourceTypes = 0,
            numResources = 0;
    static bool lastreturn;

    if (keyIsHit(SHIFTKEY))
    {
        for (i = 0; i < selSelecting.numTargets; i++)
        {
            objtype = selSelecting.TargetPtr[i]->objtype;
            if (objtype == OBJ_ShipType)
            {
                ship = (Ship *)selSelecting.TargetPtr[i];
                if (ship->playerowner == universe.curPlayerPtr)
                {
                    numFriendlies++;
                }
                else
                {
                    numEnemies++;
                }
            }
            else if ((objtype >= OBJ_AsteroidType) && (objtype <= OBJ_DustType))
            {
                resourceTypes |= (1 << objtype);
                numResources++;
            }
        }

        // are there any friendlies
        if (numFriendlies > 0)
        {
            // do we attack?
            if (((ranRandom(RAN_SoundGameThread) % 100) <= SPEECH_DISOBEY_FORCEDATTACK) && (lastreturn))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackPlayerUnits_Cond, 0);
                lastreturn = FALSE;
                return (lastreturn);
            }
            speechEvent(selSelected.ShipPtr[0], COMM_AttackPlayerUnits, 0);
        }
        else if (numEnemies > 0)
        {
            //else its only enemies
            speechEvent(selSelected.ShipPtr[0], COMM_Attack, 0);
        }
        // are there any resources
        else if (numResources > 0)
        {
            if (resourceTypes == (1 << OBJ_AsteroidType))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResources, Asteroids);
            }
            else if (resourceTypes == (1 << OBJ_NebulaType))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResources, Nebulae);
            }
            else if (resourceTypes == (1 << OBJ_GasType))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResources, GasClouds);
            }
            else if (resourceTypes == (1 << OBJ_DustType))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResources, DustClouds);
            }
            else
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResourcesGeneric, 0);
            }
        }
    }
    else
    {
        speechEvent(selSelected.ShipPtr[0], COMM_Attack, 0);
    }

    lastreturn = TRUE;
    return (lastreturn);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void speechEventUnderAttack(Ship *target)
{
    if (target->playerowner == universe.curPlayerPtr)
    {
        if (target->shiptype == Mothership)
        {
            /* need to add some local warnings for MoShip */
            if (universe.totaltimeelapsed > (universe.curPlayerPtr->timeMoShipAttacked + SPEECH_MOSHIP_WARNING_TIME))
            {
                speechEventFleetSpec(target, STAT_F_MoShip_UnderAttack, 0, target->playerowner->playerIndex);
                universe.curPlayerPtr->timeMoShipAttacked = universe.totaltimeelapsed;
            }
        }
        else if (universe.totaltimeelapsed > (target->soundevent.timeLastStatus + SPEECH_WARNING_TIME))
        {
            if (target->shiptype == ResourceCollector)
            {
                speechEventFleetSpec(target, STAT_F_ResCol_Damaged, 0, target->playerowner->playerIndex);
                target->soundevent.timeLastStatus = universe.totaltimeelapsed;
            }
            else if (univSpaceObjInRenderList((SpaceObj *)target))
            {
                /* some local warnings */
                if (target->shiptype == Carrier)
                {
                    speechEvent(target, STAT_Carrier_Damaged, 0);
                    target->soundevent.timeLastStatus = universe.totaltimeelapsed;
                }
                else if (target->shiptype == HeavyCruiser)
                {
                    if ((target->health / target->staticinfo->maxhealth) < SEL_ShipHealthYellowFactor)
                    {
                        speechEvent(target, STAT_Cruiser_Damaged, 0);
                        target->soundevent.timeLastStatus = universe.totaltimeelapsed;
                    }
                }
                else if (target->shiptype == ResourceController)
                {
                    speechEvent(target, STAT_ResCol_Damaged, 0);
                    target->soundevent.timeLastStatus = universe.totaltimeelapsed;
                }
                else if (selAnyHotKeyTest(target) && (universe.totaltimeelapsed > (selHotKeyGroup[selHotKeyGroupNumberTest(target)].timeLastStatus + SPEECH_WARNING_TIME)))
                {
                    speechEvent(target, STAT_AssGrp_UnderAttack, selHotKeyGroupNumberTest(target));
                    selHotKeyGroup[selHotKeyGroupNumberTest(target)].timeLastStatus = universe.totaltimeelapsed;
                }
            }
            else
            {
                /* global warnings */
                if ((target->shiptype == Carrier) || (target->shiptype == HeavyCruiser))
                {
                    speechEventFleetSpec(target, STAT_F_SuperCap_Damaged, (target->shiptype == Carrier)?0:1, target->playerowner->playerIndex);
                    target->soundevent.timeLastStatus = universe.totaltimeelapsed;
                }
                else if (selAnyHotKeyTest(target) && (universe.totaltimeelapsed > (selHotKeyGroup[selHotKeyGroupNumberTest(target)].timeLastStatus + SPEECH_WARNING_TIME)))
                {
                    speechEventFleet(STAT_F_AssGrp_UnderAttack, selHotKeyGroupNumberTest(target), target->playerowner->playerIndex);
                    selHotKeyGroup[selHotKeyGroupNumberTest(target)].timeLastStatus = universe.totaltimeelapsed;
                }
            }
        }
    }
}


sdword musictranslatetracknum(sdword tracknum)
{
    sdword track;
#ifdef OEM
    switch (tracknum)
    {
        case AMB_Mission1:
            track = 0;
            break;
        case AMB_Mission2:
            track = 1;
            break;
        case AMB_Mission3:
            track = 2;
            break;
        case AMB_Mission5:
            track = 3;
            break;
        case AMB13_Tutorial:
            track = 4;
            break;
        case NIS01_R1:
            track = 5;
            break;
        case NIS01_R2:
            track = 6;
            break;
        case NIS02:
            track = 7;
            break;
        case NIS03:
            track = 8;
            break;
        case NIS04:
            track = 9;
            break;
        case ANIM00_Sierra:
            track = 10;
            break;
        case ANIM00_Relic:
            track = 11;
            break;
        case ANIM00_Opening:
            track = 12;
            break;
        case ANIM01_02:
            track = 13;
            break;
        case ANIM02_03:
            track = 14;
            break;
        case ANIM03_04:
            track = 15;
            break;
        case B01_TuranicRaidersLong:
            track = 16;
            break;
        case B02_TuranicRaidersShort:
            track = 17;
            break;
        case B04_EvilEmpire:
            track = 18;
            break;
        case RollCredits:
            track = 19;
            break;
        default:
            if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
            {
                track = 0;
            }
            else
            {
                track = -1;
            }
            break;
    }
#elif defined(CGW)
    switch (tracknum)
    {
        case AMB_Mission1:
            track = 0;
            break;
        case AMB_Mission2:
            track = 1;
            break;
        case AMB12_FrontEnd:
            track = 2;
            break;
        case AMB13_Tutorial:
            track = 3;
            break;
        case NIS01_R1:
            track = 4;
            break;
        case NIS01_R2:
            track = 5;
            break;
        case NIS02:
            track = 6;
            break;
        case ANIM00_Sierra:
            track = 7;
            break;
        case ANIM00_Relic:
            track = 8;
            break;
        case ANIM00_Opening:
            track = 9;
            break;
        case ANIM01_02:
            track = 10;
            break;
        case ANIM02_03:
            track = 11;
            break;
        default:
            if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
            {
                track = 0;
            }
            else
            {
                track = -1;
            }
            break;
    }
#elif defined(Downloadable)
    if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
    {
        track = 0;
    }
    else if (tracknum == NIS01_R1)
    {
        track = 1;
    }
    else if (tracknum == NIS02)
    {
        track = 2;
    }
    else if (tracknum == ANIM01_02)
    {
        track = 3;
    }
    else
    {
        track = -1;
    }
#elif defined(DLPublicBeta)
    if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
    {
        track = 0;
    }
    else
    {
        track = -1;
    }
#else
    track = tracknum;
#endif
    return (track);
}


void MEstarttrack(MUSICINFO *pinfo, MUSICSTREAM *pstream)
{
    if (pinfo->tracknum == RollCredits)
    {
        pinfo->targetvol = (sword)(pstream->volume * volNIS);
    }
    else
    {
        pinfo->targetvol = (sword)(pstream->volume * volMusic);
    }

    soundstreamqueue(pinfo->handle, musicfilehandle,
                    pstream->offset,
                    pstream->flags | SOUND_FLAGS_QUEUESTREAM,
                    pinfo->targetvol,
                    SOUND_PAN_CENTER,
                    (sword)pstream->numchannels,
                    (sword)pstream->bitrate,
                    NULL, NULL, NULL, -1);

    pinfo->status = SOUND_STARTING;
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword musicEventPlay(sdword tracknum)
{
    MUSICINFO *pinfo;
    MUSICSTREAM *pmstream = &musicheader->mstreams[musictranslatetracknum(tracknum)];

    if (enableSpeech)
    {
        if ((tracknum <= MUS_LAST_LEVELTUNE) && (tracknum >= MUS_FIRST_AMBIENT))
        {
            levelTrack = tracknum;
        }

        if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
        {
            pinfo = &musicinfo[AMBIENTSTREAM];

            // this is an ambient for the level or a battle track
            if (!soundstreamover(pinfo->handle))
            {
                // stop this track and start the next one
                pinfo->nexttrack = tracknum;
                pinfo->status = SOUND_RESTART;
                pinfo->targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(pinfo->handle, pinfo->targetvol, MUSIC_FADE_SLOW);

                return (SOUND_OK);
            }

            pinfo->tracknum = tracknum;

            if (musicinfo[NISSTREAM].status > SOUND_FREE)
            {
                // if the NIS is still playing then wait till its shutting off to start this
                return (SOUND_OK);
            }

            MEstarttrack(pinfo, pmstream);

            return (SOUND_OK);
        }
        else if ((tracknum >= MUS_FIRST_NIS) && (tracknum <= MUS_LAST_ANIMATIC))
        {
            pinfo = &musicinfo[NISSTREAM];

            // this is an NIS or Animatic track
            if (!soundstreamover(pinfo->handle))
            {
                pinfo->nexttrack = tracknum;
                pinfo->status = SOUND_RESTART;
                pinfo->targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(pinfo->handle, pinfo->targetvol, MUSIC_FADE_FAST);

                return (SOUND_OK);
            }

            if ((musicinfo[AMBIENTSTREAM].status == SOUND_PLAYING) || (musicinfo[AMBIENTSTREAM].status == SOUND_STARTING))
            {
                // shut off the ambient if its playing
                musicinfo[AMBIENTSTREAM].status = SOUND_STOPPING;
                soundstreamvolume(musicinfo[AMBIENTSTREAM].handle, SOUND_VOL_AUTOSTOP, MUSIC_FADE_FAST);
            }

            pinfo->tracknum = tracknum;

            MEstarttrack(pinfo, pmstream);

            if (pinfo->tracknum != RollCredits)
            {
                musicinfo[AMBIENTSTREAM].tracknum = SOUND_NOTINITED;
            }

            return (SOUND_OK);
        }
    }

    return (SOUND_ERR);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword musicEventStop(sdword tracknum, real32 fadetime)
{
    if (enableSpeech)
    {
        if (tracknum == SOUND_DEFAULT)
        {
            if ((musicinfo[AMBIENTSTREAM].status == SOUND_PLAYING) || (musicinfo[AMBIENTSTREAM].status == SOUND_STARTING))
            {
                tracknum = musicinfo[AMBIENTSTREAM].tracknum;
            }
            else if ((musicinfo[NISSTREAM].status == SOUND_PLAYING) || (musicinfo[NISSTREAM].status == SOUND_STARTING))
            {
                tracknum = musicinfo[NISSTREAM].tracknum;
            }
        }
        if ((tracknum >= MUS_FIRST_NIS) && (tracknum <= MUS_LAST_ANIMATIC))
        {
            /* this is an NIS */
            if (!soundstreamover(musicinfo[NISSTREAM].handle))
            {
                musicinfo[NISSTREAM].targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(musicinfo[NISSTREAM].handle, musicinfo[NISSTREAM].targetvol, fadetime);
            }
            musicinfo[NISSTREAM].status = SOUND_STOPPING;

            speechEventStopAllSpecific(1.0f, SPEECH_NIS_FLAG);
        }
        else if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
        {
            /* this is the AMBIENT track */
            if (tracknum != musicinfo[AMBIENTSTREAM].tracknum)
            {
                return (SOUND_ERR);
            }

            if ((musicinfo[AMBIENTSTREAM].status == SOUND_PLAYING) || (musicinfo[AMBIENTSTREAM].status == SOUND_STARTING))
            {
                musicinfo[AMBIENTSTREAM].targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(musicinfo[AMBIENTSTREAM].handle, musicinfo[AMBIENTSTREAM].targetvol, fadetime);
                musicinfo[AMBIENTSTREAM].status = SOUND_STOPPING;
                musicinfo[AMBIENTSTREAM].tracknum = SOUND_ERR;
                if (tracknum > MUS_LAST_LEVELTUNE)
                {
                    musicEventPlay(levelTrack);
                }
            }
        }
    }

    return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword musicEventUpdateVolume(void)
{
    sword volume;
    MUSICINFO *pinfo;
    MUSICSTREAM *pmstream;

    if (enableSpeech)
    {
        /* Check the AMBIENT */
        pinfo = &musicinfo[AMBIENTSTREAM];
        pmstream = &musicheader->mstreams[musictranslatetracknum(pinfo->tracknum)];

        if (pinfo->status == SOUND_STARTING)
        {
            if (soundstreamgetvol(pinfo->handle) == pinfo->targetvol)
            {
                pinfo->status = SOUND_PLAYING;
            }
            else if ((!soundstreamfading(pinfo->handle)) && (!soundstreamover(pinfo->handle)))
            {
                soundstreamvolume(pinfo->handle, pinfo->targetvol, MUSIC_FADE_FAST);
            }
        }
        else if (pinfo->status == SOUND_PLAYING)
        {
            if (!soundstreamover(pinfo->handle))
            {
                if (mrRenderMainScreen)
                {
                    if (pinfo->tracknum == AMB13_Tutorial)
                    {
                        /* we're in the tutorial, play the music at a constant volume */
                        volume = (sword)(pmstream->volume * MUSIC_TUTORIAL_VOL * volMusic);
                    }
                    else
                    {
                        /* we're in the game, volume depends on the distance from the nearst ship */
                        volume = (sword)(pmstream->volume * volMusic);

                        if (nearestShipDistance <= MUSIC_DISTANCE_SILENT)
                        {
                            if (pinfo->tracknum < MUS_FIRST_BATTLE)
                            {
                                if (nearestShipMoving)
                                {
                                    volume = (sword)((real32)volume * MUSIC_MINACTIVE_VOL);
                                }
                                else
                                {
                                    volume = (sword)((real32)volume * MUSIC_MININACTIVE_VOL);
                                }
                            }
                            else
                            {
                                volume = (sword)((real32)volume * MUSIC_MINBATTLE_VOL);
                            }
                        }
                        else if (nearestShipDistance <= MUSIC_DISTANCE_MAX)
                        {
                            if (pinfo->tracknum < MUS_FIRST_BATTLE)
                            {
                                if (nearestShipMoving)
                                {
                                    volume = (sword)((real32)volume * (((nearestShipDistance - MUSIC_DISTANCE_SILENT) * musicVolFactor) + MUSIC_MINACTIVE_VOL));
                                }
                                else
                                {
                                    volume = (sword)((real32)volume * (((nearestShipDistance - MUSIC_DISTANCE_SILENT) * musicInactiveFactor) + MUSIC_MININACTIVE_VOL));
                                }
                            }
                            else
                            {
                                volume = (sword)((real32)volume * (((nearestShipDistance - MUSIC_DISTANCE_SILENT) * battleMusicVolFactor) + MUSIC_MINBATTLE_VOL));
                            }
                        }
                        else
                        {
                            if (pinfo->tracknum < MUS_FIRST_BATTLE)
                            {
                                volume = (sword)((real32)volume * MUSIC_MAXGAME_VOL);
                            }
                            else
                            {
                                volume = (sword)((real32)volume * MUSIC_MAXBATTLE_VOL);
                            }
                        }


                        if (volume > pmstream->volume)
                        {
                            volume = pmstream->volume;
                        }
                        else if (volume < 0)
                        {
                            volume = 0;
                        }
                    }
                }
                else
                {
                    if (smSensorsActive)
                    {
                        /* we're in the sensors manager, play the music at the volume defined in the script */
                        if (pinfo->tracknum < MUS_FIRST_BATTLE)
                        {
                            volume = (sword)(pmstream->volume * MUSIC_SENSORS_VOL * volMusic);
                        }
                        else
                        {
                            volume = (sword)(pmstream->volume * MUSIC_MINBATTLE_VOL * volMusic);
                        }
                    }
                    else
                    {
                        /* we're in one of the other manager screens */
                        if (pinfo->tracknum < MUS_FIRST_BATTLE)
                        {
                            volume = (sword)(pmstream->volume * MUSIC_MANAGERS_VOL * volMusic);
                        }
                        else
                        {
                            volume = (sword)(pmstream->volume * MUSIC_MINBATTLE_VOL * volMusic);
                        }
                    }
                }

                if (!bVolMusicNoFade)
                {
                    soundstreamvolume(pinfo->handle, volume, MUSIC_FADE_TIME);
                }
                else
                {
                    soundstreamvolume(pinfo->handle, volume, 0.0f);
                    bVolMusicNoFade = FALSE;
                }
            }
            else
            {
                pinfo->status = SOUND_FREE;
                musicEventPlay(pinfo->tracknum);
            }
        }
        else if (pinfo->status == SOUND_STOPPING)
        {
            if (soundstreamover(pinfo->handle))
            {
                pinfo->status = SOUND_FREE;
            }
            else if ((pinfo->targetvol != SOUND_VOL_AUTOSTOP) ||
                (!soundstreamfading(pinfo->handle) &&
                (soundstreamgetvol(pinfo->handle) > SOUND_VOL_MIN)))
            {
                pinfo->targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(pinfo->handle, pinfo->targetvol, SOUND_FADE_STOPTIME);
            }
        }
        else if (pinfo->status == SOUND_RESTART)
        {
            if (soundstreamover(pinfo->handle))
            {
                pinfo->tracknum = pinfo->nexttrack;
                pinfo->nexttrack = SOUND_NOTINITED;
                pmstream = &musicheader->mstreams[musictranslatetracknum(pinfo->tracknum)];

                MEstarttrack(pinfo, pmstream);
            }
            else if ((pinfo->targetvol != SOUND_VOL_AUTOSTOP) ||
                (!soundstreamfading(pinfo->handle) &&
                (soundstreamgetvol(pinfo->handle) > SOUND_VOL_MIN)))
            {
                pinfo->targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(pinfo->handle, pinfo->targetvol, SOUND_FADE_STOPTIME);
            }
        }

        /* Check the NIS track */
        pinfo = &musicinfo[NISSTREAM];

        if (pinfo->status == SOUND_PLAYING)
        {
            if (soundstreamover(pinfo->handle) && !nisIsRunning)
            {
                pinfo->status = SOUND_STOPPING;
            }
            else if (pinfo->targetvol == SOUND_VOL_AUTOSTOP)
            {
                soundstreamvolume(pinfo->handle, pinfo->targetvol, SOUND_FADE_STOPTIME);
                pinfo->status = SOUND_STOPPING;
            }
        }
        else if (pinfo->status == SOUND_STARTING)
        {
            if (!soundstreamover(pinfo->handle))
                pinfo->status = SOUND_PLAYING;
        }
        else if (pinfo->status == SOUND_STOPPING)
        {
            if (soundstreamover(pinfo->handle))
            {
                pinfo->status = SOUND_FREE;
                if (pinfo->tracknum <= ANIM00_Relic)
                {
                    musicEventPlay(levelTrack);
                }
            }
            else if ((pinfo->targetvol != SOUND_VOL_AUTOSTOP) ||
                (!soundstreamfading(pinfo->handle) &&
                (soundstreamgetvol(pinfo->handle) > SOUND_VOL_MIN)))
            {
                pinfo->targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(pinfo->handle, pinfo->targetvol, SOUND_FADE_STOPTIME);
            }
        }
        else if (pinfo->status == SOUND_RESTART)
        {
            if (soundstreamover(pinfo->handle))
            {
                pinfo->tracknum = pinfo->nexttrack;
                pinfo->nexttrack = SOUND_NOTINITED;
                pmstream = &musicheader->mstreams[musictranslatetracknum(pinfo->tracknum)];

                MEstarttrack(pinfo, pmstream);
            }
            else if ((pinfo->targetvol != SOUND_VOL_AUTOSTOP) ||
                (!soundstreamfading(pinfo->handle) &&
                (soundstreamgetvol(pinfo->handle) > SOUND_VOL_MIN)))
            {
                pinfo->targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(pinfo->handle, pinfo->targetvol, SOUND_FADE_STOPTIME);
            }
        }
    }

    return (SOUND_OK);
}


#if defined(CGW) || defined(Downloadable) || defined(DLPublicBeta) || defined(OEM)

void musicEventNextTrack(void)
{
    return;
}

void musicEventPrevTrack(void)
{
    return;
}
#else

void musicEventNextTrack(void)
{
    MUSICINFO *pinfo;

    if (!singlePlayerGame && enableSpeech)
    {
        pinfo = &musicinfo[AMBIENTSTREAM];

        pinfo->nexttrack = pinfo->tracknum + 1;
        if (pinfo->nexttrack > MUS_LAST_BATTLE)
        {
            pinfo->nexttrack = MUS_FIRST_AMBIENT;
        }

#if SE_VERBOSE_LEVEL >= 2
        dbgMessagef("Starting track # %d\n", pinfo->nexttrack);
#endif
        pinfo->status = SOUND_RESTART;
        soundstreamvolume(pinfo->handle, SOUND_VOL_AUTOSTOP, MUSIC_FADE_FAST);
   }
}


void musicEventPrevTrack(void)
{
    MUSICINFO *pinfo;

    if (!singlePlayerGame && enableSpeech)
    {
        pinfo = &musicinfo[AMBIENTSTREAM];

        pinfo->nexttrack = pinfo->tracknum - 1;
        if (pinfo->nexttrack < MUS_FIRST_AMBIENT)
        {
            pinfo->nexttrack = MUS_LAST_BATTLE;
        }

#if SE_VERBOSE_LEVEL >= 2
        dbgMessagef("Starting track # %d\n", pinfo->nexttrack);
#endif
        pinfo->status = SOUND_RESTART;
        soundstreamvolume(pinfo->handle, SOUND_VOL_AUTOSTOP, MUSIC_FADE_FAST);
   }
}
#endif

sdword musicEventCurrentTrack(void)
{
    return (musicinfo[AMBIENTSTREAM].tracknum);
}

