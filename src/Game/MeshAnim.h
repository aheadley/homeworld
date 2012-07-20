/*=============================================================================
    Name    : MeshAnim.h
    Purpose : Definitions for scripted mesh animations.

    Created 7/21/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MESHANIM_H
#define ___MESHANIM_H           "steaming mad"

#include "Types.h"
#include "B-Spline.h"
#include "CRC32.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define MAD_TEST_ANIMATION      0               //test animations
#ifndef HW_Release

#define MAD_ERROR_CHECKING      1               //basic error checking
#define MAD_VERBOSE_LEVEL       1               //control verbose printing

#else //HW_Debug

#define MAD_ERROR_CHECKING      0               //no error ckecking in retail
#define MAD_VERBOSE_LEVEL       0               //don't print any verbose strings in retail

#endif //HW_Debug


/*=============================================================================
    Definitions:
=============================================================================*/
#define MAD_FileVersion         0.91f
#define MAD_FileIdentifier      "Mellonn"
#define MAD_Extension           ".MAD"
#define MAD_MaxAnimations       32

//animation flags
#define MAF_Loop                0x00000001      //this animation will loop
#define MAD_NoAnimation         -1              //nCurrentAnim will be this if no animation playing

/*=============================================================================
    Type definitions
=============================================================================*/
//structure for a single object's keyframe list in the file.
typedef struct
{
    char *name;                                 //name of the object for this keyframe list
    crc16 nameCRC;                              //16-bit crc of the name.  (should upgrade to 32 later).
    ubyte pad[2];
    udword animationBits;                       //bit-flags: one for each animation
    sdword nKeyframes;                          //number of keyframes in all of file for this object
    real32 *times;                              //times array
    tcb *parameters;                            //tension, continuity, bias
    real32 *path[6];                            //arrays of x,y,z,h,p,b
}
madobjpath;

//structure for a single animation within the file
typedef struct
{
    char *name;                                 //name of the animation
    real32 startTime;                           //limits of the animation
    real32 endTime;
    udword flags;                               //flags for this animation
}
madanimation;

//structure for the header of a mesh animation file
typedef struct
{
    char identifier[8];                         //compared to MAD_FileIdentifier
    real32 version;                             //compared to MAD_FileVersion
    sdword stringBlockLength;                   //length of string block
    char *stringBlock;                          //string block: where all strings for this file are located and offsets are relative to
    real32 length;                              //length of all animations
    real32 framesPerSecond;                     //rate the animation was scripted at
    sdword nObjects;                            //number of objects in the animation (should be same as number of objects in the mesh to animate)
    madobjpath *objPath;                        //list of keyframe lists for each object
    sdword nAnimations;                         //number of separate animations in the file
    madanimation anim[1];                       //ragged array of animation headers
}
madheader;

//structure for animation data in shipstaticinfo
typedef struct
{
    madheader *header;                          //animation data loaded from disk
    sdword needStartPlacedAnimation;                   //True or false flag indicating the object possesing these animations needs a starting animation/pause hack (thats right luke, a hack!)
    sdword startPlacedAnimationIndex;                 //if above flag is true, this index is valid and points to that starting animation
    sdword needStartBuiltAnimation;                   //True or false flag indicating the object possesing these animations needs a starting animation/pause hack (thats right luke, a hack!)
    sdword startBuiltAnimationIndex;                 //if above flag is true, this index is valid and points to that starting animation
    //extra context sensitive animation data
    sdword numGunOpenIndexes;                   //number of below indices
    sdword numGunOpenDamagedIndexes;                   //number of below indices
    sdword numGunCloseIndexes;                   //number of below indices
    sdword numGunCloseDamagedIndexes;                   //number of below indices
    sdword *gunOpenIndexes;                      //arrays for gunopening/closing animation indexes both damaged and undamaged.  So we can support a number of different animationsfor the same ship
    sdword *gunOpenDamagedIndexes;
    sdword *gunCloseIndexes;
    sdword *gunCloseDamagedIndexes;
    //same, but for docking animations
    sdword numDockIndexes;                   //number of below indices
    sdword numDockDamagedIndexes;                   //number of below indices
    sdword numPostDockIndexes;                   //number of below indices
    sdword numPostDockDamagedIndexes;                   //number of below indices
    sdword *DockIndexes;                      //arrays for gunopening/closing animation indexes both damaged and undamaged.  So we can support a number of different animationsfor the same ship
    sdword *DockDamagedIndexes;
    sdword *PostDockIndexes;
    sdword *PostDockDamagedIndexes;
    sdword numDoorOpenIndexes;
    sdword numDoorCloseIndexes;
    sdword *DoorOpenIndexes;
    sdword *DoorCloseIndexes;
    sdword numSpecialOpenIndexes;
    sdword numSpecialCloseIndexes;
    sdword numSpecialOpenDamagedIndexes;
    sdword numSpecialCloseDamagedIndexes;
    sdword *specialOpenIndexes;
    sdword *specialCloseIndexes;
    sdword *specialOpenDamagedIndexes;
    sdword *specialCloseDamagedIndexes;
}
madstatic;

//structure of animation data stored within a ship
typedef struct
{
    madheader *header;                          //static animation data
    sdword totalSize;                           //size of this structure, bindings, motion curves and the whole bit
    udword flags;                               //flags for this animation
    sdword nCurrentAnim;                        //animation we're currently on, or MAD_NoAnimation for none
    bool   bPaused;                             //TRUE when the animation is paused
    real32 startTime;                           //time the animation starts at
    real32 time;                                //time within the animation, from starttime to endtime inclusive
    real32 timeElapsed;                         //time elapsed since last rendered
    splinecurve *curves;                        //spline curve array, enough for 6 * number of objects to animate
    shipbindings *saveBindings;                 //gun-mesh bindings are saved here when scripted animation is going on.
    shipbindings bindings;                      //animation bindings for this ship, must be last in list; variable size
}
madanim;

/*=============================================================================
    Macros:
=============================================================================*/
#define madHeaderSize(n)    ((sdword)sizeof(madheader) + (sdword)sizeof(madanimation) * ((n) - 1))
#define madAnimSize(n)      (sizeof(madanim) - sizeof(shipbindings) + shipBindingsLength(n))

/*=============================================================================
    Functions:
=============================================================================*/
struct Ship;
struct ShipStaticInfo;

madheader *madFileLoad(char *fileName);
void madHeaderDelete(madheader *header);

//make ship local bindings for mesh animation
void madAnimBindingsDupe(struct Ship *ship, struct ShipStaticInfo *staticInfo,bool LoadingGame);

//play an animation
void madAnimationStart(struct Ship *ship, sdword animNumber);
void madAnimationPause(struct Ship *ship, bool bFreeze);
bool madAnimationUpdate(struct Ship *ship, real32 timeElapsed);
void madAnimationStop(struct Ship *ship);

//find things in animations
sdword madAnimIndexFindByName(madheader *header, char *name);
sdword madGunBindingIndexFindByName(struct ShipStaticInfo *info, char *name);
sdword madBindingIndexFindByName(madheader *header, char *name);
void madAnimBindingMatrix(matrix *destMatrix, vector *destPos, struct Ship *ship, sdword gunIndex, sdword madIndex);

void PreFix_madBindings(struct Ship *ship,struct Ship *fixcontents);
void Save_madBindings(struct Ship *ship);
void Load_madBindings(struct Ship *ship);
void Fix_madBindings(struct Ship *ship);

#endif //___MESHANIM_H

