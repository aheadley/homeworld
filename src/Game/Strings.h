/*=============================================================================
    Name    : Strings.h
    Purpose : Header for Strings.c

    Created 5/7/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef ___STRINGS_H
#define ___STRINGS_H

#include "StringsOnly.h"
// try not to include many files in here since strings.h may be included by jpeg files, etc.

typedef enum
{
    languageEnglish,
    languageFrench,
    languageGerman,
    languageSpanish,
    languageItalian
} strLanguageType;

/*=============================================================================
    Data externs:
=============================================================================*/

extern udword strCurLanguage;
extern udword strCurKeyboardLanguage;

/*=============================================================================
    Prototype's for String accessing functions:
=============================================================================*/

bool8 strLoadLanguage(strLanguageType language);
bool8 strFreeLanguage(void);
void  strSetStringCB(char *directory,char *field,void *dataToFillIn);
strGamesMessages strNameToEnum(char *string);
void strSetCurKeyboard(void);

#endif //___STRINGS_H
