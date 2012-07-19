// Copyright (c) 1998 Relic Entertainment Inc.
// Original code written by Keith Hentschel
// Adapted by Janik Joire
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "matrix.h"

float identitymatrix[16]=
{
	1.0F,0.0F,0.0F,0.0F,
	0.0F,1.0F,0.0F,0.0F,
	0.0F,0.0F,1.0F,0.0F,
	0.0F,0.0F,0.0F,1.0F
};

enum
{
    M00=0, M01=4, M02=8,  M03=12,
    M10=1, M11=5, M12=9,  M13=13,
    M20=2, M21=6, M22=10, M23=14,
    M30=3, M31=7, M32=11, M33=15
};

void matInvert(float *c,float* m)
{

    float det;
    float tmp[16];    // allow out = in
	float d12,d13,d23,d24,d34,d41;
	float im11,im12,im13,im14;
	
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

    if((m41 != 0.0F) || (m42 != 0.0F) || (m43 != 0.0F) || (m44 != 1.0F))
    {
        matInvertEx(c,m);
        return;
    }

    // Inverse=adjoint/det

    tmp[0]=m22*m33-m23*m32;
    tmp[1]=m23*m31-m21*m33;
    tmp[2]=m21*m32-m22*m31;

    // Compute determinant using cofactors
    det=m11*tmp[0]+m12*tmp[1]+m13*tmp[2];

    // Singularity test
    if(det == 0.0F) matIdentity(c);
    else
    {
        det=1.0F/det;

        // Compute rest of inverse
        tmp[0]*=det;
        tmp[1]*=det;
        tmp[2]*=det;
        tmp[3]=0.0F;

        im11=m11*det;
        im12=m12*det;
        im13=m13*det;
        im14=m14*det;

        tmp[4]=im13*m32-im12*m33;
        tmp[5]=im11*m33-im13*m31;
        tmp[6]=im12*m31-im11*m32;
        tmp[7]=0.0F;

        // Pre-compute 2x2 dets for first two rows when computing cofactors of last two rows.
		d12=im11*m22-m21*im12;
		d13=im11*m23-m21*im13;
		d23=im12*m23-m22*im13;
		d24=im12*m24-m22*im14;
		d34=im13*m24-m23*im14;
		d41=im14*m21-m24*im11;

        tmp[8]=d23;
        tmp[9]=-d13;
        tmp[10]=d12;
        tmp[11]=0.0F;

        tmp[12]=-(m32*d34-m33*d24+m34*d23);
        tmp[13]=(m31*d34+m33*d41+m34*d13);
        tmp[14]=-(m31*d24+m32*d41+m34*d12);
        tmp[15]=1.0F;

        memcpy(c,tmp,sizeof(float)*16);
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

	return;
}

void matInvertEx(float* c,float * m)
{
    float r1[2][2],r2[2][2],r3[2][2],r4[2][2],r5[2][2],r6[2][2],r7[2][2];
    float *A;
    float *C;
    float one_over_det;

    A=m;
    C=c;

    // A is the 4x4 source matrix (to be inverted).
    // C is the 4x4 destination matrix
    // a11 is the 2x2 matrix in the upper left quadrant of A
    // a12 is the 2x2 matrix in the upper right quadrant of A
    // a21 is the 2x2 matrix in the lower left quadrant of A
    // a22 is the 2x2 matrix in the lower right quadrant of A
    // similarly, cXX are the 2x2 quadrants of the destination matrix

    // R1=inverse(a11)
    one_over_det=1.0F/((A[M00]*A[M11])-(A[M10]*A[M01]));
    r1[0][0]=one_over_det*A[M11];
    r1[0][1]=one_over_det*(-A[M01]);
    r1[1][0]=one_over_det*(-A[M10]);
    r1[1][1]=one_over_det*A[M00];

    // R2=a21xR1
    r2[0][0]=A[M20]*r1[0][0]+A[M21]*r1[1][0];
    r2[0][1]=A[M20]*r1[0][1]+A[M21]*r1[1][1];
    r2[1][0]=A[M30]*r1[0][0]+A[M31]*r1[1][0];
    r2[1][1]=A[M30]*r1[0][1]+A[M31]*r1[1][1];

    // R3=R1xa12
    r3[0][0]=r1[0][0]*A[M02]+r1[0][1]*A[M12];
    r3[0][1]=r1[0][0]*A[M03]+r1[0][1]*A[M13];
    r3[1][0]=r1[1][0]*A[M02]+r1[1][1]*A[M12];
    r3[1][1]=r1[1][0]*A[M03]+r1[1][1]*A[M13];

    // R4=a21xR3
    r4[0][0]=A[M20]*r3[0][0]+A[M21]*r3[1][0];
    r4[0][1]=A[M20]*r3[0][1]+A[M21]*r3[1][1];
    r4[1][0]=A[M30]*r3[0][0]+A[M31]*r3[1][0];
    r4[1][1]=A[M30]*r3[0][1]+A[M31]*r3[1][1];

    // R5=R4-a22
    r5[0][0]=r4[0][0]-A[M22];
    r5[0][1]=r4[0][1]-A[M23];
    r5[1][0]=r4[1][0]-A[M32];
    r5[1][1]=r4[1][1]-A[M33];

    // R6=inverse(R5)
    one_over_det=1.0F/((r5[0][0]*r5[1][1])-(r5[1][0]*r5[0][1]));
    r6[0][0]=one_over_det*r5[1][1];
    r6[0][1]=one_over_det*(-r5[0][1]);
    r6[1][0]=one_over_det*(-r5[1][0]);
    r6[1][1]=one_over_det*r5[0][0];

    // c12=R3xR6/
	C[M02]=r3[0][0]*r6[0][0]+r3[0][1]*r6[1][0];
	C[M03]=r3[0][0]*r6[0][1]+r3[0][1]*r6[1][1];
	C[M12]=r3[1][0]*r6[0][0]+r3[1][1]*r6[1][0];
	C[M13]=r3[1][0]*r6[0][1]+r3[1][1]*r6[1][1];

	// c21=R6xR2
	C[M20]=r6[0][0]*r2[0][0]+r6[0][1]*r2[1][0];
	C[M21]=r6[0][0]*r2[0][1]+r6[0][1]*r2[1][1];
	C[M30]=r6[1][0]*r2[0][0]+r6[1][1]*r2[1][0];
	C[M31]=r6[1][0]*r2[0][1]+r6[1][1]*r2[1][1];

	// R7=R3xc21
	r7[0][0]=r3[0][0]*C[M20]+r3[0][1]*C[M30];
	r7[0][1]=r3[0][0]*C[M21]+r3[0][1]*C[M31];
	r7[1][0]=r3[1][0]*C[M20]+r3[1][1]*C[M30];
	r7[1][1]=r3[1][0]*C[M21]+r3[1][1]*C[M31];

    // c11=R1-R7
    C[M00]=r1[0][0]-r7[0][0];
    C[M01]=r1[0][1]-r7[0][1];
    C[M10]=r1[1][0]-r7[1][0];
    C[M11]=r1[1][1]-r7[1][1];

    // c22=-R6
    C[M22]=-r6[0][0];
    C[M23]=-r6[0][1];
    C[M32]=-r6[1][0];
    C[M33]=-r6[1][1];

	return;
}

// Matrix multiply contributed by Thomas Malik
// c[16] = 4x4 dest matrix
// a[16] = 4x4 source matrix, also dest matrix if c = NULL
// b[16] = 4x4 transform matrix
void matProduct(float *c,float *a,float *b)
{
    int i,flag;

	float ai0,ai1,ai2,ai3;
	float tmp[16];

	flag=0;

	if(c == NULL)
	{
		c=tmp;
		flag=1;
	}

#define A(row,col) a[(col<<2)+row]
#define B(row,col) b[(col<<2)+row]
#define P(row,col) c[(col<<2)+row]

    for(i=0;i<4;i++)
    {
		ai0=A(i,0); ai1=A(i,1); ai2=A(i,2); ai3=A(i,3);

		P(i,0)=ai0*B(0,0)+ai1*B(1,0)+ai2*B(2,0)+ai3*B(3,0);
	    P(i,1)=ai0*B(0,1)+ai1*B(1,1)+ai2*B(2,1)+ai3*B(3,1);
	    P(i,2)=ai0*B(0,2)+ai1*B(1,2)+ai2*B(2,2)+ai3*B(3,2);
	    P(i,3)=ai0*B(0,3)+ai1*B(1,3)+ai2*B(2,3)+ai3*B(3,3);
    }

#undef A
#undef B
#undef P

    if(flag == 1) memcpy(a,c,sizeof(float)*16);

	return;
}

// c[3] = dest vector or NULL
// a[3] = source vector, also dest vector if c = NULL
// b[16] = 4x4 transform matrix
void matVectProduct(float *c,float *a,float *b)
{
    int flag;

	float tmp[3];

	flag=0;

	if(c == NULL)
	{
		c=tmp;
		flag=1;
	}

	c[0]=b[0]*a[0]+b[4]*a[1]+b[8]*a[2]+b[12];
	c[1]=b[1]*a[0]+b[5]*a[1]+b[9]*a[2]+b[13];
	c[2]=b[2]*a[0]+b[6]*a[1]+b[10]*a[2]+b[14];

    if(flag == 1) memcpy(a,c,sizeof(float)*3);

	return;
}

// c[3] = dest vector or NULL
// a[3] = source vector, also dest vector if c = NULL
// b[3] = transform vector
void matCrossProduct(float *c,float *a,float *b)
{
    int flag;

	float tmp[3];

	flag=0;

	if(c == NULL)
	{
		c=tmp;
		flag=1;
	}

	c[0]=a[1]*b[2]-a[2]*b[1];
	c[1]=-a[0]*b[2]+a[2]*b[0];
	c[2]=a[0]*b[1]-a[1]*b[0];

    if(flag == 1) memcpy(a,c,sizeof(float)*3);

	return;
}

void matIdentity(float *a)
{
    memcpy(a,identitymatrix,sizeof(float)*16);

	return;
}

void matTranslate(float *a,float x,float y,float z)
{
	a[12]=a[0]*x+a[4]*y+a[8]*z+a[12];
	a[13]=a[1]*x+a[5]*y+a[9]*z+a[13];
	a[14]=a[2]*x+a[6]*y+a[10]*z+a[14];
	a[15]=a[3]*x+a[7]*y+a[11]*z+a[15];

	return;
}

void matScale(float *a,float x,float y,float z)
{
    a[0]*=x; a[4]*=y; a[8]*=z;
    a[1]*=x; a[5]*=y; a[9]*=z;
    a[2]*=x; a[6]*=y; a[10]*=z;
    a[3]*=x; a[7]*=y; a[11]*=z;

	return;
}

void matRotate(float* a,float* axis,float angle)
{
    float mag,s,c;
    float xx,yy,zz,xy,yz,zx,xs,ys,zs,one_c;
    float x,y,z;
	float m[16];

    double rads;

	x=axis[0];
    y=axis[1];
    z=axis[2];

    rads=(double)angle*PI/180.0F;
    s=(float)sin(rads);
    c=(float)cos(rads);

    mag=(float)sqrt(x*x+y*y+z*z);
    if(mag == 0.0)
    {
	    matIdentity(m);
	    return;
    }

    x/=mag;
    y/=mag;
    z/=mag;

#define M(row,col) m[(col<<2)+row]

    xx=x*x;
    yy=y*y;
    zz=z*z;
    xy=x*y;
    yz=y*z;
    zx=z*x;
    xs=x*s;
    ys=y*s;
    zs=z*s;
    one_c=1.0F-c;

    M(0,0)=(one_c*xx)+c;
    M(0,1)=(one_c*xy)-zs;
    M(0,2)=(one_c*zx)+ys;
    M(0,3)=0.0F;

    M(1,0)=(one_c*xy)+zs;
    M(1,1)=(one_c*yy)+c;
    M(1,2)=(one_c*yz)-xs;
    M(1,3)=0.0F;

    M(2,0)=(one_c*zx)-ys;
    M(2,1)=(one_c*yz)+xs;
    M(2,2)=(one_c*zz)+c;
    M(2,3)=0.0F;

    M(3,0)=0.0F;
    M(3,1)=0.0F;
    M(3,2)=0.0F;
    M(3,3)=1.0F;

#undef M

	matProduct(NULL,a,m);
	
	return;
}

void matNormalize(float *a)
{
    float mag;

    mag=(float)sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
    if(mag != 0.0F)
    {
		a[0]/=mag;
		a[1]/=mag;
		a[2]/=mag;
    }

	return;
}

void matLookAt(float *a,float *aEye,float *aFocus,float *aUp)
{
    float m[16];
    float x[3],y[3],z[3];

    z[0]=aEye[0]-aFocus[0];
    z[1]=aEye[1]-aFocus[1];
    z[2]=aEye[2]-aFocus[2];

	matNormalize(z);
    
	y[0]=aUp[0];
    y[1]=aUp[1];
    y[2]=aUp[2];

	matCrossProduct(x,y,z);
	matNormalize(x);

	matCrossProduct(y,z,x);
	matNormalize(y);

#define M(row,col) m[(col<<2)+row]

    M(0,0)=x[0]; M(0,1)=x[1]; M(0,2)=x[2]; M(0,3)=0.0F;
    M(1,0)=y[0]; M(1,1)=y[1]; M(1,2)=y[2]; M(1,3)=0.0F;
    M(2,0)=z[0]; M(2,1)=z[1]; M(2,2)=z[2]; M(2,3)=0.0F;
    M(3,0)=0.0F; M(3,1)=0.0F; M(3,2)=0.0F; M(3,3)=1.0F;

#undef M

	matProduct(NULL,a,m);
	matTranslate(a,-aEye[0],-aEye[1],-aEye[2]);

	return;
}

void matPerspect(float *a,float fov,float aspect,float cnear,float cfar)
{
    float xmin,xmax,ymin,ymax;

    ymax=cnear*(float)tan(fov*PI/360.0F);
    ymin=-ymax;

    xmin=ymin*aspect;
    xmax=ymax*aspect;

    matFrustum(a,xmin,xmax,ymin,ymax,cnear,cfar);

	return;
}

void matFrustum(float *a,float left,float right,float bottom,float top,float cnear,float cfar)
{
    float x,y,aa,bb,cc,dd;
    float m[16];

    x=(2.0F*cnear)/(right-left);
    y=(2.0F*cnear)/(top-bottom);
    aa=(right+left)/(right-left);
    bb=(top+bottom)/(top-bottom);
    cc=-(cfar+cnear)/(cfar-cnear);
    dd=-(2.0F*cfar*cnear)/(cfar-cnear);

#define M(row,ccol) m[(ccol<<2)+row]

    M(0,0)=x;    M(0,1)=0.0F; M(0,2)=aa;    M(0,3)=0.0F;
    M(1,0)=0.0F; M(1,1)=y;    M(1,2)=bb;    M(1,3)=0.0F;
    M(2,0)=0.0F; M(2,1)=0.0F; M(2,2)=cc;    M(2,3)=dd;
    M(3,0)=0.0F; M(3,1)=0.0F; M(3,2)=-1.0F; M(3,3)=0.0F;

#undef M

	matProduct(NULL,a,m);
}
