/*=============================================================================
    Name    : fastmath.c
    Purpose : Provides generic fast math routines

    Created 9/9/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <float.h>
#include "Types.h"
#include "FastMath.h"
#include "Debug.h"

#if 0
real32 fmathSqrtGS(real32 num)
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
#endif

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

static int fmathsqrt_tab[SQRT_TAB_SIZE];

void fmathInitSqrt()
{
    int           i;
    double        f;
    unsigned int  *fi = (unsigned int *) &f + MOST_SIG_OFFSET;

    for (i = 0; i < SQRT_TAB_SIZE/2; i++)
    {
        f = 0; /* Clears least sig part */
        *fi = (i << MANT_SHIFTS) | (EXP_BIAS << EXP_SHIFTS);
        f = sqrt(f);
        fmathsqrt_tab[i] = *fi & MANT_MASK;

        f = 0; /* Clears least sig part */
        *fi = (i << MANT_SHIFTS) | ((EXP_BIAS + 1) << EXP_SHIFTS);
        f = sqrt(f);
        fmathsqrt_tab[i + SQRT_TAB_SIZE/2] = *fi & MANT_MASK;
    }
}

double fmathSqrtDouble(double f)
{
#ifdef _MACOSX
	return sqrt( f );
#else

    unsigned int e;
    unsigned int *fi = (unsigned int *) &f + MOST_SIG_OFFSET;
#if 0
    double test = sqrt(f);
    double test2 = (double)fmathSqrtGS((real32)f);
    double error;
#endif

    if (f == 0.0)
    {
        return 0.0;
    }
    
    e = (*fi >> EXP_SHIFTS) - EXP_BIAS;
    *fi &= MANT_MASK;
    if (e & 1)
        *fi |= EXP_LSB;
    e >>= 1;
    *fi = (fmathsqrt_tab[*fi >> MANT_SHIFTS]) |
    ((e + EXP_BIAS) << EXP_SHIFTS);
    
#if 0
    dbgAssert(f > 0.0f);
    error = ABS((f - test) / test);
    if (error > 0.0001)
    {
        dbgAssert(FALSE);
    }
    dbgAssert(!_isnan(f));
#endif

    return f;

#endif // _MACOSX
}

void fmathInit()
{
    fmathInitSqrt();
}

