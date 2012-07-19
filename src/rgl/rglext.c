/*=============================================================================
    Name    : rglext.c
    Purpose : rgl mesh rendering extensions

    Created 4/8/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include "kgl.h"
#include "rglext.h"

//mesh polygon modes for rendering
#define MPM_Flat            0
#define MPM_Texture         1
#define MPM_Smooth          2
#define MPM_SmoothTexture   3

#define MPM_MatSwitch 0

#define SLOW 1

static GLcontext* ctx;
static gl_current* current;
static vertex_buffer* VB;

static GLint ventry_size;
static GLint ventry_x;
static GLint ventry_y;
static GLint ventry_z;
static GLint ventry_normal;
static GLubyte* vertex_list;

static GLfloat* xvertex_list = NULL;
static GLfloat* xnormal_list = NULL;

static GLint nentry_size;
static GLint nentry_x;
static GLint nentry_y;
static GLint nentry_z;
static GLubyte* normal_list;

static GLint pentry_size;
static GLint pentry_normal;
static GLint pentry_vertices;
static GLint pentry_material;
static GLint pentry_texcoords;
static GLubyte* poly_list;

#define NORMAL(n) \
    { \
        current->Normal[0] = n[0]; \
        current->Normal[1] = n[1]; \
        current->Normal[2] = n[2]; \
    }

DLL void rglListSpec(RGLenum pname, RGLenum param, GLint n, GLenum format)
{
    switch (pname)
    {
    case RGL_VERTEX_LIST:
        switch (param)
        {
        case RGL_SIZE:
            ventry_size = n;
            break;
        case RGL_X:
            ventry_x = n;
            break;
        case RGL_Y:
            ventry_y = n;
            break;
        case RGL_Z:
            ventry_z = n;
            break;
        case RGL_NORMAL:
            ventry_normal = n;
            break;
        }
        break;

    case RGL_NORMAL_LIST:
        switch (param)
        {
        case RGL_SIZE:
            nentry_size = n;
            break;
        case RGL_X:
            nentry_x = n;
            break;
        case RGL_Y:
            nentry_y = n;
            break;
        case RGL_Z:
            nentry_z = n;
            break;
        }
        break;

    case RGL_POLY_LIST:
        switch (param)
        {
        case RGL_SIZE:
            pentry_size = n;
            break;
        case RGL_NORMAL:
            pentry_normal = n;
            break;
        case RGL_VERTICES:
            pentry_vertices = n;
            break;
        case RGL_MATERIAL:
            pentry_material = n;
            break;
        case RGL_TEXCOORDS:
            pentry_texcoords = n;
            break;
        }
        break;
    }
}

DLL void rglList(RGLenum pname, GLvoid const* list)
{
    ctx = gl_get_context_ext();
    VB = ctx->VB;
    current = &ctx->Current;

    switch (pname)
    {
    case RGL_VERTEX_LIST:
        vertex_list = (GLubyte*)list;
        break;
    case RGL_NORMAL_LIST:
        normal_list = (GLubyte*)list;
        break;
    case RGL_POLY_LIST:
        poly_list = (GLubyte*)list;
        break;
    }
}

DLL void rglNormal(GLint param)
{
    GLfloat* nptr;

    nptr = (GLfloat*)(normal_list + nentry_size*param + nentry_x);

    NORMAL(nptr);
}

DLL void rglTriangle(GLint iPoly)
{
    GLfloat*  fptr;
    GLfloat*  nptr;
    GLint*    iptr;
    GLushort* sptr;

    GLuint    count = VB->Count;
    GLfloat*  obj = VB->Obj[count];
    GLfloat*  normal = VB->Normal[count];

    iptr = (GLint*)(poly_list + pentry_size*iPoly + pentry_normal);
#if SLOW
    nptr = (GLfloat*)(normal_list + nentry_size*(*iptr) + nentry_x);
#else
    nptr = xnormal_list + 3*(*iptr);
#endif

    sptr = (GLushort*)(poly_list + pentry_size*iPoly + pentry_vertices);

#if SLOW
    fptr = (GLfloat*)(vertex_list + ventry_size*sptr[0] + ventry_x);
#else
    fptr = xvertex_list + 4*sptr[0];
#endif
    obj[0] = fptr[0]; obj[1] = fptr[1]; obj[2] = fptr[2]; obj[3] = 1.0f;
    normal[0] = nptr[0]; normal[1] = nptr[1]; normal[2] = nptr[2];

#if SLOW
    fptr = (GLfloat*)(vertex_list + ventry_size*sptr[1] + ventry_x);
#else
    fptr = xvertex_list + 4*sptr[1];
#endif
    obj[4] = fptr[0]; obj[5] = fptr[1]; obj[6] = fptr[2]; obj[7] = 1.0f;
    normal[3] = nptr[0]; normal[4] = nptr[1]; normal[5] = nptr[2];

#if SLOW
    fptr = (GLfloat*)(vertex_list + ventry_size*sptr[2] + ventry_x);
#else
    fptr = xvertex_list + 4*sptr[2];
#endif
    obj[8] = fptr[0]; obj[9] = fptr[1]; obj[10] = fptr[2]; obj[11] = 1.0f;
    normal[6] = nptr[0]; normal[7] = nptr[1]; normal[8] = nptr[2];

    VB->Count += 3;
}

DLL void rglTexturedTriangle(GLint iPoly)
{
    GLfloat*  fptr;
    GLfloat*  nptr;
    GLfloat*  tptr;
    GLint*    iptr;
    GLushort* sptr;

    GLuint    count = VB->Count;
    GLfloat*  obj = VB->Obj[count];
    GLfloat*  normal = VB->Normal[count];
    GLfloat*  tex = VB->TexCoord[count];

    iptr = (GLint*)(poly_list + pentry_size*iPoly + pentry_normal);
#if SLOW
    nptr = (GLfloat*)(normal_list + nentry_size*(*iptr) + nentry_x);
#else
    nptr = xnormal_list + 3*(*iptr);
#endif

    tptr = (GLfloat*)(poly_list + pentry_size*iPoly + pentry_texcoords);

    sptr = (GLushort*)(poly_list + pentry_size*iPoly + pentry_vertices);

#if SLOW
    fptr = (GLfloat*)(vertex_list + ventry_size*sptr[0] + ventry_x);
#else
    fptr = xvertex_list + 4*sptr[0];
#endif
    obj[0] = fptr[0]; obj[1] = fptr[1]; obj[2] = fptr[2]; obj[3] = 1.0f;
    normal[0] = nptr[0]; normal[1] = nptr[1]; normal[2] = nptr[2];
    tex[0] = tptr[0]; tex[1] = tptr[1];

#if SLOW
    fptr = (GLfloat*)(vertex_list + ventry_size*sptr[1] + ventry_x);
#else
    fptr = xvertex_list + 4*sptr[1];
#endif
    obj[4] = fptr[0]; obj[5] = fptr[1]; obj[6] = fptr[2]; obj[7] = 1.0f;
    normal[3] = nptr[0]; normal[4] = nptr[1]; normal[5] = nptr[2];
    tex[2] = tptr[2]; tex[3] = tptr[3];

#if SLOW
    fptr = (GLfloat*)(vertex_list + ventry_size*sptr[2] + ventry_x);
#else
    fptr = xvertex_list + 4*sptr[2];
#endif
    obj[8] = fptr[0]; obj[9] = fptr[1]; obj[10] = fptr[2]; obj[11] = 1.0f;
    normal[6] = nptr[0]; normal[7] = nptr[1]; normal[8] = nptr[2];
    tex[4] = tptr[4]; tex[5] = tptr[5];

    VB->Count += 3;
}

DLL void rglSmoothTriangle(GLint iPoly)
{
    GLfloat*  nptr;
    GLint*    nvptr;
    GLfloat*  vptr;
    GLushort* sptr;

    GLuint    count = VB->Count;
    GLfloat*  obj = VB->Obj[count];
    GLfloat*  normal = VB->Normal[count];

    sptr = (GLushort*)(poly_list + pentry_size*iPoly + pentry_vertices);

#if SLOW
    vptr = (GLfloat*)(vertex_list + ventry_size*sptr[0] + ventry_x);
#else
    vptr = xvertex_list + 4*sptr[0];
#endif
    nvptr = (GLint*)(vertex_list + ventry_size*sptr[0] + ventry_normal);
#if SLOW
    nptr = (GLfloat*)(normal_list + nentry_size*(*nvptr) + nentry_x);
#else
    nptr = xnormal_list + 3*(*nvptr);
#endif
    obj[0] = vptr[0]; obj[1] = vptr[1]; obj[2] = vptr[2]; obj[3] = 1.0f;
    normal[0] = nptr[0]; normal[1] = nptr[1]; normal[2] = nptr[2];

#if SLOW
    vptr = (GLfloat*)(vertex_list + ventry_size*sptr[1] + ventry_x);
#else
    vptr = xvertex_list + 4*sptr[1];
#endif
    nvptr = (GLint*)(vertex_list + ventry_size*sptr[1] + ventry_normal);
#if SLOW
    nptr = (GLfloat*)(normal_list + nentry_size*(*nvptr) + nentry_x);
#else
    nptr = xnormal_list + 3*(*nvptr);
#endif
    obj[4] = vptr[0]; obj[5] = vptr[1]; obj[6] = vptr[2]; obj[7] = 1.0f;
    normal[3] = nptr[0]; normal[4] = nptr[1]; normal[5] = nptr[2];

#if SLOW
    vptr = (GLfloat*)(vertex_list + ventry_size*sptr[2] + ventry_x);
#else
    vptr = xvertex_list + 4*sptr[2];
#endif
    nvptr = (GLint*)(vertex_list + ventry_size*sptr[2] + ventry_normal);
#if SLOW
    nptr = (GLfloat*)(normal_list + nentry_size*(*nvptr) + nentry_x);
#else
    nptr = xnormal_list + 3*(*nvptr);
#endif
    obj[8] = vptr[0]; obj[9] = vptr[1]; obj[10] = vptr[2]; obj[11] = 1.0f;
    normal[6] = nptr[0]; normal[7] = nptr[1]; normal[8] = nptr[2];

    VB->Count += 3;
}

DLL void rglSmoothTexturedTriangle(GLint iPoly)
{
    GLfloat*  nptr;
    GLint*    nvptr;
    GLfloat*  vptr;
    GLfloat*  tptr;
    GLushort* sptr;

    GLuint    count = VB->Count;
    GLfloat*  obj = VB->Obj[count];
    GLfloat*  normal = VB->Normal[count];
    GLfloat*  tex = VB->TexCoord[count];

    sptr = (GLushort*)(poly_list + pentry_size*iPoly + pentry_vertices);
    tptr = (GLfloat*)(poly_list + pentry_size*iPoly + pentry_texcoords);

#if SLOW
    vptr = (GLfloat*)(vertex_list + ventry_size*sptr[0] + ventry_x);
#else
    vptr = xvertex_list + 4*sptr[0];
#endif
    nvptr = (GLint*)(vertex_list + ventry_size*sptr[0] + ventry_normal);
#if SLOW
    nptr = (GLfloat*)(normal_list + nentry_size*(*nvptr) + nentry_x);
#else
    nptr = xnormal_list + 3*(*nvptr);
#endif
    obj[0] = vptr[0]; obj[1] = vptr[1]; obj[2] = vptr[2]; obj[3] = 1.0f;
    normal[0] = nptr[0]; normal[1] = nptr[1]; normal[2] = nptr[2];
    tex[0] = tptr[0]; tex[1] = tptr[1];

#if SLOW
    vptr = (GLfloat*)(vertex_list + ventry_size*sptr[1] + ventry_x);
#else
    vptr = xvertex_list + 4*sptr[1];
#endif
    nvptr = (GLint*)(vertex_list + ventry_size*sptr[1] + ventry_normal);
#if SLOW
    nptr = (GLfloat*)(normal_list + nentry_size*(*nvptr) + nentry_x);
#else
    nptr = xnormal_list + 3*(*nvptr);
#endif
    obj[4] = vptr[0]; obj[5] = vptr[1]; obj[6] = vptr[2]; obj[7] = 1.0f;
    normal[3] = nptr[0]; normal[4] = nptr[1]; normal[5] = nptr[2];
    tex[2] = tptr[2]; tex[3] = tptr[3];

#if SLOW
    vptr = (GLfloat*)(vertex_list + ventry_size*sptr[2] + ventry_x);
#else
    vptr = xvertex_list + 4*sptr[2];
#endif
    nvptr = (GLint*)(vertex_list + ventry_size*sptr[2] + ventry_normal);
#if SLOW
    nptr = (GLfloat*)(normal_list + nentry_size*(*nvptr) + nentry_x);
#else
    nptr = xnormal_list + 3*(*nvptr);
#endif
    obj[8] = vptr[0]; obj[9] = vptr[1]; obj[10] = vptr[2]; obj[11] = 1.0f;
    normal[6] = nptr[0]; normal[7] = nptr[1]; normal[8] = nptr[2];
    tex[4] = tptr[4]; tex[5] = tptr[5];

    VB->Count += 3;
}

#define S(x)   dword ptr [esi + 4*x]
#define D(x)   dword ptr [edi + 4*x]
#define M(n)   dword ptr [edx + 4*n]

static void affine_transform(
    GLuint n, GLfloat* d, GLfloat m[16], GLfloat* s)
{
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

        mov eax, 0x3f800000     //1.0f

      one:
        fld S(0)
        fmul M(0)
        fld S(0)
        fmul M(1)
        fld S(0)
        fmul M(2)

        fld S(1)
        fmul M(4)
        fld S(1)
        fmul M(5)
        fld S(1)
        fmul M(6)

        fxch st(2)
        faddp st(5), st
        faddp st(3), st
        faddp st(1), st

        fld S(2)
        fmul M(8)
        fld S(2)
        fmul M(9)
        fld S(2)
        fmul M(10)

        fxch st(2)
        faddp st(5), st
        faddp st(3), st
        faddp st(1), st

        fxch st(2)
        fadd M(12)
        fxch st(1)
        fadd M(13)
        fxch st(2)
        fadd M(14)

        fxch st(1)
        fstp D(0)
        fstp D(2)
        fstp D(1)
        mov D(3), eax

        lea esi, S(4)
        dec ecx

        lea edi, D(4)

        jnz one

      two:
        pop edi
        pop esi
    }
}

static void normal_transform(GLfloat* d, GLfloat* s, GLfloat* m, GLuint n)
{
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

      one:
        fld S(0)
        fmul M(0)
        fld S(0)
        fmul M(4)
        fld S(0)
        fmul M(8)

        fld S(1)
        fmul M(1)
        fld S(1)
        fmul M(5)
        fld S(1)
        fmul M(9)

        fxch st(2)
        faddp st(5), st
        faddp st(3), st
        faddp st(1), st

        fld S(2)
        fmul M(2)
        fld S(2)
        fmul M(6)
        fld S(2)
        fmul M(10)

        fxch st(2)
        faddp st(5), st
        faddp st(3), st
        faddp st(1), st

        fxch st(2)
        fstp D(0)
        fstp D(1)
        fstp D(2)

        lea esi, S(4)
        dec ecx

        lea edi, D(3)
        jnz one

      two:
        pop edi
        pop esi
    }
}

DLL void rglMeshRender(
    GLint n, void (*callback)(GLint material), GLint* meshPolyMode)
{
    GLint currentMaterial, material;
    GLint i, mode, nVerts;
    GLenum model;
    GLubyte* poly;
    GLfloat* xptr;
    GLfloat* vptr;
    GLfloat  temp[4], v[4];
    GLfloat modelview[16], modelviewInv[16];
    static GLfloat Identity[16] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

#if !SLOW
    if (xvertex_list == NULL)
    {
        xvertex_list = (GLfloat*)malloc(4096*4*sizeof(GLfloat));
        xnormal_list = (GLfloat*)malloc(4096*3*sizeof(GLfloat));
    }
#endif

    nVerts = (n & 0xffff0000) >> 16;
    n &= 0xffff;

#if !SLOW
    MEMCPY(modelview, ctx->ModelViewMatrix, 16*sizeof(GLfloat));
    MEMCPY(ctx->ModelViewMatrix, Identity, 16*sizeof(GLfloat));
    invert_matrix(modelview, modelviewInv);

    //transform the coordinates
    affine_transform(nVerts, xvertex_list, modelview, (GLfloat*)vertex_list);
    //transform the normals
    normal_transform(xnormal_list, (GLfloat*)normal_list, modelviewInv, nVerts);
#endif

    currentMaterial = -1;
    poly = poly_list;

    model = GL_SMOOTH;
    if (ctx->ShadeModel != GL_SMOOTH)
    {
        glShadeModel(GL_SMOOTH);
    }
    glBegin(GL_TRIANGLES);

    for (i = 0; i < n; i++, poly += pentry_size)
    {
        material = (GLint)(*((GLushort*)(poly + pentry_material)));
        if (currentMaterial != material)
        {
            glEnd();
            callback(material);
            currentMaterial = material;
            glBegin(GL_TRIANGLES);
        }

        mode = *meshPolyMode;

        switch (mode)
        {
        case MPM_Flat:
#if MPM_MatSwitch
            if (model != GL_FLAT)
            {
                glShadeModel(GL_FLAT);
                model = GL_FLAT;
            }
#endif
            rglTriangle(i);
            break;

        case MPM_Texture:
#if MPM_MatSwitch
            if (model != GL_FLAT)
            {
                glShadeModel(GL_FLAT);
                model = GL_FLAT;
            }
#endif
            rglTexturedTriangle(i);
            break;

        case MPM_Smooth:
#if MPM_MatSwitch
            if (model != GL_SMOOTH)
            {
                glShadeModel(GL_SMOOTH);
                model = GL_SMOOTH;
            }
#endif
            rglSmoothTriangle(i);
            break;

        case MPM_SmoothTexture:
#if MPM_MatSwitch
            if (model != GL_SMOOTH)
            {
                glShadeModel(GL_SMOOTH);
                model = GL_SMOOTH;
            }
#endif
            rglSmoothTexturedTriangle(i);
            break;
        }
    }

    glEnd();
#if MPM_MatSwitch
    glShadeModel(GL_SMOOTH);
#endif
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

#if !SLOW
    MEMCPY(ctx->ModelViewMatrix, modelview, 16*sizeof(GLfloat));
#endif
}
