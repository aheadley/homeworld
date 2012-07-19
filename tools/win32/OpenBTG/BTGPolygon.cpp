/*
** BTGPolygon.cpp : Code to handle interaction with BTGPolygons.
*/

#include "stdafx.h"

#include "btgpolygon.h"
#include "btgvertex.h"

BTGPolygon::BTGPolygon(void)
{
	flags = 0;
}

BTGPolygon::BTGPolygon(BTGVertex *pv0, BTGVertex *pv1, BTGVertex *pv2)
{
	// Handle default initialization;
	flags = 0;

	bVisible = TRUE;

	v0 = pv0;
	v1 = pv1;
	v2 = pv2;
}

BTGPolygon::~BTGPolygon(void)
{
	flags = 0;
}