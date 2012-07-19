#ifndef _SPAN_H
#define _SPAN_H

extern void (*write_monopixel_span)(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte r, GLubyte g, GLubyte b, GLubyte mask[]);
extern void (*write_4pixel_span)(
        GLcontext* ctx, GLint n, GLint x, GLint y,
        GLubyte c[], GLubyte mask[]);

//externals

void init_color_spanners(GLcontext* ctx);
void init_spanners(GLcontext* ctx);
void init_new_tex(GLcontext* ctx);

void gl_write_monocolor_span(GLcontext* ctx, GLuint n, GLint x, GLint y,
                             GLdepth z[], GLubyte r, GLubyte g, GLubyte b, GLubyte a,
                             GLenum primitive);

void gl_write_4color_span(GLcontext* ctx, GLuint n, GLint x, GLint y, GLdepth z[],
                          GLubyte c[], GLenum primitive);

void gl_write_texture_span(GLcontext* ctx, GLuint n, GLint x, GLint y, GLdepth z[],
                           GLfixed uv[], GLubyte c[]);

void depthlerp(GLint n, GLdepth zspan[], GLfixed zl, GLfixed dzdx);

void gl_read_color_span(GLcontext* ctx, GLuint n, GLint x, GLint y, GLubyte c[]);
void gl_blend_32bit(GLcontext* ctx, GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLuint* cdest);
void gl_blend_24bit(GLcontext* ctx, GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLubyte* cdest);
void gl_blend_16bit(GLcontext* ctx, GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLushort* cdest);
void gl_blend_15bit(GLcontext* ctx, GLubyte* sr, GLubyte* sg, GLubyte* sb, GLubyte sa, GLushort* cdest);

extern GLuint (*gl_depthtest_span_less)(GLcontext*, GLuint, GLint, GLint, GLdepth const[], GLubyte[]);
extern GLuint (*gl_depthtest_alphatest_span_less)(GLcontext*, GLuint, GLint, GLint, GLdepth const[], GLubyte[]);

GLuint gl_depthtest_span_less_write(
    GLcontext* ctx, GLuint n, GLint x, GLint y,
    GLdepth const z[], GLubyte mask[]);
GLuint gl_depthtest_alphatest_span_less_write(
    GLcontext* ctx, GLuint n, GLint x, GLint y,
    GLdepth const z[], GLubyte mask[]);
GLuint gl_depthtest_span_less_nowrite(
    GLcontext* ctx, GLuint n, GLint x, GLint y,
    GLdepth const z[], GLubyte mask[]);
GLuint gl_depthtest_alphatest_span_less_nowrite(
    GLcontext* ctx, GLuint n, GLint x, GLint y,
    GLdepth const z[], GLubyte mask[]);

#endif
