/*=============================================================================
    Name    : LOD.c
    Purpose : Service functions for level-of-detail

    Created 7/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "Types.h"
#include "StatScript.h"
#include "Memory.h"
#include "SpaceObj.h"
#include "Debug.h"
#include "File.h"
#include "Key.h"
#include "texreg.h"
#include "LOD.h"

extern meshdata *defaultmesh;          // hack for now.  remove later when defaults no longer needed

/*=============================================================================
    Data:
=============================================================================*/
//script-parsing function for LOD files
static void lodTypeRead(char *directory,char *field,void *dataToFillIn);
static void lodMeshFileLoad(char *directory,char *field,void *dataToFillIn);
static void lodSpriteFileRead(char *directory,char *field,void *dataToFillIn);
static void lodColorScalarRead(char *directory,char *field,void *dataToFillIn);
static lodinfo lodStaticInfo;
scriptStructEntry lodScriptTable[] =
{
    { "pointColor",   scriptSetRGBCB,    (udword)(&lodStaticInfo.pointColor),     (udword)(&lodStaticInfo)},
    { "type0",        lodTypeRead,       (udword)(&lodStaticInfo.level[0]),       (udword)(&lodStaticInfo)},
    { "type1",        lodTypeRead,       (udword)(&lodStaticInfo.level[1]),       (udword)(&lodStaticInfo)},
    { "type2",        lodTypeRead,       (udword)(&lodStaticInfo.level[2]),       (udword)(&lodStaticInfo)},
    { "type3",        lodTypeRead,       (udword)(&lodStaticInfo.level[3]),       (udword)(&lodStaticInfo)},
    { "type4",        lodTypeRead,       (udword)(&lodStaticInfo.level[4]),       (udword)(&lodStaticInfo)},
    { "type5",        lodTypeRead,       (udword)(&lodStaticInfo.level[5]),       (udword)(&lodStaticInfo)},
    { "mOn0",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[0].mOn),   (udword)(&lodStaticInfo)},
    { "mOn1",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[1].mOn),   (udword)(&lodStaticInfo)},
    { "mOn2",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[2].mOn),   (udword)(&lodStaticInfo)},
    { "mOn3",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[3].mOn),   (udword)(&lodStaticInfo)},
    { "mOn4",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[4].mOn),   (udword)(&lodStaticInfo)},
    { "mOn5",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[5].mOn),   (udword)(&lodStaticInfo)},
    { "mOff0",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[0].mOff),  (udword)(&lodStaticInfo)},
    { "mOff1",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[1].mOff),  (udword)(&lodStaticInfo)},
    { "mOff2",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[2].mOff),  (udword)(&lodStaticInfo)},
    { "mOff3",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[3].mOff),  (udword)(&lodStaticInfo)},
    { "mOff4",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[4].mOff),  (udword)(&lodStaticInfo)},
    { "mOff5",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[5].mOff),  (udword)(&lodStaticInfo)},
    { "bOn0",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[0].bOn),   (udword)(&lodStaticInfo)},
    { "bOn1",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[1].bOn),   (udword)(&lodStaticInfo)},
    { "bOn2",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[2].bOn),   (udword)(&lodStaticInfo)},
    { "bOn3",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[3].bOn),   (udword)(&lodStaticInfo)},
    { "bOn4",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[4].bOn),   (udword)(&lodStaticInfo)},
    { "bOn5",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[5].bOn),   (udword)(&lodStaticInfo)},
    { "bOff0",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[0].bOff),  (udword)(&lodStaticInfo)},
    { "bOff1",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[1].bOff),  (udword)(&lodStaticInfo)},
    { "bOff2",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[2].bOff),  (udword)(&lodStaticInfo)},
    { "bOff3",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[3].bOff),  (udword)(&lodStaticInfo)},
    { "bOff4",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[4].bOff),  (udword)(&lodStaticInfo)},
    { "bOff5",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[5].bOff),  (udword)(&lodStaticInfo)},
    { "pMeshFile0",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[0].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile1",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[1].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile2",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[2].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile3",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[3].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile4",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[4].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile5",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[5].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile0", lodSpriteFileRead, (udword)(&lodStaticInfo.level[0].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile1", lodSpriteFileRead, (udword)(&lodStaticInfo.level[1].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile2", lodSpriteFileRead, (udword)(&lodStaticInfo.level[2].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile3", lodSpriteFileRead, (udword)(&lodStaticInfo.level[3].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile4", lodSpriteFileRead, (udword)(&lodStaticInfo.level[4].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile5", lodSpriteFileRead, (udword)(&lodStaticInfo.level[5].pData), (udword)(&lodStaticInfo)},
    { "baseScalar",   lodColorScalarRead,0,            0},
    { "stripeScalar", lodColorScalarRead,2,          1},
    { NULL,NULL,0,0 }
};

//scaling factor for the LOD's
real32 lodScaleFactor = LOD_ScaleFactor;

//fixed version of above factor for debugging reasons
#if LOD_SCALE_DEBUG
real32 lodDebugScaleFactor = 0.0f;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : lodTableReadScript
    Description : Load in and allocate a level-of-detail table from a script
                    file.
    Inputs      : fileName - name of script file to read from
    Outputs     : Allocates a LOD table and fills it in.
    Return      : newly allocated LOD table.
----------------------------------------------------------------------------*/
lodmaxinfo lodMaxInfo;
lodinfo *lodTableReadScript(char *directory, char *fileName)
{
    lodinfo *info;
    sdword index;

    for (index = 0; index < LOD_NumberLevels; index++)
    {                                                       //initialize the lodmaxinfo structure
        lodMaxInfo.level[index].flags = LT_Invalid;
        lodMaxInfo.level[index].pData = NULL;
    }

    trBaseColorScalar = trStripeColorScalar = 0.0f;
    scriptSetStruct(directory, fileName, lodScriptTable, (ubyte *)&lodMaxInfo);//read the script file
    trBaseColorScalar = trStripeColorScalar = 0.0f;

    for (index = 0; index < LOD_NumberLevels; index++)
    {
        if ((lodMaxInfo.level[index].flags & LM_LODType) == LT_Invalid)
        {
            break;
        }
        if ((lodMaxInfo.level[index].flags & LM_LODType) == LT_Mesh)
        {                                                   //if it's a mesh
            lodMaxInfo.level[index].hBindings =                //create a bindings list
                meshBindingListCreate((meshdata *)lodMaxInfo.level[index].pData);
        }
        else
        {
            lodMaxInfo.level[index].hBindings = NULL;
        }
    }
    info = memAlloc(lodTableSize(index), "LOD Table", NonVolatile); //allocate the table
    memcpy(info, &lodMaxInfo, lodTableSize(index));            //and copy over the max table
    info->nLevels = index;                                  //store number of levels
#if LOD_AUTO_SAVE
    {
        char partialName[PATH_MAX];
        char *fullName;
        sprintf(partialName, "%s%s", directory, fileName);
        fullName = filePathPrepend(partialName, 0);
        info->fileName = memStringDupe(fullName);           //store full path to file for saving
    }
#endif
    return(info);
}

/*=============================================================================
    Callback functions for script reading
=============================================================================*/
//set correct LOD type
static void lodColorScalarRead(char *directory,char *field,void *dataToFillIn)
{
    real32 data;

    sscanf(field, "%f", &data);
    dbgAssert(data >= 0.0f && data <= 10.0f);
    if (data == 0.0f)
    {                                                       //0 is regarded as 'no change', hence 1.0
        data = 1.0f;
    }
    if (dataToFillIn == (void *)(&lodMaxInfo))
    {
        trBaseColorScalar = data;
    }
    else
    {
        trStripeColorScalar = data;
    }
}
//set correct LOD type
static void lodTypeRead(char *directory,char *field,void *dataToFillIn)
{                                                           //set correct LOD type
    lod *dest;
    unsigned int i;

    dest = (lod *)dataToFillIn;
#if LOD_AUTO_SAVE
    dest->baseScalar = (trBaseColorScalar == 0.0f) ? 1.0f : trBaseColorScalar;
    dest->stripeScalar = (trStripeColorScalar == 0.0f) ? 1.0f : trStripeColorScalar;
#endif
    /*_strupr(field);*/
    for (i = 0; (field[i] = toupper(field[i])); i++) { }

    if (strstr(field, "INVALID"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_Invalid);
    }
    else if (strstr(field, "MESH"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_Mesh);
    }
    else if (strstr(field, "TINYSPRITE"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_TinySprite);
    }
    else if (strstr(field, "SUBPIXEL"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_SubPixel);
    }
    else if (strstr(field, "FUNCTION"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_Function);
    }
    else if (strstr(field, "NULL"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_NULL);
    }
#if LOD_ERROR_CHECKING
    else
    {
        dbgFatalf(DBG_Loc, "Invalid LOD type: '%s'", field);
    }
#endif
}
//load in a mesh
static void lodMeshFileLoad(char *directory,char *field,void *dataToFillIn)
{
    char fullfilename[80];
#if LOD_VERBOSE_LEVEL >= 1
//    dbgMessagef("\nlodMeshFileLoad: %s", field);
#endif
    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,field);
    }
    else
    {
        strcpy(fullfilename,field);
    }

    if (fileExists(fullfilename,0) || meshPagedVersionExists(fullfilename))
    {
        *((meshdata **)dataToFillIn) = meshLoad(fullfilename);
    }
    else
    {
        *((meshdata **)dataToFillIn) = defaultmesh;
    }
}
//load in a tiny sprite file
static void lodSpriteFileRead(char *directory,char *field,void *dataToFillIn)
{
#if LOD_VERBOSE_LEVEL >= 1
    dbgMessagef("\nlodSpriteFileRead: %s", field);
#endif
    dbgFatal(DBG_Loc, "Can't load sprite images yet!");
}

/*-----------------------------------------------------------------------------
    Name        : lodLevelGet
    Description : Get level of detail for specified ship
    Inputs      : spaceObj - space object to get
                  camera, ship - location of ship and camera, respectively
    Outputs     : updates the currentLOD of the specified space object.
    Return      : lod structure for current level of detail
----------------------------------------------------------------------------*/
#if LOD_PRINT_DISTANCE
sdword rndLOD = 0;
sdword lodTuningMode = FALSE;
sdword lodDrawingMode = FALSE;
#endif
lod *lodLevelGet(void *spaceObj, vector *camera, vector *ship)
{
    real32 distance;
    SpaceObj *obj = (SpaceObj *)spaceObj;
    lodinfo *info = obj->staticinfo->staticheader.LOD;

    dbgAssert(info != NULL);                                //verify the LOD table exists

    vecSub(obj->cameraDistanceVector,*camera,*ship);
    obj->cameraDistanceSquared = distance = vecMagnitudeSquared(obj->cameraDistanceVector);

#if LOD_SCALE_DEBUG
    if (lodDebugScaleFactor != 0.0f)
    {
        lodScaleFactor = lodDebugScaleFactor;
    }
#endif
    if (distance > info->level[obj->currentLOD].bOff * lodScaleFactor)
    {                                                       //if drop a level of detail
        do
        {
            obj->currentLOD++;                                  //go to lower level
            if (obj->currentLOD >= info->nLevels)
            {
                obj->currentLOD = info->nLevels-1;
                break;
            }
        }
        while (distance > info->level[obj->currentLOD].bOff * lodScaleFactor);
    }
    else
    {
        while (obj->currentLOD > 0 && distance < info->level[obj->currentLOD - 1].bOn * lodScaleFactor)
        {                                                   //if go higher level of detail
            obj->currentLOD--;                              //go to higher level
        }
    }
    dbgAssert(obj->currentLOD >= 0);
    dbgAssert(obj->currentLOD < info->nLevels);             //verify we are within the available levels of detail
#if LOD_PRINT_DISTANCE
    if (keyIsStuck(WKEY))
    {
        keyClearSticky(WKEY);
        lodScaleFactor *= 0.99f;
        dbgMessagef("\nlodScaleFactor = %.3f", lodScaleFactor);
    }
    if (keyIsStuck(OKEY))
    {
        keyClearSticky(OKEY);
        lodScaleFactor *= 1.01f;
        dbgMessagef("\nlodScaleFactor = %.3f", lodScaleFactor);
    }
    if (lodTuningMode)
    {
        obj->currentLOD = min(rndLOD, info->nLevels - 1);
        return(&info->level[min(rndLOD, info->nLevels - 1)]);
    }
#endif
    return(&info->level[obj->currentLOD]);                  //return pointer to lod structure
}

/*-----------------------------------------------------------------------------
    Name        : lodPanicLevelGet
    Description : Get level of detail for specified ship
    Inputs      : spaceObj - space object to get
                  camera, ship - location of ship and camera, respectively
    Outputs     : updates the currentLOD of the specified space object.
    Return      : lod structure for current level of detail
----------------------------------------------------------------------------*/
lod* lodPanicLevelGet(void* spaceObj, vector* camera, vector* ship)
{
    SpaceObj* obj = (SpaceObj*)spaceObj;
    lodinfo* info = obj->staticinfo->staticheader.LOD;

    if (obj->currentLOD == 3)
    {
        obj->currentLOD++;
        if (obj->currentLOD >= info->nLevels)
        {
            obj->currentLOD--;
        }
        if ((info->level[obj->currentLOD].flags & LM_LODType) != LT_Mesh)
        {
            obj->currentLOD--;
        }
    }

    return(&info->level[obj->currentLOD]);
}

/*-----------------------------------------------------------------------------
    Name        : lodFree
    Description : frees LOD
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void lodFree(lodinfo *LOD)
{
    sdword i;
    void *pData;

    for (i=0;i<LOD->nLevels;i++)
    {
        pData = LOD->level[i].pData;
        switch (LOD->level[i].flags)
        {
            case LT_Mesh:
                if ((pData != defaultmesh) && (pData != NULL))
                {
                    meshFree((meshdata *)pData);
                }
                dbgAssert(LOD->level[i].hBindings != NULL);
                meshBindingListDelete(LOD->level[i].hBindings);
                break;

            default:        // later add other stuff you want to free such as sprites Luke.
                break;
        }
    }

    memFree(LOD);
}

/*-----------------------------------------------------------------------------
    Name        : lodAllMeshesRecolorize
    Description : Recolorizes all meshes in a particular LOD to reflect changes
                    in team color schemes.
    Inputs      : LOD - what LOD to recolorize;
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void lodAllMeshesRecolorize(lodinfo *LOD)
{
    sdword index;

    for (index = 0; index < LOD->nLevels; index++)
    {                                                       //for each level in this LOD
        if ((LOD->level[index].flags & LM_LODType) == LT_Mesh)
        {                                                   //if it's a mesh type
            meshRecolorize((meshdata *)LOD->level[index].pData);//recolorize it
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : lodHierarchySizeCompute
    Description : Compute the number of objects in all meshes in an LOD
    Inputs      : LOD - LOD to count the objects in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword lodHierarchySizeCompute(lodinfo *LOD)
{
    sdword index, size;
    lod *level;

    for (index = size = 0; index < LOD->nLevels; index++)
    {                                                       //for all levels of detail
        level = &LOD->level[index];
        if ((level->flags & LM_LODType) != LT_Mesh)
        {
            continue;
        }
        size += ((meshdata *)level->pData)->nPolygonObjects;
    }
    return(size);
}


/*-----------------------------------------------------------------------------
    Name        : lodAutoSave
    Description : Save out an LOD file after internal tuning.
    Inputs      : LOD - what LOD to save
    Outputs     :
    Return      : 0 (success) or ERROR
    Note        : Prints a warning if the .LOD file cannot be opened for writing.
----------------------------------------------------------------------------*/
#if LOD_AUTO_SAVE
char *lodTypeStrings[] = {"INVALID", "MESH", "TINYSPRITE", "SUBPIXEL", "FUNCTION", "NULL"};
sdword lodAutoSave(lodinfo *LOD)
{
    sdword level;
    FILE *fp;
    char *filePath;
    char *lodFileNameFull;
    real32 baseScalar = 1.0f, stripeScalar = 1.0f;

    lodFileNameFull = filePathPrepend(LOD->fileName, FF_UserSettingsPath);

    if (!fileMakeDestinationDirectory(lodFileNameFull))
    {
        dbgWarningf(
            DBG_Loc,
            "Cannot create destination directory for file '%s'.",
            LOD->fileName);
        return ERROR;
    }

    fp = fopen(lodFileNameFull, "wt");
    if (fp == NULL)
    {
        dbgWarningf(DBG_Loc, "Cannot open '%s' for writing - not checked out?", LOD->fileName);
        return(ERROR);
    }
    fprintf(fp, "[Auto-Saved LOD file]\n");
    fprintf(fp, "pointColor                  %d,%d,%d\n\n", colRed(LOD->pointColor), colGreen(LOD->pointColor), colBlue(LOD->pointColor));

    for (level = 0; level < LOD->nLevels; level++)
    {
        fprintf(fp, "\n");
        if (LOD->level[level].baseScalar != baseScalar)
        {
            baseScalar = LOD->level[level].baseScalar;
            fprintf(fp, "baseScalar                  %.2f\n", baseScalar == 0.0f ? 1.0f : baseScalar);
        }
        if (LOD->level[level].stripeScalar != stripeScalar)
        {
            stripeScalar = LOD->level[level].stripeScalar;
            fprintf(fp, "stripeScalar                %.2f\n", stripeScalar == 0.0f ? 1.0f : stripeScalar);
        }
        fprintf(fp, "type%d                       %s\n", level, lodTypeStrings[LOD->level[level].flags & LM_LODType]);
        if ((LOD->level[level].flags & LM_LODType) == LT_Mesh)
        {
#ifdef _WIN32
            filePath = strchr(((meshdata *)LOD->level[level].pData)->fileName, '\\') + 1;
#else
            filePath = strpbrk(((meshdata *)LOD->level[level].pData)->fileName, "\\/") + 1;
#endif
#if LOD_ERROR_CHECKING
            if (*filePath == 0)
            {
                dbgFatalf(DBG_Loc, "Could not find '\\' in '%s'", ((meshdata *)LOD->level[level].pData)->fileName);
            }
#endif
            fprintf(fp, "pMeshFile%d                  %s\n", level, filePath);
        }
        fprintf(fp, "mOn%d                        %.1f\n", level, LOD->level[level].mOn);
        fprintf(fp, "bOn%d                        %.0f\n", level, LOD->level[level].bOn);
        fprintf(fp, "mOff%d                       %.1f\n", level, LOD->level[level].mOff);
        fprintf(fp, "bOff%d                       %.0f\n", level, LOD->level[level].bOff);
    }
    fclose(fp);
    return(0);
}
#endif
