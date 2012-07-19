/*
** LWOPARSE.H : Header file for LWOPARSE.CPP.
*/

#ifndef __LWOPARSE_H
#define __LWOPARSE_H

#include "stdio.h"

#include "matrix.h"
#include "ccList.h"
#include "parsefunc.h"

#define NUM_LWO_PARSE_FUNCS				(5)
	
	//////////////////////////////////////////////////////////////////////////
	// LWO data types.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagLWOCOL4
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char pad;
	}LWOCOL4;

	typedef struct tagLWOVertex
	{
		float x;
		float y;
		float z;

		unsigned long iVertexNormal;
	} LWOVertex;

	typedef struct tagLWONormal
	{
		float x;
		float y;
		float z;
	} LWONormal;

	typedef struct tagLWOTextureCoord
	{
		float s;
		float t;

		float u;
		float v;
	} LWOTextureCoord;
	
	typedef struct tagLWOPolygon
	{
		unsigned short niVertices;
		unsigned short *piVertices;
		unsigned short iSurface;
		unsigned short iFaceNormal;

		LWOTextureCoord t0;
		LWOTextureCoord t1;
		LWOTextureCoord t2;
	} LWOPolygon;

	typedef struct tagLWOTexture
	{
		unsigned long	mappingType;

		unsigned short	TFLG;
		
		LWOVertex		TSIZ;
		LWOVertex		TCTR;
		LWOVertex		TFAL;
		LWOVertex		TVEL;

		LWOCOL4			TCLR;

		signed short	TVAL;

		float			TAMP;

		unsigned long	numTFP;
		float			*TFPn;
		unsigned long	numTIP;
		signed short	*TIPn;

		char			*TIMG;

		char			*TALP;

		unsigned short	TWRPw;
		unsigned short	TWRPh;

		float			TAAS;

		float			TOPC;
	} LWOTexture;

	enum tagLWOTextureMappingTypes
	{
		LTMT_Planar,
		LTMT_Cylindrical,
		LTMT_Spherical,

		LTMT_Last_LTMT
	};

	enum tagLWOTextureFlags
	{
		TF_MappingAxisX = 1,
		TF_MappingAxisY = 2,
		TF_MappingAxisZ = 4,
		TF_WorldCoords = 8,
		TF_NegativeImage = 16,
		TF_PixelBlending = 32,
		TF_Antialiasing = 64,

		TF_Last_TF
	};
	
	typedef struct tagLWOSurface
	{
        char            *name;
		LWOCOL4			COLR;
		
		unsigned short	FLAG;
		
		float			VLUM;
		float			VDIF;
		float			VSPC;
		float			VRFL;
		float			VTRN;

		signed short	GLOS;

		unsigned short	RFLT;

		char			*RIMG;

		float			RSAN;

		float			RIND;

		float			EDGE;

		float			SMAN;

		LWOTexture		CTEX;
		unsigned char	ctexEnabled;

		LWOTexture		DTEX;
		unsigned char	dtexEnabled;

		LWOTexture		STEX;
		unsigned char	stexEnabled;

		LWOTexture		RTEX;
		unsigned char	rtexEnabled;

		LWOTexture		TTEX;
		unsigned char	ttexEnabled;

		LWOTexture		LTEX;
		unsigned char	ltexEnabled;

		LWOTexture		BTEX;
		unsigned char	btexEnabled;
	} LWOSurface;

	enum tagLWOSurfaceFlags
	{
		SF_Luminous = 1,
		SF_Outline = 2,
		SF_Smoothing = 4,
		SF_ColorHighlights = 8,
		SF_ColorFilter = 16,
		SF_OpaqueEdge = 32,
		SF_TransparentEdge = 64,
		SF_SharpTerminator = 128,
		SF_DoubleSided = 256,
		SF_Additive = 1024,
		SF_ShadowAlpha = 2048,
		SF_BaseColor = 4096,
		SF_StripeColor = 8192,

		SF_Last_SF
	};

	//////////////////////////////////////////////////////////////////////////
	// Structure with info on LWObjects.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagLWObject
	{
		char *fileName;
		char *fullFileName;
		unsigned long flags;
		signed long duplicateIndex;
		
		LWOVertex		*pVertexList;
		unsigned long	nVertices;

		char			**pSurfaceNames;
		unsigned long	nSurfaceNames;

		LWOPolygon		*pPolyList;
		unsigned long	nPolygons;

		LWONormal		*pFaceNormalList;
		unsigned long	nFaceNormals;

		LWONormal		*pVertexNormalList;
		unsigned long	nVertexNormals;

		LWOSurface		*pSurfaceList;
		unsigned long	nSurfaces;

		unsigned long	iParentObject;			// 1 based.  0 means none.

		float			xp, yp, zp;				// Position, Scaling, and Rotation
		float			h, p, b;				// relative to the parent object.
		float			xs, ys, zs;
		float			xv, yv, zv;

		ccList			*objectMotions;
	} LWObject;

	enum tagLWObjectFlags
	{
		LWOF_UniqueObject = 1,
		LWOF_DuplicateObject = 2,
		LWOF_NullObject = 4,

		LWOF_Last_LWOF
	};

	//////////////////////////////////////////////////////////////////////////
	// Keyframe info for LWObjects.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagLWOKeyframe
	{
		float xp, yp, zp;
		float h, p, b;
		float xs, ys, zs;
		float xv, yv, zv;
		unsigned long frameNumber;
		float linearValue;
		float tension;
		float continuity;
		float bias;
	} LWOKeyframe;

	//////////////////////////////////////////////////////////////////////////
	// Container class for LWO's in lists.
	//////////////////////////////////////////////////////////////////////////
	class LWOContainer : public ccNode
	{
	private:
	protected:
	public:
		LWObject myObject;

		LWOContainer(void);
		~LWOContainer(void);
	};

	class LWOKeyframeContainer : public ccNode
	{
	private:
	protected:
	public:
		LWOKeyframe myKeyframe;

		LWOKeyframeContainer(void);
		~LWOKeyframeContainer(void);
	};
	
	//////////////////////////////////////////////////////////////////////////
	// LWO Global Data.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagLWOGlobalData
	{
		ccList			*pLWOList;
        ccList          *pNULLList;
		LWObject		*pCurrentLWObject;
		FunctionDef		*pLWOFuncList;
		LWOContainer	*pCurrentLWOContainer;
		unsigned long	iCurrentSurface;
		LWOTexture		*pCurrentTexture;
	} LWOGlobalData;
	
	//////////////////////////////////////////////////////////////////////////
	// Prototypes.
	//////////////////////////////////////////////////////////////////////////
	void InitializeLWOParse(void);
	void DeInitializeLWOParse(void);

	LWOContainer *CreateNewLWObject(char *fileName);
	void LWOParse_AddNullObject(char *name);

	LWOKeyframeContainer *CreateNewLWOKeyframe(void);

	signed long CheckDuplicateLWOFile(LWOContainer *pLWOContainer);

	signed char ConvertToTriangles(void);
	signed char ReorderPointOrder(void);
	signed char CalculateFaceNormals(void);
	signed char CalculateVertexNormals(void);

	void ReorientLWOData(void);
	
	void FixTextureName(char *pName);
	float fract(float a);
	void XYZToH(float x, float y, float z, float *h);
	void XYZToHP(float x, float y, float z, float *h, float *p);
	signed char CalculateVertexTextureCoords(LWOVertex *p0, LWOTextureCoord *t0, LWOSurface *pSurface);
	signed char CalculateTextureCoordinates(void);

	signed char LWOParse_ReadFile(FILE *fileHandle);

	// Main chunk parse functions.
	signed char LWOParse_FORM(char *currentLine, FILE *fileHandle);
	signed char LWOParse_PNTS(char *currentLine, FILE *fileHandle);
	signed char LWOParse_SRFS(char *currentLine, FILE *fileHandle);
	signed char LWOParse_POLS(char *currentLine, FILE *fileHandle);
	signed char LWOParse_SURF(char *currentLine, FILE *fileHandle);

	// Sub chunk parse functions.
	signed char LWOParse_COLR(char *currentLine, FILE *fileHandle);
	signed char LWOParse_FLAG(char *currentLine, FILE *fileHandle);

	signed char LWOParse_VLUM(char *currentLine, FILE *fileHandle);
	signed char LWOParse_VDIF(char *currentLine, FILE *fileHandle);
	signed char LWOParse_VSPC(char *currentLine, FILE *fileHandle);
	signed char LWOParse_VRFL(char *currentLine, FILE *fileHandle);
	signed char LWOParse_VTRN(char *currentLine, FILE *fileHandle);

	signed char LWOParse_LUMI(char *currentLine, FILE *fileHandle);
	signed char LWOParse_DIFF(char *currentLine, FILE *fileHandle);
	signed char LWOParse_SPEC(char *currentLine, FILE *fileHandle);
	signed char LWOParse_REFL(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TRAN(char *currentLine, FILE *fileHandle);

	signed char LWOParse_GLOS(char *currentLine, FILE *fileHandle);
	signed char LWOParse_RFLT(char *currentLine, FILE *fileHandle);
	signed char LWOParse_RIMG(char *currentLine, FILE *fileHandle);
	signed char LWOParse_RSAN(char *currentLine, FILE *fileHandle);
	signed char LWOParse_RIND(char *currentLine, FILE *fileHandle);
	signed char LWOParse_EDGE(char *currentLine, FILE *fileHandle);
	signed char LWOParse_SMAN(char *currentLine, FILE *fileHandle);

	signed char LWOParse_CTEX(char *currentLine, FILE *fileHandle);
	signed char LWOParse_DTEX(char *currentLine, FILE *fileHandle);
	signed char LWOParse_STEX(char *currentLine, FILE *fileHandle);
	signed char LWOParse_RTEX(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TTEX(char *currentLine, FILE *fileHandle);
	signed char LWOParse_LTEX(char *currentLine, FILE *fileHandle);
	signed char LWOParse_BTEX(char *currentLine, FILE *fileHandle);

	signed char LWOParse_TFLG(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TSIZ(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TCTR(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TFAL(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TVEL(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TCLR(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TVAL(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TAMP(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TFPn(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TIPn(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TIMG(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TALP(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TWRP(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TAAS(char *currentLine, FILE *fileHandle);
	signed char LWOParse_TOPC(char *currentLine, FILE *fileHandle);

	//////////////////////////////////////////////////////////////////////////
	// Externs.
	//////////////////////////////////////////////////////////////////////////
	extern LWOGlobalData	*gLWOData;

#endif