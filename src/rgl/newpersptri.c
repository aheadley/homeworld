#include <stdio.h>
#include "kgl.h"
#include "newtriangle.h"
#include "swdrv.h"
#include "span.h"

#define PIXEL_ADDRESS2(X,Y) (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))

extern GLint g_DepthMask;
extern GLboolean stipple;
extern GLboolean g_doAlpha;


// -----
// GENERAL_AFFINE
// -----

void general_textured_replace_affine_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define SETUP_CODE \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLdepth zspan[MAX_WIDTH]; \
    GLubyte cspan[4*MAX_WIDTH]; \
    GLfixed uv[2*MAX_WIDTH];

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        zspan[i] = FixedToDepth(ffz); \
        uv[2*i+0] = S; \
        uv[2*i+1] = T; \
        ffz += fdzdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        zspan[i] = FixedToDepth(ffz); \
        uv[2*i+0] = S; \
        uv[2*i+1] = T; \
        ffz += fdzdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFTER_LOOP gl_write_texture_span(ctx, Width, left, iy, zspan, uv, cspan);

#include "newtritemp.h"
}

void general_textured_modulate_affine_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define SETUP_CODE \
    GLboolean flat_shade = (ctx->ShadeModel == GL_FLAT); \
    GLint r, g, b, a; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLdepth zspan[MAX_WIDTH]; \
    GLubyte cspan[4*MAX_WIDTH]; \
    GLfixed uv[2*MAX_WIDTH]; \
    if (flat_shade) \
    { \
        r = VB->Color[pv][0]; \
        g = VB->Color[pv][1]; \
        b = VB->Color[pv][2]; \
        a = VB->Color[pv][3]; \
    }

#define AFFINE_LOOP0 \
    if (flat_shade) \
    { \
        for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
        { \
            zspan[i] = FixedToDepth(ffz); \
            cspan[4*i + 0] = r; \
            cspan[4*i + 1] = g; \
            cspan[4*i + 2] = b; \
            cspan[4*i + 3] = a; \
            uv[2*i+0] = S; \
            uv[2*i+1] = T; \
            ffz += fdzdx; \
            S += DeltaS; \
            T += DeltaT; \
            i++; \
        } \
    } \
    else \
    { \
        for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
        { \
            zspan[i] = FixedToDepth(ffz); \
            cspan[4*i + 0] = FixedToInt(ffr); \
            cspan[4*i + 1] = FixedToInt(ffg); \
            cspan[4*i + 2] = FixedToInt(ffb); \
            cspan[4*i + 3] = FixedToInt(ffa); \
            uv[2*i+0] = S; \
            uv[2*i+1] = T; \
            ffz += fdzdx; \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
            ffa += fdadx; \
            S += DeltaS; \
            T += DeltaT; \
            i++; \
        } \
    } \

#define AFFINE_LOOP1 \
    if (flat_shade) \
    { \
        for (Counter = WidthModLength; Counter >= 0; Counter--) \
        { \
            zspan[i] = FixedToDepth(ffz); \
            cspan[4*i + 0] = r; \
            cspan[4*i + 1] = g; \
            cspan[4*i + 2] = b; \
            cspan[4*i + 3] = a; \
            uv[2*i+0] = S; \
            uv[2*i+1] = T; \
            ffz += fdzdx; \
            S += DeltaS; \
            T += DeltaT; \
            i++; \
        } \
    } \
    else \
    { \
        for (Counter = WidthModLength; Counter >= 0; Counter--) \
        { \
            zspan[i] = FixedToDepth(ffz); \
            cspan[4*i + 0] = FixedToInt(ffr); \
            cspan[4*i + 1] = FixedToInt(ffg); \
            cspan[4*i + 2] = FixedToInt(ffb); \
            cspan[4*i + 3] = FixedToInt(ffa); \
            uv[2*i+0] = S; \
            uv[2*i+1] = T; \
            ffz += fdzdx; \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
            ffa += fdadx; \
            S += DeltaS; \
            T += DeltaT; \
            i++; \
        } \
    } \

#define AFTER_LOOP gl_write_texture_span(ctx, Width, left, iy, zspan, uv, cspan);

#include "newtritemp.h"
}

// -----
// GENERAL_PERSPECTIVE
// -----

void general_textured_replace_perspective_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_STW 1

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
        GLfloat OneOverW; \
        \
        if (n > 0) \
        { \
            for (i = 0; i < n; i++) \
            { \
                zspan[i] = FixedToDepth(ffz); \
                OneOverW = 1.0f / ww; \
                ffz += fdzdx; \
                uv[2*i+0] = FAST_TO_FIXED((ss*OneOverW) * S_SCALE); \
                uv[2*i+1] = FAST_TO_FIXED((tt*OneOverW) * T_SCALE); \
                ss += dsdx; \
                tt += dtdx; \
                ww += dwdx; \
            } \
            gl_write_texture_span(ctx, n, LEFT, Y, zspan, \
                                  uv, cspan); \
        } \
    }

#include "newtritemp.h"
}

void general_textured_modulate_perspective_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INTERP_STW 1

#define S_SCALE twidth
#define T_SCALE theight

#define SETUP_CODE \
    GLboolean flat_shade = (ctx->ShadeModel == GL_FLAT); \
    GLint r, g, b, a; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLfloat OneOverW; \
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
                    OneOverW = 1.0f / ww; \
                    cspan[4*i + 0] = r; \
                    cspan[4*i + 1] = g; \
                    cspan[4*i + 2] = b; \
                    cspan[4*i + 3] = a; \
                    uv[2*i+0] = FAST_TO_FIXED((ss*OneOverW) * S_SCALE); \
                    uv[2*i+1] = FAST_TO_FIXED((tt*OneOverW) * T_SCALE); \
                    ffz += fdzdx; \
                    ss += dsdx; \
                    tt += dtdx; \
                    ww += dwdx; \
                } \
            } \
            else \
            { \
                for (i = 0; i < n; i++) \
                { \
                    zspan[i] = FixedToDepth(ffz); \
                    OneOverW = 1.0f / ww; \
                    cspan[4*i + 0] = FixedToInt(ffr); \
                    cspan[4*i + 1] = FixedToInt(ffg); \
                    cspan[4*i + 2] = FixedToInt(ffb); \
                    cspan[4*i + 3] = FixedToInt(ffa); \
                    uv[2*i+0] = FAST_TO_FIXED((ss*OneOverW) * S_SCALE); \
                    uv[2*i+1] = FAST_TO_FIXED((tt*OneOverW) * T_SCALE); \
                    ffz += fdzdx; \
                    ffr += fdrdx; \
                    ffg += fdgdx; \
                    ffb += fdbdx; \
                    ffa += fdadx; \
                    ss += dsdx; \
                    tt += dtdx; \
                    ww += dwdx; \
                } \
            } \
            gl_write_texture_span(ctx, n, LEFT, Y, zspan, \
                                  uv, cspan); \
        } \
    }

#include "newtritemp.h"
}

// -----
// CI_REPLACE
// -----

void flat_textured_replace_affine_paletted_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLushort* palette = (ctx->UsingSharedPalette) \
        ? ctx->SharedPalette16 \
        : ((sw_texobj*)(ctx->TexBoundObject)->DriverData)->Palette; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_z_replace_affine_paletted_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLushort* palette = (ctx->UsingSharedPalette) \
        ? ctx->SharedPalette16 \
        : ((sw_texobj*)(ctx->TexBoundObject)->DriverData)->Palette; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pRow[i] = palette[texture[(t << twidth_log2) + s]]; \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

// -----
// CI_MODULATE_565
// -----

void smooth_textured_modulate_affine_paletted_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLubyte* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette : tex->Palette; \
    GLubyte* ppal; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = (t << twidth_log2) + s; \
        ppal = &palette[texture[pos] << 2]; \
        \
        pRow[i] = FORM_RGB565( \
                    PROD8(ppal[0], FixedToInt(ffr)), \
                    PROD8(ppal[1], FixedToInt(ffg)), \
                    PROD8(ppal[2], FixedToInt(ffb))); \
        \
        ffr += fdrdx; \
        ffg += fdgdx; \
        ffb += fdbdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = (t << twidth_log2) + s; \
        ppal = &palette[texture[pos] << 2]; \
        \
        pRow[i] = FORM_RGB565( \
                    PROD8(ppal[0], FixedToInt(ffr)), \
                    PROD8(ppal[1], FixedToInt(ffg)), \
                    PROD8(ppal[2], FixedToInt(ffb))); \
        \
        ffr += fdrdx; \
        ffg += fdgdx; \
        ffb += fdbdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void smooth_textured_z_modulate_affine_paletted_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLubyte* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette : tex->Palette; \
    GLubyte* ppal; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pos = (t << twidth_log2) + s; \
            ppal = &palette[texture[pos] << 2]; \
            \
            pRow[i] = FORM_RGB565( \
                        PROD8(ppal[0], FixedToInt(ffr)), \
                        PROD8(ppal[1], FixedToInt(ffg)), \
                        PROD8(ppal[2], FixedToInt(ffb))); \
            \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        ffr += fdrdx; \
        ffg += fdgdx; \
        ffb += fdbdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pos = (t << twidth_log2) + s; \
            ppal = &palette[texture[pos] << 2]; \
            \
            pRow[i] = FORM_RGB565( \
                        PROD8(ppal[0], FixedToInt(ffr)), \
                        PROD8(ppal[1], FixedToInt(ffg)), \
                        PROD8(ppal[2], FixedToInt(ffb))); \
            \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        ffr += fdrdx; \
        ffg += fdgdx; \
        ffb += fdbdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_modulate_affine_paletted_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLubyte* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette : tex->Palette; \
    GLubyte* ppal; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = (t << twidth_log2) + s; \
        ppal = &palette[texture[pos] << 2]; \
        \
        pRow[i] = FORM_RGB565( \
                    PROD8(ppal[0], fred), \
                    PROD8(ppal[1], fgreen), \
                    PROD8(ppal[2], fblue)); \
        \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = (t << twidth_log2) + s; \
        ppal = &palette[texture[pos] << 2]; \
        \
        pRow[i] = FORM_RGB565( \
                    PROD8(ppal[0], fred), \
                    PROD8(ppal[1], fgreen), \
                    PROD8(ppal[2], fblue)); \
        \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_z_modulate_affine_paletted_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLubyte* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette : tex->Palette; \
    GLubyte* ppal; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pos = (t << twidth_log2) + s; \
            ppal = &palette[texture[pos] << 2]; \
            \
            pRow[i] = FORM_RGB565( \
                        PROD8(ppal[0], fred), \
                        PROD8(ppal[1], fgreen), \
                        PROD8(ppal[2], fblue)); \
            \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pos = (t << twidth_log2) + s; \
            ppal = &palette[texture[pos] << 2]; \
            \
            pRow[i] = FORM_RGB565( \
                        PROD8(ppal[0], fred), \
                        PROD8(ppal[1], fgreen), \
                        PROD8(ppal[2], fblue)); \
            \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

// -----
// CI_MODULATE_555
// -----

void smooth_textured_modulate_affine_paletted_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLubyte* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette : tex->Palette; \
    GLubyte* ppal; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = (t << twidth_log2) + s; \
        ppal = &palette[texture[pos] << 2]; \
        \
        pRow[i] = FORM_RGB555( \
                    PROD8(ppal[0], FixedToInt(ffr)), \
                    PROD8(ppal[1], FixedToInt(ffg)), \
                    PROD8(ppal[2], FixedToInt(ffb))); \
        \
        ffr += fdrdx; \
        ffg += fdgdx; \
        ffb += fdbdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = (t << twidth_log2) + s; \
        ppal = &palette[texture[pos] << 2]; \
        \
        pRow[i] = FORM_RGB555( \
                    PROD8(ppal[0], FixedToInt(ffr)), \
                    PROD8(ppal[1], FixedToInt(ffg)), \
                    PROD8(ppal[2], FixedToInt(ffb))); \
        \
        ffr += fdrdx; \
        ffg += fdgdx; \
        ffb += fdbdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void smooth_textured_z_modulate_affine_paletted_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLubyte* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette : tex->Palette; \
    GLubyte* ppal; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pos = (t << twidth_log2) + s; \
            ppal = &palette[texture[pos] << 2]; \
            \
            pRow[i] = FORM_RGB555( \
                        PROD8(ppal[0], FixedToInt(ffr)), \
                        PROD8(ppal[1], FixedToInt(ffg)), \
                        PROD8(ppal[2], FixedToInt(ffb))); \
            \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        ffr += fdrdx; \
        ffg += fdgdx; \
        ffb += fdbdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pos = (t << twidth_log2) + s; \
            ppal = &palette[texture[pos] << 2]; \
            \
            pRow[i] = FORM_RGB555( \
                        PROD8(ppal[0], FixedToInt(ffr)), \
                        PROD8(ppal[1], FixedToInt(ffg)), \
                        PROD8(ppal[2], FixedToInt(ffb))); \
            \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        ffr += fdrdx; \
        ffg += fdgdx; \
        ffb += fdbdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_modulate_affine_paletted_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLubyte* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette : tex->Palette; \
    GLubyte* ppal; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = (t << twidth_log2) + s; \
        ppal = &palette[texture[pos] << 2]; \
        \
        pRow[i] = FORM_RGB555( \
                    PROD8(ppal[0], fred), \
                    PROD8(ppal[1], fgreen), \
                    PROD8(ppal[2], fblue)); \
        \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = (t << twidth_log2) + s; \
        ppal = &palette[texture[pos] << 2]; \
        \
        pRow[i] = FORM_RGB555( \
                    PROD8(ppal[0], fred), \
                    PROD8(ppal[1], fgreen), \
                    PROD8(ppal[2], fblue)); \
        \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_z_modulate_affine_paletted_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_STW 1
#define INTERP_STW_AFFINE 1

#define AFFINE_LENGTH 32
#define AFFINE_SHIFT 5

#define S_SCALE twidth
#define T_SCALE theight

#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    gl_texture_object* tex = ctx->TexBoundObject; \
    GLfloat twidth = (GLfloat)tex->Width; \
    GLfloat theight = (GLfloat)tex->Height; \
    GLint twidth_log2 = tex->WidthLog2; \
    GLubyte* texture = tex->Data; \
    GLubyte* palette = (ctx->UsingSharedPalette) ? ctx->SharedPalette : tex->Palette; \
    GLubyte* ppal; \
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pos = (t << twidth_log2) + s; \
            ppal = &palette[texture[pos] << 2]; \
            \
            pRow[i] = FORM_RGB555( \
                        PROD8(ppal[0], fred), \
                        PROD8(ppal[1], fgreen), \
                        PROD8(ppal[2], fblue)); \
            \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        GLdepth z = FixedToDepth(ffz); \
        if (z < zRow[i]) \
        { \
            s = FixedToInt(S) & smask; \
            t = FixedToInt(T) & tmask; \
            pos = (t << twidth_log2) + s; \
            ppal = &palette[texture[pos] << 2]; \
            \
            pRow[i] = FORM_RGB555( \
                        PROD8(ppal[0], fred), \
                        PROD8(ppal[1], fgreen), \
                        PROD8(ppal[2], fblue)); \
            \
            zRow[i] = z; \
        } \
        ffz += fdzdx; \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

