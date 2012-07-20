/*=============================================================================
    Name    : BMP.c
    Purpose : Functions to read .BMP files.

    Created 1/11/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "File.h"
#include "Debug.h"
#include "BMP.h"

/*=============================================================================
    Functions
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : bmpFileOpen
    Description : Opens a .bmp file and reads in it's header.  Error checking
                    will be performed on the header to make sure it's a format
                    we can readily read.
    Inputs      : fileName - name of file to open
    Outputs     : header - where to store the bitmap information.
    Return      : a filehandle if the file was valid or 0 if invalid.
----------------------------------------------------------------------------*/
filehandle bmpFileOpen(bmpheader *header, char *fileName)
{
    filehandle handle;

    handle = fileOpen(fileName, FF_ReturnNULLOnFail);
    if (handle)
    {
        fileBlockRead(handle, header, sizeof(bmpheader));
        if (header->ident != BMP_Ident)
        {
#if BMP_VERBOSE_LEVEL >= 1
            dbgMessagef("\nBMP file '%s' has invalid ident of 0x%d", fileName, header->ident);
#endif

            fileClose(handle);
            return(0);
        }
        if (header->biBitCount != 24 || header->biCompression != 0)
        {
#if BMP_VERBOSE_LEVEL >= 1
            dbgMessagef("\nBMP file '%s' has invalid format", fileName);
#endif

            fileClose(handle);
            return(0);
        }
//        fileSeek(handle, 54, FS_Current);
        return(handle);
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : bmpBodyRead
    Description : Read in and close the body of a previously opened .BMP file.
    Inputs      : handle - previously opened file handle.
                  header - header previously opened.
    Outputs     : dest - where to store the read file.
                    Must be header->biWidth * header->biWidth dwords.
    Return      : void
----------------------------------------------------------------------------*/
void bmpBodyRead(color *dest, filehandle handle, bmpheader *header)
{
    sdword x, y;
    ubyte colors[3];
    color *writeDest;

    for (y = header->biHeight - 1; y >= 0; y--)
    {
        writeDest = dest + y * header->biWidth;
        for (x = 0; x < header->biWidth; x++, writeDest++)
        {
            fileBlockRead(handle, colors, 3);
            *writeDest = colRGB(colors[2], colors[1], colors[0]);
        }
    }
    fileClose(handle);
}


