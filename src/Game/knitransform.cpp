/*=============================================================================
    Name    : knitransform.cpp
    Purpose : Katmai New Instructions transformation routines

    Created 2/1/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <windows.h>
#include <stdlib.h.>
//#define _MM_FUNCTIONALITY
#include <xmmintrin.h>

#define VectorSize  4
#define VectorShift 2
#define NumVerts 2048

#define SFXSR_BIT 0x200     /* CR4, OS supports saving KNI state */
#define EM_BIT    0x4       /* CR0, OS is emulating FP */

#define SELECT(i0,i1,i2,i3) (i0*64 + i1*16 + i2*4 + i3)

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

_MM_ALIGN16 struct sInVerts
{
    __m128* x;
    __m128* y;
    __m128* z;
} sInputVerts;

_MM_ALIGN16 struct sOutVerts
{
    __m128* x;
    __m128* y;
    __m128* z;
} sOutputVerts;

_MM_ALIGN16 struct sOutVerts2
{
    __m128* x;
    __m128* y;
    __m128* z;
    __m128* w;
} sOutputVerts2;

extern "C" void transFree(void* ptr);
extern "C" void* transAlloc(int n);

/*-----------------------------------------------------------------------------
    Name        : kniTransFreeVertexLists
    Description : C-callable, frees vertex list contents
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
extern "C" void kniTransFreeVertexLists(void)
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

    _asm
    {
        pusha
        mov eax, CR4
        mov [cpu_eax], eax
        popa
    }
    hasSFXSR = (cpu_eax & SFXSR_BIT) ? 1 : 0;

    _asm
    {
        pusha
        mov eax, CR0
        mov [cpu_eax], eax
        popa
    }
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
extern "C" int transDetermineKatmaiSupport(unsigned int haveKatmai)
{
#ifdef _MM_FUNCTIONALITY
    //emulated KNI, always supported
    kniSupport = 1;
#else
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
extern "C" void transSetKatmaiSupport(unsigned int on)
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
extern "C" int transCanSupportKatmai(void)
{
    return kniSupport;
}

static _MM_ALIGN16 int Mask[4] = {0x80000000,0x80000000,0x80000000,0x80000000};

extern "C" void transTransformCompletely_xmm(
    int nVerts, hvector* dest, vertexentry* source, float* M0, float* M1)
{
    int i, n;
    hvector* dp;

    static float* m0;
    static float* m1;
    static float* mat0;
    static float* mat1;

    static _MM_ALIGN16 __m128 nextX;

    //matrices
    static _MM_ALIGN16 __m128 mat[16];
    static _MM_ALIGN16 __m128 per[16];

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
extern "C" void transTransformCompletely_intrin(
    int nVerts, hvector* dest, vertexentry* source, float* m0, float* m1)
{
    int i, n;
    hvector* dp;

    _MM_ALIGN16 __m128 curX, curY, curZ;
    _MM_ALIGN16 __m128 nextX, nextY, nextZ;

    //matrices
    _MM_ALIGN16 __m128 mat00, mat04, mat08, mat12;
    _MM_ALIGN16 __m128 mat01, mat05, mat09, mat13;
    _MM_ALIGN16 __m128 mat02, mat06, mat10, mat14;

    _MM_ALIGN16 __m128 per00, per08, per05, per09, per10, per14;

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

        for (int j = 0; j < VectorSize; j++, dp++)
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
extern "C" void transTransformVertexList_intrin(int nVerts, hvector* dest, vertexentry* source, float* m)
{
    int i, n;

    _MM_ALIGN16 __m128 curX, curY, curZ;

    //matrix
    _MM_ALIGN16 __m128 mat00, mat04, mat08, mat12;
    _MM_ALIGN16 __m128 mat01, mat05, mat09, mat13;
    _MM_ALIGN16 __m128 mat02, mat06, mat10, mat14;

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
extern "C" void transPerspectiveTransform_intrin(int nVerts, hvector* dest, hvector* source, float* m)
{
    int i, n;
    hvector* dp;

    _MM_ALIGN16 __m128 curX, curY, curZ;

    _MM_ALIGN16 __m128 mat00, mat08, mat05, mat09, mat10, mat14;

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

        for (int j = 0; j < VectorSize; j++, dp++)
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
extern "C" void transGeneralPerspectiveTransform_intrin(int nVerts, hvector* dest, hvector* source, float* m)
{
    int i, n;
    hvector* dp;

    _MM_ALIGN16 __m128 curX, curY, curZ;
    _MM_ALIGN16 __m128 mat00, mat04, mat08, mat12;
    _MM_ALIGN16 __m128 mat01, mat05, mat09, mat13;
    _MM_ALIGN16 __m128 mat02, mat06, mat10, mat14;
    _MM_ALIGN16 __m128 mat03, mat07, mat11, mat15;

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

        for (int j = 0; j < VectorSize; j++, dp++)
        {
            dp->x = *(xp + j);
            dp->y = *(yp + j);
            dp->z = *(zp + j);
            dp->w = *(wp + j);
        }
    }
}

