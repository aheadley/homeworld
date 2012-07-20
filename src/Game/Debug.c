/*=============================================================================
    DEBUG.C: Functions for debugging.

    Created June 1997 by Luke Moloney
=============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "resource.h"
#include "Types.h"
#include "utility.h"
#include "debugwnd.h"
#include "Task.h"
#include "Debug.h"
#include "File.h"

/*=============================================================================
    Data:
=============================================================================*/
char dbgFatalErrorString[DBG_BufferLength];
sdword dbgInt3Enabled = TRUE;

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
    /* Debug window disabled (using stdout instead, at least for now).
       dbw*() functions in other parts of code (main.c, utility.c, and
       Options.c) will need to be uncommented if the debug window is
       reenabled. */
    /*return(dbwPrint(0, string));*/
    if (string[0] == '\n')
        string++;
    printf("%s\n", string);
    return OKAY;
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
    vsnprintf(buffer, DBG_BufferMax, format, argList);                      //prepare output string
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

    snprintf(buffer, DBG_BufferMax, "\n%s (%d): Warning- %s", file, line, string);

    return(dbgMessage(buffer));                             //print the message
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

    snprintf(newFormat, DBG_BufferMax, "\n%s (%d): Warning- %s", file, line, format);
    va_start(argList, format);                              //get first arg
    vsnprintf(buffer, DBG_BufferMax, newFormat, argList);                   //prepare output string
    va_end(argList);

    returnValue = dbgMessage(buffer);
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
    snprintf(dbgFatalErrorString, DBG_BufferMax, "\n%s (%d): Fatal error - %s", file, line, string);
#if DBG_STACK_CONTEXT
    {
        char *fileName = dbgStackDump();
        if (fileName)
        {
        sprintf(dbgFatalErrorString + strlen(dbgFatalErrorString), "\nDumped to '%s'.", fileName);
        }
    }
#endif

    dbgMessage(dbgFatalErrorString);                        //print the message
    if (dbgInt3Enabled)
    {
#if defined (_MSC_VER)
        _asm int 3
#elif defined (__GNUC__) && defined (__i386__)
        __asm__ ( "int $3\n\t" );
#endif
    }
    utyFatalErrorWaitLoop(DBG_ExitCode);                    //exit with a MessageBox
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

    snprintf(newFormat, DBG_BufferMax, "\n%s (%d): Fatal Error - %s", file, line, format);
    va_start(argList, format);                              //get first arg
    vsnprintf(dbgFatalErrorString, DBG_BufferMax, newFormat, argList);      //prepare output string
    va_end(argList);
#if DBG_STACK_CONTEXT
    {
        char *fileName = dbgStackDump();
        if (fileName)
        {
        sprintf(dbgFatalErrorString + strlen(dbgFatalErrorString), "\nDumped to '%s'.", fileName);
        }
    }
#endif

    dbgMessage(dbgFatalErrorString);                        //print the message
    if (dbgInt3Enabled)
    {
#if defined (_MSC_VER)
        _asm int 3
#elif defined (__GNUC__) && defined (__i386__)
        __asm__ ( "int $3\n\t" );
#endif
    }
    utyFatalErrorWaitLoop(DBG_ExitCode);                    //exit with a MessageBox
    return(ERROR);
}

/*-----------------------------------------------------------------------------
    Name        : dbgNonFatal
    Description : Non-fatal error handling: pops up a dialog but does not
                    exit the game.
    Inputs      : same as dbgFatal
    Outputs     :
    Return      : 0
----------------------------------------------------------------------------*/
sdword dbgNonFatal(char *file, sdword line, char *error)
{
    sprintf(dbgFatalErrorString, "\n%s (%d): Non-fatal error - %s", file, line, error);
    if (utyNonFatalErrorWaitLoop() && dbgInt3Enabled)
    {
#if defined (_MSC_VER)
        _asm int 3
#elif defined (__GNUC__) && defined (__i386__)
        __asm__ ( "int $3\n\t" );
#endif
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : dbgNonFatalf
    Description : Non-fatal error handling: pops up a dialog but does not
                    exit the game.
    Inputs      : same as dbgFatalf
    Outputs     :
    Return      : 0
----------------------------------------------------------------------------*/
sdword dbgNonFatalf(char *file, sdword line, char *format, ...)
{
    va_list argList;
    char error[DBG_BufferLength];

    va_start(argList, format);                              //get first arg
    vsprintf(error, format, argList);                       //prepare output string
    va_end(argList);

    dbgNonFatal(file, line, error);                         //print the message
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : dbgStackDump
    Description : Dumps the stack to a file for debugging purposes.
    Inputs      : void
    Outputs     :
    Return      : filename of file written to, or NULL on error
----------------------------------------------------------------------------*/
#if DBG_STACK_CONTEXT
static char dbgStackFilename[PATH_MAX + 1];
udword dbgStackBase = 0;
char *dbgStackDump(void)
{
    udword nDwords, _ESP;
    char *blankPtr;
    struct tm *newtime;
    time_t aclock;
    FILE *fp;
    udword referenceAddress;

    if (dbgStackBase == 0)
    {
        return(NULL);
    }

    //find stack pointers
#if defined (_MSC_VER)
    _asm mov eax, esp
    _asm mov _ESP, eax
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ ( "movl %%esp, %0\n\t" : "=r" (_ESP) );
#endif

    _ESP = _ESP & (~3);                                     //round off to dword boundary
    dbgStackBase = dbgStackBase & (~3);
    nDwords = dbgStackBase - _ESP;

    //create a filename with the time in it
    time( &aclock );                                        //Get time in seconds
    newtime = localtime( &aclock );                         //Convert time to struct
    sprintf(dbgStackFilename, "stack-%s.dump", asctime(newtime));
    for (blankPtr = dbgStackFilename; *blankPtr; blankPtr++)
    {
        if (strchr(": \n\r", *blankPtr))
        {
            *blankPtr = '-';
        }
    }

    blankPtr = filePathPrepend(dbgStackFilename, FF_UserSettingsPath);
    if (!fileMakeDestinationDirectory(blankPtr))
        return NULL;

    fp = fopen(blankPtr, "wb");                             //open the dump file
    if (fp == NULL)
    {
        return(NULL);
    }
    fwrite(&_ESP, sizeof(udword), 1, fp);                   //write the stack reference
    referenceAddress = (udword)&dbgFatalf;
    fwrite(&referenceAddress, sizeof(udword), 1, fp);       //write the .text reference
    while (nDwords)
    {
        fwrite((udword *)_ESP, sizeof(udword), 1, fp);      //write the stack block
        nDwords--;
        _ESP++;
    }
    fclose(fp);                                             //close the file
    return(dbgStackFilename);
}
#endif //DBG_STACK_CONTEXT

