/*=============================================================================
    DEBUGWND.H: Definitions for the Windows part of the debug window code.

        Created by Luke Moloney June 1997
=============================================================================*/

#ifndef ___DEBUGWND_H
#define ___DEBUGWND_H

/*=============================================================================
    Switches:
=============================================================================*/
#define DBW_TO_FILE             1
#define DBW_FILE_NAME           "debugMessages.txt"

#ifndef HW_Release

#define DBW_ERROR_CHECKING      1

#else //HW_Debug

#define DBW_ERROR_CHECKING      0

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//default size and location of window
#define DBW_WindowX                 (MAIN_WindowWidth + GetSystemMetrics(SM_CXSIZEFRAME) * 2)
#define DBW_WindowY                 0
#define DBW_WindowWidth             60
#define DBW_WindowHeight            30
#define DBW_BufferHeight            100

#define DBW_NumberPanes             4

//pane-specific flags
#define DPF_Enabled                 1

//name of window/class
#define DBW_ClassName               "HW_DebugWindow"
#define DBW_WindowTitle             "Homeworld Debug Window"

//font parameters
#define DBW_FontPointsMin           4
#define DBW_FontPointsMax           20

//strings for use with the ini file
#define DIS_SectionName             "DebugWindow"
#define DIS_Location                "Location"
#define DIS_Size                    "Size"
#define DIS_Font                    "Font"
#define DIS_PaneBase                "Pane"
#define DIS_FileName                "Homeworld.cfg"
#define DIS_StringLength            64

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure for data of a single debug pane
typedef struct
{
    udword flags;                   //pane-specific flags
    sdword x, y;                    //top left of pane (chars)
    sdword width, height;           //size of pane (chars)
    sdword cursorX, cursorY;        //location of 'cursor'
    sdword viewTop;                 //y location of buffer at top of on-screen pane
    sdword bufferHeight;            //height of buffer for this pane
    char *buffer;                   //pointer to buffer for this pane
    FILE *logFile;                  //log file handle
}
pane;

/*=============================================================================
    Macros
=============================================================================*/
//debug-enabled macro for pane index range checking
#if DBW_ERROR_CHECKING
#define dbwPaneOutRange(pane)   ((pane) < 0 || (pane) >= DBW_NumberPanes)
#else
#define dbwPaneOutRange(pane)   (0)
#endif //DBW_ERROR_CHECKING

//check to see if pane is enabled
#define dbwPaneEnabled(pane)    (bitTest(dbwPane[pane].flags, DPF_Enabled))

/*=============================================================================
    Functions:
=============================================================================*/
#ifdef _WIN32   /* Don't use debug window outside Windows. */
//start/close down the debug window.
sdword dbwStart(udword hInstance, udword hWndParent);
void   dbwClose(void);
void   dbwWriteOptions(void);

//setting attributes of window
sdword dbwLocationSet(sdword xPixels, sdword yPixels);
sdword dbwSizeSet(sdword xChars, sdword yChars);
sdword dbwScrollBufferSizeSet(sdword yChars);

//pane attribute functions
sdword dbwPaneEnable(sdword pane, sdword enable);

//logging
sdword dbwLogStart(sdword pane, char *fileName);
sdword dbwLogFlush(sdword pane);
sdword dbwLogEnd(sdword pane);
sdword dbwLogEndAll(void);

//printing strings to the window
sdword dbwPrint(sdword pane, char *string);
sdword dbwLineFeed(sdword pane);
void   dbwPaneClear(sdword pane);
sdword dbwCursorSet(sdword pane, sdword x, sdword y);
#endif  /* _WIN32 */

#endif //___DEBUGWND_H

