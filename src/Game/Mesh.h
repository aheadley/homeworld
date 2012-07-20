/*=============================================================================
    Name    : mesh.h
    Purpose : Definitions for loading/saving mesh files.

    Created 6/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MESH_H
#define ___MESH_H

#include "Types.h"
#include "color.h"
#include "Matrix.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define MESH_FORCED_COLORS      0               //force the red and blue colors
#define MESH_LOAD_DUMMY_TEXTURE 0               //load in a dummy texture
#define MESH_DUMMY_ST_COORDS    0               //compute default s/t coords
#define MESH_MATERIAL_STATS     1               //display material statistics
#define MESH_HACK_TEAM_COLORS   0               //hack team colors
#define MESH_TEAM_COLORS        0               //enable auto-team coloring of certain surfaces
#define MESH_SPECULAR           0               //enable specification of specular material attributes
#define MESH_PRE_CALLBACK       0
#define MESH_SURFACE_NAME_DEBUG 0

#ifndef HW_Release

#define MESH_VERBOSE_LEVEL      1
#define MESH_ERROR_CHECKING     1               //general error checking
#define MESH_ANAL_CHECKING      1               //extra-anal error checking
#define MESH_RETAIN_FILENAMES   1               //keep a copy of the fileName
#define MESH_MORPH_DEBUG        1               //debug morph render code

#else //HW_Debug

#define MESH_VERBOSE_LEVEL      0
#define MESH_ERROR_CHECKING     0               //general error checking
#define MESH_ANAL_CHECKING      1               //extra-anal error checking
#define MESH_RETAIN_FILENAMES   1               //keep a copy of the fileName
#define TC_ALLOW_PCX            1               //load .PCX file if layered format not about
#define TC_SPECIAL_DEBUG        1               //special debugging modes active
#define TC_3DFX_HACKS           1
#define MESH_MORPH_DEBUG        0

#endif //HW_Debug

/*=============================================================================
    Definitions
=============================================================================*/
#define MESH_FileID                     "RMF97ba"
#define MESH_FileID2                    "RMF99ba"
#define MESH_FileIDLength               8
#define MESH_Version                    (0x00000402)

#ifdef _WIN32
#define MESH_TextureSubDir              "Textures\\"
#else
#define MESH_TextureSubDir              "Textures/"
#endif

#define MDF_AnimatingTextureMap         1
#define MDF_Smoothing                   2
#define MDF_AllowFlatShading            4
#define MDF_2Sided                      8
#define MDF_BaseColor                   16
#define MDF_StripeColor                 32
#define MDF_SelfIllum                   64

//mesh polygon modes for rendering
#define MPM_Flat                        0
#define MPM_Texture                     1
#define MPM_Smooth                      2
#define MPM_SmoothTexture               3

//hierarchy flags for polygon objects
#define POF_UniqueObject                1
#define POF_DuplicateObject             2
#define POF_LocalChildObject            4
#define POF_GlobalChildObject           8
#define POF_LocalParentObject           16
#define POF_GlobalParentObject          32
#define POF_LocalSiblingObject          64
#define POF_GlobalSiblingObject         128
#define POF_Last_POF                    129

//hierarchy binding flags
#define HBF_Azimuth                     0x00000100  //can rotate azimuth
#define HBF_Declination                 0x00000200  //can rotate declination
#define HBF_Recoil                      0x00000400  //can recoil
#define HBM_UpdateFrequency             0x000000ff  //8 bits for update frequency
#define HB_UpdateFrequencyMax           0x00000100  //limit of update frequency

//polygon flags for fixing up uv seaming
#define PAC_S0_0  0x0001
#define PAC_S0_1  0x0002
#define PAC_T0_0  0x0004
#define PAC_T0_1  0x0008
#define PAC_S1_0  0x0010
#define PAC_S1_1  0x0020
#define PAC_T1_0  0x0040
#define PAC_T1_1  0x0080
#define PAC_S2_0  0x0100
#define PAC_S2_1  0x0200
#define PAC_T2_0  0x0400
#define PAC_T2_1  0x0800
#define PAC_FIXED 0x1000

/*=============================================================================
    Type definitions:
=============================================================================*/
//structures used for mesh files
typedef struct tagGeoFileHeader
{
    char    identifier[MESH_FileIDLength];  // File identifier.
    udword  version;                        // File version.
    char   *pName;                          // Offset to a file name.
    udword  fileSize;                       // File size (in bytes), not counting this header.
    udword  localSize;                      // Object size (in bytes).
    udword  nPublicMaterials;               // Number of public materials.
    udword  nLocalMaterials;                // Number of local materials.
    udword  oPublicMaterial;                //list of public materials
    udword  oLocalMaterial;                 //list of local materials
    udword  nPolygonObjects;                // Number of polygon objects.
    ubyte   reserved[24];                   // Reserved for future use.
}
GeoFileHeader;

typedef struct tagpolygon
{
    sdword  iFaceNormal;                    // Index into the face normal list.
    uword  iV0;                             // Index to V0 of the polygon.
    uword  iV1;                             // Index to V1 of the polygon.
    uword  iV2;                             // Index to V2 of the polygon.
    uword  iMaterial;                       // Index into material list.
    real32 s0;
    real32 t0;
    real32 s1;
    real32 t1;
    real32 s2;
    real32 t2;
    uword  flags;                           // Flags for this polygon.
    ubyte reserved[2];
}
polyentry;

typedef struct tagvertex
{
    real32 x;                               // X component of this vertex.
    real32 y;                               // Y component of this vertex.
    real32 z;                               // Z component of this vertex.
    sdword iVertexNormal;                   // Index into the point normal list.
}
vertexentry;

typedef struct tagnormal
{
    real32 x;                               // X component of this normal.
    real32 y;                               // Y component of this normal.
    real32 z;                               // Z component of this normal.
    ubyte  pad[4];                          // Reserved for later use.
}
normalentry;

typedef struct tagmaterialentry
{
    char  *pName;                           // Offset to name of material (may be a CRC32).
    color  ambient;                         // Ambient color information.
    color  diffuse;                         // Diffuse color information.
    color  specular;                        // Specular color information.
    real32 kAlpha;                          // Alpha blending information.
    udword texture;                         // Pointer to texture information (or CRC32).
    uword  flags;                           // Flags for this material.
    ubyte  nFullAmbient;                    // Number of self-illuminating colors in texture.
    bool8  bTexturesRegistered;             // Set to TRUE when some textures have been registered.
    char   *textureNameSave;                // the name of the texture, after the texture has been registered
}
materialentry;

typedef struct polygonobject
{
    char *pName;                            // Name for animation.
    ubyte   flags;                          // General flags (see above)
    ubyte   iObject;                        // fixed up at load time so we know what object index we have when recursively processing
    uword   nameCRC;                        // 16-bit CRC of name (!!!!no room for 32 right now - make room next version)
    sdword  nVertices;                      // Number of vertices in vertex list for this object.
    sdword  nFaceNormals;                   // Number of face normals for this object.
    sdword  nVertexNormals;                 // Number of vertex normals for this object.
    sdword  nPolygons;                      // Number of polygons in this object.
    vertexentry *pVertexList;               // Offset to the vertex list in this object.
    normalentry *pNormalList;               // Offset to the normal list in this object.
    polyentry   *pPolygonList;              // Offset to the polygon list in this object.
    struct polygonobject *pMother;          // link to parent object
    struct polygonobject *pDaughter;        // link to child object
    struct polygonobject *pSister;          // link to sibling object
    hmatrix localMatrix;
}
polygonobject;

//structure of mesh file header when loaded
typedef struct
{
    char *name;
    udword localSize;                       //size required for each instance of this mesh
    sdword nPublicMaterials;                //number of shared materials
    sdword nLocalMaterials;                 //number of local materials
    sdword nPolygonObjects;                 //number of polygon objects in mesh file
    struct tagmaterialentry *localMaterial; //list of local materials
    struct tagmaterialentry *publicMaterial;//list of public materials
#if MESH_RETAIN_FILENAMES
    char *fileName;                         //for debugging
#endif
    polygonobject object[1];                //array of polygon object files
}
meshdata;

//hierarchy binding information
struct shipbindings;
typedef bool (*mhbindingfunction)(udword flags, hmatrix *startMatrix, hmatrix *matrix, void *data, sdword ID);
typedef void (*mhhelperfunction)(meshdata *mesh, struct shipbindings *bindings, sdword currentLOD);
typedef struct
{
    mhbindingfunction function;             //function to call to get a new matrix
    void *userData;                         //pointer data to pass
    sdword userID;                          //integer data to pass
    udword flags;                           //flags on how this binding operates (see above)
    polygonobject *object;                  //object of this binding
}
mhbinding;

typedef struct
{
    mhbindingfunction function;             //function to call to get a new matrix
    void *userData;                         //pointer data to pass
    sdword userID;                          //integer data to pass
    udword flags;                           //flags on how this binding operates (see above)
    polygonobject *object;                  //object of this binding
    hmatrix matrix;
}
mhlocalbinding;

//hierarchy information to be found in ship structure
typedef struct shipbindings
{
    sdword frequencyCounter;                //counts how often to update the hierarchy
#if MESH_PRE_CALLBACK
    mhhelperfunction preCallback;           //called before the hierarchy is rendered
#endif
    mhhelperfunction postCallback;          //called after the hierarchy is rendered
    //this is a list of per-lod binding lists.  Length is same as number of LOD's
    mhlocalbinding *localBinding[1];        //bindings custom to this ship.
}
shipbindings;

//mesh hierarchy walking callback
typedef bool (*meshcallback)(meshdata *mesh, polygonobject *object, sdword iObject);

/*=============================================================================
    Data:
=============================================================================*/
#if MESH_MATERIAL_STATS
extern sdword meshTotalMaterials;
extern sdword meshMaterialChanges;
extern char meshMaterialStatsString[100];
extern bool usingShader;
#endif //MESH_MATERIAL_STATS

#if MESH_MORPH_DEBUG
extern bool meshMorphDebug;
#endif

/*=============================================================================
    Macros:
=============================================================================*/
#define shipBindingsLength(n)   (sizeof(shipbindings) + sizeof(mhlocalbinding *) * (n - 1))

/*=============================================================================
    Functions:
=============================================================================*/
//startup the mesh module
void meshStartup(void);
//shutdown the mesh module
void meshShutdown(void);

//load in a mesh file
meshdata *meshLoad(char *fileName);
void meshFree(meshdata *mesh);
void meshRecolorize(meshdata *mesh);
void meshFixupUV(meshdata* mesh);
bool meshPagedVersionExists(char* fileName);

//render a specific mesh
sdword meshRender(meshdata *mesh, sdword iColorScheme);
void meshMorphedObjectRender(
    polygonobject* object1, polygonobject* object2, polyentry *uvPolys,
    materialentry* materials, real32 frac, sdword iColorScheme);
void meshMorphedObjectRenderTex(polygonobject* object1, polygonobject* object2,
                                polyentry *uvPolys, materialentry* material,
                                real32 frac, sdword iColorScheme);
void meshRenderShipHierarchy(shipbindings *bindings, sdword currentLOD, meshdata *mesh, sdword iColorScheme);
void meshObjectRenderRGL(polygonobject* object, materialentry* materials, sdword iColorScheme);
void meshObjectRender(polygonobject *object, materialentry *materials, sdword iColorScheme);
void meshObjectRenderTex(polygonobject *object, materialentry *material);

//make a material current
void meshCurrentMaterialDefault(materialentry *material, sdword iColorScheme);

//binding list stuff
mhbinding *meshBindingListCreate(meshdata *mesh);
void meshBindingListDelete(mhbinding *list);
mhbinding *meshBindingFindByName(mhbinding *startBinding, mhbinding *list, meshdata *data, char *name);
sdword meshBindingIndexFindByName(mhbinding *list, meshdata *mesh, char *name);
bool meshFindHierarchyMatrixByUserData(hmatrix *dest, hmatrix *destNC, mhbinding *bindings, void *userData);
void meshHierarchyWalk(meshdata *mesh, meshcallback preMesh, meshcallback postCallback);

void meshSetFade(real32 fade);
void meshSetSpecular(sdword mode, ubyte red, ubyte green, ubyte blue, ubyte alpha); //mode -1 == off

#endif //___MESH_H
