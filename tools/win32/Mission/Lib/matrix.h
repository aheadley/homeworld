// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef MATRIX_H
#define MATRIX_H

// General constants
#ifndef PI
#define PI 3.14159265358979323846F
#endif

#ifdef __cplusplus
extern "C" {
#endif

void matInvert(float *c,float* m);
void matInvertEx(float* c,float * m);
void matProduct(float *c,float *a,float *b);
void matVectProduct(float *c,float *a,float *b);
void matCrossProduct(float *c,float *a,float *b);
void matIdentity(float *a);
void matTranslate(float *a,float x, float y, float z);
void matScale(float *a,float x,float y,float z);
void matRotate(float* a,float* axis,float angle);
void matNormalize(float *a);
void matLookAt(float *a,float *aEye,float *aFocus,float *aUp);
void matPerspect(float *a,float fov,float aspect,float cnear,float cfar);
void matFrustum(float *a,float left,float right,float bottom,float top,float cnear,float cfar);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif