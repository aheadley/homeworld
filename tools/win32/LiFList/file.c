/*=============================================================================
    File.C: Functions for reading and saving files, including .BIG files.

    Created June 1997 by Luke Moloney
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include "types.h"
#include "memory.h"
#include "debug.h"
#include "file.h"

/*=============================================================================
    Data:
=============================================================================*/
#if FILE_PREPEND_PATH
char filePrependPath[_MAX_PATH];
char filePrependedPath[_MAX_PATH];
#endif

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : fileBIGFileOpen
    Description : Open a .BIG file and read in it's directory.
    Inputs      : fileName - name of file on disk
    Outputs     : .BIG file directory is loaded into RAM
    Return      : bigHandle - handle to .BIG file.
    Note        : If .BIG file not found, a fatal error is generated.
----------------------------------------------------------------------------*/
bighandle fileBIGFileOpen(char *fileName)
{
#if FILE_VERBOSE_LEVEL >= 1
    dbgMessagef("\nfileBIGFileOpen: %s", fileName);
#endif
    fileName;                                               //do nothing
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : fileBIGFileClose
    Description : Closes a .BIG file, freeing up it's directory memory.
    Inputs      : handle - handle to the .BIG file to be freed.  If
        FBF_AllBIGFiles, all .BIG files will be closed.
    Outputs     : Memory allocated to directory freed.
    Return      : void
----------------------------------------------------------------------------*/
void fileBIGFileClose(bighandle handle)
{
#if FILE_VERBOSE_LEVEL >= 1
    dbgMessagef("\nfileBIGFileClose: 0x%x", handle);
#endif
    handle;                                                 //do nothing
}

/*-----------------------------------------------------------------------------
    Name        : fileLoadAlloc
    Description : Loads specified file into memory after allocating memory for it.
    Inputs      : fileName - path and name of file to be loaded.
                  address - location to store the allocated pointer.
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

    dbgAssert(address != NULL);

    fileName = filePathPrepend(_fileName);                   //get full path

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
    *address = memAllocAttempt(length, memoryName, 0);      //allocate the memory for this file
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

    fileName = filePathPrepend(_fileName);                   //get full path

    length = fileSizeGet(_fileName, flags);                  //get length of file

    if ((inFile = fopen(fileName, "rb")) == NULL)           //open the file
    {
        dbgFatalf(DBG_Loc, "fileLoadAlloc: coundn't open file %s", fileName);
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
    dbgMessagef("\nfileLoad: loaded %d bytes of '%s' to 0x%x from 0x%x", length, fileName, address, inFile);
#endif

    return(length);
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
    struct _finddata_t findData;
    char *fileName;

    fileName = filePathPrepend(_fileName);                   //get full path

    if (_findfirst(fileName, &findData) != -1)
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

    fileName = filePathPrepend(_fileName);                   //get full path

    if ((file = fopen(fileName, "rb")) == NULL)             //open the file
    {
        dbgFatalf(DBG_Loc, "fileSizeGet: can't find file %s", fileName);
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

/*-----------------------------------------------------------------------------
    Name        : fileOpen
    Description : Open specified file for reading
    Inputs      : fileName - name of file to be opened
                  flags - flags controlling where to find it and how to open it.
    Outputs     : if it is a .BIG file, allocated and init's a bigfile structure
    Return      : returns a handle (may be FILE * or an index into array of
                    open .BIG subfiles.
----------------------------------------------------------------------------*/
filehandle fileOpen(char *_fileName, udword flags)
{
    FILE *file;
    char access[3];
    char *fileName;

    fileName = filePathPrepend(_fileName);                   //get full path

    if (bitTest(flags, FF_WriteMode))
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

    if ((file = fopen(fileName, access)) == NULL)
    {
        dbgFatalf(DBG_Loc, "fileOpen: cannot open file %s", fileName);
    }
#if FILE_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfileOpen: '%s' handle 0x%x", fileName, file);
#endif
    return((filehandle)file);
}

/*-----------------------------------------------------------------------------
    Name        : fileClose
    Description : Closes a previously open file.
    Inputs      : handle - handle to the file to be closed
    Outputs     : may free up a .BIG file structure
    Return      : void
    Note        : generates an error if the file not open and debugging enabled
----------------------------------------------------------------------------*/
void fileClose(filehandle handle)
{
    dbgAssert(handle != NULL);

    fclose((FILE *)handle);
#if FILE_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfileClose: handle '0x%x'", handle);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : fileSeek
    Description : Seek to a new location in specified file.
    Inputs      : handle - handle to open file
                  offset - of new location
                  whence - where to seek from
    Outputs     : ..
    Return      : new offset in file
    Note        : If debugging enabled, this function will generate an fatal error
        if you try to seek past the end or before the beginning of a .BIG subfile.
----------------------------------------------------------------------------*/
sdword fileSeek(filehandle handle, sdword offset, sdword whence)
{
    sdword newLocation;

    dbgAssert(offset >= 0);

    newLocation = fseek((FILE *)handle, offset, whence);

    dbgAssert(newLocation == 0);

    newLocation = ftell((FILE *)handle);
#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileSeek: handle '0x%x' seeked %d bytes (mode %d) to %d", handle, offset, whence, newLocation);
#endif
    return(newLocation);
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

    dbgAssert(handle != NULL);

    lengthRead = fread(dest, 1, nBytes, (FILE *)handle);    //read in the data

#if FILE_ERROR_CHECKING
    if (lengthRead != nBytes)
    {                                                       //make sure it was all read in
        dbgFatalf(DBG_Loc, "fileBlockRead: expected %d bytes, read %d", nBytes, lengthRead);
    }
#endif
#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileBlockRead: handle '0x%x' read %d bytes to 0x%x", handle, lengthRead, dest);
#endif
    return(lengthRead);
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

    dbgAssert(nChars > 0);                                  //validate the params
    dbgAssert(handle != NULL);
    dbgAssert(dest != NULL);

    fgets(dest, nChars, (FILE *)handle);                    //get the bytes from file
    length = strlen(dest);                                  //get length of string

    if (length)                                             //if not a blank line
    {
        if (dest[length - 1] == '\n')                       //if newline at end of line
        {
            dest[length - 1] = 0;                           //kill newline character
            length--;                                       //one less character
        }
    }
    if (feof((FILE *)handle))                               //check if end of file
        return(FR_EndOfFile);

#if FILE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nfileLineRead: handle '0x%x' read %d chars to 0x%x ('%s')", handle, length, dest, dest);
#endif
    return(length);
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
#if FILE_VERBOSE_LEVEL >= 3
    int c;

    c = fgetc((FILE *)handle);
    dbgMessagef("\nfileCharRead: handle '0x%x' read char '%c'", handle, c);
    return(c);
#else
    return(fgetc((FILE *)handle));
#endif
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
    dbgAssert(path != NULL);
    strcpy(filePrependPath, path);                          //make copy of specified path
    dbgAssert(strlen(filePrependPath));
    if (filePrependPath[strlen(filePrependPath) - 1] != '\\')
    {                                                       //make sure a backslash is on the end
        strcat(filePrependPath, "\\");                      //put a backslash on the end
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : filePathPrepend
    Description : Prepend the default path for opening files.
    Inputs      : fileName - file name/relative path to add to end of default path
    Outputs     : Copies prepend path to global variable and concatenates fileName
    Return      : pointer to new full path
----------------------------------------------------------------------------*/
char *filePathPrepend(char *fileName)
{
#if FILE_VERBOSE_LEVEL >= 1
//can't print debug messages yet because it gets called so early on
//    dbgMessagef("\nfilePathPrepend:  File search path set to '%s'", fileName);
#endif
    dbgAssert(fileName != NULL);
    strcpy(filePrependedPath, filePrependPath);             //get the prepend path
    strcat(filePrependedPath, fileName);                    //put the file name on the end
    return(filePrependedPath);
}

