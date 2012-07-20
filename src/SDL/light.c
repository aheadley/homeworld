/*=============================================================================
    Name    : Light.c
    Purpose : Code for lighting and ship coloring

    Created 8/13/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "glinc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "Types.h"
#include "Debug.h"
#include "StatScript.h"
#include "Globals.h"
#include "Vector.h"
#include "light.h"
#include "File.h"
#include "Memory.h"
#include "MEX.h"

#include "Shader.h"

/*=============================================================================
    Data:
=============================================================================*/
//lighting properties
lightinfo lightDefaultLight;
//#if LIGHT_PLAYER_COLORS
lightinfo lightPlayerLight[MAX_MULTIPLAYER_PLAYERS];
//#endif
hvector lightPosition = {0.0f, 0.0f, 0.0f, 0.0f};

udword lightNumLights = 1;

void lightSetNumLights(udword numLights)
{
    lightNumLights = numLights;
}

struct
{
    char *name;
    lightinfo *light;
}
lightNameToInfoTable[] =
{
    {"default",         &lightDefaultLight},
//#if LIGHT_PLAYER_COLORS
    {"player0",         &lightPlayerLight[0]},
    {"player1",         &lightPlayerLight[1]},
    {"player2",         &lightPlayerLight[2]},
    {"player3",         &lightPlayerLight[3]},
    {"player4",         &lightPlayerLight[4]},
    {"player5",         &lightPlayerLight[5]},
    {"player6",         &lightPlayerLight[6]},
    {"player7",         &lightPlayerLight[7]},
//#endif //LIGHT_PLAYER_COLORS
    {NULL, NULL}
};

static lightinfo *currentLight;
static lightinfo currentLight1;
static GLfloat lightAmbient[4] = {0.2f, 0.2f, 0.2f, 1.0f};

static void lightPositionRead(char *directory,char *field,void *dataToFillIn);
static void lightTypeSet(char *directory,char *field,void *dataToFillIn);
static void lightPropertiesRead(char *directory,char *field,void *dataToFillIn);
static scriptEntry lightScriptTable[] =
{
    { "lightType",          lightTypeSet,  NULL },
    { "ambient",            lightPropertiesRead,  &lightDefaultLight.ambient },
    { "diffuse",            lightPropertiesRead,  &lightDefaultLight.diffuse },
    { "specular",           lightPropertiesRead,  &lightDefaultLight.specular },
    { "position",           lightPositionRead,  NULL },
    { NULL,NULL, NULL}
};

char lightCurrentLighting[PATH_MAX];

/*=============================================================================
    Private script-parsing functions:
=============================================================================*/
static void lightPositionRead(char *directory,char *field,void *dataToFillIn)
{
    sdword nScanned;
    nScanned = sscanf(field, "%f,%f,%f,%f", &lightPosition.x, &lightPosition.y, &lightPosition.z, &lightPosition.w);
    memcpy(&lightDefaultLight.position, &lightPosition, 4*sizeof(real32));
    dbgAssert(nScanned == 4);
}

static void lightTypeSet(char *directory,char *field,void *dataToFillIn)
{
    sdword index;

    for (index = 0; lightNameToInfoTable[index].name != NULL; index++)
    {
        if (strcmp(lightNameToInfoTable[index].name, field) == 0)
        {                                                   //if names match
            currentLight = lightNameToInfoTable[index].light;
            return;
        }
    }
#if LIGHT_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "light type %s not known.", field);
#endif
}
static void lightPropertiesRead(char *directory,char *field,void *dataToFillIn)
{
    sdword nScanned;
    real32 red, green, blue;
    real32 *properties;

    dbgAssert(currentLight != NULL);
    nScanned = sscanf(field, "%f,%f,%f", &red, &green, &blue);
    dbgAssert(nScanned == 3);
    dbgAssert(red >= 0.0f && red <= 1.0f);
    dbgAssert(green >= 0.0f && green <= 1.0f);
    dbgAssert(blue >= 0.0f && blue <= 1.0f);
    properties = (real32 *)((ubyte *)currentLight + ((ubyte *)dataToFillIn - (ubyte *)(&lightDefaultLight)));
    properties[0] = red;
    properties[1] = green;
    properties[2] = blue;
    properties[3] = 1.0f;
}

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : lightStartup
    Description : Initialize lighting module
    Inputs      :
    Outputs     : Reads in the lighting script
    Return      :
----------------------------------------------------------------------------*/
void lightStartup(void)
{
    currentLight = NULL;
    scriptSet(NULL, "Light.script", lightScriptTable); //read in the script
    currentLight = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : lightShutdown
    Description : Shut down light module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void lightShutdown(void)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : lightPositionSet
    Description : Position the current light
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void lightPositionSet(void)
{
    real32 m[16];

    glGetFloatv(GL_MODELVIEW_MATRIX, m);

//    glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat*)(&lightPosition));//position light(s) within world
//    glLightfv(GL_LIGHT1, GL_POSITION, (GLfloat*)(&lightPosition));
    if (currentLight != NULL)
    {
        glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat*)(&currentLight->position));
        shSetLightPosition(0, (real32*)&currentLight->position, m);
    }
    if (lightNumLights == 2)
    {
        glLightfv(GL_LIGHT1, GL_POSITION, (GLfloat*)(&currentLight1.position));
        shSetLightPosition(1, (real32*)&currentLight1.position, m);
    }

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightAmbient);
    shSetGlobalAmbient(lightAmbient); //shader
}

/*-----------------------------------------------------------------------------
    Name        : lightDefaultLightSet
    Description : Sets rendering to use default light properties
    Inputs      :
    Outputs     : calls glLightfv() with parameters from lightDefaultLight
    Return      :
----------------------------------------------------------------------------*/
void lightDefaultLightSet(void)
{
#if 1
    GLfloat amb[4] = {0.2f, 0.2f, 0.2f, 1.0f};

    if (currentLight == &lightDefaultLight)
    {                                                       //if default light already set
        lightSetNumLights(1);
        return;
    }
    currentLight = &lightDefaultLight;                      //retain current light set
    glLightfv( GL_LIGHT0, GL_AMBIENT, lightDefaultLight.ambient);
    glLightfv( GL_LIGHT0, GL_DIFFUSE, lightDefaultLight.diffuse);
    glLightfv( GL_LIGHT0, GL_SPECULAR, lightDefaultLight.specular);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
    shSetLighting(0, lightDefaultLight.diffuse, lightDefaultLight.ambient); //shader
    shSetGlobalAmbient(amb); //shader
//    glLightfv( GL_LIGHT0, GL_POSITION, lightDefaultLight.position);
//    glLightfv( GL_LIGHT0, GL_POSITION, (GLfloat*)(&lightPosition));

    lightSetNumLights(1);
    strcpy(lightCurrentLighting, "hsf\\default.hsf");
#else
    lightParseHSF("hsf\\default.hsf");
#endif
}

/*-----------------------------------------------------------------------------
    Name        : lightPlayerColorLightSet
    Description : Sets rendering to use a colored light specific to a player
    Inputs      : playerIndex - index of player who's colors to use
    Outputs     : calls glLightfv() with parameters from lightPlayerLight
    Return      :
----------------------------------------------------------------------------*/
#if LIGHT_PLAYER_COLORS
void lightPlayerColorLightSet(sdword playerIndex)
{
    dbgAssert(playerIndex >= 0 && playerIndex < MAX_MULTIPLAYER_PLAYERS);
    if (currentLight == &lightPlayerLight[playerIndex])
    {
        return;
    }
    currentLight = &lightPlayerLight[playerIndex];
    glLightfv( GL_LIGHT0, GL_AMBIENT, currentLight->ambient);
    glLightfv( GL_LIGHT0, GL_DIFFUSE, currentLight->diffuse);
    glLightfv( GL_LIGHT0, GL_SPECULAR, currentLight->specular);
    shSetLighting(0, currentLight->diffuse, currentLight->ambient); //shader
}
#endif

real32 fcos(real32 r)
{
    return (real32)cos((real64)r);
}

real32 fsin(real32 r)
{
    return (real32)sin((real64)r);
}

void _inplace_glify(GLfloat* v)
{
    hmatrix xmat;
    hvector rvec;
    real32 deg = -90.0f;
    real32 deg2 = -90.0f;

    hmatMakeRotAboutZ(&xmat, fcos(DEG_TO_RAD(deg)), fsin(DEG_TO_RAD(deg)));
    hmatMultiplyHVecByHMat(&rvec, (hvector*)v, &xmat);
    if (v[3] != 0.0f)
    {
        v[0] /= v[3];
        v[1] /= v[3];
        v[2] /= v[3];
    }
    memcpy(v, &rvec, 3*sizeof(real32));

    hmatMakeRotAboutX(&xmat, fcos(DEG_TO_RAD(deg2)), fsin(DEG_TO_RAD(deg2)));
    hmatMultiplyHVecByHMat(&rvec, (hvector*)v, &xmat);
    if (v[3] != 0.0f)
    {
        v[0] /= v[3];
        v[1] /= v[3];
        v[2] /= v[3];
    }
    memcpy(v, &rvec, 3*sizeof(real32));
}

/*-----------------------------------------------------------------------------
    Name        : lightParseHSF
    Description : parse a homeworld scene file for lighting info
    Inputs      : fileName - the name of the .hsf file, no extension or directory
    Outputs     : alters global variables
    Return      :
----------------------------------------------------------------------------*/
void lightParseHSF(char* fileName)
{
    udword numLights = 1;
    static char lastFileName[128];

    if (fileName == NULL)
    {
        fileName = lastFileName;
    }
    else
    {
        memStrncpy(lastFileName, fileName, 127);
    }

    lightDefaultLightSet();

    if (fileExists(fileName, 0))
    {
        HSFFileHeader*  hsfHeader;
        HSFLight*       hsfLight;
        ubyte* buf;
        real32 ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
        sdword pos = sizeof(HSFFileHeader);
        sdword fileSize = fileSizeGet(fileName, 0);

        buf = memAlloc(fileSize, "lightParseHSF", 0);
        fileLoad(fileName, buf, 0);

        hsfHeader = (HSFFileHeader*)buf;

#ifdef ENDIAN_BIG
		hsfHeader->version = LittleLong( hsfHeader->version );
		hsfHeader->nLights = LittleLong( hsfHeader->nLights );
#endif

        numLights = 0;
        while (pos <= (sdword)(fileSize - sizeof(HSFLight)))
        {
            lightinfo* linfo;

            hsfLight = (HSFLight*)(buf + pos);

#ifdef ENDIAN_BIG
			hsfLight->type      = LittleLong( hsfLight->type );
			hsfLight->x         = LittleFloat( hsfLight->x );
			hsfLight->y         = LittleFloat( hsfLight->y );
			hsfLight->z         = LittleFloat( hsfLight->z );
			hsfLight->h         = LittleFloat( hsfLight->h );
			hsfLight->p         = LittleFloat( hsfLight->p );
			hsfLight->b         = LittleFloat( hsfLight->b );
			hsfLight->coneAngle = LittleFloat( hsfLight->coneAngle );
			hsfLight->edgeAngle = LittleFloat( hsfLight->edgeAngle );
			hsfLight->intensity = LittleFloat( hsfLight->intensity );
#endif

            if (hsfLight->type != L_AmbientLight)
            {
                numLights++;
                linfo = (numLights == 2) ? &currentLight1 : currentLight;

                //"L_DistantLight"
                _inplace_glify((GLfloat*)&hsfLight->x);
                linfo->position[0] = hsfLight->x;
                linfo->position[1] = hsfLight->y;
                linfo->position[2] = hsfLight->z;
                linfo->position[3] = 0.0f;

/*                if (hsfLight->intensity > 1.0f)
                {
                    hsfLight->intensity = 1.0f;
                }*/

                linfo->ambient[0] = 0.3f * hsfLight->intensity * (real32)hsfLight->red / 255.0f;
                linfo->ambient[1] = 0.3f * hsfLight->intensity * (real32)hsfLight->green / 255.0f;
                linfo->ambient[2] = 0.3f * hsfLight->intensity * (real32)hsfLight->blue / 255.0f;
                linfo->ambient[3] = 1.0f;

                linfo->diffuse[0] = hsfLight->intensity * (real32)hsfLight->red / 255.0f;
                linfo->diffuse[1] = hsfLight->intensity * (real32)hsfLight->green / 255.0f;
                linfo->diffuse[2] = hsfLight->intensity * (real32)hsfLight->blue / 255.0f;
                linfo->diffuse[3] = 1.0f;

                linfo->specular[0] = 1.0f;
                linfo->specular[1] = 1.0f;
                linfo->specular[2] = 1.0f;
                linfo->specular[3] = 1.0f;
            }
            else
            {
                sdword i;

                //L_AmbientLight
                ambient[0] = hsfLight->intensity * (real32)hsfLight->red / 255.0f;
                ambient[1] = hsfLight->intensity * (real32)hsfLight->green / 255.0f;
                ambient[2] = hsfLight->intensity * (real32)hsfLight->blue / 255.0f;
                ambient[3] = 1.0f;

                for (i = 0; i < 4; i++)
                {
                    real32 lv = 2.0f * ambient[i];
                    if (lv > 1.0f) lv = 1.0f;
                    lightAmbient[i] = lv;
                }
            }

            pos += sizeof(HSFLight);
        }

        memFree(buf);
        strcpy(lightCurrentLighting, fileName);
    }
//    else
//    {
//        lightDefaultLightSet();
//    }

    lightSetNumLights(numLights);
}

void lightSetLighting()
{
    if (currentLight == NULL)
    {
//        lightDefaultLightSet();
        lightParseHSF("hsf\\default.hsf");
    }

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, currentLight->ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, currentLight->diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, currentLight->specular);
    shSetLighting(0, currentLight->diffuse, currentLight->ambient); //shader

    if (lightNumLights == 2)
    {
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_AMBIENT, currentLight1.ambient);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, currentLight1.diffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR, currentLight1.specular);
        shSetLighting(1, currentLight1.diffuse, currentLight1.ambient); //shader
    }
    else
    {
        glDisable(GL_LIGHT1);
    }
}

/*-----------------------------------------------------------------------------
    Name        : lightGetPosition
    Description : returns the position element of the queried light
    Inputs      : lightIndex - [0..1], the light to query
                  position - where the position goes
    Outputs     : position is modified
    Return      :
----------------------------------------------------------------------------*/
void lightGetPosition(sdword lightIndex, hvector* position)
{
    lightinfo* linfo;

    linfo = (lightIndex == 0) ? currentLight : &currentLight1;
    if (linfo == NULL)
    {
        position->x = 0.0f;
        position->y = 0.0f;
        position->z = 0.0f;
        position->w = 0.0f;
        return;
    }
    position->x = linfo->position[0];
    position->y = linfo->position[1];
    position->z = linfo->position[2];
    position->w = linfo->position[3];
}
