%{

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "Types.h"
#include "kas2c.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

int parseLevel = LEVEL_LEVEL;

int ifOnceIndex;  // up to 2^32 IFONCE statements per WATCH or INIT routine

extern FILE *yyout;
extern char *yytext;

extern int lineNum;
extern char curFilename[];

%}

%union {
    char    *string;    /* string buffer */
    int     number;     /* numeric value */
}

%token <string> QSTRING ID COMMENT2
%token <number> FSM ENDF STATES WATCH ENDW STATE ENDS INITIALIZE ENDI IF ELSEIF ELSE ENDIF IFONCE ENDIFONCE NUMBER AND OR NOT TEAM SHIPS TEAMSHIPS SHIPSPOINT TEAMSPOINT VOLUMEPOINT PATH POINT VOLUME THISTEAM THISTEAMSHIPS THISTEAMSPOINT JUMP TRUE FALSE LT LTE EQ NEQ GTE GT FSMCREATE LOCALIZATION ENDL LSTRING

%left AND OR
%left LT LTE EQ NEQ GTE GT
%left '-' '+'
%left '*' '/'
%nonassoc BANG
%nonassoc NOT
%nonassoc UMINUS

%start level

%%

level:  localization fsms initialize_block watch_block
    ;

localization:    /* empty */
    |   localization_start lstring_defs localization_end
    ;

localization_start: LOCALIZATION { kasLocalizationStart(); }
    ;

localization_end:   ENDL { kasLocalizationEnd(); }
    ;

lstring_defs:    /* empty */
    |   lstring_defs lstring_def
    ;

/*LSTRING '_' ID { kasLStringDefineStart($3); } '=' QSTRING ',' QSTRING ',' QSTRING ';' { kasLStringDefineEnd($6, $8, $10); }*/
/*lstringdef: ID
    ;*/

lstring_def:    lstring_def_name lstring_value_list { kasLStringDefineEnd(); }
    ;

lstring_def_name:   LSTRING '_' ID { kasLStringDefineStart($3); }
    ;

lstring_value_list: QSTRING { kasLStringValue($1); }
    |   lstring_value_list QSTRING { kasLStringValue($2); }
    ;

fsms:   /* empty */
    |   fsms fsm
    ;

fsm:    fsm_start state_list initialize_block watch_block states fsm_end
    ;

fsm_start:  FSM ID { kasFSMStart($2); }
    ;

fsm_end:    ENDF    { kasFSMEnd(""); }
    |       ENDF ID { kasFSMEnd($2); }
    ;

state_list:  { kasStateListClear(); kasStateListEnd(); }  /* empty */
    |       STATES { kasStateListClear(); } state_id_list ';' { kasStateListEnd(); }
    ;

state_id_list:  ID                      { kasStateListAdd($1); }
    |           state_id_list ',' ID    { kasStateListAdd($3); }
    ;

initialize_block:   { kasInitializeStart(); kasInitializeEnd(); }  /* empty */
    |               initialize_start statements initialize_end
    ;

initialize_start:   INITIALIZE     { kasInitializeStart(); }
    ;

initialize_end:     ENDI            { kasInitializeEnd(); }
    ;

watch_block:    { kasWatchStart(); kasWatchEnd(); }  /* empty */
    |           watch_start statements watch_end
    ;

watch_start:        WATCH           { kasWatchStart(); }
    ;

watch_end:          ENDW            { kasWatchEnd(); }
    ;

/*
block:  expression ';' { fprintf(yyout, ";\n"); }
    |   '{' { fprintf(yyout, " { "); } expressions '}' { fprintf(yyout, " }\n"); }
*/

statements:     /* empty */
    |           statements statement
    ;

statement:      ';' { fprintf(yyout, ";\n\t"); }
    |           expression ';' { fprintf(yyout, ";\n\t"); }
    |           JUMP ID ';'    { kasJump($2); fprintf(yyout, ";\n\treturn;\n\t"); }
    |           ifstatement
    |           ifoncestatement
//    |           msgifstatement
    |           fsmcreate
    ;

// we fake this "function" call to allow FSM validation at translation time
fsmcreate:      FSMCREATE '(' ID { kasFSMCreateStart($3); } ',' paramteam ')' ';' { kasFSMCreateEnd(); }
    ;

// we fake this "function" call to allow easier event handling via msgs to ourself
//msgifstatement: MSGIF '(' { fprintf(yyou,t "if ("); } expression ',' { fprintf(yyout, ")"); } QSTRING ')' { fprintf(yyout, "\n\t\taitMsgSend(kasThisTeamPtr, \"%s\"\n\t"); }
//    ;

ifstatement:    ifcondition elseif else ENDIF
    ;

else:           /* empty*/
    |           ELSE { fprintf(yyout, "else\n\t{\n\t"); } statements { fprintf(yyout, "}\n\t"); }
    ;

elseif:         /* empty */
    |           elseif ELSEIF '(' { fprintf(yyout, "else if ("); } expression ')' { fprintf(yyout, ")\n\t{\n\t"); } statements { fprintf(yyout, "}\n\t"); }
    ;

ifcondition:    IF '(' { fprintf(yyout, "if ("); } expression ')' { fprintf(yyout, ")\n\t{\n\t"); } statements { fprintf(yyout, "}\n\t"); }
    ;

ifoncestatement:    ifoncecondition ENDIFONCE
    ;

ifoncecondition:    IFONCE '(' { fprintf(yyout, "if (!kasfVarValueGet(\"_VIFONCE%03d\") && (", ifOnceIndex+1); } expression ')' { fprintf(yyout, ")) // \"ifonce\" #%d\n\t{\n\tkasfVarCreateSet(\"_VIFONCE%03d\", 1);\n\t", ifOnceIndex+1, ifOnceIndex+1); } statements { fprintf(yyout, "}\n\t"); ++ifOnceIndex; }
    ;

/* an expression always boils down to an int */
expression: expression '+' { fprintf(yyout, " + "); } expression
    |       expression '-' { fprintf(yyout, " - "); } expression
    |       expression '*' { fprintf(yyout, " * "); } expression
    |       expression '/' { fprintf(yyout, " / "); } expression  /* { if ($3 = 0) yyerror("divide by zero"); } */
    |       expression AND { fprintf(yyout, " && "); } expression
    |       expression OR  { fprintf(yyout, " || "); } expression
    |       expression EQ  { fprintf(yyout, " == "); } expression
    |       expression NEQ { fprintf(yyout, " != "); } expression
    |       expression GT  { fprintf(yyout, " > "); } expression
    |       expression GTE { fprintf(yyout, " >= "); } expression
    |       expression LT  { fprintf(yyout, " < "); } expression
    |       expression LTE { fprintf(yyout, " <= "); } expression
    |       '-'     { fprintf(yyout, "-"); } expression %prec UMINUS
    |       '!'     { fprintf(yyout, "!"); } expression %prec BANG
    |       NOT     { fprintf(yyout, "!"); } expression %prec NOT
    |       '(' { fprintf(yyout, "("); } expression ')' { fprintf(yyout, ")"); }
    |       NUMBER { fprintf(yyout, "%d", $1); }
    |       ID { kasFunctionStart($1); } '(' param_list ')' { kasFunctionEnd(); }
    |       TRUE { fprintf(yyout, "1"); }
    |       FALSE { fprintf(yyout, "0"); }
    ;

param_list: /* empty */
    |       param
    |       param_list ',' { fprintf(yyout, ", "); } param
    ;

param:      expression { kasFunctionParamNumber(); }
    |       QSTRING    { fprintf(yyout, "\"%s\"", $1);
                         kasFunctionParamCharPtr(); }
    |       LSTRING '_' ID  { kasLStringReference($3);
                              kasFunctionParamCharPtr(); }
    |       paramteam
/*
    |       SHIPS '_' ID   { fprintf(yyout, "kasSelectCommandPtr(\"%s\")", $3);
                             kasFunctionParamSelectCommandPtr(); }
*/
    |       SHIPS '_' ID   { fprintf(yyout, "kasGrowSelectionPtr(\"%s\")", $3);
                             kasFunctionParamSelectCommandPtr(); }
    |       PATH '_' ID    { fprintf(yyout, "kasPathPtr(\"%s\")", $3);
                             kasFunctionParamPathPtr(); }
    |       POINT '_' ID    { fprintf(yyout, "kasVectorPtr(\"%s\")", $3);
                              kasFunctionParamVectorPtr(); }
    |       VOLUME '_' ID   { fprintf(yyout, "kasVolumePtr(\"%s\")", $3);
                              kasFunctionParamVolumePtr(); }
    |       TEAMSHIPS '_' ID    { fprintf(yyout, "kasAITeamShipsPtr(\"%s\")", $3);
                                  kasFunctionParamSelectCommandPtr(); }
    |       SHIPSPOINT '_' ID    { fprintf(yyout, "kasShipsVectorPtr(\"%s\")", $3);
                                   kasFunctionParamVectorPtr(); }
    |       TEAMSPOINT '_' ID    { fprintf(yyout, "kasTeamsVectorPtr(\"%s\")", $3);
                                   kasFunctionParamVectorPtr(); }
    |       VOLUMEPOINT '_' ID    { fprintf(yyout, "kasVolumeVectorPtr(\"%s\")", $3);
                                   kasFunctionParamVectorPtr(); }
    |       THISTEAMSHIPS  { fprintf(yyout, "(&kasThisTeamPtr->shipList)");
                        kasFunctionParamSelectCommandPtr(); }
    |       THISTEAMSPOINT { fprintf(yyout, "kasThisTeamsVectorPtr()");
                                kasFunctionParamVectorPtr();  }
    ;

paramteam:  TEAM '_' ID    { fprintf(yyout, "kasAITeamPtr(\"%s\")", $3);
                             kasFunctionParamAITeamPtr(); }
    |       THISTEAM  { fprintf(yyout, "kasThisTeamPtr");
                        kasFunctionParamAITeamPtr(); }
    ;

states:     /* empty */
    |       states state
    ;

state:      state_start initialize_block watch_block state_end
    ;

state_start:    STATE ID    { kasStateStart($2); }
    ;

state_end:      ENDS    { kasStateEnd(""); }
    |           ENDS ID { kasStateEnd($2); }
    ;

%%

int lineNum = 1;
char curFilename[256]; // current input filename
char levelName[MAX_LEVEL_NAME_LENGTH+1]; // current input filename
FILE *yyhout, *yyfout;
unsigned int functionCount;  // number of FSM functions

int yyerror (char *s)
{
    if (!strcasecmp(s, "parse error"))
        fprintf(stderr, "%s(%d) error : at '%s', %s\n", curFilename, lineNum, yytext, stateHelpGet());
    else
        fprintf(stderr, "%s(%d) error : at '%s', %s (%s)\n", curFilename, lineNum, yytext, stateHelpGet(), s);
}

char *levelNameGet(void)
{
    return levelName;
}

int lineNumGet(void)
{
    return lineNum;
}

char *curFilenameGet()
{
    return curFilename;
}

main(int argc, char **argv)
{
    char infilename[256], outfilename[256], houtfilename[256], foutfilename[256];
    char *shortinfilename, tempfilename[256];
    extern FILE *yyin, *yyout;
    int i;

    if (argc >= 2 && !strcasecmp(argv[1], "-F"))
    {
        kasHeaders(0);
        exit(1);
    }
    else if (argc >= 2 && !strcasecmp(argv[1], "-FC"))
    {
        kasHeaders(1);
        exit(1);
    }
    else if (argc != 4 && argc != 5)
    {
        fprintf(stderr, "\nKAS2C Version %s\n", KAS2C_VERSION);
        fprintf(stderr, "Copyright (C) 1998 Relic Entertainment Inc.  All rights reserved.\n");
        fprintf(stderr, "Modifications for Homeworld SDL by Ted Cipicchio <ted@thereisnospork.com>\n");
        fprintf(stderr, "\n Usage:\n\n");
        fprintf(stderr, " KAS2C mission.kas mission.c mission.h [func.c]\n");
        fprintf(stderr, " - generate mission.c and mission.h (and, optionally, func.c)\n");
        fprintf(stderr, " KAS2C -f\n");
        fprintf(stderr, " - list function headers (english style)\n");
        fprintf(stderr, " KAS2C -fc\n");
        fprintf(stderr, " - list function headers (C-style)\n");
        exit(1);
    }

    strcpy(infilename, argv[1]);

    strcpy(outfilename, argv[2]);

    strcpy(houtfilename, argv[3]);

    strcpy(foutfilename, (argc == 5 ? argv[4] : ""));

    if (!(yyin = fopen(infilename, "r")))
    {
        fprintf(stderr, "%s: can't open\n", infilename);
        exit(1);
    }

    if (!(yyout = fopen(outfilename, "w")))
    {
        fprintf(stderr, "%s: can't open\n", outfilename);
        exit(1);
    }

    if (!(yyhout = fopen(houtfilename, "w")))
    {
        fprintf(stderr, "%s: can't open\n", houtfilename);
        exit(1);
    }

    if (argc == 5 && !(yyfout = fopen(foutfilename, "w")))
    {
        fprintf(stderr, "%s: can't open\n", foutfilename);
        exit(1);
    }

    // hack off leading paths for display of infilename
    shortinfilename = infilename+strlen(infilename);
    while (shortinfilename > infilename)
    {
        --shortinfilename;
        if (*shortinfilename == '\\' || *shortinfilename == '/')
        {
            ++shortinfilename;
            break;
        }
    }
    fprintf(stderr, "%s\n", shortinfilename);

    strcpy(tempfilename, shortinfilename);
    strcpy(infilename, tempfilename);

    strcpy(levelName, shortinfilename);
    i = 0;
    while (i < strlen(levelName) && levelName[i] != '.')
        ++i;
    if (i < strlen(levelName))
        levelName[i] = 0;  // knock off extension

    strcpy(curFilename, infilename);

    fprintf(yyout, "//\n");
    fprintf(yyout, "//  %s\n", outfilename);
    fprintf(yyout, "//\n");
    fprintf(yyout, "//  Finite state machine routines for \"%s\" mission\n", levelName);
    fprintf(yyout, "//\n");
    fprintf(yyout, "//  Copyright (C) 1998 Relic Entertainment Inc.\n");
    fprintf(yyout, "//  All rights reserved\n");
    fprintf(yyout, "//\n");
    fprintf(yyout, "//  This code was autogenerated from %s by KAS2C Version %s\n",
        infilename, KAS2C_VERSION);
    fprintf(yyout, "//\n\n\n");
    fprintf(yyout, "#include \"%s\"  // prototypes and #includes for exposed game functions\n", houtfilename);
    fprintf(yyout, "\n");
    fprintf(yyout, "extern AITeam *CurrentTeamP;\n");
    fprintf(yyout, "#define kasThisTeamPtr CurrentTeamP\n");
    fprintf(yyout, "\n");
    fprintf(yyout, "//  for run-time scoping of symbols (variables, timers, etc.)\n");
    fprintf(yyout, "extern sdword CurrentMissionScope;\n");
    fprintf(yyout, "extern char CurrentMissionScopeName[];\n");
    fprintf(yyout, "\n");

    fprintf(yyout, "//  for displaying localized strings\n");
    fprintf(yyout, "extern udword strCurLanguage;\n");
    fprintf(yyout, "\n");

    fprintf(yyhout, "//\n");
    fprintf(yyhout, "//  %s\n", houtfilename);
    fprintf(yyhout, "//\n");
    fprintf(yyhout, "//  Finite state machine for \"%s\" mission\n", levelName);
    fprintf(yyhout, "//\n");
    fprintf(yyhout, "//  Copyright (C) 1998 Relic Entertainment Inc.\n");
    fprintf(yyhout, "//  All rights reserved\n");
    fprintf(yyhout, "//\n");
    fprintf(yyhout, "//  This code was autogenerated from %s by KAS2C Version %s\n",
        infilename, KAS2C_VERSION);
    fprintf(yyhout, "//\n\n\n");
    fprintf(yyhout, "#ifndef __%s_H\n", levelName);
    fprintf(yyhout, "#define __%s_H\n\n\n", levelName);

    // add any other required #includes here
    fprintf(yyhout, "//\n");
    fprintf(yyhout, "//  types and exposed game functions\n");
    fprintf(yyhout, "//\n");
    fprintf(yyhout, "#include <string.h>\n");
    fprintf(yyhout, "#include \"Types.h\"\n");
    fprintf(yyhout, "#include \"Vector.h\"\n");
    fprintf(yyhout, "#include \"AITeam.h\"\n");
    fprintf(yyhout, "#include \"AIMoves.h\"\n");
    fprintf(yyhout, "#include \"CommandWrap.h\"\n");
    fprintf(yyhout, "#include \"Timer.h\"\n");
    fprintf(yyhout, "#include \"Volume.h\"\n");
    fprintf(yyhout, "#include \"Objectives.h\"\n");
    fprintf(yyhout, "#include \"Attributes.h\"\n");
    fprintf(yyhout, "#include \"TradeMgr.h\"\n");

    // declarations of function list information
    if (yyfout)
    {
        fprintf(yyhout, "\n\n");
        fprintf(yyhout, "//\n");
        fprintf(yyhout, "//  level function pointer list\n");
        fprintf(yyhout, "//\n");
        fprintf(yyhout, "extern const void* %s_FunctionPointers[];\n", levelName);
        fprintf(yyhout, "extern const unsigned int %s_FunctionPointerCount;\n", levelName);
    }

    fprintf(yyhout, "\n\n");
    fprintf(yyhout, "//\n");
    fprintf(yyhout, "//  FSM prototypes\n");
    fprintf(yyhout, "//\n");

    if (yyfout)
    {
        fprintf(yyfout, "//\n");
        fprintf(yyfout, "//  %s\n", foutfilename);
        fprintf(yyfout, "//\n");
        fprintf(yyfout, "//  Finite state machine function pointers for \"%s\" mission\n", levelName);
        fprintf(yyfout, "//\n");
        fprintf(yyfout, "//  This code was autogenerated from %s by KAS2C Version %s\n",
            infilename, KAS2C_VERSION);
        fprintf(yyfout, "//\n\n\n");
        fprintf(yyfout, "#include \"%s\"  // prototypes and #includes for exposed game functions\n", houtfilename);
        fprintf(yyfout, "\n");
        fprintf(yyfout, "// FSM init/watch function pointers.\n");
        fprintf(yyfout, "const void* %s_FunctionPointers[] =\n", levelName);
        fprintf(yyfout, "{\n");
    }

    ifOnceIndex = 0;  // reset counter for each source file parsed

    functionCount = 0;  // number of FSM init/watch functions created

    // normal yyin and yyout interaction
    yyparse();

    fprintf(yyhout, "\n\n#endif\n");

    if (yyfout)
    {
        fprintf(yyfout, "};\n");
        fprintf(yyfout, "\n");
        fprintf(yyfout, "// Number of FSM init/watch function pointers.\n");
        fprintf(yyfout, "const unsigned int %s_FunctionPointerCount = %u;\n", levelName, functionCount);
    }

    fclose(yyin);
    fclose(yyout);
    fclose(yyhout);
    if (yyfout)
        fclose(yyfout);

    // to be done: unlink output files on error?
    if (yynerrs)
    {
        unlink(outfilename);
        unlink(houtfilename);
        if (foutfilename[0]);
            unlink(foutfilename);
    }

}
