/*=============================================================================
    Name    : Researchapi.h
    Purpose : definitions for the research manager

    Created 5/25/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___RESEARCHAPI_H
#define ___RESEARCHAPI_H

#include "Types.h"
#include "Region.h"

/*=============================================================================
    Definitions:
=============================================================================*/

// convert technology from enum to bit position in masks
#define TechToBit(tech) (udword)((udword)1<<(tech))
#define AllTechnology   (udword)(0x7FFFFFFF)

// lab status defines
#define LS_NORESEARCHSHIP   0
#define LS_NORESEARCHITEM   1
#define LS_RESEARCHITEM     2
#define LS_RESEARCHITEMSOON 3

// number of labs
#define NUM_RESEARCHLABS    6

// defines for level of technology
#define TECH_NOTECHNOLOGY       0
#define TECH_ALLTECHNOLOGY      1

#define RM_Disabled             BIT31
/*=============================================================================
    Type definitions:
=============================================================================*/


// list of all technologies that can be researched
typedef enum
{
    NewAlloys=0,
    MassDrive1Kt,
    CoolingSystems,
    CloakDefenseFighter,
    TargetingSystems,
    PlasmaWeapons,
    Chassis1,
    MassDrive10Kt,
    MediumGuns,
    MineLayerTech,
    Chassis2,
    AdvancedCoolingSystems,
    MassDrive100Kt,
    FireControl,
    SupportRefuelTech,
    AdvanceTacticalSupport,
    IonWeapons,
    DDDFDFGFTech,
    Chassis3,
    MassDrive1Mt,
    AdvancedFireControl,
    MissileWeapons,
    ConstructionTech,
    HeavyGuns,
    ProximityDetector,
    SensorsArrayTech,
    GravityWellGeneratorTech,
    CloakGeneratorTech,
    RepairTech,
    SalvageTech,
    NumTechnologies
} TechnologyType;


// defines for the technology dependancies and ship technology requirements

typedef struct
{
    udword TechNeededToBuildShip[TOTAL_STD_SHIPS];   // technologies needed before this ship can be built
    udword TechNeededToResearch[NumTechnologies];  // technology needed before this can be researched
    sdword TimeToComplete[NumTechnologies];        // time in seconds technology takes to finish
} TechStatics;


// defines for the research labs, and player status.

typedef struct
{
    struct Node     link;               // link for linkedlist of research topics
    TechnologyType  techresearch;       // what tech is being researched
    sdword          numlabsresearching; // how many labs are working on this
    real32          timeleft;           // how much time left in seconds
} ResearchTopic;

typedef struct
{
    sdword          labstatus;          // what is the status of this lab
    ResearchTopic  *topic;              // pointer to topic this lab is researching
} ResearchLab;

typedef struct
{
    udword       HasTechnology;         // what tech this player has
    bool         CanDoResearch;         // are they allowed to do research
    TechStatics *techstat;              // statics for that technology
    LinkedList   listoftopics;          // list of topics being researched for this player
    ResearchLab  researchlabs[NUM_RESEARCHLABS];  // list of labs for this player
} PlayerResearchInfo;


/*=============================================================================
    Data:
=============================================================================*/

//extern TechnologyType         SetTechLevel;
extern udword         SetTechLevel;

/*=============================================================================
    Function Prototypes:
=============================================================================*/

// callback to set dependancies
void rmSetShipDependCB(char *directory, char *field, void *dataToFillIn);
void rmSetTechDependCB(char *directory, char *field, void *dataToFillIn);

char *RaceSpecificTechTypeToNiceString(TechnologyType tech, ShipRace race);
char *TechTypeToNiceString(TechnologyType tech);
char *TechTypeToString    (TechnologyType tech);
TechnologyType StrToTechType(char *tech);

// api function calls
ResearchTopic *Researching(struct Player *player, TechnologyType tech);

sdword rmResearchingAnything(struct Player *player);

void rmGiveTechToPlayerByName(struct Player *player, char *techName);
void rmGiveTechToPlayerByType(struct Player *player, TechnologyType techtype);

void   rmAddTechToPlayer(struct Player *player, udword techlevel);
sdword rmFindFreeLab(struct Player *player);
bool   rmCanBuildShip(struct Player *player, ShipType type);
//void   rmResearchTechForShip(struct Player *player, ShipType type);
void   rmClearResearchlab(struct Player *player, sdword labnumber);
void   rmAssignPlayersLabToResearch(struct Player *player, sdword labnumber, TechnologyType tech);
void   rmDeactivateLab(struct Player *player);
void   rmActivateFreeLab(struct Player *player);
void   rmUpdateResearch(void);
void   rmInitializeResearchStruct(struct Player *player, bool candoresearch, sdword techlevel);

sdword rmTechRequiredForShip(struct Player *player, ShipType type);
bool   rmResearchTechForShip(struct Player *player, ShipType type);

void rmInitializeResearchStatics(struct Player *player);

void rmEnableShip(ShipRace race, ShipType ship, bool bEnabled);
void rmRemoveAllUnneededTech(void);
void rmResetStaticInfo(void);

//start the Research Manager.
void   rmAPIStartup(void);
void   rmAPIShutdown(void);

#endif
