#include "kgl.h"
#include "span.h"

extern GLint g_DepthMask;

#define PIXEL_ADDRESS2(X,Y) (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[Y] + ctx->Buffer.ByteMult*(X))


/*=============================================================================
    565
=============================================================================*/

void flat_565_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    GLubyte* color = ctx->VB->Color[pv];
    GLushort pixel = FORM_RGB565(color[0], color[1], color[2]);

#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = pixel;

#include "linetemp.h"
}

void flat_565_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    GLubyte* color = ctx->VB->Color[pv];
    GLushort pixel = FORM_RGB565(color[0], color[1], color[2]);

    if (ctx->LineStipple)
    {
#define INTERP_Z 1
#define STIPPLE 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        *zPtr = Z; \
        *pixelPtr = pixel; \
    }
#include "linetemp.h"
    }
    else
    {
#define INTERP_Z 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        *zPtr = Z; \
        *pixelPtr = pixel; \
    }

#include "linetemp.h"
    }
}

void flat_565_blend_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    GLubyte* color = ctx->VB->Color[pv];

#define INTERP_ALPHA 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    { \
        GLubyte br, bg, bb; \
        br = color[0]; \
        bg = color[1]; \
        bb = color[2]; \
        gl_blend_16bit(ctx, &br, &bg, &bb, FixedToInt(a0), pixelPtr); \
        *pixelPtr = FORM_RGB565(br, bg, bb); \
    }

#include "linetemp.h"
}

void flat_565_blend_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    GLubyte* color = ctx->VB->Color[pv];

#define INTERP_Z 1
#define INTERP_ALPHA 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        GLubyte br, bg, bb; \
        *zPtr = Z; \
        br = color[0]; \
        bg = color[1]; \
        bb = color[2]; \
        gl_blend_16bit(ctx, &br, &bg, &bb, FixedToInt(a0), pixelPtr); \
        *pixelPtr = FORM_RGB565(br, bg, bb); \
    }

#include "linetemp.h"
}

void smooth_565_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
#define INTERP_RGB 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = FORM_RGB565(FixedToInt(r0), FixedToInt(g0), FixedToInt(b0));

#include "linetemp.h"
}

void smooth_565_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    if (ctx->LineStipple)
    {
#define INTERP_Z 1
#define INTERP_RGB 1
#define STIPPLE 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        *zPtr = Z; \
        *pixelPtr = FORM_RGB565(FixedToInt(r0), FixedToInt(g0), FixedToInt(b0)); \
    }

#include "linetemp.h"
    }
    else
    {
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        *zPtr = Z; \
        *pixelPtr = FORM_RGB565(FixedToInt(r0), FixedToInt(g0), FixedToInt(b0)); \
    }

#include "linetemp.h"
    }
}

void smooth_565_blend_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    { \
        GLubyte br, bg, bb; \
        br = FixedToInt(r0); \
        bg = FixedToInt(g0); \
        bb = FixedToInt(b0); \
        gl_blend_16bit(ctx, &br, &bg, &bb, FixedToInt(a0), pixelPtr); \
        *pixelPtr = FORM_RGB565(br, bg, bb); \
    }

#include "linetemp.h"
}

void smooth_565_blend_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        GLubyte br, bg, bb; \
        br = FixedToInt(r0); \
        bg = FixedToInt(g0); \
        bb = FixedToInt(b0); \
        *zPtr = Z; \
        gl_blend_16bit(ctx, &br, &bg, &bb, FixedToInt(a0), pixelPtr); \
        *pixelPtr = FORM_RGB565(br, bg, bb); \
    }

#include "linetemp.h"
}


/*=============================================================================
    555
=============================================================================*/

void flat_555_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    GLubyte* color = ctx->VB->Color[pv];
    GLushort pixel = FORM_RGB555(color[0], color[1], color[2]);

#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = pixel;

#include "linetemp.h"
}

void flat_555_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    GLubyte* color = ctx->VB->Color[pv];
    GLushort pixel = FORM_RGB555(color[0], color[1], color[2]);

    if (ctx->LineStipple)
    {
#define STIPPLE 1
#define INTERP_Z 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        *zPtr = Z; \
        *pixelPtr = pixel; \
    }

#include "linetemp.h"
    }
    else
    {
#define INTERP_Z 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        *zPtr = Z; \
        *pixelPtr = pixel; \
    }

#include "linetemp.h"
    }
}

void flat_555_blend_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    GLubyte* color = ctx->VB->Color[pv];

#define INTERP_ALPHA 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    { \
        GLubyte br, bg, bb; \
        br = color[0]; \
        bg = color[1]; \
        bb = color[2]; \
        gl_blend_15bit(ctx, &br, &bg, &bb, FixedToInt(a0), pixelPtr); \
        *pixelPtr = FORM_RGB555(br, bg, bb); \
    }

#include "linetemp.h"
}

void flat_555_blend_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    GLubyte* color = ctx->VB->Color[pv];

#define INTERP_Z 1
#define INTERP_ALPHA 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        GLubyte br, bg, bb; \
        br = color[0]; \
        bg = color[1]; \
        bb = color[2]; \
        *zPtr = Z; \
        gl_blend_15bit(ctx, &br, &bg, &bb, FixedToInt(a0), pixelPtr); \
        *pixelPtr = FORM_RGB555(br, bg, bb); \
    }

#include "linetemp.h"
}

void smooth_555_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
#define INTERP_RGB 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = FORM_RGB555(FixedToInt(r0), FixedToInt(g0), FixedToInt(b0));

#include "linetemp.h"
}

void smooth_555_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
    if (ctx->LineStipple)
    {
#define INTERP_Z 1
#define INTERP_RGB 1
#define STIPPLE 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        *zPtr = Z; \
        *pixelPtr = FORM_RGB555(FixedToInt(r0), FixedToInt(g0), FixedToInt(b0)); \
    }

#include "linetemp.h"
    }
    else
    {
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        *zPtr = Z; \
        *pixelPtr = FORM_RGB555(FixedToInt(r0), FixedToInt(g0), FixedToInt(b0)); \
    }

#include "linetemp.h"
    }
}

void smooth_555_blend_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    { \
        GLubyte br, bg, bb; \
        br = FixedToInt(r0); \
        bg = FixedToInt(g0); \
        bb = FixedToInt(b0); \
        gl_blend_15bit(ctx, &br, &bg, &bb, FixedToInt(a0), pixelPtr); \
        *pixelPtr = FORM_RGB555(br, bg, bb); \
    }

#include "linetemp.h"
}

void smooth_555_blend_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv)
{
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW ctx->Buffer.Pitch
#define PIXEL_ADDRESS(X,Y) PIXEL_ADDRESS2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) \
    if (Z < *zPtr) \
    { \
        GLubyte br, bg, bb; \
        *zPtr = Z; \
        br = FixedToInt(r0); \
        bg = FixedToInt(g0); \
        bb = FixedToInt(b0); \
        gl_blend_15bit(ctx, &br, &bg, &bb, FixedToInt(a0), pixelPtr); \
        *pixelPtr = FORM_RGB555(br, bg, bb); \
    }

#include "linetemp.h"
}

