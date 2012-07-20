#include <stdio.h>
#include "kgl.h"
#include "newtriangle.h"
#include "swdrv.h"
#include "span.h"

#define PIXEL_ADDRESS2(X,Y) (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))

extern GLint g_DepthMask;
extern GLboolean stipple;
extern GLboolean g_doAlpha;

//general
void general_textured_replace_triangle(GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv);
void general_textured_modulate_triangle(GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv);

//general perspective
void general_textured_replace_perspective_triangle(GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv);
void general_textured_modulate_perspective_triangle(GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv);

//general affine
void general_textured_replace_affine_triangle(GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv);
void general_textured_modulate_affine_triangle(GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv);

//prelit
void flat_textured_prelit_triangle(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_prelit_triangle(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_prelit_triangle(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_z_prelit_triangle(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//565 alphatest
void rgba_flat_textured_z_replace_at_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_z_modulate_at_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_modulate_at_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//565 blended
void rgba_flat_textured_z_replace_blend_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_z_modulate_blend_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_modulate_blend_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//565 stippled
void rgba_flat_textured_z_replace_stipple_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_z_modulate_stipple_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_modulate_stipple_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555 blended
void rgba_flat_textured_z_replace_blend_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_z_modulate_blend_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_modulate_blend_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555 stippled
void rgba_flat_textured_z_replace_stipple_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_z_modulate_stipple_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_modulate_stipple_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555 alphatest
void rgba_flat_textured_z_replace_at_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_z_modulate_at_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_modulate_at_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//565 rgba
void rgba_flat_textured_replace_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_replace_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_modulate_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_z_modulate_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_modulate_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_modulate_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//565 palette
void flat_textured_replace_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_replace_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_modulate_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_z_modulate_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_modulate_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_modulate_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555 rgba
void rgba_flat_textured_replace_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_replace_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_modulate_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_smooth_textured_z_modulate_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_modulate_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void rgba_flat_textured_z_modulate_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555 palette
void flat_textured_replace_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_replace_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_modulate_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_z_modulate_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_modulate_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_modulate_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555/565 affine replace palette
void flat_textured_replace_affine_paletted_triangle(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_replace_affine_paletted_triangle(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//565 affine modulate palette
void smooth_textured_modulate_affine_paletted_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_z_modulate_affine_paletted_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_modulate_affine_paletted_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_modulate_affine_paletted_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555 affine modulate palette
void smooth_textured_modulate_affine_paletted_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_z_modulate_affine_paletted_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_modulate_affine_paletted_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_modulate_affine_paletted_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//565 affine replace rgba
void flat_textured_replace_affine_rgba_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_replace_affine_rgba_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555 affine replace rgba
void flat_textured_replace_affine_rgba_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_replace_affine_rgba_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//565 affine modulate rgba
void smooth_textured_modulate_affine_rgba_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_z_modulate_affine_rgba_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_modulate_affine_rgba_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_modulate_affine_rgba_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

//555 affine modulate rgba
void smooth_textured_modulate_affine_rgba_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_textured_z_modulate_affine_rgba_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_modulate_affine_rgba_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_textured_z_modulate_affine_rgba_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

typedef void (*newTexSpannerFunc)(GLcontext*, GLuint, GLuint, GLuint, GLuint);
static newTexSpannerFunc general_textured_triangle = NULL;
static newTexSpannerFunc general_perspective_textured_triangle = NULL;
static newTexSpannerFunc alphatest_triangle = NULL;
static newTexSpannerFunc blended_triangle = NULL;
static newTexSpannerFunc* new_tex_spanners;
static newTexSpannerFunc* new_ptex_spanners;

#define INC_DEPTH 1
#define INC_SMOOTH 2
#define INC_RGBA 4

static GLint triIndex;

//prelit
static newTexSpannerFunc tex_spanners_prelit_modulate[8] =
{
    flat_textured_prelit_triangle,
    flat_textured_z_prelit_triangle,
    smooth_textured_prelit_triangle,
    smooth_textured_z_prelit_triangle,
    rgba_flat_textured_modulate_triangle_565,
    rgba_flat_textured_z_modulate_triangle_565,
    rgba_smooth_textured_modulate_triangle_565,
    rgba_smooth_textured_z_modulate_triangle_565
};

//replace affine 565
static newTexSpannerFunc tex_spanners_replace_affine_565[8] =
{
    flat_textured_replace_affine_paletted_triangle,
    flat_textured_z_replace_affine_paletted_triangle,
    flat_textured_replace_affine_paletted_triangle,
    flat_textured_z_replace_affine_paletted_triangle,
    flat_textured_replace_affine_rgba_triangle_565,
    flat_textured_z_replace_affine_rgba_triangle_565,
    flat_textured_replace_affine_rgba_triangle_565,
    flat_textured_z_replace_affine_rgba_triangle_565
};

//replace affine 555
static newTexSpannerFunc tex_spanners_replace_affine_555[8] =
{
    flat_textured_replace_affine_paletted_triangle,
    flat_textured_z_replace_affine_paletted_triangle,
    flat_textured_replace_affine_paletted_triangle,
    flat_textured_z_replace_affine_paletted_triangle,
    flat_textured_replace_affine_rgba_triangle_555,
    flat_textured_z_replace_affine_rgba_triangle_555,
    flat_textured_replace_affine_rgba_triangle_555,
    flat_textured_z_replace_affine_rgba_triangle_555
};

//modulate affine 565
static newTexSpannerFunc tex_spanners_modulate_affine_565[8] =
{
    flat_textured_modulate_affine_paletted_triangle_565,
    flat_textured_z_modulate_affine_paletted_triangle_565,
    smooth_textured_modulate_affine_paletted_triangle_565,
    smooth_textured_z_modulate_affine_paletted_triangle_565,
    flat_textured_modulate_affine_rgba_triangle_565,
    flat_textured_z_modulate_affine_rgba_triangle_565,
    smooth_textured_modulate_affine_rgba_triangle_565,
    smooth_textured_z_modulate_affine_rgba_triangle_565
};

//modulate affine 555
static newTexSpannerFunc tex_spanners_modulate_affine_555[8] =
{
    flat_textured_modulate_affine_paletted_triangle_555,
    flat_textured_z_modulate_affine_paletted_triangle_555,
    smooth_textured_modulate_affine_paletted_triangle_555,
    smooth_textured_z_modulate_affine_paletted_triangle_555,
    flat_textured_modulate_affine_rgba_triangle_555,
    flat_textured_z_modulate_affine_rgba_triangle_555,
    smooth_textured_modulate_affine_rgba_triangle_555,
    smooth_textured_z_modulate_affine_rgba_triangle_555
};

//replace 565
static newTexSpannerFunc tex_spanners_replace_565[8] =
{
    flat_textured_replace_triangle_565,
    flat_textured_z_replace_triangle_565,
    flat_textured_replace_triangle_565,
    flat_textured_z_replace_triangle_565,
    rgba_flat_textured_replace_triangle_565,
    rgba_flat_textured_z_replace_triangle_565,
    rgba_flat_textured_replace_triangle_565,
    rgba_flat_textured_z_replace_triangle_565
};

//modulate 565
static newTexSpannerFunc tex_spanners_modulate_565[8] =
{
    flat_textured_modulate_triangle_565,
    flat_textured_z_modulate_triangle_565,
    smooth_textured_modulate_triangle_565,
    smooth_textured_z_modulate_triangle_565,
    rgba_flat_textured_modulate_triangle_565,
    rgba_flat_textured_z_modulate_triangle_565,
    rgba_smooth_textured_modulate_triangle_565,
    rgba_smooth_textured_z_modulate_triangle_565
};

//replace 555
static newTexSpannerFunc tex_spanners_replace_555[8] =
{
    flat_textured_replace_triangle_555,
    flat_textured_z_replace_triangle_555,
    flat_textured_replace_triangle_555,
    flat_textured_z_replace_triangle_555,
    rgba_flat_textured_replace_triangle_555,
    rgba_flat_textured_z_replace_triangle_555,
    rgba_flat_textured_replace_triangle_555,
    rgba_flat_textured_z_replace_triangle_555
};

//modulate 555
static newTexSpannerFunc tex_spanners_modulate_555[8] =
{
    flat_textured_modulate_triangle_555,
    flat_textured_z_modulate_triangle_555,
    smooth_textured_modulate_triangle_555,
    smooth_textured_z_modulate_triangle_555,
    rgba_flat_textured_modulate_triangle_555,
    rgba_flat_textured_z_modulate_triangle_555,
    rgba_smooth_textured_modulate_triangle_555,
    rgba_smooth_textured_z_modulate_triangle_555
};


// -----
// GENERAL
// -----

void general_textured_replace_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        GLdepth zspan[MAX_WIDTH]; \
        GLubyte cspan[4*MAX_WIDTH]; \
        GLfixed uv[2*MAX_WIDTH]; \
        \
        if (n > 0) \
        { \
            for (i = 0; i < n; i++) \
            { \
                zspan[i] = FixedToDepth(ffz); \
                uv[2*i+0] = ffs; \
                uv[2*i+1] = fft; \
                ffz += fdzdx; \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
            gl_write_texture_span(ctx, n, LEFT, Y, zspan, \
                                  uv, cspan); \
        } \
    }

#include "newtritemp.h"
}

void general_textured_modulate_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define SETUP_CODE \
    GLboolean flat_shade = (ctx->ShadeModel == GL_FLAT); \
    GLint r, g, b, a; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    if (flat_shade) \
    { \
        r = VB->Color[pv][0]; \
        g = VB->Color[pv][1]; \
        b = VB->Color[pv][2]; \
        a = VB->Color[pv][3]; \
    }

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        GLdepth zspan[MAX_WIDTH]; \
        GLubyte cspan[4*MAX_WIDTH]; \
        GLfixed uv[2*MAX_WIDTH]; \
        \
        if (n > 0) \
        { \
            if (flat_shade) \
            { \
                for (i = 0; i < n; i++) \
                { \
                    zspan[i] = FixedToDepth(ffz); \
                    cspan[4*i + 0] = r; \
                    cspan[4*i + 1] = g; \
                    cspan[4*i + 2] = b; \
                    cspan[4*i + 3] = a; \
                    uv[2*i+0] = ffs; \
                    uv[2*i+1] = fft; \
                    ffz += fdzdx; \
                    ffs += fdsdx; \
                    fft += fdtdx; \
                } \
            } \
            else \
            { \
                for (i = 0; i < n; i++) \
                { \
                    zspan[i] = FixedToDepth(ffz); \
                    cspan[4*i + 0] = FixedToInt(ffr); \
                    cspan[4*i + 1] = FixedToInt(ffg); \
                    cspan[4*i + 2] = FixedToInt(ffb); \
                    cspan[4*i + 3] = FixedToInt(ffa); \
                    uv[2*i+0] = ffs; \
                    uv[2*i+1] = fft; \
                    ffz += fdzdx; \
                    ffr += fdrdx; \
                    ffg += fdgdx; \
                    ffb += fdbdx; \
                    ffa += fdadx; \
                    ffs += fdsdx; \
                    fft += fdtdx; \
                } \
            } \
            gl_write_texture_span(ctx, n, LEFT, Y, zspan, \
                                  uv, cspan); \
        } \
    }

#include "newtritemp.h"
}

// -----
// PRELIT_5x5
// -----

void flat_textured_prelit_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLint illum = VB->Color[pv][0] >> ILLUM_SHIFT_DOWN; \
    GLushort* palette = &ctx->SharedIllumPalettes[illum << 8]; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void smooth_textured_prelit_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_I 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLushort* palette = ctx->SharedIllumPalettes; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t, illum; \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                illum = (FixedToInt(ffi) >> ILLUM_SHIFT_DOWN) << 8; \
                pRow[i] = palette[illum + texture[(t << twidth_log2) + s]]; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffi += fdidx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void flat_textured_z_prelit_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLint illum = VB->Color[pv][0] >> ILLUM_SHIFT_DOWN; \
    GLushort* palette = &ctx->SharedIllumPalettes[illum << 8]; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
                    zRow[i] = z; \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void smooth_textured_z_prelit_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_I 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLushort* palette = ctx->SharedIllumPalettes; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t, illum; \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    illum = (FixedToInt(ffi) >> ILLUM_SHIFT_DOWN) << 8; \
                    pRow[i] = palette[illum + texture[(t << twidth_log2) + s]]; \
                    zRow[i] = z; \
                } \
                ffi += fdidx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

// -----
// REPLACE_565
// -----

void flat_textured_replace_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLushort* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette16 : ((sw_texobj*)(ctx->TexBoundObject)->DriverData)->Palette; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void flat_textured_z_replace_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLushort* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette16 : ((sw_texobj*)(ctx->TexBoundObject)->DriverData)->Palette; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
                zRow[i] = z; \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

// -----
// MODULATE_565
// -----

void smooth_textured_modulate_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLubyte* palette = ctx->TexBoundObject->Palette; \
    GLubyte* ppal; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    if (ctx->UsingSharedPalette) palette = ctx->SharedPalette;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = (t << twidth_log2) + s; \
                ppal = &palette[4*texture[pos]]; \
                \
                pRow[i] = FORM_RGB565( \
                            PROD8(ppal[0], FixedToInt(ffr)), \
                            PROD8(ppal[1], FixedToInt(ffg)), \
                            PROD8(ppal[2], FixedToInt(ffb))); \
                \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void smooth_textured_z_modulate_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_Z 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLubyte* palette = ctx->TexBoundObject->Palette; \
    GLubyte* ppal; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    if (ctx->UsingSharedPalette) palette = ctx->SharedPalette;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = (t << twidth_log2) + s; \
                ppal = &palette[4*texture[pos]]; \
                \
                pRow[i] = FORM_RGB565( \
                            PROD8(ppal[0], FixedToInt(ffr)), \
                            PROD8(ppal[1], FixedToInt(ffg)), \
                            PROD8(ppal[2], FixedToInt(ffb))); \
                \
                zRow[i] = z; \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
        } \
    }

#include "newtritemp.h"
}

void flat_textured_modulate_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLubyte* palette = ctx->TexBoundObject->Palette; \
    GLubyte* ppal; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    if (ctx->UsingSharedPalette) palette = ctx->SharedPalette;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = (t << twidth_log2) + s; \
                ppal = &palette[4*texture[pos]]; \
                \
                pRow[i] = FORM_RGB565( \
                            PROD8(ppal[0], fred), \
                            PROD8(ppal[1], fgreen), \
                            PROD8(ppal[2], fblue)); \
                \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void flat_textured_z_modulate_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLubyte* palette = ctx->TexBoundObject->Palette; \
    GLubyte* ppal; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    if (ctx->UsingSharedPalette) palette = ctx->SharedPalette;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = (t << twidth_log2) + s; \
                ppal = &palette[4*texture[pos]]; \
                \
                pRow[i] = FORM_RGB565( \
                            PROD8(ppal[0], fred), \
                            PROD8(ppal[1], fgreen), \
                            PROD8(ppal[2], fblue)); \
                \
                zRow[i] = z; \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

// -----
// REPLACE_555
// -----

void flat_textured_replace_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLushort* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette16 : ((sw_texobj*)(ctx->TexBoundObject)->DriverData)->Palette; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void flat_textured_z_replace_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLushort* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette16 : ((sw_texobj*)(ctx->TexBoundObject)->DriverData)->Palette; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
                zRow[i] = z; \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

// -----
// MODULATE_555
// -----

void smooth_textured_modulate_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLubyte* palette = ctx->TexBoundObject->Palette; \
    GLubyte* ppal; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    if (ctx->UsingSharedPalette) palette = ctx->SharedPalette;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = (t << twidth_log2) + s; \
                ppal = &palette[4*texture[pos]]; \
                \
                pRow[i] = FORM_RGB555( \
                            PROD8(ppal[0], FixedToInt(ffr)), \
                            PROD8(ppal[1], FixedToInt(ffg)), \
                            PROD8(ppal[2], FixedToInt(ffb))); \
                \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void smooth_textured_z_modulate_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_Z 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLubyte* palette = ctx->TexBoundObject->Palette; \
    GLubyte* ppal; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    if (ctx->UsingSharedPalette) palette = ctx->SharedPalette;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = (t << twidth_log2) + s; \
                ppal = &palette[4*texture[pos]]; \
                \
                pRow[i] = FORM_RGB555( \
                            PROD8(ppal[0], FixedToInt(ffr)), \
                            PROD8(ppal[1], FixedToInt(ffg)), \
                            PROD8(ppal[2], FixedToInt(ffb))); \
                \
                zRow[i] = z; \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
        } \
    }

#include "newtritemp.h"
}

void flat_textured_modulate_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLubyte* palette = ctx->TexBoundObject->Palette; \
    GLubyte* ppal; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    if (ctx->UsingSharedPalette) palette = ctx->SharedPalette;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = (t << twidth_log2) + s; \
                ppal = &palette[4*texture[pos]]; \
                \
                pRow[i] = FORM_RGB555( \
                            PROD8(ppal[0], fred), \
                            PROD8(ppal[1], fgreen), \
                            PROD8(ppal[2], fblue)); \
                \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void flat_textured_z_modulate_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_ST 1

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLubyte* palette = ctx->TexBoundObject->Palette; \
    GLubyte* ppal; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    if (ctx->UsingSharedPalette) palette = ctx->SharedPalette;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = (t << twidth_log2) + s; \
                ppal = &palette[4*texture[pos]]; \
                \
                pRow[i] = FORM_RGB555( \
                            PROD8(ppal[0], fred), \
                            PROD8(ppal[1], fgreen), \
                            PROD8(ppal[2], fblue)); \
                \
                zRow[i] = z; \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void init_new_tex(GLcontext* ctx)
{
    if (ctx->Buffer.PixelType == GL_RGB565)
    {
        if (ctx->TexEnvMode == GL_REPLACE)
        {
            new_tex_spanners = tex_spanners_replace_565;
            new_ptex_spanners = tex_spanners_replace_affine_565;

            general_textured_triangle = general_textured_replace_triangle;
            general_perspective_textured_triangle = general_textured_replace_affine_triangle;

            alphatest_triangle = rgba_flat_textured_z_replace_at_triangle_565;
            if (ctx->PolygonStipple)
            {
                blended_triangle = rgba_flat_textured_z_replace_stipple_triangle_565;
            }
            else
            {
                blended_triangle = rgba_flat_textured_z_replace_blend_triangle_565;
            }
        }
        else
        {
            if (ctx->UsingLitPalette)
            {
                new_tex_spanners = tex_spanners_prelit_modulate;
            }
            else
            {
                new_tex_spanners = tex_spanners_modulate_565;
            }
            new_ptex_spanners = tex_spanners_modulate_affine_565;

            general_textured_triangle = general_textured_modulate_triangle;
            general_perspective_textured_triangle = general_textured_modulate_affine_triangle;

            if (ctx->ShadeModel == GL_SMOOTH)
            {
                alphatest_triangle = rgba_smooth_textured_z_modulate_at_triangle_565;
                if (ctx->PolygonStipple)
                {
                    blended_triangle = rgba_smooth_textured_z_modulate_stipple_triangle_565;
                }
                else
                {
                    blended_triangle = rgba_smooth_textured_z_modulate_blend_triangle_565;
                }
            }
            else
            {
                alphatest_triangle = rgba_flat_textured_z_modulate_at_triangle_565;
                if (ctx->PolygonStipple)
                {
                    blended_triangle = rgba_flat_textured_z_modulate_stipple_triangle_565;
                }
                else
                {
                    blended_triangle = rgba_flat_textured_z_modulate_blend_triangle_565;
                }
            }
        }
    }
    else
    {
        if (ctx->TexEnvMode == GL_REPLACE)
        {
            new_tex_spanners = tex_spanners_replace_555;
            new_ptex_spanners = tex_spanners_replace_affine_555;

            general_textured_triangle = general_textured_replace_triangle;
            general_perspective_textured_triangle = general_textured_replace_affine_triangle;

            alphatest_triangle = rgba_flat_textured_z_replace_at_triangle_555;
            if (ctx->PolygonStipple)
            {
                blended_triangle = rgba_flat_textured_z_replace_stipple_triangle_555;
            }
            else
            {
                blended_triangle = rgba_flat_textured_z_replace_blend_triangle_555;
            }
        }
        else
        {
            if (ctx->UsingLitPalette)
            {
                new_tex_spanners = tex_spanners_prelit_modulate;
            }
            else
            {
                new_tex_spanners = tex_spanners_modulate_555;
            }
            new_ptex_spanners = tex_spanners_modulate_affine_555;

            general_textured_triangle = general_textured_modulate_triangle;
            general_perspective_textured_triangle = general_textured_modulate_affine_triangle;

            if (ctx->ShadeModel == GL_SMOOTH)
            {
                alphatest_triangle = rgba_smooth_textured_z_modulate_at_triangle_555;
                if (ctx->PolygonStipple)
                {
                    blended_triangle = rgba_smooth_textured_z_modulate_stipple_triangle_555;
                }
                else
                {
                    blended_triangle = rgba_smooth_textured_z_modulate_blend_triangle_555;
                }
            }
            else
            {
                alphatest_triangle = rgba_flat_textured_z_modulate_at_triangle_555;
                if (ctx->PolygonStipple)
                {
                    blended_triangle = rgba_flat_textured_z_modulate_stipple_triangle_555;
                }
                else
                {
                    blended_triangle = rgba_flat_textured_z_modulate_blend_triangle_555;
                }
            }
        }
    }

    triIndex = 0;
    if (ctx->DepthTest) triIndex += INC_DEPTH;
    if (ctx->ShadeModel == GL_SMOOTH) triIndex += INC_SMOOTH;

    stipple = ctx->PolygonStipple;
}

void general_new_textured_triangle(GLcontext* ctx, GLuint vl[], GLuint pv)
{
    general_textured_triangle(ctx, vl[0], vl[1], vl[2], pv);
}

void general_perspective_new_textured_triangle(GLcontext* ctx, GLuint vl[], GLuint pv)
{
    general_perspective_textured_triangle(ctx, vl[0], vl[1], vl[2], pv);
}

void new_perspective_textured_triangle(GLcontext* ctx, GLuint vl[], GLuint pv)
{
    GLint index = triIndex;
    if (ctx->TexBoundObject->Format != GL_COLOR_INDEX)
    {
        index += INC_RGBA;
    }
    new_ptex_spanners[index](ctx, vl[0], vl[1], vl[2], pv);
}

void new_textured_triangle(GLcontext* ctx, GLuint vl[], GLuint pv)
{
    if (ctx->AlphaTest)
    {
        alphatest_triangle(ctx, vl[0], vl[1], vl[2], pv);
    }
    else if (ctx->Blend)
    {
        if (ctx->TexBoundObject->Format == GL_COLOR_INDEX)
        {
            general_textured_triangle(ctx, vl[0], vl[1], vl[2], pv);
        }
        else
        {
            blended_triangle(ctx, vl[0], vl[1], vl[2], pv);
        }
    }
    else
    {
        GLint index = triIndex;
        if (ctx->TexBoundObject->Format != GL_COLOR_INDEX)
        {
            index += INC_RGBA;
        }
        new_tex_spanners[index](ctx, vl[0], vl[1], vl[2], pv);
    }
}
