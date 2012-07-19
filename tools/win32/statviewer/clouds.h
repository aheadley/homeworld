/*=============================================================================
    Name    : clouds.h
    Purpose : dust/gas cloud resource data structure management stuff

    Created 2/8/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _CLOUDS_H
#define _CLOUDS_H

#include "types.h"
#include "vector.h"
#include "matrix.h"
#include "color.h"
#include "objtypes.h"

/*
 * STRUCTURES
 */

#define LIGHTNING_ACTIVE        1
#define LIGHTNING_MAINSEGMENT   2
#define LIGHTNING_DISCHARGING   4

typedef ubyte lhandle;  //lightning handle type

#define MAX_LIGHTNINGS      256
#define MAX_LIGHTNING_PALS  3
#define MAX_BURSTS          3

typedef struct
{
    udword      flags;
    real32      radius;

    //system data.
    //pointers to the resource (spaceobj) object
    vector*     position;
    matrix*     rotation;

    sdword      maxhealth;

    real32      healthFactor;       //[0..1]

    color       cloudColor;

    real32      charge;             //[0..1]

    lhandle     bursts[MAX_BURSTS];
} cloudSystem;

//lightning acts in localspace of the guy who spawned them
typedef struct
{
    udword  flags;

    vector  pointA;
    vector  pointB;
    real32  distance;   //max distance of burst, may spawn to meet constraint

    //main segments need to know the configuration of the cloud they come from
    cloudSystem* parent;

    lhandle pals[MAX_LIGHTNING_PALS];   //can have n little lightnings spawned

    ubyte   countdown;  //ticks left to live
} lightning;

//set if there are clouds in the current mission.
//cloud fog overrides nebula fog
extern bool8 dontNebulate;

//lightning 0 is unusable
extern lightning _lightnings[MAX_LIGHTNINGS];

//ellipse stuff
typedef struct { real32 x, y, z; } point;
typedef struct { point p, n; } vertex;
typedef struct { int v0, v1, v2; } face;
typedef struct
{
    sdword nv, nf;
    vertex* v;
    face* f;
} ellipseObject;

extern ellipseObject genericEllipse[4];
void ellipsoid_render(ellipseObject* ellipse, real32 radius);
void ellipsoid_render_as(ellipseObject* ellipse, int what, vector* offset);
void ellipsoid_render_as_radii(ellipseObject* ellipse, int what, vector* offset, real32 ra, real32 rb, real32 rc);

/*
 * FUNCTIONS
 */

//load textures, &c
void cloudStartup(void);

//free textures, &c
void cloudShutdown(void);

//reset things
void cloudReset(void);

//create a cloud system with specified radius and given variance
cloudSystem* cloudCreateSystem(real32 radius, real32 variance);

//convenient
void cloudDeleteSystem(cloudSystem* system);

//render a cloud system
void cloudRenderSystem(void* mesh, cloudSystem* system, sdword lod);

//set fog based on camera distance to dust clouds
void cloudSetFog(void);

//ship's lightning render & think fn
void cloudRenderAndUpdateLightning(lightning* l, sdword lod);

//allocate a fresh lightning suitable for a ship
lightning* cloudNewLightning(void* system, real32 radius);

//------------ lightning

lhandle cloudGetFreshLightningHandle();
lightning* cloudGetFreshLightning();
void cloudInitLightning(lightning* l);
void cloudInitLightningHandle(lhandle handle);
lightning* cloudLightningFromHandle(lhandle handle);
void cloudKillLightning(lightning* l);
void cloudLiveLightning(lightning* l);
void cloudMainLightning(lightning* l);
void cloudRandomSphericalPoint(vector* vec);

/*=============================================================================
    Public Data:
=============================================================================*/

extern real32 DUSTCLOUD_RADIAL_SCALE;
extern real32 DUSTCLOUD_RADII[NUM_DUSTCLOUDTYPES];

#endif

