/*=============================================================================
    Name    : Pieplate.c
    Purpose : Code for the movement pieplate mechanism.

    Created 4/9/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#ifdef _WIN32
#include <windows.h>
#endif
#endif
#include "glinc.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include "SpaceObj.h"
#include "Universe.h"
#include "mouse.h"
#include "Select.h"
#include "Undo.h"
#include "font.h"
#include "FontReg.h"
#include "main.h"
#include "color.h"
#include "mainrgn.h"
#include "prim2d.h"
#include "prim3d.h"
#include "FastMath.h"
#include "render.h"
#include "Sensors.h"
#include "mainrgn.h"
#include "LOD.h"
#include "Clipper.h"
#include "PiePlate.h"
#include "glcaps.h"
#include "ProximitySensor.h"
#include "Tutor.h"
#include "SoundEvent.h"

/*=============================================================================
    Data:
=============================================================================*/
//parameters of the point spec overlays
real32 piePizzaDishRadius = PIE_PizzaDishRadius;
sdword piePizzaSlices = PIE_PizzaSlices;
sdword piePointSpecMode = PSM_Idle;
sdword pieOldPointSpecMode;
sdword piePointSpecMouseReset;
real32 piePointSpecZ;
vector piePlanePoint = { 0.0f, 0.0f, 0.0f };
vector pieHeightPoint = { 0.0f, 0.0f, 0.0f };
real32 pieOriginSizeCentre = PIE_OriginSizeCentre;
real32 pieOriginSizeDish = PIE_OriginSizeDish;
real32 pieOriginSizeHeight = PIE_OriginSizeHeight;
real32 pieShipLineTickSize = PIE_ShipLineTickSize;
real32 pieClosestDistance = PIE_ClosestDistance;
real32 pieDottedDistance = PIE_DottedDistance;
ShipPtr pieLastClosestShip;
void (*pieLineDrawCallback)(real32 distance) = NULL;
void (*pieMovementCursorDrawCallback)(real32 distance) = NULL;
void (*piePlaneDrawCallback)(real32 distance) = NULL;
real32 pieMaxMoveHorizontal = PIE_MaxMoveHorizontal;
real32 pieMaxMoveVertical = PIE_MaxMoveVertical;
real32 pieCircleSizeMax;
real32 pieCircleSizeMin;

real32 pieHeight   = 0;
real32 pieDistance = 0;
vector pieMoveTo   = {0.0f, 0.0f, 0.0f};

static fonthandle pieDistReadFont = 0;

//parameters for dashed lines
udword pieStipplePattern;
uword pieLineStipple = PIE_LineStipple;
real32 pieLineStippleCounter, pieLineStippleSpeed = PIE_StippleSpeed;

//function prototype - located in mainrgn.c
void toDrawPulsedLine(vector linestart, vector lineend, real32 pulsesize, color linecolor, color pulsecolor);

//number of segments to use to render the ship - plane circles at various LOD's
pieplanecirclesegments piePlaneCircleSegments[PIE_NumberCircleLODs];

//heading of the mothership as it last existed
vector pieMotherShipHeading = {0.0f, 1.0f, 0.0f};

color moveLineColor = colWhite;


#if PIE_VISUALIZE_EXTENTS
bool pieVisualizeExtents = FALSE;
#endif

vector pieScreen0, pieScreen1;
bool pieOnScreen = FALSE;
extern real32 smZoomMax;

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : pieStartup
    Description : initialize the pieplate module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pieStartup(void)
{
    pieCircleSizeMax = primScreenToGLScaleX(MAIN_WindowWidth / 6.0f);
    pieCircleSizeMin = primScreenToGLScaleX(8);

    piePlaneCircleSegments[0].screenRadius = primScreenToGLScaleX(200);
    piePlaneCircleSegments[0].nSegments = 32;
    piePlaneCircleSegments[1].screenRadius = primScreenToGLScaleX(80);
    piePlaneCircleSegments[1].nSegments = 20;
    piePlaneCircleSegments[2].screenRadius = primScreenToGLScaleX(32);
    piePlaneCircleSegments[2].nSegments = 16;
    piePlaneCircleSegments[3].screenRadius = primScreenToGLScaleX(10);
    piePlaneCircleSegments[3].nSegments = 12;
    piePlaneCircleSegments[4].screenRadius = primScreenToGLScaleX(4);
    piePlaneCircleSegments[4].nSegments = 8;
    piePlaneCircleSegments[5].screenRadius = primScreenToGLScaleX(2);
    piePlaneCircleSegments[5].nSegments = 6;
    piePlaneCircleSegments[6].screenRadius = primScreenToGLScaleX(0);
    piePlaneCircleSegments[6].nSegments = 4;
}

/*-----------------------------------------------------------------------------
    Name        : pieCircleSegmentsCompute
    Description : Compute a suitable number of segments required to draw a circle.
    Inputs      : screenRadius - radius of circle, in GL coordinates
    Outputs     :
    Return      : number of segments
----------------------------------------------------------------------------*/
sdword pieCircleSegmentsCompute(real32 screenRadius)
{
    sdword nSegments, index;

    nSegments = piePlaneCircleSegments[PIE_NumberCircleLODs - 1].nSegments;
    for (index = 0; index < PIE_NumberCircleLODs; index++)
    {
        if (screenRadius > piePlaneCircleSegments[index].screenRadius)
        {
            nSegments = piePlaneCircleSegments[index].nSegments;
            return(nSegments);
        }
    }
    return(piePlaneCircleSegments[0].nSegments);
}

/*-----------------------------------------------------------------------------
    Name        : pieCancelPointSpecMode
    Description : Undo callback to cancel point-spec mode.
    Inputs      :
    Outputs     : Just sets piePointSpecMode to idle
    Return      : void
----------------------------------------------------------------------------*/
bool pieCancelPointSpecMode(sdword userID, ubyte *userData, sdword length)
{
    if (piePointSpecMode != PSM_Idle)
    {
        piePointSpecMode = PSM_Idle;
        return TRUE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : piePointModeOnOff
    Description : Toggles ship movement mode and associated navigation aids.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void piePointModeOnOff(void)
{
    if (piePointSpecMode != PSM_Idle)
    {
        mouseCursorClear(MCM_Pointmode);
        piePointSpecMode = PSM_Idle;

//      soundEvent(NULL, UI_MovementGUIoff);

        tutGameMessage("Game_MoveQuit");
    }
    else if (selSelected.numShips != 0)
    {                                                       //if any ships selected
        tutGameMessage("Game_Move");

        soundEvent(NULL, UI_MovementGUIon);

        mouseCursorSet(MCM_Pointmode);
        piePointSpecMode = PSM_XY;
        piePointSpecMouseReset = FALSE;
        piePointSpecZ = 0.0f;
        pieLineStippleCounter = 0.0f;
        selCentrePointCompute();
        piePlanePoint.x = pieHeightPoint.x = selCentrePoint.x;
        piePlanePoint.y = pieHeightPoint.y = selCentrePoint.y;
        piePlanePoint.z = pieHeightPoint.z = selCentrePoint.z;
        udLatestThingPush(pieCancelPointSpecMode, 0, NULL, 0);
    }
}

/*-----------------------------------------------------------------------------
    Name        : piePointModePause
    Description : Pause/unpause point spec mode.  Overlays will still be drawn,
                    just the origins will not follow the mouse.
    Inputs      : bPause - flag telling whether to pause or unpause point
    Outputs     : Saves current piePointSpecMode to pieOldPointSpecMode
    Return      : void
----------------------------------------------------------------------------*/
void piePointModePause(sdword bPause)
{
    if (bPause)
    {
        pieOldPointSpecMode = piePointSpecMode;               //save current spec mode
        if (piePointSpecMode != PSM_Idle)
        {                                                   //if pause active point spec mode
            piePointSpecMode = PSM_Waiting;                  //pause the specification
        }
    }
    else if (piePointSpecMode == PSM_Waiting)
    {
        piePointSpecMode = pieOldPointSpecMode;               //restore the point spec mode
        piePointSpecMouseReset = TRUE;                       //reset location of mouse cursor
    }
}

/*-----------------------------------------------------------------------------
    Name        : pieMousePosSet3D
    Description : Sets the on-screen location of the mouse based upon a 3D
                    vector and the current projection/movelview matrices.
    Inputs      : modelView/projection - current matrices
                  mouse3D - 3D location to set the mouse to
    Outputs     :
    Return      : OKAY if the mouse was repositioned properly, ERROR if the
                    mouse was off-screen.
----------------------------------------------------------------------------*/
sdword pieMousePosSet3D(hmatrix *modelView, hmatrix *projection, vector *mouse3D)
{
    hvector mouseWorld, mouseCamera, mouseScreen;
    sdword mouseViewX, mouseViewY;

    mouseWorld.x = mouse3D->x;                              //make a homo coord out of the mouse coordinate
    mouseWorld.y = mouse3D->y;
    mouseWorld.z = mouse3D->z;
    mouseWorld.w = 1.0f;
    hmatMultiplyHMatByHVec(&mouseCamera, modelView, &mouseWorld);//in camera space
    hmatMultiplyHMatByHVec(&mouseScreen, projection, &mouseCamera);//in screen space
    if (mouseScreen.z <= 0.0f)                              //if point behind camera
    {
        return(ERROR);
    }
    mouseViewX = primGLToScreenX(mouseScreen.x / mouseScreen.w);//get location in viewport coords
    mouseViewY = primGLToScreenY(mouseScreen.y / mouseScreen.w);

    //dbgMessagef("\n3d->2d mouse pos: (%.2f, %.2f, %.2f) -> (%d,%d).", mouse3D->x, mouse3D->y, mouse3D->z, mouseViewX, mouseViewY);
    if (mouseViewX < 0 || mouseViewX >= MAIN_WindowWidth || //if offscreen
        mouseViewY < 0 || mouseViewY >= MAIN_WindowHeight)
    {
        return(ERROR);
    }
    mousePositionSet(mouseViewX, mouseViewY);
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : pieSameOnScreenPoint
    Description : See if two points will be differnt pixels on-screen
    Inputs      : modelView/projection - camera/viewport matrices
                  world0, world1 - worldspace coordinates to compare
    Outputs     :
    Return      : TRUE if both points will be in the same place on-screen
----------------------------------------------------------------------------*/
bool pieSameOnScreenPoint(hmatrix *modelView, hmatrix *projection, vector *world0, vector *world1)
{
    sdword x0, x1, y0, y1;
    hvector world, camera, screen;

    world.x = world0->x;
    world.y = world0->y;
    world.z = world0->z;
    world.w = 1.0f;
    hmatMultiplyHMatByHVec(&camera, modelView, &world);     //in camera space
    hmatMultiplyHMatByHVec(&screen, projection, &camera);   //in screen space
    if (screen.z <= 0.0f)
    {                                                       //if behind camera
        return(FALSE);
    }
    x0 = primGLToScreenX(screen.x / screen.w);              //get location in viewport coords
    y0 = primGLToScreenY(screen.y / screen.w);
    if (x0 < 0 || x0 >= MAIN_WindowWidth ||                 //if offscreen
        y0 < 0 || y0 >= MAIN_WindowHeight)
    {
        return(FALSE);
    }
    world.x = world1->x;
    world.y = world1->y;
    world.z = world1->z;
    world.w = 1.0f;
    hmatMultiplyHMatByHVec(&camera, modelView, &world);     //in camera space
    hmatMultiplyHMatByHVec(&screen, projection, &camera);   //in screen space
    if (screen.z <= 0.0f)
    {                                                       //if behind camera
        return(FALSE);
    }
    x1 = primGLToScreenX(screen.x / screen.w);              //get location in viewport coords
    y1 = primGLToScreenY(screen.y / screen.w);
    if (x1 < 0 || x1 >= MAIN_WindowWidth ||                 //if offscreen
        y1 < 0 || y1 >= MAIN_WindowHeight)
    {
        return(FALSE);
    }
    return(x0 == x1 && y0 == y1);
}

/*-----------------------------------------------------------------------------
    Name        : pieOriginDraw
    Description : Draw arrows along the axes.
    Inputs      : centre - centre or 'origin' of the origin.
                  size - length from the centre to end of line segments.
                  color - color to draw in
    Outputs     :
    Return      :
    Note        : Currently we will draw just plain lines.  Later, we may want
                    to add arrowheads or use a mesh.
----------------------------------------------------------------------------*/
void pieOriginDraw(vector *centre, real32 size, color c)
{
    vector p0, p1;

    //draw line on z axis
    p0.x = p1.x = centre->x;
    p0.y = p1.y = centre->y;
    p0.z = centre->z - size;
    p1.z = centre->z + size;
    primLine3(&p0, &p1, c);
    //draw line on y axis
    p0.y = centre->y - size;
    p1.y = centre->y + size;
    p0.z = p1.z = centre->z;
    primLine3(&p0, &p1, c);
    //draw line on x axis
    p0.x = centre->x - size;
    p1.x = centre->x + size;
    p0.y = p1.y = centre->y;
    primLine3(&p0, &p1, c);
}


/*-----------------------------------------------------------------------------
    Name        : pieDistanceReadoutDraw
    Description : Draw a numerical representation of the distance from the ship
                  to the movement mechanism pointer
    Inputs      : movepoint - the point where the ship is supposed to go
                  origin - the point where the ship is located
                  c - the color for drawing the text
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/

#define PIE_DIST_READOUT_X_OFFSET 10
#define PIE_DIST_READOUT_Y_OFFSET 10

#define PIE_RU_READOUT_X_OFFSET 10
#define PIE_RU_READOUT_Y_OFFSET 2

void pieDistanceReadoutDraw(vector *movepoint, vector *origin, color c)
{
    real32 dist_x, dist_y, dist_z;
    real32 screen_x, screen_y, dummy_r;
    udword distance;
    udword resources;
    real32 cost;
    fonthandle fhSave;
    color ruReadoutColour;
    hmatrix modelview, projection;
    bool depthOn;
    char dist_str[20],ru_str[20];

    dist_x = movepoint->x - origin->x;
    dist_y = movepoint->y - origin->y;
    dist_z = movepoint->z - origin->z;

    //update global variable
    pieHeight = dist_z;

    dist_x *= dist_x;
    dist_y *= dist_y;
    dist_z *= dist_z;

    //find la distance (using that wonderful formula by Pythagoras)
    pieDistance = fsqrt(dist_x + dist_y + dist_z);
    distance = (udword)pieDistance;

    //update the global moveto vector and distance variables
    pieMoveTo   = *movepoint;

    if(MP_HyperSpaceFlag)
    {
        //multiplayer...
        cost = hyperspaceCost(pieDistance,(SelectCommand *)&selSelected);
        resources = (udword) cost;
        sprintf(ru_str, "%i RU's", resources);

        if(universe.curPlayerPtr->resourceUnits < resources)
        {
            ruReadoutColour = TW_RU_READOUT_COLOR_BAD;
        }
		else
        {
            ruReadoutColour =  TW_RU_READOUT_COLOR_GOOD;
        }

    }

    //depending on the distance, scale the readout
    if (distance < 100)
    {
        sprintf(dist_str, "%i m", distance);
    }
    else if ((distance >= 100) && (distance < 10000))
    {
        distance /= 100;
        distance *= 100;
        sprintf(dist_str, "%i m", distance);
    }
    else if (distance >= 10000)
    {
        distance /= 1000;
        sprintf(dist_str, "%i km", distance);
    }

    //find the screen coordinates of the moveto pointer
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)(&modelview));
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)(&projection));
    selCircleComputeGeneral(&modelview, &projection, movepoint, 0, &screen_x, &screen_y, &dummy_r);

    fhSave = fontCurrentGet();
    fontMakeCurrent(pieDistReadFont);

    //and here I set up my own 2D space
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    depthOn = (bool)glIsEnabled(GL_DEPTH_TEST);
    if (depthOn) glDisable(GL_DEPTH_TEST);

    //using the mousecursor position to position the distance readout is more
    //stable, but can't be used when the camera is being moved, so the moveto
    //position is projected onto the screen in that case
    if ((mrHoldRight == mrNULL) && (smHoldRight == smNULL))
    {
        fontPrint((mouseCursorXPosition + PIE_DIST_READOUT_X_OFFSET),
                  (mouseCursorYPosition + PIE_DIST_READOUT_Y_OFFSET),
                  TW_DISTANCE_READOUT_COLOR, dist_str);
        if(MP_HyperSpaceFlag)
        {
            fontPrint((mouseCursorXPosition + PIE_RU_READOUT_X_OFFSET),
                  (mouseCursorYPosition + PIE_DIST_READOUT_Y_OFFSET+PIE_RU_READOUT_Y_OFFSET+fontHeight("K")),
                  ruReadoutColour, ru_str);

        }
    }
    else
    {
        fontPrint((primGLToScreenX(screen_x) + PIE_DIST_READOUT_X_OFFSET),
                  (primGLToScreenY(screen_y) + PIE_DIST_READOUT_Y_OFFSET),
                  TW_DISTANCE_READOUT_COLOR, dist_str);
        if(MP_HyperSpaceFlag)
        {
            fontPrint((primGLToScreenX(screen_x) + PIE_RU_READOUT_X_OFFSET),
                  (primGLToScreenY(screen_y) + PIE_DIST_READOUT_Y_OFFSET+PIE_RU_READOUT_Y_OFFSET+fontHeight("K")),
                  ruReadoutColour, ru_str);

        }
    }

    if (depthOn) glEnable(GL_DEPTH_TEST);

    //back to previous space
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    fontMakeCurrent(fhSave);
}
#undef PIE_DIST_READOUT_X_OFFSET
#undef PIE_DIST_READOUT_Y_OFFSET

//list of points to draw for the dots where the vertical lines meet the pie plane
struct
{
    real32 x, y;
}
piePlaneScreenPoint[PIE_PlaneScreenPointIndex];
sdword piePlaneScreenPointIndex;
/*-----------------------------------------------------------------------------
    Name        : pieAllShipsToPiePlateDraw
    Description : Draw a line from all ships to the point-spec pie plate
    Inputs      : distance - adjusted distance from camera to ship
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void pieAllShipsToPiePlateDraw(real32 distance)
{
    Node *objnode;
    Ship *ship;
    vector planePoint, shipPoint;//, innerPoint;
    real32 length;
    real32 closestDistance = REALlyBig;
    bool bHeightPointDrawn, bClosestHeightDrawn;
    ShipPtr closestShip = NULL;
    color c;
    real32 screenX, screenY, screenRadius, radius;
    sdword nSegments;

    objnode = universe.RenderList.head;                    //get first node in list

    distance *= pieShipLineTickSize;                         //get distance-adjusted size

    piePlaneScreenPointIndex = 0;
    while (objnode != NULL)
    {                                                       //for every ship
        ship = (Ship *)listGetStructOfNode(objnode);
        if (ship->flags & SOF_Dead)
        {
            goto nextnode;
        }

        if (ship->shiptype == Drone)
        {
            goto nextnode;
        }

        if (ship->objtype != OBJ_ShipType)
        {
            goto nextnode;
        }

        if (bitTest(ship->flags,SOF_Cloaked) && ship->playerowner != universe.curPlayerPtr)
        {                                            //if ship is cloaked, don't draw this stuff
            if(!proximityCanPlayerSeeShip(universe.curPlayerPtr, ship))
            {
                goto nextnode;
            }
        }

        shipPoint.x = planePoint.x = ship->posinfo.position.x;
        shipPoint.y = planePoint.y = ship->posinfo.position.y;
        shipPoint.z = ship->posinfo.position.z;
        planePoint.z = selCentrePoint.z;

        if (ship == pieLastClosestShip)
        {                                                   //if closest ship from last frame
            c = TW_SHIP_LINE_CLOSEST_COLOR;
        }
        else
        {
            c = TW_SHIP_LINE_COLOR;
        }
        primLine3(&planePoint, &shipPoint, c);              //draw line from plane to ship
        length = fsqrt(((shipPoint.x - selCentrePoint.x) * (shipPoint.x - selCentrePoint.x) +
                 (shipPoint.y - selCentrePoint.y) * (shipPoint.y - selCentrePoint.y)));
        //now draw circle on the plane
        bHeightPointDrawn = FALSE;
        if (ABS(shipPoint.z - planePoint.z) > ship->staticinfo->staticheader.staticCollInfo.collspheresize)
        {
            radius = ship->staticinfo->staticheader.staticCollInfo.collspheresize;
            selCircleComputeGeneral(&rndCameraMatrix, &rndProjectionMatrix,
                                &planePoint, radius, &screenX, &screenY, &screenRadius);
            if (screenRadius > 0.0f)
            {
//                dbgAssert(piePlaneScreenPointIndex < PIE_PlaneScreenPointIndex);
                if (piePlaneScreenPointIndex < PIE_PlaneScreenPointIndex)
                {
                    piePlaneScreenPoint[piePlaneScreenPointIndex].x = screenX;
                    piePlaneScreenPoint[piePlaneScreenPointIndex].y = screenY;
                    piePlaneScreenPointIndex++;
                    nSegments = pieCircleSegmentsCompute(screenRadius);
                    primCircleOutlineZ(&planePoint, radius,
                                       nSegments, c);
                    bHeightPointDrawn = TRUE;
                }
            }
        }

        //see if this is the closest ship to the cursor
        length = ABS(piePlanePoint.x - planePoint.x) +       //manhattan distance
            ABS(piePlanePoint.y - planePoint.y);
        if (length < closestDistance)
        {                                                   //if this ship closest so far
            closestDistance = length;
            closestShip = ship;
            bClosestHeightDrawn = bHeightPointDrawn;
        }

nextnode:
        objnode = objnode->next;
    }

    if (closestShip != NULL)
    {
        if (closestDistance > max(distance, pieClosestDistance))
        {                                                       //if the closest ship wasn't close enough
            closestShip = NULL;                                 //there was no closest ship
        }
    }

    /* play the movement gui click sound */
    if ((closestShip != NULL) && (pieLastClosestShip != closestShip) && !selShipInSelection(selSelected.ShipPtr, selSelected.numShips, closestShip) && bClosestHeightDrawn)
    {
        soundEvent(NULL, UI_MoveChimes);
    }

    pieLastClosestShip = closestShip;
}

/*-----------------------------------------------------------------------------
    Name        : pieWorldLocationCompute
    Description : Compute the location of the mouse in world coordinates by
                    'unprojecting' them.
    Outputs     : fills in world0 and world1 with 3d points describing a line
                    under the mouse.
                  eyeMag - approximate magnitude of the camera's eye position
    Return      : void
----------------------------------------------------------------------------*/
void pieWorldLocationCompute(vector *world0, vector *world1, real32 *eyeMag)
{
    world0->x = mrCamera->eyeposition.x;
    world0->y = mrCamera->eyeposition.y;
    world0->z = mrCamera->eyeposition.z;
    *eyeMag = fsqrt(vecMagnitudeSquared(*world0));

/*
    //ray-cast the four corners of the frustum
    cameraRayCast(world1, mrCamera, modelView, 0, 0, MAIN_WindowWidth, MAIN_WindowHeight);
    cameraRayCast(world1, mrCamera, modelView, MAIN_WindowWidth - 1, 0, MAIN_WindowWidth, MAIN_WindowHeight);
    cameraRayCast(world1, mrCamera, modelView, 0, MAIN_WindowHeight - 1, MAIN_WindowWidth - 1, MAIN_WindowHeight);
    cameraRayCast(world1, mrCamera, modelView, MAIN_WindowWidth - 1, MAIN_WindowHeight - 1, MAIN_WindowWidth, MAIN_WindowHeight);
*/
    cameraRayCast(world1, mrCamera, mouseCursorX(), mouseCursorY(), MAIN_WindowWidth, MAIN_WindowHeight);
    vecMultiplyByScalar(*world1, *eyeMag);
    vecAdd(*world1, *world1, *world0);
}

/*-----------------------------------------------------------------------------
    Name        : pieScreenSizeOfCircleCompute
    Description : Compute the size of the movement circle, clipping it to a
                    minimum and maximum in screen space.
    Inputs      : position - where on screen it is to be rendered
                  radius - radius, world space of the thingus
    Outputs     : outRadius - new world radius, possible adjusted
                  mSegments - number of segments to use when drawing the circle.
    Return      :
----------------------------------------------------------------------------*/
void pieScreenSizeOfCircleCompute(vector *position, real32 inRadius, real32 *outRadius, sdword *nSegments)
{
    real32 screenX, screenY, screenRadius;
    real32 factor;

    *outRadius = inRadius;
    //figure out what size it is now
    selCircleComputeGeneral(&rndCameraMatrix, &rndProjectionMatrix, position, inRadius, &screenX, &screenY, &screenRadius);

    if (screenRadius > pieCircleSizeMax)
    {                                                       //if it is currently too large
        factor = pieCircleSizeMax / screenRadius;
        *outRadius *= factor;
        screenRadius *= factor;
    }
    if (screenRadius < pieCircleSizeMin)
    {                                                       //if currently too small
        factor = pieCircleSizeMin / screenRadius;
        *outRadius *= factor;
        screenRadius *= factor;
    }
    *nSegments = pieCircleSegmentsCompute(screenRadius);
}

/*-----------------------------------------------------------------------------
    Name        : pieNeedSpecialAttackAndMoveColor
    Description : returns true if all ships in selection have an attack order
                  and are going to perform an attack and move.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool pieNeedSpecialAttackAndMoveColor()
{
    sdword i,j;
    CommandToDo *command;

    j=0;
    for(i=0;i<selSelected.numShips;i++)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer,selSelected.ShipPtr[i]);
        if(command == NULL)
            return FALSE;
        if(command->ordertype.order != COMMAND_ATTACK)
            return FALSE;

        if(selSelected.ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
           selSelected.ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette ||
           selSelected.ShipPtr[i]->staticinfo->cantMoveAndAttack)
            j++;
    }
    if(j==selSelected.numShips)
        return FALSE;           //all strike craft...don't do special move attack!
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : pieMovementCursorDraw
    Description : Default movement mechanism for rendering the dotted lines and
                    destination cursors of the movement mechanism.
    Inputs      : distance - size multiplier for no good reason?
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pieMovementCursorDraw(real32 distance)
{
    bool stipple;
    real32 scaledSize;
    sdword nSegments;

    stipple = glCapFastFeature(GL_LINE_STIPPLE);
    if (ABS(piePointSpecZ) < pieDottedDistance)
    {
        if (stipple)
        {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, (GLushort)pieStipplePattern);
        }
        primLine3(&piePlanePoint, &selCentrePoint, moveLineColor);//draw line from centre to mouse point on x/y plane
        if (stipple)
        {
            glDisable(GL_LINE_STIPPLE);
        }
    }
    else
    {                                                       //if point off standard Z-plane
        if (stipple)
        {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, (GLushort)pieStipplePattern);
        }
        primLine3(&pieHeightPoint, &selCentrePoint, moveLineColor);//draw from centre of dish to height point
        if (stipple)
        {
            glDisable(GL_LINE_STIPPLE);
        }

        primLine3(&pieHeightPoint, &piePlanePoint, moveLineColor); //TW_MOVE_PIZZA_COLOR tweakable global variable (tweak.*)
        primLine3(&piePlanePoint, &selCentrePoint, moveLineColor);//draw line from centre to mouse point on x/y plane
        //destination circle
        pieScreenSizeOfCircleCompute(&pieHeightPoint, selAverageSize, &scaledSize, &nSegments);
        primCircleOutlineZ(&pieHeightPoint, scaledSize, nSegments, moveLineColor);
    }
}

/*-----------------------------------------------------------------------------
    Name        : pieDestinationInGunRangeOfTargets
    Description : returns TRUE if all attacking ships will
                  be within gun range of at least 1 of their targets at the new
                  position
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool pieDestinationInGunRangeOfTargets()
{
    sdword i,j;
    vector distvec;
    real32 distSqr;
    sdword breaker;
    CommandToDo *command;

    for(i=0;i<selSelected.numShips;i++)
    {
        if(selSelected.ShipPtr[i]->shiptype == CloakGenerator ||
           selSelected.ShipPtr[i]->shiptype == GravWellGenerator ||
           selSelected.ShipPtr[i]->shiptype == DFGFrigate)
            continue;
        command = getShipAndItsCommand(&universe.mainCommandLayer,selSelected.ShipPtr[i]);

        dbgAssert(command != NULL);
        dbgAssert(command->ordertype.order == COMMAND_ATTACK);

        breaker = FALSE;
        for(j = 0; j<command->attack->numTargets;j++)
        {
            vecSub(distvec,pieHeightPoint,command->attack->TargetPtr[j]->posinfo.position);
            distSqr = vecMagnitudeSquared(distvec);
            if(distSqr < selSelected.ShipPtr[i]->staticinfo->bulletRangeSquared[selSelected.ShipPtr[i]->tacticstype])
            {
                //within range...break and check next ship
                breaker = TRUE;
                break;
            }
        }
        if(!breaker)
            return FALSE;   //ship will not be within range
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : piePlaneDraw
    Description : Default draw callback for rendering the movement mechanism
                    pizza dish.
    Inputs      : distance - radius of the movement mechanism
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void piePlaneDraw(real32 distance)
{
    vector world0, world1, plane;
    //Ship *mothership;
    real32 scaledSize;
    sdword nSegments;
    color pieColor,spokeColor,headingColor;

    if(pieNeedSpecialAttackAndMoveColor())
    {
        if(pieDestinationInGunRangeOfTargets())
        {
            moveLineColor = TW_MOVE_ATTACK_POINT_LINE_COLOR_IN_RANGE;
        }
        else
        {
            moveLineColor = TW_MOVE_ATTACK_POINT_LINE_COLOR_NOT_IN_RANGE;
        }
        pieColor = TW_MOVE_ATTACK_PIE_COLOR;
        spokeColor = TW_MOVE_ATTACK_SPOKE_COLOR;
        headingColor = TW_MOVE_HEADING_COLOR;
    }
    else if(MP_HyperSpaceFlag)
    {
        pieColor = TW_MOVE_HYPERSPACE_COLOUR;
        moveLineColor = TW_MOVE_POINT_HYPERSPACE_LINE_COLOR;
        spokeColor = TW_MOVE_SPOKE_HYPERSPACE_COLOR;
        headingColor = TW_MOVE_HEADING_HYPERSPACE_COLOR;
    }
    else
    {
        pieColor = TW_MOVE_PIZZA_COLOR;
        moveLineColor = TW_MOVE_POINT_LINE_COLOR;
        spokeColor = TW_MOVE_SPOKE_COLOR;
        headingColor = TW_MOVE_HEADING_COLOR;
    }

    primCircleOutlineZ(&selCentrePoint, piePizzaDishRadius * distance,
                       piePizzaSlices, pieColor);//TW_MOVE_PIZZA_COLOR tweakable global variable (tweak.*)
    //get the heading of the mothership for drawing the 'spokes' of the pie plate
    /*
    mothership = universe.curPlayerPtr->PlayerMothership;
    if (mothership != NULL)
    {
        matGetVectFromMatrixCol3(pieMotherShipHeading, mothership->rotinfo.coordsys);
        //handle an unlikely case: mothership pointing just about straight 'down'
        if (pieMotherShipHeading.z >= (1.0f - (real32)1e-6))
        {
            pieMotherShipHeading.x = pieMotherShipHeading.z = 0.0f;
            pieMotherShipHeading.y = 1.0f;
        }
        else
        {
            pieMotherShipHeading.z = 0;
            vecNormalize(&pieMotherShipHeading);
        }
    }
    dbgAssert(pieMotherShipHeading.x != REALlyBig);
    */
    pieMotherShipHeading.x = pieMotherShipHeading.z = 0.0f;
    pieMotherShipHeading.y = 1.0f;
    vecScalarMultiply(plane, pieMotherShipHeading, piePizzaDishRadius * distance);
    world0.x = selCentrePoint.x + plane.x;
    world0.y = selCentrePoint.y + plane.y;
    world0.z = selCentrePoint.z;
    primLine3(&selCentrePoint, &world0, spokeColor);//headingColor);
    world0.x = selCentrePoint.x - plane.x;
    world0.y = selCentrePoint.y - plane.y;
    primLine3(&selCentrePoint, &world0, spokeColor);
    world0.x = selCentrePoint.x + plane.y;
    world0.y = selCentrePoint.y - plane.x;
    world1.x = selCentrePoint.x - plane.y;
    world1.y = selCentrePoint.y + plane.x;
    world1.z = selCentrePoint.z;
    primLine3(&world1, &world0, spokeColor);
    //pieOriginDraw(&selCentrePoint, pieOriginSizeCentre * distance, TW_MOVE_ORIGIN_COLOR);  //TW_MOVE_ORIGIN_COLOR tweakable global variable (tweak.*)
    //plane point circle
    pieScreenSizeOfCircleCompute(&piePlanePoint, selAverageSize, &scaledSize, &nSegments);
    primCircleOutlineZ(&piePlanePoint, scaledSize, nSegments, moveLineColor);
}

/*-----------------------------------------------------------------------------
    Name        : pieMovePointClipToLimits
    Description : Clips a point to an elliptical cylindrical clipping volume.
    Inputs      : sizeX, Y, Z - size of the elliptical clipping cylinder, in half-lengths
                  pointA - where you are moving from
                  pointB - where you are moving to, this is the point that will be clipped
    Outputs     : modifies pointB if a valid clipping point found; set to pointA if not.
                  In cases where there are two possible intersections (i.e.
                    pointA and B are outside the volume) the one closer to
                    pointB will be used.
    Return      : TRUE if a valid intersection found, FALSE otherwise
----------------------------------------------------------------------------*/
#define SGN(x)      ((x) < 0.0f ? -1.0f : 1.0f)
bool pieMovePointClipToLimits(real32 sizeX, real32 sizeY, real32 sizeZ, vector *pointA, vector *pointB)
{
    real32 discriminant;
    real32 dx, dy, drSquared, rSquared, D, DSquared;
    real32 x, y, x1, y1, x2, y2;
    real32 ellipseAspect;
    real32 fsqrtResult;

#if PIE_VISUALIZE_EXTENTS
    if (pieVisualizeExtents)
    {
        vector centre = {0, 0, pointB->z};

        centre.z = sizeZ;
        primEllipseOutlineZ(&centre, sizeX, sizeY, 64, colFuscia);
        centre.z = -sizeZ;
        primEllipseOutlineZ(&centre, sizeX, sizeY, 64, colFuscia);
        centre.z = pointB->z;
        primEllipseOutlineZ(&centre, sizeX, sizeY, 64, colWhite);
    }
#endif

    //simplify the problem by converting the problem to line-circle intersection
    //the circle will have a radius of the ellipses rx (sizeX)
    ellipseAspect = sizeY / sizeX;
    x1 = pointA->x;
    y1 = pointA->y / ellipseAspect;
    x2 = pointB->x;
    y2 = pointB->y / ellipseAspect;

    //base equations for line-circle intersection:
    dx = x2 - x1;
    dy = y2 - y1;
    drSquared = dx * dx + dy * dy;
    D = x1 * y2 - x2 * y1;
    DSquared = D * D;
    rSquared = sizeX * sizeX;
    discriminant = rSquared * drSquared - DSquared;

    if (discriminant < 0.0f)
    {                                                       //if no intersection
        pointB->x = pointA->x;
        pointB->y = pointA->y;
        return(FALSE);                                      //no clipping because no intersection
    }

    fsqrtResult = fsqrt(rSquared * drSquared - DSquared);
    x1 = (D * dy + SGN(dy) * dx * fsqrtResult) / drSquared;
    x2 = (D * dy - SGN(dy) * dx * fsqrtResult) / drSquared;
    y1 = (-D * dx + ABS(dy) * fsqrtResult) / drSquared;
    y2 = (-D * dx - ABS(dy) * fsqrtResult) / drSquared;

    //convert the computed results back into elliptical coordinates
    y1 *= ellipseAspect; y2 *= ellipseAspect;

    //see if that point was clipped
    if (pointB->x >= min(x1, x2) && pointB->x <= max(x1, x2) &&
        pointB->y >= min(y1, y2) && pointB->y <= max(y1, y2))
    {                                                       //if between the intersection points
        ;                                                   //points are fine
    }
    else
    {                                                       //else use whatever intersection closer to specified destination
        dx = pointB->x - x1;
        dy = pointB->y - y1;
        x = ABS(dx * dx + dy * dy);                         //x1,y1
        dx = pointB->x - x2;
        dy = pointB->y - y2;
        y = ABS(dx * dx + dy * dy);                         //x2,y2

        if (x > y)
        {                                                   //use x2,y2
            x1 = x2;                                        //x1,y1 is always the closer intersection
            y1 = y2;
        }
        pointB->x = x1;
        pointB->y = y1;
    }
    if (pointB->z > sizeZ)
    {
        pointB->z = sizeZ;
    }
    if (pointB->z < -sizeZ)
    {
        pointB->z = -sizeZ;
    }
    return(TRUE);
}
#undef SGN

/*-----------------------------------------------------------------------------
    Name        : piePointSpecDraw
    Description : Draw the point specification overlays
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void piePointSpecDraw(void)
{
    vector world0, world1;
    vector highPoint, lowPoint;
    GLdouble modelView[16], projection[16];
    hmatrix modelViewF, projectionF;
    real32 distance, deltaX, deltaY;
    real32 oldPointSpecZ, slope, b;
    vector moveLengthVector;
    vector preUpdateCentre;
    vector updateDrift;
    real32 moveLength, maxMoveLength;
    real32 windowHeightMinusOne;
    real32 eyeMagnitude;
    //static int oldMouseX, oldMouseY;
/*
    bool stipple;

    stipple = glCapFastFeature(GL_LINE_STIPPLE);
*/
    mousePoll();

    /*
    if (oldMouseX != mouseCursorX() || oldMouseY != mouseCursorY())
    {
        cameraRayCast(&world0, mrCamera, mouseCursorX(), mouseCursorY(), MAIN_WindowWidth, MAIN_WindowHeight);
        dbgMessagef("\nVec = %.3f %.3f %.3f", world0.x, world0.y, world0.z);
        oldMouseX = mouseCursorX();
        oldMouseY = mouseCursorY();
    }
    */
    if (pieDistReadFont == 0)
    {
        pieDistReadFont = frFontRegister(TW_DISTANCE_READOUT_FONT);
    }

    if (selSelected.numShips == 0)
    {
        piePointModeOnOff();
        return;
    }
    //before recomputing the selection centre point, get the relationship between selection centre, plane point and heightpoint
    preUpdateCentre = selCentrePoint;
    selCentrePointCompute();                                //get new centre of selection
    vecSub(updateDrift, selCentrePoint, preUpdateCentre);   //compute drift of centre point for error correction

    if (piePointSpecMouseReset)
    {                                                       //if we should reset location of mouse
        glGetFloatv(GL_PROJECTION_MATRIX,                   //get the matrices in hmatrix format
                    (float *)(&projectionF));
        glGetFloatv(GL_MODELVIEW_MATRIX,
                    (float *)(&modelViewF));
        if (piePointSpecMode == PSM_XY)
        {
            vecAdd(world0, piePlanePoint, updateDrift);
            vecAdd(world1, pieHeightPoint, updateDrift);
            pieMousePosSet3D(&modelViewF, &projectionF,     //set the position of the mouse
                                    &world0);
            if (pieSameOnScreenPoint(&modelViewF, &projectionF, &world0, &world1))
            {                                               //if they're still at the same spot on-screen
                pieHeightPoint = piePlanePoint;             //kill any vertical component
                piePointSpecZ = 0.0f;
            }
        }
        else
        {
            vecAdd(world0, pieHeightPoint, updateDrift);
            pieMousePosSet3D(&modelViewF, &projectionF,     //set the postion of the mouse
                                    &world0);
        }
        piePointSpecMouseReset = FALSE;                      //don't reset on next frame
    }

    windowHeightMinusOne = (real32)(MAIN_WindowHeight - 1);

    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);           //get the matrices

    world0.x = mrCamera->eyeposition.x - selCentrePoint.x;
    world0.y = mrCamera->eyeposition.y - selCentrePoint.y;
    world0.z = mrCamera->eyeposition.z - selCentrePoint.z;
    distance = fmathSqrt(vecMagnitudeSquared(world0));
    //prepare for drawing
    primModeClear2();
    rndLightingEnable(FALSE);                               //draw the pizza dish
    rndTextureEnable(FALSE);                                //disable texturing

    //compute world coords of mouse location
    if (piePointSpecMode != PSM_Waiting)
    {
        //compute mouse location in world space
        pieWorldLocationCompute(&world0, &world1, &eyeMagnitude);

        if (piePointSpecMode == PSM_XY)
        {                                                   //find point on pizza dish plane which represents mouse
            //ensure positive slope of intersection line
            if (mrCamera->eyeposition.z >= selCentrePoint.z || world1.z == world0.z)
            {                                               //if camera above selection plane
                if (world1.z >= world0.z)
                {
                    world1.z = world0.z - PIE_PlanePointFudge * eyeMagnitude;
                }
            }
            else
            {                                               //else camera below selection sphere
                if (world1.z <= world0.z)
                {
                    world1.z = world0.z + PIE_PlanePointFudge * eyeMagnitude;
                }
            }
            vecLineIntersectWithXYPlane(&piePlanePoint, &world0,
                                        &world1, selCentrePoint.z);
            pieHeightPoint.x = piePlanePoint.x;
            pieHeightPoint.y = piePlanePoint.y;
            pieHeightPoint.z += updateDrift.z;
        }
        else
        {                                                   //else find point on vertical line passing through point on pizza dish (PSM_Z)
            if (pieOnScreen)
            {                                               //ensure the vertical is at least partly on screen
                if ((real32)mouseCursorY() < (windowHeightMinusOne - pieScreen1.y))
                {                                               //if mouse off top of line
                    mousePositionSet(mouseCursorX(), (sdword)(windowHeightMinusOne - pieScreen1.y));
                }
                if ((real32)mouseCursorY() > (windowHeightMinusOne - pieScreen0.y))
                {                                               //if mouse off bottom of line
                    mousePositionSet(mouseCursorX(), (sdword)(windowHeightMinusOne - pieScreen0.y));
                }
                slope = (pieScreen0.x - pieScreen1.x) / (pieScreen1.y - pieScreen0.y);
                b = pieScreen1.x - slope * (windowHeightMinusOne - pieScreen1.y);
                mousePositionSet((sdword)(slope * mouseCursorY() + b), mouseCursorY());

                //mouse position was adjusted; recompute mouse location in world space
                pieWorldLocationCompute(&world0, &world1, &eyeMagnitude);
                /*
                if (oldMouseX != mouseCursorX() || oldMouseY != mouseCursorY())
                {
                    dbgMessagef("\nslope = %.2f b = %.2f", slope, b);
                    dbgMessagef("\nscreen0=(%.2f,%.2f) screen1=(%.2f,%.2f).", pieScreen0.x, pieScreen0.y, pieScreen1.x, pieScreen1.y);
                    dbgMessagef("\nworld0=(%.2f,%.2f,%.2f) world1=(%.2f,%.2f,%.2f).", world0.x, world0.y, world0.z, world1.x, world1.y, world1.z);
                    oldMouseX = mouseCursorX();
                    oldMouseY = mouseCursorY();
                }
                */
            }

            deltaX = mrCamera->eyeposition.x - (piePlanePoint.x + updateDrift.x);//find along which axis point is farther from camera
            deltaY = mrCamera->eyeposition.y - (piePlanePoint.y + updateDrift.y);
            if (deltaX > deltaY)
            {
                //ensure positive slope of intersection line
                if (mrCamera->eyeposition.x >= piePlanePoint.x + updateDrift.x || world1.x == world0.x)
                {                                           //camera on positive side of test plane
                    if (world1.x >= world0.x)
                    {
                        world1.x = world0.x - PIE_PlanePointFudge * eyeMagnitude;
                    }
                }
                else
                {
                    if (world1.x <= world0.x)
                    {
                        world1.x = world0.x + PIE_PlanePointFudge * eyeMagnitude;
                    }
                }
                vecLineIntersectWithYZPlane(&pieHeightPoint, &world0,
                                            &world1, piePlanePoint.x + updateDrift.x);
                //dbgMessagef("\\");
            }
            else
            {
                //ensure positive slope of intersection line
                if (mrCamera->eyeposition.y >= piePlanePoint.y + updateDrift.y || world1.y == world0.y)
                {                                           //camera on positive side of test plane
                    if (world1.y >= world0.y)
                    {
                        world1.y = world0.y - PIE_PlanePointFudge * eyeMagnitude;
                    }
                }
                else
                {
                    if (world1.y <= world0.y)
                    {
                        world1.y = world0.y + PIE_PlanePointFudge * eyeMagnitude;
                    }
                }
                vecLineIntersectWithXZPlane(&pieHeightPoint, &world0,
                                            &world1, piePlanePoint.y + updateDrift.y);
                //dbgMessagef("/");
            }
            /*
            if (oldMouseX != mouseCursorX() || oldMouseY != mouseCursorY())
            {
                dbgMessagef("\ndeltaX = %.2f deltaY = %.2f", deltaX, deltaY);
                dbgMessagef("\nheightPoint=(%.2f,%.2f,%.2f).", pieHeightPoint.x, pieHeightPoint.y, pieHeightPoint.z);
                dbgMessagef("\nworld0=(%.2f,%.2f,%.2f) world1=(%.2f,%.2f,%.2f).", world0.x, world0.y, world0.z, world1.x, world1.y, world1.z);
                oldMouseX = mouseCursorX();
                oldMouseY = mouseCursorY();
            }
            */
            //don't use this new value if the intersection test resulted in a bad number
            if (!isnan(pieHeightPoint.z))
            {
                //compute point along this line
                pieHeightPoint.x = piePlanePoint.x;         //compute height point relative to pizza dish
                pieHeightPoint.y = piePlanePoint.y;
                oldPointSpecZ = piePointSpecZ;
                piePointSpecZ = pieHeightPoint.z - piePlanePoint.z;//get height off pizza dish
            }
            vecAddTo(piePlanePoint, updateDrift);           //make sure the plane point does not drift
            //vecAddTo(pieHeightPoint, updateDrift);          //make sure the height point does not drift
        }
    }
    else
    {                                                       //in waiting mode: keep destination point from drifting
        vecAddTo(piePlanePoint, updateDrift);               //make sure the plane point does not drift
        vecAddTo(pieHeightPoint, updateDrift);              //make sure the height point does not drift
    }

    //recompute the on-screen vertical vector
    pieHeightPoint.x = piePlanePoint.x;                     //compute height point relative to pizza dish point
    pieHeightPoint.y = piePlanePoint.y;
    highPoint.x = lowPoint.x = piePlanePoint.x;
    highPoint.y = lowPoint.y = piePlanePoint.y;
    highPoint.z = smUniverseSizeZ;
    lowPoint.z = -smUniverseSizeZ;
    pieOnScreen = clipLineToScreen(&lowPoint, &highPoint, (real32 *)(&rndCameraMatrix),
                     (real32 *)(&rndProjectionMatrix), &pieScreen0, &pieScreen1);
    if (pieScreen0.y > pieScreen1.y)
    {                                               //if upside-down
        swapReal32(pieScreen0.x, pieScreen1.x);     //transpose the endpoints
        swapReal32(pieScreen0.y, pieScreen1.y);
    }

    //limit dest vector to render list extent
    if (!smSensorsActive)
    {
        maxMoveLength = pieMaxMoveHorizontal;
        moveLengthVector.x = pieHeightPoint.x - selCentrePoint.x;
        moveLengthVector.y = pieHeightPoint.y - selCentrePoint.y;
        moveLengthVector.z = 0.0f;
        moveLength = fmathSqrt(vecMagnitudeSquared(moveLengthVector));
        if (moveLength > maxMoveLength)
        {                                                   //if distance on the plane too far
            vecMultiplyByScalar(moveLengthVector, maxMoveLength / moveLength);
            vecAdd(piePlanePoint, selCentrePoint, moveLengthVector);
            pieHeightPoint.x = piePlanePoint.x;             //clip to the render list horizontally
            pieHeightPoint.y = piePlanePoint.y;
        }
        maxMoveLength = pieMaxMoveVertical;
        moveLength = ABS(pieHeightPoint.z - piePlanePoint.z);
        if (moveLength > maxMoveLength)
        {                                                   //if moved too high
            pieHeightPoint.z = piePlanePoint.z + (pieHeightPoint.z - piePlanePoint.z) * maxMoveLength / moveLength;
        }
    }

    //now limit end point to the 'hard' limits of the universe.
    //After all, the universe has hard limits you know.
    pieMovePointClipToLimits(smUniverseSizeX, smUniverseSizeY, smUniverseSizeZ, &selCentrePoint, &pieHeightPoint);
    piePlanePoint.x = pieHeightPoint.x;
    piePlanePoint.y = pieHeightPoint.y;
    piePlanePoint.z = selCentrePoint.z;
    pieDistanceReadoutDraw(&pieHeightPoint, &selCentrePoint, TW_MOVE_ORIGIN_COLOR);

    //draw the pizza dish with spokes
    if (piePlaneDrawCallback != NULL)
    {
        piePlaneDrawCallback(distance);
    }
    //now draw lines and cross-hairs
    if (pieMovementCursorDrawCallback != NULL)
    {
        pieMovementCursorDrawCallback(distance);
    }
    //draw lines to pie plate
    if (pieLineDrawCallback != NULL)
    {
        pieLineDrawCallback(distance);
    }
    //fraw the horizon
    if (mrRenderMainScreen)
    {
        smCurrentWorldPlaneColor = colMultiplyClamped(smWorldPlaneColor, smMovementWorldPlaneDim);
        smHorizonLineDraw(mrCamera, &rndCameraMatrix, &rndProjectionMatrix, mrCamera->clipPlaneFar - mrCamera->farthestZoom);
    }
    rndLightingEnable(FALSE);
    primModeSet2();

#if PIE_MOVE_NEARTO
    //make the ship under cursor glow as a destination of the 'move near to' command
    if (selClosestTarget != NULL)
    {                                                       //if there is a ship under cursor
        selClosestTarget->flashtimer = taskTimeElapsed+FLASH_TIMER/2;                  //just glow for the next frame only
    }
#endif //PIE_MOVE_NEARTO
}

/*-----------------------------------------------------------------------------
    Name        : pieShipDied
    Description : cleans up main region stuff when a ship dies
    Inputs      :
    Outputs     :
    Return      :
    Note        : call only after ship has been removed from selSelected, selSelecting
----------------------------------------------------------------------------*/
void pieShipDied(struct Ship *ship)
{
    if (pieLastClosestShip == ship)
    {
        pieLastClosestShip = NULL;
    }

    if ((selSelected.numShips == 0) && (piePointSpecMode != PSM_Idle))
    {
        piePointModeOnOff();
    }
}

/*-----------------------------------------------------------------------------
    Name        : piePointModeToggle
    Description : Toggles between xy-mode and z-mode for point specification
    Inputs      : bOn - if true, set Z mode, XY mode otherwise
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void piePointModeToggle(sdword bOn)
{
    if (bOn && piePointSpecMode == PSM_XY)
    {
        if (!keyIsHit(CONTROLKEY))
        {
            tutGameMessage("Game_MoveZ");
            piePointSpecMode = PSM_Z;
            if (piePointSpecZ == 0.0f)
            {                                               //if height point not yet specified
                pieHeightPoint = piePlanePoint;
            }
        }
        else
        {
            pieHeightPoint = piePlanePoint;
            piePointSpecZ = 0.0f;
        }
    }
    else if (!bOn && piePointSpecMode == PSM_Z)
    {
        tutGameMessage("Game_MoveXY");
        piePointSpecMode = PSM_XY;
    }
    piePointSpecMouseReset = TRUE;                           //reset location of mouse cursor
}
