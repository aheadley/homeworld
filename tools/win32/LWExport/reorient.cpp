/*
** REORIENT.CPP : Code to handle reorientation of positions and rotations from lightwave
**				  to Homeworld.
*/

#include "math.h"

#include "reorient.h"
#include "matrix.h"

/*-----------------------------------------------------------------------------
    Name        : nisRotateAboutVector
    Description : Make a rotation matrix about an arbitrary vector
    Inputs      : axis - vector to rotate about (must be a unit vector)
                  radians - angle (in radians) to rotate by
    Outputs     : m - matrix (3x3) to fill
    Return      :
----------------------------------------------------------------------------*/
void nisRotateAboutVector(real32* m, vector* axis, real32 radians)
{
    real32 s, c;
    real32 xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
    double rads;
    real32 x = axis->x;
    real32 y = axis->y;
    real32 z = axis->z;

    rads = (double)radians;
    s = (real32)sin(rads);
    c = (real32)cos(rads);

#define M(row,col) m[((col<<1) + col)+row]

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

    M(1,0) = (one_c * xy) + zs;
    M(1,1) = (one_c * yy) + c;
    M(1,2) = (one_c * yz) - xs;

    M(2,0) = (one_c * zx) - ys;
    M(2,1) = (one_c * yz) + xs;
    M(2,2) = (one_c * zz) + c;

#undef M
}
//matrix HPBIdentityShip = {0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f};
matrix HPBIdentityShip = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
/*-----------------------------------------------------------------------------
    Name        : nisShipEulerToMatrix
    Description : Builds a coordinate system matrix from a heading,pitch,bank
                    triplicate.
    Inputs      : rotVector - vectror whose members represent the heading, pitch and bank
    Outputs     : coordsys - where to store the matrix
    Return      :
----------------------------------------------------------------------------*/
void nisShipEulerToMatrix(matrix *coordsys, vector *rotVector)
{
    matrix rotation, tempMatrix;
    vector rotateAbout = {-1.0f, 0.0f, 0.0f};

    *coordsys = IdentityMatrix;
    //rotate about up vector (heading)
    nisRotateAboutVector((real32 *)&rotation, &rotateAbout, rotVector->x);
    matMultiplyMatByMat(&tempMatrix, &rotation, coordsys);
    *coordsys = tempMatrix;
    //rotate about heading vector (pitch in lightwave, bank actually)
    matGetVectFromMatrixCol3(rotateAbout, *coordsys);
    nisRotateAboutVector((real32 *)&rotation, &rotateAbout, rotVector->y);
    matMultiplyMatByMat(&tempMatrix, &rotation, coordsys);
    *coordsys = tempMatrix;
    //rotate about heading vector (bank in lightwave, pitch actually)
    matGetVectFromMatrixCol2(rotateAbout, *coordsys);
    nisRotateAboutVector((real32 *)&rotation, &rotateAbout, -rotVector->z);
    matMultiplyMatByMat(&tempMatrix, &rotation, coordsys);
    *coordsys = tempMatrix;
}

void ShipReorientPosition(float *xp, float *yp, float *zp)
{
	float x, y, z;

	x = *xp;
	y = *yp;
	z = *zp;

	*xp = y;
	*yp = z;
	*zp = -x;
}

void WorldReorientPosition(float *xp, float *yp, float *zp)
{
	float x, y, z;

	x = *xp;
	y = *yp;
	z = *zp;

	*xp = z;
	*yp = -x;
	*zp = y;
}

void ShipReorientAngles(float *xr, float *yr, float *zr)
{
	float h, p, b;

	h = *xr;
	p = *yr;
	b = *zr;

//  *xr = -DEG_TO_RAD(h);
//  *yr = -DEG_TO_RAD(b);
//  *zr = DEG_TO_RAD(p);
	*xr = DEG_TO_RAD(h);
	*yr = DEG_TO_RAD(p);
	*zr = DEG_TO_RAD(b);
}

void WorldReorientAngles(float *xr, float *yr, float *zr)
{
	float h, p, b;

	h = *xr;
	p = *yr;
	b = *zr;

	*xr = DEG_TO_RAD(b);
	*yr = -DEG_TO_RAD(p);
	*zr = DEG_TO_RAD(h);
}

void VectorFromShipAngle(float *xr, float *yr, float *zr)
{
/*
	float h, p, b;
	matrix x, y, z;
	vector v = {0.0f, 1.0f, 0.0f}, r;

	h = *xr;
	p = *yr;
	b = *zr;

	matMakeRotAboutX(&x, (float)cos(h), (float)sin(h));
	matMakeRotAboutY(&y, (float)cos(p), (float)sin(p));
	matMakeRotAboutZ(&z, (float)cos(b), (float)sin(b));

	matMultiplyMatByVec(&r, &z, &v);
	matMultiplyMatByVec(&v, &x, &r);
	matMultiplyMatByVec(&r, &y, &v);

	*xr = r.x;
	*yr = r.y;
	*zr = r.z;
*/
    matrix mat;
    vector rotVector = {*xr, *yr, *zr};
    nisShipEulerToMatrix(&mat, &rotVector);
    matGetVectFromMatrixCol2(rotVector, mat);
    *xr = rotVector.x;
    *yr = rotVector.y;
    *zr = rotVector.z;
}

void VectorFromWorldAngle(float *xr, float *yr, float *zr)
{
}
