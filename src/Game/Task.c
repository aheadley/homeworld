/*=============================================================================
    Name    : task.c
    Purpose : Functions for implementing a non-preemptive task manager.

    Created 6/18/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Types.h"
#include "Debug.h"
#include "Memory.h"
#include "Demo.h"
#include "TitanInterfaceC.h"
#include "Task.h"

/*=============================================================================
    Data:
=============================================================================*/
//task structures
taskdata *taskData[TSK_NumberTasks];            //pointers to active tasks
sdword taskMaxTask;                             //highest indexed task enabled
sdword taskModuleInit = FALSE;

CallBacks callbacks;

//speed of task timer, in Hz
real32 taskFrequency;

//global handle of task currently being executed
taskhandle taskCurrentTask = -1;

//global branch pointers
void *taskFunctionReturn = NULL;                //location returned to from task functions
void *taskFunctionExit = NULL;                  //location branched to for exiting task functions
void *taskFunctionContinue = NULL;              //location branched to for returning from a task function

//saved context for task calling
static udword taskESISave;
static udword taskEDISave;
static udword taskESPSave;
static udword taskEBPSave;
static udword taskEBXSave;

//data and entry points for taskStackSave/Restore
void *taskStackSaveEntry = NULL;                //location branched to for taskStackSave()
void *taskStackRestoreEntry = NULL;             //location branched to for taskStackSave()
udword taskStackSaveESP;                        //stack registers are saved here
udword taskStackSaveEBP;
udword taskStackSaveESI;                        //temporary ESI/EDI save
udword taskStackSaveEDI;
sdword taskStackSaveDWORDS;                     //number of extra dwords to save
sdword taskStackSaveDWORDSParameter;            //number of extra dwords to save

real32 taskTimeDelta   = 0.0f;
real32 taskTimeElapsed = 0.0f;                  //time elapsed, in seconds, since program started

#if TASK_STACK_SAVE_HACK
udword taskSaveRestoreHackIndex;
#endif

udword taskProcessIndex = 0;                    //number of calls being executed on this task
sdword taskNumberCalls;                         //number of times to call the task

#if TASK_STACK_SAVE
sdword taskLargestLocals = 0;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for these functions)
/*-----------------------------------------------------------------------------
    Test task function
-----------------------------------------------------------------------------*/
#if TASK_TEST
void taskTestFunction(void)
{
    register sdword index;
    register sdword aba;

    index = 300;
    aba = 24;

    taskYield(0);

    for (index = 0; index < 4; index++)
    {
        dbgMessagef("\nTask test function called %d times.", index);
        aba++;
        taskYield(0);
    }
    taskExit();
}
#endif //TASK_TEST

/*-----------------------------------------------------------------------------
    Name        : taskStartup
    Description : Initialize the task module.
    Inputs      : frequency - frequency, in Hz of the timer to be used.
    Outputs     : All taskData structures cleared
    Return      : OKAY on success, ERROR if failure
----------------------------------------------------------------------------*/
sdword taskStartup(udword frequency)
{
    sdword index;

#if TASK_VERBOSE_LEVEL >= 1
    dbgMessagef("\ntaskInit: Task module started using a frequency of %dHz", frequency);
#endif

    dbgAssert(taskModuleInit == FALSE);

    for (index = 0; index < TSK_NumberTasks; index++)
    {
        taskData[index] = NULL;
    }

    taskMaxTask = 0;                                        //no tasks
    taskFrequency = (real32)frequency;                      //save the frequency
    taskModuleInit = TRUE;                                  //and say module init

    //call the task handler dispatcher once so that it will set return pointers OK
    taskExecuteAllPending(0);

#if TASK_TEST                                               //test this crazy module
    {
        taskhandle handle;
        handle = taskStart(taskTestFunction, 1, TF_OncePerFrame);
        taskRename(handle, "Task this you commie bastards!");
        taskExecuteAllPending(1);
        taskExecuteAllPending(1);
        taskExecuteAllPending(1);
        taskExecuteAllPending(1);
        taskExecuteAllPending(1);
        taskExecuteAllPending(1);
    }
#endif //TASK_TEST

#if TASK_STACK_SAVE_HACK
    taskSaveRestoreHackIndex = 0;
#endif

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : taskShutdown
    Description : Close the task manager by clearing all tasks.
    Inputs      : void
    Outputs     : Clears all task structures to unused and frees stack memory (if any)
    Return      : void
----------------------------------------------------------------------------*/
void taskShutdown(void)
{
    sdword index;

#if TASK_VERBOSE_LEVEL >= 1
    dbgMessage("\ntaskClose: closing task module");
#endif

    if (!taskModuleInit)
    {
        return;
    }
    for (index = 0; index < taskMaxTask; index++)
    {
        if (taskData[index] != NULL)
        {
            memFree(taskData[index]);                       //free structure and stack associated with this task
            taskData[index] = NULL;
        }
    }
    taskModuleInit = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : taskPointerAlloc
    Description : Allocate a task pointer out of global task data list
    Inputs      : void
    Outputs     : sets allocated task pointer to -1
    Return      : index of newly allocated task
    Note        : generates a fatal error if no task found
----------------------------------------------------------------------------*/
sdword taskPointerAlloc(void)
{
    sdword index;

    for (index = 0; index < TSK_NumberTasks; index++)       //look for a free task
    {
        if (taskData[index] == NULL)                        //if free
        {
            taskData[index] = (void *)0xffffffff;
            taskMaxTask = max(taskMaxTask, index + 1);      //update max task if needed
            return(index);
        }
    }

#if TASK_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "All %d task pointers in use.", taskMaxTask);
#endif
    return(ERROR);
}

/*-----------------------------------------------------------------------------
    Name        : taskStart
    Description : Start a specific task
    Inputs      : function - entry point of task
                  period - period between consecutive calls
                  stacksize - size of task's local stack
                  flags - control execution of task
    Outputs     : taskdata structure allocated and initialized
    Return      : handle to task for later manipulation of task
    Note        : if a taskdata structure or stack RAM cannot be allocated, the
                    function will generate a fatal error.
----------------------------------------------------------------------------*/
taskhandle taskStart(taskfunction function, real32 period, udword flags)
{
    static taskhandle handle = ERROR;
    taskdata *newTask;
    static void *taskFunctionContinueSave;
    static udword taskESISave;
    static udword taskEDISave;
    static udword taskESPSave;
    static udword taskEBPSave;
    static udword taskEBXSave;
#if TASK_STACK_SAVE
    static udword taskESPSaveInitial;
    sdword taskESPDiff;
#endif

    taskInitCheck();
    dbgAssert(function != NULL);
    dbgAssert(period > 0.0f);

    //allocate a new task and it's stack in one fell swoop
    newTask = memAlloc(sizeof(taskdata), "taskData", NonVolatile);
    handle = taskPointerAlloc();                            //alloc and save the
    taskData[handle] = newTask;                             //newest task pointer

#if TASK_VERBOSE_LEVEL >= 2
    dbgMessagef("\ntaskStart: starting task at 0x%x at %d Hz, flags 0x%x using handle %d at 0x%x",
               function, 1.0f/period, flags, handle, taskData[handle]);
#endif

    newTask->flags = flags | TF_Allocated;                  //make task in use and running
    newTask->function = function;
    newTask->ticks = 0;                                     //no residual ticks
    dbgAssert(period * taskFrequency < (real32)SDWORD_Max);
    newTask->ticksPerCall = (udword)(period * taskFrequency);//save ticks per call

#if DBG_STACK_CONTEXT
#ifndef C_ONLY
#if defined (_MSC_VER)
    _asm
    {
        mov     eax, esp
        mov     dbgStackBase, eax
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    movl %%esp, %%eax\n"
        "    movl %%eax, %0\n"
        : "=m" (dbgStackBase)
        :
        : "eax" );
#endif
#endif // C_ONLY
#endif//DBG_STACK_CONTEXT
    //now that stack is created OK, let's call it's handler function to start it
    taskFunctionContinueSave = taskFunctionContinue;
#ifndef C_ONLY
#if defined (_MSC_VER)
    _asm
    {
        mov     eax, OFFSET taskContinued
        mov     [taskFunctionContinue], eax                 //set point to continue from
    }
    //taskCurrentTask = handle;
    _asm
    {
        mov     eax, esi
        mov     [taskESISave], eax                          //save esi,edi
        mov     eax, edi
        mov     [taskEDISave], eax

        mov     eax, esp
        mov     [taskESPSave], eax
#if TASK_STACK_SAVE
        mov     [taskESPSaveInitial], eax
#endif

        mov     eax, ebp
        mov     [taskEBPSave], eax

        mov     eax, ebx
        mov     [taskEBXSave], eax
        mov     eax, function

        jmp     eax                                         //call the function
    }
taskContinued:
    _asm
    {
        mov     edx, handle                                 //get edx->taskdata structure
        shl     edx, 2
        add     edx, OFFSET taskData
        mov     edx, [edx]

        pop     eax                                         //get return IP from stack (jumped here via a CALL)
        mov     [edx + TOF_Context + 24], eax               //eip
        mov     eax, ebx
        mov     [edx + TOF_Context +  0], eax               //ebx
        mov     eax, ecx
        mov     [edx + TOF_Context +  4], eax               //ecx
        mov     eax, edi
        mov     [edx + TOF_Context +  8], eax               //edi
        mov     eax, esi
        mov     [edx + TOF_Context + 12], eax               //esi
        mov     eax, esp
        mov     [edx + TOF_Context + 16], eax               //esp
        mov     eax, ebp
        mov     [edx + TOF_Context + 20], eax               //ebp
    }
    _asm
    {
        mov     eax, [taskESISave]                          //restore esi,edi,ebp,esp
        mov     esi, eax
        mov     eax, [taskEDISave]
        mov     edi, eax
        mov     eax, [taskESPSave]
        mov     esp, eax
        mov     eax, [taskEBPSave]
        mov     ebp, eax
        mov     eax, [taskEBXSave]
        mov     ebx, eax
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    movl $(taskStart_taskContinued), %%eax\n"      /*set point to continue from*/
        "    movl %%eax, %0\n"
        : "=m" (taskFunctionContinue)
        :
        : "eax" );

        /*taskCurrentTask = handle;*/

    __asm__ __volatile__ (
        "    movl %%esi, %%eax\n"
        "    movl %%eax, %0\n"                              /*save esi,edi*/
        "    movl %%edi, %%eax\n"
        "    movl %%eax, %1\n"

        "    movl %%esp, %%eax\n"
        "    movl %%eax, %2\n"
#if TASK_STACK_SAVE
        "    movl %%eax, %8\n"
#endif

        "    movl %%ebp, %%eax\n"
        "    movl %%eax, %3\n"

        "    movl %%ebx, %%eax\n"
        "    movl %%eax, %4\n"
        "    movl %5, %%eax\n"

        "    jmp  *%%eax\n"                                 /*call the function*/

        "taskStart_taskContinued:\n"
        "    movl %6, %%edx\n"                              /*get edx->taskdata structure*/
        "    shll $2, %%edx\n"
        "    addl %7, %%edx\n"
        "    movl (%%edx), %%edx\n"

        "    popl %%eax\n"                                  /*get return IP from stack (jumped here via a CALL)*/
        "    movl %%eax, "TOF_Context_STR"+24(%%edx)\n"     /*eip*/
        "    movl %%ebx, %%eax\n"
        "    movl %%eax, "TOF_Context_STR"+0(%%edx)\n"      /*ebx*/
        "    movl %%ecx, %%eax\n"
        "    movl %%eax, "TOF_Context_STR"+4(%%edx)\n"      /*ecx*/
        "    movl %%edi, %%eax\n"
        "    movl %%eax, "TOF_Context_STR"+8(%%edx)\n"      /*edi*/
        "    movl %%esi, %%eax\n"
        "    movl %%eax, "TOF_Context_STR"+12(%%edx)\n"     /*esi*/
        "    movl %%esp, %%eax\n"
        "    movl %%eax, "TOF_Context_STR"+16(%%edx)\n"     /*esp*/
        "    movl %%ebp, %%eax\n"
        "    movl %%eax, "TOF_Context_STR"+20(%%edx)\n"     /*ebp*/

        "    movl %0, %%eax\n"                          /*restore esi,edi,ebp,esp*/
        "    movl %%eax, %%esi\n"
        "    movl %1, %%eax\n"
        "    movl %%eax, %%edi\n"
        "    movl %2, %%eax\n"
        "    movl %%eax, %%esp\n"
        "    movl %3, %%eax\n"
        "    movl %%eax, %%ebp\n"
        "    movl %4, %%eax\n"
        "    movl %%eax, %%ebx\n"
        :
        : "m" (taskESISave), "m" (taskEDISave), "m" (taskESPSave),
          "m" (taskEBPSave), "m" (taskEBXSave),
          "m" (function), "m" (handle), "m" (taskData)
#if TASK_STACK_SAVE
          , "m" (taskESPSaveInitial)
#endif
        : "eax", "edx" );
#else
#error Function uses inline x86 assembly.
#endif

#else  // C_ONLY
	function();  // calling the taskfunction 'function' pointer argument that this function was given
#endif // C_ONLY

#if TASK_STACK_SAVE
    taskESPDiff = (sdword)(taskESPSaveInitial - taskData[handle]->esp);
    dbgAssert(taskESPDiff >= 0);
    dbgAssert(taskESPDiff < BIT8);
    if (taskESPDiff > 0)
    {
        taskLargestLocals = max(taskLargestLocals, taskESPDiff);
        taskData[handle] = memRealloc(taskData[handle], sizeof(taskdata) + taskESPDiff, "task+Stack", NonVolatile);
        memcpy((ubyte *)(taskData[handle] + 1), ((ubyte *)taskData[handle]->esp), taskESPDiff);//save the extra stack stuff
    }
    taskData[handle]->nBytesStack = taskESPDiff;
#endif //TASK_STACK_SAVE
    //taskCurrentTask = -1;
    taskFunctionContinue = taskFunctionContinueSave;
    return(handle);
}

/*-----------------------------------------------------------------------------
    Name        : taskStop
    Description : Destroy the specified task (stop and deallocate)
    Inputs      : handle - handle of task returned from taskStart()
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void taskStop(taskhandle handle)
{
    taskInitCheck();
    dbgAssert(handle >= 0);
    dbgAssert(handle < taskMaxTask);
    dbgAssert(taskData[handle] != NULL);

#if TASK_VERBOSE_LEVEL >= 2
//    dbgMessagef("\ntaskDestroy: destroying task %d", handle);
#endif

    memFree(taskData[handle]);                              //free the memory
    taskData[handle] = NULL;                                //kill the task
    if (handle == taskMaxTask - 1)                          //if freeing the max task
    {
        taskMaxTask--;
    }
}

/*-----------------------------------------------------------------------------
    Name        : taskExecuteAllPending
    Description : Execute all pending tasks
    Inputs      : ticks - number of system timer ticks since the last time called
    Outputs     : all pending tasks executed
    Return      : ?
    Note        : Because the stack is corrupted in this function, we can use
        no local variables or passed parameters.
----------------------------------------------------------------------------*/
static sdword tTicks, tLocalTicks;
sdword taskExecuteAllPending(sdword ticks)
{
#if TASK_STACK_SAVE
    static ubyte localStack[BIT8];
    static ubyte *currentESP;
    static udword localStackSize;
    static udword biggestStackSize;
#endif
#if defined (__GNUC__) // && defined (__i386__)
    /* See "taskExec_taskReturned" assembly block for an explanation. */
    static ubyte continue_hack;
    continue_hack = 1;
#endif

/*
#if TASK_STACK_CHECKING                                     //set the stack underflow detection cookie
    udword validationCookie;
#endif
*/
#if TASK_VERBOSE_LEVEL >= 2
    if (ticks > 0)
    {
        dbgMessagef("\ntaskExecuteAllPending: executing tasks for %d ticks", ticks);
    }
#endif

#if DBG_STACK_CONTEXT
#ifndef C_ONLY
#if defined (_MSC_VER)
    _asm
    {
        mov     eax, esp
        mov     dbgStackBase, eax
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    movl %%esp, %%eax\n"
        "    movl %%eax, %0\n"
        : "=m" (dbgStackBase)
        :
        : "eax" );
#endif
#endif // C_ONLY
#endif//DBG_STACK_CONTEXT
    if (taskModuleInit == FALSE)
    {   //don't do a taskInitCheck because this sometimes gets called on exiting,
        //after all the tasks are shut down.
        return(OKAY);
    }
    //taskInitCheck();

    if (demDemoRecording)
    {
        demNumberTicksSave(ticks);
    }
    else if (demDemoPlaying)
    {
        ticks = demNumberTicksLoad(ticks);
    }

    taskTimeDelta = ticks / taskFrequency;
    taskTimeElapsed += taskTimeDelta;
    tTicks = ticks;                                         //save the ticks parameter

    dbgAssert(taskCurrentTask == -1);                       //verify we're not in any task currently

    //save the global task execution pointers
#ifndef C_ONLY
#if defined (_MSC_VER)
    _asm
    {
        mov     eax, OFFSET taskContinued
        mov     [taskFunctionContinue], eax
        mov     eax, OFFSET taskReturned
        mov     [taskFunctionReturn], eax
        mov     [taskFunctionExit], eax
    }
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "    movl $(taskExec_taskContinued), %%eax\n"
        "    movl %%eax, %0\n"
        "    movl $(taskExec_taskReturned), %%eax\n"
        "    movl %%eax, %1\n"
        "    movl %%eax, %2\n"
        : "=m" (taskFunctionContinue), "=m" (taskFunctionReturn),
          "=m" (taskFunctionExit)
        :
        : "eax" );
#endif
#endif // C_ONLY
#if TASK_STACK_SAVE
    //save a chunk of the stack that might get overwritten by some of the tasks
    biggestStackSize = taskLargestLocals;
    if (biggestStackSize > 0)
    {
#if defined (_MSC_VER)
        _asm
        {
            mov     [currentESP], esp
        }
#elif defined (__GNUC__) && defined (__i386__)
        __asm__ __volatile__ (
            "    movl %%esp, %0\n"
            : "=m" (currentESP) );
#endif
        memcpy(localStack, currentESP, biggestStackSize);
    }
#endif // TASK_STACK_SAVE

    for (taskCurrentTask = 0; taskCurrentTask < taskMaxTask; taskCurrentTask++)
    {
        if (taskData[taskCurrentTask] != NULL)              //if this task is enabled
        {
            if (bitTest(taskData[taskCurrentTask]->flags, TF_Paused))//don't process if paused
                continue;

            if (TitanActive)
                titanPumpEngine();

            //calculate number of calls
            if (bitTest(taskData[taskCurrentTask]->flags, TF_OncePerFrame))
            {                                               //if not real-time but frame-time
// Don't do this if check, because if you do it is possible that hundreds of other tasks will get executed
// and this task will starve when the game AI gets heavy.  Luke, later check if there's a better way to do this.
/*
                if (taskData[taskCurrentTask]->ticks == 0)  //see if time to execute
                {
*/
                    taskNumberCalls = 1;                             //flag to call once
                    taskData[taskCurrentTask]->ticks = taskData[taskCurrentTask]->ticksPerCall;
/*
                }
*/
                taskData[taskCurrentTask]->ticks--;
            }
            else
            {                                               //else it's a real-time task
                tLocalTicks = tTicks + taskData[taskCurrentTask]->ticks;//add leftover ticks from last frame
                taskData[taskCurrentTask]->ticks = tLocalTicks %//save leftover ticks for next frame
                    taskData[taskCurrentTask]->ticksPerCall;
                taskNumberCalls = tLocalTicks / taskData[taskCurrentTask]->ticksPerCall;//compute number of calls for this frame
#if TASK_MAX_TICKS
                taskNumberCalls = min(taskNumberCalls, TSK_MaxTicks);         //make sure not too many calls
#endif //TASK_MAX_TICKS
            }
#if TASK_VERBOSE_LEVEL >= 3
            if (taskNumberCalls)
            {
                dbgMessagef("\ntaskExecuteAllPending: executing %d calls of task handle %d", taskNumberCalls, taskCurrentTask);
            }
#endif
            //!!! needed?
            //save the current register context
#ifndef C_ONLY
#if defined (_MSC_VER)
            _asm
            {
                mov     eax, esi
                mov     [taskESISave], eax                  //save esi,edi,esp,ebp
                mov     eax, edi
                mov     [taskEDISave], eax
                mov     eax, esp
                mov     [taskESPSave], eax
                mov     eax, ebp
                mov     [taskEBPSave], eax
                mov     eax, ebx
                mov     [taskEBXSave], eax
            }
#elif defined (__GNUC__) && defined (__i386__)
            __asm__ __volatile__ (
                "    movl %%esi, %%eax\n"
                "    movl %%eax, %0\n"                      /*save esi,edi,esp,ebp*/
                "    movl %%edi, %%eax\n"
                "    movl %%eax, %1\n"
                "    movl %%esp, %%eax\n"
                "    movl %%eax, %2\n"
                "    movl %%ebp, %%eax\n"
                "    movl %%eax, %3\n"
                "    movl %%ebx, %%eax\n"
                "    movl %%eax, %4\n"
                : "=m" (taskESISave), "=m" (taskEDISave), "=m" (taskESPSave),
                  "=m" (taskEBPSave), "=m" (taskEBXSave)
                :
                : "eax" );
#endif
#endif // C_ONLY
            taskProcessIndex++;
#if TASK_STACK_SAVE
            localStackSize = taskData[taskCurrentTask]->nBytesStack;
            if (localStackSize > 0)
            {                                               //if this task uses some local stack
                static sdword count;
                static udword *src, *dest;

                dest = (udword *)taskData[taskCurrentTask]->esp;
                src = (udword *)(taskData[taskCurrentTask] + 1);
                for (count = localStackSize / 4; count > 0; count--, src++, dest++)
                {
                    *dest = *src;
                }
                //memcpy(((ubyte *)taskData[taskCurrentTask]->esp), (ubyte *)(taskData[taskCurrentTask] + 1), localStackSize);
            }
#endif //TASK_STACK_SAVE
            for (; taskNumberCalls > 0; taskNumberCalls--)
            {
/*
#if TASK_STACK_CHECKING                                     //set the stack underflow detection cookie
                validationCookie = TSK_StackValidation + taskCurrentTask;
#endif
*/
#ifndef C_ONLY
#if defined (_MSC_VER)
                _asm
                {
                    //restore register context from the taskdata structure

                    mov     edx, taskCurrentTask            //get edx->taskdata structure
                    shl     edx, 2
                    add     edx, OFFSET taskData
                    mov     eax, [edx]

                    mov     edx, [eax + TOF_Context +  0]   //ebx
                    mov     ebx, edx
                    mov     edx, [eax + TOF_Context +  4]   //ecx
                    mov     ecx, edx
                    mov     edx, [eax + TOF_Context +  8]   //edi
                    mov     edi, edx
                    mov     edx, [eax + TOF_Context + 12]   //esi
                    mov     esi, edx
                    mov     edx, [eax + TOF_Context + 16]   //esp
                    mov     esp, edx
                    mov     edx, [eax + TOF_Context + 20]   //ebp
                    mov     ebp, edx

                    mov     edx, [eax + TOF_Context + 24]   //eip
                    jmp     edx                             //jump to the task
                }
taskContinued:
                //save register context to the taskdata structure
                _asm
                {
                    mov     edx, taskCurrentTask            //get edx->taskdata structure
                    shl     edx, 2
                    add     edx, OFFSET taskData
                    mov     edx, [edx]

                    pop     eax                             //get return IP from stack (jumped here via a CALL)
                    mov     [edx + TOF_Context + 24], eax   //eip
                    mov     eax, ebx
                    mov     [edx + TOF_Context +  0], eax   //ebx
                    mov     eax, ecx
                    mov     [edx + TOF_Context +  4], eax   //ecx
                    mov     eax, edi
                    mov     [edx + TOF_Context +  8], eax   //edi
                    mov     eax, esi
                    mov     [edx + TOF_Context + 12], eax   //esi
//#if TASK_STACK_CHECKING
                    mov     eax, esp
                    mov     [edx + TOF_Context + 16], eax   //esp
                    mov     eax, ebp
                    mov     [edx + TOF_Context + 20], eax   //ebp
//#endif
                }
#elif defined (__GNUC__) && defined (__i386__)
                __asm__ __volatile__ (
                         /*restore register context from the taskdata structure*/

                    "    movl %0, %%edx\n"                  /*get edx->taskdata structure*/
                    "    shll $2, %%edx\n"
                    "    addl %1, %%edx\n"
                    "    movl (%%edx), %%eax\n"

                    "    movl "TOF_Context_STR"+0(%%eax), %%edx\n"   /*ebx*/
                    "    movl %%edx, %%ebx\n"
                    "    movl "TOF_Context_STR"+4(%%eax), %%edx\n"   /*ecx*/
                    "    movl %%edx, %%ecx\n"
                    "    movl "TOF_Context_STR"+8(%%eax), %%edx\n"   /*edi*/
                    "    movl %%edx, %%edi\n"
                    "    movl "TOF_Context_STR"+12(%%eax), %%edx\n"  /*esi*/
                    "    movl %%edx, %%esi\n"
                    "    movl "TOF_Context_STR"+16(%%eax), %%edx\n"  /*esp*/
                    "    movl %%edx, %%esp\n"
                    "    movl "TOF_Context_STR"+20(%%eax), %%edx\n"  /*ebp*/
                    "    movl %%edx, %%ebp\n"

                    "    movl "TOF_Context_STR"+24(%%eax), %%edx\n"  /*eip*/
                    "    jmp *%%edx\n"                      /*jump to the task*/

                    "taskExec_taskContinued:\n"
                         /*save register context to the taskdata structure*/
                    "    movl %0, %%edx\n"                  /*get edx->taskdata structure*/
                    "    shll $2, %%edx\n"
                    "    addl %1, %%edx\n"
                    "    movl (%%edx), %%edx\n"

                    "    popl %%eax\n"                      /*get return IP from stack (jumped here via a CALL)*/
                    "    movl %%eax, "TOF_Context_STR"+24(%%edx)\n"  /*eip*/
                    "    movl %%ebx, %%eax\n"
                    "    movl %%eax, "TOF_Context_STR"+0(%%edx)\n"   /*ebx*/
                    "    movl %%ecx, %%eax\n"
                    "    movl %%eax, "TOF_Context_STR"+4(%%edx)\n"   /*ecx*/
                    "    movl %%edi, %%eax\n"
                    "    movl %%eax, "TOF_Context_STR"+8(%%edx)\n"   /*edi*/
                    "    movl %%esi, %%eax\n"
                    "    movl %%eax, "TOF_Context_STR"+12(%%edx)\n"  /*esi*/
/*#if TASK_STACK_CHECKING*/
                    "    movl %%esp, %%eax\n"
                    "    movl %%eax, "TOF_Context_STR"+16(%%edx)\n"  /*esp*/
                    "    movl %%ebp, %%eax\n"
                    "    movl %%eax, "TOF_Context_STR"+20(%%edx)\n"  /*ebp*/
/*#endif*/
                    :
                    : "m" (taskCurrentTask), "m" (taskData)
                    : "eax", "edx" );
#endif

#else  // C_ONLY

	// Call current task function
	taskData[taskCurrentTask]->function();

#endif // C_ONLY

/*
#if TASK_STACK_CHECKING
                if (taskData[taskCurrentTask]->esp < taskESPSave)
                {                                           //check for stack underflow
                    dbgFatalf(DBG_Loc, "Stack underflow: 0x%x < 0x%x handle %d",
                              taskData[taskCurrentTask]->esp,
                              taskESPSave,
                              taskCurrentTask);
                }
#endif //TASK_STACK_CHECKING
*/
            }
            //!!! needed?
#ifndef C_ONLY
#if defined (_MSC_VER)
            _asm
            {
                mov     eax, [taskESISave]                  //restore esi,edi,ebp,esp
                mov     esi, eax
                mov     eax, [taskEDISave]
                mov     edi, eax
                mov     eax, [taskESPSave]
                mov     esp, eax
                mov     eax, [taskEBPSave]
                mov     ebp, eax
                mov     eax, [taskEBXSave]
                mov     ebx, eax
            }
#elif defined (__GNUC__) && defined (__i386__)
            __asm__ __volatile__ (
                "    movl %0, %%eax\n"                      /*restore esi,edi,ebp,esp*/
                "    movl %%eax, %%esi\n"
                "    movl %1, %%eax\n"
                "    movl %%eax, %%edi\n"
                "    movl %2, %%eax\n"
                "    movl %%eax, %%esp\n"
                "    movl %3, %%eax\n"
                "    movl %%eax, %%ebp\n"
                "    movl %4, %%eax\n"
                "    movl %%eax, %%ebx\n"
                :
                : "m" (taskESISave), "m" (taskEDISave), "m" (taskESPSave),
                  "m" (taskEBPSave), "m" (taskEBXSave)
                : "eax" );
#endif
#endif // C_ONLY
#if TASK_STACK_SAVE
            if (localStackSize > 0)
            {                                           //if this task uses some local stack
                static sdword count;
                static udword *src, *dest;

                src = (udword *)taskData[taskCurrentTask]->esp;
                dest = (udword *)(taskData[taskCurrentTask] + 1);
                for (count = localStackSize / 4; count > 0; count--, src++, dest++)
                {
                    *dest = *src;
                }
                //memcpy((ubyte *)(taskData[taskCurrentTask] + 1), ((ubyte *)taskData[taskCurrentTask]->esp), localStackSize);
            }
#endif //TASK_STACK_SAVE
#ifndef C_ONLY
#if defined (_MSC_VER)
            continue;
taskReturned:
            _asm
            {
                pop     eax                                 //remove the IP from the stack because it got here from a CALL
                mov     eax, [taskESISave]                  //restore esi,edi,ebp,esp
                mov     esi, eax
                mov     eax, [taskEDISave]
                mov     edi, eax
                mov     eax, [taskESPSave]
                mov     esp, eax
                mov     eax, [taskEBPSave]
                mov     ebp, eax
                mov     eax, [taskEBXSave]
                mov     ebx, eax
            }
#elif defined (__GNUC__) && defined (__i386__)
            /* The (unconditional) "continue" statement which immediately
               preceded this block caused GCC to remove all the code following
               it up to the end of the block, so we need to hack in a way to
               force GCC not to do so...bleh... */
            if (continue_hack)
                continue;
            __asm__ __volatile__ (
                "\ntaskExec_taskReturned:\n"
                "    popl %%eax\n"                          /*remove the IP from the stack because it got here from a CALL*/
                "    movl %0, %%eax\n"                      /*restore esi,edi,ebp,esp*/
                "    movl %%eax, %%esi\n"
                "    movl %1, %%eax\n"
                "    movl %%eax, %%edi\n"
                "    movl %2, %%eax\n"
                "    movl %%eax, %%esp\n"
                "    movl %3, %%eax\n"
                "    movl %%eax, %%ebp\n"
                "    movl %4, %%eax\n"
                "    movl %%eax, %%ebx\n"
                :
                : "m" (taskESISave), "m" (taskEDISave), "m" (taskESPSave),
                  "m" (taskEBPSave), "m" (taskEBXSave)
                : "eax" );
#endif
#else  // C_ONLY
			if (continue_hack)
                continue;
#endif // C_ONLY

            taskStop(taskCurrentTask);                      //kill the task
        }
    }
    taskCurrentTask = -1;                                   //not currently executing any tasks
#if TASK_STACK_SAVE
    //restore a chunk of the stack that might have been overwritten by some of the tasks
    if (biggestStackSize > 0)
    {
        memcpy(currentESP, localStack, biggestStackSize);
    }
#endif
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : taskPause
    Description : Pauses selected task.
    Inputs      : handle - handle of task
    Outputs     : Sets the paused bit of specified taskData structure
    Return      : void
----------------------------------------------------------------------------*/
void taskPause(taskhandle handle)
{
    taskInitCheck();
    dbgAssert(handle >= 0);
    dbgAssert(handle < taskMaxTask);
    dbgAssert(taskData[handle] != NULL);

    bitSet(taskData[handle]->flags, TF_Paused);
}

/*-----------------------------------------------------------------------------
    Name        : taskResume
    Description : Resumes selected task.
    Inputs      : handle - handle of task
    Outputs     : Clears the paused bit of specified taskData structure
    Return      : void
----------------------------------------------------------------------------*/
void taskResume(taskhandle handle)
{
    taskInitCheck();
    dbgAssert(handle >= 0);
    dbgAssert(handle < taskMaxTask);
    dbgAssert(taskData[handle] != NULL);

    bitClear(taskData[handle]->flags, TF_Paused);
}

/*-----------------------------------------------------------------------------
    Name        : taskSavePauseStatus
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void taskSavePauseStatus(void)
{
    sdword index;

    taskInitCheck();
    for (index = 0; index < taskMaxTask; index++)           //for all tasks
    {
        if (taskData[index] != NULL)                        //if task active
        {
            if (bitTest(taskData[index]->flags, TF_Paused)) //save the pause bit
            {
                bitSet(taskData[index]->flags, TF_PauseSave);
            }
            else
            {
                bitClear(taskData[index]->flags, TF_PauseSave);
            }
            // deliberately do not pause task, just make sure TF_PauseSave gets set correctly
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : taskFreezeAll
    Description : Freeze all tasks.
    Inputs      : void
    Outputs     : Copies paused bit into pause save bit, then sets pause bit
                    of all active tasks.
    Return      : void
----------------------------------------------------------------------------*/
void taskFreezeAll(void)
{
    sdword index;

    taskInitCheck();
    for (index = 0; index < taskMaxTask; index++)           //for all tasks
    {
        if (taskData[index] != NULL)                        //if task active
        {
            if (bitTest(taskData[index]->flags, TF_Paused)) //save the pause bit
            {
                bitSet(taskData[index]->flags, TF_PauseSave);
            }
            else
            {
                bitClear(taskData[index]->flags, TF_PauseSave);
            }
            bitSet(taskData[index]->flags, TF_Paused);      //pause this task
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : taskResumesAll
    Description : Resumes all previously unpaused tasks.
    Inputs      : void
    Outputs     : Copies pause save bit into paused bit.
    Return      : void
    Note        : It is up to the calling party to ensure that the next call to
                    taskExecuteAllPending() does not try to execute all the task
                    ticks which would have happended while the tasks were paused.
----------------------------------------------------------------------------*/
void taskResumeAll(void)
{
    sdword index;

    taskInitCheck();
    for (index = 0; index < taskMaxTask; index++)           //for all tasks
    {
        if (taskData[index] != NULL)                        //if task active
        {
            if (bitTest(taskData[index]->flags, TF_PauseSave)) //save the pause bit
            {
                bitSet(taskData[index]->flags, TF_Paused);
            }
            else
            {
                bitClear(taskData[index]->flags, TF_Paused);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : taskPeriodSet
    Description : Set frequency for task
    Inputs      : handle - handle of task to adjust
                  period - period of task
    Outputs     :
    Return      : old period
----------------------------------------------------------------------------*/
real32 taskPeriodSet(taskhandle handle, real32 period)
{
    real32 old;

    taskInitCheck();
    dbgAssert(handle >= 0);
    dbgAssert(handle < taskMaxTask);
    dbgAssert(taskData[handle] != NULL);
    old = (real32)taskData[handle]->ticksPerCall / taskFrequency;
    taskData[handle]->ticksPerCall = (udword)(period * taskFrequency);
    return(old);
}

////////////////
///  CALLBACK TASK HANDLING
////////////////

taskhandle babyTaskHandle = -1;

void taskCallBackProcess(void);
void taskCallBackInit()
{
    listInit(&callbacks.babies);        //initialize linked list of babies
    babyTaskHandle = taskStart(taskCallBackProcess, BABY_CallBackPeriod, 0);
}
void taskCallBackShutDown()
{
    if (babyTaskHandle != -1)
    {
        taskStop(babyTaskHandle);
    }

    listDeleteAll(&callbacks.babies);
}
BabyCallBack *taskCallBackRegister(babyFuncCB callback, udword num, void *data, real32 callintime)
{
    BabyCallBack *baby;
    //(maybe do check of time to see if it is volatile?)
    baby = memAlloc(sizeof(BabyCallBack), "BabyCallBack", Volatile);
    baby->babyCallBackFunc = callback;
    baby->numparm = num;
    baby->dataparm = data;
    baby->timeToExecute = callintime;
    baby->timeStarted = taskTimeElapsed;
    listAddNode(&callbacks.babies, &baby->babylink,baby);
    return(baby);
}
void taskCallBackRemove(BabyCallBack *babytogobyebye)
{
    BabyCallBack *baby;
    Node *babynode;

    babynode = callbacks.babies.head;
    while(babynode != NULL)
    {
        baby = (BabyCallBack *) listGetStructOfNode(babynode);
        if(baby == babytogobyebye)
        {
            //returned TRUE, so delete
            listDeleteNode(babynode);
            return;
        }
        babynode = babynode->next;
    }
}

void taskCallBackProcess(void)
{
    static BabyCallBack *baby;
    static Node *babynode,*tempnode;

    taskYield(0);

#ifndef C_ONLY
    while(1)
#endif
    {
        taskStackSaveCond(0);

        babynode = callbacks.babies.head;
        while(babynode != NULL)
        {
            baby = (BabyCallBack *) listGetStructOfNode(babynode);
            if(taskTimeElapsed - baby->timeStarted >= baby->timeToExecute)
            {
                //time elapsed so execute callback
                if((baby->babyCallBackFunc)(baby->numparm, baby->dataparm, baby))
                {
                    //returned TRUE, so delete
                    tempnode = babynode->next;
                    listDeleteNode(babynode);
                    babynode = tempnode;
                    continue;
                }
                //reset task time
                baby->timeStarted = taskTimeElapsed;
            }
            babynode = babynode->next;
        }
        //break;
        taskStackRestoreCond();
        taskYield(0);
    }
}

