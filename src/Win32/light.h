/*=============================================================================
    Name    : Light.h
    Purpose : Definitions for manipulating lights

    Created 8/13/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___LIGHT_H
#define ___LIGHT_H

#include "types.h"

/*=============================================================================
    Switches
=============================================================================*/
#ifndef HW_Release

#define LIGHT_ERROR_CHECKING    1               //general error checking
#define LIGHT_VERBOSE_LEVEL     2               //print extra info
#define LIGHT_PLAYER_COLORS     0               //set colored lights for players

#else //HW_Debug

#define LIGHT_ERROR_CHECKING    0               //general error checking
#define LIGHT_VERBOSE_LEVEL     0               //print extra info
#define LIGHT_PLAYER_COLORS     0               //set colored lights for players

#endif //HW_Debug
/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
    udword type;            //from tagLightTypes enum
    real32 ambient[4];
    real32 diffuse[4];
    real32 specular[4];
    real32 position[4];
}
lightinfo;

enum tagLightTypes
{
    L_DistantLight,
    L_PointLight,
    L_SpotLight,
    L_LinearLight,
    L_AreaLight,
    L_AmbientLight
};

#define HSF_FILE_ID "rrasberry"

typedef struct
{
    char   identifier[12];
    udword version;
    udword nLights;
} HSFFileHeader;

typedef struct
{
    udword type;
    real32 x, y, z;
    real32 h, p, b;
    real32 coneAngle;
    real32 edgeAngle;
    ubyte  red, green, blue;
    real32 intensity;
} HSFLight;

/*=============================================================================
    Data:
=============================================================================*/
extern udword lightNumLights;
extern char lightCurrentLighting[];

/*=============================================================================
    Functions:
=============================================================================*/
//startup/shutdown
void lightStartup(void);
void lightShutdown(void);

//set the current lighting properties
void lightPositionSet(void);
void lightDefaultLightSet(void);

#if LIGHT_PLAYER_COLORS
void lightPlayerColorLightSet(sdword playerIndex);
#endif

//parse an HSF file full of lighting data, filling in the default lightinfo struct
void lightParseHSF(char*);
//gl up the lighting info (either from an hsf, or defaultlightset)
void lightSetLighting(void);

void lightSetNumLights(udword numLights);

void lightGetPosition(sdword lightIndex, hvector* position);

#endif //___LIGHT_H
