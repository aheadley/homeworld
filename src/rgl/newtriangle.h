/*=============================================================================
    Name    : newtriangle.h
    Purpose : header file for the non-textured triangle rasterizers

    Created 5/1/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _NEWTRIANGLE_H
#define _NEWTRIANGLE_H

void choose_newtriangle_rasterizers(GLcontext* ctx);
void general_new_textured_triangle(GLcontext* ctx, GLuint vl[], GLuint pv);
void general_perspective_new_textured_triangle(GLcontext* ctx, GLuint vl[], GLuint pv);
void new_textured_triangle(GLcontext* ctx, GLuint vl[], GLuint pv);
void new_perspective_textured_triangle(GLcontext* ctx, GLuint vl[], GLuint pv);
void new_triangle(GLcontext* ctx, GLuint vl[], GLuint pv);
void ditherStartup(GLcontext* ctx);
void ditherShutdown(void);
void stippleStartup(GLcontext* ctx);
void stippleShutdown(void);

#endif
