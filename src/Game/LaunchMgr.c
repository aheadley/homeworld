/*=============================================================================
    Name    : launchMgr.c
    Purpose : Logic for launch manager

    Created 5/18/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#include <stdio.h>
#include "Types.h"
#include "LinkedList.h"
#include "Universe.h"
#include "Region.h"
#include "UIControls.h"
#include "FEFlow.h"
#include "font.h"
#include "FontReg.h"
#include "ObjTypes.h"
#include "Task.h"
#include "mouse.h"
#include "CommandLayer.h"
#include "PiePlate.h"
#include "LaunchMgr.h"
#include "Globals.h"
#include "CommandWrap.h"
#include "Scroller.h"
#include "SoundEvent.h"
#include "Randy.h"
#include "mainrgn.h"
#include "ShipDefs.h"
#include "ShipView.h"
#include "prim2d.h"
#include "TaskBar.h"
#include "texreg.h"
#include "FEReg.h"
#include "render.h"
#include "prim2d.h"
#include "Tutor.h"
#include "FEColour.h"
#include "glcompat.h"
#include "InfoOverlay.h"
#include "StringsOnly.h"

/*=============================================================================
    Defines:
=============================================================================*/

#define NUM_CARRIER_SHIPS       13 + 2
#define LM_FIGHTER_SPACER       TOTAL_NUM_SHIPS + 1
#define LM_CORVETTE_SPACER      TOTAL_NUM_SHIPS + 2
#define LM_SHIP_IMAGE_INSET     4


#define LM_VertSpacing      (fontHeight(" ") >> 1)
#define LM_HorzSpacing      (fontWidth(" ") >> 1)
#define LM_BoxSpacing       2

#define NUM_LMTEXTURES      2

#define NUM_LMCARRIERS      4

/*=============================================================================
    Data::
=============================================================================*/

// hack for Special case Italian crap...
extern udword strCurLanguage;

static sdword lmRenderEverythingCounter;

bool lmActive = FALSE;

extern ShipType svShipType;
bool lmPaletted;

LaunchShipsAvailable shipsavailable[LM_CORVETTE_SPACER+1];

#define LM_NUMSHIPS     21

sdword PrintOrder[LM_NUMSHIPS] =
{
    LM_FIGHTER_SPACER   ,

    LightDefender       ,
    HeavyDefender       ,
    LightInterceptor    ,
    HeavyInterceptor    ,
    CloakedFighter      ,
    DefenseFighter      ,
    AttackBomber        ,
    P1Fighter           ,
    P2Swarmer           ,
    P2AdvanceSwarmer    ,

    LM_CORVETTE_SPACER  ,

    LightCorvette       ,
    HeavyCorvette       ,
    RepairCorvette      ,
    SalCapCorvette      ,
    MinelayerCorvette   ,
    MultiGunCorvette    ,
    P1StandardCorvette  ,
    P1MissileCorvette   ,

    -1
};

// This is a bool that is TRUE if the ship is in the printlist. dynamically calculated
// based on the PrintOrder
bool lmShipInList[TOTAL_NUM_SHIPS];


// callback functions for the launch GUI
void lmLaunch(char *string, featom *atom);
void lmAutoLaunchM(char *string, featom *atom);
void lmAutoLaunchC1(char *string, featom *atom);
void lmAutoLaunchC2(char *string, featom *atom);
void lmAutoLaunchC3(char *string, featom *atom);
void lmAutoLaunchC4(char *string, featom *atom);
void lmClose(char *string, featom *atom);
void lmLaunchAll(char *string, featom *atom);
void lmShipsToLaunchDraw(char *string, featom *atom);

void lmFighterUsedDraw(featom *atom, regionhandle region);
void lmCorvetteUsedDraw(featom *atom, regionhandle region);

void lmMotherShipDraw(featom *atom, regionhandle region);
void lmCarrier1Draw(featom *atom, regionhandle region);
void lmCarrier2Draw(featom *atom, regionhandle region);
void lmCarrier3Draw(featom *atom, regionhandle region);
void lmCarrier4Draw(featom *atom, regionhandle region);

fecallback lmCallback[] =
{
    {lmLaunch,                  "LM_Launch"             },
    {lmAutoLaunchM,             "LM_AutoLaunchM"        },
    {lmAutoLaunchC1,            "LM_AutoLaunchC1"       },
    {lmAutoLaunchC2,            "LM_AutoLaunchC2"       },
    {lmAutoLaunchC3,            "LM_AutoLaunchC3"       },
    {lmAutoLaunchC4,            "LM_AutoLaunchC4"       },
    {lmClose,                   "LM_Close"              },
    {lmLaunchAll,               "LM_LaunchAll"          },
    {lmShipsToLaunchDraw,       "LM_ShipsToLaunch"      },
    {NULL,                      NULL                    }
};

fedrawcallback lmDrawCallback[] =
{
    {lmFighterUsedDraw,         "LM_FighterUsed"        },
    {lmCorvetteUsedDraw,        "LM_CorvetteUsed"       },
    {lmMotherShipDraw,          "LM_MotherShipDraw"     },
    {lmCarrier1Draw,            "LM_Carrier1Draw"       },
    {lmCarrier2Draw,            "LM_Carrier2Draw"       },
    {lmCarrier3Draw,            "LM_Carrier3Draw"       },
    {lmCarrier4Draw,            "LM_Carrier4Draw"       },
    {NULL,                      NULL                    }
};


void lmReLaunch(Ship *newship);

//flag indicating loaded status of this screen
fibfileheader *lmScreensHandle = NULL;

regionhandle  lmBaseRegion = NULL;
ShipsInsideMe *shipsin = NULL;
Ship          *launchship = NULL;

listwindowhandle lmListWindow=NULL;

fonthandle lmShipListFont=0;

//colors for printing selections and stuff
color lmShipListTextColor       = LM_ShipListTextColor;
color lmShipSelectedTextColor   = LM_ShipSelectedTextColor;
color lmShip2ListTextColor       = LM_Ship2ListTextColor;
color lmShip2SelectedTextColor   = LM_Ship2SelectedTextColor;
color lmUsedColor               = LM_UsedColor;
color lmAvailableColor          = LM_AvailableColor;

//variables for selection etc...

bool lmIoSaveState;


Ship *lmCarrierX[NUM_LMCARRIERS] = { NULL, NULL, NULL, NULL };


udword lmShipTexture[MAX_RACES][NUM_LMTEXTURES] =
{
    {TR_InvalidHandle, TR_InvalidHandle},
    {TR_InvalidHandle, TR_InvalidHandle}
};

lifheader *lmShipImage[MAX_RACES][NUM_LMTEXTURES] =
{
    { NULL, NULL },
    { NULL, NULL }
};


char *lmShipImagePaths[] =
{
#ifdef _WIN32
    "FEMan\\Construction_Manager\\Build_race1_",
    "FEMan\\Construction_Manager\\Build_race2_"
#else
    "FEMan/Construction_Manager/Build_race1_",
    "FEMan/Construction_Manager/Build_race2_"
#endif
};

char *lmShipIcons[] =
{
    "icon1.lif", // Mothership
    "icon2.lif", // Carrier
};


sdword lmCurrentSelect = 0;

char lmShipListFontName[LM_FontNameLength] = LM_ShipListFont;

// Variable to keep track of the ship displayed in the shipview
ShipType lmcurshipview=DefaultShip;


/*=============================================================================
    Functions:
=============================================================================*/

void lmUpdateShipView(void)
{
    udword index;
    bool   set=TRUE;
    listitemhandle listitem;

    if (lmcurshipview == DefaultShip)
    {
        for (index=0; PrintOrder[index]!=-1;index++)
        {                                                       //for all available ships
            if ((shipsavailable[PrintOrder[index]].nShips > 0) &&
                ((PrintOrder[index]!=LM_FIGHTER_SPACER) && (PrintOrder[index]!=LM_CORVETTE_SPACER)))
            {
                svSelectShip(PrintOrder[index]);
                if (lmListWindow!=NULL)
                {
                    listitem = uicListFindItemByData(lmListWindow, (ubyte *)PrintOrder[index]);
                    uicListWindowSetCurItem(lmListWindow, listitem);
                }
                lmcurshipview = PrintOrder[index];
                break;
            }
        }
    }
    else
    {
        if (shipsavailable[lmcurshipview].nShips > 0)
        {
            svSelectShip(lmcurshipview);
            if (lmListWindow!=NULL)
            {
                listitem = uicListFindItemByData(lmListWindow, (ubyte *)lmcurshipview);
                uicListWindowSetCurItem(lmListWindow, listitem);
            }
            set = FALSE;
        }
        if (set)
        {
            lmcurshipview = DefaultShip;
            for (index=0; PrintOrder[index]!=-1;index++)
            {                                                       //for all available ships
                if ((shipsavailable[PrintOrder[index]].nShips > 0) &&
                    ((PrintOrder[index]!=LM_FIGHTER_SPACER) && (PrintOrder[index]!=LM_CORVETTE_SPACER)))
                {
                    if (lmListWindow!=NULL)
                    {
                        listitem = uicListFindItemByData(lmListWindow, (ubyte *)PrintOrder[index]);
                        uicListWindowSetCurItem(lmListWindow, listitem);
                    }

                    lmcurshipview = PrintOrder[index];
                    break;
                }
            }
            svSelectShip(lmcurshipview);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : lmLaunch
    Description : process launch button press
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmLaunch(char *string, featom*atom)
{
    Node *walk;
    ShipPtr shipinside;
    InsideShip *insideShipStruct;
    sdword countShips = 0;
    sdword i;
    SelectCommand *selection;
    sdword shipnum=-1;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bLaunchLaunch)
        return;

    if (lmListWindow->CurLineSelected!=NULL)
    {
        shipnum = (sdword)lmListWindow->CurLineSelected->data;
    }

    walk = shipsin->insideList.head;
    while (walk !=NULL)
    {
        insideShipStruct = (InsideShip *)listGetStructOfNode(walk);
        shipinside = insideShipStruct->ship;
        dbgAssert(shipinside->objtype == OBJ_ShipType);
        if (shipnum == shipinside->shiptype)
        {
            countShips++;
        }
        walk = walk->next;
    }

    if (countShips > 0)
    {
        selection = memAlloc(sizeofSelectCommand(countShips),"launchships",0);
        selection->numShips = countShips;

        i = 0;
        walk = shipsin->insideList.head;
        while (walk !=NULL)
        {
            insideShipStruct = (InsideShip *)listGetStructOfNode(walk);
            shipinside = insideShipStruct->ship;
            dbgAssert(shipinside->objtype == OBJ_ShipType);
            if (shipnum == shipinside->shiptype)
            {
                selection->ShipPtr[i++] = shipinside;
            }
            walk = walk->next;
        }

        dbgAssert(i == selection->numShips);
        clWrapLaunchMultipleShips(&universe.mainCommandLayer, selection, launchship);
        memFree(selection);

        speechEventFleet(COMM_F_Launch, 0, universe.curPlayerIndex);
    }

/*
    feScreenDelete(lmBaseRegion);
    lmBaseRegion = NULL;

    // play the exit sound
    soundEvent(NULL, UI_ManagerExit);
    //restart the sound of space ambient
    soundEvent(NULL, UI_SoundOfSpace);

    // enable rendering of main game screen
    mrRenderMainScreen = TRUE;
    lmActive = FALSE;

    glcFullscreen(FALSE);

    // enable taskbar popup window
    tbDisable = FALSE;
*/
}


/*-----------------------------------------------------------------------------
    Name        : lmAutoLaunchX
    Description : process autolaunch checkbox
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmAutoLaunchM(char *string, featom *atom)
{
    if (universe.curPlayerPtr->PlayerMothership == NULL)
    {
        bitSet(atom->flags, FAF_Disabled);
        bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(atom->flags, FAF_Disabled);
        bitClear(((region *)atom->region)->status, RSF_RegionDisabled);
    }

    #ifdef DEBUG_STOMP
    regVerify((regionhandle)atom->region);
    #endif
    bitSet( ((regionhandle)atom->region)->status, RSF_DrawThisFrame);

    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(string, !(universe.curPlayerPtr->autoLaunch & BIT0));
    }
    else
    {
        // DO NOT CHANGE universe.curPlayerPtr->autoLaunch DIRECTLY OR SYNC ERRORS WILL RESULT.  JUST REQUEST
        // WANT YOU WANT via clWrapAutoLaunch
        udword newAutoLaunch = universe.curPlayerPtr->autoLaunch;

        if (!(FECHECKED(atom)))    //sense now reversed!
             //check mark TRUE means stay docked: autolaunch is FALSE
        {
            newAutoLaunch |= BIT0;
        }
        else
        {
            newAutoLaunch &= ~BIT0;
        }
        clWrapAutoLaunch(newAutoLaunch,universe.curPlayerIndex);
        dbgMessagef("\nAutolaunch sent = %d",newAutoLaunch);
    }
}

void lmAutoLaunchCX(char *string, featom *atom, udword x)
{
    static udword bits[NUM_LMCARRIERS] = { BIT1, BIT2, BIT3, BIT4 };
    static udword shifts[NUM_LMCARRIERS] = { 1,2,3,4 };
    udword bit = bits[x-1];
    udword shift = shifts[x-1];

    dbgAssert((x-1) < NUM_LMCARRIERS);

    if (lmCarrierX[x-1]==NULL)
    {
        bitSet(atom->flags, FAF_Disabled);
        bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(atom->flags, FAF_Disabled);
        bitClear(((region *)atom->region)->status, RSF_RegionDisabled);
    }

    #ifdef DEBUG_STOMP
    regVerify((regionhandle)atom->region);
    #endif
    bitSet( ((regionhandle)atom->region)->status, RSF_DrawThisFrame);

    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(string, !((universe.curPlayerPtr->autoLaunch & bit) >> shift));
    }
    else
    {
        // DO NOT CHANGE universe.curPlayerPtr->autoLaunch DIRECTLY OR SYNC ERRORS WILL RESULT.  JUST REQUEST
        // WANT YOU WANT via clWrapAutoLaunch
        udword newAutoLaunch = universe.curPlayerPtr->autoLaunch;

        if (!(FECHECKED(atom)))
        {
            newAutoLaunch |= bit;
        }
        else
        {
            newAutoLaunch &= ~bit;
        }
        clWrapAutoLaunch(newAutoLaunch,universe.curPlayerIndex);
        dbgMessagef("\nAutolaunch sent = %d",newAutoLaunch);
    }
}

void lmAutoLaunchC1(char *string, featom *atom)
{
    lmAutoLaunchCX(string,atom,1);
}

void lmAutoLaunchC2(char *string, featom *atom)
{
    lmAutoLaunchCX(string,atom,2);
}

void lmAutoLaunchC3(char *string, featom *atom)
{
    lmAutoLaunchCX(string,atom,3);
}

void lmAutoLaunchC4(char *string, featom *atom)
{
    lmAutoLaunchCX(string,atom,4);
}

/*-----------------------------------------------------------------------------
    Name        : lmLaunchAll
    Description : process LaunchAll button
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmLaunchAll(char *string, featom *atom)
{
    Node *walk;
    ShipPtr shipinside;
    InsideShip *insideShipStruct;
    SelectCommand *selection;
    sdword numShips;
    sdword i;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bLaunchLaunchAll)
        return;

    feScreenDelete(lmBaseRegion);
    lmBaseRegion = NULL;

    if (lmIoSaveState)
        ioEnable();

    numShips = shipsin->insideList.num;
    if (numShips > 0)
    {
        selection = memAlloc(sizeofSelectCommand(numShips),"launchall",0);
        selection->numShips = numShips;

        i = 0;
        walk = shipsin->insideList.head;
        while (walk !=NULL)
        {
            insideShipStruct = (InsideShip *)listGetStructOfNode(walk);
            shipinside = insideShipStruct->ship;
            dbgAssert(shipinside->objtype == OBJ_ShipType);

            selection->ShipPtr[i++] = shipinside;

            walk = walk->next;
        }

        dbgAssert(i == numShips);
        clWrapLaunchMultipleShips(&universe.mainCommandLayer,selection,launchship);
        memFree(selection);

        speechEventFleet(COMM_F_Launch, 0, universe.curPlayerIndex);
    }

    /* play the exit sound */
    soundEvent(NULL, UI_ManagerExit);
    //restart the sound of space ambient
    soundEvent(NULL, UI_SoundOfSpace);

    // enable rendering of main game screen
    mrRenderMainScreen = TRUE;
    lmActive = FALSE;

    glcFullscreen(FALSE);

    // enable taskbar popup window
    tbDisable = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : lmClose
    Description : process Close button
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmClose(char *string, featom *atom)
{
    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bLaunchClose)
        return;

    feScreenDelete(lmBaseRegion);
    lmBaseRegion = NULL;

    if (lmIoSaveState)
        ioEnable();

    /* play the exit sound */
    soundEvent(NULL, UI_ManagerExit);
    //restart the sound of space ambient
    soundEvent(NULL, UI_SoundOfSpace);

    // enable rendering of main game screen
    mrRenderMainScreen = TRUE;
    lmActive = FALSE;

    glcFullscreen(FALSE);

    lmRenderEverythingCounter = 0;

    // enable taskbar popup window
    tbDisable = FALSE;

    // shutdown the shipview
    svClose();
}

void lmCloseIfOpen()
{
    if (lmBaseRegion)
    {
        lmClose(NULL,NULL);
    }
}

/*-----------------------------------------------------------------------------
    Name        : lmSelectShipType
    Description : Find what type of ship was selected
    Inputs      : reg - region clicked on
                  yClicked - vertical location clicked on.
                  xClicked - horozontal location clicked on.
    Outputs     :
    Return      : type of ship clicked on or -1 if none
----------------------------------------------------------------------------*/
/*sdword lmSelectShipType(regionhandle region, sdword yClicked, sdword xClicked)
{
    sdword index, x, y, yspace, boxclicked=-1, xtab;
    fonthandle currentFont;
    sdword shipnum;
    rectangle rect = region->rect;

    currentFont = fontMakeCurrent(lmShipListFont);
    yspace = fontHeight(" ") + LM_VertSpacing;
    fontMakeCurrent(currentFont);


    xtab = 0;
    y    = rect.y0 + LM_VertSpacing;
    x    = rect.x0 + LM_HorzSpacing;


    for (index = 0; PrintOrder[index]!=-1; index++)
    {
        if ( (yClicked >= y)&&
             (yClicked  < y + yspace) )
        {
            boxclicked = index;
            break;
        }
        y += yspace;
    }

    if (boxclicked != -1)
        for (index=0,shipnum=-1; PrintOrder[index]!=-1;index++)
        {
            if (PrintOrder[index] != SPACER)
            {
                if (shipsavailable[PrintOrder[index]].nShips > 0)
                    shipnum++;
                if (shipnum==boxclicked)
                {
                    svSelectShip(PrintOrder[index]);
                    lmcurshipview = PrintOrder[index];
                    return(index);
                }
            }
        }



    return(-1);
}*/

/*-----------------------------------------------------------------------------
    Name        : lmSelectAvailable
    Description : Callback for selecting/deselecting ships
    Inputs      : standard region callback
    Outputs     : selects or deselects a ship for launching
    Return      :
----------------------------------------------------------------------------*/
/*sdword lmSelectAvailable(regionhandle region, sdword ID, udword event, udword data)
{
    sdword clicked, index;

    if (event == RPE_PressLeft)
    {
        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bLaunchSelectShips)
            return 0;

        clicked = lmSelectShipType(region, mouseCursorY(), mouseCursorX());

        if (clicked != -1)
        {
            if (!controlstate)
            {
                for (index=0; PrintOrder[index]!=-1; index++)
                {
                    shipsavailable[PrintOrder[index]].bSelected = FALSE;
                }
            }
            if (shiftstate)
            {
                if (curselected > clicked)
                    for (index=clicked; index <= curselected; index++ )
                        shipsavailable[PrintOrder[index]].bSelected = TRUE;
                else
                    for (index=curselected; index <= clicked; index++ )
                        shipsavailable[PrintOrder[index]].bSelected = TRUE;
            }
            if (controlstate)
            {
                if (shipsavailable[PrintOrder[clicked]].bSelected)
                    shipsavailable[PrintOrder[clicked]].bSelected = FALSE;
                else
                    shipsavailable[PrintOrder[clicked]].bSelected = TRUE;
            }
            else
                shipsavailable[PrintOrder[clicked]].bSelected = TRUE;


            if (!shiftstate)
                curselected = clicked;

            if(tutorial)
            {
            char    msg[256];

                strcpy(msg, "Game_Launch_");
                if(shipsavailable[PrintOrder[clicked]].bSelected)
                    strcat(msg, "Select_");
                else
                    strcat(msg, "Deselect_");

                strcat(msg, ShipTypeToNiceStr(PrintOrder[clicked]));
                tutGameMessage(msg);
            }
        }
        return(0);
    }
    else if (event == RPE_KeyDown)
    {
        switch (ID)
        {
            case SHIFTKEY : shiftstate=TRUE;
                return(0);
            case CONTROLKEY : controlstate=TRUE;
                return(0);
        }
    }
    else if (event == RPE_KeyUp)
    {
        switch (ID)
        {
            case SHIFTKEY : shiftstate=FALSE;
                return(0);
            case CONTROLKEY : controlstate=FALSE;
                return(0);
        }
    }


    return(0);
}*/

void lmShipItemDraw(rectangle *rect, listitemhandle data)
{
    fonthandle currentFont;
    char       tempstr[64];
    color      col, plaincol, selectcol;
    sdword     x, y;
    udword     shipnum = (udword)data->data;

    currentFont = fontMakeCurrent(lmShipListFont);

    plaincol = FEC_ListItemStandard;//lmShipListTextColor;
    selectcol = FEC_ListItemSelected;//lmShipSelectedTextColor;

    if (bitTest(data->flags,UICLI_Selected))
        col = selectcol;
    else
        col = plaincol;

    x = rect->x0;
    y = rect->y0;

    if ((shipnum!=LM_FIGHTER_SPACER) && (shipnum!=LM_CORVETTE_SPACER))
    {
        sprintf(tempstr, " %i",shipsavailable[shipnum].nShips);
        fontPrintf(x+fontWidth(" 55 ")-fontWidth(tempstr), y, col,"%s",tempstr);
        fontPrintf(x+fontWidth(" 55 "), y, col," x %s", ShipTypeToNiceStr(shipnum));
    }
    else
    {
        if (strCurLanguage==4)
        {
            if (shipnum == LM_FIGHTER_SPACER)
            {
                fontPrintf(x, y, colRGB(255,200,0), "%s %s", strGetString(strCLASS_Class), strGetString(strCLASS_Fighter));
            }
            else if (shipnum == LM_CORVETTE_SPACER)
            {
                fontPrintf(x, y, colRGB(255,200,0), "%s %s", strGetString(strCLASS_Class), strGetString(strCLASS_Corvette));
            }
        }
        else
        {
            if (shipnum == LM_FIGHTER_SPACER)
            {
                fontPrintf(x, y, colRGB(255,200,0), "%s %s", strGetString(strCLASS_Fighter), strGetString(strCLASS_Class));
            }
            else if (shipnum == LM_CORVETTE_SPACER)
            {
                fontPrintf(x, y, colRGB(255,200,0), "%s %s", strGetString(strCLASS_Corvette), strGetString(strCLASS_Class));
            }
        }
    }

    fontMakeCurrent(currentFont);
}


/*-----------------------------------------------------------------------------
    Name        : lmShipsToLaunchDraw
    Description : draw Ships to Launch in box for selection
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmShipsToLaunchDraw(char *string, featom *atom)
{
    fonthandle      oldfont;
    sdword          index;
    bool            is_corv=FALSE, is_fight=FALSE;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(lmShipListFont);

        lmListWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(lmListWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          lmShipItemDraw,              // item draw function
                          fontHeight(" ") + LM_VertSpacing, // item height
                          UICLW_CanSelect);

        fontMakeCurrent(oldfont);

        for (index=LM_NUMSHIPS-1;index>=0;index--)
        {
            if ( ((PrintOrder[index]!=LM_FIGHTER_SPACER) && (PrintOrder[index]!=LM_CORVETTE_SPACER)) &&
                 (shipsavailable[PrintOrder[index]].nShips > 0) )
            {
                uicListAddItem(lmListWindow, (ubyte *)PrintOrder[index], UICLI_CanSelect, UICLW_AddToHead);
                if (shipsavailable[PrintOrder[index]].ship->staticinfo->shipclass==CLASS_Corvette)
                {
                    is_corv = TRUE;
                }
                else if (shipsavailable[PrintOrder[index]].ship->staticinfo->shipclass==CLASS_Fighter)
                {
                    is_fight = TRUE;
                }

            }
            else if ( ( (PrintOrder[index]==LM_FIGHTER_SPACER) && (is_fight) ) ||
                      ( (PrintOrder[index]==LM_CORVETTE_SPACER) && (is_corv) )  )
            {
                uicListAddItem(lmListWindow, (ubyte *)PrintOrder[index], 0, UICLW_AddToHead);
            }
        }

        return;
    }
    else if (FELASTCALL(atom))
    {
        lmListWindow = NULL;
        return;
    }
    else if (lmListWindow->message == CM_NewItemSelected)
    {
        lmcurshipview = (sdword)lmListWindow->CurLineSelected->data;
        svSelectShip(lmcurshipview);
    }
/*    fonthandle currentFont;
    rectangle  rect = region->rect, select;
    char       tempstr[64];
    sdword     index;
    color      col, plaincol, selectcol;
    sdword     x, y, xtab;

    currentFont = fontMakeCurrent(lmShipListFont);

    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft;     //receive mouse presses from now on
        regFunctionSet(region, (regionfunction)lmSelectAvailable);          //set new region handler function
    }
    feStaticRectangleDraw(region);                          //draw standard rectangle

    xtab = 0;
    y    = rect.y0 + LM_VertSpacing;
    x    = rect.x0 + LM_HorzSpacing;

    plaincol = FEC_ListItemStandard;//lmShipListTextColor;
    selectcol = FEC_ListItemSelected;//lmShipSelectedTextColor;



    for (index=0;PrintOrder[index]!=-1;index++)
    {
        if (PrintOrder[index]!=SPACER)
        {
            if (shipsavailable[PrintOrder[index]].bSelected)
                col = selectcol;
            else
                col = plaincol;

            if (shipsavailable[PrintOrder[index]].nShips > 0)
            {
                sprintf(tempstr, " %i",shipsavailable[PrintOrder[index]].nShips);
                fontPrintf(x+fontWidth(" 55 ")-fontWidth(tempstr), y, col,"%s",tempstr);
                fontPrintf(x+fontWidth(" 55 "), y, col," x %s", ShipTypeToNiceStr(PrintOrder[index]));
            }
            else
                goto skipnewline;
        }

        if (index == curselected)
        {
            select.x0 = x-LM_BoxSpacing;
            select.y0 = y-LM_BoxSpacing;
            select.x1 = x+(((rect.x1-rect.x0)*3)/4);
            select.y1 = y-LM_BoxSpacing+fontHeight(" ") + LM_VertSpacing;
            primRectTranslucent2(&select, FEC_ListItemTranslucent);
            primRectOutline2(&select, 1, FEC_ListItemTranslucentBorder); //selectcol
        }



        if (PrintOrder[index]==SPACER)
        {
            //xtab = (rect.x1-rect.x0)>>1;
            //y    = rect.y0 + LM_VertSpacing;
            plaincol = FEC_ListItemStandard;//lmShip2ListTextColor;
            selectcol = FEC_ListItemSelected;//lmShip2SelectedTextColor;
        }
        else
        {
            y += fontHeight(" ") + LM_VertSpacing;
        }

        x = rect.x0 + xtab + LM_HorzSpacing;

skipnewline:
        ;
    }

    fontMakeCurrent(currentFont);*/
}

/*-----------------------------------------------------------------------------
    Namems        : lmBarDraw
    Description : draws a bar with a background and a foreground and percent used
    Inputs      : rect, backcol, forecol, percent
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmBarDraw(rectangle *rect, color back, color fore, real32 percent)
{
    rectangle temp;
    primRectSolid2(rect, back);

    if (percent > 1.0)
    {
        percent = 1.0;
    }

    temp.x0 = rect->x0;
    temp.y0 = rect->y0;
    temp.x1 = rect->x0 + (sdword)((rect->x1-rect->x0)*percent);
    temp.y1 = rect->y1;

    primRectSolid2(&temp, lmUsedColor);
}



/*-----------------------------------------------------------------------------
    Name        : lmFighterUsedDraw
    Description : draw fighter's used bar and number
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmFighterUsedDraw(featom *atom, regionhandle region)
{
    real32 percent;
    sdword numused;
    fonthandle currentFont;

    if (lmRenderEverythingCounter > 0)
    {
        lmRenderEverythingCounter--;
        feRenderEverything = TRUE;
    }

    if (launchship->shipsInsideMe == NULL) return;

    currentFont = fontMakeCurrent(lmShipListFont);

    numused = (sdword)launchship->shipsInsideMe->FightersInsideme;

    percent = (real32)(launchship->shipsInsideMe->FightersInsideme) /
              (real32)(launchship->staticinfo->maxDockableFighters);

    if (numused >= launchship->staticinfo->maxDockableFighters)
        numused = launchship->staticinfo->maxDockableFighters;

    lmBarDraw(&region->rect, lmAvailableColor, lmUsedColor, percent);

    fontPrintf(region->rect.x1,region->rect.y0,FEC_ListItemStandard/*lmShipListTextColor*/," %i",numused);

    fontMakeCurrent(currentFont);
}


/*-----------------------------------------------------------------------------
    Name        : lmCorvetteUsedDraw
    Description : draw Corvette's used bar and number
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmCorvetteUsedDraw(featom *atom, regionhandle region)
{
    real32 percent;
    sdword numused;
    fonthandle currentFont;

    if (launchship->shipsInsideMe == NULL) return;

    currentFont = fontMakeCurrent(lmShipListFont);

    numused = (sdword)launchship->shipsInsideMe->CorvettesInsideme;

    percent = (real32)(launchship->shipsInsideMe->CorvettesInsideme) /
              (real32)(launchship->staticinfo->maxDockableCorvettes);

    if (numused >= launchship->staticinfo->maxDockableCorvettes)
        numused = launchship->staticinfo->maxDockableCorvettes;

    lmBarDraw(&region->rect, lmAvailableColor, lmUsedColor, percent);

    fontPrintf(region->rect.x1,region->rect.y0,FEC_ListItemStandard/*lmShipListTextColor*/," %i",numused);

    fontMakeCurrent(currentFont);
}



/*-----------------------------------------------------------------------------
    Name        : lmSelectMotherShip
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword lmSelectMotherShip(regionhandle region, sdword ID, udword event, udword data)
{
    if (universe.curPlayerPtr->PlayerMothership == NULL) return(0);

    if ((event == RPE_PressLeft) && (lmCurrentSelect != 0))
    {
        lmCurrentSelect = 0;

        //lmUpdateFactory(universe.curPlayerPtr->PlayerMothership);

        dbgAssert(region!=NULL);
#ifdef DEBUG_STOMP
        regVerify(region);
#endif
        bitSet(region->status, RSF_DrawThisFrame);

        lmReLaunch(universe.curPlayerPtr->PlayerMothership);

        lmUpdateShipView();
    }

    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : lmSelectCarrierX
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword lmSelectCarrierX(regionhandle region, sdword ID, udword event, udword data, udword x)
{
    if (lmCarrierX[x-1] == NULL) return (0);

    if ((event == RPE_PressLeft) && (lmCurrentSelect != x))
    {
        lmCurrentSelect = x;

        dbgAssert(region!=NULL);
#ifdef DEBUG_STOMP
        regVerify(region);
#endif
        bitSet(region->status, RSF_DrawThisFrame);

        lmReLaunch(lmCarrierX[x-1]);

        lmUpdateShipView();
    }

    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : lmSelectCarrier1
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword lmSelectCarrier1(regionhandle region, sdword ID, udword event, udword data)
{
    return lmSelectCarrierX(region,ID,event,data,1);
}

sdword lmSelectCarrier2(regionhandle region, sdword ID, udword event, udword data)
{
    return lmSelectCarrierX(region,ID,event,data,2);
}

sdword lmSelectCarrier3(regionhandle region, sdword ID, udword event, udword data)
{
    return lmSelectCarrierX(region,ID,event,data,3);
}

sdword lmSelectCarrier4(regionhandle region, sdword ID, udword event, udword data)
{
    return lmSelectCarrierX(region,ID,event,data,4);
}

/*-----------------------------------------------------------------------------
    Name        : lmDrawShipImage
    Description : draws the button image
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void lmDrawShipImage(regionhandle region, sdword shipID)
{
    rectangle rect;
    sdword usetexture;

    usetexture = (shipID == 0) ? 0 : 1;

    rect.x0 = region->rect.x0 + LM_SHIP_IMAGE_INSET;
    rect.y0 = region->rect.y0 + LM_SHIP_IMAGE_INSET;
    rect.x1 = region->rect.x1 - LM_SHIP_IMAGE_INSET;
    rect.y1 = region->rect.y1 - LM_SHIP_IMAGE_INSET;

/*    if (shipID == lmCurrentSelect)
        ferDrawFocusWindow(region, lw_focus);
    else
        ferDrawFocusWindow(region, lw_normal);*/

    if (glcActive())
    {
        lifheader* lif = lmShipImage[universe.curPlayerPtr->race][usetexture];
        glcRectSolidTexturedScaled2(&rect,
                                    lif->width, lif->height,
                                    lif->data,
                                    lmPaletted ? lif->palette : NULL,
                                    TRUE);
    }
    else
    {
        if (lmPaletted)
        {
            trPalettedTextureMakeCurrent(lmShipTexture[universe.curPlayerPtr->race][usetexture], lmShipImage[universe.curPlayerPtr->race][usetexture]->palette);
        }
        else
        {
            trRGBTextureMakeCurrent(lmShipTexture[universe.curPlayerPtr->race][usetexture]);
        }

        rndPerspectiveCorrection(FALSE);
        primRectSolidTextured2(&rect);
    }
}

/*-----------------------------------------------------------------------------
    Name        : lmMotherShipDraw
    Description : draws the mothership button
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void lmMotherShipDraw(featom *atom, regionhandle region)
{
    Ship *shipfactory = universe.curPlayerPtr->PlayerMothership;

    //lmMotherShipRegion = region;

    if (shipfactory != NULL && shipfactory->shiptype == Mothership)
    {
        if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
        {                                         //if region not processed yet
            region->flags = RPE_PressLeft;        //receive mouse presses from now on
            region->flags |= RPE_DrawEveryFrame;
            regFunctionSet(region, (regionfunction)lmSelectMotherShip); //set new region handler function
        }
        if (0 == lmCurrentSelect)
            ferDrawFocusWindow(region, lw_focus);
        else
            ferDrawFocusWindow(region, lw_normal);

        lmDrawShipImage(region, 0);
    }
}

void lmFillInCarrierXs(void)
{
    sdword i,insert,index;

    for (i=0;i<NUM_LMCARRIERS;i++)
    {
        lmCarrierX[i] = NULL;
    }

    for (insert=0,index=0;index<cmNumCarriers;index++)
    {
        if (cmCarriers[index].owner == universe.curPlayerPtr)
        {
            if (insert < NUM_LMCARRIERS)
            {
                lmCarrierX[insert++] = cmCarriers[index].ship;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : lmCarrierXDraw
    Description : draws the carrierX button
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void lmCarrierXDraw(featom *atom, regionhandle region,udword x)
{
    sdword index, count = 0;
    //lmCarrier2Region = region;

    for (index=0;index<cmNumCarriers;index++)
    {
        if (cmCarriers[index].owner == universe.curPlayerPtr)
        {
            count++;
            if(count > (x-1))
            {
                lmCarrierX[x-1] = cmCarriers[index].ship;
                if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
                {                                         //if region not processed yet
                    region->flags = RPE_PressLeft;        //receive mouse presses from now on
                    region->flags |= RPE_DrawEveryFrame;
                    switch (x)
                    {
                        case 1: regFunctionSet(region, (regionfunction)lmSelectCarrier1); break;
                        case 2: regFunctionSet(region, (regionfunction)lmSelectCarrier2); break;
                        case 3: regFunctionSet(region, (regionfunction)lmSelectCarrier3); break;
                        case 4: regFunctionSet(region, (regionfunction)lmSelectCarrier4); break;
                        default: dbgAssert(FALSE); break;
                    }
                }
                lmDrawShipImage(region, x);
                break;
            }
        }
    }
}

void lmCarrier1Draw(featom *atom, regionhandle region)
{
    featom *batom;

    if (1 == lmCurrentSelect)
        ferDrawFocusWindow(region, lw_focus);
    else
        ferDrawFocusWindow(region, lw_normal);

    batom=feAtomFindInScreen(feScreenFind(LM_LaunchScreen),"LM_AutoLaunchC1");
    if (lmCarrierX[0]==NULL)
    {
        bitSet(batom->flags, FAF_Disabled);
        bitSet(((regionhandle)batom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(batom->flags, FAF_Disabled);
        bitClear(((regionhandle)batom->region)->status, RSF_RegionDisabled);
    }

    lmCarrierXDraw(atom,region,1);
}

void lmCarrier2Draw(featom *atom, regionhandle region)
{
    featom *batom;

    if (2 == lmCurrentSelect)
        ferDrawFocusWindow(region, lw_focus);
    else
        ferDrawFocusWindow(region, lw_normal);

    batom=feAtomFindInScreen(feScreenFind(LM_LaunchScreen),"LM_AutoLaunchC2");
    if (lmCarrierX[1]==NULL)
    {
        bitSet(batom->flags, FAF_Disabled);
        bitSet(((regionhandle)batom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(batom->flags, FAF_Disabled);
        bitClear(((regionhandle)batom->region)->status, RSF_RegionDisabled);
    }

    lmCarrierXDraw(atom,region,2);
}

void lmCarrier3Draw(featom *atom, regionhandle region)
{
    featom *batom;

    if (3 == lmCurrentSelect)
        ferDrawFocusWindow(region, lw_focus);
    else
        ferDrawFocusWindow(region, lw_normal);

    batom=feAtomFindInScreen(feScreenFind(LM_LaunchScreen),"LM_AutoLaunchC3");
    if (lmCarrierX[2]==NULL)
    {
        bitSet(batom->flags, FAF_Disabled);
        bitSet(((regionhandle)batom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(batom->flags, FAF_Disabled);
        bitClear(((regionhandle)batom->region)->status, RSF_RegionDisabled);
    }

    lmCarrierXDraw(atom,region,3);
}

void lmCarrier4Draw(featom *atom, regionhandle region)
{
    featom *batom;

    if (4 == lmCurrentSelect)
        ferDrawFocusWindow(region, lw_focus);
    else
        ferDrawFocusWindow(region, lw_normal);

    batom=feAtomFindInScreen(feScreenFind(LM_LaunchScreen),"LM_AutoLaunchC4");
    if (lmCarrierX[3]==NULL)
    {
        bitSet(batom->flags, FAF_Disabled);
        bitSet(((regionhandle)batom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(batom->flags, FAF_Disabled);
        bitClear(((regionhandle)batom->region)->status, RSF_RegionDisabled);
    }

    lmCarrierXDraw(atom,region,4);
}

/*-----------------------------------------------------------------------------
    Name        : lmLoadTextures
    Description : load the textures used by the launch manager
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void lmLoadTextures(void)
{
    char filename[128];
    sdword i, j;

    for(i=0; i < MAX_RACES; i++)
    {
        for(j = 0; j < NUM_LMTEXTURES; j++)
        {
            // Build filename for loading texture from file
            strcpy(filename, lmShipImagePaths[i]);
            strcat(filename, lmShipIcons[j]);

            // Load the image
            lmShipImage[i][j] = trLIFFileLoad(filename, NonVolatile);
            if (bitTest(lmShipImage[i][j]->flags, TRF_Paletted))
            {
                lmPaletted = TRUE;
            }
            else
            {
                lmPaletted = FALSE;
            }

            if (lmPaletted)
            {
                lmShipTexture[i][j] = trPalettedTextureCreate(
                    lmShipImage[i][j]->data,
                    lmShipImage[i][j]->palette,
                    lmShipImage[i][j]->width,
                    lmShipImage[i][j]->height);
            }
            else
            {
                lmShipTexture[i][j] = trRGBTextureCreate(
                    (color*)lmShipImage[i][j]->data,
                    lmShipImage[i][j]->width,
                    lmShipImage[i][j]->height,
                    TRUE);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : lmFreeTextures
    Description : delete the textures used by the launch manager
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void lmFreeTextures(void)
{
    sdword i,j;

    for (i=0;i<MAX_RACES;i++)
    {
        for (j=0;j<NUM_LMTEXTURES;j++)
        {
            if (lmShipImage[i][j] != NULL)
            {
                memFree(lmShipImage[i][j]);
                lmShipImage[i][j] = NULL;
            }
            if (lmShipTexture[i][j] != TR_InvalidHandle)
            {
                trRGBTextureDelete(lmShipTexture[i][j]);
                lmShipTexture[i][j] = TR_InvalidHandle;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : lmStartup
    Description : Start up the launch manager
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmStartup(void)
{
    sdword index;

    lmRenderEverythingCounter = 0;

    lmShipListFont = frFontRegister(lmShipListFontName);
    lmLoadTextures();

    for (index=0;index < TOTAL_NUM_SHIPS; index++)
    {
        lmShipInList[index] = FALSE;
    }

    for (index=0; PrintOrder[index]!=-1; index++)
    {
        if ( ((PrintOrder[index]!=LM_FIGHTER_SPACER) && (PrintOrder[index]!=LM_CORVETTE_SPACER)) &&
             (PrintOrder[index] != -1))
        {    // ship is in the list so add it in
            lmShipInList[PrintOrder[index]] = TRUE;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : lmShutdown
    Description : shut down the launch manager
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmShutdown(void)
{
    if (lmScreensHandle)
    {
        feScreensDelete(lmScreensHandle);
        lmScreensHandle = NULL;
    }
    lmFreeTextures();
}

/*-----------------------------------------------------------------------------
    Name        : lmUpdateShipsInside
    Description : update ship display in launch manager
    Inputs      : ship that created ship
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void lmUpdateShipsInside(void)
{
    uword i;
    Node *walk;
    ShipPtr shipinside;
    InsideShip *insideShipStruct;
    sdword      index;
    sdword      shipnum=-1;
    listitemhandle listitem;
    bool            is_corv=FALSE, is_fight=FALSE;

    if (lmBaseRegion!=NULL)
    {
        // get the pointer to the list of ships inside the ship
        shipsin = launchship->shipsInsideMe;

        for (i=0;i<TOTAL_NUM_SHIPS;i++)
        {
            shipsavailable[i].nShips = 0;
            shipsavailable[i].ship = NULL;
        }

        walk = shipsin->insideList.head;
        while (walk !=NULL)
        {
            insideShipStruct = (InsideShip *)listGetStructOfNode(walk);
            shipinside = insideShipStruct->ship;
            dbgAssert(shipinside->objtype == OBJ_ShipType);
            shipsavailable[shipinside->shiptype].nShips++;
            shipsavailable[shipinside->shiptype].ship = shipinside;
            walk = walk->next;
        }

        if ((lmListWindow!=NULL) && (lmListWindow->CurLineSelected!=NULL))
        {
            shipnum = (sdword)lmListWindow->CurLineSelected->data;
        }

        if (lmListWindow!=NULL)
        {
            uicListCleanUp(lmListWindow);

            for (index=LM_NUMSHIPS-1;index>=0;index--)
            {
                if ( ((PrintOrder[index]!=LM_FIGHTER_SPACER) && (PrintOrder[index]!=LM_CORVETTE_SPACER)) &&
                     (shipsavailable[PrintOrder[index]].nShips > 0) )
                {
                    uicListAddItem(lmListWindow, (ubyte *)PrintOrder[index], UICLI_CanSelect, UICLW_AddToHead);
                    if (shipsavailable[PrintOrder[index]].ship->staticinfo->shipclass==CLASS_Corvette)
                    {
                        is_corv = TRUE;
                    }
                    else if (shipsavailable[PrintOrder[index]].ship->staticinfo->shipclass==CLASS_Fighter)
                    {
                        is_fight = TRUE;
                    }

                }
                else if ( ( (PrintOrder[index]==LM_FIGHTER_SPACER) && (is_fight) ) ||
                          ( (PrintOrder[index]==LM_CORVETTE_SPACER) && (is_corv) )  )
                {
                    uicListAddItem(lmListWindow, (ubyte *)PrintOrder[index], 0, UICLW_AddToHead);
                }
            }

            if (shipnum != -1)
            {
                listitem = uicListFindItemByData(lmListWindow, (ubyte *)shipnum);
                uicListWindowSetCurItem(lmListWindow, listitem);
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : lmLaunchBegin
    Description : Begin launch manager
    Inputs      : event handles
    Outputs     :
    Return      : sdword
----------------------------------------------------------------------------*/
sdword lmLaunchBegin(regionhandle region, sdword ID, udword event, udword data)
{
    uword i;
    Node *walk;
    ShipPtr shipinside;
    InsideShip *insideShipStruct;

    if ((playPackets) || (universePause)) return 0;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bLaunch)
        return (0);

    launchship = (Ship *)ID;

    if (universe.curPlayerPtr->PlayerMothership == NULL)
    {
        return (0);
    }

    lmFillInCarrierXs();

    if ((launchship == NULL)||((launchship->shiptype!=Mothership)&&(launchship->shiptype!=Carrier)))
    {
        if (lmCurrentSelect==0)
        {
            if (universe.curPlayerPtr->PlayerMothership != NULL)
            {
                launchship = universe.curPlayerPtr->PlayerMothership;

                if (universe.curPlayerPtr->PlayerMothership->shiptype == Carrier)
                {
                    lmCurrentSelect = 1;
                }
                else if (universe.curPlayerPtr->PlayerMothership->shiptype == Mothership)
                {
                    lmCurrentSelect = 0;
                }
            }
        }
        else if (lmCarrierX[lmCurrentSelect-1]!=NULL)
        {
            launchship = lmCarrierX[lmCurrentSelect-1];
        }
        else
        {
            launchship = universe.curPlayerPtr->PlayerMothership;
            if (universe.curPlayerPtr->PlayerMothership->shiptype == Carrier)
            {
                lmCurrentSelect = 1;
            }
            else if (universe.curPlayerPtr->PlayerMothership->shiptype == Mothership)
            {
                lmCurrentSelect = 0;
            }
        }
    }
    else if (launchship->shiptype == Mothership)
    {
        lmCurrentSelect = 0;
    }
    else
    {
        sdword i;

        for (i=0;i<cmNumPlayersCarriers;i++)
        {
            if (launchship == lmCarrierX[i])
            {
                lmCurrentSelect = i+1;
                break;
            }
        }
    }

    if (launchship == NULL)
    {
        if (cmNumPlayersCarriers > 0)
        {
            launchship = lmCarrierX[0];
            lmCurrentSelect = 1;
        }
        else
            return(0);
    }

    // disable rendering of main game screen
    mrRenderMainScreen = FALSE;
    lmActive = TRUE;

    glcFullscreen(TRUE);

    lmRenderEverythingCounter = (tutorial == TUTORIAL_ONLY) ? 4 : 0;

    // clear the screen
    rndClear();

    // disable taskbar popup window
    tbDisable = TRUE;

    // get the pointer to the list of ships inside the ship
    shipsin = launchship->shipsInsideMe;

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        shipsavailable[i].nShips = 0;
        shipsavailable[i].bSelected = FALSE;
        shipsavailable[i].ship = NULL;
    }

    walk = shipsin->insideList.head;
    while (walk !=NULL)
    {
        insideShipStruct = (InsideShip *)listGetStructOfNode(walk);
        shipinside = insideShipStruct->ship;
        dbgAssert(shipinside->objtype == OBJ_ShipType);
        shipsavailable[shipinside->shiptype].nShips++;
        shipsavailable[shipinside->shiptype].ship = shipinside;
        walk = walk->next;
    }

    soundEventStopSFX(0.5f);

    /* play the intro sound */
    soundEvent(NULL, UI_ManagerIntro);
    // play the launch manager ambient
    soundEvent(NULL, UI_LaunchManager);

    if (!lmScreensHandle)
    {
        feCallbackAddMultiple(lmCallback);                  //add in the callbacks
        feDrawCallbackAddMultiple(lmDrawCallback);
        lmScreensHandle = feScreensLoad(LM_FIBFile);        //load in the screen
    }

    lmIoSaveState = ioDisable();

    lmBaseRegion = feScreenStart(region, LM_LaunchScreen);  //add new regions as siblings of current one

    mouseCursorShow();

    tutGameMessage("Start_LaunchManager");

/*    regKeyChildAlloc(lmBaseRegion, SHIFTKEY,   RPE_KeyDown|RPE_KeyUp, (regionfunction)lmSelectAvailable, 1, SHIFTKEY);
    regKeyChildAlloc(lmBaseRegion, CONTROLKEY, RPE_KeyDown|RPE_KeyUp, (regionfunction)lmSelectAvailable, 1, CONTROLKEY);*/

    lmUpdateShipView();

    return(0);
}



/*-----------------------------------------------------------------------------
    Name        : lmReLaunchBegin
    Description : switch to another carrier or the mothership
    Inputs      : event handles
    Outputs     :
    Return      : sdword
----------------------------------------------------------------------------*/

void lmReLaunch(Ship *newship)
{
    uword i;
    Node *walk;
    ShipPtr shipinside;
    InsideShip *insideShipStruct;
    sdword      index;
    sdword      shipnum=-1;
    listitemhandle listitem;
    bool            is_corv=FALSE, is_fight=FALSE;

    launchship = newship;

    // get the pointer to the list of ships inside the ship
    shipsin = launchship->shipsInsideMe;

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        shipsavailable[i].nShips = 0;
        shipsavailable[i].bSelected = FALSE;
        shipsavailable[i].ship = NULL;
    }

    walk = shipsin->insideList.head;
    while (walk !=NULL)
    {
        insideShipStruct = (InsideShip *)listGetStructOfNode(walk);
        shipinside = insideShipStruct->ship;
        dbgAssert(shipinside->objtype == OBJ_ShipType);
        shipsavailable[shipinside->shiptype].nShips++;
        shipsavailable[shipinside->shiptype].ship = shipinside;
        walk = walk->next;
    }

    if ((lmListWindow!=NULL) && (lmListWindow->CurLineSelected!=NULL))
    {
        shipnum = (sdword)lmListWindow->CurLineSelected->data;
    }

    if (lmListWindow!=NULL)
    {
        uicListCleanUp(lmListWindow);

        for (index=LM_NUMSHIPS-1;index>=0;index--)
        {
            if ( ((PrintOrder[index]!=LM_FIGHTER_SPACER) && (PrintOrder[index]!=LM_CORVETTE_SPACER)) &&
                 (shipsavailable[PrintOrder[index]].nShips > 0) )
            {
                uicListAddItem(lmListWindow, (ubyte *)PrintOrder[index], UICLI_CanSelect, UICLW_AddToHead);

                if (shipsavailable[PrintOrder[index]].ship->staticinfo->shipclass==CLASS_Corvette)
                {
                    is_corv = TRUE;
                }
                else if (shipsavailable[PrintOrder[index]].ship->staticinfo->shipclass==CLASS_Fighter)
                {
                    is_fight = TRUE;
                }

            }
            else if ( ( (PrintOrder[index]==LM_FIGHTER_SPACER) && (is_fight) ) ||
                      ( (PrintOrder[index]==LM_CORVETTE_SPACER) && (is_corv) )  )
            {
                uicListAddItem(lmListWindow, (ubyte *)PrintOrder[index], 0, UICLW_AddToHead);
            }
        }

        if (shipnum != -1)
        {
            listitem = uicListFindItemByData(lmListWindow, (ubyte *)shipnum);
            uicListWindowSetCurItem(lmListWindow, listitem);
        }
    }
}

/*-----------------------------------------------------------------------
   Name        : lmLaunchCapableShipDied
   Description : This function will remove any launch manager references
                 to the ship that just died.
   Outputs     : no outputs
   Parameters  : void
   Return      : void
-----------------------------------------------------------------------*/

void lmLaunchCapableShipDied(Ship *ship)
{
    featom* atom;

    // if shipptr passed is not a carrier or mothership then get the players mothership
/*    launchship = universe.curPlayerPtr->PlayerMothership;

    if (launchship == ship) launchship = NULL;

    lmCurrentSelect = 0;

    for (i=0;i<NUM_LMCARRIERS;i++)
    {
        lmCarrierX[i] = NULL;
    }

    for (i=0,index=0;index<cmNumCarriers;index++)
    {
        if ((cmCarriers[index].owner == universe.curPlayerPtr) && (cmCarriers[index].ship != ship))
        {
            if (i < NUM_LMCARRIERS)
            {
                lmCarrierX[i++] = cmCarriers[index].ship;
            }
        }
    }*/

    lmFillInCarrierXs();

    if (launchship == ship)
    {
        launchship = NULL;
        lmCurrentSelect--;
        if (lmCurrentSelect < 0)
        {
            lmCurrentSelect = 0;
        }
    }

    if ((launchship == NULL)||((launchship->shiptype!=Mothership)&&(launchship->shiptype!=Carrier)))
    {
        if (lmCurrentSelect==0)
        {
            if (ship != universe.curPlayerPtr->PlayerMothership)
            {
                launchship = universe.curPlayerPtr->PlayerMothership;

                if (universe.curPlayerPtr->PlayerMothership->shiptype == Carrier)
                {
                    lmCurrentSelect = 1;
                }
                else if (universe.curPlayerPtr->PlayerMothership->shiptype == Mothership)
                {
                    lmCurrentSelect = 0;
                }
            }
        }
        else if (lmCarrierX[lmCurrentSelect-1]!=NULL)
        {
            launchship = lmCarrierX[lmCurrentSelect-1];
        }
        else
        {
            launchship = universe.curPlayerPtr->PlayerMothership;
            if (universe.curPlayerPtr->PlayerMothership->shiptype == Carrier)
            {
                lmCurrentSelect = 1;
            }
            else if (universe.curPlayerPtr->PlayerMothership->shiptype == Mothership)
            {
                lmCurrentSelect = 0;
            }
        }
    }
    else if (launchship->shiptype == Mothership)
    {
        lmCurrentSelect = 0;
    }
    else
    {
        sdword i;

        for (i=0;i<NUM_LMCARRIERS;i++)
        {
            if (launchship == lmCarrierX[i])
            {
                lmCurrentSelect = i+1;
                break;
            }
        }
    }

    if (launchship == NULL)
    {
        if (cmNumPlayersCarriers > 0)
        {
            launchship = lmCarrierX[0];
            lmCurrentSelect = 1;
        }
        else
        {
            lmClose(NULL, NULL);
            return;
        }
    }

    lmUpdateShipsInside();


    if(shipsavailable[0].nShips > 0)
    {
        svSelectShip(shipsavailable[0].ship->shiptype);
    }
    else
    {
        svSelectShip(DefaultShip);
    }

    atom=feAtomFindInScreen(feScreenFind(LM_LaunchScreen),"LM_AutoLaunchC1");
    if (lmCarrierX[0]==NULL)
    {
        bitSet(atom->flags, FAF_Disabled);
        bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(atom->flags, FAF_Disabled);
        bitClear(((region *)atom->region)->status, RSF_RegionDisabled);
    }

    atom=feAtomFindInScreen(feScreenFind(LM_LaunchScreen),"LM_AutoLaunchC2");
    if (lmCarrierX[1]==NULL)
    {
        bitSet(atom->flags, FAF_Disabled);
        bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(atom->flags, FAF_Disabled);
        bitClear(((region *)atom->region)->status, RSF_RegionDisabled);
    }

    atom=feAtomFindInScreen(feScreenFind(LM_LaunchScreen),"LM_AutoLaunchC3");
    if (lmCarrierX[2]==NULL)
    {
        bitSet(atom->flags, FAF_Disabled);
        bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(atom->flags, FAF_Disabled);
        bitClear(((region *)atom->region)->status, RSF_RegionDisabled);
    }

    atom=feAtomFindInScreen(feScreenFind(LM_LaunchScreen),"LM_AutoLaunchC4");
    if (lmCarrierX[3]==NULL)
    {
        bitSet(atom->flags, FAF_Disabled);
        bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(atom->flags, FAF_Disabled);
        bitClear(((region *)atom->region)->status, RSF_RegionDisabled);
    }


    regRecursiveSetDirty(lmBaseRegion);
//    }
}
