/*=============================================================================
        Name    : shader.h
        Purpose : shader provides specialized shading models for non-rGL renderers

Created 22/06/1998 by khent
Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _SHADER_H
#define _SHADER_H

#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "color.h"

typedef struct
{
    ubyte c[4];
} shColor;

extern shColor* colorList;

#define shColorSetIndexed(index) \
    { \
        shColor* c; \
        \
        c = colorList + index; \
        glColor4ub(c->c[0], c->c[1], c->c[2], c->c[3]); \
    }

void shStartup(void);
void shShutdown(void);
void shTransformNormal(vector* out, vector* in, real32* m);
void shTransformVertex(vector* out, vector* in, real32* m);
real32 shPow(real32 a, real32 b);
void shSpecularColour(sdword specInd, sdword side, vector* vobj, vector* norm,
                      ubyte* color, real32* m, real32* minv);
void shColour(sdword side, vector* norm, ubyte* color, real32* minv);
void shColourSet(sdword side, vector* norm, real32* minv);
void shColourSet0(vector* norm);
void shInvertMatrix(real32* out, real32 const* m);
void shSetExponent(sdword index, real32 exponent);
void shSetLightPosition(sdword index, real32* position, real32* m);
void shSetGlobalAmbient(real32* ambient);
void shSetMaterial(real32* ambient, real32* diffuse);
void shSetLighting(sdword index, real32* diffuse, real32* ambient);
void shUpdateLighting(void);
void shGammaUp(void);
void shGammaDown(void);
void shGammaReset(void);
void shGammaSet(sdword gamma);
sdword shGammaGet(void);

void shGrowBuffers(sdword nVertices);
void shShadeBuffer(sdword nVertices, hvector* source);

void shDockLight(real32 t);
void shDockLightColor(color c);

void shPushLightMatrix(hmatrix *pMat);
void shPopLightMatrix(void);

#endif
