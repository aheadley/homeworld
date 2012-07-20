#ifndef __GLINC_H
#define __GLINC_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "gldll.h"
#include "rglu.h"

extern unsigned int RGL;
extern unsigned int RGLtype;

#include "glext.h"

#endif
