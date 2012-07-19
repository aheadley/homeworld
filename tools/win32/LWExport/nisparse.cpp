/*
** NISPARSE.CPP : Code to handle exporting .NIS data from Lightwave to Homeworld.
*/

#include "string.h"
#include "assert.h"

#include "export.h"
#include "nisparse.h"
#include "lwsparse.h"
#include "lwoparse.h"
#include "cclist.h"
#include "raceDefs.h"

NISGlobalData	*gNISData;

void InitializeNISParse(void)
{
	gNISData = new NISGlobalData;
	memset(gNISData, 0x00, sizeof(NISGlobalData));

	gNISData->pNISFileHeader = new NISFileHeader;
	memset(gNISData->pNISFileHeader, 0x00, sizeof(NISFileHeader));
}

void DeInitializeNISParse(void)
{
	if(gNISData->pNISFileHeader)
		delete gNISData->pNISFileHeader;

	if(gNISData->pStringBlock)
		delete [] gNISData->pStringBlock;

	if(gNISData->pObjectKeyframes)
		delete [] gNISData->pObjectKeyframes;

	if(gNISData->pCameraKeyframes)
		delete [] gNISData->pCameraKeyframes;

	if(gNISData->pLightKeyframes)
		delete [] gNISData->pLightKeyframes;

	if(gNISData->pEventKeyframes)
		delete [] gNISData->pEventKeyframes;

	if(gNISData->pLengths)
		delete [] gNISData->pLengths;
	
	if(gNISData->pTimes)
		delete [] gNISData->pTimes;

	if(gNISData->pTCB)
		delete [] gNISData->pTCB;

	if(gNISData->pX)
		delete [] gNISData->pX;

	if(gNISData->pY)
		delete [] gNISData->pY;

	if(gNISData->pZ)
		delete [] gNISData->pZ;

	if(gNISData->pH)
		delete [] gNISData->pH;

	if(gNISData->pP)
		delete [] gNISData->pP;

	if(gNISData->pB)
		delete [] gNISData->pB;

	delete gNISData;
}

void SetupBlockOffsets(void)
{
	gNISData->pNISFileHeader->oObjectPaths = sizeof(NISFileHeader);

	gNISData->pNISFileHeader->oCameraPaths =
		gNISData->pNISFileHeader->oObjectPaths + gNISData->nObjects * sizeof(spaceobjpath);

	gNISData->pNISFileHeader->oLightPaths =
		gNISData->pNISFileHeader->oCameraPaths + gNISData->nCameras * sizeof(camerapath);

	gNISData->pNISFileHeader->oEvents =
		gNISData->pNISFileHeader->oLightPaths + gNISData->nLights * sizeof(lightpath);

	gNISData->pNISFileHeader->oStringBlock =
		gNISData->pNISFileHeader->oEvents + gNISData->nEvents * sizeof(event) +
		sizeof(timeindex) * gNISData->nKeyframes +	// Lengths.
		sizeof(timeindex) * gNISData->nKeyframes +	// Frame times.
		sizeof(tcb) * gNISData->nKeyframes +		// TCB structures.
		sizeof(float) * gNISData->nKeyframes +		// X
		sizeof(float) * gNISData->nKeyframes +		// Y
		sizeof(float) * gNISData->nKeyframes +		// Z
		sizeof(float) * gNISData->nKeyframes +		// H
		sizeof(float) * gNISData->nKeyframes +		// P
		sizeof(float) * gNISData->nKeyframes;		// B
}

void NISParse_SetupNISFileHeader(void)
{
	float numFrames;

	// Set up the static members of the file header...
	strcpy(gNISData->pNISFileHeader->identifier, NIS_FILE_ID);
	gNISData->pNISFileHeader->version = NIS_VERSION;

	// Set up the length of the anim in seconds.
	numFrames = (float)gLWSData->lastFrame - gLWSData->firstFrame;
	gNISData->pNISFileHeader->tiNISLength = numFrames / gLWSData->framesPerSecond;

	// Disable the looping for now.
	gNISData->pNISFileHeader->tiLoopPoint = (float)-1;

	// Clear out the name.
	gNISData->pNISFileHeader->oName = 0;

	// Setup the file offsets.
	SetupBlockOffsets();

	// Set the number of structures.
	gNISData->pNISFileHeader->nObjectPaths = gNISData->nObjects;
	gNISData->pNISFileHeader->nCameraPaths = gNISData->nCameras;
	gNISData->pNISFileHeader->nLightPaths = gNISData->nLights;
	gNISData->pNISFileHeader->nEvents = gNISData->nEvents;
	gNISData->pNISFileHeader->sStringBlock = gNISData->sStringBlock;
}

void AllocateKeyframes(void)
{
	LWObject *pObject;
	LWOContainer *pLWOContainer;
	
	// We know there's only one camera.
	gNISData->nCameras = 1;

	// Count the number of object keyframes.
	for( pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead() ;
		pLWOContainer ; pLWOContainer = (LWOContainer *)pLWOContainer->GetNext() )
	{
		pObject = &pLWOContainer->myObject;

		gNISData->nKeyframes += pObject->objectMotions->GetNumElements();

		// To allow for 2 extra at beginning and end.
		gNISData->nKeyframes += 4;

		gNISData->nObjects ++;
	}

	// Count the number of camera keyframes
	gNISData->nKeyframes += gLWSData->pCamera->pKeyframeList->GetNumElements();
	
	// To allow for 2 extra at beginning and end.
	gNISData->nKeyframes += 4;

	gNISData->oLengthArrays = sizeof(NISFileHeader) +
		sizeof(spaceobjpath) * gNISData->nObjects +
		sizeof(camerapath) * gNISData->nCameras +
		sizeof(lightpath) * gNISData->nLights +
		sizeof(event) * gNISData->nEvents;
	gNISData->oTimeArrays = gNISData->oLengthArrays + sizeof(timeindex) * gNISData->nKeyframes;
	gNISData->oParameterArrays = gNISData->oTimeArrays + sizeof(timeindex) * gNISData->nKeyframes;
	gNISData->oXArrays = gNISData->oParameterArrays + sizeof(tcb) * gNISData->nKeyframes;
	gNISData->oYArrays = gNISData->oXArrays + sizeof(float) * gNISData->nKeyframes;
	gNISData->oZArrays = gNISData->oYArrays + sizeof(float) * gNISData->nKeyframes;
	gNISData->oHArrays = gNISData->oZArrays + sizeof(float) * gNISData->nKeyframes;
	gNISData->oPArrays = gNISData->oHArrays + sizeof(float) * gNISData->nKeyframes;
	gNISData->oBArrays = gNISData->oPArrays + sizeof(float) * gNISData->nKeyframes;
		
	// Allocate spaceobjpath structures...
	gNISData->pObjectKeyframes = new spaceobjpath [gNISData->nObjects];
	memset(gNISData->pObjectKeyframes, 0x00, sizeof(spaceobjpath) * gNISData->nObjects);

	// Allocate camerapath structures...
	gNISData->pCameraKeyframes = new camerapath [gNISData->nCameras];
	memset(gNISData->pCameraKeyframes, 0x00, sizeof(camerapath) * gNISData->nCameras);

	// Allocate lightpath structures...
	gNISData->pLightKeyframes = new lightpath [gNISData->nLights];
	memset(gNISData->pLightKeyframes, 0x00, sizeof(lightpath) * gNISData->nLights);

	// Allocate event structures...
	gNISData->pEventKeyframes = new event [gNISData->nEvents];
	memset(gNISData->pEventKeyframes, 0x00, sizeof(event) * gNISData->nEvents);

	// Allocate keyframe length arrays...
	gNISData->pLengths = new timeindex [gNISData->nKeyframes];
	memset(gNISData->pLengths, 0x00, sizeof(timeindex) * gNISData->nKeyframes);

	// Allocate time arrays...
	gNISData->pTimes = new timeindex [gNISData->nKeyframes];
	memset(gNISData->pTimes, 0x00, sizeof(timeindex) * gNISData->nKeyframes);

	// Allocate tension/contunity/bias arrays...
	gNISData->pTCB = new tcb [gNISData->nKeyframes];
	memset(gNISData->pTCB, 0x00, sizeof(tcb) * gNISData->nKeyframes);

	// Allocate X arrays...
	gNISData->pX = new float [gNISData->nKeyframes];
	memset(gNISData->pX, 0x00, sizeof(float) * gNISData->nKeyframes);

	// Allocate Y arrays...
	gNISData->pY = new float [gNISData->nKeyframes];
	memset(gNISData->pY, 0x00, sizeof(float) * gNISData->nKeyframes);

	// Allocate Z arrays...
	gNISData->pZ = new float [gNISData->nKeyframes];
	memset(gNISData->pZ, 0x00, sizeof(float) * gNISData->nKeyframes);

	// Allocate H arrays...
	gNISData->pH = new float [gNISData->nKeyframes];
	memset(gNISData->pH, 0x00, sizeof(float) * gNISData->nKeyframes);

	// Allocate P arrays...
	gNISData->pP = new float [gNISData->nKeyframes];
	memset(gNISData->pP, 0x00, sizeof(float) * gNISData->nKeyframes);

	// Allocate B arrays...
	gNISData->pB = new float [gNISData->nKeyframes];
	memset(gNISData->pB, 0x00, sizeof(float) * gNISData->nKeyframes);
}

unsigned long SetRace(char *shipFileName)
{
	char pDelimiter[] = {' ', '\\'};
	char *pBuffer, *pWork;
	unsigned long race = 0xffffffff;

	pBuffer = new char [strlen(shipFileName) + 1];
	strcpy(pBuffer, shipFileName);

	pWork = strtok(pBuffer, pDelimiter);

	// Should be at least one.
	assert(pWork);
#define ReturnRace(n)   {race = (n); break;}

	while(pWork)
	{
		if(!_stricmp(pWork, "R1"))
			ReturnRace(R1);

		if(!_stricmp(pWork, "R2"))
			ReturnRace(R2);

		if(!_stricmp(pWork, "P1"))
			ReturnRace(P1);

		if(!_stricmp(pWork, "P2"))
			ReturnRace(P2);

		if(!_stricmp(pWork, "P3"))
			ReturnRace(P3);

		if(!_stricmp(pWork, "Traders"))
			ReturnRace(Traders);

		if(!_stricmp(pWork, "Resources"))
        {
            pWork = strtok(NULL, pDelimiter);
            assert(pWork);
    		if(!_stricmp(pWork, "Asteroids"))
    			ReturnRace(NSR_Asteroid);

    		if(!_stricmp(pWork, "DustClouds"))
    			ReturnRace(NSR_DustCloud);

    		if(!_stricmp(pWork, "GasClouds"))
    			ReturnRace(NSR_GasCloud);

    		if(!_stricmp(pWork, "Nebulae"))
    			ReturnRace(NSR_Nebula);
        }

		if(!_stricmp(pWork, "Derelicts"))
			ReturnRace(NSR_Derelict);

		if(!_stricmp(pWork, "ETG"))
			ReturnRace(NSR_Effect);

		if(!_stricmp(pWork, "Misc"))
			ReturnRace(NSR_Generic);

		pWork = strtok(NULL, pDelimiter);
	}

	delete [] pBuffer;

    assert(race != 0xffffffff);
	return(race);
}

void PopulateArrays(void)
{
	LWObject *pObject;
	LWOKeyframe *pKeyframe;
	LWOContainer *pLWOContainer;
	CameraKeyframe *pCamKeyframe;
	LWOKeyframeContainer *pKContainer;
	CameraKeyframeContainer *pCamKContainer;
	unsigned long keyframeIndex = 0, arrayIndex = 0, i;
	
	// Loop through all objects and populate keyframe arrays.
	for( pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead() ;
		pLWOContainer ; pLWOContainer = (LWOContainer *)pLWOContainer->GetNext() )
	{
		pObject = &pLWOContainer->myObject;

		gNISData->pObjectKeyframes[keyframeIndex].oName   = 0; // Set this up later.
		gNISData->pObjectKeyframes[keyframeIndex].oParentName = 0; // Set this up later.
		gNISData->pObjectKeyframes[keyframeIndex].race	= SetRace(pObject->fullFileName);
		gNISData->pObjectKeyframes[keyframeIndex].nSamples = pObject->objectMotions->GetNumElements() + 4;
		gNISData->pObjectKeyframes[keyframeIndex].timeOffset = (float)-1; // Start right away.

		// Offsets to data.
		gNISData->pObjectKeyframes[keyframeIndex].oLength = sizeof(timeindex) * arrayIndex + gNISData->oLengthArrays;
		gNISData->pObjectKeyframes[keyframeIndex].oTimes = sizeof(timeindex) * arrayIndex + gNISData->oTimeArrays;
		gNISData->pObjectKeyframes[keyframeIndex].oParameters = sizeof(tcb) * arrayIndex + gNISData->oParameterArrays;
		gNISData->pObjectKeyframes[keyframeIndex].oX = sizeof(float) * arrayIndex + gNISData->oXArrays;
		gNISData->pObjectKeyframes[keyframeIndex].oY = sizeof(float) * arrayIndex + gNISData->oYArrays;
		gNISData->pObjectKeyframes[keyframeIndex].oZ = sizeof(float) * arrayIndex + gNISData->oZArrays;
		gNISData->pObjectKeyframes[keyframeIndex].oH = sizeof(float) * arrayIndex + gNISData->oHArrays;
		gNISData->pObjectKeyframes[keyframeIndex].oP = sizeof(float) * arrayIndex + gNISData->oPArrays;
		gNISData->pObjectKeyframes[keyframeIndex].oB = sizeof(float) * arrayIndex + gNISData->oBArrays;

		// Add 2 duplicate frames in the beginning.
		pKContainer = (LWOKeyframeContainer *)pObject->objectMotions->GetHead();
		pKeyframe = &pKContainer->myKeyframe;
		for( i=0 ; i<2 ; i++, arrayIndex++ )
		{
			gNISData->pTimes[arrayIndex] = pKeyframe->frameNumber / gLWSData->framesPerSecond;
			gNISData->pTCB[arrayIndex].tension = pKeyframe->tension;
			gNISData->pTCB[arrayIndex].continuity = pKeyframe->continuity;
			gNISData->pTCB[arrayIndex].bias = pKeyframe->bias;
			gNISData->pX[arrayIndex] = pKeyframe->xp;
			gNISData->pY[arrayIndex] = pKeyframe->yp;
			gNISData->pZ[arrayIndex] = pKeyframe->zp;
			gNISData->pH[arrayIndex] = pKeyframe->h;
			gNISData->pP[arrayIndex] = pKeyframe->p;
			gNISData->pB[arrayIndex] = pKeyframe->b;
		}
		
		for( pKContainer = (LWOKeyframeContainer *)pObject->objectMotions->GetHead() ;
			pKContainer ; pKContainer = (LWOKeyframeContainer *)pKContainer->GetNext() )
		{
				LWOKeyframeContainer *pNextKeyframeContainer = (LWOKeyframeContainer *)pKContainer->GetNext();
				pKeyframe = &pKContainer->myKeyframe;

				// Is there a keyframe after this one?
				if(pNextKeyframeContainer)
				{
					LWOKeyframe *pNextKeyframe = &pNextKeyframeContainer->myKeyframe;
					gNISData->pLengths[arrayIndex] = (pNextKeyframe->frameNumber - pKeyframe->frameNumber) /
						gLWSData->framesPerSecond;
				}
				else
				{
					gNISData->pLengths[arrayIndex] = (gLWSData->lastFrame - pKeyframe->frameNumber) /
						gLWSData->framesPerSecond;
				}
				
				gNISData->pTimes[arrayIndex] = pKeyframe->frameNumber / gLWSData->framesPerSecond;
				gNISData->pTCB[arrayIndex].tension = pKeyframe->tension;
				gNISData->pTCB[arrayIndex].continuity = pKeyframe->continuity;
				gNISData->pTCB[arrayIndex].bias = pKeyframe->bias;
				gNISData->pX[arrayIndex] = pKeyframe->xp;
				gNISData->pY[arrayIndex] = pKeyframe->yp;
				gNISData->pZ[arrayIndex] = pKeyframe->zp;
				gNISData->pH[arrayIndex] = pKeyframe->h;
				gNISData->pP[arrayIndex] = pKeyframe->p;
				gNISData->pB[arrayIndex] = pKeyframe->b;

				arrayIndex ++;
		}

		// Add 2 duplicate frames at the end.
		pKContainer = (LWOKeyframeContainer *)pObject->objectMotions->GetTail();
		pKeyframe = &pKContainer->myKeyframe;
		for( i=0 ; i<2 ; i++, arrayIndex++ )
		{
//			gNISData->pTimes[arrayIndex] = pKeyframe->frameNumber / gLWSData->framesPerSecond;
			gNISData->pTimes[arrayIndex] = ((float)gLWSData->lastFrame - gLWSData->firstFrame) / gLWSData->framesPerSecond;
			gNISData->pTCB[arrayIndex].tension = pKeyframe->tension;
			gNISData->pTCB[arrayIndex].continuity = pKeyframe->continuity;
			gNISData->pTCB[arrayIndex].bias = pKeyframe->bias;
			gNISData->pX[arrayIndex] = pKeyframe->xp;
			gNISData->pY[arrayIndex] = pKeyframe->yp;
			gNISData->pZ[arrayIndex] = pKeyframe->zp;
			gNISData->pH[arrayIndex] = pKeyframe->h;
			gNISData->pP[arrayIndex] = pKeyframe->p;
			gNISData->pB[arrayIndex] = pKeyframe->b;
		}

		keyframeIndex ++;
	}

	// Set camera and populate keyframe arrays.
	gNISData->pCameraKeyframes->nSamples = gLWSData->pCamera->pKeyframeList->GetNumElements() + 4;
	gNISData->pCameraKeyframes->timeOffset = (float)-1; // Start right away.

	// Offsets to data.
	gNISData->pCameraKeyframes->oLength = sizeof(timeindex) * arrayIndex + gNISData->oLengthArrays;
	gNISData->pCameraKeyframes->oTimes = sizeof(timeindex) * arrayIndex + gNISData->oTimeArrays;
	gNISData->pCameraKeyframes->oParameters = sizeof(tcb) * arrayIndex + gNISData->oParameterArrays;
	gNISData->pCameraKeyframes->oX = sizeof(float) * arrayIndex + gNISData->oXArrays;
	gNISData->pCameraKeyframes->oY = sizeof(float) * arrayIndex + gNISData->oYArrays;
	gNISData->pCameraKeyframes->oZ = sizeof(float) * arrayIndex + gNISData->oZArrays;
	gNISData->pCameraKeyframes->oH = sizeof(float) * arrayIndex + gNISData->oHArrays;
	gNISData->pCameraKeyframes->oP = sizeof(float) * arrayIndex + gNISData->oPArrays;
	gNISData->pCameraKeyframes->oB = sizeof(float) * arrayIndex + gNISData->oBArrays;

	// Add 2 duplicate frames in the beginning.
	pCamKContainer = (CameraKeyframeContainer *)gLWSData->pCamera->pKeyframeList->GetHead();
	pCamKeyframe = &pCamKContainer->myKeyframe;
	for( i=0 ; i<2 ; i++, arrayIndex++ )
	{
		gNISData->pTimes[arrayIndex] = pCamKeyframe->frameNumber / gLWSData->framesPerSecond;
		gNISData->pTCB[arrayIndex].tension = pCamKeyframe->tension;
		gNISData->pTCB[arrayIndex].continuity = pCamKeyframe->continuity;
		gNISData->pTCB[arrayIndex].bias = pCamKeyframe->bias;
		gNISData->pX[arrayIndex] = pCamKeyframe->xp;
		gNISData->pY[arrayIndex] = pCamKeyframe->yp;
		gNISData->pZ[arrayIndex] = pCamKeyframe->zp;
		gNISData->pH[arrayIndex] = pCamKeyframe->h;
		gNISData->pP[arrayIndex] = pCamKeyframe->p;
		gNISData->pB[arrayIndex] = pCamKeyframe->b;
	}
	
	for( pCamKContainer = (CameraKeyframeContainer *)gLWSData->pCamera->pKeyframeList->GetHead();
		pCamKContainer ; pCamKContainer = (CameraKeyframeContainer *)pCamKContainer->GetNext() )
	{
			LWOKeyframeContainer *pNextKeyframeContainer = (LWOKeyframeContainer *)pCamKContainer->GetNext();
			pCamKeyframe = &pCamKContainer->myKeyframe;

			// Is there a keyframe after this one?
			if(pNextKeyframeContainer)
			{
				LWOKeyframe *pNextKeyframe = &pNextKeyframeContainer->myKeyframe;
				gNISData->pLengths[arrayIndex] = (pNextKeyframe->frameNumber - pCamKeyframe->frameNumber) /
					gLWSData->framesPerSecond;
			}
			else
			{
				gNISData->pLengths[arrayIndex] = (gLWSData->lastFrame - pCamKeyframe->frameNumber) /
					gLWSData->framesPerSecond;
			}
			
			gNISData->pTimes[arrayIndex] = pCamKeyframe->frameNumber / gLWSData->framesPerSecond;
			gNISData->pTCB[arrayIndex].tension = pCamKeyframe->tension;
			gNISData->pTCB[arrayIndex].continuity = pCamKeyframe->continuity;
			gNISData->pTCB[arrayIndex].bias = pCamKeyframe->bias;
			gNISData->pX[arrayIndex] = pCamKeyframe->xp;
			gNISData->pY[arrayIndex] = pCamKeyframe->yp;
			gNISData->pZ[arrayIndex] = pCamKeyframe->zp;
			gNISData->pH[arrayIndex] = pCamKeyframe->h;
			gNISData->pP[arrayIndex] = pCamKeyframe->p;
			gNISData->pB[arrayIndex] = pCamKeyframe->b;

			arrayIndex ++;
	}

	// Add 2 duplicate frames at the end.
	pCamKContainer = (CameraKeyframeContainer *)gLWSData->pCamera->pKeyframeList->GetTail();
	pCamKeyframe = &pCamKContainer->myKeyframe;
	for( i=0 ; i<2 ; i++, arrayIndex++ )
	{
		//gNISData->pTimes[arrayIndex] = pCamKeyframe->frameNumber / gLWSData->framesPerSecond;
		gNISData->pTimes[arrayIndex] = ((float)gLWSData->lastFrame - gLWSData->firstFrame) / gLWSData->framesPerSecond;
		gNISData->pTCB[arrayIndex].tension = pCamKeyframe->tension;
		gNISData->pTCB[arrayIndex].continuity = pCamKeyframe->continuity;
		gNISData->pTCB[arrayIndex].bias = pCamKeyframe->bias;
		gNISData->pX[arrayIndex] = pCamKeyframe->xp;
		gNISData->pY[arrayIndex] = pCamKeyframe->yp;
		gNISData->pZ[arrayIndex] = pCamKeyframe->zp;
		gNISData->pH[arrayIndex] = pCamKeyframe->h;
		gNISData->pP[arrayIndex] = pCamKeyframe->p;
		gNISData->pB[arrayIndex] = pCamKeyframe->b;
	}
}

void NISParse_CreateKeyframeList(void)
{
	AllocateKeyframes();

	PopulateArrays();
}

timeindex CalcTimeIndex(LWOKeyframe *pLWOKeyframe)
{
	return(pLWOKeyframe->frameNumber / gLWSData->framesPerSecond);
}

char CheckDuplicateString(ccList *list, char *s)
{
	ccString *temp = (ccString *)list->GetHead();

	while(temp)
	{
		if(!strcmp(s, temp->s))
			return(1);

		temp = temp->GetNext();
	}

	return(0);
}

void AddString(ccList *list, char *s)
{
	if(CheckDuplicateString(list, s))
		return;

	ccString *temp = new ccString(s);

	gNISData->sStringBlock += strlen(s) + 1;

	list->AddNode(temp);
}

void NISParse_CalcStringBlockSize(ccList *stringList)
{
	LWObject *pObject;
	LWOContainer *pLWOContainer;
	
	for( pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead() ;
		pLWOContainer ; pLWOContainer = (LWOContainer *)pLWOContainer->GetNext() )
	{
		pObject = &pLWOContainer->myObject;

		AddString(stringList, pObject->fileName);
	}
}

unsigned long FindStringInBlock(char *s)
{
	char *pTemp;

	pTemp = strtok(gNISData->pStringBlock, "");
	
	while(pTemp)
	{
		if(!strcmp(pTemp, s))
			return((unsigned long)pTemp - (unsigned long)gNISData->pStringBlock);

		pTemp += strlen(pTemp) + 1;
	}

	assert(0);
	return(0);
}

void NISParse_BuildStringBlock(void)
{
	LWObject *pObject;
	ccList stringList;
	char *pStringBlock;
	ccString *tempString;
	LWOContainer *pLWOContainer;
	unsigned long keyframeIndex = 0;
	
	NISParse_CalcStringBlockSize(&stringList);

	gNISData->pStringBlock = new char [gNISData->sStringBlock];
	pStringBlock = gNISData->pStringBlock;

	// Copy over the string list into one block.
	tempString = (ccString *)stringList.GetHead();

	while(tempString)
	{
		strcpy(pStringBlock, tempString->s);
		pStringBlock += strlen(tempString->s) + 1;

		tempString = tempString->GetNext();
	}

	for( pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead() ;
		pLWOContainer ; pLWOContainer = (LWOContainer *)pLWOContainer->GetNext() )
	{
		pObject = &pLWOContainer->myObject;

		if(pObject->fileName)
			gNISData->pObjectKeyframes[keyframeIndex].oName = FindStringInBlock(pObject->fileName);

		if(pObject->iParentObject)
		{
			LWOContainer *pTempContainer = (LWOContainer *)gLWOData->pLWOList->GetNthElement(pObject->iParentObject - 1);

			if(pTempContainer->myObject.fileName)
				gNISData->pObjectKeyframes[keyframeIndex].oParentName = FindStringInBlock(pTempContainer->myObject.fileName);
		}

		keyframeIndex ++;
	}
}

signed char NISParse_WriteFile(FILE *fileHandle)
{
	NISParse_CreateKeyframeList();

	NISParse_BuildStringBlock();

	NISParse_SetupNISFileHeader();

	unsigned long i;

	for( i=0 ; i<gNISData->nObjects ; i++ )
	{
		char *pTemp;

		pTemp = (char *)(gNISData->pStringBlock + gNISData->pObjectKeyframes[i].oName);

		char *fuck = 0;
	}

	// Write out the header.
	fwrite(gNISData->pNISFileHeader, sizeof(NISFileHeader), 1, fileHandle);

	// Write out the object motion blocks.
	fwrite(gNISData->pObjectKeyframes, sizeof(spaceobjpath), gNISData->nObjects, fileHandle);

	// Write out the camera motion blocks.
	fwrite(gNISData->pCameraKeyframes, sizeof(camerapath), gNISData->nCameras, fileHandle);

	// Write out the light motion blocks.
	fwrite(gNISData->pLightKeyframes, sizeof(lightpath), gNISData->nLights, fileHandle);

	// Write out the event blocks.
	fwrite(gNISData->pEventKeyframes, sizeof(event), gNISData->nEvents, fileHandle);

	// Write out the motion path arrays.
	fwrite(gNISData->pLengths, sizeof(timeindex), gNISData->nKeyframes, fileHandle);
	fwrite(gNISData->pTimes, sizeof(timeindex), gNISData->nKeyframes, fileHandle);
	fwrite(gNISData->pTCB, sizeof(tcb), gNISData->nKeyframes, fileHandle);
	fwrite(gNISData->pX, sizeof(float), gNISData->nKeyframes, fileHandle);
	fwrite(gNISData->pY, sizeof(float), gNISData->nKeyframes, fileHandle);
	fwrite(gNISData->pZ, sizeof(float), gNISData->nKeyframes, fileHandle);
	fwrite(gNISData->pH, sizeof(float), gNISData->nKeyframes, fileHandle);
	fwrite(gNISData->pP, sizeof(float), gNISData->nKeyframes, fileHandle);
	fwrite(gNISData->pB, sizeof(float), gNISData->nKeyframes, fileHandle);
	
	// Write out the string block.
	fwrite(gNISData->pStringBlock, 1, gNISData->sStringBlock, fileHandle);

	return(SUCCESS);
}