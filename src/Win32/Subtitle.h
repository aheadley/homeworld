/*=============================================================================
    Name    : Subtitle.h
    Purpose : Definitions for subtitling

    Created 2/3/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SUBTITLE_H
#define ___SUBTITLE_H   "You are not hearing this!"

#include "prim2d.h"
#include "color.h"
#include "font.h"
#include "texreg.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define SUB_MODULE_TEST             0           //test this module

#ifndef HW_Release

#define SUB_ERROR_CHECKING          1           //general error checking
#define SUB_VERBOSE_LEVEL           3           //control specific output code
#define SUB_VISUALIZE_REGION        0           //draw the rectangle of the region

#else //HW_Debug

#define SUB_ERROR_CHECKING          0           //general error checking
#define SUB_VERBOSE_LEVEL           0           //control specific output code
#define SUB_VISUALIZE_REGION        0           //draw the rectangle of the region

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//definitions for subtitle regions
#define STR_NIS                     0           //middle of screen, where NIS's normally play their text
#define STR_LetterboxBar            1           //at the top of the screen, on top of the letterbox bar
#define STR_CloseCaptioned          2           //down near bottom for people hard-of-hearing
#define STR_LocationCard            3           //location cards in the middle of the screen

//definitions of subtitle themes
#define STT_CloseCaptioned          14
#define STT_LocationCard            15

//definitions of subtitle actors
#define STA_FleetCommand            0
#define STA_AllShips0               1
#define STA_AllShips2               2
#define STA_AllShips3               3
#define STA_FleetIntel              4
#define STA_Traders                 5
#define STA_Pirates2                6
#define STA_AllEnemyShips           7
#define STA_Ambassador              8
#define STA_Defector                9
#define STA_Narrator                10
#define STA_KharSelim               11
#define STA_Emperor                 12
#define STA_LocationCard            112

#define SUB_MaxLinesPerSubtitle     64          //maximum number of lines a subtitle can be chopped up into
#define SUB_SubtitleLength          2040
#define SUB_NumberNewSubtitles      8           //max number of new subtitles that can be added in a single frame
#define SUB_SemaphoreName           "SUBSEMAPHORE"

#define SUB_NumberThemes            16
#define SUB_NumberRegions           4

#define SUB_ScriptFile              "Subtitle.script"

#define SUB_ScrollDwellStart        0.7f
#define SUB_ScrollDwellEnd          0.5f
#define SUB_ScrollShortest          0.25f
#define SUB_TitleShortest           1.2f

#define SUB_InterlineSpacing        1           //spacing between lines

//definitions for pictures
#define SUB_PictureWidth            64
#define SUB_PictureHeight           64
#define SUB_PictureMarginY          0
#define SUB_PictureMarginX          3

/*=============================================================================
    Type definitions:
=============================================================================*/

//structure for a single "line" or "card" within a subtitle region.  It may be more than 1 line when all's all is said and done.
typedef struct
{
    real32 creationTime;                        //when this text card was created
    real32 duration;                            //how long this text card is supposed to last, including fades
    real32 fadeIn;                              //duration of fade in from black, if any
    real32 fadeOut;                             //duration of fade out to black, if any
    color c;                                    //color of text card
    bool bDropShadow;
    fonthandle font;                            //font to draw in
    sdword x, y;                                //where to draw the text
//    sword margin;                               //margin for use with multi-line text
    char *text;                                 //what text to draw
}
subcard;

//structure for a region where subtitles are to go
typedef struct
{
    //real-time stuff for the region
    real32 textScrollStart;                     //when scrolling starts
    real32 textScrollEnd;                       //when scrolling will end
    real32 scrollDistance;                      //how far to scroll
    trhandle picture;                           //what picture to draw, if any, at the side of the text
    bool8  bAborted;                            //if true, this is fading out because it was aborted
    bool8  bContinuousEvent;                    //this event will keep playing after this speech fragment

    //settings for the region
    rectangle defaultRect;                      //rectangle in 640x480 rez
    rectangle rect;                             //where the text is printed
    bool8 bScaleRezX, bScaleRezY;               //TRUE if the rectangle is to scale with the screen rez
    bool8 bEnabled;                             //a subtitle region can be disabled
    sdword iconOffsetX, iconOffsetY;            //offsets to the speech icon

    //actual text cards
    sdword numberCards;                         //max number of sentences in this region
    sdword cardIndex;                           //number of sentences currenly in use
    char chopBuffer[SUB_SubtitleLength + SUB_MaxLinesPerSubtitle];
    subcard *card;                              //actual lines of text
}
subregion;

//structure for a text theme
typedef struct
{
    color  textColor;                           //color of the text
    trhandle picture;                           //what picture to draw at side
    ubyte  bPicture;                            //is there a picure?
    ubyte  pictureColorScheme;                  //color scheme this picture uses
    ubyte  colorSchemeOfPicture;                //what color scheme the picture is
    bool8  bDropShadow;                         //is there a dropshadow?
    bool8  bCentred;                            //is it centred in the region?
    sdword margin;                              //margin to use when wrapping
    fonthandle font;                            //font to use
    real32 fadeIn;                              //duration of fade-in, seconds
    real32 fadeOut;                             //duration of fade-out, seconds
}
subtheme;

//structure for commincating new subtitles from the streamer thread
typedef struct
{
    sdword actor;
    sdword speechEvent;
    sdword length;
    real32 time;
    char text[SUB_SubtitleLength];
}
subnewsubtitle;

/*=============================================================================
    Data:
=============================================================================*/
extern subregion   subRegion[SUB_NumberRegions];
extern real32      *subTimeElapsed;
extern bool8 subCloseCaptionsEnabled;
extern sdword subMessageEnded;

/*=============================================================================
    Functions:
=============================================================================*/

void subStartup(void);
void subReset(void);
void subTexturesReset(void);
void subShutdown(void);
void subRegionsRescale(void);

bool subAnyCardsOnScreen(void);

sdword subTitleAdd(sdword actor, sdword speechEvent, char *text, sdword length, real32 time);

void subTitlesUpdate(void);

void subTitlesDraw(subregion *region);
void subTitlesFadeOut(subregion *region, real32 fadeTime);
sdword subStringsChop(rectangle *region, fonthandle font, sdword longLength, char *longString, char *chopBuffer, char **choppedStrings);

#endif //___SUBTITLE_H

