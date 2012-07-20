/*=============================================================================
    Name    : ScenPick.c
    Purpose : Code for choosing a Scenario

    Created 10/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SCENPICK_H
#define ___SCENPICK_H           3.4444445f

#include "Types.h"
#include "FEFlow.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define SCP_ERROR_CHECKING      1               //basic error checking
#define SCP_VERBOSE_LEVEL       1               //control verbose printing

#else //HW_Debug

#define SCP_ERROR_CHECKING      0               //basic error checking
#define SCP_VERBOSE_LEVEL       0               //control verbose printing

#endif //HW_Debug


/*=============================================================================
    Definitions:
=============================================================================*/
#define SCP_ScreenName           "Load_a_scenario"
#define SCP_ListFont             "Arial_12.hff"
#define SCP_NameFont             "Arial_12.hff"
#define SCP_NonSelectedColor     colRGB(0, 120, 200)
#define SCP_SelectedTabbedColor  colRGB(0, 255, 0)
#define SCP_SelectedColor        SCP_SelectedTabbedColor
#define SCP_ListMarginInterLine  1
#define SCP_ListMarginX          2
#define SCP_ListMarginY          2
#define SCP_ScenarioListLength   25
#define SCP_DisplayListLength    11      //number of scenarios displayed on the screen at one time
#define SCP_VertSpacing          (fontHeight(" ") >> 1)

#define SCP_PreviewWidthMin      32
#define SCP_PreviewWidthMax      640
#define SCP_PreviewHeightMin     32
#define SCP_PreviewHeightMax     480

#define SCP_PreviewWidth         256
#define SCP_PreviewHeight        128

#define SP_ScenarioListGrowth    10

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
    char    *title;
    char    *fileSpec;
    char    *bitmapfileSpec;
    udword   maxplayers;
    udword   minplayers;
}
spscenario;

/*=============================================================================
    Data:
=============================================================================*/
extern spscenario *spScenarios;                 //list of available scenarios
extern sdword spCurrentSelected;                //current scenario index, if OK is pressed
extern sdword spScenarioListLength;

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown
void spStartup(void);
void spShutdown(void);

//run the scenario picker
void spScenarioPick(char *dest);

//close the picker, either with 'back' or with 'OK'
void spDonePicking(char *name, featom *atom);
void spBackPicking(char *name, featom *atom);

void spFindMap(char *MapName);

sdword spScenarioListProcess(regionhandle region, sdword ID, udword event, udword data);
sdword spScenarioFind(char *scenarioName);

#endif //___SCENPICK_H
