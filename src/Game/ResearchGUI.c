/*=============================================================================
    Name    : researchgui.c
    Purpose : logic for the research manager

    Created 5/27/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#ifdef _WIN32
#include <windows.h>
#endif
#endif
#include <stdio.h>
#include <math.h>
#include "glinc.h"
#include "main.h"
#include "Types.h"
#include "Universe.h"
#include "Region.h"
#include "UIControls.h"
#include "FEFlow.h"
#include "font.h"
#include "FontReg.h"
#include "ObjTypes.h"
#include "texreg.h"
#include "Task.h"
#include "mouse.h"
#include "CommandLayer.h"
#include "PiePlate.h"
#include "Globals.h"
#include "CommandWrap.h"
#include "Scroller.h"
#include "SoundEvent.h"
#include "Randy.h"
#include "mainrgn.h"
#include "ShipDefs.h"
#include "prim2d.h"
#include "ResearchAPI.h"
#include "ResearchGUI.h"
#include "Strings.h"
#include "FEReg.h"
#include "TaskBar.h"
#include "render.h"
#include "fixed.h"
#include "SinglePlayer.h"
#include "Tutor.h"
#include "TradeMgr.h"
#include "MultiplayerGame.h"
#include "FEColour.h"
#include "glcompat.h"
#include "InfoOverlay.h"
#include "CommandNetwork.h"

/*=============================================================================
    Defines:
=============================================================================*/

#define RM_LabIcon              "Research_icon.lif"


// defines for spacing
#define RM_VertSpacing          (fontHeight(" ") >> 1)
#define RM_HorzSpacing          (fontWidth(" "))
#define RM_InfoWidth            (region->rect.x1 - region->rect.x0)

// defines for the headings in the tech list box
#define WeaponHeading           0
#define ConstructionHeading     1
#define InterdictionHeading     2
#define SupportHeading          3


// defines for the buttons sizeing
#define LB_BUTTONSIZE           45
#define LB_HALFBUTTON           (LB_BUTTONSIZE/2)
#define LB_BUTTONSPACE          6
#define LB_BUTTONWIDTH          25

// defines for the pregeneration of the circle data
#define CIRCLE_POINTS           150
#define CIRCLE_XRADIUS          0.06875
#define CIRCLE_YRADIUS          0.09166666666667

// defines for the pulse effect
#define PULSE_INC               2048
#define FIXED16_ONE             65536
#define PULSE_COLOR             colRGB(200, 200, 200)

// defines for the pictures LRM caching

#define RM_DSINIT               0
#define RM_DSNORMAL             1
#define RM_DSDRAW               2
#define RM_DSERASE              3

#define RM_MARQUEERAD           3
#define RM_NUMMARQUEE           3
#define RM_MARQUEESPACE         7
#define RM_MARQUEE_Y_SPACE      18
#define RM_MARQUEE_HEIGHT       (LB_BUTTONSIZE - (RM_MARQUEE_Y_SPACE * 2))
#define RM_MARQUEE_HALF_HEIGHT  (RM_MARQUEE_HEIGHT / 2)
#define RM_MARQUEERATE          3.0
#define RM_TEXTURE_INSET        3
#define RM_LAB_INSET            5

#define NUMTECHSINLIST          34

/*=============================================================================
    Data:
=============================================================================*/

static sdword rmRenderEverythingCounter;

static featom* _rmTechAtom;
static regionhandle _rmTechRegion;
static bool _rmTechRender = FALSE;

double chop_temp;

bool rmPaletted;

fonthandle rmTechListFont=0;
fonthandle rmTechInfoFont=0;
char rmTechListFontName [RM_FontNameLength] = RM_TechListFont;
char rmTechInfoFontName [RM_FontNameLength] = RM_TechInfoFont;

//flag indicating loaded status of this screen
fibfileheader *rmScreensHandle = NULL;

regionhandle  rmTechBriefRegion = NULL;
regionhandle  rmTechImageRegion = NULL;
regionhandle  rmTechListRegion  = NULL;
regionhandle  rmBaseRegion      = NULL;

regionhandle rmLab1Region = NULL;
regionhandle rmLab2Region = NULL;
regionhandle rmLab3Region = NULL;
regionhandle rmLab4Region = NULL;
regionhandle rmLab5Region = NULL;
regionhandle rmLab6Region = NULL;

regionhandle rmConnectorRegion = NULL;

regionhandle rmLabConnector1Region = NULL;
regionhandle rmLabConnector2Region = NULL;
regionhandle rmLabConnector3Region = NULL;
regionhandle rmLabConnector4Region = NULL;
regionhandle rmLabConnector5Region = NULL;
regionhandle rmLabConnector6Region = NULL;

listwindowhandle rmTechListWindowHandle = NULL;

// callbacks for ui elements

void rmSelectAll(char *string, featom *atom);
void rmClearAllLabs(char *string, featom *atom);
void rmClearSelectedLab(char *string, featom *atom);
void rmResearchItem(char *string, featom *atom);
void rmExitMenu(char *string, featom *atom);
void rmTechListWindow(char *string, featom *atom);
void rmExtendedInfo(char *string, featom *atom);

// callbacks for user draw areas

void rmLab1Draw(featom *atom, regionhandle region);
void rmLab2Draw(featom *atom, regionhandle region);
void rmLab3Draw(featom *atom, regionhandle region);
void rmLab4Draw(featom *atom, regionhandle region);
void rmLab5Draw(featom *atom, regionhandle region);
void rmLab6Draw(featom *atom, regionhandle region);
void rmLabConnector1Draw(featom *atom, regionhandle region);
void rmLabConnector2Draw(featom *atom, regionhandle region);
void rmLabConnector3Draw(featom *atom, regionhandle region);
void rmLabConnector4Draw(featom *atom, regionhandle region);
void rmLabConnector5Draw(featom *atom, regionhandle region);
void rmLabConnector6Draw(featom *atom, regionhandle region);
void rmConnectorDraw(featom *atom, regionhandle region);
void rmTechBriefDraw(featom *atom, regionhandle region);
void rmTechImageDraw(featom *atom, regionhandle region);

fecallback rmCallback[] =
{
    {rmSelectAll            ,   "RM_SelectAll"          },
    {rmClearAllLabs         ,   "RM_ClearAllLabs"       },
    {rmClearSelectedLab     ,   "RM_ClearLab"           },
    {rmResearchItem         ,   "RM_ResearchItem"       },
    {rmExitMenu             ,   "RM_ExitMenu"           },
    {rmTechListWindow       ,   "RM_TechListWindow"     },
    {rmExtendedInfo         ,   "RM_ExtendedInfo"       },
    {NULL,                      NULL                    }
};

fedrawcallback rmDrawCallback[] =
{
    {rmLab1Draw,                "RM_Lab1Draw"           },
    {rmLab2Draw,                "RM_Lab2Draw"           },
    {rmLab3Draw,                "RM_Lab3Draw"           },
    {rmLab4Draw,                "RM_Lab4Draw"           },
    {rmLab5Draw,                "RM_Lab5Draw"           },
    {rmLab6Draw,                "RM_Lab6Draw"           },
    {rmLabConnector1Draw,       "RM_LabConnector1Draw"  },
    {rmLabConnector2Draw,       "RM_LabConnector2Draw"  },
    {rmLabConnector3Draw,       "RM_LabConnector3Draw"  },
    {rmLabConnector4Draw,       "RM_LabConnector4Draw"  },
    {rmLabConnector5Draw,       "RM_LabConnector5Draw"  },
    {rmLabConnector6Draw,       "RM_LabConnector6Draw"  },
    {rmConnectorDraw,           "RM_ConnectorDraw"      },
    {rmTechBriefDraw,           "RM_TechBriefDraw"       },
    {rmTechImageDraw,           "RM_TechImageDraw"      },
    {NULL,                      NULL                    }
};


TechPrintList PlayerTechListStatic[NUMTECHSINLIST] =
{
    {ITEM_CLASSHEADER   ,   0               ,   WeaponHeading           },

    {ITEM_TECHNOLOGY    ,   0               ,   MassDrive1Kt            },
    {ITEM_TECHNOLOGY    ,   0               ,   Chassis1                },
    {ITEM_TECHNOLOGY    ,   0               ,   FireControl             },
    {ITEM_TECHNOLOGY    ,   0               ,   CoolingSystems          },
    {ITEM_TECHNOLOGY    ,   0               ,   PlasmaWeapons           },
    {ITEM_TECHNOLOGY    ,   0               ,   AdvancedCoolingSystems  },
    {ITEM_TECHNOLOGY    ,   0               ,   AdvancedFireControl     },
    {ITEM_TECHNOLOGY    ,   0               ,   CloakDefenseFighter     },

    {ITEM_CLASSHEADER   ,   0               ,   ConstructionHeading     },

    {ITEM_TECHNOLOGY    ,   0               ,   MassDrive10Kt           },
    {ITEM_TECHNOLOGY    ,   0               ,   Chassis2                },
    {ITEM_TECHNOLOGY    ,   0               ,   MediumGuns              },
    {ITEM_TECHNOLOGY    ,   0               ,   TargetingSystems        },
    {ITEM_TECHNOLOGY    ,   0               ,   MineLayerTech           },
    {ITEM_TECHNOLOGY    ,   0               ,   NewAlloys               },

    {ITEM_CLASSHEADER   ,   0               ,   InterdictionHeading     },

    {ITEM_TECHNOLOGY    ,   0               ,   MassDrive100Kt          },
    {ITEM_TECHNOLOGY    ,   0               ,   Chassis3                },
    {ITEM_TECHNOLOGY    ,   0               ,   IonWeapons              },
    {ITEM_TECHNOLOGY    ,   0               ,   MissileWeapons          },

    {ITEM_TECHNOLOGY    ,   0               ,   DDDFDFGFTech            },
    {ITEM_TECHNOLOGY    ,   0               ,   MassDrive1Mt            },
    {ITEM_TECHNOLOGY    ,   0               ,   RepairTech              },

    {ITEM_TECHNOLOGY    ,   0               ,   HeavyGuns               },

    {ITEM_CLASSHEADER   ,   0               ,   SupportHeading          },
    {ITEM_TECHNOLOGY    ,   0               ,   ProximityDetector       },
    {ITEM_TECHNOLOGY    ,   0               ,   SupportRefuelTech       },
    {ITEM_TECHNOLOGY    ,   0               ,   AdvanceTacticalSupport  },
    {ITEM_TECHNOLOGY    ,   0               ,   ConstructionTech        },
    {ITEM_TECHNOLOGY    ,   0               ,   SensorsArrayTech        },
    {ITEM_TECHNOLOGY    ,   0               ,   GravityWellGeneratorTech},
    {ITEM_TECHNOLOGY    ,   0               ,   CloakGeneratorTech      },

    {ITEM_ENDLIST       ,   0               ,   0                       }
};


TechPrintList SinglePlayerR1TechList[NUMTECHSINLIST];
TechPrintList SinglePlayerR2TechList[NUMTECHSINLIST];

TechPrintList MultiPlayerR1TechList[NUMTECHSINLIST];
TechPrintList MultiPlayerR2TechList[NUMTECHSINLIST];

TechPrintList MultiPlayerCR1TechList[NUMTECHSINLIST];
TechPrintList MultiPlayerCR2TechList[NUMTECHSINLIST];

TechNames TechImageNames[]=
{
    {"NewAlloys.lif"                },
    {"MassDrive1Kt.lif"             },
    {"CoolingSystems.lif"           },
    {"CloakDefenseFighter.lif"      },
    {"TargetingSystems.lif"         },
    {"PlasmaWeapons.lif"            },
    {"Chassis1.lif"                 },
    {"MassDrive10Kt.lif"            },
    {"MediumGuns.lif"               },
    {"MineLayerTech.lif"            },
    {"Chassis2.lif"                 },
    {"AdvancedCoolingSystems.lif"   },
    {"MassDrive100Kt.lif"           },
    {"FireControl.lif"              },
    {"SupportRefuelTech.lif"        },
    {"AdvanceTacticalSupport.lif"   },
    {"IonWeapons.lif"               },
    {"DDDFDFGFTech.lif"             },
    {"Chassis3.lif"                 },
    {"MassDrive1Mt.lif"             },
    {"AdvancedFireControl.lif"      },
    {"MissileWeapons.lif"           },
    {"ConstructionTech.lif"         },
    {"HeavyGuns.lif"                },
    {"ProximityDetector.lif"        },
    {"SensorsArrayTech.lif"         },
    {"GravityWellGeneratorTech.lif" },
    {"CloakGeneratorTech.lif"       },
    {"RepairTech.lif"               },
    {"SalvageTech.lif"              },
    {NULL                           }
};

char* TechImagePaths[]=
{
#ifdef _WIN32
    "ResearchGUI\\Race1\\",
    "ResearchGUI\\Race2\\",
#else
    "ResearchGUI/Race1/",
    "ResearchGUI/Race2/",
#endif
    NULL
};

TechPrintList *PlayerTechList;

//colors for printing selections and stuff

color rmSelectionTextColor     = RM_SelectionTextColor;
color rmResearchingTextColor   = RM_ResearchingTextColor;
color rmCantResearchTextColor  = RM_CantResearchTextColor;
color rmStandardTextColor      = RM_StandardTextColor;
color rmClassHeadingTextColor  = RM_ClassHeadingTextColor;
color rmProgressToGoColor      = RM_ProgressToGoColor;
color rmProgressDoneColor0     = RM_ProgressDoneColor0;
color rmProgressDoneColor1     = RM_ProgressDoneColor1;
color rmLabActiveColor         = RM_LabActiveColor;
color rmPulseColor             = RM_PulseColor;
color rmNoResearchItemColor    = RM_NoResearchItemColor;
color rmMarqueeOnColor         = RM_MarqueeOnColor;
color rmMarqueeSemiOnColor     = RM_MarqueeSemiOnColor;
color rmMarqueeOffColor        = RM_MarqueeOffColor;

sdword rmCurIndex=-1;
TechnologyType  techinfo=-1;

// is TRUE if GUI is active FALSE otherwise.
bool rmGUIActive=FALSE;

// List of lab buttons
LabPrintList labbuttons[NUM_RESEARCHLABS];

// image variables for technology bitmap
LRUPicture      pictures[RM_TOTALPICS];
udword          rmCurTexture   = TR_InvalidInternalHandle;
lifheader      *rmCurTechImage = NULL;
TechnologyType  rmCurTechTexture = -1;
sdword          drawstate;
udword          rmLabTexture[MAX_RACES] = { TR_InvalidInternalHandle, TR_InvalidInternalHandle };
lifheader      *rmLabImage[MAX_RACES] = { NULL, NULL };

// variable for marque light position
real32          marqueetime=0.0;
real32          marqueepos=0;

bool rmExtendedInfoActive = FALSE;

bool rmIoSaveState;

uword researchingthistopic;
bool multipleresearchersselected;
ResearchTopic *multipleresearchtopic;
/*=============================================================================
    Function Prototypes:
=============================================================================*/

void   rmUpdateTechList(void);
sdword rmSelectAvailableTech(regionhandle region, sdword ID, udword event, udword data);
sdword rmSelectTechType(regionhandle region, sdword yClicked);
void   SelectTechWindow(void);

/*=============================================================================
    Functions:
=============================================================================*/

void rmDirtyTechInfo()
{
    if (rmTechBriefRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmTechBriefRegion);
#endif
        rmTechBriefRegion->status |= RSF_DrawThisFrame;
    }
    if (rmTechImageRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmTechImageRegion);
#endif
        rmTechImageRegion->status |= RSF_DrawThisFrame;
    }
    if (rmConnectorRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmConnectorRegion);
#endif
        rmConnectorRegion->status |= RSF_DrawThisFrame;
    }
}

void rmDirtyTechList()
{
    if (rmTechListRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmTechListRegion);
#endif
        rmTechListRegion->status |= RSF_DrawThisFrame;
    }

    if (rmLab1Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab1Region);
#endif
        rmLab1Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab2Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab2Region);
#endif
        rmLab2Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab3Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab3Region);
#endif
        rmLab3Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab4Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab4Region);
#endif
        rmLab4Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab5Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab5Region);
#endif
        rmLab5Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab6Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab6Region);
#endif
        rmLab6Region->status |= RSF_DrawThisFrame;
    }

    if (rmLabConnector1Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector1Region);
#endif
        rmLabConnector1Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector2Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector2Region);
#endif
        rmLabConnector2Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector3Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector3Region);
#endif
        rmLabConnector3Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector4Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector4Region);
#endif
        rmLabConnector4Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector5Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector5Region);
#endif
        rmLabConnector5Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector6Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector6Region);
#endif
        rmLabConnector6Region->status |= RSF_DrawThisFrame;
    }
}

void rmSelectAll(char *string, featom *atom)
{
    sdword index;

    for (index=0; index<NUM_RESEARCHLABS; index++)
    {
        if (labbuttons[index].lab->labstatus==LS_NORESEARCHITEM)
        {
            labbuttons[index].selected = TRUE;
        }
    }
}

// This function is called when a technology has just finished being researched.  This is so the technology list window
// can be updated.  The lab highlight border is turned off.
void rmClearLab(sdword labindex)
{
/*    rmUpdateTechList();

    // Regenerate list because something has just finished being researched
    uicListCleanUp(rmTechListWindowHandle);
    for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
    {
        if ( (PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
             (PlayerTechList[index].itemstat != STAT_CANTRESEARCH) )
        {
            if (PlayerTechList[index].itemtype == ITEM_CLASSHEADER)
                uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], 0, UICLW_AddToTail);
            else
                uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], UICLI_CanSelect, UICLW_AddToTail);
        }
    }
    rmTechListWindowHandle->CurLineSelected = NULL;

    SelectTechWindow();*/

    labbuttons[labindex].selected = FALSE;
}

void rmClearAllLabs(char *string, featom *atom)
{
    sdword index;

    for (index=0; index<NUM_RESEARCHLABS; index++)
    {
        rmClearResearchlab(universe.curPlayerPtr,index);
        labbuttons[index].selected = FALSE;
    }
}


void rmClearSelectedLab(char *string, featom *atom)
{
    sdword index;
   bool halted = FALSE;

    for (index=0; index<NUM_RESEARCHLABS; index++)
    {
        if ((labbuttons[index].selected) &&
            (labbuttons[index].lab->labstatus==LS_RESEARCHITEM) )
        {
            if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchClearLab)
                return;

            if (!multiPlayerGame)
            {
                rmClearResearchlab(universe.curPlayerPtr,index);
            }
            else
            {
                clWrapResearch(RESEARCH_SUBCOMMAND_STOP, universe.curPlayerIndex, index, 0);
            }
            //labbuttons[index].selected = FALSE;
            halted = TRUE;
        }
    }

   if (halted)
   {
      speechEventFleet(COMM_F_Research_Stop, 0, universe.curPlayerIndex);
   }
}


void rmResearchItem(char *string, featom *atom)
{
    sdword          index;
    bool            found=FALSE;
    TechPrintList  *tech;

    if (rmTechListWindowHandle->CurLineSelected!=NULL)
    {
        tech = (TechPrintList*)rmTechListWindowHandle->CurLineSelected->data;

        if (tech->itemstat!=STAT_ALREADYHAVE)
        {
         if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchResearch)
            return;

            for (index=0; index<NUM_RESEARCHLABS; index++)
            {
                if ((labbuttons[index].selected) &&
                    (labbuttons[index].lab->labstatus==LS_NORESEARCHITEM) )
                {
                    if (!multiPlayerGame)
                    {
                        rmAssignPlayersLabToResearch(universe.curPlayerPtr,index,tech->itemID);
                    }
                    else
                    {
                        clWrapResearch(RESEARCH_SUBCOMMAND_START, universe.curPlayerIndex, index, tech->itemID);
                        labbuttons[index].lab->labstatus = LS_RESEARCHITEMSOON; // LS_NORESEARCHSHIP
                        labbuttons[index].lab->topic = NULL;
                    }

                    labbuttons[index].pulsepos = 0;
                    labbuttons[index].selected = FALSE;
                    found = TRUE;
               if (universe.curPlayerPtr->race == R1)
               {
                  speechEventFleet(COMM_F_Research_R1_Start, tech->itemID, universe.curPlayerIndex);
               }
               else if (universe.curPlayerPtr->race == R2)
               {
                  speechEventFleet(COMM_F_Research_R2_Start, tech->itemID, universe.curPlayerIndex);
               }
                }
            }
            if (found==FALSE)
            {
                index = rmFindFreeLab(universe.curPlayerPtr);
                if (index!=-1)
                {
                    if (!multiPlayerGame)
                    {
                        rmAssignPlayersLabToResearch(universe.curPlayerPtr,index,tech->itemID);
                    }
                    else
                    {
                        clWrapResearch(RESEARCH_SUBCOMMAND_START, universe.curPlayerIndex, index, tech->itemID);
                        labbuttons[index].lab->labstatus = LS_RESEARCHITEMSOON; // LS_NORESEARCHSHIP
                        labbuttons[index].lab->topic = NULL;
                    }

               if (universe.curPlayerPtr->race == R1)
               {
                  speechEventFleet(COMM_F_Research_R1_Start, tech->itemID, universe.curPlayerIndex);
               }
               else if (universe.curPlayerPtr->race == R2)
               {
                  speechEventFleet(COMM_F_Research_R2_Start, tech->itemID, universe.curPlayerIndex);
               }
                }
            }
#ifdef DEBUG_STOMP
            regVerify((regionhandle)&rmTechListWindowHandle->reg);
#endif
            rmTechListWindowHandle->reg.status |= RSF_DrawThisFrame;
        }
    }
}


void rmExitMenu(char *string, featom *atom)
{
    rmRenderEverythingCounter = 0;

    // used to be this
    //feScreenDelete(rmBaseRegion);
    // whoever this caused a memory leak, should use feCurrentScreenDelete to close the current screen
    //
    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchClose)
        return;

    feCurrentScreenDelete();
    rmBaseRegion = NULL;

    if (rmIoSaveState)
        ioEnable();

    // enable rendering of main game screen
    mrRenderMainScreen = TRUE;

    glcFullscreen(FALSE);

    /* play the exit sound */
    soundEvent(NULL, UI_ManagerExit);
    //restart the sound of space ambient
    soundEvent(NULL, UI_SoundOfSpace);

    spUnlockout();

    rmGUIActive = FALSE;

    rmCurTechTexture = -1;
    rmTechBriefRegion = NULL;
    rmTechImageRegion = NULL;
    rmTechListRegion  = NULL;

    rmLab1Region = NULL;
    rmLab2Region = NULL;
    rmLab3Region = NULL;
    rmLab4Region = NULL;
    rmLab5Region = NULL;
    rmLab6Region = NULL;

    rmConnectorRegion = NULL;

    rmLabConnector1Region = NULL;
    rmLabConnector2Region = NULL;
    rmLabConnector3Region = NULL;
    rmLabConnector4Region = NULL;
    rmLabConnector5Region = NULL;
    rmLabConnector6Region = NULL;

    // enable taskbar popup window
    tbDisable = FALSE;

    techinfo = -1;
}

void rmCloseIfOpen()
{
    if (rmBaseRegion)
    {
        rmExitMenu(NULL,NULL);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmMarqueePulse
    Description :
    Inputs      : LabPrintList & region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmMarqueePulse(LabPrintList *labprint, regionhandle region)
{
    sdword count, index;
    sdword x, y;
    rectangle *rect = &region->rect;
    primLine2(rect->x0, rect->y0, rect->x1, rect->y0, rmStandardTextColor);
    primLine2(rect->x0, rect->y1, rect->x1, rect->y1, rmStandardTextColor);

    x = rect->x0 + RM_MARQUEESPACE;
    y = rect->y0 + ((rect->y1 - rect->y0) >> 1);

    if ((labprint->selected) && (labprint->lab->labstatus==LS_RESEARCHITEM))
    {
        for (count=FAST_TO_INT(marqueepos),index=0;index<RM_NUMMARQUEE;index++)
        {
            if (count==0)
            {
                count=2;
                primCircleSolid2(x,y,RM_MARQUEERAD,20,rmMarqueeOnColor);
            }else if (count==1)
            {
                primCircleSolid2(x,y,RM_MARQUEERAD,20,rmMarqueeSemiOnColor);
                count--;
            }
            else
            {
                primCircleSolid2(x,y,RM_MARQUEERAD,20,rmMarqueeOffColor);
                count--;
            }
            x+=RM_MARQUEESPACE;
        }
    }
    else
    {
        for (index=0;index<RM_NUMMARQUEE;index++)
        {
            primCircleSolid2(x,y,RM_MARQUEERAD,20,rmMarqueeOffColor);
            x+=RM_MARQUEESPACE;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmDrawLabButton
    Description : draws lab button
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmDrawLabButton(LabPrintList *labprint, regionhandle region)
{
    color progressColor[4]; // Shaded values
    ferfocuswindowstate state;
    rectangle rect, progressRect;
    real32 percent, width;
    sdword pos;
    PlayerResearchInfo *research = &universe.curPlayerPtr->researchinfo;

    rect.x0 = region->rect.x0 + RM_LAB_INSET;
    rect.y0 = region->rect.y0 + RM_LAB_INSET;
    rect.x1 = region->rect.x1 - RM_LAB_INSET;
    rect.y1 = region->rect.y1 - RM_LAB_INSET;

    progressRect.x0 = rect.x0;
    progressRect.y0 = rect.y1 - 10;
    progressRect.x1 = rect.x1;
    progressRect.y1 = rect.y1;

    if (labprint->selected)
        state = lw_focus;
    else
        state = lw_normal;

    ferDrawFocusWindow(region, state);

    switch (labprint->lab->labstatus)
    {
    case LS_NORESEARCHSHIP :
        primRectSolid2(&rect, colRGB(0, 0, 0));                    // clear area
        primRectOutline2(&progressRect, 1, rmNoResearchItemColor); // draw empty progress bar
        break;

    default:
    case LS_NORESEARCHITEM :
        primRectSolid2(&rect, colRGB(0, 0, 0));                // clear area
        primRectOutline2(&progressRect, 1, colRGB(255, 0, 0)); // draw empty active progress bar

        rect.x0 += 5;
        rect.x1 -= 5;
        rect.y1 = progressRect.y0 - RM_TEXTURE_INSET; // put above the progress bar

        if (glcActive())
        {
            lifheader* lif = rmLabImage[universe.curPlayerPtr->race];
            glcRectSolidTexturedScaled2(&rect,
                                        lif->width, lif->height,
                                        lif->data,
                                        rmPaletted ? lif->palette : NULL,
                                        TRUE);
        }
        else
        {
            if (rmPaletted)
            {
                trPalettedTextureMakeCurrent(rmLabTexture[universe.curPlayerPtr->race], rmLabImage[universe.curPlayerPtr->race]->palette);
            }
            else
            {
                trRGBTextureMakeCurrent(rmLabTexture[universe.curPlayerPtr->race]);
            }

            rndPerspectiveCorrection(FALSE);
            primRectSolidTextured2(&rect);
        }
        break;

    case LS_RESEARCHITEM :
        primRectOutline2(&progressRect, 1, colRGB(255, 0, 0));

        rect.x0 += 5;
        rect.x1 -= 5;
        rect.y1 = progressRect.y0 - RM_TEXTURE_INSET; // put above the progress bar

        if (glcActive())
        {
            lifheader* lif = rmLabImage[universe.curPlayerPtr->race];
            glcRectSolidTexturedScaled2(&rect,
                                        lif->width, lif->height,
                                        lif->data,
                                        rmPaletted ? lif->palette : NULL,
                                        TRUE);
        }
        else
        {
            if (rmPaletted)
            {
                trPalettedTextureMakeCurrent(rmLabTexture[universe.curPlayerPtr->race], rmLabImage[universe.curPlayerPtr->race]->palette);
            }
            else
            {
                trRGBTextureMakeCurrent(rmLabTexture[universe.curPlayerPtr->race]);
            }

            rndPerspectiveCorrection(FALSE);
            primRectSolidTextured2(&rect);
        }

        // compute progress bar length
        percent = (real32)((1.0 - (real32)labprint->lab->topic->timeleft/(real32)research->techstat->TimeToComplete[labprint->lab->topic->techresearch]));

        // Make the progress rectangle fit within the red border
        progressRect.x0++;
        progressRect.x1--;
        progressRect.y0++;
        progressRect.y1--;

        if (labprint->selected)
        {
            pos = ((progressRect.x1 - progressRect.x0) * labprint->pulsepos) >> 16;
            pos = progressRect.x0 + pos;

            primLine2(pos, progressRect.y0 + 1, pos, progressRect.y1 - 1, 0);

            if (labprint->pulsepos > FIXED16_ONE)
            {
                labprint->pulsepos = 0;
            }
            else
                labprint->pulsepos += PULSE_INC;

            pos = ((progressRect.x1 - progressRect.x0) * labprint->pulsepos) >> 16;
            pos = progressRect.x0 + pos;

            primLine2(pos, progressRect.y0 + 1, pos, progressRect.y1 - 1, PULSE_COLOR);
        }

        width = ((real32)(progressRect.x1 - progressRect.x0)) * percent;
        progressRect.x1 = progressRect.x0 + FAST_TO_INT(width);

        progressColor[0] = rmProgressDoneColor0;
        progressColor[1] = rmProgressDoneColor0;
        progressColor[2] = rmProgressDoneColor1;
        progressColor[3] = rmProgressDoneColor1;

        primRectShaded2(&progressRect, progressColor);

        break;
        case LS_RESEARCHITEMSOON :
            primRectSolid2(&rect, colRGB(0, 0, 0));                // clear area
            primRectOutline2(&progressRect, 1, colRGB(255, 0, 0)); // draw empty active progress bar

            rect.x0 += 5;
            rect.x1 -= 5;
            rect.y1 = progressRect.y0 - RM_TEXTURE_INSET; // put above the progress bar

            if (glcActive())
            {
                lifheader* lif = rmLabImage[universe.curPlayerPtr->race];
                glcRectSolidTexturedScaled2(&rect,
                                            lif->width, lif->height,
                                            lif->data,
                                            rmPaletted ? lif->palette : NULL,
                                            TRUE);
            }
            else
            {
                if (rmPaletted)
                {
                    trPalettedTextureMakeCurrent(rmLabTexture[universe.curPlayerPtr->race], rmLabImage[universe.curPlayerPtr->race]->palette);
                }
                else
                {
                    trRGBTextureMakeCurrent(rmLabTexture[universe.curPlayerPtr->race]);
                }

                rndPerspectiveCorrection(FALSE);
                primRectSolidTextured2(&rect);
            }
            break;

    }
}

//update selection list
void SelectTechWindow(void)
{
    udword         index;
    listitemhandle listitem;

    for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
    {
        if ( (PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
             (PlayerTechList[index].itemstat != STAT_CANTRESEARCH) )
        {
            if ((PlayerTechList[index].itemtype == ITEM_TECHNOLOGY) && (PlayerTechList[index].itemID == techinfo))
            {
                listitem = uicListFindItemByData(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index]);
                uicListWindowSetCurItem(rmTechListWindowHandle, listitem);
            }
        }
    }

    /*
    not implemented yet

    listitemhandle  item;

    Node           *walk,*count;
    bool            found=FALSE;
    listitemhandle  iteminfo, newpos;
    sdword          i, relativecount;

    walk = listwindow->listofitems.head;

    while (walk != NULL)
    {
        listhandle->CurLineSelected->data)->itemID
CurLineSelected == item

        if (item == listGetStructOfNode(walk))
        {
            found = TRUE;
            break;
        }
        walk = walk->next;
    }


    bitClear(rmTechListWindowHandle->CurLineSelected->flags,UICLI_Selected);
    listhandle->CurLineSelected = item;
    bitSet(rmTechListWindowHandle->CurLineSelected->flags,UICLI_Selected);

    */

}

/*-----------------------------------------------------------------------------
    Name        : rmSelectAvailableLab
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmSelectAvailableLab(regionhandle region, udword event, sdword labID)
{
    sdword         labindex;
    ResearchTopic *topic;

    if (event == RPE_PressLeft)
    {                                                       //left press (select/add job)
        dbgAssert(region!=NULL);
#ifdef DEBUG_STOMP
        regVerify(region);
#endif
        bitSet(region->status, RSF_DrawThisFrame);
        rmDirtyTechInfo();

        if (labbuttons[labID].lab->labstatus!=LS_NORESEARCHSHIP)
        {
            if(tutorial)
            {
                char gameMessage[128];

                if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchSelectLab)
                    return;

                strcpy(gameMessage, "RM_LabSelect ");
                gameMessage[14] = '0' + labID;
                tutGameMessage(gameMessage);
            }

            topic = labbuttons[labID].lab->topic;
            if (topic != NULL)
            {
                if (multipleresearchersselected)
                {
                    for (labindex=0; labindex<NUM_RESEARCHLABS; labindex++)
                    {
                        if (labID == labindex)
                            labbuttons[labindex].selected = TRUE;
                        else
                            labbuttons[labindex].selected = FALSE;
                    }
                    multipleresearchersselected = FALSE;
                }
                else
                {
                    researchingthistopic = 0;
                    for (labindex=0; labindex<NUM_RESEARCHLABS; labindex++)
                    {
                        if (labbuttons[labindex].lab->topic == topic)
                        {
                            researchingthistopic++;
                            labbuttons[labindex].selected = TRUE;
                        }
                        else
                            labbuttons[labindex].selected = FALSE;
                    }
                    multipleresearchersselected = (researchingthistopic > 1);

                }
                techinfo = topic->techresearch;
                SelectTechWindow();
            }
            else
            {
                labbuttons[labID].selected = (sword)!labbuttons[labID].selected;
                for (labindex=0; labindex<NUM_RESEARCHLABS; labindex++)
                {
                    if (labbuttons[labindex].lab->topic != NULL)
                    {
                        labbuttons[labindex].selected = FALSE;
/*                        if (labbuttons[labindex].lab->topic->techresearch==techinfo)
                        {
                            if (rmTechListWindowHandle->CurLineSelected!=NULL)
                            {
                                techinfo = PlayerTechList[((TechPrintList*)rmTechListWindowHandle->CurLineSelected->data)->itemID].itemID;
                            }
                            else
                                techinfo = -1;
                        }*/
                    }
                }
                techinfo = -1;
                uicListWindowSetCurItem(rmTechListWindowHandle, NULL);

                multipleresearchersselected=FALSE;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab1
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab1(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 0);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab2
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab2(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 1);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab3
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab3(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 2);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab4
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab4(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 3);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab5
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab5(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 4);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab6
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab6(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 5);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmLab1Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab1Draw(featom *atom, regionhandle region)
{
    if (rmRenderEverythingCounter > 0)
    {
        rmRenderEverythingCounter--;
        feRenderEverything = TRUE;
    }

    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab1); //set new region handler function
    }

    rmLab1Region = region;
    rmDrawLabButton(&labbuttons[0], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLab2Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab2Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab2); //set new region handler function
    }

    rmLab2Region = region;
    rmDrawLabButton(&labbuttons[1], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLab3Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab3Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab3); //set new region handler function
    }

    rmLab3Region = region;
    rmDrawLabButton(&labbuttons[2], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLab4Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab4Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab4); //set new region handler function
    }

    rmLab4Region = region;
    rmDrawLabButton(&labbuttons[3], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLab5Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab5Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab5); //set new region handler function
    }

    rmLab5Region = region;
    rmDrawLabButton(&labbuttons[4], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLab6Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab6Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab6); //set new region handler function
    }

    rmLab6Region = region;
    rmDrawLabButton(&labbuttons[5], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLabConnector1Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector1Draw(featom *atom, regionhandle region)
{
    rmLabConnector1Region = region;
    rmMarqueePulse(&labbuttons[0], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector2Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector2Draw(featom *atom, regionhandle region)
{
    rmLabConnector2Region = region;
    rmMarqueePulse(&labbuttons[1], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector3Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector3Draw(featom *atom, regionhandle region)
{
    rmLabConnector3Region = region;
    rmMarqueePulse(&labbuttons[2], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector4Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector4Draw(featom *atom, regionhandle region)
{
    rmLabConnector4Region = region;
    rmMarqueePulse(&labbuttons[3], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector5Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector5Draw(featom *atom, regionhandle region)
{
    rmLabConnector5Region = region;
    rmMarqueePulse(&labbuttons[4], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector6Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector6Draw(featom *atom, regionhandle region)
{
    rmLabConnector6Region = region;
    rmMarqueePulse(&labbuttons[5], region);
}

char *getWord(char *dest, char *source)
{
    sdword index=strlen(dest);

    while ( (source[0] == ' ') || (source[0] == '-') )
    {
        dest[index] = source[0];
        dest[index+1] = 0;
        index++;
        source++;
    }

    while ( (source[0] != ' ') && (source[0] != '-') && (source[0] != 0) && (source[0] != '\n') )
    {
        dest[index] = source[0];
        dest[index+1] = 0;
        index++;
        source++;
    }

    return(source);
}

char *getShipBuild(char *dest)
{
    PlayerResearchInfo *researchinfo = &universe.curPlayerPtr->researchinfo;
    TechStatics        *techstat = researchinfo->techstat;
    sdword              hastech = researchinfo->HasTechnology;
    sdword              index;
    bool                first=TRUE;

    for (index=0;index<STD_LAST_SHIP;index++)
    {
        if ( (techstat->TechNeededToBuildShip[index]&(hastech|TechToBit(techinfo))) ==
              techstat->TechNeededToBuildShip[index])
        {
            if ( (techstat->TechNeededToBuildShip[index]&hastech) !=
                  techstat->TechNeededToBuildShip[index])
            {
                if ( ( (universe.curPlayerPtr->race == R1) &&
                       (index != DefenseFighter          ) &&
                       (index != DFGFrigate              )  ) |
                     ( (universe.curPlayerPtr->race == R2) &&
                       (index != CloakedFighter          ) &&
                       (index != DDDFrigate              )  ) )
                {
                    if (!first)
                    {
                        strcat(dest,", ");
                    }
                    if (first)
                    {
                        strcat(dest,"\n\n");
                        strcat(dest,strGetString(strCanBuild));
                        strcat(dest,"\n");
                    }
                    strcat(dest," ");
                    strcat(dest,ShipTypeToNiceStr(index));
                    first = FALSE;
                }
            }
        }
    }

    return (dest);
}

/*-----------------------------------------------------------------------------
    Name        : rmTechTexturePrepare
    Description : prepares texture for use in the research manager.
    Inputs      : index into pictures structure.
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmTechTexturePrepare(sdword index)
{
    if (pictures[index].techTexture == TR_InvalidInternalHandle)
    {
        pictures[index].techTexture = trPalettedTextureCreate(pictures[index].techImage->data, pictures[index].techImage->palette, pictures[index].techImage->width, pictures[index].techImage->height);
        rmCurTexture = pictures[index].techTexture;
        rmCurIndex = index;
    }
    else
    {
        trPalettedTextureMakeCurrent(pictures[index].techTexture, pictures[index].techImage->palette);
        rmCurTexture = pictures[index].techTexture;
        rmCurIndex = index;
    }
}


/*-----------------------------------------------------------------------------
    Name        : rmExtendedInfo
    Description :
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmExtendedInfo(char *string, featom *atom)
{
    rmExtendedInfoActive = !rmExtendedInfoActive;
    rmDirtyTechInfo();
}


/*-----------------------------------------------------------------------------
    Name        : rmTechInfoDraw
    Description : Draws technology text to the window.
    Inputs      : region
    Outputs     : technology info.
    Return      : void
----------------------------------------------------------------------------*/
void rmTechInfoDraw(regionhandle region)
{
    fonthandle  currentFont;
    sdword      x,y, width;
    char       *pos, *oldpos;
    char        oldline[100], line[100];
    char        stringtoprint[650];
    bool        justified, done;

    //rmTechInfoRegion = region;

    //feStaticRectangleDraw(region); //draw standard rectangle

    //currentFont = fontMakeCurrent(rmTechListFont);
    currentFont = fontMakeCurrent(rmTechInfoFont);

    x = region->rect.x0 + 15;
    y = region->rect.y0 + 5;

    if (techinfo != -1)
    {
        fontPrintf(x,y,FEC_ListItemStandard/*rmResearchingTextColor*/,"%s",
                   RaceSpecificTechTypeToNiceString(techinfo, universe.curPlayerPtr->race));

        y += RM_VertSpacing + fontHeight(" ");

        //fontMakeCurrent(rmTechInfoFont);

        // Bad bad design, my fault [Drew] doh!
        if (techinfo==DDDFDFGFTech)
            if (universe.curPlayerPtr->race==R1)
                strcpy(stringtoprint,strGetString(strR1DDDFTechinfo));
            else
                strcpy(stringtoprint,strGetString(strR2DFGFTechinfo));
        else if (techinfo==CloakDefenseFighter)
            if (universe.curPlayerPtr->race==R1)
                strcpy(stringtoprint,strGetString(strR1CloakFighterinfo));
            else
                strcpy(stringtoprint,strGetString(strR2DefenseFighterTechinfo));
        else
            if (universe.curPlayerPtr->race==R1)
                strcpy(stringtoprint,strGetString(techinfo+strTechInfoOffsetR1));
            else
                strcpy(stringtoprint,strGetString(techinfo+strTechInfoOffsetR2));

        getShipBuild(stringtoprint);

        pos = stringtoprint;

        done = FALSE;
        while (!done)
        {
            justified = FALSE;
            line[0]=0;
            while (!justified)
            {
                strcpy(oldline, line);
                oldpos = pos;
                pos = getWord(line, pos);

                if (pos[0] == '\n')
                {
                    if ((width = fontWidth(line)) > RM_InfoWidth - 15)
                    {
                        strcpy(line, oldline);
                        pos = oldpos;
                        while (pos[0] == ' ') pos++;
                        justified = TRUE;
                    }
                    else
                    {
                        justified = TRUE;
                        pos++;
                        while ( pos[0] == ' ' ) pos++;
                    }
                }
                else
                {
                    if ( (width=fontWidth(line)) > RM_InfoWidth - 15)
                    {
                        strcpy(line, oldline);
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

            fontPrintf(x,y,rmStandardTextColor,"%s",line);
            y += fontHeight(" ");
            if (y > region->rect.y1 + fontHeight(" ")) done=TRUE;
        }
    }

    fontMakeCurrent(currentFont);
}


/*-----------------------------------------------------------------------------
    Name        : rmTechBriefDraw
    Description :
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmTechBriefDraw(featom *atom, regionhandle region)
{
    fonthandle  currentFont;
    sdword      x,y;
    rectangle   rect;
    //char       *pos, *oldpos;
    //char        oldline[100], line[100];
    //char        stringtoprint[650];
    //bool        justified, done;

    rmTechBriefRegion = region;
    rect = region->rect;

    feStaticRectangleDraw(region); //draw standard rectangle
    rect.x0 += 2;
    rect.y0 += 2;
    rect.x1 -= 2;
    rect.y1 -= 2;
    primRectSolid2(&rect, atom->contentColor);

    currentFont = fontMakeCurrent(rmTechListFont);
    //currentFont = fontMakeCurrent(rmTechInfoFont);

    x = region->rect.x0 + 15;
    y = region->rect.y0 + 5;

    if (techinfo != -1)
    {
        fontPrintf(x,y,FEC_ListItemStandard/*rmResearchingTextColor*/,"%s",TechTypeToNiceString(techinfo));

        /*
        y += RM_VertSpacing + fontHeight(" ");

        //fontMakeCurrent(rmTechInfoFont);

        if (universe.curPlayerPtr->race==R1)
        {
            dbgAssert(strGetString(techinfo+strTechInfoOffsetR1)!=NULL);
            strcpy(stringtoprint,strGetString(techinfo+strTechInfoOffsetR1));
        }
        else
        {
            dbgAssert(strGetString(techinfo+strTechInfoOffsetR2)!=NULL);
            strcpy(stringtoprint,strGetString(techinfo+strTechInfoOffsetR2));
        }

        getShipBuild(stringtoprint);

        pos = stringtoprint;

        done = FALSE;
        while (!done)
        {
            justified = FALSE;
            line[0]=0;
            while (!justified)
            {
                strcpy(oldline, line);
                oldpos = pos;
                pos = getWord(line, pos);

                if (pos[0] == '\n')
                {
                    justified = TRUE;
                    pos++;
                    while ( pos[0] == ' ' ) pos++;
                }
                else
                {
                    if ( (width=fontWidth(line)) > RM_InfoWidth - 15)
                    {
                        strcpy(line, oldline);
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

            fontPrintf(x,y,rmStandardTextColor,"%s",line);
            y += fontHeight(" ");
            if (y > region->rect.y1 + fontHeight(" ")) done=TRUE;
        }
        */
    }

    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : rmTechImageDraw
    Description : Loads in, decompresses and draws the tech picture.
    Inputs      : none
    Outputs     : loads in rmTechImage and creates rmTechTexture
    Return      : void
----------------------------------------------------------------------------*/
void rmTechImageDraw(featom *atom, regionhandle region)
{
    char      filename[128];
    sdword    index, lru;
    real32    time=(real32)1.0e22;
    rectangle textureRect;

    rmTechImageRegion = region;

    feStaticRectangleDraw(region); //draw standard rectangle

    if (techinfo != -1)
    {
        if (techinfo != rmCurTechTexture)
        {
            for (index=0;index<RM_TOTALPICS;index++) // RM_TOTALPICS is the size of the cache
            {
                // Find least recently used texture
                if (pictures[index].timestamp < time)
                {
                    time = pictures[index].timestamp;
                    lru  = index;
                }
                // If already cached, use it and exit this routine
                if ( (pictures[index].tech==techinfo) &&
                     (pictures[index].race==universe.curPlayerPtr->race) )
                {
                    rmTechTexturePrepare(index);
                    rmCurTechTexture = techinfo;
                    rmCurTechImage = pictures[lru].techImage;
                    rmCurIndex = index;
                    pictures[index].timestamp = universe.totaltimeelapsed;

                    textureRect.x0 = region->rect.x0+RM_TEXTURE_INSET;
                    textureRect.y0 = region->rect.y0+RM_TEXTURE_INSET;
                    textureRect.x1 = region->rect.x1-RM_TEXTURE_INSET;
                    textureRect.y1 = region->rect.y1-RM_TEXTURE_INSET;

                    rndPerspectiveCorrection(FALSE);
                    if (glcActive())
                    {
                        glcRectSolidTextured2(&textureRect,
                                              pictures[rmCurIndex].techImage->width,
                                              pictures[rmCurIndex].techImage->height,
                                              pictures[rmCurIndex].techImage->data,
                                              pictures[rmCurIndex].techImage->palette,
                                              TRUE);
                    }
                    else
                    {
                        primRectSolidTextured2(&textureRect);
                    }

                    if (rmExtendedInfoActive)
                    {
                        primRectTranslucent2(&textureRect, colRGBA(0, 0, 0, 128));
                        rmTechInfoDraw(region);
                    }
                    return;
                }
            }

            // Build filename for loading texture from file
            strcpy(filename, TechImagePaths[universe.curPlayerPtr->race]);
            strcat(filename, TechTypeToString(techinfo));
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

            rmTechTexturePrepare(lru);
            rmCurTechTexture = techinfo;
            rmCurTechImage = pictures[lru].techImage;
            pictures[lru].tech = techinfo;
            pictures[lru].race = universe.curPlayerPtr->race;
            pictures[lru].timestamp = universe.totaltimeelapsed;
        }
        else
        {
            trPalettedTextureMakeCurrent(pictures[rmCurIndex].techTexture,
                                         pictures[rmCurIndex].techImage->palette);
        }

        textureRect.x0 = region->rect.x0 + RM_TEXTURE_INSET;
        textureRect.y0 = region->rect.y0 + RM_TEXTURE_INSET;
        textureRect.x1 = region->rect.x1 - RM_TEXTURE_INSET;
        textureRect.y1 = region->rect.y1 - RM_TEXTURE_INSET;

        rndPerspectiveCorrection(FALSE);
        if (glcActive())
        {
            glcRectSolidTextured2(&textureRect,
                                  pictures[rmCurIndex].techImage->width,
                                  pictures[rmCurIndex].techImage->height,
                                  pictures[rmCurIndex].techImage->data,
                                  pictures[rmCurIndex].techImage->palette,
                                  TRUE);
        }
        else
        {
            primRectSolidTextured2(&textureRect);
        }

        if (rmExtendedInfoActive)
        {
            primRectTranslucent2(&textureRect, colRGBA(0, 0, 0, 128));
            rmTechInfoDraw(region);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : rmConnectorDraw
    Description : Draws the connector between the picture of the technology and
                  the info. text
    Inputs      : region
    Outputs     : connector image
    Return      : void
----------------------------------------------------------------------------*/
void rmConnectorDraw(featom *atom, regionhandle region)
{
    rmConnectorRegion = region;

    if (techinfo != -1)
    {
        sdword centerX  = region->rect.x0 + ((region->rect.x1 - region->rect.x0) / 2);

        primLine2(centerX, region->rect.y0, centerX, region->rect.y1, colRGB(255, 0, 0));
        primCircleSolid2(centerX, region->rect.y1, 3, 16, colRGB(255, 0, 0));
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmDrawTechListItem
    Description : This function draws each item in the technology list.
    Inputs      : rectangle, and a pointer to the item handle.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmDrawTechListItem(rectangle *rect, listitemhandle data)
{
    TechPrintList *techprint=(TechPrintList *)data->data;
    sdword              x, y;
    color               c;
    Player             *player=universe.curPlayerPtr;
    PlayerResearchInfo *research=&universe.curPlayerPtr->researchinfo;
    ResearchTopic      *topic;
    TechnologyType      tech;
    char                temp[64];
    real32              percent;
    rectangle           bar;
    fonthandle          oldfont;
    color               progressColor[4];

    x = rect->x0;
    y = rect->y0;

    oldfont = fontMakeCurrent(rmTechListFont);

    if (techprint->itemtype==ITEM_TECHNOLOGY)
    {
        tech = techprint->itemID;

        sprintf(temp, "%s", RaceSpecificTechTypeToNiceString(tech, player->race));

        switch (techprint->itemstat)
        {
            case STAT_CANRESEARCH :
                x += RM_HorzSpacing<<2;
                c = FEC_ListItemStandard;//rmStandardTextColor;
            break;
            case STAT_CANTRESEARCH :
                x += RM_HorzSpacing<<2;
                c = rmCantResearchTextColor;
            break;
            case STAT_RESEARCHING :
                x += RM_HorzSpacing<<2;
                if ((topic=Researching(player,tech))!=NULL)
                {
                    percent = (real32)((real32)topic->timeleft/(real32)research->techstat->TimeToComplete[tech]);
                    bar.x0 = x-2;
                    bar.y0 = y/*-2*/;
                    bar.x1 = rect->x1-2;
                    bar.y1 = y+fontHeight(" ") + 2;
                    primRectSolid2(&bar, rmProgressToGoColor);
                    bar.x1 -= (udword)((real32)( (rect->x1-2) - (x-2))*percent);

                    progressColor[0] = rmProgressDoneColor0;
                    progressColor[1] = rmProgressDoneColor0;
                    progressColor[2] = rmProgressDoneColor1;
                    progressColor[3] = rmProgressDoneColor1;

                    primRectShaded2(&bar, progressColor);

#ifdef DEBUG_STOMP
                    regVerify((regionhandle)&rmTechListWindowHandle->reg);
#endif
                    rmTechListWindowHandle->reg.status |= RSF_DrawThisFrame;
                }
                c = rmResearchingTextColor;
            break;
            case STAT_ALREADYHAVE :
                x += RM_HorzSpacing<<2;
                c = FEC_ListItemInactive;//rmStandardTextColor;
                primCircleSolid2(x-7,y+5,3,20,rmResearchingTextColor);
            break;
        }
        if (bitTest(data->flags,UICLI_Selected))
        {
            if (techprint->itemstat == STAT_RESEARCHING)
            {
                c = rmResearchingTextColor;//rmSelectionTextColor;
            }
            else
            {
                c = FEC_ListItemSelected;//rmSelectionTextColor;
            }
        }
    }
    else if ( (techprint->itemtype==ITEM_CLASSHEADER) &&
              (techprint->itemstat==STAT_CANPRINT) )
    {
        sprintf(temp,"%s",strGetString(techprint->itemID+strTechHeadingOffset));
        c = rmClassHeadingTextColor;
    }

    fontPrintf(x,y,c,"%s",temp);

    fontMakeCurrent(oldfont);

    marqueepos += (real32)((taskTimeElapsed - marqueetime) * RM_MARQUEERATE);
    if (marqueepos > (real32)(RM_NUMMARQUEE / 3) + 1.0)
    {
        marqueepos -= (real32)(RM_NUMMARQUEE / 3) + 1.0;
    }
    marqueetime = taskTimeElapsed;
}


/*-----------------------------------------------------------------------------
    Name        : rmTechListWindow
    Description : This function handle messages from the technology listwindow.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmTechListWindow(char *string, featom *atom)
{
    fonthandle oldfont;
    sdword     index;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(rmTechListFont);
        rmTechListWindowHandle = (listwindowhandle)atom->pData;

        uicListWindowInit(rmTechListWindowHandle,
                          NULL,         //  title draw, no title
                          NULL,         //  title click process, no title
                          0,            //  title height, no title
                          rmDrawTechListItem,           // item draw funtcion
                          fontHeight(" ")+RM_VertSpacing, // item height
                          UICLW_CanSelect);

        for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
        {
            if ( (PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
                 (PlayerTechList[index].itemstat != STAT_CANTRESEARCH) )
            {
                if (PlayerTechList[index].itemtype == ITEM_CLASSHEADER)
                    uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], 0, UICLW_AddToTail);
                else
                    uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], UICLI_CanSelect, UICLW_AddToTail);
            }
        }
        return;
    } else if (FELASTCALL(atom))
    {
        rmTechListWindowHandle = NULL;
        return;
    }


    switch (rmTechListWindowHandle->message)
    {
        case CM_NewItemSelected:
        {
            if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchSelectTech)
                return;

            techinfo = ((TechPrintList*)rmTechListWindowHandle->CurLineSelected->data)->itemID;
            rmDirtyTechInfo();

            if ( ((TechPrintList*)rmTechListWindowHandle->CurLineSelected->data)->itemstat==STAT_RESEARCHING)
            {
                for (index=0; index<NUM_RESEARCHLABS; index++)
                {
                    multipleresearchersselected=0;
                    if ((labbuttons[index].lab->topic)&&(labbuttons[index].lab->topic->techresearch==techinfo))
                    {
                        labbuttons[index].selected = TRUE;
                        multipleresearchersselected++;
                    }
                    else
                        labbuttons[index].selected = FALSE;
                    multipleresearchersselected= (multipleresearchersselected>1);
                }
            }
            else
            {
                for (index=0; index<NUM_RESEARCHLABS; index++)
                {
                    if (labbuttons[index].lab->labstatus == LS_RESEARCHITEM)
                    {
                        labbuttons[index].selected = FALSE;
                    }
                }
            }

            if(tutorial)
            {
                char gameMessage[128];

                strcpy(gameMessage, "RM_TechSelect_");
                strcat(gameMessage, TechTypeToString(techinfo));
                tutGameMessage(gameMessage);
            }
        }
        break;

        case CM_DoubleClick:

            if (tutorial)
            {
                tutGameMessage("RM_DoubleClick");
            }
            rmResearchItem(NULL, NULL);
        break;



    }
}

/*-----------------------------------------------------------------------------
    Name        : rmUpdateTechList
    Description : updates the status of the technology list.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmUpdateTechList(void)
{
    PlayerResearchInfo *research=&universe.curPlayerPtr->researchinfo;
    TechnologyType tech;
    sdword index, mask, hastech, canprint=STAT_CANTPRINT;
    sdword count=0;
//    ubyte *data=NULL;

    for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
    {
        if (PlayerTechList[index].itemtype==ITEM_TECHNOLOGY)
        {
            tech    = PlayerTechList[index].itemID;
            hastech = research->HasTechnology;
            mask    = research->techstat->TechNeededToResearch[tech];

            if (TechToBit(tech)&hastech)
            {
                PlayerTechList[index].itemstat = STAT_ALREADYHAVE;
            }
            else if (Researching(universe.curPlayerPtr,tech)!=NULL)
            {
                PlayerTechList[index].itemstat = STAT_RESEARCHING;
            }
            else if (singlePlayerGame)
            {
                if ( tmTechResearchable[tech] )
                {
                    PlayerTechList[index].itemstat = STAT_CANRESEARCH;
                }
                else
                {
                    PlayerTechList[index].itemstat = STAT_CANTRESEARCH;
                }
            }
            else
            {
                if ((hastech & mask)==mask)
                {
                    PlayerTechList[index].itemstat = STAT_CANRESEARCH;
                }
                else
                {
                    PlayerTechList[index].itemstat = STAT_CANTRESEARCH;
                }
            }
        }
    }

    for (; index >= 0; index--)
    {
        if( (PlayerTechList[index].itemtype == ITEM_TECHNOLOGY) &&
         (PlayerTechList[index].itemstat != STAT_CANTRESEARCH) )
        {
            canprint=STAT_CANPRINT;
        }
        else if (PlayerTechList[index].itemtype==ITEM_CLASSHEADER)
        {
            PlayerTechList[index].itemstat = (sword)canprint;

            canprint=STAT_CANTPRINT;
        }
        if ( (PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
             (PlayerTechList[index].itemstat != STAT_CANTRESEARCH) )
        {
            count++;
        }
    }
    count--;

    // Regenerate list because something has just finished being researched

    if ( (rmTechListWindowHandle!=NULL) &&
         (count != rmTechListWindowHandle->ListTotal))
    {
        uicListCleanUp(rmTechListWindowHandle);
        for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
        {
            if ( (PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
                 (PlayerTechList[index].itemstat != STAT_CANTRESEARCH) )
            {
                if (PlayerTechList[index].itemtype == ITEM_CLASSHEADER)
                    uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], 0, UICLW_AddToTail);
                else
                    uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], UICLI_CanSelect, UICLW_AddToTail);
            }
        }

        SelectTechWindow();

        rmDirtyTechList();
    }

    if (rmTechListWindowHandle != NULL)
    {
    #ifdef DEBUG_STOMP
        regVerify((regionhandle)&rmTechListWindowHandle->reg);
    #endif
        rmTechListWindowHandle->reg.status |= RSF_DrawThisFrame;
    }

    rmDirtyTechList();
}


/*-----------------------------------------------------------------------------
    Name        : rmResearchBegin
    Description : brings up research manager GUI
    Inputs      : standard region event handles
    Outputs     : success fail
    Return      :
----------------------------------------------------------------------------*/
sdword rmResearchGUIBegin(regionhandle region, sdword ID, udword event, udword data)
{
    sdword index;

    if ((playPackets) || (universePause)) return 0;

    for(index=0; index<NUM_RESEARCHLABS; index++)
    {
        if(universe.curPlayerPtr->researchinfo.researchlabs[index].labstatus != LS_NORESEARCHSHIP)
            break;
    }

    if(index == NUM_RESEARCHLABS)
    {
      // Don't have any research labs!
      return (0);
    }

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchManager)
        return (0);

    // disable rendering of main screen
    mrRenderMainScreen = FALSE;

    glcFullscreen(TRUE);

    rmRenderEverythingCounter = (tutorial == TUTORIAL_ONLY) ? 4 : 0;

    // clear the screen
    rndClear();

    // disable taskbar popup window
    tbDisable = TRUE;

    //mouseEnable();
    //mouseCursorShow();

    spLockout(SPLOCKOUT_MR);

    if (!rmScreensHandle)
    {
        feCallbackAddMultiple(rmCallback);                  //add in the callbacks
        feDrawCallbackAddMultiple(rmDrawCallback);
        rmScreensHandle = feScreensLoad(RM_FIBFile);        //load in the screen
    }

    if (!singlePlayerGame)
    {
        if (bitTest(tpGameCreated.flag,MG_CarrierIsMothership))
        {
            if (universe.curPlayerPtr->race==R1)
                PlayerTechList = &MultiPlayerCR1TechList[0];
            else
                PlayerTechList = &MultiPlayerCR2TechList[0];
        }
        else
        {
            if (universe.curPlayerPtr->race==R1)
                PlayerTechList = &MultiPlayerR1TechList[0];
            else
                PlayerTechList = &MultiPlayerR2TechList[0];
        }
    }
    else
    {
        if (universe.curPlayerPtr->race==R1)
            PlayerTechList = &SinglePlayerR1TechList[0];
        else
            PlayerTechList = &SinglePlayerR2TechList[0];
    }

    for (index=0;index<NUM_RESEARCHLABS;index++)
    {
        labbuttons[index].lab = &universe.curPlayerPtr->researchinfo.researchlabs[index];
        labbuttons[index].selected = FALSE;
        labbuttons[index].labid = (uword) (index+1);
        labbuttons[index].pulsepos = 0;
    }

    soundEventStopSFX(0.5f);

    /* play the intro sound */
    soundEvent(NULL, UI_ManagerIntro);
    /* start the research ambient sound */
    soundEvent(NULL, UI_ResearchManager);

    rmUpdateTechList();

    rmIoSaveState = ioDisable();

    rmBaseRegion = feScreenStart(region, RM_ResearchScreen);  //add new regions as siblings of current one

    rmGUIActive = TRUE;

    tutGameMessage("Start_ResearchManager");

    mouseCursorShow();

//    rmExtendedInfoActive = FALSE;

    mouseCursorShow();

    return (0);
}

/*-----------------------------------------------------------------------------
    Name        : ShipsNeedTech
    Description : rerturns true if a ship needs the technology specified.
    Inputs      : techstatics
    Outputs     : TRUE/FALSE
    Return      : bool
----------------------------------------------------------------------------*/
bool ShipsNeedTech(TechStatics *techstat, TechnologyType tech)
{
    sdword index;

    for (index=0;index<STD_LAST_SHIP;index++)
    {
        if (techstat->TechNeededToBuildShip[index]&TechToBit(tech))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : rmSetPrintList
    Description : Sets the print lists for single and multiplayer after the dependancies are loaded;
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmSetPrintList(udword whichlist, TechStatics *techstat)
{
    sdword index, copycount;

    switch (whichlist)
    {
        case RM_SPR1 :
            for (copycount=0,index=0;index<NUMTECHSINLIST; index++)
            {
                if (PlayerTechListStatic[index].itemtype==ITEM_TECHNOLOGY)
                {
                    if (ShipsNeedTech(techstat, PlayerTechListStatic[index].itemID))
                    {
                        SinglePlayerR1TechList[copycount] = PlayerTechListStatic[index];
                        copycount++;
                    }
                }
                else
                {
                    SinglePlayerR1TechList[copycount] = PlayerTechListStatic[index];
                    copycount++;
                }
            }
            break;
        case RM_SPR2 :
            for (copycount=0,index=0;index<NUMTECHSINLIST; index++)
            {
                if (PlayerTechListStatic[index].itemtype==ITEM_TECHNOLOGY)
                {
                    if (ShipsNeedTech(techstat, PlayerTechListStatic[index].itemID))
                    {
                        SinglePlayerR2TechList[copycount] = PlayerTechListStatic[index];
                        copycount++;
                    }
                }
                else
                {
                    SinglePlayerR2TechList[copycount] = PlayerTechListStatic[index];
                    copycount++;
                }
            }
            break;
        case RM_MPR1 :
            for (copycount=0,index=0;index<NUMTECHSINLIST; index++)
            {
                if (PlayerTechListStatic[index].itemtype==ITEM_TECHNOLOGY)
                {
                    if (ShipsNeedTech(techstat, PlayerTechListStatic[index].itemID))
                    {
                        MultiPlayerR1TechList[copycount] = PlayerTechListStatic[index];
                        copycount++;
                    }
                }
                else
                {
                    MultiPlayerR1TechList[copycount] = PlayerTechListStatic[index];
                    copycount++;
                }
            }
            break;
        case RM_MPR2 :
            for (copycount=0,index=0;index<NUMTECHSINLIST; index++)
            {
                if (PlayerTechListStatic[index].itemtype==ITEM_TECHNOLOGY)
                {
                    if (ShipsNeedTech(techstat, PlayerTechListStatic[index].itemID))
                    {
                        MultiPlayerR2TechList[copycount] = PlayerTechListStatic[index];
                        copycount++;
                    }
                }
                else
                {
                    MultiPlayerR2TechList[copycount] = PlayerTechListStatic[index];
                    copycount++;
                }
            }
            break;
        case RM_MPCR1 :
            for (copycount=0,index=0;index<NUMTECHSINLIST; index++)
            {
                if (PlayerTechListStatic[index].itemtype==ITEM_TECHNOLOGY)
                {
                    if (ShipsNeedTech(techstat, PlayerTechListStatic[index].itemID))
                    {
                        MultiPlayerCR1TechList[copycount] = PlayerTechListStatic[index];
                        copycount++;
                    }
                }
                else
                {
                    MultiPlayerCR1TechList[copycount] = PlayerTechListStatic[index];
                    copycount++;
                }
            }
            break;
        case RM_MPCR2 :
            for (copycount=0,index=0;index<NUMTECHSINLIST; index++)
            {
                if (PlayerTechListStatic[index].itemtype==ITEM_TECHNOLOGY)
                {
                    if (ShipsNeedTech(techstat, PlayerTechListStatic[index].itemID))
                    {
                        MultiPlayerCR2TechList[copycount] = PlayerTechListStatic[index];
                        copycount++;
                    }
                }
                else
                {
                    MultiPlayerCR2TechList[copycount] = PlayerTechListStatic[index];
                    copycount++;
                }
            }
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmGUIStartup
    Description : Initializes the Research Manager GUI
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmGUIStartup(void)
{
    sdword index;
    char filename[128];

    rmRenderEverythingCounter = 0;

    for (index=0;index<RM_TOTALPICS;index++)
    {
        pictures[index].techImage   = NULL;
        pictures[index].tech        = -1;
        pictures[index].race        = -1;
        pictures[index].timestamp   = 0.0;
        pictures[index].techTexture = TR_InvalidInternalHandle;
    }

    for(index=0; index < MAX_RACES; index++)
    {
        //if( rmLabTexture[index] == TR_InvalidInternalHandle)
        {
            // Build filename for loading texture from file
            strcpy(filename, TechImagePaths[index]);
            strcat(filename, RM_LabIcon);

            // Load the image
            rmLabImage[index] = trLIFFileLoad(filename, NonVolatile);
            if (bitTest(rmLabImage[index]->flags, TRF_Paletted))
            {
                rmPaletted = TRUE;
            }
            else
            {
                rmPaletted = FALSE;
            }

            if (rmPaletted)
            {
                rmLabTexture[index] = trPalettedTextureCreate(
                    rmLabImage[index]->data,
                    rmLabImage[index]->palette,
                    rmLabImage[index]->width,
                    rmLabImage[index]->height);
            }
            else
            {
                rmLabTexture[index] = trRGBTextureCreate(
                    (color*)rmLabImage[index]->data,
                    rmLabImage[index]->width,
                    rmLabImage[index]->height,
                    TRUE);
            }

            //trPalettedTextureMakeCurrent(rmLabTexture[index], rmLabImage[index]->palette);
        }
    }

    rmCurTexture   = TR_InvalidInternalHandle;
    rmCurTechImage = NULL;
    rmCurTechTexture = -1;

    rmTechListFont  = frFontRegister(rmTechListFontName);
    rmTechInfoFont  = frFontRegister(rmTechInfoFontName);
}

/*-----------------------------------------------------------------------------
    Name        : rmGUIShutdown
    Description : shuts down the research managers GUI
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmGUIShutdown(void)
{
    sdword index;

    if (rmScreensHandle)
    {
        feScreensDelete(rmScreensHandle);
        rmScreensHandle = NULL;
    }

    for (index=0;index<RM_TOTALPICS;index++)
    {
        if (pictures[index].techImage != NULL)
        {
            memFree(pictures[index].techImage);
            pictures[index].techImage = NULL;
        }

        if (pictures[index].techTexture != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(pictures[index].techTexture);
            pictures[index].techTexture = TR_InvalidInternalHandle;
        }
    }

    for(index=0;index<MAX_RACES;index++)
    {
        if (rmLabImage[index] != NULL)
        {
            memFree(rmLabImage[index]);
            rmLabImage[index] = NULL;
        }
        if (rmLabTexture[index] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(rmLabTexture[index]);
            rmLabTexture[index] = TR_InvalidInternalHandle;
        }
    }
}
