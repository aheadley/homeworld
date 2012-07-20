%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kas2c.h"
#include "parser.h"
#include "Types.h"

extern int lineNum;
extern char curFilename[];
extern char *yytname[];

%}

%option noyywrap

ws      [ \t]+
comment2 \/\/.*
ppline  \#[ \t]+[0-9]+[ \t]+.*
qstring \"[^\"\n]*[\"\n]
number  (0x[a-fA-F0-9]+)|([0-9]+)
id      [a-zA-Z][a-zA-Z0-9]*
nl      \n
ceq     ==
eq      =
cneq    !=
neq     <>
lt      <
lte     <=
gt      >
gte     >=
cor     \|\|
cand    \&\&
%%

{ws}        ;
{comment2}  ;
{ppline}    {   sscanf(yytext, "# %d %s", &lineNum, curFilename);
            }
{qstring}   {   yylval.string = strdup(yytext+1); /* skip open quote */
                if (yylval.string[yyleng-2] != '"')
                    fprintf(stderr, "Unterminated character string\n");
                else
                    yylval.string[yyleng-2] = '\0'; /* remove close quote */
                return QSTRING;
            }
fsm         { return FSM; }
endf        { return ENDF; }
endfsm      { return ENDF; }
states      { return STATES; }
watch       { return WATCH; }
endw        { return ENDW; }
endwatch    { return ENDW; }
state       { return STATE; }
ends        { return ENDS; }
endstate    { return ENDS; }
initialize  { return INITIALIZE; }
endi        { return ENDI; }
init        { return INITIALIZE; }
endinit     { return ENDI; }
endinitialize { return ENDI; }
if          { return IF; }
elseif      { return ELSEIF; }
elsif       { return ELSEIF; }
else        { return ELSE; }
endif       { return ENDIF; }
ifonce      { return IFONCE; }
endifonce   { return ENDIFONCE; }
and         { return AND; }
or          { return OR; }
not         { return NOT; }
team        { return TEAM; }
path        { return PATH; }
point       { return POINT; }
volume      { return VOLUME; }
ships       { return SHIPS; }
teamships   { return TEAMSHIPS; }
shipspoint  { return SHIPSPOINT; }
teamspoint  { return TEAMSPOINT; }
volumepoint { return VOLUMEPOINT; }
thisteam    { return THISTEAM; }
thisteamships  { return THISTEAMSHIPS; }
thisteamspoint { return THISTEAMSPOINT; }
jump        { return JUMP; }
true        { return TRUE; }
false       { return FALSE; }
fsmcreate   { return FSMCREATE; }
localization { return LOCALIZATION; }
endl        { return ENDL; }
endlocalization { return ENDL; }
lstring     { return LSTRING; }
{ceq}       { return EQ; }
{eq}        { return EQ; }
{lt}        { return LT; }
{lte}       { return LTE; }
{gt}        { return GT; }
{gte}       { return GTE; }
{cneq}      { return NEQ; }
{neq}       { return NEQ; }
{id}        {   yylval.string = strdup(yytext);
                return ID;
            }
{cor}       { return OR; }
{cand}      { return AND; }
{number}    {   if (yytext[0] == '0' && yytext[1] == 'x')
                        sscanf(yytext, "%x", &yylval.number);   // hex
                else
                        yylval.number = atoi(yytext);   // decimal
                return NUMBER;
            }
{nl}        { lineNum++; }
.           { return yytext[0]; }

%%
