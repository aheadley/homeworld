/*=============================================================================
    Name    : sstglide.c
    Purpose : 3Dfx-specific support

    Created 2/19/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "SDL_loadso.h"
#include <string.h>
#include <glide.h>
#include "sstglide.h"
#include "gldll.h"

#ifdef _WIN32
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif

#ifdef _WIN32
#define DLL_GETPROCADDRESS(LIB, SYM) GetProcAddress(LIB, SYM)
#elif !defined(_MACOSX)
#define DLL_GETPROCADDRESS(LIB, SYM) dlsym(LIB, SYM)
#endif

static bool sstHaveGlide = FALSE;

typedef FxBool (APIENTRY * GRLFBLOCKproc)(GrLock_t, GrBuffer_t, GrLfbWriteMode_t,
                                           GrOriginLocation_t, FxBool, GrLfbInfo_t*);
typedef FxBool (APIENTRY * GRLFBUNLOCKproc)(GrLock_t, GrBuffer_t);
typedef void (APIENTRY * GRBUFFERSWAPproc)(int);
typedef void (APIENTRY * GRSSTIDLEproc)(void);
typedef void (APIENTRY * GRSSTQUERYBOARDSproc)(GrHwConfiguration*);

GRLFBLOCKproc sstLfbLock = NULL;
GRLFBUNLOCKproc sstLfbUnlock = NULL;
GRBUFFERSWAPproc sstBufferSwap = NULL;
GRSSTIDLEproc sstSstIdle = NULL;
GRSSTQUERYBOARDSproc sstQueryBoards = NULL;

bool sstHardwareExists(sdword* type)
{
    void* lib;
    GrHwConfiguration hwconfig;
    bool rval;

#ifdef _WIN32
    lib = (void*)GetModuleHandle("glide2x.dll");
    if (!lib)
        lib = (void*)LoadLibrary("glide2x.dll");
#else
    lib = dlopen("libglide2x.so", RTLD_NOW);
#endif
    if (!lib)
        {
            return FALSE;
        }

#ifdef _MACOSX
    sstQueryBoards = (GRSSTQUERYBOARDSproc)SDL_LoadFunction(lib, "grSstQueryBoards");
#else
    sstQueryBoards = (GRSSTQUERYBOARDSproc)DLL_GETPROCADDRESS(lib, "grSstQueryBoards");
#endif

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

#ifdef _WIN32
    while (FreeLibrary((HINSTANCE)lib)) ;
#else
    while (!dlclose(lib)) { }
#endif

    return rval;
}

static void sstFreeHandle(void)
{
    void* lib;

#ifdef _WIN32
    lib = (void*)GetModuleHandle("glide2x.dll");
    if (lib != NULL)
    {
        while (FreeLibrary((HINSTANCE)lib)) ;
    }
#else
    lib = dlopen("libglide2x.so", RTLD_LAZY);
    if (lib)
    {
        while (!dlclose(lib)) { }
    }
#endif
}

bool sstLoaded(void)
{
    return sstHaveGlide;
}

void sstStartup(void)
{
    void* lib;

    //locate Glide .DLL and grab func pointers
#ifdef _WIN32
    lib = (void*)GetModuleHandle("glide2x.dll");
    if (!lib)
        lib = (void*)LoadLibrary("glide2x.dll");
#else
    lib = dlopen("libglide2x.so", RTLD_NOW);
#endif
    if (!lib)
        {
            sstHaveGlide = FALSE;
            return;
        }

#ifdef _MACOSX
    sstLfbLock     = (GRLFBLOCKproc)SDL_LoadFunction(lib, "grLfbLock");
    sstLfbUnlock   = (GRLFBUNLOCKproc)SDL_LoadFunction(lib, "grLfbUnlock");
    sstBufferSwap  = (GRBUFFERSWAPproc)SDL_LoadFunction(lib, "grBufferSwap");
    sstSstIdle     = (GRSSTIDLEproc)SDL_LoadFunction(lib, "grSstIdle");
    sstQueryBoards = (GRSSTQUERYBOARDSproc)SDL_LoadFunction(lib, "grSstQueryBoards");
#else
    sstLfbLock     = (GRLFBLOCKproc)DLL_GETPROCADDRESS(lib, "grLfbLock");
    sstLfbUnlock   = (GRLFBUNLOCKproc)DLL_GETPROCADDRESS(lib, "grLfbUnlock");
    sstBufferSwap  = (GRBUFFERSWAPproc)DLL_GETPROCADDRESS(lib, "grBufferSwap");
    sstSstIdle     = (GRSSTIDLEproc)DLL_GETPROCADDRESS(lib, "grSstIdle");
    sstQueryBoards = (GRSSTQUERYBOARDSproc)DLL_GETPROCADDRESS(lib, "grSstQueryBoards");
#endif

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

