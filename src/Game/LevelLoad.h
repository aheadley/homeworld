
#ifndef ___LEVELLOAD_H
#define ___LEVELLOAD_H

#include "Types.h"
#include "ObjTypes.h"

/*=============================================================================
    Functions:
=============================================================================*/

void levelPreInit(char *directory,char *pickedMission);
void levelInit(char *directory,char *pickedMission);

void levelStartNext(char *directory,char *pickedMission);        // for single player game only

// utility functions
ShipType GetAppropriateShipTypeForRace(ShipType request,ShipRace shiprace);
void TryToFindMothershipsForPlayers();
ShipRace GetSinglePlayerRaceEquivalent(ShipRace race);

bool AddResourceToSphere(struct ResourceVolume *sphere,bool regrowing);
bool AddResourceToCylinder(struct ResourceVolume *cylinder,bool regrowing);
bool AddResourceToRectangle(struct ResourceVolume *cylinder,bool regrowing);

void SetInfoNeededForShipAndRelatedStaticInfo(ShipType type,ShipRace race,bool8 dataToFillIn);

/*=============================================================================
    Global variables:
=============================================================================*/

extern udword nebAttributes;
extern sdword SongNumber;

#endif

