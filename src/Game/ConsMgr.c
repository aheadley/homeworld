/*=============================================================================
    Name    : ConsMgr.c
    Purpose : Logic for the Construction Manager

    Created 7/18/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include "Types.h"
#include "LinkedList.h"
#include "Universe.h"
#include "Region.h"
#include "UIControls.h"
#include "FEFlow.h"
#include "font.h"
#include "FontReg.h"
#include "ObjTypes.h"
#include "Task.h"
#include "mouse.h"
#include "MultiplayerGame.h"
#include "CommandLayer.h"
#include "PiePlate.h"
#include "ConsMgr.h"
#include "Globals.h"
#include "CommandWrap.h"
#include "Scroller.h"
#include "SoundEvent.h"
#include "Randy.h"
#include "Strings.h"
#include "ResearchAPI.h"
#include "mainrgn.h"
#include "TaskBar.h"
#include "ShipView.h"
#include "glinc.h"
#include "glcaps.h"
#include "render.h"
#include "mainrgn.h"
#include "Sensors.h"
#include "SinglePlayer.h"
#include "Ping.h"
#include "texreg.h"
#include "FEReg.h"
#include "ShipDefs.h"
#include "NIS.h"
#include "Tutor.h"
#include "Options.h"
#include "FEColour.h"
#include "glcompat.h"
#include "InfoOverlay.h"
#include "SaveGame.h"

/*=============================================================================
    Definitions:
=============================================================================*/
#ifdef _WIN32
#define CM_FIBFile              "FEMan\\Construction_Manager.fib"
#else
#define CM_FIBFile              "FEMan/Construction_Manager.fib"
#endif
#define CM_ConstructionScreen   "Construction_manager"
//#define CM_BuildScreen          "Build_screen"
#define CM_DefaultFont          "default.hff"
#define CM_ShipListFont         "HW_EuroseCond_11.hff"
#define CM_FontNameLength       64
#define CM_HeadingColorFactor   1.5f

//margins for printing RU's
#define CM_RUMarginRight        2
#define CM_RUMarginTop          2

//margins for available ships
#define CM_ASHeadingMarginLeft  3
#define CM_ASMarginLeft         9
#define CM_ASMarginTop          2
#define CM_ASMarginRight        2
#define CM_ASInterSpacing       2

#define CM_NumberJobsPerShip    128
#define CM_NumberJobsPerClass   10


#define CM_PausedColor          colRGB(80,120,80)
#define CM_SelectedColorSolid   colRGB(60,030,0)
#define CM_SelectedColorOutline colRGB(120,070,0)
#define CM_BackgroundColor      colRGB(0,35,79)
#define CM_SelectBoxX0          0
#define CM_SelectBoxX1          190
#define CM_SelectBoxY0          0
#define CM_SelectBoxY1          12

sdword cmSelectTopAdjust = 0;
sdword cmSelectBottomAdjust = 0;
sdword cmSelectLeftAdjust = 0;
sdword cmSelectRightAdjust = 0;

#define CM_FIVEKEY              CONTROLKEY      //hold to increase/decrease by 5

//task parameters
#define CM_TaskStackSize        4096
#define CM_TaskServicePeriod      UNIVERSE_UPDATE_PERIOD

//parameter defaults
#define CM_SelectionTextColor     colRGB(255, 255, 105)
#define CM_BuildingTextColor      colRGB(0, 255, 0)
#define CM_SelectionRectColor     colRGB(50, 120, 20)
#define CM_ProgressShipColor      colRGB(100, 100, 55)
#define CM_ProgressTotalColor     colRGB(180, 180, 80)
#define CM_StandardTextColor      colRGB(0,110,172)
#define CM_ClassHeadingTextColor  colRGB(255, 200, 0)
#define CM_CapsExceededTextColor  colRGB(255,25,25)
#define CM_RefundRatio          39321

#define CM_FloatRefundRatio     1.0f

// defines for shipavailable status word

#define ITEM_CLASS          0
#define ITEM_SHIP           1

#define STAT_CANBUILD       0
#define STAT_CANTBUILD      1
#define STAT_PRINT          2
#define STAT_DONTPRINT      3

// frame count for red flash when ship limits exceeded
#define CAPS_REDFRAMECOUNT 5;

//margins for printing construction progress
#define CM_ProgressMarginLeft   2
#define CM_ProgressMarginRight  90
#define CM_ProgressMarginTop    2
#define CM_ProgressInterSpacing 2
#define CM_SHIP_IMAGE_INSET     4

#define NUM_CMTEXTURES          2

#define NUM_CMCARRIERS          4

#define UNIVERSE_TURBO_REPEAT       19

#define CM_MaxJobsMothership    2
#define CM_MaxJobsHeavyCruiser  2
#define CM_MaxJobsCarrier       2
#define CM_MaxJobsDestroyer     2
#define CM_MaxJobsFrigate       2
#define CM_MaxJobsCorvette      2
#define CM_MaxJobsFighter       3
#define CM_MaxJobsResource      2
#define CM_MaxJobsNonCombat     2

/*=============================================================================
    Data:
=============================================================================*/
static sdword cmRenderEverythingCounter;

bool cmPaletted;

#if CM_CHEAP_SHIPS
bool cmCheapShips = FALSE;
#endif

void cmDeterministicStartup(void);
void cmDeterministicShutdown(void);

//callback tables for this screen
void cmClose(char *string, featom *atom);
void cmCancelJobs(char *string, featom *atom);
void cmPauseJobs(char *string, featom *atom);
void cmBuildShips(char *string, featom *atom);
void cmNumberRUsDraw(featom *atom, regionhandle region);
void cmShipCostsDraw(featom *atom, regionhandle region);
void cmShipInfoDraw(featom *atom, regionhandle region);
void cmShipNumbersDraw(featom *atom, regionhandle region);
void cmTotalRUsDraw(featom *atom, regionhandle region);
void cmTotalShipsDraw(featom *atom, regionhandle region);
void cmConnectorDraw(featom *atom, regionhandle region);
void cmMotherShipDraw(featom *atom, regionhandle region);
void cmCarrier1Draw(featom *atom, regionhandle region);
void cmCarrier2Draw(featom *atom, regionhandle region);
void cmCarrier3Draw(featom *atom, regionhandle region);
void cmCarrier4Draw(featom *atom, regionhandle region);
void cmLeftArrowDraw(featom *atom, regionhandle region);
void cmRightArrowDraw(featom *atom, regionhandle region);

void cmScroller(char *string, featom *atom);

void cmFillInCarrierXs(void);

ShipType cmKeyToShipType(uword key);

LinkedList listofShipsInProgress;

fecallback cmCallback[] =
{
    {cmClose,               "CM_Close"      },
    {cmBuildShips,          "CM_BuildShips" },
    {cmCancelJobs,          "CM_CancelJobs" },
    {cmPauseJobs,           "CM_PauseJobs"  },
    {cmScroller,            "CM_Scroller"   },
    {NULL,                  NULL            }
};
fedrawcallback cmDrawCallback[] =
{
    {cmNumberRUsDraw,           "CM_RUs"                },
    {cmShipCostsDraw,           "CM_ShipCost"           },
    {cmShipInfoDraw,            "CM_Ships"              },
    {cmShipNumbersDraw,         "CM_ShipNumbers"        },
    {cmTotalRUsDraw,            "CM_TotalCost"          },
    {cmTotalShipsDraw,          "CM_TotalShips"         },
    {cmConnectorDraw,           "CM_ConnectorDraw"      },
    {cmMotherShipDraw,          "CM_MotherShipDraw"     },
    {cmCarrier1Draw,            "CM_Carrier1Draw"       },
    {cmCarrier2Draw,            "CM_Carrier2Draw"       },
    {cmCarrier3Draw,            "CM_Carrier3Draw"       },
    {cmCarrier4Draw,            "CM_Carrier4Draw"       },
    {cmLeftArrowDraw,           "CM_LeftArrowDraw"      },
    {cmRightArrowDraw,          "CM_RightArrowDraw"     },
    {NULL,                      NULL                    }
};

extern ShipType svShipType;

KeysToShips cmShipTypes[]=
{
//    {AKEY|CM_SHIFT,  AdvanceSupportFrigate},
    {F5KEY,          AttackBomber},
//    {KKEY,           Carrier},
    {F4KEY,           CloakedFighter},
//    {LKEY|CM_SHIFT,  CloakGenerator},
//    {UKEY|CM_SHIFT,  DDDFrigate},
    {F4KEY,          DefenseFighter},
//    {UKEY,           DFGFrigate},
//    {GKEY,           GravWellGenerator},
    {F7KEY,          HeavyCorvette},
//    {ZKEY,           HeavyCruiser},
    {F1KEY,          HeavyDefender},
    {F3KEY,          HeavyInterceptor},
//    {OKEY,           IonCannonFrigate},
    {F6KEY,          LightCorvette},
//    {DKEY,           LightDefender},
    {F2KEY,          LightInterceptor},
    {F10KEY,         MinelayerCorvette},
//    {MKEY|CM_SHIFT,  MissileDestroyer},
  //{KEY ,           Mothership},
    {F11KEY,         MultiGunCorvette},
//    {WKEY,           Probe},
//    {XKEY|CM_SHIFT,  ProximitySensor},
    {F8KEY,          RepairCorvette},
//    {XKEY,           ResearchShip},
    {F12KEY,         ResourceCollector},
//    {RKEY|CM_SHIFT,  ResourceController},
    {F9KEY,          SalCapCorvette},
//    {SKEY|CM_SHIFT,  SensorArray},
//    {YKEY,           StandardDestroyer},
//    {EKEY,           StandardFrigate},
//    {TKEY,           Drone},
//    {TKEY|CM_SHIFT,  TargetDrone},
  /*{KEY ,           P1Fighter},
    {KEY ,           P1IonArrayFrigate},
    {KEY ,           P1MissileCorvette},
    {KEY ,           P1Mothership},
    {KEY ,           P1StandardCorvette},
    {KEY ,           P2AdvanceSwarmer},
    {KEY ,           P2FuelPod},
    {KEY ,           P2Mothership},
    {KEY ,           P2MultiBeamFrigate},
    {KEY ,           P2Swarmer},
    {KEY ,           P3Destroyer},
    {KEY ,           P3Frigate},
    {KEY ,           P3Megaship},
    {KEY ,           FloatingCity},
    {KEY ,           CargoBarge},
    {KEY ,           MiningBase},
    {KEY ,           ResearchStation},
    {KEY ,           JunkYardDawg}
    {KEY ,           JunkYardHQ}
   */

};

bool cmPrintHotKey = FALSE;



//index of first displayed element
sword cmUpperIndex = 0;
//maximum number of displayable elements in our list
uword cmMaxIndex = 18;

//For drawing accelerator arrows
sdword cmArrowIndex = -1;

static scrollbarhandle g_shandle;

//flag indicating loaded status of this screen
fibfileheader *cmScreensHandle = NULL;

//handle to construction task
taskhandle cmBuildTask = -1;

#define NSHIPS_AVAILABLE_R1REG  22 + 5          // Modified by Daly on 04.21.99 (was 20 + 5)
#define NSHIPS_AVAILABLE_R1BIG  27 + 6

#define NSHIPS_AVAILABLE_R2REG  22 + 5          // Modified by Daly on 04.21.99 (was 20 + 5)
#define NSHIPS_AVAILABLE_R2BIG  27 + 6

#define NSHIPS_AVAILABLE_P1     4 + 3
#define NSHIPS_AVAILABLE_P2     4 + 2

regionhandle cmBaseRegion        = NULL;
regionhandle cmShipInfoRegion    = NULL;
regionhandle cmShipNumbersRegion = NULL;
regionhandle cmNumberRUsRegion   = NULL;
regionhandle cmTotalRUsRegion    = NULL;
regionhandle cmTotalShipsRegion  = NULL;
regionhandle cmShipCostsRegion   = NULL;
regionhandle cmConnectorRegion   = NULL;
regionhandle cmMotherShipRegion  = NULL;
regionhandle cmCarrierRegions[NUM_CMCARRIERS] = { NULL, NULL, NULL, NULL };

/*
typedef struct
{
    sdword   disabled;
    ShipType type;
} shipdisabled;

//use [0,1], ![FALSE,TRUE] for disabling
shipdisabled cmShipsDisabled[] =
{
    {0, AdvanceSupportFrigate},
    {1, AttackBomber},
    {0, Carrier},
    {0, CloakedFighter},
    {1, CloakGenerator},
    {1, DDDFrigate},
    {0, DefenseFighter},
    {1, DFGFrigate},
    {1, GravWellGenerator},
    {0, HeavyCorvette},
    {1, HeavyCruiser},
    {0, HeavyDefender},
    {0, HeavyInterceptor},
    {0, IonCannonFrigate},
    {0, LightCorvette},
    {0, LightDefender},
    {0, LightInterceptor},
    {1, MinelayerCorvette},
    {1, MissileDestroyer},
    {1, MultiGunCorvette},
    {0, Probe},
    {0, ProximitySensor},
    {0, RepairCorvette},
    {0, ResearchShip},
    {0, ResourceCollector},
    {0, ResourceController},
    {1, SalCapCorvette},
    {0, SensorArray},
    {0, StandardDestroyer},
    {0, StandardFrigate},
    {0, Drone},
    {-1, 0},
};
*/

//ships available for construction
shipavailable cmShipsAvailableR1Reg[NSHIPS_AVAILABLE_R1REG + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyDefender       ,  NULL},
    {0,ITEM_SHIP ,0,  LightInterceptor    ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyInterceptor    ,  NULL},
    {0,ITEM_SHIP ,0,  CloakedFighter      ,  NULL},
    {0,ITEM_SHIP ,0,  AttackBomber        ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  LightCorvette       ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyCorvette       ,  NULL},
    {0,ITEM_SHIP ,0,  RepairCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  SalCapCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  MinelayerCorvette   ,  NULL},
    {0,ITEM_SHIP ,0,  MultiGunCorvette    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Resource      ,  NULL},
    {0,ITEM_SHIP ,0,  ResourceCollector   ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  StandardFrigate     ,  NULL},
    {0,ITEM_SHIP ,0,  AdvanceSupportFrigate, NULL},
    {0,ITEM_SHIP ,0,  DDDFrigate          ,  NULL},
    {0,ITEM_SHIP ,0,  IonCannonFrigate    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_NonCombat     ,  NULL},
    {0,ITEM_SHIP ,0,  GravWellGenerator   ,  NULL},            // Added by Daly on 04.21.99
    {0,ITEM_SHIP ,0,  CloakGenerator      ,  NULL},            // Added by Daly on 04.21.99
    {0,ITEM_SHIP ,0,  Probe               ,  NULL},
    {0,ITEM_SHIP ,0,  ProximitySensor     ,  NULL},
    {0,ITEM_SHIP ,0,  SensorArray         ,  NULL},
    {0,ITEM_SHIP ,0,  ResearchShip        ,  NULL},

    {-1,0,0, 0                   ,  NULL}
};

shipavailable cmShipsAvailableR1Big[NSHIPS_AVAILABLE_R1BIG + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyDefender       ,  NULL},
    {0,ITEM_SHIP ,0,  LightInterceptor    ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyInterceptor    ,  NULL},
    {0,ITEM_SHIP ,0,  CloakedFighter      ,  NULL},
    {0,ITEM_SHIP ,0,  AttackBomber        ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  LightCorvette       ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyCorvette       ,  NULL},
    {0,ITEM_SHIP ,0,  RepairCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  SalCapCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  MinelayerCorvette   ,  NULL},
    {0,ITEM_SHIP ,0,  MultiGunCorvette    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Resource      ,  NULL},
    {0,ITEM_SHIP ,0,  ResourceCollector   ,  NULL},
    {0,ITEM_SHIP ,0,  ResourceController  ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  StandardFrigate     ,  NULL},
    {0,ITEM_SHIP ,0,  AdvanceSupportFrigate, NULL},
    {0,ITEM_SHIP ,0,  DDDFrigate          ,  NULL},
    {0,ITEM_SHIP ,0,  IonCannonFrigate    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Destroyer     ,  NULL},
    {0,ITEM_SHIP ,0,  StandardDestroyer   ,  NULL},
    {0,ITEM_SHIP ,0,  MissileDestroyer    ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyCruiser        ,  NULL},
    {0,ITEM_SHIP ,0,  Carrier             ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_NonCombat     ,  NULL},
    {0,ITEM_SHIP ,0,  Probe               ,  NULL},
    {0,ITEM_SHIP ,0,  ProximitySensor     ,  NULL},
    {0,ITEM_SHIP ,0,  SensorArray         ,  NULL},

    {0,ITEM_SHIP ,0,  ResearchShip        ,  NULL},
    {0,ITEM_SHIP ,0,  GravWellGenerator   ,  NULL},
    {0,ITEM_SHIP ,0,  CloakGenerator      ,  NULL},


    {-1,0,0, 0                   ,  NULL}
};

shipavailable cmShipsAvailableR2Reg[NSHIPS_AVAILABLE_R2REG + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyDefender       ,  NULL},
    {0,ITEM_SHIP ,0,  LightInterceptor    ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyInterceptor    ,  NULL},
    {0,ITEM_SHIP ,0,  DefenseFighter      ,  NULL},
    {0,ITEM_SHIP ,0,  AttackBomber        ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  LightCorvette       ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyCorvette       ,  NULL},
    {0,ITEM_SHIP ,0,  RepairCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  SalCapCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  MinelayerCorvette   ,  NULL},
    {0,ITEM_SHIP ,0,  MultiGunCorvette    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Resource      ,  NULL},
    {0,ITEM_SHIP ,0,  ResourceCollector   ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  StandardFrigate     ,  NULL},
    {0,ITEM_SHIP ,0,  AdvanceSupportFrigate, NULL},
    {0,ITEM_SHIP ,0,  DFGFrigate          ,  NULL},
    {0,ITEM_SHIP ,0,  IonCannonFrigate    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_NonCombat     ,  NULL},
    {0,ITEM_SHIP ,0,  GravWellGenerator   ,  NULL},            // Added by Daly on 04.21.99
    {0,ITEM_SHIP ,0,  CloakGenerator      ,  NULL},            // Added by Daly on 04.21.99
    {0,ITEM_SHIP ,0,  Probe               ,  NULL},
    {0,ITEM_SHIP ,0,  ProximitySensor     ,  NULL},
    {0,ITEM_SHIP ,0,  SensorArray         ,  NULL},
    {0,ITEM_SHIP ,0,  ResearchShip        ,  NULL},

    {-1,0,0, 0                   ,  NULL}
};

shipavailable cmShipsAvailableR2Big[NSHIPS_AVAILABLE_R2BIG + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyDefender       ,  NULL},
    {0,ITEM_SHIP ,0,  LightInterceptor    ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyInterceptor    ,  NULL},
    {0,ITEM_SHIP ,0,  DefenseFighter      ,  NULL},
    {0,ITEM_SHIP ,0,  AttackBomber        ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  LightCorvette       ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyCorvette       ,  NULL},
    {0,ITEM_SHIP ,0,  RepairCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  SalCapCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  MinelayerCorvette   ,  NULL},
    {0,ITEM_SHIP ,0,  MultiGunCorvette    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Resource      ,  NULL},
    {0,ITEM_SHIP ,0,  ResourceCollector   ,  NULL},
    {0,ITEM_SHIP ,0,  ResourceController  ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  StandardFrigate     ,  NULL},
    {0,ITEM_SHIP ,0,  AdvanceSupportFrigate, NULL},
    {0,ITEM_SHIP ,0,  DFGFrigate          ,  NULL},
    {0,ITEM_SHIP ,0,  IonCannonFrigate    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Destroyer     ,  NULL},
    {0,ITEM_SHIP ,0,  StandardDestroyer   ,  NULL},
    {0,ITEM_SHIP ,0,  MissileDestroyer    ,  NULL},
    {0,ITEM_SHIP ,0,  HeavyCruiser        ,  NULL},
    {0,ITEM_SHIP ,0,  Carrier             ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_NonCombat     ,  NULL},
    {0,ITEM_SHIP ,0,  Probe               ,  NULL},
    {0,ITEM_SHIP ,0,  ProximitySensor     ,  NULL},
    {0,ITEM_SHIP ,0,  SensorArray         ,  NULL},

    {0,ITEM_SHIP ,0,  ResearchShip        ,  NULL},
    {0,ITEM_SHIP ,0,  GravWellGenerator   ,  NULL},
    {0,ITEM_SHIP ,0,  CloakGenerator      ,  NULL},


    {-1,0,0, 0                   ,  NULL}
};

shipavailable cmShipsAvailableP1[NSHIPS_AVAILABLE_P1 + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  P1Fighter           ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  P1StandardCorvette  ,  NULL},
    {0,ITEM_SHIP ,0,  P1MissileCorvette   ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  P1IonArrayFrigate   ,  NULL},

    {-1,0,0,0,NULL}
};


shipavailable cmShipsAvailableP2[NSHIPS_AVAILABLE_P2 + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  P2Swarmer           ,  NULL},
    {0,ITEM_SHIP ,0,  P2AdvanceSwarmer    ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  P2FuelPod           ,  NULL},
    {0,ITEM_SHIP ,0,  P2MultiBeamFrigate  ,  NULL},

    {-1,0,0,0,NULL}
};

shipsinprogress *curshipsInProgress;
sdword NUMBER_SHIPS_AVAILABLE = 0;
shipavailable *cmShipsAvailable;

CarrierInfo *cmCarriers = NULL;
sdword cmNumCarriers = 0;
sdword cmNumPlayersCarriers = 0;

static Ship *cmCarrierX[NUM_CMCARRIERS] = { NULL, NULL, NULL, NULL };     // don't export these for sync reasons

//sdword cmSelectedShip = -1;

//colors for printing selections and stuff
color cmSelectionTextColor = CM_SelectionTextColor;
color cmBuildingTextColor = CM_BuildingTextColor;
color cmProgressShipColor = CM_ProgressShipColor;
color cmProgressTotalColor = CM_ProgressTotalColor;
color cmStandardTextColor = CM_StandardTextColor;
color cmClassHeadingTextColor = CM_ClassHeadingTextColor;
color cmCapsExceededTextColor = CM_CapsExceededTextColor;
udword cmRefundRatio = CM_RefundRatio;
real32 cmFloatRefundRatio = CM_FloatRefundRatio;

bool cmIoSaveState;

udword cmShipTexture[MAX_RACES][NUM_CMTEXTURES] =
{
    {TR_InvalidInternalHandle, TR_InvalidInternalHandle},
    {TR_InvalidInternalHandle, TR_InvalidInternalHandle}
};

lifheader *cmShipImage[MAX_RACES][NUM_CMTEXTURES] =
{
    { NULL, NULL },
    { NULL, NULL }
};


char *ShipImagePaths[] =
{
#ifdef _WIN32
    "FEMan\\Construction_Manager\\Build_race1_",
    "FEMan\\Construction_Manager\\Build_race2_"
#else
    "FEMan/Construction_Manager/Build_race1_",
    "FEMan/Construction_Manager/Build_race2_"
#endif
};

char *ShipIcons[] =
{
    "icon1.lif", // Mothership
    "icon2.lif"  // Carrier
};

udword cmArrowTexture[6] =
{
    TR_InvalidInternalHandle, TR_InvalidInternalHandle,TR_InvalidInternalHandle,
    TR_InvalidInternalHandle, TR_InvalidInternalHandle,TR_InvalidInternalHandle
};

lifheader *cmArrowIcon[6] =
{
    NULL, NULL, NULL, NULL, NULL, NULL
};


#ifdef _WIN32
#define ARROW_ICON_PATH "FEMan\\Textures\\"
#else
#define ARROW_ICON_PATH "FEMan/Textures/"
#endif

char *ArrowIcons[] =
{
    "Build_arrow_left_mouse.lif",
    "Build_Arrow_Left_on.lif",
    "Build_Arrow_Left_off.lif",
    "Build_Arrow_right_mouse.lif",
    "Build_Arrow_right_on.lif",
    "Build_Arrow_right_off.lif",
};




sdword cmCurrentSelect = 0;



//font for the ship listing
fonthandle cmDefaultFont = 0;
fonthandle cmShipListFont = 0;
char cmDefaultFontName[CM_FontNameLength]  = CM_DefaultFont;
char cmShipListFontName[CM_FontNameLength] = CM_ShipListFont;

// frame count for red flash when ship limits exceeded
#define CAPS_REDFRAMECOUNT 5;

// Ship Caps exceeded falgs
real32 ExceededCapsFrames = 0.0;
uword ShipNumberExceeded = 0;

bool8 cmLeftArrowActive  = FALSE;
bool8 cmRightArrowActive = FALSE;

// Variable for storing the last viewed ship
ShipType curshipview=DefaultShip;

// Hack to fix Johan's hack ...
#define CM_WAIT_ACC 0.25f
extern real32 mgAccTime;

// Fix for unit caps in internet game ...
sdword shiplagtotals[TOTAL_NUM_SHIPS];

bool cmReallyPausedAll = FALSE;

bool cmActive=FALSE;

typedef struct cmDetermProgress_t
{
    Node     node;
    sdword   numShips;
    ShipType shipType;
    ShipRace shipRace;
    sdword   costPerShip;
    real32   costPerTick;
    real32   costSoFar;
    sdword   ticksForOne;
    sdword   ticksLeft;
    sdword   playerIndex;
    ShipPtr  creator;
    bool     paused;
} cmDetermProgress_t;

LinkedList cmDetermProgress;

//max number of ship types per class that can be built
sdword cmClassCapRemap[NUM_CLASSES]=
{
    CLASS_Mothership,
    CLASS_Destroyer,
    CLASS_Destroyer,
    CLASS_Destroyer,
    CLASS_Frigate,
    CLASS_Corvette,
    CLASS_Fighter,
    CLASS_Resource,
    CLASS_NonCombat
};
sdword cmMaxJobsPerClass[NUM_CLASSES] =
{
    CM_MaxJobsMothership,
    CM_MaxJobsHeavyCruiser,
    CM_MaxJobsCarrier,
    CM_MaxJobsDestroyer,
    CM_MaxJobsFrigate,
    CM_MaxJobsCorvette,
    CM_MaxJobsFighter,
    CM_MaxJobsResource,
    CM_MaxJobsNonCombat,
};

/*=============================================================================
    Private functions:
=============================================================================*/

void updateShipView(void)
{
    udword index;
    bool   set=TRUE;

    if (curshipview == DefaultShip)
    {
        for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
        {                                                       //for all available ships
            if (cmShipsAvailable[index].itemtype==ITEM_SHIP)
            {
                if(cmShipsAvailable[index].itemstat==STAT_CANBUILD)
                {
                    svSelectShip(cmShipsAvailable[index].info->shiptype);
                    curshipview = cmShipsAvailable[index].info->shiptype;
                    break;
                }
            }
        }
    }
    else
    {
        for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
        {                                                       //for all available ships
            if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
                 (cmShipsAvailable[index].info->shiptype==curshipview) )
            {
                if (cmShipsAvailable[index].itemstat!=STAT_CANBUILD)
                {
                    break;
                }
                else
                {
                    svSelectShip(curshipview);
                    set = FALSE;
                }
            }
        }
        if (set)
        {
            for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
            {                                                       //for all available ships
                if (cmShipsAvailable[index].itemtype==ITEM_SHIP)
                {
                    if(cmShipsAvailable[index].itemstat==STAT_CANBUILD)
                    {
                        svSelectShip(cmShipsAvailable[index].info->shiptype);
                        curshipview = cmShipsAvailable[index].info->shiptype;
                        break;
                    }
                }
            }
        }
    }
}

//return the progress pointed to by index
shipinprogress *cmSIP(udword index)
{
    //return &curshipsInProgress->progress[index];
    shipsinprogress *factory;
    factory = curshipsInProgress;

    return &factory->progress[index];
}


void cmWheelNegative()
{
    sdword ui = (sdword)cmUpperIndex-3;

    if (ui < 0)
        ui = 0;
    cmUpperIndex = (uword)ui;
}

void cmWheelPositive()
{
    sdword ui = (sdword)cmUpperIndex + 3;

    while (ui + cmMaxIndex > NUMBER_SHIPS_AVAILABLE)
    {
        ui--;
        if (ui < 0)
        {
            ui = 0;
            break;
        }
    }
    cmUpperIndex = (sword)ui;
}

void cmDirtyShipInfo(void)
{
    sdword i;
    if (cmShipInfoRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmShipInfoRegion);
#endif
        cmShipInfoRegion->status |= RSF_DrawThisFrame;
    }
    if (cmNumberRUsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmNumberRUsRegion);
#endif
        cmNumberRUsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmTotalRUsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmTotalRUsRegion);
#endif
        cmTotalRUsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmTotalShipsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmTotalShipsRegion);
#endif
        cmTotalShipsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmShipCostsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmShipCostsRegion);
#endif
        cmShipCostsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmShipNumbersRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmShipNumbersRegion);
#endif
        cmShipNumbersRegion->status |= RSF_DrawThisFrame;
    }
    if (cmConnectorRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmConnectorRegion);
#endif
        cmConnectorRegion->status |= RSF_DrawThisFrame;
    }

    if (cmMotherShipRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmMotherShipRegion);
#endif
        cmMotherShipRegion->status |= RSF_DrawThisFrame;
    }
    for (i=0;i<NUM_CMCARRIERS;i++)
    {
        if (cmCarrierRegions[i] != NULL)
        {
    #ifdef DEBUG_STOMP
            regVerify(cmCarrierRegions[i]);
    #endif
            cmCarrierRegions[i]->status |= RSF_DrawThisFrame;
        }
    }
}

void cmScroller(char *string, featom *atom)
{
    scrollbarhandle shandle;
    sword ind;

    if (FEFIRSTCALL(atom))
    {
        g_shandle = scSetupThumbwheel(cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE, atom);
        return;
    }

    shandle = (scrollbarhandle)atom->pData;

    if (shandle->mouseMoved != 0)
    {
        sword divs, up;
        real32 mm;

        cmDirtyShipInfo();

        mm = (real32)shandle->mouseMoved;
        divs = (sword)(mm / shandle->divSize);
        up = (sword)(cmUpperIndex + divs);
        if (up < 0)
            up = 0;
        else if (up > NUMBER_SHIPS_AVAILABLE - cmMaxIndex)
            up = (sword)(NUMBER_SHIPS_AVAILABLE - cmMaxIndex);

        if (up <0)
            up = 0;

        cmUpperIndex = (uword)up;

        scAdjustThumbwheel(shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
        shandle->mouseMoved = 0;    //clear flag
        return;
    }

    switch (shandle->event)
    {
        case SC_Other:
        ind = (sword)cmUpperIndex;
        if (mouseCursorY() < shandle->thumb.y0)
        {
            ind = (sword)(ind - cmMaxIndex);
            if (ind < 0) ind = 0;
            cmUpperIndex = (uword)ind;
        }
        else
        {
            ind = (sword)(ind + cmMaxIndex);
            if (ind > NUMBER_SHIPS_AVAILABLE - cmMaxIndex)
                ind = (sword)(NUMBER_SHIPS_AVAILABLE - cmMaxIndex);

                        if (ind <0)
                                ind = 0;

            cmUpperIndex = (uword)ind;
        }
        cmDirtyShipInfo();
        break;
    case SC_Thumb:
        break;
    case SC_Negative:
        cmWheelNegative();
        cmDirtyShipInfo();
        break;
    case SC_Positive:
        cmWheelPositive();
        cmDirtyShipInfo();
        break;
    default:
        dbgMessagef("\nunhandled consMgr scroll event %d", shandle->clickType);
    }

    scAdjustThumbwheel(shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
}

shipsinprogress *cmFindFactory(struct Ship *ship)
{
    Node *node = listofShipsInProgress.head;
    shipsinprogress *sinprogress;

    while (node != NULL)
    {
        sinprogress = (shipsinprogress *)listGetStructOfNode(node);
        if (sinprogress->ship == ship)
        {
            return sinprogress;
        }
        node = node->next;
    }
    return NULL;
}

void cmAddFactory(struct Ship *ship,bool canBuildBigShips)
{
    shipsinprogress *sinprogress;

    sinprogress = memAlloc(sizeof(shipsinprogress),"sinprogress",NonVolatile);
    memset(sinprogress,0,sizeof(shipsinprogress));

    if (ship->shiptype == Carrier)       // add even if not for the player, because we want cmCarriers to be
                                            // deterministic for all machines to prevent sync problems
    {
        cmNumCarriers++;
        cmCarriers = memRealloc(cmCarriers, sizeof(CarrierInfo) * cmNumCarriers, "cmCarriers", NonVolatile);
        cmCarriers[cmNumCarriers - 1].ship  = ship;
        cmCarriers[cmNumCarriers - 1].owner = ship->playerowner;
    }
    sinprogress->ship = ship;
    sinprogress->canBuildBigShips = canBuildBigShips;
    listAddNode(&listofShipsInProgress,&sinprogress->node,sinprogress);
    cmDirtyShipInfo();

    cmFillInCarrierXs();
}

void cmReset(void)
{
    if (!singlePlayerGameLoadNewLevelFlag)      // preserve this stuff if inbetween single player levels
    {
        listDeleteAll(&listofShipsInProgress);
        if (cmCarriers != NULL)
        {
            sdword i;
            memFree(cmCarriers);
            cmCarriers = NULL;
            cmNumCarriers = 0;
            for (i=0;i<NUM_CMCARRIERS;i++)
            {
                cmCarrierX[i] = NULL;
            }
        }
    }

    cmFillInCarrierXs();
}

void cmCloseIfOpen(void)
{
    if (cmBaseRegion != NULL)
    {
        cmClose(NULL,NULL);
    }
}

void cmPauseAllJbos(void)
{
    cmReallyPausedAll = TRUE;
}

void cmUnPauseAllJobs(void)
{
    cmReallyPausedAll = FALSE;
}

void cmRemoveFactory(struct Ship *ship)
{
    sdword i, j, k;
    Node *node;
    Node *nextnode;
    cmDetermProgress_t *dprog;
    shipsinprogress *sinprogress = cmFindFactory(ship);
    if (sinprogress != NULL)
    {
        listDeleteNode(&sinprogress->node);
        if (curshipsInProgress == sinprogress)
        {
            curshipsInProgress = NULL;
            cmCloseIfOpen();
        }
    }

    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t *)listGetStructOfNode(node);
        nextnode = node->next;

        if (dprog->creator == ship)
        {
            listDeleteNode(node);
        }

        node = nextnode;
    }

    for (i = 0; i < cmNumCarriers; i++)
    {
        if (cmCarriers[i].ship == ship)
        {
            for (k=0;k<NUM_CMCARRIERS;k++)
            {
                if (cmCarriers[i].ship == cmCarrierX[k])
                    cmCarrierX[k] = NULL;
            }

            // Resize the cmCarriers array
            for (j = i + 1; j < cmNumCarriers; j++)
            {
                cmCarriers[j - 1] = cmCarriers[j];
            }
            cmNumCarriers--;

            if (cmNumCarriers > 0)
            {
                cmCarriers = memRealloc(cmCarriers, sizeof(CarrierInfo) * cmNumCarriers, "cmCarriers", NonVolatile);
            }
            else
            {
                memFree(cmCarriers);
                cmCarriers = NULL;
            }

            break;
        }
    }
    cmDirtyShipInfo();

    cmFillInCarrierXs();
}

static bool resourcesHaveBeenAboveBefore = TRUE;

#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void cmBuildTaskFunction(void)
{
    static sdword index;
    static shipinprogress *progress;
    static Node *node;
    static shipsinprogress *sinprogress;
    static Ship *factoryship;
    static sdword decrement;

    if ((!gameIsRunning) || (cmReallyPausedAll))
    {
        return;
    }

    if (playPackets)
    {
        return;     // don't want to execute this routine which messes up resourceUnits, etc in record playback
    }

/*    taskYield(0);

    while (1)
    {
        if ((!gameIsRunning) || (cmReallyPausedAll))
        {
            taskYield(0);
            continue;
        }
        taskStackSaveCond(0);*/

        if (universe.curPlayerPtr->resourceUnits >= 300)
        {
            resourcesHaveBeenAboveBefore = TRUE;
        }

        node = listofShipsInProgress.head;
        while (node != NULL)
        {
            sinprogress = (shipsinprogress *)listGetStructOfNode(node);
            factoryship = sinprogress->ship;

            //this is a patch to avoid a very intermittent crash.
            if (!factoryship)
            {
                node = node->next;
                continue;
            }

            if (factoryship->playerowner == universe.curPlayerPtr)
            {
                for (index = 0, progress = &sinprogress->progress[0]; index < TOTAL_STD_SHIPS; index++, progress++)
                {                                                   //for all classes of ships
                    if (universe.curPlayerPtr->resourceUnits <= 0)
                    {
                        break;
                    }

                    if (!progress->paused)
                    {
                        if (progress->nJobs > 0)
                        {                                               //for each construction job
                            dbgAssert(progress->info != NULL);

                            // Simulating fixed point because we need to save fractional bits across frames
                            // RUs -= 0.5 cannot be simulated properly with integer
    #if CM_CHEAP_SHIPS
                            if (cmCheapShips && !multiPlayerGame)
                            {
                                decrement = 1;
                            }
                            else
    #endif
                            {
                                decrement = ((progress->info->buildCost << 16) / progress->timeStart) + progress->timeFraction;
                                progress->timeFraction = decrement & 0x0000FFFF; // save fraction bits
                                decrement >>= 16; // Convert back to integer
                            }
/*#if (!UNIVERSE_TURBORECORD_ONLY)
#if UNIVERSE_TURBOPAUSE_DEBUG
                            if (universeTurbo)
                            {
                                decrement *= (UNIVERSE_TURBO_REPEAT+1);
                            }
#endif
#endif*/
                            if (!universePause)
                            {
                                progress->costSoFar += decrement;
                                if (!multiPlayerGame)
                                {
                                    //xxx
                                    universe.curPlayerPtr->resourceUnits -= decrement;// decrement RU's
                                }
                                if(universe.curPlayerPtr->resourceUnits <= 0)
                                {
                                    if (resourcesHaveBeenAboveBefore)
                                    {
//                                        speechEventFleet(STAT_F_ResLevels_Nil, 0, universe.curPlayerIndex);
                                        speechEventFleet(STAT_F_ConstInProgs_ResInsufficient,
                                                         0,
                                                         universe.curPlayerIndex);
                                        resourcesHaveBeenAboveBefore = FALSE;
                                    }
                                    if (!multiPlayerGame)
                                    {
                                        //xxx
                                        universe.curPlayerPtr->resourceUnits = 0;
                                    }
                                }

        #if CM_CHEAP_SHIPS
                                if (cmCheapShips && !multiPlayerGame)
                                {
                                    progress->timeLeft = 0;
                                }
                                else
        #endif
                                {
/*#if (!UNIVERSE_TURBORECORD_ONLY)
#if UNIVERSE_TURBOPAUSE_DEBUG
                                    if (universeTurbo)
                                    {
                                        progress->timeLeft-=UNIVERSE_TURBO_REPEAT;
                                    }
                                    else
#endif
#endif*/
                                    {
                                        progress->timeLeft--;                   //decrement the time
                                    }

                                }
                            }

                            if (progress->timeLeft <= 0)
                            {                                           //if job has ended
                                if(progress->timeFraction > 32767) // > 0.5, round up and reduce the resourceUnits by 1
                                {
                                    if (!multiPlayerGame)
                                    {
                                        //xxx
                                        universe.curPlayerPtr->resourceUnits--;
                                    }
                                    if(universe.curPlayerPtr->resourceUnits <= 0)
                                    {
                                        if (resourcesHaveBeenAboveBefore)
                                        {
                                            speechEventFleet(STAT_F_ResLevels_Nil, 0, universe.curPlayerIndex);
                                            resourcesHaveBeenAboveBefore = FALSE;
                                        }
                                        if (!multiPlayerGame)
                                        {
                                            //xxx
                                            universe.curPlayerPtr->resourceUnits = 0;
                                        }
                                    }
                                }

#if CM_VERBOSE_LEVEL >= 1
                                dbgMessagef("\nConstruction job #%d has finished a %s",
                                            index, ShipTypeToNiceStr(progress->info->shiptype));
#endif
//yyy                                clWrapRUTransfer(&universe.mainCommandLayer,0,universe.curPlayerPtr->playerIndex,progress->info->buildCost,RUTRANS_BUILTSHIP);
                                if (!multiPlayerGame)
                                {
                                    //xxx
                                    clWrapCreateShip(&universe.mainCommandLayer,
                                                     progress->info->shiptype,
                                                     universe.curPlayerPtr->race,
                                                     universe.curPlayerIndex,
                                                     factoryship);
                                }

                                // fix for lag in internet games
                                if (multiPlayerGame)
                                {
                                    shiplagtotals[progress->info->shiptype] ++;
                                }

                                progress->nJobs--;                      //one less job
                                progress->costSoFar -= progress->info->buildCost;
                                if (progress->nJobs > 0)
                                {                                       //if still ships to build
                                    if ((progress->info->shipclass < CLASS_Corvette) ||
                                        (progress->info->shipclass > CLASS_Fighter))
                                    {
                                        speechEventFleet(STAT_F_Const_CapShipComplete,
                                                         progress->info->shiptype,
                                                         universe.curPlayerIndex);
                                    }
                                    progress->timeFraction = 0; // reset fixed point fractional bits
                                    progress->timeLeft = progress->timeStart =
                                        progress->info->buildTime;      //restart the job
                                }
                                else
                                {
                                    progress->nJobsTotal = 0;          //all jobs for this type complete
                                    pingNewShipPingCreate(&factoryship->posinfo.position);
                                    speechEventFleet(STAT_F_Const_CapShipComplete,
                                                     progress->info->shiptype,
                                                     universe.curPlayerIndex);
                                }
                            }
                        }
                    }
                }
            }

            node = node->next;
        }

/*        taskStackRestoreCond();
        taskYield(0);
    }*/
}
#pragma optimize("", on)

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : cmLoadTextures
    Description : load textures used by the construction manager
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmLoadTextures(void)
{
    sdword race, index;
    char filename[128];

    for (race = 0; race < MAX_RACES; race++)
    {
        for (index = 0; index < NUM_CMTEXTURES; index++)
        {
            // build filename for loading texture from file
            strcpy(filename, ShipImagePaths[race]);
            strcat(filename, ShipIcons[index]);

            // load the image
            cmShipImage[race][index] = trLIFFileLoad(filename, NonVolatile);

            if (bitTest(cmShipImage[race][index]->flags, TRF_Paletted))
            {
                cmPaletted = TRUE;
            }
            else
            {
                cmPaletted = FALSE;
            }

            if (cmPaletted)
            {
                cmShipTexture[race][index] = trPalettedTextureCreate(
                    cmShipImage[race][index]->data,
                    cmShipImage[race][index]->palette,
                    cmShipImage[race][index]->width,
                    cmShipImage[race][index]->height);
            }
            else
            {
                cmShipTexture[race][index] = trRGBTextureCreate(
                    (color*)cmShipImage[race][index]->data,
                    cmShipImage[race][index]->width,
                    cmShipImage[race][index]->height,
                    TRUE);
            }
        }
    }

    for (index = 0; index < 6; index++)
    {
#if 1
        cmArrowIcon[index] = NULL;
        cmArrowTexture[index] = TR_InvalidInternalHandle;
#else
        strcpy(filename, ARROW_ICON_PATH);
        strcat(filename, ArrowIcons[index]);

        cmArrowIcon[index] = trLIFFileLoad(filename, NonVolatile);
        dbgAssert(cmArrowIcon[index]);

        cmArrowTexture[index] = trRGBTextureCreate(
            (color*)cmArrowIcon[index]->data,
            cmArrowIcon[index]->width,
            cmArrowIcon[index]->height,
            TRUE);
#endif
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmCloseTextures
    Description : free memory and handles associated with any textures we might be using
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmCloseTextures(void)
{
    sdword race, index;

    for (race = 0; race < MAX_RACES; race++)
    {
        for (index = 0; index < NUM_CMTEXTURES; index++)
        {
            if (cmShipImage[race][index] != NULL)
            {
                memFree(cmShipImage[race][index]);
                cmShipImage[race][index] = NULL;
            }
            if (cmShipTexture[race][index] != TR_InvalidInternalHandle)
            {
                trRGBTextureDelete(cmShipTexture[race][index]);
                cmShipTexture[race][index] = TR_InvalidInternalHandle;
            }
        }
    }

    for (index = 0; index < 6; index++)
    {
        if (cmArrowIcon[index] != NULL)
        {
            memFree(cmArrowIcon[index]);
            cmArrowIcon[index] = NULL;
        }
        if (cmArrowTexture[index] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(cmArrowTexture[index]);
            cmArrowTexture[index] = TR_InvalidInternalHandle;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmStartup
    Description : Start the construction manager module.
    Inputs      : void
    Outputs     : Starts a task to take care of construction.
    Return      : void
----------------------------------------------------------------------------*/
void cmStartup(void)
{
    sdword index;

    cmDeterministicStartup();

    listInit(&listofShipsInProgress);
//    cmBuildTask = taskStart(cmBuildTaskFunction, CM_TaskServicePeriod, 0);
    cmDefaultFont = frFontRegister(cmDefaultFontName);
    cmShipListFont = frFontRegister(cmShipListFontName);

    cmStandardTextColor = FEC_ListItemStandard;

    cmLoadTextures();

    for (index=0;index<TOTAL_NUM_SHIPS;index++)
    {
        shiplagtotals[index] = 0;
    }

    cmRenderEverythingCounter = 0;

    /*
    textureRect.x0 = rect.x0+SP_TEXTURE_INSET;
    textureRect.y0 = rect.y0+SP_TEXTURE_INSET;
    textureRect.x1 = rect.x1-SP_TEXTURE_INSET;
    textureRect.y1 = rect.y1-SP_TEXTURE_INSET;

    rndPerspectiveCorrection(FALSE);
    primRectSolidTextured2(&textureRect);
    */
}

/*-----------------------------------------------------------------------------
    Name        : cmShutdown
    Description : Shut down the task manager
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void cmShutdown(void)
{
    cmDeterministicShutdown();

    if (cmScreensHandle)
    {
        feScreensDelete(cmScreensHandle);
        cmScreensHandle = NULL;
    }

    if (cmBuildTask != -1)
    {
        taskStop(cmBuildTask);
    }

    cmCloseTextures();

    cmReset();
}

/*-----------------------------------------------------------------------------
    Name        : cmConnectorDraw
    Description : Draws the connector between the image of the ship and
                  the info. text
    Inputs      : region
    Outputs     : connector image
    Return      : void
----------------------------------------------------------------------------*/
void cmConnectorDraw(featom *atom, regionhandle region)
{
    sdword centerX  = region->rect.x0 + ((region->rect.x1 - region->rect.x0) / 2);

    cmConnectorRegion = region;

    primLine2(centerX, region->rect.y0, centerX, region->rect.y1, colRGB(255, 0, 0));
    primCircleSolid2(centerX, region->rect.y1, 3, 16, colRGB(255, 0, 0));
}

/*-----------------------------------------------------------------------------
    Name        : cmShipDisabled
    Description : determines whether a given shiptype has been disabled for building or not
    Inputs      : type - ship type
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
/*
bool cmShipDisabled(ShipType type)
{
    sdword index;

    for (index = 0; cmShipsDisabled[index].disabled != -1; index++)
    {
        if (cmShipsDisabled[index].type == type &&
            cmShipsDisabled[index].disabled)
        {
            return TRUE;
        }
    }

    return FALSE;
}
*/
/*-----------------------------------------------------------------------------
    Name        : cmUpdateShipAvailable
    Description : updates the ship build list for technology state.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void cmUpdateShipAvailable(void)
{
    sdword index;
    sdword isclass=STAT_DONTPRINT;

    NUMBER_SHIPS_AVAILABLE=0;

    for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {
        if (cmShipsAvailable[index].itemtype==ITEM_SHIP)
        {
            if (rmCanBuildShip(universe.curPlayerPtr, cmShipsAvailable[index].type) &&
                bitTest(cmShipsAvailable[index].info->staticheader.infoFlags, IF_InfoLoaded))
            {
                cmShipsAvailable[index].itemstat = STAT_CANBUILD;
                NUMBER_SHIPS_AVAILABLE++;
            }
            else
            {
                cmShipsAvailable[index].itemstat = STAT_CANTBUILD;
            }
        }
    }

    for (; index >= 0; index--)
    {
        if (cmShipsAvailable[index].itemtype==ITEM_SHIP)
        {
            if (cmShipsAvailable[index].itemstat == STAT_CANBUILD)
                isclass=STAT_PRINT;
        }
        else if (cmShipsAvailable[index].itemtype==ITEM_CLASS)
        {
            cmShipsAvailable[index].itemstat = (sword)isclass;
            if (isclass==STAT_PRINT) NUMBER_SHIPS_AVAILABLE++;
            isclass=STAT_DONTPRINT;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : cmClose
    Description : Callback function to close the construction manager
    Inputs      :
    Outputs     : Deletes all regions associated with construction manager
    Return      :
----------------------------------------------------------------------------*/
void cmClose(char *string, featom *atom)
{                                                           //close the construction manager
    sdword i;
    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildClose)
        return;

    cmRenderEverythingCounter = 0;

#if CM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nClose construction manager.");
#endif
    curshipsInProgress = NULL;
    cmUpperIndex=0;
    feScreenDeleteFlags(cmBaseRegion,FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND);
    cmBaseRegion = NULL;

    if (cmIoSaveState)
        ioEnable();

    // enable rendering of main game screen
    mrRenderMainScreen = TRUE;

    glcFullscreen(FALSE);

    /* play the exit sound */
    soundEvent(NULL, UI_ManagerExit);
    //restart the sound of space ambient
    soundEvent(NULL, UI_SoundOfSpace);

    spUnlockout();

    cmShipInfoRegion    = NULL;
    cmNumberRUsRegion   = NULL;
    cmTotalRUsRegion    = NULL;
    cmTotalShipsRegion  = NULL;
    cmShipCostsRegion   = NULL;
    cmShipNumbersRegion = NULL;
    cmConnectorRegion   = NULL;
    cmMotherShipRegion  = NULL;
    for (i=0;i<NUM_CMCARRIERS;i++)
    {
        cmCarrierRegions[i] = NULL;
    }

    svClose();

    opKeyDetour = 0; //no key detours

    cmPrintHotKey = FALSE;

    // enable taskbar popup window
    tbDisable = FALSE;

    cmActive = FALSE;

}
/*-----------------------------------------------------------------------------
    Name        : cmBuildShips
    Description : Callback function to build selected ships.
    Inputs      :
    Outputs     : Start building all the selected ships
    Return      :
----------------------------------------------------------------------------*/
void cmBuildShips(char *string, featom *atom)
{
    sdword index;
    bool builtSomething = FALSE;
    bool capShipBuilt = FALSE;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildBuildShips)
        return;

#if CM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nBuild selected ships..");
#endif

    for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {                                                       //for all available ships
/*        if (cmSIP(index)->paused)
        {
            cmSIP(index)->paused = FALSE;
        }*/
        if (cmShipsAvailable[index].nJobs > 0)
        {
            builtSomething = TRUE;
            cmBuildJobsAdd(curshipsInProgress,
                           cmShipsAvailable[index].info,
                           cmShipsAvailable[index].nJobs);
            if (cmShipsAvailable[index].info->shipclass <= CLASS_Frigate)
            {
                capShipBuilt = TRUE;
            }
        }
    }

    for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {                                                       //for all available ships
        cmShipsAvailable[index].nJobs = 0;                    //no jobs queued yet
    }

    if (builtSomething)
    {
        speechEventFleet(COMM_F_Const_Start, 0, universe.curPlayerIndex);
        if (capShipBuilt)
        {
            speechEvent(curshipsInProgress->ship, COMM_Const_BuildCapShip, ranRandom(RAN_SoundGameThread) % 26);
        }
    }

    cmDirtyShipInfo();
}


/*-----------------------------------------------------------------------------
    Name        : cmCancelShipType
    Description : Cancels all jobs that refer to a certain shiptype
    Inputs      : type - the shiptype to cancel
    Outputs     : Cancels the build requests of a shiptype
    Return      : void
----------------------------------------------------------------------------*/
void cmForceBuildShipType(ShipType type)
{
//    udword i;

    curshipsInProgress = cmFindFactory(universe.curPlayerPtr->PlayerMothership);

    curshipsInProgress->progress[type].timeLeft = 0;
}


/*-----------------------------------------------------------------------------
    Name        : cmCancelJobs
    Description : Callback to cancel all current jobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmCancelJobs(char *string, featom *atom)
{
    sdword index, jobCount;
    shipinprogress *progress;
    sdword refund/*,totalSpent*/;
    bool   canceled=FALSE;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildCancelJobs)
        return;

    progress = &curshipsInProgress->progress[0];

    for (index = jobCount = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {                                                       //for all available ships
        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (progress[cmShipsAvailable[index].type].selected) )
        {
            cmShipsAvailable[index].nJobs = 0;
            canceled = TRUE;

            if (multiPlayerGame)
            {
                if (cmShipsAvailable[index].info)
                {
                    //xxx
                    clWrapDeterministicBuild(CMD_CANCEL, &universe.mainCommandLayer,
                                             0, /*numShips == as.many.as.there.are*/
                                             cmShipsAvailable[index].type,
                                             cmShipsAvailable[index].info->shiprace,
                                             universe.curPlayerIndex,
                                             curshipsInProgress->ship);
                }
            }
        }
    }
    //speechEventFleet(COMM_F_Const_CancelAll, 0, universe.curPlayerIndex);

    for (index = 0; index < TOTAL_STD_SHIPS; index++, progress++)
    {                                                   //for all classes of ships
        if (progress->selected)
        {
            if (progress->nJobsTotal != 0)
            {
                refund = progress->costSoFar;
                refund = (sdword)((real32)refund * cmFloatRefundRatio);
//yyy
/*
                clWrapRUTransfer(&universe.mainCommandLayer,
                                 0,
                                 universe.curPlayerPtr->playerIndex,
                                 refund, RUTRANS_BUILDCANCEL);
*/
                if (!multiPlayerGame)
                {
                    //xxx
                    universe.curPlayerPtr->resourceUnits += refund;
                }
            }
            progress->timeFraction = 0; // No fixed point fraction bits (used for decrementing RUs)
            progress->timeStart = progress->timeLeft = 0;   //job all done
            progress->nJobsTotal = progress->nJobs = 0;     //cancel building jobs
            progress->costSoFar = 0;
            canceled = TRUE;

            if (multiPlayerGame)
            {
                if (progress->info)
                {
                    //xxx
                    clWrapDeterministicBuild(CMD_CANCEL, &universe.mainCommandLayer,
                                             0, /*numShips == as.many.as.there.are*/
                                             progress->info->shiptype,
                                             progress->info->shiprace,
                                             universe.curPlayerIndex,
                                             curshipsInProgress->ship);
                }
            }
        }
    }

    if (canceled == TRUE) speechEventFleet(COMM_F_Const_CancelBatch, 0, universe.curPlayerIndex);

    cmDirtyShipInfo();
}


/*-----------------------------------------------------------------------------
    Name        : cmPauseJobs
    Description : Callback to pause/unpause all current jobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmPauseJobs(char *string, featom *atom)
{
    sdword index;
    shipinprogress *progress;
    bool allpaused = TRUE, changed=FALSE;

    //speechEventFleet(COMM_F_Const_CancelAll, 0, universe.curPlayerIndex);   // later change to ConstructionCancel

    if((tutorial==TUTORIAL_ONLY) & !tutEnable.bBuildPauseJobs)
        return;

    //all paused?
    progress = &curshipsInProgress->progress[0];
    for (index = 0; (index < TOTAL_STD_SHIPS) && allpaused; index++, progress++)
    {                                                   //for all classes of ships
        if (progress->selected)
        {
            if (!progress->paused)
            {
                allpaused = FALSE;
            }
        }
    }

    //set all jobs to opposite of allpaused
    progress = &curshipsInProgress->progress[0];
    for (index = 0; index < TOTAL_STD_SHIPS; index++, progress++)
    {                                                   //for all classes of ships
        if (progress->selected)
        {
            udword command;

            progress->paused = !allpaused;
            if (progress->nJobs>0) changed = TRUE;

            command = progress->paused ? CMD_PAUSE : CMD_UNPAUSE;
            if (multiPlayerGame)
            {
                if (progress->info)
                {
                    //xxx
                    clWrapDeterministicBuild(command, &universe.mainCommandLayer,
                                             0, /*numShips doesn't matter here*/
                                             progress->info->shiptype,
                                             progress->info->shiprace,
                                             universe.curPlayerIndex,
                                             curshipsInProgress->ship);
                }
            }
        }
    }

    if (changed)
    {
        if (!allpaused)
        {
            speechEventFleet(COMM_F_Const_Pause, 0, universe.curPlayerIndex);
        }
        else
        {
            speechEventFleet(COMM_F_Const_Resume, 0, universe.curPlayerIndex);
        }
    }
    cmDirtyShipInfo();
}

#if(0)
/*-----------------------------------------------------------------------------
    Name        : cmPauseJobs
    Description : Callback to pause/unpause all current jobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmPauseJobsOld(char *string, featom *atom)
{
    udword index;
    bool allpaused = TRUE;
    shipinprogress *sip;

    if((tutorial==TUTORIAL_ONLY) & !tutEnable.bBuildPauseJobs)
        return;

    cmDirtyShipInfo();

    /*
    if (bitTest(taskData[cmBuildTask]->flags, TF_Paused))
    {
        speechEventFleet(COMM_F_Const_Resume, 0, universe.curPlayerIndex);
        taskResume(cmBuildTask);
    }
    else
    {
        speechEventFleet(COMM_F_Const_Pause, 0, universe.curPlayerIndex);
        taskPause(cmBuildTask);
    }
    */

    //check if all selected ships paused

    for (index=0; (cmShipsAvailable[index].nJobs!=-1) && allpaused; index++)
    {
        //if (cmShipsAvailable[index].selected)
        sip = cmSIP(index);

        if (sip->selected)
        {
            if (!sip->paused)
            {
                allpaused = FALSE;
            }
        }
    }

    if (allpaused)  //unpause them all
    {
        for (index=0; (cmShipsAvailable[index].nJobs!=-1) && allpaused; index++)
        {
            sip = cmSIP(index);
            if (sip->selected)
            {
                sip->paused = FALSE;
            }
        }

    }
    else    //pause them all
    {
        for (index=0; (cmShipsAvailable[index].nJobs!=-1) && allpaused; index++)
        {
            sip = cmSIP(index);
            if (sip->selected)
            {
                sip->paused = TRUE;
            }        }

    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : cmShipCostsDraw
    Description : Draw the cost of building selected ships.
    Inputs      : feflow callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void cmShipCostsDraw(featom *atom, regionhandle region)
{
    sdword x, y, index;
    rectangle *rect = &region->rect;
    bool        newline=FALSE;
    color c;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;
    sdword  numlines,startind=0;

    currentFont = fontCurrentGet();
    fontMakeCurrent(cmDefaultFont);

    feStaticRectangleDraw(region);                          //draw standard rectangle
    primRectSolid2(rect, CM_BackgroundColor);
    primLine2(rect->x0, rect->y0, rect->x0, rect->y1, colRGB(0, 100, 160));

    cmShipCostsRegion = region;

    numlines = 0;

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    newline=FALSE;
    numlines=0;

    y = rect->y0 + CM_ASMarginTop;
    factory = curshipsInProgress;

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= rect->y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

            if (cmShipsAvailable[index].nJobs > 0)
            {                                                   //if this ship already selected
                c = cmSelectionTextColor;
            }
            else
            {
                c = cmStandardTextColor;
            }
            if (progress->nJobs > 0)
            {                                                   //if working on this class
                c = cmBuildingTextColor;
            }

#if CM_CHEAP_SHIPS
            if (cmCheapShips && !multiPlayerGame)
            {
                x = rect->x1 - CM_ASMarginLeft - fontWidth("1");
                fontPrint(x, y, c, "1");
            }
            else
#endif
            {
                x = rect->x1 - CM_ASMarginLeft - fontWidthf("%d", cmShipsAvailable[index].info->buildCost);
                fontPrintf(x, y, c, "%d", cmShipsAvailable[index].info->buildCost);
            }

            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }


        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }

    fontMakeCurrent(currentFont);

}

/*-----------------------------------------------------------------------------
    Name        : cmSelectShipType
    Description : Find what type of ship was selected
    Inputs      : reg - region clicked on
                  yClicked - vertical location clicked on.
    Outputs     :
    Return      : type of ship clicked on or -1 if none
----------------------------------------------------------------------------*/
sdword cmSelectShipType(regionhandle region, sdword yClicked)
{
    sdword index, y, numlines, startind;
    fonthandle currentFont;
    bool newline = FALSE;

    currentFont = fontMakeCurrent(cmDefaultFont);

    numlines = 0;

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (cmShipsAvailable[index].itemtype==ITEM_SHIP)
        {
            if (cmShipsAvailable[index].itemstat != STAT_CANTBUILD)
                newline=TRUE;
        }
        else if (cmShipsAvailable[index].itemtype==ITEM_CLASS)
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    y = region->rect.y0 + CM_ASMarginTop;

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= region->rect.y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
        {
            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }

        if ((yClicked < y + fontHeight(" ") + CM_ASInterSpacing)&&(newline))
        {
            if ((cmShipsAvailable[index].itemtype==ITEM_SHIP)&&
                (cmShipsAvailable[index].itemstat==STAT_CANBUILD))
            {
                if( (tutorial==TUTORIAL_ONLY) && tutIsBuildShipRestricted((sdword)cmShipsAvailable[index].type) )
                    return(-1);
                else
                    return(index);
            }
            else
                return(-1);
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }

    fontMakeCurrent(currentFont);

    return (-1);
}

/*-----------------------------------------------------------------------------
    Name        : cmClassCapClassRemap
    Description : Get the build manager class for a given ship
    Inputs      : shipType - type of a ship
                  shipClass - class of said ship
    Outputs     :
    Return      : build manager class
----------------------------------------------------------------------------*/
ShipClass cmClassCapClassRemap(ShipType shipType, ShipClass shipClass)
{
    if (shipType == ResourceController)
    {                                                       //resource controller is a special case
        return(CLASS_Resource);
    }
    return(cmClassCapRemap[shipClass]);
}

/*-----------------------------------------------------------------------------
    Name        : cmClassCapCanCreateShip
    Description : Can we build a ship of this class or are all the berths in use?
    Inputs      : shipType - what type of ship we want to build.
                  factory - where we want to build it from.
                  cmShipsAvail - what ships are in the works.
    Outputs     :
    Return      : TRUE if we can build it, false otherwise
----------------------------------------------------------------------------*/
bool cmClassCapCanCreateShip(ShipType shipType, shipsinprogress *factory, shipavailable *cmShipsAvail)
{
    ShipStaticInfo *shipStaticInfo;
    ShipClass shipClass;
    ShipRace shipRace;
    sdword index, nInThisClass;
    bool shipsQueuedUp[TOTAL_NUM_SHIPS];

    shipStaticInfo = GetShipStaticInfo(shipType, factory->ship->shiprace);//get static info
    shipClass = cmClassCapClassRemap(shipType, shipStaticInfo->shipclass); //get ship class
    shipRace = factory->ship->playerowner->race;            //get race of player who owns ship, not race of ship

    //clear out list of queued ships
    memset(shipsQueuedUp, 0, TOTAL_NUM_SHIPS * sizeof(bool));

    //find all ships of same class queued up for construction
    for (index = nInThisClass = 0; cmShipsAvail[index].nJobs != -1; index++)
    {
        shipStaticInfo = GetShipStaticInfo(cmShipsAvail[index].type, shipRace);
        if (cmShipsAvail[index].nJobs > 0)
        {                                                   //if this ship being built
            if (cmClassCapClassRemap(shipStaticInfo->shiptype, shipStaticInfo->shipclass) == shipClass)
            {                                               //if this job the same class as the one we want to build
                shipsQueuedUp[shipStaticInfo->shiptype] = TRUE;
                if (shipStaticInfo->shiptype != shipType)
                {                                           //don't count this one if it's the one we're clicking on
                    nInThisClass++;
                }
            }
        }
    }
    //find all ships of same class already under construction in this factory
    for (index = 0; index < TOTAL_STD_SHIPS; index++)
    {
        shipStaticInfo = GetShipStaticInfo(index, shipRace);
        if (factory->progress[index].nJobs > 0)
        {
            if (cmClassCapClassRemap(shipStaticInfo->shiptype, shipStaticInfo->shipclass) == shipClass)
            {                                               //if this job the same class as the one we want to build
                if (!shipsQueuedUp[index])
                {
                    shipsQueuedUp[index] = TRUE;
                    if (shipStaticInfo->shiptype != shipType)
                    {                                       //don't count this one if it's the one we're clicking on
                        nInThisClass++;
                    }
                }
            }
        }
    }
    //find all ships not yet started due to network lag.  This does not seem to know of it's factory.
    //...or would that be covered by the first check?
    for (index = 0; index < TOTAL_STD_SHIPS; index++)
    {
        shipStaticInfo = GetShipStaticInfo(index, shipRace);
        if (shiplagtotals[index] > 0)
        {
            if (cmClassCapClassRemap(shipStaticInfo->shiptype, shipStaticInfo->shipclass) == shipClass)
            {                                               //if this job the same class as the one we want to build
                if (!shipsQueuedUp[index])
                {
                    shipsQueuedUp[index] = TRUE;
                    if (shipStaticInfo->shiptype != shipType)
                    {                                       //don't count this one if it's the one we're clicking on
                        nInThisClass++;
                    }
                }
            }
        }
    }

    return(nInThisClass < cmMaxJobsPerClass[shipClass]);
}

bool cmIncrement(shipsinprogress *factory, uword index)
{
    shipinprogress *progress;
    bool flag = FALSE;
/*
    bool disabled;

    //ctrl = add 5?

    disabled = cmShipDisabled(cmShipsAvailable[index].type);
*/
    progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
/*
    if (unitCapCanCreateShip((uword)cmShipsAvailable[index].info->shiptype,factory,cmShipsAvailable) && !disabled)
*/
    if (unitCapCanCreateShip((uword)cmShipsAvailable[index].info->shiptype,factory,cmShipsAvailable) &&
        cmClassCapCanCreateShip((ShipType)cmShipsAvailable[index].info->shiptype,factory,cmShipsAvailable))
    {
        flag = TRUE;
        if (progress->nJobsTotal > 0)
        {
            progress->nJobsTotal++;
            progress->nJobs++;

            if (multiPlayerGame)
            {
                //xxx
                clWrapDeterministicBuild(CMD_START, &universe.mainCommandLayer,
                                         1,
                                         progress->info->shiptype,
                                         universe.curPlayerPtr->race,
                                         universe.curPlayerIndex,
                                         factory->ship);
            }
        }
        else
        {
            cmShipsAvailable[index].nJobs++;
        }
    }
    else
/*
    if (!disabled)
*/
    {
        //put sound effect for too many ships HERE
        ShipNumberExceeded = (uword)index;
        ExceededCapsFrames = taskTimeElapsed;
    }
    svSelectShip(cmShipsAvailable[index].info->shiptype);
    curshipview = cmShipsAvailable[index].info->shiptype;

    return flag;
}



bool cmDecrement(shipsinprogress *factory, uword index)
{
    shipinprogress *progress;
    bool flag = TRUE, skip = FALSE;

    //ctrl = add 5?

    progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
    //inactive jobs
    if (cmShipsAvailable[index].nJobs > 0)
    {
        cmShipsAvailable[index].nJobs--;
        skip = TRUE;
    }
    //active jobs
    if (!skip && progress->nJobs > 1)
    {
        progress->nJobs--;
        progress->nJobsTotal--;
        dbgMessagef("\n!! removed a job, jobs %d total %d !!",
                    progress->nJobs, progress->nJobsTotal);

        if (multiPlayerGame)
        {
            //xxx
            clWrapDeterministicBuild(CMD_START, &universe.mainCommandLayer,
                                     1 | 0x4000,
                                     progress->info->shiptype,
                                     universe.curPlayerPtr->race,
                                     universe.curPlayerIndex,
                                     factory->ship);
        }
    }
    svSelectShip(cmShipsAvailable[index].info->shiptype);
    curshipview = cmShipsAvailable[index].info->shiptype;

    return flag;
}



/*-----------------------------------------------------------------------------
    Name        : cmRightArrowProcess
    Description :
    Inputs      : standard region callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword cmRightArrowProcess(regionhandle region, sdword ID, udword event, udword data)
{
    shipsinprogress *factory;
    //shipinprogress *progress;

    if (multiPlayerGame)
    {
        if (!multiPlayerGameUnderWay)
        {
            return (0);
        }
    }

    if (event == RPE_PressLeft)
    {
        mgAccTime = taskTimeElapsed - CM_WAIT_ACC;
        return (0);
    }

    if (event == RPE_ReleaseLeft)
    {
        cmArrowIndex = -1;
//        mgAcceleratorRelease();
        return (0);
    }
    //if(!mouseInRect(&region->rect))
    //{
    //    return (0);
    //}

    cmArrowIndex = cmSelectShipType(region, mouseCursorY());

    factory = curshipsInProgress;

    if ( (event == RPE_HoldLeft) && (cmArrowIndex!=-1)  && mgAccelerator() )
    {
        if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildArrows)
            return(0);

        if( (tutorial==TUTORIAL_ONLY) && tutIsBuildShipRestricted(cmShipsAvailable[cmArrowIndex].info->shiptype) )
            return(0);

        if (keyIsHit(CM_FIVEKEY))
        {
            uword j;
            for (j=0; j<5; j++)
            {
                if (cmIncrement(factory,(uword)cmArrowIndex))
                    cmRightArrowActive = TRUE;
                else
                    break;
            }
        }
        else
        {
            if (cmIncrement(factory,(uword)cmArrowIndex))
                cmRightArrowActive = TRUE;
        }

    }
    else if (event == RPE_WheelUp)
    {
        cmWheelNegative();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    else if (event == RPE_WheelDown)
    {
        cmWheelPositive();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    cmDirtyShipInfo();
    return(0);
}


/*-----------------------------------------------------------------------------
    Name        : cmLeftArrowProcess
    Description :
    Inputs      : standard region callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword cmLeftArrowProcess(regionhandle region, sdword ID, udword event, udword data)
{
    shipsinprogress *factory;
    //shipinprogress *progress;

    if (multiPlayerGame)
    {
        if (!multiPlayerGameUnderWay)
        {
            return (0);
        }
    }

    if (event == RPE_PressLeft)
    {
        mgAccTime = taskTimeElapsed - CM_WAIT_ACC;
        return (0);
    }

    if (event == RPE_ReleaseLeft)
    {
        cmArrowIndex = -1;
//        mgAcceleratorRelease();
        return (0);
    }
    //if(!mouseInRect(&region->rect))
    //{
    //    return (0);
    //}

    cmArrowIndex = cmSelectShipType(region, mouseCursorY());

    factory = curshipsInProgress;

    if ( (event == RPE_HoldLeft) && (cmArrowIndex!=-1) && mgAccelerator())
    {
        if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildArrows)
            return(0);

        cmLeftArrowActive = TRUE;

        if (keyIsHit(CM_FIVEKEY))
        {
            uword j;
            for (j=0; j<5; j++)
            {
                if (cmDecrement(factory,(uword)cmArrowIndex))
                    cmLeftArrowActive = TRUE;
            }
        }
        else
        {
            if (cmDecrement(factory,(uword)cmArrowIndex))
                cmLeftArrowActive = TRUE;
        }
    }
    else if (event == RPE_WheelUp)
    {
        cmWheelNegative();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    else if (event == RPE_WheelDown)
    {
        cmWheelPositive();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    cmDirtyShipInfo();
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : cmDrawArrow
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void new_cmDrawArrow(regionhandle region, bool leftArrow)
{
    sdword y, index, numlines, startind=0;
    bool newline = FALSE;
    rectangle *rect = &region->rect;
    rectangle arrowrect;

    triangle tri;
    sdword width = rect->x1 - rect->x0 - 4;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;
    sdword leftoffset = 0;

    numlines = 0;

    currentFont = fontMakeCurrent(cmDefaultFont);

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    #define YOFFSET 200

    newline=FALSE;
    numlines=0;

    y = rect->y0 + CM_ASMarginTop;
    factory = curshipsInProgress;

    if(leftArrow)
    {
        tri.x0 = rect->x1 - 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }
    else
    {
        tri.x0 = rect->x0 + 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= rect->y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

            if(leftArrow)
            {
                tri.y0 = YOFFSET + y;
                tri.y1 = YOFFSET + y + (width / 2);
                tri.y2 = YOFFSET + y + width;
            }
            else
            {
                tri.y0 = YOFFSET + y;
                tri.y1 = YOFFSET + y + width;
                tri.y2 = YOFFSET + y + (width / 2);
            }

            if(((leftArrow && cmLeftArrowActive) || (!leftArrow && cmRightArrowActive)) && (index==cmArrowIndex))
            {
                cmLeftArrowActive  = FALSE;
                cmRightArrowActive = FALSE;
                cmArrowIndex = -1;
                primTriSolid2(&tri, colRGB(200, 0, 0));
            }


            primTriOutline2(&tri, 1, colRGB(200, 200, 0));

            if (leftArrow)
                leftoffset = 3;
            else
                leftoffset = 0;


            arrowrect.x0 = rect->x0;
            arrowrect.x1 = rect->x0 + 20;
            arrowrect.y0 = y;
            arrowrect.y1 = y + 20;

            //trPalettedTextureMakeCurrent(cmArrowTexture[0 + leftoffset], cmArrowIcon[0 + leftoffset]->palette);
            trRGBTextureMakeCurrent(cmArrowTexture[0 + leftoffset]);

            rndPerspectiveCorrection(FALSE);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            primRectSolidTextured2(&arrowrect);






            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }
    fontMakeCurrent(currentFont);
}



/*-----------------------------------------------------------------------------
    Name        : cmDrawArrow
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmDrawArrow(regionhandle region, bool leftArrow)
{
    sdword y, index, numlines, startind=0;
    bool newline = FALSE;
    rectangle *rect = &region->rect;
    triangle tri;
    sdword width = rect->x1 - rect->x0 - 4;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;

    numlines = 0;

    currentFont = fontMakeCurrent(cmDefaultFont);

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    newline=FALSE;
    numlines=0;

    y = rect->y0 + CM_ASMarginTop;
    factory = curshipsInProgress;

    if(leftArrow)
    {
        tri.x0 = rect->x1 - 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }
    else
    {
        tri.x0 = rect->x0 + 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= rect->y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

            if(leftArrow)
            {
                tri.y0 = y;
                tri.y1 = y + (width / 2);
                tri.y2 = y + width;
            }
            else
            {
                tri.y0 = y;
                tri.y1 = y + width;
                tri.y2 = y + (width / 2);
            }

            if(((leftArrow && cmLeftArrowActive) || (!leftArrow && cmRightArrowActive)) && (index==cmArrowIndex))
            {
                cmLeftArrowActive  = FALSE;
                cmRightArrowActive = FALSE;
                cmArrowIndex = -1;
                primTriSolid2(&tri, colRGB(200, 0, 0));
            }

            primTriOutline2(&tri, 1, colRGB(200, 200, 0));

            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }
    fontMakeCurrent(currentFont);
}


/*-----------------------------------------------------------------------------
    Name        : cmLeftArrowDraw
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmLeftArrowDraw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_HoldLeft |
                        RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
        regFunctionSet(region, (regionfunction)cmLeftArrowProcess);         //set new region handler function
    }
    cmDrawArrow(region, TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : cmRightArrowDraw
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmRightArrowDraw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_HoldLeft |
                        RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
        regFunctionSet(region, (regionfunction)cmRightArrowProcess);        //set new region handler function
    }
    cmDrawArrow(region, FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : cmShipNumbersDraw
    Description : Draw the number of ships being built.
    Inputs      : feflow callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void cmShipNumbersDraw(featom *atom, regionhandle region)
{
    sdword x, y, index, numShips;
    rectangle *rect = &region->rect;
    bool newline = FALSE;
    color c;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;
    sdword  numlines,startind=0;

    currentFont = fontCurrentGet();
    fontMakeCurrent(cmDefaultFont);

    //if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    //{                                                       //if region not processed yet
    //    region->flags = RPE_PressLeft | RPE_PressRight |
    //                    RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
    //    regFunctionSet(region, cmSelectNumber);             //set new region handler function
    //}
    feStaticRectangleDraw(region);                          //draw standard rectangle
    primRectSolid2(rect, CM_BackgroundColor);
    primLine2(rect->x0, rect->y0, rect->x0, rect->y1, colRGB(0, 100, 160));

    cmShipNumbersRegion = region;

    numlines = 0;

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    newline=FALSE;
    numlines=0;

    y = rect->y0 + CM_ASMarginTop;
    factory = curshipsInProgress;

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= rect->y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

            if (cmShipsAvailable[index].nJobs > 0)
            {                                                   //if this ship already selected
                c = cmSelectionTextColor;
            }
            else
            {
                c = cmStandardTextColor;
            }
            if (progress->nJobs > 0)
            {                                                   //if working on this class
                c = cmBuildingTextColor;
            }

            numShips = cmShipsAvailable[index].nJobs;
            numShips += progress->nJobs;  //active jobs, too

            x = rect->x1 - CM_ASMarginLeft - fontWidthf("%d", numShips) - 5;
            fontPrintf(x, y, c, "%d", numShips);

            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }


        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }

    fontMakeCurrent(currentFont);

}


/*-----------------------------------------------------------------------------
    Name        : cmSelectAvailable
    Description : Callback for selecting/deselecting ships
    Inputs      : standard region callback
    Outputs     : selects or deselects a ship for construction
    Return      :
----------------------------------------------------------------------------*/
sdword cmSelectAvailable(regionhandle region, sdword ID, udword event, udword data)
{
    sdword index;
    shipsinprogress *factory;
    shipinprogress *progress, *kprogress;

    if (multiPlayerGame)
    {
        if (!multiPlayerGameUnderWay)
        {
            return (0);
        }
    }

    index = cmSelectShipType(region, mouseCursorY());

    factory = curshipsInProgress;

    if ( (event == RPE_PressLeft || event == RPE_DoubleLeft) && (index!=-1) )
    {
        progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

        if (progress->selected && !keyIsHit(SHIFTKEY))
        {
                        //left press (select/add job)
            if((tutorial==TUTORIAL_ONLY) && tutIsBuildShipRestricted(cmShipsAvailable[index].info->shiptype) )
                return(0);

            if (keyIsHit(CM_FIVEKEY))
            {
                uword j;
                for (j=0; j<5; j++)
                {
                    if (cmIncrement(factory,(uword)index))
                        cmRightArrowActive = TRUE;
                }
            }
            else
            {
                if (cmIncrement(factory,(uword)index))
                    cmRightArrowActive = TRUE;
            }
        }
        else
        {
            if (keyIsHit(SHIFTKEY))
            {
                progress->selected = !progress->selected;
            }
            else
            {
                udword i;

                kprogress = progress;
                progress = &curshipsInProgress->progress[0];
                for (i = 0; i < TOTAL_STD_SHIPS; i++, progress++)
                {                                                   //for all classes of ships
                    progress->selected = FALSE;
                }

                kprogress->selected = TRUE;

                /*
                for (i=0; cmShipsAvailable[i].nJobs!=-1; i++)
                {

                    sip = cmSIP(index);
                    if (i == index)
                        sip->selected = TRUE;
                    else
                        sip->selected = FALSE;*/
            }
        }

        svSelectShip(cmShipsAvailable[index].info->shiptype);
        curshipview = cmShipsAvailable[index].info->shiptype;
    }
    else if ( (event == RPE_PressRight || event == RPE_DoubleRight) && (index!=-1) )
    {
        progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

        //cmDecrement(factory, (uword)index);
        if (keyIsHit(CM_FIVEKEY))
        {
            uword j;
            for (j=0; j<5; j++)
            {
                if (cmDecrement(factory,(uword)index))
                    cmLeftArrowActive = TRUE;
            }
        }
        else
        {
            if (cmDecrement(factory,(uword)index))
                cmLeftArrowActive = TRUE;

            if (keyIsHit(SHIFTKEY))
            {
                progress->selected = !progress->selected;
            }
            else
            {
                udword i;

                kprogress = progress;
                progress = &curshipsInProgress->progress[0];
                for (i = 0; i < TOTAL_STD_SHIPS; i++, progress++)
                {                                                   //for all classes of ships
                    progress->selected = FALSE;
                }

                kprogress->selected = TRUE;
            }
        }

        svSelectShip(cmShipsAvailable[index].info->shiptype);
        curshipview = cmShipsAvailable[index].info->shiptype;
    }
    else if (event == RPE_WheelUp)
    {
        cmWheelNegative();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    else if (event == RPE_WheelDown)
    {
        cmWheelPositive();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    cmDirtyShipInfo();
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : cmShipInfoDraw
    Description : Draw the available ships and construction progress
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmShipInfoDraw(featom *atom, regionhandle region)
{
    sdword x, y, index, percent;
    rectangle rect = region->rect;
    color c;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;
    bool       newline;
    sdword     numlines, startind;

    if (cmRenderEverythingCounter > 0)
    {
        cmRenderEverythingCounter--;
        feRenderEverything = TRUE;
    }

    cmShipInfoRegion = region;

    currentFont = fontMakeCurrent(cmShipListFont);

    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_PressRight |
                        RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
        regFunctionSet(region, (regionfunction)cmSelectAvailable);          //set new region handler function
        regFilterSet(region, region->flags | RPE_DoubleLeft | RPE_DoubleRight);
    }
    feStaticRectangleDraw(region);                          //draw standard rectangle

    rect.x1 -= CM_ProgressMarginRight;
    primRectSolid2(&rect, CM_BackgroundColor);

    numlines = 0;

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    y = region->rect.y0 + CM_ASMarginTop;

    newline=FALSE;
    numlines=0;

    factory = curshipsInProgress;

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= region->rect.y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
             (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
/*
            if (cmShipDisabled(cmShipsAvailable[index].type))
            {
                c = FEC_ListItemInactive;
            }
            else
*/
            if (cmShipsAvailable[index].nJobs > 0)
            {                                                   //if this ship already selected
                c = cmSelectionTextColor;
            }
            else
            {
                if (progress->selected)
                {
                    c = FEC_ListItemSelected;
                }
                else
                {
                    c = cmStandardTextColor;
                }
            }

            if (progress->selected)
            {
                rect.x0 = region->rect.x0 + CM_SelectBoxX0 + cmSelectLeftAdjust;
                rect.y0 = y + CM_SelectBoxY0 + cmSelectTopAdjust;
                rect.x1 = region->rect.x0 + CM_SelectBoxX1 + cmSelectRightAdjust;
                rect.y1 = y + CM_SelectBoxY1 + cmSelectBottomAdjust;

                primRectTranslucent2(&rect, FEC_ListItemTranslucent/*CM_SelectedColorSolid*/);
                primRectOutline2(&rect, 1, FEC_ListItemTranslucentBorder/*CM_SelectedColorOutline*/);
            }

            if (progress->nJobs > 0)
            {                                                   //if working on this class
                c = cmBuildingTextColor;

                if (progress->paused)
                    c = CM_PausedColor;

                percent = 100 - progress->timeLeft * 100 / progress->timeStart;
                rect.x0 = region->rect.x0 + 2 + cmSelectLeftAdjust; //increment past border of window
                rect.y0 = y + CM_SelectBoxY0 + cmSelectTopAdjust;   //draw top bar (progress of current ship)
                rect.y1 = y + fontHeight(" ") / 2 + cmSelectTopAdjust;
                rect.x1 = rect.x0 - 1 + cmSelectRightAdjust + percent * (region->rect.x1 -
                    region->rect.x0 - CM_ProgressMarginLeft - CM_ProgressMarginRight) / 100;
                primRectSolid2(&rect, cmProgressShipColor);

                percent = 100 - (progress->timeLeft +
                    (progress->nJobs - 1) * progress->timeStart) * 100 /
                    (progress->timeStart * progress->nJobsTotal);
                rect.y0 = rect.y1;                               //draw bottom bar (progress of total job)
                rect.y1 = y + CM_SelectBoxY1 + cmSelectBottomAdjust - 1;
                rect.x1 = rect.x0 - 1 + cmSelectRightAdjust + percent * (region->rect.x1 -
                    region->rect.x0 - CM_ProgressMarginLeft - CM_ProgressMarginRight) / 100;
                primRectSolid2(&rect, cmProgressTotalColor);
            }

            if ((taskTimeElapsed-ExceededCapsFrames < FLASH_TIMER) && (ShipNumberExceeded==index))
            {
                c = FEC_ListItemInvalid;//cmCapsExceededTextColor;
            }


            /*
            if (progress->paused)
            {
                rect.x0 = region->rect.x0 + CM_SelectBoxX0+2;
                rect.y0 = y + CM_SelectBoxY0+2;
                rect.x1 = region->rect.x0 + CM_SelectBoxX1+2;
                rect.y1 = y + CM_SelectBoxY1+2;

                primRectOutline2(&rect, 1, CM_PausedColor);
            }
              */


            //ship fancy name
            x = region->rect.x0 + CM_ASMarginLeft;
            fontPrint(x, y, c, ShipTypeToNiceStr(cmShipsAvailable[index].info->shiptype));

            newline = TRUE;

            cmDirtyShipInfo();
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            char* classString;

            c = cmClassHeadingTextColor;
            if (cmShipsAvailable[index].type == CLASS_Destroyer)
            {
                classString = strGetString(strCLASS_SuperCapital);
            }
            else
            {
                classString = ShipClassToNiceStr(cmShipsAvailable[index].type);
            }
            fontPrintf(region->rect.x0 + CM_ASHeadingMarginLeft, y,
                       c, "%s %s", classString, strGetString(strCLASS_Class));
            //next line
            newline = TRUE;
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
            numlines++;
        }
    }

    fontMakeCurrent(currentFont);
//  cmDirtyShipInfo();
}

/*-----------------------------------------------------------------------------
    Name        : cmNumberRUsDraw
    Description : Callback to draw number of RU's available
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmNumberRUsDraw(featom *atom, regionhandle region)
{
    sdword width;
    fonthandle oldfont;
    rectangle rect = region->rect;

    cmNumberRUsRegion = region;

    oldfont = fontMakeCurrent(cmShipListFont);

    primModeSet2();
    primRectSolid2(&rect, CM_BackgroundColor);

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x0, MAIN_WindowHeight - rect.y1, rect.x1 - rect.x0, rect.y1 - rect.y0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    width = fontWidthf("%d", universe.curPlayerPtr->resourceUnits);//width of number
    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - CM_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", universe.curPlayerPtr->resourceUnits);

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : cmTotalRUsDraw
    Description : Callback to draw total cost of build queue for all ship types
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmTotalRUsDraw(featom *atom, regionhandle region)
{
    sdword width;
    sdword index, RUs;
    shipsinprogress *factory;
    shipinprogress *progress;
    rectangle rect = region->rect;
    fonthandle oldfont;
    real32 RUsAlready;

    cmTotalRUsRegion = region;

    oldfont = fontMakeCurrent(cmShipListFont);

    factory = curshipsInProgress;

    primModeSet2();
    primRectSolid2(&rect, CM_BackgroundColor);

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x0, MAIN_WindowHeight - rect.y1, rect.x1 - rect.x0, rect.y1 - rect.y0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    for (index = RUs = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {
        if (cmShipsAvailable[index].itemtype==ITEM_CLASS)
        {
            continue;
        }
        progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
#if CM_CHEAP_SHIPS
        if (cmCheapShips && !multiPlayerGame)
        {
            RUs++;
        }
        else
#endif
        {
            RUs += cmShipsAvailable[index].nJobs * cmShipsAvailable[index].info->buildCost;
        }
        if (progress->nJobs > 0)
        {
            dbgAssert(progress->info != NULL);
#if CM_CHEAP_SHIPS
            if (cmCheapShips && !multiPlayerGame)
            {
                RUs += progress->nJobs;
            }
            else
#endif
            {
                RUs += progress->nJobs * progress->info->buildCost;
                RUsAlready = (real32)progress->info->buildCost * (real32)(progress->timeStart - progress->timeLeft) / (real32)progress->info->buildTime;
                RUs -= (sdword)RUsAlready;
            }
        }
    }
    width = fontWidthf("%d", RUs);                          //width of number
    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - CM_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", RUs);
    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : cmTotalShipsDraw
    Description : Callback to draw total number of ships queued
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmTotalShipsDraw(featom *atom, regionhandle region)
{
    sdword width;
    sdword index, nShips;
    shipsinprogress *factory;
    shipinprogress *progress;
    rectangle rect = region->rect;
    fonthandle oldfont;

    cmTotalShipsRegion = region;

    oldfont = fontMakeCurrent(cmShipListFont);

    factory = curshipsInProgress;

    primModeSet2();
    primRectSolid2(&rect, CM_BackgroundColor);

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x0, MAIN_WindowHeight - rect.y1, rect.x1 - rect.x0, rect.y1 - rect.y0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    for (index = nShips = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {
        if (cmShipsAvailable[index].itemtype==ITEM_SHIP)
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
            nShips += cmShipsAvailable[index].nJobs;
            nShips += progress->nJobs;                          //active jobs, too
        }
    }
    width = fontWidthf("%d", nShips);                       //width of number
    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - CM_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", nShips);
    fontMakeCurrent(oldfont);
}

bool cmCanBuildShipType(Ship *factoryship,ShipType shiptype,bool checkResearch)
{
    ShipStaticInfo *factorystatic;
    shipavailable *shipsavail;
    sdword index;

    if (factoryship == NULL)
    {
        return FALSE;
    }

    factorystatic = factoryship->staticinfo;

    if (factorystatic->shiprace == R1)
    {
        if (factorystatic->canBuildBigShips)
        {
            shipsavail = cmShipsAvailableR1Big;
        }
        else
        {
            shipsavail = cmShipsAvailableR1Reg;
        }
    }
    else if (factorystatic->shiprace == R2)
    {
        if (factorystatic->canBuildBigShips)
        {
            shipsavail = cmShipsAvailableR2Big;
        }
        else
        {
            shipsavail = cmShipsAvailableR2Reg;
        }
    }
    else if (factorystatic->shiprace == P1)
    {
        shipsavail = cmShipsAvailableP1;
    }
    else if (factorystatic->shiprace == P2)
    {
        shipsavail = cmShipsAvailableP2;
    }
    else
    {
        dbgAssert(FALSE);           // do we need to deal with P3?
        return FALSE;
    }

    for (index = 0; shipsavail[index].nJobs != -1; index++)
    {
        if (shipsavail[index].itemtype==ITEM_SHIP)
        {
            if (shipsavail[index].type == shiptype)
            {
                if (!checkResearch)
                {
                    return TRUE;
                }

                if (rmCanBuildShip(factoryship->playerowner, shiptype))
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : cmUpdateFactory
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
sdword cmUpdateFactoryNoReset(Ship* shipfactory)
{
    ShipRace race;

    if ( (shipfactory == NULL) || (!((ShipStaticInfo *)(shipfactory->staticinfo))->canBuildShips) )
    {
        shipfactory = universe.curPlayerPtr->PlayerMothership;
    }

    if (shipfactory == NULL)
    {
#if CM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nShipfactory dead.  Can't build anything!");
#endif
        return(0);
    }

    race = shipfactory->playerowner->race;

    if (!((ShipStaticInfo *)(shipfactory->staticinfo))->canBuildShips)
    {
#if CM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nThis ship can't build anything!");
#endif
        return(0);
    }

    //if (bitTest(taskData[cmBuildTask]->flags, TF_Paused))
    //{                                                       //if build task paused
    //    taskResume(cmBuildTask);                            //continue it
    //}

    curshipsInProgress = cmFindFactory(shipfactory);
    dbgAssert(curshipsInProgress != NULL);
    if (race == R1)
    {
        if (curshipsInProgress->canBuildBigShips)
        {
            cmShipsAvailable = cmShipsAvailableR1Big;
        }
        else
        {
            cmShipsAvailable = cmShipsAvailableR1Reg;
        }
    }
    else if (race == R2)
    {
        if (curshipsInProgress->canBuildBigShips)
        {
            cmShipsAvailable = cmShipsAvailableR2Big;
        }
        else
        {
            cmShipsAvailable = cmShipsAvailableR2Reg;
        }
    }
    else if (race == P1)
    {
        cmShipsAvailable = cmShipsAvailableP1;
    }
    else if (race == P2)
    {
        cmShipsAvailable = cmShipsAvailableP2;
    }
    else
    {
        dbgAssert(FALSE);
    }

    cmDirtyShipInfo();
    cmUpdateShipAvailable();

    return(1);
}

/*-----------------------------------------------------------------------------
    Name        : cmUpdateShipsAvailable
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cmUpdateShipsAvailable(void)
{
    if (cmCurrentSelect==0)
    {
        cmUpdateFactoryNoReset(universe.curPlayerPtr->PlayerMothership);
    }
    else
    {
        cmUpdateFactoryNoReset(cmCarrierX[cmCurrentSelect-1]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmUpdateFactory
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
sdword cmUpdateFactory(Ship* shipfactory)
{
    sdword index;
    ShipRace race;
    shipinprogress *sip;

    if ( (shipfactory == NULL) || (!((ShipStaticInfo *)(shipfactory->staticinfo))->canBuildShips) )
    {
        shipfactory = universe.curPlayerPtr->PlayerMothership;
    }

    if (shipfactory == NULL)
    {
#if CM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nShipfactory dead.  Can't build anything!");
#endif
        return(0);
    }

    race = shipfactory->playerowner->race;

    if (!((ShipStaticInfo *)(shipfactory->staticinfo))->canBuildShips)
    {
#if CM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nThis ship can't build anything!");
#endif
        return(0);
    }

    //if (bitTest(taskData[cmBuildTask]->flags, TF_Paused))
    //{                                                       //if build task paused
    //    taskResume(cmBuildTask);                            //continue it
    //}

    curshipsInProgress = cmFindFactory(shipfactory);
    dbgAssert(curshipsInProgress != NULL);
    if (race == R1)
    {
        if (curshipsInProgress->canBuildBigShips)
        {
            cmShipsAvailable = cmShipsAvailableR1Big;
        }
        else
        {
            cmShipsAvailable = cmShipsAvailableR1Reg;
        }
    }
    else if (race == R2)
    {
        if (curshipsInProgress->canBuildBigShips)
        {
            cmShipsAvailable = cmShipsAvailableR2Big;
        }
        else
        {
            cmShipsAvailable = cmShipsAvailableR2Reg;
        }
    }
    else if (race == P1)
    {
        cmShipsAvailable = cmShipsAvailableP1;
    }
    else if (race == P2)
    {
        cmShipsAvailable = cmShipsAvailableP2;
    }
    else
    {
        dbgAssert(FALSE);
    }

    for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {                                                       //for all available ships
        cmShipsAvailable[index].nJobs = 0;                    //no jobs queued yet

        if (cmShipsAvailable[index].itemtype==ITEM_SHIP)
        {
            cmShipsAvailable[index].info =                        //store the static info
                GetShipStaticInfo(cmShipsAvailable[index].type,
                                  race);
        }
    }


    for (index=0; (cmShipsAvailable[index].nJobs!=-1); index++)
    {
        //if (cmShipsAvailable[index].selected)
        sip = cmSIP(index);

        sip->selected = FALSE;

    }

    cmDirtyShipInfo();
    cmUpdateShipAvailable();

    return(1);
}

/*-----------------------------------------------------------------------------
    Name        : cmConstructionBegin
    Description : Region callback handler for starting the construction manager.
    Inputs      : Standard region callback.
    Outputs     : Starts up the construction manager and returns.
    Return      : void
----------------------------------------------------------------------------*/
sdword cmConstructionBegin(regionhandle region, sdword ID, udword event, udword data)
{
    sdword status = 0;
    ShipPtr bship=NULL;

    if ((playPackets) || (universePause)) return 0;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildManager)
        return 0;

    if (smZoomingIn || smZoomingOut || universe.curPlayerPtr->PlayerMothership == NULL)
    {
        return(status);
    }

    cmFillInCarrierXs();

    bship = (Ship *)ID;

    if ((bship == NULL)||((bship->shiptype!=Mothership)&&(bship->shiptype!=Carrier)))
    {
        if (cmCurrentSelect==0)
        {
            bship = universe.curPlayerPtr->PlayerMothership;
            if (universe.curPlayerPtr->PlayerMothership->shiptype == Carrier)
            {
                cmCurrentSelect = 1;
            }
            else if (universe.curPlayerPtr->PlayerMothership->shiptype == Mothership)
            {
                cmCurrentSelect = 0;
            }
        }
        else if (cmCarrierX[cmCurrentSelect-1]!=NULL)
        {
            bship = cmCarrierX[cmCurrentSelect-1];
        }
        else
        {
            bship = universe.curPlayerPtr->PlayerMothership;
            if (universe.curPlayerPtr->PlayerMothership->shiptype == Carrier)
            {
                cmCurrentSelect = 1;
            }
            else if (universe.curPlayerPtr->PlayerMothership->shiptype == Mothership)
            {
                cmCurrentSelect = 0;
            }
        }
    }
    else if (bship->shiptype == Mothership)
    {
        cmCurrentSelect = 0;
    }
    else
    {
        sdword i;

        for (i=0;i<NUM_CMCARRIERS;i++)
        {
            if (bship == cmCarrierX[i])
            {
                cmCurrentSelect = i+1;
                break;
            }
        }
    }

    if (bship == NULL)
    {
        return status;
    }

    tutGameMessage("Start_BuildManager");

    // disable rendering of main screen
    mrRenderMainScreen = FALSE;

    glcFullscreen(TRUE);

    cmRenderEverythingCounter = (tutorial == TUTORIAL_ONLY) ? 4 : 0;

    // clear the screen
    rndClear();

    // disable taskbar popup window
    tbDisable = TRUE;

    //mouseEnable();
    //mouseCursorShow();

    spLockout(SPLOCKOUT_MR);

    status = cmUpdateFactory(bship);

    if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
    }

    if (!cmScreensHandle)
    {
        feCallbackAddMultiple(cmCallback);                  //add in the callbacks
        feDrawCallbackAddMultiple(cmDrawCallback);
        cmScreensHandle = feScreensLoad(CM_FIBFile);        //load in the screen
    }

    soundEventStopSFX(0.5f);

    /* play the intro sound */
    soundEvent(NULL, UI_ManagerIntro);
    /* play the build manager ambient */
    soundEvent(NULL, UI_BuildManager);

    cmIoSaveState = ioDisable();

    cmBaseRegion = feScreenStart(region, CM_ConstructionScreen);//add new regions as siblings of current one

    cmArrowIndex = -1;

    cmLeftArrowActive  = FALSE;
    cmRightArrowActive = FALSE;

    mouseCursorShow();

    opKeyDetour = 0;//2;    //detour keys to cmBuildHotKey
    cmPrintHotKey = FALSE;//TRUE;

    updateShipView();

    cmActive = TRUE;

    return(status);
}

/*-----------------------------------------------------------------------------
    Name        : cmBuildJobsAdd
    Description : Create a number of new build jobs on the specified ship
    Inputs      : factory - where the ship is to be built
                  info - information on the ship to build
                  nJobs - number of these ships to build
    Outputs     : adds a new entry into the list of ships in progress
    Return      : void
----------------------------------------------------------------------------*/
void cmBuildJobsAdd(shipsinprogress *factory, ShipStaticInfo *info, sdword nJobs)
{
    shipinprogress *progress;
    sdword prevJobsTotal;
//    sdword index;

#if CM_VERBOSE_LEVEL >= 1
    dbgMessagef("\ncmBuildJobAdd: Building %s", ShipTypeToNiceStr(info->shiptype));
#endif
    dbgAssert(info->shiptype < TOTAL_STD_SHIPS);                 //make sure we can add a job
    progress = &factory->progress[info->shiptype];         //update the progress
#if CM_CAP_JOBS_PER_CLASS
    if (factory->progress[info->shiptype].nJobs >= CM_NumberJobsPerClass)
    {                                                       //only so much we can do man!
        return;                                             //??? do we need this???
    }
#endif
    if (progress->nJobs == 0)
    {                                                       //if no jobs currently
        progress->timeLeft = progress->timeStart = info->buildTime;
        progress->timeFraction = 0; // no fractional bits in fixed point for RU decrement
        progress->paused = FALSE;
        progress->costSoFar = 0;
    }
    progress->info = info;                                  //init the progress structure
#if CM_CAP_JOBS_PER_CLASS
    progress->nJobs = min(progress->nJobs + nJobs, CM_NumberJobsPerClass);//add the new jobs
#else
    progress->nJobs = progress->nJobs + nJobs;              //add the new jobs
#endif
    prevJobsTotal = progress->nJobsTotal;
    progress->nJobsTotal += nJobs;

    if (multiPlayerGame)
    {
        //xxx
        clWrapDeterministicBuild(CMD_START, &universe.mainCommandLayer,
                                 progress->nJobsTotal | 0x8000, info->shiptype,
                                 universe.curPlayerPtr->race,
                                 universe.curPlayerIndex,
                                 factory->ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmSelectMotherShip
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword cmSelectMotherShip(regionhandle region, sdword ID, udword event, udword data)
{
    if ((event == RPE_PressLeft) && (cmCurrentSelect!=0))
    {
        cmCurrentSelect = 0;
        cmUpdateFactory(universe.curPlayerPtr->PlayerMothership);

        dbgAssert(region!=NULL);
#ifdef DEBUG_STOMP
        regVerify(region);
#endif
        bitSet(region->status, RSF_DrawThisFrame);

//        speechEventFleet(COMM_F_BuildCenterSelected, Mothership, universe.curPlayerIndex); //@@@
        speechEventFleetVar(COMM_F_BuildCenterSelected, 0, 0, universe.curPlayerIndex);

        updateShipView();
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : cmSelectCarrierX
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword cmSelectCarrierX(regionhandle region, sdword ID, udword event, udword data, udword x)
{
    if ((event == RPE_PressLeft) && (cmCarrierX[x-1] != NULL) && (cmCurrentSelect != x))
    {
        cmCurrentSelect = x;
        cmUpdateFactory(cmCarrierX[x-1]);

        dbgAssert(region!=NULL);
#ifdef DEBUG_STOMP
        regVerify(region);
#endif
        bitSet(region->status, RSF_DrawThisFrame);

//        speechEventFleet(COMM_F_BuildCenterSelected, Carrier, universe.curPlayerIndex);
        speechEventFleetVar(COMM_F_BuildCenterSelected, 0, 1, universe.curPlayerIndex);

        updateShipView();
    }
    return(0);
}

sdword cmSelectCarrier1(regionhandle region, sdword ID, udword event, udword data)
{
    return cmSelectCarrierX(region,ID,event,data,1);
}

sdword cmSelectCarrier2(regionhandle region, sdword ID, udword event, udword data)
{
    return cmSelectCarrierX(region,ID,event,data,2);
}

sdword cmSelectCarrier3(regionhandle region, sdword ID, udword event, udword data)
{
    return cmSelectCarrierX(region,ID,event,data,3);
}

sdword cmSelectCarrier4(regionhandle region, sdword ID, udword event, udword data)
{
    return cmSelectCarrierX(region,ID,event,data,4);
}

/*-----------------------------------------------------------------------------
    Name        : cmDrawShipImage
    Description : draws the button image
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cmDrawShipImage(regionhandle region, sdword shipID)
{
    rectangle rect;
    udword usetexture;

    usetexture = (shipID == 0) ? 0 : 1;

    rect.x0 = region->rect.x0 + CM_SHIP_IMAGE_INSET;
    rect.y0 = region->rect.y0 + CM_SHIP_IMAGE_INSET;
    rect.x1 = region->rect.x1 - CM_SHIP_IMAGE_INSET;
    rect.y1 = region->rect.y1 - CM_SHIP_IMAGE_INSET;

    if (shipID == cmCurrentSelect)
        ferDrawFocusWindow(region, lw_focus);
    else
        ferDrawFocusWindow(region, lw_normal);

    if (glcActive())
    {
        lifheader* lif = cmShipImage[universe.curPlayerPtr->race][usetexture];
        glcRectSolidTexturedScaled2(&rect,
                                    lif->width, lif->height,
                                    lif->data,
                                    cmPaletted ? lif->palette : NULL,
                                    TRUE);
    }
    else
    {
        if (cmPaletted)
        {
            trPalettedTextureMakeCurrent(cmShipTexture[universe.curPlayerPtr->race][usetexture], cmShipImage[universe.curPlayerPtr->race][usetexture]->palette);
        }
        else
        {
            trRGBTextureMakeCurrent(cmShipTexture[universe.curPlayerPtr->race][usetexture]);
        }

        rndPerspectiveCorrection(FALSE);
        primRectSolidTextured2(&rect);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmMotherShipDraw
    Description : draws the mothership button
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cmMotherShipDraw(featom *atom, regionhandle region)
{
    Ship *shipfactory = universe.curPlayerPtr->PlayerMothership;

    cmMotherShipRegion = region;

    if (shipfactory != NULL && shipfactory->shiptype == Mothership)
    {
        if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
        {                                         //if region not processed yet
            region->flags = RPE_PressLeft;        //receive mouse presses from now on
            region->flags |= RPE_DrawEveryFrame;
            regFunctionSet(region, (regionfunction)cmSelectMotherShip); //set new region handler function
        }
        cmDrawShipImage(region, 0);
    }
}

void cmFillInCarrierXs(void)
{
    sdword i,insert,index;

    for (i=0;i<NUM_CMCARRIERS;i++)
    {
        cmCarrierX[i] = NULL;
    }

    cmNumPlayersCarriers = 0;

    for (insert=0,index=0;index<cmNumCarriers;index++)
    {
        if (cmCarriers[index].owner == universe.curPlayerPtr)
        {
            if (insert < NUM_CMCARRIERS)
            {
                cmCarrierX[insert++] = cmCarriers[index].ship;

                cmNumPlayersCarriers++;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmCarrierXDraw
    Description : draws the carrierX button
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cmCarrierXDraw(featom *atom, regionhandle region,udword x)
{
    sdword index, count = 0;
    cmCarrierRegions[x-1] = region;

    for (index=0;index<cmNumCarriers;index++)
    {
        if (cmCarriers[index].owner == universe.curPlayerPtr)
        {
            count++;
            if(count > (x-1))
            {
                cmCarrierX[x-1] = cmCarriers[index].ship;
                if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
                {                                         //if region not processed yet
                    region->flags = RPE_PressLeft;        //receive mouse presses from now on
                    region->flags |= RPE_DrawEveryFrame;
                    switch (x)
                    {
                        case 1: regFunctionSet(region, (regionfunction)cmSelectCarrier1); break;
                        case 2: regFunctionSet(region, (regionfunction)cmSelectCarrier2); break;
                        case 3: regFunctionSet(region, (regionfunction)cmSelectCarrier3); break;
                        case 4: regFunctionSet(region, (regionfunction)cmSelectCarrier4); break;
                        default: dbgAssert(FALSE); break;
                    }
                }
                cmDrawShipImage(region, x);
                break;
            }
        }
    }
}

void cmCarrier1Draw(featom *atom, regionhandle region)
{
    cmCarrierXDraw(atom,region,1);
}

void cmCarrier2Draw(featom *atom, regionhandle region)
{
    cmCarrierXDraw(atom,region,2);
}

void cmCarrier3Draw(featom *atom, regionhandle region)
{
    cmCarrierXDraw(atom,region,3);
}

void cmCarrier4Draw(featom *atom, regionhandle region)
{
    cmCarrierXDraw(atom,region,4);
}

ShipType cmKeyToShipType(uword key)
{
    uword i;

    for (i=0; cmShipTypes[i].key!=-1; i++)
    {
        if (cmShipTypes[i].key == key)
        {
            return (ShipType)cmShipTypes[i].ship;

        }

    }
    return 0;
}


uword cmShipTypeToKey(ShipType ship)
{
    uword i;



    for (i=0; cmShipTypes[i].key!=-1; i++)
    {
        if (cmShipTypes[i].ship == ship)
        {
            return cmShipTypes[i].key;

        }

    }
    return 0;
}


//detoured keypresses:

bool cmBuildHotKey(keyindex key, bool shift)
{
    shipinprogress *progress;
    uword index;
    uword skey;
    bool buildhotkey = FALSE;
    ShipType ship;

    if (((key>=AKEY) && (key<=ZKEY) &&
     (key!=BKEY) &&
     (key!=PKEY) &&
     (key!=CKEY) ) ||
        ((key>=F1KEY) && (key<=F12KEY))
        )
    {
        buildhotkey = TRUE;

    }
    else if ((key>=ONEKEY) && (key<=THREEKEY))
    {
        buildhotkey = TRUE;
    }

    if (buildhotkey)
    {
        if (shift)
            skey = key | CM_SHIFT;
        else
            skey = key;

        if ((ship = cmKeyToShipType(skey)) > 0)
        {
            progress = &curshipsInProgress->progress[0];
            for (index = 0; index < TOTAL_STD_SHIPS; index++, progress++)
            {
                if ( (cmShipsAvailable[index].itemtype==ITEM_SHIP) &&
                  (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
                {
                    if (cmShipsAvailable[index].type == ship)
                    {
                        if (cmIncrement(curshipsInProgress,index))
                        {
                            cmDecrement(curshipsInProgress,index);
                            //start building now
                            cmBuildJobsAdd(curshipsInProgress,
                              cmShipsAvailable[index].info,
                              1 /*cmShipsAvailable[index].nJobs*/ );

                        }
                        else
                        {
                            //sound effect
                        }
                        break;

                    }

                }
            }
        }
        return TRUE;
    }
    else
        return FALSE;      //don't bypass - it's a standard region key

}

void cmDeterministicStartup(void)
{
    listInit(&cmDetermProgress);
}

void cmDeterministicShutdown(void)
{
    listDeleteAll(&cmDetermProgress);
}

void cmDeterministicReset(void)
{
    listDeleteAll(&cmDetermProgress);
}

crc32 cmDeterministicCRC(void)
{
    Node* node;
    cmDetermProgress_t* block;
    cmDetermProgress_t* ptr;
    cmDetermProgress_t* curNode;
    sdword size;
    crc32 crc;

    if (cmDetermProgress.num == 0)
    {
        return 0;
    }

    size = cmDetermProgress.num * sizeof(cmDetermProgress_t);
    block = (cmDetermProgress_t*)memAlloc(size, "temp dblock", Volatile);
    ptr  = block;
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        curNode = (cmDetermProgress_t*)listGetStructOfNode(node);

        //copy node into block
        memcpy(ptr, curNode, sizeof(cmDetermProgress_t));

        //clear non-deterministic members
        memset(&ptr->node, 0, sizeof(Node));
        ptr->creator = NULL;

        //advance
        ptr++;
        node = node->next;
    }

    crc = crc32Compute((ubyte*)block, size);

    memFree(block);

    return crc;
}

void cmDeterministicBuildDisplay(void)
{
    Node* node;
    cmDetermProgress_t* dprog;
    sdword y, height;

    height = fontHeight(NULL);

    y = 4 * height;
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);

        fontPrintf(
            0, y, colReddish,
            "pl %d, num %d, type %s, race %s, tleft %d, ru %d",
            (sdword)dprog->playerIndex,
            dprog->numShips,
            ShipTypeToStr(dprog->shipType),
            ShipRaceToStr(dprog->shipRace),
            dprog->ticksLeft,
            universe.players[dprog->playerIndex].resourceUnits
            );

        //next line, next node
        y += height;
        if ((y + height) > MAIN_WindowHeight)
        {
            //don't bother w/ off-screen lines
            break;
        }
        node = node->next;
    }
}

bool cmDeterministicSanityCheck(cmDetermProgress_t* dprog)
{
    bool rval = TRUE;
    Player* player = &universe.players[dprog->playerIndex];

    //check race
    if (dprog->shipRace != player->race)
    {
        rval = FALSE;
    }

    //check type
    if (dprog->shipType < STD_FIRST_SHIP ||
        dprog->shipType > STD_LAST_SHIP)
    {
        rval = FALSE;
    }

    return rval;
}

void cmDeterministicBuildStart(
    sdword numShips, ShipType shipType, ShipRace shipRace, sdword playerIndex, ShipPtr creator)
{
    Node* node;
    cmDetermProgress_t* dprog;
    ShipStaticInfo* info;

    //look for existing match in cmDetermProgress
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);
        if (dprog->shipType == shipType &&
            dprog->shipRace == shipRace &&
            dprog->playerIndex == playerIndex &&
            dprog->creator == creator)
        {
            if (bitTest(numShips, 0x8000))
            {
                //hi-bit (of uword) set, we're an absolute number
                bitClear(numShips, 0x8000);
                dprog->numShips = numShips;
                if (numShips <= 0)
                {
                    //we've run out of jobs in the batch
                    listDeleteNode(node);
                }
                return;
            }
            else
            {
                //match found, increment count & exit
                if (bitTest(numShips, 0x4000))
                {
                    bitClear(numShips, 0x4000);
                    if (dprog->numShips > 1)
                    {
                        dprog->numShips -= numShips;
                    }
                }
                else
                {
                    dprog->numShips += numShips;
                }
                return;
            }
        }

        //move on
        node = node->next;
    }

    //not found, create new node
    info = GetShipStaticInfo(shipType, shipRace);
    dprog = (cmDetermProgress_t*)memAlloc(sizeof(cmDetermProgress_t), "dprog_t", NonVolatile);
    if (bitTest(numShips, 0x8000))
    {
        bitClear(numShips, 0x8000);
    }
    if (bitTest(numShips, 0x4000))
    {
        //we're subtracting from nothing
        return;
    }
    dprog->numShips = numShips;
    dprog->shipType = shipType;
    dprog->shipRace = shipRace;

    //@todo is buildTime expressed in universe ticks or seconds ?
    dprog->ticksForOne = info->buildTime;
    dprog->ticksLeft = dprog->ticksForOne;

    dprog->costPerShip = info->buildCost;
    dprog->costPerTick = (real32)dprog->costPerShip / (real32)dprog->ticksForOne;
    dprog->costSoFar = 0.0f;

    dprog->playerIndex = playerIndex;
    dprog->creator = creator;
    dprog->paused = FALSE;

    if (cmDeterministicSanityCheck(dprog))
    {
        listAddNode(&cmDetermProgress, &dprog->node, dprog);
    }
}

void cmAdjustRU(cmDetermProgress_t* dprog, sdword amount)
{
    Player* player = &universe.players[dprog->playerIndex];
    player->resourceUnits += amount;

    universe.gameStats.playerStats[player->playerIndex].totalResourceUnitsSpent += min(player->resourceUnits, ABS(amount));

    if (player->resourceUnits < 0)
    {
        player->resourceUnits = 0;
    }
}

void cmDeterministicBuildCancel(ShipType shipType, ShipRace shipRace, sdword playerIndex, ShipPtr creator)
{
    Node* node;
    cmDetermProgress_t* dprog;

    //look for match
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);
        if (dprog->shipType == shipType &&
            dprog->shipRace == shipRace &&
            dprog->playerIndex == playerIndex &&
            dprog->creator == creator)
        {
            //match found, cancel & refund RUs
            cmAdjustRU(dprog, (sdword)(dprog->costSoFar * cmFloatRefundRatio));
            listDeleteNode(node);
            return;
        }

        //move on
        node = node->next;
    }
}

void cmDeterministicBuildPause(
    bool pause, ShipType shipType, ShipRace shipRace, sdword playerIndex, ShipPtr creator)
{
    Node* node;
    cmDetermProgress_t* dprog;

    //look for match
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);
        if (dprog->shipType == shipType &&
            dprog->shipRace == shipRace &&
            dprog->playerIndex == playerIndex &&
            dprog->creator == creator)
        {
            //match found, adjust pause state
            dprog->paused = pause;
        }

        //next node
        node = node->next;
    }
}

void cmDeterministicBuild(udword command, sdword numShips,
                          ShipType shipType, ShipRace shipRace,
                          uword playerIndex, ShipPtr creator)
{
    switch (command)
    {
    case CMD_START:
        cmDeterministicBuildStart(numShips, shipType, shipRace, playerIndex, creator);
        break;

    case CMD_CANCEL:
        cmDeterministicBuildCancel(shipType, shipRace, playerIndex, creator);
        break;

    case CMD_PAUSE:
        cmDeterministicBuildPause(TRUE, shipType, shipRace, playerIndex, creator);
        break;

    case CMD_UNPAUSE:
        cmDeterministicBuildPause(FALSE, shipType, shipRace, playerIndex, creator);
        break;

    default:
        dbgFatalf(DBG_Loc, "unknown dbuild command: %u", command);
    }
}

void cmDeterministicBuildProcess(void)
{
    Node* node;
    Node* curNode;
    cmDetermProgress_t* dprog;
    Player* player;

    if (universePause)
    {
        //universe is paused, we won't be building anything
        return;
    }

    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);
        player = &universe.players[dprog->playerIndex];
        if (player->playerState != PLAYER_ALIVE)
        {
            //dead player, clear batch
            curNode = node;
            node = node->next;
            listDeleteNode(curNode);
            continue;
        }

        //advance
        curNode = node;
        node = node->next;

        if ((sdword)dprog->numShips <= 0)
        {
            listDeleteNode(curNode);
            continue;
        }

        if (!dprog->paused)
        {
            //increment cost, decrement RUs
            sdword lastCost, newCost;
            lastCost = (sdword)dprog->costSoFar;
            newCost = (sdword)(dprog->costSoFar + dprog->costPerTick);

            if (player->resourceUnits == 0)
            {
                continue;
            }

            if (newCost != lastCost)
            {
                sdword amount = newCost - lastCost;
                //only adjust RUs if we have enough
                cmAdjustRU(dprog, -amount);

            }

            dprog->costSoFar += dprog->costPerTick;
            dprog->ticksLeft--;

            if (dprog->ticksLeft <= 0)
            {
                //finished, make the ship appear
                clCreateShip(&universe.mainCommandLayer,
                             dprog->shipType,
                             dprog->shipRace,
                             (uword)dprog->playerIndex,
                             dprog->creator);

                //@todo is this adjustment correct ?
                if (player->resourceUnits > 0)
                {
                    cmAdjustRU(dprog, -1);
                }

                //back to 0 cost for current ship
                dprog->costSoFar = 0.0f;

                dprog->numShips--;
                if (dprog->numShips == 0)
                {
                    //no more ships, delete progress node
                    listDeleteNode(curNode);
                }
                else
                {
                    //still more to go, reset tick counter for next ship
                    dprog->ticksLeft = dprog->ticksForOne;
                }
            }
        }
    }
}

/*=============================================================================
    Save Game Stuff:
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

#define RaceAndTypeToRaceType(race,type) ( ((race)<<16) + (type) )
#define GetRaceFromRaceType(racetype) ( ((racetype)>>16) & 0x0000ffff )
#define GetTypeFromRaceType(racetype) ( (racetype) & 0x0000ffff )

sdword ConvertShipStaticInfoToRaceType(ShipStaticInfo *shipstatic)
{
    if (shipstatic == NULL)
    {
        return -1;
    }
    else
    {
        return RaceAndTypeToRaceType(shipstatic->shiprace,shipstatic->shiptype);
    }
}

ShipStaticInfo *ConvertRaceTypeToShipStaticInfo(sdword racetype)
{
    if (racetype == -1)
    {
        return NULL;
    }
    else
    {
        return GetShipStaticInfo(GetTypeFromRaceType(racetype),GetRaceFromRaceType(racetype));
    }
}

void Save_shipsinprogress(shipsinprogress *sinprogress)
{
    SaveChunk *chunk;
    shipsinprogress *savecontents;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SHIPSINPROGRESS,sizeof(shipsinprogress),sinprogress);
    savecontents = (shipsinprogress *)chunkContents(chunk);

    savecontents->ship = (Ship *)SpaceObjRegistryGetID((SpaceObj *)savecontents->ship);
    dbgAssert((sdword)savecontents->ship != -1);
    for (i=0;i<TOTAL_STD_SHIPS;i++)
    {
        savecontents->progress[i].info = (ShipStaticInfo *)ConvertShipStaticInfoToRaceType(savecontents->progress[i].info);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveConsMgr()
{
    sdword num = listofShipsInProgress.num;
    sdword cur = 0;
    Node *node = listofShipsInProgress.head;

    SaveInfoNumber(num);

    while (node != NULL)
    {
        cur++;
        Save_shipsinprogress(listGetStructOfNode(node));

        node = node->next;
    }

    dbgAssert(cur == num);

    SaveInfoNumber(cmNumCarriers);
    if (cmNumCarriers > 0)
    {
        SaveChunk *chunk;
        CarrierInfo *savecontents;
        sdword i;

        chunk = CreateChunk(BASIC_STRUCTURE,sizeof(CarrierInfo) * cmNumCarriers,cmCarriers);
        savecontents = (CarrierInfo *)chunkContents(chunk);

        for (i=0;i<cmNumCarriers;i++)
        {
            savecontents[i].ship = (Ship *)SpaceObjRegistryGetID((SpaceObj *)savecontents[i].ship);
            dbgAssert((sdword)savecontents[i].ship != -1);
            savecontents[i].owner = (Player *)SavePlayerToPlayerIndex(savecontents[i].owner);
        }

        SaveThisChunk(chunk);
        memFree(chunk);
    }
}

shipsinprogress *Load_shipsinprogress()
{
    SaveChunk *chunk;
    shipsinprogress *sinprogress;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SHIPSINPROGRESS,sizeof(shipsinprogress));
    sinprogress = memAlloc(sizeof(shipsinprogress),"sinprogress",NonVolatile);
    memcpy(sinprogress,chunkContents(chunk),sizeof(shipsinprogress));

    memFree(chunk);

    sinprogress->ship = SpaceObjRegistryGetShip((sdword)sinprogress->ship);
    dbgAssert(sinprogress->ship != NULL);

    for (i=0;i<TOTAL_STD_SHIPS;i++)
    {
        sinprogress->progress[i].info = ConvertRaceTypeToShipStaticInfo((sdword)sinprogress->progress[i].info);
    }

    return sinprogress;
}

void LoadConsMgr()
{
    sdword num;
    sdword i;
    shipsinprogress *sinprogress;

    num = LoadInfoNumber();

    dbgAssert(listofShipsInProgress.num == 0);
    listInit(&listofShipsInProgress);

    for (i=0;i<num;i++)
    {
        sinprogress = Load_shipsinprogress();
        listAddNode(&listofShipsInProgress,&sinprogress->node,sinprogress);
    }

    cmNumCarriers = LoadInfoNumber();
    if (cmNumCarriers > 0)
    {
        SaveChunk *chunk;
        sdword size = sizeof(CarrierInfo) * cmNumCarriers;

        chunk = LoadNextChunk();
        VerifyChunk(chunk,BASIC_STRUCTURE,size);

        cmCarriers = memAlloc(size, "cmCarriers", NonVolatile);
        memcpy(cmCarriers,chunkContents(chunk),size);

        for (i=0;i<cmNumCarriers;i++)
        {
            cmCarriers[i].ship = SpaceObjRegistryGetShip((sdword)cmCarriers[i].ship);
            cmCarriers[i].owner = SavePlayerIndexToPlayer(cmCarriers[i].owner);
        }

        memFree(chunk);
    }

    cmFillInCarrierXs();
}

void SaveConsMgrDetermListCB(void *stuff)
{
    cmDetermProgress_t copy = *((cmDetermProgress_t *)stuff);

    copy.creator = (ShipPtr)SpaceObjRegistryGetID((SpaceObj *)copy.creator);

    SaveStructureOfSize(&copy,sizeof(copy));
}

void LoadConsMgrDetermListCB(LinkedList *list)
{
    cmDetermProgress_t *dp = LoadStructureOfSize(sizeof(cmDetermProgress_t));
    dp->creator = SpaceObjRegistryGetShip((sdword)dp->creator);

    listAddNode(list,&dp->node,dp);
}

#define BUILD_OPTIONAL_FLAG     0xb11db11d

void SaveConsMgrDetermOptional()
{
    if (cmDetermProgress.num == 0)
    {
        return;         // don't save anything - optional block
    }

    SaveInfoNumber(BUILD_OPTIONAL_FLAG);

    SaveLinkedListOfStuff(&cmDetermProgress,SaveConsMgrDetermListCB);
}

void LoadConsMgrDetermOptional()
{
    sdword optional;

    if (LoadInfoNumberOptional(&optional) && (optional == BUILD_OPTIONAL_FLAG))
    {
        LoadLinkedListOfStuff(&cmDetermProgress,LoadConsMgrDetermListCB);
    }
    else
    {
        listInit(&cmDetermProgress);
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"
