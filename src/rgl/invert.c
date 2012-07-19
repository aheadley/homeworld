#include <string.h>
#include "kgl.h"

typedef GLfloat Mat2[2][2];

GLfloat IdentityMatrix[16] =
{
1.0f, 0.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f, 0.0f,
0.0f, 0.0f, 0.0f, 1.0f
};

enum
{
    M00 = 0, M01 = 4, M02 = 8, M03 = 12,
    M10 = 1, M11 = 5, M12 = 9, M13 = 13,
    M20 = 2, M21 = 6, M22 = 10,M23 = 14,
    M30 = 3, M31 = 7, M32 = 11,M33 = 15
};

static void invert_matrix_general(GLfloat const* m, GLfloat* out)
{
    Mat2 r1, r2, r3, r4, r5, r6, r7;
    GLfloat const* A = m;
    GLfloat* C = out;
    GLfloat one_over_det;

    /*
     * A is the 4x4 source matrix (to be inverted).
     * C is the 4x4 destination matrix
     * a11 is the 2x2 matrix in the upper left quadrant of A
     * a12 is the 2x2 matrix in the upper right quadrant of A
     * a21 is the 2x2 matrix in the lower left quadrant of A
     * a22 is the 2x2 matrix in the lower right quadrant of A
     * similarly, cXX are the 2x2 quadrants of the destination matrix
     */

    /* R1 = inverse( a11 ) */
    one_over_det = 1.0f / ((A[M00] * A[M11]) - (A[M10] * A[M01]));
    r1[0][0] = one_over_det * A[M11];
    r1[0][1] = one_over_det * -A[M01];
    r1[1][0] = one_over_det * -A[M10];
    r1[1][1] = one_over_det * A[M00];

    /* R2 = a21 x R1 */
    r2[0][0] = A[M20] * r1[0][0] + A[M21] * r1[1][0];
    r2[0][1] = A[M20] * r1[0][1] + A[M21] * r1[1][1];
    r2[1][0] = A[M30] * r1[0][0] + A[M31] * r1[1][0];
    r2[1][1] = A[M30] * r1[0][1] + A[M31] * r1[1][1];

    /* R3 = R1 x a12 */
    r3[0][0] = r1[0][0] * A[M02] + r1[0][1] * A[M12];
    r3[0][1] = r1[0][0] * A[M03] + r1[0][1] * A[M13];
    r3[1][0] = r1[1][0] * A[M02] + r1[1][1] * A[M12];
    r3[1][1] = r1[1][0] * A[M03] + r1[1][1] * A[M13];

    /* R4 = a21 x R3 */
    r4[0][0] = A[M20] * r3[0][0] + A[M21] * r3[1][0];
    r4[0][1] = A[M20] * r3[0][1] + A[M21] * r3[1][1];
    r4[1][0] = A[M30] * r3[0][0] + A[M31] * r3[1][0];
    r4[1][1] = A[M30] * r3[0][1] + A[M31] * r3[1][1];

    /* R5 = R4 - a22 */
    r5[0][0] = r4[0][0] - A[M22];
    r5[0][1] = r4[0][1] - A[M23];
    r5[1][0] = r4[1][0] - A[M32];
    r5[1][1] = r4[1][1] - A[M33];

    /* R6 = inverse( R5 ) */
    one_over_det = 1.0f / ((r5[0][0] * r5[1][1]) - (r5[1][0] * r5[0][1]));
    r6[0][0] = one_over_det * r5[1][1];
    r6[0][1] = one_over_det * -r5[0][1];
    r6[1][0] = one_over_det * -r5[1][0];
    r6[1][1] = one_over_det * r5[0][0];

    /* c12 = R3 x R6 */
    C[M02] = r3[0][0] * r6[0][0] + r3[0][1] * r6[1][0];
    C[M03] = r3[0][0] * r6[0][1] + r3[0][1] * r6[1][1];
    C[M12] = r3[1][0] * r6[0][0] + r3[1][1] * r6[1][0];
    C[M13] = r3[1][0] * r6[0][1] + r3[1][1] * r6[1][1];

    /* c21 = R6 x R2 */
    C[M20] = r6[0][0] * r2[0][0] + r6[0][1] * r2[1][0];
    C[M21] = r6[0][0] * r2[0][1] + r6[0][1] * r2[1][1];
    C[M30] = r6[1][0] * r2[0][0] + r6[1][1] * r2[1][0];
    C[M31] = r6[1][0] * r2[0][1] + r6[1][1] * r2[1][1];

    /* R7 = R3 x c21 */
    r7[0][0] = r3[0][0] * C[M20] + r3[0][1] * C[M30];
    r7[0][1] = r3[0][0] * C[M21] + r3[0][1] * C[M31];
    r7[1][0] = r3[1][0] * C[M20] + r3[1][1] * C[M30];
    r7[1][1] = r3[1][0] * C[M21] + r3[1][1] * C[M31];

    /* c11 = R1 - R7 */
    C[M00] = r1[0][0] - r7[0][0];
    C[M01] = r1[0][1] - r7[0][1];
    C[M10] = r1[1][0] - r7[1][0];
    C[M11] = r1[1][1] - r7[1][1];

    /* c22 = -R6 */
    C[M22] = -r6[0][0];
    C[M23] = -r6[0][1];
    C[M32] = -r6[1][0];
    C[M33] = -r6[1][1];
}

void invert_matrix(GLfloat const* m, GLfloat* out)
{
#define MAT(m,r,c) (m)[((c)<<2)+(r)]

#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

    GLfloat det;
    GLfloat tmp[16];    /* allow out == in */

    if (m41 != 0.0f || m42 != 0.0f || m43 != 0.0f || m44 != 1.0f)
    {
        invert_matrix_general(m, out);
        return;
    }

    /* Inverse = adjoint / det */

    tmp[0] = m22 * m33 - m23 * m32;
    tmp[1] = m23 * m31 - m21 * m33;
    tmp[2] = m21 * m32 - m22 * m31;

    /* compute determinant using cofactors */
    det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2];

    /* singularity test */
    if (det == 0.0f)
    {
        MEMCPY(out, IdentityMatrix, 16*sizeof(GLfloat));
    }
    else
    {
        GLfloat d12, d13, d23, d24, d34, d41;
        GLfloat im11, im12, im13, im14;

        det= 1.0f / det;

        /* compute rest of inverse */
        tmp[0] *= det;
        tmp[1] *= det;
        tmp[2] *= det;
        tmp[3]  = 0.0f;

        im11 = m11 * det;
        im12 = m12 * det;
        im13 = m13 * det;
        im14 = m14 * det;
        tmp[4] = im13 * m32 - im12 * m33;
        tmp[5] = im11 * m33 - im13 * m31;
        tmp[6] = im12 * m31 - im11 * m32;
        tmp[7] = 0.0f;

        /* Pre-compute 2x2 dets for first two rows when computing */
        /* cofactors of last two rows. */
        d12 = im11*m22 - m21*im12;
        d13 = im11*m23 - m21*im13;
        d23 = im12*m23 - m22*im13;
        d24 = im12*m24 - m22*im14;
        d34 = im13*m24 - m23*im14;
        d41 = im14*m21 - m24*im11;

        tmp[8] =  d23;
        tmp[9] = -d13;
        tmp[10] = d12;
        tmp[11] = 0.0f;

        tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23);
        tmp[13] =  (m31 * d34 + m33 * d41 + m34 * d13);
        tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12);
        tmp[15] =  1.0f;

        MEMCPY(out, tmp, 16*sizeof(GLfloat));
    }
#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}
