// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bigtree.h"

int main(int argc,char **argv)
{
	char szPath[1024];

	// Get arg
	if(argc > 1) strcpy(szPath,argv[1]);
	else strcpy(szPath,".");
	if(szPath[strlen(szPath)-1] == '\\') szPath[strlen(szPath)-1]=0;

	// Process root
	btListFiles(szPath,"");

	return(0);
}

int btListFiles(char *szPath,char *szSubPath)
{
	char szFind[1024],szSubFind[1024];

	BOOL bFind;

	HANDLE hFind;

	WIN32_FIND_DATA rFind;

	// Create find path
	strcpy(szFind,szPath);
	strcat(szFind,"\\");
	if(strlen(szSubPath) != 0)
	{
		strcat(szFind,szSubPath);
		strcat(szFind,"\\");
	}

	// Create find subpath
	strcpy(szSubFind,szFind);
	strcat(szSubFind,"*.*");

	// Find first file
	hFind=FindFirstFile(szSubFind,&rFind);
	if(hFind == INVALID_HANDLE_VALUE) return(ERR);

	while(1)
	{
		// Check flags
		if((rFind.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{	
			// Check dir name
			if((strcmp(rFind.cFileName,".") != 0) && (strcmp(rFind.cFileName,"..") != 0))
			{
				// Get dir name
				strcpy(szSubFind,szFind);
				strcat(szSubFind,rFind.cFileName);

				// Parse dir name
				btParseFiles(szSubFind,szPath);

				// Process dir
				btListFiles(szPath,szSubFind);
			}
		}
		else
		{
			// Get file name
			strcpy(szSubFind,szFind);
			strcat(szSubFind,rFind.cFileName);

			// Parse file name
			btParseFiles(szSubFind,szPath);

			// Show file name
			printf("%s\n",szSubFind);
		}

		// Find next file
		bFind=FindNextFile(hFind,&rFind);
		if(bFind == 0) break;
	}

	// Close find
	FindClose(hFind);

	return(OK);
}

int btParseFiles(char *szPath,char *szSubPath)
{
	char *pPath,*pSubPath,nPath,nSubPath;

	// Init data
	pPath=szPath;
	pSubPath=szSubPath;

	while(1)
	{
		// Get data
		nPath=pPath[0];
		nSubPath=pSubPath[0];

		// Adjust to non-caps
		if((nPath >= 65 ) && (nPath <= 90 )) nPath=nPath+32;
		if((nSubPath >= 65 ) && (nSubPath <= 90 )) nSubPath=nSubPath+32; 

		// Compare data
		if(nPath != nSubPath) break;
		
		// Check data
		if(pPath[0] == 0) break;
		if(pSubPath[0] == 0) break;

		// Next data
		pPath++;
		pSubPath++;
	}
	
	// Set data
	strcpy(szPath,pPath+1);

	return(OK);
}