// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef RENDER_H
#define RENDER_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

// Default object (cube, 1 cubic meter)
#define REND_DNPOLYS	36	// 3*12 triangles
#define REND_DNVERTS	24	// 3*8 vertices
#define REND_DAORIG		aDefOrig
#define REND_DASIZE		aDefSize
#define REND_DFSEL		500.0F;
#define REND_DAPOLYS	aDefPolys
#define REND_DAVERTS	aDefVerts

// Object mode constants
#define REND_OHIDE	0
#define REND_OSHOW	1
#define REND_OSEL	2
#define REND_OTOG	3

// Object defaults
#define REND_OSCALE	1.0F

// Context mode constants
#define REND_CCAM	0
#define REND_CX		1
#define REND_CY		2
#define REND_CZ		3

// Context level-of-detail constants
#define REND_CHIGH	0
#define REND_CLOW	1

// Context defaults
#define REND_CSCALE	0.1F
#define REND_CGSIZE	1000.0F
#define REND_CGLIM	10.0F

// Color constants
#define REND_NONE	-1
#define REND_BLACK	RGB(0,0,0)
#define REND_WHITE	RGB(255,255,255)
#define REND_RED	RGB(255,0,0)
#define REND_GREEN	RGB(0,255,0)
#define REND_BLUE	RGB(0,0,255)

// Perspective constants
#define REND_PFOV		75.0F
#define REND_PASPECT	1.0F
#define REND_PCNEAR		1.0F	
#define REND_PCFAR		40000.0F

// Angle constants
#define REND_AMIN		-180.0F
#define REND_AMAX		180.0F

// Selection constants
#define REND_STUNE		1.0F
#define REND_SNONE		0
#define REND_SEYE		1
#define REND_SFOCUS		2

// Size constants
#define REND_SCAM		10
#define REND_SCURS		10
#define REND_SROT		2

// Macros
#define rfabs(x) ((x<0.0F)?(-(x)):(x))

// Structure typecasts
typedef struct rendobj_stc
{
	int nMode;					// Mode: hide, show, select
	int nKey;					// Key
	int nCol;					// Color

	char szUID[256];			// Unique identifier
	
	short nVerts;				// # of vertices
	short nPolys;				// # of polygons/normals

	short *pPolys;				// Polygons (triplets)

	float *pVerts;				// Vertices (triplets)

	float *pNorms;				// Normals (triplets)

	float fSel;					// Selection average

	float aSize[3];				// Size Data
	float aOrig[3];				// Origin data

	float aTrans[3];			// Translation data
	float aScale[3];			// Scale data
	float aRot[3];				// Rotation data
	
	float aTransform[16];		// Object-to-world transformation matrix

	struct rendobj_stc *pPrev;	// Previous object
	struct rendobj_stc *pNext;	// Next object
	struct rendobj_stc *pLink;	// Link object
}
RENDOBJ;

typedef struct rendcont_stc
{
	void *pDdc;			// Direct draw context

	int nMode;			// Mode: X, Y, Z, camera
	int nDetail;		// Level-of-detail: high or low

	int nBandCol;		// Band box color
	int nCamCol;		// Camera color
	int nGridCol;		// Grid color
	int nRotCol;		// Rotation color
	int nSelCol;		// Selection color
	int nCursCol;		// Cursor color

	float fOffsetx;		// X offset
	float fOffsety;		// Y offset
	
	float fWidth;		// Viewport width
	float fHeight;		// Viewport height
	
	float fScale;		// Scale factor

	float fGridSize;	// Grid size

	float aBandPos[4];	// Band box position

	float aEye[3];		// Camera eye
	float aFocus[3];	// Camera focus

	float aCursor[3];	// Cursor
	
	float aTransform[16];	// World-to-camera transformation matrix

	struct rendobj_stc *pRotObj;	// Rotation object
}
RENDCONT;

// Function declarations
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int __stdcall rendInit(void);
__declspec(dllexport) int __stdcall rendClean(void);

__declspec(dllexport) int __stdcall rendAngCheck(float *x,float *y,float *z);
__declspec(dllexport) int __stdcall rendAngConv(float *fAngh,float *fAngv,float x,float y,float z);

__declspec(dllexport) int __stdcall rendNewObj(RENDOBJ **pObj,int nKey,char *szUID);
__declspec(dllexport) int __stdcall rendFindObj(RENDOBJ **pObj,int nKey);
__declspec(dllexport) int __stdcall rendDelObj(RENDOBJ *pObj);
__declspec(dllexport) int __stdcall rendResetObj(RENDOBJ *pObj);
__declspec(dllexport) int __stdcall rendSetObjLink(RENDOBJ *pObj,long pLink);
__declspec(dllexport) int __stdcall rendGetObjTrans(RENDOBJ *pObj,float *x,float *y,float *z);
__declspec(dllexport) int __stdcall rendGetObjScale(RENDOBJ *pObj,float *x,float *y,float *z);
__declspec(dllexport) int __stdcall rendGetObjSize(RENDOBJ *pObj,float *x,float *y,float *z);
__declspec(dllexport) int __stdcall rendGetObjRot(RENDOBJ *pObj,float *x,float *y,float *z);
__declspec(dllexport) int __stdcall rendTransObj(RENDOBJ *pObj,float x,float y,float z);
__declspec(dllexport) int __stdcall rendScaleObj(RENDOBJ *pObj,float x,float y,float z);
__declspec(dllexport) int __stdcall rendRotObj(RENDOBJ *pObj,float x,float y,float z);
__declspec(dllexport) int __stdcall rendGetObjMode(RENDOBJ *pObj,int *nMode);
__declspec(dllexport) int __stdcall rendSetObjMode(RENDOBJ *pObj,int nMode);
__declspec(dllexport) int __stdcall rendSetObjCol(RENDOBJ *pObj,int nCol);

__declspec(dllexport) int __stdcall rendNewCont(RENDCONT **pContext,HWND hWnd,int nMode);
__declspec(dllexport) int __stdcall rendDelCont(RENDCONT *pContext);
__declspec(dllexport) int __stdcall rendSetContDetail(RENDCONT *pContext,int nDetail);
__declspec(dllexport) int __stdcall rendSetContView(RENDCONT *pContext,float x,float y,float w,float h);
__declspec(dllexport) int __stdcall rendSetContScale(RENDCONT *pContext,float fScale);
__declspec(dllexport) int __stdcall rendSetContBand(RENDCONT *pContext,float *aPos,int nCol);
__declspec(dllexport) int __stdcall rendSetContGrid(RENDCONT *pContext,float fSize,int nCol);
__declspec(dllexport) int __stdcall rendSetContRot(RENDCONT *pContext,RENDOBJ *pObj,int nCol);
__declspec(dllexport) int __stdcall rendSetContCursor(RENDCONT *pContext,float *aCursor,int nCol);
__declspec(dllexport) int __stdcall rendSetContSel(RENDCONT *pContext,int nCol);
__declspec(dllexport) int __stdcall rendCheckContSel(RENDCONT *pContext,int nMode,int *nKey);
__declspec(dllexport) int __stdcall rendCheckContCamera(RENDCONT *pContext,float x,float y,int *nSel);
__declspec(dllexport) int __stdcall rendGetContCamera(RENDCONT *pContext,float *aEye,float *aFocus);
__declspec(dllexport) int __stdcall rendSetContCamera(RENDCONT *pContext,float *aEye,float *aFocus,int nCol);
__declspec(dllexport) int __stdcall rendTransContCamera(RENDCONT *pContext,float x,float y);
__declspec(dllexport) int __stdcall rendScaleContCamera(RENDCONT *pContext,float nScale);
__declspec(dllexport) int __stdcall rendRotContCamera(RENDCONT *pContext,float fAngh,float fAngv);

__declspec(dllexport) int __stdcall rendUpdateCont(RENDCONT *pContext);
__declspec(dllexport) int __stdcall rendResizeCont(RENDCONT *pContext);
__declspec(dllexport) int __stdcall rendPaintCont(RENDCONT *pContext);

__declspec(dllexport) int __stdcall rendGetSel(char *szPrefix,short *nCount,char **szKeys);
__declspec(dllexport) int __stdcall rendSetSel(char *szPrefix,char *szKeys);
__declspec(dllexport) int __stdcall rendTransSel(float x,float y,float z);
__declspec(dllexport) int __stdcall rendScaleSel(float x,float y,float z);
__declspec(dllexport) int __stdcall rendRotSel(RENDCONT *pContext,float x,float y,float z);

// Internal functions
int rendRendCont(RENDCONT *pContext,HDC hdc);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif
