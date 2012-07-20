
#ifndef ___MAINSWITCHES_H
#define ___MAINSWITCHES_H

#include "Types.h"
#ifndef STATVIEWER_PROGRAM
//statviewer program is compiling this bitch...lets not do this
#include "Globals.h"
#endif
#include "ObjTypes.h"
#include "Formation.h"

//command-line switches and parameters
extern bool DebugWindow;
extern sdword MemoryHeapSize;
extern bool FilePathPrepended;
extern bool CDROMPathPrepended;
extern bool UserSettingsPathPrepended;
extern bool mouseClipped;
extern bool startupClipMouse;
extern bool showFrontEnd;
extern bool enableSFX;
extern bool enableSpeech;
extern bool reverseStereo;
extern bool useWaveout;
extern bool useDSound;
extern bool coopDSound;
extern bool noDefaultComputerPlayer;
extern bool8 ComputerPlayerEnabled[MAX_MULTIPLAYER_PLAYERS];
extern udword ComputerPlayerLevel[MAX_MULTIPLAYER_PLAYERS];
extern bool gatherStats;

extern bool showStatsFight;
extern udword showStatsFightI;
extern udword showStatsFightJ;

extern bool showStatsFancyFight;
extern char showStatsFancyFightScriptFile[50];

extern sdword GiveAllTechnology;

#ifdef DEBUG_TACTICS
    extern bool tacticsOn;
#endif
extern bool noRetreat;

extern bool recordPackets;
extern bool playPackets;
extern bool recordplayPacketsInGame;
extern bool recordFakeSendPackets;
#define MAX_RECORDPACKETFILENAME_STRLEN 50
extern char recordPacketFileName[MAX_RECORDPACKETFILENAME_STRLEN];
extern char recordPacketSaveFileName[MAX_RECORDPACKETFILENAME_STRLEN];

extern bool noAuthorization;

extern bool mainSinglePlayerEnabled;
extern bool mainScreenShotsEnabled;
extern bool mainCDCheckEnabled;
extern bool mainCDCheckEnabled;
extern bool mainEnableSpecialMissions;

extern bool SecretWON;
extern bool forceLAN;

extern bool ShortCircuitWON;

extern char versionString[];
extern char networkVersion[];
extern char languageVersion[];
extern char minorBuildVersion[];

extern bool autoSaveDebug;

extern bool determCompPlayer;

extern bool bkDisableKeyRemap;

extern bool debugPacketRecord;

extern bool pilotView;

#endif

