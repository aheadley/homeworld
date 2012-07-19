// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <ddraw.h>
#include "directdraw.h"
#include "render.h"

DDCONT *ddInit(HWND hWnd)
{
	DDCONT              *pDdc;
    HRESULT		        hRet;
    DDSURFACEDESC       ddsd;
	LPDIRECTDRAW        pDD;
    LPDIRECTDRAWCLIPPER pClipper;

	pDdc=(DDCONT *)malloc(sizeof(DDCONT));
	if(pDdc == NULL) return(NULL);

    pDdc->hWnd = hWnd;

    // Create the main DirectDraw object
    hRet = DirectDrawCreate( NULL, &pDD, NULL);
    if (hRet != DD_OK)
	{
		free(pDdc);
		return(NULL);
	}

    // Fetch DirectDraw interface
    hRet = pDD->QueryInterface(IID_IDirectDraw, (LPVOID *)&pDdc->pDD);
    if (hRet != DD_OK)
	{
		free(pDdc);
		return(NULL);
	}

    // Get normal windowed mode
    hRet = pDdc->pDD->SetCooperativeLevel(pDdc->hWnd, DDSCL_NORMAL);
    if (hRet != DD_OK)
	{
		free(pDdc);
		return(NULL);
	}

    // Get the dimensions of the viewport and screen bounds
    GetClientRect(pDdc->hWnd, &pDdc->rcViewport);
    GetClientRect(pDdc->hWnd, &pDdc->rcScreen);
    ClientToScreen(pDdc->hWnd, (POINT*)&pDdc->rcScreen.left);
    ClientToScreen(pDdc->hWnd, (POINT*)&pDdc->rcScreen.right);

    // Create the primary surface
    ZeroMemory(&ddsd,sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    hRet = pDdc->pDD->CreateSurface(&ddsd, &pDdc->pDDSPrimary, NULL);
    if (hRet != DD_OK)
	{
		free(pDdc);
		return(NULL);
	}

    // Create a clipper object since this is for a Windowed render
    hRet = pDdc->pDD->CreateClipper(0, &pClipper, NULL);
    if (hRet != DD_OK)
	{
		free(pDdc);
		return(NULL);
	}

    // Associate the clipper with the window
    pClipper->SetHWnd(0, pDdc->hWnd);
    pDdc->pDDSPrimary->SetClipper(pClipper);
    pClipper->Release();
    pClipper = NULL;

    // Get the backbuffer
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth        = pDdc->rcViewport.right-pDdc->rcViewport.left+1;
    ddsd.dwHeight       = pDdc->rcViewport.bottom-pDdc->rcViewport.top+1;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    hRet = pDdc->pDD->CreateSurface(&ddsd, &pDdc->pDDSBack, NULL);
    if (hRet != DD_OK)
	{
		free(pDdc);
		return(NULL);
	}

    return(pDdc);
}

int ddClean(DDCONT *pDdc)
{
    if (pDdc->pDD != NULL)
    {
        if (pDdc->pDDSBack != NULL)
        {
            pDdc->pDDSBack->Release();
            pDdc->pDDSBack = NULL;
        }
        if (pDdc->pDDSPrimary != NULL)
        {
            pDdc->pDDSPrimary->Release();
            pDdc->pDDSPrimary = NULL;
        }
    }

    // Release the main DDraw interface
    if (pDdc->pDD)
        pDdc->pDD->Release();

	free(pDdc);

	return(OK);
}

int ddUpdate(DDCONT *pDdc,void *pContext)
{
	HDC         hDC;
    HRESULT		hRet;
    DDBLTFX     ddbltfx;

    // Retrieve the window position after a move
    GetClientRect(pDdc->hWnd, &pDdc->rcScreen);
    ClientToScreen(pDdc->hWnd, (POINT*)&pDdc->rcScreen.left);
    ClientToScreen(pDdc->hWnd, (POINT*)&pDdc->rcScreen.right);

    // Use the blter to do a color fill to clear the back buffer
    ddbltfx.dwSize = sizeof(DDBLTFX);
    ddbltfx.dwFillColor = 0;
    pDdc->pDDSBack->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

    if (pDdc->pDDSBack->GetDC(&hDC) == DD_OK)
    {
		// Draw
		if(rendRendCont((RENDCONT *)pContext,hDC) < 0)
		{
	        pDdc->pDDSBack->ReleaseDC(hDC);
			return(ERR);
		}

        pDdc->pDDSBack->ReleaseDC(hDC);
    }

	hRet = pDdc->pDDSBack->IsLost();
	if (hRet != DD_OK) pDdc->pDDSBack->Restore();

	hRet = pDdc->pDDSPrimary->IsLost();
	if (hRet != DD_OK) pDdc->pDDSPrimary->Restore();
    
	// Perform a blt.
	while(1)
    {
		hRet = pDdc->pDDSPrimary->Blt(&pDdc->rcScreen, pDdc->pDDSBack,
                              &pDdc->rcViewport, DDBLT_WAIT,
                              NULL);
		if (hRet != DDERR_WASSTILLDRAWING ) break;
	}

    return(OK);
}

int ddResize(DDCONT *pDdc)
{
    HRESULT		        hRet;
    DDSURFACEDESC       ddsd;
	DWORD               dwIHeight,dwIWidth,dwFHeight,dwFWidth;

	dwIWidth  = pDdc->rcViewport.right-pDdc->rcViewport.left+1;
	dwIHeight = pDdc->rcViewport.bottom-pDdc->rcViewport.top+1;
    
    // Retrieve the window position after a move
    GetClientRect(pDdc->hWnd, &pDdc->rcViewport);

	dwFWidth  = pDdc->rcViewport.right-pDdc->rcViewport.left+1;
	dwFHeight = pDdc->rcViewport.bottom-pDdc->rcViewport.top+1;

	if((dwIWidth  == dwFWidth) && (dwIHeight == dwFHeight)) return(OK);
    
	if (pDdc->pDDSBack != NULL)
    {
        pDdc->pDDSBack->Release();
        pDdc->pDDSBack = NULL;
    }

    // Get the backbuffer
    ZeroMemory(&ddsd,sizeof(ddsd));
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth        = dwFWidth;
    ddsd.dwHeight       = dwFHeight;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    hRet = pDdc->pDD->CreateSurface(&ddsd, &pDdc->pDDSBack, NULL);
    if (hRet != DD_OK) return(ERR);

	return(OK);
}

int ddPaint(DDCONT *pDdc)
{
    HRESULT         hRet;

    // Retrieve the window position after a move
    GetClientRect(pDdc->hWnd, &pDdc->rcScreen);
    ClientToScreen(pDdc->hWnd, (POINT*)&pDdc->rcScreen.left);
    ClientToScreen(pDdc->hWnd, (POINT*)&pDdc->rcScreen.right);

	hRet = pDdc->pDDSBack->IsLost();
	if (hRet != DD_OK) pDdc->pDDSBack->Restore();

	hRet = pDdc->pDDSPrimary->IsLost();
	if (hRet != DD_OK) pDdc->pDDSPrimary->Restore();
    
	// Perform a blt.
	while(1)
    {
		hRet = pDdc->pDDSPrimary->Blt(&pDdc->rcScreen, pDdc->pDDSBack,
                              &pDdc->rcViewport, DDBLT_WAIT,
                              NULL);
		if (hRet != DDERR_WASSTILLDRAWING ) break;
	}

	return(OK);
}
