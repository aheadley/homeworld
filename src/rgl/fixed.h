#ifndef _FIXED_H
#define _FIXED_H

#define FLOAT2INT(i,f) \
    _asm fld dword ptr f \
    _asm fistp dword ptr i

extern double chop_temp;
#if 0

#define FAST_TO_INT(X) \
    ((chop_temp = (double)(X) + BIG_NUM), *(int*)(&chop_temp))
#define BIG_NUM ((float)(1 << 26)*(1 << 26)*1.5)

#else

#define FAST_TO_INT(X) ((int)(X))

#endif

#define FAST_TO_FIXED(X) \
    FAST_TO_INT((double)(X) * 4096.0)

#define FAST_CEIL(X) (FixedToInt(FixedCeil(FAST_TO_FIXED(X))))
//#define FAST_CEIL(X) ((int)ceil((double)(X)))

#define FAST_TO_FIXED16(X) \
    FAST_TO_INT((double)(X) * 65536.0)

#define FIXED_SCALE 4096.0f

#define FIXED_ONE  0x00001000
#define FIXED_HALF 0x00000800
#define FIXED_FRAC_MASK 0x00000FFF
#define FIXED_INT_MASK  (~FIXED_FRAC_MASK)
#define FIXED_EPSILON   1
#define FIXED_SHIFT     12
#define FixedCeil(X)    (((X) + FIXED_ONE - FIXED_EPSILON) & FIXED_INT_MASK)
#define FixedFloor(X)   ((X) & FIXED_INT_MASK)
#define FixedToUns(F)   ((unsigned)(F) >> 12u)

#define FloatToFixed(X) ((int)((X) * 4096.0f))
#define SignedFloatToFixed(X) FloatToFixed(X)
#define FixedToFloat(F) ((float)(F) / 4096.0f)
#define FixedToDepth(X) X

#define FloatToFixed24(X)   ((int)((X) * 65536.0f))
#define FixedToFloat24(F)   ((float)(F) / 65536.0f)
#define FixedToInt24(F)     ((int)(F) >> 16)
#define FIXED_SHIFT24   16

#define IntToFixed(I)   ((I) << FIXED_SHIFT)
#define FixedToInt(F)   ((F) >> FIXED_SHIFT)
#define FloatToDepth(F) ((int)((F) * DEPTH_SCALE))

#endif
