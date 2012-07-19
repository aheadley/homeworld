/*=============================================================================
    Name    : twiddle.c
    Purpose : Functions for various bit-twiddling functions

    Created 7/7/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "twiddle.h"

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : bitNumberSet
    Description : Count number of set bits in a number
    Inputs      : target - number to count bits in
                  nBits - number of bits to ckeck
    Outputs     : ..
    Return      : number of set bits in lower nBits of target
----------------------------------------------------------------------------*/
udword bitNumberSet(udword target, udword nBits)
{
    udword index, nSet;

    for (index = nSet = 0; index < nBits; index++, target >>= 1)
    {
        if (target & BIT0)
        {
            nSet++;
        }
    }
    return(nSet);
}

/*-----------------------------------------------------------------------------
    Name        : bitHighExponent2
    Description : Get the nearest exponent of 2 which is >= number
    Inputs      : number - number to find exponent for.
    Outputs     : ..
    Return      : nearest exponent of 2 which is >= number or 0xffffffff if none
----------------------------------------------------------------------------*/
udword bitHighExponent2(udword number)
{
    udword index, mask;

    for (index = 0, mask = 1; index < 31; index++, mask <<= 1)
    {
        if (mask >= number)
        {
            return(mask);
        }
    }
    return(0xffffffff);
}

/*-----------------------------------------------------------------------------
    Name        : bitLowExponent2
    Description : Get the nearest exponent of 2 which is < number
    Inputs      : number - number to find exponent for.
    Outputs     : ..
    Return      : nearest exponent of 2 which is < number
----------------------------------------------------------------------------*/
udword bitLowExponent2(udword number)
{
    udword index, mask;

    for (index = 0, mask = 1; index < 31; index++, mask <<= 1)
    {
        if (mask >= number)
        {
            break;
        }
    }
    return(mask >> 1);
}

/*-----------------------------------------------------------------------------
    Name        : bitLowBitPosition
    Description : Return bit position of lowest set bit in number
    Inputs      : number - number to process
    Outputs     :
    Return      : bit position of lowest set bit in number
----------------------------------------------------------------------------*/
udword bitLowBitPosition(udword number)
{
    sdword index;

    for (index = 0; index < 32; index++)
    {
        if (number & 1)
        {
            break;
        }
        number >>= 1;
    }
    return(index);
}
