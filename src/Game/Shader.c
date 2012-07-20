/*=============================================================================
        Name    : shader.c
        Purpose : shader provides specialized shading models for non-rGL renderers

Created 22/06/1998 by khent
Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "glinc.h"
#include <math.h>
#include <string.h>
#include "Shader.h"
#include "Clipper.h"
#include "FastMath.h"
#include "light.h"
#include "Debug.h"
#include "render.h"
#include "Memory.h"


/*=============================================================================
    Data
=============================================================================*/

#define SLOW_TO_INT 1

#ifndef CLAMP
#define CLAMP(X, MIN, MAX) ((X) < (MIN) ? (MIN) : ((X) > (MAX) ? (MAX) : (X)))
#endif

#ifndef MIN2
#define MIN2(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

// ---
static double chop_temp;

#define FAST_TO_INT(X) ((chop_temp = (double)(X) + BIG_NUM), *(int*)(&chop_temp))
#define BIG_NUM ((float)(1 << 26)*(1 << 26)*1.5)
// ---

static real32 shDockFactor = 0.0f;
static color  shDockColor  = colWhite;
static real32 shDockScalarRed, shDockScalarGreen, shDockScalarBlue;

static real32 shSpecularDefault[3] = {11.0f, 2.0f, 4.0f};
static real32 shSpecularExponent[3] = {11.0f, 2.0f, 4.0f};

static bool shNormalize = TRUE;

static real32 shGammaAdjust;
static real32 shBrightness;

typedef real32 Mat2[2][2];

real32 shIdentityMatrix[16] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

enum
{
    M00 = 0, M01 = 4, M02 = 8, M03 = 12,
    M10 = 1, M11 = 5, M12 = 9, M13 = 13,
    M20 = 2, M21 = 6, M22 = 10,M23 = 14,
    M30 = 3, M31 = 7, M32 = 11,M33 = 15
};

static real32 shGlobalAmbient[4];
static real32 shBaseColor[4];
static real32 shStdBaseColor[4];
static sdword shBaseAlpha;

typedef struct shadeMaterial
{
    real32 ambient[4];
    real32 diffuse[4];
} shadeMaterial;

static shadeMaterial shMaterial;

typedef struct shadeLight
{
    real32 diffuse[4];
    real32 ambient[4];
    real32 matdiffuse[4];
    real32 stdmatdiffuse[4];
    real32 position[4];
    real32 worldposition[4];
    real32 inverseworldposition[4];
} shadeLight;

static shadeLight shLight[2];

static hmatrix shLightMatStack[10];
static long shLightMatStackPos = 0;

static sdword nVerts;
static hvector* normalList = NULL;
shColor* colorList = NULL;

/*=============================================================================
    Code
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : shStartup
    Description : initializes the shader module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shStartup(void)
{
    sdword i;

    shSpecularDefault[0] = 11.0f;
    shSpecularDefault[1] = 2.0f;
    shSpecularDefault[2] = 4.0f;

    for (i = 0; i < 3; i++)
    {
        shSpecularExponent[i] = shSpecularDefault[i];
    }

    shGammaAdjust = 0.15f;
    shBrightness = shGammaAdjust * 127.0f;

	shLightMatStackPos = 0;
	shLightMatStack[0] = IdentityHMatrix;
}

/*-----------------------------------------------------------------------------
    Name        : shShutdown
    Description : shuts down the shader module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shShutdown(void)
{
    nVerts = 0;
    if (normalList != NULL)
    {
        memFree(normalList);
        normalList = NULL;
    }
    if (colorList != NULL)
    {
        memFree(colorList);
        colorList = NULL;
    }
}

//increase "gamma" (brightness)
void shGammaUp()
{
    shGammaAdjust += 0.05f;
    shBrightness = shGammaAdjust * 127.0f;
}

//decrease "gamma" (brightness)
void shGammaDown()
{
    shGammaAdjust -= 0.05f;
    if (shGammaAdjust < 0.0f)
    {
        shGammaAdjust = 0.0f;
    }
    shBrightness = shGammaAdjust * 127.0f;
}

//reset "gamma" (brightness) to default
void shGammaReset()
{
    shGammaAdjust = 0.15f;
    shBrightness = shGammaAdjust * 127.0f;
}

//set "gamma" (brightness) to a particular value
void shGammaSet(sdword gamma)
{
    real32 fgamma;

    fgamma = (real32)gamma * 0.01f;
    shGammaAdjust = fgamma;
    shBrightness = shGammaAdjust * 127.0f;
}

//get "gamma" (brightness) for Game Options
sdword shGammaGet(void)
{
    real32 fgamma;

    fgamma = shGammaAdjust * 100.0f;
    return (sdword)fgamma;
}

/*-----------------------------------------------------------------------------
    Name        : shSetGlobalAmbient
    Description : interface to set shGlobalAmbient variable
    Inputs      : ambient - amount of global ambient light (4-element array)
    Outputs     : shGlobalAmbient is modified
    Return      :
----------------------------------------------------------------------------*/
void shSetGlobalAmbient(real32* ambient)
{
    shGlobalAmbient[0] = ambient[0];
    shGlobalAmbient[1] = ambient[1];
    shGlobalAmbient[2] = ambient[2];
    shGlobalAmbient[3] = ambient[3];
}

/*-----------------------------------------------------------------------------
    Name        : shSetMaterial
    Description : interface to set shMaterial
    Inputs      : ambient - ambient components
                  diffuse - diffuse components
    Outputs     : shMaterial is modified
    Return      :
----------------------------------------------------------------------------*/
void shSetMaterial(real32* ambient, real32* diffuse)
{
    if (ambient != NULL)
    {
        memcpy(shMaterial.ambient, ambient, 4*sizeof(real32));
    }
    if (diffuse != NULL)
    {
        memcpy(shMaterial.diffuse, diffuse, 4*sizeof(real32));
    }
}

/*-----------------------------------------------------------------------------
    Name        : shSetLighting
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shSetLighting(sdword index, real32* diffuse, real32* ambient)
{
    memcpy(shLight[index].diffuse, diffuse, 4*sizeof(real32));
    memcpy(shLight[index].ambient, ambient, 4*sizeof(real32));
}

/*-----------------------------------------------------------------------------
    Name        : shUpdateLighting
    Description : called after the current material has been modified
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shUpdateLighting()
{
    sdword l;

	// shStdBaseColor should be (62.xx, 57.xx, 68.xx)
	// shBaseColor - (87.xx, 79.xx, 89.xx)

    shBaseColor[0] = shGlobalAmbient[0] * shMaterial.ambient[0];
    shBaseColor[1] = shGlobalAmbient[1] * shMaterial.ambient[1];
    shBaseColor[2] = shGlobalAmbient[2] * shMaterial.ambient[2];
    shBaseColor[3] = MIN2(shMaterial.diffuse[3], 1.0f);

    shStdBaseColor[0] = shGlobalAmbient[0] * 0.196078f;
    shStdBaseColor[1] = shGlobalAmbient[1] * 0.196078f;
    shStdBaseColor[2] = shGlobalAmbient[2] * 0.196078f;
    shStdBaseColor[3] = shBaseColor[3];

    for (l = 0; l < lightNumLights; l++)
    {
        //update basecolor with light+material contribution to ambient
        shBaseColor[0] += shLight[l].ambient[0] * shMaterial.ambient[0];
        shBaseColor[1] += shLight[l].ambient[1] * shMaterial.ambient[1];
        shBaseColor[2] += shLight[l].ambient[2] * shMaterial.ambient[2];
        //shLight[l].position already contains vpInfNorm
        shLight[l].matdiffuse[0] = shLight[l].diffuse[0] * shMaterial.diffuse[0];
        shLight[l].matdiffuse[1] = shLight[l].diffuse[1] * shMaterial.diffuse[1];
        shLight[l].matdiffuse[2] = shLight[l].diffuse[2] * shMaterial.diffuse[2];

        shLight[l].stdmatdiffuse[0] = shLight[l].diffuse[0] * 0.784314f;
        shLight[l].stdmatdiffuse[1] = shLight[l].diffuse[1] * 0.784314f;
        shLight[l].stdmatdiffuse[2] = shLight[l].diffuse[2] * 0.784314f;
    }

    for (l = 0; l < 4; l++)
    {
        shBaseColor[l] *= 255.0f;
        shStdBaseColor[l] *= 255.0f;
    }
    for (l = 0; l < 3; l++)
    {
        shBaseColor[l] += shBrightness;
        shStdBaseColor[l] += shBrightness;
    }

    shBaseAlpha = (sdword)shBaseColor[3];
}

/*-----------------------------------------------------------------------------
    Name        : shSetExponent
    Description : sets the exponent to use for the specular alpha term
    Inputs      : index - [0..2]
                  exponent - if -1, reset to default; else use given as exponent
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shSetExponent(sdword index, real32 exponent)
{
    if (index < 0 || index > 2)
    {
        return;
    }
    shSpecularExponent[index] = (exponent == -1.0f) ? shSpecularDefault[index] : exponent;
}

/*-----------------------------------------------------------------------------
    Name        : shDockLight
    Description : sets the docking light scale factor
    Inputs      : t - [0..1]
    Outputs     : shDockFactor == t
    Return      :
----------------------------------------------------------------------------*/
void shDockLight(real32 t)
{
    shDockFactor = t;
}

/*-----------------------------------------------------------------------------
    Name        : shDockLightColor
    Description : sets the docking light colour
    Inputs      : c - the colour of the light
    Outputs     : shDockColor == c
    Return      :
----------------------------------------------------------------------------*/
void shDockLightColor(color c)
{
    shDockColor = c;
    shDockScalarRed = colReal32(colRed(c));
    shDockScalarGreen = colReal32(colGreen(c));
    shDockScalarBlue = colReal32(colBlue(c));
}

/*-----------------------------------------------------------------------------
    Name        : shTransformNormal
    Description : transforms and normalizes a normal by provided matrix
    Inputs      : out - output normal
                  in - input normal
                  m - the matrix with which to transform the normal by
    Outputs     : out contains the resultant normal
    Return      :
----------------------------------------------------------------------------*/
void shTransformNormal(vector* out, vector* in, real32* m)
{
    real32 ux, uy, uz;
    real64 tx, ty, tz;
    real64 len, scale;

    if (shNormalize)
    {
        ux = in->x;
        uy = in->y;
        uz = in->z;
        tx = ux*m[0] + uy*m[1] + uz*m[2];
        ty = ux*m[4] + uy*m[5] + uz*m[6];
        tz = ux*m[8] + uy*m[9] + uz*m[10];
        len = fmathSqrtDouble(tx*tx + ty*ty + tz*tz);
        scale = (len > 1E-30) ? (1.0 / len) : 1.0;
        out->x = (real32)(tx*scale);
        out->y = (real32)(ty*scale);
        out->z = (real32)(tz*scale);
    }
    else
    {
        ux = in->x;
        uy = in->y;
        uz = in->z;
        out->x = ux*m[0] + uy*m[1] + uz*m[2];
        out->y = ux*m[4] + uy*m[5] + uz*m[6];
        out->z = ux*m[8] + uy*m[9] + uz*m[10];
    }
}

/*-----------------------------------------------------------------------------
    Name        : _shTransformNormal
    Description : like shTransformNormal, but takes an additional parameter
    Inputs      : [as per shTransformNormal, with 1 addition]
                  normalize - 0 (don't normalize) or 1 (normalize)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void _shTransformNormal(vector* out, vector* in, real32* m, sdword normalize)
{
    real32 ux, uy, uz;

    if (normalize)
    {
        real64 tx, ty, tz;
        real64 len, scale;

        ux = in->x;
        uy = in->y;
        uz = in->z;
        tx = ux*m[0] + uy*m[1] + uz*m[2];
        ty = ux*m[4] + uy*m[5] + uz*m[6];
        tz = ux*m[8] + uy*m[9] + uz*m[10];
        len = fmathSqrtDouble(tx*tx + ty*ty + tz*tz);
        scale = (len > 1E-30) ? (1.0 / len) : 1.0;
        out->x = (real32)(tx*scale);
        out->y = (real32)(ty*scale);
        out->z = (real32)(tz*scale);
    }
    else
    {
        ux = in->x;
        uy = in->y;
        uz = in->z;
        out->x = ux*m[0] + uy*m[1] + uz*m[2];
        out->y = ux*m[4] + uy*m[5] + uz*m[6];
        out->z = ux*m[8] + uy*m[9] + uz*m[10];
    }
}

/*-----------------------------------------------------------------------------
    Name        : shTransformVertex
    Description : transforms a vertex by provided matrix
    Inputs      : out - output vertex
                  in - input vertex
                  m - the matrix with which to transform the vertex by
    Outputs     : out contains the resultant vertex
    Return      :
----------------------------------------------------------------------------*/
void shTransformVertex(vector* out, vector* in, real32* m)
{
    real32 ox, oy, oz;

    ox = in->x;
    oy = in->y;
    oz = in->z;

    out->x = m[0]*ox + m[4]*oy +  m[8]*oz + m[12];
    out->y = m[1]*ox + m[5]*oy +  m[9]*oz + m[13];
    out->z = m[2]*ox + m[6]*oy + m[10]*oz + m[14];
}

/*-----------------------------------------------------------------------------
    Name        : shPow
    Description : wrapper for math.h's "pow"
    Inputs      : ..
    Outputs     :
    Return      : result
----------------------------------------------------------------------------*/
#define shPow(A,B) (real32)pow((real64)(A), (real64)(B))

/*-----------------------------------------------------------------------------
    Name        : shColour
    Description : shades a vertex according to the standard lighting model
    Inputs      : side = 0 or 1 (front or back)
                  norm - normal at the vertex
                  color - where output colours go
                  minv - inverse modelview matrix
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shColour(
    sdword side, vector* norm, ubyte* color, real32* minv)
{
    sdword l;
    real32 sumR, sumG, sumB;
    real32 nx, ny, nz;

    if (side == 0)
    {
        nx = norm->x;
        ny = norm->y;
        nz = norm->z;
    }
    else
    {
        nx = -norm->x;
        ny = -norm->y;
        nz = -norm->z;
    }

    sumR = shBaseColor[0];
    sumG = shBaseColor[1];
    sumB = shBaseColor[2];

    for (l = 0; l < lightNumLights; l++)
    {
        real32 nDotVP = nx * shLight[l].inverseworldposition[0]
                      + ny * shLight[l].inverseworldposition[1]
                      + nz * shLight[l].inverseworldposition[2];
        if (nDotVP > 0.0f)
        {
            nDotVP *= 255.0f;
            sumR += nDotVP * shLight[l].matdiffuse[0];
            sumG += nDotVP * shLight[l].matdiffuse[1];
            sumB += nDotVP * shLight[l].matdiffuse[2];
        }
    }

#if SLOW_TO_INT
    color[0] = (ubyte)MIN2(sumR, 255.0f);
    color[1] = (ubyte)MIN2(sumG, 255.0f);
    color[2] = (ubyte)MIN2(sumB, 255.0f);
#else
    color[0] = FAST_TO_INT(MIN2(sumR, 255.0f));
    color[1] = FAST_TO_INT(MIN2(sumG, 255.0f));
    color[2] = FAST_TO_INT(MIN2(sumB, 255.0f));
#endif
    color[3] = shBaseAlpha;
}

/*-----------------------------------------------------------------------------
    Name        : shColourSet
    Description : shades a vertex according to the standard lighting model.
                  also informs the GL of the colour via glColor4ub
    Inputs      : side = 0 or 1 (front or back)
                  norm - normal at the vertex
                  color - where output colours go
                  minv - inverse modelview matrix
    Outputs     : current GL colour is updated
    Return      :
----------------------------------------------------------------------------*/
void shColourSet(
    sdword side, vector* norm, real32* minv)
{
    sdword l;
    real32 sumR, sumG, sumB;
    real32 nx, ny, nz;
    real32 nDotVP;

    if (side == 0)
    {
        nx = norm->x;
        ny = norm->y;
        nz = norm->z;
    }
    else
    {
        nx = -norm->x;
        ny = -norm->y;
        nz = -norm->z;
    }

    sumR = shBaseColor[0];
    sumG = shBaseColor[1];
    sumB = shBaseColor[2];

    for (l = 0; l < lightNumLights; l++)
    {
        nDotVP = nx * shLight[l].inverseworldposition[0]
               + ny * shLight[l].inverseworldposition[1]
               + nz * shLight[l].inverseworldposition[2];
        if (nDotVP > 0.0f)
        {
            nDotVP *= 255.0f;
            sumR += nDotVP * shLight[l].matdiffuse[0];
            sumG += nDotVP * shLight[l].matdiffuse[1];
            sumB += nDotVP * shLight[l].matdiffuse[2];
        }
    }

#if SLOW_TO_INT
    glColor4ub((ubyte)MIN2(sumR, 255.0f),
               (ubyte)MIN2(sumG, 255.0f),
               (ubyte)MIN2(sumB, 255.0f),
               (ubyte)shBaseAlpha);
#else
    glColor4ub((ubyte)FAST_TO_INT(MIN2(sumR, 255.0f)),
               (ubyte)FAST_TO_INT(MIN2(sumG, 255.0f)),
               (ubyte)FAST_TO_INT(MIN2(sumB, 255.0f)),
               (ubyte)shBaseAlpha);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : shGrowBuffers
    Description : possibly grow the normal & colour lists
    Inputs      : n - number of vertices
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shGrowBuffers(sdword nVertices)
{
    if (normalList == NULL)
    {
        nVerts = nVertices;
        normalList = (hvector*)memAlloc(nVerts * sizeof(hvector), "sh normal list", NonVolatile);
        colorList = (shColor*)memAlloc(nVerts * sizeof(shColor), "sh color list", NonVolatile);
    }

    if (nVerts < nVertices)
    {
        nVerts = nVertices;
        normalList = (hvector*)memRealloc(normalList, nVerts * sizeof(hvector), "sh normal list", NonVolatile);
        colorList = (shColor*)memRealloc(colorList, nVerts * sizeof(shColor), "sh color list", NonVolatile);
    }
}

/*-----------------------------------------------------------------------------
    Name        : shShadeBuffer
    Description : applies shading to a list of normals
    Inputs      : nVertices - number of vertices (uh, normals)
                  source - input normals
    Outputs     : shColorList is filled
    Return      :
----------------------------------------------------------------------------*/
void shShadeBuffer(sdword nVertices, hvector* source)
{
    sdword i, l;
    real32 sumR, sumG, sumB;
    real32 nx, ny, nz;
    real32 nDotVP;
    real32 t;

    if (shDockFactor == 0.0f)
    {
        for (i = 0; i < nVertices; i++)
        {
            nx = (source + i)->x;
            ny = (source + i)->y;
            nz = (source + i)->z;

            sumR = shStdBaseColor[0];
            sumG = shStdBaseColor[1];
            sumB = shStdBaseColor[2];

            for (l = 0; l < lightNumLights; l++)
            {
                nDotVP = nx * shLight[l].inverseworldposition[0]
                       + ny * shLight[l].inverseworldposition[1]
                       + nz * shLight[l].inverseworldposition[2];
                if (nDotVP > 0.0f)
                {
                    nDotVP *= 255.0f;
                    sumR += nDotVP * shLight[l].stdmatdiffuse[0];
                    sumG += nDotVP * shLight[l].stdmatdiffuse[1];
                    sumB += nDotVP * shLight[l].stdmatdiffuse[2];
                }
            }

            colorList[i].c[0] = (ubyte)MIN2(sumR, 255.0f);
            colorList[i].c[1] = (ubyte)MIN2(sumG, 255.0f);
            colorList[i].c[2] = (ubyte)MIN2(sumB, 255.0f);
            colorList[i].c[3] = (ubyte)shBaseAlpha;
        }
    }
    else
    {
        t = 1.1f - shDockFactor;
        if (t > 1.0f)
        {
            t = 1.0f;
        }

        for (i = 0; i < nVertices; i++)
        {
            nx = (source + i)->x;
            ny = (source + i)->y;
            nz = (source + i)->z;

            sumR = shStdBaseColor[0];
            sumG = shStdBaseColor[1];
            sumB = shStdBaseColor[2];

            for (l = 0; l < lightNumLights; l++)
            {
                nDotVP = nx * shLight[l].inverseworldposition[0]
                       + ny * shLight[l].inverseworldposition[1]
                       + nz * shLight[l].inverseworldposition[2];
                if (nDotVP > 0.0f)
                {
                    nDotVP *= 255.0f;
                    sumR += nDotVP * shLight[l].stdmatdiffuse[0];
                    sumG += nDotVP * shLight[l].stdmatdiffuse[1];
                    sumB += nDotVP * shLight[l].stdmatdiffuse[2];
                }
            }

            sumR *= t;
            sumG *= t;
            sumB *= t;

            t = shDockFactor * 255.0f;

            sumR += t * shDockScalarRed;
            sumG += t * shDockScalarGreen;
            sumB += t * shDockScalarBlue;

            colorList[i].c[0] = (ubyte)MIN2(sumR, 255.0f);
            colorList[i].c[1] = (ubyte)MIN2(sumG, 255.0f);
            colorList[i].c[2] = (ubyte)MIN2(sumB, 255.0f);
            colorList[i].c[3] = (ubyte)shBaseAlpha;
        }
    }
}

void shColourSet0(vector* norm)
{
    sdword l;
    real32 sumR, sumG, sumB;
    real32 nx, ny, nz;
    real32 nDotVP;

    nx = norm->x;
    ny = norm->y;
    nz = norm->z;

    sumR = shBaseColor[0];
    sumG = shBaseColor[1];
    sumB = shBaseColor[2];

    for (l = 0; l < lightNumLights; l++)
    {
        nDotVP = nx * shLight[l].inverseworldposition[0]
               + ny * shLight[l].inverseworldposition[1]
               + nz * shLight[l].inverseworldposition[2];
        if (nDotVP > 0.0f)
        {
            nDotVP *= 255.0f;
            sumR += nDotVP * shLight[l].matdiffuse[0];
            sumG += nDotVP * shLight[l].matdiffuse[1];
            sumB += nDotVP * shLight[l].matdiffuse[2];
        }
    }

    if (shDockFactor != 0.0f)
    {
        //leave at least 10% of original light
        real32 t = 1.1f - shDockFactor;
        if (t > 1.0f)
        {
            t = 1.0f;
        }

        sumR *= t;
        sumG *= t;
        sumB *= t;

        t = shDockFactor * 255.0f;

        //add in dock light colour
        sumR += t * shDockScalarRed;
        sumG += t * shDockScalarGreen;
        sumB += t * shDockScalarBlue;
    }

#if SLOW_TO_INT
    glColor4ub((ubyte)MIN2(sumR, 255.0f),
               (ubyte)MIN2(sumG, 255.0f),
               (ubyte)MIN2(sumB, 255.0f),
               (ubyte)shBaseAlpha);
#else
    glColor4ub((ubyte)FAST_TO_INT(MIN2(sumR, 255.0f)),
               (ubyte)FAST_TO_INT(MIN2(sumG, 255.0f)),
               (ubyte)FAST_TO_INT(MIN2(sumB, 255.0f)),
               (ubyte)shBaseAlpha);
#endif
}

real32 gl_pow(real32 a, real32 b)
{
    return (real32)pow((double)a, (double)b);
}

/*-----------------------------------------------------------------------------
    Name        : shSpecularColour
    Description : shades a vertex according to the specular model in use
    Inputs      : specInd - [0..2] the index of the specular shader
                  side = 0 or 1
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shSpecularColour(
    sdword specInd, sdword side, vector* vobj, vector* norm, ubyte* color,
    real32* m, real32* minv)
{
    vector veye = {0.0f, 0.0f, 1.0f};
    vector xnorm, xvobj;
    real32 nx, ny, nz;
    real32 alpha, nDotVP;
    real32 fade;
    extern bool bFade;
    extern real32 meshFadeAlpha;

    fade = bFade ? meshFadeAlpha : 1.0f;

    shTransformNormal(&xnorm, norm, minv);

    if (side == 0)
    {
        nx = xnorm.x;
        ny = xnorm.y;
        nz = xnorm.z;
    }
    else
    {
        nx = -xnorm.x;
        ny = -xnorm.y;
        nz = -xnorm.z;
    }

    if (specInd == 0)
    {
        nDotVP = nz;
        if (nDotVP > 0.0f)
        {
            alpha = shPow(CLAMP(nDotVP, 0.0f, 1.0f), shSpecularExponent[specInd]);
        }
        else
        {
            alpha = 0.0f;
        }

#if SLOW_TO_INT
        color[3] = (ubyte)(fade * (real32)color[3] * CLAMP(alpha, 0.0f, 1.0f));
#else
        color[3] = (ubyte)FAST_TO_INT(fade * (real32)color[3] * CLAMP(alpha, 0.0f, 1.0f));
#endif
    }
    else if (specInd == 1)
    {
        vector vpInfNorm[2];
        //real32 alpha0;
        real32 alpha1;
        sdword l, c;

        memcpy(&vpInfNorm[0], shLight[0].position, sizeof(vector));
        memcpy(&vpInfNorm[1], shLight[1].position, sizeof(vector));

#if 0
        shTransformVertex(&xvobj, vobj, m);
        veye.x = xvobj.x;
        veye.y = xvobj.z;
        veye.z = xvobj.y;
        vecNormalize(&veye);
        nDotVP = nx * veye.x + ny * veye.y + nz * veye.z;
        alpha0 = gl_pow(CLAMP(nDotVP, 0.0f, 1.0f), 5.0f);
#endif

        alpha1 = 0.0f;
        for (l = 0; l < lightNumLights; l++)
        {
            nDotVP = nx * vpInfNorm[l].x
                   + ny * vpInfNorm[l].y
                   + nz * vpInfNorm[l].z;
            if (nDotVP > 0.0f)
            {
                alpha1 += shPow(nDotVP, shSpecularExponent[1]);
            }
        }

        alpha = 2.3f * alpha1;//(0.9f * alpha0) + (0.23f * alpha1);

        c = (sdword)(fade * color[3] * alpha);
        c = CLAMP(c, 0, 255);

        color[3] = (ubyte)c;
    }
    else/* if (specInd == 2)*/
    {
        shTransformVertex(&xvobj, vobj, m);
        veye.x = xvobj.x;
        veye.y = xvobj.y;
        veye.z = xvobj.z;
        vecNormalize(&veye);

        nDotVP = nx * veye.x + ny * veye.y + nz * veye.z;
        if (nDotVP < 0.0f)
        {
            nDotVP = -nDotVP;
        }
        if (nDotVP > 0.0f)
        {
            alpha = shPow(CLAMP(nDotVP, 0.0f, 1.0f), shSpecularExponent[2]);
        }
        else
        {
            alpha = 0.0f;
        }

#if SLOW_TO_INT
        color[1] = (ubyte)((real32)color[1] * CLAMP(alpha, 0.0f, 0.92f));
        color[3] = (ubyte)(fade * (real32)color[3] * CLAMP(alpha, 0.0f, 1.0f));
#else
        color[1] = (ubyte)FAST_TO_INT((real32)color[1] * CLAMP(alpha, 0.0f, 0.92f));
        color[3] = (ubyte)FAST_TO_INT(fade * (real32)color[3] * CLAMP(alpha, 0.0f, 1.0f));
#endif
    }
}

/*-----------------------------------------------------------------------------
    Name        : shInvertMatrixGeneral
    Description : matrix inverter, general case
    Inputs      : out - output matrix
                  m - input matrix
    Outputs     : out contains inverse
    Return      :
----------------------------------------------------------------------------*/
static void shInvertMatrixGeneral(real32* out, real32 const* m)
{
    Mat2 r1, r2, r3, r4, r5, r6, r7;
    real32 const* A = m;
    real32* C = out;
    real32 one_over_det;

    /*
     * A is the 4x4 source matrix (to be inverted).
     * C is the 4x4 destination matrix
     * a11 is the 2x2 matrix in the upper left quadrant of A
     * a12 is the 2x2 matrix in the upper right quadrant of A
     * a21 is the 2x2 matrix in the lower left quadrant of A
     * a22 is the 2x2 matrix in the lower right quadrant of A
     * similarly, cXX are the 2x2 quadrants of the destination matrix
     */

    /* R1 = inverse( a11 ) */
    one_over_det = 1.0f / ((A[M00] * A[M11]) - (A[M10] * A[M01]));
    r1[0][0] = one_over_det * A[M11];
    r1[0][1] = one_over_det * -A[M01];
    r1[1][0] = one_over_det * -A[M10];
    r1[1][1] = one_over_det * A[M00];

    /* R2 = a21 x R1 */
    r2[0][0] = A[M20] * r1[0][0] + A[M21] * r1[1][0];
    r2[0][1] = A[M20] * r1[0][1] + A[M21] * r1[1][1];
    r2[1][0] = A[M30] * r1[0][0] + A[M31] * r1[1][0];
    r2[1][1] = A[M30] * r1[0][1] + A[M31] * r1[1][1];

    /* R3 = R1 x a12 */
    r3[0][0] = r1[0][0] * A[M02] + r1[0][1] * A[M12];
    r3[0][1] = r1[0][0] * A[M03] + r1[0][1] * A[M13];
    r3[1][0] = r1[1][0] * A[M02] + r1[1][1] * A[M12];
    r3[1][1] = r1[1][0] * A[M03] + r1[1][1] * A[M13];

    /* R4 = a21 x R3 */
    r4[0][0] = A[M20] * r3[0][0] + A[M21] * r3[1][0];
    r4[0][1] = A[M20] * r3[0][1] + A[M21] * r3[1][1];
    r4[1][0] = A[M30] * r3[0][0] + A[M31] * r3[1][0];
    r4[1][1] = A[M30] * r3[0][1] + A[M31] * r3[1][1];

    /* R5 = R4 - a22 */
    r5[0][0] = r4[0][0] - A[M22];
    r5[0][1] = r4[0][1] - A[M23];
    r5[1][0] = r4[1][0] - A[M32];
    r5[1][1] = r4[1][1] - A[M33];

    /* R6 = inverse( R5 ) */
    one_over_det = 1.0f / ((r5[0][0] * r5[1][1]) - (r5[1][0] * r5[0][1]));
    r6[0][0] = one_over_det * r5[1][1];
    r6[0][1] = one_over_det * -r5[0][1];
    r6[1][0] = one_over_det * -r5[1][0];
    r6[1][1] = one_over_det * r5[0][0];

    /* c12 = R3 x R6 */
    C[M02] = r3[0][0] * r6[0][0] + r3[0][1] * r6[1][0];
    C[M03] = r3[0][0] * r6[0][1] + r3[0][1] * r6[1][1];
    C[M12] = r3[1][0] * r6[0][0] + r3[1][1] * r6[1][0];
    C[M13] = r3[1][0] * r6[0][1] + r3[1][1] * r6[1][1];

    /* c21 = R6 x R2 */
    C[M20] = r6[0][0] * r2[0][0] + r6[0][1] * r2[1][0];
    C[M21] = r6[0][0] * r2[0][1] + r6[0][1] * r2[1][1];
    C[M30] = r6[1][0] * r2[0][0] + r6[1][1] * r2[1][0];
    C[M31] = r6[1][0] * r2[0][1] + r6[1][1] * r2[1][1];

    /* R7 = R3 x c21 */
    r7[0][0] = r3[0][0] * C[M20] + r3[0][1] * C[M30];
    r7[0][1] = r3[0][0] * C[M21] + r3[0][1] * C[M31];
    r7[1][0] = r3[1][0] * C[M20] + r3[1][1] * C[M30];
    r7[1][1] = r3[1][0] * C[M21] + r3[1][1] * C[M31];

    /* c11 = R1 - R7 */
    C[M00] = r1[0][0] - r7[0][0];
    C[M01] = r1[0][1] - r7[0][1];
    C[M10] = r1[1][0] - r7[1][0];
    C[M11] = r1[1][1] - r7[1][1];

    /* c22 = -R6 */
    C[M22] = -r6[0][0];
    C[M23] = -r6[0][1];
    C[M32] = -r6[1][0];
    C[M33] = -r6[1][1];
}

/*-----------------------------------------------------------------------------
    Name        : shInvertMatrix
    Description : invert a matrix, possibly by using general matrix inverter
    Inputs      : out - output matrix
                  m - input matrix
    Outputs     : out contains inverse of m
    Return      :
----------------------------------------------------------------------------*/
void shInvertMatrix(real32* out, real32 const* m)
{
#define MAT(m,r,c) (m)[((c)<<2)+(r)]

#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

    real32 det;
    real32 tmp[16];    /* allow out == in */

    if (m41 != 0.0f || m42 != 0.0f || m43 != 0.0f || m44 != 1.0f)
    {
        shInvertMatrixGeneral(out, m);
        return;
    }

    /* Inverse = adjoint / det */

    tmp[0] = m22 * m33 - m23 * m32;
    tmp[1] = m23 * m31 - m21 * m33;
    tmp[2] = m21 * m32 - m22 * m31;

    /* compute determinant using cofactors */
    det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2];

    /* singularity test */
    if (det == 0.0f)
    {
        memcpy(out, shIdentityMatrix, 16*sizeof(real32));
    }
    else
    {
        real32 d12, d13, d23, d24, d34, d41;
        real32 im11, im12, im13, im14;

        det= 1.0f / det;

        /* compute rest of inverse */
        tmp[0] *= det;
        tmp[1] *= det;
        tmp[2] *= det;
        tmp[3]  = 0.0f;

        im11 = m11 * det;
        im12 = m12 * det;
        im13 = m13 * det;
        im14 = m14 * det;
        tmp[4] = im13 * m32 - im12 * m33;
        tmp[5] = im11 * m33 - im13 * m31;
        tmp[6] = im12 * m31 - im11 * m32;
        tmp[7] = 0.0f;

        /* Pre-compute 2x2 dets for first two rows when computing */
        /* cofactors of last two rows. */
        d12 = im11*m22 - m21*im12;
        d13 = im11*m23 - m21*im13;
        d23 = im12*m23 - m22*im13;
        d24 = im12*m24 - m22*im14;
        d34 = im13*m24 - m23*im14;
        d41 = im14*m21 - m24*im11;

        tmp[8] =  d23;
        tmp[9] = -d13;
        tmp[10] = d12;
        tmp[11] = 0.0f;

        tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23);
        tmp[13] =  (m31 * d34 + m33 * d41 + m34 * d13);
        tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12);
        tmp[15] =  1.0f;

        memcpy(out, tmp, 16*sizeof(real32));
    }
#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}

#define TRANSFORM_POINT(Q, M, P) \
    Q[0] = M[0] * P[0] + M[4] * P[1] + M[8]  * P[2] + M[12] * P[3]; \
    Q[1] = M[1] * P[0] + M[5] * P[1] + M[9]  * P[2] + M[13] * P[3]; \
    Q[2] = M[2] * P[0] + M[6] * P[1] + M[10] * P[2] + M[14] * P[3]; \
    Q[3] = M[3] * P[0] + M[7] * P[1] + M[11] * P[2] + M[15] * P[3];

/*-----------------------------------------------------------------------------
    Name        : shSetLightPosition
    Description : maintains knowledge of a GL's light positions
    Inputs      : index - [0..1], the light whose position is being updated
                  position - the position
                  m - the matrix to transform by
    Outputs     : shLight[index].position will contain the transformed light position
    Return      :
----------------------------------------------------------------------------*/
void shSetLightPosition(sdword index, real32* position, real32* m)
{
real32 xposition[4];

    dbgAssert(index >= 0 && index < 2);

    TRANSFORM_POINT(xposition, m, position);
    vecNormalize((vector*)&xposition);
    shLight[index].position[0] = xposition[0];
    shLight[index].position[1] = xposition[1];
    shLight[index].position[2] = xposition[2];
    shLight[index].position[3] = xposition[3];

	shLight[index].worldposition[0] = position[0];
	shLight[index].worldposition[1] = position[1];
	shLight[index].worldposition[2] = position[2];
	shLight[index].worldposition[3] = position[3];
    vecNormalize((vector *)&shLight[index].worldposition);
}

#define matrixdot(x1,x2,x3,y1,y2,y3) \
    ( ((x1)*(y1)) + ((x2)*(y2)) + ((x3)*(y3)) )

void shMultiplyHVecByHMatRotInv(hvector *result, hvector *vector, hmatrix *matrix)
{
    result->x = matrixdot(vector->x,vector->y,vector->z,matrix->m11,matrix->m21,matrix->m31);
    result->y = matrixdot(vector->x,vector->y,vector->z,matrix->m12,matrix->m22,matrix->m32);
    result->z = matrixdot(vector->x,vector->y,vector->z,matrix->m13,matrix->m23,matrix->m33);
}

static void shUpdateLightsFromMatrix(hmatrix *pMat)
{
long	i;

	for(i=0; i<2; i++)
	{
		shMultiplyHVecByHMatRotInv(
			(hvector *)shLight[i].inverseworldposition,
			(hvector *)shLight[i].worldposition,
			pMat);
	}
}

static void shMultiplyHMatByHMatRot(hmatrix *result, hmatrix *first, hmatrix *second)
{
	result->m11 = matrixdot(first->m11,first->m12,first->m13,second->m11,second->m21,second->m31);
	result->m12 = matrixdot(first->m11,first->m12,first->m13,second->m12,second->m22,second->m32);
	result->m13 = matrixdot(first->m11,first->m12,first->m13,second->m13,second->m23,second->m33);

	result->m21 = matrixdot(first->m21,first->m22,first->m23,second->m11,second->m21,second->m31);
	result->m22 = matrixdot(first->m21,first->m22,first->m23,second->m12,second->m22,second->m32);
	result->m23 = matrixdot(first->m21,first->m22,first->m23,second->m13,second->m23,second->m33);

	result->m31 = matrixdot(first->m31,first->m32,first->m33,second->m11,second->m21,second->m31);
	result->m32 = matrixdot(first->m31,first->m32,first->m33,second->m12,second->m22,second->m32);
	result->m33 = matrixdot(first->m31,first->m32,first->m33,second->m13,second->m23,second->m33);
}


void shPushLightMatrix(hmatrix *pMat)
{
hmatrix	*pCurMat, *pNewMat;

	pCurMat = &shLightMatStack[shLightMatStackPos];
	shLightMatStackPos++;

	dbgAssert(shLightMatStackPos < 10);

	pNewMat = pCurMat+1;

	shMultiplyHMatByHMatRot(pNewMat, pCurMat, pMat);

	shUpdateLightsFromMatrix(pNewMat);
}

void shPopLightMatrix(void)
{
	shLightMatStackPos--;

	dbgAssert(shLightMatStackPos >= 0);

	shUpdateLightsFromMatrix(&shLightMatStack[shLightMatStackPos]);
}

