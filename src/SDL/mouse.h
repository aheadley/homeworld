/*=============================================================================
    Name    : mouse.h
    Purpose : Hardware abstraction for the mouse.

    Created 6/26/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MOUSE_H
#define ___MOUSE_H

#include "Types.h"
#include "prim2d.h"
#include "SpaceObj.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define MOUSE_ERROR_CHECKING        1           //general error checking
#define MOUSE_CURSOR_CLAMP          1           //clamp location of cursor to bounds of the window

#else //HW_Debug

#define MOUSE_ERROR_CHECKING        0           //general error checking
#define MOUSE_CURSOR_CLAMP          1           //clamp location of cursor to bounds of the window

#endif //HW_Release

/*=============================================================================
    Definitions:
=============================================================================*/
#define MB_Left             0x0001
#define MB_Right            0x0002
#define MB_Centre           0x0004

//mouse cursor special modes
#define MCM_Pointmode      0x0001
#define MCM_SingleClick    0x0010
#define MCM_DoubleClick    0x0020
#define MCM_CursorOverObj   0x0040

//mouse cursorover types
#define MC_ENEMY_SHIP   -1
#define MC_RESOURCE     -2
#define MC_NO_SHIP      -3
#define MC_ALLIED_SHIP  -4
#define MC_DERELICT     -5

//mouse cursor hold time (in seconds)
#define MC_MOUSEHOLDTIME 1.0

//cursor colors
#define MC_ContentColor     colRGB(32, 176, 12)
#define MC_EdgeColor        colRGB(200, 200, 200)

//cursor bitmap filenames
#define MC_PATH                     "CursorBitmaps//"
#define MC_NORMAL_BM                MC_PATH"cursor.LiF"
#define MC_ADD_SHIPS_BM             MC_PATH"add.LiF"
#define MC_FOCUS_NO_SELECT_BM       MC_PATH"eye.LiF"
#define MC_BAND_BOX_ATTACK_BM       MC_PATH"attack.LiF"
#define MC_FORCE_ATTACK_BM          MC_PATH"exclaim.LiF"
#define MC_GUARD_BM                 MC_PATH"shield.LiF"
#define MC_SPECIAL_ATTACK_BM        MC_PATH"special_activate.LiF"
#define MC_SPECIAL_OP_ACTIVATE_BM   MC_PATH"special_activate.LiF"
#define MC_SPECIAL_OP_DEACTIVATE_BM MC_PATH"special_deactivate.LiF"
#define MC_GUI_BM                   MC_PATH"dialog.LiF"
#define MC_RESOURCE_BM              MC_PATH"resource.LiF"
#define MC_DOCKING_BM               MC_PATH"dock.LiF"
#define MC_RESEARCH_BM              MC_PATH"dialog.LiF"
#define MC_SENSORS_BM               MC_PATH"dialog.LiF"
#define MC_BUILD_BM                 MC_PATH"dialog.LiF"
#define MC_SALVAGE_BM               MC_PATH"special_activate.LiF"
#define MC_SUPPORT_BM               MC_PATH"support.LiF"
#define MC_TRADERS_BM               MC_PATH"trader.LiF"

#define MOUSE_IdentifyLODMask       0x3       //what LOD you have to see the ship at in order to identify it

/*=============================================================================
    Data:
=============================================================================*/
extern sdword mouseCursorXPosition, mouseCursorYPosition;
extern sdword lastUnderWidth, lastUnderHeight;
extern uword mouseButtons;
extern bool8 mouseIsVisible;
extern uword mouseWillBeVisible;
extern bool8 mouseDisabled;
extern SpaceObj *mouseCursorObjPtr;
extern SpaceObj *mouseCursorObjLast;
extern MaxSelection mouseCursorSelect;
extern real32 mouseCursorLastObjTime;
extern sdword mouseCursorLastX;
extern sdword mouseCursorLastY;

/*=============================================================================
    Macros:
=============================================================================*/
//enumerated type for different mouse types
typedef enum
{
    no_mouse = -1,
    normal_mouse,
    rotate_camera,
    zoom_camera,
    add_ships,
    focus_no_select,
    movement,
    band_box_attack,
    force_attack,
    m_guard,
    special_attack,
    special_op_activate,
    special_op_deactivate,
    gui,
    resource,
    derelict,
    docking,
   small_docking,
    research,
    sensors,
    build,
    salvage,
    support,
    traders,
   enemy_ship,
    allied_ship,
    own_ship,
    end_cursor_type
} mouseCursor;


//verify module started/not started
#if MOUSE_ERROR_CHECKING
#define mouseInitCheck()                                \
if (mouseModuleInit == FALSE)                           \
{                                                       \
    dbgFatal(DBG_Loc, "Module not initialized.");       \
}
#define mouseNotInitCheck()                             \
if (mouseModuleInit != FALSE)                           \
{                                                       \
    dbgFatal(DBG_Loc, "Module not initialized.");       \
}
#else
#define mouseInitCheck()
#define mouseNotInitCheck()
#endif //MOUSE_ERROR_CHECKING

#define mouseCursorX()          (mouseCursorXPosition)
#define mouseCursorY()          (mouseCursorYPosition)
#define mouseLeftButton()       (keyIsHit(LMOUSE_BUTTON))   //(bitTest(mouseButtons, MB_Left))
#define mouseRightButton()      (keyIsHit(RMOUSE_BUTTON))   //(bitTest(mouseButtons, MB_Right))
#define mouseCentreButton()     (0)   //(bitTest(mouseButtons, MB_Centre))
#define mouseLeftDouble()       (keyIsHit(LMOUSE_DOUBLE))
#define mouseRightDouble()       (keyIsHit(RMOUSE_DOUBLE))

#define mouseInRect(r)          (mouseCursorXPosition >= (r)->x0 && \
                                 mouseCursorYPosition >= (r)->y0 && \
                                 mouseCursorXPosition < (r)->x1 &&  \
                                 mouseCursorYPosition < (r)->y1)
#define mouseInRectGeneral(x0, y0, x1, y1)  (mouseCursorXPosition >= (x0) && \
                                             mouseCursorYPosition >= (y0) && \
                                             mouseCursorXPosition < (x1) &&  \
                                             mouseCursorYPosition < (y1))

/*=============================================================================
    Functions:
=============================================================================*/
//start/stop mouse module
sdword mouseStartup(void);
void mouseReset(void);
void mouseShutdown(void);

//hide/show the cursor
void mouseCursorHide(void);
void mouseCursorShow(void);
void mouseCursorDelayShow(uword delay);

//disable/enable cursor
void mouseDisable(void);
void mouseEnable(void);

//click special callbacks
bool mouseLDoubleClick(void);
bool mouseDoubleClickCheck(void);
void mouseLClick(void);
void mouseClickShipDied(ShipPtr deadship);

//set new position for mouse
void mousePositionSet(sdword x, sdword y);
void mouseClipToRect(rectangle *rect);
void mouseClipPointToRect(sdword *x, sdword *y, rectangle *rect);

//draw mouse cursor and update cursor position/button flags
void mouseStoreCursorUnder(void);
void mouseRestoreCursorUnder(void);
void mouseDraw(void);
void mouseDrawType(mouseCursor cursortype);
void mouseSelectCursorSetting();
void mouseSetCursorSetting();
void mouseCursorSet(udword mode);
void mouseCursorClear(udword mode);

//cursor text code
//void mouseCursorText(SpaceObj *cursorobj);
void mouseCursorTextDraw(void);

//poll current location of mouse and store internally
void mousePoll(void);

#endif //___MOUSE_H

