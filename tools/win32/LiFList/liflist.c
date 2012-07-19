/*=============================================================================
    Name    : Liflist.c
    Purpose : Main source file for creating a list of .LIF files.

    Created 1/2/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include "debug.h"
#include "memory.h"
#include "psd.h"
#include "crc32.h"
#include "quantize.h"
#include "twiddle.h"
#include "tga.h"
#include "file.h"

/*=============================================================================
    Definitions:
=============================================================================*/
#define LC_Nothing                  0
#define LC_Quantize                 1
#define LC_View                     2
#define LC_Subtract                 3
#define LC_Add                      4
#define LC_Usage                    5
#define LC_Filter                   6

#define LL_FileIdentifier           "Event13"
#define LL_FileVersion              0x104
#define LL_MaxElements              12000
#define TS_Delimiters               " \t\n,"
#define LL_MinWidth                 2
#define LL_MaxWidth                 512
#define LL_MinHeight                2
#define LL_MaxHeight                512
#define LL_MaxTexEntries            1024
//tokens defining special layers
#define TRT_BaseColor1              "COL 1"     //colorization layers
#define TRT_BaseColor2              "COL1"
#define TRT_DetailColor1            "COL 2"
#define TRT_DetailColor2            "COL2"
#define TRT_Texture                 "TEX"       //base layers
#define TRT_PanelLinesBlack         "PB"        //detail layers
#define TRT_PanelLinesWhite         "PW"
#define TRT_Labels                  "LAB"
#define TRT_DirtAndGrime            "DIRT"
#define TRT_Badge                   "BADGE"     //now defunct? (ignore for now)
#define TRT_Alpha                   "ALPHA"     //alpha-channel in alpha-channel sprites
#define TR_ColoringThreshold        128
//flags for texture list
#define LIF_FileIdentifier          "Willy 7"   //identifier for .LiF files
#define LIF_FileVersion             0x104       //version number for
#define LL_BadCRC                   0xdeadbabe

//flags for elements in a texture list
#define TRF_Paletted                0x00000002  //texture uses a palette
#define TRF_Alpha                   0x00000008  //alpha channel image
#define TRF_TeamColor0              0x00000010  //team color flags
#define TRF_TeamColor1              0x00000020
#define TRF_IgnoreDimensions        0x00000100  //ignore dimensions (they're probably unusual)

#define LL_TeamEffectBandLo         16
#define LL_TeamEffectBandHi         48
#define LL_ZeroErrorFactor          10
#define LL_NumberTeamColors         4
#define LL_MultiplierSquish         128

/*-----------------------------------------------------------------------------
    Definitions for encoding
-----------------------------------------------------------------------------*/

#define LLC_NumberImages            5
#define LLC_RedRange                32
#define LLC_GreenRange              64
#define LLC_BlueRange               32
#define LLC_AlphaRange              32
#define LLC_Team0Range              64
#define LLC_Team1Range              64

#define TR_MinimumSizeX		        1
#define TR_MinimumSizeY		        1

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure for a single RLE compressed image.  Size of image determined by container structure
typedef struct
{
    sdword redSize;
    sdword greenSize;
    sdword blueSize;
    sdword alphaSize;
    sdword team0Size;
    sdword team1Size;
    ubyte *red;
    ubyte *green;
    ubyte *blue;
    ubyte *alpha;
    ubyte *team0;
    ubyte *team1;
    sdword imageSize;
}
rleimage;
//structure for LLC_NumberImages different sizes of an image RLE compressed
//the sizes are: w*h, w/2*h, w/2*h/2, w/4*h/2, w/4*h/4
//size of largest image determined by container structure
typedef struct
{
    rleimage image[LLC_NumberImages];
    sdword nImages;
}
rleallimage;

//formats for lif listing files
typedef struct
{
    char ident[8];                              //compared to "Event13"
    sdword version;                             //version number
    sdword nElements;                           //number of textures listed
    sdword stringLength;                        //length of all strings
    sdword sharingLength;                       //length of all offsets
    sdword totalLength;                         //total length of file, this header not included
}
llfileheader;
typedef struct
{
    char *textureName;                          //name of texture, an offset from start of string block
    sdword width, height;                       //size of texture
    udword flags;                               //flags for things like alpha and luminance map
    crc32 imageCRC;                             //crc of the unquantized image
//    sdword nSizes;                              //number of image sizes
//    sdword imageSize[LLC_NumberImages];         //size of the image
    sdword nShared;                             //number of images which share this one
    sdword *sharedTo;                           //list of indices of images which share this one
    sdword sharedFrom;                          //image this one is shared from, or -1 if none
}
llelement;

//structure to assist computation of team effect palettes
typedef struct
{
    udword bestError;
    udword bestIndex;
}
errorinfo;

//structure for managing texture lists
typedef struct
{
    udword flags;                               //image flags
    layerimage *image;                          //image from file
    color *composited;                          //composited version
    ubyte *teamEffect0, *teamEffect1;           //team color effect buffers
    ubyte *teamEffectFull;                      //temporary buffer made by combining the two above
    ubyte *quantized;                           //quantized version (RGB)
    char *sourcePath;                           //filename of source
    char *destPath;                             //filename of dest
    errorinfo *error;                           //error info for computing ideal team effect palettes
}
texentry;

//formats for the .LiF files
typedef struct
{
    char ident[8];                              //compared to "Willy 7"
    sdword version;                             //version number
    sdword flags;                               //to plug straight into texreg flags
    sdword width, height;                       //dimensions of image
    crc32 paletteCRC;                           //a CRC of palettes for fast comparison
    crc32 imageCRC;                             //crc of the unquantized image
    udword data;                                //actual image
    udword palette;                             //palette for this image
    udword teamEffect0, teamEffect1;            //palettes of team color effect
//    sdword nSizes;                              //number of sizes encoded
//    udword image[LLC_NumberImages];             //offset-pointers to the individual images (from end of this header)
}
lifheader;

/*=============================================================================
    Data:
=============================================================================*/
void *utyMemoryHeap;
sdword MemoryHeapSize = MEM_HeapSizeDefault;
char errorString[256];

//current command under execution
sdword llCommand = LC_Nothing;
char *llFileName = NULL;
sdword llNElements = 0;
llelement *llElementList = NULL;
char llListingPath[_MAX_DIR];

//usage string
char usageString[] = "\
Usage: %s <Options> <Action> <FileName> [<Parameters>]\n\
Options (preceeded by a '/' or a '-'):\n\
    -A - Force the image to be alpha.\n\
    -I - Ignore dimensional errors in image.\n\
    -V - View results of quantization.\n\
Actions:\n\
    Q - Quantize specified list of textures to a common palette and add them to\n\
         the listing file.\n\
    A - Add the contents of a .Mif file to the listing file.\n\
    S - Subtract the contents of a .Mif file from the listing file.\n\
    F - List any files not on the local hard drive to stdout.\n\
    U - Print VRAM usage stats for the specified texture listing.\n\
    V - View contents of listing file.\n\
FileName:\n\
    Name of listing file.\n\
Parameters:\n\
    Except with the 'V' action, name of a .MIF file.\n\
";

char *trAlphaNames[] =
{
    TRT_Alpha,
    NULL
};
char *trBaseColorNames[] =
{
    TRT_BaseColor1,
    TRT_BaseColor2,
    NULL
};
char *trDetailColorNames[] =
{
    TRT_DetailColor1,
    TRT_DetailColor2,
    NULL
};
char *trOtherLayerNames[] =
{
    TRT_Texture        ,
    TRT_PanelLinesBlack,
    TRT_PanelLinesWhite,
    TRT_Labels         ,
    TRT_DirtAndGrime   ,
    TRT_Alpha,
    NULL
};
char *trAllLayerNames[] =
{
    TRT_BaseColor1,
    TRT_BaseColor2,
    TRT_DetailColor1,
    TRT_DetailColor2,
    TRT_Texture        ,
    TRT_PanelLinesBlack,
    TRT_PanelLinesWhite,
    TRT_Labels         ,
    TRT_DirtAndGrime   ,
    TRT_Alpha,
    NULL
};

//option variables
bool forceAlpha = FALSE;
bool viewResults = FALSE;
bool forceDimensions = TRUE;
sdword verboseLevel = 0;

//team color for team colorizing textures
//color llTeamColor0 = colRGB(252, 38, 255), llTeamColor1 = colRGB(41, 255, 0);
color llTeamColor0 = colRGB(255, 0, 0), llTeamColor1 = colRGB(255, 255, 255);

color llTeamColors0[LL_NumberTeamColors] =
{
    colRGB(252, 38, 255),           //purple-green
    colRGB(255, 0, 0),              //red-white
    colRGB(0, 0, 0),                //black-black
    colRGB(255, 255, 255)           //white-white
};
color llTeamColors1[LL_NumberTeamColors] =
{
    colRGB(41, 255, 0),
    colRGB(255, 255, 255),
    colRGB(0, 0, 0),
    colRGB(255, 255, 255)
};

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : usagePrint
    Description : Print usage information for the program, then exit
    Inputs      : name - name of .exe
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void usagePrint(char *name)
{
    fprintf(stderr, usageString, name);
    exit(0xfed5);
}

/*-----------------------------------------------------------------------------
    Name        : llSystemsStartup
    Description : Start up systems required by program
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
char *llSystemsStartup(void)
{
    //memory
    utyMemoryHeap = (void *)GlobalAlloc(GPTR, MemoryHeapSize + sizeof(memcookie) * 4);
    if (utyMemoryHeap == NULL)
    {
        sprintf(errorString, "Error allocating heap of size %d", MemoryHeapSize);
        return(errorString);
    }
    if (memInit(utyMemoryHeap, MemoryHeapSize) != OKAY)
    {
        sprintf(errorString, "Error starting memory manager with heap size %d at 0x%x", MemoryHeapSize, errorString);
        return(errorString);
    }

    //allocate a blank element list
    llElementList = memAlloc(LL_MaxElements * sizeof(llelement), "Element List", 0);
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : llSystemsShutdown
    Description : Shut down systems required by the system
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void llSystemsShutdown(void)
{
    //memory
    GlobalFree(utyMemoryHeap);
}

/*-----------------------------------------------------------------------------
    Name        : llFileInit
    Description : Loads in the named liflist file into llElementList
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void llFileInit(void)
{
    FILE *f;
    llfileheader header;
    ubyte *stringBlock, *sharingBlock;
	sdword index;

    f = fopen(llFileName, "rb");
    if (f == NULL)
    {                                                       //if file not found
        dbgMessagef("File '%s' not found - creating new one.", llFileName);
        llNElements = 0;                                    //empty element list
        return;
    }
    fread(&header, 1, sizeof(llfileheader), f);             //read the file header
    if (strcmp(header.ident, LL_FileIdentifier) != 0)
    {
        dbgFatalf(DBG_Loc, "Incorrect identifier string: '%s', not '%s' as it should be.", header.ident, LL_FileIdentifier);
    }
    if (header.version != LL_FileVersion)
    {
        dbgFatalf(DBG_Loc, "Incorrect version 0x%x, expected 0x%x.", header.version, LL_FileVersion);
    }
    if (header.nElements < 0 || header.nElements >= LL_MaxElements)
    {
        dbgFatalf(DBG_Loc, "Unexpected number of elements: %d", header.nElements);
    }
    dbgAssert((udword)header.totalLength > header.nElements * sizeof(llelement));

    //read in the existing file to the global element list
    fread(llElementList, 1, sizeof(llelement) * header.nElements, f);
    llNElements = header.nElements;
    stringBlock = memAlloc(header.stringLength, "String Block", 0);
    fread(stringBlock, 1, header.stringLength, f);

    //fix up the names of the texture files
    for (index = 0; index < llNElements; index++)
    {
        llElementList[index].textureName += (udword)stringBlock;
    }
    //read in the sharing lists
    //they're discarded at flush time but are needed for the view function
    if (header.sharingLength != 0)
    {
        sharingBlock = memAlloc(header.sharingLength, "Sharing block", 0);
        fread(sharingBlock, 1, header.sharingLength, f);

        //fix up pointers to the sharing lists
        for (index = 0; index < llNElements; index++)
        {
            if (llElementList[index].nShared != 0)
            {
                llElementList[index].sharedTo = (sdword *)((udword)llElementList[index].sharedTo + (udword)sharingBlock);
            }
        }
    }
    //close the file
    fclose(f);
}

/*-----------------------------------------------------------------------------
    Name        : llElementsView
    Description : List the contents of the elements list
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void llElementsView(void)
{
    sdword index, count;
    crc32 crc;

    printf("\nListing of %s: %d textures\n#     width  height flags(PA12)  CRC     sharedFrom/sharedTo name\n------------------------------------------------------------", llFileName, llNElements);
    for (index = 0; index < llNElements; index++)
    {
        crc = llElementList[index].imageCRC;
        printf("\n%4d%8d%8d   %c%c%c%c  0x%x%x%x%x%x%x%x%x  ", index,
            llElementList[index].width, llElementList[index].height,
            bitTest(llElementList[index].flags, TRF_Paletted) ? 'P' : ' ',
            bitTest(llElementList[index].flags, TRF_Alpha) ? 'A' : ' ',
            bitTest(llElementList[index].flags, TRF_TeamColor0) ? '1' : ' ',
            bitTest(llElementList[index].flags, TRF_TeamColor1) ? '2' : ' ',
            (crc >> 28) & 0xf,
            (crc >> 24) & 0xf,
            (crc >> 20) & 0xf,
            (crc >> 16) & 0xf,
            (crc >> 12) & 0xf,
            (crc >>  8) & 0xf,
            (crc >>  4) & 0xf,
            (crc      ) & 0xf);
        if (llElementList[index].sharedFrom != -1)
        {
            printf("%3d /", llElementList[index].sharedFrom);
        }
        else
        {
            printf("  - /");
        }
//        if (llElementList[index].nShared == 0)
//        {
            printf(" - ");
//        }
        printf("%s", llElementList[index].textureName);
        if (llElementList[index].nShared != 0)
        {
            for (count = 0; count < llElementList[index].nShared; count++)
            {
                printf("\n                                               -%s", llElementList[llElementList[index].sharedTo[count]].textureName);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : llLayeredImageMeasure
    Description : see if a .LIF file can be loaded and return it's size if possible
    Inputs      : fileName - name of .LIF file to check

    Outputs     : width, height - where you want the dimensions to be returned
                  flags - where to store the flags
                  imageCRC - CRC of read image
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
sdword llLayeredImageMeasure(char* fileName, sdword* width, sdword* height, udword *flags, crc32 *imageCRC)
{
    filehandle lifFileHandle;
    lifheader header;

    if (fileExists(fileName, 0))
    {
        lifFileHandle = fileOpen(fileName, 0);
        fileBlockRead(lifFileHandle, &header, sizeof(lifheader));
        fileClose(lifFileHandle);
        *width = header.width;
        *height = header.height;
        *flags = header.flags;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : llDatasrcToData
    Description : leading datasrc in a string -> data
    Inputs      : datasrc - path beginning with "datasrc"
    Outputs     : input datasrc is modified
    Return      : void
-----------------------------------------------------------------------------*/
void llDatasrcToData(char* datasrc)
{
    char data[1024];

    memset(data, 0, 1024);
    memcpy(data, datasrc, 4);
    strcpy(data+4, datasrc+7);
    strcpy(datasrc, data);
}

/*-----------------------------------------------------------------------------
    Name        : llAddMif
    Description : Adds contents of a mif to lif listing,
    Inputs      : mifFile - name of .MIF file to add
    Outputs     : reads in the file and adds all referenced files to the lising
    Return      : void
----------------------------------------------------------------------------*/
void llAddMif(char *mifFile)
{
    FILE *f;
    char string[256], *fileName;
    char pathBuffer[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char *dirUpper;
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    udword datasrcOffset;
	sdword index, width, height;
    udword flags;
    crc32  imageCRC;

    if ((f = fopen(mifFile, "rt")) == NULL)
    {                                                       //open the .MIF file
        dbgFatalf(DBG_Loc, "Error opening file '%s'.", mifFile);
    }
    while (!feof(f))
    {                                                       //read every line in the file
        if (fgets(string, 256, f) == NULL)
        {
            break;
        }
        if (strstr(string, "+=") == NULL)
        {                                                   //if not a valid line
            continue;                                       //skip line
        }
        strtok(string, " +=\n\t");
        fileName = strtok(NULL, " +=\n\t");
        if (llNElements >= LL_MaxElements)
        {
            dbgFatalf(DBG_Loc, "Exceeded %d textures.", LL_MaxElements);
        }
        printf("\nMeasuring '%s'...", fileName);

        //make a path to the texture in datasrc
        _splitpath( mifFile, drive, dir, fname, ext );      //get the path from the ,MIF file
        if (strchr(fileName, '.') != NULL)
        {                                                   //strip the extension
            *strchr(fileName, '.') = 0;
        }
        _makepath(pathBuffer, drive, dir, fileName, ".PSD");   //make path to the .PSD (source) file
        imageCRC = LL_BadCRC;                               //we won't know this if we measure a .PSD file
        if (psdLayeredImageMeasure(pathBuffer, &width, &height) == FALSE)
        {
            _makepath(pathBuffer, drive, dir, fileName, ".LIF");
            llDatasrcToData(pathBuffer);
            llLayeredImageMeasure(pathBuffer, &width, &height, &flags, &imageCRC);
        }
        //make sure width/height are valid
        if (forceDimensions)
        {
            if (bitNumberSet(width, 16) != 1 || bitNumberSet(width, 16) != 1)
            {
                dbgFatalf(DBG_Loc, "PSD file '%s' not sized to even exponents of 2 (%dx%d).", pathBuffer, width, height);
            }
        }
        if (width < LL_MinWidth || width > LL_MaxWidth || height < LL_MinHeight || height > LL_MaxHeight)
        {
            dbgFatalf(DBG_Loc, "PSD file '%s' is a bad size: (%dx%d).", pathBuffer, width, height);
        }
        llElementList[llNElements].width = width;
        llElementList[llNElements].height = height;
        llElementList[llNElements].flags = flags;           //don't know a CRC without loading the image
        llElementList[llNElements].imageCRC = imageCRC;

        //create a path like "R1\AdvanceSupportFrigate\Rl0\LOD2\front_tip"
        dirUpper = memStringDupe(dir);
        _strupr(dirUpper);
        datasrcOffset = (udword)strstr(dirUpper, "DATASRC\\");
        dbgAssert(datasrcOffset != 0);                      //offet of the 'datasrc\'
        datasrcOffset -= (udword)dirUpper;
        memFree(dirUpper);
        _makepath(pathBuffer, "", dir + datasrcOffset + strlen("DataSrc\\"), fileName, "");
        //see if the named texture has already been referenced
        for (index = 0; index < llNElements; index++)
        {
            if (!_stricmp(llElementList[index].textureName, pathBuffer))
            {                                               //if texture already here
                llElementList[index].width = width;   //update the widthe and height
                llElementList[index].height = height;
                if (imageCRC != LL_BadCRC)
                {                                           //don't update the CRC or flags if you don't know what the hell they are
                    llElementList[index].flags = flags;//don't know a CRC without loading the image
                    llElementList[index].imageCRC = imageCRC;
                }

                goto alreadyHere;                           //don't add a new one
            }
        }
        llElementList[llNElements].textureName = memStringDupe(pathBuffer);
        llNElements++;                                      //else add this data to new member
alreadyHere:;
    }
    fclose(f);
}

/*-----------------------------------------------------------------------------
    Name        : llTexturesSubtract
    Description : Subtracts all textures in mifFile from texture listing
    Inputs      : mifFile - file to read the texture names from
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void llTexturesSubtract(char *mifFile)
{
    FILE *f;
    char string[256], *fileName;
    char pathBuffer[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char *dirUpper;
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    udword datasrcOffset;
	sdword index;

    if ((f = fopen(mifFile, "rt")) == NULL)
    {                                                       //open the .MIF file
        dbgFatalf(DBG_Loc, "Error opening file '%s'.", mifFile);
    }
    while (!feof(f))
    {                                                       //read every line in the file
        if (fgets(string, 256, f) == NULL)
        {
            break;
        }
        if (strstr(string, "+=") == NULL)
        {                                                   //if not "TEXTURES += " line
            if (strchr(string, '\n') != NULL)
            {
                *strchr(string, '\n') = 0;
            }
            strcpy(pathBuffer, string);
        }
        else
        {
            strtok(string, " +=\n\t");
            fileName = strtok(NULL, " +=\n\t");
            if (llNElements >= LL_MaxElements)
            {
                dbgFatalf(DBG_Loc, "Exceeded %d textures.", LL_MaxElements);
            }
//            printf("\nRemoving '%s'...", fileName);

            //create a path like "R1\AdvanceSupportFrigate\Rl0\LOD2\front_tip"
            _splitpath( mifFile, drive, dir, fname, ext );      //get the path from the ,MIF file
            if (strchr(fileName, '.') != NULL)
            {                                                   //strip the extension
                *strchr(fileName, '.') = 0;
            }
            dirUpper = memStringDupe(dir);
            _strupr(dirUpper);
            datasrcOffset = (udword)strstr(dirUpper, "DATASRC\\");
            dbgAssert(datasrcOffset != 0);                      //offet of the 'datasrc\'
            datasrcOffset -= (udword)dirUpper;
            memFree(dirUpper);
            _makepath(pathBuffer, "", dir + datasrcOffset + strlen("DataSrc\\"), fileName, "");
        }
        //see if the named texture is in the texture list
        for (index = 0; index < llNElements; index++)
        {
            if (!_stricmp(llElementList[index].textureName, pathBuffer))
            {                                               //if texture found
                printf("\nRemoving '%s'...", pathBuffer);
                if (index != llNElements - 1)
                {                                           //fill in the hole
                    memcpy(&llElementList[index], &llElementList[index + 1],
                           (llNElements - index - 1) * sizeof(llelement));
                    llNElements--;                          //one less entry
                    break;
                }
            }
        }
    }
    fclose(f);
}

/*-----------------------------------------------------------------------------
    Name        : llSharingUpdate
    Description : Update sharing info for the element list, to be called at
                    list flush time.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void llSharingUpdate(void)
{
    sdword index, j, listLength;
    sdword *newList, *oldList;
    char *name, *name0, *name1;
    crc32 crc;

    //clear out all existing sharing information that currently exists
    for (index = 0; index < llNElements; index++)
    {
        llElementList[index].nShared = 0;
        llElementList[index].sharedTo = NULL;
        llElementList[index].sharedFrom = -1;
    }

    //now go through the element list and find any shared textures
    for (index = 0; index < llNElements; index++)
    {
        if (llElementList[index].sharedFrom != -1)
        {                                                   //if this one already shared
            continue;                                       //don't do anything further
        }
        crc = llElementList[index].imageCRC;
        if (crc == LL_BadCRC)
        {                                               //don't consider anything with a bad CRC
            if (verboseLevel >= 1)
            {
                dbgWarningf(DBG_Loc, "Somewhat unlikely CRC Warning:\n Image #%d ('%s') has an invalid CRC - not shared.",
                    index, llElementList[index].textureName);
            }
            continue;
        }
        listLength = 0;
        newList = NULL;
        for (j = index + 1; j < llNElements; j++)
        {
            if (llElementList[j].imageCRC == LL_BadCRC)
            {                                               //don't consider anything with a bad CRC
/*
                if (verboseLevel >= 1)
                {
                    dbgWarningf(DBG_Loc, "Somewhat unlikely CRC Warning:\n Image #%d ('%s') has an invalid CRC - not shared.",
                        j, llElementList[j].textureName);
                }
*/
                continue;
            }
            if (llElementList[j].imageCRC == crc)
            {                                               //if these CRC's match
                //make sure they have the same name
                name = llElementList[index].textureName;
                while ((name = strchr(name + 1, '\\')) != NULL)
                {                                           //find one name
                    name0 = name + 1;
                }
                name = llElementList[j].textureName;
                while ((name = strchr(name + 1, '\\')) != NULL)
                {                                           //find other name, minus path
                    name1 = name + 1;
                }
                if (strcmp(name0, name1) != 0)
                {
                    if (verboseLevel >= 1)
                    {
                        dbgWarningf(DBG_Loc, "Very unlikely CRC Warning:\n Image #%d ('%s') and image #%d ('%s') both have a CRC of 0x%x but different names.",
                            index, llElementList[index].textureName, j, llElementList[j].textureName, crc);
                    }
                    continue;
                }
                //if everything checks out, make some sharing info, reallocating as needed
                llElementList[j].sharedFrom = index;        //set the share parent
                listLength++;
                oldList = newList;
                newList = memAlloc(listLength * sizeof(sdword), "ShareList", 0);
                if (oldList != NULL)
                {
                    memcpy(newList, oldList, (listLength - 1) * sizeof(sdword));
                    memFree(oldList);
                }
                newList[listLength - 1] = j;
            }
        }
        //sharing info computed, store into element structure
        llElementList[index].nShared = listLength;
        llElementList[index].sharedTo = newList;
    }
}

/*-----------------------------------------------------------------------------
    Name        : llSortCompare
    Description : qsort callback for alphabetically sorting the list of elements
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int llSortCompare(const void *p0, const void *p1)
{
    return(_stricmp(((llelement *)p0)->textureName, ((llelement *)p1)->textureName));
}

/*-----------------------------------------------------------------------------
    Name        : llFlushList
    Description : Sort and save out the current element list to llFileName.
    Inputs      : void
    Outputs     :
    Return      : void
    Note        : All sharing is computed here.
----------------------------------------------------------------------------*/
void llFlushList(void)
{
    FILE *f;
    llfileheader header;
    sdword stringLength, index, stringOffset, sharingLength, sharingOffset, badOffset = 0xffffffff;
    //sort the list
    qsort(llElementList, llNElements, sizeof(llelement), llSortCompare);
    //update sharing info
    llSharingUpdate();

    //open the file
    if ((f = fopen(llFileName, "wb")) == NULL)
    {
        dbgFatalf(DBG_Loc, "Cannot open '%s' for writing", llFileName);
    }
    //count size of all strings and sharing lists
    for (index = stringLength = sharingLength = 0; index < llNElements; index++)
    {                                                       //for every element
        stringLength += strlen(llElementList[index].textureName) + 1;//length of name plus NULL terminator
        sharingLength += llElementList[index].nShared * sizeof(sdword);
    }
    //create and write out the header
    strcpy(header.ident, LL_FileIdentifier);
    header.version = LL_FileVersion;
    header.nElements = llNElements;
    header.stringLength = stringLength;
    header.sharingLength = sharingLength;
    header.totalLength = stringLength + sharingLength + llNElements * sizeof(llelement);
    fwrite(&header, 1, sizeof(llfileheader), f);

    //now go through list and write out the width, height, flags and string offset
    for (index = stringOffset = sharingOffset = 0; index < llNElements; index++)
    {
        fwrite(&stringOffset, 1, sizeof(sdword), f);        //write string offset
        fwrite(&llElementList[index].width, 2, sizeof(sdword), f);//write width and height
        fwrite(&llElementList[index].flags, 1, sizeof(udword), f);//write width and height
        fwrite(&llElementList[index].imageCRC, 1, sizeof(udword), f);//write crc
//        fwrite(&llElementList[index].nSizes, 1, sizeof(udword), f);//write crc
//        fwrite(llElementList[index].imageSize, LLC_NumberImages, sizeof(udword), f);//write crc
        fwrite(&llElementList[index].nShared, 1, sizeof(sdword), f);//write sharing stuff
        if (llElementList[index].sharedTo != NULL)
        {
            fwrite(&sharingOffset, 1, sizeof(sdword *), f);
            sharingOffset += sizeof(sdword) * llElementList[index].nShared;
        }
        else
        {
            fwrite(&badOffset, 1, sizeof(sdword *), f);
        }
        fwrite(&llElementList[index].sharedFrom, 1, sizeof(sdword), f);
        stringOffset += strlen(llElementList[index].textureName) + 1;

    }
    //now write the strings, along with NULL terminators
    for (index = stringOffset = 0; index < llNElements; index++)
    {
        fwrite(llElementList[index].textureName, 1, strlen(llElementList[index].textureName) + 1, f);
    }
    //now write out all the sharing lists
    for (index = stringOffset = 0; index < llNElements; index++)
    {
        if (llElementList[index].nShared)
        {                                                   //if this one has a sharing list
            fwrite(llElementList[index].sharedTo, sizeof(sdword), llElementList[index].nShared, f);
        }
    }
    //close the file
    fclose(f);
}

/*-----------------------------------------------------------------------------
    Name        : stristr
    Description : C RTL-like function to do a case insensitive search for substrings
    Inputs      : string, substring - just like strstr
    Outputs     :
    Return      : just like strstr
    Note        : Allocates then frees memory so it may not be blindingly fast
----------------------------------------------------------------------------*/
char *stristr(char *string, char *subString)
{
    char *upperString, *upperSubString, *found;

    upperString = memStringDupe(string);
    upperSubString = memStringDupe(subString);
    _strupr(upperString);
    _strupr(upperSubString);

    found = strstr(upperString, upperSubString);
    memFree(upperSubString);
    memFree(upperString);
    if (found != NULL)
    {
        return(found - upperString + string);
    }
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : trLayerNameMatch
    Description : Searches a list of string for a matching layer name
    Inputs      : name - name of layer
                  nameList - list of names to compare against
    Outputs     : converts name to upper case before comparisson
    Return      : TRUE if found, FALSE if not
----------------------------------------------------------------------------*/
sdword trLayerNameMatch(char *name, char **nameList)
{
    sdword length;
    _strupr(name);
    while (*nameList != NULL)
    {
        length = strlen(*nameList);
        if (!memcmp(name, *nameList, length))
        {
            return(TRUE);
        }
        nameList++;
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : llImageSubtractIntensity
    Description : Subtracts intensity of image1 from image0, claming to [0..255]
    Inputs      : difference - intensity difference of the two images
                  image0, image1 - images to difference
                  length - length, in pixels of the two images
    Outputs     : fills in difference array.
    Return      :
    Note        : Uses NTSC intensity calculation.
----------------------------------------------------------------------------*/
void llImageSubtractIntensity(ubyte *difference, color *image0, color *image1, sdword length)
{
    sdword intensity0, intensity1;

    for (; length > 0; length--, image0++, image1++, difference++)
    {
        intensity0 = (sdword)colIntensityNTSC(*image0);
        intensity1 = (sdword)colIntensityNTSC(*image1);
        *difference = (ubyte)(max(intensity0 - intensity1, 0));
    }
}

/*-----------------------------------------------------------------------------
    Name        : llImageHistogramMonkey
    Description : Adjust the histogram of an image by stretching the
                    upper half of the histogram and compressing the lower half.
    Inputs      : buffer - buffer to modify
                  length - length of buffer
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
sdword llHistoMid = 128;
sdword llHistoMidShift = 62;
void llImageHistogramMonkey(ubyte *buffer, sdword length)
{
    sdword index, newMidPoint = llHistoMid + llHistoMidShift;
    ubyte histoLUT[256];

    //compress lower half of histogram
    for (index = 0; index < newMidPoint; index++)
    {
        histoLUT[index] = (ubyte)(index * llHistoMid / newMidPoint);
    }
    //expand upper half of the histogram
    for (; index < 256; index++)
    {
        histoLUT[index] = (ubyte)((index - newMidPoint) * (256 - llHistoMid) / (256 - newMidPoint) + llHistoMid);
    }

    //now map the image using this new histogram LUT
    for (; length > 0; length--, buffer++)
    {
        *buffer = histoLUT[*buffer];
    }
}

/*-----------------------------------------------------------------------------
    Name        : llImageZeroIfOtherImageNonZero
    Description : For each pixel in two images, will zero out dest[n] if compare[n] != 0
    Inputs      : compare - image to be compared against
                : length - length of the image
    Outputs     : dest - image to be comparatively be zeroed
    Return      :
----------------------------------------------------------------------------*/
void llImageZeroIfOtherImageNonZero(ubyte *dest, ubyte *compare, sdword length)
{
    while (length > 0)
    {
        if (*compare != 0)
        {
            *dest = 0;
        }
        dest++;
        compare++;
        length--;
    }
}

/*-----------------------------------------------------------------------------
    Name        : llImageComposite
    Description : Create colored composite RGB image from layered image
    Inputs      : image - pointer to layered image
                  newTeamColor0,1 - out parameter for the team color effect buffer.
                    If NULL, no team color buffer is used.
                  flagsDest - where to store flags
    Outputs     : allocates an RGB buffer and decompresses image into it.
                  flagDest - has bits set based upon attributes of the layers (such as alpha)
                  Layers also deleted (unused ones)
    Return      :
----------------------------------------------------------------------------*/
color *llImageComposite(layerimage *image, ubyte **newTeamColor0, ubyte **newTeamColor1, udword *flagsDest)
{
    sdword index, outerLoop;
    color *colorData, *tempBuffer;
    ubyte *teamColor0, *teamColor1;
    color teamColor;
    bool col2Below = FALSE;

    *flagsDest = TRF_Paletted;                              //start with a paletted texture
//    *flagsDest = 0;
    if (forceAlpha)
    {
        bitSet(*flagsDest, TRF_Alpha);
    }
    colorData = liBlendBufferPrepare(image, newTeamColor0, newTeamColor1);
    if (newTeamColor0 != NULL)
    {
        teamColor0 = *newTeamColor0;
        teamColor1 = *newTeamColor1;
    }
    else
    {
        teamColor1 = NULL;
        teamColor0 = NULL;
    }
    for (outerLoop = 0; outerLoop < 5; outerLoop++)
    {
        memClearDword(colorData, colRGBA(0, 0, 0, 255),
                       image->width * image->height);       //clear to opaque black
        for (index = 0; index < image->nLayers; index++)
        {
            if (bitTest(image->layers[index].flags, LFF_Deleted))
            {                                               //if layer already deleted
                continue;                                   //ignore
            }
            if (image->layers[index].name == NULL)
            {
                dbgWarningf(DBG_Loc, "Layer %d in '%s' is unnamed - skipping.", index, image->fileName);
                continue;
            }
            if (!trLayerNameMatch(image->layers[index].name, trAllLayerNames))
            {                                               //if invalid layer name
                liLayerDelete(&image->layers[index]);       //delete it
                continue;                                   //and ignore
            }
            if (image->layers[index].flags & LFF_Channeled)
            {                                               //if layer not decompressed
                liLayerDecompress(&image->layers[index]);   //decompress layer
                image->layers[index].average = liLayerColorAverage(&image->layers[index]);
            }
            if (trLayerNameMatch(image->layers[index].name, trAlphaNames))
            {                                               //if alpha, copy red to alpha
                bitSet(*flagsDest, TRF_Alpha);
                bitClear(*flagsDest, TRF_Paletted);
                liInsertChannelCompose(colorData, index, image, 0, 3);
                continue;                                   //don't blend alpha channels normally
            }
            if (trLayerNameMatch(image->layers[index].name, trBaseColorNames))
            {                                               //colorize base color layer
                if (outerLoop == 0 && bitTest(image->layers[index].flags, LFF_TeamColor1))
                {
                    printf("\nWarning: image '%s' has col1 atop col2.", image->fileName);
                    col2Below = TRUE;
                }
                if (outerLoop == 0)                         //first pass: base color white
                {
                    teamColor = colRGB(0xff, 0xff, 0xff);
                }
                else if (outerLoop == 1)                    //2nd pass: base color black
                {
                    teamColor = colRGB(0, 0, 0);
                }
                else
                {                                           //else base color normal
                    teamColor = image->layers[index].average;
                    teamColor = colIntensityNTSC(teamColor);
                    teamColor = colRGB(teamColor, teamColor, teamColor);
                }
                liLayerColorSolid(&image->layers[index], teamColor, TR_ColoringThreshold);
                bitSet(image->layers[index].flags, LFF_TeamColor0);
                bitSet(*flagsDest, TRF_TeamColor0);
            }
            if (trLayerNameMatch(image->layers[index].name, trDetailColorNames))
            {                                               //colorize detail layer
                if (outerLoop == 2)                         //third pass: stripe color white
                {
                    teamColor = colRGB(0xff, 0xff, 0xff);
                }
                else if (outerLoop == 3)                    //fourth pass: base color black
                {
                    teamColor = colRGB(0, 0, 0);
                }
                if (teamColor == colRGBA(0,0,0,0))
                {                                           //else stripe color normal
                    teamColor = image->layers[index].average;
                    teamColor = colIntensityNTSC(teamColor);
                    teamColor = colRGB(teamColor, teamColor, teamColor);
                }
                liLayerColorSolid(&image->layers[index], teamColor, TR_ColoringThreshold);
                bitSet(image->layers[index].flags, LFF_TeamColor1);
                bitSet(*flagsDest, TRF_TeamColor1);
            }
            liLayerBlendTo(colorData, index, image);//composite this image down
        }
        switch (outerLoop)
        {
            case 0:                                         //first pass: base color white
                tempBuffer = memAlloc(image->width * image->height * sizeof(color), "TempABCDBuffer", 0);
                memcpy(tempBuffer, colorData, image->width * image->height * sizeof(color));
                break;
            case 1:                                         //second pass: base color black
                llImageSubtractIntensity(teamColor0, tempBuffer, colorData, image->width * image->height);
                llImageHistogramMonkey(teamColor0, image->width * image->height);
                break;
            case 2:                                         //third pass: stripe color white
                memcpy(tempBuffer, colorData, image->width * image->height * sizeof(color));
                break;
            case 3:                                         //fourth pass: stripe color black
                llImageSubtractIntensity(teamColor1, tempBuffer, colorData, image->width * image->height);
                llImageHistogramMonkey(teamColor0, image->width * image->height);
                memFree(tempBuffer);
                //zero out the color buffer that lies below the other color buffer
                if (bitTest(*flagsDest, TRF_TeamColor0) && bitTest(*flagsDest, TRF_TeamColor1))
                {                                           //if there was both team colors
                    if (col2Below)
                    {                                       //if col1 on top of col 2
                        llImageZeroIfOtherImageNonZero(teamColor1, teamColor0, image->width * image->height);
                    }
                    else
                    {
                        llImageZeroIfOtherImageNonZero(teamColor0, teamColor1, image->width * image->height);
                    }
                }
                break;
        }
    }
    return(colorData);
}

/*-----------------------------------------------------------------------------
    Name        : llLIFSave
    Description : Save the specified image to disk
    Inputs      : info - structure containing all the data needed to save to disk
                  palette - palette information shared with other files in this group
                  teamPalette0,1 - palettes of team color effect per pixel
    Outputs     :
    Return      : void
    Notes       : will save the image in RGB format if needed
----------------------------------------------------------------------------*/
void llLIFSave(texentry *info, color *palette, ubyte *teamPalette0, ubyte *teamPalette1)
{
    lifheader header;
    FILE *f;

    //opent the .LiF file for reading
    if ((f = fopen(info->destPath, "wb")) == NULL)
    {
        dbgFatalf(DBG_Loc, "Error creating '%s'.", info->destPath);
    }
    //create the file header
    strcpy(header.ident, LIF_FileIdentifier);
    header.version = LIF_FileVersion;
    header.flags = info->flags;                             //flags for plugging into texreg flags
    if (!forceDimensions)
    {
        bitSet(header.flags, TRF_IgnoreDimensions);
    }
    header.width = info->image->width;
    header.height = info->image->height;
    header.data = sizeof(header);                           //data comes right after the header
    if (bitTest(header.flags, TRF_Alpha))
    {                                                       //if it's a truecolor image
        header.palette = 0;
        if (bitTest(header.flags, TRF_TeamColor0))
        {
            header.teamEffect0 = header.data + header.width * header.height * sizeof(color);
        }
        else
        {
            header.teamEffect0 = header.data + header.width * header.height * (sizeof(color) - 1);
        }

        if (bitTest(header.flags, TRF_TeamColor1))
        {
            header.teamEffect1 = header.teamEffect0 + header.width * header.height;
        }
        else
        {
            header.teamEffect1 = 0;
        }
    }
    else
    {
        header.palette = header.data + header.width * header.height;//then there's the palette
        header.teamEffect0 = header.palette + sizeof(color) * 256;//and finally the team effect palettes
        header.teamEffect1 = header.teamEffect0 + 256;
    }
    header.paletteCRC = crc32Compute((ubyte *)palette, 256 * sizeof(color)) +
        crc32Compute(teamPalette0, 256) + crc32Compute(teamPalette1, 256);
    //now write the file out
    fwrite(&header, 1, sizeof(header), f);                  //write the header
    if (bitTest(header.flags, TRF_Alpha))
    {                                                       //if RGB image
        fwrite(info->composited, header.width * header.height, sizeof(color), f);
        if (bitTest(header.flags, TRF_TeamColor0))
        {
            fwrite(info->teamEffect0, header.width, header.height, f);//write the team effect images
        }
        if (bitTest(header.flags, TRF_TeamColor1))
        {
            fwrite(info->teamEffect1, header.width, header.height, f);
        }
    }
    else
    {
        fwrite(info->quantized, header.width, header.height, f);//write the image
        fwrite(palette, sizeof(color), 256, f);             //write the color palette
        fwrite(teamPalette0, 1, 256, f);                    //write the team effect palettes
        fwrite(teamPalette1, 1, 256, f);
    }
    //done
    fclose(f);
}

/*-----------------------------------------------------------------------------
    Name        : llLIFSaveCompressed
    Description : Save the specified image to disk
    Inputs      : info - structure containing all the data needed to save to disk
    Outputs     :
    Return      : void
    Notes       : Saves all the available sizes to the sam file.
----------------------------------------------------------------------------*/
/*
#define writeValue(v)   value = (v); fwrite(&value, 1, sizeof(value), f);
void llLIFSaveCompressed(texentry *info)
{
    lifheader header;
    FILE *f;
    sdword index, imageHeaderSize;
    udword imageOffset, value;

    //open the .LiF file for reading
    if ((f = fopen(info->destPath, "wb")) == NULL)
    {
        dbgFatalf(DBG_Loc, "Error creating '%s'.", info->destPath);
    }

    //compute the size of header required for each image
    imageHeaderSize = sizeof(udword) * 3;                   //every image has r,g,b members
    if (bitTest(info->flags, TRF_Alpha))
    {
        imageHeaderSize += sizeof(udword);                  //add an alpha channel pointer
    }
    if (bitTest(info->flags, TRF_TeamColor0))
    {
        imageHeaderSize += sizeof(udword);                  //add a team0 pointer
    }
    if (bitTest(info->flags, TRF_TeamColor1))
    {
        imageHeaderSize += sizeof(udword);                  //add a team1 pointer
    }
    //create the file header
    strcpy(header.ident, LIF_FileIdentifier);
    header.version = LIF_FileVersion;
    header.flags = info->flags;                             //flags for plugging into texreg flags
    header.width = info->image->width;
    header.height = info->image->height;
//    header.data = sizeof(header);                           //data comes right after the header
    header.nSizes = info->compressed->nImages;              //get proper number of images
    for (index = imageOffset = 0; index < LLC_NumberImages; index++)
    {
        if (index < header.nSizes)
        {
            header.image[index] = imageOffset;
            imageOffset += info->compressed->image[index].imageSize + imageHeaderSize;
        }
        else
        {
            header.image[index] = 0;
        }
    }

    //now write the file out
    fwrite(&header, 1, sizeof(header), f);                  //write the header

    for (index = 0; index < header.nSizes; index++)
    {
        imageOffset = 0;
        //write out the image header
        writeValue(imageHeaderSize + imageOffset);
        imageOffset += info->compressed->image[index].redSize;
        writeValue(imageHeaderSize + imageOffset);
        imageOffset += info->compressed->image[index].greenSize;
        writeValue(imageHeaderSize + imageOffset);
        imageOffset += info->compressed->image[index].blueSize;
        if (bitTest(info->flags, TRF_Alpha))
        {
            writeValue(imageHeaderSize + imageOffset);
            imageOffset += info->compressed->image[index].alphaSize;
        }
        if (bitTest(info->flags, TRF_TeamColor0))
        {
            writeValue(imageHeaderSize + imageOffset);
            imageOffset += info->compressed->image[index].team0Size;
        }
        if (bitTest(info->flags, TRF_TeamColor1))
        {
            writeValue(imageHeaderSize + imageOffset);
            imageOffset += info->compressed->image[index].team1Size;
        }
        //now write out the actual images
        fwrite(info->compressed->image[index].red, 1, info->compressed->image[index].redSize, f);
        fwrite(info->compressed->image[index].green, 1, info->compressed->image[index].greenSize, f);
        fwrite(info->compressed->image[index].blue, 1, info->compressed->image[index].blueSize, f);
        if (bitTest(info->flags, TRF_Alpha))
        {
            fwrite(info->compressed->image[index].alpha, 1, info->compressed->image[index].alphaSize, f);
        }
        if (bitTest(info->flags, TRF_TeamColor0))
        {
            fwrite(info->compressed->image[index].team0, 1, info->compressed->image[index].team0Size, f);
        }
        if (bitTest(info->flags, TRF_TeamColor1))
        {
            fwrite(info->compressed->image[index].team1, 1, info->compressed->image[index].team1Size, f);
        }
    }
    //done
    fclose(f);
}
*/

/*-----------------------------------------------------------------------------
    Name        : llTeamEffectsCombine
    Description : Combine the two specified team effect buffers into 1
    Inputs      : buffer0, buffer1 - buffers to blend together
                  size - size of the buffers
    Outputs     :
    Return      : Pointer to new buffer, in buffers not modified.
----------------------------------------------------------------------------*/
ubyte *llTeamEffectsCombine(ubyte *buffer0, ubyte *buffer1, udword size)
{
    ubyte *newBuffer, *pDest;

    newBuffer = memAlloc(size, "CombinedTeamEffectBuffers", 0);
    pDest = newBuffer;
    for (; size > 0; size--, buffer0++, buffer1++, pDest++)
    {
        //7 bits team effect, high bit tells which one it is: 0 = base, 1 = stripe
        //what if the stripe color is below th base color?  This check will not work too well...
        if (*buffer1 != 0)
        {
            *pDest = (*buffer1 / 2) | 0x80;
        }
        else
        {
            *pDest = (*buffer0 / 2);
        }
    }
    return(newBuffer);
}

/*-----------------------------------------------------------------------------
    Name        : llTeamColorize
    Description : Colorize a color using the same algorithm used in the game
    Inputs      : source - starting RGB color
                  effect0,1 - team color effects
    Outputs     :
    Return      : team colorized color
    Notes       : effect0,1 must be valid team effect values
                  Alpha is ignored and returned as opaque.
----------------------------------------------------------------------------*/
color llTeamColorize(color source, udword effect0, udword effect1, color teamColor0, color teamColor1)
{
    udword redDest, greenDest, blueDest;
    udword redSource, greenSource, blueSource;
    udword oneMinusEffect0 = 255 - effect0, oneMinusEffect1 = 255 - effect1;
    udword teamRed0, teamRed1, teamGreen0, teamGreen1, teamBlue0, teamBlue1;

    redSource = colRed(source);
    greenSource = colGreen(source);
    blueSource = colBlue(source);
    teamRed0 = colRed(teamColor0);
    teamGreen0 = colGreen(teamColor0);
    teamBlue0 = colBlue(teamColor0);
    teamRed1 = colRed(teamColor1);
    teamGreen1 = colGreen(teamColor1);
    teamBlue1 = colBlue(teamColor1);

    redDest = redSource * oneMinusEffect0 + teamRed0 * effect0;
    redDest = ((redDest * oneMinusEffect1) >> 8) + teamRed1 * effect1;
    greenDest = greenSource * oneMinusEffect0 + teamGreen0 * effect0;
    greenDest = ((greenDest * oneMinusEffect1) >> 8) + teamGreen1 * effect1;
    blueDest = blueSource * oneMinusEffect0 + teamBlue0 * effect0;
    blueDest = ((blueDest * oneMinusEffect1) >> 8) + teamBlue1 * effect1;

    return(colRGB((redDest >> 8), (greenDest >> 8), (blueDest >> 8)));
}

/*-----------------------------------------------------------------------------
    Name        : llColorDiffAbs
    Description : Return the difference between two colors.
    Inputs      : color0, color1 - colors to difference
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
udword llColorDiffAbs(color color0, color color1)
{
    return(abs((sdword)colRed(color0) - (sdword)colRed(color1)) +
           abs((sdword)colGreen(color0) - (sdword)colGreen(color1)) +
           abs((sdword)colBlue(color0) - (sdword)colBlue(color1)));
}

/*-----------------------------------------------------------------------------
    Name        : llDistanceHSV
    Description : Return the difference between two colors in HSV space.
    Inputs      :
    Outputs     :
    Return      : 0..3
----------------------------------------------------------------------------*/
real32 llDistanceHSV(color color0, color color1)
{
    real32 red0, green0, blue0, red1, green1, blue1;

    red0 = colUbyteToReal(colRed(color0));
    green0 = colUbyteToReal(colGreen(color0));
    blue0 = colUbyteToReal(colBlue(color0));
    red1 = colUbyteToReal(colRed(color1));
    green1 = colUbyteToReal(colGreen(color1));
    blue1 = colUbyteToReal(colBlue(color1));

    return(abs(red1 - red0) + abs(green1 - green0) + abs(blue1 - blue0));
}

/*-----------------------------------------------------------------------------
    Name        : llProgressStart
    Description : Start a progress bar.
    Inputs      : caption - caption to draw before the status bar
                  finished - character to draw for finished blocks.
                  unfinished - character to draw for unfinished blocks
                  length - number of characters to draw
                  calls - number of times we can expect to be called
    Outputs     :
    Return      : void
    Note        : Caption pointer remembered just by pointer.
----------------------------------------------------------------------------*/
char llProgressAnim[] = "-\\|/";
sdword llAnimIndex;
sdword llTimesCalled, llCallsExpected;
sdword llProgressLength;
char llUnfinished, llFinished;
char *llProgressCaption;
void llProgressStart(char *caption, char finished, char unfinished, sdword length, sdword calls)
{
    llProgressCaption = caption;
    llProgressLength = min(length - 2, calls);
    llCallsExpected = calls;
    llTimesCalled = llAnimIndex = 0;
    llFinished = finished;
    llUnfinished = unfinished;
    printf("\r%s[", llProgressCaption);
    for (length = llProgressLength; length > 0; length--)
    {
        printf("%c", llUnfinished);
    }
    printf("]");
}

/*-----------------------------------------------------------------------------
    Name        : llProgressUpdate
    Description : Update a progress bar.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void llProgressUpdate(void)
{
    sdword index, nChars;

    llTimesCalled++;                                        //update number of times called
    //update the spinny animating doodad
    llAnimIndex++;
    if (llProgressAnim[llAnimIndex] == 0)
    {
        llAnimIndex = 0;
    }

    if (llTimesCalled > llCallsExpected)
    {
        return;
    }
    //see how many characters to draw
    nChars = llTimesCalled * llProgressLength / llCallsExpected;

    //draw the status bar
    printf("\r%s[", llProgressCaption);
    for (index = 0; index < nChars; index++)
    {
        printf("%c", llFinished);
    }
    //draw the spinny doodad if applicable
    if (llTimesCalled * llProgressLength % llCallsExpected > 0 && nChars < llProgressLength)
    {                                                       //if there's a remainder
        printf("%c", llProgressAnim[llAnimIndex]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : llCloseIndexCompare
    Description : qsort comparison callback for sorting similar colors by difference
                    from a reference color.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
color *globalPalette = NULL;
sdword globalPaletteLength = 0;
color compareColor;
sdword llCloseIndexCompare(const void *p0, const void *p1)
{
    ubyte index0 = *((ubyte *)p0);
    ubyte index1 = *((ubyte *)p1);
    sdword error0, error1;

    error0 = (sdword)llColorDiffAbs(compareColor, globalPalette[index0]);
    error1 = (sdword)llColorDiffAbs(compareColor, globalPalette[index1]);
    return(error0 - error1);
}

/*-----------------------------------------------------------------------------
    Name        : llTeamEffectPalettesCompute
    Description : Compute palettes of team effect based upon the images in list
                    and the specified palette.
    Inputs      : list - list of texentry structures
                  listLength - length of the list of textures
                  palette - quantized palette
                  nColors - number of colors in the palette
                  teamColor0, teamColor1 - destination for the team effect palettes.
    Outputs     : fills in the teamColor0,1 buffers
    Return      : void
----------------------------------------------------------------------------*/
typedef struct
{

    uword index0[256];
    uword index1[256];
    udword length0;
    udword length1;
    udword error, bestError, bestIndex;
    udword indexIndex0, indexIndex1;
    udword nCloseIndices;
    ubyte closeIndex[256];
}
paleffect;
void llTeamEffectPalettesCompute(texentry *list, sdword listLength, color *palette, sdword nColors, ubyte *teamColor0, ubyte *teamColor1)
{
    sdword index, colorIndex, j;
    paleffect *effect;
    ubyte *pIndexed, *pTeamColor0, *pTeamColor1;
    color *pRGB;
	udword maxLength, count, length, index0, index1, lengthIndex, error;
    color coloredRGB, coloredPalette;
    errorinfo *pError;

    globalPalette = palette;
    globalPaletteLength = nColors;

    dbgAssert(nColors > 0 && nColors <= 256);
    //allocate a list of structures for team effect
    effect = memAlloc(sizeof(paleffect) * nColors, "EffectPaletteInfo", 0);

    //initialize all the structures
    for (index = 0; index < nColors; index++)
    {                                                       //for each color in the palette
        effect[index].length0 = effect[index].length1 = 0;  //no team effect for this index
        effect[index].bestError = UDWORD_Max;               //'infinite' error
        memset(effect[index].index0, 0xff, sizeof(sword) * 256);//clear the team effect arrays
        memset(effect[index].index1, 0xff, sizeof(sword) * 256);
        //find all other colors that are pretty close to this color
    }
    //find all instances of this index's team effect
    for (index = maxLength = 0; index < listLength; index++)
    {                                                       //for each image
        length = list[index].image->width * list[index].image->height;
        pIndexed = list[index].quantized;
        pError = list[index].error = memAlloc(length * sizeof(errorinfo), "ErrorInfo", 0);
        for (count = 0; count < length; count++, pIndexed++, pError++)
        {                                                   //for each pixel
            dbgAssert((sdword)(*pIndexed) < nColors);
            pError->bestError = 0xffffffff;                 //init to 'infinite' error
            //set teamColor0 reference
            if (effect[*pIndexed].index0[list[index].teamEffect0[count]] == 0xffff)
            {                                               //if this one not yet found
                dbgAssert(effect[*pIndexed].length0 < 255);
                effect[*pIndexed].index0[list[index].teamEffect0[count]] = list[index].teamEffect0[count];
                effect[*pIndexed].length0++;
                maxLength = max(maxLength, effect[*pIndexed].length0 * effect[*pIndexed].length1);
            }
            //set teamColor1 reference
            if (effect[*pIndexed].index1[list[index].teamEffect1[count]] == 0xffff)
            {                                               //if this one not yet found
                dbgAssert(effect[*pIndexed].length1 < 255);
                effect[*pIndexed].index1[list[index].teamEffect1[count]] = list[index].teamEffect1[count];
                effect[*pIndexed].length1++;
                maxLength = max(maxLength, effect[*pIndexed].length0 * effect[*pIndexed].length1);
            }
        }
    }
    dbgAssert(maxLength < 256 * 256 * 256);
    //squash all the team color entries of each color down to the bottom of the list
    for (index = 0; index < nColors; index++)
    {
        for (count = length = 0; count < effect[index].length0; count++, length++)
        {
            while (effect[index].index0[length] == 0xffff)
            {
                length++;

                dbgAssert(length < 256);
            }
            effect[index].index0[count] = effect[index].index0[length];
        }
        for (count = length = 0; count < effect[index].length1; count++, length++)
        {
            while (effect[index].index1[length] == 0xffff)
            {
                length++;

                dbgAssert(length < 256);
            }
            effect[index].index1[count] = effect[index].index1[length];
        }
        effect[index].indexIndex0 = effect[index].indexIndex1 = 0;
        effect[index].error = 0;                            //start cumulative error at 0
    }
    //now let's find the minimum error team effect palettes
    //We will use the assumption that the algorithm for team colorizing the
    //RGB is correct, as has been demonstrated in the game.  Using this
    //assumption, we will colorize the RGB buffer, and use all possible
    //paletted colorization and get the error between them.
    llProgressStart("Optimizing ", '#', '-', 79 - strlen("Optimizing "), maxLength);
    for (lengthIndex = 0; lengthIndex < maxLength; lengthIndex++)
    {
        llProgressUpdate();
        for (index = 0; index < listLength; index++)
        {                                                   //for each image
            length = list[index].image->width * list[index].image->height;
            pIndexed = list[index].quantized;
            pRGB = list[index].composited;
            pTeamColor0 = list[index].teamEffect0;
            pTeamColor1 = list[index].teamEffect1;
            pError = list[index].error;
            for (count = 0; count < length; count++, pIndexed++, pRGB++, pTeamColor0++, pTeamColor1++, pError++)
            {                                               //for each pixel in the image
                dbgAssert(*pIndexed < nColors);
                if ((udword)lengthIndex < effect[*pIndexed].length0 * effect[*pIndexed].length1)
                {                                           //if we can colorize this pixel
                    index0 = lengthIndex % effect[*pIndexed].length0;
                    index1 = lengthIndex / effect[*pIndexed].length0;
                    dbgAssert(index0 < effect[*pIndexed].length0);
                    dbgAssert(index1 < effect[*pIndexed].length1);
                    dbgAssert(effect[*pIndexed].index0[index0] != 0xffff);
                    dbgAssert(effect[*pIndexed].index1[index1] != 0xffff);
                    error = 0;
                    for (colorIndex = 0; colorIndex < LL_NumberTeamColors; colorIndex++)
                    {                                       //for all of the test colors
                        coloredRGB = llTeamColorize(*pRGB, *pTeamColor0, *pTeamColor1, llTeamColors0[colorIndex], llTeamColors1[colorIndex]);
                        coloredPalette = llTeamColorize(*pRGB, effect[*pIndexed].index0[index0], effect[*pIndexed].index1[index1], llTeamColors0[colorIndex], llTeamColors1[colorIndex]);
                        error += llColorDiffAbs(coloredRGB, coloredPalette);// * effect[*pIndexed].multiplier / LL_MultiplierSquish;
                    }

                    if (error < pError->bestError)
                    {                                       //see if this error is all good
                        pError->bestError = error;
                        pError->bestIndex = (j << 24) | lengthIndex;
                    }
                    effect[*pIndexed].error += error;
                }
            }
        }

        //now loop through all colors and see if we've got a really low error on any indices
        for (index = 0; index < nColors; index++)
        {                                                   //for all color indices
            if (lengthIndex < effect[index].length0 * effect[index].length1)
            {                                               //if error computed this time through
                if (effect[index].error < effect[index].bestError)
                {                                           //if lowest error thus far
                    effect[index].bestError = effect[index].error;
                    effect[index].bestIndex = lengthIndex;  //remember best error
                }
            }
            effect[index].error = 0;                        //reset error counter for next pass
        }
    }

    //Now we've got a best error count for all the color entries.
    //Let's make the final team effect palettes.
    for (index = 0; index < nColors; index++)
    {
        if (effect[index].length0 == 0)
        {
            teamColor0[index] = teamColor1[index] = 0;
        }
        else
        {
            dbgAssert(effect[index].bestIndex < maxLength);
            index0 = effect[index].bestIndex % effect[index].length0;
            index1 = effect[index].bestIndex / effect[index].length0;
            dbgAssert(index0 < 256);
            dbgAssert(index1 < 256);
            dbgAssert(effect[index].index0[index0] != 0xffff);
            dbgAssert(effect[index].index1[index1] != 0xffff);
            teamColor0[index] = (ubyte)(effect[index].index0[index0]);
            teamColor1[index] = (ubyte)(effect[index].index1[index1]);
        }
    }

    printf("\n");
    memFree(effect);
}

/*-----------------------------------------------------------------------------
    Name        : llImageBandFilterIndexed
    Description : Replace a band of levels in an image.
    Inputs      : image - image to filter
                  length - length of image
                  low, high - range of band (inclusive/exclusive)
                  replacement - value to store for filtered values
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void llImageBandFilterIndexed(ubyte *image, sdword length, ubyte low, ubyte high, ubyte replacement)
{
    for (; length > 0; length--, image++)
    {
        if (*image >= low && *image < high)
        {
            *image = replacement;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : llBitmapCopy
    Description : Unoptimized bitblt routine for 32-bpp images
    Inputs      : dest - destination buffer
                  destX,Y - loaction in destination buffer to copy to
                  destWidth, destHeight - size of destination buffer
                  source - source buffer
                  sourceWidth, sourceHeight - size of source buffer
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void llBitmapCopy(color *dest, sdword destX, sdword destY, sdword destWidth, sdword destHeight, color *source, sdword sourceWidth, sdword sourceHeight)
{
    color *destP;
	sdword y;

    //check if it'll go off the destination bitmap
    if (destX < 0)
    {
        return;
    }
    if (destY < 0)
    {
        return;
    }
    if (destX + sourceWidth > destWidth)
    {
        return;
    }
    if (destY + sourceHeight > destHeight)
    {
        return;
    }
    destP = dest + (destY) * destWidth + destX;
    for (y = 0; y < sourceHeight; y++)
    {
        memcpy(destP, source, sourceWidth * sizeof(color));
        destP += destWidth;
		source += sourceWidth;
    }
}

/*-----------------------------------------------------------------------------
    Name        : llPlaneDecompressRLE
    Description : Decompress a plane into a buffer
    Inputs      : dest - where to decompress to
                  compressed - compressed buffer
                  width, height - size of image.
                  cSize - compressed size of image.
    Outputs     : fills up dest
    Return      : void
----------------------------------------------------------------------------*/
/*
#define byteRead()  *pSource; pSource++
void llPlaneDecompressRLE(ubyte *dest, ubyte *compressed, sdword width, sdword height, sdword cSize, udword addAmount, udword shiftBits)
{
    sdword count, index;
    ubyte opcode, *pSource = compressed, *pixel = dest, code;

    for (count = width * height; count > 0;)
    {
        opcode = byteRead();                                //get byte from stream
        if ((opcode & 0xc0) == 0xc0)                        //see if run of bytes
        {
            opcode &= 0x3f;
            code = byteRead();                             //it's a run
            for (index = 0; index < opcode && count > 0; index++)
            {
                *pixel = code << shiftBits;
                pixel += addAmount;
				count--;
            }
        }
        else
        {                                                   //else it's just a pixel
            *pixel = opcode << shiftBits;
            pixel += addAmount;
			count--;
        }
    }
//    dbgAssert(pSource - compressed == cSize + 1);
}
*/

/*-----------------------------------------------------------------------------
    Name        : llPlaneCompressRLE
    Description : Compress a quantized plane of an image using an RLE scheme.
                    By quantization, we mean it is crunched down to less 128 or less.
    Inputs      : plane - quantized plane to compress
                  width, height - size of the plane, in pixels

    Outputs     : sizeDest - where to store the compressed size
    Return      : pointer to newly allocated copressed plane
    Note        : High bit set = duplicate next byte (n & 0x7f) + 1 times,
                  else straight copy next n + 1 bytes.
----------------------------------------------------------------------------*/
/*
ubyte *llPlaneCompressRLE(ubyte *plane, sdword width, sdword height, sdword *sizeDest)
{
    sdword size;
    ubyte *buffer, *smallBuffer, *testBuffer;   //compression buffer as allocated
    sdword index, length, runLength, x;
    ubyte *pixel, thisPixel, lastPixel;

    length = width * height - 1;
    pixel = plane;
    lastPixel = thisPixel = *pixel;
    pixel++;
    x = runLength = 0;

    size = width * height;
    size += size / 128 + height;                            //enough extra RAM for the worst case
    buffer = memAlloc(size, "RLECompressionBuffer", 0);
    size = 0;
#define byteWrite(b)    buffer[size] = (b); size++;

    index = 0;
    do
    {
        runLength = 0;
        while (thisPixel == lastPixel)
        {
            lastPixel = thisPixel;
            thisPixel = *pixel;
            dbgAssert(thisPixel <= 0x7f);
            runLength++;
            pixel++;
            x++;

            if (index + runLength >= length)
            {
                break;
            }

            if (runLength >= 0x3f)
            {
                break;
            }
        }

        if (runLength > 1)
        {
            byteWrite(runLength | 0xc0);                    //save lenght of run
            byteWrite(lastPixel);                           //save color of run
        }
        else if (lastPixel >= 0x3f)
        {
            byteWrite(runLength | 0xc0);                    //save lenght of 1
            byteWrite(lastPixel);                           //save color of pixel
        }
        else
        {
            byteWrite(lastPixel);                           //save one
        }
        lastPixel = thisPixel;
        index += runLength;
    }
    while (index < length);

    smallBuffer = memAlloc(size, "RLECompressed", 0);       //store only the amount needed into a small buffer
    memcpy(smallBuffer, buffer, size);
    memFree(buffer);
	*sizeDest = size;
    //test the compression
    testBuffer = memAlloc(width * height, "RLETestBuffer", 0);
    llPlaneDecompressRLE(testBuffer, smallBuffer, width, height, size, 1, 0);
    if (memcmp(testBuffer, buffer, width * height))
    {
        dbgWarningf(DBG_Loc, "Compression error of %d with image of size %dx%d", memcmp(testBuffer, buffer, width * height), width, height);
    }
    memFree(testBuffer);

    return(smallBuffer);
}
*/

/*-----------------------------------------------------------------------------
    Name        : llResultsView
    Description : Views the results of quantization in a TGA file.
    Inputs      : list - list of images to preview
                  listLength - length of list to preview
                  palette - common palette as computed
                  teamPalette0, 1 - team effect palettes
    Outputs     : creates and saves out llresults.tga
    Return      : void
----------------------------------------------------------------------------*/
texentry *llPreviewList = NULL;
sdword llListPreviewLength = -1;
#define LP_PreviewBlank     colRGB(0x30, 0x30, 0x30)
#define LP_PreviewBad       0xbadbb500
void llResultsView(texentry *list, sdword listLength, color *palette, ubyte *teamPalette0, ubyte *teamPalette1)
{
    sdword index, count;
    sdword width, height, tallestImage, x;
    color *buffer, *qBuffer, *qPointer, *qPointer1;
	ubyte *iPointer, *iPointer1;
//    ubyte *iBuffer0, *iBuffer1;

    //figure out how big an image we'll need
    for (index = width = height = 0; index < listLength; index++)
    {
        width += list[index].image->width;
        height = max(height, list[index].image->height);
    }
    tallestImage = height;
    height = tallestImage * 8;
    //allocate and clear out a buffer
    buffer = memAlloc(width * height * sizeof(color), "TGAPreview", 0);
    memClearDword(buffer, LP_PreviewBlank, width * height);
    //copy all images into the buffer
    for (index = x = 0; index < listLength; index++)
    {
        llBitmapCopy(buffer, x, 0, width, height, list[index].composited,
                     list[index].image->width, list[index].image->height);
        printf("\n%s", list[index].image->fileName);
        qBuffer = qPointer = memAlloc(list[index].image->width * list[index].image->height * sizeof(color), "QBuffer", 0);
        //make truecolor version of the quantized/compressed texture
        if (!bitTest(list[index].flags, TRF_Alpha))
        {
            iPointer = list[index].quantized;
            for (count = list[index].image->width * list[index].image->height; count > 0; count--, qPointer++, iPointer++)
            {
                *qPointer = palette[*iPointer];
            }
        }
        else
        {
            memClearDword(qBuffer, LP_PreviewBad, list[index].image->width * list[index].image->height);
        }
/*
        if (!bitTest(list[index].flags, TRF_Alpha))
        {
            llPlaneDecompressRLE((ubyte *)qBuffer + 0, list[index].compressed->image[0].red, list[index].image->width, list[index].image->height, list[index].compressed->image[0].redSize, 4, 3);
            llPlaneDecompressRLE((ubyte *)qBuffer + 1, list[index].compressed->image[0].green, list[index].image->width, list[index].image->height, list[index].compressed->image[0].greenSize, 4, 2);
            llPlaneDecompressRLE((ubyte *)qBuffer + 2, list[index].compressed->image[0].blue, list[index].image->width, list[index].image->height, list[index].compressed->image[0].blueSize, 4, 3);
        }
        else
        {
            memClearDword(qBuffer, LP_PreviewBad, list[index].image->width * list[index].image->height);
        }
*/
        llBitmapCopy(buffer, x, tallestImage, width, height, qBuffer,
                     list[index].image->width, list[index].image->height);
        //make a greyscale image of the unquantized team effect0 buffer
        qPointer = qBuffer;
        iPointer = list[index].teamEffect0;
        for (count = list[index].image->width * list[index].image->height; count > 0; count--, qPointer++, iPointer++)
        {
            *qPointer = colRGB(*iPointer, *iPointer, *iPointer);
        }
        llBitmapCopy(buffer, x, tallestImage * 2, width, height, qBuffer,
                     list[index].image->width, list[index].image->height);

        //make greyscale image of quantized team effect0 image
        if (!bitTest(list[index].flags, TRF_Alpha))
        {
            qPointer = qBuffer;
            iPointer = list[index].quantized;
            for (count = list[index].image->width * list[index].image->height; count > 0; count--, qPointer++, iPointer++)
            {
                *qPointer = colRGB(teamPalette0[*iPointer], teamPalette0[*iPointer], teamPalette0[*iPointer]);
            }
        }
        else
        {
            memClearDword(qBuffer, LP_PreviewBad, list[index].image->width * list[index].image->height);
        }
        llBitmapCopy(buffer, x, tallestImage * 3, width, height, qBuffer,
                     list[index].image->width, list[index].image->height);

        //make a greyscale image of the unquantized team effect1 buffer
        qPointer = qBuffer;
        iPointer = list[index].teamEffect1;
        for (count = list[index].image->width * list[index].image->height; count > 0; count--, qPointer++, iPointer++)
        {
            *qPointer = colRGB(*iPointer, *iPointer, *iPointer);
        }
        llBitmapCopy(buffer, x, tallestImage * 4, width, height, qBuffer,
                     list[index].image->width, list[index].image->height);

        //make greyscale image of quantized team effect1 image
        if (!bitTest(list[index].flags, TRF_Alpha))
        {
            qPointer = qBuffer;
            iPointer = list[index].quantized;
            for (count = list[index].image->width * list[index].image->height; count > 0; count--, qPointer++, iPointer++)
            {
                *qPointer = colRGB(teamPalette1[*iPointer], teamPalette1[*iPointer], teamPalette1[*iPointer]);
            }
        }
        else
        {
            memClearDword(qBuffer, LP_PreviewBad, list[index].image->width * list[index].image->height);
        }
        llBitmapCopy(buffer, x, tallestImage * 5, width, height, qBuffer,
                     list[index].image->width, list[index].image->height);

        //make a team colorized, unquantized image
        qPointer = qBuffer;
        qPointer1 = list[index].composited;
        iPointer = list[index].teamEffect0;
        iPointer1 = list[index].teamEffect1;

        for (count = list[index].image->width * list[index].image->height; count > 0; count--, qPointer++, iPointer++, iPointer1++, qPointer1++)
        {
            *qPointer = llTeamColorize(*qPointer1, *iPointer, *iPointer1, llTeamColor0, llTeamColor1);
        }
        llBitmapCopy(buffer, x, tallestImage * 6, width, height, qBuffer,
                     list[index].image->width, list[index].image->height);
        //make a team colorized image from the decompressed scene
        if (!bitTest(list[index].flags, TRF_Alpha))
        {
            qPointer = qBuffer;
            iPointer = list[index].quantized;

            for (count = list[index].image->width * list[index].image->height; count > 0; count--, qPointer++, iPointer++)
            {
                *qPointer = llTeamColorize(palette[*iPointer], teamPalette0[*iPointer], teamPalette1[*iPointer], llTeamColor0, llTeamColor1);
            }
        }
        else
        {
            memClearDword(qBuffer, LP_PreviewBad, list[index].image->width * list[index].image->height);
        }
/*
        qPointer = qBuffer;
        iPointer = iBuffer0 = memAlloc(list[index].image->width * list[index].image->height, "Team0TempBuffer", 0);
        iPointer1 = iBuffer1 = memAlloc(list[index].image->width * list[index].image->height, "Team1TempBuffer", 0);
        memset(iBuffer0, 0, list[index].image->width * list[index].image->height);
        memset(iBuffer1, 0, list[index].image->width * list[index].image->height);
        llPlaneDecompressRLE((ubyte *)qBuffer + 0, list[index].compressed->image[0].red, list[index].image->width, list[index].image->height, list[index].compressed->image[0].redSize, 4, 3);
        llPlaneDecompressRLE((ubyte *)qBuffer + 1, list[index].compressed->image[0].green, list[index].image->width, list[index].image->height, list[index].compressed->image[0].greenSize, 4, 2);
        llPlaneDecompressRLE((ubyte *)qBuffer + 2, list[index].compressed->image[0].blue, list[index].image->width, list[index].image->height, list[index].compressed->image[0].blueSize, 4, 3);
        if (bitTest(list[index].flags, TRF_TeamColor0))
        {
            llPlaneDecompressRLE(iBuffer0, list[index].compressed->image[0].team0, list[index].image->width, list[index].image->height, list[index].compressed->image[0].team0Size, 1, 2);
        }
        if (bitTest(list[index].flags, TRF_TeamColor1))
        {
            llPlaneDecompressRLE(iBuffer1, list[index].compressed->image[0].team1, list[index].image->width, list[index].image->height, list[index].compressed->image[0].team1Size, 1, 2);
        }
        for (count = list[index].image->width * list[index].image->height; count > 0; count--, qPointer++, iPointer++, iPointer1++)
        {
            *qPointer = llTeamColorize(*qPointer, *iPointer, *iPointer1);
        }
        memFree(iBuffer1);
        memFree(iBuffer0);
*/
        llBitmapCopy(buffer, x, tallestImage * 7, width, height, qBuffer,
                     list[index].image->width, list[index].image->height);
        memFree(qBuffer);
        x += list[index].image->width;
    }
    //write out the image and free the image buffer
    tgaHeaderWriteRGB("llresults.tga", "Quantization results", width, height);
    tgaBodyWriteRGB(buffer, width, height);
    tgaWriteClose();
    memFree(buffer);
}

/*-----------------------------------------------------------------------------
    Name        : llTexturesQuantize
    Description : Composite and quantize a set of textures to a common palette.
    Inputs      : mifFile - file which contains a listing of the textures
    Outputs     : For each .lif file in the .mif file, The corresponding
                    .psd will be loaded from DataSrc\ and quantized to a .lif
                    in Data\
    Return      : void
----------------------------------------------------------------------------*/
void llTexturesQuantize(char *mifFile)
{
    FILE *f;
    char string[256], *fileName;
    char sourcePath[_MAX_PATH], destPath[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR], otherDir[_MAX_DIR], *dirUpper;
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    udword datasrcOffset;
	sdword listLength, index, /*nColors,*/ count;
    texentry *list;                             //list of textures to load
    color commonPalette[256];                   //common color palette
    ubyte teamColor0[256], teamColor1[256];     //common team effect palettes
	sdword nColors;
//    udword paletteIndex[256];                   //'backward-palette' mapping table
//    ubyte *pByte;

    if ((f = fopen(mifFile, "rt")) == NULL)
    {                                                       //open the .MIF file
        dbgFatalf(DBG_Loc, "Error opening file '%s'.", mifFile);
    }

    //allocate the texture info list and clear to all zero
    listLength = 0;
    list = memAlloc(sizeof(texentry) * LL_MaxTexEntries, "texentry list", 0);
    memset(list, 0, sizeof(texentry) * LL_MaxTexEntries);

    while (!feof(f))
    {                                                       //read every line in the file
        if (fgets(string, 256, f) == NULL)
        {
            break;
        }
        if (strstr(string, "+=") == NULL)
        {                                                   //if not a valid line
            continue;                                       //skip line
        }
        strtok(string, " +=\n\t");
        fileName = strtok(NULL, " +=\n\t");
//        printf("\nQuantizing: '%s'...", fileName);
        //actually quantize it
        //build source/dest file paths
        dbgAssert(strchr(fileName, '.'));
        *strchr(fileName, '.') = 0;
        _splitpath(mifFile, drive, dir, fname, ext );       //get the source path from the ,MIF file
        _makepath(sourcePath, drive, dir, fileName, ".PSD");//make path to the .PSD (source) file

        dbgAssert(stristr(dir, "datasrc\\"));
        datasrcOffset = stristr(dir, "datasrc\\") - dir;
        memcpy(otherDir, dir, datasrcOffset);
        otherDir[datasrcOffset] = 0;
        strcat(otherDir, "Data\\");
        strcat(otherDir, dir + datasrcOffset + strlen("datasrc\\"));
        _makepath(destPath, drive, otherDir, fileName, ".LiF");//make path to the .LiF (dest) file

        if (listLength >= LL_MaxTexEntries)
        {
            dbgFatalf(DBG_Loc, "Overflowed texure list of length %s", LL_MaxTexEntries);
        }
        list[listLength].sourcePath = memStringDupe(sourcePath);
        list[listLength].destPath = memStringDupe(destPath);//store copies of the names
        listLength++;
    }

    //close the listing file
    fclose(f);

//    printf("\nQuantizing %d images from '%s'", listLength, mifFile);
    llProgressStart(mifFile, '0', ':', 79 - strlen(mifFile), listLength);

    qQuantizeReset();
    //first pass: load, decompress and quantize the images
    for (index = 0; index < listLength; index++)
    {
        //load the image
        list[index].image = psdLayeredFileLoad(list[index].sourcePath);
        llProgressUpdate();
        //store the info on texture to the listing file
        if (llNElements >= LL_MaxElements)
        {
            dbgFatalf(DBG_Loc, "Exceeded %d textures.", LL_MaxElements);
        }
        //make sure width/height are valid
        if (forceDimensions)
        {
            if (bitNumberSet(list[index].image->width, 16) != 1 || bitNumberSet(list[index].image->width, 16) != 1)
            {
                dbgFatalf(DBG_Loc, "PSD file '%s' not even exponent of 2 (%dx%d).", list[index].destPath, list[index].image->width, list[index].image->height);
            }
        }
        if (list[index].image->width < LL_MinWidth || list[index].image->width > LL_MaxWidth || list[index].image->height < LL_MinHeight || list[index].image->height > LL_MaxHeight)
        {
            dbgFatalf(DBG_Loc, "PSD file '%s' is a bad size: (%dx%d).", list[index].destPath, list[index].image->width, list[index].image->height);
        }
        //create a path like "R1\AdvanceSupportFrigate\Rl0\LOD2\front_tip"
        _splitpath(list[index].sourcePath, drive, dir, fname, ext );//get the source path from the ,MIF file
        dirUpper = memStringDupe(dir);
        _strupr(dirUpper);
        datasrcOffset = (udword)strstr(dirUpper, "DATASRC\\");
        dbgAssert(datasrcOffset != 0);                      //offet of the 'datasrc\'
        datasrcOffset -= (udword)dirUpper;
        memFree(dirUpper);
        _makepath(sourcePath, "", dir + datasrcOffset + strlen("DataSrc\\"), fname, "");
        //see if the named texture has already been referenced
        for (count = 0; count < llNElements; count++)
        {
            if (!_stricmp(llElementList[count].textureName, sourcePath))
            {                                           //if texture already here
                break;                                  //use this index
            }
        }
        //composite the image, making team color layers greyscale and making team color buffers
        list[index].composited = llImageComposite(list[index].image,
            &list[index].teamEffect0, &list[index].teamEffect1, &llElementList[count].flags);
        //compute a crc of the image based on the composited image and the name
//        _splitpath(llElementList[index].textureName, drive, dir, fname, ext );//get just the name of the image
        llElementList[count].imageCRC = crc32Compute((ubyte *)list[index].composited, list[index].image->width * list[index].image->height * sizeof(color)),
                                        crc32Compute(fname, strlen(fname));
        llImageBandFilterIndexed(list[index].teamEffect0,
                list[index].image->width * list[index].image->height,
                LL_TeamEffectBandLo, LL_TeamEffectBandHi, 0);
        llImageBandFilterIndexed(list[index].teamEffect1,
                list[index].image->width * list[index].image->height,
                LL_TeamEffectBandLo, LL_TeamEffectBandHi, LL_TeamEffectBandHi);
        //now we have index of entry to fill, fill it in
        llElementList[count].width = list[index].image->width;
        llElementList[count].height = list[index].image->height;
        llElementList[count].textureName = memStringDupe(sourcePath);
        list[index].flags = llElementList[count].flags;     //save these flags for later use
//        list[index].elementIndex = count;
/*
        if (bitTest(llElementList[count].flags, TRF_Alpha) && (llElementList[count].flags & (TRF_TeamColor0 | TRF_TeamColor1)))
        {
            dbgWarningf(DBG_Loc, "Image '%s' has both alpha and team color - 32x inflation possible!", list[index].destPath);
        }
*/
        if (count == llNElements)
        {
            llNElements++;                                  //else add this data to new member
        }
        //prepare this image for quantization
        list[index].quantized = memAlloc(list[index].image->width * list[index].image->height, "Index image data", 0);
        list[index].teamEffectFull = llTeamEffectsCombine(list[index].teamEffect0, list[index].teamEffect1,
            list[index].image->width * list[index].image->height);
        qRGBImageAdd(list[index].quantized, list[index].composited,//add this image into quantization pool
             list[index].image->width, list[index].image->height, list[index].teamEffectFull);
    }

    if ((listLength != 0) && (forceAlpha == FALSE))
    {
        //now quantize the whole bunch
        nColors = qRGBQuantizeQueue(commonPalette);

        //compute palettes of the team color effect
        llTeamEffectPalettesCompute(list, listLength, commonPalette, nColors, teamColor0, teamColor1);
    }
/*
    //compress all the images
    for (index = 0; index < listLength; index++)
    {
        list[index].compressed = llTexturesCompressAllSizes16Bit(list[index].composited, list[index].image->width, list[index].image->height, list[index].teamEffect0, list[index].teamEffect1, list[index].flags);
        //store compressed size info in the element structure for the .ll file
        llElementList[list[index].elementIndex].nSizes = list[index].compressed->nImages;
        for (count = 0; count < list[index].compressed->nImages; count++)
        {
            llElementList[list[index].elementIndex].imageSize[count] =
                list[index].compressed->image[count].imageSize;
        }
    }
*/
	//prepare for a preview, if it's to happen
	llListPreviewLength = listLength;
	llPreviewList = list;
    if (viewResults)
    {
        llResultsView(list, listLength, commonPalette, teamColor0, teamColor1);
    }

    //final pass: save to disk and free up everything
    for (index = 0; index < listLength; index++)
    {
        // save the image to disk
//		teamColor0[0] = 0xf4;
        llLIFSave(&list[index], commonPalette, teamColor0, teamColor1);
//        llLIFSaveCompressed(&list[index]);
        //free up all memory in the images
        memFree(list[index].teamEffect0);
        memFree(list[index].teamEffect1);
        memFree(list[index].quantized);
        memFree(list[index].composited);
        memFree(list[index].sourcePath);
        memFree(list[index].destPath);
        liImageDelete(list[index].image);
    }

    //perform cleanup
    memFree(list);                                          //done with texture list, free it

}

/*-----------------------------------------------------------------------------
    Name        : llElementUsagePrint
    Description : Prints memory usage of the current element list, both
                    per-ship and total.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void llElementUsagePrint(void)
{
    sdword index;
    char *filePath, *slashPointer, *lastSlash;
    sdword pSize, rSize;
    sdword nTextures = 0, nSharedTextures = 0;
    sdword palettedSize = 0, RGBSize = 0;
    sdword sharedPalettedSize = 0, sharedRGBSize = 0;
    sdword totalTextures = 0, totalSharedTextures = 0, skippedTextures = 0;
    sdword totalPalettedSize = 0, totalRGBSize = 0;
    sdword totalSharedPalettedSize = 0, totalSharedRGBSize = 0;
    sdword nScanned, pageIndex;

    printf("\nTexture usage stats for '%s'.", llFileName);
    printf("\nPaletted sizes do not reflect size of actual palettes.");
    printf("\nRGB sizes are largest compressed version.");
    printf("\n#                 # shared   shared ");
    printf("\nTextures     size textures     size directory");
    printf("\n---------------------------------------------------------------");
    filePath = llElementList[0].textureName;
    for (index = 0; index < llNElements; index++)
    {
        //see if the path of the current element has changed, reflecting a new ship
        if ((slashPointer = strchr(llElementList[index].textureName, '\\')) != NULL)
        {
            do                                              //isolate the path
            {
                lastSlash = slashPointer;
            }
            while ((slashPointer = strchr(lastSlash + 1, '\\')) != NULL);
            pageIndex = -1;
            nScanned = sscanf(lastSlash + 1, "page%d", &pageIndex);
            if (nScanned == 1 && pageIndex >= 0 && pageIndex < 20)
            {                                               //if it's a PACO-generated file
                skippedTextures++;
                continue;                                   //don't use it
            }
            if (_memicmp(filePath, llElementList[index].textureName,
                        lastSlash - llElementList[index].textureName) != 0)
            {                                               //if doesn't match current texture path
oneLastTime:
                //... display some info
                printf("\n%8d %8d %8d %8d %s", nTextures, palettedSize,
                       nSharedTextures, sharedPalettedSize, filePath);
                //reset the size counters
                totalPalettedSize += palettedSize;          //add up the totals
                totalRGBSize += RGBSize;
                totalSharedPalettedSize += sharedPalettedSize;
                totalSharedRGBSize += sharedRGBSize;
                totalTextures += nTextures;
                totalSharedTextures += nSharedTextures;
                palettedSize = RGBSize = nTextures = 0;     //clear the local counts
                sharedPalettedSize = sharedRGBSize = nSharedTextures = 0;
                filePath = llElementList[index].textureName;//remember current texture path
                if (index == llNElements)
                {
                    goto doneTheOneLastTime;
                }
            }
            //update the size variables
            pSize = llElementList[index].width * llElementList[index].height;
//            rSize = llElementList[index].imageSize[0];
			rSize = pSize * sizeof(color);
            if (bitTest(llElementList[index].flags, TRF_Alpha))
            {                                               //alpha textures are full-size
                pSize *= sizeof(color);
            }
            palettedSize += pSize;
            RGBSize += rSize;
            nTextures++;
            if (llElementList[index].sharedFrom == -1)
            {                                               //if it's not a shared texture
                sharedPalettedSize += pSize;
                sharedRGBSize += rSize;
                nSharedTextures++;
            }
        }
		else
		{
			dbgAssert(FALSE);
		}
    }
    goto oneLastTime;
doneTheOneLastTime:;

    //print the totals
    printf("\n---------------------------------------------------------------");
    printf("\n%8d %8d %8d %8d", totalTextures, totalPalettedSize,
           totalSharedTextures, totalSharedPalettedSize);
    dbgAssert(totalTextures + skippedTextures == llNElements);
}

/*-----------------------------------------------------------------------------
    Name        : llLostFilesFilter
    Description : Find any files in the listing that are not on the local
                    hard drive and list them to stdout.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void llLostFilesFilter(void)
{
    sdword index, status;
    char fullPath[_MAX_PATH];
    struct _finddata_t findData;

    for (index = 0; index < llNElements; index++)
    {
        sprintf(fullPath, "%s%s.lif", llListingPath, llElementList[index].textureName);
        status = _findfirst(fullPath, &findData);
        if (status == -1)
        {                                                   //if file not found
            printf("%s\n", llElementList[index].textureName);
        }
        else
        {
            _findclose(status);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : main
    Description : main function
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    char *string, *llMifFile;
    sdword argIndex;

    if ((string = llSystemsStartup()) != NULL)
    {
        printf(string);
        return(0xfed5);
    }
    //now process the command line
    //search for options, if any
    argIndex = 1;
    while (argIndex < argc && strchr("-/", argv[argIndex][0]))
    {
        switch (toupper(argv[argIndex][1]))
        {
            case 'A':
                forceAlpha = TRUE;
                break;
            case 'I':
                forceDimensions = FALSE;
                break;
            case 'V':
                viewResults = TRUE;
                break;
            default:
                usagePrint(argv[0]);
        }
		argIndex++;
    }
    if (argc <= argIndex)
    {
        usagePrint(argv[0]);
    }
    //search for a command
    dbgAssert(argIndex < argc);
    switch (toupper(*argv[argIndex]))
    {
        case 'A':
            llCommand = LC_Add;
            break;
        case 'F':
            llCommand = LC_Filter;
            break;
        case 'Q':
            llCommand = LC_Quantize;
            break;
        case 'S':
            llCommand = LC_Subtract;
            break;
        case 'U':
            llCommand = LC_Usage;
            break;
        case 'V':
            llCommand = LC_View;
            break;
        default:
            usagePrint(argv[0]);
    }
    argIndex++;
    //get name of .ll file
    dbgAssert(argIndex < argc);
    llFileName = argv[argIndex];
    _splitpath(llFileName, drive, dir, fname, ext);
    sprintf(llListingPath, "%s%s", drive, dir);
    argIndex++;
    //get any other command-specific parameters
    if (llCommand == LC_Quantize || llCommand == LC_Subtract || llCommand == LC_Add)
    {
        dbgAssert(argIndex < argc);
        llMifFile = argv[argIndex];
        argIndex++;
    }

    //now we've parsed the command line, let's read in the existing lif listing, if it exists
    llFileInit();

    //prepare for team colorization
    /*
    teamRed0 = colUbyteToReal(colRed(llTeamColor0));
    teamRed1 = colUbyteToReal(colRed(llTeamColor1));
    teamGreen0 = colUbyteToReal(colGreen(llTeamColor0));
    teamGreen1 = colUbyteToReal(colGreen(llTeamColor1));
    teamBlue0 = colUbyteToReal(colBlue(llTeamColor0));
    teamBlue1 = colUbyteToReal(colBlue(llTeamColor1));
    */
    //now perform specified command:
    switch (llCommand)
    {
        case LC_View:
            llElementsView();
            break;
        case LC_Quantize:
            llTexturesQuantize(llMifFile);
            llFlushList();
            break;
        case LC_Subtract:
            llTexturesSubtract(llMifFile);
            llFlushList();
            break;
        case LC_Add:
            llAddMif(llMifFile);
            llFlushList();
            break;
        case LC_Usage:
            llElementUsagePrint();
            break;
        case LC_Filter:
            llLostFilesFilter();
            break;
    }

    llSystemsShutdown();
    return(0);
}

