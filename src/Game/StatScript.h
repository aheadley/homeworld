/*=============================================================================
    Name    : statscript.h
    Purpose : Definitions for statscript.c (Static Scripting Utilities)

    Created 6/24/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___STATSCRIPT_H
#define ___STATSCRIPT_H

#include "Types.h"
#include "ObjTypes.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef void (*setVarCback)(char *directory,char *field,void *dataToFillIn);

typedef struct
{
    char *name;
    setVarCback setVarCB;
    udword offset1;     // should really be 1 offset, but I can't get rid of this strange compiler error
    udword offset2;
} scriptStructEntry;

typedef struct
{
    char *name;
    setVarCback setVarCB;
    void *dataPtr;
} scriptEntry;

///////DATA!!!!
extern char globalScriptFileName[50];  //file name of file loaded in a script callback function


/*=============================================================================
    Macros
=============================================================================*/

// for filling in scriptEntry
#define makeEntry(var,callback) { str$(var),callback,&(var) }
#define endEntry { NULL, NULL, 0 }

/*=============================================================================
    Functions:
=============================================================================*/

struct StaticInfoHealthGuidanceShipDerelict;
struct ShipStaticInfo;

void RemoveCommasFromString(char *field);
void StripTrailingSpaces(char *value);

void scriptSetRGBCB(char *directory,char *field,void *dataToFillIn);
void scriptSetRGBACB(char *directory,char *field,void *dataToFillIn);
void scriptSetReal32CB(char *directory,char *field,void *dataToFillIn);
void scriptSetReal32CB_ARRAY(char *directory,char *field,void *dataToFillIn);
void scriptSetReal32SqrCB(char *directory,char *field,void *dataToFillIn);
void scriptSetSbyteCB(char *directory,char *field,void *dataToFillIn);
void scriptSetUbyteCB(char *directory,char *field,void *dataToFillIn);
void scriptSetSwordCB(char *directory,char *field,void *dataToFillIn);
void scriptSetUwordCB(char *directory,char *field,void *dataToFillIn);
void scriptSetSdwordCB(char *directory,char *field,void *dataToFillIn);
void scriptSetUdwordCB(char *directory,char *field,void *dataToFillIn);
bool scriptStringToBool(char *boolString);
void scriptSetBool8(char *directory,char *field,void *dataToFillIn);
void scriptSetBool(char *directory,char *field,void *dataToFillIn);
void scriptSetBitUdword(char *directory,char *field,void *dataToFillIn);
void scriptSetBitUword(char *directory,char *field,void *dataToFillIn);
void scriptSetStringCB(char *directory,char *field,void *dataToFillIn);
void scriptSetStringPtrCB(char *directory,char *field,void *dataToFillIn);
void scriptSetCosAngCB(char *directory,char *field,void *dataToFillIn);
void scriptSetCosAngSqrCB(char *directory,char *field,void *dataToFillIn);
void scriptSetCosAngCB_ARRAY(char *directory,char *field,void *dataToFillIn);
void scriptSetSinAngCB(char *directory,char *field,void *dataToFillIn);
void scriptSetTanAngCB(char *directory,char *field,void *dataToFillIn);
void scriptSetAngCB(char *directory,char *field,void *dataToFillIn);
void scriptSetGunTypeCB(char *directory,char *field,void *dataToFillIn);
void scriptSetGunSoundTypeCB(char *directory,char *field,void *dataToFillIn);
void scriptSetBulletTypeCB(char *directory,char *field,void *dataToFillIn);
void scriptSetShipClassCB(char *directory,char *field,void *dataToFillIn);
void scriptSetShipRaceCB(char *directory,char *field,void *dataToFillIn);
void scriptSetShipTypeCB(char *directory,char *field,void *dataToFillIn);
void scriptSetVectorCB(char *directory,char *field,void *dataToFillIn);
void scriptSetLWToHWMonkeyVectorCB(char *directory,char *field,void *dataToFillIn);
void scriptSetFormationCB(char *directory,char *field,void *dataToFillIn);
void scriptSetTacticsCB(char *directory,char *field,void *dataToFillIn);
void scriptSetSalvagePointCB(char *directory,char *field,void *dataToFillIn);
void scriptSetSalvageStatCB(char *directory,char *field,void *dataToFillIn);
void scriptSetSalvageStatics(char *directory,char *filename,struct StaticInfoHealthGuidanceShipDerelict *statinfo);
void scriptSetDockOverideStatics(char *directory,char *filename,struct ShipStaticInfo *shipstatinfo);

void scriptSetStruct(char *directory,char *filename,scriptStructEntry info[],ubyte *structureToFillIn);
void scriptSet(char *directory,char *filename,scriptEntry info[]);
void scriptSetFileSystem(char *directory,char *filename,scriptEntry info[]);
void scriptSetGunStatics(char *directory,char *filename,struct ShipStaticInfo *shipstatinfo);
void scriptSetNAVLightStatics(char *directory,char *filename,struct ShipStaticInfo *shipstatinfo);
void scriptSetDockStatics(char *directory,char *filename,struct ShipStaticInfo *statinfo);
struct SphereStaticInfo *scriptSetSphereStaticInfo(char *directory,char *filename);
void scriptSetShipGroupSizeCB(char *directory, char *field, sdword *dataToFillIn);
void scriptSetShipProbCB(char *directory, char *field, real32 *dataToFillIn);
void scriptSetHyperspaceCostCB(char *directory, char *field, real32 *dataToFillIn);
void scriptSetSpecialDoorOffsetCB(char *directory, char *field, real32 *dataToFillIn);
void scriptSetBlobPropertyOverlap(char *directory,char *field,void *dataToFillIn);
void scriptSetBlobBiggestRadius(char *directory,char *field,void *dataToFillIn);

// sets all tweakable global variables in game
void scriptSetTweakableGlobals(void);

void mgGameTypeScriptInit();

#ifndef HW_Release
void CheckValidTacticsClass(TacticsType tactic,ShipClass shipclass,char *field);
#endif

#endif

