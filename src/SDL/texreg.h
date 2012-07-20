/*=============================================================================
    Name    : Texreg.h
    Purpose : Maintain a registry of textures in texture RAM

    Created 9/25/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___TEXREG_H
#define ___TEXREG_H

#include "Types.h"
#include "color.h"
#include "CRC32.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define TR_ALLOW_PCX                1           //load .PCX file if layered format not about
#define TR_SPECIAL_DEBUG            0           //special debugging modes active
#define TR_3DFX_HACKS               0
#define TR_ASPECT_CHECKING          1           //fixup extreme texture aspect ratios for GL

#ifndef HW_Release

#define TR_ERROR_CHECKING           1           //general error checking
#define TR_VERBOSE_LEVEL            1           //control specific output code
#define TR_DEBUG_TEXTURES           1           //enable special 'debug' textures
#define TR_PRINT_TEXTURE_NAMES      0           //special extra-verbose printing
#define TR_TEXTURE_USAGE            1           //create a file listing the textures loaded
#define TR_VISUALIZE_SHARING        1           //modify textures that are shared so you can see them
#define TR_NIL_TEXTURE              1           //enable the /nilTexture option

#else //HW_Debug

#define TR_ERROR_CHECKING           0           //general error checking
#define TR_VERBOSE_LEVEL            0           //control specific output code
#define TR_DEBUG_TEXTURES           0           //enable special 'debug' textures
#define TR_PRINT_TEXTURE_NAMES      0           //special extra-verbose printing
#define TR_TEXTURE_USAGE            0           //create a file listing the textures loaded
#define TR_VISUALIZE_SHARING        0           //modify textures that are shared so you can see them
#define TR_NIL_TEXTURE              0           //enable the /nilTexture option

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//general definitions
//dag nammit, this is just silly large!  Let's reduce it down to a more
//reasonable size once all the missions are finalized.
#define TR_RegistrySize             6000
#define TR_NumPalettesPerTexture    8           //maximum palettes per texture
#define TR_NumPaletteSize           (sizeof(trcolorinfo) * TR_NumPalettesPerTexture)
#define TR_ColoringThreshold        128
#define TR_PaletteAlreadyFreed      0xabadcafe  //signified palette already freed
#define TR_NP_NumHandles            4

//flags for an entry in the texture registry (must all fit in a ubyte)
#define TRF_Pending                 0x00000001  //waiting to be loaded
#define TRF_Paletted                0x00000002  //texture uses a palette
#define TRF_Allocated               0x00000004  //entry is in use
#define TRF_Alpha                   0x00000008  //alpha channel image
#define TRF_TeamColor0              0x00000010  //team color flags
#define TRF_TeamColor1              0x00000020
#define TRF_NoPalPending            0x00000040
//#define TRF_IgnoreDimensions        0x00000080  //ignore dimensions (they're probably unusual)
#define TRF_SharedFileName          0x00000080  //filename is followed by another name for the file it is shared from
#define TRM_CompareFlags            0
#define TRM_ListFlag                (TRF_Alpha | TRF_TeamColor0 | TRF_TeamColor1)

#define TR_Invalid                  0xffffffff
#define TR_InvalidHandle            0xffffffff
#define TR_InvalidInternalHandle    0xffffffff
#define TR_InvalidInternalPending   0xfffffffe
#define TR_NotShared                0xfdfbfaf5

#define TR_PaletteLength            256
#define TR_PaletteType              GL_COLOR_INDEX8_EXT
#define TR_RGBType                  GL_RGB
#define TR_RGBAType                 GL_RGBA
#define TR_PaletteEntrySize         (sizeof(color))
#define TR_PaletteSize              (TR_PaletteEntrySize * TR_PaletteLength)

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

//info for texture memory mapper
#define TR_NumberRamPools           8           //maximum number of RAM pools
#define TR_AvailabilityMargin       1/6         //fraction of total RAM unused to be considered a 'good fit'
#define TR_MaxCramIterations        5           //maximum number of times to try and fit things into RAM
#define TR_MinimumSizeX             1           //minimum texture size
#define TR_MinimumSizeY             1           //minimum texture size
#define TR_TexSizesX                10          //number of sizes thereafter
#define TR_TexSizesY                10          //1,2,4,8,16,32,64,128,256,512(512!)
#define TR_SSBMinX                  0           //1 << TR_SSBMinX = TR_MinimumSizeX
#define TR_SSBMinY                  0           //1 << TR_SSBMinY = TR_MinimumSizeY
#define TR_PerScaleFactor           (10 * (TR_TexSizesX + TR_TexSizesY))//for adjusting scale factor per-size

//info for texture listing
#define TR_LifListing               "textures.ll"
#define LL_FileIdentifier           "Event13"
#define LL_FileVersion              0x104
#define LL_MaxElements              12000

//authentication info for .LiF files
#define LIF_FileIdentifier          "Willy 7"   //authentication identifier
#define LIF_FileVersion             0x104       //version number

#define TR_BadCRC                   0xdeadb00b

//special debugging texture modes
#define TSM_None                    0
#define TSM_Uncolored               1
#define TSM_TeamColor0              2
#define TSM_TeamColor1              3
#define TSM_NumberModes             4

#define TR_SortListAllocationStep   16
#define TR_UnusedColorInfo          colRGBA(0, 0, 0, 0)
#define TR_UnusedColorPalette       colRGB(64, 255, 64)

//maximum size of textures
#define TR_TextureWidthMax          256
#define TR_TextureHeightMax         256

#define TR_MeshSortGrowBy           10

/*=============================================================================
    Type definitions:
=============================================================================*/
//type of a handle from the texture registry
typedef udword trhandle;

//information for coloring a texture
typedef struct
{
    color base;
    color detail;
}
trcolorinfo;

//structure for an entry in the texture registry
typedef struct
{
    sword diskWidth, diskHeight;                //dimensions of texture (on disk, even if it is scaled down at loading)
    sword scaledWidth, scaledHeight;            //dimensions of texture in RAM
    sdword nUsageCount;                          //number of times this texture in use
    ubyte flags;                                //flags for the texture (see above)
    sbyte nPalettes;                            //number of palettes associated with this texture
    sbyte currentPalette;                       //which palette this texture was last bound with
    ubyte nSharedTo;
    crc32 paletteCRC;                           //crc of palette image came from
    ubyte  *palettes;                           //pointer to the palette entries
    void *meshReference;                        //reference to associated mesh
    char *fileName;                             //CRC/name of file represented by this texture
    udword handle;                              //actual texture reference
    uword baseScalar, stripeScalar;             //team color effect modifiers
#if TR_DEBUG_TEXTURES
    udword uncolored;                           //uncolored image and palette
    color *uncoloredPalette;
    udword col0Handle;                          //textures of just the team effect buffers
    udword col1Handle;
#endif
    udword sharedFrom;                          //if a shared texture, where does the real texture reside? (TR_NotShared if not shared)
}
texreg;

//structure for an entry in the no-palette texture registry
typedef struct
{
    udword glhandle[TR_NP_NumHandles];  //GL texture object handles
    udword crc[TR_NP_NumHandles];       //palette CRCs (base pointer, actually)
    real32 timeStamp[TR_NP_NumHandles]; //LRU per-nopalreg
    ubyte* data;                        //8bit image
    sdword width, height;               //image dimensions
    trhandle texreghandle;              //parent's handle
}
nopalreg;

//structure for a per-mesh sorted list
typedef struct
{
    void *meshReference;                        //mesh-reference
    crc32 paletteCRC;                           //for sorting by
    sdword nAllocated;                          //number of textures allocated
    sdword nTextures;                           //number of textures in mesh
    sdword *textureList;                        //list of textures for list
    udword teamFlags;                           //team flags for consistently colorizing the whole list
}
trmeshsort;

//structure used for memory mapping all the textures
typedef struct
{
    sdword index;                               //index in texture registry
    sdword width, height;                       //new size of texture
}
trmemmap;

//structure for a pool of texture RAM which textures can be fit into
typedef struct
{
    void *base;                                 //not used in all implementations, refers to the base of this texture RAM pool
    sdword length;                              //length of this pool
    sdword used;                                //amount of this pool already used
}
trrampool;

//structures for a .LiF listing file which will speed texture loading
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

//format for the .LiF files
typedef struct
{
    char ident[8];                              //compared to "Willy 7"
    sdword version;                             //version number
    sdword flags;                               //to plug straight into texreg flags
    sdword width, height;                       //dimensions of image
    crc32 paletteCRC;                           //a CRC of palettes for fast comparison
    crc32 imageCRC;                             //crc of the unquantized image
    ubyte *data;                                //actual image
    color *palette;                             //palette for this image
    ubyte *teamEffect0, *teamEffect1;           //palettes of team color effect
//    sdword nSizes;                              //number of sizes encoded
//    udword image[LLC_NumberImages];             //offset-pointers to the individual images (from end of this header)
}
lifheader;

/*=============================================================================
    Macros:
=============================================================================*/
#define trIndex(handle)         ((handle) & 0x0000ffff)
#define trPaletteIndex(handle)  ((handle)  >> 16)
#define trStructure(handle)     (&trTextureRegistry[trIndex(handle)])
#define trHandleMake(index, palette) (index | (palette << 16))
//#define trColorsEqual(a, b)     (((a) == (b)) || (((a) != NULL) && ((b) != NULL) && ((a)->base == (b)->base) && ((a)->detail == (b)->detail)))
#define trAllocated(index)      (bitTest(trTextureRegistry[index].flags, TRF_Allocated))
#define trSetAllocated(index)   (bitSet(trTextureRegistry[index].flags, TRF_Allocated))
#define trSetFree(index)        (bitClear(trTextureRegistry[index].flags, TRF_Allocated))
//#define trPending(index)        (trTextureRegistry[index].handle == TR_InvalidInternalPending)
#define trPending(index)        (bitTest(trTextureRegistry[index].flags, TRF_Pending))
//#define trSetPending(index)     (trTextureRegistry[index].handle = TR_InvalidInternalPending)
#define trSetPending(index)     (bitSet(trTextureRegistry[index].flags, TRF_Pending))
#define trClearPending(index)     (bitClear(trTextureRegistry[index].flags, TRF_Pending))
#define trUnusedInfo(c)     ((c)->base == TR_UnusedColorInfo && (c)->detail == TR_UnusedColorInfo)

#define trNoPalStructure(index) (&trNoPalRegistry[index])
#define trNoPalAllocated(index) (trNoPalStructure(index)->data != NULL)

/*=============================================================================
    Data:
=============================================================================*/
extern char *trBaseColorNames[];
extern char *trDetailColorNames[];
extern char *trOtherLayerNames[];
extern char *trAllLayerNames[];
#if TR_DEBUG_TEXTURES
extern bool trSpecialTextures;
#endif
extern sdword texLinearFiltering;               //enable bi-linear filtering

extern real32 trBaseColorScalar;
extern real32 trStripeColorScalar;

extern bool trNoPalettes;

#if TR_NIL_TEXTURE
extern bool GLOBAL_NO_TEXTURES;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
//startup/shutdown and reset texture registry
void trStartup(void);
void trShutdown(void);
void trReset(void);
void trReload(void);    //fixup OpenGL .DLL hooks

//queue up a texture for loading
trhandle trTextureRegister(char *fileName, trcolorinfo *info, void *meshReference);
//now change one or more of it's colors
void trTextureColorsUpdate(trhandle handle, trcolorinfo *info);
trhandle trRegisterAddition(trhandle handle, trcolorinfo *info);
void trRegisterRemoval(trhandle handle/*, trcolorinfo *info*/);
void trInternalTexturesDelete(trhandle handle);

//'refresh' the texture registry which will load in all the textures requested.
void trRegistryRefresh(void);

//'unregister' a texture handle, i.e. decrement it's usage count
sdword trTextureUnregister(trhandle handle);
sdword trTextureDelete(trhandle handle);
void trTextureDeleteAllUnregistered(void);

//clear the current texture
void trClearCurrent(void);

//make a given texture current for rendering
void trMakeCurrent(trhandle handle);

//functions for directly manipulating textures, without having to go through
//the texture registry
udword trPalettedTextureCreate(ubyte *data, color *palette, sdword width, sdword height);
udword trRGBTextureCreate(color *data, sdword width, sdword height, bool useAlpha);
void trRGBTextureUpdate(udword handle, color *data, sdword width, sdword height);
void trPalettedTextureMakeCurrent(udword handle, color *palette);
void trRGBTextureMakeCurrent(udword handle);
void trRGBTextureDelete(udword handle);
lifheader *trLIFFileLoad(char *fileName, udword flags);
void trBufferColorRGB(color *dest, color *source, ubyte *teamEffect0, ubyte *teamEffect1, color teamColor0, color teamColor1, sdword size, udword flags, real32 effectScalar0, real32 effectScalar1);

//misc utility functions
sdword trLayerNameMatch(char *name, char **nameList);
void trFilterEnable(sdword bEnable);
texreg *trStructureGet(trhandle handle);
color *trImageScale(color *data, sdword width, sdword height, sdword newWidth, sdword newHeight, bool bFree);
ubyte *trImageScaleIndexed(ubyte *data, sdword width, sdword height, sdword newWidth, sdword newHeight, bool bFree);
void trSetAllPending(bool freeNoPal);
void trTextureUsageList(char *fileName);
sdword trColorsEqual(trcolorinfo *info, sdword textureIndex);

//no-palette palette functions
void trNoPalStartup(void);
void trNoPalReset(void);
void trNoPalShutdown(void);
void trNoPalTextureDelete(udword handle);
void trNoPalTextureRecreate(ubyte* palette, udword handle);
void trNoPalMakeCurrent(ubyte* palette, udword handle);
udword trNoPalTextureCreate(ubyte* data, ubyte* palette, sdword width, sdword height, trhandle texreghandle);
void trNoPalResizePool(sdword mb);
void trNoPalReadjust(void);
void trNoPalFilter(sdword bEnable, sdword handle);

#endif //___TEXREG_H

