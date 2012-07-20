/*=============================================================================
    Name    : Subtitle.c
    Purpose : Functions for printing subtitles in time with the speech

    Created 2/3/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include "glinc.h"
#include "Debug.h"
#include "Memory.h"
#include "font.h"
#include "color.h"
#include "StatScript.h"
#include "FontReg.h"
#include "main.h"
#include "Key.h"
#include "Universe.h"
#include "Teams.h"
#include "mainrgn.h"
#include "Sensors.h"
#include "NIS.h"
#include "render.h"
#include "FEReg.h"
#include "Strings.h"
#include "Subtitle.h"

/*=============================================================================
    Data:
=============================================================================*/

//strings for testing
#if SUB_MODULE_TEST
char *subTestSubtitle[NINEKEY - ONEKEY + 1] =
{
    "A short string",
    "This page was originally made to tell what I saw and what had occured to me during my 1994-1995 trip to the U.S. Amundsen-Scott South Pole Station. However the process of writing the text and preparing the pictures took so much time that I had to halt for a while so that I can pass all those courses that I need in order to stay in the Graduate School. The result is that now I have come back from the second (now the third, fourth .. ) trip which I have some thing new to tell, but I still have unfinished first-year stuff to write. So here are some stories from the 1995 trip and writings based on later ones. \n\nPhotographic notes: Most photographic information are not directly put in the text. A lot of pictures are not serious shots. Here are some photographic related information. #n#nView them the right way: There are a lot of pictures in my web pages. If you find pictures on your screen too dark or too bright, probably you need to adjust gamma of your monitor. I created all images that are used here with the gamma set to 2.2.The best viewing environment is when there is no light directly shining on your screen.  Adjust your monitor so that the center block in this image is indistinguishable against the grey part when you look at it from 6 to 8 feet away; and the border is as dark as possible. This will make your monitor have a closer presentation to mine. However I was told it could be hard to achieve on Macs. \n\nAnyway, enjoy the stories! ",
    "A meduim-length string withSomeReallyLongWordsToReallyMessWithTheWordWrappingLogic.ThisOutghToMessThemUp!",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
real32 subTestSubtitleTime[NINEKEY - ONEKEY + 1] =
{
    1.0f,
    12.0f,
    2.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f
};
#endif //SUB_MODULE_TEST

//semaphore for communicating between stream thread and main game thread
SDL_sem* subSemaphore;

//pointer to timer to use currently.  May be different depending on NIS's etc.
real32 *subTimeElapsed = NULL;

//set to TRUE when a fleet intel event ends for the benifit of KAS
sdword subMessageEnded = FALSE;

//retained copy of the player's race and all team colors so we know when to
//re-load the speech icon textures.
ShipRace subLastRace;
trcolorinfo subLastTeamColors[TE_NumberPlayers];

/*-----------------------------------------------------------------------------
    This next section of data should only be accessed under the protection
    of the previous semaphore.
-----------------------------------------------------------------------------*/
subnewsubtitle subNewSubtitles[SUB_NumberNewSubtitles];
sdword subNumberNewSubtitles;
#if SUB_VERBOSE_LEVEL >= 1
sdword subMissedSubtitles;
#endif

/*-----------------------------------------------------------------------------
    Subtitle themes and printing regions.
-----------------------------------------------------------------------------*/
subregion   subRegion[SUB_NumberRegions];
subtheme    subTheme[SUB_NumberThemes];

/*-----------------------------------------------------------------------------
    General subtitle stuff
-----------------------------------------------------------------------------*/
real32 subScrollDwellStart = SUB_ScrollDwellStart;
real32 subScrollDwellEnd = SUB_ScrollDwellEnd;
real32 subScrollShortest = SUB_ScrollShortest;
real32 subTitleShortest = SUB_TitleShortest;
bool8 subCloseCaptionsEnabled = TRUE;

/*-----------------------------------------------------------------------------
    Script parsing table for subtitle.script.
-----------------------------------------------------------------------------*/
void subThemeColorSet(char *directory, char *field, void *dataToFillIn);
void subThemeDropshadowSet(char *directory, char *field, void *dataToFillIn);
void subThemeMarginSet(char *directory, char *field, void *dataToFillIn);
void subThemeFontSet(char *directory, char *field, void *dataToFillIn);
void subThemeFadeSet(char *directory, char *field, void *dataToFillIn);
void subRegionBoundsSet(char *directory, char *field, void *dataToFillIn);
void subRegionScaleRezSet(char *directory, char *field, void *dataToFillIn);
void subRegionMaxLinesSet(char *directory, char *field, void *dataToFillIn);
void subThemeCentredSet(char *directory, char *field, void *dataToFillIn);
void subThemePictureSet(char *directory, char *field, void *dataToFillIn);
void subRegionIconOffsetSet(char *directory, char *field, void *dataToFillIn);
scriptEntry subScriptTable[] =
{
    //define theme parameters
    { "themeColor", subThemeColorSet, NULL},
    { "themeDropshadow", subThemeDropshadowSet, NULL},
    { "themeMargin", subThemeMarginSet, NULL},
    { "themeFont", subThemeFontSet, NULL},
    { "themeFadeIn", subThemeFadeSet, (void *)FALSE},
    { "themeFadeOut", subThemeFadeSet, (void *)TRUE},
    { "themePicture", subThemePictureSet, (void *)TRUE},
    { "themeCentred", subThemeCentredSet, (void *)TRUE},

    //define printing regions
    { "regionBounds", subRegionBoundsSet, NULL},
    { "regionScaleRez", subRegionScaleRezSet, NULL},
    { "regionMaxLines", subRegionMaxLinesSet, NULL},
    { "regionIconOffset", subRegionIconOffsetSet, NULL},

    //general stuff
    { "scrollDwellStart", scriptSetReal32CB, &subScrollDwellStart},
    { "scrollDwellEnd", scriptSetReal32CB, &subScrollDwellEnd},
    { "scrollShortest", scriptSetReal32CB, &subScrollShortest},
    { "subtitleShortest", scriptSetReal32CB, &subTitleShortest},

    { NULL, NULL, 0 }
};

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : subAnyCardsOnScreen
    Description : returns TRUE if any subtitle cards are on the screen
    Inputs      :
    Outputs     :
    Return      : returns TRUE if any subtitle cards are on the screen
----------------------------------------------------------------------------*/
bool subAnyCardsOnScreen(void)
{
    sdword index;

    for (index = 0; index < SUB_NumberRegions; index++)
    {
        if (subRegion[index].bEnabled && subRegion[index].cardIndex > 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : subThemeColorSet
    Description : Script-oarsing callback for setting a theme's color
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subThemeColorSet(char *directory, char *field, void *dataToFillIn)
{
    udword nScanned, iTheme, red, green, blue;

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %d %d %d", &iTheme, &red, &green, &blue);
#if SUB_ERROR_CHECKING
    if (nScanned != 4)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 4, nScanned);
    }
    if (iTheme >= SUB_NumberThemes)
    {
        dbgFatalf(DBG_Loc, "Invalid theme number %d.  Must be between 0 and %d.", iTheme, SUB_NumberThemes);
    }
    if (red > UBYTE_Max || green > UBYTE_Max || blue > UBYTE_Max)
    {
        dbgFatalf(DBG_Loc, "Invalid color in string '%s'", field);
    }
#endif
    subTheme[iTheme].textColor = colRGB(red, green, blue);
}

/*-----------------------------------------------------------------------------
    Name        : subThemeDropshadowSet
    Description : Script parse callback for setting the dropshadow flag on a theme.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subThemeDropshadowSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iTheme;
    char boolString[UBYTE_Max];

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %s", &iTheme, boolString);
#if SUB_ERROR_CHECKING
    if (nScanned != 2)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 2, nScanned);
    }
    if (iTheme < 0 || iTheme >= SUB_NumberThemes)
    {
        dbgFatalf(DBG_Loc, "Invalid theme number %d.  Must be between 0 and %d.", iTheme, SUB_NumberThemes);
    }
#endif
    subTheme[iTheme].bDropShadow = (bool8)scriptStringToBool(boolString);
}

/*-----------------------------------------------------------------------------
    Name        : subThemeCentredSet
    Description : Script parse callback for setting the centred flag on a theme.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subThemeCentredSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iTheme;
    char boolString[UBYTE_Max];

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %s", &iTheme, boolString);
#if SUB_ERROR_CHECKING
    if (nScanned != 2)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 2, nScanned);
    }
    if (iTheme < 0 || iTheme >= SUB_NumberThemes)
    {
        dbgFatalf(DBG_Loc, "Invalid theme number %d.  Must be between 0 and %d.", iTheme, SUB_NumberThemes);
    }
#endif
    subTheme[iTheme].bCentred = (bool8)scriptStringToBool(boolString);
}

/*-----------------------------------------------------------------------------
    Name        : subThemePictureSet
    Description : Script parse callback for setting the picture for a theme.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subThemePictureSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iTheme, iColorScheme;
    char pictureString[UBYTE_Max];
    char fileName[UBYTE_Max];

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %d %s", &iTheme, &iColorScheme, pictureString);
#if SUB_ERROR_CHECKING
    if (nScanned != 3)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 3, nScanned);
    }
    if (iTheme < 0 || iTheme >= SUB_NumberThemes)
    {
        dbgFatalf(DBG_Loc, "Invalid theme number %d.  Must be between 0 and %d.", iTheme, SUB_NumberThemes);
    }
    if (iColorScheme < 0 || iColorScheme >= TE_NumberPlayers)
    {
        dbgFatalf(DBG_Loc, "Invalid color scheme %d for '%s'", iColorScheme, pictureString);
    }
#endif
    if (universe.players[0].race == R1)
    {
        strcpy(fileName, "SpeechIcons\\R1\\");
    }
    else
    {
        strcpy(fileName, "SpeechIcons\\R2\\");
    }
    strcat(fileName, pictureString);
    subTheme[iTheme].picture = trTextureRegister(fileName, &teColorSchemes[iColorScheme].textureColor, (void *)&subTheme[iTheme]);
    subTheme[iTheme].pictureColorScheme = iColorScheme;
    subTheme[iTheme].bPicture = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : subThemeMarginSet
    Description : Script parse callback for setting the margin of a theme
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subThemeMarginSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iTheme, margin;

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %d", &iTheme, &margin);
#if SUB_ERROR_CHECKING
    if (nScanned != 2)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 2, nScanned);
    }
    if (iTheme < 0 || iTheme >= SUB_NumberThemes)
    {
        dbgFatalf(DBG_Loc, "Invalid theme number %d.  Must be between 0 and %d.", iTheme, SUB_NumberThemes);
    }
#endif
    subTheme[iTheme].margin = margin;
}

/*-----------------------------------------------------------------------------
    Name        : subThemeFadeSet
    Description : Script parse callback for setting a fade time
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subThemeFadeSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iTheme;
    real32 fadeTime;

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %f", &iTheme, &fadeTime);
#if SUB_ERROR_CHECKING
    if (nScanned != 2)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 2, nScanned);
    }
    if (iTheme < 0 || iTheme >= SUB_NumberThemes)
    {
        dbgFatalf(DBG_Loc, "Invalid theme number %d.  Must be between 0 and %d.", iTheme, SUB_NumberThemes);
    }
    dbgAssert(fadeTime >= 0.0f && fadeTime < 80.0f);
#endif
    if ((bool)dataToFillIn == FALSE)
    {
        subTheme[iTheme].fadeIn = fadeTime;
    }
    else
    {
        subTheme[iTheme].fadeOut = fadeTime;
    }
}

/*-----------------------------------------------------------------------------
    Name        : subThemeFontSet
    Description : Script parse callback for setting the font of a theme
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subThemeFontSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iTheme;
    char fontName[1024];

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %s", &iTheme, fontName);
#if SUB_ERROR_CHECKING
    if (nScanned != 2)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 2, nScanned);
    }
    if (iTheme < 0 || iTheme >= SUB_NumberThemes)
    {
        dbgFatalf(DBG_Loc, "Invalid theme number %d.  Must be between 0 and %d.", iTheme, SUB_NumberThemes);
    }
#endif
    subTheme[iTheme].font = frFontRegister(fontName);
}

/*-----------------------------------------------------------------------------
    Name        : subRegionBoundsSet
    Description : Script parse callback to set the bounds of a subtitle region.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subRegionBoundsSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iRegion;
    rectangle rect;

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %d %d %d %d", &iRegion, &rect.x0, &rect.y0, &rect.x1, &rect.y1);
#if SUB_ERROR_CHECKING
    if (nScanned != 5)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 5, nScanned);
    }
    if (iRegion < 0 || iRegion >= SUB_NumberRegions)
    {
        dbgFatalf(DBG_Loc, "Invalid region number %d.  Must be between 0 and %d.", iRegion, SUB_NumberRegions);
    }
    dbgAssert(rect.x0 < 640);
    dbgAssert(rect.x1 > 0);
    dbgAssert(rect.y0 < 480);
    dbgAssert(rect.y1 > 0);
    dbgAssert(rect.x0 < rect.x1);
    dbgAssert(rect.y0 < rect.y1);
#endif
    subRegion[iRegion].defaultRect = rect;
}

/*-----------------------------------------------------------------------------
    Name        : subRegionScaleRezSet
    Description : script parse callback to set the resolution scale flags for a
                    subtitle region
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subRegionScaleRezSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iRegion;
    char bScaleRezX[UBYTE_Max], bScaleRezY[UBYTE_Max];

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %s %s", &iRegion, bScaleRezX, bScaleRezY);
#if SUB_ERROR_CHECKING
    if (nScanned != 3)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 3, nScanned);
    }
    if (iRegion < 0 || iRegion >= SUB_NumberRegions)
    {
        dbgFatalf(DBG_Loc, "Invalid region number %d.  Must be between 0 and %d.", iRegion, SUB_NumberRegions);
    }
#endif
    subRegion[iRegion].bScaleRezX = (bool8)scriptStringToBool(bScaleRezX);
    subRegion[iRegion].bScaleRezY = (bool8)scriptStringToBool(bScaleRezY);
}

/*-----------------------------------------------------------------------------
    Name        : subRegionMaxLinesSet
    Description : Script parse callback for setting the max number of lines in
                    a region.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subRegionMaxLinesSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iRegion, maxLines;

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %d", &iRegion, &maxLines);
#if SUB_ERROR_CHECKING
    if (nScanned != 2)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 2, nScanned);
    }
    if (iRegion < 0 || iRegion >= SUB_NumberRegions)
    {
        dbgFatalf(DBG_Loc, "Invalid region number %d.  Must be between 0 and %d.", iRegion, SUB_NumberRegions);
    }
    dbgAssert(maxLines > 0 && maxLines < UDWORD_Max);
#endif
    subRegion[iRegion].numberCards = maxLines;
}

/*-----------------------------------------------------------------------------
    Name        : subRegionIconOffsetSet
    Description : Script parse callback for setting the speech icon offset (if any) for the region
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subRegionIconOffsetSet(char *directory, char *field, void *dataToFillIn)
{
    sdword nScanned, iRegion, x, y;

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%d %d %d", &iRegion, &x, &y);
#if SUB_ERROR_CHECKING
    if (nScanned != 3)
    {
        dbgFatalf(DBG_Loc, "Wrong number of fields in '%s': expected %d found %d", field, 3, nScanned);
    }
    if (iRegion < 0 || iRegion >= SUB_NumberRegions)
    {
        dbgFatalf(DBG_Loc, "Invalid region number %d.  Must be between 0 and %d.", iRegion, SUB_NumberRegions);
    }
#endif
    subRegion[iRegion].iconOffsetX = x;
    subRegion[iRegion].iconOffsetY = y;
}

/*-----------------------------------------------------------------------------
    Name        : subTextureInfoRemember
    Description : Remembers the player's race and team color information.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subTextureInfoRemember(void)
{
    sdword index;
    subLastRace = universe.players[0].race;
    for (index = 0; index < TE_NumberPlayers; index++)
    {
        subLastTeamColors[index] = teColorSchemes[index].textureColor;
    }
}

/*-----------------------------------------------------------------------------
    Name        : subRegionsRescale
    Description : Rescales all region rectangles for a new screen resolution.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subRegionsRescale(void)
{
    sdword index;
    sdword diffX = (MAIN_WindowWidth - 640) / 2;
    sdword diffY = (MAIN_WindowHeight - 480) / 2;

    for (index = 0; index < SUB_NumberRegions; index++)
    {
        if (subRegion[index].defaultRect.x1 != 0)
        {                                                   //if a region was defined
            if (subRegion[index].bScaleRezX)
            {                                               //if x-scale in resolution
                subRegion[index].rect.x0 = subRegion[index].defaultRect.x0 * MAIN_WindowWidth / 640;
                subRegion[index].rect.x1 = subRegion[index].defaultRect.x1 * MAIN_WindowWidth / 640;
            }
            else
            {
                subRegion[index].rect.x0 = subRegion[index].defaultRect.x0 + diffX;
                subRegion[index].rect.x1 = subRegion[index].defaultRect.x1 + diffX;
            }
            if (subRegion[index].bScaleRezY)
            {                                               //if y-scale in resolution
                subRegion[index].rect.y0 = subRegion[index].defaultRect.y0 * MAIN_WindowHeight / 480;
                subRegion[index].rect.y1 = subRegion[index].defaultRect.y1 * MAIN_WindowHeight / 480;
            }
            else
            {
                subRegion[index].rect.y0 = subRegion[index].defaultRect.y0 + diffY;
                subRegion[index].rect.y1 = subRegion[index].defaultRect.y1 + diffY;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : subStartup
    Description : Starts the subtitle module, reading in the subtitle scripts
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subStartup(void)
{
    sdword index;

#if SUB_VERBOSE_LEVEL >= 1
    dbgMessage("\nStartup subtitle module");
#endif
    subNumberNewSubtitles = 0;
#if SUB_VERBOSE_LEVEL >= 1
    subMissedSubtitles = 0;
#endif
    subSemaphore = SDL_CreateSemaphore(1);
    dbgAssert(subSemaphore != NULL);

    memset(&subRegion, 0, sizeof(subRegion));               //clear out the themes and printing regions
    memset(&subTheme, 0, sizeof(subTheme));
    scriptSet(NULL, SUB_ScriptFile, subScriptTable);        //load in the script file

    //post-setup of the output regions
    subRegionsRescale();
    for (index = 0; index < SUB_NumberRegions; index++)
    {
        if (subRegion[index].defaultRect.x1 != 0)
        {                                                   //if a region was defined
            dbgAssert(subRegion[index].numberCards > 0 && subRegion[index].numberCards < UWORD_Max);
            subRegion[index].card = memAlloc(sizeof(subcard) * subRegion[index].numberCards, "subTitleCards", NonVolatile);
            subRegion[index].bEnabled = TRUE;
            subRegion[index].bAborted = FALSE;
        }
    }
    subRegion[STR_CloseCaptioned].bEnabled = subCloseCaptionsEnabled;//special case because these are enabled with a command-line switch
    subTextureInfoRemember();
}

/*-----------------------------------------------------------------------------
    Name        : subReset
    Description : To be called between levels, clears out any leftover subtitles
                    from all regions.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void subReset(void)
{
    sdword index;

    //clear all text cards out of all regions
    for (index = 0; index < SUB_NumberRegions; index++)
    {
        subRegion[index].cardIndex = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : subTexturesReset
    Description : Resets the subtitle textures, re-coloring and ensureing the
                    correct race of textures is loaded.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void subTexturesReset(void)
{
    sdword index;
    char fileName[UBYTE_Max];
    texreg *reg;

    for (index = 0; index < SUB_NumberRegions; index++)
    {
        subRegion[index].cardIndex = 0;
    }
    //clear all text cards out of all regions
    for (index = 0; index < SUB_NumberThemes; index++)
    {
        //recolorize the textures
#if TR_NIL_TEXTURE
        if (subTheme[index].bPicture && (!GLOBAL_NO_TEXTURES))
#else
        if (subTheme[index].bPicture)
#endif
        {
            //trTextureColorsUpdate(subTheme[index].picture, &teColorSchemes[subTheme[index].pictureColorScheme].textureColor);
            //replace the image with the image for the correct race.
            reg = trStructureGet(subTheme[index].picture);
            strcpy(fileName, reg->fileName);
            if (universe.players[0].race == R1)
            {
                //Name if texture is:"SpeechIcons\RX\BlahBlah"
                //            replace this number: ^
#define SUB_NUMBER_INDEX            13
                fileName[SUB_NUMBER_INDEX] = '1';
            }
            else
            {
                fileName[SUB_NUMBER_INDEX] = '2';
            }
            if (universe.players[0].race != subLastRace ||
                memcmp(&subLastTeamColors[subTheme[index].pictureColorScheme], &teColorSchemes[subTheme[index].pictureColorScheme].textureColor, sizeof(trcolorinfo)))
            {
                //trTextureUnregister(subTheme[index].picture);
                trRegisterRemoval(subTheme[index].picture);
                subTheme[index].picture = trTextureRegister(fileName, &teColorSchemes[subTheme[index].pictureColorScheme].textureColor, (void *)&subTheme[index]);
            }
        }
    }
    subTextureInfoRemember();
}

/*-----------------------------------------------------------------------------
    Name        : subShutdown
    Description : Shuts down subtitle module, freeing memory as needed.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subShutdown(void)
{
    sdword index;

#if SUB_VERBOSE_LEVEL >= 1
    dbgMessage("\nShutdown subtitle module");
#endif
    SDL_DestroySemaphore(subSemaphore);

    //free any allocated memory
    for (index = 0; index < SUB_NumberRegions; index++)
    {
        if (subRegion[index].card != NULL)
        {
            memFree(subRegion[index].card);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : subStringsChop
    Description : Chop up a long string to make it fit in the specified region
                    using the specified type theme.
    Inputs      : region - screen size to print into
                  font - what font to print in?
                  longLength - length of string we're chopping up.
                  longString - string we are to be chopping up
    Outputs     : chopBuffer - buffer they'll be chopped up into (in case
                    NULL-terminators need to be added)
                  choppedStrings - list of pointers to chopped lines.
    Return      : number of chopped strings
----------------------------------------------------------------------------*/
sdword subStringsChop(rectangle *rect, fonthandle font, sdword longLength, char *longString, char *chopBuffer, char **choppedStrings)
{
    sdword nChopped = 0;
    sdword nBytesUsed = 0;
    sdword width = rect->x1 - rect->x0;
    char *chopStart, *chopEnd, *nextChopStart, *wrapPtr;
    fonthandle current = fontCurrentGet();
    sdword state;

    fontMakeCurrent(font);

    chopStart = longString;
//    while (chopStart - longString < longLength)
//    {                                                       //while there is still some string to chop up
        for (chopEnd = chopStart; *chopStart != 0; chopEnd++)
        {                                                   //find the string to fit on this line
            //see if there is a newline or end-of-string
            if ((chopEnd[0] == '#' || chopEnd[0] == '\\') && chopEnd[1] == 'n')
            {                                               //"#n" or "\\n" - newline
                for (nextChopStart = chopEnd + 2; *nextChopStart; nextChopStart++)
                {                                           //find next non-whitespace, non-newline character
                    if (!strchr(" \t\n\r", *nextChopStart))
                    {
                        break;
                    }
                }
            }
            else if (chopEnd[0] == '\n' || chopEnd[0] == '\r')
            {                                               // '\n' or '\r'
                for (nextChopStart = chopEnd + 1; *nextChopStart; nextChopStart++)
                {                                           //find next non-whitespace, non-newline character
                    if (!strchr(" \t\r", *nextChopStart))
                    {
                        break;
                    }
                }
            }
            else if (*chopEnd == 0)
            {                                               //end of string
                nextChopStart = chopEnd;                    //last time through this loop
            }
            //!!! no support for dashes yet

            //note: this method of fontWidth'ing is rather inefficient because
            //fontWidth will continuously be parsing the same piece of string.  Oh well.
            else if (fontWidthN(chopStart, chopEnd - chopStart + 1) > width)
            {                                               //if string too long for the region it's in
                //scan backwards to find some whitespace to cut out
                state = 0;                                  //search for whitespace state
                for (wrapPtr = chopEnd; wrapPtr > chopStart; wrapPtr--)
                {
                    if (state == 0)
                    {                                       //search for whitespace state
                        if (strchr("\t ", *wrapPtr))
                        {                                   //if this is whitespace
                            state = 1;                      //in the whitespace state
                            nextChopStart = wrapPtr + 1;    //this will be start of next line
                        }
                    }
                    else
                    {                                       //in the whitespace state
                        if (!strchr("\t ", *wrapPtr))
                        {
                            wrapPtr++;                      //point to first of the whitespace
                            break;
                        }
                    }
                }
                //did we find a whitespace break to wrap on?
                if (wrapPtr > chopStart)
                {                                           //yes, we found some whitespace to wrap on
                    chopEnd = wrapPtr;
                }
                else
                {                                           //else no whitespace was found
                    nextChopStart = chopEnd;                //next line will start right at the character that went too long
                }
            }
            else
            {                                               //still fits, keep going
                continue;
            }

            //now, the string from chopStart to chopEnd will be a line.
            memcpy(chopBuffer + nBytesUsed, chopStart, chopEnd - chopStart);//copy the string into the buffer
            chopBuffer[nBytesUsed + chopEnd - chopStart] = 0;//NULL-terminate it
            choppedStrings[nChopped] = chopBuffer + nBytesUsed;//set pointer to the string
            dbgAssert(nBytesUsed < SUB_SubtitleLength + SUB_MaxLinesPerSubtitle);
            dbgAssert(nChopped < SUB_MaxLinesPerSubtitle);
            nBytesUsed += chopEnd - chopStart + 1;          //update usage count
            nChopped++;                                     //and number of strings

            //chop the next line
            chopStart = nextChopStart;                      //set to start scanning next line
            chopEnd = chopStart - 1;
        }
//    }

    fontMakeCurrent(current);                               //restore the old font
    return(nChopped);                                       //return number of strings chopped up
}

/*-----------------------------------------------------------------------------
    Name        : subCreateScrollyLines
    Description : Compute scroll and fade information for these strings.
    Inputs      : region - what scrolly text region we're adding them to
                  theme - what type theme we're using
                  nStrings - number of strings we're adding
                  strings - array of string pointers
                  speechTime - length of speech event, in seconds
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subCreateScrollyLines(subregion *region, subtheme *theme, sdword nStrings, char **strings, real32 speechTime)
{
    sdword index, linesToScroll, linesInRegion;
    sdword x, y;
    real32 scrollDistance;
    real32 scrollDuration;
    real32 delay, duration, scrollSpeed;
    real32 totalHeight;
    fonthandle fhSave = fontCurrentGet();

    fontMakeCurrent(theme->font);

    //figure out how far we'll have to scroll
    linesInRegion = (region->rect.y1 - region->rect.y0) / (fontHeight(" ") + SUB_InterlineSpacing);
    linesToScroll = nStrings - linesInRegion;
    totalHeight = (real32)(nStrings * (fontHeight(" ") + SUB_InterlineSpacing));
    scrollDistance = (real32)(linesToScroll * (fontHeight(" ") + SUB_InterlineSpacing));
    if (scrollDistance < 0.0f)
    {
        scrollDistance = 0.0f;                              //no scrolling needed
    }

    //how long will the scroll take?
    scrollDuration = speechTime - subScrollDwellStart - subScrollDwellEnd;

    //prep the region to scroll and display new text
    region->cardIndex = 0;                                  //kill all previous cards
    if (scrollDistance > 0.0f)
    {                                                       //if we are to scroll at all
        region->textScrollStart = *subTimeElapsed + subScrollDwellStart;
        if (scrollDuration < 0)
        {                                                   //does that make 0 scroll time after all the dwelling?
            region->textScrollEnd = region->textScrollStart + subScrollShortest;//set some minimum scroll duration (only needed if somebody speaks really fast :)
            //!!! this is weird... fix me please!!!
        }
        else
        {                                                   //else we can scroll as normal
            region->textScrollEnd = region->textScrollStart + scrollDuration;
        }
        region->scrollDistance = -scrollDistance;           //upward scroll is negative
        scrollSpeed = scrollDistance / (region->textScrollEnd - region->textScrollStart);
    }
    else
    {                                                       //else no scrolling
        scrollDuration = speechTime - subScrollDwellStart - subScrollDwellEnd;//just stay around for the duration of string
        region->scrollDistance = 0.0f;
        scrollSpeed = 0.0f;                                 //should never be divided by anyhow
    }

    //now that we've computed our scroll time, let's actually add the lines
    y = 0;
    delay = 0.0f;
    for (index = 0; index < nStrings; index++)
    {
#if SUB_ERROR_CHECKING
        if (index >= region->numberCards)
        {
            dbgFatalf(DBG_Loc, "Exceeded %d lines with the text '%s'.", region->numberCards, strings[index]);
        }
#endif
        if (index < linesInRegion)
        {                                                   //if first lines to appear
            delay = 0;                                      //make it show up right now
            if (index >= nStrings - linesInRegion)
            {                                               //string is starting and ending string
                duration = subScrollDwellStart + subScrollDwellEnd + scrollDuration;
            }
            else
            {                                               //else it'll scroll off the top
                duration = subScrollDwellStart + (real32)y / scrollSpeed;
            }
        }
        else
        {                                                   //else it will scroll onto screen
            //delay = (real32)y / scrollSpeed;                //this is when it scrolls into view
            delay = subScrollDwellStart + (real32)((index - linesInRegion) * (fontHeight(" ") + SUB_InterlineSpacing)) / scrollSpeed;
            if (index >= nStrings - linesInRegion)
            {                                               //if it is an ending string
                duration = (totalHeight - (real32)y) / scrollSpeed + subScrollDwellEnd;
            }
            else
            {                                               //else it will scroll on the bottom and off the top
                duration = (real32)(linesInRegion * (fontHeight(" ") + SUB_InterlineSpacing)) / scrollSpeed;
            }
        }
        if (theme->bCentred)
        {
            x = ((region->rect.x1 - region->rect.x0) - fontWidth(strings[index])) / 2;
        }
        else
        {
            x = 0;
        }
        region->card[index].x = x;
        region->card[index].y = y;
        region->card[index].creationTime = *subTimeElapsed + delay;
        region->card[index].duration = duration;
        region->card[index].fadeIn = theme->fadeIn;
        region->card[index].fadeOut = theme->fadeOut;
        region->card[index].c = theme->textColor;
        region->card[index].bDropShadow = theme->bDropShadow;
        region->card[index].font = theme->font;
        region->card[index].text = strings[index];

        y += fontHeight(" ") + SUB_InterlineSpacing;        //move the next line down
    }
    region->cardIndex = index;
    dbgAssert(region->cardIndex < region->numberCards);
    if (theme->bPicture)
    {
        region->picture = theme->picture;
    }
    else
    {
        region->picture = TR_InvalidHandle;
    }
    fontMakeCurrent(fhSave);
}

/*-----------------------------------------------------------------------------
    Name        : subCreateNonScrollyLines
    Description : Same as above, but the lines won't scroll; intead consecutive
                    pages will fade in and out.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void subCreateNonScrollyLines(subregion *region, subtheme *theme, sdword nStrings, char **strings, real32 speechTime)
{
    sdword index, linesInRegion;
    sdword x, y;
    real32 delay;
    sdword nPages, iPage;
    real32 timePerPage;
    fonthandle fhSave = fontCurrentGet();

    dbgAssert(nStrings != 0);
    fontMakeCurrent(theme->font);

    speechTime = max(speechTime, subTitleShortest);

    //prep the region
    region->cardIndex = 0;                                  //kill all previous cards
    region->scrollDistance = 0;

    //figure out how many pages there will be
    linesInRegion = (region->rect.y1 - region->rect.y0) / (fontHeight(" ") + SUB_InterlineSpacing);
    if (linesInRegion == 0)
    {
        linesInRegion = 1;
    }
    nPages = nStrings / linesInRegion;
    if (nStrings % linesInRegion)
    {
        nPages++;
    }
    timePerPage = speechTime / (real32)nPages;

    //now that we've computed our scroll time, let's actually add the lines
    for (index = 0; index < nStrings; index++)
    {
#if SUB_ERROR_CHECKING
        if (index >= region->numberCards)
        {
            dbgFatalf(DBG_Loc, "Exceeded %d lines with the text '%s'.", region->numberCards, strings[index]);
        }
#endif
        if (index < linesInRegion)
        {                                                   //if first lines to appear
            delay = 0;                                      //make it show up right now
        }
        else
        {                                                   //else it will scroll onto screen
            iPage = index / linesInRegion;                  //what page am I on?
            delay = (real32)iPage * timePerPage;
        }
        if (theme->bCentred)
        {
            x = ((region->rect.x1 - region->rect.x0) - fontWidth(strings[index])) / 2;
        }
        else
        {
            x = 0;
        }
        y = (index % linesInRegion) * (fontHeight(" ") + SUB_InterlineSpacing);
        region->card[index].x = x;
        region->card[index].y = y;
        region->card[index].creationTime = *subTimeElapsed + delay;
        region->card[index].duration = timePerPage;
        region->card[index].fadeIn = theme->fadeIn;
        region->card[index].fadeOut = theme->fadeOut;
        region->card[index].c = theme->textColor;
        region->card[index].bDropShadow = theme->bDropShadow;
        region->card[index].font = theme->font;
        region->card[index].text = strings[index];
    }
    region->cardIndex = index;
    dbgAssert(region->cardIndex < region->numberCards);
    if (theme->bPicture)
    {
        region->picture = theme->picture;
    }
    else
    {
        region->picture = TR_InvalidHandle;
    }
    fontMakeCurrent(fhSave);
    region->bAborted = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : subControlsScan
    Description : Scan for control escape sequences at the start of a string.
    Inputs      : start - start of string to parse
    Outputs     : region - region pointer to modify on #r<n> or #c
                  theme - theme pointer to modify on #t<n> or #c
                  bContinuous - set to TRUE on \+ or #+
    Return      : start of actual string
----------------------------------------------------------------------------*/
char *subControlsScan(char *start, subregion **region, subtheme **theme, bool8 *bContinuous)
{
    sdword scannedIndex;
    while ((*start == '#' || *start == '\\') && *start != 0)
    {
        start++;
        switch (*start)
        {
            case '+':
                *bContinuous = TRUE;
                break;
            case 'r':
                start++;
                scannedIndex = *start - '0';
                dbgAssert(scannedIndex >= 0 && scannedIndex < SUB_NumberRegions);
                *region = &subRegion[scannedIndex];
                break;
            case 't':
                start++;
                if (*start >= '0' && *start <= '9')
                {
                    scannedIndex = *start - '0';
                }
                else
                {
                    scannedIndex = 0xa + *start - 'a';
                }
                dbgAssert(scannedIndex >= 0 && scannedIndex < SUB_NumberThemes);
                *theme = &subTheme[scannedIndex];
                break;
            case 'c':
                *region = &subRegion[STR_CloseCaptioned];
                *theme = &subTheme[STT_CloseCaptioned];
                break;
#if SUB_ERROR_CHECKING
            default:
                dbgFatalf(DBG_Loc, "Invalid format character '%c' in '%s'", *start, start);
#endif
        }
        start++;
    }
    return(start);
}

/*-----------------------------------------------------------------------------
    Name        : subTitlesUpdate
    Description : Called every frame, this function updates subtitles,
                    handling asynchronous data transfer from the stream thread.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void subTitlesUpdate(void)
{
    sdword lastActor, lastSpeechEvent, totalLength, index;
    real32 totalTime;
    char fullNewStringBuffer[SUB_SubtitleLength], *fullNewString;
    char *choppedStrings[SUB_MaxLinesPerSubtitle];
    sdword nChoppedStrings;
    subregion *region;
    subtheme *theme;
    char *newString;
    bool8 bContinuous = FALSE, bContinuousIgnore;

    subTimeElapsed = &universe.totaltimeelapsed;
#if SUB_MODULE_TEST
    {
        sdword key;

        for (key = ONEKEY; key <= NINEKEY; key++)
        {
            if (keyIsStuck(key))
            {
                keyClearSticky(key);
                if (subTestSubtitle[key - ONEKEY] != NULL)
                {
                    subTitleAdd(0, subTestSubtitle[key - ONEKEY],
                                strlen(subTestSubtitle[key - ONEKEY]),
                                subTestSubtitleTime[key - ONEKEY]);
                }
            }
        }
    }
#endif //SUB_MODULE_TEST
    SDL_SemWait(subSemaphore);            //make sure nobody else is working with the subtitle transfer buffers
    if (subNumberNewSubtitles > 0)
    {
        //because subtitles are delivered as a series of phrases, we must "stitch"
        //them together here.  To do this, we compare the actor index of adjacent
        //new subtitles.  We may later want to add a more accurate method of
        //concatenation, such as the speech event index.
        for (index = 0; index < subNumberNewSubtitles; index++)
        {
            strcpy(fullNewStringBuffer, subNewSubtitles[index].text);//intialize the concatenation string
            lastActor = subNewSubtitles[index].actor;       //!!! this is the concatenation criteria
            region = &subRegion[STR_LetterboxBar];          //default region to print to
            theme = &subTheme[lastActor];                   //default theme for this actor
            lastSpeechEvent = subNewSubtitles[index].speechEvent;
            fullNewString = subControlsScan(fullNewStringBuffer, &region, &theme, &bContinuous);
            totalLength = strlen(fullNewString);
            totalTime = subNewSubtitles[index].time;
            while (index + 1 < subNumberNewSubtitles &&
                   subNewSubtitles[index + 1].actor == lastActor &&
                   subNewSubtitles[index + 1].speechEvent == lastSpeechEvent)
            {
                index++;
                dbgAssert(totalLength + strlen(subNewSubtitles[index].text) < SUB_SubtitleLength);
                strcat(fullNewString, " ");                 //put a space between phrases.
                totalLength++;
                newString = subNewSubtitles[index].text;
                newString = subControlsScan(subNewSubtitles[index].text, &region, &theme, &bContinuousIgnore);
                strcpy(fullNewString + totalLength, newString);//append the new string
                totalLength = strlen(fullNewString);        //get new total length
                totalTime += subNewSubtitles[index].time;   //add all the times together
            }
        }
#if SUB_VERBOSE_LEVEL >= 1
        dbgMessagef("\nSubtitle: actor %d says 0x%x '%s' (%.2f seconds).", lastActor, lastSpeechEvent, fullNewString, totalTime);
#endif

        //now that we have a concatenated string; let's chop it up to wrap it around
        nChoppedStrings = subStringsChop(&region->rect, theme->font, totalLength, fullNewString, region->chopBuffer, choppedStrings);

#if SUB_VERBOSE_LEVEL >= 2
        if (nChoppedStrings > 1)
        {
            sdword index;
            dbgMessage("\nChopped up like this:");

            for (index = 0; index < nChoppedStrings; index++)
            {
                dbgMessagef("\n'%s'", choppedStrings[index]);
            }
        }
#endif
        //string is all chopped up.  Let's figure out how long/far to scroll and how long to stick around at the ends.
        if (strCurLanguage >= 1)
        {                                                   //use the standard timing for English; scrolly timing for forign
            subCreateScrollyLines(region, theme, nChoppedStrings, choppedStrings, totalTime);
        }
        else
        {
            subCreateNonScrollyLines(region, theme, nChoppedStrings, choppedStrings, totalTime);
        }
        region->bContinuousEvent = bContinuous;
        subNumberNewSubtitles = 0;                          //discard all processed new subtitles
    }

    //notify by debug message if we missed any phrases
#if SUB_VERBOSE_LEVEL >= 1
    if (subMissedSubtitles > 0)
    {
        dbgMessagef("\nsubtitle: %d subtitles were missed.", subMissedSubtitles);
        subMissedSubtitles = 0;
    }
#endif
    SDL_SemPost(subSemaphore);
}

/*-----------------------------------------------------------------------------
    Name        : subTitleAdd
    Description : Display a subtitle.  This function called from the speech
                    streamer when a speech event is streamed in.
    Inputs      : actor - integer for an actor.  This is used to determine where
                    to print the speech even, plus fonts and whanot.
                  speechEvnt - what speechEvent it was, straight out of speechevent.h
                  text - text to print.  Must be in proper language when passed
                    here.
                  length - length of text string, same as what you would get
                    from a strlen() call.
                  time - length of text
    Outputs     :
    Return      : ?
----------------------------------------------------------------------------*/
sdword subTitleAdd(sdword actor, sdword speechEvent, char *text, sdword length, real32 time)
{
/*
#if SUB_VERBOSE_LEVEL >= 1
    dbgMessagef("\nSubtitle: actor %d says '%s'.", actor, text);
#endif
*/
    SDL_SemWait(subSemaphore);            //make sure nobody else is working with the subtitle transfer buffers
    if (subNumberNewSubtitles < SUB_NumberNewSubtitles - 1)
    {
        dbgAssert(length < SUB_SubtitleLength);
        subNewSubtitles[subNumberNewSubtitles].actor = actor;
        subNewSubtitles[subNumberNewSubtitles].speechEvent = speechEvent;
        subNewSubtitles[subNumberNewSubtitles].length = length;
        subNewSubtitles[subNumberNewSubtitles].time = time;
        //strcpy(subNewSubtitles[subNumberNewSubtitles].text, text);
        memcpy(subNewSubtitles[subNumberNewSubtitles].text, text, length);
        subNewSubtitles[subNumberNewSubtitles].text[length] = 0;
        subNumberNewSubtitles++;
    }
    else
    {
#if SUB_VERBOSE_LEVEL >= 1
        subMissedSubtitles++;
#endif
    }
    SDL_SemPost(subSemaphore);
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : subTitlesDraw
    Description : Render the subtitles in a given region.
    Inputs      : region - what region to render the subtitles for.
    Outputs     : may delete one or more of the text cards in the region
                    because they time out.
    Return      :void
----------------------------------------------------------------------------*/
void subTitlesDraw(subregion *region)
{
    sdword index;
    fonthandle fhSave = fontCurrentGet();
    char *lineStart;//, *newLine;
    sdword x, y;
    subcard *card;
    color c;
    real32 fadeValue;
    udword multiplier;
    sdword scroll;
    real32 scrollFloat;

#if SUB_VISUALIZE_REGION
    primRectSolid2(&region->rect, colBlack);
    primRectOutline2(&region->rect, 1, colWhite);
#endif

    if ((!mrRenderMainScreen && !smFleetIntel) && region == &subRegion[STR_LetterboxBar])
    {                                                       //if some full screen gui is up
        rndTextureEnable(FALSE);
        glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        //top scissor part
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(0));
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(NIS_LetterHeight));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(NIS_LetterHeight));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(0));
        glEnd();
        //ferDrawBoxRegion(region->rect, ferMediumTextures, ferInternalGlow, NULL, FALSE);
    }
    //scroll the text, if applicable
    scroll = region->rect.y0;
    if (region->scrollDistance != 0.0f)
    {                                                       //if there is a scroll to go on
        if (*subTimeElapsed >= region->textScrollStart)
        {                                                   //if the scroll has started
            if (*subTimeElapsed <= region->textScrollEnd)
            {                                               //if currently scrolling
                scrollFloat = region->scrollDistance * (*subTimeElapsed - region->textScrollStart) / (region->textScrollEnd - region->textScrollStart);
            }
            else
            {                                               //else scroll has ended
                scrollFloat = region->scrollDistance;
            }
            scroll = (sdword)scrollFloat + region->rect.y0;
        }
    }
    for (index = 0, card = region->card; index < region->cardIndex; index++, card++)
    {
        if (*subTimeElapsed >= card->creationTime)
        {                                                   //if card has started up properly
            if (*subTimeElapsed - card->creationTime > card->duration)
            {                                               //if card elapses
//                dbgMessagef("XXXX%10s", region->card[index].text);
                if (region->cardIndex > 1)
                {
                    region->card[index] = region->card[region->cardIndex - 1];
                }
                region->cardIndex--;
                index--;
                card = &region->card[index];
                continue;
            }
            fontMakeCurrent(card->font);

            lineStart = card->text;
            //newLine = strstr(lineStart, NIS_NewLine);
            x = card->x + region->rect.x0;
            y = card->y + scroll;
            //scroll the text, if applicable
    /*
            if (card->scroll != 0.0f)
            {                                                   //if the text is scrolling
                y += (sdword)(card->scroll *                    //move to scrolled-to position
                    (card->NIS->timeElapsed - card->creationTime) /
                    card->duration);
            }
    */
            c = card->c;
            //fade the text in, if applicable
            if (card->fadeIn != 0.0f)
            {
                fadeValue = (*subTimeElapsed - card->creationTime) / card->fadeIn;
                if (fadeValue >= 1.0f)
                {
                    card->fadeIn = 0.0f;
                    fadeValue = 1.0f;
                }
                multiplier = (udword)(fadeValue * 256.0f);
                c = colRGB(colRed(c) * multiplier / 256, colGreen(c) * multiplier / 256, colBlue(c) * multiplier / 256);
            }
            //fade the the text out, if applicable
            if (card->fadeOut != 0.0f)
            {                                                   //if there's a fadeout
                if (*subTimeElapsed >= card->creationTime + card->duration - card->fadeOut)
                {                                               //if it in the fading part
                    fadeValue = (*subTimeElapsed - (card->creationTime + card->duration - card->fadeOut)) / card->fadeOut;
                    fadeValue = max(0.0, 1.0f - fadeValue);
                    multiplier = (udword)(fadeValue * 256.0f);
                    c = colRGB(colRed(c) * multiplier / 256, colGreen(c) * multiplier / 256, colBlue(c) * multiplier / 256);
    //                dbgMessagef("<0x%2x>", multiplier);
                }
            }
            //draw the text
            /*
            while (newLine != NULL)
            {
                fontPrintN(x, y, card->c, lineStart, newLine - lineStart);
                y += fontHeight(" ") + 1;
                x = card->margin;
                newLine += NIS_NewLineLength;
                lineStart = newLine;
                newLine = strstr(newLine, NIS_NewLine);
            }
            */
            if (card->bDropShadow)
            {
                fontShadowSet(FS_SE, colBlack);
            }
            fontPrint(x, y, c, lineStart);
        }
    }

    if (region->picture != TR_InvalidHandle)
    {
        rectangle rect;

        rect.x0 = region->rect.x0 + region->iconOffsetX;
        rect.y0 = region->rect.y0 + region->iconOffsetY;
        rect.x1 = rect.x0 + SUB_PictureWidth;
        rect.y1 = rect.y0 + SUB_PictureHeight;
        trMakeCurrent(region->picture);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        if (RGLtype == SWtype)
        {
            glTexCoord2f((real32)(-rect.x0), (real32)(-rect.y1));
            glDrawPixels(SUB_PictureWidth, SUB_PictureHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        }
        else
        {
            primRectSolidTextured2(&rect);
        }
        glDisable(GL_BLEND);
    }

    if (region->cardIndex == 0 && region == &subRegion[STR_LetterboxBar] && (!region->bAborted) && !region->bContinuousEvent)
    {                                                       //if all fleet intel/command cards expired
        subMessageEnded = 1;
    }
    fontShadowSet(FS_NONE, colBlack);
    fontMakeCurrent(fhSave);
}

/*-----------------------------------------------------------------------------
    Name        : subTitlesFadeOut
    Description : Causes all remaining subtitles in a region to fade out over
                    a specified period.
    Inputs      : region - what region to fade teh subtitles out of
                  fadeTime - duration of fadeout
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void subTitlesFadeOut(subregion *region, real32 fadeTime)
{
    sdword index;
    subcard *card;

    dbgAssert(region != NULL);

    for (index = 0, card = region->card; index < region->cardIndex; index++, card++)
    {
        if (card->creationTime > *subTimeElapsed)
        {                                                   //if card not started created yet
            card->creationTime = *subTimeElapsed;
            card->duration = -1;                            //make sure it never gets drawn
            continue;
        }
        if (*subTimeElapsed - card->creationTime + fadeTime < card->duration)
        {                                                   //if the card isn't going to fade out before the time specified on it's own
            card->fadeOut = fadeTime;                       //start fading it out
            card->duration = *subTimeElapsed - card->creationTime + fadeTime;//kill it when it's faded
        }
    }
    region->bAborted = TRUE;
}

