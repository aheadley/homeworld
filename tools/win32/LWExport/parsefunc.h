/*
** PARSEFUNC.H : Header file for PARSEFUNC.CPP.
*/

#ifndef __PARSEFUNC_H
#define __PARSEFUNC_H

#include "stdio.h"

	//////////////////////////////////////////////////////////////////////////
	// Function information.
	//////////////////////////////////////////////////////////////////////////
	typedef signed char (* ParseFuncPtr)(char *, FILE *);

	typedef struct tagFunctionDef
	{
		char *functionName;
		ParseFuncPtr functionPtr;
	} FunctionDef;

	void AddParseFunc(FunctionDef *fd, char *name, void *func);

	void RemParseFunc(FunctionDef *fd);

	ParseFuncPtr IsParseFunc(char *lineBuf, FunctionDef *funcDef, unsigned long numParseFuncs);

#endif