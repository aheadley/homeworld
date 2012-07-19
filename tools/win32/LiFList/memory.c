/*=============================================================================
    MEMORY.C:   Routines to manage memory
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "debug.h"
#include "memory.h"

/*=============================================================================
    Data:
=============================================================================*/
//total length of memory pool
sdword memPoolLength = MEM_HeapSizeDefault;

//global memory pool, from which all allocations are performed
ubyte *memPool;

//flag set when module started
sdword memModuleInit = FALSE;

//pointers to various cookies
memcookie *memFirst;                            //first cookie, start of heap
memcookie *memFirstFree;                        //lowest free cookie
//memcookie *memLastFree;                         //last free cookie
memcookie *memLast;                             //last cookie, end of heap

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : memInit
    Description : Starts the memory allocation module.  Call before any other
                    functions.
    Inputs      : heapStart - start of heap to use.  Should be aligned on at
                    lease 4-bit boundary.
                  heapSize - size of heap to create
    Outputs     : Global heap set up and (optionally) cleared.
    Return      : OKAY if success.
----------------------------------------------------------------------------*/
sdword memInit(void *heapStart, sdword heapSize)
{
#if MEM_ERROR_CHECKING
    if (memModuleInit == TRUE)
        dbgFatal(DBG_Loc, "Memory module started more than once.");
#endif //MEM_ERROR_CHECKING

    dbgAssert(heapStart != NULL && heapSize > MEM_BlockSize * 20);

#if MEM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nMemory module init.  Heap = 0x%x, Length = %d", heapStart, heapSize);
#endif //MEM_VERBOSE_LEVEL >= 1

    memPool = (ubyte *)(((udword)((ubyte *)heapStart + sizeof(memcookie) - 1)) & (~(sizeof(memcookie) - 1)));
    memPoolLength = heapSize;

    memModuleInit = TRUE;

    return(memReset());
}

/*-----------------------------------------------------------------------------
    Name        : memReset
    Description : Reset the memory module and optionally clear all memory.
    Inputs      : void
    Outputs     : Global heap set up and (optionally) cleared.
    Return      : OKAY on success.
----------------------------------------------------------------------------*/
sdword memReset(void)
{
    memInitCheck();

#if MEM_CLEAR_MEM
    memClearDword(memPool, MEM_ClearSetting, memPoolLength / sizeof(udword));
#endif

    //set pointer to first free memory cookie
    memFirst = memFirstFree = (memcookie *)memPool;

    //set pointer to end of pool
    memLast = (memcookie *)(memPool + memRoundDown(memPoolLength));

    //init first free cookie
    memFirstFree->flags = MBF_VerifyValue;
    memFirstFree->blocksNext = memBlocksCompute(memFirstFree, memLast) - 1;
    memFirstFree->blocksPrevious = -1;
    memNameSet(memFirstFree, MEM_NameHeap);
    memLast->blocksPrevious = memBlocksCompute(memFirstFree, memLast) - 1;
    memLast->flags = MBF_VerifyValue;

#if MEM_MODULE_TEST
    {
        ubyte *a, *b, *c, *d, *e, *f;

        a = memAlloc(10,    "TEST_A", 0);
        b = memAlloc(100,   "TEST_B", 0);
        c = memAlloc(1,     "TEST_C", 0);
        d = memAlloc(64,    "TEST_D", 0);
        e = memAlloc(10000, "TEST_E", 0);
        f = memAlloc(10,    "TEST_F", 0);
        memFree(b);
        memFree(c);
        memFree(d);
        memFree(f);
        memDefragment();
    }
#endif // MEM_MODULE_TEST

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : memClearDword
    Description : Clears the specified number of dwords to the specified pattern
    Inputs      : dest - pointer to location to recieve pattern
                  pattern - dword to be duplicated
                  nDwords - number of dwords to duplicate
    Outputs     :
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword memClearDword(void *dest, udword pattern, sdword nDwords)
{
    sdword index;
    udword *destP;

    destP = (udword *)dest;

    for (index = nDwords; index; index--, destP++)
    {
        *destP = pattern;
    }
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : memNameSetFunction
    Description : Sets the name of specified cookie
    Inputs      : cookie - pointer to memcookie structure
                  name - string to be copied
    Outputs     : cookie->name = string
    Return      : void
    Note        : If length of string is >= MEM_NameLength, it will be truncated
        at MEM_NameLength - 1 characters.  It will be NULL terminated.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
sdword memNameSetFunction(memcookie *cookie, char *name)
{
    memInitCheck();
    memCookieVerify(cookie);

    strncpy(cookie->name, name, MEM_NameLength);
    cookie->name[MEM_NameLength - 1] = 0;

#if MEM_VERBOSE_LEVEL >= 3
    dbgMessagef("\nmemNameSet: Cookie at 0x%x named to '%s'", cookie, cookie->name);
#endif
    return(OKAY);
}
#endif //MEM_USE_NAMES

/*-----------------------------------------------------------------------------
    Name        : memNameSetLongFunction
    Description : Sets the name of specified cookie
    Inputs      : cookie - pointer to memcookie structure
                  name - string to be copied
    Outputs     : cookie->name = end of string
    Return      : void
    Note        : If length of string is >= MEM_NameLength the last
                    MEM_NameLength characters will be used.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
sdword memNameSetLongFunction(memcookie *cookie, char *name)
{
    memInitCheck();
    memCookieVerify(cookie);

    if (strlen(name) >= MEM_NameLength - 1)
    {
        name = &name[strlen(name) - MEM_NameLength - 1];
    }
    strncpy(cookie->name, name, MEM_NameLength);
    cookie->name[MEM_NameLength - 1] = 0;

#if MEM_VERBOSE_LEVEL >= 3
    dbgMessagef("\nmemNameSetLong: Cookie at 0x%x named to '%s'", cookie, cookie->name);
#endif
    return(OKAY);
}
#endif //MEM_USE_NAMES

/*-----------------------------------------------------------------------------
    Name        : memRename
    Description : Renames a memory block by pointer,
    Inputs      : pointer - memory whose cookie is to be renamed
                  newName - string to be copied to name part of associated cookie
    Outputs     : cookie->name = string
    Return      : void
    Note        : If length of string is >= MEM_NameLength, it will be truncated
        at MEM_NameLength - 1 characters.  It will be NULL terminated.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
void memRename(void *pointer, char *newName)
{
    memcookie *cookie;

    memInitCheck();
    cookie = (memcookie *)pointer;
    cookie--;                                               //get pointer to cookie
    memNameSet(cookie, newName);
}
#endif //MEM_USE_NAMES

/*-----------------------------------------------------------------------------
    Name        : memClose
    Description : Closes the memory allocation module, preventing further
        allocations from having any effect.
    Inputs      : void
    Outputs     : clears the module init flag
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword memClose(void)
{
    if (!memModuleInit)
    {
        return(OKAY);
    }

    memModuleInit = FALSE;
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : memAllocFunctionA
    Description : Allocates a block of RAM
    Inputs      : length - length of requested block in bytes
                  name(optional) - name of block
                  flags - allocation control flags
    Outputs     : Newly allocated block may be cleared.
    Return      : Pointer to newly allocated block, or NULL if failure.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
void *memAllocFunctionA(sdword length, char *name, udword flags)
#else
void *memAllocFunctionA(sdword length, udword flags)
#endif
{
    ubyte *newPointer = NULL;
    memcookie *cookie, *newCookie, *nextCookie;

    memInitCheck();

    length = memRoundUp(length);                            //round length up to block size

    dbgAssert(length > 0 && length < memPoolLength);        //veify size is reasonable

#if MEM_VERBOSE_LEVEL >= 2
    dbgMessagef("\nmemAllocAttempt: allocating %d bytes for '%s'", length, name);
#endif

    cookie = memFirstFree;                                  //start at beginning of free list

    while (cookie < memLast)
    {                                                       //walk through entire list
        memCookieVerify(cookie);                            //make sure cookie is valid

        if (!bitTest(cookie->flags, MBF_AllocatedNext))     //if this block free
        {
            if (memBlocksToBytes(cookie->blocksNext) >= length)//if block bigger or equal
            {
                if (memBlocksToBytes(cookie->blocksNext) > length)//if block bigger size
                {
                    nextCookie = (memcookie *)((ubyte *)cookie +//next cookie
                            sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext));
                    memCookieVerify(nextCookie);            //make sure next cookie valid
                    dbgAssert(nextCookie->blocksPrevious == nextCookie - cookie - 1);
                    nextCookie->blocksPrevious -=           //next cookie has fewer
                        length / MEM_BlockSize + 1;         //bytes previous to it
                    newCookie = (memcookie *)((ubyte *)cookie + sizeof(memcookie) + length);
                    newCookie->flags = MBF_VerifyValue;         //init the new cookie
                    newCookie->blocksNext = cookie->blocksNext - length / MEM_BlockSize - memBytesToBlocks(sizeof(memcookie));
                    newCookie->blocksPrevious = length / MEM_BlockSize;
                    memNameSet(newCookie, MEM_HeapFree);        //set the name of new cookie

#if MEM_VERBOSE_LEVEL >= 3
                    dbgMessagef("\nmemAllocAttempt: found free block of size %d at 0x%x, made new cookie at 0x%x of size %d",
                               memBlocksToBytes(cookie->blocksNext),
                               cookie, newCookie,
                               memBlocksToBytes(newCookie->blocksNext));
#endif
                    cookie->blocksNext = length / MEM_BlockSize;//set new block size
                }
                else
                {
#if MEM_VERBOSE_LEVEL >= 3
                    dbgMessagef("\nmemAllocAttempt: found free same-size block at 0x%x", cookie);
#endif
                }
                memNameSet(cookie, name);                   //set new name
                bitSet(cookie->flags, MBF_AllocatedNext);   //say it's allocated
                newPointer = (ubyte *)(cookie + 1);         //set return pointer

                if (cookie == memFirstFree)                 //if allocating first free block
                {                                           //find next free cookie
                    while (bitTest(memFirstFree->flags, MBF_AllocatedNext))
                    {
                        memCookieVerify(memFirstFree);      //verify this cookie
                        nextCookie = (memcookie *)((ubyte *)memFirstFree +//next cookie
                                sizeof(memcookie) + memBlocksToBytes(memFirstFree->blocksNext));
                        if (nextCookie >= memLast)          //if cookie pointed past end of heap
                        {
                            goto noMoreFree;                //no more free structures
                        }
                        memFirstFree = nextCookie;
                    }
                    memNameSet(memFirstFree, MEM_NameHeapFirst);
noMoreFree:;
                }
                break;
            }
#if MEM_VERBOSE_LEVEL >= 4
            dbgMessagef("\nmemAllocAttempt: skipping free block of size %d at 0x%x - too small", memBlocksToBytes(cookie->blocksNext), cookie);
#endif
        }
        else
        {
#if MEM_VERBOSE_LEVEL >= 4
            dbgMessagef("\nmemAllocAttempt: skipping block of size %d at 0x%x - already allocated", memBlocksToBytes(cookie->blocksNext), cookie);
#endif
        }

        dbgAssert(memBlocksToBytes(cookie->blocksNext) >= 0 && memBlocksToBytes(cookie->blocksNext) < memPoolLength);
        cookie = (memcookie *)((ubyte *)cookie +            //next cookie
                            sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext));
    }

    if (newPointer == NULL)
    {
#if MEM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nmemAllocAttempt: failed to find block of length %d", length);
#endif
    }
    else
    {
#if MEM_CLEAR_MEM                                           //clear the new block
        memClearDword(newPointer, MEM_ClearSetting, length / sizeof(udword));
#endif
    }

    return(newPointer);
}

/*-----------------------------------------------------------------------------
    Name        : memAllocFunction
    Description : Allocates a block of RAM
    Inputs      : length - length of requested block in bytes
                  name(optional) - name of block
                  flags - allocation control flags
    Outputs     : Newly allocated block may be cleared.
    Return      : Pointer to newly allocated block.  This function will not
        return NULL on allocation failure, rather it will generate a fatal error.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
void *memAllocFunction(sdword length, char *name, udword flags)
#else
void *memAllocFunction(sdword length, udword flags)
#endif
{
    ubyte *newPointer = NULL;
#if MEM_ERROR_CHECKING
    if (length <= 0)
    {
        dbgFatalf(DBG_Loc, "Attempted to allocate %d bytes of '%s'", length, name);
    }
#endif
#if MEM_USE_NAMES
    newPointer = memAllocAttempt(length, name, flags);      //call the main allocation routine
#else
    newPointer = memAllocAttempt(length, 0, flags);      //call the main allocation routine
#endif

    if (newPointer == NULL)                                 //if failure
    {
#if MEM_USE_NAMES
        dbgFatalf(DBG_Loc, "memAlloc: failed to allocate %d bytes of %s", length, name);
#else
        dbgFatalf(DBG_Loc, "memAlloc: failed to allocate %d bytes", length);
#endif
    }
    return(newPointer);
}

/*-----------------------------------------------------------------------------
    Name        : memFree
    Description : Frees a block of memory
    Inputs      : pointer - pointer to memory to be freed
    Outputs     :
    Return      : OKAY (generates a fatal error on bogus pointers)
----------------------------------------------------------------------------*/
sdword memFree(void *pointer)
{
    memcookie *cookie, *nextCookie, *previousCookie;
    memcookie *stillNextCookie;//, *stillPreviousCookie;

    memInitCheck();
    dbgAssert(pointer != NULL);
    cookie = (memcookie *)pointer;
    cookie--;                                               //get pointer to cookie structure

    memCookieVerify(cookie);                                //make sure cookie is valid

#if MEM_ERROR_CHECKING
    dbgAssert(cookie->blocksNext > 0);                      //ensure non-zero size
    dbgAssert((ubyte *)cookie + sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext) <= (ubyte *)memLast);
#endif

#if MEM_VERBOSE_LEVEL >= 2
    dbgMessagef("\nmemFree: freed %d bytes of '%s' from 0x%x", memBlocksToBytes(cookie->blocksNext), cookie->name, cookie);
#endif

    bitClear(cookie->flags, MBF_AllocatedNext);             //say it's not active
    memNameSet(cookie, MEM_HeapFree);                       //clear name of cookie

    if (memFirstFree > cookie)                              //make sure first free is the actual first free
    {
        memFirstFree = cookie;
    }

#if MEM_CLEAR_MEM_ON_FREE
    memClearDword(pointer, MEM_ClearSetting, memBlocksToBytes(cookie->blocksNext) / 4);
#endif

#if MEM_DEFRAGMENT_FREE                                     //attempt to combine with adjacent free block
    nextCookie = (memcookie *)((ubyte *)cookie +            //next cookie
                            sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext));
    if (nextCookie < memLast)                               //if not end of heap
    {
        memCookieVerify(nextCookie);                        //make sure cookie is valid
        if (!bitTest(nextCookie->flags, MBF_AllocatedNext)) //if next block also free
        {
#if MEM_VERBOSE_LEVEL >= 3
            dbgMessagef("\nmemFree: combined blocks 0x%x(%d) and 0x%x(%d) into one.", cookie, memBlocksToBytes(cookie->blocksNext), nextCookie, memBlocksToBytes(nextCookie->blocksNext));
#endif
                                                            //combine sizes
            if (nextCookie < memLast)                       //update blocks previous of block past next block
            {
                stillNextCookie = nextCookie + nextCookie->blocksNext + 1;
                memCookieVerify(stillNextCookie);
                cookie->blocksNext += nextCookie->blocksNext + MEM_BlocksPerCookie;
                stillNextCookie->blocksPrevious = cookie->blocksNext;
            }

#if MEM_CLEAR_MEM                                           //clear lost cookie
            memClearDword(nextCookie, MEM_ClearSetting, sizeof(memcookie) / sizeof(udword));
#endif
        }
    }
    //attempt to combine with previous cookie if both free
    if (cookie->blocksPrevious != -1)
    {                                                       //if there is a previous block
        previousCookie = (memcookie *)((ubyte *)cookie -    //get previous block
                                sizeof(memcookie) - memBlocksToBytes(cookie->blocksPrevious));
        dbgAssert(previousCookie >= memFirst);
        memCookieVerify(previousCookie);
        if (!bitTest(previousCookie->flags, MBF_AllocatedNext)) //if previous block also free
        {
#if MEM_VERBOSE_LEVEL >= 3
            dbgMessagef("\nmemFree: combined blocks 0x%x(%d) and 0x%x(%d) into one.", previousCookie, memBlocksToBytes(previousCookie->blocksNext), cookie, memBlocksToBytes(cookie->blocksNext));
#endif
                                                            //combine sizes
            nextCookie = cookie + cookie->blocksNext + 1;
            memCookieVerify(nextCookie);
            nextCookie->blocksPrevious += cookie->blocksPrevious + 1;
            previousCookie->blocksNext = nextCookie->blocksPrevious;

#if MEM_CLEAR_MEM                                           //clear lost cookie
            memClearDword(cookie, MEM_ClearSetting, sizeof(memcookie) / sizeof(udword));
#endif
        }

    }

#endif //MEM_DEFRAGMENT_FREE

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : memDefragment
    Description : Defragments the heap by combining adjacent free blocks.
    Inputs      : void
    Outputs     : Adjacent free blocks combined.  Resets value of memFreeFirst
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword memDefragment(void)
{
    memcookie *thisCookie, *nextCookie;

    thisCookie = memFirst;                                  //start walk from very start of heap

    while (thisCookie < memLast)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        if (nextCookie >= memLast)                          //if cookie pointed past end of heap
        {
            goto noMoreFree;                                //no more free structures
        }

        while ((!bitTest(thisCookie->flags, MBF_AllocatedNext)) &&
               (!bitTest(nextCookie->flags, MBF_AllocatedNext)))//while these 2 blocks are both free
        {
#if MEM_VERBOSE_LEVEL >= 3
            dbgMessagef("\nmemDefragment: combined blocks 0x%x(%d) and 0x%x(%d) into one.", thisCookie, memBlocksToBytes(thisCookie->blocksNext), nextCookie, memBlocksToBytes(nextCookie->blocksNext));
#endif
            thisCookie->blocksNext += nextCookie->blocksNext + MEM_BlocksPerCookie;

            nextCookie = (memcookie *)((ubyte *)thisCookie +//next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
            if (nextCookie >= memLast)                      //if cookie pointed past end of heap
            {
                goto noMoreFree;                            //no more free structures
            }
            memCookieVerify(nextCookie);                    //verify next cookie
        }
        thisCookie = nextCookie;                            //next update
    }
noMoreFree:;
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : memStringDupe
    Description : Duplicate a string
    Inputs      : string - pointer to string to duplicate
    Outputs     : allocates and strcpy's the string
    Return      : newly allocated string
    Note        : Use memFree to free these strings
----------------------------------------------------------------------------*/
char *memStringDupe(char *string)
{
    char *newString;

    newString = memAlloc(strlen(string) + 1, string, 0);
    strcpy(newString, string);
    return(newString);
}

/*-----------------------------------------------------------------------------
    Name        : memFreeMemGet
    Description : Compute size of all available memory.
    Inputs      : void
    Outputs     : ..
    Return      : available memory, in bytes
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
sdword memFreeMemGet(void)
{
    memcookie *thisCookie, *nextCookie;
    sdword freeMem = 0;

    thisCookie = memFirst;                                  //start walk from very start of heap

    while (thisCookie < memLast)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        if (!bitTest(thisCookie->flags, MBF_AllocatedNext))
        {
            freeMem += memBlocksToBytes(thisCookie->blocksNext);
        }
        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        if (nextCookie >= memLast)                          //if cookie pointed past end of heap
        {
            goto noMoreFree;                                //no more free structures
        }

        thisCookie = nextCookie;                            //next update
    }
noMoreFree:;
    return freeMem;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : memUsedMemGet
    Description : Compute amount of memory used
    Inputs      : void
    Outputs     : ..
    Return      : Amount of used memory, in bytes.
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
sdword memUsedMemGet(void)
{
    memcookie *thisCookie, *nextCookie;
    sdword usedMem = 0;

    thisCookie = memFirst;                                  //start walk from very start of heap

    while (thisCookie < memLast)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        if (bitTest(thisCookie->flags, MBF_AllocatedNext))
        {
            usedMem += memBlocksToBytes(thisCookie->blocksNext);
        }
        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        if (nextCookie >= memLast)                          //if cookie pointed past end of heap
        {
            goto noMoreFree;                                //no more free structures
        }

        thisCookie = nextCookie;                            //next update
    }
noMoreFree:;
    return usedMem;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : memCookiesLengthGet
    Description : Compute amount of memory used for cookies
    Inputs      : void
    Outputs     : ..
    Return      : Amount of memory used for cookies, in bytes.
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
sdword memCookiesLengthGet(sdword *nCookies)
{
    memcookie *thisCookie, *nextCookie;
    sdword usedMem = *nCookies = 0;

    thisCookie = memFirst;                                  //start walk from very start of heap

    while (thisCookie < memLast)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        usedMem += sizeof(memcookie);

        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        (*nCookies)++;
        if (nextCookie >= memLast)                          //if cookie pointed past end of heap
        {
            goto noMoreFree;                                //no more free structures
        }

        thisCookie = nextCookie;                            //next update
    }
noMoreFree:;
    return usedMem;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : memLargestBlockGet
    Description : Returns largets block size.
    Inputs      : void
    Outputs     : ..
    Return      : Size of largest free memory block, in bytes.
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
sdword memLargestBlockGet(void)
{
    memcookie *thisCookie, *nextCookie;
    sdword largestSize = 0;

    thisCookie = memFirst;                                  //start walk from very start of heap

    while (thisCookie < memLast)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        if (!bitTest(thisCookie->flags, MBF_AllocatedNext))
        {
            largestSize = max(largestSize, memBlocksToBytes(thisCookie->blocksNext));
        }
        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        if (nextCookie >= memLast)                          //if cookie pointed past end of heap
        {
            goto noMoreFree;                                //no more free structures
        }

        thisCookie = nextCookie;                            //next update
    }
noMoreFree:;
    return largestSize;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : memAnalysisCreate
    Description : Creates and prints a detailed analysis of the current state
                    of the memory heap.
    Inputs      : void
    Outputs     : Calls the previous 3 memory computation functions, then saves
                    a detailed memory map to disk.
    Return      :
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
#define MEM_ANALYSIS_FILE_NAME      "mem.analysis"
#define MEM_MAP_FILE_NAME           "mem.map"
#define MEM_MAP_BLOCKS_PER_LINE     80
#define MEM_MAP_CHAR(c)             fputc((c), fpMap); mapColumn++; if (mapColumn >= MEM_MAP_BLOCKS_PER_LINE) {fputc('\n', fpMap);mapColumn = 0;}
void memAnalysisCreate(void)
{
    sdword largeBlock, freeMem, usedMem, cookieMem, wastedMem, nCookies;
    memcookie *thisCookie, *nextCookie;
    FILE *fpAnalysis, *fpMap;
    sdword mapColumn = 0, block;

    dbgMessagef("\nHeap length     = %d", memPoolLength);
    largeBlock = memLargestBlockGet();
    dbgMessagef("\nLargest block   = %d (%.2f%%)", largeBlock, (real32)largeBlock / (real32)memPoolLength * 100.0f);
    usedMem = memUsedMemGet();
    dbgMessagef("\nUsed memory     = %d (%.2f%%)", usedMem, (real32)usedMem / (real32)memPoolLength * 100.0f);
    freeMem = memFreeMemGet();
    dbgMessagef("\nFree memory     = %d (%.2f%%)", freeMem, (real32)freeMem / (real32)memPoolLength * 100.0f);
    cookieMem = memCookiesLengthGet(&nCookies);
    dbgMessagef("\nCookie memory   = %d (%.2f%%), %d cookies", cookieMem, (real32)cookieMem / (real32)memPoolLength * 100.0f, nCookies);
    wastedMem = memPoolLength - (usedMem + freeMem + cookieMem);
    if (wastedMem > 0)
    {
        dbgMessagef("\nLost memory     = %d (%.2f%%)", wastedMem, (real32)wastedMem / (real32)memPoolLength * 100.0f);
    }


    dbgMessagef("\nSaving detailed analysis to '%s' and map to '%s'", MEM_ANALYSIS_FILE_NAME, MEM_MAP_FILE_NAME);

    fpAnalysis = fopen(MEM_ANALYSIS_FILE_NAME, "wt");
    fpMap = fopen(MEM_MAP_FILE_NAME, "wt");
    fprintf(fpAnalysis, "Cookie Name\t\tCookie length\tPrevious block length\tCookie status\n");
    fprintf(fpMap, "Legend: * = cookie, # = allocated, . = free.  Block length = %d bytes\n", MEM_BlockSize);
    thisCookie = memFirst;                                  //start walk from very start of heap

    while (thisCookie < memLast)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        fprintf(fpAnalysis, "%20s, %10d, %10d, %s\n", thisCookie->name, memBlocksToBytes(thisCookie->blocksNext), memBlocksToBytes(thisCookie->blocksPrevious), bitTest(thisCookie->flags, MBF_AllocatedNext) ? "allocated" : "free");

        MEM_MAP_CHAR('*');
        if (bitTest(thisCookie->flags, MBF_AllocatedNext))
        {
            for (block = thisCookie->blocksNext; block > 0; block--)
            {
                MEM_MAP_CHAR('#');
            }
        }
        else
        {
            for (block = thisCookie->blocksNext; block > 0; block--)
            {
                MEM_MAP_CHAR('.');
            }
        }
        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        if (nextCookie >= memLast)                          //if cookie pointed past end of heap
        {
            break;                                          //no more free structures
        }

        thisCookie = nextCookie;                            //next update
    }
    fclose(fpAnalysis);
    fclose(fpMap);
}
#endif //MEM_ANALYSIS

