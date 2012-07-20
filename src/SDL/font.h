/*=============================================================================
    Name    : font.h
    Purpose : Definitions for drawing text strings

    Created 7/11/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___FONT_H
#define ___FONT_H

#include "Types.h"
#include "color.h"
#include "prim2d.h"

/*=============================================================================
    Switches
=============================================================================*/
#ifndef HW_Release

#define FONT_ERROR_CHECKING     1               //general error checking
#define FONT_VERBOSE_LEVEL      1               //print extra info
#define FONT_HACK_LOAD          0               //just pretend to load a font file
#define FONT_TEST               1               //test the task module
#define FONT_SAVE_TEXTURE       1               //save texture handle while printing fonts
#define FONT_ALL_CAPS           0               //print everything using caps
#define FONT_IGNORE_NON_ALPHA   1               //ignore non-alphabetical characters
#define FONT_COLOR_HACK         0               //work around a shortcoming in 3dFXGL
#define FONT_MANUAL_CLIP        1               //do a check to see if font is off-screen

#else //HW_Debug

#define FONT_ERROR_CHECKING     0               //general error checking
#define FONT_VERBOSE_LEVEL      0               //print extra info
#define FONT_HACK_LOAD          0               //just pretend to load a font file
#define FONT_TEST               0               //test the task module
#define FONT_SAVE_TEXTURE       1               //save texture handle while printing fonts
#define FONT_ALL_CAPS           0               //print everything using caps
#define FONT_IGNORE_NON_ALPHA   1               //ignore non-alphabetical characters
#define FONT_COLOR_HACK         0               //work around a shortcoming in 3dFXGL
#define FONT_MANUAL_CLIP        1               //do a check to see if font is off-screen

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define FONT_NumberCharacters   256             //max. number of characters in a font
#define FONT_Version            0x100           //current font file version
#define FONT_Identification     "Orannge"       //another mispelled fruit
#define FONT_IdentLength        8               //length of mispelled fruit
#define FONT_InvalidFontHandle  0xffffffff      //this is not a fonthandle

//font flags
#define FF_Color                0x0001          //color font
#define FF_Alpha                0x0002          //alpha-blending enabled

//font brightening coefficients
#define FONT_RedBrightFactor    350
#define FONT_GreenBrightFactor  350
#define FONT_BlueBrightFactor   350

//shadows
typedef enum {
    // cardinal directions of font shadows -- combine these at will
    FS_NONE = 0,
    FS_N  = 1,
    FS_S  = 2,
    FS_E  = 4,
    FS_W  = 8,
    FS_NE = 16,
    FS_NW = 32,
    FS_SE = 64,
    FS_SW = 128
}
FontShadowType;

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure of a character header
typedef struct
{
    ubyte *bitmap;                              //pointer to bitmap for character (was offset x/y)
    sword width, height;                        //size of character
    sword offsetX, offsetY;                     //offset in screen space to draw
}
charheader;
typedef struct
{
    sword u, v;                                 //location of top-left corner of font
    sword width, height;                        //size of image
    sword offsetX, offsetY;                     //offset in screen space to draw
}
charfileheader;

//structure of a font
typedef struct
{
    sdword nCharacters;                         //number of characters in font
    sdword spacing;                             //inter-character spacing
    sdword fullHeight;                          //total height of font, from top of tallest char to bottom of lowest hanging characters
    sdword baseLine;                            //y-value of baseline
    char *name;                                 //optional name pointer
    sdword imageWidth;                          //dimensions of font image
    sdword imageHeight;
    sdword nColors;                             //colors in image palette
    color *palette;                             //pointer to palette
    ubyte *image;                               //pointer to image
    void*  glFont;
    ubyte reserved[12];                         //reserved for expansion of future attributes
    charheader *character[256];                 //full character map
}
fontheader;

//structure for a file header
typedef struct
{
    char identification[FONT_IdentLength];      //file type string
    uword version;                              //current version
    uword flags;                                //flags for font
    fontheader header;
}
fontfileheader;

#define FONT_GL_MAXPAGES 4

//structure for a GL font page
typedef struct
{
    sdword width, height;                       //dimensions
    real32 oneOverWidth, oneOverHeight;         //1 / [dimensions]
    udword glhandle;                            //GL texture object
    udword pad;
} glfontpage;

//structure for a single GL font character
typedef struct
{
    glfontpage* page;                           //page this character resides on
    sword u, v;                                 //location on page
} glfontcharacter;

//structure for a GL font (fonts as texture pages)
typedef struct
{
    sdword numPages;                            //number of font pages for this font
    glfontpage page[FONT_GL_MAXPAGES];          //page list
    glfontcharacter character[256];             //character list
} glfontheader;

//handle on fonts
typedef udword fonthandle;

/*=============================================================================
    Macros
=============================================================================*/
#define fontHeaderSize(nChars) (sizeof(fontheader))

/*=============================================================================
    Functions:
=============================================================================*/
//load in a font
fontheader *fontLoad(char *fileName);
void fontDiscard(fonthandle font);
void fontDiscardGL(fontheader* font);

//select a font
fonthandle fontMakeCurrent(fonthandle font);
fonthandle fontCurrentGet(void);

void fontShadowSet(FontShadowType s, color color);
FontShadowType fontShadowGet(void);

//printing fonts
sdword fontPrintCentre(sdword y, color c, char *string);
sdword fontPrintCentreCentreRectangle(rectangle *rect, color c, char *string);
sdword fontPrint(sdword x, sdword y, color c, char *string);
sdword fontPrintN(sdword x, sdword y, color c, char *string, sdword maxCharacters);
sdword fontPrintf(sdword x, sdword y, color c, char *format, ...);
sdword fontWidth(char *string);
sdword fontWidthN(char *string, sdword maxCharacters);
sdword fontWidthf(char *format, ...);
sdword fontHeight(char *string);
sdword fontHeightf(char *format, ...);

void glfontRecreate(fontheader* header);


#endif //___FONT_H
