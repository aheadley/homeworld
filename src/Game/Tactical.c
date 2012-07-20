/*=============================================================================
    Name    : Tactical.c
    Purpose : Logic for the tactical overlay

    Created 8/8/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <strings.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "StatScript.h"
#include "SpaceObj.h"
#include "Universe.h"
#include "Teams.h"
#include "Vector.h"
#include "Tactical.h"
#include "Ships.h"
#include "main.h"
#include "utility.h"
#include "render.h"
#include "Alliance.h"
#include "Sensors.h"
#include "Blobs.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

extern fonthandle selGroupFont2;

//located in mainrgn.c
void toMoveLineDraw(ShipPtr ship, real32 scale);
void toFieldSphereDraw(ShipPtr ship,real32 radius, real32 scale,color passedColour);
void toDrawRadialIndicator1(ShipPtr ship, real32 radius, real32 scale,color passedColour);
void toDrawRadialIndicator2(ShipPtr ship, real32 radius, real32 scale,color passedColour);
void toDrawRadialIndicator3(ShipPtr ship, real32 radius, real32 scale,color passedColour);
void toDrawRadialIndicator4(ShipPtr ship, real32 radius, real32 scale,color passedColour);

real32 mineMineTOSize;
real32 toMinimumSize;

/*=============================================================================
    Data:
=============================================================================*/
//basic, default TO icon
toicon toDefaultIcon =
{
    0, {{0.0f, 0.0f}}
};

//to icons for all classes of ships + 1 for mines
toicon *toClassIcon[NUM_CLASSES+1];

//colors for tactical overlay of various players
color toPlayerColor[TO_NumPlayers];

//for legend: track if a particular class is currently displayed (for each player)
bool8 toClassUsed[NUM_CLASSES+1][TO_NumPlayers];

void toNewClassSet(char *directory,char *field,void *dataToFillIn);
void toNumberPointsSet(char *directory,char *field,void *dataToFillIn);
void toVertexAdd(char *directory,char *field,void *dataToFillIn);
scriptEntry toIconTweaks[] =
{
    { "IconClass",      toNewClassSet,  NULL},
    { "nPoints",        toNumberPointsSet, NULL},
    { "vertex",         toVertexAdd,    NULL},
    { "playerColor0",   scriptSetRGBCB, &toPlayerColor[0]},
    { "playerColor1",   scriptSetRGBCB, &toPlayerColor[1]},
    { "playerColor2",   scriptSetRGBCB, &toPlayerColor[2]},
    { "playerColor3",   scriptSetRGBCB, &toPlayerColor[3]},
    { "playerColor4",   scriptSetRGBCB, &toPlayerColor[4]},
    { "playerColor5",   scriptSetRGBCB, &toPlayerColor[5]},
    { "playerColor6",   scriptSetRGBCB, &toPlayerColor[6]},
    { "playerColor7",   scriptSetRGBCB, &toPlayerColor[7]},
    { NULL, NULL, 0 }
};

//retained integrals used during loading
ShipClass toCurrentClass = CLASS_Mothership;
sdword toCurrentPoint = 0;

// x,y location for the player list in the main tactical overlay
sdword TO_PLAYERLIST_Y = 480;
sdword TO_PLAYERLIST_X = 10;

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : toStartup
    Description : Startup TO module.
    Inputs      : void
    Outputs     : Initializes the toClassIcon array.
    Return      : void
----------------------------------------------------------------------------*/
void toStartup(void)
{
    sdword index;

    mineMineTOSize = primScreenToGLScaleX(TO_MineMinimumScreenSize);
    toMinimumSize  = primScreenToGLScaleX(TO_MinimumScreenSize);

    for (index = 0; index < (NUM_CLASSES+1); index++)
    {                                                       //set all icons to default
        toClassIcon[index] = &toDefaultIcon;
    }
    scriptSet(NULL, "Tactical.script", toIconTweaks);
}

/*-----------------------------------------------------------------------------
    Name        : toShutdown
    Description : Shut down the TO module.
    Inputs      : void
    Outputs     : Frees the toIconTweaks array.
    Return      : void
----------------------------------------------------------------------------*/
void toShutdown(void)
{
    sdword index;

    for (index = 0; index < (NUM_CLASSES+1); index++)
    {                                                       //set all icons to default
        if (toClassIcon[index] != &toDefaultIcon && toClassIcon[index] != NULL)
        {
            memFree(toClassIcon[index]);
        }
    }
}

/*-----------------------------------------------------------------------------
    Script-read functions to load in the TO icons
-----------------------------------------------------------------------------*/
void toNewClassSet(char *directory,char *field,void *dataToFillIn)
{                                                           //set new ship class
    if (strcasecmp(field,"CLASS_Mine") == 0)
    {
        toCurrentClass = NUM_CLASSES;   // use NUM_CLASSES to indicate mine
    }
    else
    {
        toCurrentClass = StrToShipClass(field);
    }
    dbgAssert(toCurrentClass < (NUM_CLASSES+1));
}
void toNumberPointsSet(char *directory,char *field,void *dataToFillIn)
{                                                           //set number of points and allocate structure
    sdword nPoints, nScanned;

    nScanned = sscanf(field, "%d", &nPoints);               //parse number of points
    dbgAssert(nScanned == 1);
    dbgAssert(nPoints > 0 && nPoints < 32);                 //!!! 32 is some arbitrary number
    toClassIcon[toCurrentClass] = memAlloc(toIconSize(nPoints),//allocate the icon structure
                                           "Tactical Overlay Icon", NonVolatile);
    toClassIcon[toCurrentClass]->nPoints = nPoints;
    toCurrentPoint = 0;                                     //no points loaded yet
}
void toVertexAdd(char *directory,char *field,void *dataToFillIn)
{
    real32 x, y;
    sdword nScanned;

    dbgAssert(toCurrentPoint < toClassIcon[toCurrentClass]->nPoints);//make sure not too many points
    nScanned = sscanf(field, "%f,%f", &x, &y);              //scan in the x/y
    dbgAssert(nScanned == 2);                               //make sure we got 2 numbers
    toClassIcon[toCurrentClass]->loc[toCurrentPoint].x =    //copy the points over
        x * TO_VertexScanFactorX;                           //multiplied by a scaling factor
    toClassIcon[toCurrentClass]->loc[toCurrentPoint].y =
        y * TO_VertexScanFactorY;
    toCurrentPoint++;                                       //next point
}

void toFieldSphereDrawGeneral(vector position, real32 radius,color passedColour);

/*-----------------------------------------------------------------------------
    Name        : toAllShipsDraw
    Description : Draw tactical overlays for all ships.
    Inputs      :
    Outputs     : Draw a tactical display icon overlay for all on-screen ships
    Return      : void
----------------------------------------------------------------------------*/
void toAllShipsDraw(void)
{
    Node *objnode = universe.RenderList.head;
    Ship *ship;
    real32 radius, scale;
    sdword index, player;
    toicon *icon;
    vector zero = {0,0,0};
    color c;
    udword intScale;

     // reset usage of classes (for legend)
    for (index = 0; index < (NUM_CLASSES+1); index++)
    {
          for (player = 0; player < TO_NumPlayers; ++player)
          {
               toClassUsed[index][player] = 0;
          }
    }

    rndTextureEnable(FALSE);
    rndLightingEnable(FALSE);

    while (objnode != NULL)
    {
          ship = (Ship *) listGetStructOfNode(objnode);

        if (ship->flags & SOF_Dead)
        {
            goto nextnode;
        }

        if (ship->collInfo.selCircleRadius <= 0.0f)
        {                                                   //if behind camera
            goto nextnode;                                  //don't draw it
        }

        if(ship->flags & SOF_Slaveable)
        {
            if(ship->slaveinfo->flags & SF_SLAVE)
            {
                goto nextnode;          //don't draw TO for SLAVES of some master ship
            }
        }

        if (ship->objtype == OBJ_MissileType)
        {
#define mine ((Missile *)ship)
            if (mine->missileType != MISSILE_Mine)
            {
                goto nextnode;
            }

            if (mine->playerowner != universe.curPlayerPtr)
            {
                goto nextnode;      // only show player's own mines
            }

            radius = mine->collInfo.selCircleRadius;

            if (radius < TO_MaxFadeSize)
            {

                //Get an index of the player to get the color.
#if TO_STANDARD_COLORS
                if (mine->playerowner == universe.curPlayerPtr)
                {
                    c = teFriendlyColor;
                }
                else if (allianceArePlayersAllied(mine->playerowner, universe.curPlayerPtr))
                {
                    c = teAlliedColor;
                }
                else
                {
                    c = teHostileColor;
                }
#else
                c = teColorSchemes[mine->colorScheme].tacticalColor;
#endif

                if (radius > TO_FadeStart)
                {
                    // scale the color - start fading out the overlay
                    scale = (radius - TO_MinFadeSize) * TO_FadeRate;
                    intScale = (udword)scale;
                    c = colRGB(colRed(c) / intScale, colGreen(c) / intScale, colBlue(c) / intScale);
                }

                icon = toClassIcon[NUM_CLASSES];    // NUM_CLASSES indicates Mine

                primLineLoopStart2(1, c);

                radius = max(mineMineTOSize, radius);   //ensure radius not too small

                for (index = icon->nPoints - 1; index >= 0; index--)
                {
                    primLineLoopPoint3F(mine->collInfo.selCircleX + icon->loc[index].x * radius,
                                        mine->collInfo.selCircleY + icon->loc[index].y * radius);
                }

                primLineLoopEnd2();
            }

        }
        else if (ship->objtype == OBJ_ShipType)
        {
            if (bitTest(ship->flags,SOF_Cloaked) && ship->playerowner != universe.curPlayerPtr)
            {                                                   //if cloaked, and doesn't belong to current player
                if(!proximityCanPlayerSeeShip(universe.curPlayerPtr,ship))
                {
                    goto nextnode;                                  //don't draw it
                }
            }

            if (ship->shiptype == Drone)
            {
                goto nextnode;
            }

            // tactical overlay (fades out when the ship gets large enough)
            radius = ship->collInfo.selCircleRadius * ship->staticinfo->tacticalSelectionScale;

            if (radius < TO_MaxFadeSize)
            {
                //Get an index of the player to get the color.
#if TO_STANDARD_COLORS
                if (ship->playerowner == universe.curPlayerPtr)
                {
                    c = teFriendlyColor;
                }
                else if (allianceIsShipAlly(ship, universe.curPlayerPtr))
                {
                    c = teAlliedColor;
                }
                else
                {
                    c = teHostileColor;
                }
#else
                c = teColorSchemes[ship->colorScheme].tacticalColor;
#endif

                if (radius > TO_FadeStart)
                {
                    // scale the color - start fading out the overlay
                    scale = 255.0f - ((radius - TO_MinFadeSize) * TO_FadeRate);
                    intScale = (udword)scale;
                    c = colRGB((colRed(c) * intScale) >> 8, (colGreen(c) * intScale) >> 8, (colBlue(c) * intScale) >> 8);
                }

                icon = toClassIcon[((ShipStaticInfo *)ship->staticinfo)->shipclass];
                primLineLoopStart2(1, c);

                radius = max(toMinimumSize, radius);   //ensure radius not too small

                for (index = icon->nPoints - 1; index >= 0; index--)
                {
                    primLineLoopPoint3F(ship->collInfo.selCircleX + icon->loc[index].x * radius,
                                        ship->collInfo.selCircleY + icon->loc[index].y * radius);
                }

                primLineLoopEnd2();

                     // mark class as used for this player (for legend)
                     toClassUsed[((ShipStaticInfo *)ship->staticinfo)->shipclass][ship->playerowner->playerIndex] = 1;
            }

            //for moving ships that belong to the current player, draw the moveline
            if ((!vecAreEqual(ship->moveTo, zero)) && (ship->playerowner == universe.curPlayerPtr))
            {
                toMoveLineDraw(ship, scale);
            }

            //Draw Special TO's for Special Ships
            if(ship->playerowner == universe.curPlayerPtr)
            {
                switch(ship->shiptype)
                {
                case CloakGenerator:
                    if( ((CloakGeneratorSpec *)ship->ShipSpecifics)->CloakOn)
                    {
                        toFieldSphereDraw(ship,((CloakGeneratorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo)->CloakingRadius, scale,TW_CLOAKGENERATOR_SPHERE_COLOUR);
                    }
                    break;
                case DFGFrigate:
                    toFieldSphereDraw(ship,((DFGFrigateStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo)->DFGFrigateFieldRadius, scale,TW_DFGF_SPHERE_COLOUR);
                    break;
                case GravWellGenerator:
                    if (((GravWellGeneratorSpec *)ship->ShipSpecifics)->GravFieldOn)
                    {
                        toFieldSphereDraw(ship,((GravWellGeneratorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo)->GravWellRadius, scale,TW_GRAVWELL_SPHERE_COLOUR);
                    }
                    break;
                case ProximitySensor:
                    //toFieldSphereDraw(ship,((ProximitySensorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo)->SearchRadius, scale);
                   if( ((ProximitySensorSpec *)ship->ShipSpecifics)->sensorState == SENSOR_SENSED
                        || ((ProximitySensorSpec *)ship->ShipSpecifics)->sensorState == SENSOR_SENSED2)
                   {
                       toDrawRadialIndicator4(ship,((ProximitySensorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo)->SearchRadius, scale,TW_PROXIMITY_RING_COLOUR);
                       toFieldSphereDraw(ship,((ProximitySensorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo)->SearchRadius, scale,TW_PROXIMITY_SPHERE_COLOUR_FOUND);
                   }
                   else
                   {
                       toFieldSphereDraw(ship,((ProximitySensorStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo)->SearchRadius, scale,TW_PROXIMITY_SPHERE_COLOUR);
                   }
                }
            }
        }

nextnode:
        objnode = objnode->next;
    }

#define DEBUG_DRAW_BLOBS 0

#if DEBUG_DRAW_BLOBS


    {
        blob *Blob;

        objnode = universe.collBlobList.head;
        while (objnode != NULL)
        {
            Blob = (blob *)listGetStructOfNode(objnode);

            toFieldSphereDrawGeneral(Blob->centre,Blob->radius,colReddish);

            objnode = objnode->next;
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : toLegendDraw
    Description : Draw tactical overlay legend.
    Inputs      : toClassUsed is set with a previous call to toAllShipsDraw()
    Outputs     : Draw a legend for the tactical display icons.
    Return      : void
----------------------------------------------------------------------------*/
void toLegendDraw(void)
{
    sdword rowHeight, shipClass, pl, y, index, x;
    toicon *icon;
    color col;
    real32 radius;
    fonthandle fhSave;
    rectangle playerColorRect;

    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    rowHeight = fontHeight(" "); // used to space the legend
    y = rowHeight;
    radius = primScreenToGLScaleX(rowHeight)/2;
    x = (sdword)(rowHeight * 2.5);

#if TO_STANDARD_COLORS
    col = teNeutralColor;
#else
    col = teColorSchemes[universe.curPlayerIndex].tacticalColor;
#endif
    col = colRGB(colRed(col)/TO_IconColorFade, colGreen(col)/TO_IconColorFade, colBlue(col)/TO_IconColorFade);

    // draw legend entries for any classes currently displayed
    for (shipClass = CLASS_Mothership+1; shipClass <= CLASS_NonCombat; ++shipClass)
    {
        for (pl = 0; pl < TO_NumPlayers; ++pl)
        {
            if (toClassUsed[shipClass][pl])
                break;
        }
        if (pl < TO_NumPlayers)
        {
            // icon
            icon = toClassIcon[shipClass];
            primLineLoopStart2(1, col);

            for (index = icon->nPoints - 1; index >= 0; index--)
            {
               primLineLoopPoint3F(primScreenToGLX(rowHeight*1.5) + icon->loc[index].x * radius,
                                          primScreenToGLY(y + rowHeight/2) + icon->loc[index].y * radius);
            }
            primLineLoopEnd2();

            // text
            fontPrint(x, y,
                   TO_TextColor,
                   ShipClassToNiceStr((ShipClass)shipClass));
            y += rowHeight + 1;
        }
    }

    // draw player names in the bottom left of the screen.

    if (!singlePlayerGame)
    {
        playerColorRect.x0 = TO_PLAYERLIST_X;
        playerColorRect.x1 = MAIN_WindowWidth;
        playerColorRect.y0 = 0;
        playerColorRect.y1 = MAIN_WindowHeight-TO_PLAYERLIST_Y;

        smPlayerNamesDraw(&playerColorRect);
    }

    fontMakeCurrent(fhSave);
}
