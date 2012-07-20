#ifndef __AIORDERS_H
#define __AIORDERS_H

#include "Types.h"
#include "ShipSelect.h"
#include "AIUtilities.h"

/*=============================================================================
    Constant Definitions:
=============================================================================*/
#define RECON_STANDOFF_DISTANCE     15000
#define HARASS_STANDOFF_DISTANCE    25000
#define HARASS_LIGHTINT_INITNUM     15
#define HARASS_LIGHTINT_REPLACE     12
#define HARASS_HEAVYINT_EQUIVNUM    14
#define HARASS_BOMBER_EQUIVNUM      14

/*=============================================================================
    Structures:
=============================================================================*/
typedef enum
{
    RECON_MOTHERSHIP,
    RECON_ACTIVE_GENERAL,
    RECON_ACTIVE_ENEMY,
    NUM_RECONTYPE
} ReconType;

typedef enum
{
    SUPPORT_STRIKECRAFT,
    SUPPORT_RESOURCE,
    SUPPORT_MOTHERSHIP,
    NUM_SUPPORTTYPE
} SupportType;

void aioCreateGuardShips(struct AITeam *team, SelectCommand *ships);
void aioCreateReconaissance(struct AITeam *team, ReconType type);
void aioCreateTakeoutTarget(struct AITeam *team,Ship *target);
void aioCreateHarass(struct AITeam *team);
void aioCreateTakeoutMothershipFast(struct AITeam *team,Ship *mothership);
void aioCreateTakeoutMothershipGuard(struct AITeam *team,Ship *mothership);
void aioCreateTakeoutMothershipHuge(struct AITeam *team, Ship *mothership);
void aioCreateTakeoutMothershipBig(struct AITeam *team,Ship *mothership, bool ForceBig);
void aioCreateTakeoutTargetsWithCurrentTeam(struct AITeam *team,SelectCommand *targets);
void aioCreateTakeoutTargetWithCurrentTeam(struct AITeam *team,Ship *ship);
void aioCreateFancyTakeoutTarget(struct AITeam *team,Ship *target);
void aioCreateDefendMothership(struct AITeam *team);
void aioCreatePatrol(struct AITeam *team, Path *path);
void aioCreateFastRovingDefense(struct AITeam *team);
void aioCreateSlowRovingDefense(struct AITeam *team);
void aioCreateActiveSupport(struct AITeam *team, SelectCommand *ships, SupportType type);
void aioCreateFighterStrike(struct AITeam *team);
void aioCreateCorvetteStrike(struct AITeam *team);
void aioCreateFrigateStrike(struct AITeam *team);
void aioCreateResourcer(struct AITeam *team);
void aioCreateCapture(struct AITeam *team);
void aioCreateMine(struct AITeam *team);
void aioCreateSpecialDefense(struct AITeam *team, ShipType type);
void aioCreateSwarmAttack(struct AITeam *team);
void aioCreateSwarmDefense(struct AITeam *team, SelectCommand *Pod);
void aioCreateSwarmSupport(struct AITeam *team);
void aioCreateMultiBeamAttack(struct AITeam *team);
void aioCreateP2MothershipAttack(struct AITeam *team);

#endif
