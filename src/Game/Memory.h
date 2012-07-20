/*=============================================================================
    MEMORY.H:  Definitions for Homeworld memory management module.

    Created June 1997 by Luke Moloney
=============================================================================*/

#ifndef ___MEMORY_H
#define ___MEMORY_H

#include "Task.h"
#include "LinkedList.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define MEM_FORCE_CREEP_UP      0               //force the memory pointers to creep up
#define MEM_SMALL_BLOCK_HEAP    1               //enable small block heap allocations
#define MEM_VOLATILE_CLEARING   1               //the clear dwords are volatile and unreliable

#ifndef HW_Release

#define MEM_MODULE_TEST         0               //test the module
#define MEM_ERROR_CHECKING      1               //basic error checking
#define MEM_CLEAR_MEM           1               //clear newly allocated blocks to all clear code
#define MEM_CLEAR_MEM_ON_FREE   1               //clear newly freed blocks to all clear code
#define MEM_VERBOSE_LEVEL       1               //control verbose printing
#define MEM_USE_NAMES           1               //use names in allocated blocks
#define MEM_DEFRAGMENT_FREE     1               //attempt to join with adjacent free block(s) when freeing
#define MEM_ANALYSIS            1               //enable memory analysis functions
#define MEM_STATISTICS          1               //track memory usage stats
#define MEM_ANALYSIS_KEY        QKEY            //create a memory analysis on demand
#define MEM_DETECT_VOLATILE     1               //automatically detect what memory blocks are volatile
#define MEM_FILE_NV             0               //log non-volatile allocs to a file
#define MEM_DEBUG_NV            0               //log non-volatile allocs to debug window
#define MEM_SMALLBLOCK_STATS    1               //log usage stats on small memory blocks
#define MEM_LOG_MOSTVOLATILE    1               //keep track of the most volatile types of memory
#define MEM_ANALYSIS_AUTOCREATE 1               //automatically create memory analysis if there's an allocation failure
#define MEM_VOLATILE_CLEARING   1               //the clear dwords are volatile and unreliable
#define MEM_ANAL_CHECKING       1               //extra hard-core debuggery action

#else //HW_Debug

#define MEM_MODULE_TEST         0               //don't test the module
#define MEM_ERROR_CHECKING      0               //no error ckecking in retail
#define MEM_CLEAR_MEM           0               //don't clear any blocks to zeros
#define MEM_CLEAR_MEM_ON_FREE   0               //clear newly freed blocks to all clear code
#define MEM_VERBOSE_LEVEL       0               //don't print any verbose strings in retail
#define MEM_USE_NAMES           0               //no names please ma'am
#define MEM_DEFRAGMENT_FREE     1               //attempt to join with adjacent free block(s) when freeing
#define MEM_ANALYSIS            0               //disable memory analysis functions
#define MEM_STATISTICS          0               //track memory usage stats
#define MEM_ANALYSIS_KEY        0               //create a memory analysis on demand
#define MEM_DETECT_VOLATILE     0               //automatically detect what memory blocks are volatile
#define MEM_FILE_NV             0               //log non-volatil allocs to a file
#define MEM_DEBUG_NV            0               //log non-volatil allocs to debug window
#define MEM_SMALLBLOCK_STATS    0               //log usage stats on small memory blocks
#define MEM_LOG_MOSTVOLATILE    0               //keep track of the most volatile types of memory
#define MEM_ANALYSIS_AUTOCREATE 0               //automatically create memory analysis if there's an allocation failure
#define MEM_ANAL_CHECKING       0               //extra hard-core debuggery action

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//memory block flags
#define MBF_AllocatedNext       1               //memory following block allocated
#define MBF_AllocatedPrev       2               //preceeding memory block allocated
//#define MBF_Purgable            4             //will be purged on next call to memPurge()
#define MBF_SmallBlockHeap      4               //allocated from a small block heap
#define Flammable               MBF_SmallBlockHeap//another way of saying very volatile
#define Pyrophoric              MBF_SmallBlockHeap//another way of saying very volatile
#define MBF_Volatile            0               //expect this block to be freed soon
#define Volatile                MBF_Volatile
#define MBF_NonVolatile         8
#define MBF_String              16
#define NonVolatile             MBF_NonVolatile
#define MBF_ExtendedPoolOnly    32
#define ExtendedPool            MBF_ExtendedPoolOnly
#define MBF_VerifyMask          0xffffffc0      //remaining bits used for verification mask
#define MBF_ParameterMask       (MBF_NonVolatile | MBF_SmallBlockHeap | MBF_String) //which parameter flags to store in the cookie flags

//verification value
#define MBF_VerifyValue         (0xfa7babe5 & MBF_VerifyMask)

//max length for cookie name strings
#if MEM_DETECT_VOLATILE
#define MEM_NameLength          16
#else
#define MEM_NameLength          20
#endif
#if MEM_LOG_MOSTVOLATILE
#define MBH_NameLength          4
#else
#define MBH_NameLength          8
#endif

//allocation block granularity.
#define MEM_BlockSize           ((sdword)sizeof(memcookie))
#define MEM_BlocksPerCookie     1

//default memory heap size
#define MEM_HeapSizeDefault     (18 * 1024 * 1024)
#define MEM_HeapDefaultScalar   0.40f
#define MEM_HeapDefaultMax      (64 * 1024 * 1024)

//default name strings
#define MEM_NameHeap            "HeapLow(free)"
#define MEM_NameHeapFirst       "HeapLowFirst(Free)"
#define MEM_HeapFree            "(free)"
#define MEM_NameHeapLast        "HeapHighFirst(free)"

//memory clearing settings
#define MEM_ClearSetting        0x4e7a110c
#define MEM_FreeSetting         0xf4eeda7a

//memory statistics
#define MEM_AllocsPerStatPrint  100
#define MEM_StatsKey            QKEY

//small block allocation defines
//the smallest small allocation block must be smaller than the smallest allocation
#define MSB_SizeMin             MEM_BlockSize
#define MSB_SizeMax             2048
#define MSB_NumberSizes         ((MSB_SizeMax - MSB_SizeMin) / MEM_BlockSize)
#define MEM_OptCounterFreqSmall 2048
#define MEM_OptCounterFreqBig   8192
#define MEM_GrowFactor          96 / 256        //grow by 37.5% when a SBH pool runs out

//for printing memory statistics
#define MEM_TaskStatsPeriod     1.0             //once per second

#define MEM_VolatileLong        5.0f            //any block that is freed after this amount of time is non-volatile
#define MEM_VolatileShort       1.0f

#define MEM_HeapValidation      0xbad600f       //bad guy validation

//byte offsets for finding a heap from the linked list
#define MBH_AllocatedLinkOffset 0
//#define MBH_FreeLinkOffset      (4 + sizeof(LinkedList))

//definitions for cookie volatility logging
#define MEM_NumberVolatileStats 768

//definitions for tracking the most commonly allocated cookie names
#define MS_NumberCookieNames    64
#define MS_OutputStringLength   128
#define MS_NameStringLength     64

//definitions for growing the memory heaps
#define MGH_NumberGrowthHeaps   16              //maximum number of extra heaps we can grow to
#define MGH_MinGrowthSize       (4 * 1024 * 1024 - sizeof(memcookie) * 2)//grow in 4MB increments.  That's 64MB extra heap size.  Should be sufficient.

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure for a memory block cookie
typedef struct
{
    udword flags;                               //validation and information
#if MEM_USE_NAMES
    char name[MEM_NameLength];                  //name of memory block
#else
    ubyte pad[MEM_NameLength];                  //round structure up to 16 bytes
#endif
    sdword blocksNext;                          //number of blocks to next cookie
    sdword blocksPrevious;                      //number of blocks to previous cookie
                                                //only valid when freeing memory
#if MEM_DETECT_VOLATILE
    real32 timeAllocated;                       //when was this block allocated?
#endif
}
memcookie;

//structure for a cookie in a small block heap.  These cookies are created only once.
//location of flags and name string must be compatable with the regular cookies because
//some functions are shared between cookie structure formats.
#if MEM_SMALL_BLOCK_HEAP
typedef struct
{
    udword flags;                               //validation and information
#if MEM_USE_NAMES
    char name[MBH_NameLength];                  //name of memory block (must correspond to location in structure and size as the name in regular cookies)
#else
    char pad[MBH_NameLength];
#endif
    Node link;                                  //tied to whatever linked list it's in
    sdword length;                              //un-rounded length of block allocated
#if MEM_LOG_MOSTVOLATILE
    real32 timeAllocated;                       //when was this block allocated?
#endif
}
mbhcookie;

//structure for a small block heap
typedef struct
{
    LinkedList allocated;
    LinkedList free;
    sdword blockSize;
    sdword nBlocks;
    mbhcookie *heapStart;
    mbhcookie *heapEnd;
#if MEM_ERROR_CHECKING
    udword validation;                          //used to detect that we're in a proper small block heap
#endif
#if MEM_SMALLBLOCK_STATS
    sdword nAllocationAttempts;                 //number of allocations attempted in this heap
    sdword nOverflowedAllocations;              //number of allocations which overflowed this heap
    real32 usage;                               //accumulated usage, blocks
    real32 firstAllocTime;                      //time this logging began
#endif
}
mbheap;
#endif//MEM_SMALL_BLOCK_HEAP

#if MEM_LOG_MOSTVOLATILE
//structure for logging most volatile type of allocations
typedef struct
{
    char name[MEM_NameLength];                  //name of this type allocation
    real32 nLogged;                             //number of logged allocations
    real32 totalTime;                           //total length of allocated
    real32 volatility;                          //computed and stored for sorting
}
volatilestat;
#endif

#if MEM_STATISTICS
//structure for logging most commonly allocated cookie types
typedef struct
{
    char name[MS_NameStringLength];
    sdword nAllocs;
    sdword nTotalSize;
    char outputString[MS_OutputStringLength];
}
memcookiename;
#endif

//structure for a memory heap.  This contains all other memory structures
typedef struct
{
    sdword heapLength;                          //size of heap
    sdword poolLength;                          //size of pool including small block heaps
    void *wholePool;                            //possibly non-aligned heap base
    ubyte *pool;                                //base of heap, aligned on a cookie boundary
    memcookie *first;                           //first cookie, start of heap
    memcookie *firstFree;                       //lowest free cookie
    memcookie *lastFree;                        //cookie AFTER last free cookie
    memcookie *last;                            //last cookie, end of heap
    memcookie *biggestBlock;                    //biggest block in the pool
    memcookie *biggestBlockSize;                //size of that biggest block
    memcookie *firstSize[MSB_NumberSizes + 1];  //lowest free cookie of sizes
    memcookie *lastSize[MSB_NumberSizes + 1];   //highest free cookie of sizes
}
mempool;

//info on small block heaps.  Pretty simple
typedef struct
{
    sdword blockSize;                           //size of blocks the small block heap
    sdword nBlocks;                             //number of blocks of said size
}
memsmallheapinfo;

typedef void *(*memgrowcallback)(sdword heapSize);//callback for growing memory
typedef void (memgrowthfreecallback)(void *heap);//callback for freeing growth heaps

/*=============================================================================
    Data:
=============================================================================*/
//memory statistics
#if MEM_STATISTICS
extern sdword memNumberWalks;
extern sdword memNumberAllocs;
extern char memStatString[256];
extern bool memStatsLogging;
extern taskhandle memStatsTaskHandle;
extern void memStatsTaskFunction(void);
extern memcookiename memStatsCookieNames[MS_NumberCookieNames];
#endif
extern mempool memMainPool;

/*=============================================================================
    Macros:
=============================================================================*/
//name-specific stuff
#if MEM_USE_NAMES
#define memAlloc(l, n, f) memAllocFunction((l), (n), (f));
#define memAllocAttempt(l, n, f) memAllocAttemptFunction((l), (n), (f));
#define memNameSet(c, s)    memNameSetFunction((c), (s))
#define mbhNameSet(c, s)    mbhNameSetFunction((c), (s))
#define memNameSetLong(c, s)    memNameSetFunction((c), (s))
#define memRealloc(p, l, n, f) memReallocFunction((p), (l), (n), (f));
#else
#define memAlloc(l, n, f) memAllocFunction((l), (f));
#define memAllocAttempt(l, n, f) memAllocAttemptFunction((l), (f));
#define memNameSet(c, s)
#define mbhNameSet(c, s)
#define memNameSetLong(c, s)
#define memRealloc(p, l, n, f) memReallocFunction((p), (l), (f));
#endif//MEM_USE_NAMES

//block size macros
#define memRoundUp(n)   (((n) + (MEM_BlockSize - 1)) & (~(MEM_BlockSize - 1)))
#define memRoundDown(n) ((n) & (~(MEM_BlockSize - 1)))
#define memBlocksCompute(s, e)  ((memRoundDown((udword)(e)) - memRoundUp((udword)(s))) / MEM_BlockSize)
#define memBlocksToBytes(b)     ((b) * MEM_BlockSize)
#define memBytesToBlocks(b)     ((b) / MEM_BlockSize)

//verify that a cookie is actually a cookie
#if MEM_ERROR_CHECKING
#define memCookieVerify(c)\
    if (((c)->flags & MBF_VerifyMask) != MBF_VerifyValue)\
        dbgFatalf(DBG_Loc, "Corrupt cookie: 0x%x", (c)->flags & MBF_VerifyMask)
#define mbhCookieVerify(c)\
    if (((c)->flags & MBF_VerifyMask) != MBF_VerifyValue)\
        dbgFatalf(DBG_Loc, "Corrupt small block heap cookie: 0x%x", (c)->flags & MBF_VerifyMask)
#else
#define memCookieVerify(c)
#define mbhCookieVerify(c)
#endif

//if module not started properly, generate an error
#if MEM_ERROR_CHECKING
#define memInitCheck()\
    if (memModuleInit == FALSE)\
    {\
        dbgFatal(DBG_Loc, "Called before memory module started or after closed.");\
    }
#else
#define memInitCheck()
#endif

//small block allocation
#define msbSizeBytes(length)    (pool->firstSize[((length) / MEM_BlockSize) - 1])
#define msbSizeBlocks(length)   (pool->firstSize[length - 1])
#define msbSizeIndex(length)    (((length) / MEM_BlockSize) - 1)
#define mslSizeBytes(length)    (pool->lastSize[((length) / MEM_BlockSize) - 1])
#define mslSizeBlocks(length)   (pool->lastSize[length - 1])
#define mslSizeIndex(length)    (((length) / MEM_BlockSize) - 1)

#if MEM_LOG_MOSTVOLATILE
#define memVolatilityLog(c)     memVolatilityLogFuntion(c)
#else
#define memVolatilityLog(c)
#endif

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown/reset memory module
sdword memStartup(void *heapStart, sdword heapSize, memgrowcallback grow);
sdword memReset(void);
sdword memClose(memgrowthfreecallback free);

//allocate/free memory blocks
#if MEM_USE_NAMES
void *memAllocFunction(sdword length, char *name, udword flags);
#else
void *memAllocFunction(sdword length, udword flags);
#endif
#if MEM_USE_NAMES
void *memReallocFunction(void *currentPointer, sdword newSize, char *name, udword flags);
void *memAllocAttemptFunction(sdword length, char *name, udword flags);
#else
void *memReallocFunction(void *currentPointer, sdword newSize, udword flags);
void *memAllocAttemptFunction(sdword length, udword flags);
#endif
void memFree(void *pointer);
char *memStringDupe(char *string);
char *memStringDupeNV(char *string);

//utility functions (many stubbed out in retail builds)
#if MEM_ANALYSIS
void memAnalysisCreate(void);
#endif
#if MEM_USE_NAMES
sdword memNameSetFunction(memcookie *cookie, char *name);
#if MEM_SMALL_BLOCK_HEAP
sdword mbhNameSetFunction(mbhcookie *cookie, char *name);
#endif
sdword memNameSetLongFunction(memcookie *cookie, char *name);
void memRename(void *pointer, char *newName);
#else
#define memRename(p, n)
#endif //MEM_USE_NAMES
sdword memClearDword(void *dest, udword pattern, sdword nDwords);
char *memStrncpy(char *dest, char *source, sdword count);

#if MEM_ANALYSIS
sdword memFreeMemGet(mempool *pool);
#endif

#endif //___MEMORY_H

