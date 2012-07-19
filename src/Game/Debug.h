/*=============================================================================
    DEBUG.H: Definitions for general debugging.

    Created June 1997 by Luke Moloney
=============================================================================*/

#ifndef ___DEBUG_H
#define ___DEBUG_H

//make sure the HW_Level env. variable is set correctly
#ifndef HW_Debug
#ifndef HW_Interim
#ifndef HW_Release
#error HW_Level must be one of HW_Debug, HW_Interim or HW_Release
#endif
#endif
#endif

#include "types.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define DBG_ASSERT              1           //assertion checking
#define DBG_FILE_LINE           1           //print file and line
#define DBG_STACK_CONTEXT       1           //dump stack context at fatal error time

#else //HW_Debug

#define DBG_ASSERT              0
#define DBG_FILE_LINE           0           //don't print file and line
#define DBG_STACK_CONTEXT       0           //dump stack context at fatal error time

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/

//maximum length of a single string for format printing
#define DBG_BufferLength        2048
#define DBG_BufferMax           (DBG_BufferLength - 1)

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
#if DBG_STACK_CONTEXT
extern udword dbgStackBase;
#endif

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

//non-fatal errors
sdword dbgNonFatal(char *file, sdword line, char *error);
sdword dbgNonFatalf(char *file, sdword line, char *format, ...);

//dump functions
#if DBG_STACK_CONTEXT
char *dbgStackDump(void);
#endif

//special debug mode functions not implemented yet

#endif //___DEBUG_H

