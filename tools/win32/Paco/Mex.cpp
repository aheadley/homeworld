//
// Mex.cpp
// mesh-related functions
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "Mex.h"
#include "Input.h"
#include "Main.h"
#include "list.h"
#include "Paco.h"

#define TR_InvalidHandle 0xffffffff

list_crc32 crc32list;

//
// meshLoadTextures
// load textures from a mesh
//
bool meshLoadTextures(char* filePrefix, meshdata* mesh, bool alpha)
{
    sdword index;
    char fullName[1024];

    for (index = 0; index < (mesh->nPublicMaterials + mesh->nLocalMaterials); index++)
    {
        if (mesh->localMaterial[index].texture != 0)
        {
            char* texName = (char*)mesh->localMaterial[index].texture;
            strcpy(fullName, filePrefix);
            strcat(fullName, texName);
            strcat(fullName, ".lif");
            mainLoadTexture(fullName, alpha);
        }
    }

    return true;
}

//
// meshLoadPalettedTextures
// load textures from a mesh given paletteCRC to match
//
bool meshLoadPalettedTextures(char* filePrefix, meshdata* mesh, crc32 crc)
{
    sdword index;
    char fullName[1024];

    for (index = 0; index < (mesh->nPublicMaterials + mesh->nLocalMaterials); index++)
    {
        if (mesh->localMaterial[index].texture != 0)
        {
            char* texName = (char*)mesh->localMaterial[index].texture;
            strcpy(fullName, filePrefix);
            strcat(fullName, texName);
            strcat(fullName, ".lif");
            mainLoadTextureWithCRC(fullName, crc);
        }
    }

    return true;
}

//
// meshCountPalettes
// count the number of palettes used by a mesh.
// store the CRCs in global crc32list
//
sdword meshCountPalettes(char* filePrefix, meshdata* mesh)
{
    sdword index, nPalettes;
    crc32 crc;
    char fullName[1024];

    crc32list.erase(crc32list.begin(), crc32list.end());

    for (index = nPalettes = 0; index < (mesh->nPublicMaterials + mesh->nLocalMaterials); index++)
    {
        if (mesh->localMaterial[index].texture != 0)
        {
            char* texName = (char*)mesh->localMaterial[index].texture;
            strcpy(fullName, filePrefix);
            strcat(fullName, texName);
            strcat(fullName, ".lif");
            crc = mainGetTextureCRC(fullName);
            if (crc != 0)
            {
                bool inList = false;
                for (list_crc32::iterator i = crc32list.begin(); i != crc32list.end(); ++i)
                {
                    if ((*i) == crc)
                    {
                        inList = true;
                    }
                }
                if (!inList)
                {
                    crc32list.push_back(crc);
                    nPalettes++;
                }
            }
        }
    }

    return nPalettes;
}

udword meshPageIndex(lifpage* page)
{
    sdword n = 0;
    for (list_lifpage::iterator i = lifpagelist.begin(); i != lifpagelist.end(); ++i)
    {
        if ((*i) == page)
        {
            return n;
        }
        n++;
    }
    return TR_InvalidHandle;
}

udword meshLIFsPageIndex(char* filePrefix, char* name)
{
    char fullName[1024], lifName[1024];

    strcpy(fullName, filePrefix);
    strcat(fullName, name);
    strcat(fullName, ".lif");
    _strlwr(fullName);

    //search thru liflist & return its page number
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        strcpy(lifName, (*i).filename);
        _strlwr(lifName);
        if (!strcmp(lifName, fullName))
        {
            return meshPageIndex((*i).parent);
        }
    }
    
    return TR_InvalidHandle;
}

lifpage* meshLIFsPage(char* filePrefix, char* name)
{
    char fullName[1024], lifName[1024];

    strcpy(fullName, filePrefix);
    strcat(fullName, name);
    strcat(fullName, ".lif");
    _strlwr(fullName);

    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        strcpy(lifName, (*i).filename);
        _strlwr(lifName);
        if (!strcmp(lifName, fullName))
        {
            return (*i).parent;
        }
    }

    return NULL;
}

void meshGetHeader(GeoFileHeader* header, char* filePrefix, char* fileName)
{
    char fullName[1024];
    FILE* file;

    strcpy(fullName, filePrefix);
    strcat(fullName, fileName);

    file = fopen(fullName, "rb");
    if (file == NULL)
    {
        return;
    }

    fread(header, sizeof(GeoFileHeader), 1, file);

    fclose(file);
}

meshdata* meshLoad(char* filePrefix, char* fileName)
{
    GeoFileHeader header;
    meshdata* mesh;
    sdword fileLength, offset, index;
    polygonobject* object;
    char fullName[1024];
    FILE* file;
    ubyte* data;

    strcpy(fullName, filePrefix);
    strcat(fullName, fileName);

    fileLength = fileSizeGet(fullName);

    file = fopen(fullName, "rb");
    if (file == NULL)
    {
        return NULL;
    }

    fread(&header, sizeof(header), 1, file);

    data = new ubyte[fileLength - sizeof(header) + sizeof(meshdata) - sizeof(polygonobject)];
    mesh = (meshdata*)data;
    offset = (udword)mesh - sizeof(header) + sizeof(meshdata) - sizeof(polygonobject);
    mesh->localSize = header.localSize;
    mesh->nPublicMaterials = header.nPublicMaterials;
    mesh->nLocalMaterials = header.nLocalMaterials;
    mesh->publicMaterial = (materialentry*)(header.oPublicMaterials + offset);
    mesh->localMaterial = (materialentry*)(header.oLocalMaterials + offset);
    mesh->nPolygonObjects = header.nPolygonObjects;

    fread(&mesh->object[0], fileLength - sizeof(header), 1, file);

    fclose(file);

    mesh->name = header.pName + offset;

    //fixup the polygon objects.
    //don't bother with what we won't use
    for (index = 0; index < mesh->nPolygonObjects; index++)
    {
        object = &mesh->object[index];

//        object->pVertexList = (vertexentry*)((udword)object->pVertexList + offset);
//        object->pNormalList = (normalentry*)((udword)object->pNormalList + offset);
        object->pPolygonList = (polyentry*)((udword)object->pPolygonList + offset);

#if 0
        if (object->pMother != NULL)
        {
            object->pMother = (polygonobject*)((udword)object->pMother + offset);
        }
        if (object->pDaughter != NULL)
        {
            object->pDaughter = (polygonobject*)((udword)object->pDaughter + offset);
        }
        if (object->pSister != NULL)
        {
            object->pSister = (polygonobject*)((udword)object->pSister + offset);
        }
        if (object->pName != NULL)
        {
            object->pName += offset;
        }
        object->iObject = index;
        object->nameCRC = 0;
#endif
    }

    //fixup materials
    for (index = 0; index < (mesh->nLocalMaterials + mesh->nPublicMaterials); index++)
    {
#if 0
        if (mesh->localMaterial[index].pName != NULL)
        {
            mesh->localMaterial[index].pName += offset;
        }
        mesh->localMaterial[index].bTexturesRegistered = 0;
#endif
        if (mesh->localMaterial[index].texture != 0)
        {
            mesh->localMaterial[index].texture += offset;
        }
    }

    return mesh;
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
            strcat(outFileName, "\\");
        }

        token = strtok(NULL, "\\/");
    }
}

liflist_t* meshFindNamedLIF(char* filePrefix, char* texName)
{
    char name[1024];
    crc32 crc;

    strcpy(name, filePrefix);
    strcat(name, texName);
    strcat(name, ".lif");

    crc = crc32ComputeString(name);

    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
#if LIF_CRC
        if (crc == (*i).crc)
        {
            return &(*i);
        }
#else
        if (!_stricmp((*i).filename, name))
        {
            return &(*i);
        }
#endif
    }

    return NULL;
}

liflist_t* meshFindUnpackedLIF(char* filePrefix, char* texName)
{
    char name[1024];
    crc32 crc;

    strcpy(name, filePrefix);
    strcat(name, texName);
    strcat(name, ".lif");

    crc = crc32ComputeString(name);

    for (list_lif::iterator i = notpackedlist.begin(); i != notpackedlist.end(); ++i)
    {
#if LIF_CRC
        if (crc == (*i).crc)
        {
            return &(*i);
        }
#else
        if (!_stricmp((*i).filename, name))
        {
            return &(*i);
        }
#endif
    }

    return NULL;
}

#define PAC_S0_0 0x0001
#define PAC_S0_1 0x0002
#define PAC_T0_0 0x0004
#define PAC_T0_1 0x0008
#define PAC_S1_0 0x0010
#define PAC_S1_1 0x0020
#define PAC_T1_0 0x0040
#define PAC_T1_1 0x0080
#define PAC_S2_0 0x0100
#define PAC_S2_1 0x0200
#define PAC_T2_0 0x0400
#define PAC_T2_1 0x0800

#define bitSet(n, b) ((n) |= (b))

bool peq(real32 a, real32 b, sdword iBase)
{
    real32 EPSILON = 1.0f / (real32)iBase;
    EPSILON *= 0.1f;
    a = fabs(a);
    b = fabs(b);
    if (a > (b - EPSILON) &&
        a < (b + EPSILON))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void meshMarkupPolygon_debug(polyentry* poly, char* texName, sdword pWidth, sdword pHeight, sdword width, sdword height)
{
    poly->flags = 0;
    printf("[%s] %d.%d %d.%d (%0.3f %0.3f) (%0.3f %0.3f) (%0.3f %0.3f) ",
           texName,
           pWidth, pHeight,
           width, height,
           poly->uv[0][0], poly->uv[0][1],
           poly->uv[1][0], poly->uv[1][1],
           poly->uv[2][0], poly->uv[2][1]);
    //0
    if (peq(poly->uv[0][0], 0.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S0_0);
        printf("s0_0 ");
    }
    if (peq(poly->uv[0][0], 1.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S0_1);
        printf("s0_1 ");
    }
    if (peq(poly->uv[0][1], 0.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T0_0);
        printf("t0_0 ");
    }
    if (peq(poly->uv[0][1], 1.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T0_1);
        printf("t0_1 ");
    }
    //1
    if (peq(poly->uv[1][0], 0.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S1_0);
        printf("s1_0 ");
    }
    if (peq(poly->uv[1][0], 1.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S1_1);
        printf("s1_1 ");
    }
    if (peq(poly->uv[1][1], 0.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T1_0);
        printf("t1_0 ");
    }
    if (peq(poly->uv[1][1], 1.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T1_1);
        printf("t1_1 ");
    }
    //2
    if (peq(poly->uv[2][0], 0.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S2_0);
        printf("s2_0 ");
    }
    if (peq(poly->uv[2][0], 1.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S2_1);
        printf("s2_1 ");
    }
    if (peq(poly->uv[2][1], 0.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T2_0);
        printf("t2_0 ");
    }
    if (peq(poly->uv[2][1], 1.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T2_1);
        printf("t2_1 ");
    }
    printf("\n");
}

void meshMarkupPolygon(polyentry* poly, char* texName, sdword pWidth, sdword pHeight, sdword width, sdword height)
{
    poly->flags = 0;
    //0
    if (peq(poly->uv[0][0], 0.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S0_0);
    }
    if (peq(poly->uv[0][0], 1.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S0_1);
    }
    if (peq(poly->uv[0][1], 0.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T0_0);
    }
    if (peq(poly->uv[0][1], 1.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T0_1);
    }
    //1
    if (peq(poly->uv[1][0], 0.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S1_0);
    }
    if (peq(poly->uv[1][0], 1.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S1_1);
    }
    if (peq(poly->uv[1][1], 0.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T1_0);
    }
    if (peq(poly->uv[1][1], 1.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T1_1);
    }
    //2
    if (peq(poly->uv[2][0], 0.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S2_0);
    }
    if (peq(poly->uv[2][0], 1.0f, pWidth))
    {
        bitSet(poly->flags, PAC_S2_1);
    }
    if (peq(poly->uv[2][1], 0.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T2_0);
    }
    if (peq(poly->uv[2][1], 1.0f, pHeight))
    {
        bitSet(poly->flags, PAC_T2_1);
    }
}

void meshFixupSingleUV(lifpage* page, liflist_t* lif, real32& sOut, real32& tOut, real32 sIn, real32 tIn)
{
    if (sIn < 0.0f)
    {
        sIn += 1.0f;
    }
    if (tIn < 0.0f)
    {
        tIn += 1.0f;
    }

    if (lif->flags & PACF_Rotated)
    {
        //scale to bitmap dimensions
        sOut = sIn * lif->header->height;
        tOut = tIn * lif->header->width;

        //add upper left of page
        sOut += lif->packedY;
        tOut += lif->packedX;

        //normalize
        sOut /= page->height;
        tOut /= page->width;

        real32 temp = sOut;
        sOut = tOut;
        tOut = temp;
    }
    else
    {
        //scale to bitmap dimensions
        sOut = sIn * lif->header->width;
        tOut = tIn * lif->header->height;

        //add upper left of page
        sOut += lif->packedX;
        tOut += lif->packedY;

        //normalize
        sOut /= page->width;
        tOut /= page->height;
    }
}

udword bitToExp2(udword number)
{
    switch (number)
    {
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    case 16:
        return 4;
    case 32:
        return 5;
    case 64:
        return 6;
    case 128:
        return 7;
    case 256:
        return 8;
    default:
        return 0;
    }
}

void meshFixupUV(polyentry* poly, char* texName, char* filePrefix)
{
    liflist_t* lif;
    lifpage* page;
    real32 s[3], t[3];
    sdword index;
    udword width, height;

    lif = meshFindNamedLIF(filePrefix, texName);
    if (lif == NULL)
    {
        //unpackable
        lif = meshFindUnpackedLIF(filePrefix, texName);
        if (lif == NULL)
        {
            return;
        }
        width  = bitToExp2(lif->header->width);
        height = bitToExp2(lif->header->height);
        width <<= 4;
        poly->reserved[0] = 1;
        poly->reserved[1] = width | height;
//        printf("*");
        meshMarkupPolygon(poly, lifShortName(texName),
                          lif->header->width, lif->header->height,
                          lif->header->width, lif->header->height);
        return;
    }
    page = meshLIFsPage(filePrefix, texName);

    //twiddle the original width, height onto the poly
    width  = bitToExp2(page->width);
    height = bitToExp2(page->height);
    width <<= 4;
    poly->reserved[0] = 0;
    poly->reserved[1] = width | height;

    meshMarkupPolygon(poly, lifShortName(texName),
                      page->width, page->height,
                      lif->header->width, lif->header->height);

    for (index = 0; index < 3; index++)
    {
        meshFixupSingleUV(page, lif, s[index], t[index], poly->uv[index][0], poly->uv[index][1]);
    }

#ifdef PAC_Verbosity
    fprintf(stderr, "%0.3f %0.3f -> %0.3f %0.3f [%s]\n%0.3f %0.3f -> %0.3f %0.3f\n%0.3f %0.3f -> %0.3f %0.3f\n",
            poly->uv[0][0], poly->uv[0][1], s[0], t[0],
            lifShortName(texName),
            poly->uv[1][0], poly->uv[1][1], s[1], t[1],
            poly->uv[2][0], poly->uv[2][1], s[2], t[2]);
#endif

    for (index = 0; index < 3; index++)
    {
        poly->uv[index][0] = s[index];
        poly->uv[index][1] = t[index];
    }
}

llelement* texlist = NULL;
llfileheader llheader;

void meshLoadLIFList(void)
{
    char* dir;
    char* stringBlock;
    char  fullname[128];
    FILE* in;
    llfileheader header;

    dir = getenv("HW_Data");
    if (dir == NULL)
    {
        dir = getenv("HW_DATA");
        if (dir == NULL)
        {
            texlist = NULL;
            return;
        }
    }

    strcpy(fullname, dir);
    strcat(fullname, "\\textures.ll");
    in = fopen(fullname, "rb");
    if (in == NULL)
    {
        texlist = NULL;
        return;
    }

    fread(&header, 1, sizeof(llfileheader), in);
    memcpy(&llheader, &header, sizeof(llfileheader));

    texlist = (llelement*)malloc(header.totalLength);
    stringBlock = (char*)texlist + header.nElements * sizeof(llelement);
    fread(texlist, 1, header.totalLength, in);

    for (sdword i = 0; i < header.nElements; i++)
    {
        texlist[i].textureName += (udword)stringBlock;
        texlist[i].imageCRC = crc32ComputeString(texlist[i].textureName);
    }
#if 0
    if (header.sharingLength != 0)
    {
        char* sharingBlock = (char*)stringBlock + header.stringLength;
        for (sdword i = 0; i < header.nElements; i++)
        {
            if (texlist[i].nShared != 0)
            {
                (udword)texlist[i].sharedTo += (udword)sharingBlock;
            }
        }
    }
#endif
    fclose(in);
}

bool meshIsSharedLIF(char* texName)
{
    sdword index;
    char   name[256];
    crc32  crc;

    if (texlist == NULL)
    {
        return false;
    }

    strcpy(name, rawPrefix);
    strcat(name, texName);

    crc = crc32ComputeString(name);

    for (index = 0; index < llheader.nElements; index++)
    {
        if (texlist[index].textureName != NULL)
        {
#if LIF_CRC
            if (crc == texlist[index].imageCRC)
#else
            if (_stricmp(texlist[index].textureName, name) == 0)
#endif
            {
                if ((sdword)texlist[index].sharedFrom >= 0 ||
                    (sdword)texlist[index].sharedTo >= 0)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool meshIsSharedFromLIF(char* texName)
{
    sdword index;
    char   name[256];
    crc32  crc;

    if (texlist == NULL)
    {
        return false;
    }

    strcpy(name, rawPrefix);
    strcat(name, texName);

    crc = crc32ComputeString(name);

    for (index = 0; index < llheader.nElements; index++)
    {
        if (texlist[index].textureName != NULL)
        {
#if LIF_CRC
            if (crc == texlist[index].imageCRC)
#else
            if (_stricmp(texlist[index].textureName, name) == 0)
#endif
            {
                if ((sdword)texlist[index].sharedFrom >= 0)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

FILE* sharedhandle = NULL;

void meshOpenSharedMIF(char* partialPath)
{
    char path[1024];

    if (sharedhandle != NULL)
    {
        return;
    }

    strcpy(path, "datasrc\\");
    strcat(path, partialPath);
    strcat(path, "sharedtex.mif");

    sharedhandle = fopen(path, "wt");
}

void meshWriteSharedMIF(char* texName)
{
    if (sharedhandle != NULL)
    {
        fprintf(sharedhandle, "%s.lif\n", texName);
    }
}

void meshCloseSharedMIF(void)
{
    if (sharedhandle != NULL)
    {
        fclose(sharedhandle);
        sharedhandle = NULL;
    }
}

void meshQueueUnpackables(char* filePrefix, char* fileName)
{
    char fullName[1024];

    nopacklist.erase(nopacklist.begin(), nopacklist.end());

    strcpy(fullName, filePrefix);
    strcat(fullName, fileName);

    sdword fileSize = fileSizeGet(fullName);
    if (fileSize < sizeof(GeoFileHeader))
    {
        return;
    }

    ubyte* fileData = new ubyte[fileSize];
    if (!fileLoad(fullName, fileData))
    {
        delete [] fileData;
        return;
    }

    GeoFileHeader* header = (GeoFileHeader*)fileData;

    for (sdword index = 0; index < header->nPolygonObjects; index++)
    {
        polygonobject* object = (polygonobject*)(fileData + sizeof(GeoFileHeader) + index * sizeof(polygonobject));

        polyentry* polyList = (polyentry*)((udword)fileData + (udword)object->pPolygonList);

        polyentry* poly = polyList;

        for (sdword iPoly = 0; iPoly < object->nPolygons; iPoly++, poly++)
        {
            materialentry* mat = (materialentry*)(fileData + header->oLocalMaterials);
            mat += poly->iMaterial;
            if (mat->texture != 0)
            {
                char* texName = (char*)(fileData + mat->texture);
                liflist_t* lif = meshFindNamedLIF(filePrefix, texName);
                if (lif == NULL)
                {
                    continue;
                }

			    bool obo = false;

			    if (index > 0) obo = true;
                for (sdword uvindex = 0; uvindex < 3; uvindex++)
                {
                    if (poly->uv[index][0] < -1.0f || poly->uv[index][0] > 1.0f) obo = true;
                    if (poly->uv[index][1] < -1.0f || poly->uv[index][1] > 1.0f) obo = true;
                }

                if (meshIsSharedLIF(texName))
                {
                    obo = true;
                    if (meshIsSharedFromLIF(texName))
                    {
                        meshWriteSharedMIF(texName);
                    }
                }
                if (obo)
                {
                    nopacklist.push_back(*lif);
                    notpackedlist.push_back(*lif);

                    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
                    {
                        if (&(*i) == lif)
                        {
                            liflist.erase(i);
                            break;
                        }
                    }
                }
            }
        }
    }

    sdword notPacked = 0;
    for (list_lif::iterator i = nopacklist.begin(); i != nopacklist.end(); ++i)
    {
#ifdef PAC_Verbosity
        fprintf(stderr, "omit %s\n", lifShortName((*i).filename));
#endif
        notPacked++;
    }
#ifdef PAC_Verbosity
    if (notPacked > 0)
    {
        fprintf(stderr, "%d omitted, %d remain\n", notPacked, liflist.size());
    }
#endif
}

int meshVertSortCompare(const void* p0, const void* p1)
{
    uword v0, v1;

    v0 = (uword)p0;
    v1 = (uword)p1;
    if (v0 > v1)
    {
        return 1;
    }
    else if (v0 < v1)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

#define REAL_EPSILON 0.05f

bool approxEqual(real32 a, real32 b)
{
    real32 diff;

    diff = a - b;
    if (fabs(diff) < REAL_EPSILON)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool vecEqual(vertexentry* v0, vertexentry* v1)
{
    if (approxEqual(v0->x, v1->x) &&
        approxEqual(v0->y, v1->y) &&
        approxEqual(v0->z, v1->z))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool meshPolysRoughlyEqual(polyentry* poly0, polyentry* poly1, vertexentry* vertexList)
{
    vertexentry* v0[3];
    vertexentry* v1[3];

    v0[0] = vertexList + poly0->iV0;
    v0[1] = vertexList + poly0->iV1;
    v0[2] = vertexList + poly0->iV2;
    
    v1[0] = vertexList + poly1->iV0;
    v1[1] = vertexList + poly1->iV1;
    v1[2] = vertexList + poly1->iV2;

    //identical winding
    if (vecEqual(v0[0], v1[0]) &&
        vecEqual(v0[1], v1[1]) &&
        vecEqual(v0[2], v1[2]))
    {
        return true;
    }

    //reverse winding
    if (vecEqual(v0[0], v1[2]) &&
        vecEqual(v0[1], v1[1]) &&
        vecEqual(v0[2], v1[0]))
    {
        return true;
    }

    //... and so on
    if (vecEqual(v0[0], v1[1]) &&
        vecEqual(v0[1], v1[2]) &&
        vecEqual(v0[2], v1[0]))
    {
        return true;
    }

    if (vecEqual(v0[0], v1[1]) &&
        vecEqual(v0[1], v1[0]) &&
        vecEqual(v0[2], v1[2]))
    {
        return true;
    }

    if (vecEqual(v0[0], v1[2]) &&
        vecEqual(v0[1], v1[0]) &&
        vecEqual(v0[2], v1[1]))
    {
        return true;
    }

    if (vecEqual(v0[0], v1[0]) &&
        vecEqual(v0[1], v1[2]) &&
        vecEqual(v0[2], v1[1]))
    {
        return true;
    }

    return false;
}

void meshPolyAnalysis(char* filePrefix, char* fileName)
{
    char fullName[1024];
    sdword fileSize;
    ubyte* fileData;
    GeoFileHeader* header;
    polygonobject* object;
    sdword index, iPoly, jPoly;
    sdword nPolygons;
    polyentry* polyList;
    vertexentry* vertexList;
    polyentry* poly;
    polyentry* jpoly;
    sdword hits;

    strcpy(fullName, filePrefix);
    strcat(fullName, fileName);

    fileSize = fileSizeGet(fullName);
    if (fileSize < sizeof(GeoFileHeader))
    {
        printf("fileSize error in %s\n", fullName);
        return;
    }

    fileData = new ubyte[fileSize];
    if (!fileLoad(fullName, fileData))
    {
        printf("couldn't load mesh %s\n", fullName);
        delete [] fileData;
        return;
    }

    header = (GeoFileHeader*)fileData;

    for (index = 0; index < header->nPolygonObjects; index++)
    {
        object = (polygonobject*)(fileData + sizeof(GeoFileHeader) + index * sizeof(polygonobject));

        polyList = (polyentry*)((udword)fileData + (udword)object->pPolygonList);
        vertexList = (vertexentry*)((udword)fileData + (udword)object->pVertexList);

        nPolygons = object->nPolygons;

        poly = polyList;

        for (iPoly = 0; iPoly < nPolygons; iPoly++, poly++)
        {
            //determine whether another poly w/ same vertices exists
            hits = 0;
            jpoly = polyList + iPoly;
            for (jPoly = iPoly; jPoly < nPolygons; jPoly++, jpoly++)
            {
                if (jPoly == iPoly)
                {
                    continue;
                }

                if (meshPolysRoughlyEqual(poly, jpoly, vertexList))
                {
                    printf("LOD%c polys %d.%d / %d roughly equal (%d.%d)\n",
                           filePrefix[strlen(filePrefix) - 2],
                           iPoly, jPoly, object->nPolygons,
                           poly->iMaterial, jpoly->iMaterial);
                }
            }
        }
    }

    delete [] fileData;
}

bool meshPerformFixup(char* filePrefix, char* fileName)
{
    char fullName[1024], pagedName[1024], str[1024];
    sdword numPolys, numFixups;
    sdword index, iPoly;
    sdword* fixuplist;
    real32* uvlist;
    real32  s0, t0, s1, t1, s2, t2;
    FILE* uv;

    sdword fileSize;
    ubyte* fileData;
    GeoFileHeader* header;

    strcpy(fullName, filePrefix);
    strcat(fullName, fileName);
    meshPagedName(pagedName, fullName);

    fprintf(stderr, "meshPerformFixup: %s\n", pagedName);

    fileSize = fileSizeGet(pagedName);
    if (fileSize < sizeof(GeoFileHeader))
    {
        return false;
    }

    fileData = new ubyte[fileSize];
    if (!fileLoad(pagedName, fileData))
    {
        fprintf(stderr, "couldn't load %s\n", pagedName);
        delete [] fileData;
        return false;
    }

    header = (GeoFileHeader*)fileData;

//    uv = fopen("uv.txt", "rt");
    uv = fopen("\\homeworld\\remote\\uv_r1carrier1.txt", "rt");
    if (uv == NULL)
    {
        fprintf(stderr, "meshPerformFixup: can't find uv.txt\n");
        return false;
    }
    //numPolys
    fgets(str, 1023, uv);
    sscanf(str, "%d", &numPolys);
    uvlist = new real32[8 * numPolys];
    memset(uvlist, 0, 8 * numPolys * sizeof(real32));
    //num uv fixups
    fgets(str, 1023, uv);
    sscanf(str, "%d", &numFixups);
    fixuplist = new sdword[numFixups];
    memset(fixuplist, 0, numFixups * sizeof(sdword));
    //fixup list
    for (index = 0; index < numFixups; index++)
    {
        fgets(str, 1023, uv);
        sscanf(str, "%d", &iPoly);
        fixuplist[index] = iPoly;
    }
    //read poly list
    for (index = 0; index < numPolys; index++)
    {
        fgets(str, 1023, uv);
        sscanf(str, "%d [%f %f] [%f %f] [%f %f]",
               &iPoly,
               &s0, &t0,
               &s1, &t1,
               &s2, &t2);
        uvlist[8*index + 0] = s0;
        uvlist[8*index + 1] = t0;
        uvlist[8*index + 2] = s1;
        uvlist[8*index + 3] = t1;
        uvlist[8*index + 4] = s2;
        uvlist[8*index + 5] = t2;
    }
    //close file
    fclose(uv);

    //foreach polygon in chosen object
    //    use uvlist's uv's if in fixuplist
    for (index = 0; index < header->nPolygonObjects; index++)
    {
        polygonobject* object = (polygonobject*)(fileData + sizeof(GeoFileHeader) + index * sizeof(polygonobject));
        polyentry* polyList = (polyentry*)((udword)fileData + (udword)object->pPolygonList);

        if (object->nPolygons != numPolys)
        {
            //polycount doesn't match what we expect
            continue;
        }

        polyentry* poly = polyList;
        for (iPoly = 0; iPoly < object->nPolygons; iPoly++, poly++)
        {
#if 0
            fprintf(stderr, "%d [%0.20f %0.20f] [%0.20f %0.20f] [%0.20f %0.20f]\n",
                    iPoly,
                    poly->uv[0][0], poly->uv[0][1],
                    poly->uv[1][0], poly->uv[1][1],
                    poly->uv[2][0], poly->uv[2][1]);
        }
#else
            //see if in fixuplist
            sdword fixupIndex;
            for (fixupIndex = 0; fixupIndex < numFixups; fixupIndex++)
            {
                if (iPoly == fixuplist[fixupIndex])
                {
                    //fixup this poly
                    fprintf(stderr, "fixing up poly %d\n", iPoly);
                    fprintf(stderr, "  was [%f %f] [%f %f] [%f %f]\n",
                            poly->uv[0][0], poly->uv[0][1],
                            poly->uv[1][0], poly->uv[1][1],
                            poly->uv[2][0], poly->uv[2][1]);
                    real32* puv = uvlist + 8*fixuplist[fixupIndex];
                    fprintf(stderr, "  is  [%f %f] [%f %f] [%f %f]\n",
                            puv[0], puv[1], puv[2], puv[3], puv[4], puv[5]);
                    poly->uv[0][0] = puv[0];
                    poly->uv[0][1] = puv[1];
                    poly->uv[1][0] = puv[2];
                    poly->uv[1][1] = puv[3];
                    poly->uv[2][0] = puv[4];
                    poly->uv[2][1] = puv[5];
                }
            }
        }
#endif

        //only fixup 1 object per mesh
        break;
    }

    //output new version of mesh
#if 1
    if (!fileSave(pagedName, fileData, fileSize))
    {
        fprintf(stderr, "couldn't write %s: is it checked out?\n", pagedName);
    }
#endif

    delete[] fileData;
    delete[] uvlist;
    delete[] fixuplist;

    return true;
}

bool meshResortPeo(char* filePrefix, char* fileName)
{
    sdword fileSize;
    ubyte* fileData;
    materialentry* mat;
    GeoFileHeader* header;
    polygonobject* object;
    polyentry* polyList;
    polyentry* poly;
    sdword index, iMaterial, iPoly;
    char fullName[1024], pagedName[1024];

    strcpy(fullName, filePrefix);
    strcat(fullName, fileName);
    meshPagedName(pagedName, fullName);

    fileSize = fileSizeGet(pagedName);
    if (fileSize < sizeof(GeoFileHeader))
    {
        return false;
    }

    fileData = new ubyte[fileSize];
    if (!fileLoad(pagedName, fileData))
    {
        delete [] fileData;
        return false;
    }

    header = (GeoFileHeader*)fileData;
    header->identifier[4] = '9';

    for (index = 0; index < header->nPolygonObjects; index++)
    {
        object = (polygonobject*)(fileData + sizeof(GeoFileHeader) + index * sizeof(polygonobject));
        polyList = (polyentry*)((udword)fileData + (udword)object->pPolygonList);

        polyentry* newPolyList = new polyentry[object->nPolygons];
        sdword newPolyIndex = 0;

        for (iMaterial = 0; iMaterial < (header->nLocalMaterials + header->nPublicMaterials); iMaterial++)
        {
            mat = (materialentry*)(fileData + header->oLocalMaterials);
            mat += iMaterial;

            poly = polyList;
            for (iPoly = 0; iPoly < object->nPolygons; iPoly++, poly++)
            {
                if (poly->iMaterial == iMaterial)
                {
                    memcpy(&newPolyList[newPolyIndex], &polyList[iPoly], sizeof(polyentry));
                    newPolyIndex++;
                }
            }
        }

        memcpy(polyList, newPolyList, newPolyIndex * sizeof(polyentry));
    }

    if (!fileSave(pagedName, fileData, fileSize))
    {
        fprintf(stderr, "couldn't write %s: is it checked out?\n", pagedName);
    }

    delete [] fileData;
    return true;
}

void meshExportPagedMesh(char* filePrefix, char* fileName)
{
    sdword numPages;
    char fullName[1024];
    char pagedName[1024];
    sdword fileSize;
    ubyte* fileData;
    materialentry* mat;
    GeoFileHeader* header;
    polygonobject* object;
    sdword index, iPoly;
    sdword nPolygons;
    polyentry* polyList;
    polyentry* poly;

    numPages = lifpagelist.size();

    strcpy(fullName, filePrefix);
    strcat(fullName, fileName);

    fileSize = fileSizeGet(fullName);
    if (fileSize < sizeof(GeoFileHeader))
    {
        printf("fileSize error in %s\n", fullName);
        return;
    }

    fileData = new ubyte[fileSize];
    if (!fileLoad(fullName, fileData))
    {
        printf("couldn't load mesh %s\n", fullName);
        delete [] fileData;
        return;
    }

    header = (GeoFileHeader*)fileData;
    //alter the identifier -> "RMF99ba"
    header->identifier[4] = '9';

    //fixup U,V
    for (index = 0; index < header->nPolygonObjects; index++)
    {
        object = (polygonobject*)(fileData + sizeof(GeoFileHeader) + index * sizeof(polygonobject));

        polyList = (polyentry*)((udword)fileData + (udword)object->pPolygonList);

        nPolygons = object->nPolygons;

        poly = polyList;

        for (iPoly = 0; iPoly < nPolygons; iPoly++, poly++)
        {
            //if material_has_texture
            //  fixup poly's u,v coords
            mat = (materialentry*)(fileData + header->oLocalMaterials);
            mat += poly->iMaterial;
            if (mat->texture != 0)
            {
                char* texName = (char*)(fileData + mat->texture);
                meshFixupUV(poly, texName, filePrefix);
            }
        }
    }

    //fixup materials
    mat = (materialentry*)(fileData + header->oLocalMaterials);
    for (index = 0; index < (header->nLocalMaterials + header->nPublicMaterials); index++, mat++)
    {
        if (mat->texture == 0)
        {
            continue;
        }

        udword idx = meshLIFsPageIndex(filePrefix, (char*)(fileData + mat->texture));
        if (idx != TR_InvalidHandle)
        {
            mat->texture = fileSize + 6 * idx;
        }
    }

    //remove duplicate materials
//    meshRemoveMatDups(..);

    //resort polygons to minimize state changes
//    meshResortPolys(..);

    header->fileSize += 6 * numPages;
    header->localSize += 6 * numPages;

    strcpy(fullName, filePrefix);
    strcat(fullName, fileName);
    meshPagedName(pagedName, fullName);
    FILE* out = fopen(pagedName, "wb");
    if (out == NULL)
    {
        if (!quietMode)
        {
            printf("couldn't create %s\n", pagedName);
        }
        delete [] fileData;
        return;
    }

    fwrite(fileData, 1, fileSize, out);

    for (index = 0; index < numPages; index++)
    {
        char pageName[] = "pagen";
        pageName[4] = '0' + index;
        fwrite(pageName, 1, 6, out);
    }

    fclose(out);

    delete [] fileData;

    meshResortPeo(filePrefix, fileName);
}
