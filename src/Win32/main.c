/*============================================================================
    MAIN.C: Main Windows interface for Homeworld game code, including message
        handling and window set-up.

        Created June 1997 by Luke Moloney.
============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winreg.h>

#include "leakyfaucet.h"
#include "regkey.h"
                // guess what?  The game code defines HKEY to 'H' which messes up the registry code.  So the
                // registry code gets to go here
int RegisterCommandLine(char *commandLine)
{
    HKEY key;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, BASEKEYNAME,
                        0, KEY_SET_VALUE, &key) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if ((commandLine == NULL) || (commandLine[0] == 0))
    {
        if (RegSetValueEx(key, "CmdLine", 0, REG_SZ, "", 1) != ERROR_SUCCESS)
        {
            RegCloseKey(key);
            return FALSE;
        }
    }
    else
    {
        if (RegSetValueEx(key, "CmdLine", 0, REG_SZ, (BYTE *)commandLine, strlen(commandLine)+1) != ERROR_SUCCESS)
        {
            RegCloseKey(key);
            return FALSE;
        }
    }

    RegCloseKey(key);

    return TRUE;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "glinc.h"
#include "resource.h"
#include "utility.h"
#include "key.h"
#include "mouse.h"
#include "debugwnd.h"
#include "debug.h"
#include "memory.h"
#include "file.h"
#include "camera.h"
#include "task.h"
#include "mainrgn.h"
#include "main.h"
#include "globals.h"
#include "soundevent.h"
#include "soundlow.h"
#include "nis.h"
#include "aiplayer.h"
#include "fontreg.h"
#include "fereg.h"
#include "demo.h"
#include "researchapi.h"
#include "objtypes.h"
#include "formation.h"
#include "glcaps.h"
#include "mainswitches.h"
#include "tactics.h"
#include "render.h"
#include "autolod.h"
#include "captaincy.h"
#include "options.h"
#include "sensors.h"
#include "btg.h"
#include "researchgui.h"
#include "consmgr.h"
#include "rinit.h"
#include "avi.h"
#include "bink.h"
#include "titannet.h"
#include "multiplayergame.h"
#include "subtitle.h"
#include "dxdraw.h"
#include "launchmgr.h"
#include "colpick.h"
#include "horserace.h"
#include "glcompat.h"
#include "sstglide.h"
#include "particle.h"
#include "commandlayer.h"
#include "key.h"
#include "strings.h"

#include "zmouse.h"
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL WM_MOUSELAST+1
#endif
UINT uMSH_MOUSEWHEEL = 0;

/*=============================================================================
    Data:
=============================================================================*/

extern udword gDevcaps, gDevcaps2;

bool mainSafeGL = FALSE;

udword* devTable = NULL;
sdword  devTableLength = 0;

long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static bool mainActuallyQuit = TRUE;
static DWORD style;

bool selectedRES = FALSE;
bool selectedDEVICE = FALSE;
bool selectedGL = FALSE;

sdword mainReinitRenderer = 0;
static bool reinitInProgress = FALSE;

bool bMustFree = TRUE;

bool mainPlayAVIs;

bool windowNeedsDeleting = FALSE;

bool mainNoPerspective = FALSE;

bool systemActive = FALSE;              //active flag for the program
static char classTitle[] = "Homeworld"; //name of registered class
static char windowTitle[] = "Homeworld";//name of window

//error strings
char ersWindowInit[] = "Error creating window";

//screen width, height
sdword MAIN_WindowWidth = 640;
sdword MAIN_WindowHeight = 480;
sdword MAIN_WindowDepth = 16;

sdword mainWidthAdd = 0;
sdword mainHeightAdd = 0;

sdword mainWindowWidth = 640;
sdword mainWindowHeight = 480;
sdword mainWindowDepth = 16;

extern bool CompareBigfiles;
extern bool IgnoreBigfiles;
extern bool LogFileLoads;

//command-line switches and parameters
bool mainNoDrawPixels = FALSE;
bool mainOutputCRC = FALSE;
bool mainNoPalettes = FALSE;
bool mainSoftwareDirectDraw = TRUE;
bool mainDirectDraw = TRUE;
bool mainRasterSkip = FALSE;
bool mainDoubleIsTriple = FALSE;
bool mainFastFrontend = TRUE;
bool mainForceSoftware = FALSE;
bool mainAutoRenderer = TRUE;
bool mainForceKatmai = FALSE;
bool mainAllowKatmai = FALSE;
bool mainAllow3DNow = FALSE;
bool enableAVI = TRUE;
bool mainAllowPacking = TRUE;
bool mainOnlyPacking = FALSE;
bool gShowDamage = TRUE;
bool DebugWindow = FALSE;
sdword MemoryHeapSize = MEM_HeapSizeDefault;
bool FilePathPrepended = FALSE;
bool CDROMPathPrepended = FALSE;
#if MAIN_MOUSE_FREE
bool startupClipMouse = TRUE;
#endif
bool mouseClipped = FALSE;
sdword showBackgrounds = TRUE;
bool showBorder = TRUE;
sdword enableTextures = TRUE;
sdword enableSmoothing = TRUE;
sdword enableStipple = FALSE;
sdword enableTrails = TRUE;
#if TR_NIL_TEXTURE
bool GLOBAL_NO_TEXTURES = FALSE;
#endif
bool fullScreen = TRUE;
bool slowBlits = FALSE;
#if RND_VISUALIZATION
bool dockLines = FALSE;
bool gunLines = FALSE;
bool8 RENDER_BOXES;
bool8 RENDER_LIGHTLINES;
#endif
bool enableSFX = TRUE;
bool enableSpeech = TRUE;
bool reverseStereo = FALSE;
bool useWaveout = FALSE;
bool useDSound = FALSE;
bool coopDSound = FALSE;
bool accelFirst = FALSE;
char mainDeviceToSelect[128] = "";
char mainGLToSelect[512] = "";
char mainD3DToSelect[128] = "";
char deviceToSelect[128] = "";
char glToSelect[512] = "rgl.dll";
bool8 RENDER_BOXES = FALSE;
bool8 RENDER_LIGHTLINES = FALSE;
#if CL_TEXTFEEDBACK
bool enableTextFeedback = FALSE;
#endif

#if FE_TEXTURES_DISABLABLE
bool fetEnableTextures = TRUE;
#endif
bool noDefaultComputerPlayer = FALSE;
bool8 ComputerPlayerEnabled[MAX_MULTIPLAYER_PLAYERS] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };
//udword ComputerPlayerLevel[MAX_MULTIPLAYER_PLAYERS] = {2,2,2,2,2,2,2,2};
bool gatherStats = FALSE;
bool showStatsFight = FALSE;
udword showStatsFightI = 0;
udword showStatsFightJ = 0;
bool showStatsFancyFight = FALSE;
char showStatsFancyFightScriptFile[50];
bool SecretWON = FALSE;
bool forceLAN = FALSE;
bool ShortCircuitWON = FALSE;
#if DBW_TO_FILE
bool debugToFile = FALSE;
#endif

bool debugPacketRecord = FALSE;

bool recordPackets = FALSE;
bool playPackets = FALSE;
bool recordplayPacketsInGame = FALSE;
bool recordFakeSendPackets = FALSE;
char recordPacketFileName[MAX_RECORDPACKETFILENAME_STRLEN];
char recordPacketSaveFileName[MAX_RECORDPACKETFILENAME_STRLEN];

bool autoSaveDebug = FALSE;

#ifdef DEBUG_TACTICS
    bool tacticsOn = TRUE;
#endif
bool noRetreat = FALSE;

bool noPauseAltTab = FALSE;
bool noMinimizeAltTab = FALSE;

//options altered by a password function:
bool mainSinglePlayerEnabled = FALSE;
bool mainEnableSpecialMissions = FALSE;
bool mainScreenShotsEnabled = FALSE;
bool mainCDCheckEnabled = TRUE;

//char versionString[MAX_VERSION_STRING_LEN] = "M23bCGWRC1";
//char versionString[MAX_VERSION_STRING_LEN] = "M23dFinalRC1";
//char versionString[MAX_VERSION_STRING_LEN] = "M24aFinalRC3";
//char versionString[MAX_VERSION_STRING_LEN] = "M24bPublicBeta2";
//char versionString[MAX_VERSION_STRING_LEN] = "M24cFinalRC4";
//char versionString[MAX_VERSION_STRING_LEN] = "M24ePublicBeta2";
//char versionString[MAX_VERSION_STRING_LEN] = "M24fFinalRC5";

#if defined(Downloadable)
//char networkVersion[] =   "DownloadableRC1";
char networkVersion[MAX_NETWORKVERSION_STRING_LEN] =   "DLD_05";
#elif defined(CGW)
char networkVersion[MAX_NETWORKVERSION_STRING_LEN] =   "CGWtronRC1";
#elif defined(OEM)
char networkVersion[MAX_NETWORKVERSION_STRING_LEN] =   "OEMV1";
#else
char networkVersion[MAX_NETWORKVERSION_STRING_LEN] =   "HomeworldV1C";
#endif

#if defined(OEM)
char minorBuildVersion[] = "051";
#else
char minorBuildVersion[] = "06.1";
#endif
char languageVersion[50] = "";    // constructed at beginning of program

// The version string is now constructed as follows:
//
// Network Version TAB Language TAB MinorBuild Version
//
// for example
// "DownloadableRC2\tEnglish\tBuild1"
//
//

// For purposes of games able to play each other on the network, just the networkVersion is consulted
//
// For purposes of whether a patch is required (valid version), the entire versionString (network language build) is consulted
//
// For finding out what directory to get the patch from, only the languageVersion is consulted

char versionString[MAX_VERSION_STRING_LEN] = "";        // constructed at beginning of program


//Windows-related handles
HWND ghMainWindow;
HINSTANCE ghInstance;
UINT uHWCloseMsg;

//location of window, for mouse movement and sizing
POINT WindowTopLeft = {-1, -1};

sdword mainWindowTotalWidth = 0;
sdword mainWindowTotalHeight = 0;

bool noAuthorization = FALSE;

bool determCompPlayer = FALSE;

#if MAIN_SENSOR_LEVEL
udword initialSensorLevel = 0;
#endif

bool pilotView = FALSE;

//data for password-protecting certain options
#if MAIN_Password
char *mainMonthStrings[12] =
{
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};
char mainCompileDate[] = __DATE__;

//password: each character is xor'd with the corresponding character in this string:
ubyte mainOperatorString[] = "132AS sdFmfm na5r\x0\\6234asSDFG,m.";
char mainPassword0[] =
{
    //"BravoCharlie": unlocks single player game only
    '1' ^ 'B',
    '3' ^ 'r',
    '2' ^ 'a',
    'A' ^ 'v',
    'S' ^ 'o',
    ' ' ^ 'C',
    's' ^ 'h',
    'd' ^ 'a',
    'F' ^ 'r',
    'm' ^ 'l',
    'f' ^ 'i',
    'm' ^ 'e',
    ' ' ^ 0,
/*
    'n' ^ '',
    'a' ^ '',
    '5' ^ '',
    'r' ^ '',
    0]  '',
    '\\] ^ '',
    '6' ^ '',
    '2' ^ '',
    '3' ^ '',
    '4' ^ '',
    'a' ^ '',
    's' ^ '',
    'S' ^ '',
    'D' ^ '',
    'F' ^ '',
    'G' ^ '',
    ',' ^ '',
    'm' ^ '',
    '.' ^ '',
*/
};
char mainPassword1[] =
{
    //"Cheeky-Monkey": unlocks all features
    '1' ^ 'C',
    '3' ^ 'h',
    '2' ^ 'e',
    'A' ^ 'e',
    'S' ^ 'k',
    ' ' ^ 'y',
    's' ^ '-',
    'd' ^ 'M',
    'F' ^ 'o',
    'm' ^ 'n',
    'f' ^ 'k',
    'm' ^ 'e',
    ' ' ^ 'y',
    'n' ^ 0,
/*
    'a' ^ '',
    '5' ^ '',
    'r' ^ '',
    0]  '',
    '\\] ^ '',
    '6' ^ '',
    '2' ^ '',
    '3' ^ '',
    '4' ^ '',
    'a' ^ '',
    's' ^ '',
    'S' ^ '',
    'D' ^ '',
    'F' ^ '',
    'G' ^ '',
    ',' ^ '',
    'm' ^ '',
    '.' ^ '',
*/
};
udword mainPasswordChecksum0 =
    ((udword)0x0 ^ (udword)'B') +
    ((udword)0x1 ^ (udword)'r') +
    ((udword)0x2 ^ (udword)'a') +
    ((udword)0x3 ^ (udword)'v') +
    ((udword)0x4 ^ (udword)'o') +
    ((udword)0x5 ^ (udword)'C') +
    ((udword)0x6 ^ (udword)'h') +
    ((udword)0x7 ^ (udword)'a') +
    ((udword)0x8 ^ (udword)'r') +
    ((udword)0x9 ^ (udword)'l') +
    ((udword)0xa ^ (udword)'i') +
    ((udword)0xb ^ (udword)'e');
udword mainPasswordChecksum1 =
    ((udword)0x0 ^ (udword)'C') +
    ((udword)0x1 ^ (udword)'h') +
    ((udword)0x2 ^ (udword)'e') +
    ((udword)0x3 ^ (udword)'e') +
    ((udword)0x4 ^ (udword)'k') +
    ((udword)0x5 ^ (udword)'y') +
    ((udword)0x6 ^ (udword)'-') +
    ((udword)0x7 ^ (udword)'M') +
    ((udword)0x8 ^ (udword)'o') +
    ((udword)0x9 ^ (udword)'n') +
    ((udword)0xa ^ (udword)'k') +
    ((udword)0xb ^ (udword)'e') +
    ((udword)0xc ^ (udword)'y');
char *mainPasswordPtr = NULL;
#endif //MAIN_Password

/*-----------------------------------------------------------------------------
    Leak-tracing strings.  Don't delete any old ones, and comment the
    desitination and date of each string.  Make the string fairly long, and not
    obviously a leak string.
-----------------------------------------------------------------------------*/
//To: Scott Lynch.  Date: Feb 04 98 By: Luke Moloney
//static char leakString[] = "MainObjectWindowMenuHelpOptionsForwardCancelButtonMeshPolygon";
//To: Siearra by CD burned 16 Feb 98 By: Luke Moloney
//static char leakString[] = "MainObjectWindowMenuOptionOptionsForwardButtonCancelMeshPolygon";
//To: Siearra by CD burned 05 Mar 98 By: Luke Moloney
//static char leakString[] = "MainObjectFastMenuOptionOptionsUpwardButtonCancelMeshPolygon";
//To: Siearra by CD burned 10 Mar 98 By: Luke Moloney
//static char leakString[] = "MainObjectFastMenuOptionOptionsOnClickCancelMeshPolygonXZY";
//To: Siearra by CD burned 17 Mar 98 By: Luke Moloney
//static char leakString[] = "MainNonObjectFastMenuSequenceOptionsOnInteractiveCancelMeshPolygonXZY";
//To: Scott, Jim Veevaert,Mark Hood by CD burned 18 Mar 98 By: Luke Moloney
//static char leakString[] = "MainNonObjectFastMenuSequenceOptionsOnInteractiveNomenuMeshPolygonXZY";
//To: Elliot Ness by CD burned 25 Mar 98 By: Luke Moloney
//static char leakString[] = "MainNonObjectFastBackMenuSequenceOptionsHoochyOnInteractiveMeshPolygonXZYWaynot";
//To: Scott Lynch by CD burned 25 Mar 98 By: Luke Moloney
//static char leakString[] = "MainNonGalaxyFastBackMenuSequenceOptionsGrahamOnInteractiveMeshPolygonXZYWaynot";
//To: 'The Press', numbered sequence 1 burned 25 Mar 98 By: Luke Moloney
//static char leakString[] = "ETGGoodBallsBadBoobsDeadBeefBadCafePureSqueezedAsteroidsForRUsInTheNebula";
//To: 'The Press', numbered sequence 2 burned 25 Mar 98 By: Luke Moloney
//static char leakString[] = "ETGGoodBallsBadBloodDeadBoobsBeefBadCafePureFrequencyDomainAsteroidsForRUsInTheNebula";
//To: 'The Press', numbered sequence 3 burned 25 Mar 98 By: Luke Moloney
//static char leakString[] = "Press OK Press Cancel Press Next Press Back Press Forward Error Occurred Internal Error";
//To: 'The Press', numbered sequence 4 burned 25 Mar 98 By: Luke Moloney
//static char leakString[] = "Frame rate: %d, %d, %d\x0Poly count: %d\x0Number dots: %d\x0Bryce 3d usage: %d%%\x0Gaz level: %d";
//To: Scott Lynch, numbered sequence 4 burned 15 May 98 By: Luke Moloney
//static char leakString[] = "Sound event 0x%x ('%s')\n\x0Policy event 0x%x ('%s')\n\x0ETG event 0x%x ('%s')\n\x0";
//To: E3, numbered sequence 4 burned 15 May 98 By: Luke Moloney
//static char leakString[] = "EEPROM version = 0x%x\n\x0Fatal error in file %s, line %d\n%s\nx0Warning in file %s, line %d\n%s\n\x0Notice: %s\n\x0";
//To: Europe(with Alex) numbered EXE 1, burned 12 June 98 By: Luke Moloney
//static char leakString[] = "maxvelocity maxrot  maxhealth\x0regVerify: invalid region 0x%x has va\x0ELASTIC_PULL_FACTOR  CAMERA_OUTBY_SCALE_ZOOM";
//To: Europe(with Alex) numbered EXE 2, burned 12 June 98 By: Luke Moloney
//static char leakString[] = "maxvelocity maxrot  maxhealth\x0regVerify: invalid region 0x%x has va\x0ELASTIC_PULL_FACTOR  CAMERA_OUTBY_SCALE_ZOOM";
//To: Europe(with Alex) numbered EXE 3, burned 12 June 98 By: Luke Moloney
//static char leakString[] = "..\\Game\\commandlayer.c  FALSE   Assertion of (%s) failed.FA_ToggleButton || atom->type == FA_CheckBox || atom->type == FA_RadioButton";
//To: Europe(with Alex) numbered EXE 4, burned 12 June 98 By: Luke Moloney
//static char leakString[] = "..\\Game\\feflow.c    atom != NULL    Assertion of (%s) failed.Closing front end module   Bananna Bananna feScreensLoad: Invalid header '%s'";
//To: Europe(with Alex) numbered EXE 5, burned 12 June 98 By: Luke Moloney
//static char leakString[] = "ObjectWindowMenuOption\x0burned 15 May 98 By: Luke Moloney\x0Warning in file %s, line %d\n%s\n\x0";
//To: Scott Lynch, burned 16 June 98 By: Luke Moloney
//static char leakString[] = "/debug\n/skipFE\n/noBG\n/noFilter\n/noBorder\n/noClamp\n/noTexture\n/noSmooth\n/nilTexture";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP0\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP1\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP2\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP3\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP4\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP5\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP6\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP7\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP8\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Magazines in SanFran, burned 08 July 98 By: Luke Moloney
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\n/tactics\n/noR1\n/noR2\n/noP9\n/noP1\n/noP2\n/noP3\n/noTraders\n";
//To: Scott/doug burned Jul 16 1998 by Luke Moloney.  No Leak String
//To: Stuart Seeley for beta testing burned Jul 16 1998 by Luke Moloney.
//static char leakString[] = "/stipple\n/aiplayerLog\n/demoRecord\n/demoPlay\n/hiRes\n/AllTech\n/nohint\x0ELASTIC_PULL_FACTOR\x0ETG event 0x%x ('%s')\n\x0";
//To: Scott Lynch burned Aug 13 1998 by Luke Moloney.
//static char leakString[] = "/minVal\x0/maxVal\x0/AIFast\x0/speed\x0/800\x0/1024\x0/ComputerAI\x0/LogFileIO\x0/logPackets\x0/encryptPackets";
//To: Game Works Press Show burned Aug 13 1998 by Luke Moloney.
//static char leakString[] = "/minVal\x0/maxVal\x0/AIFast\x0/speed\x0/800x600\x0/1024x768\x0/ComputerAI\x0/LogFileIO\x0/logPackets\x0/encryptPackets";
//To: ECTS (hex edit) burned Sept 3 1998 by Luke Moloney.
//static char leakString[] = "/minVal\x0/maxVal\x0/AIFast\x0/speed\x0/948\x0/1280\x0/ComputerAl\x0/LogFileIO\x0/logPackets\x0/encryptPackets";
//To: "The Press" burned Oct 1 1998 by Luke Moloney. (Hex edit of origional M16 .exe)
//static char leakString[] = "8B D9 81 C1 C4 00 00 00  83 C0 31 3D 93 00 00 00 7C BF 8B 44 24 20 00 00  00 00 98 7C 0E 00 00 80 E2 FE 88 50 35 0F B6 80  BD 0B 00 00 85 C0 75 0A";
//To: Scott and the gang burned Sept 15 1998 by Luke Moloney.
//static char leakString[] = "8B D9 81 C1 C4 00 00 00  83 C0 31 3D 93 00 00 00 7C BF 8B 44 24 20 8A 50  35 89 98 7C 0E 00 00 80 E2 FE 88 50 35 0F B6 80  BD 0B 00 00 85 C0 75 0A";
//To: Scott and the gang burned Oct 15 1998 by Luke Moloney.
//static char leakString[] = "Left click-select\x0Left hold-bandbox\x0Right hold-rotate camra\x0Left/right hold-zoom\x0Centre click-focus\x0Alt-click-focus\x0Control-bandbox-attack\x0";
//To: Scott and the gang burned Nov 24 1998 by Luke Moloney.
//static char leakString[] = "Revolutionary ne 3-d space combat with an advanced camera model and excellent graphics.";
//To: Scott and the gang burned Dec 21 1998 by Luke Moloney.
//static char leakString[] = "BEGLEITFREGATTE\n\x0ANGRIFFSBOMBER\n\x0TRÄGERSCHIFF\n\x0TARNKAPPENJÄGER\n\x0TARNFELDGENERATOR\n\x0DROHNEN-FREGATTE\n";
//To: Scott and the gang burned Dec 21 1998 by Luke Moloney.
//static char leakString[] = "noPlug\x0nisNoLockout\x0nisCounter\x0noPause\x0password\x0smCentreCamera\x0onlyPacking\x0disablePacking\x0disableAVI\x0allowPacking/noPalettes";
//To: Sierra/Sierra QA
//static char leakString[] = "Welcome to Homeworld!\x0Press 'Next' to continue.\x0At any time, you can press 'Back' to go to the previous screen.";
//To: Sierra/Sierra QA burned Dec 26, 1998 by Luke Moloney
//static char leakString[] = "Welcome to Homeworld!\x0Press 'Next' to continue!\x0At any time, you can press 'Back' to go to the previous screen.";
//To: Sierra/Sierra QA burned Feb 1, 1999 by Gary Shaw
//static char leakString[] = "Welcome to Homeworld!\x0Press 'Next' to continue!\x0At any time, you press 'Back' to go to the previous screen.";
//To: Intel: Cathy Avenatti & Mike Minahan burned Feb 3, 1999 by Keith Hentschel
//static char leakString[] = "nisNoLockout\x0nisCounter\x0password\x0noPause\x0smCentreCamera\x0disablePacking\x0onlyPacking\x0disableAVI\x0allowPacking\x0/noPalettes";
//To: Sierra/Sierra QA burned Feb 8, 1999 by Gary Shaw
//static unsigned char leakString[] = { 0xff, 0xd7, 0x23, 0xb6, 0xed, 0x8c, 0xb8, 0xe4, 0x3f, 0xcb, 0x84, 0xde, 0xa6, 0xec, 0x89, 0x80 };
//for the Europe press tour version of the above burn, the last 2 digits, '8980'
//will be incremented by 1 for the number of burns we need by hex-editing the .exe file
//Beta 4 Milestone M21
//static char leakString[] = "nisNoLockout\x0nisCounter\x0passwd\x0noPause\x0smCentreCamera\x0disablePacking\x0onlyPacking\x0disableAVI\x0allowPacking\x0/noPalettes";
//CGDC burn: full of bugs and whatnot.  10 burns to go out.  The default at the end will be incremented for additional burns.
//static char leakString[] = "/miniBeef[ = 0]\x0/lowBeef[ = 1]\x0/highBeef[ = 2]\x0/ultraBeef[ = 3]\x0/megaBeef[ = 4]\x0/insaneBeef[ = 5]\x0/default=00";
//static char leakString[] = "/miniBeef[ = 0]\x0/lowBeef[ = 1]\x0/highBeef[ = 2]\x0/ultraBeef[ = 3]\x0/megaBeef[ = 4]\x0/insaneBeef[ = 5]\x0/default=05";
//M22 burn: Sent to Sierra, Burned by Luke
//for numbered burns, the first number, 0x45, is incremented
//static ubyte leakString[] = {0x45, 0x65, 0x32, 0x89, 0x66, 0x48, 0x78, 0xf9, 0x1a, 0xaa, 0x45, 0x01};
//Demo testing burn.  Burned Apr 05/99 by Luke for Torsten and the crowd.
//static char leakString[] = "/miniBeef[ = 1]\x0/lowBeef[ = 2]\x0/highBeef[ = 3]\x0/ultraBeef[ = 4]\x0/megaBeef[ = 5]\x0/insaneBeef[ = 6]\x0/default=00";
//Downloadable demo (public beta) testing burn.  Burned Apr 05/99 by Luke for Torsten and the crowd.
//static char leakString[] = "MainObjectWindowMenuHelpOptionsForwardCancelButtonMeshPolygon";
//Downloadable demo (public beta) testing burn.  Burned Apr 16/99 by Luke for Torsten and the crowd.
//static char leakString[] = "MainObjectWindowMenuOptionOptionsForwardButtonCancelMeshPolygon";
//Downloadable demo (public beta) testing burn.  Burned Apr 21/99 by Luke for Torsten and the crowd.
//static char leakString[] = "MainObjectFastMenuOptionOptionsOnClickCancelMeshPolygonXZY";
//Downloadable demo (public beta) testing burn.  Burned Apr 28/99 by Luke for Torsten and the crowd.
//static char leakString[] = "MainNonObjectFastBackMenuSequenceOptionsHoochyOnInteractiveMeshPolygonXZYWaynot";
//M23 Burn by Luke for Sierra evaluation
//static char leakString[] = "ETGGoodBallsBadBoobsDeadBeefBadCafePureSqueezedAsteroidsForRUsInTheNebula";
//CGW Burn RC1 by Luke for bug testing
//static char leakString[] = "1001010001001110100011010011101010010100010111010010101010101000011111110001001010";
//E3 Burn RC1 by Luke for bug testing
//static char leakString[] = "1001010001001110100011010011101010010100010111010010101010101000011111110001001010";
//Update Burn RC1 by Luke for bug testing
//static char leakString[] = "01001001010101010101100111001010110101010001010101010100010010101010110101010100101010";
//M24.  2 years now.  How very dragging.  For Mark Hood and Torsten.
//static char leakString[] = "10111101001000100100101010101010001000100110110010111011101011101010010111110101001110";
//Public Beta 2 for torsten and lackies...
//static char leakString[] = "10100101001010100101101011101010001000100110110010111011101011101010010111110101001110";
//M24d Beta 2 Release Candidate 2 to torsten via FTP
//static char leakString[] = "10100101001010100101101011101010001000100010110010101010101010101010010111110101001110";
//M24e Beta 2 Release Candidate 3 to torsten via FTP
//static char leakString[] = "1010010100101010011111111101010001000100010110010101010101010101010010111110101001110";
//M24e Beta 2 Release Candidate 4 to torsten via FTP
//static char leakString[] = "1010110100101010011111111101010001000100010110010101010101010101010010111110101001110";
//For Torsten + focus group: m24ffinalrc5
//static char leakString[] = "101011010010101001111111110101000100010101011000101011101010101010010111110101001110";
//leak strings now defined in LeakyFaucet.h


/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Command-line parsing functions called when a certain flags are set
-----------------------------------------------------------------------------*/
bool HeapSizeSet(char *string)
{
    sscanf(string, "%d", &MemoryHeapSize);
    if (MemoryHeapSize <= MEM_BlockSize * 2)
    {
        MemoryHeapSize = MEM_HeapSizeDefault;
    }
    return TRUE;
}

bool PrependPathSet(char *string)
{
    filePrependPathSet(string);
    FilePathPrepended = TRUE;
    return TRUE;
}

bool CDROMPathSet(char *string)
{
    char message[80];

    if (GetDriveType(string) != DRIVE_CDROM)
    {
        sprintf(message, "'%s' Is not a valid CD-ROM; path ignored.", string);
        MessageBox(NULL, message, "Invalid CD-ROM path", MB_OK | MB_APPLMODAL);
        return FALSE;
    }
    fileCDROMPathSet(string);
    CDROMPathPrepended = TRUE;
    return TRUE;
}

bool EnableFileLoadLog(char *string)
{
    logfileClear(FILELOADSLOG);
    return TRUE;
}

bool SelectDevice(char* string)
{
    memStrncpy(deviceToSelect, string, 16 - 1);
    if (_stricmp(deviceToSelect, "d3d") == 0)
    {
        mainReinitRenderer = 2;
    }
    selectedDEVICE = TRUE;
    return TRUE;
}

bool SelectMSGL(char* string)
{
    memStrncpy(glToSelect, "opengl32.dll", 512 - 1);
    return TRUE;
}

bool SelectD3D(char* string)
{
    memStrncpy(glToSelect, "rgl.dll", 512 - 1);
    memStrncpy(deviceToSelect, "d3d", 16 - 1);
    selectedGL = TRUE;
    selectedDEVICE = TRUE;
    mainReinitRenderer = 2;
    return TRUE;
}

bool EnableRasterSkip(char* string)
{
    mainRasterSkip = TRUE;
    return TRUE;
}

bool EnableDoubleIsTriple(char* string)
{
    mainDoubleIsTriple = TRUE;
    return TRUE;
}

bool DisableFastFrontend(char* string)
{
    mainFastFrontend = !mainFastFrontend;
    return TRUE;
}

bool EnableGatherStats(char *string)
{
    noDefaultComputerPlayer = TRUE;
    return TRUE;
}

bool EnableShowStatsFancyFight(char *string)
{
    showStatsFancyFight = TRUE;
    strcpy(showStatsFancyFightScriptFile,string);
    noDefaultComputerPlayer = TRUE;
    return TRUE;
}

bool EnableShowStatsFight(char *string)
{
    sscanf(string, "%d", &showStatsFightI);

    if ((string = strtok(NULL, TS_Delimiters)) == NULL)
        return TRUE;        //error??
    sscanf(string, "%d", &showStatsFightJ);

    showStatsFight = TRUE;

    noDefaultComputerPlayer = TRUE;
    return TRUE;
}

bool SpecifyLogFilePath(char *string)
{
    strcpy(logFilePath,string);
    return TRUE;
}

/*
void EnableComputerPlayers(char *string)
{
    char playerStr[20];
    char *scanning = playerStr;
    char index;

    if ((string = strtok(NULL, TS_Delimiters)) == NULL)
        return;
    sscanf(string, "%s", playerStr);

    while (*scanning != '\0')
    {
        if ((*scanning >= '0') && (*scanning <= '9'))
        {
            index = (char)(*scanning - '0');
            dbgAssert(index < MAX_MULTIPLAYER_PLAYERS);
            dbgAssert(index >= 0);

            ComputerPlayerEnabled[index] = TRUE;
        }

        scanning++;
    }

    noDefaultComputerPlayer = TRUE;
}
*/
#if 0
bool SetAIPlayerLevels(char *string)
{
    char playerStr[20];
    char *scanning = playerStr;
    char level, i;


    if ((string = strtok(NULL, TS_Delimiters)) == NULL)
        return TRUE;    //error?
    sscanf(string, "%s", playerStr);

    for (i=0;(*scanning != '\0') || (i > MAX_MULTIPLAYER_PLAYERS); )
    {
        if ((*scanning >= '0') && (*scanning <= '9'))
        {
            level = (char)(*scanning - '0');
            dbgAssert(level < (char)AI_NUM_LEVELS);
            dbgAssert(level >= 0);

            ComputerPlayerLevel[i] = level;
            i++;
        }

        scanning++;
    }
    return TRUE;
}
#endif

#if NIS_TEST
extern char *nisTestNIS;
extern char *nisTestScript;
bool TestNISSet(char *string)
{
    static char staticString[256];

    if ((string = strtok(NULL, TS_Delimiters)) == NULL)
        return TRUE;
    memStrncpy(staticString, string, 255);
    nisTestNIS = staticString;
    return TRUE;
}
bool TestNISScriptSet(char *string)
{
    static char staticString[256];

    if ((string = strtok(NULL, TS_Delimiters)) == NULL)
        return TRUE;
    memStrncpy(staticString, string, 255);
    nisTestScript = staticString;
    return TRUE;
}
#endif

#if MAIN_SENSOR_LEVEL
bool InitialSensorLevelSet(char *string)
{
    sscanf(string, "%d", &initialSensorLevel);
    return TRUE;
}
#endif

#if LOD_SCALE_DEBUG
bool EnableLodScaleDebug(char *string)
{
    sscanf(string, "%f", &lodDebugScaleFactor);
    return TRUE;
}
#endif

bool EnableDemoRecord(char *string)
{
    if (!demDemoPlaying)
    {
        demDemoRecording = TRUE;
        strcpy(demDemoFilename, string);
    }
    return TRUE;
}

bool EnableDemoPlayback(char *string)
{
    if (!demDemoRecording)
    {
        wasDemoPlaying = demDemoPlaying = TRUE;
        noPauseAltTab = TRUE;
        strcpy(demDemoFilename, string);
    }
    return TRUE;
}

#if DEM_AUTO_DEMO
bool AutoDemoWaitSet(char *string)
{
    sscanf(string , "%f", &demAutoDemoWaitTime);
    return TRUE;
}
#endif

bool EnablePacketPlay(char *string)
{
    transferCaptaincyDisabled = TRUE;
    strcpy(recordPacketFileName, strtok(NULL, TS_Delimiters));
    return TRUE;
}

bool EnablePacketRecord(char *string)
{
    debugPacketRecord = TRUE;
    return TRUE;
}

bool EnableDebugSync(char *string)
{
    recordPackets = TRUE;
    logEnable = LOG_VERBOSE;
    autoSaveDebug = TRUE;
    return TRUE;
}

bool EnableAutoSaveDebug(char *string)
{
    autoSaveDebug = TRUE;
    return TRUE;
}

bool EnableMiniRes(char* string)
{
    selectedRES = TRUE;
    mainWindowWidth  = 320;
    mainWindowHeight = 240;
    return TRUE;
}

bool EnableLoRes(char *string)
{
    selectedRES = TRUE;
    mainWindowWidth  = 640;
    mainWindowHeight = 480;
    return TRUE;
}

bool EnableHiRes(char *string)
{
    selectedRES = TRUE;
    mainWindowWidth  = 800;
    mainWindowHeight = 600;
    return TRUE;
}

bool EnableMegaRes(char *string)
{
    selectedRES = TRUE;
    mainWindowWidth  = 1024;
    mainWindowHeight = 768;
    return TRUE;
}

bool EnableUltraRes(char* string)
{
    selectedRES = TRUE;
    mainWindowWidth  = 1280;
    mainWindowHeight = 1024;
    return TRUE;
}

bool EnableInsaneRes(char* string)
{
    selectedRES = TRUE;
    mainWindowWidth  = 1600;
    mainWindowHeight = 1200;
    return TRUE;
}

bool Enable32Bit(char* string)
{
    selectedRES = TRUE;
    MAIN_WindowDepth = 32;
    return TRUE;
}

bool Enable16Bit(char* string)
{
    selectedRES = TRUE;
    MAIN_WindowDepth = 16;
    return TRUE;
}

bool Enable24Bit(char* string)
{
    selectedRES = TRUE;
    MAIN_WindowDepth = 24;
    return TRUE;
}

#ifdef GOD_LIKE_SYNC_CHECKING

#define GUESS_NUM_SHIPS     400
bool syncDumpInit(char *string1)
{
    if(sscanf(string1,"%d!%d",&syncDumpWindowSize,&syncDumpGranularity) != 2)
    {
        return FALSE;
    }
    syncDumpOn = TRUE;
    syncDumpWindowPos=0;
    syncDumpGranTrack=0;

    //MemoryHeapSize += sizeof(Ship)*400*syncDumpWindowSize;
    return TRUE;
}
#endif

#if MAIN_Password
bool SetPassword(char *string)
{
    mainPasswordPtr = string;
    return TRUE;
}



/*-----------------------------------------------------------------------------
    Name        : mainPasswordVerify
    Description : Verifies the system password and unlocks some features, or if
                    there is no password, times the game out 30 days after compile.
    Inputs      : string - password string or NULL of none specified.
    Outputs     :
    Return      : NULL if no errors, or error string pointer.
----------------------------------------------------------------------------*/
char *mainPasswordVerify(char *string)
{
    udword index;
    udword checksum;
    time_t time0;
    struct tm *time1;
    char dayString0[16];
    char monthString0[16];
    char monthString1[16];
    sdword day0, hour0, min0, sec0, year0, month0;
    sdword day1, year1, month1;
    sdword nScanned;
    sdword daysOld;

    if (string != NULL)
    {
        //tamper-detect the password
        for (index = checksum = 0; (mainPassword1[index] ^ mainOperatorString[index]) != 0; index++)
        {
            checksum += (udword)index ^ (udword)((ubyte)(mainPassword1[index] ^ mainOperatorString[index]));
        }
        if (checksum != mainPasswordChecksum1)
        {
            //... error: wrong password checksum (tampering?)
            return("Invalid binaries.");
        }
        for (index = checksum = 0; mainPassword1[index] != 0; index++)
        {
            if (mainPassword1[index] != (string[index] ^ mainOperatorString[index]))
            {
                //... error: invalid password
                goto checkNextPassword;
            }
        }
        //... enable screen shots, single player and disable CD check
        mainScreenShotsEnabled = TRUE;
        mainSinglePlayerEnabled = TRUE;
        mainEnableSpecialMissions = TRUE;
        mainCDCheckEnabled = FALSE;
        return(NULL);                                       //they've got a password
checkNextPassword:
        //tamper-detect the password
        for (index = checksum = 0; (mainPassword0[index] ^ mainOperatorString[index]) != 0; index++)
        {
            checksum += (udword)index ^ (udword)((ubyte)(mainPassword0[index] ^ mainOperatorString[index]));
        }
        if (checksum != mainPasswordChecksum0)
        {
            //... error: wrong password checksum (tampering?)
            return("Invalid binaries.");
        }
        for (index = checksum = 0; mainPassword0[index] != 0; index++)
        {
            if (mainPassword0[index] != (string[index] ^ mainOperatorString[index]))
            {
                //... error: invalid password
                return("Invalid password.");
            }
        }
        //... enable single player only
        mainSinglePlayerEnabled = TRUE;
        mainEnableSpecialMissions = TRUE;
        return(NULL);                                       //they've got a password
    }
    else
    {                                                       //no password specified: see if it times out
        //tamper-detect the date
/*
        for (index = checksum = 0; index < 10; index++)
        {
            checksum += mainCompileDate[index] ^ index;
        }
        if (checksum != mainCompileDateChecksum)
        {
            //... error: wrong date (tampering?)
            return("Invalid binaries/data.");
        }
*/
        //compare current date to compile date and determine time expired
        time(&time0);
        time1 = gmtime(&time0);
        nScanned = sscanf(asctime(time1), "%s %s %d %d:%d:%d %d", dayString0, monthString0, &day0, &hour0, &min0, &sec0, &year0);
        if (nScanned != 7)
        {
            //... error: bad scan
            return("Bad scan xx(1)");
        }
        nScanned = sscanf(mainCompileDate, "%s %d %d", monthString1, &day1, &year1);
        if (nScanned != 3)
        {
            //... error: bad scan
            return("Bad scan xx(2)");
        }
        month0 = month1 = -1;
        for (index = 0; index < 12; index++)
        {
            if (!_stricmp(monthString0, mainMonthStrings[index]))
            {
                month0 = index;
            }
            if (!_stricmp(monthString1, mainMonthStrings[index]))
            {
                month1 = index;
            }
        }
        if (month0 == -1 || month1 == -1)
        {
            //... error: bad month string
            return("I hate this month");
        }
        daysOld = 365 * (year1 - year0) + 30 * (month1 - month0) + (day1 - day0);
        //        ^^^                     ^^
        // I know 30 isn't the length of all months, but let's just assume for now
        //
        if (daysOld > MAIN_ExpiryTime)
        {
            //... error: Evaluation copy expired
            return("Skynyrd!");
        }
    }
    return(NULL);
}
#endif //MAIN_Password

/*-----------------------------------------------------------------------------
    Structures used for command paramters and help
-----------------------------------------------------------------------------*/
#define entryFn(command, func, help)        {COF_Visible, command, func, NULL, 0, help}
#define entryVr(command, var, value, help)  {COF_Visible, command, NULL, &var, value, help}
#define entryFV(command, func, var, value, help)   {COF_Visible, command, func, &var, value, help}
#define entryFnParam(command, func, help)        {COF_Visible | COF_NextToken, command, func, NULL, 0, help}
#define entryFVParam(command, func, var, help)   {COF_Visible | COF_NextToken, command, func, &var, value, help}
#define entryFnHidden(command, func, help)        {0, command, func, NULL, 0, help}
#define entryVrHidden(command, var, value, help)  {0, command, NULL, &var, value, help}
#define entryFVHidden(command, func, var, value, help)   {0, command, func, &var, value, help}
#define entryFnParamHidden(command, func, help)        {COF_NextToken, command, func, NULL, 0, help}
#define entryFVParamHidden(command, func, var, help)   {COF_NextToken, command, func, &var, value, help}
#define entryComment(comment)               {COF_Visible, comment, NULL, NULL, 0, NULL}
commandoption commandOptions[] =
{
#ifndef HW_Release
    entryComment("DEBUGGING OPTIONS"),//-----------------------------------------------------
    entryVr("/debug",               DebugWindow, TRUE,                  " - Enable debug window."),
    entryVr("/nodebugInt",          dbgInt3Enabled,FALSE,               " - Fatal errors don't genereate an int 3 before exiting."),
#if DBW_TO_FILE
    entryVr("/debugToFile",         debugToFile, TRUE,                  " - output debugging info to a file."),
#endif
#else
    entryVrHidden("/debug",         DebugWindow, TRUE,                  " - Enable debug window."),
    entryVrHidden("/nodebugInt",    dbgInt3Enabled,FALSE,               " - Fatal errors don't genereate an int 3 before exiting."),
#if DBW_TO_FILE
    entryVrHidden("/debugToFile",   debugToFile, TRUE,                  " - output debugging info to a file."),
#endif
#endif
#if RAN_DEBUG_CALLER
    entryVr("/ranCallerDebug",      ranCallerDebug, TRUE,               " - debug non-deterministic calling of random numbers."),
#endif
#ifndef HW_Release
    entryFn("/autosavedebug",       EnableAutoSaveDebug,                " autosaves game frequently"),
#endif

    entryComment("SYSTEM OPTIONS"), //-----------------------------------------------------
    entryFnParam("/heap",           HeapSizeSet,                        " <n> - Sets size of global memory heap to [n]."),
    entryFnParam("/prepath",        PrependPathSet,                     " <path> - Sets path to search for opening files."),
    entryFnParam("/CDpath",         CDROMPathSet,                       " <path> - Sets path to CD-ROM in case of ambiguity."),
#if MAIN_MOUSE_FREE
#ifndef HW_Release
    entryVr("/freemouse",           startupClipMouse, FALSE,            " - Mouse free to move about entire screen at startup.  Use <CTRL>F11 to toggle during play."),
#else
    entryVrHidden("/freemouse",     startupClipMouse, FALSE,            " - Mouse free to move about entire screen at startup.  Use <CTRL>F11 to toggle during play."),
#endif
#endif
#ifndef HW_Release
    entryVr("/ignoreBigfiles",      IgnoreBigfiles, TRUE,               " - don't use anything from bigfile(s)"),
    entryFV("/logFileLoads",        EnableFileLoadLog,LogFileLoads,TRUE," - create log of data files loaded"),
#endif

    entryComment("PROCESSOR OPTIONS"),//-----------------------------------------------------
    entryVr("/enableSSE",           mainAllowKatmai, TRUE,              " - allow use of SSE if support is detected."),
    entryVr("/forceSSE",            mainForceKatmai, TRUE,              " - force usage of SSE even if determined to be unavailable."),
    entryVr("/enable3DNow",         mainAllow3DNow, TRUE,               " - allow use of 3DNow! if support is detected."),

    entryComment("SOUND OPTIONS"),  //-----------------------------------------------------
#if SE_DEBUG
    entryVr("/noSound",             enableSFX, FALSE,                   " - turn all sound effects off."),
    entryVr("/noSpeech",            enableSpeech, FALSE,                " - turn all speech off."),
#endif
    entryVr("/dsound",              useDSound, TRUE,                    " - forces mixer to write to DirectSound driver, even if driver reports not certified."),
    entryVr("/dsoundCoop",          coopDSound, TRUE,                   " - switches to co-operative mode of DirectSound (if supported) to allow sharing with other applications."),
    entryVr("/waveout",             useWaveout, TRUE,                   " - forces mixer to write to Waveout even if a DirectSound supported object is available."),
    entryVr("/reverseStereo",       reverseStereo, TRUE,                " - swap the left and right audio channels."),

    entryComment("DETAIL OPTIONS"), //-----------------------------------------------------
    entryFn("/rasterSkip",          EnableRasterSkip,                   " - enable interlaced display with software renderer."),
    entryVr("/noBG",                showBackgrounds, FALSE,             " - disable display of galaxy backgrounds."),
    entryVr("/noFilter",            texLinearFiltering,FALSE,           " - disable bi-linear filtering of textures."),
    entryVr("/noSmooth",            enableSmoothing, FALSE,             " - do not use polygon smoothing."),
#if TR_NIL_TEXTURE
    entryVr("/nilTexture",          GLOBAL_NO_TEXTURES,TRUE,            " - don't ever load textures at all."),
#endif
#if ETG_DISABLEABLE
    entryVr("/noEffects",           etgEffectsEnabled,FALSE,            " - disable all effects (Debug only)."),
#endif
#if FE_TEXTURES_DISABLABLE
    entryVr("/NoFETextures",        fetEnableTextures, FALSE,           " - turns off front end textures"),
#endif
    entryVr("/stipple",             enableStipple, TRUE,                " - enable stipple alpha with software renderer."),
    entryVr("/noShowDamage",        gShowDamage, FALSE,                 " - Disables showing ship damage effects."),

    entryComment("VIDEO MODE OPTIONS"),//-----------------------------------------------------
    entryVr("/safeGL",              mainSafeGL, TRUE,                   " - don't use possibly buggy optimized features of OpenGL for rendering."),
    entryFn("/triple",              EnableDoubleIsTriple,               " - use when frontend menus are flickering madly."),
    entryVr("/nodrawpixels",        mainNoDrawPixels, TRUE,             " - use when background images don't appear while loading."),
    entryVr("/noswddraw",           mainSoftwareDirectDraw, FALSE,      " - don't use DirectDraw for the software renderer."),
    entryVr("/noglddraw",           mainDirectDraw, FALSE,              " - don't use DirectDraw to setup OpenGL renderers."),
    entryVr("/sw",                  mainForceSoftware, TRUE,            " - reset rendering system to defaults at startup."),
    entryVrHidden("/noSavedMode",   mainAutoRenderer, FALSE,            " - disable recovery of previous display mode."),
    entryFn("/noFastFE",            DisableFastFrontend,                " - disable fast frontend rendering."),
    entryVr("/fullscreen",          fullScreen, TRUE,                   " - display fullscreen with software renderer (default)."),
    entryVr("/window",              fullScreen, FALSE,                  " - display in a window."),
    entryVr("/noBorder",            showBorder, FALSE,                  " - no border on window."),
    entryVrHidden("/d3dDeviceCRC",  mainOutputCRC, TRUE,                " - generate d3dDeviceCRC.txt for video troubleshooting."),
    entryFnHidden("/minny",           EnableMiniRes,                      " - run at 320x240 resolution."),
    entryFn("/640",                 EnableLoRes,                        " - run at 640x480 resolution (default)."),
    entryFn("/800",                 EnableHiRes,                        " - run at 800x600 resolution."),
    entryFn("/1024",                EnableMegaRes,                      " - run at 1024x768 resolution."),
    entryFn("/1280",                EnableUltraRes,                     " - run at 1280x1024 resolution."),
    entryFn("/1600",                EnableInsaneRes,                    " - run at 1600x1200 resolution."),
//    entryFn("/d16",                 Enable16Bit,                        " - run in 16 bits of colour."),
//    entryFn("/d24",                 Enable24Bit,                        " - run in 24 bits of colour."),
//    entryFn("/d32",                 Enable32Bit,                        " - run in 32 bits of colour."),
//    entryVr("/truecolor",           trueColor, TRUE,                    " - try 24bit modes before 15/16bit."),
//    entryVr("/slowBlits",           slowBlits, TRUE,                    " - use slow screen blits if the default is buggy."),
    entryFnParam("/device",         SelectDevice,                       " <dev> - select an rGL device by name, eg. sw, fx, d3d."),
//    entryFV("/gl",                  SelectMSGL, selectedGL, TRUE,       " - select default OpenGL as renderer."),
//    entryFn("/d3d",                 SelectD3D,                          " - select Direct3D as renderer."),
    entryVr("/nohint",              mainNoPerspective, TRUE,            " - disable usage of OpenGL perspective correction hints."),
    entryVrHidden("/noPause",             noPauseAltTab, TRUE,                " - don't pause when you alt-tab."),
    entryVrHidden("/noMinimize",          noMinimizeAltTab, TRUE,             " - don't minimize when you alt-tab."),

#ifndef HW_Release
    entryComment("CHEATS AND SHORTCUTS"),         //-----------------------------------------------------
#if CM_CHEAP_SHIPS
    entryVr("/cheapShips",          cmCheapShips, TRUE,                 " - ships only cost 1 RU."),
#endif
#if MAIN_SENSOR_LEVEL
    entryFnParam("/sensorLevel" ,   InitialSensorLevelSet,              " <n> - set initial sensors level (0.. 2).  Default is 0."),
#endif
    entryVr("/noCompPlayer",        noDefaultComputerPlayer, TRUE,      " - disable default computer players"),
#ifdef DEBUG_TACTICS
    entryVr("/notactics",           tacticsOn, FALSE,                   " - Disables tactics."),
#endif
    entryVr("/noretreat",           noRetreat, TRUE,                    " - disables the 'retreat' feature of tactics"),
#endif
    entryVrHidden("/disableAVI",    enableAVI,FALSE,                    " - don't display intro sequences."),

#if RENDER_LIGHTLINES
    entryComment("VISUALIZATION"),  //-----------------------------------------------------
    entryVr("/dockLines",           dockLines, TRUE,                    " - show dock lines."),
    entryVr("/gunLines",            gunLines, TRUE,                     " - show gun lines."),
    entryVr("/lightLines",          RENDER_LIGHTLINES, TRUE,            " - show light lines (Debug only)."),
    entryVr("/boxes",               RENDER_BOXES, TRUE,                 " - render bounding bowties on the ships."),
#endif
#if CL_TEXTFEEDBACK
    entryVr("/textFeedback",        enableTextFeedback, TRUE,           " - enable text feedback for in game commands."),
#endif
#if TR_DEBUG_TEXTURES
    entryVr("/specialTextures",     trSpecialTextures, TRUE,            " - enable special debugging textures."),
#endif
#if MESH_MORPH_DEBUG
    entryVr("/morphDebug",          meshMorphDebug, TRUE,               " - enable debugging of morphed mesh rendering code."),
#endif
#if LOD_SCALE_DEBUG
    entryFnParam("/lodScaleDebug",  EnableLodScaleDebug,                " - enable fixing a LOD scale factor."),
#endif
#if MR_CAN_FOCUS_ROIDS
    entryVr("/focusRoids",          mrCanFocusRoids, TRUE,              " - enable focussing on asteroids and dust clouds."),
#endif
#if PIE_VISUALIZE_EXTENTS
    entryVr("/showExtents",         pieVisualizeExtents, TRUE,          " - draw elliptical universe extents."),
#endif
#if UNIV_SHIP_LOADFREE_LOG
    entryVr("/loadFreeLog",         univLoadFreeLog, TRUE,              " - enable logging of what was loaded and freed between missions."),
#endif
#ifndef HW_Release
    entryVr("/NoBind",              bkDisableKeyRemap, TRUE,            " - disable key bindings so that debug keys work."),
#else
    entryVrHidden("/NoBind",        bkDisableKeyRemap, TRUE,            " - disable key bindings so that debug keys work."),
#endif

#ifndef HW_Release
    entryComment("COMPUTER PLAYER AND STATS"),//-----------------------------------------------------
//    {"/compPlayer",  EnableComputerPlayers, "=01234567 to enable all computer players"},
    entryVr("/aiplayerLog",         aiplayerLogEnable, TRUE,            " - enable AI Player Logging"),
    entryVr("/determCompPlayer",    determCompPlayer, TRUE,             " - makes computer players deterministic"),
    entryFV("/gatherStats",         EnableGatherStats, gatherStats, TRUE,"- enable gathering of stats"),
    entryFnParam("/showStatsFight", EnableShowStatsFight,               "=<i,j> to show stats fight i,j"),
    entryFnParam("/showStatsFancyFight", EnableShowStatsFancyFight,     "=filename.script"),
#endif

#ifndef HW_Release
    entryComment("NETWORK PLAY"),   //-----------------------------------------------------
    //entryVr("/captaincyLogOff",     captaincyLogEnable, FALSE,          " - turns off captaincy log file" ),
    //entryVr("/captaincyLogOn",      captaincyLogEnable, TRUE,           " - turns on captaincy log file" ),
    entryVr("/logOff",              logEnable, LOG_OFF,                 " - turns of network logging file"),
    entryVr("/logOn",               logEnable, LOG_ON,,                 " - turns network logging file on"),
    entryVr("/logOnVerbose",        logEnable, LOG_VERBOSE,             " - turns verbose network logging file on"),
    entryFnParam("/logFilePath",    SpecifyLogFilePath,                 "=filepath.txt"),
    entryFn("/debugSync",           EnableDebugSync,                    " autosaves game frequently, records packets, logonverbose" ),
    entryVrHidden("/noWon",         SecretWON, TRUE,                    " - no WON stuff" ),
    entryVr("/forceLAN",            forceLAN, TRUE,                     " - allow LAN play regardless of version" ),
    entryVrHidden("/noAuth",        noAuthorization, TRUE,              " - Disables WON Login"),
    entryVrHidden("/shortWon",      ShortCircuitWON, TRUE,              " - short circuit WON stuff" ),
#else
    entryVrHidden("/logOff",        logEnable, LOG_OFF,                 " - turns of network logging file"),
    entryVrHidden("/logOn",         logEnable, LOG_ON,,                 " - turns network logging file on"),
    entryVrHidden("/logOnVerbose",  logEnable, LOG_VERBOSE,             " - turns verbose network logging file on"),
    entryFnParamHidden("/logFilePath",SpecifyLogFilePath,               "=filepath.txt"),
//!!!shortwon in release mode!!!    entryVrHidden("/shortWon",      ShortCircuitWON, TRUE,              " - short circuit WON stuff" ),
#endif
#ifdef DEBUG_GAME_STATS
    entryVr("/statLogOn",           statLogOn, TRUE,                    " - generates game stats log file"),
#endif
#ifdef GOD_LIKE_SYNC_CHECKING
    entryFnParam("/BryceAndDrewAreGods", syncDumpInit,                  "=<X>!<Y>   X = size of SyncDumpWindow  Y = granularity in universe Frames"),
#endif

#if NIS_PRINT_INFO
    entryComment("NIS OPTIONS"),    //-----------------------------------------------------
#endif
#if NIS_TEST
    entryFn("/testNIS" ,            TestNISSet,                         " <nisFile> - enables NIS testing mode using [nisFile]."),
    entryFn("/testNISScript",       TestNISScriptSet,                   " <scriptFile> - enables NIS testing mode using [scriptFile]."),
#endif
#if NIS_PRINT_INFO
    entryVr("/nisCounter",          nisPrintInfo,TRUE,                  " - display nis time index info by default."),
    entryVr("/nisNoLockout",        nisNoLockout, TRUE,                 " - don't lock out the interface when playing an NIS."),
#endif

/*
    entryComment("RECORDED DEMOS"), //-----------------------------------------------------
    entryFnParam("/demoRecord",     EnableDemoRecord,                   " <fileName> - record a demo."),
    entryFnParam("/demoPlay",       EnableDemoPlayback,                 " <fileName> - play a demo."),
*/
#ifndef HW_Release
    entryFV("/packetRecord",        EnablePacketRecord, recordPackets, TRUE, " - record packets of this multiplayer game"),
    entryFV("/packetPlay",          EnablePacketPlay, playPackets, TRUE," <fileName> - play back packet recording"),
#else
    entryFVHidden("/packetRecord",  EnablePacketRecord, recordPackets, TRUE, " - record packets of this multiplayer game"),
    entryFVHidden("/packetPlay",    EnablePacketPlay, playPackets, TRUE," <fileName> - play back packet recording"),
#endif

    //entryVr("/compareBigfiles",     CompareBigfiles, TRUE,              " - file by file, use most recent (bigfile/filesystem)"),
#if DEM_AUTO_DEMO
    entryVr("/disableAutoDemos",    demAutoDemo,FALSE,                  " - don't automatically play demos."),
    entryFnParam("/autoDemoWait",   AutoDemoWaitSet,                    " <seconds> - time to wait on main screen before starting a demo."),
#endif
#if DEM_FAKE_RENDER_SWITCH
    entryVr("/disableFakeRenders",  demFakeRenders,FALSE,               " - disable feature where playback will try to keep up with recorded demo."),
#endif

    entryComment("TEXTURES"),       //-----------------------------------------------------
    entryVr("/nopal",               mainNoPalettes, TRUE,               " - disable paletted texture support."),
/*
    entryVrHidden("/allowPacking",  mainAllowPacking, TRUE,             " - use the packed textures if available (default)."),
    entryVr("/disablePacking",      mainAllowPacking, FALSE,            " - don't use the packed textures if available."),
#ifndef HW_Release
    entryVr("/onlyPacking",         mainOnlyPacking, TRUE,              " - only display packed textures."),
#endif
*/

    entryComment("MISC OPTIONS"),   //-----------------------------------------------------
    entryVrHidden("/smCentreCamera",      smCentreWorldPlane, FALSE,          " - centres the SM world plane about 0,0,0 rather than the camera."),
#if MAIN_Password
    entryFnParam("/password",       SetPassword,                        " <password> - specify password to enable certain features."),
#endif
#if RND_PLUG_DISABLEABLE
    entryVr("/noPlug",              rndShamelessPlugEnabled, FALSE,     " - don't display relic logo on pause."),
#endif

    entryVrHidden("/closeCaptioned",      subCloseCaptionsEnabled, TRUE,      " - close captioned for the hearing impared."),
    entryVr("/pilotView",           pilotView, TRUE, " - enable pilot view.  Focus on single ship and hit Q to toggle."),
    {0, NULL, NULL}
};

/*-----------------------------------------------------------------------------
    Command-line parsing functions to display a help message box.
-----------------------------------------------------------------------------*/
char *gHelpString;
BOOL CALLBACK CommandLineFunction(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, IDC_CommandLine, gHelpString);
            //... init the edit box
            return(TRUE);
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hDlg, 0);
            }
            return(TRUE);
        default:
            break;
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : DebugHelpDefault
    Description : Print out command-line usage help to a dialog box
    Inputs      : string - the unrecognized command-line switch that causes this help.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DebugHelpDefault(char *string)
{
    sdword index, length;

    //calc length of help string
    length = strlen("Invalid or unrecognised command line option: '%s'\x0d\x0a");
    length += strlen(string);
    for (index = 0; commandOptions[index].parameter; index++)
    {
        if (!bitTest(commandOptions[index].flags, COF_Visible))
        {
            continue;
        }
        if (commandOptions[index].helpString == NULL)
        {                                                   //no help string: it's a comment
            length += strlen(commandOptions[index].parameter) + 2;//just length of comment
        }
        else
        {                                                   //else it's a real command line option
            length += strlen(commandOptions[index].helpString); //length of help string
            length += strlen(commandOptions[index].parameter) + MCL_IndentSpace;//parameter string
        }
        length += 2;                                        //and a newline
    }

    if ((gHelpString = malloc(length)) == NULL)              //allocate string
    {
        MessageBox(NULL, "Cannot allocate memory for help.", "Command-Line Usage", MB_OK | MB_APPLMODAL);
        return;
    }

    //copy all help strings into one
    sprintf(gHelpString, "Invalid or unrecognised command line option: '%s'\x0d\x0a", string);
    for (index = 0; commandOptions[index].parameter; index++)
    {
        if (!bitTest(commandOptions[index].flags, COF_Visible))
        {
            continue;
        }
        if (commandOptions[index].helpString == NULL)
        {                                                   //no help string: it's a comment
            strcat(gHelpString, "\x0d\x0a");
            strcat(gHelpString, commandOptions[index].parameter);
            strcat(gHelpString, "\x0d\x0a");
        }
        else
        {                                                   //else it's a real command line option
            sprintf(gHelpString + strlen(gHelpString), "    %s%s\x0d\x0a", commandOptions[index].parameter, commandOptions[index].helpString);
            //                                            ^ MCL_IndentSpace spaces
        }
    }

    DialogBox(ghInstance, MAKEINTRESOURCE(IDD_CommandLine), NULL, CommandLineFunction);
    free(gHelpString);                                       //done with string, free it
}

/*-----------------------------------------------------------------------------
    Name        : ProcessCommandLine
    Description : Tokenize and process all defined command-lien flags
    Inputs      : commandLine - command-line string, sans executable name
    Outputs     : sets command-line switch variables
    Return      : void
    Remarks:
        Here are the command-line switches for Homeworld:
            -debug: Enable debug window by setting the DebugWindow flag
----------------------------------------------------------------------------*/
sdword ProcessCommandLine(char *commandLine)
{
    char *string, *nextString;
    sdword index;

    string = strtok(commandLine, TS_Delimiters);            //initial tokenize

    while (string != NULL)
    {
        if (string[0] == '-')
        {
            string[0] = '/';
        }
        for (index = 0; commandOptions[index].parameter; index++)
        {
            if (commandOptions[index].helpString == NULL)
            {                                               //don't compare against comment lines
                continue;
            }
            if (!_stricmp(string, commandOptions[index].parameter))
            {                                               //if this is the correct option
                dbgAssert(commandOptions[index].variableToModify || commandOptions[index].function);
                if (commandOptions[index].variableToModify != NULL)
                {                                           //set a variable if applicable
                    *((udword *)(commandOptions[index].variableToModify)) = commandOptions[index].valueToSet;
                }
                if (commandOptions[index].function != NULL)
                {                                           //call the function associated, if applicable
                    if (bitTest(commandOptions[index].flags, COF_NextToken))
                    {                                       //does the function take next token as a parameter?
                        nextString = strtok(NULL, TS_Delimiters);//get next token
                        if (nextString == NULL)
                        {                                   //if no next token
                            break;                          //print usage
                        }
                        string = nextString;
                    }
                    if(!commandOptions[index].function(string))
                    {
                        //error occured in parsing function
                        break;
                    }
                }
                goto stringFound;
            }
        }
        DebugHelpDefault(string);                           //no string found, print help
        return(-1);
stringFound:;
        string = strtok(NULL, TS_Delimiters);               //get next token
    }
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : CommandProcess
    Description : processes command messages sent to WindowProc
    Inputs      : Same as WindowProc without the message WM_COMMAND integer
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void CommandProcess(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch(wParam)
    {
        case CID_ExitError:                                 //error message posted, exit with a message box
            SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE |
                SWP_NOZORDER | SWP_HIDEWINDOW);
            MessageBox(hWnd, dbgFatalErrorString, "Fatal Error", MB_ICONSTOP | MB_OK);

            PostMessage(hWnd, WM_CLOSE, CID_ExitError, 0);
            break;
        case CID_ExitOK:
            PostMessage(hWnd, WM_CLOSE, CID_ExitOK, 0);
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mainDevStatsInit
    Description : initialize the devstats table.  this table contains features
                  that need to be disabled on particular (D3D) devices
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainDevStatsInit(void)
{
    filehandle handle;
    char string[512];
    crc32 crc;
    udword flags0, flags1, flags2;
    sdword size, index;

    handle = fileOpen("devstats.dat", FF_IgnorePrepend | FF_TextMode | FF_IgnoreBIG);
    for (devTableLength = 0;;)
    {
        if (fileLineRead(handle, string, 511) == FR_EndOfFile)
        {
            break;
        }
        if (string[0] == ';')
        {
            continue;
        }
        if (strlen(string) < 4)
        {
            continue;
        }

        //xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx ...
        sscanf(string, "%X %X %X %X", &crc, &flags0, &flags1, &flags2);
        if (crc != 0)
        {
            devTableLength++;
        }
    }
    fileClose(handle);

    if (devTableLength > 0)
    {
        size = 4 * sizeof(udword) * devTableLength;
        devTable = (udword*)malloc(size);
        if (devTable == NULL)
        {
            dbgFatal(DBG_Loc, "mainDevStatsInit couldn't allocate memory for devTable");
        }
        memset(devTable, 0, size);

        handle = fileOpen("devstats.dat", FF_IgnorePrepend | FF_TextMode | FF_IgnoreBIG);
        for (index = 0;;)
        {
            if (fileLineRead(handle, string, 511) == FR_EndOfFile)
            {
                break;
            }
            if (string[0] == ';')
            {
                continue;
            }
            if (strlen(string) < 4)
            {
                continue;
            }

            //xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx ...
            sscanf(string, "%X %X %X %X", &crc, &flags0, &flags1, &flags2);
            if (crc != 0)
            {
                devTable[index+0] = crc;
                devTable[index+1] = flags0;
                devTable[index+2] = flags1;
                devTable[index+3] = flags2;
                index += 4;
            }
        }
        fileClose(handle);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mainDevStatsShutdown
    Description : release memory used by the devstats table
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainDevStatsShutdown(void)
{
    if (devTable != NULL)
    {
        free(devTable);
        devTable = NULL;
    }
}

void IntroDeactivateMe(HWND hWnd)
{
    if (gl95)
    {
        return;                                             //don't allow the program to deactivate under Windows 95
    }
    sounddeactivate(TRUE);

    if (!binkDonePlaying)
    {
        binkPause(TRUE);
        hwSetRes(0, 0, 0);
    }

    if (!noMinimizeAltTab && utyTest(SS2_SystemStarted))
    {
        ShowWindow(hWnd, SW_SHOWMINNOACTIVE);
    }
    wasDemoPlaying = demDemoPlaying;                        //save demo playback state
    demDemoPlaying = FALSE;                                 //stop the demo playback for now

#if MAIN_MOUSE_FREE
    if (utySystemStarted)
    {
        utyClipMouse(FALSE);
    }
#endif
    utyTaskTimerClear();                                    //clear pending timer ticks
    keyClearAll();
    keyBufferClear();
    systemActive = FALSE;
}

void IntroActivateMe(HWND hWnd)
{
    sounddeactivate(FALSE);

    hwSetRes(-1, -1, -1);

    if (!noMinimizeAltTab && utyTest(SS2_SystemStarted))
    {
        ShowWindow(hWnd, SW_RESTORE);
    }
    demDemoPlaying = wasDemoPlaying;                        //keep playing demo if it was playing when we minimized
#if MAIN_MOUSE_FREE
    if (utySystemStarted)
    {
        utyClipMouse(startupClipMouse);                     //optionally trap the mouse
    }
#endif
    systemActive = TRUE;

    ShowWindow(hWnd, SW_RESTORE);
    utyForceTopmost(fullScreen);
    ShowCursor(FALSE);
    //make sure that the mouse is rotating properly when we come back
    utyMouseButtonsClear();

    keyClearAll();
    keyBufferClear();
    mrTacticalOverlayState(utyCapsLockToggleState());

    if (!binkDonePlaying)
    {
        binkPause(FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : DeactivateMe
    Description : deactivates this application and suspends processing
    Inputs      : hWnd
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DeactivateMe(HWND hWnd)
{
    if (gl95)
    {
        return;                                             //don't allow the program to deactivate under Windows 95
    }
    sounddeactivate(TRUE);

    if (RGL)
    {
        rglFeature(RGL_DEACTIVATE);
    }
    else
    {
        (void)hwActivate(FALSE);
    }
    if (!noMinimizeAltTab && utyTest(SS2_SystemStarted))
    {
        ShowWindow(hWnd, SW_SHOWMINNOACTIVE);
    }
    wasDemoPlaying = demDemoPlaying;                        //save demo playback state
    demDemoPlaying = FALSE;                                 //stop the demo playback for now

#if MAIN_MOUSE_FREE
    utyClipMouse(FALSE);
#endif
    utyTaskTimerClear();                                    //clear pending timer ticks
    if (multiPlayerGame || noPauseAltTab)
    {                                                       //only stop rendering if it's single player
        taskSavePauseStatus();
        taskPause(utyRenderTask);
        taskPause(regTaskHandle);
    }
    else
    {
        taskFreezeAll();                                    //stop all tasks
    }
    keyClearAll();
    keyBufferClear();
    systemActive = FALSE;

    if (!binkDonePlaying)
    {
        binkPause(TRUE);
    }
    /*
    if (utyTest2(SS2_ToggleKeys))
    {
        utyToggleKeyStatesRestore();
    }
    */
}

/*-----------------------------------------------------------------------------
    Name        : ActivateMe
    Description : activates this application and resumes processing
    Inputs      : hWnd
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ActivateMe(HWND hWnd)
{
    sounddeactivate(FALSE);

    if (!noMinimizeAltTab && utyTest(SS2_SystemStarted))
    {
        ShowWindow(hWnd, SW_RESTORE);
    }
    demDemoPlaying = wasDemoPlaying;                        //keep playing demo if it was playing when we minimized
    if (RGL)
    {
        rglFeature(RGL_ACTIVATE);
        mainReinitRenderer = 2;
    }
    else
    {
        (void)hwActivate(TRUE);
    }
    if (glcActive())
    {
        glcRenderEverythingALot();
    }
    else
    {
        feRenderEverything = TRUE;
    }
#if MAIN_MOUSE_FREE
    if (utySystemStarted)
    {
        utyClipMouse(startupClipMouse);                     //optionally trap the mouse
    }
#endif
    if (utySystemStarted)
    {                                                       //if game has started
        taskResumeAll();                                    //resume all tasks
    }
    systemActive = TRUE;

    ShowWindow(hWnd, SW_RESTORE);
    utyForceTopmost(fullScreen);
    ShowCursor(FALSE);
    //make sure that the mouse is rotating proper when we come back
    utyMouseButtonsClear();

    keyClearAll();
    keyBufferClear();

    hrBackgroundReinit = TRUE;
    /*
    if (utyTest2(SS2_ToggleKeys))
    {
        utyToggleKeyStatesSave();
    }
    */
    mrTacticalOverlayState(utyCapsLockToggleState());

    if (!binkDonePlaying)
    {
        binkPause(FALSE);
    }
}

static bool mainFileExists(char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        return FALSE;
    }
    else
    {
        fclose(file);
        return TRUE;
    }
}

static bool mainFind3DfxGL(char* path)
{
    char* dir;
    char  subdir[8];

    if (glCapNT())
    {
        strcpy(subdir, "WinNT");
    }
    else
    {
        strcpy(subdir, "Win9x");
    }

    //try cwd\3dfx\opengl32.dll first
    sprintf(path, "3dfx\\%s\\opengl32.dll", subdir);
    if (mainFileExists(path))
    {
        return TRUE;
    }

    //try $HW_Root\dll\opengl32.dll next
    dir = getenv("HW_Root");
    if (dir == NULL)
    {
        //not found, use default opengl
        strcpy(path, "opengl32.dll");
        return FALSE;
    }
    else
    {
        char concatdir[128];
        strcpy(path, dir);
        if (path[strlen(path)-1] == '\\')
        {
            sprintf(concatdir, "dll\\%s\\opengl32.dll", subdir);
        }
        else
        {
            sprintf(concatdir, "\\dll\\%s\\opengl32.dll", subdir);
        }
        strcat(path, concatdir);
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mainFreeLibrary
    Description : release references to a given .dll
    Inputs      : libname - name of the library
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void mainFreeLibrary(char* libname)
{
    HINSTANCE lib;

    lib = GetModuleHandle(libname);
    if (lib != NULL)
    {
        while (FreeLibrary(lib)) ;
    }
}

void mainFreeLibraries(void)
{
    void glCapResetRGLAddresses(void);

    mainFreeLibrary("rglsw.dll");
    mainFreeLibrary("rgld3d.dll");
    mainFreeLibrary("rgl.dll");
    mainFreeLibrary("opengl32.dll");
    mainFreeLibrary("3dfxvgl.dll");

    RGL = FALSE;
    glCapResetRGLAddresses();
}

/*-----------------------------------------------------------------------------
    Name        : mainRescaleMainWindow
    Description : rescale the main window (ghMainWindow) and call fn to
                  reinit game systems that require it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainRescaleMainWindow(void)
{
    bool wasClipped;

    mainWindowTotalWidth  = MAIN_WindowWidth  + mainWidthAdd;
    mainWindowTotalHeight = MAIN_WindowHeight + mainHeightAdd;

    wasClipped = mouseClipped;
    utyClipMouse(FALSE);

    SetWindowPos(ghMainWindow, HWND_TOP,
                 0, 0,
                 mainWindowTotalWidth, mainWindowTotalHeight,
                 (showBorder) ? SWP_DRAWFRAME : 0);

    if (wasClipped)
    {
        utyClipMouse(TRUE);
    }

    (void)utyChangeResolution(MAIN_WindowWidth, MAIN_WindowHeight, MAIN_WindowDepth);
}

/*-----------------------------------------------------------------------------
    Name        : mainStartupGL
    Description : startup an OpenGL renderer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool mainStartupGL(char* data)
{
    rndinitdata renderData;

    mainRescaleMainWindow();

    if (opUsing3DfxGL)
    {
        if (!mainFind3DfxGL(glToSelect))
        {
            memStrncpy(glToSelect, "opengl32.dll", 512 - 1);
        }
    }
    else
    {
        memStrncpy(glToSelect, "opengl32.dll", 512 - 1);
    }

    if (!glCapLoadOpenGL(glToSelect))
    {
        return FALSE;
    }
    mainDeviceToSelect[0] = '\0';
    mainD3DToSelect[0] = '\0';
    memStrncpy(mainGLToSelect, glToSelect, 512 - 1);

    if (!hwCreateWindow((int)ghMainWindow, MAIN_WindowWidth, MAIN_WindowHeight, MAIN_WindowDepth))
    {
        return FALSE;
    }

    renderData.width = MAIN_WindowWidth;
    renderData.height = MAIN_WindowHeight;
    renderData.hWnd = ghMainWindow;
    if (!rndSmallInit(&renderData, TRUE))
    {
        (void)hwDeleteWindow();
        return FALSE;
    }

    if (!glCapValidGL())
    {
        (void)hwDeleteWindow();
        return FALSE;
    }

    utyForceTopmost(fullScreen);

    glCapStartup();

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : mainMemAlloc
    Description : memory allocation wrapper (for rGL's benefit)
    Inputs      : len - amount of memory to allocate (bytes)
                  name - name of the memory handle
                  flags - flags for allocation, currently ignored
    Outputs     :
    Return      : pointer to new memory, or NULL if none available
----------------------------------------------------------------------------*/
void* mainMemAlloc(GLint len, char* name, GLuint flags)
{
    return memAllocAttempt(len, name, NonVolatile | ExtendedPool);
}

/*-----------------------------------------------------------------------------
    Name        : mainMemFree
    Description : memory free wrapper (for rGL's benefit)
    Inputs      : pointer - the memory pointer to free
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainMemFree(void* pointer)
{
    memFree(pointer);
}

/*-----------------------------------------------------------------------------
    Name        : mainContinueRGL
    Description : set rGL's features
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainContinueRGL(char* data)
{
    rglSetAllocs((MemAllocFunc)mainMemAlloc, (MemFreeFunc)mainMemFree);
    if (fullScreen)
    {
        rglFeature(RGL_FULLSCREEN);
    }
    else
    {
        rglFeature(RGL_WINDOWED);
    }
    if (mainSoftwareDirectDraw)
    {
        rglFeature(RGL_HICOLOR);
    }
    else
    {
        rglFeature(RGL_TRUECOLOR);
    }
    if (slowBlits)
    {
        rglFeature(RGL_SLOWBLT);
    }
    else
    {
        rglFeature(RGL_FASTBLT);
    }
    if (accelFirst)
    {
        rglSelectDevice("fx", "");
        lodScaleFactor = 1.0f;
    }
    if (deviceToSelect[0] != '\0')
    {
        rglSelectDevice(deviceToSelect, data);
        if (strcmp(deviceToSelect, "sw"))
        {
            lodScaleFactor = 1.0f;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mainStartupParticularRGL
    Description : startup rGL as a renderer and activate a particular device
    Inputs      : device - rGL device name
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool mainStartupParticularRGL(char* device, char* data)
{
    rndinitdata renderData;

    mainRescaleMainWindow();

    memStrncpy(glToSelect, "rgl.dll", 512 - 1);
    if (!glCapLoadOpenGL(glToSelect))
    {
        return FALSE;
    }
    memStrncpy(mainDeviceToSelect, device, 16 - 1);
    memStrncpy(mainD3DToSelect, data, 64 - 1);
    memStrncpy(mainGLToSelect, glToSelect, 512 - 1);

    mainContinueRGL(data);

    rglSelectDevice(device, data);

    renderData.width = MAIN_WindowWidth;
    renderData.height = MAIN_WindowHeight;
    renderData.hWnd = ghMainWindow;
    if (!rndSmallInit(&renderData, FALSE))
    {
        return FALSE;
    }

    if (!glCapValidGL())
    {
        return FALSE;
    }

    utyForceTopmost(fullScreen);

    glCapStartup();

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : mainActiveRenderer
    Description : returns the type of currently active renderer
    Inputs      :
    Outputs     :
    Return      : GLtype, D3Dtype, SWtype, [GLIDEtype]
----------------------------------------------------------------------------*/
sdword mainActiveRenderer(void)
{
    return RGLtype;
}

void mainRegisterClass(HANDLE hInstance)
{
    WNDCLASS windowClass;

    // set up and register window class
    windowClass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    windowClass.lpfnWndProc   = WindowProc;
    windowClass.cbClsExtra    = 0;
    windowClass.cbWndExtra    = 0;
    windowClass.hInstance     = hInstance;
    windowClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    windowClass.hCursor       = NULL;
    windowClass.hbrBackground = GetStockObject(BLACK_BRUSH);
    windowClass.lpszMenuName  = NULL;
    windowClass.lpszClassName = classTitle;

    if (!RegisterClass(&windowClass))
    {
//        __asm int 3
    }
}

void mainUnregisterClass(HANDLE hInstance)
{
    if (!UnregisterClass(classTitle, hInstance))
    {
//        __asm int 3
    }
}

void mainDestroyWindow(void)
{
    HWND hWindow;

    mainActuallyQuit = FALSE;
    (void)DestroyWindow(ghMainWindow);

    mainWindowTotalWidth  = MAIN_WindowWidth  + mainWidthAdd;
    mainWindowTotalHeight = MAIN_WindowHeight + mainHeightAdd;

    if (DebugWindow)
    {
        dbwClose();
    }

//    mainUnregisterClass(ghInstance);
//    mainRegisterClass(ghInstance);

    hWindow = CreateWindow(
//        0,//WS_EX_TOPMOST,
        classTitle,                                         // class name string
        windowTitle,                                        // title string
        style,
        0, 0,
        mainWindowTotalWidth,
        mainWindowTotalHeight,
        NULL,
        NULL,
        ghInstance,
        NULL );

    if (hWindow == NULL)
    {
        return;
    }

    ShowWindow(hWindow, 0/*nCmdShow*/);
    utyForceTopmost(fullScreen);
    UpdateWindow(hWindow);

    ghMainWindow = hWindow;

    if (DebugWindow)
    {
        dbwStart((udword)ghInstance, (udword)ghMainWindow);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mainShutdownGL
    Description : shutdown a GL renderer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainShutdownGL(void)
{
    rndClose();
    if (sstLoaded())
    {
        sstShutdown();
    }
    hwDeleteWindow();
    mainDestroyWindow();
}

/*-----------------------------------------------------------------------------
    Name        : mainShutdownRGL
    Description : shutdown rGL as a renderer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainShutdownRGL(void)
{
    rndClose();
    mainDestroyWindow();
}

/*-----------------------------------------------------------------------------
    Name        : mainResetRender
    Description : inform necessary modules that the renderer is switching
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainResetRender(void)
{
    glDLLReset();
    rndResetGLState();
    glcRenderEverythingALot();
    feRenderEverything = TRUE;
    frReset();
    ferReset();
    mouseReset();
    feReset();
    if (!reinitInProgress)
    {
        trReload();
    }
}

/*-----------------------------------------------------------------------------
    Name        : mainCloseRender
    Description : free module textures, call reset render
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainCloseRender(void)
{
    partShutdown();
    glcFreeTextures();
    ferReset();
    cpTexturesPurge();
    lmFreeTextures();
    cpPreviewImageDelete();
    rndLoadShamelessPlug(FALSE);    //shameless plug handles reloading itself
    btgCloseTextures();
    cmCloseTextures();
    rmGUIShutdown();
    mainResetRender();

    if (!reinitInProgress)
    {
        trSetAllPending(FALSE);
        trNoPalShutdown();
#if TR_ERROR_CHECKING
        if (RGL)
        {
            rglFeature(RGL_TEXTURE_LOG);
        }
#endif
    }
}

/*-----------------------------------------------------------------------------
    Name        : mainOpenRender
    Description : call reset render
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainOpenRender(void)
{
    trNoPalStartup();
    mainResetRender();
    glcLoadTextures();
    rmGUIStartup();
    cmLoadTextures();
    btgLoadTextures();
    lmLoadTextures();
    //shameless plug handles reloading itself
    frReloadGL();
}

udword saveRGLtype;
sdword saveMAIN_WindowWidth;
sdword saveMAIN_WindowHeight;
sdword saveMAIN_WindowDepth;
char saveglToSelect[512];
char savedeviceToSelect[16];
char saved3dToSelect[64];

/*-----------------------------------------------------------------------------
    Name        : mainSaveRender
    Description : save current renderer info in case a mode switch doesn't work
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainSaveRender(void)
{
    saveRGLtype = RGLtype;
    saveMAIN_WindowWidth  = MAIN_WindowWidth;
    saveMAIN_WindowHeight = MAIN_WindowHeight;
    saveMAIN_WindowDepth  = MAIN_WindowDepth;
    memStrncpy(saveglToSelect, mainGLToSelect, 512 - 1);
    memStrncpy(savedeviceToSelect, mainDeviceToSelect, 16 - 1);
    memStrncpy(saved3dToSelect, mainD3DToSelect, 64 - 1);
}

void mainSetupSoftware(void)
{
    strcpy(glToSelect, "rgl.dll");
    strcpy(mainGLToSelect, "rgl.dll");
    strcpy(deviceToSelect, "sw");
    strcpy(mainDeviceToSelect, "sw");
    strcpy(mainD3DToSelect, "");

    mainWindowWidth  = MAIN_WindowWidth  = 640;
    mainWindowHeight = MAIN_WindowHeight = 480;
    mainWindowDepth  = MAIN_WindowDepth  = 16;

    opDeviceIndex = -1;

    gDevcaps = gDevcaps2 = 0;
}

/*-----------------------------------------------------------------------------
    Name        : mainRestoreSoftware
    Description : load 640x480@16, rGL+software rendering system
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainRestoreSoftware(void)
{
    mainSetupSoftware();

    mainRescaleMainWindow();

    bMustFree = FALSE;
    if (!mainLoadParticularRGL("sw", ""))
    {
/*
        MessageBox(NULL,
                   "couldn't initialize default rendering system",
                   windowTitle, MB_APPLMODAL | MB_OK);
*/
        SetWindowPos(ghMainWindow, NULL, 0, 0, 0, 0,
                     SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_HIDEWINDOW);
        MessageBox(ghMainWindow,
                   "couldn't initialize default rendering system",
                   "Fatal Error", MB_ICONSTOP | MB_OK);
        PostMessage(ghMainWindow, WM_CLOSE, CID_ExitError, 0);
    }
    bMustFree = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : mainRestoreRender
    Description : restore saved rendering system (mainSaveRender())
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainRestoreRender(void)
{
    memStrncpy(glToSelect, saveglToSelect, 512 - 1);
    memStrncpy(mainGLToSelect, saveglToSelect, 512 - 1);
    memStrncpy(deviceToSelect, savedeviceToSelect, 16 - 1);
    memStrncpy(mainDeviceToSelect, savedeviceToSelect, 16 - 1);
    memStrncpy(mainD3DToSelect, saved3dToSelect, 64 - 1);

    mainWindowWidth  = MAIN_WindowWidth  = saveMAIN_WindowWidth;
    mainWindowHeight = MAIN_WindowHeight = saveMAIN_WindowHeight;
    mainWindowDepth  = MAIN_WindowDepth  = saveMAIN_WindowDepth;

    mainRescaleMainWindow();

    RGLtype = saveRGLtype;
    bMustFree = FALSE;
    if (RGLtype == GLtype)
    {
        if (!mainLoadGL(NULL))
        {
            //couldn't restore, try basic software
            mainRestoreSoftware();
        }
    }
    else
    {
        if (!mainLoadParticularRGL(savedeviceToSelect, saved3dToSelect))
        {
            //couldn't restore, try basic software
            mainRestoreSoftware();
        }
    }
    bMustFree = TRUE;

    feRenderEverything = TRUE;
    glcRenderEverythingALot();
}

/*-----------------------------------------------------------------------------
    Name        : mainShutdownRenderer
    Description : shutdown the existing renderer
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool mainShutdownRenderer(void)
{
    dbgMessage("\nmainShutdownRenderer");

    mainCloseRender();
    if (RGLtype == GLtype)
    {
        mainShutdownGL();
    }
    else
    {
        mainShutdownRGL();
    }
    mainFreeLibraries();

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : mainLoadGL
    Description : close existing renderer, startup a GL
    Inputs      : data - possible name of GL .DLL
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool mainLoadGL(char* data)
{
    dbgMessage("\n-- load OpenGL --");

    if (bMustFree)
    {
        mainCloseRender();

        if (RGLtype == GLtype)
        {
            mainShutdownGL();
        }
        else
        {
            mainShutdownRGL();
        }
        mainFreeLibraries();
    }

    if (!mainStartupGL(data))
    {
        return FALSE;
    }

    mainOpenRender();

    lodScaleFactor = 1.0f;
    alodStartup();

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : mainLoadParticularRGL
    Description : close existing render, startup rGL w/ specified device
    Inputs      : device - device name {sw, d3d, fx}
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool mainLoadParticularRGL(char* device, char* data)
{
    dbgMessagef("\n-- load rGL device %s --", device);

    if (bMustFree)
    {
        mainCloseRender();

        if (RGLtype == GLtype)
        {
            mainShutdownGL();
        }
        else
        {
            mainShutdownRGL();
        }
        mainFreeLibraries();
    }

    if (!mainStartupParticularRGL(device, data))
    {
        return FALSE;
    }

    mainOpenRender();

    lodScaleFactor = (RGLtype == SWtype) ? LOD_ScaleFactor : 1.0f;
    alodStartup();

    mainReinitRenderer = 2;

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : mainReinitRGL
    Description : reinitializes rGL, currently used to work around a disheartening
                  D3D problem wrt alpha textures
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool mainReinitRGL(void)
{
    if (RGLtype != D3Dtype)
    {
        return TRUE;
    }

    reinitInProgress = TRUE;

//    mainCloseRender();
    rglFeature(RGL_REINIT_RENDERER);
//    glCapStartup();
//    mainOpenRender();
//    glCapStartup();
//    lodScaleFactor = (RGLtype == SWtype) ? LOD_ScaleFactor : 1.0f;
//    alodStartup();
    dbgMessage("\n-- reinit rGL --");

    reinitInProgress = FALSE;

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : keyLanguageTranslate
    Description : This function translates the virtual key into the ASCII character.
    Inputs      : virtkey and scancode
    Outputs     : converted character
    Return      : Uh-huh
----------------------------------------------------------------------------*/
udword keyLanguageTranslate(udword wParam)
{
    switch (strCurKeyboardLanguage)
    {
        case languageEnglish:
            return(wParam);
        break;
        case languageFrench:
            if (wParam < 256)
                return(keyFrenchToEnglish(wParam));
            else
                return(0);
        break;
        case languageGerman:
            if (wParam < 256)
                return(keyGermanToEnglish(wParam));
            else
                return(0);
        break;
        case languageSpanish:
            if (wParam < 256)
                return(keySpanishToEnglish(wParam));
            else
                return(0);
        break;
        case languageItalian:
            if (wParam < 256)
                return(keyItalianToEnglish(wParam));
            else
                return(0);
        break;
    }
/*    char   buff[256], trankey[2];
    sdword i,ret;

    for (i=0;i<256;i++) buff[i] = 0;

    ret = ToAscii(wParam, 0, buff, (uword *)trankey, 0);

    if (ret==0)
    {
        return((udword)wParam);
    }
    else if (ret==1)
    {
        if ((trankey[0] >= 'a') && (trankey[0] <= 'z'))
        {
            trankey[0]-='a'-'A';
        }
        return ((udword)((ubyte)trankey[0]));
    }

    return (0);*/
}

/*-----------------------------------------------------------------------------
    Name        : WindowProc
    Description : Message handling function for main window
    Inputs      : See Windows help.
    Outputs     : Ditto
    Return      : Uh-huh
----------------------------------------------------------------------------*/
long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    extern bool utilPlayingIntro;

#if MAIN_PRINT_MESSAGES
    dbgMessagef("\nMessage = 0x%x, wParam = 0x%x, lParam = 0x%x", message, wParam, lParam);
#endif //MAIN_PRINT_MESSAGES

    if (message == uHWCloseMsg)
    {
        goto destroy;
    }

    switch( message )
    {
        case WM_SYSCOMMAND:
            if (wParam == SC_SCREENSAVE)
            {
                //render screensavers ineffective
                return 0;
            }
            break;

        case WM_ACTIVATEAPP:
#if 0
            if (!mainActuallyQuit)
            {
                return 0;
            }
#endif
            if((bool)wParam && (systemActive == FALSE))
            {                                               //we're being activated
                if (utilPlayingIntro)
                {
                    IntroActivateMe(hWnd);
                }
                else
                {
                    ActivateMe(hWnd);
                }
            }
            else if (systemActive == TRUE)
            {                                               //we're being deactivated
                if (utilPlayingIntro)
                {
                    IntroDeactivateMe(hWnd);
                }
                else
                {
                    DeactivateMe(hWnd);
                }
            }
            else
                break;

            return 0;                                       //per documentation

        case WM_KEYUP:                                      //keys up/down
            switch (wParam)
            {
                case VK_ESCAPE:
                    if (!binkDonePlaying)
                    {
                        binkStop();
                    }
                    else
                    {
                        keyPressUp(KeyMapFromWindows(wParam));
                    }
#if MAIN_MOUSE_FREE
                case VK_F11:                                //toggle clipped cursor
                    if (keyIsHit(CONTROLKEY))
                    {
                        utyClipMouse(mouseClipped ^ TRUE);
                    }
                    else
                    {
                        keyPressUp(KeyMapFromWindows(wParam));
                    }
                    break;
#endif
                case VK_F12:
                    if (!RGL)
                    {
                        if (keyIsHit(SHIFTKEY) && keyIsHit(CONTROLKEY))
                        {
                            glcFreeTextures();
                            mainCloseRender();
                            mainShutdownGL();
                            mainRestoreSoftware();
                            mainOpenRender();
                            glCapStartup();
                            glcLoadTextures();
                            lodScaleFactor = LOD_ScaleFactor;
                            alodStartup();
                        }
                    }
                    else
                    {
                        dbgMessagef("\nprevious GL RENDERER: %s", glGetString(GL_RENDERER));
                        if (keyIsHit(SHIFTKEY) && keyIsHit(CONTROLKEY))
                        {
                            mainCloseRender();
                            mainShutdownRGL();
                            mainRestoreSoftware();
                        }
                        else
                        {
                            break;
                        }
                        glCapStartup();
                        mainOpenRender();
                        glCapStartup();
                        if (RGLtype == SWtype)
                        {
                            lodScaleFactor = LOD_ScaleFactor;
                        }
                        else
                        {
                            lodScaleFactor = 1.0f;
                        }
                        alodStartup();
                        dbgMessagef("\nnew GL RENDERER: %s", glGetString(GL_RENDERER));
                    }
                    break;
                default:
                    keyPressUp(keyLanguageTranslate(wParam));//keyPressUp(KeyMapFromWindows(wParam));
            }
            return 0;

        case WM_KEYDOWN:
            if (lParam & BIT30)
            {                                               //if it's a repeating key
                //keyRepeat(KeyMapFromWindows(wParam));
                keyRepeat(keyLanguageTranslate(wParam));
            }
            else
            {                                               //else it's a freshly-pressed key
                keyPressDown(keyLanguageTranslate(wParam));
                //keyPressDown(KeyMapFromWindows(wParam));
            }
            return 0;

        case WM_DEADCHAR:
            return 0;

        case WM_SYSKEYUP:                                   //keys up/down
            keyPressUp(keyLanguageTranslate(wParam));
            return 0;

        case WM_SYSKEYDOWN:
            if (lParam & BIT30)
            {                                               //if it's a repeating key
                keyRepeat(keyLanguageTranslate(wParam));
            }
            else
            {                                               //else it's a freshly-pressed key
                keyPressDown(keyLanguageTranslate(wParam));
            }
            return 0;

        case WM_COMMAND:
            CommandProcess(hWnd, wParam, lParam);
            return 0;

        case WM_LBUTTONDBLCLK:
            if (!mouseDisabled)
            {
                keyPressDown(KeyMapFromWindows(LMOUSE_DOUBLE));
            }
            break;

        case WM_LBUTTONDOWN:                                //mouse buttons up/down
            if (!mouseDisabled)
            {
                mouseLClick();
                keyPressDown(KeyMapFromWindows(VK_LBUTTON));
            }
            break;

        case WM_LBUTTONUP:
            if (!mouseDisabled)
            {
                keyPressUp(KeyMapFromWindows(VK_LBUTTON));
                keyPressUp(KeyMapFromWindows(LMOUSE_DOUBLE));
            }
            break;

        case WM_RBUTTONDBLCLK:
            if (!mouseDisabled)
            {
                keyPressDown(KeyMapFromWindows(RMOUSE_DOUBLE));
            }
            break;

        case WM_RBUTTONDOWN:
            if (!mouseDisabled)
            {
                keyPressDown(KeyMapFromWindows(VK_RBUTTON));
            }
            break;

        case WM_RBUTTONUP:
            if (!mouseDisabled)
            {
                keyPressUp(KeyMapFromWindows(VK_RBUTTON));
                keyPressUp(KeyMapFromWindows(RMOUSE_DOUBLE));
            }
            break;

        case WM_MBUTTONDOWN:
            if (!mouseDisabled)
            {
                keyPressDown(KeyMapFromWindows(VK_MBUTTON));
            }
            break;

        case WM_MBUTTONUP:
            if (!mouseDisabled)
            {
                keyPressUp(KeyMapFromWindows(VK_MBUTTON));
                keyPressUp(KeyMapFromWindows(MMOUSE_DOUBLE));
            }
            break;

        case WM_MBUTTONDBLCLK:
            if (!mouseDisabled)
            {
                keyPressDown(KeyMapFromWindows(MMOUSE_DOUBLE));
            }
            break;

        case WM_DESTROY:
destroy:
            if (mainActuallyQuit)
            {
                WindowsCleanup();
                PostQuitMessage(0);
            }
            else
            {
                mainActuallyQuit = TRUE;
            }
            return 0;

        case WM_CLOSE:
            if ((wParam != CID_ExitError) && (wParam != CID_ExitOK))
            {                                               //if it's not one of the 'official' exit codes generated by the game
                return 0;                                   //don't let the game shut down by normal windows methods
            }
            break;                                          //else close down as normal

        case WM_ENTERIDLE:                                  //game deactivated due to a pop-up (menu or dialog)
            keyClearAll();                                  //don't leave any keys stuck in this case
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_MOVE:
            WindowTopLeft.x = (int)LOWORD(lParam);
            WindowTopLeft.y = (int)HIWORD(lParam);
            return 0;
        case WM_GETMINMAXINFO:
        {
            RECT windowRect;
            MINMAXINFO *info;

            if (WindowTopLeft.x == -1)                      //if window hasn't moved yet
            {
                break;                                      //don't force any size restrictions
            }
            info = (MINMAXINFO *)lParam;
            GetWindowRect(ghMainWindow, &windowRect);       //get window rect

            info->ptMaxSize.x = mainWindowTotalWidth;
            info->ptMaxSize.y = mainWindowTotalHeight;
                GetSystemMetrics(SM_CYSIZEFRAME) * 2;       //GetSystemMetrics( SM_CYSCREEN )
            info->ptMaxPosition.x = windowRect.left;        //maximise will not move it
            info->ptMaxPosition.y = windowRect.top;
            info->ptMinTrackSize = info->ptMaxSize;         //can't change size
            info->ptMaxTrackSize = info->ptMaxSize;
            return 0;
        }

        case WM_MOUSEWHEEL:
            if (!mouseDisabled)
            {
                if ((short)HIWORD(wParam) < 0)
                    keyPressDown(FLYWHEEL_DOWN);
                else
                    keyPressDown(FLYWHEEL_UP);
            }
            break;
        case WM_INPUTLANGCHANGE:
            // keyboard locale has changed update settings ...
            strSetCurKeyboard();
            return 0;
        default:
            if (uMSH_MOUSEWHEEL != 0 && message == uMSH_MOUSEWHEEL)
            {
                if ((int)wParam < 0)
                    keyPressDown(FLYWHEEL_DOWN);
                else
                    keyPressDown(FLYWHEEL_UP);
                break;
            }
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
} // WindowProc

/*-----------------------------------------------------------------------------
    Name        : InitWindow
    Description : Creates and initializes the main game window, whether full-screen or not.
    Inputs      :
        hInstance       - instance handle for game.
        nCmdShow        - run state (minimized etc.)
    Outputs     :
    Return      : window handle
----------------------------------------------------------------------------*/
static HWND InitWindow( HANDLE hInstance, int nCmdShow )
{
    HWND hWindow;
    char d3dToSelect[64];

    mainRegisterClass(hInstance);

    /*
     * create a window
     */

    if (fullScreen)
    {
        showBorder = FALSE;
    }

    if (mainAutoRenderer)
    {
        if (selectedRES)
        {
            MAIN_WindowWidth  = mainWindowWidth;
            MAIN_WindowHeight = mainWindowHeight;
            MAIN_WindowDepth  = mainWindowDepth;
        }
        else
        {
            mainWindowWidth  = MAIN_WindowWidth;
            mainWindowHeight = MAIN_WindowHeight;
            mainWindowDepth  = MAIN_WindowDepth;
        }
    }

    if (showBorder)
    {
        style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_EX_TOPMOST;//WS_POPUP,
        mainWidthAdd  = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
        mainHeightAdd = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME) * 2;
    }
    else
    {
        style = WS_POPUP;
        mainWidthAdd  = 0;
        mainHeightAdd = 0;
    }

    if (mainForceSoftware)
    {
        MAIN_WindowWidth  = 640;
        MAIN_WindowHeight = 480;
        MAIN_WindowDepth  = 16;
    }

    mainWindowTotalWidth  = MAIN_WindowWidth  + mainWidthAdd;
    mainWindowTotalHeight = MAIN_WindowHeight + mainHeightAdd;

    hWindow = CreateWindow(
//        0,//WS_EX_TOPMOST,
        classTitle,                                         // class name string
        windowTitle,                                        // title string
        style,
        0, 0,
        mainWindowTotalWidth,
        mainWindowTotalHeight,
        NULL,
        NULL,
        hInstance,
        NULL );

    if (hWindow == NULL)
    {
        return NULL;
    }

    mainDevStatsInit();
    rinEnumerateDevices();

    d3dToSelect[0] = '\0';

    if (mainAutoRenderer &&
        (strlen(mainGLToSelect) > 0))
    {
        if (selectedDEVICE)
        {
            d3dToSelect[0] = '\0';
        }
        else
        {
            memStrncpy(deviceToSelect, mainDeviceToSelect, 16 - 1);
            memStrncpy(d3dToSelect, mainD3DToSelect, 64 - 1);
        }
        if (!selectedGL)
        {
            memStrncpy(glToSelect, mainGLToSelect, 512 - 1);
        }
    }

    if (opDeviceCRC != rinDeviceCRC())
    {
        opDeviceIndex = -1;
        if (mainAutoRenderer)
        {
            opDeviceCRC = rinDeviceCRC();
            mainForceSoftware = TRUE;
        }
    }

    if (mainForceSoftware)
    {
        strcpy(deviceToSelect, "sw");
        strcpy(mainDeviceToSelect, "sw");
        strcpy(glToSelect, "rgl.dll");
        strcpy(mainGLToSelect, "rgl.dll");
        d3dToSelect[0] = '\0';
        mainD3DToSelect[0] = '\0';
        MAIN_WindowWidth  = 640;
        MAIN_WindowHeight = 480;
        MAIN_WindowDepth  = 16;

        ShowWindow(hWindow, nCmdShow);
        utyForceTopmost(fullScreen);
        UpdateWindow(hWindow);
        ghMainWindow = hWindow;

        mainRescaleMainWindow();

        opDeviceIndex = -1;

        gDevcaps = gDevcaps2 = 0;
    }
    else
    {
        extern udword loadedDevcaps, loadedDevcaps2;
        //successfully re-using previous device,
        //re-use previous devcaps
        gDevcaps = loadedDevcaps;
        if (gDevcaps == 0xFFFFFFFF)
        {
            gDevcaps = 0;
        }
        gDevcaps2 = loadedDevcaps2;
        if (gDevcaps2 == 0xFFFFFFFF)
        {
            gDevcaps2 = 0;
        }
    }

    if (!glCapLoadOpenGL(glToSelect))
    {
        mainSetupSoftware();

        ShowWindow(hWindow, nCmdShow);
        utyForceTopmost(fullScreen);
        UpdateWindow(hWindow);
        ghMainWindow = hWindow;

        mainRescaleMainWindow();
        if (!glCapLoadOpenGL(glToSelect))
        {
            SetWindowPos(ghMainWindow, NULL, 0, 0, 0, 0,
                         SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_HIDEWINDOW);
            MessageBox(ghMainWindow,
                       "couldn't initialize default rendering system",
                       "Fatal Error", MB_ICONSTOP | MB_OK);
            return NULL;
        }
    }
    memStrncpy(mainDeviceToSelect, deviceToSelect, 16 - 1);
    memStrncpy(mainGLToSelect, glToSelect, 512 - 1);

    glCapStartup();

    if (_stricmp(deviceToSelect, "d3d") == 0)
    {
        mainReinitRenderer = 2;
    }

    if (RGL)
    {
        rglSetAllocs((MemAllocFunc)mainMemAlloc, (MemFreeFunc)mainMemFree);
        if (fullScreen)
        {
            rglFeature(RGL_FULLSCREEN);
        }
        else
        {
            rglFeature(RGL_WINDOWED);
        }
        if (mainSoftwareDirectDraw)
        {
            rglFeature(RGL_HICOLOR);
        }
        else
        {
            rglFeature(RGL_TRUECOLOR);
        }
        if (slowBlits)
        {
            rglFeature(RGL_SLOWBLT);
        }
        else
        {
            rglFeature(RGL_FASTBLT);
        }
        if (accelFirst)
        {
            rglSelectDevice("fx", "");
            lodScaleFactor = 1.0f;                              //default scale factor is just right for 3dfx
        }
        if (deviceToSelect[0] != '\0')
        {
            rglSelectDevice(deviceToSelect, d3dToSelect);
            if (strcmp(deviceToSelect, "sw"))
            {
                lodScaleFactor = 1.0f;
            }
        }
    }
    else
    {
        lodScaleFactor = 1.0f;
    }

    ShowWindow(hWindow, nCmdShow);
    utyForceTopmost(fullScreen);
    UpdateWindow(hWindow);

    ghMainWindow = hWindow;

    return hWindow;
} /* doInit */

/*-----------------------------------------------------------------------------
    Name        : WindowsCleanup
    Description : Closes all windows, frees memory etc.
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void WindowsCleanup(void)
{
    utyGameSystemsShutdown();
    if (!RGL)
    {
        hwDeleteWindow();
    }
    rinFreeDevices();
}

/*-----------------------------------------------------------------------------
    Name        : mainCleanupAfterVideo
    Description : possibly delete main DDraw window after playing an AVI if a renderer
                  is about to be initialized that wants to create its own
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mainCleanupAfterVideo(void)
{
    if (windowNeedsDeleting)
    {
        windowNeedsDeleting = FALSE;
        //restore previous display mode
        hwSetRes(0, 0, 0);
    }
    if (!RGL)
    {
        //create a window at appropriate res
        (void)hwCreateWindow((int)ghMainWindow, MAIN_WindowWidth, MAIN_WindowHeight, MAIN_WindowDepth);
    }
}

void RunPatcher(void)
{
    DeactivateMe(ghMainWindow);

    WinExec(PATCHNAME,SW_NORMAL);

//    WindowsCleanup();
    //PostQuitMessage(0);
}


/*-----------------------------------------------------------------------------
    Name        : WinMain
    Description : Entry point to the game
    Inputs      :
        hInstance       - instance handle for application
        hPrevInstance   - warning! does nothing under Win32!
        commandLine     - un-tokenized command line string
        nCmdShow        - run state (minimized etc.)
    Outputs     :
    Return      : Exit code (Windows provides an intelligible number)
----------------------------------------------------------------------------*/
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                        LPSTR commandLine, int nCmdShow)
{
    static MSG   message;
    static char *errorString = NULL;
    static HWND  hWnd;
    static HANDLE hMapping;
    static BOOL  preInit;

    //check to see if a copy of the program already running and just exit if so.
    //This messy hack is required because hPrevInstance is not reliable under Win32.
    hMapping = CreateFileMapping( (HANDLE) 0xffffffff,
                    NULL,
                    PAGE_READONLY,
                    0,
                    32,
                    "LSMap" );

    if( hMapping )
    {
        if( GetLastError() == ERROR_ALREADY_EXISTS )
        {
            HWND hWndPrev;

            // Already an instance running. If it has a window yet, restore it.
            hWndPrev = FindWindow(windowTitle, NULL);
            if (hWndPrev)
            {
                OutputDebugString("\r\nProgram already running.\r\n");
                ShowWindow(hWndPrev, SW_RESTORE);
                BringWindowToTop(hWndPrev);
                UpdateWindow(hWndPrev);
            }
            return 0;
        }
    }
/*
    if (strlen(leakString) != MAX_LEAK_STRING_LENGTH)
    {   // leakstring has not been plugged by the Fat-Assed plumber program
        sdword index;
        time_t tt;

        index = time(&tt) % 5 + 3;
        while (index--)
        {
            MessageBox(NULL, "The Executable is possibly corrupt.  Please re-install Homeworld.  Execution may FAIL!", "Faucet Error", MB_ABORTRETRYIGNORE);
        }
    }
*/
    uHWCloseMsg = RegisterWindowMessage("CloseHomeworld");

    ghInstance = hInstance;

    mainDeviceToSelect[0] = '\0';

    //load in options from the options file
    utyOptionsFileRead();

    //copy keyboard redefinitions
    opKeyboardLoad();

    RegisterCommandLine(commandLine);       // make sure you call this before ProcessCommandLine because
                                            // ProcessCommandLine modifies commandLine

    //process the command line, setting flags to be used later
    if (ProcessCommandLine(commandLine) != OKAY)
    {
        return 0;
    }

    glNT = glCapNT();
    gl95 = glCap95();

    if (selectedRES)
    {
        MAIN_WindowWidth  = mainWindowWidth;
        MAIN_WindowHeight = mainWindowHeight;
        MAIN_WindowDepth  = mainWindowDepth;
    }

#if MAIN_Password
    if ((errorString = mainPasswordVerify(mainPasswordPtr)) != NULL)
    {
        MessageBox(NULL, errorString, "Homeworld Run-Time Error", MB_OK);
        return 0;
    }
#endif

    //initial game systems startup
    preInit = FALSE;
    if (errorString == NULL)
    {
        errorString = utyGameSystemsPreInit();
    }

    //startup the game window
    if (errorString == NULL)
    {
        if (DebugWindow)
        {
            dbwClose();
        }
        preInit = TRUE;
        if ((hWnd = InitWindow(hInstance, nCmdShow)) == NULL)
        {
            errorString = ersWindowInit;
        }
        if (DebugWindow)
        {
            dbwStart((udword)ghInstance, (udword)ghMainWindow);
            utySet(SSA_DebugWindow);
        }
    }

    mainPlayAVIs = FALSE;
    if (errorString == NULL)
    {
#ifndef Downloadable
        if (enableAVI && fullScreen)
        {
            windowNeedsDeleting = TRUE;
            //set display mode
            if (!hwSetRes(640, 480, 32))
            {
                hwSetRes(640, 480, 16);
            }
            utyForceTopmost(TRUE);
        }
        else if (!RGL)
        {
            windowNeedsDeleting = FALSE;
            (void)hwCreateWindow((int)ghMainWindow, MAIN_WindowWidth, MAIN_WindowHeight, MAIN_WindowDepth);
            utyForceTopmost(fullScreen);
        }
#else
        windowNeedsDeleting = FALSE;
        (void)hwCreateWindow((int)ghMainWindow, MAIN_WindowWidth, MAIN_WindowHeight, MAIN_WindowDepth);
        utyForceTopmost(fullScreen);
#endif
        //startup game systems
        if (errorString == NULL)
        {
            errorString = utyGameSystemsInit();
        }
    }

    if (errorString == NULL)
    {
        preInit = FALSE;

        uMSH_MOUSEWHEEL = RegisterWindowMessage(MSH_MOUSEWHEEL);

        while(TRUE)
        {
            // Give sound a break :)
            Sleep(0);

            if(PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE))
            {
                if(!GetMessage(&message, NULL, 0, 0))
                    break;
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                utyTasksDispatch();                         //execute all tasks
            }

            if (opTimerActive)
            {
                if (taskTimeElapsed > (opTimerStart + opTimerLength))
                {
                    opTimerExpired();
                }
            }

            if (patchComplete)
            {
                RunPatcher();
                patchComplete = 0;
            }
        }
    }
    else
    {                                                       //some error on startup, either from preInit or Init()
        if (preInit)
        {
            (void)utyGameSystemsPreShutdown();
        }
        MessageBox(NULL, errorString, windowTitle, MB_APPLMODAL|MB_OK);
        return ERR_ErrorStart;
    }
    return message.wParam;
} /* WinMain */
