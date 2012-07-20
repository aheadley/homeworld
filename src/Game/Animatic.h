/*=============================================================================
    Name    : animatic.h
    Purpose : playback of animatics via OpenGL

    Created 2/11/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _ANIMATIC_H
#define _ANIMATIC_H

#include "Types.h"

extern sdword animaticJustPlayed;

void animStartup(void);
void animShutdown(void);
bool animBinkPlay(sdword a, sdword b);

#endif
