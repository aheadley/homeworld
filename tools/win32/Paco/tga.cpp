/*=============================================================================
    Name    : TGA.C
    Purpose : Functions for creating and reading TGA files

    Created 3/1/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "tga.h"

/*=============================================================================
    Data:
=============================================================================*/
FILE *tgaWriteHandle;
bool tgaWriteOpen = false;

#define MIN(A,B) (((A) < (B)) ? (A) : (B))

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : tgaHeaderWriteRGB
    Description : Creates a TGA file and writes out it's header.
    Inputs      : fileName - name of file to create
                  imageID - optional image ID to save to file (may be NULL)
                  width, height - dimensions of image
    Outputs     :
    Return      : true if success
----------------------------------------------------------------------------*/
bool tgaHeaderWriteRGB(char *fileName, char *imageID, sdword width, sdword height)
{
    tgaheader header;

    if (imageID != NULL)
    {                                                       //set length of image ID
        header.imageIDLength = (ubyte)(MIN(strlen(imageID) + 1, 255));
    }
    else
    {
        header.imageIDLength = 0;
    }
    header.colorMapType = 0;                                //no color map
    header.imageType = TIT_RawRGB;                          //raw RGB file
    header.firstEntry = header.mapLength = 0;               //no stinkin palette
    header.paletteBitsPerPixel = 0;
    header.xOrigin = header.yOrigin = 0;
    header.width = (uword)width;
    header.height = (uword)height;
    header.bitsPerPixel = 24;                               //RGB file
    header.imageDescriptor = 0x20;

    tgaWriteHandle = fopen(fileName, "wb");                 //open the file
    if (tgaWriteHandle == NULL)
    {
        return(false);
    }
    fwrite(&header, 1, sizeof(header), tgaWriteHandle);     //write the header
    if (imageID != NULL)
    {                                                       //write the image ID
        fwrite(imageID, 1, header.imageIDLength, tgaWriteHandle);
    }
    tgaWriteOpen = true;
    return(true);
}

/*-----------------------------------------------------------------------------
    Name        : tgaBodyWrite
    Description : Write the body of a TGA file
    Inputs      : data - image we're writing out
                  width, height - size of image, must match dimensions passed
                    to header write function.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void tgaBodyWriteRGB(color *data, sdword width, sdword height)
{
    sdword count;

    for (count = width * height; count > 0; count--, data++)
    {
        fputc(colBlue(*data), tgaWriteHandle);
        fputc(colGreen(*data), tgaWriteHandle);
        fputc(colRed(*data), tgaWriteHandle);
    }
}

/*-----------------------------------------------------------------------------
    Name        : tgaWriteClose
    Description : Close the open tga write file after writing out a footer.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tgaWriteClose(void)
{
    tgafooter footer;

    footer.extensionOffset = footer.developerOffset = 0;
    memcpy(footer.signature, TGA_Signature, TGA_SigLength);
    fwrite(&footer, 1, sizeof(footer), tgaWriteHandle);
    fclose(tgaWriteHandle);
    tgaWriteOpen = false;
}

//
// tgaExportLIF
//
void tgaExportLIF(lifheader* lif, char* fileName)
{
    ubyte* data;
    ubyte* sp;
    color* pal;
    sdword size;

    if (lif->flags & TRF_Paletted)
    {
        //paletted image

        size = lif->width * lif->height;
        data = new ubyte[4*size];

        sp = lif->data;
        for (sdword i = 0; i < size; i++)
        {
            pal = lif->palette + sp[i];
            data[4*i+0] = colRed(*pal);
            data[4*i+1] = colGreen(*pal);
            data[4*i+2] = colBlue(*pal);
            data[4*i+3] = 255;
        }

        tgaHeaderWriteRGB(fileName, "Paco", lif->width, lif->height);
        tgaBodyWriteRGB((color*)data, lif->width, lif->height);
        tgaWriteClose();

        delete [] data;
    }
    else
    {
        tgaHeaderWriteRGB(fileName, "Paco", lif->width, lif->height);
        tgaBodyWriteRGB((color*)lif->data, lif->width, lif->height);
        tgaWriteClose();
    }
}
