/*=============================================================================
    Name    : Sensors.c
    Purpose : Code for handling and rendering the sensors manager.

    Created 10/8/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include "glinc.h"
#include "Types.h"
#include "Debug.h"
#include "Memory.h"
#include "prim2d.h"
#include "prim3d.h"
#include "mainrgn.h"
#include "PiePlate.h"
#include "utility.h"
#include "font.h"
#include "Region.h"
#include "FEFlow.h"
#include "Camera.h"
#include "CameraCommand.h"
#include "Vector.h"
#include "Vector.h"
#include "Universe.h"
#include "UnivUpdate.h"
#include "render.h"
#include "mouse.h"
#include "Select.h"
#include "ShipSelect.h"
#include "CommandWrap.h"
#include "Teams.h"
#include "TaskBar.h"
#include "SoundEvent.h"
#include "Blobs.h"
#include "light.h"
#include "FastMath.h"
#include "NIS.h"
#include "Sensors.h"
#include "ProximitySensor.h"
#include "SinglePlayer.h"
#include "Probe.h"
#include "Strings.h"
#include "mainrgn.h"
#include "main.h"
#include "InfoOverlay.h"
#include "Alliance.h"
#include "StatScript.h"
#include "Ping.h"
#include "Tactical.h"
#include "Teams.h"
#include "Tutor.h"
#include "MultiplayerGame.h"
#include "Bounties.h"
#include "BTG.h"
#include "glcaps.h"
#include "Subtitle.h"
#include "SaveGame.h"
#include "Randy.h"
#include "Transformer.h"
#include "Battle.h"
#include "GravWellGenerator.h"
#include "Shader.h"
#include "devstats.h"

//located in mainrg.c
//falko's fault...not mine..long story
void toFieldSphereDraw(ShipPtr ship,real32 radius, real32 scale);


void (*smHoldLeft)(void);
void (*smHoldRight)(void);

/*=============================================================================
    Data:
=============================================================================*/
//set this to TRUE if you want it to be the Fleet Intel version of the SM
bool smFleetIntel = FALSE;

//toggles for what to display
Camera smCamera;
Camera smTempCamera;

sdword smTacticalOverlay = FALSE;
sdword smResources = TRUE;
sdword smNonResources = TRUE;
bool smFocusOnMothershipOnClose = FALSE;

//arrays of stars for quick star rendering
extern ubyte* stararray;

extern color HorseRaceDropoutColor;

extern udword gDevcaps2;

//region handle for the sensors manager viewport
regionhandle smViewportRegion;
regionhandle smBaseRegion;
rectangle smViewRectangle;

sdword smClicking = FALSE;

//table of functions to draw a mission sphere at varying levels of visibility
void smSelectHold(void);
//point specification stuffs
/*
sdword smPointSpecMode = SPM_Idle;
sdword smCrosshairSize = SM_CrosshairSize;
sdword smMousePauseX;
sdword smMousePauseY;
*/
//colors for differing sphere statuses
color spcNormal   = SPC_Normal;
color spcSelected = SPC_Selected;
color spcTeamColor= SPC_TeamColor;

// color for printing the A for alliances
color alliescolor = colRGB(255,255,0);

//list of blobs encompassing all objects in universe
//LinkedList smBlobList;
//BlobProperties smBlobProperties;

//fog-of-war sub-blob stuff
BlobProperties smFOWBlobProperties;
real32 smFOW_AsteroidK1;
real32 smFOW_AsteroidK2;
real32 smFOW_AsteroidK3;
real32 smFOW_AsteroidK4;
real32 smFOW_DustGasK2;
real32 smFOW_DustGasK3;
real32 smFOW_DustGasK4;
real32 smFOWBlobUpdateTime = SM_FOWBlobUpdateTime;

//global flag which says the sensors manager is active
bool smSensorsActive = FALSE;

bool smSensorsDisable = FALSE;

//for updating blobs
sdword smRenderCount;
color smBackgroundColor = colFuscia, smIntBackgroundColor;
color smBlobColor = SM_BlobColor;
color smBlobColorHigh = SM_BlobColorHigh;
color smBlobColorLow = SM_BlobColorLow;
color smIntBlobColor;
color smIntBlobColorHigh;
color smIntBlobColorLow;

//for zooming
real32 smZoomTime;
region **smScrollListLeft, **smScrollListTop, **smScrollListRight, **smScrollListBottom;
sdword smScrollCountLeft, smScrollCountTop, smScrollCountRight, smScrollCountBottom;
sdword smScrollDistLeft, smScrollDistTop, smScrollDistRight, smScrollDistBottom;
sdword smScrollLeft, smScrollTop, smScrollRight, smScrollBottom;
vector smLookStart, smLookEnd;
vector smEyeStart, smEyeEnd;
bool8 smZoomingIn = FALSE, smZoomingOut = FALSE, smFocus = FALSE, smFocusTransition = FALSE;
FocusCommand *smFocusCommand = NULL;

//for selections
rectangle smSelectionRect;
//blob *smLastClosestBlob = NULL;

blob *closestBlob = NULL;
blob *probeBlob = NULL;
//for enabling 'ghost mode' after a player dies
bool smGhostMode = FALSE;
bool ioSaveState;

//option for fuzzy sensors blobs
sdword smFuzzyBlobs = TRUE;

smblurry smBlurryArray[SM_BlurryArraySize];
sdword smBlurryIndex = 0;

//cursor text font, from mouse.c
extern fonthandle mouseCursorFont;
color smCurrentWorldPlaneColor;
real32 smCurrentCameraDistance;

//switch for 'instant' transitions to sensors manager
bool smInstantTransition = FALSE;
real32 smCurrentZoomLength;
real32 smCurrentMainViewZoomLength;

//for horizon tick marks
smticktext smTickText[SM_MaxTicksOnScreen];
sdword smTickTextIndex;

//for panning the world plane about whilley-nilley
vector smCameraLookatPoint = {0.0f, 0.0f, 0.0f};
vector smCameraLookVelocity = {0.0f, 0.0f, 0.0f};
bool smCentreWorldPlane = FALSE;

//info for sorting the blobs
blob **smBlobSortList = NULL;
sdword smBlobSortListLength = 0;
sdword smNumberBlobsSorted = 0;

//for multiplayer hyperspace
sdword MP_HyperSpaceFlag=FALSE;

//for sensors weirdness
#define NUM_WEIRD_POINTS         600
#define MAX_REPLACEMENTS         5
#define WEIRDUNIVERSESTRETCH     5/4
typedef struct
{
    vector location;
    color  color;
    real32 fade;
    bool   ping;
} weirdStruct;

static weirdStruct smWeird[NUM_WEIRD_POINTS];
udword   smSensorWeirdness=0;

//region for hyperspace button
regionhandle smHyperspaceRegion = NULL;

//what ships have a TO in the SM
ubyte smShipTypeRenderFlags[TOTAL_NUM_SHIPS] =
{
    SM_TO,                                      //AdvanceSupportFrigate
    0,                                          //AttackBomber
    SM_TO | SM_Mesh,                            //Carrier
    0,                                          //CloakedFighter
    SM_TO,                                      //CloakGenerator
    SM_TO,                                      //DDDFrigate
    0,                                          //DefenseFighter
    SM_TO,                                      //DFGFrigate
    SM_TO,                                      //GravWellGenerator
    0,                                          //HeavyCorvette
    SM_TO | SM_Mesh,                            //HeavyCruiser
    0,                                          //HeavyDefender
    0,                                          //HeavyInterceptor
    SM_TO,                                      //IonCannonFrigate
    0,                                          //LightCorvette
    0,                                          //LightDefender
    0,                                          //LightInterceptor
    0,                                          //MinelayerCorvette
    SM_TO,                                      //MissileDestroyer
    SM_Mesh | SM_Exclude,                       //Mothership
    0,                                          //MultiGunCorvette
    SM_Exclude,                                 //Probe
    SM_Exclude,                                 //ProximitySensor
    0,                                          //RepairCorvette
    SM_TO,                                      //ResearchShip
    SM_TO,                                      //ResourceCollector
    SM_TO,                                      //ResourceController
    0,                                          //SalCapCorvette
    SM_TO,                                      //SensorArray
    SM_TO,                                      //StandardDestroyer
    SM_TO,                                      //StandardFrigate
    SM_Exclude,                                 //Drone
    SM_Exclude,                                 //TargetDrone
    SM_Mesh,                                    //HeadShotAsteroid
    SM_Mesh | SM_Exclude,                       //CryoTray
    0,                                          //P1Fighter
    SM_TO,                                      //P1IonArrayFrigate
    0,                                          //P1MissileCorvette
    SM_TO | SM_Mesh,                            //P1Mothership
    0,                                          //P1StandardCorvette
    0,                                          //P2AdvanceSwarmer
    SM_TO,                                      //P2FuelPod
    SM_TO | SM_Mesh,                            //P2Mothership
    SM_TO,                                      //P2MultiBeamFrigate
    0,                                          //P2Swarmer
    SM_TO,                                      //P3Destroyer
    SM_TO,                                      //P3Frigate
    SM_TO | SM_Mesh,                            //P3Megaship
    SM_TO | SM_Mesh,                            //FloatingCity
    SM_TO,                                      //CargoBarge
    SM_TO | SM_Mesh,                            //MiningBase
    SM_TO | SM_Mesh,                            //ResearchStation
    0,                                          //JunkYardDawg
    SM_TO,                                      //JunkYardHQ
    0,                                          //Ghostship
    0,                                          //Junk_LGun
    0,                                          //Junk_SGun
    0,                                          //ResearchStationBridge
    0                                           //ResearchStationTower
};
ubyte smDerelictTypeMesh[NUM_DERELICTTYPES] =
{
    0,                                          //AngelMoon,
    0,                                          //AngelMoon_clean,
    0,                                          //Crate,
    SM_Mesh,                                    //FragmentPanel0a,
    SM_Mesh,                                    //FragmentPanel0b,
    SM_Mesh,                                    //FragmentPanel0c,
    SM_Mesh,                                    //FragmentPanel1,
    SM_Mesh,                                    //FragmentPanel2,
    SM_Mesh,                                    //FragmentPanel3,
    SM_Mesh,                                    //FragmentStrut,
    0,                                          //Homeworld,
    0,                                          //LifeBoat,
    0,                                          //PlanetOfOrigin,
    0,                                          //PlanetOfOrigin_scarred,
    0,                                          //PrisonShipOld,
    0,                                          //PrisonShipNew,
    SM_Mesh,                                    //Scaffold,
    SM_Mesh,                                    //Scaffold_scarred,
    SM_Mesh,                                    //ScaffoldFingerA_scarred,
    SM_Mesh,                                    //ScaffoldFingerB_scarred,
    0,                                          //Shipwreck,
    //pre-revision ships as junkyard derelicts:
    0,                                          //JunkAdvanceSupportFrigate,
    SM_Mesh,                                    //JunkCarrier,
    0,                                          //JunkDDDFrigate,
    0,                                          //JunkHeavyCorvette,
    SM_Mesh,                                    //JunkHeavyCruiser,
    0,                                          //JunkIonCannonFrigate,
    0,                                          //JunkLightCorvette,
    0,                                          //JunkMinelayerCorvette,
    0,                                          //JunkMultiGunCorvette,
    0,                                          //JunkRepairCorvette,
    0,                                          //JunkResourceController,
    0,                                          //JunkSalCapCorvette,
    0,                                          //JunkStandardFrigate,
    0,                                          //Junk0_antenna
    0,                                          //Junk0_fin1
    0,                                          //Junk0_fin2
    0,                                          //Junk0_GunAmmo
    0,                                          //Junk0_panel
    0,                                          //Junk0_sensors
    0,                                          //Junk1_partA
    0,                                          //Junk1_partB
    0,                                          //Junk1_shell
    0,                                          //Junk1_strut
    0,                                          //Junk2_panelA
    0,                                          //Junk2_panelB
    0,                                          //Junk2_panelC
    0,                                          //Junk2_panelD
    0,                                          //Junk2_shipwreck
    0,                                          //Junk3_Boiler
    0,                                          //Junk3_BoilerCasing
    0,                                          //M13PanelA
    0,                                          //M13PanelB
    0,                                          //M13PanelC
                                                    //hyperspace gate dummy derelict
    0,                                          //HyperspaceGate

};

GLboolean smBigPoints;

/*-----------------------------------------------------------------------------
    tweakables for the sensors manager from sensors.script
-----------------------------------------------------------------------------*/
//for blobs
sdword smBlobUpdateRate         = SM_BlobUpdateRate;
real32 smClosestMargin          = SM_ClosestMargin;
real32 smFarthestMargin         = SM_FarthestMargin;
//for selections
real32 smSelectedFlashSpeed     = SM_SelectedFlashSpeed;
real32 smFocusScalar            = SM_FocusScalar;
real32 smFocusRadius            = SM_FocusRadius;
sdword smSelectionWidth         = SM_SelectionWidth;
sdword smSelectionHeight        = SM_SelectionHeight;
//zooming in/out
real32 smZoomLength             = SM_ZoomLength;
real32 smMainViewZoomLength     = SM_MainViewZoomLength;
//for tactical overlay
color smPlayerListTextColor     = SM_PlayerListTextColor;
color smCursorTextColor         = SM_CursorTextColor;
color smTOColor                 = SM_TOColor;
sdword smPlayerListTextMargin   = SM_PlayerListTextMargin;
sdword smPlayerListMarginX      = SM_PlayerListMarginX;
sdword smPlayerListMarginY      = SM_PlayerListMarginY;
sdword smTOBottomCornerX        = SM_TOBottomCornerX;
sdword smTOBottomCornerY        = SM_TOBottomCornerY;
sdword smTOLineSpacing          = SM_TOLineSpacing;
real32 smTORadius               = 1.0f;
//for world plane
color smWorldPlaneColor         = SM_WorldPlaneColor;
color smMovePlaneColor          = SM_MovePlaneColor;
color smHeightCircleColor       = SM_HeightCircleColor;
real32 smWorldPlaneDistanceFactor = SM_WorldPlaneDistanceFactor;
sdword smWorldPlaneSegments     =  SM_WorldPlaneSegments;
real32 smMovementWorldPlaneDim  = SM_MovementWorldPlaneDim;
real32 smRadialTickStart        = SM_RadialTickStart;
real32 smRadialTickInc          = SM_RadialTickInc;
real32 smTickInnerMult          = SM_TickInnerMult;
real32 smTickOuterMult          = SM_TickOuterMult;
real32 smTickExtentFactor       = SM_TickExtentFactor;
real32 smBlobCircleSize         = SM_BlobCircleSize;
real32 smShortFootLength        = SM_ShortFootLength;
real32 smBackgroundDim          = SM_BackgroundDim;
//for horizon ticks
real32 smHorizTickAngle =       SM_HorizTickAngle;
real32 smHorizTickVerticalFactor = SM_HorizTickVerticalFactor;
real32 smHorizonLineDistanceFactor =  SM_HorizonLineDistanceFactor;
real32 smHorizonTickHorizFactor = SM_HorizonTickHorizFactor;
sdword smTickTextSpacing        = SM_TickTextSpacing;
//for movement/selections
real32 smTickLength = SM_TickLength;
real32 smClosestDistance        = SM_ClosestDistance;
//for rendering
real32 smProjectionScale        = SM_ProjectionScale;
//for hot key groups
sdword smHotKeyOffsetX          = 0;
sdword smHotKeyOffsetY          = 0;
//for panning the world plane about whilley-nilley
real32 smPanSpeedMultiplier     = SM_PanSpeedMultiplier;
real32 smPanTrack               = SM_PanTrack;
real32 smPanEvalThreshold       = SM_PanEvalThreshold;
real32 smPanUnivExtentMult      = SM_PanUnivExtentMult;
sdword smCursorPanX             = SM_CursorPanX;
sdword smCursorPanY             = SM_CursorPanY;
sdword smMaxFrameTicks          = SM_MaxFrameTicks;

/*-----------------------------------------------------------------------------
    tweakables for sensors manager from level files
-----------------------------------------------------------------------------*/
//depth cuing
real32 smDepthCueRadius = SM_DepthCueRadius;
real32 smDepthCueStartRadius = SM_DepthCueStartRadius;
real32 smCircleBorder = SM_CircleBorder;
real32 smZoomMax = SM_ZoomMax;
real32 smZoomMin = SM_ZoomMin;
real32 smZoomMinFactor = SM_ZoomMinFactor;
real32 smZoomMaxFactor = SM_ZoomMaxFactor;
real32 smInitialDistance = SM_InitialDistance;
real32 smUniverseSizeX = SM_UniverseSizeX;
real32 smUniverseSizeY = SM_UniverseSizeY;
real32 smUniverseSizeZ = SM_UniverseSizeZ;

/*-----------------------------------------------------------------------------
    Tweakables for the fleet intel version of the SM
-----------------------------------------------------------------------------*/
real32 smSkipFadeoutTime = SM_SkipFadeoutTime;

/*-----------------------------------------------------------------------------
    Tweak table for sensors.script
-----------------------------------------------------------------------------*/
scriptEntry smTweaks[] =
{
    //sensors manager tweaks
    makeEntry(smBlobUpdateRate      , scriptSetSdwordCB),
    makeEntry(smSelectedFlashSpeed  , scriptSetReal32CB),
    makeEntry(smFocusScalar         , scriptSetReal32CB),
    makeEntry(smFocusRadius         , scriptSetReal32CB),
    makeEntry(smZoomLength          , scriptSetReal32CB),
    makeEntry(smMainViewZoomLength  , scriptSetReal32CB),
    makeEntry(smPlayerListTextColor , scriptSetRGBCB),
    makeEntry(smCursorTextColor     , scriptSetRGBCB),
    makeEntry(smTOColor             , scriptSetRGBCB),
    makeEntry(smWorldPlaneColor     , scriptSetRGBCB),
    makeEntry(smPlayerListTextMargin, scriptSetSdwordCB),
    makeEntry(smPlayerListMarginX   , scriptSetSdwordCB),
    makeEntry(smPlayerListMarginY   , scriptSetSdwordCB),
    makeEntry(smTOBottomCornerX     , scriptSetSdwordCB),
    makeEntry(smTOBottomCornerY     , scriptSetSdwordCB),
    makeEntry(smTOLineSpacing       , scriptSetSdwordCB),
    makeEntry(smTickLength          , scriptSetReal32CB),
    makeEntry(smClosestDistance     , scriptSetReal32CB),
    makeEntry(smProjectionScale     , scriptSetReal32CB),
    makeEntry(smSelectionWidth      , scriptSetSdwordCB),
    makeEntry(smSelectionHeight     , scriptSetSdwordCB),
    makeEntry(smZoomMaxFactor       , scriptSetReal32CB),
    makeEntry(smZoomMinFactor       , scriptSetReal32CB),
    makeEntry(smHotKeyOffsetX       , scriptSetSdwordCB),
    makeEntry(smHotKeyOffsetY       , scriptSetSdwordCB),
    makeEntry(smClosestMargin       , scriptSetReal32CB),
    makeEntry(smFarthestMargin      , scriptSetReal32CB),
    makeEntry(smBlobCircleSize      , scriptSetReal32CB),
    makeEntry(smShortFootLength     , scriptSetReal32CB),
    makeEntry(smBlobColor           , scriptSetRGBCB),
    makeEntry(smBlobColorHigh       , scriptSetRGBCB),
    makeEntry(smBlobColorLow        , scriptSetRGBCB),
    makeEntry(smTORadius            , scriptSetReal32CB),
    makeEntry(smPanSpeedMultiplier  , scriptSetReal32CB),
    makeEntry(smPanTrack            , scriptSetReal32CB),
    makeEntry(smPanEvalThreshold    , scriptSetReal32CB),
    makeEntry(smCursorPanX          , scriptSetSdwordCB),
    makeEntry(smCursorPanY          , scriptSetSdwordCB),
    makeEntry(smMaxFrameTicks       , scriptSetSdwordCB),
    makeEntry(smSkipFadeoutTime     , scriptSetReal32CB),
    makeEntry(smWorldPlaneDistanceFactor, scriptSetReal32CB),
    makeEntry(smPanUnivExtentMult   , scriptSetReal32CB),
//    { "smBobDensityLow",            scriptSetReal32CB, &smBlobProperties.bobDensityLow },
//    { "smBobDensityHigh",           scriptSetReal32CB, &smBlobProperties.bobDensityHigh },
//    { "smBobStartSphereSize",       scriptSetReal32CB, &smBlobProperties.bobStartSphereSize },
//    { "smBobRadiusCombineMargin",   scriptSetReal32CB, &smBlobProperties.bobRadiusCombineMargin },
//    { "smBobOverlapFactor",         scriptSetReal32CB, &smBlobProperties.bobOverlapFactor },
//    { "smBobSmallestRadius",         scriptSetReal32CB, &smBlobProperties.bobSmallestRadius },
//    { "smBobBiggestRadius",         scriptSetReal32CB, &smBlobProperties.bobBiggestRadius },
//    { "smBobDoingCollisionBobs",    scriptSetBool, &smBlobProperties.bobDoingCollisionBobs },
    { "FOWBobDensityLow",           scriptSetReal32CB, &smFOWBlobProperties.bobDensityLow },
    { "FOWBobDensityHigh",          scriptSetReal32CB, &smFOWBlobProperties.bobDensityHigh },
    { "FOWBobStartSphereSize",      scriptSetReal32CB, &smFOWBlobProperties.bobStartSphereSize },
    { "FOWBobRadiusCombineMargin",  scriptSetReal32CB, &smFOWBlobProperties.bobRadiusCombineMargin },
    { "FOWBobOverlapFactor",        scriptSetBlobPropertyOverlap, &smFOWBlobProperties },
    { "FOWBobSmallestRadius",        scriptSetReal32CB, &smFOWBlobProperties.bobSmallestRadius },
    { "FOWBobBiggestRadius",        scriptSetBlobBiggestRadius, &smFOWBlobProperties },
//    { "FOWBobDoingCollisionBobs",   scriptSetBool, &smFOWBlobProperties.bobDoingCollisionBobs },
    { "FOW_AsteroidK1",             scriptSetReal32CB, &smFOW_AsteroidK1 },
    { "FOW_AsteroidK2",             scriptSetReal32CB, &smFOW_AsteroidK2 },
    { "FOW_AsteroidK3",             scriptSetReal32CB, &smFOW_AsteroidK3 },
    { "FOW_AsteroidK4",             scriptSetReal32CB, &smFOW_AsteroidK4 },
    { "FOW_DustGasK2",              scriptSetReal32CB, &smFOW_DustGasK2 },
    { "FOW_DustGasK3",              scriptSetReal32CB, &smFOW_DustGasK3 },
    { "FOW_DustGasK4",              scriptSetReal32CB, &smFOW_DustGasK4 },
    { "smFOWBlobUpdateTime",        scriptSetReal32CB, &smFOWBlobUpdateTime },
    endEntry
};

/*=============================================================================
    Private Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : smStrchr
    Description : Like strchr but returns NULL when searching for a NULL terminator.
    Inputs      :
    Outputs     :
    Return      : NULL
----------------------------------------------------------------------------*/
void *smStrchr(char *string, char character)
{
    while (*string)
    {
        if (character == *string)
        {
            return(string);
        }
        string++;
    }
    return(NULL);
}

#define SM_NumberTickMarkSpacings       4
#define SM_TickSwitchHysteresis         0.9f
#define SM_SpokeLengthFactor            1.05f
typedef struct
{
    real32 radius;                              //'on' radius of the ticks
    real32 spacing;                             //spacing between ticks, meters
    real32 length;                              //factor of the circle radius
}
ticklod;
real32 smTickSwitchHysteresis = SM_TickSwitchHysteresis;
real32 smSpokeLengthFactor = SM_SpokeLengthFactor;
ticklod smTickMarkInfo[SM_NumberTickMarkSpacings] =
{
    {100000.0f, 20000.0f, 0.025f},
    {50000.0f, 10000.0f, 0.015f},
    {10000.0f, 2000.0f, 0.007f},
};
/*-----------------------------------------------------------------------------
    Name        : smWorldPlaneDraw
    Description : Draw the sensors manager world plane.
    Inputs      : z - height to draw it at so we can use same drawing code for
                    the movement pie plate.
                  c - color to use for drawing the beast
                  bDrawSpokes - true if we are to draw spokes
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static real32 smTickFactorX[4] = {0.0f, 1.0f, 0.0f, -1.0f};
static real32 smTickFactorY[4] = {1.0f, 0.0f, -1.0f, 0.0f};
void smWorldPlaneDraw(vector *centre, bool bDrawSpokes, color c)
{
    real32 radius = smCamera.distance * smWorldPlaneDistanceFactor;
    real32 tickExtent = radius * smTickExtentFactor;
    real32 ringRadius;
    real32 angle, sinTheta, cosTheta;
    sdword spoke, LOD, thisLOD;
    vector spokeRim, tickEnd0, tickEnd1, p0, p1;
    real32 tickFactorX = 0.0f, tickFactorY = 1.0f, tickRadius;

    //draw the circle
    rndTextureEnable(FALSE);
    ringRadius = (smZoomMin + smZoomMax) / 2.0f * smWorldPlaneDistanceFactor;
    primCircleOutlineZ(centre, ringRadius, smWorldPlaneSegments, c);

    //draw the radial ticks
    tickEnd0.z = tickEnd1.z = centre->z;
    for (angle = smRadialTickStart; angle < 2.0f * PI; angle += smRadialTickInc)
    {
        sinTheta = (real32)sin((double)angle);
        cosTheta = (real32)cos((double)angle);
        tickEnd0.x = tickEnd1.x = ringRadius * sinTheta;
        tickEnd0.y = tickEnd1.y = ringRadius * cosTheta;
        tickEnd0.x *= smTickInnerMult;
        tickEnd0.y *= smTickInnerMult;
        tickEnd1.x *= smTickOuterMult;
        tickEnd1.y *= smTickOuterMult;
        vecAdd(p0, *centre, tickEnd0);
        vecAdd(p1, *centre, tickEnd1);
        primLine3(&p0, &p1, c);
    }
    //draw the spokes with gradations, if applicable
    if (bDrawSpokes)
    {
        spokeRim.z = centre->z;

        for (LOD = 0; LOD < SM_NumberTickMarkSpacings; LOD++)
        {
            if (smTickMarkInfo[LOD].radius <= radius)
            {
                break;
            }
        }
        for (spoke = 0; spoke < 4; spoke++)
        {
            tickFactorX = smTickFactorX[spoke];
            tickFactorY = smTickFactorY[spoke];
            spokeRim.y = spokeRim.x = ringRadius * smSpokeLengthFactor;
            spokeRim.x *= tickFactorX;
            spokeRim.y *= tickFactorY;
            vecAdd(p0, *centre, spokeRim);
            primLine3(centre, &p0, c);

            for (thisLOD = 0; thisLOD < LOD; thisLOD++)
            {
                tickRadius = smTickMarkInfo[thisLOD].spacing;
                while (tickRadius <= tickExtent)
                {
                    tickEnd0.x = radius * smTickMarkInfo[thisLOD].length * tickFactorX;
                    tickEnd0.y = radius * smTickMarkInfo[thisLOD].length * tickFactorY;
                    tickEnd1.x = -tickEnd0.x;
                    tickEnd1.y = -tickEnd0.y;
                    tickEnd0.x += tickRadius * tickFactorY;
                    tickEnd0.y += tickRadius * tickFactorX;
                    tickEnd1.x += tickRadius * tickFactorY;
                    tickEnd1.y += tickRadius * tickFactorX;
                    vecAdd(p0, *centre, tickEnd0);
                    vecAdd(p1, *centre, tickEnd1);
                    primLine3(&p0, &p1, c);
                    tickRadius += smTickMarkInfo[thisLOD].spacing;
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsNotBeDisabled
    Description : removes disabled ships from selection
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/

void makeShipsNotBeDisabled(SelectCommand *selection)
{
    sdword i;
    for(i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[0]->flags & SOF_Disabled)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            i--;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : smHorizonLineDraw
    Description : Draw the horizon line with compass tick marks
    Inputs      : cam - context we're rendering from
                  thisBlob - blob we're rendering
                  modelView, projection - current matrices
                  distance - the distance to draw the ring at
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#define cam     ((Camera *)voidCam)
void smHorizonLineDraw(void *voidCam, hmatrix *modelView, hmatrix *projection, real32 distance)
{
    real32 startAngle, endAngle;
    real32 x, y, radius;
    vector startPoint, horizPoint, endPoint;
    real32 angle, degAngle;
    bool isEnabled = FALSE;
    sdword nPrintfed;
    fonthandle oldFont = fontCurrentGet();
//    distance = smCurrentCameraDistance; //!!!
    distance *= smWorldPlaneDistanceFactor;

    startAngle = cam->angle - PI - DEG_TO_RAD(cam->fieldofview) / 2.0f - smHorizTickAngle * 2.0;
    endAngle = cam->angle - PI + DEG_TO_RAD(cam->fieldofview) / 2.0f + smHorizTickAngle * 2.0;

    angle = (real32)floor((double)(startAngle / smHorizTickAngle)) * smHorizTickAngle;
    startPoint.x = cam->lookatpoint.x + (real32)cos((double)angle) * distance;   //position of starting point
    startPoint.y = cam->lookatpoint.y + (real32)sin((double)angle) * distance;
    endPoint.z = startPoint.z = cam->lookatpoint.z;

    fontMakeCurrent(mouseCursorFont);

    if (glIsEnabled(GL_DEPTH_TEST))
    {
        isEnabled = TRUE;
        glDisable(GL_DEPTH_TEST);
    }
    for (smTickTextIndex = 0; angle < endAngle; angle += smHorizTickAngle)
    {
        endPoint.x = cam->lookatpoint.x + (real32)cos((double)(angle + smHorizTickAngle)) * distance;//position of current point
        endPoint.y = cam->lookatpoint.y + (real32)sin((double)(angle + smHorizTickAngle)) * distance;

        primLine3(&startPoint, &endPoint, smCurrentWorldPlaneColor);//draw the vertical tick
        horizPoint.x = endPoint.x;
        horizPoint.y = endPoint.y;
        if (cam->eyeposition.z > cam->lookatpoint.z)
        {
            horizPoint.z = cam->lookatpoint.z + cam->clipPlaneFar * smHorizTickVerticalFactor;
        }
        else
        {
            horizPoint.z = cam->lookatpoint.z - cam->clipPlaneFar * smHorizTickVerticalFactor;
        }

        //figure out where to draw the tick text
        selCircleComputeGeneral(modelView, projection, &horizPoint, 1.0f, &x, &y, &radius);
        dbgAssert(smTickTextIndex < SM_MaxTicksOnScreen);
        smTickText[smTickTextIndex].x = primGLToScreenX(x);
        smTickText[smTickTextIndex].y = primGLToScreenY(y);
        degAngle = -RAD_TO_DEG(angle - smHorizTickAngle);
        if (degAngle < 0.0f)
        {
            degAngle += 360.0f;
        }
        if (degAngle > 360.0f)
        {
            degAngle -= 360.0f;
        }
        if (degAngle < 10)
        {
            nPrintfed = sprintf(smTickText[smTickTextIndex].text, "00%.0f", degAngle);
        }
        else if (degAngle < 100)
        {
            nPrintfed = sprintf(smTickText[smTickTextIndex].text, "0%.0f", degAngle);
        }
        else
        {
            nPrintfed = sprintf(smTickText[smTickTextIndex].text, "%.0f", degAngle);
        }
        dbgAssert(nPrintfed < SM_TickTextChars);
        if (cam->eyeposition.z > cam->lookatpoint.z)
        {
            smTickText[smTickTextIndex].y -= fontHeight(" ") + smTickTextSpacing;
        }
        else
        {
            smTickText[smTickTextIndex].y += smTickTextSpacing;
        }
        smTickText[smTickTextIndex].x -= fontWidth(smTickText[smTickTextIndex].text) / 2;
        smTickTextIndex++;
        //draw the horizontal tick
        primLine3(&endPoint, &horizPoint, smCurrentWorldPlaneColor);
        horizPoint.x = (endPoint.x - cam->lookatpoint.x) * smHorizonTickHorizFactor + cam->lookatpoint.x;
        horizPoint.y = (endPoint.y - cam->lookatpoint.y) * smHorizonTickHorizFactor + cam->lookatpoint.y;
        horizPoint.z = cam->lookatpoint.z;
        primLine3(&endPoint, &horizPoint, smCurrentWorldPlaneColor);
        startPoint = endPoint;                              //draw from this point to next point next time through
    }
    if (isEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    fontMakeCurrent(oldFont);
}
#undef cam

/*-----------------------------------------------------------------------------
    Name        : smTickTextDraw
    Description : Draw the tick text created in smHorizonLineDraw
    Inputs      :
    Outputs     :
    Return      : void
    Note        : must be in 2d mode for this function
----------------------------------------------------------------------------*/
void smTickTextDraw(void)
{
    fonthandle oldFont = fontCurrentGet();

    fontMakeCurrent(mouseCursorFont);

    for (smTickTextIndex--; smTickTextIndex >= 0; smTickTextIndex--)
    {
        fontPrint(smTickText[smTickTextIndex].x, smTickText[smTickTextIndex].y,
                  smCurrentWorldPlaneColor, smTickText[smTickTextIndex].text);
    }
    fontMakeCurrent(oldFont);
}

/*-----------------------------------------------------------------------------
    Name        : smBlobDrawClear
    Description : Render all the ships inside a blob.  This would be for blobs
                    with player ships or when player has full sensors researched.
    Inputs      : camera - POV to render from
                  thisBlob - blob we're rendering
                  modelView, projection - current matrices
                  background - color of the blob being drawn to
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
//render utility functions
//bool8 rndShipVisible(SpaceObj* spaceobj, Camera* camera);
//string of ship types to be rendered as meshes
void smBlobDrawClear(Camera *camera, blob *thisBlob, hmatrix *modelView, hmatrix *projection, color background)
{
    sdword index, i;
    SpaceObj **objPtr, *obj;
    color c;
    lod *level;
    hmatrix coordMatrixForGL;
    ShipStaticInfo *shipStaticInfo;
    bool bFlashOn;
    SpaceObjSelection *blobObjects = thisBlob->blobObjects;
    real32 radius;
    hvector objectPos, cameraSpace, screenSpace;
    smblurry *blurry;
    sdword nShipTOs = 0;
    struct
    {
        real32 x, y;
        real32 radius;
        color c;
        ShipClass shipClass;
    }
    shipTO[SM_NumberTOs];
    sdword nBigDots = 0;
    struct
    {
        real32 x, y;
        color c;
    }
    bigDot[SM_NumberBigDots];
    real32 pointSize;
    rectangle rect;

    //flash the selected ships
    if (fmod((double)universe.totaltimeelapsed, (double)smSelectedFlashSpeed) > smSelectedFlashSpeed / 2.0)
    {
        bFlashOn = TRUE;
    }
    else
    {
        bFlashOn = FALSE;
    }
    //draw all objects in the sphere
    for (index = 0, objPtr = blobObjects->SpaceObjPtr; index < blobObjects->numSpaceObjs; index++, objPtr++)
    {
        obj = *objPtr;
//#ifdef DEBUG_COLLBLOBS
        if (obj->flags & SOF_Targetable)
//#endif
        {
            selCircleCompute(modelView, projection, (SpaceObjRotImpTarg *)obj);//compute selection circle
        }
        switch (obj->objtype)
        {
            case OBJ_ShipType:
                if (smNonResources == FALSE)
                {
                    break;
                }
                if (bitTest(((Ship *)obj)->flags, SOF_Hide))
                {                                           //don't show hidden ships
                    break;
                }
                if (smTacticalOverlay)
                {
                    if (((Ship *)obj)->playerowner == universe.curPlayerPtr &&
                        bitTest(smShipTypeRenderFlags[((Ship *)obj)->shiptype], SM_TO))
                    {
                        if (nShipTOs < SM_NumberTOs)
                        {
                            shipTO[nShipTOs].x = ((Ship *)obj)->collInfo.selCircleX;
                            shipTO[nShipTOs].y = ((Ship *)obj)->collInfo.selCircleY;
                            shipTO[nShipTOs].radius = ((Ship *)obj)->collInfo.selCircleRadius;
                            shipTO[nShipTOs].c = colRGB(colRed(c)/TO_IconColorFade, colGreen(c)/TO_IconColorFade, colBlue(c)/TO_IconColorFade);
                            shipTO[nShipTOs].shipClass = ((Ship *)obj)->staticinfo->shipclass;
                            nShipTOs++;
                        }
                    }
                    if(((Ship *)obj)->playerowner == universe.curPlayerPtr)
                    {
                        if(((Ship *)obj)->shiptype == GravWellGenerator)
                        {
                            if (((GravWellGeneratorSpec *)((Ship *)obj)->ShipSpecifics)->GravFieldOn)
                            {
                                toFieldSphereDraw(((Ship *)obj),((GravWellGeneratorStatics *) ((ShipStaticInfo *)(((Ship *)obj)->staticinfo))->custstatinfo)->GravWellRadius, 1.0f);
                            }
                        }
                    }
                }
                if (bitTest(smShipTypeRenderFlags[((Ship *)obj)->shiptype], SM_Mesh))
                {                                           //if it's a ship type to render as a mesh
                    level = lodLevelGet((void *)obj, &camera->eyeposition, &obj->posinfo.position);
                    if ((level->flags & LM_LODType) == LT_Mesh)
                    {
                        if (!(bFlashOn && (((Ship *)obj)->flags & SOF_Selected)))
                        {
                            if(!bitTest(obj->flags,SOF_Cloaked) || ((Ship *)obj)->playerowner == universe.curPlayerPtr)
                            {       //if ship isn't cloaked, draw, or if ship is players, draw
                                glPushMatrix();
                                shipStaticInfo = (ShipStaticInfo *)obj->staticinfo;
                                hmatMakeHMatFromMat(&coordMatrixForGL,&((SpaceObjRot *)obj)->rotinfo.coordsys);
                                hmatPutVectIntoHMatrixCol4(obj->posinfo.position,coordMatrixForGL);
                                glMultMatrixf((float *)&coordMatrixForGL);//ship's rotation matrix
                                rndLightingEnable(TRUE);
                                shPushLightMatrix(&coordMatrixForGL);
                                i = ((Ship *)obj)->colorScheme;

                                if (rndShipVisible(obj, camera))
                                {
                                    bool wireOn;
                                    extern bool8 g_WireframeHack;

                                    wireOn = g_WireframeHack;
                                    g_WireframeHack = FALSE;
                                    if (((Ship *)obj)->bindings != NULL)
                                    {
                                        meshRenderShipHierarchy(((Ship *)obj)->bindings,
                                                ((Ship *)obj)->currentLOD,
                                                (meshdata *)level->pData, i);
                                    }
                                    else
                                    {
                                        meshRender((meshdata *)level->pData, i);
                                    }
                                    g_WireframeHack = (ubyte)wireOn;
                                }
                                shPopLightMatrix();
                                glDisable(GL_BLEND);
                                glDisable(GL_ALPHA_TEST);
                                rndLightingEnable(FALSE);
                                rndTextureEnable(FALSE);
                                glPopMatrix();
                            }
                        }
                    }
                    else
                    {                                       //else no LOD at this level
                        goto justRenderAsDot;
                    }
                }                                           //else draw it as a pixel
                else if ( (!bitTest(obj->flags,SOF_Cloaked) || allianceIsShipAlly((Ship *)obj, universe.curPlayerPtr)) &&
                          ((obj->attributes & ATTRIBUTES_SMColorField) != ATTRIBUTES_SMColorInvisible) )
                {       //if ship isn't cloaked, draw, or if ship is players, draw
justRenderAsDot:
#if TO_STANDARD_COLORS
                    if (((Ship *)obj)->playerowner == universe.curPlayerPtr)
                    {
                        c = teFriendlyColor;
                    }
                    else if (allianceIsShipAlly((Ship *)obj, universe.curPlayerPtr))
                    {
                        c = teAlliedColor;
                    }
                    else
                    {
                        c = teHostileColor;
                    }
                    if (obj->attributes & ATTRIBUTES_SMColorField)  // overide with special colors
                    {
                        if ((obj->attributes & ATTRIBUTES_SMColorField) == ATTRIBUTES_SMColorYellow)
                        {
                            c = teAlliedColor;
                        }
                        else if ((obj->attributes & ATTRIBUTES_SMColorField) == ATTRIBUTES_SMColorGreen)
                        {
                            c = teFriendlyColor;
                        }
                    }
#else //SM_STANDARD_COLORS
                    c = teColorSchemes[((Ship *)obj)->colorScheme].tacticalColor;
#endif //SM_STANDARD_COLORS
                    if (smTacticalOverlay)
                    {
                        if (((Ship *)obj)->playerowner == universe.curPlayerPtr &&
                            bitTest(smShipTypeRenderFlags[((Ship *)obj)->shiptype], SM_TO))
                        //if (((Ship *)obj)->shiptype == ResourceCollector)
                        {
                            if (nShipTOs < SM_NumberTOs)
                            {
                                shipTO[nShipTOs].x = ((Ship *)obj)->collInfo.selCircleX;
                                shipTO[nShipTOs].y = ((Ship *)obj)->collInfo.selCircleY;
                                shipTO[nShipTOs].radius = ((Ship *)obj)->collInfo.selCircleRadius;
                                shipTO[nShipTOs].c = colRGB(colRed(c)/TO_IconColorFade, colGreen(c)/TO_IconColorFade, colBlue(c)/TO_IconColorFade);
                                shipTO[nShipTOs].shipClass = ((Ship *)obj)->staticinfo->shipclass;
                                nShipTOs++;
                            }
                        }
                    }
                    if (((Ship *)obj)->flags & SOF_Selected)
                    {
                        if (bFlashOn)
                        {
                            c = background;
                        }
                    }

//                    if ((((Ship *)obj)->playerowner->playerIndex == universe.curPlayerIndex))
//                    {                                       //enable double-size points
                        pointSize = 2.0f;
//                        //glPointSize(2.0f);
//                    }
//                    else
//                    {
//                        pointSize = 1.0f;
//                        //glPointSize(1.0f);
//                    }
                    rndTextureEnable(FALSE);
                    if ((!smBigPoints) && pointSize != 1.0f && ((Ship *)obj)->collInfo.selCircleRadius > 0.0f)
                    {
                        dbgAssert(nBigDots < SM_NumberBigDots);
                        bigDot[nBigDots].x = ((Ship *)obj)->collInfo.selCircleX;
                        bigDot[nBigDots].y = ((Ship *)obj)->collInfo.selCircleY;
                        bigDot[nBigDots].c = c;
                        nBigDots++;
                    }
                    else
                    {
                        glPointSize(pointSize);
                        primPoint3(&obj->posinfo.position, c);  //everything is rendered as a point
                        glPointSize(1.0f);
                    }
                }
                break;
            case OBJ_AsteroidType:
                if (smResources == FALSE)
                {
                    break;
                }
                radius = obj->staticinfo->staticheader.staticCollInfo.collspheresize;

                if (radius > SM_LargeResourceSize)
                {
                    pointSize = 2.0f;
                    //glPointSize(2.0f);
                }
                rndTextureEnable(FALSE);
#if TO_STANDARD_COLORS
                c = teResourceColor;
#else
                c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
                if ((!smBigPoints) && pointSize != 1.0f && ((Ship *)obj)->collInfo.selCircleRadius > 0.0f)
                {
                    dbgAssert(nBigDots < SM_NumberBigDots);
                    bigDot[nBigDots].x = ((Ship *)obj)->collInfo.selCircleX;
                    bigDot[nBigDots].y = ((Ship *)obj)->collInfo.selCircleY;
                    bigDot[nBigDots].c = c;
                    nBigDots++;
                }
                else
                {
                    glPointSize(pointSize);
                    primPoint3(&obj->posinfo.position, c);     //everything is rendered as a point
                    glPointSize(1.0f);
                }
                pointSize = 1.0f;
                break;
            case OBJ_NebulaType:
            case OBJ_GasType:
            case OBJ_DustType:
                if (smResources == FALSE)
                {
                    break;
                }
                memcpy(&objectPos, &obj->posinfo.position, sizeof(vector));
                objectPos.w = 1.0f;
//                c = obj->staticinfo->staticheader.LOD->pointColor;
//                c = colWhite;
                hmatMultiplyHMatByHVec(&cameraSpace, modelView, &objectPos);//in camera space
                hmatMultiplyHMatByHVec(&screenSpace, projection, &cameraSpace);//in screen space
//                primBlurryPoint22(primGLToScreenX(screenSpace.x / screenSpace.w),    //everything is rendered as a blurry point
//                                 primGLToScreenY(screenSpace.y / screenSpace.w), c);
                if (screenSpace.z > 0.0f && smBlurryIndex < SM_BlurryArraySize)
                {
#if TO_STANDARD_COLORS
                    smBlurryArray[smBlurryIndex].c = teResourceColor;
#else
                    smBlurryArray[smBlurryIndex].c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
                    smBlurryArray[smBlurryIndex].x = primGLToScreenX(screenSpace.x / screenSpace.w);
                    smBlurryArray[smBlurryIndex].y = primGLToScreenY(screenSpace.y / screenSpace.w);
                    smBlurryIndex++;
                }
                break;
            case OBJ_DerelictType:
                if (((Derelict *)obj)->derelicttype == HyperspaceGate)
                {
                    goto renderDerelictAsDot;
                }
                level = lodLevelGet((void *)obj, &camera->eyeposition, &obj->posinfo.position);
                if (bitTest(smDerelictTypeMesh[((Derelict *)obj)->derelicttype], SM_Mesh) && (level->flags & LM_LODType) == LT_Mesh)
                {
                    if ( (!bitTest(obj->flags,SOF_Cloaked)) &&
                         ((obj->attributes & ATTRIBUTES_SMColorField) != ATTRIBUTES_SMColorInvisible) )
                    {       //if it isn't cloaked, draw
                        glPushMatrix();
                        shipStaticInfo = (ShipStaticInfo *)obj->staticinfo;
                        hmatMakeHMatFromMat(&coordMatrixForGL,&((SpaceObjRot *)obj)->rotinfo.coordsys);
                        hmatPutVectIntoHMatrixCol4(obj->posinfo.position,coordMatrixForGL);
                        glMultMatrixf((float *)&coordMatrixForGL);//ship's rotation matrix
                        rndLightingEnable(TRUE);
                        shPushLightMatrix(&coordMatrixForGL);
                        i = ((Derelict *)obj)->colorScheme;

                        if (rndShipVisible(obj, camera))
                        {
                            bool wireOn;
                            extern bool8 g_WireframeHack;

                            wireOn = g_WireframeHack;
                            g_WireframeHack = FALSE;
                            meshRender((meshdata *)level->pData, i);
                            g_WireframeHack = (ubyte)wireOn;
                        }
                        shPopLightMatrix();
                        glDisable(GL_BLEND);
                        glDisable(GL_ALPHA_TEST);
                        rndLightingEnable(FALSE);
                        rndTextureEnable(FALSE);
                        glPopMatrix();
                    }
                }
                else
                {
renderDerelictAsDot:
#if TO_STANDARD_COLORS
                        c = teNeutralColor;
#else
                        c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
                    if (singlePlayerGame && (((Derelict *)obj)->derelicttype == PrisonShipOld))
                    {
                        c = teFriendlyColor;
                    }

                    rndTextureEnable(FALSE);
                    primPoint3(&obj->posinfo.position,c);   //everything is rendered as a point
                }
                break;

            default:
                break;
        }
    }

    //display the nebulae tendrils as strings
    rndTextureEnable(FALSE);
    rndLightingEnable(FALSE);

    if (glCapFastFeature(GL_BLEND))
    {
        glEnable(GL_BLEND);
        rndAdditiveBlends(FALSE);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    glShadeModel(GL_FLAT);
    for (index = 0; index < numNebulae; index++)
    {
        sdword t;
        nebulae_t* neb = &nebNebulae[index];

        glBegin(GL_LINES);
        for (t = 0; t < neb->numTendrils; t++)
        {
            color c = neb->tendrilTable[t].colour;
            glColor4ub(colRed(c), colGreen(c), colBlue(c), 192);
            glVertex3fv((GLfloat*)&neb->tendrilTable[t].a->position);
            glVertex3fv((GLfloat*)&neb->tendrilTable[t].b->position);
        }
        glEnd();
    }
    glDisable(GL_BLEND);

    if (smBlurryIndex > 0 || nShipTOs > 0 || nBigDots > 0)
    {
        toicon *icon;
        color col;
        real32 radius;

        primModeSet2();
        for (blurry = smBlurryArray; smBlurryIndex > 0; smBlurryIndex--, blurry++)
        {
            primBlurryPoint22(blurry->x, blurry->y, blurry->c);
        }

        for (index = 0; index < nShipTOs; index++)
        {
            icon = toClassIcon[shipTO[index].shipClass];
            toClassUsed[shipTO[index].shipClass][0] = TRUE;

            if (shipTO[index].radius > 0.0f)
            {
                radius = max(shipTO[index].radius, smTORadius);
                col = shipTO[index].c;
                primLineLoopStart2(1, col);

                for (i = icon->nPoints - 1; i >= 0; i--)
                {
                        primLineLoopPoint3F(shipTO[index].x + icon->loc[i].x * radius,
                                       shipTO[index].y + icon->loc[i].y * radius);
                }
            }
            primLineLoopEnd2();

        }
        for (index = 0; index < nBigDots; index++)
        {
            rect.x0 = primGLToScreenX(bigDot[index].x);
            rect.y0 = primGLToScreenY(bigDot[index].y);
            rect.x1 = rect.x0 + 2;
            rect.y1 = rect.y0 + 2;
            primRectSolid2(&rect, bigDot[index].c);
        }
        primModeClear2();
        rndLightingEnable(FALSE);
        rndTextureEnable(FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : SensorsWeirdnessUtilities
    Description : Utility functions for the sensors weirdness in the single player game
    Inputs      : various
    Outputs     : various
    Return      : various
----------------------------------------------------------------------------*/
//generates a random location within (and slightly outside) the bounds of the universe
vector smwGenerateRandomPoint(void)
{
    vector vec;

    vec.x = (real32)frandyrandombetween(RAN_AIPlayer,-smUniverseSizeX*WEIRDUNIVERSESTRETCH,smUniverseSizeX*WEIRDUNIVERSESTRETCH);
    vec.y = (real32)frandyrandombetween(RAN_AIPlayer,-smUniverseSizeY*WEIRDUNIVERSESTRETCH,smUniverseSizeY*WEIRDUNIVERSESTRETCH);
    vec.z = (real32)frandyrandombetween(RAN_AIPlayer,-smUniverseSizeZ*WEIRDUNIVERSESTRETCH,smUniverseSizeZ*WEIRDUNIVERSESTRETCH);

    return vec;
}

//generates a random color for the point
color smwGenerateRandomColor(void)
{
    udword colornum;
    color  col;

    colornum = (udword)randyrandom(RAN_AIPlayer, 7);

    switch (colornum)
    {
        case 0:
//            col = teFriendlyColor;
//            break;
        case 1:
        case 2:
        case 3:
            col = teHostileColor;
            break;
        case 4:
        case 5:
        case 6:
        default:
            col = teResourceColor;
            break;
    }
    return col;
}

void smwGeneratePing(vector location)
{
    sdword pingcnt;
    char name[20] = "0";

    //very small chance of a new ping
    if (((udword)randyrandom(RAN_AIPlayer, 200)) <= 1)
    {
        pingcnt = randyrandom(RAN_AIPlayer, 3);

        sprintf(name, "Weirdness%i", pingcnt);
        pingAnomalyPingRemove(name);
        pingAnomalyPositionPingAdd(name, &location);
    }
}


/*-----------------------------------------------------------------------------
    Name        : smSensorWeirdnessDraw
    Description : Draws the sensor manager malfunction in Missions 7 and 8
    Inputs      :
    Outputs     : Draws a bunch of weirdness on the screen
    Return      : void
----------------------------------------------------------------------------*/
void smSensorWeirdnessDraw(hmatrix *modelView, hmatrix *projection)
{
    udword i,j, num_replaces;
    static udword num_points;
    static udword redraw = 0;
    vector tempvec;
    rectangle rect;

    if (!redraw)
    {
        num_points   = (udword)randyrandom(RAN_AIPlayer, (smSensorWeirdness/3));
        num_points  += (2*smSensorWeirdness/3);
        redraw       = randyrandombetween(RAN_AIPlayer, 20, 30);
    }
    else
    {
        redraw--;
    }

    num_replaces = (udword)randyrandom(RAN_AIPlayer, MAX_REPLACEMENTS);

    for (i=0;i<num_replaces;i++)
    {
        //chose a random point
        j = (udword)randyrandom(RAN_AIPlayer, smSensorWeirdness);

        //generate a random location for it
        smWeird[j].location = smwGenerateRandomPoint();
        smWeird[j].color    = smwGenerateRandomColor();
        smWeird[j].fade     = 0.0f;
        smwGeneratePing(smWeird[j].location);
    }


    primModeSet2();
    //    rndGLStateLog("RocksAreBig");
    for (i=0;i<num_points;i++)
    {
        transSingleTotalTransform(&tempvec, modelView, projection, &smWeird[i].location);
        rect.x0 = primGLToScreenX(tempvec.x);
        rect.y0 = primGLToScreenY(tempvec.y);
        rect.x1 = rect.x0 + 2;
        rect.y1 = rect.y0 + 2;
        primRectSolid2(&rect, smWeird[i].color);
    }
    primModeClear2();
}

/*-----------------------------------------------------------------------------
    Name        : smSensorsWeirdnessInit
    Description : Initializes the sensors weirdness stuff
    Inputs      :
    Outputs     : Fills the smWeirdCol and smWeirdLoc arrays
    Return      : void
----------------------------------------------------------------------------*/
void smSensorWeirdnessInit(void)
{
    udword i;

    for (i=0;i<NUM_WEIRD_POINTS;i++)
    {
        smWeird[i].location    = smwGenerateRandomPoint();
        smWeird[i].color       = smwGenerateRandomColor();
        smWeird[i].ping        = FALSE;
    }
}



/*-----------------------------------------------------------------------------
    Name        : smEnemyFogOfWarBlobCallback
    Description : Sub-blob creation callback for creating fog-of-war sub-blobs
    Inputs      : superBlob - the blob we're looking in
                  obj - object to evaluate for inclusion
    Outputs     :
    Return      : TRUE if it's an enemy ship or a foggy resource
----------------------------------------------------------------------------*/
bool smEnemyFogOfWarBlobCallback(blob *superBlob, SpaceObj *obj)
{
    switch (obj->objtype)
    {
        case OBJ_ShipType:                                  //make sure it's an enemy ship
            if (allianceIsShipAlly((Ship *)obj,universe.curPlayerPtr))
            {                                               //if this is an allied ship (alliance formed while in SM)
                return(FALSE);
            }
            if (((Ship *)obj)->flags & (SOF_Hide | SOF_Cloaked))
            {                                               //don't show hidden or cloaked ships
                return(FALSE);
            }
            if ((obj->attributes & ATTRIBUTES_SMColorField) == ATTRIBUTES_SMColorInvisible)
            {
                return(FALSE);
            }
            return(TRUE);
        case OBJ_BulletType:
            break;
        case OBJ_AsteroidType:
            return(TRUE);
        case OBJ_NebulaType:
            break;
        case OBJ_GasType:
        case OBJ_DustType:
            return(TRUE);
        case OBJ_DerelictType:
        case OBJ_EffectType:
        case OBJ_MissileType:
            break;
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : smBlobDrawCloudy
    Description : Draw a cloudy sensors blob, such as one in the shroud.
    Inputs      : same as for the clear draw
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#define SM_VISUALIZE_SUBBLOBS   0
void smBlobDrawCloudy(Camera *camera, blob *thisBlob, hmatrix *modelView, hmatrix *projection, color background)
{
    sdword index;
    SpaceObj **objPtr, *obj;
    color c;
    hvector objectPos, cameraSpace, screenSpace;
    SpaceObjSelection *blobObjects = thisBlob->blobObjects;
    Node *subBlobNode;
    blob *subBlob;
    sdword nShips;
    smblurry *blurry;
    sdword nBigDots = 0;
    struct
    {
        real32 x, y;
        color c;
    }
    bigDot[SM_NumberBigDots];
    rectangle rect;
    real32 radius;
    real32 pointSize;
    real32 screenX, screenY;

    glPointSize(2.0f);
    //compute a list of sub-blobs for the enemies.  It will be deleted with the parent blob.
    if (thisBlob->subBlobs.num == BIT31)
    {                                                       //if list not yet created
        bobSubBlobListCreate(&smFOWBlobProperties, &thisBlob->subBlobs, thisBlob, smEnemyFogOfWarBlobCallback);
        thisBlob->subBlobTime = universe.totaltimeelapsed;
    }
    else
    {
        if (universe.totaltimeelapsed - thisBlob->subBlobTime >= smFOWBlobUpdateTime)
        {
            bobListDelete(&thisBlob->subBlobs);
            bobSubBlobListCreate(&smFOWBlobProperties, &thisBlob->subBlobs, thisBlob, smEnemyFogOfWarBlobCallback);
            thisBlob->subBlobTime = universe.totaltimeelapsed;
        }
    }

    //go through all sub-blobs and remove any that are hidden due to too many resources
    thisBlob->nHiddenShips = 0;
    for (subBlobNode = thisBlob->subBlobs.head; subBlobNode != NULL; subBlobNode = subBlobNode->next)
    {
        subBlob = (blob *)listGetStructOfNode(subBlobNode);
        if (subBlob->shipMass == 0.0f)
        {                                                   //if no ships in blob
            continue;                                       //skip it
        }
        // if ((k1 * #rocks + k2 * #rockRUs) / (k3 * #ships + k4 * shipMass) > 1.0)
        nShips = subBlob->nCapitalShips + subBlob->nAttackShips + subBlob->nNonCombat;
        if ((smFOW_AsteroidK1 * (real32)subBlob->nRocks + smFOW_AsteroidK2 * subBlob->nRockRUs) /
            (smFOW_AsteroidK3 * (nShips) + smFOW_AsteroidK4 * subBlob->shipMass) > 1.0f)
        {                                                   //if there are too many asteroids to see these ships
#if SM_VISUALIZE_SUBBLOBS
           primCircleOutlineZ(&subBlob->centre, subBlob->radius, 16, colRGB(0, 255, 0));
#endif
            thisBlob->nHiddenShips += nShips;
            continue;                                       //don't draw them
        }
        // if (k2 * #dustRUs / (k3 * #ships + k4 * shipMass) > 1.0)
        if ((smFOW_DustGasK2 * subBlob->nDustRUs) /
            (smFOW_DustGasK3 * nShips + smFOW_DustGasK4 * subBlob->shipMass) > 1.0f)
        {                                                   //if there's too much dust
#if SM_VISUALIZE_SUBBLOBS
//           selCircleComputeGeneral(modelView, projection, &subBlob->centre, subBlob->radius, &subBlob->screenX, &subBlob->screenY, &subBlob->screenRadius);
           primCircleOutlineZ(&subBlob->centre, subBlob->radius, 16, colRGB(255, 255, 0));
#endif
            thisBlob->nHiddenShips += nShips;
            continue;                                       //don't draw the ships
        }
        //there's not too many resources, draw them as normal
#if SM_VISUALIZE_SUBBLOBS
        primCircleOutlineZ(&subBlob->centre, subBlob->radius, 16, colWhite);
#endif
        c = teHostileColor;
        dbgAssert(nBigDots < SM_NumberBigDots);
//        bigDot[nBigDots].x = ((Ship *)obj)->collInfo->selCircleX;
//        bigDot[nBigDots].y = ((Ship *)obj)->collInfo->selCircleY;
        selCircleComputeGeneral(modelView, projection, &subBlob->centre, 1.0f, &screenX, &screenY, &radius);
        for (index = subBlob->blobObjects->numSpaceObjs - 1; index >= 0 ; index--)
        {                                                   //all ships in sub-blob get selection circle of sub-blob
            obj = subBlob->blobObjects->SpaceObjPtr[index];
            if (obj->objtype == OBJ_ShipType)
            {
                ((Ship *)obj)->collInfo.selCircleX = screenX;
                ((Ship *)obj)->collInfo.selCircleY = screenY;
                ((Ship *)obj)->collInfo.selCircleRadius = radius;
            }
        }
        if ((!smBigPoints) && radius > 0.0f)
        {
            bigDot[nBigDots].c = c;
            bigDot[nBigDots].x = screenX;
            bigDot[nBigDots].y = screenY;
            nBigDots++;
        }
        else
        {
            primPoint3(&subBlob->centre, c);
        }
    }
    glPointSize(1.0f);
    //draw all objects in the sphere
    for (index = 0, objPtr = blobObjects->SpaceObjPtr; index < blobObjects->numSpaceObjs; index++, objPtr++)
    {
        obj = *objPtr;
//#ifdef DEBUG_COLLBLOBS
        if (obj->flags & SOF_Targetable)
//#endif
        {
            selCircleCompute(modelView, projection, (SpaceObjRotImpTarg *)obj);//compute selection circle
        }
        switch (obj->objtype)
        {
            case OBJ_ShipType:
                if (((obj->flags & (SOF_Cloaked | SOF_Hide))) ||  //if hidden or cloaked
                    ((obj->attributes & ATTRIBUTES_SMColorField) == ATTRIBUTES_SMColorInvisible))
                {
                    thisBlob->nHiddenShips++;
                }
                continue;                                   //ships were drawn from sub-blobs
            case OBJ_NebulaType:
                continue;                                   //don't even draw nebulae
            case OBJ_GasType:
            case OBJ_DustType:
                if (smResources == FALSE)
                {
                    goto dontRenderThisResource;
                }
                memcpy(&objectPos, &obj->posinfo.position, sizeof(vector));
                objectPos.w = 1.0f;
                hmatMultiplyHMatByHVec(&cameraSpace, modelView, &objectPos);//in camera space
                hmatMultiplyHMatByHVec(&screenSpace, projection, &cameraSpace);//in screen space
                if (screenSpace.z > 0.0f && smBlurryIndex < SM_BlurryArraySize)
                {
#if TO_STANDARD_COLORS
                    smBlurryArray[smBlurryIndex].c = teResourceColor;
#else
                    smBlurryArray[smBlurryIndex].c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
                    smBlurryArray[smBlurryIndex].x = primGLToScreenX(screenSpace.x / screenSpace.w);
                    smBlurryArray[smBlurryIndex].y = primGLToScreenY(screenSpace.y / screenSpace.w);
                    smBlurryIndex++;
                }
                break;
            case OBJ_AsteroidType:
#if TO_STANDARD_COLORS
                c = teResourceColor;
#else
                c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
                radius = obj->staticinfo->staticheader.staticCollInfo.collspheresize;

                if (radius > SM_LargeResourceSize)
                {
                    pointSize = 2.0f;
                }
                else
                {
                    pointSize = 1.0f;
                }
                if ((!smBigPoints) && pointSize != 1.0f && ((Ship *)obj)->collInfo.selCircleRadius > 0.0f)
                {
                    dbgAssert(nBigDots < SM_NumberBigDots);
                    bigDot[nBigDots].x = ((Ship *)obj)->collInfo.selCircleX;
                    bigDot[nBigDots].y = ((Ship *)obj)->collInfo.selCircleY;
                    bigDot[nBigDots].c = c;
                    nBigDots++;
                }
                else
                {
                    glPointSize(pointSize);
                    primPoint3(&obj->posinfo.position, c);
                    glPointSize(1.0f);
                }
                break;
            case OBJ_DerelictType:
                if (((Derelict *)obj)->derelicttype == Crate)
                {                                           //crates are draw as grey dots
                    c = teCrateColor;
                }
                else if ((((Derelict *)obj)->derelicttype == PrisonShipOld) &&
                         singlePlayerGameInfo.currentMission == 8)
                {   //!!!  Single Player game mission specific Code
                    //In Mission 8, the PrisonShipOld derelict shows up
                    //as a friendly ship
                    dbgAssert(nBigDots < SM_NumberBigDots);
                    bigDot[nBigDots].x = ((Derelict *)obj)->collInfo.selCircleX;
                    bigDot[nBigDots].y = ((Derelict *)obj)->collInfo.selCircleY;
                    bigDot[nBigDots].c = teFriendlyColor;
                    nBigDots++;
                }
                else
                {
#if TO_STANDARD_COLORS
                    c = teNeutralColor;
#else
                    c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
                }
                primPoint3(&obj->posinfo.position, c);
                break;
            default:
//#ifndef DEBUG_COLLBLOBS
//                dbgAssert(FALSE);       // don't assert if DEBUG_COLLBLOBS because debug collblobs can have bullets,etc.
//#endif
                break;
        }

dontRenderThisResource:;
    }

    if (smBlurryIndex > 0 || nBigDots > 0)
    {
        primModeSet2();
        for (blurry = smBlurryArray; smBlurryIndex > 0; smBlurryIndex--, blurry++)
        {
            primBlurryPoint22(blurry->x, blurry->y, blurry->c);
        }
        for (index = 0; index < nBigDots; index++)
        {
            rect.x0 = primGLToScreenX(bigDot[index].x);
            rect.y0 = primGLToScreenY(bigDot[index].y);
            rect.x1 = rect.x0 + 2;
            rect.y1 = rect.y0 + 2;
            primRectSolid2(&rect, bigDot[index].c);
        }
        primModeClear2();
    }
    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : smBlobListSort
    Description : qsort callback for sorting a list of blob pointers by sortDistance
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int smBlobListSort(const void *p0, const void *p1)
{
    blob *b0 = *((blob **)p0);
    blob *b1 = *((blob **)p1);

    if (b0->cameraSortDistance < b1->cameraSortDistance)
    {
        return(-1);
    }
    else
    {
        return(1);
    }
}

/*-----------------------------------------------------------------------------
    Name        : smBlobsSortToCamera
    Description : list - linked list of blobs
                  camera - camera to sort to
    Inputs      :
    Outputs     : (Re)allocates and fills in smBlobSortList with pointers to
                    the blobs in list, sorted by distance from camera->eyeposition
    Return      :
----------------------------------------------------------------------------*/
void smBlobsSortToCamera(LinkedList *list, Camera *camera)
{
    blob *thisBlob;
    Node *node;
    vector distance;

    //grow the blob sorting list if needed
    if (list->num > smBlobSortListLength)
    {
        smBlobSortListLength = list->num;
        smBlobSortList = memRealloc(smBlobSortList, sizeof(blob **) * smBlobSortListLength, "smBlobSortList", NonVolatile);
    }
    node = list->head;
    //do a pass through the blobs to figure their sorting distances and count the blobs
    for (node = list->head, smNumberBlobsSorted = 0; node != NULL; node = node->next, smNumberBlobsSorted++)
    {
        thisBlob = (blob *)listGetStructOfNode(node);

        vecSub(distance, camera->eyeposition, thisBlob->centre);
        thisBlob->cameraSortDistance = vecMagnitudeSquared(distance);//figure out a distance for sorting
        smBlobSortList[smNumberBlobsSorted] = thisBlob;     //store reference to blob
    }
    dbgAssert(smNumberBlobsSorted == list->num);
    qsort(smBlobSortList, smNumberBlobsSorted, sizeof(blob **), smBlobListSort);
}

/*-----------------------------------------------------------------------------
    Name        : smBlobDropDistance
    Description : Compute the distance from the movement cursor to a drop-down
                    line for a blob when the move mechanism is up.
    Inputs      : thisBlob - blob to use for the drop-down line
    Outputs     :
    Return      : distance from center of blob's plane point to the centre
                    point of the movement plane point.
----------------------------------------------------------------------------*/
real32 smBlobDropLineDistance(blob *thisBlob)
{
    vector planePoint = thisBlob->centre;

    planePoint.z = selCentrePoint.z;
    return(ABS(piePlanePoint.x - planePoint.x) +            //manhattan distance
            ABS(piePlanePoint.y - planePoint.y));
}

/*-----------------------------------------------------------------------------
    Name        : smBlobsDraw
    Description : Draws all blobs for the current sensors screen
    Inputs      : list - linked list of objects
    Outputs     :
    Return      : Pointer to blob most directly under the mouse cursor or NULL if none.
----------------------------------------------------------------------------*/
blob *smBlobsDraw(Camera *camera, LinkedList *list, hmatrix *modelView, hmatrix *projection, sdword sensorLevel)
{
    oval o;
    blob *thisBlob;
//    Node *node;
    blob *closestMoveBlob = NULL;
    real32 closestMoveDistance = REALlyBig, moveDistance;
    sdword index, blobIndex, nSegments, distance, closestDistance = SDWORD_Max;
    real32 distanceToOrigin, distanceToOriginSquared, factor;
    real32 closestSortDistance = REALlyBig, farthestSortDistance = REALlyNegative, highestBlob = 0.0f;
    color c;
    sdword radius;
    vector p0, p1;
    real32 x, y, screenRadius;
    bool reActivateStipple;
#if BOB_VERBOSE_LEVEL >= 2
    sdword nBlobs = 0;
#endif
    bool bClosestMove = FALSE;      // used for playing the tick sound when a blob is highligted
    static bool bPlayedSound;       // ditto
    sdword carrierHalfWidth, mothershipHalfWidth;
    char *carrier, *mothership;
    fonthandle oldFont;

    mouseCursorObjPtr  = NULL;               //Falko: got an obscure crash where mouseCursorObjPtr was mangled, will this work?

    smBigPoints = glCapFeatureExists(GL_POINT_SIZE);

    memset(toClassUsed, 0, sizeof(toClassUsed));
    closestBlob = NULL;

    distanceToOriginSquared = vecMagnitudeSquared(camera->eyeposition);
    distanceToOrigin = fmathSqrt(distanceToOriginSquared);

    smBlobsSortToCamera(list, camera);
    primModeSet2();

    //go through the list to find the farthest and closest blobs, plus the blob closest
    //to movement cursor
//    node = list->head;
//    while (node != NULL)
//        thisBlob = (blob *)listGetStructOfNode(node);
//        node = node->next;
    for (blobIndex = 0; blobIndex < smNumberBlobsSorted; blobIndex++)
    {
        thisBlob = smBlobSortList[blobIndex];
        closestSortDistance = min(closestSortDistance, thisBlob->cameraSortDistance);
        farthestSortDistance = max(farthestSortDistance, thisBlob->cameraSortDistance);
        highestBlob = max(highestBlob, ABS(thisBlob->centre.z));
        moveDistance = smBlobDropLineDistance(thisBlob);
        if (moveDistance < closestMoveDistance)
        {                                                   //closest blob to movement mechanism
            closestMoveDistance = moveDistance;
            closestMoveBlob = thisBlob;
        }
    }
    closestSortDistance = fmathSqrt(closestSortDistance);
    farthestSortDistance = fmathSqrt(farthestSortDistance);
    if (closestSortDistance >= farthestSortDistance)
    {
        closestSortDistance = farthestSortDistance - 1.0f;
    }
    closestSortDistance -= smClosestMargin;
    farthestSortDistance += smFarthestMargin;

    if (RGL && glIsEnabled(GL_POLYGON_STIPPLE))
    {
        reActivateStipple = TRUE;
        glDisable(GL_POLYGON_STIPPLE);
    }
    else
    {
        reActivateStipple = FALSE;
    }

    //do 1 pass (backwards) through all the blobs to render the blobs themselves
//    node = list->tail;
//    while (node != NULL)
//    {
//        thisBlob = (blob *)listGetStructOfNode(node);
//        node = node->prev;
    for (blobIndex = smNumberBlobsSorted - 1; blobIndex >= 0; blobIndex--)
    {
        thisBlob = smBlobSortList[blobIndex];

        //compute on-screen location of the blob
        //!!! tune this circle a bit better
        selCircleComputeGeneral(modelView, projection, &thisBlob->centre, thisBlob->radius, &thisBlob->screenX, &thisBlob->screenY, &thisBlob->screenRadius);

        if (thisBlob->screenRadius <= 0.0f)
        {
            continue;
        }

        o.centreX = primGLToScreenX(thisBlob->screenX);
        o.centreY = primGLToScreenY(thisBlob->screenY);
        o.radiusX = o.radiusY = primGLToScreenScaleX(thisBlob->screenRadius);

        if ((thisBlob->flags & (BTF_Explored | BTF_ProbeDroid)) || //if player has ships in this blob
            (sensorLevel == 2 && bitTest(thisBlob->flags, BTF_UncloakedEnemies)))//or you have a sensors array and there are uncloaked enemies
        {
            //v-grad the blob color
            if (thisBlob->centre.z > 0.0f)
            {                                               //if blob is above world plane
                factor = thisBlob->centre.z / highestBlob;
                c = colBlend(smIntBlobColorHigh, smIntBlobColor, factor);
            }
            else
            {                                               //blob below the world plane
                factor = (-thisBlob->centre.z) / highestBlob;
                c = colBlend(smIntBlobColorLow, smIntBlobColor, factor);
            }
            //depth-grad the blob brightness
            factor = fmathSqrt(thisBlob->cameraSortDistance);
            factor = (factor - closestSortDistance) /
                (farthestSortDistance - closestSortDistance);
            if (factor < 0.0f)
            {
                factor = 0.0f;
            }
            if (factor > 1.0f)
            {
                factor = 1.0f;
            }
            c = colBlend(smIntBackgroundColor, c, factor);
            thisBlob->lastColor = c;
            //radius = o.radiusX * smCircleBorder / 65536;
            if (thisBlob->screenRadius <= SM_BlobRadiusMax)
            {
                radius = primGLToScreenScaleX(thisBlob->screenRadius * smCircleBorder);
                /*
                nSegments = radius * SEL_SegmentsMax / MAIN_WindowHeight + SEL_SegmentsMin;
                nSegments = min(nSegments, SEL_SegmentsMax);
                */
                nSegments = pieCircleSegmentsCompute(thisBlob->screenRadius * smCircleBorder);
                if (smFuzzyBlobs && !bitTest(gDevcaps2, DEVSTAT2_NO_IALPHA))
                {
                    primCircleBorder(o.centreX, o.centreY, o.radiusX, radius, nSegments, c);
                }
                else
                {
                    primCircleSolid2(o.centreX, o.centreY, radius, nSegments, c);
                }
            }
        }
#if BOB_VERBOSE_LEVEL >= 2
        nBlobs++;
#endif
        if (mouseInRectGeneral(o.centreX - o.radiusX, o.centreY - o.radiusY, o.centreX + o.radiusX, o.centreY + o.radiusY))
        {                                                   //if inside bounding box of circle
            distance = ABS(o.centreX - mouseCursorX()) + ABS(o.centreY - mouseCursorY());
            if (distance < closestDistance)
            {                                               //if closest yet
                closestDistance = distance;
                closestBlob = thisBlob;
            }
        }
    }

    if (reActivateStipple)
    {
        glEnable(GL_POLYGON_STIPPLE);
    }

#if SM_VERBOSE_LEVEL >= 3
    fontPrintf(20, MAIN_WindowHeight / 2, colWhite, "%d Blobs", nBlobs);
#endif
    //set the selected bit on all selected ships.  This is only a temporary flag.
    for (index = 0; index < selSelected.numShips; index++)
    {
        bitSet(selSelected.ShipPtr[index]->flags, SOF_Selected);
    }

    lightPositionSet();
    primModeClear2();
    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);
    //draw the world plane
    //!!! always where the camera is pointing !!!
    if (smCentreWorldPlane)
    {
        p0.x = p0.y = 0.0f;
    }
    else
    {
        p0 = smCamera.lookatpoint;
    }
    p0.z = 0.0f;
    smWorldPlaneDraw(&p0, TRUE, smCurrentWorldPlaneColor);

    //draw the horizon plane
    smHorizonLineDraw(&smCamera, modelView, projection, smCurrentCameraDistance);

    primModeSet2();
    smTickTextDraw();
    primModeClear2();

    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);
    //another pass through all the blobs to render the objects in the blobs
//    node = list->head;
//    while (node != NULL)
//    {
//        thisBlob = (blob *)listGetStructOfNode(node);
//        node = node->next;
    if (smTacticalOverlay)
    {
        oldFont = fontMakeCurrent(selGroupFont3);
        carrier = ShipTypeToNiceStr(Carrier);
        mothership = ShipTypeToNiceStr(Mothership);
        carrierHalfWidth = fontWidth(carrier) / 2;
        mothershipHalfWidth = fontWidth(mothership) / 2;
    }
    for (blobIndex = 0; blobIndex < smNumberBlobsSorted; blobIndex++)
    {
        thisBlob = smBlobSortList[blobIndex];
        if ((thisBlob->flags & (BTF_Explored | BTF_ProbeDroid)) || //if players in this blob
            (sensorLevel == 2 && bitTest(thisBlob->flags, BTF_UncloakedEnemies)))//or you have a sensors array and there are uncloaked enemies
        {
            smBlobDrawClear(camera, thisBlob, modelView, projection, thisBlob->lastColor);
            if (thisBlob->screenRadius > 0.0f)
            {
                if (smTacticalOverlay)
                {
                    if (bitTest(thisBlob->flags, BTF_Mothership))
                    {
                        primModeSet2();
                        fontPrint(primGLToScreenX(thisBlob->screenX) - mothershipHalfWidth, primGLToScreenY(thisBlob->screenY) + primGLToScreenScaleX(thisBlob->screenRadius), smTOColor, mothership);
                        primModeClear2();
                        rndLightingEnable(FALSE);           //mouse is self-illuminated
                        rndTextureEnable(FALSE);
                    }
                    else if (bitTest(thisBlob->flags, BTF_Carrier))
                    {
                        primModeSet2();
                        fontPrint(primGLToScreenX(thisBlob->screenX) - carrierHalfWidth, primGLToScreenY(thisBlob->screenY) + primGLToScreenScaleX(thisBlob->screenRadius), smTOColor, carrier);
                        primModeClear2();
                        rndLightingEnable(FALSE);           //mouse is self-illuminated
                        rndTextureEnable(FALSE);
                    }
                }
            }
        }
        else
        {
            smBlobDrawCloudy(camera, thisBlob, modelView, projection, thisBlob->lastColor);
        }
    }
    if (smTacticalOverlay)
    {
        fontMakeCurrent(oldFont);
    }

    //single player Sensor Manager malfunction
    //threshold is 10 to avoid divide by zero problems
    if (smSensorWeirdness > 10)
    {
        smSensorWeirdnessDraw(modelView, projection);
    }

    rndTextureEnable(FALSE);
    rndLightingEnable(FALSE);

    {
        Node *node;
        Asteroid *thisAsteroid;

        rndGLStateLog("Minor asteroids");
        for (node = universe.MinorSpaceObjList.head; node != NULL; node = node->next)
        {
            thisAsteroid = (Asteroid *)listGetStructOfNode(node);
            dbgAssert(thisAsteroid->objtype == OBJ_AsteroidType && thisAsteroid->asteroidtype == Asteroid0);
#if TO_STANDARD_COLORS
            c = teResourceColor;
#else
            c = thisAsteroid->staticinfo->staticheader.LOD->pointColor;
#endif
            primPoint3(&thisAsteroid->posinfo.position, c);
        }
    }

    //do a pass through the blobs to draw the drop-down lines and blob circles
    if (piePointSpecMode != PSM_Idle)
    {                                                   //if we're in movement mode
//        node = list->head;
//        while (node != NULL)
//        {
//            thisBlob = (blob *)listGetStructOfNode(node);
//            node = node->next;
        for (blobIndex = 0; blobIndex < smNumberBlobsSorted; blobIndex++)
        {
            thisBlob = smBlobSortList[blobIndex];

            //draw the blob cirle on the world plane
            p0 = thisBlob->centre;                              //position of the yellow circle
            c = colBlack;
            if ( (!(thisBlob->flags & (BTF_Explored | BTF_ProbeDroid))) && (sensorLevel != 2))
            {                                               //if it's all enemies
                if (thisBlob->nCapitalShips + thisBlob->nAttackShips + thisBlob->nNonCombat > thisBlob->nHiddenShips)
                {                                           //and there are visible ships here
                    if (closestMoveBlob == thisBlob && closestMoveDistance < closestMoveBlob->radius / SM_BlobClosenessFactor)
                    {
                        c = TW_SHIP_LINE_CLOSEST_COLOR;     //draw in closest move color
                        bClosestMove = TRUE;
                    }
                    else
                    {
                        c = TW_MOVE_ENEMY_COLOR;            //in enemy move color
                    }
                }
            }
            else if (thisBlob->nCapitalShips + thisBlob->nAttackShips + thisBlob->nNonCombat + thisBlob->nFriendlyShips > 0)
            {                                               //else player has ships here
                if (thisBlob->nFriendlyShips == 0)
                {
                    c = TW_MOVE_ENEMY_COLOR;                //in enemy move color
                }
                else if (closestMoveBlob == thisBlob && closestMoveDistance < closestMoveBlob->radius / SM_BlobClosenessFactor)
                {
                    c = TW_SHIP_LINE_CLOSEST_COLOR;         //draw in closest move color
                    bClosestMove = TRUE;
                }
                else
                {
                    c = TW_MOVE_PIZZA_COLOR;                //in move color
                }
            }
            if (c != colBlack)
            {                                               //if a color was specified, whatever it may have been
                p1.x = thisBlob->centre.x;                  //draw from blob to move plane
                p1.y = thisBlob->centre.y;
                p1.z = piePlanePoint.z;
                if (ABS(p1.z - p0.z) >= smShortFootLength)
                {
                    primLine3(&p0, &p1, c);                     //draw line to plane
                    selCircleComputeGeneral(modelView, projection, &p1, smBlobCircleSize, &x, &y, &screenRadius);
                    nSegments = pieCircleSegmentsCompute(screenRadius);
                    primCircleOutlineZ(&p1, smBlobCircleSize, nSegments, c);//draw the circle on plane
                }
            }
        }
    }

    //now remove the selected bit from all selected ships.
    for (index = 0; index < selSelected.numShips; index++)
    {
        bitClear(selSelected.ShipPtr[index]->flags, SOF_Selected);
    }

    if (bClosestMove && !bPlayedSound)
    {
        bPlayedSound = TRUE;
        soundEvent(NULL, UI_MoveChimes);
    }
    else if (!bClosestMove)
    {
        bPlayedSound = FALSE;
    }

    primModeSet2();
    pingListDraw(&smCamera, modelView, projection, &smViewRectangle);
    primModeClear2();
    return(closestBlob);
}

/*-----------------------------------------------------------------------------
    Name        : smBlobRenderSelected
    Description : Renders the specifed blob as though it was selected, using a colored outline.
    Inputs      : b - blob to render
                  c - color of outline to draw
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
void smBlobRenderSelected(blob *b, color c)
{
    oval o;
    sdword index;
    SpaceObj **obj;
    SpaceObjSelection *blobObjects = b->blobObjects;

    primModeSet2();
    o.centreX = primGLToScreenX(b->screenX);
    o.centreY = primGLToScreenY(b->screenY);
    o.radiusX = o.radiusY = primGLToScreenScaleX(b->screenRadius);
    primOvalArcOutline2(&o, 0.0f, TWOPI, 1, 16, c);
    primModeClear2();
    rndTextureEnable(FALSE);
    rndLightingEnable(FALSE);

    for (index = 0, obj = blobObjects->SpaceObjPtr; index < blobObjects->numSpaceObjs; index++, obj++)
    {
        rndTextureEnable(FALSE);
        primPoint3(&(*obj)->posinfo.position, c);           //everything is rendered as a point
//            (*obj)->staticinfo->staticheader.staticCollInfo.collspheresize,

    }
}
*/

/*-----------------------------------------------------------------------------
    Name        : smXXXFactorGet
    Description : Gets the current zoom factors based on the current zoom time.
    Inputs      : current - current point in function, in  frames
                  range - extent of function, in frames
    Outputs     :
    Return      : current zoom factor
    Notes       : currently a linear interpolation.
----------------------------------------------------------------------------*/
real32 smScrollFactorGet(real32 current, real32 range)
{
    real32 val;

    val = current / range;
    if (val < 0.0f)
    {
        val = 0.0f;
    }
    if (val > 1.0f)
    {
        val = 1.0f;
    }
    return(val);
}
/*
real32 smEyeFactorGet(real32 current, real32 range)
{
    return(current / range);
}
real32 smLookFactorGet(real32 current, real32 range)
{
    return(current / range);
}
*/
/*-----------------------------------------------------------------------------
    Name        : smZoomUpdate
    Description : Updates the zoom function while transitioning TO sensors manager.
    Inputs      : current - current point in function, in time
                  range - extent of function, in time
                  bUpdateCamera - update camera if true
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smZoomUpdate(real32 current, real32 range, bool bUpdateCamera)
{
    real32 scrollFactor;
    real32 eyeFactor;
    real32 lookFactor;
    sdword index, delta, val;
    vector distvec;

    scrollFactor = smScrollFactorGet(current, range);
    eyeFactor = lookFactor = scrollFactor;
//    eyeFactor = smEyeFactorGet(current, range);
//    lookFactor = smLookFactorGet(current, range);

    //animate the sides of the sensors manager screen
    val = (sdword)((real32)smScrollDistLeft * scrollFactor);
    delta = val - smScrollLeft;
    smScrollLeft = val;
    smViewRectangle.x0 += delta;
    for (index = 0; index < smScrollCountLeft; index++)
    {
        regRegionScroll(smScrollListLeft[index], delta, 0);
    }
    val = (sdword)((real32)smScrollDistTop * scrollFactor);
    delta = val - smScrollTop;
    smScrollTop = val;
    smViewRectangle.y0 += delta;
    for (index = 0; index < smScrollCountTop; index++)
    {
        regRegionScroll(smScrollListTop[index], 0, delta);
    }
    val = (sdword)((real32)smScrollDistRight * scrollFactor);
    delta = val - smScrollRight;
    smScrollRight = val;
    smViewRectangle.x1 -= delta;
    for (index = 0; index < smScrollCountRight; index++)
    {
        regRegionScroll(smScrollListRight[index], -delta, 0);
    }
    val = (sdword)((real32)smScrollDistBottom * scrollFactor);
    delta = val - smScrollBottom;
    smScrollBottom = val;
    smViewRectangle.y1 -= delta;
    for (index = 0; index < smScrollCountBottom; index++)
    {
        regRegionScroll(smScrollListBottom[index], 0, -delta);
    }

    smIntBackgroundColor = colMultiplyClamped(smBackgroundColor, scrollFactor);
    smIntBlobColor = colMultiplyClamped(smBlobColor, scrollFactor);
    smIntBlobColorHigh = colMultiplyClamped(smBlobColorHigh, scrollFactor);
    smIntBlobColorLow = colMultiplyClamped(smBlobColorLow, scrollFactor);
    rndSetClearColor(colRGBA(colRed(smIntBackgroundColor),
                             colGreen(smIntBackgroundColor),
                             colBlue(smIntBackgroundColor),
                             255));
    btgSetColourMultiplier((1.0f - smBackgroundDim) * (1.0f - scrollFactor) + smBackgroundDim);
    if (piePointSpecMode != PSM_Idle)
    {
        smCurrentWorldPlaneColor = smWorldPlaneColor;
    }
    else
    {
        smCurrentWorldPlaneColor = colMultiplyClamped(smWorldPlaneColor, scrollFactor);
    }
    //blend between the far clip distances of the 2 cameras
    smCurrentCameraDistance = (smCamera.clipPlaneFar - mrCamera->clipPlaneFar) * scrollFactor + mrCamera->clipPlaneFar;

    //animate the camera position based upon the current zoom factors
    if (bUpdateCamera)
    {
        vecVectorsBlend(&smCamera.lookatpoint, &smLookStart, &smLookEnd, lookFactor);
        vecVectorsBlend(&smCamera.eyeposition, &smEyeStart, &smEyeEnd, eyeFactor);

        vecSub(distvec, smCamera.eyeposition, smCamera.lookatpoint);
//        distvec.z *= -1.0f;
        GetDistanceAngleDeclination(&smCamera, &distvec);
    }
    //mouseClipToRect(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : smSensorsCloseForGood
    Description : Closes sensors manager after zooming in.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void smSensorsCloseForGood(void)
{
#if SM_VERBOSE_LEVEL >= 1
    dbgMessage("\nsmSensorsCloseForGood");
#endif
    if (tutorial)
    {
        tutGameMessage("Game_SensorsClose");
    }
    mrRenderMainScreen = TRUE;
    feScreenDelete(smBaseRegion);
//    bobListDelete(&smBlobList);
//    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    rndSetClearColor(colRGBA(colRed(universe.backgroundColor),
                             colGreen(universe.backgroundColor),
                             colBlue(universe.backgroundColor),
                             255));
    smSensorsActive = FALSE;
    universe.dontUpdateRenderList = FALSE;
    if (RGL)
        rglFeature(RGL_NORMDEPTH);
    if (smScrollListLeft) memFree(smScrollListLeft);
    if (smScrollListRight) memFree(smScrollListRight);
    if (smScrollListTop) memFree(smScrollListTop);
    if (smScrollListBottom) memFree(smScrollListBottom);
    universe.mainCameraCommand.actualcamera.ignoreZoom = FALSE;
    mouseCursorShow();
    mrHoldLeft = mrHoldRight = mrNULL;
    if (ioSaveState)
    {
        ioEnable();
    }

    if(smFocusOnMothershipOnClose)
    {
//      CameraStackEntry *entry;
//      entry = currentCameraStackEntry(&universe.mainCameraCommand);

        ccFocusOnMyMothership(&universe.mainCameraCommand);
        smFocusOnMothershipOnClose = FALSE;

//      if(entry != currentCameraStackEntry(&universe.mainCameraCommand))
//          listDeleteNode(&entry->stacklink);
    }

    smZoomingIn = FALSE;
    smZoomingOut = FALSE;
    mouseClipToRect(NULL);                                  //make sure mouse does not stay locked to the SM viewport region
    MP_HyperSpaceFlag = FALSE;
    bitClear(tbDisable,TBDISABLE_SENSORS_USE);
    btgSetColourMultiplier(1.0f);                           //make sure we always come out of the SM with the BTG in good order
}

void smBlobSphereDraw(blob *closestBlob)
{
    udword angle = 0;

    vector origin = {0.0, 0.0, 0.0};
    hmatrix rotmat;
    while(angle < 180)
    {
        hmatMakeRotAboutZ(&rotmat,cos(DEG_TO_RAD(angle)),sin(DEG_TO_RAD(angle)));
         hmatPutVectIntoHMatrixCol4(closestBlob->centre, rotmat);

        glPushMatrix();
        glMultMatrixf((GLfloat *)&rotmat);

        primCircleOutline3(&origin, closestBlob->radius, 8, 0, colWhite, X_AXIS);

        glPopMatrix();
        angle+=40;
    }

    angle = 0;
    while(angle < 180)
     {
         hmatMakeRotAboutX(&rotmat,cos(DEG_TO_RAD(angle)),sin(DEG_TO_RAD(angle)));
         hmatPutVectIntoHMatrixCol4(closestBlob->centre, rotmat);

         glPushMatrix();
         glMultMatrixf((GLfloat *)&rotmat);

         primCircleOutline3(&origin, closestBlob->radius, 8, 0, colWhite, Z_AXIS);

         glPopMatrix();
         angle+=40;
     }


}

/*-----------------------------------------------------------------------------
    Name        : smAllBlobsPiePlateDraw
    Description : Draws lines from all blobs to the pie plate for the
                    movement mechanism.
    Inputs      : distance -distance from camera to centre of pie plate
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
/*
void smAllBlobsPiePlateDraw(real32 distance)
{
    Node *node;
    blob *thisBlob;
    bool dohackloop = TRUE;
    real32 closestDistance = REALlyBig, length;
    vector mouse_pos,tempvec;
    real32 tempreal;
    blob *closestBlob = NULL;

    probeBlob = NULL;


    if(mrNeedProbeHack())
        dohackloop = TRUE;
    else
        dohackloop = FALSE;


    node = smBlobList.head;

    if (piePointSpecZ != 0.0f)
    {                                       //if a height was specified
        mouse_pos = pieHeightPoint;
    }
    else
    {
        mouse_pos = piePlanePoint;         //else move to point on plane
    }


    while (node)
    {
        thisBlob = (blob *)listGetStructOfNode(node);
        node = node->next;

        if(!dohackloop)
        {
            if (!bitTest(thisBlob->flags, BTF_Explored) &&
                universe.curPlayerPtr->sensorLevel != 2)
            {
                continue;
            }
            length = smBlobDropLineDraw(thisBlob, TW_SHIP_LINE_COLOR);
            if (length < closestDistance)
            {                                                   //if this ship closest so far
                closestDistance = length;
                closestBlob = thisBlob;
            }
        }
        else
        {
            //do closestblob checking first in this loop

            vecSub(tempvec,mouse_pos,thisBlob->centre);
            tempreal  = fsqrt(vecMagnitudeSquared(tempvec));
            if(tempreal < closestDistance)
            {
                closestDistance = tempreal;
                closestBlob = thisBlob;
            }
            if (!bitTest(thisBlob->flags, BTF_Explored) &&
                universe.curPlayerPtr->sensorLevel != 2)
            {
                continue;
            }
            length = smBlobDropLineDraw(thisBlob, TW_SHIP_LINE_COLOR);
        }
    }
    glDisable(GL_DEPTH_TEST);
    if(!dohackloop)
    {
        if (closestBlob != NULL && closestDistance < closestBlob->radius / SM_BlobClosenessFactor)
        {
            smBlobDropLineDraw(closestBlob, TW_SHIP_LINE_CLOSEST_COLOR);
        }
    }
    else
    {
        if (closestBlob != NULL)
        {
            probeBlob = closestBlob;
            vecSub(tempvec,mouse_pos,probeBlob->centre);
            tempreal = fsqrt(vecMagnitudeSquared(tempvec));
            if(tempreal < probeBlob->radius)
            {
                smBlobSphereDraw(closestBlob);
            }
            else
            {
                probeBlob = NULL;
            }
        }
    }
    glEnable(GL_DEPTH_TEST);
}
*/

/*-----------------------------------------------------------------------------
    Name        : smMovePlaneDraw
    Description : Draw the movement plane for the sensors manager.
    Inputs      : distance - distance from camera to selection centre
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
void smMovePlaneDraw(real32 distance)
{
    smWorldPlaneDraw(pieHeightPoint.z, FALSE, smMovePlaneColor);
}
*/
/*-----------------------------------------------------------------------------
    Name        : smBabyRattleDraw
    Description : Draw the movement lines and cursor for the movement mechanism
                    while in the sensors manager.
    Inputs      : distance - distance from camera to selection centre
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
void smBabyRattleDraw(real32 distance)
{
    ;
}
*/
/*-----------------------------------------------------------------------------
    Name        : smClickedOnPlayer
    Description : returns the a pointer to the player you right clicked on.  NULL otherwise/
    Inputs      : none
    Outputs     : player clicked on or NULL
    Return      : void
----------------------------------------------------------------------------*/
sdword smClickedOnPlayer(rectangle *viewportRect)
{
    fonthandle oldfont;
    rectangle  playerColorRect;
    sdword     index;

    oldfont = fontMakeCurrent(selGroupFont2);

    playerColorRect.y1 = viewportRect->y1 - smPlayerListMarginY;
    playerColorRect.y0 = playerColorRect.y1 - fontHeight(" ");
    playerColorRect.x0 = viewportRect->x0 + smPlayerListMarginX;
    playerColorRect.x1 = playerColorRect.x0 + fontHeight(" ");

    //draw the list of player names/colors
    for (index = universe.numPlayers - 1; index >= 0; index--)
    {
        if (universe.players[index].playerState==PLAYER_ALIVE)
        {
            playerColorRect.x1 = fontWidth(playerNames[index]) + fontHeight(" ")*2;
            if ((mouseInRect(&playerColorRect)) && (index!=sigsPlayerIndex))
            {
                return (index);
            }
            playerColorRect.y0 -= smPlayerListMarginY + fontHeight(" ");//update the position
            playerColorRect.y1 -= smPlayerListMarginY + fontHeight(" ");
        }
    }

    fontMakeCurrent(oldfont);

    return (-1);
}


/*-----------------------------------------------------------------------------
    Name        : smPlayerNamesDraw
    Description : Draw the names of the players in a multi-player game.
    Inputs      : viewportRect - boundaries of the current viewport
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smPlayerNamesDraw(rectangle *viewportRect)
{
    fonthandle oldfont;
    sdword index;
    rectangle playerColorRect;
    color c;
    char buffer[50];
    sdword bounty;

    oldfont = fontMakeCurrent(selGroupFont2);

    playerColorRect.y1 = viewportRect->y1 - smPlayerListMarginY;
    playerColorRect.y0 = playerColorRect.y1 - fontHeight(" ");
    playerColorRect.x0 = viewportRect->x0 + smPlayerListMarginX;
    playerColorRect.x1 = playerColorRect.x0 + fontHeight(" ");

    //draw the list of player names/colors
    for (index = universe.numPlayers - 1; index >= 0; index--)
    {
        if (universe.players[index].playerState==PLAYER_ALIVE)
        {
            sdword x;
            bool playerhasdroppedOutOrQuit = playerHasDroppedOutOrQuit(index);

            c = teColorSchemes[index].textureColor.base;
            if (playerhasdroppedOutOrQuit)
            {
                c = HorseRaceDropoutColor;
            }

            primRectSolid2(&playerColorRect, c);                 //draw colored rectangle
            x = playerColorRect.x1 + smPlayerListTextMargin;
            fontPrint(x, //print the player name in a stock color
                      playerColorRect.y0, playerhasdroppedOutOrQuit ? HorseRaceDropoutColor : smPlayerListTextColor, playerNames[index]);
            x += fontWidth(playerNames[index])+fontWidth(" ");
            if (index != sigsPlayerIndex)
            {
                if ((allianceArePlayersAllied(&universe.players[sigsPlayerIndex],&universe.players[index])) ||
                    ((universe.players[sigsPlayerIndex].Allies!=0)&&(index == sigsPlayerIndex)))
                {
                    fontPrint(x,
                              playerColorRect.y0, playerhasdroppedOutOrQuit ? HorseRaceDropoutColor : alliescolor, strGetString(strPlayersAllied));
                }
            }

            x += fontWidth(strGetString(strPlayersAllied));
            if((!singlePlayerGame) && (tpGameCreated.bountySize != MG_BountiesOff))
            {
                //draw bounty values
                fontPrint(x,
                          playerColorRect.y0, playerhasdroppedOutOrQuit ? HorseRaceDropoutColor : colWhite, "B: ");
                bounty = getPlayerBountyRender(&universe.players[index]);
                /*itoa(bounty,buffer,10);*/
                sprintf(buffer, "%d", bounty);
                x += fontWidth("B: ");

                fontPrint(x, playerColorRect.y0, playerhasdroppedOutOrQuit ? HorseRaceDropoutColor : colWhite, buffer);

                x += fontWidth(buffer) + fontWidth(" ");
            }

            if (playerhasdroppedOutOrQuit)
            {
                fontPrint(x,playerColorRect.y0, c, (playersReadyToGo[index] == PLAYER_QUIT) ? strGetString(strQuit) : strGetString(strDroppedOut));
            }

            playerColorRect.y0 -= smPlayerListMarginY + fontHeight(" ");//update the position
            playerColorRect.y1 -= smPlayerListMarginY + fontHeight(" ");
        }
    }

    fontMakeCurrent(oldfont);
}
/*-----------------------------------------------------------------------------
    Name        : smCursorTextDraw
    Description : Draw sensors manager tactical overlay.  This includes the
                    player names/colors and a breakdown of what's in a mission sphere.
    Inputs      : viewportRect - boundaries of the current viewport
                  selectedBlob - blob under mouse cursor, or NULL if none.
                  sensorLevel - current sensors level
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smCursorTextDraw(rectangle *viewportRect, blob *selectedBlob, sdword sensorLevel)
{
    char *cursorText = NULL;
    SpaceObj *object;
    fonthandle fhSave;

    //now draw the cursor text to reflect what is under the mouse cursor
    if (selectedBlob != NULL)
    {                                                       //if the mouse is over a blob
        if ((selectedBlob->flags & (BTF_Explored | BTF_ProbeDroid)))
        {                                                   //if this blob has been explored
            if ((object = selClickFromArray(selectedBlob->blobObjects->SpaceObjPtr,
                selectedBlob->blobObjects->numSpaceObjs, mouseCursorX(),mouseCursorY())) != NULL)
            {
                switch (object->objtype)
                {
                    case OBJ_ShipType:
                        if (((Ship *)object)->playerowner == universe.curPlayerPtr)
                        {
//                            cursorText = strGetString(strPlayerUnits);
                            cursorText = ShipTypeToNiceStr(((Ship *)object)->shiptype);
                        }
                        else if (allianceIsShipAlly((Ship *)object, universe.curPlayerPtr))
                        {
                            cursorText = strGetString(strAlliedUnits);
                        }
                        else
                        {
                            cursorText = strGetString(strEnemyUnits);
                        }
                        break;
                    case OBJ_AsteroidType:
                    case OBJ_NebulaType:
                    case OBJ_GasType:
                    case OBJ_DustType:
                        cursorText = strGetString(strResources);
                        break;
                    case OBJ_DerelictType:
                        switch (((Derelict *)object)->derelicttype)
                        {
                            case PrisonShipOld:
                            case PrisonShipNew:
                            case Shipwreck:
                            case JunkAdvanceSupportFrigate:
                            case JunkCarrier:
                            case JunkDDDFrigate:
                            case JunkHeavyCorvette:
                            case JunkHeavyCruiser:
                            case JunkIonCannonFrigate:
                            case JunkLightCorvette:
                            case JunkMinelayerCorvette:
                            case JunkMultiGunCorvette:
                            case JunkRepairCorvette:
                            case JunkResourceController:
                            case JunkSalCapCorvette:
                            case JunkStandardFrigate:
                                cursorText = strGetString(strDerelictShip);
                                break;
                            default:
                                cursorText = NULL;
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        else
        {                                                   //else unexplored blob
            if (sensorLevel == 0)
            {                                               //don't know anything on plain blobs
                cursorText = strGetString(strUnexplored);
            }
            else
            {                                               //else we've got some sensors available
                if ((object = selClickFromArray(selectedBlob->blobObjects->SpaceObjPtr,
                    selectedBlob->blobObjects->numSpaceObjs, mouseCursorX(),mouseCursorY())) != NULL)
                {
                    switch (object->objtype)
                    {
                        case OBJ_ShipType:
                            cursorText = strGetString(strEnemyUnits);
                            break;
                        case OBJ_AsteroidType:
                            cursorText = strGetString(strAsteroids);
                            break;
                        case OBJ_NebulaType:
                            cursorText = strGetString(strNebula);
                            break;
                        case OBJ_GasType:
                            cursorText = strGetString(strGasClouds);
                            break;
                        case OBJ_DustType:
                            cursorText = strGetString(strDustClouds);
                            break;
                        case OBJ_DerelictType:
                            cursorText = strGetString(strDerelictShip);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    //cursor text chosen, print the cursor text if applicable
    if (cursorText != NULL)
    {
        fhSave = fontMakeCurrent(mouseCursorFont);
        fontPrint(viewportRect->x0 + smPlayerListMarginX,
                  viewportRect->y1 - fontHeight(cursorText) - smPlayerListMarginY,
                  TW_CURSORTEXT_COLOR, cursorText);
        fontMakeCurrent(fhSave);
    }
}

/*-----------------------------------------------------------------------------
    Name        : smTacticalOverlayDraw
    Description : Draw the tactical overlay for sensors manager, including
                  resource types and such
    Inputs      : thisBlob - blob to draw
                  viewportRect - rectangle encompassing the sensors manager
                  sensorLevel - current sensors level
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*
void smTacticalOverlayDraw(blob *thisBlob, rectangle *viewportRect, sdword sensorLevel)
{
    sdword x, y, yStart, xSpacing;

    dbgAssert(thisBlob != NULL);
    x = mouseCursorX() + smTOBottomCornerX;
    yStart = y = mouseCursorY() - fontHeight(" ") + smTOBottomCornerY;
    if (thisBlob->RUs != 0)
    {                                                   //if there's resources in the blob
        if ((sensorLevel == 2 && bitTest(thisBlob->flags, BTF_Explored)) || bitTest(thisBlob->flags, BTF_ProbeDroid))
        {                                               //if max sensor level, print breakdown of resources in the blob
            xSpacing = fontWidth(strGetString(strResourcesRes))+fontWidth(" ");
            if (thisBlob->nGasRUs != 0)
            {
                fontPrintf(x + xSpacing, y, smTOColor, "%d %s", thisBlob->nGasRUs, strGetString(strGas));
                y -= fontHeight(" ") + smTOLineSpacing;
            }
            if (thisBlob->nDustRUs != 0)
            {
                fontPrintf(x + xSpacing, y, smTOColor, "%d %s", thisBlob->nDustRUs,strGetString(strDust));
                y -= fontHeight(" ") + smTOLineSpacing;
            }
            if (thisBlob->nRockRUs != 0)
            {
                fontPrintf(x + xSpacing, y, smTOColor, "%d %s", thisBlob->nRockRUs, strGetString(strRock));
                y -= fontHeight(" ") + smTOLineSpacing;
            }
            y += fontHeight(" ") + smTOLineSpacing;
            fontPrintf(x, y, smTOColor, "%s ",strGetString(strResourcesRes));
            y -= fontHeight(" ") + smTOLineSpacing;
        }
        else if (sensorLevel >= 1 || bitTest(thisBlob->flags, BTF_Explored))
        {                                               //else crappy sensors; just print the types
            fontPrintf(x, y, smTOColor, "%s %s %s %s", strGetString(strResourcesRes),
                (thisBlob->flags & (BTF_Asteroid)) ? strGetString(strRock) : "",
                (thisBlob->flags & (BTF_DustCloud)) ? strGetString(strDust) : "",
                (thisBlob->flags & (BTF_GasCloud | BTF_Nebula)) ? strGetString(strGas) : "");
            y -= fontHeight(" ") + smTOLineSpacing;
        }
    }
    //print types and numbers of ships
    if (thisBlob->flags & (BTF_Explored | BTF_ProbeDroid))
    {
        if (sensorLevel == 2 || bitTest(thisBlob->flags, BTF_ProbeDroid))
        {                                                   //level 3 explored
            if (thisBlob->nNonCombat > 0)
            {
                fontPrintf(x, y, smTOColor, "%d %s", thisBlob->nNonCombat, strGetString(strNonCombatShips));
                y -= fontHeight(" ") + smTOLineSpacing;
            }
            if (thisBlob->nAttackShips > 0)
            {
                fontPrintf(x, y, smTOColor, "%d %s", thisBlob->nAttackShips, strGetString(strStrikeCraft));
                y -= fontHeight(" ") + smTOLineSpacing;
            }
            if (thisBlob->nCapitalShips > 0)
            {
                fontPrintf(x, y, smTOColor, "%d %s", thisBlob->nCapitalShips, strGetString(strCapitalShips));
                y -= fontHeight(" ") + smTOLineSpacing;
            }
        }
        else if (sensorLevel == 0)
        {                                                   //level 0 explored
            xSpacing = thisBlob->nCapitalShips + thisBlob->nAttackShips + thisBlob->nNonCombat;
            if (xSpacing > 0)
            {
                fontPrintf(x, y, smTOColor, "%d %s", xSpacing, strGetString(strEnemyUnits));
                y -= fontHeight(" ") + smTOLineSpacing;
            }
        }
        else if (sensorLevel == 1)
        {                                                   //level 1 explored
            xSpacing = thisBlob->nCapitalShips + thisBlob->nAttackShips + thisBlob->nNonCombat;
            if (xSpacing > 0)
            {
                fontPrint(x, y, smTOColor, strGetString(strEnemyUnits));
                y -= fontHeight(" ") + smTOLineSpacing;
            }
        }
    }
    else if (sensorLevel >= 1)
    {                                                       //level 0 unexplored
        xSpacing = thisBlob->nCapitalShips + thisBlob->nAttackShips + thisBlob->nNonCombat;
        if (xSpacing > 0)
        {
            fontPrint(x, y, smTOColor, strGetString(strEnemyUnits));
            y -= fontHeight(" ") + smTOLineSpacing;
        }
    }
    //if nothing was printed, just say it's unexplored
    if (y == yStart)
    {
        fontPrint(x, y, smTOColor, strGetString(strUnexplored));
    }
}
*/

/*-----------------------------------------------------------------------------
    Name        : smHotkeyGroupsDraw
    Description : Draw the location of hotkey groups.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smHotkeyGroupsDraw(void)
{
    sdword index;
    real32 averageSize, x, y, radius;
    vector centre;
    fonthandle currentFont = fontCurrentGet();
    MaxSelection selection;

    fontMakeCurrent(selGroupFont3);
    for (index = ZEROKEY; index <= NINEKEY; index++)
    {
        MakeShipsNotHidden((SelectCommand *)&selection, (SelectCommand *)&selHotKeyGroup[index - ZEROKEY]);
        if (selection.numShips > 0)
        {
            centre = selCentrePointComputeGeneral(&selection, &averageSize);
            selCircleComputeGeneral(&rndCameraMatrix, &rndProjectionMatrix, &centre, 1.0f, &x, &y, &radius);
            if (radius > 0.0f)
            {
                fontPrint(primGLToScreenX(x) + smHotKeyOffsetX,
                           primGLToScreenY(y) + smHotKeyOffsetY,
                           selHotKeyNumberColor, selHotKeyString[index - ZEROKEY]);
            }
        }
    }
    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : smPan
    Description : Special-function button that handles both clicks and pan operations.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void smPan(char *name, featom *atom)
{
    if (uicDragX == 0 && uicDragY == 0)
    {                                                       //if it's just a click
        smCameraLookatPoint.x = smCameraLookatPoint.y = smCameraLookatPoint.z = 0.0f;//recentre the camera
    }
    else
    {                                                       //else it's a drag operation
        vector addVector, lookVector;
        real32 mult, temp;

        vecSub(lookVector, smCamera.lookatpoint, smCamera.eyeposition);
        addVector.x = lookVector.x;
        addVector.y = lookVector.y;
        addVector.z = 0.0f;
        vecNormalize(&addVector);
        mult = smCamera.distance * smPanSpeedMultiplier;
        vecScalarMultiply(addVector, addVector, mult);      //into/out of screen vector
        vecScalarMultiply(lookVector, addVector, (real32)uicDragY);
        vecSubFrom(smCameraLookatPoint, lookVector);          //pan into/out of screen

        temp = addVector.x;                                 //negate-swap x,y
        addVector.x = addVector.y;
        addVector.y = -temp;                                //left/right vector

        vecScalarMultiply(lookVector, addVector, (real32)uicDragX);
        vecAddTo(smCameraLookatPoint, lookVector);          //pan left/right

        lookVector.x = lookVector.y = lookVector.z = 0.0f;
        pieMovePointClipToLimits(smUniverseSizeX * smPanUnivExtentMult,
                                 smUniverseSizeY * smPanUnivExtentMult,
                                 smUniverseSizeZ, &lookVector, &smCameraLookatPoint);
    }
//    smCamera.lookatpoint = smCameraLookatPoint;
}

/*-----------------------------------------------------------------------------
    Name        : smViewportRender
    Description : Callback which draws the main sensors manager viewport.
    Inputs      : standard FE callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void smViewportRender(featom *atom, regionhandle region)
{
    blob *selectedBlob;
    CameraStackEntry *curentry = currentCameraStackEntry(&universe.mainCameraCommand);
    sdword frameTicks, sensorLevel;
    real32 frameStep;
    vector rememberVec, diffVec;
    sdword index;

    frameTicks = utyNFrameTicks;
    if (frameTicks > smMaxFrameTicks)
        frameTicks = smMaxFrameTicks;

    frameStep = (real32)frameTicks / (real32)taskFrequency;
    cameraControl(&smCamera, FALSE);                         //update the camera
    curentry->remembercam.angle = smCamera.angle;
    curentry->remembercam.declination = smCamera.declination;

    if (smZoomingOut)
    {
        smZoomTime += frameStep;
        smZoomUpdate(smZoomTime, smCurrentZoomLength, TRUE);
        if (smZoomTime >= smCurrentZoomLength)
        {                                                   //if done zooming out
            soundEvent(NULL, UI_SensorsManager);
            smZoomingOut = FALSE;                           //stop zooming
        }
        if (smZoomTime < smCurrentMainViewZoomLength)
        {                                                   //if still in the regular renderer part of the zoom
            cameraCopyPositionInfo(&universe.mainCameraCommand.actualcamera, &smCamera);
            universe.mainCameraCommand.actualcamera.ignoreZoom = TRUE;
            return;                                         //let mainrgn.c handle the rendering
        }
        else
        {                                                   //else we're in the SM rendering part
            mrRenderMainScreen = FALSE;

            dbgAssert(smZoomTime >= smCurrentMainViewZoomLength);
            universe.mainCameraCommand.actualcamera.ignoreZoom = FALSE;
        }
    }
    else if (smZoomingIn)
    {
        dbgAssert(smEyeStart.x < (real32)1e19 && smEyeStart.x > (real32)-1e19);
        smZoomTime -= frameStep;
        smZoomUpdate(smZoomTime, smCurrentZoomLength, TRUE);
        if (smZoomTime <= 0.0f)
        {                                                   //fully zoomed in
            smZoomingIn = FALSE;
            //...actually close the sensors manager
            smSensorsCloseForGood();                        //stop zooming in already!
        }

        if (smZoomTime >= smCurrentMainViewZoomLength)
        {                                                   //if in sm part of zoom
            cameraCopyPositionInfo(&universe.mainCameraCommand.actualcamera, &smCamera);
            universe.mainCameraCommand.actualcamera.ignoreZoom = TRUE;
        }
        else
        {                                                   //else rendering main game screen
            if (!smFocusTransition)
            {                                               //if at transition point
                smFocusTransition = TRUE;                   //make sure we only do this once a zoom
                //... execute the ccfocus thingus
                if (smFocus)
                {
                    universe.mainCameraCommand.actualcamera.ignoreZoom = FALSE;
                    ccFocusFar(&universe.mainCameraCommand, smFocusCommand, &universe.mainCameraCommand.actualcamera);
                    memFree(smFocusCommand);
                    smFocusCommand = NULL;
                }
                mrRenderMainScreen = TRUE;

                soundEvent(NULL, UI_SoundOfSpace);
            }
            else
            {                                               //else the transition has already been made
                if (!smFocus && smZoomTime > 0.0f)
                {                                           //if not at end of zoom in with no focus
                    cameraCopyPositionInfo(&universe.mainCameraCommand.actualcamera, &smCamera);
                    universe.mainCameraCommand.actualcamera.ignoreZoom = TRUE;
                }
                return;
            }
        }
    }
    else
    {
        smCurrentWorldPlaneColor = smWorldPlaneColor;
        smCurrentCameraDistance = smCamera.clipPlaneFar;
    }
    smViewportRegion->rect = smViewRectangle;
    if (piePointSpecMode != PSM_Idle)
    {
        smCurrentWorldPlaneColor = colMultiplyClamped(smCurrentWorldPlaneColor, smMovementWorldPlaneDim);
    }

    //perform some render-time monkey work of the camera position
    if (!smZoomingIn && !smZoomingOut)
    {                                               //don't move the eye point if zooming
        uicDragX = uicDragY = 0;
        if (keyIsHit(ARRLEFT))
        {                                                       //pan the camera with the cursor keys
            uicDragX -= smCursorPanX * frameTicks / (udword)taskFrequency;
        }
        if (keyIsHit(ARRRIGHT))
        {
            uicDragX += smCursorPanX * frameTicks / (udword)taskFrequency;
        }
        if (keyIsHit(ARRUP))
        {
            uicDragY -= smCursorPanY * frameTicks / (udword)taskFrequency;
        }
        if (keyIsHit(ARRDOWN))
        {
            uicDragY += smCursorPanY * frameTicks / (udword)taskFrequency;
        }
        if (uicDragX != 0 || uicDragY != 0)
        {
            smPan(NULL, NULL);
        }
        //perform cubic spline interpolation of the panned camera point
        vecSub(rememberVec, smCamera.lookatpoint, smCameraLookatPoint);
        if (ABS(rememberVec.x) + ABS(rememberVec.y) + ABS(rememberVec.z) > smPanEvalThreshold)
        {                                                   //if there is any distance between pan point and camera point
            while (frameTicks)
            {
                rememberVec = smCamera.lookatpoint;
                EvalCubic(&smCamera.lookatpoint.x, &smCameraLookVelocity.x, smCameraLookatPoint.x, smPanTrack);
                EvalCubic(&smCamera.lookatpoint.y, &smCameraLookVelocity.y, smCameraLookatPoint.y, smPanTrack);
                EvalCubic(&smCamera.lookatpoint.z, &smCameraLookVelocity.z, smCameraLookatPoint.z, smPanTrack);

                vecSub(diffVec, smCamera.lookatpoint, rememberVec);
                vecAddTo(smCamera.eyeposition, diffVec);    //pan the eyeposition with the lookatpoint
                frameTicks--;
            }
        }
    }
    //now on to the actual rendering code:
    cameraSetEyePosition(&smCamera);

    primModeClear2();
    rndLightingEnable(FALSE);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    rgluPerspective(smCamera.fieldofview, rndAspectRatio,    //set projection matrix
                   smCamera.clipPlaneNear, smCamera.clipPlaneFar);
    glScalef(smProjectionScale, smProjectionScale, smProjectionScale);
    glMatrixMode( GL_MODELVIEW );

    glLoadIdentity();
#if RND_CAMERA_OFFSET
    rgluLookAt(smCamera.eyeposition.x + RND_CameraOffsetX, smCamera.eyeposition.y,
              smCamera.eyeposition.z, smCamera.lookatpoint.x + RND_CameraOffsetX,
              smCamera.lookatpoint.y, smCamera.lookatpoint.z,
              smCamera.upvector.x, smCamera.upvector.y, smCamera.upvector.z);
#else
    rgluLookAt(smCamera.eyeposition.x, smCamera.eyeposition.y,
              smCamera.eyeposition.z, smCamera.lookatpoint.x,
              smCamera.lookatpoint.y, smCamera.lookatpoint.z,
              smCamera.upvector.x, smCamera.upvector.y, smCamera.upvector.z);
#endif
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)(&rndCameraMatrix));
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)(&rndProjectionMatrix));

    //Draw the BTG background
    glPushMatrix();
    glTranslatef(smCamera.eyeposition.x, smCamera.eyeposition.y, smCamera.eyeposition.z);
    rndBackgroundRender(BG_RADIUS, &smCamera, FALSE);  //render the background
    glPopMatrix();

    lightSetLighting();
    rndLightingEnable(TRUE);

    //render all worlds in the universe
    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        if (universe.world[index] != NULL)
        {
            if (universe.world[index]->objtype == OBJ_DerelictType)
            {
                rndRenderAHomeworld(&smCamera, universe.world[index]);
            }
        }
        else
        {
            break;
        }
    }

    //update and draw all the sensors manager blobs
    smRenderCount++;
    if (smRenderCount >= smBlobUpdateRate)
    {
        smRenderCount = 0;
    }
    if (smGhostMode)
    {
        sensorLevel = 2;
    }
    else
    {
        sensorLevel = universe.curPlayerPtr->sensorLevel;
    }
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)(&rndCameraMatrix));
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)(&rndProjectionMatrix));

    selectedBlob = smBlobsDraw(&smCamera, &universe.collBlobList, &rndCameraMatrix, &rndProjectionMatrix, universe.curPlayerPtr->sensorLevel);

    if (piePointSpecMode != PSM_Idle)
    {                                                       //draw point spec
        mrCamera = &smCamera;                               //point spec draw uses mrCamera
        pieLineDrawCallback = NULL;
        piePlaneDrawCallback = piePlaneDraw;
        pieMovementCursorDrawCallback = pieMovementCursorDraw;
        piePointSpecDraw();                                 //draw the movement mechanism
    }

    primModeSet2();

    //draw the tactical overlay, if applicable
    if (smTacticalOverlay)
    {
#if !TO_STANDARD_COLORS
        if (!singlePlayerGame)
        {                                                   //only print player names if multi-player game is underway
            smPlayerNamesDraw(&smViewRectangle);            //draw the list of player names
        }
#endif
        smHotkeyGroupsDraw();
    }
    if (smHoldRight == smNULL && smHoldLeft == smNULL)
    {
        smCursorTextDraw(&smViewRectangle, selectedBlob, universe.curPlayerPtr->sensorLevel);
    }

    if (smHoldLeft == smSelectHold)                         //and then draw the current selection progress
    {
        primRectOutline2(&smSelectionRect, 1, TW_SELECT_BOX_COLOR);
    }
    if (nisStatic[NIS_SMStaticIndex] != NULL)
    {
        nisStaticDraw(nisStatic[NIS_SMStaticIndex]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : smNULL
    Description : NULL logic handler for when the user's not doing anything with
                    the sensors manager.
    Inputs      : void
    Outputs     : void
    Return      : void
----------------------------------------------------------------------------*/
void smNULL(void)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : mrSelectHold
    Description : Function for dragging a selection box
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void smSelectHold(void)
{
    if (ABS(mouseCursorX() - mrOldMouseX) >= selClickBoxWidth ||
        ABS(mouseCursorY() - mrOldMouseY) >= selClickBoxHeight)
    {                                                       //if mouse has moved from anchor point
        mrSelectRectBuild(&smSelectionRect, mrOldMouseX, mrOldMouseY);//create a selection rect
        selRectNone();
    }
    else
    {                                                       //else mouse hasn't moved far enough
        smSelectionRect.x0 = smSelectionRect.x1 =           //select nothing
            smSelectionRect.y0 = smSelectionRect.y1 = 0;
        selRectNone();
    }
}

/*-----------------------------------------------------------------------------
    Name        : smCullAndFocusSelecting
    Description : Culls the current selection list to a reasonable size and
                    focuses on it.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smCullAndFocusSelecting(void)
{
    sdword index, nToFocusOn;
    vector centre = {0.0f, 0.0f, 0.0f};

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bSensorsClose)
    {
        return;
    }

    nToFocusOn = ccFocusCullRadiusMean((FocusCommand *)&selSelecting, smFocusRadius * smFocusRadius, &centre);
    if (nToFocusOn == 0)
    {                                                       //nothing to focus on
        return;
    }
    //now we have a full listing of what we can focus on.  Let's focus on it.
    smFocus = TRUE;
    smFocusCommand = memAlloc(ccFocusSize(selSelecting.numTargets), "SensorsMultiFocus", 0);
    for (index = 0; index < selSelecting.numTargets; index++)
    {
        smFocusCommand->ShipPtr[index] = (Ship *)selSelecting.TargetPtr[index];
    }
    smFocusCommand->numShips = selSelecting.numTargets;
    dbgAssert(!smZoomingOut);
    smZoomingIn = TRUE;

    /* call the sound event for closing the Sensors manager */
    soundEvent(NULL, UI_SensorsExit);

    smFocusTransition = FALSE;
    smEyeEnd = smCamera.eyeposition;
    smLookEnd = smCamera.lookatpoint;
    smLookStart = centre;
    vecSub(smEyeStart, smLookEnd, smEyeEnd);
    vecNormalize(&smEyeStart);
    dbgAssert(smEyeStart.x < (real32)1e19 && smEyeStart.x > (real32)-1e19);
    vecMultiplyByScalar(smEyeStart, CAMERA_FOCUSFAR_DISTANCE);
    vecSub(smEyeStart, smLookStart, smEyeStart);
    smInitialDistance = smCamera.distance;//remember zoom-back distance for next time
    selRectNone();
    //now create a render list as it will be when fully zoomed in
    smTempCamera.lookatpoint = smLookStart;
    smTempCamera.eyeposition = smEyeStart;
    mrCamera = &smTempCamera;
    univUpdateRenderList();
    dbgAssert(smEyeStart.x < (real32)1e19 && smEyeStart.x > (real32)-1e19);

    //the player will get kicked into the sensors manager after several seconds waiting
    //with the camera nowhere near their ships.  This is a security "anti-spy" measure.
    //If when kicked into the SM the player focuses on their own ships explicitly, we
    //don't want them to focus on their mothership, rather on the ships they specified.
    smFocusOnMothershipOnClose = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : smViewportProcess
    Description : process messages for the main viewport of the sensors manager.
    Inputs      : standard region callback parameters
    Outputs     : ..
    Return      : ..
----------------------------------------------------------------------------*/
udword smViewportProcess(regionhandle region, sdword ID, udword event, udword data)
{
//    Ship *ship;
//    void *a, *b;
    if ((smZoomingIn)||(smZoomingOut))
    {
        return 0;
    }

    switch (event)
    {
        case RPE_HoldLeft:
            smHoldLeft();                                   //call current hold function
            break;
        case RPE_HoldRight:
            smHoldRight();                                  //call current hold function
            break;
        case RPE_PressLeft:
            mouseClipToRect(&smViewRectangle);
            if (smHoldRight == smNULL && (!smFleetIntel))
            {
                smClicking = TRUE;
            }
            if (smHoldRight == smNULL && (!smFleetIntel))
            {
                mrOldMouseX = mouseCursorX();               //store anchor point
                mrOldMouseY = mouseCursorY();
                smHoldLeft = smSelectHold;                  //set to select mode
                smSelectionRect.x0 = smSelectionRect.x1 =   //select nothing
                    smSelectionRect.y0 = smSelectionRect.y1 = 0;
            }
            break;
        case RPE_ReleaseLeft:
            if ((piePointSpecMode & (PSM_XY | PSM_Z)) != 0)
            {                                               //if specified a point
                vector destination;

                dbgAssert(!smFleetIntel);
                MakeShipsMobile((SelectCommand *)&selSelected);
                if (selSelected.numShips > 0)
                {
                    if (piePointSpecZ != 0.0f)
                    {                                       //if a height was specified
                        destination = pieHeightPoint;
                    }
                    else
                    {
                        destination = piePlanePoint;         //else move to point on plane
                    }

             /** !!!!  We should put the speech event AFTER incase the movement destination is
                        invalid...in the probes case this could be often...very often   **/
                    /**  Done :)  **/
                    /** Hooray !! **/
                    if (!(isnan((double)destination.x) || isnan((double)destination.x) || isnan((double)destination.x)))
                    {
                        if(mrNeedProbeHack())
                        {   //looks like we need a little probe hacksy poo here too
                            clWrapMove(&universe.mainCommandLayer,(SelectCommand *)&selSelected,selCentrePoint,destination);
                            speechEventFleetSpec(selSelected.ShipPtr[0], COMM_F_Probe_Deployed, 0, universe.curPlayerIndex);
                        }
                        else
                        {   //move normally
                            if(MP_HyperSpaceFlag)
                            {
                                vector distancevec;
                                real32 cost;
                                //we're in hyperspace pieplate mode!
                                vecSub(distancevec,selCentrePoint,destination);
                                cost = hyperspaceCost(fsqrt(vecMagnitudeSquared(distancevec)),(SelectCommand *)&selSelected);
                                if(universe.curPlayerPtr->resourceUnits < ((sdword)cost))
                                {
                                    //can't afford it!
                                    //good speech event?
                                    if (battleCanChatterAtThisTime(BCE_CannotComply, selSelected.ShipPtr[0]))
                                    {
                                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CannotComply, selSelected.ShipPtr[0], SOUND_EVENT_DEFAULT);
                                    }
                                }
                                else
                                {
                                    //make unselectable in wrap function
                                    clWrapMpHyperspace(&universe.mainCommandLayer,(SelectCommand *)&selSelected,selCentrePoint,destination);
                                }
                            }
                            else
                            {
                                //normal pieplate movement
                                clWrapMove(&universe.mainCommandLayer,(SelectCommand *)&selSelected,selCentrePoint,destination);
                                if (selSelected.ShipPtr[0]->shiptype == Mothership)
                                {
                                    speechEventFleetSpec(selSelected.ShipPtr[0], COMM_F_MoShip_Move, 0, universe.curPlayerIndex);
                                }
                                else
                                {
                                    speechEvent(selSelected.ShipPtr[0], COMM_Move, 0);
                                }
                            }
                        }

                        //untoggle correct button!
                        if(MP_HyperSpaceFlag)
                        {
                            feToggleButtonSet(SM_Hyperspace, FALSE);
                        }
                        else
                        {
                            feToggleButtonSet(SM_Dispatch, FALSE);
                        }
                    }
                }
//                piePointSpecMode = PSM_Idle;                 //no more selecting
                piePointModeOnOff();
                MP_HyperSpaceFlag = FALSE;      //for now..
            }
            else if (smClicking && !smFleetIntel)
            {
                Ship *clickedOn = NULL;

                if (ABS(mouseCursorX() - mrOldMouseX) < selClickBoxWidth &&
                    ABS(mouseCursorY() - mrOldMouseY) < selClickBoxHeight)
                {                                           //if mouse has moved much
                    smSelectionRect.x0 = mouseCursorX() - smSelectionWidth / 2;
                    smSelectionRect.x1 = mouseCursorX() + smSelectionWidth / 2;
                    smSelectionRect.y0 = mouseCursorY() - smSelectionHeight / 2;
                    smSelectionRect.y1 = mouseCursorY() + smSelectionHeight / 2;
                    clickedOn = selSelectionClick(universe.SpaceObjList.head, &smCamera, mouseCursorX(), mouseCursorY(), FALSE, FALSE);
                    if (clickedOn != NULL)
                    {
                        if (clickedOn->playerowner != universe.curPlayerPtr && (!smGhostMode))
                        {
                            clickedOn = FALSE;
                        }
                    }
                }
                else
                {
                    mrSelectRectBuild(&smSelectionRect, mrOldMouseX, mrOldMouseY);//create a selection rect
                }
                if (smHoldRight == smNULL)
                {
                    if (smGhostMode)
                    {
                        selRectDragAnybodyAnywhere(&smCamera, &smSelectionRect);//and select anything
                    }
                    else
                    {
//                        selRectDragAnywhere(&smCamera, &smSelectionRect);//and select anything
                        selRectDragAnybodyAnywhere(&smCamera, &smSelectionRect);//and select anything
                        MakeShipsFriendlyAndAlliesShips((SelectCommand *)&selSelecting, universe.curPlayerPtr);
                        makeShipsNotBeDisabled((SelectCommand *)&selSelecting);
                    }
                    if (clickedOn != NULL)
                    {
                        if(!(clickedOn->flags & SOF_Disabled))
                        {
                            selSelectionAddSingleShip((MaxSelection *)(&selSelecting), clickedOn);
                        }
                    }
                    smCullAndFocusSelecting();
                }
            }
            mouseClipToRect(NULL);
            smHoldLeft = smNULL;
            smClicking = FALSE;
            break;
        case RPE_PressRight:
            mouseClipToRect(&smViewRectangle);
            smHoldLeft = smNULL;
            smHoldRight = mrCameraMotion;
            mrOldMouseX = mouseCursorX();                   //save current mouse location for later restoration
            mrOldMouseY = mouseCursorY();
            mouseCursorHide();                              //hide cursor and move to centre of the screen
            mousePositionSet(MAIN_WindowWidth / 2, MAIN_WindowHeight / 2);
            mrMouseHasMoved = 0;                            //mouse hasn't moved yet
            region->rect.x0 = region->rect.y0 = 0;          //make it's rectangle full-screen
            region->rect.x1 = MAIN_WindowWidth;
            region->rect.y1 = MAIN_WindowHeight;
            smClicking = FALSE;
            piePointModePause(TRUE);                         //pause the point spec mode
            break;
        case RPE_ReleaseRight:
            if (smHoldRight == mrCameraMotion)
            {                                               //if in camera movement mode
                mousePositionSet(mrOldMouseX, mrOldMouseY); //restore mouse position
                mouseCursorShow();                          //show mouse cursor
                smHoldRight = smNULL;                       //idle mode
            }
            //region->rect = smViewRectangle;                 //restore origional rectangle
            piePointModePause(FALSE);
            mouseClipToRect(NULL);
            break;
        case RPE_WheelUp:
            wheel_up = TRUE;
            break;
        case RPE_WheelDown:
            wheel_down = TRUE;
            break;
        case RPE_KeyDown:
            if (smZoomingOut)
            {                                               //if sensors manager is just starting
                break;                                      //don't process key clicks
            }
            switch (ID)
            {
                case SHIFTKEY:
                    if (smHoldRight != mrCameraMotion)
                    {
                        piePointModeToggle(TRUE);
                    }
                    break;
                case MMOUSE_BUTTON:
                case FKEY:
                    if (!smFleetIntel)
                    {
                        selSelectionCopy((MaxAnySelection *)&selSelecting,(MaxAnySelection *)&selSelected);
                        smCullAndFocusSelecting();
                    }
                    break;
                case ZEROKEY:
                case ONEKEY:
                case TWOKEY:
                case THREEKEY:
                case FOURKEY:
                case FIVEKEY:
                case SIXKEY:
                case SEVENKEY:
                case EIGHTKEY:
                case NINEKEY:
                    if (!smFleetIntel)
                    {
                        if (keyIsHit(ALTKEY))
                        {                                   //alt-# select and focus on a hot key group
altCase:
                            if (selHotKeyGroup[ID - ZEROKEY].numShips != 0)
                            {
                                selSelectionCopy((MaxAnySelection *)&selSelected,(MaxAnySelection *)&selHotKeyGroup[ID - ZEROKEY]);
                                selSelectionCopy((MaxAnySelection *)&selSelecting,(MaxAnySelection *)&selSelected);
                                if(MP_HyperSpaceFlag)
                                {
                                    makeSelectionHyperspaceCapable((SelectCommand *)&selSelected);
                                }
                                smCullAndFocusSelecting();
#if SM_VERBOSE_LEVEL >= 2
                                dbgMessagef("\nHot key group %d selected and focused upon.", ID - ZEROKEY);
#endif
                            }
                        }
                        else if (ID == mrLastKeyPressed && universe.totaltimeelapsed <= mrLastKeyTime + mrNumberDoublePressTime)
                        {                                   //double-#: focus on hot-key group
                            tutGameMessage("KB_GroupSelectFocus");
                            goto altCase;                   //same as alt-#
                        }
                        else
                        {                                   //plain# select a hot key group
                            if (selHotKeyGroup[ID - ZEROKEY].numShips != 0)
                            {
                                tutGameMessage("KB_GroupSelect");

                                selSelectHotKeyGroup(&selHotKeyGroup[ID - ZEROKEY]);
                                selHotKeyNumbersSet(ID - ZEROKEY);
    #if SEL_ERROR_CHECKING
                                selHotKeyGroupsVerify();
    #endif
                                if(MP_HyperSpaceFlag)
                                {
                                    makeSelectionHyperspaceCapable((SelectCommand *)&selSelected);
                                }

                                ioUpdateShipTotals();
                                if (selSelected.numShips > 0)
                                {
                                    soundEvent(NULL, UI_Click);
                                    speechEvent(selHotKeyGroup[ID - ZEROKEY].ShipPtr[0], COMM_AssGrp_Select, ID - ZEROKEY);
                                }
                            }
                        }
                    }
                    break;
            }
            mrLastKeyPressed = ID;
            mrLastKeyTime = universe.totaltimeelapsed;
            break;
        case RPE_KeyUp:
            if (smZoomingOut)
            {                                               //if sensors manager is just starting
                break;                                      //don't process key clicks
            }
            switch (ID)
            {
                case SHIFTKEY:
                    if (smHoldRight != mrCameraMotion)
                    {
                        piePointModeToggle(FALSE);
                    }
                    break;
            }
            break;
#if SM_VERBOSE_LEVEL >= 1
        default:
            dbgMessagef("\nsmViewportProcess: unimplemented or unsupported event 0x%x. ID 0x%x, data 0x%x", event, ID, data);
#endif
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : various
    Description : Processor callbacks for FE buttons
    Inputs      : standard FE callback parameters
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smDispatch(char *name, featom *atom)
{
#if SM_VERBOSE_LEVEL >= 1
    dbgMessage("\nsmDispatch");
#endif
    if (smHoldRight == smNULL)
    {                                                       //cannot bring up MM with RMB held down
        if (!smFleetIntel && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bMove))
        {
            if(mrNeedProbeHack())
            {
                mrProbeHack();
            }
            else
            {
                mrRemoveAllProbesFromSelection();
                if(MP_HyperSpaceFlag)
                {
                    MP_HyperSpaceFlag = FALSE;
                }
                else
                {
                    if (piePointSpecMode == PSM_Idle)
                    {                                       //if bringing up the MM
                        makeShipsControllable((SelectCommand *)&selSelected,COMMAND_MP_HYPERSPACEING);
                        makeShipsNotIncludeSinglePlayerMotherships((SelectCommand *)&selSelected);
                        if(selSelected.numShips > 0)
                        {                                   //if there are ships we can control
                            piePointModeOnOff();
                        }
                    }
                    else
                    {                                       //always allow the MM to be turned off
                        piePointModeOnOff();
                    }
                }
            }
        }
    }
    feToggleButtonSet(name, piePointSpecMode != PSM_Idle);
}

void smUpdateHyperspaceStatus(bool goForLaunch)
{
    mrUpdateHyperspaceStatus(goForLaunch);

    if (smHyperspaceRegion == NULL)
    {
        return;
    }

    if (goForLaunch)
    {
        bitClear(smHyperspaceRegion->status, RSF_RegionDisabled);
    }
    else
    {
        bitSet(smHyperspaceRegion->status, RSF_RegionDisabled);
    }
}

void smHyperspace(char *name, featom *atom)
{
    regionhandle region = (regionhandle)atom->region;

#if SM_VERBOSE_LEVEL >= 1
    dbgMessage("\nsmHyperspace");
#endif

    if (FEFIRSTCALL(atom))
    {
        smHyperspaceRegion = region;

        if (singlePlayerGame)
        {
            if (singlePlayerGameInfo.playerCanHyperspace)
            {
                bitClear(region->status, RSF_RegionDisabled);
            }
            else
            {
                bitSet(region->status, RSF_RegionDisabled);
            }
        }
        else
        {
            //check research capabilities...
            //if not researched...black out button!

            if(!bitTest(tpGameCreated.flag,MG_Hyperspace))
            {
                bitSet(region->status, RSF_RegionDisabled);
            }
            //multiplayer Hyperspace initial Call!

        }
    }
    else if (FELASTCALL(atom))
    {
        smHyperspaceRegion = NULL;
        //check if movement plate for HYPERSPACE is up...
        //close it...
        if(!singlePlayerGame)
        {
            if(!bitTest(tpGameCreated.flag,MG_Hyperspace))
            {
                return;
            }
        }
        if (piePointSpecMode!=PSM_Idle
            && MP_HyperSpaceFlag)
        {
            soundEvent(NULL, UI_MovementGUIoff);
            MP_HyperSpaceFlag = FALSE;
            piePointModeOnOff();
        }
    }
    else
    {
        if(!singlePlayerGame)
        {
            if(!bitTest(tpGameCreated.flag,MG_Hyperspace))
            {
                return;
            }
        }
        if (singlePlayerGame && singlePlayerGameInfo.playerCanHyperspace)
        {
            spHyperspaceButtonPushed();
        }
        else
        {
            if (!smFleetIntel)
            {
                if(MP_HyperSpaceFlag)
                {
                    MP_HyperSpaceFlag = FALSE;
                    piePointModeOnOff();
                    soundEvent(NULL, UI_MovementGUIoff);
                }
                else
                {
                    makeShipsControllable((SelectCommand *)&selSelected,COMMAND_MP_HYPERSPACEING);
                    makeSelectionHyperspaceCapable((SelectCommand *)&selSelected);
                    if(selSelected.numShips == 0)
                    {
                        //removed all ships!
                        //if normal gui up..bring it down!
                        if (piePointSpecMode != PSM_Idle)
                        {
                            //turn it off!
                            soundEvent(NULL, UI_MovementGUIoff);
                            MP_HyperSpaceFlag = FALSE;
                            piePointModeOnOff();
                        }
                    }
                    else
                    {
                        MP_HyperSpaceFlag = TRUE;
                        if (piePointSpecMode == PSM_Idle)
                        {
                            //plate isn't up...turn it on!
                            piePointModeOnOff();
                        }
                    }
                }
            }
        }
    }
}

void smTacticalOverlayToggle(char *name, featom *atom)
{
    smTacticalOverlay ^= TRUE;
#if SM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nsmTacticalOverlayToggle: %s", smTacticalOverlay ? "ON" : "OFF");
#endif
}

void smNonResourceToggle(char *name, featom *atom)
{
    smNonResources ^= TRUE;
#if SM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nsmNonResourceToggle: %s", smNonResources ? "ON" : "OFF");
#endif
}

void smResourceToggle(char *name, featom *atom)
{
    smResources ^= TRUE;
#if SM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nsmResourceToggle: %s", smResources ? "ON" : "OFF");
#endif
}

void smSensorsClose(char *name, featom *atom)
{
    vector direction,dist;
    real32 dStart;

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bSensorsClose)
    {
        return;
    }

//    dbgAssert(!smFleetIntel);
    if (smZoomingIn || smZoomingOut)
    {                                                       //if hit while already zooming
        return;
    }
#if SM_VERBOSE_LEVEL >= 1
    dbgMessage("\nsmSensorsClose");
#endif


    if(smFocusOnMothershipOnClose)
    {
        ccFocusOnMyMothership(&universe.mainCameraCommand);
        smFocusOnMothershipOnClose = FALSE;
    }

    //!!! Jason: set the value of smLookStart to the centre of the focussed ships.
    ccLockOnTargetNow(&universe.mainCameraCommand);

    smLookStart = universe.mainCameraCommand.actualcamera.lookatpoint;
    smEyeStart = universe.mainCameraCommand.actualcamera.eyeposition;

    vecSub(direction, smEyeStart, smLookStart);
    dStart = fmathSqrt(vecMagnitudeSquared(direction));
    smEyeEnd = smCamera.eyeposition;
    smLookEnd = smCamera.lookatpoint;
    vecSub(direction, smLookEnd, smEyeEnd);
    vecNormalize(&direction);
    vecMultiplyByScalar(direction, dStart);
    vecSub(smEyeStart, smLookStart, direction);
    dbgAssert(!smZoomingOut);
    smZoomingIn = TRUE;

    /* call the sound event for closing the Sensors manager */
    soundEvent(NULL, UI_SensorsExit);

    smFocusTransition = FALSE;
    smInitialDistance = smCamera.distance;
    smFocus = FALSE;

    //now create a render list as it will be when fully zoomed in
    smTempCamera.lookatpoint = smLookStart;
    smTempCamera.eyeposition = smEyeStart;
    mrCamera = &smTempCamera;
    univUpdateRenderList();
    dbgAssert(smEyeStart.x < (real32)1e19 && smEyeStart.x > (real32)-1e19);

//Probe Hack Start   ******************
    if(mrNeedProbeHack())
    {
        if(piePointSpecMode != PSM_Idle)
        {   //turn off pie plate movement mech.
            MP_HyperSpaceFlag = FALSE;
            piePointModeOnOff();
        }
    }
    // check to see if pieplate active if it is and out of range turn it off.
    if (piePointSpecMode!=PSM_Idle)
    {
        selCentrePointCompute();
        vecSub(dist,selCentrePoint,universe.mainCameraCommand.actualcamera.lookatpoint);
        if (vecMagnitudeSquared(dist) >= MAX_MOVE_DISTANCE)
        {
            MP_HyperSpaceFlag = FALSE;
            piePointModeOnOff();
        }
    }

//Probe Hack END     ******************

    MP_HyperSpaceFlag = FALSE;
    bitClear(tbDisable,TBDISABLE_SENSORS_USE);
}

void smSensorsSkip(char *name, featom *atom)
{
    subMessageEnded = 2;                                 //the fleet intel thing was skipped
    speechEventActorStop(ACTOR_ALL_ACTORS, smSkipFadeoutTime);
    subTitlesFadeOut(&subRegion[STR_LetterboxBar], smSkipFadeoutTime);
}

void smCancelDispatch(char *name, featom *atom)
{
#if SM_VERBOSE_LEVEL >= 1
    dbgMessage("\nsmCancelDispatch");
#endif
    feToggleButtonSet(SM_TacticalOverlay, FALSE);
    piePointSpecMode = SPM_Idle;
}

#if SM_TOGGLE_SENSORLEVEL
void smToggleSensorsLevel(char *name, featom *atom)
{
    universe.curPlayerPtr->sensorLevel++;
    if (universe.curPlayerPtr->sensorLevel > 2)
    {
        universe.curPlayerPtr->sensorLevel = 0;
    }
#if SM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nsmToggleSensorsLevel: level set to %d", universe.curPlayerPtr->sensorLevel);
#endif
}
#endif //SM_TOGGLE_SENSORLEVEL

void smCancelMoveOrClose(char *name, featom *atom)
{
    MP_HyperSpaceFlag = FALSE;          //no matter what, set to FALSE
    if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
        soundEvent(NULL, UI_MovementGUIoff);
    }
    else
    {
        smSensorsClose(SM_Close, atom);
    }
}
/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : smStartup
    Description : Start the sensors manager module.
    Inputs      : void
    Outputs     : adds some callbacks, loads some stuff etc, etc...
    Return      : void
----------------------------------------------------------------------------*/
fecallback smCallbacks[] =
{
    {smDispatch, SM_Dispatch},
    {smHyperspace, SM_Hyperspace},
    {smTacticalOverlayToggle, SM_TacticalOverlay},
    {smNonResourceToggle, SM_NonResource},
    {smResourceToggle, SM_Resource},
    {smSensorsClose, SM_Close},
    {smSensorsSkip, SM_Skip},
    {smCancelDispatch, SM_CancelDispatch},
    {smPan, SM_Pan},
    {smCancelMoveOrClose, SM_CancelMoveOrClose},
#if SM_TOGGLE_SENSORLEVEL
    {smToggleSensorsLevel, SM_ToggleSensorsLevel},
#endif
    {NULL, NULL},
};
void smStartup(void)
{
    scriptSet(NULL, "sensors.script", smTweaks);
    feDrawCallbackAdd(SM_ViewportName, smViewportRender);   //add render callback
    feCallbackAddMultiple(smCallbacks);

    cameraInit(&smCamera, SM_InitialCameraDist);
    smUpdateParameters();

    smHoldRight = smNULL;
    smHoldLeft = smNULL;
    MP_HyperSpaceFlag = FALSE;      //set to FALSE...should manage itself from here on...

//    listInit(&smBlobList);
}

/*-----------------------------------------------------------------------------
    Name        : smUpdateParameters
    Description : Update the sensors manager parameters after a level has
                    loaded some sensors parameters.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smUpdateParameters(void)
{
    smCamera.clipPlaneNear = smZoomMin / smZoomMinFactor;
    smCamera.clipPlaneFar = smZoomMax * smZoomMaxFactor;
    smCamera.closestZoom = smZoomMin;
    smCamera.farthestZoom = smZoomMax;
    smCamera.distance = smInitialDistance;
    smCircleBorder = SM_CircleBorder;                       //ignore the value set in the level file
}

/*-----------------------------------------------------------------------------
    Name        : smShutdown
    Description : Shut down the sensors manager module.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smShutdown(void)
{
}

/*-----------------------------------------------------------------------------
    Name        : smSensorsBegin
    Description : Starts the sensors manager
    Inputs      : name, atom - ignored
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smSensorsBegin(char *name, featom *atom)
{
    regionhandle baseRegion, reg;
    sdword index, diff;
    fescreen *screen;
    featom *pAtom;
    sdword smViewExpandLeft;
    sdword smViewExpandRight;
    sdword smViewExpandTop;
    sdword smViewExpandBottom;
    char *screenName;

    bitClear(ghMainRegion->status, RSF_MouseInside);
    mrHoldLeft = mrHoldRight = mrNULL;                      //prevent a wierd bug

    if(smSensorsDisable || ((tutorial==TUTORIAL_ONLY) && !tutEnable.bSensorsManager))
    {
        return;
    }

    ioSaveState = ioDisable();

    tbSensorsHook();

    screenName = smFleetIntel ? SM_FleetIntelScreenName : SM_ScreenName;
    screen = feScreenFind(screenName);
    smBaseRegion = baseRegion = feScreenStart(ghMainRegion, screenName);
    //now find the user region the sensors manager is rendered in
    reg = baseRegion->child;
    smViewportRegion = NULL;
    for (index = 1; index < screen->nAtoms; index++)
    {
//        reg = (regionhandle)screen->atoms[index].region;
        regVerify(reg);
        if (((featom *)reg->userID)->pData == (ubyte *)smViewportRender) //if this is a user region
        {
            smBackgroundColor = ((featom *)reg->userID)->contentColor;
//            glClearColor(colUbyteToReal(colRed(smBackgroundColor)),
//                         colUbyteToReal(colGreen(smBackgroundColor)),
//                         colUbyteToReal(colBlue(smBackgroundColor)), 1.0f);

            smViewportRegion = reg;                         //this must be the region
            break;
        }
        reg = reg->next;
    }
    dbgAssert(smViewportRegion != NULL);
    regFunctionSet(smViewportRegion, smViewportProcess);
    regFilterSet(smViewportRegion, SM_ViewportFilter);
    smViewRectangle = smViewportRegion->rect;

    /*
    index = 640 - (smViewRectangle.x1 - smViewRectangle.x0);
    index >>= 1;
    index++;
    smViewRectangle.x0 = index;
    smViewRectangle.x1 = MAIN_WindowWidth - index;
    */
    //now let's go through all the regions and build lists of regions outside the
    //main viewport.  These regions are to scroll onto the screen during the zoom.
    reg = baseRegion->child;
    smScrollListTop = smScrollListLeft = smScrollListRight = smScrollListBottom = NULL;
    smScrollCountLeft = smScrollCountTop = smScrollCountRight = smScrollCountBottom = 0;
    smScrollDistLeft = smScrollDistTop = smScrollDistRight = smScrollDistBottom = 0;
    smScrollLeft = smScrollTop = smScrollRight = smScrollBottom = 0;
    for (index = 1; index < screen->nAtoms; index++, reg = reg->next)
    {
//        reg = (regionhandle)screen->atoms[index].region;
        regVerify(reg);
        pAtom = (featom *)reg->userID;
        if (reg->rect.x1 <= smViewportRegion->rect.x0)
        {                                                   //if off left
            smScrollCountLeft++;
            smScrollListLeft = memRealloc(smScrollListLeft, smScrollCountLeft * sizeof(region**), "ScrollListLeft", 0);
            smScrollListLeft[smScrollCountLeft - 1] = reg;
            continue;
        }
        if (reg->rect.y1 <= smViewportRegion->rect.y0)
        {                                                   //if off top
            smScrollCountTop++;
            smScrollListTop = memRealloc(smScrollListTop, smScrollCountTop * sizeof(region**), "ScrollListTop", 0);
            smScrollListTop[smScrollCountTop - 1] = reg;
            continue;
        }
        if (reg->rect.x0 >= smViewportRegion->rect.x1)
        {                                                   //if off right
            smScrollCountRight++;
            smScrollListRight = memRealloc(smScrollListRight, smScrollCountRight * sizeof(region**), "ScrollListRight", 0);
            smScrollListRight[smScrollCountRight - 1] = reg;
            continue;
        }
        if (reg->rect.y0 >= smViewportRegion->rect.y1)
        {                                                   //if off bottom
            smScrollCountBottom++;
            smScrollListBottom = memRealloc(smScrollListBottom, smScrollCountBottom * sizeof(region**), "ScrollListBottom", 0);
            smScrollListBottom[smScrollCountBottom - 1] = reg;
            continue;
        }
    }
    //compute pan distance to get these rectangles to work in high-rez
    smViewExpandLeft    = (MAIN_WindowWidth - 640) / 2;
    smViewExpandRight   = (MAIN_WindowWidth - 640) - smViewExpandLeft;
    smViewExpandTop     = (MAIN_WindowHeight - 480) / 2;
    smViewExpandBottom  = (MAIN_WindowHeight - 480) - smViewExpandTop;
    //let's figure out how far we'll have to scroll each of these lists
    for (index = 0; index < smScrollCountLeft; index++)
    {
        smScrollDistLeft = max(smScrollDistLeft, smScrollListLeft[index]->rect.x1 - smScrollListLeft[index]->rect.x0);
    }
    for (index = 0; index < smScrollCountTop; index++)
    {
        if (smFleetIntel && index == smScrollCountTop - 1)
        {                                                   //consider the last region as a letterbox bar
            dbgAssert(smScrollListTop[index]->drawFunction == feStaticRectangleDraw);
            diff = smScrollListTop[index]->rect.y1 - smScrollListTop[index]->rect.y0;
            diff = (diff) * MAIN_WindowHeight / 480 - diff;
            smScrollListTop[index]->rect.y1 += diff;
            smViewportRegion->rect.y0 += diff;
            smViewRectangle.y0 += diff;
        }
        smScrollDistTop = max(smScrollDistTop, smScrollListTop[index]->rect.y1 - smScrollListTop[index]->rect.y0);
    }
    for (index = 0; index < smScrollCountRight; index++)
    {
        smScrollDistRight = max(smScrollDistRight, smScrollListRight[index]->rect.x1 - smScrollListRight[index]->rect.x0);
    }
    for (index = 0; index < smScrollCountBottom; index++)
    {
        if (smFleetIntel && index == smScrollCountBottom - 1)
        {                                                   //consider the last region as a letterbox bar
            dbgAssert(smScrollListBottom[index]->drawFunction == feStaticRectangleDraw);
            diff = smScrollListBottom[index]->rect.y1 - smScrollListBottom[index]->rect.y0;
            diff = (diff) * MAIN_WindowHeight / 480 - diff;
            smScrollListBottom[index]->rect.y0 -= diff;
            smViewportRegion->rect.y1 -= diff;
            smViewRectangle.y1 -= diff;
        }
        smScrollDistBottom = max(smScrollDistBottom, smScrollListBottom[index]->rect.y1 - smScrollListBottom[index]->rect.y0);
    }
    //unt finally, let's scroll them far enough to be off-screen
    for (index = 0; index < smScrollCountLeft; index++)
    {
        if (smScrollListLeft[index]->rect.y0 <= smViewRectangle.y0)
        {
            smScrollListLeft[index]->rect.y0 -= smViewExpandTop;
        }
        if (smScrollListLeft[index]->rect.y1 >= smViewRectangle.y1)
        {
            smScrollListLeft[index]->rect.y1 += smViewExpandBottom;
        }
        regRegionScroll(smScrollListLeft[index], -smScrollDistLeft - smViewExpandLeft, 0);
    }
    for (index = 0; index < smScrollCountTop; index++)
    {
        if (smScrollListTop[index]->rect.x0 <= smViewRectangle.x0)
        {
            smScrollListTop[index]->rect.x0 -= smViewExpandLeft;
        }
        if (smScrollListTop[index]->rect.x1 >= smViewRectangle.x1)
        {
            smScrollListTop[index]->rect.x1 += smViewExpandRight;
        }
        regRegionScroll(smScrollListTop[index], 0, -smScrollDistTop - smViewExpandTop);
    }
    for (index = 0; index < smScrollCountRight; index++)
    {
        if (smScrollListRight[index]->rect.y0 <= smViewRectangle.y0)
        {
            smScrollListRight[index]->rect.y0 -= smViewExpandTop;
        }
        if (smScrollListRight[index]->rect.y1 >= smViewRectangle.y1)
        {
            smScrollListRight[index]->rect.y1 += smViewExpandBottom;
        }
        regRegionScroll(smScrollListRight[index], smScrollDistRight + smViewExpandRight, 0);
    }
    for (index = 0; index < smScrollCountBottom; index++)
    {
        if (smScrollListBottom[index]->rect.x0 <= smViewRectangle.x0)
        {
            smScrollListBottom[index]->rect.x0 -= smViewExpandLeft;
        }
        if (smScrollListBottom[index]->rect.x1 >= smViewRectangle.x1)
        {
            smScrollListBottom[index]->rect.x1 += smViewExpandRight;
        }
        regRegionScroll(smScrollListBottom[index], 0, smScrollDistBottom + smViewExpandBottom);
    }
    //set the view rectangle to full screen size, to be scaled as we go zoom back
    smViewRectangle.x0 -= smScrollDistLeft + smViewExpandLeft;
    smViewRectangle.y0 -= smScrollDistTop + smViewExpandTop;
    smViewRectangle.x1 += smScrollDistRight + smViewExpandRight;
    smViewRectangle.y1 += smScrollDistBottom + smViewExpandBottom;
    smViewportRegion->rect = smViewRectangle;
    smZoomTime = 0.0f;
    if (smInstantTransition && !smFleetIntel)
    {
        smCurrentZoomLength = 0.1f;
        smCurrentMainViewZoomLength = 0.05f;
    }
    else
    {
        smCurrentZoomLength = smZoomLength;
        smCurrentMainViewZoomLength = smMainViewZoomLength;
    }

    //figure out the start/end points for the camera animation
    smLookStart = universe.mainCameraCommand.actualcamera.lookatpoint;
    smEyeStart = universe.mainCameraCommand.actualcamera.eyeposition;
    //smLookEnd.x = smLookEnd.y = smLookEnd.z = 0.0f;
    smLookEnd = universe.mainCameraCommand.actualcamera.lookatpoint;
    smCameraLookatPoint = smLookEnd;
    smCameraLookVelocity.x = smCameraLookVelocity.y = smCameraLookVelocity.z = 0.0f;
    vecSub(smEyeEnd, smEyeStart, smLookStart);
    vecNormalize(&smEyeEnd);
    vecMultiplyByScalar(smEyeEnd, smInitialDistance);
    vecAddTo(smEyeEnd, smLookEnd);
    smZoomingIn = FALSE;
    smZoomingOut = TRUE;

//    mrRenderMainScreen = FALSE;
    feToggleButtonSet(SM_TacticalOverlay, smTacticalOverlay);
    feToggleButtonSet(SM_Resource       , smResources);
    feToggleButtonSet(SM_NonResource    , smNonResources);
    feToggleButtonSet(SM_Dispatch       , piePointSpecMode != PSM_Idle);

    mouseClipToRect(NULL);
    smHoldLeft = smNULL;
    smHoldRight = smNULL;
//    bobListCreate(&smBlobProperties, &smBlobList, universe.curPlayerIndex);

    smSensorsActive = TRUE;

    soundEventStopSFX(0.5f);

    /* call the sound event for opening the Sensors manager */
    soundEvent(NULL, UI_SensorsIntro);

    universe.dontUpdateRenderList = TRUE;
    if (RGL)
        rglFeature(RGL_SANSDEPTH);
    smRenderCount = 0;

    //add any additional key messages the sensors manager will need
    regKeyChildAlloc(smViewportRegion, SHIFTKEY, RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, SHIFTKEY);
    regKeyChildAlloc(smViewportRegion, MKEY, RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, MKEY);
    regKeyChildAlloc(smViewportRegion, FKEY, RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, FKEY);
    regKeyChildAlloc(smViewportRegion, MMOUSE_BUTTON, RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, MMOUSE_BUTTON);

    regKeyChildAlloc(smViewportRegion, ARRLEFT , RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, ARRLEFT );
    regKeyChildAlloc(smViewportRegion, ARRRIGHT, RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, ARRRIGHT);
    regKeyChildAlloc(smViewportRegion, ARRUP   , RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, ARRUP   );
    regKeyChildAlloc(smViewportRegion, ARRDOWN , RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, ARRDOWN );

    for (index = ZEROKEY; index <= NINEKEY; index++)
    {
        regKeyChildAlloc(smViewportRegion, index, RPE_KeyUp | RPE_KeyDown, smViewportProcess, 1, index);
    }
    mouseCursorShow();
    cameraCopyPositionInfo(&smCamera, mrCamera);
    if (smFleetIntel)
    {                                                       //if we are in the fleet intel screen, no movement mechanism!
        piePointSpecMode = SPM_Idle;
    }

    bitSet(tbDisable,TBDISABLE_SENSORS_USE);
}

/*-----------------------------------------------------------------------------
    Name        : smObjectDied
    Description : Called when an object dies, this function removes it from the
                    sensors manager blob in which it exists.
    Inputs      : object - object to delete
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void smObjectDied(void *object)
{
    sdword index;

//    bobObjectDied(object,&smBlobList);

    //remove it from the focus command, if there is one.
    if (smFocusCommand)
    {
        for (index = 0; index < smFocusCommand->numShips; index++)
        {
            if ((SpaceObj *)smFocusCommand->ShipPtr[index] == object)
            {
                for (; index < smFocusCommand->numShips - 1; index++)
                {
                    smFocusCommand->ShipPtr[index] = smFocusCommand->ShipPtr[index + 1];
                }
                smFocusCommand->numShips--;
                if (smFocusCommand->numShips <= 0)
                {
                    memFree(smFocusCommand);
                    smFocusCommand = NULL;
                    smFocus = FALSE;
                }
                return;
            }
        }
    }
}

/*=============================================================================
    Save Game Stuff:
=============================================================================*/

void smSave(void)
{
    SaveInfoNumber(TreatAsUdword(smDepthCueRadius));
    SaveInfoNumber(TreatAsUdword(smDepthCueStartRadius));
    SaveInfoNumber(TreatAsUdword(smCircleBorder));
    SaveInfoNumber(TreatAsUdword(smZoomMax));
    SaveInfoNumber(TreatAsUdword(smZoomMin));
    SaveInfoNumber(TreatAsUdword(smZoomMinFactor));
    SaveInfoNumber(TreatAsUdword(smZoomMaxFactor));
    SaveInfoNumber(TreatAsUdword(smInitialDistance));
    SaveInfoNumber(TreatAsUdword(smUniverseSizeX));
    SaveInfoNumber(TreatAsUdword(smUniverseSizeY));
    SaveInfoNumber(TreatAsUdword(smUniverseSizeZ));

    SaveInfoNumber(smSensorWeirdness);
}

void smLoad(void)
{
    sdword loadnum;

    loadnum = LoadInfoNumber(); smDepthCueRadius = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smDepthCueStartRadius = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smCircleBorder = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smZoomMax = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smZoomMin = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smZoomMinFactor = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smZoomMaxFactor = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smInitialDistance = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smUniverseSizeX = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smUniverseSizeY = TreatAsReal32(loadnum);
    loadnum = LoadInfoNumber(); smUniverseSizeZ = TreatAsReal32(loadnum);

    smSensorWeirdness = LoadInfoNumber();

    smUpdateParameters();
}

