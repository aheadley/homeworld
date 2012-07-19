#ifndef _SCAN_H
#define _SCAN_H

//internals
typedef struct
{
    GLfloat aZ[3];
    GLfloat aR[3], aG[3], aB[3], aA[3];
    GLfloat aOneOverW[3];
    GLfloat aUOverW[3], aVOverW[3];

    GLfloat dZdX, dZdY;
    GLfixed fdZdX;

    GLfloat dOneOverWdX, dOneOverWdY;
    GLfloat dUOverWdX, dUOverWdY;
    GLfloat dVOverWdX, dVOverWdY;

    GLfloat dRdX, dRdY;
    GLfloat dGdX, dGdY;
    GLfloat dBdX, dBdY;
    GLfloat dAdX, dAdY;
    GLfixed fdRdX, fdGdX, fdBdX, fdAdX;
} gradients;

typedef struct
{
    GLfixed X, XStep;
    GLint   iX, iXStep;
    GLint   ErrorTerm, Numerator, Denominator;

    GLint   Y, Height;

    GLfixed Z, ZStep;

    GLfloat OneOverW, OneOverWStep;
    GLfloat UOverW, UOverWStep;
    GLfloat VOverW, VOverWStep;

    GLfixed R, RStep;
    GLfixed G, GStep;
    GLfixed B, BStep;
    GLfixed A, AStep;
} edge;

void tri4colorlerp(GLint n, GLubyte cspan[], edge* pLeft, edge* pRight, gradients* g);
void tri3colorlerp(GLint n, GLubyte cspan[], edge* pLeft, edge* pRight, gradients* g);

/* triangle funcs */

void _draw_triangle_tex_bg(GLcontext*, GLuint[], GLuint);
void _draw_triangle_tex_rgb_z(GLcontext*, GLuint[], GLuint);
void _draw_triangle_tex_rgb(GLcontext*, GLuint[], GLuint);
void _draw_triangle_tex_z(GLcontext*, GLuint[], GLuint);
void _draw_triangle_tex(GLcontext*, GLuint[], GLuint);
void _draw_triangle_rgb_z(GLcontext*, GLuint[], GLuint);
void _draw_triangle_rgb(GLcontext*, GLuint[], GLuint);
void _draw_triangle_z(GLcontext*, GLuint[], GLuint);
void _draw_triangle(GLcontext*, GLuint[], GLuint);

void perspective_draw_triangle(GLcontext* ctx, GLuint vlist[], GLuint pv, void (*triangle_func)());

/* spanners */

extern void (*gl_suba_texture_span_bg)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight, gradients* g);
extern void (*gl_suba_texture_span_rgb_z)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight, gradients* g);
extern void (*gl_suba_texture_span_rgb_z_p)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight, gradients* g);
extern void (*gl_suba_texture_span_rgb)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight, gradients* g);
extern void (*gl_suba_texture_span_rgb_p)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight, gradients* g);
extern void (*gl_suba_texture_span_z)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight,
        gradients* g, GLubyte flat[]);
extern void (*gl_suba_texture_span_z_p)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight,
        gradients* g, GLubyte flat[]);
extern void (*gl_suba_texture_span)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight,
        gradients* g, GLubyte flat[]);
extern void (*gl_suba_texture_span_p)(
        GLcontext* ctx, GLuint Width, GLint x,
        GLdepth z[], GLubyte c[],
        edge* pLeft, edge* pRight,
        gradients* g, GLubyte flat[]);

#endif
