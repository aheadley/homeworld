/*=============================================================================
    Name    : transformer.c
    Purpose : provides vertex buffer transformation

    Created 10/8/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "glcaps.h"
#include "Transformer.h"
#include "Memory.h"
#include "main.h"

#define FPTR dword ptr
#define FSIZE 4
#define FSIZE_STR "4"


/*=============================================================================
    Data
=============================================================================*/

#if defined (_MSC_VER)
#define S(x) [esi + FSIZE*x]
#define D(x) [edi + FSIZE*x]
#define M(x) [edx + FSIZE*x]
#elif defined (__GNUC__) && defined (__i386__)
#define S(x) FSIZE_STR "*" #x "(%%esi)"
#define D(x) FSIZE_STR "*" #x "(%%edi)"
#define M(x) FSIZE_STR "*" #x "(%%edx)"
#endif

static sdword nVerts;
hvector* eyeVertexList = NULL;
hvector* clipVertexList = NULL;


//bits of the CPUID register
#define FXSR_BIT 0x1000000
#define XMM_BIT  0x2000000

//knitransform.cpp prototypes
int transDetermineKatmaiSupport(unsigned int);
int transCanSupportKatmai(void);
void transTransformCompletely_intrin(sdword, hvector*, vertexentry*, hmatrix*, hmatrix*);
void transTransformCompletely_xmm(sdword, hvector*, vertexentry*, hmatrix*, hmatrix*);
void transTransformVertexList_intrin(sdword, hvector*, vertexentry*, hmatrix*);
void transPerspectiveTransform_intrin(sdword, hvector*, hvector*, hmatrix*);
void transGeneralPerspectiveTransform_intrin(sdword, hvector*, hvector*, hmatrix*);
void kniTransFreeVertexLists(void);
void transSetKatmaiSupport(unsigned int);

void transTransformVertexList_asm(sdword n, hvector* dest, vertexentry* source, hmatrix* m);
void transPerspectiveTransform_asm(sdword n, hvector* dest, hvector* source, hmatrix* m);
void transGeneralPerspectiveTransform_asm(sdword n, hvector* dest, hvector* source, hmatrix* m);

static udword haveKatmai, haveFXSR;
static int useKatmai;

typedef void (*transVertexList_proc)(sdword, hvector*, vertexentry*, hmatrix*);
typedef void (*transPerspective_proc)(sdword, hvector*, hvector*, hmatrix*);
static transVertexList_proc transVertexList;
static transPerspective_proc transPerspective;
static transPerspective_proc transGeneral;

/*=============================================================================
    Code
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : chkcpubit
    Description : determine whether cpuid instruction is supported
    Inputs      :
    Outputs     :
    Return      : 1 (yes) or 0 (no)
----------------------------------------------------------------------------*/
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
    rval = 0;
#endif
    return rval;
}

/*-----------------------------------------------------------------------------
    Name        : transStartup
    Description : starts up the transformer module.  see transShutdown()
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void transStartup(void)
{
    static unsigned int cpu_edx;

    transShutdown();

    if (mainForceKatmai)
    {
#ifndef _MACOSX_FIX_ME
        transSetKatmaiSupport(1);
#endif
        haveKatmai = 1;
        haveFXSR = 1;
    }
    else if (!mainAllowKatmai)
    {
#ifndef _MACOSX_FIX_ME
        transSetKatmaiSupport(0);
#endif
        haveKatmai = 0;
        haveFXSR = 0;
    }
    else
    {
        if (!chkcpubit())
        {
            haveKatmai = 0;
            haveFXSR = 0;
        }
        else
        {
#if defined (_MSC_VER)
            _asm
            {
                pusha
                mov eax, 1
//                cpuid
                _emit 0x0f
                _emit 0xa2
                mov [cpu_edx], edx
                popa
            }
#elif defined (__GNUC__) && defined (__i386__)
            __asm__ __volatile__ (
                "    movl $1, %%eax\n"
                "    cpuid\n"
                : "=d" (cpu_edx)
                :
                : "eax", "ebx", "ecx" );
#endif

            haveFXSR = (cpu_edx & FXSR_BIT) ? 1 : 0;
            if (haveFXSR)
            {
                haveKatmai = (cpu_edx & XMM_BIT) ? 1 : 0;
            }
            else
            {
                haveKatmai = 0;
            }
        }
        
#ifndef _MACOSX_FIX_ME
        (void)transDetermineKatmaiSupport(haveKatmai);
#endif
    }

#ifdef _MACOSX_FIX_ME
    useKatmai = 0;
#else
    useKatmai = transCanSupportKatmai();
    transVertexList = (useKatmai) ? transTransformVertexList_intrin : transTransformVertexList_asm;
    transPerspective = (useKatmai) ? transPerspectiveTransform_intrin : transPerspectiveTransform_asm;
    transGeneral = (useKatmai) ? transGeneralPerspectiveTransform_intrin : transGeneralPerspectiveTransform_asm;
#endif 
}

/*-----------------------------------------------------------------------------
    Name        : transShutdown
    Description : shuts down the transformer module (deallocates memory)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void transShutdown(void)
{
    nVerts = 0;
    if (eyeVertexList != NULL)
    {
        memFree(eyeVertexList);
        eyeVertexList = NULL;
    }
    if (clipVertexList != NULL)
    {
        memFree(clipVertexList);
        clipVertexList = NULL;
    }

#ifndef _MACOSX_FIX_ME
    kniTransFreeVertexLists();
#endif
}

/*-----------------------------------------------------------------------------
    Name        : transFree
    Description : wrapper for calling from C++
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void transFree(void* ptr)
{
    memFree(ptr);
}

/*-----------------------------------------------------------------------------
    Name        : transAlloc
    Description : wrapper for calling from C++
    Inputs      :
    Outputs     :
    Return      :
    ASSERT      : memAlloc returns 16 byte aligned data, or multiple of
----------------------------------------------------------------------------*/
void* transAlloc(int n)
{
    return (void*)memAlloc(n, "Katmai mem", NonVolatile);
}

/*-----------------------------------------------------------------------------
    Name        : transGrowVertexLists
    Description : grows the vertex lists to accomodate an increasing number of vertices.
                  performance is expected to be very poor for a frame, then stable
                  afterwards.  TODO: transGrowAndShrinkVertexLists, a more memory-
                  conscious version of this pathetic fn
    Inputs      : nVertices - number of vertices
    Outputs     : preVertexList, xVertexList are re/allocated
    Return      :
----------------------------------------------------------------------------*/
void transGrowVertexLists(sdword nVertices)
{
    if (eyeVertexList == NULL)
    {
        nVerts = nVertices;
        eyeVertexList = (hvector*)memAlloc(nVerts * sizeof(hvector), "eye vertex list", NonVolatile);
        clipVertexList = (hvector*)memAlloc(nVerts * sizeof(hvector), "clip vertex list", NonVolatile);
    }

    if (nVerts < nVertices)
    {
        nVerts = nVertices;
        eyeVertexList = (hvector*)memRealloc(
            eyeVertexList, nVerts * sizeof(hvector), "eye vertex list", NonVolatile);
        clipVertexList = (hvector*)memRealloc(
            clipVertexList, nVerts * sizeof(hvector), "clip vertex list", NonVolatile);
    }
}

//x87 3D transform
void transTransformVertexList_asm(sdword n, hvector* dest, vertexentry* source, hmatrix* m)
{
#if defined (_MSC_VER)
    _asm
    {
        mov ecx, n
        mov esi, [source]
        mov ebx, [dest]
        mov edi, [m]
        shl ecx, 4
        add esi, ecx
        add ebx, ecx

        neg ecx

        cmp ecx, 0
        jne MEGA0
        ret

        align 16

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

        add     ecx, 4*FSIZE
        jnz     MEGA0
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    shll $4, %%ecx\n"
        "    addl %%ecx, %%esi\n"
        "    addl %%ecx, %%ebx\n"

        "    negl %%ecx\n"

        "    cmpl $0, %%ecx\n"
        "    je MEGA1\n"

        ".align 16\n"

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
        "    fstps   0*"FSIZE_STR"(%%ebx, %%ecx)\n"
        "    fxch    %%st(1)\n"
        "    fstps   1*"FSIZE_STR"(%%ebx, %%ecx)\n"
        "    fstps   2*"FSIZE_STR"(%%ebx, %%ecx)\n"

        "    addl    $(4*"FSIZE_STR"), %%ecx\n"
        "    jnz     MEGA0\n"

        "MEGA1:\n"
        :
        : "c" (n), "S" (source), "b" (dest), "D" (m) );
#endif
}

/*-----------------------------------------------------------------------------
    Name        : transTransformVertexList
    Description : transforms vertices in mesh format (vertexentry) by a 3D
                  (rotation + translation) matrix
    Inputs      : n - number of vertices
                  dest - transformed output vertices
                  source - input vertices
                  m - the matrix
    Outputs     : dest is filled
    Return      :
    Note        : need PII version of this, fxch stalls the pipe
----------------------------------------------------------------------------*/
/*
 * vEye[0] = m[0] * vObj[0] + m[4] * vObj[1] +  m[8] * vObj[2] + m[12]
 * vEye[1] = m[1] * vObj[0] + m[5] * vObj[1] +  m[9] * vObj[2] + m[13]
 * vEye[2] = m[2] * vObj[0] + m[6] * vObj[1] + m[10] * vObj[2] + m[14]
 * vEye[3] = 1.0f
 *
 * assumes next stage in pipe ignores vEye[3]
 */
void transTransformVertexList(sdword n, hvector* dest, vertexentry* source, hmatrix* m)
{
    transVertexList(n, dest, source, m);
}

//x87 perspective transform
void transPerspectiveTransform_asm(sdword n, hvector* dest, hvector* source, hmatrix* m)
{
#if defined (_MSC_VER)
    _asm
    {
        push esi
        push edi

        mov ecx, [n]
        mov esi, [source]
        mov edi, [dest]
        mov edx, [m]

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
        fld FPTR M(14)

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
        "    jz perspxform_two\n"

        "    .align 16\n"

        "perspxform_one:\n"
        "    flds "S(0)"\n"
        "    fmuls "M(0)"\n"
        "    flds "S(1)"\n"
        "    fmuls "M(5)"\n"
        "    flds "S(2)"\n"
        "    fmuls "M(10)"\n"

        "    flds "S(2)"\n"
        "    fmuls "M(8)"\n"
        "    flds "S(2)"\n"
        "    fmuls "M(9)"\n"
        "    flds "M(14)"\n"

        "    movl "S(2)", %%eax\n"
        "    leal "S(4)", %%esi\n"
        "    xorl $0x80000000, %%eax\n"
        "    decl %%ecx\n"

        "    faddp %%st, %%st(3)\n"
        "    faddp %%st, %%st(3)\n"
        "    faddp %%st, %%st(3)\n"

        "    fstps "D(2)"\n"
        "    fstps "D(1)"\n"
        "    fstps "D(0)"\n"

        "    movl %%eax, "D(3)"\n"
        "    leal "D(4)", %%edi\n"
        "    jnz perspxform_one\n"

        "perspxform_two:\n"
        :
        : "c" (n), "S" (source), "D" (dest), "d" (m)
        : "eax" );
#endif
}

/*-----------------------------------------------------------------------------
    Name        : transSinglePerspectiveTransform
    Description : transforms a vertex by a perspective matrix
    Inputs      : screenSpace - output screenspace vector
                  projection - projection matrix
                  cameraSpace - input cameraspace vector, ie. worldspace after modelview transform
    Outputs     : screenSpace hvector is computed
    Return      :
    Note        : *ONLY* use a perspective matrix here
----------------------------------------------------------------------------*/
void transSinglePerspectiveTransform(hvector* screenSpace, hmatrix* projection, hvector* cameraSpace)
{
    real32* m = (real32*)projection;
    real32* out = (real32*)screenSpace;
    real32* in  = (real32*)cameraSpace;

    out[0] = m[0]  * in[0] +  m[8] * in[2];
    out[1] = m[5]  * in[1] +  m[9] * in[2];
    out[2] = m[10] * in[2] + m[14] * in[3];
    out[3] = -in[2];
}

/*-----------------------------------------------------------------------------
    Name        : transSingleTotalTransform
    Description : transforms a vertex by a modelview & projection matrix
    Inputs      : screenSpace - output screenspace vector
                  modelview, projection - matrices
                  worldSpace - input worldspace vector
    Outputs     : screenSpace vector is computed
    Return      :
    Note        : *ONLY* use a perspective matrix for projection
----------------------------------------------------------------------------*/
void transSingleTotalTransform(vector* screenSpace, hmatrix* modelview, hmatrix* projection, vector* worldSpace)
{
#if 1
    hvector cameraSpace, screen;

    //world -> clip
    transTransformVertexList_asm(1, &cameraSpace, (vertexentry*)worldSpace, modelview);
    cameraSpace.w = 1.0f;

    //clip -> screen
    hmatMultiplyHMatByHVec(&screen, projection, &cameraSpace);

    screenSpace->x = screen.x / screen.w;
    screenSpace->y = screen.y / screen.w;
#else
    hvector cameraSpace, screen;

    //world -> clip
    transTransformVertexList_asm(1, &cameraSpace, (vertexentry*)worldSpace, modelview);

    //clip -> screen
    transPerspectiveTransform_asm(1, &screen, &cameraSpace, projection);

    screenSpace->x = screen.x / screen.w;
    screenSpace->y = screen.y / screen.w;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : transPerspectiveTransform
    Description : transforms vertices by a perspective matrix
    Inputs      : n - number of vertices
                  dest - transformed output vertices
                  source - input vertices
                  m - the matrix
    Outputs     : dest is filled
    Return      :
    Note        : *ONLY* use a perspective matrix here
----------------------------------------------------------------------------*/
/*
 * vClip[0] = m[0]  * vEye[0] +  m[8] * vEye[2]
 * vClip[1] = m[5]  * vEye[1] +  m[9] * vEye[2]
 * vClip[2] = m[10] * vEye[2] + m[14] * vEye[3]
 * vClip[3] = -vEye[2]
 *
 * assumes vEye[3] == 1.0f
 */
void transPerspectiveTransform(sdword n, hvector* dest, hvector* source, hmatrix* m)
{
    transPerspective(n, dest, source, m);
}

/*-----------------------------------------------------------------------------
    Name        : transTransformCompletely
    Description : transform 1st by a 3D matrix, then by a perspective matrix
    Inputs      : n - number of vertices
                  dest - transformed output vertices
                  intermed - temp storage for output from 3D transform
                  source - input vertices
                  m0 - 3D matrix
                  m1 - perspective matrix
    Outputs     : dest is filled
    Return      :
----------------------------------------------------------------------------*/
void transTransformCompletely(
    sdword n, hvector* dest, hvector* intermed, vertexentry* source, hmatrix* m0, hmatrix* m1)
{
    if (useKatmai)
    {
#ifndef _MACOSX_FIX_ME
        transTransformCompletely_xmm(n, dest, source, m0, m1);
#endif
    }
    else
    {
        transTransformVertexList_asm(n, intermed, source, m0);
        transPerspectiveTransform_asm(n, dest, intermed, m1);
    }
}

//x87 general perspective transform
void transGeneralPerspectiveTransform_asm(sdword n, hvector* dest, hvector* source, hmatrix* m)
{
#if defined (_MSC_VER)
    _asm
    {
        push esi
        push edi

        mov ecx, [n]
        mov edi, [dest]
        mov edx, [m]
        mov esi, [source]

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

        fld FPTR M(12)
        fld FPTR M(13)
        fld FPTR M(14)
        fld FPTR M(15)

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
        "    jz genperspxform_two\n"

        "    .align 16\n"

        "genperspxform_one:\n"
        "    flds "S(0)"\n"
        "    fmuls "M(0)"\n"
        "    flds "S(0)"\n"
        "    fmuls "M(1)"\n"
        "    flds "S(0)"\n"
        "    fmuls "M(2)"\n"
        "    flds "S(0)"\n"
        "    fmuls "M(3)"\n"

        "    flds "S(1)"\n"
        "    fmuls "M(4)"\n"
        "    flds "S(1)"\n"
        "    fmuls "M(5)"\n"
        "    flds "S(1)"\n"
        "    fmuls "M(6)"\n"
        "    flds "S(1)"\n"
        "    fmuls "M(7)"\n"

        "    fxch %%st(3)\n"
        "    faddp %%st, %%st(7)\n"
        "    fxch %%st(1)\n"
        "    faddp %%st, %%st(5)\n"
        "    faddp %%st, %%st(3)\n"
        "    faddp %%st, %%st(1)\n"

        "    flds "S(2)"\n"
        "    fmuls "M(8)"\n"
        "    flds "S(2)"\n"
        "    fmuls "M(9)"\n"
        "    flds "S(2)"\n"
        "    fmuls "M(10)"\n"
        "    flds "S(2)"\n"
        "    fmuls "M(11)"\n"

        "    fxch %%st(3)\n"
        "    faddp %%st, %%st(7)\n"
        "    fxch %%st(1)\n"
        "    faddp %%st, %%st(5)\n"
        "    faddp %%st, %%st(3)\n"
        "    faddp %%st, %%st(1)\n"

        "    flds "M(12)"\n"
        "    flds "M(13)"\n"
        "    flds "M(14)"\n"
        "    flds "M(15)"\n"

        "    fxch %%st(3)\n"
        "    faddp %%st, %%st(7)\n"
        "    fxch %%st(1)\n"
        "    faddp %%st, %%st(5)\n"
        "    faddp %%st, %%st(3)\n"

        "    leal "S(4)", %%esi\n"
        "    decl %%ecx\n"

        "    faddp %%st, %%st(1)\n"

        "    fxch %%st(3)\n"
        "    fstps "D(0)"\n"
        "    fxch %%st(1)\n"
        "    fstps "D(1)"\n"
        "    fstps "D(2)"\n"
        "    fstps "D(3)"\n"

        "    leal "D(4)", %%edi\n"

        "    jnz genperspxform_one\n"

        "genperspxform_two:\n"
        :
        : "c" (n), "D" (dest), "d" (m), "S" (source) );
#endif
}

/*-----------------------------------------------------------------------------
    Name        : transGeneralPerspectiveTransform
    Description : transforms 3D vertices by a general matrix
    Inputs      : n - number of vertices
                  dest - output vertices
                  source - input vertices
                  m - matrix
    Outputs     : dest is filled
    Return      :
----------------------------------------------------------------------------*/
/*
    sdword i;
    real32  ex, ey, ez;
    real32* mat = (real32*)m;

    for (i = 0; i < n; i++)
    {
        ex = source[i].x;
        ey = source[i].y;
        ez = source[i].z;
        dest[i].x = mat[0] * ex + mat[4] * ey + mat[8]  * ez + mat[12];
        dest[i].y = mat[1] * ex + mat[5] * ey + mat[9]  * ez + mat[13];
        dest[i].z = mat[2] * ex + mat[6] * ey + mat[10] * ez + mat[14];
        dest[i].w = mat[3] * ex + mat[7] * ey + mat[11] * ez + mat[15];
    }
*/
void transGeneralPerspectiveTransform(sdword n, hvector* dest, hvector* source, hmatrix* m)
{
    transGeneral(n, dest, source, m);
}

/*-----------------------------------------------------------------------------
    Name        : transPerspectiveMatrix
    Description : determines whether a given matrix may use the faster perspective
                  transform function
    Inputs      : m - the matrix to test
    Outputs     :
    Return      : TRUE if transPerspectiveTransform will suffice, FALSE if the matrix
                  is too general
----------------------------------------------------------------------------*/
bool transPerspectiveMatrix(hmatrix* m)
{
    real32* mat = (real32*)m;

    if (mat[4] == 0.0f && mat[12] == 0.0f &&
        mat[1] == 0.0f && mat[13] == 0.0f &&
        mat[2] == 0.0f && mat[6] == 0.0f &&
        mat[3] == 0.0f && mat[7] == 0.0f &&
        mat[11] == -1.0f && mat[15] == 0.0f)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

