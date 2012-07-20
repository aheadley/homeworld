/*=============================================================================
    Name    : CloakGenerator.c
    Purpose : Specifics for the Cloak Generator

    Created 01/06/1998 by bpasechn
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "SpaceObj.h"
#include "CloakGenerator.h"
#include "SoundEvent.h"
#include "LinkedList.h"
#include "memory.h"
#include "Universe.h"
#include "Vector.h"
#include "SaveGame.h"
#include "GenericInterceptor.h"
#include "SalCapCorvette.h"
#include "Attack.h"
#include "Battle.h"

CloakGeneratorStatics CloakGeneratorStatic;

CloakGeneratorStatics CloakGeneratorStaticRace1;
CloakGeneratorStatics CloakGeneratorStaticRace2;

scriptStructEntry CloakGeneratorStaticScriptTable[] =
{
    { "CloakingRadius",    scriptSetReal32CB, (udword) &(CloakGeneratorStatic.CloakingRadius), (udword) &(CloakGeneratorStatic) },
    { "CloakingTime",    scriptSetReal32CB, (udword) &(CloakGeneratorStatic.CloakingTime), (udword) &(CloakGeneratorStatic) },
    { "DeCloakingTime",    scriptSetReal32CB, (udword) &(CloakGeneratorStatic.DeCloakingTime), (udword) &(CloakGeneratorStatic) },
    { "MaxCloakingTime",    scriptSetReal32CB, (udword) &(CloakGeneratorStatic.MaxCloakingTime), (udword) &(CloakGeneratorStatic) },
    { "ReChargeRate",     scriptSetReal32CB, (udword) &(CloakGeneratorStatic.ReChargeRate), (udword) &(CloakGeneratorStatic) },
    { "MinCharge",     scriptSetReal32CB, (udword) &(CloakGeneratorStatic.MinCharge), (udword) &(CloakGeneratorStatic) },

    { NULL,NULL,0,0 }
};

void CloakGeneratorStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    CloakGeneratorStatics *CloakGeneratorstat = (statinfo->shiprace == R1) ? &CloakGeneratorStaticRace1 : &CloakGeneratorStaticRace2;
    statinfo->custstatinfo = CloakGeneratorstat;

    scriptSetStruct(directory,filename,CloakGeneratorStaticScriptTable,(ubyte *)CloakGeneratorstat);
    CloakGeneratorstat->CloakingRadiusSqr = CloakGeneratorstat->CloakingRadius*CloakGeneratorstat->CloakingRadius;      //Calculate Square of Cloaking Radius
    CloakGeneratorstat->CloakingTime = 1 / CloakGeneratorstat->CloakingTime;
    CloakGeneratorstat->DeCloakingTime = 1 / CloakGeneratorstat->DeCloakingTime;
}

void CloakGeneratorInit(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    CloakGeneratorStatics *cloakgeneratorstatics;
    cloakgeneratorstatics = (CloakGeneratorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    spec->CloakOn = FALSE;
    spec->CloakLowWarning = FALSE;
    spec->CloakStatus = cloakgeneratorstatics->MaxCloakingTime;
    listInit(&spec->CloakList);
}

void CloakGeneratorAddObj(Ship *ship,  SpaceObj *objtoadd)
{                       //Adds shiptoadd to 'ship's cloaklist
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    CloakStruct *newcloakstruct;

    if (bitTest(objtoadd->flags,SOF_Cloaked) || bitTest(objtoadd->flags, SOF_Cloaking) || bitTest(objtoadd->flags, SOF_DeCloaking))
    {               //if object is cloaked, don't add it...don't even think about it.
        return;
    }

    newcloakstruct = memAlloc(sizeof(CloakStruct),"CloakStruct",0);

    newcloakstruct->CloakStatus = 0.0f;
    newcloakstruct->spaceobj = objtoadd;
    listAddNode(&spec->CloakList,&newcloakstruct->cloaknode,newcloakstruct);

    //temporary...later make 'cloaking'
    bitSet(objtoadd->flags, SOF_Cloaking);
    bitSet(objtoadd->flags, SOF_CloakGenField);  //indicate object is in cloak field

    //should not have to clear the cloaked flag
    bitClear(objtoadd->flags, SOF_Cloaked);       //
    bitClear(objtoadd->flags, SOF_DeCloaking);
}
void CloakGeneratorClose(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    listDeleteAll(&spec->CloakList);        //delete the list
}

void CloakGeneratorDied(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    Node *cloaknode;
    CloakStruct *cloakstruct;

    cloaknode = spec->CloakList.head;

    while(cloaknode != NULL)
    {
        cloakstruct = (CloakStruct *)listGetStructOfNode(cloaknode);
        if(bitTest(cloakstruct->spaceobj->flags,SOF_Cloaked))
        {
            bitClear(cloakstruct->spaceobj->flags,SOF_Cloaked);
                          //Force instant decloaking
            if(cloakstruct->spaceobj->objtype == OBJ_ShipType)
            {
                //if object was a ship set objects decloaking time for cloak advagtage effect
                SpawnCloakingEffect((Ship *)cloakstruct->spaceobj, etgSpecialPurposeEffectTable[EGT_CLOAK_OFF]);
                ((Ship *) (cloakstruct->spaceobj))->shipDeCloakTime = universe.totaltimeelapsed;
            }
        }

        bitClear(cloakstruct->spaceobj->flags,SOF_Cloaking);
        bitClear(cloakstruct->spaceobj->flags,SOF_DeCloaking);
        bitClear(cloakstruct->spaceobj->flags,SOF_CloakGenField);
        cloaknode = cloaknode->next;
    }
    listDeleteAll(&spec->CloakList);        //delete the list
}

void CloakGeneratorRemoveShipReferences(Ship *ship,Ship *shiptoremove)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    Node *cloaknode;
    CloakStruct *cloakstruct;
    SpaceObj *objtoremove = (SpaceObj *)shiptoremove;

    cloaknode = spec->CloakList.head;

    if (bitTest(objtoremove->flags,SOF_Cloaked) ||     //if object inquestion isn't cloaked, cloaking
       bitTest(objtoremove->flags,SOF_Cloaking) ||   //or decloaking, then we don't walk this list
       bitTest(objtoremove->flags,SOF_DeCloaking))   //If a cloaked fighter is passed, and it isn't in the list, it will waste time, but this is good enough :)
    {
        while(cloaknode != NULL)
        {
            cloakstruct = (CloakStruct *)listGetStructOfNode(cloaknode);
            if (cloakstruct->spaceobj == objtoremove)
            {
                listDeleteNode(cloaknode);     //delete nodes memory...may not work as I think...ask gary..
                break;             //done so break and return
            }
            cloaknode = cloaknode->next;
        }

        if(bitTest(objtoremove->flags, SOF_Cloaked))
        {
            bitClear(objtoremove->flags,SOF_Cloaked);
            if(objtoremove->objtype == OBJ_ShipType)
            {
                //if object was a ship set objects decloaking time for cloak advagtage effect
                SpawnCloakingEffect((Ship *)objtoremove, etgSpecialPurposeEffectTable[EGT_CLOAK_OFF]);

                ((Ship *) (objtoremove))->shipDeCloakTime = universe.totaltimeelapsed;
            }
        }
    bitClear(objtoremove->flags,SOF_Cloaking);
    bitClear(objtoremove->flags,SOF_DeCloaking);
    bitClear(objtoremove->flags,SOF_CloakGenField);
    }
}

void CloakAddObjectsInProximity(Ship *cloakship)
{
    Player *playerowner = cloakship->playerowner;
    Node *objnode = universe.ShipList.head;
    CloakGeneratorStatics *cloakgeneratorstatics;
    Ship *spaceobj;
    real32 distanceSqr;
    vector diff;

    cloakgeneratorstatics = (CloakGeneratorStatics *) ((ShipStaticInfo *)(cloakship->staticinfo))->custstatinfo;

    while (objnode != NULL)
    {
        spaceobj = (Ship *)listGetStructOfNode(objnode);
        //Rejections...
        dbgAssert(spaceobj->objtype == OBJ_ShipType);

        if (spaceobj->playerowner != playerowner)
        {
            goto nextnode;      // must be players ship inorder to cloak
        }
        if( spaceobj->shiptype == SalCapCorvette)
        {
            if(((SalCapCorvetteSpec *)spaceobj->ShipSpecifics)->tractorBeam)
            {
                goto nextnode;
            }
        }
        if (bitTest(spaceobj->flags,SOF_Cloaked))
        {
            goto nextnode;      // if object is cloaked
        }
        if(bitTest(spaceobj->flags,SOF_Dead))
        {
            goto nextnode;
        }
        if(spaceobj->shiptype == Mothership ||
           spaceobj->shiptype == Carrier)
        {
            goto nextnode;
        }

        //add more rejections...like collision model.
        vecSub(diff,spaceobj->posinfo.position,cloakship->posinfo.position);
        distanceSqr = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;
        if(distanceSqr <= cloakgeneratorstatics->CloakingRadiusSqr)
        {
            CloakGeneratorAddObj(cloakship,  (SpaceObj *) spaceobj);
        }
nextnode:
        objnode = objnode->next;
    }
}

void DeCloakAllObjects(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    Node *cloaknode;
    CloakStruct *cloakstruct;

    cloaknode = spec->CloakList.head;

    while(cloaknode != NULL)
    {
        cloakstruct = (CloakStruct *)listGetStructOfNode(cloaknode);
        bitSet(cloakstruct->spaceobj->flags,SOF_DeCloaking);
        if(bitTest(cloakstruct->spaceobj->flags, SOF_Cloaked))
        {
            bitClear(cloakstruct->spaceobj->flags,SOF_Cloaked);
            if(cloakstruct->spaceobj->objtype == OBJ_ShipType)
            {
                //if object was a ship set objects decloaking time for cloak advagtage effect
                SpawnCloakingEffect((Ship *)cloakstruct->spaceobj, etgSpecialPurposeEffectTable[EGT_CLOAK_OFF]);

                ((Ship *) (cloakstruct->spaceobj))->shipDeCloakTime = universe.totaltimeelapsed;
            }
        }
        bitClear(cloakstruct->spaceobj->flags,SOF_Cloaking);
        cloaknode = cloaknode->next;
    }
}

bool CloakGeneratorSpecialActivate(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    CloakGeneratorStatics *cloakgeneratorstatics;

    if(spec->CloakOn == TRUE)
    {       //Ships cloak field is on...user wants it off...
        DeCloakAllObjects(ship);
        spec->CloakOn = FALSE;                  //Cloak Field goes off
        spec->CloakLowWarning = FALSE;  //reset flag
//        soundEvent(ship, PowerOff);
//        speechEvent(ship, COMM_Cloak_Decloak, 0);
        if (battleCanChatterAtThisTime(BCE_Decloaking, ship))
        {
            battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Decloaking, ship, SOUND_EVENT_DEFAULT);
        }
        if(ship->playerowner->playerIndex !=
           universe.curPlayerIndex)
        {
            ///////////////
            //speech event for enemy cloakgen decloaking
            //event num: COMM_F_Cloakgen_Decloaking
            //battle chatter...camera distance important!
            //if (battleCanChatterAtThisTime(BCE_COMM_F_Cloakgen_Decloaking, ship))
            //{
            //    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_COMM_F_Cloakgen_Decloaking, ship, SOUND_EVENT_DEFAULT);
            //}

            ///
        }
    }
    else
    {       //Ships cloakfield is off, user wants it on
        cloakgeneratorstatics = (CloakGeneratorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;
        if(spec->CloakStatus >= cloakgeneratorstatics->MinCharge)
        {       //Minimum power is available
            spec->CloakOn = TRUE;                   //Cloak Field goes on
//            soundEvent(ship, PowerOn);
//            speechEvent(ship, COMM_Cloak_CloakingOn, 0);
            if (battleCanChatterAtThisTime(BCE_CloakingOn, ship))
            {
                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CloakingOn, ship, SOUND_EVENT_DEFAULT);
            }
        }
        else
        {
//            speechEvent(ship, COMM_Cloak_InsufficientPower, 0);
            if (battleCanChatterAtThisTime(BCE_CloakingInsufficientPower, ship))
            {
                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CloakingInsufficientPower, ship, SOUND_EVENT_DEFAULT);
            }
        }
    }

    return TRUE;
}

void CloakGeneratorJustDisabled(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;

    if(spec->CloakOn == TRUE)
    {
        CloakGeneratorSpecialActivate(ship);
    }
}

void CloakGeneratorHouseKeep(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    CloakGeneratorStatics *cloakgeneratorstatics;
    Node *cloaknode, *temp;
    SpaceObj *spaceobj;
    CloakStruct *cloakstruct;

    vector diff;
    real32  distanceSqr;

    cloakgeneratorstatics = (CloakGeneratorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    if(spec->CloakOn == TRUE)       //Cloaking Generator is On
    {
        //15th frame..so do a search on other ships within 'proximity'
        if ((universe.univUpdateCounter & CLOAKGENERATOR_CLOAKCHECK_RATE) == (ship->shipID.shipNumber & CLOAKGENERATOR_CLOAKCHECK_RATE))
        {
            CloakAddObjectsInProximity(ship);   //this opp is slow!  So maybe decrease frequency
        }

        //decrement functionality time
        spec->CloakStatus -= universe.phystimeelapsed;

        if ((spec->CloakStatus <= 10.0f) && !spec->CloakLowWarning)
        {
            spec->CloakLowWarning = TRUE;
//            speechEvent(ship, STAT_Cloak_CloakPowerLow, (sdword)spec->CloakStatus);
            if (battleCanChatterAtThisTime(BCE_CloakingPowerLow, ship))
            {
                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CloakingPowerLow, ship, (sdword)spec->CloakStatus);
            }
        }

        //Cloak Field has been on too long
        if(spec->CloakStatus <= 0.0f)
        {
            DeCloakAllObjects(ship);  //decloak everything
            spec->CloakOn = FALSE;    //reset flags
            spec->CloakStatus = 0.0f; //reset CloakStatus
            spec->CloakLowWarning = FALSE;  //reset flag
//            soundEvent(ship, PowerOff);
//            speechEvent(ship, COMM_Cloak_Decloak, 0);
            if (battleCanChatterAtThisTime(BCE_Decloaking, ship))
            {
                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Decloaking, ship, SOUND_EVENT_DEFAULT);
            }
        }
    }
    else        //cloak field is off
    {
        //recharge cloaking juice at tunable rate
        spec->CloakStatus +=  universe.phystimeelapsed*cloakgeneratorstatics->ReChargeRate;
        if(spec->CloakStatus >= cloakgeneratorstatics->MaxCloakingTime)
        {
            spec->CloakStatus = cloakgeneratorstatics->MaxCloakingTime; //cap max cloaking juice
        }
    }

    //perform maintainence on cloaked ships (cloaking,decloaking and continual cloak)
    cloaknode = spec->CloakList.head;
    while(cloaknode != NULL)
    {
        cloakstruct = (CloakStruct *)listGetStructOfNode(cloaknode);
        spaceobj = cloakstruct->spaceobj;

        // wierd crash ...

		if (spaceobj==NULL) 
		{
			temp = cloaknode;
			cloaknode = cloaknode->next;

			listDeleteNode(temp);
			continue;
		}
		
		//if spaceobj is dead for some reason, remove its references and stop bothering with it
        if (bitTest(spaceobj->flags,SOF_Dead))
        {   //dead so get rid of quickly...
            cloaknode=cloaknode->next;                  //we're about to delete this node so move to next
            CloakGeneratorRemoveShipReferences(ship,(Ship *)spaceobj);  //delete everything about it
            continue;
        }

        //perform a change check!
        //this seems like a waste because we are AGAIN calculating distances.  Do better later!
        if ((universe.univUpdateCounter & CLOAKGENERATOR_CLOAKCHECK_RATE) == (ship->shipID.shipNumber & CLOAKGENERATOR_CLOAKCHECK_RATE))
        {
            vecSub(diff,spaceobj->posinfo.position,ship->posinfo.position);
            distanceSqr = vecMagnitudeSquared(diff);

            if(distanceSqr > cloakgeneratorstatics->CloakingRadiusSqr)     //maybe add fuzzy logic so thing doesn't always pop in and out in and out in and out...
            {
                if(bitTest(spaceobj->flags,SOF_Cloaked))
                {
                    bitClear(spaceobj->flags,SOF_Cloaked);
                    if(spaceobj->objtype == OBJ_ShipType)
                    {
                        //if object was a ship set objects decloaking time for cloak advagtage effect
                        SpawnCloakingEffect((Ship *)spaceobj, etgSpecialPurposeEffectTable[EGT_CLOAK_OFF]);

                        ((Ship *) (spaceobj))->shipDeCloakTime = universe.totaltimeelapsed;
                    }
                }
                bitClear(spaceobj->flags,SOF_Cloaking);
                bitSet(spaceobj->flags,SOF_DeCloaking);
            }       //potential problem..ship flys into field..flys out..
                    //starts to decloak...flys back in, ship will decloak
                    //fully before recloaking...don't worry about since
                    //cloak times too fast too care
        }

        if(spaceobj->objtype == OBJ_ShipType)
        {
            if( ((Ship *)spaceobj)->shiptype == SalCapCorvette)
            {
                if(((SalCapCorvetteSpec *)((Ship *)spaceobj)->ShipSpecifics)->tractorBeam)
                {
                    //attached so decloak
                    if(bitTest(spaceobj->flags,SOF_Cloaked))
                    {
                        bitClear(spaceobj->flags,SOF_Cloaked);
                        if(spaceobj->objtype == OBJ_ShipType)
                        {
                            //if object was a ship set objects decloaking time for cloak advagtage effect
                            SpawnCloakingEffect((Ship *)spaceobj, etgSpecialPurposeEffectTable[EGT_CLOAK_OFF]);

                            ((Ship *) (spaceobj))->shipDeCloakTime = universe.totaltimeelapsed;
                        }
                    }
                    bitClear(spaceobj->flags,SOF_Cloaking);
                    bitSet(spaceobj->flags,SOF_DeCloaking);
                }
            }
        }

        //Case 1, object is cloaking
        if(bitTest(spaceobj->flags,SOF_Cloaking))
        {
            //object is cloaking..so keep cloaking it
/***** calculated the inverse of CloakingTime in CloakGeneratorStaticInit and multiply instead of divide *****/
            cloakstruct->CloakStatus += universe.phystimeelapsed*cloakgeneratorstatics->CloakingTime;
            if(cloakstruct->CloakStatus >= 1.0f)
            {
                //object is now cloaked...so stop cloaking it and make it fully 'cloaked'
                cloakstruct->CloakStatus = 1.0f;          //set for assurance
                bitSet(spaceobj->flags,SOF_Cloaked);      //make cloaked
                bitClear(spaceobj->flags,SOF_Cloaking);   //stop from cloaking
                if(spaceobj->objtype == OBJ_ShipType)
                {
                    shipHasJustCloaked((Ship *)spaceobj);
                    SpawnCloakingEffect((Ship *)spaceobj, etgSpecialPurposeEffectTable[EGT_CLOAK_ON]);
                    //RemoveShipFromBeingTargeted(&universe.mainCommandLayer,(Ship *)spaceobj,FALSE);
                }
                else if(spaceobj->objtype == OBJ_AsteroidType)
                {
                    //need to remove the asteroid from resource collectors
                    dbgMessagef("\nMake Bryce remove asteroids from game info since you just cloaked one.");
                }
            }
        }
        else if(bitTest(spaceobj->flags,SOF_DeCloaking))
        {
            //object is decloaking, so keep decloaking it
/***** calculated the inverse of DeCloakingTime in CloakGeneratorStaticInit and multiply instead of divide *****/
            cloakstruct->CloakStatus -= universe.phystimeelapsed*cloakgeneratorstatics->DeCloakingTime;
            if(cloakstruct->CloakStatus <= 0.0f)
            {
                //Object is now fully decloaked so stop decloaking it and remove it from the list
                cloaknode=cloaknode->next;                  //we're about to delete this node so move to next
                CloakGeneratorRemoveShipReferences(ship,(Ship *)spaceobj);  //delete everything about it
                continue;       //we now continue the loop rather than letting the other code continue :)
            }
        }
        cloaknode = cloaknode->next;
    }
}

void CloakGeneratorAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    attackStraightForward(ship,target,4000.0f,3000.0f);
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void SaveCloakStruct(CloakStruct *cloakStruct)
{
    SaveChunk *chunk;
    CloakStruct *savecontents;

    chunk = CreateChunk(BASIC_STRUCTURE,sizeof(CloakStruct),cloakStruct);
    savecontents = (CloakStruct *)chunkContents(chunk);

    savecontents->spaceobj = (SpaceObj *)SpaceObjRegistryGetID(savecontents->spaceobj);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void CloakGenerator_Save(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    Node *node = spec->CloakList.head;
    sdword cur = 0;

    SaveInfoNumber(spec->CloakList.num);

    while (node != NULL)
    {
        cur++;
        SaveCloakStruct((CloakStruct *)listGetStructOfNode(node));
        node = node->next;
    }

    dbgAssert(cur == spec->CloakList.num);
}

CloakStruct *LoadCloakStruct(void)
{
    SaveChunk *chunk;
    CloakStruct *cloakStruct;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE,sizeof(CloakStruct));

    cloakStruct = memAlloc(sizeof(CloakStruct),"CloakStruct",0);
    memcpy(cloakStruct,chunkContents(chunk),sizeof(CloakStruct));

    memFree(chunk);

    return cloakStruct;
}

void CloakGenerator_Load(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    sdword num;
    sdword i;
    CloakStruct *cloakStruct;

    num = LoadInfoNumber();

    listInit(&spec->CloakList);

    for (i=0;i<num;i++)
    {
        cloakStruct = LoadCloakStruct();
        listAddNode(&spec->CloakList,&cloakStruct->cloaknode,cloakStruct);
    }
}

void CloakGenerator_Fix(Ship *ship)
{
    CloakGeneratorSpec *spec = (CloakGeneratorSpec *)ship->ShipSpecifics;
    Node *node = spec->CloakList.head;
    CloakStruct *cloakStruct;

    while (node != NULL)
    {
        cloakStruct = (CloakStruct *)listGetStructOfNode(node);

        cloakStruct->spaceobj = SpaceObjRegistryGetObj((sdword)cloakStruct->spaceobj);

        node = node->next;
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

CustShipHeader CloakGeneratorHeader =
{
    CloakGenerator,
    sizeof(CloakGeneratorSpec),
    CloakGeneratorStaticInit,
    NULL,
    CloakGeneratorInit,
    CloakGeneratorClose,
    CloakGeneratorAttack,
    NULL,
    NULL,
    CloakGeneratorSpecialActivate,
    NULL,
    CloakGeneratorHouseKeep,
    CloakGeneratorRemoveShipReferences,
    CloakGeneratorDied,
    NULL,
    CloakGenerator_Save,
    CloakGenerator_Load,
    CloakGenerator_Fix
};

