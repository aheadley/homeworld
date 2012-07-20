#ifndef __OBJECTIVES_H
#define __OBJECTIVES_H

//
//  SINGLE-PLAYER MISSION OBJECTIVES
//

#include "Types.h"
#include "FEFlow.h"
#include "LinkedList.h"

#define MAX_OBJECTIVE_LABEL_LENGTH 32
#define OBJECTIVES_ALLOC_INITIAL 8
#define OBJECTIVES_ALLOC_INCREMENT 8

typedef struct {
    Node   node;         // Linked list node
    bool8  showOnce;     // Shown and then removed from linked list
    bool8  showNow;      // Internal status of intelligence window (force creating popup)
    char   *description; // Pointer to fleet intelligence text array
}
FleetIntelligence;

typedef struct {
    char label[MAX_OBJECTIVE_LABEL_LENGTH+1];
    char *description;                    // Objectives described once
    FleetIntelligence *fleetIntelligence; // So the task bar knows which window to bring up on double clicking
    sdword status;                        // Complete / Incomplete
    sdword primary;
}
Objective;

extern LinkedList poFleetIntelligence;

void poPlayerObjectivesBegin(regionhandle region);
void poClose(char *string, featom *atom);

void objectiveStartup(void);
void objectiveShutdown(void);

void objectiveSave(void);
void objectiveLoad(void);

// *briefDescription is the text found in the list box under the taskbar
// *fullDescription is the text found in the fleet intelligence window
// The fullDescription pointer can be NULL
// showOnce forces Fleet Intelligence to be drawn once and then deleted
Objective *objectiveAndFleetIntelligenceCreate(char *label, char *briefDescription, char* fullDescription, bool8 showOnce, bool primary);

// Called independantly if you wish to show fleet intelligence only once and right away
FleetIntelligence *fleetIntelligenceCreate(char *description, bool8 showOnce);

Objective *objectiveFind(char *label);
void objectiveSet(char *label, sdword status);
sdword objectiveGet(char *label);
sdword objectiveGetAll(void);
void objectiveDestroyAll(void);
void objectiveDestroy(char *label);

void objectiveDrawStatus(void); // OBSOLETE
void objectivePopupAll(void);

void poPopupFleetIntelligence(Objective* objective);

#endif
