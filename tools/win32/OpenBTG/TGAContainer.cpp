/*
** TGAContainer.cpp : Code to handle TGA files in the BTG.
*/

#include "stdafx.h"
#include "assert.h"

#include "TGAcontainer.h"
#include "btgdialogbar.h"
#include "TGA.h"

CTGAContainer::CTGAContainer(char *fileName)
{
	BOOL bResult;
	char *fullFileName;

	fullFileName = new char [strlen(fileName) + strlen(BITMAP_LOAD_DIRECTORY) + 1];
	strcpy(fullFileName, BITMAP_LOAD_DIRECTORY);
	strcat(fullFileName, fileName);
	
	memset(&myFile, 0x00, sizeof(TGAFile));

	bResult = LoadTGAFile(fullFileName, &myFile);

	myFileName = fileName;

	assert(bResult);

	delete [] fullFileName;
}

CTGAContainer::~CTGAContainer()
{
	KillTGAFile(&myFile);
}