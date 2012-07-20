/*=============================================================================
    Name    : fxdriver.c
    Purpose : 3Dfx driver (rasterization, state) for rgl

    Created 1/2/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "kgl.h"
#include "fxdrv.h"

#define SIMPLE_TMU_USAGE    1
#define WRAP_TESTS          1

#if SIMPLE_TMU_USAGE
static FxU32 FX_TMU;
#endif

static void draw_background(GLubyte* pixels);

#define ADJUST_UPPER_LEFT(n) (_heightMinusOne - (n) - 1)

#define RUB(x) FAST_TO_INT(x * 255.0f)

#define onScreen(x,y) \
    (((x) < 0 || (y) < 0 || (x) > _widthMinusOne || (y) > _heightMinusOne) ? GL_FALSE : GL_TRUE)

#define SCISSORING

static GLushort* fbRAM = NULL;

GLboolean _lfbAccess = GL_TRUE;
GLboolean _linelfb = GL_FALSE;
GLboolean _pointlfb = GL_TRUE;

GLboolean _chromakey = GL_FALSE;

static GLint _widthMinusOne, _heightMinusOne;

//required by FAST_TO_INT
double chop_temp;

//ctx->DR.* instead of ctx->DriverFuncs.*
#define DR DriverFuncs

#define DEPTH_MULT 65535.0f*DEPTH_SCALE

FxI32 TEXTURE_ALIGN = 16;
FxI32 TEXTURE_ALIGNMASK = 0xFFFFFFF0;

static GLboolean _wbuffer = GL_TRUE;
static GLboolean _alphaUp;

static GLboolean glideInitialized = GL_FALSE;
static int _3dfxPresent = 0;
static GrHwConfiguration hwconfig;

static GLcontext* FXctx;
static fx_context* fx;

#define DEFAULT_GAMMA 1.15f
static GLfloat _gamma;

//used by choose_setup_function
#define GOURAUD_ENABLED 0x1
#define TEXTURE_ENABLED 0x2
#define FOG_ENABLED     0x4
#define WBUFFER_ENABLED 0x8
#define ZBUFFER_ENABLED 0x10

#define FXCOLOR(r,g,b,a) \
    ((((unsigned int)(a))<<24)|(((unsigned int)(b))<<16)|(((unsigned int)(g))<<8)|(r))

#define FXCOLOR565(r,g,b) \
    (FxU16)(((FxU16)0xf8 & r) << (11-3)) | \
           (((FxU16)0xfc & g) << (5-3+1)) | \
           (((FxU16)0xf8 & b) >> 3)

/*
 * function prototypes.  those that aren't static are called from fxtmg.c
 */
static void bind_texture();
void texbind(gl_texture_object* tex);
static void texparam(GLenum pname, GLfloat const* params);
static void texdel(gl_texture_object* tex);
void texpalette(gl_texture_object* tex);
void teximg(gl_texture_object* tex, GLint level, GLint internalFormat);
static void scissor(GLint x, GLint y, GLsizei width, GLsizei height);
static GLint inWhichTMU();

//log func
static FILE* flog = NULL;
void _fxLog(char* s)
{
    if (flog == NULL)
    {
        flog = fopen("fxlog.dat", "wt");
        if (flog == NULL)
            return;
    }
    fprintf(flog, s);
}

/*=============================================================================
    GrVertex setup code
=============================================================================*/

static unsigned short CW, oldCW;

#if defined (_MSC_VER)

#define loFPU() \
    __asm { \
        __asm fstcw [oldCW] \
        __asm mov ax, [oldCW] \
        __asm and eax, 0xFFFFFCFF \
        __asm mov [CW], ax \
        __asm fldcw [CW] \
    }

#define restoreFPU() \
    __asm { \
        __asm fldcw [oldCW] \
    }

#elif defined (__GNUC__) && defined (__i386__)

#define loFPU() \
    __asm__ __volatile__ (            \
        "fstcw %0\n\t"                \
        "movw %0, %%ax\n\t"           \
        "andl $0xFFFFFCFF, %%eax\n\t" \
        "movw %%ax, %1\n\t"           \
        "fldcw %1\n\t"                \
        : "=m" (oldCW)                \
        : "m" (CW)                    \
        : "eax" );

#define restoreFPU() \
    __asm__ __volatile__ ( \
        "fldcw %0\n\t"     \
        :                  \
        : "m" (oldCW) );

#else
#error Inline assembly requires x86 platform.
#endif

#define SNAPPER (float)(3L << 18)
//intel-specific snapper, relies on loFPU
#define SNAP(v) \
    { \
        (v)->x = VB->Win[i][0] + SNAPPER; \
        (v)->x -= SNAPPER; \
        (v)->y = VB->Win[i][1] + SNAPPER; \
        (v)->y -= SNAPPER; \
    }

#define NOSNAP(v) \
    { \
        (v)->x = VB->Win[i][0]; \
        (v)->y = VB->Win[i][1]; \
    }

#define GOURAUD(v) \
    { \
        (v)->r = (float)(VB->Color[i][0]); \
        (v)->g = (float)(VB->Color[i][1]); \
        (v)->b = (float)(VB->Color[i][2]); \
        (v)->a = (float)(VB->Color[i][3]); \
    }

#define WBUFFER(v) { (v)->oow = wscale / VB->Clip[i][3]; }

#define ZBUFFER(v) { (v)->ooz = DEPTH_MULT/VB->Win[i][2]; }

#define TEXTURE(v) \
    { \
        (v)->tmuvtx[0].sow = sscale*VB->TexCoord[i][0]*(v)->oow; \
        (v)->tmuvtx[0].tow = tscale*VB->TexCoord[i][1]*(v)->oow; \
    }

#define NOP(v)

//generic rasterization setup code
#define SETUP(snap, gouraud, texture, wdepth, zdepth) \
    { \
        unsigned int i; \
        vertex_buffer* VB = ctx->VB; \
        GrVertex* GVB = &fx->gwin[vstart]; \
        float wscale = fx->wscale; \
        float sscale; \
        float tscale; \
        \
        if (ctx->TexBoundObject && ctx->TexBoundObject->DriverData) \
        { \
            sscale = ((fx_texobj*)(ctx->TexBoundObject->DriverData))->sscale; \
            tscale = ((fx_texobj*)(ctx->TexBoundObject->DriverData))->tscale; \
        } \
        \
        for (i = vstart; i <= vend; i++, GVB++) \
        { \
            snap(GVB); \
            gouraud(GVB); \
            wdepth(GVB); \
            zdepth(GVB); \
            texture(GVB); \
        } \
    }

/*=============================================================================
    rasterization setup functions
=============================================================================*/

static void setup(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, NOP, NOP, NOP, NOP);
        break;
    default:
        SETUP(SNAP, NOP, NOP, NOP, NOP);
    }
}

static void setupG(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, GOURAUD, NOP, NOP, NOP);
        break;
    default:
        SETUP(SNAP, GOURAUD, NOP, NOP, NOP);
    }
}

static void setupT(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, NOP, TEXTURE, WBUFFER, NOP);
        break;
    default:
        SETUP(SNAP, NOP, TEXTURE, WBUFFER, NOP);
    }
}

static void setupGT(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, GOURAUD, TEXTURE, WBUFFER, NOP);
        break;
    default:
        SETUP(SNAP, GOURAUD, TEXTURE, WBUFFER, NOP);
    }
}

static void setupW(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, NOP, NOP, WBUFFER, NOP);
        break;
    default:
        SETUP(SNAP, NOP, NOP, WBUFFER, NOP);
    }
}

static void setupGW(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, GOURAUD, NOP, WBUFFER, NOP);
        break;
    default:
        SETUP(SNAP, GOURAUD, NOP, WBUFFER, NOP);
    }
}

static void setupZ(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, NOP, NOP, NOP, ZBUFFER);
        break;
    default:
        SETUP(SNAP, NOP, NOP, NOP, ZBUFFER);
    }
}

static void setupGZ(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, GOURAUD, NOP, NOP, ZBUFFER);
        break;
    default:
        SETUP(SNAP, GOURAUD, NOP, NOP, ZBUFFER);
    }
}

static void setupTZ(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, NOP, TEXTURE, WBUFFER, ZBUFFER);
        break;
    default:
        SETUP(SNAP, NOP, TEXTURE, WBUFFER, ZBUFFER);
    }
}

static void setupGTZ(GLcontext* ctx, GLuint vstart, GLuint vend)
{
    switch (ctx->Primitive)
    {
    case GL_POINTS:
        SETUP(NOSNAP, GOURAUD, TEXTURE, WBUFFER, ZBUFFER);
        break;
    default:
        SETUP(SNAP, GOURAUD, TEXTURE, WBUFFER, ZBUFFER);
    }
}

setup_func fxSetupFuncs[] =
{
    setup,
    setupG,
    setupT,
    setupGT,
    setupW,
    setupGW,
    setupT,
    setupGT,
    setupW,
    setupGW,
    setupT,
    setupGT,
    setupW,
    setupGW,
    setupT,
    setupGT,
    setupZ,
    setupGZ,
    setupTZ,
    setupGTZ,
    setupZ,
    setupGZ,
    setupTZ,
    setupGTZ
};

/*-----------------------------------------------------------------------------
    Name        : choose_setup_function
    Description : chooses a rasterization setup function based on the
                  current state
    Inputs      : ctx - the GL's context
    Outputs     :
    Return      : setup_func, a function pointer that expects arguments
                  (GLcontext* ctx, GLuint vstart, GLuint vend)
----------------------------------------------------------------------------*/
setup_func choose_setup_function(GLcontext* ctx)
{
    unsigned int setupIndex = 0;

    if (ctx->ShadeModel == GL_SMOOTH)
        setupIndex |= GOURAUD_ENABLED;
    if (ctx->TexEnabled)
        setupIndex |= TEXTURE_ENABLED;
    if (ctx->DepthTest)
    {
        if (_wbuffer)
            setupIndex |= WBUFFER_ENABLED;
        else
            setupIndex |= ZBUFFER_ENABLED;
    }
    return fxSetupFuncs[setupIndex];
}

/*=============================================================================
    driver functions
=============================================================================*/

static GLint fxScissorA;
static GLint fxScissorB;
static GLint fxScissorC;
static GLint fxScissorD;

static void clear_last_raster()
{
    int i;

    fx->scissor[0] = -1;
    fx->scissor[1] = -1;
    fx->scissor[2] = -1;
    fx->scissor[3] = -1;
    fx->grAlphaBlendFunction[0] = 0xff;
    fx->grAlphaBlendFunction[1] = 0xff;
    fx->grAlphaBlendFunction[2] = 0xff;
    fx->grAlphaBlendFunction[3] = 0xff;
    fx->grColorMask[0] = 2;
    fx->grColorMask[1] = 2;
    fx->grAlphaTestFunction = 0xff;
    fx->valid_grAlphaTestReferenceValue = FXFALSE;
    fx->grDepthBufferMode = 0xff;
    fx->grDepthBufferFunction = 0xff;
    fx->grDepthMask = 2;
    fx->grFogMode = 0xff;
    fx->grChromakeyMode = 0xff;
    fx->valid_grChromakeyValue = FXFALSE;
    fx->valid_grConstantColorValue = FXFALSE;
    for (i = GR_TMU0; i < GR_TMU2; i++)
    {
        fx->s_grTexClampMode[i] = 0xff;
        fx->t_grTexClampMode[i] = 0xff;
        fx->min_grTexFilterMode[i] = 0xff;
        fx->mag_grTexFilterMode[i] = 0xff;
        fx->grTexMipMapMode[i] = 0xff;
        fx->blend_grTexMipMapMode[i] = 2;
        fx->rgb_function_grTexCombine[i] = 0xff;
        fx->rgb_factor_grTexCombine[i] = 0xff;
        fx->alpha_function_grTexCombine[i] = 0xff;
        fx->alpha_factor_grTexCombine[i] = 0xff;
        fx->rgb_invert_grTexCombine[i] = 0xff;
        fx->alpha_invert_grTexCombine[i] = 0xff;
    }
    fx->function_grColorCombine = 0xff;
    fx->factor_grColorCombine = 0xff;
    fx->local_grColorCombine = 0xff;
    fx->other_grColorCombine = 0xff;
    fx->invert_grColorCombine = 2;
    fx->function_grAlphaCombine = 0xff;
    fx->factor_grAlphaCombine = 0xff;
    fx->local_grAlphaCombine = 0xff;
    fx->other_grAlphaCombine = 0xff;
    fx->invert_grAlphaCombine = 2;
}

static void wrap_grClipWindow(GLint a, GLint b, GLint c, GLint d)
{
#if WRAP_TESTS
    if (fx->scissor[0] != a ||
        fx->scissor[1] != b ||
        fx->scissor[2] != c ||
        fx->scissor[3] != d)
#endif
    {
        fx->scissor[0] = a;
        fx->scissor[1] = b;
        fx->scissor[2] = c;
        fx->scissor[3] = d;
        grClipWindow(a, b, c, d);
    }
}

static void wrap_grAlphaBlendFunction(
    GrAlphaBlendFnc_t a, GrAlphaBlendFnc_t b, GrAlphaBlendFnc_t c, GrAlphaBlendFnc_t d)
{
#if WRAP_TESTS
    if (fx->grAlphaBlendFunction[0] != a ||
        fx->grAlphaBlendFunction[1] != b ||
        fx->grAlphaBlendFunction[2] != c ||
        fx->grAlphaBlendFunction[3] != d)
#endif
    {
        grAlphaBlendFunction(a, b, c, d);
        fx->grAlphaBlendFunction[0] = a;
        fx->grAlphaBlendFunction[1] = b;
        fx->grAlphaBlendFunction[2] = c;
        fx->grAlphaBlendFunction[3] = d;
    }
}

static void wrap_grColorMask(FxBool a, FxBool b)
{
#if WRAP_TESTS
    if (fx->grColorMask[0] != a ||
        fx->grColorMask[1] != b)
#endif
    {
        grColorMask(a, b);
        fx->grColorMask[0] = a;
        fx->grColorMask[1] = b;
    }
}

static void wrap_grAlphaTestFunction(GrCmpFnc_t a)
{
#if WRAP_TESTS
    if (fx->grAlphaTestFunction != a)
#endif
    {
        grAlphaTestFunction(a);
        fx->grAlphaTestFunction = a;
    }
}

static void wrap_grAlphaTestReferenceValue(GrAlpha_t a)
{
#if WRAP_TESTS
    if (!fx->valid_grAlphaTestReferenceValue ||
        fx->grAlphaTestReferenceValue != a)
#endif
    {
        grAlphaTestReferenceValue(a);
        fx->grAlphaTestReferenceValue = a;
        fx->valid_grAlphaTestReferenceValue = FXTRUE;
    }
}

static void wrap_grDepthBufferMode(GrDepthBufferMode_t mode)
{
#if WRAP_TESTS
    if (fx->grDepthBufferMode != mode)
#endif
    {
        grDepthBufferMode(mode);
        fx->grDepthBufferMode = mode;
    }
}

static void wrap_grDepthBufferFunction(GrCmpFnc_t fnc)
{
#if WRAP_TESTS
    if (fx->grDepthBufferFunction != fnc)
#endif
    {
        grDepthBufferFunction(fnc);
        fx->grDepthBufferFunction = fnc;
    }
}

static void wrap_grDepthMask(FxBool mask)
{
#if WRAP_TESTS
    if (fx->grDepthMask != mask)
#endif
    {
        grDepthMask(mask);
        fx->grDepthMask = mask;
    }
}

static void wrap_grFogMode(GrFogMode_t mode)
{
#if WRAP_TESTS
    if (fx->grFogMode != mode)
#endif
    {
        grFogMode(mode);
        fx->grFogMode = mode;
    }
}

static void wrap_grChromakeyMode(GrChromakeyMode_t mode)
{
#if WRAP_TESTS
    if (fx->grChromakeyMode != mode)
#endif
    {
        grChromakeyMode(mode);
        fx->grChromakeyMode = mode;
    }
}

static void wrap_grChromakeyValue(GrColor_t c)
{
#if WRAP_TESTS
    if (!fx->valid_grChromakeyValue ||
        fx->grChromakeyValue != c)
#endif
    {
        grChromakeyValue(c);
        fx->grChromakeyValue = c;
        fx->valid_grChromakeyValue = FXTRUE;
    }
}

static void wrap_grConstantColorValue(GrColor_t c)
{
#if WRAP_TESTS
    if (!fx->valid_grConstantColorValue ||
        fx->grConstantColorValue != c)
#endif
    {
        grConstantColorValue(c);
        fx->grConstantColorValue = c;
        fx->valid_grConstantColorValue = FXTRUE;
    }
}

static void wrap_grTexClampMode(
    GrChipID_t chip, GrTextureClampMode_t mode_s, GrTextureClampMode_t mode_t)
{
#if WRAP_TESTS
    if (fx->s_grTexClampMode[chip] != mode_s ||
        fx->t_grTexClampMode[chip] != mode_t)
#endif
    {
        grTexClampMode(chip, mode_s, mode_t);
        fx->s_grTexClampMode[chip] = mode_s;
        fx->t_grTexClampMode[chip] = mode_t;
    }
}

static void wrap_grTexFilterMode(
    GrChipID_t chip, GrTextureFilterMode_t min, GrTextureFilterMode_t mag)
{
#if WRAP_TESTS
    if (fx->min_grTexFilterMode[chip] != min ||
        fx->mag_grTexFilterMode[chip] != mag)
#endif
    {
        grTexFilterMode(chip, min, mag);
        fx->min_grTexFilterMode[chip] = min;
        fx->mag_grTexFilterMode[chip] = mag;
    }
}

static void wrap_grTexMipMapMode(
    GrChipID_t chip, GrMipMapMode_t mode, FxBool blend)
{
#if WRAP_TESTS
    if (fx->grTexMipMapMode[chip] != mode ||
        fx->blend_grTexMipMapMode[chip] != blend)
#endif
    {
        grTexMipMapMode(chip, mode, blend);
        fx->grTexMipMapMode[chip] = mode;
        fx->blend_grTexMipMapMode[chip] = blend;
    }
}

static void wrap_grColorCombine(
    GrCombineFunction_t function, GrCombineFactor_t factor,
    GrCombineLocal_t local, GrCombineOther_t other, FxBool invert)
{
#if WRAP_TESTS
    if (fx->function_grColorCombine != function ||
        fx->factor_grColorCombine != factor ||
        fx->local_grColorCombine != local ||
        fx->other_grColorCombine != other ||
        fx->invert_grColorCombine != invert)
#endif
    {
        grColorCombine(function, factor, local, other, invert);
        fx->function_grColorCombine = function;
        fx->factor_grColorCombine = factor;
        fx->local_grColorCombine = local;
        fx->other_grColorCombine = other;
        fx->invert_grColorCombine = invert;
    }
}

static void wrap_grAlphaCombine(
    GrCombineFunction_t function, GrCombineFactor_t factor,
    GrCombineLocal_t local, GrCombineOther_t other, FxBool invert)
{
#if WRAP_TESTS
    if (fx->function_grAlphaCombine != function ||
        fx->factor_grAlphaCombine != factor ||
        fx->local_grAlphaCombine != local ||
        fx->other_grAlphaCombine != other ||
        fx->invert_grAlphaCombine != invert)
#endif
    {
        grAlphaCombine(function, factor, local, other, invert);
        fx->function_grAlphaCombine = function;
        fx->factor_grAlphaCombine = factor;
        fx->local_grAlphaCombine = local;
        fx->other_grAlphaCombine = other;
        fx->invert_grAlphaCombine = invert;
    }
}

static void wrap_grTexCombine(
    GrChipID_t chip,
    GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor,
    GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor,
    FxBool rgb_invert, FxBool alpha_invert)
{
#if WRAP_TESTS
    if (fx->rgb_function_grTexCombine[chip] != rgb_function ||
        fx->rgb_factor_grTexCombine[chip] != rgb_factor ||
        fx->alpha_function_grTexCombine[chip] != alpha_function ||
        fx->alpha_factor_grTexCombine[chip] != alpha_factor ||
        fx->rgb_invert_grTexCombine[chip] != rgb_invert ||
        fx->alpha_invert_grTexCombine[chip] != alpha_invert)
#endif
    {
        grTexCombine(chip, rgb_function, rgb_factor,
                     alpha_function, alpha_factor,
                     rgb_invert, alpha_invert);
        fx->rgb_function_grTexCombine[chip] = rgb_function;
        fx->rgb_factor_grTexCombine[chip] = rgb_factor;
        fx->alpha_function_grTexCombine[chip] = alpha_function;
        fx->alpha_factor_grTexCombine[chip] = alpha_factor;
        fx->rgb_invert_grTexCombine[chip] = rgb_invert;
        fx->alpha_invert_grTexCombine[chip] = alpha_invert;
    }
}

static void setup_scissor()
{
#ifdef SCISSORING
    GLcontext* ctx = FXctx;

    if (ctx->ScissorTest && fx->ScissorX != 0)
    {
        fxScissorA = fx->ScissorX - 1;
        fxScissorB = fx->ScissorY;
        fxScissorC = fx->ScissorWidth + fx->ScissorX;
        fxScissorD = fx->ScissorHeight + fx->ScissorY;
    }
    else if (ctx->ScissorTest)
    {
        fxScissorA = 0;
        fxScissorB = 60;
        fxScissorC = ctx->Buffer.Width;
        fxScissorD = ctx->Buffer.Height - 60;
    }
    else
    {
        fxScissorA = 0;
        fxScissorB = 0;
        fxScissorC = ctx->Buffer.Width;
        fxScissorD = ctx->Buffer.Height;
    }
    wrap_grClipWindow(fxScissorA, fxScissorB, fxScissorC, fxScissorD);
#endif
}

void setup_blend()
{
    GLcontext* ctx = FXctx;
    FxBool colorMask = ctx->ColorWrite ? FXTRUE : FXFALSE;

    if (ctx->Blend)
    {
        if (ctx->LineSmooth)
        {
            wrap_grAlphaCombine(
                        GR_COMBINE_FUNCTION_LOCAL_ALPHA,
                        GR_COMBINE_FACTOR_ONE,
                        GR_COMBINE_LOCAL_ITERATED,
                        GR_COMBINE_OTHER_NONE,
                        FXFALSE);

            wrap_grAlphaBlendFunction(
                GR_BLEND_SRC_ALPHA,
                GR_BLEND_ONE_MINUS_SRC_ALPHA,
                GR_BLEND_ONE,
                GR_BLEND_ZERO);
            wrap_grColorMask(colorMask, FXFALSE);
        }
        else
        {
            if (ctx->ShadeModel == GL_SMOOTH)
            {
                wrap_grAlphaCombine(
                            GR_COMBINE_FUNCTION_LOCAL_ALPHA,
                            GR_COMBINE_FACTOR_ONE,
                            GR_COMBINE_LOCAL_ITERATED,
                            GR_COMBINE_OTHER_NONE,
                            FXFALSE);
            }
            else
            {
                wrap_grAlphaCombine(
                            GR_COMBINE_FUNCTION_LOCAL_ALPHA,
                            GR_COMBINE_FACTOR_ONE,
                            GR_COMBINE_LOCAL_CONSTANT,
                            GR_COMBINE_OTHER_NONE,
                            FXFALSE);
            }

            switch (ctx->BlendDst)
            {
            case GL_ONE:
                wrap_grAlphaBlendFunction(
                    GR_BLEND_SRC_ALPHA,
                    GR_BLEND_ONE,
                    GR_BLEND_ONE,
                    GR_BLEND_ZERO);
                break;
            default:
                wrap_grAlphaBlendFunction(
                    GR_BLEND_SRC_ALPHA,
                    GR_BLEND_ONE_MINUS_SRC_ALPHA,
                    GR_BLEND_ONE,
                    GR_BLEND_ZERO);
            }

            wrap_grColorMask(colorMask, FXFALSE);
        }
    }
    else
    {
        if (ctx->ShadeModel == GL_SMOOTH)
        {
            wrap_grAlphaCombine(
                        GR_COMBINE_FUNCTION_LOCAL_ALPHA,
                        GR_COMBINE_FACTOR_ONE,
                        GR_COMBINE_LOCAL_ITERATED,
                        GR_COMBINE_OTHER_NONE,
                        FXFALSE);
        }
        else
        {
            wrap_grAlphaCombine(
                        GR_COMBINE_FUNCTION_LOCAL_ALPHA,
                        GR_COMBINE_FACTOR_ONE,
                        GR_COMBINE_LOCAL_CONSTANT,
                        GR_COMBINE_OTHER_NONE,
                        FXFALSE);
        }

        wrap_grAlphaBlendFunction(
            GR_BLEND_ONE,
            GR_BLEND_ZERO,
            GR_BLEND_ONE,
            GR_BLEND_ZERO);
        wrap_grColorMask(colorMask, FXFALSE);
    }
}

void setup_alphatest()
{
    GLcontext* ctx = FXctx;

    if (ctx->AlphaTest)
    {
        if (ctx->ShadeModel == GL_SMOOTH)
        {
            wrap_grAlphaCombine(
                        GR_COMBINE_FUNCTION_LOCAL_ALPHA,
                        GR_COMBINE_FACTOR_ONE,
                        GR_COMBINE_LOCAL_ITERATED,
                        GR_COMBINE_OTHER_NONE,
                        FXFALSE);
        }
        else
        {
            wrap_grAlphaCombine(
                        GR_COMBINE_FUNCTION_LOCAL_ALPHA,
                        GR_COMBINE_FACTOR_ONE,
                        GR_COMBINE_LOCAL_CONSTANT,
                        GR_COMBINE_OTHER_NONE,
                        FXFALSE);
        }

        switch (ctx->AlphaFunc)
        {
        case GL_NEVER:
            wrap_grAlphaTestFunction(GR_CMP_NEVER);
            break;
        case GL_LESS:
            wrap_grAlphaTestFunction(GR_CMP_LESS);
            break;
        case GL_EQUAL:
            wrap_grAlphaTestFunction(GR_CMP_EQUAL);
            break;
        case GL_LEQUAL:
            wrap_grAlphaTestFunction(GR_CMP_LEQUAL);
            break;
        case GL_GREATER:
            wrap_grAlphaTestFunction(GR_CMP_GREATER);
            break;
        case GL_NOTEQUAL:
            wrap_grAlphaTestFunction(GR_CMP_NOTEQUAL);
            break;
        case GL_GEQUAL:
            wrap_grAlphaTestFunction(GR_CMP_GEQUAL);
            break;
        case GL_ALWAYS:
            wrap_grAlphaTestFunction(GR_CMP_ALWAYS);
            break;
        }
        wrap_grAlphaTestReferenceValue(ctx->AlphaByteRef);
    }
    else
    {
        wrap_grAlphaTestFunction(GR_CMP_ALWAYS);
    }
}

static void setup_texture(FxU32 where)
{
    GLcontext* ctx = FXctx;
    GrCombineLocal_t localc, locala;
    gl_texture_object* tex;

    if (ctx->ShadeModel == GL_SMOOTH)
    {
        localc = GR_COMBINE_LOCAL_ITERATED;
        locala = GR_COMBINE_LOCAL_ITERATED;
    }
    else
    {
        localc = GR_COMBINE_LOCAL_CONSTANT;
        locala = GR_COMBINE_LOCAL_CONSTANT;
    }

    tex = ctx->TexBoundObject;

    if (!ctx->TexEnabled || (tex == NULL) || ((GLint)where == -1))
    {
        if (ctx->ShadeModel == GL_SMOOTH)
        {
            wrap_grColorCombine(
                           GR_COMBINE_FUNCTION_SCALE_OTHER,
                           GR_COMBINE_FACTOR_ONE,
                           GR_COMBINE_LOCAL_NONE,
                           GR_COMBINE_OTHER_ITERATED,
                           FXFALSE);
        }
        else
        {
            wrap_grColorCombine(
                           GR_COMBINE_FUNCTION_SCALE_OTHER,
                           GR_COMBINE_FACTOR_ONE,
                           GR_COMBINE_LOCAL_NONE,
                           GR_COMBINE_OTHER_CONSTANT,
                           FXFALSE);
        }
        return;
    }

    switch (ctx->TexEnvMode)
    {
    case GL_MODULATE:
        if (fx->bias)
        {
            if (ctx->ShadeModel == GL_FLAT)
            {
                _alphaUp = FXTRUE;
            }
            wrap_grAlphaCombine(
                GR_COMBINE_FUNCTION_SCALE_OTHER,
                GR_COMBINE_FACTOR_LOCAL,
                GR_COMBINE_LOCAL_ITERATED,
                GR_COMBINE_OTHER_TEXTURE,
                FXFALSE);
            wrap_grColorCombine(
                GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
                GR_COMBINE_FACTOR_ONE,
                GR_COMBINE_LOCAL_ITERATED,
                GR_COMBINE_OTHER_TEXTURE,
                FXFALSE);
        }
        else
        {
            wrap_grAlphaCombine(
                GR_COMBINE_FUNCTION_SCALE_OTHER,
                GR_COMBINE_FACTOR_LOCAL,
                locala,
                GR_COMBINE_OTHER_TEXTURE,
                FXFALSE);
            wrap_grColorCombine(
                GR_COMBINE_FUNCTION_SCALE_OTHER,
                GR_COMBINE_FACTOR_LOCAL,
                localc,
                GR_COMBINE_OTHER_TEXTURE,
                FXFALSE);
        }
        break;

    case GL_DECAL:
        wrap_grAlphaCombine(
            GR_COMBINE_FUNCTION_LOCAL,
            GR_COMBINE_FACTOR_NONE,
            locala,
            GR_COMBINE_OTHER_NONE,
            FXFALSE);
        wrap_grColorCombine(
            GR_COMBINE_FUNCTION_BLEND,
            GR_COMBINE_FACTOR_TEXTURE_ALPHA,
            localc,
            GR_COMBINE_OTHER_TEXTURE,
            FXFALSE);
        break;

    case GL_REPLACE:
        if (ctx->Blend || ctx->AlphaTest)
        {
            if (ctx->LightingAdjust != 0.0f)
            {
                fx->alphaColor = (GLint)((1.0f - ctx->LightingAdjust) * 255.0f);
                wrap_grAlphaCombine(
                               GR_COMBINE_FUNCTION_SCALE_OTHER,
                               GR_COMBINE_FACTOR_LOCAL_ALPHA,
                               GR_COMBINE_LOCAL_ITERATED,
                               GR_COMBINE_OTHER_TEXTURE,
                               FXFALSE);
                _alphaUp = FXTRUE;
            }
            else
            {
                wrap_grAlphaCombine(
                               GR_COMBINE_FUNCTION_SCALE_OTHER,
                               GR_COMBINE_FACTOR_ONE,
                               GR_COMBINE_LOCAL_NONE,
                               GR_COMBINE_OTHER_TEXTURE,
                               FXFALSE);
            }
        }
        if (fx->bias)
        {
            wrap_grColorCombine(
                GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
                GR_COMBINE_FACTOR_ONE,
                GR_COMBINE_LOCAL_ITERATED,
                GR_COMBINE_OTHER_TEXTURE,
                FXFALSE);
        }
        else
        {
            wrap_grColorCombine(
                GR_COMBINE_FUNCTION_SCALE_OTHER,
                GR_COMBINE_FACTOR_ONE,
                localc,
                GR_COMBINE_OTHER_TEXTURE,
                FXFALSE);
        }
        break;
    }
}

static void setup_tmu(GLint where)
{
    if (where == 0)
    {
        wrap_grTexCombine(FX_TMU0,
                          GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                          GR_COMBINE_FUNCTION_LOCAL_ALPHA, GR_COMBINE_FACTOR_NONE,
                          FXFALSE, FXFALSE);
    }
    else if (where == 1)
    {
        wrap_grTexCombine(FX_TMU1,
                          GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                          GR_COMBINE_FUNCTION_LOCAL_ALPHA, GR_COMBINE_FACTOR_NONE,
                          FXFALSE, FXFALSE);
        wrap_grTexCombine(FX_TMU0,
                          GR_COMBINE_FUNCTION_BLEND, GR_COMBINE_FACTOR_ONE,
                          GR_COMBINE_FUNCTION_BLEND, GR_COMBINE_FACTOR_ONE,
                          FXFALSE, FXFALSE);
    }
}

void setup_depthtest()
{
    GLcontext* ctx = FXctx;

    if (ctx->DepthTest)
    {
        if (_wbuffer)
        {
            wrap_grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
            //LESS
            wrap_grDepthBufferFunction(GR_CMP_LESS);
        }
        else
        {
            wrap_grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
            wrap_grDepthBufferFunction(GR_CMP_GREATER);
        }

        if (ctx->DepthWrite)
        {
            wrap_grDepthMask(FXTRUE);
        }
        else
        {
            wrap_grDepthMask(FXFALSE);
        }
    }
    else
    {
        wrap_grDepthBufferFunction(GR_CMP_ALWAYS);
        wrap_grDepthMask(FXFALSE);
    }
}

void setup_fog()
{
    GLcontext* ctx = FXctx;

    if (ctx->Fog)
    {
        GrFog_t fog[GR_FOG_TABLE_SIZE+1];

        if (ctx->FogDensity != fx->fogDensity)
        {
            guFogGenerateLinear(fog, 0.0f, (1.0f - ctx->FogDensity) * 65535.0f);
            grFogTable(fog);

            fx->fogDensity = ctx->FogDensity;
        }

        if (ctx->FogColor[0] != fx->fogColor[0] ||
            ctx->FogColor[1] != fx->fogColor[1] ||
            ctx->FogColor[2] != fx->fogColor[2] ||
            ctx->FogColor[3] != fx->fogColor[3])
        {
            grFogColorValue(FXCOLOR(RUB(ctx->FogColor[0]),
                                    RUB(ctx->FogColor[1]),
                                    RUB(ctx->FogColor[2]),
                                    RUB(ctx->FogColor[3])));

            V4_COPY(fx->fogColor, ctx->FogColor);
        }

        wrap_grFogMode(GR_FOG_WITH_TABLE);
    }
    else
    {
        wrap_grFogMode(GR_FOG_DISABLE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : setup_raster
    Description : called from the GL whenever a state change has affected
                  rendering
    Inputs      : ctx - the GL's context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void setup_raster(GLcontext* ctx)
{
    fx->setup = choose_setup_function(ctx);

    fx->bias = (ctx->Bias == GL_TRUE && ctx->TexEnabled) ? FXTRUE : FXFALSE;

    _alphaUp = FXFALSE;

    setup_scissor();
    setup_blend();
    setup_alphatest();
    setup_texture(inWhichTMU());
    setup_depthtest();
    setup_fog();

    setup_tmu(inWhichTMU());

#if 0
    if (_chromakey)
    {
        wrap_grChromakeyMode(GR_CHROMAKEY_ENABLE);
        wrap_grChromakeyValue(0);
    }
    else
    {
        wrap_grChromakeyMode(GR_CHROMAKEY_DISABLE);
    }
#endif
}

static void clear_color(
    GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    fx->clearc = (GrColor_t)FXCOLOR(red, green, blue, 255);
    fx->cleara = (GrAlpha_t)alpha;
}

static void clear_colorbuffer(GLcontext* ctx)
{
    wrap_grDepthMask(FXFALSE);
    grBufferClear(fx->clearc, fx->cleara, 0);
    if (ctx->DepthTest && ctx->DepthWrite)
    {
        wrap_grDepthMask(FXTRUE);
    }
}

static void clear_depthbuffer(GLcontext* ctx)
{
    wrap_grDepthMask(FXTRUE);
    wrap_grColorMask(FXFALSE, FXFALSE);

    if (_wbuffer)
    {
        //FARTHEST
        grBufferClear(fx->clearc, fx->cleara, GR_WDEPTHVALUE_FARTHEST);
    }
    else
    {
        grBufferClear(fx->clearc, fx->cleara, 1);
    }

    wrap_grColorMask(FXTRUE, FXFALSE);
    if (!FXctx->DepthTest)
    {
        wrap_grDepthMask(FXFALSE);
    }
}

/*
 * clear the color and depth buffers simultaneously for efficiency.
 * leave the clip window as it is
 */
static void clear_both_buffers(GLcontext* ctx)
{
    wrap_grDepthMask(FXTRUE);
    wrap_grColorMask(FXTRUE, FXFALSE);

    if (_wbuffer)
    {
        //FARTHEST
        grBufferClear(fx->clearc, fx->cleara, GR_WDEPTHVALUE_FARTHEST);
    }
    else
    {
        grBufferClear(fx->clearc, fx->cleara, 1);
    }

    if (!FXctx->DepthTest)
    {
        wrap_grDepthMask(FXFALSE);
    }
}

static void set_monocolor(
        GLcontext* ctx, GLint r, GLint g, GLint b, GLint a)
{
    GrColor_t c = FXCOLOR(r,g,b,a);

    if (c != fx->color)
    {
        fx->color = c;
        fx->alphaColor = a;
        wrap_grConstantColorValue(fx->color);
    }
}

/*-----------------------------------------------------------------------------
    Name        : bias_triangle
    Description : sets the vertex colors of a triangle to the GL's current
                  pixelbias values
    Inputs      : a, b, c - the verts to set
    Outputs     : modifies the rgb components of the specified verts
    Return      :
----------------------------------------------------------------------------*/
static void bias_triangle(GLuint a, GLuint b, GLuint c)
{
    GrVertex* GVB;
    float R, G, B;

    R = (float)FXctx->ByteBias[0];
    G = (float)FXctx->ByteBias[1];
    B = (float)FXctx->ByteBias[2];

    GVB = &fx->gwin[a];
    GVB->r = R;
    GVB->g = G;
    GVB->b = B;

    GVB = &fx->gwin[b];
    GVB->r = R;
    GVB->g = G;
    GVB->b = B;

    GVB = &fx->gwin[c];
    GVB->r = R;
    GVB->g = G;
    GVB->b = B;
}

/*-----------------------------------------------------------------------------
    Name        : alpha_triangle
    Description : fills in the alpha components from fx->alphaColor for
                  iterating an otherwise flat-shaded poly
    Inputs      : a, b, c - the verts to set
    Outputs     : modifies the a component of the specified verts
    Return      :
----------------------------------------------------------------------------*/
static void alpha_triangle(GLuint a, GLuint b, GLuint c)
{
    float A = (float)fx->alphaColor;

    fx->gwin[a].a = A;
    fx->gwin[b].a = A;
    fx->gwin[c].a = A;
}

/*-----------------------------------------------------------------------------
    Name        : draw_triangle
    Description : draws a triangle via glide.  may decide to alter the texture
                  clamp mode, or bias the triangle, or setup for iterated
                  alpha w/ constant rgb
    Inputs      : vl[] - vertex list, pv - dominant vert for monocolor tris
    Outputs     : renders a triangle
    Return      :
----------------------------------------------------------------------------*/
static void draw_triangle(GLuint vl[], GLuint pv)
{
    loFPU();
    fx->setup(FXctx, vl[0], vl[0]);
    fx->setup(FXctx, vl[1], vl[1]);
    fx->setup(FXctx, vl[2], vl[2]);
    restoreFPU();

    if (fx->bias)
    {
        bias_triangle(vl[0], vl[1], vl[2]);
    }
    if (_alphaUp)
    {
        alpha_triangle(vl[0], vl[1], vl[2]);
    }

    if (FXctx->ShadeModel == GL_FLAT)
    {
        vertex_buffer* VB = FXctx->VB;
        GLubyte* c = VB->Color[pv];
        set_monocolor(FXctx, c[0], c[1], c[2], c[3]);
    }

    grDrawTriangle(&fx->gwin[vl[0]], &fx->gwin[vl[1]], &fx->gwin[vl[2]]);
}

/*-----------------------------------------------------------------------------
    Name        : draw_quad
    Description : draws a quad ala glide, currently by drawing 2 triangles but ideally
                  with a strip or fan
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
    Name        : draw_triangle_fan
    Description : draws a triangle fan
    Inputs      : n - number of verts in the vertex list
                  vl - vertex list
                  pv - dominant vert (ignored)
    Outputs     : renders a triangle fan
    Return      :
----------------------------------------------------------------------------*/
static void draw_triangle_fan(GLuint n, GLuint vl[], GLuint pv)
{
    GLuint vlist[3];
    GLuint i, j0;

    j0 = vl[0];
    for (i = 2; i < n; i++)
    {
        vlist[0] = j0;
        vlist[1] = vl[i-1];
        vlist[2] = vl[i];
        draw_triangle(vlist, pv);
    }
}

/*-----------------------------------------------------------------------------
    Name        : draw_triangle_strip
    Description : draws a triangle strip
    Inputs      : n - number of verts, vl - vertex list, pv - dominant vert (ignored)
    Outputs     : renders a triangle strip
    Return      :
----------------------------------------------------------------------------*/
static void draw_triangle_strip(GLuint n, GLuint vl[], GLuint pv)
{
    GLuint vlist[3];
    GLuint i;

    for (i = 2; i < n; i++)
    {
        if (i&1)
        {
            vlist[0] = i;
            vlist[1] = i-1;
            vlist[2] = i-2;
            draw_triangle(vlist, i);
        }
        else
        {
            vlist[0] = i-2;
            vlist[1] = i-1;
            vlist[2] = i;
            draw_triangle(vlist, i);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : draw_line
    Description : draws a line via glide.  uses custom linedraw funcs for lines
                  with features of the GL not supported by 3Dfx/glide
    Inputs      : vert0 - 1st vertex of the line
                  vert1 - 2nd vertex of the line
                  pvert - dominant vertex (for monocolor) of the line
    Outputs     : a line
    Return      :
----------------------------------------------------------------------------*/
static void draw_line(GLuint vert0, GLuint vert1, GLuint pvert)
{
    GLboolean aa = (FXctx->Blend && FXctx->LineSmooth);

    if (FXctx->ShadeModel == GL_FLAT)
    {
        vertex_buffer* VB = FXctx->VB;
        GLubyte* c = VB->Color[pvert];
        set_monocolor(FXctx, c[0], c[1], c[2], c[3]);
    }

    loFPU();
    fx->setup(FXctx, vert0, vert0);
    fx->setup(FXctx, vert1, vert1);
    restoreFPU();
    if (aa)
    {
//        fx->gwin[vert0].a = 255.0f;
//        fx->gwin[vert1].a = 255.0f;
        grAADrawLine(&fx->gwin[vert0], &fx->gwin[vert1]);
    }
    else
    {
        grDrawLine(&fx->gwin[vert0], &fx->gwin[vert1]);
    }
}

void fx_blend_16bit_oneminussrcalpha(
    GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLushort* cdest)
{
    GLint r, g, b;
    GLint t = (GLint)sa;
    GLint s = 256 - t;
    GLint dr, dg, db;
    GLushort dbyte = *cdest;

    dr = dbyte >> 11;
    dg = (dbyte >> 5) & 0x3f;
    db = dbyte & 0x1f;
    dr <<= 3;
    dg <<= 2;
    db <<= 3;
    r = (*sr * t + dr * s) >> 8;
    g = (*sg * t + dg * s) >> 8;
    b = (*sb * t + db * s) >> 8;
    *sr = MIN2(r, 255);
    *sg = MIN2(g, 255);
    *sb = MIN2(b, 255);
}

void fx_blend_16bit_one(
    GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLushort* cdest)
{
    GLint r, g, b;
    GLint t = (GLint)sa;
    GLint s = 256;
    GLint dr, dg, db;
    GLushort dbyte = *cdest;

    dr = dbyte >> 11;
    dg = (dbyte >> 5) & 0x3f;
    db = dbyte & 0x1f;
    dr <<= 3;
    db <<= 2;
    db <<= 3;
    r = (*sr * t + dr * s) >> 8;
    g = (*sg * t + db * s) >> 8;
    b = (*sb * t + db * s) >> 8;
    *sr = MIN2(r, 255);
    *sg = MIN2(g, 255);
    *sb = MIN2(b, 255);
}

void fx_blend_16bit(GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLushort* cdest)
{
    if (FXctx->BlendDst == GL_ONE)
    {
        fx_blend_16bit_one(sr, sg, sb, sa, cdest);
    }
    else
    {
        fx_blend_16bit_oneminussrcalpha(sr, sg, sb, sa, cdest);
    }
}

/*-----------------------------------------------------------------------------
    Name        : draw_point
    Description : draws >= 1 points into the 3Dfx framebuffer.  uses direct
                  writes if depthtesting/writing is disabled
    Inputs      : first - starting vert
                  last - final vert
    Outputs     : displays a point
    Return      :
----------------------------------------------------------------------------*/
static void draw_point(GLuint first, GLuint last)
{
    GLuint i;
    vertex_buffer* VB = FXctx->VB;
    GrVertex* vp = &fx->gwin[first];
    GLint pointSize = (GLint)FXctx->PointSize;

    if (FXctx->PointSize != 1.0f)
    {
        fx->setup(FXctx, first, last);

        for (i = first; i <= last; i++, vp++)
        {
            if (VB->ClipMask[i] == 0)
            {
                if (FXctx->ShadeModel == GL_FLAT)
                {
                    wrap_grConstantColorValue(
                        FXCOLOR(VB->Color[i][0], VB->Color[i][1],
                        VB->Color[i][2], VB->Color[i][3]));
                }
                grDrawPoint(vp);
                vp->x += 1.0f;
                grDrawPoint(vp);
                vp->y += 1.0f;
                grDrawPoint(vp);
                vp->x -= 1.0f;
                grDrawPoint(vp);
            }
        }

        return;
    }

    if (FXctx->DepthTest && FXctx->DepthWrite
        && !FXctx->SansDepth && !FXctx->EffectPoint)
    {
        fx->setup(FXctx, first, last);

        for (i = first; i <= last; i++, vp++)
        {
            if (VB->ClipMask[i] == 0)
            {
                grDrawPoint(vp);
            }
        }
    }
#if 0
    else if (_lfbAccess && _pointlfb)
#else
    else if (_lfbAccess && FXctx->PointSize != 1.0f)
#endif
    {
        GrLfbInfo_t info, ainfo;
        FxU16* p;
        FxU16* ap;
        GLint stride, px, py;

        info.size = sizeof(info);
        if (!grLfbLock(GR_LFB_WRITE_ONLY,
                       GR_BUFFER_BACKBUFFER,
                       GR_LFBWRITEMODE_565,
                       GR_ORIGIN_UPPER_LEFT,
                       FXFALSE, &info))
        {
            return;
        }

        if (FXctx->Blend)
        {
            ainfo.size = sizeof(ainfo);
            if (!grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565,
                           GR_ORIGIN_UPPER_LEFT, FXFALSE, &ainfo))
            {
                grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
                return;
            }
        }

        stride = info.strideInBytes >> 1;

        if (1 || FXctx->ShadeModel == GL_SMOOTH)
        {
            for (i = first; i <= last; i++)
            {
                if (VB->ClipMask[i] == 0 || FXctx->PointHack)
                {
                    px = FAST_TO_INT(VB->Win[i][0]);
                    py = ADJUST_UPPER_LEFT(FAST_TO_INT(VB->Win[i][1]));
#ifdef SCISSORING
                    if (FXctx->ScissorTest)
                    {
                        if (py < fx->ScissorY || py >= (fx->ScissorY + fx->ScissorHeight))
                            continue;
                        if (px < fx->ScissorX || px >= (fx->ScissorX + fx->ScissorWidth))
                            continue;
                    }
#endif
                    p = ((FxU16*)info.lfbPtr) + px + (py*stride);

                    if (FXctx->Blend)
                    {
                        GLubyte red = VB->Color[i][0];
                        GLubyte green = VB->Color[i][1];
                        GLubyte blue = VB->Color[i][2];
                        GLubyte alpha = VB->Color[i][3];

                        ap = ((FxU16*)ainfo.lfbPtr) + px + (py*stride);

                        fx_blend_16bit(&red, &green, &blue, alpha, (GLushort*)ap);
                        *p = FXCOLOR565(red, green, blue);
                    }
                    else
                    {
                        *p = FXCOLOR565(VB->Color[i][0], VB->Color[i][1], VB->Color[i][2]);
                    }

                    //large point (size == 2)
                    if (pointSize > 1)
                    {
                        FxU16 color565 = FXCOLOR565(VB->Color[i][0], VB->Color[i][1], VB->Color[i][2]);
                        if (px+1 < FXctx->Buffer.Width)
                        {
                            *(p+1) = color565;
                        }
                        if (py+1 < FXctx->Buffer.Height)
                        {
                            p += stride;
                            *p = color565;
                            if (px+1 < FXctx->Buffer.Width)
                            {
                                *(p+1) = color565;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            FxU16 color = (FxU16)fx->color;
            for (i = first; i <= last; i++)
            {
                if (VB->ClipMask[i] == 0)
                {
                    px = FAST_TO_INT(VB->Win[i][0]);
                    py = ADJUST_UPPER_LEFT(FAST_TO_INT(VB->Win[i][1]));
                    p = ((FxU16*)info.lfbPtr)
                        + px
                        + (py*stride);
                    *p = color;
                }
            }
        }

        grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);

        if (FXctx->Blend)
        {
            grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER);
        }
    }
    else
    {
        fx->setup(FXctx, first, last);

        if (FXctx->ShadeModel == GL_SMOOTH)
        {
            for (i = first; i <= last; i++, vp++)
            {
                if (VB->ClipMask[i] == 0)
                {
                    grDrawPoint(vp);
                }
            }
        }
        else
        {
            for (i = first; i <= last; i++, vp++)
            {
                if (VB->ClipMask[i] == 0)
                {
                    wrap_grConstantColorValue(
                        FXCOLOR(VB->Color[i][0], VB->Color[i][1], VB->Color[i][2], VB->Color[i][3]));
                    grDrawPoint(vp);
                }
            }
        }
    }
}

static void draw_pixel(GLint x, GLint y, GLdepth z)
{
    GrVertex va;
    va.x = (GLfloat)x;
    va.y = (GLfloat)y;
    //TODO: use z value (?)
    grDrawPoint(&va);
}

static void draw_bitmap(
    GLcontext* ctx,
    GLsizei width, GLsizei height,
    GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove)
{
    GLubyte* bitmap = (GLubyte*)ctx->Current.Bitmap;
    FxU16* p;
    GrLfbInfo_t info;
    GLubyte* pb;
    int x, y;
    GLint r, g, b, a, px, py, scrwidth, scrheight, stride;
    FxU16 color;

    if (!_lfbAccess)
    {
        return;
    }

    scrwidth  = ctx->Buffer.Width;
    scrheight = ctx->Buffer.Height;

    px = (GLint)((ctx->Current.RasterPos[0] - xorig) + 0.0f);
    py = (GLint)((ctx->Current.RasterPos[1] - yorig) + 0.0f);

    pb = bitmap;

    grLfbWriteColorFormat(GR_COLORFORMAT_ARGB);

    info.size = sizeof(info);
    if (!grLfbLock(GR_LFB_WRITE_ONLY,
                   GR_BUFFER_BACKBUFFER,
                   GR_LFBWRITEMODE_565,
                   GR_ORIGIN_LOWER_LEFT,
                   FXFALSE, &info))
    {
        return;
    }

    r = RUB(ctx->Current.RasterColor[0]);
    g = RUB(ctx->Current.RasterColor[1]);
    b = RUB(ctx->Current.RasterColor[2]);
    a = RUB(ctx->Current.RasterColor[3]);
    color = FXCOLOR565(r,g,b);

    stride = info.strideInBytes >> 1;

    for (y = 0; y < height; y++)
    {
        GLubyte bitmask = 128;

        p = ((FxU16*)info.lfbPtr) + px + (py+y)*stride;

        for (x = 0; x < width; x++, p++)
        {
            if (*pb & bitmask)
            {
                if (onScreen(px+x, py+y))
                {
                    *p = color;
                }
            }
            bitmask >>= 1;
            if (bitmask == 0)
            {
                pb++;
                bitmask = 128;
            }
        }

        if (bitmask != 128)
            pb++;
    }

    grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);

    ctx->Current.RasterPos[0] += xmove;
    ctx->Current.RasterPos[1] += ymove;
}

static void glide_pixel(
    FxU16* p, GLint x, GLint y, GLint stride, GLubyte r, GLubyte g, GLubyte b)
{
    y = ADJUST_UPPER_LEFT(y);
    if (onScreen(x, y))
    {
        p += x + y*stride;
        *p = FXCOLOR565(r, g, b);
    }
}

static void glide_blended_pixel(
    FxU16* p, FxU16* rp, GLint x, GLint y, GLint stride, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    y = ADJUST_UPPER_LEFT(y);
    if (onScreen(x, y))
    {
        p += x + y*stride;
        rp += x + y*stride;
        fx_blend_16bit(&r, &g, &b, a, rp);
        *p = FXCOLOR565(r, g, b);
    }
}

static void nolfb_drawpixels(GLcontext* ctx, int width, int height, GLenum format)
{
    int py, px;
    int y, x;
    GLubyte* pb;
    GrVertex va;

    px = (int)ctx->Current.RasterPos[0];
    py = (int)ctx->Current.RasterPos[1];
    pb = ctx->Current.Bitmap;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++, pb += 4)
        {
            if (onScreen(px+x, py+y))
            {
                va.x = (float)(px+x);
                va.y = (float)(py+y);
                va.r = (float)pb[0];
                va.g = (float)pb[1];
                va.b = (float)pb[2];
                va.a = (float)pb[3];
                grDrawPoint(&va);
            }
        }
    }
}

static void draw_pixels(
    GLcontext* ctx,
    GLsizei width, GLsizei height,
    GLenum format, GLenum type)
{
    GLubyte* pb;
    GLint px, py;
    FxU16* p;
    GrLfbInfo_t info;
    GrLfbInfo_t ainfo;
    int x, y, stride;
    GLubyte r, g, b;

    if (!_lfbAccess)
    {
        nolfb_drawpixels(ctx, width, height, format);
        return;
    }

    if (type != GL_UNSIGNED_BYTE)
    {
        //error condition
        return;
    }

    if (format != GL_RGB &&
        format != GL_RGB16 &&
        format != GL_RGBA &&
        format != GL_RGB8 &&
        format != GL_COLOR_INDEX)
    {
        //error condition
        return;
    }

    //do the dirty work

    px = (GLint)ctx->Current.RasterPos[0];
    py = (GLint)ctx->Current.RasterPos[1];

    pb = ctx->Current.Bitmap;

    if (pb == NULL)
    {
        if (ctx->TexBoundObject == NULL || !ctx->TexBoundObject->created)
        {
            //error condition
            return;
        }
        px = (GLint)ctx->Current.TexCoord[0];
        py = (GLint)ctx->Current.TexCoord[1];
        pb = ctx->TexBoundObject->Data;
    }

    grLfbWriteColorFormat(GR_COLORFORMAT_ARGB);

    info.size = sizeof(info);
    if (!grLfbLock(GR_LFB_WRITE_ONLY,
                   GR_BUFFER_BACKBUFFER,
                   GR_LFBWRITEMODE_565,
                   GR_ORIGIN_UPPER_LEFT,
                   FXFALSE, &info))
    {
        return;
    }

    stride = info.strideInBytes >> 1;

    if (ctx->Blend || format == GL_RGB8)
    {
        ainfo.size = sizeof(ainfo);
        if (!grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565,
                       GR_ORIGIN_UPPER_LEFT, FXFALSE, &ainfo))
        {
            grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
            return;
        }
    }

    if (format == GL_RGB8)
    {
        r = RUB(ctx->Current.RasterColor[0]);
        g = RUB(ctx->Current.RasterColor[1]);
        b = RUB(ctx->Current.RasterColor[2]);

        for (y = 0; y < height; y++)
        {
            if (!onScreen(0, py+y))
            {
                pb += width;
                continue;
            }
            for (x = 0; x < width; x++, pb++)
            {
                if (*pb)
                {
#if 0
                    GLubyte dr, dg, db;
                    GLint alpha;

                    alpha = *pb * 16;

                    dr = (r * alpha) >> 8;
                    dg = (g * alpha) >> 8;
                    db = (b * alpha) >> 8;

                    glide_pixel((FxU16*)info.lfbPtr, px+x, py+y, stride, dr, dg, db);
#else
                    if (*pb > 14)
                    {
                        glide_pixel((FxU16*)info.lfbPtr, px+x, py+y, stride, r, g, b);
                    }
                    else
                    {
                        glide_blended_pixel((FxU16*)info.lfbPtr, (FxU16*)ainfo.lfbPtr,
                                            px+x, py+y, stride, r, g, b,
                                            (GLubyte)(*pb * 16));
                    }
#endif
                }
            }
        }
    }
    else if (format == GL_COLOR_INDEX)
    {
        //4*256 bytes of palette data, followed by
        //width*height bytes of bitmap data
        GLubyte* palp;
        GLubyte* cp;

        palp = ctx->Current.Bitmap;
        pb += 4*256;

        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++, pb++)
            {
                cp = palp + 4*(*pb);

                if (ctx->AlphaTest)
                {
                    switch (ctx->AlphaFunc)
                    {
                    case GL_LESS:
                        if (cp[3] >= ctx->AlphaByteRef)
                        {
                            continue;
                        }
                        break;
                    case GL_GREATER:
                        if (cp[3] <= ctx->AlphaByteRef)
                        {
                            continue;
                        }
                        break;
                    default:
                        continue;
                    }
                }

                if (ctx->Blend && (cp[3] < 255))
                {
                    if (cp[3] > 0)
                    {
                        glide_blended_pixel((FxU16*)info.lfbPtr, (FxU16*)ainfo.lfbPtr,
                                            px+x, py+y, stride, cp[0], cp[1], cp[2], cp[3]);
                    }
                }
                else
                {
                    glide_pixel((FxU16*)info.lfbPtr, px+x, py+y, stride,
                                cp[0], cp[1], cp[2]);
                }
            }
        }
    }
    else if (format == GL_RGB)
    {
        int iy;

        for (y = 0; y < height; y++)
        {
            iy = py+y;
            if (!onScreen(0, iy))
            {
                pb += 3*width;
                continue;
            }
            p = (FxU16*)info.lfbPtr + px + iy*stride;
            for (x = 0; x < width; x++, pb += 3, p++)
            {
                if (onScreen(x, iy))
                {
                    *p = FXCOLOR565(pb[0], pb[1], pb[2]);
                }
            }
        }
    }
    else if (format == GL_RGBA)
    {
        if (ctx->Blend || ctx->AlphaTest)
        {
            for (y = 0; y < height; y++)
            {
                for (x = 0; x < width; x++, pb += 4)
                {
                    if (ctx->AlphaTest)
                    {
                        switch (ctx->AlphaFunc)
                        {
                        case GL_LESS:
                            if (pb[3] >= ctx->AlphaByteRef)
                            {
                                continue;
                            }
                            break;
                        case GL_GREATER:
                            if (pb[3] <= ctx->AlphaByteRef)
                            {
                                continue;
                            }
                            break;
                        default:
                            continue;
                        }
                    }

                    if (ctx->Blend && (pb[3] < 255))
                    {
                        if (pb[3] > 0)
                        {
                            glide_blended_pixel((FxU16*)info.lfbPtr, (FxU16*)ainfo.lfbPtr, px+x, py+y,
                                                stride, pb[0], pb[1], pb[2], pb[3]);
                        }
                    }
                    else
                    {
                        glide_pixel((FxU16*)info.lfbPtr, px+x, py+y, stride, pb[0], pb[1], pb[2]);
                    }
                }
            }
        }
        else
        {
            for (y = 0; y < height; y++)
            {
                for (x = 0; x < width; x++, pb += 4)
                {
                    glide_pixel((FxU16*)info.lfbPtr, px+x, py+y, stride, pb[0], pb[1], pb[2]);
                }
            }
        }
    }
    else
    {
        FxU16* ppb = (FxU16*)pb;

        for (y = 0; y < height; y++)
        {
            p = ((FxU16*)info.lfbPtr) + px + (py + y)*stride;

            for (x = 0; x < width; x++, p++, ppb++)
            {
                if (onScreen(px+x, py+y))
                {
                    *p = *ppb;
                }
            }
        }
    }

    grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
    if (ctx->Blend || format == GL_RGB8)
    {
        grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER);
    }

    ctx->Current.RasterPos[0] += (GLfloat)width;
    ctx->Current.RasterPos[1] += (GLfloat)height;
}

/*-----------------------------------------------------------------------------
    Name        : fxQueryHardware
    Description : determines if a 3Dfx is present, and if so determines it's
                  chipset (SST1 or SST96)
    Inputs      :
    Outputs     :
    Return      : -1 or the type (hwconfig.SSTs[0].type) of the chipset
----------------------------------------------------------------------------*/
static int fxQueryHardware()
{
    if (!glideInitialized)
    {
        grGlideInit();
        if (grSstQueryHardware(&hwconfig))
        {
            grSstSelect(0);
            _3dfxPresent = 1;
        }
        else
        {
            _3dfxPresent = 0;
            glideInitialized = 1;
        }
    }

    if (!_3dfxPresent)
    {
        return -1;
    }
    else
    {
        return (hwconfig.SSTs[0].type);
    }
}

/*-----------------------------------------------------------------------------
    Name        : fx_init
    Description : 3Dfx glide-level init func
    Inputs      : ctx - the GL's context
    Outputs     :
    Return      : GL_TRUE on success or GL_FALSE on failure
----------------------------------------------------------------------------*/
static GLboolean fx_init(GLcontext* ctx)
{
    GLuint win;
    GrScreenResolution_t res;
    GrScreenRefresh_t ref;
    int type;

    LOG("fx_init\n");

    _widthMinusOne  = ctx->Buffer.Width - 1;
    _heightMinusOne = ctx->Buffer.Height - 1;

    switch (ctx->Buffer.Width)
    {
    case 640:
        res = GR_RESOLUTION_640x480;
        break;
    case 800:
        res = GR_RESOLUTION_800x600;
        break;
    case 1024:
        res = GR_RESOLUTION_1024x768;
        break;
    case 1280:
        res = GR_RESOLUTION_1280x1024;
        break;
    default:
        res = GR_RESOLUTION_1600x1200;
    }
    ref = GR_REFRESH_60Hz;

    type = fxQueryHardware();
    if (type >= 0)
    {
        win = 0;
    }
    else
    {
        return GL_FALSE;
    }

    if (!grSstWinOpen((FxU32)win, res, ref,
                      GR_COLORFORMAT_ABGR,
                      GR_ORIGIN_LOWER_LEFT,
                      2, 1))
    {
        return GL_FALSE;
    }

    wrap_grColorMask(FXTRUE, FXFALSE);
    grRenderBuffer(GR_BUFFER_BACKBUFFER);

    wrap_grClipWindow(0, 0, ctx->Buffer.Width, ctx->Buffer.Height);
    grCullMode(GR_CULL_DISABLE);
    grDitherMode(GR_DITHER_4x4);
    grGammaCorrectionValue(_gamma);

    return GL_TRUE;
}

static void deactivate()
{
    LOG("deactivate\n");
    grSstControl(GR_CONTROL_DEACTIVATE);
}

static void activate()
{
    LOG("activate\n");
    grSstControl(GR_CONTROL_ACTIVATE);
    grGammaCorrectionValue(_gamma);
    clear_last_raster();
}

DLL void shutdown_driver(GLcontext* ctx)
{
    LOG("shutdown_driver\n");
    deactivate();
    //kill texture ram
    tmClose(fx, FX_TMU0);
    if (fx->numTMUs > 1)
    {
        tmClose(fx, FX_TMU1);
    }
    if (glideInitialized)
    {
        //shutdown the fx
        grGlideShutdown();
        glideInitialized = 0;
    }

    if (flog != NULL)
    {
        fclose(flog);
        flog = NULL;
    }

    if (fbRAM != NULL)
    {
        ctx->FreeFunc(fbRAM);
        fbRAM = NULL;
    }
}

static void fx_flush()
{
    LOG("*** flush ***\n");
    grBufferSwap(0);
//    grSstIdle();
}

static void scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    fx->ScissorX = x;
    fx->ScissorY = y;
    fx->ScissorWidth = width;
    fx->ScissorHeight = height;
}

static void draw_background(GLubyte* pixels)
{
    GLushort  data[640*480];
    GLushort* dp;
    GLubyte*  bp;
    GLint     x, y;

    dp = data;
    for (y = 0; y < 480; y++)
    {
        bp = &pixels[(479 - y) * 3*640];
        for (x = 0; x < 640; x++, bp += 3, dp++)
        {
            *dp = FORM_RGB565(bp[2], bp[1], bp[0]);
        }
    }

    grLfbWriteColorFormat(GR_COLORFORMAT_RGBA);
    grLfbWriteRegion(GR_BUFFER_BACKBUFFER, 0, 0,
                     GR_LFB_SRC_FMT_565, 640, 480,
                     2*640, data);
}

static void read_pixels(
    GLcontext* ctx, GLint sx, GLint sy, GLsizei width, GLsizei height,
    GLenum format, GLenum type)
{
    GLint i, y;
    GLubyte* bp = ctx->Current.Bitmap;

    grLfbWriteColorFormat(GR_COLORFORMAT_RGBA);

    for (y = 0; y < ctx->Buffer.Height; y++)
    {
        grLfbReadRegion(GR_BUFFER_BACKBUFFER, 0,
                        (ctx->Buffer.Height - 1) - y,
                        ctx->Buffer.Width, 1, 0,
                        fbRAM + y*ctx->Buffer.Width);
    }

    for (i = 0; i < ctx->Buffer.Width * ctx->Buffer.Height; i++)
    {
        bp[0] = (fbRAM[i] & 0xf800) >> 8;
        bp[1] = (fbRAM[i] & 0x07e0) >> 3;
        bp[2] = (fbRAM[i] & 0x001f) << 3;
        bp += 3;
    }

    grLfbWriteColorFormat(GR_COLORFORMAT_ARGB);
}

static void screenshot(GLubyte* buf)
{
    GLint i, y;
    GLubyte* bp = buf;

    grLfbWriteColorFormat(GR_COLORFORMAT_RGBA);
    for (y = 0; y < FXctx->Buffer.Height; y++)
    {
        grLfbReadRegion(GR_BUFFER_BACKBUFFER, 0,
                        (FXctx->Buffer.Height - 1) - y,
                        FXctx->Buffer.Width, 1, 0,
                        fbRAM+y*FXctx->Buffer.Width);
    }
    for (i = 0; i < FXctx->Buffer.Width*FXctx->Buffer.Height; i++)
    {
        bp[0] = (fbRAM[i] & 0x001f) << 3;
        bp[1] = (fbRAM[i] & 0x07e0) >> 3;
        bp[2] = (fbRAM[i] & 0xf800) >> 8;
        bp += 3;
    }
    grLfbWriteColorFormat(GR_COLORFORMAT_ARGB);
}

static void gamma_up()
{
    _gamma += 0.05f;
    grGammaCorrectionValue(_gamma);
}

static void gamma_dn()
{
    _gamma -= 0.05f;
    grGammaCorrectionValue(_gamma);
}

static void chromakey(GLubyte r, GLubyte g, GLubyte b, GLboolean on)
{
    _chromakey = on;
}

static void super_clear()
{
#if 0
    GLint i, width, height;

    width = FXctx->Buffer.Width;
    height = FXctx->Buffer.Height;

    MEMSET(fbRAM, 0, sizeof(GLushort)*width*height);

    grLfbWriteRegion(GR_BUFFER_BACKBUFFER, 0, 0, GR_LFB_SRC_FMT_565,
                     width, height, sizeof(GLushort)*width, fbRAM);
    grLfbWriteRegion(GR_BUFFER_FRONTBUFFER, 0, 0, GR_LFB_SRC_FMT_565,
                     width, height, sizeof(GLushort)*width, fbRAM);
#endif
}

static void fastbind_set(GLboolean state)
{
    fx_context* fx = (fx_context*)FXctx->DriverCtx;

    fx->fastbind = state;
}

static void driver_caps(GLcontext* ctx)
{
    ctx->Buffer.Pitch = 2 * ctx->Buffer.Width;
    ctx->Buffer.Depth = 16;
    ctx->Buffer.PixelType = GL_RGB565;
}

static int feature_exists(GLint feature)
{
    switch (feature)
    {
    case GL_SHARED_TEXTURE_PALETTE_EXT:
        return GL_TRUE;
    case RGL_COLOROP_ADD:
        return GL_TRUE;
    default:
        return GL_FALSE;
    }
}

static GLboolean post_init_driver(GLcontext* ctx)
{
    if (!fx_init(ctx))
    {
        return GL_FALSE;
    }

    fbRAM = (GLushort*)ctx->AllocFunc(sizeof(GLushort) * ctx->Buffer.Width * ctx->Buffer.Height);

    if (hwconfig.SSTs[0].type == GR_SSTTYPE_VOODOO)
    {
        fx->numTMUs = hwconfig.SSTs[0].sstBoard.VoodooConfig.nTexelfx;
    }
    else if (hwconfig.SSTs[0].type == GR_SSTTYPE_SST96)
    {
        fx->numTMUs = hwconfig.SSTs[0].sstBoard.SST96Config.nTexelfx;
    }
    else
    {
        fx->numTMUs = 1;
    }

#if SIMPLE_TMU_USAGE
    if (fx->numTMUs == 1)
    {
        FX_TMU = FX_TMU0;
    }
    else
    {
        FX_TMU = FX_TMU1;
    }
#endif

    grHints(GR_HINT_STWHINT, 0);

    tmInit(ctx, fx, FX_TMU0);
    if (fx->numTMUs > 1)
    {
        tmInit(ctx, fx, FX_TMU1);
    }
    tmInitTexobjs(fx);

    activate();

    return GL_TRUE;
}

typedef void (*VoidFunc)(void);

DLL GLboolean init_driver(GLcontext* ctx)
{
    FXctx = ctx;

    _gamma = DEFAULT_GAMMA;

    /* setup function pointers for the GL */
    ctx->DR.init_driver = init_driver;
    ctx->DR.post_init_driver = post_init_driver;
    ctx->DR.shutdown_driver = shutdown_driver;

    ctx->DR.clear_depthbuffer = clear_depthbuffer;
    ctx->DR.clear_colorbuffer = clear_colorbuffer;
    ctx->DR.clear_both_buffers = clear_both_buffers;

    ctx->DR.allocate_depthbuffer = NULL;
    ctx->DR.allocate_colorbuffer = NULL;

    ctx->DR.setup_triangle = NULL;
    ctx->DR.setup_line = NULL;
    ctx->DR.setup_point = NULL;
    ctx->DR.setup_raster = setup_raster;

    ctx->DR.set_monocolor = set_monocolor;
    ctx->DR.flush = fx_flush;
    ctx->DR.clear_color = clear_color;
    ctx->DR.scissor = scissor;

    ctx->DR.draw_triangle = draw_triangle;
    ctx->DR.draw_triangle_array = NULL;//draw_triangle_array;
    ctx->DR.draw_quad = draw_quad;
    ctx->DR.draw_triangle_fan = draw_triangle_fan;
    ctx->DR.draw_triangle_strip = draw_triangle_strip;
    ctx->DR.draw_polygon = NULL;
    ctx->DR.draw_line = draw_line;
    ctx->DR.draw_pixel = draw_pixel;
    ctx->DR.draw_bitmap = (VoidFunc)draw_bitmap;
    ctx->DR.draw_pixels = draw_pixels;
    ctx->DR.read_pixels = read_pixels;
    ctx->DR.draw_point = draw_point;

    ctx->DR.draw_clipped_triangle = NULL;
    ctx->DR.draw_clipped_polygon = NULL;

    ctx->DR.bind_texture = bind_texture;
    ctx->DR.tex_param = texparam;
    ctx->DR.tex_del = texdel;
    ctx->DR.tex_palette = texpalette;
    ctx->DR.tex_img = teximg;

    ctx->DR.deactivate = deactivate;
    ctx->DR.activate = activate;

    ctx->DR.screenshot = screenshot;
    ctx->DR.gamma_up = gamma_up;
    ctx->DR.gamma_dn = gamma_dn;

    ctx->DR.chromakey = chromakey;
    ctx->DR.super_clear = super_clear;

    ctx->DR.fastbind_set = fastbind_set;

    ctx->DR.draw_background = draw_background;

    ctx->DR.lock_buffer = NULL;
    ctx->DR.unlock_buffer = NULL;
    ctx->DR.get_framebuffer = NULL;

    ctx->DR.create_window = NULL;
    ctx->DR.delete_window = NULL;
    ctx->DR.set_save_state = NULL;
    ctx->DR.get_scratch = NULL;

    ctx->DR.driver_caps = driver_caps;

    ctx->RequireLocking = GL_FALSE;

    ctx->DR.fog_vertices = NULL;

    ctx->DR.feature_exists = feature_exists;

    /* allocate memory for ctx->DriverCtx */
    ctx->DriverCtx = (fx_context*)ctx->AllocFunc(sizeof(fx_context));

    fx = (fx_context*)ctx->DriverCtx;
    fx->wscale = 1.0f;
    fx->setup = setup;
    fx->texbind = 0;
    fx->gl_palette[0] = NULL;
    fx->gl_palette[1] = NULL;
    fx->bias = FXFALSE;
    fx->fogDensity = -1.0f;
    V4_SET(fx->fogColor, -1.0f, -1.0f, -1.0f, -1.0f);
    fx->fastbind = GL_FALSE;

    clear_last_raster();

    rglSetExtensionString("glide");

    return GL_TRUE;
}


/*=============================================================================
    texture handling functions
=============================================================================*/

GrTextureFormat_t texgetformat(GLcontext* ctx, GLenum glformat)
{
    fx_context* fx = (fx_context*)ctx->DriverCtx;
    GrTextureFormat_t format;

    switch (glformat)
    {
    case 3:
    case GL_RGB:
        format = GR_TEXFMT_RGB_565;
        break;
    case 4:
    case GL_RGBA:
    case GL_RGBA16:
        format = GR_TEXFMT_ARGB_4444;
        break;
    case GL_COLOR_INDEX:
        format = GR_TEXFMT_P_8;
        break;
    default:
        ;//FIXME: bail
    }

    return format;
}

static fx_texobj* alloctexobjdata()
{
    fx_texobj* ti;

    if (!(ti = FXctx->AllocFunc(sizeof(fx_texobj))))
    {
        //FIXME: bail
    }

    ti->valid = GL_FALSE;
    ti->tmi.inmemory = GL_FALSE;

    return ti;
}

FxU32 FXALIGN(FxU32 adr)
{
    if (TEXTURE_ALIGN == 0)
    {
        return adr;
    }
    adr += TEXTURE_ALIGN - 1;
    adr &= TEXTURE_ALIGNMASK;
    return adr;
}

static GrTextureFilterMode_t fxfiltmap(GLenum filt)
{
    if (filt == GL_NEAREST)
        return GR_TEXTUREFILTER_POINT_SAMPLED;
    else
        return GR_TEXTUREFILTER_BILINEAR;
}

static GrTextureClampMode_t fxclampmap(GLenum clamp)
{
    if (clamp == GL_REPEAT)
        return GR_TEXTURECLAMP_WRAP;
    else
        return GR_TEXTURECLAMP_CLAMP;
}

static void filltexobjdata(gl_texture_object* tex)
{
    fx_texobj* ti = (fx_texobj*)tex->DriverData;

    if (ti == NULL)
        return;

    //min/maxfilt
    ti->minfilt = fxfiltmap(tex->Min);
    ti->maxfilt = fxfiltmap(tex->Mag);

    //mmmode
    ti->mmmode = GR_MIPMAP_DISABLE;

    //s/tclamp
    ti->sclamp = fxclampmap(tex->WrapS);
    ti->tclamp = fxclampmap(tex->WrapT);
}

static GLint inWhichTMU()
{
    fx_texobj* ti;
    gl_texture_object* tex = FXctx->TexBoundObject;

    if (tex)
    {
        if (!tex->DriverData)
        {
            tex->DriverData = alloctexobjdata();
            filltexobjdata(tex);
        }

        ti = (fx_texobj*)tex->DriverData;

        if (!ti->valid)
        {
            return -1;
        }

        return ti->tmi.whichTMU;
    }
    else
    {
        return -1;
    }
}

void texbind(gl_texture_object* tex)
{
    fx_texobj* ti;
    FxU32 where;

    if (!tex->DriverData)
    {
        tex->DriverData = alloctexobjdata();
        filltexobjdata(tex);
    }

    ti = (fx_texobj*)tex->DriverData;

    if (!ti->valid)
        return;

    ti->tmi.lastused = fx->texbind++;

#if SIMPLE_TMU_USAGE
    where = FX_TMU;
#else
    if (fx->numTMUs > 1)
    {
        if (ti->info.format == GR_TEXFMT_P_8)
        {
            where = FX_TMU0;
        }
        else
        {
            where = FX_TMU1;
        }
    }
    else
    {
        where = FX_TMU0;
    }
#endif

    tmLoadTexture(fx, tex, where);

    if (ti->info.format == GR_TEXFMT_P_8 && !FXctx->UsingSharedPalette)
    {
#if WRAP_TESTS
        if (tex->Palette != fx->gl_palette[where])
#endif
        {
            LOG("-- palette download --\n");
            fx->gl_palette[where] = tex->Palette;
            grTexDownloadTable(where, GR_TEXTABLE_PALETTE, &(ti->palette));
        }
#if 0
        else
        {
            LOG("++ redundant palette load skipped ++\n");
        }
#endif
    }

    wrap_grTexClampMode(where, ti->sclamp, ti->tclamp);
    wrap_grTexFilterMode(where, ti->minfilt, ti->maxfilt);
    wrap_grTexMipMapMode(where, ti->mmmode, FXFALSE);
    grTexSource(where, FXALIGN(ti->tmi.tm[where]->startadr), GR_MIPMAPLEVELMASK_BOTH, &ti->info);
}

static void bind_texture()
{
    fx_context* fx = (fx_context*)FXctx->DriverCtx;
    if (!fx->fastbind)
    {
        texbind(FXctx->TexBoundObject);
    }
}

static void texparam(GLenum pname, GLfloat const* params)
{
    GLenum param = (GLenum)(GLint)params[0];
    fx_texobj* ti;
    gl_texture_object* tex = FXctx->TexBoundObject;
    fx_context* fx = (fx_context*)FXctx->DriverCtx;

    if (!tex->DriverData)
    {
        tex->DriverData = alloctexobjdata();
        filltexobjdata(tex);
    }

    ti = (fx_texobj*)tex->DriverData;

    switch (pname)
    {
    case GL_TEXTURE_MIN_FILTER:
        switch (param)
        {
        case GL_NEAREST:
            ti->mmmode = GR_MIPMAP_DISABLE;
            ti->minfilt = GR_TEXTUREFILTER_POINT_SAMPLED;
            break;
        case GL_LINEAR:
            ti->mmmode = GR_MIPMAP_DISABLE;
            ti->minfilt = GR_TEXTUREFILTER_BILINEAR;
            break;
        case GL_NEAREST_MIPMAP_NEAREST:
            ti->mmmode = GR_MIPMAP_NEAREST;
            ti->minfilt = GR_TEXTUREFILTER_POINT_SAMPLED;
            break;
        case GL_NEAREST_MIPMAP_LINEAR:
            ti->mmmode = GR_MIPMAP_NEAREST;
            ti->minfilt = GR_TEXTUREFILTER_BILINEAR;
            break;
        case GL_LINEAR_MIPMAP_LINEAR:
            ti->mmmode = GR_MIPMAP_NEAREST_DITHER;
            ti->minfilt = GR_TEXTUREFILTER_BILINEAR;
            break;
        default:
            break;
        }
        break;

    case GL_TEXTURE_MAG_FILTER:
        ti->maxfilt = fxfiltmap(param);
        break;

    case GL_TEXTURE_WRAP_S:
        ti->sclamp = fxclampmap(param);
        break;

    case GL_TEXTURE_WRAP_T:
        ti->tclamp = fxclampmap(param);
        break;

    case GL_TEXTURE_BORDER_COLOR:
        //not needed (?)
        break;
    default:
        break;
    }

    if (!fx->fastbind)
    {
        texbind(tex);
    }
}

static void texdel(gl_texture_object* tex)
{
    fx_texobj* ti = (fx_texobj*)tex->DriverData;

    if (!ti)
        return;

    tmFreeTexture(fx, tex);

    FXctx->FreeFunc(ti);
    tex->DriverData = NULL;
}

void texpalette(gl_texture_object* tex)
{
    int i;
    FxU32 r, g, b, a;
    fx_texobj* ti;
    GLubyte* tp;

    if (!FXctx->UsingSharedPalette && tex)
    {
        fx_context* fx = (fx_context*)FXctx->DriverCtx;

        if (!tex->DriverData)
        {
            tex->DriverData = alloctexobjdata();
            filltexobjdata(tex);
        }

        ti = (fx_texobj*)tex->DriverData;

        for (i = 0, tp = tex->Palette; i < 256; i++, tp += 4)
        {
            r = tp[0];
            g = tp[1];
            b = tp[2];
            a = tp[3];
            ti->palette.data[i] = (a<<24) | (r<<16) | (g<<8) | b;
        }

        fx->gl_palette[0] = NULL;
        fx->gl_palette[1] = NULL;
        if (!fx->fastbind)
        {
            texbind(tex);
        }
    }
    else if (FXctx->UsingSharedPalette)
    {
        GuTexPalette pal;

        for (i = 0, tp = FXctx->SharedPalette; i < 256; i++, tp += 4)
        {
            r = tp[0];
            g = tp[1];
            b = tp[2];
            a = tp[3];
            pal.data[i] = (a<<24) | (r<<16) | (g<<8) | b;
        }

        grTexDownloadTable(FX_TMU0, GR_TEXTABLE_PALETTE, &pal);
        if (fx->numTMUs > 1)
        {
            grTexDownloadTable(FX_TMU1, GR_TEXTABLE_PALETTE, &pal);
        }
    }
}

static void texalloc(
    gl_texture_object* tex, GLenum glformat, int w, int h)
{
    GrTextureFormat_t format;
    GrLOD_t l;
    GrAspectRatio_t aspectratio;
    fx_texobj* ti = (fx_texobj*)tex->DriverData;
    int wscale, hscale;

    assert(ti);

    tmTexInfo(w, h, &l, &aspectratio,
              &(ti->sscale), &(ti->tscale),
              &wscale, &hscale);

    format = texgetformat(FXctx, glformat);

    ti->width = w;
    ti->height = h;
    ti->info.smallLod = l;
    ti->info.largeLod = l;
    ti->info.aspectRatio = aspectratio;
    ti->info.format = format;
    ti->info.data = NULL;

    ti->minfilt = fxfiltmap(tex->Min);
    ti->maxfilt = fxfiltmap(tex->Mag);

    ti->sclamp = GR_TEXTURECLAMP_WRAP;
    ti->tclamp = GR_TEXTURECLAMP_WRAP;

    ti->mmmode = GR_MIPMAP_NEAREST;

    ti->levelsdefined = 0;
}

static void texbuildimagemap(
        gl_texture_object* tex,
        GLint internalFormat, GuTexPalette* palette,
        GLint levelsdefined, unsigned short **newsrc)
{
    unsigned short *src, *srccpy;
    unsigned short *sp;
    unsigned char r, g, b, a, *data, *srcb;
    unsigned char *spb;
    int x, y, w, h, wscale, hscale, idx;

    tmTexInfo(tex->Width, tex->Height,
              NULL, NULL, NULL, NULL,
              &wscale, &hscale);
    w = tex->Width * wscale;
    h = tex->Height * hscale;

    data = tex->Data;
    switch (internalFormat)
    {
    case 1:
    case GL_COLOR_INDEX:
        if (!(*newsrc))
        {
            if (!((*newsrc) = srccpy = src = (unsigned short*)FXctx->AllocFunc(sizeof(unsigned char)*w*h)))
            {
                //FIXME: bail
            }
        }
        else
            srccpy = src = (*newsrc);

        if (wscale == hscale == 1)
        {
            MEMCPY(src, data, h*w);
        }
        else
        {
            srcb = (unsigned char*)src;
            spb = srcb;
            for (y = 0; y < h; y++)
            {
                for (x = 0; x < w; x++, spb++)
                {
                    idx = (x/wscale + (y/hscale) * (w/wscale));
                    *spb = data[idx];
                }
            }
        }
        break;

    case 3:
    case GL_RGB:
        if (!(*newsrc))
        {
            if (!((*newsrc) = srccpy = src = (unsigned short*)FXctx->AllocFunc(sizeof(unsigned short)*w*h)))
            {
                //FIXME: bail
            }
        }
        else
            srccpy = src = (*newsrc);

        if (wscale == hscale == 1)
        {
            sp = src;
            for (y = 0, idx = 0; y < h; y++)
            {
                for (x = 0; x < w; x++, idx += 4, sp++)
                {
                    r = data[idx];
                    g = data[idx+1];
                    b = data[idx+2];

                    *sp = (unsigned short)
                        (((unsigned short)0xF8 & r) << (11-3)) |
                        (((unsigned short)0xFC & g) << (5-3+1)) |
                        (((unsigned short)0xF8 & b) >> 3);
                }
            }
        }
        else
        {
            sp = src;
            for (y = 0; y < h; y++)
            {
                for (x = 0; x < w; x++, sp++)
                {
                    idx = (x/wscale + (y/hscale) * (w/wscale)) * 4;
                    r = data[idx];
                    g = data[idx+1];
                    b = data[idx+2];

                    *sp = (unsigned short)
                        (((unsigned short)0xF8 & r) << (11-3)) |
                        (((unsigned short)0xFC & g) << (5-3+1)) |
                        (((unsigned short)0xF8 & b) >> 3);
                }
            }
        }
        break;

    case GL_RGBA16:
        if (!(*newsrc))
        {
            if (!((*newsrc) = srccpy = src = (unsigned short*)FXctx->AllocFunc(sizeof(unsigned short)*w*h)))
            {
                //FIXME: bail
            }
        }
        else
        {
            srccpy = src = (*newsrc);
        }

        if (wscale == hscale == 1)
        {
            memcpy(src, data, sizeof(unsigned short)*w*h);
        }
        else
        {
            sp = src;
            for (y = 0; y < h; y++)
            {
                for (x = 0; x < w; x++, sp++)
                {
                    idx = (x/wscale + (y/hscale) * (w/wscale))*2;
                    *sp = data[idx] + data[idx+1]*256;
                }
            }
        }
        break;

    case 4:
    case GL_RGBA:
        if (!(*newsrc))
        {
            if (!((*newsrc) = srccpy = src = (unsigned short*)FXctx->AllocFunc(sizeof(unsigned short)*w*h)))
            {
                //FIXME: bail
            }
        }
        else
            srccpy = src = (*newsrc);

        if (wscale == hscale == 1)
        {
            sp = src;
            for (y = 0, idx = 0; y < h; y++)
            {
                for (x = 0; x < w; x++, idx += 4, sp++)
                {
                    r = data[idx];
                    g = data[idx+1];
                    b = data[idx+2];
                    a = data[idx+3];

                    *sp = (unsigned short)
                        (((unsigned short)0xF0 & a) << 8) |
                        (((unsigned short)0xF0 & r) << 4) |
                         ((unsigned short)0xF0 & g) |
                        (((unsigned short)0xF0 & b) >> 4);
                }
            }
        }
        else
        {
            sp = src;
            for (y = 0; y < h; y++)
            {
                for (x = 0; x < w; x++, sp++)
                {
                    idx = (x/wscale + (y/hscale) * (w/wscale))*4;
                    r = data[idx];
                    g = data[idx+1];
                    b = data[idx+2];
                    a = data[idx+3];

                    *sp = (unsigned short)
                        (((unsigned short)0xF0 & a) << 8) |
                        (((unsigned short)0xF0 & r) << 4) |
                         ((unsigned short)0xF0 & g) |
                        (((unsigned short)0xF0 & b) >> 4);
                }
            }
        }
        break;

     default:
        ;//FIXME: report error
    }
}

void teximg(gl_texture_object* tex, GLint level, GLint internalFormat)
{
    GrLOD_t lodlev;
    fx_texobj* ti;

    if (!tex->DriverData)
    {
        tex->DriverData = alloctexobjdata();
        filltexobjdata(tex);
    }

    ti = (fx_texobj*)tex->DriverData;

    /* perhaps do a format test here */

    if ((!ti->valid) || (ti->info.format != texgetformat(FXctx, internalFormat)))
        texalloc(tex, tex->Format, tex->Width, tex->Height);

    if (ti->levelsdefined & (1<<level))
    {
        texbuildimagemap(tex, internalFormat, NULL, 0, &(ti->tmi.mipmaplevel[level]));

        tmReloadMipmap(fx, tex, level, ti->tmi.whichTMU);
    }
    else
    {
        tmUnloadTexture(fx, tex);

        ti->tmi.mipmaplevel[level] = NULL;

        texbuildimagemap(tex, internalFormat, NULL, 0, &(ti->tmi.mipmaplevel[level]));

        tmTexInfo(tex->Width, tex->Height, &lodlev, NULL, NULL, NULL, NULL, NULL);

        if (lodlev < ti->info.largeLod)
            ti->info.largeLod = lodlev;
        else if (lodlev > ti->info.smallLod)
            ti->info.smallLod = lodlev;

        ti->levelsdefined |= (1<<level);
        ti->valid = GL_TRUE;

        texbind(tex);
    }
}
