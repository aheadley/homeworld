/*=============================================================================
    Name    : animatic.c
    Purpose : playback of animatics via OpenGL

    Created 2/11/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include "glinc.h"
#include "main.h"
#include "memory.h"
#include "bink.h"
#include "render.h"
#include "file.h"
#include "debug.h"
#include "animatic.h"
#include "nis.h"
#include "tutor.h"
#include "subtitle.h"
#include "universe.h"
#include "mouse.h"
#include "soundevent.h"
#include "soundlow.h"
#include "sstglide.h"
#include "glcaps.h"
#include "glcompat.h"
#include "strings.h"

#define USE_3DFX 1

#if USE_3DFX
extern bool gl3Dfx;
#endif

/* sound volume stuff */
real32 animPreviousSFXVolume, animPreviousSpeechVolume, animPreviousMusicVolume;

static sdword subY1;

static sdword g_frame, g_displayFrame;
static bool   g_cleared;

sdword animaticJustPlayed = 0;

static ubyte* surf888;
ubyte* surf8888 = NULL;

extern HDC hGLDeviceContext;

#define NUM_SP_MISSIONS 19

typedef struct
{
    char filename[32];
} animlst;

static animlst animlisting[NUM_SP_MISSIONS];

static nisheader* animScriptHeader = NULL;
static sdword animCurrentEvent;

static real32 g_timeElapsed;

#define FORM_RGB32(R,G,B) \
        ((R << 16) | (G << 8) | B)
#define FORM_RGB565(R,G,B) \
        (((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | ((B) >> 3)
#define FORM_RGB555(R,G,B) \
        (((R) & 0xF8) << 7) | (((G) & 0xF8) << 2) | ((B) >> 3)

//for localization
extern char *nisLanguageSubpath[];

/*-----------------------------------------------------------------------------
    Name        : animStartup
    Description : reads animatics.lst from the Movies directory
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animStartup(void)
{
    filehandle lst;
    char   line[128], temp[64];
    sdword level;

    memset(animlisting, 0, NUM_SP_MISSIONS*sizeof(animlst));

    if (!fileExists("Movies\\animatics.lst", 0))
    {
        return;
    }

    lst = fileOpen("Movies\\animatics.lst", FF_TextMode);
    while (fileLineRead(lst, line, 127) != FR_EndOfFile)
    {
        if (strlen(line) < 5)
        {
            continue;
        }
        if (line[0] == ';' || line[0] == '/')
        {
            continue;
        }
        sscanf(line, "%d %s", &level, temp);
        dbgAssert(level >= 0 && level < NUM_SP_MISSIONS);
        memStrncpy(animlisting[level].filename, temp, 31);
    }
    fileClose(lst);
}

/*-----------------------------------------------------------------------------
    Name        : animShutdown
    Description : releases memory required by the animatics listing file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animShutdown(void)
{
    memset(animlisting, 0, NUM_SP_MISSIONS*sizeof(animlst));
}

/*-----------------------------------------------------------------------------
    Name        : animBinkSetup
    Description : setup / reset the GL for video playback
    Inputs      : on - TRUE or FALSE, setup or reset
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animBinkSetup(bool on)
{
    static GLint matrixMode;
    static GLfloat projection[16];

    if (on)
    {
        glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
        glGetFloatv(GL_PROJECTION_MATRIX, projection);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        rgluOrtho2D(0.0f, (GLfloat)MAIN_WindowWidth, 0.0f, (GLfloat)MAIN_WindowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    else
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection);

        glMatrixMode(matrixMode);
    }
}

/*-----------------------------------------------------------------------------
    Name        : animSubtitlesSetup
    Description : setup / reset the GL for displaying subtitles
    Inputs      : on - TRUE or FALSE, setup or reset
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animSubtitlesSetup(bool on)
{
    static GLint matrixMode;
    static GLfloat projection[16];

    if (on)
    {
        glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
        glGetFloatv(GL_PROJECTION_MATRIX, projection);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    else
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection);

        glMatrixMode(matrixMode);
    }
}

/*-----------------------------------------------------------------------------
    Name        : animSubtitlesClear
    Description : clear (to black) the region that subtitles appear in
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animSubtitlesClear(void)
{
    rectangle r;

    if (subY1 < 0 || subY1 > (MAIN_WindowHeight >> 1))
    {
        return;
    }

    animSubtitlesSetup(TRUE);

    r.x0 = -1;
    r.y0 = 0;
    r.x1 = MAIN_WindowWidth;
    r.y1 = (subY1 > 128) ? subY1 : 128;
    primRectSolid2(&r, colBlack);

    animSubtitlesSetup(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : animSubtitlesDraw
    Description : render any subtitles
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animSubtitlesDraw(void)
{
    sdword index;

    animSubtitlesSetup(TRUE);

    subY1 = 0;

    for (index = 0; index < SUB_NumberRegions; index++)
    {
        if (subRegion[index].bEnabled && subRegion[index].cardIndex > 0)
        {
            subTimeElapsed = &universe.totaltimeelapsed;
            subTitlesDraw(&subRegion[index]);
            if (subRegion[index].rect.y1 > subY1)
            {
                subY1 = subRegion[index].rect.y1;
            }
        }
    }

    animSubtitlesSetup(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : animBinkReverseRGBA
    Description : inplace y-flip an RGBA image (640x480x32)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animBinkReverseRGBA(ubyte* surf)
{
    ubyte  line[4*640];
    sdword y, top, bot;
    sdword pitch;

    pitch = 4*640;

    for (y = 0; y < (480/2); y++)
    {
        top = y;
        bot = 479 - y;

        memcpy(line, surf + pitch*top, pitch);
        memcpy(surf + pitch*top, surf + pitch*bot, pitch);
        memcpy(surf + pitch*bot, line, pitch);
    }
}

#if USE_3DFX
/*-----------------------------------------------------------------------------
    Name        : animBinkDisplay3Dfx
    Description : 3Dfx-specific display callback for a frame of Bink video
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animBinkDisplay3Dfx(binkDisplayCallback_proc callback)
{
    ubyte* fb;
    sdword xOfs, yOfs;
    sdword pitch;
    ubyte* binkSurface = (ubyte*)binkGetSurface();

    if (g_frame <= g_displayFrame)
    {
        return;
    }
    g_displayFrame = g_frame;

    xOfs = (MAIN_WindowWidth  - 640) / 2;
    yOfs = (MAIN_WindowHeight - 480) / 2;

    fb = (ubyte*)sstGetFramebuffer(&pitch);
    if (fb == NULL)
    {
        return;
    }
    callback(fb, pitch, xOfs, yOfs);
    sstGetFramebuffer(NULL);

    animSubtitlesDraw();

    sstFlush();

    animSubtitlesClear();
}
#endif

/*-----------------------------------------------------------------------------
    Name        : animBinkDisplay
    Description : display callback for a frame of Bink video
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animBinkDisplay(binkDisplayCallback_proc callback)
{
    sdword xOfs, yOfs;
    ubyte* binkSurface = (ubyte*)binkGetSurface();

    if (g_frame <= g_displayFrame)
    {
        return;
    }
    g_displayFrame = g_frame;

    //prepare binkSurface
    callback(NULL, 0, 0, 0);

    xOfs = (MAIN_WindowWidth  - 640) / 2;
    yOfs = (MAIN_WindowHeight - 480) / 2;

    animBinkSetup(TRUE);
    glRasterPos2f((real32)xOfs, (real32)yOfs);
    switch (RGLtype)
    {
    case GLtype:
        glcDisplayRGBABackgroundScaled(binkSurface);
        break;
    case SWtype:
        glDrawPixels(640, 480, GL_RGBA16, GL_UNSIGNED_BYTE, binkSurface);
        break;
    case D3Dtype:
        animBinkReverseRGBA(binkSurface);
        glDrawPixels(640, 480, GL_RGBA, GL_UNSIGNED_BYTE, binkSurface);
        break;
    default:
        dbgFatalf(DBG_Loc, "what's this RGLtype: %d [binkSimpleDisplayProc]", RGLtype);
    }
    animBinkSetup(FALSE);

    animSubtitlesDraw();
    rndFlush();

    animSubtitlesClear();
}

/*-----------------------------------------------------------------------------
    Name        : animLoadNISScript
    Description : process an NIS script file containing time-keyed events
    Inputs      : scriptname - name of script file to process
    Outputs     :
    Return      : nisheader
----------------------------------------------------------------------------*/
nisheader* animLoadNISScript(char* scriptname)
{
    // for localization
	char *pString;
    char localisedPath[256];
    char string[256];
    nisheader* newHeader;

    animCurrentEvent = 0;

    newHeader = memAlloc(sizeof(nisheader), "animatic NIS header", NonVolatile);
    memset(newHeader, 0, sizeof(nisheader));

    newHeader->iLookyObject = -1;
    newHeader->length = NIS_FrameRate * 10000;
//    if (scriptname != NULL && fileExists(scriptname, 0))
    if (scriptname != NULL)
    {
        // for localization
		strcpy(localisedPath, scriptname);
        if (strCurLanguage >= 1)
        {
            for (pString = localisedPath + strlen(localisedPath); pString > localisedPath; pString--)
            {                                               //find the end of the path
                if (*pString == '\\')
                {
                    strcpy(string, pString + 1);            //save the file name
                    strcpy(pString + 1, nisLanguageSubpath[strCurLanguage]);//add the language sub-path
                    strcat(pString, string);                //put the filename back on
                    break;
                }
            }
        }

		if (fileExists(localisedPath, 0))
		{
			nisEventIndex = 0;
			nisCurrentHeader = newHeader;
			scriptSet(NULL, localisedPath, nisScriptTable);     //load in the script file
//        scriptSet(NULL, scriptname, nisScriptTable);
			nisCurrentHeader = NULL;
			newHeader->nEvents = nisEventIndex;
			if (newHeader->nEvents != 0)
			{
				newHeader->events = nisEventList;
				qsort(newHeader->events, newHeader->nEvents, sizeof(nisevent), nisEventSortCB);
				nisEventList = NULL;
			}
			else
			{
				newHeader->events = NULL;
			}
		}
		else
		{
			// file doesn't exist
			newHeader->nEvents = 0;
			newHeader->events = NULL;
		}
    }
    else
    {
		// script name = null
        newHeader->nEvents = 0;
        newHeader->events = NULL;
    }
    return newHeader;
}

/*-----------------------------------------------------------------------------
    Name        : animGenericDecode
    Description : callback once per frame decode
    Inputs      : frame - current frame number of animation
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animBinkDecode(sdword frame)
{
    real32 framerate, timeElapsed;
    nisheader* header;
    nisevent* event;

    g_frame = frame;

    if (animScriptHeader == NULL)
    {
        return;
    }

    header = animScriptHeader;

    framerate = binkFrameRate;
    timeElapsed = (real32)frame / framerate;

    event = &animScriptHeader->events[animCurrentEvent];
    while (animCurrentEvent < header->nEvents &&
           event->time <= timeElapsed)
    {
        dbgAssert(nisEventDispatch[event->code] != NULL);
        nisEventDispatch[event->code](NULL, event);
        animCurrentEvent++;
        event++;
    }

    universe.totaltimeelapsed = timeElapsed;
    subTitlesUpdate();
}

/*-----------------------------------------------------------------------------
    Name        : animBinkPlay
    Description : plays a Bink video file
    Inputs      : a, b - levels this video is playing between
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool animBinkPlay(sdword a, sdword b)
{
    bool rval;
    char filename[1024], scriptname[1024];
    void animBinkEnd(void);
    extern sdword trLitPaletteBits;

    if (tutorial == 1)
    {
        return FALSE;
    }

    g_cleared = FALSE;
    g_frame = 0;
    g_displayFrame = 0;
    animaticJustPlayed = 0;
    subReset();

    if (a < 0)
    {
        animScriptHeader = NULL;
        sprintf(filename, (char*)b);
    }
    else
    {
        dbgAssert(a >= 0 && b < NUM_SP_MISSIONS);
        if (animlisting[a].filename[0] == '\0')
        {
            return FALSE;
        }

        sprintf(filename, "Movies\\%s.bik", animlisting[a].filename);
        sprintf(scriptname, "Movies\\%s.script", animlisting[a].filename);
        animScriptHeader = animLoadNISScript(scriptname);
    }

    soundEventStopMusic(0.0f);
    soundstopall(0.0f);

    if (!binkInit(RGLtype))
    {
        return FALSE;
    }

    rndSetClearColor(colBlack);
    rndClear();

    mouseCursorHide();

    g_timeElapsed = universe.totaltimeelapsed;
    universe.totaltimeelapsed = 0.0f;

    soundEventGetVolume(&animPreviousSFXVolume, &animPreviousSpeechVolume, &animPreviousMusicVolume);

    rval = binkPlay(filename,
#if USE_3DFX
                    (gl3Dfx && sstLoaded()) ? animBinkDisplay3Dfx : animBinkDisplay,
#else
                    animBinkDisplay,
#endif
                    animBinkDecode,
                    (trLitPaletteBits == 15) ? S_RGB555 : S_RGB565,
                    FALSE, -1);

    animBinkEnd();

    return rval;
}

/*-----------------------------------------------------------------------------
    Name        : animBinkEnd
    Description : cleanup after playing a Bink video file as an animatic
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void animBinkEnd(void)
{
    universe.totaltimeelapsed = g_timeElapsed;
    soundEventStopMusic(0.0f);

    mouseCursorShow();

    rndClear();

    binkCleanup();
    soundstopall(0.0);
    speechEventCleanup();
    subReset();

    soundEventMusicVol(animPreviousMusicVolume);
    soundEventSpeechVol(animPreviousSpeechVolume);
    soundEventSFXVol(animPreviousSFXVolume);

    animaticJustPlayed = 8;
}
