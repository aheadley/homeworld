/*=============================================================================
    Name    : Teams.h
    Purpose : Maintains certain team-specific data, such as player's chosen
                colors and badges.

    Created 9/30/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___TEAMS_H
#define ___TEAMS_H

#include "Types.h"
#include "color.h"
#include "texreg.h"
#include "Globals.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define TO_STANDARD_COLORS          1       //use a set of 'standard' colors rather than the team colors of the ships

/*=============================================================================
    Definitions:
=============================================================================*/
#define TE_NumberTrailColors        4
#define TE_NumberPlayers            MAX_MULTIPLAYER_PLAYERS
#define TE_ColorScript              "TeamColors.script"
#define TE_SpecularAdjust           80000
#define TE_DiffuseAdjust            36000
#define TE_AmbientAdjust            10000

#define TE_DerelictDefaultColorScheme 7

/*=============================================================================
    Type definitions:
=============================================================================*/
//information for coloring a team's textures
/*
typedef struct
{
    color base;
    color detail;
}
trcolorinfo;
*/

//structure for HLS color adjustment of a color
typedef struct
{
    real32 hue;
    real32 lum;
    real32 sat;
}
trhlscolorize;

//structure for a team color scheme there can be an arbitrary number of these
typedef struct
{
    trcolorinfo textureColor;                   //info on coloring the textures on a team
#if !TO_STANDARD_COLORS
    color tacticalColor;                        //color for a team's tactical overlay
#endif
    color ambient;                              //how to color team-color polygons
    color diffuse;
    color specular;
    color stripeAmbient;
    color stripeDiffuse;
    color stripeSpecular;
    color trailColors[TE_NumberTrailColors];    //colors of trails at key points
}
tecolorscheme;

/*=============================================================================
    Data:
=============================================================================*/
extern tecolorscheme teColorSchemes[TE_NumberPlayers];
extern trcolorinfo teRace1Default, teRace2Default;
#if TO_STANDARD_COLORS
extern color teFriendlyColor;
extern color teAlliedColor;
extern color teHostileColor;
extern color teNeutralColor;
extern color teCrateColor;
extern color teResourceColor;
#endif

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown module
void teStartup(void);
void teReset(void);
void teShutdown(void);

//recompute team colors
void teTeamColorsSet(sdword iTeam, color baseColor, color stripeColor);

//parsing callbacks
void teTrailColorSet(char *directory,char *field,void *dataToFillIn);
void teColorSet(char *directory,char *field,void *dataToFillIn);
void teColorFactorsSet(char *directory,char *field,void *dataToFillIn);

#endif //___TEAMS_H

