//
// mex.h
// Homeworld's GEO & MEX defs
//

#ifndef _MEX_H
#define _MEX_H

#include "types.h"

#define MESH_FileID                     "RMF97ba"
#define MESH_FileIDLength               8

typedef struct GeoFileHeader
{
    char   identifier[MESH_FileIDLength];
    udword version;
    char*  pName;
    udword fileSize;
    udword localSize;
    udword nPublicMaterials;
    udword nLocalMaterials;
    udword oPublicMaterials;
    udword oLocalMaterials;
    udword nPolygonObjects;
    ubyte  reserved[24];
} GeoFileHeader;

typedef struct polyentry
{
    sdword iFaceNormal;
    uword  iV0;
    uword  iV1;
    uword  iV2;
    uword  iMaterial;
    real32 s0;
    real32 t0;
    real32 s1;
    real32 t1;
    real32 s2;
    real32 t2;
    uword  flags;
    ubyte  reserved[2];
} polyentry;

typedef struct vertexentry
{
    real32 x, y, z;
    sdword iVertexNormal;
} vertexentry;

typedef struct normalentry
{
    real32 x, y, z;
    ubyte  pad[4];
} normalentry;

typedef struct materialentry
{
    char* pName;
    color ambient;
    color diffuse;
    color specular;
    real32 kAlpha;
    udword texture;
    uword  flags;
    ubyte  nFullAmbient;
    bool8  bTexturesRegistered;
    ubyte  reserved[4];
} materialentry;

typedef struct polygonobject
{
    char* pName;
    ubyte flags;
    ubyte iObject;
    uword nameCRC;
    sdword nVertices;
    sdword nFaceNormals;
    sdword nVertexNormals;
    sdword nPolygons;
    vertexentry* pVertexList;
    normalentry* pNormalList;
    polyentry*   pPolygonList;
    struct polygonobject* pMother;
    struct polygonobject* pDaughter;
    struct polygonobject* pSister;
    real32 localMatrix[16];
} polygonobject;

typedef struct meshdata
{
    char* name;
    udword localSize;
    sdword nPublicMaterials;
    sdword nLocalMaterials;
    sdword nPolygonObjects;
    materialentry* localMaterial;
    materialentry* publicMaterial;
    char* fileName;
    polygonobject object[1];
} meshdata;

meshdata* meshLoad(char* filePrefix, char* fileName);
sdword meshCountPolygons(meshdata* mesh);

#endif
