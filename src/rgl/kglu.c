#include "kgl.h"
#include "maths.h"

static GLboolean math_init = GL_FALSE;

DLL void gluPerspective(GLdouble fovy, GLdouble aspect,
		            GLdouble zNear, GLdouble zFar)
{
    GLdouble xmin, xmax, ymin, ymax;

    ymax = zNear * tan(fovy * M_PI / 360.0);
    ymin = -ymax;

    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

DLL GLint gluProject(GLdouble objx, GLdouble objy, GLdouble objz,
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

DLL GLint gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
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

DLL void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
               GLdouble centerx, GLdouble centery, GLdouble centerz,
               GLdouble upx, GLdouble upy, GLdouble upz)
{
    GLdouble m[16];
    GLdouble x[3], y[3], z[3];
    GLdouble mag;

    if (!math_init)
    {
        init_sqrt_tab();
        math_init = GL_TRUE;
    }

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

DLL void gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
    glOrtho(left, right, bottom, top, -1.0, 1.0);
}

DLL char* gluErrorString(GLenum err)
{
    return glGluErrorString(err);
}
