#ifndef __AIEVENTS_H
#define __AIEVENTS_H

#include "Types.h"
#include "ShipSelect.h"
#include "SpaceObj.h"

struct AITeam;

//
//  the voids should actual be AITeams, but
//  I seem to have a recursive definition problem
//  then, between AITeam and AIEvents.
//
typedef void (*aieHandlerSimple) (struct AITeam *team);
typedef void (*aieHandlerShip)   (struct AITeam *team, ShipPtr ship);
typedef void (*aieHandlerShips)  (struct AITeam *team, SelectCommand *ships);
typedef void (*aieHandlerInt)    (struct AITeam *team, udword *intvar);

//
//  parameters and handlers for various events that
//  can be watched for during move execution
//
//  the watch* members are essentially parameters on
//  when the event should be fired
//
//  there are function prototypes below for enabling
//  handlers on a particular move.
//
//  default is a NULL handler
//
typedef struct {
    struct {
        bool8               oneShot;
        bool8               triggered;
        aieHandlerShips     handler;
    } gettingRocked;  // team is under attack

    struct {
        real32              watchRadius;
        bool8               oneShot;
        bool8               triggered;
        aieHandlerShips     handler;
    } enemyNearby;  // enemy within certain radius

    struct {
        real32              watchRadius;
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } enemyNotNearby;   // enemy is not (no longer) within certain radius

    struct {
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } firing;  // team has started to fire (usually this will be a one-shot)

    struct {
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } disengage; //team has stopped firing (usually this will be a one-shot)
                    //note this event only gets checked if the firing event has been triggered

    struct {
        sdword              watchPercentage;
        bool8               watchIndividual;
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } healthLow;  // team health drops below certain level

    struct {
        sdword              watchPercentage;
        bool8               watchIndividual;
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } healthHigh;  // team health rises above certain level

    struct {
        sdword              watchPercentage;
        sdword              watchBaseCount; // # of ships
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } numbersLow;  // team size drops below certain # of ships

    struct {
        sdword              watchPercentage;
        sdword              watchBaseCount; // # of ships
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } numbersHigh;  // team size rises above certain # of ships

    struct {
        sdword              watchPercentage;
        bool8               watchIndividual;
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } fuelLow;  // team fuel drops below certain level

    struct {
        sdword              watchPercentage;
        bool8               watchIndividual;
        bool8               oneShot;
        bool8               triggered;
        aieHandlerSimple    handler;
    } fuelHigh;  // team fuel rises above certain level

    struct {
        bool8               oneShot;
        bool8               triggered;
        aieHandlerShip      handler;
    } shipDied;  // team member dies

    struct {
        aieHandlerSimple    handler;
    } teamDied;  // last team member dies

    struct {
        bool8               oneShot;
        bool8               triggered;
        udword              interval;       //how often to skip this event before triggering
        udword              intervalcnt;    //counter to keep track of skips
        udword              *intvar;
        aieHandlerInt       handler;
    } interrupt;

} AIEvents;

sdword aieCheckGettingRocked(struct AITeam *team, SelectCommand **ships);
sdword aieCheckEnemyNearby(struct AITeam *team, SelectCommand **ships);
sdword aieCheckEnemyNotNearby(struct AITeam *team);
sdword aieCheckFiring(struct AITeam *team);
sdword aieCheckDisengage(struct AITeam *team);
sdword aieCheckHealthLow(struct AITeam *team);
sdword aieCheckHealthHigh(struct AITeam *team);
sdword aieCheckNumbersLow(struct AITeam *team);
sdword aieCheckNumbersHigh(struct AITeam *team);
sdword aieCheckFuelLow(struct AITeam *team);
sdword aieCheckFuelHigh(struct AITeam *team);
sdword aieCheckShipDied(struct AITeam *team, ShipPtr *ship);
sdword aieCheckTeamDied(struct AITeam *team);
sdword aieCheckInterrupt(struct AITeam *team);

struct AITeamMove;

void aieHandlerSetGettingRocked (struct AITeamMove *move, bool8 oneShot, aieHandlerShips handler);
void aieHandlerSetEnemyNearby(struct AITeamMove *move, real32 watchRadius, bool8 oneShot, aieHandlerShips handler);
void aieHandlerSetEnemyNotNearby(struct AITeamMove *move, real32 watchRadius, bool8 oneShot, aieHandlerSimple hander);
void aieHandlerSetFiring(struct AITeamMove *move, bool8 oneShot, aieHandlerSimple handler);
void aieHandlerSetDisengage(struct AITeamMove *move, bool8 oneShot, aieHandlerSimple handler);
void aieHandlerSetHealthLow(struct AITeamMove *move, sdword watchPrecentage, bool8 watchIndividual, bool8 oneShot, aieHandlerSimple handler);
void aieHandlerSetHealthHigh (struct AITeamMove *move, sdword watchPercentage, bool8 watchIndividual, bool8 oneShot, aieHandlerSimple handler);
//note: if watchBaseCount in "SetNumbersLow/High" is 0, the initial value will be set
//      using the number of ships in the team the first time the event is checked
void aieHandlerSetNumbersLow(struct AITeamMove *move, sdword watchPercentage, sdword watchBaseCount, bool8 oneShot, aieHandlerSimple handler);
void aieHandlerSetNumbersHigh (struct AITeamMove *move, sdword watchPercentage, sdword watchBaseCount, bool8 oneShot, aieHandlerSimple handler);
void aieHandlerSetFuelLow(struct AITeamMove *move, sdword watchPercentage, bool8 watchIndividual, bool8 oneShot, aieHandlerSimple handler);
void aieHandlerSetFuelHigh(struct AITeamMove *move, sdword watchPercentage, bool8 watchIndividual, bool8 oneShot, aieHandlerSimple handler);
void aieHandlerSetShipDied(struct AITeamMove *move, bool8 oneShot, aieHandlerShip handler);
void aieHandlerSetTeamDied(struct AITeamMove *move, aieHandlerSimple handler);
void aieHandlerSetInterrupt(struct AITeamMove *move, udword *intvar, udword interval, bool8 oneShot, aieHandlerInt handler);

void aieHandlersClear (struct AITeamMove *move);

void aieExecute(struct AITeam *team);

// SaveGame Stuff
void aieFixAIEvents(struct AITeamMove *move);
void aiePreFixAIEvents(struct AITeamMove *move);

aieHandlerSimple aieNumToHandler(sdword num);
sdword aieHandlerToNum(aieHandlerSimple handler);

#endif
