/*=============================================================================
    Name    : fastmath.h
    Purpose : Definitions for fastmath.c

    Created 9/9/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___FASTMATH_H
#define ___FASTMATH_H

#include "Types.h"

/*=============================================================================
    Macros:
=============================================================================*/

#define fsqrt(x) (real32)fmathSqrtDouble((real32)(x))
#define fmathSqrt(x) (real32)fmathSqrtDouble((real32)(x))

/*=============================================================================
    Functions:
=============================================================================*/

//real32 fmathSqrt(real32 num);
//real32 fmathSqrtGS(real32 num);
double fmathSqrtDouble(double f);
void fmathInit(void);

#endif

