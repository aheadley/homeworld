/*
** BTGStar.cpp : Code to handle interaction with BTGStars.
*/

#include "stdafx.h"
#include "assert.h"

#include "BTGStar.h"
#include "bmpcontainer.h"

#include "mainfrm.h"

BTGStar::BTGStar(char *fileName)
{
	flags = 0;
	x = y = 0;
	red = green = blue = alpha = 0xFF;
	
	assert(fileName);

	CTGAContainer *pContainer;

	pContainer = (CTGAContainer *)gTGAFileList.GetHead();

	while(pContainer)
	{
		if(!strcmp(fileName, pContainer->myFileName))
			break;

		pContainer = (CTGAContainer *)pContainer->GetNext();
	}

	pMyStar = pContainer;

	bVisible = TRUE;
}

BTGStar::BTGStar(CPoint *pPoint, char *fileName)
{
	// Handle default initialization;
	flags = 0;
	red = green = blue = 0;
	alpha = 255;

	assert(fileName);
	
	// Set the point;
	x = pPoint->x;
	y = pPoint->y;

	CTGAContainer *pContainer;

	pContainer = (CTGAContainer *)gTGAFileList.GetHead();

	while(pContainer)
	{
		if(!strcmp(fileName, pContainer->myFileName))
			break;

		pContainer = (CTGAContainer *)pContainer->GetNext();
	}

	pMyStar = pContainer;
}

BTGStar::~BTGStar(void)
{
	flags = 0;
	x = y = 0;
	red = green = blue = 0;
	alpha = 255;
}