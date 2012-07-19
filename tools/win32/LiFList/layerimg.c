/*=============================================================================
    Name    : LayerImg.c
    Purpose : Functions to manipulate layered images

    Created 9/29/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "debug.h"
#include "memory.h"
#include "color.h"
#include "layerimg.h"

/*=============================================================================
    Data:
=============================================================================*/
/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : liLayerFindByName
    Description : Find a named layer in the specified image.
    Inputs      : image - image to search for name in.
                  name - name of layer to search for.
    Outputs     : ..
    Return      : Pointer to named layer, or NULL if it doesn't exist.
----------------------------------------------------------------------------*/
lilayer *liLayerFindByName(layerimage *image, char *name)
{
    sdword index;

    for (index = 0; index < image->nLayers; index++)
    {                                                       //for all layers
        if (!strcmp(name, image->layers[index].name))
        {                                                   //if layer names match
            return(&image->layers[index]);                  //return pointer to layer
        }
    }
#if LI_NAMED_LAYER_WARNINGS
    dbgWarningf(DBG_Loc, "Layer '%s' does not exist in layered image '%s'.", name,
#if LI_RETAIN_NAME
        image->fileName);
#else
        "<no name available>");
#endif //LI_RETAIN_NAME
#endif //LI_NAMED_LAYER_WARNINGS
    return(NULL);                                           //no names layer found
}

/*-----------------------------------------------------------------------------
    Name        : liImageDelete
    Description : Deletes a layered image and all data associated therewith.
    Inputs      : image - layered image to delete
    Outputs     : Frees all memory, including the image itself (which must
                    therefore have been independently allocated).
    Return      : void
----------------------------------------------------------------------------*/
void liImageDelete(layerimage *image)
{
    sdword index, j;

#if LI_VERBOSE_LEVEL >= 1
    dbgMessagef("\nliImageDelete: deleting image 0x%x, filename '%s'", image,
#if LI_RETAIN_NAME
        image->fileName);
#else
        "<no name available>");
#endif //LI_RETAIN_NAME
#endif //LI_VERBOSE_LEVEL
    for (index = 0; index < image->nLayers; index++)
    {                                                       //for each layer
        if (bitTest(image->layers[index].flags, LFF_Deleted))
        {
            continue;
        }
        if (image->layers[index].flags & LFF_Channeled)
        {                                                   //if layer is channeled
            for (j = 0; j < image->layers[index].nChannels; j++)
            {                                               //for each channel of each layer
                if (image->layers[index].channels[j].scanLength != NULL)
                {                                           //if a RLE encoded channel
                    memFree(image->layers[index].channels[j].scanLength);//free the channel image data
                }
                else
                {                                           //else a RAW channel
                    memFree(image->layers[index].channels[j].scanData);//free the channel image data
                }
            }
            if (image->layers[index].channels)
            {
                memFree(image->layers[index].channels);     //free the channel list
            }
        }
        else
        {
            if (image->layers[index].decompressed)
            {                                               //if decompressed, free decompressed buffer
                memFree(image->layers[index].decompressed);
            }
        }
        if (image->layers[index].name)
        {
            memFree(image->layers[index].name);             //free the name
        }
    }
#if LI_RETAIN_NAME
    memFree(image->fileName);                               //free image file name
#endif
    memFree(image);                                         //free the actual image
}

/*-----------------------------------------------------------------------------
    Name        : liChannelDecompressRaw
    Description : 'Decompress' a single raw channel into an RGBA buffer
    Inputs      : buffer - where to decode image to
                  data - RLE compressed data
                  width - width of buffer
                  height - height of buffer
                  offset - color-element offset (0..3)
    Outputs     : copies channel into interleaved RGBA data
    Return      : void
----------------------------------------------------------------------------*/
void liChannelDecompressRaw(ubyte *buffer, ubyte *data, sdword width, sdword height, sdword offset)
{
    sdword count;

    count = width * height;
    buffer += offset;

    while (count > 0)
    {
        *buffer = *data;                                    //copy the pixel
        buffer += 4;
        data++;
        count--;
    }
}

/*-----------------------------------------------------------------------------
    Name        : liChannelDecompressRLE
    Description : Decompress a single RLE channel into a RGBA buffer
    Inputs      : buffer - where to decode image to
                  scanLength - array of number of bytes per scanline
                  data - RLE compressed data
                  width - width of buffer
                  height - height of buffer
                  offset - color-element offset (0..3)
    Outputs     : fills buffer with decompressed channel data
    Return      : void
----------------------------------------------------------------------------*/
void liChannelDecompressRLE(ubyte *buffer, uword *scanLength, ubyte *data, sdword width, sdword height, sdword offset)
{
    sdword index, x, y;
#if LI_ERROR_CHECKING
    sdword nRead;
#endif
    sbyte opCode;

    buffer += offset;
    for (y = 0; y < height; y++)
    {
#if LI_ERROR_CHECKING
        nRead = 0;
#endif
        for (x = 0; x < width;)
        {
            opCode = *data;
            data++;
#if LI_ERROR_CHECKING
            nRead++;
#endif
            if (opCode >= 0)
            {                                               //literal run
                for (index = 0; index < (sdword)opCode + 1; index++)
                {
                    *buffer = (ubyte)*data;
                    buffer += 4;
                    data++;
#if LI_ERROR_CHECKING
                    nRead++;
#endif
                    x++;
                }
            }
            else if (opCode > -128)
            {
                opCode = -opCode;
                for (index = 0; index < (sdword)opCode + 1; index++)
                {
                    *buffer = (ubyte)*data;
                    buffer += 4;
                    x++;
                }
                data++;
#if LI_ERROR_CHECKING
                nRead++;
#endif
            }
        }
//        buffer += (width - x) * 4;                          //skip straggler pixels at end
#if LI_ERROR_CHECKING
        dbgAssert(x == width);
        dbgAssert(nRead == scanLength[y]);
#endif
    }
}

/*-----------------------------------------------------------------------------
    Name        : liLayerDecompress
    Description : Decompress a layer
    Inputs      : layer - pointer to layer to decompress
    Outputs     : Allocates memory for decompressed layer and frees old
                    compressed version.  Newly decompressed layer will have
                    the size of the layer bounds, not full image size.
                    It will be referenced by layer->channels (cast to color *).
    Return      : void
----------------------------------------------------------------------------*/
void liLayerDecompress(lilayer *layer)
{
    color *newBuffer;
    sdword width = layer->bounds.x1 - layer->bounds.x0;
    sdword height = layer->bounds.y1 - layer->bounds.y0;
    sdword index, offset;

#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nliLayerDecompress: decompressing '%s' bounds (%d, %d), (%d, %d)",
                layer->name, layer->bounds.x0, layer->bounds.y0, layer->bounds.x1, layer->bounds.x1);
#endif //LI_VERBOSE_LEVEL
                                                            //create new buffer
    newBuffer = memAlloc(width * height * sizeof(color), "Decompressed layer", 0);
    memset(newBuffer, 0, width * height * sizeof(color));   //set buffer to fully transparent black
    dbgAssert(layer->channels);

    for (index = 0; index < layer->nChannels; index++)
    {                                                       //for all channels
        if (layer->channels[index].type == LCT_Invalid)
        {
            continue;
        }
        if (layer->channels[index].type == LCT_LayerMask)
        {                                                   //compute color-element offset
            offset = 3;
        }
        else
        {
            offset = (sdword)layer->channels[index].type;
        }
        if (layer->channels[index].compressed)
        {                                                   //if RLE channel
            liChannelDecompressRLE((ubyte *)newBuffer, layer->channels[index].scanLength,
                                layer->channels[index].scanData,//decompress the channel
                                layer->bounds.x1 - layer->bounds.x0,
                                layer->bounds.y1 - layer->bounds.y0, offset);
            memFree(layer->channels[index].scanLength);     //free channel image data
        }
        else
        {                                                   //else it's a raw channel
            liChannelDecompressRaw((ubyte *)newBuffer,      //decompress the channel
                                layer->channels[index].scanData,
                                layer->bounds.x1 - layer->bounds.x0,
                                layer->bounds.y1 - layer->bounds.y0, offset);
            memFree(layer->channels[index].scanData);       //free channel image data
        }
    }
    memFree(layer->channels);                               //free the channel list
    bitClear(layer->flags, LFF_Channeled);                  //set state to non-channeled
    layer->decompressed = newBuffer;                        //store newly decompressed layer
}

/*-----------------------------------------------------------------------------
    Name        : liLayerDelete
    Description : Delete all data for a given layer
    Inputs      : layer - layer to delete
    Outputs     : frees all memory associated with layer and sets pointers to NULL.
    Return      :
----------------------------------------------------------------------------*/
void liLayerDelete(lilayer *layer)
{
    sdword index;
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nliLayerDelete: deleting '%s' bounds (%d, %d), (%d, %d)",
                layer->name, layer->bounds.x0, layer->bounds.y0, layer->bounds.x1, layer->bounds.x1);
#endif //LI_VERBOSE_LEVEL
    if (layer->flags & LFF_Channeled)
    {                                                       //if layer is channeled
        if (layer->channels)
        {
            for (index = 0; index < layer->nChannels; index++)
            {                                                   //for each channel of each layer
                if (layer->channels[index].type == LCT_Invalid)
                {
                    continue;
                }
                if (layer->channels[index].scanLength != NULL)
                {
                    memFree(layer->channels[index].scanLength);//free the channel image data
                    layer->channels[index].scanLength = NULL;
                }
                else
                {
                    memFree(layer->channels[index].scanData);
                    layer->channels[index].scanData = NULL;
                }
            }
            memFree(layer->channels);                       //free the channel list
            layer->channels = NULL;
        }
    }
    else
    {
        if (layer->decompressed)
        {                                                   //if decompressed, free decompressed buffer
            memFree(layer->decompressed);
            layer->decompressed = NULL;
        }
        layer->channels = NULL;
    }
    if (layer->name)
    {
        memFree(layer->name);                               //free the name
    }
    bitSet(layer->flags, LFF_Deleted);
}

/*-----------------------------------------------------------------------------
    Name        : liBlendxxxx
    Description : Layer blending functions
    Inputs      : buffer - buffer pixels to blend into
                  layer - layer (source) pixels to blend from
                  opacity - opacity of layer
                  nPixels - number of pixels to blend
    Outputs     : blends pixels from layer into buffer
    Return      : void
    Notes       : As pixels are stored, alpha info is discarded.
        Generally, blending is handled as such:
            destVal = (sourceVal <op> destVal) * alpha + destVal * (1 - alpha);
        Where <op> is some operation and alpha is pixelAlpha * sourceLayerAlpha.
----------------------------------------------------------------------------*/
void liBlendNormal(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
    real32 redSource, greenSource, blueSource, alpha;
    real32 redDest, greenDest, blueDest;//, alphaDest;
    real32 oneMinusAlpha;

    while (nPixels > 0)
    {
        alpha = colUbyteToReal(colAlpha(*layer)) * opacity;
        oneMinusAlpha = 1.0f - alpha;
        redSource = colUbyteToReal(colRed(*layer));    //read pixel to floating point
        greenSource = colUbyteToReal(colGreen(*layer));
        blueSource = colUbyteToReal(colBlue(*layer));
        redDest = colUbyteToReal(colRed(*buffer));
        greenDest = colUbyteToReal(colGreen(*buffer));
        blueDest = colUbyteToReal(colBlue(*buffer));

        redDest = redSource * alpha + redDest * oneMinusAlpha;
        greenDest = greenSource * alpha + greenDest * oneMinusAlpha;
        blueDest = blueSource * alpha + blueDest * oneMinusAlpha;
        *buffer = colRGB(colRealToUbyte(redDest), colRealToUbyte(greenDest),
                         colRealToUbyte(blueDest));
        buffer++;
        layer++;
        nPixels--;
    }
}

void liBlendMultiply(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
    real32 redSource, greenSource, blueSource, alpha;
    real32 redDest, greenDest, blueDest;//, alphaDest;
    real32 oneMinusAlpha;

    while (nPixels > 0)
    {
        alpha = colUbyteToReal(colAlpha(*layer)) * opacity;
        oneMinusAlpha = 1.0f - alpha;
        redSource = colUbyteToReal(colRed(*layer));    //read pixel to floating point
        greenSource = colUbyteToReal(colGreen(*layer));
        blueSource = colUbyteToReal(colBlue(*layer));
        redDest = colUbyteToReal(colRed(*buffer));
        greenDest = colUbyteToReal(colGreen(*buffer));
        blueDest = colUbyteToReal(colBlue(*buffer));

        redDest = redSource * redDest * alpha + redDest * oneMinusAlpha;
        greenDest = greenSource * greenDest * alpha + greenDest * oneMinusAlpha;
        blueDest = blueSource * blueDest * alpha + blueDest * oneMinusAlpha;
        *buffer = colRGB(colRealToUbyte(redDest), colRealToUbyte(greenDest),
                         colRealToUbyte(blueDest));
        buffer++;
        layer++;
        nPixels--;
    }
}

void liBlendScreen(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
    real32 redSource, greenSource, blueSource, alpha;
    real32 redDest, greenDest, blueDest;//, alphaDest;
    real32 oneMinusAlpha;

    while (nPixels > 0)
    {
        alpha = colUbyteToReal(colAlpha(*layer)) * opacity;
        oneMinusAlpha = 1.0f - alpha;
        redSource = colUbyteToReal(colRed(*layer));    //read pixel to floating point
        greenSource = colUbyteToReal(colGreen(*layer));
        blueSource = colUbyteToReal(colBlue(*layer));
        redDest = colUbyteToReal(colRed(*buffer));
        greenDest = colUbyteToReal(colGreen(*buffer));
        blueDest = colUbyteToReal(colBlue(*buffer));

        redDest = (1.0f - (1.0f - redDest) * (1.0f - redSource)) * alpha + redDest * oneMinusAlpha;
        greenDest = (1.0f - (1.0f - greenDest) * (1.0f - greenSource)) * alpha + greenDest * oneMinusAlpha;
        blueDest = (1.0f - (1.0f - blueDest) * (1.0f - blueSource)) * alpha + blueDest * oneMinusAlpha;
        redDest = min(redDest, 1.0f);
        greenDest = min(greenDest, 1.0f);
        blueDest = min(blueDest, 1.0f);
        *buffer = colRGB(colRealToUbyte(redDest), colRealToUbyte(greenDest),
                         colRealToUbyte(blueDest));
        buffer++;
        layer++;
        nPixels--;
    }
}

void liBlendOverlay(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
    real32 redSource, greenSource, blueSource, alpha;
    real32 redDest, greenDest, blueDest;//, alphaDest;
    real32 oneMinusAlpha;
    real32 redTemp, greenTemp, blueTemp;

    while (nPixels > 0)
    {
        alpha = colUbyteToReal(colAlpha(*layer)) * opacity;
        oneMinusAlpha = 1.0f - alpha;
        redSource = colUbyteToReal(colRed(*layer));    //read pixel to floating point
        greenSource = colUbyteToReal(colGreen(*layer));
        blueSource = colUbyteToReal(colBlue(*layer));
        redDest = colUbyteToReal(colRed(*buffer));
        greenDest = colUbyteToReal(colGreen(*buffer));
        blueDest = colUbyteToReal(colBlue(*buffer));
        if (redDest < 0.5f)
        {
            redTemp = (2.0f * redSource * 2.0f * redDest) / 2.0f;
        }
        else
        {
            redTemp = 1.0f - ((2.0f * (1.0f - redSource)) * (2.0f * (1.0f - redDest)) / 2.0f);
        }
        if (greenDest < 0.5f)
        {
            greenTemp = (2.0f * greenSource * 2.0f * greenDest) / 2.0f;
        }
        else
        {
            greenTemp = 1.0f - ((2.0f * (1.0f - greenSource)) * (2.0f * (1.0f - greenDest)) / 2.0f);
        }
        if (blueDest < 0.5f)
        {
            blueTemp = (2.0f * blueSource * 2.0f * blueDest) / 2.0f;
        }
        else
        {
            blueTemp = 1.0f - ((2.0f * (1.0f - blueSource)) * (2.0f * (1.0f - blueDest)) / 2.0f);
        }
        redDest = redTemp * alpha + redDest * oneMinusAlpha;
        greenDest = greenTemp * alpha + greenDest * oneMinusAlpha;
        blueDest = blueTemp * alpha + blueDest * oneMinusAlpha;
        redDest = min(redDest, 1.0f);
        greenDest = min(greenDest, 1.0f);
        blueDest = min(blueDest, 1.0f);
        *buffer = colRGB(colRealToUbyte(redDest), colRealToUbyte(greenDest),
                         colRealToUbyte(blueDest));
        buffer++;
        layer++;
        nPixels--;
    }
}

//This one not a perfect copy, but pretty close
void liBlendColor(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
    real32 redSource, greenSource, blueSource, alpha;
    real32 redDest, greenDest, blueDest;//, alphaDest;
    real32 oneMinusAlpha;
    real32 hueS, satS, hueD, satD, lumS, lumD;
    real32 clumD, clumS;
    real32 redTemp, greenTemp, blueTemp;

#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nColor: Blending mode not implemented perfectly.");
#endif

    while (nPixels > 0)
    {
        alpha = colUbyteToReal(colAlpha(*layer)) * opacity;
        oneMinusAlpha = 1.0f - alpha;
        redSource = colUbyteToReal(colRed(*layer));    //read pixel to floating point
        greenSource = colUbyteToReal(colGreen(*layer));
        blueSource = colUbyteToReal(colBlue(*layer));
        redDest = colUbyteToReal(colRed(*buffer));
        greenDest = colUbyteToReal(colGreen(*buffer));
        blueDest = colUbyteToReal(colBlue(*buffer));
        clumS = (redSource * LI_RedGamma + greenSource * LI_GreenGamma + blueSource * LI_BlueGamma) / LI_TotalGamma;
        clumD = (redDest * LI_RedGamma + greenDest * LI_GreenGamma + blueDest * LI_BlueGamma) / LI_TotalGamma;

        colRGBToHLS(&hueS, &lumS, &satS, redSource, greenSource, blueSource);
        colRGBToHLS(&hueD, &lumD, &satD, redDest, greenDest, blueDest);
        colHLSToRGB(&redTemp, &greenTemp, &blueTemp, hueS, clumD, satS);

        redDest = redTemp * alpha + redDest * oneMinusAlpha;
        greenDest = greenTemp * alpha + greenDest * oneMinusAlpha;
        blueDest = blueTemp * alpha + blueDest * oneMinusAlpha;
        redDest = min(redDest, 1.0f);
        greenDest = min(greenDest, 1.0f);
        blueDest = min(blueDest, 1.0f);

        *buffer = colRGB(colRealToUbyte(redDest), colRealToUbyte(greenDest),
                         colRealToUbyte(blueDest));
        buffer++;
        layer++;
        nPixels--;
    }
}

void liBlendColorDodge(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
    real32 redSource, greenSource, blueSource, alpha;
    real32 redDest, greenDest, blueDest;//, alphaDest;
    real32 oneMinusAlpha;

    while (nPixels > 0)
    {
        alpha = colUbyteToReal(colAlpha(*layer)) * opacity;
        oneMinusAlpha = 1.0f - alpha;
        redSource = colUbyteToReal(colRed(*layer));    //read pixel to floating point
        greenSource = colUbyteToReal(colGreen(*layer));
        blueSource = colUbyteToReal(colBlue(*layer));
        redDest = colUbyteToReal(colRed(*buffer));
        greenDest = colUbyteToReal(colGreen(*buffer));
        blueDest = colUbyteToReal(colBlue(*buffer));
        redDest = (redDest + redSource * redSource * redDest) * alpha + redDest * oneMinusAlpha;
        greenDest = (greenDest + greenSource * greenSource * greenDest) * alpha + greenDest * oneMinusAlpha;
        blueDest = (blueDest + blueSource * blueSource * blueDest) * alpha + blueDest * oneMinusAlpha;
        redDest = min(redDest, 1.0f);
        greenDest = min(greenDest, 1.0f);
        blueDest = min(blueDest, 1.0f);
        *buffer = colRGB(colRealToUbyte(redDest), colRealToUbyte(greenDest),
                         colRealToUbyte(blueDest));
        buffer++;
        layer++;
        nPixels--;
    }
/*
    real32 redSource, greenSource, blueSource, alpha;
    real32 redDest, greenDest, blueDest;//, alphaDest;
    real32 oneMinusAlpha;
    real32 hueS, satS, valS, hueD, satD, valD;
//    real32 cvalD, cvalS;

    while (nPixels > 0)
    {
        alpha = colUbyteToReal(colAlpha(*layer)) * opacity;
        oneMinusAlpha = 1.0f - alpha;
        redSource = colUbyteToReal(colRed(*layer));    //read pixel to floating point
        greenSource = colUbyteToReal(colGreen(*layer));
        blueSource = colUbyteToReal(colBlue(*layer));
        redDest = colUbyteToReal(colRed(*buffer));
        greenDest = colUbyteToReal(colGreen(*buffer));
        blueDest = colUbyteToReal(colBlue(*buffer));
//        redDest = (redDest + redSource * redSource * redDest) * alpha + redDest * oneMinusAlpha;
//        greenDest = (greenDest + greenSource * greenSource * greenDest) * alpha + greenDest * oneMinusAlpha;
//        blueDest = (blueDest + blueSource * blueSource * blueDest) * alpha + blueDest * oneMinusAlpha;
        colRGBToHSV(&hueS, &satS, &valS, redSource * alpha, greenSource * alpha, blueSource * alpha);
        colRGBToHSV(&hueD, &satD, &valD, redDest, greenDest, blueDest);
        valD += valS;
        valD = min(valD, 1.0f);
        colHSVToRGB(&redDest, &greenDest, &blueDest, hueD, satD, valD);

        *buffer = colRGB(colRealToUbyte(redDest), colRealToUbyte(greenDest),
                         colRealToUbyte(blueDest));
        buffer++;
        layer++;
        nPixels--;
    }
*/
}

void liBlendDarken(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nDarken: Blending mode unimplemented.");
#endif
}
void liBlendLighten(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nLighten: Blending mode unimplemented.");
#endif
}
void liBlendHue(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nHue: Blending mode unimplemented.");
#endif
}
void liBlendSaturation(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nSaturation: Blending mode unimplemented.");
#endif
}
void liBlendLuminosity(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nLuminosity: Blending mode unimplemented.");
#endif
}
void liBlendDissolve(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nDissolve: Blending mode unimplemented.");
#endif
}
void liBlendHardLight(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nHard Light: Blending mode unimplemented.");
#endif
}
void liBlendSoftLight(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nSoft Light: Blending mode unimplemented.");
#endif
}
void liBlendDifference(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nDifference: Blending mode unimplemented.");
#endif
}
void liBlendColorBurn(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nColor Burn: Blending mode unimplemented.");
#endif
}
void liBlendExclusion(color *buffer, color *layer, real32 opacity, sdword nPixels)
{
#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nExclusion: Blending mode unimplemented.");
#endif
}

typedef void (*liBlendFunc)(color *buffer, color *layer, real32 opacity, sdword nPixels);
liBlendFunc liBlender[] =
{
/*LBT_Normal     */ liBlendNormal    ,
/*LBT_Darken     */ liBlendDarken    ,
/*LBT_Lighten    */ liBlendLighten   ,
/*LBT_Hue        */ liBlendHue       ,
/*LBT_Saturation */ liBlendSaturation,
/*LBT_Color      */ liBlendColor     ,
/*LBT_Luminosity */ liBlendLuminosity,
/*LBT_Multiply   */ liBlendMultiply,
/*LBT_Screen     */ liBlendScreen,
/*LBT_Dissolve   */ liBlendDissolve  ,
/*LBT_Overlay    */ liBlendOverlay   ,
/*LBT_HardLight  */ liBlendHardLight ,
/*LBT_SoftLight  */ liBlendSoftLight ,
/*LBT_Difference */ liBlendDifference,
/*LBT_ColorDodge */ liBlendColorDodge,
/*LBT_ColorBurn  */ liBlendColorBurn ,
/*LBT_Exclusion  */ liBlendExclusion ,
    NULL
};

/*-----------------------------------------------------------------------------
    Name        : liClipLayer
    Description : Clip a layer to image size.  Broken out of the last two functions.
    Inputs      : layer - layer who's bounds to clip
                  image - image to clip to
                  source, dest, width, height, sourceStride - parameters for clipped results.
    Outputs     : clipped dimensions, pointers and source stride
    Return      : void
    Note        : source, dest, width, height, sourceStride - parameters for clipped results
----------------------------------------------------------------------------*/
void liClipLayer(lilayer *layer, layerimage *image, color **source, color **dest,
                 sdword *width, sdword *height, sdword *sourceStride)
{
    *width = *sourceStride = layer->bounds.x1 - layer->bounds.x0;//width of layer, unclipped
    *height = layer->bounds.y1 - layer->bounds.y0;           //height of section to blend
    if (layer->bounds.x0 < 0)
    {                                                       //if off left
        *source -= layer->bounds.x0;
        *dest -= layer->bounds.x0;
        *width += layer->bounds.x0;
    }
    if (layer->bounds.x1 > image->width)
    {                                                       //if off right
        *width -= layer->bounds.x1 - image->width;           //don't do as many pixels
    }
    if (layer->bounds.y0 < 0)
    {                                                       //if off top
        *source -= layer->bounds.y0 * *sourceStride;
        *dest -= layer->bounds.y0 * image->width;
        *height += layer->bounds.y0;                         //fewer scanlines because of it
    }
    if (layer->bounds.y1 > image->height)
    {                                                       //if off bottom
        *height -= layer->bounds.y1 - image->height;
    }
}

/*-----------------------------------------------------------------------------
    Name        : liLayerBlendTo
    Description : Blend a given layer to the destination buffer.
    Inputs      : destBuffer - destination buffer, dimensions given by image->width,height.
                  image - image we are blending
                  layerIndex - index of layer to blend.  Must already be decompressed.
    Outputs     : dest modified to reflect new blending.
    Return      :
----------------------------------------------------------------------------*/
void liLayerBlendTo(color *destBuffer, sdword layerIndex, layerimage *image)
{
    lilayer *layer = &image->layers[layerIndex];
    color *source, *dest;
    sdword width, height, sourceStride;

#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\nliLayerBlendTo: blending layer %d ('%s')", layerIndex, layer->name);
#endif //LI_VERBOSE_LEVEL
    dbgAssert(layer->decompressed != NULL && ((layer->flags & LFF_Channeled) == 0));
    dbgAssert(liBlender[layer->blendMode]);

    source = layer->decompressed;                           //first layer pixel
    dest = destBuffer + image->width * layer->bounds.y0 + layer->bounds.x0;    //first dest pixel
    liClipLayer(layer, image, &source, &dest, &width, &height, &sourceStride);
    for (; height > 0; height--)
    {                                                       //for each scanline in our range
        liBlender[layer->blendMode](dest, source, layer->opacity, width);
        dest += image->width;
        source += sourceStride;
    }
}

/*-----------------------------------------------------------------------------
    Name        : liInsertChannelCompose
    Description : Takes a single channel from a source layer and sticks into a
                    channel in desination buffer.
    Inputs      : destBuffer - where to stick the channel.  Must be same size as image.
                  layerIndex - index of layer to get channel from
                  image - image to get layer from
                  sChannel - index of shource channel (0 = red, 1 = green,
                    2 = blue, 3 = alpha)
                  dChannel - index of dest channel
    Outputs     : copies channel from image to destBuffer
    Return      : void
----------------------------------------------------------------------------*/
void liInsertChannelCompose(color *destBuffer, sdword layerIndex, layerimage *image, udword sChannel, udword dChannel)
{
    lilayer *layer = &image->layers[layerIndex];
    color *source, *dest;
    ubyte *pSource, *pDest;
    sdword width, height, sourceStride, count;

#if LI_VERBOSE_LEVEL >= 2
    dbgMessagef("\liInsertChannelCompose: inserting channel %d of layer %d ('%s') into channel %d", sChannel, layerIndex, layer->name, dChannel);
#endif //LI_VERBOSE_LEVEL
    dbgAssert(layer->decompressed != NULL && ((layer->flags & LFF_Channeled) == 0));
    dbgAssert(liBlender[layer->blendMode]);
    dbgAssert(sChannel < 4);
    dbgAssert(dChannel < 4);
    source = layer->decompressed;                           //first layer pixel
    dest = destBuffer + image->width * layer->bounds.y0 + layer->bounds.x0;    //first dest pixel
    liClipLayer(layer, image, &source, &dest, &width, &height, &sourceStride);

    for (; height > 0; height--)
    {                                                       //for each scanline in our range
        pSource = (ubyte *)source + sChannel;               //get byte-pointers
        pDest = (ubyte *)dest + dChannel;
        for (count = 0; count < width; count++, pDest += 4, pSource += 4)
        {                                                   //copy line of channel
            *pDest = *pSource;
        }
        dest += image->width;
        source += sourceStride;
    }
}

/*-----------------------------------------------------------------------------
    Name        : liBlendBufferPrepare
    Description : Prepare a buffer, in the appropriate image size, for blending.
    Inputs      : image - image we are preparing to blend.
                  newTeamColor0,1 - out parameters for the team color effect buffer.
    Outputs     : Allocates and clears out a buffer in the same size as the image.
    Return      : This new buffer
----------------------------------------------------------------------------*/
color *liBlendBufferPrepare(layerimage *image, ubyte **newTeamColor0, ubyte **newTeamColor1)
{
    color *newBuffer;

    newBuffer = memAlloc(image->width * image->height * sizeof(color),
                         "RGBA blend buffer", 0);           //allocate the buffer
    memClearDword(newBuffer, colRGBA(0, 0, 0, 255),
                   image->width * image->height);           //clear to opaque black
    if (newTeamColor0 != NULL)
    {
        *newTeamColor0 = memAlloc(image->width * image->height, //allocate team color buffer
                                      "TeamColor0EffectBuffer", 0);
        *newTeamColor1 = memAlloc(image->width * image->height, //allocate team color buffer
                                      "TeamColor1EffectBuffer", 0);
        memset(*newTeamColor0, 0, image->width * image->height);//clear the memory
        memset(*newTeamColor1, 0, image->width * image->height);//in both buffers
    }
    return(newBuffer);                                      //return the buffer
}

/*-----------------------------------------------------------------------------
    Name        : liLayerColorSolid
    Description : Colorize all non-colored pixels in a layer to a new color
    Inputs      : layer - layer to colorize
                  newColor - new color to assign all currently colored pixels
                  threshold - alpha? value above which pixels have their colors changed
    Outputs     : ..
    Return      : void
    Note        : layer must be decompressed first
----------------------------------------------------------------------------*/
void liLayerColorSolid(lilayer *layer, color newColor, ubyte threshold)
{
    color *dest = layer->decompressed;
    sdword count = (layer->bounds.x1 - layer->bounds.x0) *
        (layer->bounds.y1 - layer->bounds.y0);

    dbgAssert(layer->decompressed != NULL);

    newColor &= 0x00ffffff;

    while (count > 0)
    {
        if (colAlpha(*dest) >= threshold)
        {                                                   //if a team color pixel
            *dest = ((*dest) & 0xff000000) | newColor;      //make it pure team color
        }
        else
        {
            *dest &= 0x00ffffff;                            //else flag as a non-team-color pixel
        }
        dest++;
        count--;
    }
}

/*-----------------------------------------------------------------------------
    Name        : liLayerColorAverage
    Description : Returns the average color of a layer
    Inputs      : layer - layer to find average color of
    Outputs     :
    Return      : average color
----------------------------------------------------------------------------*/
color liLayerColorAverage(lilayer *layer)
{
    udword red = 0, green = 0, blue = 0, alpha = 0, count, totalPixels;
    color *pColor;

    dbgAssert(layer->decompressed != NULL);
    dbgAssert(!bitTest(layer->flags, LFF_Channeled));
    totalPixels = count = (layer->bounds.x1 - layer->bounds.x0) *
        (layer->bounds.y1 - layer->bounds.y0);
    pColor = layer->decompressed;
    while (count > 0)
    {
        red += colRed(*pColor);
        green += colGreen(*pColor);
        blue += colBlue(*pColor);
        alpha += colAlpha(*pColor);
        count--;
        pColor++;
    }
    red /= totalPixels;
    green /= totalPixels;
    blue /= totalPixels;
    alpha /= totalPixels;

    return(colRGBA(red, green, blue, alpha));
}
