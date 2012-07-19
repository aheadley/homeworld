/*=============================================================================
    Name    : d3dblt.cpp
    Purpose : Direct3D texture blitters

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_COLORINDEX
    Description : copies a GL colorindexed image to a D3D surface
    Inputs      : surf - the surface to blit onto
                  data - the GL texture to blit from
                  width, height - dimensions
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_blt_COLORINDEX(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    BYTE* psurfBase;
    GLint x, y, pitch;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    if (pitch == width)
    {
        MEMCPY(psurfBase, data, width * height);
    }
    else
    {
        for (y = 0; y < height; y++)
        {
            MEMCPY(psurfBase + y*pitch, data + y*width, width);
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_setup
    Description :
    Inputs      : ddpf - pixelformat of the target surface
                  dColorBits - [r,g,b,a] number of bits per gun
                  dColorShift - [r,g,b,a] number of bits to shift into pos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void d3d_blt_setup(DDPIXELFORMAT* ddpf, GLint dColorBits[], GLint dColorShift[])
{
    dColorBits[0] = GetNumberOfBits(ddpf->dwRBitMask);
    dColorBits[1] = GetNumberOfBits(ddpf->dwGBitMask);
    dColorBits[2] = GetNumberOfBits(ddpf->dwBBitMask);
    dColorBits[3] = GetNumberOfBits(ddpf->dwRGBAlphaBitMask);

    dColorShift[0] = GetShiftBits(ddpf->dwRBitMask);
    dColorShift[1] = GetShiftBits(ddpf->dwGBitMask);
    dColorShift[2] = GetShiftBits(ddpf->dwBBitMask);
    dColorShift[3] = GetShiftBits(ddpf->dwRGBAlphaBitMask);
}

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_RGBA_generic
    Description : copies a GL RGBA image to a D3D surface
    Inputs      : surf - the surface to blit onto
                  data - the GL texture to blit from
                  width, height - dimensions
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_blt_RGBA_generic(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    GLint i, bytesPerPixel, sPos, dPos;
    GLint dColor[4], sColor[4];
    GLint dColorBits[4];
    GLint dColorShift[4];
    GLushort* dShort;
    GLuint* dLong;

    BYTE* psurfBase;
    GLint x, y, pitch;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    DDPIXELFORMAT* ddpf;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    ddpf = &ddsd.ddpfPixelFormat;
    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    bytesPerPixel = ddpf->dwRGBBitCount >> 3;

    d3d_blt_setup(ddpf, dColorBits, dColorShift);

    for (y = 0; y < height; y++)
    {
        dPos = y * pitch;
        sPos = 4 * y * width;

        for (x = 0; x < width; x++, dPos += bytesPerPixel, sPos += 4)
        {
            sColor[0] = data[sPos + 0]; //red
            sColor[1] = data[sPos + 1]; //green
            sColor[2] = data[sPos + 2]; //blue
            sColor[3] = data[sPos + 3]; //alpha

            for (i = 0; i < 4; i++)
            {
                if (dColorBits[i] == 8)
                {
                    dColor[i] = sColor[i];
                }
                else if (dColorBits[i] < 8)
                {
                    dColor[i] = sColor[i] >> (8 - dColorBits[i]);
                }
                else
                {
                    dColor[i] = sColor[i] << (dColorBits[i] - 8);
                }
            }

            switch (bytesPerPixel)
            {
            case 2:
                dShort = (GLushort*)(psurfBase + dPos);
                *dShort = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                break;

            case 4:
                dLong = (GLuint*)(psurfBase + dPos);
                *dLong = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                       | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                break;

            default:
                ;//FIXME: do something about this error
            }
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_RGBA16_generic
    Description : copies a GL RGBA16 image to a D3D surface
    Inputs      : surf - the surface to blit onto
                  data - the GL texture to blit from
                  width, height - dimensions
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
GLboolean d3d_blt_RGBA16_generic(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    GLint i, bytesPerPixel, sPos, dPos;
    GLint dColor[4], sColor[4];
    GLint dColorBits[4];
    GLint dColorShift[4];
    GLushort* dShort;
    GLushort* sShort;
    GLuint* dLong;

    BYTE* psurfBase;
    GLint x, y, pitch;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    DDPIXELFORMAT* ddpf;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    ddpf = &ddsd.ddpfPixelFormat;
    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    bytesPerPixel = ddpf->dwRGBBitCount >> 3;

    d3d_blt_setup(ddpf, dColorBits, dColorShift);

    for (y = 0; y < height; y++)
    {
        dPos = y * pitch;
        sPos = 2 * y * width;

        sShort = (GLushort*)(data + sPos);

        for (x = 0; x < width; x++, dPos += bytesPerPixel, sShort++)
        {
            sColor[0] = ((*sShort) & 0x0F00) >> 8;  //red
            sColor[1] = ((*sShort) & 0x00F0) >> 4;  //green
            sColor[2] =  (*sShort) & 0x000F;        //blue
            sColor[3] = ((*sShort) & 0xF000) >> 12; //alpha

            for (i = 0; i < 4; i++)
            {
                if (dColorBits[i] == 4)
                {
                    dColor[i] = sColor[i];
                }
                else if (dColorBits[i] < 4)
                {
                    dColor[i] = sColor[i] >> (4 - dColorBits[i]);
                }
                else
                {
                    dColor[i] = sColor[i] << (dColorBits[i] - 4);
                }
            }

            switch (bytesPerPixel)
            {
            case 2:
                dShort = (GLushort*)(psurfBase + dPos);
                *dShort = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                break;

            case 4:
                dLong = (GLuint*)(psurfBase + dPos);
                *dLong = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                       | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                break;

            default:
                ;//FIXME: do something about this error
            }
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA_0565(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    BYTE* psurfBase;
    WORD* psurf;
    GLint x, y, pitch;
    GLubyte* dp;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    for (y = 0, dp = data; y < height; y++)
    {
        psurf = (WORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp += 4, psurf++)
        {
            *psurf = FORM_RGB565(dp[0],dp[1],dp[2]);
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA_0555(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    BYTE* psurfBase;
    WORD* psurf;
    GLint x, y, pitch;
    GLubyte* dp;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    for (y = 0, dp = data; y < height; y++)
    {
        psurf = (WORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp += 4, psurf++)
        {
            *psurf = FORM_RGB555(dp[0],dp[1],dp[2]);
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA_8888(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    BYTE* psurfBase;
    DWORD* psurf;
    GLint x, y, pitch;
    GLubyte* dp;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    for (y = 0, dp = data; y < height; y++)
    {
        psurf = (DWORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp += 4, psurf++)
        {
            *psurf = RGBA_MAKE(dp[0],dp[1],dp[2],dp[3]);
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA_4444(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    BYTE* psurfBase;
    WORD* psurf;
    GLint x, y, pitch;
    GLubyte* dp;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    for (y = 0, dp = data; y < height; y++)
    {
        psurf = (WORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp += 4, psurf++)
        {
            *psurf = ((dp[3] & 0xF0) << 8) |    //a
                     ((dp[0] & 0xF0) << 4) |    //r
                      (dp[1] & 0xF0) |          //g
                     ((dp[2] & 0xF0) >> 4);     //b
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA16_4444(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    BYTE* psurfBase;
    GLint x, y, pitch;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    if (pitch == 2*width)
    {
        MEMCPY(psurfBase, data, 2 * width * height);
    }
    else
    {
        for (y = 0; y < height; y++)
        {
            MEMCPY(psurfBase + y*pitch, data + y*2*width, 2 * width);
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA16_8888(
    LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height)
{
    BYTE* psurfBase;
    DWORD* psurf;
    GLint x, y, pitch;
    GLushort* dp;
    WORD r, g, b, a;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = surf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return GL_FALSE;
    }

    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    for (y = 0, dp = (GLushort*)data; y < height; y++)
    {
        psurf = (DWORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp++, psurf++)
        {
            a = ((*dp) & 0xF000) >> 12;
            r = ((*dp) & 0x0F00) >> 8;
            g = ((*dp) & 0x00F0) >> 4;
            b =  (*dp) & 0x000F;
            a <<= 4;
            r <<= 4;
            g <<= 4;
            b <<= 4;
            *psurf = RGBA_MAKE(r, g, b, a);
        }
    }

    surf->Unlock(NULL);

    return GL_TRUE;
}

void d3d_shot_setup(DDPIXELFORMAT* ddpf, GLint dColorBits[], GLint dColorShift[])
{
    dColorBits[0] = GetNumberOfBits(ddpf->dwRBitMask);
    dColorBits[1] = GetNumberOfBits(ddpf->dwGBitMask);
    dColorBits[2] = GetNumberOfBits(ddpf->dwBBitMask);
    dColorBits[3] = GetNumberOfBits(ddpf->dwRGBAlphaBitMask);

    dColorShift[0] = GetShiftBits(ddpf->dwRBitMask);
    dColorShift[1] = GetShiftBits(ddpf->dwGBitMask);
    dColorShift[2] = GetShiftBits(ddpf->dwBBitMask);
    dColorShift[3] = GetShiftBits(ddpf->dwRGBAlphaBitMask);
}

void d3d_shot_16bit(
    GLubyte* pDest, GLubyte* pSource, GLsizei width, GLsizei height, GLint pitch, DDPIXELFORMAT* ddpf)
{
    GLint dColorBits[4], dColorShift[4];
    GLint sColor[4];
    GLint y, x, sPos, dPos;
    GLushort* dShort;
    GLushort* pfb;
    GLubyte* pb;
    GLushort c;

    if (ddpf->dwRBitMask == 0x7C00)
    {
        //555
        pb = pDest;
        for (y = 0; y < height; y++)
        {
            pfb = (GLushort*)(pSource + pitch*((height-1)-y));
            for (x = 0; x < width; x++, pfb++, pb += 3)
            {
                c = *pfb;
                pb[0] = (c & 0x7C00) >> (10-3);
                pb[1] = (c & 0x03E0) >> (5-3);
                pb[0] = (c & 0x1F) << 3;
            }
        }
    }
    else if (ddpf->dwRBitMask == 0xF800)
    {
        //565
        pb = pDest;
        for (y = 0; y < height; y++)
        {
            pfb = (GLushort*)(pSource + pitch*((height-1)-y));
            for (x = 0; x < width; x++, pfb++, pb += 3)
            {
                c = *pfb;
                pb[0] = (c & 0xF800) >> (11-3);
                pb[1] = (c & 0x07E0) >> (5-2);
                pb[2] = (c & 0x1F) << 3;
            }
        }
    }
    else
    {
        d3d_shot_setup(ddpf, dColorBits, dColorShift);

        for (y = 0; y < height; y++)
        {
            dPos = 3 * y * width;
            sPos = ((height-1)-y) * pitch;

            for (x = 0; x < width; x++, dPos += 3, sPos += 2)
            {
                dShort = (GLushort*)(pSource + sPos);
                sColor[0] = (*dShort & ddpf->dwRBitMask) >> dColorShift[0];
                sColor[1] = (*dShort & ddpf->dwGBitMask) >> dColorShift[1];
                sColor[2] = (*dShort & ddpf->dwBBitMask) >> dColorShift[2];
                sColor[0] <<= 8 - dColorBits[0];
                sColor[1] <<= 8 - dColorBits[1];
                sColor[2] <<= 8 - dColorBits[2];
                pDest[dPos + 0] = sColor[0];
                pDest[dPos + 1] = sColor[1];
                pDest[dPos + 2] = sColor[2];
            }
        }
    }
}

void d3d_shot_24bit(
    GLubyte* pDest, GLubyte* pSource, GLsizei width, GLsizei height, GLint pitch, DDPIXELFORMAT* ddpf)
{
    GLint y, x, dPos, sPos;

    //FIXME: this isn't correct

    if (ddpf->dwBBitMask == 0xFF)
    {
        for (y = 0; y < height; y++)
        {
            dPos = 3 * y * width;
            sPos = ((height-1)-y) * pitch;

            MEMCPY(pDest + dPos, pSource + sPos, 3*width);
        }
    }
    else
    {
        for (y = 0; y < height; y++)
        {
            dPos = 3 * y * width;
            sPos = ((height-1)-y) * pitch;

            for (x = 0; x < width; x++, dPos += 3, sPos += 3)
            {
                pDest[dPos + 0] = pSource[sPos + 2];
                pDest[dPos + 1] = pSource[sPos + 1];
                pDest[dPos + 2] = pSource[sPos + 0];
            }
        }
    }
}

void d3d_shot_32bit(
    GLubyte* pDest, GLubyte* pSource, GLsizei width, GLsizei height, GLint pitch, DDPIXELFORMAT* ddpf)
{
    GLint dColorBits[4], dColorShift[4];
    GLint sColor[4];
    GLint y, x, sPos, dPos, i;
    GLuint* dLong;

    d3d_shot_setup(ddpf, dColorBits, dColorShift);

    for (y = 0; y < height; y++)
    {
        dPos = 3 * y * width;
        sPos = ((height-1)-y) * pitch;

        for (x = 0; x < width; x++, dPos += 3, sPos += 4)
        {
            dLong = (GLuint*)(pSource + sPos);
            sColor[0] = (*dLong & ddpf->dwRBitMask) >> dColorShift[0];
            sColor[1] = (*dLong & ddpf->dwGBitMask) >> dColorShift[1];
            sColor[2] = (*dLong & ddpf->dwBBitMask) >> dColorShift[2];
            for (i = 0; i < 3; i++)
            {
                if (dColorBits[i] < 8)
                {
                    sColor[i] <<= 8 - dColorBits[i];
                }
                else if (dColorBits[i] > 8)
                {
                    sColor[i] >>= dColorBits[i] - 8;
                }
                pDest[dPos + i] = sColor[i];
            }
        }
    }
}

void d3d_draw_pixels_RGBA_generic(
    GLint xOfs, GLint yOfs, GLsizei width, GLsizei height, GLubyte* data)
{
    GLint i, bytesPerPixel, sPos, dPos;
    GLint dColor[4], sColor[4];
    GLint dColorBits[4];
    GLint dColorShift[4];
    GLushort* dShort;
    GLuint* dLong;
    GLubyte* sp;

    BYTE* psurfBase;
    GLint x, y, pitch;
    GLint ymax;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    DDPIXELFORMAT* ddpf;

    GLboolean blended, reversed;

    ymax = CTX->Buffer.Height - 1;

    if (xOfs < 0 && yOfs < 0)
    {
        reversed = TRUE;
        xOfs = -xOfs;
        yOfs = -yOfs;
    }
    else
    {
        reversed = FALSE;
    }

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    hr = D3D->BackSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return;
    }

    ddpf = &ddsd.ddpfPixelFormat;
    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    if (CTX->Blend || CTX->AlphaTest)
    {
        blended = GL_TRUE;
    }
    else
    {
        blended = GL_FALSE;
    }

    bytesPerPixel = ddpf->dwRGBBitCount >> 3;

    d3d_shot_setup(ddpf, dColorBits, dColorShift);

    for (y = 0; y < height; y++)
    {
        if (reversed)
        {
            dPos = pitch * (y + yOfs);
        }
        else
        {
            dPos = pitch * (ymax - (y + yOfs));
        }
        dPos += bytesPerPixel * xOfs;
        sPos = 4 * width * y;
        sp = data + sPos;

        if (dColorBits[0] == 8 && dColorBits[1] == 8 && dColorBits[2] == 8 && bytesPerPixel == 4)
        {
            //special-case 32bit
            dLong = (GLuint*)(psurfBase + dPos);
            if (!blended)
            {
                for (x = 0; x < width; x++)
                {
                    dLong[x] = RGBA_MAKE(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2], 255);
                }
            }
            else
            {
                for (x = 0; x < width; x++)
                {
                    if (sp[4*x + 3] != 0)
                    {
                        dLong[x] = RGBA_MAKE(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2], 255);
                    }
                }
            }
            dPos += bytesPerPixel * width;
        }
        else if (dColorBits[0] == 5 && dColorBits[1] == 6 && dColorBits[2] == 5 && bytesPerPixel == 2)
        {
            //special-case 16bit (565)
            dShort = (GLushort*)(psurfBase + dPos);
            if (!blended)
            {
                for (x = 0; x < width; x++)
                {
                    dShort[x] = FORM_RGB565(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2]);
                }
            }
            else
            {
                for (x = 0; x < width; x++)
                {
                    if (sp[4*x + 3] != 0)
                    {
                        dShort[x] = FORM_RGB565(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2]);
                    }
                }
            }
            dPos += bytesPerPixel * width;
        }
        else if (dColorBits[0] == 5 && dColorBits[1] == 5 && dColorBits[2] == 5 && bytesPerPixel == 2)
        {
            //special-case 16bit (555)
            dShort = (GLushort*)(psurfBase + dPos);
            if (!blended)
            {
                for (x = 0; x < width; x++)
                {
                    dShort[x] = FORM_RGB555(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2]);
                }
            }
            else
            {
                for (x = 0; x < width; x++)
                {
                    if (sp[4*x + 3] != 0)
                    {
                        dShort[x] = FORM_RGB555(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2]);
                    }
                }
            }
            dPos += bytesPerPixel * width;
        }
        else
        {
            for (x = 0; x < width; x++, dPos += bytesPerPixel, sPos += 4)
            {
                for (i = 0; i < 4; i++)
                {
                    sColor[i] = data[sPos + i];
                }

                if (blended && sColor[3] == 0)
                {
                    continue;
                }

                for (i = 0; i < 4; i++)
                {
                    if (dColorBits[i] == 8)
                    {
                        dColor[i] = sColor[i];
                    }
                    else if (dColorBits[i] < 8)
                    {
                        dColor[i] = sColor[i] >> (8 - dColorBits[i]);
                    }
                    else
                    {
                        dColor[i] = sColor[i] << (dColorBits[i] - 8);
                    }
                }

                switch (bytesPerPixel)
                {
                case 2:
                    dShort = (GLushort*)(psurfBase + dPos);
                    *dShort = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                    break;

                case 4:
                    dLong = (GLuint*)(psurfBase + dPos);
                    *dLong = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                    break;

                default:
                    ;//FIXME: do something about this error
                }
            }
        }
    }

    D3D->BackSurface->Unlock(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : d3d_draw_pixels_RGBA_pitched
    Description : copies a region from an RGBA framebuffer to the actual (generic) framebuffer
    Inputs      : x0, y0 - location to draw from, to
                  x1, y1 - lower right of rectangle to draw
                  swidth, sheight, spitch - dimensions of framebuffer
                  data - the RGBA data of the entire framebuffer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void d3d_draw_pixels_RGBA_pitched(GLint x0, GLint y0, GLint x1, GLint y1,
                                  GLsizei swidth, GLsizei sheight, GLsizei spitch,
                                  GLubyte* data)
{
    GLint i, bytesPerPixel, sPos, dPos;
    GLint width, height;
    GLint dColor[4], sColor[4];
    GLint dColorBits[4];
    GLint dColorShift[4];
    GLushort* dShort;
    GLuint* dLong;
    GLubyte* sp;

    BYTE* psurfBase;
    GLint x, y, pitch;

    HRESULT hr;
    DDSURFACEDESC2 ddsd;
    DDPIXELFORMAT* ddpf;

    width  = x1 - x0;
    height = y1 - y0;

    //early exit conditions
    if (x0 >= swidth) return;
    if (x1 < 0) return;
    if (y0 >= sheight) return;
    if (y1 < 0) return;

    //clip left
    while (x0 < 0)
    {
        x0++;
        width--;
    }
    if (x0 >= x1)
    {
        return;
    }

    //clip right
    while ((x0+width) > swidth)
    {
        width--;
    }

    //clip top
    while (y0 < 0)
    {
        y0++;
        height--;
    }
    if (y0 >= y1)
    {
        return;
    }

    //clip bottom
    while ((y0+height) > sheight)
    {
        height--;
    }

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    hr = D3D->BackSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr))
    {
        return;
    }

    ddpf = &ddsd.ddpfPixelFormat;
    psurfBase = (BYTE*)ddsd.lpSurface;
    pitch = ddsd.lPitch;

    bytesPerPixel = ddpf->dwRGBBitCount >> 3;

    d3d_shot_setup(ddpf, dColorBits, dColorShift);

    for (y = 0; y < height; y++)
    {
        dPos = pitch * (y + y0);
        dPos += bytesPerPixel * x0;

        sPos = spitch * ((sheight - 1) - (y + y0));
        sPos += 4 * x0;
        sp = data + sPos;

#if 0
        if (dColorBits[0] == 8 && dColorBits[1] == 8 && dColorBits[2] == 8 && bytesPerPixel == 4)
        {
            //special-case 32bit
            dLong = (GLuint*)(psurfBase + dPos);
            for (x = 0; x < width; x++)
            {
                dLong[x] = RGBA_MAKE(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2], 255);
            }
        }
        else if (dColorBits[0] == 5 && dColorBits[1] == 6 && dColorBits[2] == 5 && bytesPerPixel == 2)
        {
            //special-case 16bit (565)
            dShort = (GLushort*)(psurfBase + dPos);
            for (x = 0; x < width; x++)
            {
                dShort[x] = FORM_RGB565(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2]);
            }
        }
        else if (dColorBits[0] == 5 && dColorBits[1] == 5 && dColorBits[2] == 5 && bytesPerPixel == 2)
        {
            //special-case 16bit (555)
            dShort = (GLushort*)(psurfBase + dPos);
            for (x = 0; x < width; x++)
            {
                dShort[x] = FORM_RGB555(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2]);
            }
        }
        else
#endif
        {
            for (x = 0; x < width; x++, dPos += bytesPerPixel, sPos += 4)
            {
                for (i = 0; i < 4; i++)
                {
                    sColor[i] = data[sPos + i];
                }

                for (i = 0; i < 4; i++)
                {
                    if (dColorBits[i] == 8)
                    {
                        dColor[i] = sColor[i];
                    }
                    else if (dColorBits[i] < 8)
                    {
                        dColor[i] = sColor[i] >> (8 - dColorBits[i]);
                    }
                    else
                    {
                        dColor[i] = sColor[i] << (dColorBits[i] - 8);
                    }
                }

                switch (bytesPerPixel)
                {
                case 2:
                    dShort = (GLushort*)(psurfBase + dPos);
                    *dShort = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                    break;

                case 4:
                    dLong = (GLuint*)(psurfBase + dPos);
                    *dLong = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                    break;

                default:
                    ;//FIXME: do something about this error
                }
            }
        }
    }

    D3D->BackSurface->Unlock(NULL);
}
