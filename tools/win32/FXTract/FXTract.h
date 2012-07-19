// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef FXTRACT_H
#define FXTRACT_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

#define FX_MARGIN	1024
#define FX_COMMENT	"//"
#define FX_SEPAR	"\t"
#define FX_COLUMN	8
#define FX_LIMIT	5 // Up to FX_COLUMN
#define FX_UNDEF	"<Untitled>"

// Token constants
#define FX_FILETAG		"{FEMANAGER_TAG}"
#define FX_FILEEND		"{FEMANAGER_END}"
#define FX_SCREENTAG	"{SCREEN_TAG}"
#define FX_SCREENLABEL	"{SCREEN_SCREENNAME}"
#define FX_SCREENEND	"{SCREEN_END}"
#define FX_TEXTTAG		"{SCREENOBJECTSTRING_TAG}"
#define FX_TEXTLABEL	"{SCREENOBJECTSTRING_MYSTRING}"
#define FX_TEXTLABELX	"{SCREENOBJECTSTRING_MYSTRING%1ld}"
#define FX_TEXTEND		"{SCREENOBJECTSTRING_END}"

#ifdef __cplusplus
extern "C" {
#endif

// Functions

#ifdef __cplusplus
}		// extern "C"
#endif

#endif
