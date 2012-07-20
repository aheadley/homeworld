/*=============================================================================
    Name    : mesh.c
    Purpose : Functions to load, manipulate and render polygon meshes.

    Created 6/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIM32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "glinc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "Types.h"
#include "Debug.h"
#include "File.h"
#include "Memory.h"
#include "Vector.h"
#include "texreg.h"
#include "render.h"
#include "utility.h"
#include "Teams.h"
#include "ObjTypes.h"
#include "SpaceObj.h"
#include "Universe.h"
#include "Mesh.h"
#include "Shader.h"
#include "glcaps.h"
#include "AutoLOD.h"
#include "CRC32.h"
#include "Hash.h"
#include "prim3d.h"
#include "Transformer.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#define GLVERTEX(x) glVertex4fv((GLfloat*)(x))
#define VERTEXLIST clipVertexList

#define meshColour(NORMAL) shColourSet0((vector*)NORMAL);

#define _MESH_OBJECT_RENDER(O, M, C)\
    if (g_Output) meshObjectOutput(O, M, C);\
    else if (g_Points) meshObjectPoints(O, M, C);\
    else meshObjectRender(O, M, C);

#define MESH_OBJECT_RENDER(O, M, C)\
    if (g_SpecHack)\
    {\
        meshSpecObjectRender(O, M, C);\
    }\
    else if (usingShader && rndLightingEnabled && !g_WireframeHack)\
    {\
        if (RGL)\
        {\
            if (glIsEnabled(GL_CLIP_PLANE0)) meshObjectRenderLitRGL(O, M, C);\
            else meshObjectRenderLitRGLvx(O, M, C);\
        }\
        else\
        {\
            meshObjectRenderLit(O, M, C);\
        }\
    }\
    else\
    {\
        meshObjectRender(O, M, C);\
    }

/*=============================================================================
    Data:
=============================================================================*/

sdword scaledWidth, scaledHeight;
sdword visiblePoly = -1;
sdword visibleDirection = 0;
sdword visibleWhich = 0;
sdword visibleSegment = 0;
real32 visibleUV[3][2];
bool g_SpecificPoly = FALSE;

void meshSpecObjectRender(polygonobject *object, materialentry *materials, sdword iColorScheme);
void meshObjectRenderLitRGLvx(polygonobject *object, materialentry *materials, sdword iColorScheme);

bool gSelfIllum;

bool usingShader = TRUE;

bool bFade = FALSE;
real32 meshFadeAlpha = 1.0f;

bool8 g_NoMatSwitch = FALSE;
bool8 g_ReplaceHack = FALSE;
bool8 g_WireframeHack = FALSE;
bool8 g_SpecHack = FALSE;
bool8 g_HiddenRemoval = TRUE;
bool  g_Points = FALSE;
bool  g_Output = FALSE;

static sdword specIndex;
static ubyte specColour[4];

#if MESH_LOAD_DUMMY_TEXTURE
texhandle meshTestHandle = TEX_Invalid;
#endif

//current mesh poly mode
sdword meshPolyMode = MPM_Flat;

#if MESH_MATERIAL_STATS
sdword meshTotalMaterials = 0;
sdword meshMaterialChanges = 0;
char meshMaterialStatsString[100] = "";
#endif //MESH_MATERIAL_STATS

#if MESH_HACK_TEAM_COLORS
trcolorinfo meshRace1Colors = {colRGB(129, 152, 97), colRGB(139, 152, 30)};
trcolorinfo meshRace2Colors = {colRGB(17, 17, 145), colRGB(209, 28, 139)};
#endif

//funtion-pointer for changing the current material
void (*meshCurrentMaterial)(materialentry *material, sdword iColorScheme) = meshCurrentMaterialDefault;

#if MESH_SURFACE_NAME_DEBUG
bool gTestTexture = FALSE;
#endif

#if MESH_MORPH_DEBUG
bool meshMorphDebug = FALSE;
struct
{
    color from, to;
}
meshMorphLineColors[] =
{
    {colRGB(255, 0, 0),     colRGB(128, 0, 0)},
    {colRGB(0, 255, 0),     colRGB(0, 128, 0)},
    {colRGB(0, 0, 255),     colRGB(0, 0, 128)},
    {colRGB(255, 255, 0),   colRGB(128, 128, 0)},
    {colRGB(0, 255, 255),   colRGB(0, 128, 128)},
    {colRGB(255, 0, 255),   colRGB(128, 0, 128)},
    {colBlack, colBlack}
};
#endif;

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : meshStartup
    Description : startup the mesh module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshStartup()
{
    usingShader = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : meshShutdown
    Description : shutdown the mesh module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshShutdown()
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : meshSetFade
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshSetFade(real32 fade)
{
    if (!usingShader)
    {
        bFade = FALSE;
    }
    else
    {
        meshFadeAlpha = 1.0f - fade;
        if (meshFadeAlpha == 1.0f)
        {
            bFade = FALSE;
        }
        else
        {
            bFade = TRUE;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshSetSpecular
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshSetSpecular(sdword mode, ubyte red, ubyte green, ubyte blue, ubyte alpha)
{
    specIndex = mode;
    if (mode == -1)
    {
        g_SpecHack = 0;
    }
    else
    {
        specColour[0] = red;
        specColour[1] = green;
        specColour[2] = blue;
        specColour[3] = alpha;
        g_SpecHack = 1;
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshNameToRace
    Description : Convert a mesh file name to a race
    Inputs      : fileName - name of mesh file
    Outputs     :
    Return      : pointer to either a ship or a missile static info for that
                    race and type or NULL if unknown race
----------------------------------------------------------------------------*/
StaticInfo *meshNameToStaticInfo(char *fileName)
{
    char raceString[16];
    char typeString[64];
    sdword length;
    StaticInfo *info;
    ShipRace race;
    ShipType type;

#ifdef _WIN32
    dbgAssert(strchr(fileName, '\\'));
    length = strchr(fileName, '\\') - fileName;
#else
    dbgAssert(strpbrk(fileName, "\\/"));
    length = strpbrk(fileName, "\\/") - fileName;
#endif
    memcpy(raceString, fileName, length);
    raceString[length] = 0;                                 //make a race name
    fileName += length + 1;
//    dbgAssert(strchr(fileName, '\\'));                      //make a type name
#ifdef _WIN32
    if (strchr(fileName, '\\') == NULL)
#else
    if (strpbrk(fileName, "\\/") == NULL)
#endif
    {
        return(NULL);
    }
#ifdef _WIN32
    length = strchr(fileName, '\\') - fileName;
#else
    length = strpbrk(fileName, "\\/") - fileName;
#endif
    memcpy(typeString, fileName, length);
    typeString[length] = 0;
    if (strcasecmp(raceString, "Derelicts") == 0)
    {                                                       //if it's a derelict
        type = StrToDerelictType(typeString);
        dbgAssert(type != 0xffffffff);
        return((StaticInfo *)&derelictStaticInfos[type]);
    }
    //else it's a race-specific texture, process normally
    race = StrToShipRace(raceString);                       //get the race
    if (race == 0xffffffff)
    {
        race = NUM_RACES;
        return(NULL);
    }
    type = StrToShipType(typeString);
    if (type == 0xffffffff)
    {
        if (strcasecmp(typeString, "MISSILE") == 0)
        {
            info = (StaticInfo *)&missileStaticInfos[race];
        }
        else if (strcasecmp(typeString, "MINE") == 0)
        {
            info = (StaticInfo *)&mineStaticInfos[race];
        }
        else
        {
            dbgWarningf(DBG_Loc, "Don't know how to team colorize '%s' for '%s'", typeString, raceString);
            return(NULL);
        }
    }
    else
    {
        info = (StaticInfo *)GetShipStaticInfo(type, race);
    }
    return(info);
}

/*-----------------------------------------------------------------------------
    Name        : meshTextureRegisterAllPlayers
    Description : Register the named texture for each player who is of a certain race.
    Inputs      : fullName - name of texture.
                  race - race of players to compare to.  If NUM_RACES, there is no
                    race and only one handle is allocated (cookie wast can you say?)
                  meshReference - mesh reference to pass to the texture registry
    Outputs     : allocates a list of texture handles TE_NumberPlayers in length.
    Return      : pointer to this new list of handles cast to a handle.
----------------------------------------------------------------------------*/
trhandle meshTextureRegisterAllPlayers(char *fullName, void *meshReference)
{
    trhandle handleList[MAX_MULTIPLAYER_PLAYERS], *allocedList;
    sdword index, nRegistered = 0;
    StaticInfo *info;

    memClearDword(handleList, TR_Invalid, TE_NumberPlayers);//allocate and clear handle list
    info = meshNameToStaticInfo(fullName);
    if (info == NULL)
    {
        handleList[0] = trTextureRegister(fullName, NULL, meshReference);
        nRegistered = 1;
    }
    else
    {
        for (index = 0; index < MAX_MULTIPLAYER_PLAYERS; index++)
        {
            dbgAssert(index < TE_NumberPlayers);
            handleList[index] = TR_Invalid;

            if (((ShipStaticInfo *)info)->teamColor[index])
            {                                               //if this player can build this ship
                handleList[index] = trTextureRegister(fullName, //register in this players's colors
                        &teColorSchemes[index].textureColor, meshReference);
                nRegistered++;
            }
        }
    }
    if (nRegistered > 0)
    {                                                       //if some were registered
        allocedList = memAlloc(TE_NumberPlayers * sizeof(trhandle), "trhandle*TE_NumberPlayers", NonVolatile);
        memcpy(allocedList, handleList, TE_NumberPlayers * sizeof(trhandle));
        return((trhandle)(allocedList));                    //return memAlloced copy
    }
    return(TR_Invalid);
}

/*-----------------------------------------------------------------------------
    Name        : meshTextureNameToPath
    Description : Create a path for a texture from a texture name as it comes
                    from the .geo file.
    Inputs      : mesh - name of mesh file
                  tex - name of texture
    Outputs     : out - path to texture
    Return      : void
----------------------------------------------------------------------------*/
void meshTextureNameToPath(char *out, char *mesh, char *tex)
{
    char *string;
    unsigned int i;
    //make path of mesh path and \\texture\\<shipNameNoExtension\\<textureFileName>
    strcpy(out, mesh);

    string = out;
#ifdef _WIN32
    while (strchr(string, '\\'))
    {                                               //find last backslash
        string = strchr(string, '\\') + 1;
    }
#else
    while (strpbrk(string, "\\/"))
    {                                               /* find last backslash */
        string = strpbrk(string, "\\/") + 1;
    }
#endif
    dbgAssert(string != out);
    dbgAssert(*string);
    *string = 0;                                    //NULL terminate and append texture name

    strcat(out, tex);
    for (i = 0; (out[i] = toupper(out[i])); i++) { }
}

bool meshStringContainsPeriod(char* s)
{
    sdword i;

    for (i = 0; i < strlen(s); i++)
    {
        if (s[i] == '.')
        {
            return TRUE;
        }
    }

    return FALSE;
}

void meshPagedName(char* outFileName, char* inFileName)
{
    char  fileName[1024];
    char* token;

    outFileName[0] = '\0';
    strcpy(fileName, inFileName);

    token = strtok(fileName, "\\/");
    while (token != NULL)
    {
        if (meshStringContainsPeriod(token))
        {
            token[strlen(token) - 3] = 'p';
            strcat(outFileName, token);
        }
        else
        {
            strcat(outFileName, token);
#ifdef _WIN32
            strcat(outFileName, "\\");
#else
            strcat(outFileName, "/");
#endif
        }

        token = strtok(NULL, "\\/");
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshPagedVersionExists
    Description : See if the paged version (.peo) of a file exists.
    Inputs      : fileName - file to check existence of paged version
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool meshPagedVersionExists(char* fileName)
{
    char pagedName[1024];

    meshPagedName(pagedName, fileName);
    return fileExists(pagedName, 0);
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectFixupPacoUV
    Description : fixup UV seaming on a particular polygon object
    Inputs      : object - the polygonobject to fixup or un-fixup
                  on - to fixup or not fixup
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshObjectFixupPacoUV(polygonobject* object, sdword on)
{
    polyentry* polygon;
    sdword iPoly;
    sdword width, height;
    real32 s, t;

    for (iPoly = 0, polygon = object->pPolygonList; iPoly < object->nPolygons; iPoly++, polygon++)
    {
        if (polygon->reserved[0] > 1)
        {
            //will be 0 or 1 if adjust data exists
            continue;
        }

        //isolate width, height nibbles
        width  = (polygon->reserved[1] & 0xf0) >> 4;
        height =  polygon->reserved[1] & 0x0f;
        //convert exp2 to number
        width  = 1 << width;
        height = 1 << height;
        //define correction factors
        s = 0.6f / (real32)width;
        t = 0.6f / (real32)height;

        if (on)
        {
            //want to fixup
            if (bitTest(polygon->flags, PAC_FIXED))
            {
                //already fixed up
                continue;
            }
            else
            {
                //not fixed up yet
                bitSet(polygon->flags, PAC_FIXED);
            }
        }
        else
        {
            //want to unfixup
            if (bitTest(polygon->flags, PAC_FIXED))
            {
                //currently fixed up
                bitClear(polygon->flags, PAC_FIXED);
                s = -s;
                t = -t;
            }
            else
            {
                //already unfixed
                continue;
            }
        }

        //convert uv coords that need converting
        //0
        if (bitTest(polygon->flags, PAC_S0_0))
        {
            polygon->s0 += s;
        }
        if (bitTest(polygon->flags, PAC_S0_1))
        {
            polygon->s0 -= s;
        }
        if (bitTest(polygon->flags, PAC_T0_0))
        {
            polygon->t0 += t;
        }
        if (bitTest(polygon->flags, PAC_T0_1))
        {
            polygon->t0 -= t;
        }
        //1
        if (bitTest(polygon->flags, PAC_S1_0))
        {
            polygon->s1 += s;
        }
        if (bitTest(polygon->flags, PAC_S1_1))
        {
            polygon->s1 -= s;
        }
        if (bitTest(polygon->flags, PAC_T1_0))
        {
            polygon->t1 += t;
        }
        if (bitTest(polygon->flags, PAC_T1_1))
        {
            polygon->t1 -= t;
        }
        //2
        if (bitTest(polygon->flags, PAC_S2_0))
        {
            polygon->s2 += s;
        }
        if (bitTest(polygon->flags, PAC_S2_1))
        {
            polygon->s2 -= s;
        }
        if (bitTest(polygon->flags, PAC_T2_0))
        {
            polygon->t2 += t;
        }
        if (bitTest(polygon->flags, PAC_T2_1))
        {
            polygon->t2 -= t;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshFixupPacoUV
    Description : apply UV munging to fixup texture seaming to a particular mesh
    Inputs      : mesh - the mesh
                  on - to fixup or not fixup
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshFixupPacoUV(meshdata* mesh, sdword on)
{
    sdword index;
    polygonobject* object;

    if (mesh->localSize == 0xFFFFFFFF)
    {
        //old-style mesh, no fixup info
        return;
    }

    if (on)
    {
        if (mesh->localSize > 0)
        {
            //already fixed up
            return;
        }
        else
        {
            mesh->localSize = 1;
        }
    }
    else
    {
        if (mesh->localSize == 0)
        {
            //already un-fixed
            return;
        }
        else
        {
            mesh->localSize = 0;
        }
    }

    for (index = 0, object = &mesh->object[0]; index < mesh->nPolygonObjects; index++, object++)
    {
        meshObjectFixupPacoUV(object, on);
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshLoad
    Description : Loads in a mesh file.
    Inputs      : fileName - name of file to load
    Outputs     : Allocates and loads data returned in meshdata structure.
    Return      : Pointer to allocated and fixed-up meshdata structure.
----------------------------------------------------------------------------*/
meshdata *meshLoad(char *inFileName)
{
    GeoFileHeader header;
    filehandle file;
    meshdata *mesh;
    sdword index;
    udword offset;
    polygonobject *object;
    char fullName[FL_Path];
    sdword fileLength;
    trhandle handle;

#ifdef ENDIAN_BIG
    sdword i;
	udword *vertsDone  = NULL;
	udword *normsDone  = NULL;
	udword *polysDone  = NULL;
	int found          = -1;
	int vertsDoneCount = 0;
	int normsDoneCount = 0;
	int polysDoneCount = 0;
#endif

    bool allowPacking;
    bool newFormat;

    char  pagedName[1024];
    char* fileName;

    allowPacking = mainAllowPacking;

    if (allowPacking && meshPagedVersionExists(inFileName))
    {
        meshPagedName(pagedName, inFileName);
        fileName = pagedName;
    }
    else
    {
        fileName = inFileName;
    }

    fileLength = fileSizeGet(fileName, 0);

    file = fileOpen(fileName, 0);                           //open file
    fileBlockRead(file, &header, sizeof(GeoFileHeader));    //read in header

#ifdef ENDIAN_BIG
	header.version          = LittleLong( header.version );
	header.pName            = ( char *)LittleLong( ( udword )header.pName );
	header.fileSize         = LittleLong( header.fileSize );
	header.localSize        = LittleLong( header.localSize );
	header.nPublicMaterials = LittleLong( header.nPublicMaterials );
	header.nLocalMaterials  = LittleLong( header.nLocalMaterials );
	header.oPublicMaterial  = LittleLong( header.oPublicMaterial );
	header.oLocalMaterial   = LittleLong( header.oLocalMaterial );
	header.nPolygonObjects  = LittleLong( header.nPolygonObjects );

	vertsDone = ( udword *)malloc( header.nPolygonObjects * sizeof( udword ) );
	normsDone = ( udword *)malloc( header.nPolygonObjects * sizeof( udword ) );
	polysDone = ( udword *)malloc( header.nPolygonObjects * sizeof( udword ) );

	memset( vertsDone, 0, header.nPolygonObjects * sizeof( udword ) );
	memset( normsDone, 0, header.nPolygonObjects * sizeof( udword ) );
	memset( polysDone, 0, header.nPolygonObjects * sizeof( udword ) );
#endif

#if MESH_ERROR_CHECKING
    if (strncmp(header.identifier, MESH_FileID, MESH_FileIDLength))//verify ident. string
    {
        if (strncmp(header.identifier, MESH_FileID2, MESH_FileIDLength))
        {
            dbgFatalf(DBG_Loc, "meshLoad: Invalid ident string in %s.  Expected %s, found %s", fileName, MESH_FileID, header.identifier);
        }
    }
    if (header.version != MESH_Version)                    //verify correct version
    {
        dbgFatalf(DBG_Loc, "meshLoad: Invalid version # %s.  Expected 0x%x, found 0x%x", fileName, MESH_Version, header.version);
    }
#endif

    if (strncmp(header.identifier, MESH_FileID2, MESH_FileIDLength) == 0)
    {
        newFormat = TRUE;
    }
    else
    {
        newFormat = FALSE;
    }

    mesh = memAlloc(fileLength - sizeof(header)/*header.fileSize*/ + sizeof(meshdata) -   //allocate the memory for file
                    sizeof(polygonobject), fileName, NonVolatile);
    memNameSetLong((memcookie*)((ubyte *)mesh - sizeof(memcookie)), fileName);
    //now that it is all loaded in, all pointers need to be fixed up
    offset = (udword)mesh - sizeof(GeoFileHeader) +
        sizeof(meshdata) - sizeof(polygonobject);           //!!! add size of materials when they're in???
//    mesh->localSize = header.localSize;
    if (newFormat)
    {
        mesh->localSize = 0;
    }
    else
    {
        mesh->localSize = 0xFFFFFFFF;
    }
    mesh->nPublicMaterials = header.nPublicMaterials;       //set other header members
    mesh->nLocalMaterials = header.nLocalMaterials;
    mesh->publicMaterial = (materialentry *)(header.oPublicMaterial + offset);
    mesh->localMaterial = (materialentry *)(header.oLocalMaterial + offset);
    mesh->nPolygonObjects = header.nPolygonObjects;
                                                            //read remainder of the file
#if MESH_RETAIN_FILENAMES
    mesh->fileName = memStringDupe(fileName);
#endif
    fileBlockRead(file, &mesh->object[0], fileLength - sizeof(header)/*header.fileSize*/);
    fileClose(file);                                        //close the file

    mesh->name = header.pName + offset;                     //set name pointer
    //fix up the polgon objects
    for (index = 0; index < mesh->nPolygonObjects; index++)
    {
        object = &mesh->object[index];                      //get pointer to poly object

#ifdef ENDIAN_BIG
		if( !object ) continue;

		object->pName          = ( char *)LittleLong( ( udword )object->pName );
		object->nameCRC        = LittleShort( object->nameCRC );
		object->nVertices      = LittleLong( object->nVertices );
		object->nFaceNormals   = LittleLong( object->nFaceNormals );
		object->nVertexNormals = LittleLong( object->nVertexNormals );
		object->nPolygons      = LittleLong( object->nPolygons );
		object->pVertexList    = ( vertexentry *)LittleLong( ( udword )object->pVertexList );
		object->pNormalList    = ( normalentry *)LittleLong( ( udword )object->pNormalList );
		object->pPolygonList   = ( polyentry *)LittleLong( ( udword )object->pPolygonList );
		object->pMother        = ( polygonobject *)LittleLong( ( udword )object->pMother );
		object->pDaughter      = ( polygonobject *)LittleLong( ( udword )object->pDaughter );
		object->pSister        = ( polygonobject *)LittleLong( ( udword )object->pSister );

        // anonymous block so I can declare mptr with limited scope and not have
        // a plain C compiler complain
		{
            real32 *mptr = ( real32 *)&object->localMatrix;
		    for( i=0; i<16; i++ )
            {
			    mptr[i] = LittleFloat( mptr[i] );
            }
        }
#endif

        //object->pName += offset;                          //!!! is there a name
        (ubyte *)object->pVertexList += offset;             //fixup vertex list pointer
        (ubyte *)object->pNormalList += offset;             //fixup vertex list pointer
        (ubyte *)object->pPolygonList += offset;            //fixup vertex list pointer
        if (object->pMother != NULL)
        {
            (ubyte *)object->pMother += offset;
        }
        if (object->pDaughter != NULL)
        {
            (ubyte *)object->pDaughter += offset;
        }
        if (object->pSister != NULL)
        {
            (ubyte *)object->pSister += offset;
        }
        if (object->pName != NULL)
        {
            (ubyte *)object->pName += offset;               //fix up name
        }
        object->iObject = index;
        object->nameCRC = crc16Compute((ubyte*)object->pName, strlen(object->pName));

#ifdef ENDIAN_BIG
		found = -1;
		for( i=0; i<mesh->nPolygonObjects; i++ )
		{
			if( vertsDone[i] == object->pVertexList )
			{
				found = i;
				break;
			}
		}

		if( found == -1 )
		{
			for( i=0; i<object->nVertices; i++ )
			{
				object->pVertexList[i].x = LittleFloat( object->pVertexList[i].x );
				object->pVertexList[i].y = LittleFloat( object->pVertexList[i].y );
				object->pVertexList[i].z = LittleFloat( object->pVertexList[i].z );
				object->pVertexList[i].iVertexNormal = LittleLong( object->pVertexList[i].iVertexNormal );
    }

			vertsDone[vertsDoneCount++] = object->pVertexList;
		}

		found = -1;
		for( i=0; i<mesh->nPolygonObjects; i++ )
		{
			if( normsDone[i] == object->pNormalList )
			{
				found = i;
				break;
			}
		}

		if( found == -1 )
		{
			for( i=0; i<object->nFaceNormals + object->nVertexNormals; i++ )
			{
				object->pNormalList[i].x = LittleFloat( object->pNormalList[i].x );
				object->pNormalList[i].y = LittleFloat( object->pNormalList[i].y );
				object->pNormalList[i].z = LittleFloat( object->pNormalList[i].z );
			}

			normsDone[normsDoneCount++] = object->pNormalList;
		}

		found = -1;
		for( i=0; i<mesh->nPolygonObjects; i++ )
		{
			if( polysDone[i] == object->pPolygonList )
			{
				found = i;
				break;
			}
		}

		if( found == -1 )
		{
			for( i=0; i<object->nPolygons; i++ )
			{
				object->pPolygonList[i].iFaceNormal = LittleLong( object->pPolygonList[i].iFaceNormal );
				object->pPolygonList[i].iV0 = LittleShort( object->pPolygonList[i].iV0 );
				object->pPolygonList[i].iV1 = LittleShort( object->pPolygonList[i].iV1 );
				object->pPolygonList[i].iV2 = LittleShort( object->pPolygonList[i].iV2 );
				object->pPolygonList[i].iMaterial = LittleShort( object->pPolygonList[i].iMaterial );
				object->pPolygonList[i].s0 = LittleFloat( object->pPolygonList[i].s0 );
				object->pPolygonList[i].t0 = LittleFloat( object->pPolygonList[i].t0 );
				object->pPolygonList[i].s1 = LittleFloat( object->pPolygonList[i].s1 );
				object->pPolygonList[i].t1 = LittleFloat( object->pPolygonList[i].t1 );
				object->pPolygonList[i].s2 = LittleFloat( object->pPolygonList[i].s2 );
				object->pPolygonList[i].t2 = LittleFloat( object->pPolygonList[i].t2 );
				object->pPolygonList[i].flags = LittleShort( object->pPolygonList[i].flags );
			}

			polysDone[polysDoneCount++] = object->pPolygonList;
		}
    }

	free( vertsDone );
	free( normsDone );
	free( polysDone );

	for (index = 0; index < mesh->nLocalMaterials + mesh->nPublicMaterials; index++)
	{
		mesh->localMaterial[index].pName = ( char *)LittleLong( ( udword )mesh->localMaterial[index].pName );
//		mesh->localMaterial[index].ambient = LittleLong( mesh->localMaterial[index].ambient );
//		mesh->localMaterial[index].diffuse = LittleLong( mesh->localMaterial[index].diffuse );
//		mesh->localMaterial[index].specular = LittleLong( mesh->localMaterial[index].specular );
		mesh->localMaterial[index].kAlpha = LittleFloat( mesh->localMaterial[index].kAlpha );
		mesh->localMaterial[index].texture = LittleLong( mesh->localMaterial[index].texture );
		mesh->localMaterial[index].flags = LittleShort( mesh->localMaterial[index].flags );
		mesh->localMaterial[index].textureNameSave = ( char *)LittleLong( ( udword )mesh->localMaterial[index].textureNameSave );
#endif // ENDIAN_BIG

	} // ENDIAN_BIG:    end of for(index < mesh->nLocalMaterials...) loop
      // everyone else: end of for(index < mesh->nPolygonObjects)    loop

    if (allowPacking && mainOnlyPacking && (fileName != pagedName))
    {
        for (index = 0; index < mesh->nLocalMaterials + mesh->nPublicMaterials; index++)
        {
            if (mesh->localMaterial[index].pName != NULL)
            {
                mesh->localMaterial[index].pName += offset;
            }
            mesh->localMaterial[index].bTexturesRegistered = FALSE;
            mesh->localMaterial[index].texture = 0;
        }

        return(mesh);
    }

    for (index = 0; index < mesh->nLocalMaterials + mesh->nPublicMaterials; index++)
    {                                                       //for all materials
        if (mesh->localMaterial[index].pName != NULL)
        {
            mesh->localMaterial[index].pName += offset;
        }
        mesh->localMaterial[index].bTexturesRegistered = FALSE;
        if (mesh->localMaterial[index].texture != 0)
        {                                                   //if there is a texture
            (ubyte *)mesh->localMaterial[index].texture += offset;//fix up texture name pointer
            mesh->localMaterial[index].textureNameSave = (char *)mesh->localMaterial[index].texture;
            meshTextureNameToPath(fullName, fileName, (char *)mesh->localMaterial[index].texture);
            handle = meshTextureRegisterAllPlayers(fullName, mesh);//register the texture
            if (handle != TR_Invalid)
            {                                               //if anything was registered
                mesh->localMaterial[index].texture = handle;
                mesh->localMaterial[index].bTexturesRegistered = TRUE;
            }
        }
/*
        else
        {
            mesh->localMaterial[index].texture = TR_Invalid;
        }
*/
    }

    return(mesh);
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectFixupUV
    Description : nudges UV coordinates inwards to eliminate filtered oversampling errors
    Inputs      : object - polygonobject whose UVs are to be modified
                  materials - list of materials on the object
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshObjectFixupUV(polygonobject* object, materialentry* materials)
{
    sdword iPoly;
    polyentry* polygon;
    sdword currentMaterial;
    materialentry* material;
    trhandle handle;
    texreg* reg;
    real32 zero, one;

    for (iPoly = 0, polygon = object->pPolygonList; iPoly < object->nPolygons; iPoly++, polygon++)
    {
        currentMaterial = polygon->iMaterial;
        material = &materials[currentMaterial];

        if (material->bTexturesRegistered && (material->texture != 0))
        {
            handle = ((trhandle*)material->texture)[0/*iColorScheme*/];
//!!!            dbgAssert(handle != TR_InvalidHandle);
            if (handle == TR_InvalidHandle)
            {
                continue;
            }
            reg = trStructureGet(handle);

            zero = 0.51f / reg->scaledWidth;
            one = 1.0f - zero;

            if (polygon->s0 == 0.0f) polygon->s0 = zero;
            if (polygon->s0 == 1.0f) polygon->s0 = one;
            if (polygon->t0 == 0.0f) polygon->t0 = zero;
            if (polygon->t0 == 1.0f) polygon->t0 = one;

            if (polygon->s1 == 0.0f) polygon->s1 = zero;
            if (polygon->s1 == 1.0f) polygon->s1 = one;
            if (polygon->t1 == 0.0f) polygon->t1 = zero;
            if (polygon->t1 == 1.0f) polygon->t1 = one;

            if (polygon->s2 == 0.0f) polygon->s2 = zero;
            if (polygon->s2 == 1.0f) polygon->s2 = one;
            if (polygon->t2 == 0.0f) polygon->t2 = zero;
            if (polygon->t2 == 1.0f) polygon->t2 = one;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshFixupUV
    Description : nudges UV coordinates inwards to eliminate filtered oversampling errors
    Inputs      : mesh - the mesh to fixup
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshFixupUV(meshdata* mesh)
{
    sdword index;
    polygonobject* object;

    for (index = 0, object = &mesh->object[0]; index < mesh->nPolygonObjects; index++, object++)
    {
        meshObjectFixupUV(object, mesh->localMaterial);
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshRecolorize
    Description : Recolorize all the textures of a mesh based upon new colors
                    that may have been picked.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshRecolorize(meshdata *mesh)
{
    sdword index, j, k;
    trhandle *handles, validHandle;
    StaticInfo *info;
    char fullName[FL_Path];
    trhandle handle;

    info = meshNameToStaticInfo(mesh->fileName);
    if (info == NULL)
    {
        return;
    }
    for (index = 0; index < mesh->nLocalMaterials + mesh->nPublicMaterials; index++)
    {                                                       //for all materials in mesh
        //3 states of this member:TR_Invalid, pointer to list of texture handles or 0 (no texture)
        if (mesh->localMaterial[index].texture != TR_Invalid && mesh->localMaterial[index].texture != 0)
        {                                                   //if textured material
            if (!mesh->localMaterial[index].bTexturesRegistered)
            {                                               //if textures were never registered properly
                meshTextureNameToPath(fullName, mesh->fileName, (char *)mesh->localMaterial[index].texture);
                handle = meshTextureRegisterAllPlayers(fullName, mesh);//register the texture
                if (handle != TR_Invalid)
                {                                               //if anything was registered
                    mesh->localMaterial[index].texture = handle;
                    mesh->localMaterial[index].bTexturesRegistered = TRUE;
                }
            }
            else
            {                                               //textures were previously registered properly
                handles = (trhandle *)mesh->localMaterial[index].texture;
                validHandle = TR_Invalid;
                //4 passes for all players
                //first pass: see if any texture colors are way out of whack
                for (j = 0; j < MAX_MULTIPLAYER_PLAYERS; j++)
                {
                    if (handles[j] != TR_Invalid)
                    {                                       //if this texture allocated
    				    for (k = 0; k < MAX_MULTIPLAYER_PLAYERS; k++)
    					{
                            if (((ShipStaticInfo *)info)->teamColor[k])
                            {                               //if this team color will be loaded
                                if (k != j && trColorsEqual(&teColorSchemes[k].textureColor, trIndex(handles[j])) == trPaletteIndex(handles[j]))
                                {                           //if this handle's color is in some other team's desired color
                				    for (k = 0; k < MAX_MULTIPLAYER_PLAYERS; k++)
                					{                       //delete all handles; this texture will have to be loaded from scratch
                                        if (handles[k] != TR_Invalid)
                                        {
                                            trRegisterRemoval(handles[k]);
                                            handles[k] = TR_Invalid;
                                        }
                                    }
                                    goto doneDeletingAllOldTextures;//break-break, skip next 2 passes
                                }
                            }
                        }
                    }
                }

                //second pass: find a valid handle
                for (j = 0; j < MAX_MULTIPLAYER_PLAYERS; j++)
                {                                           //find a valid handle
                    if (handles[j] != TR_Invalid)
                    {
                        validHandle = handles[j];
                        break;
                    }
                }

                //third pass: removals of unused color schemes
                for (j = 0; j < MAX_MULTIPLAYER_PLAYERS; j++)
                {                                           //for all players
                    if (!((ShipStaticInfo *)info)->teamColor[j])
                    {                                       //if this player does not need this mesh
                        if (handles[j] != TR_Invalid)
                        {                                   //this texture no longer needed
                            trRegisterRemoval(handles[j]);  //delete this version of the texture (without reloading) ...
                            handles[j] = TR_Invalid;
                        }
                    }
                }
doneDeletingAllOldTextures:

                //fourth pass: additions and updates of color schemes
                for (j = 0; j < MAX_MULTIPLAYER_PLAYERS; j++)
                {                                           //for all players
                    if (((ShipStaticInfo *)info)->teamColor[j])
                    {                                       //if this player needs this mesh
                        if (handles[j] != TR_Invalid)
                        {                                   //if texture was loaded properly before
                            trTextureColorsUpdate(handles[j],//update the colors
                                &teColorSchemes[j].textureColor);
                        }
                        else
                        {                                   //this texture was never loaded properly
                            //register the texture with a new color
                            if (validHandle == TR_Invalid)
                            {                               //if no valid handles (they've all be unregistered)
                                meshTextureNameToPath(fullName, mesh->fileName, mesh->localMaterial[index].textureNameSave);
                                validHandle = handles[j] = trTextureRegister(fullName, &teColorSchemes[j].textureColor, mesh);
                            }
                            else
                            {                               //else a valid handle aleready exists for this texture
                                handles[j] = trRegisterAddition(validHandle,
                                    &teColorSchemes[j].textureColor);
                            }
                        }
                    }
#if MESH_ERROR_CHECKING
                    else if (handles[j] != TR_Invalid)
                    {                                       //this texture no longer needed
                        dbgFatalf(DBG_Loc, "Improperly removed texture for player #%d", j);
                    }
#endif
                }
/*
#if MESH_ERROR_CHECKING
	            //make sure no duplicate handles
		        for (l = 0; l < MAX_MULTIPLAYER_PLAYERS; l++)
			    {
				    for (k = 0; k < MAX_MULTIPLAYER_PLAYERS; k++)
					{                                           //find a valid handle
						if (k != l && handles[l] != TR_Invalid && handles[l] != 0)
	                    {
		                    if (handles[l] == handles[k])
						    {
					            dbgFatalf(DBG_Loc, "handles[%d] == handles[%d] in iteration %d.", l, k, j);
				            }
			            }
		            }
	            }
#endif
*/
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshFree
    Description : Free up the specified mesh.
    Inputs      : mesh - pointer to mesh to be freed.
    Outputs     :
    Return      : void
    Note        : does not free up any instances of thie mesh or any textures
                    bound to it.
----------------------------------------------------------------------------*/
void meshFree(meshdata *mesh)
{
    sdword index, j;

    for (index = 0; index < mesh->nLocalMaterials + mesh->nPublicMaterials; index++)
    {                                                       //for all materials
//        if (mesh->localMaterial[index].texture != TR_Invalid)//if material has a texture
        if (mesh->localMaterial[index].bTexturesRegistered)
        {
            for (j = 0; j < TE_NumberPlayers; j++)
            {                                               //for each player
                if (((trhandle *)mesh->localMaterial[index].texture)[j] != TR_Invalid)
                {                                           //if this player was this race
                    trTextureUnregister(((trhandle *)mesh->localMaterial[index].texture)[j]);
                }                                           //unregister the texture
            }
            memFree((ubyte *)mesh->localMaterial[index].texture);    //free the texture handle list
        }
    }
#if MESH_RETAIN_FILENAMES
    memFree(mesh->fileName);
#endif
    memFree((void *)mesh);
}

/*-----------------------------------------------------------------------------
    Name        : meshCurrentMaterialDefault
    Description : Makes specified material current
    Inputs      : material - material to make current
                  iColorScheme - index of player associated with this mesh
                    (for texture coloring) or 0 for no player.
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void meshCurrentMaterialDefault(materialentry *material, sdword iColorScheme)
{
    GLfloat attribs[4];
    GLenum face;
#if MESH_TEAM_COLORS
    color teamAmbient, teamDiffuse;
#if MESH_SPECULAR
    color teamSpecular;
#endif
#endif
    if (bitTest(material->flags, MDF_2Sided))
    {                                                       //if 2-sided material
        face = GL_FRONT_AND_BACK;
        rndBackFaceCullEnable(FALSE);                       //disable culling
        if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);    //2-sided lightmodel
    }
    else
    {
        face = GL_FRONT;
        rndBackFaceCullEnable(TRUE);                        //enable culling
        if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);   //1-sided lightmodel
    }

    gSelfIllum = FALSE;
    if (g_ReplaceHack)
    {
        rndTextureEnvironment(RTE_Replace);
    }
    else if (bitTest(material->flags, MDF_SelfIllum))
    {
        gSelfIllum = TRUE;
        if (bFade && (RGLtype == GLtype))
        {
            rndTextureEnvironment(RTE_Modulate);
        }
        else
        {
            rndTextureEnvironment(RTE_Replace);
        }
    }
    else
    {
        rndTextureEnvironment(RTE_Modulate);
    }

#if MESH_TEAM_COLORS
    if ((material->flags & (MDF_BaseColor | MDF_StripeColor)) == 0)
    {                                                       //if not a team color polygon
#endif
        attribs[0] = (GLfloat)(colRed(material->ambient)) / 255.0f;
        attribs[1] = (GLfloat)(colGreen(material->ambient)) / 255.0f;
        attribs[2] = (GLfloat)(colBlue(material->ambient)) / 255.0f;
        attribs[3] = 1.0f;
        if (usingShader)
        {
            shSetMaterial(attribs, NULL); //shader
        }
        else
        {
            rndMaterialfv(face, GL_AMBIENT, attribs);
        }

        attribs[0] = (GLfloat)(colRed(material->diffuse)) / 255.0f;
        attribs[1] = (GLfloat)(colGreen(material->diffuse)) / 255.0f;
        attribs[2] = (GLfloat)(colBlue(material->diffuse)) / 255.0f;
        if (bFade)
        {
            attribs[3] = meshFadeAlpha;
        }
        else
        {
            attribs[3] = 1.0f;
        }
        if (usingShader)
        {
            shSetMaterial(NULL, attribs); //shader
        }
        else
        {
            rndMaterialfv(face, GL_DIFFUSE, attribs);
        }

#if MESH_SPECULAR
        attribs[0] = (GLfloat)(colRed(material->specular)) / 255.0f;
        attribs[1] = (GLfloat)(colGreen(material->specular)) / 255.0f;
        attribs[2] = (GLfloat)(colBlue(material->specular)) / 255.0f;
        glMaterialfv(face, GL_SPECULAR, attribs);
#endif
#if MESH_TEAM_COLORS
    }
    else
    {                                                       //it's a team color
        if (material->flags & MDF_BaseColor)
        {                                                   //it's a base color poly
            teamAmbient  = teColorSchemes[iColorScheme].ambient;
            teamDiffuse  = teColorSchemes[iColorScheme].diffuse;
#if MESH_SPECULAR
            teamSpecular = teColorSchemes[iColorScheme].specular;
#endif
        }
        else
        {                                                   //else it's a stripe color
            teamAmbient  = teColorSchemes[iColorScheme].stripeAmbient;
            teamDiffuse  = teColorSchemes[iColorScheme].stripeDiffuse;
#if MESH_SPECULAR
            teamSpecular = teColorSchemes[iColorScheme].stripeSpecular;
#endif
        }
        attribs[0] = (GLfloat)(colRed(teamAmbient)) / 255.0f;
        attribs[1] = (GLfloat)(colGreen(teamAmbient)) / 255.0f;
        attribs[2] = (GLfloat)(colBlue(teamAmbient)) / 255.0f;
        attribs[3] = 1.0f;
        if (usingShader)
        {
            shSetMaterial(attribs, NULL); //shader
        }
        else
        {
            rndMaterialfv(face, GL_AMBIENT, attribs);
        }

        attribs[0] = (GLfloat)(colRed(teamDiffuse)) / 255.0f;
        attribs[1] = (GLfloat)(colGreen(teamDiffuse)) / 255.0f;
        attribs[2] = (GLfloat)(colBlue(teamDiffuse)) / 255.0f;
        if (bFade)
        {
            attribs[3] = meshFadeAlpha;
        }
        else
        {
            attribs[3] = 1.0f;
        }
        if (usingShader)
        {
            shSetMaterial(NULL, attribs); //shader
        }
        else
        {
            rndMaterialfv(face, GL_DIFFUSE, attribs);
        }

#if MESH_SPECULAR
        attribs[0] = (GLfloat)(colRed(teamSpecular)) / 255.0f;
        attribs[1] = (GLfloat)(colGreen(teamSpecular)) / 255.0f;
        attribs[2] = (GLfloat)(colBlue(teamSpecular)) / 255.0f;
        glMaterialfv(face, GL_SPECULAR, attribs);
#endif
    }
#endif //MESH_TEAM_COLORS
//    if (material->texture != TR_Invalid && !g_WireframeHack)
    if (material->bTexturesRegistered && !g_WireframeHack)
    {                                                       //if a textured polygon
#if MESH_SURFACE_NAME_DEBUG
        if (material->pName != NULL && (!strcmp(material->pName, "bot1front")))
        {
            gTestTexture = TRUE;
        }
        else
        {
            gTestTexture = FALSE;
        }
#endif
        rndTextureEnable(TRUE);
        trMakeCurrent(((trhandle *)material->texture)[iColorScheme]);
        if (bitTest(material->flags, MPM_Smooth) && enableSmoothing == TRUE)
        {
            meshPolyMode = MPM_SmoothTexture;
            if (!g_SpecHack)
            {
                glShadeModel(GL_SMOOTH);
            }
        }
        else
        {
            meshPolyMode = MPM_Texture;
            glShadeModel(GL_FLAT);
        }
    }
    else
    {
        rndTextureEnable(FALSE);
        if (bitTest(material->flags, MPM_Smooth) && enableSmoothing == TRUE)
        {
            meshPolyMode = MPM_Smooth;
            if (!g_SpecHack)
            {
                glShadeModel(GL_SMOOTH);
            }
        }
        else
        {
            meshPolyMode = MPM_Flat;
            glShadeModel(GL_FLAT);
        }
    }

    if (usingShader)
    {
        shUpdateLighting(); //shader
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshCurrentMaterialTex
    Description : Makes specified material current using a single texture, not
                    the one in the material entries.
    Inputs      : material - material to make current
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void meshCurrentMaterialTex(materialentry *material, sdword iColorScheme)
{
    GLenum face;

    if (bitTest(material->flags, MDF_2Sided))
    {                                                       //if 2-sided material
        face = GL_FRONT_AND_BACK;
        rndBackFaceCullEnable(FALSE);                       //disable culling
        if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);    //2-sided lightmodel
    }
    else
    {
        face = GL_FRONT;
        rndBackFaceCullEnable(TRUE);                        //enable culling
        if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);   //1-sided lightmodel
    }
//    if (enableTextures == TRUE)
//    {
        if (bitTest(material->flags, MPM_Smooth) && enableSmoothing == TRUE)
        {
            meshPolyMode = MPM_SmoothTexture;
            if (!g_SpecHack)
            {
                glShadeModel(GL_SMOOTH);
            }
        }
        else
        {
            meshPolyMode = MPM_Texture;
            glShadeModel(GL_FLAT);
        }
#if 0
    }
    else
    {
        if (bitTest(material->flags, MPM_Smooth) && enableSmoothing == TRUE)
        {
            meshPolyMode = MPM_Smooth;
            if (!g_SpecHack)
            {
                glShadeModel(GL_SMOOTH);
            }
        }
        else
        {
            meshPolyMode = MPM_Flat;
            glShadeModel(GL_FLAT);
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : meshAlmostCurrentMaterial
    Description : Current material switch function that doesn't change any GL
                    interal stuff.
    Inputs      : material - the material to use
                  iColorScheme - ignored (for compatability)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshAlmostCurrentMaterial(materialentry* material, sdword iColorScheme)
{
//    if (material->texture != TR_Invalid)
    if (material->bTexturesRegistered)
    {
        if (bitTest(material->flags, MPM_Smooth) && enableSmoothing == TRUE)
        {
            meshPolyMode = MPM_SmoothTexture;
        }
        else
        {
            meshPolyMode = MPM_Texture;
        }
    }
    else
    {
        if (bitTest(material->flags, MPM_Smooth) && enableSmoothing == TRUE)
        {
            meshPolyMode = MPM_Smooth;
        }
        else
        {
            meshPolyMode = MPM_Flat;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshSpecColour
    Description : set specular colour given parameters
    Inputs      : ...
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshSpecColour(normalentry* normal, vertexentry* vertex, real32* m, real32* minv)
{
    vector vert;
    vector norm;
    ubyte  colour[4];

    norm.x = normal->x;
    norm.y = normal->y;
    norm.z = normal->z;
    vert.x = vertex->x;
    vert.y = vertex->y;
    vert.z = vertex->z;
    colour[0] = specColour[0];
    colour[1] = specColour[1];
    colour[2] = specColour[2];
    colour[3] = specColour[3];
    shSpecularColour(specIndex, 0, &vert, &norm, colour, m, minv);
    glColor4ub(colour[0], colour[1], colour[2], colour[3]);
}

void meshMorphedSpecColour(vector* normal, real32* m, real32* minv)
{
    ubyte  colour[4];

    colour[0] = specColour[0];
    colour[1] = specColour[1];
    colour[2] = specColour[2];
    colour[3] = specColour[3];
    shSpecularColour(specIndex, 0, NULL, normal, colour, m, minv);
    glColor4ub(colour[0], colour[1], colour[2], colour[3]);
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectRender
    Description : Render a single polygon object (part of a meshdata structure)
    Inputs      : object - object to render
                  materials - material list to use in rendering
                  iColorScheme - index of player associated with this mesh
                    (for texture coloring) or 0 for no player.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#if MESH_MATERIAL_STATS
    static sdword nMaterialChanges = 0;
    static sdword iMaterialMax = 0;
#endif //MESH_MATERIAL_STATS
void meshObjectOutput(polygonobject* object, materialentry* materials, sdword iColorScheme)
{
    sdword iPoly;
    polyentry* polygon;
    char *fileNameFull;
    FILE* out;

    fileNameFull = filePathPrepend("uv.txt", FF_UserSettingsPath);

    if (!fileMakeDestinationDirectory(fileNameFull))
        return;

    out = fopen(fileNameFull, "at");
    if (out == NULL)
    {
        return;
    }

    fprintf(out, "-----\n");
    fprintf(out, "%d\n", object->nPolygons);

    polygon = object->pPolygonList;                         //get first polygon list entry

    for (iPoly = 0; iPoly < object->nPolygons; iPoly++, polygon++)
    {
        fprintf(out, "%d [%0.20f %0.20f] [%0.20f %0.20f] [%0.20f %0.20f]\n",
                iPoly,
                polygon->s0, polygon->t0,
                polygon->s1, polygon->t1,
                polygon->s2, polygon->t2);
    }

    fclose(out);
}
void meshObjectPoints(polygonobject* object, materialentry* materials, sdword iColorScheme)
{
    sdword iPoly;
    vertexentry* vertexList;
    polyentry* polygon;
    bool lightOn, texOn;

    vertexList = object->pVertexList;
    polygon = object->pPolygonList;

    lightOn = rndLightingEnable(FALSE);
    texOn = rndTextureEnable(FALSE);

    glPointSize(4.0f);

    for (iPoly = 0; iPoly < object->nPolygons; iPoly++, polygon++)
    {
        if (iPoly == visiblePoly)
        {
            glBegin(GL_POINTS);
            glColor3ub(255,0,0);
            glVertex3fv((GLfloat*)&vertexList[polygon->iV0].x);
            glColor3ub(0,255,0);
            glVertex3fv((GLfloat*)&vertexList[polygon->iV1].x);
            glColor3ub(0,0,255);
            glVertex3fv((GLfloat*)&vertexList[polygon->iV2].x);
            glEnd();
        }
    }

    glPointSize(1.0f);

    rndLightingEnable(lightOn);
    rndTextureEnable(texOn);
}
void meshObjectRender(polygonobject *object, materialentry *materials, sdword iColorScheme)
{
    sdword iPoly;
    vertexentry *vertexList;
    polyentry *polygon;
    normalentry *normal, *normalList;
    sdword currentMaterial = -1;
    GLenum mode = GL_SMOOTH;
    sdword lightOn;
    bool enableBlend;

    glShadeModel(mode);

    enableBlend = bFade;

    if (g_WireframeHack)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        lightOn = rndLightingEnable(FALSE);
        glColor3ub(200,200,200);
        enableBlend = glCapFeatureExists(GL_LINE_SMOOTH);
        if (enableBlend)
        {
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            rndAdditiveBlends(FALSE);
        }
        if (!g_HiddenRemoval)
        {
            glDisable(GL_CULL_FACE);
        }
    }

    alodIncPolys(object->nPolygons);

    vertexList = object->pVertexList;                       //get base of vertex list
    normalList = object->pNormalList;                       //get base of normal list
    polygon = object->pPolygonList;                         //get first polygon list entry

    glBegin(GL_TRIANGLES);                                  //prepare to draw triangles

    for (iPoly = 0; iPoly < object->nPolygons; iPoly++)
    {
#if MESH_ANAL_CHECKING                                      //validate vertex indices
        dbgAssert(polygon->iV0 < object->nVertices);
        dbgAssert(polygon->iV1 < object->nVertices);
        dbgAssert(polygon->iV2 < object->nVertices);
#endif
        if (polygon->iMaterial != currentMaterial)
        {                                                   //if a new material
            glEnd();                                        //end current polygon run
            currentMaterial = polygon->iMaterial;           //remember current material
            meshCurrentMaterial(&materials[currentMaterial], iColorScheme);//set new material
            if (enableBlend)
            {
                glEnable(GL_BLEND);
            }
            glBegin(GL_TRIANGLES);                          //start new run
#if MESH_MATERIAL_STATS
            nMaterialChanges++;                             //record material stats
            iMaterialMax = max(currentMaterial, iMaterialMax);
#endif //MESH_MATERIAL_STATS
        }

        if (g_SpecificPoly)
        {
            if  (iPoly != visiblePoly)
            {
                polygon++;
                continue;
            }
            else
            {
                real32 s, t;

                visibleUV[0][0] = polygon->s0;
                visibleUV[0][1] = polygon->t0;
                visibleUV[1][0] = polygon->s1;
                visibleUV[1][1] = polygon->t1;
                visibleUV[2][0] = polygon->s2;
                visibleUV[2][1] = polygon->t2;

                s = 0.5f / 256.0f;//(real32)scaledWidth;
                t = 0.5f / 256.0f;//(real32)scaledHeight;
                s *= (real32)visibleDirection;
                t *= (real32)visibleDirection;

                if (visibleDirection != 0)
                {
                    switch (visibleSegment)
                    {
                    case 0:
                        if (visibleWhich == 0)
                        {
                            polygon->s0 += s;
                        }
                        else
                        {
                            polygon->t0 += t;
                        }
                        break;
                    case 1:
                        if (visibleWhich == 0)
                        {
                            polygon->s1 += s;
                        }
                        else
                        {
                            polygon->t1 += t;
                        }
                        break;
                    default:
                        if (visibleWhich == 0)
                        {
                            polygon->s2 += s;
                        }
                        else
                        {
                            polygon->t2 += t;
                        }
                    }
                    visibleDirection = 0;
                }
            }
        }

        switch (meshPolyMode)
        {
            case MPM_Flat:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];
                glNormal3f(normal->x, normal->y, normal->z);

                glVertex3fv((GLfloat*)&vertexList[polygon->iV0].x);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1].x);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2].x);

#if RND_POLY_STATS
                rndNumberPolys++;
#endif //RND_POLY_STATS
                break;
            case MPM_Texture:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];
                glNormal3f(normal->x, normal->y, normal->z);

                glTexCoord2f(polygon->s0, polygon->t0);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0].x);

                glTexCoord2f(polygon->s1, polygon->t1);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1].x);

                glTexCoord2f(polygon->s2, polygon->t2);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2].x);

#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
#endif //RND_POLY_STATS
                break;
            case MPM_Smooth:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                glNormal3f(normal->x, normal->y, normal->z);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0].x);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                glNormal3f(normal->x, normal->y, normal->z);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1].x);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                glNormal3f(normal->x, normal->y, normal->z);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2].x);

#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            case MPM_SmoothTexture:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                glNormal3f(normal->x, normal->y, normal->z);
                glTexCoord2f(polygon->s0, polygon->t0);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0].x);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                glNormal3f(normal->x, normal->y, normal->z);
                glTexCoord2f(polygon->s1, polygon->t1);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1].x);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                glNormal3f(normal->x, normal->y, normal->z);
                glTexCoord2f(polygon->s2, polygon->t2);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2].x);

#if MESH_SURFACE_NAME_DEBUG
                if (gTestTexture)
                {
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV0].x, vertexList[polygon->iV0].y, vertexList[polygon->iV0].z, polygon->s0, polygon->t0);
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV1].x, vertexList[polygon->iV1].y, vertexList[polygon->iV1].z, polygon->s1, polygon->t1);
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV2].x, vertexList[polygon->iV2].y, vertexList[polygon->iV2].z, polygon->s2, polygon->t2);
                }
#endif
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            default:
#if MESH_ERROR_CHECKING
                dbgFatalf(DBG_Loc, "meshRender: invalid meshPolyMode: 0x%x", meshPolyMode);
#endif
                break;
        }
        polygon++;
    }
    glEnd();                                            //done drawing these triangles

    glShadeModel(GL_SMOOTH);
    if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (enableBlend)
    {
        glDisable(GL_BLEND);
    }

    if (g_WireframeHack)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        rndLightingEnable(lightOn);
        if (enableBlend)
        {
            glDisable(GL_LINE_SMOOTH);
        }
    }
}

void meshEnableVertexArrays(void* vlist, sdword first, sdword count)
{
    glVertexPointer(3, GL_FLOAT, 4*sizeof(GLfloat), vlist);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);

    if (glCapFeatureExists(GL_COMPILED_ARRAYS_EXT))
    {
        glLockArraysEXT(first, count);
    }
}

void meshDisableVertexArrays(void)
{
    glDisableClientState(GL_VERTEX_ARRAY);
    if (glCapFeatureExists(GL_COMPILED_ARRAYS_EXT))
    {
        glUnlockArraysEXT();
    }
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectRenderLit
    Description : this mesh renderer performs lighting operations itself as a
                  workaround for buggy or slow GL impl's.
                  uses ArrayElement for vertices, & will compiled arrays
    Inputs      : ...
    Outputs     :
    Return      :
    Assert      : lighting is enabled
----------------------------------------------------------------------------*/
void meshObjectRenderLit(polygonobject *object, materialentry *materials, sdword iColorScheme)
{
    sdword iPoly;
    vertexentry *vertexList;
    polyentry *polygon;
    normalentry *normal, *normalList;
    sdword currentMaterial = -1;
    GLenum mode = GL_SMOOTH;

    glShadeModel(mode);

    rndLightingEnable(FALSE);

    vertexList = object->pVertexList;                       //get base of vertex list

    normalList = object->pNormalList;                       //get base of normal list
    polygon = object->pPolygonList;                         //get first polygon list entry

    alodIncPolys(object->nPolygons);

    meshEnableVertexArrays((void*)vertexList, 0, object->nVertices);

    glBegin(GL_TRIANGLES);                                  //prepare to draw triangles

    for (iPoly = 0; iPoly < object->nPolygons; iPoly++)
    {
#if MESH_ANAL_CHECKING                                      //validate vertex indices
        dbgAssert(polygon->iV0 < object->nVertices);
        dbgAssert(polygon->iV1 < object->nVertices);
        dbgAssert(polygon->iV2 < object->nVertices);
#endif
        if (polygon->iMaterial != currentMaterial)
        {                                                   //if a new material
            glEnd();                                        //end current polygon run
            currentMaterial = polygon->iMaterial;           //remember current material
            meshCurrentMaterial(&materials[currentMaterial], iColorScheme);//set new material
            if (bFade)
            {
                glEnable(GL_BLEND);
            }
            glBegin(GL_TRIANGLES);                          //start new run
#if MESH_MATERIAL_STATS
            nMaterialChanges++;                             //record material stats
            iMaterialMax = max(currentMaterial, iMaterialMax);
#endif //MESH_MATERIAL_STATS
        }

        switch (meshPolyMode)
        {
            case MPM_Flat:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];
                meshColour(normal); //shader

                glArrayElement(polygon->iV0);
                glArrayElement(polygon->iV1);
                glArrayElement(polygon->iV2);
#if RND_POLY_STATS
                rndNumberPolys++;
#endif //RND_POLY_STATS
                break;
            case MPM_Texture:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];
                meshColour(normal); //shader

                glTexCoord2f(polygon->s0, polygon->t0);
                glArrayElement(polygon->iV0);

                glTexCoord2f(polygon->s1, polygon->t1);
                glArrayElement(polygon->iV1);

                glTexCoord2f(polygon->s2, polygon->t2);
                glArrayElement(polygon->iV2);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
#endif //RND_POLY_STATS
                break;
            case MPM_Smooth:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                meshColour(normal); //shader
                glArrayElement(polygon->iV0);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                meshColour(normal); //shader
                glArrayElement(polygon->iV1);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                meshColour(normal); //shader
                glArrayElement(polygon->iV2);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            case MPM_SmoothTexture:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                meshColour(normal); //shader
                glTexCoord2f(polygon->s0, polygon->t0);
                glArrayElement(polygon->iV0);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                meshColour(normal); //shader
                glTexCoord2f(polygon->s1, polygon->t1);
                glArrayElement(polygon->iV1);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                meshColour(normal); //shader
                glTexCoord2f(polygon->s2, polygon->t2);
                glArrayElement(polygon->iV2);
#if MESH_SURFACE_NAME_DEBUG
                if (gTestTexture)
                {
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV0].x, vertexList[polygon->iV0].y, vertexList[polygon->iV0].z, polygon->s0, polygon->t0);
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV1].x, vertexList[polygon->iV1].y, vertexList[polygon->iV1].z, polygon->s1, polygon->t1);
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV2].x, vertexList[polygon->iV2].y, vertexList[polygon->iV2].z, polygon->s2, polygon->t2);
                }
#endif
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            default:
#if MESH_ERROR_CHECKING
                dbgFatalf(DBG_Loc, "meshRender: invalid meshPolyMode: 0x%x", meshPolyMode);
#endif
                break;
        }
        polygon++;
    }
    glEnd();                                            //done drawing these triangles

    meshDisableVertexArrays();

    glShadeModel(GL_SMOOTH);
    if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (bFade)
    {
        glDisable(GL_BLEND);
    }

    rndLightingEnable(TRUE); //shader
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectRenderLitRGL
    Description : this mesh renderer performs lighting operations itself as a
                  workaround for buggy or slow GL impl's
    Inputs      : ...
    Outputs     :
    Return      :
    Assert      : lighting is enabled
----------------------------------------------------------------------------*/
void meshObjectRenderLitRGL(polygonobject *object, materialentry *materials, sdword iColorScheme)
{
    sdword iPoly;
    vertexentry *vertexList;
    polyentry *polygon;
    normalentry *normal, *normalList;
    sdword currentMaterial = -1;
    GLenum mode = GL_SMOOTH;

    glShadeModel(mode);

    rndLightingEnable(FALSE);

    vertexList = object->pVertexList;                       //get base of vertex list

    normalList = object->pNormalList;                       //get base of normal list
    polygon = object->pPolygonList;                         //get first polygon list entry

    alodIncPolys(object->nPolygons);

    glBegin(GL_TRIANGLES);                                  //prepare to draw triangles

    for (iPoly = 0; iPoly < object->nPolygons; iPoly++)
    {
#if MESH_ANAL_CHECKING                                      //validate vertex indices
        dbgAssert(polygon->iV0 < object->nVertices);
        dbgAssert(polygon->iV1 < object->nVertices);
        dbgAssert(polygon->iV2 < object->nVertices);
#endif
        if (polygon->iMaterial != currentMaterial)
        {                                                   //if a new material
            glEnd();                                        //end current polygon run
            currentMaterial = polygon->iMaterial;           //remember current material
            meshCurrentMaterial(&materials[currentMaterial], iColorScheme);//set new material
            if (bFade)
            {
                glEnable(GL_BLEND);
            }
            glBegin(GL_TRIANGLES);                          //start new run
#if MESH_MATERIAL_STATS
            nMaterialChanges++;                             //record material stats
            iMaterialMax = max(currentMaterial, iMaterialMax);
#endif //MESH_MATERIAL_STATS
        }

        switch (meshPolyMode)
        {
            case MPM_Flat:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];
                meshColour(normal); //shader

                glVertex3fv((GLfloat*)&vertexList[polygon->iV0]);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1]);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2]);
#if RND_POLY_STATS
                rndNumberPolys++;
#endif //RND_POLY_STATS
                break;
            case MPM_Texture:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];
                meshColour(normal); //shader

                glTexCoord2f(polygon->s0, polygon->t0);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0]);

                glTexCoord2f(polygon->s1, polygon->t1);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1]);

                glTexCoord2f(polygon->s2, polygon->t2);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2]);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
#endif //RND_POLY_STATS
                break;
            case MPM_Smooth:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                meshColour(normal); //shader
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0]);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                meshColour(normal); //shader
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1]);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                meshColour(normal); //shader
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2]);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            case MPM_SmoothTexture:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                meshColour(normal); //shader
                glTexCoord2f(polygon->s0, polygon->t0);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0]);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                meshColour(normal); //shader
                glTexCoord2f(polygon->s1, polygon->t1);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1]);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                meshColour(normal); //shader
                glTexCoord2f(polygon->s2, polygon->t2);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2]);
#if MESH_SURFACE_NAME_DEBUG
                if (gTestTexture)
                {
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV0].x, vertexList[polygon->iV0].y, vertexList[polygon->iV0].z, polygon->s0, polygon->t0);
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV1].x, vertexList[polygon->iV1].y, vertexList[polygon->iV1].z, polygon->s1, polygon->t1);
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV2].x, vertexList[polygon->iV2].y, vertexList[polygon->iV2].z, polygon->s2, polygon->t2);
                }
#endif
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            default:
#if MESH_ERROR_CHECKING
                dbgFatalf(DBG_Loc, "meshRender: invalid meshPolyMode: 0x%x", meshPolyMode);
#endif
                break;
        }
        polygon++;
    }
    glEnd();                                            //done drawing these triangles

    glShadeModel(GL_SMOOTH);
    if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (bFade)
    {
        glDisable(GL_BLEND);
    }

    rndLightingEnable(TRUE); //shader
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectRenderLitRGLvx
    Description : rGL version of meshObjectRenderLit that takes advantage of rGL's
                  vertex arrays which are decoupled from other client state
    Inputs      : ...
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshObjectRenderLitRGLvx(polygonobject *object, materialentry *materials, sdword iColorScheme)
{
    sdword iPoly;
    vertexentry *vertexList;
    polyentry *polygon;
    normalentry *normal, *normalList;
    sdword currentMaterial = -1;
    GLenum mode = GL_SMOOTH;
    hmatrix modelview, projection;

    //load identity matrices into modelview, projection
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat*)&projection);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)&modelview);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //tell rGL to reuse our buffers and such
    rglEnable(RGL_RASTERIZE_ONLY);

    glShadeModel(mode);

    rndLightingEnable(FALSE);

    vertexList = object->pVertexList;                       //get base of vertex list
    //possibly expand the size of the vertex pool
    transGrowVertexLists((object->nVertices + 3) & (~3));
    if (transPerspectiveMatrix(&projection))
    {
        transTransformCompletely(
            object->nVertices, clipVertexList, eyeVertexList, vertexList, &modelview, &projection);
    }
    else
    {
        //object -> eye
        transTransformVertexList(object->nVertices, eyeVertexList, vertexList, &modelview);
        //eye -> clip
        transGeneralPerspectiveTransform(object->nVertices, clipVertexList, eyeVertexList, &projection);
    }

    //use our vertex buffer
    glVertexPointer(4, GL_FLOAT, 0, (void*)clipVertexList);
    glEnableClientState(GL_VERTEX_ARRAY);

    normalList = object->pNormalList;                       //get base of normal list
    polygon = object->pPolygonList;                         //get first polygon list entry

    //increase the number of polys the autolod module has seen
    alodIncPolys(object->nPolygons);

    glBegin(GL_TRIANGLES);                                  //prepare to draw triangles

    for (iPoly = 0; iPoly < object->nPolygons; iPoly++)
    {
#if MESH_ANAL_CHECKING                                      //validate vertex indices
        dbgAssert(polygon->iV0 < object->nVertices);
        dbgAssert(polygon->iV1 < object->nVertices);
        dbgAssert(polygon->iV2 < object->nVertices);
#endif
        if (polygon->iMaterial != currentMaterial)
        {                                                   //if a new material
            glEnd();                                        //end current polygon run
            currentMaterial = polygon->iMaterial;           //remember current material
            meshCurrentMaterial(&materials[currentMaterial], iColorScheme);//set new material
            if (bFade)
            {
                glEnable(GL_BLEND);
            }
            glBegin(GL_TRIANGLES);                          //start new run
#if MESH_MATERIAL_STATS
            nMaterialChanges++;                             //record material stats
            iMaterialMax = max(currentMaterial, iMaterialMax);
#endif //MESH_MATERIAL_STATS
        }

        switch (meshPolyMode)
        {
            case MPM_Flat:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];
                meshColour(normal);

                glArrayElement(polygon->iV0);
                glArrayElement(polygon->iV1);
                glArrayElement(polygon->iV2);
#if RND_POLY_STATS
                rndNumberPolys++;
#endif //RND_POLY_STATS
                break;
            case MPM_Texture:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];
                meshColour(normal);

                glTexCoord2f(polygon->s0, polygon->t0);
                glArrayElement(polygon->iV0);

                glTexCoord2f(polygon->s1, polygon->t1);
                glArrayElement(polygon->iV1);

                glTexCoord2f(polygon->s2, polygon->t2);
                glArrayElement(polygon->iV2);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
#endif //RND_POLY_STATS
                break;
            case MPM_Smooth:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                meshColour(normal);
                glArrayElement(polygon->iV0);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                meshColour(normal);
                glArrayElement(polygon->iV1);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                meshColour(normal);
                glArrayElement(polygon->iV2);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            case MPM_SmoothTexture:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                meshColour(normal);
                glTexCoord2f(polygon->s0, polygon->t0);
                glArrayElement(polygon->iV0);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                meshColour(normal);
                glTexCoord2f(polygon->s1, polygon->t1);
                glArrayElement(polygon->iV1);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                meshColour(normal);
                glTexCoord2f(polygon->s2, polygon->t2);
                glArrayElement(polygon->iV2);
#if MESH_SURFACE_NAME_DEBUG
                if (gTestTexture)
                {
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV0].x, vertexList[polygon->iV0].y, vertexList[polygon->iV0].z, polygon->s0, polygon->t0);
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV1].x, vertexList[polygon->iV1].y, vertexList[polygon->iV1].z, polygon->s1, polygon->t1);
                    dbgMessagef("\n(%4.2f, %4.2f, %4.2f) => (%4.2f, %4.2f)", vertexList[polygon->iV2].x, vertexList[polygon->iV2].y, vertexList[polygon->iV2].z, polygon->s2, polygon->t2);
                }
#endif
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            default:
#if MESH_ERROR_CHECKING
                dbgFatalf(DBG_Loc, "meshRender: invalid meshPolyMode: 0x%x", meshPolyMode);
#endif
                break;
        }
        polygon++;
    }
    glEnd();                                            //done drawing these triangles

    glShadeModel(GL_SMOOTH);
    if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    if (bFade)
    {
        glDisable(GL_BLEND);
    }

    rndLightingEnable(TRUE);

    //reload projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat*)&projection);
    //ASSERT: modelview is always popped after a meshObjectRender
    glMatrixMode(GL_MODELVIEW);

    //reset state
    rglDisable(RGL_RASTERIZE_ONLY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

/*-----------------------------------------------------------------------------
    Name        : meshSpecObjectRender
    Description : renders a mesh with specular-only shading.  to be used with renderers
                  that don't support specular modes in their internal pipeline (ie. !rGL)
    Inputs      : ..
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshSpecObjectRender(polygonobject *object, materialentry *materials, sdword iColorScheme)
{
    sdword iPoly;
    vertexentry *vertexList;
    polyentry *polygon;
    normalentry *normal, *normalList;
    sdword currentMaterial = -1;
    GLenum mode = GL_SMOOTH;
    sdword lightOn;

    real32 modelview[16], modelviewInv[16];

    lightOn = rndLightingEnable(FALSE);

    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    shInvertMatrix(modelviewInv, modelview);

    glShadeModel(mode);

    alodIncPolys(object->nPolygons);

    vertexList = object->pVertexList;                       //get base of vertex list
    normalList = object->pNormalList;                       //get base of normal list
    polygon = object->pPolygonList;                         //get first polygon list entry

    glBegin(GL_TRIANGLES);                                  //prepare to draw triangles

    for (iPoly = 0; iPoly < object->nPolygons; iPoly++)
    {
#if MESH_ANAL_CHECKING                                      //validate vertex indices
        dbgAssert(polygon->iV0 < object->nVertices);
        dbgAssert(polygon->iV1 < object->nVertices);
        dbgAssert(polygon->iV2 < object->nVertices);
#endif
        if (polygon->iMaterial != currentMaterial)
        {                                                   //if a new material
            glEnd();                                        //end current polygon run
            currentMaterial = polygon->iMaterial;           //remember current material
            meshCurrentMaterial(&materials[currentMaterial], iColorScheme);//set new material
            glBegin(GL_TRIANGLES);                          //start new run
#if MESH_MATERIAL_STATS
            nMaterialChanges++;                             //record material stats
            iMaterialMax = max(currentMaterial, iMaterialMax);
#endif //MESH_MATERIAL_STATS
        }

        switch (meshPolyMode)
        {
            case MPM_Flat:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];

                meshSpecColour(normal, &vertexList[polygon->iV0], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0]);

                meshSpecColour(normal, &vertexList[polygon->iV1], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1]);

                meshSpecColour(normal, &vertexList[polygon->iV2], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2]);

#if RND_POLY_STATS
                rndNumberPolys++;
#endif //RND_POLY_STATS
                break;
            case MPM_Texture:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                normal = &normalList[polygon->iFaceNormal];

                glTexCoord2f(polygon->s0, polygon->t0);
                meshSpecColour(normal, &vertexList[polygon->iV0], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0]);

                glTexCoord2f(polygon->s1, polygon->t1);
                meshSpecColour(normal, &vertexList[polygon->iV1], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1]);

                glTexCoord2f(polygon->s2, polygon->t2);
                meshSpecColour(normal, &vertexList[polygon->iV2], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2]);

#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
#endif //RND_POLY_STATS
                break;
            case MPM_Smooth:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                meshSpecColour(normal, &vertexList[polygon->iV0], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0]);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                meshSpecColour(normal, &vertexList[polygon->iV1], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1]);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                meshSpecColour(normal, &vertexList[polygon->iV2], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2]);

#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            case MPM_SmoothTexture:
                dbgAssert(vertexList[polygon->iV0].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV0].iVertexNormal];
                glTexCoord2f(polygon->s0, polygon->t0);
                meshSpecColour(normal, &vertexList[polygon->iV0], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV0]);

                dbgAssert(vertexList[polygon->iV1].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV1].iVertexNormal];
                glTexCoord2f(polygon->s1, polygon->t1);
                meshSpecColour(normal, &vertexList[polygon->iV1], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV1]);

                dbgAssert(vertexList[polygon->iV2].iVertexNormal != UWORD_Max);
                normal = &normalList[vertexList[polygon->iV2].iVertexNormal];
                glTexCoord2f(polygon->s2, polygon->t2);
                meshSpecColour(normal, &vertexList[polygon->iV2], modelview, modelviewInv);
                glVertex3fv((GLfloat*)&vertexList[polygon->iV2]);

#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            default:
#if MESH_ERROR_CHECKING
                dbgFatalf(DBG_Loc, "meshRender: invalid meshPolyMode: 0x%x", meshPolyMode);
#endif
                break;
        }
        polygon++;
    }
    glEnd();                                            //done drawing these triangles

    glShadeModel(GL_SMOOTH);
    if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    rndLightingEnable(lightOn);
}

/*-----------------------------------------------------------------------------
    Name        : meshLerpVerts
    Description : Helper for morphed object rendering.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshLerpVerts(vector* c, vertexentry* a, vertexentry* b, real32 frac)
{
    real32 ofrac;

    ofrac = 1.0f - frac;

    c->x = ofrac * a->x + frac * b->x;
    c->y = ofrac * a->y + frac * b->y;
    c->z = ofrac * a->z + frac * b->z;
}

/*-----------------------------------------------------------------------------
    Name        : meshLerpNormal
    Description : Helper for morphed object rendering.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void meshLerpNormal(vector* c, normalentry* a, normalentry* b, real32 frac)
{
    real32 ofrac;

    ofrac = 1.0f - frac;

    c->x = ofrac * a->x + frac * b->x;
    c->y = ofrac * a->y + frac * b->y;
    c->z = ofrac * a->z + frac * b->z;
}

/*-----------------------------------------------------------------------------
    Name        : meshMorphedObjectRender
    Description : Render a mesh object morphed between two endpoint objects
    Inputs      : object1, object2 - objects being lerped between
                  uvPolys - polygons to get the u/v coordinates from.  Must be
                    exactly the same as the polygon objects.
                  materials - material list for this object.
                  frac - how much of object2 to use.
                  iColorScheme - color scheme to render in, if any
    Outputs     :
    Return      :
    Note        : use object1's stuff, but lerp to object2
----------------------------------------------------------------------------*/
void meshMorphedObjectRender(
    polygonobject* object1, polygonobject* object2, polyentry *uvPolys,
    materialentry* materials, real32 frac, sdword iColorScheme)
{
    sdword iPoly;
    vertexentry* vertexList1;
    vertexentry* vertexList2;
    polyentry*   polygon;
    polyentry*   uvPolygon;
    normalentry* normalList1;
    normalentry* normalList2;
    sdword currentMaterial = -1;
    vector vert0, vert1, vert2, normal;
    GLenum mode = GL_SMOOTH;
#if MESH_MORPH_DEBUG
    sdword colorIndex = 0;
    color  morphLineColor1, morphLineColor2;
    GLboolean texEnabled, lightEnabled, blendEnabled;
#endif
    bool lightOn;
    real32 modelview[16], modelviewInv[16];

    dbgAssert(object1->nVertices == object2->nVertices);
    dbgAssert(object1->nPolygons == object2->nPolygons);
    glShadeModel(mode);

    if (g_SpecHack)
    {
        lightOn = rndLightingEnable(FALSE);
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        shInvertMatrix(modelviewInv, modelview);
    }

    alodIncPolys(object1->nPolygons);

    vertexList1 = object1->pVertexList;
    vertexList2 = object2->pVertexList;
    normalList1 = object1->pNormalList;
    normalList2 = object2->pNormalList;
    polygon = object1->pPolygonList;
    uvPolygon = uvPolys;
    glBegin(GL_TRIANGLES);

    for (iPoly = 0; iPoly < object1->nPolygons; iPoly++)
    {
#if MESH_ANAL_CHECKING
        dbgAssert(polygon->iV0 < object1->nVertices);
        dbgAssert(polygon->iV1 < object1->nVertices);
        dbgAssert(polygon->iV2 < object1->nVertices);
#endif
        if (polygon->iMaterial != currentMaterial)
        {
            glEnd();
            currentMaterial = polygon->iMaterial;
            meshCurrentMaterial(&materials[currentMaterial], iColorScheme);
            glBegin(GL_TRIANGLES);
#if MESH_MATERIAL_STATS
            nMaterialChanges++;
            iMaterialMax = max(currentMaterial, iMaterialMax);
#endif //MESH_MATERIAL_STATS
        }
        meshLerpVerts(&vert0, &vertexList1[polygon->iV0], &vertexList2[polygon->iV0], frac);
        meshLerpVerts(&vert1, &vertexList1[polygon->iV1], &vertexList2[polygon->iV1], frac);
        meshLerpVerts(&vert2, &vertexList1[polygon->iV2], &vertexList2[polygon->iV2], frac);
#if MESH_MORPH_DEBUG                                        //morphed mesh debug code
        if (meshMorphDebug)
        {
            //disable a bunch of stuff
            glEnd();
            texEnabled   = glIsEnabled(GL_TEXTURE_2D);
            lightEnabled = glIsEnabled(GL_LIGHTING);
            blendEnabled = glIsEnabled(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_BLEND);

            //draw outlines of endpoint meshes
            morphLineColor1 = meshMorphLineColors[colorIndex].from;
            morphLineColor2 = meshMorphLineColors[colorIndex].to;
            colorIndex++;                                   //choose a color for the line
            if (meshMorphLineColors[colorIndex].from == colBlack)
            {
                colorIndex = 0;
            }
            if (!keyIsHit(ONEKEY))
            {
                primLine3((vector *)&vertexList1[polygon->iV0], (vector *)&vertexList1[polygon->iV1], morphLineColor2);
            }
            if (!keyIsHit(TWOKEY))
            {
                primLine3((vector *)&vertexList2[polygon->iV0], (vector *)&vertexList2[polygon->iV1], morphLineColor1);
            }
            morphLineColor1 = meshMorphLineColors[colorIndex].from;
            morphLineColor2 = meshMorphLineColors[colorIndex].to;
            colorIndex++;                                   //choose a color for the line
            if (meshMorphLineColors[colorIndex].from == colBlack)
            {
                colorIndex = 0;
            }
            if (!keyIsHit(ONEKEY))
            {
                primLine3((vector *)&vertexList1[polygon->iV1], (vector *)&vertexList1[polygon->iV2], morphLineColor2);
            }
            if (!keyIsHit(TWOKEY))
            {
                primLine3((vector *)&vertexList2[polygon->iV1], (vector *)&vertexList2[polygon->iV2], morphLineColor1);
            }
            morphLineColor1 = meshMorphLineColors[colorIndex].from;
            morphLineColor2 = meshMorphLineColors[colorIndex].to;
            colorIndex++;                                   //choose a color for the line
            if (meshMorphLineColors[colorIndex].from == colBlack)
            {
                colorIndex = 0;
            }
            if (!keyIsHit(ONEKEY))
            {
                primLine3((vector *)&vertexList1[polygon->iV2], (vector *)&vertexList1[polygon->iV0], morphLineColor2);
            }
            if (!keyIsHit(TWOKEY))
            {
                primLine3((vector *)&vertexList2[polygon->iV2], (vector *)&vertexList2[polygon->iV0], morphLineColor1);
            }
            //draw the connecting lines
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x5a5a);
            if (!keyIsHit(THREEKEY))
            {
                primLine3((vector *)&vertexList1[polygon->iV0], (vector *)&vertexList2[polygon->iV0], colWhite);
                primLine3((vector *)&vertexList1[polygon->iV1], (vector *)&vertexList2[polygon->iV1], colWhite);
                primLine3((vector *)&vertexList1[polygon->iV2], (vector *)&vertexList2[polygon->iV2], colWhite);
            }
            glDisable(GL_LINE_STIPPLE);
            if (!keyIsHit(FOURKEY))
            {
                primLine3(&vert0, &vert1, colRGB(128, 128, 128));
                primLine3(&vert1, &vert2, colRGB(128, 128, 128));
                primLine3(&vert2, &vert0, colRGB(128, 128, 128));
            }

            if (texEnabled)
            {
                glEnable(GL_TEXTURE_2D);
            }
            if (texEnabled)
            {
                glEnable(GL_LIGHTING);
            }
            if (blendEnabled)
            {
                glEnable(GL_BLEND);
            }
            glBegin(GL_TRIANGLES);
        }
#endif //MESH_MORPH_DEBUG
        switch (meshPolyMode)
        {
            case MPM_Flat:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                if (mode != GL_FLAT)
                {
                    mode = GL_FLAT;
                    glShadeModel(mode);
                }

                meshLerpNormal(&normal, &normalList1[polygon->iFaceNormal], &normalList2[polygon->iFaceNormal], frac);
                if (g_SpecHack) meshMorphedSpecColour(&normal, modelview, modelviewInv);
                glNormal3f(normal.x, normal.y, normal.z);

                glVertex3fv((GLfloat*)&vert0);
                glVertex3fv((GLfloat*)&vert1);
                glVertex3fv((GLfloat*)&vert2);
#if RND_POLY_STATS
                rndNumberPolys++;
#endif //RND_POLY_STATS
                break;
            case MPM_Texture:
                dbgAssert(polygon->iFaceNormal != UWORD_Max);
                if (mode != GL_FLAT)
                {
                    mode = GL_FLAT;
                    glShadeModel(mode);
                }

                meshLerpNormal(&normal, &normalList1[polygon->iFaceNormal], &normalList2[polygon->iFaceNormal], frac);
                if (g_SpecHack) meshMorphedSpecColour(&normal, modelview, modelviewInv);
                glNormal3f(normal.x, normal.y, normal.z);

                glTexCoord2f(uvPolygon->s0, uvPolygon->t0);
                glVertex3fv((GLfloat*)&vert0);
                glTexCoord2f(uvPolygon->s1, uvPolygon->t1);
                glVertex3fv((GLfloat*)&vert1);
                glTexCoord2f(uvPolygon->s2, uvPolygon->t2);
                glVertex3fv((GLfloat*)&vert2);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
#endif //RND_POLY_STATS
                break;
            case MPM_Smooth:
                dbgAssert(vertexList1[polygon->iV0].iVertexNormal != UWORD_Max);
                if (mode != GL_SMOOTH)
                {
                    mode = GL_SMOOTH;
                    glShadeModel(mode);
                }

                meshLerpNormal(&normal, &normalList1[vertexList1[polygon->iV0].iVertexNormal],
                               &normalList2[vertexList2[polygon->iV0].iVertexNormal], frac);
                if (g_SpecHack) meshMorphedSpecColour(&normal, modelview, modelviewInv);
                glNormal3f(normal.x, normal.y, normal.z);
                glVertex3fv((GLfloat*)&vert0);

                dbgAssert(vertexList1[polygon->iV1].iVertexNormal != UWORD_Max);
                meshLerpNormal(&normal, &normalList1[vertexList1[polygon->iV1].iVertexNormal],
                               &normalList2[vertexList2[polygon->iV1].iVertexNormal], frac);
                if (g_SpecHack) meshMorphedSpecColour(&normal, modelview, modelviewInv);
                glNormal3f(normal.x, normal.y, normal.z);
                glVertex3fv((GLfloat*)&vert1);

                dbgAssert(vertexList1[polygon->iV2].iVertexNormal != UWORD_Max);
                meshLerpNormal(&normal, &normalList1[vertexList1[polygon->iV2].iVertexNormal],
                               &normalList2[vertexList2[polygon->iV2].iVertexNormal], frac);
                if (g_SpecHack) meshMorphedSpecColour(&normal, modelview, modelviewInv);
                glNormal3f(normal.x, normal.y, normal.z);
                glVertex3fv((GLfloat*)&vert2);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            case MPM_SmoothTexture:
                if (mode != GL_SMOOTH)
                {
                    mode = GL_SMOOTH;
                    glShadeModel(mode);
                }

                dbgAssert(vertexList1[polygon->iV0].iVertexNormal != UWORD_Max);
                meshLerpNormal(&normal, &normalList1[vertexList1[polygon->iV0].iVertexNormal],
                               &normalList2[vertexList2[polygon->iV0].iVertexNormal], frac);
                if (g_SpecHack) meshMorphedSpecColour(&normal, modelview, modelviewInv);
                glNormal3f(normal.x, normal.y, normal.z);
                glTexCoord2f(uvPolygon->s0, uvPolygon->t0);
                glVertex3fv((GLfloat*)&vert0);

                dbgAssert(vertexList1[polygon->iV1].iVertexNormal != UWORD_Max);
                meshLerpNormal(&normal, &normalList1[vertexList1[polygon->iV1].iVertexNormal],
                               &normalList2[vertexList2[polygon->iV1].iVertexNormal], frac);
                if (g_SpecHack) meshMorphedSpecColour(&normal, modelview, modelviewInv);
                glNormal3f(normal.x, normal.y, normal.z);
                glTexCoord2f(uvPolygon->s1, uvPolygon->t1);
                glVertex3fv((GLfloat*)&vert1);

                dbgAssert(vertexList1[polygon->iV2].iVertexNormal != UWORD_Max);
                meshLerpNormal(&normal, &normalList1[vertexList1[polygon->iV2].iVertexNormal],
                               &normalList2[vertexList2[polygon->iV2].iVertexNormal], frac);
                if (g_SpecHack) meshMorphedSpecColour(&normal, modelview, modelviewInv);
                glNormal3f(normal.x, normal.y, normal.z);
                glTexCoord2f(uvPolygon->s2, uvPolygon->t2);
                glVertex3fv((GLfloat*)&vert2);
#if RND_POLY_STATS
                rndNumberPolys++;
                rndNumberTextured++;
                rndNumberSmoothed++;
#endif //RND_POLY_STATS
                break;
            default:
#if MESH_ERROR_CHECKING
                dbgFatalf(DBG_Loc, "meshRender: invalid meshPolyMode: 0x%x", meshPolyMode);
#endif
                break;
        }
        polygon++;
        uvPolygon++;
    }
    glEnd();

    if (g_SpecHack)
    {
        rndLightingEnable(lightOn);
    }

    glShadeModel(GL_SMOOTH);
    if (!usingShader) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : meshMorphedObjectRenderTex
    Description : Render a morphed mesh object, lerping between two meshes with
                    a single texture map.
    Inputs      : object1, object2 - objects being lerped between
                  uvPolys - polygons to get the u/v coordinates from.  Must be
                    exactly the same as the polygon objects.
                  materials - material list for this object.
                  frac - how much of object2 to use.
                  iColorScheme - color scheme to render in, or 0 if none.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void meshMorphedObjectRenderTex(polygonobject* object1, polygonobject* object2,
                                polyentry *uvPolys, materialentry* material,
                                real32 frac, sdword iColorScheme)
{
    meshCurrentMaterial = meshCurrentMaterialTex;
    meshMorphedObjectRender(object1, object2, uvPolys, material, frac, 0);
    meshCurrentMaterial = meshCurrentMaterialDefault;
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectRenderTex
    Description : Renders an object using a single texture map for the entire surface
    Inputs      : object - object to render
                  material - materials to use (surface attribs will be interpreted properly)
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void meshObjectRenderTex(polygonobject *object, materialentry *material)
{
    meshCurrentMaterial = meshCurrentMaterialTex;
    meshObjectRender(object, material, 0);
    meshCurrentMaterial = meshCurrentMaterialDefault;
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectRenderR
    Description : Render a single polygon object and all it's 'offspring' objects.
    Inputs      : object - object to render.
                  materials - material list to use in rendering
                  iColorScheme - index of player associated with this mesh
                    (for texture coloring) or 0 for no player.
    Outputs     : renders the object and all it's daughter objects.
    Return      : void
----------------------------------------------------------------------------*/
void meshObjectRenderR(polygonobject *object, materialentry *materials, sdword iColorScheme)
{
    polygonobject *daughter;

    glPushMatrix();
    glMultMatrixf((float *)&object->localMatrix);            //set matrix for this object
	shPushLightMatrix(&object->localMatrix);

    for (daughter = object->pDaughter; daughter != NULL; daughter = daughter->pSister)
    {
        meshObjectRenderR(daughter, materials, iColorScheme);     //render all daughter objects
    }

    MESH_OBJECT_RENDER(object, materials, iColorScheme)

    shPopLightMatrix();
    glPopMatrix();
}

/*-----------------------------------------------------------------------------
    Name        : meshRender
    Description : Render the specified mesh
    Inputs      : mesh - mesh to be rendered
                  iColorScheme - index of player associated with this mesh
                    (for texture coloring) or 0 for no player.
    Outputs     : ..
    Return      : ??
    Note        : This function does not even call any GL wrappers.
----------------------------------------------------------------------------*/
sdword meshRender(meshdata *mesh, sdword iColorScheme)
{
    polygonobject *object;

    meshFixupPacoUV(mesh, texLinearFiltering);

    if (g_NoMatSwitch)
    {
        meshCurrentMaterial = meshAlmostCurrentMaterial;
    }
    else
    {
        meshCurrentMaterial = meshCurrentMaterialDefault;
    }
#if MESH_MATERIAL_STATS
    nMaterialChanges = 0;
    iMaterialMax = 0;
#endif //MESH_MATERIAL_STATS
    for (object = &mesh->object[0]; object != NULL; object = object->pSister)
    {
        meshObjectRenderR(object, mesh->localMaterial, iColorScheme);
    }
#if MESH_MATERIAL_STATS
    meshMaterialChanges += nMaterialChanges;                //record material stats
    meshTotalMaterials += iMaterialMax + 1;
#endif //MESH_MATERIAL_STATS
    meshCurrentMaterial = meshCurrentMaterialDefault;
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : meshObjectRenderHierarchy
    Description : Render a polygon object and all it's offspring
    Inputs      : binding - base of a binding list which is updated and returned
                  everything else - same as meshObjectRender
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
mhlocalbinding *meshObjectRenderHierarchy(mhlocalbinding *binding, polygonobject *object, materialentry *materials, sdword iColorScheme)
{
    polygonobject *daughter;

    glPushMatrix();
    if (binding->function != NULL)
    {
        binding->function(binding->flags, &object->localMatrix,//call matrix update function !!! called every frame
                          &binding->matrix, binding->userData, binding->userID);
        glMultMatrixf((float *)&binding->matrix);           //set matrix for this object
		shPushLightMatrix(&binding->matrix);
    }
    else
    {
        glMultMatrixf((float *)&object->localMatrix);       //set matrix for this object
		shPushLightMatrix(&object->localMatrix);
    }
    binding++;
    for (daughter = object->pDaughter; daughter != NULL; daughter = daughter->pSister)
    {
        binding = meshObjectRenderHierarchy(binding, daughter, materials, iColorScheme);     //render all daughter objects
    }
    MESH_OBJECT_RENDER(object, materials, iColorScheme)
    shPopLightMatrix();
    glPopMatrix();
    return(binding);
}

/*-----------------------------------------------------------------------------
    Name        : meshRenderShipHierarchy
    Description : Render a specified LOD of a ship using it's hierarchy binding
                    info.
    Inputs      : ship - ship to render
                  currentLOD - LOD to render at.
                  mesh - mesh to render normally if a proper hierarchy cannot be found.
    Outputs     : renders a ship
    Return      : void
----------------------------------------------------------------------------*/
void meshRenderShipHierarchy(shipbindings *bindings, sdword currentLOD, meshdata *mesh, sdword iColorScheme)
{
    polygonobject *object;
    mhlocalbinding *binding;                                //binding list for this LOD

    meshFixupPacoUV(mesh, texLinearFiltering);

#if MESH_PRE_CALLBACK
    if (bindings->preCallback)
    {                                                       //call the pre callback
        bindings->preCallback(mesh, bindings, currentLOD);
    }
#endif
    if (g_NoMatSwitch)
    {
        meshCurrentMaterial = meshAlmostCurrentMaterial;
    }
    else
    {
        meshCurrentMaterial = meshCurrentMaterialDefault;
    }
#if MESH_MATERIAL_STATS
    nMaterialChanges = 0;
    iMaterialMax = 0;
#endif //MESH_MATERIAL_STATS
    binding = bindings->localBinding[currentLOD];
    for (object = &mesh->object[0]; object != NULL; object = object->pSister)
    {
        binding = meshObjectRenderHierarchy(binding, object, mesh->localMaterial, iColorScheme);
    }
#if MESH_MATERIAL_STATS
    meshMaterialChanges += nMaterialChanges;                //record material stats
    meshTotalMaterials += iMaterialMax + 1;
#endif //MESH_MATERIAL_STATS
    if (bindings->postCallback)
    {                                                       //call the post callback
        bindings->postCallback(mesh, bindings, currentLOD);
    }
    meshCurrentMaterial = meshCurrentMaterialDefault;
}

/*-----------------------------------------------------------------------------
    Name        : meshConcatByUserData
    Description : The rest of the recursion function of meshFindHierarchyMatrixByUserData
    Inputs      : binding - starting binding pointer.
                  object - current object in hierarchy.
                  userData - data to compare against.
    Outputs     : dest - where to store the concatenated matrix.
    Return      :
----------------------------------------------------------------------------*/
mhbinding *meshConcatByUserData(hmatrix *dest, hmatrix *destNC, mhbinding *binding, polygonobject *object, void *userData)
{
    polygonobject *daughter;
    hmatrix mat;

    hmatMultiplyHMatByHMat(&mat, dest, &object->localMatrix);//concatenate a matrix
    if (binding->userData == userData)
    {                                                       //if found user data we were looking for
        *dest = mat;
        *destNC = object->localMatrix;
        return(NULL);                                       //stop looking
    }
    binding++;
    for (daughter = object->pDaughter; daughter != NULL; daughter = daughter->pSister)
    {                                                       //for all child objects
        binding = meshConcatByUserData(&mat, destNC, binding,
                                       daughter, userData);//concatenate all daughter objects
        if (binding == NULL)
        {                                                   //if that tree found it
            *dest = mat;
            return(NULL);
        }
    }
    return(binding);
}

/*-----------------------------------------------------------------------------
    Name        : meshFindHierarchyMatrixByUserData
    Description : Create a concatenated matrix for the mesh hierarchy member
                    with the specified user data pointer.
    Inputs      :
                  bindings - list of mesh bindings
                  userData - userData pointer to compare against.
    Outputs     : dest - where to store the concatenated matrix.
    Return      : TRUE if found, FALSE otherwise
----------------------------------------------------------------------------*/
bool meshFindHierarchyMatrixByUserData(hmatrix *dest, hmatrix *destNC, mhbinding *binding, void *userData)
{
    polygonobject *object;

    *dest = IdentityHMatrix;
    for (object = &binding->object[0]; object != NULL; object = object->pSister)
    {
        dbgAssert(binding->object == object);
        binding = meshConcatByUserData(dest, destNC, binding, object, userData);
        if (binding == NULL)
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : meshBindingListRecurse
    Description : Mesh-hierarchy recursion function to handle the creation of
                   mesh hierarchy binding lists.
    Inputs      : bindingList - list we're creating.
                  index - current entry in this list to set
                  object - current object we're setting
    Outputs     : stores the a pointer to object and all it's children in the
                    binding list in an order appropriate for rendering.
    Return      : new index into the list
----------------------------------------------------------------------------*/
sdword meshBindingListRecurse(mhbinding *bindingList, sdword index, polygonobject *object)
{
    polygonobject *daughter;

    bindingList[index].object = object;                     //save the link to this object
    bindingList[index].function = NULL;                     //NULL means no function
    index++;
    for (daughter = object->pDaughter; daughter != NULL; daughter = daughter->pSister)
    {                                                               //service all children first
        index = meshBindingListRecurse(bindingList, index, daughter);//render all daughter objects
    }
    return(index);
}

/*-----------------------------------------------------------------------------
    Name        : meshBindingListCreate
    Description : Create a binding list for a mesh
    Inputs      : mesh - pointer to mesh to create binding list for
    Outputs     : Allocates and creates a binding list for the specified mesh.
                    Only the polyobject *will be set in each binding structure.
                  Order of structure reflects order to render in.
    Return      : Pointer to new binding list.
----------------------------------------------------------------------------*/
mhbinding *meshBindingListCreate(meshdata *mesh)
{
    sdword index = 0;
    polygonobject *sister;
    mhbinding *bindingList;

    bindingList = memAlloc(mesh->nPolygonObjects * sizeof(mhbinding), "MeshBindingList", NonVolatile);
    for (sister = &mesh->object[0]; sister != NULL; sister = sister->pSister)
    {                                                       //search all the base objects
        index = meshBindingListRecurse(bindingList, index, sister);
    }
#if MESH_ERROR_CHECKING
    if (index != mesh->nPolygonObjects)
    {
        dbgFatalf(DBG_Loc, "index %d != mesh->nPolygonObjects %d for '%s'.", index, mesh->nPolygonObjects, mesh->name);
    }
#endif
    return(bindingList);
}

/*-----------------------------------------------------------------------------
    Name        : meshBindingListDelete
    Description : Free memory associated with a binding list.
    Inputs      : list - list to delete
    Outputs     : just memFree's it
    Return      :
----------------------------------------------------------------------------*/
void meshBindingListDelete(mhbinding *list)
{
    memFree(list);
}

/*-----------------------------------------------------------------------------
    Name        : meshBindingFindByName
    Description : Find a named mesh binding.
    Inputs      : startBinding - starting binding to search from, very start if NULL
                  list - base of binding list
                  mesh - mesh to search in.
                  name - name of binding to search for.
    Outputs     : before returning the binding, sets a pointer to the source
                    poly object.
    Return      : pointer to binding whose name matches or NULL if no match
----------------------------------------------------------------------------*/
mhbinding *meshBindingFindByName(mhbinding *startBinding, mhbinding *list, meshdata *mesh, char *name)
{
    sdword count = mesh->nPolygonObjects;
    if (startBinding != NULL)
    {                                                       //start at either start of list or starting binding as referenced
        startBinding++;                                     //start on entry _after_ previous find
        count -= startBinding - list;                       //less bindings to search
        list = startBinding;                                //new list start
    }

    while (count > 0)
    {                                                       //search rest of list
        if (list->object->pName != NULL)
        {
            if (strcmp(list->object->pName, name) == 0)
            {                                               //if names match
                return(list);                               //all done
            }
        }
        list++;                                             //update pointer/counter
        count--;
    }
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : meshHierarchyWalkR
    Description : Recursion function for meshHierarchyWalk
    Inputs      : object - object to recursively walk
                  cbMesh - callback to call
    Outputs     :
    Return      : TRUE means to stop walking the list
----------------------------------------------------------------------------*/
bool meshHierarchyWalkR(meshdata *mesh, polygonobject *object, meshcallback preCallback, meshcallback postCallback)
{
    polygonobject *daughter;

    if (preCallback)
    {
        if (preCallback(mesh, object, (udword)object->iObject))
        {
            return(TRUE);
        }
    }
    for (daughter = object->pDaughter; daughter != NULL; daughter = daughter->pSister)
    {
        if (meshHierarchyWalkR(mesh, daughter, preCallback, postCallback))
        {
            return(TRUE);
        }
    }
    if (postCallback)
    {
        if (postCallback(mesh, object, (udword)object->iObject))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : meshHierarchyWalk
    Description : Walk a mesh hierarchy in the order it will be rendered,
                    calling a user function for each mesh object.
    Inputs      : mesh - mesh object to walk
                  cbMesh - callback to call for each node in the hierarchy
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void meshHierarchyWalk(meshdata *mesh, meshcallback preCallback, meshcallback postCallback)
{
    polygonobject *object;

    for (object = &mesh->object[0]; object != NULL; object = object->pSister)
    {
        if (meshHierarchyWalkR(mesh, object, preCallback, postCallback))
        {
            break;
        }
    }
}
