/*=============================================================================
    Name    : Researchgui.h
    Purpose : definitions for the research manager gui

    Created 5/25/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___RESEARCHGUI_H
#define ___RESEARCHGUI_H

#include "Types.h"
#include "Region.h"
#include "texreg.h"
#include "ResearchAPI.h"

/*=============================================================================
    Definitions:
=============================================================================*/

#ifdef _WIN32
#define RM_FIBFile          "FEMan\\Research_Manager.fib"
#else
#define RM_FIBFile          "FEMan/Research_Manager.fib"
#endif
#define RM_ResearchScreen   "Research_Manager"
#define RM_DefaultFont      "default.hff"
#define RM_FontNameLength   64

#define RM_TechListFont     "HW_EuroseCond_11.hff"
#define RM_TechInfoFont     "Arial_12.hff"

// definitions for the status of a research item

#define RI_GRAYED       0
#define RI_RESEARCHING

// definitions for print list

#define ITEM_ENDLIST        -1
#define ITEM_CLASSHEADER    0
#define ITEM_TECHNOLOGY     1

#define STAT_CANRESEARCH    0
#define STAT_CANTRESEARCH   1
#define STAT_RESEARCHING    2
#define STAT_ALREADYHAVE    4
#define STAT_CANPRINT       5
#define STAT_CANTPRINT      6

// colors for text in the research manager
#define RM_SelectionTextColor     colRGB(255, 255, 105)
#define RM_ResearchingTextColor   colRGB(255, 200, 0)
#define RM_CantResearchTextColor  colRGB(100, 100, 100)
#define RM_StandardTextColor      colRGB(0  , 255, 0)
#define RM_ClassHeadingTextColor  colRGB(255, 200, 0)
#define RM_ProgressToGoColor      colRGB(180, 180, 80)
#define RM_ProgressDoneColor0     colRGB(50, 50, 27)
#define RM_ProgressDoneColor1     colRGB(160, 160, 115)
#define RM_LabActiveColor         colRGB(0  , 160, 20)
#define RM_PulseColor             colRGB(20 , 20 , 255)
#define RM_NoResearchItemColor    colRGB(255, 0, 0)
#define RM_MarqueeOnColor         colRGB(255, 255, 0)
#define RM_MarqueeSemiOnColor     colRGB(175, 175, 0)
#define RM_MarqueeOffColor        colRGB(100, 100, 0)

#define RM_SPR1                   1
#define RM_SPR2                   2
#define RM_MPR1                   3
#define RM_MPR2                   4
#define RM_MPCR1                  5
#define RM_MPCR2                  6

/*=============================================================================
    Type definitions:
=============================================================================*/

typedef struct
{
    sword  itemtype;
    sword  itemstat;
    sdword itemID;
}
TechPrintList;

typedef struct
{
    sword  labid;
    sword  selected;
    sdword pulsepos;
    ResearchLab *lab;
}
LabPrintList;

typedef struct
{
    real32 x;
    real32 y;
}
Point;

typedef struct
{
    char *name;
}
TechNames;

typedef struct
{
    lifheader      *techImage;
    udword          techTexture;
    TechnologyType  tech;
    sdword          race;
    real32          timestamp;
}
LRUPicture;

extern bool rmGUIActive;

// LRU defines, so that the trade manager can reuse the same LRU cache
#define RM_TOTALPICS            6

extern LRUPicture pictures[RM_TOTALPICS];


/*=============================================================================
    Function Prototypes:
=============================================================================*/

// function called if something changes in the list stuff
void rmUpdateTechList(void);
void rmClearLab(sdword labindex); // Turn off lab selected

//start the Research Manager.  It will kill itself when you hit the launch button.
void   rmSetPrintList(udword whichlist, TechStatics *techstat);
sdword rmResearchGUIBegin(regionhandle region, sdword ID, udword event, udword data);
void   rmGUIStartup(void);
void   rmGUIShutdown(void);
void rmCloseIfOpen(void);

#endif
