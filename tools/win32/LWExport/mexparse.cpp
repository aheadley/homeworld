/*
** MEXPARSE.CPP : Code to export .MEX files.
*/

#include "memory.h"
#include "string.h"
#include "math.h"

#include "mexparse.h"
#include "lwsparse.h"
#include "lwoparse.h"
#include "geoparse.h"
#include "export.h"

MEXGlobalData	*gMEXData;

void InitializeMEXParse(void)
{
	gMEXData = new MEXGlobalData;

	memset(gMEXData, 0x00, sizeof(MEXGlobalData));

	// Initialize Specifics.
	gMEXData->pMEXFileHeader = new MEXFileHeader;
	memset(gMEXData->pMEXFileHeader, 0x00, sizeof(MEXFileHeader));
}

void DeInitializeMEXParse(void)
{
	if(!gMEXData)
		return;

	if(gMEXData->pGunChunks)
		delete [] gMEXData->pGunChunks;

	if(gMEXData->pEngineChunks)
		delete [] gMEXData->pEngineChunks;

	if(gMEXData->pDamageChunks)
		delete [] gMEXData->pDamageChunks;

	if(gMEXData->pDockingChunks)
		delete [] gMEXData->pDockingChunks;


	if(gMEXData->pColChunks)
		delete [] gMEXData->pColChunks;

	if(gMEXData->pColRectChunks)
		delete [] gMEXData->pColRectChunks;

	if(gMEXData->pMEXFileHeader)
		delete gMEXData->pMEXFileHeader;
		
	delete gMEXData;
}

#define MAX_GUNS			(50)
void MEXParse_SetupGuns(void)
{
	LWSLightContainer *pLightContainer;
	LWSLight *pLight;
	char *pWorkStr, pDelimiter[] = {'_', 0x00}, *realName;
	unsigned long nGunChunks = 0, iGuns = 0;
	char gunNames[MAX_GUNS][10];
	char tempName[100];
	
	// Count the number of chunks gun chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		// Is it a gun?
		if(!strcmp(pWorkStr, "GUN"))
		{
			// Get the real name of the gun.
			realName = strtok(NULL, pDelimiter);
			realName = strupr(realName);

			// Make sure it's less than 8 chars.
			if(strlen(realName) >= 7)
				realName[7] = 0x00;

			strcpy(gunNames[nGunChunks], realName);
		
			nGunChunks ++;
		}

		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}

	gMEXData->nGunChunks = nGunChunks;

	if(gMEXData->nGunChunks > 0)
	{
		// Allocate enough space to hold all the gun chunks.
		gMEXData->pGunChunks = new MEXGunChunk [nGunChunks];
	}

	// Loop through and assign the chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		// Is it a gun?
		if(!strcmp(pWorkStr, "GUN"))
		{
			// Setup the gun
			memset(gMEXData->pGunChunks[iGuns].type, 0xAB, 4);
			strcpy(gMEXData->pGunChunks[iGuns].type, "Gun");

			memset(gMEXData->pGunChunks[iGuns].name, 0xAB, 8);
			sprintf(gMEXData->pGunChunks[iGuns].name, "%s", gunNames[iGuns]);

			gMEXData->pGunChunks[iGuns].chunkSize = 32;

			gMEXData->pGunChunks[iGuns].xPos = pLight->xPos;
			gMEXData->pGunChunks[iGuns].yPos = pLight->yPos;
			gMEXData->pGunChunks[iGuns].zPos = pLight->zPos;

			gMEXData->pGunChunks[iGuns].xRot = pLight->xVec;
			gMEXData->pGunChunks[iGuns].yRot = pLight->yVec;
			gMEXData->pGunChunks[iGuns].zRot = pLight->zVec;

			gMEXData->pGunChunks[iGuns].coneAngle = pLight->coneAngle;
			gMEXData->pGunChunks[iGuns].edgeAngle = pLight->edgeAngle;

			iGuns ++;
		}
	
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}
}

void MEXParse_SetupEngines(void)
{
	LWSLightContainer *pLightContainer;
	LWSLight *pLight;
	char *pWorkStr, pDelimiter[] = {'_', 0x00};
	unsigned long nEngineChunks = 0, iEngines = 0;
	char tempName[100];
	
	// Count the number of chunks of engine chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		if(!strcmp(pWorkStr, "ENGINE"))
		{
			nEngineChunks ++;
		}
		
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}

	gMEXData->nEngineChunks = nEngineChunks;

	// Allocate enough space to hold all the engine chunks.
	gMEXData->pEngineChunks = new MEXEngineChunk [nEngineChunks];

	// Loop through and assign the chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		if(!strcmp(pWorkStr, "ENGINE"))
		{
			// Setup the engine.
			memset(gMEXData->pEngineChunks[iEngines].type, 0xAB, 4);
			strcpy(gMEXData->pEngineChunks[iEngines].type, "Eng");

			memset(gMEXData->pEngineChunks[iEngines].name, 0xAB, 8);
			sprintf(gMEXData->pEngineChunks[iEngines].name, "Eng%d", iEngines);

			gMEXData->pEngineChunks[iEngines].chunkSize = 32;

			gMEXData->pEngineChunks[iEngines].xPos = pLight->xPos;
			gMEXData->pEngineChunks[iEngines].yPos = pLight->yPos;
			gMEXData->pEngineChunks[iEngines].zPos = pLight->zPos;

			gMEXData->pEngineChunks[iEngines].xRot = pLight->xVec;
			gMEXData->pEngineChunks[iEngines].yRot = pLight->yVec;
			gMEXData->pEngineChunks[iEngines].zRot = pLight->zVec;

			gMEXData->pEngineChunks[iEngines].coneAngle = pLight->coneAngle;
			gMEXData->pEngineChunks[iEngines].edgeAngle = pLight->edgeAngle;

			iEngines ++;
		}
		
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}
}

void MEXParse_SetupDamage(void)
{
	LWSLightContainer *pLightContainer;
	LWSLight *pLight;
	char *pWorkStr, pDelimiter[] = {'_', 0x00};
	unsigned long nDamageChunks = 0, iDamage = 0;
	char tempName[100];
	
	// Count the number of chunks of damage chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		if(!strcmp(pWorkStr, "DAMAGE"))
		{
			nDamageChunks ++;
		}
		
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}

	gMEXData->nDamageChunks = nDamageChunks;

	// Allocate enough space to hold all the damage chunks.
	gMEXData->pDamageChunks = new MEXDamageChunk [nDamageChunks];

	// Loop through and assign the chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		if(!strcmp(pWorkStr, "DAMAGE"))
		{
			// Setup the damage light.
			memset(gMEXData->pDamageChunks[iDamage].type, 0xAB, 4);
			strcpy(gMEXData->pDamageChunks[iDamage].type, "Dmg");

			memset(gMEXData->pDamageChunks[iDamage].name, 0xAB, 8);
			sprintf(gMEXData->pDamageChunks[iDamage].name, "Dmg%d", iDamage);

			gMEXData->pDamageChunks[iDamage].chunkSize = 32;

			gMEXData->pDamageChunks[iDamage].xPos = pLight->xPos;
			gMEXData->pDamageChunks[iDamage].yPos = pLight->yPos;
			gMEXData->pDamageChunks[iDamage].zPos = pLight->zPos;

			gMEXData->pDamageChunks[iDamage].xRot = pLight->xVec;
			gMEXData->pDamageChunks[iDamage].yRot = pLight->yVec;
			gMEXData->pDamageChunks[iDamage].zRot = pLight->zVec;

			gMEXData->pDamageChunks[iDamage].coneAngle = pLight->coneAngle;
			gMEXData->pDamageChunks[iDamage].edgeAngle = pLight->edgeAngle;

			iDamage ++;
		}
		
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}
}

#define MAX_DOCKING			(50)
void MEXParse_SetupDocking(void)
{
	LWSLightContainer *pLightContainer;
	LWSLight *pLight;
	char *pWorkStr, pDelimiter[] = {'_', 0x00}, *realName;
	unsigned long nDockingChunks = 0, iDock = 0;
	char dockingNames[MAX_DOCKING][50];
	char tempName[100];
	
	// Count the number of chunks dock chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		// Is it a dock?
		if(!strcmp(pWorkStr, "DOCK"))
		{
			// Get the real name of the dock.
			realName = strtok(NULL, pDelimiter);
			realName = strupr(realName);

			// Make sure it's less than 50 chars.
			if(strlen(realName) >= 49)
				realName[49] = 0x00;

			strcpy((char *)dockingNames[nDockingChunks], realName);
		
			nDockingChunks ++;
		}

		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}

	gMEXData->nDockingChunks = nDockingChunks;

	if(gMEXData->nDockingChunks > 0)
	{
		// Allocate enough space to hold all the dock chunks.
		gMEXData->pDockingChunks = new MEXDockingChunk [nDockingChunks];
	}

	// Loop through and assign the chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		// Is it a dock?
		if(!strcmp(pWorkStr, "DOCK"))
		{
			// Setup the dock.
			memset(gMEXData->pDockingChunks[iDock].type, 0xAB, 4);
			strcpy(gMEXData->pDockingChunks[iDock].type, "Dok");

			memset(gMEXData->pDockingChunks[iDock].name, 0xAB, 8);
			sprintf(gMEXData->pDockingChunks[iDock].name, "Dock");

			gMEXData->pDockingChunks[iDock].chunkSize = 52;

			gMEXData->pDockingChunks[iDock].xPos = pLight->xPos;
			gMEXData->pDockingChunks[iDock].yPos = pLight->yPos;
			gMEXData->pDockingChunks[iDock].zPos = pLight->zPos;

			gMEXData->pDockingChunks[iDock].xRot = pLight->xVec;
			gMEXData->pDockingChunks[iDock].yRot = pLight->yVec;
			gMEXData->pDockingChunks[iDock].zRot = pLight->zVec;

			gMEXData->pDockingChunks[iDock].coneAngle = pLight->coneAngle;
			gMEXData->pDockingChunks[iDock].edgeAngle = pLight->edgeAngle;

			strcpy(gMEXData->pDockingChunks[iDock].dockName, dockingNames[iDock]);

			iDock ++;
		}
	
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}
}

#define MAX_SALVAGE			(20)
void MEXParse_SetupSalvage(void)
{
	LWSLightContainer *pLightContainer;
	LWSLight *pLight;
	char *pWorkStr, pDelimiter[] = {'_', 0x00}, *realName;
	unsigned long nSalvageChunks = 0, iSalvage = 0;
	char salvageNames[MAX_SALVAGE][50];
	char tempName[100];
	
	// Count the number of chunks dock chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		// Is it a dock?
		if(!strcmp(pWorkStr, "SALVAGE"))
		{
			// Get the real name of the dock.
			realName = strtok(NULL, pDelimiter);
			realName = strupr(realName);

			// Make sure it's less than 50 chars.
			if(strlen(realName) >= 49)
				realName[49] = 0x00;

			strcpy((char *)salvageNames[nSalvageChunks], realName);
		
			nSalvageChunks ++;
		}

		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}

	gMEXData->nSalvageChunks = nSalvageChunks;

	if(gMEXData->nSalvageChunks > 0)
	{
		// Allocate enough space to hold all the dock chunks.
		gMEXData->pSalvageChunks = new MEXSalvageChunk [nSalvageChunks];
	}

	// Loop through and assign the chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		// Is it a dock?
		if(!strcmp(pWorkStr, "SALVAGE"))
		{
			// Setup the dock.
			memset(gMEXData->pSalvageChunks[iSalvage].type, 0xAB, 4);
			strcpy(gMEXData->pSalvageChunks[iSalvage].type, "Sal");

			memset(gMEXData->pSalvageChunks[iSalvage].name, 0xAB, 8);
			sprintf(gMEXData->pSalvageChunks[iSalvage].name, "SALVAGE");

			gMEXData->pSalvageChunks[iSalvage].chunkSize = 52;

			gMEXData->pSalvageChunks[iSalvage].xPos = pLight->xPos;
			gMEXData->pSalvageChunks[iSalvage].yPos = pLight->yPos;
			gMEXData->pSalvageChunks[iSalvage].zPos = pLight->zPos;

			gMEXData->pSalvageChunks[iSalvage].xRot = pLight->xVec;
			gMEXData->pSalvageChunks[iSalvage].yRot = pLight->yVec;
			gMEXData->pSalvageChunks[iSalvage].zRot = pLight->zVec;

			gMEXData->pSalvageChunks[iSalvage].coneAngle = pLight->coneAngle;
			gMEXData->pSalvageChunks[iSalvage].edgeAngle = pLight->edgeAngle;

			strcpy(gMEXData->pSalvageChunks[iSalvage].Name, salvageNames[iSalvage]);

			iSalvage ++;
		}
	
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}
}


#define MAX_NAVLIGHTS			(50)
void MEXParse_SetupNAVLights(void)
{
	LWSLightContainer *pLightContainer;
	LWSLight *pLight;
	char *pWorkStr, pDelimiter[] = {'_', 0x00}, *realName;
	unsigned long nNAVLightChunks = 0, iNavLight = 0;
	char NavLightNames[MAX_NAVLIGHTS][50];
	char tempName[100];
	
	// Count the number of nav light chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		// Is it a navlight?
		if(!strcmp(pWorkStr, "NAV"))
		{
			// Get the real name of the navlight.
			realName = strtok(NULL, pDelimiter);
			realName = strupr(realName);

			// Make sure it's less than 50 chars.
			if(strlen(realName) >= 49)
				realName[49] = 0x00;

			strcpy((char *)NavLightNames[nNAVLightChunks], realName);
		
			nNAVLightChunks ++;
		}

		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}

	gMEXData->nNAVLightChunks = nNAVLightChunks;

	if(gMEXData->nNAVLightChunks > 0)
	{
		// Allocate enough space to hold all the navlight chunks.
		gMEXData->pNAVLightChunks = new MEXNAVLightChunk [nNAVLightChunks];
	}

	// Loop through and assign the chunks.
	pLightContainer = (LWSLightContainer *)gLWSData->pLightList->GetHead();

	while(pLightContainer)
	{
		pLight = &pLightContainer->myLight;
		strcpy(tempName, pLight->name);

		pWorkStr = strtok(tempName, pDelimiter);
		pWorkStr = strupr(pWorkStr);
		
		// Is it a navlight?
		if(!strcmp(pWorkStr, "NAV"))
		{
			// Setup the dock.
			memset(gMEXData->pNAVLightChunks[iNavLight].type, 0xAB, 4);
			strcpy(gMEXData->pNAVLightChunks[iNavLight].type, "Nav");

			memset(gMEXData->pNAVLightChunks[iNavLight].name, 0xAB, 8);
			sprintf(gMEXData->pNAVLightChunks[iNavLight].name, "Nav");

			gMEXData->pNAVLightChunks[iNavLight].chunkSize = 36;

			gMEXData->pNAVLightChunks[iNavLight].xPos = pLight->xPos;
			gMEXData->pNAVLightChunks[iNavLight].yPos = pLight->yPos;
			gMEXData->pNAVLightChunks[iNavLight].zPos = pLight->zPos;

			gMEXData->pNAVLightChunks[iNavLight].red = pLight->red;
			gMEXData->pNAVLightChunks[iNavLight].green = pLight->green;
			gMEXData->pNAVLightChunks[iNavLight].blue = pLight->blue;

			strcpy(gMEXData->pNAVLightChunks[iNavLight].NAVLightName, NavLightNames[iNavLight]);

			iNavLight ++;
		}
	
		pLightContainer = (LWSLightContainer *)pLightContainer->GetNext();
	}
}

void MEXParse_SetupCollisionSpheres(void)
{
	PolygonObject *pObject;
	unsigned long i, j, k;
	float xMin, xMax, yMin, yMax, zMin, zMax, rX, rY, rZ;
	Vertex pi[3];
	
	xMin = yMin = zMin = (float)100000;
	xMax = yMax = zMax = (float)-100000;
	
	for( k=0 ; k<gGEOData->pGEOFileHeader->nPolygonObjects ; k++)
	{
		if(bitTest(gGEOData->pPolygonObjects[k].flags, POF_UniqueObject))
		{
			pObject = &gGEOData->pPolygonObjects[k];

			for( i=0 ; i<pObject->nPolygons ; i++)
			{
				// Only 3 point polygons at this point.
				pi[0] = gGEOData->pVertices[k][gGEOData->pPolygons[k][i].iV0];
				pi[1] = gGEOData->pVertices[k][gGEOData->pPolygons[k][i].iV1];
				pi[2] = gGEOData->pVertices[k][gGEOData->pPolygons[k][i].iV2];
				
				for( j=0 ; j<3 ; j++ )
				{
					if(xMin > pi[j].x)
						xMin = pi[j].x;

					if(yMin > pi[j].y)
						yMin = pi[j].y;

					if(zMin > pi[j].z)
						zMin = pi[j].z;

					if(xMax < pi[j].x)
						xMax = pi[j].x;

					if(yMax < pi[j].y)
						yMax = pi[j].y;

					if(zMax < pi[j].z)
						zMax = pi[j].z;
				}
			}
		}
	}

	// Just set up the one big sphere.
	gMEXData->pColChunks = new MEXCollisionSphereChunk [1];
	memset(&gMEXData->pColChunks[0], 0x00, sizeof(MEXCollisionSphereChunk));
	gMEXData->nColChunks = 1;
	
	memset(gMEXData->pColChunks[0].type, 0xAB, 4);
	strcpy(gMEXData->pColChunks[0].type, "Col");

	memset(gMEXData->pColChunks[0].name, 0xAB, 8);
	sprintf(gMEXData->pColChunks[0].name, "Col%d", 0);

	gMEXData->pColChunks[0].chunkSize = 20;
	
	gMEXData->pColChunks[0].level = 0;

	gMEXData->pColChunks[0].x = ((xMax - xMin) / 2) + xMin;
	gMEXData->pColChunks[0].y = ((yMax - yMin) / 2) + yMin;
	gMEXData->pColChunks[0].z = ((zMax - zMin) / 2) + zMin;

	rX = xMax - gMEXData->pColChunks[0].x;
	rY = yMax - gMEXData->pColChunks[0].y;
	rZ = zMax - gMEXData->pColChunks[0].z;

	gMEXData->pColChunks[0].r = rX;

	if(rY > gMEXData->pColChunks[0].r)
		gMEXData->pColChunks[0].r = rY;

	if(rZ > gMEXData->pColChunks[0].r)
		gMEXData->pColChunks[0].r = rZ;
}

void MEXParse_SetupCollisionRectangles(void)
{
	// Here we have to parse the .GEO data because it's already been transformed into
	// Homeworld space.

	unsigned long i, j;
	float xMin, xMax, yMin, yMax, zMin, zMax;
	float ox, oy, oz, dx, dy, dz;

	xMin = yMin = zMin = (float)10000000.0;
	xMax = yMax = zMax = (float)-10000000.0;

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
		{
			for( j=0 ; j<gGEOData->pPolygonObjects[i].nVertices ; j++ )
			{
				if(gGEOData->pVertices[i][j].x < xMin)
					xMin = gGEOData->pVertices[i][j].x;

				if(gGEOData->pVertices[i][j].y < yMin)
					yMin = gGEOData->pVertices[i][j].y;

				if(gGEOData->pVertices[i][j].z < zMin)
					zMin = gGEOData->pVertices[i][j].z;

				if(gGEOData->pVertices[i][j].x > xMax)
					xMax = gGEOData->pVertices[i][j].x;

				if(gGEOData->pVertices[i][j].y > yMax)
					yMax = gGEOData->pVertices[i][j].y;

				if(gGEOData->pVertices[i][j].z > zMax)
					zMax = gGEOData->pVertices[i][j].z;
			}
		}
	}

	dx = xMax - xMin;
	dy = yMax - yMin;
	dz = zMax - zMin;

	ox = xMin;
	oy = yMin;
	oz = zMax - dz;

	// Just set up the one big sphere.
	gMEXData->pColRectChunks = new MEXCollisionRectangleChunk [1];
	memset(&gMEXData->pColRectChunks[0], 0x00, sizeof(MEXCollisionRectangleChunk));
	gMEXData->nColRectChunks = 1;
	
	memset(gMEXData->pColRectChunks[0].type, 0xAB, 4);
	strcpy(gMEXData->pColRectChunks[0].type, "Rct");

	memset(gMEXData->pColRectChunks[0].name, 0xAB, 8);
	sprintf(gMEXData->pColRectChunks[0].name, "Rct%d", 0);

	gMEXData->pColRectChunks[0].chunkSize = 28;
	
	gMEXData->pColRectChunks[0].level = 1;

	gMEXData->pColRectChunks[0].ox = ox;
	gMEXData->pColRectChunks[0].oy = oy;
	gMEXData->pColRectChunks[0].oz = oz;

	gMEXData->pColRectChunks[0].dx = dx;
	gMEXData->pColRectChunks[0].dy = dy;
	gMEXData->pColRectChunks[0].dz = dz;
}

signed char MEXParse_SetupMEXData(void)
{
	MEXParse_SetupGuns();

	MEXParse_SetupEngines();

	MEXParse_SetupDamage();

	MEXParse_SetupDocking();
	
    MEXParse_SetupSalvage();

	MEXParse_SetupNAVLights();

	MEXParse_SetupCollisionSpheres();
	
	MEXParse_SetupCollisionRectangles();

	// Setup the header.
	strcpy(gMEXData->pMEXFileHeader->identifier, MEX_FILE_ID);
	gMEXData->pMEXFileHeader->version = MEX_VERSION;
	gMEXData->pMEXFileHeader->nChunks = (unsigned short)gMEXData->nGunChunks +
										(unsigned short)gMEXData->nColChunks +
										(unsigned short)gMEXData->nColRectChunks +
										(unsigned short)gMEXData->nEngineChunks +
										(unsigned short)gMEXData->nDamageChunks +
										(unsigned short)gMEXData->nDockingChunks +
										(unsigned short)gMEXData->nSalvageChunks +
										(unsigned short)gMEXData->nNAVLightChunks;

	return(SUCCESS);
}

signed char MEXParse_WriteFile(FILE *fileHandle)
{
	signed char retVal;

	retVal = MEXParse_SetupMEXData();

	// Write out the header.
	fwrite(gMEXData->pMEXFileHeader, sizeof(MEXFileHeader), 1, fileHandle);

	// Write out the chunks.
	fwrite(gMEXData->pGunChunks, sizeof(MEXGunChunk), gMEXData->nGunChunks, fileHandle);

	fwrite(gMEXData->pEngineChunks, sizeof(MEXEngineChunk), gMEXData->nEngineChunks, fileHandle);

	fwrite(gMEXData->pDamageChunks, sizeof(MEXDamageChunk), gMEXData->nDamageChunks, fileHandle);

	fwrite(gMEXData->pDockingChunks, sizeof(MEXDockingChunk), gMEXData->nDockingChunks, fileHandle);
    
    fwrite(gMEXData->pSalvageChunks, sizeof(MEXSalvageChunk), gMEXData->nSalvageChunks, fileHandle);
    
	fwrite(gMEXData->pNAVLightChunks, sizeof(MEXNAVLightChunk), gMEXData->nNAVLightChunks, fileHandle);

	fwrite(gMEXData->pColChunks, sizeof(MEXCollisionSphereChunk), gMEXData->nColChunks, fileHandle);

	fwrite(gMEXData->pColRectChunks, sizeof(MEXCollisionRectangleChunk), gMEXData->nColRectChunks, fileHandle);

	return(retVal);
}
