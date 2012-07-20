/*=============================================================================
    MEMORY.C:   Routines to manage memory
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "Key.h"
#include "Task.h"
#include "File.h"
#include "Memory.h"
#if MEM_VOLATILE_CLEARING
#include <time.h>
#include "CRC32.h"
#include "Randy.h"
#endif

#ifdef _MSC_VER
#define CB_DECL __cdecl
#else
#define CB_DECL
#endif


/*=============================================================================
    Data:
=============================================================================*/
//main memory pool
mempool memMainPool;

//growth pools
mempool memGrowthPool[MGH_NumberGrowthHeaps];
sdword memNumberGrowthPools = 0;
memgrowcallback memGrowthAllocate = NULL;

//flag set when module started
sdword memModuleInit = FALSE;

//memory statistics
#if MEM_STATISTICS
sdword memNumberWalks;
sdword memNumberAllocs;
sdword memAllocPool;
char memStatString[256];
bool memStatsLogging;
taskhandle memStatsTaskHandle = 0xffffffff;
memcookiename memStatsCookieNames[MS_NumberCookieNames];
#endif

#if MEM_FILE_NV
FILE *memNonVolatileFile;
#endif

memsmallheapinfo memSmallHeapInfo[] =
{
    {64, 3072},     //192k
    {256, 768},     //192k
    {384, 512},     //192k
    {512, 128},     //64k
    {1024, 32},     //64k
    {2048, 16},     //32k
    {0, 0}   //total: 736k
};
sdword memSmallBlockHeapMaxSize = 0;

#if MEM_LOG_MOSTVOLATILE
volatilestat memVolatileStat[MEM_NumberVolatileStats];
#endif

#if MEM_VOLATILE_CLEARING
udword memClearSetting = MEM_ClearSetting;
udword memFreeSetting = MEM_FreeSetting;
#else
#define memClearSetting MEM_ClearSetting
#define memFreeSetting  MEM_FreeSetting
#endif

/*=============================================================================
    Functions:
=============================================================================*/

#if MEM_STATISTICS
/*-----------------------------------------------------------------------------
    Name        : memCookieNameSort
    Description : qsort callback for sorting cookie names by number of allocs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int memCookieNameSort(const void *p0, const void *p1)
{
    return(((memcookiename *)p1)->nAllocs - ((memcookiename *)p0)->nAllocs);
}

/*-----------------------------------------------------------------------------
    Name        : memStatsTaskFunction
    Description : Task function for creting printable strings of allocation
                    frequency by cookie name.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void memStatsTaskFunction(void)
{
    static sdword index, nPrinted;

    taskYield(0);

#ifndef C_ONLY
    while (1)
#endif
    {
        taskStackSaveCond(0);
        if (memStatsLogging)
        {
            sprintf(memStatString, "Walks: %d, Allocs: %d, Factor: %.2f, Average pool: %.2f", memNumberWalks, memNumberAllocs, memNumberAllocs == 0 ? 0.0f : (real32)memNumberWalks / (real32)memNumberAllocs, memNumberAllocs == 0 ? 1.0f : (real32)memAllocPool / (real32)memNumberAllocs);
            memNumberAllocs = 0;
            memNumberWalks = 0;
            memAllocPool = 0;

            for (index = 0; index < MS_NumberCookieNames; index++)
            {
                if (memStatsCookieNames[index].name[0] != 0)
                {                                           //if there is a name in this one
                    nPrinted = sprintf(memStatsCookieNames[index].outputString, "%s - %d (%d)", memStatsCookieNames[index].name, memStatsCookieNames[index].nAllocs, memStatsCookieNames[index].nTotalSize / memStatsCookieNames[index].nAllocs);
                    dbgAssert(nPrinted < MS_OutputStringLength);
                }
                else
                {                                           //no name, blank the string
                    memStatsCookieNames[index].outputString[0] = 0;
                }
                memStatsCookieNames[index].name[0] = 0;     //now there's a printable string and no cookie name (ready to be re-used)
            }
            qsort(memStatsCookieNames, MS_NumberCookieNames, sizeof(memcookiename), memCookieNameSort);
        }
        taskStackRestoreCond();
        taskYield(0);
    }
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : memCookieNameAdd
    Description : Increment the frequency counter of a particular cookie
    Inputs      : name - name of cookie as passed to memAlloc
                  length - length of allocated cookie
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void memCookieNameAdd(char *name, sdword length)
{
    sdword index;

    for (index = 0; index < MS_NumberCookieNames; index++)
    {
        if (memStatsCookieNames[index].name[0] == 0)
        {                                                   //no name defined yet
            memStrncpy(memStatsCookieNames[index].name, name, MS_NameStringLength);  //new cookie name not yet encountered
            memStatsCookieNames[index].nAllocs = 1;
            memStatsCookieNames[index].nTotalSize = length;
            break;
        }
        if (!strcmp(name, memStatsCookieNames[index].name))
        {                                                   //if this is the string
            memStatsCookieNames[index].nAllocs++;           //increment the alloc counter
            memStatsCookieNames[index].nTotalSize += length;
            break;
        }
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : memStartup
    Description : Starts the memory allocation module.  Call before any other
                    functions.
    Inputs      : heapStart - start of heap to use.  Should be aligned on at
                    lease 4-bit boundary.
                  heapSize - size of heap to create
                  grow - heap growth callback
    Outputs     : Global heap set up and (optionally) cleared.
    Return      : OKAY if success.
----------------------------------------------------------------------------*/
sdword memStartup(void *heapStart, sdword heapSize, memgrowcallback grow)
{
#if MEM_FILE_NV
    char *memNonVolatileFileName;
#endif

    if ((sizeof(mbhcookie) != 32) || (sizeof(memcookie) != 32))
    {
        dbgFatalf(DBG_Loc, "Bad cookie size: %d, %d", sizeof(mbhcookie), sizeof(memcookie));
    }

#if MEM_ERROR_CHECKING
    if (memModuleInit == TRUE)
        dbgFatal(DBG_Loc, "Memory module started more than once.");
#endif //MEM_ERROR_CHECKING

    dbgAssert(heapStart != NULL && heapSize > MEM_BlockSize * 20);

#if MEM_VERBOSE_LEVEL >= 2
    dbgMessagef("\nMemory module init.  Heap = 0x%x, Length = %d", heapStart, heapSize);
#endif //MEM_VERBOSE_LEVEL >= 2

    memMainPool.wholePool = heapStart;
    memMainPool.pool = (ubyte *)(((udword)((ubyte *)heapStart + sizeof(memcookie) - 1)) & (~(sizeof(memcookie) - 1)));
    memMainPool.poolLength = memMainPool.heapLength = heapSize;

    memModuleInit = TRUE;

#if MEM_FILE_NV
    memNonVolatileFileName = filePathPrepend("mem.nv", FF_UserSettingsPath);

    if (!fileMakeDestinationDirectory(memNonVolatileFileName))
    {
        dbgMessage("\nError creating the path for mem.nv.");
        memNonVolatileFile = NULL;
    }
    else
    {
        memNonVolatileFile = fopen(memNonVolatileFileName, "wt");
    if (memNonVolatileFile == NULL)
    {
        dbgMessage("\nError creating log file mem.nv.");
    }
    }
#endif
    memGrowthAllocate = grow;

#if MEM_VOLATILE_CLEARING
    {
        char *userName;
        if ((userName = getenv("USERNAME")) != NULL)
        {
            memClearSetting = (udword)crc32Compute((ubyte *)userName, strlen(userName));
        }
        else
        {
            memClearSetting = (udword)ranRandom(RAN_ETG);
        }
        if ((userName = getenv("USERPROFILE")) != NULL)
        {
            memFreeSetting = (udword)crc32Compute((ubyte *)userName, strlen(userName));
        }
        else
        {
            memFreeSetting = (udword)ranRandom(RAN_ETG);
        }
    }
#endif
    return(memReset());
}

/*-----------------------------------------------------------------------------
    Name        : memPoolReset
    Description : Reset the specified pool.  Clears everything out.
    Inputs      : pool - pool to clear out
                  heapStart - the address of the actual RAM this pool manages.
                  heapSize - size of the heap, in bytes
                  bSmallHeaps - small heaps for this pool.  Does it use them?
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void memPoolReset(mempool *pool, void *heapStart, sdword heapSize, bool bSmallHeaps)
{
    ubyte *poolData;
#if MEM_SMALL_BLOCK_HEAP
    sdword index, j;
    mbheap *heap;
#endif

    pool->wholePool = heapStart;
    pool->pool = (ubyte *)(((udword)((ubyte *)heapStart + sizeof(memcookie) - 1)) & (~(sizeof(memcookie) - 1)));
    pool->poolLength = pool->heapLength = heapSize;

    memInitCheck();
#if MEM_CLEAR_MEM
    memClearDword(pool->pool, memFreeSetting, pool->poolLength / sizeof(udword));
#endif

    poolData = pool->pool;
    if (bSmallHeaps)
    {
#if MEM_SMALL_BLOCK_HEAP
        dbgAssert(sizeof(memcookie) == sizeof(mbhcookie));

        //set up the small block allocation pools
        //first pass: allocate the heap header structures
        for (index = 0; memSmallHeapInfo[index].blockSize != 0; index++)
        {                                                       //for each differing block size
            heap = (mbheap *)poolData;
            poolData += sizeof(mbheap);
#if MEM_ERROR_CHECKING
            heap->validation = MEM_HeapValidation;
#endif
            heap->blockSize = memSmallHeapInfo[index].blockSize;
            heap->nBlocks = (sdword)((real32)memSmallHeapInfo[index].nBlocks * (real32)heapSize / (real32)MEM_HeapSizeDefault);
            dbgAssert(heap->nBlocks != 0);
            listInit(&heap->allocated);
            listInit(&heap->free);
            memSmallBlockHeapMaxSize = max(memSmallBlockHeapMaxSize, heap->blockSize);
#if MEM_ERROR_CHECKING
            if (index > 0)
            {
                dbgAssert(heap->blockSize >= memSmallHeapInfo[index - 1].blockSize);
            }
#endif
#if MEM_SMALLBLOCK_STATS
            heap->nAllocationAttempts = 0;
            heap->nOverflowedAllocations = 0;
            heap->usage = 0.0f;
            heap->firstAllocTime = taskTimeElapsed;
#endif
        }
        //next pass: allocate and init the heaps and all blocks in them.
        for (index = 0; memSmallHeapInfo[index].blockSize != 0; index++)
        {                                                       //for each differing block size
            heap = (mbheap *)(pool->pool + index * sizeof(mbheap));
            heap->heapStart = (mbhcookie *)poolData;                //save start of heap
            for (j = 0; j < heap->nBlocks; j++)
            {                                                   //for all blocks we're putting in this block
                ((mbhcookie *)poolData)->flags = MBF_VerifyValue | MBF_SmallBlockHeap;//give it a validation key
                listAddNode(&heap->free, &((mbhcookie *)poolData)->link, (mbhcookie *)poolData);  //add the node to the free list
                mbhNameSet((mbhcookie *)poolData, MEM_HeapFree);    //say the cookie is free
                poolData += heap->blockSize + sizeof(mbhcookie);    //update the poolData pointer
            }
            heap->heapEnd = (mbhcookie *)poolData;
        }
#endif //MEM_SMALL_BLOCK_HEAP
    }
    //set pointer to first free memory cookie
    pool->first = pool->firstFree = (memcookie *)memRoundUp((udword)poolData);
    //get length of newly sized pool
    pool->heapLength = memRoundDown(pool->pool + pool->poolLength - (ubyte *)pool->first);
    dbgAssert(pool->heapLength > 1);

    //set pointer to end of pool
    pool->lastFree = pool->last = (memcookie *)((ubyte *)pool->first + pool->heapLength - sizeof(memcookie) * 2);

    //init first free cookie
    pool->firstFree->flags = MBF_VerifyValue;
    pool->firstFree->blocksNext = memBlocksCompute(pool->firstFree, pool->last) - 1;
    pool->firstFree->blocksPrevious = -1;
    memNameSet(pool->firstFree, MEM_NameHeap);
    pool->last->blocksPrevious = memBlocksCompute(pool->firstFree, pool->last) - 1;
    pool->last->flags = MBF_VerifyValue;

    //clear out all the cookie size pointers
    memset(pool->firstSize, 0, sizeof(pool->firstSize));
    for (index = 0; index < MSB_NumberSizes + 1; index++)
    {                                                       //check all the min size pointers
        pool->lastSize[index] = pool->last;
    }
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
#if MEM_STATISTICS || MEM_LOG_MOSTVOLATILE
    sdword index;
#endif

    memInitCheck();
    memPoolReset(&memMainPool, memMainPool.wholePool, memMainPool.poolLength, TRUE);

    //!!! reset the growth pools - onlt needed if this function is ever called at times other than startup

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
#ifndef _MACOSX_FIX_ME
        memDefragment();
#endif
    }
#endif // MEM_MODULE_TEST

#if MEM_STATISTICS
    memNumberWalks = 0;
    memNumberAllocs = 0;
    memAllocPool = 0;
    memStatsLogging = FALSE;
    memStatString[0] = 0;
    for (index = 0; index < MS_NumberCookieNames; index++)
    {
        memStatsCookieNames[index].name[0] = 0;
        memStatsCookieNames[index].nAllocs = 0;
        memStatsCookieNames[index].outputString[0] = 0;
    }
#endif

#if MEM_LOG_MOSTVOLATILE
    //clear out all the volatile stat entries
    for (index = 0; index < MEM_NumberVolatileStats; index++)
    {
        memVolatileStat[index].name[0] = 0;
        memVolatileStat[index].nLogged = 0.0f;
        memVolatileStat[index].totalTime = 0.0f;
    }
#endif

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : memClose
    Description : Closes the memory allocation module, preventing further
        allocations from having any effect.
    Inputs      : freeMem - function to call for freeing growth heaps
    Outputs     : clears the module init flag
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword memClose(memgrowthfreecallback freeMem)
{
    sdword index;

    if (!memModuleInit)
    {
        return(OKAY);
    }

#if MEM_FILE_NV
    if (memNonVolatileFile)
    {
        fclose(memNonVolatileFile);
    }
#endif

    for (index = 0; index < memNumberGrowthPools; index++)
    {
        freeMem(memGrowthPool[index].wholePool);
    }
    memModuleInit = FALSE;
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
#if defined (_MSC_VER)
    _asm
    {
        mov edi,dest
        mov ecx,nDwords
        mov eax,pattern
        rep stosd
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ (
        "rep stosl\n\t"
        :
        : "D" (dest), "c" (nDwords), "a" (pattern) );
#else
	udword *d = ( udword *)dest;
    while (nDwords--)
        *d++ = pattern;
#endif
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

    memStrncpy(cookie->name, name, MEM_NameLength);
    cookie->name[MEM_NameLength - 1] = 0;

#if MEM_VERBOSE_LEVEL >= 3
    dbgMessagef("\nmemNameSet: Cookie at 0x%x named to '%s'", cookie, cookie->name);
#endif
    return(OKAY);
}
#if MEM_SMALL_BLOCK_HEAP
sdword mbhNameSetFunction(mbhcookie *cookie, char *name)
{
    memInitCheck();
    memCookieVerify(cookie);

    memStrncpy(cookie->name, name, MBH_NameLength);
    cookie->name[MBH_NameLength - 1] = 0;

#if MEM_VERBOSE_LEVEL >= 3
    dbgMessagef("\nmemNameSet: SBH cookie at 0x%x named to '%s'", cookie, cookie->name);
#endif
    return(OKAY);
}
#endif //MEM_SMALL_BLOCK_HEAP
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
    memStrncpy(cookie->name, name, MEM_NameLength);
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
    dbgAssert(!bitTest(cookie->flags, MBF_SmallBlockHeap)); //can't rename blocks from small block heap
    memNameSet(cookie, newName);
    bitSet(cookie->flags, MBF_String);                      //prevent this block from going through volatility statistics
}
#endif //MEM_USE_NAMES

#if MEM_ANAL_CHECKING
/*-----------------------------------------------------------------------------
    Name        : memPoolAnalCheck
    Description : Anally check the state of a memory pool, verifying nothing is amiss
    Inputs      : pool - what pool to verify
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void memPoolAnalCheck(mempool *pool)
{
    sdword index;

    for (index = 0; index < MSB_NumberSizes + 1; index++)
    {
        if (pool->firstSize[index] != NULL)
        {
            memCookieVerify(pool->firstSize[index]);
        }
        memCookieVerify(pool->lastSize[index]);
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : memSBHGrowBy
    Description : Grow a small block heap by a specified amount
    Inputs      : heap - what heap to grow
                  nNewCookies - how much to grow it by
                  blockSize - size to grow by
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void memSBHGrowBy(mbheap *heap, sdword nNewCookies)
{
    sdword index;
    ubyte *poolData;

    poolData = memAlloc(nNewCookies * (heap->blockSize + sizeof(mbhcookie)), "SBHGrow", NonVolatile);
    for (index = 0; index < nNewCookies; index++)
    {
        ((mbhcookie *)poolData)->flags = MBF_VerifyValue | MBF_SmallBlockHeap;//give it a validation key
        listAddNode(&heap->free, &((mbhcookie *)poolData)->link, (mbhcookie *)poolData);  //add the node to the free list
        mbhNameSet((mbhcookie *)poolData, MEM_HeapFree);    //say the cookie is free
        poolData += heap->blockSize + sizeof(mbhcookie);    //update the poolData pointer
    }
    heap->nBlocks += nNewCookies;
}

/*-----------------------------------------------------------------------------
    Name        : memAllocFunctionSBH
    Description : Attempts to allocate a block of RAM from a small block heap.
    Inputs      : length - length of requested block in bytes
                  name(optional) - name of block
                  flags - allocation control flags
    Outputs     : Newly allocated block may be cleared.
    Return      : Pointer to newly allocated block, or NULL if failure.
----------------------------------------------------------------------------*/
#if MEM_SMALL_BLOCK_HEAP
#if MEM_USE_NAMES
void *memAllocFunctionSBH(sdword length, char *name, udword flags)
#else
void *memAllocFunctionSBH(sdword length, udword flags)
#endif
{
    mbheap *heap;
    memsmallheapinfo *info;
    mbhcookie *cookie;
    ubyte *newPointer = NULL;
    sdword nNewCookies;
    //make sure it can fit in a small block
    if (length > memSmallBlockHeapMaxSize)
    {                                                       //if block to be allocated is too big
#if MEM_VERBOSE_LEVEL >= 2
        dbgMessagef("\nmemAllocFunctionSBH: cannot allocate block of size %d (> %d)", length, memSmallBlockHeapMaxSize);
#endif
        return(NULL);
    }

    for (heap = (mbheap *)memMainPool.pool, info = memSmallHeapInfo; info->blockSize != 0; heap++, info++)
    {                                                       //search through all small block heap structures
#if MEM_ERROR_CHECKING
        if (heap->validation != MEM_HeapValidation)
        {
            dbgFatalf(DBG_Loc, "Heap at 0x%x has invalid validation key of 0x%x", heap, heap->validation);
        }
        dbgAssert(heap->allocated.num + heap->free.num == heap->nBlocks);
        dbgAssert(heap->blockSize == info->blockSize);
        //dbgAssert(heap->nBlocks == info->nBlocks);
#endif
        if (info->blockSize >= length)
        {                                                   //if this heap has large enough blocks
#if MEM_SMALLBLOCK_STATS
            heap->nAllocationAttempts++;                    //one more allocation on this heap
            heap->usage += (real32)heap->allocated.num;
#endif
            if (heap->free.num == 0)
            {                                               //if the heap is empty
                nNewCookies = heap->nBlocks * MEM_GrowFactor;
#if MEM_VERBOSE_LEVEL >= 1
                dbgMessagef("\nmemAllocFunctionSBH: heap of blocks size %d all used up.  All %d blocks.\nAllocating another %d", heap->blockSize, heap->nBlocks, nNewCookies);
#endif
#if MEM_SMALLBLOCK_STATS
                heap->nOverflowedAllocations++;
#endif
                memSBHGrowBy(heap, nNewCookies);
            }
            dbgAssert(heap->free.num > 0);
            cookie = listGetStructOfNode(heap->free.tail);
            mbhCookieVerify(cookie);                        //validate this cookie

            listRemoveNode(&cookie->link);                  //remove from free list
            listAddNode(&heap->allocated, &cookie->link, cookie);//add to allocated list
            newPointer = (ubyte *)(cookie + 1);
#if MEM_CLEAR_MEM                                           //clear the new block to allocated pattern
            memClearDword(newPointer, memClearSetting, heap->blockSize / sizeof(udword));
#endif
            bitSet(cookie->flags, MBF_AllocatedNext);       //flag block as allocated
            cookie->length = length;                        //record length of cookie as allocated
#if MEM_LOG_MOSTVOLATILE
            cookie->timeAllocated = taskTimeElapsed;
#endif
#if MEM_STATISTICS
            if (keyIsStuck(MEM_StatsKey) && keyIsHit(CONTROLKEY))
            {
                keyClearSticky(MEM_StatsKey);
                memStatsLogging ^= TRUE;
            }
            memNumberAllocs++;                              //another walk, another allocation
            memNumberWalks++;
            memCookieNameAdd(name, length);
#endif
            mbhNameSet(cookie, name);
            break;                                          //correct block size found,break out of search loop
        }
    }
#if MEM_ANALYSIS_KEY
    if (keyIsHit(SHIFTKEY) && keyIsStuck(MEM_ANALYSIS_KEY))
    {
        keyClearSticky(MEM_ANALYSIS_KEY);
        memAnalysisCreate();
    }
#endif
    return(newPointer);                                     //allocation failed.  Too bad
}
#endif //MEM_SMALL_BLOCK_HEAP

/*-----------------------------------------------------------------------------
    Name        : memAllocFunctionANV
    Description : Allocates a block of RAM
    Inputs      : length - length of requested block in bytes, rounded to a block length
                  name(optional) - name of block
                  flags - allocation control flags
                  pool - pool to allocate from
    Outputs     : Newly allocated block may be cleared.
    Return      : Pointer to newly allocated block, or NULL if failure.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
void *memAllocFunctionANV(sdword length, char *name, udword flags, mempool *pool)
#else
void *memAllocFunctionANV(sdword length, udword flags, mempool *pool)
#endif
{
    ubyte *newPointer = NULL;
    memcookie *cookie, *newCookie, *nextCookie;

    memCookieVerify(pool->lastFree);

    if (length <= MSB_SizeMax && mslSizeBytes(length) < pool->lastFree)
    {
        cookie = mslSizeBytes(length) - mslSizeBytes(length)->blocksPrevious - 1;
        memCookieVerify(cookie);                            //make sure cookie is valid
    }
    else
    {
        cookie = pool->lastFree - pool->lastFree->blocksPrevious - 1;
        memCookieVerify(cookie);                            //make sure cookie is valid
    }

    while (cookie >= pool->firstFree)
    {
#if MEM_STATISTICS
        memNumberWalks++;
#endif
        memCookieVerify(cookie);                            //make sure cookie is valid

        if (!bitTest(cookie->flags, MBF_AllocatedNext))     //if this block free
        {
            if (memBlocksToBytes(cookie->blocksNext) >= length)//if block bigger or equal to size needed
            {
                if (memBlocksToBytes(cookie->blocksNext) > length)//if block bigger size
                {
                    nextCookie = (memcookie *)((ubyte *)cookie +//next cookie
                            sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext));
                    memCookieVerify(nextCookie);            //make sure next cookie valid
                    dbgAssert(nextCookie->blocksPrevious == nextCookie - cookie - 1);
                    nextCookie->blocksPrevious =            //next cookie has fewer
                        length / MEM_BlockSize;             //bytes previous to it
                    newCookie = (memcookie *)((ubyte *)nextCookie - sizeof(memcookie) - length);
                    newCookie->blocksNext = length / MEM_BlockSize;
                    newCookie->blocksPrevious = cookie->blocksNext - length / MEM_BlockSize - memBytesToBlocks(sizeof(memcookie));
                    cookie->blocksNext -= length / MEM_BlockSize + 1;
/*
                    if (newCookie->blocksNext * MEM_BlockSize <= MSB_SizeMax &&
                        mslSizeBlocks(newCookie->blocksNext) - mslSizeBlocks(newCookie->blocksNext)->blocksPrevious - 1 > newCookie)
                    {   //set the min size block to applicable value
                        mslSizeBlocks(newCookie->blocksNext) = newCookie + newCookie->blocksNext + 1;
                    }
*/
#if MEM_VERBOSE_LEVEL >= 3
                    dbgMessagef("\nmemAllocFunctionANV: found free block of size %d at 0x%x, made new cookie at 0x%x of size %d",
                               memBlocksToBytes(cookie->blocksNext),
                               cookie, newCookie,
                               memBlocksToBytes(newCookie->blocksNext));
#endif
                }
                else
                {                                           //else block is just the right length
#if MEM_VERBOSE_LEVEL >= 3
                    dbgMessagef("\nmemAllocFunctionANV: found free same-size block at 0x%x", cookie);
#endif
                    newCookie = cookie;
                }
                newCookie->flags = MBF_VerifyValue | MBF_AllocatedNext | (flags & MBF_ParameterMask);
                memNameSet(newCookie, name);
#if MEM_DETECT_VOLATILE
                newCookie->timeAllocated = taskTimeElapsed; //remember when cookie was allocated
#endif
                newPointer = (ubyte *)(newCookie + 1);      //set return pointer
                if (length <= MSB_SizeMax)
                {                                           //if this was a small block allocation
                    //while this is not the first free block in the specified
                    //size, it'll sure reduce the number of walks a lot.
                    mslSizeBytes(length) = cookie + cookie->blocksNext + 1;
                }
                if ((memcookie *)((ubyte *)newCookie + length + sizeof(memcookie)) == pool->lastFree)
                {                                           //if just allocated the last free cookie
                    nextCookie = newCookie;
                    do
                    {                                       //find new last free cookie
                        pool->lastFree = nextCookie;
                        nextCookie = (memcookie *)((ubyte *)pool->lastFree - sizeof(memcookie) - memBlocksToBytes(pool->lastFree->blocksPrevious));
                        memCookieVerify(nextCookie);
                        if (nextCookie <= pool->firstFree)
                        {
                            break;
                        }
                    }
                    while (bitTest(nextCookie->flags, MBF_AllocatedNext));
//                    memNameSet(pool->lastFree, MEM_NameHeapLast);
                }
                break;                                          //found free cookie, stop searching
            }
#if MEM_VERBOSE_LEVEL >= 4
            dbgMessagef("\nmemAllocFunctionANV: skipping free block of size %d at 0x%x - too small", memBlocksToBytes(cookie->blocksNext), cookie);
#endif
        }
        else
        {
#if MEM_VERBOSE_LEVEL >= 4
            dbgMessagef("\nmemAllocFunctionANV: skipping block of size %d at 0x%x - already allocated", memBlocksToBytes(cookie->blocksNext), cookie);
#endif
        }
        if (cookie->blocksPrevious == -1)
        {                                                   //if reached bottom of heap
            break;
        }
        dbgAssert(memBlocksToBytes(cookie->blocksPrevious) >= 0 && memBlocksToBytes(cookie->blocksPrevious) < pool->heapLength);
        cookie = (memcookie *)((ubyte *)cookie -            //next cookie
                            sizeof(memcookie) - memBlocksToBytes(cookie->blocksPrevious));
    }

    if (newPointer == NULL)
    {                                                       //if allocation failed
        if (length > MSB_SizeMax)
        {                                                   //if we were searching from the beginning
#if MEM_VERBOSE_LEVEL >= 1
            dbgMessagef("\nmemAllocFunctionANV: failed to find block of length %d", length);
#endif
        }
        else if (mslSizeBytes(length) != pool->lastFree && (mslSizeBytes(length) != pool->lastFree))
        {                                                   //set this block to last free block and try again
#if MEM_VERBOSE_LEVEL >= 1
            dbgMessagef("\nmemAllocFunctionANV: failed to find MSL block of length %d", length);
#endif
            //mslSizeBytes(length) = (memcookie *)((ubyte *)pool->lastFree - sizeof(memcookie));
            mslSizeBytes(length) = pool->lastFree;
#if MEM_USE_NAMES
            newPointer = memAllocFunctionANV(length, name, flags, pool); //try to allocate again
#else
            newPointer = memAllocFunctionANV(length, flags, pool);
#endif
            return(newPointer);
        }
    }
    else
    {
#if MEM_CLEAR_MEM                                           //clear the new block
        memClearDword(newPointer, memClearSetting, length / sizeof(udword));
#endif
    }
#if MEM_STATISTICS
    if (keyIsStuck(MEM_StatsKey) && keyIsHit(CONTROLKEY))
    {
        keyClearSticky(MEM_StatsKey);
        memStatsLogging ^= TRUE;
    }
    memNumberAllocs++;
    memCookieNameAdd(name, length);
#endif
#if MEM_ANALYSIS_KEY
    if (keyIsHit(SHIFTKEY) && keyIsStuck(MEM_ANALYSIS_KEY))
    {
        keyClearSticky(MEM_ANALYSIS_KEY);
        memAnalysisCreate();
    }
#endif
#if MEM_ANAL_CHECKING
    memPoolAnalCheck(pool);
#endif

    return(newPointer);
}

/*-----------------------------------------------------------------------------
    Name        : memAllocFunctionA
    Description : Allocates a block of volatile RAM, that is a block to be freed soon.
    Inputs      : length - length of requested block in bytes
                  name(optional) - name of block
                  flags - allocation control flags
                  pool - pool to allocate from
    Outputs     : Newly allocated block may be cleared.
    Return      : Pointer to newly allocated block, or NULL if failure.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
void *memAllocFunctionA(sdword length, char *name, udword flags, mempool *pool)
#else
void *memAllocFunctionA(sdword length, udword flags, mempool *pool)
#endif
{
    ubyte *newPointer = NULL;
    memcookie *cookie, *newCookie, *nextCookie;

    memInitCheck();

    dbgAssert(length > 0/* && length < pool->heapLength*/);        //verify size is reasonable

#if MEM_VERBOSE_LEVEL >= 2
    dbgMessagef("\nmemAllocFunctionA: allocating %d bytes for '%s'", length, name);
#endif

    //small block heap allocation scheme, if applicable
#if MEM_SMALL_BLOCK_HEAP
    if (bitTest(flags, MBF_SmallBlockHeap) && pool == &memMainPool)
    {
#if MEM_USE_NAMES
        cookie = memAllocFunctionSBH(length, name, flags);
#else
        cookie = memAllocFunctionSBH(length, flags);
#endif
        if (cookie != NULL)
        {                                                   //if small block allocation worked
            return(cookie);
        }
        //else fall through to the standard allocation scheme
        bitClear(flags, MBF_SmallBlockHeap);
    }
#endif

    length = memRoundUp(length);                            //round length up to block size

    //nonvolatile allocation scheme, if applicable
    if (bitTest(flags, MBF_NonVolatile))
    {
#if MEM_USE_NAMES
        return(memAllocFunctionANV(length, name, flags, pool));
#else
        return(memAllocFunctionANV(length, flags, pool));
#endif
    }

    //regular volatile allocation, choose small or large blocks
    if (length > MSB_SizeMax)
    {
        cookie = pool->firstFree;                              //start at beginning of free list for large allocations
    }
    else
    {
        cookie = max(pool->firstFree, msbSizeBytes(length));   //start at first free block of a given size
    }

    while (cookie < pool->lastFree)
    {                                                       //walk through entire list
#if MEM_STATISTICS
        memNumberWalks++;
#endif
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
                    newCookie->flags = MBF_VerifyValue;     //init the new cookie
                    newCookie->blocksNext = cookie->blocksNext - length / MEM_BlockSize - memBytesToBlocks(sizeof(memcookie));
                    newCookie->blocksPrevious = length / MEM_BlockSize;
                    memNameSet(newCookie, MEM_HeapFree);        //set the name of new cookie
                    //set min size pointer for newly created block if applicable
/*
                    if (newCookie->blocksNext * MEM_BlockSize <= MSB_SizeMax &&
                        msbSizeBlocks(newCookie->blocksNext) < newCookie)
                    {   //set the min size block to applicable value
                        msbSizeBlocks(newCookie->blocksNext) = newCookie;
                    }
*/
#if MEM_VERBOSE_LEVEL >= 3
                    dbgMessagef("\nmemAllocFunctionA: found free block of size %d at 0x%x, made new cookie at 0x%x of size %d",
                               memBlocksToBytes(cookie->blocksNext),
                               cookie, newCookie,
                               memBlocksToBytes(newCookie->blocksNext));
#endif
                    cookie->blocksNext = length / MEM_BlockSize;//set new block size
                }
                else
                {
#if MEM_VERBOSE_LEVEL >= 3
                    dbgMessagef("\nmemAllocFunctionA: found free same-size block at 0x%x", cookie);
#endif
                }
                memNameSet(cookie, name);                   //set new name
                cookie->flags = MBF_VerifyValue | MBF_AllocatedNext | (flags & MBF_ParameterMask);
#if MEM_DETECT_VOLATILE
                cookie->timeAllocated = taskTimeElapsed;    //remember when cookie was allocated
#endif
                newPointer = (ubyte *)(cookie + 1);         //set return pointer

                if (length <= MSB_SizeMax)
                {                                           //if this was a small block allocation
                    //while this is not the first free block in the specified
                    //size, it'll sure reduce the number of walks a lot.
                    msbSizeBytes(length) = cookie;
                }
                if (cookie == pool->firstFree)                 //if allocating first free block
                {                                           //find next free cookie
                    while (bitTest(pool->firstFree->flags, MBF_AllocatedNext))
                    {
                        memCookieVerify(pool->firstFree);      //verify this cookie
                        nextCookie = (memcookie *)((ubyte *)pool->firstFree +//next cookie
                                sizeof(memcookie) + memBlocksToBytes(pool->firstFree->blocksNext));
                        if (nextCookie >= pool->lastFree)      //if cookie pointed past end of heap
                        {
                            goto noMoreFree;                //no more free structures
                        }
                        pool->firstFree = nextCookie;
                    }
                    memNameSet(pool->firstFree, MEM_NameHeapFirst);
noMoreFree:;
                }
                break;
            }
#if MEM_VERBOSE_LEVEL >= 4
            dbgMessagef("\nmemAllocFunctionA: skipping free block of size %d at 0x%x - too small", memBlocksToBytes(cookie->blocksNext), cookie);
#endif
        }
        else
        {
#if MEM_VERBOSE_LEVEL >= 4
            dbgMessagef("\nmemAllocFunctionA: skipping block of size %d at 0x%x - already allocated", memBlocksToBytes(cookie->blocksNext), cookie);
#endif
        }

        dbgAssert(memBlocksToBytes(cookie->blocksNext) >= 0 && memBlocksToBytes(cookie->blocksNext) < pool->heapLength);
        cookie = (memcookie *)((ubyte *)cookie +            //next cookie
                            sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext));
    }

    if (newPointer == NULL)
    {                                                       //if allocation failed
        if (length > MSB_SizeMax)
        {                                                   //if we were searching from the beginning
#if MEM_VERBOSE_LEVEL >= 1
            dbgMessagef("\nmemAllocFunctionA: failed to find block of length %d", length);
#endif
        }
        else if (msbSizeBytes(length) != pool->firstFree && (msbSizeBytes(length) != pool->firstFree))
        {                                                   //failure may be due to small-block fragmentation
#if MEM_VERBOSE_LEVEL >= 1
            dbgMessagef("\nmemAllocFunctionA: failed to find MSB block of length %d", length);
#endif
            msbSizeBytes(length) = pool->firstFree;            //set this block to first free block
#if MEM_USE_NAMES
            newPointer = memAllocFunctionA(length, name, flags, pool); //try to allocate again
#else
            newPointer = memAllocFunctionA(length, flags, pool);
#endif
            return(newPointer);
        }
    }
    else
    {
#if MEM_CLEAR_MEM                                           //clear the new block
        memClearDword(newPointer, memClearSetting, length / sizeof(udword));
#endif
    }
#if MEM_STATISTICS
    if (keyIsStuck(MEM_StatsKey) && keyIsHit(CONTROLKEY))
    {
        keyClearSticky(MEM_StatsKey);
        memStatsLogging ^= TRUE;
    }
    memNumberAllocs++;
    memCookieNameAdd(name, length);
#endif
#if MEM_ANALYSIS_KEY
    if (keyIsHit(SHIFTKEY) && keyIsStuck(MEM_ANALYSIS_KEY))
    {
        keyClearSticky(MEM_ANALYSIS_KEY);
        memAnalysisCreate();
    }
#endif
#if MEM_ANAL_CHECKING
    memPoolAnalCheck(pool);
#endif

    return(newPointer);
}

/*-----------------------------------------------------------------------------
    Name        : memNewGrowthPoolAlloc
    Description : Allocate and initialize a new growth heap pool.
    Inputs      : length - length of block we're trying to allocate in the new heap
    Outputs     :
    Return      : newly allocated pool.  Will create fatal error if it cannot
                    allocate new pool.
----------------------------------------------------------------------------*/
mempool *memNewGrowthPoolAlloc(sdword length)
{
    void *newPool;

    //this error checking shall be present in release builds
    if (memNumberGrowthPools >= MGH_NumberGrowthHeaps)
    {
        dbgFatalf(DBG_Loc, "Exceeded %d growth heaps, each of length at least %d.", MGH_NumberGrowthHeaps, MGH_MinGrowthSize);
    }
    length = max(length, MGH_MinGrowthSize);                //larger of growth size and cookie
    length += sizeof(memcookie) * 4;                        //a cookie high and low: that should be a heap large enough for everyone
#if MEM_VERBOSE_LEVEL
    dbgMessagef("\nAllocating growth heap %d of size %d", memNumberGrowthPools, length);
#endif
    newPool = memGrowthAllocate(length);
    if (newPool == NULL)
    {
        dbgFatalf(DBG_Loc, "Could not allocate growth heap %d with a size of %d", memNumberGrowthPools, length);
    }
    memPoolReset(&memGrowthPool[memNumberGrowthPools], newPool, length, FALSE);
    memNumberGrowthPools++;
    return(&memGrowthPool[memNumberGrowthPools - 1]);
}

/*-----------------------------------------------------------------------------
    Name        : memAllocAttemptFunction
    Description : Attempt to allocate memory, returning NULL if there's not
                    for sale.
    Inputs      : length - length of requested block in bytes
                  name(optional) - name of block
                  flags - allocation control flags
    Outputs     : Newly allocated block may be cleared.
    Return      : New pointer or NULL on failure.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
void *memAllocAttemptFunction(sdword length, char *name, udword flags)
#else
void *memAllocAttemptFunction(sdword length, udword flags)
#endif
{
    sdword iPool;
    mempool *pool;
    ubyte *newPointer = NULL;

#if MEM_ERROR_CHECKING
    if (length <= 0)
    {
        dbgFatalf(DBG_Loc, "Attempted to allocate %d bytes of '%s'", length, name);
    }
#endif
    if (!bitTest(flags, MBF_ExtendedPoolOnly))
    {
#if MEM_USE_NAMES
        newPointer = memAllocFunctionA(length, name, flags, &memMainPool);//call the main allocation routine
#else
        newPointer = memAllocFunctionA(length, flags, &memMainPool); //call the main allocation routine
#endif

#if MEM_STATISTICS
        memAllocPool++;
#endif
    }

    if (newPointer == NULL)
    {                                                       //no RAM left in the main heap
        bitClear(flags, MBF_SmallBlockHeap);                //no SBH's in growth heaps
        for (iPool = 0, pool = memGrowthPool; iPool < memNumberGrowthPools; iPool++, pool++)
        {                                                   //try to alloc from currently active growth pools
#if MEM_STATISTICS
            memNumberAllocs--;                              //the last allocation failed
            memAllocPool++;
#endif
#if MEM_USE_NAMES
            newPointer = memAllocFunctionA(length, name, flags, pool);
#else
            newPointer = memAllocFunctionA(length, flags, pool); //call the main allocation routine for this pool
#endif
            if (newPointer != NULL)
            {                                               //if allocated from growth pool
                return(newPointer);
            }
        }
        //failed to allocate from existing growth pools
        pool = memNewGrowthPoolAlloc(length);               //allocate a new growth pool
#if MEM_USE_NAMES
        newPointer = memAllocFunctionA(length, name, flags, pool);
#else
        newPointer = memAllocFunctionA(length, flags, pool); //call the main allocation routine
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
    sdword iPool;
    mempool *pool;
#if MEM_ERROR_CHECKING
    if (length <= 0)
    {
        dbgFatalf(DBG_Loc, "Attempted to allocate %d bytes of '%s'", length, name);
    }
#endif

    if (!bitTest(flags, MBF_ExtendedPoolOnly))
    {
#if MEM_USE_NAMES
        newPointer = memAllocFunctionA(length, name, flags, &memMainPool);//call the main allocation routine
#else
        newPointer = memAllocFunctionA(length, flags, &memMainPool); //call the main allocation routine
#endif

#if MEM_STATISTICS
        memAllocPool++;
#endif
    }

    if (newPointer == NULL)                                 //if failure
    {
        bitClear(flags, MBF_SmallBlockHeap);                //no SBH's in growth heaps
        for (iPool = 0, pool = memGrowthPool + memNumberGrowthPools - 1; iPool < memNumberGrowthPools; iPool++, pool--)
        {                                                   //try to alloc from currently active growth pools
#if MEM_STATISTICS
            memNumberAllocs--;                              //the last allocation failed
            memAllocPool++;
#endif
#if MEM_USE_NAMES
            newPointer = memAllocFunctionA(length, name, flags, pool);
#else
            newPointer = memAllocFunctionA(length, flags, pool); //call the main allocation routine for this pool
#endif
            if (newPointer != NULL)
            {                                               //if allocated from growth pool
                return(newPointer);
            }
        }
        //failed to allocate from existing growth pools
        pool = memNewGrowthPoolAlloc(length);               //allocate a new growth pool
#if MEM_USE_NAMES
        newPointer = memAllocFunctionA(length, name, flags, pool);
#else
        newPointer = memAllocFunctionA(length, flags, pool); //call the main allocation routine
#endif
        if (newPointer == NULL)
        {
#if MEM_ANALYSIS_AUTOCREATE
            memAnalysisCreate();
#if MEM_USE_NAMES
            dbgFatalf(DBG_Loc, "memAlloc: failed to allocate %d bytes of %s.  Analysis saved.", length, name);
#else
            dbgFatalf(DBG_Loc, "memAlloc: failed to allocate %d bytes.  Analysis saved.", length);
#endif //MEM_USE_NAMES
#else //MEM_ANALYSIS_AUTOCREATE
#if MEM_USE_NAMES
            dbgFatalf(DBG_Loc, "memAlloc: failed to allocate %d bytes of %s.", length, name);
#else
            dbgFatalf(DBG_Loc, "memAlloc: failed to allocate %d bytes.", length);
#endif //MEM_USE_NAMES
#endif //MEM_ANALYSIS_AUTOCREATE
        }
    }
    return(newPointer);
}

/*-----------------------------------------------------------------------------
    Name        : memVolatilityLogFuntion
    Description : Log the volatility of a cookie being freed
    Inputs      : cookie - cookie to gather stats on
    Outputs     : saves the amount of time this beatch was allocated to memVolatileStat
    Return      : void
----------------------------------------------------------------------------*/
#if MEM_LOG_MOSTVOLATILE
void memVolatilityLogFuntion(memcookie *cookie)
{
    sdword index;

    if (bitTest(cookie->flags, MBF_String))
    {
        return;                                             //don't log strings
    }
    for (index = 0; index < MEM_NumberVolatileStats; index++)
    {                                                       //search through stats array
        if (memVolatileStat[index].nLogged == 0.0f)
        {                                                   //if unused slot
            memcpy(memVolatileStat[index].name, cookie->name, MEM_NameLength);
            memVolatileStat[index].nLogged = 1.0f;
            memVolatileStat[index].totalTime = taskTimeElapsed - cookie->timeAllocated;
            return;
        }
        else
        {                                                   //else something in slot
            if (!strcmp(memVolatileStat[index].name, cookie->name))
            {                                               //if this is the stats slot for this cookie
                memVolatileStat[index].nLogged += 1.0f;     //update already started stat
                memVolatileStat[index].totalTime += taskTimeElapsed - cookie->timeAllocated;
                return;
            }
        }
    }
    dbgMessagef("\nCannot log volatility of '%s': stats array length %d full", cookie->name, MEM_NumberVolatileStats);
}
int CB_DECL memVolatilitySort(const void *p0, const void *p1)
{
    real32 diff;

    diff = ((volatilestat *)p0)->volatility - ((volatilestat *)p1)->volatility;
    if (diff < 0)
    {
        return(-1);
    }
    else if (diff > 0)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : memFreeSBH
    Description : Free a block of memory allocated from a small block heap.
    Inputs      : cookie - cookie we are freeing
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#if MEM_SMALL_BLOCK_HEAP
void memFreeSBH(mbhcookie *cookie)
{
    mbheap *heap;

    dbgAssert(bitTest(cookie->flags, MBF_AllocatedNext));
    heap = (mbheap*)(((ubyte *)cookie->link.belongto) - MBH_AllocatedLinkOffset);
#if MEM_ERROR_CHECKING
    dbgAssert(heap->validation == MEM_HeapValidation);
#endif
    listRemoveNode(&cookie->link);
    listAddNode(&heap->free, &cookie->link, cookie);
    bitClear(cookie->flags, MBF_AllocatedNext);             //flag the cookie as free
    mbhNameSet(cookie, MEM_HeapFree);                       //name the cookie is free
    dbgAssert(heap->free.num + heap->allocated.num == heap->nBlocks);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : memFreeNV
    Description : Free a non-volatile memory cookie (only to be called from memFree)
    Inputs      : cookie - cookie to free
                  pool - pool to free from
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void memFreeNV(memcookie *cookie, memcookie *nextCookie, mempool *pool)
{
    memcookie *previousCookie;
    memcookie *stillNextCookie;
    udword length, index;

    if (nextCookie < pool->last)                               //if not end of heap
    {
        memCookieVerify(nextCookie);                        //make sure cookie is valid
        if (!bitTest(nextCookie->flags, MBF_AllocatedNext)) //if next block also free
        {
#if MEM_VERBOSE_LEVEL >= 3
            dbgMessagef("\nmemFree: combined blocks 0x%x(%d) and 0x%x(%d) into one.", cookie, memBlocksToBytes(cookie->blocksNext), nextCookie, memBlocksToBytes(nextCookie->blocksNext));
#endif
                                                            //combine sizes
            stillNextCookie = nextCookie + nextCookie->blocksNext + 1;
            memCookieVerify(stillNextCookie);
            cookie->blocksNext += nextCookie->blocksNext + MEM_BlocksPerCookie;
            stillNextCookie->blocksPrevious = cookie->blocksNext;
            //make sure that no min size pointers are referencing lost cookie
            for (index = 0; index < MSB_NumberSizes + 1; index++)
            {
                if (pool->firstSize[index] == nextCookie)
                {
                    pool->firstSize[index] = cookie;
                }
                if (pool->lastSize[index] == nextCookie)
                {
                    pool->lastSize[index] = cookie + cookie->blocksNext + 1;
                    memCookieVerify(pool->lastSize[index]);
                }
            }

#if MEM_CLEAR_MEM                                           //clear lost cookie
            memClearDword(nextCookie, memFreeSetting, sizeof(memcookie) / sizeof(udword));
#endif
        }
    }
    //attempt to combine with previous cookie if both free
    if (cookie->blocksPrevious != -1)
    {                                                       //if there is a previous block
        previousCookie = (memcookie *)((ubyte *)cookie -    //get previous block
                                sizeof(memcookie) - memBlocksToBytes(cookie->blocksPrevious));
        dbgAssert(previousCookie >= pool->first);
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
            memClearDword(cookie, memFreeSetting, sizeof(memcookie) / sizeof(udword));
#endif
            //make sure that no pointers are referencing lost cookie
            for (index = 0; index < MSB_NumberSizes + 1; index++)
            {                                               //check all the min size pointers
                if (pool->firstSize[index] == cookie)
                {
                    pool->firstSize[index] = previousCookie;
                }
                if (pool->lastSize[index] == cookie)
                {
                    pool->lastSize[index] = previousCookie + previousCookie->blocksNext + 1;
                    memCookieVerify(pool->lastSize[index]);
                }
            }
            if (pool->lastFree == cookie)
            {                                               //check the last free pointer
                pool->lastFree = nextCookie;
            }
            cookie = previousCookie;                        //make sure that the later resizing will work
        }
        else
        {                                                   //else prev cookie in use
            length = cookie->blocksNext * MEM_BlockSize;
            if (length <= MSB_SizeMax && mslSizeBytes(length) < cookie + cookie->blocksNext + 1)
            {                                               //set the new size block max
                mslSizeBytes(length) = cookie + cookie->blocksNext + 1;
            }
        }
    }
    if (pool->lastFree <= cookie)
    {
        pool->lastFree = (memcookie *)((ubyte *)cookie + sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext));
        dbgAssert(pool->lastFree > pool->firstFree && pool->lastFree <= pool->last);
        memCookieVerify(pool->lastFree);
    }
#if MEM_ANAL_CHECKING
    memPoolAnalCheck(pool);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : memFree
    Description : Frees a block of memory
    Inputs      : pointer - pointer to memory to be freed
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void memFree(void *pointer)
{
    memcookie *cookie, *nextCookie, *previousCookie;
    memcookie *stillNextCookie;
    udword length, index;
    mempool *pool = &memMainPool;                            //what pool does this cookie belong to?

    memInitCheck();
    dbgAssert(pointer != NULL);
    cookie = (memcookie *)pointer;
    cookie--;                                               //get pointer to cookie structure

    memCookieVerify(cookie);                                //make sure cookie is valid

    if (pointer < (void *)memMainPool.pool || pointer > (void *)memMainPool.last)
    {                                                       //if it's not part of the main pool
        for (index = 0, pool = memGrowthPool; index < memNumberGrowthPools; index++, pool++)
        {                                                   //search all existing pools
            if (pointer > (void *)pool->pool && pointer <= (void *)pool->last)
            {
                goto foundPool;
            }
        }
#if MEM_ERROR_CHECKING
        dbgFatalf(DBG_Loc, "Cound not find pool to which pointer 0x%x belongs.", pointer);
#endif
    }
foundPool:;

#if MEM_ANAL_CHECKING
    memPoolAnalCheck(pool);
#endif
    //see if it's a small block heap cookie
#if MEM_SMALL_BLOCK_HEAP
    if (bitTest(cookie->flags, MBF_SmallBlockHeap))
    {
        memVolatilityLog(cookie);
        //dbgAssert(pointer >= memMainPool.pool && pointer <= memMainPool.last);//SBH allocations can only be in the main pool
        memFreeSBH((mbhcookie *)cookie);
#if MEM_ANAL_CHECKING
        memPoolAnalCheck(pool);
#endif
        return;
    }
#endif //MEM_SMALL_BLOCK_HEAP

    memVolatilityLog(cookie);
#if MEM_DETECT_VOLATILE
    if (taskTimeElapsed - cookie->timeAllocated > MEM_VolatileLong)
    {                                                       //if cookie not volatile
        if (!bitTest(cookie->flags, MBF_NonVolatile))
        {
#if MEM_DEBUG_NV
            dbgMessagef("\nmemFree: cookie 0x%x ('%s') is NON-volatile.", cookie, cookie->name);
#endif
#if MEM_FILE_NV
            if (memNonVolatileFile)
            {
                fprintf(memNonVolatileFile, "memFree: cookie 0x%x ('%s') is NON-volatile.\n", cookie, cookie->name);
            }
#endif //MEM_FILE_NV
        }
    }
    else if (taskTimeElapsed - cookie->timeAllocated < MEM_VolatileShort)
    {                                                       //else cookie is volatile
        if (bitTest(cookie->flags, MBF_NonVolatile))
        {                                                   //and supposedto be non-volatile
#if MEM_DEBUG_NV
            dbgMessagef("\nmemFree: cookie 0x%x ('%s') is volatile.", cookie, cookie->name);
#endif
#if MEM_FILE_NV
            if (memNonVolatileFile)
            {
                fprintf(memNonVolatileFile, "memFree: cookie 0x%x ('%s') is volatile.\n", cookie, cookie->name);
            }
#endif //MEM_FILE_NV
        }
    }
#endif //MEM_DETECT_VOLATILE

#if MEM_ERROR_CHECKING
    dbgAssert(cookie->blocksNext > 0);                      //ensure non-zero size
    dbgAssert((ubyte *)cookie + sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext) <= (ubyte *)pool->last);
#endif

#if MEM_VERBOSE_LEVEL >= 2
    dbgMessagef("\nmemFree: freed %d bytes of '%s' from 0x%x", memBlocksToBytes(cookie->blocksNext), cookie->name, cookie);
#endif

    bitClear(cookie->flags, MBF_AllocatedNext);             //say it's not active
    memNameSet(cookie, MEM_HeapFree);                       //clear name of cookie
    //clear actual memory block
#if MEM_CLEAR_MEM_ON_FREE
    memClearDword(pointer, memFreeSetting, memBlocksToBytes(cookie->blocksNext) / 4);
#endif

    nextCookie = (memcookie *)((ubyte *)cookie +            //next cookie
                            sizeof(memcookie) + memBlocksToBytes(cookie->blocksNext));
    if (bitTest(cookie->flags, MBF_NonVolatile))
    {                                                       //if allocated non-volatile
        memFreeNV(cookie, nextCookie, pool);
#if MEM_ANAL_CHECKING
        memPoolAnalCheck(pool);
#endif
        return;
    }

    if (nextCookie < pool->last)                               //if not end of heap
    {
        memCookieVerify(nextCookie);                        //make sure cookie is valid
        if (!bitTest(nextCookie->flags, MBF_AllocatedNext)) //if next block also free
        {
#if MEM_VERBOSE_LEVEL >= 3
            dbgMessagef("\nmemFree: combined blocks 0x%x(%d) and 0x%x(%d) into one.", cookie, memBlocksToBytes(cookie->blocksNext), nextCookie, memBlocksToBytes(nextCookie->blocksNext));
#endif
                                                            //combine sizes
            stillNextCookie = nextCookie + nextCookie->blocksNext + 1;
            memCookieVerify(stillNextCookie);
            cookie->blocksNext += nextCookie->blocksNext + MEM_BlocksPerCookie;
            stillNextCookie->blocksPrevious = cookie->blocksNext;
            //make sure that no min size pointers are referencing lost cookie
            for (index = 0; index < MSB_NumberSizes + 1; index++)
            {
                if (pool->firstSize[index] == nextCookie)
                {
                    pool->firstSize[index] = cookie;
                }
                if (pool->lastSize[index] == nextCookie)
                {
                    pool->lastSize[index] = stillNextCookie;
                }
            }

#if MEM_CLEAR_MEM                                           //clear lost cookie
            memClearDword(nextCookie, memFreeSetting, sizeof(memcookie) / sizeof(udword));
#endif
        }
    }
    //attempt to combine with previous cookie if both free
    if (cookie->blocksPrevious != -1)
    {                                                       //if there is a previous block
        previousCookie = (memcookie *)((ubyte *)cookie -    //get previous block
                                sizeof(memcookie) - memBlocksToBytes(cookie->blocksPrevious));
        dbgAssert(previousCookie >= pool->first);
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
            memClearDword(cookie, memFreeSetting, sizeof(memcookie) / sizeof(udword));
#endif
            //make sure that no pointers are referencing lost cookie
            for (index = 0; index < MSB_NumberSizes + 1; index++)
            {                                               //check all the min size pointers
                if (pool->firstSize[index] == cookie)
                {
                    pool->firstSize[index] = previousCookie;
                }
                if (pool->lastSize[index] == cookie)
                {
                    pool->lastSize[index] = nextCookie;
                }
            }
            if (pool->lastFree == cookie)
            {                                               //check the last free pointer
                pool->lastFree = nextCookie;
            }
            cookie = previousCookie;                        //make sure that the later resizing will work
        }
        else
        {
            length = cookie->blocksNext * MEM_BlockSize;
            if (length <= MSB_SizeMax && msbSizeBytes(length) > cookie)
            {                                               //set the new size block min
                msbSizeBytes(length) = cookie;
            }
        }
    }
    if (pool->firstFree > cookie)                              //make sure first free is the actual first free
    {
        pool->firstFree = cookie;
    }
    dbgAssert(pool->lastFree > pool->firstFree && pool->lastFree <= pool->last);
#if MEM_ANAL_CHECKING
    memPoolAnalCheck(pool);
#endif
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

    newString = memAlloc(strlen(string) + 1, string, MBF_String);
    strcpy(newString, string);
    return(newString);
}

/*-----------------------------------------------------------------------------
    Name        : memStringDupeNV
    Description : Duplicate a string into non-volatile RAM
    Inputs      : string - pointer to string to duplicate
    Outputs     : allocates and strcpy's the string
    Return      : newly allocated string
    Note        : Use memFree to free these strings
----------------------------------------------------------------------------*/
char *memStringDupeNV(char *string)
{
    char *newString;

    newString = memAlloc(strlen(string) + 1, string, MBF_String);
    strcpy(newString, string);
    return(newString);
}

/*-----------------------------------------------------------------------------
    Name        : memReallocFunction
    Description : Grow or shrink a block of memory.
    Inputs      : currentPointer - currently allocated block, or NULL for no block.
                  newSize - the new size to be sized to
                  name - new name
                  flags - new allocation flags
    Outputs     :
    Return      : newly allocated or resized block.
    Note        : This function will always re-allocate the block.  This can
                    be optimized.
----------------------------------------------------------------------------*/
#if MEM_USE_NAMES
void *memReallocFunction(void *currentPointer, sdword newSize, char *name, udword flags)
#else
void *memReallocFunction(void *currentPointer, sdword newSize, udword flags)
#endif
{
    void *newPointer;
    memcookie *cookie;
    sdword oldLength;

    if (currentPointer == NULL)
    {
        newPointer = memAlloc(newSize, name, flags)
        return(newPointer);
    }
    newPointer = memAlloc(newSize, name, flags);
    cookie = (memcookie *)((ubyte *)currentPointer - sizeof(memcookie));
#if MEM_SMALL_BLOCK_HEAP
    if (bitTest(cookie->flags, MBF_SmallBlockHeap))
    {
        oldLength = ((mbhcookie*)cookie)->length;
    }
    else
    {
#endif
        oldLength = memBlocksToBytes(cookie->blocksNext);
#if MEM_SMALL_BLOCK_HEAP
    }
#endif
    memcpy(newPointer, currentPointer, min(newSize, oldLength));
    memFree(currentPointer);
    return(newPointer);
}

/*-----------------------------------------------------------------------------
    Name        : memFreeMemGet
    Description : Compute size of all available memory.
    Inputs      : pool - what pool to get the stats on
    Outputs     : ..
    Return      : available memory, in bytes
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
sdword memFreeMemGet(mempool *pool)
{
    memcookie *thisCookie, *nextCookie;
    sdword freeMem = 0;

    thisCookie = pool->first;                                  //start walk from very start of heap

    while (thisCookie < pool->last)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        if (!bitTest(thisCookie->flags, MBF_AllocatedNext))
        {
            freeMem += memBlocksToBytes(thisCookie->blocksNext);
        }
        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        if (nextCookie >= pool->last)                          //if cookie pointed past end of heap
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
    Inputs      : pool - what pool to get the stats on
    Outputs     : ..
    Return      : Amount of used memory, in bytes.
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
sdword memUsedMemGet(mempool *pool)
{
    memcookie *thisCookie, *nextCookie;
    sdword usedMem = 0;

    thisCookie = pool->first;                                  //start walk from very start of heap

    while (thisCookie < pool->last)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        if (bitTest(thisCookie->flags, MBF_AllocatedNext))
        {
            usedMem += memBlocksToBytes(thisCookie->blocksNext);
        }
        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        if (nextCookie >= pool->last)                          //if cookie pointed past end of heap
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
    Inputs      : pool - what pool to get the stats on
    Outputs     : ..
    Return      : Amount of memory used for cookies, in bytes.
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
sdword memCookiesLengthGet(mempool *pool, sdword *nCookies)
{
    memcookie *thisCookie, *nextCookie;
    sdword usedMem = *nCookies = 0;

    thisCookie = pool->first;                                  //start walk from very start of heap

    while (thisCookie < pool->last)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        usedMem += sizeof(memcookie);

        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        (*nCookies)++;
        if (nextCookie >= pool->last)                          //if cookie pointed past end of heap
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
    Inputs      : pool - what pool to get the stats on
    Outputs     : ..
    Return      : Size of largest free memory block, in bytes.
----------------------------------------------------------------------------*/
#if MEM_ANALYSIS
sdword memLargestBlockGet(mempool *pool)
{
    memcookie *thisCookie, *nextCookie;
    sdword largestSize = 0;

    thisCookie = pool->first;                                  //start walk from very start of heap

    while (thisCookie < pool->last)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
        if (!bitTest(thisCookie->flags, MBF_AllocatedNext))
        {
            largestSize = max(largestSize, memBlocksToBytes(thisCookie->blocksNext));
        }
        nextCookie = (memcookie *)((ubyte *)thisCookie +    //next cookie
                sizeof(memcookie) + memBlocksToBytes(thisCookie->blocksNext));
        if (nextCookie >= pool->last)                          //if cookie pointed past end of heap
        {
            goto noMoreFree;                                //no more free structures
        }

        thisCookie = nextCookie;                            //next update
    }
noMoreFree:;
    return largestSize;
}
#endif

#if MEM_ANALYSIS
/*-----------------------------------------------------------------------------
    Name        : memAnalysisCreateForPool
    Description : Creates and prints a detailed analysis of the current state
                    of the memory heap.
    Inputs      : pool - what pool to log the stats on
                  fpAnalysis - the analysis file pointer (mem.analysis)
                  fpMap - the map file pointer (mem.map)
    Outputs     : Calls the previous 3 memory computation functions, then saves
                    a detailed memory map to disk.
    Return      :
----------------------------------------------------------------------------*/
#define MEM_ANALYSIS_FILE_NAME      "mem.analysis"
#define MEM_MAP_FILE_NAME           "mem.map"
#define MEM_MAP_BLOCKS_PER_LINE     80
#define MEM_MAP_CHAR(c)             fputc((c), fpMap); mapColumn++; if (mapColumn >= MEM_MAP_BLOCKS_PER_LINE) {fputc('\n', fpMap);mapColumn = 0;}
void memAnalysisCreateForPool(mempool *pool, FILE *fpAnalysis, FILE *fpMap)
{
    sdword largeBlock, freeMem, usedMem, cookieMem, wastedMem, nCookies, index;
    memcookie *thisCookie, *nextCookie;
    sdword mapColumn = 0, block;
#if MEM_SMALL_BLOCK_HEAP
    Node *cookieLink;
    mbheap *heap;
    mbhcookie *bCookie;
#endif

    dbgMessagef("\nHeap length     = %d", pool->heapLength);
    fprintf(fpAnalysis, "Heap length     = %d\n", pool->heapLength);
    largeBlock = memLargestBlockGet(pool);
    dbgMessagef("\nLargest block   = %d (%.2f%%)", largeBlock, (real32)largeBlock / (real32)pool->heapLength * 100.0f);
    fprintf(fpAnalysis, "Largest block   = %d (%.2f%%)\n", largeBlock, (real32)largeBlock / (real32)pool->heapLength * 100.0f);
    usedMem = memUsedMemGet(pool);
    dbgMessagef("\nUsed memory     = %d (%.2f%%)", usedMem, (real32)usedMem / (real32)pool->heapLength * 100.0f);
    fprintf(fpAnalysis, "Used memory     = %d (%.2f%%)\n", usedMem, (real32)usedMem / (real32)pool->heapLength * 100.0f);
    freeMem = memFreeMemGet(pool);
    dbgMessagef("\nFree memory     = %d (%.2f%%)", freeMem, (real32)freeMem / (real32)pool->heapLength * 100.0f);
    fprintf(fpAnalysis, "Free memory     = %d (%.2f%%)\n", freeMem, (real32)freeMem / (real32)pool->heapLength * 100.0f);
    cookieMem = memCookiesLengthGet(pool, &nCookies);
    dbgMessagef("\nCookie memory   = %d (%.2f%%), %d cookies", cookieMem, (real32)cookieMem / (real32)pool->heapLength * 100.0f, nCookies);
    fprintf(fpAnalysis, "Cookie memory   = %d (%.2f%%), %d cookies\n", cookieMem, (real32)cookieMem / (real32)pool->heapLength * 100.0f, nCookies);
    wastedMem = pool->heapLength - (usedMem + freeMem + cookieMem);
    if (wastedMem > 0)
    {
        dbgMessagef("\nLost memory     = %d (%.2f%%)", wastedMem, (real32)wastedMem / (real32)pool->heapLength * 100.0f);
        fprintf(fpAnalysis, "Lost memory     = %d (%.2f%%)\n", wastedMem, (real32)wastedMem / (real32)pool->heapLength * 100.0f);
    }

    fprintf(fpAnalysis, "Cookie Name\t\tCookie length/maxLength\tCookie status\n");
    fprintf(fpMap, "Legend: @ = smallBlockHeap, * = cookie, # = allocated, . = free.  Block length = %d bytes\n", MEM_BlockSize);
#if MEM_SMALL_BLOCK_HEAP
    //walk through small block heaps and print contents
    if (pool == &memMainPool)
    {
        for (index = 0, heap = (mbheap *)pool->pool; memSmallHeapInfo[index].blockSize > 0; index++, heap++)
        {                                                   //for all heaps
            //dbgAssert(heap->nBlocks == memSmallHeapInfo[index].nBlocks);
            fprintf(fpAnalysis, "Heap 0x%x, length = %d\n", (size_t)heap, heap->nBlocks * heap->blockSize);
#if MEM_SMALLBLOCK_STATS
            if (heap->nAllocationAttempts == 0)
            {
                fprintf(fpAnalysis, "avg. usage = %.2f%%, overfull = %.2f%%, allocs/sec = %.2f\n", 0.0f, 0.0f, 0.0f);
            }
            else
            {
                fprintf(fpAnalysis, "avg. usage = %.2f%%, overfull = %.2f%%, allocs/sec = %.2f\n",
                       heap->usage / (real32)heap->nBlocks / (real32)heap->nAllocationAttempts * 100.0f,
                       (real32)heap->nOverflowedAllocations / (real32)heap->nAllocationAttempts * 100.0f,
                       (real32)heap->nAllocationAttempts / (taskTimeElapsed - heap->firstAllocTime));
            }
            heap->nAllocationAttempts = 0;
            heap->nOverflowedAllocations = 0;
            heap->usage = 0.0f;
            heap->firstAllocTime = taskTimeElapsed;
#endif
            MEM_MAP_CHAR('@');
            for (cookieLink = heap->allocated.head; cookieLink != NULL; cookieLink = cookieLink->next)
            {                                               //for all allocated blocks
                bCookie = listGetStructOfNode(cookieLink);
                mbhCookieVerify(bCookie);
#if MEM_USE_NAMES
                fprintf(fpAnalysis, "%20s, %10d, %10d, %10s, %10x\n", bCookie->name, bCookie->length, heap->blockSize, "allocated", (size_t)bCookie);
#else
                fprintf(fpAnalysis, "%20s, %10d, %10d, %10s, %10x\n", "noname", bCookie->length, heap->blockSize, "allocated", (size_t)bCookie);
#endif
                MEM_MAP_CHAR('*');                          //print cookie character
                for (block = 0; block < bCookie->length / MEM_BlockSize; block++)
                {                                           //print characters for portion of block that is used
                    MEM_MAP_CHAR('#');
                }
                if (bCookie->length % MEM_BlockSize > 0)
                {                                           //print the partly allocated character
                    MEM_MAP_CHAR('0' + ((bCookie->length % MEM_BlockSize) * 10) / MEM_BlockSize);
                    block++;
                }
                for (; block < heap->blockSize / MEM_BlockSize; block++)
                {                                           //print the remainder of the block(unused space)
                    MEM_MAP_CHAR('.');
                }
            }
            for (cookieLink = heap->free.head; cookieLink != NULL; cookieLink = cookieLink->next)
            {                                               //for all free blocks
                bCookie = listGetStructOfNode(cookieLink);
                mbhCookieVerify(bCookie);
#if MEM_USE_NAMES
                fprintf(fpAnalysis, "%20s, %10d, %10d, %10s, %10x\n", bCookie->name, 0, heap->blockSize, "free", (size_t)bCookie);
#else
                fprintf(fpAnalysis, "%20s, %10d, %10d, %10s, %10x\n", "noname", 0, heap->blockSize, "free", (size_t)bCookie);
#endif
                MEM_MAP_CHAR('*');                          //print cookie character
                for (block = 0; block < heap->blockSize / MEM_BlockSize; block++)
                {                                           //print characters for portion of block that is used
                    MEM_MAP_CHAR('.');
                }
            }
        }
    }
#endif //MEM_SMALL_BLOCK_HEAP
    //print values of the small block pointers
    for (index = 0; index < MSB_NumberSizes + 1; index++)
    {
        fprintf(fpAnalysis, "lowestFree 0x%x = 0x%x\n", ((index) * MEM_BlockSize) + MEM_BlockSize, (size_t)pool->firstSize[index]);
    }
    thisCookie = pool->first;                                  //start walk from very start of heap

    fprintf(fpAnalysis, "Cookie Name\t\tCookie length\tPrevious block length\tCookie status\n");
    //walk through regular heap and print analysis
    while (thisCookie < pool->last)                            //scan for free cookies in heap
    {
        memCookieVerify(thisCookie);                        //verify this cookie
#if MEM_USE_NAMES
        fprintf(fpAnalysis, "%20s, %10d, %10d, %10s, %10x\n", thisCookie->name, memBlocksToBytes(thisCookie->blocksNext), memBlocksToBytes(thisCookie->blocksPrevious), bitTest(thisCookie->flags, MBF_AllocatedNext) ? "allocated" : "free", (size_t)thisCookie);
#else
        fprintf(fpAnalysis, "%20s, %10d, %10d, %10s, %10x\n", "noname", memBlocksToBytes(thisCookie->blocksNext), memBlocksToBytes(thisCookie->blocksPrevious), bitTest(thisCookie->flags, MBF_AllocatedNext) ? "allocated" : "free", (size_t)thisCookie);
#endif
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
        if (nextCookie >= pool->last)                          //if cookie pointed past end of heap
        {
            break;                                          //no more free structures
        }

        thisCookie = nextCookie;                            //next update
    }
#if MEM_LOG_MOSTVOLATILE
    //print sorted list of memory volatility
    fprintf(fpAnalysis, "Cookie volatility sorted by average volatility.\n");
    fprintf(fpAnalysis, "Name, number allocations logged, average volatility (seconds):\n");
    //first pass: compute volatility level
    for (index = 0; index < MEM_NumberVolatileStats; index++)
    {                                                       //for all cookies
        if (memVolatileStat[index].nLogged == 0.0f)
        {                                                   //if end of stat list
            break;
        }
        //compute this stat's volatility
        memVolatileStat[index].volatility = memVolatileStat[index].totalTime / memVolatileStat[index].nLogged;
    }
    //sort by volatility
    qsort(memVolatileStat, index, sizeof(volatilestat), memVolatilitySort);
    //second pass: print values and clear structures for another logging session
    for (block = 0; block < index; block++)
    {
        fprintf(fpAnalysis, "%16s, %5.0f, %5.2f\n", memVolatileStat[block].name, memVolatileStat[block].nLogged, memVolatileStat[block].volatility);
        memVolatileStat[block].name[0] = 0;                 //clear out the name
        memVolatileStat[block].nLogged = 0.0f;              //and the rest of structure
        memVolatileStat[block].totalTime = 0.0f;
    }
#endif //MEM_LOG_MOSTVOLATILE
}

/*-----------------------------------------------------------------------------
    Name        : memAnalysisCreate
    Description : Creates and prints a detailed analysis of the current state
                    of the memory heap.
    Inputs      : void
    Outputs     : Calls the previous 3 memory computation functions, then saves
                    a detailed memory map to disk.
    Return      :
----------------------------------------------------------------------------*/
void memAnalysisCreate(void)
{
    char *memAnalysisFileNameFull, *memMapFileNameFull;
    FILE *fpAnalysis, *fpMap;
    sdword index;

    dbgMessagef("\nSaving detailed analysis to '%s' and map to '%s'", MEM_ANALYSIS_FILE_NAME, MEM_MAP_FILE_NAME);

    memAnalysisFileNameFull = filePathPrepend(
        MEM_ANALYSIS_FILE_NAME, FF_UserSettingsPath);
    memMapFileNameFull = filePathPrepend(
        MEM_MAP_FILE_NAME, FF_UserSettingsPath);

    if (!fileMakeDestinationDirectory(memAnalysisFileNameFull))
    {
        dbgWarningf(
            DBG_Loc,
            "Error creating destination directory for memory analysis files.");
        return;
    }

    fpAnalysis = fopen(memAnalysisFileNameFull, "wt");
    fpMap = fopen(memMapFileNameFull, "wt");
    if (fpMap == NULL || fpAnalysis == NULL)
    {
        dbgWarningf(DBG_Loc, "Error opening either '%s' or '%s'.", MEM_ANALYSIS_FILE_NAME, MEM_MAP_FILE_NAME);

        if (fpMap)
            fclose(fpMap);
        else if (fpAnalysis)
            fclose(fpAnalysis);

        return;
    }

    memAnalysisCreateForPool(&memMainPool, fpAnalysis, fpMap);

    for (index = 0; index < memNumberGrowthPools; index++)
    {
        memAnalysisCreateForPool(&memGrowthPool[index], fpAnalysis, fpMap);
    }
    fclose(fpAnalysis);
    fclose(fpMap);
}
#endif //MEM_ANALYSIS

/*-----------------------------------------------------------------------------
    Name        : memStrncpy
    Description : Line strncpy, but it copies the NULL terminator will always be appended.
                  If length of source > count, dest[count - 1] will be a NULL terminator.
    Inputs      : dest - where to copy to
                  source - where to copy from
                  count - max length of dest
    Outputs     :
    Return      : dest
----------------------------------------------------------------------------*/
char *memStrncpy(char *dest, char *source, sdword count)
{
    char *destStart = dest;

    while (1)
    {
        if (count == 1/* && *(source + 1) != 0*/)
        {
            *dest = 0;
            break;
        }
        *dest = *source;
        if (*source == 0/* || count == 0*/)
        {
            break;
        }
        count--;
        dest++;
        source++;
    }
    return(destStart);
}

