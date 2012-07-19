/*=============================================================================
    Name    : screenshot.h
    Purpose : output screen grabs to disk

    Created 7/15/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _SCREENSHOT_H
#define _SCREENSHOT_H

#define SS_SCREENSHOTS          1
#ifndef HW_Release

#define SS_ERROR_CHECKING      1               //basic error checking
#define SS_VERBOSE_LEVEL       1               //control verbose printing

#else //HW_Debug

#define SS_ERROR_CHECKING      0               //no error ckecking in retail
#define SS_VERBOSE_LEVEL       0               //don't print any verbose strings in retail

#endif //HW_Debug
#include "types.h"

void ssSaveScreenshot(ubyte* buf);

#endif
