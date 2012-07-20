/*=============================================================================
    Name    : crates.c
    Purpose : contains all code for all crate gameplay

    Created bryce in november
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>

#include "Types.h"
#include "SpaceObj.h"
#include "UnivUpdate.h"
#include "Universe.h"
#include "Blobs.h"
#include "AITrack.h"
#include "LinkedList.h"
#include "ResearchAPI.h"
#include "MultiplayerGame.h"
#include "LevelLoad.h"
#include "Randy.h"
#include "SoundEvent.h"
#include "Sensors.h"
#include "PiePlate.h"


real32 CRATES_DoWeNeedToAddCratesCheckTime =380.0f;  //every X seconds, the crate functions will check to see if it should add a crate to the world

sdword CRATES_ArePlayersNearUpdateMask= 63;  //bitmask to compare with univupdatecounter to see if players are near the crate
sdword CRATES_ArePlayersNearUpdateFrame=52;  //frame to check must be <= CRATES_ArePlayersNearUpdateMask
real32 CRATES_PlayerNearRadius = 2500.0f;     //distance a player needs to get to a crate in order to 'get' the crate

real32 CRATES_MaximumCratesInTheWorldPerPlayer =1;  //don't a   //probability that you get a ship as opposed to something else
real32 CRATES_GetResearch                          = 0.25f;

real32 CRATES_RandomLowDistance  =  7000.0f;
real32 CRATES_RandomHighDistance =  20000.0f;

real32 CRATES_AddACrateProb   =  0.15f;         //probability that we add a crate

sdword CRATES_MAX_RUS_GIVEN =   10000;
sdword CRATES_MIN_RUS_GIVEN =   500 ;

real32 CRATES_GetMotherShipCarrierIfDontHaveOne  = 0.80f;   //probability that you get a carrier/mothership if you didn't have one
real32 CRATES_GetAShip                         =    0.50f;//research probability
//defaults
real32 CrateClassProbCLASS_HeavyCruiser = 0.99f;
real32 CrateClassProbCLASS_Carrier = 0.97f;
real32 CrateClassProbCLASS_Destroyer = 0.90f;
real32 CrateClassProbCLASS_Frigate = 0.75f;
real32 CrateClassProbCLASS_Corvette = 0.45f;
real32 CrateClassProbCLASS_Fighter = 0.10f;
real32 CrateClassProbCLASS_Resource = 0.5f;
real32 CrateClassProbCLASS_NonCombat = 0.0f;
sdword NUM_IN_GROUPS[StandardFrigate+1];  //number of ships to be placed in agroup
real32 SHIP_PROBS[StandardFrigate+1];
real32 CRATE_EXPIRY_TIME = 300.0;

///RU's will be given instead
scriptEntry cratesScriptTable[] =
{
    makeEntry(CRATES_DoWeNeedToAddCratesCheckTime,scriptSetReal32CB),
    makeEntry(CRATES_ArePlayersNearUpdateMask,scriptSetSdwordCB),
    makeEntry(CRATES_ArePlayersNearUpdateFrame,scriptSetSdwordCB),
    makeEntry(CRATES_PlayerNearRadius,scriptSetReal32CB),
    makeEntry(CRATES_MaximumCratesInTheWorldPerPlayer,scriptSetReal32CB),
    makeEntry(CRATES_GetResearch,scriptSetReal32CB),
    makeEntry(CRATES_RandomLowDistance,scriptSetReal32CB),
    makeEntry(CRATES_RandomHighDistance,scriptSetReal32CB),
    makeEntry(CRATES_AddACrateProb,scriptSetReal32CB),
    makeEntry(CRATES_MAX_RUS_GIVEN,scriptSetSdwordCB),
    makeEntry(CRATES_MIN_RUS_GIVEN,scriptSetSdwordCB),
    makeEntry(CRATES_GetMotherShipCarrierIfDontHaveOne,scriptSetReal32CB),
    makeEntry(CRATES_GetAShip,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_HeavyCruiser,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Carrier,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Destroyer,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Frigate,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Corvette,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Fighter,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Resource,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_NonCombat,scriptSetReal32CB),
    makeEntry(CRATE_EXPIRY_TIME,scriptSetReal32CB),


    {"shipProbability", scriptSetShipProbCB, SHIP_PROBS },
    {"shipGroupSize", scriptSetShipGroupSizeCB, NUM_IN_GROUPS },


    endEntry
};

//crate reward types
#define CRATE_SHIP  0
#define CRATE_MONEY 1
#define CRATE_RESEARCH 2

void cratesPlaceCrateSelectivly();
void cratesReportCratePlacement(Derelict *crate);
void cratesPlayerFoundACrate(Derelict *crate, Player *player);
void cratesGetBestCrateLocation(vector *location);
sdword cratesRewardPlayer(Derelict *crate, Player *player);
void expireCrate(Derelict *crate);


void cratesScanForPlayersNearCrateAndReward(Derelict *crate)
{
    blob *crateBlob;
    sdword i;
    Ship *ship;

    crateBlob = crate->collMyBlob;

    if(crateBlob != NULL)
    {
        for(i=0;i<crateBlob->blobShips->numShips;)
        {
            ship = crateBlob->blobShips->ShipPtr[i];
            if(MoveReachedDestinationVariable(ship,&crate->posinfo.position,CRATES_PlayerNearRadius))
            {
                if(ship->playerowner != NULL)
                {
                    cratesPlayerFoundACrate(crate,ship->playerowner);
                    break;
                }
            }
            i++;    //move to next ship
        }
    }
}

void cratesPlaceCrateSelectivly()
{
    vector location;
    Derelict *crate;
    //etglod *etgLOD;
    //etgeffectstatic *stat;


    cratesGetBestCrateLocation(&location);

    crate = univAddDerelict(Crate,&location);   //calls reporting code!
    //cratesReportCratePlacement(crate);
    //play etg effect for placement
    //play sound event?
}

void cratesReportCratePlacement(Derelict *crate)
{
    etglod *etgLOD;
    etgeffectstatic *stat;
    vector zero = {0.0f,0.0f,0.0f};
    matrix ident = IdentityMatrix;

    universe.numCratesInWorld++;

    if(gameIsRunning)
    {
        etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_GENERATED];

        if (etgLOD != NULL)
        {
            if (crate->currentLOD >= etgLOD->nLevels)
            {
                stat = NULL;
            }
            else
            {
                stat = etgLOD->level[crate->currentLOD];
            }
        }
        else
        {
            stat = NULL;
        }
        #if ETG_DISABLEABLE
        if (stat != NULL && etgEffectsEnabled)
        #else
        if (stat != NULL)
        #endif
        {
            etgEffectCreate(stat, NULL, &crate->posinfo.position, &zero, &ident, 1.0f, EAF_AllButNLips, 0);
        }
    }
    dbgAssert(universe.numCratesInWorld <= CRATES_MaximumCratesInTheWorldPerPlayer*MAX_MULTIPLAYER_PLAYERS);

}

void cratesPlayerFoundACrate(Derelict *crate, Player *player)
{
    etglod *etgLOD;
    etgeffectstatic *stat;
    vector zero = {0.0f,0.0f,0.0f};
    matrix ident = IdentityMatrix;
    sdword type;

    universe.numCratesInWorld--;

    type = cratesRewardPlayer(crate,player);

    // only play sound for this player
    if(player->playerIndex == universe.curPlayerIndex)
    {
        soundEvent(NULL, UI_CrateFound);
    }
    /* mabe there should be a different sound for each type of crate */

    switch(type)
    {
    case CRATE_SHIP:
        etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_IS_FOUND_SHIP];
        if(player->playerIndex == universe.curPlayerIndex)
        {
            speechEventFleet(STAT_F_CrateFoundShips,0,player->playerIndex);
        }
        break;
    case CRATE_MONEY:
        etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_IS_FOUND_MONEY];
        if(player->playerIndex == universe.curPlayerIndex)
        {
            speechEventFleet(STAT_F_CrateFoundResources,0,player->playerIndex);
        }
        break;
    case CRATE_RESEARCH:
        etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_IS_FOUND_RESEARCH];
        if(player->playerIndex == universe.curPlayerIndex)
        {
            speechEventFleet(STAT_F_CrateFoundTech,0,player->playerIndex);
        }
        break;
    default:
        dbgFatalf(DBG_Loc,"\nUnknown crate contents!");
        break;
    }

    if (etgLOD != NULL)
    {
        if (crate->currentLOD >= etgLOD->nLevels)
        {
            stat = NULL;
        }
        else
        {
            stat = etgLOD->level[crate->currentLOD];
        }
    }
    else
    {
        stat = NULL;
    }
    #if ETG_DISABLEABLE
    if (stat != NULL && etgEffectsEnabled)
    #else
    if (stat != NULL)
    #endif
    {
        etgEffectCreate(stat, NULL, &crate->posinfo.position, &zero, &ident, 1.0f, EAF_AllButNLips, 0);
    }

    AddTargetToDeleteList((SpaceObjRotImpTarg *)crate,-1);
    }

//maximum number of ships in anyone class
#define MAX_NUM_IN_CLASS 10
sdword cratesCLASS_HeavyCruiser[] =
{
    HeavyCruiser
};
sdword cratesCLASS_Carrier[] =
{
    Carrier
};
sdword cratesCLASS_Destroyer[] =
{
    MissileDestroyer,
    StandardDestroyer
};
sdword cratesCLASS_Frigate[] =
{
    AdvanceSupportFrigate,
    DFGFrigate,
    DDDFrigate,
    CloakGenerator,
    GravWellGenerator,
    IonCannonFrigate,
    StandardFrigate
};
sdword cratesCLASS_Corvette[] =
{
    RepairCorvette,
    SalCapCorvette,
    MultiGunCorvette,
    MinelayerCorvette,
    LightCorvette,
    HeavyCorvette
};
sdword cratesCLASS_Fighter[] =
{
    LightInterceptor,
    LightDefender,
    HeavyInterceptor,
    HeavyDefender,
    DefenseFighter,
    CloakedFighter,
    AttackBomber
};

sdword cratesCLASS_Resource[] =
{
    ResourceCollector,
    ResourceController
};
sdword cratesCLASS_NonCombat[] =
{
    SensorArray,
    Probe,
    ProximitySensor
};

sdword NUM_IN_CLASSES[NUM_CLASSES];



sdword getShipClass()
{
    real32 prob;
    prob = frandombetween(0.0,1.0f);

    if(prob <= CrateClassProbCLASS_HeavyCruiser)
        return CLASS_HeavyCruiser;
    else if(prob <= CrateClassProbCLASS_Carrier)
        return CLASS_Carrier;
    else if(prob <= CrateClassProbCLASS_Destroyer)
        return CLASS_Destroyer;
    else if(prob <= CrateClassProbCLASS_Frigate)
        return CLASS_Frigate;
    else if(prob <= CrateClassProbCLASS_Corvette)
        return CLASS_Corvette;
    else if(prob <= CrateClassProbCLASS_Fighter)
        return CLASS_Fighter;
    else if(prob <= CrateClassProbCLASS_Resource)
        return CLASS_Resource;

    //default
    return CLASS_NonCombat;
}

ShipType gettype(sdword classtype,sdword index)
{
    if(classtype == CLASS_HeavyCruiser)
        return(cratesCLASS_HeavyCruiser[index]);
    else if(classtype == CLASS_Carrier)
        return(cratesCLASS_Carrier[index]);
    else if(classtype == CLASS_Destroyer)
        return(cratesCLASS_Destroyer[index]);
    else if(classtype == CLASS_Frigate)
        return(cratesCLASS_Frigate[index]);
    else if(classtype == CLASS_Corvette)
        return(cratesCLASS_Corvette[index]);
    else if(classtype == CLASS_Fighter)
        return(cratesCLASS_Fighter[index]);
    else if(classtype == CLASS_Resource)
        return(cratesCLASS_Resource[index]);

    return(cratesCLASS_NonCombat[index]);
}
ShipType getShipType(ShipClass classtype)
{
    sdword i;
    real32 prob,probcurrent,probtest;
    ShipType shipcurrent,shiptype;

    prob = frandombetween(0.0,1.0f);
    probcurrent = 1.0f;
    shipcurrent = 9999;

    for(i=0;i<NUM_IN_CLASSES[classtype];i++)
    {
        shiptype = gettype(classtype,i);
        if(prob <= SHIP_PROBS[shiptype])
        {
            probtest = SHIP_PROBS[shiptype]-prob;
            if(probtest <= probcurrent)
            {
                probcurrent = probtest;
                shipcurrent = shiptype;
            }
        }
    }
    dbgAssert(shipcurrent != 9999); //should have selected SOME ship from list!
    return shipcurrent;
}
sdword cratesRewardPlayer(Derelict *crate, Player *player)
{
    real32 prob;
    sdword rus;
    sdword shipclasstoget,shipnumber;
    sdword i;
    Ship *ship;
    ShipStaticInfo *teststatic;

    if(player->PlayerMothership == NULL)
    {
        prob = frandombetween(0.0f, 1.0f);
        if(prob < CRATES_GetMotherShipCarrierIfDontHaveOne)
        {
            dbgMessagef("\nGot a mothership/carrier from crate!");
            //give player mothership
            ship = univAddShip(Carrier,player->race,&crate->posinfo.position,player,0);
            gameStatsAddShip(ship,player->playerIndex);
            unitCapCreateShip(ship,player);
            return(CRATE_SHIP);
        }
    }

    prob = frandombetween(0.0f, 1.0f);
    if(prob <= CRATES_GetAShip)
    {
        sdword shiptype;
        vector cratepos;

        cratepos.x  = crate->posinfo.position.x;
        cratepos.y  = crate->posinfo.position.y;
        cratepos.z  = crate->posinfo.position.z;

        shipclasstoget = getShipClass();
        shiptype = getShipType(shipclasstoget);
        shiptype = GetAppropriateShipTypeForRace(shiptype,player->race);

        shipnumber = NUM_IN_GROUPS[shiptype];

        teststatic = GetShipStaticInfo(shiptype,player->race);

        if (bitTest(teststatic->staticheader.infoFlags, IF_InfoLoaded))
        {
            sdword sizeofselect = sizeofSelectCommand(shipnumber);
            SelectCommand *selectcom = memAlloc(sizeofselect,"selectform",0);
            selectcom->numShips = shipnumber;

            dbgMessagef("\nGot a %d ship type  from crate!",shiptype);

            //could go over unit cap limit here...
            for (i=0;i<shipnumber;i++)
            {
                ship = univAddShip(shiptype,player->race,&cratepos,player,0);
                gameStatsAddShip(ship,player->playerIndex);
                // add the "free ships" value to the initial starting value
                universe.players[ship->playerowner->playerIndex].initialShipCost += ship->staticinfo->buildCost;

                                // Update ship totals for player
                unitCapCreateShip(ship,player);

                selectcom->ShipPtr[i] = ship;

                cratepos.z += (ship->staticinfo->staticheader.staticCollInfo.collspheresize * 2.0f);
            }
            //need a way to FORCE into formation INSTANTLY...could do calculations myself here...
            //but must be an easier way.
            clFormation(&universe.mainCommandLayer,selectcom,BROAD_FORMATION);
            memFree(selectcom);
            return(CRATE_SHIP);
        }
    }

    prob = frandombetween(0.0f, 1.0f);
    if(prob <= CRATES_GetResearch)
    {
        //give research
        uqword mask;
        dbgMessagef("\nGot research from crate");

        for(i=0;i<NumTechnologies;i++)
        {
            mask = ((uqword) 1) << ((uqword) i);
            if(!bitTest(player->researchinfo.HasTechnology, mask))
            {
                rmGiveTechToPlayerByType(player,i);
                return(CRATE_RESEARCH);
            }
        }
        //no tech to give!
    }


    rus = randombetween(CRATES_MIN_RUS_GIVEN,CRATES_MAX_RUS_GIVEN);
    dbgMessagef("\nGot %d RU's from crate.",rus);

    player->resourceUnits += rus;
    universe.gameStats.playerStats[player->playerIndex].totalResourceUnitsCollected += rus;
    universe.gameStats.playerStats[player->playerIndex].totalResourceUnits += rus;
    return(CRATE_MONEY);
}

void getPlayerShipCenter(vector *shipCenter,sdword index)
{
    Node *node=universe.ShipList.head;
    Ship *ship;
    shipCenter->x =0.0f;
    shipCenter->y =0.0f;
    shipCenter->z =0.0f;
    while(node!=NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if(ship->playerowner != NULL)
        {
            if(ship->playerowner->playerIndex == index)
            {
                shipCenter->x = ship->posinfo.position.x;
                shipCenter->y = ship->posinfo.position.y;
                shipCenter->z = ship->posinfo.position.z;
                return;
            }
        }

        node = node->next;
    }

}

sdword cratesIsLocationGood(vector *location)
{
    Node *node = universe.collBlobList.head;
    blob *curblob;
    real32 blobradiusSqr,seperationSqr;
    vector seperation;

    while(node != NULL)
    {
        curblob = (blob *) listGetStructOfNode(node);
        blobradiusSqr = curblob->radius*curblob->radius;
        vecSub(seperation,*location,curblob->centre);
        seperationSqr = vecMagnitudeSquared(seperation);
        if(seperationSqr < blobradiusSqr)
        {
            sdword i;
            //in blob..need to do detecting
            for(i=0;i<curblob->blobShips->numShips;i++)
            {
                vecSub(seperation,*location,curblob->blobShips->ShipPtr[i]->collInfo.collPosition);
                if(vecMagnitudeSquared(seperation) < curblob->blobShips->ShipPtr[i]->staticinfo->staticheader.staticCollInfo.collspheresizeSqr)
                {
                    //WITHIN collision radius of ship
                    return FALSE;
                }
                //fine...so continue
            }
        }

        node = node->next;
    }

    return TRUE;

}
void cratesGetBestCrateLocation(vector *location)
{
    vector destinationVectors[MAX_MULTIPLAYER_PLAYERS];
    vector centerMotherShips = {0.0f,0.0f,0.0f};
    vector mothershipcenter;
    vector displacement = {0.0f,1000.0f,0.0f};
    sdword counted=0;

    sdword i;

    for(i=0;i<universe.numPlayers;i++)
    {
        //could store a players biggest ship here..so as to help them get
        //the crate
        if(universe.players[i].playerState == PLAYER_DEAD)
            continue;

        counted++;
        if(universe.players[i].PlayerMothership == NULL)
        {
            vector shipCenter;
            getPlayerShipCenter(&shipCenter,i);
            vecAddTo(centerMotherShips,shipCenter);
        }
        else
        {
            vecAddTo(centerMotherShips,universe.players[i].PlayerMothership->posinfo.position);
        }
    }
    //get average
    vecScalarMultiply(centerMotherShips,centerMotherShips,1.0f/counted);

    for(i=0;i<universe.numPlayers;i++)
    {
        if(universe.players[i].playerState == PLAYER_DEAD)
            continue;
        if(universe.players[i].PlayerMothership == NULL)
        {
            getPlayerShipCenter(&mothershipcenter,i);
        }
        else
        {
            mothershipcenter = universe.players[i].PlayerMothership->posinfo.position;
        }

        vecSub(destinationVectors[i],mothershipcenter,centerMotherShips);
        vecNormalize(&destinationVectors[i]);
    }

    for(i=0;i<universe.numPlayers;i++)
    {
        if(universe.players[i].playerState == PLAYER_DEAD)
            continue;
        vecScalarMultiply(destinationVectors[i],destinationVectors[i],1.0f/(max(universe.players[i].bounty,1.0f)));
    }




    location->x = 0.0f;
    location->y = 0.0f;
    location->z = 0.0f;

    //set starting vector to favor direction that weaker players are in
    for(i=0;i<universe.numPlayers;i++)
    {
        if(universe.players[i].playerState == PLAYER_DEAD)
            continue;
        vecAddTo(*location,destinationVectors[i]);
    }

    vecNormalize(location);
    location->x *= frandombetween(CRATES_RandomLowDistance,CRATES_RandomHighDistance);
    location->y *= frandombetween(CRATES_RandomLowDistance,CRATES_RandomHighDistance);
    location->z *= frandombetween(CRATES_RandomLowDistance,CRATES_RandomHighDistance);

    //add a large random element to location
    vecAddTo(*location,centerMotherShips);
    location->x += frandombetween(-smUniverseSizeX/3,smUniverseSizeX/3);
    location->y += frandombetween(-smUniverseSizeY/3,smUniverseSizeY/3);
    location->z += frandombetween(-smUniverseSizeZ/3,smUniverseSizeZ/3);

    //add a displacement offset to the location until
    //the location is valid

    //add a bit of randomness to the displacement vector
    displacement.x += frandombetween(-1000.0f,1000.0f);
    displacement.y += frandombetween(-1000.0f,1000.0f);
    displacement.z += frandombetween(-1000.0f,1000.0f);

    while (!cratesIsLocationGood(location))
    {
        vecAddTo(*location,displacement);
        //place check not to exceed game world edge?
        //wrap ..if hit edge...alter displacement value
    }

    pieMovePointClipToLimits(smUniverseSizeX,smUniverseSizeY,smUniverseSizeZ,
                              &centerMotherShips, location);

}



void cratesUpdate()
{
    Derelict *crate;
    Node *node;
    sdword numChecked;

    //only allow crates for multiplayer games
    if(singlePlayerGame)
        return;

    if (gatherStats|showStatsFight|showStatsFancyFight)
        return;

    if((tpGameCreated.flag & MG_CratesEnabled) == 0)
        return;

    universe.crateTimer+=universe.phystimeelapsed;
    if(universe.crateTimer >= CRATES_DoWeNeedToAddCratesCheckTime)
    {
        if (universe.numCratesInWorld < (CRATES_MaximumCratesInTheWorldPerPlayer*universe.numPlayers))
        {
            //time to do a crate addition check
            real32 prob;universe.crateTimer = 0.0f;
            prob = frandombetween(0.0f,1.0f);
            if(prob > CRATES_AddACrateProb)
            {
                cratesPlaceCrateSelectivly();
            }
        }
    }

    if( (universe.univUpdateCounter & CRATES_ArePlayersNearUpdateMask) == CRATES_ArePlayersNearUpdateFrame)
    {
        //time to do a crate nearity check ;)

        if(universe.numCratesInWorld > 0)
        {
            //lets do the distance checking for all crates;
            node = universe.DerelictList.head;
            numChecked = 0;
            while(node != NULL && numChecked < universe.numCratesInWorld)
            {
                crate = (Derelict *) listGetStructOfNode(node);

                if(crate->derelicttype == Crate)
                {
                    numChecked++;

                    //do time expiry checked here...
                    if((universe.totaltimeelapsed - crate->creationTime) > CRATE_EXPIRY_TIME)
                    {
                        expireCrate(crate);
                    }
                    else
                    {
                        cratesScanForPlayersNearCrateAndReward(crate);
                    }
                }

                node = node->next;
            }

        }
    }
}
void expireCrate(Derelict *crate)
{
    etglod *etgLOD;
    etgeffectstatic *stat;
    vector zero = {0.0f,0.0f,0.0f};
    matrix ident = IdentityMatrix;

    universe.numCratesInWorld--;
#ifdef HW_DEBUG
    //don't drop below 0!
    dbgAssert(universe.numCratesInWorld >= 0);
#endif

    //delete target
    AddTargetToDeleteList((SpaceObjRotImpTarg *)crate,-1);

    //play effect
    etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_TIME_OUT];

    if (etgLOD != NULL)
    {
        if (crate->currentLOD >= etgLOD->nLevels)
        {
            stat = NULL;
        }
        else
        {
            stat = etgLOD->level[crate->currentLOD];
        }
    }
    else
    {
        stat = NULL;
    }
    #if ETG_DISABLEABLE
    if (stat != NULL && etgEffectsEnabled)
    #else
    if (stat != NULL)
    #endif
    {
        etgEffectCreate(stat, NULL, &crate->posinfo.position, &zero, &ident, 1.0f, EAF_AllButNLips, 0);
    }

}

void crateInit()
{
    sdword i;
    for(i=0;i<=StandardFrigate;i++)
    {
        NUM_IN_GROUPS[i] = 1;
        SHIP_PROBS[i] = 0.0f;
    }

    scriptSet(NULL,"Crates.script",cratesScriptTable);

    NUM_IN_CLASSES[0] = 0;    //mothership not applicable
    NUM_IN_CLASSES[CLASS_HeavyCruiser] = sizeof(cratesCLASS_HeavyCruiser)/4;
    NUM_IN_CLASSES[CLASS_Carrier] = sizeof(cratesCLASS_Carrier)/4;
    NUM_IN_CLASSES[CLASS_Destroyer] = sizeof(cratesCLASS_Destroyer)/4;
    NUM_IN_CLASSES[CLASS_Frigate] = sizeof(cratesCLASS_Frigate)/4;
    NUM_IN_CLASSES[CLASS_Corvette] = sizeof(cratesCLASS_Corvette)/4;
    NUM_IN_CLASSES[CLASS_Fighter] = sizeof(cratesCLASS_Fighter)/4;
    NUM_IN_CLASSES[CLASS_Resource] = sizeof(cratesCLASS_Resource)/4;
    NUM_IN_CLASSES[CLASS_NonCombat] = sizeof(cratesCLASS_NonCombat)/4;




}

