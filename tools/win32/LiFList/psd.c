/*=============================================================================
    Name    : psh.c
    Purpose : Functions to load a .PSD file into a layered image format.
                Layered image format defined in layerimg.h.

    Created 9/29/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include "file.h"
#include "memory.h"
#include "debug.h"
#include "layerimg.h"
#include "psd.h"

/*=============================================================================
    Data:
=============================================================================*/
sdword psdBytesRead;
filehandle psdFileHandle;
sdword psdFirstAlphaChannel;

uword psdHeaderTemplate[] =
{
    PSD_TByte(4),               //signature
    PSD_TWord(1),               //version
    PSD_TByte(6),               //reserved
    PSD_TWord(1),               //nChannels
    PSD_TDword(2),              //width, height
    PSD_TWord(2),               //depth,colorMode
    0
};
/*
uword psdLayerMaskTemplate[] =
{
    PSD_TDword(4),              //bounds
    PSD_TByte(4),               //alpha/flags/padding
    0
};
*/
//table to convert the symbolic blending name to a uword enumeration
struct
{
    char name[4];
    uword type;
}
psdBlendNameTable[] =
{
    {"norm", LBT_Normal    },
    {"dark", LBT_Darken    },
    {"lite", LBT_Lighten   },
    {"hue ", LBT_Hue       },
    {"sat ", LBT_Saturation},
    {"colr", LBT_Color     },
    {"lum ", LBT_Luminosity},
    {"mul ", LBT_Multiply  },
    {"scrn", LBT_Screen    },
    {"diss", LBT_Dissolve  },
    {"over", LBT_Overlay   },
    {"hLit", LBT_HardLight },
    {"sLit", LBT_SoftLight },
    {"diff", LBT_Difference},
    {"div ", LBT_ColorDodge},
    {"idiv", LBT_ColorBurn },
    {"smud", LBT_Exclusion },
    {{0, 0, 0, 0}, 0}
};

/*=============================================================================
    Functions
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : psdBlendNameToWord
    Description : Convert a blending mode symbolic name to a uword enumeration.
    Inputs      : name - 4 characer name from .PSD file.
    Outputs     : ..
    Return      : corresponding uword enumeration.
----------------------------------------------------------------------------*/
uword psdBlendNameToWord(char *name)
{
    sdword index;

    for (index = 0; psdBlendNameTable[index].name[0] != 0; index++)
    {                                                       //for all entries in the table
        if (!memcmp(psdBlendNameTable[index].name, name, 4))
        {                                                   //if the names match
            return(psdBlendNameTable[index].type);
        }
    }
#if PSD_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "psdBlendNameToWord: blend type '%s' does not exist.", name);
#endif //PSD_ERROR_CHECKING
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : psdFileReadByte
    Description : Read a single byte in from the current .PSD file
    Inputs      : void
    Outputs     : Increments byte count of file.
    Return      : Byte read from file.
----------------------------------------------------------------------------*/
ubyte psdFileReadByte(void)
{
    psdBytesRead++;
    return((ubyte)fileCharRead(psdFileHandle));
}

/*-----------------------------------------------------------------------------
    Name        : psdFileReadWord
    Description : Read a single word in from the current .PSD file
    Inputs      : void
    Outputs     : Increments byte count of file.
    Return      : Word read from file.
----------------------------------------------------------------------------*/
uword psdFileReadWord(void)
{
    uword tempWord;

    fileBlockRead(psdFileHandle, &tempWord, 2);
    psdBytesRead += 2;
    return(uwordSwapBytes(tempWord));
}

/*-----------------------------------------------------------------------------
    Name        : psdFileReadDword
    Description : Read a single dword in from the current .PSD file
    Inputs      : void
    Outputs     : Increments byte count of file.
    Return      : Dord read from file.
----------------------------------------------------------------------------*/
udword psdFileReadDword(void)
{
    udword tempDword;

    fileBlockRead(psdFileHandle, &tempDword, 4);
    psdBytesRead += 4;
    return(udwordSwapBytes(tempDword));
}

/*-----------------------------------------------------------------------------
    Name        : psdFileReadBytes
    Description : Read a number of bytes in from the current .PSD file
    Inputs      : length - number of members to read in
                  dest - where to store read data
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void psdFileReadBytes(sdword length, ubyte *dest)
{
    fileBlockRead(psdFileHandle, dest, length);
    psdBytesRead += length;
}

/*-----------------------------------------------------------------------------
    Name        : psdFileReadWords
    Description : Read a number of words in from the current .PSD file
    Inputs      : length - number of members to read in
                  dest - where to store read data
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void psdFileReadWords(sdword length, uword *dest)
{
    sdword index;

    for (index = 0; index < length; index++)
    {
        dest[index] = psdFileReadWord();
    }
}

/*-----------------------------------------------------------------------------
    Name        : psdFileReadDwords
    Description : Read a number of dwords in from the current .PSD file
    Inputs      : length - number of members to read in
                  dest - where to store read data
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void psdFileReadDwords(sdword length, udword *dest)
{
    sdword index;

    for (index = 0; index < length; index++)
    {
        dest[index] = psdFileReadDword();
    }
}

/*-----------------------------------------------------------------------------
    Name        : psdFileSkipBytes
    Description : Skips a number of bytes in the .PSD file.
    Inputs      : nBytes - number of bytes to skip
    Outputs     : seeks in the file and updates psdBytesRead
    Return      : new offset in file
----------------------------------------------------------------------------*/
sdword psdFileSkipBytes(sdword nBytes)
{
    sdword newOffset;
    dbgAssert(nBytes >= 0);
    if (nBytes == 0)
    {
        return(psdBytesRead);
    }
    newOffset = fileSeek(psdFileHandle, nBytes, FS_Current);
    dbgAssert(psdBytesRead + nBytes == newOffset);
    psdBytesRead += nBytes;
    return(newOffset);
}

/*-----------------------------------------------------------------------------
    Name        : psdFileSkipTo
    Description : Skip to an absolute point in the .PSD file
    Inputs      : location - offset of new byte to seek to
    Outputs     : seeks in the file and updates psdBytesRead
    Return      : new offset (same as location)
----------------------------------------------------------------------------*/
sdword psdFileSkipTo(sdword location)
{
    psdBytesRead = location;
    return(fileSeek(psdFileHandle, location, FS_Start));
}

/*-----------------------------------------------------------------------------
    Name        : psdFileReadStructure
    Description : Load in a structure from the .PSD file.
    Inputs      : structure - structre which needs loading.
                  pTemplate - list of numbers which determine how to load it in
    Outputs     : reads into the structure according to the template
    Return      : new psdBytesRead
----------------------------------------------------------------------------*/
sdword psdFileReadStructure(ubyte *structure, uword *pTemplate)
{
    for (; *pTemplate; pTemplate++)
    {
        switch (*pTemplate & PSD_CodeMask)
        {
            case PSD_SkipCode:
                psdFileSkipBytes(*pTemplate & PSD_NumberMask);
                break;
            case PSD_ByteCode:
                psdFileReadBytes(*pTemplate & PSD_NumberMask, structure);
                structure += *pTemplate & PSD_NumberMask;
                break;
            case PSD_WordCode:
                psdFileReadWords(*pTemplate & PSD_NumberMask, (uword *)structure);
                structure += (*pTemplate & PSD_NumberMask) * 2;
                break;
            case PSD_DwordCode:
                psdFileReadDwords(*pTemplate & PSD_NumberMask, (udword *)structure);
                structure += (*pTemplate & PSD_NumberMask) * 4;
                break;
        }
    }
    return(psdBytesRead);
}

/*-----------------------------------------------------------------------------
    Name        : psdPascalStringRead
    Description : Read a pascal string from the .PSD file.
    Inputs      : void
    Outputs     : Allocates memory for and loads .PSD file.
    Return      : pointer to newly allocated string.
----------------------------------------------------------------------------*/
char *psdPascalStringRead(void)
{
    char *string;
    ubyte length;

    length = psdFileReadByte();                             //read length of string
    if (length == 0)                                        //return NULL string if zero length
    {
        return(NULL);
    }
    string = memAlloc((sdword)length + 1, "Pascal string", 0);//allocate string
    psdFileReadBytes((sdword)length, string);               //read it in
    string[length] = 0;                                     //NULL-terminate string
    if (((length + 1) & 3) > 0)
    {
        psdFileSkipBytes(4 - ((length + 1) & 3));           //round up to multiple of 4 bytes
    }
    return(string);
}

/*-----------------------------------------------------------------------------
    Name        : psdLayerLoadHeader
    Description : Load in the header information for a single layer
    Inputs      : layer - where to load the info to
    Outputs     : Fills in the layer and _most_ of the channel info for layer
    Return      : void
----------------------------------------------------------------------------*/
void psdLayerLoadHeader(lilayer *layer)
{
    sdword index, nBlenders, anchor, sectionLength;
    sword type;
    ubyte name[4];

    dbgAssert(layer);
    psdFileReadDwords(4, (udword *)&layer->bounds);         //read layer bounds
    swapInt(layer->bounds.x0, layer->bounds.y0);            //convert to our internal rect format
    swapInt(layer->bounds.x1, layer->bounds.y1);
    layer->nChannels = psdFileReadWord();                   //get number of channels
    layer->channels = memAlloc(layer->nChannels * sizeof (lichannel), "LayerChannelArray", 0);
    for (index = 0; index < layer->nChannels; index++)
    {
        layer->channels[index].scanLength = NULL;           //leave this stuff blank for now
        layer->channels[index].scanData = NULL;
        type = (sword)psdFileReadWord();                    //read channel type
        switch (type)
        {
            case -2:
                layer->channels[index].type = LCT_LayerMask;
                break;
            case -1:
                layer->channels[index].type = LCT_Transparency;
                break;
            case 0:
            case 1:
            case 2:
                layer->channels[index].type = (ubyte)type;
                break;
#if PSD_ERROR_CHECKING
            default:
                dbgFatalf(DBG_Loc, "Undefined layer channel type %d", type);
#endif //PSD_ERROR_CHECKING
        }
        //for now, store the length of the channel data in this pointer
        layer->channels[index].scanLength = (uword *)psdFileReadDword();
    }
#if PSD_ERROR_CHECKING
    psdFileReadBytes(4, name);
    dbgAssert(!memcmp(name, "8BIM", 4));                    //just a sanity check
#else
    psdFileSkipBytes(4);                                    //skip the '8BIM'
#endif //PSD_ERROR_CHECKING
    psdFileReadBytes(4, name);
    layer->blendMode = psdBlendNameToWord(name);
    layer->opacity = (real32)psdFileReadByte() / 255.0f;    //read opacity and convert to floating-point
    psdFileReadBytes(3, name);                              //read clipping/flags/filler byte
    if (name[0] == 0)
    {                                                       //check clipping base
        layer->flags = LFF_ClippingBase;
    }
    else
    {
        layer->flags = 0;
    }
    if (name[1] & 1)
    {                                                       //set transparency protect bit
        layer->flags |= LFF_TransProtect;
    }
    if (!(name[1] & 2))
    {                                                       //set visible bit
        layer->flags |= LFF_Visible;
    }
    layer->flags |= LFF_Channeled;                          //this layer is compressed into it's channels
    sectionLength = psdFileReadDword();                     //length of remaining data
    anchor = psdBytesLoaded(0);
    // we will skip over the layer mask info because we don't implement it
    psdFileSkipBytes(psdFileReadDword());
    nBlenders = psdFileReadDword();                         //length of blending ranges
    nBlenders = nBlenders / sizeof(liblendrange) - 1;
    dbgAssert(nBlenders >= layer->nChannels);                //make sure there is at least 1 blend range
    psdFileReadWords(4, (uword *)&layer->greyBlendRange);   //read grey blend range
    for (index = 0; index < layer->nChannels; index++)
    {
        psdFileReadWords(4, (uword *)&layer->channels[index].blendRange);//read channel blending ranges
    }
    layer->name = psdPascalStringRead();
    dbgAssert(sectionLength - psdBytesLoaded(anchor) >= 0);
    psdFileSkipBytes(sectionLength - psdBytesLoaded(anchor));       //skip the Adjustment layer
}

/*-----------------------------------------------------------------------------
    Name        : psdLayerLoadChannels
    Description : Load in the pixel data for all channels in the layer
    Inputs      : layer - previously initialized layer/channel headers
    Outputs     : Allocates memory for each channel and loads it in.
    Return      : void
----------------------------------------------------------------------------*/
void psdLayerLoadChannels(lilayer *layer)
{
    sdword index;
    udword length;

    for (index = 0; index < layer->nChannels; index++)
    {                                                       //for each channel in image
        length = (udword)layer->channels[index].scanLength; //retrieve previously stored length
        layer->channels[index].compressed = (ubyte)psdFileReadWord();
        if (length <= 2)
        {                                                   //if no channel data at all
            layer->channels[index].type = LCT_Invalid;
            continue;
        }
    if (layer->channels[index].compressed == 0)
    {           //- 2 for the length word               //if raw data in channel
            dbgAssert((udword)(layer->bounds.x1 - layer->bounds.x0) * //make sure right length
                      (udword)(layer->bounds.y1 - layer->bounds.y0) == length - 2);
            layer->channels[index].scanData =
                memAlloc(length - 2, "Raw channel data", 0);
            layer->channels[index].scanLength = NULL;
            layer->channels[index].compressed = 0;
            psdFileReadBytes(length - 2, layer->channels[index].scanData);
        }
        else
        {                                                   //else it's RLE compressed
            dbgAssert(layer->channels[index].compressed == 1);
            layer->channels[index].scanLength =             //allocate RAM for RLE buffer
                memAlloc(length - 2, "RLE channel data", 0);
            layer->channels[index].scanData =               //compute address of actual RLE data
                (ubyte *)(layer->channels[index].scanLength +
                          (layer->bounds.y1 - layer->bounds.y0));
            psdFileReadWords((layer->bounds.y1 - layer->bounds.y0),
                             layer->channels[index].scanLength);//read the length of each scan
            psdFileReadBytes(length - 2 - (layer->bounds.y1 - layer->bounds.y0) * 2, layer->channels[index].scanData);
            layer->channels[index].compressed = 1;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : psdFileHeaderRead
    Description : Load in the header of a newly-opened .PSD file.  Also performs
                    type verification.
    Inputs      : fileHeader - pointer to header to recieve new header
    Outputs     : stores loaded header in fileHeader
    Return      : void
----------------------------------------------------------------------------*/
void psdFileHeaderRead(psdfileheader *fileHeader, char *fileName)
{
    psdFileReadStructure((ubyte *)fileHeader, psdHeaderTemplate);
    swapInt(fileHeader->width, fileHeader->height);           //order of these seems to be swapped in the files

#if PSD_ERROR_CHECKING
    if (fileHeader->version != PSD_Version ||                //if incorrect version #
        memcmp(fileHeader->signature, PSD_FileSignature, PSD_FileSignatureLength))
    {                                                       //or signature invalid
        dbgFatalf(DBG_Loc, "'%s' is not a .PSD file");
    }

    if (fileHeader->colorMode != PCM_RBG)                    //if not the correct color mode
    {
        dbgFatalf(DBG_Loc, ".PSD file '%s' is color mode %d (not RGB)", fileName, fileHeader->colorMode);
    }
    if (fileHeader->depth != 8)                              //if not the correct pixel depth
    {
        dbgFatalf(DBG_Loc, ".PSD file '%s' is pixel depth %d (not 8)", fileName, fileHeader->depth);
    }
#endif //PSD_ERROR_CHECKING
}

/*-----------------------------------------------------------------------------
    Name        : psdLayeredFileLoad
    Description : Decode and load in the named Photoshop Document file
    Inputs      : fileName - name of file to load
    Outputs     : Allocated memory for and buids a layerimage structure based
                    upon the file.
    Return      : pointer to layered image.
----------------------------------------------------------------------------*/
layerimage *psdLayeredFileLoad(char *fileName)
{
    psdfileheader fileHeader;
    layerimage *image;
    udword sectionLength, anchor;
    sword nLayers, index;

#if PSD_VERBOSE_LEVEL >= 1
    dbgMessagef("\npsdLayeredFileLoad: Opening image %s", fileName);
#endif
    psdFileHandle = fileOpen(fileName, 0);                  //open the file
    psdBytesRead = 0;                                       //nothin read yet

    psdFileHeaderRead(&fileHeader, fileName);               //read the header

    //now skip over the color mode data section
    sectionLength = psdFileReadDword();
    psdFileSkipBytes(sectionLength);

    //now skip over the image resources.  Sure there's nothing we need here?
    //yes we might need: (but we can skip for now)
    //          -0x3ee (names of alpha channels as Pascal strings)
    //          -0x3fe (quick mask information)
    sectionLength = psdFileReadDword();
    psdFileSkipBytes(sectionLength);

    //now we read the length of the layer section
    sectionLength = psdFileReadDword();
#if PSD_ERROR_CHECKING
    if (sectionLength < 4)
    {
        dbgFatalf(DBG_Loc, ".PSD file '%s' has no layers.  A lot of good that is!", fileName);
    }
#endif
    anchor = psdBytesLoaded(0);
    /*sectionLength =*/ psdFileReadDword();                     //now get length of actual layer info
    nLayers = (sword)psdFileReadWord();
    if (nLayers < 0)
    {
        nLayers = -nLayers;
        psdFirstAlphaChannel = TRUE;
    }
    else
    {
        psdFirstAlphaChannel = FALSE;
    }
#if PSD_ERROR_CHECKING
    if (sectionLength < 4 || nLayers < 1)
    {
        dbgFatalf(DBG_Loc, ".PSD file '%s' has no layers.  A lot of good that is!", fileName);
    }
#endif

    //allocate and initialize the image structure
    image = memAlloc(liHeaderSize(nLayers), "Layered image", 0);
#if LI_RETAIN_NAME
    image->fileName = memStringDupe(fileName);
#endif
    image->width = fileHeader.width;
    image->height = fileHeader.height;
    image->nLayers = nLayers;
#if PSD_VERBOSE_LEVEL >= 1
    dbgMessagef(" size %d x %d, %d layers", image->width, image->height, image->nLayers);
#endif

    for (index = 0; index < nLayers; index++)
    {                                                       //for each layer
        psdLayerLoadHeader(&image->layers[index]);          //load in layer headers
#if PSD_VERBOSE_LEVEL >= 2
        dbgMessagef("\npsdLayeredFileLoad: Layer %d '%s' has %d channels",
                    index, image->layers[index].name, image->layers[index].nChannels);
#endif
    }
    // read the pixel data
    for (index = 0; index < nLayers; index++)
    {                                                       //for each layer
        psdLayerLoadChannels(&image->layers[index]);        //load in layer channel data
        if ((image->layers[index].bounds.x1 - image->layers[index].bounds.x0) * //if NULL layer
            (image->layers[index].bounds.y1 - image->layers[index].bounds.y0) == 0)
        {
            liLayerDelete(&image->layers[index]);           //delete it
        }
    }
    psdFileSkipBytes(sectionLength - psdBytesLoaded(anchor));//skip whatever is left
    dbgAssert(psdBytesLoaded(anchor) == sectionLength);

    //skip rest of file because we've got all we need
    fileClose(psdFileHandle);                               //close the file
    return(image);
}

/*-----------------------------------------------------------------------------
    Name        : psdLayeredImageMeasure
    Description : Detect if a particular .PSD file exists.  If it does, load
                    in it's header and report it's dimenstions.
    Inputs      : fileName - name of .PSD file
                  width/height - pointers to integers to recieve the
                    width/height of the image.
    Outputs     : stores width/height if image can be found.
    Return      : TRUE if the image was opened properly.
----------------------------------------------------------------------------*/
sdword psdLayeredImageMeasure(char *fileName, sdword *width, sdword *height)
{
    psdfileheader header;
    if (fileExists(fileName, 0))
    {                                                           //if the file exists
        psdFileHandle = fileOpen(fileName, 0);                  //open the file
        psdBytesRead = 0;                                       //nothin read yet
        psdFileHeaderRead(&header, fileName);                   //read the file header
        fileClose(psdFileHandle);
        *width = header.width;
        *height = header.height;
        return(TRUE);
    }
    return(FALSE);
}

