/*=============================================================================
    Name    : d3dtex.cpp
    Purpose : Direct3D texture-related functions

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"
#include "d3dlist.h"
#include "d3dblt.h"
#include "d3dtex.h"

#define II (*i)

#define DDPF_ALLPALETTED (DDPF_PALETTEINDEXED8 | DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXEDTO8)

static HRESULT WINAPI enum_texture_formats_cb(DDPIXELFORMAT* ddpf, VOID* userData)
{
    texList.push_back(*ddpf);
    return DDENUMRET_OK;
}

void enum_texture_formats(d3d_context* d3d)
{
    char formatString[64];

    texList.erase(texList.begin(), texList.end());

    d3d->d3dDevice->EnumTextureFormats(enum_texture_formats_cb, NULL);
}

static GLboolean SurfaceIsPaletted(GLuint flags)
{
    return (flags & DDPF_ALLPALETTED) ? GL_TRUE : GL_FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_match_texture_formats
    Description : tries to match GL texture formats to a D3D device's formats
    Inputs      : d3d - Direct3D context
    Outputs     : D3D->tex{internalFormat} point to approp DDPIXELFORMAT structure
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
GLboolean d3d_match_texture_formats(d3d_context* d3d)
{
    DDPIXELFORMAT* ddpf;
    GLenum formats[4] = {GL_COLOR_INDEX, GL_RGBA16, GL_RGBA, GL_RGB};
    GLint f;
    GLint bitMask, bits[4]; //bits = {r,g,b,a}

#if LOG_TEXTURE_FORMATS
    FILE* out = fopen("d3dtex.dat", "wt");
    if (out == NULL)
    {
        return GL_FALSE;
    }
#endif

    for (f = 0; f < 4; f++)
    {
        ddpf = NULL;

        switch (formats[f])
        {
        case GL_COLOR_INDEX:
            {
                for (listpf::iterator i = texList.begin(); i != texList.end(); ++i)
                {
                    if (II.dwFlags & DDPF_PALETTEINDEXED8)
                    {
                        ddpf = &II;
                        break;
                    }
                }
            }
            break;

        case GL_RGBA16:
            {
                for (listpf::iterator i = texList.begin(); i != texList.end(); ++i)
                {
                    bits[0] = GetNumberOfBits(II.dwRBitMask);
                    bits[1] = GetNumberOfBits(II.dwGBitMask);
                    bits[2] = GetNumberOfBits(II.dwBBitMask);
                    bits[3] = GetNumberOfBits(II.dwRGBAlphaBitMask);
                    bitMask = (bits[3] << 24) | (bits[0] << 16) | (bits[1] << 8) | bits[2];

                    if (!SurfaceIsPaletted(II.dwFlags) &&
                        (II.dwRGBBitCount != 24))
                    {
                        if (ddpf == NULL)
                        {
                            //seed with anything with an alpha channel
                            if (bits[3] > 0)
                            {
                                ddpf = &II;
                                if (bitMask == 0x04040404)
                                {
                                    //exact match
                                    break;
                                }
                            }
                        }
                        else if (II.dwRGBBitCount > 8)
                        {
                            if (bitMask == 0x04040404)
                            {
                                //exact match
                                ddpf = &II;
                                break;
                            }
                            else if (bits[3] > GetNumberOfBits(ddpf->dwRGBAlphaBitMask))
                            {
                                if (bits[3] > 1)
                                {
                                    //we prefer more than 1 bit of alpha
                                    ddpf = &II;
                                }
                                else
                                {
                                    if (bits[0] >= GetNumberOfBits(ddpf->dwRBitMask) &&
                                        bits[1] >= GetNumberOfBits(ddpf->dwGBitMask) &&
                                        bits[2] >= GetNumberOfBits(ddpf->dwBBitMask))
                                    {
                                        //more colour depth here
                                        ddpf = &II;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;

        case GL_RGBA:
            {
                for (listpf::iterator i = texList.begin(); i != texList.end(); ++i)
                {
                    bits[0] = GetNumberOfBits(II.dwRBitMask);
                    bits[1] = GetNumberOfBits(II.dwGBitMask);
                    bits[2] = GetNumberOfBits(II.dwBBitMask);
                    bits[3] = GetNumberOfBits(II.dwRGBAlphaBitMask);
                    bitMask = (bits[3] << 24) | (bits[0] << 16) | (bits[1] << 8) | bits[2];

                    if (!SurfaceIsPaletted(II.dwFlags) &&
                        (II.dwRGBBitCount > 8) &&
                        (II.dwRGBBitCount != 24))
                    {
                        if (ddpf == NULL)
                        {
                            //seed with anything with an alpha channel
                            if (bits[3] > 0)
                            {
                                ddpf = &II;
                            }
                        }
                        else
                        {
#if CONSERVE_MEMORY
                            if (bits[3] < GetNumberOfBits(ddpf->dwRGBAlphaBitMask) &&
                                bits[3] > 1)
                            {
                                //lowest number of bits > 1
                                ddpf = &II;
                            }
                            if (GetNumberOfBits(ddpf->dwRGBAlphaBitMask) == 1 &&
                                bits[3] > 1)
                            {
                                //if we've already got 1 bit alpha, try anything with more
                                ddpf = &II;
                            }
#else
                            if (II.dwRGBBitCount > ddpf->dwRGBBitCount)
                            {
                                if (bits[3] > 1)
                                {
                                    ddpf = &II;
                                }
                            }
                            else if (II.dwRGBBitCount == ddpf->dwRGBBitCount)
                            {
                                if (GetNumberOfBits(ddpf->dwRGBAlphaBitMask) == 1 &&
                                    bits[3] > 1)
                                {
                                    //more than 1 alpha bit
                                    ddpf = &II;
                                }
                                else if (bits[0] >= GetNumberOfBits(ddpf->dwRBitMask) &&
                                         bits[1] >= GetNumberOfBits(ddpf->dwGBitMask) &&
                                         bits[2] >= GetNumberOfBits(ddpf->dwBBitMask))
                                {
                                    //more colour depth
                                    ddpf = &II;
                                }
                            }
#endif
                        }
                    }
                }
            }
            break;

        case GL_RGB:
            {
                for (listpf::iterator i = texList.begin(); i != texList.end(); ++i)
                {
                    bits[0] = GetNumberOfBits(II.dwRBitMask);
                    bits[1] = GetNumberOfBits(II.dwGBitMask);
                    bits[2] = GetNumberOfBits(II.dwBBitMask);
                    bits[3] = GetNumberOfBits(II.dwRGBAlphaBitMask);
                    bitMask = (bits[3] << 24) | (bits[0] << 16) | (bits[1] << 8) | bits[2];

                    if (!SurfaceIsPaletted(II.dwFlags) &&
                        (II.dwRGBBitCount > 8) &&
                        (II.dwRGBBitCount != 24))
                    {
                        if (ddpf == NULL)
                        {
                            //seed with anything
                            ddpf = &II;
                        }
                        else
                        {
                            if (bits[3] == 0)
                            {
                                //prefer no alpha channel
                                if (II.dwRGBBitCount == 16)
                                {
                                    if (ddpf->dwRGBBitCount == II.dwRGBBitCount)
                                    {
                                        GLint sumOld, sumNew;
                                        sumOld = sumNew = 0;
                                        sumOld = GetNumberOfBits(ddpf->dwRBitMask) +
                                                 GetNumberOfBits(ddpf->dwGBitMask) +
                                                 GetNumberOfBits(ddpf->dwBBitMask);
                                        sumNew = bits[0] + bits[1] + bits[2];
                                        if (sumNew > sumOld)
                                        {
                                            //more colour depth, same byte size
                                            ddpf = &II;
                                        }
                                    }
                                    else
                                    {
                                        //not tracking a 16bit surface yet, we like this one
                                        ddpf = &II;
                                    }
                                }
#if CONSERVE_MEMORY
                                if (bits[0] < GetNumberOfBits(ddpf->dwRBitMask) &&
                                    bits[1] < GetNumberOfBits(ddpf->dwGBitMask) &&
                                    bits[2] < GetNumberOfBits(ddpf->dwBBitMask))
                                {
                                    if (ddpf->dwRGBBitCount != 16)
                                    {
                                        //less colour depth, but leave our existing 16bit surface alone
                                        ddpf = &II;
                                    }
                                }
#else
                                if (bits[0] >= GetNumberOfBits(ddpf->dwRBitMask) &&
                                    bits[1] >= GetNumberOfBits(ddpf->dwGBitMask) &&
                                    bits[2] >= GetNumberOfBits(ddpf->dwBBitMask))
                                {
                                    if (ddpf->dwRGBBitCount != 16)
                                    {
                                        //more colour depth, but still prefer 16bit
                                        ddpf = &II;
                                    }
                                }
#endif
                            }
                            else
                            {
                                if (GetNumberOfBits(ddpf->dwRGBAlphaBitMask) != 0)
                                {
                                    //already using an alpha channel, so we don't mind so much
#if CONSERVE_MEMORY
                                    if (bits[0] < GetNumberOfBits(ddpf->dwRBitMask) &&
                                        bits[1] < GetNumberOfBits(ddpf->dwGBitMask) &&
                                        bits[2] < GetNumberOfBits(ddpf->dwBBitMask))
                                    {
                                        //less colour depth
                                        ddpf = &II;
                                    }
#else
                                    if (bits[0] >= GetNumberOfBits(ddpf->dwRBitMask) &&
                                        bits[1] >= GetNumberOfBits(ddpf->dwGBitMask) &&
                                        bits[2] >= GetNumberOfBits(ddpf->dwBBitMask))
                                    {
                                        //more colour depth
                                        ddpf = &II;
                                    }
#endif
                                }
                            }
                        }
                    }
                }
            }
        }

        if ((ddpf == NULL) && (f == 0))
        {
            d3d->texCOLORINDEX = NULL;
#if LOG_TEXTURE_FORMATS
            fprintf(out, "colorindex NULL\n");
#endif
            continue;
        }

        if (ddpf == NULL)
        {
            return GL_FALSE;
        }

        bitMask = (GetNumberOfBits(ddpf->dwRGBAlphaBitMask) << 24) |
                  (GetNumberOfBits(ddpf->dwRBitMask) << 16) |
                  (GetNumberOfBits(ddpf->dwGBitMask) << 8) |
                   GetNumberOfBits(ddpf->dwBBitMask);

        switch (formats[f])
        {
        case GL_COLOR_INDEX:
            d3d->texCOLORINDEX = ddpf;
#if LOG_TEXTURE_FORMATS
            fprintf(out, "colorindex %x\n", bitMask);
#endif
            break;
        case GL_RGBA16:
            d3d->texRGBA16 = ddpf;
#if LOG_TEXTURE_FORMATS
            fprintf(out, "rgba16 %x\n", bitMask);
#endif
            break;
        case GL_RGBA:
#if LOG_TEXTURE_FORMATS
            fprintf(out, "rgba %x\n", bitMask);
#endif
            d3d->texRGBA = ddpf;
            break;
        case GL_RGB:
#if LOG_TEXTURE_FORMATS
            fprintf(out, "rgb %x\n", bitMask);
#endif
            d3d->texRGB = ddpf;
        }
    }

#if LOG_TEXTURE_FORMATS
    fclose(out);
#endif

    return GL_TRUE;
}

void d3d_rescale_scanline(GLsizei bytes, GLubyte* dData, GLsizei dWidth, GLubyte* sData, GLsizei sWidth)
{
    GLint dup, skip;
    GLint sx, dx;
    GLint i, j;

    if (dWidth > sWidth)
    {
        //dup x
        dup = dWidth / sWidth;
        for (sx = dx = 0; sx < bytes*sWidth; sx += bytes)
        {
            for (j = 0; j < dup; j++, dx += bytes)
            {
                for (i = 0; i < bytes; i++)
                {
                    dData[dx+i] = sData[sx+i];
                }
            }
        }
    }
    else if (sWidth > dWidth)
    {
        //skip x
        skip = sWidth / dWidth;
        for (sx = dx = 0; sx < bytes*sWidth; sx += bytes*skip, dx += bytes)
        {
            for (i = 0; i < bytes; i++)
            {
                dData[dx+i] = sData[sx+i];
            }
        }
    }
    else
    {
        //same x
        MEMCPY(dData, sData, bytes*dWidth);
    }
}

void d3d_rescale_generic(
    GLsizei bytes,
    GLubyte* dData, GLsizei dWidth, GLsizei dHeight,
    GLubyte* sData, GLsizei sWidth, GLsizei sHeight)
{
    GLubyte* pd;
    GLubyte* ps;
    GLint dy, sy;
    GLint dup, skip;
    GLint i;

    if (dHeight > sHeight)
    {
        //dup y
        dup = dHeight / sHeight;
        for (sy = dy = 0; sy < sHeight; sy++)
        {
            ps = sData + bytes*sy*sWidth;
            for (i = 0; i < dup; i++, dy++)
            {
                pd = dData + bytes*dy*dWidth;
                d3d_rescale_scanline(bytes, pd, dWidth, ps, sWidth);
            }
        }
    }
    else if (sHeight > dHeight)
    {
        //skip y
        skip = sHeight / dHeight;
        for (sy = dy = 0; sy < sHeight; sy += skip, dy++)
        {
            pd = dData + bytes*dy*dWidth;
            ps = sData + bytes*sy*sWidth;
            d3d_rescale_scanline(bytes, pd, dWidth, ps, sWidth);
        }
    }
    else
    {
        //same y
        for (sy = 0; sy < sHeight; sy++)
        {
            pd = dData + bytes*sy*dWidth;
            ps = sData + bytes*sy*sWidth;
            d3d_rescale_scanline(bytes, pd, dWidth, ps, sWidth);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_texture
    Description :
    Inputs      : tex - GL texture object
                  ddpf - pixelformat of texture surface (not needed with ONLY_GENERIC_BLITTERS)
    Outputs     : currently bound texture object's surface should contain a valid image
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
GLboolean d3d_blt_texture(gl_texture_object* tex, DDPIXELFORMAT* ddpf)
{
    d3d_texobj* t3d = (d3d_texobj*)tex->DriverData;
    GLubyte* data;
    GLubyte* tempData;
    GLsizei width, height, bytes;
    GLboolean result;

#if !ONLY_GENERIC_BLITTERS
    GLint bits = (GetNumberOfBits(ddpf->dwRGBAlphaBitMask) << 24) |
                 (GetNumberOfBits(ddpf->dwRBitMask) << 16) |
                 (GetNumberOfBits(ddpf->dwGBitMask) << 8) |
                  GetNumberOfBits(ddpf->dwBBitMask);
#endif

    tempData = NULL;

    if (t3d->width != 0 && t3d->height != 0)
    {
        //aspect-corrected texture
        width  = t3d->width;
        height = t3d->height;
        switch (tex->Format)
        {
        case GL_COLOR_INDEX:
            bytes = 1;
            break;
        case GL_RGBA16:
            bytes = 2;
            break;
        case GL_RGB:
        case GL_RGBA:
            bytes = 4;
            break;
        default:
            return FALSE;
        }
        tempData = new GLubyte[bytes * width * height];
        d3d_rescale_generic(bytes, tempData, width, height, tex->Data, tex->Width, tex->Height);
        data = tempData;
    }
    else
    {
        //normal texture
        width  = tex->Width;
        height = tex->Height;
        data = tex->Data;
    }

    result = FALSE;

    switch (tex->Format)
    {
    case GL_COLOR_INDEX:
        result = d3d_blt_COLORINDEX(t3d->texSurface, data, width, height);
        break;

    case GL_RGB:
    case GL_RGBA:
#if ONLY_GENERIC_BLITTERS
        result = d3d_blt_RGBA_generic(t3d->texSurface, data, width, height);
#else
        switch (bits)
        {
        case 0x00050605:
            result = d3d_blt_RGBA_0565(t3d->texSurface, data, width, height);
            break;
        case 0x00050505:
            result = d3d_blt_RGBA_0555(t3d->texSurface, data, width, height);
            break;
        case 0x04040404:
            result = d3d_blt_RGBA_4444(t3d->texSurface, data, width, height);
            break;
        case 0x08080808:
            result = d3d_blt_RGBA_8888(t3d->texSurface, data, width, height);
            break;
        default:
            result = d3d_blt_RGBA_generic(t3d->texSurface, data, width, height);
        }
#endif
        break;

    case GL_RGBA16:
#if ONLY_GENERIC_BLITTERS
        result = d3d_blt_RGBA16_generic(t3d->texSurface, data, width, height);
#else
        switch (bits)
        {
        case 0x04040404:
            result = d3d_blt_RGBA16_4444(t3d->texSurface, data, width, height);
            break;
        case 0x08080808:
            result = d3d_blt_RGBA16_8888(t3d->texSurface, data, width, height);
            break;
        default:
            result = d3d_blt_RGBA16_generic(t3d->texSurface, data, width, height);
        }
#endif
    }

    if (tempData != NULL)
    {
        delete [] tempData;
    }

    return result;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_create_texture
    Description : creates a D3D rep of a GL texture
    Inputs      : tex - GL texture object
    Outputs     : the driver-specific portion of the texobj will be filled in
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
GLboolean d3d_create_texture(gl_texture_object* tex)
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    DDPIXELFORMAT* ddpf;
    d3d_texobj* t3d;
    GLsizei width, height;
    GLint maxAspect;

    t3d = (d3d_texobj*)tex->DriverData;
    t3d->paletted = GL_FALSE;
    t3d->width = 0;     //default to no driver-specific scaling
    t3d->height = 0;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |
                   DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
#if 0
    if (tex->Format == GL_COLOR_INDEX)
    {
        ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
        ddsd.ddsCaps.dwCaps2 = 0;
    }
    else
#endif
    {
        ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    }
    ddsd.dwTextureStage = 0;
    ddsd.dwWidth  = tex->Width;
    ddsd.dwHeight = tex->Height;

    ddpf = NULL;
    switch (tex->Format)
    {
    case GL_COLOR_INDEX:
        t3d->paletted = GL_TRUE;
        ddpf = D3D->texCOLORINDEX;
        break;

    case GL_RGBA16:
        ddpf = D3D->texRGBA16;
        break;

    case GL_RGBA:
        ddpf = D3D->texRGBA;
        break;

    case GL_RGB:
        ddpf = D3D->texRGB;
        break;

    default:
        return GL_FALSE;
    }

    if (ddpf == NULL)
    {
        return GL_FALSE;
    }
    ddsd.ddpfPixelFormat = *ddpf;

    //handle aspect-ratio adjustments
    if (D3D->squareOnly)
    {
        maxAspect = 1;
    }
    else
    {
        maxAspect = D3D->maxTexAspect;
    }
    if (maxAspect == 0)
    {
        //no aspect restriction
        width  = tex->Width;
        height = tex->Height;

        //driver min-max cap
        if (D3D->maxTexWidth != 0)
            if (width > D3D->maxTexWidth) width = D3D->maxTexWidth;
        if (D3D->minTexWidth != 0)
            if (width < D3D->minTexWidth) width = D3D->minTexWidth;
        if (D3D->maxTexHeight != 0)
            if (height > D3D->maxTexHeight) height = D3D->maxTexHeight;
        if (D3D->minTexHeight != 0)
            if (height < D3D->minTexHeight) height = D3D->minTexHeight;

        if (width != tex->Width || height != tex->Height)
        {
            t3d->width  = width;
            t3d->height = height;

            ddsd.dwWidth  = width;
            ddsd.dwHeight = height;
        }
    }
    else
    {
        if (maxAspect == 1)
        {
            //square-only restriction
            width  = tex->Width;
            height = tex->Height;

            //enforce a max aspect to minimize wasted space
            if (height > width)
            {
#if CONSIDER_SQUARE_ASPECT
                while ((height / width) > MAX_SQUARE_ASPECT)
                {
                    height >>= 1;
                }
#endif
                width = height;
            }
            else if (width > height)
            {
#if CONSIDER_SQUARE_ASPECT
                while ((width / height) > MAX_SQUARE_ASPECT)
                {
                    width >>= 1;
                }
#endif
                height = width;
            }

            //driver min/max cap
            if (D3D->maxTexWidth != 0)
            {
                if (width > D3D->maxTexWidth)
                {
                    width  = D3D->maxTexWidth;
                    height = width;
                }
            }
            if (D3D->minTexWidth != 0)
            {
                if (width < D3D->minTexWidth)
                {
                    width  = D3D->minTexWidth;
                    height = width;
                }
            }

            if (width != tex->Width || height != tex->Height)
            {
                t3d->width  = width;
                t3d->height = height;

                ddsd.dwWidth  = width;
                ddsd.dwHeight = height;
            }
        }
        else
        {
            //arbitrary aspect restriction
            width  = tex->Width;
            height = tex->Height;

            if (width > height)
            {
                while ((width / height) > maxAspect)
                {
                    width  >>= 1;
                }
            }
            else if (height > width)
            {
                while ((height / width) > maxAspect)
                {
                    height >>= 1;
                }
            }

            //driver min/max cap
            if (D3D->maxTexWidth != 0)
                if (width > D3D->maxTexWidth) width = D3D->maxTexWidth;
            if (D3D->minTexWidth != 0)
                if (width < D3D->minTexWidth) width = D3D->minTexWidth;
            if (D3D->maxTexHeight != 0)
                if (height > D3D->maxTexHeight) height = D3D->maxTexHeight;
            if (D3D->minTexHeight != 0)
                if (height < D3D->minTexHeight) height = D3D->minTexHeight;

            if (width != tex->Width || height != tex->Height)
            {
                t3d->width  = width;
                t3d->height = height;

                ddsd.dwWidth  = width;
                ddsd.dwHeight = height;
            }
        }
    }

    hr = D3D->ddraw4->CreateSurface(&ddsd, &t3d->texSurface, NULL);
    if (FAILED(hr))
    {
        char estring[1024], texstring[16];
        switch (tex->Format)
        {
        case GL_COLOR_INDEX:
            sprintf(texstring, "COLOR_INDEX");
            break;
        case GL_RGBA16:
            sprintf(texstring, "RGBA16");
            break;
        case GL_RGBA:
            sprintf(texstring, "RGBA");
            break;
        case GL_RGB:
            sprintf(texstring, "RGB");
            break;
        default:
            sprintf(texstring, "???");
        }
        sprintf(estring,
                "d3d_create_texture(%s:CreateSurface[%d.%d was %d.%d] ... w %d.%d, h %d.%d, a %d)",
                texstring,
                t3d->width, t3d->height,
                tex->Width, tex->Height,
                D3D->minTexWidth, D3D->maxTexWidth,
                D3D->minTexHeight, D3D->maxTexHeight,
                D3D->squareOnly ? 1 : D3D->maxTexAspect);
        errLog(estring, hr);
        return GL_FALSE;
    }

#if EARLY_SHARED_ATTACHMENT
    if (CTX->UsingSharedPalette && tex->Format == GL_COLOR_INDEX)
    {
        d3d_attach_shared_palette(tex);
    }
#endif

    hr = t3d->texSurface->QueryInterface(IID_IDirect3DTexture2, (VOID**)&t3d->texObj);
    if (FAILED(hr))
    {
        t3d->texSurface->Release();
        t3d->texSurface = NULL;
        errLog("d3d_create_texture(QueryInterface)", hr);
        return GL_FALSE;
    }

    return d3d_blt_texture(tex, ddpf);
}

/*-----------------------------------------------------------------------------
    Name        : d3d_free_texture
    Description : releases the interface and surface
    Inputs      : t3d - D3D texobj
    Outputs     : t3d->valid == GL_FALSE
    Return      :
----------------------------------------------------------------------------*/
void d3d_free_texture(d3d_texobj* t3d)
{
    if (t3d->texObj != NULL)
    {
        t3d->texObj->Release();
        t3d->texObj = NULL;
    }

    if (t3d->texSurface != NULL)
    {
        t3d->texSurface->Release();
        t3d->texSurface = NULL;
    }

    t3d->valid = GL_FALSE;
}

//allocate a D3D texobj
d3d_texobj* d3d_alloc_texobj(void)
{
    d3d_texobj* t3d;

    t3d = new d3d_texobj;
    t3d->valid = GL_FALSE;
    t3d->texSurface = NULL;
    t3d->texObj = NULL;
    t3d->width = t3d->height = 0;

    return t3d;
}

//set defaults in a D3D texobj
void d3d_fill_texobj(gl_texture_object* tex)
{
    d3d_texobj* t3d = (d3d_texobj*)tex->DriverData;

    t3d->texWrapS = GL_REPEAT;
    t3d->texWrapT = GL_REPEAT;
    t3d->texMinFilter = GL_NEAREST;
    t3d->texMagFilter = GL_NEAREST;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_bind_texture
    Description : sets the texture used by the rendering device
    Inputs      : t3d - D3D texobj
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_bind_texture(d3d_texobj* t3d)
{
    HRESULT hr;

    if ((!t3d->valid) || (t3d->texObj == NULL))
    {
        D3D->d3dDevice->SetTexture(0, NULL);
        return GL_FALSE;
    }

    hr = D3D->d3dDevice->SetTexture(0, t3d->texObj);
    if (FAILED(hr))
    {
        errLog("d3d_bind_texture(SetTexture)", hr);
        return GL_FALSE;
    }
    else
    {
        return GL_TRUE;
    }
}

//create D3D reps of all textures in the GL
void d3d_load_all_textures(GLcontext* ctx)
{
    GLuint i;
    hashtable* table;
    hash_t* element;
    gl_texture_object* tex;

    table = rglGetTexobjs();
    if (table == NULL || table->maxkey == 0)
    {
        return;
    }
    for (i = 0; i < TABLE_SIZE; i++)
    {
        element = table->table[i];
        while (element != NULL)
        {
            tex = (gl_texture_object*)element->data;
            if (tex != NULL && tex->created)
            {
                texbind(tex);
                teximg(tex, 0, tex->Format);
                if (tex->Format == GL_COLOR_INDEX)
                {
                    texpalette(tex);
                }
            }

            //next chained element
            element = element->next;
        }
    }
}

//remove D3D reps from all textures in the GL
void d3d_free_all_textures(GLcontext* ctx)
{
    GLuint i;
    hashtable* table;
    hash_t* element;
    gl_texture_object* tex;

    table = rglGetTexobjs();
    if (table == NULL || table->maxkey == 0)
    {
        return;
    }
    for (i = 0; i < TABLE_SIZE; i++)
    {
        element = table->table[i];
        while (element != NULL)
        {
            tex = (gl_texture_object*)element->data;
            if (tex != NULL)
            {
                texdel(tex);
            }

            //next chained element
            element = element->next;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_create_shared_palette
    Description : create the D3D rep of the GL's shared palette.  done once
    Inputs      :
    Outputs     : D3D->sharedPalette contains a palette, already initialized but
                  perhaps simply to 0 if no shared palette is currently bound in
                  the GL
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_create_shared_palette(void)
{
    HRESULT hr;
    DWORD pe[256];
    GLubyte* pp;
    GLint i;

    if (D3D->sharedPalette != NULL)
    {
        D3D->sharedPalette->Release();
        D3D->sharedPalette = NULL;
    }

    return GL_TRUE;
#if 0
    if (CTX->SharedPalette != NULL)
    {
        for (i = 0, pp = CTX->SharedPalette; i < 256; i++)
        {
            pe[i] = PAL_MAKE(pp[4*i+0], pp[4*i+1], pp[4*i+2], 255);
        }
    }
    else
    {
        ZeroMemory(pe, 256 * sizeof(DWORD));
    }

    hr = D3D->ddraw4->CreatePalette(
            DDPCAPS_8BIT | DDPCAPS_ALLOW256 | DDPCAPS_INITIALIZE,
            (PALETTEENTRY*)pe, &D3D->sharedPalette, NULL);
    if (FAILED(hr))
    {
        errLog("d3d_create_shared_palette(CreatePalette)", hr);
    }
    return (FAILED(hr)) ? GL_FALSE : GL_TRUE;
#endif
}

static void d3d_twiddle_bound_texture(void)
{
    D3D->d3dDevice->SetRenderState(
        D3DRENDERSTATE_SHADEMODE,
        (CTX->ShadeModel == GL_SMOOTH) ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);
    D3D->d3dDevice->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, 0);
#if 1
    if (CTX->TexBoundObject != NULL)
    {
        d3d_texobj* t3d = (d3d_texobj*)CTX->TexBoundObject->DriverData;
        if (t3d != NULL && t3d->valid)
        {
            if (t3d->texObj != NULL)
            {
                //this should only be necessary w/ RAMP device
                t3d->texObj->PaletteChanged(0, 256);
            }
            if (t3d->texSurface != NULL)
            {
                //lock/unlock to force a driver refresh
                DDSURFACEDESC2 ddsd;

                ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
                ddsd.dwSize = sizeof(DDSURFACEDESC2);
                if (!FAILED(t3d->texSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL)))
                {
                    t3d->texSurface->Unlock(NULL);
                }
            }
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : d3d_modify_shared_palette
    Description : modifies the colour entries in the D3D shared palette
    Inputs      :
    Outputs     : D3D->sharedPalette's entries are modified
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_modify_shared_palette(void)
{
    HRESULT hr;
    DWORD pe[256];
    GLubyte* pp;
    GLint i;

    if (CTX->SharedPalette == NULL || D3D->sharedPalette == NULL)
    {
        return GL_TRUE;
    }

    for (i = 0, pp = CTX->SharedPalette; i < 256; i++)
    {
        pe[i] = PAL_MAKE(pp[4*i+0], pp[4*i+1], pp[4*i+2], 255);
    }

    hr = D3D->sharedPalette->SetEntries(0, 0, 256, (PALETTEENTRY*)pe);
    if (FAILED(hr))
    {
        errLog("d3d_modify_shared_palette(SetEntries)", hr);
    }

#if TEXTURE_EVICTING
    D3D->d3dObject->EvictManagedTextures();
#endif

#if TEXTURE_TWIDDLING
    d3d_twiddle_bound_texture();
#endif

    return (FAILED(hr)) ? GL_FALSE : GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_attach_shared_palette
    Description : attaches the D3D shared palette to a D3D texobj.  done once
    Inputs      : tex - GL texture object
    Outputs     : shared palette is bound to the texobj's surface
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_attach_shared_palette(gl_texture_object* tex)
{
    HRESULT hr;
    d3d_texobj* t3d;

    t3d = (d3d_texobj*)tex->DriverData;

    if (t3d->texSurface == NULL)
    {
        return GL_FALSE;
    }
    else if (D3D->sharedPalette != NULL)
    {
#if 0
        //deref previous palette
        t3d->texSurface->SetPalette(NULL);
#endif
        //bind new palette
        hr = t3d->texSurface->SetPalette(D3D->sharedPalette);
        if (FAILED(hr))
        {
            errLog("d3d_attach_shared_palette(SetPalette)", hr);
        }
        return (FAILED(hr)) ? GL_FALSE : GL_TRUE;
    }
    else
    {
        return GL_FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_attach_palette
    Description : attaches a palette to a D3D texobj
    Inputs      : tex - GL texture object
    Outputs     : a palette is created and bound to the texobj's surface.  any
                  previously bound palette is first dereferenced
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_attach_palette(gl_texture_object* tex)
{
    d3d_texobj* t3d;
    DWORD pe[256];
    BYTE* tp;
    GLint i;

    t3d = (d3d_texobj*)tex->DriverData;

    for (i = 0, tp = tex->Palette; i < 256; i++, tp += 4)
    {
        pe[i] = PAL_MAKE(tp[0], tp[1], tp[2], tp[3]);
    }

    HRESULT hr;
    LPDIRECTDRAWPALETTE pPalette;

    hr = D3D->ddraw4->CreatePalette(
            DDPCAPS_8BIT | DDPCAPS_ALLOW256 | DDPCAPS_INITIALIZE,
            (PALETTEENTRY*)pe, &pPalette, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

//neces?    (void)t3d->texSurface->SetPalette(NULL);

    hr = t3d->texSurface->SetPalette(pPalette);
    pPalette->Release();
    return (FAILED(hr)) ? GL_FALSE : GL_TRUE;
}
