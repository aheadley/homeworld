/*
** LWOPARSE.CPP : Code to handle loading and saving of Lightwave Object Files (.LWO).
*/

#include "assert.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"
#include "math.h"

#include "lwoparse.h"
#include "readwrite.h"
#include "export.h"
#include "cclist.h"
#include "lwmath.h"
#include "reorient.h"

LWOGlobalData	*gLWOData;

LWOContainer::LWOContainer(void)
{
	memset(&myObject, 0x00, sizeof(LWObject));

	myObject.objectMotions = new ccList;
}

LWOContainer::~LWOContainer(void)
{
	unsigned long i;

	delete [] myObject.fileName;

	delete [] myObject.fullFileName;

	for( i=0 ; i<myObject.nSurfaceNames ; i++ )
	{
		delete [] myObject.pSurfaceNames[i];
	}

	delete [] myObject.pSurfaceNames;

	for( i=0 ; i<myObject.nPolygons ; i++ )
	{
		delete [] myObject.pPolyList[i].piVertices;
	}

	delete [] myObject.pPolyList;

	delete [] myObject.pVertexList;

	delete [] myObject.pFaceNormalList;

	delete [] myObject.pVertexNormalList;

	for( i=0 ; i<myObject.nSurfaces ; i++ )
	{
		if(myObject.pSurfaceList[i].RIMG)
			delete [] myObject.pSurfaceList[i].RIMG;

		if(myObject.pSurfaceList[i].CTEX.TFPn)
			delete [] myObject.pSurfaceList[i].CTEX.TFPn;

		if(myObject.pSurfaceList[i].CTEX.TIPn)
			delete [] myObject.pSurfaceList[i].CTEX.TIPn;

		if(myObject.pSurfaceList[i].DTEX.TFPn)
			delete [] myObject.pSurfaceList[i].DTEX.TFPn;

		if(myObject.pSurfaceList[i].DTEX.TIPn)
			delete [] myObject.pSurfaceList[i].DTEX.TIPn;

		if(myObject.pSurfaceList[i].STEX.TFPn)
			delete [] myObject.pSurfaceList[i].STEX.TFPn;

		if(myObject.pSurfaceList[i].STEX.TIPn)
			delete [] myObject.pSurfaceList[i].STEX.TIPn;

		if(myObject.pSurfaceList[i].RTEX.TFPn)
			delete [] myObject.pSurfaceList[i].RTEX.TFPn;

		if(myObject.pSurfaceList[i].RTEX.TIPn)
			delete [] myObject.pSurfaceList[i].RTEX.TIPn;

		if(myObject.pSurfaceList[i].TTEX.TFPn)
			delete [] myObject.pSurfaceList[i].TTEX.TFPn;

		if(myObject.pSurfaceList[i].TTEX.TIPn)
			delete [] myObject.pSurfaceList[i].TTEX.TIPn;

		if(myObject.pSurfaceList[i].LTEX.TFPn)
			delete [] myObject.pSurfaceList[i].LTEX.TFPn;

		if(myObject.pSurfaceList[i].LTEX.TIPn)
			delete [] myObject.pSurfaceList[i].LTEX.TIPn;

		if(myObject.pSurfaceList[i].BTEX.TFPn)
			delete [] myObject.pSurfaceList[i].BTEX.TFPn;

		if(myObject.pSurfaceList[i].BTEX.TIPn)
			delete [] myObject.pSurfaceList[i].BTEX.TIPn;
	}

	delete [] myObject.pSurfaceList;

	delete myObject.objectMotions;
}

LWOKeyframeContainer::LWOKeyframeContainer(void)
{
	memset(&myKeyframe, 0x00, sizeof(LWOKeyframe));
}

LWOKeyframeContainer::~LWOKeyframeContainer(void)
{
	memset(&myKeyframe, 0x00, sizeof(LWOKeyframe));
}

void InitializeLWOParse(void)
{
	gLWOData = new LWOGlobalData;

	memset(gLWOData, 0x00, sizeof(LWOGlobalData));

	// Initialize Specifics.
	gLWOData->pLWOList = new ccList;
    gLWOData->pNULLList = new ccList;

	gLWOData->pLWOFuncList = new FunctionDef [NUM_LWO_PARSE_FUNCS];

	// Add parsing functions for LWS file.
	AddParseFunc(&gLWOData->pLWOFuncList[0], "FORM", (void *)LWOParse_FORM);
	AddParseFunc(&gLWOData->pLWOFuncList[1], "PNTS", (void *)LWOParse_PNTS);
	AddParseFunc(&gLWOData->pLWOFuncList[2], "SRFS", (void *)LWOParse_SRFS);
	AddParseFunc(&gLWOData->pLWOFuncList[3], "POLS", (void *)LWOParse_POLS);
	AddParseFunc(&gLWOData->pLWOFuncList[4], "SURF", (void *)LWOParse_SURF);
}

void DeInitializeLWOParse(void)
{
	unsigned long i;

	for( i=0 ; i<NUM_LWO_PARSE_FUNCS ; i++ )
		RemParseFunc(&gLWOData->pLWOFuncList[i]);
	
	delete [] gLWOData->pLWOFuncList;

	delete gLWOData->pLWOList;
	delete gLWOData->pNULLList;

	delete gLWOData;
}

LWOContainer *CreateNewLWObject(char*fileName)
{
	LWOContainer *pRet;

	pRet = new LWOContainer;

	// Object is cleared in construct.
	
	pRet->myObject.fileName = new char [strlen(fileName) + 1];
	pRet->myObject.fullFileName = new char [strlen(fileName) + 1];
	strcpy(pRet->myObject.fileName, fileName);
	strcpy(pRet->myObject.fullFileName, fileName);
	
	gLWOData->pCurrentLWOContainer = pRet;

	return(pRet);
}

void LWOParse_AddNullObject(char *name)
{
	LWOContainer *pTemp;

	pTemp = new LWOContainer;

	// Object is cleared in construct.
	
	pTemp->myObject.fileName = new char [strlen(name) + 1];;
	strcpy(pTemp->myObject.fileName, name);
	pTemp->myObject.fullFileName = NULL;
	bitSet(pTemp->myObject.flags, LWOF_NullObject);

	gLWOData->pCurrentLWOContainer = pTemp;

	gLWOData->pLWOList->AddNode(pTemp);
}

LWOKeyframeContainer *CreateNewLWOKeyframe(void)
{
	LWOKeyframeContainer *pRet;

	pRet = new LWOKeyframeContainer;

	// Keyframe is cleared in construct.

	return(pRet);
}

signed long CheckDuplicateLWOFile(LWOContainer *pLWOContainer)
{
	LWOContainer *pTemp;
	signed long i;

	pTemp = (LWOContainer *)gLWOData->pLWOList->GetHead();
	i = 0;

	while(pTemp)
	{
		// Don't check myself.
		if(pTemp != pLWOContainer)
		{
			if(!bitTest(pTemp->myObject.flags, LWOF_NullObject))
			{
				if(!strcmp(pLWOContainer->myObject.fullFileName, pTemp->myObject.fullFileName))
					return(i);
			}
		}
		
		pTemp = (LWOContainer *)pTemp->GetNext();
		i ++;
	}
	
	return(UNIQUE);
}

signed char ConvertToTriangles(void)
{
	unsigned long	i;
	LWObject		*pObject;
	LWOPolygon		*pPolygon;
	
	pObject = &gLWOData->pCurrentLWOContainer->myObject;

	for( i=0 ; i<pObject->nPolygons ; i++ )
	{
		pPolygon = &pObject->pPolyList[i];

		if(pPolygon->niVertices != 3)
		{
			printf("LWExport\t: Only 3 point polygons are support right now.\n");

			return(FAILURE);
		}
	}

	return(SUCCESS);
}

signed char ReorderPointOrder(void)
{
	unsigned long	i;
	LWObject		*pObject;
	LWOPolygon		*pPolygon;
	unsigned short	iTemp;

	// Assume 3 point polygons here b/c should have already been cut up.
	
	pObject = &gLWOData->pCurrentLWOContainer->myObject;

	for( i=0 ; i<pObject->nPolygons ; i++ )
	{
		pPolygon = &pObject->pPolyList[i];

		iTemp = pPolygon->piVertices[1];	
		pPolygon->piVertices[1] = pPolygon->piVertices[2];
		pPolygon->piVertices[2] = iTemp;
	}

	return(SUCCESS);
}

signed char CalculateFaceNormals(void)
{
	unsigned long	i, j, nUniqueNormals;
	unsigned char	foundDuplicateNormal;
	LWObject		*pObject;
	LWOPolygon		*pPolygon;
	LWOVertex		*p0, *p1, *p2;
	LWONormal		v0, v1, vN, *pTempNormalList;

	pObject = &gLWOData->pCurrentLWOContainer->myObject;

	pTempNormalList = new LWONormal [pObject->nPolygons];

	for( i=0, nUniqueNormals=0 ; i<pObject->nPolygons ; i++)
	{
		pPolygon = &pObject->pPolyList[i];

		// At this point, all the polygons should be 3 point.
		assert(pPolygon->niVertices == 3);

		p0 = &pObject->pVertexList[pPolygon->piVertices[0]];
		p1 = &pObject->pVertexList[pPolygon->piVertices[1]];
		p2 = &pObject->pVertexList[pPolygon->piVertices[2]];

		v0.x = p1->x - p0->x;
		v0.y = p1->y - p0->y;
		v0.z = p1->z - p0->z;

		v1.x = p2->x - p0->x;
		v1.y = p2->y - p0->y;
		v1.z = p2->z - p0->z;

		vecCrossProduct(vN, v0, v1);

		vecNormalize(&vN);

#ifdef ANAL_DEBUGGING
	printf("x=(%d, %d, %d), y=(%d, %d, %d), z=(%d, %d, %d)", p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
#endif

		// Check to see if we've already calculated this normal.
		for( j=0, foundDuplicateNormal = 0 ; j<i ; j++ )
		{
			if(CompareNormals(&vN, &pTempNormalList[j]))
			{
				pPolygon->iFaceNormal = (unsigned short)j;

				foundDuplicateNormal = 1;

				break;
			}
		}

		if(!foundDuplicateNormal)
		{
			pTempNormalList[nUniqueNormals] = vN;

			pPolygon->iFaceNormal = (unsigned short)nUniqueNormals;
			
			nUniqueNormals ++;
		}
	}

	// Allocate space to hold the unique face normals.
	pObject->nFaceNormals = nUniqueNormals;
	pObject->pFaceNormalList = new LWONormal [nUniqueNormals];

	for( i=0 ; i<nUniqueNormals ; i++ )
	{
		pObject->pFaceNormalList[i] = pTempNormalList[i];
	}
	
	delete [] pTempNormalList;	

	return(SUCCESS);
}

#define MAX_POLY_NORMALS	(25)

signed char CalculateVertexNormals(void)
{
	unsigned long	i, j, k, iPolyNormal, foundDuplicateNormal, nUniqueNormals=0;
	LWObject		*pObject;
	LWOPolygon		*pPolygon;
	LWOVertex		*pVertex;
	LWONormal		polyNormals[MAX_POLY_NORMALS], nTemp, *pTempNormalList;

	pObject = &gLWOData->pCurrentLWOContainer->myObject;

	pTempNormalList = new LWONormal [pObject->nVertices];
	
	for( i=0 ; i<pObject->nVertices ; i++ )
	{
		iPolyNormal = 0;
		pVertex = &pObject->pVertexList[i];

		for( j=0 ; j<pObject->nPolygons ; j++ )
		{
			pPolygon = &pObject->pPolyList[j];
			
			// Does this polygon use this vertex?
			if((i == pPolygon->piVertices[0]) ||
				(i == pPolygon->piVertices[1]) ||
				(i == pPolygon->piVertices[2]))
			{
				// Does this polygon have smoothing enabled on it's surface?
				if(bitTest(pObject->pSurfaceList[pPolygon->iSurface - 1].FLAG, SF_Smoothing))
				{
					// Make sure we haven't already used this normal.
					for( k=0, foundDuplicateNormal = 0; k<iPolyNormal ; k++)
					{
						if(CompareNormals(&pObject->pFaceNormalList[pPolygon->iFaceNormal],
							&polyNormals[k]))
						{
							foundDuplicateNormal = 1;	

							break;
						}
					}

					if(!foundDuplicateNormal)
					{
						// Save this normal.
						polyNormals[iPolyNormal] = pObject->pFaceNormalList[pPolygon->iFaceNormal];

						iPolyNormal ++;

						assert(iPolyNormal < MAX_POLY_NORMALS);
					}
				}
			}
		}
		
		if(iPolyNormal == 0)
		{
			pVertex->iVertexNormal = 0;

			continue;
		}
		
		memset(&nTemp, 0x00, sizeof(LWONormal));

		// Loop through all the used poly normals.  Get the vertex normal.
		for( j=0 ; j<iPolyNormal ; j++ )
		{
			nTemp.x += polyNormals[j].x;
			nTemp.y += polyNormals[j].y;
			nTemp.z += polyNormals[j].z;
		}

		vecNormalize(&nTemp);

		// Loop through all the vertex normals.  Is this one unique?
		// Check to see if we've already calculated this normal.
		for( j=0, foundDuplicateNormal = 0 ; j<nUniqueNormals ; j++ )
		{
			if(CompareNormals(&nTemp, &pTempNormalList[j]))
			{
				pVertex->iVertexNormal = (unsigned short)j + pObject->nFaceNormals;

				foundDuplicateNormal = 1;

				break;
			}
		}

		if(!foundDuplicateNormal)
		{
			pTempNormalList[nUniqueNormals] = nTemp;

			pVertex->iVertexNormal = (unsigned short)nUniqueNormals + pObject->nFaceNormals;
			
			nUniqueNormals ++;
		}
	}

	// Allocate space to hold the unique vertex normals.
	pObject->nVertexNormals = nUniqueNormals;
	pObject->pVertexNormalList = new LWONormal [nUniqueNormals];

	for( i=0 ; i<nUniqueNormals ; i++ )
	{
		pObject->pVertexNormalList[i] = pTempNormalList[i];
	}
	
	delete [] pTempNormalList;	

	return(SUCCESS);
}

void ReorientLWOData(void)
{
	unsigned long i;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;

	// Reorient the vertices in this object.
	for( i=0 ; i<pObject->nVertices ; i++ )
	{
		ShipReorientPosition(&pObject->pVertexList[i].x,
			&pObject->pVertexList[i].y,
			&pObject->pVertexList[i].z);
	}
}

void XYZToH(float x, float y, float z, float *h)
{
	if(x == 0.0 && z == 0.0)
		*h = (float)0.0;

	else
	{
		if(z == 0.0)
			*h = (x < 0.0) ? HALFPI : -HALFPI;

		else if(z < 0.0)
			*h = (float)-atan(x / z) + PI;

		else
			*h = (float)-atan(x / z);
	}
}

void XYZToHP(float x, float y, float z, float *h, float *p)
{
	if(x == 0.0 && z == 0.0)
	{
		*h = (float)0.0;

		if(y != 0.0)
			*p = (y < 0.0) ? -HALFPI : HALFPI;

		else
			*p = (float)0.0;
	}

	else
	{
		if(z == 0.0)
			*h = (x < 0.0) ? HALFPI : -HALFPI;

		else if(z < 0.0)
			*h = (float)-atan(x / z) + PI;

		else
			*h = (float)-atan(x / z);

		x = (float)sqrt(x * x + z * z);

		if(x == 0.0)
			*p = (y < 0.0) ? -HALFPI : HALFPI;
		else
			*p = (float)atan(y / x);
	}
}

float fract(float a)
{
	return(a - (float)floor(a));
}

signed char CalculateVertexTextureCoords(LWOVertex *p0, LWOTextureCoord *t0, LWOSurface *pSurface)
{
	float			x, y, z;
	LWOTexture		*ctex;
	float			lon, lat;

	ctex = &pSurface->CTEX;

	x = p0->x - ctex->TCTR.x;
	y = p0->y - ctex->TCTR.y;
	z = p0->z + ctex->TCTR.z;
/*
    if (!strcmp(pSurface->name, "bot1front"))
    {
        _asm nop
    }
*/
	switch(ctex->mappingType)
	{
	case LTMT_Planar:
		if(bitTest(ctex->TFLG, TF_MappingAxisX))
		{
			t0->s = (float)(-z / ctex->TSIZ.z + 0.5);
			t0->t = (float)(-y / ctex->TSIZ.y + 0.5);
		}

		if(bitTest(ctex->TFLG, TF_MappingAxisY))
		{
			t0->s = (float)(x / ctex->TSIZ.x + 0.5);
			t0->t = (float)(z / ctex->TSIZ.z + 0.5);
		}

		if(bitTest(ctex->TFLG, TF_MappingAxisZ))
		{
			t0->s = (float)(x / ctex->TSIZ.x + 0.5);
			t0->t = (float)(-y / ctex->TSIZ.y + 0.5);
		}
		
        //!!!
/*
        if (!strcmp(pSurface->name, "bot1front"))
        {
            printf("(%.2f, %.2f, %.2f) => (%.2f, %.2f)\n", x, y, z, t0->s, t0->t);
//            printf("(%.2f, %.2f, %.2f) + (%.2f, %.2f, %.2f) = (%.2f, %.2f, %.2f) => (%.2f, %.2f)\n", p0->x, p0->y, p0->z, ctex->TCTR.x, ctex->TCTR.y, ctex->TCTR.z, x, y, z, t0->s, t0->t);
        }
*/
        if (t0->s > 1.0f && t0->s < 1.001f)
        {                                                   //if juuust over 1
            t0->s = 1.0f;
        }
        if (t0->s < 0.0f && t0->s > -0.001f)
        {                                                   //if juuust under 0
            t0->s = 0.0f;
        }
        if (t0->t > 1.0f && t0->t < 1.001f)
        {                                                   //if juuust over 1
            t0->t = 1.0f;
        }
        if (t0->t < 0.0f && t0->t > -0.001f)
        {                                                   //if juuust under 0
            t0->t = 0.0f;
        }
		t0->u = fract(t0->s);
		t0->v = fract(t0->t);
		break;

	case LTMT_Cylindrical:
		if(bitTest(ctex->TFLG, TF_MappingAxisX))
		{
			XYZToH(z, x, -y, &lon);
			t0->t = (float)(-x / ctex->TSIZ.x + 0.5);
		}
		else if(bitTest(ctex->TFLG, TF_MappingAxisY))
		{
			XYZToH(-x, y, z, &lon);
			t0->t = (float)(-y / ctex->TSIZ.y + 0.5);
		}
		else // Z by default.
		{
			XYZToH(-x, z, -y, &lon);
			t0->t = (float)(-z / ctex->TSIZ.z + 0.5);
		}

		lon = (float)1.0 - lon / TWOPI;

		// Something to do with widthtiling affecting lon.  I can't find any reference to widthtiling
		// so I'll add it later if we need it.
		t0->u = fract(lon);
		t0->v = fract(t0->t);
		break;

	case LTMT_Spherical:
		if(bitTest(ctex->TFLG, TF_MappingAxisX))
			XYZToHP(z, x, -y, &lon, &lat);
		
		else if(bitTest(ctex->TFLG, TF_MappingAxisY))
			XYZToHP(-x, y, z, &lon, &lat);
		
		else // Z by default.
			XYZToHP(-x, z, -y, &lon, &lat);

		lon = (float)1.0 - lon / TWOPI;
		lat = (float)0.5 - lat / PI;

		// again with the widthtiling!!
		// now with the heighttiling!!

		t0->u = fract(lon);
		t0->v = fract(lat);
		break;

	default:
		break;
	}
	return(SUCCESS);
}

signed char CalculateTextureCoordinates(void)
{
	unsigned long	i;
	LWObject		*pObject;
	LWOPolygon		*pPolygon;
	LWOVertex		*p0, *p1, *p2;
	LWOTextureCoord *t0, *t1, *t2;
	LWOSurface		*pSurface;
	
	pObject = &gLWOData->pCurrentLWOContainer->myObject;

	for( i=0 ; i<pObject->nPolygons ; i++)
	{
		pPolygon = &pObject->pPolyList[i];

		pSurface = &pObject->pSurfaceList[pPolygon->iSurface - 1];

		// The only texture we care about is CTEX (color texture).
		if(!pSurface->ctexEnabled)
			continue;

		p0 = &pObject->pVertexList[pPolygon->piVertices[0]];
		p1 = &pObject->pVertexList[pPolygon->piVertices[1]];
		p2 = &pObject->pVertexList[pPolygon->piVertices[2]];

		t0 = &pObject->pPolyList[i].t0;
		t1 = &pObject->pPolyList[i].t1;
		t2 = &pObject->pPolyList[i].t2;

		CalculateVertexTextureCoords(p0, t0, pSurface);
		CalculateVertexTextureCoords(p1, t1, pSurface);
		CalculateVertexTextureCoords(p2, t2, pSurface);
	}
	
	return(SUCCESS);
}

void FixTextureName(char *pName)
{
	char *tName = new char [strlen(pName) + 1], *pWork;

	strcpy(tName, pName);

	// Is there an extension?
	if(pWork = strchr(tName, '.'))
	{
		*pWork = 0x00;

		strcpy(pName, tName);
	}

	// Is there a drive?
	if(pWork = strchr(tName, ':'))
	{
		*pWork = 0x00;
		pWork++;

		strcpy(pName , pWork);
	}

	// Is there a path?
	if((pWork = strchr(pName, '\\')) || (pWork = strchr(pName, '/')))
	{
		// Find the last one.
		do
		{
			pWork++;

			strcpy(pName, pWork);
		} while((pWork = strchr(pName, '\\')) || (pWork = strchr(pName, '/')));
	}

	delete [] tName;
}

signed char LWOParse_ReadFile(FILE *fileHandle)
{
	ParseFuncPtr pParseFunc;
	char tagBuffer[5];
	signed char retVal = SUCCESS;
	unsigned long skipSize, i;

	while((!feof(fileHandle)) && (retVal != FAILURE))
	{
		memset(tagBuffer, 0xAB, 5);
		
		ReadIFFTag(tagBuffer, fileHandle);

		if(feof(fileHandle))
			break;

		if(pParseFunc = IsParseFunc(tagBuffer, gLWOData->pLWOFuncList, NUM_LWO_PARSE_FUNCS))
			retVal = pParseFunc(tagBuffer, fileHandle);
		
		else
		{
			skipSize = ReadLong(fileHandle);

			for( i=0 ; i<skipSize ; i++ )
				ReadByte(fileHandle);
		}
	}
	
	if(retVal == SUCCESS)
		retVal = ConvertToTriangles();

	if(retVal == SUCCESS)
		retVal = ReorderPointOrder();

	if(retVal == SUCCESS)
		retVal = CalculateTextureCoordinates();
	
	if(retVal == SUCCESS)
		ReorientLWOData();

	if(retVal == SUCCESS)
		retVal = CalculateFaceNormals();

	if(retVal == SUCCESS)
		retVal = CalculateVertexNormals();

	return(retVal);
}

signed char LWOParse_FORM(char *currentLine, FILE *fileHandle)
{
	char tagBuffer[5];

	// Pick off the filesize.  Don't need it.
	ReadLong(fileHandle);

	// Grab the file tag.
	ReadIFFTag(tagBuffer, fileHandle);

	if(strcmp(tagBuffer, "LWOB"))
	{
		printf("LWExport\t: Not a LWO file!.\n");
		return(FAILURE);
	}
	
	return(SUCCESS);
}

#define LWOVERTEX_DISK_SIZE			(12)
signed char LWOParse_PNTS(char *currentLine, FILE *fileHandle)
{
	unsigned long chunkSize, i;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	
	// Grab the size of the chunks.
	chunkSize = ReadLong(fileHandle);

	// Calculate the number of points.
	pObject->nVertices = chunkSize / LWOVERTEX_DISK_SIZE; // Don't laugh.  This is the way LW does it.
	
	// Allocate space to hold the point list.
	pObject->pVertexList = new LWOVertex [pObject->nVertices];

	for( i=0 ; i<pObject->nVertices ; i++ )
	{
		pObject->pVertexList[i].x = ReadFloat(fileHandle);
		pObject->pVertexList[i].y = ReadFloat(fileHandle);
		pObject->pVertexList[i].z = ReadFloat(fileHandle);
	}
	
	return(SUCCESS);
}

signed char LWOParse_SRFS(char *currentLine, FILE *fileHandle)
{
	unsigned long chunkSize, i, tStrLen;
	char *stringBuffer, *pTemp;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;

	// Grab the size of this chunk.
	chunkSize = ReadLong(fileHandle);

	// Allocate space for the strings.
	stringBuffer = new char [chunkSize];

	// Get the data.
	fread(stringBuffer, 1, chunkSize, fileHandle);

	// Count the strings.
	for( i=0, pObject->nSurfaceNames=0; i<chunkSize ; i++)
	{
		if((!stringBuffer[i]) && (!((i+1)%2)))
			pObject->nSurfaceNames ++;
	}

	// Allocate pointers to the strings.
	pObject->pSurfaceNames = new char * [pObject->nSurfaceNames];
	pTemp = stringBuffer;

	// Copy over each string
	for( i=0 ; i<pObject->nSurfaceNames ; i++ )
	{
		// Get the length of this string.
		tStrLen = strlen(pTemp) + 1;

		// Allocate space to hold it.
		pObject->pSurfaceNames[i] = new char [tStrLen];

		strcpy(pObject->pSurfaceNames[i], pTemp);

		pTemp += tStrLen + (tStrLen % 2);
	}
	
	delete [] stringBuffer;

	// Set up the surfaces.
	pObject->nSurfaces = pObject->nSurfaceNames;
	pObject->pSurfaceList = new LWOSurface [pObject->nSurfaces];

	for( i=0 ; i<pObject->nSurfaces ; i++ )
	{
		memset(&pObject->pSurfaceList[i], 0x00, sizeof(LWOSurface));
	}

	return(SUCCESS);
}

signed char LWOParse_POLS(char *currentLine, FILE *fileHandle)
{
	unsigned short *pTempBuffer;
	unsigned long chunkSize, tempOffset = 0, i, j;
	unsigned short numVerts;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;

	// Get the chunk size.
	chunkSize = ReadLong(fileHandle);

	// Allocate space to hold it.
	pTempBuffer = new unsigned short [chunkSize / 2];

	// Read it in.
	for( i=0 ; i<chunkSize / 2 ; i++ )
		pTempBuffer[i] = ReadWord(fileHandle);

	pObject->nPolygons = 0;

	while(tempOffset < chunkSize/2)
	{
		numVerts = pTempBuffer[tempOffset];

		tempOffset += 1 + numVerts + 1;

		pObject->nPolygons ++;
	}

	pObject->pPolyList = new LWOPolygon [pObject->nPolygons];

	for( i=0, tempOffset=0; i<pObject->nPolygons ; i++ )
	{
		pObject->pPolyList[i].niVertices = pTempBuffer[tempOffset];
		
		tempOffset ++;
		
		// Allocate enough space for the vertices.
		pObject->pPolyList[i].piVertices = new unsigned short [pObject->pPolyList[i].niVertices];

		for( j=0 ; j<pObject->pPolyList[i].niVertices ; j++, tempOffset ++ )
		{
			pObject->pPolyList[i].piVertices[j] = pTempBuffer[tempOffset];
		}

		pObject->pPolyList[i].iSurface = pTempBuffer[tempOffset];

		memset(&pObject->pPolyList[i].t0, 0x00, sizeof(LWOTextureCoord));
		memset(&pObject->pPolyList[i].t1, 0x00, sizeof(LWOTextureCoord));
		memset(&pObject->pPolyList[i].t2, 0x00, sizeof(LWOTextureCoord));

		tempOffset ++;
	}

	delete [] pTempBuffer;
	// Get rid of the temp memory.
	
	return(SUCCESS);
}

#define NUM_SURF_SUB_CHUNK_PARSE_FUNCS	(41)

signed char LWOParse_SURF(char *currentLine, FILE *fileHandle)
{
	unsigned long i;
	signed long chunkSize;
	FunctionDef *pSURFSubChunks;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	char *surfaceName, tagBuffer[5];
	ParseFuncPtr pParseFunc;
	unsigned short skipSize;
	LWOSurface *pSurface;

	// Set up the parse funcs.
	pSURFSubChunks = new FunctionDef [NUM_SURF_SUB_CHUNK_PARSE_FUNCS];

	AddParseFunc(&pSURFSubChunks[0], "COLR", (void *)LWOParse_COLR);
	AddParseFunc(&pSURFSubChunks[1], "FLAG", (void *)LWOParse_FLAG);

	AddParseFunc(&pSURFSubChunks[2], "VLUM", (void *)LWOParse_VLUM);
	AddParseFunc(&pSURFSubChunks[3], "VDIF", (void *)LWOParse_VDIF);
	AddParseFunc(&pSURFSubChunks[4], "VSPC", (void *)LWOParse_VSPC);
	AddParseFunc(&pSURFSubChunks[5], "VRFL", (void *)LWOParse_VRFL);
	AddParseFunc(&pSURFSubChunks[6], "VTRN", (void *)LWOParse_VTRN);

	AddParseFunc(&pSURFSubChunks[7], "LUMI", (void *)LWOParse_LUMI);
	AddParseFunc(&pSURFSubChunks[8], "DIFF", (void *)LWOParse_DIFF);
	AddParseFunc(&pSURFSubChunks[9], "SPEC", (void *)LWOParse_SPEC);
	AddParseFunc(&pSURFSubChunks[10], "REFL", (void *)LWOParse_REFL);
	AddParseFunc(&pSURFSubChunks[11], "TRAN", (void *)LWOParse_TRAN);

	AddParseFunc(&pSURFSubChunks[12], "GLOS", (void *)LWOParse_GLOS);
	AddParseFunc(&pSURFSubChunks[13], "RFLT", (void *)LWOParse_RFLT);
	AddParseFunc(&pSURFSubChunks[14], "RIMG", (void *)LWOParse_RIMG);
	AddParseFunc(&pSURFSubChunks[15], "RSAN", (void *)LWOParse_RSAN);
	AddParseFunc(&pSURFSubChunks[16], "RIND", (void *)LWOParse_RIND);
	AddParseFunc(&pSURFSubChunks[17], "EDGE", (void *)LWOParse_EDGE);
	AddParseFunc(&pSURFSubChunks[18], "SMAN", (void *)LWOParse_SMAN);

	AddParseFunc(&pSURFSubChunks[19], "CTEX", (void *)LWOParse_CTEX);
	AddParseFunc(&pSURFSubChunks[20], "DTEX", (void *)LWOParse_DTEX);
	AddParseFunc(&pSURFSubChunks[21], "STEX", (void *)LWOParse_STEX);
	AddParseFunc(&pSURFSubChunks[22], "RTEX", (void *)LWOParse_RTEX);
	AddParseFunc(&pSURFSubChunks[23], "TTEX", (void *)LWOParse_TTEX);
	AddParseFunc(&pSURFSubChunks[24], "LTEX", (void *)LWOParse_LTEX);
	AddParseFunc(&pSURFSubChunks[25], "BTEX", (void *)LWOParse_BTEX);

	AddParseFunc(&pSURFSubChunks[26], "TFLG", (void *)LWOParse_TFLG);
	AddParseFunc(&pSURFSubChunks[27], "TSIZ", (void *)LWOParse_TSIZ);
	AddParseFunc(&pSURFSubChunks[28], "TCTR", (void *)LWOParse_TCTR);
	AddParseFunc(&pSURFSubChunks[29], "TFAL", (void *)LWOParse_TFAL);
	AddParseFunc(&pSURFSubChunks[30], "TVEL", (void *)LWOParse_TVEL);
	AddParseFunc(&pSURFSubChunks[31], "TCLR", (void *)LWOParse_TCLR);
	AddParseFunc(&pSURFSubChunks[32], "TVAL", (void *)LWOParse_TVAL);
	AddParseFunc(&pSURFSubChunks[33], "TAMP", (void *)LWOParse_TAMP);
	AddParseFunc(&pSURFSubChunks[34], "TFPn", (void *)LWOParse_TFPn);
	AddParseFunc(&pSURFSubChunks[35], "TIPn", (void *)LWOParse_TIPn);
	AddParseFunc(&pSURFSubChunks[36], "TIMG", (void *)LWOParse_TIMG);
	AddParseFunc(&pSURFSubChunks[37], "TALP", (void *)LWOParse_TALP);
	AddParseFunc(&pSURFSubChunks[38], "TWRP", (void *)LWOParse_TWRP);
	AddParseFunc(&pSURFSubChunks[39], "TAAS", (void *)LWOParse_TAAS);
	AddParseFunc(&pSURFSubChunks[40], "TOPC", (void *)LWOParse_TOPC);
	
	// Get the chunk size.
	chunkSize = ReadLong(fileHandle);

	// Grab the name of this surface.
	surfaceName = ReadString(fileHandle);

	// Determine which surface we're working on and set the index.
	for( i=0 ; i<pObject->nSurfaceNames ; i++ )
	{
		if(!strcmp(pObject->pSurfaceNames[i], surfaceName))
		{
			gLWOData->iCurrentSurface = i;

			break;
		}
	}
		
	chunkSize -= strlen(surfaceName) + 1;

	// To account for the odd byte padding on strings.
	if((strlen(surfaceName) + 1) % 2)
		chunkSize -= 1;

	while(chunkSize > 0)
	{
		ReadIFFTag(tagBuffer, fileHandle);

		// Size of tag.
		chunkSize -= 4;

		if(feof(fileHandle))
			break;

		if(pParseFunc = IsParseFunc(tagBuffer, pSURFSubChunks, NUM_SURF_SUB_CHUNK_PARSE_FUNCS))
			chunkSize -= pParseFunc(tagBuffer, fileHandle);
		
		else
		{
			skipSize = ReadWord(fileHandle);

			// size of skipSize;
			chunkSize -= 2;

			for( i=0 ; i<skipSize ; i++ )
			{
				ReadByte(fileHandle);
				chunkSize --;
			}
		}
			
	}
		
	// Is this the team base color or stripe color?
	pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
    pSurface->name = pObject->pSurfaceNames[gLWOData->iCurrentSurface];
	
	surfaceName = strupr(surfaceName);
	
	if(!strcmp(surfaceName, "BASECOLOR1"))
	{
		bitSet(pSurface->FLAG, SF_BaseColor);
	}

	if(!strcmp(surfaceName, "BASECOLOR2"))
	{
		bitSet(pSurface->FLAG, SF_StripeColor);
	}
	
	// Get rid of the parse funcs.
	for( i=0 ; i<NUM_SURF_SUB_CHUNK_PARSE_FUNCS ; i++ )
		RemParseFunc(&pSURFSubChunks[i]);
	
	// Get rid of the temp string.
	delete [] surfaceName;
	
	delete [] pSURFSubChunks;
	
	return(SUCCESS);
}

signed char LWOParse_COLR(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->COLR.r = ReadByte(fileHandle);
	pSurface->COLR.g = ReadByte(fileHandle);
	pSurface->COLR.b = ReadByte(fileHandle);
	pSurface->COLR.pad = ReadByte(fileHandle);
	
	return(chunkSize + 2);
}

signed char LWOParse_FLAG(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->FLAG = ReadWord(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_VLUM(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VLUM = ReadFloat(fileHandle);
		
    if (pSurface->VLUM == 1.0f)
    {
        bitSet(pSurface->FLAG, SF_Luminous);
    }
	return(chunkSize + 2);
}

signed char LWOParse_VDIF(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VDIF = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_VSPC(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VSPC = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_VRFL(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VRFL = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_VTRN(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VTRN = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_LUMI(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VLUM = ((float)ReadWord(fileHandle) / (float)256);
    if (pSurface->VLUM == 1.0f)
    {
        bitSet(pSurface->FLAG, SF_Luminous);
    }
		
	return(chunkSize + 2);
}

signed char LWOParse_DIFF(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VDIF = ((float)ReadWord(fileHandle) / (float)256);
		
	return(chunkSize + 2);
}

signed char LWOParse_SPEC(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VSPC = ((float)ReadWord(fileHandle) / (float)256);
		
	return(chunkSize + 2);
}

signed char LWOParse_REFL(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VRFL = ((float)ReadWord(fileHandle) / (float)256);
		
	return(chunkSize + 2);
}

signed char LWOParse_TRAN(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->VTRN = ((float)ReadWord(fileHandle) / (float)256);
		
	return(chunkSize + 2);
}

signed char LWOParse_GLOS(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->GLOS = ReadWord(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_RFLT(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->RFLT = ReadWord(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_RIMG(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->RIMG = ReadString(fileHandle);

	FixTextureName(pSurface->RIMG);

	return(chunkSize + 2);
}

signed char LWOParse_RSAN(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->RSAN = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_RIND(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->RIND = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_EDGE(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->EDGE = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_SMAN(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pSurface->SMAN = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_CTEX(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	char *tempStr;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	// Get the string.
	tempStr = ReadString(fileHandle);
	gLWOData->pCurrentTexture = &pSurface->CTEX;

	if(!strcmp(tempStr, "Planar Image Map"))
	{
		pSurface->CTEX.mappingType = LTMT_Planar;
	}

	if(!strcmp(tempStr, "Spherical Image Map"))
	{
		pSurface->CTEX.mappingType = LTMT_Spherical;
	}

	if(!strcmp(tempStr, "Cylindrical Image Map"))
	{
		pSurface->CTEX.mappingType = LTMT_Cylindrical;
	}

	pSurface->ctexEnabled = 1;

	delete [] tempStr;

	return(chunkSize + 2);
}

signed char LWOParse_DTEX(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	char *tempStr;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	// Get the string.
	tempStr = ReadString(fileHandle);
	gLWOData->pCurrentTexture = &pSurface->DTEX;

	if(!strcmp(tempStr, "Planar Image Map"))
	{
		pSurface->DTEX.mappingType = LTMT_Planar;
	}

	if(!strcmp(tempStr, "Spherical Image Map"))
	{
		pSurface->DTEX.mappingType = LTMT_Spherical;
	}

	if(!strcmp(tempStr, "Cylindrical Image Map"))
	{
		pSurface->DTEX.mappingType = LTMT_Cylindrical;
	}
	
	pSurface->dtexEnabled = 1;

	delete [] tempStr;

	return(chunkSize + 2);
}

signed char LWOParse_STEX(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	char *tempStr;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	// Get the string.
	tempStr = ReadString(fileHandle);
	gLWOData->pCurrentTexture = &pSurface->STEX;

	if(!strcmp(tempStr, "Planar Image Map"))
	{
		pSurface->STEX.mappingType = LTMT_Planar;
	}

	if(!strcmp(tempStr, "Spherical Image Map"))
	{
		pSurface->STEX.mappingType = LTMT_Spherical;
	}

	if(!strcmp(tempStr, "Cylindrical Image Map"))
	{
		pSurface->STEX.mappingType = LTMT_Cylindrical;
	}
	
	pSurface->stexEnabled = 1;

	delete [] tempStr;

	return(chunkSize + 2);
}

signed char LWOParse_RTEX(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	char *tempStr;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	// Get the string.
	tempStr = ReadString(fileHandle);
	gLWOData->pCurrentTexture = &pSurface->RTEX;

	if(!strcmp(tempStr, "Planar Image Map"))
	{
		pSurface->RTEX.mappingType = LTMT_Planar;
	}

	if(!strcmp(tempStr, "Spherical Image Map"))
	{
		pSurface->RTEX.mappingType = LTMT_Spherical;
	}

	if(!strcmp(tempStr, "Cylindrical Image Map"))
	{
		pSurface->RTEX.mappingType = LTMT_Cylindrical;
	}
	
	pSurface->rtexEnabled = 1;

	delete [] tempStr;

	return(chunkSize + 2);
}

signed char LWOParse_TTEX(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	char *tempStr;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	// Get the string.
	tempStr = ReadString(fileHandle);
	gLWOData->pCurrentTexture = &pSurface->TTEX;

	if(!strcmp(tempStr, "Planar Image Map"))
	{
		pSurface->TTEX.mappingType = LTMT_Planar;
	}

	if(!strcmp(tempStr, "Spherical Image Map"))
	{
		pSurface->TTEX.mappingType = LTMT_Spherical;
	}

	if(!strcmp(tempStr, "Cylindrical Image Map"))
	{
		pSurface->TTEX.mappingType = LTMT_Cylindrical;
	}
	
	pSurface->ttexEnabled = 1;

	delete [] tempStr;

	return(chunkSize + 2);
}

signed char LWOParse_LTEX(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	char *tempStr;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	// Get the string.
	tempStr = ReadString(fileHandle);
	gLWOData->pCurrentTexture = &pSurface->LTEX;

	if(!strcmp(tempStr, "Planar Image Map"))
	{
		pSurface->LTEX.mappingType = LTMT_Planar;
	}

	if(!strcmp(tempStr, "Spherical Image Map"))
	{
		pSurface->LTEX.mappingType = LTMT_Spherical;
	}

	if(!strcmp(tempStr, "Cylindrical Image Map"))
	{
		pSurface->LTEX.mappingType = LTMT_Cylindrical;
	}
	
	pSurface->ltexEnabled = 1;

	delete [] tempStr;

	return(chunkSize + 2);
}

signed char LWOParse_BTEX(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	char *tempStr;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	// Get the string.
	tempStr = ReadString(fileHandle);
	gLWOData->pCurrentTexture = &pSurface->BTEX;

	if(!strcmp(tempStr, "Planar Image Map"))
	{
		pSurface->BTEX.mappingType = LTMT_Planar;
	}

	if(!strcmp(tempStr, "Spherical Image Map"))
	{
		pSurface->BTEX.mappingType = LTMT_Spherical;
	}

	if(!strcmp(tempStr, "Cylindrical Image Map"))
	{
		pSurface->BTEX.mappingType = LTMT_Cylindrical;
	}
	
	pSurface->btexEnabled = 1;

	delete [] tempStr;

	return(chunkSize + 2);
}

signed char LWOParse_TFLG(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TFLG = ReadWord(fileHandle);

	return(chunkSize + 2);
}

signed char LWOParse_TSIZ(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

/*
    if (!strcmp(pTexture->TIMG, "botfront"))
    {
        _asm nop
    }
*/
	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TSIZ.x = ReadFloat(fileHandle);
	pTexture->TSIZ.y = ReadFloat(fileHandle);
	pTexture->TSIZ.z = -ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TCTR(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TCTR.x = ReadFloat(fileHandle);
	pTexture->TCTR.y = ReadFloat(fileHandle);
	pTexture->TCTR.z = -ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TFAL(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TFAL.x = ReadFloat(fileHandle);
	pTexture->TFAL.y = ReadFloat(fileHandle);
	pTexture->TFAL.z = -ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TVEL(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TVEL.x = ReadFloat(fileHandle);
	pTexture->TVEL.y = ReadFloat(fileHandle);
	pTexture->TVEL.z = -ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TCLR(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TCLR.r = ReadByte(fileHandle);
	pTexture->TCLR.g = ReadByte(fileHandle);
	pTexture->TCLR.b = ReadByte(fileHandle);
	pTexture->TCLR.pad = ReadByte(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TVAL(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TVAL = ReadWord(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TAMP(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TAMP = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TFPn(char *currentLine, FILE *fileHandle)
{
	unsigned long i;
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->numTFP = chunkSize / 4;

	pTexture->TFPn = new float [pTexture->numTFP];

	for( i=0 ; i<pTexture->numTFP ; i++ )
		pTexture->TFPn[i] = ReadFloat(fileHandle);

	return(chunkSize + 2);
}

signed char LWOParse_TIPn(char *currentLine, FILE *fileHandle)
{
	unsigned long i;
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->numTIP = chunkSize / 2;

	pTexture->TIPn = new signed short [pTexture->numTIP];

	for( i=0 ; i<pTexture->numTIP ; i++ )
		pTexture->TIPn[i] = ReadWord(fileHandle);

	return(chunkSize + 2);
}

signed char LWOParse_TIMG(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TIMG = ReadString(fileHandle);

	FixTextureName(pTexture->TIMG);

	return(chunkSize + 2);
}

signed char LWOParse_TALP(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TALP = ReadString(fileHandle);

	return(chunkSize + 2);
}

signed char LWOParse_TWRP(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TWRPw = ReadWord(fileHandle);
	pTexture->TWRPh = ReadWord(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TAAS(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TAAS = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}

signed char LWOParse_TOPC(char *currentLine, FILE *fileHandle)
{
	unsigned char chunkSize;
	LWObject *pObject = &gLWOData->pCurrentLWOContainer->myObject;
	LWOSurface *pSurface = &pObject->pSurfaceList[gLWOData->iCurrentSurface];
	LWOTexture *pTexture = gLWOData->pCurrentTexture;

	chunkSize = (unsigned char)ReadWord(fileHandle);

	pTexture->TOPC = ReadFloat(fileHandle);
		
	return(chunkSize + 2);
}
