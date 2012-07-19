/*=============================================================================
    Name    : StackDump.c
    Purpose : Analyze a stack dump and display a call stack from the .MAP file.

    Created 7/18/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include "debug.h"
#include "memory.h"

/*=============================================================================
    Definitions:
=============================================================================*/
#define SD_LabelListInc         20000

/*=============================================================================
    Types:
=============================================================================*/
typedef struct
{
    char *label;
    char *module;
    udword address;
}
sdfunctionlabel;

/*=============================================================================
    Data:
=============================================================================*/
void *utyMemoryHeap;
sdword MemoryHeapSize = MEM_HeapSizeDefault;
char errorString[256];

sdfunctionlabel *sdFunctions = NULL;
sdword sdNLabels = 0;
sdword sdNAllocated = 0;
udword minFunctionAddress, maxFunctionAddress;

udword *sdDump = NULL;
sdword sdDumpLength;

udword sdReferenceAddress = 0;
udword sdReferenceOffset = 0;
udword sdStackReference = 0;
udword sdStackOffset = 0;

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : llSystemsStartup
    Description : Start up systems required by program
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
char *llSystemsStartup(void)
{
    //memory
    utyMemoryHeap = (void *)GlobalAlloc(GPTR, MemoryHeapSize + sizeof(memcookie) * 4);
    if (utyMemoryHeap == NULL)
    {
        sprintf(errorString, "Error allocating heap of size %d", MemoryHeapSize);
        return(errorString);
    }
    if (memInit(utyMemoryHeap, MemoryHeapSize) != OKAY)
    {
        sprintf(errorString, "Error starting memory manager with heap size %d at 0x%x", MemoryHeapSize, errorString);
        return(errorString);
    }

    //allocate a blank element list
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : llSystemsShutdown
    Description : Shut down systems required by the system
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void llSystemsShutdown(void)
{
    //memory
    GlobalFree(utyMemoryHeap);
}

/*-----------------------------------------------------------------------------
    Name        : sdDumpFileLoad
    Description : Load in the stack dump file.
    Inputs      : dumpFile - path and name of file to load
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void sdDumpFileLoad(char *dumpFile)
{
    FILE *fp;
    sdword length, nRead;
    udword *readPtr;

    fp = fopen(dumpFile, "rb");
    if (fp == NULL)
    {
        dbgFatalf(DBG_Loc, "Cannot open dump file '%s'", dumpFile);
    }
    printf("\nLoading dump file '%s'", dumpFile);
    length = fseek(fp, 0, SEEK_END);                      //get length of file
    dbgAssert(length == 0);
    length = ftell(fp);
    rewind(fp);
    sdDumpLength = length / sizeof(udword);
    nRead = fread(&sdStackReference, sizeof(udword), 1, fp);//first dword is the reference label "dbgFatalf"
    dbgAssert(nRead == 1);
    length -= sizeof(udword);
    nRead = fread(&sdReferenceAddress, sizeof(udword), 1, fp);//first dword is the reference label "dbgFatalf"
    dbgAssert(nRead == 1);
    length -= sizeof(udword);
    readPtr = sdDump = memAlloc(length, "Dump", 0);
    sdStackOffset = sdStackReference - (udword)sdDump;      //compute stack offset
    length /= sizeof(udword);
    while (length)
    {
        nRead = fread(readPtr, sizeof(udword), 1, fp);
        dbgAssert(nRead == 1);
        length--;
        readPtr++;
    }

    fclose(fp);
}

/*-----------------------------------------------------------------------------
    Name        : sdLabelListLoad
    Description : Load in a list of function pointers from a .MAP file.
    Inputs      : mapFile - mapfile to load in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void sdLabelListLoad(char *mapFile)
{
    char readLine[256];
    char textSegmentString[10];
    char *string, *label, *module;
    FILE *fp;
    sdword nScanned;
    udword textSegment = 0xffffffff;
    udword segment, base, length;
    sdfunctionlabel *newAlloc;

    fp = fopen(mapFile, "rt");

    if (fp == NULL)
    {
        dbgFatalf(DBG_Loc, "Cannot open map file '%s'.", mapFile);
    }

    printf("\nLoading MAP file '%s'", mapFile);
    while (!feof(fp))
    {
        if (!fgets(readLine, 255, fp))
        {
            break;
        }
        if (sdNLabels >= sdNAllocated)
        {
            printf(".");
            sdNAllocated += SD_LabelListInc;
            newAlloc = memAlloc(sizeof(sdfunctionlabel) * sdNAllocated, "LabelList", 0);
            if (sdFunctions != NULL)
            {
                memcpy(newAlloc, sdFunctions, sizeof(sdfunctionlabel) * sdNAllocated);
                //memFree(sdFunctions);
            }
            sdFunctions = newAlloc;
        }
        if (strstr(readLine, " .text ") != NULL)
        {                                                   //definition of the code segment
            dbgAssert(strstr(readLine, "CODE"));
            dbgAssert(textSegment == 0xffffffff);
            nScanned = sscanf(readLine, "%x:%x %xH", &segment, &base, &length);
            dbgAssert(nScanned == 3);
            minFunctionAddress = base;
            maxFunctionAddress = base + length;
            textSegment = segment;
            dbgAssert(textSegment < 10);
            sprintf(textSegmentString, "000%d:", textSegment);
        }
        else
        {
            if (textSegment != 0xffffffff)
            {
                if ((string = strtok(readLine, " \t\n")) != NULL)
                {                                           //found anything at all
                    if (strstr(string, textSegmentString) == string)
                    {                                       //found segment/address
                        nScanned = sscanf(string, "%x:%x", &segment, &base);
                        if (nScanned == 2 && base >= minFunctionAddress && base <= maxFunctionAddress)
                        {
                            if ((string = strtok(NULL, " \t\n")) != NULL)
                            {                               //found label
                                label = string;
                                if ((string = strtok(NULL, " \t\n")) != NULL)
                                {                           //found preferred load address
                                    if ((string = strtok(NULL, " \t\n")) != NULL)
                                    {                           //found 'f'
                                        if ((string = strtok(NULL, " \t\n")) != NULL)
                                        {                   //found module name
                                            module = string;
                                            sdFunctions[sdNLabels].label = memAlloc(strlen(label) + strlen(module) + 2, "LabelModule", 0)
                                            strcpy(sdFunctions[sdNLabels].label, label);
                                            sdFunctions[sdNLabels].module = sdFunctions[sdNLabels].label + strlen(label) + 1;
                                            strcpy(sdFunctions[sdNLabels].module, module);
                                            sdFunctions[sdNLabels].address = base;
                                            sdNLabels++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (textSegment == 0xffffffff || sdNLabels < 10)
    {
        dbgFatalf(DBG_Loc, "Could not load text segment from map file '%s'.", mapFile);
    }
    fclose(fp);
}

/*-----------------------------------------------------------------------------
    Name        : sdLabelFind
    Description : Find a named label.
    Inputs      : label - label to find
    Outputs     :
    Return      : label structure
----------------------------------------------------------------------------*/
sdfunctionlabel *sdLabelFind(char *label)
{
    sdword index;

    for (index = 0; index < sdNLabels; index++)
    {
        if (!strcmp(label, sdFunctions[index].label))
        {
            return(&sdFunctions[index]);
        }
    }
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : sdLabelFindByAddress
    Description : Find a label based on an address.
    Inputs      : address - address of label to find
    Outputs     :
    Return      : label that most closely preceeds the address specified
----------------------------------------------------------------------------*/
sdfunctionlabel *sdLabelFindByAddress(udword address)
{
    sdword nJump, index;

    index = sdNLabels / 2;
    nJump = index / 2;

    while (address < sdFunctions[index].address || address >= sdFunctions[index + 1].address)
    {
        if (address < sdFunctions[index].address)
        {
            index -= nJump;
        }
        else
        {
            index += nJump;
        }
        dbgAssert(index >= 0);
        if (index >= sdNLabels - 1)
        {
            index = sdNLabels - 2;
        }
        nJump = max(nJump / 2, 1);
    }
    return(&sdFunctions[index]);
}

/*-----------------------------------------------------------------------------
    Name        : sdDumpAnalyze
    Description : Analyze the stack dump and print out a call stack
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void sdDumpAnalyze(void)
{
    //sdword index;
    sdfunctionlabel *label;
    udword *stackPointer;
    udword iStack;
    udword address;

    //start by finding the reference function
    label = sdLabelFind("_dbgFatalf");
    if (label == NULL)
    {
        dbgFatal(DBG_Loc, "Cannot locate reference label '_dbgFatalf'.");
    }
    sdReferenceOffset = sdReferenceAddress - label->address;

    //now find one of these reference labels so we know how to bias all the loaded label addresses
    iStack = sdDumpLength;
    stackPointer = sdDump;

    while (iStack)
    {
        address = *stackPointer;
        if (address >= sdReferenceOffset)
        {                                                   //if this dword has a chance of being a reference value
            address -= sdReferenceOffset;
            if (address >= minFunctionAddress && address <= maxFunctionAddress)
            {                                               //if it's within the .text address space
                label = sdLabelFindByAddress(address);
                printf("\n+0x%x: %s + 0x%x  (%s)", (udword)stackPointer - (udword)sdDump, label->label, address - label->address, label->module);
            }
        }
        iStack--;
        stackPointer++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : main
    Description : main body of program
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
    char *string;

    if ((string = llSystemsStartup()) != NULL)
    {
        printf(string);
        return(0xfed5);
    }

    if (argc < 3)
    {
        printf("usage: %s <dumpFile> <mapFile>\n");
        return(0xfed5);
    }

    sdDumpFileLoad(argv[1]);
    sdLabelListLoad(argv[2]);

    sdDumpAnalyze();

    llSystemsShutdown();
    return(0);
}

