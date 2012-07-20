/*

    Tutor.c - Functions for the Tutorial System

*/

#ifdef _WIN32
#include <windows.h>
#endif

#include "glinc.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "Types.h"
#include "FEFlow.h"
#include "AIPlayer.h"
#include "utility.h"
#include "LevelLoad.h"
#include "Region.h"
#include "KAS.h"
#include "UIControls.h"
#include "Strings.h"
#include "FontReg.h"
#include "Select.h"
#include "texreg.h"
#include "FontReg.h"
#include "render.h"
#include "mouse.h"
#include "FastMath.h"
#include "Tutor.h"
#include "ConsMgr.h"
#include "CameraCommand.h"
#include "FastMath.h"
#include "SaveGame.h"
#include "Collision.h"
#include "GamePick.h"
#include "main.h"
#include "Subtitle.h"
#include "SinglePlayer.h"
#include "SpeechEvent.h"
#include "SoundEvent.h"
#include "File.h"
#include "mainrgn.h"
#include "TaskBar.h"
#include "../Generated/Tutorial1.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

//hack to get the speech to work when game is paused
bool FalkosFuckedUpTutorialFlag = FALSE;

// Function declarations
void utySinglePlayerGameStart(char *name, featom *atom);
char *tutGetNextTextLine(char *pDest, char *pString, long Width);
udword tutProcessNextButton(struct tagRegion *reg, sdword ID, udword event, udword data);
udword tutProcessBackButton(struct tagRegion *reg, sdword ID, udword event, udword data);
udword uicButtonProcess(regionhandle region, sdword ID, udword event, udword data);

// flags for controlling game elements from the tutorial
tutGameEnableFlags tutEnable;
char tutBuildRestrict[TOTAL_STD_SHIPS];
char tutGameMessageList[16][256];
long tutGameMessageIndex = 0;
ShipType tutFEContextMenuShipType;

static char tutLastLessonName[256] = "";
static char tutCurrLessonName[256] = "";

/*
sdword  tutTextPointerType = 0;
sdword  tutPointerX, tutPointerY;
rectangle tutPointerRegion;
hvector tutPointerAIPoint;
*/
ShipPtr tutPointerShip;
rectangle *tutPointerShipHealthRect;
rectangle *tutPointerShipGroupRect;
//list of named tutorial pointers
static tutpointer tutPointer[TUT_NumberPointers];
bool tutPointersDrawnThisFrame = FALSE;


static sdword  tutTextPosX, tutTextPosY;
static sdword  tutTextSizeX, tutTextSizeY;

static rectangle   tutTextRect;
static fonthandle tutTextFont = 0;

static regionhandle    tutRootRegion;
static sdword tutRootRegionCount = 0;

// visibility flag & region handle for the displayed text
sdword  tutTextVisible = FALSE;
regionhandle tutTextRegion = NULL;
void tutDrawTextFunction(regionhandle reg);

// visibility flag & region handle for the displayed images
sdword  tutImageVisible = FALSE;
regionhandle tutImageRegion = NULL;
void tutDrawImageFunction(regionhandle reg);

// visibility flag & region handle for the "next" button
sdword tutNextVisible = FALSE;
buttonhandle tutNextRegion = NULL;
sdword  tutNextButtonState = 0;
void tutDrawNextButtonFunction(regionhandle reg);

// visibility flag & region handle for the "back" button
sdword tutBackVisible = FALSE;
sdword tutPrevVisible = FALSE;
buttonhandle tutBackRegion = NULL;
void tutDrawBackButtonFunction(regionhandle reg);

// List of image indices for the displayed images
static char szImageIndexList[16];


// Construction manager exports
extern shipavailable *cmShipsAvailable;
extern LinkedList listofShipsInProgress;

featom tutRootAtom =
{
    "TutorialWrapper",      //  char  *name;                                //optional name of control
    FAF_Function,           //  udword flags;                               //flags to control behavior
    0,                      //  udword status;                              //status flags for this atom, checked etc.
    FA_UserRegion,          //  ubyte  type;                                //type of control (button, scroll bar, etc.)
    0,                      //  ubyte  borderWidth;                         //width, in pixels, of the border
    0,                      //  uword  tabstop;                             //denotes the tab ordering of UI controls
    150,                    //  color  borderColor;                         //optional color of border
    4,                      //  color  contentColor;                        //optional color of content
    0,0,                      //  sdword x;                                   //-+
    0,0,                      //  sdword y;                                   // |>rectangle of region
    640,0,                    //  sdword width;                               // |
    480,0,                    //  sdword height;                              //-+
    NULL,                   //  ubyte *pData;                               //pointer to type-specific data
    NULL,                   //  ubyte *attribs;                             //sound(button atom) or font(static text atom) reference
    0,                      //  char   hotKeyModifiers;
    {0},                    //  char   hotKey[FE_NumberLanguages];
    {0},                    //  char   pad2[2]
    {0},                    //  udword drawstyle[2];
    0,                      //  void*  region;
    {0}                     //  udword pad[2];
};


featom tutTextAtom =
{
    "TutorialText",         //  char  *name;                                //optional name of control
    FAF_Function,           //  udword flags;                               //flags to control behavior
    0,                      //  udword status;                              //status flags for this atom, checked etc.
    FA_UserRegion,          //  ubyte  type;                                //type of control (button, scroll bar, etc.)
    0,                      //  ubyte  borderWidth;                         //width, in pixels, of the border
    0,                      //  uword  tabstop;                             //denotes the tab ordering of UI controls
    150,                    //  color  borderColor;                         //optional color of border
    4,                      //  color  contentColor;                        //optional color of content
    0,0,                      //  sdword x;                                   //-+
    0,0,                      //  sdword y;                                   // |>rectangle of region
    0,0,                      //  sdword width;                               // |
    0,0,                      //  sdword height;                              //-+
    NULL,                   //  ubyte *pData;                               //pointer to type-specific data
    NULL,                   //  ubyte *attribs;                             //sound(button atom) or font(static text atom) reference
    0,                      //  char   hotKeyModifiers;
    {0},                    //  char   hotKey[FE_NumberLanguages];
    {0},                    //  char   pad2[2]
    {0},                    //  udword drawstyle[2];
    0,                      //  void*  region;
    {0}                     //  udword pad[2];
};

featom tutImageAtom =
{
    "TutorialImage",        //  char  *name;                                //optional name of control
    FAF_Function,           //  udword flags;                               //flags to control behavior
    0,                      //  udword status;                              //status flags for this atom, checked etc.
    FA_UserRegion,          //  ubyte  type;                                //type of control (button, scroll bar, etc.)
    0,                      //  ubyte  borderWidth;                         //width, in pixels, of the border
    0,                      //  uword  tabstop;                             //denotes the tab ordering of UI controls
    150,                    //  color  borderColor;                         //optional color of border
    4,                      //  color  contentColor;                        //optional color of content
    0,0,                      //  sdword x;                                   //-+
    0,0,                      //  sdword y;                                   // |>rectangle of region
    0,0,                      //  sdword width;                               // |
    0,0,                      //  sdword height;                              //-+
    NULL,                   //  ubyte *pData;                               //pointer to type-specific data
    NULL,                   //  ubyte *attribs;                             //sound(button atom) or font(static text atom) reference
    0,                      //  char   hotKeyModifiers;
    {0},                    //  char   hotKey[FE_NumberLanguages];
    {0},                    //  char   pad2[2]
    {0},                    //  udword drawstyle[2];
    0,                      //  void*  region;
    {0}                     //  udword pad[2];
};

featom tutNextAtom =
{
    "TutorialNext",         //  char  *name;                                //optional name of control
    FAF_Function,           //  udword flags;                               //flags to control behavior
    0,                      //  udword status;                              //status flags for this atom, checked etc.
    FA_Button,              //  ubyte  type;                                //type of control (button, scroll bar, etc.)
    0,                      //  ubyte  borderWidth;                         //width, in pixels, of the border
    0,                      //  uword  tabstop;                             //denotes the tab ordering of UI controls
    150,                    //  color  borderColor;                         //optional color of border
    4,                      //  color  contentColor;                        //optional color of content
    0,0,                      //  sdword x;                                   //-+
    0,0,                      //  sdword y;                                   // |>rectangle of region
    0,0,                      //  sdword width;                               // |
    0,0,                      //  sdword height;                              //-+
    NULL,                   //  ubyte *pData;                               //pointer to type-specific data
    NULL,                   //  ubyte *attribs;                             //sound(button atom) or font(static text atom) reference
    0,                      //  char   hotKeyModifiers;
    {0},                    //  char   hotKey[FE_NumberLanguages];
    {0},                    //  char   pad2[2]
    {0},                    //  udword drawstyle[2];
    0,                      //  void*  region;
    {0}                     //  udword pad[2];
};

featom tutBackAtom =
{
    "TutorialBack",         //  char  *name;                                //optional name of control
    FAF_Function,           //  udword flags;                               //flags to control behavior
    0,                      //  udword status;                              //status flags for this atom, checked etc.
    FA_Button,              //  ubyte  type;                                //type of control (button, scroll bar, etc.)
    0,                      //  ubyte  borderWidth;                         //width, in pixels, of the border
    0,                      //  uword  tabstop;                             //denotes the tab ordering of UI controls
    150,                    //  color  borderColor;                         //optional color of border
    4,                      //  color  contentColor;                        //optional color of content
    0,0,                      //  sdword x;                                   //-+
    0,0,                      //  sdword y;                                   // |>rectangle of region
    0,0,                      //  sdword width;                               // |
    0,0,                      //  sdword height;                              //-+
    NULL,                   //  ubyte *pData;                               //pointer to type-specific data
    NULL,                   //  ubyte *attribs;                             //sound(button atom) or font(static text atom) reference
    0,                      //  char   hotKeyModifiers;
    {0},                    //  char   hotKey[FE_NumberLanguages];
    {0},                    //  char   pad2[2]
    {0},                    //  udword drawstyle[2];
    0,                      //  void*  region;
    {0}                     //  udword pad[2];
};


#define TUT_NUM_IMAGES 33

#define TUT_NEXT_ON     0
#define TUT_NEXT_OFF    1
#define TUT_NEXT_MOUSE  2
#define TUT_PREV_ON     3
#define TUT_PREV_OFF    4
#define TUT_PREV_MOUSE  5
#define TUT_REST_ON     6
#define TUT_REST_OFF    7
#define TUT_REST_MOUSE  8

char *tutImageList[TUT_NUM_IMAGES] =
{
    "Tut_Next_On",
    "Tut_Next_Off",
    "Tut_Next_Mouse",
    "Tut_previous_on",
    "Tut_previous_off",
    "Tut_previous_mouse",
    "Tut_restart_on",
    "Tut_restart_off",
    "Tut_restart_mouse",
    "Tut_Rightclick",
    "Tut_Key_F",
    "Tut_Key_M",
    "Tut_Key_Shift",
    "Tut_Key_Ctrl",
    "Tut_Key_Alt",
    "Tut_Key_Space",
    "Tut_Key_Tab",
    "Tut_Mouse_ButtonR",
    "Tut_Mouse_ButtonR_MoveLRUD",
    "Tut_Mouse_ButtonR_MoveLR",
    "Tut_Mouse_ButtonR_MoveU",
    "Tut_Mouse_ButtonR_MoveD",
    "Tut_Mouse_ButtonLR_MoveUD",
    "Tut_Mouse_ButtonLR_MoveU",
    "Tut_Mouse_ButtonLR_MoveD",
    "Tut_Mouse_ButtonL",
    "Tut_Mouse_ButtonL_MoveLRUD",
    "Tut_Mouse_ButtonM",
    "Tut_Mouse_Middle_RollD",
    "Tut_Mouse_Middle_RollU",
    "Tut_Mouse_Middle_RollUD",
    "Tut_Mouse_Lbandbox",
    ""
};

char *tutFrenchImageList[TUT_NUM_IMAGES] =
{
    "Tut_fr_Next_On",
    "Tut_fr_Next_Off",
    "Tut_fr_Next_Mouse",
    "Tut_fr_previous_on",
    "Tut_fr_previous_off",
    "Tut_fr_previous_mouse",
    "Tut_fr_restart_on",
    "Tut_fr_restart_off",
    "Tut_fr_restart_mouse",
    "Tut_Rightclick",
    "Tut_Key_F",
    "Tut_Key_D",
    "Tut_fr_Key_Shift",
    "Tut_fr_Key_Ctrl",
    "Tut_Key_Alt",
    "Tut_fr_Key_Space",
    "Tut_Key_Tab",
    "Tut_Mouse_ButtonR",
    "Tut_Mouse_ButtonR_MoveLRUD",
    "Tut_Mouse_ButtonR_MoveLR",
    "Tut_Mouse_ButtonR_MoveU",
    "Tut_Mouse_ButtonR_MoveD",
    "Tut_Mouse_ButtonLR_MoveUD",
    "Tut_Mouse_ButtonLR_MoveU",
    "Tut_Mouse_ButtonLR_MoveD",
    "Tut_Mouse_ButtonL",
    "Tut_Mouse_ButtonL_MoveLRUD",
    "Tut_Mouse_ButtonM",
    "Tut_Mouse_Middle_RollD",
    "Tut_Mouse_Middle_RollU",
    "Tut_Mouse_Middle_RollUD",
    "Tut_Mouse_Lbandbox",
    ""
};

char *tutGermanImageList[TUT_NUM_IMAGES] =
{
    "Tut_ger_Next_On",
    "Tut_ger_Next_Off",
    "Tut_ger_Next_Mouse",
    "Tut_ger_previous_on",
    "Tut_ger_previous_off",
    "Tut_ger_previous_mouse",
    "Tut_ger_restart_on",
    "Tut_ger_restart_off",
    "Tut_ger_restart_mouse",
    "Tut_Rightclick",
    "Tut_Key_F",
    "Tut_Key_W",
    "Tut_ger_Key_Shift",
    "Tut_ger_Key_Ctrl",
    "Tut_Key_Alt",
    "Tut_ger_Key_Space",
    "Tut_Key_Tab",
    "Tut_Mouse_ButtonR",
    "Tut_Mouse_ButtonR_MoveLRUD",
    "Tut_Mouse_ButtonR_MoveLR",
    "Tut_Mouse_ButtonR_MoveU",
    "Tut_Mouse_ButtonR_MoveD",
    "Tut_Mouse_ButtonLR_MoveUD",
    "Tut_Mouse_ButtonLR_MoveU",
    "Tut_Mouse_ButtonLR_MoveD",
    "Tut_Mouse_ButtonL",
    "Tut_Mouse_ButtonL_MoveLRUD",
    "Tut_Mouse_ButtonM",
    "Tut_Mouse_Middle_RollD",
    "Tut_Mouse_Middle_RollU",
    "Tut_Mouse_Middle_RollUD",
    "Tut_Mouse_Lbandbox",
    ""
};

char *tutSpanishImageList[TUT_NUM_IMAGES] =
{
    "Tut_sp_Next_On",
    "Tut_sp_Next_Off",
    "Tut_sp_Next_Mouse",
    "Tut_sp_previous_on",
    "Tut_sp_previous_off",
    "Tut_sp_previous_mouse",
    "Tut_sp_restart_on",
    "Tut_sp_restart_off",
    "Tut_sp_restart_mouse",
    "Tut_Rightclick",
    "Tut_Key_F",
    "Tut_Key_D",
    "Tut_sp_Key_Shift",
    "Tut_Key_Ctrl",
    "Tut_Key_Alt",
    "Tut_sp_Key_Space",
    "Tut_Key_Tab",
    "Tut_Mouse_ButtonR",
    "Tut_Mouse_ButtonR_MoveLRUD",
    "Tut_Mouse_ButtonR_MoveLR",
    "Tut_Mouse_ButtonR_MoveU",
    "Tut_Mouse_ButtonR_MoveD",
    "Tut_Mouse_ButtonLR_MoveUD",
    "Tut_Mouse_ButtonLR_MoveU",
    "Tut_Mouse_ButtonLR_MoveD",
    "Tut_Mouse_ButtonL",
    "Tut_Mouse_ButtonL_MoveLRUD",
    "Tut_Mouse_ButtonM",
    "Tut_Mouse_Middle_RollD",
    "Tut_Mouse_Middle_RollU",
    "Tut_Mouse_Middle_RollUD",
    "Tut_Mouse_Lbandbox",
    ""
};

char *tutItalianImageList[TUT_NUM_IMAGES] =
{
    "Tut_it_Next_On",
    "Tut_it_Next_Off",
    "Tut_it_Next_Mouse",
    "Tut_it_previous_on",
    "Tut_it_previous_off",
    "Tut_it_previous_mouse",
    "Tut_it_restart_on",
    "Tut_it_restart_off",
    "Tut_it_restart_mouse",
    "Tut_Rightclick",
    "Tut_Key_F",
    "Tut_Key_M",
    "Tut_Key_Shift",
    "Tut_Key_Ctrl",
    "Tut_Key_Alt",
    "Tut_it_Key_Space",
    "Tut_Key_Tab",
    "Tut_Mouse_ButtonR",
    "Tut_Mouse_ButtonR_MoveLRUD",
    "Tut_Mouse_ButtonR_MoveLR",
    "Tut_Mouse_ButtonR_MoveU",
    "Tut_Mouse_ButtonR_MoveD",
    "Tut_Mouse_ButtonLR_MoveUD",
    "Tut_Mouse_ButtonLR_MoveU",
    "Tut_Mouse_ButtonLR_MoveD",
    "Tut_Mouse_ButtonL",
    "Tut_Mouse_ButtonL_MoveLRUD",
    "Tut_Mouse_ButtonM",
    "Tut_Mouse_Middle_RollD",
    "Tut_Mouse_Middle_RollU",
    "Tut_Mouse_Middle_RollUD",
    "Tut_Mouse_Lbandbox",
    ""
};

// This string structure must match the tutGameEnableFlags struct
char *tutGameEnableString[] =
{
    "KASFrame",     // FOR INTERAL USE ONLY
    "GameRunning",


    "BuildManager",
    "SensorsManager",
    "ResearchManager",
    "PauseGame",
    "Dock",
    "Formation",
    "Launch",
    "Move",
    "MoveIssue",
    "Attack",
    "Harvest",
    "CancelCommand",
    "Scuttle",
    "Retire",
    "ClickSelect",
    "BandSelect",
    "CancelSelect",
    "Special",
    "BuildBuildShips",
    "BuildPauseJobs",
    "BuildCancelJobs",
    "BuildClose",
    "BuildArrows",
    "SensorsClose",
    "ContextMenus",
    "ContextFormDelta",
    "ContextFormBroad",
    "ContextFormX",
    "ContextFormClaw",
    "ContextFormWall",
    "ContextFormSphere",
    "ContextFormCustom",
    "Evasive",
    "Neutral",
    "Agressive",
    "TaskbarOpen",
    "TaskbarClose",
    "ResearchSelectTech",
    "ResearchSelectLab",
    "ResearchResearch",
    "ResearchClearLab",
    "ResearchClose",
    "Focus",
    "FocusCancel",
    "MothershipFocus",
    "LaunchSelectShips",
    "LaunchLaunch",
    "LaunchLaunchAll",
    "LaunchClose",
    ""
};

color       tutTexture[TUT_NUM_IMAGES];
lifheader   *tutImage[TUT_NUM_IMAGES];


static char *tutTutorialNames[4] = {"", "Tutorial1"};

void tutPreInitTutorial(char *dirfile, char *levelfile)
{
#ifdef _WIN32
    sprintf(dirfile, "Tutorials\\%s\\", tutTutorialNames[tutorial]);
#else
    sprintf(dirfile, "Tutorials/%s/", tutTutorialNames[tutorial]);
#endif
    sprintf(levelfile, "%s.mission", tutTutorialNames[tutorial]);

    tutEnableEverything();

    //perform a level pass to see what ships we need to load
    levelPreInit(dirfile, levelfile);
}


void tutInitTutorial(char *dirfile, char *levelfile)
{
    levelInit(dirfile, levelfile);
    tutLastLessonName[0] = '\0';
    tutCurrLessonName[0] = '\0';

    switch(tutorial)
    {
    case 1:
        kasMissionStart("tutorial1", Init_Tutorial1, Watch_Tutorial1);
        break;
    }
}

void tutSaveLesson(sdword Num, char *pName)
{
    collUpdateCollBlobs();
    collUpdateObjsInCollBlobs();

    strcpy(tutLastLessonName, tutCurrLessonName);

    sprintf(tutCurrLessonName, "%s%s", TutorialSavedGamesPath, pName);
    SaveGame(tutCurrLessonName);
}

static void SaveLong(sdword Val)
{
    SaveStructureOfSize( &Val, 4 );
}

static sdword LoadLong(void)
{
sdword Val;

    LoadStructureOfSizeToAddress( &Val, 4 );
    return Val;
}

void tutSaveTutorialGame(void)
{
    Save_String(tutCurrLessonName);
    Save_String(tutLastLessonName);

    SaveStructureOfSize( &tutEnable, sizeof(tutGameEnableFlags) );
    SaveStructureOfSize( tutBuildRestrict, sizeof(tutBuildRestrict) );

    SaveLong(tutTextPosX);
    SaveLong(tutTextPosY);
    SaveLong(tutTextSizeX);
    SaveLong(tutTextSizeY);

    SaveStructureOfSize( &tutTextRect, sizeof(rectangle) );

    SaveLong(tutTextVisible);
    SaveLong(tutNextVisible);
    SaveLong(tutBackVisible);
    SaveLong(FalkosFuckedUpTutorialFlag);
}

void tutLoadTutorialGame(void)
{
    sdword index;

    Load_StringToAddress(tutCurrLessonName);
    Load_StringToAddress(tutLastLessonName);

    LoadStructureOfSizeToAddress( &tutEnable, sizeof(tutGameEnableFlags) );
    LoadStructureOfSizeToAddress( tutBuildRestrict, sizeof(tutBuildRestrict) );

    tutTextPosX = LoadLong();
    tutTextPosY = LoadLong();
    tutTextSizeX = LoadLong();
    tutTextSizeY = LoadLong();

    LoadStructureOfSizeToAddress( &tutTextRect, sizeof(rectangle) );

    tutTextVisible = LoadLong();
    tutNextVisible = LoadLong();
    tutBackVisible = LoadLong();
    FalkosFuckedUpTutorialFlag = LoadLong();

    tutGameMessageIndex = 0;
    tutNextButtonState = 0;
    //tutTextPointerType = 0;

    //clear out all the pointers
    for (index = 0; index < TUT_NumberPointers; index++)
    {
        tutPointer[index].pointerType = TUT_PointerTypeNone;
        tutPointerShip = NULL;
        tutPointerShipGroupRect = tutPointerShipHealthRect = NULL;
    }
    tutFEContextMenuShipType = 0;
}

void tutTutorial1(char *name, featom *atom)
{
    static bool beginning;

    if (FEFIRSTCALL(atom))
    {
        beginning = FALSE;
    }
    else if (FELASTCALL(atom))
    {
        if (!beginning)
        {
            tutorial = 0;
        }
    }
    else
    {
        beginning = TRUE;
        tutorial = 1;
        dbgAssert(startingGame == FALSE);
        dbgMessagef("\nTutorial1 started");

        utySinglePlayerGameStart(name, atom);
    }
}

/*-----------------------------------------------------------------------------
    Name        : tutPointerAllocate
    Description : Allocate a tutorial pointer from the pointer list.
    Inputs      : name - name of pointer.  Will overwrite pointers of same name.
                  type - type of pointer to allocate.
    Outputs     :
    Return      : pointer to pointer.  Quaint isn't it?
----------------------------------------------------------------------------*/
tutpointer *tutPointerAllocate(char *name, sdword type)
{
    sdword index, freeIndex = -1;

    for (index = 0; index < TUT_NumberPointers; index++)
    {                                                       //make sure this name is unique
        if (!strcmp(name, tutPointer[index].name))
        {                                                   //if this one is the same name
            if (tutPointer[index].ship == tutPointerShip)
            {
                tutPointerShip = NULL;
                tutPointerShipGroupRect = tutPointerShipHealthRect = NULL;
            }
            freeIndex = index;
            break;
        }
        if (tutPointer[index].pointerType == TUT_PointerTypeNone)
        {                                                   //if this one is free
            freeIndex = index;
        }
    }
#ifndef HW_Release
    if (freeIndex < 0)
    {
        dbgFatalf(DBG_Loc, "Cannot allocate tutorial pointer '%s'.", name);
    }
#endif
    memStrncpy(tutPointer[freeIndex].name, name, TUT_PointerNameMax);
    tutPointer[freeIndex].pointerType = type;
    return(&tutPointer[freeIndex]);
}

void tutSetPointerTargetXY(char *name, sdword x, sdword y)
{
    tutpointer *pointer = tutPointerAllocate(name, TUT_PointerTypeXY);
    pointer->x = x;
    pointer->y = y;
}

//sets the tutorial pointer relative to the right side of the screen
void tutSetPointerTargetXYRight(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXY(name, MAIN_WindowWidth+x-640, y);
}

//sets the tutorial pointer relative to the bottom right corner of the screen
void tutSetPointerTargetXYBottomRight(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXY(name, MAIN_WindowWidth+x-640, MAIN_WindowHeight+y-480);
}

void tutSetPointerTargetXYTaskbar(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXY(name, feResRepositionX(x), MAIN_WindowHeight+y-480);
}

void tutSetPointerTargetXYFE(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXY(name, feResRepositionX(x), feResRepositionY(y));
}

void tutSetPointerTargetShip(char *name, ShipPtr ship)
{
    tutpointer *pointer = tutPointerAllocate(name, TUT_PointerTypeShip);
    pointer->ship = ship;
}

void tutSetPointerTargetShipSelection(char *name, SelectCommand *ships)
{
    tutpointer *pointer = tutPointerAllocate(name, TUT_PointerTypeShips);
    pointer->selection = ships;
}

void tutSetPointerTargetShipHealth(char *name, ShipPtr ship)
{
    tutpointer *pointer = tutPointerAllocate(name, TUT_PointerTypeShipHealth);
    pointer->ship = ship;
    tutPointerShip = ship;
    tutPointerShipHealthRect = &pointer->rect;
}

void tutSetPointerTargetShipGroup(char *name, ShipPtr ship)
{
    tutpointer *pointer = tutPointerAllocate(name, TUT_PointerTypeShipGroup);
    pointer->ship = ship;
    tutPointerShip = ship;
    tutPointerShipGroupRect = &pointer->rect;
}

void tutPlayerShipDied(ShipPtr ship)
{
    sdword index;

    for (index = 0; index < TUT_NumberPointers; index++)
    {
        if (tutPointer[index].ship == ship)
        {
            tutPointer[index].pointerType = TUT_PointerTypeNone;
            tutPointer[index].ship = NULL;
        }
    }
    if(ship == tutPointerShip)
    {
        //tutTextPointerType = TUT_PointerTypeNone;
        tutPointerShip = NULL;
        tutPointerShipGroupRect = tutPointerShipHealthRect = NULL;
    }
}

void tutSetPointerTargetFERegion(char *name, char *pAtomName)
{
    regionhandle    reg;

    reg = regFindChildByAtomName(&regRootRegion, pAtomName);

    if(reg)
    {
        tutpointer *pointer = tutPointerAllocate(name, TUT_PointerTypeRegion);
        pointer->rect = reg->rect;
        pointer->rect.x0 -= 1;
        pointer->rect.y0 -= 1;
        pointer->rect.y1 += 2;
    }
}

void tutSetPointerTargetRect(char *name, sdword x0, sdword y0, sdword x1, sdword y1)
{
    tutpointer *pointer = tutPointerAllocate(name, TUT_PointerTypeRegion);
    pointer->rect.x0 = x0;
    pointer->rect.y0 = y0;
    pointer->rect.x1 = x1;
    pointer->rect.y1 = y1;
}

void tutSetPointerTargetAIVolume(char *name, Volume *volume)
{
    if (volume != NULL)
    {
        tutpointer *pointer = tutPointerAllocate(name, TUT_PointerTypeAIVolume);
        pointer->volume = volume;
    }
}

/*-----------------------------------------------------------------------------
    Name        : tutRemovePointerByName
    Description : Turn off a named pointer.
    Inputs      : name - name of pointer to remove
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tutRemovePointerByName(char *name)
{
    sdword index;

    for (index = 0; index < TUT_NumberPointers; index++)
    {
        if (!strcmp(tutPointer[index].name, name))
        {
            tutPointer[index].pointerType = TUT_PointerTypeNone;
            if (tutPointer[index].ship == tutPointerShip)
            {
                tutPointerShip = NULL;
                tutPointerShipGroupRect = tutPointerShipHealthRect = NULL;
            }
            return;
        }
    }
#ifndef HW_Debug
    dbgMessagef("\ntutRemovePointerByName: '%s' not found.", name);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : tutRemoveAllPointers
    Description : Remove all pointers.  Gone.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tutRemoveAllPointers(void)
{
    sdword index;

    for (index = 0; index < TUT_NumberPointers; index++)
    {
        if (tutPointer[index].pointerType != TUT_PointerTypeNone)
        {
            tutPointer[index].pointerType = TUT_PointerTypeNone;
            if (tutPointer[index].ship == tutPointerShip)
            {
                tutPointerShip = NULL;
                tutPointerShipGroupRect = tutPointerShipHealthRect = NULL;
            }
        }
    }
}

// This function sets the current text display position and size
void tutSetTextDisplayBox(sdword x, sdword y, sdword width, sdword height, bool bScale)
{
    if(bScale)
    {
        x = (long)( ((real32)x / 640.0f) * (real32)MAIN_WindowWidth );
        y = (long)( ((real32)y / 480.0f) * (real32)MAIN_WindowHeight );
    }
    else
    {
        x += ((MAIN_WindowWidth - 640) / 2);
        y += ((MAIN_WindowHeight - 480) / 2);
    }

    tutTextPosX = x+5;
    tutTextPosY = y+5;
    tutTextSizeX = width-10;
    tutTextSizeY = height-10;

    tutTextRect.x0 = x;
    tutTextRect.y0 = y;
    tutTextRect.x1 = x + width;
    tutTextRect.y1 = y + height;
}

/*-----------------------------------------------------------------------------
    Name        : tutSetTextDisplayBoxToSubtitleRegion
    Description : Causes pointers to be clipped to the subtitle region
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tutSetTextDisplayBoxToSubtitleRegion(void)
{
    tutTextRect = subRegion[STR_LetterboxBar].rect;
}

void tutRootDrawFunction(regionhandle reg)
{
    sdword index;
    regionhandle escapeRegion = NULL;

    if (reg->previous)
    {
        for (index = 0; index <= feStackIndex; index++)
        {
            if (feStack[index].screen != NULL)
            {
                if (strcmp(feStack[index].screen->name, "Construction_manager") &&
                    strcmp(feStack[index].screen->name, "Launch_Manager") &&
                    strcmp(feStack[index].screen->name, "Research_Manager") &&
                    strcmp(feStack[index].screen->name, "Sensors_manager"))
                {                                           //if it's not one of the screens the turorial teaches
                    escapeRegion = feStack[index].baseRegion;
                    break;
                }
            }
        }
        //we want the tutorial stuff to be on top of all menu screens EXCEPT the in-game escape menu and it's minions
        if (escapeRegion == NULL)
        {                                                   //don't put it on top of the build manager or any other base-level manager.
            regSiblingMoveToFront(reg);
        }
    }
}

void tutAllocateRootRegion(void)
{
    if(tutRootRegionCount == 0)
    {
        tutRootRegion = regChildAlloc(ghMainRegion, (sdword)&tutRootAtom,
            tutRootAtom.x, tutRootAtom.y, tutRootAtom.width, tutRootAtom.height, 0, RPE_DrawEveryFrame);

        tutRootAtom.region = (void*)tutRootRegion;
        tutRootRegion->atom = &tutRootAtom;
        tutRootAtom.pData = NULL;

        regDrawFunctionSet(tutRootRegion, tutRootDrawFunction);
    }

    tutRootRegionCount++;
}

void tutDeallocateRootRegion(void)
{
    tutRootRegionCount--;

    if(tutRootRegionCount == 0)
    {
        if(tutRootRegion)
            regRegionDelete(tutRootRegion);

        tutRootRegion = NULL;
    }
}

// This function will accept a string to be displayed in the current text box.
// The current text box is set up with the tutSetTextDisplayBox() function.
void tutShowText(char *szText)
{
fonthandle  currFont;
sdword      Height;
char        Line[256], *pString;

    if(tutTextVisible)
        tutHideText();

    if (MAIN_WindowWidth >= 1024)
    {
        tutTextFont = frFontRegister("ScrollingText.hff");
    }
    else
    {
        tutTextFont = frFontRegister("SimplixSSK_13.hff");
    }

    Height = 0;
    pString = szText;
    currFont = fontMakeCurrent(tutTextFont);

    do {
        pString = tutGetNextTextLine(Line, pString, tutTextSizeX);
        Height += fontHeight(" ");
    } while(pString && pString[0]);

    fontMakeCurrent(currFont);

    tutAllocateRootRegion();

    tutTextAtom.x = tutTextPosX-5;
    tutTextAtom.y = tutTextPosY-5;
    tutTextAtom.width = tutTextSizeX + 10;
    tutTextAtom.height = max(Height, tutTextSizeY)+10;

    tutTextRect.x0 = tutTextPosX-5;
    tutTextRect.y0 = tutTextPosY-5;
    tutTextRect.x1 = tutTextRect.x0 + tutTextSizeX + 10;
    tutTextRect.y1 = tutTextRect.y0 + max(Height, tutTextSizeY) + 10;

    tutTextRegion = regChildAlloc(tutRootRegion, (sdword)&tutTextAtom,
        tutTextAtom.x, tutTextAtom.y, tutTextAtom.width, tutTextAtom.height, 0, 0);

    tutTextAtom.region = (void*)tutTextRegion;
    tutTextRegion->atom = &tutTextAtom;
    tutTextAtom.pData = (ubyte *)szText;

    tutTextVisible = TRUE;

    regDrawFunctionSet(tutTextRegion, tutDrawTextFunction);
}

void tutHideText(void)
{
//    sdword index;

    if(tutTextRegion)
    {
        regRegionDelete(tutTextRegion);
        tutDeallocateRootRegion();
    }

    tutTextRegion = NULL;
    tutTextVisible = FALSE;

    tutRemoveAllPointers();

    //kills the speech associated with the text
    speechEventActorStop(ACTOR_FLEETCOMMAND_FLAG, 0.1f);
}


/*-----------------------------------------------------------------------------
    Name        : tutIsspace
    Description : Returns TRUE if the character is a "space" character
    Inputs      : c - the character to test
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool tutIsspace(char c)
{
    if (c==' '||c=='\n')
    {
        return TRUE;
    }
    return FALSE;
}

// This function gets a line of text that is up to Width pixels wide, and returns a
// pointer to the start of the next line.  Assumes the current font is set.
char *tutGetNextTextLine(char *pDest, char *pString, long Width)
{
long    WordLen, StringLen;
char    *pstr;
long    Done = FALSE;
char    temp;

    StringLen = 0;
    WordLen = 0;
    *pDest = 0;

    if(pString[0] == 0)
        return NULL;

    do {
        // Skip leading whitespace
        pstr = &pString[StringLen];
        while( *pstr && *pstr != '\n' && (*pstr == '-' || tutIsspace(*pstr)) )
        {
            WordLen++;
            pstr++;
        }

        if(*pstr && *pstr != '\n')
        {
            while( *pstr && *pstr != '\n' && *pstr != '-' && !tutIsspace(*pstr) )
            {
                WordLen++;
                pstr++;
            }

            temp = *pstr;
            *pstr = 0;
            if(fontWidth(pString) > Width)
                Done = TRUE;
            else
            {
                StringLen += WordLen;
                WordLen = 0;
            }
            *pstr = temp;
        }
        else
        {
            Done = TRUE;
            StringLen += WordLen;
        }
    } while(!Done);

    if(StringLen)
    {
        memStrncpy(pDest, pString, StringLen+1);
        //pDest[StringLen] = 0;

        while( pString[StringLen] && tutIsspace(pString[StringLen]) )
            StringLen++;
    }
    else
        return NULL;

    return pString + StringLen;
}

static long PulseVal(long Val)
{
static long pulseDir = TUT_PointerPulseInc;

    Val += pulseDir;
    if(Val >= 255 || Val <= TUT_PointerPulseMin)
    {
        pulseDir = -pulseDir;
        Val += pulseDir;
    }
    return Val;
}

// long vector type for the clipper
typedef struct {
    sdword x,y;
} lvector;


// Clips segment to a horizontal line at y.
// Puts the intersection point in the return vector at *pDest
// returns true if clipped, false otherwise
sdword tutClipSegToHorizontal(lvector *pSeg, lvector *pDest, sdword y)
{
sdword dy0, dy1;
real32 t;

    dy0 = pSeg[1].y - pSeg[0].y;
    dy1 = y - pSeg[0].y;

    t = (real32)dy1 / dy0;

    if(t > 0.0f && t < 1.0f)
    {
        pDest->y = y;
        pDest->x = pSeg[0].x + (sdword)((real32)(pSeg[1].x - pSeg[0].x) * t + 0.5f);

        return 1;
    }
    else
        return 0;
}

// Clips segment to a vertical line at x.
// Puts the intersection point in the return vector at *pDest
// returns true if clipped, false otherwise
sdword tutClipSegToVertical(lvector *pSeg, lvector *pDest, sdword x)
{
sdword dx0, dx1;
real32 t;

    dx0 = pSeg[1].x - pSeg[0].x;
    dx1 = x - pSeg[0].x;

    t = (real32)dx1 / dx0;

    if(t > 0.0f && t < 1.0f)
    {
        pDest->x = x;
        pDest->y = pSeg[0].y + (sdword)((real32)(pSeg[1].y - pSeg[0].y) * t + 0.5f);

        return 1;
    }
    else
        return 0;
}


void tutClipSegToTextBox(sdword *x0, sdword *y0, sdword *x1, sdword *y1)
{
lvector ClipSeg[2];
lvector Clipped;

    ClipSeg[0].x = *x0; ClipSeg[0].y = *y0;
    ClipSeg[1].x = *x1; ClipSeg[1].y = *y1;

    if(tutClipSegToHorizontal(ClipSeg, &Clipped, tutTextRect.y0))
    {
        if(Clipped.x >= tutTextRect.x0 && Clipped.x <= tutTextRect.x1)
            ClipSeg[0] = Clipped;
    }

    if(tutClipSegToHorizontal(ClipSeg, &Clipped, tutTextRect.y1))
    {
        if(Clipped.x >= tutTextRect.x0 && Clipped.x <= tutTextRect.x1)
            ClipSeg[0] = Clipped;
    }

    if(tutClipSegToVertical(ClipSeg, &Clipped, tutTextRect.x0))
    {
        if(Clipped.y >= tutTextRect.y0 && Clipped.y <= tutTextRect.y1)
            ClipSeg[0] = Clipped;
    }

    if(tutClipSegToVertical(ClipSeg, &Clipped, tutTextRect.x1))
    {
        if(Clipped.y >= tutTextRect.y0 && Clipped.y <= tutTextRect.y1)
            ClipSeg[0] = Clipped;
    }

    *x0 = ClipSeg[0].x; *y0 = ClipSeg[0].y;
    *x1 = ClipSeg[1].x; *y1 = ClipSeg[1].y;
}



/*-----------------------------------------------------------------------------
    Name        : tutDrawTextPointers
    Description : Draw all active tutorial pointers
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tutDrawTextPointers(rectangle *pRect)
{
    sdword  x0, y0, x1, y1, index;
    real32  x, y, rad, angle, dx, dy, magnitude;
    static long tutPulse = TUT_PointerPulseMin;
    //hmatrix modelview, projection;
    real32  sx, sy;
    tutpointer *pointer;
    color c;
    vector temp;

    for (pointer = tutPointer, index = 0; index < TUT_NumberPointers; index++, pointer++)
    {                                                       //for each pointer
        switch(pointer->pointerType)
        {
            case TUT_PointerTypeNone:
                break;                                      //not active; ignore

            case TUT_PointerTypeXY:
                x0 = (pRect->x0 + pRect->x1) / 2;
                y0 = (pRect->y0 + pRect->y1) / 2;
                x1 = pointer->x;//tutPointerX;
                y1 = pointer->y;//tutPointerY;

                tutClipSegToTextBox(&x0, &y0, &x1, &y1);
                c = colRGB(tutPulse, tutPulse, tutPulse);
                primLine2(x0, y0, x1, y1, c);
                dx = (real32)(x0 - x1);                     //vector from arrowhead to source
                dy = (real32)(y0 - y1);
                if (dx != 0.0f || dy != 0.0f)
                {                                           //if not zero-length vector
                    magnitude = fmathSqrt(dx * dx + dy * dy);   //magnitude of vector
                    dx /= magnitude;
                    dy /= magnitude;                        //normalize the vector
                    if (dx == 0.0f)
                    {                                       //singularity: vertical axis
                        if (dy > 0.0f)
                        {
                            angle = PI / 2.0f;              //90
                        }
                        else
                        {
                            angle = PI * 3.0f / 2.0f;       //270
                        }
                    }
                    else if (dy > 0.0f)
                    {                                       //0..180
                        if (dx > 0.0f)
                        {                                   //0..90
                            angle = (real32)atan((double)(dy / dx));
                        }
                        else
                        {                                   //90..180
                            angle = PI - (real32)atan((double)(dy / -dx));
                        }
                    }
                    else
                    {                                       //180..360
                        if (dx < 0.0f)
                        {                                   //180..270
                            angle = (real32)atan((double)(dy / dx)) + PI;
                        }
                        else
                        {                                   //270..360
                            angle = 2.0 * PI - (real32)atan((double)(-dy / dx));
                        }
                    }
                    //now we have angle of main vector; offset that to get arrowhead vectors
                    x0 = x1 + (sdword)(cos((double)(angle - TUT_ArrowheadAngle)) * TUT_ArrowheadLength);
                    y0 = y1 + (sdword)(sin((double)(angle - TUT_ArrowheadAngle)) * TUT_ArrowheadLength);
                    primLine2(x0, y0, x1, y1, c);
                    x0 = x1 + (sdword)(cos((double)(angle + TUT_ArrowheadAngle)) * TUT_ArrowheadLength);
                    y0 = y1 + (sdword)(sin((double)(angle + TUT_ArrowheadAngle)) * TUT_ArrowheadLength);
                    primLine2(x0, y0, x1, y1, c);
                }
                break;

            case TUT_PointerTypeShip:
                rad = pointer->ship->collInfo.selCircleRadius;
                x = pointer->ship->collInfo.selCircleX;
                y = pointer->ship->collInfo.selCircleY;
                goto shipsCase;                             //rest of code is common

            case TUT_PointerTypeShips:
                selSelectionDimensions(&rndCameraMatrix, &rndProjectionMatrix, pointer->selection, &x, &y, &rad);
shipsCase:
                if(rad > 0.0)
                {                                           //if ship is on-screen
                    real32 sx, sy, deltx, delty, len;

                    rad = max(rad, TUT_ShipCircleSizeMin);
                    //x = pointer->ship->collInfo.selCircleX;
                    //y = pointer->ship->collInfo.selCircleY;

                    primGLCircleOutline2(x, y, rad, pieCircleSegmentsCompute(rad), colRGB(tutPulse/2, tutPulse, tutPulse/2));

                    sx = primScreenToGLX((pRect->x0 + pRect->x1) / 2);
                    sy = primScreenToGLY((pRect->y0 + pRect->y1) / 2);

                    deltx = x-sx;
                    delty = y-sy;

                    len = (real32)fsqrt(deltx*deltx + delty*delty);
                    len = (len - rad) / len;

                    if (len > 0.0f)
                    {
                        x0 = primGLToScreenX(sx);
                        y0 = primGLToScreenY(sy);

                        x1 = primGLToScreenX(sx + deltx * len);
                        y1 = primGLToScreenY(sy + delty * len);

                        tutClipSegToTextBox(&x0, &y0, &x1, &y1);
                        primLine2(x0, y0, x1, y1, colRGB(tutPulse/2, tutPulse, tutPulse/2));
                    }

                }
                break;

            case TUT_PointerTypeShipHealth:
                rad = pointer->ship->collInfo.selCircleRadius;
                if(rad > 0.0)
                {                                           //if ship on-screen
                    x0 = (pRect->x0 + pRect->x1) / 2;
                    y0 = (pRect->y0 + pRect->y1) / 2;

                    x1 = (pointer->rect.x0 + pointer->rect.x1) / 2;
                    y1 = pointer->rect.y0;

                    tutClipSegToTextBox(&x0, &x1, &y0, &y1);
                    primLine2(x0, y0, x1, y1, colRGB(tutPulse/2, tutPulse, tutPulse/2));
                    primRectOutline2(&pointer->rect, 1, colRGB(tutPulse/2, tutPulse, tutPulse/2));
                }
                break;

            case TUT_PointerTypeShipGroup:
                rad = pointer->ship->collInfo.selCircleRadius;
                if(rad > 0.0)
                {                                           //if ship on-screen
                    x0 = (pRect->x0 + pRect->x1) / 2;
                    y0 = (pRect->y0 + pRect->y1) / 2;

                    x1 = (pointer->rect.x0 + pointer->rect.x1) / 2;
                    y1 = pointer->rect.y0;

                    tutClipSegToTextBox(&x0, &y0, &x1, &y1);
                    primLine2(x0, y0, x1, y1, colRGB(tutPulse/2, tutPulse, tutPulse/2));
                    primRectOutline2(&pointer->rect, 1, colRGB(tutPulse/2, tutPulse, tutPulse/2));
                }
                break;

            case TUT_PointerTypeRegion:
                primRectOutline2(&pointer->rect, 3, colRGB(tutPulse, tutPulse, tutPulse));
                break;

            case TUT_PointerTypeAIVolume:
                temp = volFindCenter(pointer->volume);

                selCircleComputeGeneral(&rndCameraMatrix, &rndProjectionMatrix,
                    (vector *)&temp, volFindRadius(pointer->volume),
                    &sx, &sy, &rad);

                if(rad > 0.0)
                {
                    real32 x, y, deltx, delty, len;

                    rad = max(rad, TUT_ShipCircleSizeMin);
                    x = sx;
                    y = sy;
                    primGLCircleOutline2(x, y, rad, pieCircleSegmentsCompute(rad), colRGB(tutPulse/2, tutPulse, tutPulse/2));

                    sx = primScreenToGLX((pRect->x0 + pRect->x1) / 2);
                    sy = primScreenToGLY((pRect->y0 + pRect->y1) / 2);

                    deltx = x-sx;
                    delty = y-sy;

                    len = (real32)fsqrt(deltx*deltx + delty*delty);
                    len = (len - rad) / len;

                    x0 = primGLToScreenX(sx);
                    y0 = primGLToScreenY(sy);

                    x1 = primGLToScreenX(sx + deltx * len);
                    y1 = primGLToScreenY(sy + delty * len);

                    tutClipSegToTextBox(&x0, &y0, &x1, &y1);
                    primLine2(x0, y0, x1, y1, colRGB(tutPulse/2, tutPulse, tutPulse/2));
                }
                break;
        }
    }
    tutPulse = PulseVal(tutPulse);
    tutPointersDrawnThisFrame = TRUE;
}

// This function actually handles drawing the text in the region added by tutShowText
void tutDrawTextFunction(regionhandle reg)
{
char    *pString;
char    Line[256];
long    x, y, Width;
fonthandle  currFont;
featom  *pAtom = (featom *)reg->atom;
rectangle   rect;

    rect = reg->rect;
//    if(tutTextPointerType != TUT_PointerTypeNone)
    if (!tutPointersDrawnThisFrame)
    {
        tutDrawTextPointers(&rect);
    }

    // Draw the translucent blue box behind the text
    primRectTranslucent2(&rect, colRGBA(0, 0, 64, 192));

    currFont = fontMakeCurrent(tutTextFont);

    x = rect.x0+5;
    y = rect.y0+5;
    Width = (rect.x1 - rect.x0) - 10;

    pString = (char *)pAtom->pData;

    do {
        pString = tutGetNextTextLine(Line, pString, Width);
        if(Line[0])
            fontPrintf(x, y, colRGB(255, 255, 128), "%s", Line);
        y += fontHeight(" ");
    } while(pString && pString[0]);

    fontMakeCurrent(currFont);
}

void tutShowNextButton(void)
{
    if(tutNextVisible)
        tutHideNextButton();

    tutAllocateRootRegion();

    tutNextAtom.x = tutTextRect.x0 + 128;
    tutNextAtom.y = tutTextRect.y1;
    tutNextAtom.width = 64;
    tutNextAtom.height = 32;

    tutNextRegion = (buttonhandle)regChildAlloc(tutRootRegion, (sdword)&tutNextAtom,
        tutNextAtom.x, tutNextAtom.y, tutNextAtom.width, tutNextAtom.height, sizeof(buttonhandle) - sizeof(regionhandle), 0);

    tutNextAtom.region = (void*)tutNextRegion;
    tutNextRegion->reg.atom = &tutNextAtom;
    tutNextRegion->reg.flags |= RPM_MouseEvents;
    tutNextAtom.pData = NULL;

    tutNextRegion->reg.processFunction = uicButtonProcess;
    tutNextRegion->processFunction = tutProcessNextButton;
    tutNextVisible = TRUE;

    regDrawFunctionSet((regionhandle)tutNextRegion, tutDrawNextButtonFunction);
}

void tutHideNextButton(void)
{
    if(tutNextRegion)
    {
        regRegionDelete((regionhandle)tutNextRegion);
        tutDeallocateRootRegion();
    }

    tutNextRegion = NULL;
    tutNextVisible = FALSE;

}

udword tutProcessNextButton(struct tagRegion *reg, sdword ID, udword event, udword data)
{
    if(event == CM_ButtonClick)
    {
        tutNextButtonState = 1;
        return 1;
    }

    return 0;
}

void tutDrawNextButtonFunction(regionhandle reg)
{
    if(mouseInRect(&reg->rect))
    {
        if(mouseLeftButton())
            trRGBTextureMakeCurrent(tutTexture[TUT_NEXT_ON]);
        else
            trRGBTextureMakeCurrent(tutTexture[TUT_NEXT_MOUSE]);
    }
    else
        trRGBTextureMakeCurrent(tutTexture[TUT_NEXT_OFF]);

    rndPerspectiveCorrection(FALSE);
//  glEnable(GL_BLEND);
    primRectSolidTextured2(&reg->rect);
//  glDisable(GL_BLEND);
}

sdword tutNextButtonClicked(void)
{
sdword  RetVal;

    RetVal = tutNextButtonState;
    tutNextButtonState = 0;

    return RetVal;
}


void tutShowBackButton(void)
{
    if(tutBackVisible)
    {
        tutPrevVisible = FALSE;
        tutHideBackButton();
    }

    tutAllocateRootRegion();

    tutBackAtom.x = tutTextRect.x0;
    tutBackAtom.y = tutTextRect.y1;
    tutBackAtom.width = 128;
    tutBackAtom.height = 32;

    tutBackRegion = (buttonhandle)regChildAlloc(tutRootRegion, (sdword)&tutBackAtom,
        tutBackAtom.x, tutBackAtom.y, tutBackAtom.width, tutBackAtom.height, sizeof(buttonhandle) - sizeof(regionhandle), 0);

    tutBackAtom.region = (void*)tutBackRegion;
    tutBackRegion->reg.atom = &tutBackAtom;
    tutBackRegion->reg.flags |= RPM_MouseEvents;
    tutBackAtom.pData = NULL;

    tutBackRegion->reg.processFunction = uicButtonProcess;
    tutBackRegion->processFunction = tutProcessBackButton;
    tutBackVisible = TRUE;

    regDrawFunctionSet((regionhandle)tutBackRegion, tutDrawBackButtonFunction);
}

void tutShowPrevButton(void)
{
    if (tutBackVisible)
    {
        tutHideBackButton();
    }
    tutPrevVisible = TRUE;
    tutShowBackButton();
}

void tutHideBackButton(void)
{
    if(tutBackRegion)
    {
        regRegionDelete((regionhandle)tutBackRegion);
        tutDeallocateRootRegion();
    }

    tutBackRegion = NULL;
    tutBackVisible = FALSE;
    tutPrevVisible = FALSE;

}

udword tutProcessBackButton(struct tagRegion *reg, sdword ID, udword event, udword data)
{
    if(event == CM_ButtonClick)
    {
        if((tutPrevVisible) && (tutLastLessonName[0]))
            strcpy(tutCurrLessonName, tutLastLessonName);

        if (fileExists(tutCurrLessonName, 0) && (VerifySaveFile(tutCurrLessonName) == VERIFYSAVEFILE_OK))
        {
            tutEnableEverything();
            spMainScreen();
            tbForceTaskbar(FALSE);
            gameEnd();

            tutorial = 1;

            utyLoadSinglePlayerGameGivenFilename(tutCurrLessonName);
            return 1;
        }
    }

    return 0;
}

void tutDrawBackButtonFunction(regionhandle reg)
{
    if (tutPrevVisible)
    {
        //put in pointer to prevlesson texture here
        if(mouseInRect(&reg->rect))
        {
            if(mouseLeftButton())
                trRGBTextureMakeCurrent(tutTexture[TUT_PREV_ON]);
            else
                trRGBTextureMakeCurrent(tutTexture[TUT_PREV_MOUSE]);
        }
        else
            trRGBTextureMakeCurrent(tutTexture[TUT_PREV_OFF]);
    }
    else
    {
        //the back button is displayed as a "Restart Lesson"
        if(mouseInRect(&reg->rect))
        {
            if(mouseLeftButton())
                trRGBTextureMakeCurrent(tutTexture[TUT_REST_ON]);
            else
                trRGBTextureMakeCurrent(tutTexture[TUT_REST_MOUSE]);
        }
        else
            trRGBTextureMakeCurrent(tutTexture[TUT_REST_OFF]);
    }

    rndPerspectiveCorrection(FALSE);
//  glEnable(GL_BLEND);
    primRectSolidTextured2(&reg->rect);
//  glDisable(GL_BLEND);
}


// Returns the next "StartFrom" value, and copies the token into pDest
long GetNextCommaDelimitedToken(char *pString, char *pDest, long StartFrom)
{
static char *pSrc = NULL;
long Index = 0, Len;

    if(pString)
    {
        pSrc = pString;
        Index = 0;
    }

    if(pSrc)
    {
        Index = StartFrom;
        Len = 0;

        while(pSrc[Index] && pSrc[Index] != ',' && pSrc[Index] != 0x0a && pSrc[Index] != 0x0d)
        {
            pDest[Len] = pSrc[Index];
            Len++;
            Index++;
        }
        pDest[Len] = 0;

        if(pSrc[Index] == ',')
            Index++;
        else
            pSrc = NULL;
    }
    else
        return 0;

    return Index;
}

long tutFindTokenIndex(char *pTokenList[], char *pToken)
{
long    i;

    i = 0;
    while(pTokenList[i][0])
    {
        if(strcasecmp(pTokenList[i], pToken) == 0)
            return i;
        i++;
    }

    return -1;
}

long tutParseImagesIntoIndices(char *szImages)
{
char    szToken[256];
long    StrIndex, Count, TokenIndex;

    Count = 0;
    StrIndex = GetNextCommaDelimitedToken(szImages, szToken, 0);
    while(StrIndex)
    {
        TokenIndex = tutFindTokenIndex(tutImageList, szToken);

        if(TokenIndex != -1)
            szImageIndexList[Count] = TokenIndex;
        else
            szImageIndexList[Count] = 0;

        Count++;
        StrIndex = GetNextCommaDelimitedToken(NULL, szToken, StrIndex);
    }

    szImageIndexList[Count] = 0;
    return Count;
}


/*
This function will accept a comma delimited list of images to be displayed, in
order of appearance in this list, from left to right on the screen.  The image
list is included in the header.
*/
void tutShowImages(char *szImages)
{
long    ImageCount;

    if(tutImageVisible)
        tutHideImages();

    tutAllocateRootRegion();

    ImageCount = tutParseImagesIntoIndices(szImages);

    tutImageAtom.x = MAIN_WindowWidth - ImageCount * 64;
    tutImageAtom.y = MAIN_WindowHeight - 64;
    tutImageAtom.width = ImageCount * 64;
    tutImageAtom.height = 64;

    tutImageRegion = regChildAlloc(tutRootRegion, (sdword)&tutImageAtom,
        tutImageAtom.x, tutImageAtom.y, tutImageAtom.width, tutImageAtom.height, 0, 0);

    tutImageAtom.region = (void*)tutImageRegion;
    tutImageRegion->atom = &tutImageAtom;
    tutImageAtom.pData = szImages;

    tutImageVisible = TRUE;

    regDrawFunctionSet(tutImageRegion, tutDrawImageFunction);
}

void tutDrawImageFunction(regionhandle reg)
{
rectangle   rect;
long    i, Index;
long    x, y;

    x = reg->rect.x0;
    y = reg->rect.y0;

//  glEnable(GL_BLEND);
    i = 0;

    while(szImageIndexList[i])
    {
        Index = szImageIndexList[i];

        trRGBTextureMakeCurrent(tutTexture[Index]);
        rndPerspectiveCorrection(FALSE);
        rect.x0 = x + (64 * i);
        rect.y0 = y;
        rect.x1 = x + (64 * i) + 64;
        rect.y1 = y + 64;
        primRectSolidTextured2(&rect);

        i++;
    }
//  glDisable(GL_BLEND);
}


void tutHideImages(void)
{
    if(tutImageRegion)
    {
        regRegionDelete(tutImageRegion);
        tutDeallocateRootRegion();
    }

    tutImageRegion = NULL;
    tutImageVisible = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : tutStartup
    Description : tutorial init function called at game startup
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tutStartup(void)
{
    sdword index;

    for (index = 0; tutImageList[index][0]; index++)
    {
        tutTexture[index] = TR_InvalidInternalHandle;
        tutImage[index] = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : tutShutdown
    Description : Shutdown function called at game shutdown time
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tutShutdown(void)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : tutInitialize
    Description : Tutorial init function called once for every tutorial load
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tutInitialize(void)
{
    long    i;
    char    Filename[256];

    i = 0;

    while( tutImageList[i][0] )
    {
        if (tutTexture[i] == TR_InvalidInternalHandle)
        {
            dbgAssert(tutImage[i] == NULL);
#ifdef _WIN32
            strcpy(Filename, "feman\\texdecorative\\");
#else
            strcpy(Filename, "feman/texdecorative/");
#endif

            //load the correct texture depending on language
            if (strCurLanguage == languageEnglish)
            {
                strcat(Filename, tutImageList[i]);
            }
            else if (strCurLanguage == languageFrench)
            {
                strcat(Filename, tutFrenchImageList[i]);
            }
            else if (strCurLanguage == languageGerman)
            {
                strcat(Filename, tutGermanImageList[i]);
            }
            else if (strCurLanguage == languageSpanish)
            {
                strcat(Filename, tutSpanishImageList[i]);
            }
            else if (strCurLanguage == languageItalian)
            {
                strcat(Filename, tutItalianImageList[i]);
            }
            else
            {
                //defaults to english
                strcat(Filename, tutImageList[i]);
            }

            strcat(Filename, ".LiF");
            tutImage[i] = trLIFFileLoad(Filename, NonVolatile);

            dbgAssert(tutImage[i] != NULL);
            tutTexture[i] = trRGBTextureCreate((color *)tutImage[i]->data, tutImage[i]->width, tutImage[i]->height, TRUE);
            i++;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : tutUnInitialize
    Description : Tutorial shutdown function called at the end of a tutorial game
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tutUnInitialize(void)
{
long    i;

    tutHideText();
    tutHideImages();
    tutHideNextButton();
    tutHideBackButton();

    i = 0;
    while( tutImageList[i][0] )
    {
        if(tutImage[i] != NULL)
        {
            memFree(tutImage[i]);
            tutImage[i] = NULL;
        }

        if (tutTexture[i] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(tutTexture[i]);
            tutTexture[i] = TR_InvalidInternalHandle;
        }
        i++;
    }
    tutEnableEverything();
}

void tutSetFlagIndex(long Index, long Val)
{
long *pFlagMem;
long FlagBit;

    //hack code to get speech to work when game paused
    if (Index == 1)
    {
        FalkosFuckedUpTutorialFlag = !Val;
    }

    pFlagMem = (long *)&tutEnable;
    pFlagMem += Index/32;

#ifdef _MACOSX_FIX_ME // ENDIAN_BIG?
	FlagBit = 1 << (31 - Index & 31);
#else
    FlagBit = 1 << (Index & 31);
#endif

    if(Val)
        *pFlagMem |= FlagBit;
    else
        *pFlagMem &= ~FlagBit;
}

void tutEnableEverything(void)
{
long    i;

    for(i=0; i<sizeof(tutGameEnableFlags)*8; i++)
        tutSetFlagIndex(i, 1);
}

void tutDisableEverything(void)
{
long    i;

    for(i=0; i<sizeof(tutGameEnableFlags)*8; i++)
        tutSetFlagIndex(i, 0);
}

void tutSetEnableFlags(char *pFlagString, long Val)
{
char    szToken[256];
long    StrIndex, TokenIndex;

    StrIndex = GetNextCommaDelimitedToken(pFlagString, szToken, 0);
    while(StrIndex)
    {
        TokenIndex = tutFindTokenIndex(tutGameEnableString, szToken);
        tutSetFlagIndex(TokenIndex, Val);
        StrIndex = GetNextCommaDelimitedToken(NULL, szToken, StrIndex);
    }
}

void tutBuilderSetRestrictions(char *pShipTypes, bool bRestricted)
{
ShipType    st;
char    szToken[256];
long    StrIndex;

    StrIndex = GetNextCommaDelimitedToken(pShipTypes, szToken, 0);
    while(StrIndex)
    {
        st = StrToShipType(szToken);
        tutBuildRestrict[st] = bRestricted;

        StrIndex = GetNextCommaDelimitedToken(NULL, szToken, StrIndex);
    }

    // usually BuildArrows are on when the player can build things
    // but the arrows can be disabled seperately if needed
    tutSetEnableFlags("BuildArrows", !bRestricted);
}

void tutBuilderRestrictAll(void)
{
int i;

    for(i=0; i<TOTAL_STD_SHIPS; i++)
        tutBuildRestrict[i] = 1;

    tutSetEnableFlags("BuildArrows", 0);
}

void tutBuilderRestrictNone(void)
{
int i;

    for(i=0; i<TOTAL_STD_SHIPS; i++)
        tutBuildRestrict[i] = 0;

    tutSetEnableFlags("BuildArrows", 1);
}

sdword tutIsBuildShipRestricted(sdword shipType)
{
    return tutBuildRestrict[shipType];
}

sdword tutSelectedContainsShipType(ShipType st)
{
long i, j;

    for(i=0, j=0; i<selSelected.numShips; i++)
    {
        if(selSelected.ShipPtr[i]->shiptype == st)
            j++;
    }
    return j;
}

sdword tutSelectedContainsShipTypes(char *pShipTypes)
{
ShipType    st;
char    szToken[256];
long    StrIndex, Count;

    Count = 0;
    StrIndex = GetNextCommaDelimitedToken(pShipTypes, szToken, 0);
    while(StrIndex)
    {
        st = StrToShipType(szToken);

        Count += tutSelectedContainsShipType(st);

        StrIndex = GetNextCommaDelimitedToken(NULL, szToken, StrIndex);
    }

    return Count;
}

void tutGameMessage(char *commandName)
{
    if(!tutorial) return;

    dbgMessagef("\ntutGameMessage: '%s'", commandName);

    if( tutGameMessageIndex < 16 )
    {
        strcpy(tutGameMessageList[ tutGameMessageIndex ], commandName);
        tutGameMessageIndex++;
    }
}

sdword tutGameSentMessage(char *commandNames)
{
sdword  RetVal = 0, i;
char    szToken[256];
long    StrIndex, Count;

    Count = 0;
    StrIndex = GetNextCommaDelimitedToken(commandNames, szToken, 0);
    while(StrIndex)
    {
        for(i=0; i<tutGameMessageIndex; i++)
        {
            if(strcasecmp(szToken, tutGameMessageList[i]) == 0)
                RetVal = 1;
        }
        StrIndex = GetNextCommaDelimitedToken(NULL, szToken, StrIndex);
    }

    tutGameMessageIndex = 0;
    return RetVal;;
}

void tutResetGameMessageQueue(void)
{
    tutGameMessageIndex = 0;
}

sdword tutContextMenuDisplayedForShipType(char *pShipType)
{
ShipType    st;

    st = StrToShipType(pShipType);
    return (tutFEContextMenuShipType == st);
}

void tutResetContextMenuShipTypeTest(void)
{
    tutFEContextMenuShipType = (ShipType)-1;
}


sdword tutBuildManagerShipTypeInBatchQueue(char *pShipType)
{
long        i;
ShipType    st;

    st = StrToShipType(pShipType);
    for (i=0; cmShipsAvailable[i].nJobs != -1; i++)
    {
        if(cmShipsAvailable[i].type == st)
            return cmShipsAvailable[i].nJobs;
    }

    return 0;
}

sdword tutBuildManagerShipTypeInBuildQueue(char *pShipType)
{
long        index;
ShipType    st;
Node        *node;
Ship        *factoryship;
shipsinprogress *sinprogress;
shipinprogress  *progress;

    st = StrToShipType(pShipType);

    node = listofShipsInProgress.head;
    while (node != NULL)
    {
        sinprogress = (shipsinprogress *)listGetStructOfNode(node);
        factoryship = sinprogress->ship;

        if (factoryship->playerowner == universe.curPlayerPtr)
        {
            for (index = 0, progress = &sinprogress->progress[0]; index < TOTAL_STD_SHIPS; index++, progress++)
            {
                if(progress->info && progress->info->shiptype == st)
                    return progress->nJobs;
            }
        }
        node = node->next;
    }

    return 0;
}

//external function definition
shipinprogress *cmSIP(udword index);

/*-----------------------------------------------------------------------------
    Name        : tutBuildManagerShipTypeSelected
    Description : Returns TRUE if the shiptype is selected in the build manager
    Inputs      : pShipType - string of shiptypes
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/

sdword tutBuildManagerShipTypeSelected(char *pShipType)
{
    ShipType shiptype;
    shipinprogress *shipprog;

    shiptype = StrToShipType(pShipType);
    shipprog = cmSIP(shiptype);

    if (shipprog->selected)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



long tutCameraFocusShipCount(ShipType st)
{
long Count = 0;
CameraStackEntry *entry;
long    i;

    entry = currentCameraStackEntry(&universe.mainCameraCommand);

    for(i=0; i<entry->focus.numShips; i++)
    {
        if(entry->focus.ShipPtr[i]->shiptype == st)
            Count++;
    }
    return Count;
}


sdword tutCameraFocusedOnShipType(char *pShipTypes)
{
char    szToken[256];
long    StrIndex, Count;
ShipType st;

    if (smSensorsActive)
    {
        //camera can't be focused on a ship in the Sensors Manager
        return 0;
    }
    Count = 0;
    StrIndex = GetNextCommaDelimitedToken(pShipTypes, szToken, 0);
    while(StrIndex)
    {
        st = StrToShipType(szToken);
        Count += tutCameraFocusShipCount(st);
        StrIndex = GetNextCommaDelimitedToken(NULL, szToken, StrIndex);
    }
    return Count;
}
