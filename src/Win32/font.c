/*=============================================================================
    Name    : Font.c
    Purpose : Functions for loading and drawing fonts.

    Created 7/11/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "glinc.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "types.h"
#include "color.h"
#include "prim2d.h"
#include "memory.h"
#include "file.h"
#include "debug.h"
#include "utility.h"
#include "render.h"
#include "font.h"
#include "fontreg.h"
#include "main.h"
#include "glcaps.h"
#include "texreg.h"
#include "glcompat.h"
#include "twiddle.h"
#include "devstats.h"


/*=============================================================================
    Defines:
=============================================================================*/

//spacing between characters within a glfont page
#define GLFONT_Y_SPACING     4
#define GLFONT_X_SPACING     4
//output targas for the glfont pages if !0
#define GLFONT_OUTPUT_TARGAS 0


/*=============================================================================
    Data:
=============================================================================*/

extern udword gDevcaps;

udword fontRedBrightFactor =    FONT_RedBrightFactor;
udword fontGreenBrightFactor =  FONT_GreenBrightFactor;
udword fontBlueBrightFactor =   FONT_BlueBrightFactor;

//current font
fontheader *fontCurrentFont = NULL;
fonthandle  fontCurrent=0;

FontShadowType fontCurrentShadowType = FS_NONE;
color fontCurrentShadowColor;


/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : fontIndexToBitPlaneUpsideDown
    Description : Convert an index-color image to an upside-down bit plane (1-bit image)
    Inputs      : dest - destination buffer to recieve buffer.
                    Must be of size (width + 7) / 8 * height.
                  source - source buffer of color indeces for creating bit plane
                  width,height - size of source buffer
                  colorKey - index of value to be 0 in bit plane.  All other
                    values will create 1.
    Outputs     : Converts source to a 1-bit image in dest
    Return      : void
----------------------------------------------------------------------------*/
void fontIndexToBitPlaneUpsideDown(ubyte *dest, ubyte *source, sdword width, sdword height, ubyte colorKey)
{
    sdword x, y;
    ubyte *pDest;

    for (y = 0; y < height; y++)
    {                                                       //for all rows
        pDest = dest + (height - 1 - y) * (width + 7);      //create pointer to current line
        for (x = 0; x < width; x++, source++)
        {                                                   //for all columns
            if (*source == colorKey)
            {                                               //if key color
                pDest[x] = 0;
            }
            else
            {
                pDest[x] = 255;
            }
        }
    }
}

void fontCharacterCreate(sdword x, sdword y, sdword width, sdword height, ubyte *source, ubyte *dest, sdword sourceWidth, sdword sourceHeight, bool antialias)
{
    sdword line;

    memset(dest, 0, width * height);
    for (line = 0; line < height; line++)
    {
        memcpy(dest + ((height-1) - line) * width, source + (y + line) * sourceWidth + x, width);
    }

    if (!antialias)
    {
        ubyte* dp;
        sdword i;

        for (line = 0; line < height; line++)
        {
            dp = dest + width * line;

            for (i = 0; i < width; i++, dp++)
            {
                *dp = (ubyte)((*dp < 6) ? 0 : 15);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : glfontCopyChar
    Description : copy an 8bit region of a font image -> 32bit (RGBA)
    Inputs      : dest - destination RGBA image
                  du, dv - upper left coordinates in destination image
                  src - source 8bit image
                  su, sv - upper left coordinates in source image
                  width, height - dimensions of region to copy
                  dPitch, sPitch - pitch of dest, src buffers
    Outputs     : dest region has src region blitted into it
    Return      :
----------------------------------------------------------------------------*/
static void glfontCopyChar(color* dest, sdword du, sdword dv,
                           ubyte* src,  sdword su, sdword sv,
                           sdword width, sdword height,
                           sdword dPitch, sdword sPitch)
{
    sdword y, x;
    color* dp;
    ubyte* sp;

    for (y = 0; y < height; y++)
    {
        dp = (color*)((ubyte*)dest + dPitch*(y+dv) + 4*du);
        sp = src + sPitch*(y+sv) + su;
        for (x = 0; x < width; x++)
        {
            dp[x] = colRGBA(255,255,255, sp[x] * 16);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : glfontPackOntoPage
    Description : attempt to pack a font's characters onto a GL font page.  can
                  fail, in which case page dimensions should be adjusted
    Inputs      : header - fontheader type to convert from
                  glfont - fresh GL font type
                  page - the GL font page the characters are going onto
                  usedHeight - output for bottom of used space
                  lastChar - output for last character packed
    Outputs     : on success, usedHeight will contain height of used space.
                  on failure, usedHeight will contain -1
    Return      : new page on success, partial page on failure
----------------------------------------------------------------------------*/
color* glfontPackOntoPage(fontheader* header, glfontheader* glfont, glfontpage* page, sdword* usedHeight, sdword* lastChar)
{
    sdword index;
    charheader* character;
    glfontcharacter* glchar;
    sdword xIndex, yIndex;
    color* data;
    static sdword charIndex;

    if (header == NULL)
    {
        //reset starting character index
        charIndex = TreatAsUdword(lastChar);
        return NULL;
    }

    //allocate temporary memory
    data = (color*)memAlloc(sizeof(color) * page->width * page->height, "temp glfont page", NonVolatile);
    memset(data, 0, sizeof(color) * page->width * page->height);

    //set indices to upperleft
    xIndex = 0;
    yIndex = 0;

    //create texture from individual character bitmaps
    for (index = charIndex; index < 256; index++)
    {
        if (header->character[index] != NULL)
        {
            //get character pointer
            character = (charheader*)header->character[index];

            //create the GL character

            glchar = &glfont->character[index];
            glchar->page = page;
            if ((xIndex + character->width) >= page->width)
            {
                //eol, advance in y
                xIndex = 0;
                yIndex += header->imageHeight + GLFONT_Y_SPACING;
                if ((yIndex + header->imageHeight) >= page->height)
                {
                    //out of space, return NULL
                    if (usedHeight != NULL)
                    {
                        *usedHeight = -1;
                    }
                    if (lastChar != NULL)
                    {
                        *lastChar = index;
                    }
                    return data;
                }
            }

            //create character
            glchar->u = xIndex;
            glchar->v = yIndex;

            //copy character data
            glfontCopyChar(data, glchar->u, glchar->v,
                           character->bitmap, 0, 0,
                           character->width, character->height,
                           4*page->width, character->width);

            //update x location on font page
            xIndex += character->width + GLFONT_X_SPACING;
        }
    }

    if (usedHeight != NULL)
    {
        *usedHeight = yIndex + header->imageHeight;
    }
    if (lastChar != NULL)
    {
        *lastChar = index;
    }
    return data;
}

/*-----------------------------------------------------------------------------
    Name        : glfontRecreate
    Description : recreate the GL font pages for the given font
    Inputs      : header - fontheader type for the font
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glfontRecreate(fontheader* newHeader)
{
    glfontheader* glfont;
    sdword index;
    glfontpage* page;
    sdword dims[4] = {64, 128, 256, 0};
    sdword usedHeight, lastChar, lastPageChar;
    color* data;
    GLuint handle;

    glfont = (glfontheader*)newHeader->glFont;

    lastPageChar = 0;

    for (glfont->numPages = 1;;)
    {
    PACK_PAGE:
        //reset starting character index
        glfontPackOntoPage(NULL, NULL, NULL, NULL, (sdword*)lastPageChar);
        page = &glfont->page[glfont->numPages - 1];

        for (index = 0;; index++)
        {
            if (dims[index] == 0)
            {
                //failure, pack at largest size then create new page
                page->width  = 256;
                page->height = 256;
                data = glfontPackOntoPage(newHeader, glfont, page, &usedHeight, &lastChar);
                dbgMessagef("\nglfontCreate: page %d [%d.%d]",
                            glfont->numPages, page->width, page->height);

                //update starting character index
                lastPageChar = lastChar;

                //create the GL font page
                glGenTextures(1, &handle);
                page->glhandle = handle;
                glBindTexture(GL_TEXTURE_2D, handle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             page->width, page->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
                if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                memFree(data);

                glfont->numPages++;
                goto PACK_PAGE;
            }

            page->width  = dims[index];
            page->height = dims[index];

            //attempt to pack onto page
            data = glfontPackOntoPage(newHeader, glfont, page, &usedHeight, &lastChar);
            if (usedHeight == -1)
            {
                //failure, repack at larger dim

                //reset starting character index
                glfontPackOntoPage(NULL, NULL, NULL, NULL, (sdword*)lastPageChar);
                memFree(data);
            }
            else
            {
                //success

                //trim bottom of font page if not RIVA 128 (poor square only cap handling)
                if (!bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    page->height = bitHighExponent2(usedHeight);
                }
                dbgMessagef("\nglfontCreate: page %d [%d.%d]",
                            glfont->numPages, page->width, page->height);

                //create the GL font page
                glGenTextures(1, &handle);
                page->glhandle = handle;
                glBindTexture(GL_TEXTURE_2D, handle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             page->width, page->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
                if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                memFree(data);

                goto PACK_DONE;
            }
        }
    }

PACK_DONE:

    //1 / page.dims
    for (index = 0; index < glfont->numPages; index++)
    {
        glfont->page[index].oneOverWidth  = 1.0f / (real32)glfont->page[index].width;
        glfont->page[index].oneOverHeight = 1.0f / (real32)glfont->page[index].height;
    }
}

#if GLFONT_OUTPUT_TARGAS
#include "tga.h"
static void glfontReverse(ubyte* surf, sdword height, sdword pitch)
{
    ubyte  line[4*256];
    sdword y, top, bot;

    for (y = 0; y < (height/2); y++)
    {
        top = y;
        bot = (height - 1) - y;

        memcpy(line, surf + pitch*top, pitch);
        memcpy(surf + pitch*top, surf + pitch*bot, pitch);
        memcpy(surf + pitch*bot, line, pitch);
    }
}
static void glfontWritePage(fontheader* header, sdword nPage, glfontpage* page, color* data)
{
    char filename[32];

    glfontReverse((ubyte*)data, page->height, 4*page->width);
    sprintf(filename, "font%08X_%d.tga", (udword)header, nPage);

    //note: write must calc alphablend of fontdata
    tgaHeaderWriteRGB(filename, NULL, page->width, page->height);
    tgaBodyWriteRGB(data, page->width, page->height);
    tgaWriteClose();
}
#endif //GLFONT_OUTPUT_TARGAS

/*-----------------------------------------------------------------------------
    Name        : glfontCreate
    Description : create the GL rep of a font (texture pages for entire font)
    Inputs      : header - fontheader type to convert from
                  newHeader - fontheader type in progress of conversion
    Outputs     :
    Return      : a fresh glfontheader, fully loaded
----------------------------------------------------------------------------*/
glfontheader* glfontCreate(fontheader* header, fontheader* newHeader)
{
    glfontheader* glfont;
    sdword index;
    glfontpage* page;
    sdword dims[4] = {64, 128, 256, 0};
    sdword usedHeight, lastChar, lastPageChar;
    color* data;
    GLuint handle;

    glfont = memAlloc(sizeof(glfontheader), "glfont", NonVolatile);
    memset(glfont, 0, sizeof(glfontheader));

    lastPageChar = 0;

    for (glfont->numPages = 1;;)
    {
    PACK_PAGE:
        //reset starting character index
        glfontPackOntoPage(NULL, NULL, NULL, NULL, (sdword*)lastPageChar);
        page = &glfont->page[glfont->numPages - 1];

        for (index = 0;; index++)
        {
            if (dims[index] == 0)
            {
                //failure, pack at largest size then create new page
                page->width  = 256;
                page->height = 256;
                data = glfontPackOntoPage(newHeader, glfont, page, &usedHeight, &lastChar);
                dbgMessagef("\nglfontCreate: page %d [%d.%d]",
                            glfont->numPages, page->width, page->height);

                //update starting character index
                lastPageChar = lastChar;

                //create the GL font page
                glGenTextures(1, &handle);
                page->glhandle = handle;
                glBindTexture(GL_TEXTURE_2D, handle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             page->width, page->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
#if GLFONT_OUTPUT_TARGAS
                glfontWritePage(header, glfont->numPages, page, data);
#endif
                if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                memFree(data);

                glfont->numPages++;
                goto PACK_PAGE;
            }

            page->width  = dims[index];
            page->height = dims[index];

            //attempt to pack onto page
            data = glfontPackOntoPage(newHeader, glfont, page, &usedHeight, &lastChar);
            if (usedHeight == -1)
            {
                //failure, repack at larger dim

                //reset starting character index
                glfontPackOntoPage(NULL, NULL, NULL, NULL, (sdword*)lastPageChar);
                memFree(data);
            }
            else
            {
                //success

                //trim bottom of font page if not RIVA 128 (poor square only cap handling)
                if (!bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    page->height = bitHighExponent2(usedHeight);
                }
                dbgMessagef("\nglfontCreate: page %d [%d.%d]",
                            glfont->numPages, page->width, page->height);

                //create the GL font page
                glGenTextures(1, &handle);
                page->glhandle = handle;
                glBindTexture(GL_TEXTURE_2D, handle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             page->width, page->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
#if GLFONT_OUTPUT_TARGAS
                glfontWritePage(header, glfont->numPages, page, data);
#endif
                if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                memFree(data);

                goto PACK_DONE;
            }
        }
    }

PACK_DONE:

    //1 / page.dims
    for (index = 0; index < glfont->numPages; index++)
    {
        glfont->page[index].oneOverWidth  = 1.0f / (real32)glfont->page[index].width;
        glfont->page[index].oneOverHeight = 1.0f / (real32)glfont->page[index].height;
    }

    return glfont;
}

/*-----------------------------------------------------------------------------
    Name        : glfontDiscard
    Description : free all texobjs in a font, but no memFree of the glfontheader
    Inputs      : glfont - the GL font to work with
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glfontDiscard(glfontheader* glfont)
{
    sdword index;
    glfontpage* page;

    for (index = 0; index < glfont->numPages; index++)
    {
        page = &glfont->page[index];
        if (page->glhandle != 0)
        {
            glDeleteTextures(1, (GLuint*)&page->glhandle);
            page->glhandle = 0;
        }
    }
}

static GLuint lastGLHandle;

/*-----------------------------------------------------------------------------
    Name        : glfontDisplayCharacter
    Description : display a single character from a GL font
    Inputs      : font - the font
                  ch - the character
                  x, y - location in screenspace
                  c - colour to display the font with
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool glfontDisplayCharacter(fontheader* font, char ch, sdword x, sdword y, color c)
{
#define VERT(S,T,X,Y) \
    glTexCoord2f((real32)(S), (real32)(T)); \
    glVertex2f(primScreenToGLX(X), primScreenToGLY(Y));

    glfontheader* glfont;
    sdword charIndex;
    glfontcharacter* character;
    charheader* fcharacter;
    glfontpage* page;
    real32 sBegin, sEnd;
    real32 tBegin, tEnd;
    real32 xFrac, yFrac;

    glfont = (glfontheader*)font->glFont;
    dbgAssert(glfont != NULL);

    charIndex = (sdword)((ubyte)ch);
    fcharacter = font->character[charIndex];
    character = &glfont->character[charIndex];
    page = character->page;
    if (page == NULL)
    {
        //no page, no character
        return FALSE;
    }

    //texture coordinates in font page
    sBegin = (real32)character->u * page->oneOverWidth;
    sEnd   = (real32)(character->u + fcharacter->width) * page->oneOverWidth;
    tBegin = (real32)character->v * page->oneOverHeight;
    tEnd   = (real32)(character->v + fcharacter->height) * page->oneOverHeight;

    if (bitTest(gDevcaps, DEVSTAT_YADJUST))
    {
        yFrac = 0.1f * page->oneOverHeight;
        tBegin += yFrac;
        tEnd   -= yFrac;
    }
    if (bitTest(gDevcaps, DEVSTAT_XADJUST))
    {
        xFrac = 0.1f * page->oneOverWidth;
        sBegin += xFrac;
        sEnd   -= xFrac;
    }
    if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
    {
        xFrac = 0.5f * page->oneOverWidth;
        sEnd += xFrac;
    }

    //only switch textures if new page differs from last
    if (lastGLHandle != page->glhandle)
    {
        glBindTexture(GL_TEXTURE_2D, page->glhandle);
    }
    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glBegin(GL_QUADS);

    VERT(sBegin, tEnd,
         x + fcharacter->offsetX,
         y + fcharacter->offsetY);
    VERT(sBegin, tBegin,
         x + fcharacter->offsetX,
         y + fcharacter->offsetY + fcharacter->height);
    VERT(sEnd, tBegin,
         x + fcharacter->offsetX + fcharacter->width,
         y + fcharacter->offsetY + fcharacter->height);
    VERT(sEnd, tEnd,
         x + fcharacter->offsetX + fcharacter->width,
         y + fcharacter->offsetY);

    glEnd();

    //set last used texobj
    lastGLHandle = page->glhandle;

    return TRUE;
#undef VERT
}

/*-----------------------------------------------------------------------------
    Name        : glfontDisplayString
    Description : display a string of characters from a GL font
    Inputs      : font - the font
                  string - the string to display
                  x, y - screenspace location to display at
                  c - colour
    Outputs     :
    Return      : TRUE or FALSE (success or failure)
----------------------------------------------------------------------------*/
bool glfontDisplayString(fontheader* font, char* string, sdword x, sdword y, color c)
{
    char* charp;
    glfontheader* glfont;
    glfontcharacter* character;
    charheader* fcharacter;
    sdword sx, sy;
    bool texOn, blendOn, alphatestOn;
    bool rval;
    sdword brights[3];
    color bright, colour;

    glfont = (glfontheader*)font->glFont;

    //is there actually a font to display ?
    if (glfont == NULL || glfont->numPages == 0)
    {
        return FALSE;
    }

    //highlighted font colour
    brights[0] = colRed(c) * fontRedBrightFactor / 256;
    brights[1] = colGreen(c) * fontGreenBrightFactor / 256;
    brights[2] = colBlue(c) * fontBlueBrightFactor / 256;
    //clamp
    if (brights[0] > 255) brights[0] = 255;
    if (brights[1] > 255) brights[1] = 255;
    if (brights[2] > 255) brights[2] = 255;
    //set bright color
    bright = colRGB(brights[0], brights[1], brights[2]);

    //set up GL state
    trClearCurrent();
    lastGLHandle = 0;
    texOn = rndTextureEnable(TRUE);
    blendOn = (bool)glIsEnabled(GL_BLEND);
    alphatestOn = (bool)glIsEnabled(GL_ALPHA_TEST);
    if (!blendOn) glEnable(GL_BLEND);
    if (alphatestOn) glDisable(GL_ALPHA_TEST);
    rndAdditiveBlends(FALSE);
    rndPerspectiveCorrection(FALSE);
    glShadeModel(GL_FLAT);
    rndTextureEnvironment(RTE_Modulate);

    //initial screen location
    sx = x;
    sy = y;

    rval = TRUE;

    for (charp = string; *charp != '\0'; charp++)
    {
        character = &glfont->character[(ubyte)*charp];
        if (character->page == NULL)
        {
            //this character doesn't exist
            sx += fontCurrentFont->spacing;
            continue;
        }
        //test for highlight escape sequence
        if (*charp == '&')
        {
            charp++;    //advance to next character
            if (*charp == '\0')
            {
                //eos
                break;
            }
            character = &glfont->character[(ubyte)*charp];
            if (*charp == '&')
            {
                //must escape & to get an &
                colour = c;
            }
            else
            {
                //highlight colour
                colour = bright;
            }
        }
        else
        {
            //normal colour
            colour = c;
        }
        fcharacter = font->character[(ubyte)*charp];
        //display the character
        (void)glfontDisplayCharacter(font, *charp, sx, sy, colour);
        //advance screen location
        sx += fcharacter->width + fontCurrentFont->spacing;
    }

    //reset state
    rndTextureEnable(texOn);
    if (!blendOn) glDisable(GL_BLEND);
    if (alphatestOn) glEnable(GL_ALPHA_TEST);

    return rval;
}

/*-----------------------------------------------------------------------------
    Name        : fontLoad
    Description : Load in a font.
    Inputs      : fileName - name of font file
    Outputs     : allocates memory for font
    Return      : handle to font
----------------------------------------------------------------------------*/
fontheader *fontLoad(char *fileName)
{
    fontfileheader *fileHeader;
    fontheader *header, *newHeader;
    sdword length, index;
    sdword iCharacter;
    charheader *pCharacter;
    charfileheader *pFileCharacter;
    sdword sizeTotal, size, sizeUsed;
    ubyte *bitmapBase;
    bool antialias;

    antialias = TRUE;

    length = fileLoadAlloc(fileName, (void **)(&fileHeader), NonVolatile);

#if FONT_ERROR_CHECKING
    if (strcmp(fileHeader->identification, FONT_Identification))
    {                                                       //if wrong header
        dbgFatalf(DBG_Loc, "fontLoad: Invalid header '%s' in file '%s'", fileHeader->identification, fileName);
    }
    if (fileHeader->version != FONT_Version)
    {                                                       //if wrong version
        dbgFatalf(DBG_Loc, "fontLoad: expected version 0x%x, found version 0x%x", FONT_Version, fileHeader->version);
    }
#endif
    header = &fileHeader->header;
    dbgAssert(fileHeader->flags == 0);                          //!!! don't yet support color or anti-aliased fonts
#if FONT_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfontLoad: loaded %d bytes and %d characters from file '%s'", length, header->nCharacters, fileName);
#endif

    header->image += (udword)fileHeader;                    //fix-up image pointer

    header->reserved[0] = (ubyte)antialias;

    //compute size required for 8-bit images of each character
    sizeTotal = 0;
    for (index = iCharacter = 0; index < 256; index++)
    {                                                       //for all possible characters
        if (header->character[index] != NULL)
        {
            (ubyte *)header->character[index] += (udword)fileHeader;//fix up character pointer
            pFileCharacter = (charfileheader *)header->character[index];//get reference to data loaded from disk
            size = (pFileCharacter->width - pFileCharacter->offsetX) *
                   (pFileCharacter->height - pFileCharacter->offsetY);
            sizeTotal += size;
        }
    }
    //allocate data for new header, character indices and 8-bit bitmaps
    newHeader = memAlloc(fontHeaderSize(header->nCharacters) +
                         header->nCharacters * sizeof(charheader) +
                         sizeTotal,
                         "Font header", NonVolatile);

    *newHeader = *header;                                   //duplicate the header
    pCharacter = (charheader *)((ubyte *)newHeader +
                 fontHeaderSize(header->nCharacters));
    bitmapBase = (ubyte *)pCharacter + header->nCharacters * sizeof(charheader);
    sizeUsed = 0;
    for (index = iCharacter = 0; index < 256; index++)
    {                                                       //for all possible characters
        if (header->character[index] != NULL)
        {
            newHeader->character[index] = pCharacter;       //set character * for this character
            pFileCharacter = (charfileheader *)header->character[index];//get reference to data loaded from disk
            pCharacter->width = pFileCharacter->width;      //duplicate attributes
            pCharacter->height = pFileCharacter->height;
            pCharacter->offsetX = pFileCharacter->offsetX;
            pCharacter->offsetY = pFileCharacter->offsetY;
            pCharacter->bitmap = bitmapBase + sizeUsed;
            fontCharacterCreate(pFileCharacter->u + pFileCharacter->offsetX,//create an 8-bit character map
                                pFileCharacter->v + pFileCharacter->offsetY,
                                pFileCharacter->width - pFileCharacter->offsetX,
                                pFileCharacter->height - pFileCharacter->offsetY,
                                header->image, pCharacter->bitmap,
                                header->imageWidth, header->imageHeight,
                                antialias);
            iCharacter++;                                   //increment character index and pointer
            pCharacter++;
            size = (pFileCharacter->width - pFileCharacter->offsetX) *
                   (pFileCharacter->height - pFileCharacter->offsetY);
            sizeUsed += size;

            dbgAssert(sizeUsed <= sizeTotal);                //make sure not too much RAM used
            dbgAssert(iCharacter <= header->nCharacters);    //make sure not too many characters
        }
    }

    newHeader->glFont = (void*)glfontCreate(&fileHeader->header, newHeader);

    memFree(fileHeader);
    return(newHeader);
}

/*-----------------------------------------------------------------------------
    Name        : fontDiscardGL
    Description : discard (free) a font's GL texture handles
    Inputs      : font - font to free
    Outputs     : frees all texture names associated with font
    Return      :
----------------------------------------------------------------------------*/
void fontDiscardGL(fontheader* font)
{
    sdword i;

    for (i = 0; i < 256; i++)
    {
        if (font->glFont != NULL)
        {
            glfontDiscard((glfontheader*)font->glFont);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : fontDiscard
    Description : Discard (free) a font
    Inputs      : font - handle of font to free
    Outputs     : Frees all memory associated with font
    Return      : void
----------------------------------------------------------------------------*/
void fontDiscard(fonthandle font)
{
#if FONT_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfontDiscard: Freeing font 0x%x", font);
#endif
    fontDiscardGL(frFontRegistry[font].fontdat);
    if (frFontRegistry[font].fontdat->glFont != NULL)
    {
        glfontDiscard((glfontheader*)frFontRegistry[font].fontdat->glFont);
        memFree(frFontRegistry[font].fontdat->glFont);
    }
    memFree(frFontRegistry[font].fontdat);  //!!! is this enough???
}

/*-----------------------------------------------------------------------------
    Name        : fontMakeCurrent
    Description : Select font for future drawing operations.
    Inputs      : font - handle of font to make current
    Outputs     : copies font to fontCurrentFont
    Return      :
----------------------------------------------------------------------------*/
fonthandle fontMakeCurrent(fonthandle font)
{
    fonthandle old;
    old = fontCurrent;
    fontCurrent = font;
    fontCurrentFont = frFontRegistry[font].fontdat;
    return(old);
}

/*-----------------------------------------------------------------------------
    Name        : fontCurrentGet
    Description : Get handle of current font
    Inputs      : void
    Outputs     : none
    Return      : handle of current font
----------------------------------------------------------------------------*/
fonthandle fontCurrentGet(void)
{
    return(fontCurrent);
}

/*-----------------------------------------------------------------------------
    Name        : fontPrintCentreCentreRectangle
    Description : prints a text string totally centred using current font in rect
    Inputs      : rect, c, string
    Outputs     :
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrintCentreCentreRectangle(rectangle *rect, color c, char *string)
{
    sdword x;
    sdword width = fontWidth(string);
    sdword y;
    sdword height = fontHeight(NULL);
    sdword rectwidth = rect->x1 - rect->x0;
    sdword rectheight = rect->y1 - rect->y0;

    if (width >= rectwidth)
    {
        x = rect->x0;
    }
    else
    {
        x = ((rectwidth - width) >> 1) + rect->x0;
    }

    if (height >= rectheight)
    {
        y = rect->y0;
    }
    else
    {
        y = ((rectheight - height) >> 1) + rect->y0;
    }

    return fontPrint(x,y,c,string);
}

/*-----------------------------------------------------------------------------
    Name        : fontPrintCentre
    Description : prints a text string using current font, centred
    Inputs      : y, c, string
    Outputs     :
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrintCentre(sdword y, color c, char *string)
{
    sdword x;
    sdword width = fontWidth(string);

    if (width >= MAIN_WindowWidth)
    {
        x = 0;
    }
    else
    {
        x = (MAIN_WindowWidth - width) >> 1;
    }

    return fontPrint(x,y,c,string);
}

#define SHADOW_COLOR_ADJUST 2

/*-----------------------------------------------------------------------------
    Name        : fontPrint
    Description : Print a text string using current font
    Inputs      : x, y - location of top-left corner of font
                  c - color to draw text in
                  string - string to render
    Outputs     : Creates a quadrangle texture for each texture.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrint(sdword x, sdword y, color c, char *string)
{
    return fontPrintN(x, y, c, string, SDWORD_Max);
}

/*-----------------------------------------------------------------------------
//
//  set font shadow type for subsequent text output
//  (you may OR multiple shadow directions together)
//
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        : fontShadowSet
    Description : Set current dropshadow properties
    Inputs      : s - shadow cardinal direction or FS_NONE for none
                  c - color to draw drop shadow in
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void fontShadowSet(FontShadowType s, color c)
{
    fontCurrentShadowType = s;
    fontCurrentShadowColor = c;
}

/*-----------------------------------------------------------------------------
//
//  returns current shadow type
//
----------------------------------------------------------------------------*/
FontShadowType fontShadowGet(void)
{
    return fontCurrentShadowType;
}

/*-----------------------------------------------------------------------------
    Name        : fontPrintN
    Description : Print a text string using current font
    Inputs      : x, y - location of top-left corner of font
                  c - color to draw text in
                  string - string to render
                  maxCharacters - max number of characters to print
    Outputs     : Creates a quadrangle texture for each texture.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrintN(sdword x, sdword y, color c, char *string, sdword maxCharacters)
{
    charheader *character;
    real32 imageWidth, imageHeight, clipHeight = 0.0f;
    sdword clip = 0;
    ubyte ch;
    udword brightRed, brightGreen, brightBlue;
    udword red, green, blue;
    sdword xOffset, yOffset;
    FontShadowType saveType;

    if (fontCurrentShadowType != FS_NONE)
    {
        // shadow the text as requested
        xOffset = yOffset = 0;
        switch (fontCurrentShadowType)
        {
            case FS_N :
                yOffset = -1;
                break;
            case FS_S :
                yOffset = 1;
                break;
            case FS_E :
                xOffset = 1;
                break;
            case FS_W :
                xOffset = -1;
                break;
            case FS_NE:
                yOffset = -1;
                xOffset = 1;
                break;
            case FS_NW:
                yOffset = -1;
                xOffset = -1;
                break;
            case FS_SE:
                yOffset = 1;
                xOffset = 1;
                break;
            case FS_SW:
                yOffset = 1;
                xOffset = -1;
                break;
        }
        saveType = fontCurrentShadowType;
        fontCurrentShadowType = FS_NONE;
        fontPrintN(x + xOffset, y + yOffset, fontCurrentShadowColor, string, SDWORD_Max);
        fontCurrentShadowType = saveType;
    }

    //display the GL font-page version if we "should" and "can"
    if (RGLtype != SWtype && !glcActive() &&
        glfontDisplayString(fontCurrentFont, string, x, y, c))
    {
        return TRUE;
    }

#if FONT_MANUAL_CLIP
/*
    if (x <= (-MAIN_WindowWidth) || x >= MAIN_WindowWidth || y <= (-MAIN_WindowHeight) || y >= MAIN_WindowHeight)
    {                                                       //if coordinates not reasonable
        return(ERROR);
    }
*/
    if (y <= -fontCurrentFont->fullHeight)
    {
        return(ERROR);
    }
    if (MAIN_WindowHeight - y <= fontCurrentFont->fullHeight)
    {                                                       //if partly off bottom
        if (y >= MAIN_WindowHeight)
        {                                                   //if fully off bottom
            return(ERROR);
        }
/*
        else
        {
            clipHeight = (real32)(MAIN_WindowHeight - y);
            clip = MAIN_WindowHeight - y;
        }
*/
    }
#endif

    dbgAssert(fontCurrentFont != NULL);
    red = brightRed = colRed(c);
    green = brightGreen = colGreen(c);
    blue = brightBlue = colBlue(c);
    glColor3ub((ubyte)brightRed, (ubyte)brightGreen, (ubyte)brightBlue);
    brightRed = brightRed * fontRedBrightFactor / 256;
    if (brightRed > 255)
    {
        brightRed = 255;
    }
    brightGreen = brightGreen * fontGreenBrightFactor / 256;
    if (brightGreen > 255)
    {
        brightGreen = 255;
    }
    brightBlue = brightBlue * fontBlueBrightFactor / 256;
    if (brightBlue > 255)
    {
        brightBlue = 255;
    }
#if FONT_COLOR_HACK
    glBegin(GL_POINTS);
    glVertex2f(primScreenToGLX(-1), primScreenToGLY(-1));   //work around color bug in 3DFxGL
    glEnd();
#endif

    imageWidth = (real32)fontCurrentFont->imageWidth;       //get floating-point image dimesions
    imageHeight = (real32)fontCurrentFont->imageHeight;
    while (*string && maxCharacters > 0)
    {                                                       //while not at end of string
#if FONT_ALL_CAPS
        ch = toupper(*string);
#else
        ch = *string;
#endif
        if (ch == '&')
        {
            string++;
#if FONT_ALL_CAPS
            ch = toupper(*string);
#else
            ch = *string;
#endif
            if (ch == 0)
            {
                break;
            }
            else if (ch != '&')
            {
                glColor3ub((ubyte)brightRed, (ubyte)brightGreen, (ubyte)brightBlue);
            }
        }
        else
        {
            glColor3ub((ubyte)red, (ubyte)green, (ubyte)blue);
        }
        character = fontCurrentFont->character[(ubyte)ch]; //get character header
        if (character == NULL)
        {                                                   //if character not supported
            x += fontCurrentFont->spacing;                  //put a space instead of a character
            string++;                                       //and character pointer
            maxCharacters--;
            continue;
        }
        if (x < 0)
        {                                                   //if off left
            goto noDraw;
        }
        if (x > MAIN_WindowWidth - character->width)
        {                                                   //if off right
            goto noDraw;
        }

        glRasterPos2f(primScreenToGLX(x + character->offsetX),
                      primScreenToGLY(y + fontCurrentFont->fullHeight + character->offsetY - clip));

        glDrawPixels(character->width, character->height - clip, GL_RGB8, GL_UNSIGNED_BYTE, character->bitmap);

noDraw:
        x += character->width + fontCurrentFont->spacing;   //update screen location
        string++;                                           //and character pointer
        maxCharacters--;
    }

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : fontPrintf
    Description : Print a formatted text string using current font
    Inputs      : Same as font print except for format string and format params
    Outputs     :
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrintf(sdword x, sdword y, color c, char *format, ...)
{
    char buffer[DBG_BufferLength];
    va_list argList;
    va_start(argList, format);                              //get first arg
    vsprintf(buffer, format, argList);                      //prepare output string
    va_end(argList);
    return(fontPrint(x, y, c, buffer));                     //actually print
}

/*-----------------------------------------------------------------------------
    Name        : fontWidthN
    Description : Get the width, in pixels of a string up to a maximum number of letters
    Inputs      : string - string to measure
                  maxCharacters - maximum number of characters
    Outputs     :
    Return      : Width in pixels of string.  This will be the same number of
                    pixels used if the string is printed.
----------------------------------------------------------------------------*/
sdword fontWidthN(char *string, sdword maxCharacters)
{
    charheader *character;
    ubyte ch;
    sdword width = 0;

    while (*string && maxCharacters > 0)
    {                                                       //while not at end of string or exceeding a number of characters
#if FONT_ALL_CAPS
        ch = toupper(*string);
#else
        ch = *string;
#endif
        if (ch == '&')
        {
            string++;
#if FONT_ALL_CAPS
            ch = toupper(*string);
#else
            ch = *string;
#endif
            if (ch == 0)
            {
                break;
            }
        }
//        dbgAssert((sdword)ch < fontCurrentFont->nCharacters);
        character = fontCurrentFont->character[(ubyte)ch];  //get character header
        if (character == NULL)
        {                                                   //if character not supported
            width += fontCurrentFont->spacing;              //put a space instead of a character
            string++;                                       //and character pointer
            maxCharacters--;
            continue;
        }
        width += character->width + fontCurrentFont->spacing;//update screen location
        string++;                                           //and character pointer
        maxCharacters--;
    }
    return(width);
}

/*-----------------------------------------------------------------------------
    Name        : fontWidth
    Description : Get the width, in pixels of a string
    Inputs      :

    Outputs     :
    Return      : Width in pixels of string.  This will be the same number of
                    pixels used if the string is printed.
----------------------------------------------------------------------------*/
sdword fontWidth(char *string)
{
    return(fontWidthN(string, SDWORD_Max));
}

/*-----------------------------------------------------------------------------
    Name        : fontWidthf
    Description : Width of a formatted string
    Inputs      :
    Outputs     :
    Return      : Windth, in pixels required to print this formatted string.
----------------------------------------------------------------------------*/
sdword fontWidthf(char *format, ...)
{
    char buffer[DBG_BufferLength];
    va_list argList;
    va_start(argList, format);                              //get first arg
    vsprintf(buffer, format, argList);                      //prepare output string
    va_end(argList);
    return(fontWidth(buffer));                              //actual width
}

/*-----------------------------------------------------------------------------
    Name        : fontHeight/fontHeightf
    Description : Height of a printed font (same regardless of string)
    Inputs      :
    Outputs     :
    Return      : Height of currently selected font.
----------------------------------------------------------------------------*/
sdword fontHeight(char *string)
{
    return(fontCurrentFont->fullHeight);
}
sdword fontHeightf(char *format, ...)
{
    return(fontCurrentFont->fullHeight);
}

