// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef WAVE_H
#define WAVE_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

// Other constants
#define STOP	0
#define PLAY	1
                
#define WAVE_BUFSIZE 32768		// buffer about 32K
#define WAVE_SAMPLERATE	22050	// sample rate
#define WAVE_BITSAMPLE	16		// bits per sample
#define WAVE_NUMCHAN	2		// number of channels

// External functions 
#ifdef __cplusplus
extern "C" {
#endif

int InitWave(DWORD *dwNumDevs);
int EndWave();

int StartWavePlay(DWORD nDeviceID);
int StopWavePlay();

#ifdef __cplusplus
}
#endif

// Internal functions 
int AllocWaveFormat();
int FreeWaveFormat();

int AllocWaveHeaders();
int InitWaveHeaders();
int FreeWaveHeaders();

int AllocWaveBuffers();
int FreeWaveBuffers();

int CloseWavePlay();
int QueueWaveBuffer();
int ReadWaveBuffer();

int GetFormatTagDetails(WORD wFormatTag);
int GetFormatDetails(LPWAVEFORMATEX pFormatIn);

// Thread functions
int InitWaveThread(void);
int CleanWaveThread(void);
int CheckWaveThread(void);
DWORD WINAPI ExecWaveThread(void);
LRESULT CALLBACK WaveWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

#endif