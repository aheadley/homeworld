/*=============================================================================
    Name    : Ping.h
    Purpose : Code for drawing and updating pings in the sensors manager.

    Created 9/8/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___PING_H
#define ___PING_H

#include "Types.h"
#include "LinkedList.h"
#include "SpaceObj.h"
#include "Blobs.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define PNG_ERROR_CHECKING      1               //basic error checking
#define PNG_VERBOSE_LEVEL       2               //control verbose printing

#else //HW_Debug

#define PNG_ERROR_CHECKING      0               //no error ckecking in retail
#define PNG_VERBOSE_LEVEL       0               //don't print any verbose strings in retail

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define PNG_TaskPeriod              0.5f           //evaluate the pings every this often.

#define PING_AnomalyColor           colWhite
#define PING_AnomalySize            1000.0f
#define PING_AnomalyMinSize         100.0f
#define PING_AnomalyPingDuration    1.0f
#define PING_AnomalyInterPingPause  0.25f
#define PING_AnomalyEvaluatePeriod  2.0f
#define PING_AnomalyMinScreenSize   0.0f

#define PING_BattleColor            colRGB(255, 0, 0)
#define PING_BattleSize             1000.0f
#define PING_BattleMinSize          100.0f
#define PING_BattlePingDuration     0.5f
#define PING_BattleInterPingPause   0.15f
#define PING_BattleEvaluatePeriod   2.0f
#define PING_BattleMinScreenSize    0.0f

#define PING_NewShipColor           colRGB(255, 0, 0)
#define PING_NewShipSize            1000.0f
#define PING_NewShipMinSize         100.0f
#define PING_NewShipPingDuration    0.5f
#define PING_NewShipInterPingPause  0.15f
#define PING_NewShipEvaluatePeriod  2.0f
#define PING_NewShipMinScreenSize   0.0f

//TO Masks
#define PTOM_Proximity              0x00000001
#define PTOM_NewShips               0x00000002
#define PTOM_Anomaly                0x00000004
#define PTOM_Battle                 0x00000008
#define PTOM_Hyperspace             0x00000010
#define PTO_NumberTOs               6

#define PING_TOMarginX              3
#define PING_TONSegments            12
#define PING_TOLingerTime           2.0f

/*=============================================================================
    Type definitions:
=============================================================================*/

struct ping;

typedef bool (*pingeval)(struct ping *hellaPing, udword userID, char *userData, bool bRemoveReferences);
//structure of a ping
typedef struct ping
{
    Node link;                                  //link into the ping list
    color c;                //default white     //color of the ping
    real32 size;            //default 1m        //spatial size of the ping (fully expanded)
    real32 minSize;         //default 0         //spatial size of ping at it's minimum
    real32 minScreenSize;   //default 10        //minimum screen radius to draw ping at (GL coords)
    vector centre;                              //location of the ping
    SpaceObj *owner;                            //option spaceobj about which the ping is centred
    real32 creationTime;                        //when was the ping created
    real32 pingDuration;    //default 1 sec     //duration over which the ping expands to max size.
    real32 interPingPause;  //default 0         //pause between adjacent pings
    real32 evaluatePeriod;  //default max       //time between ping evaluations
    real32 lastEvaluateTime;                    //last time ping was evaluated
    pingeval evaluate;                          //function to evaluate the expiry of the ping.
    sdword userDataSize;                        //size of the user data associated with this ping
    udword userID;                              //user ID to pass back to user function
    real32 cameraDistanceSquared;               //distance^2 from the camera
    udword TOMask;          //default 0         //TO Mask, if any
}
ping;

#define sizeofping(p) (sizeof(ping) + p->userDataSize)

/*=============================================================================
    Data:
=============================================================================*/
extern SpaceObj *pingDyingObject;               //object that is dying and must be evaluated

/*=============================================================================
    Functions:
=============================================================================*/
void pingStartup(void);
void pingShutdown(void);
void pingReset(void);

ping *pingCreate(vector *loc, SpaceObj *owner, pingeval evaluate, ubyte **userData, sdword userDataSize, udword userID);
void pingObjectDied(SpaceObj *obj);
void pingListDraw(Camera *camera, hmatrix *modelView, hmatrix *projection, rectangle *viewPort);

void pingAnomalySelectionPingAdd(char *pingName, SelectCommand *selection);
void pingAnomalyObjectPingAdd(char *pingName, SpaceObj *owner);
void pingAnomalyPositionPingAdd(char *pingName, vector *position);
sdword pingAnomalyPingRemove(char *pingName);
void pingAllPingsDelete(void);

void pingRemovePingByOwner(SpaceObj *owner);

void pingAttackPingsCreate(blob *superBlob);
void pingBattlePingsCreate(LinkedList *blobList);

void pingNewShipPingCreate(vector *position);

/*=============================================================================
    Save Game stuff
=============================================================================*/

void pingSave(void);
void pingLoad(void);

#endif //___PING_H

