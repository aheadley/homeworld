/*=============================================================================
    Name    : clipper.c
    Purpose : out-of-GL line clipping / projection code

    Created 4/10/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#ifdef _WIN32
#include <windows.h>
#endif
#endif
#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "glinc.h"
#include "Clipper.h"


#define CORRECT_BBOX_CLIP 0


#ifndef DEPTH_SCALE
#define DEPTH_SCALE 65535.0f
#define CLIP_RIGHT_BIT 1
#define CLIP_LEFT_BIT 2
#define CLIP_TOP_BIT 4
#define CLIP_BOTTOM_BIT 8
#define CLIP_NEAR_BIT 16
#define CLIP_FAR_BIT 32
#define CLIP_ALL_BITS 0x3f
#endif

/*-----------------------------------------------------------------------------
    Name        : clipViewclipLine
    Description :
    Inputs      : vectors - packed array of 4 tuples, expected to have extra space
                  i - address of 1st vertex index
                  j - address of 2nd vertex index
    Outputs     : i, j - vertex indices
    Return      : 0 = line completely outside of view
                  1 = line inside view
----------------------------------------------------------------------------*/
sdword clipViewclipLine(real32* vectors, udword* i, udword* j)
{
    real32 t, dx, dy, dz, dw;
    udword ii, jj, Free;
    real32* coord = vectors;

    ii = *i;
    jj = *j;
    Free = 2;

#define GENERAL_CLIP \
    if (OUTSIDE(ii)) \
    { \
        if (OUTSIDE(jj)) \
        { \
            return 0; \
        } \
        else \
        { \
            COMPUTE_INTERSECTION(Free, jj, ii) \
            ii = Free; \
            Free++; \
        } \
    } \
    else \
    { \
        if (OUTSIDE(jj)) \
        { \
            COMPUTE_INTERSECTION(Free, ii, jj) \
            jj = Free; \
            Free++; \
        } \
    }

#define X(I) coord[4*I + 0]
#define Y(I) coord[4*I + 1]
#define Z(I) coord[4*I + 2]
#define W(I) coord[4*I + 3]

    /* clip against +X side */
#define OUTSIDE(K) (X(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dx = X(out) - X(in); \
    dw = W(out) - W(in); \
    t = (X(in) - W(in)) / (dw - dx); \
    X(new) = X(in) + t * dx; \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = W(in) + t * dw; \

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against -X side */
#define OUTSIDE(K) (X(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dx = X(out) - X(in); \
    dw = W(out) - W(in); \
    t = -(X(in) + W(in)) / (dw + dx); \
    X(new) = X(in) + t * dx; \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against +Y side */
#define OUTSIDE(K) (Y(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dy = Y(out) - Y(in); \
    dw = W(out) - W(in); \
    t = (Y(in) - W(in)) / (dw - dy); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * dy; \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against -Y side */
#define OUTSIDE(K) (Y(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dy = Y(out) - Y(in); \
    dw = W(out) - W(in); \
    t = -(Y(in) + W(in)) / (dw + dy); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * dy; \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against +Z side */
#define OUTSIDE(K) (Z(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dz = Z(out) - Z(in); \
    dw = W(out) - W(in); \
    t = (Z(in) - W(in)) / (dw - dz); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * dz; \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against -Z side */
#define OUTSIDE(K) (Z(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dz = Z(out) - Z(in); \
    dw = W(out) - W(in); \
    t = -(Z(in) + W(in)) / (dw + dz); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * dz; \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

#undef GENERAL_CLIP

    *i = ii;
    *j = jj;
    return 1;
}

/*-----------------------------------------------------------------------------
    Name        : clipTransformPoints
    Description : modelview transform stage
    Inputs      : n - number of verts
                  vObj - objectspace (modelview space) coordinates
                  vEye - cameraspace coordinates
                  m - modelview matrix
    Outputs     : fills in vEye with transformed vObj
    Return      :
    Notes       : assumes a general matrix, ie. very inefficient
----------------------------------------------------------------------------*/
void clipTransformPoints(udword n, real32* vObj, real32* vEye, real32* m)
{
    udword i;
    real32 ox, oy, oz;

    for (i = 0; i < n; i++)
    {
        ox = vObj[4*i + 0];
        oy = vObj[4*i + 1];
        oz = vObj[4*i + 2];
        vEye[4*i + 0] = m[0] * ox + m[4] * oy + m[8]  * oz + m[12];
        vEye[4*i + 1] = m[1] * ox + m[5] * oy + m[9]  * oz + m[13];
        vEye[4*i + 2] = m[2] * ox + m[6] * oy + m[10] * oz + m[14];
        vEye[4*i + 3] = m[3] * ox + m[7] * oy + m[11] * oz + m[15];
    }
}

/*-----------------------------------------------------------------------------
    Name        : clipIsPerspective
    Description : determines whether a given matrix is a perspective matrix
    Inputs      : m - projection matrix
    Outputs     :
    Return      : TRUE  = yes, it is a perspective
                  FALSE = no, it's something else
----------------------------------------------------------------------------*/
bool clipIsPerspective(real32* m)
{
    if (m[4] == 0.0f && m[12] == 0.0f && m[1] == 0.0f && m[13] == 0.0f &&
        m[2] == 0.0f && m[6] == 0.0f && m[3] == 0.0f && m[7] == 0.0f &&
        m[11] == -1.0f && m[15] == 0.0f)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clipProjectPoints
    Description : projection transform stage
    Inputs      : n - number of verts
                  vEye  - cameraspace coordinates
                  vClip - canonical (clipspace) coordinates
                  clipmask - vertex clipping flags
                  clipormask, clipandmask - clip masks for non/trivial elimination
                  m - projection matrix
    Outputs     : fills in vClip with transformed vEye
    Return      :
    Notes       : assumes a general matrix, ie. very inefficient
----------------------------------------------------------------------------*/
void clipProjectPoints(
    udword n, real32* vEye, real32* vClip, ubyte* clipmask,
    ubyte* clipormask, ubyte* clipandmask, real32* m)
{
    ubyte tmpormask  = *clipormask;
    ubyte tmpandmask = *clipandmask;
    udword i;

    if (clipIsPerspective(m))
    {
        for (i = 0; i < n; i++)
        {
            real32 ex = vEye[4*i + 0];
            real32 ey = vEye[4*i + 1];
            real32 ez = vEye[4*i + 2];
            real32 ew = vEye[4*i + 3];
            real32 cx = m[0] * ex + m[8] * ez;
            real32 cy = m[5] * ey + m[9] * ez;
            real32 cz = m[10] * ez + m[14] * ew;
            real32 cw = -ez;
            GLubyte mask = 0;
            vClip[4*i + 0] = cx;
            vClip[4*i + 1] = cy;
            vClip[4*i + 2] = cz;
            vClip[4*i + 3] = cw;
            if (cx > cw)        mask |= CLIP_RIGHT_BIT;
            else if (cx < -cw)  mask |= CLIP_LEFT_BIT;
            if (cy > cw)        mask |= CLIP_TOP_BIT;
            else if (cy < -cw)  mask |= CLIP_BOTTOM_BIT;
            if (cz > cw)        mask |= CLIP_FAR_BIT;
            else if (cz < -cw)  mask |= CLIP_NEAR_BIT;
            if (mask)
            {
                clipmask[i] |= mask;
                tmpormask |= mask;
            }
            tmpandmask &= mask;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            real32 ex = vEye[4*i + 0];
            real32 ey = vEye[4*i + 1];
            real32 ez = vEye[4*i + 2];
            real32 ew = vEye[4*i + 3];
            real32 cx = m[0] * ex + m[4] * ey + m[8]  * ez + m[12] * ew;
            real32 cy = m[1] * ex + m[5] * ey + m[9]  * ez + m[13] * ew;
            real32 cz = m[2] * ex + m[6] * ey + m[10] * ez + m[14] * ew;
            real32 cw = m[3] * ex + m[7] * ey + m[11] * ez + m[15] * ew;
            ubyte mask = 0;
            vClip[4*i + 0] = cx;
            vClip[4*i + 1] = cy;
            vClip[4*i + 2] = cz;
            vClip[4*i + 3] = cw;
            if (cx > cw)        mask |= CLIP_RIGHT_BIT;
            else if (cx < -cw)  mask |= CLIP_LEFT_BIT;
            if (cy > cw)        mask |= CLIP_TOP_BIT;
            else if (cy < -cw)  mask |= CLIP_BOTTOM_BIT;
            if (cz > cw)        mask |= CLIP_FAR_BIT;
            else if (cz < -cw)  mask |= CLIP_NEAR_BIT;
            if (mask)
            {
                clipmask[i] |= mask;
                tmpormask |= mask;
            }
            tmpandmask &= mask;
        }
    }

    *clipormask = tmpormask;
    *clipandmask = tmpandmask;
}

/*-----------------------------------------------------------------------------
    Name        : clipViewportMap
    Description : clip -> screen space projection, with perspective divide
    Inputs      : n - number of verts
                  vClip - clipspace coordinates
                  clipmask - per-vertex clip masks
                  vWin - screen space coordinates
                  viewportS - viewport scale vector
                  viewportT - viewport translation vector
                  force - ignore clip tests
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clipViewportMap(
    udword n, real32* vClip, ubyte* clipmask, real32* vWin,
    vector* viewportS, vector* viewportT, bool force)
{
    real32 sx, tx, sy, ty, sz, tz;
    udword i;

    sx = viewportS->x;
    sy = viewportS->y;
    sz = viewportS->z;
    tx = viewportT->x;
    ty = viewportT->y;
    tz = viewportT->z;

    if (clipmask && !force)
    {
        for (i = 0; i < n; i++)
        {
            if (clipmask[i] == 0)
            {
                if (vClip[4*i + 3] != 0.0f)
                {
                    real32 wInv = 1.0f / vClip[4*i + 3];
                    vWin[3*i + 0] = vClip[4*i + 0] * wInv * sx + tx;
                    vWin[3*i + 1] = vClip[4*i + 1] * wInv * sy + ty;
                    vWin[3*i + 2] = vClip[4*i + 2] * wInv * sz + tz;
                }
                else
                {
                    vWin[3*i + 0] = 0.0f;
                    vWin[3*i + 1] = 0.0f;
                    vWin[3*i + 2] = 0.0f;
                }
            }
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (vClip[4*i + 3] != 0.0f)
            {
                real32 wInv = 1.0f / vClip[4*i + 3];
                vWin[3*i + 0] = vClip[4*i + 0] * wInv * sx + tx;
                vWin[3*i + 1] = vClip[4*i + 1] * wInv * sy + ty;
                vWin[3*i + 2] = vClip[4*i + 2] * wInv * sz + tz;
            }
            else
            {
                vWin[3*i + 0] = 0.0f;
                vWin[3*i + 1] = 0.0f;
                vWin[3*i + 2] = 0.0f;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clipPointToScreen
    Description : tries to clip & project a point from modelview -> screen space
    Inputs      : p - the point
                  screen - holder for output screen coord
                  force - if TRUE, project regardless of clip test
    Outputs     : screen will contain screen projection of given point
    Return      : 0 = (failure) point is off screen
                  1 = (success) point is on screen
----------------------------------------------------------------------------*/
bool clipPointToScreen(vector* p, vector* screen, bool force)
{
    real32 modelview[16], projection[16];

    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    return clipPointToScreenWithMatrices(p, screen, modelview, projection, force);
}

/*-----------------------------------------------------------------------------
    Name        : clipPointToScreenWithMatrices
    Description : tries to clip & project a point from modelview -> screen space
    Inputs      : p - the point
                  screen - holder for output screen coord
                  modelview, projection - matrices
                  force - if TRUE, project regardless of clip test
    Outputs     : screen will contain screen projection of given point
    Return      : 0 = (failure) point is off screen
                  1 = (success) point is on screen
----------------------------------------------------------------------------*/
bool clipPointToScreenWithMatrices(
    vector* p, vector* screen, real32* modelview, real32* projection, bool force)
{
    hvector vbObj[1];
    hvector vbEye[1];
    hvector vbClip[1];
    vector  vbWin[1];
    vector  viewportS, viewportT;
    ubyte   clipmask[1];
    ubyte   clipormask = 0;
    ubyte   clipandmask = CLIP_ALL_BITS;
    GLint   param[4];

    screen->x = 0.0f;
    screen->y = 0.0f;
    screen->z = 0.0f;

    glGetIntegerv(GL_VIEWPORT, param);

    viewportS.x = (real32)param[2] / 2.0f;
    viewportS.y = (real32)param[3] / 2.0f;
    viewportS.z = DEPTH_SCALE * ((1.0f - 0.0f) / 2.0f);
    viewportT.x = viewportS.x + (real32)param[0];
    viewportT.y = viewportS.y + (real32)param[1];
    viewportT.z = DEPTH_SCALE * ((1.0f - 0.0f) / 2.0f + 0.0f);

    vbObj[0].x = p->x;
    vbObj[0].y = p->y;
    vbObj[0].z = p->z;
    vbObj[0].w = 1.0f;

    clipmask[0] = 0;

    clipTransformPoints(1, (real32*)vbObj, (real32*)vbEye, modelview);
    clipProjectPoints(1, (real32*)vbEye, (real32*)vbClip, clipmask,
                      &clipormask, &clipandmask, projection);

    if (clipandmask && !force)
    {
        return 0;
    }

    clipViewportMap(1, (real32*)vbClip,
                    clipormask ? clipmask : NULL,
                    (real32*)vbWin, &viewportS, &viewportT, force);

    if (clipormask && !force)
    {
        return 0;
    }
    else
    {
        screen->x = vbWin[0].x;
        screen->y = vbWin[0].y;
        screen->z = vbWin[0].z;
        return 1;
    }
}

#if CORRECT_BBOX_CLIP
//helper, determines whether a line is inside the frustum or not
static bool _lineinside(
    hvector rectpos[], udword v1, udword v2, ubyte clipmask[], hvector vbClip[])
{
    if (clipmask[v1] | clipmask[v2])
    {
        hvector vectors[8];
        vectors[0].x = vbClip[v1].x;
        vectors[0].y = vbClip[v1].y;
        vectors[0].z = vbClip[v1].z;
        vectors[0].w = vbClip[v1].w;
        vectors[1].x = vbClip[v2].x;
        vectors[1].y = vbClip[v2].y;
        vectors[1].z = vbClip[v2].z;
        vectors[1].w = vbClip[v2].w;
        return clipViewclipLine((real32*)vectors, &v1, &v2);
    }
    else
    {
        return TRUE;
    }
}
#endif //CORRECT_BBOX_CLIP

sdword clipTotallyIn = FALSE;

/*-----------------------------------------------------------------------------
    Name        : clipBBoxIsClipped
    Description : determines whether a given bbox is visible given the current frustum
    Inputs      : collrectoffset - gshaw's bbox array
                  uplength, rightlength, forwardlength - dimensions
    Outputs     :
    Return      : TRUE if totally outside of frustum, FALSE otherwise
----------------------------------------------------------------------------*/
bool clipBBoxIsClipped(
    real32* collrectoffset, real32 uplength, real32 rightlength, real32 forwardlength)
{
    vector upvector = {1.0f, 0.0f, 0.0f};
    vector rightvector = {0.0f, 1.0f, 0.0f};
    vector forwardvector = {0.0f, 0.0f, 1.0f};

    hvector rectpos[8]; //vbObj
    hvector vbEye[8];
    hvector vbClip[8];
    ubyte clipmask[8];
    ubyte clipormask = 0;
    ubyte clipandmask = CLIP_ALL_BITS;

    real32 modelview[16], projection[16];

    sdword i;
    bool result;

    clipTotallyIn = FALSE;

    rectpos[0].x = collrectoffset[0];
    rectpos[0].y = collrectoffset[1];
    rectpos[0].z = collrectoffset[2];

    vecMultiplyByScalar(upvector, uplength);
    vecMultiplyByScalar(rightvector, rightlength);
    vecMultiplyByScalar(forwardvector, forwardlength);

    for (i = 0; i < 8; i++)
    {
        rectpos[i].w = 1.0f;
        clipmask[i] = 0;
    }

    vecAdd(rectpos[1], rectpos[0], rightvector);
    vecAdd(rectpos[2], rectpos[1], forwardvector);
    vecAdd(rectpos[3], rectpos[0], forwardvector);

    vecAdd(rectpos[4], rectpos[0], upvector);
    vecAdd(rectpos[5], rectpos[1], upvector);
    vecAdd(rectpos[6], rectpos[2], upvector);
    vecAdd(rectpos[7], rectpos[3], upvector);

    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    clipTransformPoints(8, (real32*)rectpos, (real32*)vbEye, modelview);
    clipProjectPoints(8, (real32*)vbEye, (real32*)vbClip, clipmask,
                      &clipormask, &clipandmask, projection);

#if CORRECT_BBOX_CLIP
    result = TRUE;
    if (clipormask)
    {
#define LINETEST(v1,v2) if (_lineinside(rectpos, v1, v2, clipmask, vbClip)) { result = FALSE; goto PASSED; }
        LINETEST(0,1);
        LINETEST(1,2);
        LINETEST(2,3);
        LINETEST(3,0);
        LINETEST(4,5);
        LINETEST(5,6);
        LINETEST(6,7);
        LINETEST(7,4);
        LINETEST(0,4);
        LINETEST(1,5);
        LINETEST(2,6);
        LINETEST(3,7);
#undef LINETEST
    }
    else
    {
        result = FALSE;
    }

PASSED:
#else  //inCORRECT_BBOX_CLIP
    result = clipandmask ? TRUE : FALSE;
    if (clipormask == 0)
    {
        clipTotallyIn = TRUE;
    }
#endif //CORRECT_BBOX_CLIP
    return result;
}

/*-----------------------------------------------------------------------------
    Name        : clipLineToScreen
    Description : tries to clip & project vertices from modelview -> screen space
    Inputs      : pa, pb - endpoints of the line (3D)
                  modelview  - current modelview matrix
                  projection - current projection matrix
                  screenA, screenB - final window coordinates (2D)
    Outputs     :
    Return      : 0 = (failure) line is off screen
                  1 = (success) line is on screen
----------------------------------------------------------------------------*/
bool clipLineToScreen(
    vector* pa, vector* pb, real32* modelview, real32* projection,
    vector* screenA, vector* screenB)
{
    hvector vbObj[8];
    hvector vbEye[8];
    hvector vbClip[8];
    vector  vbWin[8];
    vector  viewportS, viewportT;
    ubyte clipmask[8];
    ubyte clipormask = 0;
    ubyte clipandmask = CLIP_ALL_BITS;
    udword i;

    GLint param[4];

    glGetIntegerv(GL_VIEWPORT, param);
    viewportS.x = (real32)param[2] / 2.0f;
    viewportS.y = (real32)param[3] / 2.0f;
    viewportS.z = DEPTH_SCALE * ((1.0f - 0.0f) / 2.0f);
    viewportT.x = viewportS.x + (real32)param[0];
    viewportT.y = viewportS.y + (real32)param[1];
    viewportT.z = DEPTH_SCALE * ((1.0f - 0.0f) / 2.0f + 0.0f);

    vbObj[0].x = pa->x;
    vbObj[0].y = pa->y;
    vbObj[0].z = pa->z;
    vbObj[0].w = 1.0f;
    vbObj[1].x = pb->x;
    vbObj[1].y = pb->y;
    vbObj[1].z = pb->z;
    vbObj[1].w = 1.0f;

    for (i = 0; i < 8; i++)
    {
        clipmask[i] = 0;
    }

    clipTransformPoints(2, (real32*)vbObj, (real32*)vbEye, modelview);
    clipProjectPoints(2, (real32*)vbEye, (real32*)vbClip, clipmask,
                      &clipormask, &clipandmask, projection);

    if (clipandmask)
    {
        return 0;
    }

    clipViewportMap(2, (real32*)vbClip,
                    clipormask ? clipmask : NULL,
                    (real32*)vbWin, &viewportS, &viewportT, 0);

    if (clipormask)
    {
        if (clipmask[0] | clipmask[1])
        {
            udword v0, v1;
            real32 ndcX, ndcY, ndcZ;

            v0 = 0;
            v1 = 1;
            if (clipViewclipLine((real32*)vbClip, &v0, &v1) == 0)
            {
                return 0;
            }

            if (vbClip[v0].w != 0.0f)
            {
                real32 wInv = 1.0f / vbClip[v0].w;
                ndcX = vbClip[v0].x * wInv;
                ndcY = vbClip[v0].y * wInv;
                ndcZ = vbClip[v0].z * wInv;
            }
            else
            {
                ndcX = ndcY = ndcZ = 0.0f;
            }

            screenA->x = vbWin[v0].x = ndcX * viewportS.x + viewportT.x;
            screenA->y = vbWin[v0].y = ndcY * viewportS.y + viewportT.y;
            screenA->z = vbWin[v0].z = ndcZ * viewportS.z + viewportT.z;

            if (vbClip[v1].w != 0.0f)
            {
                real32 wInv = 1.0f / vbClip[v1].w;
                ndcX = vbClip[v1].x * wInv;
                ndcY = vbClip[v1].y * wInv;
                ndcZ = vbClip[v1].z * wInv;
            }
            else
            {
                ndcX = ndcY = ndcZ = 0.0f;
            }

            screenB->x = vbWin[v1].x = ndcX * viewportS.x + viewportT.x;
            screenB->y = vbWin[v1].y = ndcY * viewportS.y + viewportT.y;
            screenB->z = vbWin[v1].z = ndcZ * viewportS.z + viewportT.z;
        }
        else
        {
            screenA->x = vbWin[0].x;
            screenA->y = vbWin[0].y;
            screenA->z = vbWin[0].z;
            screenB->x = vbWin[1].x;
            screenB->y = vbWin[1].y;
            screenB->z = vbWin[1].z;
        }
    }
    else
    {
        screenA->x = vbWin[0].x;
        screenA->y = vbWin[0].y;
        screenA->z = vbWin[0].z;
        screenB->x = vbWin[1].x;
        screenB->y = vbWin[1].y;
        screenB->z = vbWin[1].z;
    }

    return 1;
}
