// Copyright (c) 1999 Relic Entertainment Inc.
// Original code written by Jason Dorie
// Adapted by Janik Joire
//
// $History: $

#ifndef EULER_H
#define EULER_H

#ifndef PI
#define PI 3.14159265358979323846F
#endif

#define m(i,j) ((j*4)+i)

class Euler
{
public:
	float	x, y, z;
	long	Ord;		// Order information - see below

	Euler() {};
	Euler(float X, float Y, float Z, long Order);

	~Euler() {}

	void ToMatrix(float *M);

	void FromMatrix(float *M, long Order);
};


/*** Order type constants, constructors, extractors ***/

/*
	There are 24 possible conventions, designated by:
		o EulAxI = axis used initially
		o EulPar = parity of axis permutation
		o EulRep = repetition of initial axis as last
		o EulFrm = frame from which axes are taken

	Axes I,J,K will be a permutation of X,Y,Z.
	Axis H will be either I or K, depending on EulRep.
	Frame S takes axes from initial static frame.
	If ord = (AxI=X, Par=Even, Rep=No, Frm=S), then
	{a,b,c,ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v
	rotates v around Z by c radians.
*/

#define EulFrmS		0
#define EulFrmR		1
#define EulFrm(ord)	((unsigned)(ord)&1)
#define EulRepNo		0
#define EulRepYes		1
#define EulRep(ord)	(((unsigned)(ord)>>1)&1)
#define EulParEven	0
#define EulParOdd		1
#define EulPar(ord)	(((unsigned)(ord)>>2)&1)
#define EulSafe		"\000\001\002\000"
#define EulNext		"\001\002\000\001"
#define EulAxI(ord)  ((long)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((long)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((long)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))

/* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;\
    n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}

/* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))

/* Static axes */
#define EulOrdXYZs    EulOrd(0,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(0,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(0,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(0,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(1,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(1,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(1,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(1,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(2,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(2,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(2,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(2,EulParOdd,EulRepYes,EulFrmS)

/* Rotating axes */
#define EulOrdZYXr    EulOrd(0,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(0,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(0,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(0,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(1,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(1,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(1,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(1,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(2,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(2,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(2,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(2,EulParOdd,EulRepYes,EulFrmR)

#endif
