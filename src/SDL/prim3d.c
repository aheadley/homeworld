/*=============================================================================
    Name    : prim3d.h
    Purpose : Draw 3d primitives.

    Created 7/1/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <math.h>
#include <stdlib.h>
#include "Debug.h"
#include "glinc.h"
#include "LinkedList.h"
#include "Memory.h"
#include "prim3d.h"
#include "render.h"
#include "texreg.h"
#include "Types.h"
#include "Vector.h"
#include "glcaps.h"
#include "devstats.h"

#define sizeofverticearray(x)   sizeof(vertice_array) + (sizeof(vector) * (x-1))

typedef struct
{
    Node   node;
    udword num_vertices;
    uword  axis;
    vector vertice[1];
} vertice_array;

//globals
LinkedList CircleList;

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : primLine3
    Description : Draw a line in 3D using having a thickness of 1 pixel
    Inputs      : p1, p2 - end points of line segment to draw
                  c - color to draw it in.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void primLine3(vector *p1, vector *p2, color c)
{
    bool blendon;

    if (glCapFeatureExists(GL_LINE_SMOOTH))
    {
        blendon = glIsEnabled(GL_BLEND);
        if (!blendon) glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
        rndAdditiveBlends(FALSE);
    }

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glBegin(GL_LINES);
    glVertex3fv((const GLfloat *)p1);
    glVertex3fv((const GLfloat *)p2);
    glEnd();

    if (glCapFeatureExists(GL_LINE_SMOOTH))
    {
        if (!blendon) glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
    }
}

/*-----------------------------------------------------------------------------
    Name        : primCircleSolid3
    Description : Draw a solid circle on the z = centre->z plane
    Inputs      : centre - centre point (in 3D coords) of the circle
                  radius - radius of circle (actually, distance
                    from centre to vertices)
                  nSlices - number of polygons to draw
                  c - color of circle
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primCircleSolid3(vector *centre, real32 radius, sdword nSlices, color c)
{
    sdword index;
    GLfloat v[3];
    double theta;

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    v[0] = centre->x;
    v[1] = centre->y;
    v[2] = centre->z;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(v);                                          //centre vertex
    for (index = 0, theta = 0.0; index < nSlices; index++)
    {
        v[0] = centre->x + (real32)(sin(theta)) * radius;
        v[1] = centre->y + (real32)(cos(theta)) * radius;
        theta += 2.0 * PI / (double)nSlices;
        glVertex3fv(v);                                      //vertex on outer rim
    }
    v[0] = centre->x;
    v[1] = centre->y + radius;
    glVertex3fv(v);                                          //final vertex on outer rim
    glEnd();
}

void primCircleSolid3Fade(vector *centre, real32 radius, sdword nSlices, color c, real32 fade)
{
    sdword index;
    GLfloat v[3];
    double theta;
    GLboolean blend = glIsEnabled(GL_BLEND);

    if (!blend)
    {
        glEnable(GL_BLEND);
        rndAdditiveBlends(FALSE);
    }

    glColor4ub(colRed(c), colGreen(c), colBlue(c), (ubyte)(fade * 255.0f));
    v[0] = centre->x;
    v[1] = centre->y;
    v[2] = centre->z;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(v);                                          //centre vertex
    for (index = 0, theta = 0.0; index < nSlices; index++)
    {
        v[0] = centre->x + (real32)(sin(theta)) * radius;
        v[1] = centre->y + (real32)(cos(theta)) * radius;
        theta += 2.0 * PI / (double)nSlices;
        glVertex3fv(v);                                      //vertex on outer rim
    }
    v[0] = centre->x;
    v[1] = centre->y + radius;
    glVertex3fv(v);                                          //final vertex on outer rim
    glEnd();

    if (!blend)
    {
        glDisable(GL_BLEND);
    }
}


/*-----------------------------------------------------------------------------
    Name        : primCreateNewCircleVerticeArray
    Description : Creates a set of vertices defining a unit circle around the axis given
    Inputs      : nSlices - the number of polygons in the unit circle,
                  axis - the axis around which the circle is drawn
    Outputs     : Allocates some memory and stuff
    Return      : the new vertice array
----------------------------------------------------------------------------*/
vertice_array *primCreateNewCircleVerticeArray(sdword nSlices, uword axis)
{
    udword i;
    double theta = 0.0;
    vertice_array *vertices = (vertice_array *)memAlloc(sizeofverticearray(nSlices+1), "circle_vertices", NonVolatile);

    vertices->num_vertices = nSlices + 1;
    vertices->axis         = axis;

    for (i = 0; i < vertices->num_vertices; i++)
    {
        switch (axis)
        {
            case X_AXIS:
                vertices->vertice[i].x = 0.0;
                vertices->vertice[i].y = (real32)(cos(theta));
                vertices->vertice[i].z = (real32)(sin(theta));
                break;
            case Y_AXIS:
                vertices->vertice[i].x = (real32)(sin(theta));
                vertices->vertice[i].y = 0.0;
                vertices->vertice[i].z = (real32)(cos(theta));
                break;
            case Z_AXIS:
                vertices->vertice[i].x = (real32)(sin(theta));
                vertices->vertice[i].y = (real32)(cos(theta));
                vertices->vertice[i].z = 0.0;
                break;
        }
        theta += 2.0 * PI / (double)nSlices;
    }

    listAddNode(&CircleList, &vertices->node, vertices);

    return vertices;
}


/*-----------------------------------------------------------------------------
    Name        : primCircleOutline3
    Description : Draw a solid circle on the z = centre->z plane
    Inputs      : centre - centre point (in 3D coords) of the circle
                  radius - radius of circle (actually, distance
                    from centre to vertices)
                  nSlices - number of polygons to draw
                  nSpokes - number of 'spokes' which actually get drawn.
                    nSlices should integer divide by this with no remainder.
                  color - color of circle
                  axis - axis on which to draw the circle
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primCircleOutline3(vector *centre, real32 radius, sdword nSlices,
                        sdword nSpokes, color color, uword axis)
{
    Node *node;
    vertice_array *vertices;
    register vector *vec_ptr;
    sdword index;
    GLfloat c[3], rim[3];

    // find the vertice list containing the unit circle that has the same
    // number of slices and is aligned along the same axis as the circle to be drawn
    node = CircleList.head;
    while (node != NULL)
    {
        vertices = (vertice_array *)listGetStructOfNode(node);
        if ((vertices->num_vertices == (nSlices + 1)) && (vertices->axis == axis))
        {
            //found the correct unit circle
            break;
        }

        node = node->next;
    }

    //if the unit circle isn't found, generate a new one
    if (node == NULL)
    {
        vertices = primCreateNewCircleVerticeArray(nSlices, axis);
    }

    if (nSpokes != 0)
    {
        nSpokes = nSlices / nSpokes;
    }

    glColor3ub(colRed(color), colGreen(color), colBlue(color));
    c[0] = centre->x;                                       //compute centre point
    c[1] = centre->y;
    c[2] = centre->z;

    glShadeModel(GL_SMOOTH);
    if (glCapFeatureExists(GL_LINE_SMOOTH))
    {
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
        rndAdditiveBlends(FALSE);
    }

    vec_ptr = &vertices->vertice[0];

    switch (axis)
    {
        case X_AXIS:
            rim[0] = centre->x + vec_ptr->x * radius;

            //draw the circle
            glBegin(GL_LINE_LOOP);
            for (index = 0; index <= nSlices; index++, vec_ptr++)
            {
                rim[1] = centre->y + vec_ptr->y * radius;
                rim[2] = centre->z + vec_ptr->z * radius;
                glVertex3fv(rim);                                   //vertex on rim
            }
            glEnd();

            //now draw the spokes
            if (nSpokes)
            {
                vec_ptr = &vertices->vertice[0];
                glBegin(GL_LINES);
                for (index = 0; index <= nSlices; index += nSpokes, vec_ptr += nSpokes)
                {
                    rim[1] = centre->y + vec_ptr->y * radius;
                    rim[2] = centre->z + vec_ptr->z * radius;
                    glVertex3fv(c);
                    glVertex3fv(rim);
                }
                glEnd();
            }
            break;

        case Y_AXIS:
            rim[1] = centre->y + vec_ptr->y * radius;

            //draw the circle
            glBegin(GL_LINE_LOOP);
            for (index = 0; index <= nSlices; index++, vec_ptr++)
            {
                rim[0] = centre->x + vec_ptr->x * radius;
                rim[2] = centre->z + vec_ptr->z * radius;
                glVertex3fv(rim);                                   //vertex on rim
            }
            glEnd();

            //draw the spokes
            if (nSpokes)
            {
                vec_ptr = &vertices->vertice[0];
                glBegin(GL_LINES);
                for (index = 0; index <= nSlices; index += nSpokes, vec_ptr += nSpokes)
                {
                    rim[0] = centre->x + vec_ptr->x * radius;
                    rim[2] = centre->z + vec_ptr->z * radius;
                    glVertex3fv(c);
                    glVertex3fv(rim);
                }
                glEnd();
            }
            break;

        case Z_AXIS:
            rim[2] = centre->z + vec_ptr->z * radius;

            //draw the circle
            glBegin(GL_LINE_LOOP);
            for (index = 0; index <= nSlices; index++, vec_ptr++)
            {
                rim[0] = centre->x + vec_ptr->x * radius;
                rim[1] = centre->y + vec_ptr->y * radius;
                glVertex3fv(rim);                                   //vertex on rim
            }
            glEnd();

            //draw the spokes
            if (nSpokes)
            {
                vec_ptr = &vertices->vertice[0];
                glBegin(GL_LINES);
                for (index = 0; index <= nSlices; index += nSpokes, vec_ptr += nSpokes)
                {
                    rim[0] = centre->x + vec_ptr->x * radius;
                    rim[1] = centre->y + vec_ptr->y * radius;
                    glVertex3fv(c);
                    glVertex3fv(rim);
                }
                glEnd();
            }
            break;

        default:
            break;
    }

    if (glCapFeatureExists(GL_LINE_SMOOTH))
    {
        glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
    }
}

/*-----------------------------------------------------------------------------
    Name        : primCircleOutlineZ
    Description : Similar to the previous function, except that it has no spokes
                    and is optimized for circles aligned to the Z-axis.
    Inputs      : centre - where to draw it.
                  radius - obvious
                  nSegments - number of line segments to make up the circle
                  c - color to draw in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primCircleOutlineZ(vector *centre, real32 radius, sdword nSegments, color c)
{
    //now that primCircleOutline3 has been optimized, it is faster than this one
    //this one can probably still be tweaked even further, so I'll leave it here
    primCircleOutline3(centre, radius, nSegments, 0, c, Z_AXIS);
/*
    GLfloat rim[3];
    double theta, thetaDelta;
    real32 x, y;

    theta = 0.0f;
    thetaDelta = 2.0 * PI / (double)nSegments;
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    rndGLStateLog("primCircleOutlineZ");
    x = centre->x;
    y = centre->y;
    rim[2] = centre->z;
    glBegin(GL_LINE_STRIP);
    for (; nSegments >= 0; nSegments--)
    {
        rim[0] = x + (real32)sin(theta) * radius;
        rim[1] = y + (real32)cos(theta) * radius;
        glVertex3fv(rim);
        theta += thetaDelta;
    }
    glEnd();
*/
}

/*-----------------------------------------------------------------------------
    Name        : primEllipseOutlineZ
    Description : Draw an axis-aligned ellipse centred about a specified point on the Z plane
    Inputs      : centre - centre of the ellipse
                  rx - x - axis radius
                  ry - y - axis radius
                  nSegments - number of segments
                  c - color of ellipse
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primEllipseOutlineZ(vector *centre, real32 rx, real32 ry, sdword nSegments, color c)
{
    GLfloat rim[3];
    double theta, thetaDelta;
    real32 x, y;

    theta = 0.0f;
    thetaDelta = 2.0 * PI / (double)nSegments;
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    x = centre->x;
    y = centre->y;
    rim[2] = centre->z;
    glBegin(GL_LINE_STRIP);
    for (; nSegments >= 0; nSegments--)
    {
        rim[0] = x + (real32)sin(theta) * rx;
        rim[1] = y + (real32)cos(theta) * ry;
        glVertex3fv(rim);
        theta += thetaDelta;
    }
    glEnd();
}

/*-----------------------------------------------------------------------------
    Name        : primPoint3
    Description : Draw a 3D point
    Inputs      : p1 - location of point
                  c - color of point
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void primPoint3(vector *p1, color c)
{
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glBegin(GL_POINTS);
    glVertex3f(p1->x, p1->y, p1->z);                        //!!! no size
    glEnd();
}

static bool gFastBlends;
static bool gWasBlending;

/*-----------------------------------------------------------------------------
    Name        : primBeginPointSize3Fade
    Description : begin drawing multiple 3D points
    Inputs      : size - size of point (ie. 1.0f, 2.0f, ...)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primBeginPointSize3Fade(real32 size)
{
    if (glCapFeatureExists(GL_POINT_SIZE))
    {
        glPointSize(size);
    }
    gFastBlends = glCapFastFeature(GL_BLEND);
    if (gFastBlends)
    {
        gWasBlending = glIsEnabled(GL_BLEND);
        if (!gWasBlending)
        {
            glEnable(GL_BLEND);
        }
        rndAdditiveBlends(FALSE);
    }
    glBegin(GL_POINTS);
}

/*-----------------------------------------------------------------------------
    Name        : primNextPointSize3Fade
    Description : render a 3D point (call from inside Begin/End block only)
    Inputs      : p1 - location of point
                  c - color of point
                  fade - fadeness of point (1.0f == no fade, 0.0f == faded to black)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primNextPointSize3Fade(vector* p1, color c, real32 fade)
{
    sdword ifade = (sdword)(fade * 255.0f);
    if (gFastBlends)
    {
        glColor4ub((GLubyte)colRed(c),
                   (GLubyte)colGreen(c),
                   (GLubyte)colBlue(c),
                   (GLubyte)ifade);
    }
    else
    {
        glColor3ub((GLubyte)(((sdword)colRed(c) * ifade) >> 8),
                   (GLubyte)(((sdword)colGreen(c) * ifade) >> 8),
                   (GLubyte)(((sdword)colBlue(c) * ifade) >> 8));
    }
}

/*-----------------------------------------------------------------------------
    Name        : primEndPointSize3Fade
    Description : end multiple point mode
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primEndPointSize3Fade(void)
{
    glEnd();
    if (gFastBlends && !gWasBlending)
    {
        glDisable(GL_BLEND);
    }
}

/*-----------------------------------------------------------------------------
    Name        : primPointSize3
    Description : Draw a 3D point with size
    Inputs      : p1 - location of point
                  size - physical size of point
                  c - color of point
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void primPointSize3(vector *p1, real32 size, color c)
{
    glPointSize(size);
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glBegin(GL_POINTS);
    glVertex3f(p1->x, p1->y, p1->z);                        //!!! no size
    glEnd();
    glPointSize(1.0f);
}

void primPointSize3Fade(vector *p1, real32 size, color c, real32 fade)
{
    GLboolean blend = glIsEnabled(GL_BLEND);

    if (!blend)
    {
        glEnable(GL_BLEND);
    }

    glPointSize(size);
    glColor4ub(colRed(c), colGreen(c), colBlue(c), (ubyte)(fade * 255.0f));
    glBegin(GL_POINTS);
    glVertex3f(p1->x, p1->y, p1->z);                        //!!! no size
    glEnd();
    glPointSize(1.0f);

    if (!blend)
    {
        glDisable(GL_BLEND);
    }
}

static void primSolidTexture3_multi(vector* p1, real32 size, color c, trhandle tex)
{
    real32 halfsize;
    texreg* reg;
    extern udword gDevcaps;

    halfsize = 0.5f * size;

    rndTextureEnable(TRUE);

    trMakeCurrent(tex);
    reg = trStructureGet(tex);
    if (bitTest(reg->flags, TRF_Alpha))
    {
        glEnable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);
        rndAdditiveBlends(TRUE);
    }

    glBegin(GL_QUADS);

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(p1->x-halfsize, p1->y-halfsize, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(p1->x+halfsize, p1->y-halfsize, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(p1->x+halfsize, p1->y+halfsize, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(p1->x-halfsize, p1->y+halfsize, 0.0f);

    if (!bitTest(gDevcaps, DEVSTAT_NO_GETTEXIMAGE))
    {
        glColor3ub(172, 172, 172);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(p1->x-halfsize, p1->y-halfsize, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(p1->x+halfsize, p1->y-halfsize, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(p1->x+halfsize, p1->y+halfsize, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(p1->x-halfsize, p1->y+halfsize, 0.0f);
    }

    glEnd();

    glDisable(GL_BLEND);
    rndAdditiveBlends(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : primSolidTexture3
    Description : Draw a 3D point with size
    Inputs      : p1 - location of point
                  size - physical size of point
                  c - color of point
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void primSolidTexture3(vector *p1, real32 size, color c, trhandle tex)
{
    real32 halfsize;
    real32 biasRed, biasGreen, biasBlue;
    texreg* reg;

    if (!glCapFeatureExists(RGL_COLOROP_ADD))
    {
        //multi-pass render to approximate a missing feature
        primSolidTexture3_multi(p1, size, c, tex);
        return;
    }

    halfsize = 0.5f * size;

    rndTextureEnable(TRUE);
//    glDepthMask(GL_FALSE);

    trMakeCurrent(tex);
    reg = trStructureGet(tex);
    if (bitTest(reg->flags, TRF_Alpha))
    {
        glEnable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);
        rndAdditiveBlends(TRUE);
    }

    biasRed = colReal32(colRed(c));
    biasGreen = colReal32(colGreen(c));
    biasBlue = colReal32(colBlue(c));

    if (RGL)
    {
        glPixelTransferf(GL_RED_BIAS, biasRed);
        glPixelTransferf(GL_GREEN_BIAS, biasGreen);
        glPixelTransferf(GL_BLUE_BIAS, biasBlue);
    }
    glColor3f(biasRed, biasGreen, biasBlue);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(p1->x-halfsize, p1->y-halfsize, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(p1->x+halfsize, p1->y-halfsize, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(p1->x+halfsize, p1->y+halfsize, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(p1->x-halfsize, p1->y+halfsize, 0.0f);
    glEnd();

    if (RGL)
    {
        glPixelTransferf(GL_RED_BIAS, 0.0f);
        glPixelTransferf(GL_GREEN_BIAS, 0.0f);
        glPixelTransferf(GL_BLUE_BIAS, 0.0f);
    }

    glDisable(GL_BLEND);
//    glDepthMask(GL_TRUE);
    rndAdditiveBlends(FALSE);
}

void primSolidTexture3Fade(vector *p1, real32 size, color c, trhandle tex, real32 fade)
{
   real32 halfsize = size / 2;
   real32 biasRed, biasGreen, biasBlue;
   texreg* reg;

   rndTextureEnable(TRUE);

   trMakeCurrent(tex);
   reg = trStructureGet(tex);
   if (bitTest(reg->flags, TRF_Alpha))
   {
      glEnable(GL_BLEND);
      glDisable(GL_ALPHA_TEST);
      rndAdditiveBlends(TRUE);
   }

   biasRed = colReal32(colRed(c));
   biasGreen = colReal32(colGreen(c));
   biasBlue = colReal32(colBlue(c));

   if (RGL)
   {
       glPixelTransferf(GL_RED_BIAS, biasRed);
       glPixelTransferf(GL_GREEN_BIAS, biasGreen);
       glPixelTransferf(GL_BLUE_BIAS, biasBlue);
   }
   glColor4f(biasRed, biasGreen, biasBlue, fade);

   glBegin(GL_QUADS);
   glTexCoord2f(0.0f, 0.0f);
   glVertex3f(p1->x-halfsize, p1->y-halfsize, 0.0f);
   glTexCoord2f(1.0f, 0.0f);
   glVertex3f(p1->x+halfsize, p1->y-halfsize, 0.0f);
   glTexCoord2f(1.0f, 1.0f);
   glVertex3f(p1->x+halfsize, p1->y+halfsize, 0.0f);
   glTexCoord2f(0.0f, 1.0f);
   glVertex3f(p1->x-halfsize, p1->y+halfsize, 0.0f);
   glEnd();

   if (RGL)
   {
       glPixelTransferf(GL_RED_BIAS, 0.0f);
       glPixelTransferf(GL_GREEN_BIAS, 0.0f);
       glPixelTransferf(GL_BLUE_BIAS, 0.0f);
   }

   glDisable(GL_BLEND);
   rndAdditiveBlends(FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : prim3dStartup
    Description : Initializes some 3d drawing structures
    Inputs      :
    Outputs     : Initializes the circle vertice list
    Return      : void
----------------------------------------------------------------------------*/
void prim3dStartup(void)
{
    listInit(&CircleList);
}


/*-----------------------------------------------------------------------------
    Name        : prim3dShutdown
    Description : Performs shutdown on 3d drawing structures
    Inputs      :
    Outputs     : Deallocates the circle vertice list
    Return      : void
----------------------------------------------------------------------------*/
void prim3dShutdown(void)
{
    listDeleteAll(&CircleList);
}


