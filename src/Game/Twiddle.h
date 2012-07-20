/*=============================================================================
    Name    : twiddle.h
    Purpose : Definitions for bit twiddleing, if that is even a word.

    Created 7/7/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___TWIDDLE_H
#define ___TWIDDLE_H

#include "Types.h"

/*=============================================================================
    Functions:
=============================================================================*/
//count number of set bits in a number
udword bitNumberSet(udword target, udword nBits);

//find nearest exponent of 2
udword bitHighExponent2(udword number);
udword bitLowExponent2(udword number);

//find lowest/highest bit set in a number
udword bitLowBitPosition(udword number);

#endif  /* ___TWIDDLE_H */

