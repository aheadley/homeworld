/*
** GEOPARSE.CPP : Code to handle the loading and saving of .GEO files.  (Relic Internal).
*/

#include "stdlib.h"
#include "search.h"
#include "assert.h"
#include "memory.h"
#include "string.h"
#include "math.h"

#include "geoparse.h"
#include "lwoparse.h"
#include "reorient.h"
#include "export.h"
#include "matrix.h"
#include "vector.h"

GEOGlobalData	*gGEOData;

void InitializeGEOParse(void)
{
	gGEOData = new GEOGlobalData;

	memset(gGEOData, 0x00, sizeof(GEOGlobalData));

	// Initialize Specifics.
	gGEOData->pGEOFileHeader = new GeoFileHeader;
	memset(gGEOData->pGEOFileHeader, 0x00, sizeof(GeoFileHeader));
}

void DeInitializeGEOParse(void)
{
	unsigned long i;

	if(!gGEOData)
		return;

	if(gGEOData->pMaterials)
		delete [] gGEOData->pMaterials;
	
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(gGEOData->pPolygons)
			delete [] gGEOData->pPolygons[i];
	}

	if(gGEOData->pPolygons)
		delete [] gGEOData->pPolygons;

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(gGEOData->pVertices)
			delete [] gGEOData->pVertices[i];
	}

	if(gGEOData->pVertices)
		delete [] gGEOData->pVertices;

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(gGEOData->pNormals)
			delete [] gGEOData->pNormals[i];
	}

	if(gGEOData->pNormals)
		delete [] gGEOData->pNormals;

	if(gGEOData->pPolygonObjects)
		delete [] gGEOData->pPolygonObjects;
	
	if(gGEOData->pGEOFileHeader)
		delete gGEOData->pGEOFileHeader;
	
	delete gGEOData;
}

unsigned long FindPolygonObjectOffset(unsigned long index)
{
	// Indices are 0 based so if the index is 0, the offset is invalid.
	if(!index)
		return(0);

	return(sizeof(GeoFileHeader) + (sizeof(PolygonObject) * (index - 1)));
}

signed char GEOParse_SetupHierarchy(void)
{
	unsigned long i, j;
	LWOContainer *pLWOContainer;
	LWObject *pLWObject;
	
	pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	// Set up the parents.
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		assert(pLWOContainer);
		
		pLWObject = &pLWOContainer->myObject;

		gGEOData->pPolygonObjects[i].oParent = FindPolygonObjectOffset(pLWObject->iParentObject);

		// Is there a parent object?
		if(gGEOData->pPolygonObjects[i].oParent)
		{
			// Force local for now...
			bitSet(gGEOData->pPolygonObjects[i].flags, POF_LocalParentObject);
		}
		
		pLWOContainer = (LWOContainer *)pLWOContainer->GetNext();
	}

	// Set up the children.
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

		for( j=0 ; j<gGEOData->pGEOFileHeader->nPolygonObjects ; j++ )
		{
			assert(pLWOContainer);

			pLWObject = &pLWOContainer->myObject;

			// Do I have a parent?
			if(pLWObject->iParentObject)
			{
				// Is my parent the current object?
				if(pLWObject->iParentObject - 1 == i)
				{
					gGEOData->pPolygonObjects[i].oChild = FindPolygonObjectOffset(j + 1);

					// Force local for now...
					bitSet(gGEOData->pPolygonObjects[i].flags, POF_LocalChildObject);

					// Only set up the first child b/c of the siblings.
					break;
				}
			}
			
			pLWOContainer = (LWOContainer *)pLWOContainer->GetNext();
		}
	}

	// Set up the siblings.
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		// If no parent, no siblings b/c root.
		if(!gGEOData->pPolygonObjects[i].oParent)
			continue;

		// Has a parent.  Any siblings?
		for( j=0 ; j<gGEOData->pGEOFileHeader->nPolygonObjects ; j++ )
		{
			// Same parent as me?
			if((gGEOData->pPolygonObjects[i].oParent == gGEOData->pPolygonObjects[j].oParent) &&
				(!gGEOData->pPolygonObjects[j].oSibling) && // Already has a sibling?
				(j != i)) // Not me.
			{
				gGEOData->pPolygonObjects[i].oSibling = FindPolygonObjectOffset(j + 1);

				// Force local for now...
				bitSet(gGEOData->pPolygonObjects[i].flags, POF_LocalSiblingObject);

				// Only want one sibling.  I'm the jealous type.
				break;
			}
		}
	}


	return(SUCCESS);
}

/*-----------------------------------------------------------------------------
    Name        : PolyCompareFunc
    Description : qsort omparison callback for sorting polygons by material.
                  Here is the sort priority:
                   - material textured/nontextured
                   - what texture it has
                   - matrial index
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int PolyCompareFunc(const void *arg1, const void *arg2)
{
	Polygon *p1, *p2;

	p1 = (Polygon *)arg1;
	p2 = (Polygon *)arg2;

    if (gGEOData->pMaterials[p2->iMaterial].oTextureName != gGEOData->pMaterials[p1->iMaterial].oTextureName)
    {                                                       //if only one textured
        if (gGEOData->pMaterials[p2->iMaterial].oTextureName != 0)
        {                                                   //if p2 textured
            return(1);
        }
        else
        {                                                   //else p1 textured
            return(0);
        }
    }
    else if (gGEOData->pMaterials[p2->iMaterial].oTextureName != 0 && gGEOData->pMaterials[p1->iMaterial].oTextureName != 0)
    {                                                       //both textured: refer to texture name
        assert(gGEOData->pTextureNames[p2->iMaterial]);
        assert(gGEOData->pTextureNames[p2->iMaterial] != (char *)0xffffffff);
        assert(gGEOData->pTextureNames[p1->iMaterial]);
        assert(gGEOData->pTextureNames[p1->iMaterial] != (char *)0xffffffff);
        if (_stricmp(gGEOData->pTextureNames[p1->iMaterial], gGEOData->pTextureNames[p2->iMaterial]) == 0)
        {                                                   //same texture name, use material index
        	return(p2->iMaterial - p1->iMaterial);
        }
        else
        {                                                   //different names; use difference of texture name
            return(_stricmp(gGEOData->pTextureNames[p1->iMaterial], gGEOData->pTextureNames[p2->iMaterial]));
        }
    }
    else
    {                                                       //none textured: use material index
    	return(p2->iMaterial - p1->iMaterial);
    }

    assert(FALSE);                                          //should never get here
    return(p2->iMaterial - p1->iMaterial);
}

signed char GEOParse_OptomizeModeChanges(void)
{
	unsigned long i;

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
		{
			qsort(gGEOData->pPolygons[i], gGEOData->pPolygonObjects[i].nPolygons,
				sizeof(Polygon), PolyCompareFunc);
		}
	}
	
	return(SUCCESS);
}

signed char GEOParse_BuildLocalMatrix(void)
{
	LWOContainer *pLWOContainer;
	LWObject *pLWObject;
	unsigned long i;
	vector trVector, scVector, rotVector;
	hmatrix tm, sm, result1, result2;
    matrix mat;

	pLWOContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		pLWObject = &pLWOContainer->myObject;

		trVector.x = pLWObject->xp;
		trVector.y = pLWObject->yp;
		trVector.z = pLWObject->zp;

		rotVector.x = pLWObject->h;
		rotVector.y = pLWObject->p;
		rotVector.z = pLWObject->b;

		scVector.x = pLWObject->xs;
		scVector.y = pLWObject->ys;
		scVector.z = pLWObject->zs;

		// Handle the rotation.
/*
		hmatMakeRotAboutX(&xrm, (float)cos(rotVector.x), (float)sin(rotVector.x));
		hmatMakeRotAboutY(&yrm, (float)cos(rotVector.y), (float)sin(rotVector.y));
		hmatMakeRotAboutZ(&zrm, (float)cos(rotVector.z), (float)sin(rotVector.z));

		hmatMultiplyHMatByHMat(&result1, &xrm, &zrm);
		hmatMultiplyHMatByHMat(&result2, &result1, &yrm);
*/
        nisShipEulerToMatrix(&mat, &rotVector);
        hmatMakeHMatFromMat(&result2, &mat);

		// Handle the translation.
		tm = IdentityHMatrix;
		tm.m14 = trVector.x;
		tm.m24 = trVector.y;
		tm.m34 = trVector.z;

		hmatMultiplyHMatByHMat(&result1, &tm, &result2);

		// Handle the scaling.
		sm = IdentityHMatrix;
		sm.m11 = scVector.x;
		sm.m22 = scVector.y;
		sm.m33 = scVector.z;

		hmatMultiplyHMatByHMat(&result2, &result1, &sm);

		gGEOData->pPolygonObjects[i].localMatrix = result2;
		
		pLWOContainer = (LWOContainer *)pLWOContainer->GetNext();
	}

	return(SUCCESS);
}

signed char GEOParse_SetupGEOData(void)
{
	unsigned long i, j, k, sVertexList, sNormalList, sPolyList, oPolyList, oNormalList, oVertexList;
	unsigned long nPublicMaterials, nLocalMaterials, oStringData, sStringData;
	unsigned short objectSurfaceOffset;
	LWOContainer *pContainer;
	LWObject *pObject;
	MaterialDescriptor  *pMaterial;
	LWOSurface			*pSurface;
    char **pTextureName;

	// Set this up now, because we need it later.
	gGEOData->pGEOFileHeader->nPolygonObjects = gLWOData->pLWOList->GetNumElements();

	///////////////////////////////////////////////////////////////////////////////
	// PolygonObject stuff...
	///////////////////////////////////////////////////////////////////////////////
	gGEOData->pPolygonObjects = new PolygonObject [gGEOData->pGEOFileHeader->nPolygonObjects];
	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	// Clear out the polygon objects so no un-initialized members.
	memset(gGEOData->pPolygonObjects, 0x00,
		sizeof(PolygonObject) * gGEOData->pGEOFileHeader->nPolygonObjects);

	sVertexList = sNormalList = sPolyList = sStringData = 0;
	nPublicMaterials = nLocalMaterials = 0;

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		pObject = &pContainer->myObject;
		
		gGEOData->pPolygonObjects[i].oName = 0; // Set this up below.
		gGEOData->pPolygonObjects[i].flags = 0; // Set this up below.

		if(bitTest(pObject->flags, LWOF_DuplicateObject))
			bitSet(gGEOData->pPolygonObjects[i].flags, POF_DuplicateObject);
		else
			bitSet(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject);

		gGEOData->pPolygonObjects[i].nVertices = pObject->nVertices;
		gGEOData->pPolygonObjects[i].nFaceNormals = pObject->nFaceNormals;
		gGEOData->pPolygonObjects[i].nVertexNormals = pObject->nVertexNormals;
		gGEOData->pPolygonObjects[i].nPolygons = pObject->nPolygons;
		gGEOData->pPolygonObjects[i].oVertexList = 0; // Set this up below.
		gGEOData->pPolygonObjects[i].oNormalList = 0; // Set this up below.
		gGEOData->pPolygonObjects[i].oPolygonList = 0; // Set this up below.
		gGEOData->pPolygonObjects[i].oParent = 0; // Set this up below.
		gGEOData->pPolygonObjects[i].oChild = 0; // Set this up below.
		gGEOData->pPolygonObjects[i].oSibling = 0; // Set this up below.

		sStringData += strlen(pObject->fileName) + 1;
		
		// Only increment this if it's a unique object.
		if(bitTest(pObject->flags, LWOF_UniqueObject))
		{
			sVertexList += pObject->nVertices * sizeof(Vertex);
			sNormalList += pObject->nFaceNormals * sizeof(Normal);
			sNormalList += pObject->nVertexNormals * sizeof(Normal);
			sPolyList += pObject->nPolygons * sizeof(Polygon);
		}

		for( j=0 ; j<pObject->nSurfaceNames ; j++ )
		{
			if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
			{
	//			if(globalMaterialCondition)
	//			{
					nPublicMaterials ++;
	//			}
	//			else
	//			{
	//				nLocalMaterials ++;
	//			}
			
				if(pContainer->myObject.pSurfaceList[j].CTEX.TIMG)
				{		
					// Only increment this if it's a unique object.
					if(bitTest(pContainer->myObject.flags, LWOF_UniqueObject))
						sStringData += strlen(pContainer->myObject.pSurfaceList[j].CTEX.TIMG) + 1;
				}
			}
		}

		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	///////////////////////////////////////////////////////////////////////////////
	// GEOFileHeader stuff...
	///////////////////////////////////////////////////////////////////////////////
	strcpy(gGEOData->pGEOFileHeader->identifier, GEO_FILE_ID);
	gGEOData->pGEOFileHeader->version = GEO_VERSION;
	gGEOData->pGEOFileHeader->oName = 0; // Set this up later.
	
	gGEOData->pGEOFileHeader->nPublicMaterials = nPublicMaterials;
	gGEOData->pGEOFileHeader->nLocalMaterials = nLocalMaterials;
		
	gGEOData->pGEOFileHeader->fileSize = // Skip the file header.
		sizeof(PolygonObject) * gGEOData->pGEOFileHeader->nPolygonObjects +
		sizeof(MaterialDescriptor) * gGEOData->pGEOFileHeader->nLocalMaterials +
		sizeof(MaterialDescriptor) * gGEOData->pGEOFileHeader->nPublicMaterials +
		sPolyList + sVertexList + sNormalList + sStringData;
	
	gGEOData->pGEOFileHeader->localSize =
		sVertexList + sNormalList + nLocalMaterials * sizeof(MaterialDescriptor);

	gGEOData->pGEOFileHeader->oLocalMaterials =
		sizeof(GeoFileHeader) +
		sizeof(PolygonObject) * gGEOData->pGEOFileHeader->nPolygonObjects;

	gGEOData->pGEOFileHeader->oPublicMaterials =
		gGEOData->pGEOFileHeader->oLocalMaterials +
		sizeof(MaterialDescriptor) *
		gGEOData->pGEOFileHeader->nLocalMaterials;
	
	memset(gGEOData->pGEOFileHeader->reserved, 0xAB, 24);

	///////////////////////////////////////////////////////////////////////////////
	// Second pass for offsets PolygonObject stuff...
	///////////////////////////////////////////////////////////////////////////////
	oPolyList = sizeof(GeoFileHeader) +
		sizeof(PolygonObject) * gGEOData->pGEOFileHeader->nPolygonObjects +
		sizeof(MaterialDescriptor) * nLocalMaterials +
		sizeof(MaterialDescriptor) * nPublicMaterials;

	oVertexList = oPolyList + sPolyList;

	oNormalList = oVertexList + sVertexList;

	oStringData = oNormalList + sNormalList;

	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	// Set up all the unique ones first.
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		pObject = &pContainer->myObject;

		// Each object has a unique name regardless of instancing.
		gGEOData->pPolygonObjects[i].oName = oStringData;
		oStringData += strlen(pObject->fileName) + 1;

		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
		{	
			gGEOData->pPolygonObjects[i].oPolygonList = oPolyList;
			gGEOData->pPolygonObjects[i].oVertexList = oVertexList;
			gGEOData->pPolygonObjects[i].oNormalList = oNormalList;
			
			oPolyList += sizeof(Polygon) * gGEOData->pPolygonObjects[i].nPolygons;
			oVertexList += sizeof(Vertex) * gGEOData->pPolygonObjects[i].nVertices;
			oNormalList += sizeof(Normal) * gGEOData->pPolygonObjects[i].nFaceNormals;
			oNormalList += sizeof(Normal) * gGEOData->pPolygonObjects[i].nVertexNormals;
		}

		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	// Now set up all the duplicates.
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		pObject = &pContainer->myObject;
		
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_DuplicateObject))
		{
			gGEOData->pPolygonObjects[i].oPolygonList =
				gGEOData->pPolygonObjects[pObject->duplicateIndex].oPolygonList;
			gGEOData->pPolygonObjects[i].oVertexList =
				gGEOData->pPolygonObjects[pObject->duplicateIndex].oVertexList;
			gGEOData->pPolygonObjects[i].oNormalList =
				gGEOData->pPolygonObjects[pObject->duplicateIndex].oNormalList;
		}

		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	///////////////////////////////////////////////////////////////////////////////
	// MaterialDescriptor stuff...
	///////////////////////////////////////////////////////////////////////////////
	gGEOData->pMaterials = new MaterialDescriptor [nPublicMaterials + nLocalMaterials];
	gGEOData->pTextureNames = new char * [nPublicMaterials + nLocalMaterials];
	pMaterial = gGEOData->pMaterials;
    pTextureName = gGEOData->pTextureNames;
	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	memset(gGEOData->pMaterials, 0x00,
		sizeof(MaterialDescriptor) * (nPublicMaterials + nLocalMaterials));
	memset(gGEOData->pTextureNames, 0xff,
		sizeof(char *) * (nPublicMaterials + nLocalMaterials));
	
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
		{
			pSurface = pContainer->myObject.pSurfaceList;

			for( j=0 ; j<pContainer->myObject.nSurfaces ; j++ )
			{
				float vlum = pSurface->VLUM;

				if(vlum == 0.0)
					vlum = (float)0.25;
				
				pMaterial->oName = 0; // Set this up later.
                if (globalData->bSurfaceNames)
                {
                    pMaterial->oName = oStringData;
    		        oStringData += strlen(pSurface->name) + 1;
                }

				pMaterial->cAmbient = RGB(
					unsigned char((float)pSurface->COLR.r * vlum),
					unsigned char((float)pSurface->COLR.g * vlum),
					unsigned char((float)pSurface->COLR.b * vlum));
				pMaterial->cDiffuse = RGB(pSurface->COLR.r, pSurface->COLR.g, pSurface->COLR.b);
				pMaterial->cSpecular = RGB(
					unsigned char ((float)pSurface->COLR.r * pSurface->VSPC),
					unsigned char ((float)pSurface->COLR.g * pSurface->VSPC),
					unsigned char ((float)pSurface->COLR.b * pSurface->VSPC));
				pMaterial->kAlpha = pSurface->VTRN;
				
				if(pSurface->CTEX.TIMG)
				{
                    assert(*pTextureName == (char *)0xffffffff);
					pMaterial->oTextureName = oStringData;
					oStringData += strlen(pSurface->CTEX.TIMG) + 1;
                    *pTextureName = new char [strlen(pSurface->CTEX.TIMG) + 1];
                    memcpy(*pTextureName, pSurface->CTEX.TIMG, strlen(pSurface->CTEX.TIMG) + 1);
				}
				else
                {
                    assert(*pTextureName == (char *)0xffffffff);
					pMaterial->oTextureName = 0;
                    *pTextureName = 0;
                }

				pMaterial->flags = 0;
				
				if(bitTest(pSurface->FLAG, SF_DoubleSided))
				{
					bitSet(pMaterial->flags, MDF_2Sided);
				}

				if(bitTest(pSurface->FLAG, SF_Smoothing))
				{
					bitSet(pMaterial->flags, MDF_Smoothing);
				}

				if(bitTest(pSurface->FLAG, SF_Luminous))
				{
					bitSet(pMaterial->flags, MDF_Luminous);
				}

				if(bitTest(pSurface->FLAG, SF_BaseColor))
				{
					bitSet(pMaterial->flags, MDF_BaseColor);
				}

				if(bitTest(pSurface->FLAG, SF_StripeColor))
				{
					bitSet(pMaterial->flags, MDF_StripeColor);
				}

				pMaterial->nFullAmbient = 0; // Set this up later.
				memset(pMaterial->reserved, 0xAB, 5);

				pSurface ++;	//LWO
				pMaterial ++;	//GEO
                pTextureName++;
			}

		}
		
		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Polygon stuff...
	///////////////////////////////////////////////////////////////////////////////
	gGEOData->pPolygons = new Polygon * [gGEOData->pGEOFileHeader->nPolygonObjects];
	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	objectSurfaceOffset = 0;

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
		{
			pObject = &pContainer->myObject;
			
			gGEOData->pPolygons[i] = new Polygon [pObject->nPolygons];

			memset(gGEOData->pPolygons[i], 0x00, sizeof(Polygon));
			
			for( j=0 ; j<pObject->nPolygons ; j++ )
			{
				gGEOData->pPolygons[i][j].iV0 = pObject->pPolyList[j].piVertices[0];
				gGEOData->pPolygons[i][j].iV1 = pObject->pPolyList[j].piVertices[1];
				gGEOData->pPolygons[i][j].iV2 = pObject->pPolyList[j].piVertices[2];

				gGEOData->pPolygons[i][j].iFaceNormal = pObject->pPolyList[j].iFaceNormal;

				gGEOData->pPolygons[i][j].iMaterial = pObject->pPolyList[j].iSurface - 1 + objectSurfaceOffset;
#if NAMES_PER_POLY
                gGEOData->pPolygons[i][j].surfaceName = pObject->pSurfaceList[pObject->pPolyList[j].iSurface - 1].name;
#endif

				gGEOData->pPolygons[i][j].flags = 0; // Set this up later.

				gGEOData->pPolygons[i][j].s0 = pObject->pPolyList[j].t0.s;
				gGEOData->pPolygons[i][j].t0 = pObject->pPolyList[j].t0.t;

				gGEOData->pPolygons[i][j].s1 = pObject->pPolyList[j].t1.s;
				gGEOData->pPolygons[i][j].t1 = pObject->pPolyList[j].t1.t;

				gGEOData->pPolygons[i][j].s2 = pObject->pPolyList[j].t2.s;
				gGEOData->pPolygons[i][j].t2 = pObject->pPolyList[j].t2.t;

				memset(gGEOData->pPolygons[i][j].reserved, 0xAB, 2);
			}

			// This is to handle the fact that all objects think their surface list starts at zero.
			objectSurfaceOffset += (unsigned short)pObject->nSurfaceNames;
		}

		else
		{
			gGEOData->pPolygons[i] = 0;
		}

		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Vertex stuff...
	///////////////////////////////////////////////////////////////////////////////
	gGEOData->pVertices = new Vertex * [gGEOData->pGEOFileHeader->nPolygonObjects];
	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
		{
			pObject = &pContainer->myObject;
			
			gGEOData->pVertices[i] = new Vertex [pObject->nVertices];

			memset(gGEOData->pVertices[i], 0x00, sizeof(Vertex));
			
			for( j=0 ; j<pObject->nVertices ; j++ )
			{
				gGEOData->pVertices[i][j].x = pObject->pVertexList[j].x;
				gGEOData->pVertices[i][j].y = pObject->pVertexList[j].y;
				gGEOData->pVertices[i][j].z = pObject->pVertexList[j].z;
				gGEOData->pVertices[i][j].iVertexNormal = pObject->pVertexList[j].iVertexNormal;
			}
		}

		else
		{
			gGEOData->pVertices[i] = 0;
		}

		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Normal stuff...
	///////////////////////////////////////////////////////////////////////////////
	gGEOData->pNormals = new Normal * [gGEOData->pGEOFileHeader->nPolygonObjects];
	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
		{
			pObject = &pContainer->myObject;
			
			gGEOData->pNormals[i] = new Normal [pObject->nFaceNormals + pObject->nVertexNormals];

			memset(gGEOData->pNormals[i], 0x00, sizeof(Normal));
			
			for( j=0 ; j<pObject->nFaceNormals ; j++ )
			{
				gGEOData->pNormals[i][j].x = pObject->pFaceNormalList[j].x;
				gGEOData->pNormals[i][j].y = pObject->pFaceNormalList[j].y;
				gGEOData->pNormals[i][j].z = pObject->pFaceNormalList[j].z;
				memset(gGEOData->pNormals[i][j].pad, 0xAB, 4);
			}

			for( j=pObject->nFaceNormals, k=0 ; j<pObject->nVertexNormals + pObject->nFaceNormals ; j++, k++)
			{
				gGEOData->pNormals[i][j].x = pObject->pVertexNormalList[k].x;
				gGEOData->pNormals[i][j].y = pObject->pVertexNormalList[k].y;
				gGEOData->pNormals[i][j].z = pObject->pVertexNormalList[k].z;
				memset(gGEOData->pNormals[i][j].pad, 0xAB, 4);
			}
		}

		else
		{
			gGEOData->pNormals[i] = 0;
		}

		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	return(SUCCESS);
}

signed char GEOParse_WriteFile(FILE *fileHandle)
{
	signed char retVal = SUCCESS;
	LWOContainer *pContainer;
	LWOSurface *pSurface;
	LWObject *pObject;
	unsigned long i, j;
#if NAMES_PER_POLY
    Polygon *gon;
#endif
	
	retVal = GEOParse_SetupGEOData();

	if(retVal != SUCCESS)
		return(retVal);

	retVal = GEOParse_BuildLocalMatrix();

	if(retVal != SUCCESS)
		return(retVal);
	
	retVal = GEOParse_OptomizeModeChanges();

	if(retVal != SUCCESS)
		return(retVal);

	retVal = GEOParse_SetupHierarchy();

	if(retVal != SUCCESS)
		return(retVal);

	// Write out the file header.
	fwrite(gGEOData->pGEOFileHeader, sizeof(GeoFileHeader), 1, fileHandle);

	// Write out the poly object headers.
	fwrite(gGEOData->pPolygonObjects, sizeof(PolygonObject), gGEOData->pGEOFileHeader->nPolygonObjects, fileHandle);

	// Write out the material headers.
	fwrite(gGEOData->pMaterials, sizeof(MaterialDescriptor), gGEOData->pGEOFileHeader->nLocalMaterials, fileHandle);
	fwrite(gGEOData->pMaterials + gGEOData->pGEOFileHeader->nLocalMaterials,
		sizeof(MaterialDescriptor), gGEOData->pGEOFileHeader->nPublicMaterials, fileHandle);

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
        {
			fwrite(gGEOData->pPolygons[i], sizeof(Polygon), gGEOData->pPolygonObjects[i].nPolygons, fileHandle);
#if NAMES_PER_POLY
            gon = gGEOData->pPolygons[i];
            for (j = 0; j < gGEOData->pPolygonObjects[i].nPolygons; j++, gon++)
            {
                if (!strcmp(gGEOData->pPolygons[i][j].surfaceName, "engine1port"))
                {
                    Vertex *p0, *p1, *p2;

                    p0 =  &gGEOData->pVertices[i][gon->iV0];
                    p1 =  &gGEOData->pVertices[i][gon->iV1];
                    p2 =  &gGEOData->pVertices[i][gon->iV2];
                    printf("(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)\n",
                           p0->x, p0->y, p0->z, gon->s0, gon->t0,
                           p1->x, p1->y, p1->z, gon->s1, gon->t1,
                           p2->x, p2->y, p2->z, gon->s2, gon->t2);
                }
                if (gon->s0 > 1.0f || gon->s0 < 0.0f) printf("'%s' s0 = %4.2f\n", gGEOData->pPolygons[i][j].surfaceName, gon->s0);
                if (gon->t0 > 1.0f || gon->t0 < 0.0f) printf("'%s' t0 = %4.2f\n", gGEOData->pPolygons[i][j].surfaceName, gon->t0);
                if (gon->s1 > 1.0f || gon->s1 < 0.0f) printf("'%s' s1 = %4.2f\n", gGEOData->pPolygons[i][j].surfaceName, gon->s1);
                if (gon->t1 > 1.0f || gon->t1 < 0.0f) printf("'%s' t1 = %4.2f\n", gGEOData->pPolygons[i][j].surfaceName, gon->t1);
                if (gon->s2 > 1.0f || gon->s2 < 0.0f) printf("'%s' s2 = %4.2f\n", gGEOData->pPolygons[i][j].surfaceName, gon->s2);
                if (gon->t2 > 1.0f || gon->t2 < 0.0f) printf("'%s' t2 = %4.2f\n", gGEOData->pPolygons[i][j].surfaceName, gon->t2);
            }
#endif
        }
	}

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
			fwrite(gGEOData->pVertices[i], sizeof(Vertex), gGEOData->pPolygonObjects[i].nVertices, fileHandle);
	}

	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
			fwrite(gGEOData->pNormals[i], sizeof(Normal),
				gGEOData->pPolygonObjects[i].nFaceNormals + gGEOData->pPolygonObjects[i].nVertexNormals,
				fileHandle);
	}

	// Write out the polygonobject string data.
	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();
	
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		pObject = &pContainer->myObject;

		fwrite(pObject->fileName, 1, strlen(pObject->fileName) + 1, fileHandle);
				
		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	// Write out the material string data.
	pContainer = (LWOContainer *)gLWOData->pLWOList->GetHead();
	
	for( i=0 ; i<gGEOData->pGEOFileHeader->nPolygonObjects ; i++ )
	{
		if(bitTest(gGEOData->pPolygonObjects[i].flags, POF_UniqueObject))
		{
			pSurface = pContainer->myObject.pSurfaceList;

			for( j=0 ; j<pContainer->myObject.nSurfaces ; j++ )
			{
                if (globalData->bSurfaceNames)
                {
        			fwrite(pSurface->name, 1, strlen(pSurface->name) + 1, fileHandle);
                }
				if(pSurface->CTEX.TIMG)
				{
					fwrite(pSurface->CTEX.TIMG, 1, strlen(pSurface->CTEX.TIMG) + 1, fileHandle);
				}

				pSurface ++;
			}
		}

		pContainer = (LWOContainer *)pContainer->GetNext();
	}

	return(retVal);
}