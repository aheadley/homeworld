/*=============================================================================
    Name    : Teams.c
    Purpose : Code and data for maintaining certain team-specific data, such as
        player's chosen colors and badges.

    Created 9/30/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "StatScript.h"
#include "SpaceObj.h"
#include "Universe.h"
#include "StatScript.h"
#include "Tactical.h"
#include "Teams.h"

#define TE_DorkyCheaterBaseColor    colRGB(255, 62, 164)
#define TE_DorkyCheaterStripeColor  colRGB(255, 62, 164)
#define TE_DarkColorGraceFactor     200 / 255

/*=============================================================================
    Data:
=============================================================================*/
tecolorscheme teColorSchemes[TE_NumberPlayers];
trcolorinfo teRace1Default, teRace2Default;
trhlscolorize teTrailColorize[TE_NumberTrailColors];//how to colorize the trails
#if !TO_STANDARD_COLORS
trhlscolorize teTacticalColorize;                 //how to colorize the TO
#endif
//different dot colors for friendlies, allies, enemies and neutral
#if TO_STANDARD_COLORS
color teFriendlyColor;
color teAlliedColor;
color teHostileColor;
color teNeutralColor;
color teCrateColor;
color teResourceColor;
#endif

//adjust a team color for team color flat polygons
sdword teSpecularAdjust = TE_SpecularAdjust;
sdword teDiffuseAdjust  = TE_DiffuseAdjust;
sdword teAmbientAdjust  = TE_AmbientAdjust;
scriptEntry teColorTweaks[] =
{
    { "TrailColor",     teTrailColorSet,NULL},  //player #, control point #, R, G, B
#if !TO_STANDARD_COLORS
    { "TOColor",        teColorSet, &teColorSchemes[0].tacticalColor},  //player #, R, G, B
#endif
    { "BaseColor",      teColorSet, &teColorSchemes[0].textureColor.base},  //player #, R, G, B
    { "StripeColor",    teColorSet, &teColorSchemes[0].textureColor.detail}, //player #, R, G, B
    { "SpecularAdjust", scriptSetSwordCB, &teSpecularAdjust},           //player #, R, G, B
    { "DiffuseAdjust",  scriptSetSwordCB, &teDiffuseAdjust},           //player #, R, G, B
    { "AmbientAdjust", scriptSetSwordCB, &teAmbientAdjust},           //player #, R, G, B
#if !TO_STANDARD_COLORS
    { "BaseToTOFactors",teColorFactorsSet, &teTacticalColorize},  //H, L, S
#endif
    { "BaseToTrailFactors0",teColorFactorsSet,&teTrailColorize[0]},//H, L, S
    { "BaseToTrailFactors1",teColorFactorsSet,&teTrailColorize[1]},//H, L, S
    { "BaseToTrailFactors2",teColorFactorsSet,&teTrailColorize[2]},//H, L, S
    { "BaseToTrailFactors3",teColorFactorsSet,&teTrailColorize[3]},//H, L, S
#if TO_STANDARD_COLORS
    { "FriendlyColor",              scriptSetRGBCB,    &teFriendlyColor },
    { "AlliedColor",                scriptSetRGBCB,    &teAlliedColor   },
    { "HostileColor",               scriptSetRGBCB,    &teHostileColor  },
    { "NeutralColor",               scriptSetRGBCB,    &teNeutralColor  },
    { "CtateColor",                 scriptSetRGBCB,    &teCrateColor    },
    { "ResourceColor",              scriptSetRGBCB,    &teResourceColor },
#endif
    { NULL, NULL, 0 }
};

/*=============================================================================
    Script-parsing functions:
=============================================================================*/
void teTrailColorSet(char *directory,char *field,void *dataToFillIn)
{
    sdword iPlayer, iPoint, red, green, blue, nScanned;

    nScanned = sscanf(field, "%d,%d,%d,%d,%d", &iPlayer, &iPoint, &red, &green, &blue);
    dbgAssert(nScanned == 5);
    dbgAssert(iPlayer >= 0 && iPlayer < TE_NumberPlayers);
    dbgAssert(iPoint >= 0 && iPoint < TE_NumberTrailColors);
    dbgAssert(red >= 0 && red < 256);
    dbgAssert(green >= 0 && green < 256);
    dbgAssert(blue >= 0 && blue < 256);
    teColorSchemes[iPlayer].trailColors[iPoint] = colRGB(red, green, blue);
}

/*-----------------------------------------------------------------------------
    Name        : teColorSet
    Description : Set a color for a particular color scheme structure.
    Inputs      : script callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void teColorSet(char *directory,char *field,void *dataToFillIn)
{
    sdword iPlayer, red, green, blue, nScanned;
    color *dest;

    nScanned = sscanf(field, "%d,%d,%d,%d", &iPlayer, &red, &green, &blue);
    dbgAssert(nScanned == 4);
    dbgAssert(iPlayer >= 0 && iPlayer < TE_NumberPlayers);
    dbgAssert(red >= 0 && red < 256);
    dbgAssert(green >= 0 && green < 256);
    dbgAssert(blue >= 0 && blue < 256);
    dest = (color *)(((ubyte*)(&teColorSchemes[iPlayer])) +
                      (udword)dataToFillIn - (udword)(&teColorSchemes));
//    teColorSchemes[iPlayer].tacticalColor = colRGB(red, green, blue);
    *dest = colRGB(red, green, blue);
}

/*-----------------------------------------------------------------------------
    Name        : teColorFactorsSet
    Description : Set a triplet of factors for determining how to colorize a
                    value based upon the team base color.
    Inputs      : "iPlayer, hue, lum, sat": hue is added with wrapping,
                    sat and val are multiplied and clamped.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void teColorFactorsSet(char *directory,char *field,void *dataToFillIn)
{
    real32 hue, sat, lum;
    sdword nScanned;
    trhlscolorize *colorize;

    colorize = (trhlscolorize *)dataToFillIn;
    dbgAssert(colorize != NULL);
    nScanned = sscanf(field, "%f,%f,%f", &hue, &lum, &sat);
    dbgAssert(nScanned == 3);
    dbgAssert(hue >= 0.0f && hue <= 1.0f);
    dbgAssert(lum >= 0.0f && lum <= 4.0f);
    dbgAssert(sat >= 0.0f && sat <= 4.0f);
    colorize->hue = hue;
    colorize->lum = lum;
    colorize->sat = sat;
}

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : teStartup
    Description : Start the team module
    Inputs      :
    Outputs     : loads in the team color script
    Return      :
----------------------------------------------------------------------------*/
void teReset(void)
{
    tecolorscheme tempScheme;

    scriptSet(NULL, TE_ColorScript, teColorTweaks);
    teRace1Default = teColorSchemes[0].textureColor;
    teRace2Default = teColorSchemes[1].textureColor;
    if (whichRaceSelected == R2)
    {
        swap(teColorSchemes[0], teColorSchemes[1], tempScheme);
    }
}
void teStartup(void)
{
    teReset();
}

void teShutdown(void)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : teColorAdjust
    Description : Adjust a color according to a HLS colorizing table.
    Inputs      : baseColor - origional color
                  colorize - HLS parameters to adjust by.
    Outputs     :
    Return      : newly adjusted color
----------------------------------------------------------------------------*/
color teColorAdjust(color baseColor, trhlscolorize *colorize)
{
    real32 red, green, blue, hue, lum, sat;
    colRGBToHLS(&hue, &lum, &sat, colUbyteToReal(colRed(baseColor)),
                colUbyteToReal(colGreen(baseColor)),
                colUbyteToReal(colBlue(baseColor)));
    hue += colorize->hue;
    if (hue < 0)
    {
        hue += 1.0f;
    }
    if (hue > 1.0f)
    {
        hue -= 1.0f;
    }
    lum *= colorize->lum;
    sat *= colorize->sat;
    if (lum > 1.0f)
    {
        lum = 1.0f;
    }
    if (sat > 1.0f)
    {
        sat = 1.0f;
    }
    colHLSToRGB(&red, &green, &blue, hue, lum, sat);
    return(colRGB(colRealToUbyte(red), colRealToUbyte(green), colRealToUbyte(blue)));
}

/*-----------------------------------------------------------------------------
    Name        : teTeamColorsSet
    Description : Set a team's colors.  The TO color trail colors will be
                    derived from the base layer color.
    Inputs      : iTeam - index of team to set color for
                  baseColor/stripeColor - color to make the textures.
    Outputs     : recomputes the all colors for this team
    Return      : void
----------------------------------------------------------------------------*/
extern sdword cpDarkestColor0;
void teTeamColorsSet(sdword iTeam, color baseColor, color stripeColor)
{
    sdword index;
    real32 red, green, blue, hue, sat, val;

    dbgAssert(iTeam >= 0 && iTeam < TE_NumberPlayers);

    if (multiPlayerGame)
    {                                                       //if somebody in a multiplayer game
        red = colUbyteToReal(colRed(baseColor));
        green = colUbyteToReal(colGreen(baseColor));
        blue = colUbyteToReal(colBlue(baseColor));
        colRGBToHSV(&hue, &sat, &val, red, green, blue);
        if (colRealToUbyte(val) < cpDarkestColor0 * TE_DarkColorGraceFactor)
        {                                                   //is trying to set his ships to black
            baseColor = TE_DorkyCheaterBaseColor;
            stripeColor = TE_DorkyCheaterStripeColor;
        }
    }
    teColorSchemes[iTeam].textureColor.base = baseColor;
    teColorSchemes[iTeam].textureColor.detail = stripeColor;

#if !TO_STANDARD_COLORS
    teColorSchemes[iTeam].tacticalColor =
        teColorAdjust(baseColor, &teTacticalColorize);
#endif
    for (index = 0; index < TE_NumberTrailColors; index++)
    {
        teColorSchemes[iTeam].trailColors[index] =
            teColorAdjust(baseColor, &teTrailColorize[index]);
    }
    for (iTeam = 0; iTeam < TE_NumberPlayers; iTeam++)
    {
        teColorSchemes[iTeam].ambient = colRGB(colClamp256(colRed(teColorSchemes[iTeam].textureColor.base) * teAmbientAdjust / 65536),
            colClamp256(colGreen(teColorSchemes[iTeam].textureColor.base) * teAmbientAdjust / 65536),
            colClamp256(colBlue(teColorSchemes[iTeam].textureColor.base) * teAmbientAdjust / 65536));
        teColorSchemes[iTeam].diffuse = colRGB(colClamp256(colRed(teColorSchemes[iTeam].textureColor.base) * teDiffuseAdjust / 65536),
            colClamp256(colGreen(teColorSchemes[iTeam].textureColor.base) * teDiffuseAdjust / 65536),
            colClamp256(colBlue(teColorSchemes[iTeam].textureColor.base) * teDiffuseAdjust / 65536));
        teColorSchemes[iTeam].specular = colRGB(colClamp256(colRed(teColorSchemes[iTeam].textureColor.base) * teSpecularAdjust / 65536),
            colClamp256(colGreen(teColorSchemes[iTeam].textureColor.base) * teSpecularAdjust / 65536),
            colClamp256(colBlue(teColorSchemes[iTeam].textureColor.base) * teSpecularAdjust / 65536));
        teColorSchemes[iTeam].stripeAmbient = colRGB(colClamp256(colRed(teColorSchemes[iTeam].textureColor.base) * teAmbientAdjust / 65536),
            colClamp256(colGreen(teColorSchemes[iTeam].textureColor.base) * teAmbientAdjust / 65536),
            colClamp256(colBlue(teColorSchemes[iTeam].textureColor.base) * teAmbientAdjust / 65536));
        teColorSchemes[iTeam].stripeDiffuse = colRGB(colClamp256(colRed(teColorSchemes[iTeam].textureColor.base) * teDiffuseAdjust / 65536),
            colClamp256(colGreen(teColorSchemes[iTeam].textureColor.base) * teDiffuseAdjust / 65536),
            colClamp256(colBlue(teColorSchemes[iTeam].textureColor.base) * teDiffuseAdjust / 65536));
        teColorSchemes[iTeam].stripeSpecular = colRGB(colClamp256(colRed(teColorSchemes[iTeam].textureColor.base) * teSpecularAdjust / 65536),
            colClamp256(colGreen(teColorSchemes[iTeam].textureColor.base) * teSpecularAdjust / 65536),
            colClamp256(colBlue(teColorSchemes[iTeam].textureColor.base) * teSpecularAdjust / 65536));
    }
}

