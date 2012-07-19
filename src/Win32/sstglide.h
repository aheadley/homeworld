/*=============================================================================
    Name    : sstglide.h
    Purpose : 3Dfx-specific support

    Created 2/19/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _SSTGLIDE_H
#define _SSTGLIDE_H

#include "types.h"

#define SST_VOODOO  0
#define SST_VOODOO2 1
#define SST_OTHER   2

uword* sstGetFramebuffer(sdword* pitch);
void sstFlush(void);
void sstFinish(void);
void sstStartup(void);
void sstShutdown(void);
bool sstHardwareExists(sdword* type);
bool sstLoaded(void);

#endif
