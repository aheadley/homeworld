/*=============================================================================
    DEBUG.C: Functions for debugging.

    Created June 1997 by Luke Moloney
=============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "resource.h"
#include "types.h"
#include "debug.h"

/*=============================================================================
    Data:
=============================================================================*/
char dbgFatalErrorString[DBG_BufferLength];
sdword dbgInt3Enabled = FALSE;

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : dbgMessage
    Description : Post a generic message
    Inputs      : string - message to print
    Outputs     : ..
    Return      : ..
----------------------------------------------------------------------------*/
sdword dbgMessage(char *string)
{
//    return(printf(string));
	return(0);
}

/*-----------------------------------------------------------------------------
    Name        : dbgMessagef
    Description : Post a generic formatted message
    Inputs      :
        format - format string
        ...    - variable number of parameters
    Outputs     : ..
    Return      : Number of characters printed
----------------------------------------------------------------------------*/
sdword dbgMessagef(char *format, ...)
{
    char buffer[DBG_BufferLength];
    va_list argList;
    sdword nParams;

    va_start(argList, format);                              //get first arg
    vsprintf(buffer, format, argList);                      //prepare output string
    va_end(argList);

    nParams = dbgMessage(buffer);
    return(nParams);
}

/*-----------------------------------------------------------------------------
    Name        : dbgWarning
    Description : Post a warning message
    Inputs      :
        file - points to file name string
        line - line number called from
        string - output string
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword dbgWarning(char *file, sdword line, char *string)
{
    char buffer[DBG_BufferLength];

    sprintf(buffer, "\n%s (%d): Warning- %s", file, line, string);

    return(printf(buffer));                             //print the message
}

/*-----------------------------------------------------------------------------
    Name        : dbgWarning
    Description : Post a formatted warning message
    Inputs      :
        file - points to file name string
        line - line number called from
        format - format string
        ...    - variable number of parameters
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword dbgWarningf(char *file, sdword line, char *format, ...)
{
    char buffer[DBG_BufferLength];
    char newFormat[DBG_BufferLength];
    va_list argList;
    sdword returnValue;

    sprintf(newFormat, "\n%s (%d): Warning- %s", file, line, format);
    va_start(argList, format);                              //get first arg
    vsprintf(buffer, newFormat, argList);                   //prepare output string
    va_end(argList);

//    returnValue = dbgMessage(buffer);
    returnValue = printf(buffer);
    return(returnValue);                                    //print the message
}

/*-----------------------------------------------------------------------------
    Name        : dbgFatal
    Description : Print a fatal error message and exit program.
    Inputs      :
        file - points to file name string
        line - line number called from
        string - output string
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword dbgFatal(char *file, sdword line, char *string)
{
    sprintf(dbgFatalErrorString, "\n%s (%d): Fatal error - %s", file, line, string);

//    dbgMessage(dbgFatalErrorString);                        //print the message
    fprintf(stderr, dbgFatalErrorString);
    if (dbgInt3Enabled)
    {
        _asm int 3
    }
    exit(0xfed5);
    return(ERROR);
}

/*-----------------------------------------------------------------------------
    Name        : dbgFatalf
    Description : Print a formatted fatal error message and exit program.
    Inputs      :
        file - points to file name string
        line - line number called from
        format - format string
        ...    - variable number of parameters
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword dbgFatalf(char *file, sdword line, char *format, ...)
{
    char newFormat[DBG_BufferLength];
    va_list argList;

    sprintf(newFormat, "\n%s (%d): Fatal Error - %s", file, line, format);
    va_start(argList, format);                              //get first arg
    vsprintf(dbgFatalErrorString, newFormat, argList);      //prepare output string
    va_end(argList);

    fprintf(stderr, dbgFatalErrorString);
//    dbgMessage(dbgFatalErrorString);                        //print the message
    if (dbgInt3Enabled)
    {
        _asm int 3
    }
    exit(0xfed5);
    return(ERROR);
}

