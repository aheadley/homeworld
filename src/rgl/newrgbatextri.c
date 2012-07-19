#include <stdio.h>
#include "kgl.h"
#include "newtriangle.h"
#include "swdrv.h"

#define PIXEL_ADDRESS2(X,Y) (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))

extern GLint g_DepthMask;
extern GLboolean stipple;
extern GLboolean g_doAlpha;

// -----
// REPLACE_565
// -----

void rgba_flat_textured_replace_triangle_565(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint pos; \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pos = 4 * ((t << twidth_log2) + s); \
                pRow[i] = FORM_RGB565(texture[pos+0], texture[pos+1], texture[pos+2]); \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_replace_triangle_565(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint pos; \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pos = 4 * ((t << twidth_log2) + s); \
                pRow[i] = FORM_RGB565(texture[pos+0], texture[pos+1], texture[pos+2]); \
                zRow[i] = z; \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_replace_at_triangle_565(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    GLubyte alphaTestRef = ctx->AlphaByteRef;

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                if (texture[pos+3] > alphaTestRef) \
                { \
                    pRow[i] = FORM_RGB565(texture[pos+0], texture[pos+1], \
                                          texture[pos+2]); \
                    zRow[i] = z; \
                } \
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

void rgba_smooth_textured_modulate_triangle_565(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                pRow[i] = FORM_RGB565( \
                            PROD8(texture[pos+0], FixedToInt(ffr)), \
                            PROD8(texture[pos+1], FixedToInt(ffg)), \
                            PROD8(texture[pos+2], FixedToInt(ffb))); \
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

void rgba_smooth_textured_z_modulate_triangle_565(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                pRow[i] = FORM_RGB565( \
                            PROD8(texture[pos+0], FixedToInt(ffr)), \
                            PROD8(texture[pos+1], FixedToInt(ffg)), \
                            PROD8(texture[pos+2], FixedToInt(ffb))); \
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

void rgba_flat_textured_modulate_triangle_565(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                pRow[i] = FORM_RGB565( \
                            PROD8(texture[pos+0], fred), \
                            PROD8(texture[pos+1], fgreen), \
                            PROD8(texture[pos+2], fblue)); \
                \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_modulate_triangle_565(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                pRow[i] = FORM_RGB565( \
                            PROD8(texture[pos+0], fred), \
                            PROD8(texture[pos+1], fgreen), \
                            PROD8(texture[pos+2], fblue)); \
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

void rgba_flat_textured_z_modulate_at_triangle_565(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    GLubyte alphaTestRef = ctx->AlphaByteRef;

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                if (texture[pos+3] > alphaTestRef) \
                { \
                    pRow[i] = FORM_RGB565( \
                                PROD8(texture[pos+0], fred), \
                                PROD8(texture[pos+1], fgreen), \
                                PROD8(texture[pos+2], fblue)); \
                    \
                    zRow[i] = z; \
                } \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void rgba_smooth_textured_z_modulate_at_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_ALPHA 1
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    GLubyte alphaTestRef = ctx->AlphaByteRef;

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                if (PROD8(texture[pos+3], FixedToInt(ffa)) > alphaTestRef) \
                { \
                    pRow[i] = FORM_RGB565( \
                                PROD8(texture[pos+0], FixedToInt(ffr)), \
                                PROD8(texture[pos+1], FixedToInt(ffg)), \
                                PROD8(texture[pos+2], FixedToInt(ffb))); \
                    \
                    zRow[i] = z; \
                } \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
            ffa += fdadx; \
        } \
    }

#include "newtritemp.h"
}

// -----
// REPLACE_555
// -----

void rgba_flat_textured_replace_triangle_555(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint pos; \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pos = 4 * ((t << twidth_log2) + s); \
                pRow[i] = FORM_RGB555(texture[pos+0], texture[pos+1], texture[pos+2]); \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_replace_triangle_555(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint s, t; \
        GLint pos; \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                s = FixedToInt(ffs) & smask; \
                t = FixedToInt(fft) & tmask; \
                pos = 4 * ((t << twidth_log2) + s); \
                pRow[i] = FORM_RGB555(texture[pos+0], texture[pos+1], texture[pos+2]); \
                zRow[i] = z; \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_replace_at_triangle_555(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    GLubyte alphaTestRef = ctx->AlphaByteRef;

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                if (texture[pos+3] > alphaTestRef) \
                { \
                    pRow[i] = FORM_RGB555(texture[pos+0], texture[pos+1], \
                                          texture[pos+2]); \
                    zRow[i] = z; \
                } \
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

void rgba_smooth_textured_modulate_triangle_555(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                pRow[i] = FORM_RGB555( \
                            PROD8(texture[pos+0], FixedToInt(ffr)), \
                            PROD8(texture[pos+1], FixedToInt(ffg)), \
                            PROD8(texture[pos+2], FixedToInt(ffb))); \
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

void rgba_smooth_textured_z_modulate_triangle_555(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                pRow[i] = FORM_RGB555( \
                            PROD8(texture[pos+0], FixedToInt(ffr)), \
                            PROD8(texture[pos+1], FixedToInt(ffg)), \
                            PROD8(texture[pos+2], FixedToInt(ffb))); \
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

void rgba_flat_textured_modulate_triangle_555(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        if (len > 0) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLint s = FixedToInt(ffs) & smask; \
                GLint t = FixedToInt(fft) & tmask; \
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                pRow[i] = FORM_RGB555( \
                            PROD8(texture[pos+0], fred), \
                            PROD8(texture[pos+1], fgreen), \
                            PROD8(texture[pos+2], fblue)); \
                \
                ffs += fdsdx; \
                fft += fdtdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_modulate_triangle_555(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                pRow[i] = FORM_RGB555( \
                            PROD8(texture[pos+0], fred), \
                            PROD8(texture[pos+1], fgreen), \
                            PROD8(texture[pos+2], fblue)); \
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

void rgba_flat_textured_z_modulate_at_triangle_555(
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    GLubyte alphaTestRef = ctx->AlphaByteRef;

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                if (texture[pos+3] > alphaTestRef) \
                { \
                    pRow[i] = FORM_RGB555( \
                                PROD8(texture[pos+0], fred), \
                                PROD8(texture[pos+1], fgreen), \
                                PROD8(texture[pos+2], fblue)); \
                    \
                    zRow[i] = z; \
                } \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void rgba_smooth_textured_z_modulate_at_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_ALPHA 1
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
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    GLubyte alphaTestRef = ctx->AlphaByteRef;

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
                GLint pos = 4 * ((t << twidth_log2) + s); \
                \
                if (PROD8(texture[pos+3], FixedToInt(ffa)) > alphaTestRef) \
                { \
                    pRow[i] = FORM_RGB555( \
                                PROD8(texture[pos+0], FixedToInt(ffr)), \
                                PROD8(texture[pos+1], FixedToInt(ffg)), \
                                PROD8(texture[pos+2], FixedToInt(ffb))); \
                    \
                    zRow[i] = z; \
                } \
            } \
            ffs += fdsdx; \
            fft += fdtdx; \
            ffz += fdzdx; \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
            ffa += fdadx; \
        } \
    }

#include "newtritemp.h"
}
