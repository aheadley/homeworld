/*=============================================================================
    Name    : dxdraw.h
    Purpose : DirectDraw / non-DirectDraw display mode control

    Created 2/09/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _DXDRAW_H
#define _DXDRAW_H

//used for OpenGL devices that don't like DirectDraw,
//and for intro video sequences
unsigned int hwSetRes(int, int, int);
unsigned int hwCreateWindow(int, int, int, int);
unsigned int hwDeleteWindow(void);
unsigned int hwGetDepth(void);
unsigned int hwActivate(int activate);

//DirectDraw versions of some of the above
unsigned int ddCreateWindow(int, int, int, int);
unsigned int ddDeleteWindow(void);
unsigned int ddActivate(int activate);

// 2003-09-28: Ted Cipicchio:
// Temporary hack to allow for SDL to initialize the display with OpenGL.
extern int hwWidth, hwHeight;
extern int hwDepth;

#endif
