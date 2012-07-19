// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef BIGTREE_H
#define BIGTREE_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Functions
int btListFiles(char *szPath,char *szSubPath);
int btParseFiles(char *szPath,char *szSubPath);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif
