#ifndef __AIVAR_H
#define __AIVAR_H

#include "Types.h"

//
//  general purpose AI variable stuff (can be used as flags, counters, etc.)
//
#define AIVAR_LABEL_MAX_LENGTH 47

typedef struct AIVar {
    sdword value;
    char label[AIVAR_LABEL_MAX_LENGTH+1];
    // possibly add other data types, etc.
} AIVar;

void aivarStartup(void);
void aivarShutdown(void);
AIVar *aivarCreate(char *label);
AIVar *aivarFind(char *label);
AIVar *aivarFindAnyFSM(char *label);
char *aivarLabelGenerate(char *label);
char *aivarLabelGet(AIVar *var);
void aivarLabelSet(AIVar *var, char *label);
void aivarDestroy(AIVar *var);
void aivarDestroyAll(void);
void aivarValueSet(AIVar *var, sdword value);
sdword aivarValueGet(AIVar *var);

void aivarSave(void);
void aivarLoad(void);
sdword AIVarToNumber(AIVar *aivar);
AIVar *NumberToAIVar(sdword number);

extern AIVar *aivRenderMainScreen;

//
//  reserved pointers to all allocated AIVars
//
#define AIVAR_ALLOC_INITIAL   64
#define AIVAR_ALLOC_INCREMENT 32

#endif
