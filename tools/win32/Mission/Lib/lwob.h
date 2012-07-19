// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef LWOB_H
#define LWOB_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

#ifndef FALSE
#define FALSE		0
#endif

#ifndef TRUE
#define TRUE		1
#endif

// Comment this line to ignore detail polygons
// #define LWOB_WARN	0

// IEEE definitions

#ifndef HUGE_VAL
#define HUGE_VAL HUGE
#endif

#define FloatToUnsigned(f)	((unsigned long)(((long)(f - 2147483648.0)) + 2147483647L) + 1)
#define UnsignedToFloat(u)	(((double)((long)(u - 2147483647L - 1))) + 2147483648.0)

// LWOB definitions

#define LWOB_CH		8	// LWOB standard chunk size
#define LWOB_ID		4	// LWOB chunk ID size
#define LWOB_FORM	4	// LWOB FORM chunk size
#define LWOB_PNTS	12	// LWOB PNTS chunk size (per point)

#define LWOB_SIZE	3	// Polygon size (vertices)

#define LWOB_ERR_INVALID	-2	// Invalid LWOB file
#define LWOB_ERR_CORRUPT	-3	// Corrupt LWOB file
#define LWOB_ERR_NOPNTS		-4	// No points chunk in LWOB file
#define LWOB_ERR_NOSRFS		-5	// No surfaces chunk in LWOB file
#define LWOB_ERR_NOPOLS		-6	// No polygons chunk in LWOB file
#define LWOB_ERR_EMPTY		-7	// Empty polygons in LWOB file
#define LWOB_ERR_NOTRGL		-8	// No triangles in LWOB file
#define LWOB_ERR_DETAIL		-9	// Detail polygons in LWOB file
#define LWOB_ERR_ALLOC		-10	// Unable to allocate name

#ifdef __cplusplus
extern "C" {
#endif

// Read/write functions

short RBMShort(FILE *stream);	// Get short from stream

long RBMLong(FILE *stream);	// Get long from stream

float RBMIeee(FILE *pStream);	// Get IEEE 4-byte floating-point from stream

short WBMShort(short nVal,    // Put short to stream
	FILE *pStream);

long WBMLong(long nVal,		// Put long to stream
	FILE *pStream);

double ReadIeeeExtended(	    // Get IEEE floating-point from stream
	FILE *pStream);

int WriteIeeeExtended(		// Put IEEE floating-point to stream
	FILE *pStream,double fNum);

int ConvertToIeeeExtended(	// Convert IEEE floating-point to string
	char *aBytes,double fNum);

double ConvertFromIeeeExtended(	// Convert string to IEEE floating-point
	char *aBytes);

// LWOB functions

int GetLwobData(FILE *pStream,short *nVerts,short *nPolys,	// Get LWOB file data
	float **aVerts,short **aPolys);

char *GetLwobErr(int nErr);			// Get LWOB error

#ifdef __cplusplus
}		// extern "C"
#endif

#endif // LWOB_H
