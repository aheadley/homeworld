#include <stdio.h>
#include "kgl.h"
#include "newtriangle.h"
#include "swdrv.h"

#define PIXEL_ADDRESS2(X,Y) (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))

extern GLint g_DepthMask;
extern GLboolean stipple;
extern GLboolean g_doAlpha;


// -----
// RGBA_REPLACE_565
// -----

void flat_textured_replace_affine_rgba_triangle_565(
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
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = ((t << twidth_log2) + s) << 2; \
        pRow[i] = FORM_RGB565(texture[pos+0], texture[pos+1], texture[pos+2]); \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = ((t << twidth_log2) + s) << 2; \
        pRow[i] = FORM_RGB565(texture[pos+0], texture[pos+1], texture[pos+2]); \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_z_replace_affine_rgba_triangle_565(
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
            pos = ((t << twidth_log2) + s) << 2; \
            pRow[i] = FORM_RGB565(texture[pos+0], texture[pos+1], texture[pos+2]); \
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
            pos = ((t << twidth_log2) + s) << 2; \
            pRow[i] = FORM_RGB565(texture[pos+0], texture[pos+1], texture[pos+2]); \
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
// RGBA_REPLACE_555
// -----

void flat_textured_replace_affine_rgba_triangle_555(
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
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = ((t << twidth_log2) + s) << 2; \
        pRow[i] = FORM_RGB555(texture[pos+0], texture[pos+1], texture[pos+2]); \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#define AFFINE_LOOP1 \
    for (Counter = WidthModLength; Counter >= 0; Counter--) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = ((t << twidth_log2) + s) << 2; \
        pRow[i] = FORM_RGB555(texture[pos+0], texture[pos+1], texture[pos+2]); \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_z_replace_affine_rgba_triangle_555(
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
            pos = ((t << twidth_log2) + s) << 2; \
            pRow[i] = FORM_RGB555(texture[pos+0], texture[pos+1], texture[pos+2]); \
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
            pos = ((t << twidth_log2) + s) << 2; \
            pRow[i] = FORM_RGB555(texture[pos+0], texture[pos+1], texture[pos+2]); \
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
// RGBA_MODULATE_565
// -----

void smooth_textured_modulate_affine_rgba_triangle_565(
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
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = ((t << twidth_log2) + s) << 2; \
        \
        pRow[i] = FORM_RGB565( \
                    PROD8(texture[pos+0], FixedToInt(ffr)), \
                    PROD8(texture[pos+1], FixedToInt(ffg)), \
                    PROD8(texture[pos+2], FixedToInt(ffb))); \
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
        pos = ((t << twidth_log2) + s) << 2; \
        \
        pRow[i] = FORM_RGB565( \
                    PROD8(texture[pos+0], FixedToInt(ffr)), \
                    PROD8(texture[pos+1], FixedToInt(ffg)), \
                    PROD8(texture[pos+2], FixedToInt(ffb))); \
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

void smooth_textured_z_modulate_affine_rgba_triangle_565(
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
            pos = ((t << twidth_log2) + s) << 2; \
            \
            pRow[i] = FORM_RGB565( \
                        PROD8(texture[pos+0], FixedToInt(ffr)), \
                        PROD8(texture[pos+1], FixedToInt(ffg)), \
                        PROD8(texture[pos+2], FixedToInt(ffb))); \
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
            pos = ((t << twidth_log2) + s) << 2; \
            \
            pRow[i] = FORM_RGB565( \
                        PROD8(texture[pos+0], FixedToInt(ffr)), \
                        PROD8(texture[pos+1], FixedToInt(ffg)), \
                        PROD8(texture[pos+2], FixedToInt(ffb))); \
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

void flat_textured_modulate_affine_rgba_triangle_565(
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
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = ((t << twidth_log2) + s) << 2; \
        \
        pRow[i] = FORM_RGB565( \
                    PROD8(texture[pos+0], fred), \
                    PROD8(texture[pos+1], fgreen), \
                    PROD8(texture[pos+2], fblue)); \
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
        pos = ((t << twidth_log2) + s) << 2; \
        \
        pRow[i] = FORM_RGB565( \
                    PROD8(texture[pos+0], fred), \
                    PROD8(texture[pos+1], fgreen), \
                    PROD8(texture[pos+2], fblue)); \
        \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_z_modulate_affine_rgba_triangle_565(
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
            pos = ((t << twidth_log2) + s) << 2; \
            \
            pRow[i] = FORM_RGB565( \
                        PROD8(texture[pos+0], fred), \
                        PROD8(texture[pos+1], fgreen), \
                        PROD8(texture[pos+2], fblue)); \
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
            pos = ((t << twidth_log2) + s) << 2; \
            \
            pRow[i] = FORM_RGB565( \
                        PROD8(texture[pos+0], fred), \
                        PROD8(texture[pos+1], fgreen), \
                        PROD8(texture[pos+2], fblue)); \
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
// RGBA_MODULATE_555
// -----

void smooth_textured_modulate_affine_rgba_triangle_555(
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
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = ((t << twidth_log2) + s) << 2; \
        \
        pRow[i] = FORM_RGB555( \
                    PROD8(texture[pos+0], FixedToInt(ffr)), \
                    PROD8(texture[pos+1], FixedToInt(ffg)), \
                    PROD8(texture[pos+2], FixedToInt(ffb))); \
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
        pos = ((t << twidth_log2) + s) << 2; \
        \
        pRow[i] = FORM_RGB555( \
                    PROD8(texture[pos+0], FixedToInt(ffr)), \
                    PROD8(texture[pos+1], FixedToInt(ffg)), \
                    PROD8(texture[pos+2], FixedToInt(ffb))); \
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

void smooth_textured_z_modulate_affine_rgba_triangle_555(
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
            pos = ((t << twidth_log2) + s) << 2; \
            \
            pRow[i] = FORM_RGB555( \
                        PROD8(texture[pos+0], FixedToInt(ffr)), \
                        PROD8(texture[pos+1], FixedToInt(ffg)), \
                        PROD8(texture[pos+2], FixedToInt(ffb))); \
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
            pos = ((t << twidth_log2) + s) << 2; \
            \
            pRow[i] = FORM_RGB555( \
                        PROD8(texture[pos+0], FixedToInt(ffr)), \
                        PROD8(texture[pos+1], FixedToInt(ffg)), \
                        PROD8(texture[pos+2], FixedToInt(ffb))); \
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

void flat_textured_modulate_affine_rgba_triangle_555(
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
    GLint smask = tex->WidthMask; \
    GLint tmask = tex->HeightMask; \
    GLint s, t, pos;

#define AFFINE_LOOP0 \
    for (Counter = 0; Counter < AFFINE_LENGTH; Counter++) \
    { \
        s = FixedToInt(S) & smask; \
        t = FixedToInt(T) & tmask; \
        pos = ((t << twidth_log2) + s) << 2; \
        \
        pRow[i] = FORM_RGB555( \
                    PROD8(texture[pos+0], fred), \
                    PROD8(texture[pos+1], fgreen), \
                    PROD8(texture[pos+2], fblue)); \
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
        pos = ((t << twidth_log2) + s) << 2; \
        \
        pRow[i] = FORM_RGB555( \
                    PROD8(texture[pos+0], fred), \
                    PROD8(texture[pos+1], fgreen), \
                    PROD8(texture[pos+2], fblue)); \
        \
        S += DeltaS; \
        T += DeltaT; \
        i++; \
    }

#include "newtritemp.h"
}

void flat_textured_z_modulate_affine_rgba_triangle_555(
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
            pos = ((t << twidth_log2) + s) << 2; \
            \
            pRow[i] = FORM_RGB555( \
                        PROD8(texture[pos+0], fred), \
                        PROD8(texture[pos+1], fgreen), \
                        PROD8(texture[pos+2], fblue)); \
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
            pos = ((t << twidth_log2) + s) << 2; \
            \
            pRow[i] = FORM_RGB555( \
                        PROD8(texture[pos+0], fred), \
                        PROD8(texture[pos+1], fgreen), \
                        PROD8(texture[pos+2], fblue)); \
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

