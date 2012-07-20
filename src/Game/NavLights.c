/*=============================================================================
    Name    : NavLights.c
    Purpose : Control operation of NAV lights on ships.

    Created 6/21/1997 by agarden
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#ifdef _WIN32
#include <windows.h>
#endif
#endif
#include "NavLights.h"
#include "SpaceObj.h"
#include "Matrix.h"
#include "render.h"
#include "prim3d.h"
#include "Universe.h"
#include "glinc.h"
#include "glcaps.h"


/*-----------------------------------------------------------------------------
    Name        : navLightBillboardEnable
    Description : setup modelview matrix for rendering billboarded sprites
    Inputs      : s - ship to obtain coordinate system from
                  nls - NAV Light static info
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void navLightBillboardEnable(Ship *s, NAVLightStatic *nls)
{
    vector src, dst;

    //object -> worldspace
    src = nls->position;
    matMultiplyMatByVec(&dst, &s->rotinfo.coordsys, &src);
    vecAddTo(dst, s->posinfo.position);

    //setup billboarding
    glPushMatrix();
    rndBillboardEnable(&dst);
    glDisable(GL_CULL_FACE);
}

/*-----------------------------------------------------------------------------
    Name        : navLightBillboardDisable
    Description : reset modelview matrix to non-billboard state
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void navLightBillboardDisable(void)
{
    //undo billboarding
    rndBillboardDisable();
    glPopMatrix();
    glEnable(GL_CULL_FACE);
}

/*-----------------------------------------------------------------------------
    Name        : navLightStaticInfoDelete
    Description : Delete the static info of a set of nav lights
    Inputs      : staticInfo - array of navlight structures to free
    Outputs     : unregisters the navlight textures, if any.
    Return      :
----------------------------------------------------------------------------*/
void navLightStaticInfoDelete(NAVLightStaticInfo *staticInfo)
{
    sdword i, num = staticInfo->numNAVLights;
    NAVLightStatic *navLightStatic = staticInfo->navlightstatics;

    dbgAssert(staticInfo != NULL);

    for( i=0 ; i < num ; i++, navLightStatic++)
    {
        if (navLightStatic->texturehandle != TR_InvalidHandle)
        {
            trTextureUnregister(navLightStatic->texturehandle);
        }
    }
    memFree(staticInfo);
}

/*-----------------------------------------------------------------------------
    Name        : RenderNAVLights
    Description : TODO: render sorted by projected depth value so alpha sorts correctly
    Inputs      : ship - the ship whose navlights we are to render
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RenderNAVLights(Ship *ship)
{
   sdword i;
   NAVLight *navLight;
   NAVLightInfo *navLightInfo;
   ShipStaticInfo *shipStaticInfo;
   NAVLightStatic *navLightStatic;
   vector origin = {0.0f, 0.0f, 0.0f};
   NAVLightStaticInfo *navLightStaticInfo;
   real32 fade;
   bool lightOn;
   extern bool bFade;
   extern real32 meshFadeAlpha;

   fade = bFade ? meshFadeAlpha : 1.0f;

   shipStaticInfo = (ShipStaticInfo *)ship->staticinfo;

    navLightInfo = ship->navLightInfo;
   if(shipStaticInfo->navlightStaticInfo && navLightInfo != NULL)
   {
      glDepthMask(GL_FALSE);
      rndAdditiveBlends(TRUE);
      lightOn = rndLightingEnable(FALSE);

      navLightStaticInfo = shipStaticInfo->navlightStaticInfo;
      navLightStatic = navLightStaticInfo->navlightstatics;
      navLight = navLightInfo->navLights;

      for( i=0 ; i<navLightStaticInfo->numNAVLights ; i++, navLight ++, navLightStatic ++)
      {
			// Account for the startdelay.
			if(navLight->lastTimeFlashed == navLightStatic->startdelay)
			{
				navLight->lastTimeFlashed = universe.totaltimeelapsed + navLightStatic->startdelay;
			}
			
			if(universe.totaltimeelapsed > navLight->lastTimeFlashed)
			{
				if(navLight->lightstate == 1)
				{
					navLight->lastTimeFlashed = universe.totaltimeelapsed + navLightStatic->flashrateoff;
				}
				else
				{
					navLight->lastTimeFlashed = universe.totaltimeelapsed + navLightStatic->flashrateon;
				}
				
				navLight->lightstate = 1 - navLight->lightstate;
			}

			if(navLight->lightstate)
			{
				if (ship->currentLOD <= (sdword)navLightStatic->minLOD)
				{
					navLightBillboardEnable(ship, navLightStatic);

					if(navLightStatic->texturehandle == TR_InvalidHandle)
					{
						primCircleSolid3Fade(&origin, navLightStatic->size, 10, navLightStatic->color, fade);
					}
					else
					{
						primSolidTexture3Fade(&origin, navLightStatic->size, navLightStatic->color, navLightStatic->texturehandle, fade);
					}

					navLightBillboardDisable();
				}
				else
				{
					color tempColor;

                    tempColor = colRGB(colRed(navLightStatic->color) * 2 / 3,
					                   colGreen(navLightStatic->color) * 2 / 3,
									   colBlue(navLightStatic->color) * 2 / 3);

                    rndTextureEnable(FALSE);
                    if (RGL)
                    {
                        if (glCapFastFeature(GL_BLEND))
                        {
                            rndAdditiveBlends(TRUE);
                            primPointSize3(&navLightStatic->position, 2.0f, tempColor);
                        }
                        else
                        {
                            primPointSize3(&navLightStatic->position, 1.0f, tempColor);
                        }
                    }
                    else
                    {
                        rndAdditiveBlends(TRUE);
                        glEnable(GL_POINT_SMOOTH);
                        primPointSize3Fade(&navLightStatic->position, 2.0f, tempColor, fade);
                        glDisable(GL_POINT_SMOOTH);
                    }
				}
			}
      }

      rndLightingEnable(lightOn);
      rndAdditiveBlends(FALSE);
      glDepthMask(GL_TRUE);
    }
}

