/*=============================================================================
    Name    : asm.c
    Purpose : assembly language versions of some rGL functions

    Created 7/13/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define _MM_FUNCTIONALITY
#include <xmmintrin.h>
#include <string.h>
#include "asm.h"
#include "_stdint.h"

#define XMM_VB_SIZE (((VB_SIZE + 3) & (~3)) / 4)
#if defined (_MSC_VER)
#define SELECT(i0,i1,i2,i3) (i0*64 + i1*16 + i2*4 + i3)
#elif defined (__GNUC__) && defined (__i386__)
    #define SELECT(i0,i1,i2,i3) "$(" #i0 "*64 + " #i1 "*16 + " #i2 "*4 + " #i3 ")"
#endif

#ifdef _MSC_VER

_MM_ALIGN16 struct sInVerts
{
    __m128 x[XMM_VB_SIZE];
    __m128 y[XMM_VB_SIZE];
    __m128 z[XMM_VB_SIZE];
} sInputVerts;

_MM_ALIGN16 struct sOutVerts
{
    __m128 x[XMM_VB_SIZE];
    __m128 y[XMM_VB_SIZE];
    __m128 z[XMM_VB_SIZE];
    __m128 w[XMM_VB_SIZE];
} sOutputVerts;

#else

struct sInVerts
{
    __m128 x[XMM_VB_SIZE];
    __m128 y[XMM_VB_SIZE];
    __m128 z[XMM_VB_SIZE];
} sInputVerts __attribute__ ((aligned (16)));

struct sOutVerts
{
    __m128 x[XMM_VB_SIZE];
    __m128 y[XMM_VB_SIZE];
    __m128 z[XMM_VB_SIZE];
    __m128 w[XMM_VB_SIZE];
} sOutputVerts __attribute__ ((aligned (16)));

#endif

void gl_intrin_project(GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count);
void gl_xmm_project(GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count);

#define FSIZE      4
#define FSIZE_STR "4"
#if defined (_MSC_VER)
    #define FPTR    dword ptr

#define S(x)   [esi + FSIZE*x]
#define D(x)   [edi + FSIZE*x]
#define M(n)   [edx + FSIZE*n]

#define SOURCE  esi
#define DEST    ebx
#define MATRIX  edi
#elif defined (__GNUC__) && defined (__i386__)
    #define FPTR

    #define S(x)    "4*" #x "(%%esi)"
    #define D(x)    "4*" #x "(%%edi)"
    #define M(n)    "4*" #n "(%%edx)"

    // Remember to change the source registers in
    // gl_wicked_fast_normal_xform() and gl_fairly_fast_scaled_normal_xform()
    // if you change these.
    #define SOURCE  "%%esi"
    #define DEST    "%%edx"
    #define MATRIX  "%%edi"
#endif

#define MAGN_X(i) 	(~(((i) & 1) - 1))
#define SIGN_X(i) 	(~((((i) >> 1) & 1) - 1))
#define MAGN_Y(i) 	(~((((i) >> 2) & 1) - 1))
#define SIGN_Y(i) 	(~((((i) >> 3) & 1) - 1))
#define MAGN_Z(i) 	(~((((i) >> 4) & 1) - 1))
#define SIGN_Z(i) 	(~((((i) >> 5) & 1) - 1))
#define SIGN_W(i) 	(~((((i) >> 6) & 1) - 1))

#define CLIP_VALUE(i) 						\
	 (CLIP_RIGHT_BIT 					\
	  & ((~SIGN_X(i) & SIGN_W(i)) 				\
	     | (~SIGN_X(i) & ~SIGN_W(i) & MAGN_X(i)) 		\
	     | (SIGN_X(i) & SIGN_W(i) & ~MAGN_X(i)))) 		\
	 | (CLIP_LEFT_BIT 					\
	    & ((SIGN_X(i) & SIGN_W(i)) 				\
	       | (~SIGN_X(i) & SIGN_W(i) & ~MAGN_X(i)) 		\
	       | (SIGN_X(i) & ~SIGN_W(i) & MAGN_X(i)))) 	\
	 | (CLIP_TOP_BIT 					\
	    & ((~SIGN_Y(i) & SIGN_W(i)) 			\
	       | (~SIGN_Y(i) & ~SIGN_W(i) & MAGN_Y(i)) 		\
	       | (SIGN_Y(i) & SIGN_W(i) & ~MAGN_Y(i)))) 	\
	 | (CLIP_BOTTOM_BIT 					\
	    & ((SIGN_Y(i) & SIGN_W(i)) 				\
	       | (~SIGN_Y(i) & SIGN_W(i) & ~MAGN_Y(i)) 		\
	       | (SIGN_Y(i) & ~SIGN_W(i) & MAGN_Y(i)))) 	\
	 | (CLIP_FAR_BIT 					\
	    & ((~SIGN_Z(i) & SIGN_W(i)) 			\
	       | (~SIGN_Z(i) & ~SIGN_W(i) & MAGN_Z(i)) 		\
	       | (SIGN_Z(i) & SIGN_W(i) & ~MAGN_Z(i)))) 	\
	 | (CLIP_NEAR_BIT 					\
	    & ((SIGN_Z(i) & SIGN_W(i)) 				\
	       | (~SIGN_Z(i) & SIGN_W(i) & ~MAGN_Z(i)) 		\
	       | (SIGN_Z(i) & ~SIGN_W(i) & MAGN_Z(i))))

#define CLIP_VALUE8(i) \
	CLIP_VALUE(i + 0), CLIP_VALUE(i + 1), CLIP_VALUE(i + 2), CLIP_VALUE(i + 3), \
	CLIP_VALUE(i + 4), CLIP_VALUE(i + 5), CLIP_VALUE(i + 6), CLIP_VALUE(i + 7)

static GLubyte clip_table[8*16] =
{
    CLIP_VALUE8(0x00),
    CLIP_VALUE8(0x08),
    CLIP_VALUE8(0x10),
    CLIP_VALUE8(0x18),
    CLIP_VALUE8(0x20),
    CLIP_VALUE8(0x28),
    CLIP_VALUE8(0x30),
    CLIP_VALUE8(0x38),
    CLIP_VALUE8(0x40),
    CLIP_VALUE8(0x48),
    CLIP_VALUE8(0x50),
    CLIP_VALUE8(0x58),
    CLIP_VALUE8(0x60),
    CLIP_VALUE8(0x68),
    CLIP_VALUE8(0x70),
    CLIP_VALUE8(0x78)
};

static int chkcpubit()
{
    int rval;
#if defined (_MSC_VER)
    _asm
    {
        pusha

        pushfd
        pop eax
        test eax, 0x00200000
        jz set_21
        and eax, 0xffdfffff
        push eax
        popfd
        pushfd
        pop eax
        test eax, 0x00200000
        jz cpu_id_ok
        jmp err

    set_21:
        or eax, 0x00200000
        push eax
        popfd
        pushfd
        pop eax
        test eax, 0x00200000
        jnz cpu_id_ok

    err:
        mov eax, 0
        jmp exit

    cpu_id_ok:
        mov eax, 1

    exit:
        mov [rval], eax

        popa
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    pushfl\n"
        "    popl %%eax\n"
        "    testl $0x00200000, %%eax\n"
        "    jz chkcpubit_set_21\n"
        "    andl $0xffdfffff, %%eax\n"
        "    pushl %%eax\n"
        "    popfl\n"
        "    pushfl\n"
        "    popl %%eax\n"
        "    testl $0x00200000, %%eax\n"
        "    jz chkcpubit_cpu_id_ok\n"
        "    jmp chkcpubit_err\n"

        "chkcpubit_set_21:\n"
        "    orl $0x00200000, %%eax\n"
        "    pushl %%eax\n"
        "    popfl\n"
        "    pushfl\n"
        "    popl %%eax\n"
        "    testl $0x00200000, %%eax\n"
        "    jnz chkcpubit_cpu_id_ok\n"

        "chkcpubit_err:\n"
        "    movl $0, %%eax\n"
        "    jmp chkcpubit_exit\n"

        "chkcpubit_cpu_id_ok:\n"
        "    movl $1, %%eax\n"

        "chkcpubit_exit:\n"
        : "=a" (rval) );
#else
    #error chkcpubit(): Not implemented on this platform.
    rval = 0;
#endif
    return rval;
}

static GLuint cpu_eax, cpu_edx;

GLuint get_cputype()
{
    GLuint ptype;

    if (!chkcpubit())
    {
        cpu_edx = cpu_eax = 0;
        return 4;
    }
    else
    {
#if defined (_MSC_VER)
        _asm
        {
            pusha

            mov eax, 1
//            cpuid
            _emit 0x0f
            _emit 0xa2
            mov [cpu_eax], eax
            mov [cpu_edx], edx
            and eax, 0x0f00
            shr eax, 8
            mov [ptype], eax

            popa
        }
        return ptype;
#elif defined (__GNUC__) && defined (__i386__)
        __asm__ __volatile__ (
            "    pushl %%ebx\n"
            "    movl $1, %%eax\n"
            "    cpuid\n"
            "    popl %%ebx\n"
            : "=a" (cpu_eax), "=d" (cpu_edx)
            :
            : "ecx" );

        return (cpu_eax & 0x0f00) >> 8;
#endif
    }
}

//only call after get_cputype()
GLboolean get_cpummx()
{
    //check MMX bit (23)
    if (cpu_edx & 0x400000)
    {
        return GL_TRUE;
    }
    else
    {
        if (((cpu_eax & 0x0f00) >= 0x600) &&
            ((cpu_eax & 0x0f0) >= 0x30))
        {
            //pentium II has MMX
            return GL_TRUE;
        }
        //no MMX
        return GL_FALSE;
    }
}

//only call after get_cputype()
GLboolean get_cpukatmai()
{
    if ((cpu_edx & 0x2000000) &&    //XMM
        (cpu_edx & 0x1000000))      //FXSR
    {
        return GL_TRUE;
    }
    else
    {
        return GL_FALSE;
    }
}

void transform_points4_general(GLuint n, GLfloat* d, GLfloat* m, GLfloat* s)
{
#if defined (_MSC_VER)
    _asm
    {
        push esi
        push edi

        mov ecx, [n]
        mov edi, [d]
        mov edx, [m]
        mov esi, [s]

        test ecx, ecx
        jz two

        align 16

      one:
        fld FPTR S(0)
        fmul FPTR M(0)
        fld FPTR S(0)
        fmul FPTR M(1)
        fld FPTR S(0)
        fmul FPTR M(2)
        fld FPTR S(0)
        fmul FPTR M(3)

        fld FPTR S(1)
        fmul FPTR M(4)
        fld FPTR S(1)
        fmul FPTR M(5)
        fld FPTR S(1)
        fmul FPTR M(6)
        fld FPTR S(1)
        fmul FPTR M(7)

        fxch st(3)
        faddp st(7), st
        fxch st(1)
        faddp st(5), st
        faddp st(3), st
        faddp st(1), st

        fld FPTR S(2)
        fmul FPTR M(8)
        fld FPTR S(2)
        fmul FPTR M(9)
        fld FPTR S(2)
        fmul FPTR M(10)
        fld FPTR S(2)
        fmul FPTR M(11)

        fxch st(3)
        faddp st(7), st
        fxch st(1)
        faddp st(5), st
        faddp st(3), st
        faddp st(1), st

        fld FPTR S(3)
        fmul FPTR M(12)
        fld FPTR S(3)
        fmul FPTR M(13)
        fld FPTR S(3)
        fmul FPTR M(14)
        fld FPTR S(3)
        fmul FPTR M(15)

        fxch st(3)
        faddp st(7), st
        fxch st(1)
        faddp st(5), st
        faddp st(3), st

        lea esi, FPTR S(4)
        dec ecx

        faddp st(1), st

        fxch st(3)
        fstp FPTR D(0)
        fxch st(1)
        fstp FPTR D(1)
        fstp FPTR D(2)
        fstp FPTR D(3)

        lea edi, FPTR D(4)

        jnz one

      two:
        pop edi
        pop esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    testl %%ecx, %%ecx\n"
        "    jz trans4gen_two\n"

        "    .align 16\n"

        "trans4gen_one:\n"
        "    flds " S(0) "\n"
        "    fmuls " M(0) "\n"
        "    flds " S(0) "\n"
        "    fmuls " M(1) "\n"
        "    flds " S(0) "\n"
        "    fmuls " M(2) "\n"
        "    flds " S(0) "\n"
        "    fmuls " M(3) "\n"

        "    flds " S(1) "\n"
        "    fmuls " M(4) "\n"
        "    flds " S(1) "\n"
        "    fmuls " M(5) "\n"
        "    flds " S(1) "\n"
        "    fmuls " M(6) "\n"
        "    flds " S(1) "\n"
        "    fmuls " M(7) "\n"

        "    fxch %%st(3)\n"
        "    faddp %%st, %%st(7)\n"
        "    fxch %%st(1)\n"
        "    faddp %%st, %%st(5)\n"
        "    faddp %%st, %%st(3)\n"
        "    faddp %%st, %%st(1)\n"

        "    flds " S(2) "\n"
        "    fmuls " M(8) "\n"
        "    flds " S(2) "\n"
        "    fmuls " M(9) "\n"
        "    flds " S(2) "\n"
        "    fmuls " M(10) "\n"
        "    flds " S(2) "\n"
        "    fmuls " M(11) "\n"

        "    fxch %%st(3)\n"
        "    faddp %%st, %%st(7)\n"
        "    fxch %%st(1)\n"
        "    faddp %%st, %%st(5)\n"
        "    faddp %%st, %%st(3)\n"
        "    faddp %%st, %%st(1)\n"

        "    flds " S(3) "\n"
        "    fmuls " M(12) "\n"
        "    flds " S(3) "\n"
        "    fmuls " M(13) "\n"
        "    flds " S(3) "\n"
        "    fmuls " M(14) "\n"
        "    flds " S(3) "\n"
        "    fmuls " M(15) "\n"

        "    fxch %%st(3)\n"
        "    faddp %%st, %%st(7)\n"
        "    fxch %%st(1)\n"
        "    faddp %%st, %%st(5)\n"
        "    faddp %%st, %%st(3)\n"

        "    leal " S(4) ", %%esi\n"
        "    decl %%ecx\n"

        "    faddp %%st, %%st(1)\n"

        "    fxch %%st(3)\n"
        "    fstps " D(0) "\n"
        "    fxch %%st(1)\n"
        "    fstps " D(1) "\n"
        "    fstps " D(2) "\n"
        "    fstps " D(3) "\n"

        "    leal " D(4) ", %%edi\n"

        "    jnz trans4gen_one\n"

        "trans4gen_two:\n"
        :
        : "c" (n), "D" (d), "d" (m), "S" (s) );
#endif
}

void transform_points4_identity(GLuint n, GLfloat* d, GLfloat* m, GLfloat* s)
{
    MEMCPY(d, s, n*4*sizeof(GLfloat));
}

void transform_points4_perspective(GLuint n, GLfloat* d, GLfloat* m, GLfloat* s)
{
#if defined (_MSC_VER)
    _asm
    {
        push esi
        push edi

        mov ecx, [n]
        mov edi, [d]
        mov edx, [m]
        mov esi, [s]

        test ecx, ecx
        jz two

        align 16

      one:
        fld FPTR S(0)
        fmul FPTR M(0)
        fld FPTR S(1)
        fmul FPTR M(5)
        fld FPTR S(2)
        fmul FPTR M(10)

        fld FPTR S(2)
        fmul FPTR M(8)
        fld FPTR S(2)
        fmul FPTR M(9)
        fld FPTR S(3)
        fmul FPTR M(14)

        mov eax, FPTR S(2)
        lea esi, FPTR S(4)
        xor eax, 0x80000000
        dec ecx

        faddp st(3),st
        faddp st(3),st
        faddp st(3),st

        fstp FPTR D(2)
        fstp FPTR D(1)
        fstp FPTR D(0)

        mov FPTR D(3), eax
        lea edi, FPTR D(4)
        jnz one

      two:
        pop edi
        pop esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    testl %%ecx, %%ecx\n"
        "    jz trans4persp_two\n"

        "    .align 16\n"

        "trans4persp_one:\n"
        "    flds " S(0) "\n"
        "    fmuls " M(0) "\n"
        "    flds " S(1) "\n"
        "    fmuls " M(5) "\n"
        "    flds " S(2) "\n"
        "    fmuls " M(10) "\n"

        "    flds " S(2) "\n"
        "    fmuls " M(8) "\n"
        "    flds " S(2) "\n"
        "    fmuls " M(9) "\n"
        "    flds " S(3) "\n"
        "    fmuls " M(14) "\n"

        "    movl " S(2) ", %%eax\n"
        "    leal " S(4) ", %%esi\n"
        "    xorl $0x80000000, %%eax\n"
        "    decl %%ecx\n"

        "    faddp %%st, %%st(3)\n"
        "    faddp %%st, %%st(3)\n"
        "    faddp %%st, %%st(3)\n"

        "    fstps " D(2) "\n"
        "    fstps " D(1) "\n"
        "    fstps " D(0) "\n"

        "    movl %%eax, " D(3) "\n"
        "    leal " D(4) ", %%edi\n"
        "    jnz trans4persp_one\n"

        "trans4persp_two:\n"
        :
        : "c" (n), "D" (d), "d" (m), "S" (s)
        : "eax" );
#endif
}

void asm_cliptest(
    GLuint n, GLfloat* d, GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask)
{
#if defined (_MSC_VER)
    _asm
    {
        push esi
        push edi

        mov edi, [ormask]
        mov esi, [andmask]
        mov al, byte ptr [edi]
        mov ah, byte ptr [esi]

        mov ecx, [n]
        mov edi, [clipmask]
        mov esi, [d]

        //cliptest
        test ecx, ecx
        jz two

        push ebp
        push ebx

        align 16

      one:
        mov ebp, FPTR S(3)
        mov ebx, FPTR S(2)

        xor edx, edx
        add ebp, ebp

        adc edx, edx
        add ebx, ebx

        adc edx, edx
        cmp ebp, ebx

        adc edx, edx
        mov ebx, FPTR S(1)

        add ebx, ebx

        adc edx, edx
        cmp ebp, ebx

        adc edx, edx
        mov ebx, FPTR S(0)

        add ebx, ebx

        adc edx, edx
        cmp ebp, ebx

        adc edx, edx

        lea esi, FPTR S(4)
        mov bl, byte ptr [edi]
        mov dl, byte ptr [clip_table+edx]

        or bl, dl
        or al, dl

        and ah, dl
        mov byte ptr [edi], bl

        inc edi
        dec ecx

        jnz one

        pop ebx
        pop ebp

      two:
        //cliptest

        mov edi, [ormask]
        mov esi, [andmask]
        mov byte ptr [edi], al
        mov byte ptr [esi], ah

        pop edi
        pop esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    movl %0, %%edi\n"
        "    movl %1, %%esi\n"
        "    movb (%%edi), %%al\n"
        "    movb (%%esi), %%ah\n"

        "    movl %2, %%ecx\n"
        "    movl %3, %%edi\n"
        "    movl %4, %%esi\n"

#ifdef PIC  /* Needed since we use ebx for calculations. */
        "    movl %5, %%edx\n"
#endif

        //cliptest
        "    testl %%ecx, %%ecx\n"
        "    jz cliptest_two\n"

        "    pushl %%ebp\n"
        "    pushl %%ebx\n"
#ifdef PIC  /* Keep track of clip_table. */
        "    pushl %%edx\n"
#endif

        "    .align 16\n"

        "cliptest_one:\n"
        "    movl " S(3) ", %%ebp\n"
        "    movl " S(2) ", %%ebx\n"

        "    xorl %%edx, %%edx\n"
        "    addl %%ebp, %%ebp\n"

        "    adcl %%edx, %%edx\n"
        "    addl %%ebx, %%ebx\n"

        "    adcl %%edx, %%edx\n"
        "    cmpl %%ebx, %%ebp\n"

        "    adcl %%edx, %%edx\n"
        "    movl " S(1) ", %%ebx\n"

        "    addl %%ebx, %%ebx\n"

        "    adcl %%edx, %%edx\n"
        "    cmpl %%ebx, %%ebp\n"

        "    adcl %%edx, %%edx\n"
        "    movl " S(0) ", %%ebx\n"

        "    addl %%ebx, %%ebx\n"

        "    adcl %%edx, %%edx\n"
        "    cmpl %%ebx, %%ebp\n"

        "    adcl %%edx, %%edx\n"

        "    leal " S(4) ", %%esi\n"
        "    movb (%%edi), %%bl\n"
#ifdef PIC
        "    movl (%%esp), %%ebp\n"
        "    movb (%%edx, %%ebp), %%dl\n"
#else
        "    movb %5(%%edx), %%dl\n"
#endif

        "    orb %%dl, %%bl\n"
        "    orb %%dl, %%al\n"

        "    andb %%dl, %%ah\n"
        "    movb %%bl, (%%edi)\n"

        "    incl %%edi\n"
        "    decl %%ecx\n"

        "    jnz cliptest_one\n"

#ifdef PIC
        "    popl %%edx\n"
#endif
        "    popl %%ebx\n"
        "    popl %%ebp\n"

        "cliptest_two:\n"
        //cliptest

        "    movl %0, %%edi\n"
        "    movl %1, %%esi\n"
        "    movb %%al, (%%edi)\n"
        "    movb %%ah, (%%esi)\n"
        :
        : "m" (ormask), "m" (andmask), "m" (n), "m" (clipmask), "m" (d),
          "m" (clip_table)
        : "eax", "ecx", "edx", "edi", "esi" );
#endif
}

void asm_project_and_cliptest_general(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask)
{
    transform_points4_general(n, d, m, s);
    asm_cliptest(n, d, clipmask, ormask, andmask);
}

void asm_project_and_cliptest_identity(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask)
{
    transform_points4_identity(n, d, m, s);
    asm_cliptest(n, d, clipmask, ormask, andmask);
}

void asm_project_and_cliptest_perspective(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask)
{
    transform_points4_perspective(n, d, m, s);
    asm_cliptest(n, d, clipmask, ormask, andmask);
}

typedef struct
{
    float x, y, z, w;
} hvector;

#if defined (_MSC_VER)
static _MM_ALIGN16 __m128 mat[16], per[16];
#else
static __m128 mat[16] __attribute__ ((aligned (16))),
              per[16] __attribute__ ((aligned (16)));
#endif

void intrin_project_and_cliptest_perspective(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask)
{
    gl_intrin_project(d, s, m, n);
    asm_cliptest(n, d, clipmask, ormask, andmask);
}

void xmm_project_and_cliptest_perspective(
    GLuint n, GLfloat* d, GLfloat* m, GLfloat* s,
    GLubyte* clipmask, GLubyte* ormask, GLubyte* andmask)
{
    gl_xmm_project(d, s, m, n);
    asm_cliptest(n, d, clipmask, ormask, andmask);
}

void xmm_set_modelview(GLfloat* m)
{
    static float* m0;
    static float* mat0;

    m0 = m;
    mat0 = (float*)mat;

#if defined (_MSC_VER)
    _asm
    {
        push    esi
        push    edi

        mov     esi, [m0]
        mov     edi, [mat0]

        movss   xmm0, DWORD PTR [esi + 4*0]
        movss   xmm1, DWORD PTR [esi + 4*1]
        shufps  xmm0, xmm0, 00h
        movss   xmm2, DWORD PTR [esi + 4*2]
        shufps  xmm1, xmm1, 00h
        movaps  XMMWORD PTR [edi + 4*4*0], xmm0
        movss   xmm3, DWORD PTR [esi + 4*4]
        shufps  xmm2, xmm2, 00h
        movaps  XMMWORD PTR [edi + 4*4*1], xmm1
        movss   xmm4, DWORD PTR [esi + 4*5]
        shufps  xmm3, xmm3, 00h
        movaps  XMMWORD PTR [edi + 4*4*2], xmm2
        movss   xmm5, DWORD PTR [esi + 4*6]
        shufps  xmm4, xmm4, 00h
        movaps  XMMWORD PTR [edi + 4*4*4], xmm3
        movss   xmm6, DWORD PTR [esi + 4*8]
        shufps  xmm5, xmm5, 00h
        movaps  XMMWORD PTR [edi + 4*4*5], xmm4
        movss   xmm7, DWORD PTR [esi + 4*9]
        shufps  xmm6, xmm6, 00h
        movaps  XMMWORD PTR [edi + 4*4*6], xmm5
        shufps  xmm7, xmm7, 00h
        movaps  XMMWORD PTR [edi + 4*4*8], xmm6
        movaps  XMMWORD PTR [edi + 4*4*9], xmm7

        movss   xmm0, DWORD PTR [esi + 4*10]
        movss   xmm1, DWORD PTR [esi + 4*12]
        shufps  xmm0, xmm0, 00h
        movss   xmm2, DWORD PTR [esi + 4*13]
        shufps  xmm1, xmm1, 00h
        movaps  XMMWORD PTR [edi + 4*4*10], xmm0
        movss   xmm3, DWORD PTR [esi + 4*14]
        shufps  xmm2, xmm2, 00h
        movaps  XMMWORD PTR [edi + 4*4*12], xmm1
        shufps  xmm3, xmm3, 00h
        movaps  XMMWORD PTR [edi + 4*4*13], xmm2
        movaps  XMMWORD PTR [edi + 4*4*14], xmm3

        pop     edi
        pop     esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    movss   4*0(%%esi), %%xmm0\n"
        "    movss   4*1(%%esi), %%xmm1\n"
        "    shufps  $0x00, %%xmm0, %%xmm0\n"
        "    movss   4*2(%%esi), %%xmm2\n"
        "    shufps  $0x00, %%xmm1, %%xmm1\n"
        "    movaps  %%xmm0, 4*4*0(%%edi)\n"
        "    movss   4*4(%%esi), %%xmm3\n"
        "    shufps  $0x00, %%xmm2, %%xmm2\n"
        "    movaps  %%xmm1, 4*4*1(%%edi)\n"
        "    movss   4*5(%%esi), %%xmm4\n"
        "    shufps  $0x00, %%xmm3, %%xmm3\n"
        "    movaps  %%xmm2, 4*4*2(%%edi)\n"
        "    movss   4*6(%%esi), %%xmm5\n"
        "    shufps  $0x00, %%xmm4, %%xmm4\n"
        "    movaps  %%xmm3, 4*4*4(%%edi)\n"
        "    movss   4*8(%%esi), %%xmm6\n"
        "    shufps  $0x00, %%xmm5, %%xmm5\n"
        "    movaps  %%xmm4, 4*4*5(%%edi)\n"
        "    movss   4*9(%%esi), %%xmm7\n"
        "    shufps  $0x00, %%xmm6, %%xmm6\n"
        "    movaps  %%xmm5, 4*4*6(%%edi)\n"
        "    shufps  $0x00, %%xmm7, %%xmm7\n"
        "    movaps  %%xmm6, 4*4*8(%%edi)\n"
        "    movaps  %%xmm7, 4*4*9(%%edi)\n"

        "    movss   4*10(%%esi), %%xmm0\n"
        "    movss   4*12(%%esi), %%xmm1\n"
        "    shufps  $0x00, %%xmm0, %%xmm0\n"
        "    movss   4*13(%%esi), %%xmm2\n"
        "    shufps  $0x00, %%xmm1, %%xmm1\n"
        "    movaps  %%xmm0, 4*4*10(%%edi)\n"
        "    movss   4*14(%%esi), %%xmm3\n"
        "    shufps  $0x00, %%xmm2, %%xmm2\n"
        "    movaps  %%xmm1, 4*4*12(%%edi)\n"
        "    shufps  $0x00, %%xmm3, %%xmm3\n"
        "    movaps  %%xmm2, 4*4*13(%%edi)\n"
        "    movaps  %%xmm3, 4*4*14(%%edi)\n"
        :
        : "S" (m0), "D" (mat0) );
#endif
}

void xmm_set_projection(GLfloat* m)
{
    static float* m1;
    static float* mat1;

    m1 = m;
    mat1 = (float*)per;

#if defined (_MSC_VER)
    _asm
    {
        push    eax
        push    edx

        mov     eax, [m1]
        mov     edx, [mat1]

        movss   xmm0, DWORD PTR [eax + 4*0]
        movss   xmm1, DWORD PTR [eax + 4*5]
        shufps  xmm0, xmm0, 00h
        movss   xmm2, DWORD PTR [eax + 4*8]
        shufps  xmm1, xmm1, 00h
        movaps  XMMWORD PTR [edx + 4*4*0], xmm0
        movss   xmm3, DWORD PTR [eax + 4*9]
        shufps  xmm2, xmm2, 00h
        movaps  XMMWORD PTR [edx + 4*4*5], xmm1
        movss   xmm4, DWORD PTR [eax + 4*10]
        shufps  xmm3, xmm3, 00h
        movaps  XMMWORD PTR [edx + 4*4*8], xmm2
        movss   xmm5, DWORD PTR [eax + 4*14]
        shufps  xmm4, xmm4, 00h
        movaps  XMMWORD PTR [edx + 4*4*9], xmm3
        shufps  xmm5, xmm5, 00h
        movaps  XMMWORD PTR [edx + 4*4*10], xmm4
        movaps  XMMWORD PTR [edx + 4*4*14], xmm5

        pop     edx
        pop     eax
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    movss   4*0(%%eax), %%xmm0\n"
        "    movss   4*5(%%eax), %%xmm1\n"
        "    shufps  $0x00, %%xmm0, %%xmm0\n"
        "    movss   4*8(%%eax), %%xmm2\n"
        "    shufps  $0x00, %%xmm1, %%xmm1\n"
        "    movaps  %%xmm0, 4*4*0(%%edx)\n"
        "    movss   4*9(%%eax), %%xmm3\n"
        "    shufps  $0x00, %%xmm2, %%xmm2\n"
        "    movaps  %%xmm1, 4*4*5(%%edx)\n"
        "    movss   4*10(%%eax), %%xmm4\n"
        "    shufps  $0x00, %%xmm3, %%xmm3\n"
        "    movaps  %%xmm2, 4*4*8(%%edx)\n"
        "    movss   4*14(%%eax), %%xmm5\n"
        "    shufps  $0x00, %%xmm4, %%xmm4\n"
        "    movaps  %%xmm3, 4*4*9(%%edx)\n"
        "    shufps  $0x00, %%xmm5, %%xmm5\n"
        "    movaps  %%xmm4, 4*4*10(%%edx)\n"
        "    movaps  %%xmm5, 4*4*14(%%edx)\n"
        :
        : "a" (m1), "d" (mat1) );
#endif
}

void xmm_update_modelview(GLcontext* ctx) {}
void xmm_update_projection(GLcontext* ctx) {}

void gl_xmm_3dtransform(GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count)
{
#if defined (_MSC_VER)
    static _MM_ALIGN16 int n;
    static _MM_ALIGN16 __m128 One, nextX;
    static _MM_ALIGN16 float* m;
#else
    static int n __attribute__ ((aligned (16)));
    static __m128 One   __attribute__ ((aligned (16))),
                  nextX __attribute__ ((aligned (16)));
    static float* m __attribute__ ((aligned (16)));
#endif

    n = (count + 3) & (~3);

    One = _mm_set_ps1(1.0f);
    m = (float*)mat;

    xmm_set_modelview(matrix);

#if defined (_MSC_VER)
    _asm
    {
        push    esi

        mov     esi, [m]
        mov     ecx, [n]
        mov     eax, [source]
        mov     edx, [dest]
        shl     ecx, 4

    start:
        sub     ecx, 64

        //swizzle
        movups  xmm7, [eax+ecx]
        movups  xmm2, [eax+ecx+16]
        movups  xmm4, [eax+ecx+32]
        movups  xmm3, [eax+ecx+48]
        prefetchnta [eax+ecx-64]
        movaps  xmm6, xmm7
        movaps  xmm5, xmm4
        unpcklps xmm6, xmm2
        unpcklps xmm5, xmm3
        movaps  xmm0, xmm6
        prefetchnta [eax+ecx+16-64]
        shufps  xmm0, xmm5, SELECT(1,0,1,0) //xxxx in xmm0
        shufps  xmm6, xmm5, SELECT(3,2,3,2) //yyyy in xmm6
        unpckhps xmm7, xmm2
        prefetchnta [eax+ecx+32-64]
        unpckhps xmm4, xmm3

        //transform
        movaps  xmm3, XMMWORD PTR [esi+4*4*0]
        movaps  xmm1, XMMWORD PTR [esi+4*4*4]
        mulps   xmm3, xmm0          //x*m0 in xmm3
        prefetchnta [eax+ecx+48-64]
        shufps  xmm7, xmm4, SELECT(1,0,1,0) //zzzz in xmm7
        mulps   xmm1, xmm6          //y*m4 in xmm1
        movaps  xmm4, XMMWORD PTR [esi+4*4*8]
        movaps  xmm5, XMMWORD PTR [esi+4*4*12]
        mulps   xmm4, xmm7          //z*m8 in xmm4
        addps   xmm1, xmm3          //x*m0 + y*m4 in xmm1
        addps   xmm4, xmm5          //z*m8 + m12 in xmm4
        addps   xmm1, xmm4          //x*m0 + y*m4 + z*m8 + m12 in xmm1
        movaps  XMMWORD PTR [nextX], xmm1

        movaps  xmm3, XMMWORD PTR [esi+4*4*1]
        movaps  xmm2, XMMWORD PTR [esi+4*4*5]
        mulps   xmm3, xmm0          //x*m1 in xmm3
        mulps   xmm2, xmm6          //y*m5 in xmm2
        movaps  xmm4, XMMWORD PTR [esi+4*4*9]
        movaps  xmm5, XMMWORD PTR [esi+4*4*13]
        mulps   xmm4, xmm7          //z*m9 in xmm4
        addps   xmm2, xmm3          //x*m1 + y*m5 in xmm2
        addps   xmm4, xmm5          //z*m9 + m13 in xmm4
        movaps  xmm3, XMMWORD PTR [esi+4*4*2]   //...
        addps   xmm4, xmm2          //x*m1 + y*m5 + z*m9 + m13 in xmm4

        movaps  xmm1, XMMWORD PTR [esi+4*4*6]
        mulps   xmm3, xmm0          //x*m2 in xmm3
        mulps   xmm1, xmm6          //y*m6 in xmm1
        movaps  xmm2, XMMWORD PTR [esi+4*4*10]
        movaps  xmm5, XMMWORD PTR [esi+4*4*14]
        mulps   xmm2, xmm7          //z*m10 in xmm4
        addps   xmm1, xmm3          //x*m2 + y*m6 in xmm1
        addps   xmm2, xmm5          //z*m10 + m14 in xmm4
        addps   xmm1, xmm2          //x*m2 + y*m6 + z*m10 + m14 in xmm1

        //swizzle
        movaps  xmm2, [nextX]       //x
        movaps  xmm6, xmm1          //z
        movaps  xmm3, [One]         //w
        movaps  xmm0, xmm2
        unpcklps xmm0, xmm4                 //y1,x1,y0,x0
        movaps  xmm1, xmm0
        movaps  xmm5, xmm6
        unpcklps xmm5, xmm3                 //w1,z1,w0,z0
        shufps  xmm1, xmm5, SELECT(1,0,1,0) //w0,z0,y0,x0
        movups  [edx+ecx], xmm1
        shufps  xmm0, xmm5, SELECT(3,2,3,2) //w1,z1,y1,x1
        movups  [edx+ecx+16], xmm0
        movaps  xmm0, xmm2
        unpckhps xmm0, xmm4                 //y3,x3,y2,x2
        movaps  xmm1, xmm0
        movaps  xmm5, xmm6
        unpckhps xmm5, xmm3                 //w3,z3,w2,z2
        shufps  xmm1, xmm5, SELECT(1,0,1,0) //w2,z2,y2,x2
        movups  [edx+ecx+32], xmm1
        shufps  xmm0, xmm5, SELECT(3,2,3,2) //w3,z3,y3,x3
        movups  [edx+ecx+48], xmm0

        jnz     start

        pop     esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    shll $4, %%ecx\n"

        "xmm3dtrans_start:\n"
        "    subl $64, %%ecx\n"

        //swizzle
        "    movups   0(%%eax, %%ecx), %%xmm7\n"
        "    movups  16(%%eax, %%ecx), %%xmm2\n"
        "    movups  32(%%eax, %%ecx), %%xmm4\n"
        "    movups  48(%%eax, %%ecx), %%xmm3\n"
        "    prefetchnta -64(%%eax, %%ecx)\n"
        "    movaps  %%xmm7, %%xmm6\n"
        "    movaps  %%xmm4, %%xmm5\n"
        "    unpcklps %%xmm2, %%xmm6\n"
        "    unpcklps %%xmm3, %%xmm5\n"
        "    movaps  %%xmm6, %%xmm0\n"
        "    prefetchnta 16-64(%%eax, %%ecx)\n"
        "    shufps  " SELECT(1,0,1,0) ", %%xmm5, %%xmm0\n" //xxxx in xmm0
        "    shufps  " SELECT(3,2,3,2) ", %%xmm5, %%xmm6\n" //yyyy in xmm6
        "    unpckhps %%xmm2, %%xmm7\n"
        "    prefetchnta 32-64(%%eax, %%ecx)\n"
        "    unpckhps %%xmm3, %%xmm4\n"

        //transform
        "    movaps  4*4*0(%%esi), %%xmm3\n"
        "    movaps  4*4*4(%%esi), %%xmm1\n"
        "    mulps   %%xmm0, %%xmm3\n"          //x*m0 in xmm3
        "    prefetchnta 48-64(%%eax, %%ecx)\n"
        "    shufps  " SELECT(1,0,1,0) ", %%xmm4, %%xmm7\n" //zzzz in xmm7
        "    mulps   %%xmm6, %%xmm1\n"          //y*m4 in xmm1
        "    movaps  4*4*8(%%esi), %%xmm4\n"
        "    movaps  4*4*12(%%esi), %%xmm5\n"
        "    mulps   %%xmm7, %%xmm4\n"          //z*m8 in xmm4
        "    addps   %%xmm3, %%xmm1\n"          //x*m0 + y*m4 in xmm1
        "    addps   %%xmm5, %%xmm4\n"          //z*m8 + m12 in xmm4
        "    addps   %%xmm4, %%xmm1\n"          //x*m0 + y*m4 + z*m8 + m12 in xmm1
        "    movaps  %%xmm1, %5\n"

        "    movaps  4*4*1(%%esi), %%xmm3\n"
        "    movaps  4*4*5(%%esi), %%xmm2\n"
        "    mulps   %%xmm0, %%xmm3\n"          //x*m1 in xmm3
        "    mulps   %%xmm6, %%xmm2\n"          //y*m5 in xmm2
        "    movaps  4*4*9(%%esi), %%xmm4\n"
        "    movaps  4*4*13(%%esi), %%xmm5\n"
        "    mulps   %%xmm7, %%xmm4\n"          //z*m9 in xmm4
        "    addps   %%xmm3, %%xmm2\n"          //x*m1 + y*m5 in xmm2
        "    addps   %%xmm5, %%xmm4\n"          //z*m9 + m13 in xmm4
        "    movaps  4*4*2(%%esi), %%xmm3\n"   //...
        "    addps   %%xmm2, %%xmm4\n"          //x*m1 + y*m5 + z*m9 + m13 in xmm4

        "    movaps  4*4*6(%%esi), %%xmm1\n"
        "    mulps   %%xmm0, %%xmm3\n"          //x*m2 in xmm3
        "    mulps   %%xmm6, %%xmm1\n"          //y*m6 in xmm1
        "    movaps  4*4*10(%%esi), %%xmm2\n"
        "    movaps  4*4*14(%%esi), %%xmm5\n"
        "    mulps   %%xmm7, %%xmm2\n"          //z*m10 in xmm4
        "    addps   %%xmm3, %%xmm1\n"          //x*m2 + y*m6 in xmm1
        "    addps   %%xmm5, %%xmm2\n"          //z*m10 + m14 in xmm4
        "    addps   %%xmm2, %%xmm1\n"          //x*m2 + y*m6 + z*m10 + m14 in xmm1

        //swizzle
        "    movaps  %5, %%xmm2\n"              //x
        "    movaps  %%xmm1, %%xmm6\n"          //z
        "    movaps  %4, %%xmm3\n"              //w
        "    movaps  %%xmm2, %%xmm0\n"
        "    unpcklps %%xmm4, %%xmm0\n"                 //y1,x1,y0,x0
        "    movaps  %%xmm0, %%xmm1\n"
        "    movaps  %%xmm6, %%xmm5\n"
        "    unpcklps %%xmm3, %%xmm5\n"                 //w1,z1,w0,z0
        "    shufps  " SELECT(1,0,1,0) ", %%xmm5, %%xmm1\n" //w0,z0,y0,x0
        "    movups  %%xmm1, (%%edx, %%ecx)\n"
        "    shufps  " SELECT(3,2,3,2) ", %%xmm5, %%xmm0\n" //w1,z1,y1,x1
        "    movups  %%xmm0, 16(%%edx, %%ecx)\n"
        "    movaps  %%xmm2, %%xmm0\n"
        "    unpckhps %%xmm4, %%xmm0\n"                 //y3,x3,y2,x2
        "    movaps  %%xmm0, %%xmm1\n"
        "    movaps  %%xmm6, %%xmm5\n"
        "    unpckhps %%xmm3, %%xmm5\n"                 //w3,z3,w2,z2
        "    shufps  " SELECT(1,0,1,0) ", %%xmm5, %%xmm1\n" //w2,z2,y2,x2
        "    movups  %%xmm1, 32(%%edx, %%ecx)\n"
        "    shufps  " SELECT(3,2,3,2) ", %%xmm5, %%xmm0\n" //w3,z3,y3,x3
        "    movups  %%xmm0, 48(%%edx, %%ecx)\n"

        "    jnz     xmm3dtrans_start\n"
        :
        : "S" (m), "c" (n), "a" (source), "d" (dest),
          "m" (One), "m" (nextX) );
#endif
}

void gl_intrin_3dtransform(GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count)
{
    int i, j;
    hvector* sp;
    hvector* dp;
    float* xp;

#if defined (_MSC_VER)
    _MM_ALIGN16 __m128 curX, curY, curZ;
    _MM_ALIGN16 __m128 out[4];
#else
    __m128 curX __attribute__ ((aligned (16))),
           curY __attribute__ ((aligned (16))),
           curZ __attribute__ ((aligned (16)));
    __m128 out[4] __attribute__ ((aligned (16)));
#endif

    count = ((count + 3) & (~3)) >> 2;

    xmm_set_modelview(matrix);

    xp = (float*)&out[0];
    dp = (hvector*)dest;
    sp = (hvector*)source;
    for (i = 0; i < count; i++)
    {
        curX = _mm_set_ps((sp+4*i+3)->x, (sp+4*i+2)->x, (sp+4*i+1)->x, (sp+4*i+0)->x);
        curY = _mm_set_ps((sp+4*i+3)->y, (sp+4*i+2)->y, (sp+4*i+1)->y, (sp+4*i+0)->y);
        curZ = _mm_set_ps((sp+4*i+3)->z, (sp+4*i+2)->z, (sp+4*i+1)->z, (sp+4*i+0)->z);

        out[0] = _mm_add_ps(
                     _mm_add_ps(
                         _mm_mul_ps(curX, mat[0]),
                         _mm_mul_ps(curY, mat[4])),
                     _mm_add_ps(
                         _mm_mul_ps(curZ, mat[8]),
                         mat[12]));
        out[1] = _mm_add_ps(
                     _mm_add_ps(
                         _mm_mul_ps(curX, mat[1]),
                         _mm_mul_ps(curY, mat[5])),
                     _mm_add_ps(
                         _mm_mul_ps(curZ, mat[9]),
                         mat[13]));
        out[2] = _mm_add_ps(
                     _mm_add_ps(
                         _mm_mul_ps(curX, mat[2]),
                         _mm_mul_ps(curY, mat[6])),
                     _mm_add_ps(
                         _mm_mul_ps(curZ, mat[10]),
                         mat[14]));

        for (j = 0; j < 4; j++, dp++)
        {
            dp->x = *(xp + j + 0);
            dp->y = *(xp + j + 4);
            dp->z = *(xp + j + 8);
            dp->w = 1.0f;
        }
    }
}

#if defined (_MSC_VER)
static _MM_ALIGN16 int Mask[4] = {0x80000000,0x80000000,0x80000000,0x80000000};
#else
static int Mask[4] __attribute__ ((aligned (16))) =
    {0x80000000,0x80000000,0x80000000,0x80000000};
#endif

void gl_xmm_project(GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count)
{
#if defined (_MSC_VER)
    static _MM_ALIGN16 int n;
    static _MM_ALIGN16 __m128 Zero;
    static _MM_ALIGN16 float* m;
#else
    static int n __attribute__ ((aligned (16)));
    static __m128 Zero __attribute__ ((aligned (16)));
    static float* m __attribute__ ((aligned (16)));
#endif

    n = (count + 3) & (~3);

    Zero = _mm_set_ps1(0.0f);
    m = (float*)per;

    xmm_set_projection(matrix);

#if defined (_MSC_VER)
    _asm
    {
        push    esi

        mov     esi, [m]
        mov     ecx, [n]
        mov     eax, [source]
        mov     edx, [dest]
        shl     ecx, 4

    start:
        sub     ecx, 64

        //swizzle
        movups  xmm7, [eax+ecx]
        movups  xmm2, [eax+ecx+16]
        movups  xmm4, [eax+ecx+32]
        movups  xmm3, [eax+ecx+48]
        prefetchnta [eax+ecx-64]
        movaps  xmm6, xmm7
        movaps  xmm5, xmm4
        unpcklps xmm6, xmm2
        unpcklps xmm5, xmm3
        movaps  xmm0, xmm6
        prefetchnta [eax+ecx+16-64]
        shufps  xmm0, xmm5, SELECT(1,0,1,0) //xxxx in xmm0
        shufps  xmm6, xmm5, SELECT(3,2,3,2) //yyyy in xmm6
        unpckhps xmm7, xmm2
        prefetchnta [eax+ecx+32-64]
        unpckhps xmm4, xmm3

        //project
        movaps  xmm3, [esi+4*4*0]
        movaps  xmm2, [esi+4*4*8]
        shufps  xmm7, xmm4, SELECT(1,0,1,0) //zzzz in xmm7
        prefetchnta [eax+ecx+48-64]
        mulps   xmm3, xmm0                  //x*p0 in xmm3
        mulps   xmm2, xmm7                  //z*p8 in xmm2
        movaps  xmm5, [esi+4*4*5]           //...
        addps   xmm2, xmm3                  //x*p0 + z*p8 in xmm2

        movaps  xmm4, [esi+4*4*9]
        mulps   xmm5, xmm6                  //y*p5 in xmm5
        mulps   xmm4, xmm7                  //z*p8 in xmm2
        movaps  xmm3, [esi+4*4*10]          //...
        addps   xmm4, xmm5                  //x*p0 + z*p8 in xmm2

        movaps  xmm6, [esi+4*4*14]
        mulps   xmm3, xmm7                  //z*p10 in xmm3
        movaps  xmm1, xmm7                  //...
        addps   xmm6, xmm3                  //z*p10 + p14 in xmm6

        xorps   xmm1, [Mask]
        movaps  xmm0, xmm2                  //...
        movaps  xmm3, xmm1                  //-z in xmm3

        //swizzle
        unpcklps xmm0, xmm4                 //y1,x1,y0,x0
        movaps  xmm1, xmm0
        movaps  xmm5, xmm6
        unpcklps xmm5, xmm3                 //w1,z1,w0,z0
        shufps  xmm1, xmm5, SELECT(1,0,1,0) //w0,z0,y0,x0
        movups  [edx+ecx], xmm1
        shufps  xmm0, xmm5, SELECT(3,2,3,2) //w1,z1,y1,x1
        movups  [edx+ecx+16], xmm0
        movaps  xmm0, xmm2
        unpckhps xmm0, xmm4                 //y3,x3,y2,x2
        movaps  xmm1, xmm0
        movaps  xmm5, xmm6
        unpckhps xmm5, xmm3                 //w3,z3,w2,z2
        shufps  xmm1, xmm5, SELECT(1,0,1,0) //w2,z2,y2,x2
        movups  [edx+ecx+32], xmm1
        shufps  xmm0, xmm5, SELECT(3,2,3,2) //w3,z3,y3,x3
        movups  [edx+ecx+48], xmm0

        jnz     start

        pop     esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    shll     $4, %%ecx\n"

        "xmmproject_start:\n"
        "    subl     $64, %%ecx\n"

        //swizzle
        "    movups  (%%eax, %%ecx), %%xmm7\n"
        "    movups  16(%%eax, %%ecx), %%xmm2\n"
        "    movups  32(%%eax, %%ecx), %%xmm4\n"
        "    movups  48(%%eax, %%ecx), %%xmm3\n"
        "    prefetchnta -64(%%eax, %%ecx)\n"
        "    movaps  %%xmm7, %%xmm6\n"
        "    movaps  %%xmm4, %%xmm5\n"
        "    unpcklps %%xmm2, %%xmm6\n"
        "    unpcklps %%xmm3, %%xmm5\n"
        "    movaps  %%xmm6, %%xmm0\n"
        "    prefetchnta 16-64(%%eax, %%ecx)\n"
        "    shufps  " SELECT(1,0,1,0) ", %%xmm5, %%xmm0\n" //xxxx in xmm0
        "    shufps  " SELECT(3,2,3,2) ", %%xmm5, %%xmm6\n" //yyyy in xmm6
        "    unpckhps %%xmm2, %%xmm7\n"
        "    prefetchnta 32-64(%%eax, %%ecx)\n"
        "    unpckhps %%xmm3, %%xmm4\n"

        //project
        "    movaps  4*4*0(%%esi), %%xmm3\n"
        "    movaps  4*4*8(%%esi), %%xmm2\n"
        "    shufps  " SELECT(1,0,1,0) ", %%xmm4, %%xmm7\n" //zzzz in xmm7
        "    prefetchnta 48-64(%%eax, %%ecx)\n"
        "    mulps   %%xmm0, %%xmm3\n"                  //x*p0 in xmm3
        "    mulps   %%xmm7, %%xmm2\n"                  //z*p8 in xmm2
        "    movaps  4*4*5(%%esi), %%xmm5\n"            //...
        "    addps   %%xmm3, %%xmm2\n"                  //x*p0 + z*p8 in xmm2

        "    movaps  4*4*9(%%esi), %%xmm4\n"
        "    mulps   %%xmm6, %%xmm5\n"                  //y*p5 in xmm5
        "    mulps   %%xmm7, %%xmm4\n"                  //z*p8 in xmm2
        "    movaps  4*4*10(%%esi), %%xmm3\n"           //...
        "    addps   %%xmm5, %%xmm4\n"                  //x*p0 + z*p8 in xmm2

        "    movaps  4*4*14(%%esi), %%xmm6\n"
        "    mulps   %%xmm7, %%xmm3\n"                  //z*p10 in xmm3
        "    movaps  %%xmm7, %%xmm1\n"                  //...
        "    addps   %%xmm3, %%xmm6\n"                  //z*p10 + p14 in xmm6

        "    xorps   %4, %%xmm1\n"
        "    movaps  %%xmm2, %%xmm0\n"                  //...
        "    movaps  %%xmm1, %%xmm3\n"                  //-z in xmm3

        //swizzle
        "    unpcklps %%xmm4, %%xmm0\n"                 //y1,x1,y0,x0
        "    movaps  %%xmm0, %%xmm1\n"
        "    movaps  %%xmm6, %%xmm5\n"
        "    unpcklps %%xmm3, %%xmm5\n"                 //w1,z1,w0,z0
        "    shufps  " SELECT(1,0,1,0) ", %%xmm5, %%xmm1\n" //w0,z0,y0,x0
        "    movups  %%xmm1, (%%edx, %%ecx)\n"
        "    shufps  " SELECT(3,2,3,2) ", %%xmm5, %%xmm0\n" //w1,z1,y1,x1
        "    movups  %%xmm0, 16(%%edx, %%ecx)\n"
        "    movaps  %%xmm2, %%xmm0\n"
        "    unpckhps %%xmm4, %%xmm0\n"                 //y3,x3,y2,x2
        "    movaps  %%xmm0, %%xmm1\n"
        "    movaps  %%xmm6, %%xmm5\n"
        "    unpckhps %%xmm3, %%xmm5\n"                 //w3,z3,w2,z2
        "    shufps  " SELECT(1,0,1,0) ", %%xmm5, %%xmm1\n" //w2,z2,y2,x2
        "    movups  %%xmm1, 32(%%edx, %%ecx)\n"
        "    shufps  " SELECT(3,2,3,2) ", %%xmm5, %%xmm0\n" //w3,z3,y3,x3
        "    movups  %%xmm0, 48(%%edx, %%ecx)\n"

        "    jnz     xmmproject_start\n"
        :
        : "S" (m), "c" (n), "a" (source), "d" (dest),
          "m" (Mask) );
#endif
}

void gl_intrin_project(GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count)
{
    GLint i, j;
    hvector* sp;
    hvector* dp;
    float* xp;

#if defined (_MSC_VER)
    _MM_ALIGN16 __m128 curX, curY, curZ;
    _MM_ALIGN16 __m128 out[4];
    _MM_ALIGN16 int mask[4];
#else
    __m128 curX __attribute__ ((aligned (16))),
           curY __attribute__ ((aligned (16))),
           curZ __attribute__ ((aligned (16)));
    __m128 out[4] __attribute__ ((aligned (16)));
    int mask[4] __attribute__ ((aligned (16)));
#endif

    mask[0] = mask[1] = mask[2] = mask[3] = 0x80000000;

    count = ((count + 3) & (~3)) >> 2;

    xmm_set_projection(matrix);

    xp = (float*)&out[0];
    dp = (hvector*)dest;
    sp = (hvector*)source;
    for (i = 0; i < count; i++)
    {
        curX = _mm_set_ps((sp+4*i+3)->x, (sp+4*i+2)->x, (sp+4*i+1)->x, (sp+4*i+0)->x);
        curY = _mm_set_ps((sp+4*i+3)->y, (sp+4*i+2)->y, (sp+4*i+1)->y, (sp+4*i+0)->y);
        curZ = _mm_set_ps((sp+4*i+3)->z, (sp+4*i+2)->z, (sp+4*i+1)->z, (sp+4*i+0)->z);

        out[0] = _mm_add_ps(
                    _mm_mul_ps(curX, per[0]),
                    _mm_mul_ps(curY, per[8]));
        out[1] = _mm_add_ps(
                    _mm_mul_ps(curY, per[5]),
                    _mm_mul_ps(curZ, per[9]));
        out[2] = _mm_add_ps(
                    _mm_mul_ps(curZ, per[10]),
                    per[14]);
        out[3] = _mm_xor_ps(curZ, (*((__m128*)(&(mask[0])))));

        for (j = 0; j < 4; j++, dp++)
        {
            dp->x = *(xp + j + 0);
            dp->y = *(xp + j + 4);
            dp->z = *(xp + j + 8);
            dp->w = *(xp + j + 12);
        }
    }
}

/*
 * transform an array of vertices by a 4x4 matrix, really quickly
 */
void gl_megafast_affine_transform(
        GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint count)
{
    GLfloat floatOne = 1.0f;

#if defined (_MSC_VER)
    _asm
    {
        mov ecx, count
        mov esi, [source]
        mov ebx, [dest]
        mov edi, [matrix]
        shl ecx, 4
        add esi, ecx
        add ebx, ecx

        neg ecx

        cmp ecx, 0
        jne MEGA0
        ret

    MEGA0:
        fld     FPTR [esi+ecx+0*FSIZE]
        fmul    FPTR [edi+(0*4+0)*FSIZE]
        fld     FPTR [esi+ecx+1*FSIZE]
        fmul    FPTR [edi+(1*4+0)*FSIZE]
        fld     FPTR [esi+ecx+2*FSIZE]
        fmul    FPTR [edi+(2*4+0)*FSIZE]
        fxch    st(1)
        faddp   st(2), st
        fld     FPTR [esi+ecx+0*FSIZE]
        fmul    FPTR [edi+(0*4+1)*FSIZE]
        fxch    st(1)
        faddp   st(2), st
        fld     FPTR [esi+ecx+1*FSIZE]
        fmul    FPTR [edi+(1*4+1)*FSIZE]
        fld     FPTR [esi+ecx+2*FSIZE]
        fmul    FPTR [edi+(2*4+1)*FSIZE]
        fxch    st(1)
        faddp   st(2), st
        fld     FPTR [esi+ecx+0*FSIZE]
        fmul    FPTR [edi+(0*4+2)*FSIZE]
        fxch    st(1)
        faddp   st(2), st
        fld     FPTR [esi+ecx+1*FSIZE]
        fmul    FPTR [edi+(1*4+2)*FSIZE]
        fld     FPTR [esi+ecx+2*FSIZE]
        fmul    FPTR [edi+(2*4+2)*FSIZE]
        fxch    st(1)
        faddp   st(2), st
        fxch    st(3)
        fadd    FPTR [edi+(3*4+0)*FSIZE]
        fxch    st(1)
        faddp   st(3), st
        fxch    st(1)
        fadd    FPTR [edi+(3*4+1)*FSIZE]
        fxch    st(2)
        fadd    FPTR [edi+(3*4+2)*FSIZE]
        fxch    st(1)
        fstp    FPTR [ebx+ecx+0*FSIZE]
        fxch    st(1)
        fstp    FPTR [ebx+ecx+1*FSIZE]
        fstp    FPTR [ebx+ecx+2*FSIZE]

        fld     FPTR [floatOne]
        fstp    FPTR [ebx+ecx+3*FSIZE]

        add     ecx, 4*FSIZE
        jnz     MEGA0
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    shll $4, %%ecx\n"
        "    addl %%ecx, %%esi\n"
        "    addl %%ecx, %%eax\n"

        "    negl %%ecx\n"

        "    cmpl $0, %%ecx\n"
        "    je MEGA1\n"  /* ... */

        "MEGA0:\n"
        "    flds    0*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (0*4+0)*"FSIZE_STR"(%%edi)\n"
        "    flds    1*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (1*4+0)*"FSIZE_STR"(%%edi)\n"
        "    flds    2*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (2*4+0)*"FSIZE_STR"(%%edi)\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(2)\n"
        "    flds    0*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (0*4+1)*"FSIZE_STR"(%%edi)\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(2)\n"
        "    flds    1*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (1*4+1)*"FSIZE_STR"(%%edi)\n"
        "    flds    2*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (2*4+1)*"FSIZE_STR"(%%edi)\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(2)\n"
        "    flds    0*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (0*4+2)*"FSIZE_STR"(%%edi)\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(2)\n"
        "    flds    1*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (1*4+2)*"FSIZE_STR"(%%edi)\n"
        "    flds    2*"FSIZE_STR"(%%esi, %%ecx)\n"
        "    fmuls   (2*4+2)*"FSIZE_STR"(%%edi)\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(2)\n"
        "    fxch    %%st(3)\n"
        "    faddl   (3*4+0)*"FSIZE_STR"(%%edi)\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(3)\n"
        "    fxch    %%st(1)\n"
        "    faddl   (3*4+1)*"FSIZE_STR"(%%edi)\n"
        "    fxch    %%st(2)\n"
        "    faddl   (3*4+2)*"FSIZE_STR"(%%edi)\n"
        "    fxch    %%st(1)\n"
        "    fstps   0*"FSIZE_STR"(%%eax, %%ecx)\n"
        "    fxch    %%st(1)\n"
        "    fstps   1*"FSIZE_STR"(%%eax, %%ecx)\n"
        "    fstps   2*"FSIZE_STR"(%%eax, %%ecx)\n"

        "    flds    %4\n"
        "    fstps   3*"FSIZE_STR"(%%eax, %%ecx)\n"

        "    addl    $(4*"FSIZE_STR"), %%ecx\n"
        "    jnz     MEGA0\n"

        "MEGA1:\n"
        :
        : "c" (count), "S" (source), "a" (dest), "D" (matrix),
          "m" (floatOne) );
#endif
}

/*
 * transform normals reasonably quickly, at least quicker than
 * MS or Intel's C compilers can muster
 */
void gl_wicked_fast_normal_xform(
        GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint n)
{
#if defined (_MSC_VER)
    _asm
    {
        push esi
        push edi
        push ebx

        mov ecx,[n]
        mov SOURCE,[source]
        mov DEST,[dest]
        mov MATRIX,[matrix]

        imul ecx,ecx,3*FSIZE
        add  SOURCE,ecx
        add  DEST,ecx

        neg  ecx

    WICK0:
        fld     FPTR [SOURCE+ecx+0*FSIZE]       //s0
        fmul    FPTR [MATRIX+0*FSIZE]           //M0
        fld     FPTR [SOURCE+ecx+1*FSIZE]       //s1 M0
        fmul    FPTR [MATRIX+1*FSIZE]           //M1 M0
        fld     FPTR [SOURCE+ecx+2*FSIZE]       //s2 M1 M0
        fmul    FPTR [MATRIX+2*FSIZE]           //M2 M1 M0
        fxch    st(1)                           //M1 M2 M0
        faddp   st(2),st                        //M2 M1+M0

        fld     FPTR [SOURCE+ecx+0*FSIZE]       //s0 M2 M1+M0
        fmul    FPTR [MATRIX+4*FSIZE]           //M4 M2 M1+M0
        fxch    st(1)                           //M2 M4 M1+M0
        faddp   st(2),st                        //M4 R0

        fld     FPTR [SOURCE+ecx+1*FSIZE]       //s1 M4 R0
        fmul    FPTR [MATRIX+5*FSIZE]           //M5 M4 R0
        fld     FPTR [SOURCE+ecx+2*FSIZE]       //s2 M5 M4 R0
        fmul    FPTR [MATRIX+6*FSIZE]           //M6 M5 M4 R0
        fxch    st(1)                           //M5 M6 M4 R0
        faddp   st(1),st                        //M5+M6 M4 R0

        fld     FPTR [SOURCE+ecx+0*FSIZE]       //s0 M5+M6 M4 R0
        fmul    FPTR [MATRIX+8*FSIZE]           //M8 M5+M6 M4 R0
        fxch    st(1)                           //M5+M6 M8 M4 R0
        faddp   st(2),st                        //M8 R1 R0

        fld     FPTR [SOURCE+ecx+1*FSIZE]       //s1 M8 R1 R0
        fmul    FPTR [MATRIX+9*FSIZE]           //M9 M8 R1 R0
        fld     FPTR [SOURCE+ecx+2*FSIZE]       //s2 M9 M8 R1 R0
        fmul    FPTR [MATRIX+10*FSIZE]          //M10 M9 M8 R1 R0
        fxch    st(1)                           //M9 M10 M8 R1 R0
        faddp   st(1),st                        //M9+M10 M8 R1 R0

        fxch    st(2)                           //R1 M8 M9+M10 R0
        fstp    FPTR [DEST+ecx+1*FSIZE]         //M8 M9+M10 R0
        fxch    st(1)                           //M9+M10 M8 R0
        faddp   st(1),st                        //R2 R0
        fxch    st(1)                           //R0 R2
        fstp    FPTR [DEST+ecx+0*FSIZE]         //R2
        fstp    FPTR [DEST+ecx+2*FSIZE]

        add ecx, 3*FSIZE
        jnz WICK0

        pop ebx
        pop edi
        pop esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    imull $(3*"FSIZE_STR"), %%ecx, %%ecx\n"
        "    addl %%ecx, "SOURCE"\n"
        "    addl %%ecx, "DEST"\n"

        "    negl %%ecx\n"

        "normxform_WICK0:\n"
        "    flds    0*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   0*"FSIZE_STR"("MATRIX")\n"
        "    flds    1*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   1*"FSIZE_STR"("MATRIX")\n"
        "    flds    2*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   2*"FSIZE_STR"("MATRIX")\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(2)\n"

        "    flds    0*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   4*"FSIZE_STR"("MATRIX")\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(2)\n"

        "    flds    1*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   5*"FSIZE_STR"("MATRIX")\n"
        "    flds    2*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   6*"FSIZE_STR"("MATRIX")\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(1)\n"

        "    flds    0*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   8*"FSIZE_STR"("MATRIX")\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(2)\n"

        "    flds    1*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   9*"FSIZE_STR"("MATRIX")\n"
        "    flds    2*"FSIZE_STR"("SOURCE",%%ecx)\n"
        "    fmuls   10*"FSIZE_STR"("MATRIX")\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(1)\n"

        "    fxch    %%st(2)\n"
        "    fstps   1*"FSIZE_STR"("DEST",%%ecx)\n"
        "    fxch    %%st(1)\n"
        "    faddp   %%st, %%st(1)\n"
        "    fxch    %%st(1)\n"
        "    fstps   0*"FSIZE_STR"("DEST",%%ecx)\n"
        "    fstps   2*"FSIZE_STR"("DEST",%%ecx)\n"

        "    addl $(3*"FSIZE_STR"), %%ecx\n"
        "    jnz normxform_WICK0\n"
        :
        : "c" (n), "S" (source), "d" (dest), "D" (matrix) );
#endif
}

/*
 * transform and scale normals quicker than compiled C
 */
void gl_fairly_fast_scaled_normal_xform(
        GLfloat* dest, GLfloat* source, GLfloat* matrix, GLuint n, GLfloat scale)
{
#if defined (_MSC_VER)
    _asm
    {
        push esi
        push edi
        push ebx

        mov ecx,[n]
        mov SOURCE,[source]
        mov DEST,[dest]
        mov MATRIX,[matrix]

        imul ecx,ecx,3*FSIZE
        add  SOURCE,ecx
        add  DEST,ecx

        neg  ecx

    WICK0:
        fld     FPTR [SOURCE+ecx+0*FSIZE]       //s0
        fmul    FPTR [MATRIX+0*FSIZE]           //M0
        fld     FPTR [SOURCE+ecx+1*FSIZE]       //s1 M0
        fmul    FPTR [MATRIX+1*FSIZE]           //M1 M0
        fld     FPTR [SOURCE+ecx+2*FSIZE]       //s2 M1 M0
        fmul    FPTR [MATRIX+2*FSIZE]           //M2 M1 M0
        fxch    st(1)                           //M1 M2 M0
        faddp   st(2),st                        //M2 M1+M0

        fld     FPTR [SOURCE+ecx+0*FSIZE]       //s0 M2 M1+M0
        fmul    FPTR [MATRIX+4*FSIZE]           //M4 M2 M1+M0
        fxch    st(1)                           //M2 M4 M1+M0
        faddp   st(2),st                        //M4 R0

        fld     FPTR [SOURCE+ecx+1*FSIZE]       //s1 M4 R0
        fmul    FPTR [MATRIX+5*FSIZE]           //M5 M4 R0
        fld     FPTR [SOURCE+ecx+2*FSIZE]       //s2 M5 M4 R0
        fmul    FPTR [MATRIX+6*FSIZE]           //M6 M5 M4 R0
        fxch    st(1)                           //M5 M6 M4 R0
        faddp   st(1),st                        //M5+M6 M4 R0

        fld     FPTR [SOURCE+ecx+0*FSIZE]       //s0 M5+M6 M4 R0
        fmul    FPTR [MATRIX+8*FSIZE]           //M8 M5+M6 M4 R0
        fxch    st(1)                           //M5+M6 M8 M4 R0
        faddp   st(2),st                        //M8 R1 R0

        fld     FPTR [SOURCE+ecx+1*FSIZE]       //s1 M8 R1 R0
        fmul    FPTR [MATRIX+9*FSIZE]           //M9 M8 R1 R0
        fld     FPTR [SOURCE+ecx+2*FSIZE]       //s2 M9 M8 R1 R0
        fmul    FPTR [MATRIX+10*FSIZE]          //M10 M9 M8 R1 R0
        fxch    st(1)                           //M9 M10 M8 R1 R0
        faddp   st(1),st                        //M9+M10 M8 R1 R0

        fxch    st(2)                           //R1 M8 M9+M10 R0
        fmul    [scale]
        fstp    FPTR [DEST+ecx+1*FSIZE]         //M8 M9+M10 R0
        fxch    st(1)                           //M9+M10 M8 R0
        faddp   st(1),st                        //R2 R0
        fxch    st(1)                           //R0 R2
        fmul    [scale]
        fstp    FPTR [DEST+ecx+0*FSIZE]         //R2
        fmul    [scale]
        fstp    FPTR [DEST+ecx+2*FSIZE]

        add ecx, 3*FSIZE
        jnz WICK0

        pop ebx
        pop edi
        pop esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    imull $(3*"FSIZE_STR"), %%ecx, %%ecx\n"
        "    addl  "SOURCE", %%ecx\n"
        "    addl  "DEST", %%ecx\n"

        "    negl  %%ecx\n"

        "snormxform_WICK0:\n"
        "    flds    0*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s0
        "    fmuls   0*"FSIZE_STR"("MATRIX")\n"           //M0
        "    flds    1*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s1 M0
        "    fmuls   1*"FSIZE_STR"("MATRIX")\n"           //M1 M0
        "    flds    2*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s2 M1 M0
        "    fmuls   2*"FSIZE_STR"("MATRIX")\n"           //M2 M1 M0
        "    fxch    %%st(1)\n"                           //M1 M2 M0
        "    faddp   %%st, %%st(2)\n"                     //M2 M1+M0

        "    flds    0*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s0 M2 M1+M0
        "    fmuls   4*"FSIZE_STR"("MATRIX")\n"           //M4 M2 M1+M0
        "    fxch    %%st(1)\n"                           //M2 M4 M1+M0
        "    faddp   %%st, %%st(2)\n"                     //M4 R0

        "    flds    1*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s1 M4 R0
        "    fmuls   5*"FSIZE_STR"("MATRIX")\n"           //M5 M4 R0
        "    flds    2*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s2 M5 M4 R0
        "    fmuls   6*"FSIZE_STR"("MATRIX")\n"           //M6 M5 M4 R0
        "    fxch    %%st(1)\n"                           //M5 M6 M4 R0
        "    faddp   %%st, %%st(1)\n"                     //M5+M6 M4 R0

        "    flds    0*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s0 M5+M6 M4 R0
        "    fmuls   8*"FSIZE_STR"("MATRIX")\n"           //M8 M5+M6 M4 R0
        "    fxch    %%st(1)\n"                           //M5+M6 M8 M4 R0
        "    faddp   %%st, %%st(2)\n"                     //M8 R1 R0

        "    flds    1*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s1 M8 R1 R0
        "    fmuls   9*"FSIZE_STR"("MATRIX")\n"           //M9 M8 R1 R0
        "    flds    2*"FSIZE_STR"("SOURCE",%%ecx)\n"     //s2 M9 M8 R1 R0
        "    fmuls   10*"FSIZE_STR"("MATRIX")\n"          //M10 M9 M8 R1 R0
        "    fxch    %%st(1)\n"                           //M9 M10 M8 R1 R0
        "    faddp   %%st, %%st(1)\n"                     //M9+M10 M8 R1 R0

        "    fxch    %%st(2)\n"                           //R1 M8 M9+M10 R0
        "    fmuls   %4\n"
        "    fstps   1*"FSIZE_STR"("DEST",%%ecx)\n"       //M8 M9+M10 R0
        "    fxch    %%st(1)\n"                           //M9+M10 M8 R0
        "    faddp   %%st, %%st(1)\n"                     //R2 R0
        "    fxch    %%st(1)\n"                           //R0 R2
        "    fmuls   %4\n"
        "    fstps   0*"FSIZE_STR"("DEST",%%ecx)\n"       //R2
        "    fmuls   %4\n"
        "    fstps   2*"FSIZE_STR"("DEST",%%ecx)\n"

        "    addl $(3*"FSIZE_STR"), %%ecx\n"
        "    jnz snormxform_WICK0\n"
        :
        : "c" (n), "S" (source), "d" (dest), "D" (matrix), "m" (scale) );
#endif
}

void ablend_565(unsigned char *lpAlpha, unsigned int iAlpPitch,
                unsigned char *lpSrc,   unsigned int iSrcX, unsigned int iSrcY,
                unsigned int iSrcPitch, unsigned char *lpDst,
                unsigned int iDstX,     unsigned int iDstY,
                unsigned int iDstW,     unsigned int iDstH,
                unsigned int iDstPitch);
void ablend_555(unsigned char *lpAlpha, unsigned int iAlpPitch,
                unsigned char *lpSrc,   unsigned int iSrcX, unsigned int iSrcY,
                unsigned int iSrcPitch, unsigned char *lpDst,
                unsigned int iDstX,     unsigned int iDstY,
                unsigned int iDstW,     unsigned int iDstH,
                unsigned int iDstPitch);

void gl_mmx_blend_span(
    GLcontext* ctx, GLuint n, GLubyte* mask, GLubyte rgba[][4], GLushort* dest)
{
    GLushort  srcbuf[MAX_WIDTH+4];
    GLubyte   alphabuf[MAX_WIDTH+8];
    GLushort* src;
    GLubyte*  alpha;
    GLuint    i;

    src = srcbuf;
    while ((GLint)src & 7) src++;
    alpha = alphabuf;
    while ((GLint)alpha & 7) alpha++;

    if (ctx->Buffer.PixelType == GL_RGB565)
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                src[i] = FORM_RGB565(rgba[i][0], rgba[i][1], rgba[i][2]);
                alpha[i] = rgba[i][3];
            }
            else
            {
                alpha[i] = 0;
            }
        }

        ablend_565(alpha, n,
                   (GLubyte*)src, 0, 0, 2*n,
                   (GLubyte*)dest, 0, 0, n, 1, 2*n);
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (mask[i])
            {
                src[i] = FORM_RGB555(rgba[i][0], rgba[i][1], rgba[i][2]);
                alpha[i] = rgba[i][3];
            }
            else
            {
                alpha[i] = 0;
            }
        }

        ablend_555(alpha, n,
                   (GLubyte*)src, 0, 0, 2*n,
                   (GLubyte*)dest, 0, 0, n, 1, 2*n);
    }
}

/*
 * I'd like to thank Intel for the MMX blenders below.
 * Notes:
 *      o Could write directly to screen & save a separate span read / write if
 *        these fns looked at mask[]
 *      o (or, Alp could be 0 where mask is also 0)
 *      o It'd be nice if Src+Alp was 8888 instead of 565+8
 */

void ablend_565(unsigned char *lpAlpha, unsigned int iAlpPitch,
                unsigned char *lpSrc,   unsigned int iSrcX, unsigned int iSrcY,
                unsigned int iSrcPitch, unsigned char *lpDst,
                unsigned int iDstX,     unsigned int iDstY,
                unsigned int iDstW,     unsigned int iDstH,
                unsigned int iDstPitch)

{
#if defined (_MSC_VER)
        //Mask for isolating the red, green, and blue components
        static int64_t MASKB=0x001F001F001F001F;
        static int64_t MASKG=0x07E007E007E007E0;
        static int64_t MASKSHIFTG=0x03F003F003F003F0;
        static int64_t MASKR=0xF800F800F800F800;

        //constants used by the integer alpha blending equation
        static int64_t SIXTEEN=0x0010001000100010;
        static int64_t FIVETWELVE=0x0200020002000200;
        static int64_t SIXONES=0x003F003F003F003F;

        unsigned char *lpLinearDstBp=(iDstX<<1)+(iDstY*iDstPitch)+lpDst; //base pointer for linear destination
        unsigned char *lpLinearSrcBp=(iSrcX<<1)+(iSrcY*iSrcPitch)+lpSrc; //base pointer for linear source
        unsigned char *lpLinearAlpBp=iSrcX+(iSrcY*iAlpPitch)+lpAlpha; //base pointer for linear alpha

        _asm
        {
                mov     esi,lpLinearSrcBp;      //src
                mov     edi,lpLinearDstBp;      //dst

                mov     eax,lpLinearAlpBp;      //alpha
                mov     ecx,iDstH;      //ecx=number of lines to copy

                mov     ebx,iDstW;      //ebx=span width to copy
                test    esi,6;          //check if source address is qword aligned
                                                        //since addr coming in is always word aligned(16bit)
                jnz     done;   //if not qword aligned we don't do anything

primeloop:
                movd    mm1,[eax];      //mm1=00 00 00 00 a3 a2 a1 a0
                pxor    mm2,mm2;        //mm2=0;

                movq    mm4,[esi];      //g1: mm4=src3 src2 src1 src0
                punpcklbw mm1,mm2;      //mm1=00a3 00a2 00a1 00a0

loopqword:
                mov     edx,[eax];

                test    ebx,0xFFFFFFFC; //check if only 3 pixels left
                jz      checkback;      //3 or less pixels left

                //early out tests
                cmp     edx,0xffffffff; //test for alpha value of 1
                je      copyback;       //if 1's copy the source pixels to the destination

                test    edx,0xffffffff; //test for alpha value of 0
                jz      leavefront;     //if so go to the next 4 pixels

                //the alpha blend starts
                //green
                //i=a*sg+(63-a)*dg;
                //i=(i+32)+((i+32)>>6)>>6;
                //red
                //i=a*sr+(31-a)*dr;
                //i=(i+16)+((i+16)>>5)>>5;
                movq    mm5,[edi];      //g2: mm5=dst3 dst2 dst1 dst0
                psrlw   mm1,2;          //mm1=a?>>2 nuke out lower 2 bits

                movq    mm7,MASKSHIFTG; //g3: mm7=1 bit shifted green mask
                psrlw   mm4,1;          //g3a: move src green down by 1 so that we won't overflow

                movq    mm0,mm1;        //mm0=00a3 00a2 00a1 00a0
                psrlw   mm5,1;          //g3b: move dst green down by 1 so that we won't overflow

                psrlw   mm1,1;          //mm1=a?>>1 nuke out lower 1 bits
                pand    mm4,mm7;        //g5: mm4=sg3 sg2 sg1 sg0

                movq    mm2,SIXONES;    //g4: mm2=63
                pand    mm5,mm7;        //g7: mm5=dg3 dg2 dg1 dg0

                movq    mm3,[esi];      //b1: mm3=src3 src2 src1 src0
                psubsb  mm2,mm0;        //g6: mm2=63-a3 63-a2 63-a1 63-a0

                movq    mm7,MASKB;      //b2: mm7=BLUE MASK
                pmullw  mm4,mm0;        //g8: mm4=sg?*a?

                movq    mm0,[edi];      //b3: mm0=dst3 dst2 dst1 dst0
                pmullw  mm5,mm2;        //g9: mm5=dg?*(1-a?)

                movq    mm2,mm7;        //b4: mm2=fiveones
                pand    mm3,mm7;        //b4: mm3=sb3 sb2 sb1 sb0

                pmullw  mm3,mm1;        //b6: mm3=sb?*a?
                pand    mm0,mm7;        //b5: mm0=db3 db2 db1 db0

                movq    mm7,[esi];      //r1: mm7=src3 src2 src1 src0
                paddw   mm4,mm5;        //g10: mm4=sg?*a?+dg?*(1-a?)

                pand    mm7,MASKR;      //r2: mm7=sr3 sr2 sr1 sr0
                psubsb  mm2,mm1;        //b5a: mm2=31-a3 31-a2 31-a1 31-a0

                paddw   mm4,FIVETWELVE; //g11: mm4=(mm4+512) green
                pmullw  mm0,mm2;        //b7: mm0=db?*(1-a?)

                movq    mm5,mm4;        //g12: mm5=mm4 green
                psrlw   mm7,11;         //r4: shift src red down to position 0

                psrlw   mm4,6;          //g13: mm4=mm4>>6

                paddw   mm4,mm5;        //g14: mm4=mm4+mm5 green

                paddw   mm0,mm3;        //b8: mm0=sb?*a?+db?*(1-a?)

                movq    mm5,[edi];      //r3: mm5=dst3 dst2 dst1 dst0

                paddw   mm0,SIXTEEN;    //b9: mm0=(mm0+16) blue

                pand    mm5,MASKR;      //r5: mm5=dr3 dr2 dr1 dr0
                psrlw   mm4,5;          //g15: mm4=0?g0 0?g0 0?g0 0?g0 green

                movq    mm3,mm0;        //b10: mm3=mm0 blue
                psrlw   mm0,5;          //b11: mm0=mm0>>5 blue

                psrlw   mm5,11;         //r6: shift dst red down to position 0
                paddw   mm0,mm3;        //b12: mm0=mm3+mm0 blue

                psrlw   mm0,5;          //b13: mm0=000b 000b 000b 000b blue
                pmullw  mm7,mm1;        //mm7=sr?*a?

                pand    mm4,MASKG;      //g16: mm4=00g0 00g0 00g0 00g0 green
                pmullw  mm5,mm2;        //r7: mm5=dr?*(31-a?)

                por     mm0,mm4;        //mm0=00gb 00gb 00gb 00gb
                add     eax,4;          //move to next 4 alphas

                add     esi,8;          //move to next 4 pixels in src
                add     edi,8;          //move to next 4 pixels in dst

                movd    mm1,[eax];      //mm1=00 00 00 00 a3 a2 a1 a0
                paddw   mm5,mm7;        //r8: mm5=sr?*a?+dr?*(31-a?)

                paddw   mm5,SIXTEEN;    //r9: mm5=(mm5+16) red
                pxor    mm2,mm2;        //mm2=0;

                movq    mm7,mm5;        //r10: mm7=mm5 red
                psrlw   mm5,5;          //r11: mm5=mm5>>5 red

                movq    mm4,[esi];      //g1: mm4=src3 src2 src1 src0
                paddw   mm5,mm7;        //r12: mm5=mm7+mm5 red

                punpcklbw mm1,mm2;      //mm1=00a3 00a2 00a1 00a0
                psrlw   mm5,5;          //r13: mm5=mm5>>5 red

                psllw   mm5,11;         //r14: mm5=mm5<<10 red

                por     mm0,mm5;        //mm0=0rgb 0rgb 0rgb 0rgb
                sub     ebx,4;          //polished off 4 pixels

                movq    [edi-8],mm0;    //dst=0rgb 0rgb 0rgb 0rgb
                jmp     loopqword;      //go back to start

copyback:
                movq [edi],mm4;         //copy source to destination
leavefront:
                add     edi,8;                  //advance destination by 4 pixels
                add     eax,4;          //advance alpha by 4
                add     esi,8;          //advance source by 4 pixels
                sub     ebx,4;          //decrease pixel count by 4
                jmp     primeloop;

checkback:
                test    ebx,0xFF;       //check if 0 pixels left
                jz      nextline;       //done with this span

//backalign:    //work out back end pixels
                movq    mm5,[edi];      //g2: mm5=dst3 dst2 dst1 dst0
                psrlw   mm1,2;          //mm1=a?>>2 nuke out lower 2 bits

                movq    mm7,MASKSHIFTG; //g3: mm7=shift 1 bit green mask
                psrlw   mm4,1;          //g3a: move src green down by 1 so that we won't overflow

                movq    mm0,mm1;        //mm0=00a3 00a2 00a1 00a0
                psrlw   mm5,1;          //g3b: move dst green down by 1 so that we won't overflow

                psrlw   mm1,1;          //mm1=a?>>1 nuke out lower 1 bits
                pand    mm4,mm7;        //g5: mm4=sg3 sg2 sg1 sg0

                movq    mm2,SIXONES;//g4: mm2=63
                pand    mm5,mm7;        //g7: mm5=dg3 dg2 dg1 dg0

                movq    mm3,[esi];      //b1: mm3=src3 src2 src1 src0
                psubsb  mm2,mm0;        //g6: mm2=63-a3 63-a2 63-a1 63-a0

                movq    mm7,MASKB;      //b2: mm7=BLUE MASK
                pmullw  mm4,mm0;        //g8: mm4=sg?*a?

                movq    mm0,[edi];      //b3: mm0=dst3 dst2 dst1 dst0
                pmullw  mm5,mm2;        //g9: mm5=dg?*(1-a?)

                movq    mm2,mm7;        //b4: mm2=fiveones
                pand    mm3,mm7;        //b4: mm3=sr3 sr2 sr1 sr0

                pmullw  mm3,mm1;        //b6: mm3=sb?*a?
                pand    mm0,mm7;        //b5: mm0=db3 db2 db1 db0

                movq    mm7,[esi];      //r1: mm7=src3 src2 src1 src0
                paddw   mm4,mm5;        //g10: mm4=sg?*a?+dg?*(1-a?)

                pand    mm7,MASKR;      //r2: mm7=sr3 sr2 sr1 sr0
                psubsb  mm2,mm1;        //b5a: mm2=31-a3 31-a2 31-a1 31-a0

                paddw   mm4,FIVETWELVE; //g11: mm4=(i+512) green
                pmullw  mm0,mm2;        //b7: mm0=db?*(1-a?)

                movq    mm5,mm4;        //g12: mm5=(i+512) green
                psrlw   mm7,11;         //r4: shift src red down to position 0

                psrlw   mm4,6;          //g13: mm4=(i+512)>>6

                paddw   mm4,mm5;        //g14: mm4=(i+512)+((i+512)>>6) green

                paddw   mm0,mm3;        //b8: mm0=sb?*a?+db?*(1-a?)

                movq    mm5,[edi];      //r3: mm5=dst3 dst2 dst1 dst0

                paddw   mm0,SIXTEEN;    //b9: mm0=(i+16) blue

                pand    mm5,MASKR;      //r5: mm5=dr3 dr2 dr1 dr0
                psrlw   mm4,5;          //g15: mm4=0?g0 0?g0 0?g0 0?g0 green

                movq    mm3,mm0;        //b10: mm3=(i+16) blue
                psrlw   mm0,5;          //b11: mm0=(i+16)>>5 blue

                psrlw   mm5,11;         //r6: shift dst red down to position 0
                paddw   mm0,mm3;        //b12: mm0=(i+16)+(i+16)>>5 blue

                psrlw   mm0,5;          //b13: mm0=000r 000r 000r 000r blue
                pmullw  mm7,mm1;        //mm7=sr?*a?

                pand    mm4,MASKG;      //g16: mm4=00g0 00g0 00g0 00g0 green
                pmullw  mm5,mm2;        //r7: mm5=dr?*(31-a?)

                por     mm0,mm4;        //mm0=00gb 00gb 00gb 00gb
                add     eax,4;          //move to next 4 alphas

                //stall

                paddw   mm5,mm7;        //r8: mm5=sr?*a?+dr?*(31-a?)

                paddw   mm5,SIXTEEN;    //r9: mm5=(i+16) red

                movq    mm7,mm5;        //r10: mm7=(i+16) red
                psrlw   mm5,5;          //r11: mm5=(i+16)>>5 red

                paddw   mm5,mm7;        //r12: mm5=(i+16)+((i+16)>>5) red

                psrlw   mm5,5;          //r13: mm5=(i+16)+((i+16)>>5)>>5 red

                psllw   mm5,11;         //r14: mm5=mm5<<10 red

                por     mm0,mm5;        //mm0=0rgb 0rgb 0rgb 0rgb
                test    ebx,2;          //check if there are 2 pixels

                jz      oneendpixel;    //goto one pixel if that's it

                movd    [edi],mm0;      //dst=0000 0000 0rgb 0rgb
                psrlq   mm0,32;         //mm0>>32

                add     edi,4;          //edi=edi+4
                sub     ebx,2;          //saved 2 pixels

                jz      nextline;       //all done goto next line
oneendpixel:    //work on last pixel
                movd    edx,mm0;        //edx=0rgb

                mov     [edi],dx;       //dst=0rgb

nextline:       //goto next line
                dec     ecx;                            //nuke one line
                jz      done;                           //all done

                mov     eax,lpLinearAlpBp;      //alpha
                mov     esi,lpLinearSrcBp;      //src

                mov     edi,lpLinearDstBp;      //dst
                add     eax,iAlpPitch;  //inc alpha ptr by 1 line

                add     esi,iSrcPitch;  //inc src ptr by 1 line
                add     edi,iDstPitch;  //inc dst ptr by 1 line

                mov     lpLinearAlpBp,eax;      //save new alpha base ptr
                mov     ebx,iDstW;      //ebx=span width to copy

                mov     lpLinearSrcBp,esi;      //save new src base ptr
                mov     lpLinearDstBp,edi;      //save new dst base ptr

                jmp     primeloop;      //start the next span
done:
                emms
        }
#elif defined (__GNUC__) && defined (__i386__)
        /* Mask for isolating the red, green, and blue components */
        static int64_t MASKB=0x001F001F001F001FLL;
        static int64_t MASKG=0x07E007E007E007E0LL;
        static int64_t MASKSHIFTG=0x03F003F003F003F0LL;
        static int64_t MASKR=0xF800F800F800F800LL;

        /* constants used by the integer alpha blending equation */
        static int64_t SIXTEEN=0x0010001000100010LL;
        static int64_t FIVETWELVE=0x0200020002000200LL;
        static int64_t SIXONES=0x003F003F003F003FLL;

        /* We use ebp to replace ebx when using PIC (avoid killing the GOT),
           so avoid stack variables. */
#ifdef PIC
        static unsigned char *lpLinearDstBp;
        lpLinearDstBp=(iDstX<<1)+(iDstY*iDstPitch)+lpDst; /*base pointer for linear destination*/
        static unsigned char *lpLinearSrcBp;
        lpLinearSrcBp=(iSrcX<<1)+(iSrcY*iSrcPitch)+lpSrc; /*base pointer for linear source*/
        static unsigned char *lpLinearAlpBp;
        lpLinearAlpBp=iSrcX+(iSrcY*iAlpPitch)+lpAlpha; /*base pointer for linear alpha*/

        static unsigned int iDstWStatic;
        iDstWStatic = iDstW;
        static unsigned int iAlpPitchStatic;
        iAlpPitchStatic = iAlpPitch;
        static unsigned int iSrcPitchStatic;
        iSrcPitchStatic = iSrcPitch;
        static unsigned int iDstPitchStatic;
        iDstPitchStatic = iDstPitch;
#else
        unsigned char *lpLinearDstBp=(iDstX<<1)+(iDstY*iDstPitch)+lpDst; /*base pointer for linear destination*/
        unsigned char *lpLinearSrcBp=(iSrcX<<1)+(iSrcY*iSrcPitch)+lpSrc; /*base pointer for linear source*/
        unsigned char *lpLinearAlpBp=iSrcX+(iSrcY*iAlpPitch)+lpAlpha; /*base pointer for linear alpha*/
#endif
        __asm__ __volatile__ (
#ifdef PIC
            "    pushl %%ebp\n"
#else
            "    pushl %%ebx\n"
#endif

            "    movl    %0, %%esi\n"      //src
            "    movl    %1, %%edi\n"      //dst

            "    movl    %2, %%eax\n"      //alpha
            //"    movl    %3, %%ecx\n"      //ecx=number of lines to copy

#ifdef PIC
            "    movl    %4, %%ebp\n"      //ebp=span width to copy
#else
            "    movl    %4, %%ebx\n"      //ebx=span width to copy
#endif
            "    testl   $6, %%esi\n"          //check if source address is qword aligned
                                                //since addr coming in is always word aligned(16bit)
            "    jnz     ablend_565_done\n"     //if not qword aligned we don't do anything

            "ablend_565_primeloop:\n"
            "    movd    (%%eax), %%mm1\n"      //mm1=00 00 00 00 a3 a2 a1 a0
            "    pxor    %%mm2, %%mm2\n"        //mm2=0;

            "    movq    (%%esi), %%mm4\n"      //g1: mm4=src3 src2 src1 src0
            "    punpcklbw %%mm2, %%mm1\n"      //mm1=00a3 00a2 00a1 00a0

            "ablend_565_loopqword:\n"
            "    movl    (%%eax), %%edx\n"

#ifdef PIC
            "    testl   $0xFFFFFFFC, %%ebp\n"   //check if only 3 pixels left
#else
            "    testl   $0xFFFFFFFC, %%ebx\n"   //check if only 3 pixels left
#endif
            "    jz      ablend_565_checkback\n" //3 or less pixels left

                 //early out tests
            "    cmpl    $0xffffffff, %%edx\n"  //test for alpha value of 1
            "    je      ablend_565_copyback\n" //if 1's copy the source pixels to the destination

            "    testl   $0xffffffff, %%edx\n"    //test for alpha value of 0
            "    jz      ablend_565_leavefront\n" //if so go to the next 4 pixels

                 //the alpha blend starts
                 //green
                 //i=a*sg+(63-a)*dg
                 //i=(i+32)+((i+32)>>6)>>6
                 //red
                 //i=a*sr+(31-a)*dr
                 //i=(i+16)+((i+16)>>5)>>5
            "    movq    (%%edi), %%mm5\n"      //g2: mm5=dst3 dst2 dst1 dst0
            "    psrlw   $2, %%mm1\n"          //mm1=a?>>2 nuke out lower 2 bits

            "    movq    %10, %%mm7\n"         //g3: mm7=1 bit shifted green mask
            "    psrlw   $1, %%mm4\n"          //g3a: move src green down by 1 so that we won't overflow

            "    movq    %%mm1, %%mm0\n"        //mm0=00a3 00a2 00a1 00a0
            "    psrlw   $1, %%mm5\n"          //g3b: move dst green down by 1 so that we won't overflow

            "    psrlw   $1, %%mm1\n"          //mm1=a?>>1 nuke out lower 1 bits
            "    pand    %%mm7, %%mm4\n"        //g5: mm4=sg3 sg2 sg1 sg0

            "    movq    %14, %%mm2\n"          //g4: mm2=63
            "    pand    %%mm7, %%mm5\n"        //g7: mm5=dg3 dg2 dg1 dg0

            "    movq    (%%esi), %%mm3\n"      //b1: mm3=src3 src2 src1 src0
            "    psubsb  %%mm0, %%mm2\n"        //g6: mm2=63-a3 63-a2 63-a1 63-a0

            "    movq    %8, %%mm7\n"           //b2: mm7=BLUE MASK
            "    pmullw  %%mm0, %%mm4\n"        //g8: mm4=sg?*a?

            "    movq    (%%edi), %%mm0\n"      //b3: mm0=dst3 dst2 dst1 dst0
            "    pmullw  %%mm2, %%mm5\n"        //g9: mm5=dg?*(1-a?)

            "    movq    %%mm7, %%mm2\n"        //b4: mm2=fiveones
            "    pand    %%mm7, %%mm3\n"        //b4: mm3=sb3 sb2 sb1 sb0

            "    pmullw  %%mm1, %%mm3\n"        //b6: mm3=sb?*a?
            "    pand    %%mm7, %%mm0\n"        //b5: mm0=db3 db2 db1 db0

            "    movq    (%%esi), %%mm7\n"      //r1: mm7=src3 src2 src1 src0
            "    paddw   %%mm5, %%mm4\n"        //g10: mm4=sg?*a?+dg?*(1-a?)

            "    pand    %11, %%mm7\n"          //r2: mm7=sr3 sr2 sr1 sr0
            "    psubsb  %%mm1, %%mm2\n"        //b5a: mm2=31-a3 31-a2 31-a1 31-a0

            "    paddw   %13, %%mm4\n"          //g11: mm4=(mm4+512) green
            "    pmullw  %%mm2, %%mm0\n"        //b7: mm0=db?*(1-a?)

            "    movq    %%mm4, %%mm5\n"        //g12: mm5=mm4 green
            "    psrlw   $11, %%mm7\n"         //r4: shift src red down to position 0

            "    psrlw   $6, %%mm4\n"          //g13: mm4=mm4>>6

            "    paddw   %%mm5, %%mm4\n"        //g14: mm4=mm4+mm5 green

            "    paddw   %%mm3, %%mm0\n"        //b8: mm0=sb?*a?+db?*(1-a?)

            "    movq    (%%edi), %%mm5\n"      //r3: mm5=dst3 dst2 dst1 dst0

            "    paddw   %12, %%mm0\n"         //b9: mm0=(mm0+16) blue

            "    pand    %11, %%mm5\n"         //r5: mm5=dr3 dr2 dr1 dr0
            "    psrlw   $5, %%mm4\n"          //g15: mm4=0?g0 0?g0 0?g0 0?g0 green

            "    movq    %%mm0, %%mm3\n"        //b10: mm3=mm0 blue
            "    psrlw   $5, %%mm0\n"          //b11: mm0=mm0>>5 blue

            "    psrlw   $11, %%mm5\n"         //r6: shift dst red down to position 0
            "    paddw   %%mm3, %%mm0\n"        //b12: mm0=mm3+mm0 blue

            "    psrlw   $5, %%mm0\n"          //b13: mm0=000b 000b 000b 000b blue
            "    pmullw  %%mm1, %%mm7\n"        //mm7=sr?*a?

            "    pand    %9, %%mm4\n"           //g16: mm4=00g0 00g0 00g0 00g0 green
            "    pmullw  %%mm2, %%mm5\n"        //r7: mm5=dr?*(31-a?)

            "    por     %%mm4, %%mm0\n"        //mm0=00gb 00gb 00gb 00gb
            "    addl    $4, %%eax\n"          //move to next 4 alphas

            "    addl    $8, %%esi\n"          //move to next 4 pixels in src
            "    addl    $8, %%edi\n"          //move to next 4 pixels in dst

            "    movd    (%%eax), %%mm1\n"      //mm1=00 00 00 00 a3 a2 a1 a0
            "    paddw   %%mm7, %%mm5\n"        //r8: mm5=sr?*a?+dr?*(31-a?)

            "    paddw   %12, %%mm5\n"          //r9: mm5=(mm5+16) red
            "    pxor    %%mm2, %%mm2\n"        //mm2=0;

            "    movq    %%mm5, %%mm7\n"        //r10: mm7=mm5 red
            "    psrlw   $5, %%mm5\n"          //r11: mm5=mm5>>5 red

            "    movq    (%%esi), %%mm4\n"      //g1: mm4=src3 src2 src1 src0
            "    paddw   %%mm7, %%mm5\n"        //r12: mm5=mm7+mm5 red

            "    punpcklbw %%mm2, %%mm1\n"      //mm1=00a3 00a2 00a1 00a0
            "    psrlw   $5, %%mm5\n"          //r13: mm5=mm5>>5 red

            "    psllw   $11, %%mm5\n"         //r14: mm5=mm5<<10 red

            "    por     %%mm5, %%mm0\n"        //mm0=0rgb 0rgb 0rgb 0rgb
#ifdef PIC
            "    subl    $4, %%ebp\n"          //polished off 4 pixels
#else
            "    subl    $4, %%ebx\n"          //polished off 4 pixels
#endif

            "    movq    %%mm0, -8(%%edi)\n"    //dst=0rgb 0rgb 0rgb 0rgb
            "    jmp     ablend_565_loopqword\n" //go back to start

            "ablend_565_copyback:\n"
            "    movq %%mm4, (%%edi)\n"         //copy source to destination
            "ablend_565_leavefront:\n"
            "    addl    $8, %%edi\n"                  //advance destination by 4 pixels
            "    addl    $4, %%eax\n"          //advance alpha by 4
            "    addl    $8, %%esi\n"          //advance source by 4 pixels
#ifdef PIC
            "    subl    $4, %%ebp\n"          //decrease pixel count by 4
#else
            "    subl    $4, %%ebx\n"          //decrease pixel count by 4
#endif
            "    jmp     ablend_565_primeloop\n"

            "ablend_565_checkback:\n"
#ifdef PIC
            "    testl   $0xFF, %%ebp\n"       //check if 0 pixels left
#else
            "    testl   $0xFF, %%ebx\n"       //check if 0 pixels left
#endif
            "    jz      ablend_565_nextline\n" //done with this span

            //"ablend_565_backalign:\n"    //work out back end pixels
            "    movq    (%%edi), %%mm5\n"      //g2: mm5=dst3 dst2 dst1 dst0
            "    psrlw   $2, %%mm1\n"          //mm1=a?>>2 nuke out lower 2 bits

            "    movq    %10, %%mm7\n"         //g3: mm7=shift 1 bit green mask
            "    psrlw   $1, %%mm4\n"          //g3a: move src green down by 1 so that we won't overflow

            "    movq    %%mm1, %%mm0\n"        //mm0=00a3 00a2 00a1 00a0
            "    psrlw   $1, %%mm5\n"          //g3b: move dst green down by 1 so that we won't overflow

            "    psrlw   $1, %%mm1\n"          //mm1=a?>>1 nuke out lower 1 bits
            "    pand    %%mm7, %%mm4\n"        //g5: mm4=sg3 sg2 sg1 sg0

            "    movq    %14, %%mm2\n"          //g4: mm2=63
            "    pand    %%mm7, %%mm5\n"        //g7: mm5=dg3 dg2 dg1 dg0

            "    movq    (%%esi), %%mm3\n"      //b1: mm3=src3 src2 src1 src0
            "    psubsb  %%mm0, %%mm2\n"        //g6: mm2=63-a3 63-a2 63-a1 63-a0

            "    movq    %8, %%mm7\n"           //b2: mm7=BLUE MASK
            "    pmullw  %%mm0, %%mm4\n"        //g8: mm4=sg?*a?

            "    movq    (%%edi), %%mm0\n"      //b3: mm0=dst3 dst2 dst1 dst0
            "    pmullw  %%mm2, %%mm5\n"        //g9: mm5=dg?*(1-a?)

            "    movq    %%mm7, %%mm2\n"        //b4: mm2=fiveones
            "    pand    %%mm7, %%mm3\n"        //b4: mm3=sr3 sr2 sr1 sr0

            "    pmullw  %%mm1, %%mm3\n"        //b6: mm3=sb?*a?
            "    pand    %%mm7, %%mm0\n"        //b5: mm0=db3 db2 db1 db0

            "    movq    (%%esi), %%mm7\n"      //r1: mm7=src3 src2 src1 src0
            "    paddw   %%mm5, %%mm4\n"        //g10: mm4=sg?*a?+dg?*(1-a?)

            "    pand    %11, %%mm7\n"          //r2: mm7=sr3 sr2 sr1 sr0
            "    psubsb  %%mm1, %%mm2\n"        //b5a: mm2=31-a3 31-a2 31-a1 31-a0

            "    paddw   %13, %%mm4\n"          //g11: mm4=(i+512) green
            "    pmullw  %%mm2, %%mm0\n"        //b7: mm0=db?*(1-a?)

            "    movq    %%mm4, %%mm5\n"        //g12: mm5=(i+512) green
            "    psrlw   $11, %%mm7\n"         //r4: shift src red down to position 0

            "    psrlw   $6, %%mm4\n"          //g13: mm4=(i+512)>>6

            "    paddw   %%mm5, %%mm4\n"        //g14: mm4=(i+512)+((i+512)>>6) green

            "    paddw   %%mm3, %%mm0\n"        //b8: mm0=sb?*a?+db?*(1-a?)

            "    movq    (%%edi), %%mm5\n"      //r3: mm5=dst3 dst2 dst1 dst0

            "    paddw   %12, %%mm0\n"         //b9: mm0=(i+16) blue

            "    pand    %11, %%mm5\n"         //r5: mm5=dr3 dr2 dr1 dr0
            "    psrlw   $5, %%mm4\n"          //g15: mm4=0?g0 0?g0 0?g0 0?g0 green

            "    movq    %%mm0, %%mm3\n"        //b10: mm3=(i+16) blue
            "    psrlw   $5, %%mm0\n"          //b11: mm0=(i+16)>>5 blue

            "    psrlw   $11, %%mm5\n"         //r6: shift dst red down to position 0
            "    paddw   %%mm3, %%mm0\n"        //b12: mm0=(i+16)+(i+16)>>5 blue

            "    psrlw   $5, %%mm0\n"          //b13: mm0=000r 000r 000r 000r blue
            "    pmullw  %%mm1, %%mm7\n"        //mm7=sr?*a?

            "    pand    %9, %%mm4\n"           //g16: mm4=00g0 00g0 00g0 00g0 green
            "    pmullw  %%mm2, %%mm5\n"        //r7: mm5=dr?*(31-a?)

            "    por     %%mm4, %%mm0\n"        //mm0=00gb 00gb 00gb 00gb
            "    addl    $4, %%eax\n"          //move to next 4 alphas

                 //stall

            "    paddw   %%mm7, %%mm5\n"        //r8: mm5=sr?*a?+dr?*(31-a?)

            "    paddw   %12, %%mm5\n"          //r9: mm5=(i+16) red

            "    movq    %%mm5, %%mm7\n"        //r10: mm7=(i+16) red
            "    psrlw   $5, %%mm5\n"          //r11: mm5=(i+16)>>5 red

            "    paddw   %%mm7, %%mm5\n"        //r12: mm5=(i+16)+((i+16)>>5) red

            "    psrlw   $5, %%mm5\n"          //r13: mm5=(i+16)+((i+16)>>5)>>5 red

            "    psllw   $11, %%mm5\n"         //r14: mm5=mm5<<10 red

            "    por     %%mm5, %%mm0\n"        //mm0=0rgb 0rgb 0rgb 0rgb
#ifdef PIC
            "    testl   $2, %%ebp\n"          //check if there are 2 pixels
#else
            "    testl   $2, %%ebx\n"          //check if there are 2 pixels
#endif

            "    jz      ablend_565_oneendpixel\n" //goto one pixel if that's it

            "    movd    %%mm0, (%%edi)\n"      //dst=0000 0000 0rgb 0rgb
            "    psrlq   $32, %%mm0\n"         //mm0>>32

            "    addl    $4, %%edi\n"          //edi=edi+4
#ifdef PIC
            "    subl    $2, %%ebp\n"          //saved 2 pixels
#else
            "    subl    $2, %%ebx\n"          //saved 2 pixels
#endif

            "    jz      ablend_565_nextline\n" //all done goto next line
            "ablend_565_oneendpixel:\n"    //work on last pixel
            "    movd    %%mm0, %%edx\n"        //edx=0rgb

            "    movw    %%dx, (%%edi)\n"       //dst=0rgb

            "ablend_565_nextline:\n"       //goto next line
            "    decl    %%ecx\n"                            //nuke one line
            "    jz      ablend_565_done\n"                  //all done

            "    movl    %2, %%eax\n"      //alpha
            "    movl    %0, %%esi\n"      //src

            "    movl    %1, %%edi\n"      //dst
            "    addl    %5, %%eax\n"  //inc alpha ptr by 1 line

            "    addl    %6, %%esi\n"  //inc src ptr by 1 line
            "    addl    %7, %%edi\n"  //inc dst ptr by 1 line

            "    movl    %%eax, %2\n"      //save new alpha base ptr
#ifdef PIC
            "    movl    %4, %%ebp\n"      //ebp=span width to copy
#else
            "    movl    %4, %%ebx\n"      //ebx=span width to copy
#endif

            "    movl    %%esi, %0\n"      //save new src base ptr
            "    movl    %%edi, %1\n"      //save new dst base ptr

            "    jmp     ablend_565_primeloop\n" //start the next span
            "ablend_565_done:\n"
            "    emms\n"

#ifdef PIC
            "    popl %%ebp\n"
#else
            "    popl %%ebx\n"
#endif
            :
            : "m" (lpLinearSrcBp), "m" (lpLinearDstBp), "m" (lpLinearAlpBp),
              "c" (iDstH),
#ifdef PIC
              "m" (iDstWStatic),
              "m" (iAlpPitchStatic), "m" (iSrcPitchStatic),
              "m" (iDstPitchStatic),
#else
              "m" (iDstW),
              "m" (iAlpPitch), "m" (iSrcPitch), "m" (iDstPitch),
#endif
              "m" (MASKB), "m" (MASKG), "m" (MASKSHIFTG), "m" (MASKR),
              "m" (SIXTEEN), "m" (FIVETWELVE), "m" (SIXONES)
            : "eax", "edx", "esi", "edi" );
#endif
}

void ablend_555(unsigned char *lpAlpha,unsigned int iAlpPitch,
                unsigned char *lpSrc,unsigned int iSrcX, unsigned int iSrcY,
                unsigned int iSrcPitch, unsigned char *lpDst,
                unsigned int iDstX,     unsigned int iDstY,
                unsigned int iDstW,     unsigned int iDstH,
                unsigned int iDstPitch)
{
#if defined (_MSC_VER)
        //Mask for isolating the red, green, and blue components
        static int64_t MASKR=0x001F001F001F001F;
        static int64_t MASKG=0x03E003E003E003E0;
        static int64_t MASKB=0x7C007C007C007C00;

        //constants used by the integer alpha blending equation
        static int64_t SIXTEEN=0x0010001000100010;
        static int64_t FIVETWELVE=0x0200020002000200;

        unsigned char *lpLinearDstBp=(iDstX<<1)+(iDstY*iDstPitch)+lpDst;
        unsigned char *lpLinearSrcBp=(iSrcX<<1)+(iSrcY*iSrcPitch)+lpSrc;
        unsigned char *lpLinearAlpBp=iSrcX+(iSrcY*iAlpPitch)+lpAlpha;

        _asm
        {
                mov     esi,lpLinearSrcBp;      //src
                mov     edi,lpLinearDstBp;      //dst

                mov     eax,lpLinearAlpBp;      //alpha
                mov     ecx,iDstH;      //ecx=number of lines to copy

                mov     ebx,iDstW;      //ebx=span width to copy
                test    esi,6;          //check if source address is qword aligned
                                                        //since addr coming in is always word aligned(16bit)
                jnz     done;           //if not qword aligned we don't do anything

primeloop:
                movd    mm1,[eax];      //mm1=00 00 00 00 a3 a2 a1 a0
                pxor    mm2,mm2;        //mm2=2;

                movq    mm4,[esi];      //g1: mm4=src3 src2 src1 src0
                punpcklbw mm1,mm2;      //mm1=00a3 00a2 00a1 00a0

loopqword:
                mov     edx,[eax];

                test    ebx,0xFFFFFFFC; //check if only 3 pixels left
                jz      checkback;      //3 or less pixels left

                //early out tests
                cmp     edx,0xffffffff; //test for alpha value of 1
                je       copyback;      //if 1's copy the source pixels to the destination

                test    edx,0xffffffff; //test for alpha value of 0
                jz      leavefront;     //if so go to the next 4 pixels

                //the alpha blend starts
                //i=a*sr+(31-a)*dr;
                //i=(i+16)+((i+16)>>5)>>5;
                movq    mm5,[edi];      //g2: mm5=dst3 dst2 dst1 dst0
                psrlw   mm1,3;          //mm1=a?>>3 nuke out lower 3 bits

                movq    mm7,MASKG;      //g3: mm7=green mask
                movq    mm0,mm1;        //mm0=00a3 00a2 00a1 00a0

                movq    mm2,MASKR;      //g4: mm2=31
                pand    mm4,mm7;        //g5: mm4=sg3 sg2 sg1 sg0

                psubsb  mm2,mm0;        //g6: mm2=31-a3 31-a2 31-a1 31-a0
                pand    mm5,mm7;        //g7: mm5=dg3 dg2 dg1 dg0

                movq    mm3,[esi];      //b1: mm3=src3 src2 src1 src0
                pmullw  mm4,mm1;        //g8: mm4=sg?*a?

                movq    mm7,MASKR;      //b2: mm7=blue mask
                pmullw  mm5,mm2;        //g9: mm5=dg?*(1-a?)

                movq    mm0,[edi];      //b3: mm0=dst3 dst2 dst1 dst0
                pand    mm3,mm7;        //b4: mm3=sb3 sb2 sb1 sb0

                pand    mm0,mm7;        //b5: mm0=db3 db2 db1 db0
                pmullw  mm3,mm1;        //b6: mm3=sb?*a?

                movq    mm7,[esi];      //r1: mm7=src3 src2 src1 src0
                paddw   mm4,mm5;        //g10: mm4=sg?*a?+dg?*(1-a?)

                paddw   mm4,FIVETWELVE; //g11: mm4=(mm4+512) green
                pmullw  mm0,mm2;        //b7: mm0=db?*(1-a?)

                pand    mm7,MASKB;      //r2: mm7=sr3 sr2 sr1 sr0
                movq    mm5,mm4;        //g12: mm5=mm4 green

                psrlw   mm4,5;          //g13: mm4=mm4>>5

                paddw   mm4,mm5;        //g14: mm4=mm5+mm4 green

                movq    mm5,[edi];      //r3: mm5=dst3 dst2 dst1 dst0
                paddw   mm0,mm3;        //b8: mm0=sb?*a?+db?*(1-a?)

                paddw   mm0,SIXTEEN;    //b9: mm0=(mm0+16) blue
                psrlw   mm7,10;         //r4: shift src red down to position 0

                pand    mm5,MASKB;      //r5: mm5=dr3 dr2 dr1 dr0
                psrlw   mm4,5;          //g15: mm4=0?g0 0?g0 0?g0 0?g0 green

                movq    mm3,mm0;        //b10: mm3=mm0 blue
                psrlw   mm0,5;          //b11: mm0=mm0>>5 blue

                psrlw   mm5,10;         //r6: shift dst red down to position 0
                paddw   mm0,mm3;        //b12: mm0=mm3+mm0 blue

                psrlw   mm0,5;          //b13: mm0=000b 000b 000b 000b blue
                pmullw  mm7,mm1;        //mm7=sr?*a?

                pand    mm4,MASKG;      //g16: mm4=00g0 00g0 00g0 00g0 green
                pmullw  mm5,mm2;        //r7: mm5=dr?*(31-a?)

                por     mm0,mm4;        //mm0=00gb 00gb 00gb 00gb
                add     eax,4;          //move to next 4 alphas

                add     esi,8;          //move to next 4 pixels in src
                add     edi,8;          //move to next 4 pixels in dst

                movd    mm1,[eax];      //mm1=00 00 00 00 a3 a2 a1 a0
                paddw   mm5,mm7;        //r8: mm5=sr?*a?+dr?*(31-a?)

                paddw   mm5,SIXTEEN;    //r9: mm5=(mm5+16) red
                pxor    mm2,mm2;        //mm0=0;

                movq    mm7,mm5;        //r10: mm7=mm5 red
                psrlw   mm5,5;          //r11: mm5=mm5>>5 red

                movq    mm4,[esi];      //g1: mm4=src3 src2 src1 src0
                paddw   mm5,mm7;        //r12: mm5=mm7+mm5 red

                punpcklbw mm1,mm2;      //mm1=00a3 00a2 00a1 00a0
                psrlw   mm5,5;          //r13: mm5=mm5>>5 red

                psllw   mm5,10;         //r14: mm5=mm5<<10 red

                por     mm0,mm5;        //mm0=0rgb 0rgb 0rgb 0rgb
                sub     ebx,4;          //polished off 4 pixels

                movq    [edi-8],mm0;    //dst=0rgb 0rgb 0rgb 0rgb
                jmp     loopqword;      //go back to start

copyback:
                movq [edi],mm4;         //copy source to destination
leavefront:
                add     edi,8;          //advance destination by 4 pixels
                add     eax,4;          //advance alpha by 4
                add     esi,8;          //advance source by 4 pixels
                sub     ebx,4;          //decrease pixel count by 4
                jmp     primeloop;

checkback:
                test    ebx,0xFF;       //check if 0 pixels left
                jz      nextline;       //done with this span

//backalign:    //work out back end pixels
                movq    mm5,[edi];      //g2: mm5=dst3 dst2 dst1 dst0
                psrlw   mm1,3;          //mm1=a?>>3 nuke out lower 3 bits

                movq    mm7,MASKG;      //g3: mm7=green mask
                movq    mm0,mm1;        //mm0=00a3 00a2 00a1 00a0

                movq    mm2,MASKR;      //g4: mm2=31 mask
                pand    mm4,mm7;        //g5: mm4=sg3 sg2 sg1 sg0

                psubsb  mm2,mm0;        //g6: mm2=31-a3 31-a2 31-a1 31-a0
                pand    mm5,mm7;        //g7: mm5=dg3 dg2 dg1 dg0

                movq    mm3,[esi];      //b1: mm3=src3 src2 src1 src0
                pmullw  mm4,mm1;        //g8: mm4=sg?*a?

                movq    mm7,MASKR;      //b2: mm7=blue mask
                pmullw  mm5,mm2;        //g9: mm5=dg?*(1-a?)

                movq    mm0,[edi];      //b3: mm0=dst3 dst2 dst1 dst0
                pand    mm3,mm7;        //b4: mm3=sb3 sb2 sb1 sb0

                pand    mm0,mm7;        //b5: mm0=db3 db2 db1 db0
                pmullw  mm3,mm1;        //b6: mm3=sb?*a?

                movq    mm7,[esi];      //r1: mm7=src3 src2 src1 src0
                paddw   mm4,mm5;        //g10: mm4=sg?*a?+dg?*(1-a?)

                paddw   mm4,FIVETWELVE; //g11: mm4=(mm4+512) green
                pmullw  mm0,mm2;        //b7: mm0=db?*(1-a?)

                pand    mm7,MASKB;      //r2: mm7=sr3 sr2 sr1 sr0
                movq    mm5,mm4;        //g12: mm5=mm4 green

                psrlw   mm4,5;          //g13: mm4=mm4>>5

                paddw   mm4,mm5;        //g14: mm4=mm4+mm5 green

                movq    mm5,[edi];      //r3: mm5=dst3 dst2 dst1 dst0
                paddw   mm0,mm3;        //b8: mm0=sb?*a?+db?*(1-a?)

                paddw   mm0,SIXTEEN;    //b9: mm0=mm0 blue
                psrlw   mm7,10;         //r4: shift src red down to position 0

                pand    mm5,MASKB;      //r5: mm5=dr3 dr2 dr1 dr0
                psrlw   mm4,5;          //g15: mm4=0?g0 0?g0 0?g0 0?g0 green

                movq    mm3,mm0;        //b10: mm3=mm0 blue
                psrlw   mm0,5;          //b11: mm0=mm0>>5 blue

                psrlw   mm5,10;         //r6: shift dst red down to position 0
                paddw   mm0,mm3;        //b12: mm0=mm0+mm3 blue

                psrlw   mm0,5;          //b13: mm0=000b 000b 000b 000b blue
                pmullw  mm7,mm1;        //mm7=sr?*a?

                pand    mm4,MASKG;      //g16: mm4=00g0 00g0 00g0 00g0 green
                pmullw  mm5,mm2;        //r7: mm5=dr?*(31-a?)

                por     mm0,mm4;        //mm0=00gb 00gb 00gb 00gb

                //stall

                paddw   mm5,mm7;        //r8: mm5=sr?*a?+dr?*(31-a?)

                paddw   mm5,SIXTEEN;    //r9: mm5=mm5 red

                movq    mm7,mm5;        //r10: mm7=mm5 red
                psrlw   mm5,5;          //r11: mm5=mm5>>5 red

                paddw   mm5,mm7;        //r12: mm5=mm5+mm7 red

                psrlw   mm5,5;          //r13: mm5=mm5>>5 red

                psllw   mm5,10;         //r14: mm5=mm5<<10 red

                por     mm0,mm5;        //mm0=0rgb 0rgb 0rgb 0rgb

                test    ebx,2;          //check if there are 2 pixels
                jz      oneendpixel;    //goto one pixel if that's it

                movd    [edi],mm0;      //dst=0000 0000 0rgb 0rgb
                psrlq   mm0,32;         //mm0>>32

                add     edi,4;          //edi=edi+4
                sub     ebx,2;          //saved 2 pixels

                jz      nextline;       //all done goto next line
oneendpixel:    //work on last pixel
                movd    edx,mm0;        //edx=0rgb

                mov     [edi],dx;       //dst=0rgb

nextline:       //goto next line
                dec     ecx;            //nuke one line
                jz      done;           //all done

                mov     eax,lpLinearAlpBp;      //alpha
                mov     esi,lpLinearSrcBp;      //src

                mov     edi,lpLinearDstBp;      //dst
                add     eax,iAlpPitch;  //inc alpha ptr by 1 line

                add     esi,iSrcPitch;  //inc src ptr by 1 line
                add     edi,iDstPitch;  //inc dst ptr by 1 line

                mov     lpLinearAlpBp,eax;      //save new alpha base ptr
                mov     ebx,iDstW;      //ebx=span width to copy

                mov     lpLinearSrcBp,esi;      //save new src base ptr
                mov     lpLinearDstBp,edi;      //save new dst base ptr

                jmp     primeloop;      //start the next span
done:
                emms
        }
#elif defined (__GNUC__) && defined (__i386__)
        /* Mask for isolating the red, green, and blue components */
        static int64_t MASKR=0x001F001F001F001FLL;
        static int64_t MASKG=0x03E003E003E003E0LL;
        static int64_t MASKB=0x7C007C007C007C00LL;

        /* constants used by the integer alpha blending equation */
        static int64_t SIXTEEN=0x0010001000100010LL;
        static int64_t FIVETWELVE=0x0200020002000200LL;

        /* We use ebp to replace ebx when using PIC (avoid killing the GOT),
           so avoid stack variables. */
#ifdef PIC
        static unsigned char *lpLinearDstBp;
        lpLinearDstBp=(iDstX<<1)+(iDstY*iDstPitch)+lpDst; /*base pointer for linear destination*/
        static unsigned char *lpLinearSrcBp;
        lpLinearSrcBp=(iSrcX<<1)+(iSrcY*iSrcPitch)+lpSrc; /*base pointer for linear source*/
        static unsigned char *lpLinearAlpBp;
        lpLinearAlpBp=iSrcX+(iSrcY*iAlpPitch)+lpAlpha; /*base pointer for linear alpha*/

        static unsigned int iDstWStatic;
        iDstWStatic = iDstW;
        static unsigned int iAlpPitchStatic;
        iAlpPitchStatic = iAlpPitch;
        static unsigned int iSrcPitchStatic;
        iSrcPitchStatic = iSrcPitch;
        static unsigned int iDstPitchStatic;
        iDstPitchStatic = iDstPitch;
#else
        unsigned char *lpLinearDstBp=(iDstX<<1)+(iDstY*iDstPitch)+lpDst; /*base pointer for linear destination*/
        unsigned char *lpLinearSrcBp=(iSrcX<<1)+(iSrcY*iSrcPitch)+lpSrc; /*base pointer for linear source*/
        unsigned char *lpLinearAlpBp=iSrcX+(iSrcY*iAlpPitch)+lpAlpha; /*base pointer for linear alpha*/
#endif
        __asm__ __volatile__ (
#ifdef PIC
            "    pushl %%ebp\n"
#else
            "    pushl %%ebx\n"
#endif

            "    movl    %0, %%esi\n"      //src
            "    movl    %1, %%edi\n"      //dst

            "    movl    %2, %%eax\n"      //alpha
            //"    movl    %3, %%ecx\n"      //ecx=number of lines to copy

#ifdef PIC
            "    movl    %4, %%ebp\n"      //ebp=span width to copy
#else
            "    movl    %4, %%ebx\n"      //ebx=span width to copy
#endif
            "    testl   $6, %%esi\n"       //check if source address is qword aligned
                                            //since addr coming in is always word aligned(16bit)
            "    jnz     ablend_555_done\n" //if not qword aligned we don't do anything

            "ablend_555_primeloop:\n"
            "    movd    (%%eax), %%mm1\n"      //mm1=00 00 00 00 a3 a2 a1 a0
            "    pxor    %%mm2, %%mm2\n"        //mm2=2;

            "    movq    (%%esi), %%mm4\n"      //g1: mm4=src3 src2 src1 src0
            "    punpcklbw %%mm2, %%mm1\n"      //mm1=00a3 00a2 00a1 00a0

            "ablend_555_loopqword:\n"
            "    movl    (%%eax), %%edx\n"

#ifdef PIC
            "    testl   $0xFFFFFFFC, %%ebp\n" //check if only 3 pixels left
#else
            "    testl   $0xFFFFFFFC, %%ebx\n" //check if only 3 pixels left
#endif
            "    jz      ablend_555_checkback\n"      //3 or less pixels left

                 //early out tests"
            "    cmpl    $0xffffffff, %%edx\n" //test for alpha value of 1
            "    je      ablend_555_copyback\n"      //if 1's copy the source pixels to the destination

            "    testl   $0xffffffff, %%edx\n" //test for alpha value of 0
            "    jz      ablend_555_leavefront\n"     //if so go to the next 4 pixels

                 //the alpha blend starts"
                 //i=a*sr+(31-a)*dr"
                 //i=(i+16)+((i+16)>>5)>>5"
            "    movq    (%%edi), %%mm5\n"      //g2: mm5=dst3 dst2 dst1 dst0
            "    psrlw   $3, %%mm1\n"          //mm1=a?>>3 nuke out lower 3 bits

            "    movq    %9, %%mm7\n"           //g3: mm7=green mask
            "    movq    %%mm1, %%mm0\n"        //mm0=00a3 00a2 00a1 00a0

            "    movq    %8, %%mm2\n"           //g4: mm2=31
            "    pand    %%mm7, %%mm4\n"        //g5: mm4=sg3 sg2 sg1 sg0

            "    psubsb  %%mm0, %%mm2\n"        //g6: mm2=31-a3 31-a2 31-a1 31-a0
            "    pand    %%mm7, %%mm5\n"        //g7: mm5=dg3 dg2 dg1 dg0

            "    movq    (%%esi), %%mm3\n"      //b1: mm3=src3 src2 src1 src0
            "    pmullw  %%mm1, %%mm4\n"        //g8: mm4=sg?*a?

            "    movq    %8, %%mm7\n"           //b2: mm7=blue mask
            "    pmullw  %%mm2, %%mm5\n"        //g9: mm5=dg?*(1-a?)

            "    movq    (%%edi), %%mm0\n"      //b3: mm0=dst3 dst2 dst1 dst0
            "    pand    %%mm7, %%mm3\n"        //b4: mm3=sb3 sb2 sb1 sb0

            "    pand    %%mm7, %%mm0\n"        //b5: mm0=db3 db2 db1 db0
            "    pmullw  %%mm1, %%mm3\n"        //b6: mm3=sb?*a?

            "    movq    (%%esi), %%mm7\n"      //r1: mm7=src3 src2 src1 src0
            "    paddw   %%mm5, %%mm4\n"        //g10: mm4=sg?*a?+dg?*(1-a?)

            "    paddw   %12, %%mm4\n"          //g11: mm4=(mm4+512) green
            "    pmullw  %%mm2, %%mm0\n"        //b7: mm0=db?*(1-a?)

            "    pand    %10, %%mm7\n"          //r2: mm7=sr3 sr2 sr1 sr0
            "    movq    %%mm4, %%mm5\n"        //g12: mm5=mm4 green

            "    psrlw   $5, %%mm4\n"          //g13: mm4=mm4>>5

            "    paddw   %%mm5, %%mm4\n"        //g14: mm4=mm5+mm4 green

            "    movq    (%%edi), %%mm5\n"      //r3: mm5=dst3 dst2 dst1 dst0
            "    paddw   %%mm3, %%mm0\n"        //b8: mm0=sb?*a?+db?*(1-a?)

            "    paddw   %11, %%mm0\n"         //b9: mm0=(mm0+16) blue
            "    psrlw   $10, %%mm7\n"         //r4: shift src red down to position 0

            "    pand    %10, %%mm5\n"         //r5: mm5=dr3 dr2 dr1 dr0
            "    psrlw   $5, %%mm4\n"          //g15: mm4=0?g0 0?g0 0?g0 0?g0 green

            "    movq    %%mm0, %%mm3\n"        //b10: mm3=mm0 blue
            "    psrlw   $5, %%mm0\n"          //b11: mm0=mm0>>5 blue

            "    psrlw   $10, %%mm5\n"         //r6: shift dst red down to position 0
            "    paddw   %%mm3, %%mm0\n"        //b12: mm0=mm3+mm0 blue

            "    psrlw   $5, %%mm0\n"          //b13: mm0=000b 000b 000b 000b blue
            "    pmullw  %%mm1, %%mm7\n"        //mm7=sr?*a?

            "    pand    %9, %%mm4\n"           //g16: mm4=00g0 00g0 00g0 00g0 green
            "    pmullw  %%mm2, %%mm5\n"        //r7: mm5=dr?*(31-a?)

            "    por     %%mm4, %%mm0\n"        //mm0=00gb 00gb 00gb 00gb
            "    addl    $4, %%eax\n"          //move to next 4 alphas

            "    addl    $8, %%esi\n"          //move to next 4 pixels in src
            "    addl    $8, %%edi\n"          //move to next 4 pixels in dst

            "    movd    (%%eax), %%mm1\n"      //mm1=00 00 00 00 a3 a2 a1 a0
            "    paddw   %%mm7, %%mm5\n"        //r8: mm5=sr?*a?+dr?*(31-a?)

            "    paddw   %11, %%mm5\n"          //r9: mm5=(mm5+16) red
            "    pxor    %%mm2, %%mm2\n"        //mm0=0;

            "    movq    %%mm5, %%mm7\n"        //r10: mm7=mm5 red
            "    psrlw   $5, %%mm5\n"          //r11: mm5=mm5>>5 red

            "    movq    (%%esi), %%mm4\n"      //g1: mm4=src3 src2 src1 src0
            "    paddw   %%mm7, %%mm5\n"        //r12: mm5=mm7+mm5 red

            "    punpcklbw %%mm2, %%mm1\n"      //mm1=00a3 00a2 00a1 00a0
            "    psrlw   $5, %%mm5\n"          //r13: mm5=mm5>>5 red

            "    psllw   $10, %%mm5\n"         //r14: mm5=mm5<<10 red

            "    por     %%mm5, %%mm0\n"        //mm0=0rgb 0rgb 0rgb 0rgb
#ifdef PIC
            "    subl    $4, %%ebp\n"          //polished off 4 pixels
#else
            "    subl    $4, %%ebx\n"          //polished off 4 pixels
#endif

            "    movq    %%mm0, -8(%%edi)\n"    //dst=0rgb 0rgb 0rgb 0rgb
            "    jmp     ablend_555_loopqword\n" //go back to start

            "ablend_555_copyback:\n"
            "    movq %%mm4, (%%edi)\n"         //copy source to destination
            "ablend_555_leavefront:\n"
            "    addl    $8, %%edi\n"          //advance destination by 4 pixels
            "    addl    $4, %%eax\n"          //advance alpha by 4
            "    addl    $8, %%esi\n"          //advance source by 4 pixels
#ifdef PIC
            "    subl    $4, %%ebp\n"          //decrease pixel count by 4
#else
            "    subl    $4, %%ebx\n"          //decrease pixel count by 4
#endif
            "    jmp     ablend_555_primeloop\n"

            "ablend_555_checkback:\n"
#ifdef PIC
            "    testl   $0xFF, %%ebp\n"       //check if 0 pixels left
#else
            "    testl   $0xFF, %%ebx\n"       //check if 0 pixels left
#endif
            "    jz      ablend_555_nextline\n" //done with this span

            //"ablend_555_backalign:\n"    //work out back end pixels
            "    movq    (%%edi), %%mm5\n"      //g2: mm5=dst3 dst2 dst1 dst0
            "    psrlw   $3, %%mm1\n"          //mm1=a?>>3 nuke out lower 3 bits

            "    movq    %9, %%mm7\n"           //g3: mm7=green mask
            "    movq    %%mm1, %%mm0\n"        //mm0=00a3 00a2 00a1 00a0

            "    movq    %8, %%mm2\n"           //g4: mm2=31 mask
            "    pand    %%mm7, %%mm4\n"        //g5: mm4=sg3 sg2 sg1 sg0

            "    psubsb  %%mm0, %%mm2\n"        //g6: mm2=31-a3 31-a2 31-a1 31-a0
            "    pand    %%mm7, %%mm5\n"        //g7: mm5=dg3 dg2 dg1 dg0

            "    movq    (%%esi), %%mm3\n"      //b1: mm3=src3 src2 src1 src0
            "    pmullw  %%mm1, %%mm4\n"        //g8: mm4=sg?*a?

            "    movq    %8, %%mm7\n"           //b2: mm7=blue mask
            "    pmullw  %%mm2, %%mm5\n"        //g9: mm5=dg?*(1-a?)

            "    movq    (%%edi), %%mm0\n"      //b3: mm0=dst3 dst2 dst1 dst0
            "    pand    %%mm7, %%mm3\n"        //b4: mm3=sb3 sb2 sb1 sb0

            "    pand    %%mm7, %%mm0\n"        //b5: mm0=db3 db2 db1 db0
            "    pmullw  %%mm1, %%mm3\n"        //b6: mm3=sb?*a?

            "    movq    (%%esi), %%mm7\n"      //r1: mm7=src3 src2 src1 src0
            "    paddw   %%mm5, %%mm4\n"        //g10: mm4=sg?*a?+dg?*(1-a?)

            "    paddw   %12, %%mm4\n"          //g11: mm4=(mm4+512) green
            "    pmullw  %%mm2, %%mm0\n"        //b7: mm0=db?*(1-a?)

            "    pand    %10, %%mm7\n"          //r2: mm7=sr3 sr2 sr1 sr0
            "    movq    %%mm4, %%mm5\n"        //g12: mm5=mm4 green

            "    psrlw   $5, %%mm4\n"          //g13: mm4=mm4>>5

            "    paddw   %%mm5, %%mm4\n"        //g14: mm4=mm4+mm5 green

            "    movq    (%%edi), %%mm5\n"      //r3: mm5=dst3 dst2 dst1 dst0
            "    paddw   %%mm3, %%mm0\n"        //b8: mm0=sb?*a?+db?*(1-a?)

            "    paddw   %11, %%mm0\n"         //b9: mm0=mm0 blue
            "    psrlw   $10, %%mm7\n"         //r4: shift src red down to position 0

            "    pand    %10, %%mm5\n"         //r5: mm5=dr3 dr2 dr1 dr0
            "    psrlw   $5, %%mm4\n"          //g15: mm4=0?g0 0?g0 0?g0 0?g0 green

            "    movq    %%mm0, %%mm3\n"        //b10: mm3=mm0 blue
            "    psrlw   $5, %%mm0\n"          //b11: mm0=mm0>>5 blue

            "    psrlw   $10, %%mm5\n"         //r6: shift dst red down to position 0
            "    paddw   %%mm3, %%mm0\n"        //b12: mm0=mm0+mm3 blue

            "    psrlw   $5, %%mm0\n"          //b13: mm0=000b 000b 000b 000b blue
            "    pmullw  %%mm1, %%mm7\n"        //mm7=sr?*a?

            "    pand    %9, %%mm4\n"           //g16: mm4=00g0 00g0 00g0 00g0 green
            "    pmullw  %%mm2, %%mm5\n"        //r7: mm5=dr?*(31-a?)

            "    por     %%mm4, %%mm0\n"        //mm0=00gb 00gb 00gb 00gb

                 //stall

            "    paddw   %%mm7, %%mm5\n"        //r8: mm5=sr?*a?+dr?*(31-a?)

            "    paddw   %11, %%mm5\n"          //r9: mm5=mm5 red

            "    movq    %%mm5, %%mm7\n"        //r10: mm7=mm5 red
            "    psrlw   $5, %%mm5\n"          //r11: mm5=mm5>>5 red

            "    paddw   %%mm7, %%mm5\n"        //r12: mm5=mm5+mm7 red

            "    psrlw   $5, %%mm5\n"          //r13: mm5=mm5>>5 red

            "    psllw   $10, %%mm5\n"         //r14: mm5=mm5<<10 red

            "    por     %%mm5, %%mm0\n"        //mm0=0rgb 0rgb 0rgb 0rgb

#ifdef PIC
            "    testl   $2, %%ebp\n"          //check if there are 2 pixels
#else
            "    testl   $2, %%ebx\n"          //check if there are 2 pixels
#endif
            "    jz      ablend_555_oneendpixel\n" //goto one pixel if that's it

            "    movd    %%mm0, (%%edi)\n"      //dst=0000 0000 0rgb 0rgb
            "    psrlq   $32, %%mm0\n"         //mm0>>32

            "    addl    $4, %%edi\n"          //edi=edi+4
#ifdef PIC
            "    subl    $2, %%ebp\n"          //saved 2 pixels
#else
            "    subl    $2, %%ebx\n"          //saved 2 pixels
#endif

            "    jz      ablend_555_nextline\n" //all done goto next line
            "ablend_555_oneendpixel:\n"    //work on last pixel
            "    movd    %%mm0, %%edx\n"        //edx=0rgb

            "    movw    %%dx, (%%edi)\n"       //dst=0rgb

            "ablend_555_nextline:\n"       //goto next line
            "    decl    %%ecx\n"            //nuke one line
            "    jz      ablend_555_done\n"  //all done

            "    movl    %2, %%eax\n"      //alpha
            "    movl    %0, %%esi\n"      //src

            "    movl    %1, %%edi\n"      //dst
            "    addl    %5, %%eax\n"  //inc alpha ptr by 1 line

            "    addl    %6, %%esi\n"  //inc src ptr by 1 line
            "    addl    %7, %%edi\n"  //inc dst ptr by 1 line

            "    movl    %%eax, %2\n"      //save new alpha base ptr
#ifdef PIC
            "    movl    %4, %%ebp\n"      //ebp=span width to copy
#else
            "    movl    %4, %%ebx\n"      //ebx=span width to copy
#endif

            "    movl    %%esi, %0\n"      //save new src base ptr
            "    movl    %%edi, %1\n"      //save new dst base ptr

            "    jmp     ablend_555_primeloop\n" //start the next span
            "ablend_555_done:\n"
            "    emms\n"

#ifdef PIC
            "    popl %%ebp\n"
#else
            "    popl %%ebx\n"
#endif
            :
            : "m" (lpLinearSrcBp), "m" (lpLinearDstBp), "m" (lpLinearAlpBp),
              "c" (iDstH),
#ifdef PIC
              "m" (iDstWStatic),
              "m" (iAlpPitchStatic), "m" (iSrcPitchStatic),
              "m" (iDstPitchStatic),
#else
              "m" (iDstW),
              "m" (iAlpPitch), "m" (iSrcPitch), "m" (iDstPitch),
#endif
              "m" (MASKR), "m" (MASKG), "m" (MASKB),
              "m" (SIXTEEN), "m" (FIVETWELVE)
            : "eax", "edx", "esi", "edi" );
#endif
}
