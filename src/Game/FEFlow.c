/*=============================================================================
    Name    : FeFlow.c
    Purpose : Functions for processing front end screens

    Created 7/9/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#ifdef _WIN32
#include <windows.h>
#endif
#endif
#include "Types.h"
#include "File.h"
#include "Memory.h"
#include "Debug.h"
#include "font.h"
#include "FontReg.h"
#include "prim2d.h"
#include "utility.h"
#include "UIControls.h"
#include "FEReg.h"
#include "main.h"
#include "FEFlow.h"
#include "glinc.h"
#include "NIS.h"
#include "mainrgn.h"
#include "Scroller.h"
#include "mouse.h"
#include "glcaps.h"
#include "Strings.h"
#include "Tutor.h"
#include "SoundEvent.h"
#include "glcompat.h"

#include "Tactics.h"            //long story

/*=============================================================================
    Defines:
=============================================================================*/

// Spacer between listwindow element and the
#define LW_WindowXBarSpace  2

#define LW_BarWidth         16
#define LW_BarXPos          10
#define LW_BarHeight        0
#define LW_BarYPos          0

/*=============================================================================
    Data:
=============================================================================*/
extern bool mrMenuDontDisappear;  //TRUE if the menu shouldn't disappear

//TRUE if a blank screen is to be skipped by not flipping buffers
sdword feDontFlush = FALSE;

//TRUE if running in software.  used by feShouldSaveMouseCursor() - don't check this directly
bool feSavingMouseCursor;
//global variable indicating total redraw should occur
bool feRenderEverything = TRUE;

//list of front end callbacks
sdword feNumberCallbacks = FE_NumberCallbacks;
sdword feCallbackIndex;
fecallback *feCallback;

//list of front end draw callbacks
sdword feNumberDrawCallbacks = FE_NumberDrawCallbacks;
sdword feDrawCallbackIndex;
fedrawcallback *feDrawCallback;

//list of front end screens
sdword feNumberScreens = FE_NumberScreens;
sdword feScreenIndex;
fescreen **feScreen;

//front-end screen stack
festackentry feStack[FE_StackDepth];            //actual stack data
sdword feStackIndex;                            //index of CURRENT screen on stack

//menu attributes
color feMenuSelectedColor = FE_MenuSelectedColor;
//!!! some possibly suspect use of this pointer ???
sdword feMenuLevel = 0;
fescreen *feTempMenuScreen = NULL;

//tabstop
udword feTabStop = 1;

//margins for menus
sdword feMenuScreenMarginX = 0, feMenuScreenMarginY = 0;
sdword feMenuOverlapMarginX = 0, feMenuOverlapMarginY = 0;

/*=============================================================================
    Private functions:
=============================================================================*/
#if FEF_TEST
/*
void feTestCallback(char *string)
{
    dbgMessagef("\nfeTestCallback: '%s'", string);
}
void feTestQuit(char *string)
{
    dbgMessage("\nfeTestQuit");
    feCurrentScreenDelete();
}
*/
void uicTestTextEntry(char *name, featom *atom)
{
    uictextentry *entry = (uictextentry *)atom->pData;
    regVerify(&entry->reg);
    if (FEFIRSTCALL(atom))
    {
        uicTextEntrySet(entry, "Edit this text for cryin out lewd!", 6);
        uicTextBufferResize(entry, 36);
    }
    else
    {
        switch (uicTextEntryMessage(atom))
        {
            case CM_GainFocus:
                dbgMessagef("\nEntry '%s' gain focus.", name);
                break;
            case CM_LoseFocus:
                dbgMessagef("\nEntry '%s' lose focus.", name);
                break;
            case CM_AcceptText:
                dbgMessagef("\nEntry '%s' accept text '%s'.", name, ((textentryhandle)atom->pData)->textBuffer);
                break;
            case CM_RejectText:
                dbgMessagef("\nEntry '%s' reject text '%s'.", name, ((textentryhandle)atom->pData)->textBuffer);
                break;
            default:
                break;
                dbgAssert(FALSE);
        }
    }
}

#endif

/*-----------------------------------------------------------------------------
    Name        : feShouldSaveMouseCursor
    Description : for deciding whether to save the mouse cursor, perform region
                  dirtying, &c
    Inputs      :
    Outputs     :
    Return      : >0 (yes) or 0 (no)
----------------------------------------------------------------------------*/
bool glcfeShouldSaveMouseCursor(void)
{
    extern bool hrRunning;

    if (!glcActive())
    {
        //not active, no saving
        return FALSE;
    }

    //possible exceptions to general active rule
    if (hrRunning)          return FALSE;
    if (feRenderEverything) return FALSE;
    if (nisIsRunning)       return FALSE;
    if (smSensorsActive)    return FALSE;

    //should always be TRUE, as glcompat wouldn't be active otherwise
    return glCapFeatureExists(GL_SWAPFRIENDLY);
}
bool feShouldSaveMouseCursor(void)
{
    extern bool lmActive;
    extern bool hrRunning;

    if (RGLtype != SWtype)
    {
        return glcfeShouldSaveMouseCursor();
    }

    if (hrRunning)
    {
        return feSavingMouseCursor;
    }
    if (feRenderEverything)
    {
        return FALSE;
    }
    //launch manager
    if (lmActive)
    {
        return FALSE;
    }
    //titan picker
/*    if (tpActive)
    {
        return FALSE;
    }*/
    //nis running
    if (nisIsRunning)
    {
        return FALSE;
    }
    //sensors manager active (redundant)
    if (smSensorsActive)
    {
        return FALSE;
    }
    //game running and not in a fullscreen gui
    if (gameIsRunning && mrRenderMainScreen)
    {
        return FALSE;
    }
    return feSavingMouseCursor;
}

/*-----------------------------------------------------------------------------
    Name        : feFunctionExecute
    Description : Execute a named callback function
    Inputs      : name - name of function to execute
                  atom - atom which generates function call
                  firstcall - TRUE if this is the funcs first time being called
    Outputs     : Searches the feCallback list for a match and executes match
    Return      : void
----------------------------------------------------------------------------*/
void feFunctionExecute(char *name, featom *atom, bool firstcall)
{
    sdword index;

    for (index = 0; index < feCallbackIndex; index++)
    {                                                       //search all function callbacks
        if (feNamesEqual(name, feCallback[index].name))
        {                                                   //if names match
            if (firstcall)
            {
                bitSet(atom->status, FAS_OnCreate);
            }

            tutGameMessage(feCallback[index].name);

            feCallback[index].function(feCallback[index].name, atom);//call the callback function
            bitClear(atom->status, FAS_OnCreate);
            return;
        }
    }
#if (FEF_VERBOSE_LEVEL || FEF_ERROR_CHECKING)
    dbgMessagef("\nfeFunctionExecute: function '%s' not found", name);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : feLinkFindInScreen
    Description : Find a link in a particular screen.
    Inputs      : screen - pointer to screen to look for link in
                  linkName - name of link to find
    Outputs     : ..
    Return      : point to link if found, NULL if not found
----------------------------------------------------------------------------*/
felink *feLinkFindInScreen(fescreen *screen, char *linkName)
{
    sdword index;

    for (index = 0; index < screen->nLinks; index++)
    {
        if (feNamesEqual(linkName, screen->links[index].name))
        {
            return(&screen->links[index]);
        }
    }
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : feAtomFindInScreen
    Description : Find a atom in a particular screen.
    Inputs      : screen - pointer to screen to look for atom in
                  atomName - name of atom to find
    Outputs     : ..
    Return      : point to atom if found, NULL if not found
----------------------------------------------------------------------------*/
featom *feAtomFindInScreen(fescreen *screen, char *atomName)
{
    sdword index;

    if (screen == NULL)
    {
        return NULL;
    }

    for (index = 0; index < screen->nAtoms; index++)
    {
        if (screen->atoms[index].name != NULL)
        {
            if (feNamesEqual(atomName, screen->atoms[index].name))
            {
                return(&screen->atoms[index]);
            }
        }
    }
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : feAtomFindNextInScreen
    Description : Find a atom in a particular screen which exists after this screen.
    Inputs      : screen - pointer to screen to look for atom in
                  atom - pointer to atom to start searching from
                  atomName - name of atom to find
    Outputs     : ..
    Return      : point to atom if found, NULL if not found
----------------------------------------------------------------------------*/
featom *feAtomFindNextInScreen(fescreen *screen, featom *atom, char *atomName)
{
    sdword index;

    for (index = 0; index < screen->nAtoms; index++)
    {                                                       //search all atoms in screen
        if (&screen->atoms[index] == atom)
        {                                                   //if this is the reference atom
            for (index++; index < screen->nAtoms; index++)
            {
                if (screen->atoms[index].name != NULL)
                {
                    if (feNamesEqual(atomName, screen->atoms[index].name))
                    {                                       //if name matches
                        return(&screen->atoms[index]);
                    }
                }
            }
        }
    }
    return(NULL);
}


// scrollbar functions for wheel movement

void feWheelNegative(listwindowhandle listwindow)
{
    sdword ui = (sdword)listwindow->UpperIndex-3;
    sdword i;

    if (ui < 0)
    {
        ui = 0;
        listwindow->topitem = (listitemhandle)listwindow->listofitems.head;
    }
    else
    {
        for (i=0;i<3;i++)
        {
            if (listwindow->topitem->link.prev != NULL)
            {
                listwindow->topitem = (listitemhandle)listwindow->topitem->link.prev;
            }
            else
            {
                listwindow->topitem = (listitemhandle)listwindow->listofitems.head;
                ui = 0;
                break;
            }
        }
    }

    if ((listwindow->ListTotal > listwindow->MaxIndex) &&
        (bitTest(listwindow->windowflags,UICLW_AutoScroll)))

    {
        bitSet(listwindow->windowflags,UICLW_AutoScrollOff);
    }
    listwindow->UpperIndex = (uword)ui;
}

void feWheelPositive(listwindowhandle listwindow)
{
    sdword ui = (sdword)listwindow->UpperIndex + 3;
    sdword i;

    while (ui + listwindow->MaxIndex > listwindow->ListTotal)
    {
        ui--;

        if (ui < 0)
        {
            ui = 0;
            listwindow->topitem = (listitemhandle)listwindow->listofitems.head;
            listwindow->UpperIndex = (uword)ui;
            return;
        }
    }

    for (i=0;i<(ui-listwindow->UpperIndex);i++)
        listwindow->topitem = (listitemhandle)listwindow->topitem->link.next;

    if ((listwindow->ListTotal-listwindow->UpperIndex >= listwindow->MaxIndex) &&
        (bitTest(listwindow->windowflags,UICLW_AutoScroll)))
    {
        bitClear(listwindow->windowflags,UICLW_AutoScrollOff);
    }

    listwindow->UpperIndex = (uword)ui;
}


/*-----------------------------------------------------------------------------
    Name        : feStandardScrollBarFunction
    Description : Processes the scroll bar info for each listwindow
    Inputs      : String, name of function that invoked the callback
                  atopm, featom that invoked the callback
    Outputs     : none
    Return      : none
----------------------------------------------------------------------------*/
void feStandardScrollBarFunction(char *string, featom *atom)
{
    sword            ind;
    sdword           i;
    listwindowhandle listwindow;
    scrollbarhandle  shandle;

    listwindow = ((scrollbarhandle)atom->pData)->listwindow;
    shandle = (scrollbarhandle)atom->pData;

    if (shandle->mouseMoved != 0)
    {
        sword divs, up;
        real32 mm;

        mm = (real32)shandle->mouseMoved;
        divs = (sword)(mm / shandle->divSize);
        up = (sword)(listwindow->UpperIndex + divs);
        if (up < 0)
            up = 0;
        else if (up > listwindow->ListTotal - listwindow->MaxIndex)
            up = (sword)(listwindow->ListTotal - listwindow->MaxIndex);
        if (up < 0)
        {
            up = 0;
        }

        if (up > listwindow->UpperIndex)
        {
            for (i=0;i<(up-listwindow->UpperIndex);i++)
            {
                if (listwindow->topitem->link.next)     // shouldn't be necessary, but fixes crash while scrolling simultaneously when something is added to list
                    listwindow->topitem = (listitemhandle)listwindow->topitem->link.next;
            }
        }
        else
        {
            for (i=0;i<(listwindow->UpperIndex-up);i++)
            {
                if (listwindow->topitem->link.prev)     // shouldn't be necessary, but fixes crash while scrolling simultaneously when something is added to list
                    listwindow->topitem = (listitemhandle)listwindow->topitem->link.prev;
            }
        }

#ifndef HW_Release
        if (listwindow->ListTotal > 0)
        {
            dbgAssert(listwindow->topitem != NULL);
        }
#endif

        listwindow->UpperIndex = (uword)up;

        scAdjustThumbwheel(shandle, (uword)listwindow->UpperIndex, (uword)listwindow->MaxIndex, (uword)listwindow->ListTotal);
        shandle->mouseMoved = 0;    //clear flag

        if ((listwindow->ListTotal > listwindow->MaxIndex) &&
            (bitTest(listwindow->windowflags,UICLW_AutoScroll)))

        {
            bitSet(listwindow->windowflags,UICLW_AutoScrollOff);
        }
        if ((listwindow->ListTotal-listwindow->UpperIndex <= listwindow->MaxIndex) &&
            (bitTest(listwindow->windowflags,UICLW_AutoScroll)))
        {
            bitClear(listwindow->windowflags,UICLW_AutoScrollOff);
        }

        dbgAssert(listwindow!=NULL);
#ifdef DEBUG_STOMP
        regVerify(&listwindow->reg);
#endif
        listwindow->reg.status |= RSF_DrawThisFrame;

        return;
    }

    switch (shandle->event)
    {
    case SC_Other:
        ind = (sword)listwindow->UpperIndex;
        if (mouseCursorY() < shandle->thumb.y0)
        {
            ind = (sword)(ind - listwindow->MaxIndex);
            if (ind < 0) ind = 0;
            if (ind > listwindow->UpperIndex)
            {
                for (i=0;i<(ind-listwindow->UpperIndex);i++)
                {
                    listwindow->topitem = (listitemhandle)listwindow->topitem->link.next;
                }
            }
            else
            {
                for (i=0;i<(listwindow->UpperIndex-ind);i++)
                {
                    listwindow->topitem = (listitemhandle)listwindow->topitem->link.prev;
                }
            }
            listwindow->UpperIndex = (uword)ind;
            if ((listwindow->ListTotal > listwindow->MaxIndex) &&
                (bitTest(listwindow->windowflags,UICLW_AutoScroll)))

            {
                bitSet(listwindow->windowflags,UICLW_AutoScrollOff);
            }
        }
        else
        {
            ind = (sword)(ind + listwindow->MaxIndex);
            if (ind > listwindow->ListTotal - listwindow->MaxIndex)
                ind = (sword)(listwindow->ListTotal - listwindow->MaxIndex);
            if (ind < 0) ind = 0;
            if (ind > listwindow->UpperIndex)
            {
                for (i=0;i<(ind-listwindow->UpperIndex);i++)
                {
                    listwindow->topitem = (listitemhandle)listwindow->topitem->link.next;
                }
            }
            else
            {
                for (i=0;i<(listwindow->UpperIndex-ind);i++)
                {
                    listwindow->topitem = (listitemhandle)listwindow->topitem->link.prev;
                }
            }
            listwindow->UpperIndex = (uword)ind;
            if ((listwindow->ListTotal-listwindow->UpperIndex <= listwindow->MaxIndex) &&
                (bitTest(listwindow->windowflags,UICLW_AutoScroll)))
            {
                bitClear(listwindow->windowflags,UICLW_AutoScrollOff);
            }
        }

        dbgAssert(listwindow!=NULL);
#ifdef DEBUG_STOMP
        regVerify(&listwindow->reg);
#endif
       listwindow->reg.status |= RSF_DrawThisFrame;
        break;
    case SC_Negative:
        feWheelNegative(listwindow);
        dbgAssert(listwindow!=NULL);
#ifdef DEBUG_STOMP
        regVerify(&listwindow->reg);
#endif
        listwindow->reg.status |= RSF_DrawThisFrame;
        break;
    case SC_Positive:
        feWheelPositive(listwindow);
        dbgAssert(listwindow!=NULL);
#ifdef DEBUG_STOMP
        regVerify(&listwindow->reg);
#endif
        listwindow->reg.status |= RSF_DrawThisFrame;
        break;
    default:
        dbgMessagef("\nunhandled List Window scroll event %d", shandle->clickType);
    }

    scAdjustThumbwheel(shandle, (uword)listwindow->UpperIndex, (uword)listwindow->MaxIndex, (uword)listwindow->ListTotal);
}

/*-----------------------------------------------------------------------------
    Name        : feScrollBarProcess
    Description : Region processor callback for front end scrollbars
    Inputs      : region - region to handle processing
                  ID - used-assigned id set when region created
                  event - enumeration of event to be processed
                  data - additional message-specific event data
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
udword feScrollBarProcess(regionhandle region, sdword ID, udword event, udword data)
{
    featom *atom=NULL;
    scrollbarhandle shandle=NULL;

    switch (ID)
    {
        case UIC_ScrollbarTab:
            shandle = (scrollbarhandle)region;
            break;
        case UIC_ScrollbarUpButton:
        case UIC_ScrollbarDownButton:
            shandle = ((scrollbarbuttonhandle)region)->scrollbar;
            break;
    }

    atom = shandle->reg.atom;

    dbgAssert(atom->type == FA_ScrollBar);

    if (event == CM_ButtonClick && (atom->flags & FAF_Function))
    {
        if (atom->name == NULL)
        {
            dbgMessage("\natom has no name in feScrollBarProcess");
            return 0;
        }
        shandle->event = (udword)data;
        feFunctionExecute(atom->name, atom, FALSE);
    }
    else if (event == CM_ThumbMoved)
    {
        if (shandle->isVertical)
        {
            //vertical scroller
            udword y, oy;
            sword ry;
            y = data & 0xFFFF;
            oy = shandle->mouseY;
            ry = (sword)((sdword)y - (sdword)oy);
            shandle->mouseMoved = ry;
            shandle->event = (udword)data;
            feFunctionExecute(atom->name, atom, FALSE);
        }
        else
        {
            //horizontal scroller
            udword x, ox;
            sword rx;
            x = data >> 16;
            ox = shandle->mouseX;
            rx = (sword)((sdword)x - (sdword)ox);
            shandle->mouseMoved = rx;
            shandle->event = (udword)data;
            feFunctionExecute(atom->name, atom, FALSE);
        }
    }

    return 0;
}


/*-----------------------------------------------------------------------------
    Name        : feListWindowProcess
    Description : Region processor callback for front end listwindows
    Inputs      : region - region to handle processing
                  ID - used-assigned id set when region created
                  event - enumeration of event to be processed
                  data - additional message-specific event data
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
udword feListWindowProcess(regionhandle region, sdword ID, udword event, udword data)
{
    featom *atom = (featom *)ID;
    listwindowhandle listwindow = (listwindowhandle)region;

    dbgAssert(atom->type == FA_ListWindow);

    if (event == RPE_PressLeft)
    {                                                       //left press (select/add job)
/*        index = rmSelectTechType(region, mouseCursorY());

        if (index != -1)
        {
            rmCurSelectedTech = index;
            techinfo = PlayerTechList[index].itemID;
        }*/
    }
    else if (event == RPE_WheelUp)
    {
        feWheelNegative(listwindow);
        scAdjustThumbwheel(listwindow->scrollbar, (uword)listwindow->UpperIndex, (uword)listwindow->MaxIndex, (uword)listwindow->ListTotal);
    }
    else if (event == RPE_WheelDown)
    {
        feWheelPositive(listwindow);
        scAdjustThumbwheel(listwindow->scrollbar, (uword)listwindow->UpperIndex, (uword)listwindow->MaxIndex, (uword)listwindow->ListTotal);
    }
    return(0);
}


/*-----------------------------------------------------------------------------
    Name        : feButtonProcess
    Description : Region processor callback for front end buttons
    Inputs      : region - region to handle processing
                  ID - user-assigned ID set when region created
                  event - enumeration of event to be processed
                  data - additional event data (message specific)
    Outputs     : ...user defined...
    Return      : flags indicating further operation:

----------------------------------------------------------------------------*/
udword feButtonProcess(regionhandle region, sdword ID, udword event, udword data)
{
    featom *atom = (featom *)ID;
    felink *link;

    dbgAssert(atom->type == FA_Button || atom->type == FA_ToggleButton ||
              atom->type == FA_CheckBox || atom->type == FA_RadioButton ||
              atom->type == FA_BitmapButton || atom->type == FA_DragButton);

#if FEF_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfeButtonProcess: front end button '%s' hit.", atom->name ? atom->name : "NULL");
#endif

    if (atom->flags & FAF_Link)
    {
#if FEF_VERBOSE_LEVEL >= 2
        dbgMessagef("\nfeButtonProcess: link to '%s' hit.", atom->name ? atom->name : "NULL");
#endif
#if FEF_ERROR_CHECKING
        if (atom->name == NULL)
        {
            dbgNonFatal(DBG_Loc, "\nfeButtonProcess: NULL link button hit");
            return(0);
        }
#endif
        link = feLinkFindInScreen(feStack[feStackIndex].screen, atom->name);//find named link for button
#if FEF_ERROR_CHECKING
        if (link == NULL)
        {
            dbgNonFatalf(DBG_Loc, "\nfeButtonProcess: no link found in screen '%s' for link button '%s'", feStack[feStackIndex].screen->name, atom->name);
            return(0);
        }
#endif

        dbgAssert(link->linkToName != NULL);                //make sure we can link
        feScreenStart(feStack[feStackIndex].parentRegion, link->linkToName);          //start new screen
    }
    else if (atom->flags & FAF_Function)
    {
        dbgAssert(atom->name != NULL);
#if FEF_VERBOSE_LEVEL >= 2
        dbgMessagef("\nfeButtonProcess: function button '%s' hit.", atom->name ? atom->name : "NULL");
#endif
        feFunctionExecute(atom->name, atom, FALSE);
    }
#if FEF_ERROR_CHECKING
    else
    {
        dbgMessagef("\nfeButtonProcess: front end button '%s' does nothing.", atom->name ? atom->name : "NULL");
    }
#endif
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : feStaticTextDraw
    Description : Draw a static text region
    Inputs      : region - handle of region who's text is to be drawn
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void feStaticTextDraw(regionhandle region)
{
    fonthandle  fhSave;
    char *string;
    featom *atom = (featom *)region->userID;
    sdword index, x, width;

    for (index = 0, string = (char *)atom->pData; index < strCurLanguage; index++)
    {                                                       //find the next string
        string = (char *)memchr(string, 0, SDWORD_Max) + 1;//find the NULL terminator + 1
        /*
        while (!*string)
        {
            string++;                                       //skip any zero padding
        }
        */
    }

    fhSave = fontMakeCurrent((fonthandle)atom->attribs);             //select the appropriate font

    switch (atom->flags & FAM_Justification)
    {
        case FAM_JustLeft:
            x = region->rect.x0;
            break;
        case FAM_JustRight:
            width = fontWidth(string);
            x = region->rect.x1 - width;
            break;
        case FAM_JustCentre:
            width = fontWidth(string);
            x = (atom->width - width) / 2 + region->rect.x0;
            break;
#if FE_ERROR_CHECKING
        default:
            dbgFatalf(DBG_Loc, "Bad justification: %d", atom->flags & FAM_Justification);
#endif
    }

    dbgAssert(atom->pData);                                 //verify there is a string
    if (atom->flags & FAM_DropShadow)
    {
        fontShadowSet((atom->flags & FAM_DropShadow) >> FSB_DropShadow, atom->borderColor);
    }
//    if (RGL && (RGLtype == SWtype))
//    {
//       primRectSolid2(&region->rect, atom->contentColor);
//    }
    fontPrint(x, region->rect.y0, atom->contentColor, string);
    fontShadowSet(FS_NONE, 0);
    fontMakeCurrent(fhSave);
}

/*-----------------------------------------------------------------------------
    Name        : feStaticRectangleDraw
    Description : Draw static rectangle FE regions
    Inputs      : region - handle of region who's rectangle is to be drawn
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void feStaticRectangleDraw(regionhandle region)
{
    featom *atom = (featom *)region->userID;

    dbgAssert(atom != NULL);                                //verify there is an atom

    if (bitTest(atom->flags, FAF_ContentsVisible))
    {
        primRectSolid2(&region->rect, atom->contentColor);  //draw rectangle insides
    }

    if (bitTest(atom->flags, FAF_BorderVisible))
    {
#if FE_TEXTURES_DISABLABLE
        if (fetEnableTextures)
        {
#endif
            ferDrawBoxRegion(region->rect, (drawtype)region->drawstyle[0],
                            (drawtype)region->drawstyle[1], &region->cutouts, bitTest(atom->status, FAS_UseAlpha));
#if FE_TEXTURES_DISABLABLE
        }
        else
        {
            //draw rectangle outline
            primSeriesOfRoundRects(&region->rect, 1, atom->borderColor, 0, 3, 4, 4);
        }
#endif
    }
}

/*-----------------------------------------------------------------------------
    Name        : feUserRegionDraw
    Description : Draw a user region
    Inputs      : region - pointer to region which represents atom
    Outputs     : Looks up and calls the correct draw function for atom
    Return      : void
----------------------------------------------------------------------------*/
void feUserRegionDraw(regionhandle region)
{
    featom *atom = (featom *)region->userID;
    fedrawfunction function;

    function = (fedrawfunction)atom->pData;                 //get draw function
    function(atom, region);                                 //call the draw function
}

/*-----------------------------------------------------------------------------
    Name        : feBaseRegionDraw
    Description : Draw the base region behind all dialog items.
    Inputs      : region - pointer to region which represents atom
    Outputs     : Looks up and calls the correct draw function for atom
    Return      : void
----------------------------------------------------------------------------*/
void feBaseRegionDraw(regionhandle region)
{
    featom *atom = (featom *)region->userID;

    dbgAssert(atom != NULL);                                //verify there is an atom

//    primRectSolid2(&region->rect, atom->contentColor);
//    primRectOutline2(&region->rect, 3, atom->borderColor);
    if (bitTest(atom->flags, FAF_ContentsVisible))
    {
        primBeveledRectSolid(&region->rect, atom->contentColor, 2, 2);
    }


    if (bitTest(atom->flags, FAF_BorderVisible))
    {
#if FE_TEXTURES_DISABLABLE
        if (fetEnableTextures)
        {
#endif
            ferDrawBoxRegion(region->rect, (drawtype)region->drawstyle[0],
                            (drawtype)region->drawstyle[1], &region->cutouts, bitTest(atom->status, FAS_UseAlpha));
#if FE_TEXTURES_DISABLABLE
            return;
        }

        primSeriesOfRoundRects(&region->rect, 1,
                               atom->borderColor, 0, 5,
                               4, 4);
#endif
    }
}

/*-----------------------------------------------------------------------------
    Name        : feDrawCallbackFind
    Description : Find a named render callback
    Inputs      : name - name of callback to fond
    Outputs     : ..
    Return      : Pointer to callback function or NULL if none
----------------------------------------------------------------------------*/
fedrawfunction feDrawCallbackFind(char *name)
{
    sdword index;

    for (index = 0; index < feDrawCallbackIndex; index++)
    {
        if (feNamesEqual(name, feDrawCallback[index].name))
        {
            return(feDrawCallback[index].function);
        }
    }
#if FEF_VERBOSE_LEVEL >= 1
    dbgMessagef("\nUnable to find draw callback for '%s'", name);
#endif
    return(NULL);
}


/*-----------------------------------------------------------------------------
    Name        : feFindCutoutRegion
    Description : Finds the regions that get cut out by the cutout region
    Inputs      : x0, y0, x1, y1 - coordinates describing cutout region
    Outputs     : adds the coordinates to the region structure that gets cut
    Return      :
----------------------------------------------------------------------------*/
#define BaR  BaseRegion->rect
#define ReR  region->rect
#define CuC  cur_cutout->rect
void feFindCutoutRegion(LinkedList *cutouts, regionhandle BaseRegion)
{
    regionhandle region;
    featom *atom;
    cutouttype *cur_cutout, *new_cutout;
    Node *node;

    if (cutouts->num == 0)
    {
        return;
    }

    node = cutouts->head;

    while (node != NULL)
    {
        cur_cutout = listGetStructOfNode(node);

        atom = (featom*)cur_cutout->region->atom;

        if (!(bitTest(atom->flags, FAF_DontCutoutBase)))
        {
            //add the cutout region to the BaseRegion if needed
            if (IS_IN_RECT(CuC.x0, CuC.y0, BaR.x0, BaR.y0, BaR.x1, BaR.y1) ||
                IS_IN_RECT(CuC.x1, CuC.y1, BaR.x0, BaR.y0, BaR.x1, BaR.y1) ||
                IS_IN_RECT(CuC.x1, CuC.y0, BaR.x0, BaR.y0, BaR.x1, BaR.y1) ||
                IS_IN_RECT(CuC.x0, CuC.y1, BaR.x0, BaR.y0, BaR.x1, BaR.y1))
            {
                new_cutout = memAlloc(sizeof(cutouttype), "cutout", NonVolatile);
                new_cutout->rect.x0 = CuC.x0;
                new_cutout->rect.y0 = CuC.y0;
                new_cutout->rect.x1 = CuC.x1;
                new_cutout->rect.y1 = CuC.y1;
                listAddNode(&BaseRegion->cutouts, &new_cutout->node, new_cutout);
            }
        }

        region = cur_cutout->region->next;

        //add the cutout to any region already loaded
        while (region != NULL)
        {
            if (IS_IN_RECT(CuC.x0, CuC.y0, ReR.x0, ReR.y0, ReR.x1, ReR.y1) ||
                IS_IN_RECT(CuC.x1, CuC.y1, ReR.x0, ReR.y0, ReR.x1, ReR.y1) ||
                IS_IN_RECT(CuC.x1, CuC.y0, ReR.x0, ReR.y0, ReR.x1, ReR.y1) ||
                IS_IN_RECT(CuC.x0, CuC.y1, ReR.x0, ReR.y0, ReR.x1, ReR.y1))
            {
                new_cutout = memAlloc(sizeof(cutouttype), "cutout", NonVolatile);
                new_cutout->rect.x0 = CuC.x0;
                new_cutout->rect.y0 = CuC.y0;
                new_cutout->rect.x1 = CuC.x1;
                new_cutout->rect.y1 = CuC.y1;
                new_cutout->region  = cur_cutout->region;
                listAddNode(&region->cutouts, &new_cutout->node, new_cutout);
            }
            region = region->next;
        }
        node = node->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : feFindChildrenForARegion
    Description : Recursive functio to find and re-parent the atoms from a
                    given child list.
    Inputs      : region - region to create children for
                  firstChild - first region to search backwards through
    Outputs     :firstChild - where to continue searching
    Return      : number of children added onto region
----------------------------------------------------------------------------*/
/*
sdword feFindChildrenForARegion(regionhandle region, regionhandle *firstChild)
{
    sdword hits = 1, totalHits = 0;
    regionhandle previousPrevious, sibling, child = *firstChild;

    RESTART:
    if (!hits)
    {
        return(totalHits);
    }
    if (region == NULL)
    {
        return(totalHits);
    }

    hits = 0;
    if (region->userID > 255)
    {
        //scan previous members and see if they want to become children
        while (child != NULL)
        {
            if (regRegionInside(child, region))
            {
                hits++;
                totalHits++;

                //remove child from siblings
                if (child->previous == NULL)
                {
                    if (child->next != NULL)
                    {
                        child->next->previous = NULL;
                    }
                    child->parent->child = child->next;
                }
                else
                {
                    child->previous->next = child->next;
                    child->next->previous = child->previous;
                }
                //dbgAssert(child->child == NULL);
                previousPrevious = child->previous;         //remember how to continue the search
                child->previous = NULL;
                child->next = NULL;
                if (*firstChild == child)
                {                                           //if we're removing the first in the list
                    *firstChild = previousPrevious;         //keep the level below us from getting a bad link
                }

                //add child to children
                if (region->child == NULL)
                {
                    region->child = child;
                }
                else
                {
                    for (sibling = region->child; sibling->next != NULL;)
                    {
                        sibling = sibling->next;
                    }

                    sibling->next = child;
                    child->previous = sibling;
                }

                //re-parent
                child->parent = region;

                if (previousPrevious != NULL)
                {                                           //recurse to build real deep hierarchies if applicable
                    feFindChildrenForARegion(child, &previousPrevious);
                }
                child = previousPrevious;
                goto RESTART;
            }

            child = child->previous;
        }
    }
    return(totalHits);
}
*/
/*-----------------------------------------------------------------------------
    Name        : feFindChildren
    Description : for all children of the base region, find possible other children inside
                  buttons and such and link them appropriately
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static ubyte feCanStack[] =
{
/* 0                            */ FALSE,
/* FA_UserRegion = 1,           */ TRUE,
/* FA_StaticText,               */ TRUE,
/* FA_Button,                   */ TRUE,
/* FA_CheckBox,                 */ TRUE,
/* FA_ToggleButton,             */ TRUE,
/* FA_ScrollBar,                */ FALSE,
/* FA_StatusBar,                */ FALSE,
/* FA_TextEntry,                */ TRUE,
/* FA_ListViewExpandButton,     */ FALSE,
/* FA_TitleBar,                 */ FALSE,
/* FA_MenuItem,                 */ TRUE,
/* FA_RadioButton,              */ TRUE,
/* FA_CutoutRegion,             */ FALSE,
/* FA_DecorativeRegion,         */ FALSE,
/* FA_Divider,                  */ FALSE,
/* FA_ListWindow,               */ FALSE,
/* FA_BitmapButton,             */ TRUE,
/* FA_HorizSlider,              */ FALSE,
/* FA_VertSlider,               */ FALSE,
/* FA_DragButton,               */ TRUE,
/* FA_OpaqueDecorativeRegion,   */ FALSE,
};
void feFindChildren(regionhandle baseRegion)
{
    regionhandle region, child, sibling, lastRegion;
    sdword hits = 1;
    featom *atom;

RESTART:
    if (!hits)
    {
        return;
    }
    region = baseRegion->child;
    if (region == NULL)
    {
        return;
    }

    hits = 0;
    while (region != NULL)
    {
        atom = (featom *)region->userID;
        if ((region->userID > 255) && feCanStack[atom->type])
        {
            //scan previous members and see if they want to become children
            child = region->previous;

            lastRegion = region;

            while (child != NULL)
            {
                if (regRegionInside(child, region))
                {
                    hits++;

                    //remove child from siblings
                    if (child->previous == NULL)
                    {
                        child->next->previous = NULL;
                        child->parent->child = child->next;
                    }
                    else
                    {
                        child->previous->next = child->next;
                        child->next->previous = child->previous;
                    }

                    child->previous = NULL;
                    child->next = NULL;

                    //add child to children
                    if (region->child == NULL)
                    {
                        region->child = child;
                    }
                    else
                    {
                        for (sibling = region->child; sibling->next != NULL;)
                        {
                            sibling = sibling->next;
                        }

                        sibling->next = child;
                        child->previous = sibling;
                    }

                    //re-parent
                    child->parent = region;

                    goto RESTART;
                }

                lastRegion = child;
                child = child->previous;
            }

        }
        region = region->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : feBaseRegionDrag
    Description : Region processor callback for dragging a screen about
    Inputs      : region - region to handle processing
                  ID - user-assigned ID set when region created
                  event - enumeration of event to be processed
                  data - additional event data (message specific)
    Outputs     : ...user defined...
    Return      : flags indicating further operation:
----------------------------------------------------------------------------*/
regionhandle feDragRegion = NULL;
sdword feDragMouseX;
sdword feDragMouseY;
udword feBaseRegionDrag(regionhandle region, sdword ID, udword event, udword data)
{
    switch (event)
    {
        case RPE_PressLeft:
            feDragRegion = region;
            feDragMouseX = mouseCursorX();
            feDragMouseY = mouseCursorY();
            break;
        case RPE_ReleaseLeft:
            feDragRegion = NULL;
            break;
        case RPE_HoldLeft:
            if (feDragRegion == region)
            {
                if (mouseCursorX() != feDragMouseX || mouseCursorY() != feDragMouseY)
                {
                    regRegionScroll(region, mouseCursorX() - feDragMouseX, mouseCursorY() - feDragMouseY);
                    feDragMouseX = mouseCursorX();
                    feDragMouseY = mouseCursorY();
                }
            }
            break;
        case RPE_ExitHoldLeft:
            if (feDragRegion == region)
            {
                regRegionScroll(region, mouseCursorX() - feDragMouseX, mouseCursorY() - feDragMouseY);
                feDragMouseX = mouseCursorX();              //scroll to new position
                feDragMouseY = mouseCursorY();
                bitSet(region->status, RSF_MouseInside);    //make region handler think we never left the region
            }
            break;
    }
    return(0);
}


/*-----------------------------------------------------------------------------
    Name        : feRegionsAdd
    Description : Adds UI controls and/or regions for each control atom in
                    specified screen
    Inputs      : screen - pointer to the screen whose atoms we are to add
                  parent - region to make this a child of
                  moveToFront - makes the window on top of it's siblings.
    Outputs     : Calls uicChildButtonAlloc or regChildAlloc for each screen
                    atom and regChildAlloc for a base 'dummy' region.
    Return      : Pointer to base 'dummy' region
----------------------------------------------------------------------------*/
regionhandle feRegionsAdd(regionhandle parent, fescreen *screen, bool moveToFront)
{
    sdword index;
    regionhandle baseRegion, region;
    featom *atom, *scatom;
    buttonhandle button;
    scrollbarhandle scroller;
    LinkedList cutouts;
    cutouttype *element;
    textentryhandle entry;
    listwindowhandle listwindow;

    //first atom is always a dummy atom to form the base.
    //It will never be rendered or recieve any messages.
    dbgAssert(screen->nAtoms >= 1);                         //make sure there is at least a blank screen
    atom = screen->atoms;

    baseRegion = regChildAlloc(parent, (sdword)atom, atom->x, atom->y, //create base dummy region
                               atom->width, atom->height, 0, RPE_PressLeft);
    if (moveToFront)
    {
        bitSet(baseRegion->status, RSF_PriorityRegion);
    }
    regSiblingMoveToFront(baseRegion);
    if (bitTest(atom->flags, FAF_Modal))
    {                                                       //if it's a modal screen
        regFilterSet(baseRegion, regFilterGet(baseRegion) | RPE_ModalBreak);
    }
    if (bitTest(atom->flags, FAF_Draggable))
    {
        regFilterSet(baseRegion, regFilterGet(baseRegion) | (RPE_PressLeft | RPE_HoldLeft | RPE_ReleaseLeft | RPE_ExitHoldLeft | RPE_ExitHoldRight));
        regFunctionSet(baseRegion, feBaseRegionDrag);
    }

//    bitSet(atom->flags,FAF_UseAlpha);

    regDrawFunctionSet(baseRegion, feBaseRegionDraw);

    baseRegion->drawstyle[0] = atom->drawstyle[0];
    baseRegion->drawstyle[1] = atom->drawstyle[1];

    atom = &screen->atoms[screen->nAtoms - 1];

    listInit(&cutouts);

    for (index = 1; index < screen->nAtoms; index++, atom--)
    {
/*
        if (bitTest(atom->flags, FAF_Hidden))
        {
            continue;
        }
*/
        region = 0;
//        bitSet(atom->flags,FAF_UseAlpha);
        switch (atom->type)
        {
            /* atom->region = NULL; */                            //assume no region for this atom
            case FA_UserRegion:                             //named old user region
                atom->pData = (ubyte *)feDrawCallbackFind(atom->name);

                if (atom->pData != NULL)
                {
                    region = regChildAlloc(baseRegion, (sdword)atom, atom->x, atom->y,
                                   atom->width, atom->height, 0, 0);
                    atom->region = (void*)region;
                    region->atom = atom;
                    if (bitTest(atom->flags, FAF_Hidden))
                    {
                        regDrawFunctionSet(region, NULL);
                    }
                    else
                    {
                        regDrawFunctionSet(region, feUserRegionDraw);
                    }
                }
                else
                {
                    atom->region = NULL;
#if FEF_ERROR_CHECKING
                    dbgMessagef("\nfeRegionsAdd: no User region handler for '%s'", atom->name);
#else
#endif
                }
                break;
            case FA_StaticText:                             //static text message
                region = regChildAlloc(baseRegion, (sdword)atom, atom->x, atom->y,
                               atom->width, atom->height, 0, 0);
                atom->region = (void*)region;
                region->atom = atom;
                if (bitTest(atom->flags, FAF_Hidden))
                {
                    regDrawFunctionSet(region, NULL);
                }
                else
                {
                    regDrawFunctionSet(region, feStaticTextDraw);
                }
                break;
            case FA_DecorativeRegion:
                region = regChildAlloc(baseRegion, (sdword)atom, atom->x, atom->y,
                                       atom->width, atom->height, 0, 0);
                region->atom = atom;
                region->userID = (sdword)atom->name;
                atom->region = (void*)region;
                if (bitTest(atom->flags, FAF_Hidden))
                {
                    regDrawFunctionSet(region, NULL);
                }
                else
                {
                    regDrawFunctionSet(region, ferDrawDecorative);
                }
                break;
            case FA_OpaqueDecorativeRegion:
                region = regChildAlloc(baseRegion, (sdword)atom, atom->x, atom->y,
                                       atom->width, atom->height, 0, 0);
                region->atom = atom;
                region->userID = (sdword)atom->name;
                atom->region = (void*)region;
                if (bitTest(atom->flags, FAF_Hidden))
                {
                    regDrawFunctionSet(region, NULL);
                }
                else
                {
                    regDrawFunctionSet(region, ferDrawOpaqueDecorative);
                }
                break;
            case FA_Button:                                 //button
            case FA_ToggleButton:
            case FA_CheckBox:
            case FA_RadioButton:
            case FA_BitmapButton:
            case FA_HorizSlider:
            case FA_VertSlider:
            case FA_DragButton:
                button = uicChildButtonAlloc(baseRegion, (sdword)atom, atom->x, atom->y,
                               atom->width, atom->height, feButtonProcess, atom->type | CM_ButtonClick);
                ((regionhandle)&button->reg)->atom = atom;
                atom->region = (void*)button;
                button->screen = screen;                    //button references screen
                feAcceleratorSet(&button->reg, atom);
                // add tabstops to button region
                regTabstopSet(&button->reg, atom->tabstop);
                if (atom->tabstop == 1)
                {
                    uicSetCurrent((regionhandle)button, FALSE);
                }
                uicContentColor(button, atom->contentColor);//set button colors
                uicBorderColor(button, atom->borderColor);
                break;
            case FA_ScrollBar:
                scroller = uicChildScrollBarAlloc(baseRegion, (sdword)atom,
                                                atom->x, atom->y, atom->width, atom->height,
                                                feScrollBarProcess, atom->type | CM_ButtonClick);
                ((regionhandle)&scroller->reg)->atom = atom;
                atom->region = (void*)scroller;
                scroller->screen = screen;
                uicContentColor(scroller, atom->contentColor);
                uicBorderColor(scroller, atom->borderColor);
                break;
            case FA_TextEntry:
                entry = uicChildTextEntryAlloc(baseRegion, atom, atom->x, atom->y, atom->width, atom->height, atom->type);
                ((regionhandle)&entry->reg)->atom = atom;
                atom->region = (void*)entry;
                entry->screen = screen;
                regTabstopSet(&entry->reg, atom->tabstop);
                if (atom->tabstop == 1)
                {
                    uicSetCurrent((regionhandle)entry, FALSE);
                }
                break;
            case FA_ListWindow:
                listwindow = uicChildListWindowAlloc(baseRegion, (sdword)atom,
                                                     atom->x, atom->y, atom->width, atom->height,
                                                     feListWindowProcess, atom->type | CM_ButtonClick);
                scatom = &listwindow->scrollbaratom;
                ((regionhandle)&listwindow->reg)->atom = atom;
                *(scatom)       = *(atom);
                scatom->name    = "feStandardScrollBarFunction";
                scatom->type    = FA_ScrollBar;
                scatom->x       = atom->x+atom->width+LW_WindowXBarSpace;
                scatom->y       = atom->y+LW_BarYPos;
                scatom->width   = LW_BarWidth;
                scatom->height  = atom->height+LW_BarHeight;
                scatom->pData   = NULL;
                scatom->attribs = NULL;

                listwindow->scrollbar = uicChildScrollBarAlloc((regionhandle)listwindow, (sdword)scatom,
                                                scatom->x, scatom->y, scatom->width, scatom->height,
                                                feScrollBarProcess, FA_ScrollBar | CM_ButtonClick);

                listwindow->scrollbaratom.region = (void*)listwindow->scrollbar;
                listwindow->scrollbar->reg.atom = &listwindow->scrollbaratom;
                listwindow->scrollbar->screen = screen;
                listwindow->scrollbar->listwindow = listwindow;
                atom->region = (void*)listwindow;
                listwindow->screen = screen;
                regTabstopSet(&listwindow->reg, atom->tabstop);
                if (atom->tabstop == 1)
                {
                    uicSetCurrent((regionhandle)listwindow, FALSE);
                }
                break;
            case FA_CutoutRegion:
                //build a linked list of cutout regions we deal with the cutout
                //regions properly once all the other regions are loaded in
                element = memAlloc(sizeof(cutouttype), "cutsorama", NonVolatile);
                element->rect.x0 = atom->x;
                element->rect.y0 = atom->y;
                element->rect.x1 = atom->x + atom->width;
                element->rect.y1 = atom->y + atom->height;
                element->region  = regChildAlloc(baseRegion, (sdword)atom, atom->x, atom->y,
                                                atom->width, atom->height, 0, 0);
                atom->region = (void*)element->region;
                element->region->atom = atom;
                listAddNode(&cutouts, &element->node, element);

                break;
            default://!!! for now, default to plain rectangles
                region = regChildAlloc(baseRegion, (sdword)atom, atom->x, atom->y,
                               atom->width, atom->height, 0, 0);
                atom->region = (void*)region;
                region->atom = atom;
                if (bitTest(atom->flags, FAF_Hidden))
                {
                    regDrawFunctionSet(region, NULL);
                }
                else
                {
                    regDrawFunctionSet(region, feStaticRectangleDraw);
                }
//                region->drawstyle[0] = atom->drawstyle[0];
//                region->drawstyle[1] = atom->drawstyle[1];
                break;
//#if FEF_VERBOSE_LEVEL >= 1
//            default:
//                dbgMessagef("\nfeRegionsAdd: unprocessed region type 0x%x (%d of %d) while adding screen '%s'",
//                            atom->type, index, screen->nAtoms, screen->name);
//#endif
        }

        if (region != NULL)
        {
            region->drawstyle[0] = atom->drawstyle[0];
            region->drawstyle[1] = atom->drawstyle[1];
        }
    }

    //now link up the cutout regions to the regions they cut out
#if FE_TEXTURES_DISABLABLE
    if (fetEnableTextures)
#endif
    {
        feFindCutoutRegion(&cutouts, baseRegion);

    }
    listDeleteAll(&cutouts);

//    if (feShouldSaveMouseCursor())
    if (!gameIsRunning || !mrRenderMainScreen)
    {
        feFindChildren(baseRegion);
    }

    return(baseRegion);
}

/*-----------------------------------------------------------------------------
    Name        : feAllCallOnCreate
    Description : Call all buttons with the FAF_CallOnCreate flag enabled
    Inputs      : screen - screen just created
    Outputs     : Any atoms with the FAF_CallOnCreate will have their function called.
    Return      : void
----------------------------------------------------------------------------*/
void feAllCallOnCreate(fescreen *screen)
{
    sdword index;
    featom *atom;

    atom = &screen->atoms[screen->nAtoms - 1];

    for (index = 1; index < screen->nAtoms; index++, atom--)
    {
        if (bitTest(atom->flags, FAF_CallOnCreate))
        {                                           //if we should call at creation
            feFunctionExecute(atom->name, atom, TRUE);
        }
    }
}

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : feStartup
    Description : Start the front end processor module
    Inputs      : void
    Outputs     : Allocates the front end callback list and screen list.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword feStartup(void)
{
#if FEF_VERBOSE_LEVEL >= 1
    dbgMessage("\nStarting front end module");
#endif
    feCallbackIndex = 0;                                    //clear the callback list
    feCallback = memAlloc(feNumberCallbacks *               //allocate new callback list
                 sizeof(fecallback), "Front end callback list", NonVolatile);
#if FEF_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfeStartup: allocated %d callbacks", feNumberCallbacks);
#endif

    feDrawCallbackIndex = 0;                                //clear the draw callback list
    feDrawCallback = memAlloc(feNumberDrawCallbacks *       //allocate new callback list
                 sizeof(fedrawcallback), "Front end draw callback list", NonVolatile);
#if FEF_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfeStartup: allocated %d draw callbacks", feNumberDrawCallbacks);
#endif

    feScreenIndex = 0;                                      //clear screen list
    feScreen = memAlloc(feNumberScreens * sizeof(fescreen *), //allocate screen list
                        "Front end screen list", NonVolatile);
#if FEF_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfeStartup: allocated %d screen pointers", feNumberScreens);
#endif
    dbgAssert(ghMainRegion != NULL);                        //first entry on stack (always there) is the main region
    feStack[0].parentRegion = NULL;
    feStack[0].baseRegion = ghMainRegion;
    feStack[0].screen = NULL;
    feStackIndex = 0;                                       //currently nothing on the stack

    feCallbackAdd("Disappear", feMenuDisappear);
    feCallbackAdd("SCREEN_Disappear", feScreenDisappear);
    feCallbackAdd("feStandardScrollBarFunction", feStandardScrollBarFunction);
#if FEF_TEST
//    fontMakeCurrent(fontLoad("ABC"));
//    feScreensLoad("testicle.fib");
//    feScreenStart(ghMainRegion, "MainScreen");
//    feCallbackAdd("TestFunction", feTestCallback);
//    feCallbackAdd("MainQuit", feTestQuit);
    feCallbackAdd("UIC_TestTextEntry", uicTestTextEntry);
#endif

    feSavingMouseCursor = glCapFeatureExists(GL_SWAPFRIENDLY);

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : feReset
    Description : resets some portions of the front end module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void feReset(void)
{
    feSavingMouseCursor = glCapFeatureExists(GL_SWAPFRIENDLY);
}

/*-----------------------------------------------------------------------------
    Name        : feShutdown
    Description : Shut down the front end module
    Inputs      : void
    Outputs     : frees callback list
    Return      : void
----------------------------------------------------------------------------*/
void feShutdown(void)
{
#if FEF_VERBOSE_LEVEL >= 1
    dbgMessage("\nClosing front end module");
#endif
    memFree(feDrawCallback);
    memFree(feCallback);
    memFree(feScreen);
}

/*-----------------------------------------------------------------------------
    Name        : feResRepositionX
    Description : adjusts a frontend screen's x coordinate for hires modes
    Inputs      : x - x coordinate
    Outputs     :
    Return      : adjusted x coordinate
----------------------------------------------------------------------------*/
sdword feResRepositionX(sdword x)
{
    return(x + ((MAIN_WindowWidth - 640) / 2));
}

/*-----------------------------------------------------------------------------
    Name        : feResRepositionY
    Description : adjusts a frontend screen's y coordinate for hires modes
    Inputs      : y - y coordinate
    Outputs     :
    Return      : adjusted y coordinate
----------------------------------------------------------------------------*/
sdword feResRepositionY(sdword y)
{
    return(y + ((MAIN_WindowHeight - 480) / 2));
}

/*-----------------------------------------------------------------------------
    Name        : feResRescaleBackground
    Description : rescales a background region to fill a higher res than 640x480
    Inputs      : atom - the atom to resize
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void feResRescaleBackground(featom* atom)
{
    sdword x0, y0;
    sdword x1, y1;
    sdword bottom, right;

    x0 = feResRepositionX(0);
    y0 = feResRepositionY(0);
    x1 = feResRepositionX(640 - 1);
    y1 = feResRepositionY(480 - 1);

    atom->x = feResRepositionX(atom->x);
    atom->y = feResRepositionY(atom->y);

    //top edge
    if (atom->y <= y0)
    {
        bottom = atom->y + atom->height;
        atom->y = 0;
        atom->height = bottom;
    }

    //left edge
    if (atom->x <= x0)
    {
        right = atom->x + atom->width;
        atom->x = 0;
        atom->width = right;
    }

    //bottom edge
    if ((atom->y + atom->height) >= y1)
    {
        bottom = MAIN_WindowHeight - 1;
        atom->height = bottom - atom->y;
    }

    //right edge
    if ((atom->x + atom->width) >= x1)
    {
        right = MAIN_WindowWidth - 1;
        atom->width = right - atom->x;
    }
}

/*-----------------------------------------------------------------------------
    Name        : fePointOnScreen
    Description : determines whether a given 2D coord is off-screen (@ 640x480)
    Inputs      : x, y - coordinates
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool fePointOnScreen(sdword x, sdword y)
{
    if ((x >= 0) &&
        (y >= 0) &&
        (x < 640) &&
        (y < 480))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : feAtomOnScreen
    Description : determines whether an atom is *ENTIRELY* on-screen (@ 640x480)
    Inputs      : atom - the atom to test
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool feAtomOnScreen(featom* atom)
{
    if (fePointOnScreen(atom->x, atom->y) ||
        fePointOnScreen(atom->x + atom->width, atom->y) ||
        fePointOnScreen(atom->x + atom->width, atom->y + atom->height) ||
        fePointOnScreen(atom->x, atom->y + atom->height))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : feScreensLoad
    Description : Load in a file containing any number of front end screens
    Inputs      : fileName - name of file to load
    Outputs     : allocates and loads entire file.  Pointers fized up.
    Return      : pointer to the header newly loaded.
----------------------------------------------------------------------------*/
fibfileheader *feScreensLoad(char *fileName)
{
    ubyte *loadAddress;
    fibfileheader *header;
    fescreen *screen;
    sdword screenIndex, index;
    bool menuItemsPresent;

    fileLoadAlloc(fileName, (void **)&loadAddress, NonVolatile);     //load in the file
    header = (fibfileheader *)loadAddress;                  //get a pointer to the header

#ifdef ENDIAN_BIG
	header->version = LittleShort( header->version );
	header->nScreens = LittleShort( header->nScreens );
#endif

#if FEF_ERROR_CHECKING
    if (strcmp(header->identify, FIB_Identify))             //verify header string
    {
        dbgFatalf(DBG_Loc, "feScreensLoad: Invalid header '%s' in file '%s'.  Expected %s", header->identify, fileName, FIB_Identify);
    }
    if (header->version != FIB_Version)
    {
        dbgFatalf(DBG_Loc, "feScreensLoad: Invalid version 0x%x in file '%s'.  Expected 0x%x", header->version, fileName, FIB_Version);
    }
#endif
    screen = (fescreen *)(loadAddress + sizeof(fibfileheader));//pointer to first screen
    for (screenIndex = 0; screenIndex < header->nScreens; screenIndex++, screen++)
    {
#ifdef ENDIAN_BIG
		screen->name   = ( char *)LittleLong( ( udword )screen->name );
		screen->flags  = LittleLong( screen->flags );
		screen->nLinks = LittleShort( screen->nLinks );
		screen->nAtoms = LittleShort( screen->nAtoms );
		screen->links  = ( void *)LittleLong( ( udword )screen->links );
		screen->atoms  = ( void *)LittleLong( ( udword )screen->atoms );
#endif

        dbgAssert(screen->name != NULL);                    //screens need a name
        screen->name += (udword)loadAddress;                //fix up load address

        if (((udword)((ubyte *)screen->links) & 0x03) != 0)
        {
            dbgMessagef("\nWARNING links %s not byte aligned",fileName);
        }
        if (((udword)((ubyte *)screen->atoms) & 0x03) != 0)
        {
            dbgMessagef("\nWARNING atoms %s not byte aligned",fileName);
        }
        (ubyte *)screen->links += (udword)loadAddress;      //update list pointers
        (ubyte *)screen->atoms += (udword)loadAddress;
        for (index = 0; index < screen->nLinks; index++)
        {                                                   //for each link in screen

#ifdef ENDIAN_BIG
			screen->links[index].name       = ( char *)LittleLong( ( udword )screen->links[index].name );
			screen->links[index].flags      = LittleLong( screen->links[index].flags );
			screen->links[index].linkToName = ( char *)LittleLong( ( udword )screen->links[index].linkToName );
#endif

            if (bitTest(screen->links[index].flags, FL_Enabled))
            {                                               //if link enabled
                dbgAssert(screen->links[index].name);
                screen->links[index].name += (udword)loadAddress;//fix up the link name
                dbgAssert(screen->links[index].linkToName);
                screen->links[index].linkToName += (udword)loadAddress;//fix up the link name
            }
        }
        //see if there are any menu items present in the screen so we can
        //know if we should reposition the screen for high-rez
        menuItemsPresent = FALSE;
        for (index = 0; index < screen->nAtoms; index++)
        {                                                   //for each atom in screen
#ifdef ENDIAN_BIG
			screen->atoms[index].name = ( char *)LittleLong( ( udword )screen->atoms[index].name );
			screen->atoms[index].flags = LittleLong( screen->atoms[index].flags );
			screen->atoms[index].status = LittleLong( screen->atoms[index].status );
			screen->atoms[index].tabstop = LittleShort( screen->atoms[index].tabstop );
//			screen->atoms[index].borderColor = LittleLong( screen->atoms[index].borderColor );
//			screen->atoms[index].contentColor = LittleLong( screen->atoms[index].contentColor );
			screen->atoms[index].x = LittleShort( screen->atoms[index].x );
			screen->atoms[index].loadedX = LittleShort( screen->atoms[index].loadedX );
			screen->atoms[index].y = LittleShort( screen->atoms[index].y );
			screen->atoms[index].loadedY = LittleShort( screen->atoms[index].loadedY );
			screen->atoms[index].width = LittleShort( screen->atoms[index].width );
			screen->atoms[index].loadedWidth = LittleShort( screen->atoms[index].loadedWidth );
			screen->atoms[index].height = LittleShort( screen->atoms[index].height );
			screen->atoms[index].loadedHeight = LittleShort( screen->atoms[index].loadedHeight );
			screen->atoms[index].pData = ( ubyte *)LittleLong( ( udword )screen->atoms[index].pData );
			screen->atoms[index].attribs = ( ubyte *)LittleLong( ( udword )screen->atoms[index].attribs );
			screen->atoms[index].drawstyle[0] = LittleLong( screen->atoms[index].drawstyle[0] );
			screen->atoms[index].drawstyle[1] = LittleLong( screen->atoms[index].drawstyle[1] );
			screen->atoms[index].region = ( void *)LittleLong( ( udword )screen->atoms[index].region );
			screen->atoms[index].pad[0] = LittleLong( screen->atoms[index].pad[0] );
			screen->atoms[index].pad[1] = LittleLong( screen->atoms[index].pad[1] );
#endif

            if (screen->atoms[index].type == FA_MenuItem)
            {
                menuItemsPresent = TRUE;
            }
        }
        if (strcmp(screen->name, "HyperspaceRollCall") == 0)
        {
            menuItemsPresent = TRUE;
        }
        for (index = 0; index < screen->nAtoms; index++)
        {                                                   //for each atom in screen
            screen->atoms[index].region = NULL;
            //convert 2-point rectangle to a 1 point/width/height
            dbgAssert(screen->atoms[index].width  > screen->atoms[index].x);
            dbgAssert(screen->atoms[index].height > screen->atoms[index].y);
            if (screen->atoms[index].type == FA_ListWindow)
            {
                screen->atoms[index].width -= screen->atoms[index].x + LW_BarWidth + LW_WindowXBarSpace;
            }
            else
            {
                screen->atoms[index].width -= screen->atoms[index].x;
            }
            screen->atoms[index].height -= screen->atoms[index].y;

            screen->atoms[index].loadedX = screen->atoms[index].x;
            screen->atoms[index].loadedY = screen->atoms[index].y;
            screen->atoms[index].loadedWidth  = screen->atoms[index].width;
            screen->atoms[index].loadedHeight = screen->atoms[index].height;

            if (!menuItemsPresent)
            {
                if (bitTest(screen->atoms[index].flags, FAF_Background))
                {
                    feResRescaleBackground(&screen->atoms[index]);
                }
                else
                {
                    if (!feAtomOnScreen(&screen->atoms[index]))
                    {
                        bitSet(screen->atoms[index].flags, FAF_Hidden);
                    }

                    screen->atoms[index].x = feResRepositionX(screen->atoms[index].x);
                    screen->atoms[index].y = feResRepositionY(screen->atoms[index].y);
                }
            }

            if (screen->atoms[index].name != NULL)          //if name string present
            {
                screen->atoms[index].name += (udword)loadAddress;
            }
            if (screen->atoms[index].pData != NULL)
            {                                               //fix up data pointer
                if (screen->atoms[index].type != FA_RadioButton)
                {                                           //special case: don't fix-up radio button pointers
                    screen->atoms[index].pData += (udword)loadAddress;
                }
                if (screen->atoms[index].flags & FAF_Bitmap)
                {                                           //if this is a bitmap
                    /* _asm nop */
                    //...code to load in the bitmap and set new pointer
                }
                else if (screen->atoms[index].type != FA_RadioButton)
                {                                           //else it must be a text region
                    screen->atoms[index].type = FA_StaticText;//make it a text region
                }
            }
            if (screen->atoms[index].attribs != NULL)
            {                                               //if non-NULL attribs member
                if (screen->atoms[index].type == FA_StaticText)
                {                                           //if static text
                    screen->atoms[index].attribs = (ubyte *)//load in the font
                        frFontRegister((char *)(screen->atoms[index].attribs + (udword)loadAddress));
                    dbgAssert(screen->atoms[index].attribs != NULL);
                }
                if (screen->atoms[index].type == FA_BitmapButton)
                {
                    screen->atoms[index].attribs += (udword)loadAddress;
                    dbgAssert(screen->atoms[index].attribs != NULL);
                }
            }
        }
        feScreenEntryAdd(screen);
    }

//    return(header->nScreens);
    return(header);
}

/*-----------------------------------------------------------------------------
    Name        : feCallbackAdd
    Description : Add a front end callback function
    Inputs      : controlName - FEMan assigned name for control.  Must be static
                    data.
                  function - function for named string to invoke.
    Outputs     : adds the name and
    Return      : new number of callbacks
----------------------------------------------------------------------------*/
sdword feCallbackAdd(char *controlName, fefunction function)
{
    sdword index;

    //see if this callback already registered
    for (index = 0; index < feCallbackIndex; index++)
    {
        if (feNamesEqual(controlName, feCallback[index].name))
        {
            feCallback[index].function = function;
            return(feCallbackIndex);
        }
    }
    dbgAssert(feCallbackIndex < feNumberCallbacks);
    feCallback[feCallbackIndex].name = controlName;
    feCallback[feCallbackIndex].function = function;
    feCallbackIndex++;
    return(feCallbackIndex);
}

/*-----------------------------------------------------------------------------
    Name        : feCallbackAddMultiple
    Description : Add several callbacks to global callback table
    Inputs      : table - table of entries to add
    Outputs     : Adds whole list to feCallback list
    Return      : new number of callbacks
    Note        : Last entry in table must have a NULL function pointer
                    to terminate the list.
----------------------------------------------------------------------------*/
sdword feCallbackAddMultiple(fecallback *table)
{
    sdword index;

    for (index = 0; table[index].function != NULL; index++)
    {
        feCallbackAdd(table[index].name, table[index].function);
    }
    return(feCallbackIndex);
}

/*-----------------------------------------------------------------------------
    Name        : feDrawCallbackAdd
    Description : Add a front end callback function
    Inputs      : controlName - FEMan assigned name for control.  Must be static
                    data.
                  function - function for named string to invoke for drawing.
    Outputs     : adds the name and
    Return      : new number of callbacks
----------------------------------------------------------------------------*/
sdword feDrawCallbackAdd(char *controlName, fedrawfunction function)
{
    sdword index;

    //see if it's already been registered
    for (index = 0; index < feDrawCallbackIndex; index++)
    {
        if (feNamesEqual(controlName, feDrawCallback[index].name))
        {
            feDrawCallback[index].function = function;
            return(feDrawCallbackIndex);
        }
    }
    dbgAssert(feDrawCallbackIndex < feNumberDrawCallbacks);
    feDrawCallback[feDrawCallbackIndex].name = controlName;
    feDrawCallback[feDrawCallbackIndex].function = function;
    feDrawCallbackIndex++;
    return(feDrawCallbackIndex);
}

/*-----------------------------------------------------------------------------
    Name        : feDrawCallbackAddMultiple
    Description : Add several draw callbacks to global draw callback table
    Inputs      : table - table of entries to add
    Outputs     : Adds whole list to feDrawCallback list
    Return      : new number of callbacks
    Note        : Last entry in table must have a NULL function pointer
                    to terminate the list.
----------------------------------------------------------------------------*/
sdword feDrawCallbackAddMultiple(fedrawcallback *table)
{
    sdword index;

    for (index = 0; table[index].function != NULL; index++)
    {
        feDrawCallbackAdd(table[index].name, table[index].function);
    }
    return(feDrawCallbackIndex);
}

/*-----------------------------------------------------------------------------
    Name        : feAllScreensReposition
    Description : ...
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool feAllScreensReposition(void)
{
    sdword index, a;
    fescreen* screen;
    bool hasMenus;
    featom* atom;

    for (index = 0; index < feScreenIndex; index++)
    {
        screen = feScreen[index];

        hasMenus = FALSE;
        for (a = 0; a < screen->nAtoms; a++)
        {
            if (screen->atoms[a].type == FA_MenuItem)
            {
                hasMenus = TRUE;
                break;
            }
        }
        if (strcmp(screen->name, "HyperspaceRollCall") == 0)
        {
            hasMenus = TRUE;
        }

        if (hasMenus)
        {
            continue;
        }

        for (a = 0; a < screen->nAtoms; a++)
        {
            atom = &screen->atoms[a];

            if (atom->x != atom->loadedX ||
                atom->y != atom->loadedY ||
                atom->width  != atom->loadedWidth ||
                atom->height != atom->loadedHeight)
            {
                atom->x = atom->loadedX;
                atom->y = atom->loadedY;
                atom->width  = atom->loadedWidth;
                atom->height = atom->loadedHeight;
            }

            if (bitTest(atom->flags, FAF_Background))
            {
                feResRescaleBackground(atom);
            }
            else
            {
                if (!feAtomOnScreen(atom))
                {
                    bitSet(atom->flags, FAF_Hidden);
                }

                atom->x = feResRepositionX(atom->x);
                atom->y = feResRepositionY(atom->y);
            }
#if 0
            if (atom->region != NULL)
            {
                regionhandle region = (regionhandle)atom->region;
                region->rect.x0 = atom->x;
                region->rect.y0 = atom->y;
                region->rect.x1 = atom->x + atom->width;
                region->rect.y1 = atom->y + atom->height;
            }
#endif
        }
    }

    while (feStackIndex)
    {
        feScreenDisappear(NULL, NULL);
    }
    if (!gameIsRunning)
    {
        feScreenStart(ghMainRegion, "Main_game_screen");
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : feScreenEntryAdd
    Description : Add a front end screen entry point.
    Inputs      : screen - screen to add
    Outputs     : Adds screen to feScreen list
    Return      : new number of screens
----------------------------------------------------------------------------*/
sdword feScreenEntryAdd(fescreen *screen)
{
    sdword index;

    //see if screen already added
    for (index = 0; index < feScreenIndex; index++)
    {
        if (feScreen[index] == screen)
        {
            return(feScreenIndex);
        }
    }
    dbgAssert(feScreenIndex < feNumberScreens);
    feScreen[feScreenIndex] = screen;
    feScreenIndex++;
    return(feScreenIndex);
}

/*-----------------------------------------------------------------------------
    Name        : feScreenEntryRemove
    Description : Add a front end screen entry point.
    Inputs      : screen - screen to remove
    Outputs     : Adds screen to feScreen list
    Return      : new number of screens
----------------------------------------------------------------------------*/
sdword feScreenEntryRemove(fescreen *screen)
{
    sdword index;
    for (index = 0; index < feScreenIndex; index++)
    {
        if (feScreen[index] == screen)
        {                                                   //if found the screen
//            memFree(screen);
            for (index++; index < feScreenIndex; index++)
            {                                               //move the rest of the list back
                feScreen[index - 1] = feScreen[index];
            }
            feScreenIndex--;
            return(feScreenIndex);
        }
    }
#if FEF_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "\nfeScreenEntryremove: cannot remove screen 0x%x", screen);
#endif

    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : feScreensDelete
    Description : Delete a set of screens associated with a .fib file
    Inputs      : screens - pointer to file header
    Outputs     : deletes all entries from the screen entry list and frees the file.
    Return      : ???
----------------------------------------------------------------------------*/
sdword feScreensDelete(fibfileheader *screens)
{
    fescreen *screen;
    sdword screenIndex;

    screen = (fescreen *)((ubyte *)screens + sizeof(fibfileheader));//pointer to first screen
    for (screenIndex = 0; screenIndex < screens->nScreens; screenIndex++, screen++)
    {
        feScreenEntryRemove(screen);                        //free each screen entry
    }
    memFree(screens);
    return(0);
}

// supremem hack of all time, ala drew
extern regionhandle regClickedLeft;
extern regionhandle regClickedLeftLast;  //for making double-clicks work properly
extern regionhandle regClickedRight;
extern regionhandle regClickedCentre;

/*-----------------------------------------------------------------------------
    Name        : feScreenStart
    Description : Execute a named screen.
    Inputs      : screenName - name of screen to start
                  parent - handle of region to be a parent of this region
    Outputs     : ..
    Return      : regionhandle of the base region
    Note        : Because this screen can link to other screens, be careful you
                    don't try to delete regions which do not exist.
----------------------------------------------------------------------------*/
regionhandle feScreenStart(regionhandle parent, char *screenName)
{
    regionhandle baseRegion, temp;
    fescreen *newScreen;
    udword numButtons = 0;
    udword i;

    while (feMenuLevel)
    {                                           //after function executed, delete all menus in this stack
        feMenuDisappear(NULL, NULL);
    }

    feTabStop = 1;
    newScreen = feScreenFind(screenName);                   //find named screen

    regKeysFocussed = FALSE;                                //enable keys if someone was typing on previous screen
    keyClearAllStuckKeys();

//LMCH: the code for FAF_AlwaysOnTop doesn't work, so I'm disabling this check
//    if (!bitTest(newScreen->atoms[0].flags, FAF_AlwaysOnTop))
//    {                                                       //if it's an 'always on top' screen
        if (!bitTest(newScreen->atoms[0].flags, FAF_Popup))
        {                                                   //if this is a pop-up screen
            if (feStack[feStackIndex].parentRegion == parent)
            {                                               //of the previous screen on stack
                feCurrentScreenDelete();                    //kill the previous screen
            }
        }
        else
        {
            parent = feStack[feStackIndex].baseRegion;      //new parent: previous screen
        }

        if (feStack[feStackIndex].screen != NULL)
        {
            if (feNamesEqual(screenName, feStack[feStackIndex].screen->name))
            {   //if linking to screen already on stack (as in a 'back' from a popup)
                return(feStack[feStackIndex].baseRegion);
            }
        }
        feScreenPush();                                     //allocate a new screen from stack
        feStack[feStackIndex].parentRegion = parent;
        feStack[feStackIndex].screen = newScreen;           //init new stack entry
        feStack[feStackIndex].baseRegion = baseRegion =
            feRegionsAdd(parent, newScreen, FALSE);
//    }
//    else
//    {
//        baseRegion = feRegionsAdd(parent, newScreen, TRUE);
//    }

    //check all the buttons/regions and clear the MouseInside flag when a popup screen gets loaded
    if (feStackIndex > 0)
    {
        for (temp = parent->child; temp != NULL; temp = temp->next)
        {
            bitClear(temp->status, RSF_MouseInside);
        }
    }

    // count the total number of buttons on a screen
    for (i = 0; i < newScreen->nAtoms; i++)
    {
        if (newScreen->atoms[i].tabstop)
        {
            if (newScreen->atoms[i].type == FA_RadioButton)
            {
                if ((sdword)newScreen->atoms[i].pData == 1)
                {
                    numButtons++;
                }
            }
            else
                numButtons++;
        }

        // reset the status bits for this each atom in the screen
        newScreen->atoms[i].status = 0;
    }

    feAllCallOnCreate(newScreen);                           //call all regions with the FAF_CallOnCreate flag set

    // adds keyboard control by defining a region for the key
    if (numButtons)
    {
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicTabProcess, 1, TABKEY);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicRightArrowProcess, 1, ARRRIGHT);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicLeftArrowProcess, 1, ARRLEFT);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicUpArrowProcess, 1, ARRUP);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicDownArrowProcess, 1, ARRDOWN);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicSpacebarProcess, 1, SPACEKEY);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicReturnProcess, 1, RETURNKEY);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicEscProcess, 1, ESCKEY);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicHomeProcess, 1, HOMEKEY);
        regKeyChildAlloc(baseRegion, numButtons, RPE_KeyDown, uicEndProcess, 1, ENDKEY);
    }

    mouseCursorDelayShow(1);

    regClickedLeft = NULL;
    regClickedLeftLast = NULL;  //for making double-clicks work properly
    regClickedRight = NULL;
    regClickedCentre = NULL;

    return(baseRegion);
}

/*-----------------------------------------------------------------------------
    Name        : feMenuItemDraw
    Description : Render callback for menu items
    Inputs      : region - region we're rendering
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void feMenuItemDraw(regionhandle region)
{
    featom *atom = (featom *)region->userID;

#if FEF_ERROR_CHECKING
    dbgAssert(atom->type == FA_MenuItem);                   //verify correct type
#endif
    if (bitTest(region->status, RSF_MouseInside))
    {                                                       //if mouse inside this menu item
//        if (region->parent == feStack[feStackIndex].baseRegion)
//        {                                                   //only draw menu items from top menu
            ferDrawMenuItemSelected(&region->rect);
//        }
    }
    if (FECHECKED(atom))
    {                                                       //if this is a selected menu item
        ferDrawSelectedDot(&region->rect);
    }
    if (bitTest(atom->flags, FAF_Link))
    {
        ferDrawPopoutArrow(&region->rect);
    }
}

/*-----------------------------------------------------------------------------
    Name        : feDividerDraw
    Description : Draw a divider in a menu.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void feDividerDraw(regionhandle region)
{
    featom *atom = (featom *)region->userID;
    sdword y;

#if FEF_ERROR_CHECKING
    dbgAssert(atom->type == FA_Divider);                    //verify correct type
#endif
    y = (region->rect.y0 + region->rect.y1) / 2;
    primLine2(region->rect.x0, y, region->rect.x1, y, atom->borderColor);
}

/*-----------------------------------------------------------------------------
    Name        : feBaseRegionProcess
    Description : Region processor callback for base region of menu items.
    Inputs      : region - region to handle processing
                  ID - user-assigned ID set when region created
                  event - enumeration of event to be processed
                  data - additional event data (message specific)
    Outputs     : ...user defined...
    Return      : flags indicating further operation:

----------------------------------------------------------------------------*/
udword feBaseRegionProcess(regionhandle region, sdword ID, udword event, udword data)
{
    /*
    if (event == RPE_Exit)
    {
        //exiting the menu, kill menu if not the base menu
        if (region->parent == feStack[feStackIndex].baseRegion)
        {                                                   //if top menu
            if (feStackIndex > 0 && feStack[feStackIndex].baseRegion->processFunction == feBaseRegionProcess)
            {                                               //if 1 screen up is also a menu
                feCurrentScreenDelete();
            }
        }
    }
    */
    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : mrBottomMostAtomRegion
    Description : Return the region which is the last child of a base region.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
regionhandle mrBottomMostAtomRegion(regionhandle baseRegion)
{
    baseRegion = baseRegion->child;

    dbgAssert(baseRegion != NULL);
    while (baseRegion->next != NULL)
    {
        baseRegion = baseRegion->next;
    }
    return(baseRegion);
}

/*-----------------------------------------------------------------------------
    Name        : feMenuItemProcess
    Description : Region processor callback for menu items
    Inputs      : region - region to handle processing
                  ID - user-assigned ID set when region created
                  event - enumeration of event to be processed
                  data - additional event data (message specific)
    Outputs     : ...user defined...
    Return      : flags indicating further operation:

----------------------------------------------------------------------------*/
udword feMenuItemProcess(regionhandle region, sdword ID, udword event, udword data)
{
    featom *atom = (featom *)ID;
    fescreen *screentodraw;
    felink *link;
    regionhandle baseRegion;
    char *name;

    dbgAssert(atom->type == FA_MenuItem);

#if FEF_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfeMenuItemProcess: menu item '%s' selected.", atom->name ? atom->name : "NULL");
#endif

    if (atom->flags & FAF_Link)
    {
        if (event == RPE_Enter || event == RPE_PressLeft)
        {                                                   //if entering a menu item
            if (region->parent != feStack[feStackIndex].baseRegion)
            {                                                   //if it's not the top screen
                while (region->parent != feStack[feStackIndex].baseRegion)
                {                                           //clear down to this menu screen
                    baseRegion = mrBottomMostAtomRegion(feStack[feStackIndex].baseRegion);
                    if (bitTest(baseRegion->status, RSF_MouseInside))
                    {
                        break;
                    }
                    feMenuDisappear(NULL, NULL);
                }
            }
            if (region->parent == feStack[feStackIndex].baseRegion)
            {                                                   //if it's not the top screen
#if FEF_VERBOSE_LEVEL >= 2
                dbgMessagef("\nfeMenuItemProcess: link to '%s' hit.", atom->name ? atom->name : "NULL");
#endif
#if FEF_ERROR_CHECKING
                if (atom->name == NULL)
                {
                    dbgMessage("\nfeMenuItemProcess: NULL link menu item selected.");
                    return(0);
                }
#endif
                link = feLinkFindInScreen(feStack[feStackIndex].screen, atom->name);//find named link for menu
#if FEF_ERROR_CHECKING
                if (link == NULL)
                {
                    dbgMessagef("\nfeMenuItemProcess: no link found in menu '%s' for link menu item '%s'", feStack[feStackIndex].screen->name, atom->name);
                    return(0);
                }
#endif
                dbgAssert(link->linkToName != NULL);        //make sure we can link
                screentodraw = feScreenFind(link->linkToName);
                if (screentodraw->atoms->flags&FAF_Popup)
                {
                    if (region->rect.x1 + feMenuOverlapMarginX + screentodraw->atoms->width > MAIN_WindowWidth)
                    {
                        feMenuStart(feStack[feStackIndex].parentRegion, screentodraw,
                                    region->rect.x0 - feMenuOverlapMarginX - screentodraw->atoms->width,
                                    region->rect.y0 + feMenuOverlapMarginY);//start new menu
                    }
                    else
                    {
                        feMenuStart(feStack[feStackIndex].parentRegion, screentodraw,
                                    region->rect.x1 + feMenuOverlapMarginX,
                                    region->rect.y0 + feMenuOverlapMarginY);//start new menu
                    }
                    //soundEvent(NULL, UI_Click);
                }
                else
                {
                    feMenuStart(feStack[feStackIndex].parentRegion, screentodraw,
                                region->rect.x1 + feMenuOverlapMarginX,
                                region->rect.y0 + feMenuOverlapMarginY);//start new menu

                    soundEvent(NULL, UI_Click);
                }
            }
        }
    }
    else if (atom->flags & FAF_Function)
    {
        if (region->parent == feStack[feStackIndex].baseRegion)
        {                                                   //if it's the top screen
            if (event == RPE_ReleaseLeft)
            {
                dbgAssert(atom->name != NULL);
#if FEF_VERBOSE_LEVEL >= 2
                dbgMessagef("\nfeMenuItemProcess: menu item '%s' selected.", atom->name ? atom->name : "NULL");
#endif
                name = atom->name;
                feFunctionExecute(name, atom, FALSE);

                soundEvent(NULL, UI_ClickAccept);

                if (!mrMenuDontDisappear)
                {
                    while (feMenuLevel)
                    {                                           //after function executed, delete all menus in this stack
                        feMenuDisappear(NULL, NULL);
                    }
                }
                else
                {
                    mrMenuDontDisappear = FALSE;
                }
            }
        }
        if (region->parent != feStack[feStackIndex].baseRegion)
        {
            if (event == RPE_Enter)
            {                                               //if entering a lower menu item
                while (region->parent != feStack[feStackIndex].baseRegion)
                {                                           //clear down to this menu screen
                    baseRegion = mrBottomMostAtomRegion(feStack[feStackIndex].baseRegion);

                    if (bitTest(baseRegion->status, RSF_MouseInside))
                    {
                        break;
                    }
                    feMenuDisappear(NULL, NULL);
                }
            }
        }
    }
#if FEF_ERROR_CHECKING
    else
    {
        dbgMessagef("\nfeMenuItemProcess: menu item '%s' does nothing.", atom->name ? atom->name : "NULL");
    }
#endif
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : feMenuBaseRegionProcess
    Description : Process messages for base region of a menu
    Inputs      : generic region handler
    Outputs     : Kills the menu by deleting this region
    Return      : 0
----------------------------------------------------------------------------*/
udword feMenuBaseRegionProcess(regionhandle region, sdword ID, udword event, udword data)
{
    feMenuDisappear(NULL, NULL);                                //kill this menu
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : feMenuDisappear
    Description : Same as previous function, just called in a different manner.
    Inputs      : ..
    Outputs     : Kills the menu by deleting the base region.
    Return      : void
    Note        : This is bound to the FEMan function(Dissapear) which should
                    only ever be used in menus.
----------------------------------------------------------------------------*/
void feMenuDisappear(char *string, featom *atom)
{
    feMenuLevel--;
    feCurrentScreenDelete();                                //kill this menu
    if (feTempMenuScreen != NULL && feMenuLevel == 0)
    {
        if (feTempMenuScreen->atoms != NULL)
        {
            memFree(feTempMenuScreen->atoms);
        }
        if (feTempMenuScreen->links)
        {
            memFree(feTempMenuScreen->links);
        }
        memFree(feTempMenuScreen);
        feTempMenuScreen = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : feScreenDisappear
    Description : Same as previous function, except not menu-specific.
    Inputs      : ..
    Outputs     : Kills the screen by deleting the base region.
    Return      : void
    Note        : This is bound to the FEMan function(Dissapear) which should
                    only ever be used in screens.
----------------------------------------------------------------------------*/
void feScreenDisappear(char *string, featom *atom)
{
    feCurrentScreenDelete();                                //kill this screen
}

/*-----------------------------------------------------------------------------
    Name        : feMenuRegionsAdd
    Description : Adds menu items for specified menu
    Inputs      : screen - pointer to the screen whose atoms we are to add for menu
                  parent - region to make this a child of
                  x, y - screen location to add to each atom
    Outputs     : Calls uicChildButtonAlloc or regChildAlloc for each screen
                    atom and regChildAlloc for a base 'dummy' region.
    Return      : Pointer to base 'dummy' region
----------------------------------------------------------------------------*/
regionhandle feMenuRegionsAdd(regionhandle parent, fescreen *screen, sdword x, sdword y)
{
    sdword index;
    regionhandle baseRegion, region = NULL;
    featom *atom;
    buttonhandle button;

    //for menus, the base region will be full screen, to allow the user to kill menu
    //by right or left-clicking outside the menus
    //make the base region have alpha
    bitSet(screen->atoms[0].status, FAS_UseAlpha);

    dbgAssert(screen->nAtoms >= 1);                         //make sure there is at least a blank menu
    baseRegion = regChildAlloc(parent, 0, 0, 0,             //create base dummy region
                               MAIN_WindowWidth, MAIN_WindowHeight, 0, RPE_PressLeft | RPE_PressRight);
    regSiblingMoveToFront(baseRegion);
    regFunctionSet(baseRegion, feBaseRegionProcess);
    regFunctionSet(baseRegion, feMenuBaseRegionProcess);

    atom = &screen->atoms[0];                               //pointer to base atom
    baseRegion->drawstyle[0] = atom->drawstyle[0];
    baseRegion->drawstyle[1] = atom->drawstyle[1];

    if (x + atom->x - feMenuScreenMarginX < 0)
    {
        x -= x + atom->x;
    }
    if (y + atom->y - feMenuScreenMarginY < 0)
    {
        y -= y + atom->y;
    }
    if (x + atom->x + atom->width + feMenuScreenMarginX > MAIN_WindowWidth)
    {
        x -= x + atom->x + atom->width + feMenuScreenMarginX - MAIN_WindowWidth;
    }
    if (y + atom->y + atom->height + feMenuScreenMarginY > MAIN_WindowHeight)
    {
        y -= y + atom->y + atom->height + feMenuScreenMarginY - MAIN_WindowHeight;
    }
    atom = &screen->atoms[screen->nAtoms - 1];

    for (index = 0; index < screen->nAtoms; index++, atom--)
    {
        atom->region = NULL;                            //assume no region for this atom
        switch (atom->type)
        {
            case FA_MenuItem:                               //menu item
                region = regChildAlloc(baseRegion, (sdword)atom,
                            atom->x + x, atom->y + y, atom->width, atom->height, 0, FE_MenuFlags);
                regDrawFunctionSet(region, feMenuItemDraw);
                regFunctionSet(region, feMenuItemProcess);
                break;
            case FA_StaticText:                             //static text message
                region = regChildAlloc(baseRegion, (sdword)atom, atom->x + x, atom->y + y,
                               atom->width, atom->height, 0, 0);
                regDrawFunctionSet(region, feStaticTextDraw);
                break;
            case FA_Button:                                 //button
            case FA_ToggleButton:
            case FA_CheckBox:
                button = uicChildButtonAlloc(baseRegion, (sdword)atom, atom->x + x, atom->y + y,
                               atom->width, atom->height, feButtonProcess, atom->type | CM_ButtonClick);
                feAcceleratorSet(&button->reg, atom);
/*
                for (nKeys = 0; nKeys < 4 && atom->accelerator[nKeys] != 0; nKeys++)
                {                                           //count number of keys
                    ;
                }
                if (nKeys)
                {
                    regFilterSet(&button->reg, regFilterGet(&button->reg) | RPE_KeyDown);
                    regKeysSet(&button->reg, nKeys, atom->accelerator[0],
                               atom->accelerator[1], atom->accelerator[2], atom->accelerator[3]);
                }
*/
                uicContentColor(button, atom->contentColor);//set button colors
                uicBorderColor(button, atom->borderColor);
                break;
            case FA_Divider:
                region = regChildAlloc(baseRegion, (sdword)atom,
                            atom->x + x, atom->y + y, atom->width, atom->height, 0, 0);
                regDrawFunctionSet(region, feDividerDraw);
                break;
            default:                                        //anything else is a plain rectangle
                region = regChildAlloc(baseRegion, (sdword)atom, atom->x + x, atom->y + y,
                               atom->width, atom->height, 0, 0);
                regDrawFunctionSet(region, feStaticRectangleDraw);
                break;
        }
        if (region != NULL)
        {
            region->drawstyle[0] = atom->drawstyle[0];
            region->drawstyle[1] = atom->drawstyle[1];
        }
    }
    dbgAssert(region != NULL);                              //base region
    regFunctionSet(region, feBaseRegionProcess);
    regFilterSet(region, regFilterGet(region) | RPE_Exit);

    return(baseRegion);
}

/*-----------------------------------------------------------------------------
    Name        : feAcceleratorSet
    Description : Set the hotkeys for a region from the attributes in it's atom
    Inputs      : reg - region
                  atom - atom where hotkey info can be found
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void feAcceleratorSet(regionhandle reg, featom *atom)
{
    sdword nKeys = 0;
    sword keys[4] = {0,0,0,0};

    if (bitTest(atom->hotKeyModifiers, HKM_Control))
    {
        keys[nKeys] = CONTROLKEY;
        nKeys++;
    }
    if (bitTest(atom->hotKeyModifiers, HKM_Shift))
    {
        keys[nKeys] = SHIFTKEY;
        nKeys++;
    }
    if (bitTest(atom->hotKeyModifiers, HKM_Alt))
    {
        keys[nKeys] = ALTKEY;
        nKeys++;
    }
    if (atom->hotKey[strCurLanguage])
    {
        keys[nKeys] = atom->hotKey[strCurLanguage];
        nKeys++;
    }
    if (nKeys)
    {
        regFilterSet(reg, regFilterGet(reg) | RPE_KeyDown);
        regKeysSet(reg, nKeys, keys[0], keys[1], keys[2], keys[3]);
    }
    else
    {
        regFilterSet(reg, regFilterGet(reg) & (~RPE_KeyDown));
    }
}

/*-----------------------------------------------------------------------------
    Name        : feScreenAllHotKeysUpdate
    Description : Update the hotkeys for all all atoms in a screen.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void feScreenAllHotKeysUpdate(fescreen *screen)
{
    sdword index;

    for (index = 1; index < screen->nAtoms; index++)
    {
        if (screen->atoms[index].region != NULL)
        {
            feAcceleratorSet((regionhandle)screen->atoms[index].region, &screen->atoms[index]);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : feMenuStart
    Description : Start a menu at a specified location.
    Inputs      : menuName - name of menu to start
                  parent - handle of region to be a parent of this region
                  x, y - amount added to all atom locations
    Outputs     : ..
    Return      : regionhandle of the base region
    Note        : Because this screen can link to other screens, be careful you
                    don't try to delete regions which do not exist.
----------------------------------------------------------------------------*/
regionhandle feMenuStart(regionhandle parent, fescreen *screen, sdword x, sdword y)
{
    regionhandle baseRegion;
//    fescreen *newScreen;

#if FEF_ERROR_CHECKING
    dbgAssert(screen != NULL);
    if (feStack[feStackIndex].screen != NULL)
    {
        if (feNamesEqual(screen->name, feStack[feStackIndex].screen->name))
        {
            return(feStack[feStackIndex].baseRegion);
        }
    }
#endif
//    newScreen = feScreenFind(screenName);                   //find named screen

    if (!bitTest(screen->atoms[0].flags, FAF_Popup))
    {                                                       //if this is a pop-up screen
        if (feStack[feStackIndex].parentRegion == parent)
        {                                                   //of the previous screen on stack
            feCurrentScreenDelete();                        //kill the previous screen
        }
    }
    else
    {
        parent = feStack[feStackIndex].baseRegion;          //new parent: previous screen
    }

    if (feStack[feStackIndex].screen != NULL)
    {
        if (feNamesEqual(screen->name, feStack[feStackIndex].screen->name))
        {   //if linking to screen already on stack (as in a 'back' from a popup)
            return(feStack[feStackIndex].baseRegion);
        }
    }
    feScreenPush();                                         //allocate a new screen from stack
    feStack[feStackIndex].parentRegion = parent;
    feStack[feStackIndex].screen = screen;                  //init new stack entry
    feStack[feStackIndex].baseRegion = baseRegion =
        feMenuRegionsAdd(parent, screen, x, y);             //actually add the regions

    feMenuLevel++;
    return(baseRegion);
}

/*-----------------------------------------------------------------------------
    Name        : feScreenFind
    Description : Find a named screen
    Inputs      : name - name of screen to find
    Outputs     : ..
    Return      : pointer to named screen
    Note        : Generates an error if screen not found
----------------------------------------------------------------------------*/
fescreen *feScreenFind(char *name)
{
    sdword index;

    for (index = 0; index < feScreenIndex; index++)
    {
        if (feNamesEqual(name, feScreen[index]->name))
        {
            return(feScreen[index]);
        }
    }
    dbgFatalf(DBG_Loc, "feScreenFind: can't find screen '%s'", name);
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : feCurrentScreenDelete
    Description : Delete the current front-end screen
    Inputs      : void
    Outputs     : Deletes the feStack[feStackIndex].baseRegion and all children
                    thereof.  Also decrements the stack.
    Return      : void
----------------------------------------------------------------------------*/
void feCurrentScreenDelete(void)
{
    sdword index;
    featom *atom;

    regKeysFocussed = FALSE;                                //enable keys if someone was typing on previous screen

    keyClearAllStuckKeys();

    if(tacticsMenuRegion == feStack[feStackIndex].baseRegion)
    {
        tacticsMenuRegion = NULL;
    }

    if (feStack[feStackIndex].screen == NULL)
    {
        dbgMessage("\n-- NULL screen in feCurrentScreenDelete --");
        return;
    }

    atom = &feStack[feStackIndex].screen->atoms[feStack[feStackIndex].screen->nAtoms - 1];

    for (index = 1; index < feStack[feStackIndex].screen->nAtoms; index++, atom--)
    {
        if (bitTest(atom->flags, FAF_CallOnDelete))
        {
            bitSet(atom->status, FAS_OnDelete);

            if (atom->type==FA_UserRegion)
            {
                feUserRegionDraw((regionhandle)atom->region);
            }
            else
            {
                feFunctionExecute(atom->name, atom, FALSE);
            }

            bitClear(atom->status, FAS_OnDelete);
        }
        if (atom->type==FA_ListWindow)
        {
            uicListCleanUp((listwindowhandle)atom->pData);
        }
        else if (atom->type==FA_TextEntry)
        {
            uicTextEntryCleanUp((textentryhandle)atom->pData);
        }
    }

    regRegionDelete(feStack[feStackIndex].baseRegion);
    feScreenPop();
}

/*-----------------------------------------------------------------------------
    Name        : feScreenDelete
    Description : Explicitly delete a particular screen from the screen stack.
    Inputs      : baseRegion - region returned from feScreenStart
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void feScreenDeleteFlags(regionhandle baseRegion,sdword flags)
{
    sdword index, atomindex;
    featom *atom;

/*
#if FEF_ERROR_CHECKING
    for (index = 0; index <= feStackIndex; index++)
    {                                                       //make sure there is a screen with this as a base region
        if (feStack[index].baseRegion == baseRegion)
        {
            goto foundTheEntry;
        }
    }
    dbgFatalf(DBG_Loc, "Unable to free screen with a baseRegion of 0x%x", baseRegion);
foundTheEntry:;
#endif
*/
    regKeysFocussed = FALSE;                                //enable keys if someone was typing on previous screen

    keyClearAllStuckKeys();

    for (index = 0; index <= feStackIndex; index++)
    {
        if (feStack[index].baseRegion == baseRegion)
        {
            atom = &feStack[index].screen->atoms[feStack[index].screen->nAtoms - 1];

            for (atomindex = 1; atomindex < feStack[index].screen->nAtoms; atomindex++, atom--)
            {
                if (bitTest(atom->flags, FAF_CallOnDelete))
                {
                    bitSet(atom->status, FAS_OnDelete);

                    if (atom->type==FA_UserRegion)
                    {
                        feUserRegionDraw((regionhandle)atom->region);
                    }
                    else
                    {
                        feFunctionExecute(atom->name, atom, FALSE);
                    }

                    bitClear(atom->status, FAS_OnDelete);
                }
                if (atom->type==FA_ListWindow)
                {
                    uicListCleanUp((listwindowhandle)atom->pData);
                }
                else if (atom->type==FA_TextEntry)
                {
                    uicTextEntryCleanUp((textentryhandle)atom->pData);
                }
            }

            regRegionDelete(baseRegion);                    //delete the base region
            for (; index <= feStackIndex - 1; index++)
            {                                               //move the rest of the stack back 1
                feStack[index] = feStack[index + 1];
            }
            feStack[feStackIndex].screen = NULL;            //clear the newly freed entry in stack
            feStack[feStackIndex].baseRegion =
                feStack[feStackIndex].parentRegion = NULL;
            feStackIndex--;
            return;
        }
    }
    //else the screen not found, delete just the region

    if (!(flags & FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND))
    {
        regRegionDelete(baseRegion);
    }
}

/*-----------------------------------------------------------------------------
    Name        : feAllScreensDelete
    Description : Delete all current front-end screens
    Inputs      : void
    Outputs     : Deletes the feStack[feStackIndex].baseRegion and all children
                    thereof.  Also decrements the stack.  Repeats until stack empty
    Return      : void
----------------------------------------------------------------------------*/
void feAllScreensDelete(void)
{
    while (feStackIndex > 0)
    {
        //regRegionDelete(feStack[feStackIndex].baseRegion);
        //feScreenPop();
        feCurrentScreenDelete();
    }
}

/*-----------------------------------------------------------------------------
    Name        : feAllMenusDelete
    Description : Closes all active menus.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void feAllMenusDelete(void)
{
    while (feMenuLevel)
    {
        feMenuDisappear(NULL, NULL);
    }
}

/*-----------------------------------------------------------------------------
    Name        : feScreenPush
    Description : Allocate an entry on the FE screen stack.
    Inputs      : void
    Outputs     : ..
    Return      : coid
----------------------------------------------------------------------------*/
fescreen *feScreenPush(void)
{
#if FEF_ERROR_CHECKING
    dbgAssert(feStackIndex < FE_StackDepth);
#endif
    feStackIndex++;                                         //allocate new entry
    feStack[feStackIndex].screen = NULL;                    //clear out the entry
    feStack[feStackIndex].baseRegion = feStack[feStackIndex].parentRegion = NULL;
#if FEF_VERBOSE_LEVEL >= 1
    dbgMessagef("\nfeScreenPush: pushed screen %d", feStackIndex);
#endif
    return(feStack[feStackIndex - 1].screen);
}

/*-----------------------------------------------------------------------------
    Name        : feScreenPop
    Description : Pops the current screen from the screen stack.
    Inputs      : void
    Outputs     : decrements the stack but does not delete regions.
    Return      : pointer of screen just freed
----------------------------------------------------------------------------*/
fescreen *feScreenPop(void)
{
    fescreen *returnValue;
#if FEF_ERROR_CHECKING
    dbgAssert(feStackIndex > 0);                            //can't pop the bottom of stack
#endif
#if FEF_VERBOSE_LEVEL >= 1
    dbgMessagef("\nfeScreenPop: popped screen %d", feStackIndex);
#endif
    returnValue = feStack[feStackIndex].screen;
    feStack[feStackIndex].screen = NULL;
    feStack[feStackIndex].baseRegion =
        feStack[feStackIndex].parentRegion = NULL;
    feStackIndex--;
    if (feStackIndex == 0)
    {
        feDontFlush = TRUE;
    }

    feRenderEverything = TRUE;
    glcRenderEverything();

    return(returnValue);
}

/*-----------------------------------------------------------------------------
    Name        : feRadioButtonSet
    Description : Sets the checked state of a radio button and uncheck all others
                    in current screen.
    Inputs      : name - name of radio button (function call)
                  index - index of radio button in set
    Outputs     :
    Return      : void
    Note        : the radio button can be in any screen and the topmost one will
                    be checked (if it exists in multiple screens).
----------------------------------------------------------------------------*/
void feRadioButtonSet(char *name, sdword index)
{
    featom *atom = NULL;
    buttonhandle button;
    sdword iScreen;

    for (iScreen = feStackIndex; iScreen >= 0; iScreen--)
    {
        atom = feAtomFindInScreen(feStack[iScreen].screen,
                                  name);                    //find first named atom in any given screen
        if (atom != NULL)
        {
            break;
        }
    }
    dbgAssert(atom != NULL);

    do
    {
        button = (buttonhandle)atom->region;                //find the associated region
        if ((sdword)atom->pData == index)
        {
            bitSet(atom->status, FAS_Checked);              //un-check all buttons in series
#ifdef DEBUG_STOMP
        regVerify(&button->reg);
#endif
            bitSet(button->reg.status, RSF_DrawThisFrame);
        }
        else
        {
            bitClear(atom->status, FAS_Checked);            //un-check all buttons in series
#ifdef DEBUG_STOMP
        regVerify(&button->reg);
#endif
            bitSet(button->reg.status, RSF_DrawThisFrame);
        }
    }
    while ((atom = feAtomFindNextInScreen(feStack[iScreen].screen, atom, atom->name)) != NULL);
}

/*-----------------------------------------------------------------------------
    Name        : feToggleButtonSet
    Description : Set toggle state of a toggle button.
    Inputs      : name - name of button (function name that is)
                  bPressed - 0 if not pressed, nonzero is pressed
    Outputs     : sets the checked bit of button
    Return      : void
----------------------------------------------------------------------------*/
void feToggleButtonSet(char *name, sdword bPressed)
{
    featom *atom;
    buttonhandle button;
    sdword iScreen;

    for (iScreen = feStackIndex; iScreen >= 0; iScreen--)
    {
        atom = feAtomFindInScreen(feStack[iScreen].screen,
                                  name);                    //find first named atom in any given screen
        if (atom != NULL)
        {
            break;
        }
    }
    if (atom == NULL)
    {
        return;
    }

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

regionhandle feRegionFindByFunctionAux(char* name, regionhandle baseRegion)
{
    regionhandle reg = baseRegion;

    for (; reg != NULL; reg = reg->next)
    {
        regVerify(reg);
        if (reg->child != NULL)
        {
            regionhandle result = feRegionFindByFunctionAux(name, reg->child);
            if (result != NULL)
            {
                return result;
            }
        }
        if (((featom*)reg->atom)->name != NULL)
        {
            if (feNamesEqual(name, ((featom*)reg->atom)->name))
            {
                return reg;
            }
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : feRegionFindByFunction
    Description : Searches the current entry in the front-end stack for a atom
                    with a certain function name.  When found, it returns the
                    regionhandle which corresponds.  If not found, an error
                    is created (when debugging enabled).
    Inputs      : name - name to search for.
    Outputs     : natin
    Return      : handle of region
----------------------------------------------------------------------------*/
regionhandle feRegionFindByFunction(char *name)
{
    regionhandle reg;

    for (reg = feStack[feStackIndex].baseRegion->child; reg != NULL; reg = reg->next)
    {
        regVerify(reg);
        if (reg->child != NULL)
        {
            regionhandle result = feRegionFindByFunctionAux(name, reg->child);
            if (result != NULL)
            {
                return result;
            }
        }
        if (((featom *)reg->atom)->name != NULL)
        {
            if (feNamesEqual(name, ((featom *)reg->atom)->name))
            {
                return(reg);
            }
        }
    }
#if FEF_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "Cannot find region representing '%s'", name);
#endif
    return(NULL);
}



/*-----------------------------------------------------------------------------
    Name        : feFindRadioButtonRegion
    Description : Searches for the Region associated with the radio button
                  that is or is not currently selected.
    Inputs      : temp - region where search starts
                  selected - whether we're searching for the selectd radio
                  button or the non-selected radio button
    Outputs     : natin
    Return      :
----------------------------------------------------------------------------*/
regionhandle feFindRadioButtonRegion(regionhandle temp, bool selected)
{
    featom *atom;

    atom = (featom *)temp->userID;

    atom = feAtomFindInScreen(feStack[feStackIndex].screen, atom->name);

    //find the radio button that is selected
    if (selected)
    {
        while ((atom != NULL)&&(!FECHECKED(atom)))
        {
            atom = feAtomFindNextInScreen(feStack[feStackIndex].screen, atom, atom->name);
        }
    }
    else
    {
        while ((atom != NULL)&&(FECHECKED(atom)))
        {
            atom = feAtomFindNextInScreen(feStack[feStackIndex].screen, atom, atom->name);
        }
    }

    //if it isn't found, return NULL
    if (atom == NULL)
    {
        return NULL;
    }

/*    //start at the beginning of the region list
    temp = temp->parent->child;

    //now find the region associated with that atom
    while ((temp != NULL)&&((featom *)temp->userID != atom))
    {
        temp = temp->next;
    }*/

    return ((regionhandle)atom->region) ;  //returns NULL if region not found
}

