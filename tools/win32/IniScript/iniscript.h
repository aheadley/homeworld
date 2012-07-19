// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef INISCRIPT_H
#define INISCRIPT_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Functions
int isCommand(char *szFile,char *szArgs,unsigned long nIndex,int nDebug,int nPrompt);
int isParse(char *szFile,char *szArgs,char *szIn,char *szOut,int nPrompt);
int isGetFromIni(char *szFile,char *szSection,char *szKey,char *szData);
int isGetFromReg(char *szReg,char *szData);
int isGetFromEnv(char *szEnv,char *szData);
int isGetFromPrompt(char *szVar,char *szData);
int isGetFromArg(char *szArgs,char *szData,unsigned long nNum);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif
