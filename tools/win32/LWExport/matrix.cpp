/*=============================================================================
    MATRIX.C: Homogenous Matrix functions (4X4)

    Created June 1997 by Gary Shaw
=============================================================================*/

#include <stdio.h>
#include "types.h"
#include "matrix.h"



/*=============================================================================
    Private Macros:
=============================================================================*/

#define hmatrixdot(x1,x2,x3,x4,y1,y2,y3,y4) \
    ( ((x1)*(y1)) + ((x2)*(y2)) + ((x3)*(y3)) + ((x4)*(y4)) )

#define matrixdot(x1,x2,x3,y1,y2,y3) \
    ( ((x1)*(y1)) + ((x2)*(y2)) + ((x3)*(y3)) )



/*=============================================================================
    Public constants:
=============================================================================*/

const hmatrix IdentityHMatrix =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

const matrix IdentityMatrix =
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};



/*=============================================================================
    Homogenous Matrix Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : hmatMakeHMatFromMat
    Description : makes a homogenous matrix from a matrix
    Inputs      : mat
    Outputs     : result
    Return      :
----------------------------------------------------------------------------*/
void hmatMakeHMatFromMat(hmatrix *result,matrix *mat)
{
    result->m11 = mat->m11;
    result->m21 = mat->m21;
    result->m31 = mat->m31;
    result->m41 = 0.0f;

    result->m12 = mat->m12;
    result->m22 = mat->m22;
    result->m32 = mat->m32;
    result->m42 = 0.0f;

    result->m13 = mat->m13;
    result->m23 = mat->m23;
    result->m33 = mat->m33;
    result->m43 = 0.0f;

    result->m14 = 0.0f;
    result->m24 = 0.0f;
    result->m34 = 0.0f;
    result->m44 = 1.0f;
}

/*-----------------------------------------------------------------------------
    Name        : hmatCreateHMatFromHVecs
    Description : create a matrix, given four 4-tuple vectors.
    Inputs      : col1,col2,col3,col4 vectors
    Outputs     : result
    Return      :
----------------------------------------------------------------------------*/
void hmatCreateHMatFromHVecs(hmatrix *result,hvector *col1,hvector *col2,hvector *col3,hvector *col4)
{
    hmatPutHVectIntoHMatrixCol1(*col1,*result);
    hmatPutHVectIntoHMatrixCol2(*col2,*result);
    hmatPutHVectIntoHMatrixCol3(*col3,*result);
    hmatPutHVectIntoHMatrixCol4(*col4,*result);
}

/*-----------------------------------------------------------------------------
    Name        : hmatMultiplyHMatByHMat
    Description : result = first * second, where matrices are 4 X 4
    Inputs      : first,second
    Outputs     : result
    Return      :
    Warning     : result cannot be the same matrix as first or second.
----------------------------------------------------------------------------*/
void hmatMultiplyHMatByHMat(hmatrix *result,hmatrix *first,hmatrix *second)
{
    result->m11 = hmatrixdot(first->m11,first->m12,first->m13,first->m14,second->m11,second->m21,second->m31,second->m41);
    result->m12 = hmatrixdot(first->m11,first->m12,first->m13,first->m14,second->m12,second->m22,second->m32,second->m42);
    result->m13 = hmatrixdot(first->m11,first->m12,first->m13,first->m14,second->m13,second->m23,second->m33,second->m43);
    result->m14 = hmatrixdot(first->m11,first->m12,first->m13,first->m14,second->m14,second->m24,second->m34,second->m44);

    result->m21 = hmatrixdot(first->m21,first->m22,first->m23,first->m24,second->m11,second->m21,second->m31,second->m41);
    result->m22 = hmatrixdot(first->m21,first->m22,first->m23,first->m24,second->m12,second->m22,second->m32,second->m42);
    result->m23 = hmatrixdot(first->m21,first->m22,first->m23,first->m24,second->m13,second->m23,second->m33,second->m43);
    result->m24 = hmatrixdot(first->m21,first->m22,first->m23,first->m24,second->m14,second->m24,second->m34,second->m44);

    result->m31 = hmatrixdot(first->m31,first->m32,first->m33,first->m34,second->m11,second->m21,second->m31,second->m41);
    result->m32 = hmatrixdot(first->m31,first->m32,first->m33,first->m34,second->m12,second->m22,second->m32,second->m42);
    result->m33 = hmatrixdot(first->m31,first->m32,first->m33,first->m34,second->m13,second->m23,second->m33,second->m43);
    result->m34 = hmatrixdot(first->m31,first->m32,first->m33,first->m34,second->m14,second->m24,second->m34,second->m44);

    result->m41 = hmatrixdot(first->m41,first->m42,first->m43,first->m44,second->m11,second->m21,second->m31,second->m41);
    result->m42 = hmatrixdot(first->m41,first->m42,first->m43,first->m44,second->m12,second->m22,second->m32,second->m42);
    result->m43 = hmatrixdot(first->m41,first->m42,first->m43,first->m44,second->m13,second->m23,second->m33,second->m43);
    result->m44 = hmatrixdot(first->m41,first->m42,first->m43,first->m44,second->m14,second->m24,second->m34,second->m44);
}

/*-----------------------------------------------------------------------------
    Name        : hmatMultiplyHMatByHVec
    Description : result = matrix * vector (all are homogenous)
    Inputs      : matrix, vector
    Outputs     : result
    Return      :
    Warning     : hvector result cannot be the same as hvector vector.
----------------------------------------------------------------------------*/
void hmatMultiplyHMatByHVec(hvector *result,hmatrix *matrix,hvector *vector)
{
    result->x = hmatrixdot(matrix->m11,matrix->m12,matrix->m13,matrix->m14,vector->x,vector->y,vector->z,vector->w);
    result->y = hmatrixdot(matrix->m21,matrix->m22,matrix->m23,matrix->m24,vector->x,vector->y,vector->z,vector->w);
    result->z = hmatrixdot(matrix->m31,matrix->m32,matrix->m33,matrix->m34,vector->x,vector->y,vector->z,vector->w);
    result->w = hmatrixdot(matrix->m41,matrix->m42,matrix->m43,matrix->m44,vector->x,vector->y,vector->z,vector->w);
}

/*-----------------------------------------------------------------------------
    Name        : hmatMultiplyHVecByHMat
    Description : result = vector * matrix (all homogenous)
    Inputs      : vector, matrix
    Outputs     : result
    Return      :
    Warning     : hvector result cannot be the same as hvector vector.
----------------------------------------------------------------------------*/
void hmatMultiplyHVecByHMat(hvector *result,hvector *vector,hmatrix *matrix)
{
    result->x = hmatrixdot(vector->x,vector->y,vector->z,vector->w,matrix->m11,matrix->m21,matrix->m31,matrix->m41);
    result->y = hmatrixdot(vector->x,vector->y,vector->z,vector->w,matrix->m12,matrix->m22,matrix->m32,matrix->m42);
    result->z = hmatrixdot(vector->x,vector->y,vector->z,vector->w,matrix->m13,matrix->m23,matrix->m33,matrix->m43);
    result->w = hmatrixdot(vector->x,vector->y,vector->z,vector->w,matrix->m14,matrix->m24,matrix->m34,matrix->m44);
}

/*-----------------------------------------------------------------------------
    Name        : hmatTranspose
    Description : Transposes a matrix (exchanges rows and columns)
    Inputs      : matrix
    Outputs     : matrix
    Return      :
----------------------------------------------------------------------------*/
void hmatTranspose(hmatrix *matrix)
{
    real32 temp;

    swap(matrix->m12,matrix->m21,temp);
    swap(matrix->m13,matrix->m31,temp);
    swap(matrix->m14,matrix->m41,temp);
    swap(matrix->m23,matrix->m32,temp);
    swap(matrix->m24,matrix->m42,temp);
    swap(matrix->m34,matrix->m43,temp);
}

/*-----------------------------------------------------------------------------
    Name        : hmatCopyAndTranspose
    Description : takes src matrix, and puts its transpose in dsttrans
    Inputs      : src
    Outputs     : dsttrans
    Return      :
----------------------------------------------------------------------------*/
void hmatCopyAndTranspose(hmatrix *src,hmatrix *dsttrans)
{
    dsttrans->m11 = src->m11;   dsttrans->m12 = src->m21;   dsttrans->m13 = src->m31;   dsttrans->m14 = src->m41;
    dsttrans->m21 = src->m12;   dsttrans->m22 = src->m22;   dsttrans->m23 = src->m32;   dsttrans->m24 = src->m42;
    dsttrans->m31 = src->m13;   dsttrans->m32 = src->m23;   dsttrans->m33 = src->m33;   dsttrans->m34 = src->m43;
    dsttrans->m41 = src->m14;   dsttrans->m42 = src->m24;   dsttrans->m43 = src->m34;   dsttrans->m44 = src->m44;
}

/*-----------------------------------------------------------------------------
    Name        : hmatMakeRotAboutZ
    Description : makes rotation about z-axis matrix
    Inputs      : costheta, sintheta
    Outputs     : matrix
    Return      :
----------------------------------------------------------------------------*/
void hmatMakeRotAboutZ(hmatrix *matrix,real32 costheta,real32 sintheta)
{
    matrix->m11 = costheta;     matrix->m12 = -sintheta;    matrix->m13 = 0.0f;    matrix->m14 = 0.0f;
    matrix->m21 = sintheta;     matrix->m22 = costheta;     matrix->m23 = 0.0f;    matrix->m24 = 0.0f;
    matrix->m31 = 0.0f;         matrix->m32 = 0.0f;         matrix->m33 = 1.0f;    matrix->m34 = 0.0f;
    matrix->m41 = 0.0f;         matrix->m42 = 0.0f;         matrix->m43 = 0.0f;    matrix->m44 = 1.0f;
}

/*-----------------------------------------------------------------------------
    Name        : hmatMakeRotAboutX
    Description : makes rotation about x-axis matrix
    Inputs      : costheta, sintheta
    Outputs     : matrix
    Return      :
----------------------------------------------------------------------------*/
void hmatMakeRotAboutX(hmatrix *matrix,real32 costheta,real32 sintheta)
{
    matrix->m11 = 1.0f;            matrix->m12 = 0.0f;         matrix->m13 = 0.0f;         matrix->m14 = 0.0f;
    matrix->m21 = 0.0f;            matrix->m22 = costheta;     matrix->m23 = -sintheta;    matrix->m24 = 0.0f;
    matrix->m31 = 0.0f;            matrix->m32 = sintheta;     matrix->m33 = costheta;     matrix->m34 = 0.0f;
    matrix->m41 = 0.0f;            matrix->m42 = 0.0f;         matrix->m43 = 0.0f;         matrix->m44 = 1.0f;
}

/*-----------------------------------------------------------------------------
    Name        : hmatMakeRotAboutY
    Description : makes rotation about y-axis matrix
    Inputs      : costheta, sintheta
    Outputs     : matrix
    Return      :
----------------------------------------------------------------------------*/
void hmatMakeRotAboutY(hmatrix *matrix,real32 costheta,real32 sintheta)
{
    matrix->m11 = costheta;     matrix->m12 = 0.0f;            matrix->m13 = sintheta;    matrix->m14 = 0.0f;
    matrix->m21 = 0.0f;         matrix->m22 = 1.0f;            matrix->m23 = 0.0f;        matrix->m24 = 0.0f;
    matrix->m31 = -sintheta;    matrix->m32 = 0.0f;            matrix->m33 = costheta;    matrix->m34 = 0.0f;
    matrix->m41 = 0.0f;         matrix->m42 = 0.0f;            matrix->m43 = 0.0f;        matrix->m44 = 1.0f;
}

/*-----------------------------------------------------------------------------
    Name        : hmatPrintHMatrix
    Description : prints a homogenous matrix
    Inputs      : hmatrix
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void hmatPrintHMatrix(hmatrix *a)
{
    printf("(%8f %8f %8f %8f)\n",a->m11,a->m12,a->m13,a->m14);
    printf("(%8f %8f %8f %8f)\n",a->m21,a->m22,a->m23,a->m24);
    printf("(%8f %8f %8f %8f)\n",a->m31,a->m32,a->m33,a->m34);
    printf("(%8f %8f %8f %8f)\n",a->m41,a->m42,a->m43,a->m44);
}



/*=============================================================================
    3 X 3 Matrix Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : matCreateHMatFromHVecs
    Description : create a matrix, given three 3-tuple vectors.
    Inputs      : col1,col2,col3 vectors
    Outputs     : result
    Return      :
----------------------------------------------------------------------------*/
void matCreateMatFromVecs(matrix *result,vector *col1,vector *col2,vector *col3)
{
    matPutVectIntoMatrixCol1(*col1,*result);
    matPutVectIntoMatrixCol2(*col2,*result);
    matPutVectIntoMatrixCol3(*col3,*result);
}

/*-----------------------------------------------------------------------------
    Name        : matMultiplyMatByMat
    Description : result = first * second, where matrices are 3 X 3
    Inputs      : first,second
    Outputs     : result
    Return      :
    Warning     : result cannot be the same matrix as first or second.
----------------------------------------------------------------------------*/
void matMultiplyMatByMat(matrix *result,matrix *first,matrix *second)
{
    result->m11 = matrixdot(first->m11,first->m12,first->m13,second->m11,second->m21,second->m31);
    result->m12 = matrixdot(first->m11,first->m12,first->m13,second->m12,second->m22,second->m32);
    result->m13 = matrixdot(first->m11,first->m12,first->m13,second->m13,second->m23,second->m33);

    result->m21 = matrixdot(first->m21,first->m22,first->m23,second->m11,second->m21,second->m31);
    result->m22 = matrixdot(first->m21,first->m22,first->m23,second->m12,second->m22,second->m32);
    result->m23 = matrixdot(first->m21,first->m22,first->m23,second->m13,second->m23,second->m33);

    result->m31 = matrixdot(first->m31,first->m32,first->m33,second->m11,second->m21,second->m31);
    result->m32 = matrixdot(first->m31,first->m32,first->m33,second->m12,second->m22,second->m32);
    result->m33 = matrixdot(first->m31,first->m32,first->m33,second->m13,second->m23,second->m33);
}

/*-----------------------------------------------------------------------------
    Name        : matMultiplyMatByVec
    Description : result = matrix * vector
    Inputs      : matrix, vector
    Outputs     : result
    Return      :
    Warning     : vector result cannot be the same as vector vector.
----------------------------------------------------------------------------*/
void matMultiplyMatByVec(vector *result,matrix *matrix,vector *vector)
{
    result->x = matrixdot(matrix->m11,matrix->m12,matrix->m13,vector->x,vector->y,vector->z);
    result->y = matrixdot(matrix->m21,matrix->m22,matrix->m23,vector->x,vector->y,vector->z);
    result->z = matrixdot(matrix->m31,matrix->m32,matrix->m33,vector->x,vector->y,vector->z);
}

/*-----------------------------------------------------------------------------
    Name        : matMultiplyVecByMat
    Description : result = vector * matrix
    Inputs      : vector, matrix
    Outputs     : result
    Return      :
    Warning     : vector result cannot be the same as vector vector.
----------------------------------------------------------------------------*/
void matMultiplyVecByMat(vector *result,vector *vector,matrix *matrix)
{
    result->x = matrixdot(vector->x,vector->y,vector->z,matrix->m11,matrix->m21,matrix->m31);
    result->y = matrixdot(vector->x,vector->y,vector->z,matrix->m12,matrix->m22,matrix->m32);
    result->z = matrixdot(vector->x,vector->y,vector->z,matrix->m13,matrix->m23,matrix->m33);
}

/*-----------------------------------------------------------------------------
    Name        : matTranspose
    Description : Transposes a matrix (exchanges rows and columns)
    Inputs      : matrix
    Outputs     : matrix
    Return      :
----------------------------------------------------------------------------*/
void matTranspose(matrix *matrix)
{
    real32 temp;

    swap(matrix->m21,matrix->m12,temp);
    swap(matrix->m31,matrix->m13,temp);
    swap(matrix->m32,matrix->m23,temp);
}

/*-----------------------------------------------------------------------------
    Name        : matCopyAndTranspose
    Description : takes src matrix, and puts its transpose in dsttrans
    Inputs      : src
    Outputs     : dsttrans
    Return      :
----------------------------------------------------------------------------*/
void matCopyAndTranspose(matrix *src,matrix *dsttrans)
{
    dsttrans->m11 = src->m11;   dsttrans->m12 = src->m21;   dsttrans->m13 = src->m31;
    dsttrans->m21 = src->m12;   dsttrans->m22 = src->m22;   dsttrans->m23 = src->m32;
    dsttrans->m31 = src->m13;   dsttrans->m32 = src->m23;   dsttrans->m33 = src->m33;
}

/*-----------------------------------------------------------------------------
    Name        : matMakeRotAboutZ
    Description : makes rotation about z-axis matrix
    Inputs      : costheta, sintheta
    Outputs     : matrix
    Return      :
----------------------------------------------------------------------------*/
void matMakeRotAboutZ(matrix *matrix,real32 costheta,real32 sintheta)
{
    matrix->m11 = costheta;     matrix->m12 = -sintheta;    matrix->m13 = 0.0f;
    matrix->m21 = sintheta;     matrix->m22 = costheta;     matrix->m23 = 0.0f;
    matrix->m31 = 0.0f;         matrix->m32 = 0.0f;         matrix->m33 = 1.0f;
}

/*-----------------------------------------------------------------------------
    Name        : matMakeRotAboutX
    Description : makes rotation about x-axis matrix
    Inputs      : costheta, sintheta
    Outputs     : matrix
    Return      :
----------------------------------------------------------------------------*/
void matMakeRotAboutX(matrix *matrix,real32 costheta,real32 sintheta)
{
    matrix->m11 = 1.0f;            matrix->m12 = 0.0f;         matrix->m13 = 0.0f;
    matrix->m21 = 0.0f;            matrix->m22 = costheta;     matrix->m23 = -sintheta;
    matrix->m31 = 0.0f;            matrix->m32 = sintheta;     matrix->m33 = costheta;
}

/*-----------------------------------------------------------------------------
    Name        : matMakeRotAboutY
    Description : makes rotation about y-axis matrix
    Inputs      : costheta, sintheta
    Outputs     : matrix
    Return      :
----------------------------------------------------------------------------*/
void matMakeRotAboutY(matrix *matrix,real32 costheta,real32 sintheta)
{
    matrix->m11 = costheta;     matrix->m12 = 0.0f;            matrix->m13 = sintheta;
    matrix->m21 = 0.0f;         matrix->m22 = 1.0f;            matrix->m23 = 0.0f;
    matrix->m31 = -sintheta;    matrix->m32 = 0.0f;            matrix->m33 = costheta;
}

/*-----------------------------------------------------------------------------
    Name        : matPrintmatrix
    Description : prints a matrix
    Inputs      : matrix
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void matPrintmatrix(matrix *a)
{
    printf("(%8f %8f %8f)\n",a->m11,a->m12,a->m13);
    printf("(%8f %8f %8f)\n",a->m21,a->m22,a->m23);
    printf("(%8f %8f %8f)\n",a->m31,a->m32,a->m33);
}

