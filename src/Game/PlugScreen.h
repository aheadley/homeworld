/*=============================================================================
    Name    : PlugScreen.h
    Purpose : Data definitions for screens with sales plugs and web links.

    Created 3/31/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PLUGSCREEN_H
#define ___PLUGSCREEN_H

#include "texreg.h"
#include "Region.h"

/*=============================================================================
    Switches:
=============================================================================*/

#ifndef HW_Release

#define PS_ERROR_CHECKING      1               //basic error checking
#define PS_VERBOSE_LEVEL       1               //control verbose printing

#else //HW_Debug

#define PS_ERROR_CHECKING      0               //basic error checking
#define PS_VERBOSE_LEVEL       0               //control verbose printing

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//plug link types
#define PLT_Screen              0               //link to another screen
#define PLT_URL                 1               //link to a web address
#define PLT_Exit                2               //exit if clicked here
#define PLT_GameOn              3               //start the actual game
#define PLM_Type                0x7             //mask of the type bits

#define PLF_FadeRegion          0x80            //this region is responsible for fading in/out

#define PS_FadeTime             0.5f            //fade in/out rate

//definitions for the texture 'quilt'
#define PS_QuiltPieceWidth      64
#define PS_QuiltPieceHeight     64

//fade states
#define PFS_None                0
#define PFS_FromBlack           1
#define PFS_ToBlack             2
#define PFS_CrossFade           3

//plugscreen mode flags (in psGlobalFlags)
#define PMF_Credits             1               //special case for credits screens
#define PMF_CanSkip             2               //can skip through individual screens
#define PMF_MusicTrack          4               //this sequence has it's own music track
#define PMF_LanguageSpecific    8               //contains localized content with subdirectories

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
    region reg;                                 //region this link will use
    lifheader *onImage;                         //keep the lif headers around for various reasons
    lifheader *offImage;
    udword onTexture;                           //on/off textures
    udword offTexture;
    char linkName[1];                           //name of link screen or web address
}
pluglink;

typedef struct
{
    sdword width, height;
    udword *imageQuilt;
}
psimage;

/*=============================================================================
    Macros:
=============================================================================*/
#define plugLinkExtra(string)   (sizeof(pluglink) - 1 - sizeof(region) + strlen(string))
#define plugXMargin             ((MAIN_WindowWidth - 640) / 2)
#define plugYMargin             ((MAIN_WindowHeight - 480) / 2)

/*=============================================================================
    Functions:
=============================================================================*/

void psStartup(void);
void psShutdown(void);
void psModeBegin(char *directory, udword modeFlags);
void psModeEnd(void);
void psScreenStart(char *name);
void psCurrentScreenDelete(void);

#endif //___PLUGSCREEN_H

