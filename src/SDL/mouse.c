/*=============================================================================
    Name    : mouse.c
    Purpose : Hardware abstraction for the mouse.

    Created 6/26/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "SDL.h"

#include "DDDFrigate.h"
#include "Debug.h"
#include "Dock.h"
#include "font.h"
#include "FontReg.h"
#include "glinc.h"
#include "Globals.h"
#include "GravWellGenerator.h"
#include "main.h"
#include "mainrgn.h"
#include "mouse.h"
#include "NIS.h"
#include "prim2d.h"
#include "Select.h"
#include "Sensors.h"
#include "ShipSelect.h"
#include "SoundEvent.h"
#include "SpaceObj.h"
#include "texreg.h"
#include "Tweak.h"
#include "Types.h"
#include "Universe.h"
#include "utility.h"
#include "CommandWrap.h"
#include "Strings.h"
#include "Demo.h"
#include "FEFlow.h"
#include "ResearchGUI.h"
#include "glcaps.h"
#include "render.h"
#include "Tutor.h"
#include "SinglePlayer.h"
#include "glcompat.h"
#include "KeyBindings.h"
#include "InfoOverlay.h"
#include "devstats.h"


#define GLC_STORE 1

extern udword gDevcaps2;

/*=============================================================================
    Local Type Definitions:
=============================================================================*/
//flag settings
#define MCF_SpecialOp                   0x00000001  //has a special operation
#define MCF_SpecialOpDeployed           0x00000002  //has a special operation already deployed
#define MCF_Dockable                    0x00000004  //can dock (big ship)
#define MCF_SmallDockable               0x00000008  //can dock (small ship)
#define MCF_GUICapable                  0x00000010  //is associated with a GUI
#define MCF_NonCombat                   0x00000020  //ship is non combat ship
#define MCF_SupportOverItself           0x00000040  //the mouse is over a selected repair corvette
#define MCF_SingleClickSpecialActivate  0x00000080  //selection includes 1 or more single-click special activate ships
#define MCF_DisableAll                  0x00000100  //disable all cursor bitmaps
#define MCF_DisableMovementMech         0x00000200  //disable cursors when movement pie plate is activated
#define MCF_ShipSelecting               0x00001000  //ships are being selected
#define MCF_BuildGUI                    0x00002000  //ship opens the build manager
#define MCF_SensorsGUI                  0x00004000  //ship opens the sensors manager
#define MCF_ResearchGUI                 0x00008000  //ship opens the research manager
#define MCF_SpecialAttack               0x00010000  //ship has a special attack
#define MCF_SpecialSCSupport            0x00020000  //ship has a special support function for strikecraft
#define MCF_SpecialSupport              0x00040000  //ship has a special support function
#define MCF_SalvageOnly                 0x00080000  //shiplist only contains salvage ships
#define MCF_Salvage                     0x00100000  //shiplist contains at least one salvage ship

typedef struct
{
    udword ship_selected;
    udword resource;
    udword flags;
    mouseCursor cursorover;
} mouseInfoType;


/*=============================================================================
    Data:
=============================================================================*/
//we don't want the player to be able to salvage cryotrays in mission1
//so we need to include singlePlayerGameInfo
extern SinglePlayerGameInfo singlePlayerGameInfo;

//extern void (*mrHoldRight)(void);

sdword mouseCursorXPosition, mouseCursorYPosition;
uword mouseButtons;
udword mouseModuleInit;
rectangle mouseClipRect;
bool8 mouseClip = FALSE;
bool8 mouseIsVisible = TRUE;
uword mouseWillBeVisible = 0;
bool8 mouseDisabled = FALSE;
uword mouseInsideClient = FALSE;

static bool mouseGLInitialized;

mouseCursor  mouseCursorType = normal_mouse;
udword       mouseCursorMode = 0;
MaxSelection mouseCursorSelect;
real32       mouseDoubleClickTime   = 0.0;
real32       mouseDoubleClickExpire = REALlyBig;

real32      mouseCursorOverDistance = REALlyBig;
sdword      mouseCursorOverShipType = MC_NO_SHIP;
sdword      mouseCursorEnemyType = -1;
sdword      mouseCursorEnemyPlayer = 1;
real32      mouseCursorTextExpire   = 0.0f;
SpaceObj    *mouseCursorObjPtr      = NULL;
SpaceObj    *mouseCursorObjLast     = NULL;
real32      mouseCursorLastObjTime  = REALlyNegative;
sdword      mouseCursorLastX        = SDWORD_Max;
sdword      mouseCursorLastY        = SDWORD_Max;
ubyte       mouseCursorOverLODs     = 0;

//cursor colors
static color mouseCursorContentColor = MC_ContentColor;
static color mouseCursorEdgeColor    = MC_EdgeColor;

//cursor font
fonthandle mouseCursorFont = 0;

//cursor bitmaps
static lifheader *mouseNormalBitmap              = NULL;
static lifheader *mouseAddShipsBitmap            = NULL;
static lifheader *mouseFocusNoSelectBitmap       = NULL;
static lifheader *mouseBandBoxAttackBitmap       = NULL;
static lifheader *mouseForceAttackBitmap         = NULL;
static lifheader *mouseGuardBitmap               = NULL;
static lifheader *mouseSpecialAttackBitmap       = NULL;
static lifheader *mouseSpecialOpActivateBitmap   = NULL;
static lifheader *mouseSpecialOpDeactivateBitmap = NULL;
static lifheader *mouseGUIBitmap                 = NULL;
static lifheader *mouseResourceBitmap            = NULL;
static lifheader *mouseDockingBitmap             = NULL;
static lifheader *mouseResearchBitmap            = NULL;
static lifheader *mouseSensorsBitmap             = NULL;
static lifheader *mouseBuildBitmap               = NULL;
static lifheader *mouseSalvageBitmap             = NULL;
static lifheader *mouseSupportBitmap             = NULL;
static lifheader *mouseTradersBitmap             = NULL;


static GLuint tex_Normal = 0;
static GLuint tex_AddShips = 0;
static GLuint tex_FocusNoSelect = 0;
static GLuint tex_BandBoxAttack = 0;
static GLuint tex_ForceAttack = 0;
static GLuint tex_Guard = 0;
static GLuint tex_SpecialAttack = 0;
static GLuint tex_SpecialOpActivate = 0;
static GLuint tex_SpecialOpDeactivate = 0;
static GLuint tex_GUI = 0;
static GLuint tex_Resource = 0;
static GLuint tex_Docking = 0;
static GLuint tex_Research = 0;
static GLuint tex_Sensors = 0;
static GLuint tex_Build = 0;
static GLuint tex_Salvage = 0;
static GLuint tex_Support = 0;
static GLuint tex_Traders = 0;


static mouseInfoType mouseInfo = {0, 0, 0, MC_NO_SHIP};

/*=============================================================================
    Functions:
=============================================================================*/
// ============================================================================
// Utility functions
// ============================================================================
/*-----------------------------------------------------------------------------
    Name        : mouseMirrorBitmap
    Description : Flips a bitmap 180 degrees
    Inputs      : bitmap - pointer to the bitmap data
                  width - width of the bitmap
                  height - height of the bitmap
    Outputs     : replaces "bitmap" with a bitmap flipped 90 degrees clockwise
    Return      :
----------------------------------------------------------------------------*/
#define PIXEL(bm, x, y, offset, width) bm[((x) + (y)*(width))+(offset)]

void mouseMirrorBitmap(udword *bitmap, sdword width, sdword height)
{
    udword i, j;
    udword new_bitmap[1024];

    for (i = 0; i < (udword)width; i++)
    {
        for (j = 0; j < (udword)height; j++)
        {
            PIXEL(new_bitmap, i, (height - 1) - j, 0, width) = PIXEL(bitmap, i, j, 0, width);
        }
    }

    memcpy(bitmap,new_bitmap,sizeof(udword)*width*height);
}
#undef PIXEL

/*-----------------------------------------------------------------------------
    Name        : mouseCreateGLHandle
    Description : creates a GL texture object for a given liff
    Inputs      : lif - the lifheader structure of the bitmap
                  thandle - target for the texture name
    Outputs     : thandle contains the texture name
    Return      :
----------------------------------------------------------------------------*/
void mouseCreateGLHandle(lifheader* lif, GLuint* thandle)
{
    sdword y;
    ubyte* data;
    ubyte* sp;
    ubyte* dp;

    data = (ubyte*)memAlloc(4 * lif->width * lif->height, "temp mouse texture", 0);

    for (y = 0; y < lif->height; y++)
    {
        dp = data + 4 * lif->width * (lif->height - y - 1);
        sp = lif->data + 4 * lif->width * y;
        memcpy(dp, sp, 4 * lif->width);
    }

    if (*thandle != 0)
    {
        glDeleteTextures(1, thandle);
    }

    trClearCurrent();

    glGenTextures(1, thandle);
    glBindTexture(GL_TEXTURE_2D, *thandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lif->width, lif->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    memFree(data);
}

// ============================================================================
// Mouse functions
// ============================================================================
/*-----------------------------------------------------------------------------
    Name        : mouseStartup
    Description : Start the mouse module
    Inputs      : void
    Outputs     : sets the mouse position by getting information from the
                    Windows mouse system.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword mouseStartup(void)
{
    /*
    RECT clientRect, windowRect;
    POINT mousePoint;

    GetWindowRect(ghMainWindow, &windowRect);
    GetClientRect(ghMainWindow, &clientRect);               //get location of window
    GetCursorPos(&mousePoint);                              //get location of Windows mouse cursor
    mouseCursorXPosition = mousePoint.x - windowRect.left;  //compute relative coords
    mouseCursorYPosition = mousePoint.y - windowRect.top;
    */
    SDL_GetMouseState(&mouseCursorXPosition, &mouseCursorYPosition);

    mouseNormalBitmap              = trLIFFileLoad(MC_NORMAL_BM, NonVolatile);
    mouseAddShipsBitmap            = trLIFFileLoad(MC_ADD_SHIPS_BM, NonVolatile);
    mouseFocusNoSelectBitmap       = trLIFFileLoad(MC_FOCUS_NO_SELECT_BM, NonVolatile);
    mouseBandBoxAttackBitmap       = trLIFFileLoad(MC_BAND_BOX_ATTACK_BM, NonVolatile);
    mouseForceAttackBitmap         = trLIFFileLoad(MC_FORCE_ATTACK_BM, NonVolatile);
    mouseGuardBitmap               = trLIFFileLoad(MC_GUARD_BM, NonVolatile);
    mouseSpecialAttackBitmap       = trLIFFileLoad(MC_SPECIAL_ATTACK_BM, NonVolatile);
    mouseSpecialOpActivateBitmap   = trLIFFileLoad(MC_SPECIAL_OP_ACTIVATE_BM, NonVolatile);
    mouseSpecialOpDeactivateBitmap = trLIFFileLoad(MC_SPECIAL_OP_DEACTIVATE_BM, NonVolatile);
    mouseGUIBitmap                 = trLIFFileLoad(MC_GUI_BM, NonVolatile);
    mouseResourceBitmap            = trLIFFileLoad(MC_RESOURCE_BM, NonVolatile);
    mouseDockingBitmap             = trLIFFileLoad(MC_DOCKING_BM, NonVolatile);
    mouseResearchBitmap            = trLIFFileLoad(MC_RESEARCH_BM, NonVolatile);
    mouseSensorsBitmap             = trLIFFileLoad(MC_SENSORS_BM, NonVolatile);
    mouseBuildBitmap               = trLIFFileLoad(MC_BUILD_BM, NonVolatile);
    mouseSalvageBitmap             = trLIFFileLoad(MC_SALVAGE_BM, NonVolatile);
    mouseSupportBitmap             = trLIFFileLoad(MC_SUPPORT_BM, NonVolatile);
    mouseTradersBitmap             = trLIFFileLoad(MC_TRADERS_BM, NonVolatile);

    mouseMirrorBitmap((udword *)mouseNormalBitmap->data, mouseNormalBitmap->width, mouseNormalBitmap->height);
    mouseMirrorBitmap((udword *)mouseAddShipsBitmap->data, mouseAddShipsBitmap->width, mouseAddShipsBitmap->height);
    mouseMirrorBitmap((udword *)mouseFocusNoSelectBitmap->data, mouseFocusNoSelectBitmap->width, mouseFocusNoSelectBitmap->height);
    mouseMirrorBitmap((udword *)mouseBandBoxAttackBitmap->data, mouseBandBoxAttackBitmap->width, mouseBandBoxAttackBitmap->height);
    mouseMirrorBitmap((udword *)mouseForceAttackBitmap->data, mouseForceAttackBitmap->width, mouseForceAttackBitmap->height);
    mouseMirrorBitmap((udword *)mouseGuardBitmap->data, mouseGuardBitmap->width, mouseGuardBitmap->height);
    mouseMirrorBitmap((udword *)mouseSpecialAttackBitmap->data, mouseSpecialAttackBitmap->width, mouseSpecialAttackBitmap->height);
    mouseMirrorBitmap((udword *)mouseSpecialOpActivateBitmap->data, mouseSpecialOpActivateBitmap->width, mouseSpecialOpActivateBitmap->height);
    mouseMirrorBitmap((udword *)mouseSpecialOpDeactivateBitmap->data, mouseSpecialOpDeactivateBitmap->width, mouseSpecialOpDeactivateBitmap->height);
    mouseMirrorBitmap((udword *)mouseGUIBitmap->data, mouseGUIBitmap->width, mouseGUIBitmap->height);
    mouseMirrorBitmap((udword *)mouseResourceBitmap->data, mouseResourceBitmap->width, mouseResourceBitmap->height);
    mouseMirrorBitmap((udword *)mouseDockingBitmap->data, mouseDockingBitmap->width, mouseDockingBitmap->height);
    mouseMirrorBitmap((udword *)mouseResearchBitmap->data, mouseResearchBitmap->width, mouseResearchBitmap->height);
    mouseMirrorBitmap((udword *)mouseSensorsBitmap->data, mouseSensorsBitmap->width, mouseSensorsBitmap->height);
    mouseMirrorBitmap((udword *)mouseBuildBitmap->data, mouseBuildBitmap->width, mouseBuildBitmap->height);
    mouseMirrorBitmap((udword *)mouseSalvageBitmap->data, mouseSalvageBitmap->width, mouseSalvageBitmap->height);
    mouseMirrorBitmap((udword *)mouseSupportBitmap->data, mouseSupportBitmap->width, mouseSupportBitmap->height);
    mouseMirrorBitmap((udword *)mouseTradersBitmap->data, mouseTradersBitmap->width, mouseTradersBitmap->height);

    /*mouseDoubleClickTime = GetDoubleClickTime() / 1000.0f;*/
    mouseDoubleClickTime = 0.5f;  /* Consistent with HandleEvent() in main.c. */
    mouseCursorFont      = frFontRegister(TW_CURSORTEXT_FONT);

    mouseGLInitialized = FALSE;

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : mouseInitGL
    Description : initialize the GL texture handles for mouse bitmaps
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mouseInitGL(void)
{
    mouseCreateGLHandle(mouseNormalBitmap, &tex_Normal);
    mouseCreateGLHandle(mouseAddShipsBitmap, &tex_AddShips);
    mouseCreateGLHandle(mouseFocusNoSelectBitmap, &tex_FocusNoSelect);
    mouseCreateGLHandle(mouseBandBoxAttackBitmap, &tex_BandBoxAttack);
    mouseCreateGLHandle(mouseForceAttackBitmap, &tex_ForceAttack);
    mouseCreateGLHandle(mouseGuardBitmap, &tex_Guard);
    mouseCreateGLHandle(mouseSpecialAttackBitmap, &tex_SpecialAttack);
    mouseCreateGLHandle(mouseSpecialOpActivateBitmap, &tex_SpecialOpActivate);
    mouseCreateGLHandle(mouseSpecialOpDeactivateBitmap, &tex_SpecialOpDeactivate);
    mouseCreateGLHandle(mouseGUIBitmap, &tex_GUI);
    mouseCreateGLHandle(mouseResourceBitmap, &tex_Resource);
    mouseCreateGLHandle(mouseDockingBitmap, &tex_Docking);
    mouseCreateGLHandle(mouseResearchBitmap, &tex_Research);
    mouseCreateGLHandle(mouseSensorsBitmap, &tex_Sensors);
    mouseCreateGLHandle(mouseBuildBitmap, &tex_Build);
    mouseCreateGLHandle(mouseSalvageBitmap, &tex_Salvage);
    mouseCreateGLHandle(mouseSupportBitmap, &tex_Support);
    mouseCreateGLHandle(mouseTradersBitmap, &tex_Traders);
}

/*-----------------------------------------------------------------------------
    Name        : mouseReset
    Description : reset mouse module (delete associated GL texture objects)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mouseReset(void)
{
    if (mouseGLInitialized)
    {
        glDeleteTextures(1, &tex_Normal);
        glDeleteTextures(1, &tex_AddShips);
        glDeleteTextures(1, &tex_FocusNoSelect);
        glDeleteTextures(1, &tex_BandBoxAttack);
        glDeleteTextures(1, &tex_ForceAttack);
        glDeleteTextures(1, &tex_Guard);
        glDeleteTextures(1, &tex_SpecialAttack);
        glDeleteTextures(1, &tex_SpecialOpActivate);
        glDeleteTextures(1, &tex_SpecialOpDeactivate);
        glDeleteTextures(1, &tex_GUI);
        glDeleteTextures(1, &tex_Resource);
        glDeleteTextures(1, &tex_Docking);
        glDeleteTextures(1, &tex_Research);
        glDeleteTextures(1, &tex_Sensors);
        glDeleteTextures(1, &tex_Build);
        glDeleteTextures(1, &tex_Salvage);
        glDeleteTextures(1, &tex_Support);
        glDeleteTextures(1, &tex_Traders);

        tex_Normal = 0;
        tex_AddShips = 0;
        tex_FocusNoSelect = 0;
        tex_BandBoxAttack = 0;
        tex_ForceAttack = 0;
        tex_Guard = 0;
        tex_SpecialAttack = 0;
        tex_SpecialOpActivate = 0;
        tex_SpecialOpDeactivate = 0;
        tex_GUI = 0;
        tex_Resource = 0;
        tex_Docking = 0;
        tex_Research = 0;
        tex_Sensors = 0;
        tex_Build = 0;
        tex_Salvage = 0;
        tex_Support = 0;
        tex_Traders = 0;

        mouseGLInitialized = FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mouseShutdown
    Description : Close mouse module
    Inputs      : void
    Outputs     : not much
    Return      : void
----------------------------------------------------------------------------*/
void mouseShutdown(void)
{
    mouseReset();
}

/*-----------------------------------------------------------------------------
    Name        : mouseCursorHide
    Description : Show the cursor if it is hidden
    Inputs      : void
    Outputs     : sets the visibility flag to TRUE.  Subsequent mouse
                    mouseDraw() calls will not display a visible cursor.
    Return      : void
----------------------------------------------------------------------------*/
void mouseCursorHide(void)
{
    mouseIsVisible = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : mouseCursorShow
    Description : Show the cursor if it is hidden
    Inputs      : void
    Outputs     : sets the visibility flag to TRUE.  Subsequent mouse
                    mouseDraw() calls will not display a visible cursor.
    Return      : void
----------------------------------------------------------------------------*/
void mouseCursorShow(void)
{
    mouseIsVisible = TRUE;
}

void mouseCursorDelayShow(uword delay)
{
    mouseWillBeVisible = delay+1;
}

/*-----------------------------------------------------------------------------
    Name        : mousePositionSet
    Description : Set new mouse position.
    Inputs      : x, y - new location of mouse.  May be clamped to the edge of
                    the window.
    Outputs     : Sets the Windows cursor location and the internal cursor location.
    Return      : void
----------------------------------------------------------------------------*/
void mousePositionSet(sdword x, sdword y)
{
    rectangle clientRect;
//    RECT clientRect, windowRect;
//    POINT mousePoint;

    utyClientRectGet(&clientRect);                          //get window location
//  GetWindowRect(ghMainWindow, &windowRect);
//  GetClientRect(ghMainWindow, &clientRect);               //get location of window
/*
#if MOUSE_CURSOR_CLAMP
    //clamp specified mouse location
    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }
    if (x >= clientRect.x1 - clientRect.x0)
    {
        x = clientRect.x1 - clientRect.x0 - 1;
    }
    if (y >= clientRect.y1 - clientRect.y0)
    {
        y = clientRect.y1 - clientRect.y0 - 1;
    }
#endif
*/
    mouseCursorXPosition = x;                               //set internal mouse location
    mouseCursorYPosition = y;
    /*
    mousePoint.x = clientRect.x0 + x;                       //compute Windows mouse location
    mousePoint.y = clientRect.y0 + y;
    SetCursorPos(mousePoint.x, mousePoint.y);               //set Windows mouse location
    */

    SDL_WarpMouse(x, y);
}

/*-----------------------------------------------------------------------------
    Name        : mouseClipToRect
    Description : Clips the mouse to a given rectangle.
    Inputs      : rect - rectangle to clip to.  NULL means no rectangle.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mouseClipToRect(rectangle *rect)
{
    if (rect == NULL)
    {
        mouseClip = FALSE;
    }
    else
    {
        mouseClip = TRUE;
        mouseClipRect = *rect;
    }
}


/*-----------------------------------------------------------------------------
    Name        : mouseLDoubleClick
    Description : Processes a double click on the left mouse button
    Inputs      : void
    Outputs     : Very specific depending on the state of mouseInfo
    Return      : TRUE if some processing has been done (overrides the processing
                  for a single left click)
                  FALSE if no processing was done (processing for the single
                  left click continues)
----------------------------------------------------------------------------*/
bool mouseLDoubleClick(void)
{
    MaxSelection tempSelection;

    //if a menu is up, don't do any of this
//    if (feStackIndex > 0)
//    {
//        if (rmGUIActive)
//        {
//            return TRUE;
//        }
//        else
//        {
//            return FALSE;
//        }
//    }

//**** big chunks taken out because this function is being called
//     from mainrgn now.

//    if (bitTest(mouseCursorMode, MCM_SingleClick))
//    {
//        if (bitTest(mouseCursorMode, MCM_DoubleClick))
//        {
//            bitClear(mouseCursorMode, MCM_DoubleClick);
//            bitClear(mouseCursorMode, MCM_SingleClick);
//            mouseCursorSelect.numShips = 0;
//        }
//        else
//        {
//            bitSet(mouseCursorMode, MCM_DoubleClick);
//            mouseDoubleClickExpire = universe.totaltimeelapsed + mouseDoubleClickTime;
//            return FALSE;
//        }
//    }

    if (selSelected.numShips)
    {
#if !MR_GUI_SINGLECLICK
//        if (bitTest(mouseInfo.flags, MCF_BuildGUI))
//        {
//            tutGameMessage("Game_DoubleClickBuild");
//            cmConstructionBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
//            return TRUE;
//        }
//        else if (bitTest(mouseInfo.flags, MCF_SensorsGUI))
//        {
//            tutGameMessage("Game_DoubleClickSensors");
//            smSensorsBegin(NULL, NULL);
//            return TRUE;
//        }
//        else if (bitTest(mouseInfo.flags, MCF_ResearchGUI))
//        {
//            tutGameMessage("Game_DoubleClickResearch");
//            rmResearchGUIBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
//        }
//        else
#endif  // !MR_GUI_SINGLECLICK
//        if (ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, RepairCorvette) ||
//            ((ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, AdvanceSupportFrigate) ||
//              ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, Carrier)) &&
//             (mouseCursorObjPtr &&
//              (isShipOfClass((Ship *)mouseCursorObjPtr, CLASS_Fighter) ||
//               isShipOfClass((Ship *)mouseCursorObjPtr, CLASS_Corvette)))))

        if (mouseCursorObjPtr &&                // if there's an object being clicked on
            ((isShipOfClass((Ship *)mouseCursorObjPtr, CLASS_Fighter) ||        // Carriers only support strikecraft
              isShipOfClass((Ship *)mouseCursorObjPtr, CLASS_Corvette)) &&
             ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, Carrier)) ||
            (ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, RepairCorvette) ||
             ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, AdvanceSupportFrigate)))
        {
            // make a backup of mouseCursorSelect
            tempSelection.numShips = 0;
            selSelectionCopy((MaxAnySelection *)&tempSelection, (MaxAnySelection *)&mouseCursorSelect);

            MakeShipsSpecialTargetCapable((SelectCommand *)&mouseCursorSelect,TRUE);
            MakeShipsNotIncludeTheseShips((SelectCommand *)&mouseCursorSelect, (SelectCommand *)&selSelected);

            if (mouseCursorSelect.numShips > 0)
            {
                SelectCommand sel;

                selSelectionCopy((MaxAnySelection *)&selSelected,
                                 (MaxAnySelection *)&mouseCursorSelect);

				ioUpdateShipTotals();

                if (mouseCursorSelect.numShips < tempSelection.numShips)
                {
                    // the selection being activated has other ships as well
                    tutGameMessage("Game_DoubleClickSpecial");
                    sel.numShips = 1;
                    sel.ShipPtr[0] = (Ship *)mouseCursorObjPtr;
                    clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&mouseCursorSelect, (SpecialCommand *)&sel);
                    selSelectionCopy((MaxAnySelection *)&mouseCursorSelect,
                                     (MaxAnySelection *)&tempSelection);
                    return FALSE; //we may not be done...
                }
                else
                {
                    // the selection being activated only contains support ships
                    mouseCursorSelect.numShips = 0;
                    tutGameMessage("Game_DoubleClickSpecial");
                    sel.numShips   = 1;
                    sel.ShipPtr[0] = (Ship *)mouseCursorObjPtr;
                    clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&selSelected,(SpecialCommand *)&sel);
                    return TRUE;
                }
            }
            bitClear(mouseCursorMode, MCM_DoubleClick);
            bitClear(mouseCursorMode, MCM_SingleClick);
        }
        if (bitTest(mouseInfo.flags, MCF_SpecialOp))
        {
            MakeShipsSpecialActivateCapable((SelectCommand *)&mouseCursorSelect);

            if (mouseCursorSelect.numShips > 0)
            {
                selSelectionCopy((MaxAnySelection *)&selSelected,
                                 (MaxAnySelection *)&mouseCursorSelect);
                ioUpdateShipTotals();
                mouseCursorSelect.numShips = 0;
                tutGameMessage("Game_DoubleClickSpecial");
                clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&selSelected,NULL);
                return TRUE;
            }
            bitClear(mouseCursorMode, MCM_DoubleClick);
            bitClear(mouseCursorMode, MCM_SingleClick);

        }
    }
//    else if ((mouseCursorObjPtr != NULL) &&
//             (mouseCursorObjPtr->objtype != OBJ_ShipType))
//    {
//        if (mouseCursorSelect.numShips > 0)
//        {
//            selSelectionCopy((MaxAnySelection *)&selSelected,
//                             (MaxAnySelection *)&mouseCursorSelect);
//            mouseCursorSelect.numShips = 0;
//            tutGameMessage("Game_DoubleClickHarvest");
//            clWrapCollectResource(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (Resource*)mouseCursorObjPtr);
//        }
//        return TRUE;
//    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : mouseDoubleClickCheck
    Description : Check to see if the double click time has expired and performs
                  special tasks if it has
    Inputs      : void
    Outputs     : Very specific depending on the state of mouseInfo
    Return      : TRUE if processing was done, FALSE if not
----------------------------------------------------------------------------*/
bool mouseDoubleClickCheck(void)
{
    if (mouseDoubleClickExpire == 0.0)
    {
        return FALSE;
    }

    if (mouseDoubleClickExpire < universe.totaltimeelapsed)
    {
        if (bitTest(mouseCursorMode, MCM_DoubleClick))
        {
            bitClear(mouseCursorMode, MCM_DoubleClick);
            bitClear(mouseCursorMode, MCM_SingleClick);
            mouseDoubleClickExpire = 0.0;

            if (mouseCursorObjPtr != NULL)
            {
                if (mouseCursorSelect.numShips > 0)
                {
                    selSelectionCopy((MaxAnySelection *)&selSelected,
                                     (MaxAnySelection *)&mouseCursorSelect);
                    mouseCursorSelect.numShips = 0;
                    makeShipsDockCapable((SelectCommand *)&selSelected); //filter ships that can't dock
                    clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&selSelected,
                               DOCK_AT_SPECIFIC_SHIP, (Ship *)mouseCursorObjPtr);
                    mouseDrawType(small_docking);     //may need to have "docking" as well
                }
                return TRUE;
            }
        }
        mouseCursorSelect.numShips = 0;
        mouseDoubleClickExpire     = 0.0;
        bitClear(mouseCursorMode, MCM_SingleClick);
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : mouseLClick
    Description : Special processing a single click on the left mouse button
    Inputs      : void
    Outputs     : Very specific depending on the state of mouseInfo
    Return      : void
----------------------------------------------------------------------------*/
void mouseLClick(void)
{
    //if a menu is up, don't do any of this
    if (feStackIndex > 0)
    {
        return;
    }


    if (!bitTest(mouseCursorMode, MCM_SingleClick))
    {
        if ((mouseCursorType == docking) ||
            (mouseCursorType == small_docking))
        {
            selSelectionCopy((MaxAnySelection *)&mouseCursorSelect, (MaxAnySelection *)&selSelected);
            bitSet(mouseCursorMode, MCM_SingleClick);
            mouseDoubleClickExpire = universe.totaltimeelapsed + mouseDoubleClickTime;
        }
        else if ((mouseCursorType == special_op_activate) ||
                 (mouseCursorType == special_op_deactivate))
        {
            selSelectionCopy((MaxAnySelection *)&mouseCursorSelect, (MaxAnySelection *)&selSelected);
            mouseDoubleClickExpire = universe.totaltimeelapsed + mouseDoubleClickTime;
        }
        else if (mouseCursorType == support)
        {
            selSelectionCopy((MaxAnySelection *)&mouseCursorSelect, (MaxAnySelection *)&selSelected);
            mouseDoubleClickExpire = universe.totaltimeelapsed + mouseDoubleClickTime;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : mouseClickShipDied
    Description : Takes care of ships dying at inopportune times by clearing the
                  ship from any file global selection lists or storage areas
    Inputs      : deadship - the ship that has just died
    Outputs     : removes the ship from mouseCursorSelect and the mouseCursorObjPtr
    Return      : void
----------------------------------------------------------------------------*/
void mouseClickShipDied(ShipPtr deadship)
{
    clRemoveShipFromSelection(&mouseCursorSelect, deadship);

    if ((mouseCursorObjPtr != NULL) && (mouseCursorObjPtr == (SpaceObj *)deadship))
    {
        mouseCursorObjPtr = NULL;
    }

    if (mouseCursorObjLast == (SpaceObj *)deadship)
    {
        mouseCursorObjLast = NULL;
    }
}



/*-----------------------------------------------------------------------------
    Name        : mouseDrawTriangle
    Description : Draws the mouse triangle on the screen
    Inputs      : void
    Outputs     : Draws the mouse triangle on the screen
    Return      : void
----------------------------------------------------------------------------*/
void mouseDrawTriangle(void)
{
    triangle mouseTri, mouseTri2;

    mouseTri.x0 = mouseCursorXPosition + 0;
    mouseTri.y0 = mouseCursorYPosition + 0;
    mouseTri.x1 = mouseCursorXPosition + 4;
    mouseTri.y1 = mouseCursorYPosition + 16;
    mouseTri.x2 = mouseCursorXPosition + 14;
    mouseTri.y2 = mouseCursorYPosition + 7;
    mouseTri2 = mouseTri;
    mouseTri2.x2++;
    primTriSolid2(&mouseTri2, mouseCursorContentColor);      //draw mouse cursor
    primTriOutline2(&mouseTri, 1, mouseCursorEdgeColor);
}


/*-----------------------------------------------------------------------------
    Name        : mouseInfoReset
    Description : Resets the mouseInfo structure
    Inputs      : void
    Outputs     : Resets the mouseInfo structure
    Return      : void
----------------------------------------------------------------------------*/
void mouseInfoReset(void)
{
    mouseInfo.ship_selected = 0;
    mouseInfo.resource      = 0;
    mouseInfo.flags         = 0;
    mouseInfo.cursorover    = MC_NO_SHIP;
}

lifheader* mouseGetLIF(void)
{
    switch (mouseCursorType)
    {
        case no_mouse:
        case rotate_camera:
        case zoom_camera:
            return NULL;
        case normal_mouse:
        case movement:
            return mouseNormalBitmap;
        case add_ships:
            return mouseAddShipsBitmap;
        case focus_no_select:
            return mouseFocusNoSelectBitmap;
        case band_box_attack:
            return mouseBandBoxAttackBitmap;
        case force_attack:
            return mouseForceAttackBitmap;
        case m_guard:
            return mouseGuardBitmap;
        case special_attack:
            return mouseSpecialAttackBitmap;
        case special_op_activate:
            return mouseSpecialOpActivateBitmap;
        case special_op_deactivate:
            return mouseSpecialOpDeactivateBitmap;
        case gui:
            return mouseGUIBitmap;
        case resource:
            return mouseResourceBitmap;
        case docking:
        case small_docking:
            return mouseDockingBitmap;
        case research:
            return mouseResearchBitmap;
        case sensors:
            return mouseSensorsBitmap;
        case build:
            return mouseBuildBitmap;
        case salvage:
            return mouseSalvageBitmap;
        case support:
            return mouseSupportBitmap;
        case traders:
            return mouseTradersBitmap;
        default:
            return NULL;
    }
}

GLubyte cursorUnderContents[4096];

sdword lastUnderWidth = 0, lastUnderHeight = 0;
static sdword lastUnderX = 0, lastUnderY = 0;

void mouseRestoreCursorUnder(void)
{
    if (!feShouldSaveMouseCursor())
    {
        return;
    }
    if (RGLtype == SWtype)
    {
        rglRestoreCursorUnder(cursorUnderContents,
                              lastUnderWidth, lastUnderHeight,
                              lastUnderX, lastUnderY);
    }
#if GLC_STORE
    else
    {
        glcCursorUnder((ubyte*)cursorUnderContents,
                       lastUnderWidth, lastUnderHeight,
                       lastUnderX, lastUnderY,
                       FALSE);
    }
#endif
}

void mouseStoreCursorUnder(void)
{
    lifheader* texture;

    if (!feShouldSaveMouseCursor())
    {
        return;
    }

    texture = mouseGetLIF();
    if (texture == NULL)
    {
        memset(cursorUnderContents, 0, 4096);
    }
    else
    {
        lastUnderWidth = texture->width + 2;
        lastUnderHeight = texture->height + 2;
        if (RGLtype == SWtype)
        {
            lastUnderX = mouseCursorXPosition - 1;
            lastUnderY = mouseCursorYPosition - 1;
            rglSaveCursorUnder(cursorUnderContents, lastUnderWidth, lastUnderHeight, lastUnderX, lastUnderY);
        }
#if GLC_STORE
        else
        {
            lastUnderX = mouseCursorXPosition;
            lastUnderY = mouseCursorYPosition;
            glcCursorUnder((ubyte*)cursorUnderContents,
                           lastUnderWidth, lastUnderHeight,
                           lastUnderX, lastUnderY,
                           TRUE);
        }
#endif
    }
}


/*-----------------------------------------------------------------------------
    Name        : mouseDraw
    Description : Update the position of the mouse by polling the system and
                    draw it at it's new location.
    Inputs      : void
    Outputs     : updates mouse cursor position and button states to reflect
                    new mouse status.
    Return      : void
    Note        : because this routine performs drawing operations, it should
                    be called near the end rendering, before a buffer swap.
                  Checks to see if the mouse is inside or outside the client area
----------------------------------------------------------------------------*/
extern regionhandle spHyperspaceRollCallScreen;
#define SX primScreenToGLX
#define SY primScreenToGLY
void mouseDraw(void)
{
    bool primModeOn = TRUE;
    lifheader *texture = NULL;
    GLuint thandle = 0;
    bool textureMouse;
    static mouseCursor mousehold = normal_mouse;
    static real32 mouseholdexpire = 0.0;

    if (mouseWillBeVisible > 0)
    {
        mouseWillBeVisible--;
        if (!mouseWillBeVisible)
            mouseIsVisible = TRUE;
    }

    if ((!mouseIsVisible) || (mouseDisabled))
    {
        return;                                             //don't draw mouse if not visible
    }

//    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_HINT_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);
    //rndGLStateLog("MouseDraw (start)");

    if (RGLtype == SWtype)
    {
        textureMouse = FALSE;
    }
    else
    {
        textureMouse = TRUE;//!glcActive();
    }

    if (textureMouse && !mouseGLInitialized)
    {
        mouseInitGL();
        mouseGLInitialized = TRUE;
    }

    //mousePoll();

    if (!primModeEnabled)
    {
        primModeOn = FALSE;
        primModeSetFunction2();
    }

    glEnable(GL_BLEND);
    rndAdditiveBlends(FALSE);
    rndTextureEnvironment(RTE_Modulate);

    if ((mousehold != normal_mouse) && (mouseCursorType == normal_mouse))
    {
        if (mouseholdexpire >= universe.totaltimeelapsed)
        {
            mouseCursorType = mousehold;
        }
        else //mousehold has expired
        {
            mousehold = normal_mouse;
        }
    }

    //if a front end screen is on (but not the hyperspace roll call screen)
    //all mouse settings are overridden
    //if the movement pieplate is on, all mouse settings except for
    // the focus_no_select setting is overridden
    if (((feStackIndex > 0) && (!spHyperspaceRollCallScreen)) ||
        ((bitTest(mouseCursorMode, MCM_Pointmode)) && (mouseCursorType != focus_no_select)))
    {
        mouseCursorType = normal_mouse;
    }

    switch (mouseCursorType)
    {
        case no_mouse:
        case rotate_camera:
        case zoom_camera:
            //temporary solution
            goto finish;
            //do nothing
            break;
        case normal_mouse:
        case movement:
            texture = mouseNormalBitmap;
            thandle = tex_Normal;
            break;
        case add_ships:
            texture = mouseAddShipsBitmap;
            thandle = tex_AddShips;
            break;
        case focus_no_select:
            texture = mouseFocusNoSelectBitmap;
            thandle = tex_FocusNoSelect;
            break;
        case band_box_attack:
            texture = mouseBandBoxAttackBitmap;
            thandle = tex_BandBoxAttack;
            break;
        case force_attack:
            texture = mouseForceAttackBitmap;
            thandle = tex_ForceAttack;
            break;
        case m_guard:
            texture = mouseGuardBitmap;
            thandle = tex_Guard;
            break;
        case special_attack:
            if (mousehold == normal_mouse)
            {
                mousehold = special_attack;
                mouseholdexpire = universe.totaltimeelapsed + (real32)MC_MOUSEHOLDTIME;
            }
            texture = mouseSpecialAttackBitmap;
            thandle = tex_SpecialAttack;
            break;
        case special_op_activate:
            texture = mouseSpecialOpActivateBitmap;
            thandle = tex_SpecialOpActivate;
            break;
        case special_op_deactivate:
            texture = mouseSpecialOpDeactivateBitmap;
            thandle = tex_SpecialOpDeactivate;
            break;
        case gui:
            texture = mouseGUIBitmap;
            thandle = tex_GUI;
            break;
        case resource:
            texture = mouseResourceBitmap;
            thandle = tex_Resource;
            break;
        case docking:
        case small_docking:
            texture = mouseDockingBitmap;
            thandle = tex_Docking;
            break;
        case research:
            texture = mouseResearchBitmap;
            thandle = tex_Research;
            break;
        case sensors:
            texture = mouseSensorsBitmap;
            thandle = tex_Sensors;
            break;
        case build:
            texture = mouseBuildBitmap;
            thandle = tex_Build;
            break;
        case salvage:
            texture = mouseSalvageBitmap;
            thandle = tex_Salvage;
            break;
        case support:
            texture = mouseSupportBitmap;
            thandle = tex_Support;
            break;
        case traders:
            texture = mouseTradersBitmap;
            thandle = tex_Traders;
            break;
        default:
            break;
    }
    if (texture != NULL)
    {
        if (textureMouse)
        {
            bool texOn;

            trClearCurrent();
            texOn = rndTextureEnable(TRUE);
            glBindTexture(GL_TEXTURE_2D, thandle);

            glColor3ub(255,255,255);

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(SX(mouseCursorXPosition), SY(mouseCursorYPosition));
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(SX(mouseCursorXPosition), SY(mouseCursorYPosition + texture->height));
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(SX(mouseCursorXPosition + texture->width), SY(mouseCursorYPosition + texture->height));
            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(SX(mouseCursorXPosition + texture->width), SY(mouseCursorYPosition));
            glEnd();

            rndTextureEnable(texOn);
        }
        else
        {
            glRasterPos2f(primScreenToGLX(mouseCursorXPosition),
                          primScreenToGLY(mouseCursorYPosition + texture->height));
            glDrawPixels(texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
        }
    }
    else
    {
        mouseDrawTriangle();
    }

  finish:

    glDisable(GL_BLEND);

    if (!primModeOn)
    {
        primModeClearFunction2();
    }
    mouseCursorOverDistance = REALlyBig;
    mouseCursorOverShipType = MC_NO_SHIP;

    if (bitTest(mouseCursorMode, MCM_CursorOverObj))
    {
        bitClear(mouseCursorMode, MCM_CursorOverObj);
    }
    else
    {
        mouseCursorObjPtr = NULL;
    }
    if (mouseCursorObjPtr != NULL)
    {                                                       //if the mouse is over an object
        mouseCursorObjLast = mouseCursorObjPtr;             //remember where and when it was
        mouseCursorLastObjTime = universe.totaltimeelapsed;
        mouseCursorLastX = mouseCursorX();
        mouseCursorLastY = mouseCursorY();
    }
    //rndGLStateLog("MouseDraw (end)");
//    glPopAttrib();
}
#undef SX
#undef SY

char *tutMouseDrawMessage[] =
{   "Mouse_NormalMouse",
    "Mouse_RotateCamera",
    "Mouse_ZoomCamera",
    "Mouse_AddShips",
    "Mouse_FocusNoSelect",
    "Mouse_Movement",
    "Mouse_BandBoxAttack",
    "Mouse_ForceAttack",
    "Mouse_Guard",
    "Mouse_SpecialAttack",
    "Mouse_SpecialOpActivate",
    "Mouse_SpecialOpDeactivate",
    "Mouse_Gui",
    "Mouse_Resource",
    "Mouse_Derelict",
    "Mouse_Docking",
    "Mouse_SmallDocking",
    "Mouse_Research",
    "Mouse_Sensors",
    "Mouse_Build",
    "Mouse_Salvage",
    "Mouse_Support",
    "Mouse_Traders",
    "Mouse_EnemyShip",
    "Mouse_AlliedShip",
};



/*-----------------------------------------------------------------------------
    Name        : mouseDrawType
    Description : Sets the type of mouse cursor to be drawn
    Inputs      : cursorType - the type of cursor to draw
    Outputs     : Changes the global mouse cursor type variable
    Return      : void
----------------------------------------------------------------------------*/
void mouseDrawType(mouseCursor cursorType)
{
    //mouse messages only happen in the actual tutorial because
    //the overflow the message queue in the single player game
    if((tutorial==TUTORIAL_ONLY) && cursorType != mouseCursorType)
        tutGameMessage( tutMouseDrawMessage[(long)cursorType] );

    mouseCursorType = cursorType;
}


/*-----------------------------------------------------------------------------
    Name        : mouseCursorSet
    Description : sets the cursor mode for special case cursor settings
    Inputs      : mode - the mode to set cursormode to
    Outputs     : changes the cursormode flag
    Return      : void
----------------------------------------------------------------------------*/
void mouseCursorSet(udword mode)
{
    bitSet(mouseCursorMode, mode);
}

/*-----------------------------------------------------------------------------
    Name        : mouseCursorClear
    Description : Clears the cursor mode for special case cursor settings
    Inputs      : mode - the mode to clear
    Outputs     : changes the cursormode flag
    Return      : void
----------------------------------------------------------------------------*/
void mouseCursorClear(udword mode)
{
    bitClear(mouseCursorMode, mode);
}

/*-----------------------------------------------------------------------------
    Name        : mouseSelectCursorSetting
    Description : Gathers information about the selected ships, and
                  ships under the mouse cursor
                  I agree this function is ugly, but you have to consider
                  that any number of ships can be selected at one time,
                  and the mouse can be over any of those ships, or any
                  other ships.  This function has to take care of all of
                  that.  Poor function...  overworked, underpaid, ugly,
                  unappreciated,  just not having a good life.
                  Be kind to it...
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mouseSelectCursorSetting(void)
{
    sdword i;

    mouseInfoReset();

    //if there are ships selected
    if (selSelected.numShips)
    {
        Ship *ship;

        mouseInfo.ship_selected = selSelected.numShips;

        //go through the selected list
        for (i = 0; i < selSelected.numShips; i++)
        {
            ship = selSelected.ShipPtr[i];

            //count the number of resource collectors selected
            if (ship->shiptype == ResourceCollector)
            {
                ++mouseInfo.resource;
                bitSet(mouseInfo.flags, MCF_Dockable);
            }

            if (ship->staticinfo->canSingleClickSpecialActivate)
            {
                bitSet(mouseInfo.flags, MCF_SingleClickSpecialActivate);
            }

            //-- Set flags for selected ships under the cursor --//
            //if the cursor is over a selected ship
            if ((mouseCursorObjPtr != NULL) &&
                ((Ship *)mouseCursorObjPtr == selSelected.ShipPtr[i]))
            {
                //set flags for all GUI or special op ships
                switch (ship->shiptype)
                {
                    case Carrier:
                    case Mothership:
                        bitSet(mouseInfo.flags, MCF_GUICapable);
                        bitSet(mouseInfo.flags, MCF_BuildGUI);
                        break;
                    case ResearchShip:
                        bitSet(mouseInfo.flags, MCF_GUICapable);
                        bitSet(mouseInfo.flags, MCF_ResearchGUI);
                        break;
                    case SensorArray:
                        bitSet(mouseInfo.flags, MCF_GUICapable);
                        bitSet(mouseInfo.flags, MCF_SensorsGUI);
                        break;
                    case CloakGenerator:
                    case CloakedFighter:
                        if (bitTest(ship->flags, SOF_Cloaked))
                        {
                            bitSet(mouseInfo.flags, MCF_SpecialOpDeployed);
                        }
                        bitSet(mouseInfo.flags, MCF_SpecialOp);
                        break;
                    case DDDFrigate:
                        if (((DDDFrigateSpec *)ship->ShipSpecifics)->DDDstate == 3) //DDDSTATE_ALLOUTSIDE
                        {
                            bitSet(mouseInfo.flags, MCF_SpecialOpDeployed);
                        }
                        bitSet(mouseInfo.flags, MCF_SpecialOp);
                        break;
                    case GravWellGenerator:
                        if (((GravWellGeneratorSpec *)ship->ShipSpecifics)->GravFieldOn == TRUE)
                        {
                            bitSet(mouseInfo.flags, MCF_SpecialOpDeployed);
                        }
                        bitSet(mouseInfo.flags, MCF_SpecialOp);
                        break;
                    case MinelayerCorvette:
                        bitSet(mouseInfo.flags, MCF_SpecialOp);
                        break;
                    case AdvanceSupportFrigate:
                    case RepairCorvette:
                        //we don't want the dock cursor to show up when
                        //the mouse is over a selected repair corvette
                        bitSet(mouseInfo.flags, MCF_SupportOverItself);
                        break;
                }
            }

            // -- Set flags for ships that are selected but not under the cursor -- //
            //set flags for dockable ships
            if ((ship->staticinfo->shipclass == CLASS_Corvette) ||
                (ship->staticinfo->shipclass == CLASS_Fighter))
            {
                if (ship->shiptype != RepairCorvette)
                {
                    bitSet(mouseInfo.flags, MCF_SmallDockable);
                }
                else
                {
                    bitSet(mouseInfo.flags, MCF_Dockable);
                }
            }
            else if ((ship->staticinfo->shipclass == CLASS_Resource) ||
                     (ship->staticinfo->shipclass == CLASS_NonCombat))
            {
                bitSet(mouseInfo.flags, MCF_NonCombat);
            }
            //set flags for special attack/support ships
            switch (ship->shiptype)
            {
                case MissileDestroyer:
                case HeavyCorvette:
                    bitSet(mouseInfo.flags, MCF_SpecialAttack);
                    break;
                case Carrier:
                    bitSet(mouseInfo.flags, MCF_SpecialSCSupport);
                    break;
                case AdvanceSupportFrigate:
                case RepairCorvette:
                    bitSet(mouseInfo.flags, MCF_SpecialSupport);
                    break;
                case SalCapCorvette:
                    bitSet(mouseInfo.flags, MCF_Salvage);

                    //if this is the first ship in the list
                    if (i==0)
                    {
                        bitSet(mouseInfo.flags, MCF_SalvageOnly);
                    }
                    break;
            }

            //the special salvage cursor bitmap is displayed
            //instead of the attack bitmap if selselected only contains
            //salvage capture corvettes... checking for this here
            if (i>0 && bitTest(mouseInfo.flags, MCF_SalvageOnly))
            {
                if (ship->shiptype != SalCapCorvette)
                {
                    bitClear(mouseInfo.flags, MCF_SalvageOnly);
                }
            }

        }

        //if there isn't anything under the cursor, then the rest of this
        //function isn't needed
        if (mouseCursorOverShipType == MC_NO_SHIP)
        {
            return;
        }

        //set cursorover variable depending on the ship under cursor and
        //the value of certain flags.
        switch (mouseCursorOverShipType)
        {
            case Carrier:
            case Mothership:
                if (bitTest(mouseInfo.flags, MCF_Dockable) ||
                    bitTest(mouseInfo.flags, MCF_SmallDockable))
                {
                    mouseInfo.cursorover = docking;
                }
                else
                {
                    mouseInfo.cursorover = build;
                }
                break;
            case ResearchShip:
                mouseInfo.cursorover = research;
                break;
            case SensorArray:
                mouseInfo.cursorover = sensors;
                break;
            case ResourceController:
                mouseInfo.cursorover = docking;
                break;
            case CloakGenerator:
            case CloakedFighter:
            case DDDFrigate:
            case GravWellGenerator:
            case MinelayerCorvette:
                mouseInfo.cursorover = special_op_activate;
                break;
            //drone is not considered as a ship in this case
            case Drone:
                mouseInfo.cursorover = MC_NO_SHIP;
                break;
            case RepairCorvette:
            case ResourceCollector:
            case AdvanceSupportFrigate:
            case JunkYardHQ:
                mouseInfo.cursorover = small_docking;
                break;
            case CryoTray:
                if ((singlePlayerGameInfo.currentMission != 1) &&
                    (bitTest(mouseInfo.flags, MCF_SalvageOnly)))
                {
                    mouseInfo.cursorover = salvage;
                }
                break;
            case FloatingCity:
                mouseInfo.cursorover = traders;
                break;
            case MC_DERELICT:
                if ((mouseCursorObjPtr) && (((Derelict *)mouseCursorObjPtr)->derelicttype == HyperspaceGate))
                    mouseInfo.cursorover = enemy_ship;
                else
                    mouseInfo.cursorover = derelict;

                break;
            case MC_RESOURCE:
                mouseInfo.cursorover = resource;
                if (mouseCursorObjPtr && (mouseCursorObjPtr->attributes & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage)))
                {
                    if (mouseInfo.resource == 0)
                    {
                        mouseInfo.cursorover = enemy_ship;
                    }
                    else
                    {
                        //resouce collectors can't resource killer asteroids
                        mouseInfo.cursorover = MC_NO_SHIP;
                    }
                }
                break;
            case Ghostship:
            case MC_ENEMY_SHIP:
                mouseInfo.cursorover = enemy_ship;
                break;
            case MC_ALLIED_SHIP:
                mouseInfo.cursorover = allied_ship;
                break;
            default:
                if (mouseCursorObjPtr &&
                    ((ShipPtr)mouseCursorObjPtr)->playerowner == selSelected.ShipPtr[0]->playerowner)
                {
                    mouseInfo.cursorover = own_ship;
                }
        }

        //if a support ship is selected, and the mouse is over
        //a ship the support ship is capable of supporting, change
        //the mouse cursor
        if (mouseCursorObjPtr &&
            (bitTest(mouseInfo.flags, MCF_SpecialSCSupport) &&
             (isShipOfClass(mouseCursorObjPtr, CLASS_Fighter) ||
              isShipOfClass(mouseCursorObjPtr, CLASS_Corvette))))
        {
            mouseInfo.cursorover = support;
        }
    }
    //if there are ships being selected
    else if (selSelecting.numTargets)
    {
        bitSet(mouseInfo.flags, MCF_ShipSelecting);
    }
    else
    {
        if ((mouseCursorOverShipType == FloatingCity) ||
            ((mouseCursorOverShipType == MC_ALLIED_SHIP) &&
             (mouseCursorEnemyType == FloatingCity)))
        {
            mouseInfo.cursorover = traders;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mouseKeyCursorSetting
    Description : Sets the mouse cursor depending on key states and how the
                  mouseInfo structure is set.
    Inputs      : voidorama (uses globals)
    Outputs     : Calls mouseDrawType to change the mouse Draw type
    Return      : nothing
----------------------------------------------------------------------------*/
void mouseSetCursorSetting(void)
{
    if (!gameIsRunning)
    {
        return;
    }

    mouseDrawType(normal_mouse);

    //check if a double click task needs to be performed
    //mouse draw type will be set accordingly by this process
    if (mouseDoubleClickCheck())
    {
        return;
    }

    // if the player is disabled, only a few cursor bitmaps are shown
    if (mrDisabled)
    {
        if (keyIsHit(ALTKEY))
        {
            mouseDrawType(focus_no_select);
            return;
        }
        return;
    }

    if (!bitTest(mouseInfo.flags, MCF_DisableAll))
    {
        if (mouseInfo.ship_selected)
        {
            //forced attack
            if (keyIsHit(SHIFTKEY) && keyIsHit(CONTROLKEY))
            {
                mouseDrawType(force_attack);
                return;
            }
            //select multiple ships using the shift key
            if (keyIsHit(SHIFTKEY))
            {
                mouseDrawType(add_ships);
                return;
            }
            //guard ships
            if ((keyIsHit(CONTROLKEY) && keyIsHit(ALTKEY)) || keyIsHit(GKEY))
            {
                mouseDrawType(m_guard);
                return;
            }
            //activate/deactivate special attack
            if (kbCommandKeyIsHit(kbSHIP_SPECIAL))
            {
                if (bitTest(mouseInfo.flags, MCF_SpecialAttack))
                {
                    mouseDrawType(special_attack);
                    return;
                }
                else if (bitTest(mouseInfo.flags, MCF_SpecialSupport))
                {
                    mouseDrawType(support);
                    return;
                }
                else if (bitTest(mouseInfo.flags, MCF_Salvage))
                {
                    mouseDrawType(salvage);
                    return;
                }
            }
            //forced focus
            if (keyIsHit(ALTKEY))
            {
                mouseDrawType(focus_no_select);
                return;
            }
            //attack
            if (keyIsHit(CONTROLKEY))
            {
                if (mouseInfo.ship_selected != mouseInfo.resource)
                {
                    mouseDrawType(band_box_attack);
                }
                return;
            }

            //speed things up - if cursorover isn't a special
            //case, don't bother going through all these if statements
            if (mouseInfo.cursorover == MC_NO_SHIP)
            {
                return;
            }

            // if the mouse is over the traders
            if (mouseInfo.cursorover == traders)
            {
                mouseDrawType(traders);
            }
            //if a dockable ship is selected and the mouse cursor is over
            //a ship capable of accepting dockable ships
            else if ((bitTest(mouseInfo.flags, MCF_Dockable)) &&
                     (mouseInfo.cursorover == docking))
            {
                mouseDrawType(docking);
            }
            //same as above, but with small dockable ships (fighters & corvettes)
            else if ((bitTest(mouseInfo.flags, MCF_SmallDockable)) &&
                     (!bitTest(mouseInfo.flags, MCF_SupportOverItself)) &&
                     ((mouseInfo.cursorover == docking) || (mouseInfo.cursorover == small_docking)))
            {
                mouseDrawType(small_docking);
            }
            // if a support capable ship is selected and the mouse is over a ship
            else if (((mouseInfo.cursorover != enemy_ship) &&
                      (mouseInfo.cursorover != allied_ship) &&
                      (mouseInfo.cursorover != resource) &&
                      (mouseInfo.cursorover != derelict) &&
                      (mouseInfo.cursorover != traders)) &&
                     ((mouseInfo.cursorover == support) ||
                      (bitTest(mouseInfo.flags, MCF_SpecialSupport) &&
                       !bitTest(mouseInfo.flags, MCF_SupportOverItself))))
            {
                mouseDrawType(support);
            }
            //if the mouse is over a selected ship with a special operation and that
            //special operation has already been deployed
            else if ((bitTest(mouseInfo.flags, MCF_SpecialOpDeployed)) &&
                     (mouseInfo.cursorover == special_op_activate))
            {
                mouseDrawType(special_op_deactivate);
            }
            //if the mouse is over a selected ship with a special operation and that
            //special operation has no been deployed
            else if ((bitTest(mouseInfo.flags, MCF_SpecialOp)) &&
                     (mouseInfo.cursorover == special_op_activate))
            {
                mouseDrawType(special_op_activate);
            }
            //if the mouse is over a selected ship that has a GUI associated with it
            else if ((bitTest(mouseInfo.flags, MCF_GUICapable)) &&
                     (mouseInfo.cursorover == gui))
            {
                mouseDrawType(gui);
            }
            //if the mouse is over a selected build capable ship
            else if ((bitTest(mouseInfo.flags, MCF_BuildGUI)) &&
                     (mouseInfo.cursorover == build))
            {
                mouseDrawType(build);
            }
            //if the mouse is over a selected research ship
            else if ((bitTest(mouseInfo.flags, MCF_ResearchGUI)) &&
                     (mouseInfo.cursorover == research))
            {
                mouseDrawType(research);
            }
            //if the mouse is over a selected sensors array
            else if ((bitTest(mouseInfo.flags, MCF_SensorsGUI)) &&
                     (mouseInfo.cursorover == sensors))
            {
                mouseDrawType(sensors);
            }
            //if a resource ship is selected and the mouse is over a resource
            else if ((mouseInfo.resource) && (mouseInfo.cursorover == resource))
            {
                mouseDrawType(resource);
            }
            //if only salvage corvettes are selected and the mouse is over an enemy ship
            else if (bitTest(mouseInfo.flags, MCF_SalvageOnly) &&
                     ((mouseInfo.cursorover == enemy_ship) ||
                      (mouseInfo.cursorover == salvage)))
            {
                mouseDrawType(salvage);
            }
            //if a combat ship is selected and the mouse is over an enemy ship
            else if (!(bitTest(mouseInfo.flags, MCF_NonCombat)) &&
                     (mouseInfo.cursorover == enemy_ship))
            {
                mouseDrawType(band_box_attack);
            }
//            else if (mouseInfo.cursorover == allied_ship)
//            {
//                mouseDrawType(gui);
//            }
            //if a singleclickspecialactivate ship is selected and the mouse is over a derelict
            else if (bitTest(mouseInfo.flags, MCF_SingleClickSpecialActivate) &&
                     (mouseInfo.cursorover == derelict))
            {
                mouseDrawType(salvage);
            }
        }
        //if ships are being selected
        else if (bitTest(mouseInfo.flags, MCF_ShipSelecting) && keyIsHit(SHIFTKEY))
        {
            mouseDrawType(add_ships);
        }
        else
        {
            //forced focus
            if (keyIsHit(ALTKEY))
            {
                mouseDrawType(focus_no_select);
            }
            //if the mouse is over the trader ship
            else if (mouseInfo.cursorover == traders)
            {
                mouseDrawType(traders);
            }
        }
    }
}


/*-----------------------------------------------------------------------------
   Cursor Text Code
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        : mouseCursorTextDraw
    Description : Draws the name of the ship type associated with mouseCursorOverShipType
    Inputs      : void
    Outputs     : Screen text roughly resembling the name of the ship type
    Return      :
----------------------------------------------------------------------------*/
void mouseCursorTextDraw(void)
{
    static sdword cursortexttype = MC_NO_SHIP;
    fonthandle fhSave;

    if ((!mrRenderMainScreen) || (!gameIsRunning) || (mrWhiteOut) || (nisIsRunning) || (keyIsHit(RMOUSE_BUTTON)))
    {
        return;
    }

    // if there's something in that cool global variable
    if ((mouseCursorOverShipType != MC_NO_SHIP) && (mouseCursorOverShipType != MC_RESOURCE) && (mouseCursorOverShipType != MC_DERELICT))
    {
        cursortexttype = mouseCursorOverShipType;
    }
    else
    {
        if (mouseCursorTextExpire < universe.totaltimeelapsed)
        {
            cursortexttype = MC_NO_SHIP;
        }
    }

    if (cursortexttype != MC_NO_SHIP)
    {
        // display that baby!
        fhSave = fontMakeCurrent(mouseCursorFont);

        if (cursortexttype == MC_ENEMY_SHIP)
        {
            if (mouseCursorOverLODs & MOUSE_IdentifyLODMask)
            {                                               //if it's been seen closely
                if (!singlePlayerGame)
                {
                    fontPrintf(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR, "%s's %s",
                              playerNames[mouseCursorEnemyPlayer], ShipTypeToNiceStr(mouseCursorEnemyType));
                }
                else
                {
                    fontPrintf(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR,
                               ShipTypeToNiceStr(mouseCursorEnemyType));
                }
            }
            else
            {
                fontPrint(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR,
                          strGetString(strEnemyShip));
            }
        }
        else if (cursortexttype == MC_ALLIED_SHIP)
        {
            if (!singlePlayerGame)
            {
                fontPrintf(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR, "%s's %s",
                          playerNames[mouseCursorEnemyPlayer], ShipTypeToNiceStr(mouseCursorEnemyType));
            }
            else
            {
                // allied ships in the single player game (defector and traders) only
                // display their shiptypes and not the player name
                fontPrint(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR,
                           ShipTypeToNiceStr(mouseCursorEnemyType));
            }
        }
        else
        {
            fontPrint(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR,
                       ShipTypeToNiceStr(cursortexttype));
        }


        fontMakeCurrent(fhSave);
    }
}


/*-----------------------------------------------------------------------------
    Name        : mouseClipPointToRect
    Description : Clip an x/y coordinate to a rectangle.
    Inputs      : x, y - position to clip
                  rect - rectangle to clip to
    Outputs     : clips x, y to rect
    Return      :
----------------------------------------------------------------------------*/
void mouseClipPointToRect(sdword *x, sdword *y, rectangle *rect)
{
    if (*x < rect->x0)
    {
        *x = rect->x0;
    }
    if (*x >= rect->x1)
    {
        *x = rect->x1 - 1;
    }
    if (*y < rect->y0)
    {
        *y = rect->y0;
    }
    if (*y >= rect->y1)
    {
        *y = rect->y1 - 1;
    }
}

/*=============================================================================
    Name    : mousePoll
    Purpose : Poll current mouse location and store internally

    Created 7/7/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
void mousePoll(void)
{
    rectangle clientRect;
    //POINT mousePoint;

    if (mouseDisabled)
    {
        return;
    }
    if (demDemoPlaying)
    {
        return;
    }
    utyClientRectGet(&clientRect);                          //get window location
    /*
    GetCursorPos(&mousePoint);                              //get location of Windows mouse cursor
    mouseCursorXPosition = mousePoint.x - clientRect.x0;    //compute relative coords
    mouseCursorYPosition = mousePoint.y - clientRect.y0;
    */
    SDL_GetMouseState(&mouseCursorXPosition, &mouseCursorYPosition);

    if (mouseClip)
    {
        mouseClipPointToRect(&mouseCursorXPosition, &mouseCursorYPosition, &mouseClipRect);
        mousePositionSet(mouseCursorXPosition, mouseCursorYPosition);
    }
    //perform client area enter/exit logic to hide/show Windows system cursor
    /*
    if (primPointInRectXY2(&clientRect, mousePoint.x, mousePoint.y))
    {                                                       //mouse inside window's client area
        if (!mouseInsideClient)
        {                                                   //just entered region
            //ShowCursor(FALSE);
            SDL_ShowCursor(SDL_DISABLE);
            mouseInsideClient = TRUE;
        }
    }
    else
    {                                                       //else outside region
        if (mouseInsideClient)
        {                                                   //just exited region
            //ShowCursor(TRUE);
            SDL_ShowCursor(SDL_ENABLE);
            mouseInsideClient = FALSE;
        }
    }
    */
}

/*-----------------------------------------------------------------------------
    Name        : mouseDisable/mouseEnable
    Description : Disable and re-enable the mouse
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mouseDisable(void)
{
    mouseCursorHide();
    mouseDisabled = TRUE;
}
void mouseEnable(void)
{
    mouseCursorShow();
    mouseDisabled = FALSE;
}


