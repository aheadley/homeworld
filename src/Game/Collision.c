/*=============================================================================
    Name    : Collision
    Purpose : Handles object collisions

    Created 9/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "FastMath.h"
#include "Vector.h"
#include "Matrix.h"
#include "SpaceObj.h"
#include "prim3d.h"
#include "render.h"
#include "Collision.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "Blobs.h"
#include "SalCapCorvette.h"
#include "DefenseFighter.h"
#include "DFGFrigate.h"
#include "NIS.h"
#include "Tactics.h"
#include "Alliance.h"
#include "SinglePlayer.h"

BlobProperties collBlobProperties;

#ifdef COLLISION_CHECK_STATS

udword shipsavoidingstuff;
udword shipsavoidedwalks;
udword shipsavoidedchecks;

udword shipshipwalks;
udword shipresourcewalks;
udword shipderelictwalks;
udword shipshipchecks;
udword shipresourcechecks;
udword shipderelictchecks;

udword bulletwalks;
udword bulletchecks;

udword missilewalks;
udword missilechecks;

udword minewalks;
udword minechecks;

#endif

/*-----------------------------------------------------------------------------
    Name        : RangeToTarget
    Description : Returns range from ship to target, taking into account collision
                  information of target.
    Inputs      : ship, target, trajectory (which is target->posinfo.position - ship->posinfo.position)
    Outputs     :
    Return      : range (in metres) to target
----------------------------------------------------------------------------*/
real32 RangeToTarget(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory)
{
    real32 dist;

    dist = fsqrt(vecMagnitudeSquared(*trajectory));
    dist -= target->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    if (dist > 0.0f)
    {
        return dist;
    }
    else
    {
        return 0.0f;
    }
}

/*-----------------------------------------------------------------------------
    Name        : RangeToTargetGivenDist
    Description : Returns range from ship to target, taking into account collision
                  information of target.
                  Takes the additional parameter dist if it has been pre-calculated
    Inputs      : ship, target, trajectory (which is target->posinfo.position - ship->posinfo.position), dist
    Outputs     :
    Return      : range (in metres) to target
----------------------------------------------------------------------------*/
real32 RangeToTargetGivenDist(Ship *ship,SpaceObjRotImpTarg *target,real32 dist)
{
    dist -= target->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    if (dist > 0.0f)
    {
        return dist;
    }
    else
    {
        return 0.0f;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collUpdateCollRectangle
    Description : updates collision rectangle information
    Inputs      : irobj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collUpdateCollRectangle(SpaceObjRotImp *irobj)
{
    StaticCollInfo *staticCollInfo = &irobj->staticinfo->staticheader.staticCollInfo;
    CollInfo *collInfo = &irobj->collInfo;
    vector upvector;
    vector rightvector;
    vector forwardvector;

    if (staticCollInfo->uplength == 0)
    {
        return;         // don't bother for all-zero rectangles
    }

    matMultiplyMatByVec(&collInfo->rectpos[0],&(irobj->rotinfo.coordsys),&staticCollInfo->collrectoffset);

    matGetVectFromMatrixCol1(upvector,irobj->rotinfo.coordsys);
    matGetVectFromMatrixCol2(rightvector,irobj->rotinfo.coordsys);
    matGetVectFromMatrixCol3(forwardvector,irobj->rotinfo.coordsys);

    vecMultiplyByScalar(upvector,staticCollInfo->uplength);
    vecMultiplyByScalar(rightvector,staticCollInfo->rightlength);
    vecMultiplyByScalar(forwardvector,staticCollInfo->forwardlength);

    vecAdd(collInfo->rectpos[1],collInfo->rectpos[0],rightvector);
    vecAdd(collInfo->rectpos[2],collInfo->rectpos[1],forwardvector);
    vecAdd(collInfo->rectpos[3],collInfo->rectpos[0],forwardvector);

    vecAdd(collInfo->rectpos[4],collInfo->rectpos[0],upvector);
    vecAdd(collInfo->rectpos[5],collInfo->rectpos[1],upvector);
    vecAdd(collInfo->rectpos[6],collInfo->rectpos[2],upvector);
    vecAdd(collInfo->rectpos[7],collInfo->rectpos[3],upvector);
}

#define LEFT    -1
#define MIDDLE  0
#define RIGHT   1

/*-----------------------------------------------------------------------------
    Name        : collCheckRectLine
    Description : checks for a collision between a rectangle and a line of length linelength
    Inputs      : obj1 (object to check collision with), univpoint (origin of line in universe coordinates),
                  univdir (unit vector direction of line in universe coordinates), linelength
    Outputs     : collSide
                  if hit on a side, up == 0 (TRANS_UP), down == 1, right == 2, left == 3, forward == 4, back == 5
                  otherwise collSide == -1
    Return      : -1.0 if line misses, 0.0 if line origin is inside rectangle, else +x if line collides where x is the distance
                  from the line origin to the collision point
----------------------------------------------------------------------------*/
real32 collCheckRectLine(SpaceObjRotImp *obj1,vector *univpoint,vector *univdir,real32 linelength,sdword *collSide)
{
    StaticCollInfo *staticCollInfo = &obj1->staticinfo->staticheader.staticCollInfo;
    vector pointInWorld;
    real32 minB[3];
    real32 maxB[3];
    real32 origin[3];
    real32 dir[3];
    sdword quadrant[3];
    real32 maxT[3];
    real32 candidatePlane[3];
    real32 coord;
    real32 largestMaxT;
    sdword i;
    sdword whichPlane;
    bool inside;

    *collSide = -1;

    if (staticCollInfo->uplength == 0)
    {
        return 0.0f;            // if no rectangle information present, assume collision
    }

    // axis aligned bounding box in object co-ordinates
    minB[0] = staticCollInfo->collrectoffset.x;
    minB[1] = staticCollInfo->collrectoffset.y;
    minB[2] = staticCollInfo->collrectoffset.z;
    maxB[0] = minB[0] + staticCollInfo->uplength;
    maxB[1] = minB[1] + staticCollInfo->rightlength;
    maxB[2] = minB[2] + staticCollInfo->forwardlength;

    // convert univpoint,univdir from universe to object co-ordinates

    vecSub(pointInWorld,*univpoint,obj1->posinfo.position);
    matMultiplyVecByMat((vector *)origin,&pointInWorld,&obj1->rotinfo.coordsys);

    // now we have minB, maxB, origin, dir in object co-ordinates
    // use a ray-box intersection algorithm as found in Graphics Gems I

    inside = TRUE;

    for (i=0;i<3;i++)
    {
        if (origin[i] < minB[i])
        {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = FALSE;
        }
        else if (origin[i] > maxB[i])
        {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = FALSE;
        }
        else
        {
            quadrant[i] = MIDDLE;
        }
    }

    if (inside)
    {
        return 0.0f;
    }

    if (vecDotProduct(pointInWorld,*univdir) > 0.0f)
    {
        return -1.0f;
    }

    matMultiplyVecByMat((vector *)dir,univdir,&obj1->rotinfo.coordsys);

    // calculate distances to candidate planes
    for (i=0;i<3;i++)
    {
        if ((quadrant[i] != MIDDLE) && (dir[i] != 0.0f))
        {
            maxT[i] = (candidatePlane[i]-origin[i]) / dir[i];
        }
        else
        {
            maxT[i] = -1.0f;
        }
    }

    // find largest maxT for final choice of intersection
    whichPlane = 0;
    for (i=1;i<3;i++)
    {
        if (maxT[i] > maxT[whichPlane])
        {
            whichPlane = i;
        }
    }

    largestMaxT = maxT[whichPlane];
    if (largestMaxT < 0.0f)
    {
        return -1.0f;
    }

    if (largestMaxT > linelength)
    {
        return -1.0f;
    }

    // Check final candidate if it is actually inside box
    for (i=0;i<3;i++)
    {
        if (whichPlane == i)
        {
            continue;
        }

        coord = origin[i] + largestMaxT * dir[i];

        if ((coord <= minB[i]) || (coord >= maxB[i]))
        {
            return -1.0f;
        }
    }

    // Set collSide baesd on where the plane intersected the box:
    // up == 0 (TRANS_UP), down == 1, right == 2, left == 3, forward == 4, back == 5
    *collSide = (whichPlane<<1) + ((quadrant[whichPlane] == LEFT) ? 1 : 0);
    dbgAssert((*collSide >= 0) && (*collSide < NUM_TRANS_DEGOFFREEDOM));
    return largestMaxT;
}

/*-----------------------------------------------------------------------------
    Name        : collCheckRectPoint
    Description : checks if collision between rectangle of obj1 and point (if point is inside obj1 rectangle)
    Inputs      : obj1, point
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool collCheckRectPoint(SpaceObjRotImp *obj1,vector *point)
{
    vector rect0;
    vector rect6;

    vector upvector;
    vector rightvector;
    vector forwardvector;

    vector rect0topoint;
    vector rect6topoint;

    if (obj1->staticinfo->staticheader.staticCollInfo.uplength == 0)
    {
        return TRUE;            // if no rectangle information present, assume collision
    }

    vecAdd(rect0,obj1->collInfo.rectpos[0],obj1->posinfo.position);
    vecAdd(rect6,obj1->collInfo.rectpos[6],obj1->posinfo.position);

    matGetVectFromMatrixCol1(upvector,obj1->rotinfo.coordsys);
    matGetVectFromMatrixCol2(rightvector,obj1->rotinfo.coordsys);
    matGetVectFromMatrixCol3(forwardvector,obj1->rotinfo.coordsys);

    vecSub(rect0topoint,*point,rect0);
    vecSub(rect6topoint,*point,rect6);

    // reject if point is forward of forward face
    if (vecDotProduct(forwardvector,rect6topoint) > 0.0f)
    {
        return FALSE;
    }

    // reject if point is behind behind face
    if (vecDotProduct(forwardvector,rect0topoint) < 0.0f)
    {
        return FALSE;
    }

    // reject if point is right of right face
    if (vecDotProduct(rightvector,rect6topoint) > 0.0f)
    {
        return FALSE;
    }

    // reject if point is left of left face
    if (vecDotProduct(rightvector,rect0topoint) < 0.0f)
    {
        return FALSE;
    }

    // reject if point is above up face
    if (vecDotProduct(upvector,rect6topoint) > 0.0f)
    {
        return FALSE;
    }

    // reject if point is below bottom face
    if (vecDotProduct(upvector,rect0topoint) < 0.0f)
    {
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : collCheckRectInRect
    Description : checks if rectangle obj1 is in obj2
    Inputs      : obj1, obj2
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool collCheckRectInRect(SpaceObjRotImp *obj1,SpaceObjRotImp *obj2)
{
    vector rect0;
    vector rect6;

    vector upvector;
    vector rightvector;
    vector forwardvector;

    vector rect0topoint;
    vector rect6topoint;

    vector obj2point;

    udword i;
    bool pointinside = FALSE;

    matGetVectFromMatrixCol1(upvector,obj1->rotinfo.coordsys);
    matGetVectFromMatrixCol2(rightvector,obj1->rotinfo.coordsys);
    matGetVectFromMatrixCol3(forwardvector,obj1->rotinfo.coordsys);

    vecAdd(rect0,obj1->collInfo.rectpos[0],obj1->posinfo.position);
    vecAdd(rect6,obj1->collInfo.rectpos[6],obj1->posinfo.position);

    for (i=0;i<8;i++)
    {
        vecAdd(obj2point,obj2->collInfo.rectpos[i],obj2->posinfo.position);

        vecSub(rect0topoint,obj2point,rect0);
        vecSub(rect6topoint,obj2point,rect6);

        // reject if point is forward of forward face
        if (vecDotProduct(forwardvector,rect6topoint) > (real32)1e-6)
        {
            continue;
        }

        // reject if point is behind behind face
        if (vecDotProduct(forwardvector,rect0topoint) < (real32)-1e-6)
        {
            continue;
        }

        // reject if point is right of right face
        if (vecDotProduct(rightvector,rect6topoint) > (real32)1e-6)
        {
            continue;
        }

        // reject if point is left of left face
        if (vecDotProduct(rightvector,rect0topoint) < (real32)-1e-6)
        {
            continue;
        }

        // reject if point is above up face
        if (vecDotProduct(upvector,rect6topoint) > (real32)1e-6)
        {
            continue;
        }

        // reject if point is below bottom face
        if (vecDotProduct(upvector,rect0topoint) < (real32)-1e-6)
        {
            continue;
        }

        pointinside = TRUE;
        break;
    }

    return pointinside;
}

/*-----------------------------------------------------------------------------
    Name        : collCheckRectBumpCollision
    Description : checks if two rectangles are colliding
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool collCheckRectBumpCollision(SpaceObjRotImp *obj1,SpaceObjRotImp *obj2)
{
    sdword collSide;
    vector sourcePosition;
    vector univdir;
    real32 linelength;
    real32 temp;

    if (obj1->staticinfo->staticheader.staticCollInfo.uplength == 0)
    {
        return TRUE;            // if no rectangle information present, assume collision
    }

    if (obj2->staticinfo->staticheader.staticCollInfo.uplength == 0)
    {
        return TRUE;            // if no rectangle information present, assume collision
    }

    if (collCheckRectInRect(obj1,obj2) || collCheckRectInRect(obj2,obj1))
    {
        return TRUE;
    }

    // now check case if both rectangles are colliding, but points are not inside each other
    vecAdd(sourcePosition,obj1->collInfo.rectpos[0],obj1->posinfo.position);

    vecSub(univdir,obj1->collInfo.rectpos[6],obj1->collInfo.rectpos[0]);

    linelength = obj1->staticinfo->staticheader.staticCollInfo.diagonallength;
    vecDivideByScalar(univdir,linelength,temp);

    if (collCheckRectLine(obj2,&sourcePosition,&univdir,linelength,&collSide) >= 0.0f)
    {
        return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : collZeroRectInfo
    Description : zero's out all rectangular information so it's not used
    Inputs      : irobj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collZeroRectInfo(StaticCollInfo *staticCollInfo)
{
    staticCollInfo->collrectoffset.x = 0.0f;
    staticCollInfo->collrectoffset.y = 0.0f;
    staticCollInfo->collrectoffset.z = 0.0f;

    staticCollInfo->uplength = 0.0f;
    staticCollInfo->rightlength = 0.0f;
    staticCollInfo->forwardlength = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : collDrawCollisionInfo
    Description : draws all collision info for debugging purposes
    Inputs      : irobj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collDrawCollisionInfo(SpaceObjRotImp *irobj)
{
    vector position;
    vector univrectpos[8];
    udword i;

    if (irobj->staticinfo->staticheader.staticCollInfo.uplength == 0)
    {
        return;         // don't bother for all-zero rectangles
    }

    position = irobj->posinfo.position;

    for (i=0;i<8;i++)
    {
        vecAdd(univrectpos[i],position,irobj->collInfo.rectpos[i]);
    }

    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    primLine3(&univrectpos[0],&univrectpos[1],colRGB(255,255,255));
    primLine3(&univrectpos[1],&univrectpos[2],colRGB(255,0,0));
    primLine3(&univrectpos[2],&univrectpos[3],colRGB(0,255,0));
    primLine3(&univrectpos[3],&univrectpos[0],colRGB(0,0,255));

    primLine3(&univrectpos[4],&univrectpos[5],colRGB(200,200,200));
    primLine3(&univrectpos[5],&univrectpos[6],colRGB(255,255,0));
    primLine3(&univrectpos[6],&univrectpos[7],colRGB(0,255,255));
    primLine3(&univrectpos[7],&univrectpos[4],colRGB(255,0,255));

    primLine3(&univrectpos[0],&univrectpos[4],colRGB(50,50,50));
    primLine3(&univrectpos[1],&univrectpos[5],colRGB(50,50,50));
    primLine3(&univrectpos[2],&univrectpos[6],colRGB(50,50,50));
    primLine3(&univrectpos[3],&univrectpos[7],colRGB(50,50,50));

    rndLightingEnable(TRUE);
}

#if 0
/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collAddSpaceObjToCollBlobs(SpaceObj *obj)
{
    bobAddObjToNearestBlob(&universe.collBlobList,obj);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : collUpdateCollBlobs
    Description : Updates collision blobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collUpdateCollBlobs(void)
{
    if (universe.collUpdateAllBlobs)
    {
        universe.collUpdateAllBlobs = FALSE;
        bobListDelete(&universe.collBlobList);
    }
    bobListCreate(&collBlobProperties, &universe.collBlobList, 0);
}


/*-----------------------------------------------------------------------------
    Name        : collUpdateObjsInCollBlobs
    Description : Updates objects in collision blobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collUpdateObjsInCollBlobs(void)
{
    Node *blobnode = universe.collBlobList.head;
    blob *thisBlob;

    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);

        bobUpdateObjsInBlobCollInfo(thisBlob);

        blobnode = blobnode->next;
    }
}

//function "fixes" a ships velocity and forces such that
//a collision looks good, as opposed to physically correct...

void kamikazeVelocityFix(SpaceObjRotImpTarg *obj1, SpaceObjRotImpTarg *obj2)
{

}

bool kamikazeCheck(SpaceObjRotImpTarg *obj1, SpaceObjRotImpTarg *obj2)
{
    real32 damage1,damage2;
    sdword ret = FALSE;
    sdword killer1type,killer1index;
    sdword killer2type,killer2index;
    Ship *ship1,*ship2;
    ship1 = (Ship *)obj1;
    ship2 = (Ship *)obj2;

    killer1type = killer2type = DEATH_Killed_By_Collision;
    killer1index = killer2index = 99;
    damage1=0.0f;
    damage2=0.0f;

    if(ship1->objtype == OBJ_ShipType)
    {
        if(bitTest(ship1->specialFlags,SPECIAL_Kamikaze))
        {
            //vector einsteinVel;
            //real32 shipvel,damFactor;
            //vecSub(einsteinVel,obj1->posinfo.velocity,obj2->posinfo.velocity);
            //shipvel = fsqrt(vecMagnitudeSquared(einsteinVel));
            //damFactor = shipvel/ship1->staticinfo->staticheader.maxvelocity;
            if(ship1->attackvars.attacktarget == obj2)
            {
                //ship was kamikazeeing and has collided with it's target
                if(obj2->objtype == OBJ_ShipType)
                {
                    damage1 = ((Ship *)obj2)->staticinfo->maxhealth;//*damFactor;
                }
                else
                {
                    damage1 = ((Ship *)obj1)->staticinfo->maxhealth+1;   //set very big so kamikaze ship dies
                }
                damage2 = kamikazeDamage[((Ship *)obj1)->shiptype];//*damFactor;
                if (ship2->staticinfo->shipclass == CLASS_Fighter || ship2->staticinfo->shipclass == CLASS_Corvette)
                {                                           //if target was a strike craft
                    damage2 = min(damage2, ship1->staticinfo->maxhealth);//make sure strike craft cannot destroy targets more powerful than themselves
                }
                killer1type = DEATH_Killed_By_Kamikaze;
                killer1index = ship1->playerowner->playerIndex;
                ret = TRUE;
            }
        }
    }

    //if ship 2 was kamikazing
    if(ship2->objtype == OBJ_ShipType)
    {
        if(bitTest(ship2->specialFlags,SPECIAL_Kamikaze))
        {
            //vector einsteinVel;
            //real32 shipvel,damFactor;
            //vecSub(einsteinVel,obj1->posinfo.velocity,obj2->posinfo.velocity);
            //shipvel = fsqrt(vecMagnitudeSquared(einsteinVel));
            //vecSub(einsteinVel,obj1->posinfo.velocity,obj2->posinfo.velocity);
            //shipvel = fsqrt(vecMagnitudeSquared(einsteinVel));
            //damFactor = shipvel/ship2->staticinfo->staticheader.maxvelocity;
            if(ship2->attackvars.attacktarget == obj1)
            {
                //ship was kamikazeeing and has collided with it's target
                if(obj1->objtype == OBJ_ShipType)
                {
                    damage2 += ((Ship *)obj1)->staticinfo->maxhealth;//*damFactor;
                }
                else
                {
                    damage2 += ((Ship *)obj2)->staticinfo->maxhealth+1;   //set very big so kamikaze ship dies
                }
                damage1 += kamikazeDamage[((Ship *)obj2)->shiptype];//*damFactor;
                if (ship1->staticinfo->shipclass == CLASS_Fighter || ship1->staticinfo->shipclass == CLASS_Corvette)
                {                                           //if target was a strike craft
                    damage1 = min(damage1, ship2->staticinfo->maxhealth);//make sure strike craft cannot destroy targets more powerful than themselves
                }

                killer2type = DEATH_Killed_By_Kamikaze;
                killer2index = ship2->playerowner->playerIndex;
                ret = TRUE;
            }
        }
    }
    if(!ret)
        return FALSE;

    ApplyDamageToTarget(obj1,damage1,0,killer2type,killer2index); //99 so as to register as invalid if it tries to get used (cause it shouldn't)
    ApplyDamageToTarget(obj2,damage2,0,killer1type,killer1index);
    return TRUE;
}
/*-----------------------------------------------------------------------------
    Name        : collCheckShipShipColl
    Description : checks collisions between ships and ships, and impacts a force
                  on each one (equal and opposite forces)
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckShipShipColl(blob *thisBlob,bool checkSmallShips)
{
    sdword obj1index = 0;
    Ship *obj1;

    sdword obj2index;
    Ship *obj2;

    real32 dist;
    real32 distsquared;
    real32 colldist;
    vector distvector;

    real32 obj1collspheresize;
    real32 maxdistCollPossible;

    real32 checkdist;

    SelectCommand *selection;
    sdword numShips;

    real32 maxShipCollSphereSize;

    if (checkSmallShips)
    {
        selection = thisBlob->blobSmallShips;
        maxShipCollSphereSize = thisBlob->blobMaxSmallShipCollSphereSize;
    }
    else
    {
        selection = thisBlob->blobBigShips;
        maxShipCollSphereSize = thisBlob->blobMaxBigShipCollSphereSize;
    }
    numShips = selection->numShips;

    while (obj1index < numShips)
    {
        obj1 = selection->ShipPtr[obj1index];
        obj1collspheresize = obj1->staticinfo->staticheader.staticCollInfo.collspheresize;
        maxdistCollPossible = obj1collspheresize + maxShipCollSphereSize;

        obj2index = obj1index + 1;
        while (obj2index < numShips)
        {
            obj2 = selection->ShipPtr[obj2index];

#ifdef COLLISION_CHECK_STATS
            shipshipwalks++;
#endif

            checkdist = obj2->collOptimizeDist - obj1->collOptimizeDist;

            if (checkdist > maxdistCollPossible)
            {
                goto nextobj1;
            }

            if ((obj1->posinfo.isMoving | obj2->posinfo.isMoving) == FALSE)     // assume objects not moving can't collide
            {
                goto nocollision;
            }

            colldist = obj1collspheresize + obj2->staticinfo->staticheader.staticCollInfo.collspheresize;

            if (checkdist > colldist)
            {
                goto nocollision;
            }

            //old docking rejection code

            if ((obj1->dockingship == obj2) || (obj2->dockingship == obj1))
            {
                goto nocollision;       // docking ships shouldn't collide
            }

            if (obj1->dockingship != NULL && obj2->dockingship != NULL)
            {   //special for slaved ships in DOCKING process...silly I know
                goto nocollision;
            }

            if(bitTest(obj1->flags, SOF_Slaveable) && bitTest(obj2->flags,SOF_Slaveable))
            {    //both objects are slaveable...see if commonly linked.
                if(obj1->slaveinfo->Master == obj2->slaveinfo->Master)
                {
                    goto nocollision;
                }
            }

            // Check for collisions between obj1 and obj2 here:

#ifdef COLLISION_CHECK_STATS
            shipshipchecks++;
#endif

            vecSub(distvector,obj2->collInfo.collPosition,obj1->collInfo.collPosition);

            if (!isBetweenInclusive(distvector.x,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.y,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.z,-colldist,colldist))
            {
                goto nocollision;
            }

            if(obj1->specialFlags & SPECIAL_IsASalvager)
            {
                if( ((SalCapCorvetteSpec *)obj1->ShipSpecifics)->target == (SpaceObjRotImpTargGuidanceShipDerelict *)obj2)
                {
                    if( ((SalCapCorvetteSpec *)obj1->ShipSpecifics)->noAvoid)
                    {
                        goto nocollision;
                    }
                }
            }
            if(obj2->specialFlags & SPECIAL_IsASalvager)
            {
                if( ((SalCapCorvetteSpec *)obj2->ShipSpecifics)->target == (SpaceObjRotImpTargGuidanceShipDerelict *)obj1)
                {
                    if( ((SalCapCorvetteSpec *)obj2->ShipSpecifics)->noAvoid)
                    {
                        goto nocollision;
                    }
                }
            }

            if ((obj1->flags|obj2->flags) & SOF_Hide)
            {
                goto nocollision;
            }

            if(obj1->clampInfo != NULL)
            {
                //fix later...maybe can assume obj ios a ship?
                //no a big speed concern...not A LOT of clamping going on
                if(obj1->clampInfo->host->objtype == OBJ_ShipType)
                {
                    if(((Ship *)obj1->clampInfo->host)->shiptype == SalCapCorvette)
                    {
                        if(((SalCapCorvetteSpec *)((Ship *)obj1->clampInfo->host)->ShipSpecifics)->dockwith == obj2)
                        {
                            //ship clamped to salcap is colliding with the ship the salcap is docking with,
                            //don't really collide
                            goto nocollision;
                        }
                    }
                }
                if(obj2->clampInfo != NULL)
                {
                    if(((Ship *)obj2->clampInfo->host)->dockingship != NULL)
                    {
                        if(((Ship *)obj2->clampInfo->host)->dockingship == obj1->dockingship)
                        {
                            //docking with same ships...
                            goto nocollision;
                        }
                    }
                    if(obj2->clampInfo->host == obj1->clampInfo->host)
                    {
                        //same host, so don't do collision detection
                        goto nocollision;
                    }
                }
                else if(((Ship *)obj1->clampInfo->host)->dockingship != NULL)
                {
                    if (((Ship *)obj1->clampInfo->host)->dockingship == obj2)
                    {
                        goto nocollision;
                    }
                    if (((Ship *)obj1->clampInfo->host)->dockingship == obj2->dockingship)
                    {
                        goto nocollision;
                    }
                }
            }
            if(obj2->clampInfo != NULL)
            {
                if(obj2->clampInfo->host->objtype == OBJ_ShipType)
                {
                    if(((Ship *)obj2->clampInfo->host)->shiptype == SalCapCorvette)
                    {
                        if(((SalCapCorvetteSpec *)((Ship *)obj2->clampInfo->host)->ShipSpecifics)->dockwith == obj1)
                        {
                            //ship clamped to salcap is colliding with the ship the salcap is docking with,
                            //don't really collide
                            goto nocollision;
                        }
                    }
                    if(obj1->clampInfo != NULL)
                    {
                        if(((Ship *)obj1->clampInfo->host)->dockingship != NULL)
                        {
                            if(((Ship *)obj1->clampInfo->host)->dockingship == obj2->dockingship)
                            {
                                //docking with same ships...
                                goto nocollision;
                            }
                        }
                    }
                    else if(((Ship *)obj2->clampInfo->host)->dockingship != NULL)
                    {
                        if (((Ship *)obj2->clampInfo->host)->dockingship == obj1)
                        {
                            goto nocollision;
                        }
                        if (((Ship *)obj2->clampInfo->host)->dockingship == obj1->dockingship)
                        {
                            goto nocollision;
                        }
                    }
                }
            }
            distsquared = vecMagnitudeSquared(distvector);
            dist = (real32)fsqrt(distsquared);

            if (dist < colldist)
            {
                if (collCheckRectBumpCollision((SpaceObjRotImp *)obj1,(SpaceObjRotImp *)obj2))
                {
                    if (distsquared < 1.0f)
                    {
                        distvector.x = 10.0f;   // special case, if objects are too close together
                        distsquared = 100.0f;
                        dist = 10.0f;
                    }

                    if (!bitTest(obj1->flags, SOF_NotBumpable) && !bitTest(obj2->flags, SOF_NotBumpable))
                    {
                        if(!kamikazeCheck((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2))    //return true if kamikaze collision is true
                        {
                            if (obj1->playerowner == obj2->playerowner)
                            {
                                goto dontapplydamage;      // don't apply bonk damage for own ships
                            }

                            if ((obj1->flags & SOF_Disabled) || (obj2->flags & SOF_Disabled))
                            {
                                goto dontapplydamage;
                            }

                            if(obj1->shiptype == SalCapCorvette || obj1->shiptype == JunkYardDawg)
                            {
                                SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)obj1->ShipSpecifics;
                                if(((SpaceObjRotImpTarg *)spec->noDamageTarget) == ((SpaceObjRotImpTarg *)obj2))
                                {
                                    goto dontapplydamage;
                                }
                            }
                            else if(obj2->shiptype == SalCapCorvette || obj2->shiptype == JunkYardDawg)
                            {
                                SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)obj2->ShipSpecifics;
                                if(((SpaceObjRotImpTarg *)spec->noDamageTarget) == ((SpaceObjRotImpTarg *)obj1))
                                {
                                    goto dontapplydamage;
                                }
                            }

                            ApplyDamageToCollidingObjects((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,&distvector,dist);
dontapplydamage:
                            ObjectsCollided((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist,distsquared);
                        }
                        else
                        {
                            //fix collided ships velocity....
                            ObjectsCollided((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist,distsquared);
                            kamikazeVelocityFix((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2);

                        }

                    }

                }
            }
nocollision:
            obj2index++;
        }
nextobj1:
        obj1index++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckBigShipSmallShipColl
    Description : checks collisions between bigships and smallships, and impacts a force
                  on each one (equal and opposite forces)
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckBigShipSmallShipColl(blob *thisBlob)
{
    sdword obj1index = 0;
    Ship *obj1;

    sdword obj2index;
    Ship *obj2;

    real32 dist;
    real32 distsquared;
    real32 colldist;
    vector distvector;

    real32 obj1collspheresize;
    real32 maxdistCollPossible;

    real32 checkdist;

    SelectCommand *selection1 = thisBlob->blobBigShips;
    sdword numShips1 = selection1->numShips;

    SelectCommand *selection2 = thisBlob->blobSmallShips;
    sdword numShips2 = selection2->numShips;

    real32 maxShipCollSphereSize = thisBlob->blobMaxSmallShipCollSphereSize;

    sdword bottom,top;

    if (numShips2 == 0)
    {
        return;
    }

    while (obj1index < numShips1)
    {
        obj1 = selection1->ShipPtr[obj1index];
        obj1collspheresize = obj1->staticinfo->staticheader.staticCollInfo.collspheresize;
        maxdistCollPossible = obj1collspheresize + maxShipCollSphereSize;

        // binary search targetindex such that
        // (obj1->collOptimizeDist - obj2->collOptimizeDist) <= maxdistCollPossible
        bottom = 0;
        top = numShips2-1;
        while (top >= bottom)
        {
            obj2index = (bottom+top)>>1;
            if ((obj1->collOptimizeDist - selection2->ShipPtr[obj2index]->collOptimizeDist) > maxdistCollPossible)
            {
                // targetindex is too small
                bottom = obj2index+1;
            }
            else
            {
                top = obj2index-1;
            }
        }

        dbgAssert(obj2index >= 0);
        dbgAssert(obj2index < numShips2);

        while (obj2index < numShips2)
        {
            obj2 = selection2->ShipPtr[obj2index];

#ifdef COLLISION_CHECK_STATS
            shipshipwalks++;
#endif

            checkdist = obj2->collOptimizeDist - obj1->collOptimizeDist;

            if (checkdist > maxdistCollPossible)
            {
                goto nextobj1;
            }

            if ((obj1->posinfo.isMoving | obj2->posinfo.isMoving) == FALSE)     // assume objects not moving can't collide
            {
                goto nocollision;
            }

            colldist = obj1collspheresize + obj2->staticinfo->staticheader.staticCollInfo.collspheresize;

            if (checkdist > colldist)
            {
                goto nocollision;
            }

            //old docking rejection code

            if ((obj1->dockingship == obj2) || (obj2->dockingship == obj1))
            {
                goto nocollision;       // docking ships shouldn't collide
            }

            if (obj1->dockingship != NULL && obj2->dockingship != NULL)
            {   //special for slaved ships in DOCKING process...silly I know
                goto nocollision;
            }

            if(bitTest(obj1->flags, SOF_Slaveable) && bitTest(obj2->flags,SOF_Slaveable))
            {    //both objects are slaveable...see if commonly linked.
                if(obj1->slaveinfo->Master == obj2->slaveinfo->Master)
                {
                    goto nocollision;
                }
            }

            // Check for collisions between obj1 and obj2 here:

#ifdef COLLISION_CHECK_STATS
            shipshipchecks++;
#endif

            vecSub(distvector,obj2->collInfo.collPosition,obj1->collInfo.collPosition);

            if (!isBetweenInclusive(distvector.x,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.y,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.z,-colldist,colldist))
            {
                goto nocollision;
            }

            if(obj1->specialFlags & SPECIAL_IsASalvager)
            {
                if( ((SalCapCorvetteSpec *)obj1->ShipSpecifics)->target == (SpaceObjRotImpTargGuidanceShipDerelict *)obj2)
                {
                    if( ((SalCapCorvetteSpec *)obj1->ShipSpecifics)->noAvoid)
                    {
                        goto nocollision;
                    }
                }
            }
            if(obj2->specialFlags & SPECIAL_IsASalvager)
            {
                if( ((SalCapCorvetteSpec *)obj2->ShipSpecifics)->target == (SpaceObjRotImpTargGuidanceShipDerelict *)obj1)
                {
                    if( ((SalCapCorvetteSpec *)obj2->ShipSpecifics)->noAvoid)
                    {
                        goto nocollision;
                    }
                }
            }

            if ((obj1->flags|obj2->flags) & SOF_Hide)
            {
                goto nocollision;
            }

            if(obj1->clampInfo != NULL)
            {
                if(obj1->clampInfo->host->objtype == OBJ_ShipType)
                {
                    if(((Ship *)obj1->clampInfo->host)->shiptype == SalCapCorvette)
                    {
                        if(((SalCapCorvetteSpec *)((Ship *)obj1->clampInfo->host)->ShipSpecifics)->dockwith == obj2)
                        {
                            //ship clamped to salcap is colliding with the ship the salcap is docking with,
                            //don't really collide
                            goto nocollision;
                        }
                    }
                }
                if(obj2->clampInfo != NULL)
                {
                    if(((Ship *)obj2->clampInfo->host)->dockingship != NULL)
                    {
                        if(((Ship *)obj2->clampInfo->host)->dockingship == obj1->dockingship)
                        {
                            //docking with same ships...
                            goto nocollision;
                        }
                    }
                    if(obj2->clampInfo->host == obj1->clampInfo->host)
                    {
                        //same host, so don't do collision detection
                        goto nocollision;
                    }
                }
                else if(((Ship *)obj1->clampInfo->host)->dockingship != NULL)
                {
                    if (((Ship *)obj1->clampInfo->host)->dockingship == obj2)
                    {
                        goto nocollision;
                    }
                    if (((Ship *)obj1->clampInfo->host)->dockingship == obj2->dockingship)
                    {
                        goto nocollision;
                    }
                }
            }
            if(obj2->clampInfo != NULL)
            {
                if(obj2->clampInfo->host->objtype == OBJ_ShipType)
                {
                    if(((Ship *)obj2->clampInfo->host)->shiptype == SalCapCorvette)
                    {
                        if(((SalCapCorvetteSpec *)((Ship *)obj2->clampInfo->host)->ShipSpecifics)->dockwith == obj1)
                        {
                            //ship clamped to salcap is colliding with the ship the salcap is docking with,
                            //don't really collide
                            goto nocollision;
                        }
                    }
                }
                if(obj1->clampInfo != NULL)
                {
                    if(((Ship *)obj1->clampInfo->host)->dockingship != NULL)
                    {
                        if(((Ship *)obj1->clampInfo->host)->dockingship == obj2->dockingship)
                        {
                            //docking with same ships...
                            goto nocollision;
                        }
                    }
                }
                else if(((Ship *)obj2->clampInfo->host)->dockingship != NULL)
                {
                    if (((Ship *)obj2->clampInfo->host)->dockingship == obj1)
                    {
                        goto nocollision;
                    }

                    if (((Ship *)obj2->clampInfo->host)->dockingship == obj1->dockingship)
                    {
                        goto nocollision;
                    }
                }
            }

            distsquared = vecMagnitudeSquared(distvector);
            dist = (real32)fsqrt(distsquared);

            if (dist < colldist)
            {
                if (collCheckRectBumpCollision((SpaceObjRotImp *)obj1,(SpaceObjRotImp *)obj2))
                {
                    if (distsquared < 1.0f)
                    {
                        distvector.x = 10.0f;   // special case, if objects are too close together
                        distsquared = 100.0f;
                        dist = 10.0f;
                    }

                    if (!bitTest(obj1->flags, SOF_NotBumpable) && !bitTest(obj2->flags, SOF_NotBumpable))
                    {
                        if(!kamikazeCheck((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2))    //return true if kamikaze collision is true
                        {
                            if (obj1->playerowner == obj2->playerowner)
                            {
                                goto dontapplydamage;
                            }

                            if ((obj1->flags & SOF_Disabled) || (obj2->flags & SOF_Disabled))
                            {
                                goto dontapplydamage;
                            }

                            if(obj1->shiptype == SalCapCorvette || obj1->shiptype == JunkYardDawg)
                            {
                                SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)obj1->ShipSpecifics;
                                if(((SpaceObjRotImpTarg *)spec->noDamageTarget) == ((SpaceObjRotImpTarg *)obj2))
                                {
                                    goto dontapplydamage;
                                }
                            }
                            else if(obj2->shiptype == SalCapCorvette || obj2->shiptype == JunkYardDawg)
                            {
                                SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)obj2->ShipSpecifics;
                                if(((SpaceObjRotImpTarg *)spec->noDamageTarget) == ((SpaceObjRotImpTarg *)obj1))
                                {
                                    goto dontapplydamage;
                                }
                            }

                            ApplyDamageToCollidingObjects((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,&distvector,dist);
dontapplydamage:
                            ObjectsCollided((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist,distsquared);
                        }
                        else
                        {
                            //fix collided ships velocity....
                            ObjectsCollided((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist,distsquared);
                            kamikazeVelocityFix((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2);
                        }
                    }

                }
            }
nocollision:
            obj2index++;
        }
nextobj1:
        obj1index++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckShipResourceColl
    Description : checks collisions between ships and resources
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckShipResourceColl(blob *thisBlob)
{
    sdword obj1index = 0;
    Ship *obj1;

    sdword obj2index;
    Resource *obj2;

    real32 dist;
    real32 distsquared;
    real32 colldist;
    vector distvector;

    real32 obj1collspheresize;
    real32 maxdistCollPossible;

    real32 distcheck;
    real32 absdistcheck;

    SelectCommand *selection = thisBlob->blobShips;
    sdword numShips = selection->numShips;

    ResourceSelection *resselection = thisBlob->blobResources;
    sdword numResources = resselection->numResources;

    real32 maxResourceCollSphereSize;

    sdword bottom,top;

    if (numResources == 0)
    {
        return;
    }

    maxResourceCollSphereSize = thisBlob->blobMaxResourceCollSphereSize;

    while (obj1index < numShips)
    {
        obj1 = selection->ShipPtr[obj1index];
        obj1collspheresize = obj1->staticinfo->staticheader.staticCollInfo.collspheresize;
        maxdistCollPossible = obj1collspheresize + maxResourceCollSphereSize;

        // binary search targetindex such that
        // (obj1->collOptimizeDist - obj2->collOptimizeDist) <= maxdistCollPossible
        bottom = 0;
        top = numResources-1;
        while (top >= bottom)
        {
            obj2index = (bottom+top)>>1;
            if ((obj1->collOptimizeDist - resselection->ResourcePtr[obj2index]->collOptimizeDist) > maxdistCollPossible)
            {
                // targetindex is too small
                bottom = obj2index+1;
            }
            else
            {
                top = obj2index-1;
            }
        }

        dbgAssert(obj2index >= 0);
        dbgAssert(obj2index < numResources);

        while (obj2index < numResources)
        {
            obj2 = resselection->ResourcePtr[obj2index];

#ifdef COLLISION_CHECK_STATS
            shipresourcewalks++;
#endif
            if ((distcheck = obj2->collOptimizeDist - obj1->collOptimizeDist) > maxdistCollPossible)
            {
                goto nextobj1;
            }

            if ((absdistcheck = ABS(distcheck)) > maxdistCollPossible)
            {
                goto nocollision;
            }

            if ((obj1->posinfo.isMoving | obj2->posinfo.isMoving) == FALSE)     // assume objects not moving can't collide
            {
                goto nocollision;
            }

            colldist = obj1collspheresize + obj2->staticinfo->staticheader.staticCollInfo.collspheresize;

            if (absdistcheck > colldist)
            {
                goto nocollision;
            }

            // Check for collisions between obj1 and obj2 here:

#ifdef COLLISION_CHECK_STATS
            shipresourcechecks++;
#endif

            vecSub(distvector,obj2->collInfo.collPosition,obj1->collInfo.collPosition);

            if (!isBetweenInclusive(distvector.x,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.y,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.z,-colldist,colldist))
            {
                goto nocollision;
            }

            distsquared = vecMagnitudeSquared(distvector);
            dist = (real32)fsqrt(distsquared);

            if (dist < colldist)
            {
                if (collCheckRectBumpCollision((SpaceObjRotImp *)obj1,(SpaceObjRotImp *)obj2))
                {
                    if (distsquared < 1.0f)
                    {
                        distvector.x = 10.0f;   // special case, if objects are too close together
                        distsquared = 100.0f;
                        dist = 10.0f;
                    }

                    if (!bitTest(obj1->flags, SOF_NotBumpable) && !bitTest(obj2->flags, SOF_NotBumpable))
                    {
                        if(!kamikazeCheck((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2))    //return true if kamikaze collision is true
                        {
                            if (obj1->shiptype != ResourceCollector)   // ResourceCollector's have special armor because they always collect resources
                            {
                                ApplyDamageToCollidingObjects((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,&distvector,dist);
                            }

                            ObjectsCollided((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist,distsquared);
                        }
                        else
                        {
                             //fix collided ships velocity....
                            ObjectsCollided((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist,distsquared);
                            kamikazeVelocityFix((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2);
                        }
                    }
                }
            }
nocollision:
            obj2index++;
        }
nextobj1:
        obj1index++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckShipDerelictColl
    Description : checks collisions between ships and derelicts
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckShipDerelictColl(blob *thisBlob)
{
    sdword obj1index = 0;
    Ship *obj1;

    sdword obj2index;
    Derelict *obj2;

    real32 dist;
    real32 distsquared;
    real32 colldist;
    vector distvector;

    real32 obj1collspheresize;
    real32 maxdistCollPossible;

    real32 distcheck;
    real32 absdistcheck;

    SelectCommand *selection = thisBlob->blobShips;
    sdword numShips = selection->numShips;

    DerelictSelection *derselection = thisBlob->blobDerelicts;
    sdword numDerelicts = derselection->numDerelicts;

    real32 maxDerelictCollSphereSize;

    sdword bottom,top;

    if (numDerelicts == 0)
    {
        return;
    }

    maxDerelictCollSphereSize = thisBlob->blobMaxDerelictCollSphereSize;

    while (obj1index < numShips)
    {
        obj1 = selection->ShipPtr[obj1index];
        obj1collspheresize = obj1->staticinfo->staticheader.staticCollInfo.collspheresize;
        maxdistCollPossible = obj1collspheresize + maxDerelictCollSphereSize;

        // binary search targetindex such that
        // (obj1->collOptimizeDist - obj2->collOptimizeDist) <= maxdistCollPossible
        bottom = 0;
        top = numDerelicts-1;
        while (top >= bottom)
        {
            obj2index = (bottom+top)>>1;
            if ((obj1->collOptimizeDist - derselection->DerelictPtr[obj2index]->collOptimizeDist) > maxdistCollPossible)
            {
                // targetindex is too small
                bottom = obj2index+1;
            }
            else
            {
                top = obj2index-1;
            }
        }

        dbgAssert(obj2index >= 0);
        dbgAssert(obj2index < numDerelicts);

        while (obj2index < numDerelicts)
        {
            obj2 = derselection->DerelictPtr[obj2index];

#ifdef COLLISION_CHECK_STATS
            shipderelictwalks++;
#endif
            if ((distcheck = obj2->collOptimizeDist - obj1->collOptimizeDist) > maxdistCollPossible)
            {
                goto nextobj1;
            }

            if ((absdistcheck = ABS(distcheck)) > maxdistCollPossible)
            {
                goto nocollision;
            }

            if ((obj1->posinfo.isMoving | obj2->posinfo.isMoving) == FALSE)     // assume objects not moving can't collide
            {
                goto nocollision;
            }

            colldist = obj1collspheresize + obj2->staticinfo->staticheader.staticCollInfo.collspheresize;

            if (absdistcheck > colldist)
            {
                goto nocollision;
            }

            if(obj2->dockingship == obj1)
            {    //derelict is 'docking' with obj1 so don't do collision detection
                goto nocollision;
            }
            // Check for collisions between obj1 and obj2 here:

#ifdef COLLISION_CHECK_STATS
            shipderelictchecks++;
#endif

            vecSub(distvector,obj2->collInfo.collPosition,obj1->collInfo.collPosition);

            if (!isBetweenInclusive(distvector.x,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.y,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.z,-colldist,colldist))
            {
                goto nocollision;
            }

            if(obj1->specialFlags & SPECIAL_IsASalvager)
            {
                if( ((SalCapCorvetteSpec *)obj1->ShipSpecifics)->target == (SpaceObjRotImpTargGuidanceShipDerelict *)obj2)
                {
                    if( ((SalCapCorvetteSpec *)obj1->ShipSpecifics)->noAvoid)
                    {
                        goto nocollision;
                    }
                }
            }

            if ((obj1->flags|obj2->flags) & SOF_Hide)
            {
                goto nocollision;
            }

            distsquared = vecMagnitudeSquared(distvector);
            dist = (real32)fsqrt(distsquared);

            if (dist < colldist)
            {
                if (collCheckRectBumpCollision((SpaceObjRotImp *)obj1,(SpaceObjRotImp *)obj2))
                {
                    if (distsquared < 1.0f)
                    {
                        distvector.x = 10.0f;   // special case, if objects are too close together
                        distsquared = 100.0f;
                        dist = 10.0f;
                    }

                    if (!bitTest(obj1->flags, SOF_NotBumpable) && !bitTest(obj2->flags, SOF_NotBumpable))
                    {
                        if(!kamikazeCheck((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2))    //return true if kamikaze collision is true
                        {
                            // deliberately do not apply damage for ship-derelict collisions
                            ObjectsCollided((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist,distsquared);
                        }
                        else
                        {
                             //fix collided ships velocity....
                            ObjectsCollided((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist,distsquared);
                            kamikazeVelocityFix((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2);
                        }
                    }
                }
            }
nocollision:
            obj2index++;
        }
nextobj1:
        obj1index++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckLineOfSight
    Description :
    Inputs      : source - ship that's firing
                  target - potential destination (target)
                  sourcePosition - origin of the los test in worldspace
                  direction - worldspace unit direction
    Outputs     :
    Return      : TRUE if path is clear, FALSE if obscured
    todo: must check if target is closer than a "hit"
----------------------------------------------------------------------------*/
bool collCheckLineOfSight(Ship* source, Ship* target, vector* sourcePosition, vector* direction)
{
    sdword i, collSide;
    real32 length;
    blob* thisBlob;
    Ship* ship;
    vector distvec;

    if (target == NULL)
    {
        length = REALlyBig;
    }
    else
    {
        vecSub(distvec, target->posinfo.position, *sourcePosition);
        length = fsqrt(vecMagnitudeSquared(distvec));
    }

    if (target == NULL || target->collMyBlob == source->collMyBlob)
    {
        //los in current blob
        thisBlob = source->collMyBlob;
        for (i = 0; i < thisBlob->blobShips->numShips; i++)
        {
            ship = thisBlob->blobShips->ShipPtr[i];
            if (ship == source || ship == target)
            {
                //ignore source or target
                continue;
            }

            if (!allianceIsShipAlly(ship, source->playerowner))
            {
                //only test with allied ships, self included
                continue;
            }

            if (collCheckRectLine((SpaceObjRotImp*)ship, sourcePosition, direction, length, &collSide) != -1.0f)
            {
                //hit something of ours
                return FALSE;
            }
        }

        //path is clear
        return TRUE;
    }
    else
    {
        //los in universe
        Node* blobnode = universe.collBlobList.head;
        vector distvector;
        real32 distsquared, v, dsqr;

        while (blobnode != NULL)
        {
            thisBlob = (blob*)listGetStructOfNode(blobnode);
            blobnode = blobnode->next;

            vecSub(distvector, thisBlob->centre, *sourcePosition);
            distsquared = vecMagnitudeSquared(distvector);
            v = vecDotProduct(distvector, *direction);
			if (v > 0)
			{
				dsqr = (thisBlob->radius * thisBlob->radius) - (distsquared - (v*v));
				if (dsqr > 0)
				{
					//line intersects blob, check all ships
					for (i = 0; i < thisBlob->blobShips->numShips; i++)
					{
						ship = thisBlob->blobShips->ShipPtr[i];
						if (ship == source || ship == target)
						{
							//ignore source or target
							continue;
						}

						if (!allianceIsShipAlly(ship, source->playerowner))
						{
							//only test with allied ships, self included
							continue;
						}

						if (collCheckRectLine((SpaceObjRotImp*)ship, sourcePosition, direction, length, &collSide) != -1.0f)
						{
							//hit something of ours
							return FALSE;
						}
					}
				}
			}
        }

        //path is clear
        return TRUE;
    }
}

void collCheckBeamCollWithTargetsInBlob(Bullet *bullet,blob *thisBlob,real32 *minbeamCollideLineDist,SpaceObjRotImpTarg **minbeamTarget,sdword *minbeamCollSide)
{
    SelectAnyCommand *targetselection;
    sdword numTargets;

    SpaceObjRotImpTarg *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    real32 collideLineDist;

    sdword targetindex;

    sdword pass = 0;

passagain:

    if (pass == 0)
    {
        targetselection = thisBlob->blobSmallTargets;
    }
    else
    {
        dbgAssert(pass == 1);
        targetselection = thisBlob->blobBigTargets;
    }

    targetindex = 0;
    numTargets = targetselection->numTargets;

    while (targetindex < numTargets)
    {
        target = targetselection->TargetPtr[targetindex];
        targetstaticheader = &target->staticinfo->staticheader;

#ifdef COLLISION_CHECK_STATS
bulletwalks++;
#endif

        if (bullet->owner == (Ship *)target)
        {
            goto beamnexttarget;
        }

        // Check for collisions between bullet and target here:

#ifdef COLLISION_CHECK_STATS
bulletchecks++;
#endif

        vecSub(distvector,target->collInfo.collPosition,bullet->posinfo.position);
        distsquared = vecMagnitudeSquared(distvector);
        v = vecDotProduct(distvector,bullet->bulletheading);
		if (v > 0)
        {
			dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v*v));
			if (dsqr > 0)
			{
				d = fsqrt(dsqr);
				if ((v-d) < bullet->traveldist)
				{
					if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)target,&bullet->posinfo.position,&bullet->bulletheading,bullet->traveldist,&collSide)) >= 0.0f)
					{
						if (collideLineDist == 0.0f)
						{
							collideLineDist = v-d;
							if (collideLineDist < 0.0f)
							{
								collideLineDist = 0.0f;
							}
						}
						if (collideLineDist < *minbeamCollideLineDist)
						{
							*minbeamCollideLineDist = collideLineDist;
							*minbeamTarget = target;
							*minbeamCollSide = collSide;
						}
					}
				}
			}
		}

beamnexttarget:
        targetindex++;
    }

nextpass:
    pass++;
    if (pass < 2)
    {
        goto passagain;
    }
}

void collCheckBeamColl(blob *thisBlob,Bullet *bullet)
{
    real32 minbeamCollideLineDist = REALlyBig;
    SpaceObjRotImpTarg *minbeamTarget = NULL;
    sdword minbeamCollSide = -1;
    Node *blobnode = universe.collBlobList.head;
    blob *checkBlob;

    real32 collideLineDist;
    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    if(bullet->target != NULL)
    {
        switch(bullet->target->objtype)
        {
        case OBJ_MissileType:
            //do bullet/missle target collision with its target only!
            vecSub(distvector,bullet->target->collInfo.collPosition,bullet->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            v = vecDotProduct(distvector,bullet->bulletheading);
			//if (v > 0)
			{
				dsqr = bullet->target->staticinfo->staticheader.staticCollInfo.collspheresizeSqr - (distsquared - (v*v));
				if (dsqr > 0)
				{
					d = fsqrt(dsqr);
					if ((v-d) < bullet->traveldist)
					{
						if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)bullet->target,&bullet->posinfo.position,&bullet->bulletheading,bullet->traveldist,&collSide)) >= 0.0f)
						{
							if (collideLineDist == 0.0f)
							{
								collideLineDist = v-d;
								if (collideLineDist < 0.0f)
								{
									collideLineDist = 0.0f;
								}
							}
							if (collideLineDist < minbeamCollideLineDist)
							{
								minbeamCollideLineDist = collideLineDist;
								minbeamTarget = bullet->target;
								minbeamCollSide = collSide;
							}
						}
					}
				}
			}
            break;
        default:
            break;
        }
    }

    collCheckBeamCollWithTargetsInBlob(bullet,thisBlob,&minbeamCollideLineDist,&minbeamTarget,&minbeamCollSide);

    // do line-blob collision test for all other blobs in universe:
    while (blobnode != NULL)
    {
        checkBlob = (blob *)listGetStructOfNode(blobnode);
        blobnode = blobnode->next;

        if (checkBlob == thisBlob)
        {
            continue;
        }

        vecSub(distvector,checkBlob->centre,bullet->posinfo.position);
        distsquared = vecMagnitudeSquared(distvector);
        v = vecDotProduct(distvector,bullet->bulletheading);
		if (v > 0)
		{
			dsqr = (checkBlob->radius*checkBlob->radius) - (distsquared - (v*v));
			if (dsqr > 0)
			{
				d = fsqrt(dsqr);
				if ((v-d) < bullet->traveldist)
				{
					collCheckBeamCollWithTargetsInBlob(bullet,checkBlob,&minbeamCollideLineDist,&minbeamTarget,&minbeamCollSide);
				}
			}
		}
    }

    if (minbeamTarget != NULL)        // beam only collides with closest target
    {
        dbgAssert(minbeamCollideLineDist >= 0.0f);
        univBulletCollidedWithTarget(minbeamTarget,&minbeamTarget->staticinfo->staticheader,bullet,minbeamCollideLineDist,minbeamCollSide);
        bullet->beamtraveldist = minbeamCollideLineDist;
    }
    else
    {
        bullet->beamtraveldist = bullet->lengthmag;
    }
    //adjust length of effect associated with bullet, if applicable
    if (bullet->effect != NULL)
    {
        ((real32 *)bullet->effect->variable)[ETG_LengthVariable] = bullet->beamtraveldist;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckBulletTargetColl
    Description : checks collisions between bullets and targets.  When a bullet
                  collides with a target, it should disappear and do damage to
                  the enemy ship.
    Inputs      :
    Outputs     :
    Return      :
    Notes       : This routine uses line-sphere intersection for collision detection
----------------------------------------------------------------------------*/
void collCheckBulletTargetColl(blob *thisBlob)
{
    sdword bulletindex = 0;
    Bullet *bullet;

    sdword targetindex;
    SpaceObjRotImpTarg *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    real32 distcheck;

    bool deletebulletflag;

    real32 collideLineDist;

    real32 testcollsizesqr;

    BulletSelection *bulletselection = thisBlob->blobBullets;

    SelectAnyCommand *targetselection;
    sdword numTargets;

    sdword bottom,top;

    real32 maxTargetCollSphereSizePlusBulletTravelDist;

    sdword pass;

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (bulletindex < bulletselection->numBullets)
    {
        bullet = bulletselection->BulletPtr[bulletindex];
        dbgAssert(bullet->objtype == OBJ_BulletType);

        deletebulletflag = FALSE;

        switch (bullet->bulletType)
        {
            case BULLET_Beam:
                collCheckBeamColl(thisBlob,bullet);
                break;
            case BULLET_Laser:
                break;              //don't do collision detection for lasers
            default:
                pass = 0;


                if(bullet->target != NULL)
                {
                    switch(bullet->target->objtype)
                    {
                    case OBJ_MissileType:
                        //do bullet/missle target collision with its target only!
                         distcheck = bullet->target->collOptimizeDist - bullet->collOptimizeDist;

                        if (ABS(distcheck) > (bullet->target->staticinfo->staticheader.staticCollInfo.collspheresize+bullet->traveldist))
                        {
                            //trivially reject...
                            goto nominecollision;
                        }
                        vecSub(distvector,bullet->target->collInfo.collPosition,bullet->posinfo.position);
                        distsquared = vecMagnitudeSquared(distvector);
                        v = vecDotProduct(distvector,bullet->bulletheading);
						//if (v > 0)
						{

							testcollsizesqr = bullet->target->staticinfo->staticheader.staticCollInfo.collspheresizeSqr;

							dsqr = testcollsizesqr - (distsquared - (v*v));
							if (dsqr > 0)
							{
								d = fsqrt(dsqr);
								if ((v-d) < bullet->traveldist)
								{
									if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)bullet->target,&bullet->posinfo.position,&bullet->bulletheading,bullet->traveldist,&collSide)) >= 0.0f)
									{
										univBulletCollidedWithTarget(bullet->target,&bullet->target->staticinfo->staticheader,bullet,collideLineDist,collSide);
										deletebulletflag = TRUE;
										goto nextbullet;
									}
								}
							}
						}

                        break;
                    default:
                        break;
                    }
                }
nominecollision:
passagain:
                if (pass == 0)
                {
                    targetselection = thisBlob->blobSmallTargets;
                    numTargets = targetselection->numTargets;

                    if (numTargets == 0)
                    {
                        goto nextpass;         // go directly to next pass
                    }

                    maxTargetCollSphereSizePlusBulletTravelDist = thisBlob->blobMaxSTargetCollSphereSize + bullet->traveldist;

                    // binary search targetindex such that
                    // (bullet->collOptimizeDist - target->collOptimizeDist) <= maxTargetCollSphereSize
                    bottom = 0;
                    top = numTargets-1;
                    while (top >= bottom)
                    {
                        targetindex = (bottom+top)>>1;
                        if ((bullet->collOptimizeDist - targetselection->TargetPtr[targetindex]->collOptimizeDist) > maxTargetCollSphereSizePlusBulletTravelDist)
                        {
                            // targetindex is too small
                            bottom = targetindex+1;
                        }
                        else
                        {
                            top = targetindex-1;
                        }
                    }

                    dbgAssert(targetindex >= 0);
                    dbgAssert(targetindex < numTargets);
                }
                else
                {
                    dbgAssert(pass == 1);
                    targetselection = thisBlob->blobBigTargets;
                    numTargets = targetselection->numTargets;
                    targetindex = 0;
                    maxTargetCollSphereSizePlusBulletTravelDist = thisBlob->blobMaxBTargetCollSphereSize + bullet->traveldist;
                }

                while (targetindex < numTargets)
                {
                    target = targetselection->TargetPtr[targetindex];
                    targetstaticheader = &target->staticinfo->staticheader;

#ifdef COLLISION_CHECK_STATS
            bulletwalks++;
#endif

                    distcheck = target->collOptimizeDist - bullet->collOptimizeDist;

                    if (distcheck > maxTargetCollSphereSizePlusBulletTravelDist)
                    {
                        goto nextpass;
                    }

                    if ((target->objtype != OBJ_ShipType) || (((Ship *)target)->shiptype != DFGFrigate && ((Ship *)target)->shiptype != DefenseFighter))
                    {
                        if (ABS(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+bullet->traveldist))
                        {
                            goto nexttarget;
                        }
                    }

                    if (bullet->owner == (Ship *)target)
                    {
                        goto nexttarget;
                    }


                    if ((target->objtype == OBJ_DustType) || (target->objtype == OBJ_NebulaType))
                    {
                        goto nexttarget;
                    }

                    // Check for collisions between bullet and target here:

#ifdef COLLISION_CHECK_STATS
            bulletchecks++;
#endif

                    vecSub(distvector,target->collInfo.collPosition,bullet->posinfo.position);
                    distsquared = vecMagnitudeSquared(distvector);
                    v = vecDotProduct(distvector,bullet->bulletheading);

                    if (target->objtype == OBJ_ShipType)
                    {
                        if(((Ship *)target)->shiptype == DFGFrigate)
                        {       //if target is a dfgf,
                            DFGFrigateStatics *DFGFstatics;

                            DFGFstatics = (DFGFrigateStatics *) ((ShipStaticInfo *)(((Ship *)target)->staticinfo))->custstatinfo;
                            if(distsquared < DFGFstatics->DFGFrigateFieldRadiusSqr)
                            {   //and bullet is within its field radius...affect the bullet
                                univDFGFieldEffect((Ship *)target,bullet,universe.totaltimeelapsed);
                            }
                        }
                        if(((Ship *)target)->shiptype == DefenseFighter)
                        {    //if target is a DefenseFighter
                            DefenseFighterStatics *defensestatics;
                            defensestatics = (DefenseFighterStatics *) ((ShipStaticInfo *)(((Ship *)target)->staticinfo))->custstatinfo;
                            if(distsquared < ((Ship *)target)->staticinfo->bulletRangeSquared[((Ship *)target)->tacticstype])
                            {
                                DefenseFighterReportBullet((Ship *)target, bullet);
                            }
                        }
                    }

					//if (v > 0)
					{

						testcollsizesqr = targetstaticheader->staticCollInfo.collspheresizeSqr;

						dsqr = testcollsizesqr - (distsquared - (v*v));
						if (dsqr > 0)
						{
							d = fsqrt(dsqr);
							if ((v-d) < bullet->traveldist)
							{
								if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)target,&bullet->posinfo.position,&bullet->bulletheading,bullet->traveldist,&collSide)) >= 0.0f)
								{
									univBulletCollidedWithTarget(target,targetstaticheader,bullet,collideLineDist,collSide);
									deletebulletflag = TRUE;
									goto nextbullet;
								}
							}
						}
					}

        nexttarget:
                    targetindex++;
                }

nextpass:
                pass++;
                if (pass < 2)
                {
                    goto passagain;
                }
                break;
        }

nextbullet:
        if (deletebulletflag)
        {
            // we are deleting this bullet's index, so don't update bulletindex

            if(bitTest(bullet->SpecialEffectFlag, 0x0002))
            {    //bullet is possibly being targetted by a defense fighter...remove that reference
                DefenseFighterBulletRemoval(bullet);
            }

            bobObjectDied((SpaceObj *)bullet,&universe.collBlobList);
            listRemoveNode(&bullet->bulletlink);
            univRemoveObjFromRenderList((SpaceObj *)bullet);
            listDeleteNode(&bullet->objlink);
#if 0
            univDeleteBullet(bullet);
#endif
        }
        else
        {
            bulletindex++;
        }
    }
#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : collCheckMissileShipColl
    Description : checks collisions between missiles and ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckMissileShipColl(blob *thisBlob,MissileType missileType)
{
    sdword missileindex = 0;
    Missile *missile;

    sdword targetindex;
    Ship *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    real32 distcheck;

    real32 collideLineDist;

    vector missileheading;

    MissileSelection *missileselection;
    sdword numMissiles;

    SelectCommand *targetselection;
    sdword numTargets;

    sdword bottom,top;

    real32 maxCollSphereSizePlusMissileTravelDist;

    sdword pass;

    if (missileType == MISSILE_Mine)
    {
        missileselection = thisBlob->blobMissileMines;
    }
    else
    {
        missileselection = thisBlob->blobMissileMissiles;
    }
    numMissiles = missileselection->numMissiles;

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (missileindex < numMissiles)
    {
        missile = missileselection->MissilePtr[missileindex];
        dbgAssert(missile->objtype == OBJ_MissileType);

        if (missile->flags & SOF_Dead)
        {
            goto nextmissile;
        }

        pass = 0;

passagain:

        if (pass == 0)
        {
            targetselection = thisBlob->blobSmallShips;
            numTargets = targetselection->numShips;

            if (numTargets == 0)
            {
                goto nextpass;
            }

            maxCollSphereSizePlusMissileTravelDist = thisBlob->blobMaxSmallShipCollSphereSize+missile->maxtraveldist;

            // binary search targetindex such that
            // (missile->collOptimizeDist - target->collOptimizeDist) <= maxCollSphereSize
            bottom = 0;
            top = numTargets-1;
            while (top >= bottom)
            {
                targetindex = (bottom+top)>>1;
                if ((missile->collOptimizeDist - targetselection->ShipPtr[targetindex]->collOptimizeDist) > maxCollSphereSizePlusMissileTravelDist)
                {
                    // targetindex is too small
                    bottom = targetindex+1;
                }
                else
                {
                    top = targetindex-1;
                }
            }

            dbgAssert(targetindex >= 0);
            dbgAssert(targetindex < numTargets);
        }
        else
        {
            dbgAssert(pass == 1);
            targetselection = thisBlob->blobBigShips;
            numTargets = targetselection->numShips;
            maxCollSphereSizePlusMissileTravelDist = thisBlob->blobMaxBigShipCollSphereSize+missile->maxtraveldist;
            targetindex = 0;
        }

        while (targetindex < numTargets)
        {
            target = targetselection->ShipPtr[targetindex];

#ifdef COLLISION_CHECK_STATS
    missilewalks++;
#endif

            distcheck = target->collOptimizeDist - missile->collOptimizeDist;

            if (distcheck > maxCollSphereSizePlusMissileTravelDist)
            {
                goto nextpass;
            }

            targetstaticheader = &target->staticinfo->staticheader;

            if (ABS(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+missile->maxtraveldist))
            {
                goto nexttarget;
            }

            if (missile->owner == (Ship *)target)
            {
                goto nexttarget;
            }

            // Check for collisions between missile and target here:

#ifdef COLLISION_CHECK_STATS
    missilechecks++;
#endif
            vecSub(distvector,target->collInfo.collPosition,missile->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
            v = vecDotProduct(distvector,missileheading);
			//if (v > 0)
			{
				dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v*v));
				if (dsqr > 0)
				{
					d = fsqrt(dsqr);
					if ((v-d) < missile->maxtraveldist)
					{
						if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)target,&missile->posinfo.position,&missileheading,missile->maxtraveldist,&collSide)) >= 0.0f)
						{
							univMissileCollidedWithTarget((SpaceObjRotImpTarg *)target,targetstaticheader,missile,collideLineDist,collSide);
							AddMissileToDeleteMissileList(missile, -1);
							goto nextmissile;
						}
					}
				}
			}

nexttarget:
            targetindex++;
        }

nextpass:
        pass++;
        if (pass < 2)
        {
            goto passagain;
        }

nextmissile:
        missileindex++;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif

}

/*-----------------------------------------------------------------------------
    Name        : collCheckMissileResourceColl
    Description : checks collisions between missiles and resources
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckMissileResourceColl(blob *thisBlob,MissileType missileType)
{
    sdword missileindex = 0;
    Missile *missile;

    sdword targetindex;
    Resource *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    real32 distcheck;

    real32 collideLineDist;

    vector missileheading;

    MissileSelection *missileselection;
    sdword numMissiles;

    ResourceSelection *targetselection = thisBlob->blobResources;
    sdword numTargets = targetselection->numResources;

    sdword bottom,top;

    real32 maxCollSphereSize = thisBlob->blobMaxResourceCollSphereSize;
    real32 maxCollSphereSizePlusMissileTravelDist;

    if (numTargets == 0)
    {
        return;
    }

    if (missileType == MISSILE_Mine)
    {
        missileselection = thisBlob->blobMissileMines;
    }
    else
    {
        missileselection = thisBlob->blobMissileMissiles;
    }
    numMissiles = missileselection->numMissiles;

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (missileindex < numMissiles)
    {
        missile = missileselection->MissilePtr[missileindex];
        dbgAssert(missile->objtype == OBJ_MissileType);

        if (missile->flags & SOF_Dead)
        {
            goto nextmissile;
        }

        maxCollSphereSizePlusMissileTravelDist = maxCollSphereSize + missile->maxtraveldist;

        // binary search targetindex such that
        // (missile->collOptimizeDist - target->collOptimizeDist) <= maxCollSphereSizePlusMissileTravelDist
        bottom = 0;
        top = numTargets-1;
        while (top >= bottom)
        {
            targetindex = (bottom+top)>>1;
            if ((missile->collOptimizeDist - targetselection->ResourcePtr[targetindex]->collOptimizeDist) > maxCollSphereSizePlusMissileTravelDist)
            {
                // targetindex is too small
                bottom = targetindex+1;
            }
            else
            {
                top = targetindex-1;
            }
        }

        dbgAssert(targetindex >= 0);
        dbgAssert(targetindex < numTargets);

        while (targetindex < numTargets)
        {
            target = targetselection->ResourcePtr[targetindex];

#ifdef COLLISION_CHECK_STATS
    missilewalks++;
#endif

            distcheck = target->collOptimizeDist - missile->collOptimizeDist;

            if (distcheck > maxCollSphereSizePlusMissileTravelDist)
            {
                goto nextmissile;
            }

            targetstaticheader = &target->staticinfo->staticheader;

            if (ABS(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+missile->maxtraveldist))
            {
                goto nexttarget;
            }

            if (target->flags & SOF_NotBumpable)
            {
                goto nexttarget;
            }

            // Check for collisions between missile and target here:

#ifdef COLLISION_CHECK_STATS
    missilechecks++;
#endif
            vecSub(distvector,target->collInfo.collPosition,missile->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
            v = vecDotProduct(distvector,missileheading);
			//if (v > 0)
			{
				dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v*v));
				if (dsqr > 0)
				{
					d = fsqrt(dsqr);
					if ((v-d) < missile->maxtraveldist)
					{
						if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)target,&missile->posinfo.position,&missileheading,missile->maxtraveldist,&collSide)) >= 0.0f)
						{
							univMissileCollidedWithTarget((SpaceObjRotImpTarg *)target,targetstaticheader,missile,collideLineDist,collSide);
							AddMissileToDeleteMissileList(missile, -1);
							goto nextmissile;
						}
					}
				}
			}

nexttarget:
            targetindex++;
        }

nextmissile:
        missileindex++;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif

}


/*-----------------------------------------------------------------------------
    Name        : collCheckMissileDerelictColl
    Description : checks collisions between missiles and derelicts
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckMissileDerelictColl(blob *thisBlob,MissileType missileType)
{
    sdword missileindex = 0;
    Missile *missile;

    sdword targetindex;
    Derelict *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    real32 distcheck;

    real32 collideLineDist;

    vector missileheading;

    MissileSelection *missileselection;
    sdword numMissiles;

    DerelictSelection *targetselection = thisBlob->blobDerelicts;
    sdword numTargets = targetselection->numDerelicts;

    real32 maxCollSphereSize = thisBlob->blobMaxDerelictCollSphereSize;
    real32 maxCollSphereSizePlusMissileTravelDist;

    sdword bottom,top;

    if (numTargets == 0)
    {
        return;
    }

    if (missileType == MISSILE_Mine)
    {
        missileselection = thisBlob->blobMissileMines;
    }
    else
    {
        missileselection = thisBlob->blobMissileMissiles;
    }
    numMissiles = missileselection->numMissiles;

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (missileindex < numMissiles)
    {
        missile = missileselection->MissilePtr[missileindex];
        dbgAssert(missile->objtype == OBJ_MissileType);

        if (missile->flags & SOF_Dead)
        {
            goto nextmissile;
        }

        maxCollSphereSizePlusMissileTravelDist = maxCollSphereSize + missile->maxtraveldist;

        // binary search targetindex such that
        // (missile->collOptimizeDist - target->collOptimizeDist) <= maxCollSphereSizePlusMissileTravelDist
        bottom = 0;
        top = numTargets-1;
        while (top >= bottom)
        {
            targetindex = (bottom+top)>>1;
            if ((missile->collOptimizeDist - targetselection->DerelictPtr[targetindex]->collOptimizeDist) > maxCollSphereSizePlusMissileTravelDist)
            {
                // targetindex is too small
                bottom = targetindex+1;
            }
            else
            {
                top = targetindex-1;
            }
        }

        dbgAssert(targetindex >= 0);
        dbgAssert(targetindex < numTargets);

        while (targetindex < numTargets)
        {
            target = targetselection->DerelictPtr[targetindex];

#ifdef COLLISION_CHECK_STATS
    missilewalks++;
#endif

            distcheck = target->collOptimizeDist - missile->collOptimizeDist;

            if (distcheck > maxCollSphereSizePlusMissileTravelDist)
            {
                goto nextmissile;
            }

            targetstaticheader = &target->staticinfo->staticheader;

            if (ABS(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+missile->maxtraveldist))
            {
                goto nexttarget;
            }

            // Check for collisions between missile and target here:

#ifdef COLLISION_CHECK_STATS
    missilechecks++;
#endif
            vecSub(distvector,target->collInfo.collPosition,missile->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
            v = vecDotProduct(distvector,missileheading);
			//if (v > 0)
			{
				dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v*v));
				if (dsqr > 0)
				{
					d = fsqrt(dsqr);
					if ((v-d) < missile->maxtraveldist)
					{
						if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)target,&missile->posinfo.position,&missileheading,missile->maxtraveldist,&collSide)) >= 0.0f)
						{
							univMissileCollidedWithTarget((SpaceObjRotImpTarg *)target,targetstaticheader,missile,collideLineDist,collSide);
							AddMissileToDeleteMissileList(missile, -1);
							goto nextmissile;
						}
					}
				}
			}

nexttarget:
            targetindex++;
        }

nextmissile:
        missileindex++;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif

}

/*-----------------------------------------------------------------------------
    Name        : collCheckMissileMineColl
    Description : checks collisions between missiles and mines
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckMissileMineColl(blob *thisBlob)
{
    sdword missileindex = 0;
    Missile *missile;

    sdword targetindex;
    Missile *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    real32 distcheck;

    real32 collideLineDist;

    vector missileheading;

    MissileSelection *missileselection = thisBlob->blobMissileMissiles;
    sdword numMissiles = missileselection->numMissiles;

    MissileSelection *targetselection = thisBlob->blobMissileMines;
    sdword numTargets = targetselection->numMissiles;

    sdword bottom,top;

    real32 maxMineCollSphereSizePlusMissileTravelDist;

    if (numTargets == 0)
    {
        return;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (missileindex < numMissiles)
    {
        missile = missileselection->MissilePtr[missileindex];
        dbgAssert(missile->objtype == OBJ_MissileType);
        dbgAssert(missile->missileType == MISSILE_Regular);

        if (missile->flags & SOF_Dead)
        {
            goto nextmissile;
        }

        maxMineCollSphereSizePlusMissileTravelDist = maxMineCollSphereSize + missile->maxtraveldist;

        // binary search targetindex such that
        // (missile->collOptimizeDist - target->collOptimizeDist) <= maxMineCollSphereSizePlusMissileTravelDist
        bottom = 0;
        top = numTargets-1;
        while (top >= bottom)
        {
            targetindex = (bottom+top)>>1;
            if ((missile->collOptimizeDist - targetselection->MissilePtr[targetindex]->collOptimizeDist) > maxMineCollSphereSizePlusMissileTravelDist)
            {
                // targetindex is too small
                bottom = targetindex+1;
            }
            else
            {
                top = targetindex-1;
            }
        }

        dbgAssert(targetindex >= 0);
        dbgAssert(targetindex < numTargets);

        while (targetindex < numTargets)
        {
            target = targetselection->MissilePtr[targetindex];
            dbgAssert(target->objtype == OBJ_MissileType);
            dbgAssert(target->missileType == MISSILE_Mine);

#ifdef COLLISION_CHECK_STATS
    missilewalks++;
#endif

            distcheck = target->collOptimizeDist - missile->collOptimizeDist;

            if (distcheck > maxMineCollSphereSizePlusMissileTravelDist)
            {
                goto nextmissile;
            }

            targetstaticheader = &target->staticinfo->staticheader;

            if (ABS(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+missile->maxtraveldist))
            {
                goto nexttarget;
            }

            // Check for collisions between missile and target here:

#ifdef COLLISION_CHECK_STATS
    missilechecks++;
#endif
            vecSub(distvector,target->collInfo.collPosition,missile->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
            v = vecDotProduct(distvector,missileheading);
			//if (v > 0)
			{
				dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v*v));
				if (dsqr > 0)
				{
					d = fsqrt(dsqr);
					if ((v-d) < missile->maxtraveldist)
					{
						if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)target,&missile->posinfo.position,&missileheading,missile->maxtraveldist,&collSide)) >= 0.0f)
						{
							univMissileCollidedWithTarget((SpaceObjRotImpTarg *)target,targetstaticheader,missile,collideLineDist,collSide);
							AddMissileToDeleteMissileList(missile, -1);
							goto nextmissile;
						}
					}
				}
			}

nexttarget:
            targetindex++;
        }

nextmissile:
        missileindex++;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif

}

/*-----------------------------------------------------------------------------
    Name        : collCheckAllBumpCollisions
    Description : checks all bump collisions in universe (ship-ship, ship-resource, etc)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckAllBumpCollisions(void)
{
    Node *blobnode = universe.collBlobList.head;
    blob *thisBlob;

#ifdef COLLISION_CHECK_STATS
    shipshipwalks = 0;
    shipshipchecks = 0;
    shipresourcewalks = 0;
    shipresourcechecks = 0;
    shipderelictwalks = 0;
    shipderelictchecks = 0;
#endif

    if (!nisIsRunning)
    {
        // for each blob, check all collisions within that blob
        while (blobnode != NULL)
        {
            thisBlob = (blob *)listGetStructOfNode(blobnode);
            blobnode = blobnode->next;

            collCheckShipShipColl(thisBlob,TRUE);       // check smallship-smallship collisions
            collCheckShipShipColl(thisBlob,FALSE);      // check bigship-bigship collisions
            collCheckBigShipSmallShipColl(thisBlob);    // check bigship-smallship collisions
            collCheckShipResourceColl(thisBlob);
            collCheckShipDerelictColl(thisBlob);
        }
    }
    else if ((singlePlayerGame) && (singlePlayerGameInfo.currentMission == 4))
    {
        // for each blob, check all collisions within that blob
        while (blobnode != NULL)
        {
            thisBlob = (blob *)listGetStructOfNode(blobnode);
            blobnode = blobnode->next;

            // in case Bentusi traders run into some ships...
            collCheckShipShipColl(thisBlob,FALSE);      // check bigship-bigship collisions
            collCheckBigShipSmallShipColl(thisBlob);    // check bigship-smallship collisions
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckAllBulletMissileCollisions
    Description : checks all bullet, missile, and mine collisions in universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckAllBulletMissileCollisions(void)
{
    Node *blobnode = universe.collBlobList.head;
    blob *thisBlob;
    udword savemissilewalks;
    udword savemissilechecks;

#ifdef COLLISION_CHECK_STATS
    bulletwalks = 0;
    bulletchecks = 0;
    missilewalks = 0;
    missilechecks = 0;
    minewalks = 0;
    minechecks = 0;
#endif

    // for each blob, check all collisions within that blob
    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);
        blobnode = blobnode->next;

        collCheckBulletTargetColl(thisBlob);
        //collCheckBulletShipColl(thisBlob);
        //collCheckBulletResourceColl(thisBlob);
        //collCheckBulletDerelictColl(thisBlob);
        //collCheckBulletMissileColl(thisBlob);

        //collCheckMissileTargetColl(thisBlob);

        collCheckMissileShipColl(thisBlob,MISSILE_Regular);
        collCheckMissileResourceColl(thisBlob,MISSILE_Regular);
        collCheckMissileDerelictColl(thisBlob,MISSILE_Regular);

#ifdef COLLISION_CHECK_STATS
        savemissilewalks = missilewalks;
        savemissilechecks = missilechecks;
        missilewalks = 0;
        missilechecks = 0;
#endif

        collCheckMissileShipColl(thisBlob,MISSILE_Mine);
        //collCheckMissileResourceColl(thisBlob,MISSILE_Mine);
        collCheckMissileDerelictColl(thisBlob,MISSILE_Mine);

#ifdef COLLISION_CHECK_STATS
        minewalks += missilewalks;
        minechecks += missilechecks;
        missilewalks = savemissilewalks;
        missilechecks = savemissilechecks;
#endif

        collCheckMissileMineColl(thisBlob);
    }
}


