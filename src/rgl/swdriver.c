#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "kgl.h"
#include "span.h"
#include "scan.h"
#include "newtriangle.h"
#include "swdrv.h"
#include "lines.h"

//#define TRIANGLE_FUNC barrett_triangle

#define C_CLEAR 0

#define FAST_DEPTHBUFFER 1

#define HECKER_GENERAL_PERSPECTIVE  0
#define HECKER_GENERAL              0

#define SW_TRI_SMOOTH   0x1
#define SW_TRI_Z        0x2

#define SW_LINE_BLEND   0x1
#define SW_LINE_SMOOTH  0x2
#define SW_LINE_Z       0x4

static void draw_background(GLubyte* pixels);
static void screenshot(GLubyte* buf);

typedef void (*swLineFunc)(GLcontext*, GLuint, GLuint, GLuint);

static void (*_flat_line)(GLcontext*, GLuint, GLuint, GLuint);
static void (*_flat_z_line)(GLcontext*, GLuint, GLuint, GLuint);
static void (*_flat_blend_line)(GLcontext*, GLuint, GLuint, GLuint);
static void (*_flat_blend_z_line)(GLcontext*, GLuint, GLuint, GLuint);
static void (*_smooth_line)(GLcontext*, GLuint, GLuint, GLuint);
static void (*_smooth_z_line)(GLcontext*, GLuint, GLuint, GLuint);
static void (*_smooth_blend_line)(GLcontext*, GLuint, GLuint, GLuint);
static void (*_smooth_blend_z_line)(GLcontext*, GLuint, GLuint, GLuint);

static swLineFunc* line_funcs[] =
{
    &_flat_line,
    &_flat_blend_line,
    &_smooth_line,
    &_smooth_blend_line,
    &_flat_z_line,
    &_flat_blend_z_line,
    &_smooth_z_line,
    &_smooth_blend_z_line,
};


double chop_temp;

extern GLubyte* alphaTable;

extern GLboolean g_doBias;
extern GLboolean g_doAlpha;
extern GLboolean g_doAlphaTest;

extern GLubyte g_redBias;
extern GLubyte g_greenBias;
extern GLubyte g_blueBias;

GLint g_DepthMask = 0;
static GLboolean g_HasBeenCleared = GL_FALSE;

static void init_texobjs(void);
static void clear_texobjs(void);

#define DR DriverFuncs

#define RUB(x) FAST_TO_INT(x * 255.0f)

/*
 * the reality is that the context never changes.
 * this fact should be recognized.
 */

static GLcontext* CCsw = NULL;

static void delete_window(GLint a);

void write_pixel(
        GLcontext* ctx, GLint x, GLint y, GLdepth z,
        GLint red, GLint green, GLint blue, GLint alpha);
void write_blended_pixel(
        GLcontext* ctx, GLint x, GLint y,
        GLint red, GLint green, GLint blue, GLint alpha);

void general_rgb_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pvert);
void doublestep_rgb_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pvert);

void tex_palette(gl_texture_object* tex);

//in triangle.c and scan.c
extern void init_color_spanners(GLcontext*);
extern void init_spanners(GLcontext*);

static void setup_tempmem(sw_context* sw)
{
    sw->byteTable = NULL;
}

static void setup_alpha(sw_context* sw)
{
    GLint source, dest;
    GLint t, s, component;
    GLint alpha;

//    sw->alphaTable_oneminussrcalpha = (GLubyte*)CCsw->AllocFunc(32*32*32);
//    sw->alphaTable_one = (GLubyte*)CCsw->AllocFunc(32*32*32);
    sw->alphaTable_oneminussrcalpha = (GLubyte*)malloc(32*32*32);
    sw->alphaTable_one = (GLubyte*)malloc(32*32*32);

    //GL_ONE

    alphaTable = sw->alphaTable_one;

    for (source = 0; source < 32; source++)
    {
        for (dest = 0; dest < 32; dest++)
        {
            for (alpha = 0; alpha < 256; alpha += (256/32))
            {
                t = alpha;
                s = 256;
                component = ((8*source) * t + (8*dest) * s) >> 8;
                component = MIN2(component, 255);

                alphaTable[(alpha/8)*32*32 + source*32 + dest] = (GLubyte)component;
            }
        }
    }

    //GL_ONE_MINUS_SRC_ALPHA

    alphaTable = sw->alphaTable_oneminussrcalpha;

    for (source = 0; source < 32; source++)
    {
        for (dest = 0; dest < 32; dest++)
        {
            for (alpha = 0; alpha < 256; alpha += (256/32))
            {
                t = alpha;
                s = 256 - t;
                component = ((8*source) * t + (8*dest) * s) >> 8;
                component = MIN2(component, 255);

                alphaTable[(alpha/8)*32*32 + source*32 + dest] = (GLubyte)component;
            }
        }
    }
}

static void clear_depthbuffer(GLcontext* ctx)
{
    GLdepth clear = (GLdepth)(ctx->DepthClear * DEPTH_SCALE);

    if (ctx->DepthBuffer == NULL)
    {
        gl_problem(ctx, "gl_clear_depthbuffer can't clear NULL");
        return;
    }

    if ((sizeof(GLdepth) == 2) && ((clear & 0xFF) == (clear >> 8)))
    {
        MEMSET(ctx->DepthBuffer,
               clear >> 8,
               ctx->Buffer.Width
               * ctx->Buffer.Height
               * sizeof(GLdepth));
    }
    else
    {
        GLdepth* d = ctx->DepthBuffer;
        GLint n = ctx->Buffer.Width * ctx->Buffer.Height;
#if FAST_DEPTHBUFFER
        if (!g_HasBeenCleared)
        {
            g_HasBeenCleared = GL_TRUE;
        }
        else
        {
            g_DepthMask -= 0x01000000;
            if (g_DepthMask > 0x00ffffff)
            {
                return;
            }
        }
#endif
        g_DepthMask = 0x3f000000;
        MEMSET(ctx->DepthBuffer, 0xff, n*sizeof(GLdepth));
    }
}

static void init_linefuncs(GLcontext* ctx)
{
    switch (ctx->Buffer.PixelType)
    {
    case GL_RGB555:
        _flat_line = flat_555_line;
        _flat_z_line = flat_555_z_line;
        _flat_blend_z_line = flat_555_blend_z_line;
        _flat_blend_line = flat_555_blend_line;
        _smooth_line = smooth_555_line;
        _smooth_z_line = smooth_555_z_line;
        _smooth_blend_line = smooth_555_blend_line;
        _smooth_blend_z_line = smooth_555_blend_z_line;
        break;
    case GL_RGB565:
        _flat_line = flat_565_line;
        _flat_z_line = flat_565_z_line;
        _flat_blend_line = flat_565_blend_line;
        _flat_blend_z_line = flat_565_blend_z_line;
        _smooth_line = smooth_565_line;
        _smooth_z_line = smooth_565_z_line;
        _smooth_blend_line = smooth_565_blend_line;
        _smooth_blend_z_line = smooth_565_blend_z_line;
    }
}

static void setup_line(GLcontext* ctx)
{
    sw_context* sw = (sw_context*)ctx->DriverCtx;

    if (ctx->AlphaTest ||
        ctx->ScissorTest ||
        ctx->Speedy ||
        (ctx->LineWidth != 1.0f) ||
        (ctx->Buffer.Depth > 16))
    {
        if (ctx->DepthTest || g_doAlpha || ctx->LineStipple)
        {
            sw->line_func = general_rgb_line;
        }
        else
        {
            sw->line_func = doublestep_rgb_line;
        }
    }
    else
    {
        GLint index = 0;
        if (ctx->DepthTest) index |= SW_LINE_Z;
        if (ctx->Blend) index |= SW_LINE_BLEND;
        if (ctx->ShadeModel == GL_SMOOTH) index |= SW_LINE_SMOOTH;
        sw->line_func = *line_funcs[index];
    }
}

static void setup_triangle(GLcontext* ctx)
{
    sw_context* sw = (sw_context*)ctx->DriverCtx;

    if (ctx->Buffer.Depth > 16 ||
        ctx->LightingAdjust != 0.0f)
    {
        sw->generalTriangle = GL_TRUE;
        sw->generalPerspectiveTriangle = GL_TRUE;
    }
    else if (ctx->Blend ||
             ctx->AlphaTest ||
             ctx->Bias)
    {
        sw->generalPerspectiveTriangle = GL_TRUE;
        if ((!ctx->Blend && !ctx->Bias) ||
            (!ctx->AlphaTest && !ctx->Bias))
        {
            //there's an alphatesting & a blending z mapper
            if (ctx->DepthTest)
            {
                sw->generalTriangle = GL_FALSE;
            }
            else
            {
                sw->generalTriangle = GL_TRUE;
            }
        }
        else
        {
            sw->generalTriangle = GL_TRUE;
        }
    }
    else
    {
        sw->generalTriangle = GL_FALSE;
        sw->generalPerspectiveTriangle = GL_FALSE;
    }

    init_new_tex(ctx);
}

static void set_monocolor(
        GLcontext* ctx, GLint r, GLint g, GLint b, GLint a)
{
    //set data in ctx->DriverCtx
    sw_context* sw = (sw_context*)ctx->DriverCtx;
    sw->MonocolorR = r;
    sw->MonocolorG = g;
    sw->MonocolorB = b;
    sw->MonocolorA = a;
}

// @@@

typedef struct EdgeT
{
    GLfixed x;
    GLfloat z;
} EdgeT;

static EdgeT scan[MAX_HEIGHT][2];

void scan_convert(GLcontext* ctx, GLuint v0, GLuint v1)
{
    vertex_buffer* VB = ctx->VB;
    GLint right;
    GLint y, ey;
    GLfloat dx, dy;
    GLfloat x;
    GLfixed fx, fdx;

    if (VB->Win[v0][1] == VB->Win[v1][1])
    {
        return;
    }

    if (VB->Win[v0][1] < VB->Win[v1][1])
    {
        right = 0;
    }
    else
    {
        GLuint temp = v0;
        v0 = v1;
        v1 = temp;
        right = 1;
    }

    dy = VB->Win[v1][1] - VB->Win[v0][1];
    dx = (VB->Win[v1][0] - VB->Win[v0][0]) / dy;

    x = VB->Win[v0][0];
    y = FAST_CEIL(VB->Win[v0][1]);
    ey = FAST_CEIL(VB->Win[v1][1]);

    dy = (GLfloat)y - VB->Win[v0][1];
    x += dy * dx;

    fx = FAST_TO_FIXED(x);
    fdx = FAST_TO_FIXED(dx);

    while (y < ey)
    {
        scan[y][right].x = fx;
        fx += fdx;
        y++;
    }
}

void scan_convert_z(GLcontext* ctx, GLuint v0, GLuint v1)
{
    vertex_buffer* VB = ctx->VB;
    GLint right;
    GLint y, ey;
    GLfloat dx, dy, dz;
    GLfloat x, z;
    GLfixed fx, fdx;

    if (VB->Win[v0][1] == VB->Win[v1][1])
    {
        return;
    }

    if (VB->Win[v0][1] < VB->Win[v1][1])
    {
        right = 0;
    }
    else
    {
        GLuint temp = v0;
        v0 = v1;
        v1 = temp;
        right = 1;
    }

    dy = VB->Win[v1][1] - VB->Win[v0][1];
    dx = (VB->Win[v1][0] - VB->Win[v0][0]) / dy;
    dz = (VB->Win[v1][2] - VB->Win[v0][2]) / dy;

    x = VB->Win[v0][0];
    y = FAST_CEIL(VB->Win[v0][1]);
    z = VB->Win[v0][2];
    ey = FAST_CEIL(VB->Win[v1][1]);

    dy = (GLfloat)y - VB->Win[v0][1];
    x += dy * dx;
    z += dy * dz;

    fx = FAST_TO_FIXED(x);
    fdx = FAST_TO_FIXED(dx);

    while (y < ey)
    {
        scan[y][right].x = fx;
        scan[y][right].z = z;
        fx += fdx;
        z += dz;
        y++;
    }
}

void barrett_triangle(GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
    vertex_buffer* VB = ctx->VB;
    GLfloat ymin, ymax;
    GLint i, y, ey;
    GLdepth zspan[MAX_WIDTH];
    GLfloat (*win)[3];
    GLubyte const* color;

    color = VB->Color[pv];
    win = VB->Win;

    ymin = ymax = VB->Win[v0][1];
    if      (win[v1][1] < ymin) ymin = win[v1][1];
    else if (win[v1][1] > ymax) ymax = win[v1][1];
    if      (win[v2][1] < ymin) ymin = win[v2][1];
    else if (win[v2][1] > ymax) ymax = win[v2][1];

    if (ctx->DepthTest)
    {
        scan_convert_z(ctx, v0, v2);
        scan_convert_z(ctx, v1, v0);
        scan_convert_z(ctx, v2, v1);
    }
    else
    {
        scan_convert(ctx, v0, v2);
        scan_convert(ctx, v1, v0);
        scan_convert(ctx, v2, v1);
    }

    y = FAST_CEIL(ymin);
    ey = FAST_CEIL(ymax);

    while (y < ey)
    {
        GLuint n;
        GLint right, left;
        GLint sx = FixedToInt(scan[y][0].x);
        GLint ex = FixedToInt(scan[y][1].x);

        if (sx < ex)
        {
            n = ex - sx;
            left = 0;
            right = 1;
        }
        else
        {
            n = sx - ex;
            sx = ex;
            left = 1;
            right = 0;
        }

        if (n == 0)
        {
            y++;
            continue;
        }

        if (ctx->DepthTest)
        {
            GLdepth z0 = FAST_TO_INT(scan[y][left].z) | g_DepthMask;
            GLdepth dz = FAST_TO_INT((scan[y][right].z - scan[y][left].z) / n);
            for (i = 0; i < n; i++)
            {
                zspan[i] = z0;
                z0 += dz;
            }
        }

        gl_write_monocolor_span(
            ctx, n, sx, y, zspan,
            color[0], color[1], color[2], color[3],
            GL_POLYGON);

        y++;
    }
}

// @@@

static void draw_triangle(GLuint vl[], GLuint pv)
{
    if (CCsw->TexEnabled &&
        CCsw->TexBoundObject != NULL &&
        CCsw->TexBoundObject->created)
    {
        sw_context* sw = (sw_context*)CCsw->DriverCtx;

        if (CCsw->PerspectiveCorrect)
        {
            if (sw->generalPerspectiveTriangle)
            {
#if HECKER_GENERAL_PERSPECTIVE
                perspective_draw_triangle(CCsw, vl, pv, sw->triangle_func);
#else
                general_perspective_new_textured_triangle(CCsw, vl, pv);
#endif
            }
            else
            {
                new_perspective_textured_triangle(CCsw, vl, pv);
            }
        }
        else if (sw->generalTriangle)
        {
#if HECKER_GENERAL
            perspective_draw_triangle(CCsw, vl, pv, sw->triangle_func);
#else
            general_new_textured_triangle(CCsw, vl, pv);
#endif
        }
        else
        {
            new_textured_triangle(CCsw, vl, pv);
        }
    }
    else
    {
#ifdef TRIANGLE_FUNC
        TRIANGLE_FUNC(CCsw, vl[0], vl[1], vl[2], pv);
#else
        new_triangle(CCsw, vl, pv);
#endif
    }
}

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

static void draw_line(GLuint vert0, GLuint vert1, GLuint pvert)
{
    sw_context* sw = (sw_context*)CCsw->DriverCtx;
    sw->line_func(CCsw, vert0, vert1, pvert);
}

static void draw_pixel(GLint x, GLint y, GLdepth z)
{
    sw_context* sw = (sw_context*)CCsw->DriverCtx;
    if (g_doAlpha)
    {
        write_pixel(CCsw, x, y, z,
                    sw->MonocolorR, sw->MonocolorG, sw->MonocolorB, sw->MonocolorA);
    }
    else
    {
        write_pixel(CCsw, x, y, z,
                    sw->MonocolorR, sw->MonocolorG, sw->MonocolorB, 255);
    }
}

static void draw_bitmap(
    GLcontext* ctx,
    GLsizei width, GLsizei height,
    GLfloat xb0, GLfloat yb0, GLfloat xb1, GLfloat yb1)
{
    GLint bx, by;
    GLint px, py, pz;
    GLubyte* ptr;

    GLint r, g, b, a;
    r = (GLint)(ctx->Current.RasterColor[0] * ctx->Buffer.rscale);
    g = (GLint)(ctx->Current.RasterColor[1] * ctx->Buffer.gscale);
    b = (GLint)(ctx->Current.RasterColor[2] * ctx->Buffer.bscale);
    a = (GLint)(ctx->Current.RasterColor[3] * ctx->Buffer.ascale);

    CCsw = ctx;
    set_monocolor(CCsw, r, g, b, a);

    px = (GLint)((ctx->Current.RasterPos[0] - xb0) + 0.0f);
    py = (GLint)((ctx->Current.RasterPos[1] - yb0) + 0.0f);
    pz = (GLint)(ctx->Current.RasterPos[2] * DEPTH_SCALE);
    ptr = ctx->Current.Bitmap;

    for (by = 0; by < height; by++)
    {
        GLubyte bitmask;

        bitmask = 128;
        for (bx = 0; bx < width; bx++)
        {
            if (*ptr & bitmask)
            {
                draw_pixel(px+bx, py+by, 0);
            }
            bitmask >>= 1;
            if (bitmask == 0)
            {
                ptr++;
                bitmask = 128;
            }
        }

        if (bitmask != 128)
            ptr++;
    }

    ctx->Current.RasterPos[0] += xb1;
    ctx->Current.RasterPos[1] += yb1;
}

#define FAST_RGB8_ALPHA 1

#define CRED565(c)   ((c) >> 11)
#define CGREEN565(c) ((((c) >> 5) & 0x3f) >> 1)
#define CBLUE565(c)  ((c) & 0x1f)

#define CRED555(c)   ((c) >> 10)
#define CGREEN555(c) (((c) >> 5) & 0x1f)
#define CBLUE555(c)  ((c) & 0x1f)

static void draw_pixels_rgb8(
    GLubyte r, GLubyte g, GLubyte b,
    GLint px, GLint py,
    GLsizei width, GLsizei height, GLubyte* pixels)
{
    GLint x, y;
    GLint xlead, bufpos, stride;
    GLubyte* pb;
    GLubyte* bytebuf;
    GLushort* shortbuf;
#if FAST_RGB8_ALPHA
    GLubyte dr, dg, db, alpha;
    GLushort pixel[16];
#else
    GLint ir, ig, ib, alpha;
#endif

    pb = pixels;

    //clip left
    if (px < 0)
    {
        xlead = -px;
        width += px;
        px = 0;
        if (width <= 0)
        {
            return;
        }
    }
    else
    {
        xlead = 0;
    }

    //clip right
    if (px+width >= CCsw->Buffer.Width)
    {
        width -= (px+width) - CCsw->Buffer.Width + 1;
        if (width <= 0)
        {
            return;
        }
    }

    bytebuf = CCsw->FrameBuffer;
    bufpos = CCsw->Buffer.Pitch*((CCsw->Buffer.Height - 1) - py) + 2*px;
    stride = CCsw->scrMultByte[1] - CCsw->scrMultByte[0];

#if FAST_RGB8_ALPHA
    if (CCsw->Buffer.PixelType == GL_RGB565)
    {
        for (y = 0; y < 16; y++)
        {
            alpha = y * 16;
            dr = (r * alpha) >> 8;
            dg = (g * alpha) >> 8;
            db = (b * alpha) >> 8;
            pixel[y] = FORM_RGB565(dr, dg, db);
        }
    }
    else
    {
        for (y = 0; y < 16; y++)
        {
            alpha = y * 16;
            dr = (r * alpha) >> 8;
            dg = (g * alpha) >> 8;
            db = (b * alpha) >> 8;
            pixel[y] = FORM_RGB555(dr, dg, db);
        }
    }
#else
    ir = (r & 0xf8) << 2;
    ig = (g & 0xf8) << 2;
    ib = (b & 0xf8) << 2;
#endif

    for (y = 0; y < height; y++, bufpos += stride)
    {
        if (CCsw->Speedy && rglGetSkip(py+y))
        {
            pb += width + xlead;
            continue;
        }

        if ((py+y) < 0 || (py+y) >= CCsw->Buffer.Height)
        {
            pb += width + xlead;
            continue;
        }

        if (CCsw->ScissorTest)
        {
            if ((py+y) < CCsw->ScissorY || (py+y) >= (CCsw->ScissorY + CCsw->ScissorHeight))
            {
                pb += width + xlead;
                continue;
            }
        }

        shortbuf = (GLushort*)(bytebuf + bufpos + xlead);
        pb += xlead;

        for (x = width; x > 0; x--, pb++, shortbuf++)
        {
            if (*pb)
            {
#if FAST_RGB8_ALPHA
                *shortbuf = pixel[*pb];
#else
                alpha = ((*pb * 16) & 0xf8) << 7;
                if (CCsw->Buffer.PixelType == GL_RGB565)
                {
                    *shortbuf = FORM_RGB565(
                        alphaTable[alpha + ir + CRED565(*shortbuf)],
                        alphaTable[alpha + ig + CGREEN565(*shortbuf)],
                        alphaTable[alpha + ib + CBLUE565(*shortbuf)]);
                }
                else
                {
                    *shortbuf = FORM_RGB555(
                        alphaTable[alpha + ir + CRED555(*shortbuf)],
                        alphaTable[alpha + ig + CGREEN555(*shortbuf)],
                        alphaTable[alpha + ib + CBLUE555(*shortbuf)]);
                }
#endif
            }
        }
    }
}

static void draw_pixels(
    GLcontext* ctx,
    GLsizei width, GLsizei height,
    GLenum format, GLenum type)
{
    GLint px, py;
    GLubyte* pb;
    int x, y, step, pitch;
    GLboolean rev;
    extern void rglswDrawAnimatic(int, int, unsigned char*);

    pitch = width;
    rev = GL_FALSE;

    switch (format)
    {
    case GL_RGB:
    case GL_RGBA:
    case GL_RGB8:
    case GL_RGBA16:
        break;
    default:
        //error condition
        return;
    }

    if (format == GL_RGBA &&
        width == ctx->Buffer.Width &&
        height == ctx->Buffer.Height)
    {
        draw_background(ctx->Current.Bitmap);
        return;
    }

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
        if (ctx->TexBoundObject->Width != width ||
            ctx->TexBoundObject->Height != height)
        {
            //error condition
            return;
        }
        px = (GLint)ctx->Current.TexCoord[0];
        py = (GLint)ctx->Current.TexCoord[1];
        pb = ctx->TexBoundObject->Data;

        if (px < 0)
        {
            px = -px;
            rev = GL_TRUE;
        }
        if (py < 0)
        {
            py = (ctx->Buffer.Height - 1) + py;
        }
    }

    if (format == GL_RGB8)
    {
        GLubyte r, g, b;

        r = RUB(ctx->Current.RasterColor[0]);
        g = RUB(ctx->Current.RasterColor[1]);
        b = RUB(ctx->Current.RasterColor[2]);

        if (ctx->Buffer.Depth < 24)
        {
            draw_pixels_rgb8(r, g, b, px, py, width, height, pb);
        }
        else
        {
            for (y = 0; y < height; y++)
            {
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

                        write_pixel(ctx, px+x, py+y, 0, dr, dg, db, 255);
#else
                        write_blended_pixel(ctx, px+x, py+y, r, g, b, *pb * 16);
#endif
                    }
                }
            }
        }

        return;
    }

    if (format == GL_RGBA16 && width == 640 && height == 480)
    {
        rglswDrawAnimatic(px, py, pb);
        goto DRAWPIX_FINI;
    }

    if ((!ctx->Blend && !ctx->AlphaTest) ||
        (!ctx->Blend && ctx->AlphaTest && format == GL_RGB))
    {
        GLubyte*  bytebuf;
        GLushort* shortbuf;
        GLint xlead;

        step = (format == GL_RGBA) ? 4 : 3;

        //clip left
        if (px < 0)
        {
            xlead = -px;
            width += px;
            px = 0;
            if (width <= 0)
            {
                goto DRAWPIX_FINI;
            }
        }
        else
        {
            xlead = 0;
        }

        //clip right
        if (px+width > ctx->Buffer.Width)
        {
            width -= (px+width) - ctx->Buffer.Width + 1;
            if (width <= 0)
            {
                goto DRAWPIX_FINI;
            }
        }

        switch (ctx->Buffer.PixelType)
        {
        case GL_RGB565:
            for (y = 0; y < height; y++)
            {
                if (ctx->Speedy && rglGetSkip(py+y))
                {
                    pb += step * (pitch + xlead);
                    continue;
                }

                if (py+y < 0 || py+y >= ctx->Viewport.Height)
                {
                    pb += step * (pitch + xlead);
                    continue;
                }

                bytebuf = ctx->FrameBuffer + ctx->scrMultByte[py+y] + ctx->Buffer.ByteMult*px;
                shortbuf = (GLushort*)bytebuf + xlead;
                pb += step * xlead;

                for (x = 0; x < width; x++, pb += step)
                {
                    shortbuf[x] = FORM_RGB565(pb[0], pb[1], pb[2]);
                }
            }
            break;

        case GL_RGB555:
            for (y = 0; y < height; y++)
            {
                if (ctx->Speedy && rglGetSkip(py+y))
                {
                    pb += step * (pitch + xlead);
                    continue;
                }

                if (py+y < 0 || py+y >= ctx->Viewport.Height)
                {
                    pb += step * (pitch + xlead);
                    continue;
                }

                bytebuf = ctx->FrameBuffer + ctx->scrMultByte[py+y] + ctx->Buffer.ByteMult*px;
                shortbuf = (GLushort*)bytebuf + xlead;
                pb += step * xlead;

                for (x = 0; x < width; x++, pb += step, shortbuf++)
                {
                    *shortbuf = FORM_RGB555(pb[0], pb[1], pb[2]);
                }
            }
            break;

        default:
            gl_error(ctx, GL_INVALID_VALUE, "draw_pixels(PixelType)");
        }
    }
#if 1
    else if (ctx->AlphaTest && !ctx->Blend)
    {
        GLubyte*  bytebuf;
        GLushort* shortbuf;

//        ASSERT(format == GL_RGBA);
        step = 4;

        switch (ctx->Buffer.PixelType)
        {
        case GL_RGB565:
            for (y = 0; y < height; y++)
            {
                if (ctx->Speedy && rglGetSkip(py+y))
                {
                    pb += step * width;
                    continue;
                }
                if (py+y < 0 || py+y >= ctx->Viewport.Height)
                {
                    pb += step * width;
                    continue;
                }
                if (rev)
                {
                    bytebuf = ctx->FrameBuffer +
                              ctx->scrMultByte[py+((height-1)-y)] +
                              ctx->Buffer.ByteMult*px;
                }
                else
                {
                    bytebuf = ctx->FrameBuffer + ctx->scrMultByte[py+y] + ctx->Buffer.ByteMult*px;
                }
                shortbuf = (GLushort*)bytebuf;
                for (x = 0; x < width; x++, pb += step, shortbuf++)
                {
                    if (px+x < 0 || px+x >= ctx->Viewport.Width)
                    {
                        continue;
                    }
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
                    }
                    *shortbuf = FORM_RGB565(pb[0], pb[1], pb[2]);
                }
            }
            break;

        case GL_RGB555:
            for (y = 0; y < height; y++)
            {
                if (ctx->Speedy && rglGetSkip(py+y))
                {
                    pb += step * width;
                    continue;
                }
                if (py+y < 0 || py+y >= ctx->Viewport.Height)
                {
                    pb += step * width;
                    continue;
                }
                if (rev)
                {
                    bytebuf = ctx->FrameBuffer +
                              ctx->scrMultByte[py+((height-1)-y)] +
                              ctx->Buffer.ByteMult*px;
                }
                else
                {
                    bytebuf = ctx->FrameBuffer + ctx->scrMultByte[py+y] + ctx->Buffer.ByteMult*px;
                }
                shortbuf = (GLushort*)bytebuf;
                for (x = 0; x < width; x++, pb += step, shortbuf++)
                {
                    if (px+x < 0 || px+x >= ctx->Viewport.Width)
                    {
                        continue;
                    }
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
                    }
                    *shortbuf = FORM_RGB555(pb[0], pb[1], pb[2]);
                }
            }
            break;

        default:
            gl_error(ctx, GL_INVALID_VALUE, "draw_pixels(PixelType)");
        }
    }
#endif
    else
    {
        if (format == GL_RGBA)
        {
            if (rev)
            {
                for (y = 0; y < height; y++)
                {
                    for (x = 0; x < width; x++, pb += 4)
                    {
                        write_pixel(ctx, px+x, py+((height-1)-y), 0,
                                    pb[0], pb[1], pb[2], pb[3]);
                    }
                }
            }
            else
            {
                for (y = 0; y < height; y++)
                {
                    for (x = 0; x < width; x++, pb += 4)
                    {
                        write_pixel(ctx, px+x, py+y, 0, pb[0], pb[1], pb[2], pb[3]);
                    }
                }
            }
        }
        else
        {
            for (y = 0; y < height; y++)
            {
                for (x = 0; x < width; x++, pb += 3)
                {
                    write_pixel(ctx, px+x, py+y, 0, pb[0], pb[1], pb[2], 255);
                }
            }
        }
    }

DRAWPIX_FINI:
    ;
}

static void read_pixels(
    GLcontext* ctx, GLint x, GLint y, GLsizei width, GLsizei height,
    GLenum format, GLenum type)
{
    GLushort* sp5;
    GLubyte*  sp8;
    GLubyte*  bp;
    GLint     iy, ix;

    if (width  == ctx->Buffer.Width &&
        height == ctx->Buffer.Height)
    {
        screenshot(ctx->Current.Bitmap);
        return;
    }

    bp = ctx->Current.Bitmap;

    switch (ctx->Buffer.PixelType)
    {
    case GL_RGB565:
        for (iy = 0; iy < height; iy++)
        {
            sp5 = (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[iy]);
            for (ix = 0; ix < width; ix++, bp += 4, sp5++)
            {
                bp[0] = CBLUE565(*sp5);
                bp[1] = CGREEN565(*sp5);
                bp[2] = CRED565(*sp5);
                bp[3] = 255;
            }
        }
        break;

    case GL_RGB555:
        for (iy = 0; iy < height; iy++)
        {
            sp5 = (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[iy]);
            for (ix = 0; ix < width; ix++, bp += 4, sp5++)
            {
                bp[0] = CBLUE565(*sp5);
                bp[1] = CGREEN565(*sp5);
                bp[2] = CRED555(*sp5);
                bp[3] = 255;
            }
        }
        break;

    case GL_RGB888:
    case GL_BGR888:
        MEMSET(bp, 0, 4*width*height);
        break;

    default:
        gl_error(ctx, GL_INVALID_VALUE, "read_pixels(PixelType)");
    }
}

#define R 0
#define G 1
#define B 2
static void draw_background(GLubyte* pixels)
{
    GLint width, height, pitch;
    GLint x, y;
    GLcontext* ctx = CCsw;
    GLushort* sp5;
    GLubyte*  sp8;
    GLubyte*  bp;

    pitch  = ctx->Buffer.Pitch;
    width  = ctx->Buffer.Width;
    height = ctx->Buffer.Height;

    bp = pixels;

    switch (ctx->Buffer.PixelType)
    {
    case GL_RGB565:
        for (y = 0; y < height; y++, bp += 4*width)
        {
            sp5 = (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[y]);
            for (x = 0; x < width; x++)
            {
                sp5[x] = FORM_RGB565(bp[4*x + R], bp[4*x + G], bp[4*x + B]);
            }
        }
        break;

    case GL_RGB555:
        for (y = 0; y < height; y++, bp += 4*width)
        {
            sp5 = (GLushort*)(ctx->FrameBuffer + ctx->scrMultByte[y]);
            for (x = 0; x < width; x++)
            {
                sp5[x] = FORM_RGB555(bp[4*x + R], bp[4*x + G], bp[4*x + B]);
            }
        }
        break;

    case GL_BGR888:
        for (y = 0; y < height; y++)
        {
            sp8 = ctx->FrameBuffer + ctx->scrMultByte[y];
            for (x = 0; x < width; x++, bp += 4, sp8 += 3)
            {
                sp8[0] = bp[B];
                sp8[1] = bp[G];
                sp8[2] = bp[R];
            }
        }
        break;

    case GL_RGB888:
        for (y = 0; y < height; y++)
        {
            sp8 = ctx->FrameBuffer + ctx->scrMultByte[y];
            for (x = 0; x < width; x++, bp += 4, sp8 += 3)
            {
                sp8[0] = bp[R];
                sp8[1] = bp[G];
                sp8[2] = bp[B];
            }
        }
        break;
    }
}
#undef R
#undef G
#undef B

static void deactivate()
{
    rglDDrawActivate(0);
}

static void activate()
{
    rglDDrawActivate(1);
}

DLL void shutdown_driver(GLcontext* ctx)
{
    sw_context* sw = (sw_context*)ctx->DriverCtx;
//    deactivate();

    ditherShutdown();
    stippleShutdown();

    if (sw == NULL)
    {
        return;
    }

    if (ctx->DepthBuffer != NULL)
    {
        ctx->FreeFunc(ctx->DepthBuffer);
        ctx->DepthBuffer = NULL;
    }
    clear_texobjs();

    alphaTable = NULL;

    if (sw->byteTable != NULL)
    {
        free(sw->byteTable);
        sw->byteTable = NULL;
    }
    if (sw->alphaTable_oneminussrcalpha != NULL)
    {
//        ctx->FreeFunc(sw->alphaTable_oneminussrcalpha);
        free(sw->alphaTable_oneminussrcalpha);
        sw->alphaTable_oneminussrcalpha = NULL;
    }
    if (sw->alphaTable_one != NULL)
    {
//        ctx->FreeFunc(sw->alphaTable_one);
        free(sw->alphaTable_one);
        sw->alphaTable_one = NULL;
    }

    ctx->GammaAdjust = 0.15f;

    if (ctx->DriverCtx != NULL)
    {
        ctx->FreeFunc(ctx->DriverCtx);
        ctx->DriverCtx != NULL;
    }

    delete_window(0);
}

static void allocate_depthbuffer(GLcontext* ctx)
{
    GLdepth* db = (GLdepth*)ctx->AllocFunc(sizeof(GLdepth)*ctx->Buffer.Width*ctx->Buffer.Height);
    if (!db)
    {
	    gl_problem(NULL, "gl_allocate_depthbuffer");
    }
    ctx->DepthBuffer = db;
}

#if 0
static FILE* _logfile = NULL;
static void LOG(char* s)
{
    if (_logfile == NULL)
    {
        _logfile = fopen("swlog.dat", "wt");
    }
    if (_logfile != NULL)
    {
        fprintf(_logfile, s);
        fflush(_logfile);
    }
}
#endif

static void setup_raster(GLcontext* ctx)
{
    sw_context* sw = (sw_context*)ctx->DriverCtx;

    g_doAlpha = ctx->Blend;
    g_doAlphaTest = ctx->AlphaTest;
    g_doBias = ctx->Bias;

    if (g_doBias)
    {
        g_redBias = ctx->ByteBias[0];
        g_greenBias = ctx->ByteBias[1];
        g_blueBias = ctx->ByteBias[2];
    }

    if (ctx->LineSmooth)
    {
        g_doAlpha = GL_FALSE;
    }

    if (ctx->BlendDst == GL_ONE)
    {
        alphaTable = sw->alphaTable_one;
    }
    else
    {
        alphaTable = sw->alphaTable_oneminussrcalpha;
    }

    if (ctx->TexEnabled)
    {
        init_spanners(ctx);
    }
    else
    {
        choose_newtriangle_rasterizers(ctx);
    }

    if (ctx->DepthWrite)
    {
        gl_depthtest_span_less = gl_depthtest_span_less_write;
        gl_depthtest_alphatest_span_less = gl_depthtest_alphatest_span_less_write;
    }
    else
    {
        gl_depthtest_span_less = gl_depthtest_span_less_nowrite;
        gl_depthtest_alphatest_span_less = gl_depthtest_alphatest_span_less_nowrite;
    }

#if 0
    if (g_doBias)
    {
        char buf[128];
        sprintf(buf, "bias - a %d / at %d / sm %d / t %d / p %d\n",
                g_doAlpha, g_doAlphaTest,
                (ctx->ShadeModel == GL_SMOOTH),
                ctx->TexEnabled, ctx->IsPaletted);
        LOG(buf);
    }
#endif
}

sw_texobj* new_sw_texobj()
{
    sw_texobj* sw = (sw_texobj*)CCsw->AllocFunc(sizeof(sw_texobj));
    sw->BlendedData = NULL;
    return sw;
}

static void init_sw_texobj(gl_texture_object* tex)
{
    sw_texobj* sw = (sw_texobj*)tex->DriverData;
    if (sw == NULL)
        return;

#if 1
    if (sw->BlendedData != NULL)
    {
        CCsw->FreeFunc(sw->BlendedData);
        sw->BlendedData = NULL;
    }
#endif
}

static void clear_sw_texobj(gl_texture_object* tex)
{
    sw_texobj* sw = (sw_texobj*)tex->DriverData;
    if (sw == NULL)
        return;

#if 1
    if (sw->BlendedData != NULL)
    {
        CCsw->FreeFunc(sw->BlendedData);
        sw->BlendedData = NULL;
    }
#endif
}

static void teximg(gl_texture_object* tex, GLint level, GLint internalFormat)
{
    GLint width, height;

    width  = tex->Width;
    height = tex->Height;

    if (tex->Data == NULL)
        return;

    if (tex->Data2)
    {
        CCsw->FreeFunc(tex->Data2);
        tex->Data2 = NULL;
    }
}

static void init_texobjs()
{
    unsigned int i;
    hashtable* table;
    hash_t* element;
    gl_texture_object* tex;

    table = rglGetTexobjs();    //get the texture object hashtable
    if (table == NULL || table->maxkey == 0)
    {
        //no texture object hashtable, exit
        return;
    }
    for (i = 0; i < TABLE_SIZE; i++)
    {
        //walk the chained hash entries
        element = table->table[i];
        while (element != NULL)
        {
            tex = (gl_texture_object*)element->data;
            if (tex != NULL)
            {
                //texture object exists, setup driver data
                if (tex->DriverData != NULL)
                {
                    CCsw->FreeFunc(tex->DriverData);
                }
                tex->DriverData = new_sw_texobj();

                init_sw_texobj(tex);

                //create the texture object if it is valid
                if (tex->created)
                {
                    teximg(tex, 0, tex->Format);
                    if (tex->Format == GL_COLOR_INDEX)
                    {
                        tex_palette(tex);
                    }
                }
            }

            //next element
            element = element->next;
        }
    }
}

static void clear_texobjs()
{
    unsigned int i;
    hashtable* table;
    hash_t* element;
    gl_texture_object* tex;

    table = rglGetTexobjs();
    if (table == NULL || table->maxkey == 0)
    {
        return;
    }

    for (i = 0; i < TABLE_SIZE; i++)
    {
        //walk the chained hash entries
        element = table->table[i];
        while (element != NULL)
        {
            tex = (gl_texture_object*)element->data;
            if (tex != NULL)
            {
                //valid texture object, free driver memory
                if (tex->Data2 != NULL)
                {
                    CCsw->FreeFunc(tex->Data2);
                    tex->Data2 = NULL;
                }

                if (tex->DriverData != NULL)
                {
                    clear_sw_texobj(tex);
                    CCsw->FreeFunc(tex->DriverData);
                    tex->DriverData = NULL;
                }
            }

            //next element
            element = element->next;
        }
    }
}

static void screenshot(GLubyte* buf)
{
    GLcontext* ctx = CCsw;
    GLint x, y;
    GLubyte* bp = buf;
    GLubyte* sp8;
    GLushort* sp5;
    GLint r, g, b;
    GLint pitch;
    GLint width, height;

    pitch  = ctx->Buffer.Pitch;
    width  = ctx->Buffer.Width;
    height = ctx->Buffer.Height;

    switch (ctx->Buffer.PixelType)
    {
    case GL_BGR888:
        for (y = 0; y < height; y++)
        {
            sp8 = ctx->FrameBuffer + ((height-1)-y)*pitch;
            for (x = 0; x < width; x++)
            {
                bp[2] = sp8[0];
                bp[1] = sp8[1];
                bp[0] = sp8[2];
                bp += 3;
                sp8 += 3;
            }
        }
        break;
    case GL_RGB888:
        for (y = 0; y < height; y++)
        {
            sp8 = ctx->FrameBuffer + ((height-1)-y)*pitch;
            for (x = 0; x < width; x++)
            {
                bp[2] = sp8[2];
                bp[1] = sp8[1];
                bp[0] = sp8[0];
                bp += 3;
                sp8 += 3;
            }
        }
        break;
    case GL_RGB565:
        for (y = 0; y < height; y++)
        {
            sp5 = (GLushort*)(ctx->FrameBuffer + ((height-1)-y)*pitch);
            for (x = 0; x < width; x++)
            {
                r = (*sp5 >> 11) & 0x1f;
                g = (*sp5 >> 5) & 0x3f;
                b = *sp5 & 0x1f;
                r <<= 3;
                g <<= 2;
                b <<= 3;
                bp[0] = r;
                bp[1] = g;
                bp[2] = b;
                bp += 3;
                sp5++;
            }
        }
        break;
    case GL_RGB555:
        for (y = 0; y < height; y++)
        {
            sp5 = (GLushort*)(ctx->FrameBuffer + ((height-1)-y)*pitch);
            for (x = 0; x < width; x++)
            {
                r = (*sp5 >> 10) & 0x1f;
                g = (*sp5 >> 5) & 0x1f;
                b = *sp5 & 0x1f;
                r <<= 3;
                g <<= 3;
                b <<= 3;
                bp[0] = r;
                bp[1] = g;
                bp[2] = b;
                bp += 3;
                sp5++;
            }
        }
        break;
    }
}

static void gamma_up()
{
    CCsw->GammaAdjust += 0.05f;
    if (CCsw->GammaAdjust > 1.0f)
    {
        CCsw->GammaAdjust = 1.0f;
    }
}

static void gamma_dn()
{
    CCsw->GammaAdjust -= 0.05f;
    if (CCsw->GammaAdjust < 0.0f)
    {
        CCsw->GammaAdjust = 0.0f;
    }
}

static void tex_palette(gl_texture_object* tex)
{
    GLint i;
    GLubyte* tp;
    sw_texobj* sw;

    if (!CCsw->UsingSharedPalette && tex)
    {
        if (!tex->DriverData)
        {
            tex->DriverData = new_sw_texobj();
            init_sw_texobj(tex);
        }

        sw = (sw_texobj*)tex->DriverData;

        if (CCsw->Buffer.PixelType == GL_RGB565)
        {
            for (i = 0, tp = tex->Palette; i < 256; i++, tp += 4)
            {
                sw->Palette[i] = FORM_RGB565(tp[0], tp[1], tp[2]);
            }
        }
        else if (CCsw->Buffer.PixelType == GL_RGB555)
        {
            for (i = 0, tp = tex->Palette; i < 256; i++, tp += 4)
            {
                sw->Palette[i] = FORM_RGB555(tp[0], tp[1], tp[2]);
            }
        }

        for (i = 0, tp = tex->Palette; i < 256; i++, tp += 4)
        {
            sw->Palette444[i] = FORM_RGB444(tp[0] >> 4, tp[1] >> 4, tp[2] >> 4);
        }
    }
}

#define ABSF(n) ((GLfloat)fabs((double)n))

/*
 * apply fog by adjusting vertex colouring
 */
void sw_fog_color_vertices(GLcontext* ctx, GLuint n, GLfloat v[][4], GLubyte color[][4])
{
    GLint i;
    GLfloat d, f, one_minus_f;
    GLfloat fogr = ctx->FogColor[0] * ctx->Buffer.rscale;
    GLfloat fogg = ctx->FogColor[1] * ctx->Buffer.gscale;
    GLfloat fogb = ctx->FogColor[2] * ctx->Buffer.bscale;

    switch (ctx->FogMode)
    {
    case GL_LINEAR:
        d = 1.0f;// / (1.0f/*end*/ - 0.0f/*start*/);
        for (i = 0; i < n; i++)
        {
            f = CLAMP((1.0f/*end*/ - ABSF(v[i][2])/32767.0f) * d, 0.0f, 1.0f);
            one_minus_f = 1.0f - f;
            color[i][0] = FAST_TO_INT(f * (GLfloat)color[i][0] + one_minus_f * fogr);
            color[i][1] = FAST_TO_INT(f * (GLfloat)color[i][1] + one_minus_f * fogg);
            color[i][2] = FAST_TO_INT(f * (GLfloat)color[i][2] + one_minus_f * fogb);
        }
        break;

    case GL_EXP:
        d = -ctx->FogDensity;
        for (i = 0; i < n; i++)
        {
            f = exp(d * ABSF(v[i][2]/32767.0f));
            f = CLAMP(f, 0.0f, 1.0f);
            one_minus_f = 1.0f - f;
            color[i][0] = FAST_TO_INT(f * (GLfloat)color[i][0] + one_minus_f * fogr);
            color[i][1] = FAST_TO_INT(f * (GLfloat)color[i][1] + one_minus_f * fogg);
            color[i][2] = FAST_TO_INT(f * (GLfloat)color[i][2] + one_minus_f * fogb);
        }
        break;

    case GL_EXP2:
        d = -(ctx->FogDensity * ctx->FogDensity);
        for (i = 0; i < n; i++)
        {
            GLfloat z = ABSF(v[i][2]/32767.0f);
            f = exp(d * z*z);
            f = CLAMP(f, 0.0f, 1.0f);
            one_minus_f = 1.0f - f;
            color[i][0] = FAST_TO_INT(f * (GLfloat)color[i][0] + one_minus_f * fogr);
            color[i][1] = FAST_TO_INT(f * (GLfloat)color[i][1] + one_minus_f * fogg);
            color[i][2] = FAST_TO_INT(f * (GLfloat)color[i][2] + one_minus_f * fogb);
        }
        break;

    default:
        gl_problem(ctx, "invalid fog type");
    }
}

static void fog_vertices(GLcontext* ctx)
{
    vertex_buffer* VB = ctx->VB;

    sw_fog_color_vertices(ctx, VB->Count - VB->Start,
                          VB->Eye + VB->Start,
                          VB->Color + VB->Start);
}

static void flush()
{
    void updateFrame(void);
    updateFrame();
}

static void lock_buffer(GLcontext* ctx)
{
    void LockBuffers(void);
    LockBuffers();
}

static void unlock_buffer(GLcontext* ctx)
{
    void UnlockBuffers(void);
    UnlockBuffers();
}

static GLubyte* get_framebuffer(GLcontext* ctx)
{
    extern GLubyte* scrbuf;
    return scrbuf;
}

static void clear_colorbuffer(GLcontext* ctx)
{
    GLubyte* cp;
    GLubyte r,g,b;

    if (!ctx->ExclusiveLock)
    {
        lock_buffer(ctx);
        ctx->FrameBuffer = get_framebuffer(ctx);
    }

    if (ctx->FrameBuffer == NULL)
    {
        gl_problem(ctx, "gl_clear_colorbuffer can't clear NULL");
        return;
    }

    cp = ctx->FrameBuffer;
    r = ctx->ClearColorByte[0];
    g = ctx->ClearColorByte[1];
    b = ctx->ClearColorByte[2];

    if (ctx->ScissorTest)
    {
        GLint scissorY, scissorX, scissorHeight, scissorWidth;
        GLint startY, startX, endY, endX;
        GLint width;
        GLint y, x;
        GLboolean zero;
        GLushort fill;
        GLushort* buf;

        zero = (r == 0 && g == 0 && b == 0);

        scissorX = ctx->ScissorX;
        scissorY = ctx->ScissorY - 1;
        scissorWidth = ctx->ScissorWidth - 1;
        scissorHeight = ctx->ScissorHeight;

        startY = scissorY;
        endY = startY + scissorHeight;
        startX = scissorX;
        endX = scissorX + scissorWidth;

        width = scissorWidth;

//        startY = (ctx->Buffer.Height - 1) - startY;
//        endY = (ctx->Buffer.Height - 1) - endY;

        if (endY < startY)
        {
            y = startY;
            startY = endY;
            endY = y;
        }

        //FIXME
        while (endX > ctx->Buffer.Width)
        {
            endX--;
            width--;
        }
        while (endY > ctx->Buffer.Height)
        {
            endY--;
        }

        if (endX <= startX ||
            endY <= startY)
        {
            goto DONE_CLEARING;
        }

        if (ctx->Buffer.PixelType == GL_RGB555)
        {
            fill = FORM_RGB555(r, g, b);
        }
        else
        {
            fill = FORM_RGB565(r, g, b);
        }

        for (y = startY; y < endY; y++)
        {
            buf = CTX_FB_ADDRESS2(ctx, startX, y);
            if (zero)
            {
                MEMSET(buf, 0, 2*width);
            }
            else
            {
                for (x = 0; x < width; x++)
                {
                    buf[x] = fill;
                }
            }
        }
    }
    else
    {
#if C_CLEAR
        GLint x;
#else
        static GLint pitch;
#endif
        GLint y;
        static GLuint fill;
        static GLushort* dp;

#if !C_CLEAR
        pitch = (ctx->Buffer.Width * ctx->Buffer.ByteMult) >> 2;
#endif

        if (ctx->Buffer.PixelType == GL_RGB555)
        {
            fill = FORM_RGB555(r, g, b);
        }
        else
        {
            fill = FORM_RGB565(r, g, b);
        }
        fill |= fill << 16;

        for (y = 0; y < ctx->Buffer.Height; y++)
        {
            if (ctx->Speedy && !rglGetSkip(y))
            {
                continue;
            }
            dp = (GLushort*)(cp + ctx->Buffer.Pitch * y);
#if C_CLEAR
            for (x = 0; x < ctx->Buffer.Width; x++)
            {
                dp[x] = fill;
            }
#else
    #if defined (_MSC_VER)
            __asm
            {
                push ecx ;
                push eax ;
                push edi ;

                mov ecx, [pitch] ;
                mov eax, [fill] ;
                mov edi, [dp] ;

                align 16 ;

                rep stosd ;

                pop edi ;
                pop eax ;
                pop ecx ;
            }
    #elif defined (__GNUC__) && defined (__i386__)
            __asm__ __volatile__ (
                "    .align 16\n"
                "    rep stosl\n"
                :
                : "c" (pitch), "a" (fill), "D" (dp) );
    #endif
#endif
        }
    }

DONE_CLEARING:
    if (!ctx->ExclusiveLock)
    {
        unlock_buffer(ctx);
    }
}

static GLboolean create_window(GLint a, GLint b)
{
    //called by driver, not the GL (which merely saves parameters).
    //given parameters are irrelevant
    unsigned char rglswCreateWindow(GLint, GLint);
    return rglswCreateWindow(rglCreate0(), rglCreate1());
}

static void delete_window(GLint a)
{
    void rglswDeleteWindow(GLint);
    rglswDeleteWindow(a);
}

static void driver_caps(GLcontext* ctx)
{
    int rglGetScreenPitch(void);
    int rglGetScreenDepth(void);
    gl_pixel_type rglGetPixelType(void);

    ctx->Buffer.Pitch = rglGetScreenPitch();
    ctx->Buffer.Depth = rglGetScreenDepth();
    ctx->Buffer.PixelType = rglGetPixelType();
}

static void set_save_state(GLcontext* ctx, GLint on)
{
    void rglswSetSaveState(int);
    rglswSetSaveState(on);
}

static GLubyte* get_scratch(GLcontext* ctx)
{
    unsigned char* rglswGetScratch(void);
    return (GLubyte*)rglswGetScratch();
}

static void set_fullscreen(GLboolean fullscreen)
{
    void rglswSetWindowed(int);
    rglswSetWindowed((int)(!fullscreen));
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
    sw_context* sw = (sw_context*)CCsw->DriverCtx;

    /* perform internal initialization */
    setup_alpha(sw);
    setup_tempmem(sw);
    if (ctx->BlendDst == GL_ONE)
    {
        alphaTable = sw->alphaTable_one;
    }
    else
    {
        alphaTable = sw->alphaTable_oneminussrcalpha;
    }
    sw->generalTriangle = GL_TRUE;

    init_color_spanners(ctx);
    init_spanners(ctx);
    init_linefuncs(ctx);
    init_texobjs();
    ditherStartup(ctx);
    stippleStartup(ctx);

    return GL_TRUE;
}

static void super_clear(void)
{
    void rglswClearScratch(void);
    rglswClearScratch();
}

typedef void (*VoidFunc)(void);

DLL GLboolean init_driver(GLcontext* ctx)
{
    sw_context* sw;

    CCsw = ctx;
    g_HasBeenCleared = GL_FALSE;

    /* setup function pointers for the GL */
    ctx->DR.init_driver = init_driver;  //somewhat redundant
    ctx->DR.post_init_driver = post_init_driver;
    ctx->DR.shutdown_driver = shutdown_driver;
    ctx->DR.clear_depthbuffer = clear_depthbuffer;
    ctx->DR.clear_colorbuffer = clear_colorbuffer;
    ctx->DR.clear_both_buffers = NULL;
    ctx->DR.allocate_depthbuffer = allocate_depthbuffer;
    ctx->DR.allocate_colorbuffer = NULL;
    ctx->DR.setup_triangle = setup_triangle;
    ctx->DR.setup_line = setup_line;
    ctx->DR.setup_point = NULL;
    ctx->DR.setup_raster = setup_raster;
    ctx->DR.set_monocolor = set_monocolor;
    ctx->DR.flush = flush;
    ctx->DR.clear_color = NULL;
    ctx->DR.scissor = NULL;
    ctx->DR.draw_triangle = draw_triangle;
    ctx->DR.draw_triangle_array = NULL;
    ctx->DR.draw_quad = draw_quad;
    ctx->DR.draw_triangle_fan = NULL;
    ctx->DR.draw_triangle_strip = NULL;
    ctx->DR.draw_polygon = NULL;
    ctx->DR.draw_line = draw_line;
    ctx->DR.draw_pixel = draw_pixel;
    ctx->DR.draw_bitmap = (VoidFunc)draw_bitmap;
    ctx->DR.draw_pixels = draw_pixels;
    ctx->DR.read_pixels = read_pixels;
    ctx->DR.draw_clipped_triangle = NULL;
    ctx->DR.draw_clipped_polygon = NULL;
    ctx->DR.draw_point = NULL;
    ctx->DR.deactivate = deactivate;
    ctx->DR.activate = activate;
    ctx->DR.bind_texture = NULL;
    ctx->DR.tex_param = NULL;
    ctx->DR.tex_del = NULL;
    ctx->DR.tex_palette = tex_palette;
    ctx->DR.tex_img = teximg;
    ctx->DR.screenshot = screenshot;
    ctx->DR.gamma_up = gamma_up;
    ctx->DR.gamma_dn = gamma_dn;
    ctx->DR.chromakey = NULL;
    ctx->DR.super_clear = super_clear;
    ctx->DR.fastbind_set = NULL;
    ctx->DR.draw_background = draw_background;

    ctx->DR.lock_buffer = lock_buffer;
    ctx->DR.unlock_buffer = unlock_buffer;
    ctx->DR.get_framebuffer = get_framebuffer;

    ctx->DR.create_window = create_window;
    ctx->DR.delete_window = delete_window;
    ctx->DR.set_save_state = set_save_state;
    ctx->DR.get_scratch = get_scratch;

    ctx->DR.driver_caps = driver_caps;

    ctx->RequireLocking = GL_TRUE;

    ctx->DR.fog_vertices = fog_vertices;

    ctx->DR.feature_exists = feature_exists;

    /* allocate memory for ctx->DriverCtx */
    sw = ctx->DriverCtx = (sw_context*)ctx->AllocFunc(sizeof(sw_context));
    MEMSET(sw, 0, sizeof(sw_context));

    /* this just might work */
    set_fullscreen(rglGetFullscreen());
    if (rglGetTruecolor())
    {
        extern unsigned int useDirectDraw;
        useDirectDraw = 0;
    }
    if (!create_window(0, 0))
    {
        return GL_FALSE;
    }

//    activate();

    rglSetExtensionString("software");

    return GL_TRUE;
}

static void write_blended_pixel(
    GLcontext* ctx, GLint x, GLint y,
    GLint red, GLint green, GLint blue, GLint alpha)
{
    GLubyte* buf = ctx->FrameBuffer + ctx->scrMultByte[y] + ctx->Buffer.ByteMult*x;
    GLushort* sbuf = (GLushort*)buf;
    GLubyte bred, bgreen, bblue, balpha;

    if (ctx->ScissorTest)
    {
        if (y < ctx->ScissorY || y >= (ctx->ScissorY + ctx->ScissorHeight))
        {
            return;
        }
        if (x < ctx->ScissorX || x >= (ctx->ScissorX + ctx->ScissorWidth))
        {
            return;
        }
    }
    else
    {
        if (x < 0 || y < 0 || x >= (GLint)ctx->Buffer.Width || y >= (GLint)ctx->Buffer.Height)
        {
            return;
        }
    }

    if (alpha == 0)
    {
        return;
    }

    bred = (GLubyte)red;
    bgreen = (GLubyte)green;
    bblue = (GLubyte)blue;
    balpha = (GLubyte)alpha;

    switch (ctx->Buffer.PixelType)
    {
    case GL_RGB888:
        if (alpha < 240)
        {
            gl_blend_24bit(ctx, &bred, &bgreen, &bblue, balpha, buf);
        }
        buf[0] = bred;
        buf[1] = bgreen;
        buf[2] = bblue;
        break;
    case GL_BGR888:
        if (alpha < 240)
        {
            gl_blend_24bit(ctx, &bred, &bgreen, &bblue, balpha, buf);
        }
        buf[0] = bblue;
        buf[1] = bgreen;
        buf[2] = bred;
        break;
    case GL_RGB565:
        if (alpha < 240)
        {
            gl_blend_16bit(ctx, &bred, &bgreen, &bblue, balpha, sbuf);
        }
        *sbuf = FORM_RGB565(bred, bgreen, bblue);
        break;
    case GL_RGB555:
        if (alpha < 240)
        {
            gl_blend_15bit(ctx, &bred, &bgreen, &bblue, balpha, sbuf);
        }
        *sbuf = FORM_RGB555(bred, bgreen, bblue);
        break;
    }
}

//rgb pixel
//called write_pixel to differentiate from draw_pixel,
//which would be hooked up to the context
static void write_pixel(
        GLcontext* ctx, GLint x, GLint y, GLdepth z,
        GLint red, GLint green, GLint blue, GLint alpha)
{
    GLubyte* buf = CTX_FB_ADDRESS1(ctx, x, y);
    GLushort* sbuf = (GLushort*)buf;
    GLuint* dbuf = (GLuint*)buf;
    GLubyte bred, bgreen, bblue, balpha;

    if (ctx->Speedy && rglGetSkip(y))
    {
        return;
    }

    CLAMP(red, 0, ctx->Buffer.maxr);
    CLAMP(green, 0, ctx->Buffer.maxg);
    CLAMP(blue, 0, ctx->Buffer.maxb);

    if (x < 0 || y < 0
        || x >= (GLint)ctx->Buffer.Width
        || y >= (GLint)ctx->Buffer.Height)
    {
        return;
    }

    if (ctx->ScissorTest)
    {
        if (y < ctx->ScissorY || y >= (ctx->ScissorY + ctx->ScissorHeight))
            return;
        if (x < ctx->ScissorX || x >= (ctx->ScissorX + ctx->ScissorWidth))
            return;
    }

    if (z != 0 && ctx->DepthTest)
    {
        GLdepth* zptr = CTX_Z_ADDRESS(ctx, x, y);
        z |= g_DepthMask;
        if (z < *zptr)
            *zptr = z;
        else
            return;
    }

    if (ctx->AlphaTest)
    {
        switch (ctx->AlphaFunc)
        {
        case GL_LESS:
            if (alpha >= ctx->AlphaByteRef)
            {
                return;
            }
            break;
        case GL_GREATER:
            if (alpha <= ctx->AlphaByteRef)
            {
                return;
            }
            break;
        default:
            return;
        }
    }
    if (g_doAlpha)
    {
        bred = (GLubyte)red;
        bgreen = (GLubyte)green;
        bblue = (GLubyte)blue;
        balpha = (GLubyte)alpha;

        switch (ctx->Buffer.PixelType)
        {
        case GL_RGB32:
            gl_blend_32bit(ctx, &bred, &bgreen, &bblue, balpha, dbuf);
            *dbuf = FORM_RGB32(bred, bgreen, bblue);
            break;
        case GL_BGR32:
            gl_blend_32bit(ctx, &bred, &bgreen, &bblue, balpha, dbuf);
            *dbuf = FORM_BGR32(bred, bgreen, bblue);
            break;
        case GL_BGR888:
            gl_blend_24bit(ctx, &bred, &bgreen, &bblue, balpha, buf);
            buf[0] = bblue;
            buf[1] = bgreen;
            buf[2] = bred;
            break;
        case GL_RGB888:
            gl_blend_24bit(ctx, &bred, &bgreen, &bblue, balpha, buf);
            buf[0] = bred;
            buf[1] = bgreen;
            buf[2] = bblue;
            break;
        case GL_RGB555:
            gl_blend_15bit(ctx, &bred, &bgreen, &bblue, balpha, sbuf);
            *sbuf = FORM_RGB555(bred, bgreen, bblue);
            break;
        case GL_RGB565:
            gl_blend_16bit(ctx, &bred, &bgreen, &bblue, balpha, sbuf);
            *sbuf = FORM_RGB565(bred, bgreen, bblue);
            break;
        default:
            gl_error(ctx, GL_INVALID_VALUE, "write_pixel(PixelType)");
        }
    }
    else
    {
        switch (ctx->Buffer.PixelType)
        {
        case GL_RGB32:
            *dbuf = FORM_RGB32(red, green, blue);
            break;
        case GL_BGR32:
            *dbuf = FORM_BGR32(red, green, blue);
            break;
        case GL_BGR888:
            buf[0] = (GLubyte)blue;
            buf[1] = (GLubyte)green;
            buf[2] = (GLubyte)red;
            break;
        case GL_RGB888:
            buf[0] = (GLubyte)red;
            buf[1] = (GLubyte)green;
            buf[2] = (GLubyte)blue;
            break;
        case GL_RGB555:
            *sbuf = FORM_RGB555(red, green, blue);
            break;
        case GL_RGB565:
            *sbuf = FORM_RGB565(red, green, blue);
            break;
        default:
            gl_error(ctx, GL_INVALID_VALUE, "write_pixel(PixelType)");
        }
    }
}

static void general_rgb_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pvert)
{
    vertex_buffer* VB = ctx->VB;
    GLint x0 = FAST_CEIL(VB->Win[vert0][0]);
    GLint y0 = FAST_CEIL(VB->Win[vert0][1]);
    GLint x1 = FAST_CEIL(VB->Win[vert1][0]);
    GLint y1 = FAST_CEIL(VB->Win[vert1][1]);
    GLint dx, dy;
    GLint xstep, ystep;
    GLint z0, z1, dz, zPtrXstep, zPtrYstep;
    GLdepth* zPtr;
    GLfixed r0 = IntToFixed(VB->Color[vert0][0]);
    GLfixed dr = IntToFixed(VB->Color[vert1][0]) - r0;
    GLfixed g0 = IntToFixed(VB->Color[vert0][1]);
    GLfixed dg = IntToFixed(VB->Color[vert1][1]) - g0;
    GLfixed b0 = IntToFixed(VB->Color[vert0][2]);
    GLfixed db = IntToFixed(VB->Color[vert1][2]) - b0;
    GLfixed a0 = IntToFixed(VB->Color[vert0][3]);
    GLfixed da = IntToFixed(VB->Color[vert1][3]) - a0;
    GLushort m;
    GLint Height = ctx->Buffer.Height;

    GLint width, min, max;  //linewidth
    width = (GLint)ctx->LineWidth;
    if (width < 1)
        width = 1;
    min = -width / 2;
    max = min + width - 1;  //

    //quickly clip the line if necessary
    {
        GLint w = ctx->Buffer.Width;
        GLint h = ctx->Buffer.Height;
        if ((x0 == w) | (x1 == w))
        {
            if ((x0 == w) & (x1 == w))
                return;
            x0 -= x0 == w;
            x1 -= x1 == w;
        }
        if ((y0 == h) | (y1 == h))
        {
            if ((y0 == h) & (y1 == h))
                return;
            y0 -= y0 == h;
            y1 -= y1 == h;
        }
    }

    dx = x1 - x0;
    dy = y1 - y0;
    if (dx == 0 && dy == 0)
        return;

    zPtr = CTX_Z_ADDRESS(ctx, x0, y0);
    z0 = (GLint)VB->Win[vert0][2];
    z1 = (GLint)VB->Win[vert1][2];

    if (dx < 0)
    {
        dx = -dx;
        xstep = -1;
        zPtrXstep = -((GLint)sizeof(GLdepth));
    }
    else
    {
        xstep = 1;
        zPtrXstep = sizeof(GLdepth);
    }

    if (dy < 0)
    {
        dy = -dy;
        ystep = -1;
        zPtrYstep = -((GLint)(ctx->Buffer.Width * sizeof(GLdepth)));
    }
    else
    {
        ystep = 1;
        zPtrYstep = ctx->Buffer.Width * sizeof(GLdepth);
    }

    if (dx > dy)    //x dominant
    {
        GLint i;
        GLint errorInc = dy + dy;
        GLint error = errorInc - dx;
        GLint errorDec = error - dx;

        dz = (z1 - z0) / dx;

        dr /= dx;       //per-pixel delta
        dg /= dx;
        db /= dx;
        da /= dx;

        for (i = 0; i < dx; i++)
        {
            GLdepth Z = z0;

            GLint yy;       //linewidth
            GLint ymin = y0 + min;
            GLint ymax = y0 + max;

            if (ctx->LineStipple)
            {
                m = 1 << ((ctx->StippleCounter / ctx->StippleFactor) & 0xf);
                if (!(ctx->StipplePattern & m))
                    goto SKIPPLOT0;
            }

            if (!ctx->DepthTest || (Z|g_DepthMask) < *zPtr)
            {
                if (ctx->DepthTest)
                    *zPtr = Z|g_DepthMask;
                for (yy = ymin; yy <= ymax; yy++)
                    write_pixel(ctx, x0, yy, 0,
                                FixedToInt(r0),
                                FixedToInt(g0),
                                FixedToInt(b0),
                                FixedToInt(a0));
            }

        SKIPPLOT0:
            ctx->StippleCounter++;

            x0 += xstep;

            zPtr = (GLdepth*)((GLubyte*)zPtr + zPtrXstep);
            z0 += dz;

            r0 += dr;
            g0 += dg;
            b0 += db;
            a0 += da;

            if (error < 0)
                error += errorInc;
            else
            {
                error += errorDec;
                y0 += ystep;
                zPtr = (GLdepth*)((GLubyte*)zPtr + zPtrYstep);
            }
        }
    }
    else            //y dominant
    {
        GLint i;
        GLint errorInc = dx + dx;
        GLint error = errorInc - dy;
        GLint errorDec = error - dy;

        dz = (z1 - z0) / dy;

        dr /= dy;
        dg /= dy;
        db /= dy;
        da /= dy;

        for (i = 0; i < dy; i++)
        {
            GLdepth Z = z0;

            GLint xx;       //linewidth
            GLint xmin = x0 + min;
            GLint xmax = x0 + max;

            if (ctx->LineStipple)
            {
                m = 1 << ((ctx->StippleCounter / ctx->StippleFactor) & 0xf);
                if (!(ctx->StipplePattern & m))
                    goto SKIPPLOT1;
            }

            if (y0 < Height && (!ctx->DepthTest || (Z|g_DepthMask) < *zPtr))
            {
                if (ctx->DepthTest)
                    *zPtr = Z|g_DepthMask;
                for (xx = xmin; xx <= xmax; xx++)
                    write_pixel(ctx, xx, y0, 0,
                                FixedToInt(r0),
                                FixedToInt(g0),
                                FixedToInt(b0),
                                FixedToInt(a0));
            }

        SKIPPLOT1:
            ctx->StippleCounter++;

            if (y0 > (Height-1))
            {
                y0 += ystep;
                zPtr = CTX_Z_ADDRESS(ctx, x0, y0);
                z0 += dz;
            }
            else
            {
                y0 += ystep;
                zPtr = (GLdepth*)((GLubyte*)zPtr + zPtrYstep);
                z0 += dz;
            }

            r0 += dr;
            g0 += dg;
            b0 += db;
            a0 += da;

            if (error < 0)
                error += errorInc;
            else
            {
                error += errorDec;
                x0 += xstep;
                zPtr = (GLdepth*)((GLubyte*)zPtr + zPtrXstep);
            }
        }
    }
}

static GLcontext*   context;
static GLubyte      cred, cgreen, cblue;
static GLint        g_width, g_min, g_max;

#define swap(a,b)       {a ^= b; b ^= a; a ^= b;}
#define absolute(i,j,k) ((i-j)*(k = ((i-j) < 0 ? -1 : 1)))

static void PLOT(GLint x, GLint y, GLint flag)
{
    if (flag)
    {
        //y major
        GLint xx, xmin, xmax;
        swap(x,y);

        xmin = x + g_min;
        xmax = x + g_max;
        if (g_width > 1)
        {
            for (xx = xmin; xx <= xmax; xx++)
                write_pixel(context, xx, y, 0, cred, cgreen, cblue, 255);
        }
        else
            write_pixel(context, x, y, 0, cred, cgreen, cblue, 255);
    }
    else
    {
        //x major
        GLint yy, ymin, ymax;

        ymin = y + g_min;
        ymax = y + g_max;
        if (g_width > 1)
        {
            for (yy = ymin; yy <= ymax; yy++)
                write_pixel(context, x, yy, 0, cred, cgreen, cblue, 255);
        }
        else
            write_pixel(context, x, y, 0, cred, cgreen, cblue, 255);
    }
}

static void doublestep_rgb_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pvert)
{
    vertex_buffer* VB = ctx->VB;
    GLint a1 = FAST_CEIL(VB->Win[vert0][0]);
    GLint b1 = FAST_CEIL(VB->Win[vert0][1]);
    GLint a2 = FAST_CEIL(VB->Win[vert1][0]);
    GLint b2 = FAST_CEIL(VB->Win[vert1][1]);
    GLint dx, dy, incr1, incr2, D, x, y, xend, c, pixels_left;
    GLint x1, y1;
    GLint sign_x, sign_y, step, reverse, i;

    g_width = FAST_TO_INT(ctx->LineWidth);
    if (g_width < 1)
        g_width = 1;
    g_min = -g_width / 2;
    g_max = g_min + g_width - 1;

    context = ctx;
    cred = VB->Color[pvert][0];
    cgreen = VB->Color[pvert][1];
    cblue = VB->Color[pvert][2];

    dx = absolute(a2, a1, sign_x);
    dy = absolute(b2, b1, sign_y);
    //decide increment sign by slope sign
    if (sign_x == sign_y)
        step = 1;
    else
        step = -1;

    if (dy > dx)
    {
        swap(a1, b1);
        swap(a2, b2);
        swap(dx, dy);
        reverse = 1;
    }
    else
        reverse = 0;

    //error check for dx == 0 should be here
    if (a1 > a2)
    {
        x = a2;
        y = b2;
        x1 = a1;
        y1 = b1;
    }
    else
    {
        x = a1;
        y = b1;
        x1 = a2;
        y1 = b2;
    }

    xend = (dx - 1) / 4;
    pixels_left = (dx - 1) & 3; //% 4

    PLOT(x, y, reverse);
    PLOT(x1, y1, reverse);
    incr2 = 4 * dy - 2 * dx;
    if (incr2 < 0)
    {
        c = 2 * dy;
        incr1 = 2 * c;
        D = incr1 - dx;

        for (i = 0; i < xend; i++)
        {
            ++x;
            --x1;
            if (D < 0)
            {
                //pattern 1 forwards
                PLOT(x, y, reverse);
                PLOT(++x, y, reverse);
                //pattern 1 backwards
                PLOT(x1, y1, reverse);
                PLOT(--x1, y1, reverse);
                D += incr1;
            }
            else
            {
                if (D < c)
                {
                    //pattern 2 forwards
                    PLOT(x, y, reverse);
                    PLOT(++x, y += step, reverse);
                    //pattern 2 backwards
                    PLOT(x1, y1, reverse);
                    PLOT(--x1, y1 -= step, reverse);
                }
                else
                {
                    //pattern 3 forwards
                    PLOT(x, y += step, reverse);
                    PLOT(++x, y, reverse);
                    //pattern 3 backwards
                    PLOT(x1, y1 -= step, reverse);
                    PLOT(--x1, y1, reverse);
                }
                D += incr2;
            }
        }//for

        if (pixels_left)
        {
            if (D < 0)
            {
                //pattern 1
                PLOT(++x, y, reverse);
                if (pixels_left > 1)
                    PLOT(++x, y, reverse);
                if (pixels_left > 2)
                    PLOT(--x1, y1, reverse);
            }
            else
            {
                if (D < c)
                {
                    //pattern 2
                    PLOT(++x, y, reverse);
                    if (pixels_left > 1)
                        PLOT(++x, y += step, reverse);
                    if (pixels_left > 2)
                        PLOT(--x1, y1, reverse);
                }
                else
                {
                    //pattern 3
                    PLOT(++x, y += step, reverse);
                    if (pixels_left > 1)
                        PLOT(++x, y, reverse);
                    if (pixels_left > 2)
                        PLOT(--x1, y1 -= step, reverse);
                }
            }
        }
    }//end slope < 1/2
    else
    {
        c = 2 * (dy - dx);
        incr1 = 2 * c;
        D = incr1 + dx;
        for (i = 0; i < xend; i++)
        {
            ++x;
            --x1;
            if (D > 0)
            {
                //pattern 4 forwards
                PLOT(x, y += step, reverse);
                PLOT(++x, y += step, reverse);
                //pattern 4 backwards
                PLOT(x1, y1 -= step, reverse);
                PLOT(--x1, y1 -= step, reverse);
                D += incr1;
            }
            else
            {
                if (D < c)
                {
                    //pattern 2 forwards
                    PLOT(x, y, reverse);
                    PLOT(++x, y += step, reverse);
                    //pattern 2 backwards
                    PLOT(x1, y1, reverse);
                    PLOT(--x1, y1 -= step, reverse);
                }
                else
                {
                    //pattern 3 forwards
                    PLOT(x, y += step, reverse);
                    PLOT(++x, y, reverse);
                    //pattern 3 backwards
                    PLOT(x1, y1 -= step, reverse);
                    PLOT(--x1, y1, reverse);
                }
                D += incr2;
            }
        }//for

        if (pixels_left)
        {
            if (D > 0)
            {
                //pattern 4
                PLOT(++x, y += step, reverse);
                if (pixels_left > 1)
                    PLOT(++x, y += step, reverse);
                if (pixels_left > 2)
                    PLOT(--x1, y1 -= step, reverse);
            }
            else
            {
                if (D < c)
                {
                    //pattern 2
                    PLOT(++x, y, reverse);
                    if (pixels_left > 1)
                        PLOT(++x, y += step, reverse);
                    if (pixels_left > 2)
                        PLOT(--x1, y1, reverse);
                }
                else
                {
                    //pattern 3
                    PLOT(++x, y += step, reverse);
                    if (pixels_left > 1)
                        PLOT(++x, y, reverse);
                    if (pixels_left > 2)
                    {
                        if (D > c)
                            PLOT(--x1, y1 -= step, reverse);
                        else
                            PLOT(--x1, y1, reverse);
                    }
                }
            }
        }
    }//end slope > 1/2
}
