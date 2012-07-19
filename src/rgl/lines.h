/*=============================================================================
        Name    : lines.h
        Purpose : line-rendering prototypes

        Created 15/06/1998 by khent
        Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _LINES_H
#define _LINES_H

#include "kgl.h"

void flat_565_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void flat_565_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void flat_565_blend_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void flat_565_blend_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void smooth_565_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void smooth_565_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void smooth_565_blend_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void smooth_565_blend_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);

void flat_555_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void flat_555_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void flat_555_blend_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void flat_555_blend_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void smooth_555_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void smooth_555_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void smooth_555_blend_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);
void smooth_555_blend_z_line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pv);

#endif //_LINES_H
