// Copyright (c) 1998-99 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ddraw.h>
#include "render.h"
#include "directdraw.h"
#include "matrix.h"
#include "lwob.h"
#include "euler.h"

// Global variables
float aAxisx[3]={1.0F,0.0F,0.0F};
float aAxisy[3]={0.0F,1.0F,0.0F};
float aAxisz[3]={0.0F,0.0F,1.0F};

// Default object (cube, 1 cubic kilometer)
float REND_DAORIG[3]={0.0F,0.0F,0.0F};
float REND_DASIZE[3]={1000.0F,1000.0F,1000.0F};

float REND_DAVERTS[REND_DNVERTS]=
{
	-500.0F,-500.0F,-500.0F,	// 0
	-500.0F, 500.0F, 500.0F,	// 1
	-500.0F, 500.0F,-500.0F,	// 2
	-500.0F,-500.0F, 500.0F,	// 3
	 500.0F,-500.0F,-500.0F,	// 4
	 500.0F, 500.0F, 500.0F,	// 5
	 500.0F, 500.0F,-500.0F,	// 6
	 500.0F,-500.0F, 500.0F		// 7
};

short REND_DAPOLYS[REND_DNPOLYS]=
{
	2,0,3,3,1,2,	// side
	6,4,0,0,2,6,	// side
	5,6,2,2,1,5,	// side
	7,5,1,1,3,7,	// side
	4,7,3,3,0,4,	// side
	7,4,6,6,5,7		// side
};

unsigned long nSizeOfFloatTimes3;

float fOneOver3;

RENDOBJ *pFirst;
RENDOBJ *pLast;

__declspec(dllexport) int __stdcall rendInit(void)
{
	// Reset first and last object
	pFirst=NULL;
	pLast=NULL;

	// Reset common operatoins
	nSizeOfFloatTimes3=sizeof(float)*3;
	fOneOver3=1.0F/3.0F;

	return(OK);
}

__declspec(dllexport) int __stdcall rendClean(void)
{
	while(1)
	{
		// Check first object
		if(pFirst == NULL) break;

		// Delete first object
		rendDelObj(pFirst);
	}

	return(OK);
}

__declspec(dllexport) int __stdcall rendAngCheck(float *x,float *y,float *z)
{
	// Check angles for exceptions
	if(((*x < REND_AMIN+0.5F) && (*z < REND_AMIN+0.5F)) || ((*x > REND_AMAX-0.5F) && (*z > REND_AMAX-0.5F)))
	{
		// Adjust angles
		*x=0.0F;
		if(*y > 0.0F) *y=180.0F-*y;
		else *y=-(180.0F+*y);
		*z=0.0F;

		return(ERR);
	}

	return(OK);
}

__declspec(dllexport) int __stdcall rendAngConv(float *fAngh,float *fAngv,float x,float y,float z)
{
	float aTransform[16],aWork[3];
	
	// Check angles
	if(x > REND_AMAX) x=REND_AMIN;
	if(x < REND_AMIN) x=REND_AMAX;
	if(y > REND_AMAX) y=REND_AMIN;
	if(y < REND_AMIN) y=REND_AMAX;
	if(z > REND_AMAX) z=REND_AMIN;
	if(z < REND_AMIN) z=REND_AMAX;

	// Set transformation matrix to identity matrix
	matIdentity(aTransform);

	// Set rotation
	matRotate(aTransform,aAxisx,x);
	matRotate(aTransform,aAxisy,y);
	matRotate(aTransform,aAxisz,z);

	// Set data
	memcpy(aWork,aAxisx,nSizeOfFloatTimes3);

	// Transform to world
	matVectProduct(NULL,aWork,aTransform);
	
	// Calc horizontal angle
	*fAngh=(float)((aWork[2] >= 0.0F)?1.0F:-1.0F)*(float)acos(aWork[0]/(float)sqrt(aWork[0]*aWork[0]+aWork[2]*aWork[2]))*180.0F/PI;

	// Calc vertical angle
	*fAngv=(float)asin(aWork[1]/(float)sqrt(aWork[0]*aWork[0]+aWork[1]*aWork[1]+aWork[2]*aWork[2]))*180.0F/PI;

	return(OK);
}

__declspec(dllexport) int __stdcall rendNewObj(RENDOBJ **pObj,int nKey,char *szUID)
{
	short n,m,nTn,nTno,nTnt,nVert;

	float fWork;
	float aVerts[LWOB_SIZE][3],aNorm[3],aMin[3],aMax[3];
	
	FILE *pStream;
	
	RENDOBJ *pCur;

	// Reset object
	*pObj=NULL;

	// Check UID
	if((szUID == NULL) || (strlen(szUID) == 0)) return(OK);
		
	// Allocate memory for new object
	*pObj=(RENDOBJ *)malloc(sizeof(RENDOBJ));
	if(*pObj == NULL) return(ERR);

	// Set UID
	strncpy((*pObj)->szUID,szUID,255);

	// Set key, default mode, default color
	(*pObj)->nKey=nKey;
	(*pObj)->nMode=REND_OSHOW;
	(*pObj)->nCol=REND_WHITE;

	// Reset translation and rotation data
	memset(&((*pObj)->aTrans),0,nSizeOfFloatTimes3);
	memset(&((*pObj)->aRot),0,nSizeOfFloatTimes3);

	// Set scale data to default
	(*pObj)->aScale[0]=REND_OSCALE;
	(*pObj)->aScale[1]=REND_OSCALE;
	(*pObj)->aScale[2]=REND_OSCALE;
	
	// Set transformation matrix to identity matrix
	matIdentity((*pObj)->aTransform);
	
	// Set prev, next and link objects
	(*pObj)->pPrev=pLast;
	(*pObj)->pNext=NULL;
	(*pObj)->pLink=NULL;

	// Set first and last object
	if(pLast == NULL) pFirst=*pObj;
	else pLast->pNext=*pObj;
	pLast=*pObj;

	// Set pointer to first object
	pCur=pFirst;
	
	// Find other objects with same polys/vertices
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check UID
		if((*pObj != pCur) && (strcmp((*pObj)->szUID,pCur->szUID) == 0)) break;

		// Set pointer to next object
		pCur=pCur->pNext;
	}

	// Check temp object
	if(pCur == NULL)
	{
		// Open LWOB file
		pStream=fopen((*pObj)->szUID,"rb");
		if(pStream == NULL)
		{
			// Reset UID
			strcpy((*pObj)->szUID," ");
 
			// Set number of polys and  vertices to default
			(*pObj)->nPolys=REND_DNPOLYS/LWOB_SIZE;
			(*pObj)->nVerts=REND_DNVERTS/3;

			// Set origin, size and selection average to default
			memcpy((*pObj)->aOrig,REND_DAORIG,nSizeOfFloatTimes3);
			memcpy((*pObj)->aSize,REND_DASIZE,nSizeOfFloatTimes3);
			(*pObj)->fSel=REND_DFSEL;

			// Set pointers to polys and vertices to default
			(*pObj)->pPolys=REND_DAPOLYS;
			(*pObj)->pVerts=REND_DAVERTS;
		}
		else
		{
			// Get polys and vertices from LOB file
			if(GetLwobData(pStream,&((*pObj)->nVerts),&((*pObj)->nPolys),
				&((*pObj)->pVerts),&((*pObj)->pPolys)) < 0)
			{
				// Reset UID
				strcpy((*pObj)->szUID," ");
 
				// Set number of polys and  vertices to default
				(*pObj)->nPolys=REND_DNPOLYS/LWOB_SIZE;
				(*pObj)->nVerts=REND_DNVERTS/3;

				// Set origin, size and selection average to default
				memcpy((*pObj)->aOrig,REND_DAORIG,nSizeOfFloatTimes3);
				memcpy((*pObj)->aSize,REND_DASIZE,nSizeOfFloatTimes3);
				(*pObj)->fSel=REND_DFSEL;

				// Set pointers to polys and vertices to default
				(*pObj)->pPolys=REND_DAPOLYS;
				(*pObj)->pVerts=REND_DAVERTS;
			}

			// Close LWOB file
			fclose(pStream);
		}

		// Set minimum
		aMin[0]=0.0F;
		aMin[1]=0.0F;
		aMin[2]=0.0F;

		// Set maximum
		aMax[0]=0.0F;
		aMax[1]=0.0F;
		aMax[2]=0.0F;

		// Loop thru vertices
		for(n=0;n<(*pObj)->nVerts;n++)
		{
			nTn=3*n;
			nTno=nTn+1;
			nTnt=nTn+2;

			// Check X coordinate
			if(aMax[0] < (*pObj)->pVerts[nTn]) aMax[0]=(*pObj)->pVerts[nTn];
			if(aMin[0] > (*pObj)->pVerts[nTn]) aMin[0]=(*pObj)->pVerts[nTn];

			// Check Y coordinate
			if(aMax[1] < (*pObj)->pVerts[nTno]) aMax[1]=(*pObj)->pVerts[nTno];
			if(aMin[1] > (*pObj)->pVerts[nTno]) aMin[1]=(*pObj)->pVerts[nTno];

			// Check Z coordinate
			if(aMax[2] < (*pObj)->pVerts[nTnt]) aMax[2]=(*pObj)->pVerts[nTnt];
			if(aMin[2] > (*pObj)->pVerts[nTnt]) aMin[2]=(*pObj)->pVerts[nTnt];
		}

		// Set origin
		(*pObj)->aOrig[0]=(aMax[0]+aMin[0])/2.0F;
		(*pObj)->aOrig[1]=(aMax[1]+aMin[1])/2.0F;
		(*pObj)->aOrig[2]=(aMax[2]+aMin[2])/2.0F;

		// Set size
		(*pObj)->aSize[0]=(float)rfabs(aMax[0])+(float)rfabs(aMin[0]);
		(*pObj)->aSize[1]=(float)rfabs(aMax[1])+(float)rfabs(aMin[1]);
		(*pObj)->aSize[2]=(float)rfabs(aMax[2])+(float)rfabs(aMin[2]);

		// Set maximum
		aMax[0]=0.0F;
		aMax[1]=0.0F;
		aMax[2]=0.0F;

		// Loop thru vertices
		for(n=0;n<(*pObj)->nVerts;n++)
		{
			// Check X coordinate
			fWork=(float)rfabs((*pObj)->pVerts[n*3]-(*pObj)->aOrig[0]);
			if(aMax[0] < fWork) aMax[0]=fWork;

			// Check Y coordinate
			fWork=(float)rfabs((*pObj)->pVerts[n*3+1]-(*pObj)->aOrig[1]);
			if(aMax[1] < fWork) aMax[1]=fWork;

			// Check Z coordinate
			fWork=(float)rfabs((*pObj)->pVerts[n*3+2]-(*pObj)->aOrig[2]);
			if(aMax[2] < fWork) aMax[2]=fWork;
		}

		// Set selection average
		(*pObj)->fSel=(aMax[0]+aMax[1]+aMax[2])*fOneOver3;

		// Allocate memory for normals
		(*pObj)->pNorms=(float *)calloc((*pObj)->nPolys*3,sizeof(float));
		if((*pObj)->pNorms == NULL) return(ERR);

		// Loop thru polys
		for(n=0;n<(*pObj)->nPolys;n++)
		{
			// Loop thru vertices
			for(m=0;m<LWOB_SIZE;m++)
			{
				// Get vertices
				nVert=(*pObj)->pPolys[n*LWOB_SIZE+m];

				// Handle invalid vertices
				if(nVert > (*pObj)->nVerts-1) memset(aVerts[m],0,nSizeOfFloatTimes3);
				else memcpy(aVerts[m],&((*pObj)->pVerts[nVert*3]),nSizeOfFloatTimes3);
			}

			// Calculate vectors
			aVerts[1][0]=aVerts[0][0]-aVerts[1][0];
			aVerts[1][1]=aVerts[0][1]-aVerts[1][1];
			aVerts[1][2]=aVerts[0][2]-aVerts[1][2];

			aVerts[2][0]=aVerts[0][0]-aVerts[2][0];
			aVerts[2][1]=aVerts[0][1]-aVerts[2][1];
			aVerts[2][2]=aVerts[0][2]-aVerts[2][2];

			// Calculate normal
			matCrossProduct(aNorm,aVerts[1],aVerts[2]);
			memcpy(&((*pObj)->pNorms[n*3]),aNorm,nSizeOfFloatTimes3);
		}
	}	
	else
	{
		// Set number of polys and vertices to same as current object
		(*pObj)->nPolys=pCur->nPolys;
		(*pObj)->nVerts=pCur->nVerts;

		// Set origin and selection average to same as current object
		memcpy((*pObj)->aOrig,pCur->aOrig,nSizeOfFloatTimes3);
		memcpy((*pObj)->aSize,pCur->aSize,nSizeOfFloatTimes3);
		(*pObj)->fSel=pCur->fSel;

		// Set pointers to polys, vertices and normals to same as current object
		(*pObj)->pPolys=pCur->pPolys;
		(*pObj)->pVerts=pCur->pVerts;
		(*pObj)->pNorms=pCur->pNorms;
	}
	
	return(OK);
}

__declspec(dllexport) int __stdcall rendFindObj(RENDOBJ **pObj,int nKey)
{
	// Set pointer to first object
	*pObj=pFirst;
	
	while(1)
	{
		// Check pointer
		if(*pObj == NULL) return(ERR);

		// Check key
		if(nKey == (*pObj)->nKey) break;

		// Set pointer to next object
		*pObj=(*pObj)->pNext;
	}

	return(OK);
}

__declspec(dllexport) int __stdcall rendDelObj(RENDOBJ *pObj)
{
	RENDOBJ *pCur;

	// Check object
	if(pObj == NULL) return(ERR);

	// Set pointer to first object
	pCur=pFirst;
	
	// Find other objects with same polys/vertices
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check UID and pointer
		if((pObj != pCur) && (strcmp(pObj->szUID,pCur->szUID) == 0)) break;

		// Set pointer to next object
		pCur=pCur->pNext;
	}

	// Check UID and pointer
	if((strcmp(pObj->szUID," ") != 0) && (pCur == NULL))
	{
		// Free polys, vertices and normals
		if(pObj->pPolys != NULL) free(pObj->pPolys);
		if(pObj->pVerts != NULL) free(pObj->pVerts);
		if(pObj->pNorms != NULL) free(pObj->pNorms);
	}
	
	// Check prev object
	if(pObj->pPrev == NULL) pFirst=pObj->pNext;
	else pObj->pPrev->pNext=pObj->pNext;

	// Check next object
	if(pObj->pNext == NULL) pLast=pObj->pPrev;
	else pObj->pNext->pPrev=pObj->pPrev;

	// Set pointer to first object
	pCur=pFirst;
	
	// Find objects with links
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check and set link
		if(pCur->pLink == pObj) pCur->pLink=pObj->pLink;

		// Set pointer to next object
		pCur=pCur->pNext;
	}

	// Set first and last object
	if(pFirst == pObj)
	{
		pFirst=NULL;
		pLast=NULL;
	}

	// Free object
	free(pObj);

	return(OK);
}

__declspec(dllexport) int __stdcall rendResetObj(RENDOBJ *pObj)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Set default mode, default color
	pObj->nMode=REND_OSHOW;
	pObj->nCol=REND_WHITE;

	// Reset translation and rotation data
	memset(&(pObj->aTrans),0,nSizeOfFloatTimes3);
	memset(&(pObj->aRot),0,nSizeOfFloatTimes3);

	// Set scale data to default
	pObj->aScale[0]=REND_OSCALE;
	pObj->aScale[1]=REND_OSCALE;
	pObj->aScale[2]=REND_OSCALE;
	
	// Set transformation matrix to identity matrix
	matIdentity(pObj->aTransform);

	return(OK);
}

__declspec(dllexport) int __stdcall rendSetObjLink(RENDOBJ *pObj,long pLink)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Set link
	pObj->pLink=(RENDOBJ *)pLink;

	return(OK);
}

__declspec(dllexport) int __stdcall rendGetObjTrans(RENDOBJ *pObj,float *x,float *y,float *z)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Get translation
	*x=pObj->aTrans[0];
	*y=pObj->aTrans[1];
	*z=pObj->aTrans[2];

	return(OK);
}

__declspec(dllexport) int __stdcall rendTransObj(RENDOBJ *pObj,float x,float y,float z)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Set translation
	pObj->aTrans[0]+=x;
	pObj->aTrans[1]+=y;
	pObj->aTrans[2]+=z;

	// Translate object
	pObj->aTransform[12]+=x;
	pObj->aTransform[13]+=y;
	pObj->aTransform[14]+=z;
	return(OK);
}

__declspec(dllexport) int __stdcall rendGetObjScale(RENDOBJ *pObj,float *x,float *y,float *z)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Get scale
	*x=pObj->aScale[0];
	*y=pObj->aScale[1];
	*z=pObj->aScale[2];

	return(OK);
}

__declspec(dllexport) int __stdcall rendGetObjSize(RENDOBJ *pObj,float *x,float *y,float *z)
{
	// Check object
	if(pObj == NULL)
	{
		*x=0.0F;
		*y=0.0F;
		*z=0.0F;
		return(ERR);
	}

	// Get scale
	*x=pObj->aSize[0];
	*y=pObj->aSize[1];
	*z=pObj->aSize[2];

	return(OK);
}

__declspec(dllexport) int __stdcall rendScaleObj(RENDOBJ *pObj,float x,float y,float z)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Set scale
	pObj->aScale[0]*=x;
	pObj->aScale[1]*=y;
	pObj->aScale[2]*=z;

	// Scale object
	matScale(pObj->aTransform,x,y,z);
	return(OK);
}

__declspec(dllexport) int __stdcall rendGetObjRot(RENDOBJ *pObj,float *x,float *y,float *z)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Get rotation
	*x=pObj->aRot[0];
	*y=pObj->aRot[1];
	*z=pObj->aRot[2];

	return(OK);
}

__declspec(dllexport) int __stdcall rendRotObj(RENDOBJ *pObj,float x,float y,float z)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Set rotation
	pObj->aRot[0]+=x;
	pObj->aRot[1]+=y;
	pObj->aRot[2]+=z;

	// Check angles
	if(pObj->aRot[0] > REND_AMAX) pObj->aRot[0]=REND_AMIN;
	if(pObj->aRot[0] < REND_AMIN) pObj->aRot[0]=REND_AMAX;
	if(pObj->aRot[1] > REND_AMAX) pObj->aRot[1]=REND_AMIN;
	if(pObj->aRot[1] < REND_AMIN) pObj->aRot[1]=REND_AMAX;
	if(pObj->aRot[2] > REND_AMAX) pObj->aRot[2]=REND_AMIN;
	if(pObj->aRot[2] < REND_AMIN) pObj->aRot[2]=REND_AMAX;

	// Set transformation matrix to identity matrix
	matIdentity(pObj->aTransform);

	// Rotate object
	matRotate(pObj->aTransform,aAxisx,pObj->aRot[0]);
	matRotate(pObj->aTransform,aAxisy,pObj->aRot[1]);
	matRotate(pObj->aTransform,aAxisz,pObj->aRot[2]);

	// Scale object
	matScale(pObj->aTransform,pObj->aScale[0],pObj->aScale[1],pObj->aScale[2]);

	// Translate object
	pObj->aTransform[12]=pObj->aTrans[0];
	pObj->aTransform[13]=pObj->aTrans[1];
	pObj->aTransform[14]=pObj->aTrans[2];
	return(OK);
}

__declspec(dllexport) int __stdcall rendGetObjMode(RENDOBJ *pObj,int *nMode)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Get object mode
	*nMode=pObj->nMode;

	return(OK);
}

__declspec(dllexport) int __stdcall rendSetObjMode(RENDOBJ *pObj,int nMode)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Check mode for toggle
	if(nMode == REND_OTOG)
	{
		// Toggle object mode
		if(pObj->nMode == REND_OSHOW) pObj->nMode=REND_OSEL;
		else pObj->nMode=REND_OSHOW;
	}
	// Set object mode
	else pObj->nMode=nMode;

	return(OK);
}

__declspec(dllexport) int __stdcall rendSetObjCol(RENDOBJ *pObj,int nCol)
{
	// Check object
	if(pObj == NULL) return(ERR);

	// Set object color
	pObj->nCol=nCol;
	return(OK);
}

__declspec(dllexport) int __stdcall rendNewCont(RENDCONT **pContext,HWND hWnd,int nMode)
{
	// Allocate memory for new context
	*pContext=(RENDCONT *)malloc(sizeof(RENDCONT));
	if(*pContext == NULL) return(ERR);

	// Initialize directdraw
	(*pContext)->pDdc=ddInit(hWnd);
	if((*pContext)->pDdc == NULL)
	{
		free(*pContext);
		return(ERR);
	}

	// Set mode and detail
	(*pContext)->nMode=nMode;
	(*pContext)->nDetail=REND_CHIGH;

	// Reset origin
	(*pContext)->fOffsetx=0.0F;
	(*pContext)->fOffsety=0.0F;

	// Set default colors
	(*pContext)->nBandCol=REND_WHITE;
	(*pContext)->nCamCol=REND_WHITE;
	(*pContext)->nGridCol=REND_WHITE;
	(*pContext)->nRotCol=REND_WHITE;
	(*pContext)->nSelCol=REND_RED;
	(*pContext)->nCursCol=REND_GREEN;

	// Set rotation object
	(*pContext)->pRotObj=NULL;
	
	// Reset viewport width and height
	(*pContext)->fWidth=0.0F;
	(*pContext)->fHeight=0.0F;

	// Set default scale scale
	(*pContext)->fScale=REND_CSCALE;

	// Set default grid size
	(*pContext)->fGridSize=REND_CGSIZE;

	// Reset band box position
	memset((*pContext)->aBandPos,0,sizeof(float)*4);

	// Reset camera
	memset((*pContext)->aEye,0,nSizeOfFloatTimes3);
	memset((*pContext)->aFocus,0,nSizeOfFloatTimes3);

	// Set transformation matrix to identity matrix
	matIdentity((*pContext)->aTransform);
	
	return(OK);
}

__declspec(dllexport) int __stdcall rendDelCont(RENDCONT *pContext)
{
	// Initialize directdraw
	if(ddClean((DDCONT *)pContext->pDdc) < 0) return(ERR);

	// Check context
	if(pContext == NULL) return(ERR);

	// Free context
	free(pContext);

	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContDetail(RENDCONT *pContext,int nDetail)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Set Level-of-detail
	pContext->nDetail=nDetail;
	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContView(RENDCONT *pContext,float x,float y,float w,float h)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Set context offsets
	pContext->fOffsetx=x;
	pContext->fOffsety=y;

	// Set context viewpoert width and height
	pContext->fWidth=w;
	pContext->fHeight=h;
	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContScale(RENDCONT *pContext,float fScale)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Set context scale
	pContext->fScale=fScale;
	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContBand(RENDCONT *pContext,float *aPos,int nCol)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Set context band box position and color
	memcpy(pContext->aBandPos,aPos,sizeof(float)*4);
	pContext->nBandCol=nCol;
	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContGrid(RENDCONT *pContext,float fSize,int nCol)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Set context grid
	pContext->fGridSize=fSize;
	pContext->nGridCol=nCol;
	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContRot(RENDCONT *pContext,RENDOBJ *pObj,int nCol)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Set context rotation
	pContext->pRotObj=pObj;
	pContext->nRotCol=nCol;
	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContCursor(RENDCONT *pContext,float *aCursor,int nCol)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Set context cursor
	memcpy(pContext->aCursor,aCursor,nSizeOfFloatTimes3);

	// Set context cursor
	pContext->nCursCol=nCol;
	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContSel(RENDCONT *pContext,int nCol)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Set context selection
	pContext->nSelCol=nCol;
	return(OK);
}

__declspec(dllexport) int __stdcall rendCheckContSel(RENDCONT *pContext,int nMode,int *nKey)
{
	float fSel,fWork,fDot;
	float aOrig[3],aWork[3],aLookAt[3];
	float fMinX,fMaxX,fMinY,fMaxY;

	RENDOBJ *pCur;

	// Check context
	if(pContext == NULL) return(ERR);

	// Check camera
	if(pContext->nMode == REND_CCAM)
	{
		// Calculate look-at vector
		aLookAt[0]=pContext->aFocus[0]-pContext->aEye[0];
		aLookAt[1]=pContext->aFocus[1]-pContext->aEye[1];
		aLookAt[2]=pContext->aFocus[2]-pContext->aEye[2];
	}

	// Reset key
	*nKey=0;

	// Set pointer to first object
	pCur=pFirst;
	
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check object mode
		if(pCur->nMode != REND_OHIDE)
		{
			// Get object origin
			memcpy(aOrig,pCur->aOrig,nSizeOfFloatTimes3);

			// Object-to-world transformation
			matVectProduct(NULL,aOrig,pCur->aTransform);

			// Scale selection average
			fSel=pCur->fSel*(pCur->aScale[0]+pCur->aScale[1]+pCur->aScale[2])*fOneOver3;

			// Reset dot product
			fDot=1.0F;

			// Check context mode
			switch(pContext->nMode)
			{
				// Camera view
				case REND_CCAM:
					// Calculate camera to origin vector
					aWork[0]=aOrig[0]-pContext->aEye[0];
					aWork[1]=aOrig[1]-pContext->aEye[1];
					aWork[2]=aOrig[2]-pContext->aEye[2];

					// Calc dot product
					fDot=aLookAt[0]*aWork[0]+aLookAt[1]*aWork[1]+aLookAt[2]*aWork[2];

					// Check dot product
					if(fDot > 0.0F)
					{
						// Compose selection vector
						aWork[0]=(float)sqrt(3.0F)*fSel;
						aWork[1]=(float)sqrt(3.0F)*fSel;
						aWork[2]=(float)sqrt(3.0F)*fSel;

						// World-to-camera transformations
						matVectProduct(NULL,aOrig,pContext->aTransform);
						matVectProduct(NULL,aWork,pContext->aTransform);

						// Scale origin
						aOrig[0]/=aOrig[2];
						aOrig[1]/=aOrig[2];
						aOrig[2]=0.0F;

						// Scale selection vector
						aWork[0]/=aWork[2];
						aWork[1]/=aWork[2];
						aWork[2]=0.0F;

						// Decompose selection vector
						fSel=(float)sqrt(aWork[0]*aWork[0]+aWork[1]*aWork[1]);
					}
					break;

					// X (front) view
				case REND_CX:
					// Non-perspective transformation
					aOrig[0]=-aOrig[2];
					aOrig[1]=-aOrig[1];
					aOrig[2]=0.0F;
					break;

				// Y (top) view
				case REND_CY:
					// Non-perspective transformation
					fWork=-aOrig[0];
					aOrig[0]=-aOrig[2];
					aOrig[1]=fWork;
					aOrig[2]=0.0F;
					break;

				// Z (side) view
				case REND_CZ:
					// Non-perspective transformation
					// aOrig[0]=aOrig[0];
					aOrig[1]=-aOrig[1];
					aOrig[2]=0.0F;
					break;
			}

			// Check dot product
			if(fDot > 0.0F)
			{
				// Scale and offset origin
				aOrig[0]=aOrig[0]*pContext->fScale-pContext->fOffsetx;
				aOrig[1]=aOrig[1]*pContext->fScale-pContext->fOffsety;

				// Scale selection average
				fSel*=pContext->fScale;

				// Tune selection average
				fSel*=REND_STUNE;

				// Get min and max X
				if(pContext->aBandPos[0] < pContext->aBandPos[2])
				{
					fMinX=pContext->aBandPos[0];
					fMaxX=pContext->aBandPos[2];
				}
				else
				{
					fMinX=pContext->aBandPos[2];
					fMaxX=pContext->aBandPos[0];
				}

				// Get min and max Y
				if(pContext->aBandPos[1] < pContext->aBandPos[3])
				{
					fMinY=pContext->aBandPos[1];
					fMaxY=pContext->aBandPos[3];
				}
				else
				{
					fMinY=pContext->aBandPos[3];
					fMaxY=pContext->aBandPos[1];
				}
				
				// Check rectangular region (band-box)
				if((aOrig[0]+fSel >= fMinX) && (aOrig[0]-fSel <= fMaxX) &&
					(aOrig[1]+fSel >= fMinY) && (aOrig[1]-fSel <= fMaxY))
				{
					// Check key
					if(*nKey == 0) *nKey=pCur->nKey;

					// Check mode for toggle and set object mode
					if(nMode == REND_OSEL) pCur->nMode=REND_OSEL;
					if(nMode == REND_OTOG)
					{
						// Toggle object mode
						if(pCur->nMode == REND_OSHOW) pCur->nMode=REND_OSEL;
						else pCur->nMode=REND_OSHOW;
					}
				}
				else
				{
					// Check mode for toggle ans set object mode
					if(nMode != REND_OTOG) pCur->nMode=REND_OSHOW;
				}
			}
		}
		
		// Set pointer to next object
		pCur=pCur->pNext;
	}

	// Reset band box position
	memset(pContext->aBandPos,0,sizeof(float)*4);

	return(OK);
}

__declspec(dllexport) int __stdcall rendCheckContCamera(RENDCONT *pContext,float x,float y,int *nSel)
{
	float fWork;
	float aFocus[3],aEye[3];

	// Set selection
	*nSel=REND_SNONE;
	
	// Check context
	if(pContext == NULL) return(ERR);

	// Check camera
	if(pContext->nCamCol == REND_NONE) return(OK);

	// Copy eye and focus
	memcpy(aEye,pContext->aEye,nSizeOfFloatTimes3);
	memcpy(aFocus,pContext->aFocus,nSizeOfFloatTimes3);

	// Check context mode
	switch(pContext->nMode)
	{
		// X (front) view
		case REND_CX:
			// Set eye and focus
			aEye[0]=-aEye[2]*pContext->fScale-pContext->fOffsetx;
			aEye[1]=-aEye[1]*pContext->fScale-pContext->fOffsety;
			aEye[2]=0.0F;

			aFocus[0]=-aFocus[2]*pContext->fScale-pContext->fOffsetx;
			aFocus[1]=-aFocus[1]*pContext->fScale-pContext->fOffsety;
			aFocus[2]=0.0F;
			break;

		// Y (top) view
		case REND_CY:
			// Set eye and focus
			fWork=-aEye[0]*pContext->fScale-pContext->fOffsety;
			aEye[0]=-aEye[2]*pContext->fScale-pContext->fOffsetx;
			aEye[1]=fWork;
			aEye[2]=0.0F;

			fWork=-aFocus[0]*pContext->fScale-pContext->fOffsety;
			aFocus[0]=-aFocus[2]*pContext->fScale-pContext->fOffsetx;
			aFocus[1]=fWork;
			aFocus[2]=0.0F;
			break;

		// Z (side) view
		case REND_CZ:
			// Set eye and focus
			aEye[0]=aEye[0]*pContext->fScale-pContext->fOffsetx;
			aEye[1]=-aEye[1]*pContext->fScale-pContext->fOffsety;
			aEye[2]=0.0F;

			aFocus[0]=aFocus[0]*pContext->fScale-pContext->fOffsetx;
			aFocus[1]=-aFocus[1]*pContext->fScale-pContext->fOffsety;
			aFocus[2]=0.0F;
			break;
	}

	// Check eye and set selection
	if((aEye[0] >= x-REND_SCAM) && (aEye[0] <= x+REND_SCAM) &&
		(aEye[1] >= y-REND_SCAM) && (aEye[1] <= y+REND_SCAM))
			*nSel=REND_SEYE;
		
	// Check focus and set selection
	if((aFocus[0] >= x-REND_SCAM) && (aFocus[0] <= x+REND_SCAM) &&
		(aFocus[1] >= y-REND_SCAM) && (aFocus[1] <= y+REND_SCAM))
			*nSel=REND_SFOCUS;
		
	return(OK);
}

__declspec(dllexport) int __stdcall rendGetContCamera(RENDCONT *pContext,float *aEye,float *aFocus)
{
	// Check context
	if(pContext == NULL) return(ERR);

	// Get context camera eye and focus
	memcpy(aEye,pContext->aEye,nSizeOfFloatTimes3);
	memcpy(aFocus,pContext->aFocus,nSizeOfFloatTimes3);
	return(OK);
}

__declspec(dllexport) int __stdcall rendSetContCamera(RENDCONT *pContext,float *aEye,float *aFocus,int nCol)
{
	float aLookAt[3],fDot;

	// Check context
	if(pContext == NULL) return(ERR);

	// Set context camera
	pContext->nCamCol=nCol;

	// Calculate camera look-at vector
	aLookAt[0]=aEye[0]-aFocus[0];
	aLookAt[1]=aEye[1]-aFocus[1];
	aLookAt[2]=aEye[2]-aFocus[2];

	// Check look-at vector
	matNormalize(aLookAt);
	fDot=aLookAt[0]*aAxisy[0]+aLookAt[1]*aAxisy[1]+aLookAt[2]*aAxisy[2];
	if((fDot > -0.99F) && (fDot < 0.99F))
	{
		// Set context camera eye and focus
		memcpy(pContext->aEye,aEye,nSizeOfFloatTimes3);
		memcpy(pContext->aFocus,aFocus,nSizeOfFloatTimes3);
	}

	// Set context world-to-camera transformation matrix
	matIdentity(pContext->aTransform);
	matLookAt(pContext->aTransform,pContext->aEye,pContext->aFocus,aAxisy);
	return(OK);
}

__declspec(dllexport) int __stdcall rendTransContCamera(RENDCONT *pContext,float x,float y)
{
	float aEye[3],aFocus[3],aLookAt[3],aRight[3],aUp[3];

	// Check context
	if(pContext == NULL) return(ERR);

	// Check x and y
	if((x == 0.0F) && (y == 0.0F)) return(OK);

	// Set context camera eye and focus
	memcpy(aEye,pContext->aEye,nSizeOfFloatTimes3);
	memcpy(aFocus,pContext->aFocus,nSizeOfFloatTimes3);

	// Calculate camera look-at vector
	aLookAt[0]=aEye[0]-aFocus[0];
	aLookAt[1]=aEye[1]-aFocus[1];
	aLookAt[2]=aEye[2]-aFocus[2];

	// Calculate camera eye right and camera eye up vectors
	matCrossProduct(aRight,aAxisy,aLookAt);
	if(y != 0.0F) matCrossProduct(aUp,aLookAt,aRight);

	// Translate camera eye
	if(x != 0.0F)
	{
		// Normalize camera right vector
		matNormalize(aRight);
		
		// Scale camera right vector
		aRight[0]*=x;
		aRight[1]*=x;
		aRight[2]*=x;

		// Translate camera eye horizontally
		aEye[0]=aEye[0]+aRight[0];
		aEye[1]=aEye[1]+aRight[1];
		aEye[2]=aEye[2]+aRight[2];

		// Translate camera focus horizontally
		aFocus[0]+=aRight[0];
		aFocus[1]+=aRight[1];
		aFocus[2]+=aRight[2];
	}

	// Translate camera focus
	if(y != 0.0F)
	{
		// Normalize camera up vector
		matNormalize(aUp);

		// Scale camera up vector
		aUp[0]*=y;
		aUp[1]*=y;
		aUp[2]*=y;

		// Translate camera eye vertically
		aEye[0]=aEye[0]+aUp[0];
		aEye[1]=aEye[1]+aUp[1];
		aEye[2]=aEye[2]+aUp[2];

		// Translate camera focus vertically
		aFocus[0]+=aUp[0];
		aFocus[1]+=aUp[1];
		aFocus[2]+=aUp[2];
	}

	// Set context camera
	rendSetContCamera(pContext,aEye,aFocus,pContext->nCamCol);
	return(OK);
}

__declspec(dllexport) int __stdcall rendScaleContCamera(RENDCONT *pContext,float nScale)
{
	float aEye[3],aFocus[3],aScale[16];

	// Check context
	if(pContext == NULL) return(ERR);

	// Check scale
	if(nScale == 1.0F) return(OK);

	// Set context camera eye and focus
	memcpy(aEye,pContext->aEye,nSizeOfFloatTimes3);
	memcpy(aFocus,pContext->aFocus,nSizeOfFloatTimes3);

	// Translate camera eye
	aEye[0]-=aFocus[0];
	aEye[1]-=aFocus[1];
	aEye[2]-=aFocus[2];

	// Set scale matrix
	matIdentity(aScale);
	matScale(aScale,nScale,nScale,nScale);

	// Scale look-at vector
	matVectProduct(NULL,aEye,aScale);

	// Translate camera eye
	aEye[0]+=aFocus[0];
	aEye[1]+=aFocus[1];
	aEye[2]+=aFocus[2];

	// Set context camera
	rendSetContCamera(pContext,aEye,aFocus,pContext->nCamCol);
	return(OK);
}

__declspec(dllexport) int __stdcall rendRotContCamera(RENDCONT *pContext,float fAngh,float fAngv)
{
	float aEye[3],aFocus[3],aLookAt[3],aRight[3];
	float aRot[16];

	// Check context
	if(pContext == NULL) return(ERR);

	// Check angles
	if((fAngh == 0.0F) && (fAngv == 0.0F)) return(OK);

	// Set context camera eye and focus
	memcpy(aEye,pContext->aEye,nSizeOfFloatTimes3);
	memcpy(aFocus,pContext->aFocus,nSizeOfFloatTimes3);

	// Calculate camera look-at vector
	aLookAt[0]=aEye[0]-aFocus[0];
	aLookAt[1]=aEye[1]-aFocus[1];
	aLookAt[2]=aEye[2]-aFocus[2];

	// Translate camera eye
	aEye[0]-=aFocus[0];
	aEye[1]-=aFocus[1];
	aEye[2]-=aFocus[2];

	// Calculate camera focus right and camera focus up vectors
	matCrossProduct(aRight,aAxisy,aLookAt);

	// Set rotation matrix
	matNormalize(aRight);
	matIdentity(aRot);
	if(fAngh != 0.0F) matRotate(aRot,aAxisy,fAngh);

	// Rotate look-at vector
	if(fAngv != 0.0F) matRotate(aRot,aRight,fAngv);
	matVectProduct(NULL,aEye,aRot);

	// Translate camera eye
	aEye[0]+=aFocus[0];
	aEye[1]+=aFocus[1];
	aEye[2]+=aFocus[2];

	// Set context camera
	rendSetContCamera(pContext,aEye,aFocus,pContext->nCamCol);
	return(OK);
}

__declspec(dllexport) int __stdcall rendUpdateCont(RENDCONT *pContext)
{
	// Update
	return(ddUpdate((DDCONT *)pContext->pDdc,pContext));
}

__declspec(dllexport) int __stdcall rendResizeCont(RENDCONT *pContext)
{
	// Resize and update
	if(ddResize((DDCONT *)pContext->pDdc) < 0) return(ERR);
	return(ddUpdate((DDCONT *)pContext->pDdc,pContext));
}

__declspec(dllexport) int __stdcall rendPaintCont(RENDCONT *pContext)
{
	// Paint
	return(ddPaint((DDCONT *)pContext->pDdc));
}

__declspec(dllexport) int __stdcall rendGetSel(char *szPrefix,short *nCount,char **szKeys)
{
	short nSize;
	
	char szKey[256];

	RENDOBJ *pCur;

	// Reset count
	*nCount=0;
	
	// Reset size
	nSize=0;

	// Set pointer to first object
	pCur=pFirst;
	
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check object mode
		if(pCur->nMode == REND_OSEL)
		{
			// Get size
			sprintf(szKey,"%s%ld",szPrefix,pCur->nKey);
			nSize=nSize+strlen(szKey)+1;

			// Increment count
			(*nCount)++;
		}
		
		// Set pointer to next object
		pCur=pCur->pNext;
	}

	// Allocate memory for keys
	*szKeys=(char *)calloc(nSize,sizeof(char));
	if(*szKeys == NULL) return(ERR);
	
	// Reset count
	*nCount=0;
	
	// Reset size
	nSize=0;

	// Set pointer to first object
	pCur=pFirst;
	
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check object mode
		if(pCur->nMode == REND_OSEL)
		{
			// Copy key
			sprintf(szKey,"%s%ld",szPrefix,pCur->nKey);
			if(*nCount > 0) strcat(*szKeys," ");
			strcat(*szKeys,szKey);

			// Increment count
			(*nCount)++;
		}
		
		// Set pointer to next object
		pCur=pCur->pNext;
	}

	return(OK);
}

__declspec(dllexport) int __stdcall rendSetSel(char *szPrefix,char *szKeys)
{
	char *pKey,szKey[256],*szList;

	RENDOBJ *pCur;

	// Set pointer to first object
	pCur=pFirst;

	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check ans set object mode to show
		if(pCur->nMode != REND_OHIDE) pCur->nMode=REND_OSHOW;

		// Set pointer to next object
		pCur=pCur->pNext;
	}

	// Allocate memory for list
	szList=(char *)calloc(strlen(szKeys)+1,sizeof(char));
	if(szList == NULL) return(ERR);

	// Copy keys to list
	strcpy(szList,szKeys);
	
	// Find and check first key
	pKey=strtok(szList," ");
	if(pKey == NULL) pKey=szKeys;

	while(1)
	{
		// Check key
		if(pKey == NULL) break;

		// Set pointer to first object
		pCur=pFirst;
	
		while(1)
		{
			// Check pointer
			if(pCur == NULL) break;

			// Check key
			sprintf(szKey,"%s%ld",szPrefix,pCur->nKey);
			if(strcmp(pKey,szKey) == 0)
			{
				// Check and set object mode
				if(pCur->nMode != REND_OHIDE) pCur->nMode=REND_OSEL;
				break;
			}

			// Set pointer to next object
			pCur=pCur->pNext;
		}
		
		// Find next key
		pKey=strtok(NULL," ");
	}

	// Free list
	free(szList);

	return(OK);
}

__declspec(dllexport) int __stdcall rendTransSel(float x,float y,float z)
{
	RENDOBJ *pCur;

	// Set pointer to first object
	pCur=pFirst;
	
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check object mode
		if(pCur->nMode == REND_OSEL)
		{
			// Set translation
			pCur->aTrans[0]+=x;
			pCur->aTrans[1]+=y;
			pCur->aTrans[2]+=z;

			// Translate object
			pCur->aTransform[12]+=x;
			pCur->aTransform[13]+=y;
			pCur->aTransform[14]+=z;
		}
		
		// Set pointer to next object
		pCur=pCur->pNext;
	}

	return(OK);
}

__declspec(dllexport) int __stdcall rendScaleSel(float x,float y,float z)
{
	RENDOBJ *pCur;

	// Set pointer to first object
	pCur=pFirst;
	
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check object mode
		if(pCur->nMode == REND_OSEL)
		{
			// Set scale
			pCur->aScale[0]*=x;
			pCur->aScale[1]*=y;
			pCur->aScale[2]*=z;

			// Scale object
			matScale(pCur->aTransform,x,y,z);
		}
		
		// Set pointer to next object
		pCur=pCur->pNext;
	}

	return(OK);
}

__declspec(dllexport) int __stdcall rendRotSel(RENDCONT *pContext,float x,float y,float z)
{
	RENDOBJ *pCur;

	Euler *eObj,*eWorld,eRes;

	float aObj[16],aRes[16],aWorld[16];

	// Set pointer to first object
	pCur=pFirst;

	// Contruct world matrix
	eWorld=new Euler(x*PI/180.0F,y*PI/180.0F,z*PI/180.0F,EulOrdXYZr);
	eWorld->ToMatrix(aWorld);

	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check object mode
		if(pCur->nMode == REND_OSEL)
		{
			// Check object
			if(pCur == NULL)
			{
				// Destroy world matrix
				delete eWorld;
	
				return(ERR);
			}

			// Contruct object matrix
			eObj=new Euler(pCur->aRot[0]*PI/180.0F,pCur->aRot[1]*PI/180.0F,pCur->aRot[2]*PI/180.0F,EulOrdXYZr);
			eObj->ToMatrix(aObj);

			// Multiply matrices
			matProduct(aRes,aWorld,aObj);

			// Get angles
			eRes.FromMatrix(aRes,EulOrdXYZr);

			// Set rotation
			pCur->aRot[0]=eRes.x*180.0F/PI;
			pCur->aRot[1]=eRes.y*180.0F/PI;
			pCur->aRot[2]=eRes.z*180.0F/PI;

			// Destroy object matrix
			delete eObj;	

			// Check angles
			if(pCur->aRot[0] > REND_AMAX) pCur->aRot[0]=REND_AMIN;
			if(pCur->aRot[0] < REND_AMIN) pCur->aRot[0]=REND_AMAX;
			if(pCur->aRot[1] > REND_AMAX) pCur->aRot[1]=REND_AMIN;
			if(pCur->aRot[1] < REND_AMIN) pCur->aRot[1]=REND_AMAX;
			if(pCur->aRot[2] > REND_AMAX) pCur->aRot[2]=REND_AMIN;
			if(pCur->aRot[2] < REND_AMIN) pCur->aRot[2]=REND_AMAX;

			// Set transformation matrix to identity matrix
			matIdentity(pCur->aTransform);

			// Rotate object
			matRotate(pCur->aTransform,aAxisx,pCur->aRot[0]);
			matRotate(pCur->aTransform,aAxisy,pCur->aRot[1]);
			matRotate(pCur->aTransform,aAxisz,pCur->aRot[2]);

			// Scale object
			matScale(pCur->aTransform,pCur->aScale[0],pCur->aScale[1],pCur->aScale[2]);

			// Check context
			if(pContext != NULL)
			{
				// Get vector
				pCur->aTrans[0]-=pContext->aCursor[0];
				pCur->aTrans[1]-=pContext->aCursor[1];
				pCur->aTrans[2]-=pContext->aCursor[2];
					
				// Rotate vector
				matVectProduct(NULL,pCur->aTrans,aWorld);

				// Set vector
				pCur->aTrans[0]+=pContext->aCursor[0];
				pCur->aTrans[1]+=pContext->aCursor[1];
				pCur->aTrans[2]+=pContext->aCursor[2];
			}

			// Translate object
			pCur->aTransform[12]=pCur->aTrans[0];
			pCur->aTransform[13]=pCur->aTrans[1];
			pCur->aTransform[14]=pCur->aTrans[2];
		}
		
		// Set pointer to next object
		pCur=pCur->pNext;
	}

	// Destroy world matrix
	delete eWorld;
	
	return(OK);
}

// Internal functions
int rendRendCont(RENDCONT *pContext,HDC hdc)
{
	short n,m,nVert,nFlag;
	
	int i,j,nPosx,nPosy;

	float fDot,fSize,fInvSize,fWork,fArea;
	float aVerts[LWOB_SIZE+1][3],aCubeVerts[REND_DNVERTS];
	float aWork[3],aLookAt[3],aEye[3],aFocus[3],aCursor[3],aMin[3],aMax[3];

	HPEN hpen,hdefault;
	LOGBRUSH lb;

	RENDOBJ *pCur,rCubeObj;

	// Check context
	if(pContext == NULL) return(ERR);

	// Check camera
	if(pContext->nMode == REND_CCAM)
	{
		// Calculate look-at vector
		aLookAt[0]=pContext->aFocus[0]-pContext->aEye[0];
		aLookAt[1]=pContext->aFocus[1]-pContext->aEye[1];
		aLookAt[2]=pContext->aFocus[2]-pContext->aEye[2];
	}

	// Set brush
	lb.lbStyle=BS_SOLID;
	lb.lbHatch=0; 

	// Check camera
	if((pContext->nMode != REND_CCAM) && (pContext->nCamCol != REND_NONE))
	{
		// Copy eye and focus
		memcpy(aEye,pContext->aEye,nSizeOfFloatTimes3);
		memcpy(aFocus,pContext->aFocus,nSizeOfFloatTimes3);

		// Check context mode
		switch(pContext->nMode)
		{
			// X (front) view
			case REND_CX:
				// Set eye and focus
				aEye[0]=-aEye[2]*pContext->fScale-pContext->fOffsetx;
				aEye[1]=-aEye[1]*pContext->fScale-pContext->fOffsety;
				aEye[2]=0.0F;

				aFocus[0]=-aFocus[2]*pContext->fScale-pContext->fOffsetx;
				aFocus[1]=-aFocus[1]*pContext->fScale-pContext->fOffsety;
				aFocus[2]=0.0F;
				break;

			// Y (top) view
			case REND_CY:
				// Set eye and focus
				fWork=-aEye[0]*pContext->fScale-pContext->fOffsety;
				aEye[0]=-aEye[2]*pContext->fScale-pContext->fOffsetx;
				aEye[1]=fWork;
				aEye[2]=0.0F;

				fWork=-aFocus[0]*pContext->fScale-pContext->fOffsety;
				aFocus[0]=-aFocus[2]*pContext->fScale-pContext->fOffsetx;
				aFocus[1]=fWork;
				aFocus[2]=0.0F;
				break;

			// Z (side) view
			case REND_CZ:
				// Set eye and focus
				aEye[0]=aEye[0]*pContext->fScale-pContext->fOffsetx;
				aEye[1]=-aEye[1]*pContext->fScale-pContext->fOffsety;
				aEye[2]=0.0F;

				aFocus[0]=aFocus[0]*pContext->fScale-pContext->fOffsetx;
				aFocus[1]=-aFocus[1]*pContext->fScale-pContext->fOffsety;
				aFocus[2]=0.0F;
				break;
		}

		// Set band box color
		lb.lbColor=pContext->nCamCol;

		// Create pen
		hpen=ExtCreatePen(PS_COSMETIC|PS_SOLID,1,&lb,0,NULL); 

		// Set color and width
		hdefault=(HPEN)SelectObject(hdc,hpen);

		// Draw eye
		MoveToEx(hdc,(int)aEye[0]+REND_SCAM,(int)aEye[1]+REND_SCAM,NULL);
		LineTo(hdc,(int)aEye[0]+REND_SCAM,(int)aEye[1]-REND_SCAM);
		LineTo(hdc,(int)aEye[0]-REND_SCAM,(int)aEye[1]-REND_SCAM);
		LineTo(hdc,(int)aEye[0]-REND_SCAM,(int)aEye[1]+REND_SCAM);
		LineTo(hdc,(int)aEye[0]+REND_SCAM,(int)aEye[1]+REND_SCAM);

		// Reset to default
		SelectObject(hdc,hdefault);

		// Delete pen
		DeleteObject(hpen);

		// Create pen
		hpen=ExtCreatePen(PS_COSMETIC|PS_DASH,1,&lb,0,NULL); 

		// Set color and width
		hdefault=(HPEN)SelectObject(hdc,hpen);

		// Draw line
		MoveToEx(hdc,(int)aEye[0],(int)aEye[1],NULL);
		LineTo(hdc,(int)aFocus[0],(int)aFocus[1]);

		// Reset to default
		SelectObject(hdc,hdefault);

		// Delete pen
		DeleteObject(hpen);

		// Create pen
		hpen=ExtCreatePen(PS_COSMETIC|PS_SOLID,1,&lb,0,NULL); 

		// Set color and width
		hdefault=(HPEN)SelectObject(hdc,hpen);

		// Draw focus
		MoveToEx(hdc,(int)aFocus[0]+REND_SCAM,(int)aFocus[1],NULL);
		LineTo(hdc,(int)aFocus[0],(int)aFocus[1]+REND_SCAM);
		LineTo(hdc,(int)aFocus[0]-REND_SCAM,(int)aFocus[1]);
		LineTo(hdc,(int)aFocus[0],(int)aFocus[1]-REND_SCAM);
		LineTo(hdc,(int)aFocus[0]+REND_SCAM,(int)aFocus[1]);

		// Reset to default
		SelectObject(hdc,hdefault);

		// Delete pen
		DeleteObject(hpen);
	}

	// Check rotation flag
	if((pContext->nMode != REND_CCAM) && (pContext->pRotObj != NULL) &&
		(pContext->nRotCol != REND_NONE))
	{
		// Set rotation color
		lb.lbColor=pContext->nRotCol;

		// Create pen
		hpen=ExtCreatePen(PS_COSMETIC|PS_SOLID,1,&lb,0,NULL); 

		// Set color and width
		hdefault=(HPEN)SelectObject(hdc,hpen);

		// Set vertices
		memset(aVerts[0],0,nSizeOfFloatTimes3);
		aVerts[0][0]=pContext->pRotObj->aSize[0]*(float)REND_SROT;
		memset(aVerts[1],0,nSizeOfFloatTimes3);
		aVerts[1][1]=pContext->pRotObj->aSize[1]*(float)REND_SROT;
		memset(aVerts[2],0,nSizeOfFloatTimes3);
		aVerts[2][2]=pContext->pRotObj->aSize[2]*(float)REND_SROT;
		memset(aVerts[3],0,nSizeOfFloatTimes3);

		// Check context mode
		switch(pContext->nMode)
		{
			// X (front) view
			case REND_CX:
				// Loop thru vertices
				for(n=0;n<=LWOB_SIZE;n++)
				{
					// Object-to-world transformation
					matVectProduct(NULL,aVerts[n],pContext->pRotObj->aTransform);
	
					// Set cursor
					aVerts[n][0]=-aVerts[n][2]*pContext->fScale-pContext->fOffsetx;
					aVerts[n][1]=-aVerts[n][1]*pContext->fScale-pContext->fOffsety;
					aVerts[n][2]=0.0F;
				}
				break;

			// Y (top) view
			case REND_CY:
				// Loop thru vertices
				for(n=0;n<=LWOB_SIZE;n++)
				{
					// Object-to-world transformation
					matVectProduct(NULL,aVerts[n],pContext->pRotObj->aTransform);
	
					// Set cursor
					fWork=-aVerts[n][0]*pContext->fScale-pContext->fOffsety;
					aVerts[n][0]=-aVerts[n][2]*pContext->fScale-pContext->fOffsetx;
					aVerts[n][1]=fWork;
					aVerts[n][2]=0.0F;
				}
				break;

			// Z (side) view
			case REND_CZ:
				// Loop thru vertices
				for(n=0;n<=LWOB_SIZE;n++)
				{
					// Object-to-world transformation
					matVectProduct(NULL,aVerts[n],pContext->pRotObj->aTransform);
	
					// Set cursor
					aVerts[n][0]=aVerts[n][0]*pContext->fScale-pContext->fOffsetx;
					aVerts[n][1]=-aVerts[n][1]*pContext->fScale-pContext->fOffsety;
					aVerts[n][2]=0.0F;
				}
				break;
		}

		// Draw rotation
		MoveToEx(hdc,(int)aVerts[3][0],(int)aVerts[3][1],NULL);
		LineTo(hdc,(int)aVerts[0][0],(int)aVerts[0][1]);
		MoveToEx(hdc,(int)aVerts[3][0],(int)aVerts[3][1],NULL);
		LineTo(hdc,(int)aVerts[1][0],(int)aVerts[1][1]);
		MoveToEx(hdc,(int)aVerts[3][0],(int)aVerts[3][1],NULL);
		LineTo(hdc,(int)aVerts[2][0],(int)aVerts[2][1]);

		// Reset to default
		SelectObject(hdc,hdefault);

		// Delete pen
		DeleteObject(hpen);
	}

	// Check grid size
	if((pContext->nMode != REND_CCAM) && (pContext->fGridSize > 0.0) &&
		(pContext->nGridCol != REND_NONE))
	{
		// Set band box color
		lb.lbColor=pContext->nGridCol;

		// Create pen
		hpen=ExtCreatePen(PS_COSMETIC|PS_SOLID,1,&lb,0,NULL); 

		// Set color and width
		hdefault=(HPEN)SelectObject(hdc,hpen);

		// Calc size
		fSize=pContext->fGridSize*pContext->fScale;

		// Adjust size
		while(fSize < REND_CGLIM) fSize*=REND_CGLIM;
		fInvSize=1.0F/fSize;

		// Loop
		for(i=(int)(pContext->fOffsetx*fInvSize);
			i<=(int)((pContext->fWidth+pContext->fOffsetx)*fInvSize);i++)
		{
			nPosx=(int)(fSize*(float)i-pContext->fOffsetx);

			for(j=(int)(pContext->fOffsety*fInvSize);
				j<=(int)((pContext->fHeight+pContext->fOffsety)*fInvSize);j++)
			{
				// Calc y position
				nPosy=(int)(fSize*(float)j-pContext->fOffsety);

				// Draw fine grid
				MoveToEx(hdc,nPosx,nPosy,NULL);
				LineTo(hdc,nPosx+1,nPosy);
				
				// Check indexes
				if((i%10 == 0) && (j%10 == 0))
				{
					// Draw coarse grid
					LineTo(hdc,nPosx-2,nPosy);
					MoveToEx(hdc,nPosx,nPosy,NULL);
					LineTo(hdc,nPosx,nPosy-1);
					LineTo(hdc,nPosx,nPosy+2);
				}
			}
		}

		// Reset to default
		SelectObject(hdc,hdefault);

		// Delete pen
		DeleteObject(hpen);
	}

	// Check band box position
	if((pContext->nMode != REND_CCAM) &&
		(pContext->aBandPos[0] != pContext->aBandPos[2]) &&
		(pContext->aBandPos[1] != pContext->aBandPos[3]))
	{
		// Set band box color
		lb.lbColor=pContext->nBandCol;

		// Create pen
		hpen=ExtCreatePen(PS_COSMETIC|PS_DASH,1,&lb,0,NULL); 

		// Set color and width
		hdefault=(HPEN)SelectObject(hdc,hpen);

		// Draw band box
		MoveToEx(hdc,(int)pContext->aBandPos[0],(int)pContext->aBandPos[1],NULL);
		LineTo(hdc,(int)pContext->aBandPos[2],(int)pContext->aBandPos[1]);
		LineTo(hdc,(int)pContext->aBandPos[2],(int)pContext->aBandPos[3]);
		LineTo(hdc,(int)pContext->aBandPos[0],(int)pContext->aBandPos[3]);
		LineTo(hdc,(int)pContext->aBandPos[0],(int)pContext->aBandPos[1]);

		// Reset to default
		SelectObject(hdc,hdefault);

		// Delete pen
		DeleteObject(hpen);
	}

	// Set pointer to first object
	pCur=pFirst;
	
	while(1)
	{
		// Check pointer
		if(pCur == NULL) break;

		// Check object mode
		if(pCur->nMode != REND_OHIDE)
		{
			// Check link
			if(pCur->pLink != NULL)
			{
				// Set color
				lb.lbColor=pCur->nCol;

				// Create pen
				hpen=ExtCreatePen(PS_COSMETIC|PS_SOLID,1,&lb,0,NULL); 

				// Set color and width
				hdefault=(HPEN)SelectObject(hdc,hpen);

				// Get origins
				memset(aVerts[0],0,nSizeOfFloatTimes3);
				memset(aVerts[1],0,nSizeOfFloatTimes3);
				memset(aVerts[2],0,nSizeOfFloatTimes3);
				memset(aVerts[3],0,nSizeOfFloatTimes3);
				
				// Object-to-world transformation
				matVectProduct(NULL,aVerts[0],pCur->pLink->aTransform);
				matVectProduct(NULL,aVerts[1],pCur->aTransform);

				// Reset dot product
				fDot=1.0F;

				// Check context mode
				switch(pContext->nMode)
				{
					// Camera view
					case REND_CCAM:
						// Loop thru origins
						for(m=0;m<2;m++)
						{
							// Calculate camera to vertex vector
							aWork[0]=aVerts[m][0]-pContext->aEye[0];
							aWork[1]=aVerts[m][1]-pContext->aEye[1];
							aWork[2]=aVerts[m][2]-pContext->aEye[2];

							// Calc dot product
							if(fDot > 0.0F) fDot=aLookAt[0]*aWork[0]+aLookAt[1]*aWork[1]+aLookAt[2]*aWork[2];

							// Check dot product
							if(fDot > 0.0F)
							{
								// World-to-camera transformations
								matVectProduct(NULL,aVerts[m],pContext->aTransform);

								// Scale vertex
								aVerts[m][0]/=aVerts[m][2];
								aVerts[m][1]/=aVerts[m][2];
								aVerts[m][2]=0.0F;
							}
						}
						break;

					// X (front) view
					case REND_CX:
						// Loop thru origins
						for(m=0;m<2;m++)
						{
							// Non-perspective transformation
							aVerts[m][0]=-aVerts[m][2];
							aVerts[m][1]=-aVerts[m][1];
							aVerts[m][2]=0.0F;
						}
						break;

					// Y (top) view
					case REND_CY:
						// Loop thru origins
						for(m=0;m<2;m++)
						{
							// Non-perspective transformation
							fWork=-aVerts[m][0];
							aVerts[m][0]=-aVerts[m][2];
							aVerts[m][1]=fWork;
							aVerts[m][2]=0.0F;
						}
						break;

					// Z (side) view
					case REND_CZ:
						// Loop thru origins
						for(m=0;m<2;m++)
						{
							// Non-perspective transformation
							// aVerts[m][0]=aVerts[m][0];
							aVerts[m][1]=-aVerts[m][1];
							aVerts[m][2]=0.0F;
						}
						break;
				}

				// Check dot product
				if(fDot > 0.0F)
				{
					// Draw line
					MoveToEx(hdc,(int)(aVerts[0][0]*pContext->fScale-pContext->fOffsetx),
						(int)(aVerts[0][1]*pContext->fScale-pContext->fOffsety),NULL);
					LineTo(hdc,(int)(aVerts[1][0]*pContext->fScale-pContext->fOffsetx),
						(int)(aVerts[1][1]*pContext->fScale-pContext->fOffsety));
				}
				
				// Reset to default
				SelectObject(hdc,hdefault);

				// Delete pen
				DeleteObject(hpen);
			}

			// Check level-of-detail
			if(pContext->nDetail == REND_CLOW)
			{
				// Copy object
				memcpy(&rCubeObj,pCur,sizeof(RENDOBJ));

				// Set number of polys and vertices to cube
				rCubeObj.nPolys=REND_DNPOLYS/LWOB_SIZE;
				rCubeObj.nVerts=REND_DNVERTS/3;

				// Calculate min and max
				aMin[0]=-rCubeObj.aSize[0]/2.0F+rCubeObj.aOrig[0];
				aMin[1]=-rCubeObj.aSize[1]/2.0F+rCubeObj.aOrig[1];
				aMin[2]=-rCubeObj.aSize[2]/2.0F+rCubeObj.aOrig[2];

				aMax[0]=rCubeObj.aSize[0]/2.0F+rCubeObj.aOrig[0];
				aMax[1]=rCubeObj.aSize[1]/2.0F+rCubeObj.aOrig[1];
				aMax[2]=rCubeObj.aSize[2]/2.0F+rCubeObj.aOrig[2];

				// Calculate cube vertices
				aCubeVerts[0]=aMin[0];
				aCubeVerts[1]=aMin[1];
				aCubeVerts[2]=aMin[2];

				aCubeVerts[3]=aMin[0];
				aCubeVerts[4]=aMax[1];
				aCubeVerts[5]=aMax[2];

				aCubeVerts[6]=aMin[0];
				aCubeVerts[7]=aMax[1];
				aCubeVerts[8]=aMin[2];

				aCubeVerts[9]=aMin[0];
				aCubeVerts[10]=aMin[1];
				aCubeVerts[11]=aMax[2];

				aCubeVerts[12]=aMax[0];
				aCubeVerts[13]=aMin[1];
				aCubeVerts[14]=aMin[2];

				aCubeVerts[15]=aMax[0];
				aCubeVerts[16]=aMax[1];
				aCubeVerts[17]=aMax[2];

				aCubeVerts[18]=aMax[0];
				aCubeVerts[19]=aMax[1];
				aCubeVerts[20]=aMin[2];

				aCubeVerts[21]=aMax[0];
				aCubeVerts[22]=aMin[1];
				aCubeVerts[23]=aMax[2];

				// Set pointers to polys and vertices to cube
				rCubeObj.pPolys=REND_DAPOLYS;
				rCubeObj.pVerts=aCubeVerts;

				// Set pointer
				pCur=&rCubeObj;
			}
			
			// Check mode and set color
			if(pCur->nMode != REND_OSHOW) lb.lbColor=pContext->nSelCol;
			else lb.lbColor=pCur->nCol;
	
			// Create pen
			hpen=ExtCreatePen(PS_COSMETIC|PS_SOLID,1,&lb,0,NULL); 

			// Set color and width
			hdefault=(HPEN)SelectObject(hdc,hpen);

			// Check context mode
			switch(pContext->nMode)
			{
				// Camera view
				case REND_CCAM:
					// Loop thru polys
					for(n=0;n<pCur->nPolys;n++)
					{
						// Reset dot product
						fDot=1.0F;

						// Reset flag
						nFlag=FALSE;
							
						// Loop thru vertices
						for(m=0;m<=LWOB_SIZE;m++)
						{
							// Get vertex index
							nVert=pCur->pPolys[n*LWOB_SIZE+(m%LWOB_SIZE)];

							// Check vertex index
							if(nVert < pCur->nVerts)
							{
								// Get vertex
								memcpy(aVerts[m],&(pCur->pVerts[nVert*3]),nSizeOfFloatTimes3);

								// Object-to-world transformation
								matVectProduct(NULL,aVerts[m],pCur->aTransform);

								// Calculate camera to vertex vector
								aWork[0]=aVerts[m][0]-pContext->aEye[0];
								aWork[1]=aVerts[m][1]-pContext->aEye[1];
								aWork[2]=aVerts[m][2]-pContext->aEye[2];

								// Calc dot product
								if(fDot > 0.0F) fDot=aLookAt[0]*aWork[0]+aLookAt[1]*aWork[1]+aLookAt[2]*aWork[2];

								// Check dot product
								if(fDot > 0.0F)
								{
									// World-to-camera transformations
									matVectProduct(NULL,aVerts[m],pContext->aTransform);

									// Scale vertex
									aVerts[m][0]/=aVerts[m][2];
									aVerts[m][1]/=aVerts[m][2];
									aVerts[m][2]=0.0F;
								}
								else
								{
									// Check index
									if(m == 0)
									{
										// Set flag
										nFlag=TRUE;
										break;
									}

									// Get last vertex
									memcpy(aVerts[m],aVerts[0],nSizeOfFloatTimes3);
								}
							}
							else
							{
								// Check index
								if(m == 0)
								{
									// Set flag
									nFlag=TRUE;
									break;
								}
									
								// Get last vertex
								memcpy(aVerts[m],aVerts[0],nSizeOfFloatTimes3);
							}
						}

						// Check flag
						if(nFlag == TRUE) continue;

						// Reset area
						fArea=0.0F;

						// Loop thru vertices and calc poly area
						for(m=0;m<LWOB_SIZE;m++)
							fArea+=(aVerts[m][0]-aVerts[(m+1)%LWOB_SIZE][0])*(aVerts[m][1]+aVerts[(m+1)%LWOB_SIZE][1]);

						// Check poly area
						if(fArea*0.5F < 0.0F) continue;

						// Loop thru vertices
						for(m=0;m<=LWOB_SIZE;m++)
						{
							// Draw poly
							if(m == 0) MoveToEx(hdc,(int)(aVerts[m][0]*pContext->fScale-pContext->fOffsetx),
								(int)(aVerts[m][1]*pContext->fScale-pContext->fOffsety),NULL);
							else LineTo(hdc,(int)(aVerts[m][0]*pContext->fScale-pContext->fOffsetx),
								(int)(aVerts[m][1]*pContext->fScale-pContext->fOffsety));
						}
					}
					break;

				// X (front) view
				case REND_CX:
					// Loop thru polys
					for(n=0;n<pCur->nPolys;n++)
					{
						// Reset flag
						nFlag=FALSE;
							
						// Loop thru vertices
						for(m=0;m<=LWOB_SIZE;m++)
						{
							// Get vertex index
							nVert=pCur->pPolys[n*LWOB_SIZE+(m%LWOB_SIZE)];

							// Check vertex index
							if(nVert < pCur->nVerts)
							{
								// Get vertex
								memcpy(aVerts[m],&(pCur->pVerts[nVert*3]),nSizeOfFloatTimes3);

								// Object-to-world transformation
								matVectProduct(NULL,aVerts[m],pCur->aTransform);

								// Non-perspective transformation
								aVerts[m][0]=-aVerts[m][2];
								aVerts[m][1]=-aVerts[m][1];
								aVerts[m][2]=0.0F;
							}
							else
							{
								// Check index
								if(m == 0)
								{
									// Set flag
									nFlag=TRUE;
									break;
								}
									
								// Get last vertex
								memcpy(aVerts[m],aVerts[0],nSizeOfFloatTimes3);
							}
						}

						// Check flag
						if(nFlag == TRUE) continue;

						// Reset area
						fArea=0.0F;

						// Loop thru vertices and calc poly area
						for(m=0;m<LWOB_SIZE;m++)
							fArea+=(aVerts[m][0]-aVerts[(m+1)%LWOB_SIZE][0])*(aVerts[m][1]+aVerts[(m+1)%LWOB_SIZE][1]);

						// Check poly area
						if(fArea*0.5F < 0.0F) continue;

						// Loop thru vertices
						for(m=0;m<=LWOB_SIZE;m++)
						{
							// Draw poly
							if(m == 0) MoveToEx(hdc,(int)(aVerts[m][0]*pContext->fScale-pContext->fOffsetx),
								(int)(aVerts[m][1]*pContext->fScale-pContext->fOffsety),NULL);
							else LineTo(hdc,(int)(aVerts[m][0]*pContext->fScale-pContext->fOffsetx),
								(int)(aVerts[m][1]*pContext->fScale-pContext->fOffsety));
						}
					}
					break;

				// Y (top) view
				case REND_CY:
					// Loop thru polys
					for(n=0;n<pCur->nPolys;n++)
					{
						// Reset flag
						nFlag=FALSE;
							
						// Loop thru vertices
						for(m=0;m<=LWOB_SIZE;m++)
						{
							// Get vertex index
							nVert=pCur->pPolys[n*LWOB_SIZE+(m%LWOB_SIZE)];

							// Check vertex index
							if(nVert < pCur->nVerts)
							{
								// Get vertex
								memcpy(aVerts[m],&(pCur->pVerts[nVert*3]),nSizeOfFloatTimes3);

								// Object-to-world transformation
								matVectProduct(NULL,aVerts[m],pCur->aTransform);

								// Non-perspective transformation
								fWork=-aVerts[m][0];
								aVerts[m][0]=-aVerts[m][2];
								aVerts[m][1]=fWork;
								aVerts[m][2]=0.0F;
							}
							else
							{
								// Check index
								if(m == 0)
								{
									// Set flag
									nFlag=TRUE;
									break;
								}
									
								// Get last vertex
								memcpy(aVerts[m],aVerts[0],nSizeOfFloatTimes3);
							}
						}

						// Check flag
						if(nFlag == TRUE) continue;

						// Reset area
						fArea=0.0F;

						// Loop thru vertices and calc poly area
						for(m=0;m<LWOB_SIZE;m++)
							fArea+=(aVerts[m][0]-aVerts[(m+1)%LWOB_SIZE][0])*(aVerts[m][1]+aVerts[(m+1)%LWOB_SIZE][1]);

						// Check poly area
						if(fArea*0.5F < 0.0F) continue;

						// Loop thru vertices
						for(m=0;m<=LWOB_SIZE;m++)
						{
							// Draw poly
							if(m == 0) MoveToEx(hdc,(int)(aVerts[m][0]*pContext->fScale-pContext->fOffsetx),
								(int)(aVerts[m][1]*pContext->fScale-pContext->fOffsety),NULL);
							else LineTo(hdc,(int)(aVerts[m][0]*pContext->fScale-pContext->fOffsetx),
								(int)(aVerts[m][1]*pContext->fScale-pContext->fOffsety));
						}
					}
					break;

				// Z (side) view
				case REND_CZ:
					// Loop thru polys
					for(n=0;n<pCur->nPolys;n++)
					{
						// Reset flag
						nFlag=FALSE;
							
						// Loop thru vertices
						for(m=0;m<=LWOB_SIZE;m++)
						{
							// Get vertex index
							nVert=pCur->pPolys[n*LWOB_SIZE+(m%LWOB_SIZE)];

							// Check vertex index
							if(nVert < pCur->nVerts)
							{
								// Get vertex
								memcpy(aVerts[m],&(pCur->pVerts[nVert*3]),nSizeOfFloatTimes3);

								// Object-to-world transformation
								matVectProduct(NULL,aVerts[m],pCur->aTransform);

								// Non-perspective transformation
								// aVerts[m][0]=aVerts[m][0];
								aVerts[m][1]=-aVerts[m][1];
								aVerts[m][2]=0.0F;
							}
							else
							{
								// Check index
								if(m == 0)
								{
									// Set flag
									nFlag=TRUE;
									break;
								}
									
								// Get last vertex
								memcpy(aVerts[m],aVerts[0],nSizeOfFloatTimes3);
							}
						}

						// Check flag
						if(nFlag == TRUE) continue;

						// Reset area
						fArea=0.0F;

						// Loop thru vertices and calc poly area
						for(m=0;m<LWOB_SIZE;m++)
							fArea+=(aVerts[m][0]-aVerts[(m+1)%LWOB_SIZE][0])*(aVerts[m][1]+aVerts[(m+1)%LWOB_SIZE][1]);

						// Check poly area
						if(fArea*0.5F < 0.0F) continue;

						// Loop thru vertices
						for(m=0;m<=LWOB_SIZE;m++)
						{
							// Draw poly
							if(m == 0) MoveToEx(hdc,(int)(aVerts[m][0]*pContext->fScale-pContext->fOffsetx),
								(int)(aVerts[m][1]*pContext->fScale-pContext->fOffsety),NULL);
							else LineTo(hdc,(int)(aVerts[m][0]*pContext->fScale-pContext->fOffsetx),
								(int)(aVerts[m][1]*pContext->fScale-pContext->fOffsety));
						}
					}
					break;
			}

			// Reset to default
			SelectObject(hdc,hdefault);

			// Delete pen
			DeleteObject(hpen);
		}
		
		// Set pointer to next object
		pCur=pCur->pNext;
	}

	// Draw cursor
	if(pContext->nMode != REND_CCAM)
	{
		// Copy cursor
		memcpy(aCursor,pContext->aCursor,nSizeOfFloatTimes3);

		// Check context mode
		switch(pContext->nMode)
		{
			// X (front) view
			case REND_CX:
				// Set cursor
				aCursor[0]=-aCursor[2]*pContext->fScale-pContext->fOffsetx;
				aCursor[1]=-aCursor[1]*pContext->fScale-pContext->fOffsety;
				aCursor[2]=0.0F;
				break;

			// Y (top) view
			case REND_CY:
				// Set cursor
				fWork=-aCursor[0]*pContext->fScale-pContext->fOffsety;
				aCursor[0]=-aCursor[2]*pContext->fScale-pContext->fOffsetx;
				aCursor[1]=fWork;
				aCursor[2]=0.0F;
				break;

			// Z (side) view
			case REND_CZ:
				// Set cursor
				aCursor[0]=aCursor[0]*pContext->fScale-pContext->fOffsetx;
				aCursor[1]=-aCursor[1]*pContext->fScale-pContext->fOffsety;
				aCursor[2]=0.0F;
				break;
		}

		// Set band box color
		lb.lbColor=pContext->nCursCol;

		// Create pen
		hpen=ExtCreatePen(PS_COSMETIC|PS_SOLID,1,&lb,0,NULL); 

		// Set color and width
		hdefault=(HPEN)SelectObject(hdc,hpen);

		// Draw cursor
		MoveToEx(hdc,(int)aCursor[0]-REND_SCURS,(int)aCursor[1],NULL);
		LineTo(hdc,(int)aCursor[0]+REND_SCURS,(int)aCursor[1]);
		MoveToEx(hdc,(int)aCursor[0],(int)aCursor[1]-REND_SCURS,NULL);
		LineTo(hdc,(int)aCursor[0],(int)aCursor[1]+REND_SCURS);

		// Reset to default
		SelectObject(hdc,hdefault);

		// Delete pen
		DeleteObject(hpen);
	}

	return(OK);
}

