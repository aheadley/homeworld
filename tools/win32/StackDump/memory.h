/*=============================================================================
    MEMORY.H:  Definitions for Homeworld memory management module.

    Created June 1997 by Luke Moloney
=============================================================================*/

#ifndef ___MEMORY_H
#define ___MEMORY_H

/*=============================================================================
    Switches:
=============================================================================*/
#ifdef HW_Debug

#define MEM_MODULE_TEST         0               //test the module
#define MEM_ERROR_CHECKING      1               //basic error checking
#define MEM_CLEAR_MEM           1               //clear newly allocated blocks to all clear code
#define MEM_CLEAR_MEM_ON_FREE   1               //clear newly freed blocks to all clear code
#define MEM_VERBOSE_LEVEL       1               //control verbose printing
#define MEM_USE_NAMES           1               //use names in allocated blocks
#define MEM_DEFRAGMENT_FREE     1               //attempt to join with adjacent free block(s) when freeing
#define MEM_ANALYSIS            1               //enable memory analysis functions

#else //HW_Debug

#define MEM_MODULE_TEST         0               //don't test the module
#define MEM_ERROR_CHECKING      0               //no error ckecking in retail
#define MEM_CLEAR_MEM           0               //don't clear any blocks to zeros
#define MEM_CLEAR_MEM_ON_FREE   0               //clear newly freed blocks to all clear code
#define MEM_VERBOSE_LEVEL       0               //don't print any verbose strings in retail
#define MEM_USE_NAMES           0               //no names please ma'am
#define MEM_DEFRAGMENT_FREE     1               //attempt to join with adjacent free block(s) when freeing
#define MEM_ANALYSIS            0               //disable memory analysis functions

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//memory block flags
#define MBF_AllocatedNext       1               //memory following block allocated
#define MBF_AllocatedPrev       2               //preceeding memory block allocated
#define MBF_Purgable            4               //will be purged on next call to memPurge()
#define MBF_VerifyMask          0xfffffff8      //remaining bits used for verification mask

//verification value
#define MBF_VerifyValue         (0xfa7babe5 & MBF_VerifyMask)

//max length for cookie name strings
#define MEM_NameLength         20

//allocation block granularity.
#define MEM_BlockSize           ((sdword)sizeof(memcookie))
#define MEM_BlocksPerCookie     1

//default memory heap size
#define MEM_HeapSizeDefault     (32 * 1024 * 1024)

//default name strings
#define MEM_NameHeap            "HeapLow(free)"
#define MEM_NameHeapFirst       "HeapLowFirst(Free)"
#define MEM_HeapFree            "(free)"

//memory clearing settings
#define MEM_ClearSetting        0xbadb00b5

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
    udword pad;                                 //round structure up to 16 bytes
#endif
    sdword blocksNext;                          //number of blocks to next cookie
    sdword blocksPrevious;                      //number of blocks to previous cookie
                                                //only valid when freeing memory
}
memcookie;

/*=============================================================================
    Macros:
=============================================================================*/
//name-specific stuff
#if MEM_USE_NAMES
#define memAlloc(l, n, f) memAllocFunction((l), (n), (f));
#define memAllocAttempt(l, n, f) memAllocFunctionA((l), (n), (f));
#define memNameSet(c, s)    memNameSetFunction((c), (s))
#define memNameSetLong(c, s)    memNameSetFunction((c), (s))
#else
#define memAlloc(l, n, f) memAllocFunction((l), (f));
#define memAllocAttempt(l, n, f) memAllocFunctionA((l), (f));
#define memNameSet(c, s)
#define memNameSetLong(c, s)
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
#else
#define memCookieVerify(c)
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

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown/reset memory module
sdword memInit(void *heapStart, sdword heapSize);
sdword memReset(void);
sdword memClose(void);

//allocate/free memory blocks
#if MEM_USE_NAMES
void *memAllocFunction(sdword length, char *name, udword flags);
void *memAllocFunctionA(sdword length, char *name, udword flags);
#else
void *memAllocFunction(sdword length, udword flags);
void *memAllocFunctionA(sdword length, udword flags);
#endif
sdword memFree(void *pointer);
char *memStringDupe(char *string);

//purge/cleanup memory blocks (time-consuming, don't call these often)
sdword memPurge(udword flags);
sdword memDefragment(void);

//utility functions (many stubbed out in retail builds)
#if MEM_ANALYSIS
sdword memFreeMemGet(void);
sdword memUsedMemGet(void);
sdword memLargestBlockGet(void);
void memAnalysisCreate(void);
#endif
#if MEM_USE_NAMES
sdword memNameSetFunction(memcookie *cookie, char *name);
sdword memNameSetLongFunction(memcookie *cookie, char *name);
void memRename(void *pointer, char *newName);
#else
#define memRename(p, n)
#endif //MEM_USE_NAMES
sdword memClearDword(void *dest, udword pattern, sdword nDwords);

#endif //___MEMORY_H

