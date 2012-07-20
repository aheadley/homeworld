/*=============================================================================
    Name    : prim2d.c
    Purpose : Functions to draw 2D primitives.

    Created 6/26/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "glinc.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "main.h"
#include "render.h"
#include "prim2d.h"
#include "glcaps.h"
#include "glcompat.h"

/*=============================================================================
    Data:
=============================================================================*/
sdword primModeEnabled = FALSE;

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : primModeSetFunction2
    Description : Enables the primitive drawing mode.
    Inputs      : void
    Outputs     : sets primModeEnabled TRUE
    Return      : void
----------------------------------------------------------------------------*/
void primModeSetFunction2(void)
{
    glShadeModel(GL_FLAT);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);

    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();                                         //perform no transformations on the 2D primitives
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    rndLightingEnable(FALSE);                               //mouse is self-illuminated
    rndTextureEnable(FALSE);
    glDisable(GL_DEPTH_TEST);

    primModeEnabled = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : primModeClearFunction2
    Description : Disables the primitive drawing mode.
    Inputs      : void
    Outputs     : sets primModeEnabled FALSE
    Return      : void
----------------------------------------------------------------------------*/
void primModeClearFunction2(void)
{
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    rndLightingEnable(TRUE);                                //and lighting
    rndTextureEnable(TRUE);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    primModeEnabled = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : primTriSolid2
    Description : Draw a solid 2D triangle
    Inputs      : tri - pointer to triangle structure containing coordinates of corners
                  c - color to draw it in
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void primTriSolid2(triangle *tri, color c)
{
    if (glcActive())
    {
        glcTriSolid2(tri, c);
    }
    else
    {
        glColor3ub(colRed(c), colGreen(c), colBlue(c));
        glBegin(GL_TRIANGLES);
        glVertex2f(primScreenToGLX(tri->x0), primScreenToGLY(tri->y0));
        glVertex2f(primScreenToGLX(tri->x1), primScreenToGLY(tri->y1));
        glVertex2f(primScreenToGLX(tri->x2), primScreenToGLY(tri->y2));
        glEnd();
    }
}

/*-----------------------------------------------------------------------------
    Name        : primTriOutline2
    Description : Draw a 2D triangle outline
    Inputs      : tri - pointer to triangle structure containing coordinates of corners
                  thickness - thickness of the lines
                  c - color to draw it in
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void primTriOutline2(triangle *tri, sdword thickness, color c)
{
    if (glcActive())
    {
        glcTriOutline2(tri, thickness, c);
    }
    else
    {
        glColor3ub(colRed(c), colGreen(c), colBlue(c));
        glPushAttrib(GL_LINE_BIT);
        glLineWidth((GLfloat)thickness);
        glBegin(GL_LINE_LOOP);
        glVertex2f(primScreenToGLX(tri->x0), primScreenToGLY(tri->y0));
        glVertex2f(primScreenToGLX(tri->x1), primScreenToGLY(tri->y1));
        glVertex2f(primScreenToGLX(tri->x2), primScreenToGLY(tri->y2));
        glEnd();
        glPopAttrib();
    }
}

/*-----------------------------------------------------------------------------
    Name        : primRectSolidTextured2
    Description : Draw a solid 2d rectangle with the current texture mapped thereupon.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
#define COORD(S,T,X,Y) \
    glTexCoord2f(S, T); \
    glVertex2f(primScreenToGLX(X), primScreenToGLY(Y));
void primRectSolidTextured2(rectangle *rect)
{
    glColor3ub(255, 255, 255);

    rndTextureEnvironment(RTE_Replace);
    rndTextureEnable(TRUE);

    glBegin(GL_QUADS);
    COORD(0.0f, 0.0f, rect->x0, rect->y0);
    COORD(0.0f, 1.0f, rect->x0, rect->y1 - 1);
    COORD(1.0f, 1.0f, rect->x1, rect->y1 - 1);
    COORD(1.0f, 0.0f, rect->x1, rect->y0);
    glEnd();

    rndTextureEnable(FALSE);
    rndTextureEnvironment(RTE_Modulate);
}
void primRectSolidTexturedFullRect2(rectangle *rect)
{
    glColor3ub(255, 255, 255);

    rndTextureEnvironment(RTE_Replace);
    rndTextureEnable(TRUE);

    glBegin(GL_QUADS);
    COORD(0.0f, 0.0f, rect->x0, rect->y0);
    COORD(0.0f, 1.0f, rect->x0, rect->y1);
    COORD(1.0f, 1.0f, rect->x1, rect->y1);
    COORD(1.0f, 0.0f, rect->x1, rect->y0);
    glEnd();

    rndTextureEnable(FALSE);
    rndTextureEnvironment(RTE_Modulate);
}
void primRectSolidTexturedFullRectC2(rectangle *rect, color c)
{
    glColor4ub(colRed(c), colGreen(c), colBlue(c), colAlpha(c));

    rndTextureEnable(TRUE);

    glBegin(GL_QUADS);
    COORD(0.0f, 0.0f, rect->x0, rect->y0);
    COORD(0.0f, 1.0f, rect->x0, rect->y1);
    COORD(1.0f, 1.0f, rect->x1, rect->y1);
    COORD(1.0f, 0.0f, rect->x1, rect->y0);
    glEnd();

    rndTextureEnable(FALSE);
}
#undef COORD

/*-----------------------------------------------------------------------------
    Name        : primRectSolid2
    Description : Draw a solid 2d rectangle.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
                  c - color to draw it in.
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primRectSolid2(rectangle *rect, color c)
{
    if (glcActive())
    {
        glcRectSolid2(rect, c);
    }
    else
    {
        glColor4ub(colRed(c), colGreen(c), colBlue(c), colAlpha(c));
        glBegin(GL_QUADS);
        glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y0));
        glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y1));
        glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y1));
        glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y0));
        glEnd();
    }
}

/*-----------------------------------------------------------------------------
    Name        : primRectTranslucentRGL2
    Description : helper for primRectTranslucent2, rGL(sw) only (will temporarily
                  disable stippling)
    Inputs      : rect - rectangle structure containing coordinates
                  c - color of the rectangle
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primRectTranslucentRGL2(rectangle* rect, color c)
{
    GLboolean blendOn;
    GLboolean stippleOn;

    blendOn = glIsEnabled(GL_BLEND);
    stippleOn = glIsEnabled(GL_POLYGON_STIPPLE);
    if (!blendOn) glEnable(GL_BLEND);
    if (stippleOn) glDisable(GL_POLYGON_STIPPLE);
    glColor4ub(colRed(c), colGreen(c), colBlue(c), colAlpha(c));
    glBegin(GL_QUADS);
    glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y0));
    glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y1));
    glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y1));
    glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y0));
    glEnd();
    if (!blendOn) glDisable(GL_BLEND);
    if (stippleOn) glEnable(GL_POLYGON_STIPPLE);
}

/*-----------------------------------------------------------------------------
    Name        : primRectTranslucent2
    Description : Draw a translucent 2d rectangle.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
                  c - color to draw it in.
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primRectTranslucent2(rectangle* rect, color c)
{
    GLboolean blendOn;

    if (RGL && RGLtype == SWtype)
    {
        primRectTranslucentRGL2(rect, c);
        return;
    }

    blendOn = glIsEnabled(GL_BLEND);
    if (!blendOn) glEnable(GL_BLEND);
    if (glcActive())
    {
        glcRectTranslucent2(rect, c);
    }
    else
    {
        glColor4ub(colRed(c), colGreen(c), colBlue(c), colAlpha(c));
        glBegin(GL_QUADS);
        glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y0));
        glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y1));
        glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y1));
        glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y0));
        glEnd();
    }
    if (!blendOn) glDisable(GL_BLEND);
}

/*-----------------------------------------------------------------------------
    Name        : primRectOutline2
    Description : Draw an outline 2d rectangle.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
                  thickness - thickness of the lines
                  c - color to draw it in.
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primRectOutline2(rectangle *rect, sdword thickness, color c)
{
    sdword bottom;

    if (glcActive())
    {
        glcRectOutline2(rect, thickness, c);
        return;
    }

    bottom = rect->y1 - 1;

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glPushAttrib(GL_LINE_BIT);
    glLineWidth((GLfloat)thickness);

    glBegin(GL_LINE_LOOP);
    glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y0));
    glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y0));
    glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(bottom));
    glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(bottom));
    glEnd();

    glPopAttrib();
}

/*-----------------------------------------------------------------------------
    Name        : primRectShaded2
    Description : Draw a shaded 2d rectangle.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
                  c - pointer to 4 color values to draw it in.
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primRectShaded2(rectangle *rect, color *c)
{
    if (glcActive())
    {
        glcRectShaded2(rect, c);
        return;
    }

    glShadeModel(GL_SMOOTH);

    glBegin(GL_QUADS);

    glColor3ub(colRed(c[0]), colGreen(c[0]), colBlue(c[0]));
    glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y0));

    glColor3ub(colRed(c[1]), colGreen(c[1]), colBlue(c[1]));
    glVertex2f(primScreenToGLX(rect->x0), primScreenToGLY(rect->y1));

    glColor3ub(colRed(c[2]), colGreen(c[2]), colBlue(c[2]));
    glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y1));

    glColor3ub(colRed(c[3]), colGreen(c[3]), colBlue(c[3]));
    glVertex2f(primScreenToGLX(rect->x1), primScreenToGLY(rect->y0));

    glEnd();
}

/*-----------------------------------------------------------------------------
    Name        : primRectUnion2
    Description : Compute the union of 2 2d rectangles
    Inputs      : result - destination of union rect
                  r0, r1 - rectangles to compute union of
    Outputs     : fills reslult with union rect
    Return      : void
    Notes       : The x1/y1 parameters of the rectangles should be larger
                    than the x0/y0 members.
                  If no union exists (rectangles do not overlap), the x0
                    and/or y0 members of the reslult will be the same as
                    the x1 and/or y1 members.
----------------------------------------------------------------------------*/
void primRectUnion2(rectangle *result, rectangle *r0, rectangle *r1)
{
    result->x0 = max(r0->x0, r1->x0);                       //get min/max bounds
    result->y0 = max(r0->y0, r1->y0);
    result->x1 = min(r0->x1, r1->x1);
    result->y1 = min(r0->y1, r1->y1);

    result->x0 = min(result->x0, result->x1);               //make sure not negative width/height
    result->y0 = min(result->y0, result->y1);
}

/*-----------------------------------------------------------------------------
    Name        : primRealRectUnion2
    Description : Same as above, just with real numbers instead
    Inputs      :
    Outputs     :
    Return      :
    Notes       :
----------------------------------------------------------------------------*/
void primRealRectUnion2(realrectangle *result, realrectangle *r0, realrectangle *r1)
{
    result->x0 = max(r0->x0, r1->x0);                       //get min/max bounds
    result->y0 = max(r0->y0, r1->y0);
    result->x1 = min(r0->x1, r1->x1);
    result->y1 = min(r0->y1, r1->y1);

    result->x0 = min(result->x0, result->x1);               //make sure not negative width/height
    result->y0 = min(result->y0, result->y1);
}

// FIXME
void CirclePoints(int x, int y, color c, int cx, int cy)
{
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    dbgMessagef("\nCirclePoints %d %d %d %d", x, y, cx, cy);
    glVertex2i(x + cx, y + cy);
    glVertex2i(y + cx, x + cy);
    glVertex2i(y + cx, -x + cy);
    glVertex2i(x + cx, -y + cy);
    glVertex2i(-x + cx, -y + cy);
    glVertex2i(-y + cx, x + cy);
    glVertex2i(-x + cx, y + cy);
    glEnd();
    glPointSize(1.0f);
}

// FIXME
void primCircleSegment(uword cx, uword cy, uword r, color c)
{
    int x, y;
    real32 d;

    x = 0;
    y = r;
    d = (real32)5/(real32)4 - (real32)r;
    CirclePoints(x, y, c, cx, cy);
    while (y > x)
    {
        if (d < 0)
        {
            d = d + 2*x + 3;
            x++;
        }
        else
        {
            d = d + 2*(x-y) + 5;
            x++;
            y--;
        }
        CirclePoints(x, y, c, cx, cy);
    }
}

/*-----------------------------------------------------------------------------
    Name        : primOvalArcOutline2
    Description : Draw an outline section of an oval.
    Inputs      : o - oval structure describing location and size on-screen
                  degStarts, degEnd - degree stations of start and end of line
                  thickness - thickness of lines
                  segments - number of line segments for a complete oval
                  c - color to draw outline in
    Outputs     : ..
    Return      : void
    Note        : The coordinate system used will be the engineering system
                    where up (x = 0, y = -1) is 0 rad.
----------------------------------------------------------------------------*/
void primOvalArcOutline2(oval *o, real32 radStart, real32 radEnd, sdword thickness, sdword segments, color c)
{
    sdword segment, endSegment;
    real32 angle, angleInc;
    real32 centreX, centreY, width, height;
    real32 x, y, lastX, lastY;

    if (glcActive())
    {
        centreX = (real32)o->centreX;
        centreY = (real32)o->centreY;
        width  = (real32)o->radiusX;
        height = (real32)o->radiusY;
    }
    else
    {
        centreX = primScreenToGLX(o->centreX);                  //get floating-point version of oval attributes
        centreY = primScreenToGLY(o->centreY);
        width  = primScreenToGLScaleX(o->radiusX);
        height = primScreenToGLScaleY(o->radiusY);
    }

    segment = (sdword)(radStart * (real32)segments / (2.0f * PI));//get starting segment
    endSegment = (sdword)(radEnd * (real32)segments / (2.0f * PI) - 0.01f);//get ending segment

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glPushAttrib(GL_LINE_BIT);
    glLineWidth((GLfloat)thickness);
    glBegin(GL_LINE_STRIP);

    x = centreX + (real32)sin((double)radStart) * width;    //first vertex
    y = centreY + (real32)cos((double)radStart) * height;
    if (glcActive())
    {
        lastX = x;
        lastY = y;
    }
    else
    {
        glVertex2f(x, y);
    }
    segment++;
    angle = (real32)segment * (2.0f * PI / (real32)segments);
    angleInc = (2.0f * PI / (real32)segments);

    for (; segment <= endSegment; segment++)
    {                                                       //for start and all complete segments
        x = centreX + (real32)sin((double)angle) * width;
        y = centreY + (real32)cos((double)angle) * height;
        if (glcActive())
        {
            glcLine2((sdword)lastX, (sdword)lastY,
                     (sdword)x, (sdword)y,
                     thickness, c);
            lastX = x;
            lastY = y;
        }
        else
        {
            glVertex2f(x, y);
        }
        angle += angleInc;                                  //update angle
    }
    x = centreX + (real32)sin((double)radEnd) * width;
    y = centreY + (real32)cos((double)radEnd) * height;
    if (glcActive())
    {
        glcLine2((sdword)lastX, (sdword)lastY,
                 (sdword)x, (sdword)y,
                 thickness, c);
    }
    else
    {
        glVertex2f(x, y);                                       //draw last vertex
    }

    glEnd();
    glPopAttrib();
}

/*-----------------------------------------------------------------------------
    Name        : primCircleOutline2
    Description : Draw a circle
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primGLCircleOutline2(real32 x, real32 y, real32 radius, sdword nSegments, color c)
{
    sdword index;
    double angle, angleInc = 2.0 * PI / (double)nSegments;
    real32 radiusY = radius * MAIN_WindowWidth / MAIN_WindowHeight;

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, y + radiusY);
    for (index = 0, angle = angleInc; index <= nSegments; index++, angle += angleInc)
    {
        glVertex2f(x + (real32)sin(angle) * radius, y + (real32)cos(angle) * radiusY);
    }
    glEnd();
}

/*-----------------------------------------------------------------------------
    Name        : primErrorMessagePrint
    Description : Print any errors generated by rendering system
    Inputs      : line, file - location function called from
    Outputs     : checks error messages and prints out any errors found
    Return      :
----------------------------------------------------------------------------*/
void primErrorMessagePrintFunction(char *file, sdword line)
{
    GLenum errorEnum;
    //there can be multiple errors simultaneously.  detect them all
    //in one call to this fn for debugging simplicity
    for (;;)
    {
        errorEnum = glGetError();
        if (errorEnum == GL_NO_ERROR)
        {
            return;
        }
        else
        {
//#ifndef khentschel
            if (RGLtype != GLtype)
//#endif
            {
                dbgWarningf(file, line, "glGetError returned '%s'", rgluErrorString(errorEnum));
			}
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : primLine2
    Description : Draw a line 1 pixel wide
    Inputs      : c - attributes of line to be drawn
                  x0, y0, x1, y1 - start/end of line segment
    Outputs     : Sets GL_COLOR
    Return      : void
----------------------------------------------------------------------------*/
void primLine2(sdword x0, sdword y0, sdword x1, sdword y1, color c)
{
    bool blendon;

    if (glcActive())
    {
        glcLine2(x0, y0, x1, y1, 1, c);
        return;
    }

    if (!glCapFeatureExists(GL_LINE_SMOOTH))
    {
        primNonAALine2(x0, y0, x1, y1, c);
        return;
    }

    blendon = glIsEnabled(GL_BLEND);
    if (!blendon) glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glBegin(GL_LINES);
    glVertex2f(primScreenToGLX(x0), primScreenToGLY(y0));
    glVertex2f(primScreenToGLX(x1), primScreenToGLY(y1));
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    if (!blendon) glDisable(GL_BLEND);
}

/*-----------------------------------------------------------------------------
    Name        : primNonAALine2
    Description : Draw a non-antialiased line 1 pixel wide
    Inputs      : c - attributes of line to be drawn
                  x0, y0, x1, y1 - start/end of line segment
    Outputs     : sets GL_COLOR
    Return      :
----------------------------------------------------------------------------*/
void primNonAALine2(sdword x0, sdword y0, sdword x1, sdword y1, color c)
{
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glBegin(GL_LINES);
    glVertex2f(primScreenToGLX(x0), primScreenToGLY(y0));
    glVertex2f(primScreenToGLX(x1), primScreenToGLY(y1));
    glEnd();
}

/*-----------------------------------------------------------------------------
    Name        : primLineThick2
    Description : Draw a line "thickness" pixels wide
    Inputs      : c - attributes of line to be drawn
                  x0, y0, y1, y1 - start/end of line segment
                  thickness - width of line
    Outputs     : Sets GL_COLOR
    Return      : void
----------------------------------------------------------------------------*/
void primLineThick2(sdword x0, sdword y0, sdword x1, sdword y1, sdword thickness, color c)
{
    glPushAttrib(GL_LINE_BIT);
    glLineWidth((GLfloat)thickness);
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glBegin(GL_LINES);
    glVertex2f(primScreenToGLX(x0), primScreenToGLY(y0));
    glVertex2f(primScreenToGLX(x1), primScreenToGLY(y1));
    glEnd();
    glPopAttrib();
}

/*-----------------------------------------------------------------------------
    Name        : primLineLoopStart2
    Description : Start a line-loop by setting up color and thickness
    Inputs      : thickness, c - attributes of lines to be drawn
    Outputs     : Sets GL_COLOR and GL_LINE_THICKNESS
    Return      : void
    Note        : Must be matched with a primLineLoopEnd2 and have only
                    primLineLoopPoint calls inbetween
----------------------------------------------------------------------------*/
static bool LLblendon;
void primLineLoopStart2(sdword thickness, color c)
{
    if (glCapFeatureExists(GL_LINE_SMOOTH))
    {
        LLblendon = glIsEnabled(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
        if (!LLblendon) glEnable(GL_BLEND);
    }
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glPushAttrib(GL_LINE_BIT);
    glLineWidth((GLfloat)thickness);
    glBegin(GL_LINE_LOOP);
}

/*-----------------------------------------------------------------------------
    Name        : primLineLoopPoint3F
    Description : Insert a point in a line-loop
    Inputs      : x, y - unconverted floating-point screen coordinates
    Outputs     : calls glVertex with the coordinates
    Return      :
----------------------------------------------------------------------------*/
void primLineLoopPoint3F(real32 x, real32 y)
{
    glVertex2f(x, y);
}

/*-----------------------------------------------------------------------------
    Name        : primLineLoopEnd2
    Description : End a line - loop sequence
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void primLineLoopEnd2(void)
{
    glEnd();
    glPopAttrib();
    if (glCapFeatureExists(GL_LINE_SMOOTH))
    {
        if (!LLblendon) glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cparam
    Description : blends a single color component
    Inputs      : a - component a, b - com b, t - lerp parm
    Outputs     :
    Return      : blended component
----------------------------------------------------------------------------*/
/*
uword cparam(uword a, uword b, real32 t)
{
    uword c;
    real32 bt = (real32)1 - t;
    c = (uword)((real32)a * t + (real32)b * bt);
    if (c > 255)
        return 255;
    else
        return c;
}
*/
/*-----------------------------------------------------------------------------
    Name        : decRect
    Description : decrease size of a rectangle
    Inputs      : rect - the rectangle, width - amount to dec
    Outputs     : rect is modified
    Return      :
----------------------------------------------------------------------------*/
void decRect(rectangle *rect, uword width)
{
    rect->x0 = rect->x0 + width;
    rect->y0 = rect->y0 + width;
    rect->x1 = rect->x1 - width;
    rect->y1 = rect->y1 - width;
}

/*-----------------------------------------------------------------------------
    Name        : primSeriesOfRects
    Description : draws a series of rects decreasing in size and color
    Inputs      : rect - starting rectangle, width - width,
                  fore - fg, back - fg, steps - number of rects
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primSeriesOfRects(rectangle *rect, uword width,
                       color fore, color back, uword steps)
{
    // ignore width
    rectangle drect;
    real32 cfrac, frac;
    uword i;
    color Color;
    cfrac = (real32)1.0 / (real32)steps;
    steps++;
    frac = (real32)1.0;
    Color = fore;
    memcpy(&drect, rect, sizeof(rectangle));
    for (i = 0; i < steps; i++)
    {
        decRect(&drect, 1);
        primRectOutline2(&drect, 2, Color);
        frac -= cfrac;
        Color = colBlend(fore, back, frac);
    }
}

#define SX(x) primScreenToGLX(x)
#define SY(y) primScreenToGLY(y)
#define X0 rect->x0
#define Y0 rect->y0
#define X1 rect->x1
#define Y1 rect->y1
#define SEGS 4
/*-----------------------------------------------------------------------------
    Name        : primRectSolid2
    Description : Draw a solid 2d rectangle.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
                  c - color to draw it in, xb - x offset, yb - y offset
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primBeveledRectSolid(rectangle *rect, color c, uword xb, uword yb)
{
    bool cull;

    if (glcActive())
    {
        glcBeveledRectSolid2(rect, c, (sdword)xb, (sdword)yb);
        return;
    }

    cull = glIsEnabled(GL_CULL_FACE) ? TRUE : FALSE;
    glDisable(GL_CULL_FACE);
    glBegin(GL_POLYGON);
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glVertex2f(SX(X0+xb), SY(Y0));
    glVertex2f(SX(X1-xb), SY(Y0));
    glVertex2f(SX(X1), SY(Y0+yb));
    glVertex2f(SX(X1), SY(Y1-yb));
    glVertex2f(SX(X1-xb), SY(Y1));
    glVertex2f(SX(X0+xb), SY(Y1));
    glVertex2f(SX(X0), SY(Y1-yb));
    glVertex2f(SX(X0), SY(Y0+yb));
    glEnd();
    if (cull)
    {
        glEnable(GL_CULL_FACE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : primBeveledRectOutline
    Description : Draw an outline 2d beveled rectangle.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
                  thickness - thickness of the lines
                  c - color to draw it in, xb - x offset, yb - y offset
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primBeveledRectOutline(rectangle *rect, sdword thickness, color c,
                            uword xb, uword yb)
{
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glPushAttrib(GL_LINE_BIT);
    glLineWidth((GLfloat)thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f(SX(X0+xb), SY(Y0));
    glVertex2f(SX(X1-xb), SY(Y0));
    glVertex2f(SX(X1), SY(Y0+yb));
    glVertex2f(SX(X1), SY(Y1-yb));
    glVertex2f(SX(X1-xb), SY(Y1));
    glVertex2f(SX(X0+xb), SY(Y1));
    glVertex2f(SX(X0), SY(Y1-yb));
    glVertex2f(SX(X0), SY(Y0+yb));
    glEnd();
    glPopAttrib();
}

/*-----------------------------------------------------------------------------
    Name        : primRoundRectOutline
    Description : Draw an outline 2d rounded rectangle.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
                  thickness - thickness of the lines
                  c - color to draw it in, xb - x offset, yb - y offset
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primRoundRectOutline(rectangle *rect, sdword thickness, color c, uword xb, uword yb)
{
    oval o;
    sdword segs = SEGS;

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glPushAttrib(GL_LINE_BIT);
    glLineWidth((GLfloat)thickness);
    glBegin(GL_LINES);
    glVertex2f(SX(X0+xb), SY(Y0));
    glVertex2f(SX(X1-xb), SY(Y0));
    glVertex2f(SX(X1), SY(Y0+yb));
    glVertex2f(SX(X1), SY(Y1-yb));
    glVertex2f(SX(X1-xb), SY(Y1));
    glVertex2f(SX(X0+xb), SY(Y1));
    glVertex2f(SX(X0), SY(Y1-yb));
    glVertex2f(SX(X0), SY(Y0+yb));
    glEnd();
    glPopAttrib();

//    if (xb > 4 || yb > 4)
//        segs *= 2;

    // upper left
    o.centreX = X0+xb;
    o.centreY = Y0+yb;
    o.radiusX = xb;
    o.radiusY = yb;
    primOvalArcOutline2(&o, 3*PI/2, TWOPI, 2, segs, c);

    // upper right
    o.centreX = X1-xb;
    primOvalArcOutline2(&o, (real32)0, PI/2, 2, segs, c);

    // lower right
    o.centreY = Y1-yb;
    primOvalArcOutline2(&o, PI/2, PI, 2, segs, c);

    // lower left
    o.centreX = X0+xb;
    primOvalArcOutline2(&o, PI, 3*PI/2, 2, segs, c);
}

/*-----------------------------------------------------------------------------
    Name        : primMaskedRoundRectOutline
    Description : Draw an outline 2d rounded rectangle.
    Inputs      : rect - pointer to rectangle structure containing coordinates.
                  thickness - thickness of the lines, mask - OL_* mask for roundedness
                  c - color to draw it in, xb - x offset, yb - y offset
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void primMaskedRoundRectOutline(rectangle *rect, sdword thickness, color c,
                                uword xb, uword yb, uword mask)
{
    oval o;
    sdword segs = SEGS;

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glPushAttrib(GL_LINE_BIT);
    glLineWidth((GLfloat)thickness);
    glBegin(GL_LINES);
    glVertex2f(SX(X0+xb), SY(Y0));
    glVertex2f(SX(X1-xb), SY(Y0));
    glVertex2f(SX(X1), SY(Y0+yb));
    glVertex2f(SX(X1), SY(Y1-yb));
    glVertex2f(SX(X1-xb), SY(Y1));
    glVertex2f(SX(X0+xb), SY(Y1));
    glVertex2f(SX(X0), SY(Y1-yb));
    glVertex2f(SX(X0), SY(Y0+yb));

    if (!(mask & OL_UL))
    {
        glVertex2f(SX(X0), SY(Y0));
        glVertex2f(SX(X0+xb), SY(Y0));
        glVertex2f(SX(X0), SY(Y0));
        glVertex2f(SX(X0), SY(Y0+yb));
    }
    if (!(mask & OL_LL))
    {
        glVertex2f(SX(X0), SY(Y1));
        glVertex2f(SX(X0+xb), SY(Y1));
        glVertex2f(SX(X0), SY(Y1));
        glVertex2f(SX(X0), SY(Y1-yb));
    }
    if (!(mask & OL_UR))
    {
        glVertex2f(SX(X1-xb), SY(Y0));
        glVertex2f(SX(X1), SY(Y0));
        glVertex2f(SX(X1), SY(Y0));
        glVertex2f(SX(X1), SY(Y0+yb));
    }
    if (!(mask & OL_LR))
    {
        glVertex2f(SX(X1), SY(Y1-yb));
        glVertex2f(SX(X1), SY(Y1));
        glVertex2f(SX(X1), SY(Y1));
        glVertex2f(SX(X1-xb), SY(Y1));
    }

    glEnd();
    glPopAttrib();

    if (xb > 4 || yb > 4)
        segs *= 2;

    // upper left
    o.centreX = X0+xb;
    o.centreY = Y0+yb;
    o.radiusX = xb;
    o.radiusY = yb;
    if (mask & OL_UL)
        primOvalArcOutline2(&o, 3*PI/2, TWOPI, 2, segs, c);

    // upper right
    o.centreX = X1-xb;
    if (mask & OL_UR)
        primOvalArcOutline2(&o, 0.0f, PI/2, 2, segs, c);

    // lower right
    o.centreY = Y1-yb;
    if (mask & OL_LR)
        primOvalArcOutline2(&o, PI/2, PI, 2, segs, c);

    // lower left
    o.centreX = X0+xb;
    if (mask & OL_LL)
        primOvalArcOutline2(&o, PI, 3*PI/2, 2, segs, c);
}
#undef SEGS
#undef Y1
#undef X1
#undef Y0
#undef X0
#undef SY
#undef SX

/*-----------------------------------------------------------------------------
    Name        : primSeriesOfBeveledRects
    Description : draws a series of rects decreasing in size and color
    Inputs      : rect - starting rectangle, width - width,
                  fore - fg, back - fg, steps - number of rects,
                  xb - x offset, yb - y offset
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primSeriesOfBeveledRects(rectangle *rect, uword width,
                              color fore, color back, uword steps,
                              uword xb, uword yb)
{
    // ignore width
    rectangle drect;
    real32 cfrac, frac;
    uword i;
    color Color;
    cfrac = (real32)1.0 / (real32)steps;
    steps++;
    frac = (real32)1.0;
    Color = fore;
    memcpy(&drect, rect, sizeof(rectangle));
    for (i = 0; i < steps; i++)
    {
        decRect(&drect, 1);
        primBeveledRectOutline(&drect, 2, Color, xb, yb);
        frac -= cfrac;
        Color = colBlend(fore, back, frac);
    }
}

/*-----------------------------------------------------------------------------
    Name        : primSeriesOfRoundRects
    Description : draws a series of rects decreasing in size and color
    Inputs      : rect - starting rectangle, width - width,
                  fore - fg, back - fg, steps - number of rects,
                  xb - x offset, yb - y offset
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primSeriesOfRoundRects(rectangle *rect, uword width,
                            color fore, color back, uword steps,
                            uword xb, uword yb)
{
    // ignore width
    rectangle drect;
    real32 cfrac, frac;
    uword i;
    color Color;
    cfrac = (real32)1.0 / (real32)steps;
    steps++;
    frac = (real32)1.0;
    Color = fore;
    memcpy(&drect, rect, sizeof(rectangle));
    for (i = 0; i < steps; i++)
    {
        decRect(&drect, 1);
        primRoundRectOutline(&drect, 2, Color, xb, yb);
        frac -= cfrac;
        Color = colBlend(fore, back, frac);
    }
}

/*-----------------------------------------------------------------------------
    Name        : partCircleSolid2
    Description : Render a 2d circle, like the 3D one.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void primCircleSolid2(sdword x, sdword y, sdword rad, sdword nSlices, color c)
{
    sdword index;
    GLfloat v[3];
    double theta;
    vector centre;
    real32 radiusX, radiusY;
    bool cull;

    if (glcActive())
    {
        rectangle r;
        r.x0 = x - rad;
        r.y0 = y - rad;
        r.x1 = x + rad;
        r.y1 = y + rad;
        glcRectSolid2(&r, c);
        return;
    }

    cull = glIsEnabled(GL_CULL_FACE) ? TRUE : FALSE;

    centre.x = primScreenToGLX(x);
    centre.y = primScreenToGLY(y);
    radiusX = primScreenToGLScaleX(rad);
    radiusY = primScreenToGLScaleY(rad);

    glColor4ub(colRed(c), colGreen(c),
               colBlue(c), colAlpha(c));
    v[0] = centre.x;
    v[1] = centre.y;
    glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(v[0], v[1]);
    for (index = 0, theta = 0.0; index < nSlices; index++)
    {
        v[0] = centre.x + (real32)(sin(theta)) * radiusX;
        v[1] = centre.y + (real32)(cos(theta)) * radiusY;
        theta += 2.0 * PI / (double)nSlices;
        glVertex2f(v[0], v[1]);
    }
    v[0] = centre.x;
    v[1] = centre.y + radiusY;
    glVertex2f(v[0], v[1]);
    glEnd();
    if (cull)
    {
        glEnable(GL_CULL_FACE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : primCircleBorder
    Description : Draw a special type of circle with a solid inner with a
                    alpha ramped outer ring.
    Inputs      : x, y - location of centre of image
                  radInner - radius of the inner circle
                  radOuter - total radius of the inner and outer circles
                  nSlices - number of radial slices must be multiple of 2
                  colInner - color of inner circle
                  colOuter - color of the outer edge
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#pragma optimize("", off)   //intel compiler pukes on this function so compile it without optimizations
void primCircleBorder(sdword x, sdword y, sdword radInner, sdword radOuter, sdword nSlices, color colInner)
{
    sdword index;
    real32 centreX, centreY;
    double theta, addAmount;
    real32 radXInner, radXOuter, radYInner, radYOuter;
    real32 x0, y0, x1, y1, x2, y2;
    real32 sinTheta, cosTheta;
    ubyte red = colRed(colInner);
    ubyte green = colGreen(colInner);
    ubyte blue = colBlue(colInner);

    dbgAssert(nSlices >= 3);

    centreX = primScreenToGLX(x);                           //make floating-point versions of parameters
    centreY = primScreenToGLY(y);
    radXInner = primScreenToGLScaleX(radInner);
    radYInner = primScreenToGLScaleY(radInner);
    radXOuter = primScreenToGLScaleX(radOuter);
    radYOuter = primScreenToGLScaleY(radOuter);
    addAmount = (double)(2.0f * PI / (real32)(nSlices - 1));
    theta = addAmount;

    x0 = x2 = centreX;
    y0 = centreY - radYInner;
    y2 = centreY - radYOuter;

    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    for (index = 0; index < nSlices; index++)
    {
        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(red, green, blue, 255);

        glVertex2f(x0, y0);                                 //2 common points
        glVertex2f(centreX, centreY);

        sinTheta = (real32)sin(theta);
        cosTheta = (real32)cos(theta);

        x0 = centreX + sinTheta * radXInner;
        y0 = centreY + cosTheta * radYInner;
        glVertex2f(x0, y0);                                 //complete
        glColor4ub(red, green, blue, 0);

        x1 = centreX + sinTheta * radXOuter;
        y1 = centreY + cosTheta * radYOuter;

        glVertex2f(x1, y1);                                 //complete
        glVertex2f(x2, y2);
        x2 = x1;
        y2 = y1;

        glEnd();
        theta += addAmount;
    }
    glDisable(GL_BLEND);
    glShadeModel(GL_FLAT);
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : primBlurryPoint2
    Description : Renders a 2D blurry dot in a specified color.
    Inputs      : x, y - location of the point on-screen
                  c - color to draw it in
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#define PD1_FuzzyPointYStart     -1
#define PD1_FuzzyPointYEnd       2
#define PD1_FuzzyPointXStart     -1
#define PD1_FuzzyPointXEnd       2
#define PD1_FuzzyPointHeight     4
#define PD1_FuzzyPointWidth      4

#define PD2_FuzzyPointYStart     -2
#define PD2_FuzzyPointYEnd       3
#define PD2_FuzzyPointXStart     -2
#define PD2_FuzzyPointXEnd       3
#define PD2_FuzzyPointHeight     6
#define PD2_FuzzyPointWidth      6
/*
ubyte p2dAlphaArray[PD1_FuzzyPointWidth][PD1_FuzzyPointHeight] =
{
    {0x10, 0x20, 0x10},
    {0x20, 0x3f, 0x20},
    {0x10, 0x20, 0x10},
};
*/
//1x1 pixel alpha'd out
/*
ubyte p2dAlphaArray1[PD1_FuzzyPointWidth][PD1_FuzzyPointHeight] =
{
    {0x07, 0x0e, 0x0e, 0x07},
    {0x0e, 0x1d, 0x1d, 0x0e},
    {0x0e, 0x1d, 0x1c, 0x0e},
    {0x07, 0x0e, 0x0e, 0x07},
};
*/
//1x1 pixel alpha'd out
ubyte p2dAlphaArray1[PD1_FuzzyPointWidth][PD1_FuzzyPointHeight] =
{
    {(0x0e * 2), (0x1c * 2), (0x1c * 2), (0x0e * 2)},
    {(0x1c * 2), (0x3a * 2), (0x3a * 2), (0x1c * 2)},
    {(0x1c * 2), (0x3a * 2), (0x3a * 2), (0x1c * 2)},
    {(0x0e * 2), (0x1c * 2), (0x1c * 2), (0x0e * 2)},
};
//2x2 pixel alpha'd out
ubyte p2dAlphaArray2[PD2_FuzzyPointWidth][PD2_FuzzyPointHeight] =
{
    {(0x04 * 2), (0x09 * 2), (0x13 * 2), (0x13 * 2), (0x09 * 2), (0x04 * 2)},
    {(0x0a * 2), (0x1a * 2), (0x2d * 2), (0x2d * 2), (0x1a * 2), (0x0a * 2)},
    {(0x13 * 2), (0x2d * 2), (0x4f * 2), (0x4f * 2), (0x2d * 2), (0x13 * 2)},
    {(0x13 * 2), (0x2d * 2), (0x4f * 2), (0x4f * 2), (0x2d * 2), (0x13 * 2)},
    {(0x0a * 2), (0x1a * 2), (0x2d * 2), (0x2d * 2), (0x1a * 2), (0x0a * 2)},
    {(0x04 * 2), (0x09 * 2), (0x13 * 2), (0x13 * 2), (0x09 * 2), (0x04 * 2)}
};
void primBlurryPoint2(sdword x, sdword y, color c)
{
    sdword iX, iY;
    ubyte *alpha, red = colRed(c), green = colGreen(c), blue = colBlue(c);

    glEnable(GL_BLEND);
    glBegin(GL_POINTS);
    alpha = &p2dAlphaArray1[0][0];
    for (iY = PD1_FuzzyPointYStart; iY <= PD1_FuzzyPointYEnd; iY++)
    {
        for (iX = PD1_FuzzyPointXStart; iX <= PD1_FuzzyPointXEnd; iX++, alpha++)
        {
            glColor4ub(red, green, blue, *alpha);
            glVertex2f(primScreenToGLX((x + iX)), primScreenToGLY((y + iY)));
        }
    }
    glEnd();
    glColor4ub(0, 0, 0, 0xff);
    glDisable(GL_BLEND);
}
void primBlurryPoint22(sdword x, sdword y, color c)
{
    sdword iX, iY;
    ubyte *alpha, red = colRed(c), green = colGreen(c), blue = colBlue(c);

    glEnable(GL_BLEND);
    glBegin(GL_POINTS);
    alpha = &p2dAlphaArray2[0][0];
    for (iY = PD2_FuzzyPointYStart; iY <= PD2_FuzzyPointYEnd; iY++)
    {
        for (iX = PD2_FuzzyPointXStart; iX <= PD2_FuzzyPointXEnd; iX++, alpha++)
        {
            glColor4ub(red, green, blue, *alpha);
            glVertex2f(primScreenToGLX((x + iX)), primScreenToGLY((y + iY)));
        }
    }
    glEnd();
    glColor4ub(0, 0, 0, 0xff);
    glDisable(GL_BLEND);
}

/*-----------------------------------------------------------------------------
    Name        : primPointLineIntersection
    Description : Computes an intersection of a point and a line segment
    Inputs      : xp/yp - location of point
                  x/y|0/1 - endpoints of the line segment
    Outputs     :
    Return      : positive/negative for different sides
----------------------------------------------------------------------------*/
real32 primPointLineIntersection(real32 xp, real32 yp, real32 x0, real32 y0, real32 x1, real32 y1)
{
    return((y0 - y1) * xp + (x1 - x0) * yp + (x0 * y1 - y0 * x1));
}

