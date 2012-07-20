/*=============================================================================
    Name    : statscript.c
    Purpose : Static Scripting Utilities

    Created 6/24/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include "Types.h"
#include "ObjTypes.h"
#include "Debug.h"
#include "Memory.h"
#include "File.h"
#include "color.h"
#include "SpaceObj.h"
#include "Formation.h"
#include "StatScript.h"
#include "texreg.h"
#include "Tactics.h"
#include "MEX.h"
#include "MultiplayerGame.h"
#include "Crates.h"
#include "Mothership.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp strnicmp
#endif

//#define USE_SPHERE_TABLES

/*=============================================================================
    Externals for setting global data
=============================================================================*/
extern scriptEntry AIShipTweaks[];
extern scriptEntry AIStrategyTweaks[];
extern scriptEntry AITrackTweaks[];
extern scriptEntry CameraTweaks[];
extern scriptEntry CameraCommandTweaks[];
extern scriptEntry FormationTweaks[];
extern scriptEntry CommandLayerTweaks[];
extern scriptEntry MissphereTweaks[];
extern scriptEntry StarTweaks[];
extern scriptEntry Tweaks[];
extern scriptEntry FlightmanTweaks[];
extern scriptEntry DockTweaks[];
extern scriptEntry SoundeventTweaks[];
extern scriptEntry TrailTweaks[];
extern scriptEntry ResCollectTweaks[];
extern scriptEntry CloudTweaks[];
extern scriptEntry SinglePlayerTweaks[];
extern scriptEntry NebulaeTweaks[];
extern scriptEntry AIPlayerTweaks[];
extern scriptEntry NetTweaks[];
extern scriptEntry AIResourceManTweaks[];
extern scriptEntry ShipStaticCapLimits[];
extern scriptEntry DamageTweaks[];
extern scriptEntry ShipViewTweaks[];
extern scriptEntry AutoLODTweaks[];
extern scriptEntry GatherStatsScriptTable[];
extern scriptEntry FrontEndColourTweaks[];

char globalScriptFileName[50];  //file name of file loaded in a script callback function

/*=============================================================================
    Private Defines:
=============================================================================*/

#define MAX_LINE_CHARS 650

/*=============================================================================
    Private Function prototypes:
=============================================================================*/

void scriptSetDockPointCB(char *directory,char *field,void *dataToFillIn);
void scriptSetDockOverideCB(char *directory,char *field,void *dataToFillIn);
void scriptSetNAVLightCB(char *directory,char *field,void *dataToFillIn);
//static void setEffectPointer(char *directory,char *field,void *dataToFillIn);

/*=============================================================================
    Private Data:
=============================================================================*/

static GunStatic gunStaticTemplate;

static scriptStructEntry StaticGunInfoScriptTable[] =
{
    { "Type",        scriptSetGunTypeCB,(udword) &(gunStaticTemplate.guntype), (udword) &gunStaticTemplate },
    { "SoundType",   scriptSetGunSoundTypeCB, (udword) &(gunStaticTemplate.gunsoundtype), (udword) &gunStaticTemplate },
    { "BulletType",  scriptSetBulletTypeCB, (udword) &(gunStaticTemplate.bulletType), (udword) &gunStaticTemplate },
    { "DamageLo",    scriptSetReal32CB,  (udword) &(gunStaticTemplate.baseGunDamageLo), (udword) &gunStaticTemplate },
    { "DamageHi",    scriptSetReal32CB,  (udword) &(gunStaticTemplate.baseGunDamageHi), (udword) &gunStaticTemplate },
    { "MaxMissiles", scriptSetSdwordCB, (udword) &(gunStaticTemplate.maxMissiles), (udword) &gunStaticTemplate },
    { "BulletLifeTime",scriptSetReal32CB, (udword) &(gunStaticTemplate.bulletlifetime),(udword) &gunStaticTemplate },
    { "BulletLength",scriptSetReal32CB, (udword) &(gunStaticTemplate.bulletlength),(udword) &gunStaticTemplate },
    { "BulletRange" ,scriptSetReal32CB, (udword) &(gunStaticTemplate.bulletrange), (udword) &gunStaticTemplate },
    { "BulletSpeed" ,scriptSetReal32CB, (udword) &(gunStaticTemplate.bulletspeed), (udword) &gunStaticTemplate },
    { "BulletMass"  ,scriptSetReal32CB, (udword) &(gunStaticTemplate.bulletmass),  (udword) &gunStaticTemplate },
    { "FireTime",    scriptSetReal32CB, (udword) &(gunStaticTemplate.firetime),    (udword) &gunStaticTemplate },
    { "BurstFireTime",scriptSetReal32CB, (udword) &(gunStaticTemplate.burstFireTime),    (udword) &gunStaticTemplate },
    { "BurstWaitTime",scriptSetReal32CB, (udword) &(gunStaticTemplate.burstWaitTime),    (udword) &gunStaticTemplate },
    { "MinAngle",    scriptSetCosAngCB, (udword) &(gunStaticTemplate.cosminAngleFromNorm), (udword) &gunStaticTemplate },
    { "MaxAngle",    scriptSetCosAngCB, (udword) &(gunStaticTemplate.cosmaxAngleFromNorm), (udword) &gunStaticTemplate },
    { "TriggerHappy",scriptSetCosAngCB, (udword) &(gunStaticTemplate.triggerHappy), (udword) &gunStaticTemplate },

    { "minturnangle",scriptSetAngCB, (udword) &(gunStaticTemplate.minturnangle), (udword) &gunStaticTemplate },
    { "maxturnangle",scriptSetAngCB, (udword) &(gunStaticTemplate.maxturnangle), (udword) &gunStaticTemplate },
    { "mindeclination",scriptSetAngCB, (udword) &(gunStaticTemplate.mindeclination), (udword) &gunStaticTemplate },
    { "maxdeclination",scriptSetAngCB, (udword) &(gunStaticTemplate.maxdeclination), (udword) &gunStaticTemplate },
    { "maxanglespeed",scriptSetAngCB, (udword) &(gunStaticTemplate.maxanglespeed), (udword) &gunStaticTemplate },
    { "maxdeclinationspeed",scriptSetAngCB, (udword) &(gunStaticTemplate.maxdeclinationspeed), (udword) &gunStaticTemplate },
    { "angletracking",scriptSetReal32CB, (udword) &(gunStaticTemplate.angletracking), (udword) &gunStaticTemplate },
    { "declinationtracking",scriptSetReal32CB, (udword) &(gunStaticTemplate.declinationtracking), (udword) &gunStaticTemplate },
    { "BarrelLength",scriptSetReal32CB, (udword) &(gunStaticTemplate.barrelLength), (udword) &gunStaticTemplate },
    { "RecoilLength",scriptSetReal32CB, (udword) &(gunStaticTemplate.recoilLength), (udword) &gunStaticTemplate },
    { "OffsetX",    scriptSetReal32CB, (udword) &(gunStaticTemplate.offset.x), (udword) &gunStaticTemplate },
    { "OffsetY",    scriptSetReal32CB, (udword) &(gunStaticTemplate.offset.y), (udword) &gunStaticTemplate },
    { "OffsetZ",    scriptSetReal32CB, (udword) &(gunStaticTemplate.offset.z), (udword) &gunStaticTemplate },
    { "SlaveDriver",scriptSetSdwordCB, (udword) &(gunStaticTemplate.slaveDriver), (udword) &gunStaticTemplate },

    { NULL,NULL,0,0 }
};

static scriptStructEntry StaticDockInfoScriptTable[] =
{
    { "DockPoint",  scriptSetDockPointCB, 0, 0 },
    { NULL,NULL,0,0 }
};

static scriptStructEntry StaticDockOverideInfoScriptTable[] =
{
    { "DockOveride",  scriptSetDockOverideCB, 0, 0 },
    { NULL,NULL,0,0 }
};

static scriptStructEntry StaticSalvageInfoScriptTable[] =
{
    { "SalvagePoint",  scriptSetSalvagePointCB, 0, 0 },

    { NULL,NULL,0,0 }
};

static scriptStructEntry StaticNavLightInfoScriptTable[] =
{
    { "NavLight",  scriptSetNAVLightCB, 0, 0 },
    { NULL,NULL,0,0 }
};

/*=============================================================================
    Private Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : findStructEntry
    Description : looks through a scriptStructEntry table, and finds an entry
                  with the correct name
    Inputs      : pointer to table, name to look for in table
    Outputs     :
    Return      : pointer to table entry which it found
----------------------------------------------------------------------------*/
scriptStructEntry *findStructEntry(scriptStructEntry info[],char *name)
{
    scriptStructEntry *curentry = info;

    while (curentry->name != NULL)
    {
        if (strcmp(name,curentry->name) == 0)
        {
            return curentry;
        }
        curentry++;
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : findEntry
    Description : looks through a scriptEntry table, and finds an entry
                  with the correct name
    Inputs      : pointer to table, name to look for in table
    Outputs     :
    Return      : pointer to table entry which it found
----------------------------------------------------------------------------*/
scriptEntry *findEntry(scriptEntry info[],char *name)
{
    scriptEntry *curentry = info;

    while (curentry->name != NULL)
    {
        if (strcmp(name,curentry->name) == 0)
        {
            return curentry;
        }
        curentry++;
    }
    return NULL;
}

/*=============================================================================
    Functions:
=============================================================================*/

/*=============================================================================
    The following functions are generic callback functions to set *dataToFillIn
    with real32, sword, sdword numbers given a field string which contains
    the ascii representation of the number.
=============================================================================*/

void scriptSetRGBCB(char *directory,char *field,void *dataToFillIn)
{
    udword red, green, blue;
    sscanf(field,"%d,%d,%d",&red, &green, &blue);
    *((color *)dataToFillIn) = colRGB(red, green, blue);
}

void scriptSetRGBACB(char *directory,char *field,void *dataToFillIn)
{
    udword red, green, blue, alpha;
    sscanf(field,"%d,%d,%d,%d",&red, &green, &blue, &alpha);
    *((color *)dataToFillIn) = colRGBA(red, green, blue, alpha);
}

void scriptSetReal32CB(char *directory,char *field,void *dataToFillIn)
{
    sscanf(field,"%f",(real32 *)dataToFillIn);
}

#ifndef HW_Release
void CheckValidTacticsClass(TacticsType tactic,ShipClass shipclass,char *field)
{
    if ((tactic < 0) || (tactic >= NUM_TACTICS_TYPES))
    {
        dbgFatalf(DBG_Loc,"Invalid tactics type in %s",field);
    }

    if ((shipclass < 0) || (shipclass > NUM_CLASSES))
    {
        dbgFatalf(DBG_Loc,"Invalid class in %s",field);
    }
}
#endif

void scriptSetReal32CB_ARRAY(char *directory, char *field, void *dataToFillIn)
{
    char tactic_buffer[32];
    char class_buffer[64];
    real32 value;
    TacticsType tactic;
    ShipClass shipclass;

    sscanf(field,"%s %s %f", tactic_buffer, class_buffer, &value);

    tactic = StrToTacticsType(tactic_buffer);
    if(strcmp(class_buffer,"Default") == 0)
        shipclass = NUM_CLASSES;
    else
        shipclass = StrToShipClass(class_buffer);

#ifndef HW_Release
    CheckValidTacticsClass(tactic,shipclass,field);
#endif

    (real32*)dataToFillIn += (tactic * (NUM_CLASSES + 1)) + shipclass;
    *(real32*)dataToFillIn = value;
}
void scriptSetShipProbCB(char *directory, char *field, real32 *dataToFillIn)
{
    char shipname[64];
    real32 prob;
    ShipType shiptype;

    sscanf(field,"%s %f", shipname, &prob);

    shiptype = StrToShipType(shipname);

    dataToFillIn += shiptype;
    *dataToFillIn = prob;
}

void scriptSetHyperspaceCostCB(char *directory, char *field, real32 *dataToFillIn)
{
    char shipname[200];
    real32 mincost,slopecost,maxcost,mindistance;
    ShipType shiptype;
    sdword offset;
    HW_Cost *costData = (HW_Cost *)dataToFillIn;

    sscanf(field,"%s %f %f %f %f", shipname, &mincost,&mindistance, &slopecost,&maxcost);

    shiptype = StrToShipType(shipname);
    offset = shiptype;
    costData = costData + offset;
    costData->min = mincost;
    costData->distanceSlope = slopecost;
    costData->max = maxcost;
    costData->minDistance = mindistance;
    costData->canMpHyperspace = TRUE;
}

void scriptSetSpecialDoorOffsetCB(char *directory, char *field, real32 *dataToFillIn)
{
    char shipname[200];
    char racename[50];
    ShipType shiptype;
    ShipRace race;
    real32 offsetX,offsetY,offsetZ;
    sdword headingSdword, upSdword;
    MothershipStatics *mstatic = (MothershipStatics *)dataToFillIn;

    sscanf(field,"%s %s %f %f %f %d %d", racename, shipname, &offsetX, &offsetY, &offsetZ, &headingSdword, &upSdword);

    shiptype = StrToShipType(shipname);
    race = StrToShipRace(racename);

    //from monkeyvector callback
    //sscanf(field,"%f,%f,%f",&newvec.z, &newvec.x, &newvec.y);
    //newvec.z = -newvec.z;
    //*((vector *)dataToFillIn) = newvec;

    mstatic->specialDoorOffset[race][shiptype].z = -offsetX;
    mstatic->specialDoorOffset[race][shiptype].x = offsetY;
    mstatic->specialDoorOffset[race][shiptype].y = offsetZ;
    mstatic->specialDoorOrientationHeading[race][shiptype] = headingSdword;
    mstatic->specialDoorOrientationUp[race][shiptype] = upSdword;


}


void scriptSetShipGroupSizeCB(char *directory, char *field, sdword *dataToFillIn)
{
    char shipname[64];
    sdword numingroup;
    ShipType shiptype;

    sscanf(field,"%s %d", shipname, &numingroup);

    shiptype = StrToShipType(shipname);

    dataToFillIn += shiptype;
    *dataToFillIn = numingroup;
}


void scriptSetReal32SqrCB(char *directory,char *field,void *dataToFillIn)
{
    sscanf(field,"%f",(real32 *)dataToFillIn);
    *((real32 *)dataToFillIn) *= *((real32 *)dataToFillIn);
}

void scriptSetSbyteCB(char *directory,char *field,void *dataToFillIn)
{
    sdword readval;

    sscanf(field,"%d",&readval);

    *((sbyte *)dataToFillIn) = (sbyte)readval;
}

void scriptSetUbyteCB(char *directory,char *field,void *dataToFillIn)
{
    udword readval;

    sscanf(field,"%d",&readval);

    *((ubyte *)dataToFillIn) = (ubyte)readval;
}

void scriptSetSwordCB(char *directory,char *field,void *dataToFillIn)
{
    sdword readval;

    sscanf(field,"%d",&readval);

    *((sword *)dataToFillIn) = (sword)readval;
}

void scriptSetUwordCB(char *directory,char *field,void *dataToFillIn)
{
    udword readval;

    sscanf(field,"%d",&readval);

    *((uword *)dataToFillIn) = (uword)readval;
}

void scriptSetSdwordCB(char *directory,char *field,void *dataToFillIn)
{
    sscanf(field,"%d",(sdword *)dataToFillIn);
}

void scriptSetUdwordCB(char *directory,char *field,void *dataToFillIn)
{
    sscanf(field,"%d",(udword *)dataToFillIn);
}

/*-----------------------------------------------------------------------------
    Name        : scriptStringToBool
    Description : Convert a string to a bool.  Looks for nonzero numbers, "TRUE" or "YES"
    Inputs      : boolString - string to convert
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool scriptStringToBool(char *boolString)
{
    while (*boolString && strchr(" \t,\n", *boolString))
    {                                                       //strip the leading fluff characters
        boolString++;
    }
    if (boolString[0] >= 1 && boolString[0] <= 9)
    {
        return(TRUE);
    }
    if ((strcasecmp(boolString, "TRUE") == 0) ||
        (strcasecmp(boolString, "YES") == 0))
    {
        return(TRUE);
    }
    return(FALSE);
}

void scriptSetBool8(char *directory,char *field,void *dataToFillIn)
{
    *((bool8 *)dataToFillIn) = scriptStringToBool(field);
}

void scriptSetBool(char *directory,char *field,void *dataToFillIn)
{
    *((bool *)dataToFillIn) = scriptStringToBool(field);
}

void scriptSetBitUdword(char *directoryy,char *field,void *dataToFillIn)
{
    udword readval;

    if (strncasecmp(field, "BIT",3) == 0)
    {
        sscanf(field+3,"%d", &readval);

        *((udword *)dataToFillIn) |= (1 << readval);
    }
}

void scriptSetBitUword(char *directory,char *field,void *dataToFillIn)
{
    udword readval;

    if (strncasecmp(field, "BIT",3) == 0)
    {
        sscanf(field+3,"%d", &readval);

        *((uword *)dataToFillIn) |= (uword)(1 << readval);
    }
}

void scriptSetStringCB(char *directory,char *field,void *dataToFillIn)
{
    strcpy((char *)dataToFillIn,field);
}

void scriptSetStringPtrCB(char *directory,char *field,void *dataToFillIn)
{
    *((char **)dataToFillIn) = memStringDupeNV(field);
}

void scriptSetCosAngCB(char *directory,char *field,void *dataToFillIn)
{
    real32 ang;

    sscanf(field,"%f",&ang);
    *((real32 *)dataToFillIn) = (real32)cos(DEG_TO_RAD(ang));
}

void scriptSetCosAngSqrCB(char *directory,char *field,void *dataToFillIn)
{
    real32 ang;
    real32 cosine;

    sscanf(field,"%f",&ang);
    cosine = (real32)cos(DEG_TO_RAD(ang));
    *((real32 *)dataToFillIn) = cosine*cosine;
}

void scriptSetCosAngCB_ARRAY(char *directory, char *field, void *dataToFillIn)
{
    char tactic_buffer[32];
    char class_buffer[64];
    real32 value;
    TacticsType tactic;
    ShipClass shipclass;

    sscanf(field,"%s %s %f", tactic_buffer, class_buffer, &value);

    tactic = StrToTacticsType(tactic_buffer);
    if(strcmp(class_buffer,"Default") == 0)
        shipclass = NUM_CLASSES;
    else
        shipclass = StrToShipClass(class_buffer);

#ifndef HW_Release
    CheckValidTacticsClass(tactic,shipclass,field);
#endif

    (real32*)dataToFillIn += (tactic * (NUM_CLASSES + 1)) + shipclass;
    *(real32*)dataToFillIn = (real32)cos(DEG_TO_RAD(value));
}

void scriptSetSinAngCB(char *directory,char *field,void *dataToFillIn)
{
    real32 ang;

    sscanf(field,"%f",&ang);
    *((real32 *)dataToFillIn) = (real32)sin(DEG_TO_RAD(ang));
}

void scriptSetTanAngCB(char *directory,char *field,void *dataToFillIn)
{
    real32 ang;

    sscanf(field,"%f",&ang);
    *((real32 *)dataToFillIn) = (real32)tan(DEG_TO_RAD(ang));
}

void scriptSetAngCB(char *directory,char *field,void *dataToFillIn)
{
    real32 ang;

    sscanf(field,"%f",&ang);
    *((real32 *)dataToFillIn) = DEG_TO_RAD(ang);
}

void scriptSetGunTypeCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipClass *)dataToFillIn) = StrToGunType(field);
}

void scriptSetGunSoundTypeCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipClass *)dataToFillIn) = StrToGunSoundType(field);
}

void scriptSetBulletTypeCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipClass *)dataToFillIn) = StrToBulletType(field);
}

void scriptSetShipTypeCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipClass *)dataToFillIn) = StrToShipType(field);
}

void scriptSetShipRaceCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipClass *)dataToFillIn) = StrToShipRace(field);
}

void scriptSetShipClassCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipClass *)dataToFillIn) = StrToShipClass(field);
}

void scriptSetVectorCB(char *directory,char *field,void *dataToFillIn)
{
    vector newvec;
    sscanf(field,"%f,%f,%f",&newvec.x, &newvec.y, &newvec.z);
    *((vector *)dataToFillIn) = newvec;
}
void scriptSetLWToHWMonkeyVectorCB(char *directory,char *field,void *dataToFillIn)
{
    vector newvec;
    sscanf(field,"%f,%f,%f",&newvec.z, &newvec.x, &newvec.y);
    newvec.z = -newvec.z;
    *((vector *)dataToFillIn) = newvec;
}

void scriptSetFormationCB(char *directory,char *field,void *dataToFillIn)
{
    *((TypeOfFormation *)dataToFillIn) = StrToTypeOfFormation(field);
}

void scriptSetTacticsCB(char *directory,char *field,void *dataToFillIn)
{
    *((TacticsType *)dataToFillIn) = StrToTacticsType(field);
}


#ifdef USE_SPHERE_TABLES
void scriptSetSphereDeclinationCB(char *directory,char *field,void *dataToFillIn)
{
    SphereDeclination *dec = (SphereDeclination *)dataToFillIn;
    real32 ang;
    real32 radang;
    sdword numPoints;

    RemoveCommasFromString(field);

    sscanf(field,"%f %d",&ang,&numPoints);

    radang = DEG_TO_RAD(ang);

    dec->numAnglePoints = numPoints;
    dec->cosdeclination = (real32)cos(radang);
    dec->sindeclination = (real32)sin(radang);

    radang = TWOPI / (real32)numPoints;
    dec->cosrotang = (real32)cos(radang);
    dec->sinrotang = (real32)sin(radang);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : RemoveCommasFromString
    Description : removes all commas from string
    Inputs      : field
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveCommasFromString(char *field)
{
    char *fieldptr = field;

    while (*fieldptr != 0)
    {
        if (*fieldptr == ',')
        {
            *fieldptr = ' ';
        }
        else if (*fieldptr == '?')
        {
            //dbgMessagef("\nWarning: Unknown field in %s",field);
            *fieldptr = '0';
        }
        fieldptr++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : StripTrailingSpaces
    Description : strips trailing spaces off string value
    Inputs      : value
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void StripTrailingSpaces(char *value)
{
    char *endstr = value;

    while (*endstr != 0)
    {
        endstr++;
    }

    for (;;)
    {
        endstr--;

        if (endstr <= value)
        {
            break;
        }

        if ((*endstr == ' ') || (*endstr == '\t') || (*endstr == 10) || (*endstr == 13))
        {
            *endstr = 0;
        }
        else
        {
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : parseLine
    Description : parses a line for the name and value, and if it finds them puts these pointers
                  into returnName and returnValue and returns TRUE.  If name and value are not found
                  then it returns FALSE.
    Inputs      :
    Outputs     : returnName, returnValue
    Return      : TRUE if returnName, returnValue were found and set, FALSE otherwise
----------------------------------------------------------------------------*/
bool parseLine(char *line,char **returnName,char **returnValue)
{
    char *name;
    char *value;
    sdword length;

    if ((line[0] == '[') || (line[0] == ']') || (line[0] == 0))
    {
        return FALSE;
    }

    do
    {
        length = strlen(line);
        if (line[length - 1] == ' ' || line[length - 1] == '\t')
        {
            line[length - 1] = 0;
        }
        else
            break;
    }
    while (length > 0);

    if (strchr(line, ';') != NULL)
    {                                                   //scan for comments
        *strchr(line, ';') = 0;
    }
    if (strstr(line, "//") != NULL)
    {
        *strstr(line, "//") = 0;
    }
    for (name = line; *name; name++)
    {                                                   //find leading non-whitespace
        if (*name != ' ' && *name != '\t')
        {
            break;
        }
    }
    if (*name == 0)
    {
        return FALSE;
    }
    for (value = name; *value; value++)
    {                                                   //find blank between name and value
        if (*value == ' ' || *value == '\t' || *value == '=')
        {
            *value = 0;                                 //null-terminate name string
            value++;                                    //next blank character
            for (; *value; value++)
            {                                           //find first character in value
                if (*value != ' ' && *value != '\t')
                {
                    break;
                }
            }
            break;
        }
    }

    StripTrailingSpaces(value);

#if 0                   // allow NULL value to be returned
    if (*value == 0)
    {
        return FALSE;
    }
#endif

    *returnName = name;
    *returnValue = value;

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : scriptSetStruct
    Description : This generalized routine, you give a filename of a script file
                  which contains data to be put into a structure, a pointer to that
                  structure, and a pointer to a table which contains information
                  about the offsets into that structure and their symbolic name.
                  Then, this routine will parse the script file filename and
                  fill in the structureToFillIn correctly.
    Inputs      : directory - directory where script file resides
                  filename - name of script file to parse
                  info - array of scriptStructEntry, where each member of the array
                         contains such information as the field name, the offset
                         into the structure, and a callback routine which fills in
                         the field of the structure appropriately.
                  structureToFillIn - structure which will be filled in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scriptSetStruct(char *directory,char *filename,scriptStructEntry info[],ubyte *structureToFillIn)
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[80];
    sdword status;
    scriptStructEntry *foundentry;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fileOpen(fullfilename,FF_TextMode);

    for (;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            foundentry = findStructEntry(info,name);
            if (foundentry != NULL)
            {
                strcpy(globalScriptFileName,filename);
                foundentry->setVarCB(directory,value,structureToFillIn + (foundentry->offset1 - foundentry->offset2) );
            }
        }
    }

    fileClose(fh);
}

/*-----------------------------------------------------------------------------
    Name        : scriptSet
    Description : This generalized routine, you give a filename of a script file
                  which contains data to be set,
                  and a pointer to a table which contains information
                  about the offsets of the data and their symbolic name.
                  Then, this routine will parse the script file filename and
                  fill in the data correctly.
    Inputs      : directory - directory where script file resides
                  filename - name of script file to parse
                  info - array of scriptStructEntry, where each member of the array
                         contains such information as the field name, the offset
                         into the structure, and a callback routine which fills in
                         the field of the structure appropriately.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scriptSet(char *directory,char *filename,scriptEntry info[])
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[80];
    sdword status;
    scriptEntry *foundentry;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fileOpen(fullfilename,FF_TextMode);

    for (;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            foundentry = findEntry(info,name);
            if (foundentry != NULL)
            {
                strcpy(globalScriptFileName,filename);
                foundentry->setVarCB(directory,value,foundentry->dataPtr);
            }
        }
    }

    fileClose(fh);
}

/*-----------------------------------------------------------------------------
    Name        : scriptSetFileSystem
    Description : Just like above, except that it loads the file from the file
                    system, never the .BIG file or data directory.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scriptSetFileSystem(char *directory,char *filename,scriptEntry info[])
{
    FILE *fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[PATH_MAX];
    scriptEntry *foundentry;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fopen(fullfilename,"rt");
    if (fh == NULL)
    {
        return;
    }

    while (fgets(line, MAX_LINE_CHARS, fh) != NULL)
    {
        if (parseLine(line,&name,&value))
        {
            foundentry = findEntry(info,name);
            if (foundentry != NULL)
            {
                strcpy(globalScriptFileName,filename);
                foundentry->setVarCB(directory,value,foundentry->dataPtr);
            }
        }

    }

    fclose(fh);
}

#define SETGUNSTATE_START           0
#define SETGUNSTATE_LOOKINGFORGUN   1
#define SETGUNSTATE_LEFTBRACKET     2
#define SETGUNSTATE_GETGUNINFO      3

extern real32 ASTEROID_HARVEST_RANGE;

/*-----------------------------------------------------------------------------
    Name        : scriptSetGunStatics
    Description : allocates and sets the gunStaticInfo field of shipstatinfo
    Inputs      : directory, filename, shipstatinfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scriptSetGunStatics(char *directory,char *filename,struct ShipStaticInfo *shipstatinfo)
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[80];
    sdword status;
    scriptStructEntry *foundentry;
    sdword numGuns=0;
    sdword state = 0;
    sdword numGunsProcessed = 0;
    sdword processingGun = -1;
    sdword sizeofgunstaticinfo;
    GunStaticInfo *gunstaticinfo = NULL;
    ubyte *structureToFillIn;
    sdword i,k;
    real32 curbulletrange,bonus,spdbonus;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fileOpen(fullfilename,FF_TextMode);

    for (;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            switch (state)
            {
                case SETGUNSTATE_START:
                    if (strcmp(name,"NUMBER_OF_GUNS") == 0)
                    {
                        scriptSetSdwordCB(directory,value,&numGuns);
                        if (numGuns <= 0)
                        {
                            goto done;
                        }

                        sizeofgunstaticinfo = sizeofGunStaticInfo(numGuns);

                        gunstaticinfo = memAlloc(sizeofgunstaticinfo,"gunstaticinfo",NonVolatile);
                        memset(gunstaticinfo,0,sizeofgunstaticinfo);
                        gunstaticinfo->numGuns = numGuns;

                        shipstatinfo->gunStaticInfo = gunstaticinfo;

                        state = SETGUNSTATE_LOOKINGFORGUN;
                    }
                    break;

                case SETGUNSTATE_LOOKINGFORGUN:
                    if (strcmp(name,"GUN") == 0)
                    {
                        scriptSetSdwordCB(directory,value,&processingGun);
                        dbgAssert(processingGun >= 0);
                        dbgAssert(processingGun < numGuns);
                        gunstaticinfo->gunstatics[processingGun].slaveDriver = -1;
                        state = SETGUNSTATE_LEFTBRACKET;
                    }
                    break;

                case SETGUNSTATE_LEFTBRACKET:
                    if (name[0] == '{')
                    {
                        state = SETGUNSTATE_GETGUNINFO;
                    }
                    break;

                case SETGUNSTATE_GETGUNINFO:
                    if (name[0] == '}')
                    {
                        state = SETGUNSTATE_LOOKINGFORGUN;
                        if(shipstatinfo->shipclass == CLASS_Fighter)
                        {
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Evasive]=   gunstaticinfo->gunstatics[processingGun].baseGunDamageLo * tacticsInfo.DamageBonus[Tactics_Fighter][Evasive];
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Neutral]=   gunstaticinfo->gunstatics[processingGun].baseGunDamageLo * tacticsInfo.DamageBonus[Tactics_Fighter][Neutral];
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Aggressive]=gunstaticinfo->gunstatics[processingGun].baseGunDamageLo * tacticsInfo.DamageBonus[Tactics_Fighter][Aggressive];

                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Evasive]=   gunstaticinfo->gunstatics[processingGun].baseGunDamageHi * tacticsInfo.DamageBonus[Tactics_Fighter][Evasive];
                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Neutral]=   gunstaticinfo->gunstatics[processingGun].baseGunDamageHi * tacticsInfo.DamageBonus[Tactics_Fighter][Neutral];
                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Aggressive]=gunstaticinfo->gunstatics[processingGun].baseGunDamageHi * tacticsInfo.DamageBonus[Tactics_Fighter][Aggressive];
                        }
                        else if(shipstatinfo->shipclass == CLASS_Corvette)
                        {
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Evasive]=   gunstaticinfo->gunstatics[processingGun].baseGunDamageLo * tacticsInfo.DamageBonus[Tactics_Corvette][Evasive];
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Neutral]=   gunstaticinfo->gunstatics[processingGun].baseGunDamageLo * tacticsInfo.DamageBonus[Tactics_Corvette][Neutral];
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Aggressive]=gunstaticinfo->gunstatics[processingGun].baseGunDamageLo * tacticsInfo.DamageBonus[Tactics_Corvette][Aggressive];

                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Evasive]=   gunstaticinfo->gunstatics[processingGun].baseGunDamageHi * tacticsInfo.DamageBonus[Tactics_Corvette][Evasive];
                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Neutral]=   gunstaticinfo->gunstatics[processingGun].baseGunDamageHi * tacticsInfo.DamageBonus[Tactics_Corvette][Neutral];
                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Aggressive]=gunstaticinfo->gunstatics[processingGun].baseGunDamageHi * tacticsInfo.DamageBonus[Tactics_Corvette][Aggressive];
                        }
                        else
                        {
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Evasive]=gunstaticinfo->gunstatics[processingGun].baseGunDamageLo;
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Neutral]=gunstaticinfo->gunstatics[processingGun].baseGunDamageLo;
                            gunstaticinfo->gunstatics[processingGun].gunDamageLo[Aggressive]=gunstaticinfo->gunstatics[processingGun].baseGunDamageLo;
                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Evasive]=gunstaticinfo->gunstatics[processingGun].baseGunDamageHi;
                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Neutral]=gunstaticinfo->gunstatics[processingGun].baseGunDamageHi;
                            gunstaticinfo->gunstatics[processingGun].gunDamageHi[Aggressive]=gunstaticinfo->gunstatics[processingGun].baseGunDamageHi;
                        }

                        processingGun = -1;
                        numGunsProcessed++;
                        break;
                    }
                    else
                    {
                        foundentry = findStructEntry(StaticGunInfoScriptTable,name);
                        if (foundentry != NULL)
                        {
                            structureToFillIn = (ubyte *)&gunstaticinfo->gunstatics[processingGun];
                            foundentry->setVarCB(directory,value,structureToFillIn + (foundentry->offset1 - foundentry->offset2) );
                        }
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
    }

    dbgAssert(numGunsProcessed == (numGuns));

    if (gunstaticinfo != NULL)
    {
        for(k=0;k<NUM_TACTICS_TYPES;k++)
        {
            if(shipstatinfo->shipclass != CLASS_Fighter &&
               shipstatinfo->shipclass != CLASS_Corvette)
            {
                bonus = 1.0f;
                spdbonus = 1.0f;
            }
            else
            {
                if(shipstatinfo->shipclass == CLASS_Fighter)
                {
                    bonus = tacticsInfo.BulletRangeBonus[Tactics_Fighter][k];
                    spdbonus = tacticsInfo.BulletSpeedBonus[Tactics_Fighter][k];
                }
                else
                {
                    bonus = tacticsInfo.BulletRangeBonus[Tactics_Corvette][k];
                    spdbonus = tacticsInfo.BulletSpeedBonus[Tactics_Corvette][k];
                }
            }
            shipstatinfo->minBulletRange[k] = shipstatinfo->bulletRange[k] = gunstaticinfo->gunstatics[0].bulletrange*bonus;
            dbgAssert(numGuns > 0);
            for (i=0;i<numGuns;i++)
            {
                if (k == 0)
                {
                    if (gunstaticinfo->gunstatics[i].angletracking == 0.0f)
                    {
                        gunstaticinfo->gunstatics[i].angletracking = 1.0f;
                    }
                    if (gunstaticinfo->gunstatics[i].declinationtracking == 0.0f)
                    {
                        gunstaticinfo->gunstatics[i].declinationtracking = 1.0f;
                    }
                    if (gunstaticinfo->gunstatics[i].bulletlifetime == 0.0f)
                    {
                        gunstaticinfo->gunstatics[i].bulletlifetime = (gunstaticinfo->gunstatics[i].bulletrange*bonus) / (gunstaticinfo->gunstatics[i].bulletspeed*spdbonus);
                    }
                    //gunstaticinfo->gunstatics[i].burstWaitTime += gunstaticinfo->gunstatics[i].burstFireTime;     not needed anymore
                }

                curbulletrange = gunstaticinfo->gunstatics[i].bulletrange*bonus;
                if (curbulletrange > shipstatinfo->bulletRange[k])
                {
                    shipstatinfo->bulletRange[k] = curbulletrange;
                }
                if (curbulletrange < shipstatinfo->minBulletRange[k])
                {
                    shipstatinfo->minBulletRange[k] = curbulletrange;
                }
            }
        }
        for(k=0;k<NUM_TACTICS_TYPES;k++)
            shipstatinfo->bulletRangeSquared[k] = shipstatinfo->bulletRange[k] * shipstatinfo->bulletRange[k];
    }
    else
    {
        if(shipstatinfo->shiptype == ResourceCollector)
        {
            for(k=0;k<NUM_TACTICS_TYPES;k++)
            {
                shipstatinfo->bulletRange[k] = ASTEROID_HARVEST_RANGE;
                shipstatinfo->bulletRangeSquared[k] = shipstatinfo->bulletRange[k] * shipstatinfo->bulletRange[k];
            }
        }
    }

done:
    fileClose(fh);
}

// Reads in the parameters for the nav light.
void scriptSetNAVLightCB(char *directory,char *field,void *dataToFillIn)
{
    char navlighttypestr[30];
    char navlighttexturename[50];
    NAVLightStatic *navlightstatic = (NAVLightStatic *)dataToFillIn;

    RemoveCommasFromString(field);

    sscanf(field,"%s %s %f %f %f %f %d %s",navlightstatic->name, navlighttypestr, &navlightstatic->flashrateon,
     &navlightstatic->flashrateoff, &navlightstatic->startdelay, &navlightstatic->size, &navlightstatic->minLOD, navlighttexturename);

    if(!strcmp(navlighttexturename, "NULL"))
    {
       navlightstatic->texturehandle = TR_InvalidHandle;
    }
    else
    {
       navlightstatic->texturehandle = trTextureRegister(navlighttexturename, NULL, NULL);
    }

    navlightstatic->lightType = StrToNAVLightType(navlighttypestr);
}

#define SETNAVLIGHTSTATE_START      0
#define SETNAVLIGHTSTATE_SET        1

/*-----------------------------------------------------------------------------
    Name        : scriptSetNAVLightStatics
    Description : allocates and sets the navlightStaticInfo field of shipstatinfo
    Inputs      : directory, filename, shipstatinfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scriptSetNAVLightStatics(char *directory,char *filename,struct ShipStaticInfo *shipstatinfo)
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[80];
    sdword numNavLights;
    sdword sizeofnavlightstaticinfo;
    NAVLightStaticInfo *navlightstaticinfo = NULL;
    scriptStructEntry *foundentry;
    ubyte *structureToFillIn;
    sdword state = 0;
    sdword status;
    sdword processingNAVLight = 0;

    // Set this just in case we don't find any nav lights.
    shipstatinfo->navlightStaticInfo = 0;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fileOpen(fullfilename,FF_TextMode);

    for (;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            switch (state)
            {
                case SETNAVLIGHTSTATE_START:
                    if (strcmp(name,"NUMBER_OF_NAV_LIGHTS") == 0)
                    {
                        scriptSetSdwordCB(directory,value,&numNavLights);
                        if (numNavLights <= 0)
                        {
                            goto done;
                        }

                        sizeofnavlightstaticinfo = sizeofNavLightStaticInfo(numNavLights);

                        navlightstaticinfo = memAlloc(sizeofnavlightstaticinfo,"navlightstaticinfo",NonVolatile);
                        memset(navlightstaticinfo,0,sizeofnavlightstaticinfo);
                        navlightstaticinfo->numNAVLights = numNavLights;

                        shipstatinfo->navlightStaticInfo = navlightstaticinfo;

                        state = SETNAVLIGHTSTATE_SET;
                    }
                    break;

                case SETNAVLIGHTSTATE_SET:
                    foundentry = findStructEntry(StaticNavLightInfoScriptTable,name);
                    if (foundentry != NULL)
                    {
                        dbgAssert(processingNAVLight < numNavLights);
                        dbgAssert(numNavLights > 0);
                        structureToFillIn = (ubyte *)&navlightstaticinfo->navlightstatics[processingNAVLight];
                        foundentry->setVarCB(directory,value,structureToFillIn);
                        processingNAVLight++;
                        if (processingNAVLight >= numNavLights)
                        {
                            goto done;
                        }
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
    }

done:
    fileClose(fh);
}

void scriptSetDockPointCB(char *directory,char *field,void *dataToFillIn)
{
    char docktypestr[30] = "";

    DockStaticPoint *dockstaticpoint = (DockStaticPoint *)dataToFillIn;

    RemoveCommasFromString(field);

    sscanf(field,"%s %s %f %f %f %d %d", &dockstaticpoint->name,
                                         docktypestr,
                                         &dockstaticpoint->flyawaydist,
                                         &dockstaticpoint->mindist,
                                         &dockstaticpoint->maxdist,
                                         &dockstaticpoint->headingdirection,
                                         &dockstaticpoint->updirection);

    dockstaticpoint->type = StrToDockPointType(docktypestr);
}

#define SETDOCKSTATE_START      0
#define SETDOCKSTATE_SET        1

/*-----------------------------------------------------------------------------
    Name        : scriptSetDockStatics
    Description : allocates and sets dockStaticInfo information of shipstatinfo
    Inputs      : directory, filename, shipstatinfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scriptSetDockStatics(char *directory,char *filename,struct ShipStaticInfo *shipstatinfo)
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[80];
    sdword numDockPoints;
    sdword sizeofdockstaticinfo;
    DockStaticInfo *dockstaticinfo = NULL;
    scriptStructEntry *foundentry;
    ubyte *structureToFillIn;
    sdword state = 0;
    sdword status;
    sdword processingDockPoint = 0;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fileOpen(fullfilename,FF_TextMode);

    for (;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            switch (state)
            {
                case SETDOCKSTATE_START:
                    if (strcmp(name,"NUMBER_OF_DOCK_POINTS") == 0)
                    {
                        scriptSetSdwordCB(directory,value,&numDockPoints);
                        if (numDockPoints <= 0)
                        {
                            goto done;
                        }

                        sizeofdockstaticinfo = sizeofDockStaticInfo(numDockPoints);

                        dockstaticinfo = memAlloc(sizeofdockstaticinfo,"dockstaticinfo",NonVolatile);
                        memset(dockstaticinfo,0,sizeofdockstaticinfo);
                        dockstaticinfo->numDockPoints = numDockPoints;

                        shipstatinfo->dockStaticInfo = dockstaticinfo;

                        state = SETDOCKSTATE_SET;
                    }
                    break;

                case SETDOCKSTATE_SET:
                    foundentry = findStructEntry(StaticDockInfoScriptTable,name);
                    if (foundentry != NULL)
                    {
                        dbgAssert(processingDockPoint < numDockPoints);
                        dbgAssert(numDockPoints > 0);
                        structureToFillIn = (ubyte *)&dockstaticinfo->dockstaticpoints[processingDockPoint];
                        foundentry->setVarCB(directory,value,structureToFillIn);
                        processingDockPoint++;
                        if (processingDockPoint >= numDockPoints)
                        {
                            goto done;
                        }
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
    }

done:
    fileClose(fh);
}

/************ Overide Docking script setting *******************/

//CB function to set overide points
void scriptSetDockOverideCB(char *directory,char *field,void *dataToFillIn)
{
    char shiptypestr[30];
    char shipracestr[30];
    char *token;
    char seps[] = " ,";

    real32 tempx,tempy,tempz;
    DockStaticOveride *dockstaticoveride = (DockStaticOveride *)dataToFillIn;

    RemoveCommasFromString(field);

    token = strtok(field,seps);sscanf(token,"%s",shipracestr);
    token = strtok(NULL,seps);sscanf(token,"%s",shiptypestr);
    token = strtok(NULL,seps);sscanf(token,"%d",&dockstaticoveride->useNewOrientation);
    token = strtok(NULL,seps);sscanf(token,"%d",&dockstaticoveride->heading);
    token = strtok(NULL,seps);sscanf(token,"%d",&dockstaticoveride->up);
    token = strtok(NULL,seps);sscanf(token,"%d",&dockstaticoveride->useNewOffset);
    token = strtok(NULL,seps);sscanf(token,"%f",&tempx);
    token = strtok(NULL,seps);sscanf(token,"%f",&tempy);
    token = strtok(NULL,seps);sscanf(token,"%f",&tempz);
    token = strtok(NULL,seps);
    if(token != NULL)
    {
        //light has been specified
        dockstaticoveride->lightNameUsed = TRUE;
        strcpy(dockstaticoveride->lightName,token);
    }
    else
    {
        dockstaticoveride->lightNameUsed = FALSE;
    }

    //convert lightwave monkyness...
    dockstaticoveride->offset.x=tempy;
    dockstaticoveride->offset.y=tempz;
    dockstaticoveride->offset.z=-tempx;

    dockstaticoveride->shiptype = StrToShipType(shiptypestr);
    dockstaticoveride->shiprace = StrToShipRace(shipracestr);

}

/*-----------------------------------------------------------------------------
    Name        : scriptSetDockOverideStatics
    Description : allocates and sets dockStaticInfo information of shipstatinfo
    Inputs      : directory, filename, shipstatinfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scriptSetDockOverideStatics(char *directory,char *filename,struct ShipStaticInfo *shipstatinfo)
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[80];
    sdword numDockOveridePoints;
    sdword sizeofdockoveridestaticinfo;
    DockOverideInfo *dockoverideinfo = NULL;
    scriptStructEntry *foundentry;
    ubyte *structureToFillIn;
    sdword state = 0;
    sdword status;
    sdword processingDockOveridePoint = 0;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fileOpen(fullfilename,FF_TextMode);

    for (;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            switch (state)
            {
                case SETDOCKSTATE_START:
                    if (strcmp(name,"NUMBER_OF_DOCK_OVERIDES") == 0)
                    {
                        scriptSetSdwordCB(directory,value,&numDockOveridePoints);
                        if (numDockOveridePoints <= 0)
                        {
                            goto done;
                        }

                        sizeofdockoveridestaticinfo = sizeof(DockOverideInfo) + (numDockOveridePoints - 1)*sizeof(DockStaticOveride);

                        dockoverideinfo = memAlloc(sizeofdockoveridestaticinfo,"dockoverideinfo",NonVolatile);
                        memset(dockoverideinfo,0,sizeofdockoveridestaticinfo);
                        dockoverideinfo->numDockOverides = numDockOveridePoints;

                        shipstatinfo->dockOverideInfo = dockoverideinfo;

                        state = SETDOCKSTATE_SET;
                    }
                    break;

                case SETDOCKSTATE_SET:
                    foundentry = findStructEntry(StaticDockOverideInfoScriptTable,name);
                    if (foundentry != NULL)
                    {
                        dbgAssert(processingDockOveridePoint < numDockOveridePoints);
                        dbgAssert(numDockOveridePoints > 0);
                        structureToFillIn = (ubyte *)&dockoverideinfo->dockOverides[processingDockOveridePoint];
                        foundentry->setVarCB(directory,value,structureToFillIn);
                        processingDockOveridePoint++;
                        if (processingDockOveridePoint >= numDockOveridePoints)
                        {
                            goto done;
                        }
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
    }

done:
    fileClose(fh);
}


void scriptSetSalvageStatCB(char *directory,char *field,void *dataToFillIn)
{
    StaticInfoHealthGuidanceShipDerelict *statinfo = (StaticInfoHealthGuidanceShipDerelict *)dataToFillIn;

    scriptSetSalvageStatics(directory,globalScriptFileName,statinfo);

    if (statinfo->salvageStaticInfo != NULL)
    {
        mexGetSalvageStaticInfo(statinfo->salvageStaticInfo,statinfo->staticheader.pMexData);
    }
}

void scriptSetSalvagePointCB(char *directory,char *field,void *dataToFillIn)
{
    char saltypestr[30] = "";
    SalvageStaticPoint *salvagestaticpoint = (SalvageStaticPoint *)dataToFillIn;

    RemoveCommasFromString(field);

    //parse line of data here!
    sscanf(field,"%s %s",&salvagestaticpoint->name,saltypestr);


    //sscanf(field,"%s %s %f %f %f %d %d",&dockstaticpoint->name,docktypestr,
    //                           &dockstaticpoint->flyawaydist,&dockstaticpoint->mindist,&dockstaticpoint->maxdist,
    //                           &dockstaticpoint->headingdirection, &dockstaticpoint->updirection);

    salvagestaticpoint->type = StrToSalvagePointType(saltypestr);
}

#define SETSALVAGESTATE_SET_BIG3    5
#define SETSALVAGESTATE_SET_BIG2    4
#define SETSALVAGESTATE_SET_BIG     3
#define SETSALVAGESTATE_SET_NUM     2
#define SETSALVAGESTATE_SET         1
#define SETSALVAGESTATE_START       0

/*-----------------------------------------------------------------------------
    Name        : scriptSetSalvageStatics
    Description : allocates and sets salvageStaticInformation information staticInfo
    Inputs      : directory, filename, dataToFillIn (which is the STATIC INFO) of whatever has called it in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void scriptSetSalvageStatics(char *directory,char *filename,struct StaticInfoHealthGuidanceShipDerelict *statinfo)
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[80];
    sdword numSalvagePoints;
    sdword sizeofsalvagestaticinfo;
    SalvageStaticInfo *salvagestaticinfo = NULL;
    scriptStructEntry *foundentry;
    ubyte *structureToFillIn;
    sdword state = 0;
    sdword status;
    sdword processingSalvagePoint = 0;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fileOpen(fullfilename,FF_TextMode);

    for (;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            switch (state)
            {
                case SETSALVAGESTATE_START:
                    if (strcmp(name,"NUMBER_OF_SALVAGE_POINTS") == 0)
                    {
                        scriptSetSdwordCB(directory,value,&numSalvagePoints);
                        if (numSalvagePoints <= 0)
                        {
                            goto done;
                        }

                        sizeofsalvagestaticinfo = sizeofSalvageStaticInfo(numSalvagePoints);

                        salvagestaticinfo = memAlloc(sizeofsalvagestaticinfo,"salvagestaticinfo",NonVolatile);
                        memset(salvagestaticinfo,0,sizeofsalvagestaticinfo);
                        salvagestaticinfo->numSalvagePoints = numSalvagePoints;

                        statinfo->salvageStaticInfo = salvagestaticinfo;

                        state = SETSALVAGESTATE_SET_NUM;
                    }
                    break;
                case SETSALVAGESTATE_SET_NUM:
                    if (strcmp(name,"NUM_NEEDED_FOR_SALVAGE") == 0)
                    {
                        scriptSetSdwordCB(directory,value,&salvagestaticinfo->numNeededForSalvage);
                        state = SETSALVAGESTATE_SET_BIG;
                    }
                    break;
                case SETSALVAGESTATE_SET_BIG:
                    if (strcmp(name,"NEED_BIGR1") == 0)
                    {
                        scriptSetBool(directory,value,&salvagestaticinfo->needBigR1);
                        state = SETSALVAGESTATE_SET_BIG2;
                    }
                    break;
                case SETSALVAGESTATE_SET_BIG2:
                    if (strcmp(name,"NEED_BIGR2") == 0)
                    {
                        scriptSetBool(directory,value,&salvagestaticinfo->needBigR2);
                        state = SETSALVAGESTATE_SET_BIG3;
                    }
                    break;
                case SETSALVAGESTATE_SET_BIG3:
                    if (strcmp(name,"WILL_FIT_CARRIER") == 0)
                    {
                        scriptSetBool(directory,value,&salvagestaticinfo->willFitCarrier);
                        state = SETSALVAGESTATE_SET;
                    }
                    break;
                case SETSALVAGESTATE_SET:
                    foundentry = findStructEntry(StaticSalvageInfoScriptTable,name);
                    if (foundentry != NULL)
                    {
                        dbgAssert(processingSalvagePoint < numSalvagePoints);
                        dbgAssert(numSalvagePoints > 0);
                        structureToFillIn = (ubyte *)&salvagestaticinfo->salvageStaticPoints[processingSalvagePoint];
                        foundentry->setVarCB(directory,value,structureToFillIn);
                        processingSalvagePoint++;
                        if (processingSalvagePoint >= numSalvagePoints)
                        {
                            goto done;
                        }
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
    }

done:
    if(state == SETSALVAGESTATE_START)
    {
        statinfo->salvageStaticInfo = NULL; //set to Null if not found
    }
    fileClose(fh);
}


#ifdef USE_SPHERE_TABLES

#define SPHERESTATE_START           0
#define SPHERESTATE_LOOKINGFORTABLE 1
#define SPHERESTATE_GETTABLE        2

/*-----------------------------------------------------------------------------
    Name        : scriptSetSphereStaticInfo
    Description : allocates, sets and returns SphereStaticInfo based on contents of script file
    Inputs      : directory, filename
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
struct SphereStaticInfo *scriptSetSphereStaticInfo(char *directory,char *filename)
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    char fullfilename[80];
    sdword status;
    sdword state = 0;
    sdword numTables = 0;
    sdword curTable = -1;
    sdword nextTableToProcess = 0;
    sdword curDeclination = -1;
    sdword numDeclinations = 0;
    sdword sizeofspherestaticinfo;
    sdword sizeofspheretableentry;
    struct SphereStaticInfo *sphereStaticInfo = NULL;
    SphereTableEntry *spheretableentry = NULL;

    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,filename);
    }
    else
    {
        strcpy(fullfilename,filename);
    }

    fh = fileOpen(fullfilename,FF_TextMode);

    for (;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            switch (state)
            {
                case SPHERESTATE_START:
                    if (strcmp(name,"NUMBER_SPHERE_TABLES") == 0)
                    {
                        scriptSetSdwordCB(directory,value,&numTables);
                        if (numTables <= 0)
                        {
                            goto done;
                        }

                        sizeofspherestaticinfo = sizeofSphereStaticInfo(numTables);

                        sphereStaticInfo = memAlloc(sizeofspherestaticinfo,"spherestatinfo",NonVolatile);
                        memset(sphereStaticInfo,0,sizeofspherestaticinfo);

                        sphereStaticInfo->numTableEntries = numTables;

                        state = SPHERESTATE_LOOKINGFORTABLE;
                    }
                    break;

                case SPHERESTATE_LOOKINGFORTABLE:
                    if (strcmp(name,"SPHERE_TABLE") == 0)
                    {
foundtable:
                        curTable = nextTableToProcess++;
                        curDeclination = 0;

                        scriptSetSdwordCB(directory,value,&numDeclinations);
                        dbgAssert(numDeclinations > 0);

                        sizeofspheretableentry = sizeofSphereTableEntry(numDeclinations);

                        sphereStaticInfo->sphereTableEntryPtrs[curTable] = spheretableentry = memAlloc(sizeofspheretableentry,"spheretabentry",NonVolatile);
                        memset(spheretableentry,0,sizeofspheretableentry);

                        spheretableentry->numDeclinations = numDeclinations;

                        state = SPHERESTATE_GETTABLE;
                    }
                    break;

                case SPHERESTATE_GETTABLE:
                    dbgAssert(curTable > -1);
                    dbgAssert(curTable < numTables);
                    dbgAssert(spheretableentry);
                    dbgAssert(curDeclination >= 0);
                    if (strcmp(name,"SPHERE_TABLE") == 0)
                    {
                        goto foundtable;
                    }
                    else
                    {
                        if (strcmp(name,"Ships") == 0)
                        {
                            scriptSetSdwordCB(directory,value,&spheretableentry->numShipsCanHandle);
                        }
                        else if (strcmp(name,"Declination") == 0)
                        {
                            dbgAssert(curDeclination < numDeclinations);
                            scriptSetSphereDeclinationCB(directory,value,&spheretableentry->sphereDeclinations[curDeclination]);
                            curDeclination++;
                        }
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
    }

done:
    fileClose(fh);
    return sphereStaticInfo;
}

#endif

/*-----------------------------------------------------------------------------
    Name        : scriptSetTweakableGlobals
    Description : sets all tweakable global variables in game
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void scriptSetTweakableGlobals()
{
    // add all calls to set tweakable global variables here
    sdword i;
    for(i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        TW_HyperSpaceCostStruct[i].canMpHyperspace = FALSE;
    }

    scriptSet(NULL,"AIShip.script",AIShipTweaks);
    //scriptSet(NULL, "AIStrategy.script",AIStartegyTweaks;
    scriptSet(NULL,"AITrack.script",AITrackTweaks);
    scriptSet(NULL,"Camera.script",CameraTweaks);
    scriptSet(NULL,"CameraCommand.script",CameraCommandTweaks);
    scriptSet(NULL,"CommandLayer.script",CommandLayerTweaks);
    scriptSet(NULL,"Formation.script",FormationTweaks);
    scriptSet(NULL,"Galaxy.script",StarTweaks);
    scriptSet(NULL,"Tweak.script",Tweaks);
    scriptSet(NULL,"Flightman.script",FlightmanTweaks);
    scriptSet(NULL,"Dock.script",DockTweaks);
    scriptSet(NULL,"Soundevent.script",SoundeventTweaks);
    scriptSet(NULL,"Trail.script",TrailTweaks);
    scriptSet(NULL,"Resource.script",ResCollectTweaks);
    scriptSet(NULL,"Cloud.script",CloudTweaks);
    scriptSet(NULL,"SinglePlayer.script",SinglePlayerTweaks);
    scriptSet(NULL,"Nebulae.script",NebulaeTweaks);
    scriptSet(NULL,"AIPlayer.script",AIPlayerTweaks);
    scriptSet(NULL,"NetTweak.script",NetTweaks);
    scriptSet(NULL,"AIResourceMan.script",AIResourceManTweaks);
    scriptSet(NULL,"UnitLimitCaps.script",ShipStaticCapLimits);
    scriptSet(NULL,"Damage.script",DamageTweaks);
    scriptSet(NULL,"ShipView.script", ShipViewTweaks);
    scriptSet(NULL,"AutoLOD.script", AutoLODTweaks);
    scriptSet(NULL,"GatherStatsFor.script",GatherStatsScriptTable);
    scriptSet(NULL,"FEColour.script",FrontEndColourTweaks);
    crateInit();
    paradeSetTweakables();
}

GameType gameTemplate;

scriptStructEntry StaticMGInfoScriptTable[] =
{
    { "type.flag",    scriptSetBitUword,  (udword) &(gameTemplate.flag), (udword) &gameTemplate },
    { "type.flagNeeded", scriptSetBitUword,  (udword) &(gameTemplate.flagNeeded), (udword) &gameTemplate },

    { "type.numComputers",    scriptSetUbyteCB,  (udword) &(gameTemplate.numComputers), (udword) &gameTemplate },
    { "type.startingFleet",    scriptSetUbyteCB,  (udword) &(gameTemplate.startingFleet), (udword) &gameTemplate },
    { "type.bountySize",    scriptSetUbyteCB,  (udword) &(gameTemplate.bountySize), (udword) &gameTemplate },
    { "type.startingResources",    scriptSetUbyteCB,  (udword) &(gameTemplate.startingResources), (udword) &gameTemplate },

    { "type.resourceInjectionInterval",    scriptSetUdwordCB,  (udword) &(gameTemplate.resourceInjectionInterval), (udword) &gameTemplate },
    { "type.resourceInjectionsAmount",    scriptSetUdwordCB,  (udword) &(gameTemplate.resourceInjectionsAmount), (udword) &gameTemplate },
    { "type.resourceLumpSumTime",    scriptSetUdwordCB,  (udword) &(gameTemplate.resourceLumpSumTime), (udword) &gameTemplate },
    { "type.resourceLumpSumAmount",    scriptSetUdwordCB,  (udword) &(gameTemplate.resourceLumpSumAmount), (udword) &gameTemplate },

    { NULL,NULL,0,0 }
};

#define GT_NUMGAMES  0
#define GT_FINDGAME  1
#define GT_OPENGAME  2

//reads in gametypes.script and fills preSetGames
void mgGameTypeScriptInit()
{
    filehandle fh;
    char line[MAX_LINE_CHARS];
    char *name, *value;
    sdword state;
    sdword numGames;
    char *token;
    sdword gameNum = 0;
    scriptStructEntry *foundentry;
    ubyte *structureToFillIn;
    sdword status;
    char seps[] = ", ";
    sdword donegames;

    state = GT_NUMGAMES;

    fh = fileOpen("gametypes.script",FF_TextMode);

    donegames = FALSE;
    preSetGames = NULL;

    for(;;)
    {
        status = fileLineRead(fh,line,MAX_LINE_CHARS);

        if (status == FR_EndOfFile)
        {
            break;
        }

        if (parseLine(line,&name,&value))
        {
            switch (state)
            {
            case GT_NUMGAMES:
                if(strcmp(name,"numGames") == 0)
                {
                    scriptSetSdwordCB("",value,&numGames);
                    preSetGames = (GameTypes *) memAlloc(sizeof(GameTypes) + sizeof(GameType)*(numGames-1),"GameTypeScripts",NonVolatile);
                    memset(preSetGames,0xff,sizeof(GameTypes) + sizeof(GameType)*(numGames-1));
                    preSetGames->numGameTypes = numGames;
                    if(numGames <= 0)
                    {
                        donegames = TRUE;
                        break;
                    }
                    state = GT_FINDGAME;
                }
                break;
            case GT_FINDGAME:
                if(strcmp(name,"GAME")==0)
                {
                   token = strtok(line,seps);
                   //get game name
                   strcpy(preSetGames->gameType[gameNum].gameName,value);

                   preSetGames->gameType[gameNum].flag = 0; //for these pre-defined game types, use all bits
                   state = GT_OPENGAME;
                }
                break;
            case GT_OPENGAME:
                if(name[0] == '}')
                {
                    gameNum++;
                    state = GT_FINDGAME;
                }
                else
                {
                    foundentry = findStructEntry(StaticMGInfoScriptTable,name);
                    if (foundentry != NULL)
                    {
                        structureToFillIn = (ubyte *)&preSetGames->gameType[gameNum];
                        foundentry->setVarCB("",value,structureToFillIn + (foundentry->offset1 - foundentry->offset2) );
                    }
                }
                break;
            default:
                break;
            }
        }
    if(donegames)
        break;
    }
}

