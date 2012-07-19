// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef DD_H
#define DD_H

// Structure typecasts
typedef struct ddcont_stc
{
	HWND                    hWnd;			// Window handle
	LPDIRECTDRAW            pDD;			// DirectDraw object
	LPDIRECTDRAWSURFACE     pDDSPrimary;	// DirectDraw primary surface
	LPDIRECTDRAWSURFACE     pDDSBack;		// DirectDraw back surface
	RECT                    rcViewport;		// Pos. & size to blt from
	RECT                    rcScreen;		// Screen pos. for blt
}
DDCONT;

// Function declarations
#ifdef __cplusplus
extern "C" {
#endif

DDCONT *ddInit(HWND hWnd);
int ddClean(DDCONT *pDdc);
int ddUpdate(DDCONT *pDdc,void *pContext);
int ddResize(DDCONT *pDdc);
int ddPaint(DDCONT *pDdc);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif
