/*=============================================================================
    Name    : Taskbar.c
    Purpose : Code and data to take care of the task bar

    Created 10/7/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "Types.h"
#include "Debug.h"
#include "Memory.h"
#include "Universe.h"
#include "Region.h"
#include "mainrgn.h"
#include "Select.h"
#include "FEFlow.h"
#include "utility.h"
#include "font.h"
#include "FontReg.h"
#include "Sensors.h"
#include "Globals.h"
#include "TaskBar.h"
#include "main.h"
#include "InfoOverlay.h"
#include "Objectives.h"
#include "Tutor.h"
#include "ObjTypes.h"
#include "LaunchMgr.h"
#include "SinglePlayer.h"
#include "Strings.h"
#include "FEColour.h"
#include "Subtitle.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif


/*=============================================================================
    Defs:
=============================================================================*/

#define TB_ACTIVE_COLOR       colRGB(0,180,0)
#define TB_DIM_COLOR          colRGB(0,80,0)

#define TB_MOTHERSHIP_DIM_COLOR     colRGB(128,0,0)

#define TB_RUMarginRight      4
#define TB_ShipsMarginRight   4
#define TB_RefreshInterval    0.5f

#define TB_OffScreenAmount    800


/*=============================================================================
    Data:
=============================================================================*/

//dynamic hyperspace button variables
#define TB_Both         0
#define TB_Objectives   1
#define TB_Hyperspace   2

static sdword tbHypObjState = TB_Both;
//list of task bar buttons
taskbutton tbButtons[TB_MaxButtons];
//sdword tbNumberButtons = 0;

//fibfileheader *tbFileHeader;
regionhandle tbBaseRegion = NULL;               //base region of task bar window
regionhandle tbButtonBaseRegion = NULL;         //base region for the buttons
regionhandle tbBumperRegion = NULL;             //bumper region which is invisible

//info to bump the taskbar up/down
sdword tbBumpUpSpeed = TB_BumpUpSpeed;          //speed to move up/down
sdword tbBumpDownSpeed = TB_BumpDownSpeed;
sdword tbBumpFullHeight = 1;                    //height of bumper region when fully extended
sdword tbRegionsAttached = FALSE;               //flags wether or not the task bar regions are connected to the bumper region
sdword tbBumpSmooth = FALSE;                    //smooth or non-smooth bump of task bar
sdword tbBumpDirection = 0;                     //0 = stationary, 1 = up, 2 = down
sdword tbBumpPosition = 0;                      //0 = all the way down

//info for font printing
sdword tbDotWidth = 1;
sdword tbTextMarginY = 1;
fonthandle tbButtonCaptionFont = 0;                 //font for use with buttons
fonthandle tbObjectiveFont = 0;

bool tbDisable = FALSE;

bool tbForceTaskbarVar = FALSE;

bool tbTaskBarActive = FALSE;
//bool mommyblink;

BabyCallBack    *tbRefreshBaby=NULL;

listwindowhandle tbListWindow = NULL;
regionhandle tbListWindowRegion = NULL;

void tbListWindowInit(char *name, featom *atom);
bool tbRefreshBabyFunction(udword num, void *data, struct BabyCallBack *baby);

void tbMothershipIndicator(featom *atom, regionhandle region);
void tbMovingIndicator(featom *atom, regionhandle region);
void tbAttackingIndicator(featom *atom, regionhandle region);
void tbDockingIndicator(featom *atom, regionhandle region);
void tbGuardingIndicator(featom *atom, regionhandle region);
void tbOtherIndicator(featom* atom, regionhandle region);
void tbRUs(featom *atom, regionhandle region);
void tbTactics(char *name, featom *atom);
void tbTacticsEvasive(char *name, featom *atom);
void tbTacticsAggressive(char *name, featom *atom);
void tbTacticsNeutral(char *name, featom *atom);
void tbShips(featom *atom, regionhandle region);


void tbCalcTotalShipCommands(void);

void tbTaskBarInit(void);
void tbTaskBarEnd(void);
void feToggleButtonSetFromScreen(char *name, sdword bPressed, fescreen *screen);

regionhandle tbFindRegion(char* name);

// Found in objectives.c
extern Objective **objectives;
extern sdword objectivesUsed;


enum
{
    TB_TACTIC_NONE = 0,
    TB_TACTIC_AGGR,
    TB_TACTIC_NEUT,
    TB_TACTIC_EVAS,
};

fescreen *tbScreen;

featom *tbAtomA = NULL;
featom *tbAtomN = NULL;
featom *tbAtomE = NULL;

uword tbShipsMoving = 0;
uword tbShipsAttacking = 0;
uword tbShipsGuarding = 0;
uword tbShipsDocking = 0;
uword tbShipsOther = 0;

uword tbShipsEvas = 0;
uword tbShipsNeut = 0;
uword tbShipsAggr = 0;

// used for the objectives list window
// max width of the complete/incomplete words for wrapping
udword tbStatusWidth=0;
// timer for the selection of objectives in the window
real32 tbTimeOutSelect;


//uword tbCommonTactic = TB_TACTIC_NONE;

void tbSetupHyperspace(void);


/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : tbBumperProcess
    Description : Region processor callback for handling the bumper region
    Inputs      : region - region to handle processing
                  ID - user-assigned ID set when region created
                  event - enumeration of event to be processed
                  data - additional event data (message specific)
    Outputs     : ...user defined...
    Return      : flags indicating further operation:
----------------------------------------------------------------------------*/
udword tbBumperProcess(regionhandle region, sdword ID, udword event, udword data)
{
    if (tbDisable)
    {
        if (tbRegionsAttached)
        {
            regRegionScroll(tbBumperRegion, 0, tbBumpFullHeight);
            regMoveLinkChild(tbBaseRegion, NULL);
            tbRegionsAttached = FALSE;

            tbTaskBarEnd();
        }
        return 0;
    }

    if (event == RPE_Enter)
    {                                                       //if just entering region
        tbSetupHyperspace();

        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bTaskbarOpen)
            return 0;

        if (piePointSpecMode != PSM_Idle)
            return 0;

        if ((mrHoldRight != mrNULL) && !tbForceTaskbarVar)
            return 0;

        if (tbBumpSmooth)
        {                                                   //if smooth-bump
            tbBumpDirection = 1;                            //start moving it up
            if (!tbRegionsAttached)
            {                                               //attach the task bar to bumper
                regMoveLinkChild(tbBaseRegion, tbBumperRegion);
                tbRegionsAttached = TRUE;

                tbCalcTotalShipCommands();
                tbRefreshBaby = taskCallBackRegister(tbRefreshBabyFunction, 0, NULL, (real32)TB_RefreshInterval);
                tbTaskBarActive = TRUE;
                //dbgMessage("\nTaskbar on ");

                tutGameMessage("Game_TaskbarOn");
            }
        }
        else
        {                                                   //else bump instantly
            if (!tbRegionsAttached)
            {
                regMoveLinkChild(tbBaseRegion, tbBumperRegion);
                regRegionScroll(tbBumperRegion, 0, -(tbBumpFullHeight));
                tbRegionsAttached = TRUE;

                tbTaskBarInit();

                tutGameMessage("Game_TaskbarOn");
            }
        }
    }
    else if (event == RPE_Exit)
    {
        if((tutorial==TUTORIAL_ONLY) &&
           (!tutEnable.bTaskbarClose) &&
           (!tbForceTaskbarVar))
            return 0;

        if (tbBumpSmooth)                                   //if just exiting region
        {                                                   //if smooth-bump
            tbBumpDirection = 2;                            //start moving down
        }
        else
        {                                                   //else bump instantly
            if (tbRegionsAttached)
            {
                regRegionScroll(tbBumperRegion, 0, tbBumpFullHeight);
                regMoveLinkChild(tbBaseRegion, NULL);
                tbRegionsAttached = FALSE;

                //regRegionDelete(tbBumperRegion);

                tbTaskBarEnd();
                tutGameMessage("Game_TaskbarOff");
            }
        }
    }
    tbForceTaskbarVar = FALSE;
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : tbForceTaskbar
    Description : Forces the taskbar on the screen by simulating a mouse enter event
    Inputs      : On - if TRUE, taskbar is forced on, if FALSE, taskbar is force off.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
// warning - I use tbBumperRegion just as a holder so that some region is
//           actually passed into the callback function.  tbBumperProcess
//           doesn't seem to do anything with the region parameter for now.
void tbForceTaskbar(bool On)
{
    tbForceTaskbarVar = TRUE;

    if (On)
    {
        tbBumperProcess(tbBumperRegion, 0, RPE_Enter, 0);
    }
    else
    {
        tbBumperProcess(tbBumperRegion, 0, RPE_Exit, 0);
    }
}


void tbSensorsHook(void)
{
    if (tbRegionsAttached)
    {
        regRegionScroll(tbBumperRegion, 0, tbBumpFullHeight);
        regMoveLinkChild(tbBaseRegion, NULL);
        tbRegionsAttached = FALSE;

        tbTaskBarEnd();
    }
}

void tbSensorsBegin(char* name, featom* atom)
{
    smSensorsBegin(name, atom);
}

//this function doesn't actually draw anything, because each button is
//drawn independently.  However, it does move the task bar up/down if needed.
//!!! the motion will therefore be frame-rate dependent.!!!
void tbButtonRegionDraw(featom *atom, regionhandle region)
{
    sdword movement;
    if (tbBumpSmooth)
    {                                                       //only move smooth when it's enabled
        if (tbBumpDirection == 1)
        {                                                   //if bumping up
            movement = min(tbBumpUpSpeed, tbBumpFullHeight - TB_BumperHeight - tbBumpPosition);
            regRegionScroll(tbBumperRegion, 0, -movement);  //move up a bit
            tbBumpPosition += movement;
            if (tbBumpPosition >= tbBumpFullHeight - TB_BumperHeight)
            {                                               //if all the way up
                tbBumpDirection = 0;                        //no longer moving
            }
        }
        else if (tbBumpDirection == 2)
        {                                                   //if bumping down
            movement = min(tbBumpDownSpeed, tbBumpPosition);
            regRegionScroll(tbBumperRegion, 0, movement);   //move down a bit
            tbBumpPosition -= movement;
            if (tbBumpPosition <= 0)
            {                                               //if reaches the bottom
                tbBumpDirection = 0;                        //stop moving
                regMoveLinkChild(tbBaseRegion, NULL);       //detach the regions
                tbRegionsAttached = FALSE;
            }
        }
    }
}
void tbFleetManager(char *name, featom *atom)
{
    feScreenStart(ghMainRegion, "Fleet_manager");
}
void tbSensorsManager(char *name, featom *atom)
{
    bitClear(((regionhandle)atom->region)->status,RSF_CurrentSelected);
//    feScreenStart(ghMainRegion, "Sensors_manager");
    smSensorsBegin(NULL, NULL);
}


#if TB_SECRET_BUTTON
//exits the game, like right now
void tbExitImmediately(char *name, featom *atom)
{
    if (!multiPlayerGame)
    {
        dbgMessagef("\nQuit game, baby!");
        utyCloseOK(NULL, 0, 0, 0);
    }
}
#endif


/*-----------------------------------------------------------------------------
    Name        : tbButtonDraw
    Description : Render callback for task bar buttons
    Inputs      : region - region we're rendering
    Outputs     : ...user defined...
    Return      : void
----------------------------------------------------------------------------*/
void tbButtonDraw(regionhandle region)
{
    color c, borderColor;
    fonthandle  fhSave;
    sdword width, movement;
    char string[TB_MaxString], *stringEnd, *stringEndStart;
    featom *atom = (featom *)tbButtonBaseRegion->userID;
    rectangle r;

    movement = 0;
    if (tbBumpSmooth)
    {                                                       //only move smooth when it's enabled
        if (tbBumpDirection == 1)
        {                                                   //if bumping up
            movement = min(tbBumpUpSpeed, tbBumpFullHeight - TB_BumperHeight - tbBumpPosition);
        }
        else if (tbBumpDirection == 2)
        {                                                   //if bumping down
            movement = -min(tbBumpDownSpeed, tbBumpPosition);
        }
    }
    r.y0 = region->rect.y0 + movement;
    r.y1 = region->rect.y1 + movement;
    r.x0 = region->rect.x0;
    r.x1 = region->rect.x1;
    if (bitTest(tbButtons[region->userID].flags, TBF_Pressed))
    {
        borderColor = colWhite;
    }
    else
    {
        borderColor = atom->borderColor;
    }
    if (bitTest(region->status, RSF_LeftPressed) && bitTest(region->status, RSF_MouseInside))
    {
        c = colWhite;
        width = 2;
    }
    else
    {
        c = atom->contentColor;
        width = 2;
    }
    primRectSolid2(&r, c);
    primRectOutline2(&r, width, borderColor);

    //tbButtons[region->userID];
    strcpy(string, tbButtons[region->userID].caption);      //copy the string to local copy

    dbgAssert(tbButtons[region->userID].caption);           //verify there is a string

    fhSave = fontCurrentGet();                              //save the current font
    fontMakeCurrent(tbButtonCaptionFont);                   //select the appropriate font
    width = r.x1 - r.x0 - TB_ButtonTextMargin * 2 - tbDotWidth;
    stringEnd = stringEndStart = string + strlen(string);
    while (fontWidth(string) > width)
    {
        *stringEnd = 0;
        stringEnd--;
        dbgAssert(stringEnd > string);
    }
    if (stringEnd != stringEndStart)
    {                                                       //if truncation occurred
        strcat(string, "...");                              //tack on the truncation character
    }
    if (r.y0 + tbTextMarginY + fontHeight("...") < MAIN_WindowHeight)
    {
        fontPrint(r.x0 + TB_ButtonTextMargin,
                  r.y0 + tbTextMarginY,                     //draw string
                  atom->borderColor, string);
    }

    fontMakeCurrent(fhSave);
}

/*-----------------------------------------------------------------------------
    Name        : tbButtonProcess
    Description : Region processor callback for task bar buttons
    Inputs      : region - region to handle processing
                  ID - user-assigned ID set when region created
                  event - enumeration of event to be processed
                  data - additional event data (message specific)
    Outputs     : ...user defined...
    Return      : flags indicating further operation:

----------------------------------------------------------------------------*/
udword tbButtonProcess(regionhandle region, sdword ID, udword event, udword data)
{
    sdword index;
    switch (event)
    {                                                       //if button pressed
        case RPE_ReleaseLeft:
            for (index = 0; index < TB_MaxButtons; index++)
            {
                bitClear(tbButtons[index].flags, TBF_Pressed);//no buttons pressed
            }
            bitClear(tbButtons[ID].flags, TBF_HeldDown);    //button not depressed
            bitSet(tbButtons[ID].flags, TBF_Pressed);       //this button pressed
            tbButtons[ID].function(&tbButtons[ID], tbButtons[ID].userData);
            break;
        case RPE_ExitHoldLeft:
            bitClear(tbButtons[ID].flags, TBF_HeldDown);    //button not depressed
            break;
        case RPE_EnterHoldLeft:
            bitSet(tbButtons[ID].flags, TBF_HeldDown);      //button not depressed
            break;
    }
    return(0);
}

#if TB_TEST
void tbTestFunc(struct taskbutton *button, ubyte *userData)
{
    dbgMessagef("\nTest button %x", userData);
}
#endif
/*-----------------------------------------------------------------------------
    Name        : tbStartup
    Description : Startup the taskbar module.
    Inputs      : void
    Outputs     : loads in taskbar .FIB file, clears the button list etc.
    Return      : void
    Note        : mainrgn.c, feflow.c and region.c must be started at this point.
----------------------------------------------------------------------------*/
void tbStartup(void)
{
    fescreen *screen;
    sdword index;
    char   temp[100];
    fonthandle fhSave;
    regionhandle reg;

//    regionhandle reg;
//    fonthandle  fhSave;

//    tbFileHeader = feScreensLoad(TB_FileName);

    tbHypObjState = TB_Both;
    screen = feScreenFind(TB_ScreenName);
    tbScreen = screen;
    dbgAssert(screen->nAtoms >= 2);
    tbBumpFullHeight = screen->atoms[0].height;
    tbBumperRegion = regChildAlloc(ghMainRegion, 0, 0,      //create the 'bumper' region
            0, MAIN_WindowWidth, tbBumpFullHeight + TB_BumperHeight,
            0, RPE_Enter | RPE_Exit);

    //... specify region handler

    regFunctionSet(tbBumperRegion, tbBumperProcess);
    feCallbackAdd(TB_FleetManager,  tbFleetManager);
    feCallbackAdd(TB_SensorsManager,tbSensorsManager);
//    feCallbackAdd(TB_InfoOverlay,   tbInfoOverlay);
    feCallbackAdd(TB_ObjectivesListWindow, tbListWindowInit);

    //feCallbackAdd("TB_Tactics", tbTactics);
    feCallbackAdd("TB_Tactics_E", tbTacticsEvasive);
    feCallbackAdd("TB_Tactics_A", tbTacticsAggressive);
    feCallbackAdd("TB_Tactics_N", tbTacticsNeutral);

    feDrawCallbackAdd("TB_Moving", tbMovingIndicator);
    feDrawCallbackAdd("TB_Guarding", tbGuardingIndicator);
    feDrawCallbackAdd("TB_Attacking", tbAttackingIndicator);
    feDrawCallbackAdd("TB_Docking", tbDockingIndicator);
    feDrawCallbackAdd("TB_Other", tbOtherIndicator);
    feDrawCallbackAdd("TB_Mothership", tbMothershipIndicator);
    feDrawCallbackAdd("TB_RUs", tbRUs);
    feDrawCallbackAdd("TB_Ships", tbShips);

//    feDrawCallbackAdd(TB_MissionButtons, tbButtonRegionDraw);
#if TB_SECRET_BUTTON
    feCallbackAdd("TB_ExitImmediately", tbExitImmediately);
#endif
    tbBaseRegion = feRegionsAdd(tbBumperRegion, screen, FALSE);    //add the task bar to the button region
    tbRegionsAttached = TRUE;
                                                            //move taskbar down off bottom of screen
    regRegionScroll(tbBumperRegion, 0, MAIN_WindowHeight - TB_BumperHeight);
    regMoveLinkChild(tbBaseRegion, NULL);
    tbRegionsAttached = FALSE;

    //now scan through the screen's atom list to find the user region for drawing buttons
#if 0
    reg = tbBaseRegion->child;
    for (index = 1; index < screen->nAtoms; index++)
    {
        regVerify(reg);
        if (((featom *)reg->userID)->type == FA_UserRegion) //if this is a user region
        {
            tbButtonBaseRegion = reg;                       //this must be the region
            break;
        }
        reg = reg->next;
    }
#else
    for (index = 1; index < screen->nAtoms; index++)
    {
        if (screen->atoms[index].type == FA_UserRegion)
        {
            tbButtonBaseRegion = (regionhandle)screen->atoms[index].region;
            break;
        }
    }

#endif

    //now re-reposition to bottom of screen

    regRegionScroll(tbBaseRegion, 0, -((MAIN_WindowHeight - 480) / 2));
    //tbBaseRegion->rect.y0 -= ((MAIN_WindowHeight - 480) / 2);

    //load a font if needed
    tbButtonCaptionFont = frFontRegister(TB_FontFile);
    tbObjectiveFont = frFontRegister(TB_ObjectiveFontFile);


    // Calculate the max size of the objective status
    fhSave = fontMakeCurrent(tbObjectiveFont);

    sprintf(temp, "[%s] ", strGetString(strObjComplete));
    tbStatusWidth = fontWidth(temp);

    sprintf(temp, "[%s] ", strGetString(strObjIncomplete));
    if (fontWidth(temp) > tbStatusWidth)  tbStatusWidth = fontWidth(temp);

    fontMakeCurrent(fhSave);

#if 0
    fhSave = fontCurrentGet();                              //save the current font
    fontMakeCurrent(tbButtonCaptionFont);                   //select the appropriate font
    tbDotWidth = fontWidth("...");
    tbTextMarginY = (tbButtonBaseRegion->rect.y1 -
            tbButtonBaseRegion->rect.y0 - fontHeight("...")) / 2;
    fontMakeCurrent(fhSave);
    dbgAssert(tbButtonBaseRegion != NULL);
#endif
    feAllCallOnCreate(screen);
//    tbNumberButtons = 0;

#if TB_TEST
    tbButtonCreate("0Caption", tbTestFunc, (ubyte *)0x00, 0);
    tbButtonCreate("1Caption", tbTestFunc, (ubyte *)0x01, 0);
    tbButtonCreate("2Caption", tbTestFunc, (ubyte *)0x02, 0);
//  tbButtonCreate("3Caption", tbTestFunc, (ubyte *)0x03, 0);
//  tbButtonCreate("4Caption", tbTestFunc, (ubyte *)0x04, 0);
//  tbButtonCreate("5Caption", tbTestFunc, (ubyte *)0x05, 0);
//  tbButtonCreate("6Caption", tbTestFunc, (ubyte *)0x06, 0);
//  tbButtonCreate("7Caption", tbTestFunc, (ubyte *)0x07, 0);
//  tbButtonCreate("8Caption", tbTestFunc, (ubyte *)0x08, 0);
//  tbButtonCreate("9Caption", tbTestFunc, (ubyte *)0x09, 0);
//  tbButtonCreate("10Caption", tbTestFunc, (ubyte *)0x10, 0);
    tbButtonListRefresh();
#endif

    reg = tbFindRegion("TB_SensorsManager");
    bitSet(reg->status, RSF_CantFocusTo);
    reg = tbFindRegion("CSM_Build");
    bitSet(reg->status, RSF_CantFocusTo);
    reg = tbFindRegion("CSM_Research");
    bitSet(reg->status, RSF_CantFocusTo);
    reg = tbFindRegion("CSM_Launch");
    bitSet(reg->status, RSF_CantFocusTo);
    reg = tbFindRegion("CSM_Hyperspace");
    bitSet(reg->status, RSF_CantFocusTo);
}

/*-----------------------------------------------------------------------------
    Name        : tbShutdown
    Description : Shuts down the task bar module.
    Inputs      : void
    Outputs     : Frees the task bar screen etc...
    Return      : void
----------------------------------------------------------------------------*/
void tbShutdown(void)
{
    sdword index;

    PossiblyResetTaskbar();

    // free all the buttons
    for (index = 0; index < TB_MaxButtons; index++)
    {
        if (bitTest(tbButtons[index].flags,TBF_InUse))
        {
            tbButtonDelete(&tbButtons[index]);
        }
    }
//  while (tbNumberButtons > 0)
//  {
//      tbButtonDelete(&tbButtons[tbNumberButtons - 1]);    //free the button up
//  }
    //... free the regions if they are detached
    if (!tbRegionsAttached)                                 //if regions not attached
    {
        regRegionDelete(tbBaseRegion);                      //delete the base region and everything thereunder
    }
    regRegionDelete(tbBumperRegion);
//    feScreensDelete(tbFileHeader);                          //free the fe screen

    //uicListCleanUp(tbListWindow);
}

/*-----------------------------------------------------------------------------
    Name        : tbButtonDelete / tbButtonDeleteByData
    Description : Delete a button either by pointer or by user data reference
    Inputs      : button - used to directly free the button.
                  userData - used to search for the button to free.
    Outputs     : Frees the button and moves the rest of the list back one.
    Return      : void
----------------------------------------------------------------------------*/
void tbButtonDelete(taskbutton *button)
{
//    sdword index;

    dbgAssert(button - tbButtons >= 0 && button - tbButtons < TB_MaxButtons);
    dbgAssert(bitTest(tbButtons[button - tbButtons].flags, TBF_InUse));
//    dbgAssert(tbNumberButtons > 0);
    memFree(tbButtons[button - tbButtons].caption);
    regRegionDelete(tbButtons[button - tbButtons].reg);
    tbButtons[button - tbButtons].flags = 0;                //flag as no longer in use
//  for (index = button - tbButtons + 1; index < tbNumberButtons; index++)
//  {
//      tbButtons[index - 1] = tbButtons[index];            //move the whole list back one
//  }
//  tbNumberButtons--;                                      //one less button
}

void tbButtonDeleteByData(ubyte *userData)
{
    sdword index;

    for (index = 0; index < TB_MaxButtons; index++)
    {
        if (tbButtons[index].userData == userData)          //if this is the button
        {
            tbButtonDelete(&tbButtons[index]);
            return;
        }
    }
#if TB_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "tbButtonDeleteByData: couldn't find button with userData 0x%x in list of %d", userData, TB_MaxButtons);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : tbButtonSelect/tbButtonSelectByData
    Description : Select a button
    Inputs      : button - pointer to button to select
                  userData - data of button to search for
    Outputs     : Just sets the pressed bit.  Does not call the buttons user function
    Return      :
----------------------------------------------------------------------------*/
void tbButtonSelect(taskbutton *button)
{
    sdword index;
    for (index = 0; index < TB_MaxButtons; index++)
    {
        bitClear(tbButtons[index].flags, TBF_Pressed);      //no buttons pressed
    }
    bitSet(button->flags, TBF_Pressed);                     //this button pressed
}

void tbButtonSelectByData(ubyte *userData)
{
    sdword index;

    for (index = 0; index < TB_MaxButtons; index++)
    {
        if (tbButtons[index].userData == userData)          //if this is the button
        {
            tbButtonSelect(&tbButtons[index]);
            return;
        }
    }
#if TB_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "tbButtonSelectByData: couldn't find button with userData 0x%x in list of %d", userData, TB_MaxButtons);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : tbButtonCreate
    Description : Create a new button on the task bar
    Inputs      : caption - string to draw on the button.
                  function - function to call when it is hit
                  userData - pointer to user data passed back in the callback function.
    Outputs     : Allocates a new button and does some setup, but all resizing
                    is left to tbButtonListRefresh.
    Return      :
----------------------------------------------------------------------------*/
taskbutton *tbButtonCreate(char *caption, tbfunction function, ubyte *userData, udword flags)
{
    sdword index;

    for (index = 0; index < TB_MaxButtons; index++)
    {
        if (!bitTest(tbButtons[index].flags,TBF_InUse))
        {                                                   //if free button
            tbButtons[index].reg =                        //allocate the region structure for the button
                regChildAlloc(tbButtonBaseRegion, index, 0,
                    tbButtonBaseRegion->rect.y0, 2, tbButtonBaseRegion->rect.y1 -
                    tbButtonBaseRegion->rect.y0, 0, RPE_LeftClickButton);
            regFunctionSet(tbButtons[index].reg, tbButtonProcess);
            regDrawFunctionSet(tbButtons[index].reg, tbButtonDraw);
            tbButtons[index].flags = flags | TBF_InUse;
            tbButtons[index].caption = memStringDupe(caption);
            tbButtons[index].userData = userData;
            tbButtons[index].function = function;
            return(&tbButtons[index]);
        }
    }
#if TB_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "Ran out of task bar buttons: %d", TB_MaxButtons);
#endif
    return(NULL);
//  tbButtons[tbNumberButtons].reg =                        //allocate the region structure for the button
//      regChildAlloc(tbButtonBaseRegion, tbNumberButtons, 0,
//          tbButtonBaseRegion->rect.y0, 2, tbButtonBaseRegion->rect.y1 -
//          tbButtonBaseRegion->rect.y0, 0, RPE_LeftClickButton);
//  regFunctionSet(tbButtons[tbNumberButtons].reg, tbButtonProcess);
//  regDrawFunctionSet(tbButtons[tbNumberButtons].reg, tbButtonDraw);
//  tbButtons[tbNumberButtons].flags = flags;
//  tbButtons[tbNumberButtons].caption = memStringDupe(caption);
//  tbButtons[tbNumberButtons].userData = userData;
//  tbButtons[tbNumberButtons].function = function;
//  tbNumberButtons++;
//  return(&tbButtons[tbNumberButtons - 1]);
}

/*-----------------------------------------------------------------------------
    Name        : tbButtonListRefresh
    Description : Resize all the buttons to fit in the TB_MissionButtons user region
    Inputs      : void
    Outputs     : adjusts x0/x1 of all buttons to fit
    Return      :
----------------------------------------------------------------------------*/
void tbButtonListRefresh(void)
{
    sdword index, x, width, count;

    for (index = count = 0; index < TB_MaxButtons; index++)         //for each button
    {
        if (bitTest(tbButtons[index].flags,TBF_InUse))
        {
            count++;
        }
    }
    if (count == 0)
    {
        return;
    }
    width = tbButtonBaseRegion->rect.x1 - tbButtonBaseRegion->rect.x0;//width of button area
    width = (width - TB_ButtonMarginLeft - TB_ButtonMarginRight +//minus margins
             TB_ButtonMarginTween) /                        //over the number of buttons
                count - TB_ButtonMarginTween;               //minus a margin per button
    width = min(width, TB_MaxButtonWidth);                  //max the width out
    x = tbButtonBaseRegion->rect.x0 + TB_ButtonMarginLeft;  //location of first button
    for (index = 0; index < TB_MaxButtons; index++)         //for each button
    {
        if (bitTest(tbButtons[index].flags,TBF_InUse))
        {
            tbButtons[index].reg->rect.x0 = x;
            tbButtons[index].reg->rect.x1 = x + width;      //set horiz extents
            x += width + TB_ButtonMarginTween;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : tbObjectiveItemDraw
    Description : Draw the fleet objective text to list window (one line)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tbObjectiveItemDraw(rectangle *rect, listitemhandle data)
{
    sdword     x, y;
    fonthandle oldfont;
    uiclistitem  *listitem;
    tasklistitem  *taskitem = (tasklistitem *)(data->data);
    Objective *objective = (Objective *)((tasklistitem *)(data->data))->objective, *obj;
    char text[TBL_MaxCharsPerLine];
    color c;
    Node *search;

    oldfont = fontMakeCurrent(tbObjectiveFont);

    x = rect->x0;//+MG_HorzSpacing;
    y = rect->y0-2;//+MG_VertSpacing/2;

    search = tbListWindow->listofitems.head;
    while (search != NULL)
    {
        listitem = (uiclistitem *)listGetStructOfNode(search);

        obj = (Objective *)((tasklistitem *)(listitem->data))->objective;
        if (obj != NULL)
        {
            if (obj->status)
                listitem->flags = 0;
        }

        search = search->next;
    }

    if (taskTimeElapsed > tbTimeOutSelect)
    {
        search = tbListWindow->listofitems.head;
        while (search != NULL)
        {
            listitem = (uiclistitem *)listGetStructOfNode(search);

            bitClear(listitem->flags, UICLI_Selected);

            search = search->next;
        }

        tbTimeOutSelect = REALlyBig;
    }

    switch (taskitem->type)
    {
        case TBL_PrimaryObj1st:
        case TBL_SecondaryObj1st:
            sprintf(text, "[%s]",
                    objective->status ? strGetString(strObjComplete) : strGetString(strObjIncomplete));

            if (bitTest(data->flags, UICLI_Selected))
                c = TB_SelectedColor;
            else
                c = (objective->status) ? TB_CompleteColor : TB_IncompleteColor;

            fontPrint(x, y, c, text);

            x += tbStatusWidth;
            sprintf(text, "%s", taskitem->descFrag);
            break;
        case TBL_PrimaryObj2nd:
        case TBL_SecondaryObj2nd:
            x += tbStatusWidth;
            sprintf(text, "%s", taskitem->descFrag);

            if (bitTest(data->flags, UICLI_Selected))
                c = TB_SelectedColor;
            else
                c = (objective->status) ? TB_CompleteColor : TB_IncompleteColor;
            break;
        case TBL_HeaderSecondary:
            c = TB_SelectedColor;
            sprintf(text, "%s", taskitem->descFrag);
            break;
        default:
            // invalid taskitem
            dbgMessage("This definetly shouldn't happen, call Drew");
            dbgAssert(FALSE);
            break;
    }

    fontPrint(x, y, c, text);

    fontMakeCurrent(oldfont);
}

void tbObjectivesHyperspace(ubyte *data)
{
    tbSetupHyperspace();
    tbObjectivesListCleanUp();
}

void tbObjectivesListAddItem(ubyte *data)
{
    Objective    *objective = (Objective *)data;
    tasklistitem *taskitem;
    sdword        numchopped, i;
    char          chopbuf[TBL_MaxCharsPerLine*4];
    char         *chopstrings[6];
    Node         *search;
    uiclistitem  *listitem;
    bool          secondfound=FALSE;
    rectangle     rect;

    dbgAssert(strlen(objective->description) <= TBL_MaxCharsPerLine*3);

    // check for a secondary objective
    search = tbListWindow->listofitems.head;
    while (search != NULL)
    {
        listitem = listGetStructOfNode(search);

        if (((tasklistitem *)listitem->data)->type == TBL_HeaderSecondary)
        {
            secondfound = TRUE;
        }

        search = search->next;
    }

    rect = tbListWindow->reg.rect;
    rect.x1 -= (tbStatusWidth + 12);

    if (objective->primary == FALSE)
    {
        if (secondfound == FALSE)
        {
            // add the secondary objectives header to the end of the list
            taskitem = (tasklistitem *)memAlloc(sizeof(tasklistitem), "TaskBarObjective", NonVolatile);

            taskitem->objective = NULL;
            taskitem->type = TBL_HeaderSecondary;
            strcpy(taskitem->descFrag,strGetString(strobjSecondary));

            uicListAddItem(tbListWindow, (ubyte *)taskitem, 0, UICLW_AddToTail);
        }

        // chop up the objective
        numchopped   =  subStringsChop(&rect,
                                       tbObjectiveFont,
                                       strlen(objective->description),
                                       objective->description,
                                       chopbuf,
                                       chopstrings);

        // add all the chopped strings
        for (i=0;i<numchopped;i++)
        {
            taskitem = (tasklistitem *)memAlloc(sizeof(tasklistitem), "TaskBarObjective", NonVolatile);

            taskitem->objective = (struct Objective *)objective;
            if (i == 0) taskitem->type = TBL_SecondaryObj1st;
            else        taskitem->type = TBL_SecondaryObj2nd;

            strcpy(taskitem->descFrag, chopstrings[i]);

            uicListAddItem(tbListWindow, (ubyte *)taskitem, UICLI_CanSelect, UICLW_AddToTail);
        }
    }
    else
    {
        // chop up the objective
        numchopped   =  subStringsChop(&rect,
                                       tbObjectiveFont,
                                       strlen(objective->description),
                                       objective->description,
                                       chopbuf,
                                       chopstrings);

        // add all the chopped strings
        for (i=numchopped-1;i>=0;i--)
        {
            taskitem = (tasklistitem *)memAlloc(sizeof(tasklistitem), "TaskBarObjective", NonVolatile);

            taskitem->objective = (struct Objective *)objective;
            if (i == 0) taskitem->type = TBL_PrimaryObj1st;
            else        taskitem->type = TBL_PrimaryObj2nd;

            strcpy(taskitem->descFrag, chopstrings[i]);

            uicListAddItem(tbListWindow, (ubyte *)taskitem, UICLI_CanSelect, UICLW_AddToHead);
        }
    }
}

void tbObjectivesListRemoveItem(ubyte *data)
{
    Objective    *objective = (Objective*)data;
    Node         *search;
    uiclistitem  *listitem[4];
    sdword        numitems=-1, i;
    tasklistitem *taskitem;

    search = tbListWindow->listofitems.head;
    while (search != NULL)
    {
        taskitem = (tasklistitem *)((uiclistitem *)listGetStructOfNode(search))->data;

        if ( ((Objective *)taskitem->objective) == objective)
            listitem[++numitems] = (uiclistitem *)listGetStructOfNode(search);

        search = search->next;
    }

    for (i=numitems;i>=0;i--)
    {
        uicListRemoveItem(tbListWindow, listitem[i]);
    }
}

void tbObjectivesListCleanUp(void)
{
    Node         *search;
    tasklistitem *taskitem;

    search = tbListWindow->listofitems.head;
    while (search != NULL)
    {
        taskitem = (tasklistitem *)((uiclistitem *)listGetStructOfNode(search))->data;

        memFree(taskitem);

        search = search->next;
    }

    uicListCleanUp(tbListWindow);
}

/*-----------------------------------------------------------------------------
    Name        : tbListWindowInit
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tbListWindowInit(char *name, featom *atom)
{
    fonthandle oldfont;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(tbObjectiveFont);

        tbListWindow = (listwindowhandle)atom->pData;
        tbListWindowRegion = atom->region;

        uicListWindowInit(tbListWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          tbObjectiveItemDraw,              // item draw function
                          fontHeight(" ") + (fontHeight(" ") >> 2) - 2, // item height
                          UICLW_CanSelect);

        fontMakeCurrent(oldfont);

        atom->status = 0;

        return;
    }
    else if (FELASTCALL(atom))
    {
        tbListWindowRegion = NULL;
//        tbListWindow = NULL;
        return;
    }
    else if (tbListWindow->message == CM_NewItemSelected)
    {
        tasklistitem *taskitem;
        Objective    *objective = (Objective*)(((tasklistitem *)tbListWindow->CurLineSelected->data)->objective);
        Node         *search;
        uiclistitem  *listitem;

        tbTimeOutSelect = taskTimeElapsed + 0.5;

        search = tbListWindow->listofitems.head;
        while (search != NULL)
        {
            listitem = (uiclistitem *)listGetStructOfNode(search);
            taskitem = (tasklistitem *)listitem->data;

            if ( ((Objective *)taskitem->objective) == objective)
                bitSet(listitem->flags, UICLI_Selected);
            else
                bitClear(listitem->flags, UICLI_Selected);

            search = search->next;
        }

        if (!objective->status)
        {
            poPopupFleetIntelligence(objective);
        }
    }
}

#define TB_GENERIC_INDICATOR(var) \
    color col; \
    rectangle* rect = &region->rect; \
    \
    if ((var) > 0) \
    { \
        col = TB_ACTIVE_COLOR; \
        primRectSolid2(rect, col); \
    } \
    else \
    { \
        col = TB_DIM_COLOR; \
        primRectOutline2(rect, 1, col); \
    } \

void tbMovingIndicator(featom *atom, regionhandle region)
{
    TB_GENERIC_INDICATOR(tbShipsMoving)
}

void tbGuardingIndicator(featom *atom, regionhandle region)
{
    TB_GENERIC_INDICATOR(tbShipsGuarding)
}

void tbDockingIndicator(featom *atom, regionhandle region)
{
    TB_GENERIC_INDICATOR(tbShipsDocking)
}

void tbAttackingIndicator(featom *atom, regionhandle region)
{
    TB_GENERIC_INDICATOR(tbShipsAttacking)
}

void tbOtherIndicator(featom* atom, regionhandle region)
{
    TB_GENERIC_INDICATOR(tbShipsOther)
}

void tbBarDraw(rectangle *rect, color back, color fore, real32 percent)
//percent is actually 0.0 to 1.0
{
    rectangle temp;
    primRectSolid2(rect, back);

    if (percent > 1.0f)
    {
        percent = 1.0f;
    }

    temp.x0 = rect->x0;
    temp.y0 = rect->y0;
    temp.x1 = rect->x0 + (sdword)((rect->x1-rect->x0)*percent);
    temp.y1 = rect->y1;

    primRectSolid2(&temp, fore);
}

void tbMothershipIndicator(featom *atom, regionhandle region)
{
    color col,backcol;
    uword r,g;
    real32 percent;
    rectangle *rect = &region->rect;
    Ship *mommy;

    mommy = universe.curPlayerPtr->PlayerMothership;

    if (!mommy)
    {
        percent = 0.0f;
    }
    else
    {
        percent = mommy->health / mommy->staticinfo->maxhealth;
    }

    backcol  = colRGB(30,30,30);

    if (percent > 0.50f)
    {
        g = 255;
        //r = (uword)((1 - percent)*(510));
        r = 0;
    }
    else
    {
        if (percent < 0.25f)
        {
            r = 255;
            g = 0;
        }
        else
        {
            //g = (uword)((percent-0.25f)*(1022));
            g = 255;
            r = 255;
        }
    }

    col = colRGB(r,g,0);

    //mommyblink = !mommyblink;
    //if ((percent < 0.25f) && mommyblink) col = TB_MOTHERSHIP_DIM_COLOR;

    tbBarDraw(rect, backcol, col, percent);
}

void tbCalcTotalShipCommands(void)
{
    sdword i;
    Ship *ship;
    CommandToDo *command;

    tbShipsEvas = 0;
    tbShipsNeut = 0;
    tbShipsAggr = 0;
    tbShipsMoving = 0;
    tbShipsAttacking = 0;
    tbShipsGuarding = 0;
    tbShipsDocking = 0;
    tbShipsOther = 0;

    for(i=0; i<selSelected.numShips; ++i)
    {
        ship = selSelected.ShipPtr[i];
        command = ship->command;

        if (command)
        {
            switch (command->ordertype.order)
            {
                case COMMAND_HALT:
                case COMMAND_MILITARYPARADE:
                case COMMAND_LAUNCHSHIP:
                case COMMAND_NULL:
                    if (bitTest(command->ordertype.attributes, COMMAND_IS_PROTECTING))
                    {
                        tbShipsGuarding++;
                    }
                    break;

                case COMMAND_DOCK:
                    tbShipsDocking++;
                    break;

                case COMMAND_MOVE:
                    tbShipsMoving++;
                    break;

                case COMMAND_ATTACK:
                    tbShipsAttacking++;
                    break;

                default:
                    tbShipsOther++;
                    break;
            }
        }
        //now tactics
        switch(ship->tacticstype)
        {
        case Evasive:
            tbShipsEvas++;
            break;
        case Neutral:
            tbShipsNeut++;
            break;
        case Aggressive:
            tbShipsAggr++;
            break;
        default:
            break;
        }

        /*
        if ((evas + neut == 0) && (aggr > 0))
            tbCommonTactic = TB_TACTIC_AGGR;
        else if ((evas + aggr == 0) && (neut > 0))
            tbCommonTactic = TB_TACTIC_NEUT;
        else if ((neut + aggr == 0) && (evas > 0))
            tbCommonTactic = TB_TACTIC_EVAS;
        else
            tbCommonTactic = TB_TACTIC_NONE;
          */
    }
}

void tbRUs(featom *atom, regionhandle region)
{
    sdword width;
    fonthandle oldfont;
    //rectangle rect = region->rect;

    oldfont = fontMakeCurrent(tbObjectiveFont);

    //primModeSet2();
    //primRectSolid2(&rect, colRGB(0, 0, 0));

    width = fontWidthf("%d", universe.curPlayerPtr->resourceUnits);//width of number
    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - TB_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", universe.curPlayerPtr->resourceUnits);

    fontMakeCurrent(oldfont);
}

void tbShips(featom *atom, regionhandle region)
{
    sdword width;
    fonthandle oldfont;
    //rectangle rect = region->rect;

    oldfont = fontMakeCurrent(tbButtonCaptionFont);

    //primModeSet2();
    //primRectSolid2(&rect, colRGB(0, 0, 0));

    width = fontWidthf("%d", universe.curPlayerPtr->totalships);//width of number
    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - TB_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", universe.curPlayerPtr->totalships);

    fontMakeCurrent(oldfont);
}

bool tbRefreshBabyFunction(udword num, void *data, struct BabyCallBack *baby)
{
    if (!tbTaskBarActive) return (TRUE);

    tbCalcTotalShipCommands();

#ifdef DEBUG_STOMP
    regVerify(tbBaseRegion);
#endif
    bitSet(tbBaseRegion->status, RSF_DrawThisFrame);

    return !tbTaskBarActive;
}

void SetAtom(featom *atom, uword value)
{
    buttonhandle button = (buttonhandle)atom->region;

    bitSet(atom->status, value);
#ifdef DEBUG_STOMP
    regVerify(&button->reg);
#endif
    bitSet(button->reg.status, RSF_DrawThisFrame);

}

void tbSetTactics(TacticsType tactic)
{
    uword i;
    Ship *ship;

    for(i=0; i<selSelected.numShips; ++i)
    {
        ship = selSelected.ShipPtr[i];
        ship->tacticstype = tactic;
    }

    switch (tactic)
    {
    case Evasive:
        feToggleButtonSetFromScreen("TB_Tactics_E", TRUE, tbScreen);
        feToggleButtonSetFromScreen("TB_Tactics_A", FALSE, tbScreen);
        feToggleButtonSetFromScreen("TB_Tactics_N", FALSE, tbScreen);
        //SetAtom(tbAtomA, FALSE);
        //SetAtom(tbAtomN, FALSE);
        dbgMessage("\nE");
        break;

    case Neutral:
        feToggleButtonSetFromScreen("TB_Tactics_N", TRUE, tbScreen);
        feToggleButtonSetFromScreen("TB_Tactics_A", FALSE, tbScreen);
        feToggleButtonSetFromScreen("TB_Tactics_E", FALSE, tbScreen);
        //SetAtom(tbAtomA, FALSE);
        //SetAtom(tbAtomE, FALSE);
        dbgMessage("\nN");
    break;

    case Aggressive:
        feToggleButtonSetFromScreen("TB_Tactics_A", TRUE, tbScreen);
        feToggleButtonSetFromScreen("TB_Tactics_E", FALSE, tbScreen);
        feToggleButtonSetFromScreen("TB_Tactics_N", FALSE, tbScreen);
        //SetAtom(tbAtomE, FALSE);
        //SetAtom(tbAtomN, FALSE);
        dbgMessage("\nA");
        break;
    default:
        break;
    }
}

void tbTacticsEvasive(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSetFromScreen(name, (tbShipsEvas > 0), tbScreen);
        tbAtomE = atom;

        dbgMessage("\nFirst Evasive call!");
    }
    else
    {
        tbSetTactics(Evasive);
    }
}

void tbTacticsNeutral(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSetFromScreen(name, (tbShipsNeut > 0), tbScreen);
        tbAtomN = atom;
        dbgMessage("\nFirst Neutral call!");
    }
    else
    {
        tbSetTactics(Neutral);
    }
}

void tbTacticsAggressive(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSetFromScreen(name, (tbShipsAggr > 0), tbScreen);
        tbAtomA = atom;
        dbgMessage("\nFirst Aggressive call!");
    }
    else
    {
        tbSetTactics(Aggressive);
    }
}

void tbRefreshTacticsCheckboxes(void)
{
    if (tbShipsAggr == 0)
        feToggleButtonSetFromScreen("TB_Tactics_A", FALSE, tbScreen);
    else
        feToggleButtonSetFromScreen("TB_Tactics_A", TRUE, tbScreen);

    if (tbShipsNeut == 0)
        feToggleButtonSetFromScreen("TB_Tactics_N", FALSE, tbScreen);
    else
        feToggleButtonSetFromScreen("TB_Tactics_N", TRUE, tbScreen);

    if (tbShipsEvas == 0)
        feToggleButtonSetFromScreen("TB_Tactics_E", FALSE, tbScreen);
    else
        feToggleButtonSetFromScreen("TB_Tactics_E", TRUE, tbScreen);
}

regionhandle tbFindRegion(char* name)
{
    sdword i;
    featom* atom;

    for (i = 0; i < tbScreen->nAtoms; i++)
    {
        atom = &tbScreen->atoms[i];
        if (atom->name != NULL &&
            strcasecmp(atom->name, name) == 0)
        {
            return (regionhandle)atom->region;
        }
    }
    return NULL;
}

void tbSetupHyperspace(void)
{
    regionhandle hs = tbFindRegion("CSM_Hyperspace");
    regionhandle lw = tbFindRegion("TB_ObjectivesWindowInit");
    if (hs == NULL || lw == NULL)
    {
        return;
    }
#ifndef HW_Release
    regVerify(hs);
    regVerify(lw);
#endif

    if (tbHypObjState == TB_Both)
    {
        tbHypObjState = TB_Objectives;
        regRegionScroll(hs, 0, TB_OffScreenAmount);
    }

    if (singlePlayerGame)
    {
        if (singlePlayerGameInfo.playerCanHyperspace)
        {
            if (tbHypObjState == TB_Objectives)
            {
                tbHypObjState = TB_Hyperspace;
                regRegionScroll(hs, 0, -TB_OffScreenAmount);
                regRegionScroll(lw, 0, TB_OffScreenAmount);
            }
        }
        else
        {
            if (tbHypObjState == TB_Hyperspace)
            {
                tbHypObjState = TB_Objectives;
                regRegionScroll(hs, 0, TB_OffScreenAmount);
                regRegionScroll(lw, 0, -TB_OffScreenAmount);
            }
        }
    }
    else
    {
        if (tbHypObjState == TB_Hyperspace)
        {
            tbHypObjState = TB_Objectives;
            regRegionScroll(hs, 0, TB_OffScreenAmount);
            regRegionScroll(lw, 0, -TB_OffScreenAmount);
        }
    }

    /*    if (tbSetup)
    {
        //reset if still setup
        void tbResetHyperspace(void);
        tbResetHyperspace();
    }

    tbSetup = TRUE;

    tbSP = singlePlayerGame;

    if (singlePlayerGame)
    {
        tbHS = singlePlayerGameInfo.playerCanHyperspace;
        if (singlePlayerGameInfo.playerCanHyperspace)
        {
            regRegionScroll(lw, 0, TB_OffScreenAmount);
        }
        else
        {
            regRegionScroll(hs, 0, TB_OffScreenAmount);
        }
    }
    else
    {
        regRegionScroll(hs, 0, TB_OffScreenAmount);
    }*/
}

void tbResetHyperspace(void)
{
/*    regionhandle hs = tbFindRegion("CSM_Hyperspace");
    regionhandle lw = tbFindRegion("TB_ObjectivesWindowInit");
    if (hs == NULL || lw == NULL)
    {
        return;
    }
#ifndef HW_Release
    regVerify(hs);
    regVerify(lw);
#endif

    if (!tbSetup)
    {
        //don't reset if not setup
        return;
    }

    tbSetup = FALSE;

    if (tbSP)
    {
        if (tbHS)
        {
            regRegionScroll(lw, 0, -TB_OffScreenAmount);
        }
        else
        {
            regRegionScroll(hs, 0, -TB_OffScreenAmount);
        }
    }
    else
    {
        regRegionScroll(hs, 0, -TB_OffScreenAmount);
    }*/
}

/*void tpSetupHyperspaceTaskActive(void)
{
    regionhandle hs = tbFindRegion("CSM_Hyperspace");
    regionhandle lw = tbFindRegion("TB_ObjectivesWindowInit");
    if (hs == NULL || lw == NULL)
    {
        return;
    }
#ifndef HW_Release
    regVerify(hs);
    regVerify(lw);
#endif

    if (tbSetup)
    {
        //reset if still setup
        void tbResetHyperspace(void);
        tbResetHyperspace();
    }

    tbSetup = TRUE;

    tbSP = singlePlayerGame;

    if (singlePlayerGame)
    {
        tbHS = singlePlayerGameInfo.playerCanHyperspace;
        if (singlePlayerGameInfo.playerCanHyperspace)
        {
            regRegionScroll(lw, 0, 400);
        }
        else
        {
            regRegionScroll(hs, 0, 400);
        }
    }
    else
    {
        regRegionScroll(hs, 0, 400);
    }
}*/

void tbTaskBarInit(void)
{
    tbCalcTotalShipCommands();
    tbRefreshTacticsCheckboxes();
    tbRefreshBaby = taskCallBackRegister(tbRefreshBabyFunction, 0, NULL, (real32)TB_RefreshInterval);
    tbTaskBarActive = TRUE;
    //dbgMessage("\nTaskbar on ");

    tbSetupHyperspace();
}

void tbTaskBarEnd(void)
{
    tbTaskBarActive = FALSE;
    //dbgMessage("\nTaskbar off ");
}

void PossiblyResetTaskbar(void)
//this is called by gameEnd
{
    if (tbTaskBarActive)
    {
        if (tbRegionsAttached)
        {
            regRegionScroll(tbBumperRegion, 0, tbBumpFullHeight);
            regMoveLinkChild(tbBaseRegion, NULL);
            tbRegionsAttached = FALSE;

            tbTaskBarEnd();
        }
    }
}

void feToggleButtonSetFromScreen(char *name, sdword bPressed, fescreen *screen)
{
    featom *atom;
    buttonhandle button;

    atom = feAtomFindInScreen(screen,
                              name);                        //find first named atom in screen
    if (atom == NULL) return;
    dbgAssert(atom != NULL);

    button = (buttonhandle)atom->region;

    if (bPressed)
    {
        bitSet(atom->status, FAS_Checked);
#ifdef DEBUG_STOMP
        regVerify(&button->reg);
#endif
       bitSet(button->reg.status, RSF_DrawThisFrame);
    }
    else
    {
        bitClear(atom->status, FAS_Checked);
#ifdef DEBUG_STOMP
        regVerify(&button->reg);
#endif
        bitSet(button->reg.status, RSF_DrawThisFrame);
    }
}
