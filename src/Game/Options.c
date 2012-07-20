/*=============================================================================
    Name    : options.c
    Purpose : Logic for the Options screen

    Created 11/05/1998 by yo
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <math.h>
#include "glinc.h"
#include "Types.h"
#include "Key.h"
#include "FEFlow.h"
#include "Options.h"
#include "FontReg.h"
#include "mouse.h"
#include "utility.h"
#include "UIControls.h"
#include "texreg.h"
#include "Sensors.h"
#include "SoundEvent.h"
#include "Shader.h"
#include "AutoLOD.h"
#include "rinit.h"
#include "AIPlayer.h"
#include "FEColour.h"
#include "sstglide.h"
#include "Battle.h"
#include "soundlow.h"
#include "glcompat.h"
#include "InfoOverlay.h"
#include "KeyBindings.h"
#include "glcaps.h"
#include "MultiplayerGame.h"
#include "debugwnd.h"
#include "devstats.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif


/*=============================================================================
    Definitions:
=============================================================================*/

extern udword gDevcaps;
extern udword gDevcaps2;
static udword opDevcaps;
static udword opDevcaps2;

bool   opTimerActive = FALSE;
real32 opTimerStart;
real32 opTimerLength = 22.0f;

bool opReloading = FALSE;

static char lastDev[64] = "";

sdword speechEventCleanup(void);


#define CONTROL_FEEDBACK 0.75f

#define OP_DefaultFont          "default.hff"
#define OP_KeyVertSpacing       17
#define OP_NumKeys              25
#define OP_KeysPerColumn        10
#define OP_KBMarginTop          10
#define OP_KBMarginLeft         10
#define OP_KBPrimaryKeyTab      130
#define OP_KBSecondaryKeyTab    172
#define OP_KBColumnTab          214
#define OP_KBBoxwidth           211
#define OP_BoxSpacingX          4
#define OP_BoxSpacingY          0


#define OP_KeyNameColor         colRGB(100,200,50)
#define OP_KeyPrimaryColor      colRGB(120,140,170)
#define OP_KeySecondaryColor    colRGB(80,100,150)
#define OP_KeySelectedColor     colRGB(200,200,50)
#define OP_KeyRedefiningColor   colRGB(250,80,80)
#define OP_KeyHighlightColor    colRGB(190,223,255)

#define OP_KeyHighlightTime     1
#define OP_KeyHighlightOnTime   0.8f
#define OP_HighlightStack       32

#define OP_SMOOTHIE_RATE        0.01f

#define NUM_EQ_BANDS            8


#if(0)

bool smoothiesactive = FALSE;

real32 smoothmusicvol;
real32 smoothsfxvol;
real32 smoothspeechvol;
Smoothie volumesmoothie;
Smoothie speechsmoothie;
Smoothie sfxsmoothie;

real32 smootheqvalue[NUM_EQ_BANDS];
Smoothie smootheqsmoothie[NUM_EQ_BANDS];



region *volumeregion;
region *sfxregion;
region *speechregion;
region *nchannelsregion;


Smoothie *SmoothieArray[NUM_SMOOTHIES];


bool opSmoothiesBabyFunction(udword num, void *data, struct BabyCallBack *baby);
void AddSmoothie(Smoothie *smoo);
void RemoveSmoothie(Smoothie *smoo);
void DoSmoothies(void);

#endif


void opEQHelper(void);
void opUpdateAudioSettings(void);
void opUpdateVideoSettings(void);

void opOptionsEndHelper(char* linkName);
void opOptionsEnd(void);

// The following functions are used to save and or restore the options in each screen

// the equalizer screen
void opOptionsSaveEqualizerSettings(void);
void opRestoreSavedEqualizerSettings(void);

// the advanced audio options screen
void opOptionsSaveAdvancedSettings(void);
void opRestoreAdvancedSettings(void);

// The three index screens altogether
void opOptionsSaveSettings(void);
void opRestoreSavedCustomEffectsSettings(void);


void opResListSet(rdevice* dev);

void opRenderListLoad(void);


/*=============================================================================
    Data:
=============================================================================*/

bool opCustomEffectsToggled;
bool opEqualizerToggled;

sword opKeySelected=-1;
sword opKeyBeingDefined=-1;
uword opKeyDetour = 0;
regionhandle keyboardregion = NULL;
regionhandle drawnumchannels = NULL;
//taskhandle opHighlightTaskHandle;
BabyCallBack    *opHighlightBaby=NULL;
BabyCallBack    *opSmoothiesBaby=NULL;

typedef struct
{
    keyindex key1;
    keyindex key2;
    sword highlight;
    char *funcname;
}
tempkeytrans;

tempkeytrans TempKeyTranslations[OP_NumKeys] =
{
   {TABKEY,      0, 0 ,"NEXT FORMATION" },
   {BKEY,        0, 0 ,"BUILD MANAGER" },
   {CKEY,        0, 0 ,"PREVIOUS FOCUS" },
   {VKEY,        0, 0 ,"NEXT FOCUS" },
   {DKEY,        0, 0 ,"DOCK" },
   {EKEY,        0, 0 ,"SELECT ALL VISIBLE" },
   {FKEY,        0, 0 ,"FOCUS" },
   {RKEY,        0, 0 ,"RESEARCH MANAGER" },
   {HKEY,        0, 0 ,"HARVEST" },
   {MKEY,        0, 0 ,"MOVE" },
   {RBRACK,      0, 0 ,"NEXT TACTIC" },
   {LBRACK,      0, 0 ,"PREVIOUS TACTIC" },
   {SKEY,        0, 0 ,"SCUTTLE" },
   {ZKEY,        0, 0 ,"SHIP SPECIAL" },
   {CAPSLOCKKEY, 0, 0 ,"TACTICAL OVERLAY" },
   {SPACEKEY,    0, 0 ,"SENSORS MANAGER" },
   {HOMEKEY,     0, 0 ,"MOTHERSHIP" },
   {KKEY,        0, 0 ,"KAMIKAZE" },
   {TILDEKEY,    0, 0 ,"CANCEL ORDERS" },
   {0,0,0,""}
};

typedef struct
{
    keyindex sendkey;
    keyindex key1;
    keyindex key2;
}
keytrans;


keytrans KeyTranslations[OP_NumKeys] =
{
   {TABKEY,         TABKEY,      0 },   //NEXT FORMATION
   {BKEY,           BKEY,        0 },   //BUILD MANAGER
   {CKEY,           CKEY,        0 },   //PREVIOUS FOCUS
   {VKEY,           VKEY,        0 },   //NEXT FOCUS
   {DKEY,           DKEY,        0 },   //DOCK
   {EKEY,           EKEY,        0 },   //SELECT ALL VISIBLE
   {FKEY,           FKEY,        0 },   //FOCUS
   {RKEY,           RKEY,        0 },   //RESEARCH MANAGER
   {HKEY,           HKEY,        0 },   //HARVEST
   {MKEY,           MKEY,        0 },   //MOVE
   {RBRACK,         RBRACK,      0 },   //NEXT TACTIC
   {LBRACK,         LBRACK,      0 },   //PREVIOUS TACTIC
   {SKEY,           SKEY,        0 },   //SCUTTLE
   {ZKEY,           ZKEY,        0 },   //SHIP SPECIAL
   {CAPSLOCKKEY,    CAPSLOCKKEY, 0 },   //TACTICAL OVERLAY
   {SPACEKEY,       SPACEKEY,    0 },   //SENSORS MANAGER
   {HOMEKEY,        HOMEKEY,     0 },   //MOTHERSHIP
   {KKEY,           KKEY,        0 },   //KAMIKAZE
   {TILDEKEY,       TILDEKEY,    0 },   //CANCEL ORDERS
   {0,0,0 }
};



keyindex DefaultKeyTranslations[OP_NumKeys] =
{
   TABKEY,              //NEXT FORMATION
   BKEY,                //BUILD MANAGER
   CKEY,                //PREVIOUS FOCUS
   VKEY,                //NEXT FOCUS
   DKEY,                //DOCK
   EKEY,                //SELECT ALL VISIBLE
   FKEY,                //FOCUS
   RKEY,                //RESEARCH MANAGER
   HKEY,                //HARVEST
   MKEY,                //MOVE
   RBRACK,              //NEXT TACTIC
   LBRACK,              //PREVIOUS TACTIC
   SKEY,                //SCUTTLE
   ZKEY,                //SHIP SPECIAL
   CAPSLOCKKEY,         //TACTICAL OVERLAY
   SPACEKEY,            //SENSORS MANAGER
   HOMEKEY,             //MOTHERSHIP
   KKEY,                //KAMIKAZE
   TILDEKEY             //CANCEL ORDERS
};



typedef struct
{
    keyindex key;
    char *name;
}
KeyNameStruct;

KeyNameStruct KeyNames[] =
{
   {AKEY,           "A    "},
   {BKEY,           "B    "},
   {CKEY,           "C    "},
   {DKEY,           "D    "},
   {EKEY,           "E    "},
   {FKEY,           "F    "},
   {GKEY,           "G    "},
   {HKEY,           "H    "},
   {IKEY,           "I    "},
   {JKEY,           "J    "},
   {KKEY,           "K    "},
   {LKEY,           "L    "},
   {MKEY,           "M    "},
   {NKEY,           "N    "},
   {OKEY,           "O    "},
   {PKEY,           "P    "},
   {QKEY,           "Q    "},
   {RKEY,           "R    "},
   {SKEY,           "S    "},
   {TKEY,           "T    "},
   {UKEY,           "U    "},
   {VKEY,           "V    "},
   {WKEY,           "W    "},
   {XKEY,           "X    "},
   {YKEY,           "Y    "},
   {ZKEY,           "Z    "},

   {BACKSPACEKEY,   "BKSPC"},
   {TABKEY,         "TAB  "},
   {ARRLEFT,        "LEFT "},
   {ARRRIGHT,       "RIGHT"},
   {ARRUP,          "UP   "},
   {ARRDOWN,        "DOWN "},
   {LBRACK,         "[    "},
   {RBRACK,         "]    "},
   {CAPSLOCKKEY,    "CAPS "},
   {SPACEKEY,       "SPACE"},
   {ENTERKEY,       "ENTER"},
   {HOMEKEY,        "HOME "},
   {PAGEDOWNKEY,    "PGDN "},
   {PAGEUPKEY,      "PGUP "},
   {BACKSLASHKEY,   "\\    "},
   {PAUSEKEY,       "PAUSE"},
   {SCROLLKEY,      "SCRLL"},
   {PRINTKEY,       "PRINT"},
   {DELETEKEY,      "DELET"},
   {TILDEKEY,       "TILDE"},
   {NUMPAD0,        "NUM0"},
   {NUMPAD1,        "NUM1"},
   {NUMPAD2,        "NUM2"},
   {NUMPAD3,        "NUM3"},
   {NUMPAD4,        "NUM4"},
   {NUMPAD5,        "NUM5"},
   {NUMPAD6,        "NUM6"},
   {NUMPAD7,        "NUM7"},
   {NUMPAD8,        "NUM8"},
   {NUMPAD9,        "NUM9"},
   {NUMMINUSKEY,    "NUM-"},
   {NUMPLUSKEY,     "NUM+"},
   {NUMSTARKEY,     "NUM*"},
   {NUMSLASHKEY,    "NUM/"},
   {NUMDOTKEY,      "NUM."},
   {MINUSKEY,       "-"},
   {PLUSKEY,        "+"},
   {F1KEY,          "F1"},
   {F2KEY,          "F2"},
   {F3KEY,          "F3"},
   {F4KEY,          "F4"},
   {F5KEY,          "F5"},
   {F6KEY,          "F6"},
   {F7KEY,          "F7"},
   {F8KEY,          "F8"},
   {F9KEY,          "F9"},
   {F10KEY,          "F10"},
   {F11KEY,          "F11"},
   {F12KEY,          "F12"},

   {0,""}
};


char opKeymap[OP_KEYMAPSIZE] = "Z";


sdword opSpeakerSettings[3][NUM_EQ_BANDS] =
{
    {100,66,33,0,0,0,0,0},   //multimedia
    {0,0,0,0,0,0,0,0},       //stereo
    {0,17,33,50,50,33,17,0}, //headphones
};


fonthandle opKeyboardFont;


//use #define to modify a variable directly without modifying function

extern void mgDrawArrow(regionhandle region, bool leftArrow, bool human);

sdword opNoPalMaxMB = 32;
sdword opNoPalMinMB = 16;
sdword opSaveNoPalMB;
sdword opNoPalMB = 20;

static regionhandle opNoPalDrawRegion = NULL;

static udword opOldDevcaps;
static udword opOldDevcaps2;
static sdword opOldDeviceIndex;

udword opDeviceCRC = 0;
sdword opDeviceIndex = -1;

sdword op3DfxChanged = 0;
sdword opUsing3DfxGL = 0;

sdword opMusicVol=75;
sdword opSaveMusicVol;
sdword opSFXVol=75;
sdword opSaveSFXVol;
sdword opSpeechVol=60;
sdword opSaveSpeechVol;
sdword opNumChannels=SOUND_DEF_VOICES-SOUND_MIN_VOICES;
sdword opSaveNumChannels;
sdword opAutoChannel=SOUND_MODE_NORM;
sdword opSaveAutoChannel;
sdword opSoundQuality=SOUND_MODE_NORM;
sdword opSaveSoundQuality;
sdword opSpeakerSetting=1;
sdword opSaveSpeakerSetting;
sdword opEqualizerSettings[NUM_EQ_BANDS];
sdword opSaveEqualizerSettings[NUM_EQ_BANDS];
real32 opEQReal[NUM_EQ_BANDS];
sdword opVoiceSetting=2;
sdword opVoice0On = 1, opVoice1On = 1, opVoice2On = 1;
sdword opSaveVoice0On, opSaveVoice1On, opSaveVoice2On;
sdword opSaveVoiceSetting;
sdword opVoiceComm=1;
sdword opSaveVoiceComm;
sdword opVoiceStat=1;
sdword opSaveVoiceStat;
sdword opVoiceChat=1;
sdword opSaveVoiceChat;


#define OP_DETAIL_DIVISOR 22

sdword opSaveDetailThresholdVal;
udword opDetailThresholdVal = 100;
sdword opSaveBrightnessVal;
sdword opBrightnessVal = 30;
sdword opNoLODVal = 0;
sdword opSaveNoLODVal;

sdword opSaveMAIN_WindowWidth;
sdword opSaveMAIN_WindowHeight;
sdword opSaveMAIN_WindowDepth;
sdword opSaveRenderDevice;


sdword opLightingVal=1;
sdword opSaveLightingVal;
sdword opEffectsVal=0;
sdword opSaveEffectsVal;
sdword savetexLinearFiltering;
sdword saveshowBackgrounds;
sdword savesmInstantTransition;
sdword saveenableSmoothing;
sdword saveenableStipple;
sdword saveenableTrails;
sdword savesmFuzzyBlobs;
sdword saveHitEffects;
sdword saveDamagEffects;
sdword saveMuzzleEffects;
sdword saveBulletEffects;

sdword opAutodockFuelCheck = 0;
sdword opSaveAutodockFuelCheck;
sdword opAutodockHealthCheck = 1;
sdword opSaveAutodockHealthCheck;

sdword opAutodockHealthVal = 20;
sdword opSaveAutodockHealthVal;
sdword opAutodockFuelVal = 5;
sdword opSaveAutodockFuelVal;


sdword opMouseSens = 50;
sdword opBattleChatter = 50;
sdword opInfoOverlayVar = 1;
sdword opSaveMouseSens;
sdword opSaveBattleChatter;
sdword opSaveInfoOverlay = 1;

sdword opNumEffects;
sdword opSaveNumEffects;

textentryhandle opAutodockFuelEntryBox     = NULL;
textentryhandle opAutodockHealthEntryBox   = NULL;

#define MAX_NUM_LENGTH 12


//video listboxes

fonthandle opResListFont = 0;
listwindowhandle opResListWindow = NULL;
sdword opResNumber;
sdword opResCurrentSelected;

typedef struct opres
{
    sdword width, height, depth;
} opres;

opres* opRes = NULL;

fonthandle opRenderListFont = 0;
listwindowhandle opRenderListWindow = NULL;
sdword opRenderNumber;
sdword opRenderCurrentSelected;

sliderhandle EffectsSlider = NULL;

typedef struct oprender
{
    int type;
    char data[64];
    char name[64];
    rdevice* dev;
} oprender;

oprender* opRnd = NULL;

static oprender* opRndSelected;
static oprender* opSaveRndSelected;

regionhandle opResListRegion = NULL;

//end video


void opCancelKeyDetour(void);


/*=============================================================================
    Private functions:
=============================================================================*/

/*=============================================================================
    Functions:
=============================================================================*/

void opDirtyResListWindow(void)
{
    if (opResListRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(((regionhandle)opResListRegion));
#endif
        opResListRegion->status |= RSF_DrawThisFrame;
    }
}

/*-----------------------------------------------------------------------------
    Name        : opOptionsInit
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void opOptionsInit(void)
{
    opKeyboardFont = frFontRegister("hw_eurosecond_11.hff");
    opNumEffects = etgHistoryScalar - etgHistoryScalarMin;
    kbInitKeyBindings();
}

/*-----------------------------------------------------------------------------
    Name        : opSetDetailThreshold
    Description : reset the autolod module w/ new LOD scalar
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opSetDetailThreshold(void)
{
    udword targetPolys, polyDelta;

    alodGetTargetPolys(&targetPolys, &polyDelta);

    targetPolys = OP_DETAIL_DIVISOR * opDetailThresholdVal;

    alodSetTargetPolys(targetPolys, polyDelta);
}

/*-----------------------------------------------------------------------------
    Name        : opUpdateSettings
    Description : update game system settings after reading configuration file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opUpdateSettings(void)
{
    opUpdateVideoSettings();
    opOptionsSaveSettings();
}

/*============================================================================

    Keyboard options

============================================================================*/


udword opSelectKey(regionhandle region, sdword ID, udword event, udword data);

/*-----------------------------------------------------------------------------
    Name        : opKeyboardDraw
    Description : Callback for keyboard redefinitions
    Inputs      : standard region callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opKeyboardDraw(featom *atom, regionhandle region)
{
    sdword x, y, i;
    rectangle rect = region->rect, select;
    color c;
    fonthandle currentFont;
    bool hl = FALSE;

    keyboardregion = region;

    currentFont = fontMakeCurrent(opKeyboardFont);

    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_PressRight |
                        RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
        regFunctionSet(region, opSelectKey);          //set new region handler function
        regFilterSet(region, region->flags | RPE_DoubleLeft | RPE_DoubleRight);

        //opKeyboardStart();
    }

    feStaticRectangleDraw(region);                          //draw standard rectangle

    c = colRGB( 30, 160, 190);

    //fontPrint(100,100, c, "hello 1234");

    //rect.x1 -= CM_ProgressMarginRight;

    primRectSolid2(&rect, colRGB(0, 0, 0));

    y = region->rect.y0 + OP_KBMarginTop;
    x = region->rect.x0 + OP_KBMarginLeft;

    for (i = 0; KeyTranslations[i].sendkey != 0; i++)
    {
        if (i == OP_KeysPerColumn)
        {
            y = region->rect.y0 + OP_KBMarginTop;
            x += OP_KBColumnTab;
        }

        c = FEC_ListItemStandard;//OP_KeyNameColor;
        if (i == opKeySelected)
        {
            c = FEC_ListItemSelected;//OP_KeySelectedColor;

            select.x0 = x - OP_BoxSpacingX;
            select.y0 = y - OP_BoxSpacingY;
            select.x1 = x + OP_KBBoxwidth;
            select.y1 = y - OP_BoxSpacingY + OP_KeyVertSpacing;

            primRectTranslucent2(&select, FEC_ListItemTranslucent);
            primRectOutline2(&select, 1, FEC_ListItemTranslucentBorder);
        }

        if (i == opKeyBeingDefined)
        {
            c = OP_KeyRedefiningColor;
        }

        if (TempKeyTranslations[i].highlight)
        {
            c = OP_KeyHighlightColor;
            TempKeyTranslations[i].highlight = 0;
            hl = TRUE;
        }
        else
        {
            hl = FALSE;
        }

        fontPrint(x, y, c, TempKeyTranslations[i].funcname);

        c = (hl) ? OP_KeyHighlightColor : OP_KeyPrimaryColor;
        fontPrint(x + OP_KBPrimaryKeyTab, y, c,
                  opKeyToNiceString(TempKeyTranslations[i].key1));

        c = (hl) ? OP_KeyHighlightColor : OP_KeySecondaryColor;
        fontPrint(x + OP_KBSecondaryKeyTab, y, c,
                  opKeyToNiceString(TempKeyTranslations[i].key2));

        y += OP_KeyVertSpacing;
    }

    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : opSelectKey
    Description : Callback for selecting keystrokes
    Inputs      : standard region callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
udword opSelectKey(regionhandle region, sdword ID, udword event, udword data)
{
    //sdword index = cmSelectShipType(region, mouseCursorY());
    uword column=0,row = 0;

    bitClear(data, RF_ShiftBit);

    if ((mouseCursorX() > region->rect.x0+OP_KBMarginLeft) &&
        (mouseCursorX() < region->rect.x0+OP_KBMarginLeft+OP_KBColumnTab))
    {
        column = 1;
    }
    else if ((mouseCursorX() > region->rect.x0+OP_KBMarginLeft+OP_KBColumnTab ) &&
             (mouseCursorX() < region->rect.x0+OP_KBMarginLeft+2*OP_KBColumnTab))
    {
        column = 2;
    }

#ifdef DEBUG_STOMP
    regVerify(region);
#endif

    if (column)
    {
        if ((mouseCursorY() > region->rect.y0+OP_KBMarginTop) &&
          (mouseCursorY() < region->rect.y0+OP_KBMarginTop
           + (OP_KeyVertSpacing * OP_KeysPerColumn) ))

        {
            row = (uword)((mouseCursorY() - region->rect.y0+OP_KBMarginTop)/OP_KeyVertSpacing) - 1;
            if ( event == RPE_PressLeft   )
            {                                                       //left press (select/add job)
                opKeyBeingDefined = -1;
                opKeySelected = (column - 1)*OP_KeysPerColumn + row;
                bitSet(region->status, RSF_DrawThisFrame);
            }
            else if ( event == RPE_DoubleLeft   )
            {                                                       //left press (select/add job)
                opKeyBeingDefined = (column - 1)*OP_KeysPerColumn + row;

                feScreenStart(feStack[feStackIndex].baseRegion, "Press_Key"); //popup
                opKeyDetour = 1;         //send next keypress to opDefineThisKey
                bitSet(region->status, RSF_DrawThisFrame);
                bitSet(keyboardregion->status, RSF_KeyCapture);
                keyBufferClear();
            }
        }
    }

    return 0;
}


/*-----------------------------------------------------------------------------
    Name        : opHighlightTaskFunction
    Description : Task function for higlighting
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool opHighlightBabyFunction(udword num, void* data, struct BabyCallBack* baby)
{
    sdword i;

    for (i = 0; KeyTranslations[i].sendkey > 0; i++)
    {
        TempKeyTranslations[i].highlight = 0;
    }

#ifdef DEBUG_STOMP
    regVerify(keyboardregion);
#endif

    bitSet(keyboardregion->status, RSF_DrawThisFrame);

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : opDefineThisKey
    Description : keystroke from mrKeyPress is sent here
    Inputs      : standard region callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opDefineThisKey(keyindex key)
{
    if (key == ESCKEY)
    {
        opCancelKeyDetour();
        return;
    }

    if (opValidKey(key))
    {
        opKeyAssign(opKeyBeingDefined, key);
    }

    opCancelKeyDetour();

    //highlight
    opHighlightBaby = taskCallBackRegister(opHighlightBabyFunction, 0, NULL, (real32)OP_KeyHighlightOnTime);

#ifdef DEBUG_STOMP
    regVerify(keyboardregion);
#endif

    bitSet(keyboardregion->status, RSF_DrawThisFrame);
//    bitClear(keyboardregion->status, RSF_KeyCapture);
}


/*-----------------------------------------------------------------------------
    Name        : opKeyTranslate
    Description : translate keypresses
    Inputs      : key - keyindex of key being pressed
    Outputs     :
    Return      : translated key to be fed to mrKeyPressed
-------------------------------------------------------------------------------*/
keyindex opKeyTranslate(keyindex key)
{
    udword i;

    for (i=0; KeyTranslations[i].sendkey > 0; ++i)
    {
        if (key == KeyTranslations[i].key1)
        {
            return KeyTranslations[i].sendkey;
        }

        if (key == KeyTranslations[i].key2)
        {
            return KeyTranslations[i].sendkey;
        }
    }
    //dbgMessagef("\nUntranslatable key: 0x%x", key);
    return key;

}

/*----------------------------------------------------------------------------
    Name        : opKeyAssign
    Description : assign new keystroke to a function
    Inputs      : func: function to assign, key: keyindex of key
    Outputs     :
    Return      :
-------------------------------------------------------------------------------*/
void opKeyAssign(uword func, keyindex key)
{
    uword i;
    sword oldfunc = -1;
    bool oldfuncsecondkey = FALSE;


    //find function key was previously assigned to
    for (i=0; KeyTranslations[i].sendkey > 0; ++i)
    {
        if (key == TempKeyTranslations[i].key1)
        {
            //already a primary key
            oldfunc = i;
            break;
        }

        if (key == TempKeyTranslations[i].key2)
        {
            //already a secondary key
            oldfunc = i;
            oldfuncsecondkey = TRUE;
            break;
        }
    }

    //if it's already used:

    if (oldfunc >= 0)
    {
        if (oldfuncsecondkey)
        {
            //clear it
            TempKeyTranslations[oldfunc].key2 = 0;
            TempKeyTranslations[oldfunc].highlight = OP_KeyHighlightTime;

            //new func:
            //primary becomes secondary, new becomes primary
            TempKeyTranslations[func].key2 = TempKeyTranslations[func].key1;
            TempKeyTranslations[func].key1 = key;
            TempKeyTranslations[func].highlight = OP_KeyHighlightTime;
        }
        else
        {
            //old function was primary key

            if (TempKeyTranslations[oldfunc].key2)
            {
                //if there's second key, use it
                TempKeyTranslations[oldfunc].key1 = TempKeyTranslations[oldfunc].key2 ;
                TempKeyTranslations[oldfunc].key2 = 0;
                TempKeyTranslations[oldfunc].highlight = OP_KeyHighlightTime;

                TempKeyTranslations[func].key2 = TempKeyTranslations[func].key1;
                TempKeyTranslations[func].key1 = key;
                TempKeyTranslations[func].highlight = OP_KeyHighlightTime;
                TempKeyTranslations[func].highlight = OP_KeyHighlightTime;
            }
            else
            {
                //there is no secondary key for old function
                //swap with the current key for the function being redefined
                if (TempKeyTranslations[func].key2)
                {
                    //old func gets secondary key
                    TempKeyTranslations[oldfunc].key1 = TempKeyTranslations[func].key2;
                    TempKeyTranslations[oldfunc].highlight = OP_KeyHighlightTime;
                    //new func:
                    //primary becomes secondary, new becomes primary
                    TempKeyTranslations[func].key2 = TempKeyTranslations[func].key1;
                    TempKeyTranslations[func].key1 = key;
                    TempKeyTranslations[func].highlight = OP_KeyHighlightTime;
                }
                else
                {
                    //old func gets primary key
                    TempKeyTranslations[oldfunc].key1 = TempKeyTranslations[func].key1;
                    TempKeyTranslations[oldfunc].highlight = OP_KeyHighlightTime;
                    //new func:
                    //gets no secondary, new becomes primary
                    TempKeyTranslations[func].key2 = 0;
                    TempKeyTranslations[func].key1 = key;
                    TempKeyTranslations[func].highlight = OP_KeyHighlightTime;
                }
            }
        }
    }
    else
    {
        //the key was not used already
        TempKeyTranslations[func].key2 = TempKeyTranslations[func].key1;
        TempKeyTranslations[func].key1 = key;
        TempKeyTranslations[func].highlight = OP_KeyHighlightTime;
    }
}

/*----------------------------------------------------------------------------
    Name        : opKeyToNiceString
    Description :
    Inputs      :
    Outputs     :
    Return      :
-------------------------------------------------------------------------------*/
char* opKeyToNiceString(keyindex key)
{
    sdword i;

    if (key == 0)
    {
        return "---";
    }

    for (i = 0; KeyNames[i].key > 0; i++)
    {
        if (KeyNames[i].key == key)
        {
            return KeyNames[i].name;
        }
    }

    return "[ERR]";
}

/*----------------------------------------------------------------------------
    Name        : opValidKey
    Description :
    Inputs      :
    Outputs     :
    Return      :
-------------------------------------------------------------------------------*/
bool opValidKey(keyindex key)
{
    sdword i;

    for (i = 0; KeyNames[i].key > 0; i++)
    {
        if (KeyNames[i].key == key)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void opKeyboardStart(void)
{
    sdword i;

    for (i = 0; KeyTranslations[i].sendkey > 0; i++)
    {
        TempKeyTranslations[i].key1 = KeyTranslations[i].key1;
        TempKeyTranslations[i].key2 = KeyTranslations[i].key2;
    }
}

void opKeyResetToDefault(char* name, featom* atom)
{
    sdword i;

    for (i = 0; KeyTranslations[i].sendkey > 0; i++)
    {
        TempKeyTranslations[i].key1 = DefaultKeyTranslations[i];
        TempKeyTranslations[i].key2 = 0;
    }

#ifdef DEBUG_STOMP
    regVerify(keyboardregion);
#endif

    bitSet(keyboardregion->status, RSF_DrawThisFrame);
}


char UpperHex2Char(uword num)
{
    uword x;

    x = num >> 4;
    if (x>=10)
        return ('A' + x-10);
    else
        return ('0' + x);

}

char LowerHex2Char(uword num)
{
    uword x;

    x = num & 15;
    if (x>=10)
        return ('A' + x-10);
    else
        return ('0' + x);

}

uword Char2Hex(char ch)
{
    if (ch >= 'A')
        return (ch - 'A' + 10);
    else
        return (ch - '0');

}

char ConvertSliderToAIPlayerDifficulty(sdword sliderval)
{
    if (sliderval <= 33)
    {
        return AI_BEG;
    }

    if (sliderval <= 66)
    {
        return AI_INT;
    }

    return AI_ADV;
}

bool opResHackSupported(void)
{
    rmode* mode;
    int width, height, depth;

    width  = opSaveMAIN_WindowWidth;
    height = opSaveMAIN_WindowHeight;
    depth  = opSaveMAIN_WindowDepth;

    mode = opSaveRndSelected->dev->modes;
    while (mode != NULL)
    {
        if (mode->width == width &&
            mode->height == height &&
            mode->depth == depth)
        {
            return TRUE;
        }
        mode = mode->next;
    }

    return FALSE;
}

bool opResSupported(sdword index)
{
    opres* res;
    rdevice* dev;
    rmode* mode;
    int width, height, depth;

    res = &opRes[index];
    dev = opRnd[opRenderCurrentSelected].dev;

    width  = res->width;
    height = res->height;
    depth  = res->depth;

    mode = dev->modes;
    while (mode != NULL)
    {
        if (mode->width == width &&
            mode->height == height &&
            mode->depth == depth)
        {
            return TRUE;
        }
        mode = mode->next;
    }

    return FALSE;
}

bool opResResSupported(opres* res)
{
    rdevice* dev;
    rmode* mode;
    int width, height, depth;

    dev = opRndSelected->dev;

    width  = res->width;
    height = res->height;
    depth  = res->depth;

    mode = dev->modes;
    while (mode != NULL)
    {
        if (mode->width == width &&
            mode->height == height &&
            mode->depth == depth)
        {
            return TRUE;
        }
        mode = mode->next;
    }

    return FALSE;
}

bool opResChanged(void)
{
    if (opSaveMAIN_WindowWidth  != MAIN_WindowWidth ||
        opSaveMAIN_WindowHeight != MAIN_WindowHeight ||
        opSaveMAIN_WindowDepth  != MAIN_WindowDepth)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void opGLCStop(void)
{
    if (glcActive())
    {
        (void)glcActivate(FALSE);
    }
}

static void opGLCStart(void)
{
    if (RGLtype != SWtype)
    {
        (void)glcActivate(TRUE);
    }
    utyForceTopmost(fullScreen);
}

void opModeswitchFailed(void)
{
    GeneralMessageBox(strGetString(strOPFailed0),
                      strGetString(strOPFailed1));
    opGLCStart();
}

void opCountdownYes(char* name, featom* atom)
{
    opTimerActive = FALSE;
    feScreenDisappear(NULL, NULL);
    opGLCStart();
    if (strcasecmp(GLC_RENDERER, GENERIC_OPENGL_RENDERER) == 0)
    {
        GeneralMessageBox(strGetString(strGDIGeneric0),
                          strGetString(strGDIGeneric1));
    }
}

extern GLboolean glDLL3Dfx;
bool opWasUsing3DfxGL;

void opCountdownNo(char* name, featom* atom)
{
    opTimerActive = FALSE;
    if (name != NULL || atom != NULL)
    {
        feScreenDisappear(NULL, NULL);
    }
    soundEventShutdown();
    opGLCStop();
    mainShutdownRenderer();
    opUsing3DfxGL = opWasUsing3DfxGL;
    gDevcaps  = opOldDevcaps;
    gDevcaps2 = opOldDevcaps2;
    mainRestoreRender();
    opGLCStart();
    soundEventRestart();
    opDeviceIndex = opOldDeviceIndex;
}

void opTimerExpired(void)
{
    feAllScreensDelete();
    feScreenStart(ghMainRegion, "Main_game_screen");
    opCountdownNo(NULL, NULL);
}

void opCountdownBoxDraw(featom* atom, regionhandle region)
{
    fonthandle oldfont;
    char* str;
    sdword x, y, width, height;
    color c;
    extern fonthandle ghDefaultFont;

    oldfont = fontMakeCurrent(ghDefaultFont);

    x = region->rect.x0;
    y = region->rect.y0;
    width  = region->rect.x1 - region->rect.x0;
    height = 0;
    c = colRGB(255,200,0);

    str = strGetString(strOPCountdown0);
    fontPrintf(x + (width - fontWidth(str))/2, y + height, c, str);
    height += fontHeight(str);

    str = strGetString(strOPCountdown1);
    fontPrintf(x + (width - fontWidth(str))/2, y + height, c, str);
    height += fontHeight(str);

    str = strGetString(strOPCountdown2);
    fontPrintf(x + (width - fontWidth(str))/2, y + height, c, str, (sdword)opTimerLength - 2);

    fontMakeCurrent(oldfont);
}

void opCountdownBoxStart(void)
{
    opTimerStart  = taskTimeElapsed;
    opTimerActive = TRUE;
    feScreenStart(ghMainRegion, "Countdown_Message_Box");
}

void opOptionsAcceptHelper(char* name, featom* atom, char* linkName)
{
    udword i, temp;
    oprender* rnd;

    //copy temporary changed data to permanent storage

    //copy keyboard options

    for (i=0; KeyTranslations[i].sendkey > 0; ++i)
    {
        KeyTranslations[i].key1 = TempKeyTranslations[i].key1;
        KeyTranslations[i].key2 = TempKeyTranslations[i].key2;

        //also save for next time
        opKeymap[i*4]   = UpperHex2Char(KeyTranslations[i].key1);
        opKeymap[i*4+1] = LowerHex2Char(KeyTranslations[i].key1);
        opKeymap[i*4+2] = UpperHex2Char(KeyTranslations[i].key2);
        opKeymap[i*4+3] = LowerHex2Char(KeyTranslations[i].key2);
    }


    //audio - don't copy since we're doing live updates


    //video - copy selected stuff
    if (opResListWindow != NULL &&
        opResListWindow->CurLineSelected != NULL)
    {
        for (i = 0; i < opResNumber; i++)
        {
            if (opResListWindow->CurLineSelected->data == (ubyte*)&opRes[i])
            {
                opResCurrentSelected = i;
                mainWindowWidth  = opRes[i].width;
                mainWindowHeight = opRes[i].height;
                mainWindowDepth  = opRes[i].depth;
                break;
            }
        }
    }

    if (opRenderListWindow != NULL &&
        opRenderListWindow->CurLineSelected != NULL)
    {
        for (i = 0; i < opRenderNumber; i++)
        {
            if (opRenderListWindow->CurLineSelected->data == (ubyte*)&opRnd[i])
            {
                opRenderCurrentSelected = i;
                break;
            }
        }
    }

    opOptionsEndHelper(linkName);

    if (gameIsRunning)
    {
        return;     // don't allow render switches in game!
    }

    if (opRnd == NULL)
    {
        return;
    }

    //rswitch

    rnd = opSaveRndSelected;

    mainWindowWidth  = opSaveMAIN_WindowWidth;
    mainWindowHeight = opSaveMAIN_WindowHeight;
    mainWindowDepth  = opSaveMAIN_WindowDepth;

    opReloading = TRUE;
    opOldDeviceIndex = opDeviceIndex;
    opOldDevcaps  = gDevcaps;
    opOldDevcaps2 = gDevcaps2;

    opWasUsing3DfxGL = glDLL3Dfx;

    if (rnd->type == RIN_TYPE_DIRECT3D)
    {
        if ((RGLtype != D3Dtype) ||
            opResChanged() ||
            (strcmp(rnd->data, lastDev) != 0))
        {
            if (opResHackSupported())
            {
                soundEventShutdown();
                mainSaveRender();
                opGLCStop();

                MAIN_WindowWidth  = opSaveMAIN_WindowWidth;
                MAIN_WindowHeight = opSaveMAIN_WindowHeight;
                MAIN_WindowDepth  = opSaveMAIN_WindowDepth;

                opDevcaps  = gDevcaps;
                opDevcaps2 = gDevcaps2;
                gDevcaps  = rnd->dev->devcaps;
                gDevcaps2 = rnd->dev->devcaps2;
                if (mainLoadParticularRGL("d3d", rnd->data))
                {
                    memStrncpy(lastDev, rnd->data, 63);
                    opDeviceIndex = opRenderCurrentSelected;
                    opCountdownBoxStart();
                }
                else
                {
                    gDevcaps  = opDevcaps;
                    gDevcaps2 = opDevcaps2;
                    mainRestoreRender();
                    opModeswitchFailed();
                }
                soundEventRestart();
                opGLCStart();
            }
        }
    }
    else if (rnd->type == RIN_TYPE_OPENGL)
    {
        if (RGLtype != GLtype || opResChanged() || op3DfxChanged ||
            (opDeviceIndex != opRenderCurrentSelected))
        {
            if (opResHackSupported())
            {
                soundEventShutdown();
                mainSaveRender();
                opGLCStop();

                if (strcasecmp(rnd->name, "3dfx OpenGL") == 0)
                {
                    opUsing3DfxGL = TRUE;
                }
                else
                {
                    opUsing3DfxGL = FALSE;
                }

                MAIN_WindowWidth  = opSaveMAIN_WindowWidth;
                MAIN_WindowHeight = opSaveMAIN_WindowHeight;
                MAIN_WindowDepth  = opSaveMAIN_WindowDepth;

                opDevcaps  = gDevcaps;
                opDevcaps2 = gDevcaps2;
                gDevcaps  = rnd->dev->devcaps;
                gDevcaps2 = rnd->dev->devcaps2;
                if (mainLoadGL(rnd->data))
                {
                    opDeviceIndex = opRenderCurrentSelected;
                    opCountdownBoxStart();
                }
                else
                {
                    gDevcaps  = opDevcaps;
                    gDevcaps2 = opDevcaps2;
                    opUsing3DfxGL = opWasUsing3DfxGL;
                    mainRestoreRender();
                    opModeswitchFailed();
                }

                opUsing3DfxGL = FALSE;
                soundEventRestart();
                SDL_Delay(20);
                opGLCStart();
            }
        }
    }
    else
    {
        if (RGLtype != SWtype || opResChanged())
        {
            if (opResHackSupported())
            {
                soundEventShutdown();
                mainSaveRender();
                opGLCStop();

                MAIN_WindowWidth  = opSaveMAIN_WindowWidth;
                MAIN_WindowHeight = opSaveMAIN_WindowHeight;
                MAIN_WindowDepth  = opSaveMAIN_WindowDepth;

                opDevcaps  = gDevcaps;
                opDevcaps2 = gDevcaps2;
                gDevcaps  = rnd->dev->devcaps;
                gDevcaps2 = rnd->dev->devcaps2;
                if (mainLoadParticularRGL("sw", ""))
                {
                    opDeviceIndex = opRenderCurrentSelected;
                    opCountdownBoxStart();
                }
                else
                {
                    gDevcaps  = opDevcaps;
                    gDevcaps2 = opDevcaps2;
                    mainRestoreRender();
                    opModeswitchFailed();
                }
                soundEventRestart();
                opGLCStart();
            }
        }
    }

    opReloading = FALSE;

    alodGetTargetPolys(&opDetailThresholdVal, &temp);
    opDetailThresholdVal /= OP_DETAIL_DIVISOR;

    //update saved settings
    opOptionsSaveSettings();
}

void opOptionsAccept(char* name, featom* atom)
{
    opOptionsAcceptHelper(name, atom, "Main_game_screen");
}

void opInGameOptionsAccept(char* name, featom* atom)
{
    opOptionsAcceptHelper(name, atom, "Game_options");
}


void opKeyboardLoad(void)
{
    udword i;


    if (opKeymap[0] != 'Z' )
    {
        dbgMessage("\nKeymap not found");
        for (i=0; KeyTranslations[i].sendkey > 0; ++i)
        {
            KeyTranslations[i].key1 = Char2Hex(opKeymap[i*4])*16 + Char2Hex(opKeymap[i*4+1]);
            KeyTranslations[i].key2 = Char2Hex(opKeymap[i*4+2])*16 + Char2Hex(opKeymap[i*4+3]);
        }

    }
    // else fine - they're already initialized

}

real32 normalizeSmoothie(sdword val)
{
    real32 fval;

    fval = ((real32)val * 1.025f) / 100.0f;
    if (fval < 0.0f) fval = 0.0f;
    if (fval > 1.0f) fval = 1.0f;

    return fval;
}

void opUpdateAudioSettings()
{
    sdword min, max;

    opEQHelper();

    soundEventVocalSettings(opVoiceComm, opVoiceStat, opVoiceChat);

    soundEventMasterEQ((real32*)opEQReal);

    soundEventMusicMasterVol(normalizeSmoothie(opMusicVol));
    soundEventSFXMasterVol(normalizeSmoothie(opSFXVol));
    soundEventSpeechMasterVol(normalizeSmoothie(opSpeechVol));

    soundGetVoiceLimits(&min, &max);
    soundSetNumVoices(opNumChannels + min, opAutoChannel);

    soundMixerSetMode(opSoundQuality);
}

void opRestoreSavedEqualizerSettings()
{
    sdword i;

    opSpeakerSetting = opSaveSpeakerSetting;

    for (i = 0; i < NUM_EQ_BANDS; i++)
    {
        opEqualizerSettings[i] = opSaveEqualizerSettings[i];
        opEQReal[i] = 1.0f - ((real32)opEqualizerSettings[i] / 100.0f);
    }

    soundEventMasterEQ((real32*)opEQReal);
}

void opRestoreSavedCustomEffectsSettings()
{
    opEffectsVal = opSaveEffectsVal;
    texLinearFiltering = savetexLinearFiltering;
    showBackgrounds = saveshowBackgrounds;
    smInstantTransition = savesmInstantTransition;
    enableSmoothing = saveenableSmoothing;
    enableStipple = saveenableStipple;
    enableTrails = saveenableTrails;
    smFuzzyBlobs = savesmFuzzyBlobs;

    etgHitEffectsEnabled = saveHitEffects;
    etgDamageEffectsEnabled = saveDamagEffects;
    etgFireEffectsEnabled = saveMuzzleEffects;
    etgBulletEffectsEnabled = saveBulletEffects;
}

void opRestoreSavedSettings()
{
    //copy audio back

    opMusicVol =     opSaveMusicVol;
    opSFXVol =       opSaveSFXVol;
    opSpeechVol =    opSaveSpeechVol;
    opNumChannels =  opSaveNumChannels;
    opAutoChannel =  opSaveAutoChannel;
    opSoundQuality = opSaveSoundQuality;
//    opSpeakerSetting =  opSaveSpeakerSetting;

    opRestoreSavedEqualizerSettings();

    opUpdateAudioSettings();

    //video
    opNoPalMB = opSaveNoPalMB;
    trNoPalResizePool(opNoPalMB);
    opDetailThresholdVal = opSaveDetailThresholdVal;
    opSetDetailThreshold();
    opBrightnessVal = opSaveBrightnessVal;
    shGammaSet(opBrightnessVal);
    opNoLODVal = opSaveNoLODVal;
    opNumEffects = opSaveNumEffects;
    etgHistoryScalar = opNumEffects + etgHistoryScalarMin;

    opRestoreSavedCustomEffectsSettings();

    //gameplay
    opMouseSens = opSaveMouseSens;
    opInfoOverlayVar = opSaveInfoOverlay;
    if (opInfoOverlayVar)
        ioEnable();
    else
        ioDisable();

    cameraSensitivitySet(opMouseSens);

    opAutodockHealthCheck = opSaveAutodockHealthCheck;
    opAutodockFuelCheck = opSaveAutodockFuelCheck;
    opAutodockHealthVal = opSaveAutodockHealthVal;
    opAutodockFuelVal = opSaveAutodockFuelVal;
}

void opOptionsCancelHelper(char* name, featom* atom, char* linkName)
{
    opRestoreSavedSettings();
    opOptionsEndHelper(linkName);
    kbRestoreSavedSettings();
}

void opOptionsCancel(char* name, featom* atom)
{
    opOptionsCancelHelper(name, atom, "Main_game_screen");
}

void opInGameOptionsCancel(char* name, featom* atom)
{
    opOptionsCancelHelper(name, atom, "Game_options");
}

void opEqualizerToAudioAccept(char* name, featom* atom)
{
    if (opEqualizerToggled)
    {
        opSpeakerSetting = 3;
    }
    opOptionsSaveEqualizerSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Audio_Options");
}

void opEqualizerToAudioCancel(char* name, featom* atom)
{
    opRestoreSavedEqualizerSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Audio_Options");
}

void opEqualizerToInGameAudioAccept(char* name, featom* atom)
{
    if (opEqualizerToggled)
    {
        opSpeakerSetting = 3;
    }
    opOptionsSaveEqualizerSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Audio");
}

void opEqualizerToInGameAudioCancel(char* name, featom* atom)
{
    opRestoreSavedEqualizerSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Audio");
}

void opAdvancedToAudioAccept(char* name, featom* atom)
{
    speechEventCleanup();   // make sure we shut off and fade out and speech playing from the hear buttons
    opOptionsSaveAdvancedSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Audio_Options");
}

void opAdvancedToAudioCancel(char* name, featom* atom)
{
    speechEventCleanup();   // make sure we shut off and fade out and speech playing from the hear buttons
    opRestoreAdvancedSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Audio_Options");
}

void opAdvancedToInGameAudioAccept(char* name, featom* atom)
{
    speechEventCleanup();   // make sure we shut off and fade out and speech playing from the hear buttons
    opOptionsSaveAdvancedSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Audio");
}

void opAdvancedToInGameAudioCancel(char* name, featom* atom)
{
    speechEventCleanup();   // make sure we shut off and fade out and speech playing from the hear buttons
    opRestoreAdvancedSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Audio");
}

void opEffectsToVideoAccept(char* name, featom* atom)
{
    if (opCustomEffectsToggled)
    {
        opEffectsVal = 2;
    }
    opOptionsSaveCustomEffectsSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Video_Options");
}

void opEffectsToVideoCancel(char* name, featom* atom)
{
    opRestoreSavedCustomEffectsSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Video_Options");
}

void opEffectsToInGameVideoAccept(char* name, featom* atom)
{
    if (opCustomEffectsToggled)
    {
        opEffectsVal = 2;
    }
    opOptionsSaveCustomEffectsSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Video");
}

void opEffectsToInGameVideoCancel(char* name, featom* atom)
{
    opRestoreSavedCustomEffectsSettings();
    feScreenDisappear(NULL, NULL);
    feScreenStart(ghMainRegion, "Video");
}

#if(0)
void VolumeSmoothieCallback(real32 data, Smoothie* smoo)
{
    sdword val = ((sliderhandle)volumeregion)->value = (sdword)volumesmoothie.buffer;
    bitSet(volumeregion->status, RSF_DrawThisFrame);

    soundEventMusicVol(normalizeSmoothie(val));
    opMusicVol = val;
}

void SFXSmoothieCallback(real32 data, Smoothie* smoo)
{
    sdword val = ((sliderhandle)sfxregion)->value = (sdword)sfxsmoothie.buffer;
    bitSet(sfxregion->status, RSF_DrawThisFrame);

    soundEventSFXVol(normalizeSmoothie(val));
    opSFXVol = val;
}

void SpeechSmoothieCallback(real32 data, Smoothie* smoo)
{
    sdword val = ((sliderhandle)speechregion)->value = (sdword)speechsmoothie.buffer;
    bitSet(speechregion->status, RSF_DrawThisFrame);

    soundEventSpeechVol(normalizeSmoothie(val));
    opSpeechVol = val;
}

void EqualizerSmoothieCallback(real32 data, Smoothie* smoo)
{
    sdword i;

    ((sliderhandle)(smoo->user))->value = (sdword)data;
    bitSet(((region*)(smoo->user))->status, RSF_DrawThisFrame);

    for (i = 0; i < NUM_EQ_BANDS; i++)
    {
        opEQReal[i] = 1.0f - ((real32)opEqualizerSettings[i] / 100.0f);
    }
}
#endif

/*
void soundEventSetActor(sdword actornum, bool bOn);
    actornum is 1, 2 or 3
    bOn is TRUE if checked FALSE if not
void soundEventVocalSettings(bool bCommandsOn, bool bStatusOn, bool bChatterOn);
 */

void opOptionsSaveEqualizerSettings(void)
{
    sdword i;

    opSaveSpeakerSetting = opSpeakerSetting;

    for (i = 0; i < NUM_EQ_BANDS; i++)
    {
        opSaveEqualizerSettings[i] = opEqualizerSettings[i];
        opEQReal[i] = 1.0f - ((real32)opEqualizerSettings[i] / 100.0f);
    }

    soundEventMasterEQ((real32*)opEQReal);
}

void opOptionsSaveAdvancedSettings(void)
{
    opSaveVoiceSetting = opVoiceSetting;
    opSaveVoiceComm = opVoiceComm;
    opSaveVoiceStat = opVoiceStat;
    opSaveVoiceChat = opVoiceChat;
    opSaveVoice0On = opVoice0On;
    opSaveVoice1On = opVoice1On;
    opSaveVoice2On = opVoice2On;

    opSaveBattleChatter = opBattleChatter;
    battleChatterFrequencySet(opBattleChatter);
}

void opRestoreAdvancedSettings(void)
{
    opVoiceSetting = opSaveVoiceSetting;
    opVoiceComm = opSaveVoiceComm;
    opVoiceStat = opSaveVoiceStat;
    opVoiceChat = opSaveVoiceChat;
    opVoice0On  = opSaveVoice0On;
    opVoice1On  = opSaveVoice1On;
    opVoice2On  = opSaveVoice2On;

    opBattleChatter = opSaveBattleChatter;
    battleChatterFrequencySet(opBattleChatter);
}

void opOptionsSaveCustomEffectsSettings(void)
{
    opSaveEffectsVal = opEffectsVal;
    savetexLinearFiltering = texLinearFiltering;
    saveshowBackgrounds = showBackgrounds;
    savesmInstantTransition = smInstantTransition;
    saveenableSmoothing = enableSmoothing;
    saveenableStipple = enableStipple;
    saveenableTrails = enableTrails;
    savesmFuzzyBlobs = smFuzzyBlobs;

    saveHitEffects = (sdword)etgHitEffectsEnabled;
    saveDamagEffects = (sdword)etgDamageEffectsEnabled;
    saveMuzzleEffects = (sdword)etgFireEffectsEnabled;
    saveBulletEffects = (sdword)etgBulletEffectsEnabled;
}

void opOptionsSaveSettings(void)
{
    udword i;

    //copy keyboard options
    for (i = 0; KeyTranslations[i].sendkey > 0; i++)
    {
        TempKeyTranslations[i].key1 = KeyTranslations[i].key1;
        TempKeyTranslations[i].key2 = KeyTranslations[i].key2;
    }

    //copy audio options
    opSaveMusicVol = opMusicVol;
    opSaveSFXVol = opSFXVol;
    opSaveSpeechVol = opSpeechVol;
    opSaveNumChannels = opNumChannels;
    opSaveAutoChannel = opAutoChannel;
    opSaveSoundQuality = opSoundQuality;
//    opSaveSpeakerSetting = opSpeakerSetting;

    opOptionsSaveAdvancedSettings();

    opOptionsSaveEqualizerSettings();

    opUpdateAudioSettings();

    //video
    opSaveNoPalMB = opNoPalMB;
    alodGetTargetPolys(&opDetailThresholdVal, &i);
    opDetailThresholdVal /= OP_DETAIL_DIVISOR;
    opSaveDetailThresholdVal = opDetailThresholdVal;
    opBrightnessVal = shGammaGet();
    opSaveBrightnessVal = opBrightnessVal;
    opSaveNoLODVal = opNoLODVal;
    opSaveNumEffects = opNumEffects;

    opSaveMAIN_WindowWidth  = MAIN_WindowWidth;
    opSaveMAIN_WindowHeight = MAIN_WindowHeight;
    opSaveMAIN_WindowDepth  = MAIN_WindowDepth;

    opRenderListLoad();

    opSaveRndSelected = opRndSelected;

    opSaveMouseSens = opMouseSens;
    opSaveInfoOverlay = opInfoOverlayVar;

    opOptionsSaveCustomEffectsSettings();

    //gameplay

    opSaveAutodockHealthCheck = opAutodockHealthCheck;
    opSaveAutodockFuelCheck = opAutodockFuelCheck;
    opSaveAutodockHealthVal = opAutodockHealthVal;
    opSaveAutodockFuelVal = opAutodockFuelVal;

    kbSaveSettings();

    // save settings to .cfg file
    utyOptionsFileWrite();
    if (DebugWindow)
    {
        if (utyTest(SSA_DebugWindow))
        {
            /* dbw*() functions in other parts of code (main.c, utility.c, and
               Debug.c) will need to be uncommented if the debug window is
               reenabled. */
            /*dbwWriteOptions();*/
        }
    }
}

void opOptionsStartHelper(char* name, featom* atom, char* linkName)
{
    opOptionsSaveSettings();

/*    smoothmusicvol = (real32)opMusicVol;
    smoothsfxvol = (real32)opSFXVol;
    smoothspeechvol = (real32)opSpeechVol;

    InitSmoothie(&volumesmoothie,CONTROL_FEEDBACK,&smoothmusicvol,NULL,
                 &opMusicVol,&VolumeSmoothieCallback,1.1f);
    InitSmoothie(&sfxsmoothie,CONTROL_FEEDBACK, &smoothsfxvol,NULL,
        &opSFXVol, &SFXSmoothieCallback, 1.1f);
    InitSmoothie(&speechsmoothie, CONTROL_FEEDBACK, &smoothspeechvol, NULL,
        &opSpeechVol, &SpeechSmoothieCallback, 1.1f);

    smoothiesactive = TRUE;
    //in mousedraw
    //opSmoothiesBaby = taskCallBackRegister(opSmoothiesBabyFunction, 0, NULL, (real32)OP_SMOOTHIE_RATE);*/

    feScreenDisappear(NULL,NULL);

    feScreenStart(ghMainRegion, linkName);
}

void opOptionsStart(char* name, featom* atom)
{
    opOptionsStartHelper(name, atom, "Audio_Options");
}

void opInGameOptionsStart(char* name, featom* atom)
{
    opOptionsStartHelper(name, atom, "Game_options");
}

void opOptionsEndHelper(char* linkName)

{
#if(0)
    RemoveSmoothie(&volumesmoothie);
    RemoveSmoothie(&sfxsmoothie);
    RemoveSmoothie(&speechsmoothie);
#endif

//    smoothiesactive = FALSE;

    feScreenDisappear(NULL,NULL);

    feScreenStart(ghMainRegion, linkName);
}

void opOptionsEnd(void)
{
    opOptionsEndHelper("Main_game_screen");
}

void opCancelKeyDetour(void)
{
    opKeyBeingDefined = -1;
    opKeyDetour = 0;
    feScreenDisappear(NULL, NULL);
}


void opKeyCancel(char* name, featom* atom)
{
    opCancelKeyDetour();
}


void opMusicVolume(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;

    if (FEFIRSTCALL(atom))
    {
       //feToggleButtonSet(name, texLinearFiltering);
       dbgMessage("\nopMusicVolume() first call");
//       volumeregion = atom->region;
#ifdef DEBUG_STOMP
       regVerify(((regionhandle)atom->region));
#endif
       shandle = (sliderhandle)atom->region;
       shandle->maxvalue = 100;
       shandle->value = opMusicVol;
//       smoothmusicvol = (real32)opMusicVol;

       f = regFilterSet(atom->region, 0);
       regFilterSet(atom->region, f | RPE_HoldLeft);

       shandle->processFunction  = opMusicVolumeProcess;
       opMusicVolumeProcess(atom->region, 0, 0, 0);

//       shandle->processFunction  = opMusicVolumeProcess;

//       AddSmoothie(&volumesmoothie);
    }
    else // if (FELASTCALL(atom))
    {
        ;//RemoveSmoothie(&volumesmoothie);
    }
}

void opSFXVolume(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;

    if (FEFIRSTCALL(atom))
    {
        //feToggleButtonSet(name, texLinearFiltering);
        dbgMessage("\nopSFXVolume() first call");
//        sfxregion = atom->region;
#ifdef DEBUG_STOMP
        regVerify(((regionhandle)atom->region));
#endif
        shandle = (sliderhandle)atom->region;
        shandle->maxvalue = 100;
        shandle->value = opSFXVol;
//        smoothsfxvol = (real32)opSFXVol;

        f = regFilterSet(atom->region, 0);
        regFilterSet(atom->region, f | RPE_HoldLeft);

        shandle->processFunction  = opSFXVolumeProcess;
        opSFXVolumeProcess(atom->region, 0, 0, 0);

//        AddSmoothie(&sfxsmoothie);
    }
    else //if (FELASTCALL(atom))
    {
        ;//RemoveSmoothie(&sfxsmoothie);
    }
}

void opSpeechVolume(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;

    if (FEFIRSTCALL(atom))
    {
        //feToggleButtonSet(name, texLinearFiltering);
        dbgMessage("\nopSpeechVolume() first call");
//        speechregion = atom->region;
#ifdef DEBUG_STOMP
        regVerify(((regionhandle)atom->region));
#endif
        shandle = (sliderhandle)atom->region;
        shandle->maxvalue = 100;
        shandle->value = opSpeechVol;
//        smoothspeechvol = (real32)opSpeechVol;

        f = regFilterSet(atom->region, 0);
        regFilterSet(atom->region, f | RPE_HoldLeft);

        shandle->processFunction  = opSpeechVolumeProcess;

        opSpeechVolumeProcess(atom->region, 0, 0, 0);

//        AddSmoothie(&speechsmoothie);
    }
    else //if (FELASTCALL(atom))
    {
        ;//RemoveSmoothie(&speechsmoothie);
    }
}

/*void opBattleChatterCB(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;
    if (FEFIRSTCALL(atom))
    {
       //feToggleButtonSet(name, texLinearFiltering);
       //dbgMessage("\nopMusicVolume() first call");
       shandle = (sliderhandle)atom->region;
       shandle->maxvalue = 100;
       shandle->value = opBattleChatter;

       f = regFilterSet(atom->region, 0);
       regFilterSet(atom->region, f | RPE_HoldLeft);

       shandle->processFunction  = opBattleChatterProcess;
       opBattleChatterProcess(atom->region, 0, 0, 0);
    }
    else
    {
        ;
    }

}*/


void opNumberChannels(char* name, featom* atom)
{
    udword f;
    sdword min, max;
    sliderhandle shandle;

    if (FEFIRSTCALL(atom))
    {
        soundGetVoiceLimits(&min, &max);

        shandle = (sliderhandle)atom->region;
        shandle->maxvalue = max - min;
        shandle->value = opNumChannels;

        f = regFilterSet(atom->region, 0);
        regFilterSet(atom->region, f | RPE_HoldLeft);

        shandle->processFunction  = opNumChannelsProcess;
        opNumChannelsProcess(atom->region, 0, 0, 0);
    }
    else
    {
        ;
    }
}

void opDrawNumChannels(featom *atom, regionhandle region)
{
    fonthandle currentFont;
    sdword min, max;

    if (FELASTCALL(atom))
    {
        drawnumchannels = NULL;
        return;
    }

    drawnumchannels = region;

    currentFont = fontMakeCurrent(opKeyboardFont);

    primRectSolid2(&region->rect, colBlack);

    soundGetVoiceLimits(&min, &max);

    fontPrintf (region->rect.x0, region->rect.y0, colRGB(255,200,0), "%u", opNumChannels + min);

    fontMakeCurrent(currentFont);
}

void opAutoChannels(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opAutoChannel);
    }
    else
    {
        opAutoChannel = FECHECKED(atom) ? SOUND_MODE_AUTO : SOUND_MODE_NORM;
    }
}

udword opMusicVolumeProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;

    opMusicVol = shandle->value;
    soundEventMusicMasterVolNOW(normalizeSmoothie(opMusicVol));

    return (0);
}

udword opSFXVolumeProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;

    opSFXVol = shandle->value;
    soundEventSFXMasterVol(normalizeSmoothie(opSFXVol));

    return (0);
}

udword opSpeechVolumeProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;

    opSpeechVol = shandle->value;
    soundEventSpeechMasterVol(normalizeSmoothie(opSpeechVol));

    return (0);
}

udword opNumChannelsProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;
    sdword min, max;

    opNumChannels = shandle->value;

  //  shandle->value = (sdword)nchannelssmoothie.buffer;

    if (drawnumchannels != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(((regionhandle)drawnumchannels));
#endif

        bitSet(drawnumchannels->status, RSF_DrawThisFrame);
    }

    soundGetVoiceLimits(&min, &max);
    soundSetNumVoices(opNumChannels + min, opAutoChannel);

    return (0);
}

udword opMouseSensitivityProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    //dbgMessage(">");
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;

    opMouseSens = shandle->value;
    cameraSensitivitySet(opMouseSens);
    return (0);
}

udword opBattleChatterProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    //dbgMessage(">");
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;

    opBattleChatter = shandle->value;
    battleChatterFrequencySet(opBattleChatter);
    return (0);
}

udword opNumEffectsProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    //dbgMessage(">");
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;

    opNumEffects = shandle->value;
#ifdef ddunlop
    dbgMessagef("Number of Effects: %u\n", opNumEffects + etgHistoryScalarMin);
#endif
    etgHistoryScalar = opNumEffects + etgHistoryScalarMin;
    return (0);
}

udword opCPUDifficultyProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    featom* atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;
    tpGameCreated.aiplayerDifficultyLevel = shandle->value;
    return (0);
}

udword opCPUAttacksProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    featom* atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;
    tpGameCreated.aiplayerBigotry = shandle->value;
    return (0);
}

udword opEqualizerProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    sdword val, i;
    featom* atom = reg->atom;
    real32 EQ[NUM_EQ_BANDS];
    sliderhandle shandle = (sliderhandle)atom->region;

    val = (sdword)((real32)shandle->value * 1.025f);

    opEqualizerSettings[shandle->ID] = val;
//    smootheqvalue[shandle->ID] = (real32)val;
//    shandle->value = (sdword)smootheqsmoothie[shandle->ID].buffer;

    opEqualizerToggled = TRUE;

    for (i = 0; i < NUM_EQ_BANDS; i++)
    {
        EQ[i] = 1.0f - ((real32)opEqualizerSettings[i] / 100.0f);
    }

    soundEventMasterEQ((real32*)EQ);

    return (0);
}

void opEQHelper(void)
{
    sdword i;

    if (opSpeakerSetting >= 0 &&
        opSpeakerSetting < 3)
    {
        for (i = 0; i < NUM_EQ_BANDS; i++)
        {
            opEqualizerSettings[i] = opSpeakerSettings[opSpeakerSetting][i];
        }
    }
    else
    {
        opSpeakerSetting = 3;
    }

    for (i = 0; i < NUM_EQ_BANDS; i++)
    {
        opSaveEqualizerSettings[i] = opEqualizerSettings[i];
        opEQReal[i] = 1.0f - ((real32)opEqualizerSettings[i] / 100.0f);
    }

    soundEventMasterEQ((real32*)opEQReal);
}

void opSpeaker(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, opSpeakerSetting);
    }
    else
    {
        opSpeakerSetting = (sdword)atom->pData;
    }

    opEQHelper();
}

void opSoundQualitySet(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, opSoundQuality);
    }
    else
    {
        opSoundQuality = (sdword)atom->pData;
    }

    soundMixerSetMode(opSoundQuality);
}

void opAdvancedButton(char* name, featom* atom)
{
}

void opEqualizerButton(char* name, featom* atom)
{
    feScreenStart(feStack[feStackIndex].baseRegion, "Equalizer"); //popup
}


udword opDetailThresholdProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;
    opDetailThresholdVal = shandle->value + 50;
    opSetDetailThreshold();
    return (0);
}


void opLighting(char* name, featom* atom)
{
    region *reg;
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, opLightingVal);
        reg = atom->region;
#ifdef DEBUG_STOMP
        regVerify(((regionhandle)reg));
#endif
        bitSet(reg->status, RSF_DrawThisFrame);
    }
    else
    {
        opLightingVal = (sdword)atom->pData;
    }
}

void opEffectsHelper(void)
{
    switch (opEffectsVal)
    {
    case 0: //hi
        texLinearFiltering = TRUE;
        showBackgrounds = TRUE;
        enableSmoothing = TRUE;
        enableStipple = FALSE;
        smFuzzyBlobs = TRUE;
        enableTrails = TRUE;
        smInstantTransition = FALSE;
        opNumEffects = 256 - etgHistoryScalarMin;
        if (EffectsSlider != NULL)
        {
#ifdef DEBUG_STOMP
            regVerify(((regionhandle)EffectsSlider));
#endif
            EffectsSlider->value = opNumEffects;
            bitSet(((regionhandle)EffectsSlider)->status, RSF_DrawThisFrame);
        }

        break;

    case 1: //lo
        texLinearFiltering = FALSE;
        showBackgrounds = FALSE;
        enableSmoothing = TRUE;
        enableStipple = TRUE;
        smFuzzyBlobs = FALSE;
        enableTrails = TRUE;
        smInstantTransition = TRUE;
        opNumEffects = 0;
        if (EffectsSlider != NULL)
        {
#ifdef DEBUG_STOMP
            regVerify(((regionhandle)EffectsSlider));
#endif
            EffectsSlider->value = opNumEffects;
            bitSet(((regionhandle)EffectsSlider)->status, RSF_DrawThisFrame);
        }
        break;

    case 2: //custom
        break;

    default:
        opEffectsVal = 2;   //in case someone edits their config file
    }

    trFilterEnable(texLinearFiltering);
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

void opUpdateVideoSettings(void)
{
    opEffectsHelper();
    opSetDetailThreshold();
    shGammaSet(opBrightnessVal);
    trNoPalResizePool(opNoPalMB);
}

void opEffects(char* name, featom* atom)
{
    region* reg;

    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, opEffectsVal);
        reg = atom->region;
#ifdef DEBUG_STOMP
        regVerify(((regionhandle)reg));
#endif
        bitSet(reg->status, RSF_DrawThisFrame);
    }
    else
    {
        opEffectsVal = (sdword)atom->pData;
    }

    opEffectsHelper();
}

static sliderhandle detailSliderRegion = NULL;

void opDetailDisable(void)
{
    if (detailSliderRegion == NULL)
    {
        return;
    }

    if (opNoLODVal)
    {
        bitSet(detailSliderRegion->reg.status, RSF_RegionDisabled);
    }
    else
    {
        bitClear(detailSliderRegion->reg.status, RSF_RegionDisabled);
    }

#ifdef DEBUG_STOMP
    regVerify((regionhandle)&detailSliderRegion->reg);
#endif

    detailSliderRegion->reg.status |= RSF_DrawThisFrame;
}

void opNoLOD(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opNoLODVal);
        opDetailDisable();
    }
    else
    {
        opNoLODVal = FECHECKED(atom) ? 1 : 0;
        opDetailDisable();
    }
}

void opDetailThreshold(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;
    if (FEFIRSTCALL(atom))
    {
        dbgMessage("\nopDetailThreshold() first call");
        detailSliderRegion = (sliderhandle)atom->region;
        shandle = (sliderhandle)atom->region;
        shandle->maxvalue = 51;
        shandle->value = opDetailThresholdVal - 50;

        f = regFilterSet(atom->region, 0);
        regFilterSet(atom->region, f | RPE_HoldLeft);

        shandle->processFunction  = opDetailThresholdProcess;

        opDetailDisable();
    }
    else if (FELASTCALL(atom))
    {
        detailSliderRegion = NULL;
    }
}


udword opBrightnessProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    featom *atom = reg->atom;
    sliderhandle shandle = (sliderhandle)atom->region;
    opBrightnessVal = shandle->value;
    shGammaSet(opBrightnessVal);
    return (0);
}

void opBrightness(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;
    if (FEFIRSTCALL(atom))
    {
        //feToggleButtonSet(name, texLinearFiltering);
        dbgMessage("\nopBrightness() first call");
        shandle = (sliderhandle)atom->region;
        shandle->maxvalue = 70;
        shandle->value = opBrightnessVal;

        f = regFilterSet(atom->region, 0);
        regFilterSet(atom->region, f | RPE_HoldLeft);

        shandle->processFunction = opBrightnessProcess;
    }
    else
    {
        //dbgMessage("\nopSpeechVolume()");
    }
}

/*
void opEffects(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, opEffectsVal);
    }
    else
    {
        opEffectsVal = (ubyte)atom->pData;
    }
}
*/

void opCustomEffects(char* name, featom* atom)
{
    feScreenStart(feStack[feStackIndex].baseRegion, "Effects"); //popup
}

/*-----------------------------------------------------------------------------
    Name        : opResItemDraw
    Description : draws a single Resolution listwindow item
    Inputs      : rect - region boundary
                  data - listitem userdata
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opResItemDraw(rectangle* rect, listitemhandle data)
{
    char temp[64];
    sdword x, y;
    color c;
    fonthandle oldfont;
    opres* res = (opres*)data->data;

    oldfont = fontMakeCurrent(opResListFont);

    if (bitTest(data->flags, UICLI_Selected))
    {
        opSaveMAIN_WindowWidth  = res->width;
        opSaveMAIN_WindowHeight = res->height;
        opSaveMAIN_WindowDepth  = res->depth;

        c = FEC_ListItemSelected;
    }
    else
    {
        c = FEC_ListItemStandard;
    }

    if (!opResResSupported(res))
    {
        if (bitTest(data->flags, UICLI_Selected))
        {
            c = colRGB(0,255,0);
        }
        else
        {
            c = colRGB(127,127,127);
        }
    }

    x = rect->x0 + ((rect->x1 - rect->x0) / 2) + 9;
    y = rect->y0;

    sprintf(temp, "%dx%d", res->width, res->height);
    fontPrint(x - fontWidth(temp), y, c, temp);

    sprintf(temp, "%d bit", res->depth);
    fontPrint(x + fontWidth("I"), y, c, temp);

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : opResListSet
    Description : sets the available resolutions based on selected rendering system
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opResListSet(rdevice* dev)
{
    rmode* mode;
    opres* res;
    sdword nModes, index;
    fonthandle oldfont;

    mode = dev->modes;
    nModes = 0;
    while (mode != NULL)
    {
        nModes++;
        mode = mode->next;
    }
    opResNumber = nModes;
    opResCurrentSelected = 0;

    if (opRes != NULL)
    {
        memFree(opRes);
    }
    opRes = memAlloc(nModes * sizeof(opres), "res list", NonVolatile);

    mode = dev->modes;
    res = opRes;
    while (mode != NULL)
    {
        res->width  = mode->width;
        res->height = mode->height;
        res->depth  = mode->depth;
        res++;
        mode = mode->next;
    }

    for (index = 0; index < opResNumber; index++)
    {
        if (opSaveMAIN_WindowWidth  == opRes[index].width &&
            opSaveMAIN_WindowHeight == opRes[index].height &&
            opSaveMAIN_WindowDepth  == opRes[index].depth)
        {
            opResCurrentSelected = index;
            break;
        }
    }

    oldfont = fontMakeCurrent(opResListFont);

    uicListRemoveAllItems(opResListWindow);

    for (index = 0; index < opResNumber; index++)
    {
        udword flags = UICLI_CanSelect;

        if (index == opResCurrentSelected)
        {
            bitSet(flags, UICLI_Selected);
        }

        uicListAddItem(opResListWindow,
                       (ubyte*)&opRes[index],
                       flags, UICLW_AddToTail);
    }

    fontMakeCurrent(oldfont);

    opDirtyResListWindow();
}

/*-----------------------------------------------------------------------------
    Name        : opResListLoad
    Description : sets up opRes[], the list of possible resolutions
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opResListLoad(void)
{
    //rswitch
    opResNumber = 0;
    opResCurrentSelected = 0;

    if (opRes != NULL)
    {
        memFree(opRes);
        opRes = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : opResolution
    Description : Resolution listwindow callback
    Inputs      : name, atom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opResolution(char* name, featom* atom)
{
    fonthandle oldfont;

    if (opResListFont == 0)
    {
        opResListFont = frFontRegister(OP_ResolutionFont);
    }

    if (FEFIRSTCALL(atom))
    {
        opResListLoad();

        oldfont = fontMakeCurrent(opResListFont);

        opResListRegion = (regionhandle)atom->region;
        opResListWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(opResListWindow,
                          NULL,
                          NULL,
                          0,
                          opResItemDraw,
                          fontHeight(" ") + (fontHeight(" ") >> 1),
                          UICLW_CanSelect);

        fontMakeCurrent(oldfont);

        return;
    }
    else if (FELASTCALL(atom))
    {
        opResListWindow = NULL;
        opResListRegion = NULL;
#if 0
        if (opRes != NULL)
        {
            memFree(opRes);
            opRes = NULL;
        }
#endif
        return;
    }
}

void opSqueezeString(char* buf, rectangle* rect, sdword offset)
{
    sdword width;

    width = rect->x1 - rect->x0;
    width -= offset;

    while (fontWidth(buf) > width)
    {
        if (strlen(buf) < 2)
        {
            return;
        }
        buf[strlen(buf) - 1] = '\0';
    }
}

/*-----------------------------------------------------------------------------
    Name        : opRenderItemDraw
    Description : draws a single Render listwindow item
    Inputs      : rect - region boundary
                  data - listitem userdata
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opRenderItemDraw(rectangle* rect, listitemhandle data)
{
    char temp[128];
    sdword x, y;
    color c0, c1;
    fonthandle oldfont;
    oprender* rnd = (oprender*)data->data;

    oldfont = fontMakeCurrent(opRenderListFont);

    if (bitTest(data->flags, UICLI_Selected))
    {
        c0 = c1 = FEC_ListItemSelected;
        if (opRndSelected != rnd || opRes == NULL)
        {
            opRndSelected = rnd;
            opResListSet(rnd->dev);
        }
        opSaveRndSelected = rnd;
    }
    else
    {
        c0 = FEC_ListItemStandard;
        c1 = FEC_ListItemStandard;
    }

    x = rect->x0 + fontWidth("(D3D)");
    y = rect->y0;

    switch (rnd->type)
    {
    case RIN_TYPE_OPENGL:
        sprintf(temp, "(GL)");
        break;
    case RIN_TYPE_DIRECT3D:
        sprintf(temp, "(D3D)");
        break;
    default:
        sprintf(temp, "(SW)");
    }

    fontPrint(x - fontWidth(temp), y, c0, temp);

    sprintf(temp, "%s", rnd->name);
    opSqueezeString(temp, rect, x - rect->x0);
    fontPrint(x + fontWidth(" "), y, c1, temp);

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : opRenderListLoad
    Description : sets up opRnd[], the list of possible renderers
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opRenderListLoad(void)
{
    //rswitch
    sdword nDevices;
    rdevice* dev;

    opRenderNumber = nDevices = 0;
    dev = rinGetDeviceList();
    while (dev != NULL)
    {
        nDevices++;
        dev = dev->next;
    }

    if (opRnd != NULL)
    {
        memFree(opRnd);
    }
    opRnd = (oprender*)memAlloc(nDevices * sizeof(oprender), "opRnd device list", NonVolatile);

    dev = rinGetDeviceList();
    while (dev != NULL)
    {
        opRnd[opRenderNumber].dev = dev;
        opRnd[opRenderNumber].type = dev->type;
        memStrncpy(opRnd[opRenderNumber].data, dev->data, 63);
        memStrncpy(opRnd[opRenderNumber].name, dev->name, 63);

        switch (mainActiveRenderer())
        {
        case GLtype:
            if (dev->type == RIN_TYPE_OPENGL)
            {
                opRenderCurrentSelected = opRenderNumber;
                opRndSelected = &opRnd[opRenderNumber];
            }
            break;

        case D3Dtype:
            if (dev->type == RIN_TYPE_DIRECT3D)
            {
                if (strcmp(dev->data, lastDev) == 0)
                {
                    opRenderCurrentSelected = opRenderNumber;
                    opRndSelected = &opRnd[opRenderNumber];
                }
            }
            break;

        default:
            if (dev->type == RIN_TYPE_SOFTWARE)
            {
                opRenderCurrentSelected = opRenderNumber;
                opRndSelected = &opRnd[opRenderNumber];
            }
        }

        //increment device count
        opRenderNumber++;

        //next device
        dev = dev->next;
    }

    if (opDeviceIndex != -1)
    {
        opRenderCurrentSelected = opDeviceIndex;
        opRndSelected = &opRnd[opDeviceIndex];
    }
}

/*-----------------------------------------------------------------------------
    Name        : opRender
    Description : Render listwindow callback
    Inputs      : name, atom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void opRender(char* name, featom* atom)
{
    fescreen* screen;
    featom* npatom;
    fonthandle oldfont;
    sdword index;

    if (opRenderListFont == 0)
    {
        opRenderListFont = frFontRegister(OP_RenderFont);
    }

    if (FEFIRSTCALL(atom))
    {
        opRenderListLoad();

        oldfont = fontMakeCurrent(opRenderListFont);

        opRenderListWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(opRenderListWindow,
                          NULL,
                          NULL,
                          0,
                          opRenderItemDraw,
                          fontHeight(" ") + (fontHeight(" ") >> 1),
                          UICLW_CanSelect);

        for (index = 0; index < opRenderNumber; index++)
        {
            udword flags = UICLI_CanSelect;

            if (opSaveRndSelected == &opRnd[index])
            {
                bitSet(flags, UICLI_Selected);
            }

            uicListAddItem(opRenderListWindow,
                           (ubyte*)&opRnd[index],
                           flags, UICLW_AddToTail);
        }

        screen = feScreenFind("Video_Options");
        if (screen != NULL)
        {
            npatom = feAtomFindInScreen(screen, "NP_Draw");
            if (npatom != NULL)
            {
                opNoPalDrawRegion = (regionhandle)npatom->region;
                trNoPalResizePool(opNoPalMB);
            }
        }

        fontMakeCurrent(oldfont);
    }
    else if (FELASTCALL(atom))
    {
        opNoPalDrawRegion = NULL;
        opRenderListWindow = NULL;
#if 0
        if (opRnd != NULL)
        {
            memFree(opRnd);
            opRnd = NULL;
        }
#endif
    }
}

void EqualizerCommon(char* name, featom* atom, sdword ID)
{
    udword f;
    sliderhandle shandle;

    if (FEFIRSTCALL(atom))
    {
        shandle = (sliderhandle)atom->region;
        shandle->maxvalue = 100;
        shandle->ID = ID;
        shandle->value = opEqualizerSettings[shandle->ID];

        f = regFilterSet(atom->region, 0);
        regFilterSet(atom->region, f | RPE_HoldLeft);

        shandle->processFunction = opEqualizerProcess;

/*        //smoothies
        smootheqvalue[ID] = (real32)opEqualizerSettings[shandle->ID];

        smootheqsmoothie[ID].feedback = CONTROL_FEEDBACK;
        smootheqsmoothie[ID].source = &smootheqvalue[ID];
        smootheqsmoothie[ID].dest = NULL;
        smootheqsmoothie[ID].destint = &opEqualizerSettings[shandle->ID];
        smootheqsmoothie[ID].callback = &EqualizerSmoothieCallback;
        smootheqsmoothie[ID].threshold = 1.0f;
        smootheqsmoothie[ID].user = (sdword*)shandle;

        AddSmoothie(&smootheqsmoothie[ID]);*/
    }
}

void opEqualizer1(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        opEqualizerToggled = FALSE;
        EqualizerCommon(name, atom, 0);
    }
    else // if (FELASTCALL(atom))
    {
        ; //RemoveSmoothie(&smootheqsmoothie[0]);
    }
}

void opEqualizer2(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        EqualizerCommon(name, atom,1);
    }
    else // if (FELASTCALL(atom))
    {
        ; //RemoveSmoothie(&smootheqsmoothie[1]);
    }
}

void opEqualizer3(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        EqualizerCommon(name, atom,2);
    }
    else // if (FELASTCALL(atom))
    {
        ; //RemoveSmoothie(&smootheqsmoothie[2]);
    }
}

void opEqualizer4(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        EqualizerCommon(name, atom,3);
    }
    else // if (FELASTCALL(atom))
    {
        ; //RemoveSmoothie(&smootheqsmoothie[3]);
    }
}

void opEqualizer5(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        EqualizerCommon(name, atom,4);
    }
    else // if (FELASTCALL(atom))
    {
        ; //RemoveSmoothie(&smootheqsmoothie[4]);
    }
}

void opEqualizer6(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        EqualizerCommon(name, atom,5);
    }
    else // if (FELASTCALL(atom))
    {
        ; //RemoveSmoothie(&smootheqsmoothie[5]);
    }
}

void opEqualizer7(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        EqualizerCommon(name, atom,6);
    }
    else // if (FELASTCALL(atom))
    {
        ; //RemoveSmoothie(&smootheqsmoothie[6]);
    }
}

void opEqualizer8(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        EqualizerCommon(name, atom,7);
    }
    else // if (FELASTCALL(atom))
    {
        ; //RemoveSmoothie(&smootheqsmoothie[7]);
    }
}

void opVoice0(char* name, featom* atom)
{
#ifdef Downloadable
    featom *hear3;
#endif

    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opVoice0On);

#ifdef Downloadable
        hear3 = feAtomFindInScreen(feScreenFind("Advanced"),"OP_Hear_Voice_3");

        if (hear3 != NULL && hear3->region != NULL)
        {
            bitSet(((region *)hear3->region)->status, RSF_RegionDisabled);
        }
        hear3 = feAtomFindInScreen(feScreenFind("Advanced_speech"),"OP_Hear_Voice_3");

        if (hear3 != NULL && hear3->region != NULL)
        {
            bitSet(((region *)hear3->region)->status, RSF_RegionDisabled);
        }
#endif
    }
    else
    {
        opVoice0On = FECHECKED(atom) ? 1 : 0;
    }
}

void opVoice1(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opVoice1On);
    }
    else
    {
        opVoice1On = FECHECKED(atom) ? 1 : 0;
    }
}

void opVoice2(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
#ifdef Downloadable
        if (atom != NULL && atom->region != NULL)
        {
            bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
        }
#endif
        feToggleButtonSet(name, opVoice2On);
    }
    else
    {
        opVoice2On = FECHECKED(atom) ? 1 : 0;
    }
}

void opVoice(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, opVoiceSetting);
    }
    else
    {
        opVoiceSetting = (sdword)atom->pData;
    }

    switch (opVoiceSetting)
    {
        case 0:
            soundEventSetActor(1, TRUE);
            soundEventSetActor(2, FALSE);
            soundEventSetActor(3, FALSE);
            break;
        case 1:
            soundEventSetActor(2, TRUE);
            soundEventSetActor(1, FALSE);
            soundEventSetActor(3, FALSE);
            break;
        case 2:
            soundEventSetActor(3, TRUE);
            soundEventSetActor(1, FALSE);
            soundEventSetActor(2, FALSE);
            break;
        default:
            dbgMessage("\nSomething's not right...");
    }
}


void opHearVoice1(char* name, featom* atom)
{
    soundEventHearActor(1);

    dbgMessage("\nNow playing voice 1");
}

void opHearVoice2(char* name, featom* atom)
{
    soundEventHearActor(2);

    dbgMessage("\nNow playing voice 2");
}

void opHearVoice3(char* name, featom* atom)
{
    soundEventHearActor(3);

    dbgMessage("\nNow playing voice 3");
}

void opVoiceCommands(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opVoiceComm);
        soundEventVocalSettings(opVoiceComm, opVoiceStat, opVoiceChat);
    }
    else
    {
        opVoiceComm = FECHECKED(atom) ? 1 : 0;
        soundEventVocalSettings(opVoiceComm, opVoiceStat, opVoiceChat);
    }
}

void opVoiceStatus(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opVoiceStat);
        soundEventVocalSettings(opVoiceComm, opVoiceStat, opVoiceChat);
    }
    else
    {
        opVoiceStat = FECHECKED(atom) ? 1 : 0;
        soundEventVocalSettings(opVoiceComm, opVoiceStat, opVoiceChat);
    }
}

void opVoiceChatter(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opVoiceChat);
        soundEventVocalSettings(opVoiceComm, opVoiceStat, opVoiceChat);
    }
    else
    {
        opVoiceChat = FECHECKED(atom) ? 1 :0;
        soundEventVocalSettings(opVoiceComm, opVoiceStat, opVoiceChat);
    }
}

void opMouseSensitivity(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;
    if (FEFIRSTCALL(atom))
    {
       //feToggleButtonSet(name, texLinearFiltering);
       //dbgMessage("\nopMusicVolume() first call");
       shandle = (sliderhandle)atom->region;
       shandle->maxvalue = 100;
       shandle->value = opMouseSens;

       f = regFilterSet(atom->region, 0);
       regFilterSet(atom->region, f | RPE_HoldLeft);

       shandle->processFunction  = opMouseSensitivityProcess;

    }
    else
    {
        ;
    }

}

void opBattleChatterCB(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;
    if (FEFIRSTCALL(atom))
    {
       //feToggleButtonSet(name, texLinearFiltering);
       //dbgMessage("\nopMusicVolume() first call");
       shandle = (sliderhandle)atom->region;
       shandle->maxvalue = 100;
       shandle->value = opBattleChatter;

       f = regFilterSet(atom->region, 0);
       regFilterSet(atom->region, f | RPE_HoldLeft);

       shandle->processFunction  = opBattleChatterProcess;
       opBattleChatterProcess(atom->region, 0, 0, 0);
    }
    else
    {
        ;
    }
}

void opNumberEffects(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;
    if (FEFIRSTCALL(atom))
    {
       //feToggleButtonSet(name, texLinearFiltering);
       //dbgMessage("\nopMusicVolume() first call");
       EffectsSlider = shandle = (sliderhandle)atom->region;
       shandle->maxvalue = 256 - etgHistoryScalarMin;
       shandle->value = opNumEffects;

       f = regFilterSet(atom->region, 0);
       regFilterSet(atom->region, f | RPE_HoldLeft);

       shandle->processFunction  = opNumEffectsProcess;
       opNumEffectsProcess(atom->region, 0, 0, 0);
    }
    else if (FELASTCALL(atom))
    {
       EffectsSlider = NULL;
    }
}

void opCPUDifficulty(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;

    if (FEFIRSTCALL(atom))
    {
        shandle = (sliderhandle)atom->region;
        shandle->maxvalue = 101;
        shandle->value = tpGameCreated.aiplayerDifficultyLevel;

        f = regFilterSet(atom->region, 0);
        regFilterSet(atom->region, f | RPE_HoldLeft);

        shandle->processFunction = opCPUDifficultyProcess;
    }
    else if (FELASTCALL(atom))
    {
        ;
    }
}

void opCPUAttacks(char* name, featom* atom)
{
    udword f;
    sliderhandle shandle;

    if (FEFIRSTCALL(atom))
    {
        shandle = (sliderhandle)atom->region;
        shandle->maxvalue = 101;
        shandle->value = tpGameCreated.aiplayerBigotry;

        f = regFilterSet(atom->region, 0);
        regFilterSet(atom->region, f | RPE_HoldLeft);

        shandle->processFunction = opCPUAttacksProcess;
    }
    else if (FELASTCALL(atom))
    {
        ;
    }
}

/*-----------------------------------------------------------------------------
    Name        : opInfoOverlay
    Description : Processes toggling of Info overlay
    Inputs      : void
    Outputs     :
    Return      : Nothing
----------------------------------------------------------------------------*/
void opInfoOverlay(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        if (opInfoOverlayVar)
        {
            bitSet(atom->status, FAS_Checked);
        }
        else
        {
            bitClear(atom->status, FAS_Checked);
        }
        return;
    }

    if (bitTest(atom->status, FAS_Checked))
    {
        opInfoOverlayVar = 1;
    }
    else
    {
        opInfoOverlayVar = 0;
    }
    if (opInfoOverlayVar)
        ioEnable();
    else
        ioDisable();
}

void opTaskbarUp(char* name, featom* atom)
{

}

void opIncreaseChatting(char* name, featom* atom)
{

}

void opDecreaseChatting(char* name, featom* atom)
{

}

void opNumberChatlines(char* name, featom* atom)
{

}

void opAutodockHealth(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opAutodockHealthCheck);
    }
    else
    {
        opAutodockHealthCheck = FECHECKED(atom);
    }

}

void opAutodockFuel(char* name, featom* atom)
{
    if (FEFIRSTCALL(atom))
    {
        feToggleButtonSet(name, opAutodockFuelCheck);
    }
    else
    {
        opAutodockFuelCheck = FECHECKED(atom);
    }

}


void opAutodockHealthValue(char* name, featom* atom)
{

    char    temp[20];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        opAutodockHealthEntryBox = (textentryhandle)atom->pData;
        sprintf(temp,"%d",opAutodockHealthVal);
        uicTextEntrySet(opAutodockHealthEntryBox,temp,strlen(temp)+1);
        uicTextBufferResize(opAutodockHealthEntryBox,MAX_NUM_LENGTH-2);
        uicTextEntryInit(opAutodockHealthEntryBox,UICTE_NumberEntry);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            sscanf(opAutodockHealthEntryBox->textBuffer,"%d",&opAutodockHealthVal);
            if (uicTextEntryMessage(atom)==CM_AcceptText) feToggleButtonSet("OP_Autodock_Health", TRUE);
        break;
        case CM_GainFocus :
        break;
    }
}

void opAutodockFuelValue(char* name, featom* atom)

{

    char    temp[20];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        opAutodockFuelEntryBox = (textentryhandle)atom->pData;
        sprintf(temp,"%d",opAutodockFuelVal);
        uicTextEntrySet(opAutodockFuelEntryBox,temp,strlen(temp)+1);
        uicTextBufferResize(opAutodockFuelEntryBox,MAX_NUM_LENGTH-2);
        uicTextEntryInit(opAutodockFuelEntryBox,UICTE_NumberEntry);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            sscanf(opAutodockFuelEntryBox->textBuffer,"%d",&opAutodockFuelVal);
            if (uicTextEntryMessage(atom)==CM_AcceptText) feToggleButtonSet("OP_Autodock_Fuel", TRUE);
        break;
        case CM_GainFocus :
        break;
    }

}




// Smoothies sucked some serious ass so bye bye!!

#if(0)
void InitSmoothie(Smoothie *smoothie,real32 feedback, real32 *source,
    real32 *dest, sdword *destint, smoothieFunc callback, real32 threshold)
{
    smoothie->feedback = feedback;
    smoothie->source = source;
    smoothie->dest = dest;
    smoothie->destint = destint;
    smoothie->callback = callback;
    smoothie->threshold = threshold;
}




void AddSmoothie(Smoothie *smoo)
{
    udword i;
    for (i=0; i<NUM_SMOOTHIES; i++)
    {
        if (!SmoothieArray[i])
        {
            SmoothieArray[i] = smoo;
            SmoothieArray[i]->buffer = *(SmoothieArray[i]->source);
            return;
        }
    }
    dbgFatal(DBG_Loc, "Out of smoothies!!!");
}

void RemoveSmoothie(Smoothie *smoo)
{
    udword i;
    for (i=0; i<NUM_SMOOTHIES; i++)
    {
        if (SmoothieArray[i] == smoo)
        {
            SmoothieArray[i] = NULL;
            return;
        }
    }
    dbgFatal(DBG_Loc, "Smoothie couldn't be removed!!!");
}


void DoSmoothies(void)
{
    Smoothie* smoo;
    sdword i;

    for (i = 0; i < NUM_SMOOTHIES; i++)
    {
        if (SmoothieArray[i])
        {
            smoo = SmoothieArray[i];

#if 0
            //non-smoothed smoothies.  FIXME: check ranges
            smoo->buffer = *(smoo->source);
            if (smoo->dest)
            {
                *(smoo->dest) = smoo->buffer;
            }
            if (smoo->destint)
            {
                *(smoo->destint) = (sdword)smoo->buffer;
            }
            if (smoo->callback)
            {
                (smoo->callback)(smoo->buffer, smoo);
            }
#else
            if (ABS(*(smoo->source)) == smoo->buffer)
            {
                ; //nothing
            }
            else if (ABS(*(smoo->source) - smoo->buffer) < smoo->threshold)
            {
                //make same and do last update
                *(smoo->source) = smoo->buffer;
                if (smoo->dest)
                {
                    *(smoo->dest) = smoo->buffer;
                }
                if (smoo->destint)
                {
                    *(smoo->destint) = (sdword)smoo->buffer;
                }
                if (smoo->callback)
                {
                    (smoo->callback)(smoo->buffer, smoo);
                }
            }
            else
            {
                //smooth and update
                smoo->buffer = smoo->feedback * smoo->buffer +
                               (1.0f - smoo->feedback) * (*(smoo->source));

                if (smoo->dest)
                {
                    *(smoo->dest) = smoo->buffer;
                }
                if (smoo->destint)
                {
                    *(smoo->destint) = (sdword)smoo->buffer;
                }
                if (smoo->callback)
                {
                    (smoo->callback)(smoo->buffer, smoo);
                }
            }
#endif
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : opHighlightTaskFunction
    Description : Task function for higlighting
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool opSmoothiesBabyFunction(udword num, void* data, struct BabyCallBack* baby)
{
    DoSmoothies();
    return !smoothiesactive;
}

#endif

void opNoPalDraw(featom* atom, regionhandle region);

void opNoPalDirty(void)
{
    if (opNoPalDrawRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(opNoPalDrawRegion);
#endif
        opNoPalDrawRegion->status |= RSF_DrawThisFrame;
        opNoPalDraw(NULL, opNoPalDrawRegion);
    }
}

udword opNoPalIncreaseProcess(regionhandle region, sdword ID, udword event, udword data)
{
#ifdef DEBUG_STOMP
    regVerify(region);
#endif

    if (event == RPE_ReleaseLeft)
    {
        if (trNoPalettes)
        {
            opNoPalMB += 4;
            if (opNoPalMB > opNoPalMaxMB)
            {
                opNoPalMB = opNoPalMaxMB;
            }
            trNoPalResizePool(opNoPalMB);
        }
        region->status |= RSF_DrawThisFrame;
        opNoPalDirty();
        return 0;
    }
    region->status |= RSF_DrawThisFrame;
    return 0;
}

udword opNoPalDecreaseProcess(regionhandle region, sdword ID, udword event, udword data)
{
#ifdef DEBUG_STOMP
    regVerify(region);
#endif

    if (event == RPE_ReleaseLeft)
    {
        if (trNoPalettes)
        {
            opNoPalMB -= 4;
            if (opNoPalMB < opNoPalMinMB)
            {
                opNoPalMB = opNoPalMinMB;
            }
            trNoPalResizePool(opNoPalMB);
        }
        region->status |= RSF_DrawThisFrame;
        opNoPalDirty();
        return 0;
    }
    region->status |= RSF_DrawThisFrame;
    return 0;
}

void opNoPalDecrease(featom* atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_Enter | RPE_Exit;
        if (!bitTest(atom->flags, FAF_Disabled))
        {
            regFunctionSet(region, opNoPalDecreaseProcess);
        }
    }
    mgDrawArrow(region, TRUE, FALSE);
}

void opNoPalIncrease(featom* atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_Enter | RPE_Exit;
        if (!bitTest(atom->flags, FAF_Disabled))
        {
            regFunctionSet(region, opNoPalIncreaseProcess);
        }
    }
    mgDrawArrow(region, FALSE, FALSE);
}

void opBlackRect(sdword x, sdword y, char* str)
{
    rectangle rect;

    rect.x0 = x;
    rect.y0 = y;
    rect.x1 = x + fontWidth(str);
    rect.y1 = y + fontHeight(str);
    primRectSolid2(&rect, colBlack);
}

void opNoPalDraw(featom* atom, regionhandle region)
{
    fonthandle oldfont;
    char buf[16];
    color c;
    extern fonthandle ghDefaultFont;

    primRectSolid2(&region->rect, colBlack);
    oldfont = fontMakeCurrent(ghDefaultFont);

    sprintf(buf, "%dMB", trNoPalettes ? opNoPalMB : 0);
    c = trNoPalettes ? colRGB(255,200,0) : colRGB(191,150,0);
    opBlackRect(region->rect.x0 - (trNoPalettes ? 5 : 0),
                region->rect.y0, buf);
    fontPrint(region->rect.x0 - (trNoPalettes ? 5 : 0), region->rect.y0, c, buf);
    fontMakeCurrent(oldfont);
}

void opTroubleShoot(char* name, featom* atom)
{
    utyBrowserExec(OP_TroubleShootPage);
}
