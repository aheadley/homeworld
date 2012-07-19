/*=============================================================================
    Name    : d3dblt.h
    Purpose : Direct3D texture blitters

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _D3DBLT_H
#define _D3DBLT_H

GLboolean d3d_blt_RGBA_generic(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA16_generic(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA_0565(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA_0555(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA_8888(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA_4444(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA16_4444(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA16_8888(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_COLORINDEX(LPDIRECTDRAWSURFACE4 surf, GLubyte* data, GLsizei width, GLsizei height);

void d3d_draw_pixels_RGBA_generic(GLint xOfs, GLint yOfs, GLsizei width, GLsizei height, GLubyte* data);

void d3d_draw_pixels_RGBA_pitched(GLint x0, GLint y0, GLint x1, GLint y1,
                                  GLsizei swidth, GLsizei sheight, GLsizei spitch,
                                  GLubyte* data);

void d3d_shot_setup(DDPIXELFORMAT* ddpf, GLint dColorBits[], GLint dColorShift[]);
void d3d_shot_16bit(GLubyte* pDest, GLubyte* pSource,
                    GLsizei width, GLsizei height,
                    GLint pitch, DDPIXELFORMAT* ddpf);
void d3d_shot_24bit(GLubyte* pDest, GLubyte* pSource,
                    GLsizei width, GLsizei height,
                    GLint pitch, DDPIXELFORMAT* ddpf);
void d3d_shot_32bit(GLubyte* pDest, GLubyte* pSource,
                    GLsizei width, GLsizei height,
                    GLint pitch, DDPIXELFORMAT* ddpf);

#endif
