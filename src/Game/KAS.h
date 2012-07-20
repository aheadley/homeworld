#ifndef __KAS_H
#define __KAS_H

#include "AIUtilities.h"
#include "ShipSelect.h"
#include "Vector.h"
#include "Volume.h"
#include "KASFunc.h"

// run-time scoping for variables, timers, etc.
enum {
    KAS_SCOPE_MISSION,
    KAS_SCOPE_FSM,
    KAS_SCOPE_STATE};

typedef void (*KASInitFunction) (void);
typedef void (*KASWatchFunction) (void);

//  these have corresponding #defines in kas2c.h
#define KAS_MISSION_NAME_MAX_LENGTH 47
#define KAS_FSM_NAME_MAX_LENGTH 47
#define KAS_STATE_NAME_MAX_LENGTH 47

//  user variables for SHIPS
#define KASSELECTION_ALLOC_INCREMENT 16
#define KASSELECTION_MAX_LABEL_LENGTH 47
typedef struct {
    char label[KASSELECTION_MAX_LABEL_LENGTH+1];
    GrowSelection shipList;
} KasSelection;

//
//  labelled entities in mission layout file
//
#define KAS_MAX_LABEL_LENGTH 47
#define KAS_LABELLED_ENTITY_ALLOC_INCREMENT 16
//  team labels are stored in AITeam struct
typedef struct {
    char label[KAS_MAX_LABEL_LENGTH+1];
    Path *path;
} LabelledPath;
typedef struct {
    char label[KAS_MAX_LABEL_LENGTH+1];
    hvector *hvector;
} LabelledVector;
typedef struct {
    char label[KAS_MAX_LABEL_LENGTH+1];
    Volume *volume;
} LabelledVolume;


/*=============================================================================
    Functions
=============================================================================*/
//  to fill in table of labelled mission layout entities
void kasLabelsInit();
//void kasLabelledAITeamAdd(char *label);
Path *kasLabelledPathAdd(char *label, sdword numPoints, sdword closed);
hvector *kasLabelledVectorAdd(char *label, real32 x, real32 y, real32 z,real32 w);
Volume *kasLabelledVolumeAdd(char *label);

void kasMissionStart(char *name, KASInitFunction initFunction, KASWatchFunction watchFunction);
void kasExecute(void);

// keywords of KAS language
void kasJump(char *stateName, KASInitFunction initFunction, KASWatchFunction watchFunction);
void kasFSMCreate(char *fsmName, KASInitFunction initFunction, KASWatchFunction watchFunction, struct AITeam *team);

// labels: resolves references from script to mission layout file
struct AITeam *kasAITeamPtr(char *label);
GrowSelection *kasAITeamShipsPtr(char *label);
hvector *kasShipsVectorPtr(char *label);
hvector *kasTeamsVectorPtr(char *label);
hvector *kasVolumeVectorPtr(char *label);
hvector *kasThisTeamsVectorPtr(void);
Path   *kasPathPtr(char *label);
Path *kasPathPtrNoErrorChecking(char *label);
Volume *kasVolumePtr(char *label);
hvector *kasVectorPtr(char *label);
hvector *kasVectorPtrIfExists(char *label);
GrowSelection *kasGrowSelectionPtr(char *label);

GrowSelection *kasGetGrowSelectionPtrIfExists(char *label);

void kasShipDied(Ship *ship);

void kasGrowSelectionClear(GrowSelection *ships);

void kasLabelledEntitiesDestroy(void);

void kasAddShipToTeam(Ship *ship,char *str);

#define KAS_TEAM_NAME_MAX_LENGTH 47
char *kasAITeamName(struct AITeam *team, char *teamName);

void kasTakeADump(void);
void kasDebugDraw(void);
#define KAS_DEBUG_TEXTCOLOR            colRGB(64, 255, 196)

void kasTeamDied(struct AITeam *team);

void kasClose();
void kasInit();

// Save Game Stuff
void kasSave(void);
void kasLoad(void);
void *kasConvertOffsetToFuncPtr(sdword offset);
sdword kasConvertFuncPtrToOffset(void *func);


#endif
