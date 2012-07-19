#include <stdio.h>
#include "kgl.h"
#include "newtriangle.h"
#include "swdrv.h"
#include "stipple.h"

#define PIXEL_ADDRESS2(X,Y) (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))

extern GLint g_DepthMask;
extern GLboolean stipple;
extern GLboolean g_doAlpha;

extern GLubyte* alphaTable;

#define CRED555(c)   ((c) >> 10)
#define CGREEN555(c) (((c) >> 5) & 0x1f)
#define CBLUE555(c)  ((c) & 0x1f)

#define CRED565(c)   ((c) >> 11)
#define CGREEN565(c) ((((c) >> 5) & 0x3f) >> 1)
#define CBLUE565(c)  ((c) & 0x1f)

// -----
// REPLACE_565
// -----

void rgba_flat_textured_z_replace_blend_triangle_565(
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
    GLint sred, sgreen, sblue, salpha; \
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (texture[pos+3] & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = texture[pos+0]; \
                        sgreen = texture[pos+1]; \
                        sblue = texture[pos+2]; \
                        pRow[i] = FORM_RGB565( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED565(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN565(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE565(pRow[i])]); \
                    } \
                    \
                    zRow[i] = z; \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (texture[pos+3] & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = texture[pos+0]; \
                        sgreen = texture[pos+1]; \
                        sblue = texture[pos+2]; \
                        pRow[i] = FORM_RGB565( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED565(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN565(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE565(pRow[i])]); \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_replace_stipple_triangle_565(
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
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    if (stipple[(texture[pos+3] & 0xf0) + ((LEFT+i) & 3)]) \
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
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    if (stipple[(texture[pos+3] & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB565(texture[pos+0], texture[pos+1], \
                                              texture[pos+2]); \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

// -----
// MODULATE_565
// -----

void rgba_flat_textured_z_modulate_blend_triangle_565(
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
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    GLubyte falpha = VB->Color[pv][3]; \
    GLint sred, sgreen, sblue, salpha; \
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (PROD8(texture[pos+3], falpha) & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = PROD8(texture[pos+0], fred); \
                        sgreen = PROD8(texture[pos+1], fgreen); \
                        sblue = PROD8(texture[pos+2], fblue); \
                        pRow[i] = FORM_RGB565( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED565(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN565(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE565(pRow[i])]); \
                    } \
                    \
                    zRow[i] = z; \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (PROD8(texture[pos+3], falpha) & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = PROD8(texture[pos+0], fred); \
                        sgreen = PROD8(texture[pos+1], fgreen); \
                        sblue = PROD8(texture[pos+2], fblue); \
                        pRow[i] = FORM_RGB565( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED565(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN565(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE565(pRow[i])]); \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_modulate_stipple_triangle_565(
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
    GLubyte falpha = VB->Color[pv][3]; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        GLint salpha; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    salpha = PROD8(falpha, texture[pos+3]); \
                    if (stipple[(salpha & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB565( \
                                    PROD8(texture[pos+0], fred), \
                                    PROD8(texture[pos+1], fgreen), \
                                    PROD8(texture[pos+2], fblue)); \
                        zRow[i] = z; \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    salpha = PROD8(falpha, texture[pos+3]); \
                    if (stipple[(salpha & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB565( \
                                    PROD8(texture[pos+0], fred), \
                                    PROD8(texture[pos+1], fgreen), \
                                    PROD8(texture[pos+2], fblue)); \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_smooth_textured_z_modulate_blend_triangle_565(
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
    GLint sred, sgreen, sblue, salpha; \
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (PROD8(texture[pos+3], FixedToInt(ffa)) & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = PROD8(texture[pos+0], FixedToInt(ffr)); \
                        sgreen = PROD8(texture[pos+1], FixedToInt(ffg)); \
                        sblue = PROD8(texture[pos+2], FixedToInt(ffb)); \
                        pRow[i] = FORM_RGB565( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED565(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN565(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE565(pRow[i])]); \
                    } \
                    \
                    zRow[i] = z; \
                } \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (PROD8(texture[pos+3], FixedToInt(ffa)) & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = PROD8(texture[pos+0], FixedToInt(ffr)); \
                        sgreen = PROD8(texture[pos+1], FixedToInt(ffg)); \
                        sblue = PROD8(texture[pos+2], FixedToInt(ffb)); \
                        pRow[i] = FORM_RGB565( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED565(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN565(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE565(pRow[i])]); \
                    } \
                } \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_smooth_textured_z_modulate_stipple_triangle_565(
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
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        GLint salpha; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    salpha = PROD8(texture[pos+3], FixedToInt(ffa)); \
                    if (stipple[(salpha & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB565( \
                                    PROD8(texture[pos+0], FixedToInt(ffr)), \
                                    PROD8(texture[pos+1], FixedToInt(ffg)), \
                                    PROD8(texture[pos+2], FixedToInt(ffb))); \
                        zRow[i] = z; \
                    } \
                } \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    salpha = PROD8(texture[pos+3], FixedToInt(ffa)); \
                    if (stipple[(salpha & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB565( \
                                    PROD8(texture[pos+0], FixedToInt(ffr)), \
                                    PROD8(texture[pos+1], FixedToInt(ffg)), \
                                    PROD8(texture[pos+2], FixedToInt(ffb))); \
                    } \
                } \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

// -----
// REPLACE_555
// -----

void rgba_flat_textured_z_replace_blend_triangle_555(
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
    GLint sred, sgreen, sblue, salpha; \
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (texture[pos+3] & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = texture[pos+0]; \
                        sgreen = texture[pos+1]; \
                        sblue = texture[pos+2]; \
                        pRow[i] = FORM_RGB555( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED555(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN555(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE555(pRow[i])]); \
                    } \
                    \
                    zRow[i] = z; \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (texture[pos+3] & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = texture[pos+0]; \
                        sgreen = texture[pos+1]; \
                        sblue = texture[pos+2]; \
                        pRow[i] = FORM_RGB555( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED555(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN555(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE555(pRow[i])]); \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_replace_stipple_triangle_555(
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
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    if (stipple[(texture[pos+3] & 0xf0) + ((LEFT+i) & 3)]) \
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
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    if (stipple[(texture[pos+3] & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB555(texture[pos+0], texture[pos+1], \
                                              texture[pos+2]); \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

// -----
// MODULATE_555
// -----

void rgba_flat_textured_z_modulate_blend_triangle_555(
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
    GLubyte fred = VB->Color[pv][0]; \
    GLubyte fgreen = VB->Color[pv][1]; \
    GLubyte fblue = VB->Color[pv][2]; \
    GLubyte falpha = VB->Color[pv][3]; \
    GLint sred, sgreen, sblue, salpha; \
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (PROD8(texture[pos+3], falpha) & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = PROD8(texture[pos+0], fred); \
                        sgreen = PROD8(texture[pos+1], fgreen); \
                        sblue = PROD8(texture[pos+2], fblue); \
                        pRow[i] = FORM_RGB555( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED555(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN555(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE555(pRow[i])]); \
                    } \
                    \
                    zRow[i] = z; \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (PROD8(texture[pos+3], falpha) & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = PROD8(texture[pos+0], fred); \
                        sgreen = PROD8(texture[pos+1], fgreen); \
                        sblue = PROD8(texture[pos+2], fblue); \
                        pRow[i] = FORM_RGB555( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED555(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN555(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE555(pRow[i])]); \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_flat_textured_z_modulate_stipple_triangle_555(
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
    GLubyte falpha = VB->Color[pv][3]; \
    GLfloat twidth = (GLfloat)ctx->TexBoundObject->Width; \
    GLfloat theight = (GLfloat)ctx->TexBoundObject->Height; \
    GLint twidth_log2 = ctx->TexBoundObject->WidthLog2; \
    GLubyte* texture = ctx->TexBoundObject->Data; \
    GLint smask = ctx->TexBoundObject->WidthMask; \
    GLint tmask = ctx->TexBoundObject->HeightMask; \
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        GLint salpha; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    salpha = PROD8(falpha, texture[pos+3]); \
                    if (stipple[(salpha & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB555( \
                                    PROD8(texture[pos+0], fred), \
                                    PROD8(texture[pos+1], fgreen), \
                                    PROD8(texture[pos+2], fblue)); \
                        zRow[i] = z; \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    salpha = PROD8(falpha, texture[pos+3]); \
                    if (stipple[(salpha & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB555( \
                                    PROD8(texture[pos+0], fred), \
                                    PROD8(texture[pos+1], fgreen), \
                                    PROD8(texture[pos+2], fblue)); \
                    } \
                } \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_smooth_textured_z_modulate_blend_triangle_555(
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
    GLint sred, sgreen, sblue, salpha; \
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (PROD8(texture[pos+3], FixedToInt(ffa)) & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = PROD8(texture[pos+0], FixedToInt(ffr)); \
                        sgreen = PROD8(texture[pos+1], FixedToInt(ffg)); \
                        sblue = PROD8(texture[pos+2], FixedToInt(ffb)); \
                        pRow[i] = FORM_RGB555( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED555(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN555(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE555(pRow[i])]); \
                    } \
                    \
                    zRow[i] = z; \
                } \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    \
                    salpha = (PROD8(texture[pos+3], FixedToInt(ffa)) & 0xf8) << 7; \
                    if (salpha) \
                    { \
                        sred = PROD8(texture[pos+0], FixedToInt(ffr)); \
                        sgreen = PROD8(texture[pos+1], FixedToInt(ffg)); \
                        sblue = PROD8(texture[pos+2], FixedToInt(ffb)); \
                        pRow[i] = FORM_RGB555( \
                                    alphaTable[salpha + ((sred&0xf8)<<2) + CRED555(pRow[i])], \
                                    alphaTable[salpha + ((sgreen&0xf8)<<2) + CGREEN555(pRow[i])], \
                                    alphaTable[salpha + ((sblue&0xf8)<<2) + CBLUE555(pRow[i])]); \
                    } \
                } \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

void rgba_smooth_textured_z_modulate_stipple_triangle_555(
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
    GLint s, t, pos;

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        GLint salpha; \
        \
        if (ctx->DepthWrite) \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    salpha = PROD8(texture[pos+3], FixedToInt(ffa)); \
                    if (stipple[(salpha & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB555( \
                                    PROD8(texture[pos+0], FixedToInt(ffr)), \
                                    PROD8(texture[pos+1], FixedToInt(ffg)), \
                                    PROD8(texture[pos+2], FixedToInt(ffb))); \
                        zRow[i] = z; \
                    } \
                } \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
        else \
        { \
            for (i = 0; i < len; i++) \
            { \
                GLdepth z = FixedToDepth(ffz); \
                if (z < zRow[i]) \
                { \
                    s = FixedToInt(ffs) & smask; \
                    t = FixedToInt(fft) & tmask; \
                    pos = ((t << twidth_log2) + s) << 2; \
                    salpha = PROD8(texture[pos+3], FixedToInt(ffa)); \
                    if (stipple[(salpha & 0xf0) + ((LEFT+i) & 3)]) \
                    { \
                        pRow[i] = FORM_RGB555( \
                                    PROD8(texture[pos+0], FixedToInt(ffr)), \
                                    PROD8(texture[pos+1], FixedToInt(ffg)), \
                                    PROD8(texture[pos+2], FixedToInt(ffb))); \
                    } \
                } \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
                ffs += fdsdx; \
                fft += fdtdx; \
                ffz += fdzdx; \
            } \
        } \
    }

#include "newtritemp.h"
}

