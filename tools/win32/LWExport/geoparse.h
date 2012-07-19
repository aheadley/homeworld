/*
** GEOPARSE.H : Header file for GEOPARSE.CPP.
*/

#ifndef __GEOPARSE_H
#define __GEOPARSE_H

#include "stdio.h"
#include "matrix.h"

#define GEO_FILE_ID		"RMF97ba"
#define GEO_VERSION		(0x00000402)

#define RGB(r,g,b)		(((unsigned long)(b) << 16) + ((unsigned long)(g) << 8) + (unsigned long)(r))

#define NAMES_PER_POLY  0

	//////////////////////////////////////////////////////////////////////////
	// GEO Headers.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagGeoFileHeader
	{
		char			identifier[8];					// File identifier.
		unsigned long	version;						// File version.
		unsigned long	oName;							// Offset to a file name.
		unsigned long	fileSize;						// File size (in bytes), not counting this header.
		unsigned long	localSize;						// Object size (in bytes).
		unsigned long	nPublicMaterials;				// Number of public materials.
		unsigned long	nLocalMaterials;				// Number of local materials.
		unsigned long	oPublicMaterials;				// Offset to public material headers.
		unsigned long	oLocalMaterials;				// Offset to local material headers.
		unsigned long	nPolygonObjects;				// Number of polygon objects.
		char			reserved[24];					// Reserved for future use.
	} GeoFileHeader;

	typedef struct tagPolygonObject
	{
		unsigned long	oName;							// Name CRC for animation.
		unsigned long	flags;							// General flags for this object.
		unsigned long	nVertices;						// Number of vertices in vertex list for this object.
		unsigned long	nFaceNormals;					// Number of face normals for this object.
		unsigned long	nVertexNormals;					// Number of vertex normals for this object.
		unsigned long	nPolygons;						// Number of polygons in this object.
		unsigned long	oVertexList;					// Offset to the vertex list in this object.
		unsigned long	oNormalList;					// Offset to the normal list in this object.
		unsigned long	oPolygonList;					// Offset to the polygon list in this object.
		unsigned long	oParent;						// Offset to parent object. (0 if none)
		unsigned long	oChild;							// Offset to child object. (0 if none)
		unsigned long	oSibling;						// Offset to sibling object. (0 if none)
		hmatrix			localMatrix;					// Local transformation matrix.
	} PolygonObject;

	enum tagPolygonObjectFlags
	{
		POF_UniqueObject = 1,
		POF_DuplicateObject = 2,

		POF_LocalChildObject = 4,
		POF_GlobalChildObject = 8,

		POF_LocalParentObject = 16,
		POF_GlobalParentObject = 32,

		POF_LocalSiblingObject = 64,
		POF_GlobalSiblingObject = 128,

		POF_Last_POF
	};
	
	typedef struct tagPolygon
	{
		signed long		iFaceNormal;					// Index into the face normal list.
		unsigned short	iV0;							// Index to V0 of the polygon.
		unsigned short	iV1;							// Index to V1 of the polygon.
		unsigned short	iV2;							// Index to V2 of the polygon.
		unsigned short	iMaterial;						// Index into material list.
		float			s0;
		float			t0;
		float			s1;
		float			t1;
		float			s2;
		float			t2;
		unsigned short	flags;							// Flags for this polygon.
		unsigned char	reserved[2];					// Reserved for later use.
#if NAMES_PER_POLY
        char *surfaceName;
#endif
	} Polygon;

	typedef struct tagVertex
	{
		float			x;								// X component of this vertex.
		float			y;								// Y component of this vertex.
		float			z;								// Z component of this vertex.
		signed long		iVertexNormal;					// Index into the point normal list.
	} Vertex;

	typedef struct tagNormal
	{
		float			x;								// X component of this normal.
		float			y;								// Y component of this normal.
		float			z;								// Z component of this normal.
		char			pad[4];							// Reserved for later use.
	} Normal;

	typedef struct tagMaterialDescriptor
	{
		unsigned long	oName;							// Offset to name of material (may be a CRC32).
		unsigned long	cAmbient;						// Ambient color information.
		unsigned long	cDiffuse;						// Diffuse color information.
		unsigned long	cSpecular;						// Specular color information.
		float			kAlpha;							// Alpha blending information.
		unsigned long	oTextureName;					// Offset to texture information (or CRC32).
		unsigned short	flags;							// Flags for this material.
		unsigned char	nFullAmbient;					// Number of self-illuminating materials in texture.
		char			reserved[5];					// Reserved for later use.
	} MaterialDescriptor;

	enum tagMaterialDescriptorFlags
	{
		MDF_AnimatingTextureMap = 1,
		MDF_Smoothing = 2,
		MDF_AllowFlatShading = 4,
		MDF_2Sided = 8,
		MDF_BaseColor = 16,
		MDF_StripeColor = 32,
		MDF_Luminous = 64,
		MDF_Last_MDF,
	};

	enum tagPolygonFlags
	{
		PF_EnableEdgeCache = 1,
		PF_InterpolateRGB = 2,
		PF_StoreComputedEdges = 4,
	};

	//////////////////////////////////////////////////////////////////////////
	// GEO Global Data.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagGEOGlobalData
	{
		GeoFileHeader		*pGEOFileHeader;
		PolygonObject		*pPolygonObjects;
		MaterialDescriptor	*pMaterials;
        char                **pTextureNames;
		Polygon				**pPolygons;
		Vertex				**pVertices;
		Normal				**pNormals;
	} GEOGlobalData;

	//////////////////////////////////////////////////////////////////////////
	// Prototypes.
	//////////////////////////////////////////////////////////////////////////
	void InitializeGEOParse(void);
	void DeInitializeGEOParse(void);

	unsigned long FindPolygonObjectOffset(unsigned long index);
	signed char GEOParse_SetupHierarchy(void);
	signed char GEOParse_OptomizeModeChanges(void);
	signed char GEOParse_BuildLocalMatrix(void);
	signed char GEOParse_SetupGEOData(void);
	signed char GEOParse_WriteFile(FILE *fileHandle);

	//////////////////////////////////////////////////////////////////////////
	// Externs.
	//////////////////////////////////////////////////////////////////////////
	extern GEOGlobalData	*gGEOData;

#endif
