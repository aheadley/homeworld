#ifndef _iCLIP_H
#define _iCLIP_H

#include "kgl.h"

GLuint gl_viewclip_point(GLfloat const v[]);
GLuint gl_viewclip_line(GLcontext* ctx, GLuint* i, GLuint* j);
GLuint gl_viewclip_polygon(GLcontext* ctx, GLuint n, GLuint vlist[]);
GLuint gl_userclip_polygon(GLcontext* ctx, GLuint n, GLuint vlist[]);

#endif
