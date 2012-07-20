/*=============================================================================
    Name    : Fontreg.c
    Purpose : Font registry logic and data

    Created 8/6/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Debug.h"
#include "Memory.h"
#include "font.h"
#include "FontReg.h"
#include "Strings.h"

/*=============================================================================
    Data:
=============================================================================*/
//font registry
fontregistry frFontRegistry[FR_NumberFonts];

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : frStartup
    Description : Start font registry module
    Inputs      :
    Outputs     : init the font registry
    Return      :
----------------------------------------------------------------------------*/
void frStartup(void)
{
    sdword index;

    for (index = FR_NumberFonts - 1; index >= 0; index--)
    {                                                       //for all of the registry
        frFontRegistry[index].name = NULL;                  //nothing registered in this slot
    }
}

/*-----------------------------------------------------------------------------
    Name        : frReset
    Description : reset the font registry module
    Inputs      :
    Outputs     : a reset font registry
    Return      :
----------------------------------------------------------------------------*/
void frReset(void)
{
    sdword index;

    for (index = FR_NumberFonts - 1; index >= 0; index--)
    {
        if (frFontRegistry[index].name != NULL)
        {
            fontDiscardGL(frFontRegistry[index].fontdat);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : frReloadGL
    Description : reload the GL font pages for all fonts in the registry
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void frReloadGL(void)
{
    sdword index;

    for (index = FR_NumberFonts - 1; index >= 0; index--)
    {
        if (frFontRegistry[index].name != NULL)
        {
            glfontRecreate(frFontRegistry[index].fontdat);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : frShutdown
    Description : Shuts down the font registry module
    Inputs      :
    Outputs     : Frees all fonts in the registry
    Return      :
----------------------------------------------------------------------------*/
void frShutdown(void)
{
    sdword index;
    for (index = FR_NumberFonts - 1; index >= 1; index--)
    {                                                       //for all of the registry
        if (frFontRegistry[index].name != NULL)
        {                                                   //if non-NULL name, it has been registered
#if FR_VERBOSE_LEVEL >= 2
            dbgMessagef("\nfrShutdown: Deleting font %s with a usage count of %d",
                        frFontRegistry[index].name, frFontRegistry[index].nUsageCount);
#endif
            memFree(frFontRegistry[index].name);            //free previously allocated name
            frFontRegistry[index].name = NULL;              //no longer registered
            fontDiscard(frFontRegistry[index].handle);      //free the font
            frFontRegistry[index].handle = 0;            //no longer registered
        }
        frFontRegistry[index].name = NULL;                  //nothing registered in this slot
    }
}

/*-----------------------------------------------------------------------------
    Name        : frFontRegister
    Description : Register usage of a font, loading it if needed
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
fonthandle frFontRegister(char *fileName)
{
    sdword index, freeIndex = -1;
    char fullName[80];

    for (index = FR_NumberFonts - 1; index >= 1; index--)
    {                                                       //for all of the registry
        if (frFontRegistry[index].name == NULL)
        {                                                   //if this one free
            freeIndex = index;
        }
        else
        {
            if (frNamesCompare(fileName, frFontRegistry[index].name))
            {                                               //if this is the font you want
#if FR_VERBOSE_LEVEL >= 2
            dbgMessagef("\nfrFontRegister: Returning pre-registered font %s with usage count of %d",
                        frFontRegistry[index].name, frFontRegistry[index].nUsageCount);
#endif
                frFontRegistry[index].nUsageCount++;        //increment usage count
                return(index);       //return pre-loaded handle
            }
        }
    }
#if FR_ERROR_CHECKING
    if (freeIndex == -1)
    {
        dbgFatalf(DBG_Loc, "Font registry not big enough for %s, only %d long", fileName, FR_NumberFonts);
    }
#endif
#if FR_VERBOSE_LEVEL >= 1
    dbgMessagef("\nfrFontRegister: Font %s not pre-registered, loading now...", fileName);
#endif
    frFontRegistry[freeIndex].name = memStringDupe(fileName);//duplicate name string
    frFontRegistry[freeIndex].nUsageCount = 1;              //start usage count at 1
    strcpy(fullName, FR_PrependPath);                       //prepare file path
    if (strCurLanguage==languageEnglish)
    {
        strcat(fullName, FR_English);
    }
    else if (strCurLanguage==languageFrench)
    {
        strcat(fullName, FR_French);
    }
    else if (strCurLanguage==languageGerman)
    {
        strcat(fullName, FR_German);
    }
    else if (strCurLanguage==languageSpanish)
    {
        strcat(fullName, FR_Spanish);
    }
    else if (strCurLanguage==languageItalian)
    {
        strcat(fullName, FR_Italian);
    }
    strcat(fullName, fileName);
    frFontRegistry[freeIndex].fontdat = fontLoad(fullName);  //load file
    frFontRegistry[freeIndex].handle = freeIndex;
    return(freeIndex);
}

/*-----------------------------------------------------------------------------
    Name        : frReloadFonts
    Description : reloads all of the currently loaded fonts with respect to the currently selected language.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void frReloadFonts(void)
{
    sdword index;
    fonthandle curfont;
    char   name[64];
    char fullName[128];

    curfont = fontCurrentGet();

    for (index = FR_NumberFonts - 1; index >= 1; index--)
    {                                                       //for all of the registry
        if (frFontRegistry[index].name != NULL)
        {
            // free memory associated with the current font
            strcpy(name, frFontRegistry[index].name);
            memFree(frFontRegistry[index].name);            //free previously allocated name
            frFontRegistry[index].name = NULL;              //no longer registered
            fontDiscard(frFontRegistry[index].handle);      //free the font

            frFontRegistry[index].name = memStringDupe(name);//duplicate name string
            strcpy(fullName, FR_PrependPath);                       //prepare file path
            if (strCurLanguage==languageEnglish)
            {
                strcat(fullName, FR_English);
            }
            else if (strCurLanguage==languageFrench)
            {
                strcat(fullName, FR_French);
            }
            else if (strCurLanguage==languageGerman)
            {
                strcat(fullName, FR_German);
            }
            else if (strCurLanguage==languageSpanish)
            {
                strcat(fullName, FR_Spanish);
            }
            else if (strCurLanguage==languageItalian)
            {
                strcat(fullName, FR_Italian);
            }
            strcat(fullName, name);
            frFontRegistry[index].fontdat = fontLoad(fullName);  //load file
            frFontRegistry[index].handle = index;
        }
    }

    fontMakeCurrent(curfont);
}


