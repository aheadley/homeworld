/*=============================================================================
    Name    : MADParse.cpp
    Purpose : Export mesha animation (.MAD) files

    Created 7/17/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#include "export.h"
#include "nisparse.h"
#include "lwsparse.h"
#include "lwoparse.h"
#include "cclist.h"
#include "raceDefs.h"
#include "madparse.h"

#define DEG_PER_RADIAN (360.0f/(2.0f*PI))
#define RADIAN_PER_DEG (2.0f*PI/360.0f)

#define DEG_TO_RAD(x) ((x) * RADIAN_PER_DEG)
/*=============================================================================
    Data:
=============================================================================*/
//string block where all strings are kept
sdword stringBlockLength;
char *stringBlock;

sdword fileLength;

//data stored globally, with enough room for very large and complex animations
madmaxheader gMadHeader;
madobjpath gObjPath[MAD_NumberObjects];

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : MADParse_AddString
    Description : adds a string to the string block to be written out.
    Inputs      : String to add.
    Outputs     :
    Return      : Offset into the string block for this string.
----------------------------------------------------------------------------*/
sdword MADParse_AddString(char *string)
{
    int length = strlen(string) + 1;
    stringBlock = (char *)realloc(stringBlock, stringBlockLength + length);
    assert(stringBlock != NULL);
    strcpy(stringBlock + stringBlockLength, string);
    stringBlockLength += length;
    return(stringBlockLength - length);
}

/*-----------------------------------------------------------------------------
    Name        : MADParse_WriteFile
    Description : Writes out the .MAD file for the current scene.
    Inputs      : filehandle - handle to be writing to
    Outputs     :
    Return      : success or failure
----------------------------------------------------------------------------*/
signed char MADParse_WriteFile(FILE *fileHandle)
{
    fileLength = sizeof(gMadHeader);
	LWObject *tObject, *tNULLObject;
	LWOContainer *pContainer, *tContainer, *pNULLContainer;
    LWOKeyframeContainer *pKeyFrameContainer;
    LWOKeyframe *pKeyFrame, *pNULLStartKeyframe, *pNULLEndKeyframe;
    unsigned long fileOffset, index, j, bitMask;
    unsigned long bStartKeyframe, bEndKeyframe;
    float value[3];
    sdword nFloats;

    //basic gMadHeader setup
    strcpy(gMadHeader.identifier, MAD_FileIdentifier);
    gMadHeader.version = MAD_FileVersion;

    gMadHeader.framesPerSecond = (float)gLWSData->framesPerSecond;
    //compute number of animations and set up the start/end times.
    gMadHeader.nAnimations = 0;
    gMadHeader.length = 0;
    pContainer = (LWOContainer *)gLWOData->pNULLList->GetHead();
	while(pContainer)
	{
		tContainer = pContainer;
		tObject = (LWObject *)&pContainer->myObject;
		
		pContainer = (LWOContainer *)pContainer->GetNext();

        assert(bitTest(tObject->flags, LWOF_NullObject));

        //found a NULL object, find the last 2 keyframes to figure out
        //start/end times this animation represents.
        if (tObject->objectMotions->GetNumElements() < 2)
        {                                                   //if not enough keyframes to delimit the animation
            continue;                                       //skip it
        }
        gMadHeader.anim[gMadHeader.nAnimations].name = MADParse_AddString(tObject->fileName);//add the filename to the string block
        pKeyFrameContainer = (LWOKeyframeContainer *)tObject->objectMotions->GetTail();  //get last keyframe
        pKeyFrame = &pKeyFrameContainer->myKeyframe;
        gMadHeader.anim[gMadHeader.nAnimations].endTime = pKeyFrame->frameNumber / gLWSData->framesPerSecond;
        pKeyFrameContainer = (LWOKeyframeContainer *)pKeyFrameContainer->GetPrev();                   //get second-to-last keyframe
        pKeyFrame = &pKeyFrameContainer->myKeyframe;
        gMadHeader.anim[gMadHeader.nAnimations].startTime = pKeyFrame->frameNumber / gLWSData->framesPerSecond;
        assert(gMadHeader.nAnimations < MAD_NumberAnimations);
        gMadHeader.length += (gMadHeader.anim[gMadHeader.nAnimations].endTime -//consider this animation as part of the total
                              gMadHeader.anim[gMadHeader.nAnimations].startTime);
        gMadHeader.anim[gMadHeader.nAnimations].flags = 0;
        if (strstr(tObject->fileName, "loop"))
        {
            bitSet(gMadHeader.anim[gMadHeader.nAnimations].flags, MAF_Loop);
        }
        gMadHeader.nAnimations++;                           //one new animation
	}
    //compute number of keyframes for each object and build list of object paths
    gMadHeader.objPath = madHeaderSize(gMadHeader.nAnimations);//object paths come right after the header and animation headers
    gMadHeader.nObjects = 0;

    //first pass: count number of keyframes for each object
    pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();
    while (pContainer)
    {
		tContainer = pContainer;
		tObject = (LWObject *)&pContainer->myObject;
		
		pContainer = (LWOContainer *)pContainer->GetNext();

        assert(!bitTest(tObject->flags, LWOF_NullObject));

        assert(gMadHeader.nObjects < MAD_NumberObjects);
        gObjPath[gMadHeader.nObjects].nKeyframes = tObject->objectMotions->GetNumElements();
        if (gObjPath[gMadHeader.nObjects].nKeyframes < 2)
        {                                                   //if this object does not have enough keyframes for an animation
            continue;
        }
        gObjPath[gMadHeader.nObjects].name = MADParse_AddString(tObject->fileName);//!!! is this the correct name for this object?
//        gObjPath[gMadHeader.nObjects].nKeyframes = 0;       //assume an extra 2 at start and end
        gObjPath[gMadHeader.nObjects].nKeyframes += 2;

        //see what animations this object participates in
        gObjPath[gMadHeader.nObjects].animationBits = 0;    //start with no animations

        pKeyFrameContainer = (LWOKeyframeContainer *)tObject->objectMotions->GetHead();  //get first keyframe
        pNULLContainer = (LWOContainer *)gLWOData->pNULLList->GetHead();
    	for (bitMask = 1; pNULLContainer; bitMask <<= 1)
    	{                                                   //scan all NULL objects (animation delimiters)
            bStartKeyframe = bEndKeyframe = 0;
    		tContainer = pNULLContainer;
    		tNULLObject = (LWObject *)&pNULLContainer->myObject;
    		
    		pNULLContainer = (LWOContainer *)pNULLContainer->GetNext();

            assert(bitTest(tNULLObject->flags, LWOF_NullObject));

            pKeyFrameContainer = (LWOKeyframeContainer *)tNULLObject->objectMotions->GetTail();  //get last keyframe
            pNULLEndKeyframe = &pKeyFrameContainer->myKeyframe;
            pKeyFrameContainer = (LWOKeyframeContainer *)pKeyFrameContainer->GetPrev();                   //get second-to-last keyframe
            pNULLStartKeyframe = &pKeyFrameContainer->myKeyframe;

            pKeyFrameContainer = (LWOKeyframeContainer *)tObject->objectMotions->GetHead();  //get first keyframe
            while (pKeyFrameContainer)
            {                                               //scan object's keyframes
                pKeyFrame = &pKeyFrameContainer->myKeyframe;
                pKeyFrameContainer = (LWOKeyframeContainer *)pKeyFrameContainer->GetNext();
                if (pNULLStartKeyframe->frameNumber == pKeyFrame->frameNumber)
                {                                           //if this object has a keyframe at the start of the animation
                    bStartKeyframe = TRUE;
                }
                if (pNULLEndKeyframe->frameNumber == pKeyFrame->frameNumber)
                {                                           //if this object has a keyframe at the end of the animation
                    bEndKeyframe = TRUE;
                }
            }
            if (bStartKeyframe && bEndKeyframe)             //if object has keyframes at start and end of NULL object's time limits
            {
                bitSet(gObjPath[gMadHeader.nObjects].animationBits, bitMask);
            }
        }
        if (gObjPath[gMadHeader.nObjects].animationBits == 0)
        {                                               //don't count the object if it's not in any animations
            bitSet(tObject->flags, MAD_SkipThisOne);
            continue;
        }
/*
        while (pKeyFrameContainer)
        {
            pKeyFrame = &pKeyFrameContainer->myKeyframe;
            pKeyFrameContainer = pKeyFrameContainer->GetNext();
            gObjPath[gMadHeader.nObjects].nKeyframes++;
        }
*/
        gMadHeader.nObjects++;
    }
    //second pass: go through all objects and compute file offsets
    fileOffset = madHeaderSize(gMadHeader.nAnimations) + gMadHeader.nObjects * sizeof(madobjpath);
    index = 0;
    pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();
    while (pContainer)
    {
		tContainer = pContainer;
		tObject = (LWObject *)&pContainer->myObject;
		
		pContainer = (LWOContainer *)pContainer->GetNext();

        if (tObject->objectMotions->GetNumElements() < 2)
        {                                                   //if this object does not have enough keyframes for an animation
            continue;
        }
        if (bitTest(tObject->flags, MAD_SkipThisOne))
        {
            continue;
        }
        assert(!bitTest(tObject->flags, LWOF_NullObject));

        gObjPath[index].times = fileOffset;
        fileOffset += sizeof(float) * gObjPath[index].nKeyframes;

        gObjPath[index].parameters = fileOffset;
        fileOffset += sizeof(float) * 3 * gObjPath[index].nKeyframes;

        for (j = 0; j < 6; j++)
        {                                                   //for each of x,y,z,h,p,b
            gObjPath[index].path[j] = fileOffset;           //set it's pointer
            fileOffset += sizeof(float) * gObjPath[index].nKeyframes;
        }

        index++;                                            //next object
        assert(index <= gMadHeader.nObjects);
    }

    gMadHeader.stringBlock = fileOffset;
    gMadHeader.stringBlockLength = stringBlockLength;

    //write out the header and animation headers
    fwrite(&gMadHeader, madHeaderSize(gMadHeader.nAnimations), 1, fileHandle);

    //write out the object path parameters
    fwrite(gObjPath, sizeof(madobjpath), gMadHeader.nObjects, fileHandle);

    //write out the animation keyframe data for each object path
    pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();
    while (pContainer)
    {
		tContainer = pContainer;
		tObject = (LWObject *)&pContainer->myObject;
		
		pContainer = (LWOContainer *)pContainer->GetNext();

        if (tObject->objectMotions->GetNumElements() < 2)
        {                                                   //if this object does not have enough keyframes for an animation
            continue;
        }
        if (bitTest(tObject->flags, MAD_SkipThisOne))
        {
            continue;
        }
        assert(!bitTest(tObject->flags, LWOF_NullObject));

        //write out the time array , parameters and keyframe parameter arrays
        for (j = 0; j < 8; j++)
        {                                                   //each of t,x,y,z,h,p,b
            pKeyFrameContainer = (LWOKeyframeContainer *)tObject->objectMotions->GetHead();  //get first keyframe

            for (index = 0; pKeyFrameContainer; index++)
            {
                pKeyFrame = &pKeyFrameContainer->myKeyframe;
                if (index != 0 && index != tObject->objectMotions->GetNumElements())
                {                                           //dupe the first and last keyframes
                    pKeyFrameContainer = (LWOKeyframeContainer *)pKeyFrameContainer->GetNext();
                }
                nFloats = 1;

                switch (j)
                {
                    case 0:                                 //times array
                        if (index == 0)
                        {                                   //duped first frame
                            value[0] = -1.0f;
                        }
                        else if (index == tObject->objectMotions->GetNumElements() + 1)
                        {
                            value[0] = gMadHeader.anim[gMadHeader.nAnimations - 1].endTime + 1.0f;
                        }
                        else
                        {
                            value[0] = (float)(pKeyFrame->frameNumber / gLWSData->framesPerSecond);
                        }
                        break;
                    case 1:                                 //tcb array
                        value[0] = pKeyFrame->tension;
                        value[1] = pKeyFrame->continuity;
                        value[2] = pKeyFrame->bias;
                        nFloats = 3;
                        break;
                    case 2:                                 //x,y,z,h,p,b
                        value[0] = pKeyFrame->xp;
                        break;
                    case 3:
                        value[0] = pKeyFrame->yp;
                        break;
                    case 4:
                        value[0] = pKeyFrame->zp;
                        break;
                    case 5:
                        value[0] = DEG_TO_RAD(pKeyFrame->h);
                        break;
                    case 6:
                        value[0] = DEG_TO_RAD(pKeyFrame->p);
                        break;
                    case 7:
                        value[0] = DEG_TO_RAD(pKeyFrame->b);
                        break;
                }

                fwrite(value, sizeof(float), nFloats, fileHandle);
            }
        }
    }

    //write out the string block
    fwrite(stringBlock, stringBlockLength, 1, fileHandle);

    return(SUCCESS);
}

/*-----------------------------------------------------------------------------
    Name        : InitializeMADParse
    Description : Init the MAD parser module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitializeMADParse(void)
{
    stringBlockLength = 0;
    stringBlock = NULL;
}

