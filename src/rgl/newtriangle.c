#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kgl.h"
#include "newtriangle.h"
#include "stipple.h"
#include "span.h"

#define FixedFloor(X)   ((X) & FIXED_INT_MASK)
#define SignedFloatToFixed(X) FloatToFixed(X)
#define FixedToDepth(X) X

static GLint* scrMult;

extern GLint g_DepthMask;
extern GLboolean stipple;
extern GLboolean g_doAlpha;


//4x4 dither matrix
GLubyte _dither4x4[16] =
{
    0, 4, 1, 5,
    6, 2, 7, 3,
    1, 5, 0, 4,
    7, 3, 6, 2,
};

//FrameBuffer.Width x 4 of dither pattern
GLuint* _ditherScreen = NULL;
GLubyte* _stippleScreen[4] = {NULL,NULL,NULL,NULL};

void flat_rgba_z_triangle_general(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_rgb_z_triangle_general(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_rgba_triangle_general(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_rgb_triangle_general(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgba_z_triangle_general(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgba_triangle_general(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgb_z_triangle_general(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgb_triangle_general(GLcontext*, GLuint, GLuint, GLuint, GLuint);

void flat_rgba_zz_stippled_triangle(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgba_zz_stippled_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgba_zz_stippled_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

void flat_rgb_z_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_rgb_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgb_z_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgb_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void dithered_smooth_rgb_triangle_565(GLcontext*, GLuint, GLuint, GLuint, GLuint);

void flat_rgb_z_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void flat_rgb_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgb_z_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void smooth_rgb_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);
void dithered_smooth_rgb_triangle_555(GLcontext*, GLuint, GLuint, GLuint, GLuint);

/*-----------------------------------------------------------------------------
    Name        : ditherStartup
    Description : initializes _ditherScreen with an expansion of _dither4x4
    Inputs      : ctx - the context
    Outputs     : _ditherScreen is allocated and filled
    Return      :
----------------------------------------------------------------------------*/
void ditherStartup(GLcontext* ctx)
{
    GLuint* dp;
    GLint y, x;

    if (_ditherScreen != NULL)
    {
        free(_ditherScreen);
    }

    _ditherScreen = (GLuint*)malloc(sizeof(GLuint)*ctx->Buffer.Width*4);

    dp = _ditherScreen;
    for (y = 0; y < 4; y++)
    {
        for (x = 0; x < ctx->Buffer.Width; x++, dp++)
        {
            *dp = (GLuint)_dither4x4[(y<<2) + (x&3)] << 12;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : stippleStartup
    Description : initializes _stippleScreen with an expansion of alpha_stipple
    Inputs      : ctx - the context
    Outputs     : _stippleScreen is allocated and filled
    Return      :
----------------------------------------------------------------------------*/
void stippleStartup(GLcontext* ctx)
{
    GLubyte* sp;
    GLint y, x;

    for (y = 0; y < 4; y++)
    {
        if (_stippleScreen[y] != NULL)
        {
            free(_stippleScreen[y]);
        }
        _stippleScreen[y] = (GLubyte*)malloc(ctx->Buffer.Width);
    }

    for (y = 0; y < 4; y++)
    {
        sp = _stippleScreen[y];
        for (x = 0; x < ctx->Buffer.Width; x++, sp++)
        {
            *sp = alpha_stipple[(y << 2) + x & 3];
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : ditherShutdown
    Description : frees memory allocated by ditherStartup
    Inputs      :
    Outputs     : _ditherScreen is freed and set to NULL
    Return      :
----------------------------------------------------------------------------*/
void ditherShutdown()
{
    if (_ditherScreen != NULL)
    {
        free(_ditherScreen);
        _ditherScreen = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : stippleShutdown
    Description : frees memory allocated by stippleStartup
    Inputs      :
    Outputs     : _stippleScreen is freed and set to NULL
    Return      :
----------------------------------------------------------------------------*/
void stippleShutdown()
{
    GLint y;

    for (y = 0; y < 4; y++)
    {
        if (_stippleScreen[y] != NULL)
        {
            free(_stippleScreen[y]);
            _stippleScreen[y] = NULL;
        }
    }
}


/*
 * Render a flat-shaded RGBA depth-buffered triangle.
 */
void flat_rgba_z_triangle_general(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1

#define SETUP_CODE \
    if (ctx->DriverFuncs.set_monocolor != NULL) \
    { \
        ctx->DriverFuncs.set_monocolor( \
            ctx, \
            VB->Color[pv][0], VB->Color[pv][1], \
            VB->Color[pv][2], VB->Color[pv][3]); \
    }

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        GLdepth zspan[MAX_WIDTH]; \
        if (n > 0) \
        { \
            for (i = 0; i < n; i++) \
            { \
                zspan[i] = FixedToDepth(ffz); \
                ffz += fdzdx; \
            } \
            gl_write_monocolor_span(ctx, n, LEFT, Y, zspan, \
                                    VB->Color[pv][0], VB->Color[pv][1], \
                                    VB->Color[pv][2], VB->Color[pv][3], \
                                    GL_POLYGON); \
        } \
    }

#include "newtritemp.h"
}

/*
 * Render a flat-shaded RGB depth-buffered triangle.
 */
void flat_rgb_z_triangle_general(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1

#define SETUP_CODE \
    if (ctx->DriverFuncs.set_monocolor != NULL) \
    { \
        ctx->DriverFuncs.set_monocolor( \
            ctx, \
            VB->Color[pv][0], VB->Color[pv][1], \
            VB->Color[pv][2], VB->Color[pv][3]); \
    }

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        GLdepth zspan[MAX_WIDTH]; \
        if (n > 0) \
        { \
            for (i = 0; i < n; i++) \
            { \
                zspan[i] = FixedToDepth(ffz); \
                ffz += fdzdx; \
            } \
            gl_write_monocolor_span(ctx, n, LEFT, Y, zspan, \
                                    VB->Color[pv][0], VB->Color[pv][1], \
                                    VB->Color[pv][2], VB->Color[pv][3], \
                                    GL_POLYGON); \
        } \
    }

#include "newtritemp.h"
}

void flat_rgb_z_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1

#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLushort p = FORM_RGB565(VB->Color[pv][0], VB->Color[pv][1], \
                             VB->Color[pv][2]);

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                pRow[i] = p; \
                zRow[i] = z; \
            } \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void flat_rgb_z_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1

#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLushort p = FORM_RGB555(VB->Color[pv][0], VB->Color[pv][1], \
                             VB->Color[pv][2]);

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                pRow[i] = p; \
                zRow[i] = z; \
            } \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

/*
 * Render a flat-shaded RGBA triangle.
 */
void flat_rgba_triangle_general(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define SETUP_CODE \
    if (ctx->DriverFuncs.set_monocolor != NULL) \
    { \
        ctx->DriverFuncs.set_monocolor( \
            ctx, \
            VB->Color[pv][0], VB->Color[pv][1], \
            VB->Color[pv][2], VB->Color[pv][3]); \
    }

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        if (n > 0) \
        { \
            gl_write_monocolor_span(ctx, n, LEFT, Y, NULL, \
                                    VB->Color[pv][0], VB->Color[pv][1], \
                                    VB->Color[pv][2], VB->Color[pv][3], \
                                    GL_POLYGON); \
        } \
    }

#include "newtritemp.h"
}

/*
 * Render a flat-shaded RGB triangle.
 */
void flat_rgb_triangle_general(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define SETUP_CODE \
    if (ctx->DriverFuncs.set_monocolor != NULL) \
    { \
        ctx->DriverFuncs.set_monocolor( \
            ctx, \
            VB->Color[pv][0], VB->Color[pv][1], \
            VB->Color[pv][2], VB->Color[pv][3]); \
    }

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        if (n > 0) \
        { \
            gl_write_monocolor_span(ctx, n, LEFT, Y, NULL, \
                                    VB->Color[pv][0], VB->Color[pv][1], \
                                    VB->Color[pv][2], VB->Color[pv][3], \
                                    GL_POLYGON); \
        } \
    }

#include "newtritemp.h"
}

void flat_rgb_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLushort p = FORM_RGB565(VB->Color[pv][0], VB->Color[pv][1], \
                             VB->Color[pv][2]);

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            pRow[i] = p; \
        } \
    }

#include "newtritemp.h"
}

void flat_rgb_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLushort p = FORM_RGB555(VB->Color[pv][0], VB->Color[pv][1], \
                             VB->Color[pv][2]);

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            pRow[i] = p; \
        } \
    }

#include "newtritemp.h"
}

/*
 * Render a smooth-shaded RGBA depth-buffered triangle.
 */
void smooth_rgba_z_triangle_general(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        GLdepth zspan[MAX_WIDTH]; \
        GLubyte cspan[4*MAX_WIDTH]; \
        if (n > 0) \
        { \
            for (i = 0; i < n; i++) \
            { \
                zspan[i] = FixedToDepth(ffz); \
                cspan[4*i + 0] = FixedToInt(ffr); \
                cspan[4*i + 1] = FixedToInt(ffg); \
                cspan[4*i + 2] = FixedToInt(ffb); \
                cspan[4*i + 3] = FixedToInt(ffa); \
                ffz += fdzdx; \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
            } \
            gl_write_4color_span(ctx, n, LEFT, Y, zspan, cspan, GL_POLYGON); \
        } \
    }

#include "newtritemp.h"
}

void flat_rgba_zz_stippled_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_ALPHA 1

#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define SETUP_CODE \
    GLushort p = (ctx->Buffer.PixelType == GL_RGB565) \
                 ? FORM_RGB565(VB->Color[pv][0], VB->Color[pv][1], VB->Color[pv][2]) \
                 : FORM_RGB555(VB->Color[pv][0], VB->Color[pv][1], VB->Color[pv][2]);

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                if (stipple[(FixedToInt(ffa) & 0xf0) + ((LEFT+i) & 3)]) \
                { \
                    pRow[i] = p; \
                } \
            } \
            ffa += fdadx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void smooth_rgba_zz_stippled_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1

#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                if (stipple[(FixedToInt(ffa) & 0xf0) + ((LEFT+i) & 3)]) \
                { \
                    pRow[i] = FORM_RGB565(FixedToInt(ffr), FixedToInt(ffg), \
                                          FixedToInt(ffb)); \
                } \
            } \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
            ffa += fdadx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void smooth_rgba_zz_stippled_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1

#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLint yind = ((Y) & 3) << 2; \
        GLubyte* stipple = alpha_stipple + yind; \
        \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                if (stipple[(FixedToInt(ffa) & 0xf0) + ((LEFT+i) & 3)]) \
                { \
                    pRow[i] = FORM_RGB555(FixedToInt(ffr), FixedToInt(ffg), \
                                          FixedToInt(ffb)); \
                } \
            } \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
            ffa += fdadx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

/*
 * Render a smooth-shaded RGBA triangle.
 */
void smooth_rgba_triangle_general(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_ALPHA 1

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        GLubyte cspan[4*MAX_WIDTH]; \
        if (n > 0) \
        { \
            for (i = 0; i < n; i++) \
            { \
                cspan[4*i + 0] = FixedToInt(ffr); \
                cspan[4*i + 1] = FixedToInt(ffg); \
                cspan[4*i + 2] = FixedToInt(ffb); \
                cspan[4*i + 3] = FixedToInt(ffa); \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
                ffa += fdadx; \
            } \
            gl_write_4color_span(ctx, n, LEFT, Y, NULL, cspan, GL_POLYGON); \
        } \
    }

#include "newtritemp.h"
}

/*
 * Render a smooth-shaded RGB depth-buffered triangle.
 */
void smooth_rgb_z_triangle_general(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        GLdepth zspan[MAX_WIDTH]; \
        GLubyte cspan[4*MAX_WIDTH]; \
        if (n > 0) \
        { \
            for (i = 0; i < n; i++) \
            { \
                zspan[i] = FixedToDepth(ffz); \
                cspan[4*i + 0] = FixedToInt(ffr); \
                cspan[4*i + 1] = FixedToInt(ffg); \
                cspan[4*i + 2] = FixedToInt(ffb); \
                ffz += fdzdx; \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
            } \
            gl_write_4color_span(ctx, n, LEFT, Y, zspan, cspan, GL_POLYGON); \
        } \
    }

#include "newtritemp.h"
}

void smooth_rgb_z_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1

#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                pRow[i] = FORM_RGB565(FixedToInt(ffr), FixedToInt(ffg), \
                                      FixedToInt(ffb)); \
                zRow[i] = z; \
            } \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

void smooth_rgb_z_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1

#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            GLdepth z = FixedToDepth(ffz); \
            if (z < zRow[i]) \
            { \
                pRow[i] = FORM_RGB555(FixedToInt(ffr), FixedToInt(ffg), \
                                      FixedToInt(ffb)); \
                zRow[i] = z; \
            } \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
            ffz += fdzdx; \
        } \
    }

#include "newtritemp.h"
}

/*
 * Render a smooth-shaded RGB triangle.
 */
void smooth_rgb_triangle_general(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1

#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, n = RIGHT - LEFT; \
        GLubyte cspan[4*MAX_WIDTH]; \
        if (n > 0) \
        { \
            for (i = 0; i < n; i++) \
            { \
                cspan[4*i + 0] = FixedToInt(ffr); \
                cspan[4*i + 1] = FixedToInt(ffg); \
                cspan[4*i + 2] = FixedToInt(ffb); \
                ffr += fdrdx; \
                ffg += fdgdx; \
                ffb += fdbdx; \
            } \
            gl_write_4color_span(ctx, n, LEFT, Y, NULL, cspan, GL_POLYGON); \
        } \
    }

#include "newtritemp.h"
}

void smooth_rgb_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            pRow[i] = FORM_RGB565(FixedToInt(ffr), FixedToInt(ffg), \
                                  FixedToInt(ffb)); \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
        } \
    }
#include "newtritemp.h"
}

void smooth_rgb_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        for (i = 0; i < len; i++) \
        { \
            pRow[i] = FORM_RGB555(FixedToInt(ffr), FixedToInt(ffg), \
                                  FixedToInt(ffb)); \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
        } \
    }
#include "newtritemp.h"
}

void dithered_smooth_rgb_triangle_565(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLuint* ditherPtr = &_ditherScreen[((Y)&3) * ctx->Buffer.Width]; \
        \
        for (i = 0; i < len; i++) \
        { \
            GLuint _dither = ditherPtr[i]; \
            pRow[i] = \
               ((((ffr>>15) + ((ffr & 0x7000) > _dither)) << 11) + \
                (((ffg>>14) + (((ffg & 0x3000)<<1) > _dither)) << 5) + \
                 ((ffb>>15) + ((ffb & 0x7000) > _dither))); \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
        } \
    }
#include "newtritemp.h"
}

void dithered_smooth_rgb_triangle_555(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) (PIXEL_TYPE*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define INNER_LOOP(LEFT, RIGHT, Y) \
    { \
        GLint i, len = RIGHT - LEFT; \
        GLuint* ditherPtr = &_ditherScreen[((Y)&3) * ctx->Buffer.Width]; \
        \
        for (i = 0; i < len; i++) \
        { \
            GLuint _dither = ditherPtr[i]; \
            pRow[i] = \
               ((((ffr>>15) + ((ffr & 0x7000) > _dither)) << 10) + \
                (((ffg>>15) + ((ffg & 0x7000) > _dither)) << 5) + \
                 ((ffb>>15) + ((ffb & 0x7000) > _dither))); \
            ffr += fdrdx; \
            ffg += fdgdx; \
            ffb += fdbdx; \
        } \
    }
#include "newtritemp.h"
}

typedef void (*triFunc)(GLcontext*, GLuint, GLuint, GLuint, GLuint);

triFunc flat_rgb_z_triangle = NULL;
triFunc flat_rgb_triangle = NULL;
triFunc smooth_rgb_z_triangle = NULL;
triFunc smooth_rgb_triangle = NULL;
triFunc flat_rgba_z_triangle = NULL;
triFunc flat_rgba_triangle = NULL;
triFunc smooth_rgba_z_triangle = NULL;
triFunc smooth_rgba_triangle = NULL;

void choose_newtriangle_rasterizers(GLcontext* ctx)
{
    char* ditherVar;
    GLboolean dither;

    ditherVar = getenv("RGL_SOFTWARE_DITHER");
    if (ditherVar == NULL)
    {
        dither = GL_FALSE;
    }
    else if (strstr(ditherVar, "1") != NULL)
    {
        dither = GL_TRUE;
    }
    else
    {
        dither = GL_FALSE;
    }

    flat_rgba_triangle = flat_rgba_triangle_general;
    flat_rgba_z_triangle = flat_rgba_z_triangle_general;
    smooth_rgba_triangle = smooth_rgba_triangle_general;
    smooth_rgba_z_triangle = smooth_rgba_z_triangle_general;

    switch (ctx->Buffer.PixelType)
    {
    case GL_RGB555:
        flat_rgb_z_triangle = flat_rgb_z_triangle_555;
        flat_rgb_triangle = flat_rgb_triangle_555;
        smooth_rgb_z_triangle = smooth_rgb_z_triangle_555;
        if (dither)
        {
            smooth_rgb_triangle = dithered_smooth_rgb_triangle_555;
        }
        else
        {
            smooth_rgb_triangle = smooth_rgb_triangle_555;
        }
        if (!ctx->DepthWrite)
        {
            if (ctx->PolygonStipple)
            {
                flat_rgba_z_triangle = flat_rgba_zz_stippled_triangle;
                smooth_rgba_z_triangle = smooth_rgba_zz_stippled_triangle_555;
            }
        }
        break;
    case GL_RGB565:
        flat_rgb_z_triangle = flat_rgb_z_triangle_565;
        flat_rgb_triangle = flat_rgb_triangle_565;
        smooth_rgb_z_triangle = smooth_rgb_z_triangle_565;
        if (dither)
        {
            smooth_rgb_triangle = dithered_smooth_rgb_triangle_565;
        }
        else
        {
            smooth_rgb_triangle = smooth_rgb_triangle_565;
        }
        if (!ctx->DepthWrite)
        {
            if (ctx->PolygonStipple)
            {
                flat_rgba_z_triangle = flat_rgba_zz_stippled_triangle;
                smooth_rgba_z_triangle = smooth_rgba_zz_stippled_triangle_565;
            }
        }
        break;
    default:
        flat_rgb_z_triangle = flat_rgb_z_triangle_general;
        flat_rgb_triangle = flat_rgb_triangle_general;
        smooth_rgb_z_triangle = smooth_rgb_z_triangle_general;
        smooth_rgb_triangle = smooth_rgb_triangle_general;
        break;
    }
}

#define TRI_ALPHA   1
#define TRI_Z       2
#define TRI_SMOOTH  4

static triFunc* triangleFunctions[8] =
{
    &flat_rgb_triangle,
    &flat_rgba_triangle,
    &flat_rgb_z_triangle,
    &flat_rgba_z_triangle,
    &smooth_rgb_triangle,
    &smooth_rgba_triangle,
    &smooth_rgb_z_triangle,
    &smooth_rgba_z_triangle
};

void new_triangle(GLcontext* ctx, GLuint vl[], GLuint pv)
{
    GLint index;
    triFunc func;

    scrMult = ctx->scrMult;
    stipple = ctx->PolygonStipple;

    index = 0;
    if (g_doAlpha) index += TRI_ALPHA;
    if (ctx->DepthTest) index += TRI_Z;
    if (ctx->ShadeModel == GL_SMOOTH) index += TRI_SMOOTH;

    func = *triangleFunctions[index];
    func(ctx, vl[0], vl[1], vl[2], pv);
//    if (ctx->DepthTest) flat_rgba_z_triangle(ctx, vl[0], vl[1], vl[2], pv);
//    else flat_rgba_triangle(ctx, vl[0], vl[1], vl[2], pv);
}
