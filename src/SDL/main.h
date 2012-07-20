/*=============================================================================
    MAIN.H: Definitions for main Windows interface file for Homeworld.

    Created June 1997 By Luke Moloney
=============================================================================*/

#ifndef ___MAIN_H
#define ___MAIN_H

#include "Types.h"
#include "mainswitches.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define MAIN_Password               0           //enable password-enabling/disabling

#ifndef HW_Release

#define MAIN_MOUSE_FREE             1           //allow the mouse to be freed from the window
#define MAIN_SENSOR_LEVEL           1           //enable command-line adjustment of sensors level
#define MAIN_PRINT_MESSAGES         0           //print the window messages as they come pouring in

#else //HW_Debug

#define MAIN_MOUSE_FREE             1           //allow the mouse to be freed from the window
#define MAIN_SENSOR_LEVEL           0           //enable command-line adjustment of sensors level
#define MAIN_PRINT_MESSAGES         0           //print the window messages as they come pouring in

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define ERR_ErrorStart              0xfed5
#define MAIN_CDCheck                0
#define MAIN_ExpiryTime             30          //expires after 30 days

//command-line parsing definitions
#define TS_Delimiters               " \t,="
#define MCL_IndentSpace             4           //number of indent characters

//command-line option flags
#define COF_Visible                 1           //user can see this option with /?
#define COF_NextToken               2           //pass next token as parameter to function

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure for command-line parsing
typedef struct
{
    //bool visible;                               //is this option visible by using '/?'?
    udword flags;                               //flags for this entry.  See above for possible values.
    char *parameter;                            //parameter string (begins with a slash)
    bool (*function)(char *string);             //function to call, NULL if none.  Called after variable is set.
    void *variableToModify;                     //variable to modify, NULL if none
    udword valueToSet;                          //value to set variable to
    char *helpString;                           //string printed in help screen
}
commandoption;

/*=============================================================================
    Data:
=============================================================================*/
extern void *ghMainWindow;
extern void *ghInstance;
#if MAIN_SENSOR_LEVEL
udword initialSensorLevel;
#endif

extern sdword enableTrails;
extern sdword showBackgrounds;
extern bool   POCTrails;

extern bool mainRasterSkip;

extern bool mainSafeGL;

extern bool mainForceKatmai;
extern bool mainAllowKatmai;
extern bool mainAllowPacking;
extern bool mainOnlyPacking;

extern bool fullScreen;

extern bool enableAVI;

extern sdword mainReinitRenderer;

//size of main window
extern sdword MAIN_WindowWidth;
extern sdword MAIN_WindowHeight;
extern sdword MAIN_WindowDepth;

extern sdword mainWindowWidth;
extern sdword mainWindowHeight;
extern sdword mainWindowDepth;

extern char mainDeviceToSelect[];
extern char mainGLToSelect[];
extern char mainD3DToSelect[];

extern bool mainNoPerspective;
extern bool systemActive;

/*=============================================================================
    Functions:
=============================================================================*/
void WindowsCleanup(void);

void ActivateMe(void);
void DeactivateMe(void);

// Event handler.
sdword HandleEvent (const SDL_Event* pEvent);

//load/save options from disk
void utyOptionsFileRead(void);
void utyOptionsFileWrite(void);

//renderer swapping functions
bool mainLoadGL(char* data);
bool mainLoadRGL(void);
bool mainLoadParticularRGL(char* device, char* data);
sdword mainActiveRenderer(void);
bool mainReinitRGL(void);
bool mainShutdownRenderer(void);

void mainSaveRender(void);
void mainRestoreRender(void);
void mainRestoreSoftware(void);

//video playback module convenience functions
void mainCleanupAfterVideo(void);

#endif //___MAIN_H

