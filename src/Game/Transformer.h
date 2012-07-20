/*=============================================================================
    Name    : transformer.h
    Purpose : provides vertex buffer transformation

    Created 10/8/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _TRANSFORMER_H
#define _TRANSFORMER_H

#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "Mesh.h"

extern hvector* eyeVertexList;
extern hvector* clipVertexList;

void transStartup(void);
void transShutdown(void);
void transGrowVertexLists(sdword nVertices);
void transTransformVertexList(sdword n, hvector* dest, vertexentry* source, hmatrix* m);
void transPerspectiveTransform(sdword n, hvector* dest, hvector* source, hmatrix* m);
void transGeneralPerspectiveTransform(sdword n, hvector* dest, hvector* source, hmatrix* m);
void transTransformCompletely(sdword n, hvector* dest, hvector* intermed, vertexentry* source, hmatrix* m0, hmatrix* m1);
bool transPerspectiveMatrix(hmatrix* m);

void transSinglePerspectiveTransform(hvector* screenSpace, hmatrix* projection, hvector* cameraSpace);
void transSingleTotalTransform(vector* screenSpace, hmatrix* modelview, hmatrix* projection, vector* worldSpace);

#endif
