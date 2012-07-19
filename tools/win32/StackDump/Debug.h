/*=============================================================================
    DEBUG.H: Definitions for general debugging.

    Created June 1997 by Luke Moloney
=============================================================================*/

#ifndef ___DEBUG_H
#define ___DEBUG_H

#include "types.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifdef HW_Debug

#define DBG_ASSERT              1           //assertion checking
#define DBG_FILE_LINE           1           //print file and line

#else //HW_Debug

#define DBG_ASSERT              1
#define DBG_FILE_LINE           1           //don't print file and line

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/

//maximum length of a single string for format printing
#define DBG_BufferLength        512

//message box style
#define DBG_MBCaption           "Fatal Error"
#define DBG_MBFlags             MB_ICONSTOP | MB_OK

//default exit code
#define DBG_ExitCode            0xfed5

/*=============================================================================
    Data:
=============================================================================*/
extern char dbgFatalErrorString[DBG_BufferLength];
extern sdword dbgInt3Enabled;

/*=============================================================================
    Macros:
=============================================================================*/

#if DBG_ASSERT
#define dbgAssert(expr) if (!(expr)) dbgFatalf(DBG_Loc, "Assertion of (%s) failed.", #expr)
#else
#define dbgAssert(expr) ((void)0)
#endif

#if DBG_FILE_LINE
#define DBG_Loc         __FILE__, __LINE__
#else
#define DBG_Loc         NULL, 0
#endif

/*=============================================================================
    Functions:
=============================================================================*/
//general message handling
sdword dbgMessage(char *string);
sdword dbgMessagef(char *format, ...);

//warnings
sdword dbgWarning(char *file, sdword line, char *format);
sdword dbgWarningf(char *file, sdword line, char *format, ...);

//fatal errors
sdword dbgFatal(char *file, sdword line, char *format);
sdword dbgFatalf(char *file, sdword line, char *format, ...);

//special debug mode functions not implemented yet

#endif //___DEBUG_H

