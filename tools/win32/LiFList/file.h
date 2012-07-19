/*=============================================================================
    FILE.C:  Definitions for file I/O.  This includes .BIG files and regular
        files.

    Created June 1997 by Luke Moloney
=============================================================================*/

#ifndef ___FILE_H
#define ___FILE_H

#include <stdio.h>
#include "types.h"

/*=============================================================================
    Switches
=============================================================================*/
#ifdef HW_Debug

#define FILE_VERBOSE_LEVEL      1               //control level of verbose info
#define FILE_ERROR_CHECKING     1               //control error checking
#define FILE_OPEN_LOGGING       1               //display file names as they are opened
#define FILE_SEEK_WARNING       1               //display warnings as seeks are required
#define FILE_TEST               0               //test the file module
#define FILE_PREPEND_PATH       1               //prepend a fixed path to the start of all file open requests

#else //HW_Debug

#define FILE_VERBOSE_LEVEL      0               //control level of verbose info
#define FILE_ERROR_CHECKING     0               //control error checking
#define FILE_OPEN_LOGGING       0               //display file names as they are opened
#define FILE_SEEK_WARNING       0               //display warnings as seeks are required
#define FILE_TEST               0               //test the file module
#define FILE_PREPEND_PATH       1               //prepend a fixed path to the start of all file open requests

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//path length
#define PATH_Max                260

//file seek locations
#define FS_Start                SEEK_SET
#define FS_Current              SEEK_CUR
#define FS_End                  SEEK_END

//results of file stream operations
#define FR_EndOfFile            -12             //some unlikely number to indicate end of file

//flags for opening files
#define FF_TextMode             1               //open file in text mode
#define FF_IgnoreDisk           2               //don't search on disk, look straight in the .BIG file
#define FF_IgnoreBIG            4               //don't look in .BIG file, only try to load from disk
#define FF_IgnorePrepend        8               //don't add stock path to beginning of file names
#define FF_WriteMode            16              //open file for writing to

//names of log files
#define FN_OpenLog              "open.log"

//.BIG file definitions
#define FBF_AllBIGFiles         -1

//length of file path fragments
#define FL_Path                 256             //max length of a full path

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef sdword  bighandle;
typedef FILE *  filehandle;

/*=============================================================================
    Functions:
=============================================================================*/

//specify a path for all file open requests
#if FILE_PREPEND_PATH
void filePrependPathSet(char *path);
char *filePathPrepend(char *path);
#else
#define filePrependPathSet(p)
#define filePathPrepend(p)          (p)
#endif
//open/close .BIG files
bighandle fileBIGFileOpen(char *fileName);
void fileBIGFileClose(bighandle handle);

//load files directly into memory
sdword fileLoadAlloc(char *fileName, void **address, udword flags);
sdword fileLoad(char *fileName, void *address, udword flags);

//save files, if you want stream saving, use the ANSI C stream functions
sdword fileSave(char *fileName, void *address, sdword length);

//load files like ANSI C streams (fopen, fread etc)
filehandle fileOpen(char *fileName, udword flags);
void fileClose(filehandle handle);
sdword fileSeek(filehandle handle, sdword offset, sdword whence);
sdword fileBlockRead(filehandle handle, void *dest, sdword nBytes);
sdword fileLineRead(filehandle handle, char *dest, sdword nChars);
sdword fileCharRead(filehandle handle);

//utility functions
sdword fileExists(char *fileName, udword flags);
sdword fileSizeGet(char *fileName, udword flags);
sdword fileSizeRemaining(filehandle handle);
sdword fileLocation(filehandle handle);
sdword fileEndoOfFile(filehandle handle);
#endif //___FILE_H

