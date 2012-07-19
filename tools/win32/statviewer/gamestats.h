/*=============================================================================
    Name    : gamestats.h
    Purpose : Game Statistics Include information

    Created 06/19/1999 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

//[x] beside statistics that are functional

//resource units doesn't handle retired resources...

typedef struct PlayerStats
{
    real32 timeOfDeath;                                 //[x]
    sdword typeOfPlayerDeath;                           //[x]
    sdword totalResourceUnits;                          //[]    //this value always goes up...except when giving RU's away
    sdword totalResourceUnitsCollected;                 //[x]   //all resource units collected via harvesting/salvaging of ships
    sdword totalResourceUnitsSpent;                     //[x]
    sdword totalResourceUnitsGiven;                     //[x]
    sdword totalResourceUnitsRecieved;                  //[x]
    sdword totalRegeneratedResourceUnits;               //[x]
    sdword totalInjectedResources;                      //[x]
    sdword totalResourceUnitsViaBounties;               //[x]

    sdword totalRUsInCurrentShips;                      //[x]  //value of ships player currently has
    sdword totalShips;                                  //[x]
    sdword totalForEachShip[TOTAL_NUM_SHIPS];           //[x]
    sdword totalForEachClass[NUM_CLASSES];              //[x]

    sdword totalKills;                                  //[x]
    sdword totalRUsKilled;                              //[x]
    sdword totalKillsForEachShip[TOTAL_NUM_SHIPS];      //[x]
    sdword totalKillsForEachClass[NUM_CLASSES];         //[x]

    sdword totalLosses;                                 //[x]
    sdword totalRUsLost;                                //[x]
    sdword totalLossesForEachShip[TOTAL_NUM_SHIPS];     //[x]
    sdword totalLossesForEachClass[NUM_CLASSES];        //[x]

    real32 pointsOfDamageDoneToWho[MAX_MULTIPLAYER_PLAYERS+1];                          //[]
}PlayerStats;

typedef struct GameStats
{
    sdword totalResourcesInGame;                        //[]
    sdword totalShipsInGame;                            //[] built/aquired
    sdword totalKills;                                  //[x]
    sdword totalRUsInAllShips;                          //[x]
    sdword totalRUsKilled;                              //[x]
    sdword totalRUsLost;                                //[x]
    sdword totalResourceUnitsCollected;                 //[x]  //via harvesting
    real32 updatedRUValuesTime;
    sdword universeRUWorth;
    sdword startingResources;
    PlayerStats playerStats[MAX_MULTIPLAYER_PLAYERS+1];   //[]
}GameStats;

