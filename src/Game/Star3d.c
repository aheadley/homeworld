/*=============================================================================
    Name    : Star3d.c
    Purpose : Handles the 3-D Stars

    Created 6/21/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <math.h>
#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "Memory.h"
#include "Star3d.h"
#include "Randy.h"

/*=============================================================================
    Data:
=============================================================================*/
sdword starMaxColor;
sdword starMinColor;
sdword starRedVariance;
sdword starGreenVariance;
sdword starBlueVariance;

/*=============================================================================
    Private functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : Make3dStar
    Description : Makes a 3d-star with random position, color, and brightness
    Inputs      : innerlimit, outerlimit (distances from origin)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void Make3dStar(Star3d *star,real32 innerlimit,real32 outerlimit)
{
    real32 distance = frandyrandombetween(0,innerlimit,outerlimit);
    real32 rotaboutx = frandyrandom(0,2*PI);
    real32 rotabouty = frandyrandom(0,2*PI);
    real32 rotaboutz = frandyrandom(0,2*PI);
    matrix rotxmat;
    matrix rotymat;
    matrix rotzmat;
    vector position1 = { 0.0f,0.0f,0.0f };
    vector position2;
    udword colorComponent, red, green, blue;

    position1.z = distance;

    matMakeRotAboutX(&rotxmat,(real32)cos(rotaboutx),(real32)sin(rotaboutx));
    matMakeRotAboutY(&rotymat,(real32)cos(rotabouty),(real32)sin(rotabouty));
    matMakeRotAboutZ(&rotzmat,(real32)cos(rotaboutz),(real32)sin(rotaboutz));

    matMultiplyMatByVec(&position2,&rotxmat,&position1);
    matMultiplyMatByVec(&position1,&rotymat,&position2);
    matMultiplyMatByVec(&star->position,&rotzmat,&position1);

    star->brightness = frandyrandom(0,1.0f);

    //compute star color with an random intensity and random variance
    colorComponent = randyrandom(0,starMaxColor - starMinColor) + starMinColor;
    red = colorComponent + randyrandom(0,starRedVariance * 2) - starRedVariance;
    red = min(255, max(0, red));
    green = colorComponent + randyrandom(0,starGreenVariance * 2) - starGreenVariance;
    green = min(255, max(0, green));
    blue = colorComponent + randyrandom(0,starBlueVariance * 2) - starBlueVariance;
    blue = min(255, max(0, blue));
    star->c = colRGB(red, green, blue);
    star->r = (ubyte)red;
    star->g = (ubyte)green;
    star->b = (ubyte)blue;
}

/*=============================================================================
    Public functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : star3dInit
    Description : Creates a 3-D star system
    Inputs      : number of stars to create, distance from origin to stars
                  innerlimits and outerlimits
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Star3dInfo *star3dInit(udword numStars,real32 innerlimit,real32 outerlimit)
{
    Star3dInfo *star3dinfo;
    udword i;

    star3dinfo = memAlloc(sizeof(Star3dInfo) + (numStars-1)*sizeof(Star3d),"Star3dInfo",NonVolatile);
    star3dinfo->Num3dStars = numStars;
    star3dinfo->innerlimit = innerlimit;
    star3dinfo->outerlimit = outerlimit;

    for (i=0;i<numStars;i++)
    {
        Make3dStar(&star3dinfo->Stars[i],innerlimit,outerlimit);
    }

    return star3dinfo;
}

/*-----------------------------------------------------------------------------
    Name        : star3dClose
    Description : Destroys 3-D star system
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void star3dClose(Star3dInfo *star3dinfo)
{
    memFree(star3dinfo);
}

