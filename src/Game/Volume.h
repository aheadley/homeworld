#ifndef __VOLUME_H
#define __VOLUME_H

typedef enum {
        VOLUME_SPHERE,  // sphere
        VOLUME_AA_BOX,  // axis-aligned box
} VolumeType;

// sphere
typedef struct {
    vector center;
    real32 radius;
} VolumeSphere;

// axis-aligned box -- IMPORTANT: x0 <= x1, y0 <= y1, z0 <= z1
typedef struct {
    real32 x0, x1, y0, y1, z0, z1;
} VolumeAABox;

// generic volume
typedef struct {
        VolumeType type;
        union {
            VolumeSphere sphere;
            VolumeAABox aaBox;
        } attribs;
} Volume;

sdword volPointInside(Volume *vol, vector *point);
sdword volSphereIntersection(Volume vol, vector center, real32 radius);
vector volFindCenter(Volume *vol);
real32 volFindRadius(Volume *vol);

#endif
