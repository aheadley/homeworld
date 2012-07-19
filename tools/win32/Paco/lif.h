//
// LiF - layered image format
//

#ifndef _LIF_H
#define _LIF_H

#include "types.h"

#define TRF_Paletted    0x00000002  //texture uses a palette
#define TRF_Alpha       0x00000008  //alpha channel image
#define TRF_TeamColor0  0x00000010  //team color flags
#define TRF_TeamColor1  0x00000020

typedef struct lifheader
{
	char ident[8];
	sdword version;
	sdword flags;
	sdword width, height;
	crc32 paletteCRC;
	crc32 imageCRC;
	ubyte* data;
	color* palette;
	ubyte* teamEffect0;
	ubyte* teamEffect1;
} lifheader;

typedef struct llfileheader
{
    char ident[8];
    sdword version;
    sdword nElements;
    sdword stringLength;
    sdword sharingLength;
    sdword totalLength;
} llfileheader;

typedef struct llelement
{
    char* textureName;
    sdword width, height;
    udword flags;
    crc32 imageCRC;
    sdword nShared;
    sdword* sharedTo;
    sdword sharedFrom;
} llelement;

void lifExportPages(char* filePrefix);
char* lifShortName(char const* fullname);

#endif
