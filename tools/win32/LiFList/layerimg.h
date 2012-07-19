/*=============================================================================
    Name    : LayerImg.h
    Purpose : Routines for manipulating layered images.

    Created 9/29/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___LAYERIMG_H
#define ___LAYERIMG_H

#include "prim2d.h"
#include "color.h"

/*=============================================================================
    Switches
=============================================================================*/
#ifdef HW_Debug

#define LI_ERROR_CHECKING           1           //general error checking
#define LI_VERBOSE_LEVEL            0           //control specific output code
#define LI_RETAIN_NAME              1           //keep a copy of the file name around
#define LI_NAMED_LAYER_WARNINGS     1           //present earnings if cannot find named layers

#else //HW_Debug

#define LI_ERROR_CHECKING           0           //general error checking
#define LI_VERBOSE_LEVEL            0           //control specific output code
#define LI_RETAIN_NAME              0           //keep a copy of the file name around
#define LI_NAMED_LAYER_WARNINGS     0           //present earnings if cannot find named layers

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/

//layer blending types
#define LBT_Normal              0
#define LBT_Darken              1
#define LBT_Lighten             2
#define LBT_Hue                 3
#define LBT_Saturation          4
#define LBT_Color               5
#define LBT_Luminosity          6
#define LBT_Multiply            7
#define LBT_Screen              8
#define LBT_Dissolve            9
#define LBT_Overlay             10
#define LBT_HardLight           11
#define LBT_SoftLight           12
#define LBT_Difference          13
#define LBT_ColorDodge          14
#define LBT_ColorBurn           15
#define LBT_Exclusion           16

//layer flags
#define LFF_ClippingBase        0x0001          //pertains to clipping groups
#define LFF_TransProtect        0x0002          //transparency protect??
#define LFF_Visible             0x0004          //layer is visible and subject to blending
#define LFF_Channeled           0x0008          //channels is a set of RGBA planes
#define LFF_Deleted             0x0010          //layer has already been deleted
#define LFF_TeamColor0          0x0020          //layer is base color layer
#define LFF_TeamColor1          0x0040          //layer is stripe team color

//channel types
#define LCT_Red                 0               //first 3 same as in .PSD file
#define LCT_Green               1
#define LCT_Blue                2
#define LCT_Transparency        3               //these last 2 are somewhat similar
#define LCT_LayerMask           4               //there can be more than 1 of these
#define LCT_Invalid             5               //for a deleted/ignored channel

//per-color lkuminence gamma
#define LI_RedGamma             0.45f
#define LI_GreenGamma           0.80f
#define LI_BlueGamma            0.25f
#define LI_TotalGamma           1.5f

/*=============================================================================
    Type definitions:
=============================================================================*/
// Here are the assumptions we make about the layered image:
//  -RGBA mode only.  We won't support any other modes.
//  -All layers are only the size of the rectangle specified.
//  -All layers are compressed if they were compressed in the .PSD file.
//      They can be selectively decompressed.
//  -All name strings C mode, NULL-terminated.

//structure for a blending range
typedef struct
{   //top/bottom refers to which layer.  Bottom is underlying layer; top is layer being composited.
    //High/low refers to limits of range.  Uses full range: 0 - ffff
    uword bottomLow;
    uword bottomHigh;
    uword topLow;
    uword topHigh;
}
liblendrange;

//structure for a single channel in a layer
typedef struct
{
    liblendrange blendRange;                    //blending range info
    uword *scanLength;                          //compressed size of each scanline (bounds.bottom - bounds.top)
    ubyte *scanData;                            //actual scan-line data for the channel
    ubyte type;                                 //type of channel (see above)
    ubyte compressed;                           //nonzero if RLE-compressed
}
lichannel;

//structure for a layer mask (same as .PSD disk format)
/*
typedef struct
{
    rectangle bounds;                           //limits of the mask
    ubyte defaultAlpha;                         //'color' of the mask
    ubyte flags;                                //flags (see above)
    ubyte padding[2];                           //round off the size
}
lilayermask;
*/
//structure for a single layer in a layered image
typedef struct
{
    rectangle bounds;                           //limits of layer inclusive-exclusive
    uword flags;                                //(see above)
    uword blendMode;                            //blending mode (see above)
    real32 opacity;                             //0.0 .. 1.0
    char *name;                                 //name of layer
    liblendrange greyBlendRange;
    sword nChannels;                            //number of channels in image
    lichannel *channels;                        //list of channels or uncompressed bitmaps
    color *decompressed;                        //decompressed image
    color average;                              //average color of this layer
}
lilayer;

//structure for internal layered image format.
typedef struct
{
#if LI_RETAIN_NAME
    char *fileName;                             //name this image represents
#endif
    sdword width, height;                       //dimensions of image
    sdword nLayers;                             //number of layers
    lilayer layers[1];                          //layers in this image
}
layerimage;

/*=============================================================================
    Macros:
=============================================================================*/
#define liHeaderSize(nLayers)   (sizeof(layerimage) + sizeof(lilayer) * ((nLayers) - 1))

/*=============================================================================
    Functions:
=============================================================================*/

//find a layer in the image
lilayer *liLayerFindByName(layerimage *image, char *name);

//compress/decompress/delete a layer
void liLayerDecompress(lilayer *layer);
void liLayerDelete(lilayer *layer);

//manimulate color of a layer
void liLayerColorSolid(lilayer *layer, color newColor, ubyte threshold);
color liLayerColorAverage(lilayer *layer);

//delete an entire layered file
void liImageDelete(layerimage *image);

//blending layers together
void liLayerBlendTo(color *destBuffer, sdword layerIndex, layerimage *image);
color *liBlendBufferPrepare(layerimage *image, ubyte **newTeamColor0, ubyte **newTeamColor1);
void liInsertChannelCompose(color *destBuffer, sdword layerIndex, layerimage *image, udword sChannel, udword dChannel);

#endif //___LAYERIMG_H


