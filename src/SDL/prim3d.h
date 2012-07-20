/*=============================================================================
    Name    : prim3d.h
    Purpose : Definitions for drawing 3D primitives

    Created 7/1/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PRIM3D_H
#define ___PRIM3D_H

#include "Types.h"
#include "Vector.h"
#include "color.h"
#include "texreg.h"

#define X_AXIS  0
#define Y_AXIS  1
#define Z_AXIS  2

/*=============================================================================
    Functions:
=============================================================================*/

//draw 3D points with size
void primPoint3(vector *p1, color c);
void primPointSize3(vector *p1, real32 size, color c);
void primPointSize3Fade(vector *p1, real32 size, color c, real32 fade);

//efficient multiple 3D points with size & fade
void primBeginPointSize3Fade(real32 size);
void primNextPointSize3Fade(vector* p1, color c, real32 fade);
void primEndPointSize3Fade(void);

//draw 3D lines
void primLine3(vector *p1, vector *p2, color c);
void primLineThick3(vector *p1, vector *p2, real32 thickness, color c);

//draw 3D 'pizza dish' circle using poly fans
void primCircleSolid3(vector *centre, real32 radius, sdword nSpokes, color c);
void primCircleSolid3Fade(vector *centre, real32 radius, sdword nSpokes, color c, real32 fade);
void primCircleOutline3(vector *centre, real32 radius, sdword nSlices, sdword nSpokes, color c, uword axis);
void primCircleOutlineZ(vector *centre, real32 radius, sdword nSegments, color c);
void primEllipseOutlineZ(vector *centre, real32 rx, real32 ry, sdword nSegments, color c);

void primSolidTexture3(vector *p1, real32 size, color c, trhandle tex);
void primSolidTexture3Fade(vector *p1, real32 size, color c, trhandle tex, real32 fade);

void prim3dStartup(void);
void prim3dShutdown(void);
#endif //___PRIM3D_H

