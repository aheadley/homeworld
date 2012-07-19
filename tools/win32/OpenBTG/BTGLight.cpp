/*
** BTGLight.cpp : Code to handle interaction with BTGVertices.
*/

#include "stdafx.h"

#include "btgLight.h"

BTGLight::BTGLight(void)
{
	flags = 0;
	x = y = 0;
    dred = dgreen = dblue = 255;
    ared = agreen = ablue = 0;

	bVisible = TRUE;
}

BTGLight::BTGLight(CPoint *pPoint)
{
	// Handle default initialization;
	flags = 0;
    dred = dgreen = dblue = 255;
    ared = agreen = ablue = 0;
	
	// Set the point;
	x = pPoint->x;
	y = pPoint->y;
}

BTGLight::~BTGLight(void)
{
	flags = 0;
	x = y = 0;
    dred = dgreen = dblue = 255;
    ared = agreen = ablue = 0;
}
