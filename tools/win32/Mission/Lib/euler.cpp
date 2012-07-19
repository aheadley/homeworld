// Copyright (c) 1999 Relic Entertainment Inc.
// Original code written by Jason Dorie
// Adapted by Janik Joire
//
// $History: $

#include <math.h>
#include <float.h>
#include "euler.h"

Euler::Euler(float X, float Y, float Z, long Order)
{
	x = X;
	y = Y;
	z = Z;
	Ord = Order;
}

/* Construct matrix from Euler angles */
void Euler::ToMatrix(float *M)
{
	long	i,j,k,h,n,s,f;
	float	ci, cj, ch, si, sj, sh, cc, cs, sc, ss, t;

	EulGetOrd(Ord,i,j,k,h,n,s,f);

	if(f == EulFrmR)
	{
		t = x;
		x = z;
		z = t;
	}

	if(n == EulParOdd)
	{
		x = -x;
		y = -y;
		z = -z;
	}

	ci = (float)cos(x); cj = (float)cos(y); ch = (float)cos(z);
	si = (float)sin(x); sj = (float)sin(y); sh = (float)sin(z);
	cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;

	if(s == EulRepYes)
	{
		M[m(i,i)] = cj;	  M[m(i,j)] =  sj*si;    M[m(i,k)] =  sj*ci;
		M[m(j,i)] = sj*sh;  M[m(j,j)] = -cj*ss+cc; M[m(j,k)] = -cj*cs-sc;
		M[m(k,i)] = -sj*ch; M[m(k,j)] =  cj*sc+cs; M[m(k,k)] =  cj*cc-ss;
	}
	else
	{
		M[m(i,i)] = cj*ch; M[m(i,j)] = sj*sc-cs; M[m(i,k)] = sj*cc+ss;
		M[m(j,i)] = cj*sh; M[m(j,j)] = sj*ss+cc; M[m(j,k)] = sj*cs-sc;
		M[m(k,i)] = -sj;	 M[m(k,j)] = cj*si;    M[m(k,k)] = cj*ci;
	}

	M[m(3,0)] = M[m(3,1)] = M[m(3,2)] = M[m(0,3)] = M[m(1,3)] = M[m(2,3)] = 0.0;
	M[m(3,3)] = 1.0;
}

/* Convert matrix to Euler angles */
void Euler::FromMatrix(float *M, long Order)
{
	long	i,j,k,h,n,s,f;
	float	sy, cy, t;
	float	RadX, RadY, RadZ;

	EulGetOrd(Order,i,j,k,h,n,s,f);

	if(s == EulRepYes)
	{
		sy = (float)sqrt(M[m(i,j)]*M[m(i,j)] + M[m(i,k)]*M[m(i,k)]);
		if(sy > 16*FLT_EPSILON)
		{
			RadX = (float)atan2(M[m(i,j)], M[m(i,k)]);
			RadY = (float)atan2(sy, M[m(i,i)]);
			RadZ = (float)atan2(M[m(j,i)], -M[m(k,i)]);
		}
		else
		{
			RadX = (float)atan2(-M[m(j,k)], M[m(j,j)]);
			RadY = (float)atan2(sy, M[m(i,i)]);
			RadZ = 0.0;
		}
	}
	else
	{
		cy = (float)sqrt(M[m(i,i)]*M[m(i,i)] + M[m(j,i)]*M[m(j,i)]);
		if (cy > 16*FLT_EPSILON)
		{
			RadX = (float)atan2(M[m(k,j)], M[m(k,k)]);
			RadY = (float)atan2(-M[m(k,i)], cy);
			RadZ = (float)atan2(M[m(j,i)], M[m(i,i)]);
		}
		else
		{
			RadX = (float)atan2(-M[m(j,k)], M[m(j,j)]);
			RadY = (float)atan2(-M[m(k,i)], cy);
			RadZ = 0;
		}
	}

	x = RadX;
	y = RadY;
	z = RadZ;

	if(n == EulParOdd)
	{
		x = -x;
		y = -y;
		z = -z;
	}

	if(f == EulFrmR)
	{
		t = x;
		x = z;
		z = t;
	}

	if( (x >= PI) && (z >= PI) )
	{
		x -= PI;
		y = PI - y;
		z -= PI;
	}

	Ord = Order;
}

