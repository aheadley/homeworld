/*=============================================================================
    Name    : ShipView.c
    Purpose : Render a specific ship to a window.

    Created 7/27/1998 by pgrant
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include "Types.h"
#include "ObjTypes.h"
#include "Strings.h"
#include "ShipView.h"
#include "Camera.h"
#include "FEFlow.h"
#include "FEReg.h"
#include "glinc.h"
#include "glcaps.h"
#include "render.h"
#include "Universe.h"
#include "light.h"
#include "mouse.h"
#include "font.h"
#include "FontReg.h"
#include "SoundEvent.h"
#include "ConsMgr.h"
#include "Options.h"
#include "FEColour.h"
#include "Gun.h"
#include "glcompat.h"
#include "Probe.h"

/*=============================================================================
    Private Types:
=============================================================================*/

#define SV_ShipViewName             "SV_ShipView"

#define SV_FirepowerName            "SV_Firepower"
#define SV_CoverageName             "SV_Coverage"
#define SV_ManeuverName             "SV_Maneuver"
#define SV_ArmorName                "SV_Armor"
#define SV_TopSpeedName             "SV_TopSpeed"
#define SV_MassName                 "SV_Mass"

#define SV_InitialCameraDistance    900.0f

#define SV_FontNameLength           64
#define SV_DefaultFont              "default.hff"
#define SV_StatFont                 "HW_EuroseCond_11.hff" //"Small_Fonts_8.hff"

#define SV_ViewMargin               8

regionhandle svShipViewRegion  = NULL;
regionhandle svFirepowerRegion = NULL;
regionhandle svCoverageRegion  = NULL;
regionhandle svManeuverRegion  = NULL;
regionhandle svArmorRegion     = NULL;
regionhandle svTopSpeedRegion  = NULL;
regionhandle svMassRegion      = NULL;

fonthandle svShipViewFont = 0;
fonthandle svShipStatFont = 0;

bool ReZoom=TRUE;

real32 savecamMouseX = 0.0f;

char svShipViewFontName[SV_FontNameLength] = SV_DefaultFont;
char svShipStatFontName[SV_FontNameLength] = SV_StatFont;


// storage for printing numbers and some strings relating to Ship statistics
char buf[40];

/*=============================================================================
    Data:
=============================================================================*/

/*
real32 svScaleAdvanceSupportFrigate   =  3.0;
real32 svScaleAttackBomber            = 10.0;
real32 svScaleCarrier                 =  0.5;
real32 svScaleCloakedFighter          = 10.0;
real32 svScaleCloakGenerator          =  4.0;
real32 svScaleDDDFrigate              =  2.0;
real32 svScaleDefenseFighter          =  6.0;
real32 svScaleDFGFrigate              =  2.0;
real32 svScaleGravWellGenerator       =  2.0;
real32 svScaleHeavyCorvette           =  9.0;
real32 svScaleHeavyCruiser            =  0.5;
real32 svScaleHeavyDefender           = 10.0;
real32 svScaleHeavyInterceptor        = 10.0;
real32 svScaleIonCannonFrigate        =  2.5;
real32 svScaleLightCorvette           =  9.0;
real32 svScaleLightDefender           = 10.0;
real32 svScaleLightInterceptor        = 10.0;
real32 svScaleMinelayerCorvette       =  6.0;
real32 svScaleMissileDestroyer        =  1.0;
real32 svScaleMothership              =  4.0; //not used but here because of enumeration dependancy
real32 svScaleMultiGunCorvette        =  8.0;
real32 svScaleProbe                   =  6.0;
real32 svScaleProximitySensor         =  8.0;
real32 svScaleRepairCorvette          =  8.0;
real32 svScaleResearchShip            =  3.0;
real32 svScaleResourceCollector       =  3.0;
real32 svScaleResourceController      =  1.0;
real32 svScaleSalCapCorvette          =  8.0;
real32 svScaleSensorArray             =  1.0;
real32 svScaleStandardDestroyer       =  1.0;
real32 svScaleStandardFrigate         =  3.0;

real32 svMinZoomAdvanceSupportFrigate = 500.0;
real32 svMinZoomAttackBomber          = 500.0;
real32 svMinZoomCarrier               = 500.0;
real32 svMinZoomCloakedFighter        = 500.0;
real32 svMinZoomCloakGenerator        = 500.0;
real32 svMinZoomDDDFrigate            = 500.0;
real32 svMinZoomDefenseFighter        = 500.0;
real32 svMinZoomDFGFrigate            = 500.0;
real32 svMinZoomGravWellGenerator     = 500.0;
real32 svMinZoomHeavyCorvette         = 500.0;
real32 svMinZoomHeavyCruiser          = 500.0;
real32 svMinZoomHeavyDefender         = 500.0;
real32 svMinZoomHeavyInterceptor      = 500.0;
real32 svMinZoomIonCannonFrigate      = 500.0;
real32 svMinZoomLightCorvette         = 500.0;
real32 svMinZoomLightDefender         = 500.0;
real32 svMinZoomLightInterceptor      = 500.0;
real32 svMinZoomMinelayerCorvette     = 500.0;
real32 svMinZoomMissileDestroyer      = 500.0;
real32 svMinZoomMothership            = 500.0; //not used but here because of enumeration dependancy
real32 svMinZoomMultiGunCorvette      = 500.0;
real32 svMinZoomProbe                 = 500.0;
real32 svMinZoomProximitySensor       = 500.0;
real32 svMinZoomRepairCorvette        = 500.0;
real32 svMinZoomResearchShip          = 500.0;
real32 svMinZoomResourceCollector     = 500.0;
real32 svMinZoomResourceController    = 500.0;
real32 svMinZoomSalCapCorvette        = 500.0;
real32 svMinZoomSensorArray           = 500.0;
real32 svMinZoomStandardDestroyer     = 500.0;
real32 svMinZoomStandardFrigate       = 500.0;

real32 svMaxZoomAdvanceSupportFrigate = 1000.0;
real32 svMaxZoomAttackBomber          = 1000.0;
real32 svMaxZoomCarrier               = 1000.0;
real32 svMaxZoomCloakedFighter        = 1000.0;
real32 svMaxZoomCloakGenerator        = 1000.0;
real32 svMaxZoomDDDFrigate            = 1000.0;
real32 svMaxZoomDefenseFighter        = 1000.0;
real32 svMaxZoomDFGFrigate            = 1000.0;
real32 svMaxZoomGravWellGenerator     = 1000.0;
real32 svMaxZoomHeavyCorvette         = 1000.0;
real32 svMaxZoomHeavyCruiser          = 1000.0;
real32 svMaxZoomHeavyDefender         = 1000.0;
real32 svMaxZoomHeavyInterceptor      = 1000.0;
real32 svMaxZoomIonCannonFrigate      = 1000.0;
real32 svMaxZoomLightCorvette         = 1000.0;
real32 svMaxZoomLightDefender         = 1000.0;
real32 svMaxZoomLightInterceptor      = 1000.0;
real32 svMaxZoomMinelayerCorvette     = 1000.0;
real32 svMaxZoomMissileDestroyer      = 1000.0;
real32 svMaxZoomMothership            = 1000.0; //not used but here because of enumeration dependancy
real32 svMaxZoomMultiGunCorvette      = 1000.0;
real32 svMaxZoomProbe                 = 1000.0;
real32 svMaxZoomProximitySensor       = 1000.0;
real32 svMaxZoomRepairCorvette        = 1000.0;
real32 svMaxZoomResearchShip          = 1000.0;
real32 svMaxZoomResourceCollector     = 1000.0;
real32 svMaxZoomResourceController    = 1000.0;
real32 svMaxZoomSalCapCorvette        = 1000.0;
real32 svMaxZoomSensorArray           = 1000.0;
real32 svMaxZoomStandardDestroyer     = 1000.0;
real32 svMaxZoomStandardFrigate       = 1000.0;

*/

Camera svCamera;

real32 svAngle = -150.0;
real32 svDeclination   = -20.0;
real32 svZoomOutScalar=1.1f;
real32 svZoomInScalar=1.05f;

scriptEntry ShipViewTweaks[] =
{
    makeEntry(svAngle, scriptSetReal32CB),
    makeEntry(svDeclination, scriptSetReal32CB),
        makeEntry(svZoomInScalar, scriptSetReal32CB),
        makeEntry(svZoomOutScalar,scriptSetReal32CB),
    endEntry
};
/*

    makeEntry(svScaleAdvanceSupportFrigate, scriptSetReal32CB),
    makeEntry(svScaleAttackBomber, scriptSetReal32CB),
    makeEntry(svScaleCarrier, scriptSetReal32CB),
    makeEntry(svScaleCloakedFighter, scriptSetReal32CB),
    makeEntry(svScaleCloakGenerator, scriptSetReal32CB),
    makeEntry(svScaleDDDFrigate, scriptSetReal32CB),
    makeEntry(svScaleDefenseFighter, scriptSetReal32CB),
    makeEntry(svScaleDFGFrigate, scriptSetReal32CB),
    makeEntry(svScaleGravWellGenerator, scriptSetReal32CB),
    makeEntry(svScaleHeavyCorvette, scriptSetReal32CB),
    makeEntry(svScaleHeavyCruiser, scriptSetReal32CB),
    makeEntry(svScaleHeavyDefender, scriptSetReal32CB),
    makeEntry(svScaleHeavyInterceptor, scriptSetReal32CB),
    makeEntry(svScaleIonCannonFrigate, scriptSetReal32CB),
    makeEntry(svScaleLightCorvette, scriptSetReal32CB),
    makeEntry(svScaleLightDefender, scriptSetReal32CB),
    makeEntry(svScaleLightInterceptor, scriptSetReal32CB),
    makeEntry(svScaleMinelayerCorvette, scriptSetReal32CB),
    makeEntry(svScaleMissileDestroyer, scriptSetReal32CB),
    makeEntry(svScaleMothership, scriptSetReal32CB),
    makeEntry(svScaleMultiGunCorvette, scriptSetReal32CB),
    makeEntry(svScaleProbe, scriptSetReal32CB),
    makeEntry(svScaleProximitySensor, scriptSetReal32CB),
    makeEntry(svScaleRepairCorvette, scriptSetReal32CB),
    makeEntry(svScaleResearchShip, scriptSetReal32CB),
    makeEntry(svScaleResourceCollector, scriptSetReal32CB),
    makeEntry(svScaleResourceController, scriptSetReal32CB),
    makeEntry(svScaleSalCapCorvette, scriptSetReal32CB),
    makeEntry(svScaleSensorArray, scriptSetReal32CB),
    makeEntry(svScaleStandardDestroyer, scriptSetReal32CB),
    makeEntry(svScaleStandardFrigate, scriptSetReal32CB),

    makeEntry(svMinZoomAdvanceSupportFrigate, scriptSetReal32CB),
    makeEntry(svMinZoomAttackBomber, scriptSetReal32CB),
    makeEntry(svMinZoomCarrier, scriptSetReal32CB),
    makeEntry(svMinZoomCloakedFighter, scriptSetReal32CB),
    makeEntry(svMinZoomCloakGenerator, scriptSetReal32CB),
    makeEntry(svMinZoomDDDFrigate, scriptSetReal32CB),
    makeEntry(svMinZoomDefenseFighter, scriptSetReal32CB),
    makeEntry(svMinZoomDFGFrigate, scriptSetReal32CB),
    makeEntry(svMinZoomGravWellGenerator, scriptSetReal32CB),
    makeEntry(svMinZoomHeavyCorvette, scriptSetReal32CB),
    makeEntry(svMinZoomHeavyCruiser, scriptSetReal32CB),
    makeEntry(svMinZoomHeavyDefender, scriptSetReal32CB),
    makeEntry(svMinZoomHeavyInterceptor, scriptSetReal32CB),
    makeEntry(svMinZoomIonCannonFrigate, scriptSetReal32CB),
    makeEntry(svMinZoomLightCorvette, scriptSetReal32CB),
    makeEntry(svMinZoomLightDefender, scriptSetReal32CB),
    makeEntry(svMinZoomLightInterceptor, scriptSetReal32CB),
    makeEntry(svMinZoomMinelayerCorvette, scriptSetReal32CB),
    makeEntry(svMinZoomMissileDestroyer, scriptSetReal32CB),
    makeEntry(svMinZoomMothership, scriptSetReal32CB),
    makeEntry(svMinZoomMultiGunCorvette, scriptSetReal32CB),
    makeEntry(svMinZoomProbe, scriptSetReal32CB),
    makeEntry(svMinZoomProximitySensor, scriptSetReal32CB),
    makeEntry(svMinZoomRepairCorvette, scriptSetReal32CB),
    makeEntry(svMinZoomResearchShip, scriptSetReal32CB),
    makeEntry(svMinZoomResourceCollector, scriptSetReal32CB),
    makeEntry(svMinZoomResourceController, scriptSetReal32CB),
    makeEntry(svMinZoomSalCapCorvette, scriptSetReal32CB),
    makeEntry(svMinZoomSensorArray, scriptSetReal32CB),
    makeEntry(svMinZoomStandardDestroyer, scriptSetReal32CB),
    makeEntry(svMinZoomStandardFrigate, scriptSetReal32CB),

    makeEntry(svMaxZoomAdvanceSupportFrigate, scriptSetReal32CB),
    makeEntry(svMaxZoomAttackBomber, scriptSetReal32CB),
    makeEntry(svMaxZoomCarrier, scriptSetReal32CB),
    makeEntry(svMaxZoomCloakedFighter, scriptSetReal32CB),
    makeEntry(svMaxZoomCloakGenerator, scriptSetReal32CB),
    makeEntry(svMaxZoomDDDFrigate, scriptSetReal32CB),
    makeEntry(svMaxZoomDefenseFighter, scriptSetReal32CB),
    makeEntry(svMaxZoomDFGFrigate, scriptSetReal32CB),
    makeEntry(svMaxZoomGravWellGenerator, scriptSetReal32CB),
    makeEntry(svMaxZoomHeavyCorvette, scriptSetReal32CB),
    makeEntry(svMaxZoomHeavyCruiser, scriptSetReal32CB),
    makeEntry(svMaxZoomHeavyDefender, scriptSetReal32CB),
    makeEntry(svMaxZoomHeavyInterceptor, scriptSetReal32CB),
    makeEntry(svMaxZoomIonCannonFrigate, scriptSetReal32CB),
    makeEntry(svMaxZoomLightCorvette, scriptSetReal32CB),
    makeEntry(svMaxZoomLightDefender, scriptSetReal32CB),
    makeEntry(svMaxZoomLightInterceptor, scriptSetReal32CB),
    makeEntry(svMaxZoomMinelayerCorvette, scriptSetReal32CB),
    makeEntry(svMaxZoomMissileDestroyer, scriptSetReal32CB),
    makeEntry(svMaxZoomMothership, scriptSetReal32CB),
    makeEntry(svMaxZoomMultiGunCorvette, scriptSetReal32CB),
    makeEntry(svMaxZoomProbe, scriptSetReal32CB),
    makeEntry(svMaxZoomProximitySensor, scriptSetReal32CB),
    makeEntry(svMaxZoomRepairCorvette, scriptSetReal32CB),
    makeEntry(svMaxZoomResearchShip, scriptSetReal32CB),
    makeEntry(svMaxZoomResourceCollector, scriptSetReal32CB),
    makeEntry(svMaxZoomResourceController, scriptSetReal32CB),
    makeEntry(svMaxZoomSalCapCorvette, scriptSetReal32CB),
    makeEntry(svMaxZoomSensorArray, scriptSetReal32CB),
    makeEntry(svMaxZoomStandardDestroyer, scriptSetReal32CB),
    makeEntry(svMaxZoomStandardFrigate, scriptSetReal32CB),

    endEntry
};
*/
/*
real32* svScale[31] =
{
    &svScaleAdvanceSupportFrigate,
    &svScaleAttackBomber,
    &svScaleCarrier,
    &svScaleCloakedFighter,
    &svScaleCloakGenerator,
    &svScaleDDDFrigate,
    &svScaleDefenseFighter,
    &svScaleDFGFrigate,
    &svScaleGravWellGenerator,
    &svScaleHeavyCorvette,
    &svScaleHeavyCruiser,
    &svScaleHeavyDefender,
    &svScaleHeavyInterceptor,
    &svScaleIonCannonFrigate,
    &svScaleLightCorvette,
    &svScaleLightDefender,
    &svScaleLightInterceptor,
    &svScaleMinelayerCorvette,
    &svScaleMissileDestroyer,
    &svScaleMothership,
    &svScaleMultiGunCorvette,
    &svScaleProbe,
    &svScaleProximitySensor,
    &svScaleRepairCorvette,
    &svScaleResearchShip,
    &svScaleResourceCollector,
    &svScaleResourceController,
    &svScaleSalCapCorvette,
    &svScaleSensorArray,
    &svScaleStandardDestroyer,
    &svScaleStandardFrigate
};

real32* svMinZoom[31] =
{
    &svMinZoomAdvanceSupportFrigate,
    &svMinZoomAttackBomber,
    &svMinZoomCarrier,
    &svMinZoomCloakedFighter,
    &svMinZoomCloakGenerator,
    &svMinZoomDDDFrigate,
    &svMinZoomDefenseFighter,
    &svMinZoomDFGFrigate,
    &svMinZoomGravWellGenerator,
    &svMinZoomHeavyCorvette,
    &svMinZoomHeavyCruiser,
    &svMinZoomHeavyDefender,
    &svMinZoomHeavyInterceptor,
    &svMinZoomIonCannonFrigate,
    &svMinZoomLightCorvette,
    &svMinZoomLightDefender,
    &svMinZoomLightInterceptor,
    &svMinZoomMinelayerCorvette,
    &svMinZoomMissileDestroyer,
    &svMinZoomMothership,
    &svMinZoomMultiGunCorvette,
    &svMinZoomProbe,
    &svMinZoomProximitySensor,
    &svMinZoomRepairCorvette,
    &svMinZoomResearchShip,
    &svMinZoomResourceCollector,
    &svMinZoomResourceController,
    &svMinZoomSalCapCorvette,
    &svMinZoomSensorArray,
    &svMinZoomStandardDestroyer,
    &svMinZoomStandardFrigate
};

real32* svMaxZoom[31] =
{
    &svMaxZoomAdvanceSupportFrigate,
    &svMaxZoomAttackBomber,
    &svMaxZoomCarrier,
    &svMaxZoomCloakedFighter,
    &svMaxZoomCloakGenerator,
    &svMaxZoomDDDFrigate,
    &svMaxZoomDefenseFighter,
    &svMaxZoomDFGFrigate,
    &svMaxZoomGravWellGenerator,
    &svMaxZoomHeavyCorvette,
    &svMaxZoomHeavyCruiser,
    &svMaxZoomHeavyDefender,
    &svMaxZoomHeavyInterceptor,
    &svMaxZoomIonCannonFrigate,
    &svMaxZoomLightCorvette,
    &svMaxZoomLightDefender,
    &svMaxZoomLightInterceptor,
    &svMaxZoomMinelayerCorvette,
    &svMaxZoomMissileDestroyer,
    &svMaxZoomMothership,
    &svMaxZoomMultiGunCorvette,
    &svMaxZoomProbe,
    &svMaxZoomProximitySensor,
    &svMaxZoomRepairCorvette,
    &svMaxZoomResearchShip,
    &svMaxZoomResourceCollector,
    &svMaxZoomResourceController,
    &svMaxZoomSalCapCorvette,
    &svMaxZoomSensorArray,
    &svMaxZoomStandardDestroyer,
    &svMaxZoomStandardFrigate
};
*/
ShipType svShipType = DefaultShip;

sdword svMouseCentreX = 0;
sdword svMouseCentreY = 0;
sdword svMouseLastX   = 0;
sdword svMouseLastY   = 0;

bool8 svMouseInside     = FALSE;
bool8 svMousePressLeft  = FALSE;
bool8 svMousePressRight = FALSE;

static GLfloat lightPosition0[] = {10000.0f, 1.0f, 1.0f, 0.1f};

//for callback from glcPageFlip, if glcompat module is active
static featom* _glcAtom;
static regionhandle _glcRegion;
bool _svRender = FALSE;

/*=============================================================================
    Function Prototypes
=============================================================================*/

uword svShipCoverage(ShipStaticInfo *statinfo);
void svShipManeuverability(ShipStaticInfo *statinfo,char *name);

/*=============================================================================
    Functions:
=============================================================================*/



/*-----------------------------------------------------------------------------
    Name        : svDirtyShipView
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void svDirtyShipView(void)
{
    if (svShipViewRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(svShipViewRegion);
#endif
        svShipViewRegion->status |= RSF_DrawThisFrame;
    }
    if (svFirepowerRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(svFirepowerRegion);
#endif
        svFirepowerRegion->status |= RSF_DrawThisFrame;
    }
    if (svCoverageRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(svCoverageRegion);
#endif
        svCoverageRegion->status |= RSF_DrawThisFrame;
    }
    if (svManeuverRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(svManeuverRegion);
#endif
        svManeuverRegion->status |= RSF_DrawThisFrame;
    }
    if (svArmorRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(svArmorRegion);
#endif
        svArmorRegion->status |= RSF_DrawThisFrame;
    }
    if (svTopSpeedRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(svTopSpeedRegion);
#endif
        svTopSpeedRegion->status |= RSF_DrawThisFrame;
    }
    if (svMassRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(svMassRegion);
#endif
        svMassRegion->status |= RSF_DrawThisFrame;
    }
}

/*-----------------------------------------------------------------------------
    Name        : svSelectShip
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void svSelectShip(ShipType type)
{
    svShipType = type;
        ReZoom=TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : svClose
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void svClose(void)
{
    svShipViewRegion  = NULL;
    svFirepowerRegion = NULL;
    svCoverageRegion  = NULL;
    svManeuverRegion  = NULL;
    svArmorRegion     = NULL;
    svTopSpeedRegion  = NULL;
    svMassRegion      = NULL;

    svShipType = DefaultShip;
}

/*-----------------------------------------------------------------------------
    Name        : svReadMouseEvent
    Description : Callback for reading mouse events for use by svShipViewRender
    Inputs      : standard region callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword svReadMouseEvent(regionhandle region, sdword ID, udword event, udword data)
{
    rectangle *rect = &region->rect;
    rectangle defaultRect = {0, 0, MAIN_WindowWidth, MAIN_WindowHeight};

    switch(event)
    {
    case RPE_WheelUp:
        wheel_up = TRUE;
        break;

    case RPE_WheelDown:
        wheel_down = TRUE;
        break;

    case RPE_Enter:
        svMouseInside = TRUE;
        break;

    case RPE_Exit:
        svMouseInside     = FALSE;
        svMousePressRight = FALSE;
        svMousePressLeft  = FALSE;
        break;

    case RPE_PressRight:
        // Save last known position of cursor
        svMouseLastX = mouseCursorX();
        svMouseLastY = mouseCursorY();

        // Centre mouse
        svMouseCentreX = (rect->x0 + rect->x1) >> 1;
        svMouseCentreY = (rect->y0 + rect->y1) >> 1;
        mousePositionSet(svMouseCentreX, svMouseCentreY);
        mouseClipToRect(rect);

        svMousePressRight = TRUE;
        break;

    case RPE_PressLeft:
        svMousePressLeft = TRUE;
        break;

    case RPE_ReleaseRight:
        mousePositionSet(svMouseLastX, svMouseLastY);
        mouseClipToRect(&defaultRect);
        svMousePressRight = FALSE;
        break;

    case RPE_ReleaseLeft:
        svMousePressLeft = FALSE;
        break;
    }

    return 0;
}


#define SPIN_FEEDBACK 0.5
/*-----------------------------------------------------------------------------
    Name        : svShipViewRender
    Description : Callback which draws the main ship view.
    Inputs      : standard FE callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void svShipViewRender(featom* atom, regionhandle region)
{
    rectangle drawRect;
    rectangle* rect;
    rectangle viewRect;
    fonthandle currentFont;
    GLint viewPort[4];
    GLint box[4];
    ShipStaticInfo* info;
    real32 scale;
    sdword width, height;
    sdword x, y;
    keyindex key;
    char* keystring;
    bool resetRender = FALSE;
    char    temp[100];

    //test if we'll need a mesh callback from glcPageFlip later
    if (glcActive())
    {
        if (_svRender)
        {
            atom = _glcAtom;
            region = _glcRegion;
            resetRender = TRUE;
        }
        else
        {
            _glcAtom = atom;
            _glcRegion = region;
            _svRender = TRUE;
        }
    }

    rect = &region->rect;
    viewRect.x0 = 0;
    viewRect.y0 = 0;
    viewRect.x1 = MAIN_WindowWidth  - 1;
    viewRect.y1 = MAIN_WindowHeight - 1;
    info = NULL;

    if (svShipType != DefaultShip)
    {
        if (universe.curPlayerPtr)
        {
            info = GetShipStaticInfoSafe(svShipType, universe.curPlayerPtr->race);
        }
        if (info == NULL)
        {
            info = GetShipStaticInfoSafe(svShipType, GetValidRaceForShipType(svShipType));
        }
        if (info == NULL)
        {
            return;
        }
    }

    svShipViewRegion = region;

    if (!resetRender)
    {
        if (svMouseInside)
        {
            ferDrawFocusWindow(region, lw_focus);
        }
        else
        {
            ferDrawFocusWindow(region, lw_normal);
        }
    }

    soundEventUpdate();

    currentFont = fontMakeCurrent(svShipViewFont);

    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags =
            RPE_Enter | RPE_Exit |
            RPE_WheelDown | RPE_WheelUp |
            RPE_PressLeft | RPE_ReleaseLeft |
            RPE_PressRight | RPE_ReleaseRight;

        regFunctionSet(region, (regionfunction) svReadMouseEvent);               //set new region handler function
    }

    //scale = *svScale[svShipType];
        //svCamera.closestZoom  = *svMinZoom[svShipType];
    //svCamera.farthestZoom = *svMaxZoom[svShipType];

    scale = 1.0f; //*svScale[svShipType];

    if(svShipType != DefaultShip)
    {
        svCamera.closestZoom = info->minimumZoomDistance*svZoomInScalar;
        svCamera.farthestZoom = (svCamera.closestZoom+info->staticheader.staticCollInfo.approxcollspheresize)*svZoomOutScalar;
        if(ReZoom)
        {
            ReZoom=FALSE;
            cameraZoom(&svCamera,1.0f,FALSE);
        }

        if (svMouseInside && (wheel_down || wheel_up))
        {
            cameraControl(&svCamera, FALSE);
        }
        else if (svMouseInside && svMousePressRight)
        {
            camMouseX = (svMouseCentreX - mouseCursorX()) * 4;      //was 2
            camMouseY = (svMouseCentreY - mouseCursorY()) * 4;
            savecamMouseX = savecamMouseX * SPIN_FEEDBACK +
                (real32)camMouseX * (1.0f - SPIN_FEEDBACK);
            cameraControl(&svCamera, FALSE);                         //update the camera

            mouseCursorHide();
            mousePositionSet(svMouseCentreX, svMouseCentreY); // Reset position so it doesn't walk off region
        }
        else // auto rotate ship model
        {
            // continual 360 degree yaw rotation
            svCamera.angle += DEG_TO_RAD(1);
            
            // collapse pitch to default declination
            svCamera.declination += 0.02 * (DEG_TO_RAD(svDeclination) - svCamera.declination);

            if (svMouseInside) mouseCursorShow();
        }
    }
    //rotation

    drawRect.x0 = rect->x0 + SV_ViewMargin;
    drawRect.y0 = rect->y0 + SV_ViewMargin;
    drawRect.x1 = rect->x1 - SV_ViewMargin;
    drawRect.y1 = rect->y1 - SV_ViewMargin;

    width  = drawRect.x1 - drawRect.x0;
    height = drawRect.y1 - drawRect.y0;

    glGetIntegerv(GL_VIEWPORT, viewPort);
    glViewport(drawRect.x0, MAIN_WindowHeight - drawRect.y1, width, height);

    primModeSet2();
    if (!resetRender)
    {
        primRectSolid2(glcActive() ? &drawRect : &viewRect, FEC_Background);
    }
    primModeClear2();

    glEnable(GL_SCISSOR_TEST);
    glGetIntegerv(GL_SCISSOR_BOX, box);
    glScissor(drawRect.x0, MAIN_WindowHeight - drawRect.y1, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    rndPerspectiveCorrection(TRUE);

    //svCamera.lookatpoint.x = -info->staticheader.staticCollInfo.collsphereoffset.z * scale;
    //svCamera.lookatpoint.y = -info->staticheader.staticCollInfo.collsphereoffset.x * scale;
    //svCamera.lookatpoint.z = -info->staticheader.staticCollInfo.collsphereoffset.y * scale;
    if (svShipType == DefaultShip)
    {
        svCamera.lookatpoint.x=0.0f;
        svCamera.lookatpoint.y=0.0f;
        svCamera.lookatpoint.z=0.0f;
    }
    else
    {
        svCamera.lookatpoint.x = -info->staticheader.staticCollInfo.collsphereoffset.z;
        svCamera.lookatpoint.y = -info->staticheader.staticCollInfo.collsphereoffset.x;
        svCamera.lookatpoint.z = -info->staticheader.staticCollInfo.collsphereoffset.y;
    }

    cameraSetEyePosition(&svCamera);

    rndLightingEnable(TRUE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    rgluPerspective(
        svCamera.fieldofview,
        (float)(width) / (float)(height) /*rndAspectRatio*/,    //set projection matrix
        svCamera.clipPlaneNear,
        svCamera.clipPlaneFar);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    rgluLookAt(
        svCamera.eyeposition.x,
        svCamera.eyeposition.y,
        svCamera.eyeposition.z,
        svCamera.lookatpoint.x,
        svCamera.lookatpoint.y,
        svCamera.lookatpoint.z,
        svCamera.upvector.x,
        svCamera.upvector.y,
        svCamera.upvector.z);

    glPushMatrix();

    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)(&rndCameraMatrix));
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)(&rndProjectionMatrix));

    glEnable(GL_NORMALIZE);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);      //position light(s) within world

    glScalef(scale, scale, scale);

    if (svShipType != DefaultShip)
    {
        if (resetRender || !_svRender)
        {
            sdword index;
            //try player index colours first
            index = universe.curPlayerIndex;
            if (info->teamColor[index] == 0)
            {
                //colour scheme doesn't exist, search for something valid
                for (index = 0; index < MAX_MULTIPLAYER_PLAYERS; index++)
                {
                    if (info->teamColor[index] != 0)
                    {
                        break;
                    }
                }
                if (index == MAX_MULTIPLAYER_PLAYERS)
                {
                    //this ship doesn't have any colour info,
                    //at least avoid a GPF
                    index = universe.curPlayerIndex;
                }
            }
            meshRender((meshdata *)info->staticheader.LOD->level[0].pData, index);
        }
    }

    glDisable(GL_NORMALIZE);

    glPopMatrix();

    primModeSet2();

    glScissor(box[0], box[1], box[2], box[3]);
    glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

    rndLightingEnable(FALSE);
    rndPerspectiveCorrection(FALSE);

    x = rect->x0 + 2 + SV_ViewMargin;
    y = rect->y0 + 2 + SV_ViewMargin;

    if (svShipType != DefaultShip && !resetRender)
    {
        fontPrintf(
            x,
            y,
            FEC_ListItemStandard,
            "%s",
            ShipTypeToNiceStr(svShipType));

        y += fontHeight(" ");

        sprintf(temp, "%s %d %s",strGetString(strSVCost),info->buildCost, strGetString(strSVRUs));

        fontPrintf(
            x,
            y,
            FEC_ListItemStandard,
            temp);

        if (cmPrintHotKey)
        {
            x = rect->x1 - 2 - SV_ViewMargin;
            y = rect->y0 + 2 + SV_ViewMargin;

            key = cmShipTypeToKey(svShipType);
            keystring = opKeyToNiceString((keyindex)(key & 0x00ff));

            if (key & CM_SHIFT)
            {
                width = fontWidthf("[SHIFT-%s]",keystring);
                fontPrintf(
                    x-width,
                    y,
                    FEC_ListItemStandard,
                    "[SHIFT-%s]",
                    keystring);
            }
            else if (key)
            {
                width = fontWidthf("[%s]",keystring);
                fontPrintf(
                    x-width,
                    y,
                    FEC_ListItemStandard,
                    "[%s]",
                    keystring);
            }
        }
    }
    fontMakeCurrent(currentFont);

    svDirtyShipView();

    if (resetRender)
    {
        _svRender = FALSE;
    }
}


/*-----------------------------------------------------------------------------
    Name        : svFirepowerRender
    Description : Callback which draws the firepower stat in ship view.
    Inputs      : standard FE callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void svFirepowerRender(featom *atom, regionhandle region)
{
    rectangle *rect = &region->rect;
    fonthandle currentFont;
    uword firepower;
    char buftemp[50];
    ShipStaticInfo *info = GetShipStaticInfoSafe(svShipType, universe.curPlayerPtr->race);

    svFirepowerRegion = region;

    if (FELASTCALL(atom))
    {
        svFirepowerRegion = NULL;
        return;
    }

    if(svShipType == DefaultShip)
    {
        return;
    }
    if (info == NULL)
    {
        info = GetShipStaticInfoSafe(svShipType, GetValidRaceForShipType(svShipType));
    }
    if (info == NULL)
    {
        return;
    }

    currentFont = fontMakeCurrent(svShipStatFont);

    if (RGLtype == SWtype || glcActive()) primRectSolid2(&region->rect, FEC_Background);

    if(info->svFirePower !=0)
    {
        firepower = (uword) info->svFirePower;
    }
    else
    {
        firepower = (uword) gunShipFirePower(info, Neutral);
    }
    if(firepower != 0)
    {
        sprintf(buftemp,"%d",firepower);
    }
    else
    {
        sprintf(buftemp,"%s","-");
    }
    sprintf(buf,ShipStatToNiceStr(Firepower),buftemp);

    fontPrintf(
        rect->x0,
        rect->y0,
        FEC_ListItemStandard,
        "%s",
        buf);

    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : svCoverageRender
    Description : Callback which draws the coverage stat in ship view.
    Inputs      : standard FE callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void svCoverageRender(featom *atom, regionhandle region)
{
    rectangle *rect = &region->rect;
    sdword width;
    fonthandle currentFont;
    char buftemp[50];
    uword coverage;
    ShipStaticInfo *info = GetShipStaticInfoSafe(svShipType, universe.curPlayerPtr->race);

    svCoverageRegion = region;

    if (FELASTCALL(atom))
    {
        svCoverageRegion = NULL;
        return;
    }

    if(svShipType == DefaultShip)
    {
        return;
    }

    if (info == NULL)
    {
        info = GetShipStaticInfoSafe(svShipType, GetValidRaceForShipType(svShipType));
    }

    if (info == NULL)
    {
        return; //maybe print UNKNOWN!
    }

    currentFont = fontMakeCurrent(svShipStatFont);

    coverage = svShipCoverage(info);
    if(coverage != 0)
    {
        sprintf(buftemp,"%d %s",coverage,ShipStatToNiceStr(CoverageUnits));
    }
    else
    {
        sprintf(buftemp,"%s","-");
    }
    sprintf(buf,ShipStatToNiceStr(Coverage),buftemp);

    width = fontWidthf("%s",buf);

    if (RGLtype == SWtype || glcActive()) primRectSolid2(&region->rect, FEC_Background);

    fontPrintf(
        rect->x0,//rect->x1 - width,
        rect->y0,
        FEC_ListItemStandard,
        "%s",buf);

    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : svManeuverRender
    Description : Callback which draws the maneuver stat in ship view.
    Inputs      : standard FE callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void svManeuverRender(featom *atom, regionhandle region)
{
    rectangle *rect = &region->rect;
    fonthandle currentFont;
    char maneuverability[100] = "";
    ShipStaticInfo *info = GetShipStaticInfoSafe(svShipType, universe.curPlayerPtr->race);

    svManeuverRegion = region;

    if (FELASTCALL(atom))
    {
        svManeuverRegion = NULL;
        return;
    }

    if (svShipType == DefaultShip)
    {
        return;
    }
    if (info == NULL)
    {
        info = GetShipStaticInfoSafe(svShipType, GetValidRaceForShipType(svShipType));
    }
    if (info == NULL)
    {
        return;
    }

    currentFont = fontMakeCurrent(svShipStatFont);

    if (RGLtype == SWtype || glcActive()) primRectSolid2(&region->rect, FEC_Background);

    svShipManeuverability(info,maneuverability);
    sprintf(buf,ShipStatToNiceStr(Maneuver),maneuverability);

    fontPrintf(
        rect->x0,
        rect->y0,
        FEC_ListItemStandard,
        "%s",buf);

    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : svArmorRender
    Description : Callback which draws the armor stat in ship view.
    Inputs      : standard FE callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void svArmorRender(featom *atom, regionhandle region)
{
    rectangle *rect = &region->rect;
    sdword width;
    char buftemp[50];

    fonthandle currentFont;
    ShipStaticInfo *info = GetShipStaticInfoSafe(svShipType, universe.curPlayerPtr->race);

    svArmorRegion = region;

    if (FELASTCALL(atom))
    {
        svArmorRegion = NULL;
        return;
    }

    if(svShipType == DefaultShip)
    {
        return;
    }
    if (info == NULL)
    {
        info = GetShipStaticInfoSafe(svShipType, GetValidRaceForShipType(svShipType));
    }
    if (info == NULL)
    {
        return;
    }

    svArmorRegion = region;

    currentFont = fontMakeCurrent(svShipStatFont);

    sprintf(buftemp,"%d",(udword) info->maxhealth);
    sprintf(buf,ShipStatToNiceStr(Armor),buftemp);

    width = fontWidthf("%s",buf);

    if (RGLtype == SWtype || glcActive()) primRectSolid2(&region->rect, FEC_Background);

    fontPrintf(
        rect->x0,//rect->x1 - width,
        rect->y0,
        FEC_ListItemStandard,
        "%s",buf);

    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : svTopSpeedRender
    Description : Callback which draws the top speed stat in ship view.
    Inputs      : standard FE callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void svTopSpeedRender(featom *atom, regionhandle region)
{
    rectangle *rect = &region->rect;
    fonthandle currentFont;
    char buftemp[50];

    ShipStaticInfo *info = GetShipStaticInfoSafe(svShipType, universe.curPlayerPtr->race);

    svTopSpeedRegion = region;

    if (FELASTCALL(atom))
    {
        svTopSpeedRegion = NULL;
        return;
    }

    if(svShipType == DefaultShip)
    {
            return;
    }
    if (info == NULL)
    {
        info = GetShipStaticInfoSafe(svShipType, GetValidRaceForShipType(svShipType));
    }
    if (info == NULL)
    {
        return;
    }

    currentFont = fontMakeCurrent(svShipStatFont);

    if (RGLtype == SWtype || glcActive()) primRectSolid2(&region->rect, FEC_Background);

    //sprintf(buf,"%d", (uword)info->staticheader.maxvelocity);
    //topspeed contains a %d for the numerical location of the maxvelocity
    if(info->shiptype == Probe)
    {
        sprintf(buftemp,"%d",(udword) ((ProbeStatics *) info->custstatinfo)->ProbeDispatchMaxVelocity);
        sprintf(buf,ShipStatToNiceStr(TopSpeed),buftemp);
    }
    else
    {
        sprintf(buftemp,"%d",(udword) info->staticheader.maxvelocity);
        sprintf(buf,ShipStatToNiceStr(TopSpeed),buftemp);
    }
    fontPrintf(
        rect->x0,
        rect->y0,
        FEC_ListItemStandard,
        "%s",
        buf);



    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : svMassRender
    Description : Callback which draws the mass stat in ship view.
    Inputs      : standard FE callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void svMassRender(featom *atom, regionhandle region)
{
    rectangle *rect = &region->rect;
    fonthandle currentFont;
    sdword width;
    char buftemp[50];

    ShipStaticInfo *info = GetShipStaticInfoSafe(svShipType, universe.curPlayerPtr->race);

    svMassRegion = region;

    if (FELASTCALL(atom))
    {
        svMassRegion = NULL;
        return;
    }

    if(svShipType == DefaultShip)
    {
            return;
    }
    if (info == NULL)
    {
        info = GetShipStaticInfoSafe(svShipType, GetValidRaceForShipType(svShipType));
    }
    if (info == NULL)
    {
        return;
    }

    currentFont = fontMakeCurrent(svShipStatFont);

    //sprintf(buf, "%d", (uword)info->staticheader.mass);
    //Mass str contains a %d for the mass to expand into
    sprintf(buftemp,"%d",(udword)info->staticheader.mass);
    sprintf(buf,ShipStatToNiceStr(Mass),buftemp);

    width = fontWidthf("%s",
        buf);                       //width of number
        //ShipTypeStatToNiceStr(svShipType, Mass));//width of number


    if (RGLtype == SWtype || glcActive()) primRectSolid2(&region->rect, FEC_Background);

    fontPrintf(
        rect->x0,//rect->x1 - width,
        rect->y0,
        FEC_ListItemStandard,
        "%s",
        buf);
        //ShipTypeStatToNiceStr(svShipType, Mass));

    fontMakeCurrent(currentFont);
}


/*-----------------------------------------------------------------------------
    Name        :  svStartup()
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void svStartup(void)
{
    cameraInit(&svCamera, SV_InitialCameraDistance);

    svCamera.angle = DEG_TO_RAD(svAngle);
    svCamera.declination = DEG_TO_RAD(svDeclination);

    feDrawCallbackAdd(SV_ShipViewName, svShipViewRender);   //add render callbacks

    feDrawCallbackAdd(SV_FirepowerName, svFirepowerRender);
    feDrawCallbackAdd(SV_CoverageName, svCoverageRender);
    feDrawCallbackAdd(SV_ManeuverName, svManeuverRender);
    feDrawCallbackAdd(SV_ArmorName, svArmorRender);
    feDrawCallbackAdd(SV_TopSpeedName, svTopSpeedRender);
    feDrawCallbackAdd(SV_MassName, svMassRender);

    svMouseInside     = FALSE;
    svMousePressLeft  = FALSE;
    svMousePressRight = FALSE;

    svShipViewFont = frFontRegister(svShipViewFontName);
    svShipStatFont = frFontRegister(svShipStatFontName);
}

/*-----------------------------------------------------------------------------
    Name        :  svShutdown()
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void svShutdown(void)
{
}

uword svShipCoverage(ShipStaticInfo *statinfo)
{
    GunStaticInfo *guninfo = statinfo->gunStaticInfo;
    GunStatic *gun,*gun2;
    sdword i,j,numGuns;
    real32 coverage=0;
        real32 gunvalue,gunangledif;

    if(guninfo == NULL)
    {
        return(0);
    }

    numGuns = guninfo->numGuns;
    for(i=0;i<numGuns;i++)
    {
        gun = &guninfo->gunstatics[i];
        switch(gun->guntype)
        {
        case GUN_Fixed:
            //fixed guns don't really increase by anything..
            //but we'll make it feel better by giving it +1;
            coverage+=0.1f;
            break;
        case GUN_Gimble:
            gunvalue = (acos(gun->cosmaxAngleFromNorm))*2.0f;
                        for(j=i+1;j<numGuns;j++)
                        {
                                gun2 = &guninfo->gunstatics[i];
                                //angle difference between normals of two guns.
                                gunangledif = acos(vecDotProduct(gun->gunnormal,gun2->gunnormal));
                                gunangledif = gunangledif - acos(gun->cosmaxAngleFromNorm) - acos(gun2->cosmaxAngleFromNorm);
                                if(gunangledif < 0.0f)
                                        gunvalue+=gunangledif;
                                if(gunvalue <= 0.0f)
                                        break;
                        }
                        coverage+=max(gunvalue,0.0f);
            break;
        case GUN_NewGimble:
            coverage = coverage + (min(2*gun->maxturnangle,2*gun->maxdeclination));
                        break;
        case GUN_MissileLauncher:
        case GUN_MineLauncher:
            coverage=2*PI;
            goto donecoveragecalc;
            break;

        }
    }

donecoveragecalc:
    coverage = 100.0*coverage/(2*PI);
    return(min(((uword) coverage),100));
}

void svShipManeuverability(ShipStaticInfo *statinfo,char *name)
{
    sdword man=0;
    real32 turnspeed;
    if(statinfo->svManeuverability != 0)
    {
        man = statinfo->svManeuverability;
calcmanjump:
        switch (man)
        {
        case 1:         //very low
            sprintf(name,"%s",ShipStatToNiceStr(VeryLow));
            break;
        case 2:         //low
            sprintf(name,"%s",ShipStatToNiceStr(Low));
            break;
        case 3:         //medium
            sprintf(name,"%s",ShipStatToNiceStr(Medium));
            break;
        case 4:         //high
            sprintf(name,"%s",ShipStatToNiceStr(High));
            break;
        case 5:         //Very High
            sprintf(name,"%s",ShipStatToNiceStr(VeryHigh));
            break;
        default:
            dbgFatalf(DBG_Loc,"Error Calculating Maneuverability");
        }
        return;
    }
//calculate maneuverability
    turnspeed = statinfo->turnspeedstat[TURN_YAW]+
                statinfo->turnspeedstat[TURN_PITCH]+
                statinfo->turnspeedstat[TURN_ROLL];
    if(turnspeed < TW_SV_MAN_VERY_LOW)
        man=1;
    else if(turnspeed < TW_SV_MAN_LOW)
        man=2;
    else if(turnspeed < TW_SV_MAN_MEDIUM)
        man=3;
    else if(turnspeed < TW_SV_MAN_HIGH)
        man=4;
    else
        man=5;
    goto calcmanjump;
}

