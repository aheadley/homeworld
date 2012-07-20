
/*=============================================================================
    Name    : options.h
    Purpose : Definitons for the options

    Created  11/05/1998 by yo
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#ifndef ___OPTIONS_H
#define ___OPTIONS_H

#include "Types.h"
#include "LilOptions.h"
#include "Region.h"

/*=============================================================================
    Definitions:
=============================================================================*/
#define OP_KEYMAPSIZE 80
    //4 chars needed per key

#define OP_TroubleShootPage "http://www.sierrastudios.com/games/homeworld/noflash/c-support.html"

#define OP_ResolutionFont "Arial_12.hff"
#define OP_RenderFont "Arial_12.hff"

#define OP_ResolutionSelectedColor  colRGB(255,255,105)
#define OP_ResolutionColor          colRGB(240,240,240)

#define OP_RenderSelectedColor      colRGB(255,255,105)
#define OP_RenderColor              colRGB(240,240,240)

#define OP_DimRenderColor     colRGB(192,192,192)
#define OP_RenderColorOutline colRGB(120,70,0)

extern uword opKeyDetour;
extern char opKeymap[OP_KEYMAPSIZE];

extern bool opReloading;
extern sdword opNoPalMB;
extern sdword opMusicVol;
extern sdword opSFXVol;
extern sdword opSpeechVol;
extern sdword opSpeakerSe;
extern sdword opEqualizer;
extern sdword opSpeakerSetting;
extern sdword opVoice0On;
extern sdword opVoice1On;
extern sdword opVoice2On;
extern sdword opVoiceComm;
extern sdword opVoiceStat;
extern sdword opVoiceChat;
extern sdword opMouseSens;
extern sdword opInfoOverlayVar;
extern sdword opBattleChatter;
extern sdword opEffectsVal;
extern sdword opBrightnessVal;
extern udword opDetailThresholdVal;
extern bool opCustomEffectsToggled;
extern sdword opNoLODVal;
extern sdword opNumChannels;
extern sdword opAutoChannel;
extern sdword opSoundQuality;

extern bool   opTimerActive;
extern real32 opTimerStart;
extern real32 opTimerLength;

void opOptionsSaveCustomEffectsSettings(void);


#define NUM_SMOOTHIES 32


struct SmoothieX;

typedef void (*smoothieFunc)(real32 data, struct SmoothieX *smoo);    //callback function

typedef struct SmoothieX
{
    real32 feedback;
    real32 buffer;
    real32 *source;
    real32 *dest;
    sdword *destint;
    real32 threshold;
    sdword *user;
    smoothieFunc callback;
}
Smoothie;



/*=============================================================================
    Function Prototypes:
=============================================================================*/

void opUpdateSettings(void);
void opInitKeyTable(void);
char *opKeyToNiceString(keyindex key);
keyindex opKeyTranslate(keyindex key);
void opKeyAssign(uword func, keyindex key);
bool opValidKey(keyindex key);
void opDefineThisKey(keyindex key);
void opKeyboardLoad(void);
udword opMusicVolumeProcess(regionhandle reg, sdword ID, udword event, udword data);
udword opSFXVolumeProcess(regionhandle reg, sdword ID, udword event, udword data);
udword opSpeechVolumeProcess(regionhandle reg, sdword ID, udword event, udword data);
udword opNumChannelsProcess(regionhandle reg, sdword ID, udword event, udword data);

void opTimerExpired(void);

void InitSmoothie(Smoothie *smooothie,real32 feedback, real32 *source, real32 *dest,
                  sdword *destint, smoothieFunc callback, real32 threshold);
void AddSmoothie(Smoothie *smoo);
void RemoveSmoothie(Smoothie *smoo);

char ConvertSliderToAIPlayerDifficulty(sdword sliderval);

#endif
