/*=============================================================================
    Name    : psd.h
    Purpose : Definitions for loading in Photoshop Version 3.0/4.0 files.
                PSD files are loaded into an internal format which could later
                be used as a more efficient file type.
              Warning! this file needs to be compiles with structure packing
                enabled in order to work properly.

    Created 9/29/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PSD_H
#define ___PSD_H

#include "types.h"
#include "prim2d.h"
#include "color.h"
#include "layerimg.h"

//because these images use structures with 2-byte boundaries, let's set the
//structure packing to words for this file only.
#pragma warning( disable : 4103 )
#pragma pack(2)

/*=============================================================================
    Switches:
=============================================================================*/
#define PSD_TEST                0               //load in a test image for development

#ifdef HW_Debug

#define PSD_ERROR_CHECKING      1               //basic error checking
#define PSD_VERBOSE_LEVEL       1               //control verbose printing

#else //HW_Debug

#define PSD_ERROR_CHECKING      0               //no error ckecking in retail
#define PSD_VERBOSE_LEVEL       0               //don't print any verbose strings in retail

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define PSD_FileSignature       "8BPS"
#define PSD_FileSignatureLength 4
#define PSD_Version             1

//.PSD file color modes
#define PCM_Bitmap              0
#define PCM_Greyscale           1
#define PCM_Indexed             2
#define PCM_RBG                 3
#define PCM_CMYK                4
#define PCM_MultiChannel        7
#define PCM_Duotone             8
#define PCM_Lab                 9

#define PSD_SkipCode            0xc000
#define PSD_ByteCode            0x8000
#define PSD_WordCode            0x4000
#define PSD_DwordCode           0x0000
#define PSD_CodeMask            0xc000
#define PSD_NumberMask          0x3fff
#define PSD_TSkip(n)            (((n) &  PSD_NumberMask) | PSD_SkipCode)
#define PSD_TByte(n)            (((n) &  PSD_NumberMask) | PSD_ByteCode)
#define PSD_TWord(n)            (((n) &  PSD_NumberMask) | PSD_WordCode)
#define PSD_TDword(n)           (((n) &  PSD_NumberMask) | PSD_DwordCode)

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure of the .PSD file header
typedef struct
{
    ubyte signature[4];
    uword version;
    ubyte reserved[6];
    uword nChannels;                            //number of channels in image
    udword width;                               //dimensions of image
    udword height;
    uword depth;                                //bit depth of channels (1, 8, 16)
    uword colorMode;                            //(see above definitions)
}
psdfileheader;

/*=============================================================================
    Macros:
=============================================================================*/
//number of bytes read since a certain 'anchor' point in the file
#define psdBytesLoaded(a)       (psdBytesRead - (a))

/*=============================================================================
    Functions:
=============================================================================*/

//see if a .PSD file can be loaded and return it's size if possible
sdword psdLayeredImageMeasure(char *fileName, sdword *width, sdword *height);

//load in a .PSD file
layerimage *psdLayeredFileLoad(char *fileName);

//helper functions for loading big-endian data fields
ubyte psdFileReadByte(void);
uword psdFileReadWord(void);
udword psdFileReadDword(void);
void psdFileReadBytes(sdword length, ubyte *dest);
void psdFileReadWords(sdword length, uword *dest);
void psdFileReadDwords(sdword length, udword *dest);
sdword psdFileSkipBytes(sdword nBytes);
sdword psdFileSkipTo(sdword location);
#endif //___PSD_H

