#include <string.h>
#include "kgl.h"
#include "span.h"
#include "scan.h"
#include "asm.h"

#include "stipple.c"

#define MONOCOLOR_RED   255
#define MONOCOLOR_GREEN 0
#define MONOCOLOR_BLUE  0

#define FOURCOLOR_RED   0
#define FOURCOLOR_GREEN 255
#define FOURCOLOR_BLUE  0

#define TEXTURE_RED     0
#define TEXTURE_GREEN   0
#define TEXTURE_BLUE    255

#define FIND_MONOCOLOR  0
#define FIND_4COLOR     0
#define FIND_TEXTURE    0

extern GLboolean g_doAlpha;
extern GLboolean g_doAlphaTest;
extern GLboolean stipple;

GLuint (*gl_depthtest_span_less)
    (GLcontext*, GLuint, GLint, GLint, GLdepth const[], GLubyte[]) = NULL;
GLuint (*gl_depthtest_alphatest_span_less)
    (GLcontext*, GLuint, GLint, GLint, GLdepth const[], GLubyte[]) = NULL;

void (*write_4pixel_span)(
        GLcontext*, GLint, GLint, GLint,
        GLubyte[], GLubyte[]) = NULL;
void (*write_monopixel_span)(
        GLcontext*, GLint, GLint, GLint,
        GLubyte, GLubyte, GLubyte, GLubyte[]) = NULL;

void depthlerp(GLint n, GLdepth zspan[], GLfixed zl, GLfixed dzdx)
{
    GLint i;
    GLdepth* zp = zspan;
    if (n == 0)
        return;
    zl |= g_DepthMask;
#if 0
    if (n == 1)
    {
        *zp = zl;
        return;
    }
#endif
    for (i = 0; i < n; i++)
    {
        *zp++ = (GLdepth)zl;
        zl += dzdx;
    }
}

void tri4colorlerp(GLint n, GLubyte cspan[],
                   edge* pLeft, edge* pRight, gradients* g)
{
    GLint i;
    GLubyte* cp = cspan;
    GLfixed rl, gl, bl, al;
    GLfixed drdx, dgdx, dbdx, dadx;

    if (n == 0)
        return;

    rl = pLeft->R;
    gl = pLeft->G;
    bl = pLeft->B;
    al = pLeft->A;
    drdx = g->fdRdX;
    dgdx = g->fdGdX;
    dbdx = g->fdBdX;
    dadx = g->fdAdX;
    for (i = 0; i < n; i++)
    {
        cp[4*i + 0] = FixedToInt(rl);
        cp[4*i + 1] = FixedToInt(gl);
        cp[4*i + 2] = FixedToInt(bl);
        cp[4*i + 3] = FixedToInt(al);
        rl += drdx;
        gl += dgdx;
        bl += dbdx;
        al += dadx;
    }
}

void tri3colorlerp(GLint n, GLubyte cspan[],
                   edge* pLeft, edge* pRight, gradients* g)
{
    GLint i;
    GLubyte* cp = cspan;
    GLfixed rl, gl, bl;
    GLfixed drdx, dgdx, dbdx;

    if (n == 0)
        return;

    rl = pLeft->R;
    gl = pLeft->G;
    bl = pLeft->B;
    drdx = g->fdRdX;
    dgdx = g->fdGdX;
    dbdx = g->fdBdX;
    for (i = 0; i < n; i++)
    {
        cp[4*i + 0] = FixedToInt(rl);
        cp[4*i + 1] = FixedToInt(gl);
        cp[4*i + 2] = FixedToInt(bl);
        rl += drdx;
        gl += dgdx;
        bl += dbdx;
    }
}

void write_monopixel_span555(GLcontext* ctx, GLint n, GLint x, GLint y,
                             GLubyte r, GLubyte g, GLubyte b,
                             GLubyte mask[])
{
    GLushort* buf = CTX_FB_ADDRESS2(ctx, x, y);
    GLushort cval;
    GLint i;

    cval = FORM_RGB555(r, g, b);

    if (mask == NULL)
    {
        for (i = 0; i < n; i++)
        {
            buf[i] = cval;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                buf[i] = cval;
            }
        }
    }
}

void write_monopixel_span565(GLcontext* ctx, GLint n, GLint x, GLint y,
                             GLubyte r, GLubyte g, GLubyte b,
                             GLubyte mask[])
{
    GLushort* buf = CTX_FB_ADDRESS2(ctx, x, y);
    GLushort cval;
    GLint i;

    cval = FORM_RGB565(r, g, b);

    if (mask == NULL)
    {
        for (i = 0; i < n; i++)
        {
            buf[i] = cval;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                buf[i] = cval;
            }
        }
    }
}

void write_monopixel_span888(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte r, GLubyte g, GLubyte b, GLubyte mask[])
{
    GLubyte* buf = CTX_FB_ADDRESS1(ctx, x, y);
    GLint i;

    if (mask == NULL)       //no mask tests
    {
        for (i = 0; i < n; i++)
        {
            buf[0] = r;
            buf[1] = g;
            buf[2] = b;
            buf += 3;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                buf[0] = r;
                buf[1] = g;
                buf[2] = b;
            }
            buf += 3;
        }
    }
}

void write_monopixel_span888R(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte r, GLubyte g, GLubyte b, GLubyte mask[])
{
    GLubyte* buf = CTX_FB_ADDRESS1(ctx, x, y);
    GLint i;

    if (mask == NULL)       //no mask tests
    {
        for (i = 0; i < n; i++)
        {
            buf[0] = b;
            buf[1] = g;
            buf[2] = r;
            buf += 3;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                buf[0] = b;
                buf[1] = g;
                buf[2] = r;
            }
            buf += 3;
        }
    }
}

void write_monopixel_span32(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte r, GLubyte g, GLubyte b, GLubyte mask[])
{
    GLuint* buf = CTX_FB_ADDRESS4(ctx, x, y);
    GLuint  cval;
    GLint i;

    cval = FORM_RGB32(r, g, b);

    if (mask == NULL)       //no mask tests
    {
        for (i = 0; i < n; i++)
        {
            *buf++ = cval;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                *buf = cval;
            }
            buf++;
        }
    }
}

void write_monopixel_span32R(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte r, GLubyte g, GLubyte b, GLubyte mask[])
{
    GLuint* buf = CTX_FB_ADDRESS4(ctx, x, y);
    GLuint  cval;
    GLint i;

    cval = FORM_BGR32(r, g, b);

    if (mask == NULL)       //no mask tests
    {
        for (i = 0; i < n; i++)
        {
            *buf++ = cval;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                *buf = cval;
            }
            buf++;
        }
    }
}

void write_4pixel_span555(GLcontext* ctx, GLint n, GLint x, GLint y,
                          GLubyte c[], GLubyte mask[])
{
    GLushort* buf = CTX_FB_ADDRESS2(ctx, x, y);
    GLubyte* cp = c;
    GLint i;

    if (mask == NULL)
    {
        for (i = 0; i < n; i++)
        {
            buf[i] = FORM_RGB555(c[4*i+0], c[4*i+1], c[4*i+2]);
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                buf[i] = FORM_RGB555(c[4*i+0], c[4*i+1], c[4*i+2]);
            }
        }
    }
}

void write_4pixel_span565(GLcontext* ctx, GLint n, GLint x, GLint y,
                          GLubyte c[], GLubyte mask[])
{
    GLushort* buf = CTX_FB_ADDRESS2(ctx, x, y);
    GLint i;

    if (mask == NULL)
    {
        for (i = 0; i < n; i++)
        {
            buf[i] = FORM_RGB565(c[4*i+0], c[4*i+1], c[4*i+2]);
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                buf[i] = FORM_RGB565(c[4*i+0], c[4*i+1], c[4*i+2]);
            }
        }
    }
}

void write_4pixel_span888R(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte c[], GLubyte mask[])
{
    GLubyte* buf = CTX_FB_ADDRESS1(ctx, x, y);
    GLubyte* cp = c;
    GLint i;

    if (mask == NULL)
    {
        for (i = 0; i < n; i++)
        {
            buf[0] = cp[2];
            buf[1] = cp[1];
            buf[2] = cp[0];
            cp += 4;
            buf += 3;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                buf[0] = cp[2];
                buf[1] = cp[1];
                buf[2] = cp[0];
            }
            cp += 4;
            buf += 3;
        }
    }
}

void write_4pixel_span888(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte c[], GLubyte mask[])
{
    GLubyte* buf = CTX_FB_ADDRESS1(ctx, x, y);
    GLubyte* cp = c;
    GLint i;

    if (mask == NULL)
    {
        for (i = 0; i < n; i++)
        {
            buf[0] = cp[0];
            buf[1] = cp[1];
            buf[2] = cp[2];
            cp += 4;
            buf += 3;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                buf[0] = cp[0];
                buf[1] = cp[1];
                buf[2] = cp[2];
            }
            cp += 4;
            buf += 3;
        }
    }
}

void write_4pixel_span32(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte c[], GLubyte mask[])
{
    GLuint* buf = CTX_FB_ADDRESS4(ctx, x, y);
    GLubyte* cp = c;
    GLint i;

    if (mask == NULL)
    {
        for (i = 0; i < n; i++)
        {
            *buf++ = FORM_RGB32(cp[0], cp[1], cp[2]);
            cp += 4;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                *buf = FORM_RGB32(cp[0], cp[1], cp[2]);
            }
            cp += 4;
            buf++;
        }
    }
}

void write_4pixel_span32R(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte c[], GLubyte mask[])
{
    GLuint* buf = CTX_FB_ADDRESS4(ctx, x, y);
    GLubyte* cp = c;
    GLint i;

    if (mask == NULL)
    {
        for (i = 0; i < n; i++)
        {
            *buf++ = FORM_BGR32(cp[0], cp[1], cp[2]);
            cp += 4;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                *buf = FORM_BGR32(cp[0], cp[1], cp[2]);
            }
            cp += 4;
            buf++;
        }
    }
}

GLuint gl_depthtest_span_less_write(
    GLcontext* ctx, GLuint n, GLint x, GLint y,
    GLdepth const z[], GLubyte mask[])
{
    GLdepth* zptr;
    GLuint i;
    GLuint passed = 0;

    zptr = CTX_Z_ADDRESS(ctx, x, y);

    for (i = 0; i < n; i++)
    {
        if (z[i] < zptr[i])
        {
            //pass
            zptr[i] = z[i];
            mask[i] = 1;
            passed++;
        }
        else
        {
            //fail
            mask[i] = 0;
        }
    }
    return passed;
}

GLuint gl_depthtest_alphatest_span_less_write(
    GLcontext* ctx, GLuint n, GLint x, GLint y,
    GLdepth const z[], GLubyte mask[])
{
    GLdepth* zptr;
    GLuint i;
    GLuint passed = 0;

    zptr = CTX_Z_ADDRESS(ctx, x, y);

    for (i = 0; i < n; i++)
    {
        if (mask[i])
        {
            if (z[i] < zptr[i])
            {
                //pass
                zptr[i] = z[i];
                passed++;
            }
            else
            {
                //fail
                mask[i] = 0;
            }
        }
    }
    return passed;
}

GLuint gl_depthtest_span_less_nowrite(
        GLcontext* ctx,
        GLuint n, GLint x, GLint y,
        GLdepth const z[],
        GLubyte mask[])
{
    GLdepth* zptr = CTX_Z_ADDRESS(ctx, x, y);
    GLuint i;
    GLuint passed = 0;

    for (i = 0; i < n; i++)
    {
        if (z[i] < zptr[i])
        {
            //pass
            mask[i] = 1;
            passed++;
        }
        else
        {
            //fail
            mask[i] = 0;
        }
    }
    return passed;
}

GLuint gl_depthtest_alphatest_span_less_nowrite(
        GLcontext* ctx,
        GLuint n, GLint x, GLint y,
        GLdepth const z[],
        GLubyte mask[])
{
    GLdepth* zptr = CTX_Z_ADDRESS(ctx, x, y);
    GLuint i;
    GLuint passed = 0;

    for (i = 0; i < n; i++)
    {
        if (mask[i])
        {
            if (z[i] < zptr[i])
            {
                //pass
                passed++;
            }
            else
            {
                //fail
                mask[i] = 0;
            }
        }
    }
    return passed;
}

void gl_read_color_span(
        GLcontext* ctx, GLuint n, GLint x, GLint y, GLubyte c[])
{
    GLuint i, Width = ctx->Buffer.Width;
    GLubyte* cp = c;

    if (y < 0 || y >= (GLint)ctx->Buffer.Height)
    {
    FILLER:
        for (i = 0; i < n; i++)
        {
            c[i] = 0;
        }
        return;
    }

    if ((x+(GLint)n) < 0)
    {
        goto FILLER;
    }
    if (x < 0)
    {
        while (x < 0)
        {
            x++;
            n--;
        }
        if (n <= 0)
        {
            goto FILLER;
        }
    }
    if (x >= (GLint)Width)
    {
        goto FILLER;
    }
    if ((x+n) >= Width)
    {
        while ((x+n) >= Width)
        {
            n--;
        }
        if (n <= 0)
        {
            goto FILLER;
        }
    }

    if (ctx->Buffer.Depth == 32)
    {
        GLuint* buf = CTX_FB_ADDRESS4(ctx, x, y);
        if (ctx->Buffer.PixelType == GL_RGB32)
        {
            for (i = 0; i < n; i++, x++, cp += 4, buf++)
            {
                cp[0] = GET_RED32(*buf);
                cp[1] = GET_GREEN32(*buf);
                cp[2] = GET_BLUE32(*buf);
                cp[3] = 255;
            }
        }
        else
        {
            for (i = 0; i < n; i++, x++, cp += 4, buf++)
            {
                cp[0] = GET_RED32R(*buf);
                cp[1] = GET_GREEN32R(*buf);
                cp[2] = GET_BLUE32R(*buf);
                cp[3] = 255;
            }
        }
    }
    else if (ctx->Buffer.Depth == 24)
    {
        GLubyte* buf = CTX_FB_ADDRESS1(ctx, x, y);
        if (ctx->Buffer.PixelType == GL_BGR888)
        {
            for (i = 0; i < n; i++, x++, cp += 4, buf += 3)
            {
                cp[0] = buf[2];
                cp[1] = buf[1];
                cp[2] = buf[0];
                cp[3] = 255;
            }
        }
        else
        {
            for (i = 0; i < n; i++, cp += 4, buf += 3)
            {
                cp[0] = buf[0];
                cp[1] = buf[1];
                cp[2] = buf[2];
                cp[3] = 255;
            }
        }
    }
    else
    {
        GLushort* buf = CTX_FB_ADDRESS2(ctx, x, y);
        GLubyte r, g, b;
        GLushort color;
        if (ctx->Buffer.PixelType == GL_RGB555)
        {
            for (i = 0; i < n; i++, cp += 4, buf++)
            {
                color = *buf;
                r = (color >> 10);
                g = (color >> 5) & 0x1f;
                b = color & 0x1f;
                cp[0] = r << 3;
                cp[1] = g << 3;
                cp[2] = b << 3;
                cp[3] = 255;
            }
        }
        else
        {
            for (i = 0; i < n; i++, cp += 4, buf++)
            {
                color = *buf;
                r = (color >> 11);
                g = (color >> 5) & 0x3f;
                b = color & 0x1f;
                cp[0] = r << 3;
                cp[1] = g << 2;
                cp[2] = b << 3;
                cp[3] = 255;
            }
        }
    }
}

void gl_blend_32bit(GLcontext* ctx, GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLuint* cdest)
{
    GLint r, g, b;
    GLint t = (GLint)sa;
    GLint s = 256 - t;

    if (ctx->BlendDst == GL_ONE)
    {
        s = 256;
    }
    if (ctx->Buffer.PixelType == GL_RGB32)
    {
        r = (*sr * t + GET_RED32(*cdest) * s) >> 8;
        g = (*sg * t + GET_GREEN32(*cdest) * s) >> 8;
        b = (*sb * t + GET_BLUE32(*cdest) * s) >> 8;
    }
    else
    {
        r = (*sr * t + GET_RED32R(*cdest) * s) >> 8;
        g = (*sg * t + GET_GREEN32R(*cdest) * s) >> 8;
        b = (*sb * t + GET_BLUE32R(*cdest) * s) >> 8;
    }
    *sr = MIN2(r, 255);
    *sg = MIN2(g, 255);
    *sb = MIN2(b, 255);
}

void gl_blend_24bit(GLcontext* ctx, GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLubyte* cdest)
{
    GLint r, g, b;
    GLint t = (GLint)sa;
    GLint s = 256 - t;

    if (ctx->BlendDst == GL_ONE)
    {
        s = 256;
    }
    if (ctx->Buffer.PixelType == GL_RGB888)
    {
        r = (*sr * t + cdest[0] * s) >> 8;
        g = (*sg * t + cdest[1] * s) >> 8;
        b = (*sb * t + cdest[2] * s) >> 8;
    }
    else
    {
        r = (*sr * t + cdest[2] * s) >> 8;
        g = (*sg * t + cdest[1] * s) >> 8;
        b = (*sb * t + cdest[0] * s) >> 8;
    }
    *sr = MIN2(r, 255);
    *sg = MIN2(g, 255);
    *sb = MIN2(b, 255);
}

void gl_blend_16bit(GLcontext* ctx, GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLushort* cdest)
{
    GLint r, g, b;
    GLint t = (GLint)sa;
    GLint s = 256 - t;
    GLint dr, dg, db;
    GLushort dbyte = *cdest;

    if (ctx->BlendDst == GL_ONE)
    {
        s = 256;
    }

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

void gl_blend_15bit(GLcontext* ctx, GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLushort* cdest)
{
    GLint r, g, b;
    GLint t = (GLint)sa;
    GLint s = 256 - t;
    GLint dr, dg, db;
    GLushort dbyte = *cdest;

    if (ctx->BlendDst == GL_ONE)
    {
        s = 256;
    }

    dr = dbyte >> 10;
    dg = (dbyte >> 5) & 0x1f;
    db = dbyte & 0x1f;
    dr <<= 3;
    dg <<= 3;
    db <<= 3;
    r = (*sr * t + dr * s) >> 8;
    g = (*sg * t + dg * s) >> 8;
    b = (*sb * t + db * s) >> 8;
    *sr = MIN2(r, 255);
    *sg = MIN2(g, 255);
    *sb = MIN2(b, 255);
}

GLboolean gl_monostipple_span(
    GLcontext* ctx, GLuint n, GLint x, GLint y,
    GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha,
    GLubyte mask[])
{
    GLuint i;
    GLint  yind, index;

    if (alpha < 16)
    {
        return GL_FALSE;
    }

    yind  = (y & 3) << 2;
    index = alpha & 0xf0;
    index += yind;

    for (i = 0; i < n; i++, x++)
    {
        if (mask[i])
        {
            if (alpha_stipple[index + (x & 3)] == 0)
            {
                mask[i] = 0;
            }
        }
    }

    return GL_TRUE;
}

GLuint gl_stipple_span(
    GLcontext* ctx, GLuint n, GLint x, GLint y, GLubyte c[], GLubyte mask[])
{
    GLuint i, hits;
    GLubyte* cp = c;
    GLint yind;

    yind = (y & 3) << 2;

    for (i = hits = 0; i < n; i++, cp += 4, x++)
    {
        if (mask[i])
        {
            GLint index = cp[3] & 0xf0;
            if (alpha_stipple[index + yind + (x & 3)] == 0)
            {
                mask[i] = 0;
            }
            else
            {
                hits++;
            }
        }
    }
    return hits;
}

extern GLubyte* alphaTable;

void gl_blend_span(
        GLcontext* ctx, GLuint n, GLubyte c[],
        GLubyte cdest[], GLubyte mask[])
{
    GLuint i;
    GLubyte* cp = c;
    GLubyte* cdp = cdest;
    GLint  index;

    for (i = 0; i < n; i++, cp += 4, cdp += 4)
    {
        if (mask == NULL || mask[i])
        {
            index = (cp[3] & 0xf8) << 7;
            cp[0] = alphaTable[index + ((cp[0]&0xf8)<<2) + (cdp[0]>>3)];
            cp[1] = alphaTable[index + ((cp[1]&0xf8)<<2) + (cdp[1]>>3)];
            cp[2] = alphaTable[index + ((cp[2]&0xf8)<<2) + (cdp[2]>>3)];
        }
    }
}

void gl_alphatest_span(GLcontext* ctx, GLuint n, GLubyte c[], GLubyte mask[])
{
    GLuint i;
    GLubyte ref = ctx->AlphaByteRef;

    switch (ctx->AlphaFunc)
    {
    case GL_LESS:
        for (i = 0; i < n; i++)
        {
            mask[i] &= (c[4*i + 3] < ref);
        }
        break;

    case GL_GREATER:
        for (i = 0; i < n; i++)
        {
            mask[i] &= (c[4*i + 3] > ref);
        }
        break;

    default:
        gl_problem(ctx, "gl_alphatest_span(AlphaFunc)");
    }
}

void gl_bias_span(GLcontext* ctx, GLuint n, GLubyte c[], GLubyte mask[])
{
    GLuint i;
    GLubyte rbias = ctx->ByteBias[0];
    GLubyte gbias = ctx->ByteBias[1];
    GLubyte bbias = ctx->ByteBias[2];

    for (i = 0; i < n; i++)
    {
        if (mask[i])
        {
            c[4*i + 0] = MIN2(c[4*i + 0] + rbias, 255);
            c[4*i + 1] = MIN2(c[4*i + 1] + gbias, 255);
            c[4*i + 2] = MIN2(c[4*i + 2] + bbias, 255);
        }
    }
}

void gl_lightingadjust_span(GLcontext* ctx, GLuint n, GLubyte c[], GLubyte mask[])
{
    GLint i;
    GLint badjust = FAST_TO_INT((1.0f - ctx->LightingAdjust) * 255.0f);

    for (i = 0; i < n; i++)
    {
        if (mask[i])
        {
            c[4*i + 0] = ((GLint)c[4*i + 0] * badjust) >> 8;
            c[4*i + 1] = ((GLint)c[4*i + 1] * badjust) >> 8;
            c[4*i + 2] = ((GLint)c[4*i + 2] * badjust) >> 8;
        }
    }
}

void sample_texture(
    gl_texture_object* tex, GLuint n, GLfixed uv[], GLubyte tspan[])
{
    GLint width = tex->Width;
    GLint height = tex->Height;
    GLint sMask = tex->WidthMask;
    GLint tMask = tex->HeightMask;
    GLint shift = tex->WidthLog2;
    GLuint k;
    GLint s, t, pos;
    GLubyte* texel;

    switch (tex->Format)
    {
    case GL_RGB:
        for (k = 0; k < n; k++)
        {
            s = FixedToInt(uv[2*k + 0]) & sMask;
            t = FixedToInt(uv[2*k + 1]) & tMask;
            pos = (t << shift) | s;
            texel = tex->Data + (pos << 2);
            tspan[4*k + 0] = texel[0];
            tspan[4*k + 1] = texel[1];
            tspan[4*k + 2] = texel[2];
        }
        break;

    case GL_RGBA:
        for (k = 0; k < n; k++)
        {
            s = FixedToInt(uv[2*k + 0]) & sMask;
            t = FixedToInt(uv[2*k + 1]) & tMask;
            pos = (t << shift) | s;
            texel = tex->Data + (pos << 2);
            tspan[4*k + 0] = texel[0];
            tspan[4*k + 1] = texel[1];
            tspan[4*k + 2] = texel[2];
            tspan[4*k + 3] = texel[3];
        }
        break;

    case GL_COLOR_INDEX:
        {
            GLubyte* palette;
            GLcontext* ctx = gl_get_context_ext();

            if (ctx->UsingSharedPalette)
            {
                palette = ctx->SharedPalette;
            }
            else
            {
                palette = tex->Palette;
            }

            for (k = 0; k < n; k++)
            {
                s = FixedToInt(uv[2*k + 0]) & sMask;
                t = FixedToInt(uv[2*k + 1]) & tMask;
                pos = (t << shift) | s;
                texel = palette + (tex->Data[pos] << 2);
                tspan[4*k + 0] = texel[0];
                tspan[4*k + 1] = texel[1];
                tspan[4*k + 2] = texel[2];
                //i'm ignoring the palette's possible alpha channel
            }
        }
        break;

    default:
        gl_problem(NULL, "sample_texture(tex->Format)");
    }
}

void apply_texture(
    GLcontext* ctx, GLuint n,
    GLenum format, GLenum envmode,
    GLubyte cspan[], GLubyte tspan[])
{
    GLint i;

    if (format == GL_COLOR_INDEX) format = GL_RGB;

    switch (envmode)
    {
    case GL_REPLACE:
        switch (format)
        {
        case GL_RGB:
            for (i = 0; i < n; i++)
            {
                cspan[4*i + 0] = tspan[4*i + 0];
                cspan[4*i + 1] = tspan[4*i + 1];
                cspan[4*i + 2] = tspan[4*i + 2];
            }
            break;

        case GL_RGBA:
            MEMCPY(cspan, tspan, 4*n);
            break;

        default:
            gl_problem(NULL, "apply_texture(format)");
        }
        break;

    case GL_MODULATE:
        switch (format)
        {
        case GL_RGB:
            for (i = 0; i < n; i++)
            {
                cspan[4*i + 0] = PROD8(cspan[4*i + 0], tspan[4*i + 0]);
                cspan[4*i + 1] = PROD8(cspan[4*i + 1], tspan[4*i + 1]);
                cspan[4*i + 2] = PROD8(cspan[4*i + 2], tspan[4*i + 2]);
            }
            break;

        case GL_RGBA:
            for (i = 0; i < n; i++)
            {
                cspan[4*i + 0] = PROD8(cspan[4*i + 0], tspan[4*i + 0]);
                cspan[4*i + 1] = PROD8(cspan[4*i + 1], tspan[4*i + 1]);
                cspan[4*i + 2] = PROD8(cspan[4*i + 2], tspan[4*i + 2]);
                cspan[4*i + 3] = PROD8(cspan[4*i + 3], tspan[4*i + 3]);
            }
            break;

        default:
            gl_problem(NULL, "apply_texture(format)");
        }
        break;

    default:
        gl_problem(NULL, "apply_texture(envmode)");
    }
}

void gl_texture_span(GLcontext* ctx, GLuint n, GLfixed uv[], GLubyte cspan[])
{
    GLubyte tspan[4*MAX_WIDTH];

    sample_texture(ctx->TexBoundObject, n, uv, tspan);

    apply_texture(ctx, n,
                  ctx->TexBoundObject->Format,
                  ctx->TexEnvMode,
                  cspan, tspan);
}

//15/16-bit ONLY
void gl_direct_mmx_blend_masked(
    GLcontext* ctx, GLuint n, GLint x, GLint y,
    GLdepth z[], GLubyte c[], GLubyte mask[])
{
    GLubyte*  mp;
    GLushort* buf;

    mp = mask;
    buf = CTX_FB_ADDRESS2(ctx, x, y);
    if (x&1)
    {
        if (mask[0])
        {
            GLubyte r, g, b, a;

            r = c[0];
            g = c[1];
            b = c[2];
            a = c[3];
            if (ctx->Buffer.PixelType == GL_RGB565)
            {
                gl_blend_16bit(ctx, &r, &g, &b, a, buf);
                *buf = FORM_RGB565(r, g, b);
            }
            else
            {
                gl_blend_15bit(ctx, &r, &g, &b, a, buf);
                *buf = FORM_RGB555(r, g, b);
            }
        }
        mp++;
        buf++;
        x++;
        c += 4;
        n--;
        if (n == 0)
        {
            return;
        }
    }

    gl_mmx_blend_span(ctx, n, mp, (GLubyte(*)[4])c, buf);
}

//15/16-bit ONLY
void gl_direct_mmx_blend(
    GLcontext* ctx, GLuint n, GLint x, GLint y, GLdepth z[], GLubyte c[])
{
    GLubyte   mask[MAX_WIDTH];
    GLubyte*  mp;
    GLushort* buf;

    if (ctx->DepthTest)
    {
        if (gl_depthtest_span_less(ctx, n, x, y, z, mask) == 0)
        {
            return;
        }
    }
    else
    {
        MEMSET(mask, 1, n);
    }

    mp = mask;
    buf = CTX_FB_ADDRESS2(ctx, x, y);
    if (x&1)
    {
        if (mask[0])
        {
            GLubyte r, g, b, a;

            r = c[0];
            g = c[1];
            b = c[2];
            a = c[3];
            if (ctx->Buffer.PixelType == GL_RGB565)
            {
                gl_blend_16bit(ctx, &r, &g, &b, a, buf);
                *buf = FORM_RGB565(r, g, b);
            }
            else
            {
                gl_blend_15bit(ctx, &r, &g, &b, a, buf);
                *buf = FORM_RGB555(r, g, b);
            }
        }
        mp++;
        buf++;
        x++;
        c += 4;
        n--;
        if (n == 0)
        {
            return;
        }
    }

    gl_mmx_blend_span(ctx, n, mp, (GLubyte(*)[4])c, buf);
}

void gl_write_monocolor_span(
        GLcontext* ctx,
        GLuint n, GLint x, GLint y, GLdepth z[],
        GLubyte r, GLubyte g, GLubyte b, GLubyte a,
        GLenum primitive)
{
    GLubyte mask[MAX_WIDTH];
    GLboolean write_all = GL_TRUE;
    if (n == 0)
        return;

    if (ctx->ScissorTest)
    {
        if (y < ctx->ScissorY || y >= (ctx->ScissorY + ctx->ScissorHeight))
        {
            return;
        }
    }

    if (ctx->DepthTest || (stipple && g_doAlpha))
    {
        if (ctx->DepthTest)
        {
            GLuint m = gl_depthtest_span_less(ctx, n, x, y, z, mask);
            if (m == 0) return;
            if (m != n) write_all = GL_FALSE;
        }
        else
        {
            MEMSET(mask, 1, n);
            write_all = GL_FALSE;
        }
    }

#if FIND_MONOCOLOR
    write_monopixel_span(ctx, n, x, y,
                         MONOCOLOR_RED, MONOCOLOR_GREEN, MONOCOLOR_BLUE,
                         write_all ? NULL : mask);
#else
    if (g_doAlpha)
    {
        if (stipple)
        {
            if (!gl_monostipple_span(ctx, n, x, y, r, g, b, a, mask))
            {
                return;
            }
            write_monopixel_span(ctx, n, x, y, r, g, b, mask);
        }
        else
        {
            GLubyte cdest[4*MAX_WIDTH];
            GLubyte c[4*MAX_WIDTH];
            GLubyte* cp = c;
            GLint i;
            gl_read_color_span(ctx, n, x, y, cdest);
            for (i = 0; i < (GLint)n; i++, cp += 4)
            {
                cp[0] = r;
                cp[1] = g;
                cp[2] = b;
                cp[3] = a;
            }
            gl_blend_span(ctx, n, c, cdest, (ctx->DepthTest) ? mask : NULL);
            write_4pixel_span(ctx, n, x, y, c, write_all ? NULL : mask);
        }
    }
    else
    {
        write_monopixel_span(ctx, n, x, y, r, g, b, write_all ? NULL : mask);
    }
#endif
}

void gl_write_texture_span(GLcontext* ctx, GLuint n, GLint x, GLint y, GLdepth z[],
                           GLfixed uv[], GLubyte c[])
{
    GLubyte mask[MAX_WIDTH];
    GLboolean write_all = GL_TRUE;
    GLuint m;

    if (n == 0) return;

    //FIXME: newtritemp.h has already scissored (?)
    if (ctx->ScissorTest)
    {
        if (y < ctx->ScissorY || y >= (ctx->ScissorY + ctx->ScissorHeight))
        {
            return;
        }
    }

    MEMSET(mask, 1, n);

    gl_texture_span(ctx, n, uv, c);

    if (g_doAlphaTest)
    {
        gl_alphatest_span(ctx, n, c, mask);
    }

    if (ctx->DepthTest)
    {
        m = gl_depthtest_alphatest_span_less(ctx, n, x, y, z, mask);
        if (m == 0) return;
        if (m != n) write_all = GL_FALSE;
    }

    if (ctx->Bias)
    {
        gl_bias_span(ctx, n, c, mask);
    }

    //FIXME: should this go after blending?
    if (ctx->LightingAdjust != 0.0f)
    {
        gl_lightingadjust_span(ctx, n, c, mask);
    }

#if FIND_TEXTURE
    write_monopixel_span(ctx, n, x, y,
                         TEXTURE_RED, TEXTURE_GREEN, TEXTURE_BLUE,
                         write_all ? NULL : mask);
#else
    if (g_doAlpha)
    {
        if (stipple)
        {
            m = gl_stipple_span(ctx, n, x, y, c, mask);
            if (m == 0) return;
            if (m != n) write_all = GL_FALSE;
        }
        else if (ctx->CpuMMX &&
                 (ctx->BlendDst == GL_ONE_MINUS_SRC_ALPHA) &&
                 (ctx->Buffer.Depth < 24))
        {
            gl_direct_mmx_blend_masked(ctx, n, x, y, z, c, mask);
            return;
        }
        else
        {
            GLubyte cdest[4*MAX_WIDTH];
            gl_read_color_span(ctx, n, x, y, cdest);
            gl_blend_span(ctx, n, c, cdest, (ctx->DepthTest) ? mask : NULL);
        }
    }

    write_4pixel_span(ctx, n, x, y, c, write_all ? NULL : mask);
#endif
}

void gl_write_4color_span(GLcontext* ctx, GLuint n, GLint x, GLint y,
                          GLdepth z[], GLubyte c[], GLenum primitive)
{
    GLubyte mask[MAX_WIDTH];
    GLboolean write_all = GL_TRUE;
    GLuint m;

    if (n == 0)
    {
        return;
    }

    if (ctx->ScissorTest)
    {
        if (y < ctx->ScissorY || y >= (ctx->ScissorY + ctx->ScissorHeight))
        {
            return;
        }
    }

#if !FIND_4COLOR
    if ((g_doAlpha && !stipple) &&
        ctx->CpuMMX &&
        (ctx->BlendDst == GL_ONE_MINUS_SRC_ALPHA) &&
        (ctx->Buffer.Depth < 24))
    {
        gl_direct_mmx_blend(ctx, n, x, y, z, c);
        return;
    }
#endif

    if (ctx->DepthTest || (stipple && g_doAlpha))
    {
        if (ctx->DepthTest)
        {
            m = gl_depthtest_span_less(ctx, n, x, y, z, mask);
            if (m == 0)
            {
                return;
            }
            else if (m != n)
            {
                write_all = GL_FALSE;
            }
        }
        else
        {
            MEMSET(mask, 1, n);
            write_all = GL_FALSE;
        }
    }

#if FIND_4COLOR
    write_monopixel_span(ctx, n, x, y,
                         FOURCOLOR_RED, FOURCOLOR_GREEN, FOURCOLOR_BLUE,
                         write_all ? NULL : mask);
#else
    if (g_doAlpha)
    {
        if (stipple)
        {
            m = gl_stipple_span(ctx, n, x, y, c, mask);
            if (m == 0) return;
            if (m != n) write_all = GL_FALSE;
        }
        else
        {
            GLubyte cdest[4*MAX_WIDTH];
            gl_read_color_span(ctx, n, x, y, cdest);
            gl_blend_span(ctx, n, c, cdest, (ctx->DepthTest) ? mask : NULL);
        }
    }

    write_4pixel_span(ctx, n, x, y, c, write_all ? NULL : mask);
#endif
}

void init_color_spanners(GLcontext* ctx)
{
    switch (ctx->Buffer.PixelType)
    {
    case GL_RGB555:
        write_4pixel_span = write_4pixel_span555;
        write_monopixel_span = write_monopixel_span555;
        break;
    case GL_RGB565:
        write_4pixel_span = write_4pixel_span565;
        write_monopixel_span = write_monopixel_span565;
        break;
    case GL_RGB888:
        write_4pixel_span = write_4pixel_span888;
        write_monopixel_span = write_monopixel_span888;
        break;
    case GL_BGR888:
        write_4pixel_span = write_4pixel_span888R;
        write_monopixel_span = write_monopixel_span888R;
        break;
    case GL_RGB32:
        write_4pixel_span = write_4pixel_span32;
        write_monopixel_span = write_monopixel_span32;
        break;
    case GL_BGR32:
        write_4pixel_span = write_4pixel_span32R;
        write_monopixel_span = write_monopixel_span32R;
        break;
    }
}
