/*=============================================================================
    Name    : crc32.h
    Purpose : Compute CRC32 for packets of data

    Created 6/29/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___CRC32_H
#define ___CRC32_H

/*=============================================================================
    Type definitions
=============================================================================*/
typedef unsigned int crc32;

/*=============================================================================
    Functions
=============================================================================*/
crc32 crc32Compute(unsigned char* packet, unsigned int length);
crc32 crc32ComputeString(char* string);

#endif //___CRC32_H
