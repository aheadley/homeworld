// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef MMHASH_H
#define MMHASH_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

// MissionMan constants
#define MMH_SEC_SING	"S_"
#define MMH_SEC_MULTI	"M_"
#define MMH_SEC_OBJ		"Object_"
#define MMH_KEY_TYPE	"Type"
#define MMH_KEY_FILE	"File_"
#define MMH_EXT_OBJ		".mor"

#ifdef __cplusplus
extern "C" {
#endif

// Functions
unsigned long strhash(char *str);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif
