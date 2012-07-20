//
//  support code for the KAS language and its missions, FSMs, and states
//
//  implementations of most functions are in kasfunc.c
//

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "glinc.h"
#include "mainrgn.h"
#include "ShipSelect.h"
#include "AITeam.h"
#include "AIPlayer.h"
#include "AIUtilities.h"
#include "AIVar.h"
#include "Timer.h"
#include "Vector.h"
#include "KAS.h"
#include "File.h"
#include "Volume.h"
#include "font.h"
#include "render.h"
#include "prim3d.h"
#include "Objectives.h"
#include "SinglePlayer.h"
#include "SaveGame.h"
#include "HS.h"
#include "CommandWrap.h"

#ifdef _WIN32
#define strcasecmp  _stricmp
#define strncasecmp _strnicmp
#endif

extern sdword objectivesUsed;
#if SP_DEBUGKEYS
//#if SP_DEBUGLEVEL2
// for debug overlay
extern fonthandle selGroupFont2;
extern AIVar **vars;
extern sdword varsUsed;
extern sdword timersUsed;
extern Timer *timers;
sdword mrKASDebugDrawStates = FALSE;
sdword mrKASDebugDrawTimers = FALSE;
sdword mrKASDebugDrawVars = FALSE;
sdword mrKASDebugDrawVolumes = FALSE;
sdword mrKASSkipFactor = 0;
#endif //SP_DEBUGLEVEL2

#define OBJECTIVES_COMPLETE_DELAY 10

// run-time scoping for variables, timers, etc.
sdword CurrentMissionScope;  // what level we're currently executing at
char CurrentMissionScopeName[KAS_MISSION_NAME_MAX_LENGTH+1];

char CurrentMissionName[KAS_MISSION_NAME_MAX_LENGTH+1];
KASWatchFunction CurrentMissionWatchFunction = NULL;
AITeam *CurrentTeamP = NULL;
AITeam *kasUnpausedTeam = NULL;
sdword CurrentMissionSkillLevel = 0;

#define kasThisTeamPtr CurrentTeamP

//
//  variable space for user selections (the labelled "SHIPS" type in KAS)
//
KasSelection **Selections = NULL;
static sdword SelectionsUsed = 0;
static sdword SelectionsAllocated = 0;

//
//  table of labelled entities in mission layout file
//  (filled in at mission load time)
//

//  team labels are stored in AITeam struct
//LabelledAITeams *LabelledAITeam;
//static sdword LabelledAITeamsUsed = 0;
//static sdword LabelledAITeamsAllocated = 0;

LabelledPath **LabelledPaths = NULL;
static sdword LabelledPathsUsed = 0;
static sdword LabelledPathsAllocated = 0;

LabelledVector **LabelledVectors = NULL;
sdword LabelledVectorsUsed = 0;
sdword LabelledVectorsAllocated = 0;

LabelledVolume **LabelledVolumes = NULL;
static sdword LabelledVolumesUsed = 0;
static sdword LabelledVolumesAllocated = 0;

real32 subMessageReturnedFalseTime = 0;

// need some closing stuff for kas - global variables need to be
// made NULL etc.
void kasClose()
{
    kasUnpausedTeam = NULL;
    subMessageReturnedFalseTime = 0.0f;
}

void kasInit()
{
    subMessageReturnedFalseTime = 0.0f;
}

void kasTeamDied(struct AITeam *team)
{
    if (CurrentTeamP == team)
    {
        CurrentTeamP = NULL;
    }
}

//
//  JUMP keyword of KAS language
//
//  sets state of current team to new state (for subsequently WATCHing) and executes its
//  init function right now
//
void kasJump(char *stateName, KASInitFunction initFunction, KASWatchFunction watchFunction)
{
#ifndef HW_Release
{
    char teamName[KAS_TEAM_NAME_MAX_LENGTH+1];
    aiplayerLog((aiCurrentAIPlayer->player->playerIndex,"KAS: TEAM(\"%s\") JUMP %s", kasAITeamName(CurrentTeamP, teamName), stateName));
}
#endif
    memStrncpy(CurrentTeamP->kasStateName, stateName, KAS_STATE_NAME_MAX_LENGTH);

    CurrentTeamP->kasStateWatchFunction = watchFunction;

    initFunction();
}

//
//  FSMCreate keyword of KAS language
//
//  "creates" an FSM and hands control of a team to it
//
void kasFSMCreate(char *fsmName, KASInitFunction initFunction, KASWatchFunction watchFunction, AITeam *teamP)
{
    AITeam *saveP;
    sdword saveScope;
    char SaveMissionScopeName[KAS_MISSION_NAME_MAX_LENGTH+1];

    // save current team & scope
    saveP = CurrentTeamP;
    saveScope = CurrentMissionScope;
    strcpy(SaveMissionScopeName, CurrentMissionScopeName);

    CurrentTeamP = teamP;

#ifndef HW_Release
{
    char teamName[KAS_TEAM_NAME_MAX_LENGTH+1];
    aiplayerLog((aiCurrentAIPlayer->player->playerIndex,"\nKAS: FSMCreate(\"%s\", TEAM(\"%s\")", fsmName, kasAITeamName(teamP, teamName)));
}
#endif

    teamP->teamType = ScriptTeam;
    memStrncpy(teamP->kasFSMName, fsmName, KAS_FSM_NAME_MAX_LENGTH);
    teamP->kasFSMWatchFunction = watchFunction;
    teamP->kasStateWatchFunction = NULL;  // just in case it doesn't get set later
    teamP->kasTactics   = Neutral;        // default to neutral tactics
    teamP->kasFormation = NO_FORMATION;   // default to no formation

    strcpy(CurrentMissionScopeName, kasThisTeamPtr->kasLabel);

    initFunction();

    // restore current team & scope
    CurrentTeamP = saveP;
    CurrentMissionScope = saveScope;
    strcpy(CurrentMissionScopeName, SaveMissionScopeName);
}

//
//  all the processing of KAS controlled stuff happens here
//
//  must be called once per AI cycle
//
void kasExecute(void)
{
    sdword i;

    // no current mission?
    if (!CurrentMissionName[0] || !CurrentMissionWatchFunction)
        return;

    // watch for all objectives (if >= 1 defined) completed
#if 0
    if (timTimerExpiredDestroy("ObjectivesComplete"))
        kasfMissionCompleted();
    else if (!timTimerRemaining("ObjectivesComplete") && objectivesUsed && objectiveGetAll())
        timTimerCreateSetStart("ObjectivesComplete", OBJECTIVES_COMPLETE_DELAY);
#endif

    if (kasUnpausedTeam == NULL)
    {                                                       //if KAS is not paused
        CurrentMissionWatchFunction();          // watch at the mission level

        for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
        {
            CurrentTeamP = aiCurrentAIPlayer->teams[i];
            if (CurrentTeamP->teamType == ScriptTeam)
            {
                if (CurrentTeamP->kasFSMWatchFunction)   // watch at the FSM level
                    CurrentTeamP->kasFSMWatchFunction();
                if (CurrentTeamP->kasStateWatchFunction) // watch at the state level
                    CurrentTeamP->kasStateWatchFunction();
            }
        }
    }
    else
    {                                                       //else all FSM's but one are paused
        CurrentTeamP = kasUnpausedTeam;                     //only execute this 1 unpaused team
        if (CurrentTeamP->teamType == ScriptTeam)
        {
            if (CurrentTeamP->kasFSMWatchFunction)   // watch at the FSM level
                CurrentTeamP->kasFSMWatchFunction();
            if (CurrentTeamP->kasStateWatchFunction) // watch at the state level
                CurrentTeamP->kasStateWatchFunction();
        }
    }
}

//
//  starts a mission
//
void kasMissionStart(char *name, KASInitFunction initFunction, KASWatchFunction watchFunction)
{
    aiplayerLog((0,"\nKAS: Starting %s", name));
    memStrncpy(CurrentMissionName, name, KAS_MISSION_NAME_MAX_LENGTH);
    CurrentMissionWatchFunction = watchFunction;

    //hyperspace init
    hsStaticInit(LabelledVectorsUsed);

    initFunction();         // must go last - save game is called at end of this function, we want everything setup
}

//
//  return reference to labelled team
//
AITeam *kasAITeamPtr(char *label)
{
    sdword i = 0;
    AITeam *aiteamp = NULL;

    while (i < aiCurrentAIPlayer->teamsUsed)
    {
        aiteamp = aiCurrentAIPlayer->teams[i];
        if (aiteamp->teamType == ScriptTeam && !strncasecmp(aiteamp->kasLabel, label, KAS_TEAM_NAME_MAX_LENGTH))
            return aiteamp;
        ++i;
    }

    // special reference to sender of last message received
    if (!strcasecmp(label, "MsgSender"))
        return CurrentTeamP->msgSender;
#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to team %s", label);
#endif
    return NULL;
}

//
//  return reference to labelled team's ships (contrast with kasAITeamPtr)
//
GrowSelection *kasAITeamShipsPtr(char *label)
{
    sdword i = 0;
    AITeam *aiteamp;

    while (i < aiCurrentAIPlayer->teamsUsed)
    {
        aiteamp = aiCurrentAIPlayer->teams[i];
        if (aiteamp->teamType == ScriptTeam && !strncasecmp(aiteamp->kasLabel, label, KAS_TEAM_NAME_MAX_LENGTH))
            return &aiteamp->shipList;
        ++i;
    }
    // special reference to sender of last message received
    if (!strcasecmp(label, "MsgSender"))
        if (CurrentTeamP->msgSender)
            return &CurrentTeamP->msgSender->shipList;
#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to team %s", label);
#endif
    return NULL;
}

//
//  return reference to labelled grow selection's vector
//
hvector *kasShipsVectorPtr(char *label)
{
    static hvector errorPos= {0, 0, 0, 1};  // safe fallback
    static hvector location;
    vector loc;
    GrowSelection *grow;
    real32 dummy;

    grow = kasGetGrowSelectionPtrIfExists(label);

    if (grow && grow->selection->numShips)
    {
        loc = selCentrePointComputeGeneral((MaxSelection *)grow->selection, &dummy);
        vecMakeHVecFromVec(location, loc);
        return &location;
    }
    else
    {
        return &errorPos;
    }

#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to shiplist %s", label);
#endif
    return NULL;
}

//
// return reference to labelled team's location
//
hvector *kasTeamsVectorPtr(char *label)
{
    static hvector errorPos = {0,0,0,1};
    static hvector location;
    vector loc;
    GrowSelection *grow;
    real32 dummy;

    grow = kasAITeamShipsPtr(label);

    if (grow && grow->selection->numShips)
    {
        loc = selCentrePointComputeGeneral((MaxSelection *)grow->selection, &dummy);
        vecMakeHVecFromVec(location, loc);
        return &location;
    }
    else
    {
        return &errorPos;
    }

#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to shiplist %s", label);
#endif
    return NULL;
}

//
// return reference to labelled volume's center
//
hvector *kasVolumeVectorPtr(char *label)
{
    static hvector returnhvec;
    vector volcenter;
    Volume *vol = kasVolumePtr(label);

    if (vol)
    {
        volcenter = volFindCenter(vol);
        vecMakeHVecFromVec(returnhvec, volcenter);
        return &returnhvec;
    }
    else
    {
        return NULL;
    }
}

//
// return reference to the team's centerpoint
//
hvector *kasThisTeamsVectorPtr(void)
{
    static hvector errorPos = {0,0,0,1};
    static hvector location;
    vector loc;
    GrowSelection *grow;
    real32 dummy;

    grow = &CurrentTeamP->shipList;

    if (grow && grow->selection->numShips)
    {
        loc = selCentrePointComputeGeneral((MaxSelection *)grow->selection, &dummy);
        vecMakeHVecFromVec(location, loc);
        return &location;
    }
    else
    {
        return &errorPos;
    }

#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference in kasThisTeamsVectorPtr");
#endif
    return NULL;

}

//
//  return reference to labelled path
//
Path *kasPathPtrNoErrorChecking(char *label)
{
    sdword i = 0;
    while (i < LabelledPathsUsed)
    {
        if (!strncasecmp(LabelledPaths[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return LabelledPaths[i]->path;
        ++i;
    }
    return NULL;
}

//
//  return reference to labelled path
//
Path *kasPathPtr(char *label)
{
    sdword i = 0;
    while (i < LabelledPathsUsed)
    {
        if (!strncasecmp(LabelledPaths[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return LabelledPaths[i]->path;
        ++i;
    }
    dbgMessagef("\nKAS: unresolved reference to path %s", label);
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to path %s", label);     // deliberately crash so designers can find bug easier
    return NULL;
}

//
//  return reference to labelled volume
//
Volume *kasVolumePtr(char *label)
{
    sdword i = 0;
    while (i < LabelledVolumesUsed)
    {
        if (!strncasecmp(LabelledVolumes[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return LabelledVolumes[i]->volume;
        ++i;
    }
#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to volume %s", label);
#endif
    return NULL;
}

//
//  return reference to labelled point
//
hvector *kasVectorPtrIfExists(char *label)
{
    sdword i = 0;
    while (i < LabelledVectorsUsed)
    {
        if (!strncasecmp(LabelledVectors[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return LabelledVectors[i]->hvector;
        ++i;
    }
    return NULL;
}

hvector *kasVectorPtr(char *label)
{
    hvector *vec = kasVectorPtrIfExists(label);
#ifndef HW_Release
    if (label == NULL)
    {
        dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to point %s", label);
    }
#endif
    return vec;
}

GrowSelection *kasGetGrowSelectionPtrIfExists(char *label)
{
    sdword i = 0;

    while (i < SelectionsUsed)
    {
        if (!strncasecmp(Selections[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return &(Selections[i]->shipList);
        ++i;
    }

    return NULL;
}

//
//  on first reference to a new GrowSelection, create one and return its pointer
//  otherwise, return the pointer
//
GrowSelection *kasGrowSelectionPtr(char *label)
{
    sdword i = 0;

    while (i < SelectionsUsed)
    {
        if (!strncasecmp(Selections[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return &(Selections[i]->shipList);
        ++i;
    }

    if (!Selections)
    {
        Selections = memAlloc(sizeof(KasSelection *) * KASSELECTION_ALLOC_INCREMENT, "kasSelections", 0);
        SelectionsAllocated = KASSELECTION_ALLOC_INCREMENT;
        SelectionsUsed = 0;
    }
    // keep track of all allocations
    if (SelectionsUsed >= SelectionsAllocated)
    {
        // allocate more if necessary
        Selections = memRealloc(Selections, sizeof(KasSelection *) * (SelectionsAllocated + KASSELECTION_ALLOC_INCREMENT), "kasSelections", 0);
        SelectionsAllocated += KASSELECTION_ALLOC_INCREMENT;
    }
    Selections[SelectionsUsed] = memAlloc(sizeof(KasSelection), "kasGrowSelection", 0);
    memStrncpy(Selections[SelectionsUsed]->label, label, KAS_MAX_LABEL_LENGTH);
    growSelectInit(&Selections[SelectionsUsed]->shipList);

    return &(Selections[SelectionsUsed++]->shipList);
}

void kasGrowSelectionClear(GrowSelection *ships)
{
    if (ships && ships->selection)
    {
        ships->selection->numShips = 0;
    }
}

//
//  add new labelled path (with no points, yet), return pointer to it
//
Path *kasLabelledPathAdd(char *label, sdword numPoints, sdword closed)
{
    if (!LabelledPaths)
    {
        LabelledPaths = memAlloc(sizeof(LabelledPath *) * KAS_LABELLED_ENTITY_ALLOC_INCREMENT, "kasLabelledPaths", 0);
        LabelledPathsAllocated = KAS_LABELLED_ENTITY_ALLOC_INCREMENT;
        LabelledPathsUsed = 0;
    }
    // keep track of all allocations
    if (LabelledPathsUsed >= LabelledPathsAllocated)
    {
        // allocate more if necessary
        LabelledPaths = memRealloc(LabelledPaths, sizeof(LabelledPath *) * (LabelledPathsAllocated + KAS_LABELLED_ENTITY_ALLOC_INCREMENT), "kasLabelledPaths", 0);
        LabelledPathsAllocated += KAS_LABELLED_ENTITY_ALLOC_INCREMENT;
    }
    LabelledPaths[LabelledPathsUsed] = memAlloc(sizeof(LabelledPath), "kasLabelledPath", MBF_NonVolatile);
    LabelledPaths[LabelledPathsUsed]->path = aiuCreatePathStruct(numPoints, closed);
    memStrncpy(LabelledPaths[LabelledPathsUsed]->label, label, KAS_MAX_LABEL_LENGTH);

    return LabelledPaths[LabelledPathsUsed++]->path;
}

//
//  add new labelled volume (with no contents, yet), return pointer to it
//
Volume *kasLabelledVolumeAdd(char *label)
{
    if (!LabelledVolumes)
    {
        LabelledVolumes = memAlloc(sizeof(LabelledVolume *) * KAS_LABELLED_ENTITY_ALLOC_INCREMENT, "kasLabelledVolumes", 0);
        LabelledVolumesAllocated = KAS_LABELLED_ENTITY_ALLOC_INCREMENT;
        LabelledVolumesUsed = 0;
    }
    // keep track of all allocations
    if (LabelledVolumesUsed >= LabelledVolumesAllocated)
    {
        // allocate more if necessary
        LabelledVolumes = memRealloc(LabelledVolumes, sizeof(LabelledVolume *) * (LabelledVolumesAllocated + KAS_LABELLED_ENTITY_ALLOC_INCREMENT), "kasLabelledVolumes", 0);
        LabelledVolumesAllocated += KAS_LABELLED_ENTITY_ALLOC_INCREMENT;
    }
    LabelledVolumes[LabelledVolumesUsed] = memAlloc(sizeof(LabelledVolume), "kasLabelledVolume", MBF_NonVolatile);
    LabelledVolumes[LabelledVolumesUsed]->volume = memAlloc(sizeof(Volume), "kasVolume", MBF_NonVolatile);
    memStrncpy(LabelledVolumes[LabelledVolumesUsed]->label, label, KAS_MAX_LABEL_LENGTH);

    return LabelledVolumes[LabelledVolumesUsed++]->volume;
}

//
//  add new labelled vector, return pointer to it
//
hvector *kasLabelledVectorAdd(char *label, real32 x, real32 y, real32 z, real32 w)
{
    if (!LabelledVectors)
    {
        LabelledVectors = memAlloc(sizeof(LabelledVector *) * KAS_LABELLED_ENTITY_ALLOC_INCREMENT, "kasLabelledVectors", 0);
        LabelledVectorsAllocated = KAS_LABELLED_ENTITY_ALLOC_INCREMENT;
        LabelledVectorsUsed = 0;
    }
    // keep track of all allocations
    if (LabelledVectorsUsed >= LabelledVectorsAllocated)
    {
        // allocate more if necessary
        LabelledVectors = memRealloc(LabelledVectors, sizeof(LabelledVector *) * (LabelledVectorsAllocated + KAS_LABELLED_ENTITY_ALLOC_INCREMENT), "kasLabelledVectors", 0);
        LabelledVectorsAllocated += KAS_LABELLED_ENTITY_ALLOC_INCREMENT;
    }
    LabelledVectors[LabelledVectorsUsed] = memAlloc(sizeof(LabelledVector), "kasLabelledVector", MBF_NonVolatile);
    LabelledVectors[LabelledVectorsUsed]->hvector = memAlloc(sizeof(hvector), "kasVector", MBF_NonVolatile);
    memStrncpy(LabelledVectors[LabelledVectorsUsed]->label, label, KAS_MAX_LABEL_LENGTH);
    LabelledVectors[LabelledVectorsUsed]->hvector->x = x;
    LabelledVectors[LabelledVectorsUsed]->hvector->y = y;
    LabelledVectors[LabelledVectorsUsed]->hvector->z = z;
    LabelledVectors[LabelledVectorsUsed]->hvector->w = w;

    return LabelledVectors[LabelledVectorsUsed++]->hvector;
}

//
//  remove all labelled entities
//
void kasLabelledEntitiesDestroy(void)
{
    sdword i;

    CurrentTeamP = NULL;

    // selections
    for (i = 0; i < SelectionsUsed; ++i)
    {
        growSelectClose(&Selections[i]->shipList);
        memFree(Selections[i]);
    }
    if (Selections)
        memFree(Selections);
    SelectionsUsed = 0;
    SelectionsAllocated = 0;
    Selections = NULL;

    // vectors
    for (i = 0; i < LabelledVectorsUsed; ++i)
    {
        memFree(LabelledVectors[i]->hvector);
        memFree(LabelledVectors[i]);
    }
    if (LabelledVectors)
        memFree(LabelledVectors);
    LabelledVectorsUsed = 0;
    LabelledVectorsAllocated = 0;
    LabelledVectors = NULL;

    // volumes
    for (i = 0; i < LabelledVolumesUsed; ++i)
    {
        memFree(LabelledVolumes[i]->volume);
        memFree(LabelledVolumes[i]);
    }
    if (LabelledVolumes)
        memFree(LabelledVolumes);
    LabelledVolumesUsed = 0;
    LabelledVolumesAllocated = 0;
    LabelledVolumes = NULL;

    // paths
    for (i = 0; i < LabelledPathsUsed; ++i)
    {
        memFree(LabelledPaths[i]->path);
        memFree(LabelledPaths[i]);
    }
    if (LabelledPaths)
        memFree(LabelledPaths);
    LabelledPathsUsed = 0;
    LabelledPathsAllocated = 0;
    LabelledPaths = NULL;

    // vars
    aivarDestroyAll();

    // timers
    timTimerDestroyAll();
}

//
//  Find team label from pointer.
//  Returns teamName (& fills it in).
//  If not found, returns "NULL" or something like that.
//
char *kasAITeamName(AITeam *team, char *teamName)
{
    if (team)
        strcpy(teamName, team->kasLabel);
    else
        strcpy(teamName, "NULL");
    return teamName;
}

/*-----------------------------------------------------------------------------
    Name        : kasAddShipToTeam
    Description : utility function to add a ship to a team (creates team if doesn't exist)
    Inputs      : ship, str
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasAddShipToTeam(Ship *ship,char *label)
{
    sdword i;
    AITeam *teamp;

    // if labelled team doesn't currently exist, create it
    // otherwise, add ships to existing labelled team
    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        teamp = aiCurrentAIPlayer->teams[i];
        if (teamp->teamType == ScriptTeam && !strncasecmp(teamp->kasLabel, label, KAS_TEAM_NAME_MAX_LENGTH))
            break;
    }
    if (i >= aiCurrentAIPlayer->teamsUsed)
    {
        teamp = aitCreate(ScriptTeam);
        memStrncpy(teamp->kasLabel, label, KAS_TEAM_NAME_MAX_LENGTH);
        teamp->kasOrigShipsCount = 0;
    }

    teamp->kasOrigShipsCount++;
    teamp->kasOrigShipsType = ship->shiptype;

    aitAddShip(teamp, ship);
    teamp->newships--;
}

//
//  remove ship reference from any ship lists
//
void kasShipDied(Ship *ship)
{
    sdword i = 0;

    while (i < SelectionsUsed)
    {
        clRemoveShipFromSelection(Selections[i]->shipList.selection, ship);
        ++i;
    }
}

//
//  wipe the labelled object tables clean
//  (do this before reading in a new mission layout)
//
void kasLabelsInit(void)
{
    LabelledPathsUsed = 0;
    LabelledVectorsUsed = 0;
    LabelledVolumesUsed = 0;

    //hyperspace reset
    hsStaticReset();
}

//////////////////////////////////////////////////////////////////
//Encase all this debug stuff in this ifdef

//
//  this macro takes care of column wrapping
//  for the debug display
//
#define COLUMN_CHECK \
if (y > MAIN_WindowHeight - 2*rowHeight)    \
{                                         \
    y = rowHeight;                        \
    x += MAIN_WindowWidth/2 - rowHeight;  \
}


#if SP_DEBUGKEYS

/*-----------------------------------------------------------------------------
    Name        : kasTakeADump
    Description : Dumps all the singleplayer information into a file
    Inputs      : None
    Outputs     : A brand new file!
    Return      : void
----------------------------------------------------------------------------*/
void kasTakeADump(void)
{
    FILE *fp;
    char fileName[256] = "StatusDump.txt", *fullName;
    udword i, remaining;
    Timer *tp;

    fullName = filePathPrepend(fileName, FF_UserSettingsPath);

    if (!fileMakeDestinationDirectory(fullName))
        return;

    fp = fopen(fullName, "wt");
    if (!fp)
        return;

    fprintf(fp, "%s\n\n", CurrentMissionName);

    //printout states
    fprintf(fp, "STATES:\n");
    for (i=0;i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        CurrentTeamP = aiCurrentAIPlayer->teams[i];

        if (CurrentTeamP->teamType == ScriptTeam)
        {
            fprintf(fp, "%s {%d} : %s %s\n",
                    CurrentTeamP->kasLabel,
                    CurrentTeamP->shipList.selection->numShips,
                    CurrentTeamP->kasFSMName,
                    CurrentTeamP->kasStateName);
        }
    }

    //printout variables
    fprintf(fp, "\nVARIABLES:\n");
    for (i = 0; i < varsUsed; ++i)
    {
        if (vars[i]->label[0] != '_' && vars[i]->label[1] != 'V')  // skip non KAS vars
        {
            fprintf(fp, "%s = %d\n", vars[i]->label, vars[i]->value);
        }
    }

    //printout timers
    fprintf(fp, "\nTIMERS:\n");
    for (i=0;i<timersUsed;i++)
    {
        tp = &timers[i];
        remaining = (sdword)(tp->duration - (universe.totaltimeelapsed - tp->startTime));
        if (remaining <= 0)
        {
            fprintf(fp, "%s EXPIRED\n", tp->name);
        }
        else
        {
            fprintf(fp, "%s %s : %ds\n", tp->name, tp->enabled ? "ON" : "OFF", remaining);
        }
    }
    fclose(fp);
    clCommandMessage("Taking a Dump");
}

/*-----------------------------------------------------------------------------
    Name        : kasCountDisplayLines
    Description : Counts the number of lines the debug draw routine will be drawing
    Inputs      : none
    Outputs     : none
    Return      : Returns the number of lines to draw
----------------------------------------------------------------------------*/
udword kasCountDisplayLines(udword *states, udword *variables)
{
    udword display = 0, i=0;

    *states    = 0;
    *variables = 0;

    //count the number of states to draw
    if (mrKASDebugDrawStates)
    {
        for (i=0; i<aiCurrentAIPlayer->teamsUsed;i++)
        {
            if (aiCurrentAIPlayer->teams[i]->teamType == ScriptTeam)
            {
                display++;
                *states++;
            }
        }
    }
    //count the number of Variables to draw
    if (mrKASDebugDrawVars)
    {
        for (i=0; i<varsUsed;i++)
        {
            if (vars[i]->label[0] != '_' && vars[i]->label[1] != 'V')
            {
                display++;
                *variables++;
            }
        }
    }
    //count the number of Timers to draw
    if (mrKASDebugDrawTimers)
    {
        display += timersUsed;
    }
    return display;
}

#define KAS_NUM_SCREEN_LINES 80
void kasDebugDraw(void)
{
    sdword rowHeight, y, x, i;
//    fonthandle fhSave;
    AITeam *CurrentTeamP;
    sdword remaining;
    Timer *tp;
    char buf[256];
    vector vec0, vec1;
    Volume *volumep;
    udword numlines = 0, numstates = 0, numvars = 0, skiplines = 0;

    if (!gameIsRunning)
        return;

    if (!CurrentMissionName[0] || !CurrentMissionWatchFunction)
        return;

    // set up textual stuff
//    fhSave = fontCurrentGet();                  //save the current font
//    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    rowHeight = fontHeight("M")-2; // used to space the text
    y = rowHeight;
    x = (sdword)rowHeight;

#if SP_DEBUGLEVEL2
    //count the number of lines to display
    numlines  = kasCountDisplayLines(&numstates, &numvars);
    skiplines = mrKASSkipFactor*KAS_NUM_SCREEN_LINES;

    // if we're displaying less than the screen can hold,
    // or if the user has paged over
    if ((numlines < KAS_NUM_SCREEN_LINES) ||
        (numlines < skiplines))
    {
        skiplines       = 0;
        mrKASSkipFactor = 0;
    }

    // states
    if (mrKASDebugDrawStates)
    {
        // mission name
        fontPrint(x-rowHeight/2, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, CurrentMissionName);
        y += rowHeight + 1;
        COLUMN_CHECK

        // each team's state
        for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
        {
            CurrentTeamP = aiCurrentAIPlayer->teams[i];
            if (CurrentTeamP->teamType == ScriptTeam)
            {
                if (skiplines)
                {
                    skiplines--;
                }
                else
                {
                    sprintf(buf, "%s {%d} : %s %s",
                            CurrentTeamP->kasLabel,
                            CurrentTeamP->shipList.selection->numShips,
                            CurrentTeamP->kasFSMName,
                            CurrentTeamP->kasStateName);
                    fontPrint(x, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, buf);
                    y += rowHeight + 1;
                    COLUMN_CHECK
                }
            }
        }
        y += rowHeight/3;
        COLUMN_CHECK
    }

    // vars
    if (mrKASDebugDrawVars)
    {
        sprintf(buf,"Enemy RU = %d",universe.players[1].resourceUnits);
        fontPrint(x, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, buf);
        y += rowHeight + 1;
        COLUMN_CHECK

        for (i = 0; i < varsUsed; ++i)
        {
            if (vars[i]->label[0] != '_' && vars[i]->label[1] != 'V')  // skip non KAS vars
            {
                if (skiplines)
                {
                    skiplines--;
                }
                else
                {
                    sprintf(buf, "%s = %d", vars[i]->label, vars[i]->value);
                    fontPrint(x, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, buf);
                    y += rowHeight + 1;
                    COLUMN_CHECK
                }
            }
        }
        y += rowHeight/3;
        COLUMN_CHECK
    }

    // timers
    if (mrKASDebugDrawTimers)
    {
        for (i = 0; i < timersUsed; ++i)
        {
            if (skiplines)
            {
                skiplines--;
            }
            else
            {
                tp = &timers[i];
                remaining = (sdword)(tp->duration - (universe.totaltimeelapsed - tp->startTime));
                if (remaining <= 0)
                    sprintf(buf, "%s EXPIRED", tp->name);
                else
                    sprintf(buf, "%s %s : %ds", tp->name, tp->enabled ? "ON" : "OFF", remaining);
                fontPrint(x, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, buf);
                y += rowHeight + 1;
                COLUMN_CHECK
            }
        }
        y += rowHeight/3;
        COLUMN_CHECK
    }

    // volumes
    if (mrKASDebugDrawVolumes)
    {
        primModeClear2();
        rndLightingEnable(FALSE);
        rndTextureEnable(FALSE);
        glDisable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        rgluPerspective(mrCamera->fieldofview, rndAspectRatio, mrCamera->clipPlaneNear, mrCamera->clipPlaneFar * 15.0f);
        glMatrixMode(GL_MODELVIEW);

        #if 0
        {
            Ship *m0p = aiuFindEnemyMothership(&universe.players[0]),
                 *m1p = aiuFindEnemyMothership(&universe.players[1]);
            primCircleOutline3(&m0p->posinfo.position,
                               10000,
                               32, 4, KAS_DEBUG_TEXTCOLOR, X_AXIS);
            primCircleOutline3(&m0p->posinfo.position,
                               10000,
                               32, 4, KAS_DEBUG_TEXTCOLOR, Y_AXIS);
            primCircleOutline3(&m0p->posinfo.position,
                               10000,
                               32, 8, KAS_DEBUG_TEXTCOLOR, Z_AXIS);
            primCircleOutline3(&m1p->posinfo.position,
                               10000,
                               32, 0, KAS_DEBUG_TEXTCOLOR, X_AXIS);
            primCircleOutline3(&m1p->posinfo.position,
                               10000,
                               32, 0, KAS_DEBUG_TEXTCOLOR, Y_AXIS);
            primCircleOutline3(&m1p->posinfo.position,
                               10000,
                               32, 8, KAS_DEBUG_TEXTCOLOR, Z_AXIS);

        }
        #endif

        for (i = 0; i < LabelledVolumesUsed; ++i)
        {
            volumep = LabelledVolumes[i]->volume;
            switch (volumep->type)
            {
                case VOLUME_AA_BOX:
                    vec0.x = volumep->attribs.aaBox.x0;  vec1.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y0;  vec1.y = volumep->attribs.aaBox.y0;
                    vec0.z = volumep->attribs.aaBox.z0;  vec1.z = volumep->attribs.aaBox.z0;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y1;
                    vec0.z = volumep->attribs.aaBox.z0;
                    primLine3(&vec1, &vec0, KAS_DEBUG_TEXTCOLOR);
                    vec1.x = volumep->attribs.aaBox.x0;
                    vec1.y = volumep->attribs.aaBox.y1;
                    vec1.z = volumep->attribs.aaBox.z0;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x0;
                    vec0.y = volumep->attribs.aaBox.y0;
                    vec0.z = volumep->attribs.aaBox.z0;
                    primLine3(&vec1, &vec0, KAS_DEBUG_TEXTCOLOR);
                    vec1.x = volumep->attribs.aaBox.x0;
                    vec1.y = volumep->attribs.aaBox.y0;
                    vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y0;
                    vec0.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec1, &vec0, KAS_DEBUG_TEXTCOLOR);
                    vec1.x = volumep->attribs.aaBox.x1;
                    vec1.y = volumep->attribs.aaBox.y1;
                    vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x0;
                    vec0.y = volumep->attribs.aaBox.y1;
                    vec0.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec1, &vec0, KAS_DEBUG_TEXTCOLOR);
                    vec1.x = volumep->attribs.aaBox.x0;
                    vec1.y = volumep->attribs.aaBox.y0;
                    vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x0;  vec1.x = volumep->attribs.aaBox.x0;
                    vec0.y = volumep->attribs.aaBox.y1;  vec1.y = volumep->attribs.aaBox.y1;
                    vec0.z = volumep->attribs.aaBox.z0;  vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x1;  vec1.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y1;  vec1.y = volumep->attribs.aaBox.y1;
                    vec0.z = volumep->attribs.aaBox.z0;  vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x1;  vec1.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y0;  vec1.y = volumep->attribs.aaBox.y0;
                    vec0.z = volumep->attribs.aaBox.z0;  vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    break;

                case VOLUME_SPHERE:
                    primCircleOutline3(&volumep->attribs.sphere.center,
                                       volumep->attribs.sphere.radius,
                                       32, 4, KAS_DEBUG_TEXTCOLOR, X_AXIS);
                    primCircleOutline3(&volumep->attribs.sphere.center,
                                       volumep->attribs.sphere.radius,
                                       32, 4, KAS_DEBUG_TEXTCOLOR, Y_AXIS);
                    primCircleOutline3(&volumep->attribs.sphere.center,
                                       volumep->attribs.sphere.radius,
                                       32, 4, KAS_DEBUG_TEXTCOLOR, Z_AXIS);
                    break;
            }
        }

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf((GLfloat*)&rndProjectionMatrix);
        glMatrixMode(GL_MODELVIEW);
        primModeSet2();
    }
#endif //SP_DEBUGLEVEL2

//    fontMakeCurrent(fhSave);
}
#endif //SP_DEBUGKEYS

/*=============================================================================
    Save Game Stuff
=============================================================================*/

void SaveKasSelection(KasSelection *kasselection)
{
    SaveStructureOfSize(kasselection,sizeof(KasSelection));
    SaveGrowSelection(&kasselection->shipList);
}

KasSelection *LoadKasSelection()
{
    KasSelection *kasselection = LoadStructureOfSize(sizeof(KasSelection));
    LoadGrowSelectionAndFix(&kasselection->shipList);
    return kasselection;
}

void SaveLabelledPath(LabelledPath *labelledpath)
{
    SaveStructureOfSize(labelledpath,sizeof(LabelledPath));
    SavePath(labelledpath->path);
}

LabelledPath *LoadLabelledPath()
{
    LabelledPath *labelledpath = LoadStructureOfSize(sizeof(LabelledPath));
    labelledpath->path = LoadPath();
    return labelledpath;
}

void SaveLabelledVector(LabelledVector *labelledvector)
{
    SaveStructureOfSize(labelledvector,sizeof(LabelledVector));
    SaveStructureOfSize(labelledvector->hvector,sizeof(hvector));
}

LabelledVector *LoadLabelledVector()
{
    LabelledVector *labelledvector = LoadStructureOfSize(sizeof(LabelledVector));
    labelledvector->hvector = LoadStructureOfSize(sizeof(hvector));
    return labelledvector;
}

void SaveLabelledVolume(LabelledVolume *labelledvolume)
{
    SaveStructureOfSize(labelledvolume,sizeof(LabelledVolume));
    SaveStructureOfSize(labelledvolume->volume,sizeof(Volume));
}

LabelledVolume *LoadLabelledVolume()
{
    LabelledVolume *labelledvolume = LoadStructureOfSize(sizeof(LabelledVolume));
    labelledvolume->volume = LoadStructureOfSize(sizeof(Volume));
    return labelledvolume;
}

void kasSave(void)
{
    sdword i;

    // hyperspace save
    SaveHyperspaceGates();

    // Save Global Variables

    SaveInfoNumber(CurrentMissionScope);

    SaveStructureOfSize(CurrentMissionScopeName,sizeof(CurrentMissionScopeName));
    SaveStructureOfSize(CurrentMissionName,sizeof(CurrentMissionName));

    SaveInfoNumber(WatchFunctionToIndex(CurrentMissionWatchFunction));

    //this is a fix for a weird bug where after starting the game over and over
    //Current
    SaveInfoNumber(AITeamToTeamIndex(CurrentTeamP));

    SaveInfoNumber(CurrentMissionSkillLevel);

    // Save Selections

    SaveInfoNumber(SelectionsUsed);
    SaveInfoNumber(SelectionsAllocated);

    for (i=0;i<SelectionsUsed;i++)
    {
        SaveKasSelection(Selections[i]);
    }

    // Save LabelledPaths

    SaveInfoNumber(LabelledPathsUsed);
    SaveInfoNumber(LabelledPathsAllocated);

    for (i=0;i<LabelledPathsUsed;i++)
    {
        SaveLabelledPath(LabelledPaths[i]);
    }

    // Save LabelledVectors

    SaveInfoNumber(LabelledVectorsUsed);
    SaveInfoNumber(LabelledVectorsAllocated);

    for (i=0;i<LabelledVectorsUsed;i++)
    {
        SaveLabelledVector(LabelledVectors[i]);
    }

    // Save LabelledVolumes

    SaveInfoNumber(LabelledVolumesUsed);
    SaveInfoNumber(LabelledVolumesAllocated);

    for (i=0;i<LabelledVolumesUsed;i++)
    {
        SaveLabelledVolume(LabelledVolumes[i]);
    }

    // Save KAS Timers

    timTimerSave();
}

void kasLoad(void)
{
    sdword i;

    // hyperspace load
    LoadHyperspaceGates();

    // Load Global Variables

    CurrentMissionScope = LoadInfoNumber();

    LoadStructureOfSizeToAddress(CurrentMissionScopeName,sizeof(CurrentMissionScopeName));
    LoadStructureOfSizeToAddress(CurrentMissionName,sizeof(CurrentMissionName));

    CurrentMissionWatchFunction = IndexToWatchFunction(LoadInfoNumber());

    dbgAssert(universe.players[1].aiPlayer);
    CurrentTeamP = AITeamIndexToTeam(universe.players[1].aiPlayer,LoadInfoNumber());

    CurrentMissionSkillLevel = LoadInfoNumber();

    // Load Selections

    SelectionsUsed = LoadInfoNumber();
    SelectionsAllocated = LoadInfoNumber();

    if (SelectionsAllocated > 0)
    {
        Selections = memAlloc(sizeof(KasSelection *)*SelectionsAllocated, "kasSelections", 0);
    }
    else
    {
        Selections = NULL;
    }

    for (i=0;i<SelectionsUsed;i++)
    {
        Selections[i] = LoadKasSelection();
    }

    // Load LabelledPaths

    LabelledPathsUsed = LoadInfoNumber();
    LabelledPathsAllocated = LoadInfoNumber();

    if (LabelledPathsAllocated > 0)
    {
        LabelledPaths = memAlloc(sizeof(LabelledPath *)*LabelledPathsAllocated, "kasLabelledPaths", 0);
    }
    else
    {
        LabelledPaths = NULL;
    }

    for (i=0;i<LabelledPathsUsed;i++)
    {
        LabelledPaths[i] = LoadLabelledPath();
    }

    // Load LabelledVectors

    LabelledVectorsUsed = LoadInfoNumber();
    LabelledVectorsAllocated = LoadInfoNumber();

    if (LabelledVectorsAllocated > 0)
    {
        LabelledVectors = memAlloc(sizeof(LabelledVector *)*LabelledVectorsAllocated, "kasLabelledVectors", 0);
    }
    else
    {
        LabelledVectors = NULL;
    }

    for (i=0;i<LabelledVectorsUsed;i++)
    {
        LabelledVectors[i] = LoadLabelledVector();
    }

    // Load LabelledVolumes

    LabelledVolumesUsed = LoadInfoNumber();
    LabelledVolumesAllocated = LoadInfoNumber();

    if (LabelledVolumesAllocated > 0)
    {
        LabelledVolumes = memAlloc(sizeof(LabelledVolume *)*LabelledVolumesAllocated, "kasLabelledVolumes", 0);
    }
    else
    {
        LabelledVolumes = NULL;
    }

    for (i=0;i<LabelledVolumesUsed;i++)
    {
        LabelledVolumes[i] = LoadLabelledVolume();
    }

    // Load KAS Timers

    timTimerLoad();
}

sdword kasConvertFuncPtrToOffset(void *func)
{
    if (func == NULL)
    {
        return -1;
    }
    else
    {
        udword i;
        const void** func_list;
        udword func_list_size;

        func_list =
            IndexToFunctionList(singlePlayerGameInfo.currentMission - 1);
        func_list_size = (func_list
            ? FunctionListSize(singlePlayerGameInfo.currentMission - 1)
            : 0);

        for (i = 0; i < func_list_size; i++)
        {
            if (func_list[i] == (void*)func)
                return (sdword)i;
        }

        return -1;
    }
}

void *kasConvertOffsetToFuncPtr(sdword offset)
{
    if (offset == -1)
    {
        return NULL;
    }
    else
    {
        const void** func_list;
        udword func_list_size;

        func_list =
            IndexToFunctionList(singlePlayerGameInfo.currentMission - 1);
        func_list_size = (func_list
            ? FunctionListSize(singlePlayerGameInfo.currentMission - 1)
            : 0);

        return ((udword)offset < func_list_size
            ? func_list[offset]
            : NULL);
    }
}

