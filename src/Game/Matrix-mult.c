/*=============================================================================
    Matrix-mult.C: 3x3 matrix multiply by 3x3 matrix (to avoid being inlined in
                   Matrix.c when compiling optimized code in GCC, thus
                   duplicating the line labels).
=============================================================================*/

#include <stdio.h>
#include "Types.h"
#include "Matrix.h"

#define FPTR    DWORD PTR
#define FSIZE   4
#define FSIZE_STR "4"

#if defined (_MSC_VER)
#define SOURCE  esi
#define DEST    ebx
#define MATRIX  edi
#elif defined (__GNUC__) && defined (__i386__)
#define SOURCE  "%%esi"
#define DEST    "%%ebx"
#define MATRIX  "%%edi"
#endif

/*-----------------------------------------------------------------------------
    Name        : matMultiplyMatByMat
    Description : result = first * second, where matrices are 3 X 3
    Inputs      : first,second
    Outputs     : result
    Return      :
    Warning     : result cannot be the same matrix as first or second.
----------------------------------------------------------------------------*/
void matMultiplyMatByMat(matrix *result,matrix *first,matrix *second)
{
#if defined (_MSC_VER)
    static real32* c;
    static real32* a;
    static real32* b;

    c = (real32*)result;
    a = (real32*)first;
    b = (real32*)second;

    __asm
    {
        push      edi
        push      esi
        push      ebp
        push      ebx
        mov       ebx, FPTR [c]
        mov       ecx, FPTR [a]
        mov       edx, FPTR [b]
        mov       eax, -3
        jmp       l1
        align     4
    l2:
        fstp      FPTR [ebx+eax*FSIZE+32]
    l1:
        fld       FPTR [ecx+eax*FSIZE+36]
        fmul      FPTR [edx+8]
        fld       FPTR [edx]
        fmul      FPTR [ecx+eax*FSIZE+12]
        faddp     st(1), st
        fld       FPTR [edx+4]
        fmul      FPTR [ecx+eax*FSIZE+24]
        faddp     st(1), st
        mov       ebp, FPTR [ecx+eax*FSIZE+12]
        mov       esi, FPTR [ecx+eax*FSIZE+24]
        fld       FPTR [ecx+eax*FSIZE+24]
        fld       FPTR [ecx+eax*FSIZE+24]
        fld       FPTR [ecx+eax*FSIZE+12]
        fld       FPTR [ecx+eax*FSIZE+12]
        fld       FPTR [ecx+eax*FSIZE+36]
        fld       FPTR [ecx+eax*FSIZE+36]
        fxch      st(6)
        mov       edi, FPTR [ecx+eax*FSIZE+36]
        fstp      FPTR [ebx+eax*FSIZE+12]
        fmul      FPTR [edx+20]
        fxch      st(4)
        fmul      FPTR [edx+16]
        faddp     st(4), st
        fxch      st(1)
        fmul      FPTR [edx+12]
        faddp     st(3), st
        fxch      st(2)
        fstp      FPTR [ebx+eax*FSIZE+24]
        fmul      FPTR [edx+28]
        fxch      st(2)
        fmul      FPTR [edx+32]
        faddp     st(2), st
        fmul      FPTR [edx+24]
        faddp     st(1), st
        inc       eax
        jne       l2
        fstp      FPTR [ebx+eax*FSIZE+32]

        pop       ebx
        pop       ebp
        pop       esi
        pop       edi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ (
        "    pushl     %%ebp\n"
        "    movl      $-3, %%eax\n"
        "    jmp       mat_x_mat_l1\n"
        "    .align    4\n"
        "mat_x_mat_l2:\n"
        "    fstps     32(%%ebx, %%eax, "FSIZE_STR")\n"
        "mat_x_mat_l1:\n"
        "    flds      36(%%ecx, %%eax, "FSIZE_STR")\n"
        "    fmuls     8(%%edx)\n"
        "    flds      (%%edx)\n"
        "    fmuls     12(%%ecx, %%eax, "FSIZE_STR")\n"
        "    faddp     %%st, %%st(1)\n"
        "    flds      4(%%edx)\n"
        "    fmuls     24(%%ecx, %%eax, "FSIZE_STR")\n"
        "    faddp     %%st, %%st(1)\n"
        "    movl      12(%%ecx, %%eax, "FSIZE_STR"), %%ebp\n"
        "    movl      24(%%ecx, %%eax, "FSIZE_STR"), %%esi\n"
        "    flds      24(%%ecx, %%eax, "FSIZE_STR")\n"
        "    flds      24(%%ecx, %%eax, "FSIZE_STR")\n"
        "    flds      12(%%ecx, %%eax, "FSIZE_STR")\n"
        "    flds      12(%%ecx, %%eax, "FSIZE_STR")\n"
        "    flds      36(%%ecx, %%eax, "FSIZE_STR")\n"
        "    flds      36(%%ecx, %%eax, "FSIZE_STR")\n"
        "    fxch      %%st(6)\n"
        "    movl      36(%%ecx, %%eax, "FSIZE_STR"), %%edi\n"
        "    fstps     12(%%ebx, %%eax, "FSIZE_STR")\n"
        "    fmuls     20(%%edx)\n"
        "    fxch      %%st(4)\n"
        "    fmuls     16(%%edx)\n"
        "    faddp     %%st, %%st(4)\n"
        "    fxch      %%st(1)\n"
        "    fmuls     12(%%edx)\n"
        "    faddp     %%st, %%st(3)\n"
        "    fxch      %%st(2)\n"
        "    fstps     24(%%ebx, %%eax, "FSIZE_STR")\n"
        "    fmuls     28(%%edx)\n"
        "    fxch      %%st(2)\n"
        "    fmuls     32(%%edx)\n"
        "    faddp     %%st, %%st(2)\n"
        "    fmuls     24(%%edx)\n"
        "    faddp     %%st, %%st(1)\n"
        "    incl      %%eax\n"
        "    jne       mat_x_mat_l2\n"
        "    fstps     32(%%ebx, %%eax, "FSIZE_STR")\n"
        "    popl      %%ebp\n"
        :
        : "b" (result), "c" (first), "d" (second)
        : "eax", "edi", "esi" );
#else
#define A(row,col) a[3*col+row]
#define B(row,col) b[3*col+row]
#define P(row,col) c[3*col+row]
    real32 *c = (real32*)result;
    real32 *a = (real32*)first;
    real32 *b = (real32*)second;
    real32 ai0, ai1, ai2;
    sdword i;
    for (i = 0; i < 3; i++)
    {
        ai0 = A(i,0);
        ai1 = A(i,1);
        ai2 = A(i,2);
        P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0);
        P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1);
        P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2);
    }
#undef A
#undef B
#undef P
#endif
}
