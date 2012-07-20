/*=============================================================================
    Name    : clamp.c
    Purpose : deals with latching one spaceobj to another

    Created 11/5/1997 by bpasechn
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "SpaceObj.h"
#include "Memory.h"
#include "Debug.h"
#include "Universe.h"
#include "Physics.h"
#include "UnivUpdate.h"
#include "Clamp.h"

#define DEBUG_CLAMPING

/*-----------------------------------------------------------------------------
    Name        :   clampObjToObj
    Description :   Attaches obj to dest in exact orientation obj is at with
                    respect to dest at call time (freezes position)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void clampObjToObj(SpaceObjRotImpTargGuidance *obj,SpaceObjRotImpTargGuidance *dest)
{
    matrix tmpmat;
    vector tmpvec;

    obj->clampInfo = memAlloc(sizeof(ClampInfo),"ClampingInformation",NonVolatile);

    obj->clampInfo->host = (SpaceObjRotImpTarg *)dest;

    vecSub(tmpvec,obj->posinfo.position,dest->posinfo.position);
    matMultiplyVecByMat(&obj->clampInfo->clampOffset,&tmpvec,&dest->rotinfo.coordsys);

    //Store slaves coordsys with respect to MASTER's object space
    tmpmat = obj->rotinfo.coordsys;
    matTranspose(&tmpmat);
    matMultiplyMatByMat(&obj->clampInfo->clampCoordsys,&tmpmat,&dest->rotinfo.coordsys);
    matTranspose(&obj->clampInfo->clampCoordsys);

    bitSet(obj->flags,SOF_Clamped);

    //later, allow for some looseness in clamp
    //(will allow for 1 univupdate frame of loosness as is
}

/*-----------------------------------------------------------------------------
    Name        :   unClampObj
    Description :   DeClamps a clamped object
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void unClampObj(SpaceObjRotImpTargGuidance *obj)
{
//#ifdef DEBUG_CLAMPING
//    dbgAssert(obj->clampInfo != NULL);
//#endif
    if (obj->clampInfo != NULL)
    {
        memFree(obj->clampInfo);
        obj->clampInfo = NULL;
        bitClear(obj->flags,SOF_Clamped);
    }
}

/*-----------------------------------------------------------------------------
    Name        :   updateClampedObject
    Description :   updates A clamped Objects positional status
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void updateClampedObject(SpaceObjRotImpTargGuidance *obj)
{
    vector tmpvec;

#ifdef DEBUG_CLAMPING
    dbgAssert(obj->clampInfo != NULL);
#endif

    //Update slave finally
    switch (obj->objtype)
    {
        case OBJ_DerelictType:  physUpdateObjPosVelDerelicts((Derelict *)obj,universe.phystimeelapsed);  break;
        case OBJ_ShipType:      physUpdateObjPosVelShip((Ship *)obj,universe.phystimeelapsed);  break;
        default:                physUpdateObjPosVelBasic((SpaceObj *)obj,universe.phystimeelapsed); break;
    }

    //'decode' clamped ship's location in world space with respect to targets
    matMultiplyMatByVec(&tmpvec,&obj->clampInfo->host->rotinfo.coordsys,&obj->clampInfo->clampOffset);
    vecAdd(obj->posinfo.position, obj->clampInfo->host->posinfo.position, tmpvec);

    //'decode' slaves orientation in world space with respect to Masters.
    matMultiplyMatByMat(&obj->rotinfo.coordsys,&obj->clampInfo->host->rotinfo.coordsys, &obj->clampInfo->clampCoordsys);
    obj->posinfo.velocity = obj->clampInfo->host->posinfo.velocity;

    univUpdateObjRotInfo((SpaceObjRot *)obj);
}
