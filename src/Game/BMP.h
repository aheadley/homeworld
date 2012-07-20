/*=============================================================================
    Name    : BMP.H
    Purpose : Definitions for loading and saving .bmp files.

    Created 1/11/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___BMP_H
#define ___BMP_H

#include "color.h"
#include "File.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define BMP_ERROR_CHECKING      1               //basic error checking
#define BMP_VERBOSE_LEVEL       1               //control verbose printing

#else //HW_Debug

#define BMP_ERROR_CHECKING      0               //basic error checking
#define BMP_VERBOSE_LEVEL       0               //control verbose printing

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define BMP_Ident       0x4d42
/*=============================================================================
    Type definitions:
=============================================================================*/

//because these images use structures with 2-byte boundaries, let's set the
//structure packing to words for this file only.
#pragma warning( disable : 4103 )
#pragma pack(push, 2)

//from windows.h, the definition for the header of a .bmp
typedef struct
{
    uword  ident;
    ubyte someMiscCrap[16];
    sdword biWidth;
    sdword biHeight;
    sword  biPlanes;
    sword  biBitCount;
    udword biCompression;
    udword biSizeImage;
    sdword biXPelsPerMeter;
    sdword biYPelsPerMeter;
    udword biClrUsed;
    udword biClrImportant;
} bmpheader;

//because these images use structures with 2-byte boundaries, let's set the
//structure packing to words for this file only.
#pragma pack(pop)

/*=============================================================================
    Macros:
=============================================================================*/
#define bmpClose(handle)    fileClose(handle)

/*=============================================================================
    Functions:
=============================================================================*/
filehandle bmpFileOpen(bmpheader *header, char *fileName);
void bmpBodyRead(color *dest, filehandle handle, bmpheader *header);

#endif //___BMP_H

