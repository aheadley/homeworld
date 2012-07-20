/*=============================================================================
    Name    : task.h
    Purpose : Definitions for a preemptive task manager.

    Created 6/17/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___TASK_H
#define ___TASK_H

#include "Types.h"
#include "LinkedList.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define TASK_STACK_SAVE_HACK    0               //save the stack for Windows 95 baby
#define TASK_STACK_SAVE         0               //save local stack frame the task function may create

#ifndef HW_Release

#define TASK_ERROR_CHECKING     1               //general error checking
#define TASK_STACK_CHECKING     1               //stack corruption checks on entry/exit of tasks
#define TASK_VERBOSE_LEVEL      1               //print extra info
#define TASK_TEST               0               //test the task module
#define TASK_STACK_USAGE_MAX    1               //logs the maximum amount of stack used by task
#define TASK_MAX_TICKS          1               //certain maximum number of task ticks per frame

#else //HW_Debug

#define TASK_ERROR_CHECKING     0               //general error checking
#define TASK_STACK_CHECKING     0               //stack corruption checks on entry/exit of tasks
#define TASK_VERBOSE_LEVEL      0               //print extra info
#define TASK_TEST               0               //test the task module
#define TASK_STACK_USAGE_MAX    0               //logs the maximum amount of stack used by task
#define TASK_MAX_TICKS          1               //certain maximum number of task ticks per frame

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define TSK_NumberTasks         32              //maximum number of tasks
#define TSK_StackValidation     0xdeadbeef      //for verifying stack integrety
#define TSK_StackExtraHigh      4               //one dword high and one low
#define TSK_StackExtraLow       4               //one dword high and one low
#define TSK_StackExtra          8               //one dword high and one low

//task structure flags
#define TF_Allocated            1               //task in operation
#define TF_Paused               2               //task is paused
#define TF_OncePerFrame         4               //call once per frame as opposed to <N> Hz
//#define TF_OneOverFreq          8               //actual frequency is 1/frequency specified
#define TF_PauseSave            16              //saved pause bit for pausing all tasks

//task structure constants
#define TOF_Context             16                 //offset to start of saved context
#define TOF_Context_STR         "16"               /*string version*/

//taskStackSave/Restore definitions
#define TSK_StackSaveExtra      2               //normally save return address and ESP in C functions

//maximum number of task ticks per frame
#define TSK_MaxTicks            8

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef sdword taskhandle;
typedef void (*taskfunction)(void);

//structure for each task
typedef struct
{
    udword flags;                               //control flags for this task
    taskfunction function;                     //task handler entry point
//    ubyte *stackBase;                           //base (low) of stack allocated to this task
//    sdword stackLength;                         //size of stack
    udword ticks;                               //number of timer ticks (usually the remainder after a set of calls)
    udword ticksPerCall;                        //number of base timer ticks per call
    //saved context for task switches
    udword ebx;                                 //general-purpose registers
    udword ecx;
    udword edi;
    udword esi;
    udword esp;                                 //pointer registers
    udword ebp;
    udword eip;                                 //location to branch to for next round
#if TASK_STACK_SAVE
    udword nBytesStack;                         //amount of stack to be saved between updates
#endif
}
taskdata;

struct BabyCallBack;
typedef bool (*babyFuncCB)(udword num, void *data, struct BabyCallBack *baby);    //callback function

typedef struct BabyCallBack
{
    Node babylink;
    real32 timeStarted;
    real32 timeToExecute;
    udword numparm;
    void *dataparm;
    babyFuncCB babyCallBackFunc;
}
BabyCallBack;

typedef struct
{
    LinkedList babies;
}CallBacks;

#define BABY_CallBackPeriod   1.0f / 16.0f

extern CallBacks callbacks;

void taskCallBackInit();
void taskCallBackShutDown();
BabyCallBack *taskCallBackRegister(babyFuncCB callback, udword num, void *data, real32 callintime);
void taskCallBackRemove(BabyCallBack *babytogobyebye);
void taskCallBackProcess();

/*=============================================================================
    Data:
=============================================================================*/
//global branch pointers
extern void *taskFunctionReturn;                //location returned to from task functions
extern void *taskFunctionExit;                  //location branched to for exiting task functions
extern void *taskFunctionContinue;              //location branched to for returning from a task function
extern void *taskStackSaveEntry;                //location branched to for taskStackSave()
extern void *taskStackRestoreEntry;             //location branched to for taskStackSave()

extern sdword taskStackSaveDWORDS;              //number of extra dwords to save
extern sdword taskStackSaveDWORDSParameter;     //number of extra stack dwords to save

extern taskhandle taskCurrentTask;              //global handle of task currently being executed

//task structure pointers
extern taskdata *taskData[TSK_NumberTasks];     //pointers to active tasks

extern real32 taskTimeDelta;
extern real32 taskTimeElapsed;                  //time elapsed, in seconds, since program started

extern udword taskProcessIndex;                 //number of calls being executed on this task
extern sdword taskNumberCalls;                  //number of times to call the task

//speed of task timer, in Hz
extern real32 taskFrequency;

/*=============================================================================
    Macros:
=============================================================================*/
//if module not started properly, generate an error
#if TASK_ERROR_CHECKING
#define taskInitCheck()\
    if (taskModuleInit == FALSE)\
    {\
        dbgFatal(DBG_Loc, "Called before task module started or after closed.");\
    }
#else
#define taskInitCheck()
#endif

//macros for task continuation and exiting
#ifndef C_ONLY
#define taskYield(n)    ((void (*)(void))taskFunctionContinue)()
#define taskExit()     ((void (*)(void))taskFunctionExit)()
#else
#define taskYield(n)
#define taskExit()
#endif // C_ONLY

//rename the memory block associated with a task
#define taskRename(t, n)    memRename((void *)taskData[t], (n))

//save/restore the task local stack, optionally checking to see if in a task
#define taskStackSave(nDwords)
#define taskStackRestore()
#define taskStackSaveIf(nDwords)
#define taskStackRestoreIf()
//save/restore stack only in special hack build for Win95
#if TASK_STACK_SAVE_HACK
#define taskStackSaveCond(nDwords) taskStackSave(nDwords)
#define taskStackRestoreCond() taskStackRestore()
#else
#define taskStackSaveCond(nDwords)
#define taskStackRestoreCond()
#endif

//save/restore the task local stack only in debug builds
#ifndef HW_Release
#define taskStackSaveDebug(nDwords) taskStackSave(nDwords)
#define taskStackRestoreDebug() taskStackRestore()
#else
#define taskStackSaveDebug(nDwords)
#define taskStackRestoreDebug()
#endif

/*=============================================================================
    Functions:
=============================================================================*/
//start/close the task manager
sdword taskStartup(udword frequency);
void taskShutdown(void);

//start/pause/resume/exit/yield a specific task
taskhandle taskStart(taskfunction function, real32 period, udword flags);
void taskPause(taskhandle handle);
void taskResume(taskhandle handle);
void taskStop(taskhandle handle);

//freeze/resume all tasks
void taskFreezeAll(void);
void taskResumeAll(void);
void taskSavePauseStatus(void);

//execute all pending tasks
sdword taskExecuteAllPending(sdword ticks);

//adjust attributes of tasks
udword taskFrequencySet(taskhandle handle, udword frequency);
#endif // ___TASK_H
