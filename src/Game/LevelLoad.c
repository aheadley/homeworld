
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "Vector.h"
#include "SpaceObj.h"
#include "UnivUpdate.h"
#include "Universe.h"
#include "CommandLayer.h"
#include "StatScript.h"
#include "ScenPick.h"
#include "LevelLoad.h"
#include "light.h"
#include "Sensors.h"
#include "SinglePlayer.h"
#include "SoundEvent.h"
#include "Nebulae.h"
#include "mainrgn.h"
#include "BTG.h"
#include "ResearchAPI.h"
#include "AITeam.h"
#include "AIPlayer.h"
#include "KAS.h"
#include "File.h"
#include "Teams.h"
#include "MultiplayerGame.h"
#include "Attributes.h"
#include "Tweak.h"
#include "Ping.h"
#include "Randy.h"
#include "ResearchShip.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#define CapturableCapitalShipToGive StandardFrigate

udword nebAttributes = 0;
sdword SongNumber = -1;

typedef struct
{
    sdword playerNumber;
    vector position;
    real32 sphererotz;
} MissionSphereInfo;

ResourceDistribution resourceDistTemplate;

/*=============================================================================
    Private Function prototypes:
=============================================================================*/

static void scriptSetDerelictCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetResourcesCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetShipsCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetAIPointCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetAIPathCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetAIBoxCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetAISphereCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetMissionSphereCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetLightingCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetBackgroundCB(char* directory, char* field, void* dataToFillIn);
static void scriptSetUword2CB(char *directory,char *field,void *dataToFillIn);

/*=============================================================================
    Private Data:
=============================================================================*/

scriptStructEntry AsteroidDistScriptTable[] =
{
    { "Asteroid0", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[Asteroid0][0], (udword)&resourceDistTemplate },
    { "Asteroid1", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[Asteroid1][0], (udword)&resourceDistTemplate },
    { "Asteroid2", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[Asteroid2][0], (udword)&resourceDistTemplate },
    { "Asteroid3", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[Asteroid3][0], (udword)&resourceDistTemplate },
    { "Asteroid4", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[Asteroid4][0], (udword)&resourceDistTemplate },
    { NULL,NULL,0,0 }
};

scriptStructEntry DustCloudDistScriptTable[] =
{
    { "DustCloud0", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[DustCloud0][0], (udword)&resourceDistTemplate },
    { "DustCloud1", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[DustCloud1][0], (udword)&resourceDistTemplate },
    { "DustCloud2", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[DustCloud2][0], (udword)&resourceDistTemplate },
    { "DustCloud3", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[DustCloud3][0], (udword)&resourceDistTemplate },
    { NULL,NULL,0,0 }
};

scriptStructEntry GasCloudDistScriptTable[] =
{
    { "GasCloud0", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[GasCloud0][0], (udword)&resourceDistTemplate },
    { "GasCloud1", scriptSetUword2CB, (udword)&resourceDistTemplate.probResources[GasCloud1][0], (udword)&resourceDistTemplate },
    { NULL,NULL,0,0 }
};

scriptStructEntry MissionSphereScriptTable[] =
{
    { "Ships", scriptSetShipsCB, 0,0 },
    { "AIPoint", scriptSetAIPointCB, 0,0 },
    { "AIPath", scriptSetAIPathCB, 0,0 },
    { "AIBox", scriptSetAIBoxCB, 0,0 },
    { "AISphere", scriptSetAISphereCB, 0,0 },
    { "Resources", scriptSetResourcesCB, 0,0 },
    { "Derelict", scriptSetDerelictCB, 0,0 },
    { NULL,NULL,0,0 }
};

scriptEntry SMMissionTweaks[] =
{
    makeEntry(smDepthCueRadius,scriptSetReal32CB),
    makeEntry(smDepthCueStartRadius,scriptSetReal32CB),
    makeEntry(smCircleBorder,scriptSetSdwordCB),
    makeEntry(smZoomMax,scriptSetReal32CB),
    makeEntry(smZoomMin,scriptSetReal32CB),
    makeEntry(smInitialDistance,scriptSetReal32CB),
    makeEntry(smUniverseSizeX,scriptSetReal32CB),
    makeEntry(smUniverseSizeY,scriptSetReal32CB),
    makeEntry(smUniverseSizeZ,scriptSetReal32CB),
    makeEntry(SongNumber,     scriptSetSdwordCB),
    endEntry
};

scriptStructEntry MissionScriptTable[] =
{
    { "MissionSphere", scriptSetMissionSphereCB, 0,0 },
    { "Lighting", scriptSetLightingCB, 0,0 },
    { "Background", scriptSetBackgroundCB, 0,0 },
    { NULL,NULL,0,0 }
};

static void llExcludeShip(char *directory,char *field,void *dataToFillIn);
static void llExcludeAsteroid(char *directory,char *field,void *dataToFillIn);
static void llExcludeDustCloud(char *directory,char *field,void *dataToFillIn);
static void llExcludeGasCloud(char *directory,char *field,void *dataToFillIn);
static void llExcludeNebula(char *directory,char *field,void *dataToFillIn);
static void llExcludeDerelict(char *directory,char *field,void *dataToFillIn);
static void llAvailableColorScheme(char *directory,char *field,void *dataToFillIn);
static void scriptSetShipsToBeNeeded(char *directory,char *field,void *dataToFillIn);
static void scriptSetDerelictToBeNeeded(char *directory,char *field,void *dataToFillIn);
static void scriptPreMissionSphereCB(char *directory,char *field,void *dataToFillIn);
bool AnyShipOverlapsAsteroid(vector *position,AsteroidType asteroidtype);

scriptEntry MissionPreloadScriptTable[] =
{
    { "TrailColor",     teTrailColorSet,NULL},      //player #, control point #, R, G, B
#if !TO_STANDARD_COLORS
    { "TOColor",        teColorSet, &teColorSchemes[0].tacticalColor},  //player #, R, G, B
#endif
    { "BaseColor",      teColorSet, &teColorSchemes[0].textureColor.base},  //player #, R, G, B
    { "StripeColor",    teColorSet, &teColorSchemes[0].textureColor.detail}, //player #, R, G, B
    { "ExcludeShips",   llExcludeShip,      NULL},          //race, <shiptype|all]>
    { "IncludeShips",   llExcludeShip,      (void *)1},     //race, <shiptype|all]>
    { "ExcludeAsteroid",llExcludeAsteroid,  NULL},          //asteroidType
    { "IncludeAsteroid",llExcludeAsteroid,  (void *)1},     //asteroidType
    { "ExcludeDustCloud",llExcludeDustCloud,NULL},          //Type
    { "IncludeDustCloud",llExcludeDustCloud,(void *)1},     //Type
    { "ExcludeGasCloud",llExcludeGasCloud,  NULL},          //Type
    { "IncludeGasCloud",llExcludeGasCloud,  (void *)1},     //Type
    { "ExcludeNebula",  llExcludeNebula,    NULL},          //Type
    { "IncludeNebula",  llExcludeNebula,    (void *)1},     //Type
    { "ExcludeDerelict",llExcludeDerelict,  NULL},          //Type
    { "IncludeDerelict",llExcludeDerelict,  (void *)1},     //Type
    { "AvailableColorSchemes",llAvailableColorScheme, NULL},//race <shiptype|all> scheme#[,scheme#[,...]]

    { NULL,NULL,0 }
};

scriptEntry MissionPreloadScriptTablePass2[] =
{
    { "MissionSphere", scriptPreMissionSphereCB, NULL },
    { NULL,NULL,0 }
};

bool needR1CapitalShip = FALSE;
bool needR2CapitalShip = FALSE;

scriptStructEntry MissionPreloadMissphereTable[] =
{
    { "Ships", scriptSetShipsToBeNeeded, 0,0 },
    { "Derelict", scriptSetDerelictToBeNeeded, 0,0 },
    { NULL,NULL,0,0 }
};

bool missionman = FALSE;

/*=============================================================================
    Functions:
=============================================================================*/

static void scriptSetUword2CB(char *directory,char *field,void *dataToFillIn)
{
    sdword fillin[2] = { -1,-1 };

    RemoveCommasFromString(field);

    sscanf(field,"%d %d",&fillin[0],&fillin[1]);

    if (fillin[1] == -1)
    {
        fillin[1] = fillin[0];
    }

    ((uword *)dataToFillIn)[0] = (uword)fillin[0];
    ((uword *)dataToFillIn)[1] = (uword)fillin[1];
}

static void SetSMMissionTweaks(char *directory,char *filename)
{
    // set to defaults in case .level file doesn't define them
    smDepthCueRadius = SM_DepthCueRadius;
    smDepthCueStartRadius = SM_DepthCueStartRadius;
    smCircleBorder = SM_CircleBorder;
    smZoomMax = SM_ZoomMax;
    smZoomMin = SM_ZoomMin;

    SongNumber = -1;

    scriptSet(directory,filename,SMMissionTweaks);

    // later, put properly into sensors.c
    smUpdateParameters();
}

static void GetResourceDistribution(ResourceDistribution *resourceDistribution,char *directory,char *name,ObjType resourceobjtype)
{
    sdword i,j;
    uword cum;

    memset(resourceDistribution,0,sizeof(ResourceDistribution));

    switch (resourceobjtype)
    {
        case OBJ_AsteroidType:
            scriptSetStruct(directory,name,AsteroidDistScriptTable,(ubyte *)resourceDistribution);
            break;

        case OBJ_DustType:
            scriptSetStruct(directory,name,DustCloudDistScriptTable,(ubyte *)resourceDistribution);
            break;

        case OBJ_GasType:
            scriptSetStruct(directory,name,GasCloudDistScriptTable,(ubyte *)resourceDistribution);
            break;

        default:
            dbgFatalf(DBG_Loc,"Unknown Object Type %d for resource type",resourceobjtype);
            break;
    }

    for (j=0;j<2;j++)
    {
        for (i=0,cum=0;i<MAXNUM_RESOURCETYPES;i++)
        {
            cum += resourceDistribution->probResources[i][j];
            resourceDistribution->cumResources[i][j] = cum;
        }
        if (cum == 0)
        {
            dbgFatalf(DBG_Loc,"\nNo distribution set for %s",ObjTypeToStr(resourceobjtype));
        }
        resourceDistribution->sumTotal[j] = cum;
    }
}

udword GetRandomResourceType(ResourceDistribution *resourceDistribution,bool regrowing)
{
    udword randnum;
    udword i;
    udword index = 0;

    if (regrowing)
    {
        index = 1;
    }

    randnum = randomG(resourceDistribution->sumTotal[index]);

    for (i=0;i<MAXNUM_RESOURCETYPES;i++)
    {
        if (randnum < resourceDistribution->cumResources[i][index])
        {
            return i;
        }
    }
    dbgAssert(FALSE);
    return 0;
}

void AddRandomRotationToAsteroid(Asteroid *asteroid)
{
    dbgAssert(asteroid->asteroidtype < NUM_ASTEROIDTYPES);
    if (asteroid->asteroidtype >= Asteroid3)
    if (((udword)(gamerand() & 255)) <= ASTEROID_ROTATE_PROB)
    {
        real32 lorotate = ASTEROID_LO_ROTATE[asteroid->asteroidtype];
        real32 hirotate = ASTEROID_HI_ROTATE[asteroid->asteroidtype];
        asteroid->rotinfo.rotspeed.x = frandombetween(lorotate,hirotate);
        asteroid->rotinfo.rotspeed.y = frandombetween(lorotate,hirotate);
        asteroid->rotinfo.rotspeed.z = frandombetween(lorotate,hirotate);
    }
}

void AddRandomVelocityTowards(SpaceObj *obj,vector *from,vector *towards,real32 avgVel,real32 deviation)
{
    real32 dev2 = deviation*2.0f;
    vector devvector;
    vector diff;

    devvector.x = frandom(dev2) - deviation;
    devvector.y = frandom(dev2) - deviation;
    devvector.z = frandom(dev2) - deviation;

    vecSub(diff,*towards,*from);
    vecNormalize(&diff);
    vecAddTo(diff,devvector);

    vecMultiplyByScalar(diff,avgVel);

    obj->posinfo.velocity = diff;
    obj->posinfo.isMoving = TRUE;
}

void AddRandomVelocityDirection(SpaceObj *obj,vector *direction,real32 avgVel,real32 deviation)
{
    //real32 dev2 = deviation*2.0f;
    vector devvector;

    devvector.x = 0;//frandom(dev2) - deviation;
    devvector.y = 0;//frandom(dev2) - deviation;
    devvector.z = 0;//frandom(dev2) - deviation;

    vecNormalize(direction);
    vecAddTo(*direction,devvector);

    vecMultiplyByScalar(*direction,avgVel);

    obj->posinfo.velocity = *direction;
    obj->posinfo.isMoving = TRUE;
}

bool AddResourceToVolumeAtPosition(struct ResourceVolume *volume,bool regrowing,vector *position)
{
    switch (volume->resourceObjType)
    {
        case OBJ_AsteroidType:
        {
            Asteroid *asteroid;
            AsteroidType asteroidtype;
            asteroidtype = (AsteroidType)GetRandomResourceType(&volume->resourceDistribution,regrowing);
            dbgAssert(asteroidtype < NUM_ASTEROIDTYPES);

            if (regrowing)
            {
                // may not actually add asteroid if it overlaps...
                if (AnyShipOverlapsAsteroid(position,asteroidtype))
                {
                    return FALSE;
                }
            }

            asteroid = univAddAsteroid(asteroidtype,position);
            asteroid->attributes = volume->attributes;
            asteroid->attributesParam = volume->attributesParam;
            if (volume->attributes & ATTRIBUTES_Regrow)
            {
                asteroid->resourceVolume = volume;
            }
            AddRandomRotationToAsteroid(asteroid);
            if (volume->attributes & ATTRIBUTES_VelToMothership)
            {
                hvector dirvector;

                GetPointOfName(&dirvector,"TowardsVector");
                AddRandomVelocityDirection((SpaceObj *)asteroid,(vector *)&dirvector,attrVELOCITY_TOWARDS,attrVELOCITY_TOWARDS_DEVIATION);
            }
            if (volume->attributes & ATTRIBUTES_HeadShotVelToMothership)
            {
                hvector startpoint;

                GetPointOfName(&startpoint,"TowardsPoint");
                AddRandomVelocityTowards((SpaceObj *)asteroid,&asteroid->posinfo.position,(vector *)&startpoint,attrHEADSHOTVELOCITY_TOWARDS,attrHEADSHOTVELOCITY_TOWARDS_DEVIATION);
            }
        }
        break;

        case OBJ_DustType:
        {
            DustCloud *dustcloud;
            DustCloudType dusttype;
            dusttype = (DustCloudType)GetRandomResourceType(&volume->resourceDistribution,regrowing);
            dbgAssert(dusttype < NUM_DUSTCLOUDTYPES);
            dustcloud = univAddDustCloud(dusttype,position);
            dustcloud->attributes = volume->attributes;
            dustcloud->attributesParam = volume->attributesParam;
            if (dustcloud->attributes & ATTRIBUTES_Regrow)
            {
                dustcloud->resourceVolume = volume;
            }
        }
        break;

        case OBJ_GasType:
        {
            GasCloud *gascloud;
            GasCloudType gastype;
            gastype = (GasCloudType)GetRandomResourceType(&volume->resourceDistribution,regrowing);
            dbgAssert(gastype < NUM_GASCLOUDTYPES);
            gascloud = univAddGasCloud(gastype,position);
            gascloud->attributes = volume->attributes;
            gascloud->attributesParam = volume->attributesParam;
            if (gascloud->attributes & ATTRIBUTES_Regrow)
            {
                gascloud->resourceVolume = volume;
            }
        }
        break;
    default:
        break;
    }

    return TRUE;
}

bool AddResourceToRectangle(struct ResourceVolume *rectangle,bool regrowing)
{
    real32 width = rectangle->radius;
    real32 length = rectangle->length;
    real32 height = rectangle->roty;
    real32 rotaboutz = rectangle->rotz;

    matrix rotzmat;
    vector position1;
    vector position2;
    vector finalpos;

    dbgAssert(rectangle->resourceVolumeType == ResourceRectangleType);

    position1.x = frandombetween(-width,width);
    position1.y = frandombetween(-length,length);
    position1.z = frandombetween(-height,height);

    matMakeRotAboutZ(&rotzmat,(real32)cos(rotaboutz),(real32)sin(rotaboutz));

    matMultiplyMatByVec(&position2,&rotzmat,&position1);

    vecAdd(finalpos,position2,rectangle->centre);

    return AddResourceToVolumeAtPosition(rectangle,regrowing,&finalpos);
}

static void AddRectangle(ResourceVolume *rectangle)
{
    sdword i;

    dbgAssert(rectangle->resourceVolumeType == ResourceRectangleType);

    for (i=0;i<rectangle->number;i++)
    {
        AddResourceToRectangle(rectangle,FALSE);
    }

    if (rectangle->attributes & ATTRIBUTES_Regrow)
    {
        listAddNode(&universe.ResourceVolumeList,&rectangle->link,rectangle);
    }
    else
    {
        memFree(rectangle);
    }
}

bool AddResourceToCylinder(struct ResourceVolume *cylinder,bool regrowing)
{
    real32 rotabouty = cylinder->roty;
    real32 rotaboutz = cylinder->rotz;
    real32 radius = cylinder->radius;
    real32 length = cylinder->length;
    real32 lengthDiv2 = length * 0.5f;
    real32 diameter = radius * 2.0f;

    matrix rotymat;
    matrix rotzmat;
    vector position1;
    vector position2;

    vector finalpos;

    dbgAssert(cylinder->resourceVolumeType == ResourceCylinderType);

    matMakeRotAboutY(&rotymat,(real32)cos(rotabouty),(real32)sin(rotabouty));
    matMakeRotAboutZ(&rotzmat,(real32)cos(rotaboutz),(real32)sin(rotaboutz));

    position1.x = frandom(length) - lengthDiv2;
    position1.y = frandom(diameter) - radius;
    position1.z = frandom(diameter) - radius;

    matMultiplyMatByVec(&position2,&rotymat,&position1);
    matMultiplyMatByVec(&position1,&rotzmat,&position2);

    vecAdd(finalpos,position1,cylinder->centre);

    return AddResourceToVolumeAtPosition(cylinder,regrowing,&finalpos);
}

static void AddCylinder(ResourceVolume *cylinder)
{
    sdword i;

    dbgAssert(cylinder->resourceVolumeType == ResourceCylinderType);

    for (i=0;i<cylinder->number;i++)
    {
        AddResourceToCylinder(cylinder,FALSE);
    }

    if (cylinder->attributes & ATTRIBUTES_Regrow)
    {
        listAddNode(&universe.ResourceVolumeList,&cylinder->link,cylinder);
    }
    else
    {
        memFree(cylinder);
    }
}

bool AddResourceToSphere(struct ResourceVolume *sphere,bool regrowing)
{
    real32 diameter;
    real32 radius;
    vector randpos;

    dbgAssert(sphere->resourceVolumeType == ResourceSphereType);

    diameter = sphere->radius * 2.0f;
    radius = sphere->radius;

    if (sphere->attributes & ATTRIBUTES_ShellOfResources)
    {
        real32 width = radius * attrRING_WIDTH_SCALE;
        real32 r = frandombetween(width,radius);
        real32 ang = frandom(TWOPI);
        real32 decl = frandom(DEG_TO_RAD(160.0f)) - DEG_TO_RAD(80.0f);
        matrix roty,rotz;
        vector temp,temp2;

        vecSet(temp,r,0.0f,0.0f);
        matMakeRotAboutY(&roty,(real32)cos(decl),(real32)sin(decl));
        matMakeRotAboutZ(&rotz,(real32)cos(ang),(real32)sin(ang));
        matMultiplyMatByVec(&temp2,&roty,&temp);
        matMultiplyMatByVec(&randpos,&rotz,&temp2);
    }
    else
    {
        randpos.x = frandom(diameter) - radius;
        randpos.y = frandom(diameter) - radius;
        randpos.z = frandom(diameter) - radius;
    }

    vecAddTo(randpos,sphere->centre);

    return AddResourceToVolumeAtPosition(sphere,regrowing,&randpos);
}

static void AddSphere(ResourceVolume *sphere)
{
    sdword i;

    dbgAssert(sphere->resourceVolumeType == ResourceSphereType);

    for (i=0;i<sphere->number;i++)
    {
        AddResourceToSphere(sphere,FALSE);
    }

    if (sphere->attributes & ATTRIBUTES_Regrow)
    {
        listAddNode(&universe.ResourceVolumeList,&sphere->link,sphere);
    }
    else
    {
        memFree(sphere);
    }
}

static void scriptSetResourcesCB(char *directory,char *field,void *dataToFillIn)
{
    MissionSphereInfo *missionSphereInfo = (MissionSphereInfo *)dataToFillIn;
    vector position;
    real32 radius;
    char resourcetypestr[50];
    char layoutstr[50];
    char distributionstr[50];
    udword numAsteroids;
    real32 length;
    real32 roty,rotz;
    ResourceVolume *resourceVolume;
    matrix rotaboutz;
    vector rotatedposition;
    ObjType resourceobjtype;
    udword attributes = 0;
    sdword attributesParam = 0;

    RemoveCommasFromString(field);

    sscanf(field,"%s %f %f %f %s %s %d %f %f %f %f %d %d",resourcetypestr,&position.x,&position.y,&position.z,layoutstr,distributionstr,
                                              &numAsteroids,&radius,&length,&roty,&rotz,&attributes,&attributesParam);

    dbgAssert(attributesParam <= SWORD_Max && attributesParam >= SWORD_Min);
    if ((distributionstr[0] == '0') && (distributionstr[1] == 0))
    {
        dbgFatalf(DBG_Loc,"No distribution specified in %s %s",directory,field);
    }

    //roty = DEG_TO_RAD(roty);  not used as angle in every case, delay DEG_TO_RAD till later
    rotz = DEG_TO_RAD(rotz);

    resourceobjtype = StrToObjType(resourcetypestr);

    if (!missionman)
    {
        matMakeRotAboutZ(&rotaboutz,(real32)cos(missionSphereInfo->sphererotz),(real32)sin(missionSphereInfo->sphererotz));

        matMultiplyMatByVec(&rotatedposition,&rotaboutz,&position);
        vecAddTo(rotatedposition,missionSphereInfo->position);

        rotz += missionSphereInfo->sphererotz;
    }
    else
    {
        rotatedposition = position;
    }

    if (singlePlayerGame && bitTest(attributes, ATTRIBUTES_ScaleResources))
    {
        // scale the number of asteroids depending on fleet strength
        if (bitTest(attributes, ATTRIBUTES_KillerCollDamage))
        {
            // scale up
            numAsteroids = (udword)((real32)numAsteroids*spFleetModifier);
        }
        else
        {
            // scale down
            numAsteroids = (udword)((real32)numAsteroids*(1-spFleetModifier));
        }
    }

    if (strcmp(layoutstr,"Sphere") == 0)
    {
        resourceVolume = memAlloc(sizeof(ResourceVolume),"ResourceSphere",NonVolatile);

        resourceVolume->resourceVolumeType = ResourceSphereType;
        resourceVolume->centre = rotatedposition;
        resourceVolume->radius = radius;
        resourceVolume->number = numAsteroids;
        resourceVolume->actualnumber = numAsteroids;
        resourceVolume->attributes = attributes;
        resourceVolume->attributesParam = (sword)attributesParam;
        resourceVolume->resourceObjType = resourceobjtype;
        GetResourceDistribution(&resourceVolume->resourceDistribution,directory,distributionstr,resourceobjtype);

        AddSphere(resourceVolume);
    }
    else if (strcmp(layoutstr,"Cylinder") == 0)
    {
        resourceVolume = memAlloc(sizeof(ResourceVolume),"ResourceCyl",NonVolatile);

        resourceVolume->resourceVolumeType = ResourceCylinderType;
        resourceVolume->centre = rotatedposition;
        resourceVolume->radius = radius;
        resourceVolume->length = length;
        resourceVolume->roty = DEG_TO_RAD(roty);
        resourceVolume->rotz = rotz;
        resourceVolume->number = numAsteroids;
        resourceVolume->actualnumber = numAsteroids;
        resourceVolume->attributes = attributes;
        resourceVolume->attributesParam = (sword)attributesParam;
        resourceVolume->resourceObjType = resourceobjtype;
        GetResourceDistribution(&resourceVolume->resourceDistribution,directory,distributionstr,resourceobjtype);

        AddCylinder(resourceVolume);
    }
    else if (strcmp(layoutstr,"Rectangle") == 0)
    {
        resourceVolume = memAlloc(sizeof(ResourceVolume),"ResourceRect",NonVolatile);

        resourceVolume->resourceVolumeType = ResourceRectangleType;
        resourceVolume->centre = rotatedposition;
        resourceVolume->radius = radius;
        resourceVolume->length = length;
        resourceVolume->roty = roty;
        resourceVolume->rotz = rotz;
        resourceVolume->number = numAsteroids;
        resourceVolume->actualnumber = numAsteroids;
        resourceVolume->attributes = attributes;
        resourceVolume->attributesParam = (sword)attributesParam;
        resourceVolume->resourceObjType = resourceobjtype;
        GetResourceDistribution(&resourceVolume->resourceDistribution,directory,distributionstr,resourceobjtype);

        AddRectangle(resourceVolume);
    }
    else if (strcmp(layoutstr,"Nebula") == 0)
    {
        vector dimension;

        dimension.x = radius;
        dimension.y = length;
//        dimension.z = 10000.0f;//DEG_TO_RAD(roty);
        dimension.z = roty;
        nebAttributes = attributes;
        nebGo(&rotatedposition, &dimension, /*(udword)rotz*/10000, numAsteroids);
        nebAttributes = 0;
    }
    else
    {
        dbgFatalf(DBG_Loc,"Unknown layout found in %s",field);
    }
}

static void scriptSetDerelictCB(char *directory,char *field,void *dataToFillIn)
{
    vector position;
    MissionSphereInfo *missionSphereInfo = (MissionSphereInfo *)dataToFillIn;
    real32 roty,rotz;
    DerelictType derelicttype;
    Derelict *derelict;
    char derelicttypestr[50];
    udword attributes = 0;
    sdword attributesParam = 0;
    sdword colorScheme = -1;
    char miscStr[50];

    matrix rotaboutz;
    vector rotatedposition;

    RemoveCommasFromString(field);

    miscStr[0] = 0;
    sscanf(field,"%s %f %f %f %f %f %d %d %s",derelicttypestr,&position.x,&position.y,&position.z,&roty,&rotz,&attributes,&attributesParam,miscStr);
    dbgAssert(attributesParam <= SWORD_Max && attributesParam >= SWORD_Min);
    roty = -DEG_TO_RAD(roty);
    rotz = DEG_TO_RAD(rotz);

    derelicttype = StrToDerelictType(derelicttypestr);

    if (!((derelicttype >= 0) && (derelicttype <= NUM_DERELICTTYPES)))
    {
        dbgFatalf(DBG_Loc,"Bad derelict type in %s",field);
    }

    if (!missionman)
    {
        matMakeRotAboutZ(&rotaboutz,(real32)cos(missionSphereInfo->sphererotz),(real32)sin(missionSphereInfo->sphererotz));

        matMultiplyMatByVec(&rotatedposition,&rotaboutz,&position);
        vecAddTo(rotatedposition,missionSphereInfo->position);

        rotz += missionSphereInfo->sphererotz;
    }
    else
    {
        rotatedposition = position;
    }

    derelict = univAddDerelict(derelicttype,&rotatedposition);
    derelict->attributes = attributes;
    derelict->attributesParam = (sword)attributesParam;
    if (attributes & ATTRIBUTES_Anomaly)
    {
        pingAnomalyObjectPingAdd("Anomaly",(SpaceObj *)derelict);
    }
    //hard coded scaffold spinning
//removed and replaced by the kas function RotateDerelictType
/*    if (singlePlayerGame)
    {
        switch (derelict->derelicttype)
        {
            case Scaffold_scarred:
                derelict->rotinfo.rotspeed.x = SCAFFOLD_SCARRED_ROTX;
                derelict->rotinfo.rotspeed.y = SCAFFOLD_SCARRED_ROTY;
                derelict->rotinfo.rotspeed.z = SCAFFOLD_SCARRED_ROTZ;
                break;
            case ScaffoldFingerA_scarred:
                derelict->rotinfo.rotspeed.x = SCAFFOLDFINGERA_SCARRED_ROTX;
                derelict->rotinfo.rotspeed.y = SCAFFOLDFINGERA_SCARRED_ROTY;
                derelict->rotinfo.rotspeed.z = SCAFFOLDFINGERA_SCARRED_ROTZ;
                break;
            case ScaffoldFingerB_scarred:
                derelict->rotinfo.rotspeed.x = SCAFFOLDFINGERB_SCARRED_ROTX;
                derelict->rotinfo.rotspeed.y = SCAFFOLDFINGERB_SCARRED_ROTY;
                derelict->rotinfo.rotspeed.z = SCAFFOLDFINGERB_SCARRED_ROTZ;
                break;
        }

    }*/
    univRotateObjPitch((SpaceObjRot *)derelict,roty);
    univRotateObjYaw((SpaceObjRot *)derelict,rotz);

    //parse the color scheme, if there was one
    if (strstr(miscStr, "colorScheme") == miscStr)
    {
        sscanf(miscStr, "colorScheme(%d)", &colorScheme);
        dbgAssert(colorScheme >= 0 && colorScheme < MAX_MULTIPLAYER_PLAYERS);
        derelict->colorScheme = colorScheme;
    }
}

static char llMiscDelimiters[] = "|?,";

ShipType GetAppropriateShipTypeForRace(ShipType request,ShipRace shiprace)
{
    ShipType shiptype = request;

    dbgAssert((shiprace == R1) || (shiprace == R2));

    if ((RacesAllowedForGivenShip[shiptype] & RaceToRaceBits(shiprace)) == 0)
    {
        // this race/shiptype isn't allowed so substitute:
        switch (shiptype)
        {
            case CloakedFighter:
                shiptype = DefenseFighter;
                break;

            case DefenseFighter:
                shiptype = CloakedFighter;
                break;

            case DDDFrigate:
                shiptype = DFGFrigate;
                break;

            case DFGFrigate:
                shiptype = DDDFrigate;
                break;
        }
        dbgAssert(RacesAllowedForGivenShip[shiptype] & RaceToRaceBits(shiprace));
    }

    return shiptype;
}

#define SetShipColorScheme(cs)                      \
    if (cs != -1)                                   \
    {                                               \
        ship->colorScheme = cs;                     \
    }                                               \
    if ((ship->colorScheme < 0) || (ship->colorScheme >= MAX_MULTIPLAYER_PLAYERS))      \
    {                                                                                   \
        ship->colorScheme = (MAX_MULTIPLAYER_PLAYERS-1);                                \
    }

bool isCapturableCapitalShip(ShipType shiptype)
{
    switch (shiptype)
    {
        case AdvanceSupportFrigate:
        case CloakGenerator       :
        case DDDFrigate           :
        case DFGFrigate           :
        case GravWellGenerator    :
        case HeavyCruiser         :
        case IonCannonFrigate     :
        case MissileDestroyer     :
        case ResourceController   :
        case StandardDestroyer    :
        case StandardFrigate      :
            return TRUE;
    }
    return FALSE;
}

bool PlayerDoesntHaveCapturableCapitalShip(Player *player)
{
    Node *node;
    Ship *ship;

    for (node=universe.ShipList.head;node != NULL;node=node->next)
    {
        ship = (Ship *)listGetStructOfNode(node);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            if (isCapturableCapitalShip(ship->shiptype))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

void GivePlayerCapitalShip(Player *player)
{
    ShipType shiptype = CapturableCapitalShipToGive;
    ShipRace shiprace = player->race;
    Ship *ship;

    if (player->PlayerMothership)
    {
        ShipStaticInfo *testStatic = GetShipStaticInfo(shiptype,shiprace);
        dbgAssert(bitTest(testStatic->staticheader.infoFlags, IF_InfoLoaded));  // we want it to crash if ship not loaded
        {
            vector zerovector = { 0.0f, 0.0f, 0.0f };
            ship = univAddShip(shiptype,shiprace,&zerovector,player,0);
            gameStatsAddShip(ship,player->playerIndex);

            // Update ship totals for player
            unitCapCreateShip(ship,player);

            if(ship->shiptype == ResearchShip &&
               player->shiptotals[ResearchShip] >1)
            {
                addMonkeyResearchShip(ship);
            }
            else
            {
                GroupShipIntoMilitaryParade(&universe.mainCommandLayer,ship,player->PlayerMothership);
            }
        }
    }
}

static void scriptSetShipsCB(char *directory,char *field,void *dataToFillIn)
{
    vector position;
    real32 rot;
    char racestr[50];
    char shiptypestr[50];
//    char formationstr[50];
    char miscstr[256];
    char *miscPointer;
    char label[KAS_TEAM_NAME_MAX_LENGTH+1];
    udword numShips;
    udword numShipsUpper = 0;
    udword i;
    MissionSphereInfo *missionSphereInfo = (MissionSphereInfo *)dataToFillIn;
    Ship *ship;
    Player *player = &universe.players[missionSphereInfo->playerNumber];
    bool paradeFormation = FALSE;
    bool useAsMothership = FALSE;
    AITeam *teamp = NULL;
    udword fieldStart;
    sdword colorScheme = -1;
    udword attributes = 0;
    sdword attributesParam = 0;

    ShipRace shiprace;
    ShipType shiptype;

    udword sizeofselect;
    SelectCommand *selectcom;
    TypeOfFormation formation = 0xffffffff;

    matrix rotaboutz;
    vector rotatedposition;

    if (field[0] == '?') field++;

    RemoveCommasFromString(field);

    // grab optional label
    label[0] = 0;
    fieldStart = 0;
    while (field[fieldStart] && isspace(field[fieldStart]) && field[fieldStart] == '?')
        ++fieldStart;
    if (toupper(field[fieldStart]) >= 'A' && toupper(field[fieldStart]) <= 'Z')
    {
        if (sscanf(field, "%s", label))
            fieldStart += strlen(label);
        // assign ships to computer player (#1 in single player game)
        player = &universe.players[1];

        // if labelled team doesn't currently exist, create it
        // otherwise, add ships to existing labelled team
        for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
        {
            teamp = aiCurrentAIPlayer->teams[i];
            if (teamp->teamType == ScriptTeam && !strncmp(teamp->kasLabel, label, KAS_TEAM_NAME_MAX_LENGTH))
                break;
        }
        if (i >= aiCurrentAIPlayer->teamsUsed)
        {
            teamp = aitCreate(ScriptTeam);
            memStrncpy(teamp->kasLabel, label, KAS_TEAM_NAME_MAX_LENGTH);
            // initialize original team size & type (to be updated later)
            teamp->kasOrigShipsCount = 0;
            teamp->kasOrigShipsType = LightInterceptor;  // a default, just in case
        }
    }

    miscstr[0] = 0;
    sscanf(field + fieldStart,"%f %f %f %f %s %s %d %s %d %d %d",&position.x,&position.y,&position.z,&rot,racestr,shiptypestr,&numShips,miscstr,&attributes,&attributesParam,&numShipsUpper);
    dbgAssert(attributesParam <= SWORD_Max && attributesParam >= SWORD_Min);
    rot = DEG_TO_RAD(rot);

    if (!singlePlayerGame)
    {
        numShipsUpper = 0;
    }
    else
    {
        if ((numShipsUpper) && (spFleetModifier))
        {
            // set numShips to be between numShips and numShipsUpper, based on spFleetModifier (0 to 1)
            real32 diff = (real32) (numShipsUpper - numShips);

            if (numShipsUpper < numShips)
                dbgFatalf(DBG_Loc,"Error numShipsUpper %d too low on line %s",numShipsUpper,field);
            else if (numShipsUpper > numShips)
            {
                diff *= spFleetModifier;

                numShips += (udword)(diff + 0.5f);
            }
        }
    }

    shiprace = StrToShipRace(racestr);
    if (!((shiprace >= 0) && (shiprace < NUM_RACES)))
    {
        dbgFatalf(DBG_Loc,"Bad shiprace in %s",field);
    }
    dbgAssert(shiprace != -1);

    shiptype = StrToShipType(shiptypestr);
    if (!((shiptype >= 0) && (shiptype < TOTAL_NUM_SHIPS)))
    {
        dbgFatalf(DBG_Loc,"Bad shiptype in %s",field);
    }
    dbgAssert(shiptype != -1);

    if (singlePlayerGame && (shiptype == CryoTray))
    {
        //cryo trays are owned by the player
        player = &universe.players[0];
    }

    if (!singlePlayerGame)
    {
        if ((!bitTest(tpGameCreated.flag,MG_HarvestinEnabled)) &&
            (shiptype==ResourceCollector))
        {
            return;
        }

        if ((!bitTest(tpGameCreated.flag,MG_ResearchEnabled)) &&
            (shiptype==ResearchShip))
        {
            return;
        }

        if ((bitTest(tpGameCreated.flag,MG_CarrierIsMothership)) &&
            (shiptype == Mothership))
        {
            shiptype = Carrier;
            useAsMothership = TRUE;
        }
    }

    if ((missionman) && ((shiprace == R1) || (shiprace == R2)))
    {
        if ((player->race != shiprace) && ((player->race == R1) || (player->race == R2)))
        {
            // override shiprace and shiptype based on player's race:
            shiprace = player->race;
            shiptype = GetAppropriateShipTypeForRace(shiptype,shiprace);
        }
    }
/*
    if (strcmp(formationstr,"PARADE_FORMATION") == 0)
    {
        paradeFormation = TRUE;
    }
*/
    //parse everything else to the end of the line including formations
    miscPointer = strtok(miscstr, llMiscDelimiters);
    if (miscPointer != NULL)
    {
        do
        {
            if (strcmp(miscPointer,"PARADE_FORMATION") == 0)
            {
                paradeFormation = TRUE;
            }
            else if (strcasecmp(miscPointer,"UseAsMothership") == 0)
            {
                useAsMothership = TRUE;
            }
            else if (strstr(miscPointer, "colorScheme") == miscPointer)
            {
                sscanf(miscPointer, "colorScheme(%d)", &colorScheme);
                dbgAssert(colorScheme >= 0 && colorScheme < MAX_MULTIPLAYER_PLAYERS);
            }
            else if ((miscPointer[0] != '0') && (miscPointer[0] != '?'))
            {
                formation = StrToTypeOfFormation(miscPointer);
            }
        }
        while ((miscPointer = strtok(NULL, llMiscDelimiters)) != NULL);
    }

    if (!missionman)
    {
        matMakeRotAboutZ(&rotaboutz,(real32)cos(missionSphereInfo->sphererotz),(real32)sin(missionSphereInfo->sphererotz));
    }

    if (teamp)
    {
        // if we have a team of mixed ship types, this will obviously be a bit screwed up, but it's not critical
        teamp->kasOrigShipsCount += numShips;
        teamp->kasOrigShipsType = shiptype;
    }

    if (numShips == 1)
    {
        if (!missionman)
        {
            matMultiplyMatByVec(&rotatedposition,&rotaboutz,&position);
            vecAddTo(rotatedposition,missionSphereInfo->position);

            rot += missionSphereInfo->sphererotz;
        }
        else
        {
            rotatedposition = position;
        }

        if (shiptype == MiningBase)
        {
            sdword slab = 0;
            real32 roll = 0.0f;
            ShipStaticInfo *testStatic = GetShipStaticInfo(shiptype,shiprace);
            dbgAssert(bitTest(testStatic->staticheader.infoFlags, IF_InfoLoaded)); // we want it to crash if ship not loaded
            {
                //if info for this ship isn't loaded, we'll skip trying to add it
                for (slab=0;slab<8;slab++,roll+=(TWOPI/8))
                {
                    ship = univAddShip(shiptype,shiprace,&rotatedposition,player,0);
                    gameStatsAddShip(ship,player->playerIndex);
                    ship->attributes = attributes;
                    ship->attributesParam = (sword)attributesParam;
                    ship->rotinfo.rotspeed.z = MINING_BASE_ROTATION_SPEED;
                    SetShipColorScheme(colorScheme);
                    univRotateObjYaw((SpaceObjRot *)ship,rot);
                    univRotateObjRoll((SpaceObjRot *)ship,roll);

                    // Update ship totals for player
                    unitCapCreateShip(ship,player);

                    // add ship to team, if applicable
                    if (teamp)
                    {
                        aitAddShip(teamp, ship);
                        teamp->newships--;
                    }
                }
            }
        }
        else
        {
            {
                ShipStaticInfo *testStatic = GetShipStaticInfo(shiptype,shiprace);
                dbgAssert(bitTest(testStatic->staticheader.infoFlags, IF_InfoLoaded)); // we want it to crash if ship not loaded
                {
                    ship = univAddShip(shiptype,shiprace,&rotatedposition,player,0);
                    gameStatsAddShip(ship,player->playerIndex);
                    ship->attributes = attributes;
                    ship->attributesParam = (sword)attributesParam;
                    SetShipColorScheme(colorScheme);
                    univRotateObjYaw((SpaceObjRot *)ship,rot);

                    // Update ship totals for player
                    unitCapCreateShip(ship,player);
                                        // add ship to team, if applicable
                    if (teamp)
                    {
                        aitAddShip(teamp, ship);
                        teamp->newships--;
                    }

                    if(ship->shiptype == ResearchShip &&
                       player->shiptotals[ResearchShip] >1)
                    {
                        addMonkeyResearchShip(ship);
                    }
                }
            }
        }


        // Update research manager ship status

        // ****** IMPORTANT we have changed the research so that it is deterministic on all machines.
        if (shiptype==ResearchShip)
        {
/*            if ( (player==universe.curPlayerPtr) ||
                 (player->aiPlayer!=NULL) )
            {*/
                rmActivateFreeLab(player);
//            }
        }

        if ((shiptype == Mothership) || (useAsMothership))
        {
            player->PlayerMothership = ship;
        }
        else if (paradeFormation)
        {
            if (player->PlayerMothership == NULL)
            {
                dbgFatal(DBG_Loc,"Mothership must be defined for parade formation");
            }

            GroupShipIntoMilitaryParade(&universe.mainCommandLayer,ship,player->PlayerMothership);
        }
    }
    else if (numShips > 1)
    {
        if (paradeFormation)
        {
            if (player->PlayerMothership == NULL)
            {
                dbgFatal(DBG_Loc,"Mothership must be defined for parade formation");
            }

            if (!missionman)
            {
                matMultiplyMatByVec(&rotatedposition,&rotaboutz,&position);
                vecAddTo(rotatedposition,missionSphereInfo->position);
            }
            else
            {
                rotatedposition = position;
            }

            for (i=0;i<numShips;i++)
            {
                ShipStaticInfo *testStatic = GetShipStaticInfo(shiptype,shiprace);
                dbgAssert(bitTest(testStatic->staticheader.infoFlags, IF_InfoLoaded)); // we want it to crash if ship not loaded
                {
                    ship = univAddShip(shiptype,shiprace,&rotatedposition,player,0);
                    gameStatsAddShip(ship,player->playerIndex);
                    ship->attributes = attributes;
                    ship->attributesParam = (sword)attributesParam;
                    SetShipColorScheme(colorScheme);

                    // Update ship totals for player
                    unitCapCreateShip(ship,player);

                    // add ship to team, if applicable
                    if (teamp)
                    {
                        aitAddShip(teamp, ship);
                        teamp->newships--;
                    }

                    // Update research manager ship status

                    // ****** IMPORTANT we have changed the research so that it is deterministic on all machines.
                    if (shiptype==ResearchShip)
                    {
/*                        if ( (player==universe.curPlayerPtr) ||
                             (player->aiPlayer!=NULL) )
                        {*/
                            rmActivateFreeLab(player);
//                        }
                    }

                    if(ship->shiptype == ResearchShip &&
                       player->shiptotals[ResearchShip] >1)
                    {
                        addMonkeyResearchShip(ship);
                    }
                    else
                    {
                        GroupShipIntoMilitaryParade(&universe.mainCommandLayer,ship,player->PlayerMothership);
                    }
                }
            }
        }
        else
        {
            sizeofselect = sizeofSelectCommand(numShips);
            selectcom = memAlloc(sizeofselect,"selectform",0);

            selectcom->numShips = numShips;

            if (!missionman)
            {
                matMultiplyMatByVec(&rotatedposition,&rotaboutz,&position);
                vecAddTo(rotatedposition,missionSphereInfo->position);

                rot += missionSphereInfo->sphererotz;
            }
            else
            {
                rotatedposition = position;
            }

            for (i=0;i<numShips;i++)
            {
                ShipStaticInfo *testStatic = GetShipStaticInfo(shiptype,shiprace);
                dbgAssert(bitTest(testStatic->staticheader.infoFlags, IF_InfoLoaded));  // we want it to crash if ship not loaded
                {
                    ship = univAddShip(shiptype,shiprace,&rotatedposition,player,0);
                    gameStatsAddShip(ship,player->playerIndex);
                    ship->attributes = attributes;
                    ship->attributesParam = (sword)attributesParam;
                    SetShipColorScheme(colorScheme);
                    univRotateObjYaw((SpaceObjRot *)ship,rot);

                    // Update ship totals for player
                    unitCapCreateShip(ship,player);

                    // add ship to team, if applicable
                    if (teamp)
                    {
                        aitAddShip(teamp, ship);
                        teamp->newships--;
                    }

                    // Update research manager ship status

                    // ****** IMPORTANT we have changed the research so that it is deterministic on all machines.
                    if (shiptype==ResearchShip)
                    {
/*                        if ( (player==universe.curPlayerPtr) ||
                             (player->aiPlayer!=NULL) )
                        {*/
                            rmActivateFreeLab(player);
//                        }
                    }


                    selectcom->ShipPtr[i] = ship;

                    if(ship->shiptype == ResearchShip &&
                       player->shiptotals[ResearchShip] >1)
                    {
                        addMonkeyResearchShip(ship);
                    }
                    else
                    {
                        rotatedposition.z += (ship->staticinfo->staticheader.staticCollInfo.collspheresize * 2.0f);
                    }
                }
            }

            if (formation != -1)
            {
                clFormation(&universe.mainCommandLayer,selectcom,formation);
            }

            memFree(selectcom);
        }
    }
}

static void scriptSetAIPointCB(char *directory,char *field,void *dataToFillIn)
{
    char label[KAS_MAX_LABEL_LENGTH+1];
    real32 x, y, z, w;

    w = 0.0f;
    RemoveCommasFromString(field);

    if (sscanf(field, "%s %f %f %f %f", label, &x, &y, &z, &w) < 4)
        dbgFatalf(DBG_Loc,"Bad AIPoint definition: %s",field);

    kasLabelledVectorAdd(label, x, y, z, w);
    aiplayerLog((0,"AIPoint %s added: %f,%f,%f,%f", label, x, y, z, w));
}

static void scriptSetAIPathCB(char *directory,char *field,void *dataToFillIn)
{
    char label[KAS_MAX_LABEL_LENGTH+1];
    char openClosed[KAS_MAX_LABEL_LENGTH+1];
    sdword closed;
    sdword pointNum, numPoints;
    Path *path;
    vector point;

    RemoveCommasFromString(field);

    if (sscanf(field, "%s %f %f %f %s %d/%d",
            label, &(point.x), &(point.y), &(point.z), openClosed, &pointNum, &numPoints) != 7)
        dbgFatalf(DBG_Loc,"Bad AIPath point definition: %s",field);

    closed = !strcasecmp(openClosed, "closed");

    path = kasPathPtrNoErrorChecking(label);
    if (path == NULL)
    {
        path = kasLabelledPathAdd(label, numPoints, closed);
        aiplayerLog((0,"AIPath %s added", label));
    }

    // add point
    aiuAddPointToPath(point, pointNum-1, path);
    aiplayerLog((0,"AIPath %s point %d/%d added: %f,%f,%f", label, pointNum, numPoints, point.x, point.y, point.z));
}

static void scriptSetAIBoxCB(char *directory,char *field,void *dataToFillIn)
{
    char label[KAS_MAX_LABEL_LENGTH+1];
    real32 x, y, z, width, height, depth;
    Volume *volume;

    RemoveCommasFromString(field);

    if (sscanf(field, "%s %f %f %f %f %f %f", label, &x, &y, &z, &width, &height, &depth) != 7)
        dbgFatalf(DBG_Loc,"Bad AIBox definition: %s",field);

    volume = kasLabelledVolumeAdd(label);
    volume->type = VOLUME_AA_BOX;
    volume->attribs.aaBox.x0 = x - width/2;
    volume->attribs.aaBox.x1 = x + width/2;
    volume->attribs.aaBox.y0 = y - height/2;
    volume->attribs.aaBox.y1 = y + height/2;
    volume->attribs.aaBox.z0 = z - depth/2;
    volume->attribs.aaBox.z1 = z + depth/2;
    aiplayerLog((0,"AIBox %s added", label));
}

static void scriptSetAISphereCB(char *directory,char *field,void *dataToFillIn)
{
    char label[KAS_MAX_LABEL_LENGTH+1];
    real32 x, y, z, diameter;
    Volume *volume;

    RemoveCommasFromString(field);

    if (sscanf(field, "%s %f %f %f %f", label, &x, &y, &z, &diameter) != 5)
        dbgFatalf(DBG_Loc,"Bad AISphere definition: %s",field);

    volume = kasLabelledVolumeAdd(label);
    volume->type = VOLUME_SPHERE;
    volume->attribs.sphere.center.x = x;
    volume->attribs.sphere.center.y = y;
    volume->attribs.sphere.center.z = z;
    volume->attribs.sphere.radius = diameter/2;
    aiplayerLog((0,"AISphere %s added", label));
}

static void scriptSetLightingCB(char *directory,char *field,void *dataToFillIn)
{
    char lightingFileName[50];

    RemoveCommasFromString(field);
    sscanf(field, "%s", lightingFileName);
    if (strlen(field) > 1)
    {
        char fullFileName[PATH_MAX];

        strcpy(fullFileName, directory);
        strcat(fullFileName, lightingFileName);
        if (strchr(lightingFileName, '.') == NULL)
        {
            strcat(fullFileName, ".hsf");
        }

        if (fileExists(fullFileName,0))
        {
            lightParseHSF(fullFileName);
        }
        else
        {
#ifdef _WIN32
            strcpy(fullFileName, "HSF\\");
#else
            strcpy(fullFileName, "HSF/");
#endif
            strcat(fullFileName, lightingFileName);
            if (strchr(lightingFileName, '.') == NULL)
            {
                strcat(fullFileName, ".hsf");
            }

            if (fileExists(fullFileName,0))
            {
                lightParseHSF(fullFileName);
            }
            else
            {
                lightDefaultLightSet();
            }
        }
    }
    else
    {
        lightDefaultLightSet();
    }
}

static void scriptSetBackgroundCB(char* directory, char* field, void* dataToFillIn)
{
    char btgFileName[50];
    real32 theta = 0.0f,phi = 0.0f;

    RemoveCommasFromString(field);
    sscanf(field, "%s %f %f", btgFileName,&theta,&phi);
    btgSetTheta(theta);
    btgSetPhi(phi);
    if (strlen(field) > 1)
    {
        char fullFileName[PATH_MAX];

        // first try current directory:

        strcpy(fullFileName, directory);
        strcat(fullFileName, btgFileName);
        if (strchr(btgFileName, '.') == NULL)
        {
            strcat(fullFileName, ".btg");
        }

        if (fileExists(fullFileName,0))
        {
            btgLoad(fullFileName);
        }
        else
        {
#ifdef _WIN32
            strcpy(fullFileName, "BTG\\");
#else
            strcpy(fullFileName, "BTG/");
#endif
            strcat(fullFileName, btgFileName);
            if (strchr(btgFileName, '.') == NULL)
            {
                strcat(fullFileName, ".btg");
            }

            if (fileExists(fullFileName,0))
            {
                btgLoad(fullFileName);
            }
            else
            {
#ifdef _WIN32
                btgLoad("BTG\\default.btg");
#else
                btgLoad("BTG/default.btg");
#endif
            }
        }
    }
    else
    {
#ifdef _WIN32
        btgLoad("BTG\\default.btg");
#else
        btgLoad("BTG/default.btg");
#endif
    }
}

#if 0
static void scriptSetBackgroundThetaCB(char* directory, char* field, void* dataToFillIn)
{
    real32 btgTheta;

    RemoveCommasFromString(field);
    sscanf(field, "%f", &btgTheta);
    btgSetTheta(btgTheta);
}

static void scriptSetBackgroundPhiCB(char* directory, char* field, void* dataToFillIn)
{
    real32 btgPhi;

    RemoveCommasFromString(field);
    sscanf(field, "%f", &btgPhi);
    btgSetPhi(btgPhi);
}
#endif

static void scriptSetMissionSphereCB(char *directory,char *field,void *dataToFillIn)
{
    sdword playerNumber;
    char raceName[50];
    ShipRace shiprace;
    vector position;
    real32 radius;
    char contents[50];
    udword startingResourceUnits;
    Player *player;
    MissionSphereInfo missionSphereInfo;
    real32 sphererotz = 0.0f;

    RemoveCommasFromString(field);

    if (missionman)
    {
        char dummystr[50];
        sscanf(field,"%d %s %s %f %f %f %f %f %s %d",&playerNumber,raceName,dummystr,&position.x,&position.y,&position.z,&radius,
                                                  &sphererotz,contents,&startingResourceUnits);
    }
    else
    {
        sscanf(field,"%d %s %f %f %f %f %f %s %d",&playerNumber,raceName,&position.x,&position.y,&position.z,&radius,
                                                  &sphererotz,contents,&startingResourceUnits);
    }

    dbgAssert(playerNumber < numPlayers);
    shiprace = StrToShipRace(raceName);

    if ((playerNumber >= 0) && (!(tutorial==TUTORIAL_ONLY)))
    {
        if (!((shiprace >= 0) && (shiprace < NUM_RACES)))
        {
            dbgFatalf(DBG_Loc,"Bad shiprace in %s",field);
        }
        if (!missionman)
        {
            if (shiprace != universe.players[playerNumber].race)
            {
                return;     // ignore, until we find correct race for player
            }
        }
        else
        {
            if ((shiprace != R1) && (shiprace != R2))
            {
                // specified pirates, so let's give it to them:
                universe.players[playerNumber].race = shiprace;
            }
        }
    }

    if ((singlePlayerGame) && (!(tutorial==TUTORIAL_ONLY)))
    {
        missionSphereInfo.playerNumber = 1;
    }
    else
    {
        missionSphereInfo.playerNumber = playerNumber;
    }
    missionSphereInfo.position = position;

    if (sphererotz >= 1000.0)
    {
        missionSphereInfo.sphererotz = (real32)atan2((double)position.y,(double)position.x) - (PI/2.0f);
    }
    else
    {
        missionSphereInfo.sphererotz = DEG_TO_RAD(sphererotz);
    }

    if (playerNumber >= 0)
    {
        // this is a player's mission sphere

        player = &universe.players[playerNumber];

        player->resourceUnits = startingResourceUnits;
        player->playerState = PLAYER_ALIVE;
        universe.gameStats.playerStats[player->playerIndex].totalResourceUnits += startingResourceUnits;

        universeSwitchToPlayer((uword)playerNumber);
    }

    if (missionSphereInfo.playerNumber < 0)
    {
        missionSphereInfo.playerNumber = MAX_MULTIPLAYER_PLAYERS;       // misc player for autoguns, etc.
    }

    scriptSetStruct(directory,contents,MissionSphereScriptTable,(ubyte *)&missionSphereInfo);

    if ((playerNumber >= 0) && (playerNumber < universe.numPlayers))
    {
        if ((!singlePlayerGame) && (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip)))
        {
            player = &universe.players[playerNumber];
            if (PlayerDoesntHaveCapturableCapitalShip(player)) GivePlayerCapitalShip(player);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : scriptPreMissionSphereCB
    Description : Level preload callback to scan the mission sphere and flag
                    all ships therein for loading.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void scriptPreMissionSphereCB(char *directory,char *field,void *dataToFillIn)
{
    sdword playerNumber;
    char raceName[50];
    vector position;
    real32 radius;
    char contents[50];
    udword startingResourceUnits;
    real32 sphererotz = 0.0f;
    MissionSphereInfo missionSphereInfo;
    ShipRace shiprace;

    RemoveCommasFromString(field);

    if (missionman)
    {
        char dummystr[50];
        sscanf(field,"%d %s %s %f %f %f %f %f %s %d",&playerNumber,raceName,dummystr,&position.x,&position.y,&position.z,&radius,
                                                  &sphererotz,contents,&startingResourceUnits);
    }
    else
    {
        sscanf(field,"%d %s %f %f %f %f %f %s %d",&playerNumber,raceName,&position.x,&position.y,&position.z,&radius,
                                                  &sphererotz,contents,&startingResourceUnits);
    }

    dbgAssert(playerNumber < numPlayers);
    shiprace = StrToShipRace(raceName);

    if ((singlePlayerGame) && (!(tutorial==TUTORIAL_ONLY)))
    {
        missionSphereInfo.playerNumber = 1;
    }
    else
    {
        missionSphereInfo.playerNumber = playerNumber;
    }
    missionSphereInfo.position = position;

    if (sphererotz >= 1000.0)
    {
        missionSphereInfo.sphererotz = (real32)atan2((double)position.y,(double)position.x) - (PI/2.0f);
    }
    else
    {
        missionSphereInfo.sphererotz = DEG_TO_RAD(sphererotz);
    }

    scriptSetStruct(directory,contents,MissionPreloadMissphereTable,(ubyte *)&missionSphereInfo);

    if (singlePlayerGame)
    {
        bitSet(derelictStaticInfos[HyperspaceGate].staticheader.infoFlags, IF_InfoNeeded);
    }
}

#define MAXLINENEED 50
bool isLevelMissionManGenerated(char *directory,char *filename)
{
    filehandle fh;
    char line[MAXLINENEED];
    char fullfilename[80];
    sdword status;

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

    status = fileLineRead(fh,line,MAXLINENEED);

    fileClose(fh);

    if (status == FR_EndOfFile)
    {
        return FALSE;
    }

    if (strncmp("; MissionMan",line,12) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

ShipRace GetSinglePlayerRaceEquivalent(ShipRace race)
{
    if (race == R1)
    {
        return universe.players[0].race;
    }
    else if (race == R2)
    {
        return ((universe.players[0].race == R1) ? R2 : R1);
    }
    else
    {
        return race;
    }
}

/*-----------------------------------------------------------------------------
    Name        : SetInfoNeededForShipAndRelatedStaticInfo
    Description : Sets InfoNeeded for a ship and also for its related components (missiles, etc, if it has any)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SetInfoNeededForShipAndRelatedStaticInfo(ShipType type,ShipRace race,bool8 dataToFillIn)
{
    ShipStaticInfo *info = GetShipStaticInfo(type, race);
    bitSetTo(info->staticheader.infoFlags, IF_InfoNeeded, dataToFillIn);

    if (type == MissileDestroyer || type == P1MissileCorvette)
    {
        bitSetTo(missileStaticInfos[race].staticheader.infoFlags, IF_InfoNeeded, dataToFillIn);
    }
    else if (type == MinelayerCorvette)
    {
        bitSetTo(mineStaticInfos[race].staticheader.infoFlags, IF_InfoNeeded, dataToFillIn);
    }
    else if (race == R1 && type == DDDFrigate)
    {
        ShipStaticInfo *shipstatic;
        shipstatic = GetShipStaticInfo(Drone,R1);
        bitSetTo(shipstatic->staticheader.infoFlags, IF_InfoNeeded, dataToFillIn);
    }
}

/*-----------------------------------------------------------------------------
    Name        : SetTeamColorForShipAndRelatedStaticInfo
    Description : Sets teamcolor for ship staticinfo and its related components (missiles, etc)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SetTeamColorForShipAndRelatedStaticInfo(ShipType type,ShipRace race,bool8 schemes[])
{
    ShipStaticInfo *shipstaticinfo;

    if ((RacesAllowedForGivenShip[type] & RaceToRaceBits(race)) == 0)
    {
        dbgMessagef("Warning: not setting team color for type %d race %d",type,race);
        return;
    }

    shipstaticinfo = GetShipStaticInfo(type,race);
    memcpy(shipstaticinfo->teamColor, schemes, sizeof(shipstaticinfo->teamColor));
    if (type == MissileDestroyer || type == P1MissileCorvette)
    {
        memcpy(missileStaticInfos[race].teamColor, schemes, sizeof(missileStaticInfos[race].teamColor));
    }
    else if (type == MinelayerCorvette)
    {
        memcpy(mineStaticInfos[race].teamColor, schemes, sizeof(mineStaticInfos[race].teamColor));
    }
    else if (type == DDDFrigate)
    {
        ShipStaticInfo *ss;
        dbgAssert(race == R1);
        ss = GetShipStaticInfo(Drone,R1);
        memcpy(ss->teamColor, schemes, sizeof(ss->teamColor));
    }
}

/*-----------------------------------------------------------------------------
    Callbacks for parsing the pre-load of the .level file
-----------------------------------------------------------------------------*/
char llDelimiters[] = " ,()\t[]";
/*-----------------------------------------------------------------------------
    Name        : llExcludeShip
    Description : Include or exclude a ship from being loaded next frame.
    Inputs      : field - <race>, <shiptype|"all">
                  dataToFillIn - 0 = exclude, 1 = include
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void llExcludeShip(char *directory,char *field,void *dataToFillIn)
{
    char *string;
    ShipRace race;
    ShipType type;

    //parse the race
    string = strtok(field, llDelimiters);
    dbgAssert(string != NULL);
    race = StrToShipRace(string);
    dbgAssert(race != 0xffffffff);
    if (singlePlayerGame || tutorial)
    {
        race = GetSinglePlayerRaceEquivalent(race);
    }

    //parse the ship type
    string = strtok(NULL, llDelimiters);
    dbgAssert(string != NULL);
    if (strcasecmp(string, "ALL") == 0)
    {                                                       //if we should do this to all ships for this race
        universeFlagRaceNeeded(race, (bool8)(size_t)dataToFillIn);
    }
    else
    {                                                       //else just a single ship
        type = StrToShipType(string);
        dbgAssert(type != 0xffffffff);
        SetInfoNeededForShipAndRelatedStaticInfo(type,race,(bool8)(size_t)dataToFillIn);
        rmEnableShip(race, type, (bool)dataToFillIn);
    }
}

/*-----------------------------------------------------------------------------
    Name        : llExclude<Asteroid|DustCloud|GasCloud|Nebula|Derelict>
    Description : Include or exclude loading of one of the above mentioned objects.
    Inputs      : field - type of object to include/exclude
                  dataToFillIn - 0 = exclude, 1 = include
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void llExcludeAsteroid(char *directory,char *field,void *dataToFillIn)
{
    AsteroidType type;

    type = StrToAsteroidType(field);
    if (type == 0xffffffff)
    {
        dbgMessagef("\nWarning: tried to exclude unknown asteroid");
        return;
    }

    bitSetTo(asteroidStaticInfos[type].staticheader.infoFlags, IF_InfoNeeded, (bool8)(size_t)dataToFillIn);
}
static void llExcludeDustCloud(char *directory,char *field,void *dataToFillIn)
{
    DustCloudType type;

    type = StrToDustCloudType(field);
    dbgAssert(type != 0xffffffff);

    bitSetTo(dustcloudStaticInfos[type].staticheader.infoFlags, IF_InfoNeeded, (bool8)(size_t)dataToFillIn);
}
static void llExcludeGasCloud(char *directory,char *field,void *dataToFillIn)
{
    GasCloudType type;

    type = StrToGasCloudType(field);
    dbgAssert(type != 0xffffffff);

    bitSetTo(gascloudStaticInfos[type].staticheader.infoFlags, IF_InfoNeeded, (bool8)(size_t)dataToFillIn);
}
static void llExcludeNebula(char *directory,char *field,void *dataToFillIn)
{
    NebulaType type;

    type = StrToNebulaType(field);
    dbgAssert(type != 0xffffffff);

    bitSetTo(nebulaStaticInfos[type].staticheader.infoFlags, IF_InfoNeeded, (bool8)(size_t)dataToFillIn);
}
static void llExcludeDerelict(char *directory,char *field,void *dataToFillIn)
{
    DerelictType type;

    if (strcasecmp(field, "ALL") == 0)
    {                                                       //if we should do this to all ships for this race
        for (type = 0; type < NUM_DERELICTTYPES; type++)
        {
            bitSetTo(derelictStaticInfos[type].staticheader.infoFlags, IF_InfoNeeded, (bool8)(size_t)dataToFillIn);
        }
    }
    else
    {
        type = StrToDerelictType(field);
        dbgAssert(type != 0xffffffff);
        bitSetTo(derelictStaticInfos[type].staticheader.infoFlags, IF_InfoNeeded, (bool8)(size_t)dataToFillIn);
    }
}
/*-----------------------------------------------------------------------------
    Name        : llAvailableColorScheme
    Description : Make a certain ship load with one or more specific color schemes.
    Inputs      : field - <race> <shiptype|"all"> <scheme#>,[<scheme#>[,<scheme#>]]
    Outputs     :
    Return      :
    Note        : overrides whatever default color schemes might have been coded
                    for this ship
----------------------------------------------------------------------------*/
static void llAvailableColorScheme(char *directory,char *field,void *dataToFillIn)
{
    char *string;
    ShipRace race;
    ShipType type;
    bool8 schemes[MAX_MULTIPLAYER_PLAYERS];
    ShipType firstshiptype;
    ShipType lastshiptype;
    sdword index;

    memset(schemes, 0, MAX_MULTIPLAYER_PLAYERS);
    //parse the race
    string = strtok(field, llDelimiters);
    dbgAssert(string != NULL);
    if (strcasecmp(string, "Derelicts") == 0)
    {                                                       //if color scheme for a derelict
        string = strtok(NULL, llDelimiters);
        dbgAssert(string != NULL);
        type = StrToDerelictType(string);                   //get derelict type
        dbgAssert(type != 0xffffffff);
        while ((string = strtok(NULL, llDelimiters)) != NULL)
        {                                                   //get list of color schemes
            sscanf(string, "%d", &index);
            dbgAssert(index >= 0 && index < MAX_MULTIPLAYER_PLAYERS);
            schemes[index] = TRUE;
        }                                                   //set the color scheme list
        memcpy(derelictStaticInfos[type].teamColor, schemes, sizeof(derelictStaticInfos[type].teamColor));
        return;                                             //done
    }
    race = StrToShipRace(string);
    dbgAssert(race != 0xffffffff);
    if (singlePlayerGame)
    {
        race = GetSinglePlayerRaceEquivalent(race);
    }

    //parse the ship type
    string = strtok(NULL, llDelimiters);
    dbgAssert(string != NULL);
    if (strcasecmp(string, "ALL") == 0)
    {                                                       //if we should do this to all ships for this race
        type = 0xffffffff;
    }
    else
    {                                                       //else just a single ship
        type = StrToShipType(string);
        dbgAssert(type != 0xffffffff);
    }
    while ((string = strtok(NULL, llDelimiters)) != NULL)
    {
        sscanf(string, "%d", &index);
        dbgAssert(index >= 0 && index < MAX_MULTIPLAYER_PLAYERS);
        schemes[index] = TRUE;
    }

    if (type == 0xffffffff)
    {                                                       //if "all"
        firstshiptype = FirstShipTypeOfRace[race];          //set color scheme for all ships in this race
        lastshiptype = LastShipTypeOfRace[race];
    }
    else
    {                                                       //else just the one ship
        firstshiptype = type;
        lastshiptype = type + 1;
    }
    for (type=firstshiptype;type<lastshiptype;type++)
    {                                                       //set the team colors for whatever ships we are to effect
        SetTeamColorForShipAndRelatedStaticInfo(type,race,schemes);
    }
}

/*-----------------------------------------------------------------------------
    Name        : scriptSetShipsToBeNeeded
    Description : Level preload callback that flags a ship for loading just by
                    being in the level.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void scriptSetShipsToBeNeeded(char *directory,char *field,void *dataToFillIn)
{
    MissionSphereInfo *missionSphereInfo = (MissionSphereInfo *)dataToFillIn;
    Player *player = &universe.players[missionSphereInfo->playerNumber];
    vector position;
    real32 rot;
    char racestr[50];
    char shiptypestr[50];
    ShipRace shiprace;
    ShipType shiptype;
    ShipStaticInfo *shipstaticinfo;
    sdword fieldStart;
    char label[KAS_TEAM_NAME_MAX_LENGTH+1];
    char miscstr[256];
    char *miscPointer;
    udword numShips, nScanned;
    sdword colorScheme = -1;
    bool8 teamCol[TE_NumberPlayers];

    if (field[0] == '?') field++;

    RemoveCommasFromString(field);

    // skip optional label
    fieldStart = 0;
    while (field[fieldStart] && isspace(field[fieldStart]) && field[fieldStart] == '?')
        ++fieldStart;
    if (toupper(field[fieldStart]) >= 'A' && toupper(field[fieldStart]) <= 'Z')
    {
        if (sscanf(field, "%s", label))
            fieldStart += strlen(label);
    }
    //sscanf(field + fieldStart,"%f %f %f %f %s %s",&position.x,&position.y,&position.z,&rot,racestr,shiptypestr);
    miscstr[0] = 0;
    nScanned = sscanf(field + fieldStart,"%f %f %f %f %s %s %d %s ",&position.x,&position.y,&position.z,&rot,racestr,shiptypestr,&numShips,miscstr);
    dbgAssert(nScanned >= 7);
    rot = DEG_TO_RAD(rot);

    shiprace = StrToShipRace(racestr);
    if (!((shiprace >= 0) && (shiprace < NUM_RACES)))
    {
        dbgFatalf(DBG_Loc,"Bad shiprace in %s",field);
    }
    dbgAssert(shiprace != -1);

    shiptype = StrToShipType(shiptypestr);
    if (!((shiptype >= 0) && (shiptype < TOTAL_NUM_SHIPS)))
    {
        dbgFatalf(DBG_Loc,"Bad shiptype in %s",field);
    }
    dbgAssert(shiptype != -1);

    if (singlePlayerGame)
    {
        if (shiptype != CryoTray)       // cryo trays are always "ours"
        if (universe.players[0].race == R2)
        {
            if ((shiprace == R1) || (shiprace == R2))
            {
                shiprace = (shiprace == R1) ? R2 : R1;
                shiptype = GetAppropriateShipTypeForRace(shiptype,shiprace);
            }
        }
    }

    //find what color scheme these ships will be
    miscPointer = strtok(miscstr, llMiscDelimiters);
    if (miscPointer != NULL)
    {
        do
        {
            if (strstr(miscPointer, "colorScheme") == miscPointer)
            {
                nScanned = sscanf(miscPointer, "colorScheme(%d)", &colorScheme);
                dbgAssert(colorScheme >= 0 && colorScheme < MAX_MULTIPLAYER_PLAYERS && nScanned == 1);
            }
        }
        while ((miscPointer = strtok(NULL, llMiscDelimiters)) != NULL);
    }

    shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
    if ((!singlePlayerGame) && (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip)) && (missionSphereInfo->playerNumber >= 0))
    if (isCapturableCapitalShip(shiptype))
    {
        if (player->race == R1)
            needR1CapitalShip = FALSE;
        else if (player->race == R2)
            needR2CapitalShip = FALSE;
    }
    bitSet(shipstaticinfo->staticheader.infoFlags, IF_InfoNeeded);//this ship will have to be loaded
    if (colorScheme >= 0)
    {
        memcpy(teamCol,shipstaticinfo->teamColor,sizeof(teamCol));//make sure the right team colors are loaded
        teamCol[colorScheme] = TRUE;                            // make sure this ship's color scheme gets loaded
        SetTeamColorForShipAndRelatedStaticInfo(shiptype,shiprace,teamCol);
    }
}

/*-----------------------------------------------------------------------------
    Name        : scriptSetDerelictToBeNeeded
    Description : Level preload callback that sets a derelict to be loaded.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void scriptSetDerelictToBeNeeded(char *directory,char *field,void *dataToFillIn)
{
    DerelictType derelicttype;
    char derelicttypestr[50];

    RemoveCommasFromString(field);

    sscanf(field,"%s ",derelicttypestr);

    derelicttype = StrToDerelictType(derelicttypestr);

    if (!((derelicttype >= 0) && (derelicttype <= NUM_DERELICTTYPES)))
    {
        dbgFatalf(DBG_Loc,"Bad derelict type in %s",field);
    }
    bitSet(derelictStaticInfos[derelicttype].staticheader.infoFlags, IF_InfoNeeded);
}

#if 0
/*-----------------------------------------------------------------------------
    Name        : levelSetMaybeNeeded
    Description : Static info parsing callback to move the IF_InfoWillBeNeeded
                    bit to the IF_InfoNeeded.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void levelSetMaybeNeeded(StaticInfo *info, ObjType objType)
{
    if (bitTest(info->staticheader.infoFlags, IF_InfoWillBeNeeded))
    {
        bitSet(info->staticheader.infoFlags, IF_InfoNeeded);
        bitClear(info->staticheader.infoFlags, IF_InfoWillBeNeeded);
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : levelPreInit
    Description : A level init function to be called before any ships are loaded.
    Inputs      : directory, picked mission - path/name of .level file
    Outputs     : Sets the IF_InfoNeeded and teamColor members of ships
    Return      :
----------------------------------------------------------------------------*/
void levelPreInit(char *directory,char *pickedMission)
{
    tecolorscheme playerSchemes[MAX_MULTIPLAYER_PLAYERS];
    sdword index;
    Node *node;
    Ship *ship;
    InsideShip* insideship;
    bool8 teamCol[MAX_MULTIPLAYER_PLAYERS];

    missionman = isLevelMissionManGenerated(directory,pickedMission);

    memcpy(playerSchemes, teColorSchemes, sizeof(playerSchemes));
    //...set everything to be loaded this mission (the level file will manually exclude ships)
    universeFlagEverythingNeeded();
    //set default color schemes
    teReset();
    universeDefaultTeamColors();
    //load the actual mission
    scriptSet(directory, pickedMission, MissionPreloadScriptTable);

    for (index = 0; index < MAX_MULTIPLAYER_PLAYERS; index++)
    {                                                       //for all players
        if (index < numPlayers && universe.players[index].aiPlayer == NULL)
        {                                                   //if this is a human player
            teColorSchemes[index] = playerSchemes[index];   //restore this player's color scheme
        }
        else if (teColorSchemes[index].textureColor.base != playerSchemes[index].textureColor.base ||
                 teColorSchemes[index].textureColor.detail != playerSchemes[index].textureColor.detail)
        {                                                   //if this color scheme has changed
            teTeamColorsSet(index, teColorSchemes[index].textureColor.base,
                            teColorSchemes[index].textureColor.detail);//recompute all the engine trails etc.
        }
    }
    //go through all the players' ships and make sure they'll be loaded
    for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
    {
        insideship = (InsideShip*)listGetStructOfNode(node);
        ship = insideship->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);
        SetInfoNeededForShipAndRelatedStaticInfo(ship->shiptype,ship->shiprace,TRUE);
        memcpy(teamCol,ship->staticinfo->teamColor,sizeof(teamCol));
        teamCol[ship->colorScheme] = TRUE;                          // preserve color schemes
        SetTeamColorForShipAndRelatedStaticInfo(ship->shiptype,ship->shiprace,teamCol);

        // check inside ships too!
        if (ship->shipsInsideMe != NULL)
        {
            // check ships inside too
            Node *insidenode = ship->shipsInsideMe->insideList.head;
            InsideShip *insideship2;
            Ship *tiship;

            while (insidenode != NULL)
            {
                insideship2 = (InsideShip *)listGetStructOfNode(insidenode);
                tiship = insideship2->ship;
                dbgAssert(tiship->objtype == OBJ_ShipType);

                SetInfoNeededForShipAndRelatedStaticInfo(tiship->shiptype,tiship->shiprace,TRUE);
                memcpy(teamCol,tiship->staticinfo->teamColor,sizeof(teamCol));
                teamCol[tiship->colorScheme] = TRUE;                          // preserve color schemes
                SetTeamColorForShipAndRelatedStaticInfo(tiship->shiptype,tiship->shiprace,teamCol);

                insidenode = insidenode->next;
            }
        }
    }

    needR1CapitalShip = FALSE;
    needR2CapitalShip = FALSE;

    for (index=0;index<numPlayers;index++)
    {
        if (universe.players[index].race == R1)
        {
            needR1CapitalShip = TRUE;
        }
        if (universe.players[index].race == R2)
        {
            needR2CapitalShip = TRUE;
        }
    }

    scriptSet(directory, pickedMission, MissionPreloadScriptTablePass2);

    if (singlePlayerGame)
    {
        singlePlayerPreLoadCheck();
    }
    else
    {
        if (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip))
        {
            if (needR1CapitalShip)
            {
                SetInfoNeededForShipAndRelatedStaticInfo(CapturableCapitalShipToGive,R1,TRUE);
            }
            if (needR2CapitalShip)
            {
                SetInfoNeededForShipAndRelatedStaticInfo(CapturableCapitalShipToGive,R2,TRUE);
            }
        }

        if (bitTest(tpGameCreated.flag,MG_CratesEnabled))
        {
            bitSet(derelictStaticInfos[Crate].staticheader.infoFlags, IF_InfoNeeded);
        }
    }

    //now go through all ships in the .level file and flag them for loading
    //universeAllStaticsScan(levelSetMaybeNeeded, levelSetMaybeNeeded, levelSetMaybeNeeded, levelSetMaybeNeeded);
}

void TryToFindMothershipsForPlayers()
{
    sdword i;
    Player *player;

    for (i=0;i<universe.numPlayers;i++)
    {
        player = &universe.players[i];

        if (player->PlayerMothership == NULL)
        {
            dbgMessage("\nWarning: no mothership assigned, trying to find one:");
            if (!univFindBackupMothership(player))
            {
                dbgMessage("\nWarning: no mothership found for this player");
            }
        }
    }
}

void levelInit(char *directory,char *pickedMission)
{
    udword i;

    dbgAssert(numPlayers <= MAX_MULTIPLAYER_PLAYERS);
    dbgAssert(curPlayer < numPlayers);

    missionman = isLevelMissionManGenerated(directory,pickedMission);

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        universe.shipMaxUnits[i] = 0;
    }

    if (singlePlayerGame)
    {
        singlePlayerSetMissionAttributes(directory,pickedMission);
    }
    SetSMMissionTweaks(directory,pickedMission);
    scriptSetStruct(directory,pickedMission,MissionScriptTable,NULL);
    TryToFindMothershipsForPlayers();

    mrCamera = &(universe.mainCameraCommand.actualcamera);

    universeSwitchToPlayer(curPlayer);

    if ((tutorial) || (!singlePlayerGame))
    {
        clPresetShipsToPosition(&universe.mainCommandLayer);

        if (universe.curPlayerPtr->PlayerMothership)
        {
            ccFocusOnPlayersMothership(&universe.mainCameraCommand,curPlayer);
        }
        else
        {
            ccFocusOnFleet(&universe.mainCameraCommand);
        }
    }

    if (singlePlayerGame)
    {
        universe.curPlayerPtr->resourceUnits = SINGLEPLAYER_STARTINGRUS;
    }
    else
    {
        soundEventPlayMusic(SongNumber);
    }

    if(tutorial==TUTORIAL_ONLY)
    {
        soundEventPlayMusic(SongNumber);
        universe.curPlayerPtr->resourceUnits = resourceStartTutorial;
    }
}

void levelStartNext(char *directory,char *pickedMission)
{
    dbgAssert(singlePlayerGame);        // this function is only for the single player game

    missionman = isLevelMissionManGenerated(directory,pickedMission);

    singlePlayerSetMissionAttributes(directory,pickedMission);
    SetSMMissionTweaks(directory,pickedMission);
    scriptSetStruct(directory,pickedMission,MissionScriptTable,NULL);

    TryToFindMothershipsForPlayers();
}

bool AnyShipOverlapsAsteroid(vector *position,AsteroidType asteroidtype)
{
    Node *node;
    Ship *ship;
    vector checkpos = *position;
    real32 consider,negconsider;
    real32 asteroidr = asteroidStaticInfos[asteroidtype].staticheader.staticCollInfo.collspheresize;
    vector diff;

    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            if (ship->shiptype == MiningBase)
            {
                goto next;      // MiningBase doesn't interfere with regrowing...
            }
            consider = asteroidr + ship->staticinfo->staticheader.staticCollInfo.collspheresize;
            negconsider = -consider;
            vecSub(diff,ship->posinfo.position,checkpos);

            if (!isBetweenInclusive(diff.x,negconsider,consider))
            {
                goto next;
            }

            if (!isBetweenInclusive(diff.y,negconsider,consider))
            {
                goto next;
            }

            if (!isBetweenInclusive(diff.z,negconsider,consider))
            {
                goto next;
            }

            // consider it too close...
            return TRUE;
        }

next:
        node = node->next;
    }

    return FALSE;
}

