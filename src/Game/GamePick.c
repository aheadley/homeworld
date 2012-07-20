/*=============================================================================
    Name    : GamePick.c
    Purpose : Functionality to choose a game

    Created 10/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
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
#include "Scroller.h"
#include "SaveGame.h"
#include "GamePick.h"
#include "MultiplayerGame.h"
#include "Universe.h"
#include "NetCheck.h"
#include "Globals.h"
#include "FEColour.h"
#include "StringsOnly.h"
#include "CommandWrap.h"
#include "SinglePlayer.h"
#include "mainrgn.h"
#include "SoundEvent.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

/*=============================================================================
    Data:
=============================================================================*/

bool gpQuickSave(void);
bool gpQuickLoad(void);

void gpRecGameWindowInit(char *name, featom *atom);
void gpGameWindowInit(char *name, featom *atom);
void gpTextEntryWindowInit(char *name, featom *atom);
void gpGameNameDraw(featom *atom, regionhandle region);
void gpSaveGivenGame(char* gamename);
void gpLoadGivenGame(char* gamename);
void gpSaveGame(char *name, featom *atom);
void gpLoadGame(char *name, featom *atom);
void gpLoadTheRecordedGame(char *name, featom *atom);
void gpSaveTheRecordedGame(char *name, featom *atom);
void gpDeleteGame(char *name, featom *atom);
void gpBackToInGameEscapeMenu(char *name, featom *atom);
void gpStopPlayback(char *name, featom *atom);
void gpStopRecording(char *name, featom *atom);

void gpOverwriteYes(char *name, featom *atom);
void gpOverwriteNo(char *name, featom *atom);

bool gpLoadSinglePlayerGame = FALSE;
bool gpLoadTutorial = FALSE;

#ifdef _WIN32
//SinglePlayerSavedGamesPath is non-static because it is used in KASFunc.c
char SinglePlayerSavedGamesPath[] = "SavedGames\\SinglePlayer\\";
static char MultiPlayerSavedGamesPath[] = "SavedGames\\MultiPlayer\\";
static char RecordedGamesPath[] = "SavedGames\\RecordedGames\\";
//TutorialSavedGamesPath is non-static because it is used in Tutor.c
char TutorialSavedGamesPath[] = "SavedGames\\Training\\";
#else
//SinglePlayerSavedGamesPath is non-static because it is used in KASFunc.c
char SinglePlayerSavedGamesPath[] = "SavedGames/SinglePlayer/";
static char MultiPlayerSavedGamesPath[] = "SavedGames/MultiPlayer/";
static char RecordedGamesPath[] = "SavedGames/RecordedGames/";
//TutorialSavedGamesPath is non-static because it is used in Tutor.c
char TutorialSavedGamesPath[] = "SavedGames/Training/";
#endif
static char QuickSaveName[] = "Quick Save";

char *SavedGamesPath = NULL;

fecallback gpCallbacks[] =
{
    {gpGameWindowInit,         "FE_RecordedGameWindowInit"},
    {gpGameWindowInit,         "FE_GameWindowInit"},
    {gpGameWindowInit,         "FE_TutorialGameWindowInit"},
    {gpTextEntryWindowInit,    "IG_TextEntryWindowInit"},
    {gpStopPlayback,           "IG_StopPlayback"},
    {gpStopRecording,          "IG_StopRecording"},
    {gpLoadGame,               "IG_LoadGame"},
    {gpLoadTheRecordedGame,    "LoadRecordedGame"},
    {gpSaveTheRecordedGame,    "IG_SaveRecordedGame"},
    {gpSaveGame,               "IG_SaveGame"},
    {gpDeleteGame,             "IG_DeleteGame"},
    {gpBackToInGameEscapeMenu, "IG_StartEscapeMenu"},
    //{gpAcceptBackToInGameEscapeMenu, "IG_StartEscapeMenu_AcceptOptions"},
    {gpOverwriteYes,           "OverwriteGameYes" },
    {gpOverwriteNo,            "OverwriteGameNo" },
    {NULL, NULL}
};

fedrawcallback gpDrawCallbacks[] =
{
    {gpGameNameDraw, "FE_GameName"},
    {NULL, NULL}
};

listwindowhandle gpGameListWindow = NULL;
textentryhandle  gpNameEntryBox   = NULL;

enum {
    ESCAPE_DEFAULT = 0,
    ESCAPE_SINGLE,
    ESCAPE_RECORDING,
    ESCAPE_PLAYING,
    ESCAPE_TUTORIAL,
    ESCAPE_DEFAULT_NOSAVE,
    ESCAPE_RECORDING_NOSAVE
};

/*=============================================================================*/

gpgame *gpGames;                                //list of available games
sdword gpCurrentGame = 0;                       //current game index, 0 is always default
sdword gpCurrentSelected = 0;                   //current game index, if OK is pressed
sdword gpNumberGames = 0;                       //number of available games
char   *gpTextEntryName;

fonthandle gpListFont, gpNameFont;              //fonts used for font printing

color gpSelectedColor = GP_SelectedColor;       //color to draw selected one at

uword gpTopIndex = 0;                          //index of what is at the top of the list
uword gpDisplayLength = GP_DisplayListLength;  //number of games displayed at one time

sdword escapemenu;                              //remember what scren to return to

/*=============================================================================
    Private functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : gpGameItemDraw
    Description : Draw the list of available games
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpGameItemDraw(rectangle *rect, listitemhandle data)
{
    char            temp[64];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    gpgame         *gpGame = (gpgame *)data->data;

    oldfont = fontMakeCurrent(gpListFont);

    if (data->flags&UICLI_Selected)
        c = FEC_ListItemSelected;
    else
        c = FEC_ListItemStandard;

    x = rect->x0;
    y = rect->y0;

    sprintf(temp,"%s",gpGame->title);
    fontPrint(x,y,c,temp);

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : gpGameNameDraw
    Description : Draw the currently selected game name in screens other
                    than the game picker.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpGameNameDraw(featom *atom, regionhandle region)
{
    fonthandle fhSave;
    char gameName[128];
    char curr_ch;
    unsigned int i;

    rectangle *r = &region->rect;

    fhSave = fontMakeCurrent(gpNameFont); //select the appropriate font
    for (i = 0; i < 127 && (curr_ch = gpGames[gpCurrentSelected].title[i]); i++)
        gameName[i] = toupper(curr_ch);
    gameName[i] = '\0';
    fontPrintf(r->x0, r->y0, GP_SelectedColor, gameName);
    fontMakeCurrent(fhSave);
}

/*-----------------------------------------------------------------------------
    Name        : compareGamesCB
    Description : compare's the games alphabetically
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int compareGamesCB(const void *arg1,const void *arg2)
{
    int result;

    gpgame *game1 = (gpgame *)arg1;
    gpgame *game2 = (gpgame *)arg2;

    if (!strcasecmp(game1->title, QuickSaveName))
    {
        return -1;
    }
    else if (!strcasecmp(game2->title, QuickSaveName))
    {
        return 1;
    }

    result = strcasecmp(game1->title,game2->title);
    return result;
}

/*-----------------------------------------------------------------------------
    Name        : gpCountTrainingSavegames
    Description : count the number of training savegames
    Inputs      :
    Outputs     :
    Return      : >= 0
----------------------------------------------------------------------------*/
sdword gpCountTrainingSavegames(void)
{
#ifdef _WIN32
    struct _finddata_t find;
    sdword handle, startHandle;
#else
    DIR* dp;
    struct dirent* dir_entry;
#endif
    char fileSearch[128];
    sdword hits;

    hits = 0;

    strcpy(fileSearch, TutorialSavedGamesPath);
#ifdef _WIN32
    strcat(fileSearch, "*.*");

    startHandle = handle = _findfirst(filePathPrepend(fileSearch, FF_UserSettingsPath), &find);

    while (handle != -1)
    {
        if (find.name[0] != '.')
        {
            hits++;
        }

        handle = _findnext(startHandle, &find);
    }
#else
    dp = opendir(filePathPrepend(fileSearch, FF_UserSettingsPath));
    if (!dp)
        return 0;

    while ((dir_entry = readdir(dp)))
    {
        if (dir_entry->d_name[0] == '.')
            continue;
        hits++;
    }

    closedir(dp);
#endif

    return hits;
}

/*-----------------------------------------------------------------------------
    Name        : gpTitleListLoad
    Description : Scans the games directory and loads in the titles of all
                    the game files available.
    Inputs      : void
    Outputs     : Fills in gpGames and gpNumberGames
    Return      : void
----------------------------------------------------------------------------*/
void gpTitleListLoad(void)
{
#ifdef _WIN32
    struct _finddata_t find;
    sdword handle, startHandle;
#else
    DIR* dp;
    struct dirent* dir_entry;
#endif
    sdword index;
    char fileName[PATH_MAX];
    char fileSearch[100];
//    char *pString;
//    char *title;

    for (index = 0; index < gpNumberGames; index++)
    {
        memFree(gpGames[index].fileSpec);
        memFree(gpGames[index].title);
    }
    if (gpNumberGames > 0)
    {
        memFree(gpGames);
        gpGames = NULL;
        gpNumberGames = 0;
    }

    strcpy(fileSearch,SavedGamesPath);
#ifdef _WIN32
    strcat(fileSearch,"*.*");

    startHandle = handle = _findfirst(filePathPrepend(fileSearch, FF_UserSettingsPath), &find);

    while (handle != -1)
    {
        if (find.name[0] == '.')
        {
            goto alreadyLoaded;
        }
        fileName[0] = 0;

        strcpy(fileName, find.name);

        if (fileName[0] == 0)
        {
            goto alreadyLoaded;
        }

        if (strstr(fileName,PKTS_EXTENSION))
        {
            goto alreadyLoaded;
        }

        for (index = 0; index < gpNumberGames; index++)
        {
            if (!strcasecmp(gpGames[index].fileSpec, fileName))
            {                                               //if matching file specs,
                goto alreadyLoaded;                         //break-continue
            }
        }

        gpGames = memRealloc(gpGames, (gpNumberGames+1) * sizeof(gpgame), "gpGames", NonVolatile);

        gpGames[gpNumberGames].fileSpec = memStringDupe(fileName);
        //gpGames[gpNumberGames].title = title;
        gpGames[gpNumberGames].title = memStringDupe(fileName);

        gpNumberGames++;
alreadyLoaded:;
        handle = _findnext(startHandle, &find);
    }
#else
    dp = opendir(filePathPrepend(fileSearch, FF_UserSettingsPath));

    if (dp)
    {
        while ((dir_entry = readdir(dp)))
        {
            if (dir_entry->d_name[0] == '.')
                continue;
            fileName[0] = 0;

            strcpy(fileName, dir_entry->d_name);

            if (fileName[0] == 0)
                continue;

            if (strstr(fileName,PKTS_EXTENSION))
                continue;

            for (index = 0; index < gpNumberGames; index++)
            {
                if (!strcasecmp(gpGames[index].fileSpec, fileName))
                {                                               /*if matching file specs,*/
                    break;                                      /*break-continue*/
                }
            }
            if (index < gpNumberGames)
                continue;

            gpGames = memRealloc(gpGames, (gpNumberGames+1) * sizeof(gpgame), "gpGames", NonVolatile);

            gpGames[gpNumberGames].fileSpec = memStringDupe(fileName);
            //gpGames[gpNumberGames].title = title;
            gpGames[gpNumberGames].title = memStringDupe(fileName);

            gpNumberGames++;
        }

        closedir(dp);
    }
#endif

    if (gpNumberGames > 1)
    {
        //alphabetically sort the game list
        qsort(&gpGames[0],gpNumberGames,sizeof(gpgame),compareGamesCB);
    }
    gpCurrentSelected = gpCurrentGame = 0;      //set default game
}
/*-----------------------------------------------------------------------------
    Name        : gpTextEntryWindowInit
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpTextEntryWindowInit(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        gpNameEntryBox = (textentryhandle)atom->pData;
//        uicTextEntrySet(gpNameEntryBox,gpTextEntryName,strlen(gpTextEntryName)+1);
        uicTextBufferResize(gpNameEntryBox,GP_GAMENAME_LENGTH-2);
        bitSet(gpNameEntryBox->textflags, UICTE_FileName);
        return;
    }
    else if ((FELASTCALL(atom)))
    {
        gpNameEntryBox = NULL;
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            strcpy(gpTextEntryName,gpNameEntryBox->textBuffer);
        break;
        case CM_GainFocus :
        break;
    }
}

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : gpStartup
    Description :
    Inputs      :
    Outputs     : Registers some callbacks for buttons and user regions.
    Return      :
----------------------------------------------------------------------------*/
void gpStartup(void)
{
    feDrawCallbackAddMultiple(gpDrawCallbacks);
    feCallbackAddMultiple(gpCallbacks);

    gpListFont = frFontRegister(GP_ListFont);
    gpNameFont = frFontRegister(GP_NameFont);
    gpTextEntryName = memAlloc(GP_GAMENAME_LENGTH, "gpTextEntryName", NonVolatile);
    gpTextEntryName[0] = 0;
    gpGames = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : gpReset
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpReset(void)
{
    SavedGamesPath = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : gpShutdown
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpShutdown(void)
{
    sdword index;

    for (index = 0; index < gpNumberGames; index++)
    {
        memFree(gpGames[index].fileSpec);
        memFree(gpGames[index].title);
    }
    if (gpGames!=NULL)
        memFree(gpGames);
    if (gpTextEntryName!=NULL)
        memFree(gpTextEntryName);
//    frFontUnregister(gpListFont);
//    frFontUnregister(gpNameFont);
}

char* gpQuickSetup(void)
{
    if (gameIsRunning)
    {
        if (singlePlayerGame)
        {
            gpLoadSinglePlayerGame = TRUE;
        }
        else
        {
            gpLoadSinglePlayerGame = FALSE;
        }

        gpLoadTutorial = (tutorial==TUTORIAL_ONLY) ? TRUE : FALSE;
    }
    else
    {
        if (mgRunning)
        {
            gpLoadSinglePlayerGame = FALSE;
        }
        else
        {
            gpLoadSinglePlayerGame = TRUE;
        }

        gpLoadTutorial = (tutorial==TUTORIAL_ONLY) ? TRUE : FALSE;
    }

    if (gpLoadSinglePlayerGame)
    {
        return(gpLoadTutorial ? TutorialSavedGamesPath : SinglePlayerSavedGamesPath);
    }
    else
    {
        return(MultiPlayerSavedGamesPath);
    }
}

bool gpQuickSave(void)
{
    char* path;
    char  filename[128];

    path = gpQuickSetup();
    if (path == NULL)
    {
        dbgMessage("\ngpQuickSave couldn't find a dir");
        return FALSE;
    }

    strcpy(filename, path);
    strcat(filename, QuickSaveName);
    gpSaveGivenGame(filename);

    return TRUE;
}

bool gpQuickLoad(void)
{
    char* path;
    char  filename[128];

    path = gpQuickSetup();
    if (path == NULL)
    {
        dbgMessage("\ngpQuickLoad couldn't find a dir");
        return FALSE;
    }

    strcpy(filename, path);
    strcat(filename, QuickSaveName);
    gpLoadGivenGame(filename);

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gpGameWindowInit
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpGameWindowInit(char *name, featom *atom)
{
    fonthandle  oldfont;
    sdword      index;

    if (FEFIRSTCALL(atom))
    {
        if (strcmp(name,"FE_RecordedGameWindowInit") == 0)
        {
            SavedGamesPath = RecordedGamesPath;
        }
        else if (strcmp(name,"FE_TutorialGameWindowInit") == 0)
        {
            SavedGamesPath = TutorialSavedGamesPath;
            tutorial = TUTORIAL_ONLY;
            gpLoadTutorial = TRUE;
            gpLoadSinglePlayerGame = TRUE;
        }
        else
        {
            if (gameIsRunning)
            {
                if (singlePlayerGame)
                {
                    gpLoadSinglePlayerGame = TRUE;
                }
                else
                {
                    gpLoadSinglePlayerGame = FALSE;
                }

                gpLoadTutorial = (tutorial==TUTORIAL_ONLY) ? TRUE : FALSE;
            }
            else
            {
                if (mgRunning)
                {
                    gpLoadSinglePlayerGame = FALSE;
                }
                else
                {
                    gpLoadSinglePlayerGame = TRUE;
                }

                gpLoadTutorial = (tutorial==TUTORIAL_ONLY) ? TRUE : FALSE;
            }

            if (gpLoadSinglePlayerGame)
            {
                SavedGamesPath = gpLoadTutorial ? TutorialSavedGamesPath : SinglePlayerSavedGamesPath;
            }
            else
            {
                SavedGamesPath = MultiPlayerSavedGamesPath;
            }
        }

        gpTitleListLoad();

        oldfont = fontMakeCurrent(gpListFont);

        gpGameListWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(gpGameListWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          gpGameItemDraw,                   // item draw function
                          fontHeight(" ")+GP_VertSpacing,   // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);

        for (index = 0; index < gpNumberGames; index++)
        {
            if (index == 0)
            {
                uicListAddItem(gpGameListWindow, (ubyte *)&gpGames[index], UICLI_CanSelect|UICLI_Selected, UICLW_AddToTail);
                gpCurrentSelected = 0;
            }
            else
            {
                uicListAddItem(gpGameListWindow, (ubyte *)&gpGames[index], UICLI_CanSelect, UICLW_AddToTail);
            }
        }

        fontMakeCurrent(oldfont);
        return;
    } else if (FELASTCALL(atom))
    {
        gpGameListWindow = NULL;
        return;
    }
    else if (gpGameListWindow->message == CM_NewItemSelected)
    {
        if (gpNameEntryBox != NULL)
        {
            uicTextEntrySet(gpNameEntryBox, ((gpgame *)gpGameListWindow->CurLineSelected->data)->title,strlen(((gpgame *)gpGameListWindow->CurLineSelected->data)->title));
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : gpGamePick
    Description : Starts the game picker
    Inputs      : dest - where to store file name of destiation file.
                    Will be "" if nothing (yet) chosen.
    Outputs     : Starts the game picker screen.  When chosen, the name
                    will be stored in dest.
    Return      : void
----------------------------------------------------------------------------*/
void gpGamePick(char *dest)
{
    if (dest[0] == 'S')
    {
        gpLoadSinglePlayerGame = TRUE;
    }
    else
    {
        gpLoadSinglePlayerGame = FALSE;
    }

    SavedGamesPath = (gpLoadSinglePlayerGame) ? SinglePlayerSavedGamesPath : MultiPlayerSavedGamesPath;

    feScreenStart(ghMainRegion, GP_ScreenName);
}

/*-----------------------------------------------------------------------------
    Name        : gpDonePicking/gpBackPicking
    // if the region is the game picker
    Description : Close the color picker and optionally specify the file path
                    of the chosen file.
    Inputs      : standard FE callback
    Outputs     : DonePicking stores chosen file name in parameter to gpGamePick
    Return      : void
----------------------------------------------------------------------------*/
void gpDonePicking(char *name, featom *atom)
{
    sdword index;

    gpCurrentSelected = gpCurrentGame;

    if (gpGameListWindow->CurLineSelected!=NULL)
    {
        for (index = 0; index < gpNumberGames; index++)
        {
            if (gpGameListWindow->CurLineSelected->data == (ubyte *)&gpGames[index])
            {
                gpCurrentSelected = index;
                break;
            }
        }
    }
}

static char overwritefilename[200] = "";
static bool overwriteRecGame = FALSE;

void gpOverwriteYes(char *name, featom *atom)
{
    dbgAssert(overwritefilename[0] != 0);

    if (overwriteRecGame)
    {
        overwriteRecGame = FALSE;

        recPackInGameStartCB(overwritefilename);
        feAllScreensDelete();
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
    }
    else
    {
        if (SaveGame(overwritefilename))
            clCommandMessage(strGetString(strSavedGame));
        else
            clCommandMessage(strGetString(strPatchUnableWriteFile));
        feAllScreensDelete();
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
    }
}

void gpOverwriteNo(char *name, featom *atom)
{
    feScreenDisappear(NULL, NULL);
}

void gpSaveGame(char *name, featom *atom)
{
    char filename[300];

    if ((gpTextEntryName != NULL) && (strlen(gpTextEntryName) > 0))
    {
        if (strlen(gpTextEntryName) < 2)
        {
            GeneralMessageBox(strGetString(strErrorInSaveGameName),strGetString(strAtLeast2Chars));
            return;
        }

        strcpy(filename,SavedGamesPath);
        strcat(filename,gpTextEntryName);

        if (fileExists(filename,0))
        {
            overwriteRecGame = FALSE;
            strcpy(overwritefilename,filename);
            feScreenStart(ghMainRegion, "OverwriteGamePopup");
            return;
        }

        if (!SaveGame(filename))
        {
            GeneralMessageBox(strGetString(strPatchUnableWriteFile),NULL);
            return;
        }

        clCommandMessage(strGetString(strSavedGame));
        feScreenDisappear(NULL, NULL);
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        return;
    }

    if (gpNumberGames > 0)
    {
        gpDonePicking(name, atom);
        dbgAssert(gpCurrentSelected < gpNumberGames);
        strcpy(filename,SavedGamesPath);
        strcat(filename,gpGames[gpCurrentSelected].title);

        if (fileExists(filename,0))
        {
            overwriteRecGame = FALSE;
            strcpy(overwritefilename,filename);
            feScreenStart(ghMainRegion, "OverwriteGamePopup");
            return;
        }

        if (!SaveGame(filename))
        {
            GeneralMessageBox(strGetString(strPatchUnableWriteFile),NULL);
            return;
        }

        clCommandMessage(strGetString(strSavedGame));
        feScreenDisappear(NULL, NULL);
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        return;
    }

    GeneralMessageBox(strGetString(strErrorInSaveGameName),strGetString(strAtLeast2Chars));
}

void gpLoadTheRecordedGame(char *name, featom *atom)
{
    sdword verifysavename;

    if (!gpGetGameName(name,atom,recordPacketSaveFileName))
    {
        return;
    }

    strcpy(recordPacketFileName,recordPacketSaveFileName);
    strcat(recordPacketFileName,PKTS_EXTENSION);

    if ((verifysavename = VerifySaveFile(recordPacketSaveFileName)) != VERIFYSAVEFILE_OK)
    {
        if (verifysavename == VERIFYSAVEFILE_BADVERSION)
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFileVersion),NULL);
        else
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFile),NULL);
        return;
    }

    feScreenDisappear(NULL, NULL);

    recPackPlayInGameInit();

    utyLoadMultiPlayerGameGivenFilename(recordPacketSaveFileName);
}

void gpSaveTheRecordedGame(char *name, featom *atom)
{
    char filename[300];

    if ((gpTextEntryName != NULL) && (strlen(gpTextEntryName) > 0))
    {
        if (strlen(gpTextEntryName) < 2)
        {
            GeneralMessageBox(strGetString(strErrorInSaveGameName),strGetString(strAtLeast2Chars));
            return;
        }

        strcpy(filename,SavedGamesPath);
        strcat(filename,gpTextEntryName);

        if (fileExists(filename,0))
        {
            overwriteRecGame = TRUE;
            strcpy(overwritefilename,filename);
            feScreenStart(ghMainRegion, "OverwriteGamePopup");
            return;
        }

        recPackInGameStartCB(filename);
        feScreenDisappear(NULL, NULL);
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        return;
    }

    if (gpNumberGames > 0)
    {
        gpDonePicking(name, atom);
        dbgAssert(gpCurrentSelected < gpNumberGames);
        strcpy(filename,SavedGamesPath);
        strcat(filename,gpGames[gpCurrentSelected].title);

        if (fileExists(filename,0))
        {
            overwriteRecGame = TRUE;
            strcpy(overwritefilename,filename);
            feScreenStart(ghMainRegion, "OverwriteGamePopup");
            return;
        }

        recPackInGameStartCB(filename);
        feScreenDisappear(NULL, NULL);
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        return;
    }

    GeneralMessageBox(strGetString(strErrorInSaveGameName),strGetString(strAtLeast2Chars));
}

void gpStopPlayback(char *name, featom *atom)
{
    dbgMessage("\nStop playback.");
}

void gpStopRecording(char *name, featom *atom)
{
    recPackInGameStopCB();
    feScreenDisappear(NULL, NULL);
    if (!multiPlayerGame)
    {
        universePause = FALSE;      // unpause game
    }
}

void gpSaveGivenGame(char* gamename)
{
    if (SaveGame(gamename))
        clCommandMessage(strGetString(strQuickSave));
    else
        clCommandMessage(strGetString(strPatchUnableWriteFile));

    if (!multiPlayerGame)
    {
        universePause = FALSE;
    }
}

void gpLoadGivenGame(char* gamename)
{
    sdword verifysavename;

    if (!fileExists(gamename, 0))
    {
        return;
    }

    if ((verifysavename = VerifySaveFile(gamename)) != VERIFYSAVEFILE_OK)
    {
        if (verifysavename == VERIFYSAVEFILE_BADVERSION)
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFileVersion),NULL);
        else
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFile),NULL);
        return;
    }

    gameEnd();

    if (gpLoadSinglePlayerGame)
    {
        if (gpLoadTutorial)
        {
            tutorial = 1;
        }
        else
        {
            tutorial = 2;
        }
        utyLoadSinglePlayerGameGivenFilename(gamename);
    }
    else
    {
        utyLoadMultiPlayerGameGivenFilename(gamename);
    }
    if (!multiPlayerGame)
    {
        universePause = FALSE;
    }
}

void spRestartLevel(char *name, featom *atom)
{
    if (strlen(CurrentLevelName) > 0)
    {
        gpLoadSinglePlayerGame = TRUE;
        gpLoadTutorial = FALSE;
        gpLoadGivenGame(CurrentLevelName);
    }
}

void gpLoadGame(char *name, featom *atom)
{
    char filename[100];
    sdword verifysavename;

    if (gpNumberGames > 0)
    {
        gpDonePicking(name, atom);

        dbgAssert(gpCurrentSelected < gpNumberGames);
        if (gpLoadTutorial)
        {
            strcpy(filename,TutorialSavedGamesPath);
        }
        else
        {
            strcpy(filename,SavedGamesPath);
        }
        strcat(filename,gpGames[gpCurrentSelected].title);

        if ((verifysavename = VerifySaveFile(filename)) != VERIFYSAVEFILE_OK)
        {
            if (verifysavename == VERIFYSAVEFILE_BADVERSION)
                GeneralMessageBox(strGetString(strErrorInvalidSaveGameFileVersion),NULL);
            else
                GeneralMessageBox(strGetString(strErrorInvalidSaveGameFile),NULL);
            return;
        }

        feScreenDisappear(NULL, NULL);

        soundEventStopTrack(SOUND_EVENT_DEFAULT, 1.0f);
        soundEventPause(TRUE);
        gameEnd();
        if (gpLoadSinglePlayerGame)
        {
            if (gpLoadTutorial)
            {
                tutorial = 1;
            }
            else
            {
                //for single player games, tutorial == 2 to use certain tutorial functions
                tutorial = 2;
            }
            utyLoadSinglePlayerGameGivenFilename(filename);
        }
        else
        {
            utyLoadMultiPlayerGameGivenFilename(filename);
        }
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        soundEventPause(FALSE);
    }
}


void gpDeleteGame(char *name, featom *atom)
{
    char filename[100];
    sdword i;
    sdword index;
    fonthandle  oldfont;

    if (!gpGetGameName(name,atom,filename))
    {
        return;
    }

    // gpCurrentSelected will be set by gpGetGameName

    feScreenDisappear(NULL, NULL);

    fileDelete(filename);
    if (SavedGamesPath == RecordedGamesPath)
    {
        char tmpfile[100];
        strcpy(tmpfile,filename);
        strcat(tmpfile,PKTS_EXTENSION);
        fileDelete(tmpfile);
    }

    for (i=gpCurrentSelected;i<gpNumberGames-1;i++)
    {
        gpGames[i] = gpGames[i+1];
    }
    gpNumberGames--;
    if (gpNumberGames == 0)
    {
        gpCurrentSelected = 0;
    }
    else if (gpCurrentSelected >= gpNumberGames)
    {
        gpCurrentSelected--;
        dbgAssert(gpCurrentSelected >= 0);
        dbgAssert(gpCurrentSelected < gpNumberGames);
    }

    dbgAssert(gpNumberGames >= 0);

    oldfont = fontMakeCurrent(gpListFont);      // fontHeight called later on in this function

    uicListCleanUp(gpGameListWindow);

    uicListWindowInit(gpGameListWindow,
                      NULL,                             // title draw, no title
                      NULL,                             // title click process, no title
                      0,                                // title height, no title
                      gpGameItemDraw,                   // item draw function
                      fontHeight(" ")+GP_VertSpacing,   // item height
                      UICLW_CanSelect);

    for (index = 0; index < gpNumberGames; index++)
    {
        if (index==gpCurrentSelected)
            uicListAddItem(gpGameListWindow, (ubyte *)&gpGames[index], UICLI_CanSelect|UICLI_Selected, UICLW_AddToTail);
        else
            uicListAddItem(gpGameListWindow, (ubyte *)&gpGames[index], UICLI_CanSelect, UICLW_AddToTail);
    }

    fontMakeCurrent(oldfont);
}

bool gpGetGameName(char *name, featom *atom, char *filename)
{
    if (gpNumberGames > 0)
    {
        gpDonePicking(name, atom);
        dbgAssert(gpCurrentSelected < gpNumberGames);
        strcpy(filename,SavedGamesPath);
        strcat(filename,gpGames[gpCurrentSelected].title);
        return TRUE;
    }
    return FALSE;
}

void gpBackPicking(char *name, featom *atom)
{
    feScreenDisappear(NULL, NULL);
}

#if 0
void gpAcceptBackToInGameEscapeMenu(char* name, featom* atom)
{
    void opInGameOptionsAccept(char* name, featom* atom);

    opInGameOptionsAccept(name, atom);

    feScreenStart(ghMainRegion, "In_game_esc_menu");
}
#endif

void gpBackToInGameEscapeMenu(char *name, featom *atom)
{
    feScreenDisappear(NULL, NULL);

#if defined(Downloadable)
    if ((singlePlayerGame) && (!(utyCreditsSequence|utyPlugScreens)) && (universe.quittime > 0.0f) && (universe.totaltimeelapsed >= universe.quittime))
#else
    if ((singlePlayerGame) && (!(utyCreditsSequence)) && (universe.quittime > 0.0f) && (universe.totaltimeelapsed >= universe.quittime))
#endif
    {
        feScreenStart(ghMainRegion, "SP_Game_Over");
        return;
    }

    switch (escapemenu)
    {
        case ESCAPE_DEFAULT:
            feScreenStart(ghMainRegion, "In_game_esc_menu");
            break;

        case ESCAPE_SINGLE:
            feScreenStart(ghMainRegion, "In_game_esc_menu2");
            break;

        case ESCAPE_RECORDING:
            feScreenStart(ghMainRegion, "In_game_esc_menu_recording");
            break;

        case ESCAPE_PLAYING:
            feScreenStart(ghMainRegion, "In_game_esc_menu_playing");
            break;

        case ESCAPE_TUTORIAL:
            feScreenStart(ghMainRegion, "In_game_esc_menu_tutorial");
            break;

        case ESCAPE_DEFAULT_NOSAVE:
            feScreenStart(ghMainRegion, "In_game_esc_menu_nosave");
            break;

        case ESCAPE_RECORDING_NOSAVE:
            feScreenStart(ghMainRegion, "In_game_esc_menu_nosave_recording");
            break;
    }
}

void gpStartInGameEscapeMenu(void)
{
    if (tutorial==TUTORIAL_ONLY)
    {
        escapemenu = ESCAPE_TUTORIAL;
    }
    else if (singlePlayerGame)
    {
        escapemenu = ESCAPE_SINGLE;
    }
    else if (playPackets)
    {
        escapemenu = ESCAPE_PLAYING;
    }
    else if (recordPackets)
    {
        if (debugPacketRecord)
        {
            if (multiPlayerGame)
                escapemenu = ESCAPE_DEFAULT_NOSAVE;
            else
                escapemenu = ESCAPE_DEFAULT;
        }
        else
        {
            if (multiPlayerGame)
                escapemenu = ESCAPE_RECORDING_NOSAVE;
            else
                escapemenu = ESCAPE_RECORDING;
        }
    }
    else
    {
        if (multiPlayerGame)
            escapemenu = ESCAPE_DEFAULT_NOSAVE;
        else
            escapemenu = ESCAPE_DEFAULT;
    }

    mrReset();
    gpBackToInGameEscapeMenu(NULL, NULL);
}




