// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>

#include "wave.h"
#include "avi.h"

char *szClassName="HWWaveClass";
char *szWndName="HWWaveWnd";

int nModeFlag;

short nBufIndex;		// controls which wave buffer we're using

BOOL bDevAvail;
BOOL bDevOpen;
BOOL bDataEnd;

HANDLE hWaveThread;
HWND hWaveWnd;

WAVEFORMATEX wfWaveFormat;	// wave format structure

char aWaveBuf[2][WAVE_BUFSIZE];	// wave data buffers
WAVEHDR whWaveHead[2];	// wave headers
HWAVEOUT hWaveOut;

DWORD dwBufSize;		// size of buffer as a multiple of the nBlockAlign header field

/*
 Allocate format and wave headers and data buffers
*/
int InitWave(DWORD *dwNumDevs)
{
    DWORD	dwThreadID;

	bDevAvail=FALSE;

	// find a device compatible with the available wave characteristics...
	*dwNumDevs=(DWORD)waveInGetNumDevs();
	if(*dwNumDevs == 0) return -1;

	bDevAvail=TRUE;

	// initialize wave format structure...
	if(InitWaveFormat() < 0) return -2;
	
	// create thread to handle wave window messages
	hWaveThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ExecWaveThread,NULL,0,&dwThreadID);
	if(hWaveThread == NULL) return -3;

	return 0;
}

/*
 Free the memory associated with the wave buffers.
*/
int EndWave()
{
	// stop 
	StopWavePlay();

	// check wave thread status
	if(CheckWaveThread() == ERR) return -1;
	
	// destroy wave window and terminate thread
	SendMessage(hWaveWnd,WM_CLOSE,0,0);

	// release thread handle
	CloseHandle(hWaveThread);
	
	bDevAvail=FALSE;

	return 0;
}

/*
 Initialize WAVEFORMATEX structure
*/
int InitWaveFormat()
{
	// initialize the format to 2-channel, 16-bit, 22.05 KHz PCM...
	memset(&wfWaveFormat,0,sizeof(WAVEFORMATEX));

	wfWaveFormat.wFormatTag=WAVE_FORMAT_PCM;
	wfWaveFormat.nChannels=WAVE_NUMCHAN;
	wfWaveFormat.nSamplesPerSec=WAVE_SAMPLERATE;
	wfWaveFormat.wBitsPerSample=WAVE_BITSAMPLE;
	wfWaveFormat.nBlockAlign=wfWaveFormat.nChannels*wfWaveFormat.wBitsPerSample/8;
	wfWaveFormat.nAvgBytesPerSec=wfWaveFormat.nSamplesPerSec*wfWaveFormat.nBlockAlign;
	
	return 0;
}
 
/*
 Zero out the wave headers and initialize the data pointers and buffer lengths.
*/
int InitWaveHeaders()
{
	// make the wave buffer size a multiple of the block align...
	dwBufSize=(WAVE_BUFSIZE-(WAVE_BUFSIZE%wfWaveFormat.nBlockAlign));

	// zero out the wave headers...	
	memset(&whWaveHead[0],0,sizeof(WAVEHDR));
	memset(&whWaveHead[1],0,sizeof(WAVEHDR));
	
	// now init the data pointers and buffer lengths...
	whWaveHead[0].dwBufferLength=dwBufSize;
	whWaveHead[1].dwBufferLength=dwBufSize;
	whWaveHead[0].lpData=aWaveBuf[0];
	whWaveHead[1].lpData=aWaveBuf[1];

	// prepare the headers...
	if((waveOutPrepareHeader(hWaveOut,&whWaveHead[0],sizeof(WAVEHDR)) != MMSYSERR_NOERROR) ||
		 (waveOutPrepareHeader(hWaveOut,&whWaveHead[1],sizeof(WAVEHDR)) != MMSYSERR_NOERROR))
			return -1;

	return 0;
}

/*
 Prepare headers, add buffer, and start recording.
*/
int StartWavePlay(DWORD dwDeviceID)
{
	// if no devices, just return
	if(bDevAvail == FALSE) return 0;

	// if the device is still open, just return...
	if(bDevOpen == TRUE) return 0;
	
	nBufIndex=0;
	
	// open the device for recording...
	if(waveOutOpen(&hWaveOut,(UINT)dwDeviceID,(LPWAVEFORMATEX)&wfWaveFormat,(DWORD)hWaveWnd,0,CALLBACK_WINDOW|WAVE_ALLOWSYNC) != MMSYSERR_NOERROR) return -1;
	
	bDevOpen=TRUE;
	
	// initialize the headers...
	if(InitWaveHeaders() < 0)
	{
		CloseWavePlay();
		return -2;
	}

	bDataEnd=FALSE;

	// write the first buffer to start playing,..
	if(QueueWaveBuffer() < 0)
	{
		CloseWavePlay();
		return -3;
	}
	
	nModeFlag=PLAY;
	
	// and queue the next buffer up...
	QueueWaveBuffer();

	return 0;
}

/*
 Write the buffer to the wave device and toggle buffer index.
*/
int QueueWaveBuffer()
{
	// fill the wave buffer with data...
	if(ReadWaveBuffer() < 0) return 0;

	// reset flags field (remove WHDR_DONE attribute)...
	whWaveHead[nBufIndex].dwFlags=(DWORD)WHDR_PREPARED;
		
	// now queue the buffer for output...
	if(waveOutWrite(hWaveOut,&whWaveHead[nBufIndex],sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		StopWavePlay();
		return -1;
	}

	// toggle for next buffer...
	nBufIndex=1-nBufIndex;
	
	return 0;
}

/*
 Read a wave chunk
*/
int ReadWaveBuffer()
{
	DWORD dwBlockSize;

	// set size
	dwBlockSize=dwBufSize/wfWaveFormat.nBlockAlign;

	// get avi blocks or mute blocks
	if(aviGetSamples(aWaveBuf[nBufIndex],&dwBlockSize,dwBufSize) == FALSE)
	{
		bDataEnd=TRUE;
		return -1;
	}

	return 0;
}

/*
 Stop the wave playing.
*/
int StopWavePlay()
{
	// if no devices, just return
	if(bDevAvail == FALSE) return 0;

	// if the device isn't open, just return...
	if(bDevOpen == FALSE) return 0;

	// stop playing on next callback...
	if(bDataEnd == FALSE)
	{
		bDataEnd=TRUE;
		return 0;
	}

	// stop playing...
	waveOutReset(hWaveOut);

	bDataEnd=FALSE;
	nModeFlag=STOP;
	
	// close the device and unprepare the headers...
	CloseWavePlay();

	return 0;
}

/*
 Close the wave output device.
*/
int CloseWavePlay()
{
	// unprepare the headers...
	waveOutUnprepareHeader(hWaveOut,&whWaveHead[0],sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut,&whWaveHead[1],sizeof(WAVEHDR));

	// close the device...
	waveOutClose(hWaveOut);
	
	bDevOpen=FALSE;

	return 0;
}

int InitWaveThread(void)
{
	WNDCLASS wc;

	// Set wave window class data
	wc.style=0; 
    wc.lpfnWndProc=WaveWndProc; 
    wc.cbClsExtra=0; 
    wc.cbWndExtra=0; 
    wc.hInstance=NULL; 
    wc.hIcon=NULL;
	wc.hCursor=NULL;
    wc.hbrBackground=NULL; 
	wc.lpszMenuName=NULL; 
	wc.lpszClassName=szClassName; 

	// Register wave window class
    if(RegisterClass(&wc) == 0) return(ERR);

	// Create wave window
    hWaveWnd=CreateWindow(szClassName,szWndName,0,0,0,0,0, NULL,NULL,NULL,NULL); 
    if(hWaveWnd == NULL) 
	{
		CleanWaveThread();
		return(ERR);
	}
 
	return(OK);
}

DWORD WINAPI ExecWaveThread(void)
{
    MSG	msg; 

	// Initialize wave thread
	if(InitWaveThread() == ERR) return(1L);
	
	// Execute thread
    while(1)
	{
		// Get and dispatch messages
		if(GetMessage(&msg,NULL,0,0) != 0) DispatchMessage(&msg);
		else break;
	}
 
	// Cleanup wave thread
	if(CleanWaveThread() == ERR) return(1L);

	return(0L);
}

int CleanWaveThread(void)
{
	// Unregister wave window class
	if(UnregisterClass(szClassName,NULL) == 0) return(ERR);

	return(OK);
}

int CheckWaveThread(void)
{
	DWORD	dwExitCode;
	
	// Get wave thread exit code
	if(GetExitCodeThread(hWaveThread,&dwExitCode) == 0) return(ERR);

	// Check wave thread exit code
	if(dwExitCode != STILL_ACTIVE) return(ERR);

	// Check wave window handle
	if(IsWindow(hWaveWnd) == 0) return(ERR);

	return(OK);
}

// Windows procedure for MM_WOM_DONE & MM_WOM_CLOSE messages
LRESULT CALLBACK WaveWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
		case MM_WOM_DONE:
			// stop playing...
			if(bDataEnd == TRUE)
				StopWavePlay();

			// or queue next buffer...
			else QueueWaveBuffer();
			break;

		case MM_WOM_CLOSE:
			bDevOpen=FALSE;
			break;

		// Cleanup
		case WM_DESTROY: 
			PostQuitMessage(0L); 
			break;

		default:
			return(DefWindowProc(hWnd,uMsg,wParam,lParam)); 
	}

	return(0L);
}
