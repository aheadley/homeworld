/*=============================================================================
    Name    : mainrgn.c
    Purpose : Logic for handling region events for the main game screen.

    Created 6/27/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#undef UTY_SCREEN_SHOT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "glinc.h"
#include <math.h>
#include <float.h>
#include "switches.h"
#include "types.h"
#include "fastmath.h"
#include "region.h"
#include "feflow.h"
#include "debug.h"
#include "render.h"
#include "camera.h"
#include "cameracommand.h"
#include "mouse.h"
#include "utility.h"
#include "prim2d.h"
#include "prim3d.h"
#include "select.h"
#include "universe.h"
#include "univupdate.h"
#include "soundevent.h"
#include "consmgr.h"
#include "trademgr.h"
#include "CommandWrap.h"
#include "Undo.h"
#include "tactical.h"
#include "globals.h"
#include "font.h"
#include "sensors.h"
#include "teams.h"
#include "mainrgn.h"
#include "flightman.h"
#include "particle.h"
#include "etg.h"
#include "NumberDefs.h"
#include "nis.h"
#include "netcheck.h"
#include "tweak.h"
#include "main.h"
#include "fontreg.h"
#include "probe.h"
#include "gun.h"
#include "pieplate.h"
#include "researchship.h"
#include "launchMgr.h"
#include "researchgui.h"
#include "tactics.h"
#include "soundlow.h"
#include "InfoOverlay.h"
#include "glcaps.h"
#include "singleplayer.h"
#include "KAS.h"
#include "gamechat.h"
#include "meshanim.h"
#include "chatting.h"
#include "Alliance.h"
#include "shader.h"
#include "Objectives.h"
#include "KASFunc.h"
#include "tutor.h"
#include "strings.h"
#include "options.h"
#include "taskbar.h"
#include "shipview.h"
#include "AIvar.h"
#include "gamepick.h"
#include "battle.h"
#include "Captaincy.h"
#include "Commandnetwork.h"
#include "bink.h"
#include "KeyBindings.h"

/*=============================================================================
    Data:
=============================================================================*/

//global flag controlling rendering of main game screen
sdword mrRenderMainScreen = TRUE;

//camera for main screen
Camera *mrCamera = NULL;

bool mrWhiteOut = FALSE;
real32 mrWhiteOutT = 0.0f;

//rectangle of currently dragged selection
rectangle mrSelectionRect;
sdword mrOldMouseX, mrOldMouseY;

//logic handlers for main region
void mrNULL(void);
void (*mrHoldLeft)(void) = mrNULL;
void (*mrHoldRight)(void) = mrNULL;

//flag indicating mouse has moved since right mouse button pressed
sdword mrMouseHasMoved;
sdword mrMouseMovementClickLimit = MR_MouseMovementClickLimit;

//global handle to the main region
regionhandle ghMainRegion = NULL;

//tactical overlay on/off flag
bool mrDrawTactical = FALSE, mrSaveTactical = FALSE;

//help ifno screen up or not
bool helpinfoactive = FALSE;

regionhandle helpinforegion;

//debug for turning off enemy AI
bool mrNoAI = FALSE;

//fonthandle for Big Font
static fonthandle mrBigFont = 0;

//key definitions
struct
{
    keyindex key[REG_NumberKeys];
    ubyte nKeys;
    udword filter;
}
mrKeyFunction[] =
{
    {{ESCKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{TABKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{ARRLEFT,   0,      0,      0}, 1, RPE_KeyDown},
    {{ARRRIGHT,  0,      0,      0}, 1, RPE_KeyDown},
    {{BKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{AKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{CKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{DKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{EKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{FKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{RKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{MMOUSE_BUTTON,0,   0,      0}, 1, RPE_KeyDown},
    {{MMOUSE_DOUBLE,0,   0,      0}, 1, RPE_KeyDown},
    {{HKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{IKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{JKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{KKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{LKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{MKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{LBRACK,    0,      0,      0}, 1, RPE_KeyDown},
    {{RBRACK,    0,      0,      0}, 1, RPE_KeyDown},
    {{SKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{VKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{ZKEY,      0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{CAPSLOCKKEY,0,     0,      0}, 1, RPE_KeyDown},
    {{F1KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F2KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F3KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F4KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{TILDEKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{ZEROKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{ONEKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{TWOKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{THREEKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{FOURKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{FIVEKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{XKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{SIXKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{SEVENKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{EIGHTKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{NINEKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{SPACEKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{ENTERKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{F5KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F6KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F7KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F8KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F9KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F10KEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{F11KEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{HOMEKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{PAGEDOWNKEY,0,     0,      0}, 1, RPE_KeyDown},
    {{PAGEUPKEY, 0,      0,      0}, 1, RPE_KeyDown},
#if UNIVERSE_TURBOPAUSE_DEBUG
    {{BACKSLASHKEY,      0,      0,      0}, 1, RPE_KeyDown},
#endif
    {{PKEY,              0,      0,      0}, 1, RPE_KeyDown},
#if MR_TEST_GUNS
    {{TKEY,      0,      0,      0}, 1, RPE_KeyDown},
#else
    {{TKEY,      0,      0,      0}, 1, RPE_KeyDown},
#endif

#if GUN_TUNE_MODE
    {{YKEY,      0,      0,      0}, 1, RPE_KeyDown},
#endif
/*
#if MR_SCREENSHOTS
    {{PAUSEKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{SCROLLKEY, 0,      0,      0}, 1, RPE_KeyDown},
#endif
*/
    {{SHIFTKEY,  0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
#if ETG_RELOAD_KEY
    {{ETG_RELOAD_KEY,0,  0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
#endif
#if NIS_TEST
    {{NUMPAD1,   0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
#endif
#if MR_TEST_HPB
    {{OKEY,   0,      0,      0}, 1, RPE_KeyDown},
#endif
    {{NUMPLUSKEY,   0,      0,      0},     1,  RPE_KeyDown | RPE_KeyUp},
    {{PLUSKEY,      0,      0,      0},     1,  RPE_KeyDown | RPE_KeyUp},
    {{NUMMINUSKEY,  0,      0,      0},     1,  RPE_KeyDown | RPE_KeyUp},
    {{MINUSKEY,     0,      0,      0},     1,  RPE_KeyDown | RPE_KeyUp},

// Keys for music selection in multiplayer game
    {{LESSTHAN    , 0,      0,      0}, 1, RPE_KeyDown},
    {{GREATERTHAN , 0,      0,      0}, 1, RPE_KeyDown},

// Extra keys that are mappable that the mainregion must capture
#ifndef HW_Debug  //only add these for release builds so that random debug keys will work
    {{PAUSEKEY    , 0,      0,      0}, 1, RPE_KeyDown},
    {{PRINTKEY    , 0,      0,      0}, 1, RPE_KeyDown},
    {{ARRUP       , 0,      0,      0}, 1, RPE_KeyDown},
    {{ARRDOWN     , 0,      0,      0}, 1, RPE_KeyDown},
    {{ENDKEY      , 0,      0,      0}, 1, RPE_KeyDown},
    {{INSERTKEY   , 0,      0,      0}, 1, RPE_KeyDown},
    {{DELETEKEY   , 0,      0,      0}, 1, RPE_KeyDown},
    {{NKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{OKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{QKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{UKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{WKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{XKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{YKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD0     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD1     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD2     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD3     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD4     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD5     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD6     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD7     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD8     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMPAD9     , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMSTARKEY  , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMSLASHKEY , 0,      0,      0}, 1, RPE_KeyDown},
    {{NUMDOTKEY   , 0,      0,      0}, 1, RPE_KeyDown},
#else
    {{UKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{YKEY        , 0,      0,      0}, 1, RPE_KeyDown},
    {{QKEY        , 0,      0,      0}, 1, RPE_KeyDown},
#endif

    {{0,         0,      0,      0}, 0, RPE_KeyDown}
};

//stuff for changing formations
#define PARADE_FORMATION_FLAG   ((udword)-1)
TypeOfFormation mrNewFormation = 0;                 //index of current formation we're selecting
char *mrFormationName = NULL;                   //if non-NULL, we're in the changing formations state
real32 mrFormationTime;                         //time a particular formation was set
bool   mrDrawFormation=FALSE;
real32 mrDrawFormationTime=0.0;
fonthandle mrFormationFont = FONT_InvalidFontHandle;

#if MAD_TEST_ANIMATION
sdword mrTestAnimationIndex = 0;                //index of mesh animation we're testing
#endif

// for kas text display
extern sdword popupTextNumLines;

// number indicates which player was right clicked on in the player list
sdword playerClickedOn=-1;

//for handling bot z-bandbox and just pressing z
bool mrSpecialBandBox;

//for double-press number keys to focus on ships
real32 mrNumberDoublePressTime = MR_NumberDoublePressTime;
sdword mrLastKeyPressed = -1;
real32 mrLastKeyTime = 0;

//to help clicking on fast-moving ships:
real32 mrFastMoveShipClickTime = MR_FastMoveShipClickTime;
sdword mrFastMoveShipClickX = MR_FastMoveShipClickX;
sdword mrFastMoveShipClickY = MR_FastMoveShipClickY;

#if MR_CAN_FOCUS_ROIDS
bool mrCanFocusRoids = FALSE;
#endif

bool mrMenuDontDisappear = FALSE;       //forces the context menus to stay on the screen

#define TIMEOUT_TIME 15.0 // timer for the printing of player dropped messages

#ifdef HW_Debug
extern real32 pilotupoffset;
#endif

/*=============================================================================
    Private Function prototypes:
=============================================================================*/

//sdword mrClickedOnPlayer(void);
void mrSetTheFormation(TypeOfFormation formationtype);

/*=============================================================================
    Functions:
=============================================================================*/

static udword originalMrFilter;
bool   mrDisabled = FALSE;

void mrDisable(void)
{
    if (!mrDisabled)
    {
        dbgAssert(ghMainRegion);
        originalMrFilter = ghMainRegion->flags;
    //    regFilterSet(ghMainRegion, 0);
        regFilterSet(ghMainRegion, RPM_MouseEvents);
        mrDisabled = TRUE;
    }
}

void mrEnable(void)
{
    if (mrDisabled)
    {
        dbgAssert(ghMainRegion);
        regFilterSet(ghMainRegion,originalMrFilter);
        mrDisabled = FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrNeedProbeHack()
    Description : checks if a single probe is selcted and if so returns true,
                  otherwise removes it from the selection.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool mrNeedProbeHack(void)
{
    if(selSelected.numShips == 1)
    {
        if(selSelected.ShipPtr[0]->shiptype == Probe)
        {    //woo hoo!  1 ship selected and its a probe
            ProbeSpec *spec;
            spec = (ProbeSpec *) selSelected.ShipPtr[0]->ShipSpecifics;
            //no longer do checks for if a probe has moved..allow infinite moves!
            if(!spec->HaveMoved)
            {

                return TRUE;
            }
            else
            {
                //fix later
                //should give SOUND EVENT to indicate probe is no good anymore
                soundEvent(NULL, UI_ClickCancel);
            }
        }
    }
    return FALSE;
}
/*-----------------------------------------------------------------------------
    Name        : mrRemoveAllProbesFromSelection
    Description : removes any probes from the current selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrRemoveAllProbesFromSelection()
{
    sdword i;
    for(i=0;i<selSelected.numShips;)
    {
        if(selSelected.ShipPtr[i]->shiptype == Probe)
        {
            selSelected.numShips--;
            selSelected.ShipPtr[i] = selSelected.ShipPtr[selSelected.numShips];
            continue;
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : salCapCorvetteInSelection
    Description : returns TRUE if a salcap corvette is in the selection
                  otherwise returns false
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool salCapCorvetteInSelection(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[i]->specialFlags & SPECIAL_IsASalvager)
        {
            return TRUE;
        }
    }
    return FALSE;
}
/*-----------------------------------------------------------------------------
    Name        : mrRemoveDerelictsAndAllPlayerShipsFromSelecting
    Description : removes any of player's own ships from the current selection
                  also removes any drones from selection too
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrRemoveDerelictsAndAllPlayerShipsFromSelecting()
{
    sdword i;
    SpaceObjRotImpTarg *target;
    for(i=0;i<selSelecting.numTargets;)
    {
        target = selSelecting.TargetPtr[i];
        if (target->objtype == OBJ_DerelictType)
        {
            if (((Derelict *)target)->derelicttype != HyperspaceGate)
            {
                if(!salCapCorvetteInSelection((SelectCommand *)&selSelected))
                {
                    //special case luke suggested hack
                    goto removeit;
                }
            }
        }
        if (target->objtype == OBJ_ShipType)
        {
            if ((((Ship *)target)->playerowner == universe.curPlayerPtr) ||
                (allianceIsShipAlly((Ship *)target, universe.curPlayerPtr)) ||
                (((Ship *)target)->shiptype == Drone))
            {
removeit:
                selSelecting.numTargets--;
                selSelecting.TargetPtr[i] = selSelecting.TargetPtr[selSelecting.numTargets];
                continue;
            }
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        :  mrProbeHack
    Description :  Called when a single probe was selected and the Movement mech
                    was being started...
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrProbeHack()
{
    if(!smSensorsActive)
    {   //start sensors manager if it isn't active already
        smSensorsBegin(NULL, NULL);     //step one complete!
    }
    if(!smSensorsDisable && !smZoomingIn)
    {   //if sensors aren't disabled AND we aren't zooming in
        piePointModeOnOff();
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrStartup
    Description : Start main region processor logic
    Inputs      : void
    Outputs     : Many many things...
    Return      : void
----------------------------------------------------------------------------*/
void mrStartup(void)
{
    sdword index;
    ghMainRegion = regChildAlloc(NULL, 0, 0, 0,             //create main region
                   MAIN_WindowWidth, MAIN_WindowHeight, 0,
                   RPE_LeftClickButton | RPE_HoldLeft |
                   RPE_RightClickButton | RPE_HoldRight |
                   RPE_WheelUp | RPE_WheelDown | RPE_DoubleLeft | RPE_DoubleCentre);
    regDrawFunctionSet(ghMainRegion, mrRegionDraw);         //set it's functions
    regFunctionSet(ghMainRegion, mrRegionProcess);

    for (index = 0; mrKeyFunction[index].nKeys != 0; index++)
    {                                                       //now set all the keys
        regKeyChildAlloc(ghMainRegion, mrKeyFunction[index].key[0], mrKeyFunction[index].filter,
                         mrRegionProcess, mrKeyFunction[index].nKeys,
                         mrKeyFunction[index].key[0], mrKeyFunction[index].key[1],
                         mrKeyFunction[index].key[2], mrKeyFunction[index].key[3]);
    }

    mrBigFont = frFontRegister("Arial_b17.hff");
    mrFormationFont = frFontRegister("default.hff");
}

/*-----------------------------------------------------------------------------
    Name        : mrShutdown
    Description : Shuts down main region
    Inputs      : void
    Outputs     : Destroys ghMainRegion and all child windows
    Return      : void
----------------------------------------------------------------------------*/
void mrShutdown(void)
{
    regRegionDelete(ghMainRegion);
}

/*-----------------------------------------------------------------------------
    Callback functions for right-click menus:
-----------------------------------------------------------------------------*/
void mrDockingOrders(char *string, featom *atom)
{
    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bDock)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    makeShipsDockCapable((SelectCommand *)&selSelected); //filter ships that can't dock

    if (selSelected.numShips > 0)
    {
        soundEvent(NULL, UI_Click);
        clWrapDock(&universe.mainCommandLayer,(SelectCommand *)&selSelected,dockGetAppropriateTypeOfDocking((SelectCommand *)&selSelected),NULL);
    }
    else
    {
        soundEvent(NULL, UI_ClickCancel);
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nDocking orders");
#endif
}
void mrDeltaFormation(char *string, featom *atom)
{
    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormDelta)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(DELTA_FORMATION);
}
void mrBroadFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormBroad)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(BROAD_FORMATION);
}

void mrXFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormX)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(DELTA3D_FORMATION);
}

void mrClawFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormClaw)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(CLAW_FORMATION);
}

void mrWallFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormWall)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(WALL_FORMATION);
}

void mrSphereFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormSphere)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(SPHERE_FORMATION);
}

void mrPicketFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormCustom)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(CUSTOM_FORMATION);
}

void mrHarvestResources(char *string, featom *atom)
{
    MaxSelection tempSelection;
    Resource *nearestresource;

    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bHarvest)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    if (selSelected.numShips > 0)
    {
        if (MakeShipsHarvestCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected))
        {
            nearestresource = univFindNearestResource(tempSelection.ShipPtr[0],0,NULL);
            if (nearestresource != NULL)
            {
                soundEvent(NULL, UI_ClickAccept);
                speechEvent(tempSelection.ShipPtr[0], COMM_ResCol_Harvest, (nearestresource->objtype - OBJ_AsteroidType));
                clWrapCollectResource(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,NULL);
                soundEventStartEngine(tempSelection.ShipPtr[0]);
            }
            else
            {
                soundEvent(NULL, UI_ClickCancel);
                speechEvent(tempSelection.ShipPtr[0], COMM_ResCol_NoMoreRUs, 0);
            }
        }
    }
}
void mrBuildShips(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled)) return;

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildManager)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    if ((atom != NULL) && ((regionhandle)atom->region != NULL))
    {
        bitClear(((regionhandle)atom->region)->status,RSF_CurrentSelected);
    }

    soundEvent(NULL, UI_Click);
    if (selSelected.numShips == 1)
    {
        cmConstructionBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
    }
    else
    {
        cmConstructionBegin(ghMainRegion, 0, 0, 0);
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nBuild ships.");
#endif
}

void mrTradeStuff(char *string, featom *atom)
{

        tmTradeBegin(ghMainRegion, 0, 0, 0);

#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nTrading.");
#endif
}





void mrMoveShips(char *string, featom *atom)
{
    vector dist;

    if ((playPackets) || (mrDisabled))
    {
        return;
    }

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bMove)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    makeShipsNotIncludeSinglePlayerMotherships((SelectCommand *)&selSelected);
    makeShipsControllable((SelectCommand *)&selSelected,COMMAND_MOVE);
    if(mrNeedProbeHack())                                   //performs needed stuff for probe hack and
    {                                                       //removes probes from selection if other ships there!
        mrProbeHack();
    }
    else
    {
        if (mrHoldRight != mrNULL || mrHoldLeft != mrNULL)
        {
            return;
        }
        else if (selSelected.numShips >= 1)
        {   //single probe not selected so do normal movement....
    //        universe.mainCameraCommand.actualcamera
            selCentrePointCompute();
            vecSub(dist,selCentrePoint,universe.mainCameraCommand.actualcamera.lookatpoint);
            if ((piePointSpecMode!=PSM_Idle)||(vecMagnitudeSquared(dist) <= MAX_MOVE_DISTANCE))
            {
                if (piePointSpecMode!=PSM_Idle)
                {
                    soundEvent(NULL, UI_MovementGUIoff);
                }
                mrRemoveAllProbesFromSelection();
                piePointModeOnOff();
            }
            else
            {
                if (piePointSpecMode==PSM_Idle)
                {
                    smSensorsBegin(NULL,NULL);
                    piePointModeOnOff();
                }
            }
        }
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nMove ships.");
#endif
}
void mrInfo(char *string, featom *atom)
{
    dbgMessagef("\nInfo - stubbed out.");
}
void mrCancel(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bCancelCommand)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    if (selSelected.numShips > 0)
    {
        clWrapHalt(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nAll stop - cancel orders.");
#endif
}
void mrScuttle(char *string, featom *atom)
{
    static real32 confirmTime=-7.0f;    //initial value

    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bScuttle)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    if(selSelected.numShips == 0)
        return;

    if (!singlePlayerGame)
    {
        if (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip))
        {
            MakeShipsNonCapital((SelectCommand *)&selSelected);
            if(selSelected.numShips > 0)
            {
                if (battleCanChatterAtThisTime(BCE_CannotComply, selSelected.ShipPtr[0]))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CannotComply, selSelected.ShipPtr[0], SOUND_EVENT_DEFAULT);
                }
                return;     // don't allow scuttling on levels where you have to capture ships!
            }
        }
    }
    if (selSelected.numShips > 0)
    {
        if((universe.totaltimeelapsed - confirmTime) < TW_ScuttleReconfirmTime)
        {
            tutGameMessage("Game_ScuttleConfirm");

            soundEvent(NULL, UI_Click);
            confirmTime = -TW_SCUTTLE_CONFIRM_TIME;
            MakeShipMastersIncludeSlaves((SelectCommand *)&selSelected);
            clWrapScuttle(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
#if MR_VERBOSE_LEVEL >= 1
        dbgMessage("\nScuttle.");
#endif
        }
        else
        {
            /////////////////
            //reconfrim speech event:  FLEET Event
            //event num: COMM_F_ScuttleReaffirm
            if(battleCanChatterAtThisTime(BCE_COMM_F_ScuttleReaffirm, selSelected.ShipPtr[0]))
            {
                battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_COMM_F_ScuttleReaffirm, SOUND_EVENT_DEFAULT, &selSelected.ShipPtr[0]->posinfo.position);
            }
            //speechEventFleet(COMM_F_ScuttleReaffirm,0,universe.curPlayerIndex);
            ////////////////////
            tutGameMessage("Game_ScuttleRequest");
            confirmTime = universe.totaltimeelapsed;
#if MR_VERBOSE_LEVEL >= 1
        dbgMessage("\nScuttle. Reconfirm.");
#endif
        }
    }
}
void mrRetire(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bRetire)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    if(selSelected.numShips == 0)
        return;

    if (!singlePlayerGame)
    {
        if (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip))
        {
            //filter out capital ships
            MakeShipsNonCapital((SelectCommand *)&selSelected);
            if(selSelected.numShips > 0)
            {
                if (battleCanChatterAtThisTime(BCE_CannotComply, selSelected.ShipPtr[0]))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CannotComply, selSelected.ShipPtr[0], SOUND_EVENT_DEFAULT);
                }
                return;     // don't allow scuttling on levels where you have to capture ships!
            }
        }
    }
    makeShipsRetireable((SelectCommand *)&selSelected); //filter ships that can't dock
    if (selSelected.numShips > 0)
    {
        soundEvent(NULL, UI_Click);
        MakeShipMastersIncludeSlaves((SelectCommand *)&selSelected);
        clWrapDock(&universe.mainCommandLayer,(SelectCommand *)&selSelected,DOCK_FOR_RETIRE,NULL);
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nRetire orders");
#endif
}
void mrUpdateHyperspaceStatus(bool goForLaunch)
{
}
void mrHyperspace(char *string, featom *atom)
{
    extern bool uicButtonReleased;
    if (FEFIRSTCALL(atom))
    {
        //...
    }
    else if (FELASTCALL(atom) && !uicButtonReleased)
    {
        //...
    }
    else
    {
        if (singlePlayerGame)
        {
            spHyperspaceButtonPushed();
        }
    }
}
void mrLaunch(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((atom != NULL) && ((regionhandle)atom->region != NULL))
    {
        bitClear(((regionhandle)atom->region)->status,RSF_CurrentSelected);
    }

    soundEvent(NULL, UI_Click);

    if (selSelected.numShips == 1)
    {
        lmLaunchBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
    }
    else
    {
        lmLaunchBegin(ghMainRegion, 0, 0, 0);
    }

    dbgMessagef("\nLaunch Ships.");
}
void mrResearch(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((atom != NULL) && ((regionhandle)atom->region != NULL))
    {
        bitClear(((regionhandle)atom->region)->status,RSF_CurrentSelected);
    }

    soundEvent(NULL, UI_Click);

    rmResearchGUIBegin(ghMainRegion, 0, 0, 0);

#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nResearch.");
#endif
}
void mrEvasiveTactics(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bEvasive)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    if (selSelected.numShips > 0)
    {
        speechEvent(selSelected.ShipPtr[0], COMM_TacticsEvasive, 0);
    }
    clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,Evasive);

    dbgMessagef("\nEvasive Tactics - stubbed out.");
}
void mrNeutralTactics(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bNeutral)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    if (selSelected.numShips > 0)
    {
        speechEvent(selSelected.ShipPtr[0], COMM_TacticsNeutral, 0);
    }
    clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,Neutral);

    dbgMessagef("\nNeutral Tactics - stubbed out.");
}
void mrAgressiveTactics(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bAgressive)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    if (selSelected.numShips > 0)
    {
        speechEvent(selSelected.ShipPtr[0], COMM_TacticsAggressive, 0);
    }
    clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,Aggressive);

    dbgMessagef("\nAgressive Tactics - stubbed out.");
}

void mrFormAlliance(char *string, featom *atom)
{
    if (playerClickedOn!=-1)
    {
        allianceFormWith(playerClickedOn);
    }
}

void mrBreakAlliance(char *string, featom *atom)
{
    if (playerClickedOn!=-1)
    {
        allianceBreakWith(playerClickedOn);
    }
}

void mrTransferRUS(char *string, featom *atom)
{
    if (playerClickedOn!=-1)
    {
        gcRUTransferStart((uword)playerClickedOn);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrSelectRectBuild
    Description : builds a selection rect based upon mouse coords
    Inputs      : dest - where to put the result
                  anchorX, anchorY - coordinates of the 'anchor' point of
                    the selection rect
    Outputs     : fills in mrSelectionRect
    Return      : void
----------------------------------------------------------------------------*/
void mrSelectRectBuild(rectangle *dest, sdword anchorX, sdword anchorY)
{
    dest->x0 = anchorX;                                     //build selection rect
    dest->y0 = anchorY;
    dest->x1 = mouseCursorX();
    dest->y1 = mouseCursorY();
    if (dest->x0 > dest->x1)                                //make sure first corner above and left of second corner
    {
        swapInt(dest->x0, dest->x1);
    }
    if (dest->y0 > dest->y1)
    {
        swapInt(dest->y0, dest->y1);
    }
    dest->x1++;                                             //make sure rect draws inclusively
    dest->y1++;                                             // to lower right corner
}

/*-----------------------------------------------------------------------------
    Name        : mrSelectionUndo
    Description : Undo callback to reset selection
    Inputs      : length, userData - MaxSelection substructure
    Outputs     : Copies a selection of length userID to selSelected
    Return      : TRUE if successful
----------------------------------------------------------------------------*/
/*
bool mrSelectionUndo(sdword userID, ubyte *userData, sdword length)
{
    selSelectionCopy((MaxAnySelection *)&selSelected, (MaxAnySelection *)userData);
    ioUpdateShipTotals();
    return TRUE;
}
*/
/*-----------------------------------------------------------------------------
    Name        : mrHotKeyAssignUndo
    Description : Undo callback to reset selection
    Inputs      : length, userData - MaxSelection substructure
                  userID - hot key #
    Outputs     : Copies a selection of length userID to selSelected
    Return      :
----------------------------------------------------------------------------*/
/*
bool mrHotKeyAssignUndo(sdword userID, ubyte *userData, sdword length)
{
    selHotKeyNumbersSet(&selHotKeyGroup[userID], SEL_InvalidHotKey);
    selSelectionCopy((MaxAnySelection *)&selHotKeyGroup[userID], (MaxAnySelection *)userData);
    selHotKeyNumbersSet(&selHotKeyGroup[userID], selHotKeyNumber(userID));
    return TRUE;
}
*/
/*-----------------------------------------------------------------------------
    Name        : mrCanZBandBox
    Description : Determine if we can z-bandbox right now
    Inputs      : bFriendlies - we can bandbox friendlies
                  bEnemies - we can bandbox enemies
    Outputs     :
    Return      : we can bandbox at all
----------------------------------------------------------------------------*/
bool mrCanZBandBox(bool *bFriendlies, bool *bEnemies)
{
    sdword index ;
    ShipStaticInfo *shipstatic;
    *bEnemies = *bFriendlies = FALSE;
    if (selSelected.numShips == 0)
    {
        return(FALSE);
    }
    for (index = 0; index < selSelected.numShips; index++)
    {
        shipstatic = (ShipStaticInfo *)selSelected.ShipPtr[index]->staticinfo;
        if (shipstatic->custshipheader.CustShipSpecialTarget != NULL)
        {
            if (shipstatic->canSpecialBandBoxFriendlies)
            {
                *bFriendlies = TRUE;
                return(TRUE);
            }
            else
            {
                *bEnemies = TRUE;
                return(TRUE);
            }
        }
    }
    return(*bFriendlies || *bEnemies);
}

/*-----------------------------------------------------------------------------
    Name        : various
    Description : Main region processor functions, called by function pointer
    Inputs      : void
    Outputs     : various
    Return      : void
----------------------------------------------------------------------------*/
//do nothing
void mrNULL(void)
{
    ;                                                       //do nothing
}
//drag the selection box
void mrSelectHold(void)
{
    bool bFriendlies, bEnemies;
    MaxSelection tempSelection;

    if (abs(mouseCursorX() - mrOldMouseX) >= selClickBoxWidth ||
        abs(mouseCursorY() - mrOldMouseY) >= selClickBoxHeight)
    {                                                       //if mouse has moved from anchor point
        mrSelectRectBuild(&mrSelectionRect, mrOldMouseX, mrOldMouseY);//create a selection rect
        if (keyIsHit(GKEY) || (keyIsHit(CONTROLKEY) && keyIsHit(ALTKEY)))
        {                                                   //guard mode
            selRectDrag(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
            if ((playPackets) || (universePause) || (mrDisabled))
            {                           // if in recorded packet playback then can't issue a gaurd order.
                selRectNone();
            }
        }
        else if (keyIsHit(CONTROLKEY))
        {
            if (keyIsHit(SHIFTKEY))
            {                                               //ctrl-shift - select anything targetable
                selRectDragAnythingToAttack(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
                if ((playPackets) || (universePause) || (mrDisabled))
                {                           // if in recorded packet playback then can't issue a gaurd order.
                    selRectNone();
                }
            }
            else
            {                                               //ctrl - select all enemies
                selRectDragAnythingToAttack(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
                if ((playPackets) || (universePause) || (mrDisabled))
                {                           // if in recorded packet playback then can't issue a gaurd order.
                    selRectNone();
                }

                if (!MakeShipsSingleClickSpecialCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected))
                {                                           //if no ships single-click special attack capable
                    mrRemoveDerelictsAndAllPlayerShipsFromSelecting();
                }

                if (ShiptypeInSelection((SelectCommand *)&selSelected, SalCapCorvette))
                {
                    MakeTargetsSalvageable((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);
                }
                else
                {
                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);//always remove allied/player ships from selection
                }
            }
        }
        else if (keyIsHit(ALTKEY))
        {                                                   //alt - select any ships
            selRectDragAnybody(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
        }
        else if (kbCommandKeyIsHit(kbSHIP_SPECIAL))
        {                                                   //special action modifier
            if (mrCanZBandBox(&bFriendlies, &bEnemies))
            {
                if ((playPackets) || (universePause) || (mrDisabled))
                {                           // if in recorded packet playback then can't issue a gaurd order.
                    selRectNone();
                }

                if (bEnemies)
                {                                           //select enemies and friends
                    selRectDragAnybody(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
                }
                else
                {                                           //select just friends
                    selRectDrag(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
                }
                if (!bFriendlies)
                {                                           //remove friends from selection
                    if (ShiptypeInSelection((SelectCommand *)&selSelected, SalCapCorvette))
                    {
                        MakeTargetsSalvageable((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);
                    }
                    else
                    {
                        mrRemoveDerelictsAndAllPlayerShipsFromSelecting();
                    }
                }
            }
            else
            {
                selRectNone();
            }
        }
        else
        {                                                   //select player's ships
            selRectDrag(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
        }
    }
    else
    {                                                       //else mouse hasn't moved far enough
        mrSelectionRect.x0 = mrSelectionRect.x1 =           //select nothing
            mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
        selRectNone();
    }
}
//rotate and/or zoom the camera
void mrCameraMotion(void)
{
    rectangle rect;
    sdword cx, cy;
    if (helpinfoactive)
    {
        if (!mouseInRect(&helpinforegion->rect))
        {

            cx = helpinforegion->rect.x0 / 2;
            cy = MAIN_WindowHeight / 2;

            if (mouseCursorX() != cx ||
            mouseCursorY() != cy)
            {
                mrMouseHasMoved += abs(mouseCursorX() - cx);
                mrMouseHasMoved += abs(mouseCursorY() - cy);
                camMouseX = cx - mouseCursorX();
                camMouseY = cy - mouseCursorY();
                mousePositionSet(cx, cy);

                rect.x0 = 0;
                rect.y0 = 0;
                rect.x1 = helpinforegion->rect.x0;
                rect.y1 = MAIN_WindowHeight;
                mouseClipToRect(&rect);
            }
        }
    }
    else
    {
        if (mouseCursorX() != MAIN_WindowWidth / 2 ||
        mouseCursorY() != MAIN_WindowHeight / 2)
        {
            mrMouseHasMoved += abs(mouseCursorX() - MAIN_WindowWidth / 2);
            mrMouseHasMoved += abs(mouseCursorY() - MAIN_WindowHeight / 2);
            camMouseX = MAIN_WindowWidth / 2 - mouseCursorX();
            camMouseY = MAIN_WindowHeight / 2 - mouseCursorY();
            mousePositionSet(MAIN_WindowWidth / 2, MAIN_WindowHeight / 2);
        }

    }
}

/*-----------------------------------------------------------------------------
    Name        : mrKeyRelease
    Description : handle key releases
    Inputs      : ID - keyindex of key being released
    Outputs     : dispatches approproiate function(s) for specified key
    Return      : void
----------------------------------------------------------------------------*/
void mrKeyRelease(sdword ID)
{
    MaxSelection tempSelection;

    // Drew's keybinding
    ID = (sdword)kbCheckBindings(ID);

    switch (ID)
    {
        case SHIFTKEY:
            if (mrHoldRight != mrCameraMotion)
            {
                piePointModeToggle(FALSE);
                ioSetSelection(TRUE);
            }
            break;
        case ZKEY:
            if (selSelected.numShips > 0 && mrHoldLeft == mrNULL && mrSpecialBandBox == FALSE)
            {
                selSelectionCopy((MaxAnySelection *)&tempSelection, (MaxAnySelection *)&selSelected);
                MakeShipsSpecialActivateCapable((SelectCommand *)&tempSelection);
                if (tempSelection.numShips > 0)       // check again in case some ships were eliminated.
                {
                    tutGameMessage("KB_Special");
                    clWrapSpecial(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,NULL);
                }
            }
            break;
        case NUMPLUSKEY:
        case PLUSKEY:
            zoomInNow = FALSE;
            break;
        case MINUSKEY:
        case NUMMINUSKEY:
            zoomOutNow = FALSE;
            break;

        default:
#if MR_VERBOSE_LEVEL >= 2
            dbgMessagef("\nmrKeyRelease: unprocessed key = 0x%x", ID);
#endif
            break;
    }
}

#if MR_KEYBOARD_CHEATS
void mrScanDebugCodes(sdword ID);
#endif
void gpStartInGameEscapeMenu(void);

bool NoShift(void)
{
    if (keyIsHit(SHIFTKEY) || keyIsHit(CONTROLKEY) || keyIsHit(ALTKEY))
        return FALSE;
    else
        return TRUE;
}

void mrSetTheFormation(TypeOfFormation formationtype)
{
    char msgName[256];      // for the tutorial message

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)
        return;

    strcpy(msgName, "Game_Formation_");

    makeShipsFormationCapable((SelectCommand *)&selSelected);

    if(selSelected.numShips > 0)
    {
        if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) >= 0)
        {
            soundEvent(NULL, UI_Click);
            mrNewFormation = PARADE_FORMATION_FLAG;
            mrFormationName = strGetString(strPARADE_FORMATION);
            speechEvent(selSelected.ShipPtr[0], COMM_SetFormation, PICKET_FORMATION);
            clWrapSetMilitaryParade(&universe.mainCommandLayer,(SelectCommand *)&selSelected);

            strcat(msgName, strGetString(strPARADE_FORMATION));
        }
        else
        {
            soundEvent(NULL, UI_Click);
            mrNewFormation = formationtype;
            mrFormationName = TypeOfFormationToNiceStr(formationtype);
            if (formationtype == CUSTOM_FORMATION)
            {
                speechEvent(selSelected.ShipPtr[0], COMM_SetFormation, formationtype+1);
            }
            else
            {
                speechEvent(selSelected.ShipPtr[0], COMM_SetFormation, formationtype);
            }
            clWrapFormation(&universe.mainCommandLayer,     //set new formation
                        (SelectCommand *)&selSelected,formationtype);

            strcat(msgName, TypeOfFormationToStr(formationtype));

        }
    }

    // Draw the formation set to the screen
    mrDrawFormationTime = universe.totaltimeelapsed;
    mrDrawFormation = TRUE;

    tutGameMessage(msgName);
    mrFormationName = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : mrMothershipInFocus
    Description : Return TRUE if the specified mothership-class ship is in focus.
    Inputs      : mothership - the specified mothership
    Outputs     :
    Return      : TRUE if motherhip in focus, FALSE otherwise.
----------------------------------------------------------------------------*/
#define MR_ONLY_MOTHERSHIP          1                       //1=focus only on mothership, 0 = focus on mothership along with other ships
#define MR_NumberMothershipTypes    8
ubyte mrMothershipShipTypes[] = {Carrier, Mothership, P1Mothership, P2Mothership, P3Megaship, FloatingCity, ResearchStation, JunkYardHQ, 0};
Ship *mrMothershipPtr = NULL;
BOOL mrMothershipInFocus(Ship *mothership)
{
    FocusCommand *focus;

    dbgAssert(memchr(mrMothershipShipTypes, mothership->shiptype, MR_NumberMothershipTypes));
    focus = &universe.mainCameraCommand.currentCameraStack->focus;
#if MR_ONLY_MOTHERSHIP
    if (focus->numShips == 1 && focus->ShipPtr[0] == mothership)
    {                                                       //if focussed only on the mothership
        return(TRUE);
    }
    return(FALSE);
#else     11
    return(selShipInSelection(focus->ShipPtr, focus->numShips, mothership));
#endif
}

/*-----------------------------------------------------------------------------
    Name        : mrNextMothershipPtr
    Description : Return a pointer to the next mothership if the player has more
                    than one mothership.
    Inputs      : mothership - the currently focused mothership
    Outputs     :
    Return      : Next player mothership in shiplist (may be NULL or the same as mothership)
----------------------------------------------------------------------------*/
Ship *mrNextMothershipPtr(Ship *mothership)
{
    Node *shipNode;
    Ship *ship, *startingShip;
    bool bFirstShip = TRUE;
    sdword count = 0;

    if (mothership == NULL)
    {                                                       //mothership not defined yet
        if (universe.curPlayerPtr->PlayerMothership != NULL)
        {                                                   //get link from player's mothership
            shipNode = &universe.curPlayerPtr->PlayerMothership->shiplink;
        }
        else
        {                                                   //get link from start of shiplist
            shipNode = universe.ShipList.head;
        }
        if (shipNode == NULL) return NULL;
        startingShip = (Ship *)listGetStructOfNode(shipNode);
        if (startingShip == NULL) return NULL;      // believe it or not this check is required or it will crash under certain conditions
    }
    else
    {                                                       //get link from specified mothership
        shipNode = mothership->shiplink.next;
        if (shipNode == NULL)
        {
            shipNode = universe.ShipList.head;
        }
        if (shipNode == NULL) return NULL;
        startingShip = mothership;
    }

    while (1)
    {
        if (shipNode == NULL) return NULL;
        ship = listGetStructOfNode(shipNode);
        if (ship == NULL) return NULL;          // believe it or not this check is required or it will crash under certain conditions
        if (ship == startingShip && (!bFirstShip))
        {                                                   //if we've come all the way back to the player's mothership
            return(NULL);
        }
        if (ship->playerowner == universe.curPlayerPtr &&
            memchr(mrMothershipShipTypes, ship->shiptype, MR_NumberMothershipTypes) &&
            bitTest(ship->flags, SOF_Selectable))
        {                                                   //if it's the player's ship and it's a mothership type
            break;                                          //use this one as the mothership
        }
        shipNode = shipNode->next;
        if (shipNode == NULL)
        {                                                   //if reached end of shiplist
            shipNode = universe.ShipList.head;              //go back to start of shiplist
        }
        if (count >= universe.ShipList.num)
        {                                                   //if we've already looked at every ship in list
            return(NULL);
        }
        count++;
        if (shipNode == NULL) return NULL;
        bFirstShip = FALSE;
    }
    return(ship);
}

/*-----------------------------------------------------------------------------
    Name        : mrKeyPress
    Description : handle key presses
    Inputs      : ID - keyindex of key being pressed
    Outputs     : dispatches approproiate function(s) for specified key
    Return      : void
----------------------------------------------------------------------------*/
void mrKeyPress(sdword ID)
{
    if (!gameIsRunning)
    {
        return;
    }
    if (nisIsRunning && thisNisPlaying && (ID == ESCKEY || ID == SPACEKEY || ID == ENTERKEY))
    {
        if (singlePlayerGame)
        {
            if (binkDonePlaying)
            {
                goto processEscapeKey;
            }
        }
        else
        {
            goto processEscapeKey;
        }
    }

    //now translate
    //ID = (sdword)opKeyTranslate((keyindex)ID);

    // Drew's keybinding
    ID = (sdword)kbCheckBindings(ID);

    if (ID == CAPSLOCKKEY)
    {
        goto docapslock;        // TO always on
    }

    if (mrDisabled)
    {
        return;
    }

#if SP_DEBUGKEYS
//    if (singlePlayerGame)     // check occurs in singlePlayerCheckDebugKeys now
//    {
        if (keyIsHit(WKEY))
        {
            singlePlayerCheckDebugKeys(ID);
            return;
        }
//    }
#endif //SP_DEBUGKEYS
#if MR_KEYBOARD_CHEATS
    if keyIsHit(SHIFTKEY) mrScanDebugCodes(ID);
#endif

    switch (ID)
    {
        case SHIFTKEY:
            if (mrHoldRight != mrCameraMotion)
            {
                piePointModeToggle(TRUE);
            }
            break;
        case HOMEKEY:
#if LOD_PRINT_DISTANCE
            if (!lodTuningMode)
            {
#endif
                tutGameMessage("KB_FocusMothership");
                goto focusOnMothership;
#if LOD_PRINT_DISTANCE
            }
            else
            {
                keySetSticky(HOMEKEY);
            }
#endif
            break;
        case PAGEDOWNKEY:
            gcPageDownProcess();
            break;
        case PAGEUPKEY:
            gcPageUpProcess();
            break;
        case ZEROKEY:
        case ONEKEY:
        case TWOKEY:
        case THREEKEY:
        case FOURKEY:
        case FIVEKEY:
        case SIXKEY:
        case SEVENKEY:
        case EIGHTKEY:
        case NINEKEY:
            dbgAssert((ID - ZEROKEY) < COMMAND_MAX_SHIPS);
#if NIS_PRINT_INFO
            if (keyIsHit(NKEY))
            {                                               //n-#: play an NIS or NISlet
                if (ID == ZEROKEY)
                {
                    AIVar *var;
                    var = aivarCreate("PlayNis");
                    aivarValueSet(var, singlePlayerGameInfo.currentMission);
                }
                else
                {
                    if (ID <= THREEKEY)
                    {
                        AIVar *var;
                        sdword nisLetNumber;
                        char *nisName, *scriptName;

                        nisLetNumber = (singlePlayerGameInfo.currentMission) * 10 + ID - ZEROKEY - 1;
                        if (singlePlayerNISletNamesGet(&nisName, &scriptName, nisLetNumber))
                        {
                            var = aivarCreate("PlayNisLet");
                            aivarValueSet(var, nisLetNumber);
                        }
                    }
                }
            }
            else
#endif
            if (keyIsHit(CONTROLKEY))
            {                                               //control-# assign a hot key group
#if SP_NISLET_TEST
                if (keyIsHit(ALTKEY) && singlePlayerGame)
                {                                           //control-alt-#: attempt to test a NISlet
                    spNISletTestAttempt(ID - ZEROKEY - 1);
                }
#endif
                if (selSelected.numShips != 0)
                {
                    bool bReinforced;
//                    selHotKeyNumbersSet(&selHotKeyGroup[ID - ZEROKEY], (uword)SEL_InvalidHotKey);

                    tutGameMessage("KB_GroupAssign");

                    selHotKeyGroupRemoveReferences(ID - ZEROKEY);
                    bReinforced = selSelectionIsReinforced((MaxAnySelection *)&selHotKeyGroup[ID - ZEROKEY],(MaxAnySelection *)&selSelected);
                    selSelectionCopy((MaxAnySelection *)&selHotKeyGroup[ID - ZEROKEY],(MaxAnySelection *)&selSelected);
                    selHotKeyNumbersSet(ID - ZEROKEY);
#if SEL_ERROR_CHECKING
                    selHotKeyGroupsVerify();
#endif
#if MR_VERBOSE_LEVEL >= 2
                    dbgMessagef("\nHot key group %d assigned.", ID - ZEROKEY);
#endif
                    soundEvent(NULL, UI_ClickAccept);
                    if (bReinforced)
                    {                                       //if this assignment was a reinforcement
                        speechEventFleet(COMM_F_AssGrp_AddingShips, ID - ZEROKEY, universe.curPlayerIndex);
                    }
                    else
                    {                                       //no reinforcment, straight assignment
                        speechEventFleet(COMM_F_Group_Assigning, ID - ZEROKEY, universe.curPlayerIndex);
                    }
                }
            }
            else if (keyIsHit(ALTKEY))
            {                                               //alt-# select and focus on a hot key group
altCase:
                if (selHotKeyGroup[ID - ZEROKEY].numShips != 0)
                {
                    tutGameMessage("KB_GroupFocus");

                    selSelectHotKeyGroup(&selHotKeyGroup[ID - ZEROKEY]);
                    selHotKeyNumbersSet(ID - ZEROKEY);
                    ioUpdateShipTotals();
#if SEL_ERROR_CHECKING
                    selHotKeyGroupsVerify();
#endif
                    if (selSelected.numShips > 0)
                    {
                        ccFocus(&(universe.mainCameraCommand),(FocusCommand *)&selSelected);
#if MR_VERBOSE_LEVEL >= 2
                        dbgMessagef("\nHot key group %d selected and focused upon.", ID - ZEROKEY);
#endif
                        soundEvent(NULL, UI_Click);
                        speechEvent(selHotKeyGroup[ID - ZEROKEY].ShipPtr[0], COMM_AssGrp_Select, ID - ZEROKEY);
                    }
               }
            }
            else if (keyIsHit(SHIFTKEY))
            {                                               //shift-# add hot key group to current selection
                sdword index;
                if(tutorial && selHotKeyGroup[ID - ZEROKEY].numShips != 0)
                    tutGameMessage("KB_GroupAddSelect");

                for (index = selHotKeyGroup[ID - ZEROKEY].numShips - 1; index >= 0; index--)
                {                                           //for all ships in hot key group
                    if( selHotKeyGroup[ID - ZEROKEY].ShipPtr[index]->collMyBlob != NULL &&
                        selHotKeyGroup[ID - ZEROKEY].ShipPtr[index]->flags & SOF_Selectable )
                    {
                        selSelectionAddSingleShip(&selSelected, selHotKeyGroup[ID - ZEROKEY].ShipPtr[index]);
                    }
                }
                selHotKeyNumbersSet(ID - ZEROKEY);
#if SEL_ERROR_CHECKING
                selHotKeyGroupsVerify();
#endif
#if MR_VERBOSE_LEVEL >= 2
                dbgMessagef("\nHot key group %d added to selection.", ID - ZEROKEY);
#endif
                ioUpdateShipTotals();
                if (selHotKeyGroup[ID - ZEROKEY].numShips > 0)
                {
                    soundEvent(NULL, UI_Click);
                    speechEvent(selHotKeyGroup[ID - ZEROKEY].ShipPtr[0], COMM_AssGrp_Select, ID - ZEROKEY);
                }
            }
            else if (ID == mrLastKeyPressed && universe.totaltimeelapsed <= mrLastKeyTime + mrNumberDoublePressTime)
            {                                               //double-#: focus on hot-key group
                tutGameMessage("KB_GroupSelectFocus");
                goto altCase;                               //same as alt-#
            }
            else
            {                                               //plain# select a hot key group
                if (selHotKeyGroup[ID - ZEROKEY].numShips != 0)
                {
                    tutGameMessage("KB_GroupSelect");

                    selSelectHotKeyGroup(&selHotKeyGroup[ID - ZEROKEY]);
                    selHotKeyNumbersSet(ID - ZEROKEY);
#if SEL_ERROR_CHECKING
                    selHotKeyGroupsVerify();
#endif
                    ioUpdateShipTotals();
                    if (selSelected.numShips > 0)
                    {
                        soundEvent(NULL, UI_Click);
                        speechEvent(selHotKeyGroup[ID - ZEROKEY].ShipPtr[0], COMM_AssGrp_Select, ID - ZEROKEY);
                    }
                }
            }
            break;
        case SPACEKEY:
            tutGameMessage("KB_Sensors");
            smSensorsBegin(NULL, NULL);
            break;
        case ENTERKEY:
//            mrResearch(NULL, NULL);
            if (lastshiptospeak != NULL)
            {
                if (lastshiptospeak == (Ship *)-1)
                {
                    if (selHotKeyGroup[lastgrouptospeak].numShips != 0)
                    {
                        tutGameMessage("KB_FocusLast");

                        selSelectHotKeyGroup(&selHotKeyGroup[lastgrouptospeak]);
                        selHotKeyNumbersSet(lastgrouptospeak);
                        ioUpdateShipTotals();
#if SEL_ERROR_CHECKING
                        selHotKeyGroupsVerify();
#endif
                        if (selSelected.numShips > 0)
                        {
#if MR_VERBOSE_LEVEL >= 2
                            dbgMessagef("\nHot key group %d selected and focused upon.", lastgrouptospeak);
#endif
                            soundEvent(NULL, UI_Click);
                            ccFocus(&(universe.mainCameraCommand),(FocusCommand *)&selSelected);
                        }
                   }
                }
                else
                {
                    dbgMessagef("Focus on last ship: shipptr %d\n", lastshiptospeak);
                    tutGameMessage("KB_FocusLast");

                    selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)lastshiptospeak; //make a temporary dummy selection
                    selSelecting.numTargets = 1;
                    soundEvent(NULL, UI_Click);
                    ccFocus(&universe.mainCameraCommand,
                            (FocusCommand *)&selSelecting);//focus on these ships
                    selSelecting.numTargets = 0;
                }
            }
            break;
processEscapeKey:
    case ESCKEY:

            if (singlePlayerGame && popupTextNumLines)
            {
                popupTextNumLines = 0;
            }
            else if ( (multiPlayerGame) && (ViewingBuffer) )
            {
                gcCancelViewingBuffer();
            }
            else if (thisNisPlaying)
            {                                               // single player game NIS playing
                if (nisCameraCutTime == 0.0f && nisScissorFadeIn == 0.0f)
                {                                           //if not just starting the NIS
                    nisGoToEnd(thisNisPlaying);             //abort; skip the NIS
                }                                           //else you can't skip NIS.  Is this acceptable?
            }
            else if (piePointSpecMode != PSM_Idle)
            {
                piePointModeOnOff();
                soundEvent(NULL, UI_MovementGUIoff);
            }
            else if (mrHoldLeft == mrSelectHold)
            {
                mrHoldLeft = mrNULL;
                mrSelectionRect.x0 = mrSelectionRect.x1 =   //clear selection rectangle
                    mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
                selSelecting.numTargets = 0;
            }
            else if ((selSelected.numShips > 0) &&
                     ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bCancelSelect))
            {
                soundEvent(NULL, UI_Click);
                tutGameMessage("KB_SelectCancel");
                selSelected.numShips = 0;
                ioUpdateShipTotals();
            }
            else
            {
                if (!multiPlayerGame)
                {
                    universePause = TRUE;
                }
                soundEvent(NULL, UI_Click);
                PossiblyResetTaskbar();
                //feScreenStart(ghMainRegion, "In_game_esc_menu");
                tutGameMessage("SP_EscMenu");
                gpStartInGameEscapeMenu();
            }
            break;
        case ARRLEFT:
#if NIS_TEST
            if (testPlaying != NULL && !multiPlayerGame)
            {
                if ((selSelected.numShips > 0) && (!multiPlayerGame))
                {
                    nisTestAnother(-1);
                }
            }
            else
#endif
#ifdef HW_Debug
            if (pilotView)
            {
                pilotupoffset *= 0.95f;
                dbgMessagef("\nUpOff %f",pilotupoffset);
            }
#endif
            if (keyIsHit(ALTKEY))
            {
                goto cancelfocus;
            }
            break;
        case ARRRIGHT:
#if NIS_TEST
            if (testPlaying != NULL && !multiPlayerGame)
            {
                if ((selSelected.numShips > 0) && (!multiPlayerGame))
                {
                    nisTestAnother(1);
                }
            }
            else
#endif
#ifdef HW_Debug
            if (pilotView)
            {
                pilotupoffset *= 1.05f;
                dbgMessagef("\nUpOff %f",pilotupoffset);
            }
#endif
            if (keyIsHit(ALTKEY))
            {
                goto forwardfocus;
            }
            break;
        case TABKEY:
#if 0
            if((!multiPlayerGame) && (!singlePlayerGame))
            if(keyIsHit(CONTROLKEY))
            {
                univAddDerelict(Crate,&mrCamera->lookatpoint);
                break;
            }
#endif
            if (selSelected.numShips >= MIN_SHIPS_IN_FORMATION)
            {
                if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) >= 0)
                {
                    mrNewFormation = PARADE_FORMATION_FLAG;
                    mrFormationName = strGetString(strPARADE_FORMATION);
                    mrFormationTime = universe.totaltimeelapsed;
                    mrDrawFormationTime = universe.totaltimeelapsed;
                    mrDrawFormation = TRUE;
                }
                else
                {
                    TypeOfFormation alreadyformed = clSelectionAlreadyInFormation(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
                    if ((alreadyformed == NO_FORMATION && mrFormationName == NULL) || (mrNewFormation == PARADE_FORMATION_FLAG))
                    {
                        mrNewFormation = 0;
                    }
                    else
                    {
                        if (keyIsHit(SHIFTKEY))
                        {
                            if (mrNewFormation <= 0)
                            {
                                mrNewFormation = CUSTOM_FORMATION - 1;
                            }
                            else
                            {
                                mrNewFormation--;
                            }
                            tutGameMessage("KB_FormationPrevious");
                        }
                        else
                        {
                            mrNewFormation++;
                            if (mrNewFormation >= CUSTOM_FORMATION)
                            {
                                mrNewFormation = 0;
                            }
                            tutGameMessage("KB_FormationNext");
                        }
                    }
                    mrFormationName = TypeOfFormationToNiceStr(mrNewFormation);
                    mrFormationTime = universe.totaltimeelapsed;
                    mrDrawFormationTime = universe.totaltimeelapsed;
                    mrDrawFormation = TRUE;
                }
                soundEvent(NULL, UI_Click);
            }
            break;
        case AKEY:
/*            if (keyIsHit(CONTROLKEY))
            {
                goto caseEKey;
            }*/
#if MAD_TEST_ANIMATION
            if (keyIsHit(ALTKEY))
            {                                               //alt-a: test mesh animation
                if (selSelected.numShips >= 1)
                {                                           //if ships selected
                    if (selSelected.ShipPtr[0]->madBindings != NULL)
                    {                                       //if this ship has an animation structure
                        if (selSelected.ShipPtr[0]->madBindings->nCurrentAnim == MAD_NoAnimation)
                        {                                   //if nothing currently playing
                            if (mrTestAnimationIndex >= selSelected.ShipPtr[0]->madBindings->header->nAnimations)
                            {                                   //if already tested them all
                                mrTestAnimationIndex = 0;       //go back to first animation
                            }
                            madAnimationStart(selSelected.ShipPtr[0], mrTestAnimationIndex);
                            mrTestAnimationIndex++;
                        }
                        else
                        {
                            if (bitTest(selSelected.ShipPtr[0]->madBindings->header->anim[selSelected.ShipPtr[0]->madBindings->nCurrentAnim].flags, MAF_Loop))
                            {                               //if looping animation
                                madAnimationStop(selSelected.ShipPtr[0]);
                            }
                            else
                            {                               //else non-looping animation
                                madAnimationPause(selSelected.ShipPtr[0], !selSelected.ShipPtr[0]->madBindings->bPaused);
                            }
                        }
                    }
                }
            }
#endif
            break;


        case BKEY:
            if (keyIsHit(CONTROLKEY) && singlePlayerGame)
            {
                //mrTradeStuffTest(NULL, NULL);

            }
            else if (NoShift())
            {
                if (playPackets)
                    break;  // packet playback or universe paused, can't go into build manager!!

                tutGameMessage("KB_Build");
                mrBuildShips(NULL, NULL);
            }
            break;

        case CKEY:
            if (NoShift())
            {
cancelfocus:
                tutGameMessage("KB_CancelFocus");
                ccCancelFocus(&(universe.mainCameraCommand));

                soundEvent(NULL, UI_ClickCancel);
            }
            break;

        case DKEY:

            if (NoShift())
            {
                tutGameMessage("KB_Dock");
                mrDockingOrders(NULL, NULL);
            }
#if MR_SOUND_RELOAD_VOLUMES
            else if (keyIsHit(CONTROLKEY))
            {
                if (keyIsHit(SHIFTKEY))
                {
                    soundMixerSetMode(SOUND_MODE_LOW);
                }
                else
                {
                    soundMixerSetMode(SOUND_MODE_NORM);
                }
            }
#endif
            break;

        case EKEY:
            caseEKey:

            if (NoShift() && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bBandSelect))
            {                                               //'E': select everyone onscreen
                rectangle fullScreen = {0, 0, MAIN_WindowWidth, MAIN_WindowHeight};
                MaxSelection tempSelection;

                tutGameMessage("KB_SelectEveryone");

                selSelectionCopy((MaxAnySelection *)&tempSelection,(MaxAnySelection *)&selSelected);
                selRectSelect(&(universe.mainCameraCommand.actualcamera),
                              &fullScreen);
                if ((tempSelection.numShips != 0) &&
                    (selSelectionCompare((MaxAnySelection *)&selSelected,(MaxAnySelection *)&tempSelection) == 0))
                {                                           //if identical selection last time E was hit
                    selSelected.numShips = 0;               //select nothing
                }
                selSelecting.numTargets = 0;
                ioUpdateShipTotals();

                soundEvent(NULL, UI_Click);
            }
            break;
        case MMOUSE_BUTTON:
        case MMOUSE_DOUBLE:
        case FKEY:
            if (keyIsHit(ALTKEY))
            {
                goto focusOnMothership;
            }
            else if ((selSelected.numShips != 0) && NoShift())
            {
                MaxSelection tempselected;

                tutGameMessage("KB_Focus");

                soundEvent(NULL, UI_Click);

                tempselected = selSelected;     //copy structure
                MakeShipMastersIncludeSlaves((SelectCommand *)&tempselected);
                ccFocus(&universe.mainCameraCommand,(FocusCommand *)&tempselected);
            }
            break;
        case HKEY:

            if (NoShift())
            {
                tutGameMessage("KB_Harvest");
                mrHarvestResources(NULL, NULL);

            }
            break;

        case LKEY:
#if LOD_AUTO_SAVE
            if (keyIsHit(CONTROLKEY))
            {
                lodinfo *LOD;
                if (keyIsHit(SHIFTKEY))
                {                                           //ctrl-shift-L - save LOD file
                    if (selSelected.numShips == 1)
                    {
                        if (lodTuningMode)
                        {
                            LOD = selSelected.ShipPtr[0]->staticinfo->staticheader.LOD;
                            lodAutoSave(LOD);
                            dbgMessagef("\nLOD file '%s' saved.", LOD->fileName);
                        }
                    }
                }
                else if (lodTuningMode)
                {                                           //ctrl-L - remember LOD
                    if (selSelected.numShips == 1)
                    {
                        LOD = selSelected.ShipPtr[0]->staticinfo->staticheader.LOD;
                        if (rndLOD >= 0 && rndLOD < LOD->nLevels)
                        {
                            vector shippos = selSelected.ShipPtr[0]->posinfo.position;
                            vector length;

                            vecSub(length,mrCamera->eyeposition,shippos);

                            LOD->level[rndLOD].mOn = LOD->level[rndLOD].mOff = 1.0f;
                            if ((LOD->level[rndLOD].flags & LM_LODType) == LT_SubPixel)
                            {
                                LOD->level[rndLOD].bOff = REALlyBig;
                            }
                            else
                            {
                                LOD->level[rndLOD].bOff = vecMagnitudeSquared(length) / lodScaleFactor;
                            }
                            LOD->level[rndLOD].bOn = LOD->level[rndLOD].bOff * 0.9f;
                            dbgMessagef("\nLOD #%d remembered at %.2f", rndLOD, LOD->level[rndLOD].bOff);
                        }
                    }
                }
                break;
            }
#endif //LOD_AUTO_SAVE
#if RND_GL_STATE_DEBUG
            if (keyIsHit(GKEY))
            {
                keySetSticky(LKEY);
                break;
            }
#endif //RND_GL_STATE_DEBUG


            if (NoShift())
            {
                tutGameMessage("KB_Launch");
                mrLaunch(NULL, NULL);
            }
            break;

        case LBRACK:
            if(keyIsHit(SHIFTKEY))
            {
                if (RGL) rglFeature(RGL_GAMMA_DN);
                shGammaDown();
                break;
            }
            if(selSelected.numShips > 0)
            {
                sdword curTactics = selSelected.ShipPtr[0]->tacticstype;
                curTactics--;
                if(curTactics < 0)
                    curTactics = 2;

                soundEvent(NULL, UI_Click);

                tacticsPopUpSetUp(curTactics);
                clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,curTactics);

                tutGameMessage("KB_TacticsPrevious");
            }
            break;
        case RBRACK:
            if(keyIsHit(SHIFTKEY))
            {
                if (RGL) rglFeature(RGL_GAMMA_UP);
                shGammaUp();
                break;
            }
            if(selSelected.numShips > 0)
            {
                sdword curTactics = selSelected.ShipPtr[0]->tacticstype;
                curTactics++;
                if(curTactics > 2)
                    curTactics = 0;

                soundEvent(NULL, UI_Click);

                tacticsPopUpSetUp(curTactics);
                clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,curTactics);

                tutGameMessage("KB_TacticsNext");
            }
            break;
        case JKEY:
            if ((!singlePlayerGame) && (bitTest(tpGameCreated.flag,MG_Hyperspace)))
            {
                makeShipsNotIncludeSinglePlayerMotherships((SelectCommand *)&selSelected);
                makeShipsControllable((SelectCommand *)&selSelected,COMMAND_MP_HYPERSPACEING);
                makeSelectionHyperspaceCapable((SelectCommand *)&selSelected);
                if(selSelected.numShips == 0)
                {
                    //removed all ships!
                    //if normal gui up..bring it down!
                    if (piePointSpecMode != PSM_Idle)
                    {
                        //turn it off!
                        soundEvent(NULL, UI_MovementGUIoff);
                        piePointModeOnOff();
                    }
                }
                else
                {
                    if(!smSensorsActive)
                    {   //start sensors manager if it isn't active already
                        smSensorsBegin(NULL, NULL);     //step one complete!
                    }
                    if(!smSensorsDisable && !smZoomingIn)
                    {   //if sensors aren't disabled AND we aren't zooming in
                        MP_HyperSpaceFlag=TRUE;
                        if (piePointSpecMode == PSM_Idle)
                        {
                            //turn it on if it was off!
                            soundEvent(NULL, UI_MovementGUIon);
                            piePointModeOnOff();
                        }
                    }
                    //we are now going into the sensors manager
                }
            }
            else if ( (singlePlayerGame) && (singlePlayerGameInfo.playerCanHyperspace) )
            {
                spHyperspaceButtonPushed();
            }
            break;
        case MKEY:
            if (keyIsHit(ALTKEY))
            {
                focusOnMothership:;
                {
                    Ship *mothership = mrMothershipPtr;
                    FocusCommand focus;
                    //focus on mothership
                    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bMothershipFocus)
                        return;

                    if (mrMothershipPtr == NULL || mrMothershipInFocus(mrMothershipPtr))
                    {                                       //if focussed on nothing or on a mothership
                        mrMothershipPtr = mrNextMothershipPtr(mrMothershipPtr);
                    }
                    if (mrMothershipPtr != NULL && !bitTest(mrMothershipPtr->flags, SOF_Dead))
                    {                                       //if there's a mothership to focus upon
                        focus.ShipPtr[0] = mrMothershipPtr;
                        focus.numShips = 1;
                        ccFocus(&universe.mainCameraCommand, &focus);
                        soundEvent(NULL, UI_Click);

                        tutGameMessage("KB_FocusMothership");
                    }
                }
            }
            else if (NoShift())
            {
                tutGameMessage("KB_Move");
                mrMoveShips(NULL, NULL);
            }
            break;
        case RKEY:
#if ETG_RELOAD_KEY
            if (!multiPlayerGame && keyIsHit(CONTROLKEY))
            {
                etgReset();
                break;
            }
#endif
#if MR_SOUND_RELOAD_VOLUMES
            if (keyIsHit(SHIFTKEY))
            {
                soundEventReloadVolumes();
                break;
            }
#endif

            if (NoShift())
            {
                tutGameMessage("KB_Research");
                mrResearch(NULL, NULL);

            }
            break;

        case SKEY:

            if (NoShift())
            {
                tutGameMessage("KB_Scuttle");
                mrScuttle(NULL, NULL);
            }
            break;
#if UNIVERSE_TURBOPAUSE_DEBUG
        case BACKSLASHKEY:
            if ((!multiPlayerGame) || (playPackets) || (universePause) || (mrDisabled) )
            {
                universeTurbo = !universeTurbo;
                dbgMessage(universeTurbo ? "\nTurbo ON" : "\nTurbo OFF");
            }
            break;
#endif
        case IKEY:
            {
                tutGameMessage("KB_Retire");
                mrRetire(NULL, NULL);
            }
            break;
        case PKEY:
            if ((!multiPlayerGame) && NoShift())
            {
                tutGameMessage("KB_Pause");
                if ((tutorial==TUTORIAL_ONLY) && (!tutEnable.bPauseGame))
                {
                    break;
                }
                if (!universePause)
                {
                    if (piePointSpecMode!=PSM_Idle)
                    {
                        piePointModeOnOff();
                    }
                }

                universePause = !universePause;
                dbgMessage(universePause ? "\nPause ON" : "\nPause OFF");
                soundEvent(NULL, UI_Click);
            }
#if MR_SOUND_RELOAD_VOLUMES
            else if (keyIsHit(SHIFTKEY))
            {
                if (keyIsHit(CONTROLKEY))
                {
                    soundEventPause(TRUE);
                }
                else
                {
                    soundEventPause(FALSE);
                }
            }
#endif
            break;
        case QKEY:
            if (pilotView)
                bitToggle(universe.mainCameraCommand.ccMode,CCMODE_PILOT_SHIP);
            break;

#if MR_TEST_GUNS
        case TKEY:
            if (selSelected.numShips == 1 && !multiPlayerGame)
            {
                Ship *ship = selSelected.ShipPtr[0];
                ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
                if (shipstatic->custshipheader.CustShipFire != NULL)
                {
                    shipstatic->custshipheader.CustShipFire(ship,NULL);
                }
            }
            else
                if (multiPlayerGame)
                {
                    gcChatEntryStart(keyIsHit(CONTROLKEY));
                }
            break;
#else
        case TKEY:
            if ((multiPlayerGame) && !keyIsHit(SHIFTKEY))
            {
                gcChatEntryStart(keyIsHit(CONTROLKEY));
            }
            break;
#endif
#ifndef HW_Release
#if GUN_TUNE_MODE
        case YKEY:
            if (selSelected.numShips == 1 && !multiPlayerGame)
            {
                gunTuningMode = !gunTuningMode;
            }
            break;
#endif
#endif

        case VKEY:

            if (NoShift())
            {
forwardfocus:
            tutGameMessage("KB_ForwardFocus");
            ccForwardFocus(&universe.mainCameraCommand);

            soundEvent(NULL, UI_ClickAccept);
            }

            break;
#if NIS_TEST
        case NUMPAD1:
            if (testPlaying == NULL && !multiPlayerGame)
            {
                matrix *identity = (matrix *)&IdentityMatrix;
                if ((selSelected.numShips > 0) && (!multiPlayerGame))
                {
                    nisTest(&selSelected.ShipPtr[0]->posinfo.position, identity);
                }
            }
            else
            {
                keySetSticky(NUMPAD1);                      //reset the sticky key
            }
            break;
#endif
        case CAPSLOCKKEY:
docapslock:
            mrDrawTactical ^= TRUE;

            if(mrDrawTactical)
            {
                soundEvent(NULL, UI_TacOverlayOn);
                tutGameMessage("KB_TacticalDisplayOn");
            }
            else
            {
                soundEvent(NULL, UI_TacOverlayOff);
                tutGameMessage("KB_TacticalDisplayOff");
            }
            break;

    case XKEY:
#ifndef HW_Release
            if(keyIsHit(CONTROLKEY))
            {
                if (!multiPlayerGame)
                {
                    universeSwitchToNextPlayer();
                }
            }
#endif
            break;

    case F1KEY:
            if(!keyIsHit(CONTROLKEY))
            {
                tutGameMessage("KB_FleetView");
                ccViewToggleMissionSphere(&universe.mainCameraCommand);
            }
            else
            {
                if(!multiPlayerGame)
                {
                    if(keyIsHit(ALTKEY))
                    {
                        universeRealTimeTweak((SelectCommand *)&selSelected);
                    }
                    else
                    {
                        tacticsShutDown();
                        tacticsStartUp();
                    }
                }
            }
            break;

        case KKEY:
            if(MakeSelectionKamikazeCapable((SelectCommand *)&selSelected) && NoShift())
            {
                CommandToDo *command;
                sdword i;
                tutGameMessage("KB_Kamikaze");
                for(i=0;i<selSelected.numShips;i++)
                {
                    command = getShipAndItsCommand(&universe.mainCommandLayer,selSelected.ShipPtr[i]);
                    if(command == NULL || (command->ordertype.order != COMMAND_ATTACK &&
                        command->ordertype.order != COMMAND_SPECIAL))
                    {
                        continue;
                    }
                    //if we get here...there is at least 1 valid ship that can kamikaze
                    break;
                }
                if(i==selSelected.numShips)
                {
                    //no valid ships!
                    if (selSelected.numShips > 0)
                        speechEvent(selSelected.ShipPtr[0],COMM_Kamikaze_NoTargets,0);
                }
                else
                {
                    soundEvent(NULL, UI_Click);
                    clWrapSetKamikaze(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
                }
            }
            break;

        case ZKEY:
            mrSpecialBandBox = FALSE;
            break;

        case F2KEY:
            if((selSelected.numShips > 0) && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bEvasive))
            {
                tutGameMessage("KB_TacticsEvasive");
                tacticsPopUpSetUp(Evasive);
                clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,Evasive);
                soundEvent(NULL, UI_ClickAccept);
            }
            break;
        case F3KEY:
            if((selSelected.numShips > 0) && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bNeutral))
            {
                tutGameMessage("KB_TacticsNeutral");
                tacticsPopUpSetUp(Neutral);
                clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,Neutral);
                soundEvent(NULL, UI_ClickAccept);
            }
            break;
        case F4KEY:
            if((selSelected.numShips > 0) && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bAgressive))
            {
                tutGameMessage("KB_TacticsAggressive");
                tacticsPopUpSetUp(Aggressive);
                clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,Aggressive);
                soundEvent(NULL, UI_ClickAccept);
            }
            break;
        case F5KEY:
        case F6KEY:
        case F7KEY:
        case F8KEY:
        case F9KEY:
        case F10KEY:
        case F11KEY:
            if (keyIsHit(SHIFTKEY))
            {
                if (ID == F6KEY)
                {
                    (void)gpQuickSave();
                }
                else if (ID == F7KEY)
                {
                    (void)gpQuickLoad();
                }
            }
            if (selSelected.numShips >= MIN_SHIPS_IN_FORMATION)
            {
                mrSetTheFormation(ID - F5KEY);
            }
            break;

#if MR_SCREENSHOTS
        case SCROLLKEY:
#if MAIN_Password
            if (mainScreenShotsEnabled)
#endif //MAIN_Password
            {
                soundEvent(NULL, UI_Click);
                rndTakeScreenshot = TRUE;
            }
            break;
/*        case PAUSEKEY:
            if (RGL)
            {
                rglFeature(RGL_MULTISHOT_END);
            }*/
            break;
#endif //MR_SCREENSHOTS
#if MR_TEST_HPB
        case OKEY:
            if (keyIsHit(CONTROLKEY))
            {
                if (madTestHPBIndex > 0)
                {
                    madTestHPBIndex--;
                }
            }
            else if (keyIsHit(SHIFTKEY))
            {
                madTestHPBIndex++;
            }
            break;
#endif
        case TILDEKEY:
            if (keyIsHit(SHIFTKEY))
            {                                               //shift-tilde select nothing
                tutGameMessage("KB_SelectNone");
                soundEvent(NULL, UI_Click);
                selSelectNone();
            }
            else if (keyIsHit(CONTROLKEY))
            {                                               //control-tilde, remove current hot-key group
                sdword index = SEL_InvalidHotKey, i;

                //find the first ship with a hotkey number displaying
                for (i=0;(index == SEL_InvalidHotKey) && (i < selSelected.numShips);i++)
                {
                    index = selHotKeyGroupNumberTest(selSelected.ShipPtr[i]);
                }

                // if the first ship is displaying a hotkey
                // and the entire selection is the same hotkey
                if ((index != SEL_InvalidHotKey) && (i==1) &&
                    selSelectionCompare((MaxAnySelection *)&selSelected,
                        (MaxAnySelection *)&selHotKeyGroup[index]) == 0)
                {
//                        selHotKeyNumbersSet(&selHotKeyGroup[index], SEL_InvalidHotKey);
                    soundEvent(NULL, UI_Click);
                    selHotKeyGroupRemoveReferences(index);
                    selHotKeyGroup[index].numShips = 0;  //select nothing for this hot key group
                    tutGameMessage("KB_GroupDelete");
#if SEL_ERROR_CHECKING
                    selHotKeyGroupsVerify();
#endif
                    break;
                }
                // if no hot key group was removed, then remove the selselected for all groups
                else if (index != SEL_InvalidHotKey)
                {
                    selHotKeyGroupRemoveReferencesFromAllGroups();
                }
            }
            else
            {                                               //else just undo
                if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bCancelCommand)
                    break;
                tutGameMessage("KB_Undo");
                udLatestThingUndo();
                soundEvent(NULL, UI_Click);
            }
            break;
        case NUMPLUSKEY:
        case PLUSKEY:
            soundEvent(NULL, UI_Click);
            zoomInNow = TRUE;
            break;
        case MINUSKEY:
        case NUMMINUSKEY:
            soundEvent(NULL, UI_Click);
            zoomOutNow = TRUE;
            break;
        case LESSTHAN:
            musicEventPrevTrack();
            break;
        case GREATERTHAN:
            musicEventNextTrack();
            break;

        default:
#if MR_VERBOSE_LEVEL >= 2
            dbgMessagef("\nmrKeyPress: unprocessed key = 0x%x", ID);
#endif
            break;
    }
    mrLastKeyPressed = ID;
    mrLastKeyTime = universe.totaltimeelapsed;
}

//bit fields for the action mask
#define MAM_Info                0x00000001
#define MAM_Formations          0x00000002
#define MAM_Tactics             0x00000004
#define MAM_Move                0x00000008
#define MAM_Scuttle             0x00000010
#define MAM_Retire              0x00000020
#define MAM_Divider             0x00000040
#define MAM_Dock                0x00000080
#define MAM_Launch              0x00000100
#define MAM_Harvest             0x00000200
#define MAM_Build               0x00000400
#define MAM_Research            0x00000800
#define MAM_Trade               0x00001000
#define MAM_Hyperspace          0x00002000
#define MAM_BasicSet            (MAM_Info | MAM_Formations | MAM_Tactics | MAM_Move)
#define MAM_MedSet              MAM_BasicSet | MAM_Scuttle
#define MAM_ExtSet              MAM_BasicSet | MAM_Scuttle | MAM_Retire

struct
{
    udword mask;
    char *string;
}
mrActionString[] =
{
    {MAM_Info,          "CSM_Info"},
    {MAM_Formations,    "Group_menu-TO-Group_Formations_submenu"},
    {MAM_Tactics,       "Group_menu-TO-Group_Tactics_submenu"},
    {MAM_Move,          "CSM_Move"},
    {MAM_Scuttle,       "CSM_Scuttle"},
    {MAM_Retire,        "CSM_Retire"},
    {MAM_Dock,          "CSM_Dock"},
    {MAM_Hyperspace,    "CSM_Hyperspace"},
    {MAM_Launch,        "CSM_Launch"},
    {MAM_Harvest,       "CSM_Harvest"},
    {MAM_Build,         "CSM_Build"},
    {MAM_Research,      "CSM_Research"},
    {MAM_Trade,         "CSM_Trade"},
    {0,      NULL},
};
udword mrMenuActionsByShipType[TOTAL_NUM_SHIPS] =
{
    /* AdvanceSupportFrigate */  MAM_ExtSet,
    /* AttackBomber          */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* Carrier               */  MAM_ExtSet | MAM_Divider | MAM_Build | MAM_Launch,
    /* CloakedFighter        */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* CloakGenerator        */  MAM_ExtSet,
    /* DDDFrigate            */  MAM_ExtSet,
    /* DefenseFighter        */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* DFGFrigate            */  MAM_ExtSet,
    /* GravWellGenerator     */  MAM_ExtSet,
    /* HeavyCorvette         */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* HeavyCruiser          */  MAM_ExtSet,
    /* HeavyDefender         */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* HeavyInterceptor      */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* IonCannonFrigate      */  MAM_ExtSet,
    /* LightCorvette         */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* LightDefender         */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* LightInterceptor      */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* MinelayerCorvette     */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* MissileDestroyer      */  MAM_ExtSet,
    /* Mothership            */  MAM_BasicSet | MAM_Divider | MAM_Scuttle | MAM_Build | MAM_Launch,
    /* MultiGunCorvette      */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* Probe                 */  MAM_MedSet | MAM_Divider | MAM_Dock,
    /* ProximitySensor       */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* RepairCorvette        */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* ResearchShip          */  MAM_ExtSet | MAM_Divider | MAM_Research,
    /* ResourceCollector     */  MAM_ExtSet | MAM_Divider | MAM_Harvest | MAM_Dock,
    /* ResourceController    */  MAM_ExtSet,
    /* SalCapCorvette        */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* SensorArray           */  MAM_ExtSet,
    /* StandardDestroyer     */  MAM_ExtSet,
    /* StandardFrigate       */  MAM_ExtSet,
    /* Drone                 */  MAM_ExtSet,
    /* TargetDrone           */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* HeadShotAsteroid      */  MAM_ExtSet,
    /* CryoTray              */  MAM_ExtSet,
    /* P1Fighter             */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P1IonArrayFrigate     */  MAM_ExtSet,
    /* P1MissileCorvette     */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P1Mothership          */  MAM_ExtSet,
    /* P1StandardCorvette    */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P2AdvanceSwarmer      */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P2FuelPod             */  MAM_ExtSet,
    /* P2Mothership          */  MAM_ExtSet,
    /* P2MultiBeamFrigate    */  MAM_ExtSet,
    /* P2Swarmer             */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P3Destroyer           */  MAM_ExtSet,
    /* P3Frigate             */  MAM_ExtSet,
    /* P3Megaship            */  MAM_ExtSet,
    /* FloatingCity          */  MAM_ExtSet,
    /* CargoBarge            */  MAM_ExtSet,
    /* MiningBase            */  MAM_ExtSet,
    /* ResearchStation       */  MAM_ExtSet,
    /* JunkYardDawg           */  MAM_ExtSet
};

char *mrMenuItemByFormation[] =
{
    /* DELTA_FORMATION   */ "CSM_DeltaFormation",
    /* BROAD_FORMATION   */ "CSM_BroadFormation",
    /* DELTA3D_FORMATION */ "CSM_XFormation",
    /* CLAW_FORMATION    */ "CSM_ClawFormation",
    /* WALL_FORMATION    */ "CSM_WallFormation",
    /* SPHERE_FORMATION  */ "CSM_SphereFormation",
    /* PICKET_FORMATION  */ "CSM_PicketFormation",
};

char *mrMenuItemByTactic[] =
{
    /* Evasive,     */ "CSM_Evasive",
    /* Neutral,     */ "CSM_Neutral",
    /* Aggressive,  */ "CSM_Agressive",
};

/*-----------------------------------------------------------------------------
    Name        : mrAtomSortCompare
    Description : Callback for sorting an atom list by y position.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int mrAtomSortCompare(const void *p0, const void *p1)
{
//    return(((featom *)p0)->y - ((featom *)p1)->y);
    if (((featom *)p0)->y > ((featom *)p1)->y)
    {
        return(1);
    }
    else if (((featom *)p0)->y == ((featom *)p1)->y)
    {
        return(((featom *)p1)->width - ((featom *)p0)->width);
    }
    else
    {
        return(-1);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrDeleteAllAtomsWithin
    Description : Delete all atoms from the atom list of specified screen that
                    are within the bounds of the specified atom
    Inputs      : screen - screen to delete atoms from
                  atomIndex - index of atom to get the bounds from
    Outputs     : Will delete at least 1 atom, the one specified.
    Return      : lowest index of a deleted atom
----------------------------------------------------------------------------*/
sdword mrDeleteAllAtomsWithin(fescreen *screen, sdword atomIndex)
{
    sdword index, j, lowest = SDWORD_Max;
    sdword thisX, thisY, thisWidth, thisHeight;

    thisX = screen->atoms[atomIndex].x;                     //remember location of this atom
    thisY = screen->atoms[atomIndex].y;
    thisWidth = screen->atoms[atomIndex].width;
    thisHeight = screen->atoms[atomIndex].height;
    for (index = 0; index < screen->nAtoms; index++)
    {                                                       //search all atoms
        if (screen->atoms[index].x >= thisX && screen->atoms[index].y >= thisY &&
            screen->atoms[index].x + screen->atoms[index].width <= thisX + thisWidth &&
            screen->atoms[index].y + screen->atoms[index].height <= thisY + thisHeight)
        {                                                   //if atom is inside the specified atom
            lowest = min(index, lowest);                    //remember the lowest index
            for (j = index; j < screen->nAtoms - 1; j++)
            {                                               //move the rest of the list back 1 space
                screen->atoms[j] = screen->atoms[j + 1];
            }
            index--;                                        //one less atom to check
            screen->nAtoms--;
        }
    }
    dbgAssert(lowest != SDWORD_Max);
    return(lowest);
}

/*-----------------------------------------------------------------------------
    Name        : mrMenuDisplay
    Description : Display a right-click menu for a certain set of ships
    Inputs      : actionMask - bit fields for ship types
                  currentFormation - what formation the selected ships are in, if any
                  tacticsBits - one bit for each type of tactic present in the group
    Outputs     : finds best matching menu in mrRightClickTable and starts it.
    Return      : void
----------------------------------------------------------------------------*/
void mrMenuDisplay(udword actionMask, TypeOfFormation currentFormation, udword tacticsBits)
{
#define NUMBER_GAPS         12
    fescreen *newScreen, *staticScreen;
    sdword index, j;
    char *name;
    sdword nGaps = 0;
    struct
    {
        sdword position, height;
    }
    gap[NUMBER_GAPS];
    featom *atom;

    //set the selected bit for the current formation
    staticScreen = feScreenFind("Group_Formations_submenu");
    for (index = 0; index < staticScreen->nAtoms; index++)
    {                                                       //clear all atoms of the selected bit
        bitClear(staticScreen->atoms[index].status, FAS_Checked);
    }
    //now set the formation bit on the correct formation
    if (currentFormation != NO_FORMATION)
    {
        atom = feAtomFindInScreen(staticScreen, mrMenuItemByFormation[currentFormation]);
        dbgAssert(atom != NULL);
        bitSet(atom->status, FAS_Checked);
    }
    //set the selected bit(s) for the current tactics
    staticScreen = feScreenFind("Group_Tactics_submenu");
    for (index = 0; index < staticScreen->nAtoms; index++)
    {                                                       //clear all atoms of the selected bit
        bitClear(staticScreen->atoms[index].status, FAS_Checked);
    }
    for (index = 0; index < NUM_TACTICS_TYPES; index++)
    {
        if (tacticsBits & (1 << index))
        {
            atom = feAtomFindInScreen(staticScreen, mrMenuItemByTactic[index]);
            dbgAssert(atom != NULL);
            bitSet(atom->status, FAS_Checked);
        }
    }
    //take the full menu and remove any atoms which represent the entries we are to remove
    staticScreen = feScreenFind("RightClickMenu");          //find the screen
    newScreen = memAlloc(sizeof(fescreen), "tempMenu", 0);  //allocate the temp screen
    *newScreen = *staticScreen;                             //copy the static screen to the temp screen
    if (staticScreen->nLinks > 0)
    {
        newScreen->links = memAlloc(sizeof(felink) * staticScreen->nLinks, "tempMenuLink", 0);
        memcpy(newScreen->links, staticScreen->links, sizeof(felink) * staticScreen->nLinks);
    }
    else
    {
        newScreen->links = NULL;
    }
    newScreen->atoms = memAlloc(sizeof(featom) * staticScreen->nAtoms, "tempMenuAtoms", 0);
    memcpy(newScreen->atoms, staticScreen->atoms, sizeof(featom) * staticScreen->nAtoms);
    qsort(newScreen->atoms, staticScreen->nAtoms, sizeof(featom), mrAtomSortCompare);

    //scan through the menu and remove useless atoms
    for (index = 0; index < newScreen->nAtoms; index++)
    {
        if (bitTest(newScreen->atoms[index].flags, FAF_Function))
        {                                                   //if a function atom
            name = newScreen->atoms[index].name;             //keep pointer to string
            for (j = 0; mrActionString[j].string != 0; j++)
            {                                               //for all possible actions
                if ((actionMask & mrActionString[j].mask) == 0)
                {                                           //if this action is not to be
                    if (feNamesEqual(name, mrActionString[j].string))
                    {                                       //and this atom has the action name
                        dbgAssert(nGaps < NUMBER_GAPS);
                        gap[nGaps].position = newScreen->atoms[index].y;
                        gap[nGaps].height = newScreen->atoms[index].height;
                        nGaps++;
                        index = mrDeleteAllAtomsWithin(newScreen, index);
                        index--;
                        break;
                    }
                }
            }
        }
        if (newScreen->atoms[index].type == FA_Divider)
        {                                                   //if this is a divider
            if (!bitTest(actionMask, MAM_Divider))
            {                                               //if we are not allowed dividers
                dbgAssert(nGaps < NUMBER_GAPS);
                gap[nGaps].position = newScreen->atoms[index].y;
                gap[nGaps].height = newScreen->atoms[index].height;
                nGaps++;
                index = mrDeleteAllAtomsWithin(newScreen, index);
                index--;
            }
        }
    }
    //now scan through the remaining atoms and move up any atoms below a given gap
    for (index = nGaps - 1; index >= 0; index--)
    {                                                       //for each gap
        for (j = 0; j < newScreen->nAtoms; j++)
        {                                                   //for each atom
            if (newScreen->atoms[j].y >= gap[index].position)
            {                                               //move all atoms up
                newScreen->atoms[j].y -= gap[index].height;
            }
        }
        newScreen->atoms[0].height -= gap[index].height;//decrease height of screen
    }
    //start the actual menu
    feMenuStart(ghMainRegion, newScreen, mouseCursorX(), mouseCursorY());
    feTempMenuScreen = newScreen;
#undef NUMBER_GAPS
}

/*-----------------------------------------------------------------------------
    Name        : mrClickedOnPlayer
    Description : returns the a pointer to the player you right clicked on.  NULL otherwise/
    Inputs      : none
    Outputs     : player clicked on or NULL
    Return      : void
----------------------------------------------------------------------------*/
/*sdword mrClickedOnPlayer(void)
{
    fonthandle oldfont;
    rectangle  playerColorRect;
    sdword     index;

    if ((mrDrawTactical)&&(multiPlayerGame)&&(multiPlayerGameUnderWay))
    {
        oldfont = fontMakeCurrent(selGroupFont2);

        playerColorRect.y1 = TO_PLAYERLIST_Y-fontHeight("M");
        playerColorRect.y0 = playerColorRect.y1-fontHeight("M");
        playerColorRect.x0 = TO_PLAYERLIST_X;
        playerColorRect.x1 = playerColorRect.x0 + fontHeight(" ");

        //draw the list of player names/colors
        for (index = universe.numPlayers - 1; index >= 0; index--)
        {
            playerColorRect.x1 = fontWidth(playerNames[index]) + fontHeight(" ")*2;
            if ((mouseInRect(&playerColorRect)) && (index!=sigsPlayerIndex))
            {
                return (index);
            }
            playerColorRect.y0 -= fontHeight("M")+1;//update the position
            playerColorRect.y1 -= fontHeight("M")+1;
        }

        fontMakeCurrent(oldfont);
    }

    return (-1);
}*/


/*-----------------------------------------------------------------------------
    Name        : mrRightClickMenu
    Description : Process right-click at current mouse location
    Inputs      : void
    Outputs     : sees what ship was ckicked on and performs appropriate action.
    Return      : void
----------------------------------------------------------------------------*/
void mrRightClickMenu(void)
{
    Ship *ship;
    udword actionMask = 0, shipSelected = 0;
    sdword index;
    TypeOfFormation formation;
    udword tacticsBits = 0;
    fescreen *screen;
    rectangle playerColorRect;

    playerColorRect.x0 = TO_PLAYERLIST_X;
    playerColorRect.x1 = MAIN_WindowWidth;
    playerColorRect.y0 = 0;
    playerColorRect.y1 = MAIN_WindowHeight-TO_PLAYERLIST_Y;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextMenus)
        return;

    if ((ship = selSelectionClick(universe.RenderList.head,
            &(universe.mainCameraCommand.actualcamera),
            mouseCursorX(), mouseCursorY(), FALSE, FALSE)) != NULL)
    {                                                       //if right clicked on a ship
        if (ship->playerowner == universe.curPlayerPtr)
        {
            if (ship->flags & SOF_Selectable)               // make sure it is selectable
            {
                if (selSelected.numShips == 0)
                {                                           //if no ships selected
                    selSelectionSetSingleShip(ship);        //select this cat
                    tacticsBits = ship->tacticstype;

                    if(tutorial)
                        tutFEContextMenuShipType = ship->shiptype;

                    mrMenuDisplay(mrMenuActionsByShipType[ship->shiptype], NO_FORMATION, tacticsBits);
                }
                else
                {                                           //else there are ships selected
                    for (index = 0; index < selSelected.numShips; index++)
                    {
                        if (selSelected.ShipPtr[index] == ship)
                        {                                   //if this is the clicked-upon ship
                            shipSelected = TRUE;
                        }                                   //build ship-type mask
                        actionMask |= mrMenuActionsByShipType[selSelected.ShipPtr[index]->shiptype];
                        tacticsBits |= 1 << selSelected.ShipPtr[index]->tacticstype;
                    }
                    if (shipSelected)
                    {                                       //if ckicked on a selected ship
                        formation = clSelectionAlreadyInFormation(&universe.mainCommandLayer,
                                            (SelectCommand *)(&selSelected));

                        if(tutorial)
                            tutFEContextMenuShipType = ship->shiptype;

                        mrMenuDisplay(actionMask, formation, tacticsBits);//display a menu
                    }
                }
                soundEvent(NULL, UI_Click);
            }
        }
        else
        {
#if MR_VERBOSE_LEVEL >= 1
            dbgMessagef("\nTry clicking on some of your own ships!");
#endif
        }
    }
    else if ( ((playerClickedOn = smClickedOnPlayer(&playerColorRect))!=-1) &&
              (playerClickedOn < sigsNumPlayers) &&
              (mrDrawTactical)&&(multiPlayerGame)&&(multiPlayerGameUnderWay) &&
              (universe.players[sigsPlayerIndex].playerState != PLAYER_DEAD) &&
              (universe.players[playerClickedOn].playerState != PLAYER_DEAD))
    {
        if (bitTest(universe.curPlayerPtr->Allies,PLAYER_MASK(playerClickedOn)))
        {
            screen = feScreenFind("PlayerListRightClickAlly");

            feMenuStart(ghMainRegion, screen, mouseCursorX(), mouseCursorY());
        }
        else
        {
            screen = feScreenFind("PlayerListRightClick");

            feMenuStart(ghMainRegion, screen, mouseCursorX(), mouseCursorY());
        }
    }
#if MR_VERBOSE_LEVEL >= 1
    else
    {
        dbgMessagef("\nDin't click on nuttin'!");
    }
#endif
}

#if MR_TEST_HPB
#define MR_ROTATE_SCALAR        0.007f
extern sdword madTestHPBIndex;
bool mrTestHPBMode = FALSE;
real32 mrHeading, mrPitch, mrBank;
void mrHeadingPitchHold(void)
{
    if (mouseCursorX() != MAIN_WindowWidth / 2)
    {
        mrHeading += (real32)(mouseCursorX() - MAIN_WindowWidth / 2) * MR_ROTATE_SCALAR;
        mousePositionSet(MAIN_WindowWidth / 2, mouseCursorY());
    }
    if (mouseCursorY() != MAIN_WindowHeight / 2)
    {
        mrPitch += (real32)(mouseCursorY() - MAIN_WindowHeight / 2) * MR_ROTATE_SCALAR;
        mousePositionSet(mouseCursorX(), MAIN_WindowHeight / 2);
    }
    mrTestHPBMode = TRUE;
}
void mrBankHold(void)
{
    if (mouseCursorX() != MAIN_WindowWidth / 2)
    {
        mrBank += (real32)(MAIN_WindowWidth / 2 - mouseCursorX()) * MR_ROTATE_SCALAR;
        mousePositionSet(MAIN_WindowWidth / 2, mouseCursorY());
    }
    mrTestHPBMode = TRUE;
}
#endif

// later put in shipselect.c
#define ShipIsntSelectable(x)        bitTest((x)->flags, SOF_Hide|SOF_Disabled|SOF_Crazy)

/*-----------------------------------------------------------------------------
    Name        : MakeShipsSelectable
    Description : Makes sure that the ships in this selection are capable of
                  attack.  Removes any ships from this selection that are not
                  capable of attacking.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsSelectable(SelectCommand *selection)
{
    Ship *ship;
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];
        if (ShipIsntSelectable(ship) || !(ship->flags & SOF_Selectable) || ship->playerowner != universe.curPlayerPtr)
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrFormationShips
    Description : Find the ships for a given mouse click, considering the
                    formation it is in.
    Inputs      : ship - ship clicked on.
    Outputs     : selection - where to store the selection
    Return      : void
----------------------------------------------------------------------------*/
void mrFormationShips(MaxSelection *selection, Ship *ship)
{
    SelectCommand *formSelection;
    MaxSelection selectedInFormation;
    bool parade, groupselect = FALSE;
    //sdword i;

    if (ship->shiptype == Probe)
    {
        selection->ShipPtr[0] = ship;  //select just one probe, because you can only dispatch them 1 at a time anyway
        selection->numShips = 1;
        goto gotselection;
    }

    formSelection = getShipAndItsCommandSelection(&universe.mainCommandLayer, ship, &parade);
    MakeShipsSelectable(formSelection);
    if (formSelection->numShips > 1)
    {                                                           //if this ship is actually in formation
        if (parade)
        {
            selSelectionCopyByType(selection, (MaxSelection *)formSelection,
                             ship->shiptype);

            selSelectionCopyByType(&selectedInFormation, &selSelected,
                             ship->staticinfo->shiptype);
            groupselect = TRUE;

        }
        else
        {
            selSelectionCopy((MaxAnySelection *)selection, (MaxAnySelection *)formSelection);

            selSelectionCopy((MaxAnySelection *)&selectedInFormation, (MaxAnySelection *)&selSelected);
            groupselect = TRUE;
        }

        if (selSelectionCompare((MaxAnySelection *)(selection),
                                (MaxAnySelection *)(&selectedInFormation)) == 0)
        {                                                   //if the current selection is the same as the formation clicked on
            selection->numShips = 0;
        }
        if (selShipInSelection(selSelected.ShipPtr, selSelected.numShips, ship) && keyIsHit(SHIFTKEY))
        {
            selection->numShips = 0;
        }

        if (selection->numShips == 0)
        {                                                   //if everything was pruned
            selection->ShipPtr[0] = ship;                   //select at least this ship
            selection->numShips = 1;
            groupselect = FALSE;
        }
        tutGameMessage("Game_ClickSelectGroup");
    }
    else
    {
        selection->ShipPtr[0] = ship;                       //select just this ship
        selection->numShips = 1;
    }
    memFree(formSelection);

/*
    for(i=0;i<selSelected.numShips;)
    {
        if(selSelected.ShipPtr[i]->playerowner != universe.curPlayerPtr)
        {
            selSelected.numShips--;
            selSelected.ShipPtr[i]=selSelected.ShipPtr[selSelected.numShips];
            continue;
        }
        i++;
    }
*/
    //free the formation selection
gotselection:
#if MR_GUI_SINGLECLICK
    if (selSelected.numShips == 1 && selection->numShips == 1 && selSelected.ShipPtr[0] == selection->ShipPtr[0])
    {
        switch (selSelected.ShipPtr[0]->shiptype)
        {
            case Carrier:
            case Mothership:
/*
                if (mouseCursorSelect.numShips > 0)
                {
                    selSelectionCopy((MaxAnySelection *)&selSelected,
                                     (MaxAnySelection *)&mouseCursorSelect);
                    mouseCursorSelect.numShips = 0;
                    makeShipsDockCapable((SelectCommand *)&selSelected); //filter ships that can't dock
                    clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&selSelected,
                               DOCK_AT_SPECIFIC_SHIP, (Ship *)mouseCursorObjPtr);
                }
                else
                {
*/
                    cmConstructionBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
//                }
                break;
            case ResearchShip:
                rmResearchGUIBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
                break;
            case SensorArray:
                smSensorsBegin(NULL, NULL);
                break;
        }
    }
#endif //MR_GUI_SINGLECLICK
}

/*-----------------------------------------------------------------------------
    Name        : mrObjectClick
    Description : Called when someone clicks on a single ship.  Handles modifiers etc.
    Inputs      : ship - object clicked on (can be a ship, asteroid, dust cloud or derelict)
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mrObjectClick(Ship *ship)
{
    ProtectCommandMax6 protectOne;
    AttackCommandMax6 attackOne;
    MaxSelection tempSelection;
    bool bFriendlies, bEnemies;
    bool bForceAttackEnemies = FALSE;

//    dbgAssert(ship->objtype == OBJ_ShipType);
    if (((keyIsHit(ALTKEY) && keyIsHit(CONTROLKEY)) || keyIsHit(GKEY)) && ship->objtype == OBJ_ShipType)
    {
        if ((playPackets)  || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
        if (!mrDisabled && selSelected.numShips > 0)
        {                                                   //ctrl-alt/g click: guard
            if ((ship->playerowner == universe.curPlayerPtr) || allianceIsShipAlly(ship,universe.curPlayerPtr))
            {
                if (!ShipInSelection((SelectCommand *)&selSelected,ship))
                {
                    MakeShipsGuardCapable((SelectCommand *)&selSelected);

                    if (selSelected.numShips > 0)
                    {
                        protectOne.numShips = 1;
                        protectOne.ShipPtr[0] = ship;

                        MakeShipMastersIncludeSlaves((SelectCommand *)&protectOne);
                        speechEvent(selSelected.ShipPtr[0], COMM_Guard, 0);
                        clWrapProtect(&universe.mainCommandLayer,(SelectCommand *)&selSelected,(ProtectCommand *)&protectOne);
                        tutGameMessage("Game_ClickGuard");
                    }
                }
            }
        }
    }
    else if (keyIsHit(ALTKEY))
    {                                                       //alt-click: focus on a ship
#if MR_CAN_FOCUS_ROIDS
        if ((mrCanFocusRoids && !multiPlayerGame) || ship->objtype == OBJ_ShipType || ship->objtype == OBJ_DerelictType)
#else
        if (ship->objtype == OBJ_ShipType || ship->objtype == OBJ_DerelictType)
#endif
        {
            selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)ship; //make a temporary dummy selection
            selSelecting.numTargets = 1;
            MakeShipMastersIncludeSlaves((SelectCommand *)&selSelecting);

            tutGameMessage("Game_ClickFocus");
            ccFocusClose(&universe.mainCameraCommand,
                    (FocusCommand *)&selSelecting);         //focus on these ships
            selSelecting.numTargets = 0;
        }
    }
    else if (kbCommandKeyIsHit(kbSHIP_SPECIAL))
    {                                                       //z-key: special target operation
        if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
        if (mrCanZBandBox(&bFriendlies, &bEnemies))
        {                                                   //if we can z-bandbox
            selSelecting.numTargets = 1;
            selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)ship;
            if (bEnemies)
            {                                               //if we're salvaging
                if (ShiptypeInSelection((SelectCommand *)&selSelected, SalCapCorvette))
                {                                           //get only salvageable ships
                    MakeTargetsSalvageable((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);
                }
                else
                {                                           //select enemies only
                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);
                }
            }
            else if (bFriendlies)
            {                                               //select friendlies only
                MakeShipsFriendlyShips((SelectCommand *)&selSelecting, universe.curPlayerPtr);
            }
            MakeShipsSpecialTargetCapable((SelectCommand *)&selSelected, bFriendlies);
            if (selSelected.numShips > 0)   // check again in case we eliminated some of our ships
            {
                MakeShipsNotIncludeTheseShips((SelectCommand *)&selSelecting,(SelectCommand *)&selSelected);
//                MakeTargetsNotIncludeMissiles((SelectAnyCommand *)&selSelecting);
                MakeShipMastersIncludeSlaves((SelectCommand *)&selSelecting);

                if (selSelecting.numTargets > 0)// check again in case we eliminated all targets
                {
                    clWrapSpecial(&universe.mainCommandLayer,(SelectCommand *)&selSelected,(SpecialCommand *)&selSelecting);
                    tutGameMessage("Game_ClickSpecial");
                }
            }
            selSelecting.numTargets = 0;
        }
    }
    else if ((ship->objtype == OBJ_DerelictType || (ship->objtype == OBJ_ShipType &&
                (!allianceIsShipAlly(ship, universe.curPlayerPtr)))) &&
                keyIsHit(CONTROLKEY) && keyIsHit(SHIFTKEY))
    {                                                       //control-shift-click on derelict or enemy:attack
        if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
        goto regularEnemyCase;
    }
    else if (((ship->objtype == OBJ_DerelictType) ||
              (ship->objtype == OBJ_ShipType)) &&
             (MakeShipsSingleClickSpecialCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected)))
    {                                                       //single-click special ships
        if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
        selSelecting.numTargets = 1;                        //selSelecting = targets, tempSelection = singlclickspecialactivate ships
        selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)ship;

        //make sure the target is a single click special capable target
        if (ShiptypeInSelection((SelectCommand *)&tempSelection, SalCapCorvette))
        {
            MakeTargetsSalvageable((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);
        }
        else
        {
            MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);
        }

        if (selSelecting.numTargets)
        {
            clWrapSpecial(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(SpecialCommand *)&selSelecting);
            selSelecting.numTargets = 0;
            goto regularEnemyCase;
        }
        else if (ship->objtype != OBJ_DerelictType)
        {
            if (ship->playerowner == universe.curPlayerPtr)
            {
                goto regularFriendlyCase;
            }
            else if (!allianceIsShipAlly(ship, universe.curPlayerPtr))
            {
                goto regularEnemyCase;
            }
        }
    }
    else if ((ship->objtype == OBJ_AsteroidType || ship->objtype == OBJ_DustType) &&
             keyIsHit(CONTROLKEY) && keyIsHit(SHIFTKEY))
    {                                                   //ctrl-shift-click resource: force attack
        if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
        goto regularEnemyCase;
    }
    else if ((ship->objtype == OBJ_AsteroidType || ship->objtype == OBJ_DustType) &&
             MakeShipsHarvestCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected))
    {                                                   //there are harvesters present
        if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
        speechEvent(tempSelection.ShipPtr[0], COMM_ResCol_Harvest, (ship->objtype - OBJ_AsteroidType));
        clWrapCollectResource(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(Resource *)ship);
        soundEventStartEngine(tempSelection.ShipPtr[0]);
        tutGameMessage("Game_ClickHarvest");
    }
    else if ((ship->objtype == OBJ_AsteroidType) &&
             (ship->attributes & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage)) )
    {
        if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
        goto regularEnemyCase;
    }
    else if ((ship->objtype == OBJ_DerelictType) && (((Derelict *)ship)->derelicttype == HyperspaceGate))
    {
        if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
        goto regularEnemyCase;
    }
    else if (!mrDisabled)
    {
        if (ship->objtype == OBJ_ShipType)
        {
            if (ship->playerowner == universe.curPlayerPtr)
            {                                               //clicked on a friendly ship
regularFriendlyCase:
                if (keyIsHit(CONTROLKEY) && keyIsHit(SHIFTKEY) && ship->shiptype != CryoTray)
                {                                           //ctrl-shift-click friendly ship: attack
                    if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
                    bForceAttackEnemies = TRUE;
                    goto regularEnemyCase;
                }
                else if (ship->flags & SOF_Selectable)
                {
                    if (keyIsHit(SHIFTKEY))
                    {                                       //shift click:add/remove to/from selection
                        if (selShipInSelection(selSelected.ShipPtr, selSelected.numShips, ship))
                        {                                   //if ship already selected
                            selSelectionRemoveSingleShip(&selSelected, ship);
                        }
                        else
                        {
                            selSelectionAddSingleShip(&selSelected, ship);
                        }
                        ioUpdateShipTotals();
                    }
                    else
                    {                                       //regular click: select just this ship
                        mrFormationShips(&tempSelection, ship);
                        selSelectionCopy((MaxAnySelection *)(&selSelected),
                                         (MaxAnySelection *)(&tempSelection));
                        ioUpdateShipTotals();

                        if ((playPackets) || (universePause) || (mrDisabled)) return;            // playing back a recorded game do nothing!
                        if (selShipInSelection(tempSelection.ShipPtr, tempSelection.numShips, universe.curPlayerPtr->PlayerMothership)
                            && (universe.curPlayerPtr->PlayerMothership->shiptype == Mothership))
                        {                                   //if the mothership was in selection clicked
                            speechEventFleet(COMM_F_MoShip_Selected,0,universe.curPlayerIndex);
                        }
                        else
                        {                                   //no mothership: regular selection
                            speechEvent(ship, COMM_Selection, 0);
                        }
                    }
                }
            }
            else    // must be clicking on enemy ship
            {
                if ((playPackets) || (universePause) || (mrDisabled)) return;// playing back a recorded game do nothing!
                if (ship->shiptype == FloatingCity) //trader ship
                {

                    tmTradeBegin(ghMainRegion, 0, 0, 0);

                }
                else if (ship->shiptype == JunkYardHQ)
                {
                    makeShipsDockCapable((SelectCommand *)&selSelected); //filter ships that can't dock
                    clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&selSelected,
                               DOCK_AT_SPECIFIC_SHIP, ship);
                }
                else if (!allianceIsShipAlly(ship, universe.curPlayerPtr))
                {
regularEnemyCase:
                    if (selSelected.numShips > 0)
                    {
                        if (MakeShipsAttackCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected))
                        {                                   //if there are ships that can attack
                            if (!selShipInSelection(selSelected.ShipPtr, selSelected.numShips, ship))
                            {                               //if not attacking your own ship
                                attackOne.numTargets = 1;
                                attackOne.TargetPtr[0] = (SpaceObjRotImpTarg *)ship;
                                MakeShipMastersIncludeSlaves((SelectCommand *)&attackOne);

                                if (bForceAttackEnemies)
                                {                           //if it's a forced attack on friendlies
                                    selSelecting.numTargets = 1;
                                    selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)ship;
                                    if (speechEventAttack())
                                    {
                                        clWrapAttack(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(SelectAnyCommand *)&attackOne);
                                        selSelecting.numTargets = 0;
                                    }
                                }
                                else
                                {                           //else regular attack on enemies
                                    speechEvent(tempSelection.ShipPtr[0], COMM_Attack, 0);
                                    clWrapAttack(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(SelectAnyCommand *)&attackOne);
                                    tutGameMessage("Game_ClickAttack");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrRegionProcess
    Description : Region logic handler for main region.
    Inputs      : same as any region processor
    Outputs     : ..
    Return      : same as any region processor
----------------------------------------------------------------------------*/
udword mrRegionProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    Ship *ship;
    bool bFriendlies, bEnemies;
    MaxSelection tempSelection;


    rectangle defaultRect = {0, 0, MAIN_WindowWidth, MAIN_WindowHeight};

    while (feMenuLevel > 0)
    {
        feMenuDisappear(NULL, NULL);
    }
    if (smZoomingIn || smZoomingOut)
    {
        return(0);
    }
    switch (event)
    {
        case RPE_HoldLeft:
            mrHoldLeft();                                   //call current hold function
            break;
        case RPE_HoldRight:
            mrHoldRight();                                  //call current hold function
            break;
       case RPE_PressLeft:
            if (mrHoldRight == mrNULL)
            {                                               //if mouse idle or selecting
#if MR_TEST_HPB
                if (keyIsHit(OKEY))
                {
                    mrHoldLeft = mrHeadingPitchHold;
                }
                else
#endif
                if (piePointSpecMode == PSM_Idle)
                {
                    mrOldMouseX = mouseCursorX();           //store anchor point
                    mrOldMouseY = mouseCursorY();
                    mrHoldLeft = mrSelectHold;              //set to select mode
                }
                else if (keyIsHit(ALTKEY))
                {
                    mrOldMouseX = mouseCursorX();           //store anchor point
                    mrOldMouseY = mouseCursorY();
                    mrHoldLeft = mrSelectHold;              //set to select mode
                    piePointModePause(TRUE);
                }
            }                                               //else just go from rotate into zoom mode
            break;
        case RPE_PressRight:
#if MR_TEST_HPB
            if (keyIsHit(OKEY))
            {
                mrHoldRight = mrBankHold;
                break;
            }
            else
#endif
            if (mrHoldLeft == mrSelectHold)
            {                                               //if currently selecting
                mrHoldLeft = mrNULL;                        //stop selecting
                mrSelectionRect.x0 = mrSelectionRect.x1 =           //select nothing
                    mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
                selRectNone();
            }
#if !MR_TEST_HPB
            else
            {                                               //else just right button pressed
                dbgAssert(mrHoldLeft == mrNULL);            //should be in idle mode.  Make sure.
            }
#endif
            piePointModePause(TRUE);                         //pause point specification for the camera motion
            mrHoldRight = mrCameraMotion;                   //set to zoom/rotate mode
            mrOldMouseX = mouseCursorX();                   //save current mouse location for later restoration
            mrOldMouseY = mouseCursorY();
            mouseCursorHide();                              //hide cursor and move to centre of the screen

            if (helpinfoactive)
            {
                mousePositionSet(helpinforegion->rect.x0 / 2, MAIN_WindowHeight / 2);
            }
            else
            {
                mousePositionSet(MAIN_WindowWidth / 2, MAIN_WindowHeight / 2);
            }

            mrMouseHasMoved = 0;                            //mouse hasn't moved yet
            break;
        case RPE_DoubleLeft:
            if (mouseCursorObjPtr != NULL)
            {
                if (!mouseLDoubleClick())
                {
                    if (mouseCursorSelect.numShips > 0)
                    {
                        selSelectionCopy((MaxAnySelection *)&selSelected,
                                         (MaxAnySelection *)&mouseCursorSelect);
                        ioUpdateShipTotals();\
                        mouseCursorSelect.numShips = 0;
                        tutGameMessage("Game_DoubleClickDock");
                        makeShipsDockCapable((SelectCommand *)&selSelected); //filter ships that can't dock
                        clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&selSelected,
                                   DOCK_AT_SPECIFIC_SHIP, (Ship *)mouseCursorObjPtr);
                        mouseDrawType(small_docking);           //may need to have "docking" as well
                        break;
                    }
                }
                else
                {
                    //mouseLDoubleClick returned true, so break outta here
                    break;
                }
            }
            goto plainOldClickAction;
        case RPE_ReleaseLeft:
            if (mouseRightButton())
            {                                               //if in zoom mode
                ;                                           //stay in camera movement mode
            }
            else if (piePointSpecMode == PSM_XY || piePointSpecMode == PSM_Z)
            {                                               //if the movement mechanism is up
                vector destination;

                if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bMoveIssue)
                {
                    break;
                }

                MakeShipsMobile((SelectCommand *)&selSelected);
                if (selSelected.numShips > 0)
                {                                           //if some ships are selected
#if PIE_MOVE_NEARTO
                    if (selClosestTarget != NULL)
                    {                                       //if there's a ship under the mouse
                        destination = selClosestTarget->posinfo.position;
                    }
                    else
#endif //PIE_MOVE_NEARTO
                    if (piePointSpecZ != 0.0f)
                    {                                       //if a height was specified
                        destination = pieHeightPoint;
                    }
                    else
                    {
                        destination = piePlanePoint;         //else move to point on plane
                    }

                    if (!(_isnan((double)destination.x) || _isnan((double)destination.x) || _isnan((double)destination.x)))
                    {
                        clWrapMove(&universe.mainCommandLayer,(SelectCommand *)&selSelected,selCentrePoint,destination);
//                        tutGameMessage("Game_MoveIssued");
                        if (selSelected.ShipPtr[0]->shiptype == Mothership)
                        {
                            speechEventFleetSpec(selSelected.ShipPtr[0], COMM_F_MoShip_Move, 0, universe.curPlayerIndex);
                        }
                        else
                        {
                            speechEvent(selSelected.ShipPtr[0], COMM_Move, 0);
                        }
                    }

                    soundEvent(NULL, UI_MovementGUIoff);
                }
                else
                {
                    soundEvent(NULL, UI_MovementGUIoff);
                }

                piePointModeOnOff();
//                piePointSpecMode = PSM_Idle;                 //no more selecting
                break;
            }
            else if (mrHoldLeft == mrSelectHold)
            {                                               //if selecting
                if (abs(mouseCursorX() - mrOldMouseX) >= selClickBoxWidth ||
                    abs(mouseCursorY() - mrOldMouseY) >= selClickBoxHeight)
                {                                           //if selection rect dragged
                    mrSelectRectBuild(&mrSelectionRect, mrOldMouseX, mrOldMouseY);//select a group of ships
                    if ((keyIsHit(ALTKEY) && keyIsHit(CONTROLKEY)) | keyIsHit(GKEY))
                    {                                       //alt key pressed
//                        if (keyIsHit(CONTROLKEY))
//                        {                                   //ctrl-alt bandbox: guard
                        if (mrDisabled) goto endReleaseButtonLogic;

                        selRectDragAnybody(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);//most recent selection
                        if (selSelecting.numTargets > 0)
                        {                                   //if some ships within band-box
                            if (selSelected.numShips > 0)   // if we have our own ships selected
                            {
                                MakeShipsFriendlyAndAlliesShips((SelectCommand *)&selSelecting,universe.curPlayerPtr);
                                MakeShipsNotIncludeTheseShips((SelectCommand *)&selSelecting,(SelectCommand *)&selSelected);

                                MakeShipMastersIncludeSlaves((SelectCommand *)&selSelecting);

                                MakeShipsGuardCapable((SelectCommand *)&selSelected);

                                if ((selSelecting.numTargets > 0) && (selSelected.numShips > 0))      // check again in case we removed all the ships
                                {
                                    speechEvent(selSelected.ShipPtr[0], COMM_Guard, 0);
                                    clWrapProtect(&universe.mainCommandLayer,(SelectCommand *)&selSelected,(ProtectCommand *)&selSelecting);
                                    tutGameMessage("Game_BandBoxGuard");
                                }
                            }
                            selSelecting.numTargets = 0;
                        }
//                        }
                    }
                    else if (keyIsHit(ALTKEY))
                    {                                       //alt-bandbox: focus
                        selRectDragAnybody(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);//most recent selection
                        if (selSelecting.numTargets != 0)
                        {                                   //if some ships within band-box
                            MakeShipMastersIncludeSlaves((SelectCommand *)&selSelecting);

                            ccFocusClose(&(universe.mainCameraCommand),
                                    (FocusCommand *)&selSelecting);//focus on these ships
                            selSelecting.numTargets = 0;
                            tutGameMessage("Game_BandBoxFocus");
                        }
                    }
                    else if (keyIsHit(CONTROLKEY))
                    {                                       //CTRL-bandbox: attack selected ships
                        if (mrDisabled) goto endReleaseButtonLogic;

                        if (MakeShipsSingleClickSpecialCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected))
                        {                                       //if some ships single-click special attack capable
                            selRectDragAnybody(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);//most recent selection
                            MakeShipMastersIncludeSlaves((SelectCommand *)&selSelecting);

                            if (ShiptypeInSelection((SelectCommand *)&tempSelection, SalCapCorvette))
                            {
                                MakeTargetsSalvageable((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);
                            }
                            else
                            {
                                MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&selSelecting,universe.curPlayerPtr);
                            }

                            if (selSelecting.numTargets != 0)
                            {
                                clWrapSpecial(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(SpecialCommand *)&selSelecting);
                            }
                        }
                        selRectDragAnythingToAttack(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);//most recent selection
                        if (selSelecting.numTargets > 0)
                        {                                   //if some ships within band-box
                            if (selSelected.numShips > 0)       // if we have our own ships selected
                            {
                                MakeShipsNotIncludeTheseShips((SelectCommand *)&selSelecting,(SelectCommand *)&selSelected);
                                if (!keyIsHit(SHIFTKEY))    //CTRL-Shift bandbox: force attack
                                {
                                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&selSelecting,universe.curPlayerPtr);
                                }
/*
                                if (MakeShipsSingleClickSpecialCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected))
                                {                                       //if some ships single-click special attack capable
                                    clWrapSpecial(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(SpecialCommand *)&selSelecting);
                                }
*/
                                MakeShipsAttackCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected);
                                MakeShipMastersIncludeSlaves((SelectCommand *)&selSelecting);

                                if ((selSelecting.numTargets > 0) && (tempSelection.numShips > 0))      // check again in case we removed all the ships
                                {
                                    if (speechEventAttack())
                                    {
                                        clWrapAttack(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(AttackCommand *)&selSelecting);
                                        if(keyIsHit(SHIFTKEY))    //CTRL-Shift bandbox: force attack
                                            tutGameMessage("Game_BandBoxForceAttack");
                                        tutGameMessage("Game_BandBoxAttack");
                                    }
                                }
                            }
                            selSelecting.numTargets = 0;
                        }
                    }
                    else if (kbCommandKeyIsHit(kbSHIP_SPECIAL))
                    {                                       //z-bandbox: special action
                        if (mrDisabled) goto endReleaseButtonLogic;

                        if (mrCanZBandBox(&bFriendlies, &bEnemies))
                        {                                   //if we can z-bandbox
                            if (bEnemies)
                            {                               //select enemies and friends
                                selRectDragAnybody(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
                            }
                            else
                            {                               //select just friends
                                selRectDrag(&(universe.mainCameraCommand.actualcamera), &mrSelectionRect);
                            }
                            if (!bFriendlies)
                            {
                                if (ShiptypeInSelection((SelectCommand *)&selSelected, SalCapCorvette))
                                {
                                    MakeTargetsSalvageable((SelectAnyCommand *)&selSelecting, universe.curPlayerPtr);
                                }
                                else
                                {
                                    //remove friends from selection
                                    mrRemoveDerelictsAndAllPlayerShipsFromSelecting();
                                }
                            }
                        }
                        else
                        {                                   //can't select anything
                            selRectNone();
                        }
                        if (selSelecting.numTargets > 0)
                        {                                   //if some ships within band-box
                            MakeShipsSpecialTargetCapable((SelectCommand *)&selSelected, bFriendlies);
                            if (selSelected.numShips > 0)   // check again in case we eliminated some of our ships
                            {
                                mrSpecialBandBox = TRUE;
                                MakeShipsNotIncludeTheseShips((SelectCommand *)&selSelecting,(SelectCommand *)&selSelected);
                                MakeTargetsNotIncludeMissiles((SelectAnyCommand *)&selSelecting);
                                MakeShipMastersIncludeSlaves((SelectCommand *)&selSelecting);

                                if (selSelecting.numTargets > 0)// check again in case we eliminated all targets
                                {
                                    clWrapSpecial(&universe.mainCommandLayer,(SelectCommand *)&selSelected,(SpecialCommand *)&selSelecting);
                                    tutGameMessage("Game_BandBoxSpecial");
                                }
                            }
                            selSelecting.numTargets = 0;
                        }
                    }
                    else
                    {                                       //plain bandbox: select ships
                        if((!mrDisabled && piePointSpecMode != PSM_Waiting) && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bBandSelect))
                        {
                            if (keyIsHit(SHIFTKEY))
                            {                               //if shift-band-boxing
                                selRectSelectAdd(&(universe.mainCameraCommand.actualcamera),
                                                 &mrSelectionRect);
                            }
                            else
                            {                               //else plain band-boxing
                                if((!(tutorial==TUTORIAL_ONLY)) ||              // if the tutorial isn't running OR
                                    ((selSelecting.numTargets != 0) || ((selSelecting.numTargets == 0) && tutEnable.bCancelSelect)))
                                {                           //band-selecting no ships disabled if tutorial's cancel select disabled
                                    selRectSelect(&(universe.mainCameraCommand.actualcamera),
                                                  &mrSelectionRect);
                                    if ((selSelected.numShips > 0) && (!playPackets) && (!universePause))
                                    {
                                        if (selShipInSelection(selSelected.ShipPtr, selSelected.numShips, universe.curPlayerPtr->PlayerMothership)
                                            && (universe.curPlayerPtr->PlayerMothership->shiptype == Mothership))
                                        {                                   //if the mothership was in selection clicked
                                            speechEventFleet(COMM_F_MoShip_Selected,0,universe.curPlayerIndex);
                                        }
                                        else
                                        {                                   //no mothership: regular selection
                                            speechEvent(selSelected.ShipPtr[0], COMM_Selection, 0);
                                        }
                                    }
                                }
                            }

                            ioUpdateShipTotals();

                            selSelecting.numTargets = 0;
                        }
                    }
                }
                else
                {                                           //no band-box, just a click
plainOldClickAction:
                    if (mrHoldRight == mrNULL)
                    {
                        ship = selSelectionClick(universe.RenderList.head,
                            &(universe.mainCameraCommand.actualcamera),
                            mouseCursorX(), mouseCursorY(), TRUE, TRUE);

                        if (ship != NULL)
                        {                                       //if mouse over a ship
                            if((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bClickSelect)
                            {
                                mrObjectClick(ship);
                            }
                        }
                        else
                        {                                       //else select nothing
                            if (mouseCursorObjLast != NULL &&   //if mouse recently over an object
                                mouseCursorObjLast->objtype == OBJ_ShipType &&//a ship to be particular
                                mouseCursorLastObjTime + mrFastMoveShipClickTime >= universe.totaltimeelapsed && //recently enough
                                abs(mouseCursorX() - mouseCursorLastX) < mrFastMoveShipClickX && //in roughly it's current position
                                abs(mouseCursorY() - mouseCursorLastY) < mrFastMoveShipClickY)
                            {
                                if((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bClickSelect)
                                    mrObjectClick((Ship *)mouseCursorObjLast);//pretend we just clicked on this ship
                            }
                            else if (!keyIsHit(SHIFTKEY))            //if shift key not pressed
                            {
                                if((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bCancelSelect)
                                    selSelectNone();
                            }
                        }
                    }
                }
endReleaseButtonLogic:
                mrSelectionRect.x0 = mrSelectionRect.x1 =   //clear selection rectangle
                    mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
                mrHoldLeft = mrNULL;                        //idle mode
                piePointModePause(FALSE);
            }
#if MR_TEST_HPB
            if (mrHoldLeft == mrHeadingPitchHold)
            {
                mrHoldLeft = mrNULL;                        //idle mode
            }
            if (mrHoldRight == mrBankHold)
            {
                mrHoldRight = mrNULL;
            }
#endif
            break;
    case RPE_ReleaseRight:
            if (helpinfoactive)
            {

            }
            if (mrHoldRight == mrCameraMotion)
            {                                               //if in camera movement mode
                piePointModePause(FALSE);                    //unpause point specification for the camera motion
                mousePositionSet(mrOldMouseX, mrOldMouseY); //restore mouse position
                //mouseCursorShow();                          //show mouse cursor
                mouseCursorShow();
                mrHoldRight = mrNULL;                       //idle mode
                mouseClipToRect(&defaultRect);

                if (!mrDisabled && mrMouseHasMoved <= MR_MouseMovementClickLimit)
                {                                           //if it's moved a fair bit
                    mrRightClickMenu();                     //process the right click
                }
            }
#if MR_TEST_HPB
            if (mrHoldRight == mrBankHold)
            {
                mrHoldRight = mrNULL;
            }
#endif
            break;
        case RPE_KeyDown:
            if (!(smZoomingIn | smZoomingOut))
            {
                mrKeyPress(ID);
            }
            break;
        case RPE_KeyUp:
            if (!(smZoomingIn | smZoomingOut))
            {
                mrKeyRelease(ID);
            }
            break;
        case RPE_WheelUp:
            wheel_up = TRUE;
            break;
        case RPE_WheelDown:
            wheel_down = TRUE;
            break;
        default:
#if MR_VERBOSE_LEVEL >= 2
            dbgMessagef("\nmrRegionProcess: unprocessed message. reg = 0x%x, ID = 0x%x, event = 0x%x, data = %d", reg, ID, event, data);
#endif
            break;
    }
    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : mrPlayerNameDraw
    Description : Draw the name of a particular player
    Inputs      : playerIndex - index of player who's name we are to print
                  x, y - location to print
    Outputs     : Draws names of specified multi-player player
    Return      : void
----------------------------------------------------------------------------*/
/*
void mrPlayerNameDraw(sdword playerIndex, sdword x, sdword y)
{
    color c;

#if TO_STANDARD_COLORS
    if (&universe.players[playerIndex] == universe.curPlayerPtr)
    {
        c = teFriendlyColor;
    }
    else if (allianceArePlayersAllied(&universe.players[playerIndex], universe.curPlayerPtr))
    {
        c = teAlliedColor;
    }
    else
    {
        c = teHostileColor;
    }

#else
    c = teColorSchemes[playerIndex].tacticalColor;
#endif
    fontPrint(x, y, c, playerNames[playerIndex]);
}
*/

/*-----------------------------------------------------------------------------
    Name        : mrCommandMessageDraw
    Description : Draw the current list of player commands
    Inputs      : None (uses global variable gMessage)
    Outputs     : Draws list of latest player commands
    Return      : void
----------------------------------------------------------------------------*/
void mrCommandMessageDraw(void)
{
    ubyte  i = MAX_MESSAGES;
    udword j = MR_MESSAGE_LINE_SPACING;

    LockMutex(gMessageMutex);

    if (gMessage[0].message[0] == (char)NULL)
        goto done;

    // find the last valid message in global array
    while (gMessage[i].message[0] == (char)NULL)
    {
        i--;
    }

    // print it
    fontPrint(MR_MESSAGE_LINE_START, 0, colWhite, gMessage[i].message);

    // print remaining messages in global array
    while (i > 0)
    {
        i--;
        fontPrint(MR_MESSAGE_LINE_START, j, colWhite, gMessage[i].message);
        j += MR_MESSAGE_LINE_SPACING;
    }

    // remove expired messages
    while ((gMessage[0].MessageExpire < universe.totaltimeelapsed) &&
           (gMessage[0].message[0] != (char)NULL))
    {
        // shift up remaining messages to fill in void left by expired message
        while (gMessage[i+1].message[0] != (char)NULL)
        {
            strcpy(gMessage[i].message, gMessage[i+1].message);
            gMessage[i].MessageExpire = gMessage[i+1].MessageExpire;
            i++;
        }
        gMessage[i].message[0]  = (char)NULL;
        i = 0;
    }

done:
    UnLockMutex(gMessageMutex);
}

void bigmessageDisplay(char *msg,sdword position)
{
    dbgAssert(position < MAX_BIGMESSAGES);

    bMessage[position].messageOn = TRUE;
    dbgAssert(strlen(msg) < MAX_BIGMESSAGE_LENGTH);

    strcpy(bMessage[position].message,msg);
}

void bigmessageErase(sdword position)
{
    bMessage[position].messageOn = FALSE;
}

static sdword bigmessageYPosition[MAX_BIGMESSAGES] = { 200,260 };

void mrBigMessageDraw(void)
{
    sdword i;
    fonthandle fhSave;

    dbgAssert(mrBigFont != 0);

    for (i=0;i<MAX_BIGMESSAGES;i++)
    {
        if (bMessage[i].messageOn)
        {
            fhSave = fontCurrentGet();
            fontMakeCurrent(mrBigFont);

            fontPrintCentre(bigmessageYPosition[i],colWhite,bMessage[i].message);

            fontMakeCurrent(fhSave);
        }
    }
}

//
// Move Line Functions
//
// NOTE: This code is supposed to reside in tactical.c, but for some reason nothing
//       gets rendered when it is moved there.  Therefore, these functions should stay
//       here.  Move them at your own risk!
#define TO_MOVE_LINE_COLOR              TW_MOVETO_LINE_COLOR
#define TO_MOVE_LINE_PULSE_COLOR        TW_MOVETO_PULSE_COLOR
#define TO_ATT_MOVE_LINE_COLOR          TW_ATTMOVETO_LINE_COLOR
#define TO_ATT_MOVE_LINE_PULSE_COLOR    TW_ATTMOVETO_PULSE_COLOR
#define TO_PULSE_CLOSE_SHRINK           0.8f
#define TO_FADE_SIZE                    2  //*pulsesize
#define TO_MOVE_LINE_RADIUS_STRETCH     TW_MOVETO_CIRCLE_RADIUS
#define TO_PULSE_SPEED_SCALE            TW_MOVETO_PULSE_SPEED_SCALE
#define TO_MOVETO_ENDCIRCLE_RADIUS      TW_MOVETO_ENDCIRCLE_RADIUS

//a kludgy global variable
bool pulse_at_beginning = FALSE;

/*-----------------------------------------------------------------------------
    Name        : toDrawPulsedLine
    Description : Draws a line with a cool ass pulse flying down it at incredible velocities
    Inputs      : linestart  - where the line starts
                  lineend    - where the line ends
                  pulsesize  - how big that cool ass pulse is
                  linecolor  - the color of the line
                  pulsecolor - the color of the cool ass pulse
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
bool toDrawPulsedLine(vector linestart, vector lineend, real32 pulsesize, color linecolor, color pulsecolor)
{
    static real32 lasttime = 0.0, pulsestartfraction = 0.0;
    real32 distance, pulsedistance, fadesize;
    vector dirvect, pulsestart, pulseend, fadestart, fadeend;
    bool pulse_at_end = FALSE;
    udword test = 0;
    bool draw_pulse_beg = TRUE, draw_fadein = FALSE, draw_fadeout = FALSE;

    pulse_at_beginning = FALSE;

    if (universe.totaltimeelapsed != lasttime)
    {
        pulsestartfraction += (universe.totaltimeelapsed - lasttime)/TO_PULSE_SPEED_SCALE;
        lasttime            = universe.totaltimeelapsed;

        // - first check is for the very first iteration of toDrawPulsedLine where
        //   "universe.totaltimeelapsed" will be much larger that "lasttime" and
        //   therefore "pulsestartfraction" is quite large
        // - the second check is to fix a very obscure bug where "lasttime" somehow
        //   gets set to a value larger than "universe.totaltimeelapsed".  We can't
        //   figure out why "lasttime" does this, though...
        if ((pulsestartfraction > 1.0) || (pulsestartfraction < 0.0))
        {
            pulsestartfraction = 0.0;
            pulse_at_beginning = TRUE;
        }
    }

    //calculate the pulse
    //get the unit direction vector of the pulse line
    vecSub(dirvect, lineend, linestart);
    distance = (real32) fsqrt(vecMagnitudeSquared(dirvect));
    vecNormalize(&dirvect);

    //find the start and end points of the pulse
    pulsedistance = pulsestartfraction * distance;

    while (pulsesize > distance/2)
    {
        pulsesize *= TO_PULSE_CLOSE_SHRINK;
    }

    //if the pulse is at the end of the moveto line
    if (pulsedistance > (distance + pulsesize))
    {
        pulsestartfraction = 0.0;
        pulse_at_beginning = TRUE;
    }

    //calculate the fade regions
    fadesize = TO_FADE_SIZE*pulsesize;

    //if the pulse is far enough away from the beginning of the moveto line to have a fadein region
    if (pulsedistance > fadesize)
    {
        vecScalarMultiply(fadestart, dirvect, pulsedistance - fadesize);
        vecAddTo(fadestart, linestart);
        draw_fadein = TRUE;
    }
    else if (pulsedistance < pulsesize + fadesize)
    {
        pulse_at_beginning = TRUE;

        if (pulsedistance < pulsesize)
        {
            draw_pulse_beg = FALSE;
        }
    }

    //if the pulse is far enough away from the end of the moveto line to have a fadeout region
    if (pulsedistance < (distance - fadesize - pulsesize))
    {
        vecScalarMultiply(fadeend, dirvect, (pulsedistance + pulsesize + fadesize));
        vecAddTo(fadeend, linestart);
        draw_fadeout = TRUE;
    }

    if ((pulsedistance > (distance - fadesize/3 - pulsesize)) ||
        (distance < TW_MOVETO_ENDCIRCLE_RADIUS))
    {
        pulse_at_end = TRUE;
    }

    //find the point locations of the start and end of the pulse
    vecScalarMultiply(pulsestart, dirvect, (pulsedistance /*- (pulsestartfraction * pulsesize)*/));
    vecAddTo(pulsestart, linestart);
    vecScalarMultiply(pulseend, dirvect, (pulsedistance + (pulsestartfraction * pulsesize)));
    vecAddTo(pulseend, linestart);


    glBegin(GL_LINES);
    {
        if (draw_pulse_beg)
        {
            //draw the line from the ship to the pulse
            glColor3ub(colRed(linecolor), colGreen(linecolor), colBlue(linecolor));
            glVertex3fv((GLfloat *)&linestart);

            //draw the fadein
            if (draw_fadein)
            {
                glVertex3fv((GLfloat *)&fadestart);
                glVertex3fv((GLfloat *)&fadestart);
            }
            glColor3ub(colRed(pulsecolor), colGreen(pulsecolor), colBlue(pulsecolor));
            glVertex3fv((GLfloat *)&pulsestart);

        }
        else
        {
            glColor3ub(colRed(pulsecolor), colGreen(pulsecolor), colBlue(pulsecolor));
            glVertex3fv((GLfloat *)&linestart);
            glVertex3fv((GLfloat *)&pulsestart);
        }

        //draw the pulse
        glVertex3fv((GLfloat *)&pulsestart);

        if (!pulse_at_end)
        {
            glVertex3fv((GLfloat *)&pulseend);

            //draw the line from the pulse to the end of the line
            glVertex3fv((GLfloat *)&pulseend);
            glColor3ub(colRed(linecolor), colGreen(linecolor), colBlue(linecolor));

            //draw the fadeout
            if (draw_fadeout)
            {
                glVertex3fv((GLfloat *)&fadeend);
                glVertex3fv((GLfloat *)&fadeend);
            }
        }
        glVertex3fv((GLfloat *)&lineend);
    }
    glEnd();

    return pulse_at_end;
}


/*-----------------------------------------------------------------------------
    Name        : toDrawMoveToCircle
    Description : Draws a circle at the destination of the ship
    Inputs      : ship - the ship
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toDrawMoveToEndCircle(ShipPtr ship, bool pulse, color linecolor, color pulsecolor)
{
    if (pulse)
    {
        primCircleOutline3(&ship->moveTo, TO_MOVETO_ENDCIRCLE_RADIUS,
                           32, 0, pulsecolor, Z_AXIS);
    }
    else
    {
        primCircleOutline3(&ship->moveTo, TO_MOVETO_ENDCIRCLE_RADIUS,
                            32, 0, linecolor, Z_AXIS);
    }
}



/*-----------------------------------------------------------------------------
    Name        : toDrawMoveToLine
    Description : Draws a single move line from a ship to it's move point
    Inputs      : ship - the aformentioned ship
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toDrawMoveToLine(ShipPtr ship, color linecolor, color pulsecolor)
{
    vector shipfront, shipright, dirfrontproj, dirrightproj, dirvect, dirproj,
           zero = {0,0,0}, negdir;
    real32 distance, pulsesize, shipradius;
    bool pulse = FALSE;

    //initialize a bunch of vectors and distances
    shipradius  = ship->magnitudeSquared*ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize;
    pulsesize   = 2*shipradius;

    matGetVectFromMatrixCol3(shipfront, ship->rotinfo.coordsys);
    matGetVectFromMatrixCol2(shipright, ship->rotinfo.coordsys);

    vecSub(dirvect, ship->moveTo, ship->collInfo.collPosition);
    distance = (real32) fsqrt(vecMagnitudeSquared(dirvect));

    //find the moveto starting point
    vecScalarMultiply(dirfrontproj, shipfront, vecDotProduct(dirvect, shipfront));
    vecScalarMultiply(dirrightproj, shipright, vecDotProduct(dirvect, shipright));
    vecAdd(dirproj, dirfrontproj, dirrightproj);
    vecNormalize(&dirproj);

    //calculate the edge of the MoveToEndCircle
    //find the negative of the direction
    vecSub(negdir, zero, dirvect);
    negdir.z = 0;
    vecNormalize(&negdir);
    vecMultiplyByScalar(negdir, TO_MOVETO_ENDCIRCLE_RADIUS);
    vecAddTo(negdir, ship->moveTo);

    vecMultiplyByScalar(dirproj, (shipradius)*TO_MOVE_LINE_RADIUS_STRETCH);
    vecAddTo(dirproj, ship->collInfo.collPosition);


    //if the ship is pretty close to the end of the move, don't draw the line
    if (distance > pulsesize)
    {
        pulse = toDrawPulsedLine(dirproj, negdir, pulsesize, linecolor, pulsecolor);
    }

    if (distance > ((shipradius*TO_MOVE_LINE_RADIUS_STRETCH) + TO_MOVETO_ENDCIRCLE_RADIUS))
    {
        toDrawMoveToEndCircle(ship, pulse, linecolor, pulsecolor);
    }


}


/*-----------------------------------------------------------------------------
    Name        : toDrawMoveFromLine
    Description : Draws a single move line from a ship's movefrom point to the ship
    Inputs      : ship - the aformentioned ship
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toDrawMoveFromLine(ShipPtr ship)
{
    vector shipback, shippos, dirvect;
    real32 distance,shipradius;

    shipradius = TO_MOVE_LINE_RADIUS_STRETCH*ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    //calculate the line
    matGetVectFromMatrixCol3(shipback, ship->rotinfo.coordsys);
    vecMultiplyByScalar(shipback, (shipradius));
    shippos = ship->collInfo.collPosition;
    vecSubFrom(shippos, shipback);
    shipback = shippos;

    vecSub(dirvect, shipback, ship->moveFrom);
    distance = (real32) fsqrt(vecMagnitudeSquared(dirvect));

    if (distance < shipradius)
    {
        return;
    }

    glBegin(GL_LINES);
    {
        //draw the line from the ship to the pulse
        glColor3ub(colRed(TO_MOVE_LINE_COLOR), colGreen(TO_MOVE_LINE_COLOR), colBlue(TO_MOVE_LINE_COLOR));
        glVertex3fv((GLfloat *)&shipback);
        glVertex3fv((GLfloat *)&ship->moveFrom);
    }
    glEnd();


}



/*-----------------------------------------------------------------------------
    Name        : toDrawMoveCircle
    Description : Draws a circle around the moving ship
    Inputs      : ship - the ship
                  scale - the tactical overlay color scale factor to fade in the move
                          circle (not implemented)
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toDrawMoveCircle(ShipPtr ship, real32 scale, color linecolor, color pulsecolor)
{
    real32  shipradius;
    hmatrix rotmat;
    vector origin = {0.0, 0.0, 0.0};

    hmatMakeHMatFromMat(&rotmat, &ship->rotinfo.coordsys);
    hmatPutVectIntoHMatrixCol4(ship->collInfo.collPosition, rotmat);

    glPushMatrix();
    glMultMatrixf((GLfloat *)&rotmat);

    shipradius = ship->magnitudeSquared*TO_MOVE_LINE_RADIUS_STRETCH*ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    if ((ship->formationcommand)&&(ship->formationcommand->formation.pulse))
    {
        primCircleOutline3(&origin, shipradius, 32, 0, pulsecolor, X_AXIS);
    }
    else
    {
        primCircleOutline3(&origin, shipradius, 32, 0, linecolor, X_AXIS);
    }

    glPopMatrix();

}




/*-----------------------------------------------------------------------------
    Name        : toMoveLineDraw
    Description : Draws a line with a cool pulse from the ship's current position to
                  it's moveTo position.  Also draws a circle around the moving ship
    Inputs      : ship - the ship
                  scale - the tactical overlay color scale factor to fade in the move
                          circle (not used at the moment)
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toMoveLineDraw(ShipPtr ship, real32 scale)
{
    vector temp1;
    real32 temp2;
    bool   primEnabled;
    color  linecolor;
    color  pulsecolor;

#ifndef fpoiker
    if ((ship->staticinfo->shipclass != CLASS_Corvette) &&
        (ship->staticinfo->shipclass != CLASS_Fighter))
    {
#endif
        //do da graphics setting up stuff
        primEnabled = primModeEnabled;
        if (primEnabled)
        {
            primModeClear2();
        }
        rndLightingEnable(FALSE);
        rndTextureEnable(FALSE);

        //setup color: attack moves different from normal moves
        if ((ship->command != NULL) && (ship->command->ordertype.order == COMMAND_ATTACK))
        {
            linecolor  = TO_ATT_MOVE_LINE_COLOR;
            pulsecolor = TO_ATT_MOVE_LINE_PULSE_COLOR;
        }
        else
        {
            linecolor  = TO_MOVE_LINE_COLOR;
            pulsecolor = TO_MOVE_LINE_PULSE_COLOR;
        }

        //if a ship is in a formation but not the leader
        if ((ship->formationcommand)&&(ship!=ship->formationcommand->selection->ShipPtr[0]))
        {
            //only draw the moveto circle
            toDrawMoveCircle(ship, scale, linecolor, pulsecolor);
        }
        else if(ship->shiptype == ResearchShip)
        {
            toFakeOneShip(ship,&temp1, &temp2);

            //den draw da linez
            toDrawMoveToLine(ship, linecolor, pulsecolor);
            //toDrawMoveFromLine(ship);
            toDrawMoveCircle(ship, scale, linecolor, pulsecolor);

            toUnFakeOneShip(ship,&temp1, &temp2);

        }
        else
        {
            //den draw da linez
            toDrawMoveToLine(ship, linecolor, pulsecolor);

            if (ship->formationcommand)
            {
                ship->formationcommand->formation.pulse = pulse_at_beginning;
            }

            //toDrawMoveFromLine(ship);
            toDrawMoveCircle(ship, scale, linecolor, pulsecolor);
        }

        //den do sum more graphics non-setting up stuff
        if (primEnabled)
        {
            primModeSet2();
        }
#ifndef fpoiker
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : toMoveLineDraw
    Description : Draws a line with a cool pulse from the ship's current position to
                  it's moveTo position.  Also draws a circle around the moving ship
    Inputs      : ship - the ship
                  scale - the tactical overlay color scale factor to fade in the move
                          circle (not used at the moment)
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toFieldSphereDraw(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{
    hmatrix rotmat;
    vector origin = {0.0, 0.0, 0.0};
    vector up =    {1.0, 0.0, 0.0};
    vector right = {0.0, 1.0, 0.0};
    vector head =  {0.0, 0.0, 1.0};
    matrix tmpmat;

    //Turn off 2D primmode
    primModeClear2();
    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    //Draw the circles
    //    hmatMakeHMatFromMat(&rotmat, &ship->rotinfo.coordsys);
        //hmatPutVectIntoHMatrixCol4(ship->collInfo.collPosition, rotmat);
    //     hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


    matPutVectIntoMatrixCol1(up,tmpmat);
    matPutVectIntoMatrixCol2(right,tmpmat);
    matPutVectIntoMatrixCol3(head,tmpmat);

    hmatMakeHMatFromMat(&rotmat, &tmpmat);
    hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


    glPushMatrix();
        glMultMatrixf((GLfloat *)&rotmat);

        primCircleOutline3(&origin, radius, 32, 4, passedColour, Z_AXIS);
        primCircleOutline3(&origin, radius, 32, 4, passedColour, X_AXIS);
        primCircleOutline3(&origin, radius, 32, 4, passedColour, Y_AXIS);

        glPopMatrix();
    //stop drawing circles


    //Turn on 2D primmode
    primModeSet2();
}

void toFieldSphereDrawGeneral(vector position, real32 radius,color passedColour)
{
    hmatrix rotmat;
    vector origin = {0.0, 0.0, 0.0};
    vector up =    {1.0, 0.0, 0.0};
    vector right = {0.0, 1.0, 0.0};
    vector head =  {0.0, 0.0, 1.0};
    matrix tmpmat;

    //Turn off 2D primmode
    primModeClear2();
    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    //Draw the circles
    //    hmatMakeHMatFromMat(&rotmat, &ship->rotinfo.coordsys);
        //hmatPutVectIntoHMatrixCol4(ship->collInfo.collPosition, rotmat);
    //     hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


    matPutVectIntoMatrixCol1(up,tmpmat);
    matPutVectIntoMatrixCol2(right,tmpmat);
    matPutVectIntoMatrixCol3(head,tmpmat);

    hmatMakeHMatFromMat(&rotmat, &tmpmat);
    hmatPutVectIntoHMatrixCol4(position, rotmat);


    glPushMatrix();
        glMultMatrixf((GLfloat *)&rotmat);

        primCircleOutline3(&origin, radius, 32, 4, passedColour, Z_AXIS);
        primCircleOutline3(&origin, radius, 32, 4, passedColour, X_AXIS);
        primCircleOutline3(&origin, radius, 32, 4, passedColour, Y_AXIS);

        glPopMatrix();
    //stop drawing circles


    //Turn on 2D primmode
    primModeSet2();
}

real32 pulse1_timeelapsed=0.0f;
real32 pulse1_lasttime=0.0f;
real32 pulse1_timeelapsed_fast=0.0f;
real32 pulse1_lasttime_fast=0.0f;
real32 pulsedistance = 0.0f;
real32 pulsefastdistance = 0.0f;
real32 delay = 0.0f;

#define CROSS_HALF_HIGHT   TO_CROSS_HALF_SIZE


void toPulse1_positionupdate();
void toPulse1(vector linestart, vector lineend, real32 pulsesize, color pulsecolor);


//Draws a special radial display for the TO of special ships: Proximity Sensor
void toDrawRadialIndicator1(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{

   hmatrix rotmat;
   vector origin = {0.0, 0.0, 0.0};
   vector up =    {1.0, 0.0, 0.0};
   vector right = {0.0, 1.0, 0.0};
   vector head =  {0.0, 0.0, 1.0};

   matrix tmpmat;
   vector lineend,tmpvec1,tmpvec2,tmpvecup1,tmpvecup2,tmpvecright1,tmpvecright2,tmpvechead1,tmpvechead2;

   //Turn off 2D primmode
   primModeClear2();
   rndLightingEnable(FALSE);
   rndTextureEnable(FALSE);

   matPutVectIntoMatrixCol1(up,tmpmat);
   matPutVectIntoMatrixCol2(right,tmpmat);
   matPutVectIntoMatrixCol3(head,tmpmat);

   hmatMakeHMatFromMat(&rotmat, &tmpmat);
   //hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


   glPushMatrix();
   glMultMatrixf((GLfloat *)&rotmat);

   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Z_AXIS);
   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, X_AXIS);
   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Y_AXIS);

   vecScalarMultiply(tmpvecup1,up,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecup2,up,-CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecright1,right,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecright2,right,-CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvechead1,head,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvechead2,head,-CROSS_HALF_HIGHT);

   toPulse1_positionupdate();                //update time information
    if(delay > 0.0f)
    {
        delay -= pulse1_timeelapsed;
        if(delay <= 0.0f)
            delay = 0.0f;

    }
////
   vecScalarMultiply(lineend,right,radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecScalarMultiply(lineend,right,-radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

////
   vecScalarMultiply(lineend,up, radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecScalarMultiply(lineend,up, -radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

////
   vecScalarMultiply(lineend,head, radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecScalarMultiply(lineend,head, -radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   glPopMatrix();
   //stop drawing circles


   //Turn on 2D primmode
   primModeSet2();

}

#define TO_PULSE1_SPEED    TO_PULSE1_SPEED
#define TO_DELAY_PULSE1    TO_DELAY_PULSE1

void toPulse1_positionupdate()
{
   pulse1_timeelapsed = universe.totaltimeelapsed - pulse1_lasttime;
   pulse1_lasttime = universe.totaltimeelapsed;
   if(delay == 0.0f)
   {
       pulsedistance += pulse1_timeelapsed*TO_PULSE1_SPEED;
   }

}

/*-----------------------------------------------------------------------------
    Name        : toPulse1
    Description : Draws a pulse from linestart to lineend
    Inputs      : linestart  - where the line starts
                  lineend    - where the line ends
                  pulsesize  - how big that cool ass pulse is
                  linecolor  - the color of the line
                  pulsecolor - the color of the cool ass pulse
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toPulse1(vector linestart, vector lineend, real32 pulsesize, color pulsecolor)
{
    real32 distance, pulseenddistance;
    vector dirvect, pulsestart, pulseend;

    if(delay > 0.0f)
    {
       return;
    }

    vecSub(dirvect, lineend, linestart);
    distance = fsqrt(vecMagnitudeSquared(dirvect));
    vecNormalize(&dirvect);


    pulseenddistance = pulsedistance+pulsesize;
   if((pulseenddistance) > distance)
   {
     //pulse has overshot end so
     pulsedistance = 0.0f;
     pulseenddistance = pulsesize;
     delay = TO_DELAY_PULSE1;
     //pulse1_timeelapsed = 0.0f;
     //pulse1_lasttime = 0.0f;
   }
   vecScalarMultiply(pulsestart, dirvect, pulsedistance);

    vecScalarMultiply(pulseend, dirvect, pulseenddistance);
    vecAddTo(pulsestart,linestart);
    vecAddTo(pulseend,linestart);

    glBegin(GL_LINES);
    {
        glColor3ub(colRed(pulsecolor), colGreen(pulsecolor), colBlue(pulsecolor));
        glVertex3fv((GLfloat *)&pulsestart);
        glVertex3fv((GLfloat *)&pulseend);

    }
    glEnd();

}

/*-----------------------------------------------------------------------------
Radial Indicator # 2...same as above but different speeds
-----------------------------------------------------------------------------*/

real32 pulse2_timeelapsed=0.0f;
real32 pulse2_lasttime=0.0f;
real32 pulse2_timeelapsed_fast=0.0f;
real32 pulse2_lasttime_fast=0.0f;
real32 pulsedistance2 = 0.0f;
real32 pulsefastdistance2 = 0.0f;
real32 delay2 = 0.0f;


void toPulse2_positionupdate();
void toPulse2(vector linestart, vector lineend, real32 pulsesize, color pulsecolor);


//Draws a special radial display for the TO of special ships: Proximity Sensor
void toDrawRadialIndicator2(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{

   hmatrix rotmat;
   vector origin = {0.0, 0.0, 0.0};
   vector up =    {1.0, 0.0, 0.0};
   vector right = {0.0, 1.0, 0.0};
   vector head =  {0.0, 0.0, 1.0};

   matrix tmpmat;
   vector lineend,tmpvec1,tmpvec2,tmpvecup1,tmpvecup2,tmpvecright1,tmpvecright2,tmpvechead1,tmpvechead2;

   //Turn off 2D primmode
   primModeClear2();
   rndLightingEnable(FALSE);
   rndTextureEnable(FALSE);

   matPutVectIntoMatrixCol1(up,tmpmat);
   matPutVectIntoMatrixCol2(right,tmpmat);
   matPutVectIntoMatrixCol3(head,tmpmat);

   hmatMakeHMatFromMat(&rotmat, &tmpmat);
   //hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


   glPushMatrix();
   glMultMatrixf((GLfloat *)&rotmat);

   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Z_AXIS);
   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, X_AXIS);
   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Y_AXIS);

   vecScalarMultiply(tmpvecup1,up,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecup2,up,-CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecright1,right,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecright2,right,-CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvechead1,head,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvechead2,head,-CROSS_HALF_HIGHT);

   toPulse2_positionupdate();                //update time information
    if(delay2 > 0.0f)
    {
        delay2 -= pulse2_timeelapsed;
        if(delay2 <= 0.0f)
            delay2 = 0.0f;

    }
////
   vecScalarMultiply(lineend,right,radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecScalarMultiply(lineend,right,-radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

////
   vecScalarMultiply(lineend,up, radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecScalarMultiply(lineend,up, -radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

////
   vecScalarMultiply(lineend,head, radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecScalarMultiply(lineend,head, -radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   glPopMatrix();
   //stop drawing circles


   //Turn on 2D primmode
   primModeSet2();

}

#define TO_PULSE2_SPEED    TO_PULSE2_SPEED
#define TO_DELAY_PULSE2    TO_DELAY_PULSE2

void toPulse2_positionupdate()
{
   pulse2_timeelapsed = universe.totaltimeelapsed - pulse2_lasttime;
   pulse2_lasttime = universe.totaltimeelapsed;
   if(delay2 == 0.0f)
   {
      pulsedistance2 += pulse2_timeelapsed*TO_PULSE2_SPEED;
   }
}

/*-----------------------------------------------------------------------------
    Name        : toPulse2
      -same as above but faster...
      ..need because of different time variables
----------------------------------------------------------------------------*/
void toPulse2(vector linestart, vector lineend, real32 pulsesize, color pulsecolor)
{
    real32 distance, pulseenddistance;
    vector dirvect, pulsestart, pulseend;

    if(delay2 > 0.0f)
    {
       return;
    }

    vecSub(dirvect, lineend, linestart);
    distance = fsqrt(vecMagnitudeSquared(dirvect));
    vecNormalize(&dirvect);


    pulseenddistance = pulsedistance2+pulsesize;
   if((pulseenddistance) > distance)
   {
     //pulse has overshot end so
     pulsedistance2 = 0.0f;
     pulseenddistance = pulsesize;
     delay2 = TO_DELAY_PULSE2;
     //pulse1_timeelapsed = 0.0f;
     //pulse1_lasttime = 0.0f;
   }
   vecScalarMultiply(pulsestart, dirvect, pulsedistance2);

    vecScalarMultiply(pulseend, dirvect, pulseenddistance);
    vecAddTo(pulsestart,linestart);
    vecAddTo(pulseend,linestart);

    glBegin(GL_LINES);
    {
        glColor3ub(colRed(pulsecolor), colGreen(pulsecolor), colBlue(pulsecolor));
        glVertex3fv((GLfloat *)&pulsestart);
        glVertex3fv((GLfloat *)&pulseend);

    }
    glEnd();

}

//Radial Indicator 3  Circles pulsing to Outline a Sphere

#define TO_CIRCLE3_SPEED    4500.0f
#define TO_CIRCLE3_DELAY    3.0f
#define TO_CIRCLE3_DELAY_SPACEING 0.05f
#define TO_NUM_CIRCLES_3      1

real32 circle3_timeelapsed[TO_NUM_CIRCLES_3];
real32 circle3_lasttime[TO_NUM_CIRCLES_3];
real32 circle3_timeelapsed_fast[TO_NUM_CIRCLES_3];
real32 circle3_lasttime_fast[TO_NUM_CIRCLES_3];
real32 circle3distance[TO_NUM_CIRCLES_3];
real32 circle3fastdistance[TO_NUM_CIRCLES_3];
real32 circle3delay[TO_NUM_CIRCLES_3];

//Draws a special radial display for the TO of special ships: Proximity Sensor
void toDrawRadialIndicator3(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{

   hmatrix rotmat;
   real32 newrad;
   vector origin = {0.0, 0.0, 0.0};
   vector up =    {1.0, 0.0, 0.0};
   vector right = {0.0, 1.0, 0.0};
   vector head =  {0.0, 0.0, 1.0};
   sdword count;
   matrix tmpmat;
   vector circlestart,circlestart2;


   count = 0;

   while(count < TO_NUM_CIRCLES_3)
   {
        vecScalarMultiply(circlestart,head,radius);

        circle3_timeelapsed[count] = universe.totaltimeelapsed - circle3_lasttime[count];
        circle3_lasttime[count] = universe.totaltimeelapsed;
        if(circle3delay[count] <= 0.0f)
        {
            circle3delay[count] = 0.0f;
            circle3distance[count] += circle3_timeelapsed[count]*TO_CIRCLE3_SPEED;
        }
        else
        {
            circle3delay[count] -= circle3_timeelapsed[count];
            count++;
            continue;
        }
        if(circle3distance[count] > 2*radius)      //make faster :)    precalc diameter
        {
            circle3distance[count] = 0.0f;
            if(count == 0)
            {
                circle3delay[count] = TO_CIRCLE3_DELAY;
            }
            else
            {
                circle3delay[count] = TO_CIRCLE3_DELAY_SPACEING + circle3delay[count-1];
            }
            count++;
            continue;
        }

        vecScalarMultiply(circlestart2,head,-circle3distance[count]);
        vecAddTo(circlestart,circlestart2);
        vecAddTo(circlestart,ship->posinfo.position);
        if(circle3distance[count] > radius)
        {
            newrad = 2*radius - circle3distance[count];
        }
        else
        {
            newrad = circle3distance[count];
        }
       newrad = fsqrt(radius*radius - (radius - newrad)*(radius - newrad));

       //Turn off 2D primmode
       primModeClear2();
       rndLightingEnable(FALSE);
       rndTextureEnable(FALSE);

       matPutVectIntoMatrixCol1(up,tmpmat);
       matPutVectIntoMatrixCol2(right,tmpmat);
       matPutVectIntoMatrixCol3(head,tmpmat);

       hmatMakeHMatFromMat(&rotmat, &tmpmat);
       //hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


       glPushMatrix();
       glMultMatrixf((GLfloat *)&rotmat);

       primCircleOutline3(&circlestart, newrad, 32, 0, passedColour, Z_AXIS);
       //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, X_AXIS);
       //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Y_AXIS);

       glPopMatrix();
       //stop drawing circles


       //Turn on 2D primmode
       primModeSet2();
        count++;
   }

}
//Radial Indicator 4  Circles pulsing to Outline a Sphere

#define TO_CIRCLE4_SPEED    7500.0f
#define TO_CIRCLE4_DELAY    0.5f
#define TO_CIRCLE4_DELAY_SPACEING 0.05f


real32 circle4_timeelapsed[TO_NUM_CIRCLES_3];
real32 circle4_lasttime[TO_NUM_CIRCLES_3];
real32 circle4_timeelapsed_fast[TO_NUM_CIRCLES_3];
real32 circle4_lasttime_fast[TO_NUM_CIRCLES_3];
real32 circle4distance[TO_NUM_CIRCLES_3];
real32 circle4fastdistance[TO_NUM_CIRCLES_3];
real32 circle4delay[TO_NUM_CIRCLES_3];

//Draws a special radial display for the TO of special ships: Proximity Sensor
void toDrawRadialIndicator4(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{

   hmatrix rotmat;
   real32 newrad;
   vector origin = {0.0, 0.0, 0.0};
   vector up =    {1.0, 0.0, 0.0};
   vector right = {0.0, 1.0, 0.0};
   vector head =  {0.0, 0.0, 1.0};
   sdword count;
   matrix tmpmat;
   vector circlestart,circlestart2;

   count = 0;

   while(count < TO_NUM_CIRCLES_3)
   {
        vecScalarMultiply(circlestart,head,radius);

        circle4_timeelapsed[count] = universe.totaltimeelapsed - circle4_lasttime[count];
        circle4_lasttime[count] = universe.totaltimeelapsed;
        if(circle4delay[count] <= 0.0f)
        {
            circle4delay[count] = 0.0f;
            circle4distance[count] += circle4_timeelapsed[count]*TO_CIRCLE4_SPEED;
        }
        else
        {
            circle4delay[count] -= circle4_timeelapsed[count];
            count++;
            continue;
        }
        if(circle4distance[count] > 2*radius)      //make faster :)    precalc diameter
        {
            circle4distance[count] = 0.0f;
            if(count == 0)
            {
                circle4delay[count] = TO_CIRCLE4_DELAY;
            }
            else
            {
                circle4delay[count] = TO_CIRCLE4_DELAY_SPACEING + circle4delay[count-1];
            }
            count++;
            continue;
        }

        vecScalarMultiply(circlestart2,head,-circle4distance[count]);
        vecAddTo(circlestart,circlestart2);
        vecAddTo(circlestart,ship->posinfo.position);
        if(circle3distance[count] > radius)
        {
            newrad = 2*radius - circle4distance[count];
        }
        else
        {
            newrad = circle4distance[count];
        }
       newrad = fsqrt(radius*radius - (radius - newrad)*(radius - newrad));

       //Turn off 2D primmode
       primModeClear2();
       rndLightingEnable(FALSE);
       rndTextureEnable(FALSE);

       matPutVectIntoMatrixCol1(up,tmpmat);
       matPutVectIntoMatrixCol2(right,tmpmat);
       matPutVectIntoMatrixCol3(head,tmpmat);

       hmatMakeHMatFromMat(&rotmat, &tmpmat);
       //hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


       glPushMatrix();
       glMultMatrixf((GLfloat *)&rotmat);

       primCircleOutline3(&circlestart, newrad, 32, 0, passedColour, Z_AXIS);
       //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, X_AXIS);
       //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Y_AXIS);

       glPopMatrix();
       //stop drawing circles


       //Turn on 2D primmode
       primModeSet2();
        count++;
   }

}

/*-----------------------------------------------------------------------------
    Name        : mrRegionDraw
    Description : Region callback for drawing the main region.
    Inputs      : standard region callback crap
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword mrFrameCounter = 0;
void mrRegionDraw(regionhandle reg)
{
    udword shiftBits;
    static bool mrClearRenderEverything = FALSE;
    sdword index;

    if (singlePlayerGame)
    {
        mrFrameCounter++;
    }
    if (aivRenderMainScreen)
    {
        //doing this directly for efficiency
//        aivarValueSet(aivRenderMainScreen, mrRenderMainScreen);
        aivRenderMainScreen->value = mrRenderMainScreen;
    }

    if ((!mrRenderMainScreen) || nisBlackFade == 1.0f)
    {
        if (nisIsRunning && nisFullyScissored && rndScissorEnabled)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, NIS_LetterHeight, MAIN_WindowWidth, MAIN_WindowHeight - NIS_LetterHeight * 2);
        }
        if (mrClearRenderEverything)
        {
            mrClearRenderEverything = FALSE;
            feRenderEverything = FALSE;
        }
        if (feRenderEverything)
        {
            mrClearRenderEverything = TRUE;
        }
        if (nisTextCardIndex)
        {                                                   //render text cards on a black screen
            nisTextCardListDraw();
        }
        if (nisSMPTECounter != NULL)
        {
            nisSMPTECounterDraw(thisNisPlaying, nisSMPTECounter);
        }
        for (index = 0; index < NIS_NumberStatics; index++)
        {
            if (nisStatic[index] != NULL)
            {
                nisStaticDraw(nisStatic[index]);
            }
        }

        /*
        if (!rndScissorEnabled)
        {                                                   //if we can't scissor properly
            rndDrawScissorBars(rndScissorEnabled);
        }
        */
        return;
    }

    if (!glCapFastFeature(GL_BLEND))
    {
        if (mrWhiteOut)
        {
            rectangle rect = { 0,0,MAIN_WindowWidth,MAIN_WindowHeight };
            sdword c;
            real32 t;

            if (mrWhiteOutT < 0.5f)
            {
                t = mrWhiteOutT * 2.0f;
                c = (sdword)(t * 255.0f);
                primRectSolid2(&rect, colRGB((70*c)>>8,(70*c)>>8,(255*c)>>8));
            }
            else
            {
                t = 2.0f * (mrWhiteOutT - 0.5f);
                c = (sdword)(t * 155.0f);
                primRectSolid2(&rect, colRGB(70+c,70+c,255));
            }

            return;
        }
    }

    if (piePointSpecMode != PSM_Idle)
    {                                                       //draw point spec
        pieLineDrawCallback = pieAllShipsToPiePlateDraw;
        piePlaneDrawCallback = piePlaneDraw;
        pieMovementCursorDrawCallback = pieMovementCursorDraw;
        rndPostObjectCallback = piePointSpecDraw;
    }

    if (nisIsRunning && nisFullyScissored && rndScissorEnabled)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, NIS_LetterHeight, MAIN_WindowWidth, MAIN_WindowHeight - NIS_LetterHeight * 2);
    }
    if (nisCaptureCamera)
    {
        mrCamera = nisCamera;
    }
    else if (gameIsRunning)
    {
        mrCamera = &(universe.mainCameraCommand.actualcamera);
        ccControl(&universe.mainCameraCommand);
    }
    else
    {
        mrCamera = &defaultCamera;
    }
    rndMainViewRender(mrCamera);

    if (feStack[feStackIndex].screen == NULL)
        mouseCursorTextDraw();

    rndTextureEnable(FALSE);
    glDisable(GL_TEXTURE_2D);

    if (!thisNisPlaying)
    {
        mrTacticalOverlayState(utyCapsLockToggleState());   //make sure we have the right TO state
    }
    if (mrDrawTactical)
    {                                                       //need better key mechanism
        rndTextureEnable(FALSE);
        glDisable(GL_TEXTURE_2D);

        toAllShipsDraw();                                   //draw tactical overlays
        toLegendDraw();                                     //draw legend for overlays
    }

    selSelectedDraw();                                      //draw overlays for selected ships, if any
    if (mrHoldLeft == mrSelectHold)                         //and then draw the current selection progress
    {
        selSelectingDraw();
        primRectOutline2(&mrSelectionRect, 1, TW_SELECT_BOX_COLOR);
    }

#if SP_DEBUGKEYS
    kasDebugDraw();
#endif

    if (singlePlayerGame)
    {
        objectiveDrawStatus();
        kasfPopupTextDraw();
    }

    if (nisTextCardIndex)
    {
        nisTextCardListDraw();
    }

    if (nisSMPTECounter != NULL)
    {
        nisSMPTECounterDraw(thisNisPlaying, nisSMPTECounter);
    }
    for (index = 0; index < NIS_NumberStatics; index++)
    {
        if (nisStatic[index] != NULL)
        {
            nisStaticDraw(nisStatic[index]);
        }
    }

    //glDisable(GL_SCISSOR_TEST);

    //this should be reworked to use the actual formation CSM

    dbgAssert(mrFormationFont != FONT_InvalidFontHandle);
    if (mrDrawFormation && (!((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)))
    {
        fonthandle oldFont = fontMakeCurrent(mrFormationFont);
        fontPrint(0, 0, colWhite, TypeOfFormationToNiceStr(mrNewFormation));

        if (universe.totaltimeelapsed - mrDrawFormationTime >= MR_FormationDelay)
            mrDrawFormation = FALSE;
        fontMakeCurrent(oldFont);
    }

    if (printCaptainMessage)
    {
        fonthandle fhSave;
        udword y;

        y = mainWindowHeight/2 - ((fontHeight("W") + 4)*numPlayerDropped)/2 - fontHeight("W") - 6;

        fhSave = fontMakeCurrent(mrBigFont);

        fontPrintCentre(y,colWhite,strGetString(strCaptainTransfering));

        fontMakeCurrent(fhSave);
    }

    if ((multiPlayerGame) && (numPlayerDropped>0))
    {
        if (universe.totaltimeelapsed < (printTimeout + TIMEOUT_TIME))
        {
            udword y;
            char   temp[80];
            fonthandle fhSave;

            fhSave = fontMakeCurrent(mrBigFont);
            y = mainWindowHeight/2 - ((fontHeight("W") + 4)*numPlayerDropped)/2;

            for (index=0;index<numPlayerDropped;index++)
            {
                sprintf(temp, "%s %s", playerNames[playersDropped[index]], (playersReadyToGo[index] == PLAYER_QUIT) ? strGetString(strQuit) : strGetString(strDroppedOut));
                fontPrintCentre(y,colWhite,temp);
                y += fontHeight("W") + 4;
            }

            fontMakeCurrent(fhSave);
        }
        else
        {
            numPlayerDropped=0;
        }
    }

    if ((mrFormationName != NULL) && (!((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)))                            //if changing formations
    {
        //fontPrint(0, 0, colWhite, mrFormationName);
        if (universe.totaltimeelapsed - mrFormationTime >= MR_FormationDelay)
        {                                                   //if on this formation long enough
            char msgName[128];

            //mrFormationName = TypeOfFormationToStr(mrNewFormation);
            mrFormationTime = universe.totaltimeelapsed;

            makeShipsFormationCapable((SelectCommand *)&selSelected);
            if(selSelected.numShips >0)
            {
                // check again if mothership or carrier in - don't want them in delta, etc.
                if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) >= 0)
                {
                    mrNewFormation = PARADE_FORMATION_FLAG;
                }

                strcpy(msgName, "Game_Formation_");
                if (mrNewFormation == PARADE_FORMATION_FLAG)
                {
                    strcat(msgName, strGetString(strPARADE_FORMATION));
                }
                else
                {
                    strcat(msgName, TypeOfFormationToStr(mrNewFormation));
                }
                tutGameMessage(msgName);

                mrFormationName = NULL;                         //no longer selecting formations
                if (mrNewFormation == PARADE_FORMATION_FLAG)
                {
                    speechEvent(selSelected.ShipPtr[0], COMM_SetFormation, PICKET_FORMATION);
                    clWrapSetMilitaryParade(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
                }
                else
                {
                    speechEvent(selSelected.ShipPtr[0], COMM_SetFormation, mrNewFormation);
                    clWrapFormation(&universe.mainCommandLayer,     //set new formation
                        (SelectCommand *)&selSelected,mrNewFormation);
                }
            }
        }
    }

    if (gMessage[0].message[0] != (char)NULL)
    {
        mrCommandMessageDraw();
    }

    mrBigMessageDraw();

    //draw the bearing numbers on the horizon line
    if (smTickTextIndex > 0)
    {
        smTickTextDraw();
    }

    //compute animating stipple pattern every time mainrgn is drawn
    pieLineStippleCounter += pieLineStippleSpeed;
    shiftBits = ((udword)pieLineStippleCounter) % 16;
    pieStipplePattern = pieLineStipple | ((udword)pieLineStipple << 16);
    if ((sdword)shiftBits < 0)
    {
        pieStipplePattern >>= 16 - shiftBits;
    }
    else
    {
        pieStipplePattern >>= shiftBits;
    }

    //draw a big black rectangle to do a fade between the scissor bars
#define NIS_EXCESSSCISSORMARGIN     7       //!!! Keith fix this
    if (nisBlackFade != 0.0f)
    {                                                       //if there is some sort of NIS fade going down
        sdword y;
        if (nisScissorFade != 0.0f)
        {
            y = (sdword)((real32)NIS_LetterHeight * nisScissorFade);
        }
        else
        {
            y = NIS_LetterHeight;
        }
        glEnable(GL_BLEND);
        glColor4f(0.0f, 0.0f, 0.0f, nisBlackFade);
        glBegin(GL_QUADS);
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(y - NIS_EXCESSSCISSORMARGIN));
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight - y + NIS_EXCESSSCISSORMARGIN));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(MAIN_WindowHeight - y + NIS_EXCESSSCISSORMARGIN));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(y - NIS_EXCESSSCISSORMARGIN));
        glEnd();
        glDisable(GL_BLEND);
    }

    if (mrWhiteOut)
    {
        rectangle rect = { -1,0,MAIN_WindowWidth,MAIN_WindowHeight };
        sdword c;
        real32 t;

        if (mrWhiteOutT < 0.5f)
        {
            t = mrWhiteOutT * 2.0f;
            c = (sdword)(t * 255.0f);
            glEnable(GL_BLEND);
            rndAdditiveBlends(TRUE);
            primRectSolid2(&rect, colRGBA(70,70,255,c));
            rndAdditiveBlends(FALSE);
            glDisable(GL_BLEND);
        }
        else
        {
            t = 2.0f * (mrWhiteOutT - 0.5f);
            c = (sdword)((1.0f - t) * 255.0f);
            primRectSolid2(&rect, colRGB((70*c)>>8,(70*c)>>8,(255*c)>>8));
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrReset
    Description : Reset the state of the main region, such as turning off camera rotation.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrReset(void)
{
    mrHoldRight = mrHoldLeft = mrNULL;                      //in case player was band-boxing or rotating
    mouseCursorShow();
    piePointSpecMode = PSM_Idle;                             //in case player was moving
    if (RGLtype == SWtype)
    {
        rglFeature(RGL_SPEEDY);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrNISStarting
    Description : Called from NIS.c when an NIS, this function will clear out
                    any persistant control variables.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mrNISStarting(void)
{
    if (mrHoldLeft != mrNULL)
    {                                                       //if player is selecting
        mrSelectionRect.x0 = mrSelectionRect.x1 =           //selecting nothing
            mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
        selRectNone();
        mrHoldLeft = mrNULL;
    }
    if (mrHoldRight != mrNULL)
    {                                                       //if rotating mouse
        mrHoldRight = mrNULL;                               //stop it!
    }
    mrSaveTactical = mrDrawTactical;                        //turn off the tactical overlay
    mrDrawTactical = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : mrNISStopping
    Description : Like the above function, except it's called when an NIS ends.
                    It can optionally reset previously cleared variables.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mrNISStopping(void)
{
    if (mrHoldLeft != mrNULL)
    {                                                       //if player is selecting
        mrSelectionRect.x0 = mrSelectionRect.x1 =           //selecting nothing
            mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
        selRectNone();
        mrHoldLeft = mrNULL;
    }
    if (mrHoldRight != mrNULL)
    {                                                       //if rotating mouse
        mrHoldRight = mrNULL;                               //stop it!
    }
    mrDrawTactical = mrSaveTactical;                        //turn on the tactical overlay if it was on before
}

/*-----------------------------------------------------------------------------
    Name        : mrShipDied
    Description : Called when a ship died to remove any persistant references.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mrShipDied(Ship *ship)
{
    if (mrMothershipPtr == ship)
    {
        mrMothershipPtr = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrTacticalOverlayState
    Description : Sets the state of the tactical overlay.
    Inputs      : bTactical - TRUE for on, FALSE for off.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrTacticalOverlayState(bool bTactical)
{
    if (kbKeyBoundToCommand(kbTACTICAL_OVERLAY)==CAPSLOCKKEY)
    {
        mrDrawTactical = bTactical;
    }
}

#if MR_KEYBOARD_CHEATS
void mrAddResources(void)
{
    #define MR_RESOURCE_INCREASE 1000

    dbgMessage("\nAdding Resources");
    universe.curPlayerPtr->resourceUnits += MR_RESOURCE_INCREASE;
}

void mrAllTechnology(void)
{
    dbgMessage("\nAdding All Technologies");
    rmInitializeResearchStruct(universe.curPlayerPtr,TRUE,TECH_ALLTECHNOLOGY);
}

extern bool cmCheapShips;
void mrCheapShips(void)
{
    cmCheapShips = !cmCheapShips;
    if (cmCheapShips)
        dbgMessage("\nCheap Ships On");
    else
        dbgMessage("\nCheap Ships Off");

}

void mrToggleAI(void)
{
    mrNoAI = !mrNoAI;
    if (mrNoAI)
        dbgMessage("\nComputer AI On");
    else
        dbgMessage("\nComputer AI Off");
}


void mrAsteroids(void)
{

    dbgMessage("\nAsteroids");
    mrTradeStuffTest(NULL, NULL);
}

bool gMosaic = 0;
void mrBackground(void)
{
    gMosaic = !gMosaic;
}

void mrFilter(void)
{
    texLinearFiltering = !texLinearFiltering;
    trFilterEnable(texLinearFiltering);
}

#define MR_DEBUG_SIZE 16
#define MR_SINGLEPLAYER     0x0001      //allow in singleplayer
#define MR_NETGAME          0x0002      //allow in netgames

typedef void (*debugFunc)(void);    //callback function


typedef struct debugStructX
{
    char *string;
    debugFunc callback;
    udword mask;
}
debugStruct;

debugStruct mrDebugFunctions[] = {
    //for now, don't use QWIOGJN in the string - they're intercepted elsewhere

    //backwards string,        callback,             permission masks

    {"YMMUY",                   mrAddResources,      MR_SINGLEPLAYER},
    {"HCETLLA",                 mrAllTechnology,     MR_SINGLEPLAYER},
    {"PAEHC",                   mrCheapShips,        MR_SINGLEPLAYER},
    {"YMMUD",                   mrToggleAI,          MR_SINGLEPLAYER},
    {"RETSA",                   mrAsteroids,         0},
    {"YVURJ",                   mrBackground,        MR_SINGLEPLAYER | MR_NETGAME},
    {"RTLF",                    mrFilter,            MR_SINGLEPLAYER | MR_NETGAME},
    {"",                        NULL,                0},
};





char mrDebugKeyBuffer[MR_DEBUG_SIZE];
sdword mrDebugPointer;



bool mrPlayerHasCheated = FALSE;

void mrScanDebugCodes(sdword ID)
{
    sdword i,j,k;
    bool match;

    mrDebugPointer = (mrDebugPointer+1) & (MR_DEBUG_SIZE-1);
    mrDebugKeyBuffer[mrDebugPointer] = ID;

    for (i=0; mrDebugFunctions[i].callback!=NULL; i++)
    {
        match = TRUE;
        for (k=0,j = mrDebugPointer; mrDebugFunctions[i].string[k]>0; k++,j=(j+MR_DEBUG_SIZE-1) & (MR_DEBUG_SIZE-1))
        {
            if (mrDebugKeyBuffer[j] != mrDebugFunctions[i].string[k])
            {
                match = FALSE;
                break;
            }
        }
        if (match)
        {
            if (singlePlayerGame)
            {
                if (!(mrDebugFunctions[i].mask & MR_SINGLEPLAYER))
                {
                    return;
                }
            }

            if (multiPlayerGame)
            {
                if (!(mrDebugFunctions[i].mask & MR_NETGAME))
                {
                    return;
                }
            }

            mrPlayerHasCheated = TRUE;
            dbgMessage("\nDebug code entered");
            //add SFX
            (mrDebugFunctions[i].callback)();

        }

    }
}

#endif //MR_KEYBOARD_CHEATS
