/****************************************

    Name: HorseRace.h

    Defines for begining game horse Race

*****************************************/
#ifndef HORSERACE_H
#define HORSERACE_H

#include "Types.h"
#include "Globals.h"

/*=============================================================================
    defines:
=============================================================================*/

#ifdef _WIN32
#define HR_FIBFile          "FEMAN\\Horse_Race.fib"
#else
#define HR_FIBFile          "FEMAN/Horse_Race.fib"
#endif
#define HR_RaceScreen       "Horse_Race"
#define HR_RaceScreenNotNetwork "Horse_Race_NonNetwork"
#define HR_SingleRaceScreen "Horse_Race_Single"

#define HR_BackBarColor colRGB(75,75,75)
#define HR_ChatTextColor colRGB(0,200,256)

#define DOWNLOADMAP_BAR         0
#define UNIVERSE_BAR            1
#define ETG_BAR                 2
#define TEXTURE1_BAR            3
#define TEXTURE2_BAR            4

/*=============================================================================
    structures:
=============================================================================*/

#define MAX_HRSTATUS_STRING     100

typedef struct
{
    udword barnum[MAX_MULTIPLAYER_PLAYERS];
    real32 percent[MAX_MULTIPLAYER_PLAYERS];
    char hrstatusstr[MAX_MULTIPLAYER_PLAYERS][MAX_HRSTATUS_STRING];
}HorseStatus;

#define MAX_POSSIBLE_NUM_BARS 20
typedef struct HorseRaceBars
{
    sdword numBars;
    real32 perc[MAX_POSSIBLE_NUM_BARS];
} HorseRaceBars;

/*=============================================================================
    data:
=============================================================================*/

extern bool hrRunning;
extern bool hrBackgroundReinit;

extern HorseStatus horseracestatus;

/*=============================================================================
    Prototypes:
=============================================================================*/
struct ChatPacket;

void horseRaceInit(void);
void horseRaceShutdown(void);

void horseRaceWaitForNetworkGameStart(void);

void hrProcessPacket(struct ChatPacket *packet);

void recievedHorsePacketCB(ubyte *packet,udword sizeofpacket);

void HorseRaceBeginBar(uword barnum);
bool HorseRaceNext(real32 percent);

#endif
