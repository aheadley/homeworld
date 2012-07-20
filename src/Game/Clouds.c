/*=============================================================================
    Name    : clouds.c
    Purpose : dust/gas cloud resource data structure management stuff

    Created 2/9/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#ifdef _WIN32
#include <windows.h>
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Types.h"
#include "Memory.h"
#include "Clouds.h"
#include "Vector.h"
#include "Matrix.h"
#include "Randy.h"
#include "glinc.h"
#include "render.h"
#include "LinkedList.h"
#include "Universe.h"
#include "FastMath.h"
#include "StatScript.h"
#include "mainrgn.h"
#include "Clipper.h"
#include "Shader.h"
#include "glcaps.h"
#include "AutoLOD.h"
#include "UnivUpdate.h"
#include "devstats.h"

#ifndef M_PI_2
#define M_PI_2 3.14159265358979323846 / 2.0
#endif

extern udword gDevcaps2;

static ubyte gCloudColor[4];

static real32 fogColor[4];

static GLuint _counter = 0;

bool8  dontNebulate;
real32 g_FogSum = 0.0f;

real32 DUSTCLOUD_RADIAL_SCALE = 9.0f;

real32 DUSTCLOUD_RADII[NUM_DUSTCLOUDTYPES] = {350.0f, 500.0f, 800.0f, 1300.0f};

static real32 DUSTCLOUD_BG_SCALAR = 0.3f;
static real32 DUSTCLOUD_MAX_DENSITY = 0.9f;
static real32 DUSTCLOUD_DENSITY_THRESHOLD = 0.2f;

static real32 DUSTCLOUD_RED = 0.67f;
static real32 DUSTCLOUD_GREEN = 0.57f;
static real32 DUSTCLOUD_BLUE = 0.38f;

static real32 DUSTCLOUD_RED_DIST = 0.05f;
static real32 DUSTCLOUD_GREEN_DIST = 0.05f;
static real32 DUSTCLOUD_BLUE_DIST = 0.05f;

static real32 DUSTCLOUD_FOG_RED = 0.67f;
static real32 DUSTCLOUD_FOG_GREEN = 0.57f;
static real32 DUSTCLOUD_FOG_BLUE = 0.38f;

static real32 DUSTCLOUD_RAN_MULTIPLIER = 0.007f;

//lightning
static real32 CLOUD_LDT0 = 800.0f;
static real32 CLOUD_LDT1 = 200.0f;

static real32 CLOUD_LIGHTNING_MAXDIST = 500.0f;

static udword CLOUD_LIGHTNING_BRIGHTEN = 40;

static real32 CLOUD_DISCHARGE_CHARGE = 0.02f;

static real32 CLOUD_RADIAL_DAMAGE_SCALAR = 1.0f;

static real32 CLOUD_ZAP_SCALAR = 0.7f;

static real32 CLOUD_LIGHTNING_WIDTH_UPCLOSE = 10.0f;
static real32 CLOUD_LIGHTNING_WIDTH_FARAWAY = 13.0f;

static real32 CLOUD_LIGHTNING_LINE_RED = 0.47f;
static real32 CLOUD_LIGHTNING_LINE_GREEN = 0.47f;
static real32 CLOUD_LIGHTNING_LINE_BLUE = 0.675f;

static real32 CLOUD_LIGHTNING_FRINGE_SCALAR = 1.5f;
static real32 CLOUD_LIGHTNING_MIDPOINT_SCALAR = 0.8f;

static real32 CLOUD_LIGHTNING_FRINGE_RED = 0.0f;
static real32 CLOUD_LIGHTNING_FRINGE_GREEN = 0.0f;
static real32 CLOUD_LIGHTNING_FRINGE_BLUE = 0.824f;
static real32 CLOUD_LIGHTNING_FRINGE_ALPHA = 0.294f;

static real32 CLOUD_LIGHTNING_MAIN_RED = 0.47f;
static real32 CLOUD_LIGHTNING_MAIN_GREEN = 0.47f;
static real32 CLOUD_LIGHTNING_MAIN_BLUE = 0.51f;
static real32 CLOUD_LIGHTNING_MAIN_ALPHA = 0.55f;

//cloud tweakables.  see ~/data/cloud.script
scriptEntry CloudTweaks[] =
{
    makeEntry(DUSTCLOUD_RADIAL_SCALE, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_RADII[DustCloud0],scriptSetReal32CB),
    makeEntry(DUSTCLOUD_RADII[DustCloud1],scriptSetReal32CB),
    makeEntry(DUSTCLOUD_RADII[DustCloud2],scriptSetReal32CB),
    makeEntry(DUSTCLOUD_RADII[DustCloud3],scriptSetReal32CB),
    makeEntry(DUSTCLOUD_BG_SCALAR, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_MAX_DENSITY, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_DENSITY_THRESHOLD, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_RED, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_GREEN, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_BLUE, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_RED_DIST, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_GREEN_DIST, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_BLUE_DIST, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_FOG_RED, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_FOG_GREEN, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_FOG_BLUE, scriptSetReal32CB),
    makeEntry(DUSTCLOUD_RAN_MULTIPLIER, scriptSetReal32CB),
    makeEntry(CLOUD_LDT0, scriptSetReal32CB),
    makeEntry(CLOUD_LDT1, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_MAXDIST, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_BRIGHTEN, scriptSetUdwordCB),
    makeEntry(CLOUD_RADIAL_DAMAGE_SCALAR, scriptSetReal32CB),
    makeEntry(CLOUD_ZAP_SCALAR, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_WIDTH_UPCLOSE, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_WIDTH_FARAWAY, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_LINE_RED, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_LINE_GREEN, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_LINE_BLUE, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_FRINGE_SCALAR, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_MIDPOINT_SCALAR, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_FRINGE_RED, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_FRINGE_GREEN, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_FRINGE_BLUE, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_FRINGE_ALPHA, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_MAIN_RED, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_MAIN_GREEN, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_MAIN_BLUE, scriptSetReal32CB),
    makeEntry(CLOUD_LIGHTNING_MAIN_ALPHA, scriptSetReal32CB),
    endEntry
};

/*
 * LIGHTNING DATA / FUNCTIONS
 */

lightning _lightnings[MAX_LIGHTNINGS];

//return a fresh lightning handle
lhandle cloudGetFreshLightningHandle()
{
    udword i;

    for (i = 1; i < MAX_LIGHTNINGS; i++)
    {
        if (!bitTest(_lightnings[i].flags, LIGHTNING_ACTIVE))
        {
            return (lhandle)i;
        }
    }
    return 0;
}

//return a fresh lightning structure
lightning* cloudGetFreshLightning()
{
    lhandle handle = cloudGetFreshLightningHandle();
    return (handle == 0) ? NULL : &_lightnings[handle];
}

//clean up a lightning structure
void cloudInitLightning(lightning* l)
{
    udword i;
    vector origin = {0.0f, 0.0f, 0.0f};

    dbgAssert(l != NULL);

    for (i = 0; i < MAX_LIGHTNING_PALS; i++)
    {
        l->pals[i] = 0;
    }

    l->pointA = origin;
    l->pointB = origin;
    l->distance = 0.0f;
    l->parent = NULL;
    l->countdown = 0;
}

//clean up a lightning structure given its handle
void cloudInitLightningHandle(lhandle handle)
{
    dbgAssert(handle != 0);
    cloudInitLightning(&_lightnings[handle]);
}

//return a structure given a handle
lightning* cloudLightningFromHandle(lhandle handle)
{
    return (handle == 0) ? NULL : &_lightnings[handle];
}

//"kill" a lightning structure
void cloudKillLightning(lightning* l)
{
    udword i;

    dbgAssert(l != NULL);
    l->flags = 0;

    for (i = 0; i < MAX_LIGHTNING_PALS; i++)
    {
        if (l->pals[i] != 0)
        {
            lightning* light = cloudLightningFromHandle(l->pals[i]);
            light->flags = 0;
        }
    }
}

//"live" a lightning structure
void cloudLiveLightning(lightning* l)
{
    dbgAssert(l != NULL);
    bitSet(l->flags, LIGHTNING_ACTIVE);
}

//main-ify a lightning structure
void cloudMainLightning(lightning* l)
{
    dbgAssert(l != NULL);
    bitSet(l->flags, LIGHTNING_MAINSEGMENT);
}

/*
 * ELLIPSE DATA / FUNCTIONS
 */

static bool8 g_NoBumpy = FALSE;

//geometry for the cloud lods
static ellipseObject ellipseLOD[5];
ellipseObject genericEllipse[4];

//prototypes
void ellipsoid_init(sdword n);
void ellipsoid_seq(ellipseObject* ellipsoid, sdword n, float a, float b, float c);

typedef struct slot
{
    float cos, sin;
    enum { None, Only, Done } flag;
} slot;

static sdword n_max = 0;
static slot* table = NULL;
static vertex* octant = NULL;

#define SetP(p,px,py,pz) \
    { \
        (p).x = px; \
        (p).y = py; \
        (p).z = pz; \
    }

#if 0
#define SetV(v,px,py,pz,nx,ny,nz) \
    { \
        SetP((v)->p,px,py,pz); \
        SetP((v)->n,nx,ny,nz); \
    }
#else
void SetV(vertex* v, real32 x, real32 y, real32 z,
          real32 nx, real32 ny, real32 nz)
{
    real32 rand = (real32)(ranRandom(RAN_Clouds) % 10) * DUSTCLOUD_RAN_MULTIPLIER;
    if (ranRandom(RAN_Clouds) & 1)
    {
        rand = -rand;
    }
    if (g_NoBumpy)
    {
        rand = 0.0f;
    }
    v->p.x = x + rand;
    v->p.y = y + rand;
    v->p.z = z + rand;

    v->n.x = nx;
    v->n.y = ny;
    v->n.z = nz;
}
#endif

#define SetF(f,i0,i1,i2) \
    { \
        (f)->v0 = i0; \
        (f)->v1 = i1; \
        (f)->v2 = i2; \
    }

/*-----------------------------------------------------------------------------
    Name        : ellipsoid_render
    Description : renders an ellipseObject structure at given radius
    Inputs      : ellipse - the ellipse object to render
                  radius - coordinate multiplier (unit sized ellipse scalar)
    Outputs     : draws an appropriate shape
    Return      :
----------------------------------------------------------------------------*/
void ellipsoid_render(ellipseObject* ellipse, real32 radius)
{
    alodIncPolys(ellipse->nf / 2);

    if (RGL)
    {
        sdword f, i, vert[3];
        real32 x, y, z;

        glShadeModel(GL_SMOOTH);
        glBegin(GL_TRIANGLES);

        for (f = 0; f < ellipse->nf; f++)
        {
            vert[0] = ellipse->f[f].v0;
            vert[1] = ellipse->f[f].v1;
            vert[2] = ellipse->f[f].v2;

            for (i = 0; i < 3; i++)
            {
                glNormal3fv((GLfloat*)&ellipse->v[vert[i]].n);

                x = ellipse->v[vert[i]].p.x * radius;
                y = ellipse->v[vert[i]].p.y * radius;
                z = ellipse->v[vert[i]].p.z * radius;
                glVertex3f(x, y, z);

#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberSmoothed++;
#endif
            }
        }

        glEnd();
    }
    else
    {
        sdword f, i, vert[3];
        real32 x, y, z;
        real32 modelview[16], modelviewInv[16];
        vector vertex;
        sdword lightOn;
        GLubyte colour[4];

        lightOn = rndLightingEnable(FALSE);

        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        shInvertMatrix(modelviewInv, modelview);

        if (bitTest(gDevcaps2, DEVSTAT2_NO_IALPHA) &&
            glIsEnabled(GL_BLEND))
        {
            glShadeModel(GL_FLAT);
        }
        else
        {
            glShadeModel(GL_SMOOTH);
        }
        glBegin(GL_TRIANGLES);

        for (f = 0; f < ellipse->nf; f++)
        {
            vert[0] = ellipse->f[f].v0;
            vert[1] = ellipse->f[f].v1;
            vert[2] = ellipse->f[f].v2;

            for (i = 0; i < 3; i++)
            {
                glNormal3fv((GLfloat*)&ellipse->v[vert[i]].n);

                x = ellipse->v[vert[i]].p.x * radius;
                y = ellipse->v[vert[i]].p.y * radius;
                z = ellipse->v[vert[i]].p.z * radius;

                vertex.x = x;
                vertex.y = y;
                vertex.z = z;
                colour[0] = gCloudColor[0];
                colour[1] = gCloudColor[1];
                colour[2] = gCloudColor[2];
                colour[3] = gCloudColor[3];
                shSpecularColour(0, 0, &vertex, (vector*)&ellipse->v[vert[i]].n,
                                 colour, modelview, modelviewInv);
                glColor4ub(colour[0], colour[1], colour[2], colour[3]);

                glVertex3f(x, y, z);

#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberSmoothed++;
#endif
            }
        }

        glEnd();
        rndLightingEnable(lightOn);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ellipsoid_render_as
    Description : renders an ellipseObject structure at given radius as points at
                  provided translation
    Inputs      : ellipse - the ellipse object to render
                  what - GL_TRIANGLES or GL_POINTS
                  offset - add this to vertex position
    Outputs     : draws an appropriate shape
    Return      :
----------------------------------------------------------------------------*/
void ellipsoid_render_as(ellipseObject* ellipse, int what, vector* offset)
{
    sdword f, i, vert[3];
    real32 x, y, z;

    glBegin(what);

    for (f = 0; f < ellipse->nf; f++)
    {
        vert[0] = ellipse->f[f].v0;
        vert[1] = ellipse->f[f].v1;
        vert[2] = ellipse->f[f].v2;

        for (i = 0; i < 3; i++)
        {
            glNormal3fv((GLfloat*)&ellipse->v[vert[i]].n);

            x = ellipse->v[vert[i]].p.x + offset->x;
            y = ellipse->v[vert[i]].p.y + offset->y;
            z = ellipse->v[vert[i]].p.z + offset->z;
            glVertex3f(x, y, z);

#if RND_POLY_STATS
            rndNumberPolys++;
            rndNumberSmoothed++;
#endif
        }
    }

    glEnd();
}

void ellipsoid_render_as_radii(ellipseObject* ellipse, int what, vector* offset, real32 ra, real32 rb, real32 rc)
{
    sdword f, i, vert[3];
    real32 x, y, z;

    glBegin(what);

    for (f = 0; f < ellipse->nf; f++)
    {
        vert[0] = ellipse->f[f].v0;
        vert[1] = ellipse->f[f].v1;
        vert[2] = ellipse->f[f].v2;

        for (i = 0; i < 3; i++)
        {
            glNormal3fv((GLfloat*)&ellipse->v[vert[i]].n);

            x = ra * ellipse->v[vert[i]].p.x + offset->x;
            y = rb * ellipse->v[vert[i]].p.y + offset->y;
            z = rc * ellipse->v[vert[i]].p.z + offset->z;
            glVertex3f(x, y, z);

#if RND_POLY_STATS
            rndNumberPolys++;
            rndNumberSmoothed++;
#endif
        }
    }

    glEnd();
}

/*-----------------------------------------------------------------------------
    Name        : ellipsoid_free
    Description : frees the memory allocated for all ellipse stuff
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ellipsoid_free()
{
    udword i;

    if (table)
    {
        memFree(table);
        table = NULL;
    }
    if (octant)
    {
        memFree(octant);
        octant = NULL;
    }

    for (i = 0; i < 4; i++)
    {
        if (ellipseLOD[i].v)
        {
            memFree(ellipseLOD[i].v);
            memFree(ellipseLOD[i].f);
            ellipseLOD[i].v = NULL;
            ellipseLOD[i].f = NULL;
        }
        if (genericEllipse[i].v)
        {
            memFree(genericEllipse[i].v);
            memFree(genericEllipse[i].f);
            genericEllipse[i].v = NULL;
            genericEllipse[i].f = NULL;
        }
    }
    if (ellipseLOD[4].v)
    {
        memFree(ellipseLOD[4].v);
        memFree(ellipseLOD[4].f);
        ellipseLOD[4].v = NULL;
        ellipseLOD[4].f = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ellipsoid_init
    Description : allocates and generates ellipse luts
    Inputs      : n - max depth
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ellipsoid_init(sdword n)
{
    sdword n_table, i, j, k, l, m, h, d;
    slot* t0;
    slot* t1;
    slot* t2;
    real32 theta;

    if (n > n_max)
    {
        n_max = n;

        if (table)
        {
            memFree(table);
        }

        if ((n_table = ((n-1)*n)/2) == 0)
        {
            table = NULL;
        }
        else
        {
            table = (slot*)memAlloc(n_table * sizeof(slot), "ellipse table", NonVolatile);
        }

        if (octant)
        {
            memFree(octant);
        }

        octant = (vertex*)memAlloc(((n+1)*(n+2))/2 * sizeof(vertex), "ellipse octant", NonVolatile);

        for (t0 = table, k = n_table; k > 0; k--, t0++)
        {
            t0->flag = None;
        }

        for (t0 = table, k = 0, l = 1, m = 3, i = 2; i <= n_max; i++)
        {
            l += m;
            m += 2;
            h = n_max / i - 1;

            for (t1 = t0+i - 2, j = 1; j < i; j++, k++, t0++, t1--)
            {
                if (t0->flag == None)
                {
                    theta = (real32)((M_PI_2 * j) / i);
                    t0->cos = t1->sin = (real32)cos((double)theta);
                    t0->sin = t1->cos = (real32)sin((double)theta);
                    t0->flag = t1->flag = Only;
                }

                if (t0->flag == Only)
                {
                    t0->flag = Done;

                    for (d = k+l, t2 = t0; h > 0; h--)
                    {
                        t2 += d;
                        d += l;
                        t2->cos = t0->cos;
                        t2->sin = t0->sin;
                        t2->flag = Done;
                    }
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : ellipsoid_octant
    Description : generates a single ellipse octant
    Inputs      : n - depth
                  a, b, c - radii
    Outputs     : sets verts and norms of global octant structure
    Return      :
----------------------------------------------------------------------------*/
static void ellipsoid_octant(sdword n, real32 a, real32 b, real32 c)
{
    sdword i, j;
    real32 a_1, b_1, c_1;
    real32 cos_ph, sin_ph, px, py, pz, nx, ny, nz, nznz, rnorm, tmp;
    vertex* o = octant;
    slot* table_th;
    slot* table_ph;

    a_1 = 1.0f / a;
    b_1 = 1.0f / b;
    c_1 = 1.0f / c;
    o = octant;
    table_th = table;
    table_ph = table + ((n-1)*(n-2))/2;

    SetV(o, 0.0f, 0.0f, c, 0.0f, 0.0f, 1.0f);
    o++;

    for (i = 1; i < n; i++, table_ph++)
    {
        cos_ph = table_ph->cos;
        sin_ph = table_ph->sin;
        pz = cos_ph * c;
        nz = cos_ph * c_1;
        nznz = nz * nz;

        px = sin_ph * a;
        nx = sin_ph * a_1;
        rnorm = 1.0f / fsqrt(nx*nx + nznz);
        SetV(o, px, 0.0f, pz, nx*rnorm, 0.0f, nz*rnorm);
        o++;

        for (j = i; --j > 0; table_th++)
        {
            tmp = table_th->cos * sin_ph;
            px = tmp * a;
            nx = tmp * a_1;
            tmp = table_th->sin * sin_ph;
            py = tmp * b;
            ny = tmp * b_1;
            rnorm = 1.0f / fsqrt(nx*nx + ny*ny + nznz);
            SetV(o, px, py, pz, nx*rnorm, ny*rnorm, nz*rnorm);
            o++;
        }

        py = sin_ph * b;
        ny = sin_ph * b_1;
        rnorm = 1.0f / fsqrt(ny*ny + nznz);
        SetV(o, 0.0f, py, pz, 0.0f, ny*rnorm, nz*rnorm);
        o++;
    }

    SetV(o, a, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    o++;

    for (j = i; --j > 0; table_th++)
    {
        tmp = table_th->cos;
        px = tmp * a;
        nx = tmp * a_1;
        tmp = table_th->sin;
        py = tmp * b;
        ny = tmp * b_1;
        rnorm = 1.0f / fsqrt(nx*nx + ny*ny);
        SetV(o, px, py, 0.0f, nx*rnorm, ny*rnorm, 0.0f);
        o++;
    }

    SetV(o, 0.0f, b, 0.0f, 0.0f, 1.0f, 0.0f);
}

/*-----------------------------------------------------------------------------
    Name        : ellipsoid_seq
    Description : generates an entire ellipsoid
    Inputs      : ellipsoid - ellipse structure to fill in
                  n - depth
                  a, b, c - radii
    Outputs     : a generated ellipse object
    Return      :
----------------------------------------------------------------------------*/
void ellipsoid_seq(ellipseObject* ellipsoid, sdword n, real32 a, real32 b, real32 c)
{
    vertex *v, *o;
    face *f;
    sdword i, j, ko, kv, kw, kv0, kw0;

    /* Check parameters for validity. */
    if (n <= 0 || n_max < n || a <= 0.0 || b <= 0.0 || c <= 0.0)
    {
        ellipsoid->nv = 0; ellipsoid->v = NULL;
        ellipsoid->nf = 0; ellipsoid->f = NULL;
        return;
    }

    /* Initialize the base octant. */
    ellipsoid_octant(n, a, b, c);

    /* Allocate memories for vertices and faces. */
    ellipsoid->nv = 4*n*n + 2;
    ellipsoid->nf = 8*n*n;
    ellipsoid->v = (vertex*)memAlloc(ellipsoid->nv * sizeof(vertex), "ellipse verts", NonVolatile);
    ellipsoid->f = (face*)memAlloc(ellipsoid->nf * sizeof(face), "ellipse faces", NonVolatile);

    /* Generate vertices of the ellipsoid from octant[]. */
    v = ellipsoid->v;
    o = octant;
#define op o->p
#define on o->n
    SetV(v, op.x, op.y, op.z, on.x, on.y, on.z);        /* the north pole */
    v++;

    for (i = 0; ++i <= n;)
    {
        o += i;
        for (j = i; --j >= 0; o++, v++)                       /* 1st octant */
        {
            SetV(v, op.x, op.y, op.z, on.x, on.y, on.z);
        }
        for (j = i; --j >= 0; o--, v++)                       /* 2nd octant */
        {
            SetV(v, -op.x, op.y, op.z, -on.x, on.y, on.z);
        }
        for (j = i; --j >= 0; o++, v++)                       /* 3rd octant */
        {
            SetV(v, -op.x, -op.y, op.z, -on.x, -on.y, on.z);
        }
        for (j = i; --j >= 0; o--, v++)                       /* 4th octant */
        {
            SetV(v, op.x, -op.y, op.z, on.x, -on.y, on.z);
        }
    }

    for (; --i > 1;)
    {
        o -= i;
        for (j = i; --j > 0; o++, v++)                        /* 5th octant */
        {
            SetV(v, op.x, op.y, -op.z, on.x, on.y, -on.z);
        }
        for (j = i; --j > 0; o--, v++)                        /* 6th octant */
        {
            SetV(v, -op.x,  op.y, -op.z, -on.x,  on.y, -on.z);
        }
        for (j = i; --j > 0; o++, v++)                        /* 7th octant */
        {
            SetV(v, -op.x, -op.y, -op.z, -on.x, -on.y, -on.z);
        }
        for (j = i; --j > 0; o--, v++)                        /* 8th octant */
        {
            SetV(v, op.x, -op.y, -op.z, on.x, -on.y, -on.z);
        }
    }

    o--;
    SetV(v, -op.x, -op.y, -op.z, -on.x, -on.y, -on.z);  /* the south pole */
#undef op
#undef on

    /* Generate triangular faces of the ellipsoid. */
    f = ellipsoid->f;
    kv = 0, kw = 1;

    for (i = 0; i < n; i++)
    {
        kv0 = kv, kw0 = kw;

        for (ko = 1; ko <= 3; ko++)                /* the 1st, 2nd, 3rd octants */
        {
            for (j = i;; j--)
            {
                SetF(f, kv, kw, ++kw);
                f++;
                if (j == 0) break;
                SetF(f, kv, kw, ++kv);
                f++;
            }
        }

        for (j = i;; j--)                          /* the 4th octant */
        {
            if (j == 0)
            {
                SetF(f, kv0, kw, kw0);
                kv++;
                kw++;
                f++;
                break;
            }
            SetF(f, kv, kw, ++kw);
            f++;
            if (j == 1)
            {
                SetF(f, kv, kw, kv0);
                f++;
            }
            else
            {
                SetF(f, kv, kw, ++kv);
                f++;
            }
        }
    }

    for (; --i >= 0;)
    {
        kv0 = kv, kw0 = kw;
        for (ko = 5; ko <= 7; ko++)                /* the 5th, 6th, 7th octants */
        {
            for (j = i;; j--)
            {
                SetF(f, kv, kw, ++kv);
                f++;
                if (j == 0) break;
                SetF(f, kv, kw, ++kw);
                f++;
            }
        }
        for (j = i;; j--)                          /* the 8th octant */
        {
            if (j == 0)
            {
                SetF(f, kv, kw0, kv0);
                kv++;
                kw++;
                f++;
                break;
            }

            SetF(f, kv, kw, ++kv);
            f++;

            if (j == 1)
            {
                SetF(f, kv, kw, kw0);
                f++;
            }
            else
            {
                SetF(f, kv, kw, ++kw);
                f++;
            }
        }
    }
}

/*
 * LITTLE HELPERS
 */

static real64 randomAngle(udword a)
{
    udword r = ranRandom(RAN_Clouds) % a;
    return DEG_TO_RAD((real64)r);
}

static void _homogenize(vector* v, hvector* h)
{
    real32 oneOverW;

    vecGrabVecFromHVec(*v, *h);
    if (h->w == 0.0f || h->w == 1.0f)
        return;

    oneOverW = 1.0f / h->w;
    v->x *= oneOverW;
    v->y *= oneOverW;
    v->z *= oneOverW;
}

real32 cloudRealDist(real32 n, real32 d)
{
    real32 r = (real32)((real64)(ranRandom(RAN_Clouds) % 1000000) / 1000000.0);
    real32 sign = (ranRandom(RAN_Clouds) % 2 == 0) ? -1.0f : 1.0f;
    if (d < 0.0f)
        sign = -1.0f;
    return n + sign*r*d;
}

/*
 * FUNCTIONS
 */

/*-----------------------------------------------------------------------------
    Name        : cloudStartup
    Description : allocates cloud data, generates spheres, sets up fog
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudStartup()
{
    fogColor[0] = DUSTCLOUD_FOG_RED;
    fogColor[1] = DUSTCLOUD_FOG_GREEN;
    fogColor[2] = DUSTCLOUD_FOG_BLUE;
    fogColor[3] = 0.5f;

    dontNebulate = FALSE;

    glFogi(GL_FOG_MODE, GL_LINEAR);

    ellipsoid_init(5);
    ellipsoid_seq(&ellipseLOD[0], 5, 1.0f, 1.0f, 1.0f);
    ellipsoid_seq(&ellipseLOD[1], 4, 1.0f, 1.0f, 1.0f);
    ellipsoid_seq(&ellipseLOD[2], 3, 1.0f, 1.0f, 1.0f);
    ellipsoid_seq(&ellipseLOD[3], 2, 1.0f, 1.0f, 1.0f);
    ellipsoid_seq(&ellipseLOD[4], 1, 1.0f, 1.0f, 1.0f);
    g_NoBumpy = TRUE;
    ellipsoid_seq(&genericEllipse[0], 5, 1.0f, 1.0f, 1.0f);
    ellipsoid_seq(&genericEllipse[1], 4, 1.0f, 1.0f, 1.0f);
    ellipsoid_seq(&genericEllipse[2], 3, 1.0f, 1.0f, 1.0f);
    ellipsoid_seq(&genericEllipse[3], 2, 1.0f, 1.0f, 1.0f);
    g_NoBumpy = FALSE;
}

//convenience
void _clearcolorUniverse()
{
    color c = universe.backgroundColor;
    if (!smSensorsActive)
    {
        rndSetClearColor(c);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cloudReset
    Description : resets cloud things
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudReset()
{
    _clearcolorUniverse();
    rndFogOn = FALSE;
    dontNebulate = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : cloudShutdown
    Description : shuts down the cloud stuff, frees memory &c
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudShutdown()
{
    cloudReset();

    ellipsoid_free();
}

/*-----------------------------------------------------------------------------
    Name        : cloudCreateSystem
    Description : creates a new cloud system
    Inputs      : radius - radius of the sphere
                  variance - randomness parameter
    Outputs     :
    Return      : new cloudSystem, with not much initialized
----------------------------------------------------------------------------*/
cloudSystem* cloudCreateSystem(real32 radius, real32 variance)
{
    udword i;
    cloudSystem* system = memAlloc(sizeof(cloudSystem), "cloud system", NonVolatile);

    system->flags = 0;
    system->radius = radius;

    system->position = NULL;
    system->rotation = NULL;

    system->healthFactor = 1.0f;

    system->cloudColor = colRGB((ubyte)(cloudRealDist(DUSTCLOUD_RED, DUSTCLOUD_RED_DIST) * 255.0f),
                                (ubyte)(cloudRealDist(DUSTCLOUD_GREEN, DUSTCLOUD_GREEN_DIST) * 255.0f),
                                (ubyte)(cloudRealDist(DUSTCLOUD_BLUE, DUSTCLOUD_BLUE_DIST) * 255.0f));

    system->charge = 0.0f;

    for (i = 0; i < MAX_BURSTS; i++)
    {
        system->bursts[i] = 0;
    }

    return system;
}

/*-----------------------------------------------------------------------------
    Name        : cloudDeleteSystem
    Description : frees the memory used by the given system
    Inputs      : system - the cloud system
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudDeleteSystem(cloudSystem* system)
{
    udword i;

    for (i = 0; i < MAX_BURSTS; i++)
    {
        if (system->bursts[i] != 0)
        {
            lightning* l = cloudLightningFromHandle(system->bursts[i]);
            cloudKillLightning(l);
        }
    }

    //free the container
    memFree(system);
}

/*-----------------------------------------------------------------------------
    Name        : cloudRandomSphericalPoint
    Description : distributes a point randomly on a unit sphere
    Inputs      : vec - the vector to hold the output point
    Outputs     : vec - contains the output point
    Return      :
----------------------------------------------------------------------------*/
void cloudRandomSphericalPoint(vector* vec)
{
    hvector hvec, rvec;
    hmatrix result;

    real64 a = randomAngle(360);
    real32 sinTheta = (real32)sin(a);
    real32 cosTheta = (real32)cos(a);

    vecZeroVector(*vec);
    vec->x = 1.0f;
    vecMakeHVecFromVec(hvec, *vec);
    hmatMakeRotAboutZ(&result, sinTheta, cosTheta);
    hmatMultiplyHMatByHVec(&rvec, &result, &hvec);

    a = randomAngle(360);
    sinTheta = (real32)sin(a);
    cosTheta = (real32)cos(a);

    hmatMakeRotAboutX(&result, sinTheta, cosTheta);
    hmatMultiplyHMatByHVec(&hvec, &result, &rvec);
    rvec = hvec;
    hmatMultiplyHMatByHVec(&rvec, &result, &hvec);
    _homogenize(vec, &rvec);
    vecNormalize(vec);
}

/*-----------------------------------------------------------------------------
    Name        : cloudPointInSphere
    Description : scales a point representing a point on a unit sphere by a
                  permutation of the provided radius
    Inputs      : point - the point
                  radius - base radius
    Outputs     : point is multiplied by a scalar
    Return      :
----------------------------------------------------------------------------*/
void cloudPointInSphere(vector* point, real32 radius)
{
    vecMultiplyByScalar(*point, cloudRealDist(0.75f * radius, 0.4f * radius));
}

real32 cloudLightningDeviation()
{
    return(0.54f * ((real32)(ranRandom(RAN_Clouds) % 100) - 50.0f));
}

void cloudGetVcross(vector* from, vector* to, vector* vcross)
{
    vector veye, vseg;

    vecSub(veye, *from, mrCamera->eyeposition);
    vecSub(vseg, *to, *from);
    vecNormalize(&veye);
    vecNormalize(&vseg);
    vecCrossProduct(*vcross, vseg, veye);
    vecNormalize(vcross);
}

/*-----------------------------------------------------------------------------
    Name        : cloudRenderLightning
    Description : renders a lightning effect between 2 points
    Inputs      : pa - start point
                  pb - end point
                  depth - depth of recursion
                  lod - the lod number
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudRenderLightning(vector* pa, vector* pb, udword depth, sdword lod, vector* vcross)
{
    vector from, to, mid;
    real32 width;

    switch (lod)
    {
    case 0:
    case 1:
        width = CLOUD_LIGHTNING_WIDTH_UPCLOSE;
        break;
    case 2:
    case 3:
        width = CLOUD_LIGHTNING_WIDTH_FARAWAY;
        break;
    default:
        width = 0.0f;
    }

    from = *pa;
    to = *pb;

    mid.x = (from.x + to.x)*0.5f + cloudLightningDeviation();
    mid.y = (from.y + to.y)*0.5f + cloudLightningDeviation();
    mid.z = (from.z + to.z)*0.5f + cloudLightningDeviation();

    if (depth == 0)
    {
        if (width == 0.0f)
        {
            glColor3f(CLOUD_LIGHTNING_LINE_RED, CLOUD_LIGHTNING_LINE_GREEN, CLOUD_LIGHTNING_LINE_BLUE);
            glBegin(GL_LINES);
            glVertex3fv((real32*)&from);
            glVertex3fv((real32*)&mid);
            glVertex3fv((real32*)&mid);
            glVertex3fv((real32*)&to);
            glEnd();
        }
        else
        {
            vector fromHi, midHi, toHi;
            vector _from, _mid, _to;
            vector subAmount;

            // ----

            vecScalarMultiply(fromHi, *vcross, CLOUD_LIGHTNING_FRINGE_SCALAR * width);
            vecAddTo(fromHi, from);

            vecScalarMultiply(midHi, *vcross,
                CLOUD_LIGHTNING_FRINGE_SCALAR * CLOUD_LIGHTNING_MIDPOINT_SCALAR * width);
            vecAddTo(midHi, mid);

            vecScalarMultiply(toHi, *vcross, CLOUD_LIGHTNING_FRINGE_SCALAR * width);
            vecAddTo(toHi, to);

            _from = from;
            _mid = mid;
            _to = to;
            vecScalarMultiply(subAmount, *vcross, (CLOUD_LIGHTNING_FRINGE_SCALAR - 1.0f) * width);
            vecSubFrom(_from, subAmount);
            vecSubFrom(_mid, subAmount);
            vecSubFrom(_to, subAmount);

            glColor4f(CLOUD_LIGHTNING_FRINGE_RED, CLOUD_LIGHTNING_FRINGE_GREEN,
                      CLOUD_LIGHTNING_FRINGE_BLUE, CLOUD_LIGHTNING_FRINGE_ALPHA);
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);

            glBegin(GL_QUADS);

            glVertex3fv((GLfloat*)&_from);
            glVertex3fv((GLfloat*)&fromHi);
            glVertex3fv((GLfloat*)&midHi);
            glVertex3fv((GLfloat*)&_mid);

            glVertex3fv((GLfloat*)&_mid);
            glVertex3fv((GLfloat*)&midHi);
            glVertex3fv((GLfloat*)&toHi);
            glVertex3fv((GLfloat*)&_to);

            glEnd();

            // ----

            vecScalarMultiply(fromHi, *vcross, width);
            vecAddTo(fromHi, from);

            vecScalarMultiply(midHi, *vcross, CLOUD_LIGHTNING_MIDPOINT_SCALAR * width);
            vecAddTo(midHi, mid);

            vecScalarMultiply(toHi, *vcross, width);
            vecAddTo(toHi, to);
            glColor4f(CLOUD_LIGHTNING_MAIN_RED, CLOUD_LIGHTNING_MAIN_GREEN,
                      CLOUD_LIGHTNING_MAIN_BLUE, CLOUD_LIGHTNING_MAIN_ALPHA);

            glBegin(GL_QUADS);

            glVertex3fv((GLfloat*)&from);
            glVertex3fv((GLfloat*)&fromHi);
            glVertex3fv((GLfloat*)&midHi);
            glVertex3fv((GLfloat*)&mid);

            glVertex3fv((GLfloat*)&mid);
            glVertex3fv((GLfloat*)&midHi);
            glVertex3fv((GLfloat*)&toHi);
            glVertex3fv((GLfloat*)&to);

            glEnd();

            // ----

            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
        }
    }
    else
    {
        cloudRenderLightning(&from, &mid, depth-1, lod, vcross);
        cloudRenderLightning(&mid, &to, depth-1, lod, vcross);
    }
}

bool8 cloudHasLightning(cloudSystem* system)
{
    udword i;

    for (i = 0; i < MAX_BURSTS; i++)
    {
        if (system->bursts[i] != 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

color cloudBrighten(color c, real32 charge)
{
    sdword red = colRed(c);
    sdword green = colGreen(c);
    sdword blue = colBlue(c);
    real32 adjCharge = 2.0f * charge;
    sdword sBRIGHTER;

    if (adjCharge > 1.0f) adjCharge = 1.0f;
    sBRIGHTER = (sdword)((real32)CLOUD_LIGHTNING_BRIGHTEN * adjCharge);
    //FIXME: add slight randomization (flitter)

    red += sBRIGHTER;
    green += sBRIGHTER;
    blue += sBRIGHTER;

    if (red > 255) red = 255;
    if (green > 255) green = 255;
    if (blue > 255) blue = 255;

    return colRGB(red, green, blue);
}

/*-----------------------------------------------------------------------------
    Name        : cloudRenderSystem
    Description : renders (displays) a cloud system
    Inputs      : mesh - no longer used
                  system - the cloud system
                  lod - lod to render at
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudRenderSystem(void* mesh, cloudSystem* system, sdword lod)
{
    GLfloat radius;
    vector origin = {0.0f, 0.0f, 0.0f};
    color cloudColor = system->cloudColor;
    udword i;
    bool fogOn;

    if (system == NULL)
    {
        return;
    }

    fogOn = glIsEnabled(GL_FOG);
    if (fogOn) glDisable(GL_FOG);

    radius = system->radius * system->healthFactor;

    if (RGL)
    {
        if (rglIsClipped((GLfloat*)&origin, radius, radius, radius))
        {
            return;
        }
    }
    if (clipBBoxIsClipped((real32*)&origin, radius, radius, radius))
    {
        return;
    }

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    rndTextureEnable(FALSE);
    if (RGL)
    {
        rglFeature(RGL_SPECULAR_RENDER);
        if (glIsEnabled(GL_POLYGON_STIPPLE))
        {
            glShadeModel(GL_FLAT);
        }
        else
        {
            glShadeModel(GL_SMOOTH);
        }
    }
    else
    {
        glShadeModel(GL_SMOOTH);
    }
    glEnable(GL_BLEND);
    rndAdditiveBlends(FALSE);

    glDepthMask(GL_FALSE);

    if (cloudHasLightning(system))
    {
        cloudColor = cloudBrighten(cloudColor, system->charge);
    }

    {
        GLubyte alpha;
        if (glCapFastFeature(GL_BLEND) || glIsEnabled(GL_POLYGON_STIPPLE))
        {
            alpha = 127;
        }
        else
        {
            alpha = 192;
        }
        gCloudColor[0] = colRed(cloudColor);
        gCloudColor[1] = colGreen(cloudColor);
        gCloudColor[2] = colBlue(cloudColor);
        gCloudColor[3] = alpha;
        glColor4ub(gCloudColor[0], gCloudColor[1], gCloudColor[2], gCloudColor[3]);
    }

    if (glCapFastFeature(GL_BLEND))
    {
        switch (lod)
        {
        case 0:
//            ellipsoid_render(&ellipseLOD[0], radius);
//            break;
        case 1:
//            ellipsoid_render(&ellipseLOD[1], radius);
//            break;
        case 2:
            ellipsoid_render(&ellipseLOD[2], radius);
            break;
        default:
            ellipsoid_render(&ellipseLOD[3], radius);
        }
    }
    else
    {
        ellipsoid_render(&ellipseLOD[3], radius);
    }

    glDisable(GL_BLEND);
    if (RGL) rglFeature(RGL_NORMAL_RENDER);

    rndLightingEnable(FALSE);

    //lightning effect in effect
    for (i = 0; i < MAX_BURSTS; i++)
    {
        udword depth;
        real32 distance;
        vector pointA, pointB, distvec;
        lightning* l;

        if (system->bursts[i] == 0)
        {
            continue;
        }

        l = cloudLightningFromHandle(system->bursts[i]);

        pointA = l->pointA;
        pointB = l->pointB;

        vecSub(distvec, pointB, pointA);
        distance = fsqrt(vecMagnitudeSquared(distvec));

        //lod for lightning jaggies
        if (distance > CLOUD_LDT0) depth = 3;
        else if (distance > CLOUD_LDT1) depth = 2;
        else depth = 1;

        {
            vector vcross;
            cloudGetVcross(&pointA, &pointB, &vcross);
            rndAdditiveBlends(TRUE);
            cloudRenderLightning(&pointA, &pointB, depth, lod, &vcross);
        }
    }

    glDepthMask(GL_TRUE);
    rndLightingEnable(TRUE);
    rndAdditiveBlends(FALSE);
    if (fogOn) glEnable(GL_FOG);
}

//ship lightning render & think fn
void cloudRenderAndUpdateLightning(lightning* l, sdword lod)
{
    vector pointA, pointB, distvec;
    real32 distance;
    udword depth;

    pointA = l->pointA;
    pointB = l->pointB;

    vecSub(distvec, pointB, pointA);
    distance = vecMagnitudeSquared(distvec);
    distance = fsqrt(distance);

    if (distance > CLOUD_LDT0) depth = 3;
    else if (distance > CLOUD_LDT1) depth = 2;
    else depth = 1;

    rndTextureEnable(FALSE);
    glShadeModel(GL_SMOOTH);
    rndLightingEnable(FALSE);
    glDepthMask(GL_FALSE);

    {
        vector vcross;
        cloudGetVcross(&pointA, &pointB, &vcross);
        cloudRenderLightning(&pointA, &pointB, depth, lod, &vcross);
    }

    glDepthMask(GL_TRUE);
    rndLightingEnable(TRUE);
}

real32 cloudLightningDistance(lightning* l)
{
    vector dist;

    vecSub(dist, l->pointB, l->pointA);
    return fsqrt(vecMagnitudeSquared(dist));
}

void cloudCapLightningDistance(lightning* l)
{
    real32 dist = cloudLightningDistance(l);
    if (dist > CLOUD_LIGHTNING_MAXDIST)
    {
        vector diff;

        //get direction
        vecSub(diff, l->pointB, l->pointA);
        vecNormalize(&diff);

        //make direction a scaled vector
        vecMultiplyByScalar(diff, CLOUD_LIGHTNING_MAXDIST);
        vecAddTo(diff, l->pointA);

        //assign as point b
        l->pointB = diff;
    }
}

void cloudCapLightningDistanceWithSuppliedRadius(lightning* l, real32 radius)
{
    real32 dist = cloudLightningDistance(l);
    if (dist > radius)
    {
        vector diff;

        //get direction
        vecSub(diff, l->pointB, l->pointA);
        vecNormalize(&diff);

        //make direction a scaled vector
        vecMultiplyByScalar(diff, radius);
        vecAddTo(diff, l->pointA);

        //assign as point b
        l->pointB = diff;
    }
}

/*-----------------------------------------------------------------------------
    Name        : cloudApplyDamage
    Description : actually applies damage to a ship
    Inputs      : target - the ship
                  damagetaken - the damage to take
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudApplyDamage(SpaceObjRotImpTarg* target, real32 damagetaken)
{
    ApplyDamageToTarget(target, damagetaken, GS_SmallIonCannon,DEATH_Killed_By_Cloud,99);
}

/*-----------------------------------------------------------------------------
    Name        : cloudNewLightning
    Description : creates a new lightning effect for attachment to a ship
    Inputs      : system - the parent structure (generic type)
                  radius - radius of the target, for sizing the lightning
    Outputs     :
    Return      : a fresh lightning structure
----------------------------------------------------------------------------*/
lightning* cloudNewLightning(void* system, real32 radius)
{
    lhandle handle;
    lightning* l;

    handle = cloudGetFreshLightningHandle();
    l = cloudLightningFromHandle(handle);
    cloudInitLightning(l);
    cloudLiveLightning(l);
    cloudMainLightning(l);
    l->parent = (cloudSystem*)system;       //not necessarily a cloudSystem, mind you

    //frame, not tick-based
    l->countdown = (ubyte)((ranRandom(RAN_Clouds) % 18) + 14);

    cloudRandomSphericalPoint(&l->pointA);
    cloudPointInSphere(&l->pointA, radius);

    cloudRandomSphericalPoint(&l->pointB);
    cloudPointInSphere(&l->pointB, radius);

    l->distance = 5.0f * cloudLightningDistance(l);

    cloudCapLightningDistanceWithSuppliedRadius(l, 0.5f * radius);

    return l;
}

/*-----------------------------------------------------------------------------
    Name        : cloudZapLoop
    Description : zaps nearby ships with electric death
    Inputs      : system - the cloud system
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudZapLoop(cloudSystem* system)
{
    Node* node;
    SpaceObj* spaceobj;
    real32 radius, radiusSquared;
    udword hits;

    radius = CLOUD_RADIAL_DAMAGE_SCALAR * system->radius * system->healthFactor;
    radiusSquared = radius * radius;
    hits = 0;

    //gather loop
    node = universe.SpaceObjList.head;
    while (node != NULL)
    {
        spaceobj = (SpaceObj*)listGetStructOfNode(node);

        if (spaceobj->objtype == OBJ_ShipType)
        {
            vector diff;

            vecSub(diff, *system->position, spaceobj->posinfo.position);
            if (vecMagnitudeSquared(diff) <= radiusSquared)
            {
                hits++;
            }
        }

        node = node->next;
    }

    //damage loop
    if (hits > 0)
    {
        real32 damage;

        damage = (CLOUD_ZAP_SCALAR * ((real32)system->maxhealth / (real32)hits));

        node = universe.SpaceObjList.head;
        while (node != NULL)
        {
            spaceobj = (SpaceObj*)listGetStructOfNode(node);

            if (spaceobj->objtype == OBJ_ShipType)
            {
                vector diff;

                vecSub(diff, *system->position, spaceobj->posinfo.position);
                if (vecMagnitudeSquared(diff) <= radiusSquared)
                {
                    Ship* ship = (Ship*)spaceobj;
                    udword i;

                    cloudApplyDamage((SpaceObjRotImpTarg*)spaceobj, damage);

                    for (i = 0; i < 2; i++)
                    {
                        if (ship->lightning[i] != NULL)
                        {
                            cloudKillLightning(ship->lightning[i]);
                        }

                        ship->lightning[i] = cloudNewLightning(
                            (void*)ship,
                            ship->staticinfo->staticheader.staticCollInfo.collspheresize);
                    }
                }
            }

            node = node->next;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : cloudNewBurst
    Description : create a long-lived burst of lightning in the system
    Inputs      : system - the cloud system
                  i - burst slot in system
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudNewBurst(cloudSystem* system, udword i)
{
    lhandle handle;
    lightning* l;
    real32 radius;

    radius = 0.9f * system->radius * system->healthFactor;

    handle = cloudGetFreshLightningHandle();
    system->bursts[i] = handle;
    l = cloudLightningFromHandle(handle);
    cloudInitLightning(l);
    cloudLiveLightning(l);
    cloudMainLightning(l);
    l->parent = system;

    l->countdown = (ubyte)((ranRandom(RAN_Clouds) % 2) + 1);

    cloudRandomSphericalPoint(&l->pointA);
    cloudPointInSphere(&l->pointA, radius);

    cloudRandomSphericalPoint(&l->pointB);
    cloudPointInSphere(&l->pointB, radius);

    l->distance = 10.0f * cloudLightningDistance(l);

    cloudCapLightningDistance(l);
}

/*-----------------------------------------------------------------------------
    Name        : cloudUpdateSystem
    Description : determines whether randomized lightning should be displayed.
                  maintains the state of associated lightning objects
    Inputs      : system - the cloud system structure
    Outputs     : possibly alters the cloud state
    Return      :
----------------------------------------------------------------------------*/
void cloudUpdateSystem(cloudSystem* system)
{
    udword i, thresh;

    thresh = (udword)(system->charge * 20.0f);

    if (bitTest(system->flags, LIGHTNING_DISCHARGING))
    {
        //system is discharging, clear the flag and reset the charge
        bitClear(system->flags, LIGHTNING_DISCHARGING);
        system->charge = CLOUD_DISCHARGE_CHARGE;

        //zap internal and nearby external ships
        cloudZapLoop(system);

        //max out bursts on system
        for (i = 0; i < MAX_BURSTS; i++)
        {
            if (system->bursts[i] == 0)
            {
                cloudNewBurst(system, i);
            }
        }
    }

    for (i = 0; i < MAX_BURSTS; i++)
    {
        lightning* l;

        if (system->bursts[i] != 0)
        {
            l = cloudLightningFromHandle(system->bursts[i]);
            l->countdown--;

            if (l->countdown == 0)
            {
                real32 dist = cloudLightningDistance(l);
                //countdown expired, have we traveled far enough?
                if (dist >= l->distance)
                {
                    //done, kill
                    system->bursts[i] = 0;
                    cloudKillLightning(l);
                }
                else
                {
                    real32 radius = 0.9f * system->radius * system->healthFactor;

                    //decrement our distance counter
                    l->distance -= dist;

                    //reset the countdown
                    l->countdown = (ubyte)((ranRandom(RAN_Clouds) % 2) + 1);

                    //endpoint -> startpoint
                    memcpy(&l->pointA, &l->pointB, sizeof(vector));

                    //new endpoint
                    cloudRandomSphericalPoint(&l->pointB);
                    cloudPointInSphere(&l->pointB, radius);

                    //cap distance
                    cloudCapLightningDistance(l);
                }
            }
            else
            {
#if 1
                real32 dist = cloudLightningDistance(l);
                //maybe spawn a new guy right now to cover more of the distance
                if (dist < l->distance)
                {
                    udword j, hit;
                    for (j = hit = 0; j < MAX_BURSTS; j++)
                    {
                        if (system->bursts[j] == 0)
                        {
                            hit = 1;
                            break;
                        }
                    }

                    if (hit != 0)
                    {
                        //found a slot, use it
                        real32 radius;
                        lightning* light;
                        lhandle handle;

                        //get a new lightning set up
                        handle = cloudGetFreshLightningHandle();
                        system->bursts[j] = handle;
                        light = cloudLightningFromHandle(handle);
                        cloudInitLightning(light);
                        cloudLiveLightning(light);
                        cloudMainLightning(light);
                        light->parent = system;

                        radius = 0.9f * system->radius * system->healthFactor;
                        light->countdown = (ubyte)((ranRandom(RAN_Clouds) % 2) + 1);

                        //startpoint is previous lightning's endpoint
                        memcpy(&light->pointA, &l->pointB, sizeof(vector));

                        //new endpoint
                        cloudRandomSphericalPoint(&light->pointB);
                        cloudPointInSphere(&light->pointB, radius);

                        //cap distance traveled
                        cloudCapLightningDistance(light);

                        //set distance left to travel
                        light->distance = l->distance - cloudLightningDistance(light);

                        //previous guy's time is up
                        l->distance = 0.0f;
                    }
                }
#endif
            }
        }
        else if (ranRandom(RAN_Clouds) % 1000 < thresh)
        {
            real32 radius;
            lhandle handle;
            lightning* l;

            //initialization
            handle = cloudGetFreshLightningHandle();
            system->bursts[i] = handle;
            l = cloudLightningFromHandle(handle);
            cloudInitLightning(l);
            cloudLiveLightning(l);
            cloudMainLightning(l);
            l->parent = system;

            //timespan
            radius = 0.9f * system->radius * system->healthFactor;
            l->countdown = (ubyte)((ranRandom(RAN_Clouds) % 2) + 1);

            //point a
            {
                udword k, hit;

                //try to start at the endpoint of an existing burst
                for (k = hit = 0; k < MAX_BURSTS; k++)
                {
                    if (system->bursts[k] != 0)
                    {
                        lightning* light = cloudLightningFromHandle(system->bursts[k]);
                        memcpy(&l->pointA, &light->pointB, sizeof(vector));
                        hit = 1;
                        break;
                    }
                }

                if (hit == 0)
                {
                    cloudRandomSphericalPoint(&l->pointA);
                    cloudPointInSphere(&l->pointA, radius);
                }
            }

            //point b
            cloudRandomSphericalPoint(&l->pointB);
            cloudPointInSphere(&l->pointB, radius);

            //distance
            l->distance = 3.0f * cloudLightningDistance(l);

            //cap distance
            cloudCapLightningDistance(l);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : cloudSetFog
    Description : cloud "task" function, scans the renderlist and determines
                  global fogginess &c
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cloudSetFog()
{
    Node* node;
    SpaceObj* spaceobj;
    real32 radius, sumOf = 0.0f, dist;

    if (++_counter & 1)
    {
        return;
    }

    node = universe.ResourceList.head;
    while (node != NULL)
    {
        spaceobj = (SpaceObj*)listGetStructOfNode(node);
        if (spaceobj->staticinfo == NULL)
        {
            //shots and such
            node = node->next;
            continue;
        }

        if (spaceobj->objtype == OBJ_DustType)
        {
            cloudUpdateSystem((cloudSystem*)((DustCloud*)spaceobj)->stub);
        }

        node = node->next;
    }

    node = universe.RenderList.head;
    while (node != NULL)
    {
        spaceobj = (SpaceObj*)listGetStructOfNode(node);
        if (spaceobj->staticinfo == NULL)
        {
            //shots and such
            node = node->next;
            continue;
        }

        if (spaceobj->objtype == OBJ_DustType)
        {
            dontNebulate = TRUE;    //there's clouds here, don't do nebula fog

            radius = spaceobj->staticinfo->staticheader.staticCollInfo.collspheresize;
            dist = fsqrt(spaceobj->cameraDistanceSquared);
            if (dist != 0.0f)
            {
                sumOf += radius * (1.0f / dist);
            }
        }

        node = node->next;
    }

    sumOf *= 0.1f;
    g_FogSum = sumOf;

    if (sumOf <= 1e-2f)
    {
        //this is also the case if there are no clouds in the renderlist at all
        rndFogOn = FALSE;
        _clearcolorUniverse();
    }
    else
    {
        int r, g, b;

        if (sumOf > DUSTCLOUD_DENSITY_THRESHOLD)
        {
            GLfloat sum = sumOf - DUSTCLOUD_DENSITY_THRESHOLD;
            rndFogOn = TRUE;
            if (sum > DUSTCLOUD_MAX_DENSITY)
            {
                sum = DUSTCLOUD_MAX_DENSITY;
            }
            glFogfv(GL_FOG_COLOR, fogColor);
            glFogf(GL_FOG_DENSITY, sum);
        }
        else
        {
            rndFogOn = FALSE;
        }

        if (sumOf > 1.0f)
        {
            sumOf = 1.0f;
        }

        if (!smSensorsActive)
        {
            r = (int)(DUSTCLOUD_FOG_RED * sumOf * DUSTCLOUD_BG_SCALAR * 255.0f);
            g = (int)(DUSTCLOUD_FOG_GREEN * sumOf * DUSTCLOUD_BG_SCALAR * 255.0f);
            b = (int)(DUSTCLOUD_FOG_BLUE * sumOf * DUSTCLOUD_BG_SCALAR * 255.0f);
            r += colRed(universe.backgroundColor);
            g += colGreen(universe.backgroundColor);
            b += colBlue(universe.backgroundColor);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            rndSetClearColor(colRGBA(r,g,b,255));
        }
    }
}
