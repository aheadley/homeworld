/*=============================================================================
    Name    : GamePick.c
    Purpose : Code for choosing a Game

    Created 10/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___GAMEPICK_H
#define ___GAMEPICK_H           3.4444445f

#include "Types.h"
#include "FEFlow.h"

/*=============================================================================
    Switches:
=============================================================================*/

/*=============================================================================
    Definitions:
=============================================================================*/
#define GP_ScreenName           "Load_game"
#define GP_ListFont             "Arial_12.hff"
#define GP_NameFont             "Arial_12.hff"
#define GP_NonSelectedColor     colRGB(0, 75, 120)
#define GP_SelectedTabbedColor  colRGB(1, 170, 221)
#define GP_SelectedColor        GP_SelectedTabbedColor
#define GP_ListMarginInterLine  1
#define GP_ListMarginX          2
#define GP_ListMarginY          2
#define GP_DisplayListLength    11      //number of games displayed on the screen at one time
#define GP_VertSpacing          (fontHeight(" ") >> 1)
#define GP_GAMENAME_LENGTH      32

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
    char    *title;
    char    *fileSpec;
}
gpgame;

/*=============================================================================
    Data:
=============================================================================*/
extern gpgame *gpGames;             //list of available games
extern sdword gpCurrentSelected;    //current game index, if OK is pressed
extern char   *gpTextEntryName;     //text box entry name

extern bool gpLoadSinglePlayerGame;
extern bool gpLoadTutorial;

extern char TutorialSavedGamesPath[];

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown
void gpStartup(void);
void gpShutdown(void);
void gpReset(void);

//run the game picker
void gpGamePick(char *dest);

//close the picker, either with 'back' or with 'OK'
void gpDonePicking(char *name, featom *atom);
void gpBackPicking(char *name, featom *atom);

bool gpQuickSave(void);
bool gpQuickLoad(void);

bool gpGetGameName(char *name, featom *atom, char *filename);

sdword gpGameListProcess(regionhandle region, sdword ID, udword event, udword data);

sdword gpCountTrainingSavegames(void);

#endif //___GAMEPICK_H
