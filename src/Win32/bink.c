/*=============================================================================
    Name    : bink.c
    Purpose : routines for playback of Bink files

    Created 6/8/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include "bink.h"
#include "main.h"
#include "file.h"
#include "debug.h"
#include "key.h"
#include "soundevent.h"
#include "dxdraw.h"
#include "utility.h"


#define USE_TIMER 0
#define INCREMENTAL_FRAMES 0

#define BINK_SemaphoreName "BINKSEMAPHORE"

#define SWtype    0
#define GLIDEtype 1
#define D3Dtype   2
#define GLtype    3

HANDLE binkSemaphore;

bool binkDonePlaying = TRUE;
static bool binkIsPaused = FALSE;

static sdword g_RGLtype;

static binkDisplay_proc g_displayProc;
static binkDecodeCallback_proc g_decodeProc;
static binkEndCallback_proc g_endProc;
static sdword g_trackNum;
static sdword g_surfType;

static HBINK bnk = 0;

MMRESULT binkTimerHandle = 0;
static bool decodeTimer;

bool binkPlaying = FALSE;
static bool inDecode = FALSE;
static bool binkStopNow = FALSE;
static bool binkUpdate = FALSE;

static sdword binkDecodeFrameCount;
static sdword binkDisplayFrameCount;

static u32 binkDisplayFlags;

static LARGE_INTEGER binkTimerFrequency;
static LONGLONG binkTimerDivisor;
static LONGLONG binkTimerStart;
static sdword binkTimerFrame;

static uword* binkSurface = NULL;

real32 binkFrameRate = 0.0f;
sdword binkFrameAdd;

extern HDC hGLDeviceContext;

void binkTimeReset(void);


/*-----------------------------------------------------------------------------
    Name        : binkGetSurface
    Description : return the surface that frames are decoded onto
    Inputs      :
    Outputs     :
    Return      : binkSurface (pitch = width*(depth>>3))
----------------------------------------------------------------------------*/
uword* binkGetSurface(void)
{
    return binkSurface;
}

/*-----------------------------------------------------------------------------
    Name        : binkInit
    Description :
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE (success or failure)
----------------------------------------------------------------------------*/
bool binkInit(sdword rgltype)
{
    sdword size;

    g_RGLtype = rgltype;

    binkSemaphore = CreateSemaphore(NULL, 1, 1, BINK_SemaphoreName);
    dbgAssert(binkSemaphore != NULL);

    switch (g_RGLtype)
    {
    case SWtype:
        size = 2;
        break;
    case -1:
    case GLtype:
    case D3Dtype:
        size = 4;
        break;
    default:
        dbgFatalf(DBG_Loc, "what's this type: %d [binkInit]", g_RGLtype);
    }

    binkSurface = (uword*)radmalloc(size*640*480);

    binkTimeReset();

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : binkCleanup
    Description : cleanup after displaying a Bink video file
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool binkCleanup(void)
{
    keyClearAll();

    CloseHandle(binkSemaphore);

    if (binkSurface != NULL)
    {
        radfree(binkSurface);
        binkSurface = NULL;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : binkGetBink
    Description : returns the Bink internal structure
    Inputs      :
    Outputs     :
    Return      : bnk
----------------------------------------------------------------------------*/
HBINK binkGetBink(void)
{
    return bnk;
}

/*-----------------------------------------------------------------------------
    Name        : binkOpen
    Description : wrapper for opening a Bink video file
    Inputs      : filename - name of the Bink video file to open
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
static bool binkOpen(char* filename)
{
    char  fullname[1024];
    char  cdname[1024];
    char* dir;

    // get CD path
    strcpy(cdname,filePathPrepend(filename,FF_CDROM));

    dir = getenv("HW_Data");
    if (dir == NULL)
    {
		// set default path
        strcpy(fullname, filename);
    }
    else
    {
		// set HW_Data path
        strcpy(fullname, dir);
        strcat(fullname, "\\");
        strcat(fullname, filename);
    }

    BinkSoundUseDirectSound(NULL);

	// try default path or HW_Data path
    bnk = BinkOpen(fullname, BINKNOTHREADEDIO);
    if (!bnk)
    {
		// try filename alone
		bnk = BinkOpen(filename, BINKNOTHREADEDIO);
		if (!bnk)
		{
			// try CD path
			bnk = BinkOpen(cdname, BINKNOTHREADEDIO);
			if (!bnk)
			{
				return FALSE;
			}
		}
	}

//	BinkSetSoundOnOff(bnk, 0);

    binkFrameRate = (real32)bnk->FrameRate / (real32)bnk->FrameRateDiv;

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : binkTimeReset
    Description : reset the frame timer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void binkTimeReset(void)
{
    LARGE_INTEGER timer;

    QueryPerformanceFrequency(&binkTimerFrequency);
    binkTimerDivisor = binkTimerFrequency.QuadPart;

    QueryPerformanceCounter(&timer);
    binkTimerStart = timer.QuadPart;

    binkTimerFrame = 1;
}

/*-----------------------------------------------------------------------------
    Name        : binkTimeElapsed
    Description : calculate amount of time (ms) elapsed since binkTimeReset was called
    Inputs      :
    Outputs     :
    Return      : ms of elapsed time
----------------------------------------------------------------------------*/
static sdword binkTimeElapsed(void)
{
    LARGE_INTEGER timer;
    LONGLONG difference;
    real32 secs;

    QueryPerformanceCounter(&timer);
    difference = timer.QuadPart - binkTimerStart;
    secs = (real32)difference / (real64)binkTimerDivisor;
    return (udword)(secs * 1000.0);
}

/*-----------------------------------------------------------------------------
    Name        : binkIdealFrame
    Description : returns the frame (based on time elapsed) that a video should be currently at
    Inputs      :
    Outputs     :
    Return      : the frame
----------------------------------------------------------------------------*/
static sdword binkIdealFrame(void)
{
#if INCREMENTAL_FRAMES
    binkTimerFrame++;
    return (binkTimerFrame + binkFrameAdd);
#else
    real32 ms;
    sdword frame;

    ms = 1000.0f / binkFrameRate;
    frame = (sdword)((real32)binkTimeElapsed() / ms);
    frame += binkFrameAdd;
    return frame;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : binkUserPaintCallback
    Description :
    Inputs      : psurf - surface pointer or NULL
                  pitch, x, y - ignored
    Outputs     : frame is blitted onto binkSurface
    Return      :
----------------------------------------------------------------------------*/
static void binkUserPaintCallback(void* psurf, sdword pitch, sdword x, sdword y)
{
    WaitForSingleObject(binkSemaphore, INFINITE);

    binkDisplayFlags = BINKSURFACECOPYALL;

    if (psurf != NULL)
    {
        BinkCopyToBuffer(bnk, psurf, pitch, 480, x, y, BINKSURFACE565 | binkDisplayFlags);
    }
    else
    {
        switch (g_RGLtype)
        {
        case -1:
            if (pitch == 2*640)
            {
                BinkCopyToBuffer(bnk, binkSurface, pitch, 480, 0, 0,
                                 ((g_surfType == S_RGB565) ? BINKSURFACE565 : BINKSURFACE555) | binkDisplayFlags);
            }
            else
            {
                BinkCopyToBuffer(bnk, binkSurface, pitch, 480, 0, 0, BINKSURFACE32 | binkDisplayFlags);
            }
            break;
        case SWtype:
            BinkCopyToBuffer(bnk, binkSurface, 2*640, 480, 0, 0,
                             ((g_surfType == S_RGB565) ? BINKSURFACE565 : BINKSURFACE555) | binkDisplayFlags);
            break;
        case GLtype:
        case D3Dtype:
            BinkCopyToBuffer(bnk, binkSurface, 4*640, 480, 0, 0, BINKSURFACE32 | binkDisplayFlags);
            break;
        default:
            dbgFatalf(DBG_Loc, "what's this type: %d [binkUserPaintCallback]", g_RGLtype);
        }
    }

    ReleaseSemaphore(binkSemaphore, 1, NULL);
}

/*-----------------------------------------------------------------------------
    Name        : binkNextFrame
    Description : advance to next frame of the Bink video file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void binkNextFrame(void)
{
    if (WaitForSingleObject(binkSemaphore, 0) == WAIT_FAILED)
    {
        return;
    }

    BinkDoFrame(bnk);
    binkUpdate = TRUE;
    if (bnk->FrameNum == (bnk->Frames - 1))
    {
        binkStopNow = TRUE;
    }
    else
    {
        BinkNextFrame(bnk);
    }

    ReleaseSemaphore(binkSemaphore, 1, NULL);
}

/*-----------------------------------------------------------------------------
    Name        : binkReverse
    Description : y-flips a given 640x480 image
    Inputs      : surf - surface to y-flip
                  pitch - surface pitch
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void binkReverse(ubyte* surf, sdword pitch)
{
    ubyte  line[4*640];
    sdword y, top, bot;

    for (y = 0; y < (480/2); y++)
    {
        top = y;
        bot = 479 - y;

        memcpy(line, surf + pitch*top, pitch);
        memcpy(surf + pitch*top, surf + pitch*bot, pitch);
        memcpy(surf + pitch*bot, line, pitch);
    }
}

/*-----------------------------------------------------------------------------
    Name        : binkDisplayWin32
    Description : Win32 method of displaying a frame of Bink video
    Inputs      : callback - fn that blits onto generic surface
                  depth - bitdepth of display device
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void binkDisplayWin32(binkDisplayCallback_proc callback, sdword depth)
{
    BITMAPINFOHEADER* pbi;
    BITMAPINFO bmi;
    BITMAPINFO* pbmi;
    HBITMAP hbitmap;
    BYTE* pbits;
    WORD* pbitmap;
    HDC hdc, hdcTemp;
    sdword bitsize;
    sdword xofs, yofs;
    sdword mult;

    mult = depth >> 3;

    callback(NULL, mult*640, 0, 0);
    binkReverse((ubyte*)binkSurface, mult*640);

    pbmi = &bmi;

    bitsize = mult * 640 * 480;
    pbits = (BYTE*)binkSurface;

    pbi = (BITMAPINFOHEADER*)pbmi;
    pbi->biSize = sizeof(BITMAPINFOHEADER);
    pbi->biWidth = 640;
    pbi->biHeight = 480;
    pbi->biPlanes = 1;
    pbi->biBitCount = depth;
    pbi->biCompression = BI_RGB;
    pbi->biSizeImage = 0;
    pbi->biXPelsPerMeter = 0;
    pbi->biYPelsPerMeter = 0;
    pbi->biClrUsed = 0;
    pbi->biClrImportant = 0;

    hdc = GetDC(ghMainWindow);
    hbitmap = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS, (VOID**)&pbitmap, 0, 0);
    SetDIBits(hdc, hbitmap, 0, 480, pbits, pbmi, DIB_RGB_COLORS);

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, hbitmap);

    if (fullScreen)
    {
        xofs = yofs = 0;
    }
    else
    {
        xofs = (MAIN_WindowWidth  - 640) / 2;
        yofs = (MAIN_WindowHeight - 480) / 2;
    }

    BitBlt(hdc, xofs, yofs, 640, 480, hdcTemp, 0, 0, SRCCOPY);

    DeleteDC(hdcTemp);
    ReleaseDC(ghMainWindow, hdc);

    DeleteObject(hbitmap);
}

/*-----------------------------------------------------------------------------
    Name        : binkDisplay16
    Description : Windows-specific 16bit RGB555 frame display
    Inputs      : ...
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void binkDisplay16(binkDisplayCallback_proc callback)
{
    binkDisplayWin32(callback, 16);
}

/*-----------------------------------------------------------------------------
    Name        : binkDisplay32
    Description : Windows-specific 32bit frame display
    Inputs      : ...
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void binkDisplay32(binkDisplayCallback_proc callback)
{
    binkDisplayWin32(callback, 32);
}

/*-----------------------------------------------------------------------------
    Name        : binkDecodeSimply
    Description : simple frame decoder
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void binkDecodeSimply(void)
{
    if (systemActive && !BinkWait(bnk))
    {
        binkDecodeFrameCount++;
        binkNextFrame();
        binkUpdate = TRUE;
        binkDisplayFlags = BINKSURFACECOPYALL;

        if (g_decodeProc != NULL)
        {
            g_decodeProc(binkDecodeFrameCount);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : binkDecodeTimerProc
    Description : mmtimer callback proc for decoding a frame of Bink video
    Inputs      : [all ignored]
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CALLBACK binkDecodeTimerProc(UINT uid, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    if (!systemActive)
    {
        binkUpdate = FALSE;
        binkStopNow = TRUE;
        return;
    }

    if (inDecode || binkStopNow)
    {
        if (binkStopNow)
        {
            binkUpdate = FALSE;
        }
        return;
    }

    inDecode = TRUE;

    soundEventUpdate();
    speechEventUpdate();

    if (bnk->FrameNum < 2)
    {
        binkTimeReset();
        binkDecodeSimply();
        inDecode = FALSE;
        return;
    }

    if (systemActive)
    {
        sdword diff, i;
        sdword idealFrame = binkIdealFrame();

        (void)BinkWait(bnk);

        if (idealFrame >= bnk->Frames)
        {
            idealFrame = bnk->Frames - 1;
            binkStopNow = TRUE;
            inDecode = FALSE;
            binkUpdate = FALSE;
            if (binkTimerHandle != 0)
            {
                (void)timeKillEvent(binkTimerHandle);
                binkTimerHandle = 0;
            }
            return;
        }

        diff = idealFrame - bnk->FrameNum;
        if (diff < 0)
        {
            diff = 0;
        }
        binkDisplayFlags = (diff > 1) ? BINKSURFACECOPYALL : 0;
        for (i = 0; i < diff; i++)
        {
            if ((i > 0) && (!(i & 3)))
            {
                (void)BinkWait(bnk);
            }

            binkDecodeFrameCount++;
            binkNextFrame();
            binkUpdate = TRUE;

            if (binkStopNow)
            {
                break;
            }
        }

        if (g_decodeProc != NULL)
        {
            g_decodeProc(binkDecodeFrameCount);
        }
    }

    if (binkStopNow)
    {
        binkUpdate = FALSE;
    }

    inDecode = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : binkPlayLoop
    Description : playback loop (message handling) for playing Bink video files
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void binkPlayLoop(void)
{
    MSG  msg;
    BOOL gotMsg;
    TIMECAPS ptc;

    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

    timeGetDevCaps((LPTIMECAPS)&ptc, sizeof(TIMECAPS));
#if USE_TIMER
    binkTimerHandle = timeSetEvent(ptc.wPeriodMin, 0, binkDecodeTimerProc, 0, TIME_PERIODIC);
#else
    binkTimerHandle = 0;
#endif
    decodeTimer = (binkTimerHandle == 0) ? FALSE : TRUE;

    if (g_trackNum != -1)
    {
        soundEventPlayMusic(g_trackNum);
    }

    for (;;)
    {
        Sleep(0);

        if (systemActive)
        {
            gotMsg = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        }
        else
        {
            gotMsg = GetMessage(&msg, NULL, 0, 0);
        }

        BinkService(bnk);

        if (gotMsg)
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (systemActive && !binkIsPaused)
        {
            if (binkUpdate)
            {
                binkUpdate = FALSE;
                g_displayProc(binkUserPaintCallback);
                binkDisplayFrameCount++;
            }
            Sleep(0);
            if (!decodeTimer)
            {
                binkDecodeTimerProc(0, 0, 0, 0, 0);
            }
        }

        if (binkStopNow)
        {
            break;
        }
    }

    if (binkTimerHandle != 0)
    {
        (void)timeKillEvent(binkTimerHandle);
        binkTimerHandle = 0;
    }

    if (g_trackNum != -1)
    {
        soundEventStopMusic(0.0f);
    }
}

/*-----------------------------------------------------------------------------
    Name        : binkPlay
    Description : playback a Bink video file
    Inputs      : ...
    Outputs     :
    Return      : TRUE or FALSE (success or failure)
----------------------------------------------------------------------------*/
bool binkPlay(char* filename,
              binkDisplay_proc displayProc,
              binkDecodeCallback_proc decodeProc,
              sdword surfType, bool rev, sdword trackNum)
{
    if (!systemActive)
    {
        return FALSE;
    }
    g_trackNum = trackNum;
    if (trackNum != -1)
    {
        soundEventUpdate();
        speechEventUpdate();
        Sleep(150);
        soundEventUpdate();
        speechEventUpdate();
    }

    binkStopNow = FALSE;
    binkUpdate = FALSE;
    binkIsPaused = FALSE;

    binkFrameAdd = 0;

    if (!binkOpen(filename))
    {
        return FALSE;
    }

    binkDecodeFrameCount = 0;
    binkDisplayFrameCount = 0;
    binkDisplayFlags = BINKSURFACECOPYALL;

    binkDonePlaying = FALSE;
    binkPlaying = TRUE;

    g_surfType = surfType;
    if (displayProc == NULL)
    {
        g_displayProc = (hwGetDepth() > 16) ? binkDisplay32 : binkDisplay16;
    }
    else
    {
        g_displayProc = displayProc;
    }
    g_decodeProc = decodeProc;
    binkPlayLoop();

    binkPlaying = FALSE;

    BinkClose(bnk);

    binkDonePlaying = TRUE;

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : binkStop
    Description : flag video playback to stop
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE (currently cannot fail)
----------------------------------------------------------------------------*/
bool binkStop(void)
{
    binkStopNow = TRUE;
    if (binkTimerHandle != 0)
    {
        (void)timeKillEvent(binkTimerHandle);
        binkTimerHandle = 0;
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : binkPause
    Description : un/pause video playback
    Inputs      : pause - TRUE or FALSE, pause or continue
    Outputs     :
    Return      : TRUE or FALSE (currently cannot fail)
----------------------------------------------------------------------------*/
bool binkPause(bool pause)
{
    if (pause)
    {
        binkIsPaused = TRUE;
    }
    else
    {
        binkIsPaused = FALSE;
        //reset ideal time
        binkTimeReset();
        binkFrameAdd = bnk->FrameNum;
    }
    return TRUE;
}
