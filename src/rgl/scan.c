#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "kgl.h"
#include "span.h"
#include "swdrv.h"
#include "scan.h"

#include "stipple.h"

#define TEX_PALETTE_H "tex_palette_fn.h"
#define TEX_PALETTE_BLEND_H "tex_palette_fn_blend.h"
#define COLORTABLE    0

#define CRED444(c)   ((c) >> 8)
#define CGREEN444(c) (((c) >> 4) & 0xf)
#define CBLUE444(c)  ((c) & 0xf)

#define CRED555(c)   ((c) >> 10)
#define CGREEN555(c) (((c) >> 5) & 0x1f)
#define CBLUE555(c)  ((c) & 0x1f)

#define CRED565(c)   ((c) >> 11)
#define CGREEN565(c) ((((c) >> 5) & 0x3f) >> 1)
#define CBLUE565(c)  ((c) & 0x1f)

#define MOD555(A,B)  colorTable[(_reduce555to444[A] << 12) | _reduce555to444[B]]
#define MODB555(A,B) colorTable[((A) << 12) | _reduce555to444[B]]
#define MOD565(A,B)  colorTable[(_reduce565to444[A] << 12) | _reduce565to444[B]]
#define MODB565(A,B) colorTable[((A) << 12) | _reduce565to444[B]]

#define MOD0(A,B)    colorTable[((A) << 12) | B]

#define FIVESIXFIVE   0
#define FIVEFIVEFIVE  0
#define TWENTYFOURBIT 0

GLubyte*  alphaTable;
GLushort* colorTable;
GLushort* _reduce555to444;
GLushort* _reduce565to444;

void (*gl_suba_texture_span_rgb_z)() = NULL;
void (*gl_suba_texture_span_rgb)() = NULL;
void (*gl_suba_texture_span_z)() = NULL;
void (*gl_suba_texture_span)() = NULL;
void (*gl_suba_texture_span_rgb_z_p)() = NULL;
void (*gl_suba_texture_span_rgb_p)() = NULL;
void (*gl_suba_texture_span_z_p)() = NULL;
void (*gl_suba_texture_span_p)() = NULL;

GLboolean texModulateMode;
GLboolean paletted;
GLboolean adjusted;
GLboolean stipple;

extern GLint g_DepthMask;

GLint* scrMultByte = NULL;

static unsigned short FPUCW;
static unsigned short OldFPUCW;

GLboolean g_doAlpha = GL_FALSE;
GLboolean g_doBias = GL_FALSE;
GLboolean g_doAlphaTest = GL_FALSE;

GLubyte g_redBias = 0;
GLubyte g_greenBias = 0;
GLubyte g_blueBias = 0;

GLboolean _alphatest(GLcontext* ctx, GLubyte alpha)
{
    switch (ctx->AlphaFunc)
    {
    case GL_LESS:
        if (alpha < ctx->AlphaByteRef)
            return GL_TRUE;
        else
            return GL_FALSE;
    case GL_GREATER:
        if (alpha > ctx->AlphaByteRef)
            return GL_TRUE;
        else
            return GL_FALSE;
    }
    return GL_TRUE;
}

void loFPU()
{
#if defined (_MSC_VER)
    _asm
    {
        fstcw   [OldFPUCW]
        mov     ax,[OldFPUCW]
        and     eax,0xFF
        mov     [FPUCW],ax
        fldcw   [FPUCW]
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "   fstcw   %0\n"
        "   movw    %0, %%ax\n"
        "   andl    $0xFF, %%eax\n"
        "   movw    %%ax, %1\n"
        "   fldcw   %1\n"
        :
        : "m" (OldFPUCW), "m" (FPUCW)
        : "eax" );
#endif
}

void restoreFPU()
{
#if defined (_MSC_VER)
    _asm
    {
        fldcw [OldFPUCW]
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "   fldcw %0\n"
        :
        : "m" (OldFPUCW) );
#endif
}

static void adjust_color_to_black(
    GLubyte* red, GLubyte* green, GLubyte* blue, GLfloat adjust)
{
    GLint badjust = FAST_TO_INT((1.0f - adjust) * 255.0f);

    *red = (GLubyte)(((GLint)(*red) * badjust) >> 8);
    *green = (GLubyte)(((GLint)(*green) * badjust) >> 8);
    *blue = (GLubyte)(((GLint)(*blue) * badjust) >> 8);
}

void init_spanners(GLcontext* ctx)
{
}

#if COLORTABLE
static GLboolean _colorSetup = GL_FALSE;
#else
static GLboolean _colorSetup = GL_TRUE;
#endif

static void setupColor(GLcontext* ctx)
{
    GLint   source, dest;
    GLubyte sred, sgreen, sblue;
    GLubyte dred, dgreen, dblue;

    colorTable = (GLushort*)malloc(2*4096*4096);

    for (source = 0; source < 4096; source++)
    {
        for (dest = 0; dest < 4096; dest++)
        {
            sred = CRED444(source) << 4;
            sgreen = CGREEN444(source) << 4;
            sblue = CBLUE444(source) << 4;

            dred = CRED444(dest) << 4;
            dgreen = CGREEN444(dest) << 4;
            dblue = CBLUE444(dest) << 4;

            if (ctx->Buffer.PixelType == GL_RGB555)
            {
                colorTable[4096*source + dest] =
                    FORM_RGB555(PROD8(sred, dred),
                                PROD8(sgreen, dgreen),
                                PROD8(sblue, dblue));
            }
            else
            {
                colorTable[4096*source + dest] =
                    FORM_RGB565(PROD8(sred, dred),
                                PROD8(sgreen, dgreen),
                                PROD8(sblue, dblue));
            }
        }
    }

    if (ctx->Buffer.PixelType == GL_RGB555)
    {
        _reduce555to444 = (GLushort*)malloc(2*32768);

        for (source = 0; source < 32768; source++)
        {
            sred = CRED555(source) >> 1;
            sgreen = CGREEN555(source) >> 1;
            sblue = CBLUE555(source) >> 1;
            _reduce555to444[source] = FORM_RGB444(sred, sgreen, sblue);
        }
    }
    else
    {
        _reduce565to444 = (GLushort*)malloc(2*65536);

        for (source = 0; source < 65536; source++)
        {
            sred = CRED565(source) >> 1;
            sgreen = CGREEN565(source) >> 1;
            sblue = CBLUE565(source) >> 1;
            _reduce565to444[source] = FORM_RGB444(sred, sgreen, sblue);
        }
    }
}

void perspective_draw_triangle(
    GLcontext* ctx, GLuint vlist[], GLuint pv,
    void (*triangle_func)())
{
#if SPAN_STATS
    if (!span_stats)
    {
        init_span_stats();
    }
#endif
#if COLORTABLE
    if (!_colorSetup)
    {
        setupColor(ctx);
        _colorSetup = GL_TRUE;
    }
#endif
    if (scrMultByte == NULL)
    {
        scrMultByte = ctx->scrMultByte;
    }

    texModulateMode = (ctx->TexEnvMode == GL_MODULATE);
    stipple = ctx->PolygonStipple;
    paletted = (ctx->TexBoundObject->Format == GL_COLOR_INDEX);
    adjusted = (ctx->LightingAdjust != 0.0f);

    triangle_func(ctx, vlist, pv);
}
