/*=============================================================================
    Name    : sdldraw.c
    Purpose : Software renderer drawing routines using SDL (ported from
              draw.cpp).

    Created 9/29/2003 by Ted Cipicchio
=============================================================================*/
#define NAME  "SDLDraw"

#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "kgl.h"

Uint8 WINDOWED = 1;

void rglswSetWindowed(int windowed)
{
    WINDOWED = (windowed ? 1 : 0);
}

/* Ignored... */
unsigned int useDirectDraw = 1;

static int SCRWIDTH;
static int SCRHEIGHT;

Uint8 bSaveFramebuffer = 0;
Uint8 bActive = 0;
Uint32 depth = 0;
Uint32 scrpitch = 0;
Uint8* scrbuf = 0;

Uint32 scratch_pitch = 0;
Uint8* scratch_buffer;
//Uint8* scratch_buffer_2;

unsigned char restoreAll()
{
    return 1;
}

static void finiObjects(unsigned char freeDD)
{
    Uint32 flags;
    if (!bActive)
    {
        return;
    }
    flags = SDL_WasInit(SDL_INIT_EVERYTHING);
    if (!(flags & SDL_INIT_VIDEO))
        return;
    if (flags ^ SDL_INIT_VIDEO)
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    else
        SDL_Quit();
}

unsigned char DrawBegin(void);
unsigned char DrawEnd(void);

void LockBuffers()
{
    DrawBegin();
}

void UnlockBuffers()
{
    DrawEnd();
}

void rglswDrawAnimatic(int px, int py, unsigned char* pb)
{
    int y;
    Uint8* pSource;
    Uint8* pDest;

    LockBuffers();

    for (y = 0; y < 480; y++)
    {
        pSource = pb + 2*640*y;
        pDest = scrbuf + scrpitch*(y+py) + 2*px;
        MEMCPY(pDest, pSource, 2*640);
    }

    UnlockBuffers();
}

unsigned char* rglswGetScratch()
{
    return(bSaveFramebuffer ? scratch_buffer : scrbuf);
}

void rglswClearScratch()
{
    int y;

    for (y = 0; y < SCRHEIGHT; y++)
    {
        MEMSET(scratch_buffer + scratch_pitch*y, 0, 2*SCRWIDTH);
    }
}

static void clear_buffers(void)
{
    /* SDL hides access to the display surface, so we'll only clear the back
       buffer. */
    SDL_Surface* pSurface = SDL_GetVideoSurface();
    if (!pSurface)
        return;
    SDL_FillRect(pSurface, 0, 0);
}

unsigned int GetPitch()
{
    SDL_Surface* pSurface = SDL_GetVideoSurface();
    return (pSurface ? pSurface->pitch : 0);
}

unsigned char DrawBegin()
{
    SDL_Surface* pSurface;

    //if (bSaveFramebuffer || !useDirectDraw)
    if (bSaveFramebuffer)
    {
        scrpitch = scratch_pitch;
        scrbuf = scratch_buffer;
        return 1;
    }

    pSurface = SDL_GetVideoSurface();
    if (!pSurface)
    {
        return 0;
    }

    if (SDL_LockSurface(pSurface) == -1)
    {
        scrpitch = scratch_pitch;
        scrbuf = scratch_buffer;
        return 1;
    }

    scrpitch = pSurface->pitch;
    scratch_pitch = scrpitch;
    scrbuf = (Uint8*)pSurface->pixels;

    return 1;
}

unsigned char DrawEnd()
{
    SDL_Surface* pSurface;

    //if (bSaveFramebuffer || !useDirectDraw)
    if (bSaveFramebuffer)
    {
        scrbuf = scratch_buffer;
        scrpitch = 0;
        return 1;
    }

    pSurface = SDL_GetVideoSurface();
    if (!pSurface)
    {
        return 0;
    }

    if (scrbuf == scratch_buffer)
    {
        scrpitch = 0;
        return 1;
    }

    if (bActive)
    {
        SDL_UnlockSurface(pSurface);
    }

    scrpitch = 0;
    scrbuf = 0;

    return 1;
}

int rglGetScreenPitch()
{
    int pitch;
    DrawBegin();
    pitch = scrpitch;
    DrawEnd();
    return pitch;
}

int rglGetScreenDepth()
{
    return depth;
}

static void rglswPitchedCopy(
    Uint8* dest, int destPitch,
    Uint8* source, int sourcePitch,
    int width, int height)
{
    if (destPitch == sourcePitch)
    {
        memcpy(dest, source, sourcePitch * height);
    }
    else
    {
        Uint8* dp;
        Uint8* sp;
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
    Uint8 bSave = bSaveFramebuffer;

    //if (!useDirectDraw) return;

    bSaveFramebuffer = 0;

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
    Uint8 bSave = bSaveFramebuffer;

    //if (!useDirectDraw) return;

    bSaveFramebuffer = 0;

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
    Uint8 bSave = bSaveFramebuffer;

    bSaveFramebuffer = 0;

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

void rglswSetSaveState(int on)
{
    if (on && !bSaveFramebuffer)
    {
        rglCopyToScratchBuffer();
    }

    bSaveFramebuffer = on ? 1 : 0;
}

void updateFrame(void)
{
    SDL_Surface* pSurface;

    if (bSaveFramebuffer)
    {
        rglCopyScratchBuffer();
    }

    pSurface = SDL_GetVideoSurface();
    if (!pSurface)
        return;
    SDL_Flip(pSurface);
}

unsigned char rglswCreateWindow(GLint ihwnd, GLint width)
{
    Uint32 flags;
    SDL_Surface* pSurface;

    /* Set up screen dimensions we'll use. */
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

    /* Initialize SDL video if it has not already been done. */
    flags = SDL_WasInit(SDL_INIT_EVERYTHING);
    if (!flags)
    {
        if (SDL_Init(SDL_INIT_VIDEO) == -1)
            return 0;
    }
    else if (!(flags & SDL_INIT_VIDEO))
    {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
            return 0;
    }

    /* Create a double-buffered display. */
    flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
    if (!WINDOWED)
        flags |= SDL_FULLSCREEN;
    if (!(pSurface = SDL_SetVideoMode(SCRWIDTH, SCRHEIGHT, 16, flags)))
        return 0;

    /* Verify the render surface bit depth. */
    depth = pSurface->format->BitsPerPixel;
    if (depth < 15 || depth > 16)
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return 0;
    }

    bActive = 1;

    scratch_pitch = GetPitch();
    scratch_buffer = (Uint8*)malloc(scratch_pitch*SCRHEIGHT);
    memset(scratch_buffer, 0, scratch_pitch*SCRHEIGHT);

    clear_buffers();

    return 1;
}

void rglswDeleteWindow(GLint ihwnd)
{
    DrawEnd();
    finiObjects(1);

    if (scratch_buffer)
    {
        free(scratch_buffer);
        scratch_buffer = 0;
    }
}

gl_pixel_type rglGetPixelType()
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

DLL void rglDDrawActivate(unsigned char active)
{
    // Okay...
}
