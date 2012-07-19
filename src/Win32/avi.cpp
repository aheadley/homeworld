/*=============================================================================
    Name    : avi.cpp
    Purpose : routines for playing AVI files

    Created 1/9/1999 by jdorie, khent, jjoire
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <windows.h>
#include <vfw.h>
#include "avi.h"
#include "wave.h"

extern "C" int fullScreen;
extern "C" void* ghMainWindow;
extern "C" int MAIN_WindowWidth;
extern "C" int MAIN_WindowHeight;
extern "C" int systemActive;

PAVISTREAM    g_VidStream;      //the AVI video stream
AVISTREAMINFO g_VidStreamInfo;  //info about the AVI stream

PAVISTREAM    g_AudStream;      //the AVI audio stream
AVISTREAMINFO g_AudStreamInfo;  //info about the AVI stream

long          g_dwCurrFrame;    //current frame of the AVI
long          g_dwCurrSample;   //current sample of the AVI
PGETFRAME     g_pFrame;         //an object to hold info about the video frame when we get it

double g_FramesPerSec;
double g_SamplesPerSec;

BOOL g_bMoreFrames;

int aviDonePlaying = 1;
int aviIsPlaying = 0;
int aviHasAudio = 0;

HBITMAP g_hBitmap = 0;
WORD*   g_pBitmap = 0;

MMRESULT g_timerHandle = NULL;

BOOL aviVerifyResult(HRESULT Result)
{
    if (Result == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL aviStart(char* filename)
{
    HRESULT Res;

    //initialize the AVI library
    AVIFileInit();

    Res = AVIStreamOpenFromFile(&g_VidStream, filename, streamtypeVIDEO, 0, OF_READ, NULL);
    if (!aviVerifyResult(Res))
    {
        return FALSE;
    }

    g_pFrame = AVIStreamGetFrameOpen(g_VidStream, NULL);
    if (g_pFrame == NULL)
    {
        return FALSE;
    }

    Res = AVIStreamInfo(g_VidStream, &g_VidStreamInfo, sizeof(AVISTREAMINFO));
    if (!aviVerifyResult(Res))
    {
        return FALSE;
    }

    //now grab the audio stream and its info
    Res = AVIStreamOpenFromFile(&g_AudStream, filename, streamtypeAUDIO, 0, OF_READ, NULL);
    if (!aviVerifyResult(Res))
    {
        return FALSE;
    }

    Res = AVIStreamInfo(g_AudStream, &g_AudStreamInfo, sizeof(AVISTREAMINFO));
    if (!aviVerifyResult(Res))
    {
        return FALSE;
    }

    //convert the "rate and scale" values into meaningful numbers
    g_FramesPerSec  = (double)g_VidStreamInfo.dwRate / (double)g_VidStreamInfo.dwScale;
    g_SamplesPerSec = (double)g_AudStreamInfo.dwRate / (double)g_AudStreamInfo.dwScale;

	//check for compressed audio
	if((g_SamplesPerSec == (double)WAVE_SAMPLERATE)
		&& (g_AudStreamInfo.dwRate == WAVE_SAMPLERATE*(WAVE_BITSAMPLE/8)*WAVE_NUMCHAN)
		&& (g_AudStreamInfo.dwScale == (WAVE_BITSAMPLE/8)*WAVE_NUMCHAN))
			aviHasAudio = TRUE;

    //init the frame and sample numbers

    g_dwCurrFrame  = 0;
    g_dwCurrSample = 0;

    //so good so far
    return TRUE;
}

BOOL aviGetNextFrame(BITMAPINFO** ppbmi)
{
    *ppbmi = (LPBITMAPINFO)AVIStreamGetFrame(g_pFrame, g_dwCurrFrame);
    g_dwCurrFrame++;

    //any more frames left?
    return !(g_dwCurrFrame >= g_VidStreamInfo.dwLength);
}

int aviGetSamples(void* pBuf, long* pNumSamples, long nBufSize)
{
    HRESULT Res;
    long nSampRead;

	if(aviHasAudio == FALSE) return FALSE;

	if(aviIsPlaying == FALSE)
	{
		memset(pBuf,0,nBufSize);
		return TRUE;
	}

    Res = AVIStreamRead(g_AudStream, g_dwCurrSample, *pNumSamples, pBuf, nBufSize, NULL, &nSampRead);
    if (!aviVerifyResult(Res))
    {
        return FALSE;
    }

    *pNumSamples = nSampRead;
    g_dwCurrSample += nSampRead;

	return TRUE;
}

void aviResetStream(void)
{
    g_dwCurrFrame  = 0;
    g_dwCurrSample = 0;
}

int aviStop(void)
{
    HRESULT Res;

    g_bMoreFrames = FALSE;
    aviDonePlaying = TRUE;

    if (g_timerHandle != NULL)
    {
        (void)timeKillEvent(g_timerHandle);
        g_timerHandle = NULL;
    }

    Res = AVIStreamGetFrameClose(g_pFrame);
    if (!aviVerifyResult(Res))
    {
        return FALSE;
    }

    Res = AVIStreamRelease(g_VidStream);
    if (!aviVerifyResult(Res))
    {
        return FALSE;
    }

    Res = AVIStreamRelease(g_AudStream);
    if (!aviVerifyResult(Res))
    {
        return FALSE;
    }

    AVIFileExit();

    return TRUE;
}

void aviShowFrame(BITMAPINFO* pMap)
{
    HDC hdc, hdcTemp;
    int XbmSize, YbmSize;
    int xOffset, yOffset;

    XbmSize = pMap->bmiHeader.biWidth;
    YbmSize = pMap->bmiHeader.biHeight;

    hdc = GetDC(ghMainWindow);
    g_hBitmap = CreateDIBSection(hdc, pMap, DIB_RGB_COLORS, (VOID**)&g_pBitmap, NULL, NULL);
    SetDIBits(hdc, g_hBitmap, 0, YbmSize, (BYTE*)(&pMap->bmiColors), pMap, DIB_RGB_COLORS);

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, g_hBitmap);

    if (!fullScreen &&
        (XbmSize != MAIN_WindowWidth || YbmSize != MAIN_WindowHeight))
    {
        xOffset = (MAIN_WindowWidth  - XbmSize) / 2;
        yOffset = (MAIN_WindowHeight - YbmSize) / 2;
    }
    else
    {
        xOffset = 0;
        yOffset = 0;
    }
    BitBlt(hdc, xOffset, yOffset, XbmSize, YbmSize, hdcTemp, 0, 0, SRCCOPY);

    DeleteDC(hdcTemp);
    ReleaseDC(ghMainWindow, hdc);

    DeleteObject(g_hBitmap);
}

BITMAPINFO* g_pbmi = NULL;

void CALLBACK aviTimeProc(UINT uid, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (g_bMoreFrames)
	{
		g_bMoreFrames = aviGetNextFrame(&g_pbmi);
	}
	else
	{
		aviDonePlaying = TRUE;
	}
}

void aviDisplayFrame()
{
    if (g_pbmi != NULL)
    {
        aviShowFrame(g_pbmi);
    }

	if(aviHasAudio == TRUE)
	{
		//give audio thread a break :)
		Sleep(0);
	}
}

void aviPlayLoop()
{
    MSG msg;
    BOOL bGotMsg;
    int frameCount = 0;

    PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	g_timerHandle = timeSetEvent(
				 (long)(1.0f / g_FramesPerSec * 1000.0f),
				 0,
				 aviTimeProc,
				 NULL,
				 TIME_PERIODIC);
	if (g_timerHandle == NULL)
	{
		return;
	}

    while (msg.message != WM_QUIT)
    {
        if (aviDonePlaying)
        {
            return;
        }

        if (systemActive)
        {
            bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
        }
        else
        {
            bGotMsg = GetMessage(&msg, NULL, 0U, 0U);
        }

        if (bGotMsg)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (g_dwCurrFrame != frameCount)
            {
                frameCount = g_dwCurrFrame;
                aviDisplayFrame();
            }
			if (aviDonePlaying)
			{
                aviDisplayFrame();  //display final frame

				(void)timeKillEvent(g_timerHandle);
                g_timerHandle = NULL;
			}
        }
   }
}

int aviPlay(char* filename)
{
    char  fullname[1024];
    char* dir;

	dir = getenv("HW_Data");
    if (dir == NULL)
    {
        strcpy(fullname, "Movies\\");
    }
    else
    {
        strcpy(fullname, dir);
        strcat(fullname, "\\Movies\\");
    }
    strcat(fullname, filename);

    //try Homeworld\Data\Movies first
    if (!aviStart(fullname))
    {
        //try current directory next
        if (!aviStart(filename))
        {
            return FALSE;
        }
    }

	if(aviHasAudio == TRUE)
	{
		//start audio playback
		if(StartWavePlay(WAVE_MAPPER) < 0)
        {
            //proceed without audio if something bad happened
            aviHasAudio = FALSE;
        }
        else
        {
    		//wait to start video playback
    		Sleep(250);
        }
	}

    g_bMoreFrames = TRUE;
    aviIsPlaying = TRUE;
    aviDonePlaying = FALSE;
    aviPlayLoop();
    aviIsPlaying = FALSE;

	if(aviHasAudio == TRUE)
	{
		//stop audio playback
		if(StopWavePlay() < 0) return FALSE;

		//wait for audio device to stop
		Sleep(250);
	}

    return TRUE;
}

int aviInit()
{
	DWORD nNum;

	//initialize audio
	if(InitWave(&nNum) < 0) return FALSE;

    return TRUE;
}

int aviCleanup()
{
	//cleanup audio
	if(EndWave() < 0) return FALSE;

    return TRUE;
}
