//
// Input.h
// Paco's input routines
//

#ifndef _INPUT_H
#define _INPUT_H

#include "types.h"
#include "lif.h"

sdword fileSizeGet(char* filename);
bool fileLoad(char* filename, ubyte* data);
bool fileSave(char* filename, ubyte* data, sdword length);
lifheader* inLIFFileLoad(char* filename);
void inLIFFileClose(lifheader* header);

#endif
