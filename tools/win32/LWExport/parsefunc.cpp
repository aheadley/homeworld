/*
** PARSEFUNC.CPP : Code to handle parsing functions.
*/

#include "parsefunc.h"

#include "assert.h"
#include "string.h"

void AddParseFunc(FunctionDef *fd, char *name, void *func)
{
	fd->functionName = new char [strlen(name) + 1];

	strcpy(fd->functionName, name);

	fd->functionPtr = (ParseFuncPtr)func;
}

void RemParseFunc(FunctionDef *fd)
{
	if(fd->functionName)
		delete [] fd->functionName;

	fd->functionName = 0;

	fd->functionPtr = 0;
}

ParseFuncPtr IsParseFunc(char *lineBuf, FunctionDef *funcDef, unsigned long numParseFuncs)
{
	unsigned long i;
	char delimiter[] = {' ', 0x00};
	char *pBufTemp, *pBufToken;

	// We want to preserve the line buffer.
	pBufTemp = new char [strlen(lineBuf) + 1];
	strcpy(pBufTemp, lineBuf);

#ifdef ANAL_DEBUGGING	
	printf("Analyzing %s:\n", pBufTemp);
#endif

	pBufToken = strtok(pBufTemp, delimiter);

	if(!pBufToken)
	{
		delete [] pBufTemp;
		return((ParseFuncPtr)0);	
	}

	do
	{
#ifdef ANAL_DEBUGGING
		printf("Tokenizing %s:\n", pBufToken);
#endif

		// Loop through the string tags for each function name and compare them to the data from the file.
		for( i=0 ; i<numParseFuncs ; i++ )
		{
			if(!strcmp(funcDef[i].functionName, pBufToken))
			{
				delete [] pBufTemp;
				return(funcDef[i].functionPtr);
			}
		}
	} while(pBufToken = strtok(NULL, delimiter));

	delete [] pBufTemp;
	return((ParseFuncPtr)0);
}