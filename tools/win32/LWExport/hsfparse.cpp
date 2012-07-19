/*
** HSFPARSE.C : Code to handle exporting Homeworld Scene Files.
*/

#include "stdio.h"
#include "assert.h"
#include "string.h"
#include "math.h"

#include "export.h"
#include "hsfparse.h"
#include "lwsparse.h"
#include "matrix.h"

HSFGlobalData *gHSFData;

void InitializeHSFParse(void)
{
	gHSFData = new HSFGlobalData;
	memset(gHSFData, 0x00, sizeof(HSFGlobalData));

	gHSFData->pHSFFileHeader = new HSFFileHeader;
	memset(gHSFData->pHSFFileHeader, 0x00, sizeof(HSFFileHeader));
}

void DeInitializeHSFParse(void)
{
	if(gHSFData->pLights)
		delete [] gHSFData->pLights;

	delete gHSFData;
}

unsigned long CountLights(void)
{
	LWSLight *pLight;
	unsigned long nLights = 0;
	LWSLightContainer *pContainer;
	char pDelimiter[] = {'_'}, *pWork, *pBuffer = 0;
	
	for( pContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead() ; pContainer ; 
		pContainer = (LWSLightContainer *)pContainer->GetNext() )
	{
		pLight = &pContainer->myLight;

		// Should always be one.
		assert(pLight->name);

		if(pBuffer)
		{
			delete [] pBuffer;
			pBuffer = 0;
		}

		pBuffer = new char [strlen(pLight->name) + 1];
		strcpy(pBuffer, pLight->name);

		pWork = strtok(pBuffer, pDelimiter);

		// Is this a light we care about?
		if(strcmp(pWork, "HWLIGHT"))
			continue;

		// Yes...
		nLights ++;
	}

	if(pBuffer)
	{
		delete [] pBuffer;
		pBuffer = 0;
	}

	// Make room for the ambient light.
	nLights ++;

	return(nLights);
}

void HSFParse_SetupLights(void)
{
	LWSLight *pLight;
	LWSLightContainer *pContainer;
	unsigned long nLights = 0, lightIndex = 0;
	char pDelimiter[] = "_", *pWork, *pBuffer = 0;
	
	nLights = CountLights();

	gHSFData->nLights = nLights;
	gHSFData->pLights = new HSFLight [nLights];
	memset(gHSFData->pLights, 0x00, sizeof(HSFLight) * nLights);
	
	// Set up the ambient light.
	gHSFData->pLights[lightIndex].type = HSFL_AmbientLight;

	gHSFData->pLights[lightIndex].red = gLWSData->ambientRed;
	gHSFData->pLights[lightIndex].green = gLWSData->ambientGreen;
	gHSFData->pLights[lightIndex].blue = gLWSData->ambientBlue;

	gHSFData->pLights[lightIndex].intensity = gLWSData->ambientIntensity;
	
	lightIndex ++;
	
	for( pContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead() ; pContainer ; 
		pContainer = (LWSLightContainer *)pContainer->GetNext() )
	{
		pLight = &pContainer->myLight;

		// Should always be one.
		assert(pLight->name);

		if(pBuffer)
		{
			delete [] pBuffer;
			pBuffer = 0;
		}
		
		pBuffer = new char [strlen(pLight->name) + 1];
		strcpy(pBuffer, pLight->name);

		pWork = strtok(pBuffer, pDelimiter);

		// Is this a light we care about?
		if(strcmp(pWork, "HWLIGHT"))
			continue;
		
		gHSFData->pLights[lightIndex].type = pLight->type;

		gHSFData->pLights[lightIndex].x = pLight->xPos;
		gHSFData->pLights[lightIndex].y = pLight->yPos;
		gHSFData->pLights[lightIndex].z = pLight->zPos;

		gHSFData->pLights[lightIndex].h = pLight->xRot;
		gHSFData->pLights[lightIndex].p = pLight->yRot;
		gHSFData->pLights[lightIndex].b = pLight->zRot;

		gHSFData->pLights[lightIndex].coneAngle = pLight->coneAngle;
		gHSFData->pLights[lightIndex].edgeAngle = pLight->edgeAngle;

		gHSFData->pLights[lightIndex].red = pLight->red;
		gHSFData->pLights[lightIndex].green = pLight->green;
		gHSFData->pLights[lightIndex].blue = pLight->blue;

		gHSFData->pLights[lightIndex].intensity = pLight->intensity;

		lightIndex ++;
	}

	if(pBuffer)
		delete [] pBuffer;
}

void HSFParse_SetupFileHeader(void)
{
	strcpy(gHSFData->pHSFFileHeader->identifier, HSF_FILE_ID);
	gHSFData->pHSFFileHeader->version = HSF_VERSION;
	gHSFData->pHSFFileHeader->nLights = gHSFData->nLights;
}

signed char HSFParse_WriteFile(FILE *fileHandle)
{
	HSFParse_SetupLights();
	
	HSFParse_SetupFileHeader();

	// Write out the header.
	fwrite(gHSFData->pHSFFileHeader, sizeof(HSFFileHeader), 1, fileHandle);

	// Write out the lights.
	fwrite(gHSFData->pLights, sizeof(HSFLight), gHSFData->nLights, fileHandle);
	
	return(SUCCESS);
}