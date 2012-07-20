/*=============================================================================
    SPACEOBJ.H: Object-oriented "Space Object" formats

    Created June 1997 by Gary Shaw
=============================================================================*/

#ifndef ___SPACEOBJ
#define ___SPACEOBJ

#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "LinkedList.h"
#include "Mesh.h"
#include "LOD.h"
#include "ObjTypes.h"
#include "ETG.h"
#include "trails.h"
#include "color.h"
#include "SoundStructs.h"
#include "MeshAnim.h"
#include "Clouds.h"
#include "Nebulae.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release

#define SO_ERROR_CHECKING       1               //general error checking
#define SO_VERBOSE_LEVEL        2               //print extra info
#define SO_CLOOGE_SCALE         1               //clooge a largeness factor for the ships

#else //HW_Debug

#define SO_ERROR_CHECKING       0               //general error checking
#define SO_VERBOSE_LEVEL        0               //print extra info
#define SO_CLOOGE_SCALE         0               //clooge a largeness factor for the ships

#endif //HW_Debug


/*-----------------------------------------------------------------------------
    Gun Object
    Note: the gun object is not an independent object like an asteroid or ship.
          It must be part of a ship object.
-----------------------------------------------------------------------------*/
//flags for the gun object
#define GF_MultiLevelMatrix     0x00000001
typedef struct
{
    GunType guntype;
    GunSoundType gunsoundtype;
    BulletType bulletType;      // type of bullet gun fires
    udword flags;               //flags for the gun
    udword gunindex;            //index of gun

    vector position;            // position relative to ship

    // for GUN_Gimble
    vector gunnormal;           // normal vector describing gun orientation (rel to ship)
    real32 cosminAngleFromNorm; // cos of min angle required between gunnormal and gunheading
    real32 cosmaxAngleFromNorm; // cos of max angle possible between gunnormal and gunheading

    // for GUN_NewGimble
    matrix defaultgunorientation;
    matrix defaultGunOrientationNonConcat;

    real32 minturnangle,maxturnangle;
    real32 mindeclination,maxdeclination;
    real32 maxanglespeed;
    real32 maxdeclinationspeed;
    real32 angletracking;
    real32 declinationtracking;

    // for GUN_MissileLauncher
    sdword maxMissiles;

    real32 triggerHappy;        // cosine of maximum allowed angle allowed before a fixed gun will shoot
    real32 gunDamageLo[NUM_TACTICS_TYPES];          // low damage potential
    real32 gunDamageHi[NUM_TACTICS_TYPES];          // high damage potential
    real32 baseGunDamageLo;     //based values loaded in from script
    real32 baseGunDamageHi;   //dido, only used before game loads to precalculate different damages
    real32 bulletlength;        // length of bullets fired by gun
    real32 bulletlifetime;      // life time of bullet
    real32 bulletspeed;         // speed of bullet (0 indicates infinite speed)
    real32 bulletrange;         // total distance bullet can travel
    real32 bulletmass;          // mass of bullet gun shoots
    real32 firetime;            // minimum time required before gun can fire again
    real32 burstFireTime;       // time in seconds a gun can burst (if burstFireTime == 0, this gun doesn't fire in bursts)
    real32 burstWaitTime;       // time in seconds gun must cool down after a burst

    real32 barrelLength;        // length of gun barrel for offsetting muzzle flashes
    real32 recoilLength;        // amount of recoil on the gun when it fires

    //additional stuff for use only in the R1 Heavy Cruiser main ion guns
    vector offset;              //offset to rotate about gun's pivot point
    sdword slaveDriver;         //for slaving guns together

} GunStatic;

typedef struct
{
    NAVLightType lightType;      // type of nav light
    vector position;             // position relative to ship
    rgbquad color;               // color of nav light
    real32 flashrateon;          // delay in seconds each flash is on.
    real32 flashrateoff;         // delay in seconds each flash is off.
    real32 startdelay;             // time to wait before starting.
    real32 size;                 // size of light in meters.
    udword minLOD;               // min LOD to draw light at.
    char name[20];               // name of nav light
    udword texturehandle;      // handle to a texture (if any).
} NAVLightStatic;

typedef struct
{
    vector position;            // position relative to ship
    vector normal;              // normal vector (rel to ship)
} DamageLightStatic;

typedef struct
{
    vector position;            // position relative to ship
    vector nozzlenormal;        // normal vector describing nozzle orientation (rel to ship)
} ResNozzleStatic;

typedef struct
{
    vector position;            // position relative to ship
} RepairNozzleStatic;

typedef struct
{
    vector position;            // position relative to ship
    vector nozzlenormal;        // normal vector describing nozzle orientation (rel to ship)
} TractorBeamStatic;

// types for mine formations

#define PULSE_START 0
#define PULSE_SPAWN_EFFECT 1
#define PULSE_MONITOR_EFFECT 2
#define PULSE_REDIRECT 3

typedef struct
{
    Node FormLink;
    LinkedList MineList;
    vector pulse_dir;               //stores pulses current location
    vector effect_going_to;
    real32 waittime;
    struct Player *playerowner;
    bool FULL;
    sdword wallstate;
    struct Effect *effect;
    udword numat;
} MineFormationInfo;

typedef struct
{
    // for GUN_Gimble
    vector gunheading;          // gun heading (relative to ship)

    // for GUN_NewGimble
    matrix curgunorientation;
    matrix curGunOrientationNonConcat;
    real32 angle;
    real32 declination;
    real32 anglespeed;
    real32 declinationspeed;

    // for GUN_MissileLauncher
    sdword numMissiles;

    sdword gimblehandle;

    real32 lasttimefired;       // last time gun was fired
    real32 burstStartTime;      // last time gun started burst
    real32 useBurstFireTime;    // actual burst fire/wait times to use after modified (optimization)
    real32 useBurstWaitTime;
    udword burstState;          // gun burst state variable
    GunStatic *gunstatic;       // pointer to static information
} Gun;

typedef struct
{
    udword lightstate;
    real32 lastTimeFlashed;
    NAVLightStatic *navlightstatic;
} NAVLight;

typedef struct
{
    sdword numGuns;
    Gun guns[1];
} GunInfo;

typedef struct
{
    sdword numLights;
    NAVLight navLights[1];
} NAVLightInfo;

typedef struct
{
    sdword numGuns;
    GunStatic gunstatics[1];
} GunStaticInfo;

typedef struct
{
    sdword numNAVLights;
    NAVLightStatic navlightstatics[1];
} NAVLightStaticInfo;

/*  -------------------------------------------------------
    Static Salvaging Info
    - used to define latch on points for salvage
      capturing vessels
    ------------------------------------------------------- */

//maximum number of ships that can be captured and waiting inside
//a mothership or carrier at any one time to pop out after being
//retrofitted
#define SALVAGE_MAX_CAPTURABLE  50

typedef struct
{
    udword number;
    SalvagePointType type;
    vector position;
    vector conenormal;
    real32 coneangle;
    char name[20];
}SalvageStaticPoint;

typedef struct
{
    sdword numSalvagePoints;
    sdword numNeededForSalvage;
    sdword needBigR1;
    sdword needBigR2;
    sdword willFitCarrier;
    SalvageStaticPoint headingPoint;
    SalvageStaticPoint salvageStaticPoints[1];   //ragged array
} SalvageStaticInfo;

//Non static salvage Information
typedef struct
{
    sdword busy;
    SalvageStaticPoint *salvageStaticPoint;
}SalvagePoint;

typedef struct
{
    sdword numSalvagePoints;
    sdword numSalvagePointsFree;
    sdword numNeededForSalvage;
    SalvagePoint salvagePoints[1];   //ragged array
} SalvageInfo;

#define sizeofSalvageStaticInfo(x)  sizeof(SalvageStaticPoint)*(x-1)+sizeof(SalvageStaticInfo)
#define sizeofSalvageInfo(x)  sizeof(SalvagePoint)*(x) +sizeof(SalvageInfo)

/*-----------------------------------------------------------------------------
    Static Docking Information
    - not an independent object, but part of ShipStaticInfo
-----------------------------------------------------------------------------*/
#define LIGHTNAME_LENGTH    20
typedef struct
{
  ShipType shiptype;
  ShipRace shiprace;
  sword useNewOrientation;
  uword heading;
  uword up;
  sword useNewOffset;
  vector offset;    //offset modification
  bool lightNameUsed;
  char lightName[LIGHTNAME_LENGTH];
} DockStaticOveride;

typedef struct
{
  sdword numDockOverides;
  DockStaticOveride dockOverides[1];
} DockOverideInfo;


typedef struct
{
    DockPointType type;
    sdword dockindex;
    vector position;
    vector conenormal;
    real32 coneangle;
    real32 flyawaydist;
    real32 mindist;
    real32 maxdist;
    udword headingdirection;
    udword updirection;
    char name[LIGHTNAME_LENGTH];
} DockStaticPoint;

typedef struct
{
    sdword numDockPoints;
    DockStaticPoint dockstaticpoints[1];
} DockStaticInfo;

/*-----------------------------------------------------------------------------
    Docking Information
    - not an independent object, but part of Ship
-----------------------------------------------------------------------------*/

typedef struct
{
    DockStaticPoint *dockstaticpoint;
    udword thisDockBusy;
} DockPoint;

typedef struct
{
    sdword busyness;
    sdword numDockPoints;
    DockPoint dockpoints[1];
} DockInfo;

typedef struct
{
    Node node;
    struct Ship *ship;
} InsideShip;

typedef struct
{
    LinkedList insideList;
    sword      FightersInsideme;
    sword      CorvettesInsideme;
//    LinkedList launchList;
} ShipsInsideMe;

//CLAMPING INFO

typedef struct
{
    struct SpaceObjRotImpTarg *host;
    matrix clampCoordsys;
    vector clampOffset;
}ClampInfo;

/*=============================================================================
    Object Static Types
=============================================================================*/

/*=============================================================================
    Basic SpaceObjStaticInfo type, from which all StaticInfo objects are derived
=============================================================================*/

typedef struct
{
    real32 approxcollspheresize;    // rough collision sphere size more repesentative of object's actual size, rather
                                    // than worst case size used for trivial rejection
    real32 avoidcollspheresize;     // collision sphere size used for collision avoidance
    real32 avoidcollspherepad;      // collision sphere size used for collision avoidance, with padding

    real32 approxcollmodifier;    // approxcollspheresize = approxcollmodifier * collspheresize
    real32 avoidcollmodifier;     // avoidcollspheresize = avoidcollmodifier * collspheresize

    real32 originalcollspheresize;  // original collspheresize DO NOT CHANGE THIS IF OBJECT SCALES!
    real32 collspheresize;  // bounding sphere for rough collision detection
    real32 collspheresizeSqr; // square of collspheresize precomputed
    vector collsphereoffset;// bounding sphere offset in object co-ordinates

    vector collrectoffset;  // collision rectangle offset in object co-ordinates (leftdownback corner of cube)
    real32 uplength;        // length of rectangle in up direction
    real32 rightlength;     // length of rectangle in right direction
    real32 forwardlength;   // length of rectangle in forward direction
    real32 diagonallength;  // length of rectangle from point 0 to point 6 or rectangle
    udword preciseSelection; //object uses precise selection mechanism if LOD <= this level

    //dimensional scale factors
    real32 collBoxScaleForward;
    real32 collBoxScaleUp;
    real32 collBoxScaleRight;

    real32 collBoxOffsetX;
    real32 collBoxOffsetY;
    real32 collBoxOffsetZ;
} StaticCollInfo;

#define IF_InfoNeeded           0x01
#define IF_InfoLoaded           0x02
#define IF_InfoWillBeNeeded     0x04
#define IF_NotPostLoaded        0x08

typedef struct
{
    lodinfo *LOD;           // Level-of-detail information
    void *pMexData;         // pointer to MEX data (mesh extensions)
    real32 mass;            // object mass (can be negative for neat effects)
    real32 momentOfInertiaX;// moment of inertia of object about X axis
    real32 momentOfInertiaY;// moment of inertia of object about Y axis
    real32 momentOfInertiaZ;// moment of inertia of object about Z axis
    real32 oneOverMass;            // object mass (can be negative for neat effects)
    real32 oneOverMomentOfInertiaX;// moment of inertia of object about X axis
    real32 oneOverMomentOfInertiaY;// moment of inertia of object about Y axis
    real32 oneOverMomentOfInertiaZ;// moment of inertia of object about Z axis
    real32 maxvelocity;     // maximum velocity of object
    real32 maxrot;          // maximum rotate speed (rad/s) of object
//    bool8  infoNeeded;      //static info is needed for this level and should be loaded
//    bool8  infoLoaded;      //static info has been loaded properly
    ubyte  infoFlags;       //IF_InfoNeeded, IF_InfoLoaded, IF_InfoWillBeNeeded
    bool8  immobile;        // flag indicating object is immobile
    sbyte  rightOfWay;
    StaticCollInfo staticCollInfo;
} StaticHeader;

typedef struct
{
    StaticHeader staticheader;
} StaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
} StaticInfoHealth;

#define TRANS_UP                0
#define TRANS_DOWN              1
#define TRANS_RIGHT             2
#define TRANS_LEFT              3
#define TRANS_FORWARD           4
#define TRANS_BACKWARD          5
#define NUM_TRANS_DEGOFFREEDOM  6
#define TRANS_BACK              TRANS_BACKWARD

#define ROT_ABOUTXCCW           0
#define ROT_ABOUTXCW            1
#define ROT_ABOUTYCCW           2
#define ROT_ABOUTYCW            3
#define ROT_ABOUTZCCW           4
#define ROT_ABOUTZCW            5
#define NUM_ROT_DEGOFFREEDOM    6

#define ROT_YAWLEFT             ROT_ABOUTXCCW
#define ROT_YAWRIGHT            ROT_ABOUTXCW
#define ROT_PITCHUP             ROT_ABOUTYCCW
#define ROT_PITCHDOWN           ROT_ABOUTYCW
#define ROT_ROLLRIGHT           ROT_ABOUTZCCW
#define ROT_ROLLLEFT            ROT_ABOUTZCW

#define TURN_ABOUTX             0
#define TURN_ABOUTY             1
#define TURN_ABOUTZ             2
#define NUM_TURN_TYPES          3

#define TURN_YAW                TURN_ABOUTX
#define TURN_PITCH              TURN_ABOUTY
#define TURN_ROLL               TURN_ABOUTZ

#define MAX_NUM_TRAILS          4

typedef struct StaticInfoHealthGuidance
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];
} StaticInfoHealthGuidance;

typedef struct StaticInfoHealthGuidanceShipDerelict
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];

    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];           //what color schemes this can be
    SalvageStaticInfo   *salvageStaticInfo;           //static info for salvageable objects

    //these 3 variables must be next
    real32 sinbank;             // variables used by the AIShip routines
    real32 pitchturn;
    real32 pitchdescend;
} StaticInfoHealthGuidanceShipDerelict;

//info for SpaceObjRotImpTarg objects that was once
//static, but now needs to be non-static
typedef struct
{
    real32 thruststrength[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrength[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeed[NUM_TURN_TYPES];
} NonStats;

/*=============================================================================
    Ship StaticInfo
=============================================================================*/

#define THISSHIPIS_FIGHTER              0
#define THISSHIPIS_CORVETTE             1
#define THISSHIPIS_RESOURCER            2
#define THISSHIPIS_OTHERNONCAPITALSHIP  3

struct ShipStaticInfo;
struct CommandToDo;

typedef struct
{
    ShipType shiptype;
    udword sizeofShipSpecifics;
    void (*CustShipStaticInit) (char *directory,char *filename,struct ShipStaticInfo *statinfo);
    void (*CustShipStaticInitPost) (struct ShipStaticInfo *statinfo);
    void (*CustShipInit) (struct Ship *ship);
    void (*CustShipClose) (struct Ship *ship);
    void (*CustShipAttack) (struct Ship *ship,struct SpaceObjRotImpTarg *target,real32 maxdist);
    void (*CustShipFire) (struct Ship *ship, struct SpaceObjRotImpTarg *target);
    void (*CustShipAttackPassive) (struct Ship *ship,struct Ship *target,bool rotate);
    bool (*CustShipSpecialActivate) (struct Ship *ship);
    bool (*CustShipSpecialTarget) (struct Ship *ship,void *custom);
    void (*CustShipHousekeep) (struct Ship *ship);
    void (*CustShipRemoveShipReferences)(struct Ship *ship, struct Ship *shiptoremove);
    void (*CustShipDied)(struct Ship *ship);
    void (*CustShip_PreFix)(struct Ship *ship);
    void (*CustShip_Save)(struct Ship *ship);
    void (*CustShip_Load)(struct Ship *ship);
    void (*CustShip_Fix)(struct Ship *ship);
} CustShipHeader;

typedef struct ShipStaticInfo
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    //these 3 things are static and don't affect a ship after the
    //ship has been built.  They are copied into the NonStats data
    //structure due to a need for them to be nonstatic
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];

    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];           //what color schemes this can be
    SalvageStaticInfo   *salvageStaticInfo;           //static info for salvageable objects

    //these 3 variables must be next
    real32 sinbank;             // variables used by the AIShip routines
    real32 pitchturn;
    real32 pitchdescend;

    NAVLightStaticInfo *navlightStaticInfo;
/*ship specific stuff below */
    CustShipHeader custshipheader;
    void *custstatinfo;
    real32 bulletRangeSquared[NUM_TACTICS_TYPES];
    real32 bulletRange[NUM_TACTICS_TYPES];
    real32 minBulletRange[NUM_TACTICS_TYPES];
    real32 blastRadiusShockWave;
    real32 blastRadiusDamage;
    real32 passiveRetaliateZone;
    real32 collSideModifiers[NUM_TRANS_DEGOFFREEDOM];

    ShipType shiptype;
    ShipRace shiprace;
    ShipClass shipclass;

    bool8 canBuildShips;
    bool8 canReceiveShips;
    bool8 canReceiveResources;
    bool8 canReceiveShipsPermanently;
    bool8 canReceiveShipsForRetire;
    bool8 canReceiveSomething;
    bool8 canBuildBigShips;
    bool8 canTargetMultipleTargets;
    bool8 rotateToRetaliate;
    bool8 canReceiveTheseShips[4];

    bool passiveAttackPenaltyExempt;

    bool8 specialActivateIsContinuous;
    bool8 canSpecialBandBoxFriendlies;
    bool8 canSingleClickSpecialActivate;

    sword maxDockableFighters;  // maximum fighters that can dock at this ship
    sword maxDockableCorvettes; // maximum corvettes that can dock at this ship

    sdword canHandleNumShipsDocking;
    sdword canHandleNumShipsDepositingRU;

    bool repairBeamCapable;
    real32 healthPerSecond;
    real32 CapitalDistanceRepairStart2;
    real32 CapitalDistanceRepairStart;
    real32 AngleDotProdThreshold;

    sdword buildCost;
    sdword buildTime;
    sdword svManeuverability;
    sdword svCoverage;
    sdword svFirePower;

    sdword resourcesAtOneTime;
    sdword maxresources;
    sdword harvestRate;
    sdword harvestAmount;

    sdword groupSize;

    real32 maxfuel;
    real32 lowfuelpoint;
    real32 fuelburnrate;
    real32 attackfuelburnrate;

    real32 repairTime;          // self-repair for capital ships
    real32 repairDamage;
    real32 repairCombatTime;    // self-repair for capital ships in combat
    real32 repairCombatDamage;

    real32 pumpFuelRate;        // rate at which ship can refuel other ships
    real32 repairOtherShipRate; // rate at which ship can repair other ships
    real32 repairResourceCollectorRate; //specifically for when a ship is repairing a resource collector.  0.0f deafults to previous value
    real32 clearanceDistance;   // distance which other docking ship should fly away for clearance
    udword clearanceDirection;  // direction which other ship should fly away for clearance (0..7)

    real32 dockShipRange;       // distance to ship to get for docking, squared

    real32 formationPaddingModifier;
    bool cannotForceAttackIfOwnShip;    //variable..if set to TRUE player cannot force attack this ship IF it is their own ship
    bool shipIsCapital;                 //Set to TRUE is ship is a TRUE capital ship (class defs are unreliable)
    bool cantMoveAndAttack;             //if set, SHIP explicitly cannot move and attack...will be given a special move order.
    vector engineNozzleOffset[MAX_NUM_TRAILS];      //offset of engine in object co-ordinates
    trailstatic* trailStatic[MAX_NUM_TRAILS];
    udword multipleEngines;

    real32 scaleCap;            //minimum sizing cap, which prevents a ship from getting too small

    sdword hierarchySize;       //size of the ship hierarchy
    madstatic *madStatic;       //mesh animation binding and animation data
    real32 madGunOpenDamagedHealthThreshold;
    real32 madWingOpenDamagedHealthThreshold;

    // Alex's repair droid stuff...
//    udword nRepairDroids;

    // Keith's engine trail stuff...
    udword trailStyle[MAX_NUM_TRAILS];
    real32 trailWidth[MAX_NUM_TRAILS];
    real32 trailHeight[MAX_NUM_TRAILS];
    real32 trailAngle[MAX_NUM_TRAILS];
    real32 trailRibbonAdjust[MAX_NUM_TRAILS];
    real32 trailLength[MAX_NUM_TRAILS];
    real32 trailScaleCap[MAX_NUM_TRAILS];
    real32 trailSpriteRadius[MAX_NUM_TRAILS];
    real32 trailSpriteOffset[MAX_NUM_TRAILS];

    real32 minimumZoomDistance;
    real32 renderlistFade, renderlistLimitSqr;

    real32 dockLightNear, dockLightFar;
    color  dockLightColor;

    color  hyperspaceColor;

    real32 tacticalSelectionScale;

#if SO_CLOOGE_SCALE
    real32 scaleFactor;
#endif
    GunStaticInfo *gunStaticInfo;
    DockStaticInfo *dockStaticInfo;
    DockOverideInfo *dockOverideInfo;
    ResNozzleStatic *resNozzleStatic;
    RepairNozzleStatic *repairNozzleStatic;
    TractorBeamStatic *tractorEmitterStatic;
    bool (*ShipXDocksAtShipY) (struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith);
    bool (*LaunchShipXFromShipY) (struct Ship *ship,struct Ship *dockwith);
} ShipStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;
} ResourceStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;

    AsteroidType asteroidtype;      // Asteroid specific stuff from here on
    bool8 asteroidCanShrink;
    bool8 asteroidCanBreak;
    bool8 pad2;
    bool8 pad3;
    real32 asteroidMinShrink;
    udword breakinto[NUM_ASTEROIDTYPES];
} AsteroidStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;

    NebulaType nebulatype;
    bool8 nebulaCanShrink;
    bool8 pad[3];
    real32 nebulaMinShrink;
} NebulaStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;

    GasCloudType gascloudtype;
    bool8 gascloudCanShrink;
    bool8 pad[3];
    real32 gascloudMinShrink;
} GasCloudStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;

    DustCloudType dustcloudtype;
    bool8 dustcloudCanShrink;
    bool8 pad[3];
    real32 dustcloudMinShrink;
} DustCloudStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];

    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];           //what color schemes this can be
    SalvageStaticInfo   *salvageStaticInfo;           //static info for salvageable objects

    //these 3 variables must be next to be compatible with ship static info for aiship stuff
    real32 sinbank;             // variables used by the AIShip routines
    real32 pitchturn;
    real32 pitchdescend;

    NAVLightStaticInfo *navlightStaticInfo;
/* Derelict specific below */
    DerelictType derelicttype;
    bool    salvageable;                        //TRUE if derelict is to be targetable by salcap corvettes

    real32 scaleFactor;                         //only used for 'world rendering'
    bool   worldRender;                         //TRUE for Angel moon, homeworld, and POO

    real32 minimumZoomDistance;
    real32 renderlistFade, renderlistLimitSqr;
} DerelictStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];                   // these 8 first lines are same as ShipStaticInfo, allowing
    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];           //what color schemes this can be


/* missile specific below*/
    real32 mineRange;
    real32 mineRangeForced;
    real32 MINE_STOP_FRICTION;
    real32 MINE_RANGESQR;
    real32 MINE_RANGESQR_FORCED;
    real32 maxvelocity_FORCED;

    //missile trail stuff
    vector engineNozzleOffset;
    trailstatic* trailStatic;

    real32 trailWidth;
    real32 trailHeight;
} MissileStaticInfo;

/*=============================================================================
    Object Instance Information
=============================================================================*/
/*
typedef struct
{
    sdword nSides;          //3 or 4 sides
    vector p[4];            //corners;
}
precisepoly;
*/
typedef struct
{
    vector worldRectPos[8]; //collision rect in world space
    ubyte  corner[24];      //3 polys, 4 corners each, plus padding for run-over
//    precisepoly poly[5];        //there could be up to 5 polys
    sdword  nPolys;         //number of camera-facing sides of collision box (0-5)
}
PreciseSelection;

#define ISMOVING_MOVING     1
#define ISMOVING_ROTATING   2

typedef struct
{
    vector position;        // object position in universe
    vector velocity;        // velocity of object
    vector force;           // force acting on object
    bool16 isMoving;        // flag indicating if object is moving
    bool16 haventCalculatedDist;  // flag indicating haven't yet calculated distance
} PosInfo;

typedef struct
{
    vector collOffset;      // collision sphere offset from position in object coordinates.
    vector collPosition;    // object collision position in universe = position + collOffset
    vector rectpos[8];      // rectangle corner positions (in object coordinates)
    PreciseSelection *precise;//precise collision info; not all ships have this
    real32 selCircleX, selCircleY;// on-screen location of selection circle on-screen
    real32 selCircleRadius; // on-screen size of selection circle
    real32 selCircleDepth;  // depth of object in camera space
} CollInfo;

typedef struct
{
    matrix coordsys;        // matrix describing co-ordinate system of object
    vector rotspeed;        // rotational speed (rad/s)
    vector torque;          // torque acting on object
} RotInfo;

#define     SF_SLAVE    0x01
#define     SF_MASTER   0x02
#define     SF_DEAD     0x04
typedef struct
{
    udword flags;           //indicates status of Slavery
    struct Ship *Master;           //
    vector offset;          //ships position in MASTER space
    matrix coordsys;        //ships orientation in MASTER space
    LinkedList slaves;      //Defined only if a master (master not in list)
} SlaveInfo;

/*-----------------------------------------------------------------------------
Layout of co-ordinate system:

The ship heading, upvector, and rightvectors are stored in the coordsys
matrix as follows:

|   upx     rightx  headingx  |
|   upy     righty  headingy  |
|   upz     rightz  headingz  |
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    SpaceObj - General, base object from which all objects are derived
-----------------------------------------------------------------------------*/

#define SOF_Rotatable           0x00000001
#define SOF_Impactable          0x00000002      // objects such as asteroids, ships which
                                                // can impart a change in momentum
#define SOF_DontApplyPhysics    0x00000004      // don't apply physics to such things as
                                                // explosions
#define SOF_AttachPosition      0x00000008      // prevent 'torpedo effects'
#define SOF_AttachCoordsys      0x00000010      // ditto, but more so
#define SOF_AttachVelocity      0x00000020
#define SOF_AttachNLips         0x00000040      // for ETG N-Lips inherited from ships

#define SOF_NISShip             0x00000080      //subject to 'NIS physics'

#define SOF_CloakGenField       0x00000100      //if the object is cloaked with the local players own field...i.e. if he/she can see the object
#define SOF_Cloaked             0x00000200      //object is cloaked for whatever reason
#define SOF_Cloaking            0x00000400      //Set if Object is In Process of Cloaking
#define SOF_DeCloaking          0x00000800      //Set if Object is in process of DeCloaking

#define SOF_Dead                0x00001000      //Indicates object is dead, and cannot interact with selection/targeting in any way

#define SOF_Selected            0x00002000      //ship selection shite
#define SOF_DontCreateBlob      0x00002000      //this object's blob is fine as it is
#define SOF_BitForSaleCheap     0x00004000

#define SOF_Selectable          0x00008000      //Indicates object is selectable

#define SOF_Resource            0x00010000      //Indicates object is a resource

#define SOF_Hide                0x00020000      //Indicates object should be hidden (never drawn, and never put in render list)
#define SOF_ForceVisible        0x00040000      //object will always be in render list no matter what
#define SOF_BigObject           0x00080000      //Indicates object is big (e.g. A Mothership)

#define SOF_StaticInfoIsDynamic 0x00100000      //Indicates static info has been copied and modified and is dynamic

#define SOF_NotBumpable         0x00200000      //Indicates colliding with object should not result in momentum transferal

#define SOF_Targetable          0x00400000      // Indicates object is targetable

#define SOF_Slaveable           0x00800000      // Indicates if object is ABLE to be slaved..and
                                                //hence has relevant info in its slaveinfo struct

#define SOF_Crazy               0x01000000      // Indicates object is in crazy mode, and about to die. (Used for ships only)

#define SOF_DontDrawTrails      0x02000000      // Indicates we do not want to draw engine trails. (Used for ships only)

#define SOF_Disabled            0x04000000      //set if ship is to be a derelict in space

#define SOF_Clamped             0x08000000      //indicates ship is CLAMPED to another ship

#define SOF_Hyperspace          0x10000000      //indicates ship is in hyperspace ether

#define SOF_AlwaysSortFront     0x20000000      //object will always sort to the front

#define SOF_HideSave            0x40000000      //object was hidden
#define SOF_HideSave2           0x80000000      //object was hidden

typedef struct
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    StaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;           // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
} SpaceObj;

typedef SpaceObj *SpaceObjPtr;

/*=============================================================================
    SpaceObj - rotatable
=============================================================================*/

typedef struct
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    StaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;           // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
} SpaceObjRot;

/*=============================================================================
    SpaceObj - rotatable, impactable
=============================================================================*/

typedef struct
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    StaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
} SpaceObjRotImp;

/*=============================================================================
    SpaceObj - rotatable, impactable, targetable
=============================================================================*/

typedef struct SpaceObjRotImpTarg
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    StaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

} SpaceObjRotImpTarg;

typedef SpaceObjRotImpTarg *TargetPtr;

/*=============================================================================
    SpaceObj - rotatable, impactable, targetable, guidance
=============================================================================*/

typedef struct SpaceObjRotImpTargGuidance
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    StaticInfoHealthGuidance *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;
    ClampInfo *clampInfo;

} SpaceObjRotImpTargGuidance;   // used by missiles, ships, derelicts

typedef struct SpaceObjRotImpTargGuidanceShipDerelict
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    StaticInfoHealthGuidanceShipDerelict *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;
    ClampInfo *clampInfo;

    SalvageInfo *salvageInfo;       //salvage info...

    //must come next to be compatible with Derelict in aiship stuff
    ubyte aistate;                  // AI state variable
    ubyte pad1[2];
    sbyte putOnDoor;
    struct Ship *dockingship;       // if non-NULL, indicates no collisions should occur with this ship
    udword specialFlags;            //general purpose ship flags, see above for defines
    udword specialFlags2;           //more general purpose ship flags, see above for defines
    sdword salvageNumTagged[MAX_MULTIPLAYER_PLAYERS];        //number of salcaps vying for ship...used to disperese them amonst many targets
    // HERE UP SAME for Derelicts and Ships
} SpaceObjRotImpTargGuidanceShipDerelict;   // ships, derelicts

typedef struct TargetID
{
    uword objNumber;    // must be first member of structure to be compatible with other ID's
    uword objtype;
} TargetID;

/*=============================================================================
    Repair Droid Stuff:
=============================================================================*/
/*
typedef struct RepairDroid
{
    udword flags;       // RD Flags...
    PosInfo posinfo;    // Position
    RotInfo rotinfo;    // Rotation
} RepairDroid;
*/
/*-----------------------------------------------------------------------------
    Ship Object
-----------------------------------------------------------------------------*/

#define ATTACKEVENT_ENEMYTARGET_DIED    1
#define ATTACKEVENT_LEADER_DIED         2
#define ATTACKEVENT_WINGMAN_DIED        4

#define ATTACKSITUATION_EVADINGREPOS    1
#define ATTACKSITUATION_JUSTEVADING     2
#define ATTACKSITUATION_SPLITTING       3
#define ATTACKSITUATION_SANDWICH        4

//  universe ticks before a ship "forgets" that it was under attack
//  (used by the recentlyAttacked member -- keep this within a ubyte for now)
#define RECENT_ATTACK_DURATION        ((ubyte)(5 * UNIVERSE_UPDATE_RATE))

typedef struct
{
    sdword numAttackTargets;
    TargetPtr TargetPtr[1];
} AttackTargets;

#define sizeofAttackTargets(n) (sizeof(AttackTargets) + (n-1)*sizeof(TargetPtr))

typedef struct
{
    AttackTargets *multipleAttackTargets;   // only used by ships which can target multiple targets.
    SpaceObjRotImpTarg *attacktarget;
    struct Ship *myLeaderIs;
    struct Ship *myWingmanIs;
    uword flightmansLeft;       // number of flight maneuvers left
    uword attacksituation;      // what I am currently doing
    uword attackevents;         // event notification using ATTACKEVENT flags
    uword pad;
} SpecificAttackVars;

typedef struct
{
    struct Ship *dockship;
    struct Ship *busyingThisShip;
    vector destination;
    ubyte dockstate;
    ubyte dockstate2;
    ubyte dockstate3;
    sbyte reserveddocking;
    DockStaticPoint *dockstaticpoint;
    void *customdockinfo;
    sdword **launchpoints;
} SpecificDockVars;

typedef struct ShipID
{
    uword shipNumber;
} ShipID;

#define MAX_ID_NUMBER   65535

#define MAD_MAX_CUEABLE 2           //maximum number of animations that can be cued in a row

//Ship death flags for flag 'howDidIDie'
//howDidIDie == 0 ==> not dead, or didn't die by these means!
#define DEATH_Killed_By_Player          666 //another player killed this person (could be self..check other variable)
#define DEATH_Killed_By_Player_Explosion 667
#define DEATH_Killed_By_Kamikaze        668
#define DEATH_I_DIDNT_DIE_I_WAS_SALAVAGED 1004 //ship was salvaged!  So deal with gamestats accordingly
//non player depenedant
#define DEATH_Killed_By_Collision       777 //ship raninto another ship/asteroid/object and died
#define DEATH_Killed_By_Cloud           888 //Dust Cloud damage kills a ship
#define DEATH_Killed_By_Dead_Player     999 //player that did killing is dead and unreferenceable
#define DEATH_BY_GAME_END               1000 //game ended, ship was freed
#define DEATH_BY_RETIREMENT             1001 //ship was retired by player
#define DEATH_BY_AFTER_DEAD             1002 //ship self destructed after player was already dead
#define DEATH_Left_Behind_At_Hyperspace 1003 //ship didn't finish dokcing in time for hyperspace

//If howDidIDie isn't set with the above, it is assumed to have been POOFED out of existance.
//#define DEATH_By_Poofing                888 //somewheres in the code this ship was poofed out of existence.

//ship struct "specialflags" defines
#define SPECIAL_StopForResearchDocking              0x00000001
#define SPECIAL_Kamikaze                            0x00000002
#define SPECIAL_BrokenFormation                     0x00000004
#define SPECIAL_KamikazeCrazyFast                   0x00000008
#define SPECIAL_SpeedBurst                          0x00000010
#define SPECIAL_SpeedBurstCooling                   0x00000020
#define SPECIAL_BurstFiring                         0x00000040
#define SPECIAL_SinglePlayerWithinHyperSpaceRange   0x00000080
#define SPECIAL_SinglePlayerLimitSpeed              0x00000100
#define SPECIAL_BUSY_BECAUSE_SALVAGE_DOCKING        0x00000200
#define SPECIAL_Harvested                           0x00000400
#define SPECIAL_EXITING_DOOR                        0x00000800
#define SPECIAL_STAY_TILL_EXPLICITLAUNCH            0x00001000
#define SPECIAL_KasSpecialDamageModifier            0x00002000  // !!!FREE FLAG YOU CAN USE!!!///
#define SPECIAL_Finished_Resource                   0x00004000  //flag indicates when a resource collector has just finished collecting resources from one particular resource (computer player specific)
#define SPECIAL_Resourcing                          0x00008000  //flag indicates when a resource collector has started resourcing - computer player specific
#define SPECIAL_SinglePlayerInParade                0x00010000
#define SPECIAL_rowGettingOutOfWay                  0x00020000
#define SPECIAL_ATTACKMOVECANCEL                    0x00040000
#define SPECIAL_FriendlyStatus                      0x00080000  //set by KAS - player's ships don't autoretaliate against this ship.  Also allows you to dock with them.
#define SPECIAL_IsASalvager                         0x00100000  //used by junkyard dawg and salcap corvette...elminates double checks of the ship types!
#define SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse 0x00200000
#define SPECIAL_AttackFromAbove                     0x00400000
#define SPECIAL_SalvagedTargetGoingIntoDockWithShip 0x00800000
#define SPECIAL_SalvageTakingHomeTechnology         0x01000000
#define SPECIAL_Hyperspacing                        0x02000000
#define SPECIAL_ForcedAttackStatus                  0x04000000  //ship is in combat status, despite not actually attacking ships (must be turned off - doesn't expire)

#define SPECIAL_LaunchedFromKas                     0x10000000
#define SPECIAL_KasCheckDoneLaunching               0x20000000
#define SPECIAL_Launching                           0x40000000      //indicates ship is launching
#define SPECIAL_ParadeNeedTomoveCloser              0x80000000


//////////SPECIAL_2 Flags
#define SPECIAL_2_SalvageTargetAlreadyDriven        0x00000001
#define SPECIAL_2_ShipInGravwell                    0x00000002
#define SPECIAL_2_InUnitCaps                        0x00000004
#define SPECIAL_2_CircularGuard                     0x00000008
#define SPECIAL_2_NoMoreDockingAllowed              0x00000010
#define SPECIAL_2_DontPlowThroughEnemyShips         0x00000020
#define SPECIAL_2_BusiedDockPoint                   0x00000040
#define SPECIAL_2_DisabledForever                   0x00000080
				

struct Resource;

typedef struct Ship         // Ship object
{
    Node objlink;
    ObjType objtype;                // object type (ship or bullet, etc)
    udword flags;                   // flags whether the object is rotatable, etc.
    ShipStaticInfo *staticinfo;         // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;               // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;
    ClampInfo *clampInfo;

    SalvageInfo *salvageInfo;       //salvage info...

    //must come next to be compatible with Derelict in aiship stuff
    ubyte aistate;                  // AI state variable
    ubyte pad1[2];
    sbyte putOnDoor;
    struct Ship *dockingship;       // if non-NULL, indicates no collisions should occur with this ship
    udword specialFlags;            //general purpose ship flags, see above for defines
    udword specialFlags2;            //general purpose ship flags, see above for defines
    sdword salvageNumTagged[MAX_MULTIPLAYER_PLAYERS];        //number of salcaps vying for ship...used to disperese them amonst many targets
    NAVLightInfo *navLightInfo;
    // HERE UP SAME for Derelicts and Ships

    Node shiplink;
    vector moveTo;                  // move destination of the ship
    vector moveFrom;                // where the ship is moving from
    vector formationOffset;         // offset in formation
    vector formationHeading;        // heading in formation (optional)
    vector engineOffset;    // engine nozzle offset from position in object coordinates.
    vector enginePosition;  // engine position in universe
    struct Ship *rowGetOutOfWay;    // Right of way - ship to get out of way of
    vector rowOriginalPoint;        // Right of way - my original point
    vector rowOutOfWayPoint;        // Right of way - point to go to be out of way
    ShipType shiptype;
    ShipRace shiprace;
    //*******Tactics stuff start
    //FIX LATER to support fewer variables using bitmasks
    TacticsType tacticstype;        //ships current tactcis setting
    bool isDodging;                 //flag, true if ship is performing a dodge
    real32 DodgeTime;               //variable used to control when a ship will stop dodging
    uword  DodgeDir;
    uword  pad2;
    bool tacticsFormationVar1;      //variable for formation attacking
    udword DodgeFlag;               //info flag for dodging
    sdword tactics_ordertype;       //ships order type convulved into the Tactics Domain
    udword tacticsTalk;             //var used for intership comunication
    sdword tacticsNeedToFlightMan;  //same as above
    //*******Tactics stuff end
    sdword dockwaitforNorm;
    real32 timeCreated;
    sdword kamikazeState;
    vector kamikazeVector;
    real32 speedBurstTime;
    real32 singlePlayerSpeedLimiter;  // limiter of ships maxvelocity, single player game only!
    real32 deathtime;       // time at which this ship should blow up, if SOF_Crazy is set
    //mad animation variabes
    udword madAnimationFlags;       //general purpose flags for mad animations
    udword madGunStatus;            //gun status
    udword madWingStatus;           //wing animation status
    udword madDoorStatus;
    udword madSpecialStatus;
    udword cuedAnimationIndex;            //Animation index waiting in the wings to be played
    udword cuedAnimationType;      //cue of animation types (guns, wings, etc...)
    udword nextAnim;
    udword shaneyPooSoundEventAnimationTypeFlag;    //opening,closing,damaged_opening,damaged_closing
    sdword forceCancelDock;
    real32 gravwellTimeEffect;      //time the gravwell effect sould be played again
    //
    struct Player *playerowner;
    sdword resources;
    real32 fuel;
    real32 lastTimeRepaired;
    real32 magnitudeSquared;        // most recent scaling factor
    struct Ship *gettingrocked;     // ship which is rocking you (shooting you)
    struct Ship *recentAttacker;    // ship which has attacked you recently (ie, not just this AI tick) -- see recentlyAttacked
    struct Ship *firingAtUs;        // ship which has recently fired upon us (but needn't have hit) see recentlyFiredUpon
    bool8 shipidle;                 // flag to indicate if ship has no command to execute
    bool8 autostabilizeship;        // if TRUE, we should stabilize the ship
    ubyte autostabilizestate;       // state of autostabilization
    ubyte autostabilizewait;        // how many frames to wait before stabilizing in autostabilize
    ubyte aistateattack;            // AI attack state variable
    ubyte flightmanState1;          // generic state variables for flight maneuvers
    ubyte flightmanState2;          // generic state variables for flight maneuvers
    ubyte rcstate1;                 // state variable for resource collecting
    ubyte rcstate2;                 // state variable for resource collecting
    ubyte aistatecommand;
    sbyte dontrotateever;
    sbyte dontapplyforceever;
    bool8 shipisattacking;
    sbyte colorScheme;              //what color scheme does this ship use?
    ubyte recentlyAttacked;         // counts down from RECENT_ATTACK_DURATION -- see recentAttacker
    ubyte recentlyFiredUpon;        // same, see firingAtUs
    ubyte chatterBits;              // flags for battle chatter
    ubyte rowState;                 // state variable for right of way
    real32 aidescend;               // AI variable for capital ships descending
    udword flightman;               // current flight maneuver handle
    void *flightmanInfo;            // pointer to any additional flight maneuver info
    struct CommandToDo *formationcommand;   // pointer to formationcommand, if ship in formation
    struct CommandToDo *command;    // pointer to ship's current command, if any
    SpecificAttackVars attackvars;  // attack information
    SpecificDockVars dockvars;      // dock information
    struct Effect *rceffect;        // effect variable for resource collecting
    void (*ShipXHarvestsResourceY)(struct Ship *ship,struct Resource *resource);    // function pointer for resource collecting
    sdword visibleToWho;              //bitmask flag indicating to which players this ship is visible to...relevant only if ship is cloaked
    sdword visibleToWhoPreviousFrame; //same as above, but relevant for the previous univupdate frame
    sdword howDidIDie;              //defined above: Set when a ship is set to die, that way, at somepoint (hopefully) we can tell how the bitch died.
    sdword whoKilledMe;             //playerIndex of the player that killed this ship
    udword nDamageLights;
    DamageLightStatic* damageLights;
    struct Effect* showingDamage[3][3];
    vector holdingPatternPos;
    real32 damageModifier;
    real32 passiveAttackCancelTimer;

    //later will make most of these into a single flag
    udword tractorbeaminfo;
    udword tractorvector;
    struct Player *tractorbeam_playerowner;
    sdword tractorstate;            //info variable for tractorbeaming state
    sdword tractorPosinfo[4];        //more tractor info!
    sdword hyperspacePing;          //true or false flag indicating if the ship has had a hyperspace ping created for it...NON-deterministic!

    sdword needNewLeader;
    struct Ship *newDockWithTransfer;
    sdword newDockIndexTransfer;

    real32 shipDeCloakTime;         //time is set when a ship decloaks...this time is used to give decloaking ships an advantage
    shiptrail* trail[MAX_NUM_TRAILS];   //ship's engine trail(s)
    ShipID shipID;
    uword hotKeyGroup;              // what hot-key group this ship belongs to
    sdword retaliateSpeechVar;
    real32 retaliateSpeechTime;

    SOUNDEVENT soundevent;
    Node slavelink;                 //Nodal link for slavery info
    SlaveInfo *slaveinfo;           //pointer to struct of info regarding ships slavery status
    GunInfo *gunInfo;
    DockInfo *dockInfo;
    ShipsInsideMe *shipsInsideMe;
    shipbindings *bindings;         //ship hierarchy bindings
    madanim *madBindings;
//    RepairDroid *pRepairDroids;     // Alex's repair droid stuff...
    struct ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo;     // stuff for the single player game
    void* lightning[2];             //lightning effect stuff
    udword sizeofShipSpecifics;
    udword ShipSpecifics[1];        //this member must go last
} Ship;

typedef Ship *ShipPtr;

/*-----------------------------------------------------------------------------
    Missile Object
-----------------------------------------------------------------------------*/

typedef struct MissileID
{
    uword missileNumber;
} MissileID;

typedef struct
{
    Node objlink;
    ObjType objtype;                            // object type (ship or bullet, etc)
    udword flags;                               // flags whether the object is rotatable, etc.
    MissileStaticInfo *staticinfo;              // pointer to static info
    Node renderlink;                            // link for rendering list
    ubyte currentLOD;                           // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;               // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;                // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;                    // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;
    ClampInfo *clampInfo;

    //missile trail stuff
    missiletrail* trail;
    vector enginePosition;

    Node missilelink;
    MissileType missileType;
    GunSoundType soundType;
    ShipRace race;
    real32 maxtraveldist;
    real32 timelived;
    real32 totallifetime;
    struct Player *playerowner;
    Ship *owner;
    SpaceObjRotImpTarg *target;
    etglod *hitEffect;                          //effect to play on impact
    real32 damage;
    sdword haveCheckedForLoss;
    MissileID missileID;
    uword pad2;
    udword mineAIState;                         //ai state of missile MINES
    vector formation_position;                  //vector that records the position the mine WANTS to be in while in formation
    sdword FORCE_DROPPED;                       //flag..true if force_dropped...used for variable selection
    Node formationLink;
    MineFormationInfo *formationinfo;
    sdword formation_number_X;
    sdword formation_number_Y;
    ubyte colorScheme;
    ubyte pad[3];
} Missile;

typedef Missile *MissilePtr;

/*-----------------------------------------------------------------------------
    Effect Object
-----------------------------------------------------------------------------*/

typedef struct Effect
{
    Node objlink;
    ObjType objtype;                            // object type (ship or bullet, etc)
    udword flags;                               // flags whether the object is rotatable, etc.
    StaticInfo *staticinfo;                     // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;                           // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;               // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;                    // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;

    //... effect-specific stuff
    Node effectLink;
    udword effectFlags;                         //effects-specific flags
    Ship *owner;                                //owner of this effect (e.g. ship that blew up)
    real32 magnitudeSquared;                    //N-lips scale factor, if any
    sdword nSpawnedAttachedToOwner;             //number of spawned effects attached to this effect's owner
    real32 timeElapsed;                         //age of effect (can be negative for tim-offset effects)
    vector LoF;                                 //line-of-fire vector
    vector velocity;                            //velocity vector
    real32 theta;                               //R-plane rotation vector

    real32 drag;                                //drag for moving entire effect

    real32 spin;                                //control the spin of the effect
    real32 deltaSpin;

    ubyte *variable;                            //variable block
    ubyte *variableRate;                        //variable rate block (effectors)
    ubyte *effectID;                            //effector ID block for variables
//    udword *alternate;                          //list of alternates
    sdword nParticleBlocks;                     //max number of particle systems
    sdword iParticleBlock;                      //current number of particle systems
    ubyte *particleBlock[1];                    //ragged array of particle systems in effect
} Effect;

/*-----------------------------------------------------------------------------
    Bullet Object
-----------------------------------------------------------------------------*/

typedef struct                                  // Bullet object
{
    Node objlink;
    ObjType objtype;                            // object type (ship or bullet, etc)
    udword flags;                               // flags whether the object is rotatable, etc.
    StaticInfo *staticinfo;                     // pointer to static info
    Node renderlink;                            // link for rendering list
    ubyte currentLOD;                           // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;               // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;                // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;                    // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;

    Node bulletlink;
    vector bulletheading;                       // heading in universe
    vector lengthvec;
    real32 lengthmag;
    real32 traveldist;
    real32 beamtraveldist;                      // only used for BULLET_Beam
    BulletType bulletType;
    GunSoundType soundType;
    real32 timelived;
    real32 totallifetime;
    Ship *owner;                                // pointer to bullet's owner
    Gun *gunowner;                              // pointer to bullet's gun from which it was fired
    SpaceObjRotImpTarg *target;                 // target of bullet
    color bulletColor;                          // race of player who shot bullet
    real32 bulletmass;
    etglod *hitEffect;                          //effect to play when it hits
    Effect *effect;                             //effect to play as it flies
    real32 damage;                              //current damage of bullet
    real32 damageFull;                          //unadjusted (initial) damage
    udword SpecialEffectFlag;                   //Used by special ships to see if they have affected this bullet
    real32 DFGFieldEntryTime;                   //used by DFGF.
    real32 BulletSpeed;
    struct Player *playerowner;
} Bullet;

typedef Bullet *BulletPtr;

/*=============================================================================
    Derelict
=============================================================================*/

typedef struct DerelictID
{
    uword derelictNumber;
} DerelictID;

typedef struct
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    DerelictStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;
    ClampInfo *clampInfo;

    SalvageInfo *salvageInfo;       //salvage info...

    //must come next to be compatible with Derelict in aiship stuff
    ubyte aistate;                  // AI state variable
    ubyte pad1[2];
    sbyte putOnDoor;
    struct Ship *dockingship;       //Used to trick collision detection when being sent into docking bays
    udword specialFlags;            //general purpose ship flags, see above for defines
    udword specialFlags2;           //more general purpose ship flags, see above for defines
    sdword salvageNumTagged[MAX_MULTIPLAYER_PLAYERS];        //number of salcaps vying for ship...used to disperese them amonst many targets
    NAVLightInfo *navLightInfo;
    // HERE UP SAME for Derelicts and Ships

    Node derelictlink;
    DerelictID derelictID;
    sbyte colorScheme;              //what color scheme does this ship use?
    ubyte pad[3];

    sdword ambientSoundHandle;      //used by sound engine to keep track of the ambient sound the derelict makes
    sdword randomSoundHandle;       //used by sound engine to keep track of the random ambient sound the derelict makes
    real32 nextRandomTime;          //time to play the next random ambient sound

    sdword TractorTrue;             //used to stop friction on derelicts when being pushed
    udword tractorbeaminfo;         //counter for # of tractor beams on derelict
    udword tractorvector;
    struct Player *tractorbeam_playerowner;
    sdword tractorstate;            //info variable for tractorbeaming state
    sdword tractorPosinfo[4];        //more tractor info!
    sdword needNewLeader;
    struct Ship *newDockWithTransfer;
    sdword newDockIndexTransfer;
    real32 creationTime;
    DerelictType derelicttype;
} Derelict;

typedef Derelict *DerelictPtr;

/*=============================================================================
    Resource Objects
=============================================================================*/

typedef struct ResourceID
{
    uword resourceNumber;
} ResourceID;

/*-----------------------------------------------------------------------------
    Generic Resource, from which all resources (asteroids, nebula, etc. are derived)
-----------------------------------------------------------------------------*/

typedef struct Resource
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to
} Resource;

typedef Resource *ResourcePtr;    // all resources are assumed to be rotatable, and impactable

/*-----------------------------------------------------------------------------
    Asteroid
-----------------------------------------------------------------------------*/

typedef struct Asteroid
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to

    AsteroidType asteroidtype;  // asteroid specific information
    real32 scaling;
} Asteroid;

/*-----------------------------------------------------------------------------
    Nebula
-----------------------------------------------------------------------------*/

typedef struct Nebula
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to

    // put nebula specific stuff here only
    NebulaType nebulatype;
    real32 scaling;
    nebChunk* stub;
    sdword nebNebulaeIndex;
} Nebula;

/*-----------------------------------------------------------------------------
    Gas Cloud
-----------------------------------------------------------------------------*/

typedef struct GasCloud
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to

    // put Gas Cloud pecific stuff here only
    GasCloudType gascloudtype;  // gascloud specific information
    real32 scaling;
} GasCloud;

/*-----------------------------------------------------------------------------
    Dust Cloud
-----------------------------------------------------------------------------*/

typedef struct DustCloud
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to

    // Dust Cloud specific stuff
    DustCloudType dustcloudtype;
    real32 scaling;
    cloudSystem* stub;
} DustCloud;

#define MAX_NUM_DROP    5        //maximum number of targets that can be dropped at a motherships/carrier


#ifdef GOD_LIKE_SYNC_CHECKING

typedef struct
{
    udword univUpdateCounterValue;
    udword numShips;    //number of ships in this snap shot
    Ship ship[1];       //dynamic array of ships
}UniverseSnapShot;

typedef struct
{
    char playerName[20];    //name of curplayer
    sdword windowSize;      //number of universeSnapShots in this file
    UniverseSnapShot *universeSnapShot[1]; //array of snapshots
}GodWritePacketHeader;

extern GodWritePacketHeader *godUniverseSnapShotHeader;

#define sizeofGodHeader(n)      sizeof(GodWritePacketHeader) + sizeof(UniverseSnapShot *)*(n-1)
#define sizeofSnapShot(n)       sizeof(UniverseSnapShot)+ sizeof(Ship)*(n)



#endif

/*=============================================================================
    Macros:
=============================================================================*/

#define sizeofGunInfo(n) (sizeof(GunInfo) + (n-1)*sizeof(Gun))
#define sizeofGunStaticInfo(n) (sizeof(GunStaticInfo) + (n-1)*sizeof(GunStatic))
#define sizeofDockStaticInfo(n) (sizeof(DockStaticInfo) + (n-1)*sizeof(DockStaticPoint))
#define sizeofNavLightStaticInfo(n) (sizeof(NAVLightStaticInfo) + (n-1)*sizeof(NAVLightStatic))
#define sizeofDockInfo(n) (sizeof(DockInfo) + (n-1)*sizeof(DockPoint))
#define sizeofNavLightInfo(n) (sizeof(NAVLightInfo) + (n-1)*sizeof(NAVLight))

//#define isCapitalShip(ship) (((ShipStaticInfo *)((ship)##->staticinfo))->shipclass <= CLASS_Frigate)
//#define isCapitalShipStatic(shipstatic) (shipstatic##->shipclass <= CLASS_Frigate)
#define isCapitalShip(ship) (((ShipStaticInfo *)((ship)->staticinfo))->shipIsCapital)
#define isCapitalShipStatic(shipstatic) ((shipstatic)->shipIsCapital)

#define isShipOfClass(ship,checkclass) (((ShipStaticInfo *)((ship)->staticinfo))->shipclass == (checkclass))

/*=============================================================================
    Support Functions:
=============================================================================*/

#endif //___SPACEOBJ

