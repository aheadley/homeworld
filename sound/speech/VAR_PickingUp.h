/* VAR_Motion */
// capships, capship, strike
#define		Motion_Approaching		0
#define		Motion_Advancing		1
#define		Motino_Withdrawing		2
#define		Motino_Retreating		3
#define		Motion_Holding			4
#define		Motion_Maintaining		5
#define		Motion_Motionless		6

/* VAR_Roger */
#define		RogerThat		0
#define		CopyThat		1
#define		Roger			2
#define		Copy			3
#define		Acknowledged	4
#define		Confirmed		5

/* VAR_Damage */
#define		Damage_Green	0
#define		Damage_Yellow	1
#define		Damage_Red		2

/* VAR_Resources */
#define		Res_Asteroids		0
#define		Res_Nebulae			1
#define		Res_GasClouds		2
#define		Res_DustClouds		3

/* pickingup, capshipsand, capshipand, ID, Numberlist */
/* VAR_Percent */
/* VAR_Degrees */
/* VAR_Numbers */
#define		Zero		0
#define		One			1
#define		Two			2
#define		Three		3
#define		Four		4
#define		Five		5
#define		Six			6
#define		Seven		7
#define		Eight		8
#define		Niner		9
#define		Ten			10
#define		Eleven		11

/* VAR_Letters */
#define		Alpha		0
#define		Bravo		1
#define		Charlie		2
#define		Delta		3
#define		Echo		4
#define		Foxtrot		5
#define		Golf		6
#define		Hotel		7
#define		India		8
#define		Juliette	9
#define		Kilo		10
#define		Lima		11
#define		Mike		12
#define		November	13
#define		Oscar		14
#define		Papa		15
#define		Quebec		16
#define		Romeo		17
#define		Sierra		18
#define		Tango		19
#define		Uniform		20
#define		Victor		21
#define		Whiskey		22
#define		Xray		23
#define		Yankee		24
#define		Zulu		25

/* VAR_Ships */
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
#define Drone                           31
#define TargetDrone                     32
#define P1Fighter                       33
#define P1IonArrayFrigate               34
#define P1MissileCorvette               35
#define P1Mothership                    36
#define P1StandardCorvette              37
#define P2AdvanceSwarmer                38
#define P2FuelPod                       39
#define P2Mothership                    40
#define P2MultiBeamFrigate              41
#define P2Swarmer                       42
#define P3Destroyer                     43
#define P3Frigate                       44
#define P3Megaship                      45
#define FloatingCity                    46
#define CargoBarge                      47
#define MiningBase                      48
#define ResearchStation                 49
#define JunkYardDawg                    50

/* VAR_Class */
#define		CLASS_Mothership		0
#define 	CLASS_HeavyCruiser		1
#define 	CLASS_Carrier			2
#define 	CLASS_Destroyer			3
#define 	CLASS_Frigate			4
#define 	CLASS_Corvette			5
#define 	CLASS_Fighter			6
#define 	CLASS_Resource			7
#define 	CLASS_NonCombat			8
#define		CLASS_Research			9
#define		CLASS_Sensor			10
#define		CLASS_Controller		11
#define 	NUM_CLASSES			(CLASS_NonCombat + 1)


/* VAR_Unknown */
#define		Unknown_Ship			0
#define		Unknown_Craft			1
#define		Unknown_Spacecraft		2
#define		Unknown_Spaceship		3

/* VAR_Formations */
#define		Formation_Delta			0
#define		Formation_Broad			1
#define		Formation_X				2
#define		Formation_Claw			3
#define		Formation_Wall			4
#define		Formation_Sphere		5
#define		Formation_Picket		6
#define		Formation_Custom		7


NewAlloys	4) NEW Fighter Production
MassDrive1Kt    5) NEW Fighter Drive              
6) NEW Cloaked Fighter             
7) NEW Defense Fighter            
TargetingSystems	8) NEW Fast-tracking Turret 
PlasmaWeapons	9) NEW Plasma Bomb Launcher 
Chassis1	10) NEW Heavy Fighter Chassis
MassDrive10Kt	11) NEW Corvette Drive
MediumGuns	12) NEW Heavy Corvette Upgrade     
MineLayerTech	13) NEW Minelaying Tech 
Chassis2	14) NEW Corvette Chassis 
MassDrive100Kt	15) NEW Capital Ship Drive         
FireControl	16) NEW Defender Sub-System       
SupportRefuelTech	17) NEW Corvette Production        
IonWeapons	18) NEW Ion Cannon                
19) NEW Drone Technology           
20) NEW Defense Field              
Chassis3	21) NEW Capital Ship Chassis       
MassDrive1Mt	22) NEW Super-capital Ship Drive   
MissileWeapons	23) NEW Guided Missile            
HeavyGuns	24) NEW Heavy Gun                 
ProximityDetector	25) NEW Proximity Detector         
SensorsArrayTech	26) NEW Sensors Array              
GravityWellGeneratorTech	27) NEW Gravity Generator          
CloakGeneratorTech	28) NEW Cloak Generator            
RepairTech	29) NEW Super-heavy Chassis        
SalvageTech	30) NEW Capital Ship Production    


    CoolingSystems,
    CloakDefenseFighter,
    AdvancedCoolingSystems,
    AdvanceTacticalSupport,
    IonWeapons,
    DDDFDFGFTech,
    AdvancedFireControl,
    ConstructionTech,



; Strings for names of technologies
strCloakFighter                         CLOAKED FIGHTER
strDefenseFighterTech                   DEFENSE FIGHTER
strAdvancedCoolingSystems               ADVANCED COOLING SYSTEMS
strAdvanceTacticalSupport               STRIKE CRAFT SUPPORT
strDDDFTech                             DRONE TECHNOLOGY
strDFGFTech                             DEFENSE FIELD
strAdvancedFireControl                  ADVANCED TARGETING
strConstructionTech                     CARRIER TECH
strSalvageTech                          CAPITAL SHIP PRODUCTION

/* VAR_Race */
#define		R1			0
#define 	R2			1
#define 	P1			2
#define 	P2			3
#define 	P3			4
#define 	Traders		5
#define 	NUM_RACES	(Traders + 1)




