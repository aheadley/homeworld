/*=============================================================================
    Name    : btg.h
    Purpose : data structures &c for the BTG

    Created 4/27/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _BTG_H
#define _BTG_H

#include "Types.h"


#define BTG_FILE_VERSION 0x600


// -----
// structures
// -----

typedef struct btgHeader
{
    udword  btgFileVersion;
    udword  numVerts;
    udword  numStars;
    udword  numPolys;
    sdword  xScroll, yScroll;
    udword  zoomVal;
    sdword  pageWidth, pageHeight;
    sdword  mRed, mGreen, mBlue;
    sdword  mBGRed, mBGGreen, mBGBlue;
    bool    bVerts, bPolys, bStars, bOutlines, bBlends;
    sdword  renderMode;
} btgHeader;

typedef struct btgVertex
{
    udword  flags;
    real64  x, y;
    sdword  red, green, blue, alpha, brightness;
} btgVertex;

typedef struct btgStar
{
    udword  flags;
    real64  x, y;
    sdword  red, green, blue, alpha;
    char    filename[48];
    udword  glhandle;
    sdword  width, height;
} btgStar;

typedef struct btgPolygon
{
    udword  flags;
    udword  v0, v1, v2;
} btgPolygon;


/*=============================================================================
    Data:
=============================================================================*/
extern char btgLastBackground[128];
extern real32 btgFieldOfView;

// -----
// prototypes
// -----

void btgStartup(void);
void btgReset(void);
void btgShutdown(void);
void btgSetTheta(real32 theta);
void btgSetPhi(real32 theta);
real32 btgGetTheta(void);
real32 btgGetPhi(void);
void btgCloseTextures(void);
void btgLoadTextures(void);
void btgLoad(char* filename);
void btgConvertVerts(void);
void btgRender(void);
void btgSetColourMultiplier(real32 t);

#endif
