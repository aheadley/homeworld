//
// Main.h
// Paco's top-level stuff
//

#ifndef _MAIN_H
#define _MAIN_H

#include "types.h"
#include "Mex.h"
#include "crc.h"

bool mainStartupOnce(void);
bool mainStartup(void);
bool mainLoadTextures(char* filename);
bool mainLoadTexture(char* filename, bool alpha);
bool mainLoadTextureWithCRC(char* filename, crc32 crc);
bool mainClassifyTextures(void);
bool mainShutdown(void);
crc32 mainGetTextureCRC(char* filename);

#endif
