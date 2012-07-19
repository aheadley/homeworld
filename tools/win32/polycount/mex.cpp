//
// Mex.cpp
// mesh-related functions
//

#include <stdio.h>
#include <string.h>
#include "mex.h"
#include "file.h"

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

sdword meshCountPolygons(meshdata* mesh)
{
    sdword i, total;
    polygonobject* object;

    for (i = total = 0; i < mesh->nPolygonObjects; i++)
    {
        object = &mesh->object[i];
        total += object->nPolygons;
    }
    
    return total;
}
