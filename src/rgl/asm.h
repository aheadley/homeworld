/*=============================================================================
        Name    : asm.h
        Purpose : assembly language versions of some rGL functions

Created 7/13/1998 by khent
Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _ASM_H
#define _ASM_H

#include "kgl.h"

GLuint get_cputype();
GLboolean get_cpummx();
GLboolean get_cpukatmai();

void xmm_update_modelview(GLcontext* ctx);
void xmm_update_projection(GLcontext* ctx);

void asm_cliptest(
    GLuint n, GLfloat* d, GLubyte* clipmask,
    GLubyte* ormask, GLubyte* andmask);
void asm_project_and_cliptest_general(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask);
void asm_project_and_cliptest_identity(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask);
void asm_project_and_cliptest_perspective(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask);

void intrin_project_and_cliptest_perspective(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask);
void xmm_project_and_cliptest_perspective(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask);

void gl_megafast_affine_transform(
    GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count);
void gl_intrin_3dtransform(
    GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count);
void gl_xmm_3dtransform(
    GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count);

void gl_wicked_fast_normal_xform(
    GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint n);
void gl_fairly_fast_scaled_normal_xform(
    GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint n, GLfloat scale);

void gl_mmx_blend_span(
    GLcontext* ctx, GLuint n, GLubyte* mask, GLubyte rgba[][4], GLushort* dest);

#endif
