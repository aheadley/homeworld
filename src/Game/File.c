/*=============================================================================
    File.C: Functions for reading and saving files, including .BIG files.

    Created June 1997 by Luke Moloney
    Updated August 1998 by Darren Stone - added bigfile support
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "File.h"
#include "BigFile.h"
#include "BitIO.h"
#include "LZSS.h"

//
//  How to interpret the LOGFILELOADS output log:
//
//  3 columns:  main_bigfile  update_bigfile  filesystem
//  M = main bigfile
//  U = update bigfile
//  F = filesystem
//  Each column may be blank (indicating the file was not present there),
//  or have a lowercase letter there (m/u/f) (indicating that it had an
//  old or unused version of the file there), or have an uppercase letter
//  there ([M]/[U]/[F]) (indicating that the file there was used for the
//  load).
//
//  WARNING: If you run with comparebigfiles OFF, it could invalidate some
//  of the conclusions that this output draws.
//




// bigfile externs -- options and structures from bigfile.c
extern bool IgnoreBigfiles;
extern bool CompareBigfiles;
extern bool LogFileLoads;
extern bigTOC updateTOC;
extern bigTOC mainTOC;
extern unsigned char *mainNewerAvailable;
extern unsigned char *updateNewerAvailable;
extern FILE *mainFP;
extern FILE *updateFP;

/*=============================================================================
    Data:
=============================================================================*/
#if FILE_PREPEND_PATH
char filePrependPath[PATH_MAX + 1];
char fileCDROMPath[PATH_MAX + 1];
char fileUserSettingsPath[PATH_MAX + 1];
char filePrependedPath[PATH_MAX + 1];
#endif

// filehandles are an index into this array
// (NOTE:  the first entry is wasted -- since a filehandle of 0
//  usually indicates an error, we don't want to use it.  And having
//  to add one to the index all the time seems like an invitation
//  for bugs.)
fileOpenInfo filesOpen[MAX_FILES_OPEN+1];

//  space required for opening and reading from compressed streams within a bigfile
static char  *decompWorkspaceP = NULL;
static sdword decompWorkspaceSize = 0;
static sdword decompWorkspaceInUse = FALSE;
#define decompWorkspaceIncrement 65536;


/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : fileNameReplaceSlashes
    Description : Replaces slashes in the file name with slashes used by the
                  target platform to separate path components in the local
                  filesystem.
    Inputs      : fileName - Path and name of file.
    Outputs     : fileName - Modified file name.
    Return      : (None.)
    Note        : This is not needed for use with BIG files (slashes are
                  replaced as needed within BIG file functions).  Slashes are
                  also not condensed as with filenameSlashMassage() in
                  BigFile.c.
----------------------------------------------------------------------------*/
static void fileNameReplaceSlashes (char* fileName)
{
    char ch;

    for ( ; (ch = *fileName); fileName++)
    {
#ifdef _WIN32
        if (ch == '/')
            *fileName = '\\';
#else
        if (ch == '\\')
            *fileName = '/';
#endif
    }
}


/*-----------------------------------------------------------------------------
    Name        : fileNameReducePath
    Description : Removes extraneous slashes in the given path and reduces
                  occurences of "." and "..".
    Inputs      : pathName - File path.
    Return      : (None.)
----------------------------------------------------------------------------*/
static void fileNameReducePath (char *pathName)
{
    size_t pathLen;

    char *pathElem[PATH_MAX];
    udword pathElemCount;

    size_t searchLoc;
    bool8 isPathAbsolute;

    udword i;

    dbgAssert(pathName != NULL);

    pathLen = strlen(pathName);

    /* Reduce slashes. */
    for (i = 1; i < pathLen; i++)
    {
        char chPrev = pathName[i - 1];
        char chCurr = pathName[i];
        if ((chPrev == '/' || chPrev == '\\') &&
            (chCurr == '/' || chCurr == '\\'))
        {
            memmove(pathName + i - 1, pathName + i, pathLen - i + 1);
            pathLen--;
            i--;
        }
    }

    /* Remove trailing slashes. */
    while (pathName[pathLen - 1] == '/' ||
           pathName[pathLen - 1] == '\\')
    {
        pathLen--;
        pathName[pathLen] = '\0';
    }

    /* Split up the path string into an array containing each path
       component. */
    pathElem[0] = pathName;
    pathElemCount = 1;

    searchLoc = strcspn(pathName, "/\\");
    while (searchLoc != pathLen)
    {
        size_t searchLocPrev = searchLoc;

        pathName[searchLoc] = '\0';

        pathElem[pathElemCount] = pathName + searchLoc + 1;
        pathElemCount++;

        searchLoc = strcspn(pathName + searchLocPrev + 1, "/\\");
        searchLoc += searchLocPrev + 1;
    }

#ifdef _WIN32
    isPathAbsolute = (isalpha[pathName[0]] && pathName[1] == ':' &&
        pathName[2] == '\0');
#else
    isPathAbsolute = (pathName[0] == '\0');
#endif

    if (isPathAbsolute)
    {
        /* Skip over the root path element, we don't need to look at it for
           now. */
        pathElemCount--;
        if (pathElemCount != 0)
        {
            memmove(pathElem, pathElem + 1, sizeof(char *) * pathElemCount);
        }
    }

    /* Using our array of path components, reduce any occurences of "." and
       "..". */
    for (i = 0; i < pathElemCount; i++)
    {
        char *currentElem = pathElem[i];

        if (currentElem[0] != '.')
            continue;

        if (currentElem[1] == '\0')
        {
            /* The current path element refers to the current directory ("."),
               so we can remove it. */
            pathElemCount--;
            if (i < pathElemCount)
            {
                memmove(pathElem + i, pathElem + i + 1,
                    sizeof(char *) * pathElemCount - i);
            }
            i--;

            continue;
        }

        if (!(currentElem[1] == '.' && currentElem[2] == '\0'))
            continue;

        /* The current path element refers to the parent directory (".."). */
        if (i == 0)
        {
            if (isPathAbsolute)
            {
                dbgMessagef("\nfileNameReducePath(): Attempted to reach a "
                            "parent directory of the root directory.");

                /* Attempting to reach a directory below the root directory, so
                   just get rid of the current element. */
                pathElemCount--;
                if (pathElemCount != 0)
                {
                    memmove(pathElem, pathElem + 1,
                        sizeof(char *) * pathElemCount);
                }
                i--;
            }

            continue;
        }

        if (!strcmp(pathElem[i - 1], ".."))
        {
            /* Previous path element also represents the previous directory, so
               keep this element and allow further traversal up the directory
               tree. */
            continue;
        }

        /* Remove the current and previous path elements. */
        dbgAssert(pathElemCount >= 2);
        pathElemCount--;
        if (i < pathElemCount)
        {
            memmove(pathElem + i - 1, pathElem + i + 1,
                sizeof(char *) * pathElemCount - i);
        }
        pathElemCount--;
        i -= 2;
    }

    /* Rebuild the path string. */
    pathLen = 0;

    if (isPathAbsolute)
    {
#ifdef _WIN32
        pathName[2] = '\\';
        pathLen = 3;
#else
        pathName[0] = '/';
        pathLen = 1;
#endif
    }

    if (pathElemCount == 0)
    {
        pathName[pathLen] = '\0';
        return;
    }

    for (i = 0; i < pathElemCount; i++)
    {
        char *currentElem = pathElem[i];
        size_t elemLen = strlen(currentElem);

        memmove(pathName + pathLen, currentElem,
            sizeof(char) * elemLen);

        pathLen += elemLen;
#ifdef _WIN32
        pathName[pathLen] = '\\';
#else
        pathName[pathLen] = '/';
#endif
        pathLen++;
    }

    /* Remove the trailing slash. */
    pathName[pathLen - 1] = '\0';
}


/*-----------------------------------------------------------------------------
    Name        : fileMakeDirectory
    Description : Creates the specified directory, creating parent directories
                  as necessary.
    Inputs      : directoryName - Name of the directory to create.
    Return      : TRUE if successful, FALSE if not.
----------------------------------------------------------------------------*/
bool8 fileMakeDirectory (const char *directoryName)
{
    char directoryCopy[PATH_MAX + 1];
    size_t directoryLen;

    char *ch;
    udword i;

    dbgAssert(directoryName != NULL);
    dbgAssert(strlen(directoryName) <= PATH_MAX);

    /* Make a copy of the directory name with which we can modify as needed. */
    strncpy(directoryCopy, directoryName, PATH_MAX);
    directoryCopy[PATH_MAX] = '\0';

    /* Reduce the path name. */
    fileNameReducePath(directoryCopy);

    directoryLen = strlen(directoryCopy);
    if (directoryLen == 0)
        return TRUE;

#ifdef _WIN32
    if (directoryCopy[directoryLen - 1] != '\\')
    {
        directoryCopy[directoryLen] = '\\';
        directoryLen++;
        directoryCopy[directoryLen] = '\0';
    }
#else
    if (directoryCopy[directoryLen - 1] != '/')
    {
        directoryCopy[directoryLen] = '/';
        directoryLen++;
        directoryCopy[directoryLen] = '\0';
    }
#endif

    /* Find the first path element that isn't the root directory or a parent
       directory delimiter. */
#ifdef _WIN32
    ch = strchr(directoryCopy, '\\');
    if (ch)
    {
        *ch = '\0';

        if (isalpha(directoryCopy[0]) && directoryCopy[1] == ':' &&
            directoryCopy[2] == '\0')
        {
            *ch = '\\';
            ch = strchr(ch + 1, '\\');
            *ch = '\0';
        }
    }
#else
    ch = strchr(directoryCopy, '/');
    if (ch)
    {
        *ch = '\0';

        if (directoryCopy[0] == '\0')
        {
            *ch = '/';
            ch = strchr(ch + 1, '/');
            *ch = '\0';
        }
    }
#endif

    /* Create each directory as needed. */
    struct stat fileStat;
    while (ch)
    {
        *ch = 0;

        /* Check if the directory exists. */
        if (stat(directoryCopy, &fileStat) == 0)
        {
            /* A filesystem entry exists, so make sure it's a directory. */
            if (!S_ISDIR(fileStat.st_mode))
                return FALSE;
        }
        else
        {
            /* Attempt to create the directory. */
            if (mkdir(directoryCopy, 0777) == -1)
                return FALSE;
        }

        /* Continue with the next path element. */
#ifdef _WIN32
        *ch = '\\';
        ch = strchr(ch + 1, '\\');
#else
        *ch = '/';
        ch = strchr(ch + 1, '/');
#endif
    }

    return TRUE;
}


/*-----------------------------------------------------------------------------
    Name        : fileMakeDestinationDirectory
    Description : Creates the directory in which the given file resides.
    Inputs      : fileName - path and name of file.
    Return      : TRUE if the directory exists or could be created, FALSE if
                  not.
----------------------------------------------------------------------------*/
bool8 fileMakeDestinationDirectory(const char *fileName)
{
    char *directoryName[PATH_MAX + 1];
    char *ch0, *ch1;

    dbgAssert(fileName);

    /* Make a copy of the directory name string, excluding the file itself. */
    strncpy(directoryName, fileName, PATH_MAX);
    directoryName[PATH_MAX] = '\0';

    ch0 = strrchr(directoryName, '/');
    ch1 = strrchr(directoryName, '\\');
    if (ch1 > ch0)
        ch0 = ch1;

    /* If the file is in the current directory, assume the directory exists. */
    if (!ch0)
        return TRUE;

    /* Create the directory. */
    *ch0 = '\0';

    return fileMakeDirectory(directoryName);
}


/*-----------------------------------------------------------------------------
    Name        : fileLoadAlloc
    Description : Loads specified file into memory after allocating memory for it.
    Inputs      : fileName - path and name of file to be loaded.
                  address - location to store the allocated pointer.
                  flags - NonVolatile, Pyrophoric or file access flags
    Outputs     : *address - pointer to newly allocated and loaded file.
    Return      : number of bytes allocated and loaded.
    Note        : Generates a fatal error if file doesn't exist.  If you don't
        like this behavior, call fileExists() first.
----------------------------------------------------------------------------*/
sdword fileLoadAlloc(char *_fileName, void **address, udword flags)
{
    char *memoryName;
    sdword nameLength;
    sdword length, lengthRead;
    FILE *inFile;
    char *fileName;
    sdword bigfileResult;
    sdword mainFileNum;
    sdword updateFileNum;
    sdword existsInUpdateBigfile;
    sdword existsInMainBigfile;

    dbgAssert(address != NULL);

    //  try to load from bigfile
    if (!IgnoreBigfiles && !bitTest(flags, FF_CDROM|FF_IgnoreBIG|FF_UserSettingsPath))
    {
        // possibly still skip the bigfile version of the file
        // if there's something newer available in the filesystem
        existsInUpdateBigfile = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&updateFileNum);
        if (existsInUpdateBigfile && !(CompareBigfiles && updateNewerAvailable[updateFileNum] > 1))
        {
            bigfileResult = bigFileLoadAlloc(&updateTOC, updateFP, _fileName, updateFileNum, address);
            if (bigfileResult != -1)
            {
                if (LogFileLoads)
                {
                    existsInMainBigfile = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
                    logfileLogf(FILELOADSLOG, "%-80s |  %s [U] %s  |\n",
                                _fileName,
                                existsInMainBigfile ? "m" : " ",
                                updateNewerAvailable[updateFileNum] == 1 ? "f" : " ");
                }
                return bigfileResult;
            }
        }
        else if (!existsInUpdateBigfile)
        {
            // try the main bigfile
            existsInMainBigfile = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
            if (existsInMainBigfile && !(CompareBigfiles && mainNewerAvailable[mainFileNum] > 1))
            {
                bigfileResult = bigFileLoadAlloc(&mainTOC, mainFP, _fileName, mainFileNum, address);
                if (bigfileResult != -1)
                {
                    if (LogFileLoads)
                        logfileLogf(FILELOADSLOG, "%-80s | [M]    %s  |\n",
                                    _fileName,
                                    mainNewerAvailable[mainFileNum] == 1 ? "f" : " ");
                    return bigfileResult;
                }
            }

        }
    }

    // filesystem load

    fileName = filePathPrepend(_fileName, flags);           //get full path
    fileNameReplaceSlashes(fileName);

    nameLength = strlen(fileName);                          //set memory name to the
    dbgAssert(nameLength > 1);                              //end of the filename if filename too long
    if (nameLength > MEM_NameLength)
    {
        memoryName = &fileName[nameLength - MEM_NameLength];
    }
    else
        memoryName = fileName;

    length = fileSizeGet(_fileName, flags);                 //get size of file
    dbgAssert(length > 0);                                  //and verify it
    *address = memAllocAttempt(length, memoryName, (flags & (NonVolatile | Pyrophoric)) | MBF_String);//allocate the memory for this file
    if (*address == NULL)                                   //if couldn't allocate enough
    {                                                       //generate a fatal error
        dbgFatalf(DBG_Loc, "fileLoadAlloc: couldn't allocate %d bytes for %s", length, fileName);
    }

    if ((inFile = fopen(fileName, "rb")) == NULL)           //open the file
    {
        dbgFatalf(DBG_Loc, "fileLoadAlloc: coundn't open file %s", fileName);
    }
    lengthRead = fread(*address, 1, length, inFile);        //read the file

#if FILE_ERROR_CHECKING
    if (lengthRead != length)                               //verify correct amount read
    {
        dbgFatalf(DBG_Loc, "fileLoadAlloc: expected %d bytes, read %d bytes", length, lengthRead);
    }
#endif

    fclose(inFile);

#if FILE_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfileLoadAlloc: loaded %d bytes of '%s' to 0x%x from handle 0x%x", length, fileName, *address, inFile);
#endif

    if (LogFileLoads)
    {
        if (!IgnoreBigfiles && CompareBigfiles)
        {
            sdword inMain = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
            sdword inUpdate = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&updateFileNum);
            logfileLogf(FILELOADSLOG, "%-80s |  %s  %s [F] |\n",
                        _fileName,
                        inMain ? "m" : " ",
                        inUpdate ? "u" : " ");
        }
        else
            logfileLogf(FILELOADSLOG, "%-80s |       [F] |\n", _fileName);
    }

    return(length);
}

/*-----------------------------------------------------------------------------
    Name        : fileLoad
    Description : Loads named file into specified address.
    Inputs      : fileName - path/name of file to load
                  address - address to load it to
                  flags - various load flags
    Outputs     : Reads the entire file into the buffer pointed to by address.
    Return      : number of bytes loaded
    Note        : Generates a fatal error if file doesn't exist.  If you don't
        like this behavior, call fileExists() first.
----------------------------------------------------------------------------*/
sdword fileLoad(char *_fileName, void *address, udword flags)
{
    FILE *inFile;
    sdword length, lengthRead;
    char *fileName;
    sdword bigfileResult;
    sdword updateFileNum;
    sdword mainFileNum;
    sdword existsInUpdateBigfile;
    sdword existsInMainBigfile;

    //  try to load from bigfile
    if (!IgnoreBigfiles && !bitTest(flags, FF_CDROM|FF_IgnoreBIG|FF_UserSettingsPath))
    {
        // possibly still skip the bigfile version of the file
        // if there's something newer available in the filesystem
        existsInUpdateBigfile = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&updateFileNum);
        if (existsInUpdateBigfile && !(CompareBigfiles && updateNewerAvailable[updateFileNum] > 1))
        {
            bigfileResult = bigFileLoad(&updateTOC, updateFP, updateFileNum, address);
            if (bigfileResult != -1)
            {
                if (LogFileLoads)
                {
                    existsInMainBigfile = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
                    logfileLogf(FILELOADSLOG, "%-80s |  %s [U] %s  |\n",
                                _fileName,
                                existsInMainBigfile ? "m" : " ",
                                updateNewerAvailable[updateFileNum] == 1 ? "f" : " ");
                }
                return bigfileResult;
            }
        }
        else if (!existsInUpdateBigfile)
        {
            // try the main bigfile
            existsInMainBigfile = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
            if (existsInMainBigfile && !(CompareBigfiles && mainNewerAvailable[mainFileNum] > 1))
            {
                bigfileResult = bigFileLoad(&mainTOC, mainFP, mainFileNum, address);
                if (bigfileResult != -1)
                {
                    if (LogFileLoads)
                        logfileLogf(FILELOADSLOG, "%-80s | [M]    %s  |\n",
                                    _fileName,
                                    mainNewerAvailable[mainFileNum] == 1 ? "f" : " ");
                    return bigfileResult;
                }
            }

        }
    }

    // filesystem

    fileName = filePathPrepend(_fileName, flags);            //get full path
    fileNameReplaceSlashes(fileName);

    length = fileSizeGet(_fileName, flags);                  //get length of file

    if ((inFile = fopen(fileName, "rb")) == NULL)           //open the file
    {
        dbgFatalf(DBG_Loc, "fileLoadAlloc: couldn't open file %s", fileName);
    }
    lengthRead = fread(address, 1, length, inFile);         //read the file

#if FILE_ERROR_CHECKING
    if (lengthRead != length)                               //verify correct amount read
    {
        dbgFatalf(DBG_Loc, "fileLoadAlloc: expected %d bytes, read %d bytes", length, lengthRead);
    }
#endif
    fclose(inFile);

#if FILE_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfileLoad: loaded %d bytes of '%s' to 0x%x", length, fileName, address);
#endif

    if (LogFileLoads)
    {
        if (!IgnoreBigfiles && CompareBigfiles)
        {
            sdword inMain = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
            sdword inUpdate = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&mainFileNum);
            logfileLogf(FILELOADSLOG, "%-80s |  %s  %s [F] |\n",
                        _fileName,
                        inMain ? "m" : " ",
                        inUpdate ? "u" : " ");
        }
        else
            logfileLogf(FILELOADSLOG, "%-80s |       [F] |\n", _fileName);
    }

    return(length);
}

/*-----------------------------------------------------------------------------
    Name        : fileSave
    Description : Saves data of lenght,address to fileName
    Inputs      : fileName, address, length
    Outputs     :
    Return      : length
----------------------------------------------------------------------------*/
sdword fileSave(char *_fileName, void *address, sdword length)
{
    FILE *outFile;
    char *fileName;
    sdword lengthWrote;

    fileName = filePathPrepend(_fileName, 0);               //get full path
    fileNameReplaceSlashes(fileName);

    if (!fileMakeDestinationDirectory(fileName))
    {
        dbgFatalf(
            DBG_Loc,
            "fileSave: unable to create destination directory for file %s",
            fileName);
        return 0;
    }

    if ((outFile = fopen(fileName, "wb")) == NULL)           //open the file
    {
        dbgFatalf(DBG_Loc, "fileSave: couldn't open file %s", fileName);
    }

    lengthWrote = fwrite(address,1,length,outFile);

#if FILE_ERROR_CHECKING
    if (lengthWrote != length)                               //verify correct amount read
    {
        dbgFatalf(DBG_Loc, "fileSave: expected %d bytes, wrote %d bytes", length, lengthWrote);
    }
#endif

    fclose(outFile);

#if FILE_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfileSave: saved %d bytes of '%s' to 0x%x", length, fileName, address);
#endif

    return lengthWrote;
}

/*-----------------------------------------------------------------------------
    Name        : fileExistsInBigFile
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword fileExistsInBigFile(char *_fileName)
{
    sdword existsInBigfile;
    sdword fileNum;

    if (!IgnoreBigfiles)
    {
        existsInBigfile = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&fileNum);
        if (existsInBigfile)
        {
            return TRUE;
        }

        existsInBigfile = bigTOCFileExists(&mainTOC, _fileName, (int *)&fileNum);
        if (existsInBigfile)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : fileExists
    Description : Checks to see if a file exists
    Inputs      : fileName - name of file to find
                  flags - where to search (disk or in .BIG files)
    Outputs     : ..
    Return      : TRUE if file found, FALSE otherwise
----------------------------------------------------------------------------*/
sdword fileExists(char *_fileName, udword flags)
{
    struct stat file_stat;
    char *fileName;
    sdword existsInBigfile;
    sdword fileNum;

    if (!IgnoreBigfiles && !bitTest(flags, FF_CDROM|FF_IgnoreBIG|FF_UserSettingsPath))
    {
        // possibly still skip the bigfile version of the file
        // if there's something newer available in the filesystem
        existsInBigfile = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&fileNum);
        if (existsInBigfile && !(CompareBigfiles && updateNewerAvailable[fileNum] > 1))
        {
#if FILE_VERBOSE_LEVEL >= 3
            dbgMessagef("\nfileExists: '%s' exists (in update_bigfile)", _fileName);
#endif
            return TRUE;
        }
        else if (!existsInBigfile)
        {
            existsInBigfile = bigTOCFileExists(&mainTOC, _fileName, (int *)&fileNum);
            if (existsInBigfile && !(CompareBigfiles && mainNewerAvailable[fileNum] > 1))
            {
    #if FILE_VERBOSE_LEVEL >= 3
                dbgMessagef("\nfileExists: '%s' exists (in main_bigfile)", _fileName);
    #endif
                return TRUE;
            }
        }
    }

    fileName = filePathPrepend(_fileName, flags);            //get full path
    fileNameReplaceSlashes(fileName);

    if (stat(fileName, &file_stat) != -1)
    {
#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileExists: '%s' exists", fileName);
#endif
        return(TRUE);
    }
#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileExists: '%s' does not exist", fileName);
#endif
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : fileSizeGet
    Description : Finds the length, in bytes, of a file.
    Inputs      : fileName - name of file to find the length of.
                  flags - where to search for file (dosk or in .BIG files)
    Outputs     : ..
    Return      : Length of file.  Generates a fatal error if file doesn't exist.
    Note        : Because this may actually open the file to find it's length,
        it should never be called on files which are already open.
----------------------------------------------------------------------------*/
sdword fileSizeGet(char *_fileName, udword flags)
{
    FILE *file;
    sdword length;
    char *fileName;
    sdword fileNum;
    sdword existsInBigfile;

    if (!IgnoreBigfiles && !bitTest(flags, FF_CDROM|FF_IgnoreBIG|FF_UserSettingsPath))
    {
        // possibly still skip the bigfile version of the file
        // if there's something newer available in the filesystem
        existsInBigfile = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&fileNum);
        if (existsInBigfile && !(CompareBigfiles && updateNewerAvailable[fileNum] > 1))
        {
            length = (updateTOC.fileEntries + fileNum)->realLength;
#if FILE_VERBOSE_LEVEL >= 3
            dbgMessagef("\nfileSizeGet: '%s' is %d bytes in length (in update_bigfile)", _fileName, length);
#endif
            return length;
        }
        else if (!existsInBigfile)
        {
            existsInBigfile = bigTOCFileExists(&mainTOC, _fileName, (int *)&fileNum);
            if (existsInBigfile && !(CompareBigfiles && mainNewerAvailable[fileNum] > 1))
            {
                length = (mainTOC.fileEntries + fileNum)->realLength;
    #if FILE_VERBOSE_LEVEL >= 3
                dbgMessagef("\nfileSizeGet: '%s' is %d bytes in length (in main_bigfile)", _fileName, length);
    #endif
                return length;
            }
        }

    }

    fileName = filePathPrepend(_fileName, flags);            //get full path
    fileNameReplaceSlashes(fileName);

    if ((file = fopen(fileName, "rb")) == NULL)             //open the file
    {
#if defined(DLPublicBeta)
        udword *spp;
        _asm mov spp,esp
        dbgFatalf(DBG_Loc, "fileSizeGet: file error '%s'.  0x%x-(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)", fileName, (udword)fileSizeGet, *(spp - 7), *(spp - 6), *(spp - 5), *(spp - 4), *(spp - 3), *(spp - 2), *(spp - 1));
#else
        dbgFatalf(DBG_Loc, "fileSizeGet: can't find file '%s'.", fileName);
#endif
    }
    length = fseek(file, 0, SEEK_END);                      //get length of file
    dbgAssert(length == 0);
    length = ftell(file);
    fclose(file);
#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileSizeGet: '%s' is %d bytes in length", fileName, length);
#endif
    return(length);
}

void fileDelete(char *_fileName)
{
    char *fileName;

    fileName = filePathPrepend(_fileName, 0);               //get full path
    fileNameReplaceSlashes(fileName);

    remove(fileName);
}

/*-----------------------------------------------------------------------------
    Name        : fileOpen
    Description : Open specified file for reading
    Inputs      : fileName - name of file to be opened
                  flags - flags controlling where to find it and how to open it.
    Outputs     : creates a fileOpenInfo for subsequent accesses
    Return      : returns a handle (which may be reference a FILE * or the bigfile)
 ----------------------------------------------------------------------------*/
filehandle fileOpen(char *_fileName, udword flags)
{
    FILE *file;
    char access[3];
    char *fileName;
    sdword fileNum; // for bigfile
    filehandle fh;
    sdword existsInBigfile;
    sdword usingBigfile = FALSE;
    int expandedSize, storedSize;
    BIT_FILE *bitFile;
    sdword firstBufUse = FALSE;

    //  find next available filehandle
    fh = 1;
    while (fh <= MAX_FILES_OPEN)
    {
        if (!filesOpen[fh].inUse)
            break;
        ++fh;
    }
    if (fh > MAX_FILES_OPEN)
    {
        if (bitTest(flags, FF_ReturnNULLOnFail))
        {
            return 0;
        }
        dbgFatalf(DBG_Loc, "fileOpen: too many files open - cannot open file %s", _fileName);
    }

    strcpy(filesOpen[fh].path, _fileName);

    //  try to load from bigfile
    if (!IgnoreBigfiles && !bitTest(flags, FF_CDROM|FF_IgnoreBIG|FF_UserSettingsPath))
    {
        // possibly still skip the bigfile version of the file
        // if there's something newer available in the filesystem
        existsInBigfile = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&fileNum);
        if (existsInBigfile && !(CompareBigfiles && updateNewerAvailable[fileNum] > 1))
        {
            // no write support for bigfiles currently
            dbgAssert(!bitTest(flags, FF_AppendMode));
            dbgAssert(!bitTest(flags, FF_WriteMode));

            filesOpen[fh].usingBigfile = TRUE;
            filesOpen[fh].bigFP = updateFP;
            filesOpen[fh].bigTOC = &updateTOC;
            filesOpen[fh].textMode = bitTest(flags, FF_TextMode);
            filesOpen[fh].offsetStart = (filesOpen[fh].bigTOC->fileEntries + fileNum)->offset + (filesOpen[fh].bigTOC->fileEntries + fileNum)->nameLength + 1;
            filesOpen[fh].offsetVirtual = 0;
            filesOpen[fh].length = (filesOpen[fh].bigTOC->fileEntries + fileNum)->realLength;
            usingBigfile = TRUE;
        }
        else if (!existsInBigfile)
        {
            existsInBigfile = bigTOCFileExists(&mainTOC, _fileName, (int *)&fileNum);
            if (existsInBigfile && !(CompareBigfiles && mainNewerAvailable[fileNum] > 1))
            {
                // no write support for bigfiles currently
                dbgAssert(!bitTest(flags, FF_AppendMode));
                dbgAssert(!bitTest(flags, FF_WriteMode));

                filesOpen[fh].usingBigfile = TRUE;
                filesOpen[fh].bigFP = mainFP;
                filesOpen[fh].bigTOC = &mainTOC;
                filesOpen[fh].textMode = bitTest(flags, FF_TextMode);
                filesOpen[fh].offsetStart = (filesOpen[fh].bigTOC->fileEntries + fileNum)->offset + (filesOpen[fh].bigTOC->fileEntries + fileNum)->nameLength + 1;
                filesOpen[fh].offsetVirtual = 0;
                filesOpen[fh].length = (filesOpen[fh].bigTOC->fileEntries + fileNum)->realLength;
                usingBigfile = TRUE;
            }
        }

        if (usingBigfile)  // common stuff, whether it's in the main or update bigfile
        {
            if ((filesOpen[fh].bigTOC->fileEntries + fileNum)->compressionType)
            {
                // compressed file
                if (!decompWorkspaceInUse)
                {
                    if (decompWorkspaceSize < filesOpen[fh].length)  // workspace isn't big enough
                    {
                        // add a little extra room for the next possible reuse
                        // (an attempt to reduce too many memReallocs)
                        if (decompWorkspaceSize == 0)
                            firstBufUse = TRUE;
                        decompWorkspaceSize = filesOpen[fh].length + decompWorkspaceIncrement;
                        decompWorkspaceP = memRealloc(decompWorkspaceP, decompWorkspaceSize,
                                                      "decompWorkspace", 0);
                        if (LogFileLoads)
                        {
                            logfileLogf(FILELOADSLOG, "%-80s", _fileName);
                            if (filesOpen[fh].bigFP == updateFP)
                            {
                                sdword mainFileNum, updateFileNum;
                                sdword inMain = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
                                sdword inUpdate = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&updateFileNum);
                                logfileLogf(FILELOADSLOG, " |  %s [U] %s  | ",
                                            inMain ? "m" : " ",
                                            inUpdate && updateNewerAvailable[updateFileNum] == 1 ? "f" : " ");
                            }
                            else
                            {
                                sdword mainFileNum;
                                logfileLogf(FILELOADSLOG, " | [M]    %s  | ",
                                            mainNewerAvailable[mainFileNum] == 1 ? "f" : " ");
                            }

                            if (firstBufUse)
                                logfileLogf(FILELOADSLOG, "(decomp buffer created %dk)\n",
                                        decompWorkspaceSize/1024);
                            else
                                logfileLogf(FILELOADSLOG, "(decomp buffer increased to %dk)\n",
                                        decompWorkspaceSize/1024);
                        }
                    }
                    else  // reuse buffer
                    {
                        if (LogFileLoads)
                        {
                            logfileLogf(FILELOADSLOG, "%-80s", _fileName);
                            if (filesOpen[fh].bigFP == updateFP)
                            {
                                sdword mainFileNum, updateFileNum;
                                sdword inMain = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
                                sdword inUpdate = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&updateFileNum);
                                logfileLogf(FILELOADSLOG, " |  %s [U] %s  | ",
                                            inMain ? "m" : " ",
                                            inUpdate && updateNewerAvailable[updateFileNum] == 1 ? "f" : " ");
                            }
                            else
                            {
                                sdword mainFileNum;
                                logfileLogf(FILELOADSLOG, " | [M]    %s  | ",
                                            mainNewerAvailable[mainFileNum] == 1 ? "f" : " ");
                            }

                            logfileLogf(FILELOADSLOG, "(decomp buffer reused)\n");
                        }
                    }
                    filesOpen[fh].decompBuf = decompWorkspaceP;
                    decompWorkspaceInUse = TRUE;
                }
                else // allocate a new, temporary buffer
                {
                    filesOpen[fh].decompBuf = memAlloc(filesOpen[fh].length, "decompBuf", 0);

                    if (LogFileLoads)
                    {
                        logfileLogf(FILELOADSLOG, "%-80s", _fileName);
                        if (filesOpen[fh].bigFP == updateFP)
                        {
                            sdword mainFileNum, updateFileNum;
                            sdword inMain = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
                            sdword inUpdate = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&updateFileNum);
                            logfileLogf(FILELOADSLOG, " |  %s [U] %s  | ",
                                        inMain ? "m" : " ",
                                        inUpdate && updateNewerAvailable[updateFileNum] == 1 ? "f" : " ");
                        }
                        else
                        {
                            sdword mainFileNum;
                            logfileLogf(FILELOADSLOG, " | [M]    %s  | ",
                                        mainNewerAvailable[mainFileNum] == 1 ? "f" : " ");
                        }

                        logfileLogf(FILELOADSLOG, "(decomp buffer created %dk)\n",
                                    decompWorkspaceSize/1024);
                    }
                }
                // decompress from file directly into workspace
                fseek(filesOpen[fh].bigFP, filesOpen[fh].offsetStart, SEEK_SET);
                bitFile = bitioFileInputStart(filesOpen[fh].bigFP);
                expandedSize = lzssExpandFileToBuffer(bitFile, filesOpen[fh].decompBuf, filesOpen[fh].length);
                storedSize = bitioFileInputStop(bitFile);
                dbgAssert(expandedSize == filesOpen[fh].length);
                dbgAssert(storedSize == (filesOpen[fh].bigTOC->fileEntries + fileNum)->storedLength);
            }
            else
            {
                filesOpen[fh].decompBuf = NULL;
                if (LogFileLoads)
                {
                    logfileLogf(FILELOADSLOG, "%-80s", _fileName);
                    if (filesOpen[fh].bigFP == updateFP)
                    {
                        sdword mainFileNum, updateFileNum;
                        sdword inMain = bigTOCFileExists(&mainTOC, _fileName, (int *)&mainFileNum);
                        sdword inUpdate = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&updateFileNum);
                        logfileLogf(FILELOADSLOG, " |  %s [U] %s  | ",
                                    inMain ? "m" : " ",
                                    inUpdate && updateNewerAvailable[updateFileNum] == 1 ? "f" : " ");
                    }
                    else
                    {
                        sdword mainFileNum;
                        logfileLogf(FILELOADSLOG, " | [M]    %s  | ",
                                    mainNewerAvailable[mainFileNum] == 1 ? "f" : " ");
                    }

                    logfileLogf(FILELOADSLOG, "(uncompressed file)\n");
                }
            }
#if FILE_VERBOSE_LEVEL >= 2
            dbgMessagef("\nfileOpen: '%s' (from bigfile) handle 0x%x", _fileName, fh);
#endif

            filesOpen[fh].inUse = TRUE;
            return fh;
        }
    }

    // resort to the good old disk filesystem

    fileName = filePathPrepend(_fileName, flags);            //get full path
    fileNameReplaceSlashes(fileName);

    if (bitTest(flags, FF_AppendMode))
    {
        access[0] = 'a';
    }
    else if (bitTest(flags, FF_WriteMode))
    {
        access[0] = 'w';
    }
    else
    {
        access[0] = 'r';
    }

    if (bitTest(flags, FF_TextMode))
    {
        access[1] = 't';
    }
    else
    {
        access[1] = 'b';
    }

    access[2] = 0;

    if (LogFileLoads)
    {
        if (!IgnoreBigfiles && CompareBigfiles)
        {
            sdword inMain = bigTOCFileExists(&mainTOC, _fileName, (int *)&fileNum);
            sdword inUpdate = updateFP && bigTOCFileExists(&updateTOC, _fileName, (int *)&fileNum);
            logfileLogf(FILELOADSLOG, "%-80s |  %s  %s [F] |\n",
                        _fileName,
                        inMain ? "m" : " ",
                        inUpdate ? "u" : " ");
        }
        else
            logfileLogf(FILELOADSLOG, "%-80s |       [F] |\n", _fileName);
    }

    if (bitTest(flags, FF_AppendMode | FF_WriteMode) &&
        !fileMakeDestinationDirectory(fileName))
    {
        dbgFatalf(
            DBG_Loc,
            "fileOpen: unable to create destination directory for file %s",
            fileName);
        return 0;
    }

    if ((file = fopen(fileName, access)) == NULL)
    {
        if (bitTest(flags, FF_ReturnNULLOnFail))
        {
            return 0;
        }
        dbgFatalf(DBG_Loc, "fileOpen: cannot open file %s", fileName);
    }
#if FILE_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfileOpen: '%s' (from filesystem) handle 0x%x, FILE *0x%x", fileName, fh, file);
#endif

    filesOpen[fh].inUse = TRUE;
    filesOpen[fh].fileP = file;
    filesOpen[fh].usingBigfile = FALSE;
    return fh;
}

/*-----------------------------------------------------------------------------
    Name        : fileClose
    Description : Closes a previously open file.
    Inputs      : handle - handle to the file to be closed
    Outputs     :
    Return      : void
    Note        : generates an error if the file not open and debugging enabled
----------------------------------------------------------------------------*/
void fileClose(filehandle handle)
{
    dbgAssert(handle);
    dbgAssert(filesOpen[handle].inUse);

    if (!filesOpen[handle].usingBigfile)
        fclose(filesOpen[handle].fileP);
    else if (filesOpen[handle].decompBuf)
    {
        if (filesOpen[handle].decompBuf == decompWorkspaceP)
            decompWorkspaceInUse = FALSE;
        else
            // free decompression buffer if it was in use and not the stock workspace
            // (ie, we must have allocated it during fileOpen())
            memFree(filesOpen[handle].decompBuf);
    }

    filesOpen[handle].inUse = FALSE;

#if FILE_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfileClose: handle 0x%x", handle);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : fileSeek
    Description : Seek to a new location in specified file.
    Inputs      : handle - handle to open file
                  offset - of new location
                  whence - where to seek from
    Outputs     : ..
    Return      : new location
    Note        :
----------------------------------------------------------------------------*/
sdword fileSeek(filehandle handle, sdword offset, sdword whence)
{
    sdword newLocation;

    // dbgAssert(offset >= 0);
    dbgAssert(handle);
    dbgAssert(filesOpen[handle].inUse);

    if (filesOpen[handle].usingBigfile)
    {
        switch (whence)
        {
            case FS_Start:
                dbgAssert(!filesOpen[handle].textMode || !offset);  // we can seek to the start of a text file, but that's it
                newLocation =
                    (filesOpen[handle].offsetVirtual = offset);
                break;
            case FS_Current:
                dbgAssert(!filesOpen[handle].textMode);
                newLocation =
                    (filesOpen[handle].offsetVirtual += offset);
                break;
            case FS_End:
                dbgAssert(!filesOpen[handle].textMode);
                newLocation =
                    (filesOpen[handle].offsetVirtual = filesOpen[handle].length - 1 + offset);
                break;
        }
        dbgAssert(newLocation < filesOpen[handle].length);
    }
    else  // filesystem
    {
        newLocation = fseek(filesOpen[handle].fileP, offset, whence);
        dbgAssert(newLocation == 0);
        newLocation = ftell(filesOpen[handle].fileP);
    }

#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileSeek: handle 0x%x seeked %d bytes (mode %d) to %d", handle, offset, whence, newLocation);
#endif
    return newLocation;
}

/*-----------------------------------------------------------------------------
    Name        : fileBlockRead
    Description : Reads a block from the specified file.
    Inputs      : handle - handle to open file
                  dest - buffer to read the data to
                  nBytes - number of bytes to read, in bytes
    Outputs     : fills the dest buffer with nBytes read from handle
    Return      : number of bytes read.  If debugging enabled, we compare this
        to the amount requested and generate a fatal error if there is a difference.
----------------------------------------------------------------------------*/
sdword fileBlockRead(filehandle handle, void *dest, sdword nBytes)
{
    sdword lengthRead;

    dbgAssert(handle);
    dbgAssert(filesOpen[handle].inUse);

    if (filesOpen[handle].usingBigfile)
    {
        dbgAssert(!filesOpen[handle].textMode);
        dbgAssert(filesOpen[handle].offsetVirtual + nBytes <= filesOpen[handle].length);
        if (filesOpen[handle].decompBuf)
        {
            // from memory buffer
            memcpy(dest, filesOpen[handle].decompBuf + filesOpen[handle].offsetVirtual, nBytes);
            lengthRead = nBytes;
        }
        else
        {
            // from bigfile
            fseek(filesOpen[handle].bigFP, filesOpen[handle].offsetVirtual + filesOpen[handle].offsetStart, SEEK_SET);
            lengthRead = fread(dest, 1, nBytes, filesOpen[handle].bigFP);    //read in the data
        }
        filesOpen[handle].offsetVirtual += lengthRead;
    }
    else
    {
        lengthRead = fread(dest, 1, nBytes, filesOpen[handle].fileP);    //read in the data
    }

#if FILE_ERROR_CHECKING
    if (lengthRead != nBytes)
    {                                                       //make sure it was all read in
        dbgFatalf(DBG_Loc, "fileBlockRead: expected %d bytes, read %d", nBytes, lengthRead);
    }
#endif
#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileBlockRead: handle 0x%x read %d bytes to 0x%x", handle, lengthRead, dest);
#endif
    return lengthRead;
}

/*-----------------------------------------------------------------------------
    Name        : fileBlockReadNoError
    Description : Reads a block from the specified file.
    Inputs      : handle - handle to open file
                  dest - buffer to read the data to
                  nBytes - number of bytes to read, in bytes
    Outputs     : fills the dest buffer with nBytes read from handle
    Return      : number of bytes read.  This function will NOT generate an error
                  if fewer than expected bytes are read
----------------------------------------------------------------------------*/
sdword fileBlockReadNoError(filehandle handle, void *dest, sdword nBytes)
{
    sdword lengthRead, ReadAmt = nBytes;

    dbgAssert(handle);
    dbgAssert(filesOpen[handle].inUse);

    if (filesOpen[handle].usingBigfile)
    {
        dbgAssert(!filesOpen[handle].textMode);

        if(filesOpen[handle].offsetVirtual + nBytes > filesOpen[handle].length)
            ReadAmt = filesOpen[handle].length - filesOpen[handle].offsetVirtual;

        if (filesOpen[handle].decompBuf)
        {
            // from memory buffer
            memcpy(dest, filesOpen[handle].decompBuf + filesOpen[handle].offsetVirtual, ReadAmt);
            lengthRead = ReadAmt;
        }
        else
        {
            // from bigfile
            fseek(filesOpen[handle].bigFP, filesOpen[handle].offsetVirtual + filesOpen[handle].offsetStart, SEEK_SET);
            lengthRead = fread(dest, 1, ReadAmt, filesOpen[handle].bigFP);    //read in the data
        }
        filesOpen[handle].offsetVirtual += lengthRead;
    }
    else
    {
        lengthRead = fread(dest, 1, nBytes, filesOpen[handle].fileP);    //read in the data
    }

    return lengthRead;
}

/*-----------------------------------------------------------------------------
    Name        : fileLineRead
    Description : Reads a line from the specified file.
    Inputs      : handle - handle of open file
                  dest - destination string
                  nChars - length of string (dest)
    Outputs     : Reads up to nChars bytes from file converting CF/LF pairs to
        just a CR.  Removed LF characters are not counted in nChars
    Return      : Number of characters read or FR_EndOfFile if the end of file
                    reached.  Note that a zero return value reflects empty lines.
----------------------------------------------------------------------------*/
sdword fileLineRead(filehandle handle, char *dest, sdword nChars)
{
    sdword length;
    int ch;
    char *retVal;
    char *loc;

    dbgAssert(nChars > 0);                                  //validate the params
    dbgAssert(handle);
    dbgAssert(dest != NULL);
    dbgAssert(filesOpen[handle].inUse);

    if (filesOpen[handle].usingBigfile)
    {
        dbgAssert(filesOpen[handle].textMode);
        if (filesOpen[handle].offsetVirtual >= filesOpen[handle].length)
            return FR_EndOfFile;

        if (filesOpen[handle].decompBuf)
        {
            // fake the filesystem fgets stuff on a mem buffer
            length = 0;
            loc = (char *)filesOpen[handle].decompBuf + filesOpen[handle].offsetVirtual;
            while (length < nChars-1)
            {
                if (filesOpen[handle].offsetVirtual >= filesOpen[handle].length)
                    break;
                // getc
                ch = *(loc++);
                ++filesOpen[handle].offsetVirtual;
                if (ch == 13)
                {
                    // getc
                    ch = *(loc++);
                    ++filesOpen[handle].offsetVirtual;
                    if (ch != 10)
                    {
                        // ungetc
                        --loc;
                        --filesOpen[handle].offsetVirtual;
                    }
                    break;
                }
                *(dest+length) = ch;
                ++length;
            }
            *(dest+length) = 0;
        }
        else
        {
            fseek(filesOpen[handle].bigFP, filesOpen[handle].offsetVirtual + filesOpen[handle].offsetStart, SEEK_SET);
            // fake the filesystem fgets stuff (like below) here in binary mode
            length = 0;
            while (length < nChars-1)
            {
                if (feof(filesOpen[handle].bigFP) || filesOpen[handle].offsetVirtual >= filesOpen[handle].length)
                    break;
                ch = getc(filesOpen[handle].bigFP);
                if (ch == EOF)
                    break;
                ++filesOpen[handle].offsetVirtual;
                if (ch == 13)
                {
                    ch = getc(filesOpen[handle].bigFP);
                    if (ch != 10)
                        ungetc(ch, filesOpen[handle].bigFP);
                    else
                        ++filesOpen[handle].offsetVirtual;
                    break;
                }
                *(dest+length) = ch;
                ++length;
            }
            *(dest+length) = 0;
#if 0
            if (feof(filesOpen[handle].bigFP) ||
                filesOpen[handle].offsetVirtual >= filesOpen[handle].length)
            {
                return FR_EndOfFile;
            }
#endif
        }
    }
    else
    {
        if (feof(filesOpen[handle].fileP))                               //check if end of file
        {
            return FR_EndOfFile;
        }
        retVal = fgets(dest, nChars, filesOpen[handle].fileP);  //get the bytes from file
        if (retVal != dest)
        {
            return(FR_EndOfFile);
        }
        length = strlen(dest);                                  //get length of string

        if (length)                                             //if not a blank line
        {
            if (dest[length - 1] == '\n')                       //if newline at end of line
            {
                dest[length - 1] = 0;                           //kill newline character
                length--;                                       //one less character
            }
        }
    }

#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileLineRead: handle 0x%x read %d chars to 0x%x ('%s')", handle, length, dest, dest);
#endif
    return length;
}

/*-----------------------------------------------------------------------------
    Name        : fileCharRead
    Description : Read a character from a file.
    Inputs      : handle - handle of open file.
    Outputs     : ..
    Return      : Character read from file.
----------------------------------------------------------------------------*/
sdword fileCharRead(filehandle handle)
{
    sdword ch;

    dbgAssert(handle);
    dbgAssert(filesOpen[handle].inUse);

    if (filesOpen[handle].usingBigfile)
    {
        if (filesOpen[handle].offsetVirtual >= filesOpen[handle].length)
            return EOF;
        if (filesOpen[handle].decompBuf)
        {
            ch = filesOpen[handle].decompBuf[filesOpen[handle].offsetVirtual++];
        }
        else
        {
            fseek(filesOpen[handle].bigFP, filesOpen[handle].offsetVirtual + filesOpen[handle].offsetStart, SEEK_SET);
            ch = fgetc(filesOpen[handle].bigFP);
            if (ch != EOF)
                ++filesOpen[handle].offsetVirtual;
        }
        return ch;
    }
    else
        return fgetc(filesOpen[handle].fileP);
}


//
// for using regular underlying i/o stream (filesystem, not bigfile)
//
sdword fileUsingBigfile(filehandle handle)
{
    dbgAssert(handle);
    dbgAssert(filesOpen[handle].inUse);
    return filesOpen[handle].usingBigfile;
}


//
// for using regular underlying i/o stream (for files of the filesystem, not files of a bigfile)
//
FILE *fileStream(filehandle handle)
{
    dbgAssert(handle);
    dbgAssert(filesOpen[handle].inUse);
    return filesOpen[handle].fileP;
}


/*-----------------------------------------------------------------------------
    Name        : fileSetPrependPath
    Description : Set the path to look in for all file open requests.
    Inputs      : path - path to look in for files
    Outputs     : stores path in a global variable
    Return      : void
----------------------------------------------------------------------------*/
#if FILE_PREPEND_PATH
void filePrependPathSet(char *path)
{
    unsigned int path_len;
    dbgAssert(path != NULL);
    strcpy(filePrependPath, path);                          //make copy of specified path

    path_len = strlen(filePrependPath);
    dbgAssert(path_len);
#ifdef _WIN32
    if (filePrependPath[path_len - 1] != '\\')
    {                                                       //make sure a backslash is on the end
        strcat(filePrependPath, "\\");                      //put a backslash on the end
    }
#else
    if (filePrependPath[path_len - 1] != '\\' &&
        filePrependPath[path_len - 1] != '/')
    {                                    /* make sure a slash is on the end */
        strcat(filePrependPath, "/");    /* put a slash on the end */
    }
#endif
}
#endif

/*-----------------------------------------------------------------------------
    Name        : fileCDROMPathSet
    Description : Like above, but it sets the path for a CD-ROM file
    Inputs      : path - path to the root of the CD-ROM
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void fileCDROMPathSet(char *path)
{
    unsigned int path_len;
    dbgAssert(path != NULL);
    strcpy(fileCDROMPath, path);                            //make copy of specified path

    path_len = strlen(fileCDROMPath);
    dbgAssert(path_len);
#ifdef _WIN32
    if (fileCDROMPath[path_len - 1] != '\\')
    {                                                       //make sure a backslash is on the end
        strcat(fileCDROMPath, "\\");                        //put a backslash on the end
    }
#else
    if (fileCDROMPath[path_len - 1] != '\\' &&
        fileCDROMPath[path_len - 1] != '/')
    {                                    /* make sure a slash is on the end */
        strcat(fileCDROMPath, "/");      /* put a slash on the end */
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : fileUserSettingsPathSet
    Description : Like above, but it sets the user settings path for storing
                  configuration settings, saved games, and screenshots
                  (typically ~/.homeworld)
    Inputs      : path - path for user settings
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void fileUserSettingsPathSet(char *path)
{
    unsigned int path_len;
    dbgAssert(path != NULL);
    strcpy(fileUserSettingsPath, path);                     //make copy of specified path

    path_len = strlen(fileUserSettingsPath);
    dbgAssert(path_len);
#ifdef _WIN32
    if (fileUserSettingsPath[path_len - 1] != '\\')
    {                                                       //make sure a backslash is on the end
        strcat(fileUserSettingsPath, "\\");                 //put a backslash on the end
    }
#else
    if (fileUserSettingsPath[path_len - 1] != '\\' &&
        fileUserSettingsPath[path_len - 1] != '/')
    {                                       /* make sure a slash is on the end */
        strcat(fileUserSettingsPath, "/");  /* put a slash on the end */
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : filePathPrepend
    Description : Prepend the default path for opening files.
    Inputs      : fileName - file name/relative path to add to end of default path
                  flags - flags for the file we want.  Only the FF_CDROM and
                          FF_UserSettingsPath flags are used
    Outputs     : Copies prepend path to global variable and concatenates fileName
    Return      : pointer to new full path
----------------------------------------------------------------------------*/
char *filePathPrepend(char *fileName, udword flags)
{
#if FILE_VERBOSE_LEVEL >= 1
//can't print debug messages yet because it gets called so early on
//    dbgMessagef("\nfilePathPrepend:  File search path set to '%s'", fileName);
#endif
    dbgAssert(fileName != NULL);

    if (bitTest(flags, FF_IgnorePrepend))
    {
        strcpy(filePrependedPath,fileName);     // just copy, don't actually prepend
        return(filePrependedPath);
    }
    else if (bitTest(flags, FF_UserSettingsPath))
    {
        strcpy(filePrependedPath, fileUserSettingsPath);
    }
    else if (bitTest(flags, FF_CDROM))
    {
        strcpy(filePrependedPath, fileCDROMPath);           //get the CD-ROM path
    }
    else
    {
        strcpy(filePrependedPath, filePrependPath);         //get the prepend path
    }
    strcat(filePrependedPath, fileName);                    //put the file name on the end
    return(filePrependedPath);
}

/*-----------------------------------------------------------------------------
    Name        : logfileClear
    Description : clears the logfile
    Inputs      : logfile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void logfileClear(char *logfile)
{
    FILE *file;

    if ((file = fopen(logfile, "wt")) == NULL)      // open and close to make it 0 size
    {
        return;
    }
    fclose(file);
}

/*-----------------------------------------------------------------------------
    Name        : logFileLog
    Description : logs a log into the logfile
    Inputs      : logfile, str
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void logfileLog(char *logfile,char *str)
{
    FILE *file;

    if ((file = fopen(logfile, "at")) != NULL)      // open and close to make it 0 size
    {
        fprintf(file,str);
        fclose(file);
    }
}

/*-----------------------------------------------------------------------------
    Name        : logfileLogf
    Description : logs a log into the logfile, variable parameters
    Inputs      : logfile, str, ...
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void logfileLogf(char *logfile,char *format, ...)
{
    char buffer[200];
    va_list argList;
    va_start(argList, format);                              //get first arg
    vsprintf(buffer, format, argList);                      //prepare output string
    va_end(argList);

    logfileLog(logfile,buffer);
}


