//-----------------------------------------------------------------------------
// ---------------------
// File ....: interfce.h
// ---------------------
// Author...: Gus J Grubba
// Date ....: October 1995
// Descr....: C Interface to JPEG library
//
// History .: Oct, 27 1995 - Started
//
//-----------------------------------------------------------------------------

#ifndef __JPEGINTERFACE_H__
#define __JPEGINTERFACE_H__

#include "file.h"

typedef struct _jpegdata {
   unsigned char *ptr;
   int    width;
   int    height;
   FILE  *output_file;
   filehandle  input_file;
   int    aritcoding;
   int    CCIR601sampling;
   int    smoothingfactor;
   int    quality;
   udword hWnd;
   int    ProgressMsg;
   int    status;
   int    components;
} JPEGDATA;

void JpegWrite( JPEGDATA *data );
void JpegInfo(  JPEGDATA *data );
void JpegRead(  JPEGDATA *data );

//-- interfce.c ---------------------------------------------------------------

#endif
