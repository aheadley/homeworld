//
//  SINGLE-PLAYER MISSION OBJECTIVES
//

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "Types.h"
#include "Objectives.h"
#include "ShipSelect.h"
#include "KASFunc.h"
#include "Memory.h"
#include "Strings.h"
#include "font.h"
#include "render.h"
#include "FEFlow.h"
#include "mainrgn.h"
#include "Sensors.h"
#include "Universe.h"
#include "TaskBar.h"
#include "KASFunc.h"
#include "SaveGame.h"
#include "FontReg.h"
#include "SinglePlayer.h"
#include "SoundEvent.h"
#include "AIVar.h"
#include "NIS.h"
#include "Subtitle.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

extern sdword popupTextNumLines;
extern char *getWord(char *dest, char *source); // defined in researchgui.c
extern regionhandle ghMainRegion;

// for objective status overlay
extern fonthandle selGroupFont2;
#define OBJECTIVE_TITLE_TEXTCOLOR      colRGB(255, 255, 255)
#define OBJECTIVE_TEXTCOLOR            colRGB(200, 200, 0)
#define OBJECTIVE_POPUPTEXT_COLOR      colRGB(0, 200, 200)
#define PO_FONTNAMESIZE                64
#define poPopupFont                    selGroupFont2

LinkedList poFleetIntelligence;
Objective **objectives = NULL;
sdword objectivesAllocated = 0, objectivesUsed = 0;
FleetIntelligence *poCurFleetIntelligence = NULL;

regionhandle poBaseRegion = NULL;

char poHeadingFontName[PO_FONTNAMESIZE]="blah blah blah blah blah blah";
char poTextFontName[PO_FONTNAMESIZE]="blah blah blah blah blah blah ";
fonthandle poHeadingFont = 0;
fonthandle poTextFont = 0;




// Forward reference for callback tables
void poClose(char *string, featom *atom);
void poBackgroundDraw(featom *atom, regionhandle region);
void poTextDraw(featom *atom, regionhandle region);

fecallback poCallback[] =
{
    {poClose,           "PO_CloseObjectiveWindow"},
    {NULL, NULL}
};

fedrawcallback poDrawCallback[] =
{
    {poTextDraw,        "PO_TextDraw"},
    {NULL, NULL}
};


void fleetIntelligenceDestroy(FleetIntelligence *fleetIntelligence)
{
    memFree(fleetIntelligence->description);
    listDeleteNode(&fleetIntelligence->node);
}


FleetIntelligence *fleetIntelligenceCreate(char *description, bool8 showOnce)
{
    FleetIntelligence *fleetIntelligence;

    fleetIntelligence = memAlloc(sizeof(FleetIntelligence), "Fleet Intelligence", 0);
    fleetIntelligence->showOnce = showOnce;
    fleetIntelligence->showNow  = TRUE;
    fleetIntelligence->description = memStringDupe(description);
    listAddNode(&poFleetIntelligence, &fleetIntelligence->node, (void*) fleetIntelligence);

    // ASSUMING THAT THIS IS TRUE ONLY WHILE IN THE GAME ALREADY AND NOT LOADING
    if (showOnce)
    {
        poCurFleetIntelligence = fleetIntelligence;
        poPlayerObjectivesBegin(ghMainRegion);
        fleetIntelligence = NULL;
    }

    return fleetIntelligence;
}

Objective *objectiveAndFleetIntelligenceCreate(char *label, char *briefDescription, char* fullDescription, bool8 showOnce, bool primary)
{
    Objective *objective;
    bool hyperspace;

    if (objectiveFind(label))
    {
        return NULL;
    }

    // allocate the node
    objective = memAlloc(sizeof(Objective), "objective", 0);
    objective->status = FALSE;
    objective->primary = primary;
    memStrncpy(objective->label, label, MAX_OBJECTIVE_LABEL_LENGTH);
    objective->description = memStringDupe(briefDescription);

    if (!strcasecmp(label, "hyperspace"))
    {
        hyperspace = TRUE;
        singlePlayerGameInfo.playerCanHyperspace = TRUE;
        smUpdateHyperspaceStatus(TRUE);
        objective->fleetIntelligence = NULL;
    }
    else
    {
        hyperspace = FALSE;
        if (fullDescription != NULL)
        {
            objective->fleetIntelligence = fleetIntelligenceCreate(fullDescription, showOnce);
        }
        else
        {
            objective->fleetIntelligence = NULL;
        }
    }

    // keep track of all allocations
    if (!objectives)
    {
        objectives = memAlloc(sizeof(Objective*)*OBJECTIVES_ALLOC_INITIAL, "objectives", 0);
        objectivesAllocated = OBJECTIVES_ALLOC_INITIAL;
        objectivesUsed = 0;
    }
    if (objectivesUsed >= objectivesAllocated)
    {
        // allocate more if necessary
        objectives = memRealloc(objectives, sizeof(Objective*)*(objectivesAllocated + OBJECTIVES_ALLOC_INCREMENT), "objectives", 0);
        objectivesAllocated += OBJECTIVES_ALLOC_INCREMENT;
    }

    objectives[objectivesUsed++] = objective;

    if (hyperspace)
    {
        tbObjectivesHyperspace((ubyte*)objectives[objectivesUsed-1]);
    }
    else
    {
        tbObjectivesListAddItem((ubyte*)objectives[objectivesUsed-1]);
    }

    return objective;
}


void fleetIntelligenceDestroyAll(void)
{
    FleetIntelligence *fleetIntelligence;
    udword numEntries;
    udword count;
    Node *node;

    numEntries = poFleetIntelligence.num;
    node = poFleetIntelligence.head;

    for (count = 0; count < numEntries; count++)
    {
        fleetIntelligence = (FleetIntelligence*) listGetStructOfNode(node);
        node = node->next;
        if (node != NULL && fleetIntelligence != NULL)
        {
            fleetIntelligenceDestroy(fleetIntelligence);
        }
        else
            break;
    }
}

void objectiveStartup(void)
{
    objectives = NULL;
    objectivesAllocated = 0;
    objectivesUsed = 0;

    feCallbackAddMultiple(poCallback); //add in the callbacks

    listInit(&poFleetIntelligence);
}

void objectiveShutdown(void)
{
    objectiveDestroyAll();
    //fleetIntelligenceDestroyAll();
}

//
//  Set status of an objective.
//  True == complete
//  False == not complete
//
void objectiveSet(char *label, sdword status)
{
    Objective *obj = objectiveFind(label);

    if (!obj)
        return;

    obj->status = status;
}

//
//  Get status of an objective.
//  True == complete
//  False == not complete
//
sdword objectiveGet(char *label)
{
    Objective *obj = objectiveFind(label);

    if (!obj)
        return FALSE;

    return obj->status;
}

//
//  Get status of all objectives.
//  True == all complete
//  False == not all complete
//
//  note that with no objectives, this will return TRUE
//
sdword objectiveGetAll(void)
{
    sdword i = 0;

    while (i < objectivesUsed)
    {
        if (!objectives[i]->status)
            return FALSE;
        ++i;
    }

    return TRUE;
}

//
//  remove one objective
//
void objectiveDestroy(char *label)
{
    sdword i = 0;
    Objective *obj = NULL;

    while (i < objectivesUsed)
    {
        if (!strncmp(objectives[i]->label, label, MAX_OBJECTIVE_LABEL_LENGTH))
        {
            obj = objectives[i];
            break;
        }
        ++i;
    }
    if (!obj)
        return;

    tbObjectivesListRemoveItem((ubyte *)obj);

    if (obj->fleetIntelligence)
    {
       if (obj->fleetIntelligence == poCurFleetIntelligence)
       {
           poCurFleetIntelligence = NULL;
       }
       fleetIntelligenceDestroy(obj->fleetIntelligence);
    }

    if (obj->description)
       memFree(obj->description);
    memFree(obj);

    --objectivesUsed;

    if (i < objectivesUsed)
        memmove(&objectives[i], &objectives[i+1], sizeof(Objective*)*(objectivesUsed-i));
}

//
//  Remove all objectives from a mission.
//
void objectiveDestroyAll(void)
{
    sdword i = 0;
    Objective *obj;

    // ASSUMING THIS GETS CALLED AT THE END OF A MISSION
    tbObjectivesListCleanUp();
    fleetIntelligenceDestroyAll();

    while (i < objectivesUsed)
    {
        obj = objectives[i];
        memFree(obj->description);
        memFree(obj);
        ++i;
    }
    objectivesUsed = 0;

    if (objectives)
        memFree(objectives);
    objectives = NULL;
    objectivesAllocated = 0;
}

//
//  finds an objective by label
//
//  returns NULL if not found
//
Objective *objectiveFind(char *label)
{
    sdword i = 0;

    while (i < objectivesUsed)
    {
        if (!strncmp(objectives[i]->label, label, MAX_OBJECTIVE_LABEL_LENGTH))
            return objectives[i];
        ++i;
    }
    return NULL;
}

//
//  this can be removed, NOW OBSOLETE (remove reference in MAINRGN.C)
//
void objectiveDrawStatus(void)
{
}

/*-----------------------------------------------------------------------------
    Name        : poPopupTextDraw
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void poPopupTextDraw(rectangle *rect)
{
    sdword x, y, width, rectWidth;
//    udword count;
    fonthandle currentFont;
    bool8 done, justified;
    char *pos, *oldpos;
    char oldline[256], line[256];

    if (!gameIsRunning || nisIsRunning || (tutorial==TUTORIAL_ONLY))
        return;

    rectWidth = (rect->x1 - rect->x0);


    poHeadingFont = frFontRegister(poHeadingFontName);
    poTextFont = frFontRegister(poTextFontName);


    //currentFont = fontMakeCurrent(poPopupFont);
    currentFont = fontMakeCurrent(poTextFont);

    if (poCurFleetIntelligence != NULL)
    {
        x = rect->x0;
        y = rect->y0;

        dbgAssert(poCurFleetIntelligence->description != NULL);

        pos = poCurFleetIntelligence->description;

        done = FALSE;
        while (!done)
        {
            justified = FALSE;
            line[0]=0;
            while (!justified)
            {
                strcpy(oldline, line);
                oldpos = pos;
                pos = getWord(line, pos);

                if (pos[0] == '\n')
                {
                    justified = TRUE;
                    pos++;
                    while ( pos[0] == ' ' )
                        pos++;
                }
                else
                {
                    if ( (width=fontWidth(line)) > rectWidth)
                    {
                        strcpy(line, oldline);
                        pos = oldpos;
                        while ( pos[0] == ' ' )
                            pos++;

                        justified = TRUE;
                    }
                    if (pos[0]==0)
                    {
                        justified = TRUE;
                        done      = TRUE;
                    }
                }
            }

            fontPrintf(x,y,OBJECTIVE_POPUPTEXT_COLOR,"%s",line);
            y += fontHeight(" ");
            if (y > rect->y1 + fontHeight(" "))
                done=TRUE;
        }
    }
    fontMakeCurrent(currentFont);
}


/*-----------------------------------------------------------------------------
    Name        : poPlayerObjectivesBegin
    Description : startup the mission objectives window
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void poPlayerObjectivesBegin(regionhandle region)
{
    spMainScreen();     // make sure we're on main screen

    // disbale rendering of main screen - NOT!
    //mrRenderMainScreen = FALSE;

    // disable taskbar popup window
    tbDisable = TRUE;

    smSensorsDisable = TRUE;
    mrDisable();

    universePause = TRUE;

    feDrawCallbackAddMultiple(poDrawCallback);

    poBaseRegion = feScreenStart(region, "Single_Player_Objective");
}


/*-----------------------------------------------------------------------------
    Name        : poPopupFleetIntelligence
    Description : Starts the fleet intel event for a given objective
    Inputs      : objective - objective clicked on
    Outputs     :
    Return      :
    Note        : Creates a KAS global variable with the name "G_CLICK_<label>"
----------------------------------------------------------------------------*/
void poPopupFleetIntelligence(Objective* objective)
{
    /*
    if (objective->fleetIntelligence != NULL)
    {
        poCurFleetIntelligence = objective->fleetIntelligence;
        poPlayerObjectivesBegin(ghMainRegion);
    }
    */
    char labelName[AIVAR_LABEL_MAX_LENGTH] = "CLICK_";
    AIVar *var;

    strcat(labelName, objective->label);
    dbgAssert(strlen(labelName) < AIVAR_LABEL_MAX_LENGTH);
    if ((var = aivarCreate(labelName)) != NULL)
    {
        var->value = TRUE;
    }
    subMessageEnded = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : poTextDraw
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void poTextDraw(featom *atom, regionhandle region)
{
    poPopupTextDraw(&region->rect);
}

/*-----------------------------------------------------------------------------
    Name        : poClose
    Description : Callback function to close the mission objectives window
    Inputs      :
    Outputs     : Deletes all regions associated with mission objectives window
    Return      :
----------------------------------------------------------------------------*/
void poClose(char *string, featom *atom)
{                                                           //close the window
#if CM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nClose mission objectives window.");
#endif
    feScreenDeleteFlags(poBaseRegion, FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND);
    poBaseRegion = NULL;

    // enable rendering of main game screen - NOT DISABLED!
    //mrRenderMainScreen = TRUE;

    // enable taskbar popup window
    tbDisable = FALSE;

    smSensorsDisable = FALSE;
    mrEnable();

    universePause = FALSE;

    if (poCurFleetIntelligence != NULL)
    {
        if (poCurFleetIntelligence->showOnce)
            fleetIntelligenceDestroy(poCurFleetIntelligence);

        poCurFleetIntelligence = NULL;
    }

    // shut up the fleet intel speech
    speechEventFleetIntelStop(1.0f);
}

/*=============================================================================
    Save Game Stuff:
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void SaveObjective(Objective *objective)
{
    Objective *sc;
    SaveChunk *chunk;

    chunk = CreateChunk(BASIC_STRUCTURE,sizeof(Objective),objective);
    sc = chunkContents(chunk);

    sc->fleetIntelligence = (FleetIntelligence *)ConvertPointerInListToNum(&poFleetIntelligence,sc->fleetIntelligence);

    SaveThisChunk(chunk);
    memFree(chunk);

    Save_String(objective->description);
}

Objective *LoadObjective()
{
    SaveChunk *chunk;
    Objective *objective;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE,sizeof(Objective));

    objective = memAlloc(sizeof(Objective),"objective",0);
    memcpy(objective,chunkContents(chunk),sizeof(Objective));

    memFree(chunk);

    objective->fleetIntelligence = ConvertNumToPointerInList(&poFleetIntelligence,(sdword)objective->fleetIntelligence);

    objective->description = Load_String();

    return objective;
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

void SaveFleetIntelligence(void *stuff)
{
    SaveStructureOfSize(stuff,sizeof(FleetIntelligence));
    Save_String(((FleetIntelligence *)stuff)->description);
}

void LoadFleetIntelligence(LinkedList *list)
{
    FleetIntelligence *fleetintelligence = LoadStructureOfSize(sizeof(FleetIntelligence));
    fleetintelligence->description = Load_String();

    listAddNode(list,&fleetintelligence->node,fleetintelligence);
}

void objectiveSave(void)
{
    sdword i;

    SaveLinkedListOfStuff(&poFleetIntelligence,SaveFleetIntelligence);

    // Save objectives

    SaveInfoNumber(objectivesUsed);
    SaveInfoNumber(objectivesAllocated);

    for (i=0;i<objectivesUsed;i++)
    {
        SaveObjective(objectives[i]);
    }
}

void objectiveLoad(void)
{
    sdword i;
    Objective *hyperspaceObjective = NULL;

    poCurFleetIntelligence = NULL;

    LoadLinkedListOfStuff(&poFleetIntelligence,LoadFleetIntelligence);

    // Load objectives

    objectivesUsed = LoadInfoNumber();
    objectivesAllocated = LoadInfoNumber();

    if (objectivesAllocated > 0)
    {
        objectives = memAlloc(sizeof(Objective*)*objectivesAllocated, "objectives", 0);
    }
    else
    {
        objectives = NULL;
    }

    for (i=0;i<objectivesUsed;i++)
    {
        objectives[i] = LoadObjective();
        if (!strcasecmp(objectives[i]->label, "hyperspace"))
        {
            dbgAssert(hyperspaceObjective == NULL);
            hyperspaceObjective = objectives[i];
        }
        else
            tbObjectivesListAddItem((ubyte *)objectives[i]);
    }

    // make sure hyperspace objective always goes to head of list
    if (hyperspaceObjective)
    {
        //tbObjectivesListAddItemToHead((ubyte*)hyperspaceObjective);
        tbObjectivesHyperspace((ubyte*)hyperspaceObjective);
    }

    feDrawCallbackAddMultiple(poDrawCallback);
}

