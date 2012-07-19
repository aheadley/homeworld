/*=============================================================================
    Name    : d3dinit.cpp
    Purpose : Direct3D initialization

    Created 10/3/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"
#include "d3dlist.h"
#include "d3denum.h"
#include "d3dtex.h"
#include "d3dinit.h"

#include "3dhw.h"

#include <assert.h>

LPDIRECTDRAW lpDirectDraw = NULL;

//initialize directdraw
GLboolean init_direct_draw(d3d_context* d3d)
{
    HRESULT hr;
    DWORD   coopFlags;
    DDCAPS  hal, hel;

    lpDirectDraw = NULL;

    enum_dd_devices(d3d);
    enum_dd_cleanup(d3d);

    if (lpDirectDraw == NULL)
    {
        hr = DirectDrawCreate(NULL, &d3d->ddraw1, NULL);
        if (FAILED(hr))
        {
            errLog("init_direct_draw(DirectDrawCreate)", hr);
            return GL_FALSE;
        }
    }
    else
    {
        d3d->ddraw1 = lpDirectDraw;
        lpDirectDraw = NULL;
    }

    hr = d3d->ddraw1->QueryInterface(IID_IDirectDraw4, (void**)&d3d->ddraw4);
    if (FAILED(hr))
    {
        errLog("init_direct_draw(QueryInterface)", hr);
        return GL_FALSE;
    }

    ZeroMemory(&hal, sizeof(DDCAPS));
    ZeroMemory(&hel, sizeof(DDCAPS));
    hal.dwSize = sizeof(DDCAPS);
    hel.dwSize = sizeof(DDCAPS);
    hr = d3d->ddraw4->GetCaps(&hal, &hel);
    if (FAILED(hr))
    {
        errLog("init_direct_draw(GetCaps)", hr);
        return GL_FALSE;
    }
    if (hal.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
    {
        d3d->canRenderWindowed = GL_TRUE;
    }
    else
    {
        d3d->canRenderWindowed = GL_FALSE;
        d3d->Fullscreen = GL_TRUE;
    }

    if ((hal.dwCaps2 & DDCAPS2_CANCALIBRATEGAMMA) &&
        (hal.dwCaps2 & DDCAPS2_PRIMARYGAMMA))
	{
		d3d->canGammaControl = GL_TRUE;
	}
	else
	{
		d3d->canGammaControl = GL_FALSE;
	}

    d3d->hwnd = (HWND)rglCreate0();
    if (d3d->Fullscreen)
    {
        coopFlags = DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
    }
    else
    {
        coopFlags = DDSCL_NORMAL;
    }
    hr = d3d->ddraw4->SetCooperativeLevel(d3d->hwnd, coopFlags);
    if (FAILED(hr))
    {
        errLog("init_direct_draw(SetCooperativeLevel)", hr);
        return GL_FALSE;
    }

    return GL_TRUE;
}

#if PAGE_FLIPPING
GLboolean init_surfaces_fullscreen(d3d_context* d3d)
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    DDSCAPS2 ddscaps;

    int width = CTX->Buffer.Width;
    int height = CTX->Buffer.Height;
    int depth = CTX->Buffer.Depth;

    SetRect(&d3d->ViewportRect, 0, 0, width, height);
    MEMCPY(&d3d->ScreenRect, &d3d->ViewportRect, sizeof(RECT));

    hr = d3d->ddraw4->SetDisplayMode(width, height, depth, 0, 0);
    if (FAILED(hr))
    {
        errLog("init_surfaces_fullscreen(SetDisplayMode)", hr);
        return GL_FALSE;
    }

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
    ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;

    hr = d3d->ddraw4->CreateSurface(&ddsd, &d3d->PrimarySurface, NULL);
    if (FAILED(hr))
    {
        errLog("init_surfaces_fullscreen(CreateSurface)", hr);
        return GL_FALSE;
    }

    if (d3d->canGammaControl)
    {
        hr = d3d->PrimarySurface->QueryInterface(IID_IDirectDrawGammaControl, (LPVOID*)&d3d->GammaControl);
        if (FAILED(hr))
        {
            errLog("init_surfaces_fullscreen(QueryInterface[Gamma])", hr);
            d3d->GammaControl = NULL;
        }
        else
        {
            hr = d3d->GammaControl->GetGammaRamp(0, &d3d->awOldLUT);
            if (FAILED(hr))
            {
                errLog("init_surfaces_fullscreen(GetGammaRamp)", hr);
                d3d->GammaControl->Release();
                d3d->GammaControl = NULL;
            }
            else
            {
                hr = d3d->GammaControl->SetGammaRamp(DDSGR_CALIBRATE, (DDGAMMARAMP*)awHomeworldLUT);
                if (FAILED(hr))
                {
                    errLog("init_surfaces_fullscreen(SetGammaRamp)", hr);
                }
            }
        }
    }
    else
    {
        d3d->GammaControl = NULL;
    }

    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    hr = d3d->PrimarySurface->GetAttachedSurface(&ddscaps, &d3d->BackSurface);
    if (FAILED(hr))
    {
        errLog("init_surfaces_fullscreen(GetAttachedSurface)", hr);
        return GL_FALSE;
    }

    return GL_TRUE;
}
#else
GLboolean init_surfaces_fullscreen(d3d_context* d3d)
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    DDSCAPS2 ddscaps;

    int width = CTX->Buffer.Width;
    int height = CTX->Buffer.Height;
    int depth = CTX->Buffer.Depth;

    SetRect(&d3d->ViewportRect, 0, 0, width, height);
    MEMCPY(&d3d->ScreenRect, &d3d->ViewportRect, sizeof(RECT));

    hr = d3d->ddraw4->SetDisplayMode(width, height, depth, 0, 0);
    if (FAILED(hr))
    {
        errLog("init_surfaces_fullscreen(SetDisplayMode)", hr);
        return GL_FALSE;
    }

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

    hr = d3d->ddraw4->CreateSurface(&ddsd, &d3d->PrimarySurface, NULL);
    if (FAILED(hr))
    {
        errLog("init_surfaces_fullscreen(CreateSurface)", hr);
        return GL_FALSE;
    }

    if (d3d->canGammaControl)
    {
        hr = d3d->PrimarySurface->QueryInterface(IID_IDirectDrawGammaControl, (LPVOID*)&d3d->GammaControl);
        if (FAILED(hr))
        {
            errLog("init_surfaces_fullscreen(QueryInterface[Gamma])", hr);
            d3d->GammaControl = NULL;
        }
        else
        {
            hr = d3d->GammaControl->GetGammaRamp(0, &d3d->awOldLUT);
            if (FAILED(hr))
            {
                errLog("init_surfaces_fullscreen(GetGammaRamp)", hr);
                d3d->GammaControl->Release();
                d3d->GammaControl = NULL;
            }
            else
            {
                hr = d3d->GammaControl->SetGammaRamp(DDSGR_CALIBRATE, (DDGAMMARAMP*)awHomeworldLUT);
                if (FAILED(hr))
                {
                    errLog("init_surfaces_fullscreen(SetGammaRamp)", hr);
                }
            }
        }
    }
    else
    {
        d3d->GammaControl = NULL;
    }

    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    ddsd.dwWidth = CTX->Buffer.Width;
    ddsd.dwHeight = CTX->Buffer.Height;
    hr = d3d->ddraw4->CreateSurface(&ddsd, &d3d->BackSurface, NULL);
    if (FAILED(hr))
    {
        errLog("init_surfaces_fullscreen(GetAttachedSurface)", hr);
        return GL_FALSE;
    }

    return GL_TRUE;
}
#endif //PAGE_FLIPPING

GLboolean init_surfaces_windowed(d3d_context* d3d)
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = d3d->ddraw4->CreateSurface(&ddsd, &d3d->PrimarySurface, NULL);
    if (FAILED(hr))
    {
        errLog("init_surfaces_windowed(CreateSurface)", hr);
        return GL_FALSE;
    }

    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

    GetClientRect(d3d->hwnd, &d3d->ScreenRect);
    GetClientRect(d3d->hwnd, &d3d->ViewportRect);
    ClientToScreen(d3d->hwnd, (POINT*)&d3d->ScreenRect.left);
    ClientToScreen(d3d->hwnd, (POINT*)&d3d->ScreenRect.right);
    ddsd.dwWidth = d3d->ScreenRect.right - d3d->ScreenRect.left;
    ddsd.dwHeight = d3d->ScreenRect.bottom - d3d->ScreenRect.top;

    d3d->GammaControl = NULL;

    hr = d3d->ddraw4->CreateSurface(&ddsd, &d3d->BackSurface, NULL);
    if (FAILED(hr))
    {
        errLog("init_surfaces_windowed(CreateSurface)", hr);
        return GL_FALSE;
    }

    return GL_TRUE;
}

GLboolean init_clipper(d3d_context* d3d)
{
    HRESULT hr;
    LPDIRECTDRAWCLIPPER Clipper;

    hr = d3d->ddraw4->CreateClipper(0, &Clipper, NULL);
    if (FAILED(hr))
    {
        errLog("init_clipper(CreateClipper)", hr);
        return GL_FALSE;
    }

    Clipper->SetHWnd(0, d3d->hwnd);
    d3d->PrimarySurface->SetClipper(Clipper);
    Clipper->Release();

    return GL_TRUE;
}

GLboolean init_depthsurface(d3d_context* d3d, DDPIXELFORMAT* pf)
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
    if (d3d->d3dDeviceGUID == RGB_GUID || d3d->d3dDeviceGUID == REF_GUID)
    {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    }
    else
    {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    }

    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    ddsd.dwWidth = d3d->ScreenRect.right - d3d->ScreenRect.left;
    ddsd.dwHeight = d3d->ScreenRect.bottom - d3d->ScreenRect.top;
    MEMCPY(&ddsd.ddpfPixelFormat, pf, sizeof(DDPIXELFORMAT));

    hr = d3d->ddraw4->CreateSurface(&ddsd, &d3d->DepthSurface, NULL);
    if (FAILED(hr))
    {
        errLog("init_depthsurface(CreateSurface)", hr);
        return GL_FALSE;
    }

    hr = d3d->BackSurface->AddAttachedSurface(d3d->DepthSurface);
    if (FAILED(hr))
    {
        errLog("init_depthbuffer(AddAttachedSurface)", hr);
        d3d->DepthSurface->Release();
        d3d->DepthSurface = NULL;
        return GL_FALSE;
    }

    return GL_TRUE;
}

GLboolean init_depthbuffer(d3d_context* d3d)
{
    DDPIXELFORMAT ddpfDepthBuffer;
    GLboolean rval;

    ZeroMemory(&ddpfDepthBuffer, sizeof(DDPIXELFORMAT));

    loDepthList.erase(loDepthList.begin(), loDepthList.end());
    hiDepthList.erase(hiDepthList.begin(), hiDepthList.end());

    d3d->d3dObject->EnumZBufferFormats(d3d->d3dDeviceGUID, enum_depthbuffer_cb, NULL);

    if (loDepthList.empty() && hiDepthList.empty())
    {
        return GL_FALSE;
    }

    rval = GL_FALSE;

#if DEEP_DB
    for (listpf::iterator i = hiDepthList.begin(); i != hiDepthList.end(); ++i)
    {
        MEMCPY(&ddpfDepthBuffer, &(*i), sizeof(DDPIXELFORMAT));
        if (init_depthsurface(d3d, &ddpfDepthBuffer))
        {
            rval = GL_TRUE;
            break;
        }
    }
#endif

    if (!rval)
    {
        for (listpf::iterator i = loDepthList.begin(); i != loDepthList.end(); ++i)
        {
            MEMCPY(&ddpfDepthBuffer, &(*i), sizeof(DDPIXELFORMAT));
            if (init_depthsurface(d3d, &ddpfDepthBuffer))
            {
                rval = GL_TRUE;
                break;
            }
        }
    }

    loDepthList.erase(loDepthList.begin(), loDepthList.end());
    hiDepthList.erase(hiDepthList.begin(), hiDepthList.end());
    return rval;
}

//power of 2 <= n
GLint MIN_P2(GLint n)
{
    if (n == 0) return 0;
    if (n < 4) return 2;
    if (n < 8) return 4;
    if (n < 16) return 8;
    if (n < 32) return 16;
    if (n < 64) return 32;
    if (n < 128) return 64;
    if (n < 256) return 128;
    if (n < 512) return 256;
    if (n < 1024) return 512;
    return 1024;
}

//power of 2 >= n
GLint MAX_P2(GLint n)
{
    if (n < 2) return n;
    if (n <= 2) return 2;
    if (n <= 4) return 4;
    if (n <= 8) return 8;
    if (n <= 16) return 16;
    if (n <= 32) return 32;
    if (n <= 64) return 64;
    if (n <= 128) return 128;
    if (n <= 256) return 256;
    if (n <= 512) return 512;
    return 1024;
}

#if LOG_DEVICE_CAPS
static void log_device_caps(d3d_context* d3d)
{
    FILE* out;

    out = fopen("d3dDeviceCaps.txt", "wt");
    if (out == NULL)
    {
        return;
    }

    fprintf(out, "canRenderWindowed %d\n", d3d->canRenderWindowed);
    fprintf(out, "canGammaControl %d\n", d3d->canGammaControl);
    fprintf(out, "maxTexAspect %d\n", d3d->maxTexAspect);
    fprintf(out, "minTexWidth %d\n", d3d->minTexWidth);
    fprintf(out, "maxTexWidth %d\n", d3d->maxTexWidth);
    fprintf(out, "minTexHeight %d\n", d3d->minTexHeight);
    fprintf(out, "maxTexHeight %d\n", d3d->maxTexHeight);
    fprintf(out, "canTexModulate %d\n", d3d->canTexModulate);
    fprintf(out, "canTexSelectArg1 %d\n", d3d->canTexSelectArg1);
    fprintf(out, "canTexAdd %d\n", d3d->canTexAdd);
    fprintf(out, "canDither %d\n", d3d->canDither);
    fprintf(out, "canZCmpLess %d\n", d3d->canZCmpLess);
    fprintf(out, "canZCmpLessEqual %d\n", d3d->canZCmpLessEqual);
    fprintf(out, "canSrcBlendSrcAlpha %d\n", d3d->canSrcBlendSrcAlpha);
    fprintf(out, "canSrcBlendOne %d\n", d3d->canSrcBlendOne);
    fprintf(out, "canSrcBlendZero %d\n", d3d->canSrcBlendZero);
    fprintf(out, "canDestBlendInvSrcAlpha %d\n", d3d->canDestBlendInvSrcAlpha);
    fprintf(out, "canDestBlendOne %d\n", d3d->canDestBlendOne);
    fprintf(out, "canDestBlendZero %d\n", d3d->canDestBlendZero);
    fprintf(out, "canAlphaTestGreater %d\n", d3d->canAlphaTestGreater);
    fprintf(out, "canAlphaTestLess %d\n", d3d->canAlphaTestLess);
    fprintf(out, "canPerspectiveCorrect %d\n", d3d->canPerspectiveCorrect);
    fprintf(out, "squareOnly %d\n", d3d->squareOnly);
    fprintf(out, "canFilterLinear %d\n", d3d->canFilterLinear);
    fprintf(out, "canFilterNearest %d\n", d3d->canFilterNearest);
    fprintf(out, "canClamp %d\n", d3d->canClamp);
    fprintf(out, "canWrap %d\n", d3d->canWrap);

    fclose(out);
}
#endif

GLboolean init_d3d(d3d_context* d3d)
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    D3DDEVICEDESC  hal, hel;
    D3DDEVICEDESC* caps;

    hr = d3d->ddraw4->QueryInterface(IID_IDirect3D3, (void**)&d3d->d3dObject);
    if (FAILED(hr))
    {
        errLog("init_d3d(QueryInterface)", hr);
        return GL_FALSE;
    }

    if (!init_depthbuffer(d3d))
    {
        return GL_FALSE;
    }

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    d3d->ddraw4->GetDisplayMode(&ddsd);
    if (ddsd.ddpfPixelFormat.dwRGBBitCount <= 8)
    {
        return GL_FALSE;
    }

    hr = d3d->d3dObject->CreateDevice(d3d->d3dDeviceGUID,
                                      d3d->BackSurface,
                                      &d3d->d3dDevice,
                                      NULL);
    if (FAILED(hr))
    {
        errLog("init_d3d(CreateDevice)", hr);
        return GL_FALSE;
    }

    ZeroMemory(&hal, sizeof(D3DDEVICEDESC));
    ZeroMemory(&hel, sizeof(D3DDEVICEDESC));
    hal.dwSize = sizeof(D3DDEVICEDESC);
    hel.dwSize = sizeof(D3DDEVICEDESC);
    hr = d3d->d3dDevice->GetCaps(&hal, &hel);
    if (FAILED(hr))
    {
        errLog("init_d3d(GetCaps)", hr);
        return GL_FALSE;
    }
    caps = hal.dwFlags ? &hal : &hel;
    if (caps->dwFlags & D3DDD_TRICAPS)
    {
        //texture sizes
        d3d->maxTexAspect = MIN_P2(caps->dwMaxTextureAspectRatio);
        d3d->minTexWidth  = MAX_P2(caps->dwMinTextureWidth);
        d3d->minTexHeight = MAX_P2(caps->dwMinTextureHeight);
        d3d->maxTexWidth  = MIN_P2(caps->dwMaxTextureWidth);
        d3d->maxTexHeight = MIN_P2(caps->dwMaxTextureHeight);

        //tricaps are valid
        D3DPRIMCAPS* tri = &caps->dpcTriCaps;

        //edge antialiasing
        d3d->canAntialiasTriEdges = tri->dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES;
        d3d->canAntialiasTriDep = tri->dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT;
        d3d->canAntialiasTriIndep = tri->dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT;

        //dithering
        d3d->canDither = tri->dwRasterCaps & D3DPRASTERCAPS_DITHER;

        //wbuffer
        d3d->canWBuffer = tri->dwRasterCaps & D3DPRASTERCAPS_WBUFFER;

        //zbias
        d3d->canZBias = tri->dwRasterCaps & D3DPRASTERCAPS_ZBIAS;

        //depthbuffer comparison
        d3d->canZCmpLess = tri->dwZCmpCaps & D3DPCMPCAPS_LESS;
        d3d->canZCmpLessEqual = tri->dwZCmpCaps & D3DPCMPCAPS_LESSEQUAL;

        //source blending
        d3d->canSrcBlendSrcAlpha = tri->dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA;
        d3d->canSrcBlendOne = tri->dwSrcBlendCaps & D3DPBLENDCAPS_ONE;
        d3d->canSrcBlendZero = tri->dwSrcBlendCaps & D3DPBLENDCAPS_ZERO;
        if (d3d->canSrcBlendSrcAlpha)
        {
            d3d->srcBlendFallback = D3DBLEND_SRCALPHA;
        }
        else if (d3d->canSrcBlendOne)
        {
            d3d->srcBlendFallback = D3DBLEND_ONE;
        }
        else
        {
            d3d->srcBlendFallback = D3DBLEND_ZERO;
        }

        //destination blending
        d3d->canDestBlendInvSrcAlpha = tri->dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA;
        d3d->canDestBlendOne = tri->dwDestBlendCaps & D3DPBLENDCAPS_ONE;
        d3d->canDestBlendZero = tri->dwDestBlendCaps & D3DPBLENDCAPS_ZERO;
        d3d->destBlendFallback = D3DBLEND_INVSRCALPHA;

        //alphatest
        d3d->canAlphaTestGreater = tri->dwAlphaCmpCaps & D3DPCMPCAPS_GREATER;
        d3d->canAlphaTestLess = tri->dwAlphaCmpCaps & D3DPCMPCAPS_LESS;
        if (tri->dwAlphaCmpCaps & D3DPCMPCAPS_NEVER)
        {
            d3d->alphaTestFallback = D3DCMP_NEVER;
        }
        if (tri->dwAlphaCmpCaps & D3DPCMPCAPS_ALWAYS)
        {
            d3d->alphaTestFallback = D3DCMP_ALWAYS;
        }

        //texture perspective correction
        d3d->canPerspectiveCorrect = tri->dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE;

        //square-only cap
        d3d->squareOnly = tri->dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY;

        //texture filtering
        d3d->canFilterLinear = tri->dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR;
        d3d->canFilterNearest = tri->dwTextureFilterCaps & D3DPTFILTERCAPS_NEAREST;

        //texture address
        d3d->canClamp = tri->dwTextureAddressCaps & D3DPTADDRESSCAPS_CLAMP;
        d3d->canWrap = tri->dwTextureAddressCaps & D3DPTADDRESSCAPS_WRAP;
    }
    if (caps->dwFlags & D3DDD_LINECAPS)
    {
        //linecaps are valid
        D3DPRIMCAPS* line = &caps->dpcLineCaps;

        //edge antialiasing
        d3d->canAntialiasLineEdges = line->dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES;
        d3d->canAntialiasLineDep = line->dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT;
        d3d->canAntialiasLineIndep = line->dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT;
    }

    d3d->canTexModulate = hal.dwTextureOpCaps & D3DTEXOPCAPS_MODULATE;
    d3d->canTexSelectArg1 = hal.dwTextureOpCaps & D3DTEXOPCAPS_SELECTARG1;
    d3d->canTexAdd = hal.dwTextureOpCaps & D3DTEXOPCAPS_ADD;

    //can we alphablend at all ?
    if (d3d->canSrcBlendSrcAlpha || d3d->canSrcBlendOne || d3d->canSrcBlendZero)
    {
        if (d3d->canDestBlendInvSrcAlpha || d3d->canDestBlendOne || d3d->canDestBlendZero)
        {
            d3d->canAlphaBlend = GL_TRUE;
        }
        else
        {
            d3d->canAlphaBlend = GL_FALSE;
        }
    }
    else
    {
        d3d->canAlphaBlend = GL_FALSE;
    }

    //can we alphatest at all ?
    if (d3d->canAlphaTestGreater || d3d->canAlphaTestLess)
    {
        d3d->canAlphaTest = GL_TRUE;
    }
    else
    {
        d3d->canAlphaTest = GL_FALSE;
    }

#if LOG_DEVICE_CAPS
    log_device_caps(d3d);
#endif

    return GL_TRUE;
}

GLboolean init_viewport(d3d_context* d3d)
{
    HRESULT hr;
    D3DVIEWPORT2 view;

    ZeroMemory(&view, sizeof(D3DVIEWPORT2));

    view.dwSize = sizeof(D3DVIEWPORT2);
    view.dwWidth = d3d->ScreenRect.right - d3d->ScreenRect.left;
    view.dwHeight = d3d->ScreenRect.bottom - d3d->ScreenRect.top;
    view.dvClipX = -1.0f;
    view.dvClipWidth = 2.0f;
    view.dvClipY = 1.0f;
    view.dvClipHeight = 2.0f;
    view.dvMaxZ = 1.0f;

    hr = d3d->d3dObject->CreateViewport(&d3d->d3dViewport, NULL);
    if (FAILED(hr))
    {
        errLog("init_viewport(CreateViewport)", hr);
        return GL_FALSE;
    }

    d3d->d3dDevice->AddViewport(d3d->d3dViewport);
    d3d->d3dViewport->SetViewport2(&view);

    d3d->d3dDevice->SetCurrentViewport(d3d->d3dViewport);

    return GL_TRUE;
}

static void LoadIdentity(D3DMATRIX* mat)
{
    mat->_11 = mat->_22 = mat->_33 = mat->_44 = 1.0f;
    mat->_12 = mat->_13 = mat->_14 = mat->_41 = 0.0f;
    mat->_21 = mat->_23 = mat->_24 = mat->_42 = 0.0f;
    mat->_31 = mat->_32 = mat->_34 = mat->_43 = 0.0f;
}

static void init_matrices(d3d_context* d3d)
{
    D3DMATRIX mat;
    D3DMATRIX matWorld;
    D3DMATRIX matView;
    D3DMATRIX matProj;

    LoadIdentity(&mat);

    matWorld = mat;
    d3d->d3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

    matView = mat;
    d3d->d3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matView);

    matProj = mat;
    d3d->d3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);
}

GLboolean initialize_directx(GLcontext* ctx)
{
    d3d_context* d3d = (d3d_context*)ctx->DriverCtx;

    d3d->Fullscreen = rglGetFullscreen();

    if (!init_direct_draw(d3d))
    {
        d3d_shutdown(ctx);
        return GL_FALSE;
    }

    if (d3d->Fullscreen)
    {
        if (!init_surfaces_fullscreen(d3d))
        {
            d3d_shutdown(ctx);
            return GL_FALSE;
        }
    }
    else
    {
        if (!init_surfaces_windowed(d3d))
        {
            d3d_shutdown(ctx);
            return GL_FALSE;
        }

        if (!init_clipper(d3d))
        {
            d3d_shutdown(ctx);
            return GL_FALSE;
        }
    }

    if (!init_d3d(d3d))
    {
        d3d_shutdown(ctx);
        return GL_FALSE;
    }

    enum_texture_formats(d3d);
    if (!d3d_match_texture_formats(d3d))
    {
        d3d_shutdown(ctx);
        return GL_FALSE;
    }

    if (!init_viewport(d3d))
    {
        d3d_shutdown(ctx);
        return GL_FALSE;
    }

    init_matrices(d3d);

    return GL_TRUE;
}

void d3d_shutdown(GLcontext* ctx)
{
    d3d_context* d3d = (d3d_context*)ctx->DriverCtx;

    d3d_end_scene(d3d);

    LOG(NULL);

    if (d3d->GammaControl != NULL)
    {
        LOG("resetting Gamma ...");
        d3d->GammaControl->SetGammaRamp(DDSGR_CALIBRATE, &d3d->awOldLUT);
        LOG(" reset.\n");
        LOG("releasing GammaControl ...");
        d3d->GammaControl->Release();
        LOG(" released.\n");
        d3d->GammaControl = NULL;
    }

    if (d3d->d3dViewport != NULL)
    {
        LOG("releasing d3dViewport ...");
        d3d->d3dViewport->Release();
        LOG(" released.\n");
        d3d->d3dViewport = NULL;
    }
    if (d3d->d3dDevice != NULL)
    {
        LOG("releasing d3dDevice ...");
        if (0 < d3d->d3dDevice->Release())
        {
            //FAILURE
            assert("d3dDevice->Release()" == "failure");
        }
        else
        {
            d3d->d3dDevice = NULL;
        }
        LOG(" released.\n");
    }
    if (d3d->d3dObject != NULL)
    {
        LOG("releasing d3dObject ...");
        d3d->d3dObject->Release();
        LOG(" released.\n");
        d3d->d3dObject = NULL;
    }
    if (d3d->DepthSurface != NULL)
    {
        LOG("releasing DepthSurface ...");
        d3d->DepthSurface->Release();
        LOG(" released.\n");
        d3d->DepthSurface = NULL;
    }
    if ((!d3d->Fullscreen) &&
        (d3d->BackSurface != NULL))
    {
        LOG("releasing BackSurface ...");
        d3d->BackSurface->Release();
        LOG(" released.\n");
        d3d->BackSurface = NULL;
    }
    if (d3d->PrimarySurface != NULL)
    {
        LOG("releasing PrimarySurface ...");
        d3d->PrimarySurface->Release();
        LOG(" released.\n");
        d3d->PrimarySurface = NULL;
        if (d3d->Fullscreen)
        {
            d3d->BackSurface = NULL;
        }
    }

    if (d3d->ddraw4 != NULL)
    {
        LOG("cooperating w/ ddraw4 ...");
        d3d->ddraw4->SetCooperativeLevel(d3d->hwnd, DDSCL_NORMAL);
        LOG(" cooperated.\n");
        LOG("releasing ddraw4 ...");
        d3d->ddraw4->Release();
        LOG(" released.\n");
        d3d->ddraw4 = NULL;
    }

    if (d3d->ddraw1 != NULL)
    {
        LOG("releasing ddraw1 ...");
        if (0 < d3d->ddraw1->Release())
        {
            //FAILURE
            assert("ddraw1->Release()" == "failure");
        }
        else
        {
            d3d->ddraw1 = NULL;
        }
        LOG(" released.\n");
    }
}
