/*=============================================================================
    UTILITY.C: General utilities for Windows interface of Homeworld

    Created June 1997 by Luke Moloney
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winreg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shellapi.h>
#include "glinc.h"
#include "liloptions.h"
#include "autodownloadmap.h"
#include "regkey.h"

extern char mainDeviceToSelect[];
extern char mainGLToSelect[];
extern char mainD3DToSelect[];

#define REG_UDWORD 0
#define REG_STRING 1

// #define HW_NOTOVERSIZE   1                   // comment to enable oversize-CD code
#define REG_MAGIC_STR "D657E436967616D4"    // used for oversize-CD code

udword regMagicNum = 0; // used right below
char regLanguageVersion[50]; // used right below

typedef struct registryOption
{
    char*  name;
    DWORD  type;
    void*  data;
} registryOption;

extern sdword MAIN_WindowWidth, MAIN_WindowHeight, MAIN_WindowDepth;
extern udword gDevcaps, gDevcaps2;
udword loadedDevcaps  = 0xFFFFFFFF;
udword loadedDevcaps2 = 0xFFFFFFFF;

registryOption regOptionsList[] =
{
    {"deviceCRC",       REG_UDWORD, &opDeviceCRC},
    {"deviceCaps",      REG_UDWORD, &loadedDevcaps},
    {"deviceCaps2",     REG_UDWORD, &loadedDevcaps2},
    {"deviceIndex",     REG_UDWORD, &opDeviceIndex},
    {"deviceToSelect",  REG_STRING, &mainDeviceToSelect},
    {"glToSelect",      REG_STRING, &mainGLToSelect},
    {"d3dToSelect",     REG_STRING, &mainD3DToSelect},
    {"screenWidth",     REG_UDWORD, &MAIN_WindowWidth},
    {"screenHeight",    REG_UDWORD, &MAIN_WindowHeight},
    {"screenDepth",     REG_UDWORD, &MAIN_WindowDepth},
    {"HW_Language",     REG_STRING, &regLanguageVersion},
    {REG_MAGIC_STR,     REG_UDWORD, &regMagicNum},  // used for oversize-CD support (UK version only)
    {NULL, 0, NULL}
};

int utyEnsureRegistry(void)
{
    HKEY key;
    DWORD disp;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, BASEKEYNAME, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
    {
        //already there
        return 1;
    }

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, BASEKEYNAME, 0,
                       "", REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, NULL,
                       &key, &disp) == ERROR_SUCCESS)
    {
        //successfully created
        RegCloseKey(key);
        return 1;
    }

    //couldn't create
    return 0;
}

void utyRegistryOptionsRead(void)
{
    registryOption* reg;
    HKEY key;
    DWORD type, length;
    BYTE data[128];
    DWORD* pdword;

    if (!utyEnsureRegistry())
    {
        return;
    }

    reg = regOptionsList;
    while (reg->name != NULL)
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, BASEKEYNAME,
                         0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
        {
            reg++;
            continue;
        }

        length = 128;
        if (RegQueryValueEx(key, reg->name, 0, &type, data, &length) == ERROR_SUCCESS)
        {
            switch (type)
            {
            case REG_DWORD_LITTLE_ENDIAN:
                pdword = (DWORD*)reg->data;
                *pdword = *((DWORD*)data);
                break;

            case REG_SZ:
                memcpy((BYTE*)reg->data, data, length);
                break;
            }
        }

        RegCloseKey(key);

        reg++;
    }
}

void utyRegistryOptionsWrite(void)
{
    registryOption* reg;
    HKEY key;

    if (!utyEnsureRegistry())
    {
        return;
    }

    loadedDevcaps  = gDevcaps;
    loadedDevcaps2 = gDevcaps2;

    reg = regOptionsList;
    while (reg->name != NULL)
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, BASEKEYNAME,
                         0, KEY_SET_VALUE, &key) != ERROR_SUCCESS)
        {
            reg++;
            continue;
        }

        switch (reg->type)
        {
        case REG_UDWORD:
            (void)RegSetValueEx(key, reg->name, 0, REG_DWORD_LITTLE_ENDIAN,
                                reg->data, sizeof(udword));
            break;

        case REG_SZ:
            (void)RegSetValueEx(key, reg->name, 0, REG_SZ,
                                reg->data, strlen((char*)reg->data)+1);
            break;
        }

        RegCloseKey(key);

        reg++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : utyBrowserExec
    Description : Spawns the default web browser with the specified URL
    Inputs      :
    Outputs     :
    Return      : TRUE if the browser was started OK, FALSE otherwise
----------------------------------------------------------------------------*/
bool utyBrowserExec(char *URL)
{
    HKEY key;
    char shellCommand[BIT8];
    //char options[BIT8] = "";
    char *optionsString;
    char *shellCommandString;
    udword shellCommandLength = BIT8;
    udword type;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\http\\shell\\open\\command",
            0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
    {
        return(FALSE);
    }

    if (RegQueryValueEx(key, NULL, 0, &type, (LPBYTE)shellCommand, &shellCommandLength) != ERROR_SUCCESS)
    {
       RegCloseKey(key);
        return(FALSE);
    }

    if ((optionsString = strstr(shellCommand, " /")) != NULL ||
       (optionsString = strstr(shellCommand, " -")) != NULL)
    {                                               //if there are command-line options
       //strcpy(options, optionsString + 1);
       //strcat(options, " ");
       *optionsString = 0;                         //actually, we don't want any of these options
    }
    /*
    else
    {
       options[0] = 0;
    }
    */
    //      strcpy(options, URL);
    shellCommandString = shellCommand;
    while (strlen(shellCommandString) && shellCommandString[0] == '"')
    {                                               //strip leading quotes
       shellCommandString++;
    }
    while (strlen(shellCommandString) && shellCommandString[strlen(shellCommandString) - 1] == '"')
    {                                               //strip trailing quotes
       shellCommandString[strlen(shellCommandString) - 1] = 0;
    }
    ShellExecute(NULL, "open", shellCommandString, URL, NULL, SW_SHOW);
    /*
    {
       char *operaString = " d:\\Internet\\Opera\\Opera.exe ";
       ShellExecute(NULL, "open", operaString, options, NULL, SW_SHOW);
    }
    */

    RegCloseKey(key);
    return(TRUE);
}

#include "types.h"
#include "resource.h"
#include "key.h"
#include "debugwnd.h"
#include "debug.h"
#include "memory.h"
#pragma warning( 4 : 4142 )     //turn off "benign redefinition of type" warning
#include "main.h"
#pragma warning( 2 : 4142 )
#include "universe.h"
#include "task.h"
#include "render.h"
#include "file.h"
#include "region.h"
#include "mouse.h"
#include "mainrgn.h"
#include "select.h"
#include "prim2d.h"
#include "uicontrols.h"
#include "FeFlow.h"
#include "Font.h"
#include "FontReg.h"
#include "statscript.h"
#include "stats.h"
#include "ConsMgr.h"
#include "soundevent.h"
#include "globals.h"
#include "commandnetwork.h"
#include "netcheck.h"
#include "undo.h"
#include "tactical.h"
#include "trails.h"
#include "light.h"
#include "texreg.h"
#include "TaskBar.h"
#include "sensors.h"
#include "levelload.h"
#include "colpick.h"
#include "teams.h"
#include "ScenPick.h"
#include "randy.h"
#include "etg.h"
#include "NIS.h"
#include "spaceobj.h"
#include "utility.h"
#include "univupdate.h"
#include "clouds.h"
#include "fastmath.h"
#include "singleplayer.h"
#include "FeReg.h"
#include "nebulae.h"
#include "aiplayer.h"
#include "TitanInterfaceC.h"
#include "btg.h"
#include "gun.h"
#include "strings.h"
#include "prim3d.h"
#include "launchMgr.h"
#include "demo.h"
#include "pieplate.h"
#include "researchapi.h"
#include "tactics.h"
#include "InfoOverlay.h"
#include "horserace.h"
#include "MultiplayerGame.h"
#include "MultiplayerLANGame.h"
#include "titannet.h"
#include "glcaps.h"
#include "gamepick.h"
#include "gamechat.h"
#include "shipview.h"
#include "damage.h"
#include "SaveGame.h"
#include "GamePick.h"
#include "BigFile.h"
#include "tutor.h"
#include "autolod.h"
#include "ping.h"
#include "battle.h"
#include "shader.h"
#include "transformer.h"
#include "Captaincy.h"
#include "options.h"
#include "linklimits.h"
#include "particle.h"
#include "trademgr.h"
#include "bink.h"
#include "bounties.h"
#include "subtitle.h"
#include "animatic.h"
#include "dxdraw.h"
#include "sstglide.h"
#include "PlugScreen.h"
#include "hs.h"
#include "glcompat.h"
#include "KeyBindings.h"
#include "ResearchGui.h"

//
//  for bigfiles
//
extern bool CompareBigfiles;
extern bool IgnoreBigfiles;
extern bigTOC mainTOC;
extern bigTOC updateTOC;
extern FILE *updateFP;     // for NULL checking
extern unsigned char *mainNewerAvailable;
extern unsigned char *updateNewerAvailable;

/*=============================================================================
    Data:
=============================================================================*/

bool utilPlayingIntro = FALSE;

unsigned long HomeworldCRC[4] = { 0,0,0,0 };    // dont use udword, this goes in TitanInterfaceC.h

static char* dataEnvironment = NULL;

void *utyMemoryHeap;
char errorString[UTY_ErrorStringLength];

// used to skip the tutorial warning after you have played a non tutorial mission
bool needtutorial=TRUE;

//data for the timing of tasks
LARGE_INTEGER utyTimerFrequency;
LONGLONG utyTimerDivisor;
LONGLONG utyTimerLast;

//flag stating system has started properly
sdword utySystemStarted = FALSE;

//bit-field flags stating which system has started properly
bool8 utyStartedBits[SSA_NumberBits];

udword utyNFrameTicks = 0;

//global handle of default font
fonthandle ghDefaultFont = 0;

// name of bigfile
#if defined(CGW)
char utyBigFilename[] = "HomeworldCGW.big";
#elif defined(Downloadable) || defined(DLPublicBeta)
char utyBigFilename[] = "HomeworldDL.big";
//#elif defined(OEM)
//char utyBigFilename[] = "HomeworldMOE.big";
#else
char utyBigFilename[] = "Homeworld.big";
#endif

// name of music data file
#if defined(CGW)
char utyMusicFilename[] = "CGW_Music.wxd";
#elif defined(Downloadable)
char utyMusicFilename[] = "DL_Music.wxd";
#elif defined(DLPublicBeta)
char utyMusicFilename[] = "PB_Music.wxd";
#elif defined(OEM)
char utyMusicFilename[] = "OEM_Music.wxd";
#else
char utyMusicFilename[] = "HW_Music.wxd";
#endif

// name of voice file
#if  defined(CGW)
char utyVoiceFilename[] = "CGW_Demo.vce";
#elif defined(Downloadable) || defined(DLPublicBeta)
char utyVoiceFilename[] = "DL_Demo.vce";
#else
char utyVoiceFilename[] = "HW_comp.vce";
#endif

// name of other files
char utyUpdateBigFilename[] = "Update.big"; // name for future updates to main bigfile
char utyPadBigFilename[] = "Extra.big"; // name of pad file
char utyLockFilename[] = "AutoLock.txt"; // name of autorun lock file

filehandle utyLockFilehandle; // handle of autorun lock file

// global string for the player name
char utyName[MAX_PERSONAL_NAME_LEN]={0,0};
char utyPassword[MAX_PERSONAL_NAME_LEN]="";

char levelfile[80];
char dirfile[100];

//basic global front end data
char *utyFrontEndFiles[] =
{                                                           //basic front-end screens
    "FEMan\\Front_end.fib",
    "FEMan\\Sensors_manager.fib",
//    "FEMan\\Sensors_manager_SingleP.fib",
    "FEMan\\Research_manager.fib",
    "FEMan\\In_game_ESC_menu.fib",
    "FEMan\\CSM-All.fib",
    "FEMan\\TaskBar.fib",
    "FEMan\\single_player_objective.fib",
    //"FEMan\\Single_player_objective.fib",
    NULL
};

void scriptSetHomeworldCRC(char *directory,char *field,void *dataToFillIn);

bool onlygetfirstcrc = FALSE;

scriptEntry WonStuffSet[] =
{
    makeEntry(HomeworldCRC,scriptSetHomeworldCRC),
    endEntry
};

//render task
taskhandle utyRenderTask;

//TRUE if tutorial savegame check is not to be performed
static bool forceSP = FALSE;

//global flag for demo functionality
#if defined(CGW) || defined (Downloadable) || defined(DLPublicBeta) || defined(OEM)
bool utyPlugScreens = FALSE;
#endif
bool utyCreditsSequence = FALSE;

//front-end callback functions
void utySinglePlayerOptions(char *name, featom *atom);
void utySinglePlayerGame(char *name, featom *atom);
void utySelectSinglePlayerGame(char *name, featom *atom);
void utyLoadMultiSinglePlayerGame(char *name, featom *atom);
void utyLoadSinglePlayerGame(char *name, featom *atom);
void utySelectMultiPlayerGame(char *name, featom *atom);
void utyLoadMultiPlayerGame(char *name, featom *atom);
void utySinglePlayerGameStart(char *name, featom *atom);
void utyBackFromSinglePlayer(char *name, featom *atom);
void utyBackFromSinglePlayerOptions(char *name, featom *atom);
void utyGameQuit(char *name, featom *atom);
void utyMultiplayerGameStart(char *name, featom *atom);
void utyShowCredits(char *name, featom *atom);
void utyStartInternetGame(char *name, featom *atom);
void utyBackgroundToggle(char *name, featom *atom);
void utyBackgroundBitmapToggle(char *name, featom *atom);
void utyFilterToggle(char *name, featom *atom);
void utyMagFilterToggle(char *name, featom *atom);
void utyFilterBitmapToggle(char *name, featom *atom);
void utySmoothingToggle(char *name, featom *atom);
void utySmoothingBitmapToggle(char *name, featom *atom);
void utyStippleToggle(char* name, featom* atom);
void utyStippleBitmapToggle(char* name, featom* atom);
void utyTrailsToggle(char* name, featom* atom);
void utyTrailsBitmapToggle(char* name, featom* atom);
void utyLanguageToggle(char* name, featom* atom);
void utySensorsBlobsToggle(char* name, featom* atom);
void utyInstantTransitionToggle(char *name, featom *atom);
void utyInstantTransitionBitmapToggle(char* name, featom* atom);
void utySensorsBlobsBitmapToggle(char* name, featom* atom);
void utyBulletBitmapToggle(char* name, featom* atom);
void utyBulletToggle(char* name, featom* atom);
void utyMuzzleBitmapToggle(char* name, featom* atom);
void utyMuzzleToggle(char* name, featom* atom);
void utyDamageBitmapToggle(char* name, featom* atom);
void utyDamageToggle(char* name, featom* atom);
void utyHitBitmapToggle(char* name, featom* atom);
void utyHitToggle(char* name, featom* atom);

void utyPickColors(char *name, featom *atom);
//void utyScenarioPick(char *name, featom *atom);
void utyHideMenuOrGotoQuitGame(char *name, featom *atom);
//void utyNewGameRace(char *name, featom *atom);
//void utyTeaserEnd(void);

void utyInGameCancel(char *name, featom *atom);
void utyInGameLoad(char *name, featom *atom);
void utyInGameSave(char *name, featom *atom);

void utySaveRecordedGame(char *name, featom *atom);
//from options.c

void opKeyCancel(char* name, featom* atom);

void opDrawNumChannels(featom *atom, regionhandle region);

void opOptionsAccept(char* name, featom* atom);
void opOptionsInit(void);
void opOptionsStart(char* name, featom* atom);
void opOptionsCancel(char* name, featom* atom);

void opInGameOptionsStart(char* name, featom* atom);
void opInGameOptionsCancel(char* name, featom* atom);
void opInGameOptionsAccept(char* name, featom* atom);

void opEqualizerToAudioAccept(char* name, featom* atom);
void opEqualizerToAudioCancel(char* name, featom* atom);
void opEqualizerToInGameAudioAccept(char* name, featom* atom);
void opEqualizerToInGameAudioCancel(char* name, featom* atom);
void opAdvancedToAudioAccept(char* name, featom* atom);
void opAdvancedToAudioCancel(char* name, featom* atom);
void opAdvancedToInGameAudioCancel(char* name, featom* atom);
void opAdvancedToInGameAudioAccept(char* name, featom* atom);

void opEffectsToVideoAccept(char* name, featom* atom);
void opEffectsToVideoCancel(char* name, featom* atom);
void opEffectsToInGameVideoAccept(char* name, featom* atom);
void opEffectsToInGameVideoCancel(char* name, featom* atom);

void opMusicVolume(char* name, featom* atom);
void opSFXVolume(char* name, featom* atom);
void opSpeechVolume(char* name, featom* atom);
void opNumberChannels(char* name, featom* atom);
void opAutoChannels(char* name, featom* atom);
void opSpeaker(char* name, featom* atom);
void opSoundQualitySet(char* name, featom* atom);
void opAdvancedButton(char* name, featom* atom);
void opEqualizerButton(char* name, featom* atom);
void opEqualizer1(char* name, featom* atom);
void opEqualizer2(char* name, featom* atom);
void opEqualizer3(char* name, featom* atom);
void opEqualizer4(char* name, featom* atom);
void opEqualizer5(char* name, featom* atom);
void opEqualizer6(char* name, featom* atom);
void opEqualizer7(char* name, featom* atom);
void opEqualizer8(char* name, featom* atom);
//advanced:
void opVoice(char* name, featom* atom);
void opVoice0(char* name, featom* atom);
void opVoice1(char* name, featom* atom);
void opVoice2(char* name, featom* atom);
void opHearVoice1(char* name, featom* atom);
void opHearVoice2(char* name, featom* atom);
void opHearVoice3(char* name, featom* atom);
void opVoiceCommands(char* name, featom* atom);
void opVoiceStatus(char* name, featom* atom);
void opVoiceChatter(char* name, featom* atom);
//game
void opMouseSensitivity(char* name, featom* atom);
void opBattleChatterCB(char* name, featom* atom);
void opNumberEffects(char* name, featom* atom);
void opCPUDifficulty(char* name, featom* atom);
void opCPUAttacks(char* name, featom* atom);
void opInfoOverlay(char* name, featom* atom);
void opTaskbarUp(char* name, featom* atom);
void opIncreaseChatting(char* name, featom* atom);
void opDecreaseChatting(char* name, featom* atom);
void opNumberChatlines(char* name, featom* atom);
void opAutodockHealth(char* name, featom* atom);
void opAutodockFuel(char* name, featom* atom);
void opAutodockHealthValue(char* name, featom* atom);
void opAutodockFuelValue(char* name, featom* atom);

//video
void opNoLOD(char* name, featom* atom);
void opDetailThreshold(char* name, featom* atom);
void opBrightness(char* name, featom* atom);
void opLighting(char* name, featom* atom);
void opEffects(char* name, featom* atom);
void opCustomEffects(char* name, featom* atom);
void opResolution(char* name, featom* atom);
void opRender(char* name, featom* atom);

void versionNumDraw(featom *atom, regionhandle region);

void utyVideoPlay(char* name, featom* atom);

void utyTutorialAlertCancel(char* name, featom* atom);

void tbSensorsBegin(char* name, featom* atom);

void opNoPalDecrease(featom* atom, regionhandle region);
void opNoPalIncrease(featom* atom, regionhandle region);
void opNoPalDraw(featom* atom, regionhandle region);

void utyStartTutorial(char* name, featom* atom);
void utyStartSinglePlayer(char* name, featom* atom);
void utyStartSkirmish(char* name, featom* atom);

void opTroubleShoot(char* name, featom* atom);

void spRestartLevel(char *name, featom *atom);

// these callbacks are for the drop player dialog
void utyYesDropPlayer(char *name, featom *atom);
void utyDontDropPlayer(char *name, featom *atom);
void utyDrawDroppedPlayer(featom *atom, regionhandle region);

void opCountdownBoxDraw(featom* atom, regionhandle region);
void opCountdownYes(char* name, featom* atom);
void opCountdownNo(char* name, featom* atom);

sdword utyPlayerDroppedDisplay=-1;

// R1 or R2, modified in utyNewGameRace callback
ShipRace whichRaceSelected = R1;

fecallback utyCallbacks[] =
{                                                           //base front-end callbacks
    {opTroubleShoot,            "OP_TroubleShoot"},
    {utyStartTutorial,          "TA_StartTutorial"},
    {utyStartSinglePlayer,      "TA_StartSinglePlayer"},
    {utyStartSkirmish,          "TA_StartSkirmish"},
    {utyVideoPlay,              "VideoPlay"},
    {utyTutorialAlertCancel,    "TutorialAlertCancel"},
    {utyTutorialAlertCancel,    "TutorialSkirmishAlertCancel"},
    {spRestartLevel,            "SP_Restart_Level"},
    {utySinglePlayerOptions,    "SinglePlayerOptions"},     //load saved game or start new game
    {utySinglePlayerGame,       "SinglePlayerGame"},        //start new game
    {utySelectSinglePlayerGame, "SelectSinglePlayerGame"},  //select from saved games
    {utyLoadMultiSinglePlayerGame,   "LoadMultiSinglePlayerGameFromMain"},    //load a saved game & start
    {utySelectMultiPlayerGame,  "SelectMultiPlayerGame"},  //select from saved games
    {utySinglePlayerGameStart,  "StartSinglePlayerGame"},
    {utyBackFromSinglePlayer,   "BackFromSinglePlayer"},
    {utyBackFromSinglePlayerOptions, "BackFromSinglePlayerOptions"},
    {utyNewGameStart,           "StartNewGame"},
    {tutTutorial1,              "Tutorial1"},
    {utyMultiplayerGameStart,   "FE_Multiplayer_Game"},
    {utyShowCredits,            "FE_SHOWCREDITS"},
    {utyGameQuit,               "QuitGame"},
    {utyGameQuitToMain,         "QuitToMain"},
    {utyInGameCancel,           "IG_Cancel"},
//    {utyInGameLoad,             "IG_LoadGame"},
//    {utyInGameSave,             "IG_SaveGame"},
//    {utySaveRecordedGame,       "IG_SaveRecordedGame"},
    {mrDockingOrders,           "CSM_Dock"},                //menu commands
    {mrDeltaFormation,          "CSM_DeltaFormation"},
    {mrBroadFormation,          "CSM_BroadFormation"},
    {mrXFormation,              "CSM_XFormation"},
    {mrClawFormation,           "CSM_ClawFormation"},
    {mrWallFormation,           "CSM_WallFormation"},
    {mrSphereFormation,         "CSM_SphereFormation"},
    {mrPicketFormation,         "CSM_PicketFormation"},
    {mrHarvestResources,        "CSM_Harvest"},
    {mrBuildShips,              "CSM_Build"},
    {mrTradeStuff,              "CSM_Trade"},
    {mrMoveShips,               "CSM_Move"},
    {mrInfo,                    "CSM_Info"},
    {mrCancel,                  "CSM_Cancel"},
    {mrScuttle,                 "CSM_Scuttle"},
    {mrRetire,                  "CSM_Retire"},
    {mrHyperspace,              "CSM_Hyperspace"},
    {mrLaunch,                  "CSM_Launch"},
    {mrResearch,                "CSM_Research"},
    {mrEvasiveTactics,          "CSM_Evasive"},
    {mrNeutralTactics,          "CSM_Neutral"},
    {mrAgressiveTactics,        "CSM_Agressive"},
    {mrFormAlliance,            "PLM_FormAlliance"},
    {mrBreakAlliance,           "PLM_BreakAlliance"},
    {mrTransferRUS,             "PLM_TransferRUS"},
    {utyFilterToggle,           "VO_Filter"},               //options screen controls
    {utyMagFilterToggle,        "VO_MagFilter"},
    {utyFilterBitmapToggle,     "VO_BitmapFilter"},
    {utyBackgroundToggle,       "VO_Background"},
    {utyBackgroundBitmapToggle, "VO_BitmapBackground"},
    {utySmoothingToggle,        "VO_Smoothing"},
    {utySmoothingBitmapToggle,  "VO_BitmapSmoothing"},
    {utyStippleToggle,          "VO_Stipple"},
    {utyStippleBitmapToggle,    "VO_BitmapStipple"},
    {utyTrailsToggle,           "VO_Trails"},
    {utyTrailsBitmapToggle,     "VO_BitmapTrails"},

    {utyHitToggle,           "VO_Hit"},
    {utyHitBitmapToggle,     "VO_BitmapHitEffects"},

    {utyDamageToggle,           "VO_Damage"},
    {utyDamageBitmapToggle,     "VO_BitmapDamage"},

    {utyMuzzleToggle,           "VO_Muzzle"},
    {utyMuzzleBitmapToggle,     "VO_BitmapMuzzle"},

    {utyBulletToggle,           "VO_Bullet"},
    {utyBulletBitmapToggle,     "VO_BitmapBullet"},

    {utyLanguageToggle,         "VO_Language"},
    {utySensorsBlobsToggle,     "VO_SensorsBlobs"},
    {utySensorsBlobsBitmapToggle,"VO_BitmapSensorsBlobs"},
    {smSensorsBegin,            "SensorsManager"},
    {tbSensorsBegin,            "TB_SensorsManager"},
    {utyPickColors,             "NewGameSelectColour"},
    {utyHideMenuOrGotoQuitGame, "FE_HideMenuOrGotoQuitGame"},
    {utyInstantTransitionToggle,"VO_InstantTransition"},
    {utyInstantTransitionBitmapToggle, "VO_BitmapInstantTransition"},
/*    {utyPickColors,             "NewMultiGameSelectColour"},
    {utyPickColors,             "JoinMultiGameSelectColour"},
    {utyScenarioPick,           "CS_ChooseScenario"},
    {spDonePicking,             "CS_Done"},
    {spBackPicking,             "CS_Back"},
    {spScroller,                "CS_Scroller"},             //scenario picker scrollbar
    {tpStartGame,               "T_StartGame"},
    {tpBackPicking,             "T_Back"},
    {tpScroller,                "T_Scroller"},              //titan scrollbar
    {tpCreateGame,              "T_CreateGame"},
    {tpJoinGame,                "T_JoinGame"},
    {tpLeaveGame,               "T_LeaveGame"},
    {tpRefreshGame,             "T_RefreshGame"},
    {tpTextEntry,               "T_TextEntry"},*/
    {opOptionsStart,            "OP_Options_Start"},
    {opOptionsCancel,           "OP_Options_Cancel"},
    {opOptionsAccept,           "OP_Options_Accept"},
    {opInGameOptionsStart,      "OP_InGameOptions_Start"},
    {opInGameOptionsCancel,     "OP_InGameOptions_Cancel"},
    {opInGameOptionsAccept,     "OP_InGameOptions_Accept"},
    {opEqualizerToAudioCancel,  "OP_Equalizer_To_Audio_Cancel"},
    {opEqualizerToAudioAccept,  "OP_Equalizer_To_Audio_Accept"},
    {opEqualizerToInGameAudioCancel, "OP_Equalizer_To_InGameAudio_Cancel"},
    {opEqualizerToInGameAudioAccept, "OP_Equalizer_To_InGameAudio_Accept"},
    {opAdvancedToAudioCancel,   "OP_Advanced_To_Audio_Cancel"},
    {opAdvancedToAudioAccept,   "OP_Advanced_To_Audio_Accept"},
    {opAdvancedToInGameAudioCancel, "OP_Advanced_To_InGameAudio_Cancel"},
    {opAdvancedToInGameAudioAccept, "OP_Advanced_To_InGameAudio_Accept"},
    {opEffectsToVideoCancel,    "OP_Effects_To_Video_Cancel"},
    {opEffectsToVideoAccept,    "OP_Effects_To_Video_Accept"},
    {opEffectsToInGameVideoCancel, "OP_Effects_To_InGameVideo_Cancel"},
    {opEffectsToInGameVideoAccept, "OP_Effects_To_InGameVideo_Accept"},
    {opKeyCancel,               "OP_Key_Cancel"},
    {kbKeyResetToDefault,       "OP_KeyResetToDefault"},
    {opMusicVolume,             "OP_Music_Volume"},
    {opSFXVolume,               "OP_SFX_Volume"},
    {opSpeechVolume,            "OP_Speech_Volume"},
    {opNumberChannels,          "OP_Num_Channels"},
    {opAutoChannels,            "OP_AutoChannels"},
    {opSpeaker,                 "OP_Speaker"},
    {opSoundQualitySet,         "OP_DCTQuality"},
    {opAdvancedButton,          "OP_Advanced_Button"},
    {opEqualizerButton,         "OP_Equalizer_Button"},
    {opDetailThreshold,         "OP_Detail_Threshold"},
    {opBrightness,              "OP_Brightness"},
    {opLighting,                "OP_Lighting"},
    {opEffects,                 "OP_Effects"},
    {opCustomEffects,           "OP_Custom_Effects"},
    {opResolution,              "OP_Resolution"},
    {opRender,                  "OP_Render"},
    {opEqualizer1,              "OP_Equalizer1"},
    {opEqualizer2,              "OP_Equalizer2"},
    {opEqualizer3,              "OP_Equalizer3"},
    {opEqualizer4,              "OP_Equalizer4"},
    {opEqualizer5,              "OP_Equalizer5"},
    {opEqualizer6,              "OP_Equalizer6"},
    {opEqualizer7,              "OP_Equalizer7"},
    {opEqualizer8,              "OP_Equalizer8"},
    {opVoice,                   "OP_Voice"},
    {opVoice0,                  "OP_Voice0"},
    {opVoice1,                  "OP_Voice1"},
    {opVoice2,                  "OP_Voice2"},
    {opHearVoice1,              "OP_Hear_Voice_1"},
    {opHearVoice2,              "OP_Hear_Voice_2"},
    {opHearVoice3,              "OP_Hear_Voice_3"},
    {opVoiceCommands,           "OP_Voice_Commands"},
    {opVoiceStatus,             "OP_Voice_Status"},
    {opVoiceChatter,            "OP_Voice_Chatter"},
    {opMouseSensitivity,        "OP_Mouse_Sensitivity"},
    {opBattleChatterCB,         "OP_BattleChatter"},
    {opNumberEffects,           "OP_NumEffects"},
    {opCPUDifficulty,           "OP_CPU_Difficulty"},
    {opCPUAttacks,              "OP_CPU_Attacks"},
    {opInfoOverlay,             "OP_Info_Overlay"},
    {opTaskbarUp,               "OP_Taskbar_Up"},
    {opIncreaseChatting,        "OP_Increase_Chatting"},
    {opDecreaseChatting,        "OP_Dencrease_Chatting"},
    {opNumberChatlines,         "OP_Number_Chatlines"},
    {opAutodockHealth,          "OP_Autodock_Health"},
    {opAutodockFuel,            "OP_Autodock_Fuel"},
    {opAutodockHealthValue,     "OP_Autodock_Health_Value"},
    {opAutodockFuelValue,       "OP_Autodock_Fuel_Value"},
    {opNoLOD,                   "OP_NoLOD"},
    {kbListWindowCB,            "OP_KeyBindingsList"},
    {kbPoolListWindowCB,        "OP_KeyBindingsPoolList"},
    {utyYesDropPlayer,          "UTY_DropPlayer"},
    {utyDontDropPlayer,         "UTY_DontDropPlayer"},
    {opCountdownYes,            "OP_Countdown_Yes"},
    {opCountdownNo,             "OP_Countdown_No"},

    {NULL, NULL}
};

fedrawcallback utyDrawCallbacks[] =
{                                                           //base front-end callbacks
    {opDrawNumChannels,         "OP_NumChannels"},
    {versionNumDraw,            "FE_DrawVersionNum"},
    {opNoPalDecrease,           "NP_Decrease"},
    {opNoPalIncrease,           "NP_Increase"},
    {opNoPalDraw,               "NP_Draw"},
    {utyDrawDroppedPlayer,      "UTY_PlayerDropped"},
    {opCountdownBoxDraw,        "OP_DrawCountdownBox"},
    {NULL, NULL}
};


//options file options:
/*
struct
{
    sdword *flag;
    char *flagName;
}
utyOptionsList[] =
{
    &enableSmoothing,               "enableSmoothing",
    &showBackgrounds,               "showBackgrounds",
    &texLinearFiltering,            "texLinearFiltering",
    &enableStipple,                 "enableStipple",
    &enableTrails,                  "enableTrails",
    &strCurLanguage,                "language",
    &smFuzzyBlobs,                  "fuzzyBlobs",
    &utyBaseColor,                  "PlayerBaseColor",
    &utyStripeColor,                "PlayerStripeColor",
    &cpColorsPicked,                "ColorsPicked",
    &whichRaceSelected,             "PlayerRace",
    &smInstantTransition,           "InstantTransition",
    &colPreviousColors[0].base,     "PrevColor0.base",
    &colPreviousColors[0].detail,   "PrevColor0.detail",
    &colPreviousColors[1].base,     "PrevColor1.base",
    &colPreviousColors[1].detail,   "PrevColor1.detail",
    &colPreviousColors[2].base,     "PrevColor2.base",
    &colPreviousColors[2].detail,   "PrevColor2.detail",
    &colPreviousColors[3].base,     "PrevColor3.base",
    &colPreviousColors[3].detail,   "PrevColor3.detail",
    &colPreviousColors[4].base,     "PrevColor4.base",
    &colPreviousColors[4].detail,   "PrevColor4.detail",
    NULL,                           NULL
};

struct
{
    sdword *flag;
    char   *flagName;
}
utyStringOptionsList[] =
{
     (sdword *)utyName              ,   "PlayerName",
     (sdword *)utyPassword          ,   "PlayerPwd",
     (sdword *)opKeymap             ,   "Keymap",
//     utyPassword        ,   "PlayerPassword",
     NULL               ,   NULL
};
*/
//teaser NIS stuff:
//nisheader *utyTeaserHeader;
//nisplaying *utyTeaserPlaying = NULL;

extern sdword opEqualizerSettings[];
extern unsigned long firewallButton;

color versionColor = colWhite;

scriptEntry utyOptionsList[] =
{
    {"noPalMB",                 scriptSetUdwordCB, &opNoPalMB},
    {"effectsLevel",            scriptSetUdwordCB, &opEffectsVal},
    {"speakerSetting",          scriptSetUdwordCB, &opSpeakerSetting},
    {"brightnessVal",           scriptSetUdwordCB, &opBrightnessVal},
    {"detailThreshold",         scriptSetUdwordCB, &opDetailThresholdVal},
    {"noLOD",                   scriptSetUdwordCB, &opNoLODVal},
    {"enableSmoothing",         scriptSetUdwordCB, &enableSmoothing},
    {"showBackgrounds",         scriptSetUdwordCB, &showBackgrounds},
    {"texLinearFiltering",      scriptSetUdwordCB, &texLinearFiltering},
    {"enableStipple",           scriptSetUdwordCB, &enableStipple},
    {"enableTrails",            scriptSetUdwordCB, &enableTrails},
    {"language",                scriptSetUdwordCB, &strCurLanguage},
    {"fuzzyBlobs",              scriptSetUdwordCB, &smFuzzyBlobs},
    {"PlayerBaseColor",         scriptSetUdwordCB, &utyBaseColor},
    {"PlayerStripeColor",       scriptSetUdwordCB, &utyStripeColor},
    {"ColorsPicked",            scriptSetUdwordCB, &cpColorsPicked},
    {"PlayerRace",              scriptSetUdwordCB, &whichRaceSelected},
    {"InstantTransition",       scriptSetUdwordCB, &smInstantTransition},
    {"PrevColor0.base",         scriptSetUdwordCB, &colPreviousColors[0].base},
    {"PrevColor0.detail",       scriptSetUdwordCB, &colPreviousColors[0].detail},
    {"PrevColor1.base",         scriptSetUdwordCB, &colPreviousColors[1].base},
    {"PrevColor1.detail",       scriptSetUdwordCB, &colPreviousColors[1].detail},
    {"PrevColor2.base",         scriptSetUdwordCB, &colPreviousColors[2].base},
    {"PrevColor2.detail",       scriptSetUdwordCB, &colPreviousColors[2].detail},
    {"PrevColor3.base",         scriptSetUdwordCB, &colPreviousColors[3].base},
    {"PrevColor3.detail",       scriptSetUdwordCB, &colPreviousColors[3].detail},
    {"PrevColor4.base",         scriptSetUdwordCB, &colPreviousColors[4].base},
    {"PrevColor4.detail",       scriptSetUdwordCB, &colPreviousColors[4].detail},
    {"PlayerName",              scriptSetStringCB, &utyName},
    {"PlayerPwd",               scriptSetStringCB, &utyPassword},
//    {"Keymappings",             scriptSetStringCB, &opKeymap},
    {"NEXT_FORMATION",          scriptSetUdwordCB, &kbKeySavedKeys[kbNEXT_FORMATION]},
    {"BUILD_MANAGER",           scriptSetUdwordCB, &kbKeySavedKeys[kbBUILD_MANAGER]},
    {"PREVIOUS_FOCUS",          scriptSetUdwordCB, &kbKeySavedKeys[kbPREVIOUS_FOCUS]},
    {"NEXT_FOCUS",              scriptSetUdwordCB, &kbKeySavedKeys[kbNEXT_FOCUS]},
    {"DOCK",                    scriptSetUdwordCB, &kbKeySavedKeys[kbDOCK]},
    {"SELECT_ALL_VISIBLE",      scriptSetUdwordCB, &kbKeySavedKeys[kbSELECT_ALL_VISIBLE]},
    {"FOCUS",                   scriptSetUdwordCB, &kbKeySavedKeys[kbFOCUS]},
    {"RESEARCH_MANAGER",        scriptSetUdwordCB, &kbKeySavedKeys[kbRESEARCH_MANAGER]},
    {"HARVEST",                 scriptSetUdwordCB, &kbKeySavedKeys[kbHARVEST]},
    {"MOVE",                    scriptSetUdwordCB, &kbKeySavedKeys[kbMOVE]},
    {"NEXT_TACTIC",             scriptSetUdwordCB, &kbKeySavedKeys[kbNEXT_TACTIC]},
    {"PREVIOUS_TACTIC",         scriptSetUdwordCB, &kbKeySavedKeys[kbPREVIOUS_TACTIC]},
    {"SCUTTLE",                 scriptSetUdwordCB, &kbKeySavedKeys[kbSCUTTLE]},
    {"SHIP_SPECIAL",            scriptSetUdwordCB, &kbKeySavedKeys[kbSHIP_SPECIAL]},
    {"TACTICAL_OVERLAY",        scriptSetUdwordCB, &kbKeySavedKeys[kbTACTICAL_OVERLAY]},
//    {"SENSORS_MANAGER",         scriptSetUdwordCB, &kbKeySavedKeys[kbSENSORS_MANAGER]},
    {"MOTHERSHIP",              scriptSetUdwordCB, &kbKeySavedKeys[kbMOTHERSHIP]},
    {"KAMIKAZE",                scriptSetUdwordCB, &kbKeySavedKeys[kbKAMIKAZE]},
    {"CANCEL_ORDERS",           scriptSetUdwordCB, &kbKeySavedKeys[kbCANCEL_ORDERS]},
    {"LAUNCH_MANAGER",          scriptSetUdwordCB, &kbKeySavedKeys[kbLAUNCH_MANAGER]},
    {"Equalizer0",              scriptSetUdwordCB, &opEqualizerSettings[0]},
    {"Equalizer1",              scriptSetUdwordCB, &opEqualizerSettings[1]},
    {"Equalizer2",              scriptSetUdwordCB, &opEqualizerSettings[2]},
    {"Equalizer3",              scriptSetUdwordCB, &opEqualizerSettings[3]},
    {"Equalizer4",              scriptSetUdwordCB, &opEqualizerSettings[4]},
    {"Equalizer5",              scriptSetUdwordCB, &opEqualizerSettings[5]},
    {"Equalizer6",              scriptSetUdwordCB, &opEqualizerSettings[6]},
    {"Equalizer7",              scriptSetUdwordCB, &opEqualizerSettings[7]},
    {"MusicVolume",             scriptSetUdwordCB, &opMusicVol},
    {"SFXVolume",               scriptSetUdwordCB, &opSFXVol},
    {"SpeechVolume",            scriptSetUdwordCB, &opSpeechVol},
    {"Voice0",                  scriptSetUdwordCB, &opVoice0On},
    {"Voice1",                  scriptSetUdwordCB, &opVoice1On},
    {"Voice2",                  scriptSetUdwordCB, &opVoice2On},
    {"VoiceCommands",           scriptSetUdwordCB, &opVoiceComm},
    {"VoiceStatus",             scriptSetUdwordCB, &opVoiceStat},
    {"VoiceChatter",            scriptSetUdwordCB, &opVoiceChat},
    {"opMouseSens",             scriptSetUdwordCB, &opMouseSens},
    {"InfoOverlay",             scriptSetUdwordCB, &opInfoOverlayVar},
    {"opBattleChatter",         scriptSetUdwordCB, &opBattleChatter},
    {"spCurrentSelected",       scriptSetUdwordCB, &spCurrentSelected},
    {"etgDamageEffectsEnabled", scriptSetUdwordCB, &etgDamageEffectsEnabled},
    {"etgHitEffectsEnabled",    scriptSetUdwordCB, &etgHitEffectsEnabled},
    {"etgFireEffectsEnabled",   scriptSetUdwordCB, &etgFireEffectsEnabled},
    {"etgBulletEffectsEnabled", scriptSetUdwordCB, &etgBulletEffectsEnabled},
    {"etgHistoryScalar",        scriptSetUdwordCB, &etgHistoryScalar},
    {"NumChannels",             scriptSetUdwordCB, &opNumChannels},
    {"AutoChannel",             scriptSetUdwordCB, &opAutoChannel},
    {"SoundQuality",            scriptSetUdwordCB, &opSoundQuality},
    {"FirewallDetect",          scriptSetUdwordCB, &firewallButton},
    {"TutorialNeeded",          scriptSetUdwordCB, &needtutorial},
    {"MPGameFlags",             scriptSetUdwordCB, &tpGameCreated.pad2},
    {"MultiPlayerGameSetting",  scriptSetUdwordCB, &tpGameCreated.numComputers},
    {"ResInjInterval",          scriptSetUdwordCB, &tpGameCreated.resourceInjectionInterval},
    {"ResInjAmmount",           scriptSetUdwordCB, &tpGameCreated.resourceInjectionsAmount},
    {"ResLumpSumTime",          scriptSetUdwordCB, &tpGameCreated.resourceLumpSumTime},
    {"ResLumpSumAmmount" ,      scriptSetUdwordCB, &tpGameCreated.resourceLumpSumAmount},
    endEntry
};
/*=============================================================================
    Functions:
=============================================================================*/

void utyVideoPlay(char* name, featom* atom)
{
    bool active;

    active = glcActive();
    if (active)
    {
        glcActivate(FALSE);
    }
    animBinkPlay(0,1);
    if (active)
    {
        glcActivate(TRUE);
    }

#ifdef DEBUG_STOMP
    regVerify((regionhandle)&regRootRegion);
#endif
    bitSet(regRootRegion.status, RSF_DrawThisFrame);
    regDirtyEverythingUpwards(atom->region);
    feRenderEverything = TRUE;
}

void utyTutorialAlertCancel(char* name, featom* atom)
{
    while (feStackIndex)
    {
        feScreenDisappear(NULL, NULL);
    }
    feScreenStart(ghMainRegion, "Main_game_screen");
#ifdef DEBUG_STOMP
    regVerify((regionhandle)&regRootRegion);
#endif
    bitSet(regRootRegion.status, RSF_DrawThisFrame);
    regDirtyEverythingUpwards(atom->region);
    feRenderEverything = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : versionNumDraw
    Description : draws version number of Homeworld
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void versionNumDraw(featom *atom, regionhandle region)
{
    rectangle pos=region->rect;
    sdword fontheight;
    fonthandle oldfont;
    char versionstr[70];

    primRectSolid2(&region->rect, colBlack);

    strcpy(versionstr, strGetString(strVersion));
    strcat(versionstr, " ");
    strcat(versionstr, networkVersion);
    strcat(versionstr, ".");
    strcat(versionstr, minorBuildVersion);

    dbgAssert(strlen(versionstr) < 70);

    oldfont = fontMakeCurrent(selGroupFont2);
    fontheight = fontHeight(" ");

    pos.y0 += fontheight;

    fontPrint(pos.x0, pos.y0, versionColor, versionstr);

#ifndef HW_Release
    pos.y0 += fontheight;
    fontPrint(pos.x0, pos.y0, versionColor, (char*)GLC_VENDOR);
    pos.y0 += fontheight;
    fontPrint(pos.x0, pos.y0, versionColor, (char*)GLC_RENDERER);
#endif

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : utyOptionsFileRead
    Description : Read a set of options from the Homeworld.ini file
    Inputs      : void
    Outputs     : reads in the switches listed in utyOptionsList array
    Return      :
----------------------------------------------------------------------------*/
#define UTY_StringLength        80
#define UTY_FileName            "Homeworld.ini"
#define UTY_SectionName         "Options"
void utyOptionsFileRead(void)
{
    /*
    sdword index;
    char returnString[UTY_StringLength];
    char defaultString[UTY_StringLength];

    for (index = 0; utyOptionsList[index].flag != NULL; index++)
    {
        sprintf(defaultString, "%d", *utyOptionsList[index].flag);
        GetPrivateProfileString(UTY_SectionName, utyOptionsList[index].flagName,
                                defaultString, returnString, UTY_StringLength, UTY_FileName);
        sscanf(returnString, "%d", utyOptionsList[index].flag);
    }

    for (index = 0; utyStringOptionsList[index].flag != NULL; index++)
    {
        sprintf(defaultString, "%s", (char *)utyStringOptionsList[index].flag);
        GetPrivateProfileString(UTY_SectionName, utyStringOptionsList[index].flagName,
                                defaultString, returnString, UTY_StringLength, UTY_FileName);
        strcpy((char *)utyStringOptionsList[index].flag,returnString);
    }
    */

    scriptSetFileSystem("", DIS_FileName, utyOptionsList);

    //call any functions that need to acknowledge a change due to loading
    cameraSensitivitySet(opMouseSens);
    battleChatterFrequencySet(opBattleChatter);

    utyRegistryOptionsRead();
}

/*-----------------------------------------------------------------------------
    Name        : utyOptionsFileWrite
    Description : Write a set of options from the Homeworld.ini file
    Inputs      : void
    Outputs     : writes out the switches listed in utyOptionsList array
    Return      :
----------------------------------------------------------------------------*/
void utyOptionsFileWrite(void)
{
    sdword index;
    FILE *f;
/*
    char returnString[UTY_StringLength];

    for (index = 0; utyOptionsList[index].flag != NULL; index++)
    {
        sprintf(returnString, "%d", *utyOptionsList[index].flag);
        WritePrivateProfileString(UTY_SectionName, utyOptionsList[index].flagName,
                                returnString, UTY_FileName);
    }

    for (index = 0; utyStringOptionsList[index].flag != NULL; index++)
    {
        sprintf(returnString, "%s", (char *)utyStringOptionsList[index].flag);
        WritePrivateProfileString(UTY_SectionName, utyStringOptionsList[index].flagName,
                                returnString, UTY_FileName);
    }
*/
    f = fopen(DIS_FileName, "wt");
    if (f == NULL)
    {
        goto REGISTRY;
    }
    for (index = 0; utyOptionsList[index].name != NULL; index++)
    {
        if (utyOptionsList[index].setVarCB == scriptSetUdwordCB)
        {
            fprintf(f, "%s    %d\n", utyOptionsList[index].name, *((udword *)utyOptionsList[index].dataPtr));
        }
        else
        {
            fprintf(f, "%s    %s\n", utyOptionsList[index].name, utyOptionsList[index].dataPtr);
        }
    }
    fclose(f);

REGISTRY:
    utyRegistryOptionsWrite();
}

/*-----------------------------------------------------------------------------
    Name        : utyPickColors
    Description : Pick some colors (just testing for now)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
color utyBaseColor;
color utyStripeColor;
//bool utyColorsPicked = FALSE;

void utyPickColors(char *name, featom *atom)
{
    cpColorsPick(&utyBaseColor, &utyStripeColor, &whichRaceSelected);
    //utyColorsPicked = TRUE;
//    cpColorsPick(&teColorSchemes[0].textureColor.base,
                 //&teColorSchemes[0].textureColor.detail);
}

/*-----------------------------------------------------------------------------
    Control handlers for the options screen:
-----------------------------------------------------------------------------*/
void utySetCustomEffects(void)
{
    opCustomEffectsToggled = TRUE;
}
void utyUnsetCustomEffects(void)
{
    opOptionsSaveCustomEffectsSettings();
    opCustomEffectsToggled = FALSE;
}

void utyMagFilterToggle(char* name, featom* atom)
{
    extern sdword texLinearMag;

    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, texLinearMag);
        utyUnsetCustomEffects();
    }
    else
    {
        if (FECHECKED(atom))
        {
            texLinearMag = TRUE;
        }
        else
        {
            texLinearMag = FALSE;
        }
    }

    trFilterEnable(texLinearFiltering);
}

void utyFilterToggle(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, texLinearFiltering);
        utyUnsetCustomEffects();
    }
    else
    {
        if (FECHECKED(atom))
            texLinearFiltering = TRUE;
        else
            texLinearFiltering = FALSE;
        feToggleButtonSet("VO_BitmapFilter",texLinearFiltering);
    }

    trFilterEnable(texLinearFiltering);

    utySetCustomEffects();
}
void utyFilterBitmapToggle(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, texLinearFiltering);
    }
    else
    {
        if (FECHECKED(atom))
            texLinearFiltering = TRUE;
        else
            texLinearFiltering = FALSE;
        feToggleButtonSet("VO_Filter",texLinearFiltering);
    }

    trFilterEnable(texLinearFiltering);
}
void utyBackgroundToggle(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, showBackgrounds);
    }
    else
    {
        if (FECHECKED(atom))
            showBackgrounds = TRUE;
        else
            showBackgrounds = FALSE;
        feToggleButtonSet("VO_BitmapBackground",showBackgrounds);
    }

    utySetCustomEffects();
}
void utyBackgroundBitmapToggle(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, showBackgrounds);
    }
    else
    {
        if (FECHECKED(atom))
            showBackgrounds = TRUE;
        else
            showBackgrounds = FALSE;
        feToggleButtonSet("VO_Background",showBackgrounds);
    }
}
void utyInstantTransitionToggle(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, smInstantTransition);
    }
    else
    {
        if (FECHECKED(atom))
            smInstantTransition = TRUE;
        else
            smInstantTransition = FALSE;
        feToggleButtonSet("VO_BitmapInstantTransition",smInstantTransition);
    }

    utySetCustomEffects();
}
void utyInstantTransitionBitmapToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, smInstantTransition);
    }
    else
    {
        if (FECHECKED(atom))
            smInstantTransition = TRUE;
        else
            smInstantTransition = FALSE;
        feToggleButtonSet("VO_InstantTransition", smInstantTransition);
    }
}
void utySmoothingToggle(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, enableSmoothing);
    }
    else
    {
        if (FECHECKED(atom))
            enableSmoothing = TRUE;
        else
            enableSmoothing = FALSE;
        feToggleButtonSet("VO_BitmapSmoothing",enableSmoothing);
    }

    utySetCustomEffects();
}
void utySmoothingBitmapToggle(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, enableSmoothing);
    }
    else
    {
        if (FECHECKED(atom))
            enableSmoothing = TRUE;
        else
            enableSmoothing = FALSE;
        feToggleButtonSet("VO_Smoothing",enableSmoothing);
    }
}
void utyStippleToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, enableStipple);
    }
    else
    {
        if (FECHECKED(atom))
            enableStipple = TRUE;
        else
            enableStipple = FALSE;
        feToggleButtonSet("VO_BitmapStipple",enableStipple);

        if (RGL)
        {
            if (enableStipple)
            {
                glEnable(GL_POLYGON_STIPPLE);
            }
            else
            {
                glDisable(GL_POLYGON_STIPPLE);
            }
        }
    }

    utySetCustomEffects();
}
void utyStippleBitmapToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, enableStipple);
    }
    else
    {
        if (FECHECKED(atom))
            enableStipple = TRUE;
        else
            enableStipple = FALSE;
        feToggleButtonSet("VO_Stipple",enableStipple);

        if (RGL)
        {
            if (enableStipple)
            {
                glEnable(GL_POLYGON_STIPPLE);
            }
            else
            {
                glDisable(GL_POLYGON_STIPPLE);
            }
        }
    }
}
void utyTrailsToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, enableTrails);
    }
    else
    {
        if (FECHECKED(atom))
            enableTrails = TRUE;
        else
            enableTrails = FALSE;
        feToggleButtonSet("VO_BitmapTrails", enableTrails);
    }

    utySetCustomEffects();
}
void utyTrailsBitmapToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, enableTrails);
    }
    else
    {
        if (FECHECKED(atom))
            enableTrails = TRUE;
        else
            enableTrails = FALSE;
        feToggleButtonSet("VO_Trails", enableTrails);
    }
}
void utyHitToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, etgHitEffectsEnabled);
    }
    else
    {
        if (FECHECKED(atom))
            etgHitEffectsEnabled = TRUE;
        else
            etgHitEffectsEnabled = FALSE;
        feToggleButtonSet("VO_BitmapHitEffects", etgHitEffectsEnabled);
    }

    utySetCustomEffects();
}
void utyHitBitmapToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, etgHitEffectsEnabled);
    }
    else
    {
        if (FECHECKED(atom))
            etgHitEffectsEnabled = TRUE;
        else
            etgHitEffectsEnabled = FALSE;
        feToggleButtonSet("VO_Hit", etgHitEffectsEnabled);
    }
}
void utyDamageToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, etgDamageEffectsEnabled);
    }
    else
    {
        if (FECHECKED(atom))
            etgDamageEffectsEnabled = TRUE;
        else
            etgDamageEffectsEnabled = FALSE;
        feToggleButtonSet("VO_BitmapDamage", etgDamageEffectsEnabled);
    }

    utySetCustomEffects();
}
void utyDamageBitmapToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, etgDamageEffectsEnabled);
    }
    else
    {
        if (FECHECKED(atom))
            etgDamageEffectsEnabled = TRUE;
        else
            etgDamageEffectsEnabled = FALSE;
        feToggleButtonSet("VO_Damage", etgDamageEffectsEnabled);
    }
}
void utyMuzzleToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, etgFireEffectsEnabled);
    }
    else
    {
        if (FECHECKED(atom))
            etgFireEffectsEnabled = TRUE;
        else
            etgFireEffectsEnabled = FALSE;
        feToggleButtonSet("VO_BitmapMuzzle", etgFireEffectsEnabled);
    }

    utySetCustomEffects();
}
void utyMuzzleBitmapToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, etgFireEffectsEnabled);
    }
    else
    {
        if (FECHECKED(atom))
            etgFireEffectsEnabled = TRUE;
        else
            etgFireEffectsEnabled = FALSE;
        feToggleButtonSet("VO_Muzzle", etgFireEffectsEnabled);
    }
}
void utyBulletToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, etgBulletEffectsEnabled);
    }
    else
    {
        if (FECHECKED(atom))
            etgBulletEffectsEnabled = TRUE;
        else
            etgBulletEffectsEnabled = FALSE;
        feToggleButtonSet("VO_BitmapBullet", etgBulletEffectsEnabled);
    }

    utySetCustomEffects();
}
void utyBulletBitmapToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, etgBulletEffectsEnabled);
    }
    else
    {
        if (FECHECKED(atom))
            etgBulletEffectsEnabled = TRUE;
        else
            etgBulletEffectsEnabled = FALSE;
        feToggleButtonSet("VO_Bullet", etgBulletEffectsEnabled);
    }
}


void utySensorsBlobsToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, smFuzzyBlobs);
    }
    else
    {
        if (FECHECKED(atom))
            smFuzzyBlobs = TRUE;
        else
            smFuzzyBlobs = FALSE;
        feToggleButtonSet("VO_BitmapSensorsBlobs",smFuzzyBlobs);
    }

    utySetCustomEffects();
}
void utySensorsBlobsBitmapToggle(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, smFuzzyBlobs);
    }
    else
    {
        if (FECHECKED(atom))
            smFuzzyBlobs = TRUE;
        else
            smFuzzyBlobs = FALSE;
        feToggleButtonSet("VO_SensorsBlobs",smFuzzyBlobs);
    }
}

void utyLanguageToggle(char* name, featom* atom)
{
    sdword index;
//#if defined(CGW) || defined(Downloadable) || defined(DLPublicBeta) || defined(OEM)
//    //disable this function in demos
//    bitSet(atom->flags, FAF_Disabled);
//    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
//#else
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, strCurLanguage);
    }
    else
    {
        strLoadLanguage((strLanguageType)atom->pData);
        frReloadFonts();
//        bitSet(feStack[feStackIndex].baseRegion->flags, RSF_DrawThisFrame);
#ifdef DEBUG_STOMP
        regVerify((regionhandle)&regRootRegion);
#endif
        bitSet(regRootRegion.status, RSF_DrawThisFrame);           //flag everything to be redrawn
        regDirtyEverythingUpwards(atom->region);
        for (index = 0; index <= feStackIndex; index++)
        {
            feScreenAllHotKeysUpdate(feStack[feStackIndex].screen);
        }
    }
//#endif
}

//void utyScenarioPick(char *name, featom *atom)
//{
//    spScenarioPick(NULL);//!!! make some real pointer
//}

extern bool nisScreenStarted;
void utyHideMenuOrGotoQuitGame(char *name, featom *atom)
{
/*
    if (nisEnabled && utyTeaserPlaying != NULL && atom->hotKey[strCurLanguage] == ESCKEY)
    {
        keyClearAll();
        feScreenDisappear(NULL, NULL);
        mouseCursorHide();
        nisScreenStarted = FALSE;
    }
    else
*/
    {
        feScreenStart(feStack[feStackIndex].baseRegion, "Quit_game");
    }
}

/*-----------------------------------------------------------------------------
    Name        : utyCloseOK
    Description : generates an exit command message
    Inputs      : generic region callback
    Outputs     : ..
    Return      : generic region callback
----------------------------------------------------------------------------*/
udword utyCloseOK(regionhandle region, sdword ID, udword event, udword data)
{
    PostMessage(ghMainWindow, WM_COMMAND, CID_ExitOK, 0);   //exit the game
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : utyFatalErrorWaitLoop
    Description : Handles messages in case of fatal errors.  Also sends message
                    to create a message box.
    Inputs      : exitCode - code passed to exit()
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void utyFatalErrorWaitLoop(int exitCode)
{
    MSG   msg;

    OutputDebugString(dbgFatalErrorString);                 //print error string to the debugger

    while (PeekMessage(&msg, ghMainWindow, 0, 0, PM_REMOVE))//remove all messages from Queue
        ;
    SendMessage(ghMainWindow, WM_COMMAND, CID_ExitError, 0);//tell app to exit

    while (PeekMessage(&msg, ghMainWindow, 0, 0, PM_REMOVE))//remove all messages from Queue
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    exit(exitCode);
}

/*-----------------------------------------------------------------------------
    Name        : utyNonFatalErrorWaitLoop
    Description : Handles messages in case of fatal errors.  Also sends message
                    to create a message box.
    Outputs     : ..
    Return      : TRUE if the CANCEL button (debug) was selected
----------------------------------------------------------------------------*/
sdword utyNonFatalErrorWaitLoop(void)
{
    int returnValue;

    OutputDebugString(dbgFatalErrorString);                 //print error string to the debugger
    returnValue = MessageBox(ghMainWindow, dbgFatalErrorString, "Non-Fatal Error (cancel to debug)", MB_ICONSTOP | MB_OKCANCEL);

    return(returnValue == IDCANCEL);
}

/*-----------------------------------------------------------------------------
    Name        : gameStart
    Description : starts the game
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
//needed to be made global
bool8 etgHasBeenStarted;
void FancyFightPreLoad(void);

void gameStart(char *loadfilename)
{
    char *useScenarioFile;
    Player *player;
    uword i;
    udword numCompPlayers = 0;

#if MAIN_Password && MAIN_CDCheck
    if (mainCDCheckEnabled)
    {
        if (!fileExists(utyMusicFilename, FF_CDROM))
        {
            dbgFatalf(DBG_Loc, "Homeworld CD not detected.");
        }
    }
#endif
    soundEventStopTrack(SOUND_FRONTEND_TRACK, 1.0f);                          //stop it

    if (loadfilename != NULL)
    {
        PreLoadGame(loadfilename);                          // sets numPlayers, races of players, static info needed
    }

    dbgAssert(numPlayers > 0);
    dbgAssert(numPlayers <= MAX_MULTIPLAYER_PLAYERS);
    universe.numPlayers = numPlayers;

    unitCapInitStatics(numPlayers);
/*
    if (nisEnabled && utyTeaserPlaying != NULL)
    {
        mouseCursorShow();
        utyTeaserEnd();
    }
*/
    tbStartup();                                            //start the task bar

    wkTradeStuffActive = FALSE;

    utySet(SSA_TaskBar);

    (void)glcActivate(FALSE);

    //reset deterministic build process
    cmDeterministicReset();

    //free the alternate list
    partShutdown();

    if (loadfilename == NULL)
    {
        if (singlePlayerGame)
        {
            aiplayerStartup(2,1,1);     // tpGameCreated not valid for singlePlayerGame
        }
        else
        {
            aiplayerChangeBigotry(tpGameCreated.aiplayerBigotry);
            aiplayerStartup(numPlayers, tpGameCreated.numPlayers, tpGameCreated.numComputers);
        }


        utySet2(SS2_AIPlayer);

        cpColorsAddToPreviousList(utyBaseColor, utyStripeColor);

        for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
        {
            // initialize player alliance information
            universe.players[i].Allies = 0;
            universe.players[i].AllianceProposals = 0;
            universe.players[i].AllianceProposalsTimeout = 0;
        }
    }

    if (multiPlayerGame)
    {
        dbgAssert(numPlayers == tpGameCreated.numPlayers+tpGameCreated.numComputers);
        dbgAssert(loadfilename == NULL);                    // don't allow loading of games for network games

#if UTY_PLAYER_LOGGING
        logfileLogf("PlayerStart.log", "Starting '%s'\n", tpGameCreated.Name);
#endif
        for (i=0;i<numPlayers;i++)
        {
            // later check if Colors Picked?  Fix later
            if (i <numPlayers-tpGameCreated.numComputers)
            {
                // human player setup
                if (tpGameCreated.playerInfo[i].colorsPicked)
                {                                           //if this player picked their color properly
                    teTeamColorsSet(i, tpGameCreated.playerInfo[i].baseColor, tpGameCreated.playerInfo[i].stripeColor);
                }
                else
                {
                    teTeamColorsSet(i, teColorSchemes[i].textureColor.base,teColorSchemes[i].textureColor.detail);
                }
                universe.players[i].race = tpGameCreated.playerInfo[i].race;
                dbgAssert(strlen(tpGameCreated.playerInfo[i].PersonalName) < MAX_PERSONAL_NAME_LEN);
                strcpy(playerNames[i],tpGameCreated.playerInfo[i].PersonalName);
            }
            else
            {
                universe.players[i].race = tpGameCreated.playerInfo[i].race;
                if (i == 1 && cpColorsPicked)
                {                                           //if second player is computer and player picked colors
                    if (universe.players[1].race == R1)
                    {                                       //choose color appropriate to that race
                        teTeamColorsSet(1, teRace1Default.base, teRace1Default.detail);
                    }
                    else
                    {
                        teTeamColorsSet(1, teRace2Default.base, teRace2Default.detail);
                    }
                }
                else
                {
                    teTeamColorsSet(i, teColorSchemes[i].textureColor.base,teColorSchemes[i].textureColor.detail);
                }
                sprintf(playerNames[i], "%s %i", strGetString(strComputerName), i-sigsNumPlayers+1);
            }
#if UTY_PLAYER_LOGGING
            logfileLogf("PlayerStart.log", "%d %d 0x%x 0x%x %s\n", i, tpGameCreated.playerInfo[i].race, teColorSchemes[i].textureColor.base, teColorSchemes[i].textureColor.detail, tpGameCreated.playerInfo[i].PersonalName);
#endif
        }

        useScenarioFile = tpGameCreated.MapName;
    }
    else
    {
        ShipRace otherRace;

        if (loadfilename == NULL)
        {
            if (cpColorsPicked)
            {
                teTeamColorsSet(0, utyBaseColor, utyStripeColor);
            }
            else
            {
                teTeamColorsSet(0, teColorSchemes[0].textureColor.base, teColorSchemes[0].textureColor.detail);
            }

            if (tutorial==1)
            {
                // !!! hack-o-rama (does not work with R2; Fix this Jason!)
                //whichRaceSelected = R1;
                universe.players[0].race = whichRaceSelected;
                universe.players[1].race = (whichRaceSelected == R1) ? R2 : R1;
                ComputerPlayerEnabled[1] = TRUE;
            }
            else
            {
                useScenarioFile = spScenarios[spCurrentSelected].fileSpec;

                //set race for player
                universe.players[0].race = whichRaceSelected;

                dbgMessagef("\nplayer is race %d", whichRaceSelected);
                otherRace = (whichRaceSelected == R1) ? R2 : R1;
                for (i = 1; i < numPlayers; i++)
                {
                    universe.players[i].race = otherRace;

                }
                if (universe.players[1].race == R1)
                {                                       //choose color appropriate to that race
                    teTeamColorsSet(1, teRace1Default.base, teRace1Default.detail);
                }
                else
                {
                    teTeamColorsSet(1, teRace2Default.base, teRace2Default.detail);
                }

                if (!noDefaultComputerPlayer)
                {
                    if ((numPlayers == 2) && (!singlePlayerGame))
                    {
                        ComputerPlayerEnabled[1] = TRUE;
                    }
                }

                if (singlePlayerGame)
                {
                    ComputerPlayerEnabled[1] = TRUE;
                }
            }
        }
        else
        {
            for (i=0;i<universe.numPlayers;i++)
            {
                teTeamColorsSet(i, teColorSchemes[i].textureColor.base, teColorSchemes[i].textureColor.detail);
            }
        }
    }

    if (multiPlayerGame|playPackets)
    {
        gcStartup();
        utySet2(SS2_GameChat);
    }

    //universeDefaultTeamColors(); NO LONGER NEEDED - levelPreInit calls this.  Also, SaveGame doesn't need this.

    if (tutorial==1)
    {
        tutPreInitTutorial(dirfile, levelfile);
    }
    else if (!singlePlayerGame)
    {
        if (loadfilename == NULL)
        {
            sprintf(levelfile,useScenarioFile,numPlayers);
            strcpy(dirfile,"MultiPlayer\\");
            strcat(dirfile,levelfile);
            strcat(dirfile,"\\");
            strcat(levelfile,".level");

            //perform a level pass to see what ships we need to load
            levelPreInit(dirfile, levelfile);
        }
        // else PreLoadGame already figures out what ships we need to load
    }
    else
    {
        //loadfilename == NULL then singlePlayerGameInfo.currentMission set from singlePlayerInit called earlier
        //else loadfilename != NULL then singlePlayerGameInfo.currentMission will be set from PreLoadGame
        //!!!do a pass on the first SP level and find what ships/textures are needed
        if (loadfilename == NULL)
        {
            GetMissionsDirAndFile(singlePlayerGameInfo.currentMission);
            levelPreInit(spMissionsDir, spMissionsFile);
        }
        // else PreLoadGame already figures out what ships we need to load
    }

    if (gatherStats|showStatsFight|showStatsFancyFight)
    {
        dbgAssert(loadfilename == NULL);        // can't load games when doing stats stuff
        if (showStatsFancyFight)
            FancyFightPreLoad();
        else
            universeFlagEverythingNeeded();
    }

    /* pause sound engine */
    soundEventPause(TRUE);

    //load in all the ships, register all the textures
    universeStaticInit();
    if (hrAbortLoadingGame)
    {
        goto abortloading;
    }

    if (loadfilename == NULL)
    {
        universe.numPlayers = numPlayers;

        //initialize stats gathering variables
        initGameStats(&universe);
        BountyInit(tpGameCreated.bountySize);
        memset(&universe.players[MAX_MULTIPLAYER_PLAYERS],0,sizeof(Player));
        universe.players[MAX_MULTIPLAYER_PLAYERS].playerIndex = MAX_MULTIPLAYER_PLAYERS;
        for (i=0;i<universe.numPlayers;i++)
        {
            player = &universe.players[i];

            player->PlayerMothership = NULL;
            player->resourceUnits = 0;
            player->autoLaunch = BIT0 | BIT1 | BIT2 | BIT3 | BIT4;
            player->playerState = PLAYER_ALIVE;
            player->bounty        = TW_BountySizes[tpGameCreated.bountySize];

                        // Initialize units to zero for this player
            unitCapInitialize(player);

            // Initialize research structure, only for computer players, and human players on this machine

            // ****** IMPORTANT we have changed the research so that it is deterministic on all machines.
            if (ComputerPlayerEnabled[i])
                rmInitializeResearchStruct(player, TRUE, (singlePlayerGame) ? TECH_ALLTECHNOLOGY : TECH_NOTECHNOLOGY);
            else
                rmInitializeResearchStruct(player, TRUE, TECH_NOTECHNOLOGY);

    #if MAIN_SENSOR_LEVEL
            if (multiPlayerGame)
            {
                player->sensorLevel = 0;
            }
            else
            {
                player->sensorLevel = (ubyte)initialSensorLevel;
            }
    #else //MAIN_SENSOR_LEVEL
            player->sensorLevel = 0;
    #endif //MAIN_SENSOR_LEVEL
            player->playerIndex = (uword)i;
            player->aiPlayer = NULL;
            if (ComputerPlayerEnabled[i])
            {
                if (singlePlayerGame)
                    player->aiPlayer = aiplayerInit(player,AI_ADV);
                else
                    player->aiPlayer = aiplayerInit(player,ConvertSliderToAIPlayerDifficulty(tpGameCreated.aiplayerDifficultyLevel));

            }
            player->timeMoShipAttacked = universe.totaltimeelapsed;
        }
    }

    trailsRecolorize();
    mistrailsRecolorize();

    tmClearTechs();

    if (loadfilename == NULL)
    {
        univupdateReset();                                      //reset univupdate
        ranStartup();                                           //restart the random-number generator
    }
    else
    {
        LoadGame(loadfilename);
    }

    //now start the level
    if (tutorial==1)
    {
        aiCurrentAIPlayer = universe.players[1].aiPlayer;
        aiCurrentAIPlayer->primaryEnemyPlayer = &universe.players[0];
        if (loadfilename == NULL)
            tutInitTutorial(dirfile, levelfile);
        tutInitialize();
    }
    else if (singlePlayerGame)
    {
        aiCurrentAIPlayer = universe.players[1].aiPlayer;
        aiCurrentAIPlayer->primaryEnemyPlayer = &universe.players[0];
        if (loadfilename == NULL)
            singlePlayerStartGame();
    }
    else
    {
        if (showStatsFancyFight)
        {
            smGhostMode = TRUE;
            dbgAssert(loadfilename == NULL);
            statsLoadFightStats();
            statsShowFancyFight(showStatsFightI,showStatsFightJ);
        }
        else if (showStatsFight)
        {
            dbgAssert(loadfilename == NULL);
            statsLoadFightStats();
            statsShowFight(showStatsFightI,showStatsFightJ);
        }
        else
        {
            if (gatherStats)
            {
                dbgAssert(loadfilename == NULL);
                statsGatherFightStats();
            }
            else
            {
                statsLoadFightStats();
            }
            if (loadfilename == NULL)
                levelInit(dirfile,levelfile);
        }

        if (loadfilename == NULL)
            for (i=0;i<universe.numPlayers;i++)
            {
                switch (tpGameCreated.startingResources)
                {
                    case MG_Small :
                        universe.players[i].resourceUnits = resourceStartSmall;
                        universe.gameStats.startingResources = resourceStartSmall;
                    break;
                    case MG_Medium :
                        universe.players[i].resourceUnits = resourceStartMedium;
                        universe.gameStats.startingResources = resourceStartMedium;
                    break;
                    case MG_Large :
                        universe.players[i].resourceUnits = resourceStartLarge;
                        universe.gameStats.startingResources = resourceStartLarge;
                    break;
                    default:
                        universe.players[i].resourceUnits = 10;
                        universe.gameStats.startingResources = 10;
                    break;
                }
            }

        if (loadfilename == NULL)
        {
            if (!bitTest(tpGameCreated.flag,MG_HarvestinEnabled))
            {
                rmEnableShip(R1, ResourceCollector, FALSE);
                rmEnableShip(R2, ResourceCollector, FALSE);
            }
        }
    }

    if (recordplayPacketsInGame)
    {
        smGhostMode = TRUE;
    }

    if (loadfilename == NULL)
    {
        udword techLevel = 0;

        dbgMessagef("\nSYNC NUMBER: %d",gamerand());

        if ((!singlePlayerGame) && (!bitTest(tpGameCreated.flag,MG_ResearchEnabled)))
        {
            techLevel = AllTechnology;
        }

        for (i=0;i<universe.numPlayers;i++)
        {
            rmAddTechToPlayer(&universe.players[i], techLevel);
        }

        for (i=0;i<universe.numPlayers;i++)
        {
            player = &universe.players[i];
            if (player->aiPlayer != NULL)
            {
                aiplayerGameStart(player->aiPlayer);
            }
        }
    }
    else
    {
        soundEventPlayMusic(SongNumber);
    }

    if (etgHasBeenStarted)
    {                                                       //if effects already started
        etgReloadReset();
    }
    else
    {
        etgStartup();
        utySet(SSA_Effect);
        etgHasBeenStarted = TRUE;
    }
    pingReset();                                            //clear out all pings

    //clear out any persistent subtitles
    subReset();
    subTexturesReset();

    trRegistryRefresh();                                    //refresh registry - load in all textures
    if (hrAbortLoadingGame)
    {
        goto abortloading;
    }
    etgFixupUV();
    utySet(SSA_RegistryRefreshed);

    for (;;)
    {
        if (hrAbortLoadingGame)
        {
            break;
        }

        if (HorseRaceNext(1.0f))
        {
            break;
        }
    }

abortloading:

    if (hrAbortLoadingGame)
    {                                                       //if loading was aborted
        trSetAllPending(TRUE);
    }

//    rmRemoveAllUnneededTech();

    bobInitProperties();

    if (!hrAbortLoadingGame)
        rndSetClearColor(universe.backgroundColor|0xff000000);

    /* restore sound engine */
    soundEventPause(FALSE);

    if(tutorial==TUTORIAL_ONLY)
    {
        soundEventPlayMusic(SongNumber);
    }

    gameIsRunning = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gameEnd
    Description : ends the game
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gameEnd(void)
{
    sdword index;

    if (RGLtype != SWtype)
    {
        (void)glcActivate(TRUE);
    }

    //reset the colors schemes to prevent re-entrant bugs
    teReset();

    //clear out any persistent subtitles
    subReset();

    if (singlePlayerGame)
        tutRemoveAllPointers();

    if (multiPlayerGame)
    {
        SendInGameQuittingPacket(sigsPlayerIndex);
        TransferCaptaincyGameEnded();
        if (TitanActive)
        {
            titanPumpEngine();
        }
        Sleep(100);     // Give titan some time to handle this

        if (TitanActive)
        {
            titanGameEnded();
        }
        else
        {
            // later add other non-titan game ended callbacks
        }
    }

    // reset player alliance information
    for (index=0;index<MAX_MULTIPLAYER_PLAYERS;index++)
    {
        universe.players[index].Allies = 0;
        universe.players[index].AllianceProposals = 0;
        universe.players[index].AllianceProposalsTimeout = 0;
    }

    bobResetProperties();

    if (utyTest2(SS2_GameChat))
    {
        gcShutdown();
        utyClear2(SS2_GameChat);
    }

    writeGameStatsToFile(strGetString(strStatsGameEndStats));     //write game stats to a file for perusal

    universeClosePlayers();

    rmResetStaticInfo();

    hsStaticReset();

    if (singlePlayerGame)
    {
        singlePlayerClose();
    }

    if (utyTest2(SS2_GamePicker))
    {
        gpReset();
    }

    tacticsShutDown();

//    soundEventReset();

    cloudReset();

    nebReset();

    btgReset();

    alodReset();

    universeReset();

    if (utyTest2(SS2_AIPlayer))
    {
        aiplayerShutdown();
        utyClear2(SS2_AIPlayer);
    }

    netcheckClose();

#ifdef GOD_LIKE_SYNC_CHECKING
    if(syncDumpOn)
    {
        godSyncShutDown();
    }
#endif

    if (playPackets)
    {
        recPackPlayClose();
    }

    if(tutorial==1)
        tutUnInitialize();

    pingAllPingsDelete();

    universeStaticClose();

    universeSetZeroPlayers();

    selReset();

    ioUpdateShipTotals();

    globalsReset();
    if (utyTest2(SS2_NetworkQueues))
    {
        autodownloadmapReset();
        KeepAliveReset();
        ResetQueue(&ProcessCmdPktQ);
        ResetQueue(&ProcessSyncPktQ);
        ResetQueue(&ProcessRequestedSyncPktQ);
        ResetQueue(&ProcessCaptaincyPktQ);
        ResetLastSyncPktsQ();
    }

    rndSetClearColor(colBlack);

    trTextureDeleteAllUnregistered();
    mrReset();                                              //reset the state of the main viewport region

    if (utyTest(SSA_TaskBar))
    {
        tbShutdown();                                            //shut down the task bar
        utyClear(SSA_TaskBar);
    }

    //reset the scissor bars and fade out maintained by NIS's
    nisScissorBarsReset();

    //make sure the mouse is not erroneously clipped to anything
    mouseClipToRect(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : utyMultiplayerGameStart
    Description : Starts up the screen for multplayer game.
    Inputs      : name
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void utyMultiplayerGameStart(char *name, featom *atom)
{
    mgStartMultiPlayerGameScreens(ghMainRegion,0,0,0, FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : utyShowCredits
    Description : Starts the Credits a showin!!
    Inputs      : atom name and atom
    Outputs     : Why Credits of course
    Return      : void
----------------------------------------------------------------------------*/
void utyShowCredits(char *name, featom *atom)
{
    soundEventPlayMusic(RollCredits);
    psModeBegin("Credits\\", PMF_Credits | PMF_LanguageSpecific | PMF_MusicTrack);
    psScreenStart("Credits-button.cred");
}

/*-----------------------------------------------------------------------------
    Name        : utyNewGameRace
    Description : Race select callback
    Inputs      : name - name of entry to envoke this function
    Outputs     : modifies whichRaceSelected (== R1 or R2)
    Return      :
----------------------------------------------------------------------------*/
#if 0
void utyNewGameRace(char *name, featom *atom)
{
    udword raceOne, raceTwo;
    ShipRace oldRace = whichRaceSelected;
    tecolorscheme tempScheme;

    if (!FEFIRSTCALL(atom))
    {
        whichRaceSelected = (!strcmp(name, "CB_NewGameRaceOne")) ? R1 : R2;
        if (whichRaceSelected != oldRace && (!cpColorsPicked))
        {                                                   //if new race selected and no colors picked
            swap(teColorSchemes[0], teColorSchemes[1], tempScheme);//swap default R1/R2 color schemes
            utyBaseColor = teColorSchemes[0].textureColor.base;
            utyStripeColor = teColorSchemes[0].textureColor.detail;
            cpColorsReset(&utyBaseColor, &utyStripeColor);
            //cpColorsReset(&teColorSchemes[0].textureColor.base, &teColorSchemes[0].textureColor.detail);
        }
    }
    // ensure non-selected button Off, selected On
    raceOne = (whichRaceSelected == R1) ? TRUE : FALSE;
    raceTwo = (whichRaceSelected == R2) ? TRUE : FALSE;
/*    if (!FEFIRSTCALL(atom))
    {
        // ... or we'll be toggled elsewhere again
        if (raceOne) raceOne = FALSE;
        if (raceTwo) raceTwo = FALSE;
    }*/
    feToggleButtonSet("CB_NewGameRaceOne", raceOne);
    feToggleButtonSet("CB_NewGameRaceTwo", raceTwo);
    cpPreviewImageDelete();
}
#endif

/*-----------------------------------------------------------------------------
    This is a table of state information that will be saved/restored surrounding
    demo playback.  This list is designed to be expandable and backward-compatable
    as long as new entries are added at the end of the list and the order is not
    changed.
-----------------------------------------------------------------------------*/

#define DST_Dword               0
#define DST_Bool                1
#define DST_Real32              2
#define DST_Byte                3
#define DST_Bool8               4
#define DST_StringOfSize        BIT8
#define DST_StructOfSize        BIT16
struct
{
    udword type;
    void *data;
}
utyDemoStateTable[] =
{
    {DST_Bool, &smInstantTransition},
    {DST_Real32, &taskTimeElapsed},
    {DST_Bool, &determCompPlayer},
    {DST_Bool, &noDefaultComputerPlayer},
    {DST_Real32, &CAMERA_MOUSE_SENS},
    {DST_Dword, &batGlobalFrequencyModifier},
    {0, NULL},
};

/*-----------------------------------------------------------------------------
    Name        : utyPreDemoStateSaveCB
    Description : A demo recording callback, this function saves the state of
                    various user options before the demo starts recording.
    Inputs      :
    Outputs     : buffer - pointer allocated by memAlloc() where the state info
                    is stored.  This will be freed by the caller of this function.
                  length - length of aforementioned buffer.
    Return      :
----------------------------------------------------------------------------*/
#define BUFFER_GRANULARITY      256
void utyPreDemoStateSaveCB(ubyte **buffer, sdword *length)
{
    ubyte *stateBuffer, *newPointer;
    sdword bufferLength = BUFFER_GRANULARITY, bufferPos, index, newSize, newType;

    stateBuffer = memAlloc(BUFFER_GRANULARITY, "PreDemoState", Pyrophoric);

    for (index = bufferPos = 0; utyDemoStateTable[index].data != NULL; index++)
    {
        newPointer = utyDemoStateTable[index].data;         //default pointer
        newType = utyDemoStateTable[index].type;            //default type
        if (utyDemoStateTable[index].type >= DST_StructOfSize)
        {                                                   //save a structure of a given size
            newSize = utyDemoStateTable[index].type - DST_StructOfSize;
        }
        else
        {
            switch (utyDemoStateTable[index].type)
            {                                               //save default types of know sizes
                case DST_Dword:
                case DST_Bool:
                case DST_Real32:
                    newSize = sizeof(udword);
                    break;
                case DST_Byte:
                case DST_Bool8:
                    newSize = sizeof(ubyte);
                    break;
                case DST_StringOfSize:
                    newPointer = *((ubyte **)utyDemoStateTable[index].data);
                    newSize = strlen((char *)newPointer) + 1;
                    newType = DST_StringOfSize + newSize;
                    break;
                default:
                    dbgAssert(FALSE);
            }
        }
        if (bufferPos + newSize + sizeof(udword) >= bufferLength)
        {                                                   //we may bave to grow out the buffer
            bufferLength = max(bufferLength + BUFFER_GRANULARITY, bufferPos + newSize + sizeof(udword));
            stateBuffer = memRealloc(stateBuffer, bufferLength, "REDemoState", Pyrophoric);
        }
        *((sdword *)(stateBuffer + bufferPos)) = newType;
        bufferPos += sizeof(udword);
        memcpy(stateBuffer + bufferPos, newPointer, newSize);//copy the data of given type
        bufferPos += newSize;
    }

    *buffer = stateBuffer;
    *length = bufferPos;
}

/*-----------------------------------------------------------------------------
    Name        : utyPreDemoStateLoad
    Description : Load a bunch of options as defined in utyDemoStateTable from
                    a buffer.
    Inputs      : stateBuffer - buffer to load options fron
                  stateSize - length of that buffer.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyPreDemoStateLoad(ubyte *stateBuffer, sdword stateSize)
{
    sdword index, statePos, type, sizeToRead;
    ubyte *readTo;

    for (index = statePos = 0; utyDemoStateTable[index].data != NULL && statePos < stateSize; index++)
    {                                                       //scan through items in table
        type = *((sdword *)(stateBuffer + statePos));       //read in the type dword
        statePos += sizeof(sdword);
        readTo = utyDemoStateTable[index].data;             //default read address
        if (type > DST_StructOfSize)
        {                                                   //it's a structure; figure a size
            sizeToRead = type - DST_StructOfSize;
        }
        else if (type > DST_StringOfSize)
        {                                                   //it's a string; figure a size
            sizeToRead = type - DST_StringOfSize;
            readTo = *((ubyte **)readTo);
        }
        else
        {                                                   //default types (dwords and bytes)
            switch (type)
            {
                case DST_Dword:
                case DST_Bool:
                case DST_Real32:
                    sizeToRead = sizeof(udword);
                    break;
                case DST_Byte:
                case DST_Bool8:
                    sizeToRead = sizeof(ubyte);
                    break;
            }
        }
        memcpy(readTo, stateBuffer + statePos, sizeToRead); //copy the data to where it should go
        statePos += sizeToRead;                             //increment the buffer index
    }
}

/*-----------------------------------------------------------------------------
    Name        : utyPreDemoStateLoadCB
    Description : A demo recording callback to load in a user-defined state.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
//these variables store the backup of options saved for demo playback
ubyte *utyDemoOptionsSaved = NULL;
sdword utyDemoOptionsLength = 0;
//position of mouse when the baby callback was created
sdword utyDemoWaitMouseX;
sdword utyDemoWaitMouseY;
void utyPreDemoStateLoadCB(ubyte *stateBuffer, sdword stateSize)
{
    utyPreDemoStateSaveCB(&utyDemoOptionsSaved, &utyDemoOptionsLength);//put options somewhere to be restored later
    utyPreDemoStateLoad(stateBuffer, stateSize);            //now load the options from disk
}

/*-----------------------------------------------------------------------------
    Name        : utyDemoFinishedCB
    Description : A demo callback, this function performs cleanup at the end of a demo.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyDemoFinishedCB(void)
{
    utyPreDemoStateLoad(utyDemoOptionsSaved, utyDemoOptionsLength);
    memFree(utyDemoOptionsSaved);
    utyDemoOptionsSaved = NULL;
    utyDemoOptionsLength = 0;
#if DEM_INTERRUPTABLE
    if (!demPlaybackInterrupted)
#endif
    {                                                       //if demo ended normally, i.e. was not interrupted
        if (gameIsRunning)
        {
            //... if game is running, end the game
            utyGameQuitToMain(NULL, NULL);
        }
        else
        {
            //... if not at main screen, go to main screen
            feAllScreensDelete();
            feScreenStart(ghMainRegion, "Main_game_screen");
        }
    }
}

#if DEM_AUTO_DEMO
/*-----------------------------------------------------------------------------
    Name        : utyDemoAutoPlay
    Description : Baby timer callback to start a demo after chilling at the
                    startup screen for a while.
    Inputs      :
    Outputs     :
    Return      : TRUE - don't try this again, dag nabbit!
                  FALSE - we'd like to try this again later.
----------------------------------------------------------------------------*/
bool utyDemoAutoPlay(udword num, void* data, struct BabyCallBack* baby)
{
    sdword index, attempt, chosen;
    char *names[DEM_NumberNames];
    char string[256];
    filehandle handle;

    if (demDemoRecording || demDemoPlaying || (!demAutoDemo))
    {                                                       //don't play any demos if we're already playing one
        return(TRUE);
    }
#if DEM_VERBOSE_LEVEL >= 1
    dbgMessage("\nAttempting to play a demo...");
#endif
    if (mouseCursorX() != utyDemoWaitMouseX || mouseCursorY() != utyDemoWaitMouseY || taskTimeElapsed - keyLastTimeHit < demAutoDemoWaitTime)
    {
        utyDemoWaitMouseX = mouseCursorX();                 //re-save the mouse position
        utyDemoWaitMouseY = mouseCursorY();
        return(FALSE);                                      //the user was moving the mouse or hitting keys.  Let's try this again later.
    }
    if (feStackIndex >= 0 && feStack[feStackIndex].screen != NULL)
    {
        if (feAtomFindInScreen(feStack[feStackIndex].screen, "SinglePlayerOptions"))
        {                                                   //if we really are on the main screen when this sucker elapses
            handle = fileOpen("demos\\AutoDemos.list", FF_TextMode);//open the demo listing file
            for (index = 0; index < DEM_NumberNames; index++)
            {                                               //read in a bunch of demo file names
                if (fileLineRead(handle, string, 256) == FR_EndOfFile)
                {
                    break;
                }
                names[index] = memStringDupe(string);
            }
            fileClose(handle);                              //close the file

            for (attempt = 0; attempt < DEM_NumberTries; attempt++)
            {
                chosen = ranRandom(RAN_Sound) % index;      //choose a demo to play (sound random stream is non-deterministic)
                sprintf(string, "%s%dx%d.dem", names[chosen],//get the resolution-dependent name
                        MAIN_WindowWidth, MAIN_WindowHeight);

#if DEM_VERBOSE_LEVEL >= 1
                dbgMessagef("\nTrying to play '%s'...", string);
#endif
                if (fileExists(string, 0))
                {                                           //see if this demo exists in this resolution
#if DEM_VERBOSE_LEVEL >= 1
                    dbgMessagef("playing...", string);
#endif
                    demDemoPlaying = TRUE;
                    demPlayStart(string, utyPreDemoStateLoadCB, utyDemoFinishedCB);
                    break;
                }
#if DEM_VERBOSE_LEVEL >= 1
                else
                {
                    dbgMessagef("Doesn't exist.  Try again.", string);
                }
#endif
            }
            for (index--; index >= 0; index--)
            {                                               //free all the name strings
                memFree(names[index]);
            }

        }
    }
    return(TRUE);                                           //don't call me here anymore
}
#endif //DEM_AUTO_DEMO

/*-----------------------------------------------------------------------------
    Name        : utySinglePlayerOptions
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utySinglePlayerOptions(char *name, featom *atom)
{

#if defined(Downloadable)

    featom *bitchatom;

    bitchatom = feAtomFindInScreen(feScreenFind("Main_game_screen"),"FE_SHOWCREDITS");

    bitSet(bitchatom->flags, FAF_Disabled);
    bitSet(((region *)bitchatom->region)->status, RSF_RegionDisabled);


#endif

#if defined(DLPublicBeta)
    //disable this function in demos
    bitSet(atom->flags, FAF_Disabled);
    bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
#if DEM_AUTO_DEMO
    if (FEFIRSTCALL(atom))
    {
        //prepare to auto-play a demo after a period of time
        utyDemoWaitMouseX = mouseCursorX();
        utyDemoWaitMouseY = mouseCursorY();
        taskCallBackRegister(utyDemoAutoPlay, 0, NULL, demAutoDemoWaitTime);
    }
#endif //DEM_AUTO_DEMO
#else //defined(DLPublicBeta)
    if (FEFIRSTCALL(atom))
    {
#if DEM_AUTO_DEMO
        //prepare to auto-play a demo after a period of time
        utyDemoWaitMouseX = mouseCursorX();
        utyDemoWaitMouseY = mouseCursorY();
        taskCallBackRegister(utyDemoAutoPlay, 0, NULL, demAutoDemoWaitTime);
#endif //DEM_AUTO_DEMO
        return;
    }
#if MAIN_Password
    if (mainSinglePlayerEnabled)
#endif
    {
        feScreenDisappear(NULL,NULL);

        feScreenStart(ghMainRegion, "Create_new_game");
    }
#if MAIN_Password
    else
    {
        bitSet(atom->flags, FAF_Disabled);
    }
#endif //MAIN_Password
#endif // defined(CGW) || defined(Downloadable) || defined(DLPublicBeta)
}

/*-----------------------------------------------------------------------------
    Name        : utySinglePlayerGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utySinglePlayerGame(char *name, featom *atom)
{
    feScreenDisappear(NULL,NULL);

    feScreenStart(ghMainRegion, "Create_new_game");
}

/*-----------------------------------------------------------------------------
    Name        : utySelectSinglePlayerGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utySelectSinglePlayerGame(char *name, featom *atom)
{
    gpGamePick("S");
}

void utyLoadSinglePlayerGameGivenFilename(char *filename)
{
    cpResetRegions();

    dbgMessagef("\nNew single player game being loaded");
    feAllScreensDelete();
/*
    if (nisEnabled && utyTeaserPlaying != NULL)
    {
        mouseCursorShow();
        utyTeaserEnd();
    }
*/
    singlePlayerGame = TRUE;
    //objectivesShown = TRUE;

    if (!(tutorial==TUTORIAL_ONLY))
    {
        tutorial = TUTORIAL_SINGLEPLAYER;
    }

    singlePlayerInit();

    horseRaceInit();
    gameStart(filename);
    horseRaceShutdown();

    if (hrAbortLoadingGame)
    {
        utyGameQuitToMain(NULL,NULL);
        hrAbortLoadingGame = FALSE;
    }
    else
        singlePlayerPostInit(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : utyLoadSinglePlayerGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyLoadSinglePlayerGame(char *name, featom *atom)
{
    char filename[100];
    sdword verifysavename;

    if (!gpGetGameName(name,atom,filename))
    {
        return;
    }

    if ((verifysavename = VerifySaveFile(filename)) != VERIFYSAVEFILE_OK)
    {
        if (verifysavename == VERIFYSAVEFILE_BADVERSION)
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFileVersion),NULL);
        else
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFile),NULL);
        return;
    }

    utyLoadSinglePlayerGameGivenFilename(filename);
}

/*-----------------------------------------------------------------------------
    Name        : utyLoadSinglePlayerGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyLoadMultiSinglePlayerGame(char *name, featom *atom)
{
    if (gpLoadSinglePlayerGame)
    {
        utyLoadSinglePlayerGame(name,atom);
    }
    else
    {
        utyLoadMultiPlayerGame(name,atom);
    }
}

/*-----------------------------------------------------------------------------
    Name        : utySelectMultiPlayerGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utySelectMultiPlayerGame(char *name, featom *atom)
{
    gpGamePick("M");
}

void utyLoadMultiPlayerGameGivenFilename(char *filename)
{
    dbgAssert(startingGame == FALSE);

    dbgMessagef("\nNew game started");
    startingGame = TRUE;

    if (mgRunning)
    {
        mgShutdownMultiPlayerGameScreens();
    }

    feAllScreensDelete();

    dbgAssert(!multiPlayerGame);        // don't allow loading of network games

    singlePlayerInit();

    horseRaceInit();
    gameStart(filename);
    horseRaceShutdown();

    if (hrAbortLoadingGame)
    {
        utyGameQuitToMain(NULL,NULL);
        hrAbortLoadingGame = FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : utyLoadMultiPlayerGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyLoadMultiPlayerGame(char *name, featom *atom)
{
    char filename[100];
    sdword verifysavename;

    if (!gpGetGameName(name,atom,filename))
    {
        return;
    }

    if ((verifysavename = VerifySaveFile(filename)) != VERIFYSAVEFILE_OK)
    {
        if (verifysavename == VERIFYSAVEFILE_BADVERSION)
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFileVersion),NULL);
        else
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFile),NULL);
        return;
    }

    utyLoadMultiPlayerGameGivenFilename(filename);
}

/*-----------------------------------------------------------------------------
    Name        : utyBackFromSinglePlayerOptions
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyBackFromSinglePlayerOptions(char *name, featom *atom)
{
    cpResetRegions();

    feScreenDisappear(NULL,NULL);

    feScreenStart(ghMainRegion, "Main_game_screen");
}

/*-----------------------------------------------------------------------------
    Name        : utyStartTutorial
    Description : called from the Tutorial Alert popup
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyStartTutorial(char* name, featom* atom)
{
    forceSP = TRUE;
    tutorial = 1;
    utySinglePlayerGameStart(name, atom);
}

/*-----------------------------------------------------------------------------
    Name        : utyStartSinglePlayer
    Description : called from the Tutorial Alert popup
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyStartSinglePlayer(char* name, featom* atom)
{
    forceSP = TRUE;
    needtutorial = FALSE;
    utySinglePlayerGameStart(name, atom);
}

/*-----------------------------------------------------------------------------
    Name        : utyStartSkirmish
    Description : called from the Training Alert popup
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyStartSkirmish(char* name, featom* atom)
{
    forceSP = TRUE;
    needtutorial = FALSE;
    utyNewGameStart(name, atom);
}

/*-----------------------------------------------------------------------------
    Name        : utySinglePlayerGameStart
    Description : Starts a new single player game
    Inputs      : name - name of entry to envoke this function
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utySinglePlayerGameStart(char *name, featom *atom)
{
    cpResetRegions();

    if (_stricmp(name, "Tutorial1") == 0)
    {
        forceSP = TRUE;
    }

    if ((!forceSP) &&
        (gpCountTrainingSavegames() == 0) &&
        (needtutorial))
    {
        feScreenStart(ghMainRegion, "Tutorial_Alert");
        return;
    }

    dbgMessagef("\nNew single player game started");
    feAllScreensDelete();

    forceSP = FALSE;
/*
    if (nisEnabled && utyTeaserPlaying != NULL)
    {
        mouseCursorShow();
        utyTeaserEnd();
    }
*/
    singlePlayerGame = TRUE;
    //objectivesShown = TRUE;

    if (!(tutorial==TUTORIAL_ONLY))
    {
        tutorial = TUTORIAL_SINGLEPLAYER;
    }


    numPlayers = 2;     // player 1 is always computer (pirates, opposite race, whatever)
    curPlayer = 0;      // player 0 is human
    strcpy(playerNames[0],"Player");

    singlePlayerInit();

    horseRaceInit();
    gameStart(NULL);
    horseRaceShutdown();

    if (hrAbortLoadingGame)
    {
        utyGameQuitToMain(NULL,NULL);
        hrAbortLoadingGame = FALSE;
    }
    else
    {
        singlePlayerPostInit(FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : utyBackFromSinglePlayer
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyBackFromSinglePlayer(char *name, featom *atom)
{
    cpResetRegions();

    feScreenDisappear(NULL,NULL);

    feScreenStart(ghMainRegion, "Single_player_options");
}

/*-----------------------------------------------------------------------------
    Name        : utyNewGameStart
    Description : Starts new game
    Inputs      : name - name of entry to envoke this function
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyNewGameStart(char *name, featom *atom)
{
    udword i,j;

#if defined(DLPublicBeta)
    //disable this function in demos
    if (atom != NULL)
    {
        bitSet(atom->flags, FAF_Disabled);
        bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
    }
    if (!multiPlayerGame)
    {
        dbgFatal(DBG_Loc, "CRC (Cyclic Redundancy Check)");
    }
#endif
    if (FEFIRSTCALL(atom))
    {
        return;
    }
    if ((!forceSP) &&
        (!multiPlayerGame) &&
        (gpCountTrainingSavegames() == 0) &&
        (needtutorial))
    {
        feScreenStart(ghMainRegion, "Tutorial_Skirmish_Alert");
        return;
    }

    if (!multiPlayerGame)       // moved up so that check if minplayers too low can be made before feAllScreensDelete, etc
    {
        if ((!recordplayPacketsInGame) && (playPackets))
        {
            netcheckInit();
            recPackPlayInit();      // sets numPlayers, curPlayer
        }
        else if (!noDefaultComputerPlayer)
        {
            numPlayers = 1 + tpGameCreated.numComputers;
            for (i=1;i<tpGameCreated.numComputers+1;i++)
            {
                ComputerPlayerEnabled[i] = TRUE;
            }
            curPlayer = 0;
        }
        else
        {
            numPlayers = singleMachineNumPlayers;
            curPlayer = 0;
        }

        if ((!singlePlayerGame) && (spScenarios[spCurrentSelected].minplayers > (numPlayers)))
        {
            char str[200];

            sprintf(str,strGetString(strMinNumPlayersRequired),spScenarios[spCurrentSelected].minplayers);
            mgPrepareMessageBox(str,NULL);
            mgShowScreen(MGS_Message_Box,FALSE);
            return;
        }
    }

    forceSP = FALSE;

    dbgAssert(startingGame == FALSE);

    dbgMessagef("\nNew game started");
    startingGame = TRUE;

    if (mgRunning)
    {
        mgShutdownMultiPlayerGameScreens();
    }

    feAllScreensDelete();
    if (multiPlayerGame)
    {
        for (i=0;i<TOTAL_NUM_SHIPS;i++)
        {
            shiplagtotals[i] = 0;
        }

        dbgMessagef("\nsigsPlayerIndex %d",sigsPlayerIndex);
        for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
        {
            playersReadyToGo[i] = FALSE;
        }
        numPlayers = (uword)sigsNumPlayers;
        curPlayer = (uword)sigsPlayerIndex;
        netcheckInit();

#ifdef GOD_LIKE_SYNC_CHECKING
        if(syncDumpOn)
        {
            godSyncInit();
        }
#endif

        if (!recordplayPacketsInGame)
        {
            if (playPackets)
            {
                recPackPlayInit();      // sets numPlayers, curPlayer
            }
            else if (recordPackets)
            {
                recPackRecordInit();
            }
        }

//        if (!noDefaultComputerPlayer)
//        {
            if (tpGameCreated.numComputers>0)
            {
                // ditribute the computer players in a round robin fashion
                j=0;
                for (i=numPlayers; i<numPlayers + tpGameCreated.numComputers; i++)
                {
                    if (j==sigsPlayerIndex)
                    {
                        ComputerPlayerEnabled[i] = TRUE;
                    }
                    ComputerPlayerOn[i] = j;
                    j++;
                    if (j > numPlayers-1)
                    {
                        j = 0;
                    }
                }

                numPlayers += tpGameCreated.numComputers;
            }
  //      }

        singlePlayerInit();

        startingGameState = CHECK_AUTODOWNLOAD_MAP;
        if (numPlayers == 1)
        {
            multiPlayerGameUnderWay = TRUE;
        }
    }
    else
    {
        for (i=0;i<numPlayers;i++)
        {
            if (ComputerPlayerEnabled[i])
            {
                sprintf(playerNames[i], "%s %i", strGetString(strComputerName), i);
            }
            else
            {
                strcpy(playerNames[i],utyName);
            }
        }

        singlePlayerInit();

        horseRaceInit();
        gameStart(NULL);
        horseRaceShutdown();

        if (hrAbortLoadingGame)
        {
            utyGameQuitToMain(NULL,NULL);
            hrAbortLoadingGame = FALSE;
        }
    }
#ifdef DEBUG_GAME_STATS
        if(statLogOn)
        {
            if(multiPlayerGame)
            {
                gameStatsInitLog();
            }
        }
#endif

}

/*-----------------------------------------------------------------------------
    Name        : utyGameQuit
    Description : Quit game
    Inputs      : name - name of entry to envoke this function
    Outputs     : Calls utyCloseOK
    Return      : void
----------------------------------------------------------------------------*/
void utyGameQuit(char *name, featom *atom)
{
    dbgMessagef("\nQuit game, baby!");
#if defined(CGW) || defined(Downloadable) || defined(DLPublicBeta) || defined(OEM)
    if (enableAVI)
    {
        psModeBegin("Plugscreens\\", PMF_CanSkip);
        psScreenStart("BuyHomeworld0.plug");
    }
    else
    {
        utyCloseOK(NULL, 0, 0, 0);
    }
#else
        utyCloseOK(NULL, 0, 0, 0);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : utyGameQuitToMain
    Description : Quit game
    Inputs      : name - name of entry to envoke this function
    Outputs     : Calls utyCloseOK
    Return      : void
----------------------------------------------------------------------------*/
void utyGameQuitToMain(char *name, featom *atom)
{
    bool networkgame=multiPlayerGame;
    dbgMessagef("\nQuit to main menu!");

    feAllScreensDelete();

    soundEventStopMusic(1.0f);
    soundEventPause(TRUE);

    gameEnd();

#if defined(CGW) || defined (Downloadable) || defined(DLPublicBeta) || defined(OEM)
    if (utyPlugScreens && enableAVI)
    {
        psModeBegin("Plugscreens\\", PMF_CanSkip);
        psScreenStart("BuyHomeworld0.plug");
        soundEventPlayMusic(SOUND_FRONTEND_TRACK);
        utyPlugScreens = FALSE;
        return;
    }
    else
    {
        soundEventPlayMusic(SOUND_FRONTEND_TRACK);
    }
#else
    if (utyCreditsSequence)
    {
        soundEventPlayMusic(RollCredits);
        psModeBegin("Credits\\", PMF_Credits | PMF_LanguageSpecific | PMF_MusicTrack);
        psScreenStart("Credits-GameEnd.cred");
        utyCreditsSequence = FALSE;
    }
    else
    {
        soundEventPlayMusic(SOUND_FRONTEND_TRACK);
    }
#endif
    if (networkgame)
    {
        if (LANGame)
        {
            lgStartMultiPlayerLANGameScreens(ghMainRegion, 0, 0, 0, TRUE);
        }
        else
        {
            mgStartMultiPlayerGameScreens(ghMainRegion, 0, 0, 0, TRUE);
        }
    }
    else
    {
        feScreenStart(ghMainRegion, "Main_game_screen");
    }
    soundEventPause(FALSE);
    soundEventUpdate();
}

/*-----------------------------------------------------------------------------
    Name        : utyGameQuitToPlugScreens
    Description : Quits the current game and exits to the plug screens (demo versions only)
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
/*
#if defined(CGW) || defined (Downloadable) || defined(DLPublicBeta) || defined(OEM)
void utyGameQuitToPlugScreens(void)
{
    feAllScreensDelete();
    gameEnd();
    psModeBegin("Plugscreens\\", PMF_CanSkip);
    psScreenStart("BuyHomeworld0.plug");
}
#endif
*/

/*-----------------------------------------------------------------------------
    Name        : utyInGameCancel
    Description : call back function for when in-game menu is deleted
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyInGameCancel(char *name, featom *atom)
{
    if (!multiPlayerGame)
    {
        universePause = FALSE;      // unpause game
    }

    feScreenDisappear(name,atom);
}

/*-----------------------------------------------------------------------------
    Name        : utyInGameLoad
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyInGameLoad(char *name, featom *atom)
{
/* This is where we actually load the game */
}

/*-----------------------------------------------------------------------------
    Name        : utyInGameSave
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyInGameSave(char *name, featom *atom)
{
/* This is where we actually save the game */
}



/*-----------------------------------------------------------------------------
    Name        : utyInGameSave
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utySaveRecordedGame(char *name, featom *atom)
{
/* This is where we start the recording */
}



/*-----------------------------------------------------------------------------
    Name        : utyFrontEndDataLoad
    Description : Load data required for front end.  Other modules can load
                    their own front-end data.
    Inputs      : void
    Outputs     : Does some stuff.  Or something.
    Return      : void
----------------------------------------------------------------------------*/
void utyFrontEndDataLoad(void)
{
    sdword index;

    ghDefaultFont = frFontRegister("default.hff");          //load in the default font
    fontMakeCurrent(ghDefaultFont);                         //make default font current

    for (index = 0; utyFrontEndFiles[index] != NULL; index++)
    {
        utyFrontEndFiles[index] = (char *)feScreensLoad(utyFrontEndFiles[index]);
    }
    feCallbackAddMultiple(utyCallbacks);
    feDrawCallbackAddMultiple(utyDrawCallbacks);

    opOptionsInit();
}

/*-----------------------------------------------------------------------------
    Name        : utyFrontEndDataFree
    Description : Free data loaded for front-end screens.
    Inputs      : void
    Outputs     : Does some stuff.  Or something.
    Return      : void
----------------------------------------------------------------------------*/
void utyFrontEndDataFree(void)
{
    sdword index;

    for (index = 0; utyFrontEndFiles[index] != NULL; index++)
    {                                                       //for each screen
        feScreensDelete((fibfileheader *)utyFrontEndFiles[index]);//free it
    }
}

/*-----------------------------------------------------------------------------
    Name        : utyTeaserStart
    Description : Start the teaser NIS to play in the background of the menu
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
//file names for Non-Interactive NIS Sequences
/*
#define NIS_TeaserName          "nis\\Teaser.nis", "nis\\Teaser.script"
#define NIS_Radius      10000.0f
void utyTeaserStart(void)
{
    sdword index;
    ShipStaticInfo *info;
    vector centre = {0.0f, 0.0f, 0.0f};
    Player *player;
    ShipType shiptype;
    ShipRace shiprace;
    sdword i;
    matrix thisIdentityMatrix;

    universe.numPlayers = numPlayers = 2;
    for (i=0;i<universe.numPlayers;i++)
    {
        player = &universe.players[i];

        player->PlayerMothership = NULL;
        player->resourceUnits = 0;
        player->playerIndex = 0;
        player->race = i;
    }

    universeDefaultTeamColors();                            //set default team colors for all ships

    universe.curPlayerPtr = &universe.players[0];

    utyTeaserHeader = nisLoad(NIS_TeaserName);
    //utyTeaserHeader->loop = 0.0f;                           //make NIS loop

    for (index = 0; index < utyTeaserHeader->nObjectPaths; index++)
    {                                                       //load in the ship statics needed for NIS
        shiptype = utyTeaserHeader->objectPath[index].type;
        shiprace = utyTeaserHeader->objectPath[index].race;
        info = GetShipStaticInfo(shiptype,shiprace);
        bitSet(info->staticheader.infoFlags, IF_InfoNeeded);
        if (shiptype == DDDFrigate)
        {
            info = GetShipStaticInfo(Drone, shiprace);
            bitSet(info->staticheader.infoFlags, IF_InfoNeeded);
        }
    }
    universeStaticInit();
    if (etgHasBeenStarted)
    {                                                       //if effects already started
        etgReloadReset();
    }
    else
    {
        etgStartup();
        utySet(SSA_Effect);
        etgHasBeenStarted = TRUE;
    }

    trRegistryRefresh();                                    //load textures for NIS
    thisIdentityMatrix = IdentityMatrix;
    utyTeaserPlaying = nisStart(utyTeaserHeader, &centre, &thisIdentityMatrix);

    nisFullyScissored = TRUE;
//    soundEventPlayCD(1);
    soundEventPlayMusic(11);
    nisTaskResume();
}
*/
/*-----------------------------------------------------------------------------
    Name        : utyTeaserEnd
    Description : Stop the teaser from playing
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
void utyTeaserEnd(void)
{
    nisStop(utyTeaserPlaying);
    nisDelete(utyTeaserHeader);
    utyTeaserPlaying = NULL;
    nisTaskPause();
    nisCamera = NULL;
    nisIsRunning = nisCaptureCamera = FALSE;
    univupdateCloseAllObjectsAndMissionSpheres();
//    soundEventStopCD();
    soundEventStopMusic(1.0f);
    trTextureDeleteAllUnregistered();
    utyTeaserHeader = NULL;
    nisFullyScissored = FALSE;
}
*/

void scriptSetHomeworldCRC(char *directory,char *field,void *dataToFillIn)
{
    if (onlygetfirstcrc)
    {
        return;
    }
    HomeworldCRC[0] = 0;
    HomeworldCRC[1] = 0;
    HomeworldCRC[2] = 0;
    HomeworldCRC[3] = 0;
    sscanf(field,"%x %x %x %x",&HomeworldCRC[0],&HomeworldCRC[1],&HomeworldCRC[2],&HomeworldCRC[3]);
    onlygetfirstcrc = TRUE;
}

char *strBetweenWords(char *buffer,char *word1,char *word2)
{
    char *word1loc;
    char *word2loc;

    if ((!buffer) || (buffer[0] == 0)) return NULL;

    word1loc = strstr(buffer,word1);
    if (!word1loc) return NULL;

    for(;;)
    {
        if (*word1loc == 0)
            return NULL;

        if (*word1loc == ' ')
            break;

        word1loc++;
    }

    word1loc++;
    if (*word1loc == 0)
        return NULL;

    word2loc = strstr(buffer,word2);
    if (!word2loc)
    {
        return NULL;
    }

    word2loc--;

    if (word2loc < word1loc)
    {
        return NULL;
    }

    *word2loc = 0;
    return word1loc;
}

/*-----------------------------------------------------------------------------
    Name        : utyGetFirstCDPath
    Description : Gets path to first CD-ROM drive containing the HW CD ("D:\", say)
    Inputs      : N/A
    Outputs     : szPath (path to first CD-ROM drive continaning the HW CD - 4 chars)
    Return      : N/A
----------------------------------------------------------------------------*/
void utyGetFirstCDPath(char *szPath)
{
    char nLetter,szFile[255];

    HANDLE handle;

    /* Cycle thru drive letters */
    for(nLetter='A';nLetter<='Z';nLetter++)
    {
        /* Initialize path */
        strcpy(szPath," :");
        szPath[0]=nLetter;

        /* Check drive letter */
        if(GetDriveType(szPath) == DRIVE_CDROM)
        {
            /* Append \ */
            strcat(szPath,"\\");

            /* Set file to find */
            strcpy(szFile,szPath);
            strcat(szFile,utyMusicFilename);

            /* Find file */
            handle = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if(handle != INVALID_HANDLE_VALUE)
            {
                /* File found, exit */
                CloseHandle(handle);
                return;
            }

            /* File not found, continue */
        }
    }

    /* No CD-ROM available */
    strcpy(szPath,"");
    return;
}

/*-----------------------------------------------------------------------------
    Name        : PrepareVersionStringStuff
    Description : makes sure that networkVersion, languageVersion, minorBuildVersion and the combined versionString are all set
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PrepareVersionStringStuff()
{
    switch (strCurLanguage)
    {
        default:
        case languageEnglish:       strcpy(languageVersion,"English"); break;
        case languageFrench:        strcpy(languageVersion,"French"); break;
        case languageGerman:        strcpy(languageVersion,"German"); break;
        case languageSpanish:       strcpy(languageVersion,"Spanish"); break;
        case languageItalian:       strcpy(languageVersion,"Italian"); break;
        // can also put stuff like "EnglishOEM", etc.
    }

    strcpy(versionString,networkVersion);
    strcat(versionString,"\t");
    strcat(versionString,languageVersion);
    strcat(versionString,"\t");
    strcat(versionString,minorBuildVersion);
}

/*-----------------------------------------------------------------------------
    Name        : utyGrowthHeapAlloc
    Description : Memory manager callback to allocate a new growth heap.  The
                    pointer returned here will be passed to utyGrowthHeapFree
                    at shutdown time.
    Inputs      : size - size of heap to allocate
    Outputs     :
    Return      : pointer to newly allocated memory
----------------------------------------------------------------------------*/
void *utyGrowthHeapAlloc(sdword size)
{
    dbgAssert(size > 0);
    return((void *)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE));
}

/*-----------------------------------------------------------------------------
    Name        : utyGrowthHeapFree
    Description : Memory manager shutdown callback for freeing a growth heap.
    Inputs      : heap - heap to free
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void utyGrowthHeapFree(void *heap)
{
    BOOL result;
    result = VirtualFree(heap, 0, MEM_RELEASE);
    dbgAssert(result);
}

char *utyMissingCDMessages[] =
{
    "Invalid or missing Homeworld CD. Please insert valid CD.",
    "CD Homeworld non valide. Veuillez insérer un CD valide.",
    "Ungültige oder fehlende Homeworld-CD. Bitte gültige CD einlegen.",
    "El CD de Homeworld no es correcto o no se encuentra en la unidad. Introduce el CD correcto.",
    "CD di Homeworld mancante o non valido. Inserisci un CD valido.",
};
char *utyInvalidCDMessages[] =
{
    "Invalid Homeworld CD.",
    "CD Homeworld invalide.",
    "Ungültige Homeworld-CD.",
    "CD de Homeworld incorrecto.",
    "CD di Homeworld non valido.",
};
char *utyIncompatibleBigMessages[] =
{
    "Incompatible file: %s",
    "Fichier incompatible: %s",
    "Datei nicht kompatibel: %s",
    "Archivo incompatible: %s",
    "File incompatibile: %s",
};
char *utyCannotOpenBigMessages[] =
{
    "Unable to open file: %s",
    "Impossible d’ouvrir le fichier: %s",
    "Datei kann nicht geöffnet werden: %s",
    "Imposible abrir archivo: %s",
    "Impossibile aprire il file: %s",
};

/*-----------------------------------------------------------------------------
    Name        : utyGameSystemsPreInit
    Description : Starts up all miscellaneous game systems.  call occurs before the
                  main window (hWnd) is created
    Inputs      : void
    Outputs     :
    Return      : NULL on success, string pointing to error message on error
----------------------------------------------------------------------------*/
char* utyGameSystemsPreInit(void)
{
#ifndef HW_NOTOVERSIZE // oversized-CD code - variables
#ifndef OEM
#ifndef Downloadable
#ifdef HW_Release
    HANDLE syshandle;

    filehandle handle;

    char drive[256], volumename[256], filesysname[64], tmpfile[256], data[4];

    udword filesize, datasize, totalsize;
    udword serialnum = 0, componentlen, filesysflags;
    udword sectspclust, bytespsect, freeclusts, clusts, bytes;
#endif
#endif
#endif
#endif
    memset(utyStartedBits, 0, SSA_NumberBits);              //clear all module startup bits
    globalsInit();
    fmathInit();

    keyInit();

    // Initialize the font registry
    frStartup();
    utySet(SSA_FontReg);

    // Set file search path
    if (FilePathPrepended == FALSE)
    {
        if ((dataEnvironment = getenv("HW_Data")) != NULL)
        {
            filePrependPathSet(dataEnvironment);
            utySet(SSA_FilePrepend);
        }
    }

    // Attempt to set the CD-ROM path
    if(CDROMPathPrepended == FALSE)
    {
        char drivePath[4];

        // Find the first CD-ROM drive containing the HW CD
        utyGetFirstCDPath(drivePath);

        // If found, set the CD-ROM path
        if(strlen(drivePath) > 0) fileCDROMPathSet(drivePath);

/*
        sdword index;
        char drivePath[4] = "A:";
        for (index = 'A'; index <= 'Z'; index++)
        {
            drivePath[0] = index;
            if (GetDriveType(drivePath) == DRIVE_CDROM)
            {
                fileCDROMPathSet(drivePath);
                break;
            }
        }
*/
    }


#if MAIN_Password && MAIN_CDCheck
    if (mainCDCheckEnabled)
    {
        if (!fileExists(utyMusicFilename, FF_CDROM))
        {
            return("Homeworld CD not detected.");
        }
    }
#endif

#ifndef HW_NOTOVERSIZE // oversized-CD code - check
#ifndef OEM
#ifndef Downloadable
#ifdef HW_Release
    // check pad file
    if (!fileExists(utyPadBigFilename, FF_CDROM))
    // if (!fileExists(utyPadBigFilename, FF_IgnoreBIG))
    {
        return(utyMissingCDMessages[strCurLanguage]);
    }

    filesize = (udword)fileSizeGet(utyPadBigFilename, FF_CDROM);
    // filesize = (udword)fileSizeGet(utyPadBigFilename, FF_IgnoreBIG);

    handle = fileOpen(utyPadBigFilename, FF_CDROM);
    // handle = fileOpen(utyPadBigFilename, FF_IgnoreBIG);
    fileSeek(handle,filesize-sizeof(udword),FS_Start);
    fileBlockRead(handle, &datasize, sizeof(udword));
    fileClose(handle);

    // check pad file size
    if (filesize != datasize)
    {
        return(utyInvalidCDMessages[strCurLanguage]);
    }

    // get more info about CD
    strcpy(drive,filePathPrepend("",FF_CDROM));
    GetVolumeInformation(drive, volumename, 255, &serialnum, &componentlen, &filesysflags, filesysname, 63);

    // check for CDFS file system on CD
    if (_stricmp(filesysname, "CDFS") != 0)
    {
        return(utyInvalidCDMessages[strCurLanguage]);
    }

    // check for 0 bytes left on CD
    freeclusts = 4; // confuse them
    GetDiskFreeSpace(drive, &sectspclust, &bytespsect, &freeclusts, &clusts);
    if (freeclusts != 0)
    {
        return(utyInvalidCDMessages[strCurLanguage]);
    }

    // not only must the CD be > 670MB but it must also be < 685MB - the CD must not exceed 685MB as
    // manufacturing cuases problems with anything bigger. Anyway, it's good to check for an upper size
    // limit, in case someone has managed to fake all other aspects of a CD on a hard drive.
    totalsize  = (bytespsect * sectspclust) * clusts;
    if ((totalsize < 702545920) || (totalsize > 718274560))
    {
        totalsize = 738197504; // confuse them
        return(utyInvalidCDMessages[strCurLanguage]);
    }

    // attempt to create a temporary file on the CD
    sprintf(tmpfile,"%s~hw.tmp",drive);
    strcpy(data,"~hw");
    syshandle = CreateFile(tmpfile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (syshandle != INVALID_HANDLE_VALUE)
    {
        if (!WriteFile(syshandle, data, 4, &bytes, NULL))
        {
            if (bytes != 0)
            {
                CloseHandle(syshandle);
                DeleteFile(tmpfile);
                return(utyInvalidCDMessages[strCurLanguage]);
            }
        }

        CloseHandle(syshandle);
        DeleteFile(tmpfile);
        return(utyInvalidCDMessages[strCurLanguage]);
    }
#endif
#endif
#endif
#endif
    // check data dir on HD...
    if (!fileExists(utyMusicFilename, 0))
    {
        // check first CDROM drive...
        if (!fileExists(utyMusicFilename, FF_CDROM))
        {
            return(utyMissingCDMessages[strCurLanguage]);
        }
    }

    // check data dir on HD...
    if (!fileExists(utyVoiceFilename, 0))
    {
        // check first CDROM drive...
        if (!fileExists(utyVoiceFilename, FF_CDROM))
        {
            return(utyMissingCDMessages[strCurLanguage]);
        }
    }

    //start up the memory manager
    if (MemoryHeapSize == MEM_HeapSizeDefault)
    {                                                       //if the user has not specified a heap size
        sdword newSize;
        MEMORYSTATUS memStat;
        GlobalMemoryStatus(&memStat);

        newSize = (sdword)((real32)memStat.dwTotalPhys * MEM_HeapDefaultScalar);
        if (newSize > MEM_HeapSizeDefault)
        {
            MemoryHeapSize = min(newSize, MEM_HeapDefaultMax);
        }
    }
    utyMemoryHeap = (void *)VirtualAlloc(NULL, MemoryHeapSize + sizeof(memcookie) * 4, MEM_COMMIT, PAGE_READWRITE);
    if (utyMemoryHeap == NULL)
    {
        sprintf(errorString, "Error allocating heap of size %d", MemoryHeapSize);
        return(errorString);
    }
    utySet(SSA_MemoryHeap);
    if (memStartup(utyMemoryHeap, MemoryHeapSize, utyGrowthHeapAlloc) != OKAY)
    {
        sprintf(errorString, "Error starting memory manager with heap size %d at 0x%x", MemoryHeapSize, errorString);
        return(errorString);
    }
    utySet(SSA_MemoryModule);

    //
    //  initialize bigfile(s)
    //
    if (!bigOpen(utyBigFilename, utyUpdateBigFilename))
    {
        if (bigCheck(utyUpdateBigFilename) == -1)
        {
            sprintf(errorString, utyIncompatibleBigMessages[strCurLanguage], utyUpdateBigFilename);
            return errorString;
        }
        else if (bigCheck(utyBigFilename) == -1)
        {
            sprintf(errorString, utyIncompatibleBigMessages[strCurLanguage], utyBigFilename);
            return errorString;
        }
        else
        {
            sprintf(errorString, utyCannotOpenBigMessages[strCurLanguage], utyBigFilename);
            return errorString;
        }
    }
    if (!IgnoreBigfiles)
    {
        mainNewerAvailable = memAlloc(mainTOC.numFiles, "mainNewerAvailable", 0);
        memset(mainNewerAvailable, 0, mainTOC.numFiles);
        if (updateFP)
        {
            updateNewerAvailable = memAlloc(updateTOC.numFiles, "updateNewerAvailable", 0);
            memset(updateNewerAvailable, 0, updateTOC.numFiles);
        }
        if (CompareBigfiles)
        {
            if (!dataEnvironment || !dataEnvironment[0])
                // in absence of environment vars (like in a retail install), assume
                // data file structure will start alongside the EXE
                bigFilesystemCompare(".", "", &mainTOC, &updateTOC, mainNewerAvailable, updateNewerAvailable);
            else
                bigFilesystemCompare(dataEnvironment, "", &mainTOC, &updateTOC, mainNewerAvailable, updateNewerAvailable);
        }
    }

#if 0       // ShortCircuitWON done in titaninterface.cpp now
    if (ShortCircuitWON)
    {
        onlygetfirstcrc = FALSE;
        scriptSet(NULL,"WONstuff.txt",WonStuffSet);     // later, take this out before game is released, fix later
    }
    else
    {
#endif
        HomeworldCRC[0] = utyCodeCRCCompute();
        if (!IgnoreBigfiles)
        {
            bigCRC(&HomeworldCRC[1],&HomeworldCRC[2]);
        }
        else
        {
            HomeworldCRC[1] = 0;    HomeworldCRC[2] = 0;
        }
        HomeworldCRC[3] = 0;
//  }

    PrepareVersionStringStuff();

    if (SecretWON)
    {
        if (IgnoreBigfiles)
        {
            return("Big files must be on for this feature");
        }
        else
        {
            if (titanSaveWonstuff())
                return("WON stuff saved");
            else
                return("WON stuff save error");
        }
    }

    scriptSetTweakableGlobals();   // Must be called after file stuff is initialized

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : utyGameSystemsInit
    Description : Starts up all miscellaneous game systems.  call occurs after the
                  main window (hWnd) is created
    Inputs      : void
    Outputs     :
    Return      : NULL on success, string pointing to error message on error
----------------------------------------------------------------------------*/
char *utyGameSystemsInit(void)
{
    rndinitdata renderData;

    utyToggleKeyStatesSave();
    utySet2(SS2_ToggleKeys);

    strLoadLanguage((strLanguageType) strCurLanguage); // load in the language strings for messages to player
                                                       // must be called after file stuff and memory stuff

    // set the default player name based on the current language
    if (utyName[0] == 0)
    {
        strcpy(utyName, strGetString(strDefaultPlayerName));
    }

    utySet2(SS2_Strings);

    dbgMessagef("\nHomeworld CRC = 0x%x 0x%x 0x%x 0x%x", HomeworldCRC[0],HomeworldCRC[1],HomeworldCRC[2],HomeworldCRC[3]);
    //startup timer
    if (QueryPerformanceFrequency(&utyTimerFrequency) != TRUE)
    {                                                       //get performance timer speed
        return("Unable to query system performance timer.");
    }

        if (utyTimerFrequency.QuadPart < UTY_TimerResloutionMax)
    {                                                       //make sure it runs fast enough
        sprintf(errorString, "System performance timer runs at %d Hz.  We need at least %d Hz.",
                utyTimerFrequency, UTY_TimerResloutionMax);
        return(errorString);
    }
    utyTimerDivisor = utyTimerFrequency.QuadPart / UTY_TimerResloutionMax;
    utySet(SSA_Timer);
                                                            //start the task manager
    taskStartup((udword)(utyTimerFrequency.QuadPart / utyTimerDivisor));
    utySet(SSA_Task);

#if MEM_STATISTICS
    if (memStatsTaskHandle == 0xffffffff)
    {
        memStatsTaskHandle = taskStart(memStatsTaskFunction, MEM_TaskStatsPeriod, 0);//start frame rate task
    }
#endif

    // create the autorun lock file
    utyLockFilehandle = 0;
    utyLockFilehandle = fileOpen(utyLockFilename, FF_WriteMode|FF_IgnoreBIG);

    if (regStartup() != OKAY)                               //startup the region processor
    {
        return("Unable to start the region processor.");
    }
    utySet(SSA_Region);

    // initialize sound engine
    soundEventInit();
    utySet2(SS2_SoundEngine);

#if (!defined(Downloadable) || defined(DLPublicBeta))
    if (enableAVI)
    {
        MSG msg;
        BOOL gotMsg;
        sdword intro;

        utilPlayingIntro = TRUE;

        for (intro = 0;;)
        {
            Sleep(0);
            if (systemActive)
            {
                gotMsg = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
            }
            else
            {
                gotMsg = GetMessage(&msg, NULL, 0, 0);
            }
            if (gotMsg)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                switch (intro)
                {
                case 0:
                    binkInit(-1);
                    intro++;
                    break;
                case 1:
                    binkPlay("Movies\\sierra.bik", NULL, NULL, S_RGB555, TRUE, ANIM00_Sierra);
                    intro++;
                    break;
                case 2:
                    binkPlay("Movies\\relicintro.bik", NULL, NULL, S_RGB555, TRUE, ANIM00_Relic);
                    intro++;
                    break;
                case 3:
                    binkCleanup();
                    mainCleanupAfterVideo();
                    intro++;
                    break;
                case 4:
                    goto DONE_INTROS;
                }
            }
        }
    }
#else
    //show the opening plugscreens later
#endif

DONE_INTROS:
    utilPlayingIntro = FALSE;

    renderData.width = MAIN_WindowWidth;                    //setup data for
    renderData.height = MAIN_WindowHeight;                  //initializing the
    renderData.hWnd = ghMainWindow;                         //rendering system
    if (rndInit(&renderData) != OKAY)                       //startup the rendering system
    {
        //fallback to 640x480@16 rGL+sw, and fatally exit if that doesn't work either
        mainRestoreSoftware();
    }

    if (mouseStartup() != OKAY)
    {
        return("Unable to start mouse.");
    }
    utySet(SSA_Mouse);

    //create a region for the main window
    mrStartup();
    utySet(SSA_MainRegion);

    // Startup the information overlay, must occur after mrStartup()
    ioStartup();
    utySet2(SS2_InfoOverlay);

    utySet(SSA_Render);
    lightStartup();
    utySet(SSA_Lights);
    utyRenderTask = taskStart(rndRenderTask, 1.0f, TF_OncePerFrame);
    trStartup();
    utySet(SSA_TextureRegistry);

    taskCallBackInit();
    utySet2(SS2_BabyCallBackRegistry);

    universeInit();
    utySet(SSA_Universe);

    taskStart(universeUpdateTask, UNIVERSE_UPDATE_PERIOD, 0);

    autodownloadmapStartup();
    KeepAliveStartup();
    InitQueue(&ProcessSyncPktQ,50000);
    InitQueue(&ProcessCmdPktQ,50000);
    InitQueue(&ProcessRequestedSyncPktQ,20000);
    InitQueue(&ProcessCaptaincyPktQ,10000);
    InitLastSyncPktsQ();
    utySet2(SS2_NetworkQueues);
    taskStart(captainServerTask, CAPTAINSERVER_PERIOD, 0);

    gunStartup();
    utySet2(SS2_Guns);

    selStartup();
    utySet(SSA_Selection);

    uicStartup();                                           //start up UI controls
    utySet(SSA_UIControls);

    meshStartup();

    glcStartup();

    //startup transformer module
    transStartup();

    if (feStartup() != OKAY)
    {                                                       //start the front end
        return("Unable to start front end.");
    }
    utySet(SSA_FEFlow);
#if MAIN_MOUSE_FREE
    utyClipMouse(startupClipMouse);                         //clip mouse to window if needed
#endif
    utySet(SSA_MouseClipped);

    utyFrontEndDataLoad();
    utySet(SSA_FrontEndData);

    tacticsStartUp();

    lmStartup();
    utySet2(SS2_LaunchMgr);

    tmStartup();

    rmAPIStartup();
    utySet2(SS2_ResearchMgr);

#if defined(CGW) || defined(Downloadable) || defined(DLPublicBeta) || defined(OEM)
    psStartup();
    utySet2(SS2_PlugScreen);
#endif

    smStartup();                                            //start the sensors manager
    utySet(SSA_SensorsManager);

    svStartup();
    utySet2(SS2_ShipView);

    pingStartup();
    utySet2(SS2_Ping);

    battleChatterStartup();
    utySet2(SS2_BattleChatter);

    teStartup();                                            //start the team-specific stuff
    if (!cpColorsPicked)
    {
        utyBaseColor = teColorSchemes[0].textureColor.base;
        utyStripeColor = teColorSchemes[0].textureColor.detail;
    }
    utySet(SSA_Teams);

    cpStartup(&utyBaseColor, &utyStripeColor);                //start the color picker
    utySet(SSA_ColorPicker);

    subStartup();
    utySet2(SS2_SubTitle);

    tutStartup();
    utySet2(SS2_Tutorial);

    animStartup();      //start the animatics module

    spStartup();                                            //start the scenario picker
    utySet(SSA_ScenarioPicker);

    gpStartup();
    utySet2(SS2_GamePicker);

    mgStartup();
    lgStartup();
    mgGameTypeScriptInit();
    utySet2(SS2_MultiplayerGame);

    cmStartup();                                            //start the construction manager
    utySet(SSA_ConstructionManager);

    ranStartup();                                           //start the random-number generator
    utySet(SSA_RandomNumbers);

    bsStartup();
    utySet2(SS2_BSpline);

    nisStartup();
    utySet2(SS2_NIS);

    udStartup();                                            //start the undo module
    utySet(SSA_Undo);

    toStartup();                                            //start the TO module
    utySet(SSA_TacticalOverlay);

    dmgStartup();

    cloudStartup();

    shStartup();

    nebStartup();

    btgStartup();

    alodStartup();

    partStartup();

    pieStartup();

    trailStartup();                                         //start the trails module
    utySet(SSA_ShipTrails);

    //clear out the task timer.  Make sure this is the last call in this function.
    utyTaskTimerClear();

#if FE_TEXTURES_DISABLABLE
    if (fetEnableTextures)
#endif
    {
        ferStartup();                                       //start front end texture registry
    }

    prim3dStartup();                                        //start the prim 3d stuff
    utySet2(SS2_Prim3D);

    utySystemStarted = TRUE;

    opUpdateSettings();

    soundEventPlayMusic(SOUND_FRONTEND_TRACK);
//    if (!nisEnabled)
//    {
        feScreenStart(ghMainRegion, "Main_game_screen");
        mouseCursorShow();
//    }

#if (defined(Downloadable) || defined(DLPublicBeta))
    if (enableAVI)
    {
        primModeSetFunction2();
        psModeBegin("Plugscreens\\", PMF_CanSkip);
        psScreenStart("SierraIntro.plug");
    }
#endif
    if (demDemoRecording)
    {                                                       //if recording a demo
        sprintf(demDemoFilename + strlen(demDemoFilename), "%dx%d.dem", MAIN_WindowWidth, MAIN_WindowHeight);
#if DEM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nRecording demo '%s'.", demDemoFilename);
#endif
        determCompPlayer = TRUE;                            //computer player must be deterministic to record demos
        demRecordStart(demDemoFilename, utyPreDemoStateSaveCB);
    }
    else if (demDemoPlaying)
    {                                                       //if playing a demo
        sprintf(demDemoFilename + strlen(demDemoFilename), "%dx%d.dem", MAIN_WindowWidth, MAIN_WindowHeight);
        if (fileExists(demDemoFilename, 0))
        {
            demPlayStart(demDemoFilename, utyPreDemoStateLoadCB, utyDemoFinishedCB);
        }
        else
        {
#if DEM_VERBOSE_LEVEL >= 1
            dbgMessagef("\nDemo '%d' not found.", demDemoFilename);
#endif
            demDemoPlaying = FALSE;
        }
    }

/*
    if (nisEnabled)
    {
        mouseCursorHide();
        utyTeaserStart();
    }
*/

    if (RGLtype != SWtype)
    {
        (void)glcActivate(TRUE);
    }

    utySet(SS2_SystemStarted);                              //!!! leave this at the end of this function
    return(NULL);                                           //success, return no error
}

/*-----------------------------------------------------------------------------
    Name        : utyGameSystemsPreShutdown
    Description : Closes miscellaneous game systems.  ONLY CALL THIS if utyGameSystemsInit
                  failed but utyGameSystemsPreInit was successful
    Inputs      : void
    Outputs     :
    Return      : NULL on success, string pointing to error message on error
----------------------------------------------------------------------------*/
char* utyGameSystemsPreShutdown(void)
{
    BOOL result;
    if (utyTest(SSA_FontReg))
    {
        frShutdown();
        utyClear(SSA_FontReg);
    }

    bigClose();

    keyClose();

#if MEM_ANALYSIS
    memAnalysisCreate();
#endif
    if (utyTest(SSA_MemoryModule))
    {
        memClose(utyGrowthHeapFree);
        utyClear(SSA_MemoryModule);
    }
    if (utyTest(SSA_MemoryHeap))
    {
        result = VirtualFree(utyMemoryHeap, 0, MEM_RELEASE);
        dbgAssert(result);
        utyClear(SSA_MemoryHeap);
    }

    if (DebugWindow)
    {
        if (utyTest(SSA_DebugWindow))
        {
            dbwClose();
            utyClear(SSA_DebugWindow);
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : utyGameSystemsShutdown
    Description : Closes all miscellaneous game systems
    Inputs      : void
    Outputs     :
    Return      : NULL on success, string pointing to error message on error
----------------------------------------------------------------------------*/
char *utyGameSystemsShutdown(void)
{
    // close the autorun lock file
    if(utyLockFilehandle != 0) fileClose(utyLockFilehandle);

    // save the game options
    utyOptionsFileWrite();

    // write the debug window's options if so required
    if (DebugWindow)
    {
        if (utyTest(SSA_DebugWindow))
        {
            dbwWriteOptions();
        }
    }

    if (demDemoRecording)
    {                                                   //if recording a demo
        demRecordEnd();
    }
    else if (demDemoPlaying)
    {
        demPlayEnd();
    }

    if (utyTest2(SS2_Strings))
    {
        strFreeLanguage();
        utyClear2(SS2_Strings);
    }

    if (utyTest(SSA_Universe))
    {
        universeClose();
        utyClear(SSA_Universe);
    }

    if (utyTest2(SS2_AIPlayer))
    {
        aiplayerShutdown();
        utyClear2(SS2_AIPlayer);
    }

    if (utyTest2(SS2_SoundEngine))
    {
        // shutdown sound engine
        soundEventClose();
        utyClear2(SS2_SoundEngine);
    }

    if(utyTest2(SS2_BabyCallBackRegistry))
    {
        taskCallBackShutDown();
    }

    if (utyTest2(SS2_NetworkQueues))
    {
        autodownloadmapShutdown();
        KeepAliveShutdown();
        CloseQueue(&ProcessCmdPktQ);
        CloseQueue(&ProcessSyncPktQ);
        CloseQueue(&ProcessRequestedSyncPktQ);
        CloseQueue(&ProcessCaptaincyPktQ);
        CloseLastSyncPktsQ();
        utyClear2(SS2_NetworkQueues);
    }
    dmgShutdown();
    cloudShutdown();
    shShutdown();
    nebShutdown();
    btgShutdown();
    alodShutdown();
    partShutdown();
    if (utyTest(SSA_ShipTrails))
    {
        trailShutdown();                                    //shut down the TO module
        utyClear(SSA_ShipTrails);
    }
    if (utyTest(SSA_TacticalOverlay))
    {
        toShutdown();                                       //shut down the TO module
        utyClear(SSA_TacticalOverlay);
    }
    if (utyTest(SSA_Undo))
    {
        udShutdown();                                       //shut down the undo module
        utyClear(SSA_Undo);
    }
    if (utyTest(SSA_ColorPicker))
    {
        cpShutdown();                                       //shut down the color picker
        utyClear(SSA_ColorPicker);
    }
    if (utyTest(SSA_RandomNumbers))
    {
        ranShutdown();                                      //shut down the random-number genrator
        utyClear(SSA_RandomNumbers);
    }
    if (utyTest2(SS2_BSpline))
    {
        bsShutdown();                                       //shut down the random-number genrator
        utyClear2(SS2_BSpline);
    }
    if (utyTest2(SS2_NIS))
    {
        nisShutdown();                                      //shut down the random-number genrator
        utyClear2(SS2_NIS);
    }
    if (utyTest(SSA_ScenarioPicker))
    {
        spShutdown();                                       //shut down the scenario picker
        utyClear(SSA_ScenarioPicker);
    }
    animShutdown();     //shut down the animatics module
    if (utyTest2(SS2_GamePicker))
    {
        gpShutdown();
        utyClear2(SS2_GamePicker);
    }
    if (utyTest2(SS2_MultiplayerGame))
    {
        mgShutdown();
        lgShutdown();
        utyClear2(SS2_MultiplayerGame);
    }
    if(preSetGames != NULL)
    {
        memFree(preSetGames);
        preSetGames = NULL;
    }
    if (utyTest(SSA_Teams))
    {
        teShutdown();                                       //shut down the team-specifics
        utyClear(SSA_Teams);
    }
    if (utyTest(SSA_SensorsManager))
    {
        smShutdown();                                       //shut down the sensors manager
        utyClear(SSA_SensorsManager);
    }
    if (utyTest2(SS2_SubTitle))
    {
        subShutdown();
        utyClear2(SS2_SubTitle);
    }
    if (utyTest2(SS2_Tutorial))
    {
        tutShutdown();
        utyClear2(SS2_Tutorial);
    }
    if (utyTest2(SS2_BattleChatter))
    {
        battleChatterShutdown();
        utyClear2(SS2_BattleChatter);
    }
    if (utyTest2(SS2_Ping))
    {
        pingShutdown();
        utyClear2(SS2_Ping);
    }
    if (utyTest2(SS2_ShipView))
    {
        svShutdown();
        utyClear2(SS2_ShipView);
    }
    if (utyTest(SSA_TaskBar))
    {
        tbShutdown();                                            //shut down the task bar
        utyClear(SSA_TaskBar);
    }
    if (utyTest(SSA_ConstructionManager))
    {
        cmShutdown();                                            //shut down the construction manager
        utyClear(SSA_ConstructionManager);
    }
    if (utyTest2(SS2_LaunchMgr))
    {
        lmShutdown();
        utyClear2(SS2_LaunchMgr);
    }

    tmShutdown();

#if defined(CGW) || defined(Downloadable) || defined(DLPublicBeta) || defined(OEM)
    if (utyTest2(SS2_PlugScreen))
    {
        psShutdown();
        utyClear2(SS2_PlugScreen);
    }
#endif
    if (utyTest2(SS2_ResearchMgr))
    {
        rmAPIShutdown();
        utyClear2(SS2_ResearchMgr);
    }
    if (utyTest(SSA_Lights))
    {
        lightShutdown();
        utyClear(SSA_Lights);
    }
    if (utyTest(SSA_FrontEndData))
    {
        utyFrontEndDataFree();
        utyClear(SSA_FrontEndData);
    }
    if (utyTest(SSA_FontReg))
    {
        frShutdown();
        utyClear(SSA_FontReg);
    }
    if (utyTest(SSA_FEFlow))
    {
        feShutdown();
        utyClear(SSA_FEFlow);
    }
    if (utyTest(SSA_UIControls))
    {
        uicShutdown();
        utyClear(SSA_UIControls);
    }
    if (utyTest(SSA_Effect))
    {
        etgShutdown();
        //effShutdown();
        utyClear(SSA_Effect);
    }
    if (utyTest2(SS2_InfoOverlay))
    {
        ioShutdown();
        utyClear2(SS2_InfoOverlay);
    }
    if (utyTest(SSA_TextureRegistry))
    {
        trShutdown();
        utyClear(SSA_TextureRegistry);
    }

    glcShutdown();

    if (utyTest(SSA_Render))
    {
        if (!RGL)
        {
            hwDeleteWindow();
        }
        rndClose();
        //the GL has now SHUTDOWN
        utyClear(SSA_Render);
    }

    if (utyTest(SSA_MainRegion))
    {
        mrShutdown();
        utyClear(SSA_MainRegion);
    }

    if (utyTest(SSA_Region))
    {
        regShutdown();
        utyClear(SSA_Region);
    }
    if (utyTest2(SS2_Guns))
    {
        gunShutdown();
        utyClear2(SS2_Guns);
    }
    if (utyTest(SSA_Selection))
    {
        selShutdown();
        utyClear(SSA_Selection);
    }
    if (utyTest(SSA_Mouse))
    {
        mouseShutdown();
        utyClear(SSA_Mouse);
    }

    titanShutdown();        // safe to call multiple times

    //cleans up the prim3d structures
    if (utyTest2(SS2_Prim3D))
    {
        prim3dShutdown();
        utyClear2(SS2_Prim3D);
    }
    //shutdown front end texture registry
#if FE_TEXTURES_DISABLABLE
    if (fetEnableTextures)
#endif
    {
        ferShutdown();
    }
    if (utyTest(SSA_Task))
    {
        taskShutdown();
        utyClear(SSA_Task);
    }

    //shutdown mesh.c
    meshShutdown();

    //shutdown transformer module
    transShutdown();

    bigClose();

    keyClose();

#if MEM_ANALYSIS
    memAnalysisCreate();
#endif
    if (utyTest(SSA_MemoryModule))
    {
        memClose(utyGrowthHeapFree);
        utyClear(SSA_MemoryModule);
    }
    if (utyTest(SSA_MemoryHeap))
    {
        bool result;

        result = VirtualFree(utyMemoryHeap, 0, MEM_RELEASE);
        dbgAssert(result);
        utyClear(SSA_MemoryHeap);
    }

    // close down the debug window if so required
    if (DebugWindow)
    {
        if (utyTest(SSA_DebugWindow))
        {
            dbwClose();
            utyClear(SSA_DebugWindow);
        }
    }

    globalsClose();

    if (utyTest2(SS2_ToggleKeys))
    {
        utyToggleKeyStatesRestore();
    }

    utySystemStarted = FALSE;
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : utyTasksDispatch
    Description : Query system timer and call task manager with appropriate
                    number of ticks.
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void utyTasksDispatch(void)
{
    LARGE_INTEGER timer;
    LONGLONG difference, remainder;

    QueryPerformanceCounter(&timer);                        //get counter, let's assume it works
    difference = timer.QuadPart - utyTimerLast;             //get difference of this frame to last frame
    utyNFrameTicks = (udword)(difference / utyTimerDivisor);
    remainder = difference % utyTimerDivisor;               //get remainder of the division
    taskExecuteAllPending(utyNFrameTicks);                  //execute tasks for the correct number of ticks
    utyTimerLast = timer.QuadPart - remainder;              //save timer for differencing next frame
}

/*-----------------------------------------------------------------------------
    Name        : utyTaskTimerClear
    Description : Cancel all previous task timer ticks to the present.
    Inputs      : void
    Outputs     : re-querys the system timer and storts result for later
                    differencing
    Return      : void
----------------------------------------------------------------------------*/
void utyTaskTimerClear(void)
{
    LARGE_INTEGER timer;

    QueryPerformanceCounter(&timer);                        //get counter, let's assume it works
    utyTimerLast = timer.QuadPart;                          //save timer for differencing next frame

}

/*-----------------------------------------------------------------------------
    Name        : utyClientRectGet
    Description : Gets the rectangle corresponding to the client rectange
                    _in screen coords_
    Inputs      : rect - rectangle to fill with new coords
    Outputs     : Calls GetClientRect() and ClientToScreen() on both corners.
                    rect gets the new rectangle.
    Return      :
----------------------------------------------------------------------------*/
void utyClientRectGet(rectangle *rect)
{
    RECT clientRect;
    POINT point;
    GetClientRect(ghMainWindow, &clientRect);               //get location of window
    point.x = clientRect.left;
    point.y = clientRect.top;
    ClientToScreen(ghMainWindow, &point);
    rect->x0 = point.x;
    rect->y0 = point.y;
    /*
    point.x = clientRect.right;
    point.y = clientRect.bottom;
    ClientToScreen(ghMainWindow, &point);
    rect->x1 = point.x;
    rect->y1 = point.y;
    */
    rect->x1 = rect->x0 + MAIN_WindowWidth - 1;
    rect->y1 = rect->y0 + MAIN_WindowHeight - 1;
}

/*-----------------------------------------------------------------------------
    Name        : utyForceTopmost
    Description : Determine if the window should be forced topmost
    Inputs      : bTopMost - if TRUE, window will be forced topmost (and above the taskbar)
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void utyForceTopmost(bool bTopMost)
{
    SetWindowPos(ghMainWindow, bTopMost ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOSENDCHANGING);
}

/*-----------------------------------------------------------------------------
    Name        : utyClipMouse
    Description : Sets or frees the mouse clipped to the client area of main window
    Inputs      : clip - boolean for clip state.  TRUE = clip, FALSE = free
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
#if MAIN_MOUSE_FREE
void utyClipMouse(sdword clip)
{
    RECT rect;
    rectangle *clientRect;

    if ((!mouseClipped) && (clip))
    {
        clientRect = (rectangle *)(&rect);
        utyClientRectGet(clientRect);
        ClipCursor(&rect);
    }
    if ((mouseClipped) && (!clip))
    {
        ClipCursor(NULL);
    }
    mouseClipped = clip;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : utyMouseButtonsClear
    Description : Resets the internal mouse button states.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyMouseButtonsClear(void)
{
    mousePoll();
    if (GetKeyState(VK_LBUTTON) & BIT15)
    {
        keyPressDown(KeyMapFromWindows(VK_LBUTTON));
    }
    else
    {
        keyPressUp(KeyMapFromWindows(VK_LBUTTON));
    }
    if (GetKeyState(VK_RBUTTON) & BIT15)
    {
        keyPressDown(KeyMapFromWindows(VK_RBUTTON));
    }
    else
    {
        keyPressUp(KeyMapFromWindows(VK_RBUTTON));
    }
    if (GetKeyState(VK_MBUTTON) & BIT15)
    {
        keyPressDown(KeyMapFromWindows(VK_MBUTTON));
    }
    else
    {
        keyPressUp(KeyMapFromWindows(VK_MBUTTON));
    }
}

/*-----------------------------------------------------------------------------
    Name        : utyCaretBlinkRate
    Description : Get the rate of the cursor blink action from Windows.
    Inputs      :
    Outputs     :
    Return      : Blink rate, in seconds.
----------------------------------------------------------------------------*/
real32 utyCaretBlinkRate(void)
{
    return((real32)GetCaretBlinkTime() / 500.0f);
}

/*-----------------------------------------------------------------------------
    Name        : utyDoubleClick
    Description : Process a double-click event
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyDoubleClick(void)
{
    if (!mouseLDoubleClick())
    {
        //this is needed because the second button click is
        //not registered as a WM_LBUTTONDOWN click.  Only call this
        //function if mouseLDoubleClick didn't do any special processing
        keyPressDown(KeyMapFromWindows(VK_LBUTTON));
    }
    keyPressUp(KeyMapFromWindows(VK_LBUTTON));
}

/*-----------------------------------------------------------------------------
    Name        : utyCodeCRCCompute
    Description : Compute a CRC of the game's code segment
    Inputs      : void
    Outputs     :
    Return      : CRC of code segment
----------------------------------------------------------------------------*/
udword utyCodeCRCCompute(void)
{
    return((udword)crc32Compute((ubyte *)linkStartCode, (udword)linkEndCode - (udword)linkStartCode));
}

/*-----------------------------------------------------------------------------
    Name        : utyDataCRCCompute
    Description : Compute a CRC of the game's code segment
    Inputs      : void
    Outputs     :
    Return      : CRC of code segment
----------------------------------------------------------------------------*/
udword utyDataCRCCompute(void)
{
    return((udword)crc32Compute((ubyte *)linkStartData, (udword)linkEndData - (udword)linkStartData));
}

/*-----------------------------------------------------------------------------
    Name        : utyChangeResolution
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool utyChangeResolution(sdword width, sdword height, sdword depth)
{
    if (!utyTest(SSA_Render))
    {
        return FALSE;
    }

    ghMainRegion->rect.x1 = width;
    ghMainRegion->rect.y1 = height;

    (void)feAllScreensReposition();

    //rescale the subtitle regions
    subRegionsRescale();

    //reposition the info overlay regions
    ioResolutionChange();

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : utyToggleKeyStatesSave
    Description : Save the state of all toggle keys.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
static sdword utyCapsLockState;
static sdword utyNumLockState;
static sdword utyScrollLockState;
void utyToggleKeyStatesSave(void)
{
    ubyte state[256];

    GetKeyboardState(state);
    utyCapsLockState = state[VK_CAPITAL] & 1;
    utyNumLockState = state[VK_NUMLOCK] & 1;
    utyScrollLockState = state[VK_SCROLL] & 1;
}

/*-----------------------------------------------------------------------------
    Name        : utyToggleKeyStatesRestore
    Description : Restore the caps, scroll and num lock keys to what they were
                    at when you started the game or whence last you alt-tabbed
                    back to the game.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void utyToggleKeyStatesRestore(void)
{
    ubyte state[256];

    GetKeyboardState(state);
    if((utyCapsLockState && !(state[VK_NUMLOCK] & 1)) || (!utyCapsLockState && (state[VK_CAPITAL] & 1)) )
    {
        // Simulate a key press
        keybd_event( VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0 );

        // Simulate a key release
        keybd_event( VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }
    if((utyScrollLockState && !(state[VK_NUMLOCK] & 1)) || (!utyScrollLockState && (state[VK_SCROLL] & 1)) )
    {
        // Simulate a key press
        keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0 );

        // Simulate a key release
        keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }
    if((utyNumLockState && !(state[VK_NUMLOCK] & 1)) || (!utyNumLockState && (state[VK_NUMLOCK] & 1)) )
    {
        // Simulate a key press
        keybd_event( VK_NUMLOCK, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0 );

        // Simulate a key release
        keybd_event( VK_NUMLOCK, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }
    state[VK_CAPITAL]  = (state[VK_CAPITAL] & 0xfe) | (utyCapsLockState);
    state[VK_NUMLOCK]  = (state[VK_NUMLOCK] & 0xfe) | (utyNumLockState);
    state[VK_SCROLL]  = (state[VK_SCROLL] & 0xfe) | (utyScrollLockState);
    SetKeyboardState(state);
}

/*-----------------------------------------------------------------------------
    Name        : utyCapsLockToggleState
    Description : Returns the toggle state of the caps lock key
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool utyCapsLockToggleState(void)
{
    return(GetKeyState(VK_CAPITAL) & 1);
}

void utyStartDroppedDialog(sdword playernum)
{
    cmCloseIfOpen();
    rmCloseIfOpen();
    lmCloseIfOpen();
    mrReset();
    utyPlayerDroppedDisplay = playernum;
    mrDisable();
    feScreenStart(ghMainRegion, "Player_Drop");
}

void utyYesDropPlayer(char *name, featom *atom)
{
    KeepAliveDropPlayerCB(utyPlayerDroppedDisplay);
    utyPlayerDroppedDisplay = -1;

    mrEnable();
    feScreenDisappear(NULL,NULL);
}

void utyDontDropPlayer(char *name, featom *atom)
{
    KeepAliveDontDropPlayerCB(utyPlayerDroppedDisplay);
    utyPlayerDroppedDisplay = -1;

    mrEnable();
    feScreenDisappear(NULL,NULL);
}

void utyDrawDroppedPlayer(featom *atom, regionhandle region)
{
    fonthandle fhsave;

    if (FELASTCALL(atom))
    {
        KeepAliveDontDropPlayerCB(utyPlayerDroppedDisplay);
        utyPlayerDroppedDisplay = -1;

        mrEnable();

        return;
    }

    if (utyPlayerDroppedDisplay != -1)
    {
        fhsave = fontMakeCurrent(selGroupFont0);

        fontPrint(region->rect.x0, region->rect.y0, colRGB(255,200,0), playerNames[utyPlayerDroppedDisplay]);

        fontMakeCurrent(fhsave);
    }
}


