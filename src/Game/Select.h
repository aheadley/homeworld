/*=============================================================================
    Name    : select.h
    Purpose : Logic for selecting ships and groups of ships.

    Created 7/2/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SELECT_H
#define ___SELECT_H

#include "Types.h"
#include "prim2d.h"
#include "Matrix.h"
#include "SpaceObj.h"
#include "CommandLayer.h"
#include "Camera.h"
#include "font.h"

#include "PiePlate.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release
#define SEL_DRAW_BOXES         1

#define SEL_ERROR_CHECKING     1                //general error checking

#else //HW_Debug

#define SEL_ERROR_CHECKING     0                //general error checking

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//smallest box within which mouse movement is a click, not a drag
#define SEL_ClickBoxWidth           6
#define SEL_ClickBoxHeight          6
//amount of selection circle which needs to be inside selection rectangle for it to be selected/deselected
#define SEL_BandBoxInsideIn         0.2f
#define SEL_BandBoxInsideOut        0.25f

#define SEL_ClickMargin             4           //'fuzzy' logic for selection
#define SEL_DragMargin              1           //'fuzzy' logic for drag-selecting

#define SEL_NumberSelections        COMMAND_MAX_SHIPS
#define SEL_NumberHotKeyGroups      10

extern real32 SEL_MinSelectionRadius;
#define SEL_SelectedColor           colRGB(100, 200, 200)
#define SEL_SelectingColor          colRGB(75, 150, 150)
#define SEL_OutlineColor            colRGB(75, 75, 95)
#define SEL_ShipHealthGreen         colRGB(107, 227, 99)
#define SEL_ShipHealthYellow        colRGB(213, 215, 29)
#define SEL_ShipHealthRed           colRGB(215, 29, 29)
#define SEL_ShipHealthDarkGreen     colRGB(107 * 2 / 3, 227 * 2 / 3, 99 * 2 / 3)
#define SEL_ShipHealthDarkYellow    colRGB(213 * 2 / 3, 215 * 2 / 3, 29 * 2 / 3)
#define SEL_ShipHealthDarkRed       colRGB(213 * 2 / 3, 29 * 2 / 3, 29 * 2 / 3)
#define SEL_ShipHealthSolidGreen    colRGB(107 * 2 / 3, 227 * 2 / 3, 99 * 2 / 3)
#define SEL_ShipHealthSolidYellow   colRGB(213 * 2 / 3, 215 * 2 / 3, 29 * 2 / 3)
#define SEL_ShipHealthSolidRed      colRGB(213 * 2 / 3, 29 * 2 / 3, 29 * 2 / 3)
#define SEL_FuelGreen               colRGB(44, 62, 249)
#define SEL_FuelDarkGreen           colRGB(44 * 2 / 3, 62 * 2 / 3, 249 * 2 / 3)
#define SEL_FuelGreenFactor         0.2f
#define SEL_ShipHealthGreenFactor   0.5f
#define SEL_ShipHealthYellowFactor  0.25f
#define SEL_SelectionWidthFactor0   0.7f
#define SEL_NumberLOD               6
#define SEL_AsteroidMoveNeartoSize  300.0f
#define SEL_DepthSelectMultiplier   3.5f

//hot-key group definitions
#define SEL_InvalidHotKey           0xf
#define SEL_HotKeyNumberBits        5
#define SEL_HotKeyNumberMask        0xf
#define SEL_HotKeyGroupsMask        0x7fe0
#define SEL_HotKeyNumberColor       colRGB(250, 250, 250)
#define SEL_NumberMargin            3

#define SEL_ReinforceFactor         215/256

/*=============================================================================
    Data:
=============================================================================*/
//script-adjustable parameters
extern sdword selClickBoxWidth;
extern sdword selClickBoxHeight;
extern real32 selBandBoxInsideIn;
extern real32 selBandBoxInsideOut;

//selection lists
extern MaxSelection selSelected;
extern MaxAnySelection selSelecting;
extern MaxSelection selHotKeyGroup[SEL_NumberHotKeyGroups];

//centre of selection
extern vector selCentrePoint;
extern real32 selAverageSize;

//camera-space location of centre point of ship most recently considered for selection
extern hvector selCameraSpace;

//fonts used for printing group numbers
extern fonthandle selGroupFont0;
extern fonthandle selGroupFont1;
extern fonthandle selGroupFont2;
extern fonthandle selGroupFont3;

//hot-key number stuff
extern color selHotKeyNumberColor;
extern udword selNumberMargin;
extern char *selHotKeyString[10];

#if PIE_MOVE_NEARTO
//movement mechanism 'move near to' stuff
extern SpaceObjRotImpTarg *selClosestTarget;
extern sdword selClosestDistance;
#endif //PIE_MOVE_NEARTO

/*=============================================================================
    Macros:
=============================================================================*/
#define selRectNone()                       selSelecting.numTargets = 0
#define selSelectNone()                     selSelected.numShips = 0;   \
                                            ioUpdateShipTotals()
#define selRectDrag(c, r)                   selRectDragFunction(universe.RenderList.head, (c), (r), selSelecting.TargetPtr, &selSelecting.numTargets,        TRUE, FALSE, FALSE)
#define selRectDragAnywhere(c, r)           selRectDragFunction(universe.SpaceObjList.head, (c), (r), selSelecting.TargetPtr, &selSelecting.numTargets, TRUE, FALSE, FALSE)
#define selRectSelect(c, r)                 selRectDragFunction(universe.RenderList.head, (c), (r), (SpaceObjRotImpTarg **)selSelected.ShipPtr, &selSelected.numShips,        TRUE, FALSE, FALSE);   \
                                            ioUpdateShipTotals()
#define selRectDragAnybody(c, r)            selRectDragFunction(universe.RenderList.head, (c), (r), selSelecting.TargetPtr, &selSelecting.numTargets, FALSE, FALSE, FALSE)
#define selRectDragAnybodyAnywhere(c, r)    selRectDragFunction(universe.SpaceObjList.head, (c), (r), selSelecting.TargetPtr, &selSelecting.numTargets, FALSE, FALSE, FALSE)
#define selRectSelectAnybody(c, r)          selRectDragFunction(universe.RenderList.head, (c), (r), (SpaceObjRotImpTarg **)selSelected.ShipPtr, &selSelected.numShips, FALSE, FALSE, FALSE);   \
                                            ioUpdateShipTotals()
#define selRectDragAnybodyToAttack(c, r)    selRectDragFunction(universe.RenderList.head, (c), (r), selSelecting.TargetPtr, &selSelecting.numTargets, FALSE, FALSE, TRUE)
#define selRectDragAnybodyAnywhereToAttack(c, r) selRectDragFunction(universe.SpaceObjList.head, (c), (r), selSelecting.TargetPtr, &selSelecting.numTargets, FALSE, FALSE, TRUE)
#define selRectSelectAnybodyToAttack(c, r)  selRectDragFunction(universe.RenderList.head, (c), (r), (SpaceObjRotImpTarg **)selSelected.ShipPtr, &selSelected.numShips, FALSE, FALSE, TRUE);   \
                                            ioUpdateShipTotals()
#define selRectDragAnything(c, r)           selRectDragFunction(universe.RenderList.head, (c), (r), selSelecting.TargetPtr, &selSelecting.numTargets, FALSE, TRUE, FALSE)
#define selRectSelectAnything(c, r)         selRectDragFunction(universe.RenderList.head, (c), (r), selSelected.ShipPtr, &selSelected.numShips, FALSE, TRUE, FALSE);   \
                                            ioUpdateShipTotals()
#define selRectDragAnythingToAttack(c, r)   selRectDragFunction(universe.RenderList.head, (c), (r), selSelecting.TargetPtr, &selSelecting.numTargets, FALSE, TRUE, TRUE)
#define selRectSelectAnythingToAttack(c, r) selRectDragFunction(universe.RenderList.head, (c), (r), selSelected.ShipPtr, &selSelected.numShips, FALSE, TRUE, TRUE);   \
                                            ioUpdateShipTotals()
#define selRectSelectAdd(c, r)              selRectDragAddFunction(universe.RenderList.head, (c), (r))
#define selSelectionSize(n)                 (sizeof(SelectCommand) + ((n) - 1) * sizeof(ShipPtr))
#define selSelectionNumber(size)            (((size) - sizeof(sdword)) / sizeof(ShipPtr))
//#define selHotKeyNumber(n)                  ((uword)((n) | SEL_HotKeyBit))
//set/test/clear the active hot key group number.  This number appears on the selection overlay
#define selHotKeyGroupNumberSet(ship, n)    ((ship)->hotKeyGroup = ((ship)->hotKeyGroup & (~SEL_HotKeyNumberMask)) + (n) + 1)
#define selHotKeyGroupNumberTest(ship)      (((ship)->hotKeyGroup - 1) & SEL_HotKeyNumberMask)
#define selHotKeyGroupNumberClear(ship)     ((ship)->hotKeyGroup = (ship)->hotKeyGroup & (~SEL_HotKeyNumberMask))
//set/test/clear bits corresponding to hot-key groups
#define selHotKeySet(ship, n)               bitSet((ship)->hotKeyGroup, (1 << (SEL_HotKeyNumberBits + (n))))
#define selHotKeyTest(ship, n)              bitTest((ship)->hotKeyGroup, (1 << (SEL_HotKeyNumberBits + (n))))
#define selHotKeyClear(ship, n)             bitClear((ship)->hotKeyGroup, (1 << (SEL_HotKeyNumberBits + (n))))
#define selAnyHotKeyTest(ship)              ((ship)->hotKeyGroup & SEL_HotKeyGroupsMask)
#define selHotKeyClearAll(ship)             ((ship)->hotKeyGroup &= SEL_HotKeyNumberMask)

/*=============================================================================
    Functions:
=============================================================================*/
//startup/shutdown
void selStartup(void);
void selShutdown(void);
void selReset(void);

//compute screen size/location of selection circle for selected ship or mission sphere
void selCircleComputeGeneral(hmatrix *modelView, hmatrix *projection, vector *location, real32 radius, real32 *destX, real32 *destY, real32 *destRadius);
void selCircleCompute(hmatrix *modelView, hmatrix *projection, SpaceObjRotImpTarg *target);

//explicit selections
void selSelectionSetSingleShip(Ship *ship);
void selSelectionAddSingleShip(MaxSelection *dest, Ship *ship);
//void selSelectionAddSingleShip(Ship *ship);
void selSelectionRemoveSingleShip(MaxSelection *dest, Ship *ship);
sdword selShipInSelection(ShipPtr *shipList, sdword nShips, ShipPtr ship);
bool selShipsInSelection(MaxSelection *dest, MaxSelection *list);

//selections by mouse dragging
void selRectDragFunction(Node *startNode, Camera *camera, rectangle *rect, SpaceObjRotImpTarg **destList, sdword *destCount, sdword playerSpecific, bool selectAnything, bool bAttack);
void selRectDragAddFunction(Node *startNode, Camera *camera, rectangle *rect);

//selection by mouse click (on release)
Ship *selSelectionClick(Node *listhead, Camera *camera, sdword x, sdword y, bool bIncludeDerelicts, bool bIncludeResources);
SpaceObj *selClickFromArray(SpaceObj **list, sdword length, sdword x, sdword y);

//selections by key

//draw selected lists
void selSelectedDraw(void);
void selSelectingDraw(void);

//commpute centre of selection
vector selCentrePointComputeGeneral(MaxSelection *selection, real32 *average_size);

//update selCentrePoint and selAverageSize
void selCentrePointCompute(void);

//on-screen position of a selection of ships
void selSelectionDimensions(hmatrix *modelView, hmatrix *projection, SelectCommand *selection, real32 *destX, real32 *destY, real32 *destRad);

//selection manipulation
sdword selSelectionCopy(MaxAnySelection *dest, MaxAnySelection *source);
bool selSelectionIsReinforced(MaxAnySelection *dest, MaxAnySelection *source);
sdword selSelectionCopyByClass(MaxSelection *dest, MaxSelection *source, ShipClass classMask);
sdword selSelectionCopyByType(MaxSelection *dest, MaxSelection *source, ShipType typeMask);
sdword selHotKeyNumbersSet(sdword group);
void selHotKeyGroupRemoveReferences(sdword group);
void selHotKeyGroupRemoveReferencesFromAllGroups(void);
sdword selSelectionCompare(MaxAnySelection *s0, MaxAnySelection *s1);
#if SEL_ERROR_CHECKING
void selHotKeyGroupsVerify(void);
#endif
void selSelectHotKeyGroup(MaxSelection *hotkeygroup);

//debug functions
#if SEL_DRAW_BOXES
void selDrawBoxes(SpaceObjRotImpTarg *target);
#endif
#endif //___SELECT_H

