/*=============================================================================
    Name    : sstglide.c
    Purpose : 3Dfx-specific support

    Created 2/19/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <windows.h>
#include <string.h>
#include <glide.h>
#include "sstglide.h"

static bool sstHaveGlide = FALSE;

typedef void (__stdcall * ALPHAFUNCproc)(GLenum, GLclampf);

typedef FxBool (__stdcall * GRLFBLOCKproc)(GrLock_t, GrBuffer_t, GrLfbWriteMode_t,
                                           GrOriginLocation_t, FxBool, GrLfbInfo_t*);
typedef FxBool (__stdcall * GRLFBUNLOCKproc)(GrLock_t, GrBuffer_t);
typedef void (__stdcall * GRBUFFERSWAPproc)(int);
typedef void (__stdcall * GRSSTIDLEproc)(void);
typedef void (__stdcall * GRSSTQUERYBOARDSproc)(GrHwConfiguration*);

GRLFBLOCKproc sstLfbLock = NULL;
GRLFBUNLOCKproc sstLfbUnlock = NULL;
GRBUFFERSWAPproc sstBufferSwap = NULL;
GRSSTIDLEproc sstSstIdle = NULL;
GRSSTQUERYBOARDSproc sstQueryBoards = NULL;

bool sstHardwareExists(sdword* type)
{
    HINSTANCE lib;
    GrHwConfiguration hwconfig;
    bool rval;

    lib = GetModuleHandle("glide2x.dll");
    if (lib == NULL)
    {
        lib = LoadLibrary("glide2x.dll");
        if (lib == NULL)
        {
            return FALSE;
        }
    }

    sstQueryBoards = (GRSSTQUERYBOARDSproc)GetProcAddress(lib, "_grSstQueryBoards@4");
    if (sstQueryBoards == NULL)
    {
        return FALSE;
    }

    sstQueryBoards(&hwconfig);

    if (hwconfig.num_sst > 0)
    {
        if (type != NULL)
        {
#if 1
            *type = SST_VOODOO2;
#else
            if (hwconfig.SSTs[0].type == GR_SSTTYPE_VOODOO ||
                hwconfig.SSTs[0].type == GR_SSTTYPE_Voodoo2)
            {
                if (hwconfig.SSTs[0].sstBoard.VoodooConfig.fbiRev & 0x100)
                {
                    *type = SST_VOODOO2;
                }
                else
                {
                    *type = SST_VOODOO;
                }
            }
            else
            {
                *type == SST_OTHER;
            }
#endif
        }
        rval = TRUE;
    }
    else
    {
        rval = FALSE;
    }

    while (FreeLibrary(lib)) ;

    return rval;
}

static void sstFreeHandle(void)
{
    HINSTANCE lib;

    lib = GetModuleHandle("glide2x.dll");
    if (lib != NULL)
    {
        while (FreeLibrary(lib)) ;
    }
}

bool sstLoaded(void)
{
    return sstHaveGlide;
}

void sstStartup(void)
{
    HINSTANCE lib;

    //locate Glide .DLL and grab func pointers
    lib = GetModuleHandle("glide2x.dll");
    if (lib == NULL)
    {
        lib = LoadLibrary("glide2x.dll");
        if (lib == NULL)
        {
            sstHaveGlide = FALSE;
            return;
        }
    }

    sstLfbLock = (GRLFBLOCKproc)GetProcAddress(lib, "_grLfbLock@24");
    sstLfbUnlock = (GRLFBUNLOCKproc)GetProcAddress(lib, "_grLfbUnlock@8");
    sstBufferSwap = (GRBUFFERSWAPproc)GetProcAddress(lib, "_grBufferSwap@4");
    sstSstIdle = (GRSSTIDLEproc)GetProcAddress(lib, "_grSstIdle@0");
    sstQueryBoards = (GRSSTQUERYBOARDSproc)GetProcAddress(lib, "_grSstQueryBoards@4");

    if (sstLfbLock == NULL ||
        sstLfbUnlock == NULL ||
        sstBufferSwap == NULL ||
        sstQueryBoards == NULL)
    {
        sstFreeHandle();
        sstHaveGlide = FALSE;
    }
    else
    {
        sstHaveGlide = TRUE;
    }
}

void sstShutdown(void)
{
    //release Glide .DLL reference
    sstFreeHandle();
    sstHaveGlide = FALSE;
}

uword* sstGetFramebuffer(sdword* pitch)
{
    GrLfbInfo_t info;

    if (!sstHaveGlide)
    {
        if (pitch != NULL)
        {
            *pitch = 0;
        }
        return NULL;
    }

    if (pitch == NULL)
    {
        sstLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
        return NULL;
    }

    memset(&info, 0, sizeof(info));
    info.size = sizeof(info);
    if (!sstLfbLock(GR_LFB_WRITE_ONLY,
                    GR_BUFFER_BACKBUFFER,
                    GR_LFBWRITEMODE_565,
                    GR_ORIGIN_UPPER_LEFT,
                    FXFALSE, &info))
    {
        if (pitch != NULL)
        {
            *pitch = 0;
        }
        return NULL;
    }

    *pitch = info.strideInBytes;
    return (uword*)info.lfbPtr;
}

void sstFlush(void)
{
    if (sstHaveGlide)
    {
        sstBufferSwap(1);
    }
}

void sstFinish(void)
{
    if (sstHaveGlide)
    {
        sstSstIdle();
    }
}

