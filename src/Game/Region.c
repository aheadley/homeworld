/*=============================================================================
    Name    : region.c
    Purpose : Process the creation, deletion, processing and drawing of all
        region-based UI elements including buttons and menus.

    Created 6/25/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "Types.h"
#include "Debug.h"
#include "Memory.h"
#include "Task.h"
#include "Key.h"
#include "prim2d.h"
#include "mouse.h"
#include "Demo.h"
#include "main.h"
#include "utility.h"
#include "Region.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

/*=============================================================================
    Data:
=============================================================================*/
void regNULLRenderFunction(regionhandle region);
//the base, root region to which everything is linked.  Even the main play
//screen is a child of this region.  Sometimes referring to a regionhandle
//of NULL will really mean this region.
region regRootRegion =
{
    {0, 0, 0, 0},                               //no physical size
    regNULLRenderFunction,                      //no draw function
    regNULLProcessFunction,                     //no processor functions
    NULL, NULL, NULL, NULL,                     //no links to start
    0,//RFF_ModalBreak | RFF_NULLRegion,            //the processing stops here
    0,                                          //no status flags
    0,                                          //zero accelerator keys
    {0},                                        //no key accelerator
    0,                                          //no user ID
#if REG_ERROR_CHECKING
    REG_ValidationKey
#endif
};

sdword regModuleInit = FALSE;                   //module init flag
sdword regDrawBorders = TRUE;
//sdword regContinueTree;

//region render event queue
sdword regNumberRenderEvents = REG_RenderEventsDefault;//default number of render events
sdword regRenderEventIndex;
regrenderevent *regRenderEvent;

taskhandle regTaskHandle;
taskhandle regDrawTaskHandle;

//regions the mouse was clicked in
regionhandle regClickedLeft;
regionhandle regClickedLeftLast;  //for making double-clicks work properly
regionhandle regClickedRight;
regionhandle regClickedCentre;

//flag that we are processing regions and we should continue not alter behaviour
//of some functions which may be called from region handler functions
sdword regProcessingRegions;

#if REG_INDEX_ALLOCS
sdword regAllocCounter = 0;
#endif

udword regRegionFrameDrawCount=0;

bool regKeysFocussed = FALSE;

/*=============================================================================
    Private functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : regDrawBorderFunction
    Description : Add a draw event to draw a border around specified region.
    Inputs      : same as any render function
    Outputs     : renders the border using primitives
    Return      : void
----------------------------------------------------------------------------*/
#if REG_DRAW_REGION_BORDERS
void regDrawBorderFunction(regionhandle reg)
{
    primRectOutline2(&reg->rect, 1, REG_OutlineColor);
}
#endif //REG_DRAW_REGION_BORDERS


/*-----------------------------------------------------------------------------
    Name        : regFunctionCall
    Description : Calls the region's processor function with the specified
                    event and data.
    Inputs      : reg - region which generated message
                  event - event message to generate
                  data - data to pass onto processor function
    Outputs     : calls function passing along same data
    Return      : TRUE if message processed properly, FALSE otherwise
----------------------------------------------------------------------------*/
sdword regFunctionCall(regionhandle reg, udword event, udword data, udword *maskPointer)
{
    udword returnValue;
    udword mask = *maskPointer;

    if ( (bitTest(reg->status, RSF_ToBeDeleted)) ||
         (bitTest(reg->status, RSF_RegionDisabled)))
    {
        return(FALSE);
    }
    if (!regFlag(event))                                    //only pass events in filter
    {
        return(FALSE);
    }
    returnValue = reg->processFunction(reg, reg->userID, event, data);
    if (bitTest(returnValue, RPR_Redraw))
    {
#ifdef DEBUG_STOMP
        regVerify(reg);
#endif
        bitSet(reg->status, RSF_DrawThisFrame);
    }
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : regKeysPressed/regKeysStuck/regKeysPressedOrStuck
    Description : Check if all keys for specified region pressed/stuck/either
    Inputs      : reg - region to ckeck keys for
    Outputs     : ..
    Return      : TRUE if all region keys pressed, false otherwise.
----------------------------------------------------------------------------*/
sdword regKeysPressed(regionhandle reg)
{
    sdword index;

    regVerify(reg);
    for (index = 0; index < reg->nKeys; index++)
    {
        if (!(keyIsHit(reg->key[index])))
        {                                                   //if this key not pressed
            return(FALSE);
        }
    }
    return(TRUE);
}
sdword regKeysStuck(regionhandle reg)
{
    sdword index;

    regVerify(reg);
    for (index = 0; index < reg->nKeys; index++)
    {
        if (!(keyIsStuck(reg->key[index])))
        {                                                   //if this key not pressed
            return(FALSE);
        }
    }
    return(TRUE);
}
sdword regKeysPressedOrStuck(regionhandle reg)
{
    sdword index;

    regVerify(reg);
    for (index = 0; index < reg->nKeys; index++)
    {
        if (!(keyIsHit(reg->key[index]) || keyIsStuck(reg->key[index])))
        {                                                   //if this key not pressed
            return(FALSE);
        }
    }
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : regKeysUnstick
    Description : Clear sticky bit on keys for a region
    Inputs      : reg - region with keys
    Outputs     : Calls keyClearSticky for each key in region.
    Return      : void
    Note        : Later, it may be useful to only clear the sticky bit for
                    the last key.  That way, two keys of CTRL-Q and CTRL-C would
                    work without having to ever release the CTRL key.
----------------------------------------------------------------------------*/
void regKeysUnstick(regionhandle reg)
{
    sdword index;

    regVerify(reg);
    for (index = 0; index < reg->nKeys; index++)
    {
        keyClearSticky(reg->key[index]);
        keyClearRepeat(reg->key[index]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : regDrawFunctionAddPossibly
    Description : Add draw function to draw function list if it is not already there.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regDrawFunctionAddPossibly(regionhandle region)
{
    if (region->lastframedrawn < regRegionFrameDrawCount)
    {
        regDrawFunctionAdd(region->drawFunction, region);
        region->lastframedrawn = regRegionFrameDrawCount;
    }
}

/*-----------------------------------------------------------------------------
    Name        : regDirtyChildren
    Description : dirties all children of a region
    Inputs      : reg - the region whose children will be recursively dirtied
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regDirtyChildren(regionhandle reg)
{
regionhandle child;

    if (reg == NULL)
        return;

    for (child = reg->child; child != NULL; child = child->next)
    {
        regDrawFunctionAddPossibly(child);
        regDirtyChildren(child->child);
    }
}


/*-----------------------------------------------------------------------------
    Name        : regRecursiveSetDirty
    Description : Sets the RSF_DrawThisFrame bit for all children of a region
    Inputs      : reg - the region whose children will be recursively dirtied
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regRecursiveSetDirty(regionhandle reg)
{
regionhandle child;

    if (reg == NULL)
        return;

    // now dirty all the siblings and their children
    for (child = reg->child; child != NULL; child = child->next)
    {
#ifdef DEBUG_STOMP
        regVerify(child);
#endif
        bitSet(child->status, RSF_DrawThisFrame);
        regRecursiveSetDirty(child);
    }
#ifdef DEBUG_STOMP
    regVerify(reg);
#endif
    bitSet(reg->status, RSF_DrawThisFrame);
}
/*-----------------------------------------------------------------------------
    Name        : regRecursiveSetReallyDirty
    Description : Sets the RSF_ReallyDirty bit for all children of a region
    Inputs      : reg - the region whose children will be recursively dirtied
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regRecursiveSetReallyDirty(regionhandle reg)
{
regionhandle child;

    if (reg == NULL)
        return;

    // now dirty all the siblings and their children
    for (child = reg->child; child != NULL; child = child->next)
    {
        bitSet(child->status, RSF_ReallyDirty);
        regRecursiveSetDirty(child);
    }
    bitSet(reg->status, RSF_ReallyDirty);
}


/*-----------------------------------------------------------------------------
    Name        : regPointOnRegion
    Description : determines whether a point is on the border or inside a region
    Inputs      : x, y - screen location of point to test
                  region - the region to test against
    Outputs     :
    Return      : TRUE (on) or FALSE (not on)
----------------------------------------------------------------------------*/
bool regPointOnRegion(sdword x, sdword y, regionhandle region)
{
    rectangle* r = &region->rect;
    if (x >= r->x0 && x <= r->x1 &&
        y >= r->y0 && y <= r->y1)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : regRegionInside
    Description : determines whether a region is entirely inside/on another
    Inputs      : needle - subregion
                  haystack - base region
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool regRegionInside(regionhandle needle, regionhandle haystack)
{
#define TOTALLY_INSIDE      0
#if TOTALLY_INSIDE
    if (regPointOnRegion(needle->rect.x0, needle->rect.y0, haystack) &&
        regPointOnRegion(needle->rect.x0, needle->rect.y1, haystack) &&
        regPointOnRegion(needle->rect.x1, needle->rect.y1, haystack) &&
        regPointOnRegion(needle->rect.x1, needle->rect.y0, haystack))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else //TOTALLY_INSIDE
    //!!! temporary hack to make button text visible.  This function checks to see
    //if the top region is more or less on the bottom region:  It must be within the
    //vertical limits and no more that 50% off horizontally.
    rectangle *nRect, *hRect;
    sdword nWidth, hWidth, overlap;

    nRect = &needle->rect;
    hRect = &haystack->rect;
#define OVERLAP_FACTOR      2/3
    if (nRect->y0 >= hRect->y0 && nRect->y1 <= hRect->y1)
    {                                                       //if within the vertical limits
        if (nRect->x0 >= hRect->x0 && nRect->x1 <= hRect->x1)
        {                                                   //if within the horizontal limits
            return(TRUE);                                   //condition 3
        }
        else
        {                                                   //not inside, look for overlap
            hWidth = hRect->x1 - hRect->x0;
            nWidth = nRect->x1 - nRect->x0;
            if (nRect->x0 < hRect->x0)
            {                                               //condition 0, 1 or 4
                if (nRect->x1 > hRect->x1)
                {                                           //condition 0
                    overlap = hWidth;
                }
                else if (nRect->x1 > hRect->x0)
                {                                           //condition 1
                    overlap = nRect->x1 - hRect->x0;
                }
                else
                {                                           //condition 4
                    overlap = -1;
                }
            }
            else
            {                                               //condition 2 or 5
                if (hRect->x1 > nRect->x0)
                {                                           //condition 2
                    overlap = hRect->x1 - nRect->x0;
                }
                else
                {                                           //condition 5
                    overlap = -1;
                }
            }
        }
        if (overlap >= nWidth * OVERLAP_FACTOR)
        {                                                   //if it overlaps enough
            return(TRUE);
        }
    }
    return(FALSE);
#endif //TOTALLY_INSIDE
}

/*-----------------------------------------------------------------------------
    Name        : regDirtySiblingsInside
    Description : dirties all siblings inside another region, "upwards"
    Inputs      : region - the region to start with
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regDirtySiblingsInside(regionhandle region)
{
    regionhandle reg;

    dbgAssert(region != NULL);

    reg = region->previous;

    //dirty internal siblings
    while (reg != NULL)
    {
        if (regRegionInside(reg, region))
        {
            regDrawFunctionAddPossibly(reg);
        }
        reg = reg->previous;
    }
}

/*-----------------------------------------------------------------------------
    Name        : regDoRegionsOverlap
    Description : determines whether two provided regions overlap
    Inputs      : a, b - the regions to test
    Outputs     :
    Return      : 0 (no overlap) or 1 (overlap)
----------------------------------------------------------------------------*/
bool regDoRegionsOverlap(regionhandle a, regionhandle b)
{
    rectangle c;

    primRectUnion2(&c, &a->rect, &b->rect);
    return (c.x0 == c.x1 || c.y0 == c.y1);
}

/*-----------------------------------------------------------------------------
    Name        : regDirtyEverythingUpwardsSelectively
    Description : dirties all parents of a region recursively, only if they overlap
                  the provided topmost region
    Inputs      : region - the region to proceed upwards from
                  overlap - the region to overlap test against (topmost region)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regDirtyEverythingUpwardsSelectively(regionhandle region, regionhandle overlap)
{
    regionhandle reg = region;

    if (reg->parent == NULL)
    {
        return;
    }

    reg = reg->parent;
    for (;; reg = reg->next)
    {
        if (regDoRegionsOverlap(overlap, reg))
        {
#ifdef DEBUG_STOMP
            regVerify(reg);
#endif
            bitSet(reg->status, RSF_DrawThisFrame);
        }
        if (reg->parent != NULL)
        {
            regDirtyEverythingUpwardsSelectively(reg, overlap);
        }
        if (reg->next == NULL)
        {
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : regDirtyEverythingUpwards
    Description : dirties all parents of a region recursively
    Inputs      : region - the region to proceed upwards from
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regDirtyEverythingUpwards(regionhandle region)
{
    regionhandle reg = region;

    if (reg->parent == NULL)
    {
        return;
    }

    reg = reg->parent;
    for (;; reg = reg->next)
    {
#ifdef DEBUG_STOMP
        regVerify(reg);
#endif
        bitSet(reg->status, RSF_DrawThisFrame);
        if (reg->parent != NULL)
        {
            regDirtyEverythingUpwards(reg);
        }
        if (reg->next == NULL)
        {
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : regDirtyScreensAboveRegion
    Description : Dirties the screens above (on top) of the region
    Inputs      : region
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regDirtyScreensAboveRegion(regionhandle region)
{
    sdword i,j;

    if (region->parent == NULL)
    {
        return;
    }

    for (i=0;i<feStackIndex;i++)
    {
        if (feStack[i].baseRegion == region->parent)
        {
            for (j=i+1;j<=feStackIndex;j++)
            {
                regRecursiveSetDirty(feStack[j].baseRegion);
            }
            return;
        }
    }

    return;
}

/*-----------------------------------------------------------------------------
    Name        : regRegionProcess
    Description : Process a region, including it's subtree.
    Inputs      : reg - region to process
                  mask - region processing mask to be ANDed with the region's
                    flags member.
    Outputs     : processes events for region and all it's children
    Return      : modified mask variable for use with parents
    Note        : this function is recursive and regions process all their
                    children before processing itself.  This way, the function
                    will progress from bottom to top.
----------------------------------------------------------------------------*/
udword regRegionProcess(regionhandle reg, udword mask)
{
    regionhandle child, nextChild;
    udword returnMask;

    regVerify(reg);
    bitSet(reg->status, RSF_Processing);                    //we're currently using this region

    // Test to see if this and all children need updating
    if(bitTest(reg->status, RSF_ReallyDirty))
    {
        regRecursiveSetDirty(reg);
        bitClear(reg->status, RSF_ReallyDirty);
    }

    for (child = reg->child; child != NULL; child = nextChild)//process all children first
    {
        mask = regRegionProcess(child, mask);
        nextChild = child->next;
        if (bitTest(child->status, RSF_ToBeDeleted))        //if this region marked for deletion
        {                                                   //delete it and all it's children
            regRegionDelete(child);
        }
    }
    //after processing children, process self
#if REG_VERBOSE_LEVEL >= 2
    dbgMessagef("\nProcessing region 0x%x", reg);
#endif
    if (regFlag(RPE_ModalBreak))
    {                                                       //if it's a modal break region
        mask &= ~(RPM_AllEvents);                           //process no more events
    }
    returnMask = mask;

    //perform mouse logic
    if ((regClickedLeft == NULL || regClickedLeft == reg) && bitTest(reg->flags, RPM_MouseEvents))
    {
        if (mouseInRect(&reg->rect))
        {                                                   //inside region
            if (keyIsHit(FLYWHEEL_UP))
            {
//                keyScanCode[FLYWHEEL_UP].keypressed--;
                keyPressUp(FLYWHEEL_UP);
                regFunctionCall(reg, RPE_WheelUp, 0, &mask);
            }
            if (keyIsHit(FLYWHEEL_DOWN))
            {
//                keyScanCode[FLYWHEEL_DOWN].keypressed--;
                keyPressUp(FLYWHEEL_DOWN);
                regFunctionCall(reg, RPE_WheelDown, 0, &mask);
            }
            if (regFlag(RPE_DoubleLeft))
            {
                if (keyIsStuck(LMOUSE_DOUBLE))
                {
                    if (reg == regClickedLeftLast)
                    {
                        regFunctionCall(reg, RPE_DoubleLeft, 0, &mask);
                        keyClearSticky(LMOUSE_DOUBLE);
                        bitClear(returnMask, RPE_DoubleLeft);
                    }
                    else
                    {
                        dbgMessage("\nSecond of double click in new region");
                        keyClearSticky(LMOUSE_DOUBLE);
                        bitClear(returnMask, RPE_DoubleLeft);
                    }

                }
            }
            if (regFlag(RPE_DoubleRight))
            {
                if (keyIsStuck(RMOUSE_DOUBLE))
                {
                    regFunctionCall(reg, RPE_DoubleRight, 0, &mask);
                    keyClearSticky(RMOUSE_DOUBLE);
                    bitClear(returnMask, RPE_DoubleRight);

                }
            }
            if (regFlag(RPE_DoubleCentre))
            {
                if (keyIsStuck(MMOUSE_DOUBLE))
                {
                    regFunctionCall(reg, RPE_DoubleCentre, 0, &mask);
                    keyClearSticky(MMOUSE_DOUBLE);
                    bitClear(returnMask, RPE_DoubleCentre);

                }
            }
            if (!regInside(reg))
            {                                               //just entered region
                if (regLeftPressed(reg))
                {                                           //left held and entering region
                    //...call hold-enter function...
                    regFunctionCall(reg, RPE_EnterHoldLeft, 0, &mask);
                    bitSet(reg->status, RSF_LeftPressed);   //set pressed status bit
                    regClickedLeft = reg;                   //no has mouse press focus
                    regClickedLeftLast = reg;               //save for doubleclick check
                }
                if (regRightPressed(reg))
                {                                           //right held and entering region
                    //...call hold-enter function...
                    regFunctionCall(reg, RPE_EnterHoldRight, 0, &mask);
                    bitSet(reg->status, RSF_RightPressed);  //set pressed status bit
                }
                if (regLeftPressed(reg))
                {                                           //centre held and entering region
                    //...call hold-enter function...
                    regFunctionCall(reg, RPE_EnterHoldCentre, 0, &mask);
                    bitSet(reg->status, RSF_CentrePressed); //set pressed status bit
                }
                //...same for right/centre
                //...call enter function...
                regFunctionCall(reg, RPE_Enter, 0, &mask);
                bitSet(reg->status, RSF_MouseInside);       //set inside bit
//              if (mouseLeftButton())
//              {                                           //if mouse button unpressed outside of region
//                  bitSet(reg->status, RSF_LeftPressed);   //set pressed status bit
//                  regClickedLeft = NULL;                  //no has mouse press focus
//              }
            }
            else
            {                                               //else still in region
                if (mouseLeftButton())
                {                                           //left button down in region
                    if (!regLeftPressed(reg))
                    {                                       //left button pressed in region
                        //...call pressed function, set bit...
                        if (regFunctionCall(reg, RPE_PressLeft, 0, &mask))
                        {
                            bitSet(reg->status, RSF_LeftPressed);
                            regClickedLeft = reg;           //record clicked-in region
                            regClickedLeftLast = reg;               //save for doubleclick check

                        }
                        bitClear(returnMask, RPM_PressRelease); //disallow any further mouse clicks
                    }
                    else
                    {                                       //left button held in region
                        //...call hold function...
                        regFunctionCall(reg, RPE_HoldLeft, 0, &mask);
                    }
                }
                else
                {                                           //else button not pressed
                    if (regLeftPressed(reg))
                    {                                       //button just released
                        //...call release function, clear bit...
                        regFunctionCall(reg, RPE_ReleaseLeft, 0, &mask);
                        regClickedLeft = NULL;              //no buttons pressed
                        bitClear(returnMask, RPM_PressRelease); //disallow any further mouse clicks
                        bitClear(reg->status, RSF_LeftPressed);
                    }
                }
                if (mouseRightButton())
                {                                           //right button down in region
                    if (!regRightPressed(reg))
                    {                                       //right button pressed in region
                        //...call pressed function, set bit...
                        if (regFunctionCall(reg, RPE_PressRight, 0, &mask))
                        {
                            bitSet(reg->status, RSF_RightPressed);
                        }
                        bitClear(returnMask, RPM_PressRelease); //disallow any further mouse clicks
                    }
                    else
                    {                                       //right button held in region
                        //...call hold function...
                        regFunctionCall(reg, RPE_HoldRight, 0, &mask);
                    }
                }
                else
                {                                           //else button not pressed
                    if (regRightPressed(reg))
                    {                                       //button just released
                        //...call release function, clear bit...
                        regFunctionCall(reg, RPE_ReleaseRight, 0, &mask);
                        bitClear(returnMask, RPM_PressRelease); //disallow any further mouse clicks
                        bitClear(reg->status, RSF_RightPressed);
                    }
                }
                if (mouseCentreButton())
                {                                           //centre button down in region
                    if (!regCentrePressed(reg))
                    {                                       //centre button pressed in region
                        //...call pressed function, set bit...
                        if (regFunctionCall(reg, RPE_PressCentre, 0, &mask))
                        {
                            bitSet(reg->status, RSF_CentrePressed);
                        }
                        bitClear(returnMask, RPM_PressRelease); //disallow any further mouse clicks
                    }
                    else
                    {                                       //centre button held in region
                        //...call hold function...
                        regFunctionCall(reg, RPE_HoldCentre, 0, &mask);
                    }
                }
                else
                {                                           //else button not pressed
                    if (regCentrePressed(reg))
                    {                                       //button just released
                        //...call release function, clear bit...
                        regFunctionCall(reg, RPE_ReleaseCentre, 0, &mask);
                        bitClear(returnMask, RPM_PressRelease); //disallow any further mouse clicks
                        bitClear(reg->status, RSF_CentrePressed);
                    }
                }
            }
        }
        else
        {                                                   //else outside region
            if (regInside(reg))
            {                                               //just exiting region
                bitClear(reg->status, RSF_MouseInside);     //exiting, clear inside bit
                if (regLeftPressed(reg))
                {                                           //left held and exiting region
                    //...call hold-exit function...
                    regFunctionCall(reg, RPE_ExitHoldLeft, 0, &mask);
                }
                if (regRightPressed(reg))
                {                                           //right held and exiting region
                //...call hold-exit function...
                    regFunctionCall(reg, RPE_ExitHoldRight, 0, &mask);
                }
                if (regCentrePressed(reg))
                {                                           //centre held and exiting region
                    //...call hold-exit function...
                    regFunctionCall(reg, RPE_ExitHoldCentre, 0, &mask);
                }
                //...call exit function...
                regFunctionCall(reg, RPE_Exit, 0, &mask);
            }
            if (!mouseLeftButton())
            {                                               //if mouse button unpressed outside of region
                if (bitTest(reg->status, RSF_LeftPressed))
                {
#ifdef DEBUG_STOMP
                    regVerify(reg);
#endif
                    bitSet(reg->status, RSF_DrawThisFrame);
                }
                bitClear(reg->status, RSF_LeftPressed);     //clear pressed status bit
                regClickedLeft = NULL;                      //no has mouse press focus
            }
        }
    }
    //perform key logic
    if (regStatus(RSF_KeyCapture))
    {                                                       //if region has input forcus for keyboard
        sdword iKey;
        bool bShift;

        while ((iKey = keyBufferedKeyGet(&bShift)) != 0)
        {
            if (bShift)
            {
                iKey |= RF_ShiftBit;
            }
            regFunctionCall(reg, RPE_KeyDown, iKey, &mask);//call the function
        }
    }
    else if (bitTest(reg->flags, RPM_KeyEvent) && (!regKeysFocussed))
    {                                                       //if any key checking for this region
        if (!regKeyPressed(reg))
        {                                                   //if region key bit not set
            if (regKeysStuck(reg))
            {                                               //if now pressed
                //...call key down function, set down bit...
                if (regFunctionCall(reg, RPE_KeyDown, 0, &mask))//call press function
                {
                    regKeysUnstick(reg);                    //unstick keys
                    bitSet(reg->status, RSF_KeyPressed);    //set pressed bit
                }
            }
            else
            {                                               //keys not pressed, clear pressed bit
                bitClear(reg->status, RSF_KeyPressed);
            }
        }
        else
        {                                                   //region key bit not set
            if (regKeysPressed(reg))
            {                                               //keys held, check hold or repeat
                if (regFlag(RPE_KeyHold))
                {                                           //if a hold region(hold and repeat are exclusive)
                    regFunctionCall(reg, RPE_KeyHold, 0, &mask);
                }
                else if (regFlag(RPE_KeyRepeat))
                {                                           //if a repeat key
                    if (regKeysStuck(reg))
                    {                                       //unstick stuck keys
                        regKeysUnstick(reg);                //unstick keys
                        regFunctionCall(reg, RPE_KeyRepeat, 0, &mask);//call repeat function
                    }
                }
            }
            else
            {                                               //region keys just released
                //...key up function, clear down bit...
                regFunctionCall(reg, RPE_KeyUp, 0, &mask);
                bitClear(reg->status, RSF_KeyPressed);
            }
        }
    }
    bitClear(reg->status, RSF_Processing);                  //no longer using this region
    if (feShouldSaveMouseCursor())
    {
        if (bitTest(reg->flags, RPE_DrawEveryFrame))            //check permanent draw flag for this region
        {
#ifdef DEBUG_STOMP
            regVerify(reg);
#endif
            bitSet(reg->status, RSF_DrawThisFrame);
        }
    }
    else
    {
        //we must redraw the entire frontend
#ifdef DEBUG_STOMP
        regVerify(reg);
#endif
        bitSet(reg->status, RSF_DrawThisFrame);
    }
#if REG_DRAW_REGION_BORDERS
    if (regDrawBorders)
    {
        regDrawFunctionAdd(regDrawBorderFunction, reg);     //draw a dummy border in special debugging mode
    }
#endif //REG_DRAW_REGION_BORDERS
#ifdef DEBUG_STOMP
    regVerify(reg);
#endif
    if (bitTest(reg->status, RSF_DrawThisFrame))            //if need to draw this frame
    {
        if (feShouldSaveMouseCursor())
        {
            regDirtyChildren(reg);
//            regDirtySiblingsInside(reg);
        }
        regDrawFunctionAddPossibly(reg);
#ifdef DEBUG_STOMP
        regVerify(reg);
#endif
        bitClear(reg->status, RSF_DrawThisFrame);
    }
    return(returnMask);
}

/*-----------------------------------------------------------------------------
    Name        : regProcessTask
    Description : Task function to act as region processor kernel.
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void regProcessTask(void)
{
    taskYield(0);

#ifndef C_ONLY
    while (1)
#endif
    {
        taskStackSaveCond(0);
#if REG_VERBOSE_LEVEL >= 2
        dbgMessage("\nProcessing regions...");
#endif
        mousePoll();                                        //poll mouse
#if defined (_MSC_VER)
        _asm xor eax,eax
        _asm mov regRenderEventIndex, eax
#elif defined (__GNUC__) && defined (__i386__)
        __asm__ __volatile__ ( "xorl %%eax, %%eax\n\t" : "=a" (regRenderEventIndex) );
#else
        regRenderEventIndex = 0;
#endif
//        regRenderEventIndex = regRenderEventIndex - regRenderEventIndex;                            //no render events yet this frame

        //special-case code for double-clicks
/*
        if (keyIsHit(LMOUSE_DOUBLE))
        {
            keyPressUp(LMOUSE_DOUBLE);
            utyDoubleClick();
        }
*/
        if (demDemoRecording)
        {
            memcpy((ubyte *)&keyScanCode[0], (ubyte *)&keySaveScan[0], sizeof(keyScanCode));//freeze a snapshot of the key state
            demStateSave();
        }
        else if (demDemoPlaying)
        {
            demStateLoad();
        }
        regProcessingRegions = TRUE;
        regRegionProcess(&regRootRegion, 0xffffffff);
        regProcessingRegions = FALSE;

        taskStackRestoreCond();
        taskYield(0);
    }

    taskExit();
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Test processing functions:
-----------------------------------------------------------------------------*/
#if REG_TEST
static udword regProcessCallback(regionhandle region, sdword ID, udword event, udword data)
{
    dbgMessagef("\nregProcessCallback: region = 0x%x, ID = 0x%x, event = 0x%x, data = %d", region, ID, event, data);
    return 0;
}
#endif

/*-----------------------------------------------------------------------------
    Null processor functions for processing and drawing
-----------------------------------------------------------------------------*/
void regNULLRenderFunction(regionhandle region)
{
    ;
}
udword regNULLProcessFunction(regionhandle region, sdword ID, udword event, udword data)
{
    return(0);
}

/*=============================================================================
    Functions
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : regStartup
    Description : Startup the region handling module.
    Inputs      : void
    Outputs     : Allocates render event list and starts the region handling task.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword regStartup(void)
{
    regNotInitCheck();

    //allocate the render event list
    regRenderEvent = memAlloc(regNumberRenderEvents * sizeof(regrenderevent), "RegionRenderEvents", NonVolatile);

    //start the region processor task
    regTaskHandle = taskStart(regProcessTask, REG_TaskFrequencyOPF, TF_OncePerFrame);
    regModuleInit = TRUE;                                   //flag module started

    regClickedLeft = regClickedRight = regClickedCentre = NULL;//mouse not clicked anywhere
#if REG_TEST
    {
        regionhandle a, b, c, d, e, f, g, master;

        master = regChildAlloc(NULL, 'M', 0, 0, 200, 200, 0, RPE_LeftClickButton); //create root dummy regions
        regNameSet(master, "Master Region");
        regFunctionSet(master, regProcessCallback);
        a = regChildAlloc(master, 'A', 0, 0, 10, 10, 0, RPE_LeftClickButton);        //create some dummy regions
        regNameSet(a, "Region A");
        regFunctionSet(a, regProcessCallback);
        b = regChildAlloc(master, 'B', 20, 0, 10, 10, 0, RPE_LeftClickButton);
        regNameSet(b, "Region B");
        regFunctionSet(b, regProcessCallback);
        c = regChildAlloc(a, 'C', 0, 20, 10, 10, 0, RPE_LeftClickButton);
        regNameSet(c, "Region C");
        regFunctionSet(c, regProcessCallback);
        d = regSiblingAlloc(b, 'D', 40, 0, 10, 10, 0, RPE_LeftClickButton);
        regNameSet(d, "Region D");
        regFunctionSet(d, regProcessCallback);
        e = regSiblingAlloc(a, 'E', 12, 0, 6, 10, 0, RPE_LeftClickButton);
        regNameSet(e, "Region E");
        regFunctionSet(e, regProcessCallback);
        f = regChildAlloc(c, 'F', 0, 40, 10, 10, 0, RPE_LeftClickButton);
        regNameSet(f, "Region F");
        regFunctionSet(f, regProcessCallback);
        g = regChildAlloc(c, 'G', 20, 40, 10, 10, 0, RPE_LeftClickButton);
        regNameSet(g, "Region G");
        regFunctionSet(g, regProcessCallback);
//        regRegionDelete(a);
//        regRegionDelete(b);
//        regRegionDelete(e);
//        regRegionDelete(d);
    }
#endif
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : regShutdown
    Description : Shutdown the region processing module.
    Inputs      : void
    Outputs     : Stops the region processor task and frees all memory
                    associated with this module.
    Return      : void
----------------------------------------------------------------------------*/
void regShutdown(void)
{
    regionhandle child, nextRegion;

    regInitCheck();
#if REG_INDEX_ALLOCS
    dbgMessagef("\nregShutdown: %d allocs", regAllocCounter);
#endif
    for (child = regRootRegion.child; child != NULL;)        //delete all children of the root region
    {
        nextRegion = child->next;
        regRegionDelete(child);
        child = nextRegion;
    }
    taskStop(regTaskHandle);                                //stop region processor task
    memFree(regRenderEvent);                                //free render event memory
    regRenderEventIndex = 0;                                //don't try to render any more regions
    regModuleInit = FALSE;                                  //flag module not started
}

/*-----------------------------------------------------------------------------
    Name        : regDrawFunctionAdd
    Description : Add a region draw event.
    Inputs      : function - function to call
                  reg - handle of region to draw
    Outputs     : allocates and fills in an entry in regRenderEvent
    Return      : void
    Notes       : These queued functions will be drawn in order reverse of
                    specification at render time.
                  If there is no space left in the regRenderEvent array,
                    an error is generated if error checking enabled.
----------------------------------------------------------------------------*/
void regDrawFunctionAdd(regiondrawfunction function, regionhandle reg)
{
    regrenderevent *event;

    dbgAssert(function != NULL);
    if (regRenderEventIndex >= regNumberRenderEvents)       //if no render event available
    {
        dbgFatalf(DBG_Loc, "Tried to add render function 0x%x for region 0x%x.  All %d in use", function, reg, regNumberRenderEvents);
    }
    event = &regRenderEvent[regRenderEventIndex];
    event->function = function;
    event->reg = reg;
    regRenderEventIndex++;
}

/*-----------------------------------------------------------------------------
    Name        : regFunctionsDraw
    Description : Render all previously queued render events
    Inputs      : void
    Outputs     : Calls all queued render events in reverse order of which they
                    were created and sets regRenderEventIndex to zero.
    Return      : void
----------------------------------------------------------------------------*/
void regFunctionsDraw(void)
{
    sdword regNumberDraws = regRenderEventIndex;

    for (regNumberDraws--; regNumberDraws >= 0; regNumberDraws--)
    {
        regRenderEvent[regNumberDraws].function(regRenderEvent[regNumberDraws].reg);
    }
    regRegionFrameDrawCount++;
}

/*-----------------------------------------------------------------------------
    Name        : regChildAlloc
    Description : Create a region as a child of the specified region.
    Inputs      : parent - regionhandle of the parent of the region to be created
                  ID - user-defined ID for region to be passed event process function
                  x, y, width, height - location and size of region
                  extra - amount of extra RAM to allocate with the region structure.
                  flags - control flags for new region
    Outputs     : Allocates memory for the structure and inserts into linked list.
    Return      : pointer to newly allocated region.
----------------------------------------------------------------------------*/
regionhandle regChildAlloc(regionhandle parent, sdword ID, sdword x, sdword y, sdword width, sdword height, sdword extra, udword flags)
{
    regionhandle newRegion;

    regInitCheck();
#if REG_INDEX_ALLOCS
    {
        char string[32];
        sprintf(string, "Region%d", regAllocCounter);
        regAllocCounter++;
        newRegion = memAlloc(sizeof(region) + extra, string, NonVolatile);//allocate memory for new region
    }
#else
    newRegion = memAlloc(sizeof(region) + extra, "Region", NonVolatile);//allocate memory for new region
#endif
    newRegion->rect.x0 = x;                                 //set new rectangle
    newRegion->rect.y0 = y;
    newRegion->rect.x1 = x + width;
    newRegion->rect.y1 = y + height;
    newRegion->flags = flags;
    newRegion->status = RSF_DrawThisFrame;
    newRegion->drawFunction = regNULLRenderFunction;        //blank processing functions
    newRegion->processFunction = regNULLProcessFunction;
    newRegion->userID = ID;
    newRegion->lastframedrawn = 0;
    newRegion->tabstop = 0;
    newRegion->atom = NULL;
    *((udword *)newRegion->key) = 0;
    newRegion->nKeys = 0;
    listInit(&newRegion->cutouts);
#if REG_ERROR_CHECKING
    newRegion->validationKey = REG_ValidationKey;           //set validation key
#endif
#if REG_VERBOSE_LEVEL >= 1
    dbgMessagef("\nregChildAlloc: added child 0x%x of parent 0x%x at (%d, %d), size %d x %d, flags 0x%x",
                newRegion, parent, x, y, width, height, flags);
#endif
    if (parent == NULL)
    {
        parent = &regRootRegion;
    }
    newRegion->child = NULL;
    regChildInsert(newRegion, parent);                      //insert region into linked list
    return(newRegion);                                      //return newly allocated region
}

/*-----------------------------------------------------------------------------
    Name        : regSiblingAlloc
    Description : Create a region as a sibling of the specified region.
    Inputs      : sibling - regionhandle of the previous of the region to be created
                  ID - user-defined ID for region to be passed event process function
                  x, y, width, height - location and size of region
                  extra - amount of extra RAM to allocate with the region structure.
                  flags - control flags for new region
    Outputs     : Allocates memory for the structure and inserts into linked list.
    Return      : pointer to newly allocated region.
----------------------------------------------------------------------------*/
regionhandle regSiblingAlloc(regionhandle sibling, sdword ID, sdword x, sdword y, sdword width, sdword height, sdword extra, udword flags)
{
    regionhandle newRegion;

    regInitCheck();
#if REG_INDEX_ALLOCS
    {
        char string[32];
        sprintf(string, "Region%d", regAllocCounter);
        regAllocCounter++;
        newRegion = memAlloc(sizeof(region) + extra, string, NonVolatile);//allocate memory for new region
    }
#else
    newRegion = memAlloc(sizeof(region) + extra, "Region", NonVolatile);//allocate memory for new region
#endif
    newRegion->rect.x0 = x;                                 //set new rectangle
    newRegion->rect.y0 = y;
    newRegion->rect.x1 = x + width;
    newRegion->rect.y1 = y + height;
    newRegion->flags = flags;
    newRegion->status = 0;
    newRegion->drawFunction = regNULLRenderFunction;        //blank processing functions
    newRegion->processFunction = regNULLProcessFunction;
    newRegion->userID = ID;
    /**((udword *)newRegion->key) = 0;*/
    memset(newRegion->key, 0, sizeof(keyindex) * REG_NumberKeys);
    newRegion->nKeys = 0;
    listInit(&newRegion->cutouts);
#if REG_ERROR_CHECKING
    newRegion->validationKey = REG_ValidationKey;           //set validation key
#endif
#if REG_VERBOSE_LEVEL >= 1
    dbgMessagef("\nregSiblingAlloc: added sibling 0x%x to 0x%x at (%d, %d) size %d x %d, flags 0x%x",
                newRegion, sibling, x, y, width, height, flags);
#endif
    dbgAssert(sibling != NULL);
    regSiblingInsert(newRegion, sibling);                   //insert region into linked list
    return(newRegion);                                      //return newly allocated region
}

/*-----------------------------------------------------------------------------
    Name        : regSiblingMoveToFront
    Description : Adjust links so region is first in it's sibling list
    Inputs      : region - handle of region to adjust
    Outputs     : ..
    Return      : The displaced region which was at the head of the list.
----------------------------------------------------------------------------*/
regionhandle regSiblingMoveToFront(regionhandle region)
{
    regionhandle sibling, parent;

    regVerify(region);
    parent = region->parent;
    sibling = parent->child;
    if (sibling == region)
    {                                                       //if already at front
        return(region);                                     //do nothing
    }
    dbgAssert(sibling->previous == NULL);
    dbgAssert(region->parent == sibling->parent);           //verify thay have the same parent
    //find what sibling to put it in front of
    if (!bitTest(region->status, RSF_PriorityRegion))
    {                                                       //if this is not a priority region
        while (sibling->next != NULL && bitTest(sibling->status, RSF_PriorityRegion))
        {                                                   //find first low-priority region
            sibling = sibling->next;
        }
    }
    //remove node from sibling list
    if (region->next != NULL)                               //remove region from list
    {
        (region->next)->previous = region->previous;
    }
    if (region->previous != NULL)
    {
        (region->previous)->next = region->next;
    }
    //insert the node back into list
    if (parent->child == sibling)
    {                                                       //if inserting at head of list
        parent->child = region;
        sibling->previous = region;
        region->previous = NULL;
        region->next = sibling;
    }
    else
    {                                                       //else inserting at some arbitrary point in the list
        region->previous = sibling->previous;
        dbgAssert(region->previous != NULL);                //make sure not head of list
        (region->previous)->next = region;
        region->next = sibling;
        sibling->previous = region;
    }
    return(sibling);
}

/*-----------------------------------------------------------------------------
    Name        : regKeyChildAlloc/regKeySiblingAlloc
    Description : Create a key-only region.
    Inputs      : parent/sibling - region to link to
                  ID - user-defined ID to pass to processor functions
                  filter - event filter/flags
                  function - region processor draw function
                  nKeys - number of keys for this region
                  ... - varaible number of keys.  They are of keyindex type.
    Outputs     : Allocates and initializes a new region structure.
    Return      : Handle of new region.
    Notes       : Filter flags other than key events can be passed in, however
        mouse events will never be genterated because the region has zero size.
----------------------------------------------------------------------------*/
regionhandle regKeyChildAlloc(regionhandle parent, sdword ID, udword filter, regionfunction function, sdword nKeys, ...)
{
    regionhandle newRegion;
    sdword index;
    va_list argPointer;

#if REG_VERBOSE_LEVEL >= 1
    dbgMessagef("\nregKeyChildAlloc: adding child to 0x%x, flags 0x%x, function 0x%x",
                parent, filter, function);
#endif
    newRegion = regChildAlloc(parent, ID, 0, 0, 0, 0, 0, filter);
    regFunctionSet(newRegion, function);

    dbgAssert(nKeys <= REG_NumberKeys);                     //verify validity of key info

    newRegion->nKeys = (sword)nKeys;                        //store number of keys
    va_start(argPointer, nKeys);
    for (index = 0; index < nKeys; index++)
    {                                                       //store each key
        newRegion->key[index] = va_arg(argPointer, keyindex);
    }
    va_end(argPointer);
    return(newRegion);
}
regionhandle regKeySiblingAlloc(regionhandle sibling, sdword ID, udword filter, regionfunction function, sdword nKeys, ...)
{
    regionhandle newRegion;
    sdword index;
    va_list argPointer;

#if REG_VERBOSE_LEVEL >= 1
    dbgMessagef("\nregKeySiblingAlloc: adding sibling 0x%x to 0x%x, flags 0x%x, function 0x%x",
                sibling, filter, function);
#endif
    newRegion = regSiblingAlloc(sibling, ID, 0, 0, 0, 0, 0, filter);
    regFunctionSet(newRegion, function);

    dbgAssert(nKeys <= REG_NumberKeys);                     //verify validity of key info

    newRegion->nKeys = (sword)nKeys;                        //store number of keys
    va_start(argPointer, nKeys);
    for (index = 0; index < nKeys; index++)
    {                                                       //store each key
        newRegion->key[index] = va_arg(argPointer, keyindex);
    }
    va_end(argPointer);
    return(newRegion);
}

/*-----------------------------------------------------------------------------
    Name        : regRegionScroll
    Description : Relative scroll of all regions children of the one listed.
    Inputs      : reg - region to scroll
                  scrollX, scrollY - amount to add to the region rectangles
    Outputs     : recursively scrolls all subchildren
    Return      : void
----------------------------------------------------------------------------*/
void regRegionScroll(regionhandle reg, sdword scrollX, sdword scrollY)
{
    regionhandle child;

    regVerify(reg);
    for (child = reg->child; child != NULL; child = child->next)//process all children first
    {
        regRegionScroll(child, scrollX, scrollY);             //scroll all the children first
    }

#ifdef DEBUG_STOMP
    regVerify(reg);
#endif
    bitSet(reg->status, RSF_DrawThisFrame);

    reg->rect.x0 += scrollX;                                //scroll this region
    reg->rect.x1 += scrollX;
    reg->rect.y0 += scrollY;
    reg->rect.y1 += scrollY;
}

/*-----------------------------------------------------------------------------
    Name        : regLinkRemove
    Description : Remove the parent and sibling links of a region.
    Inputs      : region - region whose parent links we are to delete
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void regLinkRemove(regionhandle region)
{
    if (region->parent != NULL)
    {
        if ((region->parent)->child == region)              //if this is first sibling
        {
            if (region->next)                               //if there are more siblings
            {
                (region->parent)->child = region->next;     //next sibling is first sibling
            }
            else
            {
                (region->parent)->child = NULL;            //only child, clear parent's reference
            }
        }
    }
    if (region->previous != NULL)
    {
        (region->previous)->next = region->next;            //clear previous sibling's reference
    }
    if (region->next != NULL)
    {
        (region->next)->previous = region->previous;        //clear next sibling's reference
    }
    region->previous = region->next = region->parent = NULL;
    if (regClickedLeft == region)
    {
        regClickedLeft = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : regMoveLinkChild
    Description : Alter the links of a region to reflect a new parent.
    Inputs      : regionToMove - region to move.  Can be NULL for no parent.
                  newParent - new parent.  If NULL, there is no new parent and
                  this region will not be implicitly processed.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void regMoveLinkChild(regionhandle regionToMove, regionhandle newParent)
{
    if (regionToMove->parent != NULL)
    {
        regLinkRemove(regionToMove);                        //remove old links
    }
    if (newParent != NULL)
    {
        regChildInsert(regionToMove, newParent);            //create new links
    }
    if (regClickedLeft == regionToMove)
    {
        regClickedLeft = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : regRecursiveDisable
    Description : Disbales a region and all it's children
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void regRecursiveDisable(regionhandle reg)
{
    regionhandle child;

    bitClear(reg->flags, RPM_AllEvents);                    //disable all events for this region
    for (child = reg->child; child != NULL; child = child->next)
    {
        regRecursiveDisable(child);
    }
}

/*-----------------------------------------------------------------------------
    Name        : regRegionDelete
    Description : Delete a region and all it's child regions.
    Inputs      : region - region who's subtree is to be deleted
    Outputs     : Fixes up all dangling links from parents and siblings.
    Return      : void
    Note        : This function is recursive and will call itself for all
                    child regions.
----------------------------------------------------------------------------*/
void regRegionDelete(regionhandle region)
{
    regionhandle child, nextChild;
    sdword index;

    regInitCheck();
    regVerify(region);
    listDeleteAll(&region->cutouts);

    //dirty regions below us if we need to
    if (feShouldSaveMouseCursor())
    {
        regDirtyEverythingUpwardsSelectively(region, region);
    }

    if (bitTest(region->status, RSF_Processing))            //if processing this region
    {
        bitSet(region->status, RSF_ToBeDeleted);            //flag for later deletion
        if (bitTest(region->flags, RPM_KeyEvent))
        {
            regKeysFocussed = FALSE;
        }
        for (child = region->child; child != NULL; child = nextChild)
        {                                                   //disable but don't delete all children
            nextChild = child->next;
            regRecursiveDisable(region);
        }
        return;                                             //do no more
    }
    if (region->child)                                      //if there are any children
    {
        for (child = region->child; child != NULL; child = nextChild)
        {
            nextChild = child->next;
            regRegionDelete(child);                         //delete all child nodes
        }
    }
#if REG_VERBOSE_LEVEL >= 1
    dbgMessagef("\nregRegionDelete: deleting region 0x%x", region);
#endif
    regLinkRemove(region);

    if (regClickedLeft == region)
    {
        regClickedLeft = NULL;
    }
    //delete all references to region from render list

    for (index = 0; index < regRenderEventIndex; index++)
    {
        if (region == regRenderEvent[index].reg)
        {
            regRenderEvent[index].function = regNULLRenderFunction;
        }
    }
    memFree((void *)region);                                //free memory associated with this region
}

/*-----------------------------------------------------------------------------
    Name        : regChildInsert
    Description : Insert a child of the specified region.
    Inputs      : regionToInsert - handle of the region to be inserted into list.
                  parent - handle of parent to make new child of
    Outputs     : potentially sets link pointers for parent, regionToInsert and
                    last child of parent.
    Return      : void
    Notes       : Inserts after last child of parent.  If parent has no children,
                    the new node is inserted as only child.  The child pointer of
                    inserted string is not modified, although the other links are,
----------------------------------------------------------------------------*/
void regChildInsert(regionhandle regionToInsert, regionhandle parent)
{
    regionhandle sibling;

    regInitCheck();
    dbgAssert(parent != NULL);                              //ensure valid nodes
    regVerify(regionToInsert);
    regVerify(parent);

    regionToInsert->parent = //regionToInsert->child =        //start by clearing all link pointers to NULL
        regionToInsert->previous = regionToInsert->next = NULL;
    if (parent->child == NULL)                              //if parent has no children
    {
        parent->child = regionToInsert;
    }
    else
    {
        for (sibling = parent->child; sibling->next != NULL; )
        {                                                   //scan for last sibling
            sibling = sibling->next;
        }
        sibling->next = regionToInsert;                     //create sibling links
        regionToInsert->previous = sibling;
    }
    regionToInsert->parent = parent;
}

/*-----------------------------------------------------------------------------
    Name        : regSiblingInsert
    Description : Insert a sibling of the specified region.
    Inputs      : regionToInsert - handle of the region to be inserted into list.
                  sibling - handle of parent to make new sibling of
    Outputs     : Sets link pointers for regionToInsert and sibling.
    Return      : void
    Notes       : If sibling specifed is not last sibling in list, new node will
                    be inserted in between sibling region and it's next sibling.
----------------------------------------------------------------------------*/
void regSiblingInsert(regionhandle regionToInsert, regionhandle sibling)
{
    regInitCheck();
    dbgAssert(sibling != NULL);                             //ensure valid nodes
    regVerify(regionToInsert);
    regVerify(sibling);

    regionToInsert->parent = regionToInsert->child =        //start by clearing all link pointers to NULL
        regionToInsert->previous = regionToInsert->next = NULL;

    if (sibling->next != NULL)                              //if not last node in list
    {
        (sibling->next)->previous = regionToInsert;
        regionToInsert->next = sibling->next;
    }

    sibling->next = regionToInsert;                         //set sibling to point to this node
    regionToInsert->previous = sibling;                     //this node points at sibling node
    regionToInsert->parent = sibling->parent;               //duplicate parent link
}

/*-----------------------------------------------------------------------------
    Name        : regDrawFunctionSet
    Description : Change the draw function for a region
    Inputs      : region - regions whose draw function is to be changed
                  function - new draw function
    Outputs     : sets region->drawfunction to function or regNULLRenderFunction if NULL
    Return      : provious draw function
----------------------------------------------------------------------------*/
regiondrawfunction regDrawFunctionSet(regionhandle region, regiondrawfunction function)
{
    regiondrawfunction old;

    regVerify(region);
    old = region->drawFunction;
    if (function == NULL)
    {
        region->drawFunction = regNULLRenderFunction;
    }
    else
    {
        region->drawFunction = function;
    }
    return(old);
}

/*-----------------------------------------------------------------------------
    Name        : regFunctionSet
    Description : Change the process function for a region
    Inputs      : region - regions whose process function is to be changed
                  function - new process function
    Outputs     : guess
    Return      : provious process function
----------------------------------------------------------------------------*/
regionfunction regFunctionSet(regionhandle region, regionfunction function)
{
    regionfunction old;

    regVerify(region);
    dbgAssert(function != NULL);
    old = region->processFunction;
    region->processFunction = function;
    return(old);
}

/*-----------------------------------------------------------------------------
    Name        : regKeysSet
    Description : Sets the key(s) for a particular region.
    Inputs      : region - region whose keys we are modifying
                  nKeys - number of keys for this region
                  ... - varaible number of keys.  They are of keyindex type.
    Outputs     : Sets the key sequence for specified region.
    Return      : void
    Notes       : Region needs to have some key message filter flags enabled.
----------------------------------------------------------------------------*/
void regKeysSet(regionhandle region, sdword nKeys, ...)
{
    sdword index;
    va_list argPointer;

    dbgAssert(nKeys <= REG_NumberKeys);                     //verify validity
    dbgAssert(region != NULL);
    regVerify(region);

    region->nKeys = (sword)nKeys;                           //store number of keys
    va_start(argPointer, nKeys);
    for (index = 0; index < nKeys; index++)
    {
        region->key[index] = tolower(va_arg(argPointer, keyindex));
    }
    va_end(argPointer);
}

/*-----------------------------------------------------------------------------
    Name        : regFilterSet
    Description : Adjust the filter for a specified region
    Inputs      : region - region to adjust
                  filter - filter to set in region
    Outputs     : sets the event filter amd flags for specified region
    Return      : Previous filter
----------------------------------------------------------------------------*/
udword regFilterSet(regionhandle region, udword filter)
{
    udword old;

    old = region->flags;
    region->flags = filter;
    return(old);
}

/*-----------------------------------------------------------------------------
    Name        : regTabstopSet
    Description : Set the Tabstop for a specified region
    Inputs      : region - region to adjust
                  tabstop - tabstop to set in region
    Outputs     : sets the tabstop for specified region
    Return      : Previous tabstop
----------------------------------------------------------------------------*/
udword regTabstopSet(regionhandle region, udword tabstop)
{
    udword old;

    old = region->tabstop;
    region->tabstop = tabstop;
    return(old);
}

/*-----------------------------------------------------------------------------
    Name        : regFocusLost
    Description : Called when the mouse control has been relinquished to someone
                    else, such as during an alt-tab.
    Inputs      : region - the region to clear status bit on.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
void regFocusLost(regionhandle reg)
{
    regionhandle child, nextChild;

    regVerify(reg);
    for (child = reg->child; child != NULL; child = child->next)//process all children first
    {
        regVerify(child);
        regFocusLost(child);
    }
    reg->status &= ~(RSF_MouseInside | RSF_LeftPressed | RSF_RightPressed | RSF_CentrePressed | RSF_KeyPressed | RSF_CurrentSelected);
}
*/

/*-----------------------------------------------------------------------------
    Name        : regSiblingFindByFunction
    Description : Find a sibling of a particular region by it's process function.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
regionhandle regSiblingFindByFunction(regionhandle reg, regionfunction find)
{
    regionhandle sibling;

    sibling = reg->parent->child;

    while (sibling != NULL)
    {
        regVerify(sibling);
        if (sibling->processFunction == find)
        {
            return(sibling);
        }
        sibling = sibling->next;
    }
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : regFindChildByAtomName
    Description : Find a region by the name of its atom
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
regionhandle regFindChildByAtomName(regionhandle reg, char *pAtomName)
{
regionhandle child;
regionhandle Result = NULL;
featom  *pAtom;

    child = reg->child;

    while(child)
    {
        if(child->atom)
        {
            pAtom = (featom *)child->atom;
            if(pAtom->name && (strcasecmp(pAtom->name, pAtomName) == 0))
                return child;
        }

        if(child->child)
        {
            Result = regFindChildByAtomName(child, pAtomName);
            if(Result)
                return Result;
        }

        child = child->next;
    }

    return(NULL);
}
