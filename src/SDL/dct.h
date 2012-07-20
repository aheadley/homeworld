// Copyright (c) 1997-99 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $
//
// Version 1.6a

#ifndef DCT_H
#define DCT_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

#ifndef PI
#define PI		3.14159265358979323846F
#endif

#define DOUBSIZE 512
#define FULLSIZE 256
#define HALFSIZE 128

#ifdef __cplusplus
extern "C" {
#endif

int dct(float *f,float *g,float *c,unsigned long n);
int idct(float *f,float *g,float *c,unsigned long n);
int Initdct(float *c,unsigned long n);

void fft(long n,float *xRe,float *xIm,float *yRe,float *yIm);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif  // DCT_H
