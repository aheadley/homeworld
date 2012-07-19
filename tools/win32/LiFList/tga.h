/*=============================================================================
    Name    : TGA.C
    Purpose : Code to load and save Truevision Targa files

    Created 3/1/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___TGA_H
#define ___TGA_H

#include "types.h"
#include "color.h"

/*=============================================================================
    Definitions:
=============================================================================*/

//for identifying the file:
#define TGA_SigLength           (26-8)
#define TGA_Signature           "TRUEVISION-XFILE"

//image types
#define TIT_NoImage             0               //no image data
#define TIT_RawPaletted         1               //uncompressed paletted image
#define TIT_RawRGB              2               //    "        RGB image
#define TIT_RawBW               3               //    "        black-and-white image
#define TIT_RLEPaletted         9               //rle versions of above
#define TIT_RLERGB              10
#define TIT_RLEBW               11

//because these images use structures with 1-byte boundaries, let's set the
//structure packing to bytes for this file only.
#pragma warning( disable : 4103 )
#pragma pack(1)
/*=============================================================================
    Type definitions:
=============================================================================*/
//header of the file
typedef struct
{
    ubyte imageIDLength;                        //length of the image ID
    ubyte colorMapType;                         //type of color map-0 = no color map
    ubyte imageType;                            //see definitions above
    uword firstEntry;                           //first entry in color palette
    uword mapLength;                            //length of color palette
    ubyte paletteBitsPerPixel;                  //bits per pixel of the color palette
    uword xOrigin;                              //origin of the image
    uword yOrigin;
    uword width;                                //size of the image
    uword height;
    ubyte bitsPerPixel;                         //bpp of the actual image
    ubyte imageDescriptor;                      //alpha, origin descriptor but usually zero
}
tgaheader;

//footer of the file
typedef struct
{
    udword extensionOffset;                     //offset to the extension area of the file
    udword developerOffset;                     //offset to the developer area of the file
    ubyte signature[TGA_SigLength];             //file signature
}
tgafooter;

/*=============================================================================
    Functions:
=============================================================================*/

bool tgaHeaderWriteRGB(char *fileName, char *imageID, sdword width, sdword height);
void tgaBodyWriteRGB(color *data, sdword width, sdword height);
void tgaWriteClose(void);
#endif ___TGA_H


