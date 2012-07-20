/*=============================================================================
    Name    : InfoOverlay.c
    Purpose : This file contains the logic for the information overlay.

    Created 6/8/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "InfoOverlay.h"
#include "Key.h"
#include "FontReg.h"
#include "utility.h"
#include "Select.h"
#include "Region.h"
#include "Task.h"
#include "StatScript.h"
#include "Strings.h"
#include "Tutor.h"

#include <stdio.h>

/*=============================================================================
    Defines:
=============================================================================*/

#define numOverlays             (TOTAL_NUM_SHIPS)
#define numButtons              TOTAL_NUM_SHIPS
#define IO_VertSpacing          (fontHeight(" ") >> 1)
#define IO_HorzSpacing          (fontWidth(" "))
#define IO_ListWidth            150

#define IO_DefaultFont          "HW_EuroseCond_11.hff"


/*=============================================================================
    Data:
=============================================================================*/


fonthandle   ioShipListFont=0;
char         ioShipListFontName[64] = IO_DefaultFont;

ShipListInfo    listinfo[numOverlays + 1]=
{
    {0      , 0     , LightDefender         , NULL },
    {0      , 0     , HeavyDefender         , NULL },
    {0      , 0     , LightInterceptor      , NULL },
    {0      , 0     , HeavyInterceptor      , NULL },
    {0      , 0     , CloakedFighter        , NULL },
    {0      , 0     , DefenseFighter        , NULL },
    {0      , 0     , AttackBomber          , NULL },
    {0      , 0     , LightCorvette         , NULL },
    {0      , 0     , HeavyCorvette         , NULL },
    {0      , 0     , RepairCorvette        , NULL },
    {0      , 0     , SalCapCorvette        , NULL },
    {0      , 0     , MinelayerCorvette     , NULL },
    {0      , 0     , MultiGunCorvette      , NULL },
    {0      , 0     , ResourceCollector     , NULL },
    {0      , 0     , ResourceController    , NULL },
    {0      , 0     , StandardFrigate       , NULL },
    {0      , 0     , AdvanceSupportFrigate , NULL },
    {0      , 0     , DDDFrigate            , NULL },
    {0      , 0     , DFGFrigate            , NULL },
    {0      , 0     , IonCannonFrigate      , NULL },
    {0      , 0     , StandardDestroyer     , NULL },
    {0      , 0     , MissileDestroyer      , NULL },
    {0      , 0     , HeavyCruiser          , NULL },
    {0      , 0     , Carrier               , NULL },
    {0      , 0     , Probe                 , NULL },
    {0      , 0     , ProximitySensor       , NULL },
    {0      , 0     , SensorArray           , NULL },
    {0      , 0     , ResearchShip          , NULL },
    {0      , 0     , GravWellGenerator     , NULL },
    {0      , 0     , CloakGenerator        , NULL },
    {0      , 0     , Mothership            , NULL },   //mothership
    {0      , 0     , DefaultShip           , NULL },   //target drone
    {0      , 0     , DefaultShip           , NULL },   //drone
    {0      , 0     , DefaultShip           , NULL },    //headshot asteroid

    {0      , 0     , CryoTray           , NULL },    //headshot asteroid
    {0      , 0     , P1Fighter           , NULL },    //headshot asteroid
    {0      , 0     , P1IonArrayFrigate           , NULL },    //headshot asteroid
    {0      , 0     , P1MissileCorvette           , NULL },    //headshot asteroid
    {0      , 0     , P1Mothership           , NULL },    //headshot asteroid
    {0      , 0     , P1StandardCorvette           , NULL },    //headshot asteroid
    {0      , 0     , P2AdvanceSwarmer           , NULL },    //headshot asteroid
    {0      , 0     , P2FuelPod           , NULL },    //headshot asteroid
    {0      , 0     , P2Mothership           , NULL },    //headshot asteroid
    {0      , 0     , P2MultiBeamFrigate           , NULL },    //headshot asteroid
    {0      , 0     , P2Swarmer           , NULL },    //headshot asteroid
    {0      , 0     , P3Destroyer           , NULL },    //headshot asteroid
    {0      , 0     , P3Frigate           , NULL },    //headshot asteroid
    {0      , 0     , P3Megaship           , NULL },    //headshot asteroid
    {0      , 0     , FloatingCity           , NULL },    //headshot asteroid
    {0      , 0     , CargoBarge           , NULL },    //headshot asteroid
    {0      , 0     , MiningBase           , NULL },    //headshot asteroid
    {0      , 0     , ResearchStation           , NULL },    //headshot asteroid
    {0      , 0     , JunkYardDawg           , NULL },    //headshot asteroid
    {0      , 0     , JunkYardHQ           , NULL },    //headshot asteroid
    {0      , 0     , Ghostship           , NULL },    //headshot asteroid
    {0      , 0     , Junk_LGun           , NULL },    //headshot asteroid
    {0      , 0     , Junk_SGun           , NULL },    //headshot asteroid
    {0      , 0     , ResearchStationBridge           , NULL },    //headshot asteroid
    {-1      , 0     , ResearchStationTower           , NULL }    //headshot asteroid
};

ShipOverlayInfo overlayinfo[numButtons];

sdword          lookupforlist[TOTAL_NUM_SHIPS + 1];

color ioListTextColor     = IO_ListTextColor;
color ioSelectedTextColor = IO_SelectedTextColor;

bool            ioRunning=TRUE;

/*=============================================================================
    Logic:
=============================================================================*/

// Function callback for maks of region
udword ioListClick(regionhandle reg, sdword ID, udword event, udword data);

// Draw callback for list of ships selected
void ioShipListDraw(regionhandle region);

bool crapthing(udword num, void *data, struct BabyCallBack *baby)
{
    ioUpdateShipTotals();

    return(TRUE);
}

udword ioListClick(regionhandle reg, sdword ID, udword event, udword data)
{
    if (keyIsHit(SHIFTKEY))
    {
        overlayinfo[ID].listinfo->bSelected = (bool16)!overlayinfo[ID].listinfo->bSelected;
    }
    else
    {
        overlayinfo[ID].listinfo->bSelected = TRUE;
        ioSetSelection(FALSE);
    }
    tutGameMessage("Game_InfoOverlayClick");

    return(0);
}

void ioShipListDraw(regionhandle region)
{
    fonthandle oldfont;
    char       tempstr[64];
    color      col;

    oldfont = fontMakeCurrent(ioShipListFont);

    if (overlayinfo[region->userID].listinfo->bSelected)
        col = ioSelectedTextColor;
    else
        col = ioListTextColor;

    sprintf(tempstr, " %i",overlayinfo[region->userID].listinfo->nShips);
    fontPrintf(region->rect.x0+fontWidth(" 55 ")-fontWidth(tempstr), region->rect.y0, col,"%s",tempstr);
    fontPrintf(region->rect.x0+fontWidth(" 55 "), region->rect.y0, col," x %s", strGetString(overlayinfo[region->userID].listinfo->shipnum + strShipAbrevOffset));

    fontMakeCurrent(oldfont);
}

void ioSetSelection(bool shiftrelease)
{
    MaxSelection temp;
    sdword       index;
    bool         ships=FALSE;

    temp.numShips = 0;

    for (index=0;index<selSelected.numShips;index++)
    {
        if (selSelected.ShipPtr[index]->shiptype < TOTAL_NUM_SHIPS)
        {
            if (listinfo[lookupforlist[selSelected.ShipPtr[index]->shiptype]].bSelected)
            {
//                if (selSelected.ShipPtr[index]->shiptype!=Mothership)
//                {
                    selSelectionAddSingleShip(&temp,selSelected.ShipPtr[index]);
                    ships=TRUE;
//                }
            }
        }
    }

    if (shiftrelease)
    {
        if (ships)
        {
            selSelectionCopy((MaxAnySelection *)&selSelected, (MaxAnySelection *)&temp);

            taskCallBackRegister(crapthing, 0, NULL, (real32)0.25);
            tutGameMessage("Game_InfoOverlayShiftSelect");
        }
    }
    else
    {
        selSelectionCopy((MaxAnySelection *)&selSelected, (MaxAnySelection *)&temp);

        taskCallBackRegister(crapthing, 0, NULL, (real32)0.25);
        tutGameMessage("Game_InfoOverlaySelect");
    }
}

void ioUpdateShipTotals(void)
{
    sdword index, screen;

    if (ioRunning || (tutorial==TUTORIAL_ONLY))
    {
        for (index=0;listinfo[index].nShips!=-1;index++)
        {
            listinfo[index].nShips = 0;
            listinfo[index].bSelected   = FALSE;
        }

        for (index=0;index<selSelected.numShips;index++)
        {
            if (selSelected.ShipPtr[index]->shiptype < TOTAL_NUM_SHIPS )
            {
                listinfo[lookupforlist[selSelected.ShipPtr[index]->shiptype]].nShips++;
            }
        }

        for (index=0,screen=0;listinfo[index].nShips!=-1;index++)
        {
            if (listinfo[index].nShips>0)
            {
                overlayinfo[screen].listinfo = &listinfo[index];
                if (!overlayinfo[screen].inlist)
                {
                    regChildInsert(overlayinfo[screen].region,ghMainRegion);
                    overlayinfo[screen].inlist = TRUE;
                }
                screen++;
            }
        }

        for (;screen<numButtons;screen++)
        {
            if (overlayinfo[screen].inlist)
            {
                regLinkRemove(overlayinfo[screen].region);
                overlayinfo[screen].inlist = FALSE;
            }
        }
    }
}

bool ioDisable(void)
{
    sdword index;
    bool   save;

    save = ioRunning;
    ioRunning = FALSE;
    for (index=0;index<numButtons;index++)
    {
        if (overlayinfo[index].inlist)
        {
            regLinkRemove(overlayinfo[index].region);
            overlayinfo[index].inlist = FALSE;
        }
    }
    return (save);
}

void ioEnable(void)
{
    ioRunning = TRUE;
    ioUpdateShipTotals();
}

void ioStartup(void)
{
    sdword index, x, y;
    fonthandle oldfont;

    ioShipListFont = frFontRegister(ioShipListFontName);
    oldfont        = fontMakeCurrent(ioShipListFont);

    y = ghMainRegion->rect.y0+IO_VertSpacing;
    x = ghMainRegion->rect.x1-IO_ListWidth;

    for (index=0;index<numButtons;index++)
    {
        overlayinfo[index].region = regChildAlloc(ghMainRegion, index,
                                                  x, y, IO_ListWidth, fontHeight(" "),
                                                  0, RPE_PressLeft);
        regDrawFunctionSet(overlayinfo[index].region, ioShipListDraw);
        regFunctionSet(overlayinfo[index].region, ioListClick);
        regLinkRemove(overlayinfo[index].region);
        overlayinfo[index].inlist = FALSE;
        y+=fontHeight(" ")-2;
    }

    for (index=0;index<TOTAL_NUM_SHIPS;index++)
    {
        lookupforlist[index] = numOverlays;
    }

    for (index=0;listinfo[index].nShips!=-1;index++)
    {
        lookupforlist[listinfo[index].shipnum] = index;
    }

    fontMakeCurrent(oldfont);
}

void ioShutdown(void)
{
    sdword index;

    for (index=0;index<numButtons;index++)
    {
        regRegionDelete(overlayinfo[index].region);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ioResolutionChange
    Description : Called when there is a change in screen resolution.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ioResolutionChange(void)
{
    sdword index, x, diff;

    x = ghMainRegion->rect.x1-IO_ListWidth;
    diff = x - overlayinfo[0].region->rect.x0;

    for (index=0;index<numButtons;index++)
    {
        overlayinfo[index].region->rect.x0 += diff;
        overlayinfo[index].region->rect.x1 += diff;
    }

}

