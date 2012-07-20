/*

	Tutor.h - types & protoypes for the Tutorial System

*/

#ifndef TUTOR_H
#define TUTOR_H

#include "Types.h"
#include "ShipSelect.h"
#include "ShipDefs.h"
#include "FEFlow.h"
#include "Volume.h"

/*=============================================================================
    Definitions:
=============================================================================*/
//tutorial pointer types
#define TUT_PointerTypeNone         0
#define TUT_PointerTypeXY           1
#define TUT_PointerTypeShip         2
#define TUT_PointerTypeShips        3
#define TUT_PointerTypeShipHealth   4
#define TUT_PointerTypeShipGroup    5
#define TUT_PointerTypeRegion       6
#define TUT_PointerTypeAIVolume     7

#define TUT_PointerPulseInc         8           //pointer pulse rate
#define TUT_PointerPulseMin         100         //pointer pulse minimum

//length of a pointer name string
#define TUT_PointerNameLength       60
#define TUT_PointerNameMax          (TUT_PointerNameLength - 1)

#define TUT_NumberPointers          16  //max number of pointers

#define TUT_ArrowheadAngle          (PI / 12.0f)
#define TUT_ArrowheadLength         13.0f
#define TUT_ShipCircleSizeMin       primScreenToGLScaleX(4)

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
	udword	bKASFrame:1;	// For internal use only by the game
	udword	bGameRunning:1;	// Turned off, it pauses the universe, but not KAS

	// The following bit fields enable their respective commands
	udword	bBuildManager:1;
	udword	bSensorsManager:1;
	udword	bResearchManager:1;
    udword  bPauseGame:1;
	udword	bDock:1;
    udword  bFormation:1;
	udword	bLaunch:1;
	udword	bMove:1;
    udword  bMoveIssue:1;
	udword	bAttack:1;
    udword  bHarvest:1;
    udword  bCancelCommand:1;
    udword  bScuttle:1;
    udword  bRetire:1;
	udword	bClickSelect:1;
	udword	bBandSelect:1;
	udword	bCancelSelect:1;
	udword	bSpecial:1;
	udword	bBuildBuildShips:1;
	udword	bBuildPauseJobs:1;
	udword	bBuildCancelJobs:1;
	udword	bBuildClose:1;
    udword  bBuildArrows:1;
    udword  bSensorsClose:1;
	udword	bContextMenus:1;
    udword  bContextFormDelta:1;
    udword  bContextFormBroad:1;
    udword  bContextFormX:1;
    udword  bContextFormClaw:1;
    udword  bContextFormWall:1;
    udword  bContextFormSphere:1;
    udword  bContextFormCustom:1;
    udword  bEvasive:1;
    udword  bNeutral:1;
    udword  bAgressive:1;
	udword	bTaskbarOpen:1;
	udword	bTaskbarClose:1;
	udword	bResearchSelectTech:1;
	udword	bResearchSelectLab:1;
	udword	bResearchResearch:1;
	udword	bResearchClearLab:1;
	udword	bResearchClose:1;
	udword	bFocus:1;
	udword	bFocusCancel:1;
	udword  bMothershipFocus:1;
	udword	bLaunchSelectShips:1;
	udword	bLaunchLaunch:1;
	udword	bLaunchLaunchAll:1;
	udword	bLaunchClose:1;
} tutGameEnableFlags;

//structure of a named pointer
typedef struct
{
    char name[TUT_PointerNameLength];           //name of pointer
    sdword pointerType;                         //type of object the pointer points to
    sdword x, y;                                //if the pointer points at a point on screen
    ShipPtr ship;                               //if the pointer points to a ship
    rectangle rect;                             //if the pointer points to a rectangle of some sort
    Volume *volume;                             //if it points to an AI point
    SelectCommand *selection;                   //optional selection of ships
}
tutpointer;

extern bool tutPointersDrawnThisFrame;

/*=============================================================================
    Functions:
=============================================================================*/
void tutPreInitTutorial(char *dirfile, char *levelfile);
void tutInitTutorial(char *dirfile, char *levelfile);

extern tutGameEnableFlags tutEnable;
extern char tutBuildRestrict[TOTAL_STD_SHIPS];
extern ShipType tutFEContextMenuShipType;
extern ShipPtr tutPointerShip;
extern rectangle *tutPointerShipHealthRect;
extern rectangle *tutPointerShipGroupRect;

void tutSaveLesson(sdword Num, char *pName);

void tutSaveTutorialGame(void);
void tutLoadTutorialGame(void);

void tutTutorial1(char *name, featom *atom);

void tutSetPointerTargetOff(void);
void tutSetPointerTargetXY(char *name, sdword x, sdword y);
void tutSetPointerTargetXYRight(char *name, sdword x, sdword y);
void tutSetPointerTargetXYBottomRight(char *name, sdword x, sdword y);
void tutSetPointerTargetXYTaskbar(char *name, sdword x, sdword y);
void tutSetPointerTargetXYFE(char *name, sdword x, sdword y);
void tutSetPointerTargetShip(char *name, ShipPtr ship);
void tutSetPointerTargetShipSelection(char *name, SelectCommand *ships);
void tutSetPointerTargetShipHealth(char *name, ShipPtr ship);
void tutSetPointerTargetShipGroup(char *name, ShipPtr ship);
void tutPlayerShipDied(ShipPtr ship);
void tutSetPointerTargetFERegion(char *name, char *pAtomName);
void tutSetPointerTargetRect(char *name, sdword x0, sdword y0, sdword x1, sdword y1);
void tutSetPointerTargetAIVolume(char *name, Volume *volume);
void tutRemovePointerByName(char *name);
void tutRemoveAllPointers(void);
void tutRemoveAllPointers(void);
void tutDrawTextPointers(rectangle *pRect);

void tutSetTextDisplayBox(sdword x, sdword y, sdword width, sdword height, bool bScale);
void tutSetTextDisplayBoxToSubtitleRegion(void);
void tutShowText(char *szText);
void tutHideText(void);
void tutShowNextButton(void);
void tutHideNextButton(void);
sdword tutNextButtonClicked(void);
void tutShowBackButton(void);
void tutHideBackButton(void);
void tutShowPrevButton(void);
void tutShowImages(char *szImages);
void tutHideImages(void);

void tutStartup(void);
void tutShutdown(void);
void tutInitialize(void);
void tutUnInitialize(void);

void tutEnableEverything(void);
void tutDisableEverything(void);
void tutSetEnableFlags(char *pFlagString, long Val);

void tutBuilderSetRestrictions(char *pShipTypes, bool bRestricted);
void tutBuilderRestrictAll(void);
void tutBuilderRestrictNone(void);


sdword tutIsBuildShipRestricted(sdword shipType);
sdword tutSelectedContainsShipTypes(char *pShipTypes);

void tutGameMessage(char *commandName);
sdword tutGameSentMessage(char *commandName);
void tutResetGameMessageQueue(void);
sdword tutContextMenuDisplayedForShipType(char *pShipType);
void  tutResetContextMenuShipTypeTest(void);

sdword tutBuildManagerShipTypeInBatchQueue(char *pShipType);
sdword tutBuildManagerShipTypeInBuildQueue(char *pShipType);
sdword tutBuildManagerShipTypeSelected(char *pShipType);

sdword tutCameraFocusedOnShipType(char *pShipTypes);

#endif
