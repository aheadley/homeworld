/*=============================================================================
    Name    : TradeMgr.c
    Purpose : Logic for the Trade Manager

    Created 10/06/1998 by jthornto
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include "FastMath.h"

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
#include "ConsMgr.h"
#include "TradeMgr.h"
#include "Globals.h"
#include "CommandWrap.h"
#include "SoundEvent.h"
#include "Randy.h"
#include "Strings.h"
#include "ResearchAPI.h"
#include "mainrgn.h"
#include "TaskBar.h"
#include "ShipView.h"
#include "glinc.h"
#include "glcaps.h"
#include "render.h"
#include "Sensors.h"
#include "SinglePlayer.h"
#include "Ping.h"
#include "texreg.h"
#include "FEReg.h"
#include "ShipDefs.h"
#include "NIS.h"
#include "ResearchGUI.h"
#include "Select.h"
#include "Matrix.h"
#include "Vector.h"
#include "Key.h"
#include "CameraCommand.h"
#include "SaveGame.h"
#include "InfoOverlay.h"

/*=============================================================================
    Definitions:
=============================================================================*/

extern char TM_TechListFont[64];
extern char TM_Font[64];


#ifdef _WIN32
#define TM_FIBFile              "FEMan\\Trader_Interface.fib"
#else
#define TM_FIBFile              "FEMan/Trader_Interface.fib"
#endif
#define TM_TradeScreen          "Trader_Interface"
#define TM_FontNameLength       64
#define TM_HeadingColorFactor   1.5f

//margins for printing RU's
#define TM_RUMarginRight        2
#define TM_RUMarginTop          2

//margins for available techs
#define TM_ASHeadingMarginLeft  3
#define TM_ASMarginLeft         9
#define TM_ASMarginTop          2
#define TM_ASMarginRight        2
#define TM_ASInterSpacing       2

#define TM_VertSpacing          (fontHeight(" ") >> 1)
#define TM_HorzSpacing          (fontWidth(" "))
#define TM_InfoWidth            (region->rect.x1 - region->rect.x0)

//parameter defaults
extern color TM_SelectionTextColor;
extern color TM_SelectionRectColor;
extern color TM_StandardTextColor;
extern color TM_CantAffordTextColor;

#define TM_TEXTURE_INSET        3

// defines for techavailable status word

#define ITEM_CLASS          0
#define ITEM_TECH           1

#define STAT_CANBUILD       0
#define STAT_CANTBUILD      1
#define STAT_PRINT          2
#define STAT_DONTPRINT      3

// frame count for red flash when tech limits exceeded
#define CAPS_REDFRAMECOUNT 5;

//margins for printing construction progress
//...
#define TM_SHIP_IMAGE_INSET     4

/*=============================================================================
    Data:
=============================================================================*/

bool tmCheapTechs = TRUE;

bool tmTradeDisabled = FALSE;
bool tmTradeActive = FALSE;

char        oldtline[650], tline[650];

//callback tables for this screen
void tmLeave(char *string, featom *atom);
void tmAcceptOffer(char *string, featom *atom);
void tmNumberRUsDraw(featom *atom, regionhandle region);
void tmTechCostsDraw(featom *atom, regionhandle region);

void tmTechListDraw(featom *atom, regionhandle region);
void tmTechImageDraw(featom *atom, regionhandle region);

void tmDirtyTechInfo(void);
void tmDialog(char *string, featom *atom);

fecallback tmCallback[] =
{
    {tmLeave,                   "TM_Leave"      },
    {tmAcceptOffer,             "TM_AcceptOffer" },
    {NULL,                      NULL            }
};

fedrawcallback tmDrawCallback[] =
{
    {tmNumberRUsDraw,           "TM_XRUs"                },

    {tmTechListDraw,            "TM_TechList"           },
    {tmTechInfoDraw,            "TM_TechInfo"           },
    {tmTechImageDraw,           "TM_TechImage"           },
    {tmDialogDraw,              "TM_Dialog"             },
    {tmCostListDraw,            "TM_CostList"           },
    {NULL,                      NULL                    }
};

char *getWord(char *dest, char *source);

sdword tmDrawTextBlock(char *s, sdword x, sdword y, sdword width, sdword height, color c);

//flag indicating loaded status of this screen
fibfileheader *tmScreensHandle = NULL;

// defines for the pictures LRM caching
#define TM_TOTALPICS            6

sdword tmCurIndex=-1;
TechnologyType  tmtechinfo=-1;

regionhandle tmBaseRegion        = NULL;
regionhandle tmTechInfoRegion    = NULL;
regionhandle tmNumberRUsRegion   = NULL;
regionhandle tmDialogRegion    = NULL;
regionhandle tmTechListRegion  = NULL;
regionhandle tmTechImageRegion  = NULL;
regionhandle tmCostListRegion   = NULL;


sword tmDialogPhrase = 0;      //what the traders say
sword tmStuffToBuy = -1;

sword tmPriceScale = 100;

//techs available for purchase

extern TechNames *TechImageNames;

sword tmTechSelected = -1;

    //for tmTechForSale[]
#define TM_TECH_IS_NOT_FOR_SALE 0
#define TM_TECH_IS_FOR_SALE 1
#define TM_TECH_IS_ALREADY_OWNED 2

    //for tmTechResearchable[]
#define TM_TECH_IS_NOT_RESEARCHABLE 0
#define TM_TECH_IS_RESEARCHABLE 1

wkTradeType wkTradeShips[WK_MAX_SHIPS];
wkTradeType *wkTradeControlShip;
bool wkTradeStuffActive;

extern char* TechImagePaths[];

udword tmTechTexture[MAX_RACES][3] =
{
    {TR_InvalidInternalHandle, TR_InvalidInternalHandle,TR_InvalidInternalHandle},
    {TR_InvalidInternalHandle, TR_InvalidInternalHandle,TR_InvalidInternalHandle}
};

lifheader *tmTechImage[MAX_RACES][3] =
{
    { NULL, NULL, NULL },
    { NULL, NULL, NULL }
};

sdword tmCurrentSelect = 0;

//font for the tech listing
fonthandle tmTechListFont = 0;
fonthandle tmFont;

// image variables for technology bitmap
udword          tmCurTexture   = TR_InvalidInternalHandle;
lifheader      *tmCurTechImage = NULL;
TechnologyType  tmCurTechTexture = -1;
udword          tmLabTexture[MAX_RACES] = { TR_InvalidInternalHandle, TR_InvalidInternalHandle };
lifheader      *tmLabImage[MAX_RACES] = { NULL, NULL };

bool tmIoSaveState;

/*=============================================================================
    Data to save:
=============================================================================*/

sword tmTechResearchable[TM_NUM_TECHS];
udword tmTechForSale[TM_NUM_TECHS];

sdword tmTechPrice[TM_NUM_TECHS] =
{
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100
};

#define NUM_KAS_DIALOGS     6
char *tmKASDialog[NUM_KAS_DIALOGS] = {
    NULL,NULL,NULL,NULL,NULL,NULL
};

/*=============================================================================
    Private functions:
=============================================================================*/


/*-----------------------------------------------------------------------------
    Name        : tmClearTechs
    Description : clears arrays in beginning of game
    Inputs      :
    Outputs     :
    Return      :

----------------------------------------------------------------------------*/
void tmClearTechs(void)
{
    sdword i;

    for (i=0; i<TM_NUM_TECHS; i++)
    {
        tmTechForSale[i] = TM_TECH_IS_NOT_FOR_SALE;
        tmTechResearchable[i] = TM_TECH_IS_NOT_RESEARCHABLE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : tmTechTexturePrepare
    Description : prepares texture for use in the trade manager.
    Inputs      : index into pictures structure.
    Outputs     : none
    Return      : void

----------------------------------------------------------------------------*/
void tmTechTexturePrepare(sdword index)
{
    if (pictures[index].techTexture == TR_InvalidInternalHandle)
    {
        pictures[index].techTexture = trPalettedTextureCreate(pictures[index].techImage->data, pictures[index].techImage->palette, pictures[index].techImage->width, pictures[index].techImage->height);
        tmCurTexture = pictures[index].techTexture;
        tmCurIndex = index;
    }
    else
    {
        trPalettedTextureMakeCurrent(pictures[index].techTexture, pictures[index].techImage->palette);
        tmCurTexture = pictures[index].techTexture;
        tmCurIndex = index;
    }
}


void tmDirtyTechInfo(void)
{
    if (tmTechInfoRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(tmTechInfoRegion);
#endif
        tmTechInfoRegion->status |= RSF_DrawThisFrame;
    }

    if (tmTechImageRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(tmTechImageRegion);
#endif
        tmTechImageRegion->status |= RSF_DrawThisFrame;
    }

    if (tmNumberRUsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(tmNumberRUsRegion);
#endif
        tmNumberRUsRegion->status |= RSF_DrawThisFrame;
    }

    if (tmDialogRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(tmDialogRegion);
#endif
        tmDialogRegion->status |= RSF_DrawThisFrame;
    }

    if (tmCostListRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(tmCostListRegion);
#endif
        tmCostListRegion->status |= RSF_DrawThisFrame;
    }

    if (tmTechListRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(tmTechListRegion);
#endif
        tmTechListRegion->status |= RSF_DrawThisFrame;
    }
}


void tmCloseIfOpen(void)
{
    if (tmBaseRegion != NULL)
    {
        tmLeave(NULL,NULL);
    }
}



/*=============================================================================
    Functions:
=============================================================================*/


/*-----------------------------------------------------------------------------
    Name        : tmLeave
    Description : Callback function to close the trade manager
    Inputs      :
    Outputs     : Deletes all regions associated with trade manager
    Return      :
----------------------------------------------------------------------------*/
void tmLeave(char *string, featom *atom)
{                                                           //close the construction manager
#if TM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nClose trade manager.");
#endif
    feScreenDeleteFlags(tmBaseRegion,FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND);
    tmBaseRegion = NULL;

    if (tmIoSaveState)
        ioEnable();

    // enable rendering of main game screen
    mrRenderMainScreen = TRUE;

    /* play the exit sound */
    soundEvent(NULL, UI_ManagerExit);
    //restart the sound of space ambient
    soundEvent(NULL, UI_SoundOfSpace);

    spUnlockout();

    bitClear(tbDisable,TBDISABLE_TRADEMGR_USE);

    tmReset();

    tmTradeActive = FALSE;

    svClose();
}

/*-----------------------------------------------------------------------------
    Name        : tmAcceptOffer
    Description : Callback function to accept the offered technologies
    Inputs      :
    Outputs     : Get the technologies
    Return      :
----------------------------------------------------------------------------*/
void tmAcceptOffer(char *string, featom *atom)
{
    udword price;

    if (tmTechSelected == -1) return;

    price = (tmTechPrice[tmTechSelected] * tmPriceScale) / 100;
#if TM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nReceived the technologies...");
#endif

    //if the player can't afford the technology selected, bugger out.
    if (price > universe.curPlayerPtr->resourceUnits)
    {
        tmDialogPhrase = DialogCantAffordThat;
        tmDirtyTechInfo();
        return;
    }

    universe.curPlayerPtr->resourceUnits -= price;

    tmTechForSale[tmTechSelected] = TM_TECH_IS_ALREADY_OWNED;


    //rmAddTechToPlayer(universe.curPlayerPtr, tmTechSelected);
    universe.curPlayerPtr->researchinfo.HasTechnology |= TechToBit(tmTechSelected);

    tmtechinfo = -1;
    tmTechSelected = -1;

    if (tmStuffToBuy)
    {
       tmDialogPhrase = DialogPurchaseMade; //"want some more?"
    }
    else
    {
       tmDialogPhrase = DialogCantAffordAnything; //"bye!"
    }

    tmDirtyTechInfo();
}


/*-----------------------------------------------------------------------------
    Name        : tmTechCostsDraw
    Description : Draw the cost of building selected techs.
    Inputs      : feflow callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void tmCostListDraw(featom *atom, regionhandle region)
{
    udword price;
    sdword x, y, index;
    rectangle *rect = &region->rect;
    bool        newline=FALSE;
    color c;
    fonthandle currentFont;
    sdword  numlines;

    currentFont = fontCurrentGet();
    fontMakeCurrent(tmTechListFont);

    tmCostListRegion = region;

    numlines = 0;


    for (index=0; index<TM_NUM_TECHS; index++)
    {
        if  (tmTechForSale[index] == TM_TECH_IS_FOR_SALE)
            newline=TRUE;
        if (newline)
        {
            newline = FALSE;
            numlines++;
        }
    }

    y = region->rect.y0 + TM_ASMarginTop;

    newline=FALSE;
    numlines=0;

    for (index=0; index < TM_NUM_TECHS; index++)
    {
        price = (tmTechPrice[index] * tmPriceScale) / 100;
        if (y + fontHeight(" ") >= region->rect.y1)
        {
            break;
        }

        if (tmTechForSale[index] == TM_TECH_IS_FOR_SALE)
        {
            if (universe.curPlayerPtr->resourceUnits < price)
            {                                                   //if this tech already selected
                c = TM_CantAffordTextColor;
            }

            else
            {
                c = TM_StandardTextColor;
            }
                        //tech fancy name
            x = rect->x1 - TM_ASMarginLeft - fontWidthf("%d", price);
            fontPrintf(x, y, c, "%d", price);

            newline = TRUE;

            tmDirtyTechInfo();
        }
        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + TM_ASInterSpacing;

        }
    }

    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : tmSelectTechType
    Description : Find what type of tech was selected
    Inputs      : reg - region clicked on
                  yClicked - vertical location clicked on.
    Outputs     :
    Return      : type of tech clicked on or -1 if none
----------------------------------------------------------------------------*/
sdword tmSelectTechType(regionhandle region, sdword yClicked)
{
    sdword index, y, numlines, startind;
    fonthandle currentFont;
    bool newline = FALSE;

    currentFont = fontMakeCurrent(tmTechListFont);

    numlines = 0;

    startind = 0;

    y = region->rect.y0 + TM_ASMarginTop;

    for (index=startind; index<TM_NUM_TECHS; index++)
    {
        if (y + fontHeight(" ") >= region->rect.y1)
        {
            break;
        }

        if  (tmTechForSale[index] == TM_TECH_IS_FOR_SALE )

        {
            newline=TRUE;
        }


        if ((yClicked < y + fontHeight(" ") + TM_ASInterSpacing)&&(newline))
        {
                return(index);
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + TM_ASInterSpacing;
        }
    }

    fontMakeCurrent(currentFont);

    return (-1);
}



/*-----------------------------------------------------------------------------
    Name        : tmSelectAvailable
    Description : Callback for selecting/deselecting techs
    Inputs      : standard region callback
    Outputs     : selects or deselects a tech for construction
    Return      :
----------------------------------------------------------------------------*/

sdword tmSelectAvailable(regionhandle region, sdword ID, udword event, udword data)
{
    sdword index;
    sdword price;

    if (multiPlayerGame)
    {
        if (!multiPlayerGameUnderWay)
        {
            return (0);
        }
    }

    index = tmSelectTechType(region, mouseCursorY());

    dbgMessagef("\nSelected %i",index);

    price = (tmTechPrice[index] * tmPriceScale) / 100;

    if ( (event == RPE_PressLeft) && (index!=-1) )
    {                                                       //left press (select/add job)
        if (universe.curPlayerPtr->resourceUnits >= price)
        {
            tmTechSelected = index;
            tmtechinfo = index;
            tmDialogPhrase = DialogFirstClick;
        }
        else
        {
           tmDialogPhrase = DialogCantAffordThat;
        }
    }
    else if ( (event == RPE_PressRight) && (index!=-1) )
    {
        tmtechinfo = index;
    }

    tmDirtyTechInfo();
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : tmTechListDraw
    Description : Draw the available technologies and whatever else
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void tmTechListDraw(featom *atom, regionhandle region)
{
    sdword x, y, index;
    color c;
    fonthandle currentFont;
    bool       newline;
    sdword     numlines, buyable=0;
    sdword     price;

    if (tmTechSelected == -1)
    {
        sdword i, price;

        for (i = 0; i < TM_NUM_TECHS; i++)
        {
            if (tmTechForSale[i] == TM_TECH_IS_FOR_SALE)
            {
                price = (tmTechPrice[i] * tmPriceScale) / 100;
                if (universe.curPlayerPtr->resourceUnits >= price)
                {
                    tmTechSelected = i;
                    tmtechinfo = i;
                    tmDirtyTechInfo();
                    break;
                }
            }
        }
    }

    tmTechListRegion = region;

    currentFont = fontMakeCurrent(tmTechListFont);

    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_PressRight |
                        RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
        regFunctionSet(region, (regionfunction)tmSelectAvailable);          //set new region handler function
    }
    feStaticRectangleDraw(region);                          //draw standard rectangle

    numlines = 0;

    for (index=0; index<TM_NUM_TECHS; index++)
    {
        if (tmTechForSale[index] == TM_TECH_IS_FOR_SALE)

        newline=TRUE;
        if (newline)
        {
            newline = FALSE;
            numlines++;
        }
    }


    y = region->rect.y0 + TM_ASMarginTop;

    newline=FALSE;
    numlines=0;

    for (index=0; index < TM_NUM_TECHS; index++)
    {
        price = (tmTechPrice[index] * tmPriceScale) / 100;

        if (y + fontHeight(" ") >= region->rect.y1)
        {
            break;
        }

        if (tmTechForSale[index] == TM_TECH_IS_FOR_SALE)
        {
            if (universe.curPlayerPtr->resourceUnits < price)
            {                                                   //if this tech already selected
                c = TM_CantAffordTextColor;
            }
            else
            {
                c = TM_StandardTextColor;
                buyable++;
            }

            if (index == tmTechSelected)
            {
                c = TM_SelectionTextColor;
            }
                        //tech fancy name
            x = region->rect.x0 + TM_ASMarginLeft;
            fontPrint(x, y, c, RaceSpecificTechTypeToNiceString(index,universe.curPlayerPtr->race));

            newline = TRUE;

            tmDirtyTechInfo();
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + TM_ASInterSpacing;
            numlines++;
        }
    }
    tmStuffToBuy = buyable;

    fontMakeCurrent(currentFont);
    //tmDirtyTechInfo();
}

/*-----------------------------------------------------------------------------
    Name        : tmNumberRUsDraw
    Description : Callback to draw number of RU's available
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tmNumberRUsDraw(featom *atom, regionhandle region)
{
    sdword width;
    fonthandle oldfont;
    rectangle rect = region->rect;
    tmNumberRUsRegion = region;

    oldfont = fontMakeCurrent(tmTechListFont);
    width = fontWidthf("%d", universe.curPlayerPtr->resourceUnits);//width of number

    primModeSet2();
    primRectSolid2(&rect, colRGB(0, 0, 0));

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x0, MAIN_WindowHeight - rect.y1, rect.x1 - rect.x0, rect.y1 - rect.y0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - TM_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", universe.curPlayerPtr->resourceUnits);

    fontMakeCurrent(oldfont);
}


void tmDialogDraw(featom *atom, regionhandle region)
{
    sdword      x,y, width;
    char       *pos, *oldpos;
    char        stringtoprint[650];
    bool        justified, done;
    fonthandle oldfont;

    char tmKASMissing[] = "Hello there, fellow space travellers!  Until somebdoy gives me some new lines in KAS, that is all I can say.";

    tmDialogRegion = region;

    oldfont = fontMakeCurrent(tmFont);

    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop

    x = region->rect.x0 + 15;
    y = region->rect.y0 + 5 ;

    if (tmKASDialog[tmDialogPhrase])
        strcpy(stringtoprint,tmKASDialog[tmDialogPhrase]);
    else
        strcpy(stringtoprint,tmKASMissing);

    y += TM_VertSpacing + fontHeight(" ");

    pos = stringtoprint;

    done = FALSE;
    while (!done)
    {
        justified = FALSE;
        tline[0]=0;
        while (!justified)
        {
            strcpy(oldtline, tline);
            oldpos = pos;
            pos = getWord(tline, pos);

            if (pos[0] == '\n')
            {
                justified = TRUE;
                pos++;
                while ( pos[0] == ' ' ) pos++;
            }
            else
            {
                if ( (width=fontWidth(tline)) > TM_InfoWidth - 15)
                {
                    strcpy(tline, oldtline);
                    pos = oldpos;
                    while ( pos[0] == ' ' ) pos++;

                    justified = TRUE;
                }
                if (pos[0]==0)
                {
                    justified = TRUE;
                    done      = TRUE;
                }
            }
        }

        fontPrintf(x,y,TM_StandardTextColor,"%s",tline);
        y += fontHeight(" ");
        if (y > region->rect.y1 + fontHeight(" ")) done=TRUE;

    }
    fontMakeCurrent(oldfont);
}

void tmTechInfoDraw(featom *atom, regionhandle region)
{
    fonthandle  currentFont;
    sdword      x,y, width;
    char       *pos, *oldpos;
    char        stringtoprint[650];
    bool        justified, done;

    tmTechInfoRegion = region;

    feStaticRectangleDraw(region); //draw standard rectangle

    currentFont = fontMakeCurrent(tmTechListFont);

    x = region->rect.x0 + 15;
    y = region->rect.y0 + 5 ;

    if (tmtechinfo != -1)
    {
        fontPrintf(x,y,TM_SelectionTextColor,"%s",RaceSpecificTechTypeToNiceString(tmtechinfo, universe.curPlayerPtr->race));

        y += TM_VertSpacing + fontHeight(" ");

        // Bad bad design, my fault [Drew] doh!
        if (tmtechinfo==DDDFDFGFTech)
            if (universe.curPlayerPtr->race==R1)
                strcpy(stringtoprint,strGetString(strR1DDDFTechinfo));
            else
                strcpy(stringtoprint,strGetString(strR2DFGFTechinfo));
        else if (tmtechinfo==CloakDefenseFighter)
            if (universe.curPlayerPtr->race==R1)
                strcpy(stringtoprint,strGetString(strR1CloakFighterinfo));
            else
                strcpy(stringtoprint,strGetString(strR2DefenseFighterTechinfo));
        else
            if (universe.curPlayerPtr->race==R1)
                strcpy(stringtoprint,strGetString(tmtechinfo+strTechInfoOffsetR1));
            else
                strcpy(stringtoprint,strGetString(tmtechinfo+strTechInfoOffsetR2));

        pos = stringtoprint;

        done = FALSE;
        while (!done)
        {
            justified = FALSE;
            tline[0]=0;
            while (!justified)
            {
                strcpy(oldtline, tline);
                oldpos = pos;
                pos = getWord(tline, pos);

                if (pos[0] == '\n')
                {
                    justified = TRUE;
                    pos++;
                    while ( pos[0] == ' ' ) pos++;
                }
                else
                {
                    if ( (width=fontWidth(tline)) > TM_InfoWidth - 15)
                    {
                        strcpy(tline, oldtline);
                        pos = oldpos;
                        while ( pos[0] == ' ' ) pos++;

                        justified = TRUE;
                    }
                    if (pos[0]==0)
                    {
                        justified = TRUE;
                        done      = TRUE;
                    }
                }
            }

            fontPrintf(x,y,TM_StandardTextColor,"%s",tline);
            y += fontHeight(" ");
            if (y > region->rect.y1 + fontHeight(" ")) done=TRUE;
        }
    }

    fontMakeCurrent(currentFont);
}

bool tmCanBuildTechType(void)
{
    return TRUE;
}

void tmReset(void)
{
    tmBaseRegion      = NULL;
    tmTechInfoRegion  = NULL;
    tmNumberRUsRegion = NULL;
    tmDialogRegion    = NULL;
    tmTechListRegion  = NULL;
    tmTechImageRegion = NULL;
    tmCostListRegion  = NULL;

    tmTechSelected = -1;
}

/*-----------------------------------------------------------------------------
    Name        : tmStartup
    Description : Start the trade manager module.
    Inputs      : void
    Outputs     : Starts a task to take care of construction.
    Return      : void
----------------------------------------------------------------------------*/
void tmStartup(void)
{

}

/*-----------------------------------------------------------------------------
    Name        : tmShutdown
    Description : Shut down the trade manager
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void tmShutdown(void)
{
    sdword i;

    tmReset();
    for (i=0;i<NUM_KAS_DIALOGS;i++)
    {
        if (tmKASDialog[i])
        {
            memFree(tmKASDialog[i]);
            tmKASDialog[i] = NULL;
        }
    }
}

sdword tmTradeBegin(regionhandle region, sdword ID, udword event, udword data)
{
    sdword status = 0;
    sdword i;

    //if disabled, just return
    if ((tmTradeDisabled) || (playPackets) || (universePause) || (tmTradeActive))
    {
       return 0;
    }

    // disbale rendering of main screen
    mrRenderMainScreen = FALSE;

    // clear the screen
    rndClear();

    tmTradeActive = TRUE;

    tmTechListFont = frFontRegister(TM_TechListFont);
    tmFont = frFontRegister(TM_Font);

    tmCurrentSelect = 0;
    tmDialogPhrase = DialogWelcome;

    if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
    }

    if (!tmScreensHandle)
    {
        feCallbackAddMultiple(tmCallback);                  //add in the callbacks
        feDrawCallbackAddMultiple(tmDrawCallback);
        tmScreensHandle = feScreensLoad(TM_FIBFile);        //load in the screen
    }

    soundEventStopSFX(0.5f);

    /* play the intro sound */
    soundEvent(NULL, UI_ManagerIntro);
    /* play the build manager ambient */
    soundEvent(NULL, UI_Trading);

    tmBaseRegion = feScreenStart(region, TM_TradeScreen);//add new regions as siblings of current one

    tmIoSaveState = ioDisable();

    tmTechSelected = -1;
    for (i = 0; i < TM_NUM_TECHS; i++)
    {
        if (tmTechForSale[i] == TM_TECH_IS_FOR_SALE)
        {
            tmTechSelected = i;
            tmtechinfo = i;
            tmDirtyTechInfo();
            break;
        }
    }

    mouseCursorShow();

    bitSet(tbDisable,TBDISABLE_TRADEMGR_USE);

    return(status);
}


/*-----------------------------------------------------------------------------
    Name        : tmTechImageDraw
    Description : Loads in, decompresses and draws the tech picture.
    Inputs      : none
    Outputs     : loads in tmTechImage and creates tmTechTexture
    Return      : void
----------------------------------------------------------------------------*/
void tmTechImageDraw(featom *atom, regionhandle region)
{
    char      filename[128];
    sdword    index, lru;
    real32    time=(real32)1.0e22;
    rectangle textureRect;

    tmTechImageRegion = region;

    feStaticRectangleDraw(region); //draw standard rectangle

    if (tmtechinfo != -1)
    {
        if (tmtechinfo != tmCurTechTexture)
        {
            for (index=0;index<TM_TOTALPICS;index++) // RM_TOTALPICS is the size of the cache
            {
                // Find least recently used texture
                if (pictures[index].timestamp < time)
                {
                    time = pictures[index].timestamp;
                    lru  = index;
                }
                // If already cached, use it and exit this routine
                if ( (pictures[index].tech==tmtechinfo) &&
                     (pictures[index].race==universe.curPlayerPtr->race) )
                {
                    tmTechTexturePrepare(index);
                    tmCurTechTexture = tmtechinfo;
                    tmCurTechImage = pictures[lru].techImage;
                    tmCurIndex = index;
                    pictures[index].timestamp = universe.totaltimeelapsed;

                    textureRect.x0=region->rect.x0+TM_TEXTURE_INSET;
                    textureRect.y0=region->rect.y0+TM_TEXTURE_INSET;
                    textureRect.x1=region->rect.x1-TM_TEXTURE_INSET;
                    textureRect.y1=region->rect.y1-TM_TEXTURE_INSET;

                    rndPerspectiveCorrection(FALSE);
                    primRectSolidTextured2(&textureRect);

                    //if(tmExtendedInfoActive)
                    //{
                    //    primRectTranslucent2(&textureRect, colRGBA(0, 0, 0, 128));
                    //    tmTechInfoDraw(region);
                    //}
                    return;
                }
            }

            // Build filename for loading texture from file
            strcpy(filename, TechImagePaths[universe.curPlayerPtr->race]);
            strcat(filename, TechTypeToString(tmtechinfo));
            strcat(filename,".lif");

            // Remove oldest (least recently used) texture from memory
            if (pictures[lru].techImage != NULL)
            {
                memFree(pictures[lru].techImage);
                pictures[lru].techImage = NULL;
            }
            if (pictures[lru].techTexture != TR_InvalidInternalHandle)
            {
                trRGBTextureDelete(pictures[lru].techTexture);
                pictures[lru].techTexture = TR_InvalidInternalHandle;
            }

            // Load the image into LRU cache
            pictures[lru].techImage = trLIFFileLoad(filename, NonVolatile);
            dbgAssert(pictures[lru].techImage->flags & TRF_Paletted);

            tmTechTexturePrepare(lru);
            tmCurTechTexture = tmtechinfo;
            tmCurTechImage = pictures[lru].techImage;
            pictures[lru].tech = tmtechinfo;
            pictures[lru].race = universe.curPlayerPtr->race;
            pictures[lru].timestamp = universe.totaltimeelapsed;
        }
        else
        {
            trPalettedTextureMakeCurrent(pictures[tmCurIndex].techTexture,
                                         pictures[tmCurIndex].techImage->palette);
        }

        textureRect.x0=region->rect.x0+TM_TEXTURE_INSET;
        textureRect.y0=region->rect.y0+TM_TEXTURE_INSET;
        textureRect.x1=region->rect.x1-TM_TEXTURE_INSET;
        textureRect.y1=region->rect.y1-TM_TEXTURE_INSET;

        rndPerspectiveCorrection(FALSE);
        primRectSolidTextured2(&textureRect);
    }
}

bool tmFirstWordNULL(char *s)
{
    char line[100];

    line[0] = 0;
    getWord(line,s);
    if(!strcmp(line,"NULL"))
        return (TRUE);
    else
        return (FALSE);
}

sdword tmDrawTextBlock(char *s, sdword x, sdword y, sdword width, sdword height, color c)
{
    char *oldpos;
    bool justified, done;

    if(tmFirstWordNULL(s))
        return(y);
    done = FALSE;
    while (!done)
    {
        justified = FALSE;
        tline[0] = 0;
        while (!justified)
        {
            strcpy(oldtline, tline);
            oldpos = s;
            s = getWord(tline, s);

            if (s[0] == '\n')
            {
                justified = TRUE;
                s++;
                while ( s[0] == ' ' ) s++;
            }
            else
            {
                if (fontWidth(tline) > width)
                {
                    strcpy(tline, oldtline);
                    s = oldpos;
                    while ( s[0] == ' ' ) s++;

                    justified = TRUE;
                }
                if (s[0] == 0)
                {
                    justified = TRUE;
                    done      = TRUE;
                }
            }
        }

        fontPrintf(x, y, c, "%s", tline);
        y += fontHeight(" ");
        if (y > (y + height))
            done = TRUE;
    }
    y += fontHeight(" ");
    return(y);
}

/*-----------------------------------------------------------------------------
    Name        : tmTechInit
    Description : Call at the start of 1P game
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tmTechInit(void)
{
    udword index;

    for (index=0; index<TM_NUM_TECHS; index++)
    {
        tmTechForSale[index] = TM_TECH_IS_NOT_FOR_SALE;
    }
}

//KAS interface functions
// - call from scripts
// - should also be called by anyone else who updates the tech tree
// - all for single player game

extern bool rmGUIActive;
void rmUpdateTechList(void);

void AllowPlayerToResearch(char *techname)
{
    TechnologyType tech = StrToTechType(techname);

    if ((rmGUIActive) && (tmTechResearchable[tech] == FALSE))
    {
        tmTechResearchable[tech] = TRUE;
        rmUpdateTechList();
    }
    else
        tmTechResearchable[tech] = TRUE;
}

void AllowPlayerToPurchase(char *techname)
{
   TechnologyType tech = StrToTechType(techname);
   if (tmTechForSale[tech] != TM_TECH_IS_ALREADY_OWNED) tmTechForSale[tech] = TM_TECH_IS_FOR_SALE;
}

void PlayerAcquiredTechnology(char *techname)
{
   TechnologyType tech = StrToTechType(techname);
   tmTechForSale[tech] = TM_TECH_IS_ALREADY_OWNED;
   rmAddTechToPlayer(universe.curPlayerPtr, ((udword)1)<<tech);
}

sdword CanPlayerResearch(char *techname)
{
   TechnologyType tech = StrToTechType(techname);
   return (tmTechResearchable[tech]);
}

sdword CanPlayerPurchase(char *techname)
{
   TechnologyType tech = StrToTechType(techname);
   return (tmTechForSale[tech] == TM_TECH_IS_FOR_SALE);
}

sdword DoesPlayerHave(char *techname)
{
   TechnologyType tech = StrToTechType(techname);
   return (tmTechForSale[tech] == TM_TECH_IS_ALREADY_OWNED);
}

sdword GetBaseTechnologyCost(char *techname)
{
   TechnologyType tech = StrToTechType(techname);
   return (tmTechPrice[tech]);
}

void SetBaseTechnologyCost(char *techname, sdword cost)
{
   TechnologyType tech = StrToTechType(techname);
   tmTechPrice[tech] = cost;
}

//more interface functions

void tmEnableTraderGUI(void)
{
    mrTradeStuff(NULL, NULL);
}

bool tmTraderGUIActive(void)
{
    return tmTradeActive;
}

void tmSetDialog(sdword phrasenum, char *sentence)
{
    if (tmKASDialog[phrasenum])
    {
        memFree(tmKASDialog[phrasenum]);
        tmKASDialog[phrasenum] = NULL;
    }
    tmKASDialog[phrasenum] = memStringDupe(sentence);
}

void tmSetPriceScale(udword percent)
{
    tmPriceScale = percent;
}

uword tmGetPriceScale(void)
{
    return tmPriceScale;
}

void tmSetTradeDisabled(bool trade)
{
    tmTradeDisabled = trade;
}

/*=============================================================================
    Asteroids?
=============================================================================*/

void wkTradeInit(void);
void wkTradeControl(void);
void wkTradeFocusedShip(void);
void wkTradeUpdate(void);

extern real32 fcos(real32 r);
extern real32 fsin(real32 r);

void mrTradeStuffTest(sdword *a, sdword *b)
{
    sdword i;
    Ship *ship;

    vector vect = {0.0f, 0.0f, 1.0f};
    vector vect2 = {0.0f, 1.0f, 0.0f};
    vector vect3 = {1.0f, 0.0f, 0.0f};

    wkTradeInit();
    wkTradeFocusedShip();
    wkTradeStuffActive = TRUE;

    return;

    for(i=0; i<selSelected.numShips; ++i)
    {
        ship = selSelected.ShipPtr[i];

        switch (selSelected.numShips)
        {
        case 1:
            ship->posinfo.position.x += 200.0f;
            break;

        case 2:
            ship->posinfo.velocity.x += 1000.0f;
            break;

        case 3:
            ship->posinfo.force.x += 1000.0f;
            break;

        case 4:
            ship->rotinfo.rotspeed.x += 1000.0f;
            break;

        case 5:
            ship->rotinfo.torque.x += 100000.0f;
            break;

        case 6:
            matCreateCoordSysFromHeading(&ship->rotinfo.coordsys,&vect);
            break;

        case 7:
            matCreateCoordSysFromHeading(&ship->rotinfo.coordsys,&vect2);
            break;

        case 8:
            matCreateCoordSysFromHeading(&ship->rotinfo.coordsys,&vect3);
            break;
        }
    }
}

#define WK_ANGULAR_ACC 0.015f
#define WK_ANGULAR_MAXVEL 0.10f
#define WK_LINEAR_ACC 2.0f
#define WK_LINEAR_REVACC 1.0f
#define WK_LINEAR_MAXVEL 60.0f
#define WK_STRAFE_ACC 1.0f

#define WK_VEL_SCALE 240
#define WK_MASS_SCALE 8.0f


void wkTradeInit(void)
{
    sdword index;
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    real32 mass;

    index = 0;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        wkTradeShips[index].ship = ship;
        mass = ship->staticinfo->staticheader.mass / WK_MASS_SCALE;


        wkTradeShips[index].x = 0.0f;
        wkTradeShips[index].y = 0.0f;
        wkTradeShips[index].vx = 0.0f;
        wkTradeShips[index].vy = 0.0f;
        wkTradeShips[index].ang = 0.0f;
        wkTradeShips[index].vang = 0.0f;
        wkTradeShips[index].vangacc = WK_ANGULAR_ACC / fsqrt(mass);
        wkTradeShips[index].vangmax = WK_ANGULAR_MAXVEL / fsqrt(fsqrt(mass));
        wkTradeShips[index].acc = WK_LINEAR_ACC / fsqrt(mass);
        wkTradeShips[index].maxvel = WK_LINEAR_MAXVEL;
        wkTradeShips[index].revacc = WK_LINEAR_REVACC / fsqrt(mass);
        wkTradeShips[index].strafeacc = WK_STRAFE_ACC / fsqrt(mass);
        wkTradeShips[index].controlthrust = 0;
        wkTradeShips[index].controlrot = 0;
        wkTradeShips[index].controlstrafe = 0;
        wkTradeShips[index].controlfire = 0;

        index++;
        objnode = objnode->next;
    }
    wkTradeShips[index].ship = NULL;

}


void wkTradeControl(void)
{
    if (keyIsHit(ARRRIGHT))
    {
        if (keyIsHit(ALTKEY))
        {
            wkTradeControlShip->controlstrafe = 1;
            wkTradeControlShip->controlrot = 0;
        }
        else
        {
            wkTradeControlShip->controlstrafe = 0;
            wkTradeControlShip->controlrot = -1;
        }

    } else if (keyIsHit(ARRLEFT))
    {
        if (keyIsHit(ALTKEY))
        {
            wkTradeControlShip->controlstrafe = -1;
            wkTradeControlShip->controlrot = 0;
        }
        else
        {
            wkTradeControlShip->controlstrafe = 0;
            wkTradeControlShip->controlrot = 1;
        }
    }
    else
    {
        wkTradeControlShip->controlstrafe = 0;
        wkTradeControlShip->controlrot = 0;
    }

    if (keyIsHit(ARRUP))
    {
        wkTradeControlShip->controlthrust = 1;
    }
    else if (keyIsHit(ARRDOWN))
    {
        wkTradeControlShip->controlthrust = -1;
    }
    else
    {
        wkTradeControlShip->controlthrust = 0;
    }

    if (keyIsHit(SHIFTKEY))
    {
        wkTradeControlShip->controlfire = 1;
    }
    else
    {
        wkTradeControlShip->controlfire = 0;
    }
}


void wkTradeFocusedShip(void)
{
    sword index;
    Ship *ship;

    if (selSelected.numShips>0)
    {
        ship = selSelected.ShipPtr[0];
    }

    for (index=0; wkTradeShips[index].ship!=NULL; index++)
    {
        if (ship == wkTradeShips[index].ship)
        {
            wkTradeControlShip = &wkTradeShips[index];
            wkTradeControlShip->x = ship->posinfo.position.x;
            wkTradeControlShip->y = ship->posinfo.position.y;

            return;
        }
    }
    wkTradeControlShip = NULL;
}

void wkTradeUpdate(void)
{
    Ship *ship;
    Resource *resource;
    wkTradeType *trader;
    real32 vel;
    vector vect;
    //sword index;
    Node *objnode;
    objnode = universe.ShipList.head;

    if (wkTradeControlShip)
    {
        wkTradeControl();

        trader = wkTradeControlShip;
        ship = trader->ship;

        //dbgAssert(ship->objtype == OBJ_ShipType);
        if (ship->objtype != OBJ_ShipType)
        {
            //ship has exploded
            wkTradeControlShip = NULL;
        }

        if (trader->controlthrust)
        {
            if (trader->controlthrust > 0)
            {
                trader->vx += fcos(trader->ang)*trader->acc;
                trader->vy += fsin(trader->ang)*trader->acc;
            }
            else
            {
                trader->vx -= fcos(trader->ang)*trader->revacc;
                trader->vy -= fsin(trader->ang)*trader->revacc;
            }
        }

        if (trader->controlstrafe)
        {
            trader->vx += fsin(trader->ang)
                *trader->acc*trader->controlstrafe;
            trader->vy += fcos(trader->ang)
                *trader->acc*trader->controlstrafe;

        }

        if (trader->controlfire)
        {
            Ship *ship = trader->ship;
            ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
            if (shipstatic->custshipheader.CustShipFire != NULL)
            {
                shipstatic->custshipheader.CustShipFire(ship,NULL);
            }
        }

        vel = fsqrt(trader->vx*trader->vx +
                    trader->vy*trader->vy);

        if (vel > trader->maxvel)
        {
            trader->vx *= trader->maxvel/vel;
            trader->vy *= trader->maxvel/vel;

        }

        trader->x += trader->vx;
        trader->y += trader->vy;

        trader->ang += trader->vang;

        if (trader->controlrot)
        {
            trader->vang += trader->controlrot*trader->vangacc;
            if (ABS(trader->vang) > trader->vangmax)
            {
                if (trader->vang>0)
                    trader->vang = trader->vangmax;
                else
                    trader->vang = -trader->vangmax;
            }
        }
        else
        {
            if (ABS(trader->vang) < trader->vangacc)
            {
                trader->vang = 0.0f;
            }
            else
            {
                if (trader->vang > 0)
                    trader->vang -= trader->vangacc;
                else
                    trader->vang += trader->vangacc;
            }
        }

        ship->posinfo.position.x = trader->x;
        ship->posinfo.position.y = trader->y;
        ship->posinfo.position.z = 0.0f;

        ship->posinfo.velocity.x = trader->vx*WK_VEL_SCALE;
        ship->posinfo.velocity.y = trader->vy*WK_VEL_SCALE;

        vect.x = fcos(trader->ang);
        vect.y = fsin(trader->ang);
        vect.z = 0;
        matCreateCoordSysFromHeading(&ship->rotinfo.coordsys,&vect);
    }

    //now update all ships
    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        ship->posinfo.position.z = 0.0f;

        objnode = objnode->next;
    }

    //resources
    objnode = universe.ResourceList.head;
    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        dbgAssert(resource->flags & SOF_Resource);

        resource->posinfo.position.z = 0.0f;

        objnode = objnode->next;
    }
}

/*=============================================================================
    Save Game Stuff here
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : tmSave
    Description : Saves TradeMgr stuff
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tmSave(void)
{
    sdword i;

    SaveStructureOfSize(tmTechResearchable,sizeof(tmTechResearchable[0])*TM_NUM_TECHS);
    SaveStructureOfSize(tmTechForSale, sizeof(tmTechForSale[0])*TM_NUM_TECHS);
    SaveStructureOfSize(tmTechPrice, sizeof(tmTechPrice[0])*TM_NUM_TECHS);

    for (i=0;i<NUM_KAS_DIALOGS;i++)
    {
        Save_String(tmKASDialog[i]);
    }

    SaveInfoNumber(tmTradeDisabled);
}

/*-----------------------------------------------------------------------------
    Name        : tmLoad
    Description : Loads TradeMgr stuff
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tmLoad(void)
{
    sdword i;

    LoadStructureOfSizeToAddress(tmTechResearchable,sizeof(tmTechResearchable[0])*TM_NUM_TECHS);
    LoadStructureOfSizeToAddress(tmTechForSale,sizeof(tmTechForSale[0])*TM_NUM_TECHS);
    LoadStructureOfSizeToAddress(tmTechPrice,sizeof(tmTechPrice[0])*TM_NUM_TECHS);

    for (i=0;i<NUM_KAS_DIALOGS;i++)
    {
        tmKASDialog[i] = Load_String();
    }

    tmTradeDisabled = LoadInfoNumber();
}


