#include "AIUtilities.h"
#include "Vector.h"
#include "Volume.h"

//
//  returns true if the point is in the volume
//
sdword volPointInside(Volume *vol, vector *point)
{
    real32 radiusSquared;
    switch (vol->type)
    {
        case VOLUME_AA_BOX:
            return (point->x >= vol->attribs.aaBox.x0 &&
                    point->x <= vol->attribs.aaBox.x1 &&
                    point->y >= vol->attribs.aaBox.y0 &&
                    point->y <= vol->attribs.aaBox.y1 &&
                    point->z >= vol->attribs.aaBox.z0 &&
                    point->z <= vol->attribs.aaBox.z1);

        case VOLUME_SPHERE:
            radiusSquared = vol->attribs.sphere.radius * vol->attribs.sphere.radius;
            return aiuFindDistanceSquared(*point, vol->attribs.sphere.center) <= radiusSquared;

        default:
            return 0;
    }
}


/*-----------------------------------------------------------------------------
    Name        : volSphereIntersection
    Description : Returns true if the volume intersects the sphere
    Inputs      : volume - the volume,
                  center, radius - parameters defining the sphere
    Outputs     :
    Return      : TRUE if the volume and the sphere intersect
----------------------------------------------------------------------------*/
#define SQR(x) (x)*(x)
sdword volSphereIntersection(Volume vol, vector center, real32 radius)
{
    real32 volDistSq, dmin = 0;

    switch (vol.type)
    {
        case VOLUME_AA_BOX:
            if (center.x < min(vol.attribs.aaBox.x0, vol.attribs.aaBox.x1))
                dmin += SQR(center.x - min(vol.attribs.aaBox.x0, vol.attribs.aaBox.x1));
            else if (center.x > max(vol.attribs.aaBox.x0, vol.attribs.aaBox.x1))
                dmin += SQR(center.x - max(vol.attribs.aaBox.x0, vol.attribs.aaBox.x1));

            if (center.y < min(vol.attribs.aaBox.y0, vol.attribs.aaBox.y1))
                dmin += SQR(center.y - min(vol.attribs.aaBox.y0, vol.attribs.aaBox.y1));
            else if (center.y > max(vol.attribs.aaBox.y0, vol.attribs.aaBox.y1))
                dmin += SQR(center.y - max(vol.attribs.aaBox.y0, vol.attribs.aaBox.y1));

            if (center.z < min(vol.attribs.aaBox.z0, vol.attribs.aaBox.z1))
                dmin += SQR(center.z - min(vol.attribs.aaBox.z0, vol.attribs.aaBox.z1));
            else if (center.z > max(vol.attribs.aaBox.z0, vol.attribs.aaBox.z1))
                dmin += SQR(center.z - max(vol.attribs.aaBox.z0, vol.attribs.aaBox.z1));

            return dmin <= radius*radius;
            break;
        case VOLUME_SPHERE:
            volDistSq = aiuFindDistanceSquared(center, vol.attribs.sphere.center);
            return (volDistSq < ((radius+vol.attribs.sphere.radius) * (radius+vol.attribs.sphere.radius)));
            break;
        default:
            return 0;
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : volFindCenter
    Description : Returns the center point of the volume
    Inputs      : vol - the volume to find the center point
    Outputs     :
    Return      : The center of the volume, or 0,0,0 as an error condition
----------------------------------------------------------------------------*/
vector volFindCenter(Volume *vol)
{
    vector returnvec;

    switch (vol->type)
    {
        case VOLUME_AA_BOX:
            returnvec.x = (vol->attribs.aaBox.x0 + vol->attribs.aaBox.x1) / 2;
            returnvec.y = (vol->attribs.aaBox.y0 + vol->attribs.aaBox.y1) / 2;
            returnvec.z = (vol->attribs.aaBox.z0 + vol->attribs.aaBox.z0) / 2;
            return returnvec;
            break;
        case VOLUME_SPHERE:
            return vol->attribs.sphere.center;
            break;
        default:
            vecZeroVector(returnvec);
            return returnvec;
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : volFindRadius
    Description : Finds the radius of the volume
    Inputs      : volume - the volume to find the radius of
    Outputs     :
    Return      : The radius of the volume
----------------------------------------------------------------------------*/
real32 volFindRadius(Volume *vol)
{
    real32 radius = 0;

    switch (vol->type)
    {
        case VOLUME_AA_BOX:
            //divides the average side length by two
            radius = (((vol->attribs.aaBox.x1 - vol->attribs.aaBox.x0) + (vol->attribs.aaBox.y1 - vol->attribs.aaBox.y0) + (vol->attribs.aaBox.z1 - vol->attribs.aaBox.z0))/3)/2;
            break;
        case VOLUME_SPHERE:
            radius = vol->attribs.sphere.radius;
            break;
        default:
            break;
    }
    return radius;
}



