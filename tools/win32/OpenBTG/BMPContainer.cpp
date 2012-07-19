/*
** BMPContainer.cpp : Code to handle BMP files in the BTG.
*/

#include "stdafx.h"
#include "assert.h"

#include "bmpcontainer.h"
#include "btgdialogbar.h"
#include "bmp.h"

CBMPContainer::CBMPContainer(char *fileName)
{
	BOOL bResult;
	char *fullFileName;

	fullFileName = new char [strlen(fileName) + strlen(BITMAP_LOAD_DIRECTORY) + 1];
	strcpy(fullFileName, BITMAP_LOAD_DIRECTORY);
	strcat(fullFileName, fileName);
	
	memset(&myFile, 0x00, sizeof(BMPFile));

	bResult = LoadBMPFile(fullFileName, &myFile);

	myFileName = fileName;

	assert(bResult);

	delete [] fullFileName;
}

CBMPContainer::~CBMPContainer()
{
	KillBMPFile(&myFile);
}