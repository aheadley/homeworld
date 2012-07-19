/*=============================================================================
    Name    : Texreg.c
    Purpose : Maintain a texture registry

    Created 9/26/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#include <windows.h>
#endif
#include "glinc.h"
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "memory.h"
#include "color.h"
#include "twiddle.h"
#include "prim2d.h"
#include "file.h"
#include "key.h"
#include "texreg.h"
#include "statscript.h"
#include "strings.h"
#include "colpick.h"
#include "render.h"

#include "main.h"
#include "utility.h"
#include "HorseRace.h"

#include "glcaps.h"
#include "universe.h"

/*=============================================================================
    Data:
=============================================================================*/
//configuration information:
sdword trRegistrySize = TR_RegistrySize;        //size of texture registry
static bool trNoPalInitialized = FALSE;

sdword trTextureChanges = 0;
sdword trAvoidedChanges = 0;

bool trNoPalettes = FALSE;
bool trSharedPalettes = FALSE;

typedef void (APIENTRY * PFNGLCOLORTABLEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);

PFNGLCOLORTABLEEXTPROC glColorTableEXT = NULL;  //address of palette-download function
PFNGLCOLORTABLEEXTPROC glLitColorTableEXT = NULL;

sdword trLitPaletteBits;

//actual registry:
texreg *trTextureRegistry = NULL;
crc32 *trNameCRCs;                              //separate list to reduce cache misses during searches.
sdword trLowestFree = 0;                        //indices of extremes free texture indices
sdword trHighestAllocated = -1;

//default coloration palette
trcolorinfo trDefaultColorInfo = {colWhite, colWhite};
trcolorinfo trUnusedColorInfo = {TR_UnusedColorInfo, TR_UnusedColorInfo};
trmemmap *trSizeSortList = NULL;
sdword trSizeSortLength;
trmeshsort *trMeshSortList;
sdword trMeshSortLength;
//sdword trMeshSortSubLength;
sdword trMeshSortIndex;

//info on RAM pools for texture fitting
//!!! for now, we just say there is one pool of 4MB
trrampool trRamPoolList[TR_NumberRamPools] =
{
    {
        NULL,                                   //no real base
#ifdef khentschel
        1024 * 1024 * 12,
#else
        1024 * 1024 * 10,//!!!,                        //n MB of texture RAM
#endif
        0                                       //none used yet
    }
};
sdword trNumberRamPools = 1;                    //only one pool for now
sdword trRamPoolGranularity = 16;               //granularity - this is just a guess

#if TR_DEBUG_TEXTURES
color trTestPalette0[TR_PaletteLength];         //black-to-white palette
bool trSpecialTextures = FALSE;
sdword trSpecialTextureMode = TSM_None;
char *trSpecialTextureName[TSM_NumberModes] =
{"Normal", "Uncolored", "Base color buffer", "Stripe color buffer"};
#endif

sdword texLinearFiltering = TRUE;               //enable bi-linear filtering
sdword texLinearMag = TRUE;

//multiplier to the team color effect mulipliers
real32 trBaseColorScalar = 0.0f;
real32 trStripeColorScalar = 0.0f;

#if TR_PRINT_TEXTURE_NAMES
bool trPrintTextureNames = FALSE;
#endif

trhandle trCurrentHandle = TR_Invalid;

sdword trNoPalHighestAllocated;
sdword trNoPalBytesAllocated;

static sdword trNpNumHandles = TR_NP_NumHandles;
nopalreg* trNoPalRegistry = NULL;

#define ADJ_NPQUEUE(x) ((x) % (trNoPalQueueSize - 1))

sdword  trNoPalQueueSize;
sdword  trNoPalQueueHead, trNoPalQueueTail;
udword* trNoPalQueue;

sdword trNoPalMaxBytes = 20 * 1024 * 1024;

bool glRGBA16 = FALSE;


/*=============================================================================
    Functions:
=============================================================================*/

void trNoPalReadjustWithoutPending(void);

/*-----------------------------------------------------------------------------
    Name        : trLitPaletteBitDepth
    Description : determines the required bit depth of pre-lit palettes
    Inputs      :
    Outputs     :
    Return      : number of bits in palette entries
----------------------------------------------------------------------------*/
sdword trLitPaletteBitDepth(void)
{
    sdword bits;
    dbgAssert(glLitColorTableEXT != NULL);
    glLitColorTableEXT(0, 0, 0, 0, 0, &bits);
    return bits;
}

/*-----------------------------------------------------------------------------
    Name        : trColorTable
    Description : wrapper for glColorTableEXT
    Inputs      : palette - the palette
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static bool trUpdate = FALSE;
static ubyte* trLastPalette = NULL;
void trColorTable(color* palette)
{
    if (trNoPalettes)
    {
        return;
    }
    if (RGL)
    {
        if (trSharedPalettes && !trUpdate && (trLastPalette == (ubyte*)palette))
        {
            return;
        }

        trLastPalette = (ubyte*)palette;
        trUpdate = FALSE;

        glColorTableEXT(GL_TEXTURE_2D, GL_RGBA, TR_PaletteLength,
                        GL_RGBA, GL_UNSIGNED_BYTE, palette);
    }
    else
    {
        ubyte  newpalette[3*256];
        sdword i;
        color  c;

        if (trSharedPalettes && !trUpdate && (trLastPalette == (ubyte*)palette))
        {
            return;
        }

        trLastPalette = (ubyte*)palette;
        trUpdate = FALSE;

        for (i = 0; i < 256; i++)
        {
            c = palette[i];
            newpalette[3*i + 0] = colRed(c);
            newpalette[3*i + 1] = colGreen(c);
            newpalette[3*i + 2] = colBlue(c);
        }
        glColorTableEXT(GL_TEXTURE_2D, GL_RGB, TR_PaletteLength,
                        GL_RGB, GL_UNSIGNED_BYTE, newpalette);
    }
}

/*-----------------------------------------------------------------------------
    Name        : trReload
    Description : fixup .DLL hooks that may have changed
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trReload(void)
{
    //glcaps module has already asserted that paletted texture extensions exist
    if (!trNoPalettes)
    {
        glColorTableEXT = (PFNGLCOLORTABLEEXTPROC)rwglGetProcAddress("glColorTableEXT");
        dbgAssert(glColorTableEXT != NULL);
    }

    glLitColorTableEXT = NULL;
    if (RGLtype == SWtype)
    {
        glLitColorTableEXT = (PFNGLCOLORTABLEEXTPROC)rwglGetProcAddress("glLitColorTableEXT");
        trLitPaletteBits = trLitPaletteBitDepth();
    }
    else
    {
        trLitPaletteBits = 0;
    }

    if (RGL)
    {
        if (enableStipple)
        {
            glEnable(GL_POLYGON_STIPPLE);
        }
        else
        {
            glDisable(GL_POLYGON_STIPPLE);
        }
    }

    trSharedPalettes = glCapFeatureExists(GL_SHARED_TEXTURE_PALETTE_EXT);
    if (trSharedPalettes)
    {
        glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
    }
}

/*-----------------------------------------------------------------------------
    Name        : trStartup
    Description : startup the texture registry
    Inputs      : void
    Outputs     : allocates the texture registry
    Return      :
----------------------------------------------------------------------------*/
void trStartup(void)
{
#if TR_DEBUG_TEXTURES
    sdword index;
    for (index = 0; index < TR_PaletteLength; index++)
    {
        trTestPalette0[index] = colRGB(index, index, index);
    }
#endif

#if TR_VERBOSE_LEVEL >= 1
    dbgMessagef("\ntrStartup: creating a registry of %d entries", trRegistrySize);
#endif  //TR_VERBOSE_LEVEL
    dbgAssert(trTextureRegistry == NULL);
    trTextureRegistry = memAlloc(trRegistrySize * sizeof(texreg),
                                 "Texture registry", NonVolatile);//allocate texture registry
    trNameCRCs = memAlloc(trRegistrySize * sizeof(crc32), "texRegCRC's", NonVolatile);

    trNoPalStartup();                                       //must come before trReset
    trReset();                                              //reset the newly-allocated texture registry

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    //glcaps module has already asserted that paletted texture extensions exist
    if (!trNoPalettes)
    {
        glColorTableEXT = (PFNGLCOLORTABLEEXTPROC)rwglGetProcAddress("glColorTableEXT");
        dbgAssert(glColorTableEXT != NULL);
    }

    glLitColorTableEXT = NULL;
    if (RGLtype == SWtype)
    {
        glLitColorTableEXT = (PFNGLCOLORTABLEEXTPROC)rwglGetProcAddress("glLitColorTableEXT");
        trLitPaletteBits = trLitPaletteBitDepth();
    }
    else
    {
        trLitPaletteBits = 0;
    }

    if (RGL)
    {
        if (enableStipple)
        {
            glEnable(GL_POLYGON_STIPPLE);
        }
        else
        {
            glDisable(GL_POLYGON_STIPPLE);
        }
    }

    trSharedPalettes = glCapFeatureExists(GL_SHARED_TEXTURE_PALETTE_EXT);
    if (trSharedPalettes)
    {
        glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
    }
}

/*-----------------------------------------------------------------------------
    Name        : trReset
    Description : Reset the texture registry
    Inputs      :
    Outputs     : Deletes any textures which may be floating about.
    Return      :
----------------------------------------------------------------------------*/
static sdword bNewList = TRUE;
void trReset(void)
{
    sdword index;
    texreg *reg;

    glRGBA16 = glCapTexSupport(GL_RGBA16);

    trNoPalReset();

#if TR_VERBOSE_LEVEL >= 1
    dbgMessagef("\ntrReset: resetting texture registry");
#endif  //TR_VERBOSE_LEVEL
    for (index = 0; index < trRegistrySize; index++)
    {
        if ((!bNewList) && (trAllocated(index)))
        {
            trTextureDelete(index);                         //delete the texture if it exists
        }
        //since we really only need to set the internal handle as invalid
        //to dictate that the registry entry is blank, we could leave the
        //other members in whatever state they were previously.
        //Hence, we only do this in debug mode.
        reg = trStructure(index);
        reg->flags = 0;
#if TR_ERROR_CHECKING
        reg->meshReference = NULL;
        reg->nUsageCount = 0;
        reg->diskWidth = reg->diskHeight = 0;
        reg->scaledWidth = reg->scaledHeight = 0;
        reg->fileName = NULL;
        reg->nPalettes = 0;
        reg->palettes = NULL;
#endif
        reg->nSharedTo = 0;
        reg->sharedFrom = TR_NotShared;
        trSetFree(index);
        trClearPending(index);
    }
    trLowestFree = 0;
    trHighestAllocated = -1;
    bNewList = FALSE;
    trCurrentHandle = TR_Invalid;
    memClearDword(trNameCRCs, 0, trRegistrySize);           //clear all CRC's to 0
}

/*-----------------------------------------------------------------------------
    Name        : trShutdown
    Description : Shut down the texture registry.
    Inputs      :
    Outputs     : Deletes any textures which may be floating about.
    Return      :
----------------------------------------------------------------------------*/
void trShutdown(void)
{
#if TR_VERBOSE_LEVEL >= 1
    dbgMessagef("\ntrShutdown: shutting down texture registry");
#endif  //TR_VERBOSE_LEVEL
    trReset();                                              //this'll kill all the remaining textures
    memFree(trTextureRegistry);                             //free the texture registry itself
    trTextureRegistry = NULL;
    memFree(trNameCRCs);
    trNameCRCs = NULL;

    bNewList = TRUE;

    trNoPalShutdown();

    if (trSharedPalettes)
    {
        glDisable(GL_SHARED_TEXTURE_PALETTE_EXT);
    }
}

/*-----------------------------------------------------------------------------
    Name        : trInternalTexturesDelete
    Description : Delete the internal textures of a texreg, including the
                    handle(s) and palette(s) without deleting anything else in it.
    Inputs      : handle - handle of the textures to delete
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trInternalTexturesDelete(trhandle handle)
{
    texreg *reg = trStructure(handle);
    trcolorinfo *oldColorInfos, *newColorInfos;
    sdword index, colorIndex = trPaletteIndex(handle);
    udword *handles;

    dbgAssert(!trPending(trIndex(handle)));                 //make sure it has internal textures
    //sharing handled
    if (reg->sharedFrom != TR_NotShared)
    {
        oldColorInfos = (trcolorinfo *)reg->palettes;
    }
    else if (bitTest(reg->flags, TRF_Paletted))
    {                                                       //if it's a paletted texture
        oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + TR_PaletteSize * reg->nPalettes);
    }
    else
    {                                                       //else's it's an RGB texture
        oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + sizeof(udword) * reg->nPalettes);
    }
    newColorInfos = memAlloc(TR_NumPaletteSize, "dupe trcolorinfo list", NonVolatile);
    memcpy(newColorInfos, oldColorInfos, sizeof(trcolorinfo) * reg->nPalettes);
    if (trNoPalettes && bitTest(reg->flags, TRF_Paletted))
    {
        if (reg->handle != TR_InvalidInternalHandle)
        {
            trNoPalTextureDelete(reg->handle);
        }
    }
    else if (bitTest(reg->flags, TRF_Paletted) ||           //if unpaletted texture
             reg->nPalettes == 1)
    {
        if (reg->handle != TR_InvalidInternalHandle)
        {                                                   //if this texture exists
            glDeleteTextures(1, (GLuint*)&reg->handle);              //delete GL-internal texture
        }
    }
    else
    {
        handles = (udword *)reg->palettes;
        for (index = 0; index < reg->nPalettes; index++)
        {
            if (handles[index] != TR_InvalidInternalHandle)
            {
                glDeleteTextures(1, (GLuint*)&handles[index]);
                handles[index] = TR_InvalidInternalHandle;
            }
        }
    }
    reg->handle = TR_InvalidInternalHandle;                 //no handle left here
    //don't even bother killing the special textures

    //free the palette or list of texture handles
    //sharing handled
    if (reg->sharedFrom != TR_NotShared)
    {                                                       //if it's shared
        memFree(reg->palettes);
    }
    else if (bitTest(reg->flags, TRF_Paletted))
    {   //if it's a paletted texture, it shares it's palette with other textures.
        //We have a usage count, the alpha (high bit) of the first entry is our
        //usage count.
        if ((((color *)reg->palettes)[0] & 0xff000000) != 0)
        {                                                   //if the palette has a usage count
            dbgAssert((((color *)reg->palettes)[0] & 0xff000000) != 0xff000000);
            ((color *)reg->palettes)[0] -= 0x01000000;      //decrement the usage count
            if (((((color *)reg->palettes)[0]) & 0xff000000) == 0)
            {                                               //if usage count reaches 0
                memFree(reg->palettes);                     //free the palette
            }
        }
    }
    else
    {                                                       //else RGB: free list of texture handles
        dbgAssert(reg->palettes != NULL);
        memFree(reg->palettes);
    }
    reg->palettes = (ubyte *)newColorInfos;                 //save list of color infos
}

/*-----------------------------------------------------------------------------
    Name        : trAllSharedFromDelete
    Description : 'Unshare' any texture shared from the specified index.
    Inputs      : iSharedFrom - index to remove references to
                  bRemoveInternalTextures - if TRUE, the texture will have it's
                    innards freed properly.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trAllSharedFromDelete(sdword iSharedFrom, bool bRemoveInternalTextures)
{
    sdword index;

    for (index = 0; index <= trHighestAllocated; index++)
    {
        if (index == iSharedFrom || (!trAllocated(index)))
        {
            continue;
        }
        if (trTextureRegistry[index].sharedFrom == iSharedFrom)
        {                                                   //if this is one of the textures sharing the one we're removing references from
            //trTextureDelete(index);
            if (bRemoveInternalTextures && (!trPending(index)))
            {
                trInternalTexturesDelete(index);
            }
            trSetPending(index);                            //make this texture pending
            trTextureRegistry[index].sharedFrom = TR_NotShared;//set it to not shared
            trTextureRegistry[iSharedFrom].nSharedTo--;
            if (trTextureRegistry[iSharedFrom].nSharedTo == 0)
            {
                break;
            }
        }
    }
    dbgAssert(trTextureRegistry[iSharedFrom].nSharedTo == 0);
}

/*-----------------------------------------------------------------------------
    Name        : trTextureDelete
    Description : Deletes a given texture.
    Inputs      : handle - handle of the texture to delete
    Outputs     : Deletes ALL instances of the texture.  This is NOT the
                    same as trTextureUnregister.
    Return      : Old usage count
----------------------------------------------------------------------------*/
sdword trTextureDelete(trhandle handle)
{
    texreg *reg = trStructure(handle);
    sdword oldUsageCount;

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        return(0);
    }
#endif
#if TR_VERBOSE_LEVEL >= 2
    dbgMessagef("\ntrTextureDelete: deleting texture handle 0x%x, internalHandle 0x%x", handle, reg->handle);
#endif  //TR_VERBOSE_LEVEL
    dbgAssert(trAllocated(trIndex(handle)));
    oldUsageCount = reg->nUsageCount;                       //remember usage count
    //kill the GL textures
    if (!trPending(trIndex(handle)))
    {                                                       //if this texture was loaded proper
        trInternalTexturesDelete(handle);
    }
    //sharing handled
    if (reg->sharedFrom != TR_NotShared)
    {                                                       //if this one is shared from another texture
        dbgAssert(reg->nSharedTo == 0);
        dbgAssert(trAllocated(reg->sharedFrom));
        dbgAssert(trTextureRegistry[reg->sharedFrom].nSharedTo > 0);
        trTextureRegistry[reg->sharedFrom].nSharedTo--;
    }
    if (reg->nSharedTo > 0)
    {                                                       //if this texture is shared to otherTextures
        trAllSharedFromDelete(trIndex(handle), FALSE);
    }

    //delete the name string
    memFree(reg->fileName);                                 //free the name of the texture
    trNameCRCs[trIndex(handle)] = 0;                        //clear this CRC to blank
    reg->flags = 0;
#if TR_ERROR_CHECKING
    reg->meshReference = NULL;
    reg->nUsageCount = 0;
    reg->diskWidth = reg->diskHeight = 0;
    reg->scaledWidth = reg->scaledHeight = 0;
    reg->fileName = NULL;
    reg->nPalettes = 0;
    reg->palettes = NULL;
#endif
    reg->nSharedTo = 0;
    reg->sharedFrom = TR_NotShared;
    trSetFree(trIndex(handle));
    trLowestFree = min(trLowestFree, (sdword)trIndex(handle));//maintain index of lowest free index
    if ((sdword)trIndex(handle) == trHighestAllocated)
    {                                                       //if deleting highest allocated
        for (; trHighestAllocated > 0; trHighestAllocated--)
        {                                                   //search for highest allocated
            if (trAllocated(trHighestAllocated))
            {                                               //if this guy allocated
                break;
            }
        }
    }
    return(oldUsageCount);
}

/*-----------------------------------------------------------------------------
    Name        : trColorInfosPointer
    Description : Get the list of colorinfo structures for a given texture.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
trcolorinfo *trColorInfosPointer(sdword textureIndex)
{
    texreg *reg = trStructure(textureIndex);

    if (trPending(textureIndex) || (reg->sharedFrom != TR_NotShared))
    {                                                   //if pending or shared
        return((trcolorinfo *)reg->palettes);
    }

    if (bitTest(reg->flags, TRF_Paletted))
    {                                                   //if it's a paletted texture
        return((trcolorinfo *)((ubyte *)reg->palettes + TR_PaletteSize * reg->nPalettes));
    }
    else
    {                                                   //else's it's an RGB texture
        return((trcolorinfo *)((ubyte *)reg->palettes + sizeof(udword) * reg->nPalettes));
    }
}

/*-----------------------------------------------------------------------------
    Name        : trColorsEqual
    Description : See if there is a version of this texture with already
                    matching palette.
    Inputs      : info - color info requested for this texture
                  textureIndex - index of texture to check for matches
    Outputs     : Examines all palettes associated with this texture for a match with info.
    Return      : index of matching palette or -1 if no match
----------------------------------------------------------------------------*/
sdword trColorsEqual(trcolorinfo *info, sdword textureIndex)
{
    sdword index;
    trcolorinfo *textureColor;

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        return(-1);
    }
#endif
    dbgAssert(info != NULL);
    dbgAssert(trTextureRegistry[textureIndex].palettes != NULL);

    if (info == &trDefaultColorInfo)
    {
        return(0);                                          //return 0 for default palettes
    }
    textureColor = trColorInfosPointer(textureIndex);
    //textureColor = (trcolorinfo *)trTextureRegistry[textureIndex].palettes;//!!! <- does this work for non-pending textures
    for (index = 0; index < trTextureRegistry[textureIndex].nPalettes; index++, textureColor++)
    {                                                       //for all palettes in this texture
        if (trUnusedInfo(textureColor))
        {                                                   //if this slot not in use
            continue;                                       //skip it
        }
        if (info->base == textureColor->base &&             //if base/detail match
            info->detail == textureColor->detail)
        {
            return(index);                                  //this is the palette we want
        }
    }
    return(-1);                                             //no matches found
}

/*-----------------------------------------------------------------------------
    Name        : trAllColorsEqual
    Description : See if the color allocations to a pair of texreg structures
                    are the same.
    Inputs      : index0, 1 - indeces to compare.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool trAllColorsEqual(sdword index0, sdword index1)
{
    sdword index;
    trcolorinfo *textureColor0;
    trcolorinfo *textureColor1;

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        return(FALSE);
    }
#endif
    //dbgAssert(trPending(index0));
    //dbgAssert(trPending(index1));
    dbgAssert(index0 != index1);
    dbgAssert(index0 <= trHighestAllocated);
    dbgAssert(index1 <= trHighestAllocated);
    if (trTextureRegistry[index0].nPalettes != trTextureRegistry[index1].nPalettes)
    {
        return(FALSE);
    }
    textureColor0 = trColorInfosPointer(index0);//(trcolorinfo *)trTextureRegistry[index0].palettes;
    textureColor1 = trColorInfosPointer(index1);//(trcolorinfo *)trTextureRegistry[index1].palettes;
    for (index = 0; index < trTextureRegistry[index0].nPalettes; index++, textureColor0++, textureColor1++)
    {                                                       //for all palettes in this texture
        if (textureColor0->base != textureColor1->base ||   //if base/detail differ
            textureColor0->detail != textureColor1->detail)
        {
            return(FALSE);                                  //we must discard the whole batch
        }
    }
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : trColorsIndexAlloc
    Description : Allocate a new colorinfo for a pending texture load
    Inputs      : info - what color we need
                  textureIndex - what texture to allocate it for
    Outputs     : puts the new info into the color info list in a free spot if
                    possible, or allocates a new slot.
    Return      :
    Note        : Should already have called trColorsEqual to verify this
                    color scheme not already there.
----------------------------------------------------------------------------*/
sdword trColorsIndexAlloc(trcolorinfo *info, sdword textureIndex)
{
    sdword index;
    texreg *reg = &trTextureRegistry[textureIndex];
    trcolorinfo *textureColor;

    textureColor = trColorInfosPointer(textureIndex);
    for (index = 0; index < reg->nPalettes; index++, textureColor++)
    {
        if (trUnusedInfo(textureColor))
        {                                                   //if this slot free
            *textureColor = *info;                          //put the color into the last slot
            return(index);                                  //use it
        }
    }
    //no slot found, allocate a new one
    dbgAssert(reg->nPalettes < TR_NumPalettesPerTexture);   //make sure there is room
    *textureColor = *info;                                  //put the color into the last slot
    reg->nPalettes++;                                       //one more palette now
    return(reg->nPalettes - 1);
}

/*-----------------------------------------------------------------------------
    Name        : trFindTextureIndexByName
    Description : Find the handle for a given texture name, if it has been registered.
    Inputs      : fileName - name of texture to find.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword trFindTextureIndexByName(char *fileName)
{
    sdword index;
    sdword length;
    crc32 nameCRC, *regCRC;
    texreg *reg;

    strupr(fileName);                                       //!!! is this going to screw anything up?
    length = strlen(fileName);
    dbgAssert(length > 0);

    nameCRC = crc32Compute((ubyte*)fileName, length);               //compute a name for the CRC

    reg = &trTextureRegistry[trHighestAllocated];
    regCRC = &trNameCRCs[trHighestAllocated];
    for (index = trHighestAllocated; index >= 0 ; index--, reg--, regCRC--)
    {                                                       //for all registry entries
        if (*regCRC == nameCRC)
        {                                                   //if names have matching CRC's
            if (!trAllocated(index))
            {                                               //if not allocated
                continue;                                   //skip it
            }
            if (!strcmp(reg->fileName, fileName))
            {                                               //and they match names
                return(index);
            }
        }
    }
    return(TR_NotShared);                                   //no index found; error
}

/*-----------------------------------------------------------------------------
    Name        : trTextureRegister
    Description : Register a texture for later loading
    Inputs      : fileName - path and name of texture to load
                  info - color information for colorizing texture or NULL if
                    texture is not to be colored.
                  meshReference - pointer to the mesh requesting texture.
                    Only used for colorization.
    Outputs     : Allocates and fills in an entry in the texture registry.
    Return      : Handle to registered texture.
----------------------------------------------------------------------------*/
trhandle trTextureRegister(char *fileName, trcolorinfo *info, void *meshReference)
{
    sdword index, paletteIndex, length;
    crc32 nameCRC, *regCRC;
    texreg *reg;

    strupr(fileName);                                       //!!! is this going to screw anything up?
    length = strlen(fileName);
    dbgAssert(length > 0);

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        return(0);
    }
#endif
#if TR_VERBOSE_LEVEL >= 2
    dbgMessagef("\ntrTextureRegister: registering '%s', info 0x%x", fileName, info);
#endif  //TR_VERBOSE_LEVEL
    if (info == NULL)
    {
        info = &trDefaultColorInfo;                         //default colorInfo on NULL
    }

    nameCRC = crc32Compute((ubyte*)fileName, length);               //compute a name for the CRC

    reg = &trTextureRegistry[trHighestAllocated];
    regCRC = &trNameCRCs[trHighestAllocated];
    for (index = trHighestAllocated; index >= 0 ; index--, reg--, regCRC--)
    {                                                       //for all registry entries
        if (*regCRC == nameCRC)
        {                                                   //if names have matching CRC's
            if (!trAllocated(index))
            {                                               //if not allocated
                continue;                                   //skip it
            }
            if (!strcmp(reg->fileName, fileName))
            {                                               //and they match names
                if ((paletteIndex = trColorsEqual(info, index)) >= 0)
                {                                           //and they're the same color
                    //... use this texture
                    dbgAssert(reg->nUsageCount < SDWORD_Max);
                    reg->nUsageCount++; //update usage count
#if TR_VERBOSE_LEVEL >= 2
                    dbgMessagef("\ntrTextureRegister: texture handle 0x%x nUsageCount incremented to %d", index, reg->nUsageCount);
#endif  //TR_VERBOSE_LEVEL
                    return(trHandleMake(index, paletteIndex));//and use this texture
                }
                else
                {                                           //colors do not match
                    //!!! create a new paletted copy
                    //... if it is a pending texture
                    dbgAssert(reg->nUsageCount < SDWORD_Max);
                    reg->nUsageCount++; //update usage count
#if TR_VERBOSE_LEVEL >= 2
                    dbgMessagef("\ntrTextureRegister: texture handle 0x%x nUsageCount incremented to %d", index, reg->nUsageCount);
#endif  //TR_VERBOSE_LEVEL
                    /*
                    ((trcolorinfo *)reg->palettes)
                        [reg->nPalettes] = *info;//retain reference to the color info
                    reg->nPalettes++;//update palette index
                    dbgAssert(reg->nPalettes <= TR_NumPalettesPerTexture);
                    */
                    paletteIndex = trColorsIndexAlloc(info, index);
                    return(trHandleMake(index, paletteIndex));
                }
            }
        }
    }
    //if we get here, no matching texture was found; allocate a
    //registry structure and load in texture
    for (index = trLowestFree; index < trRegistrySize; index++)
    {                                                       //search for a free structure
        if (!trAllocated(index))
        {                                                   //if free structure
#if TR_VERBOSE_LEVEL >= 2
            dbgMessagef("\ntrTextureRegister: allocated index 0x%x for texture '%s'.", index, fileName);
#endif  //TR_VERBOSE_LEVEL
            // Notice how we just say that all textures are paletted?
            //  Well, we may want to put in a check for some other condition
            //  which forces the texture to be RGB, say for inclusion of an
            //  alpha channel or something.
            /*
            if (!loadAlpha)
            {
                bitSet(trTextureRegistry[index].flags, TRF_Paletted);
            }
            else
            {
                bitSet(trTextureRegistry[index].flags, TRF_Alpha);
            }
            */
            trTextureRegistry[index].meshReference = meshReference;//texture's parent mesh
            trTextureRegistry[index].nUsageCount = 1;       //one usage of this texture
            trNameCRCs[index] = nameCRC;                    //save the name CRC
            trTextureRegistry[index].fileName = memAlloc(length + 1, "NameTex", NonVolatile);
            strcpy(trTextureRegistry[index].fileName, fileName);
            trTextureRegistry[index].nPalettes = 1;         //one palette to start
            trTextureRegistry[index].currentPalette = -1;   //no palette yet registered
            trTextureRegistry[index].paletteCRC = TR_BadCRC;
            trTextureRegistry[index].palettes =             //allocate the temp palette list
                memAlloc(TR_NumPaletteSize, "Temp trcolorinfo list", 0);
            memClearDword(trTextureRegistry[index].palettes, TR_UnusedColorInfo, TR_NumPaletteSize / sizeof(udword));
            trTextureRegistry[index].baseScalar = (uword)colRealToUdword(trBaseColorScalar);
            trTextureRegistry[index].stripeScalar = (uword)colRealToUdword(trStripeColorScalar);
            trTextureRegistry[index].handle = TR_InvalidInternalHandle;
            dbgAssert(info != NULL);
            *((trcolorinfo *)trTextureRegistry[index].palettes) = *info;
            trSetPending(index);                            //texture is pending
            trSetAllocated(index);                          //and in use
            trHighestAllocated = max(trHighestAllocated, index);//update highest allocated
            if (index == trLowestFree)
            {                                               //if allocating lowest free
                for (; trLowestFree < trRegistrySize; trLowestFree++)
                {                                           //search for lowest free structure
                    if (!trAllocated(trLowestFree))
                    {                                       //if structure free
                        break;                              //done
                    }
                }
            }
            return(trHandleMake(index, 0));
        }
    }
#if TR_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "\ntrTextureRegister: unable to allocate texture '%s' from list of %d entries", fileName, trRegistrySize);
#endif //TR_ERROR_CHECKING
    //should never get here
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : trRegisterAddition
    Description : Register an 'addition' to a texture.  This means add a color
                    that is not already there.
    Inputs      : handle - handle of texture already registered
                  info - color to register
    Outputs     :
    Return      : new handle
----------------------------------------------------------------------------*/
trhandle trRegisterAddition(trhandle handle, trcolorinfo *info)
{
    texreg *reg;
    sdword colorIndex, index = trIndex(handle);
    trhandle newHandle;
    trcolorinfo *oldColorInfos, *newColorInfos;

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        return(0);
    }
#endif

    dbgAssert(trAllocated(index));

    reg = trStructure(handle);

    colorIndex = trColorsEqual(info, index);

    //sharing handled
    if (trPending(index) || reg->sharedFrom != TR_NotShared)
    {                                                       //if texture not loaded yet
        if (colorIndex >= 0)
        {                                                   //if these guys are the same color
            newHandle = trHandleMake(index, colorIndex);    //use existing handle
        }
        else
        {                                                   //else this color not registered yet
            colorIndex = trColorsIndexAlloc(info, index);
            newHandle = trHandleMake(index, colorIndex);
        }
    }
    else
    {   //... kill the current textures to load from scratch ...

        //get list of old colors
        /*
        if (bitTest(reg->flags, TRF_Paletted))
        {                                                   //if it's a paletted texture
            oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + TR_PaletteSize * reg->nPalettes);
        }
        else
        {                                                   //else's it's an RGB texture
            oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + sizeof(udword) * reg->nPalettes);
        }
        */
        oldColorInfos = trColorInfosPointer(index);
        if (colorIndex >= 0 &&
            oldColorInfos[colorIndex].base == info->base &&
            oldColorInfos[colorIndex].detail == info->detail)
        {                                                   //if the colors match
            newHandle = trHandleMake(index, colorIndex);
        }
        else
        {                                                   //if either color has changed
    #if TR_VERBOSE_LEVEL >= 2
            dbgMessagef("\ntrRegisterAddition: updating texture handle 0x%x", handle);
    #endif  //TR_VERBOSE_LEVEL
            //make duplicate copy of the colorinfo structures
            newColorInfos = memAlloc(TR_NumPaletteSize, "dupe trcolorinfo list", NonVolatile);
            memcpy(newColorInfos, oldColorInfos, sizeof(trcolorinfo) * reg->nPalettes);

            //kill the GL textures and palettes
            trInternalTexturesDelete(handle);

            reg->palettes = (ubyte *)newColorInfos;         //save list of color infos
            trSetPending(index);                            //make this texture pending, to be re-loaded
            colorIndex = trColorsIndexAlloc(info, index);
            newHandle = trHandleMake(index, colorIndex);
        }
    }
    dbgAssert(reg->nUsageCount < SDWORD_Max);
    reg->nUsageCount++;                                     //update usage count
#if TR_VERBOSE_LEVEL >= 2
    dbgMessagef("\ntrTextureRegister: texture handle 0x%x nUsageCount incremented to %d", index, reg->nUsageCount);
#endif  //TR_VERBOSE_LEVEL
    return(newHandle);
}

/*-----------------------------------------------------------------------------
    Name        : trRegisterRemoval
    Description : Remove a particular color from a texture.
    Inputs      : handle - handle we no longer need
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trRegisterRemoval(trhandle handle)
{
    texreg *reg = trStructure(handle);
    sdword index = trIndex(handle);
    sdword colorIndex = trPaletteIndex(handle);//, infoIndex;
    trcolorinfo *oldColorInfos, *newColorInfos;

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        return;
    }
#endif
    dbgAssert(trAllocated(index));

    if (reg->nUsageCount == 0)
    {                                                       //if they've all been deleted already
        return;                                             //nothing to do
    }
    //sharing handled
    if (trPending(index) || reg->sharedFrom != TR_NotShared)
    {                                                       //if the texture has not yet been loaded
        ((trcolorinfo *)reg->palettes)[colorIndex] = trUnusedColorInfo;//make this default colors (no longer needed)
    }
    else
    {                                                       //else texture is already loaded
        //get list of old colors
        /*
        if (bitTest(reg->flags, TRF_Paletted))
        {                                                   //if it's a paletted texture
            oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + TR_PaletteSize * reg->nPalettes);
        }
        else
        {                                                   //else's it's an RGB texture
            oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + sizeof(udword) * reg->nPalettes);
        }
        */
        oldColorInfos = trColorInfosPointer(index);
        //make duplicate copy of the colorinfo structures
        newColorInfos = memAlloc(TR_NumPaletteSize, "dupe trcolorinfo list", NonVolatile);
        memcpy(newColorInfos, oldColorInfos, sizeof(trcolorinfo) * reg->nPalettes);
        newColorInfos[colorIndex] = trUnusedColorInfo;      //clear this color info

        //kill the GL textures and palettes
        trInternalTexturesDelete(handle);

        //now set the new colorinfo structures into the texreg
        reg->palettes = (ubyte *)newColorInfos;
        trSetPending(index);
    }
    reg->nUsageCount--;                                     //one less reference to this texture
}

/*-----------------------------------------------------------------------------
    Name        : trTextureColorsUpdate
    Description : Update the color of a given texture, flagging it for re-
                    loading if the colors have changed.
    Inputs      : handle - handle of already loaded texture
                  info - new colors
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trTextureColorsUpdate(trhandle handle, trcolorinfo *info)
{
    texreg *reg;
    trcolorinfo *oldColorInfos, *newColorInfos;
    sdword colorIndex;

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        return;
    }
#endif
    reg = trStructure(handle);
    colorIndex = trPaletteIndex(handle);

    //first, let's see if this texture is already loaded.  That's the
    //only time it makes sense.
    //sharing handled
    if (trPending(trIndex(handle)) || reg->sharedFrom != TR_NotShared)
    {                                                       //if texture not yet loaded
        newColorInfos = (trcolorinfo *)reg->palettes;       //get list of colors already allocated
        newColorInfos[colorIndex] = *info;                  //set new color
    }
    else
    {
        //get list of old colors
        /*
        if (bitTest(reg->flags, TRF_Paletted))
        {                                                   //if it's a paletted texture
            oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + TR_PaletteSize * reg->nPalettes);
        }
        else
        {                                                   //else's it's an RGB texture
            oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + sizeof(udword) * reg->nPalettes);
        }
        */
        oldColorInfos = trColorInfosPointer(trIndex(handle));
        if (oldColorInfos[colorIndex].base != info->base ||
            oldColorInfos[colorIndex].detail != info->detail)
        {                                                   //if either color has changed
    #if TR_VERBOSE_LEVEL >= 2
            dbgMessagef("\ntrTextureColorsUpdate: updating texture handle 0x%x", handle);
    #endif  //TR_VERBOSE_LEVEL
            //make duplicate copy of the colorinfo structures
            newColorInfos = memAlloc(TR_NumPaletteSize, "dupe trcolorinfo list", NonVolatile);
            memcpy(newColorInfos, oldColorInfos, sizeof(trcolorinfo) * reg->nPalettes);

            //kill the GL textures and palettes
            trInternalTexturesDelete(handle);

            newColorInfos[colorIndex] = *info;              //set new color
            reg->palettes = (ubyte *)newColorInfos;         //save list of color infos
            trSetPending(trIndex(handle));                  //make this texture pending, to be re-loaded
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : trTextureUnregister
    Description : 'Unregisters' a texture. i.e. decrements it's usage count.
    Inputs      : handle - handle of texture whose usage count is to be decremented.
    Outputs     : Decrements usage count.  Does not delete texture.
    Return      : New usage count for texture.
    Note        : All texture no longer in use will be freed up by either
                    trRegistryRefresh or trTextureDeleteAllUnregistered.
----------------------------------------------------------------------------*/
sdword trTextureUnregister(trhandle handle)
{
    sdword index = trIndex(handle);
    sdword paletteIndex = trPaletteIndex(handle);

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        return(0);
    }
#endif
    //sharing handled - nothing special here...
    dbgAssert(trAllocated(index));
    trTextureRegistry[index].nUsageCount--;
    if (trPending(index))
    {                                                       //if texture not yet loaded
        ((trcolorinfo *)trTextureRegistry[index].palettes)[paletteIndex] = trUnusedColorInfo;//this palette not needed anymore
    }
#if TR_VERBOSE_LEVEL >= 2
    dbgMessagef("\ntrTextureUnregister: handle 0x%x nUsageCount decremented to %d", handle, trTextureRegistry[index].nUsageCount);
#endif  //TR_VERBOSE_LEVEL
    return(trTextureRegistry[index].nUsageCount);
}

/*-----------------------------------------------------------------------------
    Name        : trTextureDeleteAllUnregistered
    Description : Delete all texture unregistered to <= 0 usage count
    Inputs      : void
    Outputs     : Deletes all texture no longer needed
    Return      : void
----------------------------------------------------------------------------*/
void trTextureDeleteAllUnregistered(void)
{
    sdword index;

#if TR_VERBOSE_LEVEL >= 1
    dbgMessagef("\ntrTextureDeleteAllUnregistered");
#endif  //TR_VERBOSE_LEVEL
    for (index = 0; index <= trHighestAllocated; index++)
    {                                                       //for all textures in allocated range
        if (trAllocated(index))
        {                                                   //if texture allocated
            if (trTextureRegistry[index].nUsageCount <= 0)
            {                                               //if texture no longer in use
                trTextureDelete(index);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : trMeshSortAdd
    Description : Add a reference to this texture in the mesh-sorted list.
    Inputs      : regIndex - index into texture registry of texture
                : teamFlags - falgs for team effect from the image measurment.
    Outputs     : Adds to mesh-sorted list.
    Return      : void
----------------------------------------------------------------------------*/
void trMeshSortAdd(sdword regIndex, udword teamFlags)
{
    sdword index, j;
    void *meshReference = trStructure(regIndex)->meshReference;
    trcolorinfo *palettes, *otherPalettes;
    crc32 paletteCRC;
    sdword paletteSize, otherPaletteSize;

    palettes = trColorInfosPointer(regIndex);
    paletteSize = sizeof(trcolorinfo) * trStructure(regIndex)->nPalettes;
    dbgAssert(paletteSize > 0);
    paletteCRC = crc32Compute((ubyte *)palettes, paletteSize);

    for (index = 0; index < trMeshSortIndex; index++)
    {                                                       //search through list
        if (trMeshSortList[index].meshReference == meshReference && trMeshSortList[index].paletteCRC == paletteCRC)
        {                                                   //if matching mesh reference and palette CRC
            otherPaletteSize = sizeof(trcolorinfo) * trStructure(trMeshSortList[index].textureList[0])->nPalettes;
            dbgAssert(otherPaletteSize > 0);
            if (paletteSize == otherPaletteSize)
            {                                               //if they have the same numper of palettes
                otherPalettes = trColorInfosPointer(trMeshSortList[index].textureList[0]);
                if (!memcmp((void *)palettes, (void *)otherPalettes, paletteSize))
                {                                           //and the actual palettes are the same (just making sure they're the same color)
#if TR_ERROR_CHECKING
                    for (j = 0; j < trMeshSortList[index].nTextures; j++)
                    {                                       //verify this texture not already referenced
                        dbgAssert(trMeshSortList[index].textureList[j] != regIndex);
                    }
#endif
                    if (trMeshSortList[index].nTextures >= trMeshSortList[index].nAllocated)
                    {                                       //grow this texture list
                        trMeshSortList[index].nAllocated += TR_SortListAllocationStep;
                        trMeshSortList[index].textureList = memRealloc(trMeshSortList[index].textureList,
                                trMeshSortList[index].nAllocated * sizeof(sdword),
                                "TextureMeshSortSubList", 0);
                    }
                    trMeshSortList[index].textureList[trMeshSortList[index].nTextures] = regIndex;
                    trMeshSortList[index].nTextures++;      //store new texture reference
                    trMeshSortList[index].teamFlags |= teamFlags & (TRF_TeamColor0 | TRF_TeamColor1);
                    return;
                }
            }
        }
    }
    //if it gets here, no matching reference was found, we need to create a new one
    //grow the texture sort list if needed
    if (index >= trMeshSortLength)
    {                                                       //if the sort list too short
        dbgAssert(index == trMeshSortLength);               //should never be over
        trMeshSortList = memRealloc(trMeshSortList, (trMeshSortLength + TR_MeshSortGrowBy) * sizeof(trmeshsort), "Texture Mesh Sort List", 0);
        for (j = index; j < index + TR_MeshSortGrowBy; j++)
        {
            trMeshSortList[j].meshReference = NULL;         //no mesh reference yet
            trMeshSortList[j].nAllocated = TR_SortListAllocationStep;//starting amount of space for textures
            trMeshSortList[j].nTextures = 0;                //no textures in list yet
            trMeshSortList[j].teamFlags = 0;                //default is no team colors
            trMeshSortList[j].textureList =                 //allocate our local list
                memAlloc(TR_SortListAllocationStep * sizeof(sdword),
                         "TextureMeshSortSubList", 0);
        }
        trMeshSortLength += TR_MeshSortGrowBy;
    }
    trMeshSortList[index].meshReference = meshReference;    //use new mesh-sort structure
    trMeshSortList[index].paletteCRC = paletteCRC;
    trMeshSortList[index].textureList[0] = regIndex;
    trMeshSortList[index].nTextures = 1;
    trMeshSortList[index].teamFlags |= teamFlags & (TRF_TeamColor0 | TRF_TeamColor1);
    trMeshSortIndex++;
}

/*=============================================================================
    Following are a set of texture for directly creating and manipulating
        textures of different types.  These functions entirely circumvent the
        texture registry.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : trCreateUnpalettedTexture
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trCreateUnpalettedTexture(ubyte* palette, ubyte* data, sdword width, sdword height)
{
    ubyte* sp;
    ubyte* dp;
    sdword i, index;
    ubyte* rgba;

    rgba = memAlloc(4 * width * height, "temp rgba", Pyrophoric);

    dp = rgba;
    sp = data;
    for (i = 0; i < width*height; i++, dp += 4, sp++)
    {
        index = (*sp) << 2;
        dp[0] = palette[index + 0];
        dp[1] = palette[index + 1];
        dp[2] = palette[index + 2];
        dp[3] = palette[index + 3];
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

    memFree(rgba);
}

/*-----------------------------------------------------------------------------
    Name        : trPalettedTextureCreate
    Description : Create a new texture object handle from some indexed data and
                    a palette.
    Inputs      : data - indexed-mode image to make texture of
                  palette - (one of the) palettes to go with image
                  width/height - dimensions of image
    Outputs     : Creats a texture object and downloads it
    Return      : new texture handle
----------------------------------------------------------------------------*/
udword trPalettedTextureCreate(ubyte *data, color *palette, sdword width, sdword height)
{
    udword newHandle;
#if TR_ERROR_CHECKING
    udword internalWidth;
#endif
    ubyte* tempData;
#if TR_ASPECT_CHECKING
    sdword oldWidth, oldHeight;
#endif

    tempData = NULL;

    glGenTextures(1, (GLuint*)&newHandle);                           //create a texture name
    primErrorMessagePrint();
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, newHandle);                //bind texture for modification
    primErrorMessagePrint();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (texLinearFiltering)
    {                                                       //set min/mag filters to point samplingor linear
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    primErrorMessagePrint();
    //first see if this texture can fit in RAM.  It should because we've already packed the textures
    if (!RGL)
    {
#if TR_ASPECT_CHECKING
        if ((width / height) > 8 || (height / width) > 8)
        {
            dbgWarningf(DBG_Loc, "\ntrPalettedTextureCreate: aspect overflow in texture of size %d x %d", width, height);

            oldWidth  = width;
            oldHeight = height;
            while ((width / height) > 8)
            {
                width >>= 1;
            }
            while ((height / width) > 8)
            {
                height >>= 1;
            }
            tempData = trImageScaleIndexed(data, oldWidth, oldHeight, width, height, FALSE);
            data = tempData;
        }
#endif //TR_ASPECT_CHECKING
#if TR_ERROR_CHECKING
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, TR_PaletteType, width,
                     height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, NULL);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &internalWidth);
        if (internalWidth == 0)
        {
            dbgFatalf(DBG_Loc, "\ntrPalettedTextureCreate: unable to create proxy texture size %d x %d", width, height);
        }
        primErrorMessagePrint();
#endif //TR_ERROR_CHECKING
    }

    if (trNoPalettes)
    {
        trCreateUnpalettedTexture((ubyte*)palette, data, width, height);
    }
    else
    {
        trUpdate = TRUE;
        trColorTable(palette);
        primErrorMessagePrint();                                //new palette downloaded
        glTexImage2D(GL_TEXTURE_2D, 0, TR_PaletteType, width,   //create the GL texture object
                     height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, data);
    }
    if (tempData != NULL)
    {
        memFree(tempData);
    }
    primErrorMessagePrint();

    return(newHandle);                                      //return new handle
}

/*-----------------------------------------------------------------------------
    Name        : trRGBTextureCreate
    Description : Creates a new RGB texture object based upon the specified RGB image.
    Inputs      : data - RGBA data to create texture from
                  width/height - size of image
    Outputs     : ..
    Return      : new texture handle
    Note        : textures created in this manner should only be deleted using
                    the following function and made current by the function
                    after that.
----------------------------------------------------------------------------*/
udword trRGBTextureCreate(color *data, sdword width, sdword height, bool useAlpha)
{
    udword newHandle, destType;
    color* tempData;
#if TR_ASPECT_CHECKING
    sdword oldWidth, oldHeight;
#endif

    tempData = NULL;

    if (useAlpha)
    {
        destType = TR_RGBAType;
    }
    else
    {
        destType = TR_RGBType;
    }
    glGenTextures(1, (GLuint*)&newHandle);                           //create a texture name
    primErrorMessagePrint();
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, newHandle);                //bind texture for modification
    primErrorMessagePrint();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (texLinearFiltering)
    {                                                       //set min/mag filters to point samplingor linear
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    //first see if this texture can fit in RAM.  It should because we've already packed the textures
#if TR_ASPECT_CHECKING
    if (!RGL)
    {
        if (width / height > 8 || height / width > 8)
        {
            dbgWarningf(DBG_Loc, "\ntrRGBTextureCreate: aspect overflow in texture of size %d x %d", width, height);

            oldWidth  = width;
            oldHeight = height;
            while ((width / height) > 8)
            {
                width >>= 1;
            }
            while ((height / width) > 8)
            {
                height >>= 1;
            }
            tempData = trImageScale(data, oldWidth, oldHeight, width, height, FALSE);
            data = tempData;
        }
    }
#endif //TR_ASPECT_CHECKING
    glTexImage2D(GL_TEXTURE_2D, 0, destType, width,       //create the GL texture object
                 height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    if (tempData != NULL)
    {
        memFree(tempData);
    }
    primErrorMessagePrint();
    return(newHandle);                                      //return new handle
}

/*-----------------------------------------------------------------------------
    Name        : trRGBTextureUpdate
    Description : Update a previously created texture.
    Inputs      : handle - handle of previously created texture
                  data,width,height - same as for creation
    Outputs     : copies data to the texture and makes texture current
    Return      : void
----------------------------------------------------------------------------*/
void trRGBTextureUpdate(udword handle, color *data, sdword width, sdword height)
{
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, handle);                   //bind texture for modification
    primErrorMessagePrint();
    glTexImage2D(GL_TEXTURE_2D, 0, TR_RGBType, width,       //create the GL texture object
                 height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}
/*-----------------------------------------------------------------------------
    Name        : trRGBTextureDelete
    Description : Delete a texture which was created using trRGBTextureDelete
    Inputs      : handle - handle of texture returned from trRGBTextureCreate
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void trRGBTextureDelete(udword handle)
{
    glDeleteTextures(1, (GLuint*)&handle);
    primErrorMessagePrint();
}

/*-----------------------------------------------------------------------------
    Name        : trRGBTextureMakeCurrent
    Description : Make an RGB texture current
    Inputs      : handle - handle of texture to make current
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void trRGBTextureMakeCurrent(udword handle)
{
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, handle);
    primErrorMessagePrint();
}

/*-----------------------------------------------------------------------------
    Name        : trPalettedTextureMakeCurrent
    Description : Make an RGB texture current
    Inputs      : handle - handle of texture to make current
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void trPalettedTextureMakeCurrent(udword handle, color *palette)
{
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, handle);
    trUpdate = TRUE;
    trColorTable(palette);
    primErrorMessagePrint();
}

/*-----------------------------------------------------------------------------
    Name        : trScaleDown
    Description : Scale a single scanline of an image down by varying amounts.
    Inputs      : dest - destination scanline
                  source - source scanline
                  width - width of destination buffer
                  scalefactor - amount we're scaling down by
    Outputs     : copies source pixels to dest
    Return      : void
----------------------------------------------------------------------------*/
void trScaleDown(color *dest, color *source, sdword width, sdword scaleFactor)
{
    while (width > 0)
    {
        *dest = *source;
        width--;
        dest++;
        source += scaleFactor;
    }
}

/*-----------------------------------------------------------------------------
    Name        : trScaleArbitrary
    Description : Scale a single scanline of an image .
    Inputs      : dest - destination scanline
                  source - source scanline
                  width - width of destination buffer
                  scalefactor - amount we're scaling by (64k = 1.0)
    Outputs     : copies source pixels to dest
    Return      : void
----------------------------------------------------------------------------*/
void trScaleArbitrary(color *dest, color *source, sdword width, sdword scaleFactor)
{
    udword accumulator = 0;

    while (width > 0)
    {
        *dest = source[accumulator >> 16];
        dest++;
        width--;
        accumulator += scaleFactor;
    }
}

/*-----------------------------------------------------------------------------
    Name        : trImageScale
    Description : Scales an image down to a new size.
    Inputs      : data - RGBA buffer to be scaled down
                  width/height - size of image
                  newWidth/newHeight - size to scale to must be <= width/height,
                        even exponent of 2, >= 4
                  bFree - if true, the source image will be freed after scaling
                    if false, image will not be freed at all;  Memory will always
                    be allocated.
    Outputs     : Allocates new buffer and frees old one.
    Return      : Newly allocated and scaled buffer.
----------------------------------------------------------------------------*/
color *trImageScale(color *data, sdword width, sdword height, sdword newWidth, sdword newHeight, bool bFree)
{
    color *newBuffer;
    sdword index, scaleX, scaleY;
    udword accumulator, srcY;

    dbgAssert(newWidth <= width && newHeight <= height);    //verify the preconditions
    dbgAssert(newWidth >= TR_MinimumSizeX && newHeight >= TR_MinimumSizeY);
    if (newWidth == width && newHeight == height)
    {                                                       //if new size same as old size
        if (bFree)
        {
            return(data);                                       //done
        }
        else
        {
            newBuffer = memAlloc(sizeof(color) * newWidth * newHeight,
                                 "trImageScaled buffer", 0);
            memcpy(newBuffer, data, sizeof(color) * newWidth * newHeight);
            return(newBuffer);
        }
    }
    newBuffer = memAlloc(sizeof(color) * newWidth * newHeight,
                         "trImageScaled buffer", 0);

    if (bitNumberSet(width, 16) != 1 || bitNumberSet(height, 16) != 1)
    {                                                       //arbitrary scale
        scaleX = (width << 16) / newWidth;
        scaleY = (height << 16) / newHeight;
        accumulator = 0;

        for (index = 0; index < newHeight; index++)
        {
            srcY = accumulator >> 16;
            trScaleArbitrary(newBuffer + newWidth * index, data + width * srcY, newWidth, scaleX);
            accumulator += scaleY;
        }
    }
    else
    {
        scaleX = width / newWidth;
        scaleY = height / newHeight;

        for (index = 0; index < newHeight; index++)
        {                                                       //for each new scanline
            trScaleDown(newBuffer + newWidth * index,           //scale the scanline down
                     data + width * index * scaleY, newWidth, scaleX);
        }
    }
    if (bFree)
    {
        memFree(data);                                          //free the old color buffer
    }
    return(newBuffer);                                      //and allocate the new one
}

/*-----------------------------------------------------------------------------
    Name        : trLIFFileLoad
    Description : Load in a .lif file and fix up it's pointers
    Inputs      : fileName - name of file to load
                  flags - load volatile?
    Outputs     :
    Return      : pointer to the header of newly allocated file.
    Note        : the header of this image can be memfreed as it's all 1 big block
----------------------------------------------------------------------------*/
lifheader *trLIFFileLoad(char *fileName, udword flags)
{
    lifheader *newHeader;

    fileLoadAlloc(fileName, (void**)&newHeader, flags);             //load in the .LiF file
#if TR_ERROR_CHECKING
    if (strcmp(newHeader->ident, LIF_FileIdentifier))
    {                                                       //verify proper file
        dbgFatalf(DBG_Loc, "Incorrect file identifier in '%s'.", fileName);
    }
    if (newHeader->version != LIF_FileVersion)
    {
        dbgFatalf(DBG_Loc, "File '%s' is version 0x%x instead of 0x%x", fileName, newHeader->version, LIF_FileVersion);
    }
    /*
    if (newHeader->width > TR_TextureWidthMax || newHeader->height > TR_TextureHeightMax)
    {
        dbgFatalf(DBG_Loc, "Texture '%s' too big ( %dx%d > %dx%d ).", fileName, newHeader->width, newHeader->height, TR_TextureWidthMax, TR_TextureHeightMax);
    }
    */
#endif
    //fixup pointers in the header
    (ubyte *)newHeader->data += (udword)newHeader;
    (ubyte *)newHeader->palette += (udword)newHeader;
    (ubyte *)newHeader->teamEffect0 += (udword)newHeader;
    (ubyte *)newHeader->teamEffect1 += (udword)newHeader;
    return(newHeader);
}

/*-----------------------------------------------------------------------------
    Name        : trScaleDownIndexed
    Description : Scale a single scanline of an image down by varying amounts.
    Inputs      : dest - destination scanline
                  source - source scanline
                  width - width of destination buffer
                  scalefactor - amount we're scaling down by
    Outputs     : copies source pixels to dest
    Return      : void
----------------------------------------------------------------------------*/
void trScaleDownIndexed(ubyte *dest, ubyte *source, sdword width, sdword scaleFactor)
{
    while (width > 0)
    {
        *dest = *source;
        width--;
        dest++;
        source += scaleFactor;
    }
}

/*-----------------------------------------------------------------------------
    Name        : trImageScaleIndexed
    Description : Scales an image down to a new size.
    Inputs      : data - RGBA buffer to be scaled down
                  width/height - size of image
                  newWidth/newHeight - size to scale to must be <= width/height,
                        even exponent of 2, >= 4
                  bFree - if TRUE, the passed data will be freed if scaling performed.
                    If false, memory will be allocated no matter if the size is the same.
    Outputs     : Allocates new buffer and frees old one.
    Return      : Newly allocated and scaled buffer.
----------------------------------------------------------------------------*/
ubyte *trImageScaleIndexed(ubyte *data, sdword width, sdword height, sdword newWidth, sdword newHeight, bool bFree)
{
    ubyte *newBuffer;
    sdword index, scaleX, scaleY;

    dbgAssert(newWidth <= width && newHeight <= height);    //verify the preconditions
    dbgAssert(newWidth >= TR_MinimumSizeX && newHeight >= TR_MinimumSizeY);
    dbgAssert(bitNumberSet(width, 16) == 1 && bitNumberSet(height, 16) == 1);
    if (newWidth == width && newHeight == height)
    {                                                       //if new size same as old size
        if (bFree)
        {
            return(data);
        }
        else
        {
            newBuffer = memAlloc(newWidth * newHeight,
                                 "trImageScaleIndexed buffer", Pyrophoric);
            memcpy(newBuffer, data, width * height);
            return(newBuffer);
        }
    }

    newBuffer = memAlloc(newWidth * newHeight,
                         "trImageScaleIndexed buffer", Pyrophoric);
    scaleX = width / newWidth;
    scaleY = height / newHeight;

    for (index = 0; index < newHeight; index++)
    {                                                       //for each new scanline
        trScaleDownIndexed(newBuffer + newWidth * index,           //scale the scanline down
                 data + width * index * scaleY, newWidth, scaleX);
    }
    if (bFree)
    {
        memFree(data);                                          //free the old color buffer
    }
    return(newBuffer);                                      //and allocate the new one
}

/*-----------------------------------------------------------------------------
    Name        : trTextureHandleScale
    Description : Takes a GL internal texture handle, scales it to a new size
                    and creates a new texture.
    Inputs      : handle - handle of existing texture
                  width, height - size of existing texture
                  newWidth, newHeight - size to scale it to
    Outputs     :
    Return      : new texture handle
----------------------------------------------------------------------------*/
udword trTextureHandleScale(udword handle, sdword width, sdword height, sdword newWidth, sdword newHeight, bool bUseAlpha)
{
    color *newRGB, *oldRGB;
    udword newHandle;

    oldRGB = memAlloc(width * height * sizeof(color), "ghq(TempBuffffer(:#", Pyrophoric);
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, handle);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, oldRGB);
    newRGB = trImageScale(oldRGB, width, height, newWidth, newHeight, TRUE);
    newHandle = trRGBTextureCreate(newRGB, newWidth, newHeight, bUseAlpha);
    memFree(newRGB);
    return(newHandle);
}
/*-----------------------------------------------------------------------------
    Name        : trPreLoadedTextureScale
    Description : Scale an already-loaded image to a new size.
    Inputs      : handle - index into texture registry
                  newWidth, newHeight - new size to make it.
    Outputs     : Creates a new, scaled down texture
    Return      : void
    Note        : If it's an RGBA texture, all colorized copies of the texture
                    will be scaled.
                  If the image has to be scaled up, it will be just flagged for
                    later re-loading (made pending).
----------------------------------------------------------------------------*/
void trPreLoadedTextureScale(sdword handle, sdword newWidth, sdword newHeight)
{
    texreg *reg = &trTextureRegistry[handle];
    ubyte *newIndexed, *oldIndexed;
    trcolorinfo *oldColorInfos, *newColorInfos;
    udword *handles;
    bool bUseAlpha;
    sdword index;

#ifndef HW_Release
#ifdef khentschel
    __asm int 3
#endif
#endif
    //sharing handled
    if (reg->sharedFrom != TR_NotShared)
    {                                                       //do nothing to shared textures; they're not included in VRAM sizing anyhow
        return;
    }
    //actually do we really?  Maybe we don't...
    dbgAssert(newWidth != reg->scaledWidth || newHeight != reg->scaledHeight);
    dbgAssert(!trPending(handle));

    if (RGLtype != SWtype || newWidth > reg->scaledWidth || newHeight > reg->scaledHeight || trNoPalettes)
    {                                                       //if texture is to be scaled up
        //... flag the texture for re-loading (make pending).
        if (bitTest(reg->flags, TRF_Paletted))
        {                                                   //if it's a paletted texture
            oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + TR_PaletteSize * reg->nPalettes);
        }
        else
        {                                                   //else's it's an RGB texture
            oldColorInfos = (trcolorinfo *)((ubyte *)reg->palettes + sizeof(udword) * reg->nPalettes);
        }
        //make duplicate copy of the colorinfo structures
        newColorInfos = memAlloc(TR_NumPaletteSize, "dupe trcolorinfo list", NonVolatile);
        memcpy(newColorInfos, oldColorInfos, sizeof(trcolorinfo) * reg->nPalettes);

        trInternalTexturesDelete(handle);                   //kill the GL textures and palettes
        reg->palettes = (ubyte *)newColorInfos;             //save new list of color infos
        trSetPending(trIndex(handle));                      //make this texture pending, to be re-loaded
    }
    else
    {                                                       //texture is scaling down
        //... get texture from the GL, delete old texture, scale down, create new ones
        if (bitTest(reg->flags, TRF_Paletted))
        {                                                   //if it's a paletted texture
            oldIndexed = memAlloc(reg->scaledWidth * reg->scaledHeight, "ZZY(TempBuffer)", Pyrophoric);
            trClearCurrent();
            glBindTexture(GL_TEXTURE_2D, reg->handle);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, oldIndexed);
            glDeleteTextures(1, (GLuint*)&reg->handle);              //delete GL-internal texture
            newIndexed = trImageScaleIndexed(oldIndexed, reg->scaledWidth, reg->scaledHeight, newWidth, newHeight, TRUE);
            reg->handle = trPalettedTextureCreate(newIndexed, (color *)reg->palettes, newWidth, newHeight);
            memFree(newIndexed);
        }
        else
        {                                                   //else's it's an RGB texture
            bUseAlpha = bitTest(reg->flags, TRF_Alpha);     //is this an alpha texture? most likely
            dbgAssert(bUseAlpha);
            if (reg->nPalettes == 1)
            {                                               //if only 1 palette
                reg->handle = trTextureHandleScale(reg->handle, reg->scaledWidth, reg->scaledHeight, newWidth, newHeight, bUseAlpha);
            }
            else
            {                                               //else scale all colorized copies of this texture down
                handles = (udword *)reg->palettes;
                for (index = 0; index < reg->nPalettes; index++, handles++)
                {
                    *handles = trTextureHandleScale(*handles, reg->scaledWidth, reg->scaledHeight, newWidth, newHeight, bUseAlpha);
                }
            }
        }
    }
    reg->scaledWidth = newWidth;
    reg->scaledHeight = newHeight;
}

/*-----------------------------------------------------------------------------
    Name        : trMeshSortListSort
    Description : Sorts a mesh-sorted list to put the paletted textures at the start
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int meshSortSortCompare(void const* p0, void const* p1)
{
    return((sdword)bitTest(trTextureRegistry[*((sdword*)p1)].flags, TRF_Paletted) -
           (sdword)bitTest(trTextureRegistry[*((sdword*)p0)].flags, TRF_Paletted));
}
void trMeshSortListSort(trmeshsort* sortList)
{
    qsort(sortList->textureList, sortList->nTextures, sizeof(sdword), meshSortSortCompare);
}

/*-----------------------------------------------------------------------------
    Name        : trBufferColorRGB
    Description : Colorize an image (or palette) based upon team effect buffers
    Inputs      : dest - where to put the colorized image
                  source - where to read the uncolorized version
                  teamEffect0,1 - team color effect buffers
                  teamColor0,1 - team colors to colorize to
                  size - size fo buffer, in pixels
                  flags - texreg flags telling us how to colorize the image
                  effectScalar0, effectScalar1 - scalar of
                    the team effect values to be read.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void trBufferColorRGB(color *dest, color *source,
                      ubyte *teamEffect0, ubyte *teamEffect1,
                      color teamColor0, color teamColor1,
                      sdword size, udword flags,
                      real32 effectScalar0, real32 effectScalar1)
{
    real32 teamRed0 = colUbyteToReal(colRed(teamColor0));
    real32 teamRed1 = colUbyteToReal(colRed(teamColor1));
    real32 teamGreen0 = colUbyteToReal(colGreen(teamColor0));
    real32 teamGreen1 = colUbyteToReal(colGreen(teamColor1));
    real32 teamBlue0 = colUbyteToReal(colBlue(teamColor0));
    real32 teamBlue1 = colUbyteToReal(colBlue(teamColor1));
    real32 redSource, greenSource, blueSource, redDest, greenDest, blueDest, effectReal0, effectReal1;

    while (size > 0)
    {
        //read the info from buffers into floating point
        redSource = colUbyteToReal(colRed(*source));
        greenSource = colUbyteToReal(colGreen(*source));
        blueSource = colUbyteToReal(colBlue(*source));
        if (bitTest(flags, TRF_TeamColor0))
        {
            effectReal0 = min(1.0f, colUbyteToReal(*teamEffect0) * effectScalar0);
        }
        else
        {
            effectReal0 = 0.0f;
        }
        if (bitTest(flags, TRF_TeamColor1))
        {
            effectReal1 = min(1.0f, colUbyteToReal(*teamEffect1) * effectScalar1);
        }
        else
        {
            effectReal1 = 0.0f;
        }
        //blend
        redDest = redSource * (1.0f - effectReal0) + teamRed0 * effectReal0;
        redDest = redDest * (1.0f - effectReal1) + teamRed1 * effectReal1;
        greenDest = greenSource * (1.0f - effectReal0) + teamGreen0 * effectReal0;
        greenDest = greenDest * (1.0f - effectReal1) + teamGreen1 * effectReal1;
        blueDest = blueSource * (1.0f - effectReal0) + teamBlue0 * effectReal0;
        blueDest = blueDest * (1.0f - effectReal1) + teamBlue1 * effectReal1;
        //write back
        *dest = colRGBA(colRealToUbyte(redDest), colRealToUbyte(greenDest), colRealToUbyte(blueDest), colAlpha(*source));
        //update pointers
        size--;
        dest++;
        source++;
        teamEffect0++;
        teamEffect1++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : trMeshSortListLoad
    Description : Load a list of texture associated with a particular mesh.
    Inputs      : sortList - which mesh to load
    Outputs     : Loads all the pending textures
    Return      : void
    Note        : This function works for indexed and RGB mode textures.
                    See notice in comment block of trMakeCurrent for more detail.
----------------------------------------------------------------------------*/
void trMeshSortListLoad(trmeshsort *sortList)
{
    sdword index, count;
    sdword paletteIndex;
    texreg *reg, *otherReg;
    sdword firstIndex = -1;
    trcolorinfo *colorInfo;
    char fullName[_MAX_PATH];
    color *colorData;
    lifheader *lifFile;
    ubyte *scaledData;
    color *registeredPalette;
    real32 scalar0[MAX_MULTIPLAYER_PLAYERS], scalar1[MAX_MULTIPLAYER_PLAYERS];
    real32 baseScalar, stripeScalar;
    ShipRace race;

#if TR_ERROR_CHECKING
    sdword j;
    //make sure all pending textures have the same palette requirements.
    for (index = 0; index < sortList->nTextures; index++)
    {
        if (trPending(sortList->textureList[index]))
        {                                                   //if texture needs loading
            if (firstIndex == -1)
            {                                               //if this is the first pending texture
                firstIndex = sortList->textureList[index];  //just remember the first one
            }
            else
            {                                               //else not first pending texture
                dbgAssert(trTextureRegistry[sortList->textureList[index]].nPalettes ==
                          trTextureRegistry[firstIndex].nPalettes);   //make sure same # palettes
                for (j = 0; j < trTextureRegistry[firstIndex].nPalettes; j++)
                {                                           //make sure identical palette info
                    dbgAssert(((trcolorinfo *)trTextureRegistry[sortList->textureList[index]].palettes)[j].base ==
                              ((trcolorinfo *)trTextureRegistry[firstIndex].palettes)[j].base);
                    dbgAssert(((trcolorinfo *)trTextureRegistry[sortList->textureList[index]].palettes)[j].detail ==
                              ((trcolorinfo *)trTextureRegistry[firstIndex].palettes)[j].detail);
                }
            }
        }
    }
    firstIndex = -1;
#endif //TR_ERROR_CHECKING

    //load in prequantized images
    for (index = 0; index < sortList->nTextures; index++)
    {
        if (trPending(sortList->textureList[index]))
        {                                                   //if texture needs loading
            reg = &trTextureRegistry[sortList->textureList[index]];
/*
            if (_stricmp(reg->fileName, "DERELICTS\\SHIPWRECK\\RL0\\LOD2\\PAGE1") == 0)
            {
                _asm int 3
            }
*/
            if (reg->sharedFrom != TR_NotShared)
            {                                               //if this texture just refers to another texture
                // notice that we keep the allocated copy of the colorInfo structures (reg->palettes)
                // this is so we can ensure that the colorinfos of this texture and the texture to
                // which it refers are consistant.  This is OK, but we must keep track of this case
                // and treat it specially later on when we try to access the color info structures
                trClearPending(sortList->textureList[index]);
                continue;
            }
            //load in the image from pre-quantized .LiF file
            if (bitTest(reg->flags, TRF_SharedFileName))
            {
                strcpy(fullName, (char *)memchr(reg->fileName, 0, SWORD_Max) + 1);
            }
            else
            {
                strcpy(fullName, reg->fileName);
            }
            strcat(fullName, ".LiF");                       //create full filename
            lifFile = trLIFFileLoad(fullName, 0);           //load in the file

            bitClear(reg->flags, TRF_TeamColor0);
            bitClear(reg->flags, TRF_TeamColor1);
            bitClear(reg->flags, TRF_Paletted);
            bitClear(reg->flags, TRF_Alpha);
            if (bitTest(lifFile->flags, TRF_TeamColor0))
            {
                bitSet(reg->flags, TRF_TeamColor0);
            }
            if (bitTest(lifFile->flags, TRF_TeamColor1))
            {
                bitSet(reg->flags, TRF_TeamColor1);
            }
            if (bitTest(lifFile->flags, TRF_Paletted))
            {
                bitSet(reg->flags, TRF_Paletted);
            }
            if (bitTest(lifFile->flags, TRF_Alpha))
            {
                bitSet(reg->flags, TRF_Alpha);
            }

#if TR_ERROR_CHECKING
            if (lifFile->width != reg->diskWidth || lifFile->height != reg->diskHeight)
            {
                dbgFatalf(DBG_Loc, "File '%s' is size (%dx%d) instead of (%dx%d) as noted in textures.ll.",
                    fullName, lifFile->width, lifFile->height, reg->diskWidth, reg->diskHeight);
            }
#endif
            colorInfo = (trcolorinfo *)reg->palettes;
            //find out what race we're dealing with and get it's hue-specific scalars for all teams
//            reg = &trTextureRegistry[sortList->textureList[0]];
            baseScalar = colUdwordToReal(reg->baseScalar);
            stripeScalar = colUdwordToReal(reg->stripeScalar);
            if (strchr(reg->fileName, '\\') && (baseScalar != 0.0f || stripeScalar != 0.0f))
            {
                count = strchr(reg->fileName, '\\') - (reg->fileName);
                dbgAssert(count > 0);
                memStrncpy(fullName, reg->fileName, count + 1);
                //fullName[count] = 0;
                race = StrToShipRace(fullName);
                if (race != 0xffffffff)
                {
                    for (count = 0; count < reg->nPalettes; count++)
                    {
                        cpTeamEffectScalars(&scalar0[count], &scalar1[count], colorInfo[count].base, colorInfo[count].detail, race);
                        scalar0[count] = (scalar0[count] - 1.0f) * baseScalar + 1.0f;
                        scalar1[count] = (scalar1[count] - 1.0f) * stripeScalar + 1.0f;
                    }
                }
            }
            else
            {
                for (count = 0; count < reg->nPalettes; count++)
                {
                    scalar0[count] = scalar1[count] = 1.0f;
                }
            }
            if (bitTest(reg->flags, TRF_Paletted))
            {                                               //load in paletted style
                //duplicate and blend the palettes for this image
#if TR_DEBUG_TEXTURES
                if (trSpecialTextures)
                {
                    ubyte *tempBuffer;
                    sdword index;
                    //create uncolored palette
                    reg->uncolored = trPalettedTextureCreate(lifFile->data, lifFile->palette, reg->diskWidth, reg->diskHeight);
                    //duplicate the uncolored palette
                    reg->uncoloredPalette = memAlloc(TR_PaletteSize, "UncoloredTestPalette", 0);
                    memcpy(reg->uncoloredPalette, lifFile->palette, TR_PaletteSize);
                    //create textures of the team effect buffers
                    tempBuffer = memAlloc(reg->diskWidth * reg->diskHeight, "FHGEEABuffer", 0);
                    if (bitTest(sortList->teamFlags, TRF_TeamColor0))
                    {
                        for (index = 0; index < reg->diskWidth * reg->diskHeight; index++)
                        {
                            tempBuffer[index] = lifFile->teamEffect0[lifFile->data[index]];
                        }
                    }
                    else
                    {
                        for (index = 0; index < reg->diskWidth * reg->diskHeight; index++)
                        {
                            tempBuffer[index] = 0;
                        }
                    }
                    reg->col0Handle = trPalettedTextureCreate(tempBuffer, trTestPalette0, reg->diskWidth, reg->diskHeight);
                    if (bitTest(sortList->teamFlags, TRF_TeamColor1))
                    {
                        for (index = 0; index < reg->diskWidth * reg->diskHeight; index++)
                        {
                            tempBuffer[index] = lifFile->teamEffect1[lifFile->data[index]];
                        }
                    }
                    else
                    {
                        for (index = 0; index < reg->diskWidth * reg->diskHeight; index++)
                        {
                            tempBuffer[index] = 0;
                        }
                    }
                    reg->col1Handle = trPalettedTextureCreate(tempBuffer, trTestPalette0, reg->diskWidth, reg->diskHeight);
                    memFree(tempBuffer);
                    //set the test texture flag
                }
#endif
                //see if the palette is already loaded by a previous texture in this list
                for (paletteIndex = 0; paletteIndex < index; paletteIndex++)
                {
                    otherReg = &trTextureRegistry[sortList->textureList[paletteIndex]];
                    if (bitTest(otherReg->flags, TRF_Paletted) && otherReg->sharedFrom == TR_NotShared)
                    {                                       //if other texture is paletted and not a shared texture
                        dbgAssert(otherReg->paletteCRC != TR_BadCRC);
                        if (otherReg->paletteCRC == lifFile->paletteCRC)
                        {                                   //if this palette matches other palette
                            //update palette register count
                            registeredPalette = (color *)otherReg->palettes;
#if TR_ERROR_CHECKING
                            if ((registeredPalette[0] & 0xff000000) >= 0xfe000000)
                            {
                                dbgFatalf(DBG_Loc, "Texture '%s', index %d has a usage count of %d.", otherReg->fileName, sortList->textureList[paletteIndex], registeredPalette[0] >> 24);
                            }
#endif
                            registeredPalette[0] += 0x01000000;
                            reg->palettes = (ubyte *)registeredPalette;
                            goto alreadyFoundPalette;       //skip the allocation bit
                        }
                    }
                }
                //else no matching palette was found, we must allocate and colorize some
                reg->palettes = memAlloc(TR_PaletteSize * reg->nPalettes +
                                         sizeof(trcolorinfo) * reg->nPalettes,
                                         "TexturePalettePool", NonVolatile);
                //keep track of what the colors were, for later reference
                memcpy((ubyte *)reg->palettes + TR_PaletteSize * reg->nPalettes,
                       colorInfo, sizeof(trcolorinfo) * reg->nPalettes);
                for (count = 0; count < reg->nPalettes; count++)
                {
                    //duplicate the palettes
                    if (!trUnusedInfo(&colorInfo[count]))
                    {                                       //if this color scheme in use
                        if (sortList->teamFlags & (TRF_TeamColor0 | TRF_TeamColor1))
                        {                                   //if there is team color
                            trBufferColorRGB((color *)(reg->palettes + count * TR_PaletteSize), lifFile->palette,
                                lifFile->teamEffect0, lifFile->teamEffect1, colorInfo[count].base, colorInfo[count].detail,
                                256, sortList->teamFlags, scalar0[count], scalar1[count]);
                        }
                        else
                        {                                   //no team color, don't colorize it
                            memcpy(reg->palettes + count * TR_PaletteSize, lifFile->palette, TR_PaletteSize);
                        }
                    }
                    else
                    {                                       //color not in use, clear this palette to rave-borg green so we can spot it
/*
#ifdef lmoloney
                        _asm int 3
#endif
*/
                        memClearDword((color *)(reg->palettes + count * TR_PaletteSize), TR_UnusedColorPalette, 256);
                    }
                }
                ((color *)reg->palettes)[0] &= 0x01ffffff;  //set new palette usage count to 1
alreadyFoundPalette:;
                //scale the image down as needed
                scaledData = trImageScaleIndexed(lifFile->data, reg->diskWidth, reg->diskHeight, reg->scaledWidth, reg->scaledHeight, FALSE);

                //create the GL texture object now
                if (trNoPalettes)
                {
                    reg->handle = trNoPalTextureCreate(scaledData, (ubyte*)reg->palettes, reg->scaledWidth, reg->scaledHeight, sortList->textureList[index]);
                }
                else
                {
                    reg->handle = trPalettedTextureCreate(scaledData, (color *)reg->palettes, reg->scaledWidth, reg->scaledHeight);
                }

                //save the palette CRC for palette sharing
                reg->paletteCRC = lifFile->paletteCRC;

                //free the image as loaded
                memFree(scaledData);                        //free scaled copy, no longer needed
            }
            else
            {                                               //load it in RGB style
                ubyte *scaledTeam0, *scaledTeam1;           //team color effect buffers scaled down
#if TR_DEBUG_TEXTURES
                sdword index;
                if (trSpecialTextures)
                {
                    color *tempBuffer;
                    //create uncolored palette
                    reg->uncolored = trRGBTextureCreate((color *)lifFile->data, reg->diskWidth, reg->diskHeight, FALSE);
                    //create textures of the team effect buffers
                    tempBuffer = memAlloc(sizeof(color) * reg->diskWidth * reg->diskHeight, "TempDKJFBuffer", 0);
                    if (bitTest(sortList->teamFlags, TRF_TeamColor0))
                    {
                        for (index = 0; index < reg->diskWidth * reg->diskHeight; index++)
                        {
                            tempBuffer[index] = colRGB(lifFile->teamEffect0[index], lifFile->teamEffect0[index], lifFile->teamEffect0[index]);
                        }
                    }
                    else
                    {
                        for (index = 0; index < reg->diskWidth * reg->diskHeight; index++)
                        {
                            tempBuffer[index] = colRGB(0, 0, 0);
                        }
                    }
                    reg->col0Handle = trRGBTextureCreate(tempBuffer, reg->diskWidth, reg->diskHeight, FALSE);
                    if (bitTest(sortList->teamFlags, TRF_TeamColor1))
                    {
                        for (index = 0; index < reg->diskWidth * reg->diskHeight; index++)
                        {
                            tempBuffer[index] = colRGB(lifFile->teamEffect1[index], lifFile->teamEffect1[index], lifFile->teamEffect1[index]);
                        }
                    }
                    else
                    {
                        for (index = 0; index < reg->diskWidth * reg->diskHeight; index++)
                        {
                            tempBuffer[index] = colRGB(0, 0, 0);
                        }
                    }
                    reg->col1Handle = trRGBTextureCreate(tempBuffer, reg->diskWidth, reg->diskHeight, FALSE);
                    memFree(tempBuffer);
                    //set the test texture flag
                }
#endif
                //scale the texture down
                colorData = trImageScale((color *)lifFile->data, reg->diskWidth, reg->diskHeight, reg->scaledWidth, reg->scaledHeight, FALSE);
                scaledTeam0 = trImageScaleIndexed(lifFile->teamEffect0, reg->diskWidth, reg->diskHeight, reg->scaledWidth, reg->scaledHeight, FALSE);
                scaledTeam1 = trImageScaleIndexed(lifFile->teamEffect1, reg->diskWidth, reg->diskHeight, reg->scaledWidth, reg->scaledHeight, FALSE);
                //duplicate and blend the texture for the different teams
                if (colorInfo != NULL)
                {
                    //allocate the list of handles
                    reg->palettes = memAlloc(sizeof(udword) * reg->nPalettes +
                                sizeof(trcolorinfo) * reg->nPalettes,
                                "TextureRGBHandleList", NonVolatile);
                    //keep track of what the colors were, for later reference
                    memcpy((ubyte *)reg->palettes + sizeof(udword) * reg->nPalettes,
                           colorInfo, sizeof(trcolorinfo) * reg->nPalettes);
                    for (count = 0; count < reg->nPalettes; count++)
                    {
                        //blend the buffer by team color
                        if (!trUnusedInfo(&colorInfo[count]))
                        {                                   //if this team color in use
                            if (reg->flags & (TRF_TeamColor0 | TRF_TeamColor1))
                            {                               //and there is team color
                                trBufferColorRGB((color *)lifFile->data, colorData,
                                    scaledTeam0, scaledTeam1, colorInfo[count].base, colorInfo[count].detail,
                                    reg->scaledWidth * reg->scaledHeight, reg->flags, scalar0[count], scalar1[count]);
                                reg->handle =
                                    trRGBTextureCreate((color *)lifFile->data, reg->scaledWidth, reg->scaledHeight, bitTest(reg->flags, TRF_Alpha));
                            }
                            else
                            {                               //else no team color
                                reg->handle =
                                    trRGBTextureCreate(colorData, reg->scaledWidth, reg->scaledHeight, bitTest(reg->flags, TRF_Alpha));
                            }
                            //create the texture
                            if (reg->nPalettes > 1)
                            {
                                ((udword *)reg->palettes)[count] = reg->handle;
                                reg->handle = TR_InvalidInternalHandle;
                            }
                        }
                        else
                        {
                            ((udword *)reg->palettes)[count] = TR_InvalidInternalHandle;
                        }
                    }
                }
                else
                {
                    reg->palettes = NULL;
                    reg->handle =                           //create the texture
                        trRGBTextureCreate(colorData, reg->scaledWidth, reg->scaledHeight, bitTest(reg->flags, TRF_Alpha));
                }
                //free the no longer needed texture
                memFree(scaledTeam0);
                memFree(scaledTeam1);
                memFree(colorData);
            }
            trClearPending(sortList->textureList[index]);
//            bitClear(reg->flags, TRF_Pending);              //texture no longer pending
            memFree(colorInfo);
            memFree(lifFile);                               //get rid of the image which is no longer needed
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : trSizeSortCompare
    Description : Qsort callback for sorting the size sort list
    Inputs      : e1, e2 - cast to trmemmap pointers
    Outputs     :
    Return      : area2 - area1
----------------------------------------------------------------------------*/
int trSizeSortCompare(const void *e1, const void *e2)
{
    return(((trmemmap *)e2)->width * ((trmemmap *)e2)->height -
           ((trmemmap *)e1)->width * ((trmemmap *)e1)->height);
}

/*-----------------------------------------------------------------------------
    Name        : trCramScaleTableCompute
    Description : Build a table for all possible texture sizes, what
                  the closest size, integer exponents of 2, would be when
                  multiplied by the scaling factor.
    Inputs      : dest - where to buld the table
                  scaleFactor - how we are scaling the thing.
    Outputs     : computes an optimum scaling table in dest; a set of DWORDS where:
                    high word = width, low word = height
    Return      : void
----------------------------------------------------------------------------*/
void trCramScaleTableCompute(sdword dest[TR_TexSizesX][TR_TexSizesY], sdword scaleFactor)
{
    sdword x, y, index, closestSizeDiff, closestIndex;
    sdword closestWidth, closestHeight, widthToMatch, heightToMatch, width, height;
    sdword xDiff, yDiff;

    dbgAssert(scaleFactor <= 65536);                        //make sire we're not scaling up
    for (x = 0; x < TR_TexSizesX; x++)
    {                                                       //for all widths/heights
        for (y = 0; y < TR_TexSizesY; y++)
        {
            closestSizeDiff = SDWORD_Max;                   //ensure we'll find a better fit
            closestWidth = SDWORD_Max;
            closestHeight = SDWORD_Max;
            widthToMatch = (TR_MinimumSizeX << x) * scaleFactor / 65536;
            heightToMatch = (TR_MinimumSizeY << y) * scaleFactor / 65536;
            for (index = max(x, y); index >= 0; index--)    //for all x/y sizes smaller than this x/y size,
            {
                width = (TR_MinimumSizeX << max(x - index, 0));
                height = (TR_MinimumSizeX << max(y - index, 0));
                if (abs(width - widthToMatch) + abs(height - heightToMatch) <= closestSizeDiff &&
                    (x >= max(x - index, 0)) && (y >= max(y - index, 0)))
                {
                    closestSizeDiff = abs(width - widthToMatch) + abs(height - heightToMatch);
                    closestWidth = width;
                    closestHeight = height;
                    closestIndex = index;
                    if (closestSizeDiff == 0)
                    {                                       //if size matches exactly
                        break;                              //don't check any other sizes
                    }
                }
                if (widthToMatch > heightToMatch)
                {                                           //w/2 x h
                    width = (TR_MinimumSizeX << max(x - index - 1, 0));
                    height = (TR_MinimumSizeX << max(y - index, 0));
                }
                else
                {                                           //w x h/2
                    width = (TR_MinimumSizeX << max(x - index, 0));
                    height = (TR_MinimumSizeX << max(y - index - 1, 0));
                }
                if (abs(width - widthToMatch) + abs(height - heightToMatch) <= closestSizeDiff &&
                    (x >= max(x - index - 1, 0)) && (y >= max(y - index, 0)))
                {                                           //if this is as good as any other fit
                    closestSizeDiff = abs(width - widthToMatch) + abs(height - heightToMatch);
                    closestWidth = width;
                    closestHeight = height;
                    closestIndex = index | 0x40000000;      //negative or positive tells which one we tried
                    if (closestSizeDiff == 0)
                    {                                       //if size matches exactly
                        break;                              //don't check any other sizes
                    }
                }
            }
            if (!(closestIndex & 0x40000000))
            {                                               //if closest was x * y
                //high word - width, low word - height
                dest[x + TR_SSBMinX][y + TR_SSBMinY] = ((TR_MinimumSizeX << (max(x - closestIndex, 0))) << 16) |
                    (TR_MinimumSizeY << (max(y - closestIndex, 0)));
                dbgAssert((TR_MinimumSizeX << (max(x - closestIndex, 0))) <=
                          (TR_MinimumSizeX << x));          //make sure smaller than dimensions to match
                dbgAssert((TR_MinimumSizeY << (max(y - closestIndex, 0))) <=
                          (TR_MinimumSizeY << y));
            }
            else
            {                                               //closest was x/2 * y
                if (x > y)
                {                                           //if texture larger in x
                    xDiff = 1;                              //scale x down
                    yDiff = 0;
                }
                else
                {                                           //texture larger in y
                    xDiff = 0;
                    yDiff = 1;                              //scale y down instead of x
                }
                closestIndex &= 0x0fffffff;                 //remove the flag bit
                dest[x + TR_SSBMinX][y + TR_SSBMinY] = ((TR_MinimumSizeX << (max(x - closestIndex - xDiff, 0))) << 16) |
                    (TR_MinimumSizeY << (max(y - closestIndex - yDiff, 0)));
                dbgAssert((TR_MinimumSizeX << (max(x - closestIndex - xDiff, 0))) <=
                          (TR_MinimumSizeX << x));          //make sure smaller than dimensions to match
                dbgAssert((TR_MinimumSizeY << (max(y - closestIndex - yDiff, 0))) <=
                          (TR_MinimumSizeY << y));
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : trCramAttempt
    Description : Attempt to fit all textures into the current RAM pools
                    given a certain scaling factor.
    Inputs      : scaleFactor - amount to resize the textures, 65536 being to
                    not scale at all.
    Outputs     :
    Return      : Total amount of RAM needed for this scaling, accounting for
                    granularity.
----------------------------------------------------------------------------*/
sdword trCramAttempt(sdword scaleFactor)
{
    sdword x, y, index, used, size;
#if TR_ERROR_CHECKING
    sdword width, height;
#endif
    sdword newSize[TR_TexSizesX][TR_TexSizesY];
    texreg *reg;

#if TR_VERBOSE_LEVEL >= 2
    dbgMessagef("\ntrCramAttempt: fitting textures, scale factor %d", scaleFactor);
#endif  //TR_VERBOSE_LEVEL
    for (index = used = 0; index < trNumberRamPools; index++)
    {
        trRamPoolList[index].used = 0;                      //all pools empty
    }
    trCramScaleTableCompute(newSize, scaleFactor);          //compute table of sizes

    //now we have our crazy table, we can go on to see if we can fit these guys
    for (index = 0; index < trSizeSortLength; index++)
    {
        if (!trAllocated(index))
        {                                                   //don't count unallocated textures
            continue;
        }
        reg = &trTextureRegistry[trSizeSortList[index].index];//get pointer to registry structure
        //sharing handled
        if (reg->sharedFrom != TR_NotShared)
        {                                                   //don't consider shared textures in VRAM size computations
            continue;
        }
        dbgAssert(bitNumberSet((udword)reg->diskWidth, 16) == 1);//make sure integer exponent of 2 in size
        dbgAssert(bitNumberSet((udword)reg->diskHeight, 16) == 1);
        size = newSize[bitLowBitPosition((udword)reg->diskWidth) - TR_SSBMinX]
            [bitLowBitPosition((udword)reg->diskHeight) - TR_SSBMinY];//get image size from table
#if TR_ERROR_CHECKING
        width = (size & 0xffff0000) >> 16;
        height = size & 0x0000ffff;
        dbgAssert(reg->diskWidth >= width);
        dbgAssert(reg->diskHeight >= height);
#endif
        size = ((size & 0xffff0000) >> 16) * (size & 0x0000ffff);//convert to just size
        size = (size + (trRamPoolGranularity - 1)) &
            (~(trRamPoolGranularity - 1));                  //round up for granularity
        //now try to fit a texture of this new size into some RAM pool
        //!!! might want to alternate the starting pool so the
        //!!! big ones get evenly distributed
        if (bitTest(reg->flags, TRF_Paletted))
        {                                                   //if paletted texture
            for (x = 0; x < trNumberRamPools; x++)
            {
                if (trRamPoolList[x].used + size <= trRamPoolList[x].length)
                {                                           //if it'll fit in this one
                    trRamPoolList[x].used += size;          //update this pools used amount
                    used += size;                           //update total used amount
                    break;                                  //stop running through the pools
                }
                else
                {
                    //unable to fit texture in any slot, keep calculating total size anyhow
                    used += size;
                }
            }
        }
        else
        {                                                   //else an RGB texture
            for (y = 0; y < reg->nPalettes; y++)
            {                                               //do a multiple of times because RGB textures need to be duplicated
                for (x = 0; x < trNumberRamPools; x++)
                {
                    if (trRamPoolList[x].used + size <= trRamPoolList[x].length)
                    {                                       //if it'll fit in this one
                        trRamPoolList[x].used += size * 2;      //update this pools used amount
                        used += size * 2;                       //update total used amount
                        goto itFitFine;                     //stop running through the pools
                    }
                }
                //unable to fit texture in any slot, keep calculating total size anyhow
                used += size;
itFitFine:;
            }
        }
    }
    return(used);                                           //now we've tried to fit all the textures
}

/*-----------------------------------------------------------------------------
    Name        : trCramRAMScaleCompute
    Description : Compute a scaling factor for fitting all registered textures
                    into VRAM.
    Inputs      : void
    Outputs     :
    Return      : Newly computed scaling factor
----------------------------------------------------------------------------*/
sdword trCramRAMScaleCompute(void)
{
    sdword index, scaleFactor, bestScaleFactor, scaleOld;
    sdword totalUsed, totalAvailable, margin, bestMargin;
    sdword iterations = 0;

    for (index = totalAvailable = 0; index < trNumberRamPools; index++)
    {
        totalAvailable += trRamPoolList[index].length;      //compute length of all pools
    }

    margin = -1;
    bestScaleFactor = scaleFactor = 65536;                  //fixed-point multiplier for scaling.  This means 1
    bestMargin = SDWORD_Max;
    while (margin < 0 || margin > totalAvailable * TR_AvailabilityMargin)
    {
        totalUsed = trCramAttempt(scaleFactor);
        margin = totalAvailable - totalUsed;
        // detect if it fit no problem on first try
        if (scaleFactor >= 65536 && margin >= 0)
        {                                                   //if it fit at full size
            bestScaleFactor = scaleFactor;                  //we're done
            break;
        }
        // record best scale factor
        if (margin >= 0 && margin < bestMargin)
        {                                                   //if best margin so far
            bestMargin = margin;                            //remember this scale factor
            bestScaleFactor = scaleFactor;
        }
        scaleOld = scaleFactor;
        scaleFactor += (sdword)((__int64)margin * (__int64)65536 /
                                (__int64)totalAvailable);
        if (scaleFactor <= 0)
        {
            scaleFactor = scaleOld / 2;
        }
        if (margin > 65536)
        {                                                   //if it 'over-fit'
            break;
        }
        if (iterations > TR_MaxCramIterations && margin >= 0)//only do this so many times
        {
            break;
        }
        iterations++;
    }
#if TR_VERBOSE_LEVEL >= 1
    dbgMessagef("\ntrCramRAMScaleCompute: fit textures, scale factor %d, in %d iterations", min(65536, bestScaleFactor), iterations);
#endif  //TR_VERBOSE_LEVEL
    //now we have a scale factor which will enable us to fit all textures
    //into a given amount of texture RAM, provided we follow the same fitting
    //algorithm as trCramAttempt.  The final step here is to run through all the
    //textures and set their scaled sizes, if they are pending, or actually
    //scale them down as needed, if they are already loaded.
    return(min(65536, bestScaleFactor));                    //make sure we're not scaling anything UP
}

/*-----------------------------------------------------------------------------
    Name        : trCramIntoRAM
    Description : Adjusts sizes of all registered textures to fit them into a
                    specified amount of texture RAM.  Available RAM pools are
                    given by trRamPoolList[].
    Inputs      : texture scaling factor, provided by trCramRAMScaleCompute
                    or supplied independently.
    Outputs     : Any textures previously loaded will be copied to system RAM,
                    scaled down and copied back to texture RAM if they need
                    scaling down.
    Return      : void
    Note        : This function is almost identical to trCramAttempt except
        that it actually fits the textures, rather than just seeing if it fits.
----------------------------------------------------------------------------*/
void trCramIntoRAM(sdword scaleFactor)
{
    sdword x, y, index, used, size, width, height;
    sdword newSize[TR_TexSizesX][TR_TexSizesY];
    texreg *reg;

#if TR_VERBOSE_LEVEL >= 2
    dbgMessagef("\ntrCramIntoRAM: fitting textures, scale factor %d", scaleFactor);
#endif  //TR_VERBOSE_LEVEL
    for (index = used = 0; index < trNumberRamPools; index++)
    {
        trRamPoolList[index].used = 0;                      //all pools empty
    }
    trCramScaleTableCompute(newSize, scaleFactor);          //compute table of sizes

    //now we have our crazy table, we can go on and fit these guys
    for (index = 0; index < trSizeSortLength; index++)
    {
        reg = &trTextureRegistry[trSizeSortList[index].index];//get pointer to registry structure
        dbgAssert(bitNumberSet((udword)reg->diskWidth, 16) == 1);//make sure integer exponent of 2 in size
        dbgAssert(bitNumberSet((udword)reg->diskHeight, 16) == 1);
        size = newSize[bitLowBitPosition((udword)reg->diskWidth) - TR_SSBMinX]
            [bitLowBitPosition((udword)reg->diskHeight) - TR_SSBMinY];//get image size from table
        width = (size & 0xffff0000) >> 16;
        height = size & 0x0000ffff;
        size = width * height;                              //convert to just size
        size = (size + (trRamPoolGranularity - 1)) &
            (~(trRamPoolGranularity - 1));                  //round up for granularity
        if (trPending(trSizeSortList[index].index))
        {                                                   //if image hasn't been loaded yet
            reg->scaledWidth = (sword)width;                //just set the size of the texture
            reg->scaledHeight = (sword)height;
            dbgAssert(reg->diskWidth >= reg->scaledWidth);
            dbgAssert(reg->diskHeight >= reg->scaledHeight);
        }
        else
        {                                                   //else image already loaded and in use
            if (width != reg->scaledWidth || height != reg->scaledHeight)
            {                                               //if it needs to be scaled down
                trPreLoadedTextureScale(trSizeSortList[index].index,
                                        width, height);     //scale it back
            }
        }
        //now try to fit a texture of this new size into some RAM pool
        if (bitTest(reg->flags, TRF_Paletted))
        {                                                   //if paletted texture
            for (x = 0; x < trNumberRamPools; x++)
            {
                if (trRamPoolList[x].used + size <= trRamPoolList[x].length)
                {                                           //if it'll fit in this one
                    trRamPoolList[x].used += size;          //update this pools used amount
                    used += size;                           //update total used amount
                    break;                                  //stop running through the pools
                }
                else
                {
                    //unable to fit texture in any slot, keep calculating total size anyhow
                    used += size;
                }
            }
        }
        else
        {                                                   //else an RGB texture
            for (y = 0; y < reg->nPalettes; y++)
            {                                               //do a multiple of times because RGB textures need to be duplicated
                for (x = 0; x < trNumberRamPools; x++)
                {
                    if (trRamPoolList[x].used + size <= trRamPoolList[x].length)
                    {                                       //if it'll fit in this one
                        trRamPoolList[x].used += size;      //update this pools used amount
                        used += size;                       //update total used amount
                        goto itFitFine;                     //stop running through the pools
                    }
                }
                //unable to fit texture in any slot, keep calculating total size anyhow
#if TR_ERROR_CHECKING
                dbgFatalf(DBG_Loc, "\ntrCramIntoRAM: Ran out of texture RAM!");
#endif
itFitFine:;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : trListFileLoad
    Description : Loads in the named liflist file.
    Inputs      : name - name of list file to load
                  number - parameter to recieve the number of elements
    Outputs     :
    Return      : pointer to the .lif listing
    Note        : the element list can be freed by memFree
----------------------------------------------------------------------------*/
llelement *trListFileLoad(char *name, sdword *number)
{
    filehandle f;
    llfileheader header;
    sdword index;
    llelement *list;
    char *stringBlock, *sharingBlock;

    f = fileOpen(name, 0);
    fileBlockRead(f, &header, sizeof(llfileheader));        //read the file header

#if TR_ERROR_CHECKING
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
#endif
    //allocate the list and string block
    list = memAlloc(header.totalLength, "LiFList", 0);
    stringBlock = (char *)list + header.nElements * sizeof(llelement);
    //read in the existing file to the element list/string table
    fileBlockRead(f, list, header.totalLength);

    //fix up the names of the texture files
    for (index = 0; index < header.nElements; index++)
    {
        list[index].textureName += (udword)stringBlock;
    }
    //fix up any sharing pointers there may be
    if (header.sharingLength != 0)
    {
        sharingBlock = (char *)stringBlock + header.stringLength;
        for (index = 0; index < header.nElements; index++)
        {
            if (list[index].nShared != 0)
            {
                (ubyte *)list[index].sharedTo += (udword)sharingBlock;
            }
        }
    }
    fileClose(f);
    //return some info
    *number = header.nElements;
    return(list);
}

/*-----------------------------------------------------------------------------
    Name        : trImageMeasureFromListing
    Description : Get the dimensions of a named texture from the specified listing file
    Inputs      : name - name to look for
                  list - list of textures to use
                  length - length of the list
    Outputs     : width, height, flags - where to store the dimensions and flags, if found
                  sharedFrom - name of texture we're shared from, or NULL
    Return      : TRUE if found, FALSE otherwise
----------------------------------------------------------------------------*/
bool trImageMeasureFromListing(char *name, llelement *list, sdword listLength, sdword *width, sdword *height, udword *flags, char **sharedFrom)
{
    sdword base = 0, index, length = listLength, result;

    while (length >= 1)
    {
        index = length / 2 + base;                          //get centre of section
        dbgAssert(index >= 0 && index < listLength);
        result = _stricmp(name, list[index].textureName);
        if (result == 0)
        {                                                   //if exact match
            *width = list[index].width;
            *height = list[index].height;
            *flags = list[index].flags;
            if (list[index].sharedFrom < 0)
            {
                *sharedFrom = NULL;
            }
            else
            {
                *sharedFrom = list[list[index].sharedFrom].textureName;
            }
            return(TRUE);                                   //done searching
        }
        if (result < 0)
        {                                                   //if low half of section
            length = index - base;
        }
        else
        {
            if (length <= 1)
            {
                return(FALSE);                              //fails if it gets here
            }
            length = length + base - index;
            base = index;
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : trSharedFilenameCreate
    Description : Make the filename of the indexed texture sharing-compatable.
    Inputs      : trIndex - index of texreg structure
                  lifListing - .ll file
                  listingLength - length of .ll file
    Outputs     : the indexed texreg will have the filename of the share parent
                    added to the end of the regular filename.
    Return      : void
----------------------------------------------------------------------------*/
void trSharedFilenameCreate(sdword trIndex, llelement *lifListing, sdword listingLength)
{
    sdword width, height;
    udword listFlags;
    char *pSharedFrom, *newFilename;
    bool bResult;
    texreg *reg = trStructure(trIndex);

    bResult = trImageMeasureFromListing(reg->fileName, lifListing, listingLength, &width, &height, &listFlags, &pSharedFrom);
    dbgAssert(bResult);                                     //get the share parent name from the .ll file
    dbgAssert(pSharedFrom != NULL);
    newFilename = memAlloc(strlen(reg->fileName) + strlen(pSharedFrom) + 2, "SharedFileName", NonVolatile);
    strcpy(newFilename, reg->fileName);                     //create the concatenated names
    strcpy((char *)memchr(newFilename, 0, SWORD_Max) + 1, pSharedFrom);
    memFree(reg->fileName);                                 //kill the old name string
    reg->fileName = newFilename;                            //remember new file name
    bitSet(reg->flags, TRF_SharedFileName);
}

/*-----------------------------------------------------------------------------
    Name        : trLiFMeasure
    Description : Measure a .LiF file by loading it's header.
    Inputs      : fileName - name of .LiF file
    Outputs     : width, height, flags - out parameters for the file attributes
    Return      : TRUE if the file measured OK, FALSE otherwise.
----------------------------------------------------------------------------*/
bool trLiFMeasure(char *fileName, sdword *width, sdword *height, udword *flags)
{
    filehandle handle;
    lifheader header;

    if (!fileExists(fileName, 0))
    {
        return(FALSE);
    }
    handle = fileOpen(fileName, 0);
    fileBlockRead(handle, &header, sizeof(lifheader));
    fileClose(handle);

    if (strcmp(header.ident, LIF_FileIdentifier))
    {                                                       //verify proper file
        return(FALSE);
    }
    if (header.version != LIF_FileVersion)
    {
        return(FALSE);
    }
    *width = header.width;
    *height = header.height;
    *flags = header.flags;
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : trRegistryRefresh
    Description : Refresh the texture registry, deleting old textures and
                    loading in new ones.
    Inputs      : void
    Outputs     : See description.
    Return      : void
    Note        : Nothing is currently done to ensure textures are memory
                    mapped the same size as they were in previous levels.
----------------------------------------------------------------------------*/
void trRegistryRefresh(void)
{
    sdword index, width, height, scaleFactor, texes;
    char fullName[_MAX_PATH];
    char *pSharedFrom;
    texreg *reg;
    llelement *lifListing;
    sdword listingLength;
    udword listFlags;

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
        return;
#endif

#if TR_VERBOSE_LEVEL >= 1
    dbgMessagef("\ntrRegistryRefresh: Refreshing up to %d textures", trHighestAllocated);
#endif  //TR_VERBOSE_LEVEL
    trTextureDeleteAllUnregistered();                       //start by getting rid of dead weight

    //allocate the texture size sort list
    dbgAssert(trSizeSortList == NULL);
    trSizeSortList = memAlloc((trHighestAllocated + 1) * sizeof(trmemmap), "Texture Size Sort List", 0);
    trSizeSortLength = 0;

    //allocate the mesh reference sort list
    trMeshSortLength = 0;
    trMeshSortIndex = 0;
    trMeshSortList = NULL;

    //load in the .ll file for sharing and sizing purposes
    lifListing = trListFileLoad(TR_LifListing, &listingLength);

    //sharing handled
    //this pass will ensure that all shared textures are consitent in color with the textures from which they are shared.
    //texture that are inconsitent will be made pending.
    for (index = 0; index <= trHighestAllocated; index++)
    {                                                       //for all loaded textures
        if (trAllocated(index))
        {                                                   //texture slot not free
            reg = &trTextureRegistry[index];                //get pointer to current texture reg struct
            if (reg->sharedFrom != TR_NotShared)
            {                                               //if this texture is shared from another texture
                if (!trAllColorsEqual(index, reg->sharedFrom))
                {                                           //if the shared texture is different colors from the texture it shares
                    //set this texture as pending
                    dbgAssert(trAllocated(reg->sharedFrom));
                    dbgAssert(trTextureRegistry[reg->sharedFrom].nSharedTo > 0);
                    trTextureRegistry[reg->sharedFrom].nSharedTo--;
                    reg->sharedFrom = TR_NotShared;         //no longer shared
                    trSetPending(index);                    //make it get loaded normally
                    trSharedFilenameCreate(index, lifListing, listingLength);//make the shared filename
                }
            }
        }
    }
    HorseRaceBeginBar(TEXTURE1_BAR);
    //now run through list and get image size of all pending images
    for (index = 0; index <= trHighestAllocated; index++)
    {                                                       //for all loaded textures
        if (trAllocated(index))
        {                                                   //texture slot not free
            reg = &trTextureRegistry[index];                //get pointer to current texture reg struct
            if (trPending(index))
            {                                               //if pending a load
                /*
                if (_stricmp(reg->fileName, "R1\\AdvanceSupportFrigate\\RL0\\LOD1\\frontupperantenna") == 0)
                {
                    _asm int 3
                }
                */
                //... get width/height/flags trying several methods
                if (!trImageMeasureFromListing(reg->fileName, lifListing, listingLength, &width, &height, &listFlags, &pSharedFrom))
                {
#if TR_VERBOSE_LEVEL >= 1
                    dbgMessagef("\nImage '%s' not found in listing", reg->fileName);
#endif
                    strcpy(fullName, reg->fileName);
                    strcat(fullName, ".LiF");
                    if (trLiFMeasure(fullName, &width, &height, &listFlags))
                    {
#if TR_VERBOSE_LEVEL >= 1
                        dbgMessagef("...regular .LiF", reg->fileName);
#endif
                        reg->diskWidth = (sword)width;
                        reg->diskHeight = (sword)height;
#if TR_ERROR_CHECKING
                        if ((reg->flags & TRM_CompareFlags) != (listFlags & TRM_CompareFlags))
                        {
                            dbgWarningf(DBG_Loc, "Texture '%s' flags from listing inconsistent with expected.", reg->fileName);
                        }
#endif
                        reg->flags |= (uword)listFlags & TRM_ListFlag;//plug these flags straight in
                        if (bitTest(reg->flags, TRF_Alpha))
                        {                                   //if image says it's alpha
                            bitClear(reg->flags, TRF_Paletted); //it can't be paletted
                        }
                        else
                        {
                            bitSet(reg->flags, TRF_Paletted);
                        }
                    }
                    else
                    {
                        dbgFatalf(DBG_Loc, "Cannot open '%s'.", fullName);
                    }
                }
                else
                {                                           //else the list search measurment worked
                    reg->diskWidth = (sword)width;
                    reg->diskHeight = (sword)height;
                    reg->flags |= (ubyte)(listFlags & TRM_ListFlag);//plug these flags straight in
                    if (pSharedFrom == NULL)
                    {                                       //this texture not shared
                        reg->sharedFrom = TR_NotShared;
                    }
                    else
                    {                                       //this texture is shared
                        reg->sharedFrom = trFindTextureIndexByName(pSharedFrom);//find what texture it's shared to
                        if (reg->sharedFrom == TR_NotShared)
                        {                                   //the share parent is not loaded
                            trSharedFilenameCreate(index, lifListing, listingLength);//make the shared filename
#if TR_VERBOSE_LEVEL >= 2
                            dbgMessagef("\nTexture '%s' invalid for sharedFrom of '%s'.", pSharedFrom, reg->fileName);
#endif
                        }
                        else if (!trAllColorsEqual(index, reg->sharedFrom))
                        {                                   //if these textures do not share common texture colors
#if TR_VERBOSE_LEVEL >= 2
                            if (reg->sharedFrom == TR_NotShared)
                            {
                                dbgMessagef("\nTexture '%s' has different colors than '%s'", pSharedFrom, reg->fileName);
                            }
#endif
                            reg->sharedFrom = TR_NotShared; //can't allow this case
                            trSharedFilenameCreate(index, lifListing, listingLength);//make the shared filename
                        }
                        if (reg->sharedFrom != TR_NotShared)
                        {                                   //was it shared properly?
                            dbgAssert(trTextureRegistry[reg->sharedFrom].sharedFrom == TR_NotShared);//share parent must not be shared
                            dbgAssert(trTextureRegistry[reg->sharedFrom].nSharedTo < UBYTE_Max - 1);
                            trTextureRegistry[reg->sharedFrom].nSharedTo++;
                            //sharing handled - load a shared texture normally
                        }
                    }
#if TR_ERROR_CHECKING
                    if ((reg->flags & TRM_CompareFlags) != (listFlags & TRM_CompareFlags))
                    {
                        dbgWarningf(DBG_Loc, "Texture '%s' flags from header inconsistent with expected.", reg->fileName);
                    }
#endif
                    if (bitTest(reg->flags, TRF_Alpha))
                    {                                       //if image says it's alpha
                        bitClear(reg->flags, TRF_Paletted); //it can't be paletted
                    }
                    else
                    {
                        bitSet(reg->flags, TRF_Paletted);   //else it has to be paletted
                    }
                }
            }
            //only pending and pre-loaded textures get here
            dbgAssert(trAllocated(index));
            dbgAssert(trSizeSortLength <= trHighestAllocated);
            trSizeSortList[trSizeSortLength].index = index; //build unsorted list
            trSizeSortList[trSizeSortLength].width = reg->diskWidth;
            trSizeSortList[trSizeSortLength].height = reg->diskHeight;
            trSizeSortLength++;
            trMeshSortAdd(index, listFlags);                //add entry to mesh-sort list
        }

        if ((index & 31) == 0)
        {
            HorseRaceNext(((real32)index)/((real32)trHighestAllocated));
        }

        if (hrAbortLoadingGame)
        {
            break;
        }
    }
    memFree(lifListing);                                    //done with the listing, free it

    if (hrAbortLoadingGame)
    {
        goto abortloading;
    }

    //now that we have the lists, we can sort them
    qsort(trSizeSortList, trSizeSortLength, sizeof(trmemmap), trSizeSortCompare);

    //!!! important note:  the texture cramming operation uses an order inherently
    //  different from the one used in actual texture creation (size-sorted as
    //  opposed to mesh-sorted).  As such, it is possible that texture allocation
    //  could fail after all the textures have been supposedly crammed.  If this
    //  happens, this list could be arranged to cram textures in the same order
    //  as the mesh-sort lists.

    //fit all textures into allotted RAM
    scaleFactor = trCramRAMScaleCompute();
    trCramIntoRAM(scaleFactor);

    //now run through the mesh-sorted lists and load them in that order
    for (index = texes = 0; index < trMeshSortLength; index++)
    {
        if (trMeshSortList[index].nTextures)
            texes++;
    }
    HorseRaceBeginBar(TEXTURE2_BAR);  //texture barnumber = second parm
    for (index = 0; index < trMeshSortLength; index++)
    {                                                       //for each mesh-sort list
        HorseRaceNext(((real32)index)/((real32)texes));
        if (hrAbortLoadingGame)
        {
            break;
        }
        trMeshSortListSort(&trMeshSortList[index]);         //make sure alpha mapped textures are toward the end
        trMeshSortListLoad(&trMeshSortList[index]);         //load all textures for this list
    }
abortloading:
    //Horse Race temporary fix.  This needs to go after
    //the last bar is done loading.  HorseRaceNext will return true
    //after everyone has reached 100%

    //fix later!!!  This will hang if a player is dropped!
    //while(!HorseRaceNext(1.0f));

    //free the mesh reference sort lists
    for (index = trMeshSortLength - 1; index >= 0; index--)
    {                                                       //free all the sub-lists in reverse order of allocation
        memFree(trMeshSortList[index].textureList);
    }
    if (trMeshSortList != NULL)
    {
        memFree(trMeshSortList);                            //free the master list
    }

    //free the size-sorted list
    memFree(trSizeSortList);                                //free the size sort list
    trSizeSortList = NULL;                                  //and prevent any future references
    trSizeSortLength = 0;
    trCurrentHandle = TR_Invalid;
#if MEM_ANALYSIS
    memAnalysisCreate();
#endif
#if TR_ERROR_CHECKING
    if (RGL)
    {
        rglFeature(RGL_TEXTURE_LOG);
    }
#endif

#if TR_TEXTURE_USAGE
    trTextureUsageList("texturesUsage.list");
#endif
}

/*-----------------------------------------------------------------------------
    Name        : trClearCurrent
    Description : clear the current texture (assign it to an invalid handle)
    Inputs      :
    Outputs     : trCurrentHandle is set to TR_InvalidHandle
    Return      :
----------------------------------------------------------------------------*/
void trClearCurrent()
{
    trCurrentHandle = TR_InvalidHandle;
}

/*-----------------------------------------------------------------------------
    Name        : trMakeCurrent
    Description : Make a given texture current
    Inputs      : handle - handle of texture/palette to use
    Outputs     : Makes the specified texture current, plus downloads palettes
                    as needed.
    Return      : void
    Note        : When in paletted mode, the integer part of reg->handle is
                    the GL handle of the indexed palette and palette is a set
                    of consecutive paletted for the differing teams.
                    When in non-paletted mode, or it's a non-paletted texture,
                    reg->handle is undefined and reg->palettes is a list of GL
                    handles which define the various colorized versions of the
                    texture.
----------------------------------------------------------------------------*/
void trMakeCurrent(trhandle handle)
{
    ubyte *newPalette;
    texreg *reg;
    static bool currentAlpha = FALSE;

#if TR_NIL_TEXTURE
    if (GLOBAL_NO_TEXTURES)
    {
        rndTextureEnable(FALSE);
        return;
    }
#endif
    if (handle == TR_Invalid)
    {
        rndTextureEnable(FALSE);
        return;
    }
    if (trCurrentHandle == handle)
    {                                                       //if this texture already enabled
        trAvoidedChanges++;
        return;                                             //don't do anything
    }
    trTextureChanges++;
    trCurrentHandle = handle;                               //save the current handle
    reg = trStructure(handle);                              //get pointer to registry entry
    if (reg->sharedFrom != TR_NotShared)
    {                                                       //is this a shared texture
#if TR_VISUALIZE_SHARING
        if (keyIsHit(SKEY) && keyIsHit(TKEY))
        {                                                   //texture sharing debuggery
            rndTextureEnable(FALSE);
            return;
        }
        else
#endif
        {
            reg = &trTextureRegistry[reg->sharedFrom];      //get texture it's shared from
        }
    }

#if TR_PRINT_TEXTURE_NAMES
    if (trPrintTextureNames)
    {
        dbgMessagef("\ntrMakeCurrent: %s", reg->fileName);
    }
#endif
#if TR_DEBUG_TEXTURES
    if (trSpecialTextures)
    {
        if (keyIsStuck(LBRACK))
        {
            keyClearSticky(LBRACK);
            trSpecialTextureMode = max(TSM_None, trSpecialTextureMode - 1);
            dbgMessagef("\n%s texture mode.", trSpecialTextureName[trSpecialTextureMode]);
        }
        if (keyIsStuck(RBRACK))
        {
            keyClearSticky(RBRACK);
            trSpecialTextureMode = min(TSM_NumberModes - 1, trSpecialTextureMode + 1);
            dbgMessagef("\n%s texture mode.", trSpecialTextureName[trSpecialTextureMode]);
        }
        if (trSpecialTextures && trSpecialTextureMode != TSM_None)
        {
            dbgAssert(trSpecialTextureMode < TSM_NumberModes);
            dbgAssert(trSpecialTextureMode > TSM_None);
            switch (trSpecialTextureMode)
            {
                case TSM_Uncolored:
                    glBindTexture(GL_TEXTURE_2D, reg->uncolored);
                    if (bitTest(reg->flags, TRF_Paletted))
                    {
                        trColorTable(reg->uncoloredPalette);
                    }
                    break;
                case TSM_TeamColor0:
                    glBindTexture(GL_TEXTURE_2D, reg->col0Handle);
                    if (bitTest(reg->flags, TRF_Paletted))
                    {
                        trColorTable(trTestPalette0);
                    }
                    break;
                case TSM_TeamColor1:
                    glBindTexture(GL_TEXTURE_2D, reg->col1Handle);
                    if (bitTest(reg->flags, TRF_Paletted))
                    {
                        trColorTable(trTestPalette0);
                    }
                    break;
            }
            primErrorMessagePrint();
            return;
        }
    }
#endif //TR_DEBUG_TEXTURES

    if (bitTest(reg->flags, TRF_Paletted))
    {                                                       //if we should use palettes
        if (trNoPalettes)
        {
            if ((udword)reg->currentPalette != trPaletteIndex(handle))
            {
                reg->currentPalette = (sword)trPaletteIndex(handle);
                dbgAssert((sdword)trPaletteIndex(handle) < reg->nPalettes);
                newPalette = &reg->palettes[trPaletteIndex(handle) * TR_PaletteSize];
            }
            else
            {
                dbgAssert(trPaletteIndex(handle) < (udword)reg->nPalettes);
                newPalette = &reg->palettes[trPaletteIndex(handle) * TR_PaletteSize];
            }
            if (bitTest(reg->flags, TRF_NoPalPending))
            {
                trNoPalTextureRecreate(newPalette, reg->handle);
            }
            trNoPalMakeCurrent(newPalette, reg->handle);
            primErrorMessagePrint();
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, reg->handle);          //set new texture
            primErrorMessagePrint();
            if ((udword)reg->currentPalette != trPaletteIndex(handle))
            {                                                   //if we need to download a new palette
                reg->currentPalette = (sword)trPaletteIndex(handle);
                dbgAssert((sdword)trPaletteIndex(handle) < reg->nPalettes);

                newPalette = &reg->palettes[trPaletteIndex(handle) * TR_PaletteSize];//select appropriate palette
                trColorTable((color*)newPalette);               //download new palette

                primErrorMessagePrint();
            }
            else// if (!RGL && !trSharedPalettes)
            {
                dbgAssert(trPaletteIndex(handle) < (udword)reg->nPalettes);

                newPalette = &reg->palettes[trPaletteIndex(handle) * TR_PaletteSize];
                trColorTable((color*)newPalette);               //download new palette

                primErrorMessagePrint();
            }
        }
//!!!        glDisable(GL_BLEND);                                //never alpha on paletted textures
        glDisable(GL_ALPHA_TEST);                           //ditto
    }
    else
    {                                                       //else it's an non-paletted texture
        if (bitTest(reg->flags, TRF_Alpha))
        {                                                   //and has alpha
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.0f);
        }
        else
        {                                                   //else turning alpha off
            glDisable(GL_ALPHA_TEST);
        }
        //bind the proper version of this texture
        if (reg->nPalettes > 1)
        {
            glBindTexture(GL_TEXTURE_2D,                //set new texture
                          ((udword *)reg->palettes)[trPaletteIndex(handle)]);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, reg->handle);  //set new texture
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : trFilterEnable
    Description : Enables/disables texture filtering
    Inputs      : bEnable - TRUE to enable filtering
    Outputs     : Makes each texture current and changes it's filtering mode
    Return      :
----------------------------------------------------------------------------*/
void trFilterEnable(sdword bEnable)
{
    sdword index, j;
    texreg *reg;

    if (RGL) rglFeature(RGL_FASTBIND_ON);

    for (index = 0; index < trRegistrySize; index++)
    {
        if (trAllocated(index) && (!trPending(index)) && (trTextureRegistry[index].sharedFrom == TR_NotShared))
        {                                                   //if we can change this guy
            reg = &trTextureRegistry[index];

            if (bitTest(reg->flags, TRF_Paletted) || reg->nPalettes == 1)
            {                                               //if paletted image
                if (bitTest(reg->flags, TRF_Paletted) && trNoPalettes)
                {
                    trNoPalFilter(bEnable, reg->handle);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, reg->handle);  //set texture for modification
                    if (bEnable)
                    {                                           //set min/mag filters to linear
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    else
                    {                                           //or point sampling
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    }
                }
            }
            else
            {
                if (reg->palettes != NULL)
                {
                    for (j = 0; j < reg->nPalettes; j++)
                    {                                       //for each sub-texture
                        glBindTexture(GL_TEXTURE_2D, ((udword *)reg->palettes)[j]);
                        if (bEnable)
                        {                                   //set min/mag filters to linear
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        }
                        else
                        {                                   //or point sampling
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        }
                    }
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, reg->handle);
                    if (bEnable)
                    {                                       //set min/mag filters to linear
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    else
                    {                                       //or point sampling
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    }
                }
            }
        }
    }
    texLinearFiltering = bEnable;                           //set new filter mode flag

    trClearCurrent();

    if (RGL) rglFeature(RGL_FASTBIND_OFF);
}

/*-----------------------------------------------------------------------------
    Name        : trStructureGet
    Description : Get a texreg structure from a handle
    Inputs      : handle - valid texture handle
    Outputs     :
    Return      : structure
----------------------------------------------------------------------------*/
texreg *trStructureGet(trhandle handle)
{
    texreg *reg = trStructure(handle);

    //sharing handled
    if (reg->sharedFrom != TR_NotShared)
    {
        reg = trStructure(reg->sharedFrom);
    }
    return(reg);
}

/*-----------------------------------------------------------------------------
    Name        : trSetAllPending
    Description : ...
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trSetAllPending(bool freeNoPal)
{
    sdword index;

    for (index = 0; index <= trHighestAllocated; index++)
    {                                                       //for all textures in allocated range
        if (trAllocated(index))
        {                                                   //if texture allocated
            if (trTextureRegistry[index].nSharedTo > 0)
            {                                               //if this texture is shared to otherTextures
                trAllSharedFromDelete(index, TRUE);
            }
            if (!trPending(index))
            {
                trInternalTexturesDelete(index);
                trSetPending(index);
                if (!freeNoPal)
                {
                    //clear nopal pending flag.
                    //this texture isn't pending, it doesn't exist
                    bitClear(trTextureRegistry[index].flags, TRF_NoPalPending);
                }
                if (trTextureRegistry[index].sharedFrom != TR_NotShared)
                {                                               //if shared from another
                    dbgAssert(trTextureRegistry[index].nSharedTo == 0);
                    dbgAssert(trAllocated(trTextureRegistry[index].sharedFrom));
                    dbgAssert(trTextureRegistry[trTextureRegistry[index].sharedFrom].nSharedTo > 0);
                    trTextureRegistry[trTextureRegistry[index].sharedFrom].nSharedTo--;
                    trTextureRegistry[index].sharedFrom = TR_NotShared;//remove this sharing link
                }
            }
        }
    }

    if (freeNoPal)
    {
        trNoPalReadjust();
    }
    else
    {
        trNoPalReadjustWithoutPending();
    }
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalSingleRepDelete
    Description : delete a single un-paletted rep
    Inputs      : handle - the np handle
                  index - index within the np struct
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalSingleRepDelete(udword handle, sdword index)
{
    nopalreg* reg;

    dbgAssert(trNoPalAllocated(handle));

    reg = trNoPalStructure(handle);
    glDeleteTextures(1, (GLuint*)&reg->glhandle[index]);
    if (glRGBA16)
    {
        trNoPalBytesAllocated -= 2 * reg->width * reg->height;
    }
    else
    {
        trNoPalBytesAllocated -= 4 * reg->width * reg->height;
    }
    reg->glhandle[index] = 0;
    reg->crc[index] = 0;
    reg->timeStamp[index] = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalTextureDelete
    Description : wipe out a np struct - reps and data alike
    Inputs      : handle - the np handle
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalTextureDelete(udword handle)
{
    nopalreg* reg;
    sdword index;

    if (!trNoPalAllocated(handle))
    {
        return;
    }

    reg = trNoPalStructure(handle);
    for (index = 0; index < trNpNumHandles; index++)
    {
        if (reg->glhandle[index] != 0)
        {
            trNoPalSingleRepDelete(handle, index);
        }
    }

    if (reg->data != NULL)
    {
        memFree(reg->data);
        reg->data = NULL;
        trNoPalBytesAllocated -= reg->width * reg->height;
    }

    reg->texreghandle = TR_InvalidInternalHandle;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalTextureDeleteFromTexreg
    Description : flag the parent texreg w/ TRF_NoPalPending and delete all
                  un-paletted reps
    Inputs      : handle - the np handle
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalTextureDeleteFromTexreg(udword handle)
{
    nopalreg* reg;
    sdword index;

    dbgAssert(trNoPalAllocated(handle));

    reg = trNoPalStructure(handle);

    dbgAssert(reg->texreghandle != TR_InvalidInternalHandle);

    for (index = 0; index < trNpNumHandles; index++)
    {
        if (reg->glhandle[index] != 0)
        {
            trNoPalSingleRepDelete(handle, index);
            //only set pending if there's something to pend
            bitSet(trStructure(reg->texreghandle)->flags, TRF_NoPalPending);
        }
    }
}

void trNoPalTextureDeleteFromTexregWithoutPending(udword handle)
{
    nopalreg* reg;
    sdword index;

    dbgAssert(trNoPalAllocated(handle));

    reg = trNoPalStructure(handle);

    dbgAssert(reg->texreghandle != TR_InvalidInternalHandle);

    for (index = 0; index < trNpNumHandles; index++)
    {
        if (reg->glhandle[index] != 0)
        {
            trNoPalSingleRepDelete(handle, index);
        }
    }
}

#define NP_MEMFLAGS NonVolatile|ExtendedPool

/*-----------------------------------------------------------------------------
    Name        : trNoPalQueueStartup
    Description : startup the lra queue of np handles
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalQueueStartup(void)
{
    trNoPalQueueHead = 0;
    trNoPalQueueTail = 0;
    trNoPalQueueSize = trRegistrySize;
    trNoPalQueue = (udword*)memAlloc(trNoPalQueueSize * sizeof(udword), "nopal queue", NP_MEMFLAGS);
    memset(trNoPalQueue, 0, trNoPalQueueSize * sizeof(udword));
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalQueueReset
    Description : reset the lra queue of np handles
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalQueueReset(void)
{
    trNoPalQueueHead = 0;
    trNoPalQueueTail = 0;
    memset(trNoPalQueue, 0, trNoPalQueueSize * sizeof(udword));
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalQueueShutdown
    Description : shutdown the lra queue of np handles
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalQueueShutdown(void)
{
    memFree(trNoPalQueue);
    trNoPalQueueTail = 0;
    trNoPalQueueHead = 0;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalQueueFreeNext
    Description : free next item in the queue
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalQueueFreeNext(void)
{
    if (trNoPalQueueHead == trNoPalQueueTail)
    {
        //0-length queue
        return;
    }

    //free tail item
    if (trNoPalAllocated(trNoPalQueue[trNoPalQueueTail]))
    {
        trNoPalTextureDeleteFromTexreg(trNoPalQueue[trNoPalQueueTail]);
    }

    //move tail up
    trNoPalQueueTail = ADJ_NPQUEUE(trNoPalQueueTail+1);
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalQueueAdd
    Description : add an np texture to the queue
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalQueueAdd(udword handle)
{
    sdword head, targetBytes, prevBytes;

    //full-circle ?
    if (ADJ_NPQUEUE(trNoPalQueueHead+1) == trNoPalQueueTail)
    {
        trNoPalQueueFreeNext();
    }

    head = trNoPalQueueHead;
    trNoPalQueueHead = ADJ_NPQUEUE(trNoPalQueueHead+1);

    //need to make some room ?
    if (trNoPalBytesAllocated > trNoPalMaxBytes)
    {
        prevBytes = trNoPalBytesAllocated;
        targetBytes = trNoPalBytesAllocated * 2 / 3;

        //ASSERT: this will never be an infinite loop
        while (trNoPalBytesAllocated > targetBytes)
        {
            trNoPalQueueFreeNext();
        }

        dbgMessagef("\n** nopal freed %dMB of textures **",
                    (prevBytes - trNoPalBytesAllocated) >> 20);
    }

    //add to head
    trNoPalQueue[head] = handle;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalReadjust
    Description : free all np textures, clear the lra queue
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalReadjust(void)
{
    sdword index;

    if (!trNoPalettes)
    {
        return;
    }
    dbgAssert(trNoPalInitialized);

    dbgMessagef("\n** nopal freed %dMB of textures **", trNoPalBytesAllocated >> 20);

    for (index = 0; index < trRegistrySize; index++)
    {
        if (trNoPalAllocated(index))
        {
            trNoPalTextureDeleteFromTexreg(index);
        }
    }

    trNoPalQueueReset();
}

void trNoPalReadjustWithoutPending(void)
{
    sdword index;

    if (!trNoPalettes)
    {
        return;
    }
    dbgAssert(trNoPalInitialized);

    dbgMessagef("\n** nopalw/o freed %dMB of textures **", trNoPalBytesAllocated >> 20);

    for (index = 0; index < trRegistrySize; index++)
    {
        if (trNoPalAllocated(index))
        {
            trNoPalTextureDeleteFromTexregWithoutPending(index);
        }
    }

    trNoPalQueueReset();
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalGetHandle
    Description : obtain a fresh np handle
    Inputs      :
    Outputs     :
    Return      : an np handle
----------------------------------------------------------------------------*/
udword trNoPalGetHandle(void)
{
    udword handle;
    sdword index;

    //look for a handle in a fragmented pool.
    //this is less than efficient
    for (index = 0; index < trNoPalHighestAllocated; index++)
    {
        if (!trNoPalAllocated(index))
        {
            return index;
        }
    }

    handle = trNoPalHighestAllocated;

    trNoPalHighestAllocated++;
    dbgAssert(trNoPalHighestAllocated < trRegistrySize);

    return handle;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalTexImage
    Description : create an un-paletted version of a color index texture
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalTexImage(ubyte* data, ubyte* palette, sdword width, sdword height)
{
    ubyte* sp;
    ubyte* dp;
    sdword i, index;
    ubyte* rgba;
    uword* sdp;
    sdword r, g, b;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (texLinearFiltering)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    if (glRGBA16)
    {
        rgba = memAlloc(2 * width * height, "temp srgba", Pyrophoric);

        sdp = (uword*)rgba;
        sp = data;
        for (i = 0; i < width*height; i++, sdp++, sp++)
        {
            index = (*sp) << 2;
            r = palette[index + 0];
            g = palette[index + 1];
            b = palette[index + 2];
            *sdp = (0xF0 << 8) | ((r & 0xF0) << 4) | (g & 0xF0) | ((b & 0xF0) >> 4);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_RGBA16, GL_UNSIGNED_BYTE, rgba);

        memFree(rgba);

        trNoPalBytesAllocated += 2 * width * height;
    }
    else
    {
        rgba = memAlloc(4 * width * height, "temp rgba", Pyrophoric);

        dp = rgba;
        sp = data;
        for (i = 0; i < width*height; i++, dp += 4, sp++)
        {
            index = (*sp) << 2;
            dp[0] = palette[index + 0];
            dp[1] = palette[index + 1];
            dp[2] = palette[index + 2];
            dp[3] = palette[index + 3];
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

        memFree(rgba);

        trNoPalBytesAllocated += 4 * width * height;
    }
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalTextureCreate
    Description :
    Inputs      : data - 8bit image
                  palette - initial palette for the image
                  width, height - image dimensions
                  texreghandle - parent texreg struct handle
    Outputs     :
    Return      : an np handle
----------------------------------------------------------------------------*/
udword trNoPalTextureCreate(ubyte* data, ubyte* palette, sdword width, sdword height, trhandle texreghandle)
{
    nopalreg* reg;
    udword handle;

    handle = trNoPalGetHandle();

    reg = trNoPalStructure(handle);

    dbgAssert(reg->data == NULL);

    //not pending
//    dbgAssert(!bitTest(trStructure(texreghandle)->flags, TRF_NoPalPending));

    reg->texreghandle = texreghandle;
    reg->timeStamp[0] = taskTimeElapsed;//universe.totaltimeelapsed;
    reg->crc[0] = (udword)palette;

    //save 8bit image for re-palettizing
    reg->data = (ubyte*)memAlloc(width*height, "nopal data", NP_MEMFLAGS);
    memcpy(reg->data, data, width*height);
    trNoPalBytesAllocated += width * height;

    reg->width = width;
    reg->height = height;

    //create a rep w/ given palette
//    dbgAssert(reg->glhandle[0] == 0);
    glGenTextures(1, (GLuint*)&reg->glhandle[0]);
    glBindTexture(GL_TEXTURE_2D, reg->glhandle[0]);
    trNoPalTexImage(data, palette, width, height);

    //add handle to lra queue
    trNoPalQueueAdd(handle);

    return handle;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalTextureRecreate
    Description : 8bit image data still exists but all un-paletted reps are gone,
                  so recreate one
    Inputs      : palette - image palette
                  handle - an np handle
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalTextureRecreate(ubyte* palette, udword handle)
{
    nopalreg* reg;

    reg = trNoPalStructure(handle);

    dbgAssert(reg->data != NULL);
    dbgAssert(reg->texreghandle != TR_InvalidInternalHandle);

    //no longer pending
    bitClear(trStructure(reg->texreghandle)->flags, TRF_NoPalPending);

    //create a rep
    reg->crc[0] = (udword)palette;
//    dbgAssert(reg->glhandle[0] == 0);
    glGenTextures(1, (GLuint*)&reg->glhandle[0]);
    glBindTexture(GL_TEXTURE_2D, reg->glhandle[0]);
    trNoPalTexImage(reg->data, palette, reg->width, reg->height);

    //add handle to lra queue
    trNoPalQueueAdd(handle);

    reg->timeStamp[0] = taskTimeElapsed;//universe.totaltimeelapsed;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalMakeCurrent
    Description : bind an appropriate version of an np texture to the GL.
                  if a version of the image w/ given palette doesn't yet exist,
                  create it
    Inputs      : palette - the image palette
                  handle - an np handle
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalMakeCurrent(ubyte* palette, udword handle)
{
    nopalreg* reg;
    udword newcrc;
    sdword index, minIndex;
    real32 minTime;

    dbgAssert(trNoPalAllocated(handle));

    reg = trNoPalStructure(handle);

    //check for existing rep
    newcrc = (udword)palette;
    for (index = 0; index < trNpNumHandles; index++)
    {
        if (newcrc == reg->crc[index])
        {
            glBindTexture(GL_TEXTURE_2D, reg->glhandle[index]);
            return;
        }
    }

    //find a free spot
    for (index = 0; index < trNpNumHandles; index++)
    {
        if (reg->glhandle[index] == 0)
        {
            reg->crc[index] = newcrc;
            glGenTextures(1, (GLuint*)&reg->glhandle[index]);
            glBindTexture(GL_TEXTURE_2D, reg->glhandle[index]);
            trNoPalTexImage(reg->data, palette, reg->width, reg->height);
            reg->timeStamp[index] = taskTimeElapsed;//universe.totaltimeelapsed;
            return;
        }
    }

    //out of handles, free a rep (lru)
    minTime  = reg->timeStamp[0];
    minIndex = 0;
    for (index = 1; index < trNpNumHandles; index++)
    {
        if (reg->timeStamp[index] < minTime)
        {
            minTime  = reg->timeStamp[index];
            minIndex = index;
        }
    }
    trNoPalSingleRepDelete(handle, minIndex);
    reg->crc[minIndex] = newcrc;
    glGenTextures(1, (GLuint*)&reg->glhandle[minIndex]);
    glBindTexture(GL_TEXTURE_2D, reg->glhandle[minIndex]);
    trNoPalTexImage(reg->data, palette, reg->width, reg->height);
    reg->timeStamp[minIndex] = taskTimeElapsed;//universe.totaltimeelapsed;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalStartup
    Description : initialize and allocate necessary things for un-palettizing textures
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalStartup(void)
{
    glRGBA16 = glCapTexSupport(GL_RGBA16);

    if (trNoPalInitialized/* || !trNoPalettes*/)
    {
        return;
    }
    trNoPalRegistry = (nopalreg*)memAlloc(trRegistrySize * sizeof(nopalreg), "nopal registry", NP_MEMFLAGS);
    memset(trNoPalRegistry, 0, trRegistrySize * sizeof(nopalreg));
    trNoPalHighestAllocated = 0;
    trNoPalBytesAllocated = 0;

    trNoPalQueueStartup();

    trNoPalInitialized = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalReset
    Description : clear the queue, delete all reps and data
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalReset(void)
{
    sdword index;

    if (!trNoPalInitialized/* || !trNoPalettes*/)
    {
        return;
    }

    for (index = 0; index < trRegistrySize; index++)
    {
        if (trNoPalAllocated(index))
        {
            trNoPalTextureDeleteFromTexreg(index);//xxx
        }
    }

    trNoPalQueueReset();
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalShutdown
    Description : shutdown the un-palettizing structures
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalShutdown(void)
{
    sdword index;

//    ASSERT: trReset calls trNoPalReset
//    ASSERT: trShutdown calls trReset

    if (!trNoPalInitialized/* || !trNoPalettes*/)
    {
        return;
    }

    for (index = 0; index < trRegistrySize; index++)
    {
        if (trNoPalAllocated(index))
        {
            trNoPalTextureDelete(index);
        }
    }

    memFree(trNoPalRegistry);
    trNoPalRegistry = NULL;
    trNoPalHighestAllocated = 0;
//    dbgAssert(trNoPalBytesAllocated == 0);

    trNoPalQueueShutdown();

    trNoPalInitialized = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalResizePool
    Description : resize the pool of un-paletted textures
    Inputs      : mb - size of pool in megabytes
    Outputs     : trNoPalMaxBytes is set
    Return      :
----------------------------------------------------------------------------*/
void trNoPalResizePool(sdword mb)
{
    dbgMessagef("\ntrNoPalResizePool: %dMB", mb);
    trNoPalMaxBytes = mb << 20;
    trNoPalReadjust();
}

/*-----------------------------------------------------------------------------
    Name        : trNoPalFilter
    Description : enable / disable filtering on an np texture
    Inputs      : bEnable - TRUE or FALSE
                  index - texreg handle
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void trNoPalFilter(sdword bEnable, sdword handle)
{
    nopalreg* reg;
    sdword index;

    dbgAssert(handle != TR_InvalidInternalHandle);

    if (!trNoPalAllocated(handle))
    {
        return;
    }

    reg = trNoPalStructure(handle);

    for (index = 0; index < trNpNumHandles; index++)
    {
        if (reg->glhandle[index] != 0)
        {
            glBindTexture(GL_TEXTURE_2D, reg->glhandle[index]);
            if (bEnable)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : trTextureUsageList
    Description : Print out a texture usage list, much like texreg used to do.
                    We're doing it here because packed textures will confuse the
                    liflist results.
    Inputs      : fileName - file to dump the usage to.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if TR_TEXTURE_USAGE
void trTextureUsageList(char *fileName)
{
    FILE *fp;
    char directoryName[256], *pSlash;
    sdword index = 0;
    sdword size, directorySize = 0;
    sdword sharedSize, sharedDirectorySize = 0;
    texreg *reg;

    for (index = 0; index < trHighestAllocated && !trAllocated(index); index++)
    {
        ;
    }
    dbgAssert(trAllocated(index));
    strcpy(directoryName, trTextureRegistry[index].fileName);
    pSlash = directoryName;
    while ((pSlash = strchr(pSlash + 1, '\\')) != NULL)
    {
        if (!strchr(pSlash + 1, '\\'))
        {
            *pSlash = 0;
            break;
        }
    }
    fp = fopen(fileName, "wt");
    if (fp == NULL)
    {
        return;
    }
    reg = trTextureRegistry + index;
    for (; index < trHighestAllocated; index++, reg++)
    {
        if (!trAllocated(index))
        {
            continue;
        }
        if (strncmp(directoryName, reg->fileName, strlen(directoryName)))
        {                                                   //is this a new name?
            fprintf(fp, "%12d %12d %s\n", directorySize, sharedDirectorySize, directoryName);
            strcpy(directoryName, reg->fileName);
            pSlash = directoryName;
            while ((pSlash = strchr(pSlash + 1, '\\')) != NULL)
            {
                if (!strchr(pSlash + 1, '\\'))
                {
                    *pSlash = 0;
                    break;
                }
            }
            directorySize = 0;
            sharedDirectorySize = 0;
        }
        size = reg->diskWidth * reg->diskHeight;//size of texture
        if (bitTest(reg->flags, TRF_Alpha))
        {                                                   //account for alpha
            size *= sizeof(color);
            if (reg->flags & (TRF_TeamColor0 | TRF_TeamColor1))
            {                                               //account for texture duplication due to teams
                size *= MAX_MULTIPLAYER_PLAYERS;
            }
        }
        sharedSize = size;
        if (reg->sharedFrom != TR_NotShared)
        {                                                   //if this is a shared texture
            sharedSize = 0;
        }
        directorySize += size;
        sharedDirectorySize += sharedSize;
    }
    //print out the closing directory
    fprintf(fp, "%12d %12d %s\n", directorySize, sharedDirectorySize, directoryName);

    fclose(fp);
}
#endif //TR_TEXTURE_USAGE

