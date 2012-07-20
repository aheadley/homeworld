/*=============================================================================
    Name    : damage.c
    Purpose : things necessary for showing ship damage

    Created 7/30/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <string.h>
#include "glinc.h"
#include "SpaceObj.h"
#include "ETG.h"
#include "trails.h"
#include "UnivUpdate.h"
#include "Damage.h"
#include "StatScript.h"
#include "Debug.h"
#include "MEX.h"
#include "Memory.h"
#include "Randy.h"
#include "SinglePlayer.h"


#define DMG_WORLDSPACE 1


/*=============================================================================
    Data
=============================================================================*/

sdword maxEffects[DMG_NumberDamageTypes] = {1,2,1};

static real32 DMG_StrikeTrailLimit = 0.6f;
static real32 DMG_StrikeLightLimit = 0.4f;
static real32 DMG_TrailLimit = 0.6f;
static real32 DMG_LightLimit = 0.4f;
static real32 DMG_HeavyLimit = 0.2f;
static real32 DMG_DyingLimit = 0.075f;

scriptEntry DamageTweaks[] =
{
    makeEntry(DMG_StrikeTrailLimit, scriptSetReal32CB),
    makeEntry(DMG_StrikeLightLimit, scriptSetReal32CB),
    makeEntry(DMG_TrailLimit, scriptSetReal32CB),
    makeEntry(DMG_LightLimit, scriptSetReal32CB),
    makeEntry(DMG_HeavyLimit, scriptSetReal32CB),
    makeEntry(DMG_DyingLimit, scriptSetReal32CB),
    endEntry
};


/*=============================================================================
    Code
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : dmgStartup
    Description : startup the damage module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dmgStartup()
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : dmgShutdown
    Description : shutdown the damage module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dmgShutdown()
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : dmgCoordSysFromVector
    Description : Constructs a coordinate system from a normal vector
    Inputs      : m - resultant 3x3 matrix
                  axis - characteristic vector
    Outputs     : m - 3x3 matrix to fill
    Return      :
----------------------------------------------------------------------------*/
void dmgCoordSysFromVector(matrix* m, vector* axis)
{
    vector up, LOF, right;

    vecSet(up, 0.0f, 0.0f, 1.0f);
    mexVecToVec(&LOF, axis);
    vecCrossProduct(right, LOF, up);
    vecCrossProduct(up, LOF, right);
    vecNormalize(&LOF);
    vecNormalize(&right);
    vecNormalize(&up);

    matPutVectIntoMatrixCol1(up, *m);
    matPutVectIntoMatrixCol2(right, *m);
    matPutVectIntoMatrixCol3(LOF, *m);
}

/*-----------------------------------------------------------------------------
    Name        : dmgRandomDamageLocation
    Description : select a damage light randomly from the ship, convert it to a
                  coordinate system + position, and return it (in shipspace)
    Inputs      : ship - the Ship
                  location - where the position goes
                  coordsys - where the coordinate system goes
    Outputs     : location, coordsys
    Return      :
----------------------------------------------------------------------------*/
void dmgRandomDamageLocation(Ship* ship, vector* location, matrix* coordsys)
{
    vector* lightNormal;
    vector* lightPosition;
    sdword  index;
    real32 scale = ship->magnitudeSquared;

    if (ship->nDamageLights == 0)
    {
        //no damage lights, use centre of ship
        vecZeroVector(*location);
        *coordsys = IdentityMatrix;
        return;
    }

    //random index
    index = ranRandom(RAN_Damage) % ship->nDamageLights;

    //obtain light data
    lightNormal = &ship->damageLights[index].normal;
    lightPosition = &ship->damageLights[index].position;

    //store location
    //*location = *lightPosition;
    vecScalarMultiply(*location, *lightPosition, scale);

    //turn its heading into a coordinate system
    dmgCoordSysFromVector(coordsys, lightNormal);
    //coordsys->m11 *= scale;
    //coordsys->m22 *= scale;
    //coordsys->m33 *= scale;
}

/*-----------------------------------------------------------------------------
    Name        : dmgCoordSysFromChunk
    Description : extract a coordinate system + offset from a light
    Inputs      : ship - the Ship
                  location, coordsys - resultant coordinate system goes here
                  type - eg. "Eng"
                  name - eg. "Eng0"
    Outputs     : location, coordsys
    Return      :
----------------------------------------------------------------------------*/
void dmgCoordSysFromChunk(Ship* ship, vector* location, matrix* coordsys, char* type, char* name)
{
    ShipStaticInfo* shipStatic;
    MEXChunk* mexChunk;
    vector* LOFChunk;
    vector* posChunk;
    real32 scale = ship->magnitudeSquared;

    shipStatic = ship->staticinfo;
    mexChunk = mexGetChunk(shipStatic->staticheader.pMexData, type, name);

    if (mexChunk == NULL)
    {
        vecZeroVector(*location);
        *coordsys = IdentityMatrix;
        return;
    }

    LOFChunk = &((MEXEngineChunk*)mexChunk)->normal;
    posChunk = &((MEXEngineChunk*)mexChunk)->position;

    //*location = *posChunk;
    vecScalarMultiply(*location, *posChunk, scale);
    dmgCoordSysFromVector(coordsys, LOFChunk);
    //coordsys->m11 *= scale;
    //coordsys->m22 *= scale;
    //coordsys->m33 *= scale;
}

/*-----------------------------------------------------------------------------
    Name        : dmgNumEffects
    Description : returns the number of effects currently playing at given level of damage
    Inputs      : ship - the Ship
                  level - level of damage
    Outputs     :
    Return      : the number of effects
----------------------------------------------------------------------------*/
sdword dmgNumEffects(Ship* ship, sdword level)
{
    sdword i, count;

    for (i = count = 0; i < maxEffects[level]; i++)
    {
        if (ship->showingDamage[level][i] != NULL)
        {
            count++;
        }
    }

    return count;
}

/*-----------------------------------------------------------------------------
    Name        : dmgAddEffectToShip
    Description : try to add an damage effect to a ship
    Inputs      : ship - the Ship
                  effect - the Effect
                  level - level of damage
    Outputs     : ship->showingDamage[level][?] contains the effect, or a fatal error is raised
    Return      :
----------------------------------------------------------------------------*/
void dmgAddEffectToShip(Ship* ship, Effect* effect, sdword level)
{
    sdword i;
    udword uship;

    for (i = 0; i < maxEffects[level]; i++)
    {
        if (ship->showingDamage[level][i] == NULL)
        {
#ifdef khent
#if DMG_VERBOSE >= 1
            dbgMessagef("\n-- damage effect level %d --", level);
#endif //DMG_VERBOSE
#endif //khent
            ship->showingDamage[level][i] = effect;
            return;
        }
    }

    uship = TreatAsUdword(ship);
    dbgFatalf(DBG_Loc, "too many damage effects at level %d, ship %u", level, uship);
}

/*-----------------------------------------------------------------------------
    Name        : dmgPlayEffect
    Description : try to play an effect to show ship damage
    Inputs      : ship - the Ship
                  effect - the Effect (type etglod) to play
                  level - level of damage
    Outputs     : the effect may begin playing if the maximum number of effects
                  are not already playing
    Return      :
----------------------------------------------------------------------------*/
void dmgPlayEffect(Ship* ship, etglod* effect, sdword level)
{
    etgeffectstatic* stat;
    sdword LOD;
    real32 floatDamage, floatSize;
    udword intDamage, intSize;
    matrix coordsys;
    vector location;

    //ASSERT: ship in renderlist

    if (effect == NULL)
    {
        //no effect to play
        return;
    }

    if (dmgNumEffects(ship, level) >= maxEffects[level])
    {
        //already playing the maximum number of effects
        return;
    }

    LOD = ship->currentLOD;
    if (LOD < effect->nLevels)
    {
        stat = effect->level[LOD];
    }
    else
    {
        //no effect to play, so simply return
        return;
    }

    //obtain location & coordsys for the effect from a random light
    switch (level)
    {
    case DMG_Light:
        //engine
        dmgCoordSysFromChunk(ship, &location, &coordsys, "Eng", "Eng0");
        break;

    case DMG_Heavy:
        //random "dmg" light
        dmgRandomDamageLocation(ship, &location, &coordsys);
        break;

    case DMG_Dying:
        //centre
        vecZeroVector(location);
        coordsys = IdentityMatrix;
        break;

    default:
        dbgFatalf(DBG_Loc, "\ndmgPlayEffect level %d unrecognized", level);
    }

    //obtain damage (health ratio)
    floatDamage = ship->health * ship->staticinfo->oneOverMaxHealth;
    intDamage = TreatAsUdword(floatDamage);
    floatSize = ship->staticinfo->staticheader.staticCollInfo.collspheresize;
    if (RGLtype == SWtype)
    {                                                       //smaller damage effects in software
        floatSize *= etgSoftwareScalarDamage;
    }
    intSize = TreatAsUdword(floatSize);

#if ETG_DISABLEABLE
    if (etgEffectsEnabled && etgDamageEffectsEnabled)
#endif
    {
#if DMG_WORLDSPACE
        Effect* effect;
        matrix  finalCoordSys;
        vector  finalLocation;
        hmatrix hmatA, hmatB, hmatC;

        hmatMakeHMatFromMatAndVec(&hmatB, &coordsys, &location);
        hmatMakeHMatFromMatAndVec(&hmatA, &ship->rotinfo.coordsys, &ship->posinfo.position);
        hmatMultiplyHMatByHMat(&hmatC, &hmatA, &hmatB);
        matGetMatFromHMat(&finalCoordSys, &hmatC);
        hmatGetVectFromHMatrixCol4(finalLocation, hmatC);

        effect = etgEffectCreate(stat, ship,
                                 &finalLocation,
                                 &ship->posinfo.velocity,
                                 &finalCoordSys,
                                 ship->magnitudeSquared, EAF_NLips | EAF_Velocity,
                                 2, intDamage, intSize);
#else
        Effect* effect = etgEffectCreate(stat, ship, &location, &ship->posinfo.velocity,
                                         &coordsys, ship->magnitudeSquared, EAF_NLips | EAF_Velocity | EAF_OwnerCoordSys,
                                         2, intDamage, intSize);
#endif
        dmgAddEffectToShip(ship, effect, level);
    }
}

/*-----------------------------------------------------------------------------
    Name        : dmgWipeOutEffect
    Description : delete an effect completely (?)
    Inputs      : effect - the Effect
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dmgWipeOutEffect(Effect* effect)
{
    etgEffectDelete(effect);
    univDeleteEffect(effect);
}

/*-----------------------------------------------------------------------------
    Name        : dmgForgetEffects
    Description : set all damage effects in a ship to NULL.  NOT TO BE USED TO
                  FREE DAMAGE EFFECTS in a general sense!
    Inputs      : ship - the ship to free effects from
    Outputs     : ship->showingDamage effects are forgotten
    Return      :
----------------------------------------------------------------------------*/
void dmgForgetEffects(Ship* ship)
{
    sdword level, i;

    for (level = DMG_Light; level < DMG_All; level++)
    {
        for (i = 0; i < maxEffects[level]; i++)
        {
            ship->showingDamage[level][i] = NULL;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : dmgStopEffect
    Description : stop all effects playing at a particular level of damage
    Inputs      : ship - the Ship
                  level - DMG_All clears all ship damage effects.  DMG_Light,
                  DMG_Heavy, or DMG_Dying clear only that level of damage
    Outputs     : ship->showingDamage[level]* effects are deleted and such
    Return      :
----------------------------------------------------------------------------*/
void dmgStopEffect(Ship* ship, sdword level)
{
    sdword i;

    if (level == DMG_All)
    {
        trailMakeWobbly(ship, FALSE);

        for (level = DMG_Light; level < DMG_All; level++)
        {
            for (i = 0; i < maxEffects[level]; i++)
            {
                if (ship->showingDamage[level][i] != NULL)
                {
                    dmgWipeOutEffect(ship->showingDamage[level][i]);
                    ship->showingDamage[level][i] = NULL;
                }
            }
        }
    }
    else
    {
        for (i = 0; i < maxEffects[level]; i++)
        {
            if (ship->showingDamage[level][i] != NULL)
            {
                dmgWipeOutEffect(ship->showingDamage[level][i]);
                ship->showingDamage[level][i] = NULL;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : dmgStopSingleEffect
    Description : search the effect's owner's damage effect list and remove the provided
                  effect if it belongs to the owner
    Inputs      : effect - the Effect
    Outputs     : effect is cleared from the ship's damage effect list, but
                  IS NOT DELETED (a flag is set for the etg to do so in etgDamageDone).
                  DO NOT call this fn directly
    Return      :
----------------------------------------------------------------------------*/
void dmgStopSingleEffect(Effect* effect)
{
    sdword level, j;
    Ship* ship;

    dbgAssert(effect->owner != NULL);

    ship = (Ship*)effect->owner;

    for (level = DMG_Light; level < DMG_All; level++)
    {
        for (j = 0; j < maxEffects[level]; j++)
        {
            if (ship->showingDamage[level][j] == effect)
            {
                ship->showingDamage[level][j] = NULL;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : dmgShipThink
    Description : think fn for showing ship damage.  determines whether damage
                  should be shown
    Inputs      : ship - the Ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dmgShipThink(Ship* ship)
{
    ShipStaticInfo* shipstaticinfo;
    ShipSinglePlayerGameInfo* ssinfo;
    ShipClass shipClass;
    real32 maxhealth, health, healthRatio;
    extern bool gShowDamage;

    if (!gShowDamage)
    {
        return;
    }

    if (!univSpaceObjInRenderList((SpaceObj*)ship))
    {
        return;
    }

    ssinfo = ship->shipSinglePlayerGameInfo;
    if (ssinfo != NULL && ssinfo->hsState != HS_INACTIVE)
    {
        return;
    }

    shipstaticinfo = ship->staticinfo;

    shipClass = shipstaticinfo->shipclass;

    maxhealth = shipstaticinfo->maxhealth;
    health = ship->health;
    healthRatio = health / maxhealth;

    switch (shipClass)
    {
    //strike craft
    case CLASS_Corvette:
    case CLASS_Fighter:
        if (healthRatio <= DMG_StrikeTrailLimit)
        {
            trailMakeWobbly(ship, TRUE);
        }
        else
        {
            trailMakeWobbly(ship, FALSE);
        }

        if (healthRatio <= DMG_StrikeLightLimit)
        {
            dmgPlayEffect(ship, etgDamageEffectTable[shipClass][DMG_Light], DMG_Light);
        }
        else
        {
            dmgStopEffect(ship, DMG_Light);
        }

        break;

    //everything else
    default:
        if (healthRatio <= DMG_TrailLimit)
        {
            trailMakeWobbly(ship, TRUE);
        }
        else
        {
            trailMakeWobbly(ship, FALSE);
        }

        if (healthRatio <= DMG_LightLimit)
        {
            dmgPlayEffect(ship, etgDamageEffectTable[shipClass][DMG_Light], DMG_Light);
        }
        else
        {
            dmgStopEffect(ship, DMG_Light);
        }

        if (healthRatio <= DMG_HeavyLimit)
        {
            dmgPlayEffect(ship, etgDamageEffectTable[shipClass][DMG_Heavy], DMG_Heavy);
        }
        else
        {
            dmgStopEffect(ship, DMG_Heavy);
        }

        if (healthRatio <= DMG_DyingLimit)
        {
            dmgPlayEffect(ship, etgDamageEffectTable[shipClass][DMG_Dying], DMG_Dying);
        }
        else
        {
            dmgStopEffect(ship, DMG_Dying);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : dmgGetLights
    Description : convert mex chunks containing damage lights into something more
                  easily (and speedily) traversable
    Inputs      : ship - the Ship
    Outputs     : ship->damageLights, etc
    Return      :
----------------------------------------------------------------------------*/
void dmgGetLights(Ship* ship)
{
    vector* lightNormal;
    vector* lightPosition;
    ShipStaticInfo* shipStatic;
    MEXChunk* mexChunk;
    char name[5] = "Dmg0";
    sdword i, numChunks;

    shipStatic = ship->staticinfo;

    //count Dmg chunks
    for (i = numChunks = 0; i < 50; i++)
    {
        name[3] = '0' + i;
        mexChunk = mexGetChunk(shipStatic->staticheader.pMexData, "Dmg", name);
        if (mexChunk == NULL)
        {
            break;
        }
        else
        {
            numChunks++;
        }
    }

    if (numChunks == 0)
    {
        //no chunks, do nothing
        return;
    }

    //allocate space for chunks
    ship->nDamageLights = numChunks;
    ship->damageLights = (DamageLightStatic*)memAlloc(numChunks*sizeof(DamageLightStatic), "dmgLightStatic", NonVolatile);
    dbgAssert(ship->damageLights != NULL);

    //assign light data
    for (i = 0; i < numChunks; i++)
    {
        name[3] = '0' + i;
        mexChunk = mexGetChunk(shipStatic->staticheader.pMexData, "Dmg", name);
        dbgAssert(mexChunk != NULL);    //shouldn't be NULL

        lightNormal = &((MEXEngineChunk*)mexChunk)->normal;
        lightPosition = &((MEXEngineChunk*)mexChunk)->position;

        memcpy(&ship->damageLights[i].position, lightPosition, sizeof(vector));
        memcpy(&ship->damageLights[i].normal, lightNormal, sizeof(vector));
    }
}

/*-----------------------------------------------------------------------------
    Name        : dmgClearLights
    Description : release the memory allocated to a ship's damage lights
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dmgClearLights(Ship* ship)
{
    if (ship->nDamageLights > 0)
    {
        dbgAssert(ship->damageLights != NULL);
        memFree(ship->damageLights);
        ship->nDamageLights = 0;
    }
    else
    {
        dbgAssert(ship->damageLights == NULL);
    }
}
