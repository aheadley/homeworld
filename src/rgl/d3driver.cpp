/*=============================================================================
    Name    : d3driver.cpp
    Purpose : Direct3D driver for rGL (simple rasterization-only first pass)

    Created 9/23/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "d3dlist.h"
#include "d3dinit.h"
#include "d3dtex.h"
#include "d3dblt.h"

#define DR DriverFuncs

#define VERTEX_FLAGS D3DDP_DONOTCLIP | D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS

#define SETUP setupFunc
void choose_setup_function(GLcontext* ctx);

typedef void (*setupFunc_t)(GLuint vl[], GLuint start, GLuint n, GLuint pv);
static setupFunc_t setupFunc = NULL;

static double chop_temp;

void d3d_fullscene(GLboolean on);

GLcontext* CTX = NULL;
d3d_context* D3D = NULL;

listdd ddList;          //DirectDraw enumerants
listdev devList;        //Direct3D devices
listpf texList;         //texture formats
listpf loDepthList;     //<= 16bit depthbuffers
listpf hiDepthList;     //> 16bit depthbuffers

/* vertex formats */
typedef struct xyz_c
{
    float x, y, z;
    DWORD c;
} xyz_c;

typedef struct xyz_t
{
    float x, y, z;
    float s, t;
} xyz_t;

typedef struct xyz_c_t
{
    float x, y, z;
    DWORD c;
    float s, t;
} xyz_c_t;

static GLint nVerts = 0;
static xyz_c vert_c[VB_SIZE];
static xyz_t vert_t[VB_SIZE];
static xyz_c_t vert_c_t[VB_SIZE];

typedef struct xyzw_c
{
    float x, y, z;
    float w;
    DWORD c;
} xyzw_c;

typedef struct xyzw_c_t
{
    float x, y, z;
    float w;
    DWORD c;
    float s, t;
} xyzw_c_t;

typedef struct xyzw_t
{
    float x, y, z;
    float w;
    float s, t;
} xyzw_t;

#define MAX_VERTS VB_SIZE

static xyzw_c   d3dVert[MAX_VERTS];
static xyzw_c_t d3dVertct[MAX_VERTS];
static xyzw_t   d3dVertt[MAX_VERTS];

static GLint  gVertexNumber;
static DWORD  gVertexFlags;
static DWORD  gVertexType;
static LPVOID gVertices;

void bind_texture(void);

typedef struct error_table
{
    HRESULT hr;
    char*   english;
} error_table;

static error_table errTab[] =
{
    {DDERR_DIRECTDRAWALREADYCREATED, "dd already created"},
    {DDERR_GENERIC, "dd generic"},
    {DDERR_INVALIDDIRECTDRAWGUID, "dd invalid guid"},
    {DDERR_INVALIDPARAMS, "dd invalid params"},
    {DDERR_NODIRECTDRAWHW, "dd no ddraw hw"},
    {DDERR_OUTOFMEMORY, "dd out of memory"},

    {DDERR_INCOMPATIBLEPRIMARY, "incompatible primary"},
    {DDERR_INVALIDCAPS, "invalid caps"},
    {DDERR_INVALIDOBJECT, "invalid object"},
    {DDERR_INVALIDPIXELFORMAT, "invalid pixel format"},
    {DDERR_NOALPHAHW, "no alpha hw"},
    {DDERR_NOCOOPERATIVELEVELSET, "no cooperative level set"},
    {DDERR_NOEMULATION, "no emulation"},
    {DDERR_NOEXCLUSIVEMODE, "no exclusive mode"},
    {DDERR_NOFLIPHW, "no flip hw"},
    {DDERR_NOMIPMAPHW, "no mipmap hw"},
    {DDERR_NOOVERLAYHW, "no overlay hw"},
    {DDERR_NOZBUFFERHW, "no zbuffer hw"},
    {DDERR_OUTOFVIDEOMEMORY, "out of video memory"},
    {DDERR_PRIMARYSURFACEALREADYEXISTS, "primary surface already exists"},
    {DDERR_UNSUPPORTEDMODE, "unsupported mode"},

    {D3DERR_BADMAJORVERSION, "bad major version"},
    {D3DERR_BADMINORVERSION, "bad minor version"},
    {D3DERR_COLORKEYATTACHED, "color key attached"},
    {D3DERR_CONFLICTINGTEXTUREFILTER, "conflicting texture filter"},
    {D3DERR_CONFLICTINGRENDERSTATE, "conflicting raster state"},
    {D3DERR_DEVICEAGGREGATED, "device aggregated"},
    {D3DERR_EXECUTE_CLIPPED_FAILED, "execute clipped failed"},
    {D3DERR_EXECUTE_CREATE_FAILED, "execute create failed"},
    {D3DERR_EXECUTE_DESTROY_FAILED, "execute destroy failed"},
    {D3DERR_EXECUTE_FAILED, "execute failed"},
    {D3DERR_EXECUTE_LOCK_FAILED, "execute lock failed"},
    {D3DERR_EXECUTE_LOCKED, "execute locked"},
    {D3DERR_EXECUTE_NOT_LOCKED, "execute not locked"},
    {D3DERR_EXECUTE_UNLOCK_FAILED, "execute unlock failed"},
    {D3DERR_INITFAILED, "init failed"},
    {D3DERR_INBEGIN, "in begin"},
    {D3DERR_INVALID_DEVICE, "invalid device"},
    {D3DERR_INVALIDCURRENTVIEWPORT, "invalid current viewport"},
    {D3DERR_INVALIDMATRIX, "invalid matrix"},
    {D3DERR_INVALIDPALETTE, "invalid palette"},
    {D3DERR_INVALIDPRIMITIVETYPE, "invalid primitive type"},
    {D3DERR_INVALIDRAMPTEXTURE, "invalid ramp texture"},
    {D3DERR_INVALIDVERTEXFORMAT, "INVALIDVERTEXFORMAT"},
    {D3DERR_INVALIDVERTEXTYPE, "INVALIDVERTEXTYPE"},
    {D3DERR_LIGHT_SET_FAILED, "LIGHT_SET_FAILED"},
    {D3DERR_LIGHTHASVIEWPORT, "LIGHTHASVIEWPORT"},
    {D3DERR_LIGHTNOTINTHISVIEWPORT, "LIGHTNOTINTHISVIEWPORT"},
    {D3DERR_MATERIAL_CREATE_FAILED, "MATERIAL_CREATE_FAILED"},
    {D3DERR_MATERIAL_DESTROY_FAILED, "MATERIAL_DESTROY_FAILED"},
    {D3DERR_MATERIAL_GETDATA_FAILED, "MATERIAL_GETDATA_FAILED"},
    {D3DERR_MATERIAL_SETDATA_FAILED, "MATERIAL_SETDATA_FAILED"},
    {D3DERR_MATRIX_CREATE_FAILED, "MATRIX_CREATE_FAILED"},
    {D3DERR_MATRIX_DESTROY_FAILED, "MATRIX_DESTROY_FAILED"},
    {D3DERR_MATRIX_GETDATA_FAILED, "MATRIX_GETDATA_FAILED"},
    {D3DERR_MATRIX_SETDATA_FAILED, "MATRIX_SETDATA_FAILED"},
    {D3DERR_NOCURRENTVIEWPORT, "NOCURRENTVIEWPORT"},
    {D3DERR_NOTINBEGIN, "NOTINBEGIN"},
    {D3DERR_NOVIEWPORTS, "NOVIEWPORTS"},
    {D3DERR_SCENE_BEGIN_FAILED, "SCENE_BEGIN_FAILED"},
    {D3DERR_SCENE_END_FAILED, "SCENE_END_FAILED"},
    {D3DERR_SCENE_IN_SCENE, "SCENE_IN_SCENE"},
    {D3DERR_SCENE_NOT_IN_SCENE, "SCENE_NOT_IN_SCENE"},
    {D3DERR_SETVIEWPORTDATA_FAILED, "SETVIEWPORTDATA_FAILED"},
    {D3DERR_STENCILBUFFER_NOTPRESENT, "STENCILBUFFER_NOTPRESENT"},
    {D3DERR_SURFACENOTINVIDMEM, "SURFACENOTINVIDMEM"},
    {D3DERR_TEXTURE_BADSIZE, "TEXTURE_BADSIZE"},
    {D3DERR_TEXTURE_CREATE_FAILED, "TEXTURE_CREATE_FAILED"},
    {D3DERR_TEXTURE_DESTROY_FAILED, "TEXTURE_DESTROY_FAILED"},
    {D3DERR_TEXTURE_GETSURF_FAILED, "TEXTURE_GETSURF_FAILED"},
    {D3DERR_TEXTURE_LOAD_FAILED, "TEXTURE_LOAD_FAILED"},
    {D3DERR_TEXTURE_LOCK_FAILED, "TEXTURE_LOCK_FAILED"},
    {D3DERR_TEXTURE_LOCKED, "TEXTURE_LOCKED"},
    {D3DERR_TEXTURE_NO_SUPPORT, "TEXTURE_NO_SUPPORT"},
    {D3DERR_TEXTURE_NOT_LOCKED, "TEXTURE_NOT_LOCKED"},
    {D3DERR_TEXTURE_SWAP_FAILED, "TEXTURE_SWAP_FAILED"},
    {D3DERR_TEXTURE_UNLOCK_FAILED, "TEXTURE_UNLOCK_FAILED"},
    {D3DERR_TOOMANYOPERATIONS, "TOOMANYOPERATIONS"},
    {D3DERR_TOOMANYPRIMITIVES, "TOOMANYPRIMITIVES"},
    {D3DERR_UNSUPPORTEDALPHAARG, "UNSUPPORTEDALPHAARG"},
    {D3DERR_UNSUPPORTEDALPHAOPERATION, "UNSUPPORTEDALPHAOPERATION"},
    {D3DERR_UNSUPPORTEDCOLORARG, "UNSUPPORTEDCOLORARG"},
    {D3DERR_UNSUPPORTEDCOLOROPERATION, "UNSUPPORTEDCOLOROPERATION"},
    {D3DERR_UNSUPPORTEDFACTORVALUE, "UNSUPPORTEDFACTORVALUE"},
    {D3DERR_UNSUPPORTEDTEXTUREFILTER, "UNSUPPORTEDTEXTUREFILTER"},
    {D3DERR_VBUF_CREATE_FAILED, "VBUF_CREATE_FAILED"},
    {D3DERR_VERTEXBUFFERLOCKED, "VERTEXBUFFERLOCKED"},
    {D3DERR_VERTEXBUFFEROPTIMIZED, "VERTEXBUFFEROPTIMIZED"},
    {D3DERR_VIEWPORTDATANOTSET, "VIEWPORTDATANOTSET"},
    {D3DERR_VIEWPORTHASNODEVICE, "VIEWPORTHASNODEVICE"},
    {D3DERR_WRONGTEXTUREFORMAT, "WRONGTEXTUREFORMAT"},
    {D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY, "ZBUFF_NEEDS_SYSTEMMEMORY"},
    {D3DERR_ZBUFF_NEEDS_VIDEOMEMORY, "ZBUFF_NEEDS_VIDEOMEMORY"},
    {D3DERR_ZBUFFER_NOTPRESENT, "ZBUFFER_NOTPRESENT"},
    {D3D_OK, "D3D_OK"}
};

char* d3d_map_error(HRESULT hr)
{
    error_table* ep = errTab;
    if (hr == D3D_OK)
    {
        return "* D3D_OK *";
    }
    while (ep->hr != D3D_OK)
    {
        if (ep->hr == hr)
        {
            return ep->english;
        }
        ep++;
    }
    return "D3DERR_UNKNOWN";
}

void errLogFn(char* s, HRESULT hr)
{
    FILE* out;
    char* e;

    if (hr == D3D_OK)
    {
        out = fopen("d3d.err", "wt");
    }
    else
    {
        out = fopen("d3d.err", "at");
    }
    if (out == NULL)
    {
        return;
    }
    e = d3d_map_error(hr);
    fprintf(out, "%s [%s]\n", s, e);
    fclose(out);
}

void errLogNull(char* s, HRESULT hr)
{
    //nothing here
}

void logFn(char* s)
{
    FILE* out;

    if (s == NULL)
    {
        out = fopen("d3d.dat", "wt");
        if (out != NULL)
        {
            fclose(out);
        }
        return;
    }
    else
    {
        out = fopen("d3d.dat", "at");
    }
    if (out == NULL)
    {
        return;
    }
    fputs(s, out);
    fclose(out);
}

void logNull(char* s)
{
    //nothing here
}

char* depthToStr(DWORD depth)
{
    switch (depth)
    {
    case DDBD_1: return "1";
    case DDBD_2: return "2";
    case DDBD_4: return "4";
    case DDBD_8:  return "8";
    case DDBD_16: return "16";
    case DDBD_24: return "24";
    case DDBD_32: return "32";
    default: return "?";
    }
}

WORD GetNumberOfBits(DWORD mask)
{
    WORD bits = 0;

    while (mask)
    {
        mask = mask & (mask - 1);
        bits++;
    }

    return bits;
}

GLuint GetShiftBits(GLuint mask)
{
    GLuint test = 1, bits = 0;

    if (mask == 0)
    {
        return 0;
    }

    while (test)
    {
        if (mask & test)
        {
            return bits;
        }
        else
        {
            bits++;
            test <<= 1;
        }
    }

    return bits;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_fullscene
    Description : turn fullscene antialiasing on / off if supported
    Inputs      : on - TRUE or FALSE
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void d3d_fullscene(GLboolean on)
{
    if (!D3D->canAntialiasTriIndep)
    {
        return;
    }
    if (on)
    {
        D3D->d3dDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
        D3D->d3dDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, FALSE);
        D3D->d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, FALSE);
    }
    else
    {
        D3D->d3dDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
        D3D->d3dDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, FALSE);
        D3D->d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_begin_scene
    Description : begin a scene
    Inputs      : d3d - Direct3D context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
GLboolean d3d_begin_scene(d3d_context* d3d)
{
    HRESULT hr;

#if INSCENE_CHECKING
    if (d3d->inScene)
    {
        return GL_TRUE;
    }
#endif

    hr = d3d->d3dDevice->BeginScene();
    if (FAILED(hr))
    {
        errLog("d3d_begin_scene(BeginScene)", hr);
        return GL_FALSE;
    }
    else
    {
        d3d->inScene = GL_TRUE;
        return GL_TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_end_scene
    Description : end a scene
    Inputs      : d3d - Direct3D context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
GLboolean d3d_end_scene(d3d_context* d3d)
{
    HRESULT hr;

    //FIXME: this occurs when ddraw init fails
    if (d3d == NULL || d3d->d3dDevice == NULL)
    {
        return GL_TRUE;
    }

#if INSCENE_CHECKING
    if (!d3d->inScene)
    {
        return GL_TRUE;
    }
#endif

    hr = d3d->d3dDevice->EndScene();
    if (FAILED(hr))
    {
        errLog("d3d_end_scene(EndScene)", hr);
        return GL_FALSE;
    }
    else
    {
        d3d->inScene = GL_FALSE;
        return GL_TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_flush
    Description : perform backbuffer blt / flip, depending on buffer type
    Inputs      : d3d - Direct3D context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static GLboolean d3d_flush(d3d_context* d3d)
{
    HRESULT hr;

    if (d3d->PrimarySurface == NULL)
    {
        return GL_FALSE;
    }

#if PAGE_FLIPPING
    if (d3d->Fullscreen)
    {
        hr = d3d->PrimarySurface->Flip(NULL, DDFLIP_WAIT);
    }
    else
#endif
    {
        hr = d3d->PrimarySurface->Blt(
                &d3d->ScreenRect, d3d->BackSurface,
                &d3d->ViewportRect, DDBLT_WAIT, NULL);
    }
    if (FAILED(hr))
    {
        errLog("d3d_flush(Blt)", hr);
        return GL_FALSE;
    }
    else
    {
        return GL_TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_blend
    Description : maps GL blend ops -> D3D, restricted by current device's caps
    Inputs      : d3d - Direct3D context
                  source - TRUE if BlendSrc, FALSE if BlendDst
                  blend - the GL blend op type
    Outputs     :
    Return      : converted blend op
----------------------------------------------------------------------------*/
static D3DBLEND d3d_map_blend(d3d_context* d3d, GLboolean source, GLenum blend)
{
    if (source)
    {
        switch (blend)
        {
        case GL_ONE:
            if (d3d->canSrcBlendOne)
            {
                return D3DBLEND_ONE;
            }
            else
            {
                return d3d->srcBlendFallback;
            }

        case GL_ZERO:
            if (d3d->canSrcBlendZero)
            {
                return D3DBLEND_ZERO;
            }
            else
            {
                return d3d->srcBlendFallback;
            }

        default:
            if (d3d->canSrcBlendSrcAlpha)
            {
                return D3DBLEND_SRCALPHA;
            }
            else
            {
                return d3d->srcBlendFallback;
            }
        }
    }
    else
    {
        switch (blend)
        {
        case GL_ONE:
            if (d3d->canDestBlendOne)
            {
                return D3DBLEND_ONE;
            }
            else if (d3d->canDestBlendInvSrcAlpha)
            {
                return D3DBLEND_INVSRCALPHA;
            }
            else
            {
                return d3d->destBlendFallback;
            }

        case GL_ZERO:
            if (d3d->canDestBlendZero)
            {
                return D3DBLEND_ZERO;
            }
            else
            {
                return d3d->destBlendFallback;
            }

        default:
            if (d3d->canDestBlendInvSrcAlpha)
            {
                return D3DBLEND_INVSRCALPHA;
            }
            else if (d3d->canDestBlendOne)
            {
                return D3DBLEND_ONE;
            }
            else
            {
                return d3d->destBlendFallback;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_linesmooth
    Description : maps GL linesmoothing -> D3D, rectricted by current device's caps
    Inputs      : d3d - Direct3D context
                  on - TRUE (on) or FALSE (off)
    Outputs     :
    Return      : appropriate D3D antialias mode
----------------------------------------------------------------------------*/
static D3DANTIALIASMODE d3d_map_linesmooth(d3d_context* d3d, GLboolean on)
{
    if (on)
    {
        if (d3d->canAntialiasLineDep || d3d->canAntialiasTriDep)
        {
            return D3DANTIALIAS_SORTDEPENDENT;
        }
        else if (d3d->canAntialiasLineIndep || d3d->canAntialiasTriIndep)
        {
            return D3DANTIALIAS_SORTINDEPENDENT;
        }
        else
        {
            return D3DANTIALIAS_NONE;
        }
    }
    else
    {
        return D3DANTIALIAS_NONE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_depthenable
    Description : chooses appropriate depthenable param
    Inputs      : ctx - GL context
    Outputs     :
    Return      : type of depthbuffering (D3D enum), or FALSE (off)
----------------------------------------------------------------------------*/
static DWORD d3d_map_depthenable(GLcontext* ctx)
{
    d3d_context* d3d = (d3d_context*)ctx->DriverCtx;

    if (ctx->DepthTest)
    {
#if WILL_WBUFFER
        if (d3d->canWBuffer)
        {
            return D3DZB_USEW;
        }
        else
#endif
        {
            return D3DZB_TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_alphafunc
    Description : GL alphatest -> D3D mapper
    Inputs      : d3d - Direct3D context
                  alphaFunc - GL alphafunc to convert
    Outputs     :
    Return      : D3D alphafunc, subject to device caps
----------------------------------------------------------------------------*/
static D3DCMPFUNC d3d_map_alphafunc(d3d_context* d3d, GLenum alphaFunc)
{
    switch (alphaFunc)
    {
    case GL_GREATER:
        if (d3d->canAlphaTestGreater)
        {
            return D3DCMP_GREATER;
        }
        else
        {
            return d3d->alphaTestFallback;
        }

    case GL_LESS:
        if (d3d->canAlphaTestLess)
        {
            return D3DCMP_LESS;
        }
        else
        {
            return d3d->alphaTestFallback;
        }

    default:
        return d3d->alphaTestFallback;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_alpharef
    Description : GL -> D3D alphatest reference value mapper
    Inputs      : d3d - Direct3D context
                  alphaRef - [0..255]
    Outputs     :
    Return      : D3D-friendly alpharef val (fixed pt)
----------------------------------------------------------------------------*/
#define D3D_ALPHAMULT (ONE_OVER_255f * 65536.0f)
static DWORD d3d_map_alpharef(d3d_context* d3d, GLubyte alphaRef)
{
    return (DWORD)((float)alphaRef * D3D_ALPHAMULT);
}

/*-----------------------------------------------------------------------------
    Name        : d3d_wrap_s
    Description : maps GL texture wrap modes -> D3D (texture S coord)
    Inputs      : d3d - Direct3D context
                  wrap - GL wrap mode
    Outputs     :
    Return      : D3D wrap mode, subject to device caps
----------------------------------------------------------------------------*/
static void d3d_wrap_s(d3d_context* d3d, GLenum wrap)
{
#if PREFER_CLAMPING
    if (d3d->canClamp)
    {
        d3d->d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    }
#else
    switch (wrap)
    {
    case GL_REPEAT:
        if (d3d->canWrap)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        }
        break;

    case GL_CLAMP:
        if (d3d->canClamp)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : d3d_wrap_t
    Description : maps GL texture wrap modes -> D3D (texture T coord)
    Inputs      : d3d - Direct3D context
                  wrap - GL wrap mode
    Outputs     :
    Return      : D3D wrap mode, subject to device caps
----------------------------------------------------------------------------*/
static void d3d_wrap_t(d3d_context* d3d, GLenum wrap)
{
#if PREFER_CLAMPING
    if (d3d->canClamp)
    {
        d3d->d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
    }
#else
    switch (wrap)
    {
    case GL_REPEAT:
        if (d3d->canWrap)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        }
        break;

    case GL_CLAMP:
        if (d3d->canClamp)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : d3d_min_filter
    Description : maps GL texture filter mode -> D3D (min filter)
    Inputs      : d3d - Direct3D context
                  filter - GL filter type
    Outputs     :
    Return      : D3D filter mode
----------------------------------------------------------------------------*/
static void d3d_min_filter(d3d_context* d3d, GLenum filter)
{
    switch (filter)
    {
    case GL_LINEAR:
        if (d3d->canFilterLinear)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
        }
        else if (d3d->canFilterLinear)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        }
        break;

    default:
        if (d3d->canFilterNearest)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        }
        break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_mag_filter
    Description : maps GL texture filter mode -> D3D (mag filter)
    Inputs      : d3d - Direct3D context
                  filter - GL filter type
    Outputs     :
    Return      : D3D filter mode
----------------------------------------------------------------------------*/
static void d3d_mag_filter(d3d_context* d3d, GLenum filter)
{
    switch (filter)
    {
    case GL_LINEAR:
        if (d3d->canFilterLinear)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
        }
        else if (d3d->canFilterNearest)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        }
        break;

    default:
        if (d3d->canFilterNearest)
        {
            d3d->d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        }
        break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_setup_filter
    Description : maps GL wrap and filter modes -> D3D, and sets D3D state
    Inputs      : tex - GL texture object
    Outputs     : filter state is set
    Return      :
----------------------------------------------------------------------------*/
static void d3d_setup_filter(gl_texture_object* tex)
{
    d3d_texobj* t3d = (d3d_texobj*)tex->DriverData;
    d3d_context* d3d = D3D;

    if (t3d != NULL)
    {
        if (t3d->texWrapS != d3d->texWrapS)
        {
            d3d_wrap_s(d3d, t3d->texWrapS);
            d3d->texWrapS = t3d->texWrapS;
        }
        if (t3d->texWrapT != d3d->texWrapT)
        {
            d3d_wrap_t(d3d, t3d->texWrapT);
            d3d->texWrapT = t3d->texWrapT;
        }
        if (t3d->texMinFilter != d3d->texMinFilter)
        {
            d3d_min_filter(d3d, t3d->texMinFilter);
            d3d->texMinFilter = t3d->texMinFilter;
        }
        if (t3d->texMagFilter != d3d->texMagFilter)
        {
            d3d_mag_filter(d3d, t3d->texMagFilter);
            d3d->texMagFilter = t3d->texMagFilter;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_setup_texop
    Description : select a (new) texture op, ie. GL texture environment
    Inputs      : d3d - the Direct3D context
    Outputs     : textureop is set appropriately
    Return      :
----------------------------------------------------------------------------*/
static void d3d_setup_texop(d3d_context* d3d)
{
    D3DTEXTUREOP colorOp, alphaOp;

    switch (CTX->TexEnvMode)
    {
    case GL_MODULATE:
        if (d3d->canTexModulate)
        {
            if (CTX->Bias && d3d->canTexAdd)
            {
                colorOp = D3DTOP_ADD;
            }
            else
            {
                colorOp = D3DTOP_MODULATE;
            }
            alphaOp = D3DTOP_MODULATE;
        }
        else
        {
            return;
        }
        break;

    case GL_REPLACE:
        if (d3d->canTexSelectArg1)
        {
            colorOp = D3DTOP_SELECTARG1;
            if (d3d->canTexModulate)
            {
                alphaOp = D3DTOP_MODULATE;
            }
            else
            {
                alphaOp = D3DTOP_SELECTARG1;
            }
        }
        else
        {
            return;
        }
        break;

    default:
        ;//FIXME: report error
    }

    if (!CTX->TexEnabled)
    {
        colorOp = D3DTOP_DISABLE;
        alphaOp = D3DTOP_DISABLE;
    }

    if (d3d->colorOp != colorOp)
    {
        d3d->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, colorOp);
        d3d->colorOp = colorOp;
    }
    if (d3d->alphaOp != alphaOp)
    {
        d3d->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, alphaOp);
        d3d->alphaOp = alphaOp;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_cull
    Description : maps GL cullface mode -> D3D
    Inputs      : ctx - the GL context
    Outputs     :
    Return      : D3D cull mode
----------------------------------------------------------------------------*/
static D3DCULL d3d_map_cull(GLcontext* ctx)
{
    if (!ctx->DriverTransforms)
    {
        return D3DCULL_NONE;
    }
    if (ctx->CullFace)
    {
        return D3DCULL_CW;
    }
    else
    {
        return D3DCULL_NONE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_depthfunc
    Description : maps GL depthfunc -> D3D
    Inputs      : ctx - the GL context
    Outputs     :
    Return      : D3D zfunc
----------------------------------------------------------------------------*/
D3DCMPFUNC d3d_map_depthfunc(GLcontext* ctx)
{
    if (ctx->DepthFunc == GL_LEQUAL)
    {
        if (D3D->canZCmpLessEqual)
        {
            return D3DCMP_LESSEQUAL;
        }
        else
        {
            return D3DCMP_LESS;
        }
    }
    else
    {
        return D3DCMP_LESS;
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_alphablendstate
    Description : enable/disable alphablend state, if supported
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void d3d_map_alphablendstate(d3d_context* d3d, GLboolean state)
{
    if (d3d->canAlphaBlend)
    {
        d3d->d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, state ? TRUE : FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_alphablendparm
    Description : set alphablend parameters, if supported
    Inputs      : d3d - Direct3D context
                  src, dst - source, destination blending modes
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void d3d_map_alphablendparm(d3d_context* d3d, GLenum src, GLenum dest)
{
    if (d3d->canAlphaBlend)
    {
        d3d->d3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  d3d_map_blend(d3d, GL_TRUE,  src));
        d3d->d3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, d3d_map_blend(d3d, GL_FALSE, dest));
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_map_alphateststate
    Description : enable/disable alphatest state, if supported.  try alphablending
                  if no alphatest support exists.  if that fails, do nothing
    Inputs      : d3d - Direct3D context
                  state - TRUE or FALSE
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void d3d_map_alphateststate(d3d_context* d3d, GLboolean state)
{
    if (d3d->canAlphaTest)
    {
        d3d->d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, state ? TRUE : FALSE);
    }
    else
    {
        d3d_map_alphablendstate(d3d, state);
    }
}

/*-----------------------------------------------------------------------------
    Name        : init_raster
    Description : initialize the driver state
    Inputs      : ctx - the GL's context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void init_raster(GLcontext* ctx)
{
    d3d_context* d3d = (d3d_context*)ctx->DriverCtx;
    LPDIRECT3DDEVICE3 device = d3d->d3dDevice;

    //texture environment
    d3d->colorOp = D3DTOP_SELECTARG2;   //never use, force reset
    d3d->alphaOp = D3DTOP_SELECTARG2;
    d3d_setup_texop(d3d);

    //filtering
    d3d->texWrapS = GL_REPEAT;
    d3d->texWrapT = GL_REPEAT;
    d3d->texMinFilter = GL_NEAREST;
    d3d->texMagFilter = GL_NEAREST;
    d3d_wrap_s(d3d, GL_REPEAT);
    d3d_wrap_t(d3d, GL_REPEAT);
    d3d_min_filter(d3d, GL_NEAREST);
    d3d_mag_filter(d3d, GL_NEAREST);
    if (ctx->TexBoundObject != NULL)
    {
        d3d_setup_filter(ctx->TexBoundObject);
    }

    //dithering
    if (d3d->canDither)
    {
        if (ctx->Buffer.Depth <= 16)
        {
            device->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
        }
        else
        {
            device->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
        }
    }

    //culling
    d3d->CullFace = ctx->CullFace;
    device->SetRenderState(D3DRENDERSTATE_CULLMODE, d3d_map_cull(ctx));

    //specular
    device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);

    //depthbuffer
    d3d->DepthFunc = ctx->DepthFunc;
    device->SetRenderState(
        D3DRENDERSTATE_ZFUNC,
        d3d_map_depthfunc(ctx));

    d3d->DepthTest = ctx->DepthTest;
    device->SetRenderState(
        D3DRENDERSTATE_ZENABLE,
        d3d_map_depthenable(ctx));

    d3d->DepthWrite = ctx->DepthWrite;
    device->SetRenderState(
        D3DRENDERSTATE_ZWRITEENABLE,
        ctx->DepthWrite ? TRUE : FALSE);

    //ASSERT: devices that canWBuffer can also persp tex ?
    device->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE,
                           d3d->canPerspectiveCorrect ? TRUE : FALSE);

    //shademodel
    d3d->ShadeModel = ctx->ShadeModel;
    device->SetRenderState(
        D3DRENDERSTATE_SHADEMODE,
        (ctx->ShadeModel == GL_SMOOTH) ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);

    //blending
    d3d->LineSmooth = ctx->LineSmooth;
#if WILL_ANTIALIAS
    device->SetRenderState(
        D3DRENDERSTATE_ANTIALIAS, d3d_map_linesmooth(d3d, ctx->LineSmooth));
#else
    device->SetRenderState(
        D3DRENDERSTATE_ANTIALIAS, d3d_map_linesmooth(d3d, GL_FALSE));
#endif

    d3d->Blend = ctx->Blend;
    d3d_map_alphablendstate(d3d, ctx->Blend);

    d3d->BlendSrc = ctx->BlendSrc;
    d3d->BlendDst = ctx->BlendDst;
    d3d_map_alphablendparm(d3d, ctx->BlendSrc, ctx->BlendDst);

    //alphatest
    d3d->AlphaTest = ctx->AlphaTest;
    d3d_map_alphateststate(d3d, d3d->AlphaTest);

    d3d->AlphaFunc = ctx->AlphaFunc;
    device->SetRenderState(
        D3DRENDERSTATE_ALPHAFUNC, d3d_map_alphafunc(d3d, ctx->AlphaFunc));

    d3d->AlphaByteRef = ctx->AlphaByteRef;
    device->SetRenderState(
        D3DRENDERSTATE_ALPHAREF, d3d_map_alpharef(d3d, ctx->AlphaByteRef));
}

/*-----------------------------------------------------------------------------
    Name        : setup_raster
    Description : called from the GL whenever a state change has affected rendering
    Inputs      : ctx - the GL's context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void setup_raster(GLcontext* ctx)
{
    d3d_context* d3d = (d3d_context*)ctx->DriverCtx;
    LPDIRECT3DDEVICE3 device = d3d->d3dDevice;

    //pick vertex setup fn
    choose_setup_function(ctx);

    //culling
    if (d3d->CullFace != ctx->CullFace)
    {
        device->SetRenderState(D3DRENDERSTATE_CULLMODE, d3d_map_cull(ctx));
        d3d->CullFace = ctx->CullFace;
    }

    //texture environment
    d3d_setup_texop(d3d);

    //texture
    if (d3d->TexEnabled && !ctx->TexEnabled)
    {
        d3d->TexEnabled = ctx->TexEnabled;
        d3d->d3dDevice->SetTexture(0, NULL);
    }
    else if (!d3d->TexEnabled && ctx->TexEnabled)
    {
        d3d->TexEnabled = ctx->TexEnabled;
        bind_texture();
    }

    //depthbuffer
    if (d3d->DepthFunc != ctx->DepthFunc)
    {
        device->SetRenderState(
            D3DRENDERSTATE_ZFUNC,
            d3d_map_depthfunc(ctx));
        d3d->DepthFunc = ctx->DepthFunc;
    }
    if (d3d->DepthTest != ctx->DepthTest)
    {
        device->SetRenderState(
            D3DRENDERSTATE_ZENABLE,
            d3d_map_depthenable(ctx));
        d3d->DepthTest = ctx->DepthTest;
    }
    if (d3d->DepthWrite != ctx->DepthWrite)
    {
        device->SetRenderState(
            D3DRENDERSTATE_ZWRITEENABLE,
            ctx->DepthWrite ? TRUE : FALSE);
        d3d->DepthWrite = ctx->DepthWrite;
    }

    //shademodel
    if (d3d->ShadeModel != ctx->ShadeModel)
    {
        device->SetRenderState(
            D3DRENDERSTATE_SHADEMODE,
            (ctx->ShadeModel == GL_SMOOTH) ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);
        d3d->ShadeModel = ctx->ShadeModel;
    }

    //alphatest
    if (d3d->AlphaTest != ctx->AlphaTest)
    {
        d3d_map_alphateststate(d3d, ctx->AlphaTest);
        d3d->AlphaTest = ctx->AlphaTest;
    }
    if (d3d->AlphaFunc != ctx->AlphaFunc)
    {
        device->SetRenderState(
            D3DRENDERSTATE_ALPHATESTENABLE, d3d_map_alphafunc(d3d, ctx->AlphaFunc));
        d3d->AlphaFunc = ctx->AlphaFunc;
    }
    if (d3d->AlphaByteRef != ctx->AlphaByteRef)
    {
        device->SetRenderState(
            D3DRENDERSTATE_ALPHAREF, d3d_map_alpharef(d3d, ctx->AlphaByteRef));
        d3d->AlphaByteRef = ctx->AlphaByteRef;
    }

    //blending
#if WILL_ANTIALIAS
    if (d3d->LineSmooth != ctx->LineSmooth)
    {
        device->SetRenderState(D3DRENDERSTATE_ANTIALIAS, d3d_map_linesmooth(d3d, ctx->LineSmooth));
        d3d->LineSmooth = ctx->LineSmooth;
    }
#endif
    if (d3d->Blend != ctx->Blend)
    {
        d3d_map_alphablendstate(d3d, ctx->Blend);
        d3d->Blend = ctx->Blend;
    }

    if ((d3d->BlendSrc != ctx->BlendSrc) ||
        (d3d->BlendDst != ctx->BlendDst))
    {
        d3d_map_alphablendparm(d3d, ctx->BlendSrc, ctx->BlendDst);
        d3d->BlendSrc = ctx->BlendSrc;
        d3d->BlendDst = ctx->BlendDst;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clear_colorbuffer
    Description : device colorbuffer clear fn
    Inputs      : ctx - GL context
    Outputs     : clear colorbuffer
    Return      :
----------------------------------------------------------------------------*/
static void clear_colorbuffer(GLcontext* ctx)
{
    DWORD c;

    c = RGBA_MAKE(ctx->ClearColorByte[0],
                  ctx->ClearColorByte[1],
                  ctx->ClearColorByte[2],
                  ctx->ClearColorByte[3]);

    if (D3D->d3dViewport != NULL)
    {
        D3D->d3dViewport->Clear2(1ul, (D3DRECT*)&D3D->ViewportRect, D3DCLEAR_TARGET, c, 0l, 0l);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clear_depthbuffer
    Description : device depthbuffer clear fn
    Inputs      : ctx - GL context
    Outputs     : clear depthbuffer
    Return      :
----------------------------------------------------------------------------*/
static void clear_depthbuffer(GLcontext* ctx)
{
    if (D3D->d3dViewport != NULL)
    {
        D3D->d3dViewport->Clear2(1ul, (D3DRECT*)&D3D->ViewportRect, D3DCLEAR_ZBUFFER, 0, 1.0f, 0l);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clear_both_buffers
    Description : clear the colorbuffer and depthbuffer simultaneously, if device supports
    Inputs      : ctx - GL context
    Outputs     : clear color & depthbuffers
    Return      :
----------------------------------------------------------------------------*/
static void clear_both_buffers(GLcontext* ctx)
{
    DWORD c;

    c = RGBA_MAKE(ctx->ClearColorByte[0],
                  ctx->ClearColorByte[1],
                  ctx->ClearColorByte[2],
                  ctx->ClearColorByte[3]);

    if (D3D->d3dViewport != NULL)
    {
        D3D->d3dViewport->Clear2(1ul, (D3DRECT*)&D3D->ViewportRect,
                                 D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                                 c, 1.0f, 0l);
    }
}

/*-----------------------------------------------------------------------------
    Name        : set_monocolor
    Description : set the device's monocolor state
    Inputs      : ctx - GL context
                  r, g, b, a - [0..255]
    Outputs     : D3D context is adjusted
    Return      :
----------------------------------------------------------------------------*/
static void set_monocolor(GLcontext* ctx, GLint r, GLint g, GLint b, GLint a)
{
    D3D->Monocolor = RGBA_MAKE(r, g, b, a);
}

#define za(n) n/*(((n) - CTX->Viewport.Tz) / CTX->Viewport.Sz)*/

static void setup_xyzw_t(GLuint vl[], GLuint start, GLuint n, GLuint pv)
{
    vertex_buffer* VB = CTX->VB;
    GLuint i;
    GLfloat height, oneOverW;

    height = (GLfloat)CTX->Buffer.Height - 1.0f;

    for (i = 0; i < n; i++)
    {
        oneOverW = 1.0f / VB->Clip[vl[i]][3];
        d3dVertct[start+i].c = RGBA_MAKE(255,255,255,VB->Color[vl[i]][3]);
        d3dVertct[start+i].x = VB->Win[vl[i]][0];
        d3dVertct[start+i].y = height - VB->Win[vl[i]][1];
        d3dVertct[start+i].z = za(VB->Win[vl[i]][2]);
        d3dVertct[start+i].w = oneOverW;
        d3dVertct[start+i].s = VB->TexCoord[vl[i]][0];
        d3dVertct[start+i].t = VB->TexCoord[vl[i]][1];
    }
}

static void smooth_setup_xyzw_c_t(GLuint vl[], GLuint start, GLuint n, GLuint pv)
{
    vertex_buffer* VB = CTX->VB;
    GLuint i;
    GLfloat height, oneOverW;

    height = (GLfloat)CTX->Buffer.Height - 1.0f;

    if (CTX->Bias && D3D->canTexAdd)
    {
        for (i = 0; i < n; i++)
        {
            oneOverW = 1.0f / VB->Clip[vl[i]][3];
            d3dVertct[start+i].c = RGBA_MAKE(CTX->ByteBias[0],
                                             CTX->ByteBias[1],
                                             CTX->ByteBias[2],
                                             VB->Color[vl[i]][3]);
            d3dVertct[start+i].x = VB->Win[vl[i]][0];
            d3dVertct[start+i].y = height - VB->Win[vl[i]][1];
            d3dVertct[start+i].z = za(VB->Win[vl[i]][2]);
            d3dVertct[start+i].w = oneOverW;
            d3dVertct[start+i].s = VB->TexCoord[vl[i]][0];
            d3dVertct[start+i].t = VB->TexCoord[vl[i]][1];
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            oneOverW = 1.0f / VB->Clip[vl[i]][3];
            d3dVertct[start+i].c = RGBA_MAKE(VB->Color[vl[i]][0],
                                             VB->Color[vl[i]][1],
                                             VB->Color[vl[i]][2],
                                             VB->Color[vl[i]][3]);
            d3dVertct[start+i].x = VB->Win[vl[i]][0];
            d3dVertct[start+i].y = height - VB->Win[vl[i]][1];
            d3dVertct[start+i].z = za(VB->Win[vl[i]][2]);
            d3dVertct[start+i].w = oneOverW;
            d3dVertct[start+i].s = VB->TexCoord[vl[i]][0];
            d3dVertct[start+i].t = VB->TexCoord[vl[i]][1];
        }
    }
}

static void flat_setup_xyzw_c_t(GLuint vl[], GLuint start, GLuint n, GLuint pv)
{
    vertex_buffer* VB = CTX->VB;
    GLuint i;
    GLfloat height, oneOverW;

    height = (GLfloat)CTX->Buffer.Height - 1.0f;

    DWORD c = RGBA_MAKE(VB->Color[pv][0],
                        VB->Color[pv][1],
                        VB->Color[pv][2],
                        VB->Color[pv][3]);
    for (i = 0; i < n; i++)
    {
        oneOverW = 1.0f / VB->Clip[vl[i]][3];
        d3dVertct[start+i].c = c;
        d3dVertct[start+i].x = VB->Win[vl[i]][0];
        d3dVertct[start+i].y = height - VB->Win[vl[i]][1];
        d3dVertct[start+i].z = za(VB->Win[vl[i]][2]);
        d3dVertct[start+i].w = oneOverW;
        d3dVertct[start+i].s = VB->TexCoord[vl[i]][0];
        d3dVertct[start+i].t = VB->TexCoord[vl[i]][1];
    }
}

//FIXME: 1/w needed when wbuffering
static void setup_xyzw_c(GLuint vl[], GLuint start, GLuint n, GLuint pv)
{
    vertex_buffer* VB = CTX->VB;
    GLuint i;
    GLfloat height;

    height = (GLfloat)CTX->Buffer.Height - 1.0f;

    if (D3D->Primitive == GL_POINTS)
    {
        for (i = 0; i < n; i++)
        {
            d3dVert[start+i].c = RGBA_MAKE(VB->Color[vl[i]][0],
                                           VB->Color[vl[i]][1],
                                           VB->Color[vl[i]][2],
                                           VB->Color[vl[i]][3]);
            d3dVert[start+i].x = VB->Win[vl[i]][0];
            d3dVert[start+i].y = height - VB->Win[vl[i]][1];
            d3dVert[start+i].z = za(VB->Win[vl[i]][2]);
            d3dVert[start+i].w = 1.0f;
        }
        return;
    }

    if (CTX->ShadeModel == GL_SMOOTH)
    {
        for (i = 0; i < n; i++)
        {
            d3dVert[start+i].c = RGBA_MAKE(VB->Color[vl[i]][0],
                                           VB->Color[vl[i]][1],
                                           VB->Color[vl[i]][2],
                                           VB->Color[vl[i]][3]);
            d3dVert[start+i].x = VB->Win[vl[i]][0];
            d3dVert[start+i].y = height - VB->Win[vl[i]][1];
            d3dVert[start+i].z = za(VB->Win[vl[i]][2]);
            d3dVert[start+i].w = 1.0f;
        }
    }
    else
    {
        DWORD c = RGBA_MAKE(VB->Color[pv][0],
                            VB->Color[pv][1],
                            VB->Color[pv][2],
                            VB->Color[pv][3]);
        for (i = 0; i < n; i++)
        {
            d3dVert[start+i].c = c;
            d3dVert[start+i].x = VB->Win[vl[i]][0];
            d3dVert[start+i].y = height - VB->Win[vl[i]][1];
            d3dVert[start+i].z = za(VB->Win[vl[i]][2]);
            d3dVert[start+i].w = 1.0f;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : choose_setup_function
    Description : select a vertex setup fn according to GL state
    Inputs      : ctx - GL context
    Outputs     : global setupFunc is set
    Return      :
----------------------------------------------------------------------------*/
static void choose_setup_function(GLcontext* ctx)
{
    if (ctx->DriverTransforms) return;

    if (ctx->TexEnabled)
    {
        if (ctx->TexEnvMode == GL_REPLACE)
        {
            setupFunc = setup_xyzw_t;
            gVertexFlags = VERTEX_FLAGS;
//            gVertexType = D3DFVF_XYZRHW | D3DFVF_TEX1;
//            gVertices = d3dVertt;
            gVertexType = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
            gVertices = d3dVertct;
        }
        else
        {
            setupFunc = (ctx->ShadeModel == GL_SMOOTH) ? smooth_setup_xyzw_c_t : flat_setup_xyzw_c_t;
            gVertexFlags = VERTEX_FLAGS;
            gVertexType = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
            gVertices = d3dVertct;
        }
    }
    else
    {
        setupFunc = setup_xyzw_c;
        gVertexFlags = VERTEX_FLAGS;
        gVertexType = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
        gVertices = d3dVert;
    }
}

static void vertex0(GLfloat x, GLfloat y, GLfloat z)
{
    vert_t[nVerts].x = x;
    vert_t[nVerts].y = y;
    vert_t[nVerts].z = z;
    vert_t[nVerts].s = CTX->Current.TexCoord[0];
    vert_t[nVerts].t = CTX->Current.TexCoord[1];

    nVerts++;
}

static void vertex1(GLfloat x, GLfloat y, GLfloat z)
{
    vert_c_t[nVerts].x = x;
    vert_c_t[nVerts].y = y;
    vert_c_t[nVerts].z = z;
    vert_c_t[nVerts].c = RGBA_MAKE_ARRAY(CTX->Current.Color);
    vert_c_t[nVerts].s = CTX->Current.TexCoord[0];
    vert_c_t[nVerts].t = CTX->Current.TexCoord[1];

    nVerts++;
}

static void vertex2(GLfloat x, GLfloat y, GLfloat z)
{
    vert_c[nVerts].x = x;
    vert_c[nVerts].y = y;
    vert_c[nVerts].z = z;
    vert_c[nVerts].c = RGBA_MAKE_ARRAY(CTX->Current.Color);

    nVerts++;
}

static void vertex0Q(GLfloat x, GLfloat y, GLfloat z)
{
    GLint vert, modVert;

    vert = nVerts;
    modVert = vert & 3;
    if (modVert == 2) vert++;
    else if (modVert == 3) vert--;

    vert_t[vert].x = x;
    vert_t[vert].y = y;
    vert_t[vert].z = z;
    vert_t[vert].s = CTX->Current.TexCoord[0];
    vert_t[vert].t = CTX->Current.TexCoord[1];

    nVerts++;
}

static void vertex1Q(GLfloat x, GLfloat y, GLfloat z)
{
    GLint vert, modVert;

    vert = nVerts;
    modVert = vert & 3;
    if (modVert == 2) vert++;
    else if (modVert == 3) vert--;

    vert_c_t[vert].x = x;
    vert_c_t[vert].y = y;
    vert_c_t[vert].z = z;
    vert_c_t[vert].c = RGBA_MAKE_ARRAY(CTX->Current.Color);
    vert_c_t[vert].s = CTX->Current.TexCoord[0];
    vert_c_t[vert].t = CTX->Current.TexCoord[1];

    nVerts++;
}

static void vertex2Q(GLfloat x, GLfloat y, GLfloat z)
{
    GLint vert, modVert;

    vert = nVerts;
    modVert = vert & 3;
    if (modVert == 2) vert++;
    else if (modVert == 3) vert--;

    vert_c[vert].x = x;
    vert_c[vert].y = y;
    vert_c[vert].z = z;
    vert_c[vert].c = RGBA_MAKE_ARRAY(CTX->Current.Color);

    nVerts++;
}

static void vertexNULL(GLfloat x, GLfloat y, GLfloat z)
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : begin
    Description : device glBegin(prim) handler
    Inputs      : ctx - GL context
                  primitive - the primitive being begun
    Outputs     : D3D context's Primitive member is set
    Return      :
----------------------------------------------------------------------------*/
typedef void (*vertexFunc)(GLfloat, GLfloat, GLfloat);
vertexFunc vertexProc = NULL;
static void begin(GLcontext* ctx, GLenum primitive)
{
    D3D->Primitive = primitive;
    nVerts = 0;

    if (ctx->DriverTransforms)
    {
        if (ctx->TexEnabled)
        {
            if (ctx->TexEnvMode == GL_REPLACE)
            {
                gVertexType = D3DFVF_XYZ | D3DFVF_TEX1;
                gVertices = vert_t;
                D3D->vertexMode = 0;
                vertexProc = (primitive == GL_QUADS) ? vertex0Q : vertex0;
            }
            else
            {
                gVertexType = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
                gVertices = vert_c_t;
                D3D->vertexMode = 1;
                vertexProc = (primitive == GL_QUADS) ? vertex1Q : vertex1;
            }
        }
        else
        {
            gVertexType = D3DFVF_XYZ | D3DFVF_DIFFUSE;
            gVertices = vert_c;
            D3D->vertexMode = 2;
            vertexProc = (primitive == GL_QUADS) ? vertex2Q : vertex2;
        }
    }
    else
    {
        gVertexNumber = 0;
        vertexProc = vertexNULL;
    }
}

static void vertex(GLfloat x, GLfloat y, GLfloat z)
{
    vertexProc(x, y, z);
}

typedef struct c4ub_v3f_s
{
    GLubyte c[4];
    GLfloat v[3];
} c4ub_v3f;

static void draw_triangle_elements(GLsizei count, GLsizei numVerts, GLvoid const* indices)
{
    DWORD vertexCount, indexCount;
    c4ub_v3f* ptr = (c4ub_v3f*)CTX->VertexArray;

    vertexCount = count / 3;
    indexCount  = count;

    for (GLint i = 0; i < vertexCount; i++, ptr++)
    {
        vert_c[i].x = ptr->v[0];
        vert_c[i].y = ptr->v[1];
        vert_c[i].z = ptr->v[2];
        vert_c[i].c = RGBA_MAKE_ARRAY(ptr->c);
    }

    D3D->d3dDevice->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST,
        D3DFVF_XYZ | D3DFVF_DIFFUSE,
        (LPVOID)vert_c,
        vertexCount,
        (LPWORD)indices,
        indexCount,
        D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
}

static void draw_c4ub_v3f(GLenum mode, GLint first, GLsizei count)
{
    GLint i;
    c4ub_v3f* ptr;

    ptr = (c4ub_v3f*)CTX->VertexArray;
    ptr += first;

    for (i = first; i < count; i++, ptr++)
    {
        vert_c[i].x = ptr->v[0];
        vert_c[i].y = ptr->v[1];
        vert_c[i].z = ptr->v[2];
        vert_c[i].c = RGBA_MAKE_ARRAY(ptr->c);
    }

    D3D->d3dDevice->DrawPrimitive(
        D3DPT_POINTLIST, D3DFVF_XYZ | D3DFVF_DIFFUSE, (LPVOID)vert_c, count, NULL);
}

static void end(GLcontext* ctx)
{
    if (!CTX->DriverTransforms) return;

    if (nVerts == 0) return;

    switch (D3D->Primitive)
    {
    case GL_QUADS:
        D3D->d3dDevice->DrawPrimitive(
            D3DPT_TRIANGLESTRIP, gVertexType, gVertices, nVerts, NULL);
        break;

    case GL_TRIANGLES:
        D3D->d3dDevice->DrawPrimitive(
            D3DPT_TRIANGLELIST, gVertexType, gVertices, nVerts, NULL);
        break;

    case GL_POINTS:
        D3D->d3dDevice->DrawPrimitive(
            D3DPT_POINTLIST, gVertexType, gVertices, nVerts, NULL);
        break;

    case GL_LINES:
        D3D->d3dDevice->DrawPrimitive(
            D3DPT_LINELIST, gVertexType, gVertices, nVerts, NULL);
        break;

    case GL_LINE_LOOP:
        switch (D3D->vertexMode)
        {
        case 0:
            vert_t[nVerts] = vert_t[0];
            break;
        case 1:
            vert_c_t[nVerts] = vert_c_t[0];
            break;
        case 2:
            vert_c[nVerts] = vert_c[0];
            break;
        default:
            ;//FIXME: report error
        }
        nVerts++;
        D3D->d3dDevice->DrawPrimitive(
            D3DPT_LINESTRIP, gVertexType, gVertices, nVerts, NULL);
        break;
    }
}

static void update_modelview(void)
{
    if (!CTX->DriverTransforms) return;
    D3D->d3dDevice->SetTransform(
        D3DTRANSFORMSTATE_WORLD, (LPD3DMATRIX)CTX->ModelViewMatrix);
}

static void update_projection(void)
{
    if (!CTX->DriverTransforms) return;
    D3D->d3dDevice->SetTransform(
        D3DTRANSFORMSTATE_PROJECTION, (LPD3DMATRIX)CTX->ProjectionMatrix);
}

/*-----------------------------------------------------------------------------
    Name        : flush_batch
    Description : renders a batch of primitives coming from draw_[prim] fn
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void flush_batch(void)
{
    if (gVertexNumber > 0)
    {
        D3D->d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, gVertexType, gVertices, gVertexNumber, NULL);
    }
}

/*-----------------------------------------------------------------------------
    Name        : draw_triangle
    Description : draws a triangle
    Inputs      : vl[] - vertex list, pv - dominant vert for monocolor tris
    Outputs     : renders a triangle
    Return      :
----------------------------------------------------------------------------*/
static void draw_triangle(GLuint vl[], GLuint pv)
{
    SETUP(vl, gVertexNumber, 3, pv);
    gVertexNumber += 3;
}

/*-----------------------------------------------------------------------------
    Name        : draw_quad
    Description : draws a quad
    Inputs      : vl[] - vertex list, pv - dominant vert for monocolor quads
    Outputs     : renders a quad
    Return      :
----------------------------------------------------------------------------*/
static void draw_quad(GLuint vl[], GLuint pv)
{
    GLuint vlist[3];

    vlist[0] = vl[0];
    vlist[1] = vl[1];
    vlist[2] = vl[3];
    draw_triangle(vlist, pv);

    vlist[0] = vl[1];
    vlist[1] = vl[2];
    vlist[2] = vl[3];
    draw_triangle(vlist, pv);
}

/*-----------------------------------------------------------------------------
    Name        : draw_line
    Description : draws a line
    Inputs      : vert0 - 1st vertex of the line
                  vert1 - 2nd vertex of the line
                  pv - dominant vertex (for monocolor) of the line
    Outputs     : a line
    Return      :
----------------------------------------------------------------------------*/
static void draw_line(GLuint vert0, GLuint vert1, GLuint pv)
{
    GLuint vl[2];

    vl[0] = vert0;
    vl[1] = vert1;

    SETUP(vl, 0, 2, pv);
    D3D->d3dDevice->DrawPrimitive(D3DPT_LINELIST, gVertexType, gVertices, 2, NULL);
}

//draw_pixel device fn
static void draw_pixel(GLint x, GLint y, GLdepth z)
{
    d3dVert[0].c = D3D->Monocolor;
    d3dVert[0].x = (GLfloat)x;
    d3dVert[0].y = (GLfloat)((CTX->Buffer.Height - 1) - y);
    d3dVert[0].z = 0.5f;
    d3dVert[0].w = 1.0f;
    D3D->d3dDevice->DrawPrimitive(
        D3DPT_POINTLIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, d3dVert, 1, NULL);
}

/*-----------------------------------------------------------------------------
    Name        : draw_point
    Description : draws >= 1 points
    Inputs      : first - starting vert
                  last - final vert
    Outputs     : displays a point(s)
    Return      :
----------------------------------------------------------------------------*/
static void draw_point(GLuint first, GLuint last)
{
    vertex_buffer* VB = CTX->VB;
    GLuint i, n;
    GLuint vl[VB_SIZE];

    for (i = first, n = 0; i <= last; i++)
    {
        if (VB->ClipMask[i] == 0)
        {
            vl[n] = i;
            n++;
        }
    }

    SETUP(vl, 0, n, 0);

    if (CTX->PointSize != 1.0f)
    {
        D3D->d3dDevice->DrawPrimitive(D3DPT_POINTLIST, gVertexType, gVertices, n, NULL);
        for (i = 0; i < n; i++)
        {
            d3dVert[i].x += 1.0f;
        }
        D3D->d3dDevice->DrawPrimitive(D3DPT_POINTLIST, gVertexType, gVertices, n, NULL);
        for (i = 0; i < n; i++)
        {
            d3dVert[i].y += 1.0f;
        }
        D3D->d3dDevice->DrawPrimitive(D3DPT_POINTLIST, gVertexType, gVertices, n, NULL);
        for (i = 0; i < n; i++)
        {
            d3dVert[i].x -= 1.0f;
        }
        D3D->d3dDevice->DrawPrimitive(D3DPT_POINTLIST, gVertexType, gVertices, n, NULL);
    }
    else
    {
        D3D->d3dDevice->DrawPrimitive(D3DPT_POINTLIST, gVertexType, gVertices, n, NULL);
    }
}

static void read_pixels(
    GLcontext* ctx, GLint x, GLint y, GLsizei width, GLsizei height,
    GLenum format, GLenum type)
{
    GLubyte* pb;
    GLubyte* fb;
    GLint pitch;
    DDSURFACEDESC2 ddsd;
    HRESULT hr;
    bool primary;

    //this fn only takes screenshots

    if (width  != ctx->Buffer.Width ||
        height != ctx->Buffer.Height ||
        format != GL_RGB)
    {
        return;
    }

    pb = ctx->Current.Bitmap;

#if IMPLICIT_BEGINS
    d3d_end_scene(D3D);
#endif

    //attempt to lock framebuffer
    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    primary = false;
    hr = D3D->BackSurface->Lock(NULL, &ddsd, 0, NULL);
    if (FAILED(hr))
    {
        errLog("read_pixels(BackSurface->Lock[RW])", hr);
    }
    else
    {
        goto GOOD_SHOT;
    }
    hr = D3D->BackSurface->Lock(NULL, &ddsd, DDLOCK_READONLY, NULL);
    if (FAILED(hr))
    {
        errLog("read_pixels(BackSurface->Lock[RO])", hr);
    }
    else
    {
        goto GOOD_SHOT;
    }

    primary = true;
    hr = D3D->PrimarySurface->Lock(NULL, &ddsd, 0, NULL);
    if (FAILED(hr))
    {
        errLog("read_pixels(PrimarySurface->Lock[RW])", hr);
    }
    else
    {
        goto GOOD_SHOT;
    }
    hr = D3D->PrimarySurface->Lock(NULL, &ddsd, DDLOCK_READONLY, NULL);
    if (FAILED(hr))
    {
        errLog("read_pixels(PrimarySurface->Lock[RO])", hr);
    }
    else
    {
        goto GOOD_SHOT;
    }
    MEMSET(pb, 0, 3*width*height);
#if IMPLICIT_BEGINS
    d3d_begin_scene(D3D);
#endif
    return;

GOOD_SHOT:
    pitch = ddsd.lPitch;
    fb = (GLubyte*)ddsd.lpSurface;

    if (ddsd.dwFlags & DDSD_PIXELFORMAT)
    {
        DDPIXELFORMAT* ddpf = &ddsd.ddpfPixelFormat;

        switch (ddpf->dwRGBBitCount)
        {
        case 16:
            d3d_shot_16bit(pb, fb, width, height, pitch, ddpf);
            break;

        case 24:
            d3d_shot_24bit(pb, fb, width, height, pitch, ddpf);
            break;

        case 32:
            d3d_shot_32bit(pb, fb, width, height, pitch, ddpf);
            break;

        default:
            errLog("read_pixels(bitcount)", D3DERR_TEXTURE_BADSIZE);
            MEMSET(pb, 0, 3*width*height);
        }
    }
    else
    {
        errLog("read_pixels(PIXELFORMAT)", DDERR_INVALIDPIXELFORMAT);
        MEMSET(pb, 0, 3*width*height);
    }

    if (primary)
    {
        D3D->PrimarySurface->Unlock(NULL);
    }
    else
    {
        D3D->BackSurface->Unlock(NULL);
    }

#if IMPLICIT_BEGINS
    d3d_begin_scene(D3D);
#endif
}

//draw_pixels(..) helper, actually draws the pixels
static void d3d_draw_pixels(GLint n)
{
    D3D->d3dDevice->DrawPrimitive(
        D3DPT_POINTLIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, d3dVert, n, NULL);
}

static void draw_pitched_pixels(
    GLint x0, GLint y0, GLint x1, GLint y1,
    GLsizei width, GLsizei height, GLsizei pitch,
    GLvoid const* pixels)
{
    d3d_draw_pixels_RGBA_pitched(x0, y0, x1, y1, width, height, pitch, (GLubyte*)pixels);
}

/*-----------------------------------------------------------------------------
    Name        : draw_pixels
    Description : device's glDrawPixels(..) fn
    Inputs      : ctx - GL context
                  width, height - dimensions of what we're drawing
                  format, type - description of the source surface
    Outputs     : pixels are rendered, very slowly
    Return      :
----------------------------------------------------------------------------*/
static void draw_pixels(
    GLcontext* ctx,
    GLsizei width, GLsizei height,
    GLenum format, GLenum type)
{
#define DP_SIZE 256
    GLint py, px;
    GLint y, x;
    GLubyte* pb;
    GLint pixelcount;
    GLint fbHeight;
    GLubyte r, g, b;

    px = (GLint)ctx->Current.RasterPos[0];
    py = (GLint)ctx->Current.RasterPos[1];
    pb = ctx->Current.Bitmap;
    if (pb == NULL)
    {
        return;
    }

    if (format == GL_RGBA && (width < 256 || width >= 640))
    {
        d3d_draw_pixels_RGBA_generic(px, py, width, height, pb);
        return;
    }

    switch (format)
    {
    case GL_RGB:
    case GL_RGBA:
    case GL_RGB8:
        break;
    default:
        return;
    }

    if (!ctx->DriverTransforms)
    {
        py++;
    }

    fbHeight = ctx->Buffer.Height - 1;

    switch (format)
    {
    case GL_RGB8:
        r = FAST_TO_INT(ctx->Current.RasterColor[0] * 255.0f);
        g = FAST_TO_INT(ctx->Current.RasterColor[1] * 255.0f);
        b = FAST_TO_INT(ctx->Current.RasterColor[2] * 255.0f);

        for (y = 0, pixelcount = 0; y < height; y++)
        {
            for (x = 0; x < width; x++, pb++)
            {
                if (*pb)
                {
                    d3dVert[pixelcount].c = RGBA_MAKE(r, g, b, (*pb) * 16);
                    d3dVert[pixelcount].x = (GLfloat)(px+x);
                    d3dVert[pixelcount].y = (GLfloat)(fbHeight - (py+y));
                    pixelcount++;
                    if (pixelcount == DP_SIZE)
                    {
                        d3d_draw_pixels(pixelcount);
                        pixelcount = 0;
                    }
                }
            }
        }
        break;

    case GL_RGBA:
        for (y = 0, pixelcount = 0; y < height; y++)
        {
            for (x = 0; x < width; x++, pb += 4)
            {
                d3dVert[pixelcount].c = RGBA_MAKE(pb[0], pb[1], pb[2], pb[3]);
                d3dVert[pixelcount].x = (GLfloat)(px+x);
                d3dVert[pixelcount].y = (GLfloat)(fbHeight - (py+y));
                pixelcount++;
                if (pixelcount == DP_SIZE)
                {
                    d3d_draw_pixels(pixelcount);
                    pixelcount = 0;
                }
            }
        }
        break;
    }

    if (pixelcount != 0)
    {
        d3d_draw_pixels(pixelcount);
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_restore
    Description : restore D3D surfaces if they're lost
    Inputs      : d3d - Direct3D context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void d3d_restore(d3d_context* d3d)
{
    if (d3d->PrimarySurface != NULL)
    {
        if (d3d->PrimarySurface->IsLost())
        {
            d3d->PrimarySurface->Restore();
        }
    }

    if (d3d->BackSurface != NULL)
    {
        if (d3d->BackSurface->IsLost())
        {
            d3d->BackSurface->Restore();
        }
    }

    if (d3d->DepthSurface != NULL)
    {
        if (d3d->DepthSurface->IsLost())
        {
            d3d->DepthSurface->Restore();
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : shutdown_driver
    Description : makes call to fns to free textures, release surfaces.
                  deallocates memory for the D3D context
    Inputs      : ctx - GL context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void shutdown_driver(GLcontext* ctx)
{
    d3d_free_all_textures(ctx);
    d3d_shutdown(ctx);

    if (ctx->DriverCtx != NULL)
    {
        delete ctx->DriverCtx;
        ctx->DriverCtx = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : flush
    Description : driver glFlush() handler
    Inputs      :
    Outputs     : last rendered frame is displayer
    Return      :
----------------------------------------------------------------------------*/
static void flush()
{
#if IMPLICIT_BEGINS
    d3d_end_scene(D3D);
#endif
    if (!d3d_flush(D3D))
    {
        d3d_restore(D3D);
    }
#if IMPLICIT_BEGINS
    d3d_begin_scene(D3D);
#endif
}

static void scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    //D3D can't scissor
}

/*-----------------------------------------------------------------------------
    Name        : texpalette
    Description : glColorTableEXT handler
    Inputs      : tex - GL texture object to apply the palette to if not using
                  shared palettes
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void texpalette(gl_texture_object* tex)
{
    d3d_texobj* t3d;

    if (tex == NULL)
    {
        return;
    }

    if (!tex->DriverData)
    {
        tex->DriverData = d3d_alloc_texobj();
        d3d_fill_texobj(tex);
    }

    t3d = (d3d_texobj*)tex->DriverData;

    if (CTX->UsingSharedPalette)
    {
        if (!d3d_modify_shared_palette())
        {
            return;
        }
//        texbind(tex);
    }
    else
    {
        if (tex->Format != GL_COLOR_INDEX)
        {
            return;
        }

        //FIXME: this requires teximg be called before binding a palette,
        //and if the texture changes so must the palette
        if (!t3d->valid)
        {
            return;
        }

        if (!d3d_attach_palette(tex))
        {
            return;
        }
        else
        {
//            texbind(tex);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : texbind
    Description : bind_texture helper
    Inputs      : tex - GL texture object to bind
    Outputs     : D3D will use the new texture if no errors occurred, wrap / filter
                  states are adjusted if necessary
    Return      :
----------------------------------------------------------------------------*/
void texbind(gl_texture_object* tex)
{
    d3d_texobj* t3d;

    if (!tex->DriverData)
    {
        tex->DriverData = d3d_alloc_texobj();
        d3d_fill_texobj(tex);
    }

    t3d = (d3d_texobj*)tex->DriverData;

    if (!t3d->valid)
    {
        return;
    }

    d3d_setup_filter(tex);

    if (!d3d_bind_texture(t3d))
    {
        //FIXME: handle error
    }
}

/*-----------------------------------------------------------------------------
    Name        : bind_texture
    Description : glBindTexture handler.  calls texbind(gl_texture_object*)
    Inputs      :
    Outputs     : tries to get D3D to bind the new texture
    Return      :
----------------------------------------------------------------------------*/
static void bind_texture()
{
    if (CTX->TexBoundObject != NULL)
    {
        texbind(CTX->TexBoundObject);
    }
}

/*-----------------------------------------------------------------------------
    Name        : teximg
    Description : glTexImage2D handler
    Inputs      : tex - GL texture object
                  level - mipmap level, totally ignored
                  internalFormat - also totally ignored
    Outputs     : old texture is deleted if neces, new texture is created,
                  the shared palette is bound if UsingSharedPalette
    Return      :
----------------------------------------------------------------------------*/
typedef void (*FreeFunc_p)(void*);
void teximg(gl_texture_object* tex, GLint level, GLint internalFormat)
{
    d3d_texobj* t3d;

    if (!tex->DriverData)
    {
        tex->DriverData = d3d_alloc_texobj();
        d3d_fill_texobj(tex);
    }

    t3d = (d3d_texobj*)tex->DriverData;

    if (t3d->valid)
    {
        d3d_free_texture(t3d);
    }

    if (d3d_create_texture(tex))
    {
        t3d->valid = GL_TRUE;
#if !EARLY_SHARED_ATTACHMENT
        if (CTX->UsingSharedPalette)
        {
            if (tex->Format == GL_COLOR_INDEX)
            {
                d3d_attach_shared_palette(tex);
            }
        }
#endif
        texbind(tex);

#if 0
        FreeFunc_p FreeFunc = (FreeFunc_p)CTX->FreeFunc;
        FreeFunc(tex->Data);
        tex->Data = NULL;
#endif
    }
    else
    {
        //FIXME: handle error
    }
}

/*-----------------------------------------------------------------------------
    Name        : texparam
    Description : GL texture parameter fn, handles texture wrap / filter states
    Inputs      : pname - the parameter name
                  params - the parameters
    Outputs     : the device is informed of any changes to the current texture states
    Return      :
----------------------------------------------------------------------------*/
static void texparam(GLenum pname, GLfloat const* params)
{
    GLenum param = (GLenum)(GLint)params[0];
    gl_texture_object* tex = CTX->TexBoundObject;
    d3d_texobj* t3d;

    if (!tex)
    {
        return;
    }

    if (!tex->DriverData)
    {
        tex->DriverData = d3d_alloc_texobj();
        d3d_fill_texobj(tex);
    }

    t3d = (d3d_texobj*)tex->DriverData;

    switch (pname)
    {
    case GL_TEXTURE_WRAP_S:
        t3d->texWrapS = param;
        break;

    case GL_TEXTURE_WRAP_T:
        t3d->texWrapT = param;
        break;

    case GL_TEXTURE_MIN_FILTER:
        t3d->texMinFilter = param;
        break;

    case GL_TEXTURE_MAG_FILTER:
        t3d->texMagFilter = param;
    }

    d3d_setup_filter(tex);
}

/*-----------------------------------------------------------------------------
    Name        : texdel
    Description : frees a texture object from the device
    Inputs      : tex - the texture object to free
    Outputs     : texture surfaces are freed, texture driverdata also
    Return      :
----------------------------------------------------------------------------*/
void texdel(gl_texture_object* tex)
{
    d3d_texobj* t3d = (d3d_texobj*)tex->DriverData;

    if (t3d == NULL)
    {
        return;
    }

    d3d_free_texture(t3d);

    delete t3d;
    tex->DriverData = NULL;
}

//this driver doesn't really need this
static void driver_caps(GLcontext* ctx)
{
    ctx->Buffer.Pitch = 2 * ctx->Buffer.Width;
    ctx->Buffer.Depth = ctx->Buffer.Depth;
    ctx->Buffer.PixelType = GL_RGB565;
}

static GLboolean post_init_driver(GLcontext* ctx)
{
    if (!d3d_create_shared_palette())
    {
        return GL_FALSE;
    }

    d3d_load_all_textures(ctx);

    ctx->ScaleDepthValues = GL_FALSE;

    return GL_TRUE;
}

typedef GLboolean (*BoolFunc)(void);
typedef void (*VoidFunc)(void);
typedef void (*BoolParamFunc)(GLboolean);
typedef void (*DrawElemFunc)(GLsizei, GLsizei, GLvoid const*);

static void lock_buffer(GLcontext* ctx)
{
    d3d_begin_scene(D3D);
}

static void unlock_buffer(GLcontext* ctx)
{
    d3d_end_scene(D3D);
}

static int feature_exists(GLint feature)
{
    switch (feature)
    {
    case GL_SHARED_TEXTURE_PALETTE_EXT:
        return (D3D->texCOLORINDEX == NULL) ? GL_FALSE : GL_TRUE;
    case RGL_BROKEN_MIXED_DEPTHTEST:
        return (CTX->D3DReference == 0) ? GL_TRUE : GL_FALSE;
    case RGL_COLOROP_ADD:
        return (D3D->canTexAdd);
    case RGL_D3D_FULLSCENE:
        return D3D->canAntialiasTriIndep;
    default:
        return GL_FALSE;
    }
}

static void d3d_restore_surfaces(void)
{
    D3D->PrimarySurface->Restore();
    D3D->BackSurface->Restore();
    D3D->DepthSurface->Restore();
}

static void activate(void)
{
    HRESULT hr;

    if (!D3D->Fullscreen)
    {
        return;
    }

    for (;;)
    {
        hr = D3D->ddraw4->SetDisplayMode(CTX->Buffer.Width,
                                         CTX->Buffer.Height,
                                         CTX->Buffer.Depth,
                                         0, 0);
        if (!FAILED(hr))
        {
            break;
        }
        if (hr == DDERR_SURFACELOST)
        {
            d3d_restore_surfaces();
            break;
        }
    }
}

static void deactivate(void)
{
    if (!D3D->Fullscreen)
    {
        return;
    }

    (void)D3D->ddraw4->RestoreDisplayMode();
}

extern "C" DLL GLboolean init_driver(GLcontext* ctx)
{
    CTX = ctx;

    /* setup function pointers for the GL */
    ctx->DR.init_driver = (BoolFunc)init_driver;
    ctx->DR.post_init_driver = (BoolFunc)post_init_driver;
    ctx->DR.shutdown_driver = (VoidFunc)shutdown_driver;

    ctx->DR.clear_depthbuffer = (VoidFunc)clear_depthbuffer;
    ctx->DR.clear_colorbuffer = (VoidFunc)clear_colorbuffer;
    ctx->DR.clear_both_buffers = (VoidFunc)clear_both_buffers;

    ctx->DR.allocate_depthbuffer = NULL;
    ctx->DR.allocate_colorbuffer = NULL;

    ctx->DR.setup_triangle = NULL;
    ctx->DR.setup_line = NULL;
    ctx->DR.setup_point = NULL;
    ctx->DR.setup_raster = (VoidFunc)setup_raster;

    ctx->DR.set_monocolor = (VoidFunc)set_monocolor;
    ctx->DR.flush = (VoidFunc)flush;
    ctx->DR.clear_color = NULL;
    ctx->DR.scissor = (VoidFunc)scissor;

    ctx->DriverTransforms = GL_FALSE;
    ctx->DR.begin = (VoidFunc)begin;
    ctx->DR.end = (VoidFunc)end;
    ctx->DR.vertex = vertex;
    ctx->DR.update_modelview = (VoidFunc)update_modelview;
    ctx->DR.update_projection = (VoidFunc)update_projection;
    ctx->DR.draw_c4ub_v3f = draw_c4ub_v3f;

    ctx->DR.flush_batch = (VoidFunc)flush_batch;

    ctx->DR.draw_triangle = (VoidFunc)draw_triangle;
    ctx->DR.draw_triangle_array = NULL;
    ctx->DR.draw_quad = (VoidFunc)draw_quad;
    ctx->DR.draw_triangle_fan = NULL;
    ctx->DR.draw_triangle_strip = NULL;
    ctx->DR.draw_polygon = NULL;
    ctx->DR.draw_line = (VoidFunc)draw_line;
    ctx->DR.draw_bitmap = NULL;
    ctx->DR.draw_pixels = (VoidFunc)draw_pixels;
    ctx->DR.read_pixels = (VoidFunc)read_pixels;
    ctx->DR.draw_pixel = (VoidFunc)draw_pixel;
    ctx->DR.draw_point = (VoidFunc)draw_point;

    ctx->DR.draw_triangle_elements = (DrawElemFunc)draw_triangle_elements;

    ctx->DR.draw_clipped_triangle = NULL;
    ctx->DR.draw_clipped_polygon = NULL;

    ctx->DR.bind_texture = (VoidFunc)bind_texture;
    ctx->DR.tex_param = (VoidFunc)texparam;
    ctx->DR.tex_del = (VoidFunc)texdel;
    ctx->DR.tex_palette = (VoidFunc)texpalette;
    ctx->DR.tex_img = (VoidFunc)teximg;
    ctx->DR.tex_env = NULL;

    ctx->DR.deactivate = deactivate;
    ctx->DR.activate = activate;

    ctx->DR.screenshot = NULL;
    ctx->DR.gamma_up = NULL;
    ctx->DR.gamma_dn = NULL;

    ctx->DR.super_clear = NULL;

    ctx->DR.fastbind_set = NULL;

    ctx->DR.draw_background = NULL;

    ctx->DR.lock_buffer = (VoidFunc)lock_buffer;
    ctx->DR.unlock_buffer = (VoidFunc)unlock_buffer;
    ctx->DR.get_framebuffer = NULL;
    ctx->DR.get_animaticbuffer = NULL;

    ctx->DR.draw_pitched_pixels = draw_pitched_pixels;

    ctx->DR.create_window = NULL;
    ctx->DR.delete_window = NULL;
    ctx->DR.set_save_state = NULL;
    ctx->DR.get_scratch = NULL;

    ctx->DR.fullscene = (BoolParamFunc)d3d_fullscene;

    ctx->DR.driver_caps = (VoidFunc)driver_caps;

#if IMPLICIT_BEGINS
    ctx->RequireLocking = GL_FALSE;
#else
    ctx->RequireLocking = GL_TRUE;
#endif

    ctx->DR.fog_vertices = NULL;

    ctx->DR.feature_exists = feature_exists;

    /* allocate memory for ctx->DriverCtx */
    ctx->DriverCtx = new d3d_context;
    D3D = (d3d_context*)ctx->DriverCtx;
    ZeroMemory(D3D, sizeof(d3d_context));

    D3D->inScene = GL_FALSE;

    errLog("init_driver(startup)", D3D_OK);

    switch (ctx->D3DReference)
    {
    case 0:
        D3D->d3dDeviceGUID = PRIMARY_GUID;
        rglSetRendererString((PRIMARY_GUID == HAL_GUID) ? "HAL" : "RGB");
        break;
    case 1:
        D3D->d3dDeviceGUID = RGB_GUID;
        rglSetRendererString("RGB");
        break;
    case 2:
        D3D->d3dDeviceGUID = REF_GUID;
        rglSetRendererString("REF");
    }

    if (!initialize_directx(ctx))
    {
        d3d_shutdown(ctx);
        return GL_FALSE;
    }

    init_raster(ctx);
#if IMPLICIT_BEGINS
    if (!d3d_begin_scene(D3D))
    {
        d3d_shutdown(ctx);
        return GL_FALSE;
    }
#else
    ctx->RequireLocking = GL_TRUE;
#endif

    rglSetExtensionString("direct3d");

    return GL_TRUE;
}
