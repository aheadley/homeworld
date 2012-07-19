/*=============================================================================
    Name    : fastmath.c
    Purpose : Provides generic fast math routines

    Created 9/9/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "fastmath.h"

real32 fmathSqrt(real32 num)
{
    real32 x = num;
    sdword *exp = (sdword *)&x;
    sdword modexp = *exp;

    modexp >>= 23;
    modexp &= 255;

    modexp -= 127;
    modexp >>= 1;
    modexp += 127;

    modexp <<= 23;

    *exp = (*exp & 0x807fffff) | modexp;

    x = 0.5f * (x + num/x);
    x = 0.5f * (x + num/x);

    return x;
}


