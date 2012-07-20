/*=============================================================================
    Name    : glcompat.h
    Purpose : GL compatibility layer - for fast GL frontends, DrawPixels -> texture, &c

    Created 7/14/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _GLCOMPAT_H
#define _GLCOMPAT_H

#include "prim2d.h"
#include "color.h"

extern bool glcLinear;

bool glcStartup(void);
bool glcShutdown(void);
bool glcAllocateScratch(bool allocate);
ubyte* glcGetScratch(sdword* pitch);
ubyte* glcGetScratchMaybeAllocate(sdword* pitch);
void glcDisplayRGBABackground(ubyte* surface);
void glcDisplayRGBABackgroundScaled(ubyte* surface);
void glcDisplayRGBABackgroundLinearScaled(ubyte* surface);
void glcDisplayRGBABackgroundWithoutScaling(ubyte* surface);
void glcMatrixSetup(bool on);
void glcPageFlip(bool scaled);
bool glcActive(void);
bool glcActivate(bool active);
bool glcFullscreen(bool full);
bool glcIsFullscreen(void);

void glcRectSolid2(rectangle* rect, color c);
void glcBeveledRectSolid2(rectangle* rect, color c, sdword xb, sdword yb);
void glcRectTranslucent2(rectangle* rect, color c);
void glcRectShaded2(rectangle* rect, color* c);
void glcRectOutline2(rectangle* rect, sdword thickness, color c);
void glcRectSolidTextured2(rectangle* rect, sdword width, sdword height, ubyte* data, color* palette, bool reverse);
void glcRectSolidTexturedScaled2(rectangle* rect, sdword width, sdword height,
                                 ubyte* data, color* palette, bool reverse);
void glcPoint2(sdword x, sdword y, sdword thickness, color c);
void glcLine2(sdword x0, sdword y0, sdword x1, sdword y1, sdword thickness, color c);
void glcTriSolid2(triangle* tri, color c);
void glcTriOutline2(triangle* tri, sdword thickness, color c);

void glcMouseDraw(void);
void glcCursorUnder(ubyte* data, sdword width, sdword height, sdword x0, sdword y0, bool store);

void glcRenderEverything(void);
void glcRenderEverythingALot(void);

void glcLoadTextures(void);
void glcFreeTextures(void);

#endif
