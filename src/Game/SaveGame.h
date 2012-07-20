/*=============================================================================
    Name    : SaveGame.h
    Purpose : Definitions for SaveGame.c

    Created date by user
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "SpaceObj.h"
#include "ShipSelect.h"

/* Save game version for original Homeworld. */
/*#define SAVE_VERSION_NUMBER             0x10ad0027*/
/* Save game version for Homeworld SDL. */
#define SAVE_VERSION_NUMBER             0x10ad0028

#define BASIC_STRUCTURE                 0x80000000
#define VARIABLE_STRUCTURE              0x40000000
#define INFO_CHUNK                      0x20000000
#define LINKEDLISTSPACEOBJS             0x10000000
#define AIPLAYER                        0x08000000

#define SAVE_PING                       0x00001000
#define IDTOPTRTABLE                    0x00000f00
#define SAVE_SALVAGEINFO                0x00000e00
#define SAVESTRING                      0x00000d00
#define PATH                            0x00000c00
#define AITEAMMOVE                      0x00000b00
#define AITEAM                          0x00000a00
#define SELECTION                       0x00000900
#define SAVE_NEBTENDRIL                 0x00000800
#define SAVE_NEBCHUNK                   0x00000700
#define SAVE_SHIPSINPROGRESS            0x00000600
#define SAVE_ATTACKATOM                 0x00000500
#define SAVE_RETREATATOM                0x00000400
#define SAVE_SLAVEINFO                  0x00000300
#define SAVE_STATICINFO                 0x00000200
#define SAVE_MINEFORMATIONINFO          0x00000100
#define SAVE_SHIPSINSIDEME              0x000000f0
#define SAVE_MILITARYSLOT               0x000000e0
#define SAVE_MILITARYPARADE             0x000000d0
#define SAVE_RESEARCHTOPIC              0x000000c0
#define SAVE_CLOUDSYSTEM                0x000000b0
#define SAVE_COMMANDTODO                0x000000a0
#define SAVE_CAMERACOMMAND              0x00000090
#define SAVE_CAMERASTACKENTRY           0x00000080
#define SAVE_SHIPSINGLEPLAYERGAMEINFO   0x00000070
#define SAVE_DOCKINFO                   0x00000060
#define SAVE_GUNINFO                    0x00000050
#define SAVE_BLOB                       0x00000040
#define SAVE_ATTACKTARGETS              0x00000030
#define SAVE_UNIVERSE                   0x00000020
#define SAVE_SPACEOBJ                   0x00000010


#define SAVE_OBJTYPEMASK                0x0000000f

typedef sdword TypeOfSaveChunk;     // STRUCTURE | SAVE_xxx     e.g. BASIC_STRUCTURE|SAVE_UNIVERSE

typedef struct
{
    TypeOfSaveChunk type;
    sdword contentsSize;
    // chunk contents goes here
} SaveChunk;

typedef struct
{
    sdword info;
} InfoChunkContents;

#define chunkContents(c) ((void *) (((ubyte *)c) + sizeof(SaveChunk)))

#define sizeofSaveChunk(n) (sizeof(SaveChunk) + n)

#define VerifyChunk(c,t,s)              \
    dbgAssert(c);                       \
    dbgAssert((c)->type == (t));        \
    dbgAssert((c)->contentsSize == (s))

#define VerifyChunkNoSize(c,t)          \
    dbgAssert(c);                       \
    dbgAssert((c)->type == (t));

bool SaveGame(char *filename);
void LoadGame(char *filename);
void PreLoadGame(char *filename);

void SaveThisChunk(SaveChunk *thischunk);
SaveChunk *CreateChunk(TypeOfSaveChunk type,sdword contentsSize,void *contents);
SaveChunk *LoadNextChunk();

sdword SpaceObjRegistryGetID(SpaceObj *obj);
SpaceObj *SpaceObjRegistryGetObj(sdword ID);
Ship *SpaceObjRegistryGetShip(sdword ID);
Resource *SpaceObjRegistryGetResource(sdword ID);
Bullet *SpaceObjRegistryGetBullet(sdword ID);
TargetPtr SpaceObjRegistryGetTarget(sdword ID);
void SaveInfoNumber(sdword info);
sdword LoadInfoNumber();
void *ConvertNumToPointerInList(LinkedList *list,sdword num);
sdword ConvertPointerInListToNum(LinkedList *list,void *entry);
void SpaceObjRegistryRegister(SpaceObj *obj);

void SaveSelection(SpaceObjSelection *selection);
SpaceObjSelection *LoadSelection(void);
SpaceObjSelection *LoadSelectionAndFix(void);
void FixSelection(SpaceObjSelection *selection);

void SaveGrowSelection(GrowSelection *grow);
void LoadGrowSelection(GrowSelection *grow);
void LoadGrowSelectionAndFix(GrowSelection *grow);
void FixGrowSelection(GrowSelection *grow);

void SaveMaxSelection(MaxSelection *maxselection);
void LoadMaxSelectionAndFix(MaxSelection *maxselection);

void SaveLinkedListOfInsideShips(LinkedList *list);
void LoadLinkedListOfInsideShips(LinkedList *list);
void FixLinkedListOfInsideShips(LinkedList *list);

#define SavePlayerToPlayerIndex(p) (((p) == NULL) ? -1 : (p)->playerIndex)
#define SavePlayerIndexToPlayer(i) ((((sdword)(i)) == -1) ? NULL : &universe.players[((sdword)(i))])

typedef void (*SaveStuffInLinkedListCB)(void *stuff);
typedef void (*LoadStuffInLinkedListCB)(LinkedList *list);
typedef void (*FixStuffInLinkedListCB)(void *stuff);

void SaveLinkedListOfStuff(LinkedList *list,SaveStuffInLinkedListCB savestuffCB);
void LoadLinkedListOfStuff(LinkedList *list,LoadStuffInLinkedListCB loadstuffCB);
void FixLinkedListOfStuff(LinkedList *list,FixStuffInLinkedListCB fixstuffCB);

void SaveStructureOfSize(void *structure,sdword size);
void *LoadStructureOfSize(sdword size);
void LoadStructureOfSizeToAddress(void *address,sdword size);

void Save_String(char *string);
char *Load_String(void);
void Load_StringToAddress(char *addr);

#define VERIFYSAVEFILE_OK               0
#define VERIFYSAVEFILE_ERROROPENING     -1
#define VERIFYSAVEFILE_BADVERSION       -2

sdword VerifySaveFile(char *filename);

bool LoadInfoNumberOptional(sdword *info);
SaveChunk *LoadNextChunkSafe();


