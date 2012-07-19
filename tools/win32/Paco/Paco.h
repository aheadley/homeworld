//
// Paco, the texture packer
//

#ifndef _PACO_H
#define _PACO_H

#include "types.h"

#undef PAC_Verbosity
#define LIF_CRC 1

extern bool exportPages;
extern bool quietMode;
extern bool polyMode;

extern char rawPrefix[];

extern bool pacOutputShared;
extern bool pacOutputPages;
extern bool pacDumpTargas;
extern bool pacExportMesh;
extern bool pacResortPeo;
extern bool pacPerformFixup;

extern sdword pacLOD;

bool mainStuff(char* filePrefix, char* fileName);

#endif
