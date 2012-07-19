#include <math.h>
#include <stdio.h>
#include <string.h>
#include "kgl.h"
#include "maths.h"

typedef enum {X,Y,Z,W} quat_coefficient;

GLfloat xaxis[3] = {1.0f,0.0f,0.0f};
GLfloat yaxis[3] = {0.0f,1.0f,0.0f};
GLfloat zaxis[3] = {0.0f,0.0f,1.0f};

static GLfloat mat4_identity_matrix[16];


void v3_output(GLfloat* v)
{
    printf("|%f %f %f|", v[0], v[1], v[2]);
}

void mat4_output(GLfloat* m)
{
    int i;
    for (i = 0; i < 16; i += 4) {
        printf("|%f %f %f %f|",
               m[i+0], m[i+1], m[i+2], m[i+3]);
	    if (i < 12)
            puts("");
    }
}


/* MOST_SIG_OFFSET gives the (int *) offset from the address of the double
 * to the part of the number containing the sign and exponent.
 * You will need to find the relevant offset for your architecture.
 */

#define MOST_SIG_OFFSET 1

/* SQRT_TAB_SIZE - the size of the lookup table - must be a power of four.
 */

#define SQRT_TAB_SIZE 16384

/* MANT_SHIFTS is the number of shifts to move mantissa into position.
 * If you quadruple the table size subtract two from this constant,
 * if you quarter the table size then add two.
 * Valid values are: (16384, 7) (4096, 9) (1024, 11) (256, 13)
 */

#define MANT_SHIFTS   7

#define EXP_BIAS   1023       /* Exponents are always positive     */
#define EXP_SHIFTS 20         /* Shifs exponent to least sig. bits */
#define EXP_LSB    0x00100000 /* 1 << EXP_SHIFTS                   */
#define MANT_MASK  0x000FFFFF /* Mask to extract mantissa          */

static int sqrt_tab[SQRT_TAB_SIZE];

void init_sqrt_tab()
{
    int           i, j;
    double        f;
    unsigned int  *fi = (unsigned int *) &f + MOST_SIG_OFFSET;

    for (i = 0; i < 4; i++)
	    for (j = 0; j < 4; j++)
        {
	        f = (i == j) ? 1.0 : 0.0;
	        mat4_identity_matrix[4*i + j] = (GLfloat)f;
	    }

    for (i = 0; i < SQRT_TAB_SIZE/2; i++)
    {
	    f = 0; /* Clears least sig part */
	    *fi = (i << MANT_SHIFTS) | (EXP_BIAS << EXP_SHIFTS);
	    f = sqrt(f);
	    sqrt_tab[i] = *fi & MANT_MASK;

	    f = 0; /* Clears least sig part */
	    *fi = (i << MANT_SHIFTS) | ((EXP_BIAS + 1) << EXP_SHIFTS);
	    f = sqrt(f);
	    sqrt_tab[i + SQRT_TAB_SIZE/2] = *fi & MANT_MASK;
    }
}

double fsqrt(double f)
{
    unsigned int e;
    unsigned int *fi = (unsigned int *) &f + MOST_SIG_OFFSET;

    if (f == 0.0)
	    return 0.0;
    e = (*fi >> EXP_SHIFTS) - EXP_BIAS;
    *fi &= MANT_MASK;
    if (e & 1)
	    *fi |= EXP_LSB;
    e >>= 1;
    *fi = (sqrt_tab[*fi >> MANT_SHIFTS]) |
	((e + EXP_BIAS) << EXP_SHIFTS);
    return f;
}


/* ----- v3 */

GLfloat v3_mag2(GLfloat* v)
{
    return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

GLfloat v3_magnitude(GLfloat* v)
{
    GLfloat mag = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    return (GLfloat)fsqrt(mag);
}

void v3_set(GLfloat* v, GLfloat x, GLfloat y, GLfloat z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

void v3_copy(GLfloat* d, GLfloat* s)
{
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
}

GLfloat v3_dot(GLfloat* a, GLfloat* b)
{
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void v3_normalize(GLfloat* v)
{
    GLfloat len = v3_magnitude(v);
    int i;
    if (len > 0.0f)
	    len = 1.0f / len;
    for (i = 0; i < 3; i++)
	    v[i] *= len;
}

#define ROWCOL(a,v,i) (a[i+0]*v[0] + a[i+1]*v[1] + a[i+2]*v[2] + a[i+3])
void v3_project(GLfloat* pout, GLfloat* m, GLfloat* pin)
{
#if 0
    pout[0] = ROWCOL(m,pin,0);
    pout[1] = ROWCOL(m,pin,4);
    pout[2] = ROWCOL(m,pin,8);
#else
    fprintf(stderr, "v3_project is broken\n");
#endif
}

void v4_project(GLfloat u[4], GLfloat const v[4], GLfloat const m[16])
{
    GLfloat v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
#define M(row,col)  m[(col<<2)+row]
    u[0] = v0 * M(0,0) + v1 * M(1,0) + v2 * M(2,0) + v3 * M(3,0);
    u[1] = v0 * M(0,1) + v1 * M(1,1) + v2 * M(2,1) + v3 * M(3,1);
    u[2] = v0 * M(0,2) + v1 * M(1,2) + v2 * M(2,2) + v3 * M(3,2);
    u[3] = v0 * M(0,3) + v1 * M(1,3) + v2 * M(2,3) + v3 * M(3,3);
#undef M
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

void v3_scale(GLfloat* c, GLfloat* a, GLfloat s)
{
    c[0] = a[0] * s;
    c[1] = a[1] * s;
    c[2] = a[2] * s;
}

void v3_add(GLfloat* c, GLfloat* a, GLfloat* b)
{
    int i;
    for (i = 0; i < 3; i++)
	    c[i] = a[i] + b[i];
}

void v3_subtract(GLfloat* c, GLfloat* a, GLfloat* b)
{
    int i;
    for (i = 0; i < 3; i++)
	    c[i] = a[i] - b[i];
}

void v3_negate(GLfloat* v)
{
    int i;
    for (i = 0; i < 3; i++)
	    v[i] = -v[i];
}

void v3_cross(GLfloat* c, GLfloat* a, GLfloat* b)
{
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}

void v3_vecproject(GLfloat* c, GLfloat* a, GLfloat* b)
{
    GLfloat mag;
    if (v3_mag2(b) == 0.0)
    {
	    v3_copy(c, a);
	    return;
    }
    if (v3_mag2(a) == 0.0)
    {
	    v3_copy(c, b);
	    return;
    }
    mag = v3_magnitude(b);
    mag = v3_dot(a, b) / (mag*mag);
    v3_scale(c, b, mag);
}

GLboolean v3_equal(GLfloat* a, GLfloat* b, GLfloat epsilon)
{
    int i;
    for (i = 0; i < 3; i++)
	    if (fabs(a[i] - b[i]) > epsilon)
	        return GL_FALSE;
    return GL_TRUE;
}


/* ----- mat4 */

void mat4_get_translation(GLfloat* t, GLfloat* m)
{
    t[0] = m[12];
    t[1] = m[13];
    t[2] = m[14];
}

void mat4_set_translationv(GLfloat* m, GLfloat* t)
{
    m[12] = t[0];
    m[13] = t[1];
    m[14] = t[2];
}

void mat4_set_translation(GLfloat* m, GLfloat x, GLfloat y, GLfloat z)
{
    m[12] = x;
    m[13] = y;
    m[14] = z;
}

void mat4_rotation(GLfloat* m, GLfloat* axis, GLfloat degrees)
{
    GLfloat mag, s, c;
    GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
    GLdouble rads;
    GLfloat x = axis[0];
    GLfloat y = axis[1];
    GLfloat z = axis[2];

    rads = (GLdouble)degrees * M_PI / 180.0;
    s = (GLfloat)sin(rads);
    c = (GLfloat)cos(rads);
    mag = (GLfloat)fsqrt((GLdouble)(x*x + y*y + z*z));
    if (mag == 0.0)
    {
	    mat4_identity(m);
	    return;
    }

    x /= mag;
    y /= mag;
    z /= mag;

#define M(row,col) m[(col<<2)+row]

    xx = x*x;
    yy = y*y;
    zz = z*z;
    xy = x*y;
    yz = y*z;
    zx = z*x;
    xs = x*s;
    ys = y*s;
    zs = z*s;
    one_c = 1.0f - c;

    M(0,0) = (one_c * xx) + c;
    M(0,1) = (one_c * xy) - zs;
    M(0,2) = (one_c * zx) + ys;
    M(0,3) = 0.0f;

    M(1,0) = (one_c * xy) + zs;
    M(1,1) = (one_c * yy) + c;
    M(1,2) = (one_c * yz) - xs;
    M(1,3) = 0.0f;

    M(2,0) = (one_c * zx) - ys;
    M(2,1) = (one_c * yz) + xs;
    M(2,2) = (one_c * zz) + c;
    M(2,3) = 0.0f;

    M(3,0) = 0.0f;
    M(3,1) = 0.0f;
    M(3,2) = 0.0f;
    M(3,3) = 1.0f;

#undef M
}

void mat4_mult(GLfloat* c, GLfloat const* a, GLfloat const* b)
{
    /* This matmul was contributed by Thomas Malik */
    int i;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  c[(col<<2)+row]

    /* i-te Zeile */
    for (i = 0; i < 4; i++)
    {
	    GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
	    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
	    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
	    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
	    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
    }
}

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

#undef A
#undef B
#undef P
}

void mat4_identity(GLfloat* m)
{
    memcpy(m, mat4_identity_matrix, 16*sizeof(GLfloat));
}

void mat4_copy(GLfloat* d, GLfloat* s)
{
    memcpy(d, s, 16*sizeof(GLfloat));
}

void mat4_transpose(GLfloat* d, GLfloat* s)
{
    GLfloat  t[16];
    GLfloat* a = s;
    int i, j;
    if (d == s)
    {
	    a = t;
	    mat4_copy(a, d);
    }
    for (i = 0; i < 4; i++)
	    for (j = 0; j < 4; j++)
	        d[j*4 + i] = a[i*4 + j];
}

void mat4_inverse(GLfloat* d, GLfloat* s)
{
    extern void invert_matrix(GLfloat const*, GLfloat*);
    invert_matrix(s, d);
}

void mat4_inversed(GLdouble* d, GLdouble* s)
{
    GLfloat in[16], out[16];
    GLint i;
    for (i = 0; i < 16; i++)
        in[i] = (GLfloat)s[i];
    invert_matrix(in, out);
    for (i = 0; i < 16; i++)
        d[i] = (GLdouble)out[i];
}