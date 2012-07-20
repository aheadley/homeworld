/*=============================================================================
    Name    : region.h
    Purpose : Process the creation, deletion, processing and drawing of all
        region-based UI elements including buttons and menus.

    Created 6/25/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

/*=============================================================================
    Process & draw callback prototypes
=============================================================================*/
#if 0
/*-----------------------------------------------------------------------------
    Name        : XXX
    Description : Render callback for YYY
    Inputs      : region - region we're rendering
    Outputs     : ...user defined...
    Return      : void
----------------------------------------------------------------------------*/
void XXX(regionhandle region)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : XXX
    Description : Region processor callback for YYY
    Inputs      : region - region to handle processing
                  ID - user-assigned ID set when region created
                  event - enumeration of event to be processed
                  data - additional event data (message specific)
    Outputs     : ...user defined...
    Return      : flags indicating further operation:

----------------------------------------------------------------------------*/
udword XXX(regionhandle region, sdword ID, udword event, udword data)
{
    return(0);
}

#endif //0

#ifndef ___REGION_H
#define ___REGION_H

#include "Types.h"
#include "Memory.h"
#include "Key.h"
#include "Debug.h"
#include "prim2d.h"
#include "LinkedList.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define REG_INDEX_ALLOCS            0           //a bug-hunting feature

#ifndef HW_Release

#define REG_ERROR_CHECKING          1           //general error checking
#define REG_VERBOSE_LEVEL           0           //control specific output code
#define REG_TEST                    0           //test of region code
#define REG_DRAW_REGION_BORDERS     0           //draw a recrangle for debugging

#else //HW_Debug

#define REG_ERROR_CHECKING          0           //general error checking
#define REG_VERBOSE_LEVEL           0           //control specific output code
#define REG_TEST                    0           //test of region code
#define REG_DRAW_REGION_BORDERS     0           //draw a recrangle for debugging

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//region status flags
#define RSF_ReallyDirty             0x0001      //Draw the region and all its children guaranteed
#define RSF_CantFocusTo             0x0002      //Can't focus to this region ever
#define RSF_RegionDisabled          0x0004      //region is disabled, don't pass any info to it
#define RSF_DrawThisFrame           0x0008      //draw on this frame
#define RSF_MouseCapture            0x0010      //region has mouse input focus
#define RSF_KeyCapture              0x0020      //region has keyboard input focus
                                                //and will be notified of all key events
#define RSF_ToBeDeleted             0x0040      //region has been deleted
#define RSF_Processing              0x0080      //this region currently in processing
#define RSF_PriorityRegion          0x0100      //this region will be on top of sibling regions
#define RSF_CurrentSelected         0x0200      //the region has input focus
#define RSF_ScenarioList            0x0400      //the region is a scenario list
#define RSF_MouseInside             0x0800      //mouse inside region
#define RSF_LeftPressed             0x1000      //centre mouse pressed state
#define RSF_RightPressed            0x2000      //right mouse pressed state
#define RSF_CentrePressed           0x4000      //left mouse pressed state
#define RSF_KeyPressed              0x8000      //current key state for this region

//region processor function event messages:
//pass these as a filter on creation
#define RPE_Enter                   0x00000001  //entering region
#define RPE_Exit                    0x00000002  //exiting region
#define RPE_EnterHoldLeft           0x00000004  //entering region with a button pressed
#define RPE_EnterHoldRight          0x00000008  //
#define RPE_EnterHoldCentre         0x00000010  //
#define RPE_ExitHoldLeft            0x00000020  //exiting region with a button pressed
#define RPE_ExitHoldRight           0x00000040  //
#define RPE_ExitHoldCentre          0x00000080  //
#define RPE_WheelUp                 0x00000100  //mouse wheel motion
#define RPE_WheelDown               0x00000200  //
#define RPE_HoldLeft                0x00000400  //button held
#define RPE_HoldRight               0x00000800  //
#define RPE_HoldCentre              0x00001000  //
#define RPE_PressLeft               0x00002000  //button pressed
#define RPE_PressRight              0x00004000  //
#define RPE_PressCentre             0x00008000  //
#define RPE_ReleaseLeft             0x00010000  //mouse button released
#define RPE_ReleaseRight            0x00020000  //
#define RPE_ReleaseCentre           0x00040000  //
#define RPM_PressRelease            0x0007e000  //mask for mouse press/release
#define RPM_MouseEvents             0x0007ffff  //mask for all mouse events
#define RPE_KeyDown                 0x00080000  //key pressed
#define RPE_KeyUp                   0x00100000  //key released
#define RPE_KeyRepeat               0x00200000  //
#define RPE_KeyHold                 0x00400000  //
#define RPM_KeyEvent                0x00780000  //mask for all key events
//#define RPE_MouseMoveRel            0x00800000  //
//#define RPE_MouseMoveAbs            0x01000000  //
#define RPE_DoubleLeft              0x00800000  //double-click left button
#define RPE_DoubleRight             0x01000000  //double-click right button
#define RPE_DoubleCentre            0x02000000  //bouble-click centre button
#define RPM_AllEvents               0x01ffffff  // mask for all events

#define RPE_DrawFunctionAdded       0x10000000
#define RPE_ModalBreak              0x40000000  //process no parents of this region
#define RPE_DrawEveryFrame          0x80000000  //draw this region every frame
//composite event filters for common region types
#define RPE_LeftClick               (RPE_PressLeft | RPE_ReleaseLeft)
#define RPE_RightClick              (RPE_PressRight | RPE_ReleaseRight)
#define RPE_LeftClickButton         (RPE_LeftClick | RPE_EnterHoldLeft | RPE_ExitHoldLeft)
#define RPE_RightClickButton        (RPE_RightClick | RPE_EnterHoldRight | RPE_ExitHoldRight)

//region process function return value flags
#define RPR_Redraw                  0x00002000  //redraw later this frame
#define RPR_Continue                0x00400000  //continue to process parent nodes

//generic definitions
#define REG_NumberKeys              4           //max number of keys in validation sequence
#define REG_ValidationKey           0xf1ab4a55
#define REG_RenderEventsDefault     512         //default number of loggable render events
#define REG_TaskFrequency           16          //process regions every update frame
#define REG_TaskFrequencyOPF        1           //process regions every rendering frame
#define REG_TaskStackSize           100000      //size of region processing task stack
#define REG_OutlineColor            colRGB(10, 10, 30)//color of outlines

#define RF_ShiftBit                 BIT10       //shift status in certain functions

/*=============================================================================
    Type definitions:
=============================================================================*/

struct tagRegion;

//pointers to region functions
typedef void (*regiondrawfunction) (struct tagRegion *reg);
typedef udword (*regionfunction) (struct tagRegion *reg, sdword ID, udword event, udword data);

//structure for a region, the base element of the front end
typedef struct tagRegion
{
    rectangle rect;                             //rectangle defining limits of region
    regiondrawfunction drawFunction;            //render function for region
    regionfunction processFunction;             //logic processing function for region
    struct tagRegion *parent, *child;           //region list links
    struct tagRegion *previous, *next;
    udword flags;                               //region control flags (see above)
    uword status;                               //status bits (32 bits not enough)
    sword nKeys;
    keyindex key[REG_NumberKeys];               //accelerator key sequence
    sdword userID;                              //user-assigned ID for processing
#if REG_ERROR_CHECKING
    udword validationKey;                       //used for validation of structure integrity
#endif
    udword tabstop;                             //tabstop for key navigation
    LinkedList cutouts;
    udword drawstyle[2];
    udword lastframedrawn;
    void *atom;
}
region;

//'handle' to a region; simply a pointer to the region
typedef region *regionhandle;

//data structure for a render event
typedef struct tagRegrenderevent
{
    regiondrawfunction function;
    regionhandle reg;
}
regrenderevent;

extern udword regRegionFrameDrawCount;

/*=============================================================================
    Data:
=============================================================================*/
extern region regRootRegion;
extern bool regKeysFocussed;

/*=============================================================================
    Macros:
=============================================================================*/

//macro to verify that the specified region is valid
#if REG_ERROR_CHECKING
#define regVerify(r)                                    \
if ((r)->validationKey != REG_ValidationKey)            \
{                                                       \
    dbgFatalf(DBG_Loc, "regVerify: invalid region 0x%x has validation key 0x%x", (r), (r)->validationKey);\
}
#else
#define regVerify(r)
#endif

//set the name of a region for debugging
#if MEM_USE_NAMES
#define regNameSet(r, n) memRename((void *)(r), (n))
#else
#define regNameSet(r, n)
#endif

//verify module started/not started
#if REG_ERROR_CHECKING
#define regInitCheck()                                  \
if (regModuleInit == FALSE)                             \
{                                                       \
    dbgFatal(DBG_Loc, "Module not initialized.");       \
}
#define regNotInitCheck()                               \
if (regModuleInit != FALSE)                             \
{                                                       \
    dbgFatal(DBG_Loc, "Module not initialized.");       \
}
#else
#define regInitCheck()
#define regNotInitCheck()
#endif

//bit field testing macros
#define regInside(r)            ((r)->status & RSF_MouseInside)
#define regLeftPressed(r)       ((r)->status & RSF_LeftPressed)
#define regRightPressed(r)      ((r)->status & RSF_RightPressed)
#define regCentrePressed(r)     ((r)->status & RSF_CentrePressed)
#define regKeyPressed(r)        ((r)->status & RSF_KeyPressed)
#define regFlag(flag)           (((mask & reg->flags) & (flag)) == (flag))
#define regStatus(stat)         (((reg)->status & (stat)) == (stat))

//task handles for pausing etc.
extern taskhandle regTaskHandle;
extern taskhandle regDrawTaskHandle;

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown region module
sdword regStartup(void);
void regShutdown(void);

//insert regions as either a child or a sibling of another region
regionhandle regChildAlloc(regionhandle parent, sdword ID, sdword x, sdword y, sdword width, sdword height, sdword extra, udword filter);
regionhandle regSiblingAlloc(regionhandle sibling, sdword ID, sdword x, sdword y, sdword width, sdword height, sdword extra, udword filter);
regionhandle regKeyChildAlloc(regionhandle parent, sdword ID, udword filter, regionfunction function, sdword nKeys, ...);
regionhandle regKeySiblingAlloc(regionhandle sibling, sdword ID, udword filter, regionfunction function, sdword nKeys, ...);
void regRegionDelete(regionhandle region);

//adjust region links (usually only used by region allocation/deletion functions)
bool regRegionInside(regionhandle needle, regionhandle haystack);
void regChildInsert(regionhandle regionToInsert, regionhandle parent);
void regSiblingInsert(regionhandle regionToInsert, regionhandle sibling);
void regMoveLinkChild(regionhandle regionToMove, regionhandle newParent);
void regMoveLinkSibling(regionhandle regionToMove, regionhandle newSibling);
void regLinkRemove(regionhandle region);
regionhandle regSiblingMoveToFront(regionhandle region);

//delete a node, freeing memory associated with it
void regRegionDelete(regionhandle region);

//set various attributes of regions
regiondrawfunction regDrawFunctionSet(regionhandle region, regiondrawfunction function);
regionfunction regFunctionSet(regionhandle region, regionfunction function);
udword regFilterSet(regionhandle region, udword filter);
void regKeysSet(regionhandle region, sdword nKeys, ...);
udword regTabstopSet(regionhandle region, udword tabstop);
void regDirtyEverythingUpwardsSelectively(regionhandle region, regionhandle overlap);
void regDirtyEverythingUpwards(regionhandle region);
void regDirtyChildren(regionhandle reg);
void regRecursiveSetDirty(regionhandle reg);
void regRecursiveSetReallyDirty(regionhandle reg);
void regDirtyScreensAboveRegion(regionhandle region);

//void regFocusLost(regionhandle region);

//get attributes of regions
#define regFilterGet(r)    ((r)->flags)
regiondrawfunction regDrawFunctionGet(regionhandle region);
regionfunction regFunctionGet(regionhandle region);

//add render events and render them all
void regDrawFunctionAdd(regiondrawfunction function, regionhandle reg);
//only add a draw function if it hasn't already been added
void regDrawFunctionAddPossibly(regionhandle region);
void regFunctionsDraw(void);

//move regions about
void regRegionScroll(regionhandle reg, sdword scrollX, sdword scrollY);

//misc utility functions
regionhandle regSiblingFindByFunction(regionhandle reg, regionfunction find);
regionhandle regFindChildByAtomName(regionhandle reg, char *pAtomName);

//process null poo
udword regNULLProcessFunction(regionhandle region, sdword ID, udword event, udword data);


#endif //___REGION_H


