/*=============================================================================
    Name    : researchapi.c
    Purpose : logic for the research manager api functions.

    Created 5/27/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>

#include "Types.h"
#include "ShipDefs.h"
#include "ObjTypes.h"
#include "ResearchAPI.h"
#include "Universe.h"
#include "StatScript.h"
#include "ResearchGUI.h"
#include "Strings.h"
#include "LinkedList.h"
#include "SoundEvent.h"

#include "FEReg.h"
#include "Region.h"
#include "TradeMgr.h"

#include "MultiplayerGame.h"
#include "ConsMgr.h"

#define makeSPR1TechEntry(var) {"SPR1" #var, scriptSetSdwordCB, &SinglePlayerR1TechStatic.TimeToComplete[var]}
#define makeSPR2TechEntry(var) {"SPR2" #var, scriptSetSdwordCB, &SinglePlayerR2TechStatic.TimeToComplete[var]}
#define makeMPR1TechEntry(var) {"MPR1" #var, scriptSetSdwordCB, &MultiPlayerR1TechStatic.TimeToComplete[var]}
#define makeMPR2TechEntry(var) {"MPR2" #var, scriptSetSdwordCB, &MultiPlayerR2TechStatic.TimeToComplete[var]}
#define makeMPCR1TechEntry(var) {"MPCR1" #var, scriptSetSdwordCB, &MultiPlayerCR1TechStatic.TimeToComplete[var]}
#define makeMPCR2TechEntry(var) {"MPCR2" #var, scriptSetSdwordCB, &MultiPlayerCR2TechStatic.TimeToComplete[var]}

/*=============================================================================
    Data:
=============================================================================*/

TechStatics            SinglePlayerR1TechStatic;
TechStatics            SinglePlayerR2TechStatic;

TechStatics            MultiPlayerR1TechStatic;
TechStatics            MultiPlayerR2TechStatic;

TechStatics            MultiPlayerCR1TechStatic;
TechStatics            MultiPlayerCR2TechStatic;

real32                 labdec[NUM_RESEARCHLABS+1]=
{
    (real32)0.0,    // time decrement for combined lab research
    (real32)1.0,    // 1 lab = 1.0
    (real32)1.6,
    (real32)2.25,
    (real32)2.8,
    (real32)3.25,
    (real32)3.6
};

scriptEntry TechnologyDependancies[] =
{
    { "SPR1ShipDepend", rmSetShipDependCB, &SinglePlayerR1TechStatic.TechNeededToBuildShip[0]},
    { "SPR1TechDepend", rmSetTechDependCB, &SinglePlayerR1TechStatic.TechNeededToResearch[0]},
    makeSPR1TechEntry(NewAlloys),
    makeSPR1TechEntry(MassDrive1Kt),
    makeSPR1TechEntry(CoolingSystems),
    makeSPR1TechEntry(CloakDefenseFighter),
    makeSPR1TechEntry(TargetingSystems),
    makeSPR1TechEntry(PlasmaWeapons),
    makeSPR1TechEntry(Chassis1),
    makeSPR1TechEntry(MassDrive10Kt),
    makeSPR1TechEntry(MediumGuns),
    makeSPR1TechEntry(MineLayerTech),
    makeSPR1TechEntry(Chassis2),
    makeSPR1TechEntry(AdvancedCoolingSystems),
    makeSPR1TechEntry(MassDrive100Kt),
    makeSPR1TechEntry(FireControl),
    makeSPR1TechEntry(SupportRefuelTech),
    makeSPR1TechEntry(AdvanceTacticalSupport),
    makeSPR1TechEntry(IonWeapons),
    makeSPR1TechEntry(DDDFDFGFTech),
    makeSPR1TechEntry(Chassis3),
    makeSPR1TechEntry(MassDrive1Mt),
    makeSPR1TechEntry(AdvancedFireControl),
    makeSPR1TechEntry(MissileWeapons),
    makeSPR1TechEntry(ConstructionTech),
    makeSPR1TechEntry(HeavyGuns),
    makeSPR1TechEntry(ProximityDetector),
    makeSPR1TechEntry(SensorsArrayTech),
    makeSPR1TechEntry(GravityWellGeneratorTech),
    makeSPR1TechEntry(CloakGeneratorTech),
    makeSPR1TechEntry(RepairTech),
    makeSPR1TechEntry(SalvageTech),

    { "SPR2ShipDepend", rmSetShipDependCB, &SinglePlayerR2TechStatic.TechNeededToBuildShip[0]},
    { "SPR2TechDepend", rmSetTechDependCB, &SinglePlayerR2TechStatic.TechNeededToResearch[0]},
    makeSPR2TechEntry(NewAlloys),
    makeSPR2TechEntry(MassDrive1Kt),
    makeSPR2TechEntry(CoolingSystems),
    makeSPR2TechEntry(CloakDefenseFighter),
    makeSPR2TechEntry(TargetingSystems),
    makeSPR2TechEntry(PlasmaWeapons),
    makeSPR2TechEntry(Chassis1),
    makeSPR2TechEntry(MassDrive10Kt),
    makeSPR2TechEntry(MediumGuns),
    makeSPR2TechEntry(MineLayerTech),
    makeSPR2TechEntry(Chassis2),
    makeSPR2TechEntry(AdvancedCoolingSystems),
    makeSPR2TechEntry(MassDrive100Kt),
    makeSPR2TechEntry(FireControl),
    makeSPR2TechEntry(SupportRefuelTech),
    makeSPR2TechEntry(AdvanceTacticalSupport),
    makeSPR2TechEntry(IonWeapons),
    makeSPR2TechEntry(DDDFDFGFTech),
    makeSPR2TechEntry(Chassis3),
    makeSPR2TechEntry(MassDrive1Mt),
    makeSPR2TechEntry(AdvancedFireControl),
    makeSPR2TechEntry(MissileWeapons),
    makeSPR2TechEntry(ConstructionTech),
    makeSPR2TechEntry(HeavyGuns),
    makeSPR2TechEntry(ProximityDetector),
    makeSPR2TechEntry(SensorsArrayTech),
    makeSPR2TechEntry(GravityWellGeneratorTech),
    makeSPR2TechEntry(CloakGeneratorTech),
    makeSPR2TechEntry(RepairTech),
    makeSPR2TechEntry(SalvageTech),

    { "MPR1ShipDepend", rmSetShipDependCB, &MultiPlayerR1TechStatic.TechNeededToBuildShip[0]},
    { "MPR1TechDepend", rmSetTechDependCB, &MultiPlayerR1TechStatic.TechNeededToResearch[0]},
    makeMPR1TechEntry(NewAlloys),
    makeMPR1TechEntry(MassDrive1Kt),
    makeMPR1TechEntry(CoolingSystems),
    makeMPR1TechEntry(CloakDefenseFighter),
    makeMPR1TechEntry(TargetingSystems),
    makeMPR1TechEntry(PlasmaWeapons),
    makeMPR1TechEntry(Chassis1),
    makeMPR1TechEntry(MassDrive10Kt),
    makeMPR1TechEntry(MediumGuns),
    makeMPR1TechEntry(MineLayerTech),
    makeMPR1TechEntry(Chassis2),
    makeMPR1TechEntry(AdvancedCoolingSystems),
    makeMPR1TechEntry(MassDrive100Kt),
    makeMPR1TechEntry(FireControl),
    makeMPR1TechEntry(SupportRefuelTech),
    makeMPR1TechEntry(AdvanceTacticalSupport),
    makeMPR1TechEntry(IonWeapons),
    makeMPR1TechEntry(DDDFDFGFTech),
    makeMPR1TechEntry(Chassis3),
    makeMPR1TechEntry(MassDrive1Mt),
    makeMPR1TechEntry(AdvancedFireControl),
    makeMPR1TechEntry(MissileWeapons),
    makeMPR1TechEntry(ConstructionTech),
    makeMPR1TechEntry(HeavyGuns),
    makeMPR1TechEntry(ProximityDetector),
    makeMPR1TechEntry(SensorsArrayTech),
    makeMPR1TechEntry(GravityWellGeneratorTech),
    makeMPR1TechEntry(CloakGeneratorTech),
    makeMPR1TechEntry(RepairTech),
    makeMPR1TechEntry(SalvageTech),

    { "MPR2ShipDepend", rmSetShipDependCB, &MultiPlayerR2TechStatic.TechNeededToBuildShip[0]},
    { "MPR2TechDepend", rmSetTechDependCB, &MultiPlayerR2TechStatic.TechNeededToResearch[0]},
    makeMPR2TechEntry(NewAlloys),
    makeMPR2TechEntry(MassDrive1Kt),
    makeMPR2TechEntry(CoolingSystems),
    makeMPR2TechEntry(CloakDefenseFighter),
    makeMPR2TechEntry(TargetingSystems),
    makeMPR2TechEntry(PlasmaWeapons),
    makeMPR2TechEntry(Chassis1),
    makeMPR2TechEntry(MassDrive10Kt),
    makeMPR2TechEntry(MediumGuns),
    makeMPR2TechEntry(MineLayerTech),
    makeMPR2TechEntry(Chassis2),
    makeMPR2TechEntry(AdvancedCoolingSystems),
    makeMPR2TechEntry(MassDrive100Kt),
    makeMPR2TechEntry(FireControl),
    makeMPR2TechEntry(SupportRefuelTech),
    makeMPR2TechEntry(AdvanceTacticalSupport),
    makeMPR2TechEntry(IonWeapons),
    makeMPR2TechEntry(DDDFDFGFTech),
    makeMPR2TechEntry(Chassis3),
    makeMPR2TechEntry(MassDrive1Mt),
    makeMPR2TechEntry(AdvancedFireControl),
    makeMPR2TechEntry(MissileWeapons),
    makeMPR2TechEntry(ConstructionTech),
    makeMPR2TechEntry(HeavyGuns),
    makeMPR2TechEntry(ProximityDetector),
    makeMPR2TechEntry(SensorsArrayTech),
    makeMPR2TechEntry(GravityWellGeneratorTech),
    makeMPR2TechEntry(CloakGeneratorTech),
    makeMPR2TechEntry(RepairTech),
    makeMPR2TechEntry(SalvageTech),

    { "MPCR1ShipDepend", rmSetShipDependCB, &MultiPlayerCR1TechStatic.TechNeededToBuildShip[0]},
    { "MPCR1TechDepend", rmSetTechDependCB, &MultiPlayerCR1TechStatic.TechNeededToResearch[0]},
    makeMPCR1TechEntry(NewAlloys),
    makeMPCR1TechEntry(MassDrive1Kt),
    makeMPCR1TechEntry(CoolingSystems),
    makeMPCR1TechEntry(CloakDefenseFighter),
    makeMPCR1TechEntry(TargetingSystems),
    makeMPCR1TechEntry(PlasmaWeapons),
    makeMPCR1TechEntry(Chassis1),
    makeMPCR1TechEntry(MassDrive10Kt),
    makeMPCR1TechEntry(MediumGuns),
    makeMPCR1TechEntry(MineLayerTech),
    makeMPCR1TechEntry(Chassis2),
    makeMPCR1TechEntry(AdvancedCoolingSystems),
    makeMPCR1TechEntry(MassDrive100Kt),
    makeMPCR1TechEntry(FireControl),
    makeMPCR1TechEntry(SupportRefuelTech),
    makeMPCR1TechEntry(AdvanceTacticalSupport),
    makeMPCR1TechEntry(IonWeapons),
    makeMPCR1TechEntry(DDDFDFGFTech),
    makeMPCR1TechEntry(Chassis3),
    makeMPCR1TechEntry(MassDrive1Mt),
    makeMPCR1TechEntry(AdvancedFireControl),
    makeMPCR1TechEntry(MissileWeapons),
    makeMPCR1TechEntry(ConstructionTech),
    makeMPCR1TechEntry(HeavyGuns),
    makeMPCR1TechEntry(ProximityDetector),
    makeMPCR1TechEntry(SensorsArrayTech),
    makeMPCR1TechEntry(GravityWellGeneratorTech),
    makeMPCR1TechEntry(CloakGeneratorTech),
    makeMPCR1TechEntry(RepairTech),
    makeMPCR1TechEntry(SalvageTech),

    { "MPCR2ShipDepend", rmSetShipDependCB, &MultiPlayerCR2TechStatic.TechNeededToBuildShip[0]},
    { "MPCR2TechDepend", rmSetTechDependCB, &MultiPlayerCR2TechStatic.TechNeededToResearch[0]},
    makeMPCR2TechEntry(NewAlloys),
    makeMPCR2TechEntry(MassDrive1Kt),
    makeMPCR2TechEntry(CoolingSystems),
    makeMPCR2TechEntry(CloakDefenseFighter),
    makeMPCR2TechEntry(TargetingSystems),
    makeMPCR2TechEntry(PlasmaWeapons),
    makeMPCR2TechEntry(Chassis1),
    makeMPCR2TechEntry(MassDrive10Kt),
    makeMPCR2TechEntry(MediumGuns),
    makeMPCR2TechEntry(MineLayerTech),
    makeMPCR2TechEntry(Chassis2),
    makeMPCR2TechEntry(AdvancedCoolingSystems),
    makeMPCR2TechEntry(MassDrive100Kt),
    makeMPCR2TechEntry(FireControl),
    makeMPCR2TechEntry(SupportRefuelTech),
    makeMPCR2TechEntry(AdvanceTacticalSupport),
    makeMPCR2TechEntry(IonWeapons),
    makeMPCR2TechEntry(DDDFDFGFTech),
    makeMPCR2TechEntry(Chassis3),
    makeMPCR2TechEntry(MassDrive1Mt),
    makeMPCR2TechEntry(AdvancedFireControl),
    makeMPCR2TechEntry(MissileWeapons),
    makeMPCR2TechEntry(ConstructionTech),
    makeMPCR2TechEntry(HeavyGuns),
    makeMPCR2TechEntry(ProximityDetector),
    makeMPCR2TechEntry(SensorsArrayTech),
    makeMPCR2TechEntry(GravityWellGeneratorTech),
    makeMPCR2TechEntry(CloakGeneratorTech),
    makeMPCR2TechEntry(RepairTech),
    makeMPCR2TechEntry(SalvageTech),


    endEntry
};

static NumStrXlate techtypeinfo[] =
{
    { (udword)NewAlloys,                str$(NewAlloys)},
    { (udword)MassDrive1Kt,             str$(MassDrive1Kt)},
    { (udword)CoolingSystems,           str$(CoolingSystems)},
    { (udword)CloakDefenseFighter,      str$(CloakDefenseFighter)},
    { (udword)TargetingSystems,         str$(TargetingSystems)},
    { (udword)PlasmaWeapons,            str$(PlasmaWeapons)},
    { (udword)Chassis1,                 str$(Chassis1)},
    { (udword)MassDrive10Kt,            str$(MassDrive10Kt)},
    { (udword)MediumGuns,               str$(MediumGuns)},
    { (udword)MineLayerTech,            str$(MineLayerTech)},
    { (udword)Chassis2,                 str$(Chassis2)},
    { (udword)AdvancedCoolingSystems,   str$(AdvancedCoolingSystems)},
    { (udword)MassDrive100Kt,           str$(MassDrive100Kt)},
    { (udword)FireControl,              str$(FireControl)},
    { (udword)SupportRefuelTech,        str$(SupportRefuelTech)},
    { (udword)AdvanceTacticalSupport,   str$(AdvanceTacticalSupport)},
    { (udword)IonWeapons,               str$(IonWeapons)},
    { (udword)DDDFDFGFTech,             str$(DDDFDFGFTech)},
    { (udword)Chassis3,                 str$(Chassis3)},
    { (udword)MassDrive1Mt,             str$(MassDrive1Mt)},
    { (udword)AdvancedFireControl,      str$(AdvancedFireControl)},
    { (udword)MissileWeapons,           str$(MissileWeapons)},
    { (udword)ConstructionTech,         str$(ConstructionTech)},
    { (udword)HeavyGuns,                str$(HeavyGuns)},
    { (udword)ProximityDetector,        str$(ProximityDetector)},
    { (udword)SensorsArrayTech,         str$(SensorsArrayTech)},
    { (udword)GravityWellGeneratorTech, str$(GravityWellGeneratorTech)},
    { (udword)CloakGeneratorTech,       str$(CloakGeneratorTech)},
    { (udword)RepairTech,               str$(RepairTech)},
    { (udword)SalvageTech,              str$(SalvageTech)},
    { (udword)NumTechnologies,          str$(NumTechnologies)},
    { 0,NULL }
};



/*=============================================================================
    Functions:
=============================================================================*/

char *RaceSpecificTechTypeToNiceString(TechnologyType tech, ShipRace race)
{
    if (tech==DDDFDFGFTech)
        if (race==R1)
            return strGetString(strDDDFTech);
        else
            return strGetString(strDFGFTech);
    else if (tech==CloakDefenseFighter)
        if (race==R1)
            return strGetString(strCloakFighter);
        else
            return strGetString(strDefenseFighterTech);
    else
        return TechTypeToNiceString(tech);

}

char *TechTypeToNiceString(TechnologyType tech)
{
    return (strGetString((uword)tech+strTechOffset));
}

char *TechTypeToString(TechnologyType tech)
{
    return NumToStr(techtypeinfo,(uword)tech);
}

TechnologyType StrToTechType(char *tech)
{
    return (StrToNum(techtypeinfo, tech));
}


/*-----------------------------------------------------------------------------
    Name        : Reseaching
    Description : checks to see if the passed in tech is currently being researched.
    Inputs      : tech
    Outputs     : true false
    Return      : bool
----------------------------------------------------------------------------*/
ResearchTopic *Researching(Player *player, TechnologyType tech)
{
    PlayerResearchInfo *research=&player->researchinfo;
    ResearchTopic      *topic;
    Node               *walk;

    walk = research->listoftopics.head;

    // walk through list of topics being researched
    while (walk != NULL)
    {
        topic = (ResearchTopic *)listGetStructOfNode(walk);
        if (topic->techresearch == tech)
            return(topic);
        walk = walk->next;
    }

    return(NULL);
}

//
//  returns 1 if the player is currently researching anything
//  returns 0 otherwise
//
sdword rmResearchingAnything(struct Player *player)
{
    PlayerResearchInfo *research=&player->researchinfo;

    return research->listoftopics.head != NULL;
}

void   rmAddTechToPlayer(struct Player *player, udword techlevel)
{
    player->researchinfo.HasTechnology |= techlevel;
}


/*-----------------------------------------------------------------------------
    Name        : rmCanBuildShip
    Description : returns true if player has tech needed to build that ship.
    Inputs      : player
    Outputs     : true/false
    Return      : bool
----------------------------------------------------------------------------*/
bool rmCanBuildShip(Player *player, ShipType type)
{
    if (bitTest(player->researchinfo.techstat->TechNeededToBuildShip[type], RM_Disabled))
    {
        return(FALSE);
    }
    return ( (player->researchinfo.techstat->TechNeededToBuildShip[type]&
              player->researchinfo.HasTechnology) ==
             player->researchinfo.techstat->TechNeededToBuildShip[type]);
}


/*-----------------------------------------------------------------------------
    Name        : rmFindFreeLab
    Description : finds a free lab
    Inputs      : player
    Outputs     : ResearchLab, or -1
    Return      : sdword
----------------------------------------------------------------------------*/
sdword rmFindFreeLab(Player *player)
{
    sdword       labindex;
    ResearchLab *lab;

    for (labindex=0;labindex<NUM_RESEARCHLABS;labindex++)
    {
        lab = &player->researchinfo.researchlabs[labindex];
        if (lab->labstatus==LS_NORESEARCHITEM)
        {
            if ( (rmGUIActive) && (player->aiPlayer == NULL) )
                 rmUpdateTechList();
            return (labindex);
        }
    }
    return (-1);
}


/*-----------------------------------------------------------------------------
    Name        : rmClearResearchlab
    Description : Clears the specified labs research status.
    Inputs      : lab to clear, and player
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmClearResearchlab(Player *player, sdword labnumber)
{
    ResearchLab   *lab = &player->researchinfo.researchlabs[labnumber];
    ResearchTopic *topic = player->researchinfo.researchlabs[labnumber].topic;

    if (lab->labstatus==LS_RESEARCHITEM)
    {
        if (topic->numlabsresearching==1)
        {
            lab->topic = NULL;
            listDeleteNode(&topic->link);
            lab->labstatus = LS_NORESEARCHITEM;
        }
        else
        {
            lab->topic = NULL;
            lab->labstatus = LS_NORESEARCHITEM;
            topic->numlabsresearching--;
        }
        if ((player == universe.curPlayerPtr)&&(rmGUIActive)) rmUpdateTechList();
    }
}


/*-----------------------------------------------------------------------------
    Name        : rmAssignPlayersLabToResearch
    Description : assigns a lab to a certain research item
    Inputs      : Lab number, research topic number
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmAssignPlayersLabToResearch(Player *player, sdword labnumber, TechnologyType tech)
{
    ResearchTopic *topic;
    if ((topic=Researching(player, tech))!=NULL)
    {
        player->researchinfo.researchlabs[labnumber].labstatus = LS_RESEARCHITEM;
        player->researchinfo.researchlabs[labnumber].topic = topic;
        topic->numlabsresearching++;
    }
    else
    {
        topic = (ResearchTopic *)memAlloc(sizeof(ResearchTopic),"ResearchTopic",NonVolatile);
        listAddNode(&player->researchinfo.listoftopics, &topic->link, topic);
        player->researchinfo.researchlabs[labnumber].labstatus = LS_RESEARCHITEM;
        player->researchinfo.researchlabs[labnumber].topic = topic;
        topic->numlabsresearching = 1;
        topic->techresearch       = tech;
        topic->timeleft           = (real32)player->researchinfo.techstat->TimeToComplete[tech];
    }

    if ((player == universe.curPlayerPtr)&&(rmGUIActive)) rmUpdateTechList();
}

/*-----------------------------------------------------------------------------
    Name        : rmDeactivateLab
    Description : deactivates a research lab, if ship dies lab dies too.
    Inputs      : player
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmDeactivateLab(Player *player)
{
    sdword index, maxindex=-1;
    real32 maxtimeleft=0.0;
    ResearchLab *lab;

    index = rmFindFreeLab(player);

    if (index!=-1)
    {
        player->researchinfo.researchlabs[index].labstatus = LS_NORESEARCHSHIP;
    }
    else
    {
        for (index=0;index<NUM_RESEARCHLABS;index++)
        {
            lab = &player->researchinfo.researchlabs[index];
            if (lab->labstatus!=LS_NORESEARCHSHIP)
            {
                if (lab->topic->timeleft > maxtimeleft)
                {
                    maxtimeleft = lab->topic->timeleft;
                    maxindex    = index;
                }
            }
        }

        rmClearResearchlab(player, maxindex);
        player->researchinfo.researchlabs[maxindex].labstatus = LS_NORESEARCHSHIP;
    }
    if ((player == universe.curPlayerPtr)&&(rmGUIActive)) rmUpdateTechList();
}


/*-----------------------------------------------------------------------------
    Name        : rmActivateFreeLab
    Description : Activates a lab.
    Inputs      : player that requires a lab to be activated
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmActivateFreeLab(Player *player)
{
    sdword       labindex;
    ResearchLab *lab;

    for (labindex=0;labindex<NUM_RESEARCHLABS;labindex++)
    {
        lab = &player->researchinfo.researchlabs[labindex];
        if (lab->labstatus==LS_NORESEARCHSHIP)
        {
            // if lab not assigned then activate the lab
            lab->labstatus=LS_NORESEARCHITEM;
            if ((player == universe.curPlayerPtr)&&(rmGUIActive)) rmUpdateTechList();
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmUpdateResearch
    Description : updates all research
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmUpdateResearch(void)
{
    sdword              index, labindex, i;
    ResearchLab        *lab;
    PlayerResearchInfo *research;
    ResearchTopic      *topic;
    Node               *walk;
    LinkedList          deletelist;
    bool                shipcanbuild[STD_LAST_SHIP];

    listInit(&deletelist);

    // search through list of players
    for (index=0;index<universe.numPlayers;index++)
    {
        research = &universe.players[index].researchinfo;

        if (research->CanDoResearch)
        {
            walk = research->listoftopics.head;

            // walk through list of topics being researched
            while (walk != NULL)
            {
                topic = (ResearchTopic *)listGetStructOfNode(walk);
                topic->timeleft -= labdec[topic->numlabsresearching];
                if (topic->timeleft < 0)
                {
                    for (i = 0; i < STD_LAST_SHIP; i++)
                    {
                        shipcanbuild[i] = rmCanBuildShip(&(universe.players[index]), i);
                    }

                    // Insert sound for technology completed
                    if (universe.players[index].race == R1)
                    {
                        speechEventFleet(STAT_F_Research_R1_Completed, topic->techresearch, index);
                    }
                    else if (universe.players[index].race == R2)
                    {
                        speechEventFleet(STAT_F_Research_R2_Completed, topic->techresearch, index);
                    }

                    topic->timeleft = 0;

                    research->HasTechnology |= TechToBit(topic->techresearch);



                    listRemoveNode(&topic->link);
                    listAddNode(&deletelist,&topic->link,topic);

                    for (labindex=0;labindex<NUM_RESEARCHLABS;labindex++)
                    {
                        lab = &research->researchlabs[labindex];
                        if (lab->topic==topic)
                        {
                            lab->labstatus = LS_NORESEARCHITEM;
                            lab->topic     = NULL;
                            if (rmGUIActive)
                                rmClearLab(labindex);
                        }
                    }

                    for (i = 0; i < STD_LAST_SHIP; i++)
                    {
                        if (shipcanbuild[i] != rmCanBuildShip(&(universe.players[index]), i))
                        {
                            speechEventFleet(STAT_F_Research_CompletedShip, i, index);
                            if (cmActive)
                            {
                                cmUpdateShipsAvailable();
                            }
                        }
                    }

                    if (singlePlayerGame && (index == 0))
                    {
                        tmTechForSale[topic->techresearch] = TM_TECH_IS_ALREADY_OWNED;
                    }

                    if (rmGUIActive)
                        rmUpdateTechList();
                }

                walk = walk->next;
            }

            listDeleteAll(&deletelist);
        }
    }
}

sdword rmNumTechRequiredForTech(Player *player, TechnologyType type)
{
    sdword i, numtech=0;
    udword techdepend, temp;

    temp=player->researchinfo.techstat->TechNeededToResearch[type];
    techdepend = temp & player->researchinfo.HasTechnology;
    techdepend ^= temp;

    for (i=0;i<NumTechnologies;i++)
    {
        if (bitTest(techdepend, TechToBit(i)))
        {
            if ((temp=player->researchinfo.techstat->TechNeededToResearch[i])!=0)
            {
                numtech += rmNumTechRequiredForTech(player, i);
            }
            numtech ++;
        }
    }
    return (numtech);
}

udword rmTechRequiredForTech(Player *player, TechnologyType type)
{
    sdword i;
    udword techdepend, temp;

    temp=player->researchinfo.techstat->TechNeededToResearch[type];
    techdepend = temp & player->researchinfo.HasTechnology;
    techdepend ^= temp;

    for (i=NumTechnologies;i>=0;i--)
    {
        if (bitTest(techdepend, TechToBit(i)))
        {
            return (i);
        }
    }
    return (0);
}

/*-----------------------------------------------------------------------
   Name        : rmAllTechRequiredforTech
   Description : Returns all of the technology needed to research a tech
   Inputs      : player techtype
   Outputs     : technology bitmask
   Parameters  : Player *player, TechnologyType type
   Return      : udword
-----------------------------------------------------------------------*/
udword rmAllTechRequiredforTech(Player *player, TechnologyType type)
{
    udword i;
    udword techdepend,temp;

    techdepend = temp = player->researchinfo.techstat->TechNeededToResearch[type];

    for (i=0;i<NumTechnologies;i--)
    {
        if (bitTest(temp, TechToBit(i)))
        {
            techdepend |= rmAllTechRequiredforTech(player, i);
        }
    }

    return (techdepend);
}


/*-----------------------------------------------------------------------
    Name        : rmAllTechRequredForShip
    Description : Returns all of the technolgy needed to research a ship
    Inputs      : player and shiptype
    Outputs     : tech bitmask of tech required for that ship
    Parameters  : Player *player, ShipType type
    Return      : udword
-----------------------------------------------------------------------*/
udword rmAllTechRequredForShip(Player *player, ShipType type)
{
    sdword i;
    udword techneeded, temp;

    temp = player->researchinfo.techstat->TechNeededToBuildShip[type];
    techneeded = temp;

    for (i=0;i<NumTechnologies;i++)
    {
        if (bitTest(temp, TechToBit(i)))
        {
            techneeded |= rmAllTechRequiredforTech(player, i);
        }
    }

    techneeded &= (~player->researchinfo.HasTechnology);

    return (techneeded);
}

/*-----------------------------------------------------------------------------
    Name        : rmTechRequiredForShip
    Description : This function returns the number of research topics required
                  to build the ship specified
    Inputs      : player and shiptype
    Outputs     : number of research topics required
    Return      : sdword
----------------------------------------------------------------------------*/
sdword rmTechRequiredForShip(Player *player, ShipType type)
{
    sdword i, numtech=0;
    udword techneeded, temp;

    temp = player->researchinfo.techstat->TechNeededToBuildShip[type];
    techneeded = temp;

    for (i=0;i<NumTechnologies;i++)
    {
        if (bitTest(temp, TechToBit(i)))
        {
            techneeded |= rmAllTechRequiredforTech(player, i);
        }
    }

    techneeded &= (~player->researchinfo.HasTechnology);

    for (i=0;i<NumTechnologies;i++)
    {
        if (bitTest(techneeded, TechToBit(i)))
        {
            numtech++;
        }
    }

    return (numtech);
}

/*-----------------------------------------------------------------------------
    Name        : rmResearchTechForShip
    Description : This function researches technology required to build a ship.
    Inputs      : player and shiptype
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
bool rmResearchTechForShip(struct Player *player, ShipType type)
{
    udword numtech, techneeded;
    sdword freelab;
    sdword techtoresearch = -1;
    sdword i;
    bool researching = FALSE;

    numtech = rmTechRequiredForShip(player, type);

    if (numtech > 0)
    {
        techneeded = rmAllTechRequredForShip(player, type);

        for (i=0;i<NumTechnologies;i++)
        {
            if (bitTest(techneeded, TechToBit((udword)i)))
            {
                if ((player->researchinfo.techstat->TechNeededToResearch[i]&(~player->researchinfo.HasTechnology)) == 0)
                {
                    techtoresearch = i;
                    freelab = rmFindFreeLab(player);
                    if (freelab != -1)
                    {
                        rmAssignPlayersLabToResearch(player,freelab, techtoresearch);
                        researching = TRUE;
                    }
                }
            }
        }
    }

    if (techtoresearch != -1)
    {
        while ((freelab = rmFindFreeLab(player))!=-1)
        {
            rmAssignPlayersLabToResearch(player,freelab, techtoresearch);
            researching = TRUE;
        }
    }
    return researching;
}

/*-----------------------------------------------------------------------------
    Name        : rmGiveTechToPlayerByName
    Description : Gives player technology 'techName' and all previous technologies
                  needed to get it
    Inputs      : player - player to get tech
                  techName - name of technology as defined in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rmGiveTechToPlayerByName(struct Player *player, char *techName)
{
    rmGiveTechToPlayerByType(player,StrToTechType(techName));
}

/*-----------------------------------------------------------------------------
    Name        : rmGiveTechToPlayerByType
    Description : Gives player technology 'techName' and all previous technologies
                  needed to get it
    Inputs      : player - player to get tech
                  techName - name of technology as defined in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rmGiveTechToPlayerByType(struct Player *player, TechnologyType techtype)
{
    TechnologyType techCanGet;
    PlayerResearchInfo *research;

    research = &player->researchinfo;

    while(1)
    {
        techCanGet = rmTechRequiredForTech(player, techtype);
        if(techCanGet == 0)
        {
            //no more dependant tech to give
            //so give main tech and return;

            research->HasTechnology |= TechToBit(techtype);
            return;
        }
        research->HasTechnology |= TechToBit(techCanGet);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmInitializeResearchStatics
    Description : Reset the research statics for a given player
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rmInitializeResearchStatics(struct Player *player)
{
    if (!singlePlayerGame)
    {
        if (bitTest(tpGameCreated.flag,MG_CarrierIsMothership))
        {
            if (player->race==R1)
            {
                player->researchinfo.techstat   = &MultiPlayerCR1TechStatic;
            }
            if (player->race==R2)
            {
                player->researchinfo.techstat   = &MultiPlayerCR2TechStatic;
            }
        }
        else
        {
            if (player->race==R1)
            {
                player->researchinfo.techstat   = &MultiPlayerR1TechStatic;
            }
            if (player->race==R2)
            {
                player->researchinfo.techstat   = &MultiPlayerR2TechStatic;
            }
        }
    }
    else
    {
        if (player->race==R1)
        {
            player->researchinfo.techstat   = &SinglePlayerR1TechStatic;
        }
        if (player->race==R2)
        {
            player->researchinfo.techstat   = &SinglePlayerR2TechStatic;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmInitializeResearchStruct
    Description : initializes research structure for player
    Inputs      : Player
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmInitializeResearchStruct(Player *player, bool candoresearch, sdword techlevel)
{
    sdword index;

    player->researchinfo.CanDoResearch = candoresearch;

    switch (techlevel)
    {
        case TECH_NOTECHNOLOGY  :
            player->researchinfo.HasTechnology = 0;
        break;
        case TECH_ALLTECHNOLOGY :
            player->researchinfo.HasTechnology = AllTechnology;
        break;
    }

    rmInitializeResearchStatics(player);

    for (index=0;index<NUM_RESEARCHLABS;index++)
    {
        player->researchinfo.researchlabs[index].labstatus = LS_NORESEARCHSHIP;
        player->researchinfo.researchlabs[index].topic     = NULL;
    }

    listInit(&player->researchinfo.listoftopics);
}


/*-----------------------------------------------------------------------------
    Name        : rmAPIStartup
    Description : starts up the API for the research manager loading all dependancies
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmAPIStartup(void)
{
    sdword index;

    for (index = 0; index < NumTechnologies; index++)
    {
        SinglePlayerR1TechStatic.TechNeededToResearch[index] = 0;
        MultiPlayerR1TechStatic.TechNeededToResearch[index] = 0;
        MultiPlayerCR1TechStatic.TechNeededToResearch[index] = 0;
        SinglePlayerR2TechStatic.TechNeededToResearch[index] = 0;
        MultiPlayerR2TechStatic.TechNeededToResearch[index] = 0;
        MultiPlayerCR2TechStatic.TechNeededToResearch[index] = 0;
    }

    for (index = 0; index < STD_LAST_SHIP; index++)
    {
        SinglePlayerR1TechStatic.TechNeededToBuildShip[index] = 0;
        MultiPlayerR1TechStatic.TechNeededToBuildShip[index] = 0;
        MultiPlayerCR1TechStatic.TechNeededToBuildShip[index] = 0;
        SinglePlayerR2TechStatic.TechNeededToBuildShip[index] = 0;
        MultiPlayerR2TechStatic.TechNeededToBuildShip[index] = 0;
        MultiPlayerCR2TechStatic.TechNeededToBuildShip[index] = 0;
    }

#if (defined(CGW) || defined(Downloadable) /*|| defined(OEM)*/)
    scriptSet(NULL,"DemoResearchDepend.script",TechnologyDependancies);
#else
    scriptSet(NULL,"ResearchDepend.script",TechnologyDependancies);
#endif
    rmSetPrintList(RM_SPR1,&SinglePlayerR1TechStatic);
    rmSetPrintList(RM_SPR2,&SinglePlayerR2TechStatic);
    rmSetPrintList(RM_MPR1,&MultiPlayerR1TechStatic);
    rmSetPrintList(RM_MPR2,&MultiPlayerR2TechStatic);
    rmSetPrintList(RM_MPCR1,&MultiPlayerCR1TechStatic);
    rmSetPrintList(RM_MPCR2,&MultiPlayerCR2TechStatic);

    rmGUIStartup();
}


/*-----------------------------------------------------------------------------
    Name        : rmAPIShutdown
    Description : shut down the API for the research manager
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmAPIShutdown(void)
{
    rmGUIShutdown();
}


/*-----------------------------------------------------------------------------
    Name        : rmRemoveUnneededTech
    Description : Removes any technology that when researched gives nothing!
    Inputs      : Pointer to the TechStatics to be modified.
    Outputs     : void
    Return      : none
----------------------------------------------------------------------------*/
void rmRemoveUnneededTech(TechStatics *techstat)
{
    udword i,j,k, tech, techtmp;
    bool   notneeded;

    //

    for (i=0; i < STD_LAST_SHIP; i++)   // for all ships
    {
        tech = techstat->TechNeededToBuildShip[i];
        if (bitTest(tech,RM_Disabled))  // is ship disabled ?
        {
            for (j=0; j < NumTechnologies; j++)     // for all technologies needed to build this ship.
            {
                notneeded = TRUE;

                if (bitTest(tech,TechToBit(j)))
                {
                    for (k=0; k < STD_LAST_SHIP; k ++)
                    {
                        techtmp = techstat->TechNeededToBuildShip[k];
                        if ((!(bitTest(techtmp,RM_Disabled))) &&(k != i))
                        {
                            if (bitTest(techtmp,TechToBit(j)))
                            {
                                notneeded = FALSE;
                            }
                        }
                    }
                    if (notneeded)
                    {
                        techstat->TechNeededToResearch[j] |= RM_Disabled;
                    }
                }
            }
        }
    }
}

void rmRemoveAllUnneededTech(void)
{
    rmRemoveUnneededTech(&MultiPlayerR1TechStatic);
    rmRemoveUnneededTech(&MultiPlayerCR1TechStatic);
    rmRemoveUnneededTech(&SinglePlayerR1TechStatic);
    rmRemoveUnneededTech(&MultiPlayerR2TechStatic);
    rmRemoveUnneededTech(&MultiPlayerCR2TechStatic);
    rmRemoveUnneededTech(&SinglePlayerR2TechStatic);
}

/*-----------------------------------------------------------------------------
    Name        : rmEnableShipFunction
    Description : disables a ship so that a player cannot build it
    Inputs      : race - race of shipt to disable
                  ship - type of ship to disable
                  bEnable - TRUE to enable, FALSE to disable
    Outputs     : none
    Return      : none
----------------------------------------------------------------------------*/
void rmEnableShip(ShipRace race, ShipType ship, bool bEnabled)
{
    udword flag = bEnabled ? 0 : RM_Disabled;

    if (ship <= STD_LAST_SHIP)
    {
        if (race == R1)
        {
            MultiPlayerR1TechStatic.TechNeededToBuildShip[ship]  = (MultiPlayerR1TechStatic.TechNeededToBuildShip[ship] & (~RM_Disabled)) | flag;
            MultiPlayerCR1TechStatic.TechNeededToBuildShip[ship] = (MultiPlayerCR1TechStatic.TechNeededToBuildShip[ship] & (~RM_Disabled)) | flag;
            SinglePlayerR1TechStatic.TechNeededToBuildShip[ship] = (SinglePlayerR1TechStatic.TechNeededToBuildShip[ship] & (~RM_Disabled)) | flag;
        }
        else if (race == R2)
        {
            MultiPlayerR2TechStatic.TechNeededToBuildShip[ship]  = (MultiPlayerR2TechStatic.TechNeededToBuildShip[ship] & (~RM_Disabled)) | flag;
            MultiPlayerCR2TechStatic.TechNeededToBuildShip[ship] = (MultiPlayerCR2TechStatic.TechNeededToBuildShip[ship] & (~RM_Disabled)) | flag;
            SinglePlayerR2TechStatic.TechNeededToBuildShip[ship] = (SinglePlayerR2TechStatic.TechNeededToBuildShip[ship] & (~RM_Disabled)) | flag;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmResetStaticInfo
    Description : resets the static info to what was in the script file.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmResetStaticInfo(void)
{
    sdword index;

    for (index = 0; index < NumTechnologies; index++)
    {
        SinglePlayerR1TechStatic.TechNeededToResearch[index] = 0;
        MultiPlayerR1TechStatic.TechNeededToResearch[index] = 0;
        MultiPlayerCR1TechStatic.TechNeededToResearch[index] = 0;
        SinglePlayerR2TechStatic.TechNeededToResearch[index] = 0;
        MultiPlayerR2TechStatic.TechNeededToResearch[index] = 0;
        MultiPlayerCR2TechStatic.TechNeededToResearch[index] = 0;
    }

    for (index = 0; index < STD_LAST_SHIP; index++)
    {
        SinglePlayerR1TechStatic.TechNeededToBuildShip[index] = 0;
        MultiPlayerR1TechStatic.TechNeededToBuildShip[index] = 0;
        MultiPlayerCR1TechStatic.TechNeededToBuildShip[index] = 0;
        SinglePlayerR2TechStatic.TechNeededToBuildShip[index] = 0;
        MultiPlayerR2TechStatic.TechNeededToBuildShip[index] = 0;
        MultiPlayerCR2TechStatic.TechNeededToBuildShip[index] = 0;
    }

#if (defined(CGW) || defined(Downloadable) /*|| defined(OEM)*/)
    scriptSet(NULL,"DemoResearchDepend.script",TechnologyDependancies);
#else
    scriptSet(NULL,"ResearchDepend.script",TechnologyDependancies);
#endif

    rmSetPrintList(RM_SPR1,&SinglePlayerR1TechStatic);
    rmSetPrintList(RM_SPR2,&SinglePlayerR2TechStatic);
    rmSetPrintList(RM_MPR1,&MultiPlayerR1TechStatic);
    rmSetPrintList(RM_MPR2,&MultiPlayerR2TechStatic);
    rmSetPrintList(RM_MPCR1,&MultiPlayerCR1TechStatic);
    rmSetPrintList(RM_MPCR2,&MultiPlayerCR2TechStatic);
}


/*-----------------------------------------------------------------------------
    Name        : rmSetDependCB
    Description : sets the ship technology dependancies based on a list of strings
    Inputs      : standard callback inputs
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmSetShipDependCB(char *directory, char *field, void *dataToFillIn)
{
    sdword index;
    char temp[64];
    ShipType shiptype;
    TechnologyType techtype;
    udword mask=0;

    for (index=0;*field != ' ';index++,field++)
    {
        temp[index]=*field;
    }
    temp[index]=0;

    while ((*field == ' ') && (*field != 0))
        field++;

    if (*field == 0) return;

    if (*field != 0)
    {
        RemoveCommasFromString(field);

        shiptype = StrToShipType(temp);

        dbgAssert(shiptype!=-1);

        while (*field != 0)
        {
            for (index=0;(*field != ' ')&&(*field != 0);index++,field++)
            {
                temp[index]=*field;
            }
            temp[index]=0;

            techtype = StrToTechType(temp);

            dbgAssert(techtype != -1);

            mask |= TechToBit(techtype);

            while (*field == ' ')
                field++;

        }
    }

    ((udword *)dataToFillIn)[shiptype] = mask;
}

/*-----------------------------------------------------------------------------
    Name        : rmSetTechDependCB
    Description : sets the technology dependancies based on a list of strings
    Inputs      : standard callback inputs
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmSetTechDependCB(char *directory, char *field, void *dataToFillIn)
{
    sdword index;
    char temp[64];
    TechnologyType techtype, techset;
    udword mask=0;

    for (index=0;*field != ' ';index++,field++)
    {
        temp[index]=*field;
    }
    temp[index]=0;

    while ((*field == ' ') && (*field != 0))
        field++;

    if (*field == 0) return;

    if (*field != 0)
    {

        RemoveCommasFromString(field);

        techset = StrToTechType(temp);

        dbgAssert(techset!=-1);

        while (*field != 0)
        {
            for (index=0;(*field != ' ')&&(*field != 0);index++,field++)
            {
                temp[index]=*field;
            }
            temp[index]=0;

            techtype = StrToTechType(temp);

            dbgAssert(techtype != -1);

            mask |= TechToBit(techtype);

            while (*field == ' ')
                field++;

        }
    }

    ((udword *)dataToFillIn)[techset] = mask;
}

