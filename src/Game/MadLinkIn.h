/*=============================================================================
    Name    : madLinkIn.h
    Purpose : defines and prototypes for madlinkin.c

    Created 31/08/1998 by bryce pasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#include "CommandLayer.h"


//prototypes

//general prototypes
void madLinkInUpdateMeshAnimations(Ship *ship);
void madLinkInSetUpInitialBuiltMadState(Ship *ship);
void madLinkInSetUpInitialPlacedMadState(Ship *ship);

//gun opening closing
void madLinkInOpenGuns(CommandToDo *command,udword priority);
void madLinkInCloseGuns(CommandToDo *command,udword priority);
void madOpenGunsShip(Ship *ship);
void madLinkInCloseGunsShip(Ship *ship);

//docking oriented (fins opening closing)
void madLinkInPreDocking(CommandToDo *command,udword priority);
void madLinkInPreDockingShip(Ship *ship);
void madLinkInPostDocking(CommandToDo *command,udword priority);
void madLinkInPostDockingShip(Ship *ship);
void madLinkInOpenSpecialShip(Ship *ship);
void madLinkInCloseSpecialShip(Ship *ship);

void madLinkInOpenDoor(Ship *ship);
void madLinkInCloseDoor(Ship *ship);
bool madLinkInGetDoorInfo(Ship *ship, matrix *coordsys, vector *position);



