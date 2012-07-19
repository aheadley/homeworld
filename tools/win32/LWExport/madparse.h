/*=============================================================================
    Name    : MADParse.h
    Purpose : Definitions for creating mesh animations (.MAD) files.

    Created 7/17/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MAD_H
#define ___MAD_H    "fuming mad"

/*=============================================================================
    Definitions:
=============================================================================*/

#define MAD_FileVersion         0.91f
#define MAD_FileIdentifier      "Mellonn"
#define MAD_NumberAnimations    256
#define MAD_NumberObjects       256

#define MAF_Loop                0x00000001
#define MAD_SkipThisOne         0x00000100

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure for an animation header
typedef struct
{
    unsigned long name;                         //pointer to name
    float startTime;                            //time of start/end
    float endTime;
    unsigned long flags;                        //flags for the animation
}
madanimation;

//structure for a keyframe list for an animation for a single object
typedef struct
{
    unsigned long name;                         //pointer to name of object
    unsigned short nameCRC;                     //16-bit crc of the name.  (should upgrade to 32 later).
    unsigned char pad[2];
    unsigned long animationBits;                //bit-flags: one for each animation
    unsigned long nKeyframes;                   //number of keyframes for this object
    unsigned long times;                        //times of the keyframes
    unsigned long parameters;                   //tension, continuity, bias parameter structures
    unsigned long path[6];                      //x,y,z,h,p,b
}
madobjpath;

//structure for the header of these crazy files, not including animation
//header structures
typedef struct
{
    char identifier[8];
    float version;
    unsigned long stringBlockLength;
    unsigned long stringBlock;
    float length;                               //total length of all animations
    float framesPerSecond;                      //rate the animation was scripted at
    unsigned long nObjects;                     //number of objects in animation
    unsigned long objPath;                      //pointer to animations
    unsigned long nAnimations;                  //number of different animations
}
madheader;

//structure for the header of these crazy files, including animation
//header structures
typedef struct
{
    char identifier[8];
    float version;
    unsigned long stringBlockLength;
    unsigned long stringBlock;
    float length;                               //total length of all animations
    float framesPerSecond;                      //rate the animation was scripted at
    unsigned long nObjects;                     //number of objects in animation
    unsigned long objPath;                      //pointer to animations
    unsigned long nAnimations;                  //number of different animations
    madanimation anim[MAD_NumberAnimations];    //lots of animations
}
madmaxheader;

/*=============================================================================
    Macros:
=============================================================================*/
#define madHeaderSize(n)  (sizeof(madheader) + sizeof(madanimation) * (n))

/*=============================================================================
    Functions:
=============================================================================*/
signed char MADParse_WriteFile(FILE *fileHandle);
void InitializeMADParse(void);

#endif //___MAD_H

