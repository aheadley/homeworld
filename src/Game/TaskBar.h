/*=============================================================================
    Name    : TaskBar.h
    Purpose : Definitions for the task bar, which keeps track of missions spheres.

    Created 10/7/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___TASKBAR_H
#define ___TASKBAR_H

#include "Types.h"
#include "Region.h"
#include "FEFlow.h"
#include "FEColour.h"
#include "UIControls.h"

/*=============================================================================
    Switches:
=============================================================================*/

#define TB_TEST                     0           //test this module
#ifndef HW_Release

#define TB_ERROR_CHECKING           1           //general error checking
#define TB_VERBOSE_LEVEL            1           //control specific output code
#define TB_SECRET_BUTTON            1           //secret button on lower right extreme of task bar exits game immediately

#else //HW_Debug

#define TB_ERROR_CHECKING           0           //general error checking
#define TB_VERBOSE_LEVEL            0           //control specific output code
#define TB_SECRET_BUTTON            0           //secret button on lower right extreme of task bar exits game immediately

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//general definitions
#ifdef _WIN32
#define TB_FileName             "FEMan\\TaskBar.fib"//filename of taskbar feman file
#else
#define TB_FileName             "FEMan/TaskBar.fib" /*filename of taskbar feman file*/
#endif
#define TB_ScreenName           "Task_Bar"      //name of the taskbar FEMan screen
#define TB_FleetManager         "TB_Fleet"      //entry points/function callbacks
#define TB_SensorsManager       "TB_Sensors"
#define TB_InfoOverlay          "TB_InfoOverlay"
//#define TB_MissionButtons       "TB_TaskButtons"//user region to fit the mission sphere buttons into
#define TB_ObjectivesListWindow "TB_ObjectivesWindowInit"
#define TB_FontFile             "Arial_b12.hff"
#define TB_ObjectiveFontFile    "HW_Eurosecond_11.hff"

//parameters for task buttons
#define TB_MaxButtonWidth       96
#define TB_ButtonHeight         24
#define TB_ButtonMarginLeft     2
#define TB_ButtonMarginRight    2
#define TB_ButtonMarginTween    1
#define TB_MaxButtons           16
#define TB_MaxString            64
#define TB_ButtonTextMargin     2
#define TB_ButtonTextMarginTop  2

//info on bumping the bar up/down
#define TB_BumperHeight         2               //height of 'bumper' region which is actually invisible
#define TB_BumpUpSpeed          8               //speed to move up/down
#define TB_BumpDownSpeed        8

//task button control flags
#define TBF_Alert               0x00000001      //an 'alert' button, needing immediate pressing
#define TBF_Disabled            0x00000004      //this button in enabled
//all flags after here for internal use only
#define TBF_InUse               0x20000000
#define TBF_HeldDown            0x40000000      //button held down but not actually pressed
#define TBF_Pressed             0x80000000      //this is the button which is pressed

/*=============================================================================
    Type definitions:
=============================================================================*/
struct taskbutton;

//taskbar button callback function
typedef void (*tbfunction)(struct taskbutton *button, ubyte *userData);

//structure for a button on the task bar
typedef struct taskbutton
{
    regionhandle reg;                           //handle of region to draw
    udword flags;                               //flags for this button
    char *caption;                              //text caption for task button
    ubyte *userData;                            //user data for button, typically a mission sphere pointer
    tbfunction function;                        //function called when button pressed
}
taskbutton;

#define TBL_MaxCharsPerLine         100

#define TBL_PrimaryObj1st           0
#define TBL_PrimaryObj2nd           1
#define TBL_HeaderSecondary         2
#define TBL_SecondaryObj1st         3
#define TBL_SecondaryObj2nd         4

typedef struct tasklistitem
{
    struct Objective *objective;
    udword            type;
    char              descFrag[TBL_MaxCharsPerLine];
}
tasklistitem;

#define TBDISABLE_NORMAL            1
#define TBDISABLE_SENSORS_USE       2
#define TBDISABLE_TRADEMGR_USE      4
//exported variable
extern bool tbDisable;

/*=============================================================================
    Functions:
=============================================================================*/
//startup/shutdown
void tbStartup(void);
void tbShutdown(void);
void PossiblyResetTaskbar(void);

//create/remove task bar buttons
taskbutton *tbButtonCreate(char *caption, tbfunction function, ubyte *userData, udword flags);
void tbButtonDelete(taskbutton *button);
void tbButtonDeleteByData(ubyte *userData);
void tbButtonListRefresh(void);

//select a button
void tbButtonSelect(taskbutton *button);
void tbButtonSelectByData(ubyte *userData);

// Single player objectives list window on taskbar
void tbObjectivesListAddItem(ubyte *data);
void tbObjectivesHyperspace(ubyte *data);
void tbObjectivesListRemoveItem(ubyte *data);
void tbObjectivesListCleanUp(void);

//bump down the taskbar
void tbSensorsHook(void);

//bump up the taskbar
void tbForceTaskbar(bool On);

#endif //___TASKBAR_H

