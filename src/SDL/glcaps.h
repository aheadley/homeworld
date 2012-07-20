/*=============================================================================
        Name    : glcaps.h
        Purpose : determine the capabilities of the GL currently in use

Created 19/06/1998 by khent
Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _GLCAPS_H
#define _GLCAPS_H

#ifndef SW_Render
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#endif
#include "Types.h"
#include "glinc.h"

#define GL_SWAPFRIENDLY 0xffe01
#define GL_COMPILED_ARRAYS_EXT 0xffe02

extern char GENERIC_OPENGL_RENDERER[];

extern GLenum glCapDepthFunc;
extern bool gl3Dfx;
extern bool glNT;
extern bool gl95;

extern char const* GLC_VENDOR;
extern char const* GLC_RENDERER;
extern char const* GLC_EXTENSIONS;

bool glCapFastFeature(GLenum feature);
bool glCapFeatureExists(GLenum feature);
bool glCapTexSupport(GLenum format);
void glCapStartup(void);
bool glCapLoadOpenGL(char* dllName);

sdword glCapNumBuffers(void);
bool glCapNT(void);
bool glCap95(void);

bool glCapValidGL(void);

#endif
