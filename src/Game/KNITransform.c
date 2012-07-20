/*=============================================================================
    Name    : knitransform.c
    Purpose : Katmai New Instructions transformation routines

    Created 2/1/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#if !defined (_MSC_VER) && !(defined (__GNUC__) && defined (__i386__))
#error KNI transform routines only available on x86 systems.
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdlib.h>
//#define _MM_FUNCTIONALITY
#include <xmmintrin.h>

/* Prefix and suffix for declaring types as 16-byte aligned. */
#ifdef _MSC_VER
#define MM_ALIGN16_PRE  _MM_ALIGN16
#define MM_ALIGN16_POST
#else
#define MM_ALIGN16_PRE
#define MM_ALIGN16_POST __attribute__ ((aligned (16)))
#endif

#define VectorSize  4
#define VectorShift 2
#ifndef _MSC_VER
#define VectorShift_STR "2" /* Needed for GCC inline ASM. */
#endif
#define NumVerts 2048

#define SFXSR_BIT 0x200     /* CR4, OS supports saving KNI state */
#define EM_BIT    0x4       /* CR0, OS is emulating FP */

#if defined (_MSC_VER)
#define SELECT(i0,i1,i2,i3) (i0*64 + i1*16 + i2*4 + i3)
#elif defined (__GNUC__) && defined (__i386__)
#define SELECT(i0,i1,i2,i3) "$(" #i0 "*64 + " #i1 "*16 + " #i2 "*4 + " #i3 ")"
#endif

static int kniSupport;

typedef struct tagvertex
{
    float x;                               // X component of this vertex.
    float y;                               // Y component of this vertex.
    float z;                               // Z component of this vertex.
    int   iVertexNormal;                   // Index into the point normal list.
} vertexentry;

typedef struct
{
    float x, y, z, w;
} hvector;

typedef struct
{
    float m11,m21,m31,m41, m12,m22,m32,m42, m13,m23,m33,m43, m14,m24,m34,m44;
} hmatrix;

static int lastNumVerts = 0;

MM_ALIGN16_PRE struct sInVerts
{
    __m128* x;
    __m128* y;
    __m128* z;
} sInputVerts MM_ALIGN16_POST;

MM_ALIGN16_PRE struct sOutVerts
{
    __m128* x;
    __m128* y;
    __m128* z;
} sOutputVerts MM_ALIGN16_POST;

MM_ALIGN16_PRE struct sOutVerts2
{
    __m128* x;
    __m128* y;
    __m128* z;
    __m128* w;
} sOutputVerts2 MM_ALIGN16_POST;

void transFree(void* ptr);
void* transAlloc(int n);

/*-----------------------------------------------------------------------------
    Name        : kniTransFreeVertexLists
    Description : C-callable, frees vertex list contents
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kniTransFreeVertexLists(void)
{
    if (lastNumVerts != 0)
    {
        lastNumVerts = 0;
        transFree((void*)sInputVerts.x);
        transFree((void*)sInputVerts.y);
        transFree((void*)sInputVerts.z);
        transFree((void*)sOutputVerts.x);
        transFree((void*)sOutputVerts.y);
        transFree((void*)sOutputVerts.z);
        transFree((void*)sOutputVerts2.x);
        transFree((void*)sOutputVerts2.y);
        transFree((void*)sOutputVerts2.z);
        transFree((void*)sOutputVerts2.w);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kniTransGrowVertexLists
    Description : grows vertex lists to accomodate a possibly increasing amount
    Inputs      : n - number of vectored vertices
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void kniTransGrowVertexLists(int n)
{
    if (n > lastNumVerts)
    {
        kniTransFreeVertexLists();

        lastNumVerts = n;

        int nBytes = n * VectorSize * sizeof(float);
        sInputVerts.x = (__m128*)transAlloc(nBytes);
        sInputVerts.y = (__m128*)transAlloc(nBytes);
        sInputVerts.z = (__m128*)transAlloc(nBytes);
        sOutputVerts.x = (__m128*)transAlloc(nBytes);
        sOutputVerts.y = (__m128*)transAlloc(nBytes);
        sOutputVerts.z = (__m128*)transAlloc(nBytes);
        sOutputVerts2.x = (__m128*)transAlloc(nBytes);
        sOutputVerts2.y = (__m128*)transAlloc(nBytes);
        sOutputVerts2.z = (__m128*)transAlloc(nBytes);
        sOutputVerts2.w = (__m128*)transAlloc(nBytes);
    }
}

/*-----------------------------------------------------------------------------
    Name        : chkxmmbits
    Description : check control registers (not under NT) to determine OS-level
                  support for KNI
    Inputs      :
    Outputs     :
    Return      : 1 (supported) or 0 (not supported)
----------------------------------------------------------------------------*/
static int chkxmmbits(void)
{
    static unsigned int cpu_eax, cpu_edx;
    int hasSFXSR, hasEM;

#if defined (_MSC_VER)
    _asm
    {
        pusha
        mov eax, CR4
        mov [cpu_eax], eax
        popa
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "pusha\n\t"
        "movl %%CR4, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "popa\n\t"
        : "=m" (cpu_eax) );
#endif
    hasSFXSR = (cpu_eax & SFXSR_BIT) ? 1 : 0;

#if defined (_MSC_VER)
    _asm
    {
        pusha
        mov eax, CR0
        mov [cpu_eax], eax
        popa
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "pusha\n\t"
        "movl %%CR0, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "popa\n\t"
        : "=m" (cpu_eax) );
#endif
    hasEM = (cpu_eax & EM_BIT) ? 1 : 0;

    if (hasSFXSR && !hasEM)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : transDetermineKatmaiSupport
    Description : determines whether Katmai New Instructions are supported by the
                  processor & the OS.  kind of guesses under NT
    Inputs      : haveKatmai - !0 if CPU.XMM bit set
    Outputs     :
    Return      : 1 or 0 (does or doesn't)
----------------------------------------------------------------------------*/
int transDetermineKatmaiSupport(unsigned int haveKatmai)
{
#ifdef _MM_FUNCTIONALITY
    //emulated KNI, always supported
    kniSupport = 1;
#else

#ifdef _WIN32
    DWORD version;

    version = GetVersion();

    if (version < 0x80000000)
    {
        //WinNT
        kniSupport = (haveKatmai) ? 1 : 0;
    }
    else
    {
        //Win9x or Win32
        kniSupport = (haveKatmai) ? 1/*chkxmmbits()*/ : 0;
    }
#else
    kniSupport = (haveKatmai) ? 1 : 0;
#endif

#endif
    return kniSupport;
}

/*-----------------------------------------------------------------------------
    Name        : transSetKatmaiSupport
    Description : forces / disallows Katmai support at the user's whim
    Inputs      : on - 1 or 0 (enable or disable)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void transSetKatmaiSupport(unsigned int on)
{
    kniSupport = on;
}

/*-----------------------------------------------------------------------------
    Name        : transCanSupportKatmai
    Description : wrapper for global variable set in transDetermineKatmaiSupport()
    Inputs      :
    Outputs     :
    Return      : 1 or 0 (can or can't)
----------------------------------------------------------------------------*/
int transCanSupportKatmai(void)
{
    return kniSupport;
}

static MM_ALIGN16_PRE int Mask[4] MM_ALIGN16_POST =
    {0x80000000,0x80000000,0x80000000,0x80000000};

void transTransformCompletely_xmm(
    int nVerts, hvector* dest, vertexentry* source, float* M0, float* M1)
{
    int i, n;
    hvector* dp;

    static float* m0;
    static float* m1;
    static float* mat0;
    static float* mat1;

    static MM_ALIGN16_PRE __m128 nextX MM_ALIGN16_POST;

    //matrices
    static MM_ALIGN16_PRE __m128 mat[16] MM_ALIGN16_POST;
    static MM_ALIGN16_PRE __m128 per[16] MM_ALIGN16_POST;

    //setup statics (easily accessible from asm)
    m0 = M0;
    m1 = M1;
    mat0 = (float*)mat;
    mat1 = (float*)per;

    _mm_prefetch((char*)m0, 0);

    //vectorize nVerts
    n = ((nVerts + (VectorSize-1)) & (~(VectorSize-1))) >> VectorShift;

    _mm_prefetch((char*)mat0, 0);

    //possibly expand our vertex lists
    kniTransGrowVertexLists(n);

#if defined (_MSC_VER)
    _asm
    {
        push    esi
        push    edi

        //
        //assignment
        //

        //3D
        mov     esi, [m0]
        mov     edi, [mat0]
        mov     eax, [m1]
        mov     edx, [mat1]

        prefetchnta [edx]
        movss   xmm0, DWORD PTR [esi + 4*0]
        movss   xmm1, DWORD PTR [esi + 4*1]
        shufps  xmm0, xmm0, 00h
        movss   xmm2, DWORD PTR [esi + 4*2]
        shufps  xmm1, xmm1, 00h
        movaps  XMMWORD PTR [edi + 4*4*0], xmm0
        movss   xmm3, DWORD PTR [esi + 4*4]
        prefetchnta [eax]
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

        //...

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

        //perspective
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

        //
        //transformation
        //

        mov     esi, [mat0]
        mov     edi, [mat1]
        mov     ecx, [n]
        mov     eax, [source]
        mov     edx, [dest]
        shl     ecx, 4+VectorShift

    start:
        sub     ecx, 64

        //swizzle
        movups  xmm7, XMMWORD PTR [eax+ecx]         //1st xyz
        movups  xmm2, XMMWORD PTR [eax+ecx+16]      //2nd xyz
        movups  xmm4, XMMWORD PTR [eax+ecx+32]      //3rd xyz
        movups  xmm3, XMMWORD PTR [eax+ecx+48]      //4th xyz
        prefetchnta [eax+ecx-64]
        movaps  xmm6, xmm7
        movaps  xmm5, xmm4
        unpcklps xmm6, xmm2
        unpcklps xmm5, xmm3
        movaps  xmm0, xmm6
        prefetchnta [eax+ecx+16-64]
        shufps  xmm6, xmm5, SELECT(3,2,3,2) //yyyy in xmm6
        shufps  xmm0, xmm5, SELECT(1,0,1,0) //xxxx in xmm0
        unpckhps xmm4, xmm3
        prefetchnta [eax+ecx+32-64]
        unpckhps xmm7, xmm2

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
        addps   xmm2, xmm4          //x*m1 + y*m5 + z*m9 + m13 in xmm2

        movaps  xmm3, XMMWORD PTR [esi+4*4*2]
        movaps  xmm1, XMMWORD PTR [esi+4*4*6]
        mulps   xmm3, xmm0          //x*m2 in xmm3
        mulps   xmm1, xmm6          //y*m6 in xmm1
        movaps  xmm4, XMMWORD PTR [esi+4*4*10]
        movaps  xmm5, XMMWORD PTR [esi+4*4*14]
        mulps   xmm4, xmm7          //z*m10 in xmm4
        addps   xmm1, xmm3          //x*m2 + y*m6 in xmm1
        addps   xmm4, xmm5          //z*m10 + m14 in xmm4
        addps   xmm1, xmm4          //x*m2 + y*m6 + z*m10 + m14 in xmm1

        //project
        movaps  xmm0, XMMWORD PTR [nextX]       //X in xmm0
        movaps  xmm6, xmm2          //Y in xmm6
        movaps  xmm7, xmm1          //Z in xmm7

        movaps  xmm3, XMMWORD PTR [edi+4*4*0]
        movaps  xmm2, XMMWORD PTR [edi+4*4*8]
        mulps   xmm3, xmm0          //x*p0 in xmm3
        mulps   xmm2, xmm7          //z*p8 in xmm2
        movaps  xmm5, XMMWORD PTR [edi+4*4*5]   //...
        addps   xmm2, xmm3          //x*p0 + z*p8 in xmm2

        movaps  xmm4, XMMWORD PTR [edi+4*4*9]
        mulps   xmm5, xmm6          //y*p5 in xmm5
        mulps   xmm4, xmm7          //z*p9 in xmm4
        movaps  xmm3, XMMWORD PTR [edi+4*4*10]  //...
        addps   xmm4, xmm5          //y*p5 + z*p9 in xmm4

        movaps  xmm6, XMMWORD PTR [edi+4*4*14]
        mulps   xmm3, xmm7          //z*p10 in xmm3
        movaps  xmm1, xmm7          //...
        addps   xmm6, xmm3          //z*p10 + p14 in xmm6

        xorps   xmm1, [Mask]
        movaps  xmm0, xmm2          //...
        movaps  xmm3, xmm1          //-z in xmm3

        //swizzle
        unpcklps xmm0, xmm4
        movaps  xmm1, xmm0
        movaps  xmm5, xmm6
        unpcklps xmm5, xmm3
        shufps  xmm1, xmm5, SELECT(1,0,1,0)
        movaps  XMMWORD PTR [edx+ecx], xmm1     //w0,z0,y0,x0
        shufps  xmm0, xmm5, SELECT(3,2,3,2)
        movaps  XMMWORD PTR [edx+ecx+16], xmm0  //w1,z1,y1,x1
        movaps  xmm0,xmm2
        unpckhps xmm0, xmm4
        movaps  xmm1, xmm0
        movaps  xmm5, xmm6
        unpckhps xmm5, xmm3
        shufps  xmm1, xmm5, SELECT(1,0,1,0)
        movaps  XMMWORD PTR [edx+ecx+32], xmm1  //w2,z2,y1,x2
        shufps  xmm0, xmm5, SELECT(3,2,3,2)
        movaps  XMMWORD PTR [edx+ecx+48], xmm0  //w3,z3,y1,x3

        jnz     start

        pop     edi
        pop     esi
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
             /*
              * assignment
              */

             /* 3D */
        "    movl    %0, %%esi\n"
        "    movl    %1, %%edi\n"
        "    movl    %2, %%eax\n"
        "    movl    %3, %%edx\n"

        "    prefetchnta (%%edx)\n"
        "    movss   4*0(%%esi), %%xmm0\n"
        "    movss   4*1(%%esi), %%xmm1\n"
        "    shufps  $0x00, %%xmm0, %%xmm0\n"
        "    movss   4*2(%%esi), %%xmm2\n"
        "    shufps  $0x00, %%xmm1, %%xmm1\n"
        "    movaps  %%xmm0, 4*4*0(%%edi)\n"
        "    movss   4*4(%%esi), %%xmm3\n"
        "    prefetchnta (%%eax)\n"
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

             /* ... */

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

             /* perspective */
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

             /*
              * transformation
              */

        "    movl    %1, %%esi\n"
        "    movl    %3, %%edi\n"
        "    movl    %4, %%ecx\n"
        "    movl    %5, %%eax\n"
        "    movl    %6, %%edx\n"
        "    shll    $(4+"VectorShift_STR"), %%ecx\n"

        "transcomp_start:\n"
        "    subl    $64, %%ecx\n"

             /* swizzle */
        "    movups  (%%eax, %%ecx), %%xmm7\n"        /* 1st xyz */
        "    movups  16(%%eax, %%ecx), %%xmm2\n"      /* 2nd xyz */
        "    movups  32(%%eax, %%ecx), %%xmm4\n"      /* 3rd xyz */
        "    movups  48(%%eax, %%ecx), %%xmm3\n"      /* 4th xyz */
        "    prefetchnta -64(%%eax, %%ecx)\n"
        "    movaps  %%xmm7, %%xmm6\n"
        "    movaps  %%xmm4, %%xmm5\n"
        "    unpcklps %%xmm2, %%xmm6\n"
        "    unpcklps %%xmm3, %%xmm5\n"
        "    movaps  %%xmm6, %%xmm0\n"
        "    prefetchnta 16-64(%%eax, %%ecx)\n"
        "    shufps  "SELECT(3,2,3,2)", %%xmm5, %%xmm6\n" //yyyy in xmm6 */
        "    shufps  "SELECT(1,0,1,0)", %%xmm5, %%xmm0\n" //xxxx in xmm0 */
        "    unpckhps %%xmm3, %%xmm4\n"
        "    prefetchnta 32-64(%%eax, %%ecx)\n"
        "    unpckhps %%xmm2, %%xmm7\n"

             /* transform */
        "    movaps  4*4*0(%%esi), %%xmm3\n"
        "    movaps  4*4*4(%%esi), %%xmm1\n"
        "    mulps   %%xmm0, %%xmm3\n"          /* x*m0 in xmm3 */
        "    prefetchnta 48-64(%%eax, %%ecx)\n"
        "    shufps  "SELECT(1,0,1,0)", %%xmm4, %%xmm7\n" /* zzzz in xmm7 */
        "    mulps   %%xmm6, %%xmm1\n"          /* y*m4 in xmm1 */
        "    movaps  4*4*8(%%esi), %%xmm4\n"
        "    movaps  4*4*12(%%esi), %%xmm5\n"
        "    mulps   %%xmm7, %%xmm4\n"          /* z*m8 in xmm4 */
        "    addps   %%xmm3, %%xmm1\n"          /* x*m0 + y*m4 in xmm1 */
        "    addps   %%xmm5, %%xmm4\n"          /* z*m8 + m12 in xmm4 */
        "    addps   %%xmm4, %%xmm1\n"          /* x*m0 + y*m4 + z*m8 + m12 in xmm1 */
        "    movaps  %%xmm1, %7\n"

        "    movaps  4*4*1(%%esi), %%xmm3\n"
        "    movaps  4*4*5(%%esi), %%xmm2\n"
        "    mulps   %%xmm0, %%xmm3\n"          /* x*m1 in xmm3 */
        "    mulps   %%xmm6, %%xmm2\n"          /* y*m5 in xmm2 */
        "    movaps  4*4*9(%%esi), %%xmm4\n"
        "    movaps  4*4*13(%%esi), %%xmm5\n"
        "    mulps   %%xmm7, %%xmm4\n"          /* z*m9 in xmm4 */
        "    addps   %%xmm3, %%xmm2\n"          /* x*m1 + y*m5 in xmm2 */
        "    addps   %%xmm5, %%xmm4\n"          /* z*m9 + m13 in xmm4 */
        "    addps   %%xmm4, %%xmm2\n"          /* x*m1 + y*m5 + z*m9 + m13 in xmm2 */

        "    movaps  4*4*2(%%esi), %%xmm3\n"
        "    movaps  4*4*6(%%esi), %%xmm1\n"
        "    mulps   %%xmm0, %%xmm3\n"          /* x*m2 in xmm3 */
        "    mulps   %%xmm6, %%xmm1\n"          /* y*m6 in xmm1 */
        "    movaps  4*4*10(%%esi), %%xmm4\n"
        "    movaps  4*4*14(%%esi), %%xmm5\n"
        "    mulps   %%xmm7, %%xmm4\n"          /* z*m10 in xmm4 */
        "    addps   %%xmm3, %%xmm1\n"          /* x*m2 + y*m6 in xmm1 */
        "    addps   %%xmm5, %%xmm4\n"          /* z*m10 + m14 in xmm4 */
        "    addps   %%xmm4, %%xmm1\n"          /* x*m2 + y*m6 + z*m10 + m14 in xmm1 */

             /* project */
        "    movaps  %7, %%xmm0\n"              /* X in xmm0 */
        "    movaps  %%xmm2, %%xmm6\n"          /* Y in xmm6 */
        "    movaps  %%xmm1, %%xmm7\n"          /* Z in xmm7 */

        "    movaps  4*4*0(%%edi), %%xmm3\n"
        "    movaps  4*4*8(%%edi), %%xmm2\n"
        "    mulps   %%xmm0, %%xmm3\n"          /* x*p0 in xmm3 */
        "    mulps   %%xmm7, %%xmm2\n"          /* z*p8 in xmm2 */
        "    movaps  4*4*5(%%edi), %%xmm5\n"    /* ... */
        "    addps   %%xmm3, %%xmm2\n"          /* x*p0 + z*p8 in xmm2 */

        "    movaps  4*4*9(%%edi), %%xmm4\n"
        "    mulps   %%xmm6, %%xmm5\n"          /* y*p5 in xmm5 */
        "    mulps   %%xmm7, %%xmm4\n"          /* z*p9 in xmm4 */
        "    movaps  4*4*10(%%edi), %%xmm3\n"   /* ... */
        "    addps   %%xmm5, %%xmm4\n"          /* y*p5 + z*p9 in xmm4 */

        "    movaps  4*4*14(%%edi), %%xmm6\n"
        "    mulps   %%xmm7, %%xmm3\n"          /* z*p10 in xmm3 */
        "    movaps  %%xmm7, %%xmm1\n"          /* ... */
        "    addps   %%xmm3, %%xmm6\n"          /* z*p10 + p14 in xmm6 */

        "    xorps   %8, %%xmm1\n"
        "    movaps  %%xmm2, %%xmm0\n"          /* ... */
        "    movaps  %%xmm1, %%xmm3\n"          /* -z in xmm3 */

             //swizzle
        "    unpcklps %%xmm4, %%xmm0\n"
        "    movaps  %%xmm0, %%xmm1\n"
        "    movaps  %%xmm6, %%xmm5\n"
        "    unpcklps %%xmm3, %%xmm5\n"
        "    shufps  "SELECT(1,0,1,0)", %%xmm5, %%xmm1\n"
        "    movaps  %%xmm1, (%%edx, %%ecx)\n"    /* w0,z0,y0,x0 */
        "    shufps  "SELECT(3,2,3,2)", %%xmm5, %%xmm0\n"
        "    movaps  %%xmm0, 16(%%edx, %%ecx)\n"  /* w1,z1,y1,x1 */
        "    movaps  %%xmm2, %%xmm0\n"
        "    unpckhps %%xmm4, %%xmm0\n"
        "    movaps  %%xmm0, %%xmm1\n"
        "    movaps  %%xmm6, %%xmm5\n"
        "    unpckhps %%xmm3, %%xmm5\n"
        "    shufps  "SELECT(1,0,1,0)", %%xmm5, %%xmm1\n"
        "    movaps  %%xmm1, 32(%%edx, %%ecx)\n"  /* w2,z2,y1,x2 */
        "    shufps  "SELECT(3,2,3,2)", %%xmm5, %%xmm0\n"
        "    movaps  %%xmm0, 48(%%edx, %%ecx)\n"  /* w3,z3,y1,x3 */

        "    jnz     transcomp_start\n"
        :
        : "m" (m0), "m" (mat0), "m" (m1), "m" (mat1),
          "m" (n), "m" (source), "m" (dest),
          "m" (nextX),
          "m" (Mask)
          : "eax", "edx", "ecx", "edi", "esi" );
#endif
}

/*-----------------------------------------------------------------------------
    Name        : transTransformCompletely_intrin
    Description : transform a bunch of vertices by a 3D matrix, then by a perspective matrix
    Inputs      : nVerts - number of vertices (not vectored)
                  dest - destination vertices
                  source - source vertices
                  m0 - 3D matrix, 4x4
                  m1 - perspective matrix, 4x4
    Outputs     : dest is filled
    Return      :
----------------------------------------------------------------------------*/
void transTransformCompletely_intrin(
    int nVerts, hvector* dest, vertexentry* source, float* m0, float* m1)
{
    int i, j, n;
    hvector* dp;

    MM_ALIGN16_PRE __m128 curX MM_ALIGN16_POST,
                          curY MM_ALIGN16_POST,
                          curZ MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 nextX MM_ALIGN16_POST,
                          nextY MM_ALIGN16_POST,
                          nextZ MM_ALIGN16_POST;

    //matrices
    MM_ALIGN16_PRE __m128 mat00 MM_ALIGN16_POST,
                          mat04 MM_ALIGN16_POST,
                          mat08 MM_ALIGN16_POST,
                          mat12 MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 mat01 MM_ALIGN16_POST,
                          mat05 MM_ALIGN16_POST,
                          mat09 MM_ALIGN16_POST,
                          mat13 MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 mat02 MM_ALIGN16_POST,
                          mat06 MM_ALIGN16_POST,
                          mat10 MM_ALIGN16_POST,
                          mat14 MM_ALIGN16_POST;

    MM_ALIGN16_PRE __m128 per00 MM_ALIGN16_POST,
                          per08 MM_ALIGN16_POST,
                          per05 MM_ALIGN16_POST,
                          per09 MM_ALIGN16_POST,
                          per10 MM_ALIGN16_POST,
                          per14 MM_ALIGN16_POST;

    mat00 = _mm_set_ps1(m0[0]);
    mat04 = _mm_set_ps1(m0[4]);
    mat08 = _mm_set_ps1(m0[8]);
    mat12 = _mm_set_ps1(m0[12]);
    mat01 = _mm_set_ps1(m0[1]);
    mat05 = _mm_set_ps1(m0[5]);
    mat09 = _mm_set_ps1(m0[9]);
    mat13 = _mm_set_ps1(m0[13]);
    mat02 = _mm_set_ps1(m0[2]);
    mat06 = _mm_set_ps1(m0[6]);
    mat10 = _mm_set_ps1(m0[10]);
    mat14 = _mm_set_ps1(m0[14]);

    per00 = _mm_set_ps1(m1[0]);
    per08 = _mm_set_ps1(m1[8]);
    per05 = _mm_set_ps1(m1[5]);
    per09 = _mm_set_ps1(m1[9]);
    per10 = _mm_set_ps1(m1[10]);
    per14 = _mm_set_ps1(m1[14]);

    //vectorize nVerts
    n = ((nVerts + (VectorSize-1)) & (~(VectorSize-1))) >> VectorShift;

    //possibly expand our vertex lists
    kniTransGrowVertexLists(n);

    //swizzle
    for (i = 0; i < n; i++)
    {
        sInputVerts.x[i] = _mm_set_ps((source+4*i+3)->x, (source+4*i+2)->x, (source+4*i+1)->x, (source+4*i+0)->x);
        sInputVerts.y[i] = _mm_set_ps((source+4*i+3)->y, (source+4*i+2)->y, (source+4*i+1)->y, (source+4*i+0)->y);
        sInputVerts.z[i] = _mm_set_ps((source+4*i+3)->z, (source+4*i+2)->z, (source+4*i+1)->z, (source+4*i+0)->z);
    }

    //transform
    for (i = 0; i < n; i++)
    {
        //modelview
        curX = sInputVerts.x[i];
        curY = sInputVerts.y[i];
        curZ = sInputVerts.z[i];

        nextX = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(curX, mat00),
                        _mm_mul_ps(curY, mat04)),
                    _mm_add_ps(
                        _mm_mul_ps(curZ, mat08),
                        mat12));
        nextY = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(curX, mat01),
                        _mm_mul_ps(curY, mat05)),
                    _mm_add_ps(
                        _mm_mul_ps(curZ, mat09),
                        mat13));
        nextZ = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(curX, mat02),
                        _mm_mul_ps(curY, mat06)),
                    _mm_add_ps(
                        _mm_mul_ps(curZ, mat10),
                        mat14));

        //projection
        sOutputVerts2.x[i] = _mm_add_ps(
                                _mm_mul_ps(nextX, per00),
                                _mm_mul_ps(nextZ, per08));
        sOutputVerts2.y[i] = _mm_add_ps(
                                _mm_mul_ps(nextY, per05),
                                _mm_mul_ps(nextZ, per09));
        sOutputVerts2.z[i] = _mm_add_ps(
                                _mm_mul_ps(nextZ, per10),
                                per14);
        sOutputVerts2.w[i] = _mm_xor_ps(nextZ, (*((__m128*)(&(Mask[0])))));
    }

    //swizzle
    for (i = 0, dp = dest; i < n; i++)
    {
        float* xp = (float*)&sOutputVerts2.x[i];
        float* yp = (float*)&sOutputVerts2.y[i];
        float* zp = (float*)&sOutputVerts2.z[i];
        float* wp = (float*)&sOutputVerts2.w[i];

        for (j = 0; j < VectorSize; j++, dp++)
        {
            dp->x = *(xp + j);
            dp->y = *(yp + j);
            dp->z = *(zp + j);
            dp->w = *(wp + j);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : transTransformVertexList_intrin
    Description : transform a bunch of vertices by a 3D matrix
    Inputs      : nVerts - number of vertices (not vectored)
                  dest - destination vertices, not used (kept in temporary vectored storage instead)
                  source - source vertices
                  m - 3D matrix, 4x4
    Outputs     : sOutputVerts is filled
    Return      :
----------------------------------------------------------------------------*/
void transTransformVertexList_intrin(int nVerts, hvector* dest, vertexentry* source, float* m)
{
    int i, n;

    MM_ALIGN16_PRE __m128 curX MM_ALIGN16_POST,
                          curY MM_ALIGN16_POST,
                          curZ MM_ALIGN16_POST;

    //matrix
    MM_ALIGN16_PRE __m128 mat00 MM_ALIGN16_POST,
                          mat04 MM_ALIGN16_POST,
                          mat08 MM_ALIGN16_POST,
                          mat12 MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 mat01 MM_ALIGN16_POST,
                          mat05 MM_ALIGN16_POST,
                          mat09 MM_ALIGN16_POST,
                          mat13 MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 mat02 MM_ALIGN16_POST,
                          mat06 MM_ALIGN16_POST,
                          mat10 MM_ALIGN16_POST,
                          mat14 MM_ALIGN16_POST;

    mat00 = _mm_set_ps1(m[0]);
    mat04 = _mm_set_ps1(m[4]);
    mat08 = _mm_set_ps1(m[8]);
    mat12 = _mm_set_ps1(m[12]);
    mat01 = _mm_set_ps1(m[1]);
    mat05 = _mm_set_ps1(m[5]);
    mat09 = _mm_set_ps1(m[9]);
    mat13 = _mm_set_ps1(m[13]);
    mat02 = _mm_set_ps1(m[2]);
    mat06 = _mm_set_ps1(m[6]);
    mat10 = _mm_set_ps1(m[10]);
    mat14 = _mm_set_ps1(m[14]);

    n = ((nVerts + (VectorSize-1)) & (~(VectorSize-1))) >> VectorShift;

    kniTransGrowVertexLists(n);

    for (i = 0; i < n; i++)
    {
        sInputVerts.x[i] = _mm_set_ps((source+4*i+3)->x, (source+4*i+2)->x, (source+4*i+1)->x, (source+4*i+0)->x);
        sInputVerts.y[i] = _mm_set_ps((source+4*i+3)->y, (source+4*i+2)->y, (source+4*i+1)->y, (source+4*i+0)->y);
        sInputVerts.z[i] = _mm_set_ps((source+4*i+3)->z, (source+4*i+2)->z, (source+4*i+1)->z, (source+4*i+0)->z);
    }

    //transform
    for (i = 0; i < n; i++)
    {
        curX = sInputVerts.x[i];
        curY = sInputVerts.y[i];
        curZ = sInputVerts.z[i];

        sOutputVerts.x[i] = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(curX, mat00),
                                    _mm_mul_ps(curY, mat04)),
                                _mm_add_ps(
                                    _mm_mul_ps(curZ, mat08),
                                    mat12));
        sOutputVerts.y[i] = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(curX, mat01),
                                    _mm_mul_ps(curY, mat05)),
                                _mm_add_ps(
                                    _mm_mul_ps(curZ, mat09),
                                    mat13));
        sOutputVerts.z[i] = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(curX, mat02),
                                    _mm_mul_ps(curY, mat06)),
                                _mm_add_ps(
                                    _mm_mul_ps(curZ, mat10),
                                    mat14));
    }
}

/*-----------------------------------------------------------------------------
    Name        : transPerspectiveTransform_intrin
    Description : transform a bunch of vertices by a 1-point projection matrix.
                  this fn continues transTransformVertexList_intrin(..), and hence
                  uses its temporary storage for setup
    Inputs      : nVerts - number of vertices (not vectored)
                  dest - destination vertices
                  source - source vertices, not used
                  m - projection matrix
    Outputs     : dest is filled
    Return      :
----------------------------------------------------------------------------*/
void transPerspectiveTransform_intrin(int nVerts, hvector* dest, hvector* source, float* m)
{
    int i, j, n;
    hvector* dp;

    MM_ALIGN16_PRE __m128 curX MM_ALIGN16_POST,
                          curY MM_ALIGN16_POST,
                          curZ MM_ALIGN16_POST;

    MM_ALIGN16_PRE __m128 mat00 MM_ALIGN16_POST,
                          mat08 MM_ALIGN16_POST,
                          mat05 MM_ALIGN16_POST,
                          mat09 MM_ALIGN16_POST,
                          mat10 MM_ALIGN16_POST,
                          mat14 MM_ALIGN16_POST;

    mat00 = _mm_set_ps1(m[0]);
    mat08 = _mm_set_ps1(m[8]);
    mat05 = _mm_set_ps1(m[5]);
    mat09 = _mm_set_ps1(m[9]);
    mat10 = _mm_set_ps1(m[10]);
    mat14 = _mm_set_ps1(m[14]);

    n = ((nVerts + (VectorSize-1)) & (~(VectorSize-1))) >> VectorShift;

    for (i = 0; i < n; i++)
    {
        curX = sOutputVerts.x[i];
        curY = sOutputVerts.y[i];
        curZ = sOutputVerts.z[i];

        sOutputVerts2.x[i] = _mm_add_ps(
                                _mm_mul_ps(curX, mat00),
                                _mm_mul_ps(curZ, mat08));
        sOutputVerts2.y[i] = _mm_add_ps(
                                _mm_mul_ps(curY, mat05),
                                _mm_mul_ps(curZ, mat09));
        sOutputVerts2.z[i] = _mm_add_ps(
                                _mm_mul_ps(curZ, mat10),
                                mat14);
        sOutputVerts2.w[i] = _mm_xor_ps(curZ, (*((__m128*)(&(Mask[0])))));
    }

    for (i = 0, dp = dest; i < n; i++)
    {
        float* xp = (float*)&sOutputVerts2.x[i];
        float* yp = (float*)&sOutputVerts2.y[i];
        float* zp = (float*)&sOutputVerts2.z[i];
        float* wp = (float*)&sOutputVerts2.w[i];

        for (j = 0; j < VectorSize; j++, dp++)
        {
            dp->x = *(xp + j);
            dp->y = *(yp + j);
            dp->z = *(zp + j);
            dp->w = *(wp + j);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : transGeneralPerspectiveTransform_intrin
    Description : transform a bunch of vertices by a general projection matrix.
                  this fn continues transTransformVertexList_intrin(..), and hence
                  uses its temporary storage for setup
    Inputs      : nVerts - number of vertices (not vectored)
                  dest - destination vertices
                  source - source vertices, not used
                  m - general projection matrix
    Outputs     : dest is filled
    Return      :
----------------------------------------------------------------------------*/
void transGeneralPerspectiveTransform_intrin(int nVerts, hvector* dest, hvector* source, float* m)
{
    int i, j, n;
    hvector* dp;

    MM_ALIGN16_PRE __m128 curX MM_ALIGN16_POST,
                          curY MM_ALIGN16_POST,
                          curZ MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 mat00 MM_ALIGN16_POST,
                          mat04 MM_ALIGN16_POST,
                          mat08 MM_ALIGN16_POST,
                          mat12 MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 mat01 MM_ALIGN16_POST,
                          mat05 MM_ALIGN16_POST,
                          mat09 MM_ALIGN16_POST,
                          mat13 MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 mat02 MM_ALIGN16_POST,
                          mat06 MM_ALIGN16_POST,
                          mat10 MM_ALIGN16_POST,
                          mat14 MM_ALIGN16_POST;
    MM_ALIGN16_PRE __m128 mat03 MM_ALIGN16_POST,
                          mat07 MM_ALIGN16_POST,
                          mat11 MM_ALIGN16_POST,
                          mat15 MM_ALIGN16_POST;

    mat00 = _mm_set_ps1(m[0]);
    mat01 = _mm_set_ps1(m[1]);
    mat02 = _mm_set_ps1(m[2]);
    mat03 = _mm_set_ps1(m[3]);
    mat04 = _mm_set_ps1(m[4]);
    mat05 = _mm_set_ps1(m[5]);
    mat06 = _mm_set_ps1(m[6]);
    mat07 = _mm_set_ps1(m[7]);
    mat08 = _mm_set_ps1(m[8]);
    mat09 = _mm_set_ps1(m[9]);
    mat10 = _mm_set_ps1(m[10]);
    mat11 = _mm_set_ps1(m[11]);
    mat12 = _mm_set_ps1(m[12]);
    mat13 = _mm_set_ps1(m[13]);
    mat14 = _mm_set_ps1(m[14]);
    mat15 = _mm_set_ps1(m[15]);

    n = ((nVerts + (VectorSize-1)) & (~(VectorSize-1))) >> VectorShift;

    for (i = 0; i < n; i++)
    {
        curX = sOutputVerts.x[i];
        curY = sOutputVerts.y[i];
        curZ = sOutputVerts.z[i];

        sOutputVerts2.x[i] = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(curX, mat00),
                                    _mm_mul_ps(curY, mat04)),
                                _mm_add_ps(
                                    _mm_mul_ps(curZ, mat08),
                                    mat12));
        sOutputVerts2.y[i] = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(curX, mat01),
                                    _mm_mul_ps(curY, mat05)),
                                _mm_add_ps(
                                    _mm_mul_ps(curZ, mat09),
                                    mat13));
        sOutputVerts2.z[i] = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(curX, mat02),
                                    _mm_mul_ps(curY, mat06)),
                                _mm_add_ps(
                                    _mm_mul_ps(curZ, mat10),
                                    mat14));
        sOutputVerts2.w[i] = _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(curX, mat03),
                                    _mm_mul_ps(curY, mat07)),
                                _mm_add_ps(
                                    _mm_mul_ps(curZ, mat11),
                                    mat15));
    }

    for (i = 0, dp = dest; i < n; i++)
    {
        float* xp = (float*)&sOutputVerts2.x[i];
        float* yp = (float*)&sOutputVerts2.y[i];
        float* zp = (float*)&sOutputVerts2.z[i];
        float* wp = (float*)&sOutputVerts2.w[i];

        for (j = 0; j < VectorSize; j++, dp++)
        {
            dp->x = *(xp + j);
            dp->y = *(yp + j);
            dp->z = *(zp + j);
            dp->w = *(wp + j);
        }
    }
}

