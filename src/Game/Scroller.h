/*=============================================================================
    Name    : scroller.h
    Purpose : Functions for scrollbar type

    Created 11/10/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef __SCROLLER_H
#define __SCROLLER_H

#include "prim2d.h"
#include "UIControls.h"

#define widthOf(r) ((uword)(r.x1 - r.x0))
#define heightOf(r) ((uword)(r.y1 - r.y0))

typedef enum {SC_Other = 0, SC_Thumb, SC_Negative, SC_Positive, SC_NoRegion} sc_region;
typedef enum {SC_UpArrow = 0, SC_DownArrow, SC_LeftArrow, SC_RightArrow} sc_direction;

uword scClassifyRegion(scrollbarhandle shandle);
void scArrow(rectangle *r, uword width, uword thickness, color c, sc_direction dir);
void scAdjustThumbwheel(scrollbarhandle shandle, uword up, uword maxDisp, uword max);
uword scThumbRelativeX(scrollbarhandle shandle);
uword scThumbRelativeY(scrollbarhandle shandle);
uword scRelativeX(scrollbarhandle shandle);
uword scRelativeY(scrollbarhandle shandle);
scrollbarhandle scSetupThumbwheel(uword up, uword max, uword maxDisp, featom* atom);

#endif
