/*=============================================================================
    Name    : glcompat.c
    Purpose : GL compatibility layer - for fast GL frontends, DrawPixels -> texture, &c

    Created 7/14/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "SDL.h"
#include <stdlib.h>
#include "glinc.h"
#include "main.h"
#include "render.h"
#include "texreg.h"
#include "Memory.h"
#include "glcompat.h"
#include "gldll.h"
#include "Debug.h"
#include "mouse.h"
#include "glcaps.h"
#include "FEColour.h"
#include "ShipView.h"
#include "LaunchMgr.h"
#include "ConsMgr.h"
#include "ResearchGUI.h"
#include "ScenPick.h"
#include "Randy.h"


//magic 2D coordinate adjustment factor
#define GLC_MAGIC 0.05f

#ifdef khentschel
#define VISIBLE_TEXTURES 0
#define VISIBLE_EXTENTS  0
#define GLC_FULLSCREEN   0
#else
#define VISIBLE_TEXTURES 0
#define VISIBLE_EXTENTS  0
#define GLC_FULLSCREEN   0
#endif

#define GLC_MAX_BUFFERS 3

#define TRANSFORM_POINT(Q, M, P) \
    Q[0] = M[0] * P[0] + M[4] * P[1] + M[8]  * P[2] + M[12] * P[3]; \
    Q[1] = M[1] * P[0] + M[5] * P[1] + M[9]  * P[2] + M[13] * P[3]; \
    Q[2] = M[2] * P[0] + M[6] * P[1] + M[10] * P[2] + M[14] * P[3]; \
    Q[3] = M[3] * P[0] + M[7] * P[1] + M[11] * P[2] + M[15] * P[3];

#if VISIBLE_EXTENTS
static bool int3 = FALSE;
#endif

bool glcLinear = FALSE;                 //whether to use bilinear filtering for quilts

//dimensions of the scratch buffer
static sdword _width, _height, _pitch;
static sdword _heightMinusOne;
static sdword xDiff, yDiff;             //offsets from main window dimensions

static sdword glcForceUpdates;          //number of buffers to update completely
static bool glcXReuse;
static bool glcLaggy;                   //TRUE if the fast front end is laggy

static sdword glcMaxHandles, glcHighestHandle;
static GLuint* handle;                  //handles for GL texture objects

static sdword glcNumBuffers;
static sdword glcActiveBuffer;

//dirty extents
static rectangle glcRect[GLC_MAX_BUFFERS];
//last mouse rectangle
static rectangle glcMouse[GLC_MAX_BUFFERS];

//render entire screen n times
static sdword glcForceUpdate[GLC_MAX_BUFFERS];
static bool glcDrawingMouse;            //don't update dirty extents if TRUE

static ubyte* glTex = NULL;             //to contain a single wedge of the quilt
static ubyte* glScratch = NULL;         //our scratch framebuffer
static sdword* glcScrMult = NULL;       //scratch buffer precomputed y pitch

static bool glcActiveStatus = FALSE;    //whether glcompat is active or not
static bool glcFullscreenStatus = FALSE; //whether we're running a fullscreen in-game gui

//the GL's function pointers
static DRAWPIXELSproc _glcDrawPixels;
static RASTERPOS2Fproc _glcRasterPos2f;
static COLOR3UBproc _glcColor3ub;
static CLEARproc _glcClear;
static ENABLEproc _glcEnable;
static DISABLEproc _glcDisable;

static sdword glcRasterPos[2];          //2D raster position
static sdword glcColor[3];              //current color

static GLuint glcBackgroundHandle = 0;  //texture object for background blue wedge
static GLuint glcFullscreenHandle = 0;  //texture object for in-game background wedge (black)

//state of GL caps
static bool glcBlend, glcAlphaTest, glcScissorTest;

#if VISIBLE_TEXTURES
#define GLC_RANTABLE_SIZE 16384
static bool8  glcRanTable[GLC_RANTABLE_SIZE];
static sdword glcRanIndex;
#endif


/*-----------------------------------------------------------------------------
    Name        : glcStartup
    Description : startup the glcompat module, allocating glTex memory
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE (success or failure)
----------------------------------------------------------------------------*/
bool glcStartup(void)
{
    sdword i;

#if VISIBLE_TEXTURES
    for (i = 0; i < GLC_RANTABLE_SIZE; i++)
    {
        glcRanTable[i] = ranRandom(RAN_ParticleStream) & 0xFF;
    }
    glcRanIndex = 0;
#endif

    glcNumBuffers = glCapNumBuffers();
    glcForceUpdates = glcNumBuffers;

    if (glTex != NULL)
    {
        memFree(glTex);
    }
    glTex = (ubyte*)memAlloc(4*64*64, "glTex", NonVolatile);

    glcLoadTextures();

    for (i = 0; i < GLC_MAX_BUFFERS; i++)
    {
        glcForceUpdate[i] = glcForceUpdates;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : glcShutdown
    Description : shutdown the glcompat module, freeing glTex and scratch buffer memory
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE (success or failure)
----------------------------------------------------------------------------*/
bool glcShutdown(void)
{
    if (glcActive())
    {
        (void)glcActivate(FALSE);
    }

    glcFreeTextures();

    if (glTex != NULL)
    {
        memFree(glTex);
        glTex = NULL;
    }

    if (handle != NULL)
    {
        memFree(handle);
        handle = NULL;
    }
    glcMaxHandles = 0;

    return glcAllocateScratch(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : glcLoadTextures
    Description : generate necessary texture handles and contents
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glcLoadTextures(void)
{
    ubyte  backgroundData[3*4*4];
    sdword i;

    //free textures first
    glcFreeTextures();

    //create texture objects for quilting
    if (handle != NULL)
    {
        memFree(handle);
    }
    glcXReuse = !glNT;
    if (glcXReuse)
    {
        glcMaxHandles = 2 * (MAIN_WindowWidth / 32);
    }
    else
    {
        glcMaxHandles = ((MAIN_WindowWidth / 64) * (MAIN_WindowHeight / 64)) + 2 * (MAIN_WindowWidth / 64);
    }
    handle = (GLuint*)memAlloc(glcMaxHandles * sizeof(GLuint), "glcompat handles", NonVolatile);
    memset(handle, 0, glcMaxHandles * sizeof(GLuint));
    glGenTextures(glcMaxHandles, handle);

    //generate blue background wedge
    trClearCurrent();
    glGenTextures(1, &glcBackgroundHandle);
    glBindTexture(GL_TEXTURE_2D, glcBackgroundHandle);
    for (i = 0; i < 3*4*4; i += 3)
    {
        backgroundData[i + 0] = colRed(FEC_Background);
        backgroundData[i + 1] = colGreen(FEC_Background);
        backgroundData[i + 2] = colBlue(FEC_Background);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 4, 4,
                 0, GL_RGB, GL_UNSIGNED_BYTE,
                 backgroundData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //generate black background wedge
    glGenTextures(1, &glcFullscreenHandle);
    glBindTexture(GL_TEXTURE_2D, glcFullscreenHandle);
    memset(backgroundData, 0, 3*4*4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 4, 4,
                 0, GL_RGB, GL_UNSIGNED_BYTE,
                 backgroundData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/*-----------------------------------------------------------------------------
    Name        : glcFreeTextures
    Description : free generated texture objects
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glcFreeTextures(void)
{
    sdword i;

    //free quilt texture objects
    for (i = 0; i < glcMaxHandles; i++)
    {
        if (handle[i] != 0)
        {
            glDeleteTextures(1, &handle[i]);
            handle[i] = 0;
        }
    }

    //free background texture objects
    if (glcBackgroundHandle != 0)
    {
        glDeleteTextures(1, &glcBackgroundHandle);
        glcBackgroundHandle = 0;
    }
    if (glcFullscreenHandle != 0)
    {
        glDeleteTextures(1, &glcFullscreenHandle);
        glcFullscreenHandle = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glcAllocateScratch
    Description : allocate the GL scratch buffer (RGBA, 640x480)
    Inputs      : allocate - TRUE or FALSE (allocate or deallocate)
    Outputs     :
    Return      : TRUE or FALSE (success or failure)
----------------------------------------------------------------------------*/
bool glcAllocateScratch(bool allocate)
{
    sdword y;

    if (allocate)
    {
        _width  = MAIN_WindowWidth;
        _height = MAIN_WindowHeight;
        _pitch  = 4*_width;
        _heightMinusOne = _height - 1;

        xDiff = 0;
        yDiff = 0;

        if (glScratch != NULL)
        {
            free(glScratch);
        }
        if (glcScrMult != NULL)
        {
            memFree(glcScrMult);
        }

        glScratch = (ubyte*)malloc(_pitch * _height);

        glcScrMult = (sdword*)memAlloc(sizeof(sdword) * _height, "glcScrMult", NonVolatile);
        for (y = 0; y < _height; y++)
        {
            glcScrMult[y] = _pitch * y;
        }
    }
    else
    {
        if (glScratch != NULL)
        {
            free(glScratch);
            glScratch = NULL;
        }
        if (glcScrMult != NULL)
        {
            memFree(glcScrMult);
            glcScrMult = NULL;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : glcGetScratch
    Description : return the scratch buffer & pitch, if it exists
    Inputs      : pitch - storage for pitch, or NULL if you don't care
    Outputs     :
    Return      : glScratch
----------------------------------------------------------------------------*/
ubyte* glcGetScratch(sdword* pitch)
{
    if (pitch != NULL)
    {
        if (glScratch == NULL)
        {
            *pitch = 0;
        }
        else
        {
            *pitch = _pitch;
        }
    }
    return glScratch;
}

/*-----------------------------------------------------------------------------
    Name        : glcGetScratchMaybeAllocate
    Description : will allocate the scratch buffer if not already so, then
                  return glcGetScratch
    Inputs      : pitch - see glcGetScratch
    Outputs     :
    Return      : see glcGetScratch
----------------------------------------------------------------------------*/
ubyte* glcGetScratchMaybeAllocate(sdword* pitch)
{
    if (glScratch == NULL)
    {
        if (!glcAllocateScratch(TRUE))
        {
            if (pitch != NULL)
            {
                *pitch = 0;
            }
            return NULL;
        }
    }
    return glcGetScratch(pitch);
}

/*-----------------------------------------------------------------------------
    Name        : glcNextBuffer
    Description : return the incremented buffer index (for tracking page flips)
    Inputs      :
    Outputs     :
    Return      : next buffer index
----------------------------------------------------------------------------*/
sdword glcNextBuffer(void)
{
    return ((glcActiveBuffer + 1) % glcNumBuffers);
}

/*-----------------------------------------------------------------------------
    Name        : glcPrevBuffer
    Description : return the decremented buffer index (for tracking page flips)
    Inputs      : numPrevs - amount to decrement buffer index by
    Outputs     :
    Return      : previous buffer index
----------------------------------------------------------------------------*/
sdword glcPrevBuffer(sdword numPrevs)
{
    sdword index, buffer;

    buffer = glcActiveBuffer;
    for (index = 0; index < numPrevs; index++)
    {
        if (buffer == 0)
        {
            buffer = glcNumBuffers - 1;
        }
        else
        {
            buffer--;
        }
    }
    return buffer;
}

/*-----------------------------------------------------------------------------
    Name        : glcUpdateExtents
    Description : update dirty rectangle extents
    Inputs      : x0, x1, y0, y1 - rectangle coordinates
    Outputs     : glcRect may be modified
    Return      :
----------------------------------------------------------------------------*/
static void glcUpdateExtents(sdword x0, sdword x1, sdword y0, sdword y1)
{
    rectangle* rect;
    sdword temp;

    if (glcDrawingMouse)
    {
        return;
    }

    rect = &glcRect[glcActiveBuffer];

    if (x0 + x1 + y0 + y1 == -4)
    {
        rect->x0 = -1;
        rect->x1 = -1;
        rect->y0 = -1;
        rect->y1 = -1;
    }
    else
    {
        x0 -= 1;
        x1 += 2;
        if (y0 < y1)
        {
            y0 -= 1;
            y1 += 2;
        }
        else
        {
            y1 -= 1;
            y0 += 2;
        }

        if (x0 < 0) x0 = 0;
        if (x1 >= _width) x1 = _width - 1;
        if (y0 < 0) y0 = 0;
        if (y1 >= _height) y1 = _height - 1;

        y0 = _heightMinusOne - y0;
        y1 = _heightMinusOne - y1;
        temp = y0;
        y0 = y1;
        y1 = temp;

        if (rect->x0 == -1)
        {
            rect->x0 = x0;
        }
        else if (x0 < rect->x0)
        {
            rect->x0 = x0;
        }

        if (rect->x1 == -1)
        {
            rect->x1 = x1;
        }
        else if (x1 > rect->x1)
        {
            rect->x1 = x1;
        }

        if (rect->y0 == -1)
        {
            rect->y0 = y0;
        }
        else if (y0 < rect->y0)
        {
            rect->y0 = y0;
        }

        if (rect->y1 == -1)
        {
            rect->y1 = y1;
        }
        else if (y1 > rect->y1)
        {
            rect->y1 = y1;
        }
    }
#if VISIBLE_EXTENTS
    if ((rect->x1 - rect->x0) > 640 ||
        (rect->y1 - rect->y0) > 480)
    {
        if (int3)
        {
            __asm int 3 ;
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : glcCreateTexture
    Description : create a subtexture of source RGBA 640x480 screen
    Inputs      : buf - source RGBA image
                  xofs, yofs - offset inside the source image
                  xend, yend - extents of the source image to create texture from
    Outputs     : currently bound texture is updated
    Return      :
----------------------------------------------------------------------------*/
static void glcCreateTexture(ubyte* buf, sdword xofs, sdword yofs, sdword xend, sdword yend)
{
    sdword y, x;
    sdword swidth, sheight;
    ubyte* ptex;
    ubyte* pbuf;
    ubyte  r, g, b;

    if (xend < 0)
    {
        xend = -xend;
        swidth  = 640;
        sheight = 480;
    }
    else
    {
        swidth  = _width;
        sheight = _height;
    }

    r = 0;//colRed(FEC_Background);
    g = 0;//colGreen(FEC_Background);
    b = 0;//colBlue(FEC_Background);

    for (y = 0; y < yend; y++)
    {
        if (y+yofs < 0 || y+yofs >= sheight)
        {
            if (RGLtype == D3Dtype)
            {
                //Direct3D optimization - avoid redundant byte shuffling
                ptex = glTex + 4*xend*y;
                for (x = 0; x < xend; x++)
                {
                    ptex[4*x + 0] = r;
                    ptex[4*x + 1] = g;
                    ptex[4*x + 2] = b;
                    ptex[4*x + 3] = 0;
                }
            }
            else
            {
                ptex = glTex + 3*xend*y;
                for (x = 0; x < xend; x++, ptex += 3)
                {
                    ptex[0] = r;
                    ptex[1] = g;
                    ptex[2] = b;
                }
            }
            continue;
        }
        pbuf = buf + 4*swidth*(y+yofs) + 4*xofs;
        if (RGLtype == D3Dtype)
        {
            //Direct3D optimization - avoid redundant byte shuffling
            ptex = glTex + 4*xend*y;
            for (x = 0; x < xend; x++)
            {
                if (xofs+x < 0 || xofs+x >= swidth)
                {
                    ptex[4*x + 0] = r;
                    ptex[4*x + 1] = g;
                    ptex[4*x + 2] = b;
                    ptex[4*x + 3] = 0;
                    continue;
                }
#if VISIBLE_TEXTURES
                if (glcRanTable[(glcRanIndex++) & (GLC_RANTABLE_SIZE - 1)])
#endif
                {
                    ptex[4*x + 0] = pbuf[4*x + 0];
                    ptex[4*x + 1] = pbuf[4*x + 1];
                    ptex[4*x + 2] = pbuf[4*x + 2];
                    ptex[4*x + 3] = 255;
                }
#if VISIBLE_TEXTURES
                else
                {
                    ptex[4*x + 0] = 127;
                    ptex[4*x + 1] = 127;
                    ptex[4*x + 2] = 127;
                    ptex[4*x + 3] = 255;
                }
#endif
            }
        }
        else
        {
            ptex = glTex + 3*xend*y;
            for (x = 0; x < xend; x++, ptex += 3)
            {
                if (xofs+x < 0 || xofs+x >= swidth)
                {
                    ptex[0] = r;
                    ptex[1] = g;
                    ptex[2] = b;
                    continue;
                }
#if VISIBLE_TEXTURES
                if (glcRanTable[(glcRanIndex++) & (GLC_RANTABLE_SIZE - 1)])
#endif
                {
                    ptex[0] = pbuf[4*x + 0];
                    ptex[1] = pbuf[4*x + 1];
                    ptex[2] = pbuf[4*x + 2];
                }
#if VISIBLE_TEXTURES
                else
                {
                    ptex[0] = 127;
                    ptex[1] = 127;
                    ptex[2] = 127;
                }
#endif
            }
        }
    }

    if (RGLtype == D3Dtype)
    {
        //avoid redundant byte shuffling w/ source format of RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     xend, xend,
                     0, GL_RGBA, GL_UNSIGNED_BYTE,
                     glTex);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     xend, xend,
                     0, GL_RGB, GL_UNSIGNED_BYTE,
                     glTex);
    }

    if (glcLinear)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
}

/*-----------------------------------------------------------------------------
    Name        : glcQuadAt
    Description : displays a textured quad at given location
    Inputs      : x, y - position for display
                  xend, yend - subtexture dimensions
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void glcQuadAt(sdword x, sdword y, sdword xend, sdword yend)
{
#define VERT(S,T,X,Y) \
    glTexCoord2f((real32)(S),(real32)(T));\
    glVertex2f((real32)(X) + GLC_MAGIC,(real32)(Y) + GLC_MAGIC);

    real32 xfrac, yfrac;

    xfrac = (real32)xend / (real32)xend;
    yfrac = (real32)yend / (real32)xend;

    glBegin(GL_QUADS);
    VERT(0, yfrac, x, y);
    VERT(xfrac, yfrac, x + xend, y);
    VERT(xfrac, 0, x + xend, y + yend);
    VERT(0, 0, x, y + yend);
    glEnd();
#undef VERT
}

/*-----------------------------------------------------------------------------
    Name        : glcScaledQuadAt
    Description : displays a textured quad at given location, scaled to simulate
                  640x480 in any display mode
    Inputs      : x, y - position for display
                  xend, yend - subtexture dimensions
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void glcScaledQuadAt(sdword x, sdword y, sdword xend, sdword yend)
{
#define VERT(S,T,X,Y) \
    glTexCoord2f((real32)(S),(real32)(T));\
    glVertex2f(((real32)(X))*xscale,((real32)(Y))*yscale);

    real32 xfrac, yfrac;
    real32 xscale, yscale;

    xfrac = (real32)xend / (real32)xend;
    yfrac = (real32)yend / (real32)xend;

    xscale = (real32)MAIN_WindowWidth  / 640.0f;
    yscale = (real32)MAIN_WindowHeight / 480.0f;
 
    glBegin(GL_QUADS);
    VERT(0, yfrac, x, y);
    VERT(xfrac, yfrac, x + xend, y);
    VERT(xfrac, 0, x + xend, y + yend);
    VERT(0, 0, x, y + yend);
    glEnd();
#undef VERT
}

/*-----------------------------------------------------------------------------
    Name        : glcDisplayRGBABackgroundScaled
    Description : display a RGBA 640x480 image as a series of quilted textures,
                  scaled to current display resolution
    Inputs      : surface - source image
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glcDisplayRGBABackgroundScaled(ubyte* surface)
{
    sdword handles = 0;
    sdword y, x, yend;

    rndTextureEnvironment(RTE_Replace);
    rndTextureEnable(TRUE);
    rndLightingEnable(FALSE);
    glDisable(GL_DEPTH_TEST);
    glColor4ub(255,255,255,255);

    trClearCurrent();

    if (!glcXReuse)
    {
        handles = 0;
    }

    for (y = 0; y < 480; y += 64)
    {
        if (glcXReuse)
        {
            handles = 0;
        }
        yend = (y+64 > 480) ? 32 : 64;
        for (x = 0; x < 640; x += 64)
        {
            glBindTexture(GL_TEXTURE_2D, handle[handles]);
            handles++;
            dbgAssert(handles < glcMaxHandles);
            glcCreateTexture(surface, x, y, -64, yend);
            glcScaledQuadAt(x, 479 - y + (64 - yend) - 64, 64, yend);
        }
        SDL_Delay(0);
    }

    rndTextureEnable(FALSE);

    if (handles > glcHighestHandle)
    {
        glcHighestHandle = handles;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glcDisplayRGBABackgroundWithoutScaling
    Description : display a RGBA 640x480 image as a series of quilted textures
    Inputs      : surface - source image
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glcDisplayRGBABackgroundWithoutScaling(ubyte* surface)
{
    sdword handles = 0;
    sdword y, x, yend;
    sdword xOfs, yOfs;

    xOfs = (MAIN_WindowWidth  - 640) / 2;
    yOfs = (MAIN_WindowHeight - 480) / 2;

    rndTextureEnvironment(RTE_Replace);
    rndTextureEnable(TRUE);
    rndLightingEnable(FALSE);
    glDisable(GL_DEPTH_TEST);
    glColor4ub(255,255,255,255);

    trClearCurrent();

    if (!glcXReuse)
    {
        handles = 0;
    }

    for (y = 0; y < 480; y += 64)
    {
        if (glcXReuse)
        {
            handles = 0;
        }
        yend = (y+64 > 480) ? 32 : 64;
        for (x = 0; x < 640; x += 64)
        {
            glBindTexture(GL_TEXTURE_2D, handle[handles]);
            handles++;
            dbgAssert(handles < glcMaxHandles);
            glcCreateTexture(surface, x, y, -64, yend);
            glcQuadAt(xOfs + x, yOfs + 479 - y + (64 - yend) - 64, 64, yend);
        }
        SDL_Delay(0);
    }

    rndTextureEnable(FALSE);

    if (handles > glcHighestHandle)
    {
        glcHighestHandle = handles;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glcMatrixSetup
    Description : setup a 2D orthographic projection
    Inputs      : on - TRUE or FALSE, setup or takedown
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glcMatrixSetup(bool on)
{
    static bool cull;
    static GLint matrixMode;
    static GLfloat projection[16];

    if (on)
    {
        cull = (bool)glIsEnabled(GL_CULL_FACE);
        if (cull)
        {
            glDisable(GL_CULL_FACE);
        }

        glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
        glGetFloatv(GL_PROJECTION_MATRIX, projection);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        rgluOrtho2D(0.0, (GLdouble)MAIN_WindowWidth, (GLdouble)MAIN_WindowHeight, 0.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    else
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection);

        glMatrixMode(matrixMode);

        if (cull)
        {
            glEnable(GL_CULL_FACE);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : glcDisplayRGBABackground
    Description : display a RGBA 640x480 image as a series of quilted textures
    Inputs      : surface - source image
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glcDisplayRGBABackground(ubyte* surface)
{
    sdword handles = 0;
    sdword xOfs, yOfs;
    sdword y, x, yend;

    xOfs = xDiff;
    yOfs = yDiff;

    glcMatrixSetup(TRUE);

    rndTextureEnvironment(RTE_Replace);
    rndTextureEnable(TRUE);
    rndLightingEnable(FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glColor4ub(255,255,255,255);

    trClearCurrent();

    if (!glcXReuse)
    {
        handles = 0;
    }

    for (y = 0; y < _height; y += 64)
    {
        if (glcXReuse)
        {
            handles = 0;
        }
        yend = (y+64 > _height) ? ((y+64) - _height) : 64;
        for (x = 0; x < _width; x += 64)
        {
            glBindTexture(GL_TEXTURE_2D, handle[handles]);
            handles++;
            dbgAssert(handles < glcMaxHandles);
            glcCreateTexture(surface, x, y, 64, yend);
            glcQuadAt(xOfs + x, yOfs + _heightMinusOne - y + (64 - yend) - 64, 64, yend);
        }
    }

    rndTextureEnable(FALSE);

    glcMatrixSetup(FALSE);

    if (handles > glcHighestHandle)
    {
        glcHighestHandle = handles;
    }
}

static void glcWedgeAt(sdword x, sdword y, sdword width, sdword height)
{
#define VERT(S,T,X,Y) \
    glTexCoord2f((real32)(S),(real32)(T));\
    glVertex2f((real32)(X),(real32)(Y));

    glBegin(GL_QUADS);
    VERT(0, 0, x, y);
    VERT(1, 0, x + width, y);
    VERT(1, 1, x + width, y + height);
    VERT(0, 1, x, y + height);
    glEnd();
#undef VERT
}

/*-----------------------------------------------------------------------------
    Name        : glcDisplayBackground
    Description : "clear" the screen to "blue" or "black"
    Inputs      : wedges - whether to cover only outer bounds of screen or not
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void glcDisplayBackground(bool wedges)
{
    bool blue;

    if (glScratch[0] == colRed(FEC_Background) &&
        glScratch[1] == colGreen(FEC_Background) &&
        glScratch[2] == colBlue(FEC_Background))
    {
        blue = TRUE;
    }
    else
    {
        blue = FALSE;
    }

    glcMatrixSetup(TRUE);

    rndTextureEnvironment(RTE_Replace);
    rndTextureEnable(TRUE);
    rndLightingEnable(FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glColor4ub(255,255,255,255);

    trClearCurrent();

    if (wedges)
    {
        glBindTexture(GL_TEXTURE_2D, glcFullscreenHandle);
        glcWedgeAt(-1, -1, MAIN_WindowWidth + 1, 3);
        glcWedgeAt(MAIN_WindowWidth - 1, -1, 3, MAIN_WindowHeight + 1);
        glcWedgeAt(-1, MAIN_WindowHeight - 1, MAIN_WindowWidth + 1, 3);
        glcWedgeAt(-1, -1, 3, MAIN_WindowHeight + 1);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, blue ? glcBackgroundHandle : glcFullscreenHandle);
        //draw a single big-ass quad covering the whole screen
        glcWedgeAt(-1, -1, MAIN_WindowWidth + 1, MAIN_WindowHeight + 1);
    }

    rndTextureEnable(FALSE);

    glcMatrixSetup(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : glcDisplayRGBABackgroundPortion
    Description : ...
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE (could or couldn't display the portion)
----------------------------------------------------------------------------*/
bool glcDisplayRGBABackgroundPortion(ubyte* surface, rectangle* rect, bool mouse)
{
    sdword handles;
    sdword xOfs, yOfs;
    sdword y, x, iy;

    if (mouse)
    {
        mouse = FALSE;
    }

    handles = 0;
    if (glcXReuse)
    {
        for (x = rect->x0; x < rect->x1; x += 32)
        {
            handles++;
        }
    }
    else
    {
        for (y = rect->y0; y < rect->y1; y += 32)
        {
            for (x = rect->x0; x < rect->x1; x += 32)
            {
                handles++;
            }
        }
    }
    if (handles >= glcMaxHandles)
    {
        return FALSE;
    }

    glcMatrixSetup(TRUE);

    xOfs = xDiff;
    yOfs = yDiff;

    rndTextureEnvironment(RTE_Replace);
    rndTextureEnable(TRUE);
    rndLightingEnable(FALSE);
    glDisable(GL_DEPTH_TEST);
    if (mouse)
    {
        rndAdditiveBlends(FALSE);
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    glColor4ub(255,255,255,255);

    trClearCurrent();

    if (!glcXReuse)
    {
        handles = 0;
    }

    for (y = rect->y0; y < rect->y1; y += 32)
    {
        if (glcXReuse)
        {
            handles = 0;
        }
        iy = _heightMinusOne - y;
        for (x = rect->x0; x < rect->x1; x += 32)
        {
            glBindTexture(GL_TEXTURE_2D, handle[handles]);
            handles++;
            dbgAssert(handles < glcMaxHandles);
            glcCreateTexture(surface, x, iy-32, 32, 32);
            glcQuadAt(xOfs + x, yOfs + _heightMinusOne - iy, 32, 32);
        }
    }

    if (mouse)
    {
        glDisable(GL_BLEND);
    }
    rndTextureEnable(FALSE);

    glcMatrixSetup(FALSE);

    if (handles > glcHighestHandle)
    {
        glcHighestHandle = handles;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : glcMouseDraw
    Description : call mouseDraw after setting flags to not update dirty extents
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glcMouseDraw(void)
{
#if 0
    glcDrawingMouse = TRUE;
    mouseDraw();
    glcDrawingMouse = FALSE;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : glcPurgeHandles
    Description : ensure that the quilts are minimally affecting the GL's
                  texture cache management (create small textures)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void glcPurgeHandles(void)
{
    sdword i;
    ubyte  data[3*2*2];

    memset(data, 0, 3*2*2);

    for (i = 0; i < glcHighestHandle; i++)
    {
        if (handle[i] != 0)
        {
            glBindTexture(GL_TEXTURE_2D, handle[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         2, 2,
                         0, GL_RGB, GL_UNSIGNED_BYTE,
                         data);
        }
    }

    glcHighestHandle = 0;
}

/*-----------------------------------------------------------------------------
    Name        : glcGetDirect3DMouse
    Description : gets the mouse rectangle, adjusted for Direct3D-ness
    Inputs      : r - where to put the mouse rectangle
                  index - current framebuffer page
    Outputs     : r is filled
    Return      :
----------------------------------------------------------------------------*/
static void glcGetDirect3DMouse(rectangle* r, sdword index)
{
    sdword x0, x1;
    sdword y0, y1;
    sdword height;

    x0 = glcMouse[index].x0;
    x1 = glcMouse[index].x1;
    y0 = glcMouse[index].y0;
    y1 = glcMouse[index].y1;

    r->x0 = x0 - 2;
    r->x1 = x1 + 1;

    height = abs(y1 - y0);
    y1 += height;
    if (y0 < y1)
    {
        r->y0 = y0;
        r->y1 = y1;
    }
    else
    {
        r->y0 = y1;
        r->y1 = y0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glcPageFlipHelper
    Description : convenient fn to render subsidiary frontend elements if necessary
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void glcPageFlipHelper(void)
{
    //make the screen borders sane
    glcDisplayBackground(TRUE);

#if 0
    //render the shipview mesh
    if (_svRender)
    {
        svShipViewRender(NULL, NULL);
    }
#endif

    //render the mouse atop everything
    mouseDraw();
}

static void glcDrawPitchedPixels(sdword x0, sdword y0, sdword x1, sdword y1,
                                 sdword width, sdword height, sdword pitch,
                                 ubyte* data)
{
    if (RGLtype == D3Dtype)
    {
        rglDrawPitchedPixels(x0, y0, x1, y1, width, height, pitch, data);
    }
    else
    {
        sdword height = y1 - y0;
        y0 += height;
        glcMatrixSetup(TRUE);
        _glcRasterPos2f((GLfloat)x0, (GLfloat)y0);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, _width);
        _glcDrawPixels(x1 - x0, height,
                       GL_RGBA, GL_UNSIGNED_BYTE,
                       data + pitch*(_heightMinusOne - y0) + 4*x0);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glcMatrixSetup(FALSE);
    }
}

#if VISIBLE_EXTENTS
static void glcDisplayExtents(void)
{
    rectangle rect;
    sdword    width, height;

    rect = glcRect[glcActiveBuffer];
    width  = rect.x1 - rect.x0;
    height = rect.y1 - rect.y0;
    while (width & 31)
    {
        rect.x1++;
        width++;
    }
    if (height < 0)
    {
        height = -height;
        while (height & 31)
        {
            rect.y0++;
            height++;
        }
    }
    else
    {
        while (height & 31)
        {
            rect.y1++;
            height++;
        }
    }

    glcMatrixSetup(TRUE);
    glBegin(GL_LINE_LOOP);
    glVertex2f((real32)rect.x0, (real32)rect.y0);
    glVertex2f((real32)rect.x1, (real32)rect.y0);
    glVertex2f((real32)rect.x1, (real32)rect.y1);
    glVertex2f((real32)rect.x0, (real32)rect.y1);
    glEnd();
    glcMatrixSetup(FALSE);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : glcPageFlip
    Description : displays the scratch buffer as a texture
    Inputs      : scaled - TRUE or FALSE, scale to current res or not
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glcPageFlip(bool scaled)
{
    rectangle r;
    sdword i, index;

    if (scaled)
    {
        glcMatrixSetup(TRUE);
        glcDisplayRGBABackgroundScaled(glScratch);
        glcMatrixSetup(FALSE);
    }
    else if (RGLtype == D3Dtype)
    {
        for (i = 0; i < glcNumBuffers; i++)
        {
            index = glcPrevBuffer(i);
#if VISIBLE_EXTENTS
            if (1)
#else
            if (glcForceUpdate[index])
#endif
            {
                if (glcForceUpdate[index] > 0)
                {
                    glcForceUpdate[index]--;
                }
                glcDrawPitchedPixels(0, 0, _width, _height,
                                     _width, _height, _pitch,
                                     glScratch);
            }
            else if ((glcRect[index].x0 + glcRect[index].x1 +
                      glcRect[index].y0 + glcRect[index].y1) != -4)
            {
                r = glcRect[index];
                glcDrawPitchedPixels(r.x0, r.y0, r.x1, r.y1,
                                     _width, _height, _pitch,
                                     glScratch);
                glcGetDirect3DMouse(&r, index);
                glcDrawPitchedPixels(r.x0, r.y0, r.x1, r.y1,
                                     _width, _height, _pitch,
                                     glScratch);
            }
            else
            {
                glcGetDirect3DMouse(&r, index);
                glcDrawPitchedPixels(r.x0, r.y0, r.x1, r.y1,
                                     _width, _height, _pitch,
                                     glScratch);
            }
            glcPageFlipHelper();
        }
    }
    else
    {
        for (i = 0; i < glcNumBuffers; i++)
        {
            index = glcPrevBuffer(i);
#if VISIBLE_EXTENTS
            if (1)
#else
            if (glcForceUpdate[index])
#endif
            {
                glcForceUpdate[index]--;
                glcDisplayBackground(FALSE);
                r.x0 = (MAIN_WindowWidth  - 640) >> 1;
                r.y0 = (MAIN_WindowHeight - 480) >> 1;
                r.x1 = r.x0 + 640;
                r.y1 = r.y0 + 480;
                if (!glcDisplayRGBABackgroundPortion(glScratch, &r, FALSE))
                {
                    glcDisplayRGBABackground(glScratch);
                }
                glcDisplayRGBABackgroundPortion(glScratch, &glcMouse[index], TRUE);
                glcPageFlipHelper();
            }
            else if ((glcRect[index].x0 + glcRect[index].x1 +
                      glcRect[index].y0 + glcRect[index].y1) != -4)
            {
                if (!glcDisplayRGBABackgroundPortion(glScratch, &glcRect[index], FALSE))
                {
                    glcDisplayRGBABackground(glScratch);
                }
                glcDisplayRGBABackgroundPortion(glScratch, &glcMouse[index], TRUE);
                glcPageFlipHelper();
            }
            else
            {
                if (glcLaggy && i == 0)
                {
                    SDL_Delay(30);
                }
                glcDisplayRGBABackgroundPortion(glScratch, &glcMouse[index], TRUE);
                glcPageFlipHelper();
            }
        }
    }

#if VISIBLE_EXTENTS
    glcDisplayExtents();
#endif

    glcMouse[glcActiveBuffer].x0 = mouseCursorXPosition - xDiff;
    glcMouse[glcActiveBuffer].y1 = mouseCursorYPosition - yDiff;
    glcMouse[glcActiveBuffer].x1 = glcMouse[glcActiveBuffer].x0 + lastUnderWidth;
    glcMouse[glcActiveBuffer].y0 = glcMouse[glcActiveBuffer].y1 - lastUnderHeight;

    glcMouse[glcActiveBuffer].x0 -= 2;

    glcActiveBuffer = glcNextBuffer();

    glcUpdateExtents(-1, -1, -1, -1);
}

/*-----------------------------------------------------------------------------
    Name        : glcActive
    Description : returns glcompat module active status
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool glcActive(void)
{
    return glcActiveStatus;
}

static void glcRectangle(rectangle* rect, color c, bool blend)
{
    ubyte* db;
    sdword y, x;
    sdword y0, y1;
    sdword x0, x1;
    sdword r, g, b;

    if (rect->y0 >= rect->y1 || rect->x0 >= rect->x1)
    {
        return;
    }

    y0 = _heightMinusOne - (rect->y0 - yDiff);
    y1 = _heightMinusOne - (rect->y1 - yDiff);
    if (y0 > y1)
    {
        y = y0;
        y0 = y1;
        y1 = y;
    }

    x0 = rect->x0;
    x1 = rect->x1;
    x0 -= xDiff;
    x1 -= xDiff;
    if (x0 < 0) x0 = 0;
    if (x0 >= _width) return;
    if (x1 < 0) return;
    if (x1 >= _width) x1 = _width;
    if ((x1 - x0) <= 0)
    {
        return;
    }

    glcUpdateExtents(x0, x1, y0, y1);

    r = colRed(c);
    g = colGreen(c);
    b = colBlue(c);

    if (!blend)
    {
        //ensure 255 alpha
        c |= 0xFF000000;
    }

    for (y = y0; y < y1; y++)
    {
        if (y < 0 || y >= _height)
        {
            continue;
        }
        db = glScratch + glcScrMult[y];
        if (blend)
        {
            sdword s, t;
            t = colAlpha(c);
            s = 256 - t;
            for (x = x0; x < x1; x++)
            {
                db[4*x + 0] = (t*r + s*db[4*x + 0]) >> 8;
                db[4*x + 1] = (t*g + s*db[4*x + 1]) >> 8;
                db[4*x + 2] = (t*b + s*db[4*x + 2]) >> 8;
                db[4*x + 3] = 255;
            }
        }
        else
        {
            udword* udb = (udword*)db;
            for (x = x0; x < x1; x++)
            {
                udb[x] = c;
            }
        }
    }
}

static void glcRectangleShaded(rectangle* rect, color* c)
{
    udword* udb;
    sdword y, x;
    sdword y0, y1;
    sdword x0, x1;
    sdword xwidth;
    sdword r, g, b;
    sdword rMax, gMax, bMax;
    sdword rDiff, gDiff, bDiff;

    if (rect->y0 >= rect->y1 || rect->x0 >= rect->x1)
    {
        return;
    }

    y0 = _heightMinusOne - (rect->y0 - yDiff);
    y1 = _heightMinusOne - (rect->y1 - yDiff);
    if (y0 > y1)
    {
        y = y0;
        y0 = y1;
        y1 = y;
    }
    else if (y0 == y1)
    {
        return;
    }

    x0 = rect->x0;
    x1 = rect->x1;
    x0 -= xDiff;
    x1 -= xDiff;
    if (x0 < 0) x0 = 0;
    if (x0 >= _width) return;
    if (x1 < 0) return;
    if (x1 >= _width) x1 = _width;
    xwidth = x1 - x0;
    if (xwidth <= 0)
    {
        return;
    }

    glcUpdateExtents(x0, x1, y0, y1);

    r = colRed(c[0]);
    g = colGreen(c[0]);
    b = colBlue(c[0]);
    rMax = colRed(c[2]);
    gMax = colGreen(c[2]);
    bMax = colBlue(c[2]);
    rDiff = (rMax - r) / xwidth;
    gDiff = (gMax - g) / xwidth;
    bDiff = (bMax - b) / xwidth;

    for (y = y0; y < y1; y++)
    {
        if (y < 0 || y >= _height)
        {
            continue;
        }

        r = colRed(c[0]);
        g = colGreen(c[0]);
        b = colBlue(c[0]);

        udb = (udword*)(glScratch + glcScrMult[y]);
        //this is so slow ... lotsa compares
        for (x = x0; x < x1; x++)
        {
            udb[x] = colRGB(r, g, b);
            r += rDiff;
            g += gDiff;
            b += bDiff;
            if (r > rMax) r = rMax;
            else if (r < 0) r = 0;
            if (g > gMax) g = gMax;
            else if (g < 0) g = 0;
            if (b > bMax) b = bMax;
            else if (b < 0) b = 0;
        }
    }
}

void glcRectSolid2(rectangle* rect, color c)
{
    dbgAssert(glScratch != NULL);
    glcRectangle(rect, c, FALSE);
}

void glcBeveledRectSolid2(rectangle* rect, color c, sdword xb, sdword yb)
{
    dbgAssert(glScratch != NULL);
    glcRectangle(rect, c, FALSE);
}

void glcRectTranslucent2(rectangle* rect, color c)
{
    dbgAssert(glScratch != NULL);
    glcRectangle(rect, c, TRUE);
}

void glcRectShaded2(rectangle* rect, color* c)
{
    dbgAssert(glScratch != NULL);
    glcRectangleShaded(rect, c);
}

void glcPoint2(sdword x, sdword y, sdword thickness, color c)
{
    color* db;

    x -= xDiff;
    y = _heightMinusOne - (y - yDiff);

    //ensure 255 alpha
    c |= 0xFF000000;

    if (x < 0 || x >= _width || y < 0 || y >= _height)
    {
        return;
    }

    glcUpdateExtents(x, x, y, y);

    db = (color*)(glScratch + glcScrMult[y] + 4*x);
    *db = c;

    if (thickness > 1)
    {
        db[1] = c;
        db[-_width] = c;
        db[-_width+1] = c;
    }
}

void glcLine2(sdword x0, sdword y0, sdword x1, sdword y1, sdword thickness, color c)
{
    sdword dx, dy;
    sdword xstep, ystep;
    sdword width, min, max;

    width = thickness;
    min = -width / 2;
    max = min + width - 1;

    dx = x1 - x0;
    dy = y1 - y0;
    if (dx == 0 && dy == 0)
    {
        return;
    }

    if (dx < 0)
    {
        dx = -dx;   /* make positive */
        xstep = -1;
    }
    else
    {
        xstep = 1;
    }

    if (dy < 0)
    {
        dy = -dy;   /* make positive */
        ystep = -1;
    }
    else
    {
        ystep = 1;
    }

    if (dx > dy)
    {
        /*
         * X-major line
         */
        sdword i;
        sdword errorInc = dy + dy;
        sdword error = errorInc - dx;
        sdword errorDec = error - dx;

        for (i = 0; i < dx; i++)
        {
            sdword yy;
            sdword ymin = y0 + min;
            sdword ymax = y0 + max;
            for (yy = ymin; yy <= ymax; yy++)
            {
                glcPoint2(x0, yy, 1, c);
            }
            x0 += xstep;
            if (error < 0)
            {
                error += errorInc;
            }
            else
            {
                error += errorDec;
                y0 += ystep;
            }
        }
    }
    else
    {
        /*
         * Y-major line
         */
        sdword i;
        sdword errorInc = dx + dx;
        sdword error = errorInc - dy;
        sdword errorDec = error - dy;

        for (i = 0; i < dy; i++)
        {
            sdword xx;
            sdword xmin = x0 + min;
            sdword xmax = x0 + max;
            for (xx = xmin; xx <= xmax; xx++)
            {
                glcPoint2(xx, y0, 1, c);
            }
            y0 += ystep;
            if (error < 0)
            {
                error += errorInc;
            }
            else
            {
                error += errorDec;
                x0 += xstep;
            }
        }
    }
}

//x raster table for triangle scanconversion
static real32 scan[1200][2];

/*-----------------------------------------------------------------------------
    Name        : scan_convert
    Description : rasterizes a triangle
    Inputs      : x0, y0; x1, y1 - edge
    Outputs     : scan (y-table) is filled w/ x coords
    Return      :
----------------------------------------------------------------------------*/
static void scan_convert(sdword x0, sdword y0, sdword x1, sdword y1)
{
    sdword right, y, ey;
    real32 dx, x;

    if (y0 == y1)
    {
        //horizontal edge
        return;
    }

    if (y0 < y1)
    {
        //left edge
        right = 0;
    }
    else
    {
        //right edge
        swapInt(x0, x1);
        swapInt(y0, y1);
        right = 1;
    }

    //dYdX
    dx = (real32)(x1 - x0) / (real32)(y1 - y0);

    //setup
    x = (real32)x0;
    y = y0;
    ey = y1;

    //scan
    while (y < ey)
    {
        scan[y][right] = x;
        y++;
        x += dx;
    }
}

/*-----------------------------------------------------------------------------
    Name        : barrett_triangle
    Description : display a flat-shaded triangle in the given colour.
                  accuracy is not a concern here
    Inputs      : x0, y0, ... triangle vertex locations
                  c - colour to fill
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void barrett_triangle(sdword x0, sdword y0, sdword x1, sdword y1, sdword x2, sdword y2, color c)
{
    sdword ymin, ymax;
    sdword y, ey;
    sdword x;

    //find min/max y extent
    ymin = ymax = y0;
    if      (y1 < ymin) ymin = y1;
    else if (y1 > ymax) ymax = y1;
    if      (y2 < ymin) ymin = y2;
    else if (y2 > ymax) ymax = y2;

    //scan edges
    scan_convert(x0, y0, x2, y2);
    scan_convert(x1, y1, x0, y0);
    scan_convert(x2, y2, x1, y1);

    //setup
    y  = ymin;
    ey = ymax;

    while (y < ey)
    {
        sdword n;
        sdword sx = (sdword)scan[y][0];
        sdword ex = (sdword)scan[y][1];

        if (sx < ex)
        {
            //l -> r
            n = ex - sx;
        }
        else
        {
            //r -> l
            n = sx - ex;
            sx = ex;
        }

        if (n == 0)
        {
            //0-width
            y++;
            continue;
        }

        //fill raster
        for (x = 0; x < n; x++)
        {
            glcPoint2(sx+x, y, 1, c);
        }

        //next raster line
        y++;
    }
}

void glcTriSolid2(triangle* tri, color c)
{
    dbgAssert(glScratch != NULL);

    barrett_triangle(tri->x0, tri->y0,
                     tri->x1, tri->y1,
                     tri->x2, tri->y2,
                     c);
}

void glcTriOutline2(triangle* tri, sdword thickness, color c)
{
    dbgAssert(glScratch != NULL);

    glcLine2(tri->x0, tri->y0, tri->x1, tri->y1, thickness, c);
    glcLine2(tri->x1, tri->y1, tri->x2, tri->y2, thickness, c);
    glcLine2(tri->x2, tri->y2, tri->x0, tri->y0, thickness, c);
}

void glcRectSolidTexturedScaled2(rectangle* rect, sdword width, sdword height, ubyte* data, color* palette, bool reverse)
{
    ubyte* newData;
    sdword newHeight, newWidth;
    real32 ystep, ry;
    real32 xstep, rx;
    sdword newy, oldy;
    sdword newx, oldx;
    color* sp;
    color* dp;
    ubyte* sb;
    ubyte* db;

    newHeight = abs(rect->y1 - rect->y0);
    newWidth  = abs(rect->x1 - rect->x0);
    if ((newWidth == width) && (newHeight == height))
    {
        //don't actually need to scale anything
        glcRectSolidTextured2(rect, width, height, data, palette, reverse);
        return;
    }
    newData = (ubyte*)memAlloc(((palette == NULL) ? 4 : 1) * newWidth * newHeight,
                               "scaledGLCTEX", Volatile);

    ystep = (real32)height / (real32)newHeight;
    xstep = (real32)width  / (real32)newWidth;
    ry = 0.0f;
    for (newy = 0; newy < newHeight; newy++, ry += ystep)
    {
        oldy = (sdword)ry;
        rx = 0.0f;
        if (palette == NULL)
        {
            for (newx = 0; newx < newWidth; newx++, rx += xstep)
            {
                oldx = (sdword)rx;
                sp = (color*)(data + 4*width*oldy + 4*oldx);
                dp = (color*)(newData + 4*newWidth*newy + 4*newx);
                *dp = *sp;
            }
        }
        else
        {
            for (newx = 0; newx < newWidth; newx++, rx += xstep)
            {
                oldx = (sdword)rx;
                sb = data + width*oldy + oldx;
                db = newData + newWidth*newy + newx;
                *db = *sb;
            }
        }
    }
    glcRectSolidTextured2(rect, newWidth, newHeight, newData, palette, reverse);
    memFree(newData);
}

void glcRectSolidTextured2(rectangle* rect, sdword width, sdword height, ubyte* data, color* palette, bool reverse)
{
    ubyte* bdp;
    color* cdp;
    color* udb;
    sdword y, x, xwidth;
    sdword y0, y1;
    sdword x0, x1;

    if (rect->y0 >= rect->y1 || rect->x0 >= rect->x1)
    {
        return;
    }

    y0 = _heightMinusOne - (rect->y0 - yDiff);
    y1 = _heightMinusOne - (rect->y1 - yDiff);
    if (y0 > y1)
    {
        y = y0;
        y0 = y1;
        y1 = y;
    }

    x0 = rect->x0 - xDiff;
    x1 = rect->x1 - xDiff;
    if (x0 < 0) x0 = 0;
    if (x0 >= _width) return;
    if (x1 < 0) return;
    if (x1 >= _width) x1 = _width;
    xwidth = x1 - x0;
    if (xwidth <= 0)
    {
        return;
    }

    glcUpdateExtents(x0, x1, y0, y1);

    if (palette == NULL)
    {
        cdp = reverse ? (color*)data + width*(height-1) : (color*)data;
        for (y = y0; y < y1; y++, cdp = reverse ? cdp - width : cdp + width)
        {
            if (y < 0 || y >= _height)
            {
                continue;
            }
            udb = (color*)(glScratch + glcScrMult[y] + 4*x0);
            memcpy(udb, cdp, 4*xwidth);
        }
    }
    else
    {
        bdp = reverse ? data + width*(height-1) : data;
        for (y = y0; y < y1; y++, bdp = reverse ? bdp - width : bdp + width)
        {
            if (y < 0 || y >= _height)
            {
                continue;
            }
            udb = (color*)(glScratch + glcScrMult[y] + 4*x0);
            for (x = 0; x < xwidth; x++)
            {
                udb[x] = palette[bdp[x]];
            }
        }
    }
}

void glcRectOutline2(rectangle* rect, sdword thickness, color c)
{
    glcLine2(rect->x0, rect->y0, rect->x1, rect->y0, thickness, c);
    glcLine2(rect->x1, rect->y0, rect->x1, rect->y1, thickness, c);
    glcLine2(rect->x1, rect->y1, rect->x0, rect->y1, thickness, c);
    glcLine2(rect->x0, rect->y1, rect->x0, rect->y0, thickness, c);
}

void _glcRectOutline2(rectangle* rect, sdword thickness, color c)
{
    udword* db0;
    udword* db1;
    sdword y, x;
    sdword y0, y1;
    sdword x0, x1;

    dbgAssert(glScratch != NULL);

    y0 = _heightMinusOne - (rect->y0 - yDiff);
    y1 = _heightMinusOne - (rect->y1 - yDiff);
    if (y0 > y1)
    {
        y = y0;
        y0 = y1;
        y1 = y;
    }

    x0 = rect->x0 - xDiff;
    x1 = rect->x1 - xDiff;

    glcUpdateExtents(x0, x1, y0, y1);

    //ensure 255 alpha
    c |= 0xFF000000;

    //top & bottom
    db0 = (udword*)(glScratch + glcScrMult[y0]);
    db1 = (udword*)(glScratch + glcScrMult[y1]);
    for (x = x0; x <= x1; x++)
    {
        db0[x] = c;
        db1[x] = c;
    }

    //left & right
    db0 = (udword*)(glScratch + glcScrMult[y0] + 4*x0);
    db1 = (udword*)(glScratch + glcScrMult[y0] + 4*x1);
    for (y = y0; y < y1; y++, db0 += _width, db1 += _width)
    {
        *db0 = c;
        *db1 = c;
    }
}

static void glcDrawPixelsHelper(GLenum format, sdword width, sdword height, ubyte const* data)
{
    ubyte* pb;
    ubyte* db;
    sdword y, x;
    sdword py, px;
    sdword xlead, pitch;
    sdword alpha;
    sdword r, g, b;

    pitch = width;

    px = glcRasterPos[0] - xDiff;
    py = glcRasterPos[1] - yDiff;

    glcUpdateExtents(px, px+width, py, py+height);

    pb = (ubyte*)data;

    r = glcColor[0];
    g = glcColor[1];
    b = glcColor[2];

    if (px < 0)
    {
        xlead = -px;
        width += px;
        px = 0;
        if (width <= 0)
        {
            return;
        }
    }
    else
    {
        xlead = 0;
    }

    if (px+width > _width)
    {
        width -= (px+width) - _width + 1;
        if (width <= 0)
        {
            return;
        }
    }

    if (format == GL_RGBA)
    {
        for (y = 0; y < height; y++, pb += 4*(pitch+xlead))
        {
            if (py+y < 0 || py+y >= _height)
            {
                continue;
            }

            db = glScratch + glcScrMult[py+y] + 4*(px+xlead);

            if (glcBlend || glcAlphaTest)
            {
                for (x = 0; x < width; x++, db += 4)
                {
                    if (pb[4*x + 3] > 0)
                    {
                        db[0] = pb[4*x + 0];
                        db[1] = pb[4*x + 1];
                        db[2] = pb[4*x + 2];
                        db[3] = 255;
                    }
                }
            }
            else
            {
                for (x = 0; x < width; x++, db += 4)
                {
                    db[0] = pb[4*x + 0];
                    db[1] = pb[4*x + 1];
                    db[2] = pb[4*x + 2];
                    db[3] = 255;
                }
            }
        }
    }
    else if (format == GL_RGB8)
    {
        for (y = 0; y < height; y++, pb += pitch + xlead)
        {
            if (py+y < 0 || py+y >= _height)
            {
                continue;
            }

            db = glScratch + glcScrMult[py+y] + 4*(px+xlead);

            for (x = 0; x < width; x++, db += 4)
            {
                if (pb[x] > 0)
                {
                    alpha = pb[x] * 16;
                    db[0] = (r * alpha) >> 8;
                    db[1] = (g * alpha) >> 8;
                    db[2] = (b * alpha) >> 8;
                    db[3] = 255;
                }
            }
        }
    }
}

void APIENTRY glcDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid const* pixels)
{
    dbgAssert(glScratch != NULL);
    dbgAssert(pixels != NULL);

    switch (format)
    {
    case GL_RGBA:
    case GL_RGB8:
        glcDrawPixelsHelper(format, (sdword)width, (sdword)height, (ubyte const*)pixels);
        break;
    default:
        dbgFatalf(DBG_Loc, "glcDrawPixels: unknown format %d", (sdword)format);
    }
}

void APIENTRY glcRasterPos2f(GLfloat x, GLfloat y)
{
    real32 v[4], eye[4], clip[4];
    real32 viewport[2][2];
    real32 d, ndc[3];
    real32 modelview[16], projection[16];

    glGetFloatv(GL_MODELVIEW_MATRIX,  (GLfloat*)modelview);
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat*)projection);

    v[0] = x;
    v[1] = y;
    v[2] = 0.0f;
    v[3] = 1.0f;
    TRANSFORM_POINT(eye, modelview, v);
    TRANSFORM_POINT(clip, projection, eye);

    viewport[0][0] = (real32)MAIN_WindowWidth  / 2.0f;
    viewport[0][1] = viewport[0][0];
    viewport[1][0] = (real32)MAIN_WindowHeight / 2.0f;
    viewport[1][1] = viewport[1][0];

    d = 1.0f / clip[3];
    ndc[0] = clip[0] * d;
    ndc[1] = clip[1] * d;
    ndc[2] = clip[2] * d;
    glcRasterPos[0] = (sdword)(ndc[0] * viewport[0][0] + viewport[0][1]);
    glcRasterPos[1] = (sdword)(ndc[1] * viewport[1][0] + viewport[1][1]);
}

void APIENTRY glcClear(GLbitfield mask)
{
    if ((glScratch != NULL) &&
        (mask & GL_COLOR_BUFFER_BIT))
    {
        memset(glScratch, 0, _pitch*_height);
    }
    _glcClear(mask);
}

void APIENTRY glcColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    glcColor[0] = (sdword)r;
    glcColor[1] = (sdword)g;
    glcColor[2] = (sdword)b;
    _glcColor3ub(r, g, b);
}

void APIENTRY glcEnable(GLenum cap)
{
    switch (cap)
    {
    case GL_BLEND:
        glcBlend = TRUE;
        break;
    case GL_ALPHA_TEST:
        glcAlphaTest = TRUE;
        break;
    case GL_SCISSOR_TEST:
        glcScissorTest = TRUE;
        break;
    }
    _glcEnable(cap);
}

void APIENTRY glcDisable(GLenum cap)
{
    switch (cap)
    {
    case GL_BLEND:
        glcBlend = FALSE;
        break;
    case GL_ALPHA_TEST:
        glcAlphaTest = FALSE;
        break;
    case GL_SCISSOR_TEST:
        glcScissorTest = FALSE;
        break;
    }
    _glcDisable(cap);
}

/*-----------------------------------------------------------------------------
    Name        : glcCursorUnder
    Description : backing re/store for the mouse pointer
    Inputs      : data - backing storage
                  width, height - dimensions of backing store rectangle
                  x0, y0 - backing store location on screen
                  store - TRUE or FALSE (store or restore)
    Outputs     : store ? data is filled : data is written to glScratch
    Return      :
----------------------------------------------------------------------------*/
void glcCursorUnder(ubyte* data, sdword width, sdword height, sdword x0, sdword y0, bool store)
{
    sdword ybegin, yend;
    sdword xbegin, xend;
    sdword y, x;
    udword* udb;
    udword* ufb;

    if (glScratch == NULL)
    {
        return;
    }

    x0 -= xDiff;
    y0 -= yDiff;

    ybegin = _heightMinusOne - y0;
    yend   = ybegin - height;
    if (ybegin > yend)
    {
        y = ybegin;
        ybegin = yend;
        yend = y;
    }

    xbegin = x0;
    xend   = xbegin + width;

    ybegin--;
    yend++;
    xbegin--;
    xend++;

    if (xbegin >= _width || xend < 0)
    {
        return;
    }

    udb = (udword*)data;

    for (y = ybegin; y < yend; y++)
    {
        if (y < 0 || y >= _height)
        {
            for (x = 0; x < width; x++, udb++)
            {
                *udb = 0;//FEC_Background;
            }
            continue;
        }
        ufb = (udword*)(glScratch + glcScrMult[y]);
        for (x = xbegin; x < xend; x++, udb++)
        {
            if (x >= 0 && x < _width)
            {
                if (store)
                {
                    *udb = ufb[x];
                }
                else
                {
                    ufb[x] = *udb;
                }
            }
            else
            {
                if (store)
                {
                    *udb = 0;//FEC_Background;
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : glcActivate
    Description : de/activate the glcompat module
    Inputs      : active - TRUE or FALSE
    Outputs     :
    Return      : previous active status
----------------------------------------------------------------------------*/
bool glcActivate(bool active)
{
    bool lastStatus;

#ifdef _MACOSX_FIX_ME  // without this you get a wacky red front end menu
    active = FALSE;
#endif

    if (!glCapFeatureExists(GL_SWAPFRIENDLY))
    {
        return glcActiveStatus;
    }
    if (glcActiveStatus == active)
    {
        //avoid redundant state change
        return glcActiveStatus;
    }
    lastStatus = glcActiveStatus;
    glcActiveStatus = active;
    if (active)
    {
        if (gl3Dfx)
        {
            glcLaggy = TRUE;
        }
        else if (strstr(GLC_RENDERER, "Voodoo Graphics") != NULL ||
                 strstr(GLC_RENDERER, "Voodoo2") != NULL)
        {
            glcLaggy = TRUE;
        }
        else
        {
            glcLaggy = FALSE;
        }

        glcBlend = (bool)glIsEnabled(GL_BLEND);
        glcAlphaTest = (bool)glIsEnabled(GL_ALPHA_TEST);
        glcScissorTest = (bool)glIsEnabled(GL_SCISSOR_TEST);

        _glcDrawPixels = (DRAWPIXELSproc)glDrawPixels;
        glDrawPixels = (DRAWPIXELSproc)glcDrawPixels;
        dbgAssert(glDrawPixels != _glcDrawPixels);

        _glcRasterPos2f = (RASTERPOS2Fproc)glRasterPos2f;
        glRasterPos2f = (RASTERPOS2Fproc)glcRasterPos2f;
        dbgAssert(glRasterPos2f != _glcRasterPos2f);

        _glcColor3ub = (COLOR3UBproc)glColor3ub;
        glColor3ub = (COLOR3UBproc)glcColor3ub;
        dbgAssert(glColor3ub != _glcColor3ub);

        _glcClear = (CLEARproc)glClear;
        glClear = (CLEARproc)glcClear;
        dbgAssert(glClear != _glcClear);

        _glcEnable = glEnable;
        glEnable = (ENABLEproc)glcEnable;
        dbgAssert(glEnable != _glcEnable);

        _glcDisable = glDisable;
        glDisable = (DISABLEproc)glcDisable;
        dbgAssert(glDisable != _glcDisable);

        (void)glcAllocateScratch(TRUE);

        glcNumBuffers = glCapNumBuffers();
        glcForceUpdates = glcNumBuffers;
        glcActiveBuffer = 0;

        glcDrawingMouse = FALSE;
        glcUpdateExtents(-1, -1, -1, -1);
        glcRenderEverythingALot();
    }
    else
    {
        if (glNT && gl3Dfx)
        {
            ;
        }
        else
        {
            glcPurgeHandles();
        }
        glDLLGetGLCompat();
        (void)glcAllocateScratch(FALSE);
    }
    return lastStatus;
}

/*-----------------------------------------------------------------------------
    Name        : glcIsFullscreen
    Description : returns fullscreen status (in-game glcActive ...)
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool glcIsFullscreen(void)
{
    return glcFullscreenStatus;
}

/*-----------------------------------------------------------------------------
    Name        : glcFullscreen
    Description : switches into / outof fullscreen mode (in-game glcActivate ...)
    Inputs      : full - TRUE or FALSE, desired state
    Outputs     :
    Return      : previous state
----------------------------------------------------------------------------*/
bool glcFullscreen(bool full)
{
    bool lastStatus;

#if !GLC_FULLSCREEN
    return FALSE;
#endif

    if (RGLtype == SWtype)
    {
        return FALSE;
    }

    if (!glCapFeatureExists(GL_SWAPFRIENDLY))
    {
        return FALSE;
    }

    if (full == glcFullscreenStatus)
    {
        //avoid redundant state change
        return full;
    }

    lastStatus = glcFullscreenStatus;
    if (full)
    {
        (void)glcActivate(TRUE);
    }
    else
    {
        (void)glcActivate(FALSE);
    }
    glcFullscreenStatus = full;
    return lastStatus;
}

/*-----------------------------------------------------------------------------
    Name        : glcRenderEverything
    Description : force glcompat to re-render the entire screen
    Inputs      :
    Outputs     : glcForceUpdate[..] = glcForceUpdates
    Return      :
----------------------------------------------------------------------------*/
void glcRenderEverything(void)
{
    sdword i;

    for (i = 0; i < GLC_MAX_BUFFERS; i++)
    {
        glcForceUpdate[i] = glcForceUpdates;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glcRenderEverythingALot
    Description : force glcompat to re-render the entire screen 2x the amount
                  that glcRenderEverything would
    Inputs      :
    Outputs     : glcForceUpdate[..] = 2 * glcForceUpdates
    Return      :
----------------------------------------------------------------------------*/
void glcRenderEverythingALot(void)
{
    sdword i;

    for (i = 0; i < GLC_MAX_BUFFERS; i++)
    {
        glcForceUpdate[i] = 2 * glcForceUpdates;
    }
}
