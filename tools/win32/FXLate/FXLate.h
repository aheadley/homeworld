// Copyright (c) 1998-99 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef FXLATE_H
#define FXLATE_H

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
#define FX_HOTKEY	'&'
#define FX_NUM		8
#define FX_RATIO	0.5
#define FX_EXT		".bak"
#define FX_TEMP		"~FXLate.tmp"
#define FX_OFFSET	2

// Language costants
#define FX_LGDATA	aLangDat
#define FX_LGNUM	6
#define FX_LGSIZE	4

// Token constants
#define FX_GENFONT		"FONT}"
#define FX_GENLABEL		"_MYSTRING"
#define FX_GENINFO		"INFO}"
#define FX_FILETAG		"{FEMANAGER_TAG}"
#define FX_FILEEND		"{FEMANAGER_END}"
#define FX_SCREENTAG	"{SCREEN_TAG}"
#define FX_SCREENLABEL	"{SCREEN_SCREENNAME}"
#define FX_SCREENEND	"{SCREEN_END}"
#define FX_RECTTAG		"{SCREENOBJECTRECT_TAG}"
#define FX_RECTEND		"{SCREENOBJECTRECT_END}"
#define FX_TEXTTAG		"{SCREENOBJECTSTRING_TAG}"
#define FX_TEXTLABEL	"{SCREENOBJECTSTRING_MYSTRING}"
#define FX_TEXTLABELX	"{SCREENOBJECTSTRING_MYSTRING%1ld}"
#define FX_TEXTJUST		"{SCREENOBJECTSTRING_JUSTIFICATION}"
#define FX_TEXTEND		"{SCREENOBJECTSTRING_END}"
#define FX_OBJBOUNDS	"{SCREENOBJECT_BOUNDS}"
#define FX_OBJSRC		"{SCREENOBJECT_SRC}"
#define FX_OBJINFO		"{SCREENOBJECT_USERINFO}"
#define FX_INFOBUTTON	"Button"
#define FX_INFOTOGGLE	"ToggleButton"
#define FX_INFODRAG		"DragButton"
#define FX_INFORADIO	"RadioButton"
#define FX_INFOBITMAP	"BitmapButton"
#define FX_INFOCHECK	"CheckBox"
#define FX_INFOHOTKEY	"Hotkey"
#define FX_JUSTCENT		2

#ifdef __cplusplus
extern "C" {
#endif

// Functions

#ifdef __cplusplus
}		// extern "C"
#endif

#endif
