/*=============================================================================
    Name    : univupdate
    Purpose : updates the universe, main game loop

    Created 10/4/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "glinc.h"
#include "Types.h"
#include "Debug.h"
#include "Matrix.h"
#include "FastMath.h"
#include "LinkedList.h"
#include "ObjTypes.h"
#include "SpaceObj.h"
#include "Memory.h"
#include "Collision.h"
#include "Physics.h"
#include "AITrack.h"
#include "AIShip.h"
#include "CommandLayer.h"
#include "Universe.h"
#include "Select.h"
#include "StatScript.h"
#include "SoundEvent.h"
#include "color.h"
#include "Globals.h"
#include "FlightMan.h"
#include "UnivUpdate.h"
#include "DFGFrigate.h"
#include "Ships.h"
#include "Dock.h"
#include "mainrgn.h"
#include "PiePlate.h"
#include "Sensors.h"
#include "File.h"
#include "NetCheck.h"
#include "CommandWrap.h"
#include "utility.h"
#include "SinglePlayer.h"
#include "NIS.h"
#include "MEX.h"
#include "mouse.h"
#include "AIPlayer.h"
#include "Clouds.h"
#include "Nebulae.h"
#include "Blobs.h"
#include "Strings.h"
#include "ResearchAPI.h"
#include "Tactics.h"
#include "InfoOverlay.h"
#include "MultiplayerGame.h"
#include "MeshAnim.h"
#include "Alliance.h"
#include "Damage.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"
#include "Clamp.h"
#include "LevelLoad.h"
#include "Attributes.h"
#include "Ping.h"
#include "Teams.h"
#include "Bounties.h"
#include "Tutor.h"
#include "TradeMgr.h"
#include "Crates.h"
#include "SaveGame.h"
#include "Battle.h"
#include "Randy.h"
#include "HS.h"
#include "LaunchMgr.h"
#include "ProfileTimers.h"

#ifndef HW_Release

#if 0
#ifdef gshaw
#define DEBUG_COLLISIONS
#endif
#endif

#endif

vector defaultshipupvector = { 0.0f, 0.0f, 1.0f };
vector defaultshiprightvector = { 0.0f, -1.0f, 0.0f };
vector defaultshipheadingvector = { 1.0f, 0.0f, 0.0f };
matrix defaultshipmatrix = { 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

IDToPtrTable ShipIDToPtr;
IDToPtrTable ResourceIDToPtr;
IDToPtrTable DerelictIDToPtr;
IDToPtrTable MissileIDToPtr;

/*=============================================================================
    Private functions:
=============================================================================*/

void setSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *object);


bool ApplyDamageToTarget(SpaceObjRotImpTarg *target,real32 damagetaken,GunSoundType soundType,sdword damageHow,sdword playerIndex);
static void MakeExplosionRockBumpables(Ship *ship,bool scuttle);
void pointExplosionInSpace(vector *position,real32 blastRadius,real32 maxDamage,sdword playerIndex);

void univCheckShipState(Ship *ship);
void univSetupShipForControl(Ship *ship);
void univMinorSetupShipForControl(Ship *ship);

/******************************************************************************
Private data
******************************************************************************/

bool gameIsEnding=FALSE;

/*=============================================================================
    Tweakables
=============================================================================*/

real32 FLY_INTO_WORLD_PERCENT_DIST = 0.7f;

udword NUM_STARS            = 800;
udword NUM_BIG_STARS        = 175;

color backgroundColor = colRGB(0, 25, 0);

real32 MAX_VELOCITY_TANGENT_FACTOR = 1.0f;
sdword MIN_COLLISION_DAMAGE = 5;

scriptEntry StarTweaks[] =
{
    { "NumStars",               scriptSetUdwordCB, &NUM_STARS },
    { "NumBigStars",            scriptSetUdwordCB, &NUM_BIG_STARS },
    { "starMaxColor",           scriptSetUdwordCB, &starMaxColor },
    { "starMinColor",           scriptSetUdwordCB, &starMinColor },
    { "starRedVariance",        scriptSetUdwordCB, &starRedVariance },
    { "starGreenVariance",        scriptSetUdwordCB, &starGreenVariance },
    { "starBlueVariance",        scriptSetUdwordCB, &starBlueVariance },
    { NULL, NULL, 0 }
};
scriptEntry GalaxyTweaks[] =
{
    { "BackgroundColor",        scriptSetRGBCB, &backgroundColor },
    { NULL, NULL, 0 }
};

void univUpdateObjRotInfo(SpaceObjRot *robj)
{
#define irobj ((SpaceObjRotImp *)robj)
#define shipobj ((Ship *)robj)

    if (bitTest(robj->flags,SOF_Impactable))
    {
        matMultiplyMatByVec(&irobj->collInfo.collOffset,&irobj->rotinfo.coordsys,&irobj->staticinfo->staticheader.staticCollInfo.collsphereoffset);
        vecAdd(irobj->collInfo.collPosition,irobj->posinfo.position,irobj->collInfo.collOffset);

        collUpdateCollRectangle(irobj);

        if (irobj->objtype == OBJ_ShipType)
        {
            matMultiplyMatByVec(&shipobj->engineOffset,&shipobj->rotinfo.coordsys,&((ShipStaticInfo *)shipobj->staticinfo)->engineNozzleOffset[0]);

            vecAdd(shipobj->enginePosition,shipobj->posinfo.position,shipobj->engineOffset);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRotateObjPitch
    Description : rotates an obj about its right vector (producing pitch), correctly
                  rotating collision information, etc.
    Inputs      : obj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRotateObjPitch(SpaceObjRot *robj,real32 rot)
{
    matrix rotymat;
    matrix tempmat;

    matMakeRotAboutY(&rotymat,(real32)cos(rot),(real32)sin(rot));
    tempmat = robj->rotinfo.coordsys;
    matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotymat);

    univUpdateObjRotInfo(robj);
}

/*-----------------------------------------------------------------------------
    Name        : univRotateObjYaw
    Description : rotates an obj about its up vector (producing yaw), correctly
                  rotating collision information, etc.
    Inputs      : obj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRotateObjYaw(SpaceObjRot *robj,real32 rot)
{
    matrix rotxmat;
    matrix tempmat;

    matMakeRotAboutX(&rotxmat,(real32)cos(rot),(real32)sin(rot));
    tempmat = robj->rotinfo.coordsys;
    matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotxmat);

    univUpdateObjRotInfo(robj);
}

/*-----------------------------------------------------------------------------
    Name        : univRotateObjRoll
    Description : rotates an obj about its heading vector (producing Roll), correctly
                  rotating collision information, etc.
    Inputs      : obj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRotateObjRoll(SpaceObjRot *robj,real32 rot)
{
    matrix rotzmat;
    matrix tempmat;

    matMakeRotAboutZ(&rotzmat,(real32)cos(rot),(real32)sin(rot));
    tempmat = robj->rotinfo.coordsys;
    matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotzmat);

    univUpdateObjRotInfo(robj);
}

/*-----------------------------------------------------------------------------
    Name        : univInitSpaceObjPosRot
    Description : initializes the position and rotation info of a space object,
                  including orientation (heading), position
    Inputs      : position, randomOrientation flag
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univInitSpaceObjPosRot(SpaceObj *obj,vector *position,bool randomOrientation)
{
    memset(&(obj->posinfo),0,sizeof(obj->posinfo));
    obj->posinfo.position = *position;
    obj->posinfo.isMoving = FALSE;
    obj->posinfo.haventCalculatedDist = TRUE;

    if (bitTest(obj->flags,SOF_Rotatable))
    {
#define robj ((SpaceObjRot *)obj)
        memset(&robj->rotinfo,0,sizeof(robj->rotinfo));
        robj->rotinfo.coordsys = defaultshipmatrix;

        if (randomOrientation)
        {
            real32 rotaboutx = frandom(2*PI);
            real32 rotabouty = frandom(2*PI);
            real32 rotaboutz = frandom(2*PI);
            matrix rotxmat;
            matrix rotymat;
            matrix rotzmat;
            matrix tempmat;

            matMakeRotAboutX(&rotxmat,(real32)cos(rotaboutx),(real32)sin(rotaboutx));
            matMakeRotAboutY(&rotymat,(real32)cos(rotabouty),(real32)sin(rotabouty));
            matMakeRotAboutZ(&rotzmat,(real32)cos(rotaboutz),(real32)sin(rotaboutz));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotxmat);
            matMultiplyMatByMat(&tempmat,&robj->rotinfo.coordsys,&rotymat);
            matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotzmat);
        }

        univUpdateObjRotInfo((SpaceObjRot *)obj);
    }
}

#define IDTOPTR_GROWBATCH       250

void IDToPtrTableInit(IDToPtrTable *table)
{
    table->numEntries = 0;
    table->objptrs = NULL;
}

void IDToPtrTableClose(IDToPtrTable *table)
{
    if (table->objptrs)
    {
        memFree(table->objptrs);
        table->objptrs = NULL;
    }
    table->numEntries = 0;
}

void IDToPtrTableReset(IDToPtrTable *table)
{
    IDToPtrTableClose(table);
    IDToPtrTableInit(table);
}

void IDToPtrTableObjDied(IDToPtrTable *table,uword ID)
{
    if (ID >= table->numEntries)
    {
        return;
    }

    table->objptrs[ID] = NULL;      // remove reference
}

SpaceObjPtr IDToPtrTableIDToObj(IDToPtrTable *table,uword ID)
{
    if (ID >= table->numEntries)
    {
        return NULL;
    }

    return table->objptrs[ID];
}

void IDToPtrTableAdd(IDToPtrTable *table,uword ID,SpaceObj *obj)
{
    dbgAssert(obj);

    if (ID >= table->numEntries)
    {
        sdword newnumber = table->numEntries+IDTOPTR_GROWBATCH;
#ifdef gshaw
        if (ID > (table->numEntries+1))
        {
            _asm int 3
        }
#endif
        if (table->objptrs)
        {
            table->objptrs = memRealloc(table->objptrs,sizeof(SpaceObjPtr) * newnumber,"idtoptrs",0);
        }
        else
        {
            table->objptrs = memAlloc(sizeof(SpaceObjPtr) * newnumber,"idtoptrs",0);
        }
        memset(&table->objptrs[table->numEntries],0,sizeof(SpaceObjPtr)*IDTOPTR_GROWBATCH);
        table->numEntries += IDTOPTR_GROWBATCH;
    }

    dbgAssert(ID < table->numEntries);

    if (table->objptrs[ID])
    {
        dbgFatalf(DBG_Loc,"Obj with this ID %d already exists",ID);
    }
    else
    {
        table->objptrs[ID] = obj;
    }
    return;
}

/*=============================================================================
    Save Game Stuff
=============================================================================*/
#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

typedef struct SaveIDToPtrTableStruct {
    sdword num;
    sdword ID[1];
} SaveIDToPtrTableStruct;

#define sizeofSaveIDToPtrTableStruct(n) (sizeof(SaveIDToPtrTableStruct) + (n-1)*sizeof(sdword))

void SaveIDToPtrTable(IDToPtrTable *table)
{
    sdword num = table->numEntries;
    SaveChunk *chunk;
    struct SaveIDToPtrTableStruct *savecontents;
    sdword i;

    chunk = CreateChunk(IDTOPTRTABLE,sizeofSaveIDToPtrTableStruct(num),NULL);

    savecontents = chunkContents(chunk);
    savecontents->num = num;

    for (i=0;i<num;i++)
    {
        savecontents->ID[i] = SpaceObjRegistryGetID(table->objptrs[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadIDToPtrTable(IDToPtrTable *table)
{
    SaveChunk *chunk;
    SaveIDToPtrTableStruct *loadcontents;
    sdword num;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,IDTOPTRTABLE);

    loadcontents = (SaveIDToPtrTableStruct *)chunkContents(chunk);
    num = loadcontents->num;

    dbgAssert(sizeofSaveIDToPtrTableStruct(num) == chunk->contentsSize);

    table->numEntries = num;
    if (num == 0)
    {
        table->objptrs = NULL;
    }
    else
    {
        table->objptrs = memAlloc(sizeof(SpaceObjPtr) * num,"idtoptrs",0);
    }

    for (i=0;i<num;i++)
    {
        table->objptrs[i] = SpaceObjRegistryGetObj(loadcontents->ID[i]);
}

    memFree(chunk);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"
/*=============================================================================
    Save Game Stuff End
=============================================================================*/

void univInitFastNetworkIDLookups(void)
{
    IDToPtrTableInit(&ShipIDToPtr);
    IDToPtrTableInit(&ResourceIDToPtr);
    IDToPtrTableInit(&DerelictIDToPtr);
    IDToPtrTableInit(&MissileIDToPtr);
}

void univCloseFastNetworkIDLookups(void)
{
    IDToPtrTableClose(&ShipIDToPtr);
    IDToPtrTableClose(&ResourceIDToPtr);
    IDToPtrTableClose(&DerelictIDToPtr);
    IDToPtrTableClose(&MissileIDToPtr);
}

void univResetFastNetworkIDLookups(void)
{
    IDToPtrTableReset(&ShipIDToPtr);
    IDToPtrTableReset(&ResourceIDToPtr);
    IDToPtrTableReset(&DerelictIDToPtr);
    IDToPtrTableReset(&MissileIDToPtr);
}

/*-----------------------------------------------------------------------------
    Name        : ShipIDtoShip
    Description : Converts a ShipID to a pointer to the actual ship
    Inputs      : shipID
    Outputs     :
    Return      :
    Notes       : Later optimize this routine (perhaps use a table look-up)
----------------------------------------------------------------------------*/
Ship *ShipIDtoShip(ShipID shipID,bool considerInsideShips)
{
    Ship *ship = (Ship *)IDToPtrTableIDToObj(&ShipIDToPtr,shipID.shipNumber);

    if (ship != NULL)
    {
        dbgAssert(ship->objtype == OBJ_ShipType);
        if (!considerInsideShips)
        {
            if (ship->flags & SOF_Hide)
            {
                ship = NULL;
            }
        }
    }

    return ship;
}

/*-----------------------------------------------------------------------------
    Name        : univFindShipIAmInside
    Description : returns the ship I am inside if I'm inside one, otherwise NULL
    Inputs      : me
    Outputs     :
    Return      : returns the ship I am inside if I'm inside one, otherwise NULL
----------------------------------------------------------------------------*/
Ship *univFindShipIAmInside(Ship *me)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->shipsInsideMe)
        {
            Node *insidenode = ship->shipsInsideMe->insideList.head;
            InsideShip *insideship;

            while (insidenode != NULL)
            {
                insideship = (InsideShip *)listGetStructOfNode(insidenode);
                dbgAssert(insideship->ship->objtype == OBJ_ShipType);

                if (insideship->ship == me)
                {
                    return ship;
                }

                insidenode = insidenode->next;
            }
        }

        objnode = objnode->next;
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : univAmIInsideThisShip
    Description : returns TRUE if me ship is inside ship
    Inputs      : me,ship
    Outputs     :
    Return      : returns TRUE if me ship is inside ship
----------------------------------------------------------------------------*/
bool univAmIInsideThisShip(Ship *me,Ship *ship)
{
    if (ship->shipsInsideMe)
    {
        Node *insidenode = ship->shipsInsideMe->insideList.head;
        InsideShip *insideship;

        while (insidenode != NULL)
        {
            insideship = (InsideShip *)listGetStructOfNode(insidenode);
            dbgAssert(insideship->ship->objtype == OBJ_ShipType);

            if (insideship->ship == me)
            {
                return TRUE;
            }

            insidenode = insidenode->next;
        }
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : ResourceIDtoResourcePtr
    Description : Converts a resourceID to a pointer to the actual resource
    Inputs      : resourceID
    Outputs     :
    Return      :
    Notes       : Later optimize this routine (perhaps use a table look-up)
----------------------------------------------------------------------------*/
ResourcePtr ResourceIDtoResourcePtr(ResourceID resourceID)
{
    return (Resource *)IDToPtrTableIDToObj(&ResourceIDToPtr,resourceID.resourceNumber);
}

/*-----------------------------------------------------------------------------
    Name        : DerelictIDtoDerelictPtr
    Description : Converts a derelictID to a pointer to the actual derelict
    Inputs      : derelictID
    Outputs     :
    Return      :
    Notes       : Later optimize this routine (perhaps use a table look-up)
----------------------------------------------------------------------------*/
DerelictPtr DerelictIDToDerelictPtr(DerelictID derelictID)
{
    return (Derelict *)IDToPtrTableIDToObj(&DerelictIDToPtr,derelictID.derelictNumber);
}

/*-----------------------------------------------------------------------------
    Name        : MissileIDtoMissilePtr
    Description : Converts a missileID to a pointer to the actual missile
    Inputs      : missileID
    Outputs     :
    Return      :
    Notes       : Later optimize this routine (perhaps use a table look-up)
----------------------------------------------------------------------------*/
MissilePtr MissileIDtoMissilePtr(MissileID missileID)
{
    return (Missile *)IDToPtrTableIDToObj(&MissileIDToPtr,missileID.missileNumber);
}

/*-----------------------------------------------------------------------------
    Name        : univAddAsteroid
    Description : adds an asteroid to the mission sphere at position astpos
    Inputs      : astpos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Asteroid *univAddAsteroid(AsteroidType asteroidtype,vector *astpos)
{
    Asteroid *newAsteroid;

    dbgAssert(asteroidtype < NUM_ASTEROIDTYPES);

    newAsteroid = memAlloc(sizeof(Asteroid),"Asteroid",NonVolatile);

    newAsteroid->resourceID.resourceNumber = universe.resourceNumber;
    IDToPtrTableAdd(&ResourceIDToPtr,universe.resourceNumber,(SpaceObj *)newAsteroid);
    universe.resourceNumber++;

    newAsteroid->objtype = OBJ_AsteroidType;
    newAsteroid->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable + SOF_Resource;
    if (asteroidtype == Asteroid0) newAsteroid->flags |= SOF_DontApplyPhysics;      // optimization for Asteroid0
    newAsteroid->staticinfo = (ResourceStaticInfo *)&asteroidStaticInfos[asteroidtype];
    ClearNode(newAsteroid->renderlink);
    newAsteroid->collMyBlob = NULL;
    newAsteroid->currentLOD = 0;
    newAsteroid->cameraDistanceSquared = REALlyBig;
    newAsteroid->flashtimer = 0;
    newAsteroid->lasttimecollided = 0;
    newAsteroid->attributes = 0;
    newAsteroid->attributesParam = 0;
    newAsteroid->resourcevalue = asteroidStaticInfos[asteroidtype].resourcevalue;
    newAsteroid->resourceNotAccessible = 0;
    newAsteroid->resourceVolume = NULL;
    newAsteroid->asteroidtype = asteroidtype;
    newAsteroid->scaling = 1.0f;
    newAsteroid->health = newAsteroid->staticinfo->maxhealth;
    newAsteroid->collInfo.precise = NULL;

    vecZeroVector(newAsteroid->posinfo.force);
    vecZeroVector(newAsteroid->rotinfo.torque);

    univInitSpaceObjPosRot((SpaceObj *)newAsteroid,astpos,TRUE);

    if (asteroidtype == Asteroid0)
    {
        newAsteroid->collMyBlob = NULL;
        listAddNode(&universe.MinorSpaceObjList,&(newAsteroid->objlink),newAsteroid);
    }
    else
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)newAsteroid);
        listAddNode(&universe.SpaceObjList,&(newAsteroid->objlink),newAsteroid);
        listAddNode(&universe.ResourceList,&(newAsteroid->resourcelink),newAsteroid);
        listAddNode(&universe.ImpactableList,&(newAsteroid->impactablelink),newAsteroid);
    }

    return newAsteroid;
}

/*-----------------------------------------------------------------------------
    Name        : univAddDerelict
    Description : adds a derelict to the mission sphere at position pos
    Inputs      : pos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Derelict *univAddDerelict(DerelictType derelicttype,vector *pos)
{
    dbgAssert(derelicttype < NUM_DERELICTTYPES);
    return(univAddDerelictByStatInfo(derelicttype, (DerelictStaticInfo *)&derelictStaticInfos[derelicttype], pos));
}

Derelict *univAddHyperspaceGateAsDerelict(hvector *posAndRotation)
{
    Derelict *derelict = univAddDerelict(HyperspaceGate,(vector *)posAndRotation);
    univRotateObjYaw((SpaceObjRot *)derelict,posAndRotation->w);
    bitSet(derelict->flags,SOF_NotBumpable|SOF_DontApplyPhysics);
#ifdef gshaw
    //bitSet(derelict->flags,SOF_ForceVisible);
#endif
    return derelict;
}

/*-----------------------------------------------------------------------------
    Name        : univAddMissile
    Description : Adds a missile to the world.  This missile will have no
                    owner, no target and it's velocity, position and coordsys
                    must be set up by the owner.  Some missile this is!
    Inputs      : race - race of missile to create
    Outputs     :
    Return      : pointer to newly created missile
----------------------------------------------------------------------------*/
Missile *univAddMissile(ShipRace race)
{
    Missile *missile;
    sdword i;

    missile = memAlloc(sizeof(Missile),"Missile",0);
    memset(missile,0,sizeof(Missile));

    missile->objtype = OBJ_MissileType;
    missile->flags = SOF_Rotatable + SOF_Impactable + SOF_NotBumpable + SOF_Targetable;

    missile->enginePosition.x = 0.0f;
    missile->enginePosition.y = 0.0f;
    missile->enginePosition.z = 0.0f;
    missile->trail = NULL;

    missile->staticinfo = &missileStaticInfos[race];
    missile->maxtraveldist = missile->staticinfo->staticheader.maxvelocity * universe.phystimeelapsed;
    missile->missileType = MISSILE_Regular;
    missile->FORCE_DROPPED = FALSE;
    missile->formationinfo = NULL;
    missile->trail = mistrailNew(missile->staticinfo->trailStatic, missile);

    missile->health = ((MissileStaticInfo *)missile->staticinfo)->maxhealth;

    ClearNode(missile->renderlink);
    missile->currentLOD = 0;
    missile->cameraDistanceSquared = 1.0f;  //!!!
    missile->flashtimer = 0;
    missile->lasttimecollided = 0;
    missile->attributes = 0;
    missile->attributesParam = 0;

    vecZeroVector(missile->posinfo.force);
    vecZeroVector(missile->rotinfo.torque);

    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
       missile->nonstatvars.thruststrength[i] = missile->staticinfo->thruststrengthstat[i];
    }
    for(i=0;i< NUM_ROT_DEGOFFREEDOM;i++)
    {
       missile->nonstatvars.rotstrength[i] = missile->staticinfo->rotstrengthstat[i];
    }
    for(i=0;i< NUM_TURN_TYPES;i++)
    {
       missile->nonstatvars.turnspeed[i] = missile->staticinfo->turnspeedstat[i];
    }

    missile->soundType = GS_MissleLauncher;
    missile->race = race;
    missile->timelived = 0.0f;
    missile->totallifetime = 12.26f;                        //!!! from the missile destroyer .ship file
    missile->playerowner = &universe.players[((race == universe.players[0].race)) ? 0 : 1];
    missile->owner = NULL;
    missile->target = NULL;

    missile->damage = 48.0f;                                //from the.shp file

    missile->missileID.missileNumber = universe.missileNumber;
    if (multiPlayerGame) IDToPtrTableAdd(&MissileIDToPtr,universe.missileNumber,(SpaceObj *)missile);
    universe.missileNumber++;

    //position, velocity and coordsys left blank
    missile->posinfo.isMoving = TRUE;
    missile->posinfo.haventCalculatedDist = TRUE;

    collAddSpaceObjToCollBlobs((SpaceObj *)missile);

    //proper hit effect
    missile->hitEffect = etgGunEventTable[race][GS_MissleLauncher][EGT_GunHit];//get pointer to hit effect

    listAddNode(&universe.SpaceObjList,&(missile->objlink),missile);
    listAddNode(&universe.ImpactableList,&(missile->impactablelink),missile);
    listAddNode(&universe.MissileList,&(missile->missilelink),missile);

    return(missile);
}

/*-----------------------------------------------------------------------------
    Name        : univAddToWorldList
    Description : Add a derelict to the universe's world list
    Inputs      : world - what object to add to the world list
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univAddToWorldList(Derelict *world)
{
    sdword index;

    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        if (universe.world[index] == NULL)
        {
            universe.world[index] = world;
            return;
        }
    }
    dbgAssert(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveFromWorldList
    Description : Remove a derelict reference from the universe's world list
    Inputs      : world - what world we are to remove
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univRemoveFromWorldList(Derelict *world)
{
    sdword index, j;

    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        if (universe.world[index] == world)
        {
            if (index < UNIV_NUMBER_WORLDS - 1)
            {
                for (j = UNIV_NUMBER_WORLDS - 1; j > index; j--)
                {
                    if (universe.world[j] != NULL)
                    {
                        universe.world[index] = universe.world[j];
                        universe.world[j] = NULL;
                        return;
                    }
                }
            }
            universe.world[index] = NULL;
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univAddDerelictByStatInfo
    Description : Add a derelict by direct staticinfo pointer rather than just
                    by type.  Mostly used for generic objects in NIS's.
    Inputs      : derelictType - type of derelict.  Can be outside range of 0..NUM_DERELICTTYPES
                  stat - static info for this derelict.
                  pos - where to create it
    Outputs     :
    Return      : Newly allocated derelict.
----------------------------------------------------------------------------*/
Derelict *univAddDerelictByStatInfo(DerelictType derelictType, DerelictStaticInfo *stat, vector *pos)
{
    Derelict *newDerelict;
    sdword i;

    newDerelict = memAlloc(sizeof(Derelict),"Derelict",NonVolatile);

    newDerelict->derelictID.derelictNumber = universe.derelictNumber;
    IDToPtrTableAdd(&DerelictIDToPtr,universe.derelictNumber,(SpaceObj *)newDerelict);
    universe.derelictNumber++;

    newDerelict->objtype = OBJ_DerelictType;
    newDerelict->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable;
    newDerelict->staticinfo = stat;
    ClearNode(newDerelict->renderlink);
    newDerelict->collMyBlob = NULL;
    newDerelict->renderedLODs = 0;
    newDerelict->currentLOD = 0;
    newDerelict->cameraDistanceSquared = REALlyBig;
    newDerelict->flashtimer = 0;
    newDerelict->lasttimecollided = 0;
    newDerelict->attributes = 0;
    newDerelict->attributesParam = 0;
    newDerelict->derelicttype = derelictType;
    newDerelict->health = newDerelict->staticinfo->maxhealth;
    newDerelict->collInfo.precise = NULL;
    newDerelict->dockingship = NULL;
    newDerelict->putOnDoor=FALSE;
    newDerelict->TractorTrue = FALSE;
    newDerelict->tractorbeaminfo = 0;
    newDerelict->tractorbeam_playerowner = NULL;
    newDerelict->tractorPosinfo[0] = FALSE;
    newDerelict->tractorPosinfo[1] = FALSE;
    newDerelict->tractorPosinfo[2] = FALSE;
    newDerelict->tractorPosinfo[3] = FALSE;
    newDerelict->aistate = 0;
    newDerelict->specialFlags = 0;
    newDerelict->specialFlags2 = 0;
    newDerelict->clampInfo = NULL;
    newDerelict->creationTime = universe.totaltimeelapsed;
    newDerelict->ambientSoundHandle = SOUND_NOTINITED;
    newDerelict->randomSoundHandle = SOUND_NOTINITED;
    newDerelict->nextRandomTime = universe.totaltimeelapsed;

    vecZeroVector(newDerelict->posinfo.force);
    vecZeroVector(newDerelict->rotinfo.torque);

    for(i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        newDerelict->salvageNumTagged[i] = 0;
    }


    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        newDerelict->nonstatvars.thruststrength[i] = stat->thruststrengthstat[i];
    }
    for(i=0;i< NUM_ROT_DEGOFFREEDOM;i++)
    {
        newDerelict->nonstatvars.rotstrength[i] = stat->rotstrengthstat[i];
    }
    for(i=0;i< NUM_TURN_TYPES;i++)
    {
        newDerelict->nonstatvars.turnspeed[i] = stat->turnspeedstat[i];
    }

    newDerelict->newDockWithTransfer = NULL;
    univInitSpaceObjPosRot((SpaceObj *)newDerelict,pos,FALSE);

    //needed hack to Fix LUKES ridiculous NIS hack
    if(derelictType < NUM_DERELICTTYPES)
    {
        //yes, it is a valid derelict
        setSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)newDerelict);
    }
    else
    {
        //fix it for good:
        stat->salvageStaticInfo = NULL;
        newDerelict->salvageInfo = NULL;
    }

    InitializeNavLights((Ship *)newDerelict);

    collAddSpaceObjToCollBlobs((SpaceObj *)newDerelict);
    listAddNode(&universe.SpaceObjList,&(newDerelict->objlink),newDerelict);
    listAddNode(&universe.DerelictList,&(newDerelict->derelictlink),newDerelict);
    listAddNode(&universe.ImpactableList,&(newDerelict->impactablelink),newDerelict);

    if (stat->worldRender)
    {                                                       //if planet-style rendering
//        bitSet(newDerelict->flags, SOF_ForceVisible);
        univAddToWorldList(newDerelict);
    }

    //find a color scheme
    for (i = 0; i < MAX_MULTIPLAYER_PLAYERS; i++)
    {
        if (stat->teamColor[i])
        {
            newDerelict->colorScheme = i;
            break;
        }
    }

    //if a crate, report the placement!
    if (!singlePlayerGame)
    {
        if(derelictType == Crate)
        {
            cratesReportCratePlacement(newDerelict);
        }
    }

    return newDerelict;
}

/*-----------------------------------------------------------------------------
    Name        : univAddDustCloud
    Description : adds a dustcloud to the mission sphere at position cloudpos
    Inputs      : cloudpos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DustCloud *univAddDustCloud(DustCloudType dustcloudtype,vector *cloudpos)
{
    DustCloud* newCloud;

    dbgAssert(dustcloudtype < NUM_DUSTCLOUDTYPES);

    newCloud = memAlloc(sizeof(DustCloud),"DustCloud",NonVolatile);

    newCloud->resourceID.resourceNumber = universe.resourceNumber;
    IDToPtrTableAdd(&ResourceIDToPtr,universe.resourceNumber,(SpaceObj *)newCloud);
    universe.resourceNumber++;

    newCloud->objtype = OBJ_DustType;
    newCloud->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable + SOF_Resource + SOF_NotBumpable;
    newCloud->staticinfo = (ResourceStaticInfo *)&dustcloudStaticInfos[dustcloudtype];
    ClearNode(newCloud->renderlink);
    newCloud->collMyBlob = NULL;
    newCloud->currentLOD = 0;
    newCloud->cameraDistanceSquared = REALlyBig;
    newCloud->flashtimer = 0;
    newCloud->lasttimecollided = 0;
    newCloud->attributes = 0;
    newCloud->attributesParam = 0;
    newCloud->resourcevalue = dustcloudStaticInfos[dustcloudtype].resourcevalue;
    newCloud->resourceNotAccessible = 0;
    newCloud->resourceVolume = NULL;
    newCloud->dustcloudtype = dustcloudtype;
    newCloud->scaling = 1.0f;
    newCloud->health = newCloud->staticinfo->maxhealth;
    newCloud->collInfo.precise = NULL;
    newCloud->stub = (void*)cloudCreateSystem(newCloud->staticinfo->staticheader.staticCollInfo.collspheresize, 0.1f);
    {
        cloudSystem* csys = (cloudSystem*)newCloud->stub;
        csys->position = &newCloud->posinfo.position;
        csys->rotation = &newCloud->rotinfo.coordsys;
        csys->maxhealth = (sdword) newCloud->health;
    }

    vecZeroVector(newCloud->posinfo.force);
    vecZeroVector(newCloud->rotinfo.torque);

    univInitSpaceObjPosRot((SpaceObj *)newCloud,cloudpos,TRUE);

    collAddSpaceObjToCollBlobs((SpaceObj *)newCloud);
    listAddNode(&universe.SpaceObjList,&(newCloud->objlink),newCloud);
    listAddNode(&universe.ResourceList,&(newCloud->resourcelink),newCloud);
    listAddNode(&universe.ImpactableList,&(newCloud->impactablelink),newCloud);

    return newCloud;
}

/*-----------------------------------------------------------------------------
    Name        : univAddGasCloud
    Description : adds a Gascloud to the mission sphere at position cloudpos
    Inputs      : cloudpos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
GasCloud *univAddGasCloud(GasCloudType gascloudtype,vector *cloudpos)
{
    GasCloud *newCloud;

    dbgAssert(gascloudtype < NUM_GASCLOUDTYPES);

    newCloud = memAlloc(sizeof(GasCloud),"GasCloud",NonVolatile);

    newCloud->resourceID.resourceNumber = universe.resourceNumber;
    IDToPtrTableAdd(&ResourceIDToPtr,universe.resourceNumber,(SpaceObj *)newCloud);
    universe.resourceNumber++;

    newCloud->objtype = OBJ_GasType;
    newCloud->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable + SOF_Resource + SOF_NotBumpable;
    newCloud->staticinfo = (ResourceStaticInfo *)&gascloudStaticInfos[gascloudtype];
    ClearNode(newCloud->renderlink);
    newCloud->collMyBlob = NULL;
    newCloud->currentLOD = 0;
    newCloud->cameraDistanceSquared = REALlyBig;
    newCloud->flashtimer = 0;
    newCloud->lasttimecollided = 0;
    newCloud->attributes = 0;
    newCloud->attributesParam = 0;
    newCloud->resourcevalue = gascloudStaticInfos[gascloudtype].resourcevalue;
    newCloud->resourceNotAccessible = 0;
    newCloud->resourceVolume = NULL;
    newCloud->gascloudtype = gascloudtype;
    newCloud->scaling = 1.0f;
    newCloud->health = newCloud->staticinfo->maxhealth;
    newCloud->collInfo.precise = NULL;

    vecZeroVector(newCloud->posinfo.force);
    vecZeroVector(newCloud->rotinfo.torque);

    univInitSpaceObjPosRot((SpaceObj *)newCloud,cloudpos,TRUE);

    collAddSpaceObjToCollBlobs((SpaceObj *)newCloud);
    listAddNode(&universe.SpaceObjList,&(newCloud->objlink),newCloud);
    listAddNode(&universe.ResourceList,&(newCloud->resourcelink),newCloud);
    listAddNode(&universe.ImpactableList,&(newCloud->impactablelink),newCloud);

    return newCloud;
}

/*-----------------------------------------------------------------------------
    Name        : univAddNebula
    Description : adds a nebula to the mission sphere at position nebpos
    Inputs      : nebpos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Nebula* univAddNebula(NebulaType nebulatype,vector* nebpos, void* stub)
{
    Nebula* newNeb;

    dbgAssert(nebulatype < NUM_NEBULATYPES);

    newNeb = memAlloc(sizeof(Nebula), "Nebula", NonVolatile);

    newNeb->resourceID.resourceNumber = universe.resourceNumber;
    IDToPtrTableAdd(&ResourceIDToPtr,universe.resourceNumber,(SpaceObj *)newNeb);
    universe.resourceNumber++;

    newNeb->objtype = OBJ_NebulaType;
    newNeb->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable + SOF_Resource + SOF_NotBumpable;
    newNeb->staticinfo = (ResourceStaticInfo *)&nebulaStaticInfos[nebulatype];
    ClearNode(newNeb->renderlink);
    newNeb->collMyBlob = NULL;
    newNeb->currentLOD = 0;
    newNeb->cameraDistanceSquared = REALlyBig;
    newNeb->flashtimer = 0;
    newNeb->lasttimecollided = 0;
    if (nebAttributes != 0)
    {
        newNeb->attributes = nebAttributes;
    }
    else
    {
        newNeb->attributes = 0;
    }
    newNeb->attributesParam = 0;
    newNeb->resourcevalue = nebulaStaticInfos[nebulatype].resourcevalue;
    newNeb->resourceNotAccessible = 0;
    newNeb->resourceVolume = NULL;
    newNeb->nebulatype = nebulatype;
    newNeb->scaling = 1.0f;
    newNeb->health = newNeb->staticinfo->maxhealth;
    newNeb->collInfo.precise = NULL;

    newNeb->stub = stub;
    nebNewNeb(newNeb);

    vecZeroVector(newNeb->posinfo.force);
    vecZeroVector(newNeb->rotinfo.torque);

    univInitSpaceObjPosRot((SpaceObj*)newNeb, nebpos, TRUE);

    collAddSpaceObjToCollBlobs((SpaceObj *)newNeb);
    listAddNode(&universe.SpaceObjList, &(newNeb->objlink), newNeb);
    listAddNode(&universe.ResourceList, &(newNeb->resourcelink), newNeb);
    listAddNode(&universe.ImpactableList, &(newNeb->impactablelink), newNeb);

    return newNeb;
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteEffect
    Description : Deletes an effect
    Inputs      : effect - pointer to effect to delete
    Outputs     : frees the effect and it's effector list (actually a single memFree operation)
    Return      :
----------------------------------------------------------------------------*/
void univDeleteEffect(Effect *effect)
{
    univRemoveObjFromRenderList((SpaceObj *)effect);
    listDeleteNode(&effect->objlink);
}


/*-----------------------------------------------------------------------------
    Name        : univMeshBindingsDupe
    Description : Duplicate the local static bindings for a ship and fix up gun
                    pointers local to this ship instance.
    Inputs      : shipstaticinfo - staticinfo for this ship
                  newShip - newly allocated ship
    Outputs     : allocates new RAM and copies/fixes up the binding info
    Return      : address of newly allocated RAM or NULL
----------------------------------------------------------------------------*/
shipbindings *univMeshBindingsDupe(ShipStaticInfo *shipstaticinfo, Ship *newShip)
{
    shipbindings *newBindings;
    mhbinding *bindingListSource;
    mhlocalbinding *bindingListDest;
    sdword nLevels, index, nObjects, j;
    lodinfo *lodInfo = shipstaticinfo->staticheader.LOD;

    if (shipstaticinfo->hierarchySize == 0)
    {                                                       //do nothing if there is no bindings to dupe
        return(NULL);
    }
    nLevels = lodInfo->nLevels;                             //record number of LOD's
    newBindings = memAlloc(shipBindingsLength(nLevels) +    //allocate bindings
                           shipstaticinfo->hierarchySize * sizeof(mhlocalbinding),
                           "ShipLocalBindings", NonVolatile);
#if MESH_PRE_CALLBACK
    newBindings->preCallback = NULL;
#endif
    newBindings->postCallback = NULL;                       //no callbacks needed for guns
    bindingListDest = (mhlocalbinding *)((ubyte *)newBindings +
                    shipBindingsLength(nLevels));           //start of first binding list
    for (index = 0; index < nLevels; index++)
    {                                                       //for each LOD
        if ((lodInfo->level[index].flags & LM_LODType) != LT_Mesh)
        {
            newBindings->localBinding[index] = NULL;
            continue;
        }
        newBindings->localBinding[index] = bindingListDest; //store list pointer for this LOD
        nObjects = ((meshdata *)lodInfo->level[index].pData)->nPolygonObjects;
        bindingListSource = lodInfo->level[index].hBindings;
//        memcpy(bindingList, lodInfo->level[index].hBindings,//copy the bindings
//                nObjects * sizeof(mhbinding));
        for (j = 0; j < nObjects; j++)
        {
            if (bindingListSource->function != NULL)
            {
                //!!! should do a switch statement based upon type, however,
                //there is only one type for now:guns
                dbgAssert(newShip->gunInfo != NULL);
                //using an index set up in setGunBindInfo(), set new gun pointer
                bindingListDest->function = bindingListSource->function;
                bindingListDest->userData = &newShip->gunInfo->guns[bindingListSource->userID];
                bindingListDest->userID = (sdword)newShip;    //also save ship pointer
                bindingListDest->flags = bindingListSource->flags;
                bindingListDest->object = bindingListSource->object;
            }
            else
            {
                bindingListDest->function = NULL;         //this is all we need if there is no function
            }
            bindingListDest++;
            bindingListSource++;
        }
//        bindingListDest += nObjects;                        //update location of list
    }
    newBindings->frequencyCounter = 0;                      //counter expires right now
    dbgAssert((udword)((ubyte *)bindingListDest - (ubyte *)newBindings) == shipBindingsLength(nLevels) + shipstaticinfo->hierarchySize * sizeof(mhlocalbinding));
#if MESH_VERBOSE_LEVEL >= 2
    dbgMessagef("\nunivMeshBindingsDupe: localizing bindings 0x%x for ship 0x%x lengh %d",
                newBindings, newShip, shipBindingsLength(nLevels) + shipstaticinfo->hierarchySize * sizeof(mhlocalbinding));
#endif
    return(newBindings);
}

/*-----------------------------------------------------------------------------
    Name        : univMeshBindingsDelete
    Description : Delete the mesh bindings for a ship about to be deleted.
    Inputs      : ship - ship being deleted
    Outputs     : frees the memory
    Return      :
----------------------------------------------------------------------------*/
void univMeshBindingsDelete(Ship *ship)
{
    if (ship->madBindings && ship->madBindings->nCurrentAnim != MAD_NoAnimation)
    {                                                       //if there's a mesh animation going on
        ship->bindings = ship->madBindings->saveBindings;   //restore old bindings
    }
    if (ship->bindings == NULL)
    {
        return;
    }
#if MESH_VERBOSE_LEVEL >= 2
    dbgMessagef("\nunivMeshBindingsDelete: freeing bindings for ship 0x%x", ship);
#endif
    dbgAssert(ship->bindings != NULL);
    memFree(ship->bindings);
}

void InitializeEngineTrails(Ship *newship)
{
    ShipStaticInfo *shipstaticinfo = newship->staticinfo;
    char engine[5];
    sdword idx;
    sdword i;
    sdword numTrails;

    numTrails = 0;
    strcpy(engine, "EngX");
    for (idx = 0; idx < MAX_NUM_TRAILS; idx++)
    {
        engine[3] = (char) ('0' + (char)idx);
        if (mexGetChunk((void*)shipstaticinfo->staticheader.pMexData, "Eng", engine) != 0)
        {
            numTrails = idx + 1;
        }
    }

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        newship->trail[i] = NULL;
    }
    if (shipstaticinfo->trailStatic[0] != NULL)
    {
        for (i = 0; i < numTrails; i++)
        {
            bool8 second = (bool8) ((i == 0) ? 0 : 1);
            newship->trail[i] = trailNew(shipstaticinfo->trailStatic[i], (void*)newship, second, (ubyte)i);
        }
    }
    else
    {
        //capital ship engine glow
        for (i = 0; i < numTrails; i++)
        {
            if (newship->shiprace == R2)
            {
                newship->trail[i] = trailNew(shipstaticinfo->trailStatic[i], (void*)newship, 2, (ubyte)i);
            }
            else
            {
                newship->trail[i] = trailNew(shipstaticinfo->trailStatic[i], (void*)newship, 3, (ubyte)i);
            }
        }
    }
}

void univResetNewGimbleGun(Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;
    dbgAssert(gunstatic->guntype == GUN_NewGimble);

    gun->angle = 0.0f;
    gun->declination = 0.0f;
    gun->anglespeed = 0.0f;
    gun->declinationspeed = 0.0f;
    gun->curgunorientation = gunstatic->defaultgunorientation;
    if (bitTest(gunstatic->flags, GF_MultiLevelMatrix))
    {
        gun->curGunOrientationNonConcat = gunstatic->defaultGunOrientationNonConcat;
    }
    matGetVectFromMatrixCol3(gun->gunheading,gun->curgunorientation);
}

void InitializeNavLights(Ship *newship)
{
    ShipStaticInfo *shipstaticinfo = newship->staticinfo;

    if (shipstaticinfo->navlightStaticInfo != NULL)
    {
        NAVLightStaticInfo *navlightstaticinfo = shipstaticinfo->navlightStaticInfo;
        sdword numNAVLights = navlightstaticinfo->numNAVLights;
        NAVLightStatic *curnavlightstatic;
        NAVLight *curnavlight;
        sdword i;

        dbgAssert(numNAVLights > 0);

        newship->navLightInfo = memAlloc(sizeofNavLightInfo(numNAVLights),"navlightInfo",NonVolatile);
        newship->navLightInfo->numLights = numNAVLights;

        for (i=0,curnavlight=&newship->navLightInfo->navLights[0],curnavlightstatic=&navlightstaticinfo->navlightstatics[0];i<numNAVLights;i++,curnavlight++,curnavlightstatic++)
        {
            curnavlight->navlightstatic = curnavlightstatic;
            curnavlight->lightstate = 0;
            curnavlight->lastTimeFlashed = curnavlightstatic->startdelay;
        }
    }
    else
    {
        newship->navLightInfo = NULL;
    }
}

void InitializeShowingDamage(Ship* ship)
{
    sdword i, j;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            ship->showingDamage[i][j] = NULL;
        }
    }

    ship->nDamageLights = 0;
    ship->damageLights = NULL;

    dmgGetLights(ship);
}

/*-----------------------------------------------------------------------------
    Name        : univCreateShip
    Description : creates a ship, but does not add it to the universe
    Inputs      : shiptype, shiprace, shippos, playerowner,
                  built - flag indicating if ship was built or placed in world
    Outputs     :
    Return      : address of ship created
----------------------------------------------------------------------------*/
Ship *univCreateShip(ShipType shiptype,ShipRace shiprace,vector *shippos,struct Player *playerowner,sdword built)
{
    Ship *newship;
    udword sizespec;
    udword sizeship = sizeof(Ship), totalsize, madAssedSize;
    ShipStaticInfo *shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
    sdword i;
    ubyte *extraPointer;

    dbgAssert(playerowner != NULL);
/*
#ifndef HW_Release
    if (playerowner->race != shiprace)
    {
        dbgFatal(DBG_Loc,"Illegal race of ship for player");
    }
#endif
*/
#ifndef HW_Release
    if (!bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded))
    {                                                       //if this ships was not loaded properly
//#ifdef gshaw
        dbgFatalf(DBG_Loc,"\n%s\\%s was not loaded properly - stubbing out.", ShipRaceToStr(shiprace), ShipTypeToStr(shiptype));
//#endif
        dbgMessagef("\n%s\\%s was not loaded properly - stubbing out.", ShipRaceToStr(shiprace), ShipTypeToStr(shiptype));
        universeForceDefaultShip = TRUE;
        InitStatShipInfo(shipstaticinfo, shiptype, shiprace);
        bitSet(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
        universeForceDefaultShip = FALSE;
    }
#endif
    if ((sizespec = shipstaticinfo->custshipheader.sizeofShipSpecifics) != 0)
    {
        sizeship += sizespec - sizeof(udword);
    }
    totalsize = sizeship;
    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {                                                       //account for the local precise selection
        totalsize += sizeof(PreciseSelection);
    }
    if (shipstaticinfo->madStatic != NULL)
    {                                                       //account for the local animation bindings
        madAssedSize = madAnimSize(shipstaticinfo->staticheader.LOD->nLevels) + //size of the binding info
                     shipstaticinfo->hierarchySize * sizeof(mhlocalbinding) + //size of the actual binding structures
                     shipstaticinfo->madStatic->header->nObjects * sizeof(splinecurve) * 6; //size of spline curves, enough for all objects

        totalsize += madAssedSize;
    }
    newship = memAlloc(totalsize,"Ship",NonVolatile);
    memset(newship,0,sizeship);                             // set everything to 0 by default
    extraPointer = (ubyte *)newship + sizeship;             //point to end of ship where extra RAM was allocated

    newship->sizeofShipSpecifics = sizespec;
    newship->shipDeCloakTime = -20.0f;                  //initialize to a negative number so if a battle occurs at time 0.0f or anytime near by, then it will not screw up.
    newship->objtype = OBJ_ShipType;
    newship->flags = SOF_Rotatable + SOF_Impactable + SOF_Selectable + SOF_Targetable;
    if (shiptype == Drone)
    {
        newship->flags |= SOF_NotBumpable;
    }
    if (shipstaticinfo->staticheader.staticCollInfo.collspheresize > VERYBIGSHIP_SIZE)
    {
        newship->flags |= SOF_BigObject;
    }
    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {
//        newship->collInfo.precise = (PreciseSelection *)(((ubyte *)newship) + sizeship);
        newship->collInfo.precise = (PreciseSelection *)extraPointer;
        extraPointer += sizeof(PreciseSelection);
    }
    else
    {
        newship->collInfo.precise = NULL;
    }
    newship->staticinfo = shipstaticinfo;
    bitClear(newship->specialFlags,SPECIAL_ParadeNeedTomoveCloser);
//    ClearNode(newship->renderlink);

    //need to copy over static info to non static structures so we
    //can beging fiddling with them...

    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        newship->nonstatvars.thruststrength[i] = shipstaticinfo->thruststrengthstat[i];
    }
    for(i=0;i< NUM_ROT_DEGOFFREEDOM;i++)
    {
        newship->nonstatvars.rotstrength[i] = shipstaticinfo->rotstrengthstat[i];
    }
    for(i=0;i< NUM_TURN_TYPES;i++)
    {
        newship->nonstatvars.turnspeed[i] = shipstaticinfo->turnspeedstat[i];
    }

    newship->shipID.shipNumber = universe.shipNumber;
    IDToPtrTableAdd(&ShipIDToPtr,universe.shipNumber,(SpaceObj *)newship);
    universe.shipNumber++;

    newship->shiptype = shiptype;
    newship->shiprace = shiprace;
    newship->tacticstype = Neutral;         //default to neutral tactical setting
    newship->tactics_ordertype = NO_ORDER_NO_ENEMY;  //default to no bonuses...
    newship->tacticsTalk = 0;
    newship->tacticsNeedToFlightMan = FALSE;
    newship->dockwaitforNorm=FALSE;
    newship->putOnDoor=FALSE;
    newship->tacticsFormationVar1 = FALSE;
    newship->DodgeTime = 0.0f;
    newship->isDodging = FALSE;
    newship->timeCreated = universe.totaltimeelapsed;
//    newship->currentLOD = 0;                                //start at highest LOD by default
//    newship->collMyBlob = NULL;
    newship->cameraDistanceSquared = REALlyBig;
    newship->playerowner = playerowner;
    newship->dockingship = NULL;
    newship->specialFlags = 0;
    newship->specialFlags = 0;
    newship->kamikazeState = 0;
    newship->visibleToWho = 0;      //initialize cloaked visible flag to invisible to everyone!
    newship->visibleToWhoPreviousFrame = 0;
    newship->damageModifier = 1.0f;
    newship->forceCancelDock=FALSE;
    newship->hyperspacePing=FALSE;
    newship->gravwellTimeEffect=0.0f;
    newship->passiveAttackCancelTimer=0.0f;
    // Alex's Repair Droid stuff...
//    newship->pRepairDroids = memAlloc(sizeof(RepairDroid), "RDroid", NonVolatile);

    univInitSpaceObjPosRot((SpaceObj *)newship,shippos,FALSE);

    newship->tractorbeaminfo = 0;
    newship->tractorbeam_playerowner = NULL;
    newship->tractorPosinfo[0] = FALSE;
    newship->tractorPosinfo[1] = FALSE;
    newship->tractorPosinfo[2] = FALSE;
    newship->tractorPosinfo[3] = FALSE;
    newship->tractorstate = 0;            //info variable for tractorbeaming state
    newship->needNewLeader = FALSE;

    for(i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        newship->salvageNumTagged[i] = 0;
    }

    newship->clampInfo = NULL;

    newship->dontrotateever = FALSE;
    newship->dontapplyforceever = FALSE;
    newship->health = shipstaticinfo->maxhealth;
    newship->fuel = shipstaticinfo->maxfuel;
//    newship->resources = 0;
//    newship->aistate = 0;
//    newship->aistateattack = 0;
//    newship->hotKeyGroup = 0;

    newship->shipSinglePlayerGameInfo = spNewShipSinglePlayerGameInfo();

    InitializeEngineTrails(newship);

    InitializeShowingDamage(newship);

    if (shipstaticinfo->gunStaticInfo != NULL)
    {
        sdword numGuns = shipstaticinfo->gunStaticInfo->numGuns;
        Gun *gun;
        GunStatic *gunstatic;

        newship->gunInfo = memAlloc(sizeofGunInfo(numGuns),"gunInfo",NonVolatile);
        newship->gunInfo->numGuns = numGuns;

        for (i=0;i<numGuns;i++)
        {
            gun = &newship->gunInfo->guns[i];
            gunstatic = &shipstaticinfo->gunStaticInfo->gunstatics[i];
            gun->gunstatic = gunstatic;

            switch (gunstatic->guntype)
            {
                case GUN_MissileLauncher:
                    gun->numMissiles = gunstatic->maxMissiles;
                    // deliberately fall through
                case GUN_MineLauncher:
                    gun->numMissiles = gunstatic->maxMissiles;      //Use same var for mines
                case GUN_Gimble:
                case GUN_Fixed:
                    gun->gunheading = gunstatic->gunnormal;
                    break;

                case GUN_NewGimble:
                    univResetNewGimbleGun(gun);
                    break;
            }
            gun->lasttimefired = 0.0f;
            gun->burstStartTime = 0.0f;
            gun->burstState = 0;
            gun->gimblehandle = SOUND_NOTINITED;
        }
    }
    else
    {
        newship->gunInfo = NULL;
    }

    setSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)newship);

    if (shipstaticinfo->dockStaticInfo != NULL)
    {
        DockStaticInfo *dockstaticinfo = shipstaticinfo->dockStaticInfo;
        sdword numDockPoints = dockstaticinfo->numDockPoints;
        DockStaticPoint *curdockstaticpoint;
        DockPoint *curdockpoint;

        dbgAssert(numDockPoints > 0);

        newship->dockInfo = memAlloc(sizeofDockInfo(numDockPoints),"dockInfo",NonVolatile);
        newship->dockInfo->busyness = 0;
        newship->dockInfo->numDockPoints = numDockPoints;

        for (i=0,curdockpoint=&newship->dockInfo->dockpoints[0],curdockstaticpoint=&dockstaticinfo->dockstaticpoints[0];i<numDockPoints;i++,curdockpoint++,curdockstaticpoint++)
        {
            curdockpoint->dockstaticpoint = curdockstaticpoint;
            curdockpoint->thisDockBusy = 0;
        }
    }
    else
    {
        newship->dockInfo = NULL;
    }

    InitializeNavLights(newship);

    if (shipstaticinfo->canBuildShips | shipstaticinfo->canReceiveShipsPermanently)
    {
        newship->shipsInsideMe = memAlloc(sizeof(ShipsInsideMe),"ShipsInsideMe",NonVolatile);
        listInit(&newship->shipsInsideMe->insideList);
        newship->shipsInsideMe->FightersInsideme = 0;
        newship->shipsInsideMe->CorvettesInsideme = 0;
//        listInit(&newship->shipsInsideMe->launchList);
    }
    else
    {
        newship->shipsInsideMe = NULL;
    }

    if (shipstaticinfo->canBuildShips)
    {
        cmAddFactory(newship,shipstaticinfo->canBuildBigShips);
    }

    /* init the sound variables for this ship */
    soundEventInitStruct(&newship->soundevent);

    if (((ShipStaticInfo *)newship->staticinfo)->custshipheader.CustShipInit)
    {
        ((ShipStaticInfo *)newship->staticinfo)->custshipheader.CustShipInit(newship);
    }

    //lightning effect stuff
    newship->lightning[0] = NULL;
    newship->lightning[1] = NULL;

    //create the instance version of the hierarchy bindings
    newship->bindings = univMeshBindingsDupe(shipstaticinfo, newship);

    if (shipstaticinfo->madStatic != NULL)
    {
        newship->madBindings = (madanim *)extraPointer;
        extraPointer += sizeof(madanim);
        newship->madBindings->header = shipstaticinfo->madStatic->header;
        newship->madBindings->nCurrentAnim = -1;
        newship->madBindings->time = 0.0f;
        newship->madBindings->totalSize = madAssedSize;
        madAnimBindingsDupe(newship, shipstaticinfo, FALSE);

        //ship has mesh animations associated with it
        //so lets set its inital state.
        newship->madAnimationFlags = 0;
        newship->madGunStatus = 0;            //gun status //invalid states!
        newship->madWingStatus = 0;           //wing animation status
        newship->madSpecialStatus = 0;
        newship->madDoorStatus = MAD_STATUS_DOOR_CLOSED;
        newship->cuedAnimationIndex = 0;
        newship->cuedAnimationType = 0;

        if(built == 1)
        {
            madLinkInSetUpInitialBuiltMadState(newship);
        }
        else if(built == 0)
        {
            madLinkInSetUpInitialPlacedMadState(newship);
        }
        else if(built == 2)
        {
            //ignore since it wasn't built or placed (nis ship)
        }
        else
        {
            dbgFatalf(DBG_Loc,"\nUnknown create ship state.");
        }

    }

    //determine what color scheme to use
    if ((playerowner->playerIndex < MAX_MULTIPLAYER_PLAYERS) && (shipstaticinfo->teamColor[playerowner->playerIndex]))
    {                                                       //if this player's default scheme woudl be a good color
        newship->colorScheme = playerowner->playerIndex;
    }
    else
    {                                                       //else let's find any old color scheme
        for (i = 0; i < MAX_MULTIPLAYER_PLAYERS; i++)
        {
            if (shipstaticinfo->teamColor[i])
            {
                newship->colorScheme = i;
                break;
            }
        }
    }
    return newship;
}

/*-----------------------------------------------------------------------------
    Name        : univAddShip
    Description : adds a ship to the universe
    Inputs      : shiptype, shiprace, shippos, playerowner
    Outputs     :
    Return      : address of ship added
----------------------------------------------------------------------------*/
Ship *univAddShip(ShipType shiptype,ShipRace shiprace,vector *shippos,struct Player *playerowner,sdword built)
{
    Ship *newship = univCreateShip(shiptype,shiprace,shippos,playerowner,built);

    collAddSpaceObjToCollBlobs((SpaceObj *)newship);
    listAddNode(&universe.SpaceObjList,&(newship->objlink),newship);
    listAddNode(&universe.ShipList,&(newship->shiplink),newship);
    listAddNode(&universe.ImpactableList,&(newship->impactablelink),newship);

    //special case - don't want cryotrays to be selectable
    if (shiptype == CryoTray)
    {
        bitClear(newship->flags, SOF_Selectable);
    }

    return newship;
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelMinorObjs
    Description : Exclusive physics updating for what would seem to
                  be Asteroid ZERO's in the single player game?
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void univUpdateAllPosVelMinorObjs()
{
    Node *objnode;
    SpaceObj *spaceobj;

    if ((singlePlayerGame) && (singlePlayerGameInfo.asteroid0sCanMove))
    {
        objnode = universe.MinorSpaceObjList.head;

        while (objnode != NULL)
        {
            spaceobj = (SpaceObj *)listGetStructOfNode(objnode);
            physUpdateObjPosVelBasic(spaceobj,universe.phystimeelapsed);
            objnode = objnode->next;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelBullets
    Description : Exclusive physics updating for bulletes
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelBullets()
{
    Node *bulletnode = universe.BulletList.head;
    Node *deletenode;
    Bullet *bullet;
    bool addEffect = FALSE;
    etglod *etgLOD;
    sdword LOD;
    etgeffectstatic *stat;
    Effect *effect;

    while (bulletnode != NULL)
    {
        bullet = (Bullet *)listGetStructOfNode(bulletnode);

        if ((bullet->flags & SOF_DontApplyPhysics) == 0)
        {
            if (physUpdateBulletPosVel(bullet,universe.phystimeelapsed))
            {
                //bullet has died so clean it up

                //turn off sound of beam
                if (bullet->bulletType == BULLET_Beam)
                {
                    if ((bullet->owner != NULL) && (bullet->gunowner != NULL))
                    {
                        soundEventBurstStop(bullet->owner, bullet->gunowner);
                    }
                }

                //delete effect associated with bullet
                effect = bullet->effect;
#if ETG_DISABLEABLE
                if (effect != NULL && etgEffectsEnabled)
#else
                if (effect != NULL)
#endif
                {
                    if (bitTest(((etgeffectstatic*)effect->staticinfo)->specialOps, ESO_SelfDeleting))
                    {                               //if the effect can delete itself
                        ((real32 *)effect->variable)[ETG_BulletDurationParam] =
                            effect->timeElapsed;    //time it out
                        effect->owner = NULL;
                        bitClear(effect->flags, SOF_AttachPosition);
                        bitClear(effect->flags, SOF_AttachCoordsys);
                        bitClear(effect->flags, SOF_AttachVelocity);
                    }
                    else
                    {
                        etgEffectDelete(effect);
                        univRemoveObjFromRenderList((SpaceObj *)(effect));
                        listDeleteNode(&effect->objlink);
                        addEffect = TRUE;
                    }
                }
                deletenode = bulletnode;
                bulletnode = bulletnode->next;

                if(bullet->bulletType == BULLET_SpecialBurst)
                {
                    etgLOD = etgSpecialPurposeEffectTable[EGT_BURST_EXPLOSION_EFFECT];

                    if (etgLOD != NULL)
                    {
                        LOD = bullet->currentLOD;

                        if (LOD >= etgLOD->nLevels)
                        {
                            stat = NULL;
                        }
                        else
                        {
                            stat = etgLOD->level[LOD];
                        }
                    }
                    else
                    {
                        stat = NULL;
                    }

                    if (addEffect)
                    {
#if ETG_DISABLEABLE
                        if (stat != NULL && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
                        if (stat != NULL && !etgFrequencyExceeded(stat))
#endif
                        {
                            vector velocity;
                            matrix coordsys = bullet->rotinfo.coordsys;
                            velocity.x = 0.0f;
                            velocity.y = 0.0f;
                            velocity.z = 0.0f;

                            etgEffectCreate(stat, NULL, &bullet->posinfo.position, &velocity, &coordsys, 1.0f, EAF_AllButNLips, 0);
                        }
                    }

                    pointExplosionInSpace(&bullet->posinfo.position,burstRadius,bullet->damage,bullet->playerowner->playerIndex);
                }

                //Defense Fighter 'hack' to remove its reference
                if(bitTest(bullet->SpecialEffectFlag, 0x0002))
                {    //bullet is possibly being targetted by a defense fighter...remove that reference
                    DefenseFighterBulletRemoval(bullet);
                }

                bobObjectDied(((SpaceObj *)bullet),&universe.collBlobList);
                listRemoveNode(&bullet->bulletlink);
                univRemoveObjFromRenderList(((SpaceObj *)bullet));
                listDeleteNode(&bullet->objlink);
                continue;
            }
        }
        bulletnode = bulletnode->next;
    }
}
/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelMissiles
    Description : Exclusive physics updating for misiles
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelMissiles()
{
    Node *misnode = universe.MissileList.head;
    Missile *missile;
    bool deleteflag;

    while (misnode != NULL)
    {
        missile = (Missile *)listGetStructOfNode(misnode);

        if ((missile->flags & SOF_DontApplyPhysics) == 0)
        {
            if(missile->missileType == MISSILE_Regular)
            {   //if missileType is a normal missile
                deleteflag = aishipGuideMissile(missile);
            }
            else if(missile->missileType == MISSILE_Mine)
            {   //if missiletype is a mine
                deleteflag = aishipGuideMine(missile);
            }
            else
            {
                deleteflag = FALSE;
                dbgAssert(FALSE);
            }

            physUpdateObjPosVelMissile(missile,universe.phystimeelapsed);
            if ((deleteflag) && ((((SpaceObj *)missile)->flags & SOF_Dead) == 0))
            {
                AddMissileToDeleteMissileList(missile, -1);
            }
        }
        misnode = misnode->next;
    }
}
/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelDerelicts
    Description : Exclusive physics updating for derelicts
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelDerelicts()
{
    Node *objnode = universe.DerelictList.head;
    Derelict *derelict;

    while (objnode != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(objnode);

        if ((derelict->flags & SOF_DontApplyPhysics) == 0)
        {
            if ((derelict->flags & SOF_NISShip) == 0)
            {
                physUpdateObjPosVelDerelicts(derelict,universe.phystimeelapsed);
            }
        }
        objnode = objnode->next;
    }
}
/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelResources
    Description : Exclusive physics updating for resources
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelResources()
{
    Node *objnode = universe.ResourceList.head;
    Resource *resource;

    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);

        if ((resource->flags & (SOF_DontApplyPhysics|SOF_NISShip)) == 0)
        {
            physUpdateObjPosVelBasic((SpaceObj *)resource,universe.phystimeelapsed);
        }

        if (resource->resourceNotAccessible > 0)
        {
            resource->resourceNotAccessible--;
        }

        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelShips
    Description : Exclusive physics updating for ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelShips()
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    growSelectReset(&universe.HousekeepShipList);
    if(ClampedShipList.selection == NULL)
    {
        growSelectInit(&ClampedShipList);
    }
    growSelectReset(&ClampedShipList);

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);

        if ((ship->flags & SOF_DontApplyPhysics) == 0)
        {
            if ((ship->flags & SOF_NISShip) == 0)
            {
                univCheckShipState(ship);

                if(bitTest(ship->flags, SOF_Slaveable))
                {    //object is slaveable
                    if(bitTest(ship->slaveinfo->flags,SF_SLAVE))
                    {   //if object is a slave
                        goto nowdosetupforshipsShipCustomFunc;
                    }
                    else
                    {   //object should be a master
                        //update slaves of object now
                        physUpdateObjPosVelShip(ship,universe.phystimeelapsed);
                        dockUpdateSlaves(ship);
                        //update master
                        goto nowdosetupforshipsShipCustomFunc;
                    }
                }
                if(((SpaceObjRotImpTargGuidance *)ship)->clampInfo != NULL)
                {
                    //object has clampinfo, should be updated
                    //updateClampedObject((SpaceObjRotImpTargGuidance *)spaceobj);
                    growSelectAddShip(&ClampedShipList,ship);
                    goto nowdosetupforshipsShipCustomFunc;
                }
                physUpdateObjPosVelShip(ship,universe.phystimeelapsed);

nowdosetupforshipsShipCustomFunc:
                univSetupShipForControl(ship);
            }
        }
        else
        {
            univMinorSetupShipForControl(ship);
        }
        //update mesh animations
        if (ship->madBindings != NULL)
        {
            if(ship->madAnimationFlags & MAD_ANIMATION_NEED_PROC)
            {
                madLinkInUpdateMeshAnimations(ship);
            }
            else if (ship->madBindings->nCurrentAnim != -1)
            {
                madAnimationUpdate(ship, universe.phystimeelapsed);
            }
        }


        objnode = objnode->next;
    }

    {
        Ship *obj;
        SelectCommand *clampedShipSelection;
        sdword i;
        sdword numShips;

        //update those poor baby clamped ships!
        clampedShipSelection = ClampedShipList.selection;
        numShips = clampedShipSelection->numShips;

        for (i=0;i<numShips;i++)
        {
            obj = clampedShipSelection->ShipPtr[i];
            if (obj)    //???????
            {
                if(obj->clampInfo != NULL)
                {
                    //do this check again because it is possible for a ship to become unclamped at some point down the road...
                    updateClampedObject((SpaceObjRotImpTargGuidance *)obj);
                }
            }
        }
    }

    {
        Ship *obj;
        SelectCommand *housekeepSelection;
        sdword i;
        sdword numShips;

        housekeepSelection = universe.HousekeepShipList.selection;
        numShips = housekeepSelection->numShips;

        for (i=0;i<numShips;i++)
        {
            obj = housekeepSelection->ShipPtr[i];
            if (obj)
            {
                obj->staticinfo->custshipheader.CustShipHousekeep(obj);
            }
        }
    }

    growSelectReset(&universe.HousekeepShipList);       // don't need to keep track of them anymore
    growSelectReset(&ClampedShipList);       // don't need to keep track of them anymore
}

/*-----------------------------------------------------------------------------
    Name        : univPausedUpdateAllPosVelShips
    Description : A minor version of the above function that only does update
                    operations appropriate for when the universe is paused
                    by nisUniversePaused.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univPausedUpdateAllPosVelShips(void)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        univMinorSetupShipForControl(ship);
        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelEffects
    Description : Exclusive physics updating for Effects -
                  must update effects AFTER anything that can be an
                  effect owner (be on safe side and do it last)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelEffects()
{
    Node *objnode = universe.effectList.head;
    Node *deletenode;
    Effect *effect;

    while (objnode != NULL)
    {
        effect = (Effect *)listGetStructOfNode(objnode);

        if ((effect->flags & SOF_DontApplyPhysics) == 0)
        {
            if (etgEffectUpdate(effect, universe.phystimeelapsed))
            {
                deletenode = objnode;
                objnode = objnode->next;

                etgEffectDelete(effect);
                univRemoveObjFromRenderList((SpaceObj *)effect);
                listDeleteNode(&effect->objlink);
                continue;
            }
        }
        objnode = objnode->next;
    }

}

/*-----------------------------------------------------------------------------
    Name        : MakeNewAsteroidStaticInfo
    Description : makes a new asteroid staticinfo based on its scaling
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeNewAsteroidStaticInfo(Asteroid *asteroid)
{
    AsteroidStaticInfo *originalAsteroidStaticInfo = &asteroidStaticInfos[asteroid->asteroidtype];
    AsteroidStaticInfo *newAsteroidStaticInfo;
    StaticHeader *newStaticHeader;
    StaticHeader *originalStaticHeader;
    StaticCollInfo *newStaticCollInfo;
    StaticCollInfo *originalStaticCollInfo;
    real32 scaling = asteroid->scaling;

    dbgAssert(scaling > 0.0f);

    if ((asteroid->flags & SOF_StaticInfoIsDynamic) == 0)
    {
        asteroid->staticinfo = memAlloc(sizeof(AsteroidStaticInfo),"aststatic",NonVolatile);
        *((AsteroidStaticInfo *)(asteroid->staticinfo)) = asteroidStaticInfos[asteroid->asteroidtype];  // structure copy
        asteroid->flags |= SOF_StaticInfoIsDynamic;
    }

    newAsteroidStaticInfo = (AsteroidStaticInfo *)asteroid->staticinfo;

    newAsteroidStaticInfo->harvestableByMultipleShips = (udword)(((real32)originalAsteroidStaticInfo->harvestableByMultipleShips) * scaling);
    if (newAsteroidStaticInfo->harvestableByMultipleShips == 0)
    {
        newAsteroidStaticInfo->harvestableByMultipleShips = 1;
    }

    newStaticHeader = &newAsteroidStaticInfo->staticheader;
    originalStaticHeader = &originalAsteroidStaticInfo->staticheader;

    newStaticHeader->mass = originalStaticHeader->mass * scaling;
    newStaticHeader->momentOfInertiaX = originalStaticHeader->momentOfInertiaX * scaling;
    newStaticHeader->momentOfInertiaY = originalStaticHeader->momentOfInertiaY * scaling;
    newStaticHeader->momentOfInertiaZ = originalStaticHeader->momentOfInertiaZ * scaling;

    newStaticHeader->oneOverMass =             1.0f /      newStaticHeader->mass;
    newStaticHeader->oneOverMomentOfInertiaX = 1.0f /      newStaticHeader->momentOfInertiaX;
    newStaticHeader->oneOverMomentOfInertiaY = 1.0f /      newStaticHeader->momentOfInertiaY;
    newStaticHeader->oneOverMomentOfInertiaZ = 1.0f /      newStaticHeader->momentOfInertiaZ;

    newStaticCollInfo = &newStaticHeader->staticCollInfo;
    originalStaticCollInfo = &originalStaticHeader->staticCollInfo;

    newStaticCollInfo->originalcollspheresize = originalStaticCollInfo->originalcollspheresize;  // DO NOT CHANGE INTENTIONALLY
    newStaticCollInfo->approxcollspheresize = originalStaticCollInfo->approxcollspheresize * scaling;
    newStaticCollInfo->avoidcollspheresize = originalStaticCollInfo->avoidcollspheresize * scaling;
    newStaticCollInfo->avoidcollspherepad = originalStaticCollInfo->avoidcollspherepad * scaling;
    newStaticCollInfo->collspheresize = originalStaticCollInfo->collspheresize * scaling;
    newStaticCollInfo->collspheresizeSqr = newStaticCollInfo->collspheresize * newStaticCollInfo->collspheresize;

    vecScalarMultiply(newStaticCollInfo->collsphereoffset,originalStaticCollInfo->collsphereoffset,scaling);
    vecScalarMultiply(newStaticCollInfo->collrectoffset,originalStaticCollInfo->collrectoffset,scaling);

    newStaticCollInfo->forwardlength = originalStaticCollInfo->forwardlength * scaling;
    newStaticCollInfo->rightlength = originalStaticCollInfo->rightlength * scaling;
    newStaticCollInfo->uplength = originalStaticCollInfo->uplength * scaling;

    newStaticCollInfo->diagonallength = originalStaticCollInfo->diagonallength * scaling;

    collUpdateCollRectangle((SpaceObjRotImp *)asteroid);
}

/*-----------------------------------------------------------------------------
    Name        : MakeNewDustCloudStaticInfo
    Description : makes a new dustcloud staticinfo based on its scaling
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeNewDustCloudStaticInfo(DustCloud *dustcloud)
{
    DustCloudStaticInfo *originalDustCloudStaticInfo = &dustcloudStaticInfos[dustcloud->dustcloudtype];
    DustCloudStaticInfo *newDustCloudStaticInfo;
    StaticHeader *newStaticHeader;
    StaticHeader *originalStaticHeader;
    StaticCollInfo *newStaticCollInfo;
    StaticCollInfo *originalStaticCollInfo;
    real32 scaling = dustcloud->scaling;

    dbgAssert(scaling > 0.0f);

    if ((dustcloud->flags & SOF_StaticInfoIsDynamic) == 0)
    {
        dustcloud->staticinfo = memAlloc(sizeof(DustCloudStaticInfo),"dcloudstatic",NonVolatile);
        *((DustCloudStaticInfo *)(dustcloud->staticinfo)) = dustcloudStaticInfos[dustcloud->dustcloudtype];  // structure copy
        dustcloud->flags |= SOF_StaticInfoIsDynamic;
    }

    newDustCloudStaticInfo = (DustCloudStaticInfo *)dustcloud->staticinfo;

    newDustCloudStaticInfo->harvestableByMultipleShips = (udword)(((real32)originalDustCloudStaticInfo->harvestableByMultipleShips) * scaling);
    if (newDustCloudStaticInfo->harvestableByMultipleShips == 0)
    {
        newDustCloudStaticInfo->harvestableByMultipleShips = 1;
    }

    newStaticHeader = &newDustCloudStaticInfo->staticheader;
    originalStaticHeader = &originalDustCloudStaticInfo->staticheader;

    newStaticHeader->mass = originalStaticHeader->mass * scaling;
    newStaticHeader->momentOfInertiaX = originalStaticHeader->momentOfInertiaX * scaling;
    newStaticHeader->momentOfInertiaY = originalStaticHeader->momentOfInertiaY * scaling;
    newStaticHeader->momentOfInertiaZ = originalStaticHeader->momentOfInertiaZ * scaling;

    newStaticHeader->oneOverMass =             1.0f /      newStaticHeader->mass;
    newStaticHeader->oneOverMomentOfInertiaX = 1.0f /      newStaticHeader->momentOfInertiaX;
    newStaticHeader->oneOverMomentOfInertiaY = 1.0f /      newStaticHeader->momentOfInertiaY;
    newStaticHeader->oneOverMomentOfInertiaZ = 1.0f /      newStaticHeader->momentOfInertiaZ;

    newStaticCollInfo = &newStaticHeader->staticCollInfo;
    originalStaticCollInfo = &originalStaticHeader->staticCollInfo;

    newStaticCollInfo->originalcollspheresize = originalStaticCollInfo->originalcollspheresize;  // DO NOT CHANGE INTENTIONALLY
    newStaticCollInfo->approxcollspheresize = originalStaticCollInfo->approxcollspheresize * scaling;
    newStaticCollInfo->avoidcollspheresize = originalStaticCollInfo->avoidcollspheresize * scaling;
    newStaticCollInfo->avoidcollspherepad = originalStaticCollInfo->avoidcollspherepad * scaling;
    newStaticCollInfo->collspheresize = originalStaticCollInfo->collspheresize * scaling;
    newStaticCollInfo->collspheresizeSqr = newStaticCollInfo->collspheresize * newStaticCollInfo->collspheresize;

    vecScalarMultiply(newStaticCollInfo->collsphereoffset,originalStaticCollInfo->collsphereoffset,scaling);
    vecScalarMultiply(newStaticCollInfo->collrectoffset,originalStaticCollInfo->collrectoffset,scaling);

    newStaticCollInfo->forwardlength = originalStaticCollInfo->forwardlength * scaling;
    newStaticCollInfo->rightlength = originalStaticCollInfo->rightlength * scaling;
    newStaticCollInfo->uplength = originalStaticCollInfo->uplength * scaling;

    newStaticCollInfo->diagonallength = originalStaticCollInfo->diagonallength * scaling;

    collUpdateCollRectangle((SpaceObjRotImp *)dustcloud);
}

/*-----------------------------------------------------------------------------
    Name        : MakeNewGasCloudStaticInfo
    Description : makes a new gascloud staticinfo based on its scaling
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeNewGasCloudStaticInfo(GasCloud *gascloud)
{
    GasCloudStaticInfo *originalGasCloudStaticInfo = &gascloudStaticInfos[gascloud->gascloudtype];
    GasCloudStaticInfo *newGasCloudStaticInfo;
    StaticHeader *newStaticHeader;
    StaticHeader *originalStaticHeader;
    StaticCollInfo *newStaticCollInfo;
    StaticCollInfo *originalStaticCollInfo;
    real32 scaling = gascloud->scaling;

    dbgAssert(scaling > 0.0f);

    if ((gascloud->flags & SOF_StaticInfoIsDynamic) == 0)
    {
        gascloud->staticinfo = memAlloc(sizeof(GasCloudStaticInfo),"gcloudstatic",NonVolatile);
        *((GasCloudStaticInfo *)(gascloud->staticinfo)) = gascloudStaticInfos[gascloud->gascloudtype];  // structure copy
        gascloud->flags |= SOF_StaticInfoIsDynamic;
    }

    newGasCloudStaticInfo = (GasCloudStaticInfo *)gascloud->staticinfo;

    newGasCloudStaticInfo->harvestableByMultipleShips = (udword)(((real32)originalGasCloudStaticInfo->harvestableByMultipleShips) * scaling);
    if (newGasCloudStaticInfo->harvestableByMultipleShips == 0)
    {
        newGasCloudStaticInfo->harvestableByMultipleShips = 1;
    }

    newStaticHeader = &newGasCloudStaticInfo->staticheader;
    originalStaticHeader = &originalGasCloudStaticInfo->staticheader;

    newStaticHeader->mass = originalStaticHeader->mass * scaling;
    newStaticHeader->momentOfInertiaX = originalStaticHeader->momentOfInertiaX * scaling;
    newStaticHeader->momentOfInertiaY = originalStaticHeader->momentOfInertiaY * scaling;
    newStaticHeader->momentOfInertiaZ = originalStaticHeader->momentOfInertiaZ * scaling;

    newStaticHeader->oneOverMass =             1.0f /      newStaticHeader->mass;
    newStaticHeader->oneOverMomentOfInertiaX = 1.0f /      newStaticHeader->momentOfInertiaX;
    newStaticHeader->oneOverMomentOfInertiaY = 1.0f /      newStaticHeader->momentOfInertiaY;
    newStaticHeader->oneOverMomentOfInertiaZ = 1.0f /      newStaticHeader->momentOfInertiaZ;

    newStaticCollInfo = &newStaticHeader->staticCollInfo;
    originalStaticCollInfo = &originalStaticHeader->staticCollInfo;

    newStaticCollInfo->originalcollspheresize = originalStaticCollInfo->originalcollspheresize;  // DO NOT CHANGE INTENTIONALLY
    newStaticCollInfo->approxcollspheresize = originalStaticCollInfo->approxcollspheresize * scaling;
    newStaticCollInfo->avoidcollspheresize = originalStaticCollInfo->avoidcollspheresize * scaling;
    newStaticCollInfo->avoidcollspherepad = originalStaticCollInfo->avoidcollspherepad * scaling;
    newStaticCollInfo->collspheresize = originalStaticCollInfo->collspheresize * scaling;
    newStaticCollInfo->collspheresizeSqr = newStaticCollInfo->collspheresize * newStaticCollInfo->collspheresize;

    vecScalarMultiply(newStaticCollInfo->collsphereoffset,originalStaticCollInfo->collsphereoffset,scaling);
    vecScalarMultiply(newStaticCollInfo->collrectoffset,originalStaticCollInfo->collrectoffset,scaling);

    newStaticCollInfo->forwardlength = originalStaticCollInfo->forwardlength * scaling;
    newStaticCollInfo->rightlength = originalStaticCollInfo->rightlength * scaling;
    newStaticCollInfo->uplength = originalStaticCollInfo->uplength * scaling;

    newStaticCollInfo->diagonallength = originalStaticCollInfo->diagonallength * scaling;

    collUpdateCollRectangle((SpaceObjRotImp *)gascloud);
}

#if 0
/*-----------------------------------------------------------------------------
    Name        : ObjectsAreMovingCloserTogether
    Description : determines if objects are moving closer together based on
                  current positions and velocity
    Inputs      : ship1, ship2, curdistsquared (between ship1,ship2)
    Outputs     :
    Return      : TRUE if objects are moving closer together, FALSE otherwise
----------------------------------------------------------------------------*/
bool ObjectsAreMovingCloserTogether(SpaceObjRotImp *obj1,SpaceObjRotImp *obj2,real32 curdistsquared)
{
    vector newpos1,newpos2;
    vector d1,d2;
    vector newdist;

    vecScalarMultiply(d1,obj1->posinfo.velocity,universe.phystimeelapsed);
    vecScalarMultiply(d2,obj2->posinfo.velocity,universe.phystimeelapsed);
    vecAdd(newpos1,obj1->collInfo.collPosition,d1);
    vecAdd(newpos2,obj2->collInfo.collPosition,d2);
    vecSub(newdist,newpos2,newpos1);

    if (vecMagnitudeSquared(newdist) < curdistsquared)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : ApplyDamageToCollidingObjects
    Description : Applies collision damage to two colliding objects
    Inputs      : obj1, obj2, distvector, dist
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define HYPERING_IN_DAMAGE    twHyperingInDamage
#define HYPERED_INTO_DAMAGE   twHyperedIntoDamage

void ApplyDamageToCollidingObjects(SpaceObjRotImpTarg *obj1,SpaceObjRotImpTarg *obj2,vector *distvector,real32 dist)
{
    real32 damage;
    real32 damage2;
    real32 mindamage;
    real32 maxdamage;
    StaticInfoHealth *staticobj1;
    StaticInfoHealth *staticobj2;
    real32 veltangentfactor;
    vector obj2VelRelToObj1;

    if ((obj1->attributes^obj2->attributes) & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage))
    {
        // if one or the other but not both objects have KillerCollDamage
        if (obj1->attributes & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage))
        {
            ApplyDamageToTarget(obj1,min(obj2->health,obj1->health),0,DEATH_Killed_By_Collision,99);
            ApplyDamageToTarget(obj2,(obj1->attributes & ATTRIBUTES_HeadShotKillerCollDamage) ? attrHEADSHOTKillerCollDamage : attrKillerCollDamage,0,DEATH_Killed_By_Collision,99); // and do lots of damage to thing it hit
            return;
        }
        else if (obj2->attributes & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage))
        {
            ApplyDamageToTarget(obj2,min(obj2->health,obj1->health),0,DEATH_Killed_By_Collision,99);
            ApplyDamageToTarget(obj1,(obj2->attributes & ATTRIBUTES_HeadShotKillerCollDamage) ? attrHEADSHOTKillerCollDamage : attrKillerCollDamage,0,DEATH_Killed_By_Collision,99); // and do lots of damage to thing it hit
            return;
        }
    }

    vecSub(obj2VelRelToObj1,obj2->posinfo.velocity,obj1->posinfo.velocity);
    veltangentfactor = ABS(vecDotProduct(*distvector,obj2VelRelToObj1));
    veltangentfactor /= (COLLDAMAGE_VELOCITY_TANGENT_SCALE * dist);
    // now veltangentfactor is equal to k * |vel| cos(theta) where theta is angle between distvector and obj2VelRelToObj1

    damage = min(obj1->health,obj2->health);
    damage = (damage * veltangentfactor);

    staticobj1 = (StaticInfoHealth *)obj1->staticinfo;
    staticobj2 = (StaticInfoHealth *)obj2->staticinfo;
    mindamage = min(staticobj1->minCollDamage,staticobj2->minCollDamage);
    maxdamage = min(staticobj1->maxCollDamage,staticobj2->maxCollDamage);

    if (damage < mindamage)
    {
        damage = mindamage;
    }
    else if (damage > maxdamage)
    {
        damage = maxdamage;
    }
    damage2 = damage;

    if(obj1->objtype == OBJ_ShipType)
    {
       if(((Ship *)obj1)->shiptype == P2Mothership)
       {
          damage /= 3.0f;
       }
       else if (((Ship *)obj1)->shiptype == FloatingCity)
       {
          return;       // Floating city's are peaceful and magical - don't do any collision damage
       }
       else
       {
          CommandToDo *obj1command=getShipAndItsCommand(&universe.mainCommandLayer,((Ship *)obj1));
          if(obj1command != NULL)
          {
             if(obj1command->ordertype.order == COMMAND_MP_HYPERSPACEING)
             {
                //if ship is hyperspaceing IN, modify its damage
                if(obj1command->hyperspaceState == COM_HYPERSPACE_IN)
                {
                   damage *= HYPERING_IN_DAMAGE;
                   damage2 *= HYPERED_INTO_DAMAGE;
                }
             }
          }
       }
    }

    if(obj2->objtype == OBJ_ShipType)
    {
       if(((Ship *)obj2)->shiptype == P2Mothership)
       {
          damage2 /= 3.0f;
       }
       else if (((Ship *)obj2)->shiptype == FloatingCity)
       {
          return;       // Floating city's are peaceful and magical - don't do any collision damage
       }
       else
       {
          CommandToDo *obj2command=getShipAndItsCommand(&universe.mainCommandLayer,((Ship *)obj2));
          if(obj2command != NULL)
          {
             if(obj2command->ordertype.order == COMMAND_MP_HYPERSPACEING)
             {
                //if ship is hyperspaceing IN, modify its damage
                if(obj2command->hyperspaceState == COM_HYPERSPACE_IN)
                {
                   damage2 *= HYPERING_IN_DAMAGE;
                   damage *= HYPERED_INTO_DAMAGE;
                }
             }
          }
       }
    }



    ApplyDamageToTarget(obj1,damage,0,DEATH_Killed_By_Collision,99);
    ApplyDamageToTarget(obj2,damage2,0,DEATH_Killed_By_Collision,99);
}

bool ObjectsHaveCollidedLately(SpaceObjRotImpTarg *obj1,SpaceObjRotImpTarg *obj2)
{
    if ( ((universe.totaltimeelapsed - obj1->lasttimecollided) < COLLISION_CLEAR_TIME) &&
         ((universe.totaltimeelapsed - obj2->lasttimecollided) < COLLISION_CLEAR_TIME) )
    {
        return TRUE;
    }
    obj1->lasttimecollided =  universe.totaltimeelapsed;
    obj2->lasttimecollided =  universe.totaltimeelapsed;
    return FALSE;
}

bool ShouldDoGlancingCollision(SpaceObjRotImpTarg *fastobj,real32 fastobjvelmag2,SpaceObjRotImpTarg *slowobj,real32 slowobjvelmag2)
{
    // don't do glancing collision for kamikaze stuff
    if(fastobj->objtype == OBJ_ShipType)
        if(bitTest(((Ship *)fastobj)->specialFlags,SPECIAL_Kamikaze))
            return FALSE;

    if(slowobj->objtype == OBJ_ShipType)
        if(bitTest(((Ship *)slowobj)->specialFlags,SPECIAL_Kamikaze))
            return FALSE;

    if ((fastobjvelmag2/slowobjvelmag2) >= GLANCE_COLLISION_VEL_RATIO)
    {
        return TRUE;
    }

    if (vecDotProduct(fastobj->posinfo.velocity,slowobj->posinfo.velocity) < 0)
    {
        return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : ObjectsCollided
    Description : Handles momentum transferral of objects colliding
    Inputs      : obj1, obj2, distvector, dist, distsquared
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ObjectsCollided(SpaceObjRotImpTarg *obj1,SpaceObjRotImpTarg *obj2,real32 colldist,vector *distvector,real32 dist,real32 distsquared)
{
    vector collforce;
    vector cforce1;
    vector cforce2;
    vector repulseoverlap;
    real32 temp;
    real32 repulse = (colldist - dist) * COLLISION_REPULSE_FACTOR / dist;
    vector up = { 0.0f, 0.0f, 1.0f };
    real32 collforcemag;
    vector tangforce;

    real32 obj1velmag2;
    real32 obj2velmag2;

    real32 slowobjvelmag2;
    real32 fastobjvelmag2;

    SpaceObjRotImpTarg *slowobj;
    SpaceObjRotImpTarg *fastobj;

    vector fastesttoslowest;

    vector leftrightdecidevec;

#ifdef DEBUG_COLLISIONS
    dbgMessagef("\n%x and %x collided, Repulsion %f",(udword)obj1,(udword)obj2,repulse);
#endif

    if (!ObjectsHaveCollidedLately(obj1,obj2))
    {
        // decide which ship is fastest ship in collision

        obj1velmag2 = vecMagnitudeSquared(obj1->posinfo.velocity);
        obj2velmag2 = vecMagnitudeSquared(obj2->posinfo.velocity);

        if (obj1velmag2 > obj2velmag2)
        {
            fastobj = obj1; fastobjvelmag2 = obj1velmag2;
            slowobj = obj2; slowobjvelmag2 = obj2velmag2;
        }
        else
        {
            fastobj = obj2; fastobjvelmag2 = obj2velmag2;
            slowobj = obj1; slowobjvelmag2 = obj1velmag2;
        }

        // collforce = (m2v2 - m1v1)/t
        vecScalarMultiply(cforce1,obj2->posinfo.velocity,obj2->staticinfo->staticheader.mass);
        vecScalarMultiply(cforce2,obj1->posinfo.velocity,obj1->staticinfo->staticheader.mass);
        vecSub(collforce,cforce1,cforce2);
        vecDivideByScalar(collforce,universe.phystimeelapsed,temp);

        if (ShouldDoGlancingCollision(fastobj,fastobjvelmag2,slowobj,slowobjvelmag2))
        {
#ifdef DEBUG_COLLISIONS
        dbgMessage("\nGlancing Collision");
#endif
            if ((collforce.x == 0.0f) && (collforce.y == 0.0f))
            {
                collforce.x = 0.1f;     // so cross product with up doesn't blow up
            }

            collforcemag = fsqrt(vecMagnitudeSquared(collforce));

            // make force tangential:

            vecCrossProduct(tangforce,collforce,up);
            vecNormalizeToLength(&tangforce,collforcemag);

            // decide which way to apply tangential force based on if "velocity" vector of fastest obj is left or right
            // of vector from fastestobj to slowestobj

            vecSub(fastesttoslowest,slowobj->collInfo.collPosition,fastobj->collInfo.collPosition);

            vecCrossProduct(leftrightdecidevec,fastesttoslowest,fastobj->posinfo.velocity);

            if (leftrightdecidevec.z > 0)
            {
                vecCopyAndNegate(collforce,tangforce);
            }
            else
            {
                collforce = tangforce;
            }
        }
        else
        {
#ifdef DEBUG_COLLISIONS
        dbgMessage("\nXchanging Velocities");
#endif
        }

        if (!fastobj->staticinfo->staticheader.immobile)
        {
            if(!(fastobj->attributes & ATTRIBUTES_KillerCollDamage))
            {
                vecAddTo(fastobj->posinfo.force,collforce);
            }
        }
        if (!slowobj->staticinfo->staticheader.immobile)
        {
            if(!(slowobj->attributes & ATTRIBUTES_KillerCollDamage))
            {
                vecSubFrom(slowobj->posinfo.force,collforce);
            }
        }
    }
    else
    {
        // add a repulsion factor if objects overlap, so they don't stick together
        vecScalarMultiply(repulseoverlap,*distvector,repulse);
        if (!obj2->staticinfo->staticheader.immobile)
        {
            if(!(obj2->attributes & ATTRIBUTES_KillerCollDamage))
            {
                vecAddTo(obj2->posinfo.force,repulseoverlap);
            }
        }
        if (!obj1->staticinfo->staticheader.immobile)
        {
            if(!(obj1->attributes & ATTRIBUTES_KillerCollDamage))
            {
               vecSubFrom(obj1->posinfo.force,repulseoverlap);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : ApplyCareen
    Description : Applies careen (random spin) to a ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ApplyCareen(Ship *ship)
{
    // apply three random torques, to ship,

    ship->rotinfo.torque.x += frandombetween(YAWCAREENLO,YAWCAREENHI);
    ship->rotinfo.torque.y += frandombetween(PITCHCAREENLO,PITCHCAREENHI);
    ship->rotinfo.torque.z += frandombetween(ROLLCAREENLO,ROLLCAREENHI);
}

void ApplyCareenRotationDirectly(Ship *ship)
{
    StaticHeader *staticheader = &ship->staticinfo->staticheader;
    vector a;

    // apply three random torques, to ship,

    a.x = frandombetween(YAWCRAZYLO,YAWCRAZYHI) * universe.phystimeelapsed; // * staticheader->oneOverMomentOfInertiaX;
    a.y = frandombetween(PITCHCRAZYLO,PITCHCRAZYHI) * universe.phystimeelapsed; // * staticheader->oneOverMomentOfInertiaY;
    a.z = frandombetween(ROLLCRAZYLO,ROLLCRAZYHI) * universe.phystimeelapsed; // * staticheader->oneOverMomentOfInertiaZ;

    // w = w + alpha*t
    vecAddTo(ship->rotinfo.rotspeed,a);

    // cap rotation speed
    vecCapVectorSloppy(&ship->rotinfo.rotspeed,staticheader->maxrot);
}

/*-----------------------------------------------------------------------------
    Name        : AddShipToDeleteShipList
    Description : adds ship to the DeleteShip list (for deletion at end of main loop)
    Inputs      : ship, damageType
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddShipToDeleteShipList(Ship *ship,GunSoundType damageType)
{
    DeleteShip *deleteShip;
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    ship->flags |= SOF_Dead;

    if (!shipstaticinfo->staticheader.immobile)
    {
        ApplyCareen(ship);
    }

    //if (shipstaticinfo->shipclass != CLASS_Fighter)
        MakeExplosionRockBumpables(ship,(damageType == -1));

    // add to delete ship list
    deleteShip = memAlloc(sizeof(DeleteShip),"DeleteShip",Pyrophoric);
    deleteShip->ship = ship;
    if (ship->health <= etgBigDeathFactor[shipstaticinfo->shiprace][shipstaticinfo->shipclass])
    {                                   //ship took a really big bong hit
        deleteShip->deathBy = (udword)etgDeathModeByGunType[damageType];
    }
    else
    {                                   //if it just barely died, use the small explosion
        deleteShip->deathBy = EDT_AccumDamage;
    }
    listAddNode(&universe.DeleteShipList,&(deleteShip->objlink),deleteShip);
}

/*-----------------------------------------------------------------------------
    Name        : AddResourceToDeleteResourceList
    Description : adds resource to the DeleteResource list (for deletion at end of main loop)
    Inputs      : resource
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddResourceToDeleteResourceList(Resource *resource)
{
    // add this resource to the delete resource list
    DeleteResource *deleteResource = memAlloc(sizeof(DeleteResource),"DeleteResource",Pyrophoric);

    resource->flags |= SOF_Dead;

    deleteResource->resource = resource;
    listAddNode(&universe.DeleteResourceList,&(deleteResource->objlink),deleteResource);
}

/*-----------------------------------------------------------------------------
    Name        : AddDerelictToDeleteDerelictList
    Description : adds derelict to the DeleteDerelict list (for deletion at end of main loop)
    Inputs      : derelict
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddDerelictToDeleteDerelictList(Derelict *derelict,GunSoundType damageType)
{
    // add this derelict to the delete derelict list
    DeleteDerelict *deleteDerelict = memAlloc(sizeof(DeleteDerelict),"DeleteDerelict",Pyrophoric);

    bobObjectDied((SpaceObj *)derelict,&universe.collBlobList);//can't have dead derelicts in the blobs
    derelict->flags |= SOF_Dead;

    deleteDerelict->derelict = derelict;
    if (derelict->health <= etgBigDeathFactorDerelict[derelict->derelicttype])
    {                                   //derelict took a really big bong hit
        deleteDerelict->deathBy = (udword)etgDeathModeByGunType[damageType];
    }
    else
    {                                   //if it just barely died, use the small explosion
        deleteDerelict->deathBy = EDT_AccumDamage;
    }
    listAddNode(&universe.DeleteDerelictList,&(deleteDerelict->objlink),deleteDerelict);
    //univRemoveFromWorldList(derelict);                      //it may be a planet; remove from the planet list if so.
}

/*-----------------------------------------------------------------------------
    Name        : AddMissileToDeleteMissileList
    Description : adds missile to the DeleteMissile list (for deletion at end of main loop)
    Inputs      : missile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddMissileToDeleteMissileList(Missile *missile,GunSoundType damageType)
{
    // add this missile to the delete missile list
    DeleteMissile *deleteMissile = memAlloc(sizeof(DeleteMissile),"DeleteMissile",Pyrophoric);

    missile->flags |= SOF_Dead;

    deleteMissile->missile = missile;
    deleteMissile->deathBy = damageType;
    listAddNode(&universe.DeleteMissileList,&(deleteMissile->objlink),deleteMissile);
}

/*-----------------------------------------------------------------------------
    Name        : AddTargetToDeleteList
    Description : adds target (ship, asteroid, missile) to appropriate delete list
    Inputs      : target, soundType (indicating type of death)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddTargetToDeleteList(SpaceObjRotImpTarg *target,GunSoundType soundType)
{
    switch (target->objtype)
    {
        case OBJ_ShipType:
            AddShipToDeleteShipList((Ship *)target,soundType);
            break;

        case OBJ_AsteroidType:
        case OBJ_NebulaType:
        case OBJ_GasType:
        case OBJ_DustType:
            AddResourceToDeleteResourceList((Resource *)target);
            break;

        case OBJ_MissileType:
            AddMissileToDeleteMissileList((Missile *)target, soundType);
            break;

        case OBJ_DerelictType:
            AddDerelictToDeleteDerelictList((Derelict *)target, soundType);
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

void addSlavesToDeleteList(Ship *master, GunSoundType deathtype)
{
    Node *slavenode;
    Ship *slave;

    dbgAssert(master->slaveinfo->flags & SF_MASTER);  //only master should have been added at first!
    slavenode = master->slaveinfo->slaves.head;
    master->slaveinfo->flags |= SF_DEAD;
    while(slavenode != NULL)
    {
        slave = (Ship *) listGetStructOfNode(slavenode);
        slave->slaveinfo->flags |= SF_DEAD;
        if(!(slave->flags & SOF_Dead))
            AddTargetToDeleteList((SpaceObjRotImpTarg *)slave,deathtype);
        slavenode = slavenode->next;
    }
}

void ApplyExplosionRockToTarget(SpaceObjRotImp *obj,real32 blaststrength)
{
    obj->rotinfo.torque.x += frandombetween(YAWEXPLOSIONROCKLO,YAWEXPLOSIONROCKHI) * blaststrength;
    obj->rotinfo.torque.y += frandombetween(PITCHEXPLOSIONROCKLO,PITCHEXPLOSIONROCKHI) * blaststrength;
    obj->rotinfo.torque.z += frandombetween(ROLLEXPLOSIONROCKLO,ROLLEXPLOSIONROCKHI) * blaststrength;
}

void ApplyRockToTarget(SpaceObjRotImpTarg *target,real32 projectilemass)
{
    target->rotinfo.torque.x += frandombetween(YAWROCKLO,YAWROCKHI) * projectilemass;
    target->rotinfo.torque.y += frandombetween(PITCHROCKLO,PITCHROCKHI) * projectilemass;
    target->rotinfo.torque.z += frandombetween(ROLLROCKLO,ROLLROCKHI) * projectilemass;
}

/*-----------------------------------------------------------------------------
    Name        : MakeExplosionRockBumpables
    Description : Make all impactable objects within blast radius get rocked
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeExplosionRockBumpables(Ship *ship,bool scuttle)
{
    Node *objnode = universe.ImpactableList.head;
    ShipStaticInfo *shipstatic = ship->staticinfo;
    SpaceObjRotImp *obj;
    vector distvector;
    real32 distsquared;
    real32 dist;
    real32 blastradius = shipstatic->staticheader.staticCollInfo.collspheresize * shipstatic->blastRadiusShockWave *
                         (scuttle ? BLAST_RADIUS_SCUTTLE_BONUS : 1.0f);
    real32 blastradiussqr = blastradius * blastradius;
    real32 blaststrength;
    real32 damage;
    real32 damageperc;

    while (objnode != NULL)
    {
        obj = (SpaceObjRotImp *) listGetStructOfNode(objnode);

        if ((obj->flags & (SOF_Impactable|SOF_NotBumpable)) != (SOF_Impactable))
        {
            goto nextnode;      // object must be impactable, bumpable
        }

        vecSub(distvector,obj->collInfo.collPosition,ship->collInfo.collPosition);

        if (!isBetweenInclusive(distvector.x,-blastradius,blastradius))
        {
            goto nextnode;
        }

        if (!isBetweenInclusive(distvector.y,-blastradius,blastradius))
        {
            goto nextnode;
        }

        if (!isBetweenInclusive(distvector.z,-blastradius,blastradius))
        {
            goto nextnode;
        }

        distsquared = vecMagnitudeSquared(distvector);
        if (distsquared > blastradiussqr)
        {
            goto nextnode;
        }

        if (obj == (SpaceObjRotImp *)ship)
        {
            goto nextnode;
        }

        if (obj->staticinfo->staticheader.immobile)
        {
            goto nextnode;
        }

        // obj is within blast radius, so it should get rocked

        if (distsquared < 1.0f)
        {
            distsquared = 1.0f;
        }
        dist = fsqrt(distsquared);
        blaststrength = 1.0f - (dist / blastradius);
        vecNormalizeToLength(&distvector,(scuttle ? BLAST_CONSTANT_SCUTTLE : BLAST_CONSTANT) * blaststrength);

        ApplyExplosionRockToTarget(obj,blaststrength);
        physApplyForceVectorToObj(obj,distvector);

        if (obj->flags & SOF_Targetable)
        {
            // calculate damage based on ship blowing up
            real32 health = (shipstatic->blastRadiusDamage != 0.0f) ? shipstatic->blastRadiusDamage : shipstatic->maxhealth;

            damageperc = (scuttle ? BLAST_DAMAGE_MAX_PERCENT_SCUTTLE : BLAST_DAMAGE_MAX_PERCENT) * blaststrength;
            damage =  health * damageperc;

            if (damage > 0)
            {
                if(ship->playerowner != NULL)
                    ApplyDamageToTarget((SpaceObjRotImpTarg *)obj,damage,0,DEATH_Killed_By_Player_Explosion,ship->playerowner->playerIndex);
                else
                    ApplyDamageToTarget((SpaceObjRotImpTarg *)obj,damage,0,DEATH_Killed_By_Dead_Player,99);
            }
        }

nextnode:
        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ApplyDamageToTarget
    Description : applies damage to target, possible resulting in target being added
                  to the delete list.
    Inputs      : damageHow - Defines how the damaged was taken:
                              DEATH_Killed_By_Collision    - collision
                              DEATH_Killed_By_Player - player ship attacking
                              (anything else is assumed to be something else and essentially nothing
                              to worry about)
                  playerIndex = playerIndex of playerdoing damage
    Outputs     :
    Return      : returns TRUE if damage was fatal damage
----------------------------------------------------------------------------*/
bool ApplyDamageToTarget(SpaceObjRotImpTarg *target,real32 damagetaken,GunSoundType soundType,sdword damageHow,sdword playerIndex)
{
    bool targetWasAlive = ((target->flags & SOF_Dead) == 0);
    real32 previousHealth = target->health;
    real32 maxHealth;

    if (nisIsRunning)
    {                                                       //do no damage during an NIS
        return FALSE;
    }

    if(target->flags & SOF_Slaveable)
    {    //target is a ship and is slaveable
         if(((Ship *)target)->slaveinfo->flags & SF_SLAVE)
         {    //object is a slave
             target = (SpaceObjRotImpTarg *) ((Ship *) target)->slaveinfo->Master;            //potential reprocussions
         }
    }
    target->health -= damagetaken;
    switch (target->objtype)
    {
        case OBJ_AsteroidType:
            if (AsteroidTakesDamage((Asteroid *)target,(sdword)damagetaken,targetWasAlive))
            {
                AddTargetToDeleteList(target,soundType);
                return TRUE;                    //used by effect
            }
            else
            {
                return FALSE;
            }
            break;

        case OBJ_DustType:
            switch (soundType)
            {
            case GS_LargeIonCannon:
            case GS_MediumIonCannon:
            case GS_SmallIonCannon:
            case GS_VeryLargeIonCannon:
                //charge, but don't take damage
                DustCloudChargesUp((DustCloud*)target, (sdword)damagetaken, targetWasAlive);
                return FALSE;
            default:
                break;
            }

#if 0
            if (DustCloudTakesDamage((DustCloud*)target, (sdword)damagetaken, targetWasAlive))
            {
                AddTargetToDeleteList(target, soundType);
                return TRUE;
            }
            else
#endif
            {
                return FALSE;
            }
            break;

        case OBJ_DerelictType:
            if (((Derelict *)target)->derelicttype == HyperspaceGate)
            {
                hsDerelictTakesDamage((Derelict *)target);
            }
            goto fallthrough;

        case OBJ_ShipType:
            switch(damageHow)
            {
                case DEATH_Killed_By_Player:
                case DEATH_Killed_By_Kamikaze:
                case DEATH_Killed_By_Player_Explosion:
                    universe.gameStats.playerStats[playerIndex].pointsOfDamageDoneToWho[((Ship *)target)->playerowner->playerIndex] += damagetaken;
                    break;
            }
            maxHealth = ((ShipStaticInfo *)target->staticinfo)->maxhealth;
            if (previousHealth /  maxHealth >= BAT_InTheRedFactor)
            {                                               //it wasn't in the red
                if (target->health / maxHealth < BAT_InTheRedFactor)
                {                                           //if it's just now in the red
                    bitSet(((Ship *)target)->chatterBits, BCB_DamageInTheRed);//flag as such
                }
            }
        // purposefully fall through
fallthrough:;
        default:
            if ((target->health <= 0) && (targetWasAlive))      // target just died, so add to DeleteShipList
            {
                if(target->objtype == OBJ_ShipType)
                {
                    ((Ship *)target)->howDidIDie = damageHow;
                    ((Ship *)target)->whoKilledMe = playerIndex; //if was collision death, who cares
                    bountyShipWasKilled((Ship *)target);
                }
                if(target->flags & SOF_Slaveable)
                {    //target is a ship and is slaveable
                    if(!(((Ship *)target)->slaveinfo->flags & SF_DEAD))   //if we haven't added then already
                    {
                        addSlavesToDeleteList((Ship *)target,soundType);
                    }
                    if(!(target->flags & SOF_Dead))
                        AddTargetToDeleteList(target,soundType);
                }
                else
                {
                    AddTargetToDeleteList(target,soundType);
                }
                return TRUE;                    //used by effect
            }
            else
            {
                return FALSE;
            }
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : pointExplosionInSpace(vector *position,real32 blastRadius)
    Description : Make all impactable objects within blast radius get rocked
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pointExplosionInSpace(vector *position,real32 blastRadius,real32 maxDamage,sdword playerIndex)
{
    Node *objnode = universe.ImpactableList.head;
    SpaceObjRotImp *obj;
    vector distvector;
    real32 distsquared;
    real32 dist;
    //real32 blastradius = shipstatic->staticheader.staticCollInfo.collspheresize * shipstatic->blastRadiusShockWave;
    real32 blastradiussqr = blastRadius * blastRadius;

    real32 blaststrength;

    while (objnode != NULL)
    {
        obj = (SpaceObjRotImp *) listGetStructOfNode(objnode);

        if ((obj->flags & (SOF_Impactable|SOF_NotBumpable)) != (SOF_Impactable))
        {
            goto nextnode;      // object must be impactable, bumpable
        }

        vecSub(distvector,obj->collInfo.collPosition,*position);

        if (!isBetweenInclusive(distvector.x,-blastRadius,blastRadius))
        {
            goto nextnode;
        }

        if (!isBetweenInclusive(distvector.y,-blastRadius,blastRadius))
        {
            goto nextnode;
        }

        if (!isBetweenInclusive(distvector.z,-blastRadius,blastRadius))
        {
            goto nextnode;
        }

        distsquared = vecMagnitudeSquared(distvector);
        if (distsquared > blastradiussqr)
        {
            goto nextnode;
        }

        if (obj->staticinfo->staticheader.immobile)
        {
            goto nextnode;
        }

        // obj is within blast radius, so it should get rocked

        if (distsquared < 1.0f)
        {
            distsquared = 1.0f;
        }
        dist = fsqrt(distsquared);
        blaststrength = 1.0f - (dist / blastRadius);
        vecNormalizeToLength(&distvector,BLAST_CONSTANT * blaststrength);

        ApplyExplosionRockToTarget(obj,blaststrength);
        vecScalarMultiply(distvector,distvector,TW_BURST_ATTACK_FORCE_MODIFIER);
        physApplyForceVectorToObj(obj,distvector);

        if (obj->flags & SOF_Targetable)
        {
            // calculate damage based on blast as a % of maxhealth
            ApplyDamageToTarget((SpaceObjRotImpTarg *)obj,maxDamage,0,DEATH_Killed_By_Player_Explosion,playerIndex);
        }

nextnode:
        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univBulletCollidedWithTarget
    Description : Call when bullet collides with a target
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univBulletCollidedWithTarget(SpaceObjRotImpTarg *target,StaticHeader *targetstaticheader,Bullet *bullet,real32 collideLineDist,sdword collSide)
{
    real32 damagetaken = bullet->damage;
    Ship *bulletowner = bullet->owner;
    real32 massovertime;
    vector recoil;
    etgeffectstatic *stat;
    sdword LOD;
    vector LOF;
    bool fatalHit;
    vector hitLocation;
    matrix hitCoordsys;
    real32 floatDamage;
    real32 damageMult = 1.0f;
    udword intDamage;
    real32 targetScale;

    // if target isn't immobile, apply impact force from bullet
    if (!targetstaticheader->immobile && !bitTest(target->flags, SOF_NotBumpable))
    {
        if(target->objtype == OBJ_ShipType )
        {
            if(((Ship *)target)->specialFlags2 & SPECIAL_2_ShipInGravwell)
            {
                goto nobulletmasstransfer;
            }
        }
        massovertime = bullet->bulletmass / universe.phystimeelapsed;
        recoil = bullet->posinfo.velocity;
        vecMultiplyByScalar(recoil,massovertime);
        ApplyRockToTarget(target,bullet->bulletmass);
        physApplyForceVectorToObj(target,recoil);
    }

nobulletmasstransfer:
    if (target->objtype == OBJ_ShipType)
    {
        damageMult = ((Ship *)target)->magnitudeSquared;

        // apply different damage modifier based on collision side:
        if (collSide >= 0)
        {
            dbgAssert(collSide < NUM_TRANS_DEGOFFREEDOM);
            damagetaken *= ((Ship *)target)->staticinfo->collSideModifiers[collSide];
        }

        if (bulletowner != NULL)
        {
            if ((bulletowner->shiptype == HeavyDefender) && (((ShipStaticInfo *)target->staticinfo)->shipclass == CLASS_Fighter))
            {
                // armour piercing bullets vs fighters
                damagetaken *= armourPiercingModifier;
            }

            if (bulletowner->playerowner == ((Ship *)target)->playerowner)
            {                                               //if it's friendly fire
        //!!! why only in non-debug???
#ifndef HW_Debug
                damagetaken *= friendlyFireModifier;
#endif
                bitSet(((Ship *)target)->chatterBits, BCB_FriendlyFire);
            }
            else if (allianceIsShipAlly((Ship *)target, bulletowner->playerowner))
            {
                bitSet(((Ship *)target)->chatterBits, BCB_AlliedFire);
            }
            else
            {
                speechEventUnderAttack((Ship *)target);
            }
            ((Ship *)target)->gettingrocked = bulletowner;

            ((Ship *)target)->recentAttacker = bulletowner;
            ((Ship *)target)->recentlyAttacked = RECENT_ATTACK_DURATION;  // start counting down
        }
        else
        {
            speechEventUnderAttack((Ship *)target);
        }
    }

    if(bulletowner != NULL && bulletowner->playerowner != NULL)
        fatalHit = ApplyDamageToTarget(target,damagetaken,bullet->soundType,DEATH_Killed_By_Player,bulletowner->playerowner->playerIndex);
    else
        fatalHit = ApplyDamageToTarget(target,damagetaken,bullet->soundType,DEATH_Killed_By_Dead_Player,99);

    //delete the bullet effect
#if ETG_DISABLEABLE
    if (bullet->effect != NULL && etgEffectsEnabled && bullet->bulletType != BULLET_Beam)
#else
    if (bullet->effect != NULL && bullet->bulletType != BULLET_Beam)
#endif
    {
        etgEffectDelete(bullet->effect);
        univRemoveObjFromRenderList((SpaceObj *)bullet->effect);
        listDeleteNode(&bullet->effect->objlink);
        bullet->effect = NULL;
    }

    if (bullet->bulletType != BULLET_Beam)
    {
        if (collideLineDist == 0.0f)
        {                                                   //if it's a beam weapon and there is no collision vector
            return;                                         //don't play any hit effects
        }
    }
    //play the imact effect
    if (bullet->hitEffect != NULL)
    {
        LOD = target->currentLOD;
        if (LOD < bullet->hitEffect->nLevels)
        {
            stat = bullet->hitEffect->level[LOD];
        }
        else
        {
            stat = NULL;
        }
    }
    else
    {
        stat = NULL;
    }
    if (univSpaceObjInRenderList((SpaceObj *)target))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgHitEffectsEnabled && etgEffectsEnabled && (!etgFrequencyExceeded(stat)))
#else
        if (stat != NULL && etgHitEffectsEnabled && (!etgFrequencyExceeded(stat)))
#endif
        {
            if (collideLineDist == 0.0f)
            {
                hitLocation = bullet->posinfo.position;
            }
            else
            {
                dbgAssert(collideLineDist > 0.0f);
                vecScalarMultiply(hitLocation,bullet->bulletheading,collideLineDist);
                vecAdd(hitLocation,bullet->posinfo.position,hitLocation);
            }
            //N-Lip the impact position
            if (target->objtype == OBJ_ShipType)
            {
                targetScale = ((Ship *)target)->magnitudeSquared;
                vecSubFrom(hitLocation, target->posinfo.position);
                vecMultiplyByScalar(hitLocation, targetScale);
                vecAddTo(hitLocation, target->posinfo.position);
            }
            else
            {
                targetScale = 1.0f;
            }

            vecCopyAndNegate(LOF,bullet->bulletheading);
            matCreateCoordSysFromHeading(&hitCoordsys,&LOF);
            floatDamage = (real32)damagetaken;
            if (RGLtype == SWtype)
            {                                               //smaller hit effects in software
                floatDamage *= etgSoftwareScalarHit;
            }
            floatDamage *= damageMult;
            intDamage = TreatAsUdword(floatDamage);
            etgEffectCreate(stat, NULL, &hitLocation, &target->posinfo.velocity, &hitCoordsys, targetScale, 0, 2, intDamage, fatalHit);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univMissileCollidedWithTarget
    Description : Call when a missile collides with a target
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univMissileCollidedWithTarget(SpaceObjRotImpTarg *target,StaticHeader *targetstaticheader,Missile *missile,real32 collideLineDist,sdword collSide)
{
    real32 damagetaken = missile->damage;
    Ship *missileowner = missile->owner;
    MissileStaticInfo *missilestaticinfo = (MissileStaticInfo *)missile->staticinfo;
    real32 massovertime;
    vector recoil;
    etgeffectstatic *stat;
    sdword LOD;
    bool fatalHit;
    real32 floatDamage;
    udword intDamage;
    vector missileheading;
    vector hitLocation;
    real32 targetScale;
    real32 damageMult = 1.0f;

    if(target->objtype == OBJ_ShipType)
    {
        if(((Ship *)target)->playerowner->playerIndex == universe.curPlayerIndex)
        {
            if(missile->FORCE_DROPPED)
            {
                ////////////////////
                //target hit by force drop mines..local players ship
                //speech event num: STAT_Grp_InMineField
                //or STAT_AssGrp_InMineField
                //use battle chatter
                if(selHotKeyGroupNumberTest((Ship *)target) == SEL_InvalidHotKey)
                 {                                          //not in a group!
                     if (battleCanChatterAtThisTime(BCE_STAT_Grp_InMineField, ((Ship *)target)))
                     {
                         battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Grp_InMineField, ((Ship *)target), SOUND_EVENT_DEFAULT);
                     }
                 }
                 else
                 {
                     if (battleCanChatterAtThisTime(BCE_STAT_AssGrp_InMineField, ((Ship *)target)))
                     {
                         battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_AssGrp_InMineField, ((Ship *)target), selHotKeyGroupNumberTest((Ship *)target));
                     }
                 }
                //////////////
            }
        }
    }
    // if target isn't immobile, apply impact force from missile
    if (!targetstaticheader->immobile && !bitTest(target->flags, SOF_NotBumpable))
    {
        massovertime = missilestaticinfo->staticheader.mass / universe.phystimeelapsed;
        recoil = missile->posinfo.velocity;
        vecMultiplyByScalar(recoil,massovertime);
        ApplyRockToTarget(target,missilestaticinfo->staticheader.mass);
        physApplyForceVectorToObj(target,recoil);
    }

    if (target->objtype == OBJ_ShipType)
    {
        damageMult = ((Ship *)target)->magnitudeSquared;
        // apply different damage modifier based on collision side:
        if (collSide >= 0)
        {
            dbgAssert(collSide < NUM_TRANS_DEGOFFREEDOM);
            damagetaken *= ((Ship *)target)->staticinfo->collSideModifiers[collSide];
        }

        if (missileowner != NULL)
        {
            if (missileowner->playerowner == ((Ship *)target)->playerowner)
            {                                               //if it's friendly fire
        //!!! why only in non-debug???
#ifndef HW_Debug
                damagetaken *= friendlyFireModifier;
#endif
                bitSet(((Ship *)target)->chatterBits, BCB_FriendlyFire);
            }
            else if (allianceIsShipAlly((Ship *)target, missileowner->playerowner))
            {
                bitSet(((Ship *)target)->chatterBits, BCB_AlliedFire);
            }
            else
            {
                speechEventUnderAttack((Ship *)target);
            }
            ((Ship *)target)->gettingrocked = missileowner;

            ((Ship *)target)->recentAttacker = missileowner;
            ((Ship *)target)->recentlyAttacked = RECENT_ATTACK_DURATION;  // start counting down
        }
        else
        {
            speechEventUnderAttack((Ship *)target);
        }
    }

    if(missileowner != NULL && missileowner->playerowner != NULL)
        fatalHit = ApplyDamageToTarget(target,damagetaken,missile->soundType,DEATH_Killed_By_Player,missileowner->playerowner->playerIndex);
    else
        fatalHit = ApplyDamageToTarget(target,damagetaken,missile->soundType,DEATH_Killed_By_Dead_Player,99);

    if (missile->hitEffect != NULL)
    {
        LOD = target->currentLOD;
        if (LOD < missile->hitEffect->nLevels)
        {
            stat = missile->hitEffect->level[LOD];
        }
        else
        {
            stat = NULL;
        }
    }
    else
    {
        stat = NULL;
    }
    if (univSpaceObjInRenderList((SpaceObj *)target))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgHitEffectsEnabled && etgEffectsEnabled && (!etgFrequencyExceeded(stat)))
#else
        if (stat != NULL && etgHitEffectsEnabled && (!etgFrequencyExceeded(stat)))
#endif
        {
            if (collideLineDist == 0.0f)
            {
                hitLocation = missile->posinfo.position;
            }
            else
            {
                dbgAssert(collideLineDist > 0.0f);
                matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
                vecScalarMultiply(hitLocation,missileheading,collideLineDist);
                vecAdd(hitLocation,missile->posinfo.position,hitLocation);
            }
            //N-Lip the impact position
            if (target->objtype == OBJ_ShipType)
            {
                targetScale = ((Ship *)target)->magnitudeSquared;
                vecSubFrom(hitLocation, target->posinfo.position);
                vecMultiplyByScalar(hitLocation, targetScale);
                vecAddTo(hitLocation, target->posinfo.position);
            }
            else
            {
                targetScale = 1.0f;
            }

            floatDamage = (real32)damagetaken;
            if (RGLtype == SWtype)
            {                                               //smaller hit effects in software
                floatDamage *= etgSoftwareScalarHit;
            }
            floatDamage *= damageMult;
            intDamage = TreatAsUdword(floatDamage);
            etgEffectCreate(stat, NULL, &hitLocation, &target->posinfo.velocity, &missile->rotinfo.coordsys, targetScale, 0, 2, intDamage, fatalHit);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveAllTargetReferencesFromBullets
    Description : removes all ship references from any bullets
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveAllTargetReferencesFromBullets(SpaceObjRotImpTarg *target)
{
    Node *objnodebullet = universe.BulletList.head;
    Bullet *bullet;

    // Update bullet->owner for all bullets whose owner is now dead
    while (objnodebullet != NULL)
    {
        bullet = (Bullet *)listGetStructOfNode(objnodebullet);
        dbgAssert(bullet->objtype == OBJ_BulletType);

        if (bullet->owner == (Ship *)target)
        {
            bullet->owner = NULL;       // bullet has no owner now that parent ship died
            bullet->gunowner = NULL;
            if (bullet->bulletType == BULLET_Beam)
            {
                bullet->timelived += 10000.0f;  // delete bullet if it is a beam weapon and target wielding it died
            }
        }

        if (bullet->target == target)
        {
            bullet->target = NULL;
        }

        objnodebullet = objnodebullet->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveAllTargetReferencesFromMissiles
    Description : removes all target references from any missiles
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveAllTargetReferencesFromMissiles(SpaceObjRotImpTarg *target)
{
    Node *objnodemissile = universe.MissileList.head;
    Missile *missile;

    // Update missile->owner for all missiles whose owner is now dead
    while (objnodemissile != NULL)
    {
        missile = (Missile *)listGetStructOfNode(objnodemissile);
        dbgAssert(missile->objtype == OBJ_MissileType);

        if (missile->owner == (Ship *)target)
        {
            missile->owner = NULL;       // missile has no owner now that parent ship died
        }

        if (missile->target == target)
        {
            missile->target = NULL;
        }

        objnodemissile = objnodemissile->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveAllDerelictReferencesFromSpecialShips
    Description : removes all derelict references from special ships
    Inputs      : d - derelictto remove
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveAllDerelictReferencesFromSpecialShips(Derelict *d)
{
    Node *node = universe.ShipList.head;
    Ship *ship;
    InsideShip *insideShip;

    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if(ship->specialFlags & SPECIAL_IsASalvager)
            salCapRemoveDerelictReferences(ship,d);

        node = node->next;
    }

    // walk through ships in hyperspace too!

    node = singlePlayerGameInfo.ShipsInHyperspace.head;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if(ship->specialFlags & SPECIAL_IsASalvager)
            salCapRemoveDerelictReferences(ship,d);

        node = node->next;
    }
}

void univRemoveAllShipReferencesFromSpecialShip(Ship *shiptoremove,Ship *ship)
{
    if (ship->recentlyAttacked && ship->recentAttacker == shiptoremove)
    {
        // forget that this ship had recently attacked anyone
        ship->recentlyAttacked = 0;
        ship->recentAttacker = NULL;
    }
    if (ship->recentlyFiredUpon && ship->firingAtUs == shiptoremove)
    {
        ship->recentlyFiredUpon = 0;
        ship->firingAtUs = NULL;
    }

    if (ship->rowGetOutOfWay == shiptoremove)
    {
        rowGetOutOfWayShipDiedCB(ship);
        ship->rowGetOutOfWay = NULL;
    }

    if(!(gameIsEnding && ship->shiptype == ResearchShip))
    {
        if(((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipRemoveShipReferences != NULL)
        {
            ((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipRemoveShipReferences(ship, shiptoremove);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveAllShipReferencesFromSpecialShips
    Description : removes all ship references from special ships
    Inputs      : shiptoremove
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveAllShipReferencesFromSpecialShips(Ship *shiptoremove)
{
    Node *node = universe.ShipList.head;
    Ship *ship;
    InsideShip *insideShip;

    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);
        dbgAssert(ship->objtype == OBJ_ShipType);

        univRemoveAllShipReferencesFromSpecialShip(shiptoremove,ship);

        node = node->next;
    }

    // walk through ships in hyperspace too!

    node = singlePlayerGameInfo.ShipsInHyperspace.head;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        univRemoveAllShipReferencesFromSpecialShip(shiptoremove,ship);

        node = node->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : DeleteShipsInsideMe
    Description : deletes ships that are inside me
    Inputs      : ship which contains other ships
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DeleteShipsInsideMe(Ship *ship)
{
    InsideShip *insideShip;
    Node *node = ship->shipsInsideMe->insideList.head;

    while (node != NULL)
    {
        insideShip = listGetStructOfNode(node);

        dbgAssert(insideShip->ship);
        insideShip->ship->howDidIDie = ship->howDidIDie;
        insideShip->ship->whoKilledMe = ship->whoKilledMe;
        univDeleteWipeInsideShipOutOfExistence(insideShip->ship);

        node = node->next;
    }

    listDeleteAll(&ship->shipsInsideMe->insideList);
    memFree(ship->shipsInsideMe);
    ship->shipsInsideMe = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : FreeShipsInsideMe
    Description : frees ships contents that are inside me
    Inputs      : ship which contains other ships
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FreeShipsInsideMe(Ship *ship)
{
    InsideShip *insideShip;
    Node *node = ship->shipsInsideMe->insideList.head;

    while (node != NULL)
    {
        insideShip = listGetStructOfNode(node);

        dbgAssert(insideShip->ship);
        insideShip->ship->howDidIDie = ship->howDidIDie;
        insideShip->ship->whoKilledMe = ship->whoKilledMe;
        univFreeShipContents(insideShip->ship);
        memFree(insideShip->ship);

        node = node->next;
    }

    listDeleteAll(&ship->shipsInsideMe->insideList);
    memFree(ship->shipsInsideMe);
    ship->shipsInsideMe = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : univWipeShipOutOfExistence
    Description : totally destroys and wipes this ship out of memory, and
                  removes any remaining references.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univWipeShipOutOfExistence(Ship *ship)
{
    dbgAssert(ship != NULL);
    if (ship->shiplink.belongto != NULL)
    {
        listRemoveNode(&ship->shiplink);
    }
    if (ship->impactablelink.belongto != NULL)
    {
        listRemoveNode(&ship->impactablelink);
    }
    nisObjectDied((SpaceObj *)ship);
    etgShipDied(ship);
    univRemoveObjFromRenderList((SpaceObj *)ship);
    univFreeShipContents(ship);
    listDeleteNode(&ship->objlink);
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteWipeInsideShipOutOfExistence
    Description : totally destroys and wipes a "ship that was inside another ship"
                  out of memory.
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteWipeInsideShipOutOfExistence(Ship *ship)
{
    univFreeShipContents(ship);
    memFree(ship);                  //added Ala the request of Gary
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveShipFromOutside
    Description : removes a ship from the universe and from mission spheres so that it
                  can be placed inside a ship
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveShipFromOutside(Ship *ship)
{
    // if a ship goes inside another ship, missile can no longer target it
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)ship);

    shipHasJustDisappeared(ship);

    // Update Camera focus stacks
    ccRemoveShip(&universe.mainCameraCommand,ship);

    // Let any mission spheres know ship is leaving it
//    msShipLeavingMissphere(ship,ship->myMissphere,ship->playerowner->homeMissphere);

    // Remove ships that are destroyed from any current selection
    clRemoveShipFromSelection((SelectAnyCommand *)&selSelected,ship);
    clRemoveShipFromSelection((SelectAnyCommand *)&selSelecting,ship);
    ioUpdateShipTotals();

    // Remove ship from hotkey group if it belongs to one.
    //univRemoveShipFromHotkeyGroup(ship, FALSE);       maintain hotkey groups

    //if sensors manager is running, delete it from any sm spheres
    if (smSensorsActive)
    {
        smObjectDied(ship);
    }

    bobObjectDied((SpaceObj *)ship,&universe.collBlobList);
    pingObjectDied((SpaceObj *)ship);
    ship->collMyBlob = NULL;

    pieShipDied(ship);
    mouseClickShipDied(ship);

    if (ship->shiplink.belongto != NULL)
    {
        listRemoveNode(&ship->shiplink);
    }
    if (ship->impactablelink.belongto != NULL)
    {
        listRemoveNode(&ship->impactablelink);
    }
    if (ship->objlink.belongto != NULL)
    {
        listRemoveNode(&ship->objlink);
    }
    univRemoveObjFromRenderList((SpaceObj *)ship);

    // Tell the sound layer that the ship is removed from the universe
    soundEventShipRemove(ship);

    ship->flags |= SOF_Hide;
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveShipFromHotkeyGroup
    Description : Removes ship from hotkey group
    Inputs      : ship
    Outputs     : if ship was removed or not.
    Return      : bool8
----------------------------------------------------------------------------*/
bool8 univRemoveShipFromHotkeyGroup(Ship *ship, bool8 PlayDefeat)
{
    udword i;

    if (selAnyHotKeyTest(ship))
    {                                                       //if ship in any hot key groups
        // Remove ships that are destroyed from any hot key groups
        for (i=0;i<SEL_NumberHotKeyGroups;i++)
        {
            if (selHotKeyTest(ship, i))
            {                                               //if it's in this hot-key group
                if (clRemoveShipFromSelection((SelectCommand *)&selHotKeyGroup[i],ship))
                {
                    if (selHotKeyGroup[i].numShips == 0)
                    {
                        selHotKeyGroup[i].timeLastStatus = universe.totaltimeelapsed;
                        if (PlayDefeat)
                        {
                            speechEventFleet(STAT_F_AssGrp_Defeat, i, ship->playerowner->playerIndex);
                        }
                    }
                }
            }
        }

        ship->hotKeyGroup = 0;

        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : univHideShipFromSpheres
    Description : Removes a ship from all mission spheres but not from the universe
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univHideShipFromSpheres(Ship *ship)
{
    univRemoveObjFromRenderList((SpaceObj *)ship);
    ship->flags |= SOF_Hide;
}

/*-----------------------------------------------------------------------------
    Name        : univHideJustAboutEverything
    Description : Sets the SOF_Hide bit on evertything that is not a resource
                    and does not have the SOF_NISShip bit set.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univHideJustAboutEverything(void)
{
    Node *node;
    SpaceObj *obj;

    for (node = universe.SpaceObjList.head; node != NULL; node = node->next)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);
        if (!bitTest(obj->flags, SOF_NISShip))
        {
            switch (obj->objtype)
            {
                case OBJ_ShipType:
                case OBJ_BulletType:
                case OBJ_DerelictType:
                case OBJ_EffectType:
                case OBJ_MissileType:
                    if (bitTest(obj->flags, SOF_Hide))
                    {
                        bitSet(obj->flags, SOF_HideSave2);
                    }
                    else
                    {
                        univRemoveObjFromRenderList(obj);
                        bitSet(obj->flags, SOF_Hide);
                    }
                    break;
                case OBJ_AsteroidType:
                case OBJ_NebulaType:
                case OBJ_GasType:
                case OBJ_DustType:
                    break;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univUnhideJustAboutEverything
    Description : Unhide everything that was hidden in a previous call
                    to univHideJustAboutEverything.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univUnhideJustAboutEverything(void)
{
    Node *node;
    SpaceObj *obj;

    for (node = universe.SpaceObjList.head; node != NULL; node = node->next)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);
        if (bitTest(obj->flags, SOF_HideSave2))
        {                                                   //if it was already hidden last univHideJustAboutEverything
            bitClear(obj->flags, SOF_HideSave2);
        }
        else
        {
            bitClear(obj->flags, SOF_Hide);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadShip
    Description : Removes all references to the ship, except for a few ones
                  to let the ship hang around after the explosion.
    Inputs      : ship
                  deathby - mode of death, or -1 for no death effects
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadShip(Ship *ship, sdword deathBy)
{
    Effect *effect;
    etgeffectstatic *explosion;
    etglod *etgLOD;
    sdword LOD;
    real32 colSize;
    udword colSizeDword, velMagnitudeDword;
    real32 velMagnitude, maxVelocity;

    vecZeroVector(ship->posinfo.force);
    vecZeroVector(ship->rotinfo.torque);

    ship->flags |= SOF_Dead;
    bitClear(ship->flags,SOF_Selectable);

    if (((ShipStaticInfo *)(ship->staticinfo))->canBuildShips)
    {
        cmRemoveFactory(ship);
    }

    if (ship->rceffect != NULL)
    {
        TurnHarvestEffectOff(ship);
    }

    // Check if launch manager is up, if it is then fix it update it on a ships death.

    if (lmActive)
    {
        if ((ship->shiptype==Carrier) || (ship->shiptype==Mothership))
        {
            lmLaunchCapableShipDied(ship);
        }
    }

    if(((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipDied != NULL)
    {
        ((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipDied(ship);
    }

    tacticsShipDied(ship);

    if (ship->shipsInsideMe)
    {
        DeleteShipsInsideMe(ship);
    }

    if (bitTest(ship->flags,SOF_Slaveable))
    {   //ship was involved with the slave trade...so cleanup
        dockDealWithDeadSlaveable(ship);
    }
    // Remove repair droids...
/*    if(ship->pRepairDroids)
    {
        memFree(ship->pRepairDroids);
        ship->pRepairDroids = 0;
    }*/

#if ETG_DISABLEABLE
    if (deathBy >= 0 && etgEffectsEnabled)
#else
    if (deathBy >= 0)
#endif
    {
        if (deathBy == EDT_AccumDamage)
        {
            battleShipDyingWithTimeToScream(ship);
        }
//        if (univSpaceObjInRenderList((SpaceObj *)ship))
//        {
            etgLOD = etgDeathEventTable[((ShipStaticInfo *)ship->staticinfo)->shiprace]
                [((ShipStaticInfo *)ship->staticinfo)->shipclass][deathBy];
            if (etgLOD != NULL)
            {
                LOD = ship->currentLOD;
                explosion = etgLOD->level[min(etgLOD->nLevels - 1, LOD)];
            }
            else
            {
                explosion = NULL;
            }
            if (explosion)
            {
                colSize = ((ShipStaticInfo *)(ship->staticinfo))->staticheader.staticCollInfo.collspheresize;
//                if (univSpaceObjInRenderList((SpaceObj *)ship))
//                {
                    colSize *= ship->magnitudeSquared;
//                }
                colSizeDword = TreatAsUdword(colSize);

                maxVelocity = ship->staticinfo->staticheader.maxvelocity;
                if (maxVelocity != 0.0f)
                {
                    velMagnitude = fsqrt(vecMagnitudeSquared(ship->posinfo.velocity));
                    velMagnitude /= maxVelocity;
                }
                else
                {
                    velMagnitude = 0;
                }
                velMagnitudeDword = TreatAsUdword(velMagnitude);

                //!!!??? should there be some EAF_ here???
                effect = etgEffectCreate(explosion, ship, NULL, NULL, NULL, 1.0f, 0, 2, colSizeDword, velMagnitudeDword);
                /*
                if (!univSpaceObjInRenderList((SpaceObj *)effect))
                {
                    univAddObjToRenderList((SpaceObj *)effect);//make sure it's in render list
                }
                */
                bitSet(effect->flags, SOF_ForceVisible);
            }
//        }
    }

    //notify leader of a lost wingman
    if (ship->tacticstype == Evasive)
    {                                               //if in evasive tactics
        if (ship->attackvars.myLeaderIs != NULL)
        {                                           //if ship has a wingman
            bitSet(ship->attackvars.myLeaderIs->chatterBits, BCB_WingmanLost);
        }
    }

    univRemoveAllTargetReferencesFromBullets((SpaceObjRotImpTarg *)ship);
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)ship);

    univRemoveAllShipReferencesFromSpecialShips(ship);

    //if sensors manager is running, delete it from any sm spheres
    if (smSensorsActive)
    {
        smObjectDied(ship);
    }

    bobObjectDied((SpaceObj *)ship,&universe.collBlobList);
    pingObjectDied((SpaceObj *)ship);
    ship->collMyBlob = NULL;

    // Update Ship Command Layer to reflect ship death
    clShipDied(&universe.mainCommandLayer,ship);

    // Update computer player structures to reflect ship death
    aiplayerShipDied(ship);

    // Update Camera focus stacks
    ccRemoveShip(&universe.mainCameraCommand,ship);

    // Let any mission spheres know ship is leaving it
//    msShipLeavingMissphere(ship,ship->myMissphere,ship->playerowner->homeMissphere);

    // Tell single player game module ship died
    singlePlayerShipDied(ship);

    if(tutorial)
        tutPlayerShipDied(ship);

    // Tell the sound layer that the ship died
    soundEventShipDied(ship);

    IDToPtrTableObjDied(&ShipIDToPtr,ship->shipID.shipNumber);

    growSelectRemoveShipBySettingNULL(&universe.HousekeepShipList,ship);

    // Remove ships that are destroyed from any current selection
    clRemoveShipFromSelection((SelectCommand *)&selSelected,ship);
    clRemoveShipFromSelection((SelectCommand *)&selSelecting,ship);
    ioUpdateShipTotals();

    pieShipDied(ship);
    mouseClickShipDied(ship);

    // Remove ships that are destroyed from any hot key groups and play group death sound if all dead
    univRemoveShipFromHotkeyGroup(ship, TRUE);

//if no effects, we should delete it properly
    /*
#if ETG_DISABLEABLE
    if (!etgEffectsEnabled)
    {
        univWipeShipOutOfExistence(ship);
    }
#endif
*/
}

void DisableAllPlayersShips(sdword playerIndex)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    Player *player = &universe.players[playerIndex];

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        if (ship->playerowner == player)
        {
            if(!(ship->flags & SOF_Disabled))
            {
                ship->flags |= SOF_Crazy;
                ship->deathtime = universe.totaltimeelapsed + frandombetween(PLAYERLOSE_SHIPDEATHTIMEMIN,PLAYERLOSE_SHIPDEATHTIMEMAX);
                if ((!singlePlayerGame) && (ship->shiptype != Mothership)) ApplyCareenRotationDirectly(ship);
            }

            bitClear(ship->flags,SOF_Selectable);

            // Remove ships that are destroyed from any current selection
            clRemoveShipFromSelection((SelectCommand *)&selSelected,ship);

            // Remove ships that are destroyed from any hot key groups
            univRemoveShipFromHotkeyGroup(ship,FALSE);
        }

        objnode = objnode->next;
    }
    ioUpdateShipTotals();
    if (piePointSpecMode != PSM_Idle)
    {                                                       //make sure the movement mechanism is off
        piePointModeOnOff();
    }
}

void DisplayPlayerHasBeenDefeatedMessage(sdword playerIndex)
{
    char defeatmsg[100];

    strcpy(defeatmsg,playerNames[playerIndex]);
    strcat(defeatmsg," ");
    strcat(defeatmsg,strGetString(strPlayerHasDied));
    clCommandMessage(defeatmsg);
}

void CheckPlayerWin(void)
{
    udword numPlayersAlive = 0;
    udword i;

    if (gatherStats|showStatsFight|showStatsFancyFight)
        return;     // just gathering stats, don't want to do the player win thing

    if (!singlePlayerGame)
    {
        // check how many players are still alive
        for (i=0;i<universe.numPlayers;i++)
        {
            if (universe.players[i].playerState == PLAYER_ALIVE)
            {
                numPlayersAlive++;
            }
        }

        if (numPlayersAlive <= 1)
        {
            // end of game
            universe.wintime = universe.totaltimeelapsed + WAIT_TIME_TO_WIN;
            universe.quittime = universe.totaltimeelapsed + WAIT_TIME_TO_QUIT;
        }

        //check for allied victory
        if((!singlePlayerGame) && (tpGameCreated.flag & MG_AlliedVictory))
        {
            sdword ally = -1;
            bool alliedvictory = TRUE;
            //all living players on same team?
            //find allies of first player, make sure all other living players
            // are allies

            // This statement does nothing but presumably was meant to do something. I'm leaving it
            // here in case someone debugging this code realises it forms part of the solution...
            //universe.players[sigsPlayerIndex].Allies;
            
            for (i=0; ((i<universe.numPlayers) && alliedvictory); i++)
            {
                if (universe.players[i].playerState == PLAYER_ALIVE)
                {
                    if (ally == -1)
                    {
                        //first player
                        ally = i;

                    }
                    else
                    {
                        if ( !((1<<i) & universe.players[ally].Allies) )
                        {

                            alliedvictory = FALSE;
                        }
                    }
                }
            }
            if (alliedvictory)
            {
                // end of game
                universe.wintime = universe.totaltimeelapsed + WAIT_TIME_TO_WIN;
                universe.quittime = universe.totaltimeelapsed + WAIT_TIME_TO_QUIT;
            }
        }
    }
    else
    {
        if (universe.players[0].playerState == PLAYER_DEAD)
        {
            // end of game
            universe.wintime = universe.totaltimeelapsed + WAIT_TIME_TO_WIN;
            universe.quittime = universe.totaltimeelapsed + WAIT_TIME_TO_QUIT;
        }
    }
}

void DeclarePlayerWin(void)
{
    udword numPlayersAlive = 0;
    udword i, count=0;
    char winmsg[200];

    if (singlePlayerGame)
    {
        return;
    }

    // check how many players are still alive
    for (i=0;i<universe.numPlayers;i++)
    {
        if (universe.players[i].playerState == PLAYER_ALIVE)
        {
            numPlayersAlive++;
        }
    }

    if (numPlayersAlive == 0)
    {
        bigmessageDisplay(strGetString(strAllPlayersDead),1);
    }
    else
    {
        sdword otherplayersdied = 0;
        sdword otherplayersdroppedout = 0;

        strcpy(winmsg,"");
        for (i=0;i<universe.numPlayers;i++)
        {
            if (universe.players[i].playerState == PLAYER_ALIVE)
            {
                strcat(winmsg,playerNames[i]);
                strcat(winmsg," ");
                count++;
                if ((numPlayersAlive-count)>1)
                {
                    strcat(winmsg, ", ");
                } else if ((numPlayersAlive-count)==1)
                {
                    strcat(winmsg, strGetString(strAnd));
                    strcat(winmsg, " ");
                }
            }
            else
            {
                if (i != universe.curPlayerIndex)
                {
                    otherplayersdied++;
                    if (universe.gameStats.playerStats[i].typeOfPlayerDeath == PLAYERKILLED_DROPPEDOUT)
                    {
                        otherplayersdroppedout++;
                    }
                }
            }
        }

        if ((otherplayersdroppedout > 0) && (otherplayersdroppedout == otherplayersdied))
        {
            bigmessageDisplay(strGetString(strAllOtherPlayersDroppedOut),1);
        }
        else
        {
            strcat(winmsg," ");
            if (numPlayersAlive > 1)
                strcat(winmsg,strGetString(strPlayersHaveWon));
            else
                strcat(winmsg,strGetString(strPlayerHasWon));
            bigmessageDisplay(winmsg,1);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : univFindBackupMothership
    Description : Looks through the universeshiplist to find out if the
                  player who's mothership died has a backup mothership
    Inputs      : player - the player who's mothership just died
    Outputs     :
    Return      : TRUE if a backup was found
----------------------------------------------------------------------------*/
bool univFindBackupMothership(struct Player *player)
{
    Node *shipnode = universe.ShipList.head;
    Ship *ship;

    while (shipnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(shipnode);

        if (((ship->flags & SOF_Dead) == 0) && (ship->playerowner == player) && (ship != player->PlayerMothership))
        {
            if ((ship->shiptype == Carrier) || (ship->staticinfo->shipclass == CLASS_Mothership))
            {
                if (player->PlayerMothership != NULL)
                {
                    if (gameIsRunning && (player->PlayerMothership->shiptype == Mothership))
                    {
                        speechEventFleet(STAT_F_ReassignedToCarrier, 0, player->playerIndex);
                    }
                }
                player->PlayerMothership = ship;
                return TRUE;
            }
        }

        shipnode = shipnode->next;
    }
    return FALSE;
}

void univKillOtherPlayersIfDead(Ship *ship)
{
    bool aplayerdied=FALSE;
    sdword i;
    //for (i=0;i<universe.numPlayers;i++)
    //{
    i = ship->playerowner->playerIndex;

        if (ship == universe.players[i].PlayerMothership)
        {
            if (singlePlayerGame)
            {
                if (i != 0)
                {
                    // player 1 of single player game just lost mothership - but don't kill player
                    universe.players[i].PlayerMothership = NULL;
                }
                else
                {
                    aplayerdied = TRUE;
                    univKillPlayer(i,PLAYERKILLED_STANDARDDEATH);
                }
            }
            else if (!univFindBackupMothership(&universe.players[i]))
            {
                if(tpGameCreated.flag & MG_LastMotherShip || universe.players[i].totalships == 0)
                {
                    aplayerdied = TRUE;
                    univKillPlayer(i,PLAYERKILLED_STANDARDDEATH);
                }
                else
                {
                    universe.players[i].PlayerMothership = NULL;
                }
            }
        }
        else if(universe.players[i].totalships == 0)
        {
            if(!singlePlayerGame)
            {
                //all players ships are dead...he's dead
                aplayerdied = TRUE;
                univKillPlayer(i,PLAYERKILLED_STANDARDDEATH);
            }
        }

    if (aplayerdied)
    {
        CheckPlayerWin();
    }
    //}
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadShips
    Description : deletes all dead ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadShips()
{
    Node *objnode = universe.DeleteShipList.head;
    DeleteShip *deleteShip;
    Ship *ship;
    Node *nextnode;

    if (objnode == NULL)
    {
        return;
    }

    do
    {
        nextnode = objnode->next;

        deleteShip = (DeleteShip *)listGetStructOfNode(objnode);
        ship = deleteShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        univDeleteDeadShip(ship,deleteShip->deathBy);
        if (ship->impactablelink.belongto != NULL)
        {
            listRemoveNode(&ship->impactablelink);
        }

        if (ship->shiptype == ResourceCollector)
        {
            if (ship->resources > 0)
            {
                DustCloud *dustCloud = univAddDustCloud(DustCloud0,&ship->posinfo.position);
                dustCloud->resourcevalue = (sdword) (ship->resources * DUSTCLOUD_RCOLLECTOR_BLOWUP_SCALE);
                HandleDustCloudScaling(dustCloud);
                univAddObjToRenderListIf((SpaceObj *)dustCloud,(SpaceObj *)ship);     // add to render list if parent is in render list
            }
        }

        univKillOtherPlayersIfDead(ship);

        // don't totally delete ship, until after explosion
#if ETG_DISABLEABLE
        if (!etgEffectsEnabled)
        {
            univWipeShipOutOfExistence(ship);
        }
#endif

        objnode = nextnode;

    } while (objnode != NULL);

    listDeleteAll(&universe.DeleteShipList);
}

/*-----------------------------------------------------------------------------
    Name        : univReallyDeleteThisShipRightNow
    Description : Like you need to ask?  It kills it.  No effects, no melodrama,
                    no waiting around to delete it later.  Just BOOM!
    Inputs      : ship - ship to delete
    Outputs     :
    Return      : void
    Note        : do not call this while you are walking any ship-related
                    linked lists.
----------------------------------------------------------------------------*/
void univReallyDeleteThisShipRightNow(Ship *ship)
{
    univDeleteDeadShip(ship, -1);
    if (ship->impactablelink.belongto != NULL)
    {
        listRemoveNode(&ship->impactablelink);
    }
    univWipeShipOutOfExistence(ship);
}
/*-----------------------------------------------------------------------------
    Name        : DeleteDeadResource
    Description : Deletes an individual resource, and removes all references
                  to it.  Also frees its contents.
    Inputs      : resource
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DeleteDeadResource(Resource *resource)
{
    //if sensors manager is running, delete it from any sm spheres
    if (smSensorsActive)
    {
        smObjectDied((SpaceObj *)resource);
    }

    ccRemoveShip(&universe.mainCameraCommand,(Ship *)resource);

    bobObjectDied((SpaceObj *)resource,&universe.collBlobList);
    pingObjectDied((SpaceObj *)resource);

    // Update Command Layer to reflect resource death
    univRemoveAllTargetReferencesFromBullets((SpaceObjRotImpTarg *)resource);
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)resource);
    clResourceDied(&universe.mainCommandLayer,resource);
    aiplayerResourceDied(resource);

    IDToPtrTableObjDied(&ResourceIDToPtr,resource->resourceID.resourceNumber);

    univFreeResourceContents(resource);
}

/*-----------------------------------------------------------------------------
    Name        : univReallyDeleteThisResourceRightNow
    Description : Like you need to ask?  It kills it.  No effects, no melodrama,
                  no waiting around to delete it later.  Just BOOM!
    Inputs      : resource - resource to delete
    Outputs     :
    Return      : do not call this while you are walking any resource-related
                  linked lists.
----------------------------------------------------------------------------*/
void univReallyDeleteThisResourceRightNow(Resource *resource)
{
    DeleteDeadResource(resource);

    if ((resource->objtype == OBJ_AsteroidType) && (((Asteroid *)resource)->asteroidtype == Asteroid0))
    {
        univRemoveObjFromMinorRenderList((SpaceObj *)resource);
        listDeleteNode(&resource->objlink);
    }
    else
    {
        listRemoveNode(&resource->resourcelink);
        listRemoveNode(&resource->impactablelink);
        univRemoveObjFromRenderList((SpaceObj *)resource);
        listDeleteNode(&resource->objlink);
    }
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadResources
    Description : deletes all dead resources
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadResources()
{
    Node *objnode = universe.DeleteResourceList.head;
    DeleteResource *deleteResource;
    Resource *resource;

    if (objnode == NULL)
    {
        return;
    }

    do
    {
        deleteResource = (DeleteResource *)listGetStructOfNode(objnode);
        resource = deleteResource->resource;

        if (resource->resourcevalue == 0)
        {
            // resource was harvested away, so give opportunity for more to grow:
            if (resource->resourceVolume)
            {
                if (resource->resourceVolume->actualnumber > 0)
                {
                    resource->resourceVolume->actualnumber--;
                }
            }
        }

        if (resource->objtype == OBJ_AsteroidType)
        {
            if (((Asteroid *)resource)->resourcevalue > 0)
            {
                BreakAsteroidUp((Asteroid *)resource);
            }
        }

        univReallyDeleteThisResourceRightNow(resource);

        objnode = objnode->next;

    } while (objnode != NULL);

    listDeleteAll(&universe.DeleteResourceList);
}

/*-----------------------------------------------------------------------------
    Name        : DeleteDeadDerelict
    Description : Deletes an individual derelict, and removes all references
                  to it.  Also frees its contents.
    Inputs      : derelict - derelict to delete
                  deathBy - how it died.  Used to create an effect.  -1 means no effect.
    Outputs     :
    Return      : TRUE if an effect for deleting the derelict was played
----------------------------------------------------------------------------*/
bool DeleteDeadDerelict(Derelict *derelict, sdword deathBy)
{
    Effect *effect;
    etgeffectstatic *explosion = NULL;
    etglod *etgLOD;
    sdword LOD;
    real32 colSize;
    udword colSizeDword, velMagnitudeDword;
    real32 velMagnitude;

    if (derelict->derelicttype == HyperspaceGate)
    {
        hsDerelictDied(derelict);
    }

    bobObjectDied((SpaceObj *)derelict,&universe.collBlobList);
    //if sensors manager is running, delete it from any sm spheres
    if (smSensorsActive)
    {
        smObjectDied((SpaceObj *)derelict);
    }

    /* shut off the derelicts sounds */
    soundEventDerelictRemove(derelict);

    univRemoveAllDerelictReferencesFromSpecialShips(derelict);
    pingObjectDied((SpaceObj *)derelict);
    ccRemoveShip(&universe.mainCameraCommand,(Ship *)derelict);
    // Update Command Layer to reflect derelict death
    univRemoveAllTargetReferencesFromBullets((SpaceObjRotImpTarg *)derelict);
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)derelict);
    clDerelictDied(&universe.mainCommandLayer,derelict);

    IDToPtrTableObjDied(&DerelictIDToPtr,derelict->derelictID.derelictNumber);

#if ETG_DISABLEABLE
    if (deathBy >= 0 && etgEffectsEnabled)
#else
    if (deathBy >= 0)
#endif
    {                                                       //if we should create an effect
        etgLOD = etgDeathEventTableDerelict[derelict->derelicttype][deathBy];
        if (etgLOD != NULL)
        {
            LOD = derelict->currentLOD;
            explosion = etgLOD->level[min(etgLOD->nLevels - 1, LOD)];
        }
        else
        {
            explosion = NULL;
        }
        if (explosion)
        {
            colSize = ((ShipStaticInfo *)(derelict->staticinfo))->staticheader.staticCollInfo.collspheresize;
            //colSize *= derelict->magnitudeSquared;
            colSizeDword = TreatAsUdword(colSize);

            velMagnitude = fsqrt(vecMagnitudeSquared(derelict->posinfo.velocity));
            velMagnitude /= derelict->staticinfo->staticheader.maxvelocity;
            velMagnitudeDword = TreatAsUdword(velMagnitude);
            //??? EAF_ flags ???
            effect = etgEffectCreate(explosion, derelict, NULL, NULL, NULL, 1.0f, 0, 2, colSizeDword, velMagnitudeDword);
            bitSet(effect->flags, SOF_ForceVisible);
        }
    }
    univFreeDerelictContents(derelict);
    listRemoveNode(&derelict->derelictlink);
    listRemoveNode(&derelict->impactablelink);
    derelict->derelictlink.belongto = NULL;
    derelict->impactablelink.belongto = NULL;
    univRemoveFromWorldList(derelict);                      //if it's a planet, remove it from the planet list
    return(explosion != NULL);
}

/*-----------------------------------------------------------------------------
    Name        : univReallyDeleteThisDerelictRightNow
    Description : Like you need to ask?  It kills it.  No effects, no melodrama,
                  no waiting around to delete it later.  Just BOOM!
    Inputs      : derelict - derelict to delete
    Outputs     :
    Return      : do not call this while you are walking any derelict-related
                  linked lists.
----------------------------------------------------------------------------*/
void univReallyDeleteThisDerelictRightNow(Derelict *derelict)
{
    bitSet(derelict->flags, SOF_Dead);
    DeleteDeadDerelict(derelict, -1);
//    listRemoveNode(&derelict->derelictlink);
//    listRemoveNode(&derelict->impactablelink);
    univRemoveObjFromRenderList((SpaceObj *)derelict);
//    univRemoveFromWorldList(derelict);                      //if it's a planet, remove it from the planet list
    listDeleteNode(&derelict->objlink);
}

/*-----------------------------------------------------------------------------
    Name        : univWipeDerelictOutOfExistance
    Description : This function deletes a derelict upon command of the death effect.
    Inputs      : derelict - derelict to go bye-bye
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univWipeDerelictOutOfExistance(Derelict *derelict)
{
    bobObjectDied((SpaceObj *)derelict,&universe.collBlobList);
    nisObjectDied((SpaceObj *)derelict);
    if (derelict->derelictlink.belongto != NULL)
    {
        listRemoveNode(&derelict->derelictlink);
    }
    if (derelict->impactablelink.belongto != NULL)
    {
        listRemoveNode(&derelict->impactablelink);
    }
    univRemoveObjFromRenderList((SpaceObj *)derelict);
    etgShipDied((Ship *)derelict);
    univRemoveFromWorldList(derelict);                      //if it's a planet, remove it from the planet list
    listDeleteNode(&derelict->objlink);
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadDerelicts
    Description : deletes all dead derelicts
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadDerelicts()
{
    Node *objnode = universe.DeleteDerelictList.head;
    DeleteDerelict *deleteDerelict;
    Derelict *derelict;

    if (objnode == NULL)
    {
        return;
    }

    do
    {
        deleteDerelict = (DeleteDerelict *)listGetStructOfNode(objnode);
        derelict = deleteDerelict->derelict;
        dbgAssert(derelict->objtype == OBJ_DerelictType);

        // don't totally delete ship, until after explosion
#if ETG_DISABLEABLE
        if (!etgEffectsEnabled)
        {
            univReallyDeleteThisDerelictRightNow(derelict);
        }
        else
#endif
        {
            if (!DeleteDeadDerelict(derelict, deleteDerelict->deathBy))
            {                                               //if there was no effect
                univRemoveObjFromRenderList((SpaceObj *)derelict);
                listDeleteNode(&derelict->objlink);         //finish deleting it properly
            }
        }

        objnode = objnode->next;

    } while (objnode != NULL);

    listDeleteAll(&universe.DeleteDerelictList);
}

/*-----------------------------------------------------------------------------
    Name        : DeleteDeadMissile
    Description : Deletes a dead missile, and removes all references to it.
                  Also frees it contents.
    Inputs      : deathBy - how the missile/mine died
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DeleteDeadMissile(Missile *missile, sdword deathBy)
{
    etgeffectstatic *stat;
    sdword LOD;
    bool fatalHit;
    real32 floatDamage;
    udword intDamage;
    vector hitLocation;

    if (deathBy >= 0)
    {                                                       //if it was destroyed by forced attack
        if (missile->hitEffect != NULL)
        {                                                   //use the regular hit effect
            LOD = missile->currentLOD;
            if (LOD < missile->hitEffect->nLevels)
            {
                stat = missile->hitEffect->level[LOD];
            }
            else
            {
                stat = NULL;
            }
        }
        else
        {
            stat = NULL;
        }
        if (univSpaceObjInRenderList((SpaceObj *)missile))
        {
#if ETG_DISABLEABLE
            if (stat != NULL && etgHitEffectsEnabled && etgEffectsEnabled && (!etgFrequencyExceeded(stat)))
#else
            if (stat != NULL && etgHitEffectsEnabled && (!etgFrequencyExceeded(stat)))
#endif
            {
                hitLocation = missile->posinfo.position;

                floatDamage = missile->damage;
                if (RGLtype == SWtype)
                {                                               //smaller hit effects in software
                    floatDamage *= etgSoftwareScalarHit;
                }
                intDamage = TreatAsUdword(floatDamage);
                fatalHit = FALSE;
                etgEffectCreate(stat, NULL, &hitLocation, &missile->posinfo.velocity, &missile->rotinfo.coordsys, 1.0, 0, 2, intDamage, fatalHit);
            }
        }
    }
    bobObjectDied((SpaceObj *)missile,&universe.collBlobList);
    pingObjectDied((SpaceObj *)missile);

    univRemoveAllTargetReferencesFromBullets((SpaceObjRotImpTarg *)missile);
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)missile);
    nisRemoveMissileReference(missile);
    clMissileDied(&universe.mainCommandLayer,missile);

    IDToPtrTableObjDied(&MissileIDToPtr,missile->missileID.missileNumber);

    univFreeMissileContents(missile);
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadMissiles
    Description : deletes all dead missiles
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadMissiles()
{
    Node *objnode = universe.DeleteMissileList.head;
    DeleteMissile *deleteMissile;
    Missile *missile;

    if (objnode == NULL)
    {
        return;
    }

    do
    {
        deleteMissile = (DeleteMissile *)listGetStructOfNode(objnode);
        missile = deleteMissile->missile;

        DeleteDeadMissile(missile, deleteMissile->deathBy);
        listRemoveNode(&missile->missilelink);
        listRemoveNode(&missile->impactablelink);
        univRemoveObjFromRenderList((SpaceObj *)missile);
        listDeleteNode(&missile->objlink);

        objnode = objnode->next;

    } while (objnode != NULL);

    listDeleteAll(&universe.DeleteMissileList);
}

/*-----------------------------------------------------------------------------
    Name        : univGetResourceStatistics
    Description : outputs the total resourceValue, numHarvestableResources, and numAsteroid0s in the universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univGetResourceStatistics(sdword *resourceValue,sdword *numHarvestableResources,sdword *numAsteroid0s)
{
    Node *objnode = universe.ResourceList.head;
    Resource *resource;
    sdword total = 0;
    sdword numharvestable = 0;
    sdword numasteroid0s = 0;

    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        dbgAssert(resource->flags & SOF_Resource);
        if (nisIsRunning && (resource->health >= REALlyBig))
        {
            goto nextnode;       // don't pick NIS resources
        }
#if 0       // Asteroid0's not in Resource list anymore so this isn't needed
        if ((resource->objtype == OBJ_AsteroidType) && (((Asteroid *)resource)->asteroidtype == Asteroid0))
        {
            goto nextnode;      // don't harvest pebbles
        }
#endif
        total += resource->resourcevalue;
        numharvestable++;

nextnode:
        objnode = objnode->next;
    }

    for (objnode = universe.MinorSpaceObjList.head;objnode != NULL;objnode = objnode->next)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        if ((resource->objtype == OBJ_AsteroidType) && (((Asteroid *)resource)->asteroidtype == Asteroid0))
        {
            numasteroid0s++;
        }
    }

    *resourceValue = total;
    *numHarvestableResources = numharvestable;
    *numAsteroid0s = numasteroid0s;
}

bool univObjectOutsideWorld(SpaceObj *obj)
{
    vector pos = obj->posinfo.position;
    vector oldpos = pos;
    vector zero = { 0.0f, 0.0f, 0.0f };

    pieMovePointClipToLimits(smUniverseSizeX,smUniverseSizeY,smUniverseSizeZ,&zero,&pos);

    if (vecAreEqual(pos,oldpos))
    {
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : univFindNearestResource
    Description : finds the nearest resource to ship in optional volume specified by volumePosition and volumeRadius.
                  If VolumeRadius == 0, then volume is ignored
    Inputs      :
    Outputs     :
    Return      : returns the nearest resource (or NULL if none found)
----------------------------------------------------------------------------*/
Resource *univFindNearestResource(Ship *ship,real32 volumeRadius,vector *volumePosition)
{
    Node *objnode = universe.ResourceList.head;
    vector diff;
    real32 dist;
    real32 mindist = (real32)1e20;
    Resource *closestResource = NULL;
    Resource *resource;

    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        dbgAssert(resource->flags & SOF_Resource);
        if (nisIsRunning && (resource->health >= REALlyBig))
        {
            goto nextnode;       // don't pick NIS resources
        }
#if 0       // Asteroid0's not in Resource list anymore so this isn't needed
        if ((resource->objtype == OBJ_AsteroidType) && (((Asteroid *)resource)->asteroidtype == Asteroid0))
        {
            goto nextnode;      // don't harvest pebbles
        }
#endif
        if (ResourceMovingTooFast(resource))
        {
            goto nextnode;
        }

        if (ResourceAlreadyBeingHarvested(&universe.mainCommandLayer,NULL,resource))
        {
            goto nextnode;
        }

        if (resource->resourceNotAccessible)
        {
            goto nextnode;
        }

        if (volumeRadius > 0.0f)
        {
            vecSub(diff,resource->posinfo.position,*volumePosition);

            if (!isBetweenInclusive(diff.x,-volumeRadius,volumeRadius))
            {
                goto nextnode;
            }

            if (!isBetweenInclusive(diff.y,-volumeRadius,volumeRadius))
            {
                goto nextnode;
            }

            if (!isBetweenInclusive(diff.z,-volumeRadius,volumeRadius))
            {
                goto nextnode;
            }
        }

        if (univObjectOutsideWorld((SpaceObj *)resource))
        {
            goto nextnode;
        }

        vecSub(diff,resource->posinfo.position,ship->posinfo.position);
        dist = vecMagnitudeSquared(diff);

        if (dist < mindist)
        {
            mindist = dist;
            closestResource = resource;
        }

nextnode:
        objnode = objnode->next;
    }

    return closestResource;
}

/*-----------------------------------------------------------------------------
    Name        : univGetChecksum
    Description : returns a checksum representing the status of the universe
    Inputs      :
    Outputs     : numShipsInChecksum
    Return      : checksum of mission sphere at this point in time
----------------------------------------------------------------------------*/
real32 univGetChecksum(sdword *numShipsInChecksum)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    Derelict *derelict;
    Resource *resource;
#if BINNETLOG
    Bullet   *bullet;
#endif
    sdword countShips = 0;

    real32 x=0.0f;
    real32 y=0.0f;
    real32 z=0.0f;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        if ((ship->flags & SOF_Dead) == 0)
        {
            x += ship->posinfo.position.x;
            y += ship->posinfo.position.y;
            z += ship->posinfo.position.z;

            if ((netlogfile) && (logEnable == LOG_VERBOSE))
            {
#if BINNETLOG
                binnetlogShipInfo bns;

                bns.header = makenetcheckHeader('S','S','S','S');
                bns.shipID = ship->shipID.shipNumber;
                bns.playerIndex = ship->playerowner->playerIndex;
                bns.shiprace = ship->shiprace;
                bns.shiptype = ship->shiptype;

                if (ship->command)
                {
                    bns.shiporder = ship->command->ordertype.order;
                    bns.shipattributes = ship->command->ordertype.attributes;
                }
                else
                {
                    bns.shiporder = -1;
                    bns.shipattributes = -1;
                }

                bns.tacticstype = ship->tacticstype;
                bns.isDodging = ship->isDodging;
                bns.DodgeDir = ship->DodgeDir;

                bns.health = ship->health;
                bns.x = ship->posinfo.position.x;    bns.y = ship->posinfo.position.y;    bns.z = ship->posinfo.position.z;
                bns.vx = ship->posinfo.velocity.x;  bns.vy = ship->posinfo.velocity.y;    bns.vz = ship->posinfo.velocity.z;
                bns.fuel = ship->fuel;

                fwrite(&bns, sizeof(bns), 1 ,netlogfile);
#else
                fprintf(netlogfile,"S:%d %d %d %d (%f %f %f) %f %d %d %d %d (%d %d)\n",ship->playerowner->playerIndex,ship->shiprace,ship->shiptype,
                        ship->shipID.shipNumber,ship->posinfo.position.x,ship->posinfo.position.y,ship->posinfo.position.z,ship->fuel,ship->tacticstype,ship->isDodging,ship->DodgeDir,ship->tactics_ordertype,
                        (ship->command == NULL) ? -1 : ship->command->ordertype.order,
                        (ship->command == NULL) ? -1 : ship->command->ordertype.attributes);
#endif

                if ((ship->command) && (ship->command->ordertype.order == COMMAND_COLLECTRESOURCE))
                {
                    CollectResourceCommand *collect = &ship->command->collect;
#if BINNETLOG
                    binnetlogShipResourceInfo bnr;
                    bnr.header = makenetcheckHeader('S','R','S','R');
                    bnr.resourceID = (collect->resource != NULL) ? collect->resource->resourceID.resourceNumber : -1;
                    bnr.volume = collect->resourceVolumeSize;
                    bnr.x = collect->resourceVolumePosition.x;
                    bnr.y = collect->resourceVolumePosition.y;
                    bnr.z = collect->resourceVolumePosition.z;
                    fwrite(&bnr, sizeof(bnr), 1 ,netlogfile);
#else
                    fprintf(netlogfile,"R:%d ",(collect->resource != NULL) ? collect->resource->resourceID.resourceNumber : -1,collect->resourceVolumeSize,
                            collect->resourceVolumePosition.x,collect->resourceVolumePosition.y,collect->resourceVolumePosition.z);
#endif
                }

                if (ship->dockInfo != NULL)
                {
                    DockInfo *dockInfo = ship->dockInfo;
                    sdword i;
#if BINNETLOG
                    binnetlogShipDockInfo bnd;
                    udword orflag = 1;
                    bnd.header = makenetcheckHeader('S','D','S','D');
                    bnd.busyness = dockInfo->busyness;
                    bnd.numDockPoints = dockInfo->numDockPoints;
                    bnd.thisDockBusy = 0;
                    for (i=0;i<dockInfo->numDockPoints;i++)
                    {
                        if (dockInfo->dockpoints[i].thisDockBusy)
                        {
                            bnd.thisDockBusy |= orflag;
                        }
                        orflag <<= 1;
                    }
                    fwrite(&bnd, sizeof(bnd), 1 ,netlogfile);
#else
                    fprintf(netlogfile,"B:%d P:",dockInfo->busyness);
                    for (i=0;i<dockInfo->numDockPoints;i++)
                    {
                        fprintf(netlogfile," %d",dockInfo->dockpoints[i].thisDockBusy ? 1 : 0);
                    }
                    fprintf(netlogfile,"\n");
#endif
                }
                if (ship->madBindings != NULL)
                {
#if BINNETLOG
                    binnetlogShipMadInfo bnm;
                    bnm.header = makenetcheckHeader('S','M','S','M');
                    bnm.info[0] = ship->madAnimationFlags;
                    bnm.info[1] = ship->madGunStatus;
                    bnm.info[2] = ship->madWingStatus;
                    bnm.info[3] = ship->madDoorStatus;
                    bnm.info[4] = ship->madSpecialStatus;
                    bnm.info[5] = ship->cuedAnimationIndex;
                    bnm.info[6] = ship->cuedAnimationType;
                    bnm.info[7] = 0;
                    bnm.info[8] = 0;
                    fwrite(&bnm, sizeof(bnm), 1 ,netlogfile);
#else
                    //                  flags,guns,wings,dock,special,index,type
                    fprintf(netlogfile,"M: %df %dg %dw %dd %ds %di %dt\n", ship->madAnimationFlags,ship->madGunStatus,ship->madWingStatus, ship->madDoorStatus, ship->madSpecialStatus,ship->cuedAnimationIndex,ship->cuedAnimationType);
#endif
                }
            }

            countShips++;
        }
        objnode = objnode->next;
    }

#ifndef HW_Release

#if BINNETLOG
    if ((netlogfile) && (logEnable == LOG_VERBOSE))
    {
        objnode = universe.BulletList.head;
        while (objnode != NULL)
        {
            bullet = (Bullet *)listGetStructOfNode(objnode);

            if ((!bitTest(bullet->flags, SOF_Dead)))
            {
#if BINNETLOG
                binnetlogBulletInfo bi;

                bi.header = makenetcheckHeader('B','B','B','B');
                bi.bullettype = bullet->bulletType;
                bi.bulletplayerowner = (bullet->playerowner!= NULL) ? bullet->playerowner->playerIndex : -1;
                bi.bulletowner = (bullet->owner != NULL) ? bullet->owner->shipID.shipNumber : -1;
                bi.x = bullet->posinfo.position.x; bi.y = bullet->posinfo.position.y; bi.z = bullet->posinfo.position.z;
                bi.vx = bullet->posinfo.velocity.x; bi.vy = bullet->posinfo.velocity.y; bi.vz = bullet->posinfo.velocity.z;
                bi.timelived = bullet->timelived;
                bi.totallifetime = bullet->totallifetime;
                bi.traveldist = bullet->traveldist;
                bi.damage = bullet->damage;
                bi.damageFull = bullet->damageFull;
                bi.DFGFieldEntryTime = bullet->DFGFieldEntryTime;
                bi.BulletSpeed = 0.0f;
                bi.collBlobSortDist = (bullet->collMyBlob) ? bullet->collMyBlob->sortDistance : 0.0f;
                fwrite(&bi,sizeof(bi),1,netlogfile);
#else
                if (bullet->playerowner!= NULL)
                    fprintf(netlogfile,"B: %d %d ", universe.univUpdateCounter, bullet->playerowner->playerIndex);
                else
                    fprintf(netlogfile,"B: %d N ", universe.univUpdateCounter);

                fprintf(netlogfile,"(%f %f %f) [%f %f] %f <%f %f> %f %f ",
                        bullet->posinfo.position.x,
                        bullet->posinfo.position.y,
                        bullet->posinfo.position.z,
                        bullet->timelived,
                        bullet->totallifetime,
                        bullet->traveldist,
                        bullet->damage,
                        bullet->damageFull,
                        bullet->DFGFieldEntryTime,
                        bullet->BulletSpeed);

                if (bullet->collMyBlob)
                    fprintf(netlogfile, "%f\n", bullet->collMyBlob->sortDistance);
                else
                    fprintf(netlogfile, "N\n");
#endif
            }

            objnode = objnode->next;
        }
    }
#endif

#if BINNETLOG
    if ((netlogfile) && (logEnable == LOG_VERBOSE))
    {
        clChecksum();
    }
#endif

    objnode = universe.DerelictList.head;
    while (objnode != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(objnode);
        if ((derelict->flags & SOF_Dead) == 0)
        {
            x += derelict->posinfo.position.x;
            y += derelict->posinfo.position.y;
            z += derelict->posinfo.position.z;

            if ((netlogfile) && (logEnable == LOG_VERBOSE))
            {
#if BINNETLOG
                binnetDerelictInfo di;
                di.header = makenetcheckHeader('D','D','D','D');
                di.derelictid = derelict->derelictID.derelictNumber;
                di.derelicttype = derelict->derelicttype;
                di.health = derelict->health;
                di.x = derelict->posinfo.position.x; di.y = derelict->posinfo.position.y; di.z = derelict->posinfo.position.z;
                di.vx = derelict->posinfo.velocity.x; di.vy = derelict->posinfo.velocity.y; di.vz = derelict->posinfo.velocity.z;
                fwrite(&di,sizeof(di),1,netlogfile);
#else
                fprintf(netlogfile,"  Derelict:%d %s (%f %f %f)\n",derelict->derelictID.derelictNumber,DerelictTypeToStr(derelict->derelicttype),
                        derelict->posinfo.position.x,derelict->posinfo.position.y,derelict->posinfo.position.z);
#endif
            }

            countShips++;
        }
        objnode = objnode->next;
    }

    objnode = universe.ResourceList.head;
    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);

        x += resource->posinfo.position.x;
        y += resource->posinfo.position.y;
        z += resource->posinfo.position.z;

#if BINNETLOG
        if ((netlogfile) && (logEnable == LOG_VERBOSE))
        {
        binnetResourceInfo ri;
        ri.header = makenetcheckHeader('R','R','R','R');
        ri.resourceid = resource->resourceID.resourceNumber;
        ri.resourcetype = resource->objtype;
        ri.resourceValue = resource->resourcevalue;
        ri.health = resource->health;
        ri.x = resource->posinfo.position.x; ri.y = resource->posinfo.position.y; ri.z = resource->posinfo.position.z;
        ri.vx = resource->posinfo.velocity.x; ri.vy = resource->posinfo.velocity.y; ri.vz = resource->posinfo.velocity.z;
        fwrite(&ri,sizeof(ri),1,netlogfile);
        }
#endif

        countShips++;

        objnode = objnode->next;
    }

#endif

    *numShipsInChecksum = countShips;

    return (x+y+z);
}

/*-----------------------------------------------------------------------------
    Name        : univCalcShipChecksum
    Description : returns a checksum representing the status of the ships in the universe
    Inputs      :
    Outputs     : numShipsInChecksum
    Return      : checksum of mission sphere at this point in time
----------------------------------------------------------------------------*/
udword univCalcShipChecksum(void)
{
    udword  shipcheck=0;
    Node   *search;

    Bullet *bullet;

    /*
    Ship   *ship;
    udword  i;

    search = universe.ShipList.head;

    while (search!=NULL)
    {
        ship = listGetStructOfNode(search);
        if(ship->flags & SOF_Dead)
        {
            //don't consider dead ships...they're bad news
            //they still get written to the syncdump.txt, but
            //they won't generate a sync error.
            search = search->next;
            continue;
        }
        shipcheck += TreatAsUdword(ship->collOptimizeDist);

        shipcheck += TreatAsUdword(ship->posinfo.position.x);
        shipcheck += TreatAsUdword(ship->posinfo.position.y);
        shipcheck += TreatAsUdword(ship->posinfo.position.z);
        shipcheck += TreatAsUdword(ship->posinfo.velocity.x);
        shipcheck += TreatAsUdword(ship->posinfo.velocity.y);
        shipcheck += TreatAsUdword(ship->posinfo.velocity.z);
        shipcheck += TreatAsUdword(ship->posinfo.force.x);
        shipcheck += TreatAsUdword(ship->posinfo.force.y);
        shipcheck += TreatAsUdword(ship->posinfo.force.z);
        shipcheck += (udword)ship->posinfo.isMoving;
        shipcheck += (udword)ship->posinfo.haventCalculatedDist;

        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m11);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m21);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m31);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m12);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m22);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m32);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m13);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m23);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m33);
        shipcheck += TreatAsUdword(ship->rotinfo.rotspeed.x);
        shipcheck += TreatAsUdword(ship->rotinfo.rotspeed.y);
        shipcheck += TreatAsUdword(ship->rotinfo.rotspeed.z);
        shipcheck += TreatAsUdword(ship->rotinfo.torque.x);
        shipcheck += TreatAsUdword(ship->rotinfo.torque.y);
        shipcheck += TreatAsUdword(ship->rotinfo.torque.z);

        shipcheck += TreatAsUdword(ship->collInfo.collOffset.x);
        shipcheck += TreatAsUdword(ship->collInfo.collOffset.y);
        shipcheck += TreatAsUdword(ship->collInfo.collOffset.z);
        shipcheck += TreatAsUdword(ship->collInfo.collPosition.x);
        shipcheck += TreatAsUdword(ship->collInfo.collPosition.y);
        shipcheck += TreatAsUdword(ship->collInfo.collPosition.z);

        for (i=0;i<8;i++)
        {
            shipcheck += TreatAsUdword(ship->collInfo.rectpos[i].x);
            shipcheck += TreatAsUdword(ship->collInfo.rectpos[i].y);
            shipcheck += TreatAsUdword(ship->collInfo.rectpos[i].z);
        }

        shipcheck += TreatAsUdword(ship->health);
        shipcheck += TreatAsUdword(ship->lasttimecollided);

        for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
            shipcheck += TreatAsUdword(ship->nonstatvars.thruststrength[i]);
        for (i=0;i<NUM_ROT_DEGOFFREEDOM;i++)
            shipcheck += TreatAsUdword(ship->nonstatvars.rotstrength[i]);
        for (i=0;i<NUM_TURN_TYPES;i++)
            shipcheck += TreatAsUdword(ship->nonstatvars.turnspeed[i]);

        shipcheck += (udword)ship->aistate;
        shipcheck += ship->specialFlags&(~(SPECIAL_Resourcing|SPECIAL_Finished_Resource));
        shipcheck += ship->specialFlags2;

        for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
            shipcheck += ship->salvageNumTagged[i];

        shipcheck += TreatAsUdword(ship->moveTo.x);
        shipcheck += TreatAsUdword(ship->moveTo.y);
        shipcheck += TreatAsUdword(ship->moveTo.z);
        shipcheck += TreatAsUdword(ship->moveFrom.x);
        shipcheck += TreatAsUdword(ship->moveFrom.y);
        shipcheck += TreatAsUdword(ship->moveFrom.z);
        shipcheck += TreatAsUdword(ship->formationOffset.x);
        shipcheck += TreatAsUdword(ship->formationOffset.y);
        shipcheck += TreatAsUdword(ship->formationOffset.z);
        shipcheck += TreatAsUdword(ship->formationHeading.x);
        shipcheck += TreatAsUdword(ship->formationHeading.y);
        shipcheck += TreatAsUdword(ship->formationHeading.z);
        shipcheck += TreatAsUdword(ship->engineOffset.x);
        shipcheck += TreatAsUdword(ship->engineOffset.y);
        shipcheck += TreatAsUdword(ship->engineOffset.z);
        shipcheck += TreatAsUdword(ship->enginePosition.x);
        shipcheck += TreatAsUdword(ship->enginePosition.y);
        shipcheck += TreatAsUdword(ship->enginePosition.z);

        shipcheck += TreatAsUdword(ship->rowOriginalPoint.x);
        shipcheck += TreatAsUdword(ship->rowOriginalPoint.y);
        shipcheck += TreatAsUdword(ship->rowOriginalPoint.z);
        shipcheck += TreatAsUdword(ship->rowOutOfWayPoint.x);
        shipcheck += TreatAsUdword(ship->rowOutOfWayPoint.y);
        shipcheck += TreatAsUdword(ship->rowOutOfWayPoint.z);

        shipcheck += (udword)ship->shiptype;
        shipcheck += (udword)ship->shiprace;
        shipcheck += (udword)ship->tacticstype;

        shipcheck += (udword)ship->isDodging;
        shipcheck += TreatAsUdword(ship->DodgeTime);
        shipcheck += (udword)ship->DodgeDir;
        shipcheck += (udword)ship->tacticsFormationVar1;
        shipcheck += ship->DodgeFlag;
        shipcheck += (udword)ship->tactics_ordertype;
        shipcheck += ship->tacticsTalk;
        shipcheck += (udword)ship->tacticsNeedToFlightMan;

        shipcheck += TreatAsUdword(ship->timeCreated);
        shipcheck += (udword)ship->kamikazeState;

        shipcheck += TreatAsUdword(ship->kamikazeVector.x);
        shipcheck += TreatAsUdword(ship->kamikazeVector.y);
        shipcheck += TreatAsUdword(ship->kamikazeVector.z);

        shipcheck += TreatAsUdword(ship->speedBurstTime);
        shipcheck += TreatAsUdword(ship->singlePlayerSpeedLimiter);
        shipcheck += TreatAsUdword(ship->deathtime);
        shipcheck += ship->madAnimationFlags;
        shipcheck += ship->madGunStatus;
        shipcheck += ship->madWingStatus;
        shipcheck += ship->madDoorStatus;
        shipcheck += ship->madSpecialStatus;
        shipcheck += ship->cuedAnimationIndex;
        shipcheck += ship->cuedAnimationType;
        shipcheck += ship->nextAnim;
        shipcheck += (udword)ship->forceCancelDock;
        shipcheck += (udword)ship->resources;
        shipcheck += TreatAsUdword(ship->fuel);
        shipcheck += TreatAsUdword(ship->lastTimeRepaired);

        shipcheck += (udword)ship->shipidle;
        shipcheck += (udword)ship->autostabilizeship;
        shipcheck += (udword)ship->autostabilizestate;
        shipcheck += (udword)ship->autostabilizewait;
        shipcheck += (udword)ship->aistateattack;
        shipcheck += (udword)ship->flightmanState1;
        shipcheck += (udword)ship->flightmanState2;
        shipcheck += (udword)ship->rcstate1;
        shipcheck += (udword)ship->rcstate2;
        shipcheck += (udword)ship->aistatecommand;
        shipcheck += (udword)ship->dontrotateever;
        shipcheck += (udword)ship->dontapplyforceever;
        shipcheck += (udword)ship->shipisattacking;
        shipcheck += (udword)ship->colorScheme;
        shipcheck += (udword)ship->recentlyAttacked;
        shipcheck += (udword)ship->recentlyFiredUpon;
        shipcheck += (udword)ship->rowState;
        shipcheck += TreatAsUdword(ship->aidescend);
        shipcheck += ship->flightman;

        shipcheck += ship->attackvars.flightmansLeft;
        shipcheck += ship->attackvars.attacksituation;
        shipcheck += ship->attackvars.attackevents;

        shipcheck += TreatAsUdword(ship->dockvars.destination.x);
        shipcheck += TreatAsUdword(ship->dockvars.destination.y);
        shipcheck += TreatAsUdword(ship->dockvars.destination.z);

        shipcheck += (udword)ship->dockvars.dockstate;
        shipcheck += (udword)ship->dockvars.dockstate2;
        shipcheck += (udword)ship->dockvars.dockstate3;
        shipcheck += (udword)ship->dockvars.reserveddocking;

        shipcheck += (udword)ship->visibleToWho;
        shipcheck += (udword)ship->visibleToWhoPreviousFrame;
        shipcheck += (udword)ship->howDidIDie;
        shipcheck += (udword)ship->whoKilledMe;
        shipcheck += (udword)ship->nDamageLights;

        shipcheck += TreatAsUdword(ship->holdingPatternPos.x);
        shipcheck += TreatAsUdword(ship->holdingPatternPos.y);
        shipcheck += TreatAsUdword(ship->holdingPatternPos.z);
        shipcheck += TreatAsUdword(ship->damageModifier);

        shipcheck += ship->tractorbeaminfo;
        shipcheck += ship->tractorvector;

        shipcheck += (udword)ship->tractorstate;

        for (i=0;i<4;i++)
            shipcheck += (udword)ship->tractorPosinfo[i];

        shipcheck += (udword)ship->needNewLeader;
        shipcheck += (udword)ship->newDockIndexTransfer;

        shipcheck += TreatAsUdword(ship->shipDeCloakTime);
        shipcheck += (udword)ship->shipID.shipNumber;

        search = search->next;
    }
    */

    search = universe.BulletList.head;

    while (search!=NULL)
    {
        bullet = (Bullet *)listGetStructOfNode(search);
        if(bullet->flags & SOF_Dead)
        {
            search = search->next;
            continue;
        }

        //shipcheck += TreatAsUdword(bullet->posinfo.position.x);
        //shipcheck += TreatAsUdword(bullet->posinfo.position.y);
        //shipcheck += TreatAsUdword(bullet->posinfo.position.z);

        //shipcheck += TreatAsUdword(bullet->posinfo.velocity.x);
        //shipcheck += TreatAsUdword(bullet->posinfo.velocity.y);
        //shipcheck += TreatAsUdword(bullet->posinfo.velocity.z);
        //shipcheck += (udword)bullet->posinfo.isMoving;
        //shipcheck += (udword)bullet->posinfo.haventCalculatedDist;

        //shipcheck += TreatAsUdword(bullet->lengthmag);
        //shipcheck += TreatAsUdword(bullet->traveldist);
        //shipcheck += TreatAsUdword(bullet->timelived);
        //shipcheck += TreatAsUdword(bullet->totallifetime);
        //shipcheck += TreatAsUdword(bullet->damage);
        //shipcheck += TreatAsUdword(bullet->damageFull);
        //shipcheck += (udword)bullet->SpecialEffectFlag;
        //shipcheck += TreatAsUdword(bullet->DFGFieldEntryTime);
        //shipcheck += TreatAsUdword(bullet->BulletSpeed);
        if(bullet->owner != NULL)
        {
            shipcheck += 128;
        }
        //if(bullet->gunowner != NULL)
        //{
        //    shipcheck += 32;
        //}
        search = search->next;
    }

    return (shipcheck);
}

/*-----------------------------------------------------------------------------
    Name        : univMinorSetupShipForControl
    Description : A version of univSetupShipForControl specific to ships with
                    the SOF_DontApplyPhysics bit set (such as NIS ships).
    Inputs      : Ship - ship to "set up"
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univMinorSetupShipForControl(Ship *ship)
{
    sdword i;

    ship->gettingrocked = NULL;     // always make sure this gets set to NULL - don't want any bad references

    dmgShipThink(ship);

    //update engine trails
    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (ship->trail[i] != NULL)
        {
            trailUpdate(ship->trail[i], &ship->enginePosition);
        }
    }

}

void univSetupShipForControl(Ship *ship)
{
    ShipStaticInfo *shipstatic;
    sdword i,t;

    ship->gettingrocked = NULL;     // always make sure this gets set to NULL - don't want any bad references

    if (ship->flags & SOF_Dead)
    {
        return;
    }

    shipstatic = (ShipStaticInfo *)ship->staticinfo;

    //vecZeroVector(ship->posinfo.force);
    //vecZeroVector(ship->rotinfo.torque);

    if((universe.univUpdateCounter & TW_ProximitySensorSearchRate) == TW_ProximitySensorSearchFrame)
    {
        //this univupdate, proximity sensors will search again for
        //ships nearby, lets make all ships 'invisible' and then let
        //the proximity sensor make those ships visible that should be

        //do a check like so here: Check if ship has just become
        //invisible to other ships, if so, remove it from being targetted
        //by those people.
        if(ship->flags & SOF_Cloaked)
        {
            for(t = 0; t <= universe.numPlayers; t++)
            {
                //looking for state such that the player COULD see the ship in the
                //previous frame(2 ago), but now cannot (1 ago)
                if(bitTest(ship->visibleToWhoPreviousFrame,0x01<<t))
                {
                    //ship was seen 2 frames ago
                    if(!bitTest(ship->visibleToWho,0x01<<t))
                    {
                        //now ship isn't visible to player
                        shipHasJustCloaked(ship);
                        break;  //above function will take care of targetting info for all ships/players
                    }
                }
            }
        }
        ship->visibleToWhoPreviousFrame = ship->visibleToWho;
        ship->visibleToWho = 0;
    }

    ship->shipidle = TRUE;
    ship->autostabilizeship = TRUE;

    //clear this flag incase it is a target of a salcapcorvette
    bitClear(ship->specialFlags2,SPECIAL_2_SalvageTargetAlreadyDriven);
    // memory of attackers fades away...
    if (ship->recentlyAttacked)
    {
        if (!ship->recentAttacker)
        {
            ship->recentlyAttacked = 0;
        }
        else
        {
            --ship->recentlyAttacked;
            if (!ship->recentlyAttacked)
                ship->recentAttacker = NULL;
        }
    }
    if (ship->recentlyFiredUpon)
    {
        if (ship->firingAtUs == NULL)
        {
            ship->recentlyFiredUpon = 0;
        }
        else
        {
            ship->recentlyFiredUpon--;
            if (ship->recentlyFiredUpon == 0)
            {
                ship->firingAtUs = NULL;
            }
        }
    }

    bitClear(ship->flags, SOF_DontDrawTrails);  // always clear this bit, SOF_Disabled ships never have trails drawn

    dmgShipThink(ship);

    //update engine trails
    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (ship->trail[i] != NULL)
        {
            trailUpdate(ship->trail[i], &ship->enginePosition);
        }
    }

    if ((ship->flags & SOF_Disabled) == 0)
    {
        if (shipstatic->repairDamage != 0)
        {
            if (ship->health < shipstatic->maxhealth)
            {
                real32 repairTime;
                real32 repairDamage;

                if (ship->recentlyAttacked | ship->shipisattacking | bitTest(ship->specialFlags, SPECIAL_ForcedAttackStatus))
                {
                    repairTime = shipstatic->repairCombatTime;
                    repairDamage = shipstatic->repairCombatDamage;
                }
                else
                {
                    repairTime = shipstatic->repairTime;
                    repairDamage = shipstatic->repairDamage;
                }

                if ((universe.totaltimeelapsed - ship->lastTimeRepaired) >= repairTime)
                {
                    ship->lastTimeRepaired = universe.totaltimeelapsed;

                    ship->health += repairDamage;
                    if (ship->health > shipstatic->maxhealth)
                    {
                        ship->health = shipstatic->maxhealth;
                    }
                }
            }
            else
            {
                ship->lastTimeRepaired = universe.totaltimeelapsed;
            }
        }
    }

    if (shipstatic->custshipheader.CustShipHousekeep != NULL)       // put in a list, and execute all housekeeps later
    {                                                               // to make sure all of the ships have been properly setup
        growSelectAddShip(&universe.HousekeepShipList,ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : univSetupShipsForControl
    Description : initializes the shipidle and gettingrocked flags for this pass
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univSetupShipsForControl()
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    SelectCommand *housekeepSelection;
    sdword i;
    sdword numShips;

    growSelectReset(&universe.HousekeepShipList);

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        univSetupShipForControl(ship);

        objnode = objnode->next;
    }

    housekeepSelection = universe.HousekeepShipList.selection;
    numShips = housekeepSelection->numShips;

    for (i=0;i<numShips;i++)
    {
        ship = housekeepSelection->ShipPtr[i];
        if (ship)
        {
            ship->staticinfo->custshipheader.CustShipHousekeep(ship);
        }
    }

    growSelectReset(&universe.HousekeepShipList);       // don't need to keep track of them anymore
}

//same as above but for derelicts
void univSetupDerelictsForControl()
{

}
SelectCommand *getEnemiesWithinProximity(Ship *thisship,real32 retaliateZone)
{
    Player *playerowner = thisship->playerowner;
    blob *thisblob = thisship->collMyBlob;
    sdword objindex = 0;
    SelectCommand *blobships = thisblob->blobShips;
    udword num = blobships->numShips;
    SelectCommand *select;
    udword shipIndex = 0;
    Ship *ship;
    vector diff;
    real32 timesincecloak;
    real32 negRetaliateZone = -retaliateZone;

    if ((thisship->shiptype == FloatingCity) || (thisship->shiptype == TargetDrone) ||
        (bitTest(thisship->specialFlags, SPECIAL_FriendlyStatus)))
    {
        return NULL;      // traders get special status, not considered enemy to anyone
    }

    select = memAlloc(sizeofSelectCommand(num),"se(selectenemies)",Pyrophoric);    // worst case scenario

    while (objindex < num)
    {
        ship = blobships->ShipPtr[objindex];

        if (allianceIsShipAlly(ship,playerowner))
        {
            goto nextnode;      // must be enemy ship
        }
        if ((ship->shiptype == FloatingCity) || (ship->shiptype == TargetDrone) ||
            (bitTest(ship->specialFlags, SPECIAL_FriendlyStatus|SPECIAL_Hyperspacing)))
        {
            goto nextnode;      // traders get special status, not considered enemy to anyone
        }
        if (bitTest(ship->flags,SOF_Cloaked))
        {
            if(!proximityCanPlayerSeeShip(thisship->playerowner,ship))
            {
                goto nextnode;      // if ship is cloaked, and can't be seen by that ships player..
            }
        }
        timesincecloak = universe.totaltimeelapsed - ship->shipDeCloakTime;
        if(timesincecloak <= TW_SECOND_SEE_TIME)
        {
            if(timesincecloak < TW_NOONE_SEE_TIME)
            {
                //ship being considered has decloaked so recently that 'thisship' couldn't
                //possibly see it yet
                goto nextnode;
            }
            else if(timesincecloak < TW_FIRST_SEE_TIME)
            {
                //ship has decloaked so long ago that only a small percentage could see it now
                if(randombetween(0,100) > TW_FIRST_SEE_PERCENTAGE)
                {
                    goto nextnode;
                }
            }
            else if(timesincecloak < TW_SECOND_SEE_TIME)
            {
                //ship has decloaked a fair ammount of time ago, and some more ships are likely to see it now
                if(randombetween(0,100) > TW_SECOND_SEE_PERCENTAGE)
                {
                    goto nextnode;
                }
            }
        }
        if(bitTest(ship->flags,SOF_Disabled|SOF_Hide))
        {
            //don't attack disabled ships.
            goto nextnode;
        }
        vecSub(diff,ship->posinfo.position,thisship->posinfo.position);

        if (isBetweenExclusive(diff.x,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.y,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.z,negRetaliateZone,retaliateZone) )
        {
            select->ShipPtr[shipIndex++] = ship;
        }

nextnode:
        objindex++;
    }

    if (shipIndex == 0)
    {
        memFree(select);
        return NULL;
    }

    select->numShips = shipIndex;
    return select;
}

void univCheckShipState(Ship *ship)
{
    SelectCommand selectone;
    AttackCommand *attack;
    CommandToDo *command;

    if (ship->deathtime != 0.0f)
    {
        if ((ship->flags & SOF_Dead) == 0)
        {
            if (universe.totaltimeelapsed >= ship->deathtime)
            {
                // Self-destruct ship
                if(!(ship->flags & SOF_Disabled))
                {
                    //if ship is disabled..it didn't receive the
                    //self destruct order
                    ship->health = 0;
                    AddShipToDeleteShipList(ship,-1);
                }
                else if(ship->shiptype == GravWellGenerator)
                {
                    if( ((GravWellGeneratorSpec *)ship->ShipSpecifics)->GravDerelict)
                    {
                        //if ship is disabled..it didn't receive the
                        //self destruct order
                        ship->health = 0;
                        AddShipToDeleteShipList(ship,-1);
                    }
                }
            }
        }
    }

    if (ship->flags & SOF_Dead)
    {
        return;
    }

    if (ship->specialFlags & SPECIAL_rowGettingOutOfWay)
    {
        rowFlyShipOutOfWay(ship);
        //ship->autostabilizeship = FALSE;
    }

    if (ship->autostabilizeship && !bitTest(ship->flags, SOF_Disabled|SOF_Hide|SOF_Hyperspace|SOF_NISShip|SOF_Crazy) && !bitTest(ship->attributes,ATTRIBUTES_DontApplyFriction) && (ship->shiptype != Drone))
    {
        bool isShipSteady;

        if (ship->shiptype == MiningBase)
            isShipSteady = aitrackIsShipSteadyAnywhere(ship);       // don't care about orientation up
        else
            isShipSteady = aitrackIsShipSteady(ship);

        if (isShipSteady)
        {
            aitrackForceSteadyShip(ship);
            ship->autostabilizestate = 0;
        }
        else
        {
            switch (ship->autostabilizestate)
            {
                case 0:
                    // we just detected ship isn't stable
                    ship->autostabilizewait = AUTOSTABILIZE_FRAME_KICK_IN;
                    ship->autostabilizestate = 1;

                case 1:
                    if (ship->autostabilizewait > 0)
                    {
                        ship->autostabilizewait--;
                        break;
                    }
                    else
                    {
                        ship->autostabilizestate = 2;
                    }

                case 2:
                    if (ship->shiptype == MiningBase)
                        aitrackSteadyShipAnywhere(ship);
                    else
                        aitrackSteadyShip(ship);
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
    }

#ifndef HW_Release
    if (!shipsRetaliate)
    {
        return;
    }
#endif

    if (nisIsRunning)
    {
        return;      // don't autoretaliate if NIS running
    }

    if(!bitTest(ship->flags, SOF_Disabled))
    {
        tacticsUpdate(ship);    //perform tactical processes for ship
        if ((universe.univUpdateCounter & CHECK_SHIP_OUTOFWORLD_RATE) == (ship->shipID.shipNumber & CHECK_SHIP_OUTOFWORLD_RATE))
        {
            if (!(singlePlayerGame && (ship->playerowner->playerIndex == 1)))        // don't apply to KAS ships in single player game
            {
                if (univObjectOutsideWorld((SpaceObj *)ship))
                {
                    vector newdest;
                    SelectCommand *selectmove = getShipAndItsFormation(&universe.mainCommandLayer,ship);
                    CommandToDo *commandofship = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                    dbgAssert(selectmove);

                    if(commandofship != NULL)
                    {
                        if(commandofship->ordertype.order == COMMAND_MP_HYPERSPACEING)
                        {
                            //if ship is hyperspaceing, don't check if its in the world...once it is done, then we can move it!
                            goto DONT_MOVE_ME_YET;
                        }

                        if(commandofship->ordertype.order == COMMAND_ATTACK)
                        {
                            // let's see if we can find a target that is inside the world and attack that:
                            SpaceObjRotImpTarg *currenttarget = ship->attackvars.attacktarget;
                            SpaceObjRotImpTarg *testtarget;
                            AttackCommand *attack;
                            sdword i;

                            if (currenttarget)
                            {
                                if (!univObjectOutsideWorld((SpaceObj *)currenttarget))
                                {
                                    // our target is still in world, so we don't have to move into world
                                    goto DONT_MOVE_ME_YET;
                                }
                            }

                            attack = commandofship->attack;
                            for (i=0;i<attack->numTargets;i++)
                            {
                                testtarget = attack->TargetPtr[i];
                                if (testtarget == currenttarget)
                                {
                                    continue;
                                }

                                if (!univObjectOutsideWorld((SpaceObj *)testtarget))
                                {
                                    ship->attackvars.attacktarget = testtarget;
                                    goto DONT_MOVE_ME_YET;
                                }
                            }
                        }
                    }
                    newdest = ship->posinfo.position;
                    vecMultiplyByScalar(newdest,FLY_INTO_WORLD_PERCENT_DIST);
                    clMove(&universe.mainCommandLayer,selectmove,ship->posinfo.position,newdest);
                    memFree(selectmove);
                    return;
                }
            }
        }
    }
DONT_MOVE_ME_YET:
    if (ship->shipidle)
    {
        if(!bitTest(ship->flags, SOF_Disabled))
        {
            if (ship->shiptype == ResourceCollector)
            {
                if ((ship->gettingrocked != NULL) && (ship->tacticstype != Aggressive))
                {                                                   // tell ResourceCollector to run away
                    selectone.numShips = 1;
                    selectone.ShipPtr[0] = ship;

                    clDock(&universe.mainCommandLayer,&selectone,DOCK_TO_DEPOSIT_RESOURCES | DOCK_FOR_REFUELREPAIR,NULL);
                }
            }
            else
            {
                if ((ship->gettingrocked != NULL) && (!allianceIsShipAlly(ship,ship->gettingrocked->playerowner)))
                {
                    // attack ship->gettingrocked and its formation
                    attack = (AttackCommand *)getShipAndItsFormation(&universe.mainCommandLayer,ship->gettingrocked);
                    dbgAssert(attack->numTargets > 0);

                    command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                    if (command != NULL)
                    {
                        if (AreAnyShipsPassiveAttackCapable(command->selection))
                        {
#ifdef DEBUG_TACTICS
                            if(tacticsOn)
#endif
                            {
                                tacticsDelegateAttackCommand(ship,command,attack,TRUE,FALSE);
                            }
#ifdef DEBUG_TACTICS
                            else
                            {
                                if (canChangeOrderToPassiveAttack(command,attack))
                                {
                                    ChangeOrderToPassiveAttack(command,attack);
                                }
                            }
#endif
                        }
                    }
                    else
                    {
                        if (isShipPassiveAttackCapable(ship))
                        {
                            selectone.numShips = 1;
                            selectone.ShipPtr[0] = ship;
#ifdef DEBUG_TACTICS
                            if(tacticsOn)
#endif
                            {
                                tacticsDelegateSingleAttack(ship,&universe.mainCommandLayer,&selectone,attack,TRUE);
                            }
#ifdef DEBUG_TACTICS
                            else
                            {
                                clPassiveAttack(&universe.mainCommandLayer,&selectone,attack);
                            }
#endif
                        }
                    }
                    memFree(attack);
                }
                else if (((universe.univUpdateCounter & CHECK_PASSIVE_ATTACK_RATE) == (ship->shipID.shipNumber & CHECK_PASSIVE_ATTACK_RATE)) && (ship->staticinfo->passiveRetaliateZone != 0.0f))
                {
                    if(bitTest(ship->flags,SOF_Cloaked) || bitTest(ship->flags,SOF_Cloaking))
                    {   //if a ship is cloaked or it is cloaking, don't passive attack because we
                        //want to act stealthy :)
                        return;
                    }

                    command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                    if (command != NULL)
                    {
                        if ((canChangeOrderToPassiveAttack(command,NULL)) && (AreAnyShipsPassiveAttackCapable(command->selection)))
                        {
                            attack = (AttackCommand *)getEnemiesWithinProximity(ship,ship->staticinfo->passiveRetaliateZone);
                            if (attack != NULL)
                            {
#ifdef DEBUG_TACTICS
                                if(tacticsOn)
#endif
                                {
                                    tacticsDelegateAttackCommand(ship,command,attack,FALSE,TRUE);
                                }
#ifdef DEBUG_TACTICS
                                else
                                {
                                    ChangeOrderToPassiveAttack(command,attack);
                                }
#endif

                                memFree(attack);
                            }
                        }
                    }
                    else
                    {
                        if (isShipPassiveAttackCapable(ship))
                        {
                            selectone.numShips = 1;
                            selectone.ShipPtr[0] = ship;

                            attack = (AttackCommand *)getEnemiesWithinProximity(ship,ship->staticinfo->passiveRetaliateZone);
                            if (attack != NULL)
                            {
                                //fix later by removing this and all other tacticsOn checks
#ifdef DEBUG_TACTICS
                                if(tacticsOn)
#endif
                                {
                                    tacticsDelegateSingleAttack(ship,&universe.mainCommandLayer,&selectone,attack,FALSE);
                                }
#ifdef DEBUG_TACTICS
                                else
                                {
                                    clPassiveAttack(&universe.mainCommandLayer,&selectone,attack);
                                }
#endif
                               memFree(attack);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        //ship isn't idle
        //so lets do a check every so often to see if
        //ship should attempt passive attacking
        if(!bitTest(ship->flags, SOF_Disabled))
        {
            if (((universe.univUpdateCounter & CHECK_PASSIVE_ATTACK_WHILEMOVING_RATE) == (ship->shipID.shipNumber & CHECK_PASSIVE_ATTACK_WHILEMOVING_RATE)) && (ship->staticinfo->passiveRetaliateZone != 0.0f) && (isCapitalShip(ship)))
            {

                command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                if (command != NULL)
                {
                    if(command->ordertype.order == COMMAND_MOVE
                    ||command->ordertype.order == COMMAND_DOCK)
                    {
                        if(bitTest(ship->flags,SOF_Cloaked) || bitTest(ship->flags,SOF_Cloaking))
                        {   //if a ship is cloaked or it is cloaking, don't passive attack because we
                            //want to act stealthy :)
                            return;
                        }
                        if ((canChangeOrderToPassiveAttack(command,NULL)) && (AreAnyShipsPassiveAttackCapable(command->selection)))
                        {
                            attack = (AttackCommand *)getEnemiesWithinProximity(ship,ship->staticinfo->passiveRetaliateZone);
                            if (attack != NULL)
                            {
#ifdef DEBUG_TACTICS
                                if(tacticsOn)
#endif
                                {
                                    tacticsDelegateAttackCommand(ship,command,attack,FALSE,TRUE);
                                }
#ifdef DEBUG_TACTICS
                                else
                                {
                                    ChangeOrderToPassiveAttack(command,attack);
                                }
#endif
                                memFree(attack);
                            }
                        }
                    }
                }
            }
        }
    }
}

#if 0           // no longer needed - univCheckShipState called in univUpdateAllObjPosVel
/*-----------------------------------------------------------------------------
    Name        : univCheckShipStates
    Description : checks if shipidle and gettingrocked, and if so, tells ship
                  to retaliate
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univCheckShipStates()
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
//    bool attackpassive;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        univCheckShipState(ship);

nextnode:
        objnode = objnode->next;
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : univupdateReset
    Description : resets univupdate
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univupdateReset()
{
    universe.phystimeelapsed = UNIVERSE_UPDATE_PERIOD;
    universe.totaltimeelapsed = 0.0f;
    universe.radius = 20000.0f;
    universe.univUpdateCounter = 0;
    universe.resourceNumber = 0;
    universe.derelictNumber = 0;
    universe.shipNumber = 0;
    universe.missileNumber = 0;
    univResetFastNetworkIDLookups();
    universe.quittime = 0.0f;
    universe.wintime = 0.0f;
    universe.aiplayerProcessing = FALSE;

    universe.CapitalShipCaptured = 0;
    universe.PlayerWhoWon = 0;

    universe.lasttimeadded = 0;
    universe.DerelictTech=FALSE;

    universe.crateTimer = 0.0f;
    universe.numCratesInWorld = 0;
    universe.bounties = FALSE;

    universe.collUpdateAllBlobs = FALSE;

    listInit(&universe.RenderList);
    listInit(&universe.MinorRenderList);
    universe.dontUpdateRenderList = FALSE;

    listInit(&universe.SpaceObjList);
    listInit(&universe.MinorSpaceObjList);
    listInit(&universe.effectList);
    listInit(&universe.ShipList);
    listInit(&universe.BulletList);
    listInit(&universe.ResourceList);
    listInit(&universe.DerelictList);
    listInit(&universe.ImpactableList);
    listInit(&universe.MissileList);
    listInit(&universe.MineFormationList);

    listInit(&universe.DeleteMissileList);
    listInit(&universe.DeleteResourceList);
    listInit(&universe.DeleteDerelictList);
    listInit(&universe.DeleteShipList);

    listInit(&universe.collBlobList);

    // tactics stuff
    listInit(&universe.RetreatList);
    listInit(&universe.AttackMemory);

    // regrow resources stuff
    listInit(&universe.ResourceVolumeList);

    growSelectClose(&universe.HousekeepShipList);
    growSelectInit(&universe.HousekeepShipList);
    growSelectClose(&ClampedShipList);
    growSelectInit(&ClampedShipList);

}

/*-----------------------------------------------------------------------------
    Name        : univupdateInit
    Description : intializes univupdate
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univupdateInit()
{
    univInitFastNetworkIDLookups();
    growSelectInit(&universe.HousekeepShipList);
    growSelectInit(&ClampedShipList);

    universe.bountySize = 0;
    univupdateReset();
    universe.star3dinfo = star3dInit(NUM_STARS, universe.radius - 1.0f, universe.radius);
    scriptSet(NULL,"Galaxy.script",GalaxyTweaks);
    universe.backgroundColor = backgroundColor;
}

/*-----------------------------------------------------------------------------
    Name        : univFreeEffectContents
    Description : frees the effect (for exiting the program)
    Inputs      : effect
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeEffectContents(Effect *effect)
{
    etgEffectDelete(effect);
}

/*-----------------------------------------------------------------------------
    Name        : univFreeDerelictContents
    Description : Frees the contents of a derelict without freeing the derelict itself.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeDerelictContents(Derelict *derelict)
{
    if (derelict->navLightInfo)
    {
        memFree(derelict->navLightInfo);
        derelict->navLightInfo = NULL;
    }
    if(derelict->salvageInfo != NULL)
    {
        memFree(derelict->salvageInfo);
        derelict->salvageInfo = NULL;
    }
    if(derelict->clampInfo != NULL)
    {
        memFree(derelict->clampInfo);
        derelict->clampInfo = NULL;
    }

}

/*-----------------------------------------------------------------------------
    Name        : univFreeAsteroidContents
    Description : frees the asteroid contents
    Inputs      : asteroid
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeAsteroidContents(Asteroid *asteroid)
{
    if (asteroid->flags & SOF_StaticInfoIsDynamic)
    {
        memFree(asteroid->staticinfo);
    }
}

void univFreeNebulaContents(Nebula *nebula)
{
    if (nebula->stub != NULL)
    {
        nebDeleteChunk((nebChunk*)nebula->stub);
        nebula->stub = NULL;
    }

    if (nebula->flags & SOF_StaticInfoIsDynamic)
    {
        memFree(nebula->staticinfo);
    }
}

void univFreeGasCloudContents(GasCloud *gascloud)
{
    if (gascloud->flags & SOF_StaticInfoIsDynamic)
    {
        memFree(gascloud->staticinfo);
    }
}

void univFreeDustCloudContents(DustCloud *dustcloud)
{
    if (dustcloud->stub != NULL)
    {
        cloudDeleteSystem((cloudSystem*)dustcloud->stub);
        dustcloud->stub = NULL;
    }

    if (dustcloud->flags & SOF_StaticInfoIsDynamic)
    {
        memFree(dustcloud->staticinfo);
    }
}

void univFreeResourceContents(Resource *resource)
{
    switch (resource->objtype)
    {
        case OBJ_AsteroidType:
            univFreeAsteroidContents((Asteroid *)resource);
            break;

        case OBJ_NebulaType:
            univFreeNebulaContents((Nebula *)resource);
            break;

        case OBJ_GasType:
            univFreeGasCloudContents((GasCloud *)resource);
            break;

        case OBJ_DustType:
            univFreeDustCloudContents((DustCloud *)resource);
            break;
        default:
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univFreeBulletContents
    Description : frees the bullet (for exiting program)
    Inputs      : bullet
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeBulletContents(Bullet *bullet)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : univFreeMissileContents
    Description : frees the missile (for exiting program)
    Inputs      : missile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeMissileContents(Missile *missile)
{
    if(missile->formationinfo != NULL)
    {
        listRemoveNode(&missile->formationLink);     //take out of formation info list
    }
    if(missile->trail != NULL)
    {
        mistrailDelete(missile->trail);
        missile->trail = NULL;
    }
    if(missile->clampInfo != NULL)
    {
        memFree(missile->clampInfo);
        missile->clampInfo = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveShipReferences
    Description : Calls functions to remove references to ships.  This will
                    always be called no matter how a ship dies.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveShipReferences(Ship *ship)
{
    ship->flags |= SOF_Dead;
    bitClear(ship->flags,SOF_Selectable);

    //hopefully, at this point, the ships howDidIDied variable is set and valid!
    gameStatsShipDied(ship);

    if (((ShipStaticInfo *)(ship->staticinfo))->canBuildShips)
    {
        cmRemoveFactory(ship);
    }

    if (ship->rceffect != NULL)
    {
        TurnHarvestEffectOff(ship);
    }

    if(!(gameIsEnding && ship->shiptype == ResearchShip))
    {
        if(((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipDied != NULL)
        {
            ((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipDied(ship);
        }
    }

    tacticsShipDied(ship);

    if (ship->shipsInsideMe)
    {
        DeleteShipsInsideMe(ship);
    }

    if (bitTest(ship->flags,SOF_Slaveable))
    {
        dockDealWithDeadSlaveable(ship);        //this probably won't be called ever
    }

    univRemoveAllTargetReferencesFromBullets((SpaceObjRotImpTarg *)ship);
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)ship);

    univRemoveAllShipReferencesFromSpecialShips(ship);

    //if sensors manager is running, delete it from any sm spheres
    if (smSensorsActive)
    {
        smObjectDied(ship);
    }

    bobObjectDied((SpaceObj *)ship,&universe.collBlobList);
    pingObjectDied((SpaceObj *)ship);

    // Update Ship Command Layer to reflect ship death
    clShipDied(&universe.mainCommandLayer,ship);

    // Update computer player structures to reflect ship death
    aiplayerShipDied(ship);

    // Tell single player game module ship died
    singlePlayerShipDied(ship);

    if(tutorial)
        tutPlayerShipDied(ship);

    // Tell the sound layer that the ship died
//    soundEventShipDied(ship);
//  06/23/99 SA - I changed the above line because I think it'll only be called when
//  a ship is retired, this was causing it to say "ship lost" when it was retired.
//  the soundEventShipRemove() doesn't say the ship died speech event
    soundEventShipRemove(ship);

    IDToPtrTableObjDied(&ShipIDToPtr,ship->shipID.shipNumber);

    growSelectRemoveShipBySettingNULL(&universe.HousekeepShipList,ship);

    // Remove ships that are destroyed from any hot key groups
    univRemoveShipFromHotkeyGroup(ship,FALSE);

    mrShipDied(ship);
}

/*-----------------------------------------------------------------------------
    Name        : univFreeShipContents
    Description : frees the ship (for exiting program)
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeShipContents(Ship *ship)
{
    udword i;

    univRemoveShipReferences(ship);

    if (ship->attackvars.multipleAttackTargets != NULL)
    {
        memFree(ship->attackvars.multipleAttackTargets);
        ship->attackvars.multipleAttackTargets = NULL;
    }

    if (ship->flightman != FLIGHTMAN_NULL)
    {
        flightmanClose(ship);
    }

    if (ship->shipSinglePlayerGameInfo)
    {
        memFree(ship->shipSinglePlayerGameInfo);
        ship->shipSinglePlayerGameInfo = NULL;
    }

    // Remove repair droids...
/*    if(ship->pRepairDroids)
    {
        memFree(ship->pRepairDroids);
        ship->pRepairDroids = NULL;
    }*/

    if (ship->navLightInfo)
    {
        memFree(ship->navLightInfo);
        ship->navLightInfo = NULL;
    }
    if(ship->salvageInfo != NULL)
    {
        memFree(ship->salvageInfo);
        ship->salvageInfo = NULL;
    }
    if(ship->clampInfo != NULL)
    {
        memFree(ship->clampInfo);
        ship->clampInfo = NULL;
    }
    //remove lightnings
    for (i = 0; i < 2; i++)
    {
        if (ship->lightning[i] != NULL)
        {
            cloudKillLightning((lightning*)ship->lightning[i]);
            ship->lightning[i] = NULL;
        }
    }

    dmgStopEffect(ship, DMG_All);
    dmgClearLights(ship);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (ship->trail[i] != NULL)
        {
            trailDelete(ship->trail[i]);
            ship->trail[i] = NULL;
        }
    }

    if (ship->gunInfo != NULL)
    {
        memFree(ship->gunInfo);
        ship->gunInfo = NULL;
    }

    if (ship->dockInfo != NULL)
    {
        memFree(ship->dockInfo);
        ship->dockInfo = NULL;
    }

    if(ship->slaveinfo != NULL)
    {
        dockDealWithDeadSlaveable(ship);
    }

    if (ship->shipsInsideMe)
    {
        FreeShipsInsideMe(ship);
    }

    if (((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipClose)
    {
        ((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipClose(ship);
    }
    univMeshBindingsDelete(ship);
    if (ship->flags & SOF_StaticInfoIsDynamic)
    {
        memFree(ship->staticinfo);
    }
}


/*-----------------------------------------------------------------------------
    Name        : univupdateCloseAllObjectsAndMissionSpheres
    Description : closes all objects in the universe and all mission spheres in the universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univupdateCloseAllObjectsAndMissionSpheres()
{
    Node *objnode = universe.SpaceObjList.head;
    SpaceObj *obj;
    sdword i;

    univCloseFastNetworkIDLookups();
    growSelectClose(&universe.HousekeepShipList);
    growSelectClose(&ClampedShipList);

    gameIsEnding = TRUE;    //set global flag to indicate to other functions that the game is ending

    singlePlayerGameCleanup();

    while (objnode != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(objnode);

        switch (obj->objtype)
        {
            case OBJ_ShipType:
                ((Ship *)obj)->howDidIDie = DEATH_BY_GAME_END;
                univFreeShipContents((Ship *)obj);
                break;

            case OBJ_BulletType:
                univFreeBulletContents((Bullet *)obj);
                break;

            case OBJ_AsteroidType:
                univFreeAsteroidContents((Asteroid *)obj);
                break;

            case OBJ_NebulaType:
                univFreeNebulaContents((Nebula *)obj);
                break;

            case OBJ_GasType:
                univFreeGasCloudContents((GasCloud *)obj);
                break;

            case OBJ_DustType:
                univFreeDustCloudContents((DustCloud *)obj);
                break;

            case OBJ_DerelictType:
                univFreeDerelictContents((Derelict *)obj);
                break;

            case OBJ_EffectType:
                univFreeEffectContents((Effect *)obj);
                break;

            case OBJ_MissileType:
                univFreeMissileContents((Missile *)obj);
                break;

            default:
                dbgAssert(FALSE);
                break;
        }
        univRemoveObjFromRenderList(obj);

        objnode = objnode->next;
    }

    objnode = universe.MinorSpaceObjList.head;
    while (objnode != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(objnode);

        switch (obj->objtype)
        {
            case OBJ_AsteroidType:
                univFreeAsteroidContents((Asteroid *)obj);
                break;

            default:
                dbgAssert(FALSE);
                break;
        }
        univRemoveObjFromMinorRenderList(obj);

        objnode = objnode->next;
    }

    listDeleteAll(&universe.SpaceObjList);
    listDeleteAll(&universe.MinorSpaceObjList);

    listInit(&universe.effectList);
    listInit(&universe.ShipList);
    listInit(&universe.BulletList);
    listInit(&universe.ResourceList);
    listInit(&universe.DerelictList);
    listInit(&universe.ImpactableList);
    listInit(&universe.MissileList);

    bobListDelete(&universe.collBlobList);

    listDeleteAll(&universe.MineFormationList);
    listDeleteAll(&universe.ResourceVolumeList);

    // deselect everything as well
    selSelected.numShips = 0;
    selSelecting.numTargets = 0;

    if (!singlePlayerGameLoadNewLevelFlag)
    {
        for (i=0;i<SEL_NumberHotKeyGroups;i++)      // clear hotkey groups if not loading a new level
        {
            selHotKeyGroup[i].numShips = 0;
        }

        growSelectClose(&singlePlayerGameInfo.ShipsToHyperspace);
    }
    gameIsEnding=FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : univupdateClose
    Description : closes a mission
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univupdateClose()
{
    univupdateCloseAllObjectsAndMissionSpheres();

    star3dClose(universe.star3dinfo);
    universe.star3dinfo = NULL;
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void setSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *object)
{
    StaticInfoHealthGuidanceShipDerelict *statinfo = (StaticInfoHealthGuidanceShipDerelict *)object->staticinfo;
    sdword sizeofSalvageInfo;

    switch(object->objtype)
    {
    case OBJ_ShipType:
        if(((Ship *)object)->staticinfo->salvageStaticInfo == NULL)
        {
            object->salvageInfo = NULL;
            return;
        }
        break;
    case OBJ_DerelictType:
        if(((Derelict *)object)->staticinfo->salvageStaticInfo == NULL)
        {
            object->salvageInfo = NULL;
            return;
        }
        break;
    default:
        return;
    }
    {
        SalvagePoint *cursalvagepoint;
        SalvageStaticPoint *cursalvagestaticpoint;
        sdword i;
        sizeofSalvageInfo = sizeofSalvageInfo(statinfo->salvageStaticInfo->numSalvagePoints-1);
        object->salvageInfo = memAlloc(sizeofSalvageInfo,"SalvageInfo",NonVolatile);
        object->salvageInfo->numSalvagePoints = statinfo->salvageStaticInfo->numSalvagePoints;
        //list free ones...account for heading and up ones!
        object->salvageInfo->numSalvagePointsFree = object->salvageInfo->numSalvagePoints;

        for(i=0,cursalvagepoint=&object->salvageInfo->salvagePoints[0],cursalvagestaticpoint=&statinfo->salvageStaticInfo->salvageStaticPoints[0];i<statinfo->salvageStaticInfo->numSalvagePoints;i++,cursalvagepoint++,cursalvagestaticpoint++)
        {
            cursalvagepoint->salvageStaticPoint = cursalvagestaticpoint;
            cursalvagepoint->busy = 0;
        }
        object->salvageInfo->numNeededForSalvage = statinfo->salvageStaticInfo->numNeededForSalvage;

    }
}


/*-----------------------------------------------------------------------------
    Name        : univRegrowAsteroids
    Description : regrows an asteroid
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRegrowAsteroids()
{
    ;       // complete later, regrowing asteroids properly
}

#if 0
/*-----------------------------------------------------------------------------
    Name        : ObjDistCompareCB
    Description : callback function to compare camera distances of obj1,obj2
    Inputs      : obj1,obj2
    Outputs     :
    Return      : returns TRUE if obj1 camera distance > obj2 camera distance
----------------------------------------------------------------------------*/
bool ObjDistCompareCB(SpaceObj *obj1,SpaceObj *obj2)
{
    return (obj1->collOptimizeDist > obj2->collOptimizeDist);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : univSpaceObjInRenderList
    Description : returns TRUE if the SpaceObj is in the render list
    Inputs      :
    Outputs     :
    Return      : returns TRUE if the SpaceObj is in the render list
----------------------------------------------------------------------------*/
bool univSpaceObjInRenderList(SpaceObj *spaceobj)
{
    if (spaceobj->renderlink.belongto != NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveObjFromRenderList
    Description : removes the spaceobj from the render list (if it is there)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveObjFromRenderList(SpaceObj *spaceobj)
{
    listVerify(&universe.RenderList);
    if (univSpaceObjInRenderList(spaceobj))
    {
        listRemoveNode(&spaceobj->renderlink);
    }
    listVerify(&universe.RenderList);
}

/*-----------------------------------------------------------------------------
    Name        : univAddObjToRenderListIf
    Description : adds newobj to renderlist if ifobj is in render list
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univAddObjToRenderListIf(SpaceObj *newobj,SpaceObj *ifobj)
{
    listVerify(&universe.RenderList);
    if (univSpaceObjInRenderList(ifobj))
    {
        if ((newobj->flags & SOF_Hide) == 0)        // don't add hidden objects
        {
            if (bitTest(newobj->flags, SOF_AlwaysSortFront))
            {
                univAddObjToRenderList(newobj);
                return;
            }
            newobj->cameraDistanceVector = ifobj->cameraDistanceVector;
            newobj->cameraDistanceSquared = ifobj->cameraDistanceSquared;

            listAddNodeAfter(&ifobj->renderlink,&newobj->renderlink,newobj);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univAddObjToRenderList
    Description : adds newobj to renderlist
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univAddObjToRenderList(SpaceObj *newobj)
{
    listVerify(&universe.RenderList);
    if ((newobj->flags & SOF_Hide) == 0)
    {
        vector eyeposition = universe.mainCameraCommand.actualcamera.eyeposition;
        Node *objnode;
        SpaceObj *obj;

        vecSub(newobj->cameraDistanceVector,eyeposition,newobj->posinfo.position);
        newobj->cameraDistanceSquared = vecMagnitudeSquared(newobj->cameraDistanceVector);

        // now find where to put object in list to maintain sort order

        // render list is sorted from back to front, so find first node which has cameraDistanceSquared less than ours

        if (bitTest(newobj->flags, SOF_AlwaysSortFront))
        {                                                   //if this object has to go in front
            objnode = universe.RenderList.tail;
            while (objnode != NULL)
            {
                obj = (SpaceObj *)listGetStructOfNode(objnode);

                if (obj->cameraDistanceSquared > newobj->cameraDistanceSquared)
                {
                    listAddNodeAfter(objnode, &newobj->renderlink,newobj);
                    listVerify(&universe.RenderList);
                    goto doneForward;
                }
                objnode = objnode->prev;
            }
            listAddNode(&universe.RenderList,&newobj->renderlink,newobj);
            listVerify(&universe.RenderList);
doneForward:;
        }
        else
        {
            objnode = universe.RenderList.head;
            while (objnode != NULL)
            {
                obj = (SpaceObj *)listGetStructOfNode(objnode);

                if (obj->cameraDistanceSquared < newobj->cameraDistanceSquared)
                {
                    listAddNodeBefore(objnode,&newobj->renderlink,newobj);
                    goto doneRegular;
                }

                objnode = objnode->next;
            }

            listAddNode(&universe.RenderList,&newobj->renderlink,newobj);
doneRegular:
            ;
        }
    }
}

void univUpdateMinorRenderList()
{
    Node *objnode = universe.MinorSpaceObjList.head;
    SpaceObj *obj;
    vector distvec;
    vector lookatposition;
    vector eyeposition;
    real32 distsqr;

    if (mrCamera == NULL)
    {
        return;     // don't update render list if no mrCamera
    }

    lookatposition = mrCamera->lookatpoint;
    eyeposition = mrCamera->eyeposition;

    listVerify(&universe.RenderList);
    listRemoveAll(&universe.MinorRenderList);

    while (objnode != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(objnode);

#ifdef gshaw
        dbgAssert((obj->objtype == OBJ_AsteroidType) && (((Asteroid *)obj)->asteroidtype == Asteroid0));
#endif

        vecSub(distvec,lookatposition,obj->posinfo.position);
        distsqr = vecMagnitudeSquared(distvec);

        if (distsqr < RENDER_VIEWABLE_DISTANCE_SQR)
        {
            if ((obj->flags & SOF_Hide) == 0)   // don't add hidden objects
            {
                vecSub(obj->cameraDistanceVector,eyeposition,obj->posinfo.position);
                obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

                listAddNode(&universe.MinorRenderList,&obj->renderlink,obj);
            }
        }

        objnode = objnode->next;
    }
    listVerify(&universe.RenderList);
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateRenderList
    Description : updates the render list
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateRenderList()
{
    Node *objnode = universe.SpaceObjList.head;
    Node *nextnode;
    SpaceObj *obj;
    vector distvec;
    vector lookatposition;
    vector eyeposition;
    real32 distsqr;
    static bool limited = FALSE;
    static real32 limits[NUM_CLASSES];
    LinkedList sortFrontList;

    listVerify(&universe.RenderList);
    listInit(&sortFrontList);

    if (mrCamera == NULL)
    {
        return;     // don't update render list if no mrCamera
    }

    if (!limited)
    {
        limits[CLASS_Mothership] = RENDER_LIMIT_MOTHERSHIP;
        limits[CLASS_HeavyCruiser] = RENDER_LIMIT_HEAVYCRUISER;
        limits[CLASS_Carrier] = RENDER_LIMIT_CARRIER;
        limits[CLASS_Destroyer] = RENDER_LIMIT_DESTROYER;
        limits[CLASS_Frigate] = RENDER_LIMIT_FRIGATE;
        limits[CLASS_Corvette] = RENDER_LIMIT_CORVETTE;
        limits[CLASS_Fighter] = RENDER_LIMIT_FIGHTER;
        limits[CLASS_Resource] = RENDER_LIMIT_RESOURCE;
        limits[CLASS_NonCombat] = RENDER_LIMIT_NONCOMBAT;
        limited = TRUE;
    }

    lookatposition = mrCamera->lookatpoint;
    eyeposition = mrCamera->eyeposition;

    listRemoveAll(&universe.RenderList);

    while (objnode != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(objnode);

#ifdef gshaw
#ifndef HW_Release
        if ((obj->objtype == OBJ_AsteroidType) && (((Asteroid *)obj)->asteroidtype == Asteroid0))
        {
            dbgAssert(FALSE);
        }
#endif
#endif

        vecSub(distvec,lookatposition,obj->posinfo.position);
        distsqr = vecMagnitudeSquared(distvec);

        if (obj->flags & SOF_ForceVisible)
        {                                                   //if object is to always be in render list
            vecSub(obj->cameraDistanceVector, eyeposition, obj->posinfo.position);
            obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

            listAddNode(&universe.RenderList, &obj->renderlink, obj);
        }
        else
        {                                                   //else check if object should be in render list
            if (obj->objtype == OBJ_ShipType)
            {
                ShipClass shipclass;
                real32 limit;

                shipclass = ((ShipStaticInfo*)obj->staticinfo)->shipclass;
                dbgAssert((udword)shipclass < NUM_CLASSES);

                if (((ShipStaticInfo*)obj->staticinfo)->renderlistLimitSqr != 0.0f)
                {
                    limit = ((ShipStaticInfo*)obj->staticinfo)->renderlistLimitSqr;
                }
                else
                {
                    limit = limits[(sdword)shipclass];
                }

                if ((distsqr < limit) && !(obj->flags & SOF_Hide))
                {
                    vecSub(obj->cameraDistanceVector, eyeposition, obj->posinfo.position);
                    obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

                    listAddNode(&universe.RenderList, &obj->renderlink, obj);
                }
            }
            else if (obj->objtype == OBJ_DerelictType)
            {
                real32 limit;

                if (((DerelictStaticInfo*)obj->staticinfo)->renderlistLimitSqr != 0.0f)
                {
                    limit = ((DerelictStaticInfo*)obj->staticinfo)->renderlistLimitSqr;
                }
                else
                {
                    limit = (obj->flags & SOF_BigObject) ? RENDER_MAXVIEWABLE_DISTANCE_SQR : RENDER_VIEWABLE_DISTANCE_SQR;
                }

                if (distsqr < limit)
                {
                    if ((obj->flags & SOF_Hide) == 0)
                    {
                        vecSub(obj->cameraDistanceVector, eyeposition, obj->posinfo.position);
                        obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

                        if (bitTest(obj->flags, SOF_AlwaysSortFront))
                        {
                            listAddNode(&sortFrontList, &obj->renderlink, obj);
                        }
                        else
                        {
                            listAddNode(&universe.RenderList, &obj->renderlink, obj);
                        }
                    }
                }
            }
            else
            {
                if (distsqr < RENDER_MAXVIEWABLE_DISTANCE_SQR)
                {
                    if ((distsqr < RENDER_VIEWABLE_DISTANCE_SQR) || (obj->flags & SOF_BigObject))
                    {
                        if ((obj->flags & SOF_Hide) == 0)   // don't add hidden objects
                        {
                            vecSub(obj->cameraDistanceVector,eyeposition,obj->posinfo.position);
                            obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

                            if (bitTest(obj->flags, SOF_AlwaysSortFront))
                            {
                                listAddNode(&sortFrontList, &obj->renderlink, obj);
                            }
                            else
                            {
                                listAddNode(&universe.RenderList,&obj->renderlink,obj);
                            }
                        }
                    }
                }
            }
        }

        objnode = objnode->next;
    }

    listVerify(&universe.RenderList);
    listMergeSort2(&universe.RenderList);

    listVerify(&universe.RenderList);
    if (sortFrontList.num > 0)
    {                                                       //if there are force-front-sort objects
        listMergeSort2(&sortFrontList);                     //sort them like regular rederlist objects
        objnode = sortFrontList.head;
        while (objnode != NULL)
        {
            obj = (SpaceObj *)listGetStructOfNode(objnode);
            nextnode = objnode->next;
            dbgAssert(&obj->renderlink == objnode);
            listAddNode(&universe.RenderList, &obj->renderlink, obj);
            objnode = nextnode;
        }
    }
    listVerify(&universe.RenderList);
}

/*-----------------------------------------------------------------------------
    Name        : univCheckRegrowResources
    Description : checks if any resources should be regrown
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univCheckRegrowResources(void)
{
    Node *node = universe.ResourceVolumeList.head;
    ResourceVolume *resvol;

    while (node != NULL)
    {
        resvol = (ResourceVolume *)listGetStructOfNode(node);

        if (resvol->actualnumber < resvol->number)
        {
            // we are short some resources, so there is a chance we might regrow them

            real32 shortperc = 1.0f - ( (real32) resvol->actualnumber / (real32) resvol->number );
            sdword regrowrate = resvol->attributesParam;

            if ((regrowrate < 0) || (regrowrate > REGROWRATE_Max))
            {
                regrowrate = 0;
            }

            shortperc *= REGROW_RATE_MODIFIER[regrowrate];

            if (frandom(1) < shortperc)
            {
                sdword tr;

                // let's regrow the resource ... or at least try 3 times (make sure it doesn't overlap ship)
                switch (resvol->resourceVolumeType)
                {
                    case ResourceSphereType:
                        for (tr=0;tr<3;tr++)
                        {
                            if (AddResourceToSphere(resvol,TRUE))
                            {
#ifdef HW_Debug
                                dbgMessage("\nRegrowing resource!");
#endif
                                resvol->actualnumber++;
                                break;
                            }
                        }
                        break;

                    case ResourceCylinderType:
                        for (tr=0;tr<3;tr++)
                        {
                            if (AddResourceToCylinder(resvol,TRUE))
                            {
#ifdef HW_Debug
                                dbgMessage("\nRegrowing resource!");
#endif
                                resvol->actualnumber++;
                                break;
                            }
                        }
                        break;

                    case ResourceRectangleType:
                        for (tr=0;tr<3;tr++)
                        {
                            if (AddResourceToRectangle(resvol,TRUE))
                            {
#ifdef HW_Debug
                                dbgMessage("\nRegrowing resource!");
#endif
                                resvol->actualnumber++;
                                break;
                            }
                        }
                        break;

                    default:
                        dbgAssert(FALSE);
                        break;
                }
            }
        }

        node = node->next;
    }
}




void wkSmoothShips(void);

/*-----------------------------------------------------------------------------
    Name        : univUpdate
    Description : Updates the mission sphere ships and objects
    Inputs      : phystimeelapsed, time since last time it was updated
    Outputs     :
    Return      : TRUE if done (game over)
----------------------------------------------------------------------------*/
bool univUpdate(real32 phystimeelapsed)
{
#ifdef _WIN32
#define TMP_SAVEDGAMES_PATH "SavedGames\\"
#else
#define TMP_SAVEDGAMES_PATH "SavedGames/"
#endif

    if ((autoSaveDebug) && ((universe.univUpdateCounter & 31) == 0))    // every 2s
    {
        char savegamename[200];
        static sdword savenumber = 0;

        if (multiPlayerGame)
        {
            sprintf(savegamename,TMP_SAVEDGAMES_PATH "AutoSave%f",universe.totaltimeelapsed);
            SaveGame(savegamename);
            clCommandMessage(strGetString(strSavedGame));
        }
        else
        {
            if ((universe.univUpdateCounter & 255) == 0)        // only save every 16s
            {
                if (((singlePlayerGame) && (singlePlayerGameInfo.hyperspaceState)) ||
                    (nisIsRunning))
                {
                    ; // don't save game
                }
                else
                {
                    sprintf(savegamename,TMP_SAVEDGAMES_PATH "AutoSaveDebug%d",savenumber);
                    savenumber = (savenumber+1) & 7;
                    SaveGame(savegamename);
                    clCommandMessage(strGetString(strSavedGame));
                }
            }
        }
    }

#undef TMP_SAVEDGAMES_PATH

#ifdef GOD_LIKE_SYNC_CHECKING
    if(multiPlayerGame)
    {
        if(syncDumpOn)
        {
            industrialStrengthSyncDebugging(universe.univUpdateCounter);
            netcheckIndustrialChecksum();
        }
    }
#endif

    universe.univUpdateCounter++;

#define firstTime (universe.univUpdateCounter == 1)

    universe.phystimeelapsed = phystimeelapsed;
    universe.totaltimeelapsed += phystimeelapsed;

    if (universe.quittime != 0.0f)
    {
        real32 usequittime = universe.quittime;

        if (universe.wintime != 0.0f)
        {
            if (universe.totaltimeelapsed >= universe.wintime)
            {
                DeclarePlayerWin();
                universe.wintime = 0.0f;
            }
        }

        if ((multiPlayerGame) && (IAmCaptain))
        {
            usequittime += 5.0f;    // give a extra 5s so we transmit some more sync pkts other people so they quit too...
        }

        if (universe.totaltimeelapsed >= usequittime)
        {
            // quit the game here
            spMainScreen();

#if (defined(Downloadable) || defined(OEM))
            if ((singlePlayerGame) && (!(utyCreditsSequence|utyPlugScreens)))
#else
            if ((singlePlayerGame) && (!(utyCreditsSequence)))
#endif
            {
                gameIsRunning = FALSE;
                mrReset();
                feScreenStart(ghMainRegion, "SP_Game_Over");
                return TRUE;
            }
            else
            {
                utyGameQuitToMain(NULL,NULL);
                return TRUE;
            }
        }
    }
    else if (multiPlayerGame)
    {
        if (queueNumberOverruns(ProcessSyncPktQ) > 0)
        {
            bigmessageDisplay(strGetString(strSyncPktQOverrun),1);
            universe.quittime = universe.totaltimeelapsed + WAIT_TIME_TO_QUIT;
        }
    }

    PTSLAB(0,"univupdate");
#ifdef OPTIMIZE_VERBOSE
    vecNormalizeCounter=0;
#endif
    if ((!singlePlayerGame) && ((universe.univUpdateCounter & regenerateRUrate) == 0))
    {
        sdword i;
        Player *player;

        for (i=0;i<universe.numPlayers;i++)
        {
            player = &universe.players[i];

            if ((player->playerState != PLAYER_DEAD) && (player->resourceUnits < regenerateRUsIfUnder))
            {
                sdword before = player->resourceUnits;
                player->resourceUnits += regenerateRUs;
                if (player->resourceUnits > regenerateRUsIfUnder)
                {
                    player->resourceUnits = regenerateRUsIfUnder;

                }
                universe.gameStats.playerStats[player->playerIndex].totalRegeneratedResourceUnits += (player->resourceUnits - before);
            }
        }
    }

    if(tutorialdone)
    {
        // quit the game here
        spMainScreen();
        utyGameQuitToMain(NULL,NULL);
        return TRUE;
    }

    PTSLAB(1,"collblobs");

    if (firstTime || ((universe.univUpdateCounter & REFRESH_COLLBLOB_RATE) == REFRESH_COLLBLOB_FRAME))
    {
        collUpdateCollBlobs();
    }
    else
    {
        if ((universe.univUpdateCounter & REFRESH_COLLBLOB_RATE) == REFRESH_COLLBLOB_BATTLEPING_FRAME)
        {
            pingBattlePingsCreate(&universe.collBlobList);
        }
        collUpdateObjsInCollBlobs();
    }

    PTEND(1);

    cloudSetFog();      //dust/gas cloud think
    nebSetFog();        //nebula think

    //construction manager deterministic think
    cmDeterministicBuildProcess();

    // crappy fucking construction manager, no longer a task
    cmBuildTaskFunction();

    if ((universe.univUpdateCounter & REFRESH_RESEARCH_RATE) == REFRESH_RESEARCH_FRAME)
    {
        rmUpdateResearch();
    }

    if (!singlePlayerGame)
    {
        if ( (bitTest(tpGameCreated.flag,MG_ResourceInjections)) &&
             ((universe.univUpdateCounter-universe.lasttimeadded) == tpGameCreated.resourceInjectionInterval) )
        {
            universe.lasttimeadded = universe.univUpdateCounter;
            resourceInjectionTask();
            //speech event to notify of this:
            if (universe.players[universe.curPlayerIndex].playerState != PLAYER_DEAD)
                speechEventFleet(STAT_F_ResourceInjectionRecieved,0,universe.curPlayerIndex);
        }

        if ( (bitTest(tpGameCreated.flag,MG_ResourceLumpSum)) &&
             (universe.univUpdateCounter == tpGameCreated.resourceLumpSumTime))
        {
            resourceLumpSum();
            if (universe.players[universe.curPlayerIndex].playerState != PLAYER_DEAD)
                speechEventFleet(STAT_F_ResourceInjectionRecieved,0,universe.curPlayerIndex);
        }

        if (universe.univUpdateCounter == universe.players[sigsPlayerIndex].AllianceProposalsTimeout)
        {
            universe.players[sigsPlayerIndex].AllianceProposals = 0;
        }
    }

#ifdef COLLISION_CHECK_STATS
    shipsavoidingstuff = 0;
    shipsavoidedwalks = 0;
    shipsavoidedchecks = 0;
#endif

    //univZeroForcesOnResources();      // no longer needed - force, torque cleared after physUpdateObjPosVel acts on it
    //univZeroForcesOnDerelicts();
    //univZeroForcesOnMissiles();
    if (firstTime)
    {
        univSetupShipsForControl();     // only do it first time, since its done at end of univUpdateAllPosVel
        gameStatsCalcShipCostTotals();      // calculate the total ship cost at game start for the game stats
    }
    //univSetupDerelictsForControl();


    if (wkTradeStuffActive)
    {
        wkTradeUpdate();
    }

    if (singlePlayerGame)
        singlePlayerGameUpdate();
    else if (bitTest(tpGameCreated.flag,MG_Hyperspace))
        UpdateMidLevelHyperspacingShips();

    PTSLAB(3,"collbulmis");
    collCheckAllBulletMissileCollisions();
    PTEND(3);

    PTSLAB(4,"compplayer");

    cratesUpdate();

    aiplayerUpdateAll();
    PTEND(4);

    PTSLAB(5,"cmdlayer");

    PTSLABLITTLE(2,"aiship");

    if (!nisUniversePause)
    {
        clProcess(&universe.mainCommandLayer);

        clPostProcess(&universe.mainCommandLayer);
    }

    PTENDLITTLE(2);

    //do global tactics updates
    tacticsGlobalUpdate();

#if 0                           // no longer needed - univCheckShipState called in univUpdateAllPosVel now
    if (!nisUniversePause)
    {
        univCheckShipStates();
    }
#endif

    univUpdateMineWallFormations();

    PTEND(5);

    PTSLAB(7,"collbump");
    collCheckAllBumpCollisions();
    PTEND(7);

    PTSLAB(6,"updateobjpos");

    //old school function replaced with new functions below
    //univUpdateAllPosVel();       // also does a univCheckShipStates
    univUpdateAllPosVelMinorObjs();
    univUpdateAllPosVelBullets();
    univUpdateAllPosVelMissiles();
    if (!nisUniversePause)
    {

        univUpdateAllPosVelDerelicts();
        univUpdateAllPosVelResources();
        univUpdateAllPosVelShips();     // also does a univCheckShipStates

    }
    else
    {
        univPausedUpdateAllPosVelShips();   //certain operations only for when universe is paused
    }
    univUpdateAllPosVelEffects();   // MUST do effects sometime after ships+bullets and possibly other things that they may be attached to

    PTEND(6);

    univDeleteDeadShips();
    univDeleteDeadResources();
    univDeleteDeadDerelicts();
    univDeleteDeadMissiles();

    if ((universe.univUpdateCounter & REGROW_RESOURCES_CHECK_RATE) == REGROW_RESOURCES_CHECK_FRAME)
    {
        univCheckRegrowResources();
    }

    PTEND(0);

    if (singlePlayerGameLoadNewLevelFlag)
    {
        singlePlayerLoadNewLevel();
        if (hrAbortLoadingGame)
        {
            utyGameQuitToMain(NULL,NULL);
            hrAbortLoadingGame = FALSE;
            return TRUE;
        }
    }

    if(!universe.bounties || (universe.univUpdateCounter & CHECK_BOUNTIES_RATE) == CHECK_BOUNTIES_FRAME)
    {
        universe.bounties = TRUE;
        calculatePlayerBounties();
    }
    return FALSE;
}

void univKillPlayer(sdword i,sdword playerdeathtype)
{
    char filename[50];

    if (gatherStats|showStatsFight|showStatsFancyFight)
        return;     // just gathering stats, don't want to do the player win thing

    if (universe.players[i].playerState == PLAYER_DEAD)
    {
        return;     // player already dead
    }

    if (i == MAX_MULTIPLAYER_PLAYERS)
    {
        return;     // special player for autoguns, just exit
    }

    universe.players[i].PlayerMothership = NULL;
    universe.players[i].playerState = PLAYER_DEAD;
    universe.players[i].sensorLevel = 2;
    universe.gameStats.playerStats[i].timeOfDeath = universe.totaltimeelapsed;
    universe.gameStats.playerStats[i].typeOfPlayerDeath = playerdeathtype;
    aiplayerPlayerDied(&universe.players[i]);
    if (i == universe.curPlayerIndex)
    {
        smGhostMode = TRUE;
    }

    /* This will only be played if its a Relic player that died */
    speechEventFleet(STAT_F_RelicPlayerDies, i, SOUND_EVENT_DEFAULT);

    // player i has lost the game
    if (!singlePlayerGame) DisplayPlayerHasBeenDefeatedMessage(i);
    DisableAllPlayersShips(i);
    if (universe.curPlayerIndex == i)
    {
        if (singlePlayerGame)
        {
            bigmessageDisplay(strGetString(strQuestIsLost),0);
        }
        else
        {
            if (playerdeathtype == PLAYERKILLED_DROPPEDOUT)
            {
                bigmessageDisplay(strGetString(strHaveBeenDroppedOut),0);
            }
            else
            {
                bigmessageDisplay(strGetString(strHaveBeenDefeated),0);
            }
        }
    }
    strcpy(filename,playerNames[i]);
    strcat(filename,strGetString(strStatsDiedStats));
    writeGameStatsToFile(filename);
}

