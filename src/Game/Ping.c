/*=============================================================================
    Name    : Ping.c
    Purpose : Code to update and draw sensors manager pings.

    Created 9/8/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <string.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "Task.h"
#include "prim2d.h"
#include "SpaceObj.h"
#include "LinkedList.h"
#include "Universe.h"
#include "Camera.h"
#include "PiePlate.h"
#include "Select.h"
#include "Blobs.h"
#include "Strings.h"
#include "Tactical.h"
#include "Teams.h"
#include "ProximitySensor.h"
#include "Sensors.h"
#include "Alliance.h"
#include "Battle.h"
#include "SaveGame.h"
#include "SoundEvent.h"
#include "CameraCommand.h"
#include "Ping.h"

/*=============================================================================
    Data:
=============================================================================*/
taskhandle pingTaskHandle;                      //ping evaluation task
LinkedList pingList;                            //list of pings

SpaceObj *pingDyingObject;                      //object that is dying and must be evaluated

extern bool8 smZoomingIn;
extern bool8 smZoomingOut;

//battle-blob stuff
BlobProperties pingBattleBlobProperties;

//anomaly ping tweakables
color  pingAnomalyColor          = PING_AnomalyColor;
real32 pingAnomalySize           = PING_AnomalySize;
real32 pingAnomalyMinSize        = PING_AnomalyMinSize;
real32 pingAnomalyPingDuration   = PING_AnomalyPingDuration;
real32 pingAnomalyInterPingPause = PING_AnomalyInterPingPause;
real32 pingAnomalyEvaluatePeriod = PING_AnomalyEvaluatePeriod;
real32 pingAnomalyMinScreenSize  = PING_AnomalyMinScreenSize;

//battle ping tweakables
color  pingBattleColor           = PING_BattleColor;
//real32 pingBattleSize            = PING_BattleSize;
real32 pingBattleMinSize         = PING_BattleMinSize;
real32 pingBattlePingDuration    = PING_BattlePingDuration;
real32 pingBattleInterPingPause  = PING_BattleInterPingPause;
real32 pingBattleEvaluatePeriod  = PING_BattleEvaluatePeriod;
real32 pingBattleMinScreenSize   = PING_BattleMinScreenSize;

//NewShip ping tweakables
color  pingNewShipColor           = PING_NewShipColor;
real32 pingNewShipSize            = PING_NewShipSize;
real32 pingNewShipMinSize         = PING_NewShipMinSize;
real32 pingNewShipPingDuration    = PING_NewShipPingDuration;
real32 pingNewShipInterPingPause  = PING_NewShipInterPingPause;
real32 pingNewShipEvaluatePeriod  = PING_NewShipEvaluatePeriod;
real32 pingNewShipMinScreenSize   = PING_NewShipMinScreenSize;

//misc tweakables
sdword pingTOMarginX              = PING_TOMarginX;
sdword pingTONSegments            = PING_TONSegments;
real32 pingTOLingerTime           = PING_TOLingerTime;

scriptEntry pingTweaks[] =
{
    { "AnomalyColor",           scriptSetRGBCB,    &pingAnomalyColor },
    { "AnomalySize",            scriptSetReal32CB, &pingAnomalySize },
    { "AnomalyMinSize",         scriptSetReal32CB, &pingAnomalyMinSize },
    { "AnomalyPingDuration",    scriptSetReal32CB, &pingAnomalyPingDuration },
    { "AnomalyInterPingPause",  scriptSetReal32CB, &pingAnomalyInterPingPause },
    { "AnomalyEvaluatePeriod",  scriptSetReal32CB, &pingAnomalyEvaluatePeriod },
    { "AnomalyMinScreenSize",   scriptSetReal32CB, &pingAnomalyMinScreenSize },

    { "BattleColor",            scriptSetRGBCB,    &pingBattleColor },
//    { "BattleSize",             scriptSetReal32CB, &pingBattleSize },
    { "BattleMinSize",          scriptSetReal32CB, &pingBattleMinSize },
    { "BattlePingDuration",     scriptSetReal32CB, &pingBattlePingDuration },
    { "BattleInterPingPause",   scriptSetReal32CB, &pingBattleInterPingPause },
    { "BattleEvaluatePeriod",   scriptSetReal32CB, &pingBattleEvaluatePeriod },
    { "BattleMinScreenSize",    scriptSetReal32CB, &pingBattleMinScreenSize },

    { "NewShipColor",           scriptSetRGBCB,    &pingNewShipColor },
    { "NewShipSize",            scriptSetReal32CB, &pingNewShipSize },
    { "NewShipMinSize",         scriptSetReal32CB, &pingNewShipMinSize },
    { "NewShipPingDuration",    scriptSetReal32CB, &pingNewShipPingDuration },
    { "NewShipInterPingPause",  scriptSetReal32CB, &pingNewShipInterPingPause },
    { "NewShipEvaluatePeriod",  scriptSetReal32CB, &pingNewShipEvaluatePeriod },
    { "NewShipMinScreenSize",   scriptSetReal32CB, &pingNewShipMinScreenSize },

    { "pingBobDensityLow",      scriptSetReal32CB, &pingBattleBlobProperties.bobDensityLow },
    { "pingBobDensityHigh",     scriptSetReal32CB, &pingBattleBlobProperties.bobDensityHigh },
    { "pingBobStartSphereSize", scriptSetReal32CB, &pingBattleBlobProperties.bobStartSphereSize },
    { "pingBobRadiusCombineMargin", scriptSetReal32CB, &pingBattleBlobProperties.bobRadiusCombineMargin },
    { "pingBobOverlapFactor",   scriptSetBlobPropertyOverlap, &pingBattleBlobProperties },
    { "bobSmallestRadius",      scriptSetReal32CB, &pingBattleBlobProperties.bobSmallestRadius },
    { "pingBobBiggestRadius",   scriptSetBlobBiggestRadius, &pingBattleBlobProperties },
//    { "pingBobDoingCollisionBobs", scriptSetBool, &pingBattleBlobProperties.bobDoingCollisionBobs },

    { "pingTOLingerTime",       scriptSetReal32CB, &pingTOLingerTime },
    endEntry
};

//TO stuff
struct
{
    strGamesMessages stringEnum;
    color *c;
    udword bitMask;
    real32 lastTimeDrawn;
}
pingTOList[PTO_NumberTOs] =
{
    {strPingTO0, &ProximitySensorBlipColor, PTOM_Proximity},
    {strPingTO1, &pingNewShipColor, PTOM_NewShips},
    {strPingTO2, &pingAnomalyColor, PTOM_Anomaly},
    {strPingTO3, &pingBattleColor, PTOM_Battle},
    {strPingTO4, &TW_HW_PING_COLOUR_OUT, PTOM_Hyperspace}
};

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : pingUpdateTask
    Description : Task to update and evaluate ping tasks
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void pingUpdateTask(void)
{
    static Node *thisNode, *nextNode;
    static ping *thisPing;

    taskYield(0);
    
#ifndef C_ONLY
    while (1)
#endif
    {
        taskStackSaveCond(0);

        thisNode = pingList.head;

        //... code to go here...

        while (thisNode != NULL)
        {                                                   //scan all pings
            nextNode = thisNode->next;
            thisPing = listGetStructOfNode(thisNode);
            if (universe.totaltimeelapsed - thisPing->lastEvaluateTime >= thisPing->evaluatePeriod)
            {                                               //if time to evaluate a ping
                thisPing->lastEvaluateTime = universe.totaltimeelapsed;
                if (thisPing->evaluate(thisPing, thisPing->userID, (char *)(thisPing + 1), FALSE))
                {                                           //did this ping just expire?
                    listDeleteNode(thisNode);               //kill it if so
                }
            }

            thisNode = nextNode;
        }
        taskStackRestoreCond();
        taskYield(0);
    }
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : pingStartup
    Description : Startup the ping module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingStartup(void)
{
    scriptSet(NULL, "ping.script", pingTweaks);
    pingAnomalyMinScreenSize = primScreenToGLScaleX(pingAnomalyMinScreenSize);
    pingNewShipMinScreenSize = primScreenToGLScaleX(pingNewShipMinScreenSize);
    pingBattleMinScreenSize = primScreenToGLScaleX(pingBattleMinScreenSize);
    pingTaskHandle = taskStart(pingUpdateTask, PNG_TaskPeriod, 0);
    listInit(&pingList);
    pingReset();
}

/*-----------------------------------------------------------------------------
    Name        : pingReset
    Description : Reset the ping module.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingReset(void)
{
    sdword index;

    for (index = 0; index < PTO_NumberTOs; index++)
    {                                                       //no ping legends yet
        pingTOList[index].lastTimeDrawn = REALlyNegative;
    }
}

/*-----------------------------------------------------------------------------
    Name        : pingShutdown
    Description : Shut down the ping module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingShutdown(void)
{
    taskStop(pingTaskHandle);
}

/*-----------------------------------------------------------------------------
    Name        : pingCreate
    Description : Create a ping
    Inputs      : loc - location to create the ping at.  Ping will not move
                  owner - optional object about which to postion a ping.
                        Ping will move with the object.  May be NULL.
                  evaluate - function to evaluate the ping.
                  userDataSize - extra memory to allocate for user data, such
                        as a ship list.
                  userID - number to pass back to evaluation function.
    Outputs     : userData - pointer to extra memory as allocated with the
                    ping structure.
    Return      : Pointer to newly allocated ping structure.
----------------------------------------------------------------------------*/
ping *pingCreate(vector *loc, SpaceObj *owner, pingeval evaluate, ubyte **userData, sdword userDataSize, udword userID)
{
    ping *newPing;

    newPing = memAlloc(sizeof(ping) + userDataSize, "Ping!", Pyrophoric);

    listAddNode(&pingList, &newPing->link, newPing);
    newPing->c = colWhite;
    newPing->size = 1.0f;
    newPing->minScreenSize = 10;
    newPing->minSize = 0.0f;
    if (owner == NULL)
    {
        newPing->centre = *loc;
    }
    else
    {
        newPing->centre = owner->posinfo.position;
    }
    newPing->owner = owner;
    newPing->creationTime = universe.totaltimeelapsed;
    newPing->pingDuration = 1.0f;
    newPing->interPingPause = 0.0f;
    newPing->evaluatePeriod = PNG_TaskPeriod;
    newPing->lastEvaluateTime = universe.totaltimeelapsed;
    newPing->evaluate = evaluate;
    newPing->userDataSize = userDataSize;
    newPing->userID = userID;
    newPing->TOMask = 0;
    if (userData != NULL)
    {
        *userData = (ubyte *)(newPing + 1);
    }
    return(newPing);
}

/*-----------------------------------------------------------------------------
    Name        : pingObjectDied
    Description : Called when an object dies, removes references of said object
                    from all ping structures.
    Inputs      : obj - object that just died
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pingObjectDied(SpaceObj *obj)
{
    Node *thisNode, *nextNode;
    ping *thisPing;

    pingDyingObject = obj;

    thisNode = pingList.head;

    while (thisNode != NULL)
    {                                                       //scan all pings
        nextNode = thisNode->next;
        thisPing = listGetStructOfNode(thisNode);
        if (thisPing->owner == obj)
        {                                                   //if this ping centered on object
            thisPing->centre = obj->posinfo.position;
            thisPing->owner = NULL;                         //remove object from reference
        }
        if (thisPing->userID != 0 || thisPing->userDataSize != 0)
        {                                                   //see if there's anything in the user data
            if (thisPing->evaluate(thisPing, thisPing->userID, (char *)(thisPing + 1), TRUE))
            {                                               //did this ping just expire?
                listDeleteNode(thisNode);                   //kill it if it did
            }
        }

        thisNode = nextNode;
    }
}

/*-----------------------------------------------------------------------------
    Name        : pingFindByFunction
    Description : Find a ping in the ping list by function
    Inputs      : evalueate - the evaluation function of the ping to find
    Outputs     :
    Return      : First ping in list with that evaluate function or NULL if none
----------------------------------------------------------------------------*/
ping *pingFindByFunction(pingeval evaluate)
{
    Node *thisNode;
    ping *thisPing;

    thisNode = pingList.head;

    while (thisNode != NULL)
    {                                                       //scan all pings
        thisPing = listGetStructOfNode(thisNode);

        if (thisPing->evaluate == evaluate)
        {
            return(thisPing);
        }
        thisNode = thisNode->next;;
    }
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : pingListSortCallback
    Description : Merge sort callback for sorting the ping list by distance
                    from the camera.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool pingListSortCallback(void *firststruct,void *secondstruct)
{
    if (((ping *)(firststruct))->cameraDistanceSquared > ((ping *)(secondstruct))->cameraDistanceSquared)
    {
        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : pingListDraw
    Description : Draw all pings from farthest to nearest.
    Inputs      : camera - the camera we're rendering from
                  modelView, projection - current matrices
                  viewPort - rectangle we're rending in, for the TO legend
    Outputs     :
    Return      :
    Note        : The renderer should be in 2D mode at this point.
----------------------------------------------------------------------------*/
void pingListDraw(Camera *camera, hmatrix *modelView, hmatrix *projection, rectangle *viewPort)
{
    real32 pingAge, pingCycle, pingMod, pingSize;
    real32 x, y, radius;
    Node *thisNode, *nextNode;
    ping *thisPing;
    vector distSquared;
    sdword nSegments, index, rowHeight, xScreen, yScreen;
    oval o;
    udword TOFlags = 0;
    fonthandle fhSave;
    toicon *icon;
    color col;
    real32 realMargin;
    ShipClass shipClass;
    static real32 lastProximityPing = REALlyBig;
    static real32 lastAnomolyPing = REALlyBig;
    static real32 lastBattlePing = REALlyBig;
    static real32 lastHyperspacePing = REALlyBig;
    static real32 lastNewshipPing = REALlyBig;
    bool pingset;

    //start by sorting the ping list from farthest to nearest
    thisNode = pingList.head;

    while (thisNode != NULL)
    {                                                       //scan all pings
        nextNode = thisNode->next;
        thisPing = listGetStructOfNode(thisNode);

        if (thisPing->owner != NULL)
        {
            thisPing->centre = thisPing->owner->posinfo.position;
        }
        vecSub(distSquared, camera->eyeposition, thisPing->centre);
        thisPing->cameraDistanceSquared = vecMagnitudeSquared(distSquared);
        TOFlags |= thisPing->TOMask;

        thisNode = nextNode;
    }
    listMergeSortGeneral(&pingList, pingListSortCallback);

    //now the list is sorted; proceed to draw all the pings
    thisNode = pingList.head;

    pingset = FALSE;

    while (thisNode != NULL)
    {                                                       //scan all pings
        nextNode = thisNode->next;
        thisPing = listGetStructOfNode(thisNode);

        pingCycle = thisPing->pingDuration + thisPing->interPingPause;
        pingAge = universe.totaltimeelapsed - thisPing->creationTime;
        pingMod = (real32)fmod((double)pingAge, (double)pingCycle);
        if (pingMod <= thisPing->pingDuration)
        {
            pingSize = (thisPing->size - thisPing->minSize) * pingMod / thisPing->pingDuration + thisPing->minSize;
            selCircleComputeGeneral(modelView, projection, &thisPing->centre, max(thisPing->size,thisPing->minSize), &x, &y, &radius);
            if (radius > 0.0f)
            {
                radius = max(radius, thisPing->minScreenSize);
                radius *= pingSize / max(thisPing->size,thisPing->minSize);
                o.centreX = primGLToScreenX(x);
                o.centreY = primGLToScreenY(y);
                o.radiusX = o.radiusY = primGLToScreenScaleX(radius);
                nSegments = pieCircleSegmentsCompute(radius);
                primOvalArcOutline2(&o, 0.0f, 2*PI, 1, nSegments, thisPing->c);

                /* starting to draw a new ping so play the sound */
                if (!smZoomingIn && !smZoomingOut && !pingset)
                {
                    switch (thisPing->TOMask)
                    {
                        case PTOM_Anomaly:
                            if (pingSize <=lastAnomolyPing)
                            {
                                soundEvent(NULL, UI_SensorsPing);
                                pingset = TRUE;
                                lastAnomolyPing = pingSize;
                            }
                            break;
                        case PTOM_Battle:
                            if (pingSize <= lastBattlePing)
                            {
                                soundEvent(NULL, UI_PingBattle);
                                pingset = TRUE;
                                lastBattlePing = pingSize;
                            }
                            break;
                        case PTOM_Hyperspace:
                            if (pingSize <=  lastHyperspacePing)
                            {
                                soundEvent(NULL, UI_PingHyperspace);
                                pingset = TRUE;
                                lastHyperspacePing = pingSize;
                            }
                            break;
                        case PTOM_Proximity:
                            if (pingSize <= lastProximityPing)
                            {
                                soundEvent(NULL, UI_PingProximity);
                                pingset = TRUE;
                                lastProximityPing = pingSize;
                            }
                            break;
                        case PTOM_NewShips:
                            if (pingSize <= lastNewshipPing)
                            {
                                soundEvent(NULL, UI_PingNewShips);
                                pingset = TRUE;
                                lastNewshipPing = pingSize;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        thisNode = nextNode;
    }

    //draw the blip TO
    if (smTacticalOverlay)
    {
        realMargin = primScreenToGLScaleX(viewPort->x0);
        fhSave = fontCurrentGet();                          //save the current font
        fontMakeCurrent(selGroupFont2);                     // use a common, fairly small font
        rowHeight = fontHeight("M");                        // used to space the legend
        yScreen = viewPort->y0 + rowHeight;                 //leave some space at the top to start
        radius = primScreenToGLScaleX(rowHeight)/2;
        xScreen = viewPort->x0 + (sdword)(rowHeight * 2.5);

        for (index = 0; index < PTO_NumberTOs; index++)
        {
            if ((TOFlags & pingTOList[index].bitMask))
            {
    //            fontPrint(xScreen, yScreen, *pingTOList[index].c, "O");
                pingTOList[index].lastTimeDrawn = universe.totaltimeelapsed;
            }
            if (universe.totaltimeelapsed - pingTOList[index].lastTimeDrawn <= pingTOLingerTime)
            {
                o.centreX = viewPort->x0 + rowHeight * 3 / 2;
                o.centreY = yScreen + rowHeight / 2;
                o.radiusX = o.radiusY = rowHeight / 2;
                primOvalArcOutline2(&o, 0.0f, TWOPI, 1, pingTONSegments, *pingTOList[index].c);
                fontPrint(xScreen, yScreen, TO_TextColor, strGetString(pingTOList[index].stringEnum));
                yScreen += rowHeight + 1;
            }
        }
        for (shipClass = 0; shipClass < NUM_CLASSES; shipClass++)
        {
            if (!toClassUsed[shipClass][0])
            {
                continue;
            }
            icon = toClassIcon[shipClass];
            fontPrint(xScreen, yScreen + (rowHeight>>2), TO_TextColor, ShipClassToNiceStr(shipClass));
#if TO_STANDARD_COLORS
            col = teFriendlyColor;
#else
            col = teColorSchemes[universe.curPlayerIndex].tacticalColor;
#endif
            col = colRGB(colRed(col)/TO_IconColorFade, colGreen(col)/TO_IconColorFade, colBlue(col)/TO_IconColorFade);
            primLineLoopStart2(1, col);

            for (index = icon->nPoints - 1; index >= 0; index--)
            {
               primLineLoopPoint3F(realMargin + primScreenToGLX(rowHeight*1.5) + icon->loc[index].x * radius,
                                          primScreenToGLY(yScreen + rowHeight/2) + icon->loc[index].y * radius);
            }
            primLineLoopEnd2();
            yScreen += rowHeight + 1;
        }

        fontMakeCurrent(fhSave);
    }
}

/*-----------------------------------------------------------------------------
    Name        : pingAnomalyPingTimeout
    Description : Ping callback to evaluate if the ping has timed out.
    Inputs      : hellaPing - the ping we're evaluating
                  userID - 0
                  userData - string name of anomaly ping
                  bRemoveReferneces - ignore
    Outputs     :
    Return      : TRUE if the ping times out.
----------------------------------------------------------------------------*/
bool pingAnomalyPingTimeout(struct ping *hellaPing, udword userID, char *userData, bool bRemoveReferences)
{
    SelectCommand *selection;
    real32 dummySize;

    selection = (SelectCommand *)(hellaPing + 1);
    if (bRemoveReferences)
    {
        if (hellaPing->owner == pingDyingObject)
        {                                                   //if this ping's object is dying
            return(TRUE);                                   //kill it
        }
        if (selection->numShips > 0)
        {                                                   //if there's a real selection here
            selSelectionRemoveSingleShip((MaxSelection *)selection, (Ship *)pingDyingObject);
            if (selection->numShips == 0)
            {                                               //if no ships left
                return(TRUE);                               //kill the ping
            }
        }
        return(FALSE);                                      //not dead yet!
    }
    /*
    if (userID != (udword)hellaPing->owner)
    {                                                       //if the pingy object died
        return(TRUE);                                       //kill the ping
    }
    */
    if (selection->numShips > 0)
    {                                                       //if this ping has a selection
        hellaPing->centre = selCentrePointComputeGeneral((MaxSelection *)selection, &dummySize);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : pingAnomalySelectionPingAdd
    Description : Create an anomaly ping around a whole bunch of ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingAnomalySelectionPingAdd(char *pingName, SelectCommand *selection)
{
    ubyte *userData;
    ping *newPing;
    sdword dataSize;
    vector position;
    real32 dummySize;

    position = selCentrePointComputeGeneral((MaxSelection *)selection, &dummySize);
    dataSize = strlen(pingName) + 1 + selSelectionSize(selection->numShips);
    newPing = pingCreate(&position, NULL, pingAnomalyPingTimeout, &userData, dataSize, 0);
    selSelectionCopy((MaxAnySelection *)userData, (MaxAnySelection *)selection);//copy the ship selection to the ping
    strcpy((char *)userData + selSelectionSize(selection->numShips), pingName);//set the name
    newPing->c              = pingAnomalyColor;
    newPing->size           = pingAnomalySize;
    newPing->minSize        = pingAnomalyMinSize;
    newPing->pingDuration   = pingAnomalyPingDuration;
    newPing->interPingPause = pingAnomalyInterPingPause;
    newPing->evaluatePeriod = pingAnomalyEvaluatePeriod;
    newPing->TOMask         = PTOM_Anomaly;
    newPing->minScreenSize  = pingAnomalyMinScreenSize;
}

/*-----------------------------------------------------------------------------
    Name        : pingAnomalyObjectPingAdd
    Description : Create an anomaly ping about an object
    Inputs      : pingName - name of a ping
                  owner - object about which the ping in centred
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pingAnomalyObjectPingAdd(char *pingName, SpaceObj *owner)
{
    ubyte *userData;
    ping *newPing;
    sdword dataSize, zero = 0;

    dataSize = strlen(pingName) + 1 + selSelectionSize(zero);
    newPing = pingCreate(NULL, owner, pingAnomalyPingTimeout, &userData, dataSize, (udword)owner);
    ((SelectCommand *)userData)->numShips = 0;              //dummy selection
    strcpy((char *)userData + selSelectionSize(zero), pingName);       //set the name
    newPing->c              = pingAnomalyColor;
    newPing->size           = pingAnomalySize;
    newPing->minSize        = pingAnomalyMinSize;
    newPing->pingDuration   = pingAnomalyPingDuration;
    newPing->interPingPause = pingAnomalyInterPingPause;
    newPing->evaluatePeriod = pingAnomalyEvaluatePeriod;
    newPing->TOMask         = PTOM_Anomaly;
    newPing->minScreenSize  = pingAnomalyMinScreenSize;
}

/*-----------------------------------------------------------------------------
    Name        : pingAnolalyPositionPingAdd
    Description : Create an anomaly ping about an arbitrary position
    Inputs      : pingName - name of the ping
                  position - location of the ping
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pingAnomalyPositionPingAdd(char *pingName, vector *position)
{
    ubyte *userData;
    ping *newPing;
    sdword dataSize, zero = 0;

    dataSize = strlen(pingName) + 1 + selSelectionSize(zero);
    newPing = pingCreate(position, NULL, pingAnomalyPingTimeout, &userData, dataSize, 0);
    ((SelectCommand *)userData)->numShips = 0;              //dummy selection
    strcpy((char *)userData + selSelectionSize(zero), pingName);       //set the name
    newPing->c              = pingAnomalyColor;
    newPing->size           = pingAnomalySize;
    newPing->minSize        = pingAnomalyMinSize;
    newPing->pingDuration   = pingAnomalyPingDuration;
    newPing->interPingPause = pingAnomalyInterPingPause;
    newPing->evaluatePeriod = pingAnomalyEvaluatePeriod;
    newPing->TOMask         = PTOM_Anomaly;
    newPing->minScreenSize  = pingAnomalyMinScreenSize;
}

/*-----------------------------------------------------------------------------
    Name        : pingAnomalyPingRemove
    Description : Remove all pings of a given name from the ping list
    Inputs      :
    Outputs     :
    Return      : number of pings removed
----------------------------------------------------------------------------*/
sdword pingAnomalyPingRemove(char *pingName)
{
    Node *thisNode, *nextNode;
    ping *thisPing;
    char *name;
    SelectCommand *selection;
    sdword nRemoved = 0;

    thisNode = pingList.head;

    while (thisNode != NULL)
    {                                                       //scan all pings
        nextNode = thisNode->next;
        thisPing = listGetStructOfNode(thisNode);
        if (thisPing->evaluate == pingAnomalyPingTimeout)
        {
            selection = (SelectCommand *)(thisPing + 1);
            name = (char *)selection + selSelectionSize(selection->numShips );
            if (!strcmp(pingName, name))
            {                                               //if names match
                listDeleteNode(thisNode);                   //kill it if it did
                nRemoved++;                                 //inc number removed
            }
        }

        thisNode = nextNode;
    }
    return(nRemoved);
}

/*-----------------------------------------------------------------------------
    Name        : pingRemovePingByOwner
    Description : removes a ping based on its owner
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingRemovePingByOwner(SpaceObj *owner)
{
    Node *node = pingList.head;
    ping *thisPing;

    while (node != NULL)
    {
        thisPing = listGetStructOfNode(node);

        if (thisPing->owner == owner)
        {
            listDeleteNode(node);
            return;
        }

        node = node->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : pingAllPingsDelete
    Description : Deletes all pings.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingAllPingsDelete(void)
{
    listDeleteAll(&pingList);
}

/*-----------------------------------------------------------------------------
    Name        : pingBattleBlobCriteria
    Description : sub-blob creation callback to see if an object is a ship in a battle
    Inputs      : superBlob - what blob we're searching through
                  obj - what object we're currently processing
    Outputs     :
    Return      : TRUE if it's a player ship in a battle
----------------------------------------------------------------------------*/
bool pingBattleBlobCriteria(blob *superBlob, SpaceObj *obj)
{
    if (obj->objtype != OBJ_ShipType)
    {
        return(FALSE);                                      //not a ship - no good
    }
//    if (!allianceIsShipAlly((Ship *)obj, universe.curPlayerPtr))
//    {
        //!!! should we consider alliances here?
//        return(FALSE);                                      //does not belong to the player
//    }

    if (obj->flags & (SOF_Dead|SOF_Hide|SOF_Crazy|SOF_Disabled))
    {
        return(FALSE);
    }

    if ((((Ship *)obj)->recentlyAttacked && ((Ship *)obj)->recentAttacker->playerowner != universe.curPlayerPtr) ||
        (((Ship *)obj)->shipisattacking))
    {
        if (!bitTest(smShipTypeRenderFlags[((Ship *)obj)->shiptype], SM_Exclude))
        {                                                   //if we should include in battle blobs
            return(TRUE);                                   //it's a player ship in a battle - good!
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : pingBattlePingEvaluate
    Description : Called when a battle ping times out
    Inputs      : ignored
    Outputs     : Battle sound code will be performed here.
    Return      : TRUE, or maybe false, depending on the mood
----------------------------------------------------------------------------*/
bool pingBattlePingEvaluate(struct ping *hellaPing, udword userID, char *userData, bool bRemoveReferences)
{
    sdword index;
    Ship **thisShip;
    battleping *battlePing;

    battlePing = (battleping *)(hellaPing + 1);

    if (bRemoveReferences == FALSE)
    {                                                       //ping is being evaluated
        if (battlePing->nEnemyShips == 0)
        {                                                   //no enemies - we must have just won the battle
            if (battlePing->nFriendlyShips == 0)
            {                                               //no friendlies or enemies
                return(TRUE);                               //there's no battle going on anymore, delete this ping
            }
            else
            {
                battlePingEvaluateNoEnemies(hellaPing, battlePing);
            }
        }
        else if (battlePing->nFriendlyShips == 0)
        {                                                   // no friendlies - could be friendly fire
            battlePingEvaluateNoFriendlies(hellaPing, battlePing);
        }
        else
        {
            battlePingEvaluate(hellaPing, battlePing);      //evaluate the ping normally, maybe saying something
        }
        return(TRUE);                                       //time out on first evaluation
        //!!!! try a few times here.  This could be an efficiency issue
    }
                                                            //else an object just died
    if (pingDyingObject->objtype != OBJ_ShipType)
    {
        return(FALSE);                                      //only ships in battle pings
    }
    for (index = 0, thisShip = battlePing->ship; index < battlePing->nShips; index++, thisShip++)
    {
        if (*thisShip == (Ship *)pingDyingObject)
        {                                                   //if this is the object that is dying
            battleAccountForLosses(*thisShip, battlePing);  //account for this damage on computation of future BSN's
            if (allianceIsShipAlly(*thisShip, universe.curPlayerPtr))
            {                                               //we're losin' em here
                battlePing->nFriendlyShips--;
            }
            else
            {
                battlePing->nEnemyShips--;                  //we're beatin' em
            }
            battlePing->nShips--;                            //decrement total number of ships
            if (index < battlePing->nShips)
            {                                               //copy last entry to current entry (fill in this space)
                *thisShip = battlePing->ship[battlePing->nShips];
            }
            break;                                          //done searching for ship
        }
    }
    dbgAssert(battlePing->nFriendlyShips + battlePing->nEnemyShips == battlePing->nShips);
    dbgAssert(battlePing->nShips >= 0);
    if (battlePing->nShips == 0)
    {                                                       //all ships removed, kill the ping
        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : pingAttackPingsCreate
    Description : Create a bunch of attack pings in a given blob by creating
                    a list of sub-blobs.
    Inputs      : superBlob - blob to search for battles within.
    Outputs     :
    Return      :
    Note!!!     : no pings will be created if any battle pings are presently going on
----------------------------------------------------------------------------*/
void pingAttackPingsCreate(blob *superBlob)
{
    LinkedList battleBlobs;
    Node *thisNode;
    blob *thisBlob;
    battleping *battlePing;
    ping *newPing;
    static sdword nTotalShips;
    CommandToDo *command;
    Ship **ship, **commandShip, **baseShip;
    sdword index, commandIndex, baseIndex, startShips;

    bobSubBlobListCreate(&pingBattleBlobProperties, &battleBlobs, superBlob, pingBattleBlobCriteria);

    thisNode = battleBlobs.head;

    while (thisNode != NULL)
    {
        thisBlob = listGetStructOfNode(thisNode);

        //create the ping structure
        nTotalShips = superBlob->nCapitalShips + superBlob->nAttackShips + superBlob->nNonCombat + superBlob->nFriendlyShips;
        if (superBlob->blobShips->numShips > nTotalShips)
        {
            nTotalShips = superBlob->blobShips->numShips;
        }
        newPing = pingCreate(&thisBlob->centre, NULL, pingBattlePingEvaluate, (ubyte **)&battlePing, battlePingSize(nTotalShips), 0);
        newPing->c              = pingBattleColor;
//        newPing->size           = pingBattleSize;
        newPing->size           = thisBlob->radius;
        newPing->minSize        = pingBattleMinSize;
        newPing->pingDuration   = pingBattlePingDuration;
        newPing->interPingPause = pingBattleInterPingPause;
        newPing->evaluatePeriod = pingBattleEvaluatePeriod;
        newPing->TOMask         = PTOM_Battle;
        newPing->minScreenSize  = pingBattleMinScreenSize;

        //create the battleping structure
        battlePing->nEnemyShips = thisBlob->nCapitalShips + thisBlob->nAttackShips + thisBlob->nNonCombat;
        battlePing->nFriendlyShips = thisBlob->nFriendlyShips;
        battlePing->nShips = battlePing->nEnemyShips + battlePing->nFriendlyShips;
        battlePing->radius = thisBlob->radius;
        battlePing->friendlyFirepowerRecentlyLost =         //no recent losses just yet
            battlePing->friendlyValueRecentlyLost =
            battlePing->enemyFirepowerRecentlyLost =
            battlePing->enemyValueRecentlyLost = 0.0f;
        battlePing->nShips = thisBlob->blobObjects->numSpaceObjs;
        memcpy(battlePing->ship, thisBlob->blobObjects->SpaceObjPtr, battlePing->nShips * sizeof(SpaceObj *));

        //add all ships in the attack commands of all ships in the selection
        ship = battlePing->ship;
        startShips = battlePing->nShips;
        for (index = 0; index < battlePing->nShips; index++, ship++)
        {                                                   //for each ship in the ping
            dbgAssert((*ship)->objtype == OBJ_ShipType);
            dbgAssert((*ship)->playerowner != NULL);
            command = (*ship)->command;                     //get the ship's command

            if ((command != NULL) &&
                ((command->ordertype.order == COMMAND_ATTACK)))// ||
                 //(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING|COMMAND_IS_PASSIVEATTACKING)))
            {                                               //if it's a valid attack command
                if (universe.totaltimeelapsed != command->pingUpdateTime || battlePing != command->updatedPing)
                {                                           //if this command has not already been added to this ping
                    commandShip = (Ship **)command->attack->TargetPtr;
                    for (commandIndex = command->attack->numTargets; commandIndex > 0; commandIndex--, commandShip++)
                    {                                       //for each ship in the attack command
                        if (!pingBattleBlobCriteria(NULL, (SpaceObj *)*commandShip))
                        {                                   //if it would be rejected by blob criteria
                            continue;                       //then we don't want it
                        }
                        //see if this ship is already in the ping.  We don't want doubles.
                        baseShip = battlePing->ship;
                        for (baseIndex = 0; baseIndex < battlePing->nShips; baseIndex++, baseShip++)
                        {
                            if (*baseShip == *commandShip)
                            {                               //is this ship already here?
                                goto shipAlreadyInSelection;
                            }
                        }
                        //not found in the selection yet
                        if (battlePing->nShips < nTotalShips)
                        {                                   //if there is room for this ship in the ping
                            battlePing->ship[battlePing->nShips] = *commandShip;
                            if (allianceIsShipAlly(*commandShip, universe.curPlayerPtr))
                            {                               //was that a friendly or an enemy?
                                battlePing->nFriendlyShips++;
                            }
                            else
                            {
                                battlePing->nEnemyShips++;
                            }
                            battlePing->nShips++;           //another ship
                        }
#if PNG_VERBOSE_LEVEL >= 2
                        else
                        {
                            dbgMessagef("\nShip 0x%x tossed out of ping with %d ships.", commandShip, nTotalShips);
                        }
#endif
shipAlreadyInSelection:;
                    }
                    command->pingUpdateTime = universe.totaltimeelapsed;
                    command->updatedPing = battlePing;      //flag this command as already processed
                }
            }
        }
        dbgAssert(battlePing->nEnemyShips + battlePing->nFriendlyShips == battlePing->nShips);
        thisNode = thisNode->next;
    }
    bobListDelete(&battleBlobs);
//    listDeleteAll(&battleBlobs);
}

/*-----------------------------------------------------------------------------
    Name        : pingNewShipCallback
    Description : Ping evalueation callback for new ship pings
    Inputs      : all ignored so I'm not going to bother explaining it
    Outputs     :
    Return      : TRUE because this function should only be called when the
                    ping times out
----------------------------------------------------------------------------*/
bool pingNewShipCallback(struct ping *hellaPing, udword userID, char *userData, bool bRemoveReferences)
{
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : pingNewShipPingCreate
    Description : Create a ping at the location of new ship creation.
    Inputs      : position - position of new ship
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pingNewShipPingCreate(vector *position)
{
    ping *newPing;
    ubyte *userData;

    newPing = pingCreate(position, NULL, pingNewShipCallback, &userData, 0, 0);
    newPing->c              = pingNewShipColor;
    newPing->size           = pingNewShipSize;
    newPing->minSize        = pingNewShipMinSize;
    newPing->pingDuration   = pingNewShipPingDuration;
    newPing->interPingPause = pingNewShipInterPingPause;
    newPing->evaluatePeriod = pingNewShipEvaluatePeriod;
    newPing->TOMask         = PTOM_NewShips;
    newPing->minScreenSize  = pingNewShipMinScreenSize;
}

/*-----------------------------------------------------------------------------
    Name        : pingBattlePingsCreate
    Description : Create a list of attack pings from a blob list
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingBattlePingsCreate(LinkedList *blobList)
{
    Node *thisNode;
    blob *thisBlob;

    if (pingFindByFunction(pingBattlePingEvaluate) != NULL)
    {                                                       //if there are already battle pings active
        return;                                             //don't create any new ones
    }
    thisNode = blobList->head;

    while (thisNode != NULL)
    {
        thisBlob = listGetStructOfNode(thisNode);

        if (thisBlob->flags & BTM_PieceOfTheAction)
        {                                                   //if there's action in this blob
            pingAttackPingsCreate(thisBlob);
        }
        thisNode = thisNode->next;
    }
}

/*=============================================================================
    Save Game Stuff
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void SaveAnomalyPing(ping *tping)
{
    SaveChunk *chunk;
    sdword size = sizeofping(tping);
    ping *savecontents;

    chunk = CreateChunk(VARIABLE_STRUCTURE|SAVE_PING,size,tping);
    savecontents = (ping *)chunkContents(chunk);

    savecontents->owner = (SpaceObj *)SpaceObjRegistryGetID(tping->owner);
    savecontents->userID = SpaceObjRegistryGetID((SpaceObj *)tping->userID);

    SaveThisChunk(chunk);
    memFree(chunk);

    if (tping->userDataSize > 0)
    {
        SaveSelection((SpaceObjSelection *) (tping + 1));
    }
}

ping *LoadAndFixAnomalyPing(void)
{
    SaveChunk *chunk;
    ping *newping;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE|SAVE_PING);

    size = sizeofping(((ping *)chunkContents(chunk)));

    dbgAssert(size == chunk->contentsSize);

    newping = memAlloc(size,"Ping!",0);
    memcpy(newping,chunkContents(chunk),size);

    memFree(chunk);

    newping->owner = SpaceObjRegistryGetObj((sdword)newping->owner);
    newping->userID = (sdword)SpaceObjRegistryGetObj((sdword)newping->userID);
    newping->evaluate = pingAnomalyPingTimeout;

    if (newping->userDataSize > 0)
    {
        SelectAnyCommand *tempsel = (SelectAnyCommand *)LoadSelectionAndFix();
        memcpy((newping + 1),tempsel,sizeofSelectCommand(tempsel->numTargets));
        memFree(tempsel);
    }

    return newping;
}

void pingSave(void)
{
    Node *node;
    ping *tping;
    sdword num = 0;

    for (node = pingList.head; node != NULL; node = node->next)
    {
        tping = listGetStructOfNode(node);
        if (tping->evaluate == pingAnomalyPingTimeout)
        {
            num++;
        }
    }

    SaveInfoNumber(num);

    for (node = pingList.head; node != NULL; node = node->next)
    {
        tping = listGetStructOfNode(node);
        if (tping->evaluate == pingAnomalyPingTimeout)
        {
            SaveAnomalyPing(tping);
        }
    }
}

void pingLoad(void)
{
    sdword i;
    sdword num;
    ping *tping;

    num = LoadInfoNumber();

    dbgAssert(pingList.num == 0);

    for (i=0;i<num;i++)
    {
        tping = LoadAndFixAnomalyPing();
        listAddNode(&pingList, &tping->link, tping);
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

/*=============================================================================
    End of Save Game Stuff
=============================================================================*/

