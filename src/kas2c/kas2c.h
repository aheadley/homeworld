#ifndef __KAS2C_H
#define __KAS2C_H

//#define KAS2C_VERSION "2.05"
#define KAS2C_VERSION "2.05sdl"

// "depth" of script we're currently parsing at
#define LEVEL_LEVEL 0
#define LEVEL_FSM   1
#define LEVEL_STATE 2

//  this name length may have a corresponding #define in the game code
#define MAX_LEVEL_NAME_LENGTH 32

//  this name length may have a corresponding #define in the game code
#define MAX_FSM_NAME_LENGTH 32
#define MAX_FSM_COUNT       256

//  this name length may have a corresponding #define in the game code
#define MAX_STATE_NAME_LENGTH 32
#define MAX_STATE_COUNT       256

#define MAX_LSTRING_NAME_LENGTH 32
#define MAX_LSTRING_COUNT       256

// this defines how many languages we're localizing for -- keep this in sync with the game code
#define LSTRING_COUNT_REQUIRED  5

#define MAX_FUNC_DEPTH          16
#define MAX_FUNC_NAME_LENGTH    128
#define MAX_PARAM_NAME_LENGTH   128
#define MAX_PARAM_VALUE_LENGTH  512
#define MAX_FUNC_PARAMS         16

// function parameter types
// NOTE: keep kasParamTypeToString and kasParamTypeToC updated to match order!
enum {
    // primitive types
    PARAM_SBYTE,
    PARAM_UBYTE,
    PARAM_SWORD,
    PARAM_UWORD,
    PARAM_SDWORD,
    PARAM_UDWORD,
    PARAM_REAL32,
    PARAM_REAL64,
    PARAM_BOOL,
    PARAM_BOOL8,
    PARAM_BOOL16,
    // special types (some conversion required)
    PARAM_SHIPPTR,
    PARAM_GROWSELECTIONPTR,
    PARAM_AITEAMPTR,
    PARAM_CHARPTR,
    PARAM_PATHPTR,
    PARAM_VECTORPTR,
    PARAM_VOLUMEPTR,
};

typedef struct {
    char name[MAX_PARAM_NAME_LENGTH+1];
    int  type;
} FunctionParam;

typedef struct {
    char name[MAX_FUNC_NAME_LENGTH+1];
    int returnsNumber;
    char realName[MAX_FUNC_NAME_LENGTH+1];
    int numParams;
    FunctionParam params[MAX_FUNC_PARAMS];
} FunctionCall;

typedef struct {
    int functionNum;
    int paramsSoFar;
} CallStack;

// support functions in KAS.c
void kasFSMAdd(char *name);
void kasFSMStart(char *name);
void kasFSMEnd(char *name);
int kasFSMValid(char *name);
void kasFSMCreateStart(char *name);
void kasFSMCreateEnd(void);
void kasStateListClear(void);
void kasStateListAdd(char *name);
void kasStateListEnd(void);
int kasStateValid(char *name);
void kasStateStart(char *name);
void kasStateEnd(char *name);
void kasInitializeStart(void);
void kasInitializeEnd(void);
void kasWatchStart(void);
void kasWatchEnd(void);
void kasFunctionStart(char *name);
void kasFunctionEnd(void);
void kasJump(char *newStateName);
int kasFunction(char *name);
void kasFunctionParamNumber(void);
void kasFunctionParamCharPtr(void);
void kasFunctionParamAITeamPtr(void);
void kasFunctionParamSelectCommandPtr(void);
void kasFunctionParamPathPtr(void);
void kasFunctionParamVectorPtr(void);
void kasFunctionParamVolumePtr(void);
void kasHeaders(int ctype);
char *kasParamTypeToC(int type);
char *kasParamTypeToString(int type);
void kasLocalizationStart(void);
void kasLocalizationEnd(void);
void kasLStringClearAll(void);
int kasLStringValid(char *name);
void kasLStringDefineStart(char *name);
void kasLStringDefineEnd(void);
void kasLStringValue(char *value);
void kasLStringReference(char *name);

char* stateHelpGet ();

char* levelNameGet ();

#endif
