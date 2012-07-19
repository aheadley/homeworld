/*
** LWSPARSE.CPP : Code to handle loading and saving of Lightwave Scene files. (.LWS).
*/

#include "stdio.h"
#include "memory.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "assert.h"

#include "readwrite.h"
#include "lwsparse.h"
#include "lwoparse.h"
#include "export.h"
#include "matrix.h"
#include "reorient.h"

LWSGlobalData	*gLWSData;

LWSLightContainer::LWSLightContainer(void)
{
	myLight.name = 0;
}

LWSLightContainer::~LWSLightContainer(void)
{
	if(myLight.name)
		delete [] myLight.name;
}

CameraKeyframeContainer::CameraKeyframeContainer(void)
{
	memset(&myKeyframe, 0x00, sizeof(CameraKeyframe));
}

CameraKeyframeContainer::~CameraKeyframeContainer(void)
{
}

void InitializeLWSParse(void)
{
	gLWSData = new LWSGlobalData;
	memset(gLWSData, 0x00, sizeof(LWSGlobalData));

	gLWSData->pCamera = new LWSCamera;
	memset(gLWSData->pCamera, 0x00, sizeof(LWSCamera));
	gLWSData->pCamera->pKeyframeList = new ccList;

	// Initialize Specifics.
	gLWSData->pLWSFuncList = new FunctionDef [NUM_LWS_PARSE_FUNCS];

	gLWSData->pLightList = new ccList;

	// Add parsing functions for LWS file.
	AddParseFunc(&gLWSData->pLWSFuncList[0], "LoadObject", (void *)LWSParse_LoadObject);
	AddParseFunc(&gLWSData->pLWSFuncList[1], "PivotPoint", (void *)LWSParse_PivotPoint);
	AddParseFunc(&gLWSData->pLWSFuncList[2], "AddLight", (void *)LWSParse_AddLight);
	AddParseFunc(&gLWSData->pLWSFuncList[3], "ParentObject", (void *)LWSParse_ParentObject);
	AddParseFunc(&gLWSData->pLWSFuncList[4], "ObjectMotion", (void *)LWSParse_ObjectMotion);
	AddParseFunc(&gLWSData->pLWSFuncList[5], "FirstFrame", (void *)LWSParse_FirstFrame);
	AddParseFunc(&gLWSData->pLWSFuncList[6], "LastFrame", (void *)LWSParse_LastFrame);
	AddParseFunc(&gLWSData->pLWSFuncList[7], "FrameStep", (void *)LWSParse_FrameStep);
	AddParseFunc(&gLWSData->pLWSFuncList[8], "FramesPerSecond", (void *)LWSParse_FramesPerSecond);
	AddParseFunc(&gLWSData->pLWSFuncList[9], "AddNullObject", (void *)LWSParse_AddNullObject);
	AddParseFunc(&gLWSData->pLWSFuncList[10], "AmbientColor", (void *)LWSParse_AmbientColor);
	AddParseFunc(&gLWSData->pLWSFuncList[11], "AmbIntensity", (void *)LWSParse_AmbIntensity);
	AddParseFunc(&gLWSData->pLWSFuncList[12], "CameraMotion", (void *)LWSParse_CameraMotion);
}

void DeInitializeLWSParse(void)
{
	unsigned long i;

	for( i=0 ; i<NUM_LWS_PARSE_FUNCS ; i++ )
		RemParseFunc(&gLWSData->pLWSFuncList[i]);
	
	delete [] gLWSData->pLWSFuncList;

	delete gLWSData->pLightList;

	delete gLWSData->pCamera->pKeyframeList;
	delete gLWSData->pCamera;

	delete gLWSData;
}

signed char LWSParse_ReadFile(FILE *fileHandle)
{
	ParseFuncPtr pParseFunc;
	char lineBuffer[255];
	signed char retVal = SUCCESS;

	while((!feof(fileHandle)) && (retVal != FAILURE))
	{
		ReadTextLine(lineBuffer, fileHandle);

		if(pParseFunc = IsParseFunc(lineBuffer, gLWSData->pLWSFuncList, NUM_LWS_PARSE_FUNCS))
			retVal = pParseFunc(lineBuffer, fileHandle);
	}

	if(retVal == SUCCESS)
		ReorientLWSData();

	if(retVal == SUCCESS)
		retVal = FixupLWOParentIndices();

	if(retVal == SUCCESS)
		retVal = CalculateLWObjectNames();
	
	return(retVal);
}

/*
** The LoadObject function has the filename of a .LWO file to load as it's only parameter.
** The filename is an explicit path, so we may have to fix it up using the HW_Root parameter.
*/
signed char LWSParse_LoadObject(char *currentLine, FILE *fileHandle)
{
	char *pTempBuf, pDelimiter[] = {' ', 0x00};
	LWOContainer *pLWOContainer;
	FILE *lwoFileHandle;
	signed long duplicateIndex;

	// The first strtok skips gets the function name.  The second gets the filename.
	pTempBuf = strtok(currentLine, pDelimiter);
	pTempBuf = strtok(NULL, pDelimiter);

#ifdef ANAL_DEBUGGING
	printf("Calling LWSParse_LoadObject\n");
	printf("LineBuffer: %s\n", currentLine);
	printf("FileName: %s\n", pTempBuf);
#endif

	if(FindRootFile(pTempBuf, getenv("HW_Root")) == FAILURE)
		return(FAILURE);

	// Load in this object.
	pLWOContainer = CreateNewLWObject(pTempBuf);

	// Make sure this isn't a duplicate of an already loaded object.
	duplicateIndex = CheckDuplicateLWOFile(pLWOContainer);

	// save the filename.
	pLWOContainer->myObject.fileName = new char [strlen(pTempBuf) + 1];
	strcpy(pLWOContainer->myObject.fileName, pTempBuf);

	if(duplicateIndex != UNIQUE)
	{
		bitSet(pLWOContainer->myObject.flags, LWOF_DuplicateObject);
		pLWOContainer->myObject.duplicateIndex = duplicateIndex;
	}
	else
	{
		bitSet(pLWOContainer->myObject.flags, LWOF_UniqueObject);
		pLWOContainer->myObject.duplicateIndex = duplicateIndex;
	}
	
	lwoFileHandle = fopen(pTempBuf, "rb");

#ifdef ANAL_DEBUGGING
	printf("LWExport\t: \tImporting %s\n", pTempBuf);
#endif

	if(LWOParse_ReadFile(lwoFileHandle) == FAILURE)
	{
		fclose(lwoFileHandle);

		delete pLWOContainer;

		return(FAILURE);
	}

	gLWOData->pLWOList->AddNode(pLWOContainer);

	gLWOData->pCurrentLWObject = &pLWOContainer->myObject;

	fclose(lwoFileHandle);

	return(SUCCESS);
}

signed char LWSParse_PivotPoint(char *currentLine, FILE *fileHandle)
{
	printf("Calling LWSParse_PivotPoint\n");
	return(SUCCESS);
}

signed char LWSParse_AmbientColor(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the colors.
	pWork = strtok(currentLine, pDelimiter);

	// Red...
	pWork = strtok(NULL, pDelimiter);
	gLWSData->ambientRed = atoi(pWork);

	// Green...
	pWork = strtok(NULL, pDelimiter);
	gLWSData->ambientGreen = atoi(pWork);

	// Blue...
	pWork = strtok(NULL, pDelimiter);
	gLWSData->ambientBlue = atoi(pWork);

	return(SUCCESS);
}

signed char LWSParse_AmbIntensity(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWSData->ambientIntensity = (float)atof(pWork);

	return(SUCCESS);
}

signed char CalculateLWObjectNames(void)
{
	LWOContainer *pLWOContainer, *pLWOContainerCheck;
	LWObject *pLWObject, *pLWObjectCheck;
	char *pSearch, *pWork, *pCopy;
	int objectIndex;

	pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	while(pLWOContainer)
	{
		pLWObject = &pLWOContainer->myObject;

		// If no filename, there's a problem.
		assert(pLWObject->fileName);

		// Strip off the path. (if any)
		pSearch = pLWObject->fileName;

		while(pSearch = strchr(pSearch, '\\'))
		{
			pSearch ++;
			pWork = pSearch;
		}

		pCopy = new char [strlen(pWork) + 1];
		strcpy(pCopy, pWork);

		pSearch = pWork = pCopy;

		// Strip off the extension (if any).
		if(pSearch = strchr(pSearch, '.'))
			*pSearch = 0x00;

		// Free up the existing filename.  No check b/c of the assert above.
		delete [] pLWObject->fileName;

		pLWObject->fileName = new char [strlen(pCopy) + 1];
		strcpy(pLWObject->fileName, pCopy);

		delete [] pCopy;

		pLWOContainer = (LWOContainer *)pLWOContainer->GetNext();
	}

	// Now all the filenames are set up correctly.

	pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	while(pLWOContainer)
	{
		pLWObject = &pLWOContainer->myObject;

		pLWOContainerCheck = (LWOContainer *)gLWOData->pLWOList->GetHead();

		objectIndex = 2;

		while(pLWOContainerCheck)
		{
			pLWObjectCheck = &pLWOContainerCheck->myObject;

			// Make sure it's not me.
			if(pLWObject != pLWObjectCheck)
			{
				// Do I have the same name as anyone else?
				if(!strcmp(pLWObject->fileName, pLWObjectCheck->fileName))
				{
					// Make a copy of the name and allow space for (nnn)
					pCopy = new char [strlen(pLWObjectCheck->fileName) + 5 + 1];
					sprintf(pCopy, "%s(%i)", pLWObjectCheck->fileName, objectIndex);

					delete [] pLWObjectCheck->fileName;

					pLWObjectCheck->fileName = new char [strlen(pCopy) + 1];
					strcpy(pLWObjectCheck->fileName, pCopy);

					delete [] pCopy;

					objectIndex ++;
				}
			}

			pLWOContainerCheck = (LWOContainer *)pLWOContainerCheck->GetNext();
		}

		// Did we find any copies?
		if(objectIndex > 2)
		{
			// We use 1 here explicitly b/c we know this one was the first.
			pCopy = new char [strlen(pLWObject->fileName) + 3 + 1];
			sprintf(pCopy, "%s(%i)", pLWObject->fileName, 1);

			delete [] pLWObject->fileName;

			pLWObject->fileName = new char [strlen(pCopy) + 1];
			strcpy(pLWObject->fileName, pCopy);

			delete [] pCopy;
		}
		
		pLWOContainer = (LWOContainer *)pLWOContainer->GetNext();
	}

	return(SUCCESS);
}

signed char FixupLWOParentIndices(void)
{
	LWObject *pObject, *tObject;
	LWOContainer *pContainer, *tContainer;
	unsigned long nNullNodes = 0, lwIndex = 1, actualIndex = 1;

	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	while(pContainer)
	{
		pObject = (LWObject *)&pContainer->myObject;

		// Is this a null node?
		if(bitTest(pObject->flags, LWOF_NullObject))
		{
			nNullNodes ++;

			// Keep track of LW's 1 based index.
			lwIndex ++;

			pContainer = (LWOContainer *)pContainer->GetNext();

			continue;
		}

		// Have there been any null nodes before me?
		if(nNullNodes)
		{
			for( tContainer = (LWOContainer *)gLWOData->pLWOList->GetHead() ; tContainer ; tContainer = (LWOContainer *)tContainer->GetNext())
			{
				tObject = (LWObject *)&tContainer->myObject;

				if(bitTest(tObject->flags, LWOF_NullObject))
					continue;

				if(tObject->iParentObject == lwIndex)
					tObject->iParentObject = actualIndex;

				if(tObject->duplicateIndex == (signed long)lwIndex - 1)
					tObject->duplicateIndex = actualIndex - 1;
			}
		}

		// Keep track of LW's 1 based index.
		lwIndex ++;
		// This is a node.
		actualIndex ++;

		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	// Remove all the null nodes and put them into the NULL object list.
	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	while(pContainer)
	{
		tContainer = pContainer;
		tObject = (LWObject *)&pContainer->myObject;
		
		pContainer = (LWOContainer *)pContainer->GetNext();

		if(bitTest(tObject->flags, LWOF_NullObject))
        {                                                   //if this is a NULL object
			gLWOData->pLWOList->FlushNode(tContainer);      //remove from object list
            gLWOData->pNULLList->AddNode(tContainer);       //add to NULL list
        }
	}

	return(SUCCESS);
}

void ReorientLWSData(void)
{
	LWSLightContainer *pLightContainer;
	LWSLight *pLight;
	LWOContainer *pLWOContainer;
	LWObject *pObject;
	LWOKeyframeContainer *pKeyframeContainer;
	LWOKeyframe *pKeyframe;
	CameraKeyframeContainer *pCKeyframeContainer;
	CameraKeyframe *pCKeyframe;

	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	// Reorient the lights.
	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;

		ShipReorientPosition(&pLight->xPos, &pLight->yPos, &pLight->zPos);
		ShipReorientAngles(&pLight->xRot, &pLight->yRot, &pLight->zRot);
		ShipReorientAngles(&pLight->xVec, &pLight->yVec, &pLight->zVec);
		VectorFromShipAngle(&pLight->xVec, &pLight->yVec, &pLight->zVec);
		
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}

	pCKeyframeContainer = (CameraKeyframeContainer *)gLWSData->pCamera->pKeyframeList->GetHead();

	// Reorient the camera keyframes.
	while(pCKeyframeContainer)
	{
		pCKeyframe = &pCKeyframeContainer->myKeyframe;

//		WorldReorientPosition(&pCKeyframe->xp, &pCKeyframe->yp, &pCKeyframe->zp);
//		WorldReorientAngles(&pCKeyframe->h, &pCKeyframe->p, &pCKeyframe->b);
//		WorldReorientAngles(&pCKeyframe->xv, &pCKeyframe->yv, &pCKeyframe->zv);
//		VectorFromWorldAngle(&pCKeyframe->xv, &pCKeyframe->yv, &pCKeyframe->zv);
		
		pCKeyframeContainer = (CameraKeyframeContainer *)pCKeyframeContainer->GetNext();
	}

	pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	// Reorient the objects.
	while(pLWOContainer)
	{
		pObject = &pLWOContainer->myObject;

		ShipReorientPosition(&pObject->xp, &pObject->yp, &pObject->zp);
		ShipReorientAngles(&pObject->h, &pObject->p, &pObject->b);
		ShipReorientAngles(&pObject->xv, &pObject->yv, &pObject->zv);
		VectorFromShipAngle(&pObject->xv, &pObject->yv, &pObject->zv);

		pKeyframeContainer = (LWOKeyframeContainer *)pObject->objectMotions->GetHead();

		// Reorient the object keyframes.
		while(pKeyframeContainer)
		{
			pKeyframe = &pKeyframeContainer->myKeyframe;

//			WorldReorientPosition(&pKeyframe->xp, &pKeyframe->yp, &pKeyframe->zp);
//			WorldReorientAngles(&pKeyframe->h, &pKeyframe->p, &pKeyframe->b);
//			WorldReorientAngles(&pKeyframe->xv, &pKeyframe->yv, &pKeyframe->zv);
//			VectorFromWorldAngle(&pKeyframe->xv, &pKeyframe->yv, &pKeyframe->zv);

			pKeyframeContainer = (LWOKeyframeContainer *)pKeyframeContainer->GetNext();
		}

		pLWOContainer = (LWOContainer *)pLWOContainer->GetNext();
	}
}

LWSLight *CreateNewLight(void)
{
	LWSLightContainer *pRet;

	pRet = new LWSLightContainer;

	gLWSData->pLightList->AddNode(pRet);

	return(&pRet->myLight);
}

#define NUM_LIGHT_PARSE_FUNCS		(8)

signed char LWSParse_AddLight(char *currentLine, FILE *fileHandle)
{
	ParseFuncPtr pParseFunc;
	char lineBuffer[255];
	signed char retVal = SUCCESS;
	FunctionDef *pLightFuncList;
	unsigned long i;

	// Create a new light.
	gLWSData->pCurrentLight = CreateNewLight();
	
	pLightFuncList = new FunctionDef [NUM_LIGHT_PARSE_FUNCS];

	// Add the local light parse funcs.
	AddParseFunc(&pLightFuncList[0], "LightName", (void *)LWSParse_LightName);
	AddParseFunc(&pLightFuncList[1], "LightMotion", (void *)LWSParse_LightMotion);
	AddParseFunc(&pLightFuncList[2], "ConeAngle", (void *)LWSParse_ConeAngle);
	AddParseFunc(&pLightFuncList[3], "EdgeAngle", (void *)LWSParse_EdgeAngle);
	AddParseFunc(&pLightFuncList[4], "LightType", (void *)LWSParse_LightType);
	AddParseFunc(&pLightFuncList[5], "LightColor", (void *)LWSParse_LightColor);
	AddParseFunc(&pLightFuncList[6], "LgtIntensity", (void *)LWSParse_LgtIntensity);
	// ShadowType is the last one we care about and it's used to break.
	AddParseFunc(&pLightFuncList[7], "ShadowType", (void *)LWSParse_ShadowType);

	// When end of tag is encountered for this light, that parse func returns failure.
	while((!feof(fileHandle)) && (retVal != FAILURE))
	{
		ReadTextLine(lineBuffer, fileHandle);

		if(pParseFunc = IsParseFunc(lineBuffer, pLightFuncList, NUM_LIGHT_PARSE_FUNCS))
			retVal = pParseFunc(lineBuffer, fileHandle);
	}

	for( i=0 ; i<NUM_LIGHT_PARSE_FUNCS ; i++ )
		RemParseFunc(&pLightFuncList[i]);

	delete [] pLightFuncList;

	return(SUCCESS);
}

signed char LWSParse_LightName(char *currentLine, FILE *fileHandle)
{
	char *pTempBuf, pDelimiter[] = {' ', 0x00};
	
	// The first strtok skips gets the function name.  The second gets the name of the light.
	pTempBuf = strtok(currentLine, pDelimiter);
	pTempBuf = strtok(NULL, pDelimiter);

	gLWSData->pCurrentLight->name = new char [strlen(pTempBuf) + 1];

	strcpy(gLWSData->pCurrentLight->name, pTempBuf);
	
	return(SUCCESS);
}

signed char LWSParse_ShadowType(char *currentLine, FILE *fileHandle)
{

	// Use this to signify the end of the light.
	return(FAILURE);
}

signed char LWSParse_LightMotion(char *currentLine, FILE *fileHandle)
{
	char pTemp[255], *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the first two lines.
	ReadTextLine(pTemp, fileHandle);
	ReadTextLine(pTemp, fileHandle);

	// This is the important one.
	ReadTextLine(pTemp, fileHandle);

	// the order is pos(x, y, z) : rot(x, y, z)
	pWork = strtok(pTemp, pDelimiter); // xp
	gLWSData->pCurrentLight->xPos = (float)atof(pWork);
	
	pWork = strtok(NULL, pDelimiter); // yp
	gLWSData->pCurrentLight->yPos = (float)atof(pWork);

	pWork = strtok(NULL, pDelimiter); // zp
	gLWSData->pCurrentLight->zPos = (float)atof(pWork);

	pWork = strtok(NULL, pDelimiter); // xr
	gLWSData->pCurrentLight->xRot = (float)atof(pWork);

	pWork = strtok(NULL, pDelimiter); // yr
	gLWSData->pCurrentLight->yRot = (float)atof(pWork);

	pWork = strtok(NULL, pDelimiter); // zr
	gLWSData->pCurrentLight->zRot = (float)atof(pWork);

	// Converted to vectors later.
	gLWSData->pCurrentLight->xVec = gLWSData->pCurrentLight->xRot;
	gLWSData->pCurrentLight->yVec = gLWSData->pCurrentLight->yRot;
	gLWSData->pCurrentLight->zVec = gLWSData->pCurrentLight->zRot;

	return(SUCCESS);
}

signed char LWSParse_ConeAngle(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWSData->pCurrentLight->coneAngle = (float)atof(pWork);

	return(SUCCESS);
}

signed char LWSParse_EdgeAngle(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWSData->pCurrentLight->edgeAngle = (float)atof(pWork);

	return(SUCCESS);
}

signed char LWSParse_LightType(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the colors.
	pWork = strtok(currentLine, pDelimiter);

	// Type...
	pWork = strtok(NULL, pDelimiter);
	gLWSData->pCurrentLight->type = atoi(pWork);

	return(SUCCESS);
}

signed char LWSParse_LightColor(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the colors.
	pWork = strtok(currentLine, pDelimiter);

	// Red...
	pWork = strtok(NULL, pDelimiter);
	gLWSData->pCurrentLight->red = atoi(pWork);

	// Green...
	pWork = strtok(NULL, pDelimiter);
	gLWSData->pCurrentLight->green = atoi(pWork);

	// Blue...
	pWork = strtok(NULL, pDelimiter);
	gLWSData->pCurrentLight->blue = atoi(pWork);

	return(SUCCESS);
}

signed char LWSParse_LgtIntensity(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWSData->pCurrentLight->intensity = (float)atof(pWork);

	return(SUCCESS);
}

signed char LWSParse_ParentObject(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWOData->pCurrentLWObject->iParentObject = atoi(pWork);

	return(SUCCESS);
}

signed char LWSParse_ObjectMotion(char *currentLine, FILE *fileHandle)
{
	char pTemp[255], *pWork;
	char pDelimiter[] = {' ', 0x00};
	unsigned long nInfoChannels, nKeyframes, i;
	LWOKeyframeContainer *pKContainer;
	LWOKeyframe *pKeyframe;

	// Get the # information channels.
	ReadTextLine(pTemp, fileHandle);
	pWork = strtok(pTemp, pDelimiter);
	nInfoChannels = (unsigned long)atoi(pWork);
	// It should be 9.
	assert(nInfoChannels == 9);

	// Get the number of keyframes for this object.
	ReadTextLine(pTemp, fileHandle);
	pWork = strtok(pTemp, pDelimiter);
	nKeyframes = (unsigned long)atoi(pWork);

	for( i=0 ; i<nKeyframes ; i++ )
	{
		// Create a new keyframe object.
		pKContainer = CreateNewLWOKeyframe();
		pKeyframe = &pKContainer->myKeyframe;

		// Read in pos(x, y, z) : rot(x, y, z) : scale(x, y, z)
		ReadTextLine(pTemp, fileHandle);

		// the order is pos(x, y, z) : rot(x, y, z) : scale(x, y, z)
		pWork = strtok(pTemp, pDelimiter); // xp
		pKeyframe->xp = (float)atof(pWork);
		
		pWork = strtok(NULL, pDelimiter); // yp
		pKeyframe->yp = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // zp
		pKeyframe->zp = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // xr
		pKeyframe->h = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // yr
		pKeyframe->p = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // zr
		pKeyframe->b = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // xs
		pKeyframe->xs = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // ys
		pKeyframe->ys = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // zs
		pKeyframe->zs = (float)atof(pWork);

		// Converted to vectors later.
		pKeyframe->xv = pKeyframe->h;
		pKeyframe->yv = pKeyframe->p;
		pKeyframe->zv = pKeyframe->b;

		// Read in framenumber, linearValue, tension, continuity, bias
		ReadTextLine(pTemp, fileHandle);

		// the order is framenumber, linearValue, tension, continuity, bias
		pWork = strtok(pTemp, pDelimiter); // frameNumber
		pKeyframe->frameNumber = (unsigned long)atoi(pWork);
		
		pWork = strtok(NULL, pDelimiter); // linearValue
		pKeyframe->linearValue = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // tension
		pKeyframe->tension = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // continuity
		pKeyframe->continuity = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // bias
		pKeyframe->bias = (float)atof(pWork);

		// Add this keyframe to the object's motion list.
		gLWOData->pCurrentLWObject->objectMotions->AddNode(pKContainer);
	}

	// Grab Keyframe 0 and save it into the object.  Safe, b/c always at least 1 keyframe.
	pKContainer = (LWOKeyframeContainer *)gLWOData->pCurrentLWObject->objectMotions->GetHead();
	pKeyframe = &pKContainer->myKeyframe;

	gLWOData->pCurrentLWObject->xp = pKeyframe->xp;
	gLWOData->pCurrentLWObject->yp = pKeyframe->yp;
	gLWOData->pCurrentLWObject->zp = pKeyframe->zp;

	gLWOData->pCurrentLWObject->h = pKeyframe->h;
	gLWOData->pCurrentLWObject->p = pKeyframe->p;
	gLWOData->pCurrentLWObject->b = pKeyframe->b;
	
	gLWOData->pCurrentLWObject->xs = pKeyframe->xs;
	gLWOData->pCurrentLWObject->ys = pKeyframe->ys;
	gLWOData->pCurrentLWObject->zs = pKeyframe->zs;

	// Converted to vectors later.
	gLWOData->pCurrentLWObject->xv = gLWOData->pCurrentLWObject->h;
	gLWOData->pCurrentLWObject->yv = gLWOData->pCurrentLWObject->p;
	gLWOData->pCurrentLWObject->zv = gLWOData->pCurrentLWObject->b;

	return(SUCCESS);
}

signed char LWSParse_FirstFrame(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWSData->firstFrame = (unsigned long)atoi(pWork);

	return(SUCCESS);
}

signed char LWSParse_LastFrame(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWSData->lastFrame = (unsigned long)atoi(pWork);

	return(SUCCESS);
}

signed char LWSParse_FrameStep(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWSData->frameStep = (unsigned long)atoi(pWork);

	return(SUCCESS);
}

signed char LWSParse_FramesPerSecond(char *currentLine, FILE *fileHandle)
{
	char *pWork;
	char pDelimiter[] = {' ', 0x00};

	// Skip this the tag and go to the num.
	pWork = strtok(currentLine, pDelimiter);
	pWork = strtok(NULL, pDelimiter);

	gLWSData->framesPerSecond = (float)atof(pWork);

	return(SUCCESS);
}

signed char LWSParse_AddNullObject(char *currentLine, FILE *fileHandle)
{
    char *pTempBuf;
	// The first strtok skips gets the function name.  The second gets the filename.
	pTempBuf = strtok(currentLine, " ");
	pTempBuf = strtok(NULL, " ");
	LWOParse_AddNullObject(pTempBuf);
	gLWOData->pCurrentLWObject = &gLWOData->pCurrentLWOContainer->myObject;
	return(SUCCESS);
}

signed char LWSParse_CameraMotion(char *currentLine, FILE *fileHandle)
{
	char pTemp[255], *pWork;
	char pDelimiter[] = {' ', 0x00};
	unsigned long nInfoChannels, nKeyframes, i;
	CameraKeyframeContainer *pKContainer;
	CameraKeyframe *pKeyframe;

	// Get the # information channels.
	ReadTextLine(pTemp, fileHandle);
	pWork = strtok(pTemp, pDelimiter);
	nInfoChannels = (unsigned long)atoi(pWork);
	// It should be 9.
	assert(nInfoChannels == 9);

	// Get the number of keyframes for this camera.
	ReadTextLine(pTemp, fileHandle);
	pWork = strtok(pTemp, pDelimiter);
	nKeyframes = (unsigned long)atoi(pWork);

	for( i=0 ; i<nKeyframes ; i++ )
	{
		// Create a new camera keyframe.
		pKContainer = new CameraKeyframeContainer;
		pKeyframe = &pKContainer->myKeyframe;

		// Read in pos(x, y, z) : rot(x, y, z) : scale(x, y, z)
		ReadTextLine(pTemp, fileHandle);

		// the order is pos(x, y, z) : rot(x, y, z) : scale(x, y, z)
		pWork = strtok(pTemp, pDelimiter); // xp
		pKeyframe->xp = (float)atof(pWork);
		
		pWork = strtok(NULL, pDelimiter); // yp
		pKeyframe->yp = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // zp
		pKeyframe->zp = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // xr
		pKeyframe->h = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // yr
		pKeyframe->p = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // zr
		pKeyframe->b = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // xs
		pKeyframe->xs = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // ys
		pKeyframe->ys = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // zs
		pKeyframe->zs = (float)atof(pWork);

		// Converted to vectors later.
		pKeyframe->xv = pKeyframe->h;
		pKeyframe->yv = pKeyframe->p;
		pKeyframe->zv = pKeyframe->b;

		// Read in framenumber, linearValue, tension, continuity, bias
		ReadTextLine(pTemp, fileHandle);

		// the order is framenumber, linearValue, tension, continuity, bias
		pWork = strtok(pTemp, pDelimiter); // frameNumber
		pKeyframe->frameNumber = (unsigned long)atoi(pWork);
		
		pWork = strtok(NULL, pDelimiter); // linearValue
		pKeyframe->linearValue = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // tension
		pKeyframe->tension = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // continuity
		pKeyframe->continuity = (float)atof(pWork);

		pWork = strtok(NULL, pDelimiter); // bias
		pKeyframe->bias = (float)atof(pWork);

		// Add this keyframe to the camera's motion list.
		gLWSData->pCamera->pKeyframeList->AddNode(pKContainer);
	}

	return(SUCCESS);
}