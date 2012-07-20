/*=============================================================================
    Name    : ScenPick.c
    Purpose : Functionality to choose scenarios

    Created 10/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <dirent.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <limits.h>
#include "Types.h"
#include "Debug.h"
#include "File.h"
#include "Memory.h"
#include "FEFlow.h"
#include "utility.h"
#include "font.h"
#include "FontReg.h"
#include "prim2d.h"
#include "mouse.h"
#include "Scroller.h"
#include "ScenPick.h"
#include "texreg.h"
#include "render.h"
#include "BMP.h"
#include "MultiplayerGame.h"
#include "FEColour.h"
#include "StatScript.h"
#include "Subtitle.h"
#include "glcompat.h"
#include "Strings.h"

#ifdef _MSC_VER
    #define strcasecmp _stricmp
#endif

/*=============================================================================
    Data:
=============================================================================*/

char DefaultScenario[100] = "Default";

void spScenarioWindowInit(char *name, featom *atom);
void spScenarioNameDraw(featom *atom, regionhandle region);
void spScenarioBitmap(featom *atom, regionhandle region);

//void spScenarioBitmap(char *name, featom *atom);

fecallback spCallbacks[] =
{
    {spScenarioWindowInit, "CS_ScenarioWindowInit"},
    //{spScenarioBitmap, "CS_ScenarioBitmap"},
    {NULL, NULL}
};

fedrawcallback spDrawCallbacks[] =
{
    {spScenarioNameDraw,   "CS_ScenarioName"},
    {spScenarioBitmap, "CS_ScenarioBitmap"},
    {NULL, NULL}
};

listwindowhandle spScenarioListWindow = NULL;
//regionhandle     spScenarioBitmapWindow = NULL;

#ifdef _WIN32
char *ScenarioImagePath = "MultiPlayer\\";
#else
char *ScenarioImagePath = "MultiPlayer/";
#endif

lifheader *scenarioImage;
udword scenarioTexture = TR_InvalidInternalHandle;

sdword spTextureWidth, spTextureHeight;
ubyte* spTextureData = NULL;

bool spPickerStarted = FALSE;

#define SCP_TEXTURE_INSET 3


spscenario *spScenarios;                        //list of available scenarios
sdword spCurrentSelected = 0;                   //current scenario index, if OK is pressed
sdword spNumberScenarios = 0;                   //number of available scenarios
sdword spScenarioListLength = SCP_ScenarioListLength;//length of scenario list

fonthandle spListFont, spNameFont;              //fonts used for font printing

color spSelectedColor = SCP_SelectedColor;       //color to draw selected one at

uword spTopIndex = 0;                          //index of what is at the top of the list
uword spDisplayLength = SCP_DisplayListLength;  //number of scenarios displayed at one time

//text description stuff
sdword spNDescriptionLines = 0;                 //number of text description lines
char **spDescriptionLines = NULL;               //the actual lines
char *spDescription = NULL;                     //the actual description (may be chopped)
fonthandle spDescriptionFont = FONT_InvalidFontHandle;//font to print in
bool spDescriptionShadow = FALSE;                      //print with a dropshadow?
color spDescriptionColor = colWhite;            //print in what color?
GameType *gameTypeFromDescription = NULL;
static void spMissionDescriptionSet(char *directory,char *field,void *dataToFillIn);
static void spDescriptionFontSet(char *directory,char *field,void *dataToFillIn);
extern scriptStructEntry StaticMGInfoScriptTable[];
scriptEntry spDescriptionTweaks[] =
{
    {"Description", spMissionDescriptionSet, NULL},
    {"Font", spDescriptionFontSet, NULL},
    {"Color", scriptSetRGBCB, &spDescriptionColor},
    {"DropShadow", scriptSetBool8, &spDescriptionShadow},
    endEntry
};

scriptEntry spDescriptionTweaksFrench[] =
{
    {"DescriptionFrench", spMissionDescriptionSet, NULL},
    {"Font", spDescriptionFontSet, NULL},
    {"Color", scriptSetRGBCB, &spDescriptionColor},
    {"DropShadow", scriptSetBool8, &spDescriptionShadow},
    endEntry
};

scriptEntry spDescriptionTweaksGerman[] =
{
    {"DescriptionGerman", spMissionDescriptionSet, NULL},
    {"Font", spDescriptionFontSet, NULL},
    {"Color", scriptSetRGBCB, &spDescriptionColor},
    {"DropShadow", scriptSetBool8, &spDescriptionShadow},
    endEntry
};

scriptEntry spDescriptionTweaksSpanish[] =
{
    {"DescriptionSpanish", spMissionDescriptionSet, NULL},
    {"Font", spDescriptionFontSet, NULL},
    {"Color", scriptSetRGBCB, &spDescriptionColor},
    {"DropShadow", scriptSetBool8, &spDescriptionShadow},
    endEntry
};

scriptEntry spDescriptionTweaksItalian[] =
{
    {"DescriptionItalian", spMissionDescriptionSet, NULL},
    {"Font", spDescriptionFontSet, NULL},
    {"Color", scriptSetRGBCB, &spDescriptionColor},
    {"DropShadow", scriptSetBool8, &spDescriptionShadow},
    endEntry
};

/*=============================================================================
    Private functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : spScenarioItemDraw
    Description : Draw the list of available scenarios
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void spScenarioItemDraw(rectangle *rect, listitemhandle data)
{
    char            temp[64];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    spscenario     *spScenario = (spscenario *)data->data;

    oldfont = fontMakeCurrent(spListFont);

    if (data->flags&UICLI_Selected)
        c = FEC_ListItemSelected;
    else
        c = FEC_ListItemStandard;

    x = rect->x0;//+MG_HorzSpacing;
    y = rect->y0;//+MG_VertSpacing/2;

    sprintf(temp,"%s",spScenario->title);
    fontPrint(x,y,c,temp);

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : spScenarioNameDraw
    Description : Draw the currently selected scenario name in screens other
                    than the scenario picker.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void spScenarioNameDraw(featom *atom, regionhandle region)
{
    fonthandle fhSave;
    char scenarioName[128];
    bool useDisplayName = FALSE;
    unsigned int i;

    rectangle *r = &region->rect;

    fhSave = fontMakeCurrent(spNameFont); //select the appropriate font

    if (multiPlayerGame && (!GameCreator))
    {
        if (LANGame)
        {
            if (currentScreen == MGS_Basic_Options_View)
            {
                useDisplayName = TRUE;
            }
        }
        else
        {
            if (WaitingForGame)
            {
                useDisplayName = TRUE;
            }
        }
    }

    if (useDisplayName)
        memStrncpy(scenarioName, tpGameCreated.DisplayMapName, 127);
    else
        memStrncpy(scenarioName, spScenarios[spCurrentSelected].title, 127);
    for (i = 0; (scenarioName[i] = toupper(scenarioName[i])); i++) { }
    fontPrintf(r->x0, r->y0, FEC_ListItemSelected, scenarioName);
    fontMakeCurrent(fhSave);
}

/*-----------------------------------------------------------------------------
    Name        : spTitleFind
    Description : Finds the title of a mission file (on first line, in brackets)
    Inputs      : directory, fileName - combine to form path to the file.
    Outputs     : Allocated memory for and duplicates a string.
    Return      : Pointer to title string.
----------------------------------------------------------------------------*/
char *spTitleFind(char *directory, char *fileName)
{
    char string[256], fullName[PATH_MAX];
    filehandle handle;
    sdword status;

    memStrncpy(fullName, directory, PATH_MAX - 1);
    strcat(fullName, fileName);
#ifdef _WIN32
    strcat(fullName,"\\");
#else
    strcat(fullName,"/");
#endif
    strcat(fullName, fileName);
    strcat(fullName,".level");
    handle = fileOpen(fullName, FF_TextMode|FF_ReturnNULLOnFail);
    if (!handle)
    {
        return NULL;
    }

    for (;;)
    {
        status = fileLineRead(handle,string,256);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if ((string[0] == '\n') || (string[0] == '/') || (string[0] == ';') || (string[0] == ' '))
        {
            continue;
        }

        if (strlen(string) >= 3 && *string == '[' && string[strlen(string) - 1] == ']')
        {
            fileClose(handle);
            string[strlen(string) - 1] = 0;
            return(memStringDupe(&string[1]));
        }
    }

    fileClose(handle);

    return memStringDupe(fullName);
}

/*-----------------------------------------------------------------------------
    Name        : compareScenariosCB
    Description : compare's the scenarios alphabetically
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int compareScenariosCB(const void *arg1,const void *arg2)
{
    int result;

    spscenario *scenario1 = (spscenario *)arg1;
    spscenario *scenario2 = (spscenario *)arg2;

    result = strcasecmp(scenario1->title,scenario2->title);
    return result;
}

/*-----------------------------------------------------------------------------
    Name        : spScenarioFind
    Description : Find a scenario by name
    Inputs      : scenarioName - name of scenario to find
    Outputs     :
    Return      : Index of specified scenario or 0 if not found
----------------------------------------------------------------------------*/
sdword spScenarioFind(char *scenarioName)
{
    sdword index;

    for (index = 0; index < spNumberScenarios; index++)
    {
        if (strstr(spScenarios[index].title, scenarioName) == spScenarios[index].title)
        {                                               //if this is the default level
            return(index);
            spCurrentSelected = index;                  //set default scenario
            break;
        }
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : spTitleListLoad
    Description : Scans the missions directory and loads in the titles of all
                    the mission files available.
    Inputs      : void
    Outputs     : Fills in spScenarios and spNumberScenarios
    Return      : void
----------------------------------------------------------------------------*/
void spTitleListLoad(void)
{
#if !(defined(CGW) || defined (Downloadable) || defined(DLPublicBeta) || defined(OEM))
#ifdef _WIN32
    struct _finddata_t find;
    sdword handle, startHandle;
#else
    DIR* dp;
    struct dirent* dir_entry;
#endif
#endif
    sdword index, numplayers;
    char fileName[PATH_MAX], nameBuffer[PATH_MAX];
#if MAIN_Password
    char upperNameBuffer[PATH_MAX];
#endif
    char bitmapfileName[PATH_MAX];
    char *pString;
    char *title;
    filehandle scriptFile;
#if MAIN_Password
    unsigned int i;
#endif

#if defined(CGW) || defined(Downloadable) || defined(DLPublicBeta)
    scriptFile = fileOpen("DemoMissions.script", FF_TextMode);
#else
    //oem has all missions!
    scriptFile = fileOpen("multiPlayerMissions.script", FF_TextMode);
#endif
    dbgAssert(scriptFile != 0);

    while (fileLineRead(scriptFile, nameBuffer, PATH_MAX) != FR_EndOfFile)
    {
        if (nameBuffer[0] == ';' || (nameBuffer[0] == '/' && nameBuffer[1] == '/'))
        {
            continue;
        }
        numplayers = 0;
        for (pString = nameBuffer; *pString != 0; pString++)
        {                                                   //search for a numeral character
            if (strchr("0123456789", *pString) != NULL)
            {                                               //if this is a numeral
				sscanf( pString, "%d", &numplayers );
                memset(fileName, 0, PATH_MAX);
                memStrncpy(fileName, nameBuffer, pString - nameBuffer + 1);//copy the start of the string
                memStrncpy(bitmapfileName, nameBuffer, pString - nameBuffer + 1);//copy the start of the string
                strcat(fileName, "%d");                     //make something like:
                strcat(fileName, pString + 1);              //'StdGame%d.level'
            }
        }

        if (numplayers == 0)
        {
            goto alreadyLoaded;
        }

#ifdef _WIN32
        title = spTitleFind("MultiPlayer\\", nameBuffer);
#else
        title = spTitleFind("MultiPlayer/", nameBuffer);
#endif
        if (title == NULL)
        {
            goto alreadyLoaded;                             //break-continue
        }
#if MAIN_Password
        if (!mainEnableSpecialMissions)
        {                                                   //if this is an "off-limits" mission
            for (i = 0; (upperNameBuffer[i] = toupper(title[i])); i++) { }
            if (strstr(upperNameBuffer, "ALL"))
            {
                memFree(title);
                goto alreadyLoaded;                         //break-continue
            }
        }
#endif //MAIN_Password

        for (index = 0; index < spNumberScenarios; index++)
        {
            if (!strcasecmp(spScenarios[index].fileSpec, fileName))
            {                                               //if matching file specs,
                // matches, but we should update max number of player if necessary
                if (numplayers > spScenarios[index].maxplayers) spScenarios[index].maxplayers = numplayers;
                if (numplayers < spScenarios[index].minplayers) spScenarios[index].minplayers = numplayers;
                memFree(title);
                goto alreadyLoaded;                         //break-continue
            }
        }

        if (spNumberScenarios >= spScenarioListLength)
        {
            spScenarioListLength += SP_ScenarioListGrowth;
            spScenarios = memRealloc(spScenarios, spScenarioListLength * sizeof(spscenario), "spScenarios", NonVolatile);
        }
        dbgAssert(spNumberScenarios < spScenarioListLength);
        spScenarios[spNumberScenarios].fileSpec = memStringDupe(fileName);
        spScenarios[spNumberScenarios].bitmapfileSpec = memStringDupe(bitmapfileName);
        spScenarios[spNumberScenarios].title = title;
        spScenarios[spNumberScenarios].maxplayers = numplayers;
        spScenarios[spNumberScenarios].minplayers = numplayers;
        spNumberScenarios++;

alreadyLoaded:;
    }
    fileClose(scriptFile);

#if !(defined(CGW) || defined (Downloadable) || defined(DLPublicBeta) || defined(OEM))
#ifdef _WIN32
    startHandle = handle = _findfirst(filePathPrepend("MultiPlayer\\*.", 0), &find);

    while (handle != -1)
    {
        if (find.name[0] == '.')
        {
            goto alreadyLoadedFromFileSystem;
        }
        fileName[0] = 0;
        numplayers = 0;
        for (pString = find.name; *pString != 0; pString++)
        {                                                   //search for a numeral character
            if (strchr("0123456789", *pString) != NULL)
            {                                               //if this is a numeral
				sscanf( pString, "%d", &numplayers );
                memset(fileName, 0, PATH_MAX);
                strncpy(fileName, find.name, pString - find.name);//copy the start of the string
                memStrncpy(bitmapfileName, find.name, pString - find.name + 1);//copy the start of the string
                strcat(fileName, "%d");                     //make something like:
                strcat(fileName, pString + 1);              //'StdGame%d.level'
            }
        }
        if (numplayers == 0)
        {
            goto alreadyLoadedFromFileSystem;
        }

        if (fileName[0] == 0)
        {
            goto alreadyLoadedFromFileSystem;
        }

        title = spTitleFind("MultiPlayer\\", find.name);
        if (title == NULL)
        {
            goto alreadyLoadedFromFileSystem;
        }
#if MAIN_Password
        if (!mainEnableSpecialMissions)
        {                                                   //if this is an "off-limits" mission
            for (i = 0; (upperNameBuffer[i] = toupper(title[i])); i++) { }
            if (strstr(upperNameBuffer, "ALL"))
            {
                memFree(title);
                goto alreadyLoadedFromFileSystem;           //break-continue
            }
        }
#endif //MAIN_Password

        for (index = 0; index < spNumberScenarios; index++)
        {
            if (!strcasecmp(spScenarios[index].fileSpec, fileName))
            {                                               //if matching file specs,
                // matches, but we should update max number of player if necessary
                if (numplayers > spScenarios[index].maxplayers) spScenarios[index].maxplayers = numplayers;
                if (numplayers < spScenarios[index].minplayers) spScenarios[index].minplayers = numplayers;
                memFree(title);
                goto alreadyLoadedFromFileSystem;                         //break-continue
            }
        }

        if (spNumberScenarios >= spScenarioListLength)
        {
            spScenarioListLength += SP_ScenarioListGrowth;
            spScenarios = memRealloc(spScenarios, spScenarioListLength * sizeof(spscenario), "spScenarios", NonVolatile);
        }
        dbgAssert(spNumberScenarios < spScenarioListLength);
        spScenarios[spNumberScenarios].fileSpec = memStringDupe(fileName);
        spScenarios[spNumberScenarios].bitmapfileSpec = memStringDupe(bitmapfileName);
        spScenarios[spNumberScenarios].title = title;
        spScenarios[spNumberScenarios].maxplayers = numplayers;
        spScenarios[spNumberScenarios].minplayers = numplayers;
        spNumberScenarios++;

alreadyLoadedFromFileSystem:;
        handle = _findnext(startHandle, &find);
    }
#else   /* File search, not _WIN32... */
    dp = opendir(filePathPrepend("MultiPlayer", 0));

    if (dp)
    {
        while ((dir_entry = readdir(dp)))
        {
            if (dir_entry->d_name[0] == '.')
                continue;

            fileName[0] = 0;
            numplayers = 0;
            while ((pString = strpbrk(pString, "0123456789")))
            {                                               /*search for a numeral character*/
				sscanf( pString, "%d", &numplayers );
                memset(fileName, 0, PATH_MAX);
                strncpy(fileName, dir_entry->d_name,
                    pString - dir_entry->d_name);           /*copy the start of the string*/
                memStrncpy(bitmapfileName, dir_entry->d_name,
                    pString - dir_entry->d_name + 1);       /*copy the start of the string*/
                strcat(fileName, "%d");                     /*make something like:*/
                strcat(fileName, pString + 1);              /*'StdGame%d.level'*/
            }

            if (numplayers == 0)
                continue;

            if (fileName[0] == '\0')
                continue;

            title = spTitleFind("MultiPlayer/", dir_entry->d_name);
            if (title == NULL)
                continue;
#if MAIN_Password
            if (!mainEnableSpecialMissions)
            {                                               /*if this is an "off-limits" mission*/
                for (i = 0; (upperNameBuffer[i] = toupper(title[i])); i++) { }
                if (strstr(upperNameBuffer, "ALL"))
                {
                    memFree(title);
                    continue;           /*break-continue*/
                }
            }
#endif //MAIN_Password

            for (index = 0; index < spNumberScenarios; index++)
            {
                if (strcasecmp(spScenarios[index].fileSpec, fileName))
                    continue;

                /* matches, but we should update max number of player if necessary */
                if (numplayers > spScenarios[index].maxplayers) spScenarios[index].maxplayers = numplayers;
                if (numplayers < spScenarios[index].minplayers) spScenarios[index].minplayers = numplayers;
                memFree(title);
                break;                         /*break-continue*/
            }
            if (index < spNumberScenarios)
                continue;

            if (spNumberScenarios >= spScenarioListLength)
            {
                spScenarioListLength += SP_ScenarioListGrowth;
                spScenarios = memRealloc(spScenarios, spScenarioListLength * sizeof(spscenario), "spScenarios", NonVolatile);
            }
            dbgAssert(spNumberScenarios < spScenarioListLength);
            spScenarios[spNumberScenarios].fileSpec = memStringDupe(fileName);
            spScenarios[spNumberScenarios].bitmapfileSpec = memStringDupe(bitmapfileName);
            spScenarios[spNumberScenarios].title = title;
            spScenarios[spNumberScenarios].maxplayers = numplayers;
            spScenarios[spNumberScenarios].minplayers = numplayers;
            spNumberScenarios++;
        }

        closedir(dp);
    }
#endif   /* _WIN32 */
#endif //defined(CGW) || defined (Downloadable) || defined(DLPublicBeta) || defined(OEM)

    dbgAssert(spNumberScenarios > 0);
    if (spNumberScenarios > 1)
    {
        //alphabetically sort the scenario list
        qsort(&spScenarios[0],spNumberScenarios,sizeof(spscenario),compareScenariosCB);
    }

    //find the default scenario's index
    if (spCurrentSelected == 0)
    {                                                       //if it's not already selected
        spCurrentSelected = spScenarioFind(DefaultScenario);
    }
}

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : spStartup
    Description :
    Inputs      :
    Outputs     : Registers some callbacks for buttons and user regions.
    Return      :
----------------------------------------------------------------------------*/
void spStartup(void)
{
    feDrawCallbackAddMultiple(spDrawCallbacks);
    spListFont = frFontRegister(SCP_ListFont);
    spNameFont = frFontRegister(SCP_NameFont);
    spScenarios = memAlloc(spScenarioListLength * sizeof(spscenario), "spScenarios", NonVolatile);
    spTitleListLoad();
    if (spCurrentSelected >= spNumberScenarios)
    {
        spCurrentSelected = spScenarioFind(DefaultScenario);
    }

}

/*-----------------------------------------------------------------------------
    Name        : spShutdown
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void spShutdown(void)
{
    sdword index;

    for (index = 0; index < spNumberScenarios; index++)
    {
        memFree(spScenarios[index].bitmapfileSpec);
        memFree(spScenarios[index].fileSpec);
        memFree(spScenarios[index].title);
    }
    memFree(spScenarios);
}


void spNewItem(void);

/*-----------------------------------------------------------------------------
    Name        : spScenarioWindowInit
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void spScenarioWindowInit(char *name, featom *atom)
{
    fonthandle  oldfont;
    sdword      index;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(spListFont);

        spScenarioListWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(spScenarioListWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0/*fontHeight(" ")*2*/,                // title height, no title
                          spScenarioItemDraw,               // item draw function
                          fontHeight(" ")+SCP_VertSpacing,   // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);


        //spScenarioListWindow->reg.flags |=

        for (index = 0; index < spNumberScenarios; index++)
        {
            if (index==spCurrentSelected)
                uicListAddItem(spScenarioListWindow, (ubyte *)&spScenarios[index], UICLI_CanSelect|UICLI_Selected, UICLW_AddToTail);
            else
                uicListAddItem(spScenarioListWindow, (ubyte *)&spScenarios[index], UICLI_CanSelect, UICLW_AddToTail);
        }

        fontMakeCurrent(oldfont);
        return;
    }
    else
    {
        switch (spScenarioListWindow->message)
        {
            case CM_AcceptText:
                spDonePicking(NULL,NULL);
                break;
            case CM_DoubleClick:

                spDonePicking(NULL,NULL);
                break;
            case CM_NewItemSelected:
                spNewItem();

        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : spScenarioPick
    Description : Starts the scenario picker
    Inputs      : dest - where to store file name of destiation file.
                    Will be "" if nothing (yet) chosen.
    Outputs     : Starts the scenario picker screen.  When chosen, the name
                    will be stored in dest.
    Return      : void
----------------------------------------------------------------------------*/
void spScenarioPick(char *dest)
{
    feCallbackAddMultiple(spCallbacks);

    feScreenStart(ghMainRegion, SCP_ScreenName);

    spNewItem();

    spPickerStarted = TRUE;
}

bool spSelectionValid(sdword candidate)
{
    if (tpGameCreated.numPlayers + tpGameCreated.numComputers > spScenarios[candidate].maxplayers)
    {
        if (tpGameCreated.numPlayers > spScenarios[candidate].maxplayers)
        {
            return FALSE; // too many people for this map
        }
        else
        {
            // let's bump computers out for humans to join
            tpGameCreated.numComputers = spScenarios[candidate].maxplayers - tpGameCreated.numPlayers;
            // this line caused a warning in release build, numComputers is always >= 0 it is unsigned!
            //dbgAssert(tpGameCreated.numComputers >= 0);
            dbgAssert(tpGameCreated.numPlayers + tpGameCreated.numComputers <= spScenarios[candidate].maxplayers);
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : spDescriptionDefaultsAndFreeText
    Description : Set text description defaults.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void spDescriptionDefaultsAndFreeText(void)
{
    if (spDescription != NULL)
    {                                                       //if there was a description
        memFree(spDescription);                             //kill it
        spDescription = NULL;
        memFree(spDescriptionLines);
        spNDescriptionLines = 0;
    }
    spDescriptionFont = FONT_InvalidFontHandle;             //set printing defaults
    spDescriptionShadow = FALSE;
    spDescriptionColor = colWhite;
    if (gameTypeFromDescription != NULL)
    {
        memFree(gameTypeFromDescription);
        gameTypeFromDescription = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : spDonePicking/spBackPicking
    // if the region is the scenario picker
    Description : Close the color picker and optionally specify the file path
                    of the chosen file.
    Inputs      : standard FE callback
    Outputs     : DonePicking stores chosen file name in parameter to spScenarioPick
    Return      : void
----------------------------------------------------------------------------*/
void spDonePicking(char *name, featom *atom)
{
    sdword index;
    sdword spCurrentSelectedCandidate = spCurrentSelected;

    if (spScenarioListWindow->CurLineSelected!=NULL)
    {
        for (index = 0; index < spNumberScenarios; index++)
        {
            if (spScenarioListWindow->CurLineSelected->data == (ubyte *)&spScenarios[index])
            {
                spCurrentSelectedCandidate = index;
                break;
            }
        }
    }

    if (gameTypeFromDescription != NULL)
    {                                                       //if there is a game type from description file
        mgSetGameTypeByStruct(gameTypeFromDescription);     //set that game type
        feAllCallOnCreate(feStack[feStackIndex - 1].screen);//update buttons to reflect changes
        mgGameTypesOtherButtonPressed();
    }
    spDescriptionDefaultsAndFreeText();

    if (scenarioTexture != TR_InvalidInternalHandle)
    {                                                       //delete the preview image, if there was one
        trRGBTextureDelete(scenarioTexture);
        scenarioTexture = TR_InvalidInternalHandle;
        if (spTextureData != NULL)
        {
            memFree(spTextureData);
            spTextureData = NULL;
        }
    }

    if (spSelectionValid(spCurrentSelectedCandidate))
    {
        spCurrentSelected = spCurrentSelectedCandidate;
    }

    feScreenDisappear(NULL, NULL);
    spPickerStarted = FALSE;
}

void spBackPicking(char *name, featom *atom)
{
    spDescriptionDefaultsAndFreeText();

    if (scenarioTexture != TR_InvalidInternalHandle)
    {                                                       //delete the preview image, if there was one
        trRGBTextureDelete(scenarioTexture);
        scenarioTexture = TR_InvalidInternalHandle;
        if (spTextureData != NULL)
        {
            memFree(spTextureData);
            spTextureData = NULL;
        }
    }

    feScreenDisappear(NULL, NULL);
    spPickerStarted = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : spFindMap
    Description : Sets the current selected scenario to the mapname specified
    Inputs      :
    Outputs     : DonePicking stores chosen file name in parameter to spScenarioPick
    Return      : void
----------------------------------------------------------------------------*/
void spFindMap(char *MapName)
{
    sdword index;

    for (index = 0; index < spNumberScenarios; index++)
    {
        if (strcasecmp(spScenarios[index].title,MapName)==0)
        {
            spCurrentSelected = index;
            break;
        }
    }
}




/*-----------------------------------------------------------------------------
    Name        : spFindImage
    Description : Attempt to find the preview image for a given scenario
    Inputs      : scenarioFile - name of scenario
    Outputs     : bitmapFile - name of preview.bmp file if found
                  textFile - name of description.txt file if found
    Return      : void
----------------------------------------------------------------------------*/
void spFindImage(char* bitmapFile, char *textFile, char *scenarioFile)
{
    sdword i;
    bool foundBitmap = FALSE, foundText = FALSE;

    for (i = 2; i<=8; i++)
    {
#if 0       // if 0 out because it's a bug in my bug list
        if (!foundBitmap)
        {
            sprintf(bitmapFile, "%s%s%c\\preview.bmp", ScenarioImagePath, scenarioFile, '0' + i);
            if (fileExists(bitmapFile, 0))
            {                                               //see if there is a preview bitmap in this directory
                foundBitmap = TRUE;
            }
        }
#endif
        if (!foundText)
        {
#ifdef _WIN32
            sprintf(textFile, "%s%s%c\\description.txt", ScenarioImagePath, scenarioFile, '0' + i);
#else
            sprintf(textFile, "%s%s%c/description.txt", ScenarioImagePath, scenarioFile, '0' + i);
#endif
            if (fileExists(textFile, 0))
            {                                               //see if there is a description text file in this directory
                foundText = TRUE;
            }
        }
    }
    if (!foundBitmap)
    {
        *bitmapFile = 0;
    }
    if (!foundText)
    {
        *textFile = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : spMissionDescriptionSet
    Description : Script-parse callback for setting description text
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void spMissionDescriptionSet(char *directory,char *field,void *dataToFillIn)
{
    if (spDescription == NULL)
    {
        spDescription = memStringDupe(field);
    }
    else
    {
        spDescription = memRealloc(spDescription, strlen(spDescription) + strlen(field) + 3, "DescriptionString", 0);
        strcat(spDescription, "\\n");                       //auto-newline
        strcat(spDescription, field);
    }
}

/*-----------------------------------------------------------------------------
    Name        : spDescriptionFontSet
    Description : Script-parsing callback to set the font for the description text.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void spDescriptionFontSet(char *directory,char *field,void *dataToFillIn)
{
    spDescriptionFont = frFontRegister(field);
}

/*-----------------------------------------------------------------------------
    Name        : spNewItem
    Description : Called when a new item in the color picker list is selected.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void spNewItem(void)
{
    char bitmapFile[PATH_MAX];
    char descriptionFile[PATH_MAX];
    char *chopBuffer;
    sdword scen;
    rectangle textureRect;
//    rectangle rect = spScenarioBitmapWindow->rect;
    color *bmpBuffer;
    filehandle handle;
    bmpheader header;
    featom *atom;

    scen = spScenarioListWindow->CurLineSelected->position;
    atom = feAtomFindInScreen(feStack[feStackIndex].screen, "CS_ScenarioBitmap");
    dbgAssert(atom != NULL);

    if (scenarioTexture != TR_InvalidInternalHandle)
    {                                                       //delete the old texture before creating a new one
        trRGBTextureDelete(scenarioTexture);
        scenarioTexture = TR_InvalidInternalHandle;
        if (spTextureData != NULL)
        {
            memFree(spTextureData);
            spTextureData = NULL;
        }
    }

    //free the previous text and set defaults
    spDescriptionDefaultsAndFreeText();

    spFindImage(bitmapFile, descriptionFile, spScenarios[scen].bitmapfileSpec);
    if (strlen(bitmapFile) > 0)
    {
        handle = bmpFileOpen(&header, bitmapFile);         //open the file
        if (handle != 0)
        {                                                   //if opened properly
            if (header.biWidth < SCP_PreviewWidthMin || header.biHeight > SCP_PreviewWidthMax)
            {
#if SCP_VERBOSE_LEVEL >= 1
                dbgMessagef("\n'%s': width of %d bad.", bitmapFile, header.biWidth);
#endif
                bmpClose(handle);
                return;
            }
            else if (header.biHeight < SCP_PreviewHeightMin || header.biHeight > SCP_PreviewHeightMax)
            {
#if SCP_VERBOSE_LEVEL >= 1
                dbgMessagef("\n'%s': height of %d bad.", bitmapFile, header.biHeight);
#endif
                bmpClose(handle);
                return;
            }
            //allocate the image buffer
            bmpBuffer = memAlloc(header.biWidth * header.biHeight * sizeof(color), "TempBMPbuffer", Pyrophoric);
            bmpBodyRead(bmpBuffer, handle, &header);        //read in the body of the file
            if (header.biWidth != SCP_PreviewWidth || header.biHeight != SCP_PreviewHeight)
            {                                               //if the image is the wrong size
                //rescale the image to the proper size
                bmpBuffer = trImageScale(bmpBuffer, header.biWidth, header.biHeight, SCP_PreviewWidth, SCP_PreviewHeight, TRUE);
                header.biWidth = SCP_PreviewWidth;           //make it think we're the right size
                header.biHeight = SCP_PreviewHeight;
            }
            scenarioTexture = trRGBTextureCreate(bmpBuffer, header.biWidth, header.biHeight, FALSE);
            spTextureWidth  = header.biWidth;
            spTextureHeight = header.biHeight;
            if (spTextureData != NULL)
            {
                memFree(spTextureData);
            }
            spTextureData = (ubyte*)memAlloc(4*spTextureWidth*spTextureHeight, "scenpick data", NonVolatile);
            memcpy(spTextureData, bmpBuffer, 4*spTextureWidth*spTextureHeight);
            memFree(bmpBuffer);                             //free the image
        }
        else
        {
            return;
        }
    }
    else
    {
#if SCP_VERBOSE_LEVEL >= 1
        dbgMessage("\nNo preview image found!");
#endif
        textureRect.x0 = atom->x + SCP_TEXTURE_INSET;
        textureRect.y0 = atom->y + SCP_TEXTURE_INSET;
        textureRect.x1 = atom->x + atom->width - SCP_TEXTURE_INSET;
        textureRect.y1 = atom->y + atom->height - SCP_TEXTURE_INSET;
        primRectSolid2(&textureRect, colBlack);
    }

    if (strlen(descriptionFile) > 0)
    {                                                       //if there is a description file
        // load in the proper description based on the language
        if (strCurLanguage==languageEnglish)
        {
            scriptSet(NULL, descriptionFile, spDescriptionTweaks);//load in the description text
        }
        else if (strCurLanguage==languageFrench)
        {
            scriptSet(NULL, descriptionFile, spDescriptionTweaksFrench);//load in the description text
            if (spDescription==NULL)
            {
                // if french isn't found load in english
                scriptSet(NULL, descriptionFile, spDescriptionTweaks);//load in the description text
            }
        }
        else if (strCurLanguage==languageGerman)
        {
            scriptSet(NULL, descriptionFile, spDescriptionTweaksGerman);//load in the description text
            if (spDescription==NULL)
            {
                // if german isn't found load in english
                scriptSet(NULL, descriptionFile, spDescriptionTweaks);//load in the description text
            }
        }
        else if (strCurLanguage==languageSpanish)
        {
            scriptSet(NULL, descriptionFile, spDescriptionTweaksSpanish);//load in the description text
            if (spDescription==NULL)
            {
                // if german isn't found load in english
                scriptSet(NULL, descriptionFile, spDescriptionTweaks);//load in the description text
            }
        }
        else if (strCurLanguage==languageItalian)
        {
            scriptSet(NULL, descriptionFile, spDescriptionTweaksItalian);//load in the description text
            if (spDescription==NULL)
            {
                // if german isn't found load in english
                scriptSet(NULL, descriptionFile, spDescriptionTweaks);//load in the description text
            }
        }

        if (spDescription != NULL)
        {                                                   //if there was real text in that file
            if (spDescriptionFont == FONT_InvalidFontHandle)
            {                                               //make sure we have a font
                spDescriptionFont = frFontRegister("default.hff");
            }
            textureRect.x0 = atom->x + SCP_TEXTURE_INSET;   //and a region to print into
            textureRect.y0 = atom->y + SCP_TEXTURE_INSET;
            textureRect.x1 = atom->x + atom->width - SCP_TEXTURE_INSET;
            textureRect.y1 = atom->y + atom->height - SCP_TEXTURE_INSET;

            spDescriptionLines = memAlloc(sizeof(char *) * (textureRect.y1 - textureRect.y0), "DescLines", NonVolatile);//allocate the chopping pointers
            chopBuffer = memAlloc(strlen(spDescription) + textureRect.y1 - textureRect.y0, "DescChopped", NonVolatile);//allocate chop buffer
            spNDescriptionLines = subStringsChop(&textureRect, spDescriptionFont, strlen(spDescription), spDescription, chopBuffer, spDescriptionLines);
            memFree(spDescription);                         //free unchopped description
            spDescription = chopBuffer;                     //keep pointer to chopped description
        }
        gameTypeFromDescription = memAlloc(sizeof(GameType), "DescGameType", 0);
        memset(gameTypeFromDescription, 0xff, sizeof(GameType));       //flag all members as unused
        gameTypeFromDescription->flag = gameTypeFromDescription->flagNeeded = 0;    //flag none of the flags needed

        scriptSetStruct(NULL, descriptionFile, StaticMGInfoScriptTable, (ubyte *)gameTypeFromDescription);
    }

    if (strlen(descriptionFile) > 0 || strlen(bitmapFile) > 0)
    {                                                       //if there is a description file or a bitmap
        if (spPickerStarted)
        {
            spScenarioBitmap(atom, (regionhandle)atom->region);//draw it/them
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : spScenarioBitmap
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void spScenarioBitmap(featom *atom, regionhandle region)
{
    rectangle textureRect;
    sdword index, y;
    fonthandle oldFont;

    textureRect.x0 = region->rect.x0 + SCP_TEXTURE_INSET;
    textureRect.y0 = region->rect.y0 + SCP_TEXTURE_INSET;
    textureRect.x1 = region->rect.x1 - SCP_TEXTURE_INSET;
    textureRect.y1 = region->rect.y1 - SCP_TEXTURE_INSET;

    //draw the bitmap...
    if (scenarioTexture != TR_InvalidInternalHandle)
    {
        if (glcActive())
        {
            glcRectSolidTexturedScaled2(&textureRect,
                                        spTextureWidth, spTextureHeight,
                                        spTextureData, NULL, TRUE);
        }
        else
        {
            trRGBTextureMakeCurrent(scenarioTexture);
            rndPerspectiveCorrection(FALSE);
            primRectSolidTextured2(&textureRect);               //draw the bitmap
        }
        feStaticRectangleDraw(region);                      //draw a border
    }
    //draw the description text...
    if (spDescription != NULL)
    {                                                       //if there is description text
        dbgAssert(spDescriptionFont != FONT_InvalidFontHandle);
        oldFont = fontMakeCurrent(spDescriptionFont);       //set the font
        if (spDescriptionShadow)
        {                                                   //optionally enable dropshadow
            fontShadowSet(FS_SE, colBlack);
        }
        for (index = 0, y = textureRect.y0; index < spNDescriptionLines; index++)
        {                                                   //draw each line
            dbgAssert(spDescriptionLines[index] != NULL);
            if (y + fontHeight(" ") >= textureRect.y1)
            {                                               //if this line will extend off bottom of region
                break;
            }
            fontPrint(textureRect.x0, y, spDescriptionColor, spDescriptionLines[index]);
            y += fontHeight(" ") + 1;
        }
        fontShadowSet(FS_NONE, colBlack);
        fontMakeCurrent(oldFont);
    }
}
