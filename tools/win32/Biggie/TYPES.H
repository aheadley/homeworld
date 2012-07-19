/*=========================================================================
   TYPES.H definitions for Watcom C
==========================================================================*/

#ifndef ___TYPES_H
#define ___TYPES_H

/*-------------------------------------------------------------------------
   Declare size of integers in bytes by compiler type
--------------------------------------------------------------------------*/
#if defined(_WIN32) && defined(_M_IX86)
#else  // not Microsoft C
#error Use of types.h for compiler other than Microsoft C
#endif

/*-----------------------------------------------------------------------------
    Warning turn ons:
-----------------------------------------------------------------------------*/
#pragma warning( 2 : 4013)  //function undefined
#pragma warning( 2 : 4035)  //no return value
#pragma warning( 2 : 4101)  //unreferenced local variable
#pragma warning( 2 : 4245)  //signed/unsigned mismatch
#pragma warning( 2 : 4505)  //spots unused local functions
#pragma warning( 2 : 4706)  //spots '=' instead of '==' in conditional statements

/*-------------------------------------------------------------------------
   Declare basic integer data types
--------------------------------------------------------------------------*/
typedef signed   char       sbyte;
typedef unsigned char       ubyte;
typedef signed   short int  sword;
typedef unsigned short int  uword;
typedef signed   long  int  sdword;
typedef unsigned long  int  udword;
typedef signed   __int64    sqword;
typedef unsigned __int64    uqword;
typedef float               real32;
typedef double              real64;

typedef sdword              bool;
typedef sbyte               bool8;
typedef sword               bool16;

/*-------------------------------------------------------------------------
   Declare common numbers etc.
--------------------------------------------------------------------------*/

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef TRUE
#define TRUE               (!FALSE)
#endif
#ifndef FALSE
#define FALSE              0
#endif
#ifndef OKAY
#define OKAY               0
#endif
#ifndef ERROR
#define ERROR              -1
#endif

#define  SBYTE_Min     -128                     // minimums for signed types
#define  SWORD_Min     -32768L
#define  SDWORD_Min    -2147483648L

#define  SBYTE_Max     127                      // maximums for signed types
#define  SWORD_Max     32767
#define  SDWORD_Max    2147483647L

#define  UBYTE_Max     255                      // maximums for unsigned types
#define  UWORD_Max     65535L
#define  UDWORD_Max    4294967295L

#define  BIT0       0x01
#define  BIT1       0x02
#define  BIT2       0x04
#define  BIT3       0x08
#define  BIT4       0x10
#define  BIT5       0x20
#define  BIT6       0x40
#define  BIT7       0x80
#define  BIT8       0x0100
#define  BIT9       0x0200
#define  BIT10      0x0400
#define  BIT11      0x0800
#define  BIT12      0x1000
#define  BIT13      0x2000
#define  BIT14      0x4000
#define  BIT15      0x8000
#define  BIT16      0x00010000L
#define  BIT17      0x00020000L
#define  BIT18      0x00040000L
#define  BIT19      0x00080000L
#define  BIT20      0x00100000L
#define  BIT21      0x00200000L
#define  BIT22      0x00400000L
#define  BIT23      0x00800000L
#define  BIT24      0x01000000L
#define  BIT25      0x02000000L
#define  BIT26      0x04000000L
#define  BIT27      0x08000000L
#define  BIT28      0x10000000L
#define  BIT29      0x20000000L
#define  BIT30      0x40000000L
#define  BIT31      0x80000000L

/*-------------------------------------------------------------------------
   Declare basic macros
--------------------------------------------------------------------------*/

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) > (b) ? (b) : (a))
#endif
#ifndef abs
#define abs(a)   ((a) < 0 ? -(a) : (a))
#endif

#define frandyrandom(stream,n) (ranRandom(stream) * (((real32)(n)) * (1.0f/((real32)UDWORD_Max))))
#define frandyrandombetween(stream,a,b) (frandyrandom(stream,(b)-(a)) + (a))

#define randyrandom(stream,n) (ranRandom(stream) % (n))
#define randyrandombetween(stream,a,b) ( randyrandom(stream,((b)-(a)+1)) + (a) )

#define isBetweenExclusive(num,lowerbound,upperbound) \
            ( ((num) > (lowerbound)) && ((num) < (upperbound)) )

#define isBetweenInclusive(num,lowerbound,upperbound) \
            ( ((num) >= (lowerbound)) && ((num) <= (upperbound)) )

#define capNumber(value,cap) ((value > cap) ? cap : value)

#ifndef PI
#define PI 3.14159265358979f
#endif
#define TWOPI   (2.0f * PI)

#define DEG_PER_RADIAN (360.0f/(2.0f*PI))
#define RADIAN_PER_DEG (2.0f*PI/360.0f)

#define DEG_TO_RAD(x) ((x) * RADIAN_PER_DEG)
#define RAD_TO_DEG(x) ((x) * DEG_PER_RADIAN)

//standard swap, works on any type
#ifndef swap
#define swap(a,b,temp) \
        (temp) = (a);  \
        (a) = (b);     \
        (b) = (temp)
#endif
//integral swap, including pointers
#define swapInt(a, b)   \
        (a) ^= (b);     \
        (b) ^= (a);     \
        (a) ^= (b);
//floating-point swap
#define swapReal32(a, b)                            \
        *((udword *)(&(a))) ^= *((udword *)(&(b)));   \
        *((udword *)(&(b))) ^= *((udword *)(&(a)));   \
        *((udword *)(&(a))) ^= *((udword *)(&(b)));

#define str$(x) #x

//macros to set/test/clear bits
#define bitSet(dest, bitFlags)      ((dest) |= (bitFlags))
#define bitClear(dest, bitFlags)    ((dest) &= ~(bitFlags))
#define bitTest(dest, bitFlags)     ((dest) & (bitFlags))
#define bitToggle(dest, bitFlags)   if ((dest) & (bitFlags)) bitClear((dest), (bitFlags)); else bitSet((dest), (bitFlags))
//#define bitSetTo(dest, bitFlags, bSet) ((bSet) ? bitSet(dest, bitFlags) : bitClear(dest, bitFlags)
#define bitSetTo(dest, bitFlags, bSet) ((dest) = ((dest) & (~bitFlags)) | ((bSet)?bitFlags:0))

//fuzzy equals using a tolerance
#define roughlyEqual(a, b, t)   (((b) <= ((a) + (t))) && ((b) >= ((a) - (t))))


//big numbers (theoretically the largest representable with reals)
#define REALlyBig           ((real32)10e20)
#define REALlySmall         ((real32)10e-20)
#define REALlyNegative      ((real32)-10e20)

#define TreatAsUdword(x) (*((udword *)(&(x))))
#define TreatAsReal32(x) (*((real32 *)(&(x))))

//byte-order swapping
#define uwordSwapBytes(w)  ((((w) & 0x00ff) << 8) | (((w) & 0xff00) >> 8))
#define udwordSwapBytes(d) ((((d) & 0x000000ff) << 24) | (((d) & 0x0000ff00) << 8) | (((d) & 0x00ff0000) >> 8) | (((d) & 0xff000000) >> 24))

//volume, area computations
#define circleCircumference(radius) (2.0f * PI * (radius))
#define circleArea(radius)          (PI * radius * (radius))
#define sphereArea(radius)          (4.0f * PI * (radius) * (radius))
#define sphereVolume(radius)        (4.0f / 3.0f * PI * (radius) * (radius) * (radius))

#endif  // ___TYPES_H

