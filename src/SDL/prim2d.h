/*=============================================================================
    Name    : prim2d.h
    Purpose : Abstraction for drawing 2D primitives.

    Created 6/26/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PRIM2D_H
#define ___PRIM2D_H

#include "Types.h"
#include "color.h"
#include "main.h"

/*=============================================================================
    Functions:
=============================================================================*/
#ifndef HW_Release

#define PRIM_ERROR_CHECKING     1               //general error checking

#else //HW_Debug

#define PRIM_ERROR_CHECKING     0               //general error checking

#endif //HW_Debug


/*=============================================================================
    Define:
=============================================================================*/
#define P2_OvalSegments         16

/*=============================================================================
    Type definitions:
=============================================================================*/
//basic 2D rectangle
typedef struct
{
    sdword x0, y0, x1, y1;
}
rectangle;

//basic 2D rectangle
typedef struct
{
    real32 x0, y0, x1, y1;
}
realrectangle;

//basic triangle structure
typedef struct
{
    sdword x0, y0, x1, y1, x2, y2;
}
triangle;

//basic oval structure
typedef struct
{
    sdword centreX, centreY;
    sdword radiusX, radiusY;
}
oval;

/*=============================================================================
    Data:
=============================================================================*/
extern sdword primModeEnabled;

/*=============================================================================
    Macros:
=============================================================================*/
#define primModeSet2() if (!primModeEnabled) primModeSetFunction2();
#define primModeClear2() if (primModeEnabled) primModeClearFunction2();

#define primScreenToGLX(x) (((real32)(x)+0.325f) / (real32)(MAIN_WindowWidth) * 2.0f - 1.0f)
#define primScreenToGLY(y) (1.0f - ((real32)(y)+0.325f) / (real32)(MAIN_WindowHeight) * 2.0f)
#define primScreenToGLScaleX(x) ((real32)(x) / (real32)(MAIN_WindowWidth) * 2.0f)
#define primScreenToGLScaleY(y) ((real32)(y) / (real32)(MAIN_WindowHeight) * 2.0f)
#define primGLToScreenX(x) (MAIN_WindowWidth / 2 + (sdword)((x) * (real32)MAIN_WindowWidth / 2.0f))
#define primGLToScreenY(y) (MAIN_WindowHeight / 2 - (sdword)((y) * (real32)MAIN_WindowHeight / 2.0f))
#define primGLToScreenScaleX(x) ((sdword)((x) * (real32)MAIN_WindowWidth / 2.0f))
#define primGLToScreenScaleY(y) ((sdword)((y) * (real32)MAIN_WindowHeight / 2.0f))

#define primPointInRectXY2(r, x, y)   ((x) >= (r)->x0 && (y) >= (r)->y0 && \
                                     (x) < (r)->x1 && (y) < (r)->y1)

#if PRIM_ERROR_CHECKING
#define primErrorMessagePrint()  primErrorMessagePrintFunction(__FILE__, __LINE__)
#else
#define primErrorMessagePrint()
#endif

/*=============================================================================
    Functions:
=============================================================================*/

//enable/disable primitive drawing mode do not call directly, use macros instead
void primModeSetFunction2(void);
void primModeClearFunction2(void);

//draw a single colored triangle
void primTriSolid2(triangle *tri, color c);
void primTriOutline2(triangle *tri, sdword thickness, color c);
//draw a rectangle
void primRectSolid2(rectangle *rect, color c);
void primRectTranslucent2(rectangle *rect, color c);
void primBeveledRectSolid(rectangle *rect, color c, uword xb, uword yb);
void primRectOutline2(rectangle *rect, sdword thickness, color c);
void primBeveledRectOutline(rectangle *rect, sdword thickness, color c, uword xb, uword yb);
void primRoundRectOutline(rectangle *rect, sdword thickness, color c, uword xb, uword yb);
void primRectShaded2(rectangle *rect, color *c); // color *c is a pointer to an array of 4 color values

#define OL_UL   0x01
#define OL_UR   0x02
#define OL_LR   0x04
#define OL_LL   0x08
#define OL_ALL  0x0F
void primMaskedRoundRectOutline(rectangle *rect, sdword thickness, color c,
        uword xb, uword yb, uword mask);

void primRectSolidTextured2(rectangle *rect);
void primRectSolidTexturedFullRect2(rectangle *rect);
void primRectSolidTexturedFullRectC2(rectangle *rect, color c);
//draw a line
void primLine2(sdword x0, sdword y0, sdword x1, sdword y1, color c);
void primNonAALine2(sdword x0, sdword y0, sdword x1, sdword y1, color c);
void primLineThick2(sdword x0, sdword y0, sdword x1, sdword y1, sdword thickness, color c);
//draw a line loop
void primLineLoopStart2(sdword thickness, color c);
void primLineLoopPoint3F(real32 x, real32 y);
void primLineLoopEnd2(void);
//draw solid circular things
void primCircleSolid2(sdword x, sdword y, sdword rad, sdword nSlices, color c);
void primCircleBorder(sdword x, sdword y, sdword radInner, sdword radOuter, sdword nSlices, color colInner);
//2d rectangle utility functions
void primRectUnion2(rectangle *result, rectangle *r0, rectangle *r1);
void primRealRectUnion2(realrectangle *result, realrectangle *r0, realrectangle *r1);
//draw oval arcs
void primOvalArcOutline2(oval *o, real32 radStart, real32 radEnd, sdword thickness, sdword segments, color c);
void primGLCircleOutline2(real32 x, real32 y, real32 radius, sdword nSegments, color c);
//series of successively blended rect outlines
void primSeriesOfRects(rectangle *rect, uword width,
                       color fore, color back, uword steps);
void primSeriesOfBeveledRects(rectangle *rect, uword width,
                              color fore, color back, uword steps,
                              uword xb, uword yb);
void primSeriesOfRoundRects(rectangle *rect, uword width,
                            color fore, color back, uword steps,
                            uword xb, uword yb);

//points
void primBlurryPoint2(sdword x, sdword y, color c);
void primBlurryPoint22(sdword x, sdword y, color c);

//report errors
void primErrorMessagePrintFunction(char *file, sdword line);

//utility functions
real32 primPointLineIntersection(real32 xp, real32 yp, real32 x0, real32 y0, real32 x1, real32 y1);

#endif //___PRIM2D_H


