/*=============================================================================
    Name    : rglu.c
    Purpose : glu* functions for Homeworld

    Created 7/6/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#include <math.h>
#include "glinc.h"
#include "Shader.h"
#include "FastMath.h"


#ifndef M_PI
#define M_PI 3.14159265358979324
#endif


#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  c[(col<<2)+row]
void mat4_multd(GLdouble* c, GLdouble const* a, GLdouble const* b)
{
    int i;

    for (i = 0; i < 4; i++)
    {
	    GLdouble ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
	    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
	    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
	    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
	    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
    }
}
#undef A
#undef B
#undef P

void mat4_inversed(GLdouble* d, GLdouble* s)
{
    GLfloat in[16], out[16];
    GLint i;

    for (i = 0; i < 16; i++)
    {
        in[i] = (GLfloat)s[i];
    }
    shInvertMatrix(out, in);
    for (i = 0; i < 16; i++)
    {
        d[i] = (GLdouble)out[i];
    }
}

void v4_projectd(GLdouble u[4], GLdouble const v[4], GLdouble const m[16])
{
    GLdouble v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
#define M(row,col)  m[(col<<2)+row]
    u[0] = v0 * M(0,0) + v1 * M(1,0) + v2 * M(2,0) + v3 * M(3,0);
    u[1] = v0 * M(0,1) + v1 * M(1,1) + v2 * M(2,1) + v3 * M(3,1);
    u[2] = v0 * M(0,2) + v1 * M(1,2) + v2 * M(2,2) + v3 * M(3,2);
    u[3] = v0 * M(0,3) + v1 * M(1,3) + v2 * M(2,3) + v3 * M(3,3);
#undef M
}


void rgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    GLdouble xmin, xmax, ymin, ymax;

    ymax = zNear * tan(fovy * M_PI / 360.0);
    ymin = -ymax;

    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

GLint rgluProject(GLdouble objx, GLdouble objy, GLdouble objz,
                  GLdouble const* model, GLdouble const* proj,
                  GLint const* viewport,
                  GLdouble* winx, GLdouble* winy, GLdouble* winz)
{
    GLdouble in[4], out[4];

    in[0] = objx;
    in[1] = objy;
    in[2] = objz;
    in[3] = 1.0;
    v4_projectd(out, model, in);
    v4_projectd(in, proj, out);

    if (in[3] == 0.0)
        return GL_FALSE;

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];

    *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
    *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;
    *winz = (1 + in[2]) / 2;
    return GL_TRUE;
}

static void transform_point(
                GLdouble out[4],
                GLdouble const m[16],
			    GLdouble const in[4])
{
#define M(row,col)  m[col*4+row]
   out[0] = M(0,0) * in[0] + M(0,1) * in[1] + M(0,2) * in[2] + M(0,3) * in[3];
   out[1] = M(1,0) * in[0] + M(1,1) * in[1] + M(1,2) * in[2] + M(1,3) * in[3];
   out[2] = M(2,0) * in[0] + M(2,1) * in[1] + M(2,2) * in[2] + M(2,3) * in[3];
   out[3] = M(3,0) * in[0] + M(3,1) * in[1] + M(3,2) * in[2] + M(3,3) * in[3];
#undef M
}

GLint rgluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
                    GLdouble const* model, GLdouble const* proj,
                    GLint const* viewport,
                    GLdouble* objx, GLdouble* objy, GLdouble* objz)
{
    GLdouble m[16], A[16];
    GLdouble in[4], out[4];

    in[0] = (winx - viewport[0]) * 2 / viewport[2] - 1.0;
    in[1] = (winy - viewport[1]) * 2 / viewport[3] - 1.0;
    in[2] = 2 * winz - 1.0;
    in[3] = 1.0;

    mat4_multd(A, proj, model);
    mat4_inversed(m, A);

    transform_point(out, m, in);
    if (out[3] == 0.0)
        return GL_FALSE;
    *objx = out[0] / out[3];
    *objy = out[1] / out[3];
    *objz = out[2] / out[3];
    return GL_TRUE;
}

void rgluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
                GLdouble centerx, GLdouble centery, GLdouble centerz,
                GLdouble upx, GLdouble upy, GLdouble upz)
{
    GLdouble m[16];
    GLdouble x[3], y[3], z[3];
    GLdouble mag;

    z[0] = eyex - centerx;
    z[1] = eyey - centery;
    z[2] = eyez - centerz;
    mag = fsqrt(z[0]*z[0] + z[1]*z[1] + z[2]*z[2]);
    if (mag)
    {
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }

    y[0] = upx;
    y[1] = upy;
    y[2] = upz;

    x[0] =  y[1]*z[2] - y[2]*z[1];
    x[1] = -y[0]*z[2] + y[2]*z[0];
    x[2] =  y[0]*z[1] - y[1]*z[0];

    y[0] =  z[1]*x[2] - z[2]*x[1];
    y[1] = -z[0]*x[2] + z[2]*x[0];
    y[2] =  z[0]*x[1] - z[1]*x[0];

    mag = fsqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
    if (mag)
    {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = fsqrt(y[0]*y[0] + y[1]*y[1] + y[2]*y[2]);
    if (mag)
    {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }

#define M(row,col) m[(col<<2)+row]
    M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
    M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
    M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
    M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;  M(3,3) = 1.0;
#undef M

    glMultMatrixd(m);
    glTranslated(-eyex, -eyey, -eyez);
}

void rgluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
    glOrtho(left, right, bottom, top, -1.0, 1.0);
}

char* rgluErrorString(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:
        return "no error";
    case GL_INVALID_ENUM:
        return "invalid enumeration";
    case GL_INVALID_VALUE:
        return "invalid value";
    case GL_INVALID_OPERATION:
        return "invalid operation";
    case GL_STACK_OVERFLOW:
        return "stack overflow";
    case GL_STACK_UNDERFLOW:
        return "stack underflow";
    case GL_OUT_OF_MEMORY:
        return "out of memory";
    default:
        return "unknown error";
    }
}
