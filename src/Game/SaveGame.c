/*=============================================================================
    Name    : SaveGame.c
    Purpose : Code for saving and loading games (saves entire contents of universe, etc.)

    Created 98/07/12 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <limits.h>
#include "Types.h"
#include "LinkedList.h"
#include "SpaceObj.h"
#include "Blobs.h"
#include "ShipSelect.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "SinglePlayer.h"
#include "File.h"
#include "CameraCommand.h"
#include "Clouds.h"
#include "Star3d.h"
#include "SalCapCorvette.h"
#include "Tactics.h"
#include "ConsMgr.h"
#include "AIPlayer.h"
#include "SaveGame.h"
#include "Objectives.h"
#include "utility.h"
#include "LevelLoad.h"
#include "Damage.h"
#include "Tutor.h"
#include "TradeMgr.h"
#include "Ping.h"
#include "Teams.h"
#include "BTG.h"
#include "light.h"
#include "CommandWrap.h"
#include "Strings.h"
#include "Sensors.h"
#include "Randy.h"
#include "SoundEvent.h"
#include "MultiplayerGame.h"

void SaveConsMgr();
void LoadConsMgr();

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

GrowSelection SpaceObjRegistry;
GrowSelection BlobRegistry;
filehandle savefile = 0;
sdword savefilestatus = 0;

/*=============================================================================
    Private Function prototypes
=============================================================================*/

void RegisterAllBlobs(void);
void RegisterAllSpaceObjs(void);
void LoadAllBlobs();
void SaveAllBlobs();
void FixAllBlobs();
void SaveBlob(blob *tblob);
blob *LoadBlob(void);
void FixBlob(blob *tblob);
void SaveAllSpaceObjs();
void LoadAllSpaceObjs();
void FixAllSpaceObjs();
SpaceObj *LoadSpaceObj();
void FixSpaceObj(SpaceObj *obj);
void SaveSpaceObj(SpaceObj *obj);
void LoadUniverse();
void SaveUniverse();

typedef void (*ListAddCB)(LinkedList *list,SpaceObj *obj);

void SaveLinkedListOfSpaceObjs(LinkedList *list);
void LoadLinkedListOfSpaceObjs(LinkedList *list,ListAddCB listAddCB);
void JustLoadLinkedListOfSpaceObjs(LinkedList *list);
void FixLinkedListOfSpaceObjs(LinkedList *list,ListAddCB listAddCB);

extern sdword actorFlagsEnabled;

/*=============================================================================
    Functions:
=============================================================================*/

void SpaceObjRegistryInit()
{
    Node* objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        SpaceObj* spaceobj = (SpaceObj*)listGetStructOfNode(objnode);
        dmgStopEffect((Ship*)spaceobj, DMG_All);
        objnode = objnode->next;
    }
    growSelectInit(&SpaceObjRegistry);
}

void SpaceObjRegistryClose()
{
    growSelectClose(&SpaceObjRegistry);
}

sdword SpaceObjRegistryObjPresent(SpaceObj *obj)
{
    sdword i;
    sdword numObjs = SpaceObjRegistry.selection->numShips;

    for (i=0;i<numObjs;i++)
    {
        if (obj == (SpaceObj *)SpaceObjRegistry.selection->ShipPtr[i])
        {
            return i;
        }
    }

    return -1;
}

void SpaceObjRegistryRegister(SpaceObj *obj)
{
    sdword index = SpaceObjRegistryObjPresent(obj);
    if (index == -1)        // object not present, so add
    {
        growSelectAddShip(&SpaceObjRegistry,(Ship *)obj);
    }
}

void SpaceObjRegistryRegisterNoCheck(SpaceObj *obj)
{
    growSelectAddShip(&SpaceObjRegistry,(Ship *)obj);
}

sdword SpaceObjRegistryGetID(SpaceObj *obj)
{
    if (obj != NULL)
    {
        return SpaceObjRegistryObjPresent(obj);
    }
    else
    {
        return -1;
    }
}

SpaceObj *SpaceObjRegistryGetObj(sdword ID)
{
    if (ID == -1)
    {
        return NULL;
    }

    dbgAssert(ID < SpaceObjRegistry.selection->numShips);

    return (SpaceObj *)SpaceObjRegistry.selection->ShipPtr[ID];
}

Ship *SpaceObjRegistryGetShip(sdword ID)
{
    Ship *ship;

    if (ID == -1)
    {
        return NULL;
    }

    dbgAssert(ID < SpaceObjRegistry.selection->numShips);
    ship = SpaceObjRegistry.selection->ShipPtr[ID];
    dbgAssert(ship->objtype == OBJ_ShipType);
    return ship;
}

Resource *SpaceObjRegistryGetResource(sdword ID)
{
    Resource *resource;

    if (ID == -1)
    {
        return NULL;
    }

    dbgAssert(ID < SpaceObjRegistry.selection->numShips);
    resource = (Resource *)SpaceObjRegistry.selection->ShipPtr[ID];
    dbgAssert(resource->flags & SOF_Resource);
    return resource;
}

Bullet *SpaceObjRegistryGetBullet(sdword ID)
{
    Bullet *bullet;

    if (ID == -1)
    {
        return NULL;
    }

    dbgAssert(ID < SpaceObjRegistry.selection->numShips);
    bullet = (Bullet *)SpaceObjRegistry.selection->ShipPtr[ID];
    dbgAssert(bullet->objtype == OBJ_BulletType);
    return bullet;
}

TargetPtr SpaceObjRegistryGetTarget(sdword ID)
{
    TargetPtr target;

    if (ID == -1)
    {
        return NULL;
    }

    dbgAssert(ID < SpaceObjRegistry.selection->numShips);
    target = (TargetPtr)SpaceObjRegistry.selection->ShipPtr[ID];
    dbgAssert(target->flags & SOF_Targetable);
    return target;
}

void BlobRegistryInit()
{
    growSelectInit(&BlobRegistry);
}

void BlobRegistryClose()
{
    growSelectClose(&BlobRegistry);
}

sdword BlobRegistryBlobPresent(blob *tblob)
{
    sdword i;
    sdword numObjs = BlobRegistry.selection->numShips;

    for (i=0;i<numObjs;i++)
    {
        if (tblob == (blob *)BlobRegistry.selection->ShipPtr[i])
        {
            return i;
        }
    }

    return -1;
}

void BlobRegistryRegister(blob *tblob)
{
    dbgAssert(BlobRegistryBlobPresent(tblob) == -1);
    growSelectAddShip(&BlobRegistry,(Ship *)tblob);
}

sdword BlobRegistryGetID(blob *tblob)
{
    if (tblob == NULL)
    {
        return -1;
    }
    else
    {
        sdword ID = BlobRegistryBlobPresent(tblob);
        dbgAssert(ID != -1);
        return ID;
    }
}

sdword BlobRegistryGetIDNoBlobOkay(blob *tblob)
{
    if (tblob == NULL)
    {
        return -1;
    }
    else
    {
        sdword ID = BlobRegistryBlobPresent(tblob);
        return ID;
    }
}

blob *BlobRegistryGetBlob(sdword ID)
{
    if (ID == -1)
    {
        return NULL;
    }
    else
    {
        dbgAssert(ID >= 0);
        dbgAssert(ID < BlobRegistry.selection->numShips);
        return (blob *)BlobRegistry.selection->ShipPtr[ID];
    }
}

/*-----------------------------------------------------------------------------
    Name        : SaveThisChunk
    Description : saves thischunk
    Inputs      : thischunk
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SaveThisChunk(SaveChunk *thischunk)
{
    FILE *fp;
    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);

    if (fwrite(thischunk, sizeofSaveChunk(thischunk->contentsSize), 1, fp) != 1)
    {
        savefilestatus = 1;
    }
}

SaveChunk *CreateChunk(TypeOfSaveChunk type,sdword contentsSize,void *contents)
{
    SaveChunk *chunk = memAlloc(sizeofSaveChunk(contentsSize),"savechunk",Pyrophoric);

    chunk->type = type;
    chunk->contentsSize = contentsSize;

    dbgAssert(contentsSize >= 0);
    if ((contentsSize > 0) && (contents != NULL))
    {
        memcpy(chunkContents(chunk),contents,contentsSize);
    }

    return chunk;
}

SaveChunk *LoadNextChunk()
{
    SaveChunk readchunk;
    sdword num;
    SaveChunk *returnchunk;
    FILE *fp;

    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);

    num = fread(&readchunk,sizeof(SaveChunk),1,fp);
    dbgAssert(num != 0);

    dbgAssert(readchunk.contentsSize >= 0);

    returnchunk = memAlloc(sizeofSaveChunk(readchunk.contentsSize),"readchunk",Pyrophoric);

    memcpy(returnchunk,&readchunk,sizeof(SaveChunk));

    if (readchunk.contentsSize > 0)
    {
        num = fread(((ubyte *)returnchunk) + sizeof(SaveChunk),readchunk.contentsSize,1,fp);
        dbgAssert(num != 0);
    }

    return returnchunk;
}

SaveChunk *LoadNextChunkSafe()
{
    SaveChunk readchunk;
    sdword num;
    SaveChunk *returnchunk;
    FILE *fp;

    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);

    num = fread(&readchunk,sizeof(SaveChunk),1,fp);
    if (num == 0)
    {
        return NULL;
    }

    if (readchunk.contentsSize < 0)
    {
        return NULL;
    }

    returnchunk = memAlloc(sizeofSaveChunk(readchunk.contentsSize),"readchunk",Pyrophoric);

    memcpy(returnchunk,&readchunk,sizeof(SaveChunk));

    if (readchunk.contentsSize > 0)
    {
        num = fread(((ubyte *)returnchunk) + sizeof(SaveChunk),readchunk.contentsSize,1,fp);
        if (num == 0)
        {
            memFree(returnchunk);
            return NULL;
        }
    }

    return returnchunk;
}

void SaveInfoNumber(sdword info)
{
    InfoChunkContents infocontents;
    SaveChunk *chunk;

    infocontents.info = info;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));
    memFree(chunk);
}

sdword LoadInfoNumber()
{
    SaveChunk *chunk;
    sdword num;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(chunk))->info;
    memFree(chunk);

    return num;
}

bool LoadInfoNumberOptional(sdword *info)
{
    SaveChunk *chunk;

    chunk = LoadNextChunkSafe();
    if (chunk == NULL)
    {
        return FALSE;
    }

    VerifyChunk(chunk,INFO_CHUNK,sizeof(InfoChunkContents));

    *info = ((InfoChunkContents *)chunkContents(chunk))->info;
    memFree(chunk);

    return TRUE;
}

void SaveBackground(void)
{
    real32 theta = btgGetTheta();
    real32 phi = btgGetPhi();

    SaveInfoNumber(TreatAsUdword(theta));
    SaveInfoNumber(TreatAsUdword(phi));
    Save_String(btgLastBackground);
}

void LoadBackground(void)
{
    sdword datatheta = LoadInfoNumber();
    sdword dataphi = LoadInfoNumber();

    btgSetTheta(TreatAsReal32(datatheta));
    btgSetPhi(TreatAsReal32(dataphi));
    Load_StringToAddress(btgLastBackground);

    btgLoadTextures();
}

void SaveLighting(void)
{
    Save_String(lightCurrentLighting);
}

void LoadLighting(void)
{
    Load_StringToAddress(lightCurrentLighting);

    if (lightCurrentLighting[0])
    {
        char tmp_path[PATH_MAX];
        strcpy(tmp_path, lightCurrentLighting);
        lightParseHSF(tmp_path);
    }
    else
    {
        lightDefaultLightSet();
    }
}

void SaveVersionInfo(void)
{
    sdword version = SAVE_VERSION_NUMBER;
    FILE *fp;

    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);

    if (fwrite(&version, sizeof(sdword), 1, fp) != 1)
    {
        savefilestatus = 1;
    }
}

sdword LoadVersionInfo(void)
{
    sdword num;
    sdword version;
    FILE *fp;

    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);

    num = fread(&version,sizeof(sdword),1,fp);
    if (num == 0)
    {
        return VERIFYSAVEFILE_ERROROPENING;
    }

    if (version != SAVE_VERSION_NUMBER)
    {
        return VERIFYSAVEFILE_BADVERSION;
    }

    return VERIFYSAVEFILE_OK;
}

void SavePreGameInfo(void)
{
    sdword i;

    SaveInfoNumber(singlePlayerGame);
    SaveInfoNumber(musicEventCurrentTrack());
    SaveInfoNumber(universe.numPlayers);
    SaveStructureOfSize(&playerNames[0][0],MAX_MULTIPLAYER_PLAYERS*MAX_PERSONAL_NAME_LEN);
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        SaveInfoNumber(universe.players[i].race);
        SaveInfoNumber(teColorSchemes[i].textureColor.base);
        SaveInfoNumber(teColorSchemes[i].textureColor.detail);
    }

    if (singlePlayerGame)
    {
        SaveInfoNumber(singlePlayerGameInfo.currentMission);
    }
    else
    {
        SaveStructureOfSize(&tpGameCreated,sizeof(tpGameCreated));
    }

    if (!(tutorial==TUTORIAL_ONLY))
    {
        universeSaveEverythingNeeded();
    }
}

void LoadPreGameInfo(void)
{
    sdword i;
    sdword sp;

    sp = LoadInfoNumber();
    if (sp)
    {
        dbgAssert(singlePlayerGame);
    }
    else
    {
        dbgAssert(!singlePlayerGame);
    }

    SongNumber = LoadInfoNumber();
    numPlayers = (uword)LoadInfoNumber();
    LoadStructureOfSizeToAddress(&playerNames[0][0],MAX_MULTIPLAYER_PLAYERS*MAX_PERSONAL_NAME_LEN);
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        universe.players[i].race = LoadInfoNumber();
        teColorSchemes[i].textureColor.base = LoadInfoNumber();
        teColorSchemes[i].textureColor.detail = LoadInfoNumber();
    }

    if (sp)
    {
        singlePlayerGameInfo.currentMission = LoadInfoNumber();
    }
    else
    {
        LoadStructureOfSizeToAddress(&tpGameCreated,sizeof(tpGameCreated));
    }

    if (!(tutorial==TUTORIAL_ONLY))
    {
        universeLoadEverythingNeeded();
    }

    if ((!sp)&&
        (!bitTest(tpGameCreated.flag,MG_HarvestinEnabled)))
    {
        rmEnableShip(R1, ResourceCollector, FALSE);
        rmEnableShip(R2, ResourceCollector, FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : SaveGame
    Description :
    Inputs      :
    Outputs     :
    Return      : TRUE on success
----------------------------------------------------------------------------*/
bool SaveGame(char *filename)
{
    sdword i;

    savefile = fileOpen(filename, FF_WriteMode | FF_ReturnNULLOnFail | FF_UserSettingsPath);
    if (savefile == NULL)
    {
        return FALSE;
    }
    savefilestatus = 0;

    SaveVersionInfo();
    SavePreGameInfo();

    SpaceObjRegistryInit();
    BlobRegistryInit();

    RegisterAllSpaceObjs();
    RegisterAllBlobs();
    SaveAllSpaceObjs();
    SaveAllBlobs();
    SaveUniverse();
    SaveIDToPtrTable(&ShipIDToPtr);
    SaveIDToPtrTable(&ResourceIDToPtr);
    SaveIDToPtrTable(&DerelictIDToPtr);
    SaveIDToPtrTable(&MissileIDToPtr);
    nebSave_Nebula();
    SaveConsMgr();
    aiplayerSave();

    SaveSinglePlayerGame();

    if (singlePlayerGame)
    {
        kasSave();
        objectiveSave();
        tmSave();
        pingSave();
        Save_String(CurrentLevelName);
    }

    if( tutorial==TUTORIAL_ONLY )
    {
        tutSaveTutorialGame();
    }

    SaveMaxSelection(&selSelected);
    for (i=0;i<SEL_NumberHotKeyGroups;i++)
    {
        SaveMaxSelection(&selHotKeyGroup[i]);
    }

    SaveBackground();
    SaveLighting();

    smSave();
    ranSave();

    SaveInfoNumber(actorFlagsEnabled);
    SaveInfoNumber(smGhostMode);

    SaveConsMgrDetermOptional();                // added for V1.04 patch

    fileClose(savefile);
    savefile = 0;

    if (savefilestatus)
    {
        fileDelete(filename);
        savefilestatus = 0;
        return FALSE;
    }
    else
    {
        return TRUE;        // save successful
    }
}

/*-----------------------------------------------------------------------------
    Name        : VerifySaveFile
    Description : verifies filename is valid save game file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword VerifySaveFile(char *filename)
{
    sdword verify;

    savefile = fileOpen(filename, FF_ReturnNULLOnFail | FF_UserSettingsPath);
    if (savefile == NULL)
    {
        return VERIFYSAVEFILE_ERROROPENING;
    }
    verify = LoadVersionInfo();

    if (savefile)
    {
        fileClose(savefile);
        savefile = 0;
    }

    return verify;
}

/*-----------------------------------------------------------------------------
    Name        : PreLoadGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PreLoadGame(char *filename)
{
    sdword verify;

    savefile = fileOpen(filename, FF_UserSettingsPath);
    verify = LoadVersionInfo();
    if (verify != VERIFYSAVEFILE_OK)
    {
        dbgFatalf(DBG_Loc,"Error %d loading game %s",verify,filename);
        return;
    }
    LoadPreGameInfo();
}

/*-----------------------------------------------------------------------------
    Name        : LoadGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void LoadGame(char *filename)
{
    sdword i;

    dbgAssert(savefile);

    SpaceObjRegistryInit();
    BlobRegistryInit();

    LoadAllSpaceObjs();
    LoadAllBlobs();
    LoadUniverse();
    LoadIDToPtrTable(&ShipIDToPtr);
    LoadIDToPtrTable(&ResourceIDToPtr);
    LoadIDToPtrTable(&DerelictIDToPtr);
    LoadIDToPtrTable(&MissileIDToPtr);
    nebLoad_Nebula();
    LoadConsMgr();
    aiplayerLoad();

    LoadSinglePlayerGame();

    if (singlePlayerGame)
    {
        kasLoad();
        objectiveLoad();
        tmLoad();
        pingLoad();
        Load_StringToAddress(&CurrentLevelName[0]);
    }

    if(tutorial==TUTORIAL_ONLY)
        tutLoadTutorialGame();

    LoadMaxSelectionAndFix(&selSelected);
    for (i=0;i<SEL_NumberHotKeyGroups;i++)
    {
        LoadMaxSelectionAndFix(&selHotKeyGroup[i]);
    }

    LoadBackground();
    LoadLighting();

    smLoad();
    ranLoad();

    actorFlagsEnabled = LoadInfoNumber();
    smGhostMode = LoadInfoNumber();

    LoadConsMgrDetermOptional();            // optional, won't crash if not there, added for V1.04 patch

    FixAllSpaceObjs();
    FixAllBlobs();

    listInit(&universe.effectList);

    fileClose(savefile);
    savefile = 0;
}

/*=============================================================================
    Actual Save routines for various key data structures:
=============================================================================*/

void RegisterAllBlobs(void)
{
    Node *blobnode = universe.collBlobList.head;
    blob *thisBlob;

    // for each blob, check all collisions within that blob
    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);

        BlobRegistryRegister(thisBlob);

        blobnode = blobnode->next;
    }
}

void RegisterAllSpaceObjsInsideMe(ShipsInsideMe *shipsInsideMe)
{
    Node *insidenode = shipsInsideMe->insideList.head;
    InsideShip *insideship;

    while (insidenode != NULL)
    {
        insideship = (InsideShip *)listGetStructOfNode(insidenode);
        dbgAssert(insideship->ship->objtype == OBJ_ShipType);
        if ((insideship->ship->flags & SOF_Dead) == 0)
        {
            SpaceObjRegistryRegister((SpaceObj *)insideship->ship);
        }

        insidenode = insidenode->next;
    }
}

void RegisterAllSpaceObjs(void)
{
    Node *node = universe.SpaceObjList.head;
    SpaceObj *obj;

    while (node != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);

        if (obj->objtype == OBJ_EffectType)
        {
            goto nextnode;
        }
        if (obj->flags & SOF_Dead)
        {
            goto nextnode;
        }

        SpaceObjRegistryRegister(obj);
        if (obj->objtype == OBJ_ShipType)
        {
            if (((Ship *)obj)->shipsInsideMe != NULL)
            {
                // check ships inside too
                RegisterAllSpaceObjsInsideMe(((Ship *)obj)->shipsInsideMe);
            }

            if (((Ship *)obj)->specialFlags & SPECIAL_IsASalvager)
            {
                SalCapCorvette_RegisterExtraSpaceObjs((Ship *)obj);
            }
        }

nextnode:
        node = node->next;
    }

    node = universe.MinorSpaceObjList.head;
    while (node != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);

        if ((obj->flags & SOF_Dead) == 0)
        {
            SpaceObjRegistryRegister(obj);
        }

        node = node->next;
    }

    // ALWAYS saved now because we can have hyperspace ships in multiplayergame too
    {
        Ship *ship;
        InsideShip *insideship;
        node = singlePlayerGameInfo.ShipsInHyperspace.head;
        while (node != NULL)
        {
            insideship = (InsideShip *)listGetStructOfNode(node);
            ship = insideship->ship;
            dbgAssert(ship->objtype == OBJ_ShipType);
            dbgAssert((ship->flags & SOF_Dead) == 0);

            SpaceObjRegistryRegister((SpaceObj *)ship);
            if (ship->shipsInsideMe != NULL)
            {
                // check ships inside too
                RegisterAllSpaceObjsInsideMe(ship->shipsInsideMe);
            }

            node = node->next;
        }
    }
}

void SaveAllBlobs()
{
    sdword numBlobs = BlobRegistry.selection->numShips;
    sdword i;
    InfoChunkContents infocontents;
    SaveChunk *chunk;

    infocontents.info = numBlobs;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));

    for (i=0;i<numBlobs;i++)
    {
        SaveBlob((blob *)BlobRegistry.selection->ShipPtr[i]);
    }

    memFree(chunk);
}

void LoadAllBlobs()
{
    SaveChunk *infochunk;
    sdword numBlobs;
    blob *tblob;
    sdword i;

    infochunk = LoadNextChunk();
    VerifyChunk(infochunk,INFO_CHUNK,sizeof(InfoChunkContents));

    numBlobs = ((InfoChunkContents *)chunkContents(infochunk))->info;

    memFree(infochunk);

    for (i=0;i<numBlobs;i++)
    {
        tblob = LoadBlob();
        BlobRegistryRegister(tblob);
    }
}

void FixAllBlobs()
{
    sdword numBlobs = BlobRegistry.selection->numShips;
    sdword i;

    for (i=0;i<numBlobs;i++)
    {
        FixBlob((blob *)BlobRegistry.selection->ShipPtr[i]);
    }
}

void SaveBlob(blob *tblob)
{
    blob *sblob;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_BLOB,sizeof(blob),tblob);

    sblob = chunkContents(chunk);

    sblob->subBlobs.num = BIT31;        // flag subblobs as nonexistent

    SaveThisChunk(chunk);

    dbgAssert(bitTest(sblob->flags, BTF_FreeThisBlob));
    dbgAssert(bitTest(sblob->flags, BTF_FreeBlobObjects));

    if (sblob->blobShips != NULL) SaveSelection((SpaceObjSelection *)sblob->blobShips);
    if (sblob->blobSmallShips != NULL) SaveSelection((SpaceObjSelection *)sblob->blobSmallShips);
    if (sblob->blobBigShips != NULL) SaveSelection((SpaceObjSelection *)sblob->blobBigShips);
    if (sblob->blobSmallTargets != NULL) SaveSelection((SpaceObjSelection *)sblob->blobSmallTargets);
    if (sblob->blobBigTargets != NULL) SaveSelection((SpaceObjSelection *)sblob->blobBigTargets);
    if (sblob->blobResources != NULL) SaveSelection((SpaceObjSelection *)sblob->blobResources);
    if (sblob->blobDerelicts != NULL) SaveSelection((SpaceObjSelection *)sblob->blobDerelicts);
    if (sblob->blobBullets != NULL) SaveSelection((SpaceObjSelection *)sblob->blobBullets);
    if (sblob->blobMissileMissiles != NULL) SaveSelection((SpaceObjSelection *)sblob->blobMissileMissiles);
    if (sblob->blobMissileMines != NULL) SaveSelection((SpaceObjSelection *)sblob->blobMissileMines);

    if (sblob->blobObjects != NULL) SaveSelection((SpaceObjSelection *)sblob->blobObjects);

    memFree(chunk);
}

blob *LoadBlob(void)
{
    SaveChunk *chunk;
    blob *lblob;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_BLOB,sizeof(blob));

    lblob = memAlloc(sizeof(blob),"loadblob",0);
    memcpy(lblob,chunkContents(chunk),sizeof(blob));

    lblob->subBlobs.num = BIT31;            // flag subblobs as nonexistent

    memFree(chunk);

    if (lblob->blobShips != NULL) { lblob->blobShips = (SelectCommand *)LoadSelection(); lblob->maxNumShips = lblob->blobShips->numShips; }
    if (lblob->blobSmallShips != NULL) { lblob->blobSmallShips = (SelectCommand *)LoadSelection(); lblob->maxNumSmallShips = lblob->blobSmallShips->numShips; }
    if (lblob->blobBigShips != NULL) { lblob->blobBigShips = (SelectCommand *)LoadSelection(); lblob->maxNumBigShips = lblob->blobBigShips->numShips; }
    if (lblob->blobSmallTargets != NULL) { lblob->blobSmallTargets = (SelectAnyCommand *)LoadSelection(); lblob->maxNumSmallTargets = lblob->blobSmallTargets->numTargets; };
    if (lblob->blobBigTargets != NULL) { lblob->blobBigTargets = (SelectAnyCommand *)LoadSelection(); lblob->maxNumBigTargets = lblob->blobBigTargets->numTargets; }
    if (lblob->blobResources != NULL) { lblob->blobResources = (ResourceSelection *)LoadSelection(); lblob->maxNumResources = lblob->blobResources->numResources; }
    if (lblob->blobDerelicts != NULL) { lblob->blobDerelicts = (DerelictSelection *)LoadSelection(); lblob->maxNumDerelicts = lblob->blobDerelicts->numDerelicts; }
    if (lblob->blobBullets != NULL) { lblob->blobBullets = (BulletSelection *)LoadSelection(); lblob->maxNumBullets = lblob->blobBullets->numBullets; }
    if (lblob->blobMissileMissiles != NULL) { lblob->blobMissileMissiles = (MissileSelection *)LoadSelection(); lblob->maxNumMissileMissiles = lblob->blobMissileMissiles->numMissiles; }
    if (lblob->blobMissileMines != NULL) { lblob->blobMissileMines = (MissileSelection *)LoadSelection(); lblob->maxNumMissileMines = lblob->blobMissileMines->numMissiles; }

    if (lblob->blobObjects != NULL) { lblob->blobObjects = LoadSelection(); lblob->maxNumObjects = lblob->blobObjects->numSpaceObjs; }

    return lblob;
}

void FixBlob(blob *tblob)
{
    if (tblob->blobShips != NULL) FixSelection((SpaceObjSelection *)tblob->blobShips);
    if (tblob->blobSmallShips != NULL) FixSelection((SpaceObjSelection *)tblob->blobSmallShips);
    if (tblob->blobBigShips != NULL) FixSelection((SpaceObjSelection *)tblob->blobBigShips);
    if (tblob->blobSmallTargets != NULL) FixSelection((SpaceObjSelection *)tblob->blobSmallTargets);
    if (tblob->blobBigTargets != NULL) FixSelection((SpaceObjSelection *)tblob->blobBigTargets);
    if (tblob->blobResources != NULL) FixSelection((SpaceObjSelection *)tblob->blobResources);
    if (tblob->blobDerelicts != NULL) FixSelection((SpaceObjSelection *)tblob->blobDerelicts);
    if (tblob->blobBullets != NULL) FixSelection((SpaceObjSelection *)tblob->blobBullets);
    if (tblob->blobMissileMissiles != NULL) FixSelection((SpaceObjSelection *)tblob->blobMissileMissiles);
    if (tblob->blobMissileMines != NULL) FixSelection((SpaceObjSelection *)tblob->blobMissileMines);

    if (tblob->blobObjects != NULL) FixSelection((SpaceObjSelection *)tblob->blobObjects);
}

void SaveAllSpaceObjs()
{
    sdword numObjs = SpaceObjRegistry.selection->numShips;
    sdword i;
    InfoChunkContents infocontents;
    SaveChunk *chunk;

    infocontents.info = numObjs;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));
    for (i=0;i<numObjs;i++)
    {
        SaveSpaceObj((SpaceObj *)SpaceObjRegistry.selection->ShipPtr[i]);
    }

    memFree(chunk);
}

void LoadAllSpaceObjs()
{
    SaveChunk *infochunk;
    sdword numObjs;
    SpaceObj *obj;
    sdword i;

    infochunk = LoadNextChunk();
    VerifyChunk(infochunk,INFO_CHUNK,sizeof(InfoChunkContents));

    numObjs = ((InfoChunkContents *)chunkContents(infochunk))->info;

    memFree(infochunk);

    for (i=0;i<numObjs;i++)
    {
        obj = LoadSpaceObj();
        SpaceObjRegistryRegisterNoCheck(obj);
    }
}

void FixAllSpaceObjs()
{
    sdword numObjs = SpaceObjRegistry.selection->numShips;
    sdword i;

    for (i=0;i<numObjs;i++)
    {
        FixSpaceObj((SpaceObj *)SpaceObjRegistry.selection->ShipPtr[i]);
    }
}

void SaveAttackTargets(AttackTargets *multipleAttackTargets)
{
    AttackTargets *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_ATTACKTARGETS,sizeofAttackTargets(multipleAttackTargets->numAttackTargets),multipleAttackTargets);
    savecontents = chunkContents(chunk);

    for (i=0;i<multipleAttackTargets->numAttackTargets;i++)
    {
        savecontents->TargetPtr[i] = (TargetPtr)SpaceObjRegistryGetID((SpaceObj *)multipleAttackTargets->TargetPtr[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

AttackTargets *LoadAttackTargets()
{
    AttackTargets *loadedAttackTargets;
    AttackTargets *originalAttackTargets;
    SaveChunk *chunk;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_ATTACKTARGETS);

    originalAttackTargets = (AttackTargets *)chunkContents(chunk);
    size = sizeofAttackTargets(originalAttackTargets->numAttackTargets);
    dbgAssert(size == chunk->contentsSize);
    loadedAttackTargets = memAlloc(size,"AttackTargets",0);
    memcpy(loadedAttackTargets,originalAttackTargets,size);

    memFree(chunk);

    return loadedAttackTargets;
}

void FixAttackTargets(AttackTargets *multipleAttackTargets)
{
    sdword i;

    for (i=0;i<multipleAttackTargets->numAttackTargets;i++)
    {
        multipleAttackTargets->TargetPtr[i] = SpaceObjRegistryGetTarget((sdword)multipleAttackTargets->TargetPtr[i]);
    }
}

void SaveGunInfo(Ship *ship)
{
    GunInfo *originalGunInfo = ship->gunInfo;
    GunInfo *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_GUNINFO,sizeofGunInfo(originalGunInfo->numGuns),originalGunInfo);
    savecontents = chunkContents(chunk);

    for (i=0;i<originalGunInfo->numGuns;i++)
    {
        savecontents->guns[i].gunstatic = NULL;
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadGunInfo(Ship *ship)
{
    GunInfo *loadedGunInfo;
    GunInfo *originalGunInfo;
    SaveChunk *chunk;
    sdword size;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_GUNINFO);

    originalGunInfo = (GunInfo *)chunkContents(chunk);
    size = sizeofGunInfo(originalGunInfo->numGuns);
    dbgAssert(size == chunk->contentsSize);
    loadedGunInfo = memAlloc(size,"gunInfo",NonVolatile);
    memcpy(loadedGunInfo,originalGunInfo,size);

    memFree(chunk);

    ship->gunInfo = loadedGunInfo;

    for (i=0;i<loadedGunInfo->numGuns;i++)
    {
        loadedGunInfo->guns[i].gunstatic = &ship->staticinfo->gunStaticInfo->gunstatics[i];
#if 0
        if (loadedGunInfo->guns[i].gunstatic->guntype == GUN_NewGimble)
        {
            univResetNewGimbleGun(&loadedGunInfo->guns[i]);
        }
#endif
    }
}

void SaveDockInfo(Ship *ship)
{
    DockInfo *originalDockInfo = ship->dockInfo;
    DockInfo *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_DOCKINFO,sizeofDockInfo(originalDockInfo->numDockPoints),originalDockInfo);
    savecontents = chunkContents(chunk);

    for (i=0;i<originalDockInfo->numDockPoints;i++)
    {
        if (savecontents->dockpoints[i].dockstaticpoint == NULL)
        {
            savecontents->dockpoints[i].dockstaticpoint = -1;
        }
        else
        {
            savecontents->dockpoints[i].dockstaticpoint = (DockStaticPoint*)savecontents->dockpoints[i].dockstaticpoint->dockindex;
        }
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadDockInfo(Ship *ship)
{
    DockInfo *loadedDockInfo;
    DockInfo *originalDockInfo;
    SaveChunk *chunk;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_DOCKINFO);

    originalDockInfo = (DockInfo *)chunkContents(chunk);
    size = sizeofDockInfo(originalDockInfo->numDockPoints);
    dbgAssert(size == chunk->contentsSize);
    loadedDockInfo = memAlloc(size,"dockInfo",NonVolatile);
    memcpy(loadedDockInfo,originalDockInfo,size);

    memFree(chunk);

    ship->dockInfo = loadedDockInfo;
}

void FixDockInfo(Ship *ship)
{
    DockInfo *dockInfo = ship->dockInfo;
    DockPoint *dockPoint;
    sdword i;

    for (i=0;i<dockInfo->numDockPoints;i++)
    {
        dockPoint = &dockInfo->dockpoints[i];
        if (dockPoint->dockstaticpoint == -1)
        {
            dockPoint->dockstaticpoint = NULL;
        }
        else
        {
            dockPoint->dockstaticpoint = &ship->staticinfo->dockStaticInfo->dockstaticpoints[(sdword)dockPoint->dockstaticpoint];
        }
    }
}

void SaveSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *ship)
{
    SalvageInfo *originalSalvageInfo = ship->salvageInfo;
    SalvageInfo *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SALVAGEINFO,sizeofSalvageInfo(originalSalvageInfo->numSalvagePoints),originalSalvageInfo);
    savecontents = chunkContents(chunk);

    for (i=0;i<originalSalvageInfo->numSalvagePoints;i++)
    {
        if (savecontents->salvagePoints[i].salvageStaticPoint == NULL)
        {
            savecontents->salvagePoints[i].salvageStaticPoint = -1;
        }
        else
        {
            savecontents->salvagePoints[i].salvageStaticPoint = savecontents->salvagePoints[i].salvageStaticPoint->number;
        }
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *ship)
{
    SalvageInfo *loadedSalvageInfo;
    SalvageInfo *originalSalvageInfo;
    SaveChunk *chunk;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_SALVAGEINFO);

    originalSalvageInfo = (SalvageInfo *)chunkContents(chunk);
    size = sizeofSalvageInfo(originalSalvageInfo->numSalvagePoints);
    dbgAssert(size == chunk->contentsSize);
    loadedSalvageInfo = memAlloc(size,"salvageInfo",NonVolatile);
    memcpy(loadedSalvageInfo,originalSalvageInfo,size);

    memFree(chunk);

    ship->salvageInfo = loadedSalvageInfo;
}

void FixSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *ship)
{
    SalvageInfo *salvageInfo = ship->salvageInfo;
    SalvagePoint *salvagePoint;
    sdword i;

    for (i=0;i<salvageInfo->numSalvagePoints;i++)
    {
        salvagePoint = &salvageInfo->salvagePoints[i];
        if (salvagePoint->salvageStaticPoint == -1)
        {
            salvagePoint->salvageStaticPoint = NULL;
        }
        else
        {
            salvagePoint->salvageStaticPoint = &ship->staticinfo->salvageStaticInfo->salvageStaticPoints[(sdword)salvagePoint->salvageStaticPoint];
        }
    }
}

void SaveShipSinglePlayerGameInfo(Ship *ship)
{
//    ShipSinglePlayerGameInfo *savecontents;
    SaveChunk *chunk;

    ship->shipSinglePlayerGameInfo->hyperspaceEffect = NULL;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SHIPSINGLEPLAYERGAMEINFO,sizeof(ShipSinglePlayerGameInfo),ship->shipSinglePlayerGameInfo);
//    savecontents = chunkContents(chunk);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadShipSinglePlayerGameInfo(Ship *ship)
{
    ShipSinglePlayerGameInfo *loadedShipSinglePlayerInfo;
    ShipSinglePlayerGameInfo *originalShipSinglePlayerInfo;
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SHIPSINGLEPLAYERGAMEINFO,sizeof(ShipSinglePlayerGameInfo));

    originalShipSinglePlayerInfo = (ShipSinglePlayerGameInfo *)chunkContents(chunk);
    loadedShipSinglePlayerInfo = memAlloc(sizeof(ShipSinglePlayerGameInfo),"shipsingleGame",0);
    memcpy(loadedShipSinglePlayerInfo,originalShipSinglePlayerInfo,sizeof(ShipSinglePlayerGameInfo));

    ship->shipSinglePlayerGameInfo = loadedShipSinglePlayerInfo;

    ship->shipSinglePlayerGameInfo->hyperspaceEffect = NULL;

    memFree(chunk);
}

void Save_String(char *string)
{
    SaveChunk *chunk;

    if (string == NULL)
    {
        chunk = CreateChunk(VARIABLE_STRUCTURE|SAVESTRING,0,string);
    }
    else
    {
        chunk = CreateChunk(VARIABLE_STRUCTURE|SAVESTRING,strlen(string)+1,string);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

char *Load_String(void)
{
    SaveChunk *chunk;
    char *str;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE|SAVESTRING);

    if (chunk->contentsSize == 0)
    {
        memFree(chunk);
        return NULL;
    }

    size = strlen((char *)chunkContents(chunk)) + 1;
    dbgAssert(size == chunk->contentsSize);

    str = memAlloc(size,"loadedstring",0);
    memcpy(str,chunkContents(chunk),size);

    memFree(chunk);
    return str;
}

void Load_StringToAddress(char *addr)
{
    SaveChunk *chunk;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE|SAVESTRING);

    size = strlen((char *)chunkContents(chunk)) + 1;
    dbgAssert(size == chunk->contentsSize);

    memcpy(addr,chunkContents(chunk),size);

    memFree(chunk);
}

void SaveStructureOfSize(void *structure,sdword size)
{
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE,size,structure);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void *LoadStructureOfSize(sdword size)
{
    SaveChunk *chunk;
    void *structure;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE,size);

    structure = memAlloc(size,"loadedstructure",0);
    memcpy(structure,chunkContents(chunk),size);

    memFree(chunk);
    return structure;
}

void LoadStructureOfSizeToAddress(void *address,sdword size)
{
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE,size);

    memcpy(address,chunkContents(chunk),size);

    memFree(chunk);
}

void SaveStructureWithSize(void *structure)
{
    SaveChunk *chunk;
    chunk = CreateChunk(VARIABLE_STRUCTURE,*((sdword *)structure),structure);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void *LoadStructureWithSize()
{
    SaveChunk *chunk;
    void *chunkcontents;
    void *loadedstructure;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE);

    chunkcontents = chunkContents(chunk);
    dbgAssert( (*((sdword *)chunkcontents)) == chunk->contentsSize);

    loadedstructure = memAlloc(chunk->contentsSize,"loadedvarstr",0);
    memcpy(loadedstructure,chunkcontents,chunk->contentsSize);

    memFree(chunk);

    return loadedstructure;
}

void SaveShipsInsideMe(Ship *ship)
{
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SHIPSINSIDEME,sizeof(ShipsInsideMe),ship->shipsInsideMe);

    SaveThisChunk(chunk);
    memFree(chunk);

    SaveLinkedListOfInsideShips(&ship->shipsInsideMe->insideList);
}

void LoadShipsInsideMe(Ship *ship)
{
    SaveChunk *chunk;
    ShipsInsideMe *shipsInsideMe;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SHIPSINSIDEME,sizeof(ShipsInsideMe));

    shipsInsideMe = memAlloc(sizeof(ShipsInsideMe),"ShipsInsideMe",NonVolatile);
    memcpy(shipsInsideMe,chunkContents(chunk),sizeof(ShipsInsideMe));
    ship->shipsInsideMe = shipsInsideMe;

    memFree(chunk);

    LoadLinkedListOfInsideShips(&ship->shipsInsideMe->insideList);
}

void SaveSlaveInfo(Ship *ship)
{
    SaveChunk *chunk;
    SlaveInfo *savecontents;

    dbgAssert(ship->slaveinfo);

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SLAVEINFO,sizeof(SlaveInfo),ship->slaveinfo);
    savecontents = (SlaveInfo *)chunkContents(chunk);

    savecontents->Master = SpaceObjRegistryGetID((SpaceObj *)savecontents->Master);

    SaveThisChunk(chunk);
    memFree(chunk);

    if (ship->slaveinfo->flags & SF_MASTER)
    {
        SaveLinkedListOfSpaceObjs(&ship->slaveinfo->slaves);
    }
}

void LoadSlaveInfo(Ship *ship)
{
    SaveChunk *chunk;
    SlaveInfo *slaveinfo;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SLAVEINFO,sizeof(SlaveInfo));

    slaveinfo = memAlloc(sizeof(SlaveInfo),"SlaveInfo",0);
    memcpy(slaveinfo,chunkContents(chunk),sizeof(SlaveInfo));
    memFree(chunk);

    if (slaveinfo->flags & SF_MASTER)
    {
        JustLoadLinkedListOfSpaceObjs(&slaveinfo->slaves);
    }

    ship->slaveinfo = slaveinfo;
}

void SlaveAddToSlaveListCB(LinkedList *list,SpaceObj *obj);

void FixSlaveInfo(Ship *ship)
{
    SlaveInfo *slaveinfo = ship->slaveinfo;

    slaveinfo->Master = SpaceObjRegistryGetShip((sdword)slaveinfo->Master);

    if (slaveinfo->flags & SF_MASTER)
    {
        FixLinkedListOfSpaceObjs(&slaveinfo->slaves,SlaveAddToSlaveListCB);
    }
}

void FixShipsInsideMe(Ship *ship)
{
    FixLinkedListOfInsideShips(&ship->shipsInsideMe->insideList);
}

void CopyPointersFromRealStaticHeader(StaticHeader *dynamicstaticheader,StaticHeader *realstaticheader)
{
    dynamicstaticheader->LOD = realstaticheader->LOD;
    dynamicstaticheader->pMexData = realstaticheader->pMexData;
}

void CopyPointersFromRealAsteroidStaticInfo(AsteroidStaticInfo *dynamicstaticinfo,AsteroidStaticInfo *realstaticinfo)
{
    CopyPointersFromRealStaticHeader(&dynamicstaticinfo->staticheader,&realstaticinfo->staticheader);
}

void CopyPointersFromRealDustCloudStaticInfo(DustCloudStaticInfo *dynamicstaticinfo,DustCloudStaticInfo *realstaticinfo)
{
    CopyPointersFromRealStaticHeader(&dynamicstaticinfo->staticheader,&realstaticinfo->staticheader);
}

void CopyPointersFromRealShipStaticInfo(ShipStaticInfo *dynamicstaticinfo,ShipStaticInfo *realstaticinfo)
{
    sdword i;

    CopyPointersFromRealStaticHeader(&dynamicstaticinfo->staticheader,&realstaticinfo->staticheader);

    dynamicstaticinfo->salvageStaticInfo = realstaticinfo->salvageStaticInfo;
    dynamicstaticinfo->custshipheader = realstaticinfo->custshipheader;
    dynamicstaticinfo->custstatinfo = realstaticinfo->custstatinfo;

    for (i=0;i<MAX_NUM_TRAILS;i++)
    {
        dynamicstaticinfo->trailStatic[i] = realstaticinfo->trailStatic[i];
    }

    dynamicstaticinfo->madStatic = realstaticinfo->madStatic;

    dynamicstaticinfo->gunStaticInfo = realstaticinfo->gunStaticInfo;
    dynamicstaticinfo->dockStaticInfo = realstaticinfo->dockStaticInfo;
    dynamicstaticinfo->dockOverideInfo = realstaticinfo->dockOverideInfo;
    dynamicstaticinfo->navlightStaticInfo = realstaticinfo->navlightStaticInfo;
    dynamicstaticinfo->resNozzleStatic = realstaticinfo->resNozzleStatic;
    dynamicstaticinfo->tractorEmitterStatic = realstaticinfo->tractorEmitterStatic;
    dynamicstaticinfo->ShipXDocksAtShipY = realstaticinfo->ShipXDocksAtShipY;
    dynamicstaticinfo->LaunchShipXFromShipY = realstaticinfo->LaunchShipXFromShipY;
}

void SaveClampInfo(ClampInfo *clampInfo)
{
    SaveChunk *chunk;
    ClampInfo *sc;

    chunk = CreateChunk(BASIC_STRUCTURE,sizeof(ClampInfo),clampInfo);
    sc = chunkContents(chunk);

    sc->host = SpaceObjRegistryGetID((SpaceObj *)sc->host);

    SaveThisChunk(chunk);

    memFree(chunk);
}

ClampInfo *LoadClampInfo(void)
{
    ClampInfo *clampInfo = LoadStructureOfSize(sizeof(ClampInfo));
    return clampInfo;
}

void FixClampInfo(ClampInfo *clampInfo)
{
    clampInfo->host = SpaceObjRegistryGetTarget((sdword)clampInfo->host);
}

void SaveShip(Ship *ship)
{
    udword sizespec;
    udword sizeship = sizeof(Ship), totalsize; //, madoffset;
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;
    sdword i,j;

    Ship *savecontents;
    SaveChunk *chunk;

    if ((sizespec = shipstaticinfo->custshipheader.sizeofShipSpecifics) != 0)
    {
        sizeship += sizespec - sizeof(udword);
    }
    totalsize = sizeship;
    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {
        totalsize += sizeof(PreciseSelection);
    }

    if (ship->madBindings != NULL)
    {
        //madoffset = totalsize;
        totalsize += ship->madBindings->totalSize;
    }

    chunk = CreateChunk(VARIABLE_STRUCTURE|SAVE_SPACEOBJ|OBJ_ShipType,totalsize,ship);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetID(ship->collMyBlob);
    savecontents->collInfo.precise = NULL;
    savecontents->playerowner = SavePlayerToPlayerIndex(savecontents->playerowner);

    savecontents->dockingship = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->dockingship);
    savecontents->rowGetOutOfWay = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->rowGetOutOfWay);
    savecontents->gettingrocked = NULL;
    savecontents->recentAttacker = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->recentAttacker);
    savecontents->firingAtUs = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->firingAtUs);

    // ship->flightmanInfo saved in chunk if non-NULL
    savecontents->formationcommand = ConvertPointerInListToNum(&universe.mainCommandLayer.todolist,savecontents->formationcommand);
    savecontents->command = ConvertPointerInListToNum(&universe.mainCommandLayer.todolist,savecontents->command);

    //ship->attackvars.multipleAttackTarget saved in chunk if non-NULL

    savecontents->attackvars.attacktarget = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->attackvars.attacktarget);
    savecontents->attackvars.myLeaderIs = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->attackvars.myLeaderIs);
    savecontents->attackvars.myWingmanIs = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->attackvars.myWingmanIs);

    savecontents->dockvars.dockship = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecontents->dockvars.dockship);
    savecontents->dockvars.busyingThisShip = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecontents->dockvars.busyingThisShip);

    if (ship->dockvars.dockstaticpoint == NULL)
    {
        savecontents->dockvars.dockstaticpoint = -1;
    }
    else
    {
        dbgAssert(ship->dockvars.dockship);
        savecontents->dockvars.dockstaticpoint = savecontents->dockvars.dockstaticpoint->dockindex;
    }

    // ship->dockvars.customdockinfo saved in chunk if non-NULL;

    savecontents->rceffect = NULL;

    PreFix_ShipXHarvestsResourceY(savecontents);

    for (i=0;i<3;i++)
    {
        for (j=0;j<3;j++)
        {
            savecontents->showingDamage[i][j] = NULL;
        }
    }

    savecontents->tractorbeam_playerowner = SavePlayerToPlayerIndex(savecontents->tractorbeam_playerowner);

    savecontents->newDockWithTransfer = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecontents->newDockWithTransfer);

    for (i=0;i<MAX_NUM_TRAILS;i++)
    {
        savecontents->trail[i] = 0;     // on load re-initialize trails
    }

    // savecontents->soundevent will get reinitialized on load

    // slaveInfo saved

    // gunInfo saved
    // dockInfo saved

    // reinitialize navLightInfo

    // shipsInsideMe saved

    if (ship->madBindings != NULL)
    {
        //savecontents->madBindings = (madanim *)(((ubyte *)savecontents) + madoffset);
        PreFix_madBindings(ship,savecontents);
    }

    // fix later
    for (i=0;i<2;i++)
    {
        savecontents->lightning[i] = 0;
    }

    if (shipstaticinfo->custshipheader.CustShip_PreFix != NULL)
    {
        shipstaticinfo->custshipheader.CustShip_PreFix(savecontents);
    }

    SaveThisChunk(chunk);

    if (ship->flags & SOF_StaticInfoIsDynamic)
    {
        SaveChunk *statchunk = CreateChunk(BASIC_STRUCTURE|SAVE_STATICINFO,sizeof(ShipStaticInfo),ship->staticinfo);
        SaveThisChunk(statchunk);
        memFree(statchunk);
    }

    if (ship->clampInfo != NULL)
    {
        SaveClampInfo(ship->clampInfo);
    }

    if (ship->salvageInfo != NULL)
    {
        SaveSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)ship);
    }

    if (ship->flightmanInfo != NULL)
    {
        SaveStructureWithSize(ship->flightmanInfo);
    }

    if (ship->attackvars.multipleAttackTargets != NULL)
    {
        SaveAttackTargets(ship->attackvars.multipleAttackTargets);
    }

    if (ship->dockvars.customdockinfo != NULL)
    {
        SaveStructureWithSize(ship->dockvars.customdockinfo);
    }

    if (ship->damageLights)
    {
        dbgAssert(ship->nDamageLights > 0);
        SaveStructureOfSize(ship->damageLights,ship->nDamageLights*sizeof(DamageLightStatic));
    }

    if (ship->flags & SOF_Slaveable)
    {
        dbgAssert(ship->slaveinfo);
        SaveSlaveInfo(ship);
    }

    if (ship->gunInfo != NULL)
    {
        SaveGunInfo(ship);
    }

    if (ship->dockInfo != NULL)
    {
        SaveDockInfo(ship);
    }

    if (ship->shipsInsideMe != NULL)
    {
        SaveShipsInsideMe(ship);
    }

    if (ship->madBindings != NULL)
    {
        Save_madBindings(ship);
    }

    if (ship->shipSinglePlayerGameInfo != NULL)
    {
        SaveShipSinglePlayerGameInfo(ship);
    }

    if (shipstaticinfo->custshipheader.CustShip_Save != NULL)
    {
        shipstaticinfo->custshipheader.CustShip_Save(ship);
    }
done:
    memFree(chunk);
}

Ship *LoadShip(SaveChunk *chunk)
{
    Ship *ship;
    udword sizespec;
    udword sizeship = sizeof(Ship), totalsize;
    ShipStaticInfo *shipstaticinfo;

    VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE|SAVE_SPACEOBJ|OBJ_ShipType);

    ship = memAlloc(chunk->contentsSize,"Ship",NonVolatile);
    memcpy(ship,chunkContents(chunk),chunk->contentsSize);

    if (ship->flags & SOF_StaticInfoIsDynamic)
    {
        SaveChunk *statchunk = LoadNextChunk();
        VerifyChunk(statchunk,BASIC_STRUCTURE|SAVE_STATICINFO,sizeof(ShipStaticInfo));

        ship->staticinfo = shipstaticinfo = memAlloc(sizeof(ShipStaticInfo),"shipstatic",NonVolatile);
        memcpy(ship->staticinfo,chunkContents(statchunk),statchunk->contentsSize);
        memFree(statchunk);

        CopyPointersFromRealShipStaticInfo(ship->staticinfo,GetShipStaticInfo(ship->shiptype,ship->shiprace));
    }
    else
    {
        ship->staticinfo = shipstaticinfo = GetShipStaticInfo(ship->shiptype,ship->shiprace);
    }

    if ((sizespec = shipstaticinfo->custshipheader.sizeofShipSpecifics) != 0)
    {
        sizeship += sizespec - sizeof(udword);
    }
    totalsize = sizeship;
    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {
        totalsize += sizeof(PreciseSelection);
    }

    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {
        ship->collInfo.precise = (PreciseSelection *)(((ubyte *)ship) + sizeship);
    }
    else
    {
        ship->collInfo.precise = NULL;
    }

    if (ship->madBindings)
    {
        ship->madBindings = (madanim *)(((ubyte *)ship) + totalsize);
        totalsize += ship->madBindings->totalSize;
    }

    dbgAssert(totalsize == chunk->contentsSize);

    memFree(chunk);

    // re-initialize engine trails
    InitializeEngineTrails(ship);

    if (ship->clampInfo != NULL)
    {
        ship->clampInfo = LoadClampInfo();
    }

    if (ship->salvageInfo != NULL)
    {
        LoadSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)ship);
    }

    if (ship->flightmanInfo != NULL)
    {
        ship->flightmanInfo = LoadStructureWithSize();
    }

    if (ship->attackvars.multipleAttackTargets != NULL)
    {
        ship->attackvars.multipleAttackTargets = LoadAttackTargets();
    }

    if (ship->dockvars.customdockinfo != NULL)
    {
        ship->dockvars.customdockinfo = LoadStructureWithSize();
    }

    if (ship->damageLights != NULL)
    {
        dbgAssert(ship->nDamageLights > 0);
        ship->damageLights = LoadStructureOfSize(ship->nDamageLights*sizeof(DamageLightStatic));
    }

    if (ship->flags & SOF_Slaveable)
    {
        LoadSlaveInfo(ship);
    }

    if (ship->gunInfo != NULL)
    {
        LoadGunInfo(ship);
    }

    if (ship->dockInfo != NULL)
    {
        LoadDockInfo(ship);
    }

    InitializeNavLights(ship);

    if (ship->shipsInsideMe != NULL)
    {
        LoadShipsInsideMe(ship);
    }

    if (ship->bindings != NULL)
    {
        ship->bindings = univMeshBindingsDupe(shipstaticinfo, ship);        // reinitialize bindings
    }

    if (ship->madBindings != NULL)
    {
        Load_madBindings(ship);
    }

    if (ship->shipSinglePlayerGameInfo != NULL)
    {
        LoadShipSinglePlayerGameInfo(ship);
    }

    if (shipstaticinfo->custshipheader.CustShip_Load != NULL)
    {
        shipstaticinfo->custshipheader.CustShip_Load(ship);
    }
done:
    return ship;
}

void FixShip(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;

    ship->collMyBlob = BlobRegistryGetBlob((sdword)ship->collMyBlob);

    ship->playerowner = SavePlayerIndexToPlayer(ship->playerowner);

    ship->dockingship = SpaceObjRegistryGetShip((sdword)ship->dockingship);

    ship->rowGetOutOfWay = SpaceObjRegistryGetShip((sdword)ship->rowGetOutOfWay);
    ship->recentAttacker = SpaceObjRegistryGetShip((sdword)ship->recentAttacker);
    ship->firingAtUs = SpaceObjRegistryGetShip((sdword)ship->firingAtUs);

    ship->formationcommand = ConvertNumToPointerInList(&universe.mainCommandLayer.todolist,(sdword)ship->formationcommand);
    ship->command = ConvertNumToPointerInList(&universe.mainCommandLayer.todolist,(sdword)ship->command);

    if (ship->clampInfo)
    {
        FixClampInfo(ship->clampInfo);
    }

    if (ship->salvageInfo)
    {
        FixSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)ship);
    }

    if (ship->attackvars.multipleAttackTargets)
    {
        FixAttackTargets(ship->attackvars.multipleAttackTargets);
    }
    ship->attackvars.attacktarget = SpaceObjRegistryGetTarget((sdword)ship->attackvars.attacktarget);
    ship->attackvars.myLeaderIs = SpaceObjRegistryGetShip((sdword)ship->attackvars.myLeaderIs);
    ship->attackvars.myWingmanIs = SpaceObjRegistryGetShip((sdword)ship->attackvars.myWingmanIs);

    ship->dockvars.dockship = SpaceObjRegistryGetShip((sdword)ship->dockvars.dockship);
    ship->dockvars.busyingThisShip = SpaceObjRegistryGetShip((sdword)ship->dockvars.busyingThisShip);

    if (ship->dockvars.dockstaticpoint == -1)
    {
        ship->dockvars.dockstaticpoint = NULL;
    }
    else
    {
        Ship *dockingwith = ship->dockvars.dockship;
        dbgAssert(dockingwith);
        ship->dockvars.dockstaticpoint = &dockingwith->staticinfo->dockStaticInfo->dockstaticpoints[(sdword)ship->dockvars.dockstaticpoint];
    }

    if (ship->dockvars.dockship)
        ship->dockvars.launchpoints = GetLaunchPoints(shipstaticinfo,ship->dockvars.dockship->staticinfo);

    if (ship->flags & SOF_Slaveable)
    {
        FixSlaveInfo(ship);
    }

    if (ship->dockInfo != NULL)
    {
        FixDockInfo(ship);
    }

    if (ship->shipsInsideMe != NULL)
    {
        FixShipsInsideMe(ship);
    }

    if (ship->madBindings != NULL)
    {
        Fix_madBindings(ship);
    }

    Fix_ShipXHarvestsResourceY(ship);

    ship->tractorbeam_playerowner = SavePlayerIndexToPlayer(ship->tractorbeam_playerowner);

    ship->newDockWithTransfer = SpaceObjRegistryGetShip((sdword)ship->newDockWithTransfer);

    if (shipstaticinfo->custshipheader.CustShip_Fix != NULL)
    {
        shipstaticinfo->custshipheader.CustShip_Fix(ship);
    }
}

void SaveAsteroid(Asteroid *asteroid)
{
    Asteroid *savecontents;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_AsteroidType,sizeof(Asteroid),asteroid);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetIDNoBlobOkay(asteroid->collMyBlob);

    savecontents->resourceVolume = ConvertPointerInListToNum(&universe.ResourceVolumeList,asteroid->resourceVolume);

    SaveThisChunk(chunk);
    memFree(chunk);

    dbgAssert(asteroid->staticinfo != NULL);
    if (asteroid->flags & SOF_StaticInfoIsDynamic)
    {
        SaveChunk *statchunk = CreateChunk(BASIC_STRUCTURE|SAVE_STATICINFO,sizeof(AsteroidStaticInfo),asteroid->staticinfo);
        SaveThisChunk(statchunk);
        memFree(statchunk);
    }
}

Asteroid *LoadAsteroid(SaveChunk *chunk)
{
    Asteroid *asteroid;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_AsteroidType,sizeof(Asteroid));

    asteroid = memAlloc(sizeof(Asteroid),"Asteroid",NonVolatile);

    memcpy(asteroid,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    if (asteroid->flags & SOF_StaticInfoIsDynamic)
    {
        SaveChunk *statchunk = LoadNextChunk();
        VerifyChunk(statchunk,BASIC_STRUCTURE|SAVE_STATICINFO,sizeof(AsteroidStaticInfo));

        asteroid->staticinfo = memAlloc(sizeof(AsteroidStaticInfo),"aststatic",NonVolatile);
        memcpy(asteroid->staticinfo,chunkContents(statchunk),statchunk->contentsSize);
        memFree(statchunk);

        CopyPointersFromRealAsteroidStaticInfo((AsteroidStaticInfo *)asteroid->staticinfo,&asteroidStaticInfos[asteroid->asteroidtype]);
    }
    else
    {
        asteroid->staticinfo = (ResourceStaticInfo *)&asteroidStaticInfos[asteroid->asteroidtype];
    }

    // asteroid->collMyBlob gets fixed in FixAllSpaceObjs

    return asteroid;
}

void FixAsteroid(Asteroid *asteroid)
{
    asteroid->collMyBlob = BlobRegistryGetBlob((sdword)asteroid->collMyBlob);
    asteroid->resourceVolume = ConvertNumToPointerInList(&universe.ResourceVolumeList,(sdword)asteroid->resourceVolume);
}

void SaveCloudSystem(DustCloud *dustcloud)
{
    cloudSystem *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_CLOUDSYSTEM,sizeof(cloudSystem),dustcloud->stub);

    savecontents = chunkContents(chunk);

    savecontents->position = NULL;
    savecontents->rotation = NULL;

    for (i=0;i<MAX_BURSTS;i++)
    {
        savecontents->bursts[i] = 0;        // reset all lightening for now.  Fix later
    }

    SaveThisChunk(chunk);

    memFree(chunk);
}

void LoadCloudSystem(DustCloud *dustcloud)
{
    cloudSystem *stub;
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_CLOUDSYSTEM,sizeof(cloudSystem));

    stub = memAlloc(sizeof(cloudSystem),"cloud system", NonVolatile);

    memcpy(stub,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    dustcloud->stub = stub;
    stub->position = &dustcloud->posinfo.position;
    stub->rotation = &dustcloud->rotinfo.coordsys;
}

void SaveDustCloud(DustCloud *dustcloud)
{
    DustCloud *savecontents;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_DustType,sizeof(DustCloud),dustcloud);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetID(dustcloud->collMyBlob);

    savecontents->resourceVolume = ConvertPointerInListToNum(&universe.ResourceVolumeList,dustcloud->resourceVolume);

    SaveThisChunk(chunk);
    memFree(chunk);

    dbgAssert(dustcloud->staticinfo != NULL);
    if (dustcloud->flags & SOF_StaticInfoIsDynamic)
    {
        SaveChunk *statchunk = CreateChunk(BASIC_STRUCTURE|SAVE_STATICINFO,sizeof(DustCloudStaticInfo),dustcloud->staticinfo);
        SaveThisChunk(statchunk);
        memFree(statchunk);
    }

    if (dustcloud->stub != NULL)
    {
        SaveCloudSystem(dustcloud);
    }
}

DustCloud *LoadDustCloud(SaveChunk *chunk)
{
    DustCloud *dustcloud;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_DustType,sizeof(DustCloud));

    dustcloud = memAlloc(sizeof(DustCloud),"DustCloud",NonVolatile);

    memcpy(dustcloud,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    if (dustcloud->flags & SOF_StaticInfoIsDynamic)
    {
        SaveChunk *statchunk = LoadNextChunk();
        VerifyChunk(statchunk,BASIC_STRUCTURE|SAVE_STATICINFO,sizeof(DustCloudStaticInfo));

        dustcloud->staticinfo = memAlloc(sizeof(DustCloudStaticInfo),"dcloudstatic",NonVolatile);
        memcpy(dustcloud->staticinfo,chunkContents(statchunk),statchunk->contentsSize);
        memFree(statchunk);

        CopyPointersFromRealDustCloudStaticInfo((DustCloudStaticInfo *)dustcloud->staticinfo,&dustcloudStaticInfos[dustcloud->dustcloudtype]);
    }
    else
    {
        dustcloud->staticinfo = (ResourceStaticInfo *)&dustcloudStaticInfos[dustcloud->dustcloudtype];
    }

    // dustcloud->collMyBlob gets fixed in FixAllSpaceObjs

    if (dustcloud->stub != NULL)
    {
        LoadCloudSystem(dustcloud);
    }

    return dustcloud;
}

void FixDustCloud(DustCloud *dustcloud)
{
    dustcloud->collMyBlob = BlobRegistryGetBlob((sdword)dustcloud->collMyBlob);
    dustcloud->resourceVolume = ConvertNumToPointerInList(&universe.ResourceVolumeList,(sdword)dustcloud->resourceVolume);
}

void SaveNebula(Nebula *nebula)
{
    Nebula *savecontents;
    SaveChunk *chunk;
    nebChunk* nebchunk;
    nebulae_t* neb;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_NebulaType,sizeof(Nebula),nebula);

    dbgAssert((nebula->flags & SOF_StaticInfoIsDynamic) == 0);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetID(nebula->collMyBlob);
    savecontents->resourceVolume = ConvertPointerInListToNum(&universe.ResourceVolumeList,nebula->resourceVolume);
    nebchunk = (nebChunk*)nebula->stub;
    neb = (nebulae_t*)nebchunk->nebulae;
    savecontents->stub = nebChunkPtrToNum(neb, nebula->stub);

    SaveThisChunk(chunk);
    memFree(chunk);
}

Nebula *LoadNebula(SaveChunk *chunk)
{
    Nebula *nebula;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_NebulaType,sizeof(Nebula));

    nebula = memAlloc(sizeof(Nebula),"Nebula",NonVolatile);

    memcpy(nebula,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    nebula->staticinfo = (ResourceStaticInfo *)&nebulaStaticInfos[nebula->nebulatype];

    return nebula;
}

void FixNebula(Nebula *nebula)
{
    nebulae_t* neb;

    nebula->collMyBlob = BlobRegistryGetBlob((sdword)nebula->collMyBlob);
    nebula->resourceVolume = ConvertNumToPointerInList(&universe.ResourceVolumeList,(sdword)nebula->resourceVolume);

    neb = &nebNebulae[nebula->nebNebulaeIndex];
    nebula->stub = nebNumToChunkPtr(neb, (sdword)nebula->stub);
}

void SaveBullet(Bullet *bullet)
{
    Bullet *savecontents;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_BulletType,sizeof(Bullet),bullet);

    savecontents = chunkContents(chunk);

    dbgAssert(savecontents->staticinfo == NULL);
    savecontents->collMyBlob = BlobRegistryGetID(bullet->collMyBlob);

    savecontents->owner = SpaceObjRegistryGetID((SpaceObj *)bullet->owner);
    if (bullet->gunowner != NULL)
    {
        savecontents->gunowner = bullet->gunowner->gunstatic->gunindex;
    }
    else
    {
        savecontents->gunowner = -1;
    }
    savecontents->target = SpaceObjRegistryGetID((SpaceObj *)bullet->target);

    savecontents->hitEffect = NULL;
    savecontents->effect = NULL;

    savecontents->playerowner = SavePlayerToPlayerIndex(bullet->playerowner);

    SaveThisChunk(chunk);

    memFree(chunk);
}

Bullet *LoadBullet(SaveChunk *chunk)
{
    Bullet *bullet;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_BulletType,sizeof(Bullet));

    bullet = memAlloc(sizeof(Bullet),"Bullet",0);

    memcpy(bullet,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    // bullet->collMyBlob gets fixed in FixAllSpaceObjs
    return bullet;
}

void FixBullet(Bullet *bullet)
{
    bullet->collMyBlob = BlobRegistryGetBlob((sdword)bullet->collMyBlob);

    bullet->owner = SpaceObjRegistryGetShip((sdword)bullet->owner);
    if (bullet->gunowner != -1)
    {
        dbgAssert((sdword)bullet->gunowner < bullet->owner->gunInfo->numGuns);
        bullet->gunowner = &bullet->owner->gunInfo->guns[(sdword)bullet->gunowner];
    }
    else
    {
        bullet->gunowner = NULL;
    }
    bullet->target = SpaceObjRegistryGetTarget((sdword)bullet->target);

    bullet->playerowner = SavePlayerIndexToPlayer(bullet->playerowner);
}

void SaveDerelict(Derelict *derelict)
{
    Derelict *savecontents;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_DerelictType,sizeof(Derelict),derelict);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetIDNoBlobOkay(derelict->collMyBlob);       // planets aren't in blobs

    savecontents->dockingship = SpaceObjRegistryGetID((SpaceObj *)derelict->dockingship);
    savecontents->tractorbeam_playerowner = SavePlayerToPlayerIndex(derelict->tractorbeam_playerowner);

    savecontents->newDockWithTransfer = SpaceObjRegistryGetID((SpaceObj *)derelict->newDockWithTransfer);

    SaveThisChunk(chunk);

    memFree(chunk);

    if (derelict->clampInfo != NULL)
    {
        SaveClampInfo(derelict->clampInfo);
    }

    if (derelict->salvageInfo != NULL)
    {
        SaveSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)derelict);
    }
}

Derelict *LoadDerelict(SaveChunk *chunk)
{
    Derelict *derelict;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_DerelictType,sizeof(Derelict));

    derelict = memAlloc(sizeof(Derelict),"Derelict",NonVolatile);

    memcpy(derelict,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    dbgAssert(derelict->derelicttype < NUM_DERELICTTYPES);
    derelict->staticinfo = (DerelictStaticInfo *)&derelictStaticInfos[derelict->derelicttype];

    InitializeNavLights((Ship *)derelict);

    if (derelict->clampInfo != NULL)
    {
        derelict->clampInfo = LoadClampInfo();
    }

    if (derelict->salvageInfo != NULL)
    {
        LoadSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)derelict);
    }

    return derelict;
}

void FixDerelict(Derelict *derelict)
{
    derelict->collMyBlob = BlobRegistryGetBlob((sdword)derelict->collMyBlob);

    derelict->dockingship = SpaceObjRegistryGetShip((sdword)derelict->dockingship);
    derelict->tractorbeam_playerowner = SavePlayerIndexToPlayer(derelict->tractorbeam_playerowner);

    derelict->newDockWithTransfer = SpaceObjRegistryGetShip((sdword)derelict->newDockWithTransfer);

    if (derelict->clampInfo)
    {
        FixClampInfo(derelict->clampInfo);
    }

    if (derelict->salvageInfo)
    {
        FixSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)derelict);
    }
}

void SaveMissile(Missile *missile)
{
    Missile *savecontents;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_MissileType,sizeof(Missile),missile);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetID(missile->collMyBlob);

    savecontents->trail = NULL;

    savecontents->playerowner = SavePlayerToPlayerIndex(missile->playerowner);
    savecontents->owner = SpaceObjRegistryGetID((SpaceObj *)missile->owner);

    savecontents->target = SpaceObjRegistryGetID((SpaceObj *)missile->target);

    savecontents->hitEffect = saveEtglodGunEventToIndex(missile->hitEffect);

    savecontents->formationinfo = ConvertPointerInListToNum(&universe.MineFormationList,savecontents->formationinfo);

    SaveThisChunk(chunk);

    memFree(chunk);

    if (missile->clampInfo != NULL)
    {
        SaveClampInfo(missile->clampInfo);
    }
}

Missile *LoadMissile(SaveChunk *chunk)
{
    Missile *missile;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_MissileType,sizeof(Missile));

    missile = memAlloc(sizeof(Missile),"Missile",0);

    memcpy(missile,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    if(missile->missileType == MISSILE_Regular)
    {
        missile->staticinfo = &missileStaticInfos[missile->race];
    }
    else
    {
        dbgAssert(missile->missileType == MISSILE_Mine);
        missile->staticinfo = &mineStaticInfos[missile->race];
    }

    if (missile->clampInfo != NULL)
    {
        missile->clampInfo = LoadClampInfo();
    }

    return missile;
}

void FixMissile(Missile *missile)
{
    missile->collMyBlob = BlobRegistryGetBlob((sdword)missile->collMyBlob);

    missile->playerowner = SavePlayerIndexToPlayer(missile->playerowner);

    missile->owner = SpaceObjRegistryGetShip((sdword)missile->owner);
    missile->target = SpaceObjRegistryGetTarget((sdword)missile->target);

    missile->hitEffect = saveIndexToEtglodGunEvent((sdword)missile->hitEffect);

    missile->formationinfo = ConvertNumToPointerInList(&universe.MineFormationList,(sdword)missile->formationinfo);

    if (missile->clampInfo)
    {
        FixClampInfo(missile->clampInfo);
    }
}

void SaveSpaceObj(SpaceObj *obj)
{
    switch (obj->objtype)
    {
        case OBJ_ShipType:
            SaveShip((Ship *)obj);
            break;

        case OBJ_BulletType:
            SaveBullet((Bullet *)obj);
            break;

        case OBJ_AsteroidType:
            SaveAsteroid((Asteroid *)obj);
            break;

        case OBJ_NebulaType:
            SaveNebula((Nebula *)obj);
            break;

        case OBJ_GasType:
            dbgFatal(DBG_Loc,"Saving Gas clouds not supported yet");
            break;

        case OBJ_DustType:
            SaveDustCloud((DustCloud *)obj);
            break;

        case OBJ_DerelictType:
            SaveDerelict((Derelict *)obj);
            break;

        case OBJ_EffectType:
            dbgAssert(FALSE);
            break;

        case OBJ_MissileType:
            SaveMissile((Missile *)obj);
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

void FixSpaceObj(SpaceObj *obj)
{
    switch (obj->objtype)
    {
        case OBJ_ShipType:
            FixShip((Ship *)obj);
            break;

        case OBJ_BulletType:
            FixBullet((Bullet *)obj);
            break;

        case OBJ_AsteroidType:
            FixAsteroid((Asteroid *)obj);
            break;

        case OBJ_NebulaType:
            FixNebula((Nebula *)obj);
            break;

        case OBJ_GasType:
            dbgFatal(DBG_Loc,"Saving Gas clouds not supported yet");
            break;

        case OBJ_DustType:
            FixDustCloud((DustCloud *)obj);
            break;

        case OBJ_DerelictType:
            FixDerelict((Derelict *)obj);
            break;

        case OBJ_EffectType:
            dbgAssert(FALSE);
            break;

        case OBJ_MissileType:
            FixMissile((Missile *)obj);
            break;

        default:
            dbgAssert(FALSE);
            break;
    }

    if (obj->flags & SOF_Targetable)
    {
        ((SpaceObjRotImpTarg *)obj)->flashtimer = 0.0f;
    }
}

SpaceObj *LoadSpaceObj()
{
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    dbgAssert(chunk->type & SAVE_SPACEOBJ);

    switch (chunk->type & SAVE_OBJTYPEMASK)
    {
        case OBJ_ShipType:
            return (SpaceObj *)LoadShip(chunk);

        case OBJ_BulletType:
            return (SpaceObj *)LoadBullet(chunk);

        case OBJ_AsteroidType:
            return (SpaceObj *)LoadAsteroid(chunk);

        case OBJ_NebulaType:
            return (SpaceObj *)LoadNebula(chunk);

        case OBJ_GasType:
            dbgFatal(DBG_Loc,"Saving Gas clouds not supported yet");
            break;

        case OBJ_DustType:
            return (SpaceObj *)LoadDustCloud(chunk);

        case OBJ_DerelictType:
            return (SpaceObj *)LoadDerelict(chunk);

        case OBJ_EffectType:
            dbgAssert(FALSE);
            return NULL;

        case OBJ_MissileType:
            return (SpaceObj *)LoadMissile(chunk);

        default:
            dbgAssert(FALSE);
            return NULL;
    }

	return NULL;
}

typedef struct SaveLLSpaceObj {
    sdword num;
    sdword ID[1];
} SaveLLSpaceObj;

#define sizeofSaveLLSpaceObj(n) (sizeof(SaveLLSpaceObj) + (n-1)*sizeof(sdword))

void SaveLinkedListOfSpaceObjs(LinkedList *list)
{
    sdword num = list->num;
    SaveChunk *chunk;
    SaveLLSpaceObj *savecontents;
    Node *node = list->head;
    SpaceObj *obj;
    sdword cur = 0;

    chunk = CreateChunk(LINKEDLISTSPACEOBJS,sizeofSaveLLSpaceObj(num),NULL);

    savecontents = chunkContents(chunk);
    savecontents->num = num;

    while (node != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);
        dbgAssert(obj);

        savecontents->ID[cur++] = SpaceObjRegistryGetID(obj);

        node = node->next;
    }

    dbgAssert(cur == num);

    SaveThisChunk(chunk);

    memFree(chunk);
}

void JustLoadLinkedListOfSpaceObjs(LinkedList *list)
{
    SaveChunk *chunk;
    SaveLLSpaceObj *loadcontents;
    SaveLLSpaceObj *returncontents;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,LINKEDLISTSPACEOBJS);

    loadcontents = (struct SaveLLSpaceObj *)chunkContents(chunk);

    dbgAssert(sizeofSaveLLSpaceObj(loadcontents->num) == chunk->contentsSize);

    returncontents = memAlloc(chunk->contentsSize,"unfixedlistobj",0);
    memcpy(returncontents,loadcontents,chunk->contentsSize);
    memFree(chunk);

    list->head = (void *)returncontents;
}

void FixLinkedListOfSpaceObjs(LinkedList *list,ListAddCB listAddCB)
{
    SaveLLSpaceObj *objs;
    SpaceObj *obj;
    sdword num;
    sdword cur = 0;

    objs = (SaveLLSpaceObj *)list->head;
    dbgAssert(objs);

    listInit(list);

    num = objs->num;

    while (cur < num)
    {
        obj = SpaceObjRegistryGetObj(objs->ID[cur++]);
        if (obj != NULL)
        {
            listAddCB(list,obj);
        }
    }

    memFree(objs);
}

void LoadLinkedListOfSpaceObjs(LinkedList *list,ListAddCB listAddCB)
{
    SaveChunk *chunk;
    SaveLLSpaceObj *loadcontents;
    sdword num;
    sdword cur = 0;
    SpaceObj *obj;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,LINKEDLISTSPACEOBJS);

    loadcontents = (struct SaveLLSpaceObj *)chunkContents(chunk);
    num = loadcontents->num;

    dbgAssert(sizeofSaveLLSpaceObj(num) == chunk->contentsSize);

    listInit(list);

    while (cur < num)
    {
        obj = SpaceObjRegistryGetObj(loadcontents->ID[cur++]);
        if (obj != NULL)
        {
            listAddCB(list,obj);
        }
    }

    memFree(chunk);
}

void RenderListAddCB(LinkedList *list,SpaceObj *obj)
{
    listAddNode(list,&obj->renderlink,obj);
}

void SpaceObjListAddCB(LinkedList *list,SpaceObj *obj)
{
    listAddNode(list,&obj->objlink,obj);
}

#define MinorRenderListAddCB RenderListAddCB
#define MinorSpaceObjListAddCB SpaceObjListAddCB

void ShipListAddCB(LinkedList *list,SpaceObj *obj)
{
#define ship ((Ship *)obj)
    dbgAssert(ship->objtype == OBJ_ShipType);
    listAddNode(list,&ship->shiplink,obj);
#undef ship
}

void BulletListAddCB(LinkedList *list,SpaceObj *obj)
{
#define bullet ((Bullet *)obj)
    dbgAssert(bullet->objtype == OBJ_BulletType);
    listAddNode(list,&bullet->bulletlink,obj);
#undef bullet
}

void ResourceListAddCB(LinkedList *list,SpaceObj *obj)
{
#define resource ((Resource *)obj)
    dbgAssert(resource->flags & SOF_Resource);
    listAddNode(list,&resource->resourcelink,obj);
#undef resource
}

void DerelictListAddCB(LinkedList *list,SpaceObj *obj)
{
#define derelict ((Derelict *)obj)
    dbgAssert(derelict->objtype == OBJ_DerelictType);
    listAddNode(list,&derelict->derelictlink,obj);
#undef derelict
}

void ImpactableListAddCB(LinkedList *list,SpaceObj *obj)
{
#define impactable ((SpaceObjRotImp *)obj)
    dbgAssert(impactable->flags & SOF_Impactable);
    listAddNode(list,&impactable->impactablelink,obj);
#undef impactable
}

void MissileListAddCB(LinkedList *list,SpaceObj *obj)
{
#define missile ((Missile *)obj)
    dbgAssert(missile->objtype == OBJ_MissileType);
    listAddNode(list,&missile->missilelink,obj);
#undef missile
}

void MineAddToMineListCB(LinkedList *list,SpaceObj *obj)
{
#define mine ((Missile *)obj)
    dbgAssert(mine->objtype == OBJ_MissileType);
    listAddNode(list,&mine->formationLink,obj);
#undef mine
}

void SlaveAddToSlaveListCB(LinkedList *list,SpaceObj *obj)
{
#define ship ((Ship *)obj)
    dbgAssert(ship->objtype == OBJ_ShipType);
    listAddNode(list,&ship->slavelink,obj);
#undef ship
}

typedef struct SaveSelSpaceObj {
    sdword num;
    sdword ID[1];
} SaveSelSpaceObj;

#define sizeofSaveSelSpaceObj(n) (sizeof(SaveSelSpaceObj) + (n-1)*sizeof(sdword))


void SaveSelection(SpaceObjSelection *selection)
{
    sdword num = selection->numSpaceObjs;
    SaveChunk *chunk;
    SaveSelSpaceObj *savecontents;
    sdword i;

    chunk = CreateChunk(SELECTION,sizeofSaveSelSpaceObj(num),NULL);

    savecontents = chunkContents(chunk);
    savecontents->num = num;

    for (i=0;i<num;i++)
    {
        dbgAssert(selection->SpaceObjPtr[i] != NULL);
        savecontents->ID[i] = SpaceObjRegistryGetID(selection->SpaceObjPtr[i]);
        dbgAssert(savecontents->ID[i] != -1);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveMaxSelection(MaxSelection *maxselection)
{
    SaveSelection((SpaceObjSelection *)maxselection);
}

void SaveGrowSelection(GrowSelection *grow)
{
    SaveSelection((SpaceObjSelection *)grow->selection);
}

SpaceObjSelection *LoadSelection(void)
{
    SpaceObjSelection *selection;
    SaveChunk *chunk;
    SaveSelSpaceObj *loadcontents;
    sdword num;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,SELECTION);

    loadcontents = (struct SaveSelSpaceObj *)chunkContents(chunk);
    num = loadcontents->num;

    dbgAssert(sizeofSaveSelSpaceObj(num) == chunk->contentsSize);

    selection = memAlloc(sizeofSelectCommand(num),"loadselect",0);
    selection->numSpaceObjs = num;

    for (i=0;i<num;i++)
    {
        dbgAssert(loadcontents->ID[i] != -1);
        selection->SpaceObjPtr[i] = loadcontents->ID[i];
    }

    memFree(chunk);

    return selection;
}

void LoadGrowSelection(GrowSelection *grow)
{
    grow->selection = (SelectCommand *)LoadSelection();
    grow->maxNumShips = grow->selection->numShips;
}

void FixSelection(SpaceObjSelection *selection)
{
    sdword num = selection->numSpaceObjs;
    sdword i;

    for (i=0;i<num;i++)
    {
        selection->SpaceObjPtr[i] = SpaceObjRegistryGetObj((sdword)selection->SpaceObjPtr[i]);
        dbgAssert(selection->SpaceObjPtr[i]);
    }
}

void FixGrowSelection(GrowSelection *grow)
{
    FixSelection((SpaceObjSelection *)grow->selection);
}

SpaceObjSelection *LoadSelectionAndFix(void)
{
    SpaceObjSelection *selection;
    SaveChunk *chunk;
    SaveSelSpaceObj *loadcontents;
    sdword num;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,SELECTION);

    loadcontents = (struct SaveSelSpaceObj *)chunkContents(chunk);
    num = loadcontents->num;

    dbgAssert(sizeofSaveSelSpaceObj(num) == chunk->contentsSize);

    selection = memAlloc(sizeofSelectCommand(num),"loadselect",0);
    selection->numSpaceObjs = num;

    for (i=0;i<num;i++)
    {
        selection->SpaceObjPtr[i] = SpaceObjRegistryGetObj(loadcontents->ID[i]);
        dbgAssert(selection->SpaceObjPtr[i]);
    }

    memFree(chunk);

    return selection;
}

void LoadMaxSelectionAndFix(MaxSelection *maxselection)
{
    SelectCommand *selection = (SelectCommand *)LoadSelectionAndFix();
    memcpy(maxselection,selection,sizeofSelectCommand(selection->numShips));
    memFree(selection);
}

void LoadGrowSelectionAndFix(GrowSelection *grow)
{
    grow->selection = (SelectCommand *)LoadSelectionAndFix();
    grow->maxNumShips = grow->selection->numShips;
}

void SaveLinkedListOfStuff(LinkedList *list,SaveStuffInLinkedListCB savestuffCB)
{
    Node *node = list->head;

    SaveInfoNumber(list->num);

    while (node != NULL)
    {
        savestuffCB(listGetStructOfNode(node));

        node = node->next;
    }
}

void LoadLinkedListOfStuff(LinkedList *list,LoadStuffInLinkedListCB loadstuffCB)
{
    sdword num = LoadInfoNumber();
    sdword i;

    listInit(list);

    for (i=0;i<num;i++)
    {
        loadstuffCB(list);
    }

    dbgAssert(num == list->num);
}

void FixLinkedListOfStuff(LinkedList *list,FixStuffInLinkedListCB fixstuffCB)
{
    Node *node = list->head;

    while (node != NULL)
    {
        fixstuffCB(listGetStructOfNode(node));

        node = node->next;
    }
}

void SaveFormation(FormationCommand *formcommand)
{
    if (formcommand->formationSpecificInfo != NULL)
    {
        SaveStructureWithSize(formcommand->formationSpecificInfo);
    }
}

void LoadFormation(FormationCommand *formcommand)
{
    if (formcommand->formationSpecificInfo != NULL)
    {
        formcommand->formationSpecificInfo = LoadStructureWithSize();
    }
}

void SaveMilitarySlot(MilitarySlot *milslot)
{
    SaveChunk *chunk;
    MilitarySlot *savecontents;
    sdword i;

    chunk = CreateChunk(VARIABLE_STRUCTURE|SAVE_MILITARYSLOT,sizeofMilitarySlot(milslot->numSubslots),milslot);
    savecontents = (MilitarySlot *)chunkContents(chunk);

    for (i=0;i<savecontents->numSubslots;i++)
    {
        savecontents->subslots[i].ship = SpaceObjRegistryGetID((SpaceObj *)savecontents->subslots[i].ship);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveMilitaryParade(MilitaryParadeCommand *militaryParade)
{
    SaveChunk *chunk;
    MilitaryParadeCommand *savecontents;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_MILITARYPARADE,sizeof(MilitaryParadeCommand),militaryParade);
    savecontents = (MilitaryParadeCommand *)chunkContents(chunk);

    savecontents->aroundShip = SpaceObjRegistryGetID((SpaceObj *)militaryParade->aroundShip);

    SaveThisChunk(chunk);
    memFree(chunk);

    for (i=0;i<MAX_MILITARY_SLOTS;i++)
    {
        if (militaryParade->militarySlots[i] != NULL)
        {
            SaveMilitarySlot(militaryParade->militarySlots[i]);
        }
    }
}

MilitarySlot *LoadMilitarySlot(void)
{
    SaveChunk *chunk;
    MilitarySlot *newmilslot;
    sdword i;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE|SAVE_MILITARYSLOT);
    size = sizeofMilitarySlot(((MilitarySlot *)chunkContents(chunk))->numSubslots);

    dbgAssert(size == chunk->contentsSize);

    newmilslot = memAlloc(size,"milslot",0);
    memcpy(newmilslot,chunkContents(chunk),size);

    memFree(chunk);

    for (i=0;i<newmilslot->numSubslots;i++)
    {
        newmilslot->subslots[i].ship = SpaceObjRegistryGetShip((sdword)newmilslot->subslots[i].ship);
    }

    return newmilslot;
}

MilitaryParadeCommand *LoadMilitaryParade(void)
{
    SaveChunk *chunk;
    MilitaryParadeCommand *newmilparade;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_MILITARYPARADE,sizeof(MilitaryParadeCommand));

    newmilparade = memAlloc(sizeof(MilitaryParadeCommand),"milparadecom",0);
    memcpy(newmilparade,chunkContents(chunk),sizeof(MilitaryParadeCommand));

    memFree(chunk);

    newmilparade->aroundShip = SpaceObjRegistryGetShip((sdword)newmilparade->aroundShip);

    for (i=0;i<MAX_MILITARY_SLOTS;i++)
    {
        if (newmilparade->militarySlots[i] != NULL)
        {
            newmilparade->militarySlots[i] = LoadMilitarySlot();
        }
    }

    return newmilparade;
}

void Save_CommandToDo(CommandToDo *command)
{
    SaveChunk *chunk;
    CommandToDo *savecommand;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_COMMANDTODO,sizeof(CommandToDo),command);
    savecommand = (CommandToDo *)chunkContents(chunk);

    // fixing
    switch (command->ordertype.order)
    {
        case COMMAND_MOVE:
            // none needed
            break;

        case COMMAND_ATTACK:
            // save
            break;

        case COMMAND_DOCK:
            // none needed;
            break;

        case COMMAND_LAUNCHSHIP:
            // fixing
            savecommand->launchship.receiverShip = (sdword)SpaceObjRegistryGetID((SpaceObj *)command->launchship.receiverShip);
            break;

        case COMMAND_COLLECTRESOURCE:
            // fixing
            savecommand->collect.resource = (sdword)SpaceObjRegistryGetID((SpaceObj *)command->collect.resource);
            break;

        case COMMAND_BUILDINGSHIP:
            // fixing
            savecommand->buildingship.creator = (sdword)SpaceObjRegistryGetID((SpaceObj *)command->buildingship.creator);
            break;

        case COMMAND_SPECIAL:
            // save
            break;

        case COMMAND_MILITARYPARADE:
            // save
            break;
    }

    SaveThisChunk(chunk);

    dbgAssert(command->selection);
    SaveSelection((SpaceObjSelection *)command->selection);

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        SaveFormation(&command->formation);
    }

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        dbgAssert(command->protect);
        SaveSelection((SpaceObjSelection *)command->protect);
    }

    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        dbgAssert(command->ordertype.order != COMMAND_ATTACK);
        dbgAssert(command->attack);
        SaveSelection((SpaceObjSelection *)command->attack);
    }

    // saving
    switch (command->ordertype.order)
    {
        case COMMAND_MOVE:
            // none needed
            break;

        case COMMAND_ATTACK:
            // save
            dbgAssert((command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) == NULL);
            dbgAssert(command->attack);
            SaveSelection((SpaceObjSelection *)command->attack);
            break;

        case COMMAND_DOCK:
            // none needed;
            break;

        case COMMAND_LAUNCHSHIP:
            // fixing
            break;

        case COMMAND_COLLECTRESOURCE:
            // fixing
            break;

        case COMMAND_BUILDINGSHIP:
            // fixing
            break;

        case COMMAND_SPECIAL:
            // save
            if (command->specialtargets != NULL)
            {
                SaveSelection((SpaceObjSelection *)command->specialtargets);
            }
            break;

        case COMMAND_MILITARYPARADE:
            // save
            dbgAssert(command->militaryParade);
            SaveMilitaryParade(command->militaryParade);
            break;
    }

    memFree(chunk);
}

void Load_CommandToDo(LinkedList *list)
{
    SaveChunk *chunk;
    CommandToDo *loadedcommand;
    CommandToDo *command;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_COMMANDTODO,sizeof(CommandToDo));

    loadedcommand = (CommandToDo *)chunkContents(chunk);

    command = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    memcpy(command,loadedcommand,sizeof(CommandToDo));

    memFree(chunk);

    // fixing
    switch (command->ordertype.order)
    {
        case COMMAND_MOVE:
            // none needed
            break;

        case COMMAND_ATTACK:
            // load
            break;

        case COMMAND_DOCK:
            // none needed;
            break;

        case COMMAND_LAUNCHSHIP:
            // fixing
            command->launchship.receiverShip = SpaceObjRegistryGetShip((sdword)command->launchship.receiverShip);
            break;

        case COMMAND_COLLECTRESOURCE:
            // fixing
            command->collect.resource = SpaceObjRegistryGetResource((sdword)command->collect.resource);
            break;

        case COMMAND_BUILDINGSHIP:
            // fixing
            command->buildingship.creator = SpaceObjRegistryGetShip((sdword)command->buildingship.creator);
            break;

        case COMMAND_SPECIAL:
            // load
            break;

        case COMMAND_MILITARYPARADE:
            // load
            break;
    }

    command->selection = (SelectCommand *)LoadSelectionAndFix();

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        LoadFormation(&command->formation);
    }

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        command->protect = (ProtectCommand *)LoadSelectionAndFix();
    }

    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        dbgAssert(command->ordertype.order != COMMAND_ATTACK);
        command->attack = (AttackCommand *)LoadSelectionAndFix();
    }

    // loading
    switch (command->ordertype.order)
    {
        case COMMAND_MOVE:
            // none needed
            break;

        case COMMAND_ATTACK:
            // load
            dbgAssert((command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) == NULL);
            command->attack = (AttackCommand *)LoadSelectionAndFix();
            break;

        case COMMAND_DOCK:
            // none needed;
            break;

        case COMMAND_LAUNCHSHIP:
            // fixing
            break;

        case COMMAND_COLLECTRESOURCE:
            // fixing
            break;

        case COMMAND_BUILDINGSHIP:
            // fixing
            break;

        case COMMAND_SPECIAL:
            // load
            if (command->specialtargets != NULL)
            {
                command->specialtargets = (SpecialCommand *)LoadSelectionAndFix();
            }
            break;

        case COMMAND_MILITARYPARADE:
            // load
            dbgAssert(command->militaryParade);
            command->militaryParade = LoadMilitaryParade();
            break;
    }

    listAddNode(list,&command->todonode,command);
}


void Save_CommandLayer(CommandLayer *comlayer)
{
    sdword num = comlayer->todolist.num;
    sdword cur = 0;
    InfoChunkContents infocontents;
    Node *node = comlayer->todolist.head;
    CommandToDo *command;
    SaveChunk *chunk;

    infocontents.info = num;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));

    while (node != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(node);

        Save_CommandToDo(command);
        cur++;

        node = node->next;
    }

    dbgAssert(cur == num);

    memFree(chunk);
}

void Load_CommandLayer(CommandLayer *comlayer)
{
    SaveChunk *infochunk;
    sdword num;
    sdword i;

    infochunk = LoadNextChunk();
    VerifyChunk(infochunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(infochunk))->info;

    memFree(infochunk);

    listInit(&comlayer->todolist);
    for (i=0;i<num;i++)
    {
        Load_CommandToDo(&comlayer->todolist);
    }
}

void Save_CameraStackEntry(CameraStackEntry *currententry)
{
    SaveChunk *chunk;
    CameraStackEntry *savecse;
    sdword numShips = currententry->focus.numShips;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_CAMERASTACKENTRY,sizeofCameraStackEntry(numShips),currententry);
    savecse = (CameraStackEntry *)chunkContents(chunk);

    // fix savecse

    savecse->remembercam.playerowner = SavePlayerToPlayerIndex(savecse->remembercam.playerowner);

    for (i=0;i<numShips;i++)
    {
        savecse->focus.ShipPtr[i] = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecse->focus.ShipPtr[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void Load_CameraStackEntry(LinkedList *list)
{
    SaveChunk *chunk;
    CameraStackEntry *loadedcse;
    CameraStackEntry *cse;
    sdword numShips;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_CAMERASTACKENTRY);

    loadedcse = (CameraStackEntry *)chunkContents(chunk);
    numShips = loadedcse->focus.numShips;

    dbgAssert(sizeofCameraStackEntry(numShips) == chunk->contentsSize);

    cse = memAlloc(chunk->contentsSize,"CamStackEntry",NonVolatile);
    memcpy(cse,loadedcse,chunk->contentsSize);

    memFree(chunk);

    // fix cse

    cse->remembercam.playerowner = SavePlayerIndexToPlayer(cse->remembercam.playerowner);

    for (i=0;i<numShips;i++)
    {
        cse->focus.ShipPtr[i] = (Ship *)SpaceObjRegistryGetObj((sdword)cse->focus.ShipPtr[i]);  // don't use SpaceObjRegistryGetShip because it might not actually be a ship
    }

    listAddNode(list,&cse->stacklink,cse);
}

sdword ConvertPointerInListToNum(LinkedList *list,void *entry)
{
    Node *node = list->head;
    sdword cur = -1;

    if (entry == NULL)
    {
        return -1;
    }

    while (node != NULL)
    {
        cur++;
        if (entry == listGetStructOfNode(node))
        {
            return cur;
        }

        node = node->next;
    }

    return -1;
}

void *ConvertNumToPointerInList(LinkedList *list,sdword num)
{
    Node *node;
    sdword cur = 0;

    if (num == -1)
    {
        return NULL;
    }

    dbgAssert(num < list->num);
    node = list->head;

    while (cur < num)
    {
        node = node->next;
        cur++;
    }
    dbgAssert(cur == num);

    return listGetStructOfNode(node);
}

void Save_CameraCommand(CameraCommand *cameracommand)
{
    SaveChunk *chunk;
    CameraCommand *savecc;
    sdword num = cameracommand->camerastack.num;
    sdword cur = 0;
    InfoChunkContents infocontents;
    Node *node = cameracommand->camerastack.head;
    CameraStackEntry *currententry;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_CAMERACOMMAND,sizeof(CameraCommand),cameracommand);
    savecc = (CameraCommand *)chunkContents(chunk);

    // fix savecc
    savecc->currentCameraStack = ConvertPointerInListToNum(&savecc->camerastack,savecc->currentCameraStack);
    savecc->actualcamera.playerowner = SavePlayerToPlayerIndex(savecc->actualcamera.playerowner);

    savecc->dontFocusOnMe = ConvertPointerInListToNum(&savecc->camerastack,savecc->dontFocusOnMe);

    SaveThisChunk(chunk);

    infocontents.info = num;
    SaveThisChunk(CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));

    while (node != NULL)
    {
        currententry = (CameraStackEntry *)listGetStructOfNode(node);

        Save_CameraStackEntry(currententry);
        cur++;

        node = node->next;
    }

    dbgAssert(cur == num);

    memFree(chunk);
}

void Load_CameraCommand(CameraCommand *cameracommand)
{
    SaveChunk *chunk;
    CameraCommand *loadcc;
    SaveChunk *infochunk;
    sdword num;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_CAMERACOMMAND,sizeof(CameraCommand));

    loadcc = (CameraCommand *)chunkContents(chunk);
    memcpy(cameracommand,loadcc,sizeof(CameraCommand));

    memFree(chunk);

    infochunk = LoadNextChunk();
    VerifyChunk(infochunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(infochunk))->info;

    memFree(infochunk);

    listInit(&cameracommand->camerastack);
    for (i=0;i<num;i++)
    {
        Load_CameraStackEntry(&cameracommand->camerastack);
    }

    // fix cameracommand

    cameracommand->currentCameraStack = ConvertNumToPointerInList(&cameracommand->camerastack,(sdword)cameracommand->currentCameraStack);
    cameracommand->actualcamera.playerowner = SavePlayerIndexToPlayer(cameracommand->actualcamera.playerowner);

    cameracommand->dontFocusOnMe = ConvertNumToPointerInList(&cameracommand->camerastack,(sdword)cameracommand->dontFocusOnMe);
}

void SaveLinkedListOfInsideShips(LinkedList *list)
{
    sdword num = list->num;
    SaveChunk *chunk;
    SaveLLSpaceObj *savecontents;
    Node *node = list->head;
    Ship *ship;
    sdword cur = 0;

    chunk = CreateChunk(LINKEDLISTSPACEOBJS,sizeofSaveLLSpaceObj(num),NULL);

    savecontents = chunkContents(chunk);
    savecontents->num = num;

    while (node != NULL)
    {
        ship = ((InsideShip *)listGetStructOfNode(node))->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        savecontents->ID[cur] = SpaceObjRegistryGetID((SpaceObj *)ship);
        dbgAssert(savecontents->ID[cur] != -1);
        cur++;

        node = node->next;
    }

    dbgAssert(cur == num);

    SaveThisChunk(chunk);

    memFree(chunk);
}

void LoadLinkedListOfInsideShips(LinkedList *list)
{
    SaveChunk *chunk;
    SaveLLSpaceObj *loadcontents;
    sdword num;
    sdword cur = 0;
    InsideShip *insideShip;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,LINKEDLISTSPACEOBJS);

    loadcontents = (struct SaveLLSpaceObj *)chunkContents(chunk);
    num = loadcontents->num;

    dbgAssert(sizeofSaveLLSpaceObj(num) == chunk->contentsSize);

    listInit(list);

    while (cur < num)
    {
        insideShip = memAlloc(sizeof(InsideShip),"InsideShip",0);
        insideShip->ship = (Ship*)loadcontents->ID[cur++];
        if (insideShip->ship != -1)
        {
            listAddNode(list,&insideShip->node,insideShip);
        }
    }

    memFree(chunk);
}

void FixLinkedListOfInsideShips(LinkedList *list)
{
    Node *node = list->head;
    InsideShip *insideShip;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        insideShip->ship = SpaceObjRegistryGetShip((sdword)insideShip->ship);
        dbgAssert(insideShip->ship != NULL);
        dbgAssert(insideShip->ship->objtype == OBJ_ShipType);
        node = node->next;
    }
}

void SaveLinkedListOfResearchTopics(LinkedList *list)
{
    InfoChunkContents infocontents;
    SaveChunk *chunk;
    Node *node = list->head;
    sdword cur = 0;

    infocontents.info = list->num;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));
    memFree(chunk);

    while (node != NULL)
    {
        cur++;
        chunk = CreateChunk(BASIC_STRUCTURE|SAVE_RESEARCHTOPIC,sizeof(ResearchTopic),listGetStructOfNode(node));
        SaveThisChunk(chunk);
        memFree(chunk);

        node = node->next;
    }

    dbgAssert(cur == list->num);
}

void LoadLinkedListOfResearchTopics(LinkedList *list)
{
    SaveChunk *chunk;
    sdword num;
    sdword i;
    ResearchTopic *newtopic;

    listInit(list);

    chunk = LoadNextChunk();
    VerifyChunk(chunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(chunk))->info;

    memFree(chunk);

    for (i=0;i<num;i++)
    {
        chunk = LoadNextChunk();
        VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_RESEARCHTOPIC,sizeof(ResearchTopic));
        newtopic = (ResearchTopic *)memAlloc(sizeof(ResearchTopic),"ResearchTopic",NonVolatile);
        memcpy(newtopic,chunkContents(chunk),sizeof(ResearchTopic));
        listAddNode(list, &newtopic->link, newtopic);
        memFree(chunk);
    }
}

void Prefix_PlayerResearchInfo(PlayerResearchInfo *researchinfo)
{
    sdword i;

    for (i=0;i<NUM_RESEARCHLABS;i++)
    {
        if (researchinfo->researchlabs[i].labstatus == LS_RESEARCHITEM)
        {
            researchinfo->researchlabs[i].topic = ConvertPointerInListToNum(&researchinfo->listoftopics,researchinfo->researchlabs[i].topic);
            dbgAssert(researchinfo->researchlabs[i].topic != -1);
        }
        else
        {
            researchinfo->researchlabs[i].topic = -1;
        }
    }
}

void Save_PlayerResearchInfo(PlayerResearchInfo *researchinfo)
{
    SaveLinkedListOfResearchTopics(&researchinfo->listoftopics);
}

void Load_PlayerResearchInfo(Player *player,PlayerResearchInfo *researchinfo)
{
    sdword i;

    rmInitializeResearchStatics(player);

    LoadLinkedListOfResearchTopics(&researchinfo->listoftopics);

    for (i=0;i<NUM_RESEARCHLABS;i++)
    {
        if (researchinfo->researchlabs[i].labstatus == LS_RESEARCHITEM)
        {
            researchinfo->researchlabs[i].topic = ConvertNumToPointerInList(&researchinfo->listoftopics,(sdword)researchinfo->researchlabs[i].topic);
            dbgAssert(researchinfo->researchlabs[i].topic != NULL);
        }
        else
        {
            researchinfo->researchlabs[i].topic = NULL;
        }
    }
}

void SaveMineFormationInfo(MineFormationInfo *mineFormationInfo)
{
    SaveChunk *chunk;
    MineFormationInfo *savecontents;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_MINEFORMATIONINFO,sizeof(MineFormationInfo),mineFormationInfo);
    savecontents = (MineFormationInfo *)chunkContents(chunk);

    savecontents->playerowner = SavePlayerToPlayerIndex(savecontents->playerowner);
    savecontents->effect = NULL;

    SaveThisChunk(chunk);
    memFree(chunk);

    SaveLinkedListOfSpaceObjs(&mineFormationInfo->MineList);
}

MineFormationInfo *LoadMineFormationInfo(void)
{
    SaveChunk *chunk;
    MineFormationInfo *mineFormationInfo;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_MINEFORMATIONINFO,sizeof(MineFormationInfo));

    mineFormationInfo = memAlloc(sizeof(MineFormationInfo),"MineFormationInfo",NonVolatile);
    memcpy(mineFormationInfo,chunkContents(chunk),sizeof(MineFormationInfo));
    memFree(chunk);

    mineFormationInfo->playerowner = SavePlayerIndexToPlayer(mineFormationInfo->playerowner);

    LoadLinkedListOfSpaceObjs(&mineFormationInfo->MineList,MineAddToMineListCB);

    return mineFormationInfo;
}

void SaveMineFormationList(LinkedList *list)
{
    InfoChunkContents infocontents;
    SaveChunk *chunk;
    Node *node = list->head;
    sdword cur = 0;
    MineFormationInfo *mineFormationInfo;

    infocontents.info = list->num;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));
    memFree(chunk);

    while (node != NULL)
    {
        cur++;
        mineFormationInfo = (MineFormationInfo *)listGetStructOfNode(node);
        SaveMineFormationInfo(mineFormationInfo);
        node = node->next;
    }

    dbgAssert(cur == list->num);
}

void LoadMineFormationList(LinkedList *list)
{
    SaveChunk *chunk;
    sdword num;
    sdword i;
    MineFormationInfo *mineFormationInfo;

    listInit(list);

    chunk = LoadNextChunk();
    VerifyChunk(chunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(chunk))->info;

    memFree(chunk);

    for (i=0;i<num;i++)
    {
        mineFormationInfo = LoadMineFormationInfo();
        listAddNode(list, &mineFormationInfo->FormLink, mineFormationInfo);
    }
}

void SaveRetreatAtom(RetreatAtom *retreat)
{
    SaveChunk *chunk;
    RetreatAtom *savecontents;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_RETREATATOM,sizeof(RetreatAtom),retreat);
    savecontents = (RetreatAtom *)chunkContents(chunk);

    savecontents->retreater = SpaceObjRegistryGetID((SpaceObj *)savecontents->retreater);
    savecontents->fleeingfrom = ConvertPointerInListToNum(&universe.mainCommandLayer.todolist,savecontents->fleeingfrom);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveRetreatList(LinkedList *list)
{
    sdword num = list->num;
    sdword cur = 0;
    Node *node = list->head;

    SaveInfoNumber(num);

    while (node != NULL)
    {
        cur++;
        SaveRetreatAtom((RetreatAtom *)listGetStructOfNode(node));

        node = node->next;
    }

    dbgAssert(cur == num);
}

RetreatAtom *LoadRetreatAtom()
{
    SaveChunk *chunk;
    RetreatAtom *retreat;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_RETREATATOM,sizeof(RetreatAtom));
    retreat = (RetreatAtom *)memAlloc(sizeof(RetreatAtom),"Retreat Data", NonVolatile);
    memcpy(retreat,chunkContents(chunk),sizeof(RetreatAtom));
    memFree(chunk);

    retreat->retreater = SpaceObjRegistryGetShip((sdword)retreat->retreater);
    retreat->fleeingfrom = ConvertNumToPointerInList(&universe.mainCommandLayer.todolist,(sdword)retreat->fleeingfrom);

    return retreat;
}

void LoadRetreatList(LinkedList *list)
{
    sdword num;
    sdword i;
    RetreatAtom *retreat;

    num = LoadInfoNumber();

    listInit(list);

    for (i=0;i<num;i++)
    {
        retreat = LoadRetreatAtom();
        listAddNode(list,&retreat->retreatLink,retreat);
    }
}

void SaveAttackAtom(AttackAtom *attackAtom)
{
    SaveChunk *chunk;
    AttackAtom *savecontents;
    sdword size;
    sdword i;
    sdword numShips;

    size = sizeof(AttackAtom) + sizeof(ShipPtr)*(attackAtom->attackerList.numShips-1);

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_ATTACKATOM,size,attackAtom);
    savecontents = (AttackAtom *)chunkContents(chunk);

    savecontents->retreater = SpaceObjRegistryGetID((SpaceObj *)savecontents->retreater);

    numShips = savecontents->attackerList.numShips;
    for (i=0;i<numShips;i++)
    {
        savecontents->attackerList.ShipPtr[i] = SpaceObjRegistryGetID((SpaceObj *)savecontents->attackerList.ShipPtr[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveAttackMemory(LinkedList *list)
{
    sdword num = list->num;
    sdword cur = 0;
    Node *node = list->head;

    SaveInfoNumber(num);

    while (node != NULL)
    {
        cur++;
        SaveAttackAtom((AttackAtom *)listGetStructOfNode(node));

        node = node->next;
    }

    dbgAssert(cur == num);
}

AttackAtom *LoadAttackAtom()
{
    SaveChunk *chunk;
    AttackAtom *attackAtom;
    AttackAtom *loadedAttackAtom;
    sdword size;
    sdword i;
    sdword numShips;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_ATTACKATOM);
    loadedAttackAtom = (AttackAtom *)chunkContents(chunk);

    numShips = loadedAttackAtom->attackerList.numShips;
    size = sizeof(AttackAtom) + sizeof(ShipPtr)*(numShips-1);

    dbgAssert(size == chunk->contentsSize);

    attackAtom = (AttackAtom *)memAlloc(size,"tacticsattackmemory",NonVolatile);
    memcpy(attackAtom,loadedAttackAtom,size);

    attackAtom->retreater = SpaceObjRegistryGetShip((sdword)attackAtom->retreater);

    for (i=0;i<numShips;i++)
    {
        attackAtom->attackerList.ShipPtr[i] = SpaceObjRegistryGetShip((sdword)attackAtom->attackerList.ShipPtr[i]);
    }

    memFree(chunk);
    return attackAtom;
}

void LoadAttackMemory(LinkedList *list)
{
    sdword num;
    sdword i;
    AttackAtom *attack;

    num = LoadInfoNumber();

    listInit(list);

    for (i=0;i<num;i++)
    {
        attack = LoadAttackAtom();
        listAddNode(list,&attack->attackLink,attack);
    }
}

void SaveResourceVolumeCB(void *stuff)
{
    SaveStructureOfSize(stuff,sizeof(ResourceVolume));
}

void LoadResourceVolumeCB(LinkedList *list)
{
    ResourceVolume *resvol = LoadStructureOfSize(sizeof(ResourceVolume));

    listAddNode(list,&resvol->link,resvol);
}

void SaveUniverse()
{
    SaveChunk *chunk;
    Universe *savecontents;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_UNIVERSE,sizeof(Universe),&universe);
    savecontents = (Universe *)chunkContents(chunk);

    savecontents->curPlayerPtr = SavePlayerToPlayerIndex(universe.curPlayerPtr);

    for (i=0;i<universe.numPlayers;i++)
    {
        savecontents->players[i].PlayerMothership = (sdword)SpaceObjRegistryGetID((SpaceObj *)universe.players[i].PlayerMothership);
        Prefix_PlayerResearchInfo(&savecontents->players[i].researchinfo);
    }

    for (i=0;i<UNIV_NUMBER_WORLDS;i++)
    {
        savecontents->world[i] = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecontents->world[i]);
    }

    SaveThisChunk(chunk);

    Save_CommandLayer(&universe.mainCommandLayer);
    Save_CameraCommand(&universe.mainCameraCommand);

    SaveLinkedListOfSpaceObjs(&universe.RenderList);
    SaveLinkedListOfSpaceObjs(&universe.MinorRenderList);

    SaveLinkedListOfSpaceObjs(&universe.SpaceObjList);
    SaveLinkedListOfSpaceObjs(&universe.MinorSpaceObjList);
    SaveLinkedListOfSpaceObjs(&universe.ShipList);
    SaveLinkedListOfSpaceObjs(&universe.BulletList);
    SaveLinkedListOfSpaceObjs(&universe.ResourceList);
    SaveLinkedListOfSpaceObjs(&universe.DerelictList);
    SaveLinkedListOfSpaceObjs(&universe.ImpactableList);
    SaveLinkedListOfSpaceObjs(&universe.MissileList);
    SaveMineFormationList(&universe.MineFormationList);

    // Delete Lists assumed to be empty, because SaveUniverse is never called in the middle of a univupdate
    dbgAssert(universe.DeleteMissileList.num == 0);
    dbgAssert(universe.DeleteResourceList.num == 0);
    dbgAssert(universe.DeleteDerelictList.num == 0);
    dbgAssert(universe.DeleteShipList.num == 0);

    SaveRetreatList(&universe.RetreatList);
    SaveAttackMemory(&universe.AttackMemory);

    SaveLinkedListOfStuff(&universe.ResourceVolumeList,SaveResourceVolumeCB);

    SaveGrowSelection(&universe.HousekeepShipList);

    for (i=0;i<universe.numPlayers;i++)
    {
        Save_PlayerResearchInfo(&universe.players[i].researchinfo);
    }

    memFree(chunk);
}

void FillOutCollBlobList(LinkedList *list)
{
    sdword numBlobs = BlobRegistry.selection->numShips;
    sdword i;
    blob *tblob;

    listInit(list);
    for (i=0;i<numBlobs;i++)
    {
        tblob = (blob *)BlobRegistry.selection->ShipPtr[i];
        listAddNode(list,&tblob->node,tblob);
    }
}

void LoadUniverse()
{
    SaveChunk *chunk;
    sdword i;

    if (universe.star3dinfo)
    {
        star3dClose(universe.star3dinfo);
        universe.star3dinfo = NULL;
    }

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_UNIVERSE,sizeof(Universe));

    memcpy(&universe,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    Load_CommandLayer(&universe.mainCommandLayer);
    Load_CameraCommand(&universe.mainCameraCommand);

    LoadLinkedListOfSpaceObjs(&universe.RenderList,RenderListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.MinorRenderList,MinorRenderListAddCB);

    LoadLinkedListOfSpaceObjs(&universe.SpaceObjList,SpaceObjListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.MinorSpaceObjList,MinorSpaceObjListAddCB);

    LoadLinkedListOfSpaceObjs(&universe.ShipList,ShipListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.BulletList,BulletListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.ResourceList,ResourceListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.DerelictList,DerelictListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.ImpactableList,ImpactableListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.MissileList,MissileListAddCB);
    LoadMineFormationList(&universe.MineFormationList);

    listInit(&universe.DeleteMissileList);
    listInit(&universe.DeleteResourceList);
    listInit(&universe.DeleteDerelictList);
    listInit(&universe.DeleteShipList);

    FillOutCollBlobList(&universe.collBlobList);

    LoadRetreatList(&universe.RetreatList);
    LoadAttackMemory(&universe.AttackMemory);

    LoadLinkedListOfStuff(&universe.ResourceVolumeList,LoadResourceVolumeCB);

    LoadGrowSelectionAndFix(&universe.HousekeepShipList);

    universe.star3dinfo = star3dInit(NUM_STARS, universe.radius - 1.0f, universe.radius);

    universe.curPlayerPtr = SavePlayerIndexToPlayer(universe.curPlayerPtr);

    for (i=0;i<universe.numPlayers;i++)
    {
        universe.players[i].PlayerMothership = SpaceObjRegistryGetShip((sdword)universe.players[i].PlayerMothership);
        Load_PlayerResearchInfo(&universe.players[i],&universe.players[i].researchinfo);
    }

    memset(&universe.players[MAX_MULTIPLAYER_PLAYERS],0,sizeof(Player));
    universe.players[MAX_MULTIPLAYER_PLAYERS].playerIndex = MAX_MULTIPLAYER_PLAYERS;

    for (i=0;i<UNIV_NUMBER_WORLDS;i++)
    {
        universe.world[i] = (Derelict *)SpaceObjRegistryGetObj((sdword)universe.world[i]);
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

