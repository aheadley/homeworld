/*=============================================================================
    Name    : scroller.c
    Purpose : Functions for scrollbar type

    Created 11/10/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "UIControls.h"
#include "prim2d.h"
#include "mouse.h"
#include "Scroller.h"

#ifdef khent
#define SCROLL_DEBUG 0
#endif

uword scClassifyRegion(scrollbarhandle shandle)
{
/*    if (mouseInRect(&shandle->neg))
        return SC_Negative;
    else if (mouseInRect(&shandle->pos))
        return SC_Positive;
    else if (mouseInRect(&shandle->thumb))
        return SC_Thumb;
    else*/
        return SC_Other;
}

uword scThumbRelativeX(scrollbarhandle shandle)
{
    return((uword)(mouseCursorX() - shandle->thumb.x0));
}

uword scThumbRelativeY(scrollbarhandle shandle)
{
    return((uword)(mouseCursorY() - shandle->thumb.y0));
}

uword scRelativeX(scrollbarhandle shandle)
{
    return((uword)(mouseCursorX() - shandle->thumbreg.x0));
}

uword scRelativeY(scrollbarhandle shandle)
{
    return((uword)(mouseCursorY() - shandle->thumbreg.y0));
}

/*-----------------------------------------------------------------------------
    Name        : scArrow
    Description : draw an arrow
    Inputs      : r - rectangle to put arrow in, width - width of lines,
                  c - color of lines, dir - 0 up, 1 dn, 2 lf, 3 rg
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scArrow(rectangle *r, uword width, uword thickness, color c, sc_direction dir)
{
    udword rw, rh, x, y, w2;
    rw = r->x1 - r->x0;
    rh = r->y1 - r->y0;
    w2 = width / 2;
    switch (dir)
    {
    case SC_UpArrow:
        y = r->y0 + ((rh - width) / 2);
        x = (rw / 2) + r->x0;
        primLineThick2(x, y, x - w2, y + width, thickness, c);
        primLineThick2(x, y, x + w2, y + width, thickness, c);
        break;
    case SC_DownArrow:
        y = r->y1 - ((rh - width) / 2);
        x = (rw / 2) + r->x0;
        primLineThick2(x, y, x - w2, y - width, thickness, c);
        primLineThick2(x, y, x + w2, y - width, thickness, c);
        break;
    case SC_LeftArrow:
        y = (rh / 2) + r->y0;
        x = r->x0 + ((rw - width) / 2);
        primLineThick2(x, y, x + width, y - w2, thickness, c);
        primLineThick2(x, y, x + width, y + w2, thickness, c);
        break;
    case SC_RightArrow:
        y = (rh / 2) + r->y0;
        x = r->x1 - ((rw - width) / 2);
        primLineThick2(x, y, x - width, y - w2, thickness, c);
        primLineThick2(x, y, x - width, y + w2, thickness, c);
        break;
    default:
        dbgMessagef("\nscArrow ? %d", dir);
    }
}

//adjust thumbwheel
void scAdjustThumbwheel(scrollbarhandle shandle, uword up, uword maxDisp, uword max)
{
    if (shandle->isVertical)
    {
        udword height, top, thumbHeight;
        real32 divSize;
#if SCROLL_DEBUG
        dbgMessagef("\nadjustThumbwheel %d %d %d", up, maxDisp, max);
#endif
        if (max <= maxDisp)
        {
            thumbHeight = (uword)(heightOf(shandle->thumbreg));
            divSize = 0.0f;
            shandle->thumb.y0 = shandle->thumbreg.y0;
            shandle->thumb.y1 = shandle->thumbreg.y0 + thumbHeight;
        }
        else
        {
            thumbHeight = (uword)(heightOf(shandle->thumbreg));
            thumbHeight = (uword)((real32)thumbHeight * (real32)maxDisp / (real32)max);
            height = heightOf(shandle->thumbreg)
                     - thumbHeight;
            divSize = (real32)height / (real32)(max - maxDisp);
            top = (uword)(divSize * (real32)up);
            shandle->thumb.y0 = shandle->thumbreg.y0 + top;
            shandle->thumb.y1 = shandle->thumbreg.y0 + top + thumbHeight;
        }
#if SCROLL_DEBUG
        dbgMessagef("\ntop = %d", top);
#endif
        shandle->divSize = divSize;
    }
    else
    {
        udword width, left, thumbWidth;
        real32 divSize;
#if SCROLL_DEBUG
        dbgMessagef("\nadjustThumbwheel %d %d %d", up, maxDisp, max);
#endif
        thumbWidth = (uword)(widthOf(shandle->thumbreg));
        thumbWidth = (uword)((real32)thumbWidth * (real32)maxDisp / (real32)max);
        width = widthOf(shandle->thumbreg)
                - thumbWidth;
        divSize = (real32)width / (real32)(max - maxDisp);
        left = (uword)(divSize * (real32)up);
        shandle->thumb.x0 = shandle->thumbreg.x0 + left;
        shandle->thumb.x1 = shandle->thumbreg.x0 + left + thumbWidth;
#if SCROLL_DEBUG
        dbgMessagef("\nleft = %d", left);
#endif
        shandle->divSize = divSize;
    }

#ifdef DEBUG_STOMP
    regVerify(((regionhandle)&shandle->reg));
#endif
    bitSet(((regionhandle)&shandle->reg)->status, RSF_DrawThisFrame);

}

//setup thumbwheel
scrollbarhandle scSetupThumbwheel(uword up, uword max, uword maxDisp, featom* atom)
{
    scrollbarhandle shandle;

    dbgAssert(atom != NULL);

    shandle = (scrollbarhandle)atom->pData;
    scAdjustThumbwheel(shandle, up, max, maxDisp);
    return shandle;
}
