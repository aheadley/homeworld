#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "SaveGame.h"
#include "AIVar.h"

AIVar **vars = NULL;
sdword varsAllocated = 0, varsUsed = 0;
sdword uniqueNum = 0;

AIVar *aivRenderMainScreen = NULL;      // special variable for rendering knowledge

void aivarStartup(void)
{
    vars = NULL;
    varsAllocated = 0;
    varsUsed = 0;
    uniqueNum = 0;

    aivRenderMainScreen = aivarCreate("RenderMainScreen");
}

void aivarShutdown(void)
{
    if (vars != NULL)
    {
        sdword i;
        for (i=0;i<varsUsed;i++)
        {
            dbgMessagef("\nWarning Var %s not closed",vars[i]->label);
            memFree(vars[i]);
        }
        memFree(vars);
        vars = NULL;
    }
    varsAllocated = 0;
    varsUsed = 0;

    aivRenderMainScreen = NULL;
}

//
//  create a new AIVar (with a unique label)
//
//  returns NULL if label isn't unique
//
AIVar *aivarCreate(char *label)
{
    AIVar *var;

    if (aivarFind(label))
        return NULL;

    // allocate the var
    var = memAlloc(sizeof(AIVar), "aivar", 0);
    aivarValueSet(var, 0); // default, just to be nice
    aivarLabelSet(var, label);

    // keep track of all allocations
    if (!vars)
    {
        vars = memAlloc(sizeof(AIVar*)*AIVAR_ALLOC_INITIAL, "aivarlist", 0);
        varsAllocated = AIVAR_ALLOC_INITIAL;
        varsUsed = 0;
    }
    if (varsUsed >= varsAllocated)
    {
        // allocate more if necessary
        vars = memRealloc(vars, sizeof(AIVar*)*(varsAllocated + AIVAR_ALLOC_INCREMENT), "aivarlist", 0);
        varsAllocated += AIVAR_ALLOC_INCREMENT;
    }
    vars[varsUsed++] = var;

    return var;
}

//
//  return pointer to var with given label
//
//  return NULL if not found
//
AIVar *aivarFind(char *label)
{
    sdword i = 0;

    while (i < varsUsed)
    {
        if (!strncmp(vars[i]->label, label, AIVAR_LABEL_MAX_LENGTH))
            return vars[i];
        ++i;
    }
    return NULL;
}

//
//  return pointer to var with given label, ignoring the <FSMLabel>. from the front of it
//
//  return NULL if not found
//
AIVar *aivarFindAnyFSM(char *label)
{
    sdword i = 0;
    char *dot;

    while (i < varsUsed)
    {
        if ((dot = strchr(vars[i]->label, '.')) != NULL)
        {                                                   //if there's a dot (created in some FSM)
            dot++;
            if (!strncmp(dot, label, AIVAR_LABEL_MAX_LENGTH - (dot - vars[i]->label)))
                return vars[i];
        }
        else
        {                                                   //else regular var search
            if (!strncmp(vars[i]->label, label, AIVAR_LABEL_MAX_LENGTH))
                return vars[i];
        }
        ++i;
    }
    return NULL;
}

void aivarDestroy(AIVar *var)
{
    sdword i = 0;

    if (!var)
        return;

    if (var == aivRenderMainScreen)
    {
        aivRenderMainScreen = NULL;
    }

    while (i < varsUsed)
    {
        if (vars[i] == var)
        {
            varsUsed--;
            vars[i] = vars[varsUsed];
            break;
        }
        ++i;
    }
    memFree(var);
}

void aivarDestroyAll(void)
{
    aivarShutdown();
}

//
//  returns a guaranteed unique label for an AIVar
//
//  pass in a buffer of adequate size (AIVAR_LABEL_MAX_LENGTH+1)
//  and it will be filled in (and returned)
//
char *aivarLabelGenerate(char *label)
{
    label[0] = '_';
    label[1] = 'V';
    sprintf(&label[2], "%d", uniqueNum);
    uniqueNum++;
    return label;
}

char *aivarLabelGet(AIVar *var)
{
    if (var)
        return var->label;
    else
        return NULL;
}

void aivarLabelSet(AIVar *var, char *label)
{
    if (!var)
        return;

    memStrncpy(var->label, label, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(label) < AIVAR_LABEL_MAX_LENGTH)
        var->label[strlen(label)] = 0;
    else
        var->label[AIVAR_LABEL_MAX_LENGTH] = 0;
        */
}

void aivarValueSet(AIVar *var, sdword value)
{
    if (var)
        var->value = value;
}

sdword aivarValueGet(AIVar *var)
{
    if (var)
        return var->value;
    else
        return 0;
}

/*=============================================================================
    Save Game stuff here
=============================================================================*/

void SaveThisAIVar(AIVar *aivar)
{
    SaveStructureOfSize(aivar,sizeof(AIVar));
}

AIVar *LoadThisAIVar(void)
{
    return (AIVar *)LoadStructureOfSize(sizeof(AIVar));
}

sdword AIVarToNumber(AIVar *aivar)
{
    if (aivar == NULL)
    {
        return -1;
    }
    else
    {
        sdword i;

        for (i=0;i<varsUsed;i++)
        {
            if (aivar == vars[i])
            {
                return i;
            }
        }

        dbgAssert(FALSE);
        return -1;
    }
}

AIVar *NumberToAIVar(sdword number)
{
    if (number == -1)
    {
        return NULL;
    }
    else
    {
        dbgAssert(number < varsUsed);
        return vars[number];
    }
}

void aivarSave(void)
{
    sdword i;

    SaveInfoNumber(varsAllocated);
    SaveInfoNumber(varsUsed);
    SaveInfoNumber(uniqueNum);

    for (i=0;i<varsUsed;i++)
    {
        SaveThisAIVar(vars[i]);
    }
}

void aivarLoad(void)
{
    sdword i;

    varsAllocated = LoadInfoNumber();
    varsUsed = LoadInfoNumber();
    uniqueNum = LoadInfoNumber();

    if (varsAllocated > 0)
    {
        vars = memAlloc(sizeof(AIVar*)*varsAllocated, "aivarlist", 0);
    }
    else
    {
        vars = NULL;
    }

    for (i=0;i<varsUsed;i++)
    {
        vars[i] = LoadThisAIVar();
    }

    aivRenderMainScreen = aivarFind("RenderMainScreen");
    if (!aivRenderMainScreen)
        aivRenderMainScreen = aivarCreate("RenderMainScreen");
}

