#define AdvanceSupportFrigate           0
#define AttackBomber                    1
#define Carrier                         2
#define CloakedFighter                  3
#define CloakGenerator                  4
#define DDDFrigate                      5
#define DefenseFighter                  6
#define DFGFrigate                      7
#define GravWellGenerator               8
#define HeavyCorvette                   9
#define HeavyCruiser                    10
#define HeavyDefender                   11
#define HeavyInterceptor                12
#define IonCannonFrigate                13
#define LightCorvette                   14
#define LightDefender                   15
#define LightInterceptor                16
#define MinelayerCorvette               17
#define MissileDestroyer                18
#define Mothership                      19
#define MultiGunCorvette                20
#define Probe                           21
#define ProximitySensor                 22
#define RepairCorvette                  23
#define ResearchShip                    24
#define ResourceCollector               25
#define ResourceController              26
#define SalCapCorvette                  27
#define SensorArray                     28
#define StandardDestroyer               29
#define StandardFrigate                 30
#define Drone                           31      // Special ship of R1
#define TargetDrone                     32
#define HeadShotAsteroid                33      // Headshot asteroid is now a ship!
#define CryoTray                        34      // CryoTray is now a ship!
#define P1Fighter                       35
#define P1IonArrayFrigate               36
#define P1MissileCorvette               37
#define P1Mothership                    38
#define P1StandardCorvette              39
#define P2AdvanceSwarmer                40
#define P2FuelPod                       41
#define P2Mothership                    42
#define P2MultiBeamFrigate              43
#define P2Swarmer                       44
#define P3Destroyer                     45
#define P3Frigate                       46
#define P3Megaship                      47
#define FloatingCity                    48
#define CargoBarge                      49
#define MiningBase                      50
#define ResearchStation                 51
#define JunkYardDawg                    52
#define JunkYardHQ                      53
#define Ghostship                       54
#define Junk_LGun                       55
#define Junk_SGun                       56
#define ResearchStationBridge           57
#define ResearchStationTower            58

#define STD_FIRST_SHIP                  AdvanceSupportFrigate
#define STD_LAST_SHIP                   CryoTray

#define P1_FIRST_SHIP                   P1Fighter
#define P1_LAST_SHIP                    P1StandardCorvette

#define P2_FIRST_SHIP                   P2AdvanceSwarmer
#define P2_LAST_SHIP                    P2Swarmer

#define P3_FIRST_SHIP                   P3Destroyer
#define P3_LAST_SHIP                    P3Megaship

#define TRADERS_FIRST_SHIP              FloatingCity
#define TRADERS_LAST_SHIP               ResearchStationTower

#define DefaultShip                     (TRADERS_LAST_SHIP+1)    // not really a ship, just a flag indicating we should use Default ship behaviour
#define TOTAL_NUM_SHIPS                 (TRADERS_LAST_SHIP+1)

#define TOTAL_STD_SHIPS                 (STD_LAST_SHIP - STD_FIRST_SHIP + 1)
#define TOTAL_P1_SHIPS                  (P1_LAST_SHIP - P1_FIRST_SHIP + 1)
#define TOTAL_P2_SHIPS                  (P2_LAST_SHIP - P2_FIRST_SHIP + 1)
#define TOTAL_P3_SHIPS                  (P3_LAST_SHIP - P3_FIRST_SHIP + 1)
#define TOTAL_TRADERS_SHIPS             (TRADERS_LAST_SHIP - TRADERS_FIRST_SHIP + 1)

#define TOTAL_OTHER_SHIPS   (TOTAL_P1_SHIPS + TOTAL_P2_SHIPS + TOTAL_P3_SHIPS + TOTAL_TRADERS_SHIPS)

