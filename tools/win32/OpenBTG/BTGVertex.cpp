/*
** BTGVertex.cpp : Code to handle interaction with BTGVertices.  They are the atomic primitive of this program.
*/

#include "stdafx.h"

#include "btgvertex.h"

BTGVertex::BTGVertex(void)
{
	flags = 0;
	x = y = 0;
	red = green = blue = 0;
	alpha = brightness = 255;

	bVisible = TRUE;
}

BTGVertex::BTGVertex(CPoint *pPoint)
{
	// Handle default initialization;
	flags = 0;
	red = green = blue = 0;
	alpha = brightness = 255;
	
	// Set the point;
	x = pPoint->x;
	y = pPoint->y;
}

BTGVertex::~BTGVertex(void)
{
	flags = 0;
	x = y = 0;
	red = green = blue = 0;
	alpha = brightness = 255;
}