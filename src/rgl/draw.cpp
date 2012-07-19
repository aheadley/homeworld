#define NAME  "DDraw"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

extern "C" {
#include "kgl.h"
}

extern "C" BOOL WINDOWED = TRUE;

extern "C" void rglswSetWindowed(int windowed)
{
    WINDOWED = windowed ? TRUE : FALSE;
}

extern "C" unsigned int useDirectDraw = 1;

static int SCRWIDTH;
static int SCRHEIGHT;

LPDIRECTDRAW        lpDD;
LPDIRECTDRAWSURFACE PrimarySurface;
LPDIRECTDRAWSURFACE BackSurface;

HWND hwnd;

BOOL bSaveFramebuffer = FALSE;
BOOL bActive = FALSE;
DWORD depth = 0;
DWORD scrpitch = 0;
extern "C" BYTE* scrbuf = NULL;

DWORD scratch_pitch = 0;
BYTE* scratch_buffer;
BYTE* scratch_buffer_2;

BOOL restoreAll()
{
    return PrimarySurface->Restore() == DD_OK &&
           BackSurface->Restore()    == DD_OK;
}

static void finiObjects(BOOL freeDD)
{
    if (!bActive)
    {
        return;
    }
    if (lpDD != NULL)
    {
        if (PrimarySurface != NULL)
        {
            PrimarySurface->Release();
            PrimarySurface = NULL;
        }
        if (BackSurface != NULL)
        {
            BackSurface->Release();
            BackSurface = NULL;
        }
        if (freeDD)
        {
            lpDD->Release();
            lpDD = NULL;
        }
    }
}

BOOL DrawBegin(void);
BOOL DrawEnd(void);

extern "C" void LockBuffers()
{
    DrawBegin();
}

extern "C" void UnlockBuffers()
{
    DrawEnd();
}

extern "C" void rglswDrawAnimatic(int px, int py, unsigned char* pb)
{
    int y;
    BYTE* pSource;
    BYTE* pDest;

    LockBuffers();

    for (y = 0; y < 480; y++)
    {
        pSource = pb + 2*640*y;
        pDest = scrbuf + scrpitch*(y+py) + 2*px;
        MEMCPY(pDest, pSource, 2*640);
    }

    UnlockBuffers();
}

extern "C" unsigned char* rglswGetScratch()
{
    return(bSaveFramebuffer ? scratch_buffer : scrbuf);
}

extern "C" void rglswClearScratch()
{
    int y;

    for (y = 0; y < SCRHEIGHT; y++)
    {
        MEMSET(scratch_buffer + scratch_pitch*y, 0, 2*SCRWIDTH);
    }
}

static void clear_buffers(void)
{
    LPDIRECTDRAWSURFACE surfs[] = {BackSurface, PrimarySurface};
    LPDIRECTDRAWSURFACE surf;
    DDSURFACEDESC ddsd;
    BYTE* psurf;
    int i, y;

    for (i = 0; i < 2; i++)
    {
        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);

        surf = surfs[i];
        if (surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL) != DD_OK)
        {
            continue;
        }

        for (y = 0; y < SCRHEIGHT; y++)
        {
            psurf = (BYTE*)ddsd.lpSurface + y*ddsd.lPitch;
            ZeroMemory(psurf, 2*SCRWIDTH);
        }

        surf->Unlock(NULL);
    }
}

DWORD GetPitch()
{
    DDSURFACEDESC ddsd;
    HRESULT       result;

    if (BackSurface == NULL)
    {
        return FALSE;
    }

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    result = BackSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (result != DD_OK)
    {
        return 0;
    }

    DWORD pitch = ddsd.lPitch;

    BackSurface->Unlock(NULL);

    return pitch;
}

BOOL DrawBegin()
{
    DDSURFACEDESC ddsd;
    HRESULT       result;

    if (bSaveFramebuffer || !useDirectDraw)
    {
        scrpitch = scratch_pitch;
        scrbuf = scratch_buffer;
        return TRUE;
    }

    if (BackSurface == NULL)
    {
        return FALSE;
    }

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    result = BackSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (result != DD_OK)
    {
        scrpitch = scratch_pitch;
        scrbuf = scratch_buffer;
        return TRUE;
    }

    scrpitch = ddsd.lPitch;
    scratch_pitch = scrpitch;
    scrbuf = (BYTE*)ddsd.lpSurface;

    return TRUE;
}

BOOL DrawEnd()
{
    if (bSaveFramebuffer || !useDirectDraw)
    {
        scrbuf = scratch_buffer;
        scrpitch = 0;
        return TRUE;
    }

    if (BackSurface == NULL)
    {
        return FALSE;
    }

    if (scrbuf == scratch_buffer)
    {
        scrpitch = 0;
        return TRUE;
    }

    if (bActive)
    {
        BackSurface->Unlock(NULL);
    }

    scrpitch = 0;
    scrbuf = NULL;

    return TRUE;
}

extern "C" int rglGetScreenPitch()
{
    int pitch;
    DrawBegin();
    pitch = scrpitch;
    DrawEnd();
    return pitch;
}

extern "C" int rglGetScreenDepth()
{
    return depth;
}

static void rglswPitchedCopy(
    BYTE* dest, int destPitch,
    BYTE* source, int sourcePitch,
    int width, int height)
{
    if (destPitch == sourcePitch)
    {
        memcpy(dest, source, sourcePitch * height);
    }
    else
    {
        BYTE* dp;
        BYTE* sp;
        int y;

        dp = dest;
        sp = source;
        for (y = 0; y < height; y++, dp += destPitch, sp += sourcePitch)
        {
            memcpy(dp, sp, 2*width);
        }
    }
}

static void rglCopyScratchBuffer()
{
    BOOL bSave = bSaveFramebuffer;

    if (!useDirectDraw) return;

    bSaveFramebuffer = FALSE;

    if (!DrawBegin())
    {
        bSaveFramebuffer = bSave;
        return;
    }
    rglswPitchedCopy(scrbuf, scrpitch,
                     scratch_buffer, scratch_pitch,
                     SCRWIDTH, SCRHEIGHT);
    DrawEnd();

    bSaveFramebuffer = bSave;
}

static void rglCopyToScratchBuffer()
{
    BOOL bSave = bSaveFramebuffer;

    if (!useDirectDraw) return;

    bSaveFramebuffer = FALSE;

    if (!DrawBegin())
    {
        bSaveFramebuffer = bSave;
        return;
    }
    rglswPitchedCopy(scratch_buffer, scratch_pitch,
                     scrbuf, scrpitch,
                     SCRWIDTH, SCRHEIGHT);
    DrawEnd();

    bSaveFramebuffer = bSave;
}

static void rglCopyToUnpitchedScratchBuffer()
{
    BOOL bSave = bSaveFramebuffer;

    bSaveFramebuffer = FALSE;

    if (!DrawBegin())
    {
        bSaveFramebuffer = bSave;
        return;
    }
    rglswPitchedCopy(scratch_buffer, 2*SCRWIDTH,
                     scrbuf, scrpitch,
                     SCRWIDTH, SCRHEIGHT);

    bSaveFramebuffer = bSave;
}

extern "C" void rglswSetSaveState(int on)
{
    if (on && !bSaveFramebuffer)
    {
        rglCopyToScratchBuffer();
    }

    bSaveFramebuffer = on ? TRUE : FALSE;
}

static void flipImage(void)
{
    int y, top, bot;
    int pitch;

    pitch = 2*SCRWIDTH;

    for (y = 0; y < (SCRHEIGHT/2); y++)
    {
        top = y;
        bot = (SCRHEIGHT-1) - y;

        memcpy(scratch_buffer_2 + pitch*top, scratch_buffer + pitch*bot, pitch);
        memcpy(scratch_buffer_2 + pitch*bot, scratch_buffer + pitch*top, pitch);
    }
}

static void noddrawUpdateFrame(void)
{
    BITMAPINFOHEADER* pbi;
    BITMAPINFO bmi;
    BITMAPINFO* pbmi;
    HBITMAP hbitmap;
    BYTE* pbits;
    WORD* pbitmap;
    HDC hdc, hdcTemp;
    int bitsize;

    flipImage();

    pbmi = &bmi;

    bitsize = 2 * SCRWIDTH * SCRHEIGHT;
    pbits = (BYTE*)scratch_buffer_2;

    pbi = (BITMAPINFOHEADER*)pbmi;
    pbi->biSize = sizeof(BITMAPINFOHEADER);
    pbi->biWidth = SCRWIDTH;
    pbi->biHeight = SCRHEIGHT;
    pbi->biPlanes = 1;
    pbi->biBitCount = 16;
    pbi->biCompression = BI_RGB;
    pbi->biSizeImage = 0;
    pbi->biXPelsPerMeter = 0;
    pbi->biYPelsPerMeter = 0;
    pbi->biClrUsed = 0;
    pbi->biClrImportant = 0;

    hdc = GetDC(hwnd);
    hbitmap = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS, (VOID**)&pbitmap, 0, 0);
    SetDIBits(hdc, hbitmap, 0, SCRHEIGHT, pbits, pbmi, DIB_RGB_COLORS);

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, hbitmap);

    BitBlt(hdc, 0, 0, SCRWIDTH, SCRHEIGHT, hdcTemp, 0, 0, SRCCOPY);

    DeleteDC(hdcTemp);
    ReleaseDC(hwnd, hdc);

    DeleteObject(hbitmap);
}

extern "C" void updateFrame(void)
{
    HRESULT ddrval;
    RECT    destRect, rcRect;
    POINT   pt;

    if (!useDirectDraw)
    {
        noddrawUpdateFrame();
        return;
    }

    if (WINDOWED)
    {
        GetClientRect(hwnd, &destRect);
        pt.x = pt.y = 0;
        ClientToScreen(hwnd, &pt);
        OffsetRect(&destRect, pt.x, pt.y);
        rcRect.left = 0;
        rcRect.top = 0;
        rcRect.right = SCRWIDTH;
        rcRect.bottom = SCRHEIGHT;
    }
    else
    {
        rcRect.left = rcRect.top = 0;
        rcRect.right = SCRWIDTH;
        rcRect.bottom = SCRHEIGHT;
        destRect = rcRect;
    }

    if (bSaveFramebuffer)
    {
        rglCopyScratchBuffer();
    }

    for (;;)
    {
        ddrval = PrimarySurface->Blt(&destRect, BackSurface, &rcRect, 0, NULL);
//        ddrval = PrimarySurface->BltFast(0, 0, BackSurface, &rcRect, 0);
        if (ddrval == DD_OK)
        {
            break;
        }
        if (ddrval == DDERR_SURFACELOST)
        {
            if (!restoreAll())
            {
                return;
            }
        }
        if (ddrval != DDERR_WASSTILLDRAWING)
        {
            return;
        }
    }
}

extern "C" unsigned char rglswCreateWindow(GLint ihwnd, GLint width)
{
    DDSURFACEDESC ddsd;
    HRESULT       ddrval;
    DDPIXELFORMAT pix;
    DDSCAPS       ddscaps;
    DDCAPS        ddcaps;
    HWND          hWnd = (HWND)ihwnd;

    SCRWIDTH = width;
    switch (width)
    {
    case 1600:
        SCRHEIGHT = 1200;
        break;
    case 1280:
        SCRHEIGHT = 1024;
        break;
    case 1024:
        SCRHEIGHT = 768;
        break;
    case 800:
        SCRHEIGHT = 600;
        break;
    default:
        SCRWIDTH  = 640;
        SCRHEIGHT = 480;
    }

    hwnd = hWnd;

    ddrval = DirectDrawCreate(NULL, &lpDD, NULL);
    if (ddrval != DD_OK)
    {
        return FALSE;
    }

    ZeroMemory(&ddcaps, sizeof(ddcaps));
    ddcaps.dwSize = sizeof(ddcaps);
    lpDD->GetCaps(&ddcaps, NULL);

    if (WINDOWED)
    {
        ddrval = lpDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
        if (ddrval != DD_OK)
        {
            return FALSE;
        }
    }
    else
    {
        ddrval = lpDD->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
        if (ddrval != DD_OK)
        {
            return FALSE;
        }

        ddrval = lpDD->SetDisplayMode(SCRWIDTH, SCRHEIGHT, 16);
        if (ddrval != DD_OK)
        {
            return FALSE;
        }
    }

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    if (WINDOWED)
    {
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        ddrval = lpDD->CreateSurface(&ddsd, &PrimarySurface, NULL);
    }
    else
    {
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        ddrval = lpDD->CreateSurface(&ddsd, &PrimarySurface, NULL);
    }
    if (ddrval != DD_OK)
    {
        return FALSE;
    }

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
    ddsd.dwWidth = SCRWIDTH;
    ddsd.dwHeight = SCRHEIGHT;
    ddrval = lpDD->CreateSurface(&ddsd, &BackSurface, NULL);
    if (ddrval != DD_OK)
    {
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        ddrval = lpDD->CreateSurface(&ddsd, &BackSurface, NULL);
    }
    if (ddrval != DD_OK)
    {
        return FALSE;
    }

    pix.dwSize = sizeof(pix);
    pix.dwFlags = DDPF_RGB;
    BackSurface->GetPixelFormat(&pix);

    if (pix.dwRBitMask == 0x7C00)
    {
        depth = 15;
    }
    else
    {
        depth = 16;
    }

    bActive = TRUE;

    if (useDirectDraw)
    {
        scratch_pitch = GetPitch();
        scratch_buffer_2 = NULL;
    }
    else
    {
        depth = 15;
        scratch_pitch = 2*SCRWIDTH;
        scratch_buffer_2 = (BYTE*)malloc(scratch_pitch*SCRHEIGHT);
    }
    scratch_buffer = (BYTE*)malloc(scratch_pitch*SCRHEIGHT);
    memset(scratch_buffer, 0, scratch_pitch*SCRHEIGHT);

    clear_buffers();

    return TRUE;
}

extern "C" void rglswDeleteWindow(GLint ihwnd)
{
    HWND hwnd = (HWND)ihwnd;

    DrawEnd();
    finiObjects(TRUE);

    if (scratch_buffer != NULL)
    {
        free(scratch_buffer);
        scratch_buffer = NULL;
    }
    if (scratch_buffer_2 != NULL)
    {
        free(scratch_buffer_2);
        scratch_buffer_2 = NULL;
    }
}

extern "C" gl_pixel_type rglGetPixelType()
{
    switch (depth)
    {
    case 15:
        return GL_RGB555;
    case 16:
        return GL_RGB565;
    default:
        return GL_RGBUNKNOWN;
    }
}

extern "C" DLL void rglDDrawActivate(unsigned char active)
{
    HRESULT rval;

    if (WINDOWED || lpDD == NULL)
    {
        return;
    }
    if (active)
    {
        if (!bActive)
        {
            for (;;)
            {
                rval = lpDD->SetDisplayMode(SCRWIDTH, SCRHEIGHT, 16);
                if (rval == DD_OK)
                {
                    break;
                }
                if (rval == DDERR_SURFACELOST)
                {
                    if (!restoreAll())
                    {
                        break;
                    }
                }
            }

            bActive = TRUE;
        }
    }
    else
    {
        if (bActive)
        {
            for (;;)
            {
                rval = lpDD->RestoreDisplayMode();
                if (rval == DD_OK)
                {
                    break;
                }
                if (rval == DDERR_SURFACELOST)
                {
                    if (!restoreAll())
                    {
                        break;
                    }
                }
            }

            bActive = FALSE;
        }
    }
}
