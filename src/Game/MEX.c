/*=============================================================================
    Name    : Mex.c
    Purpose : Mesh extensions

    Created 7/18/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <strings.h>
#include <math.h>
#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "Memory.h"
#include "Debug.h"
#include "File.h"
#include "MEX.h"
#include "color.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

bool mexVerify(void *mex)
{
    ubyte *curmexptr = (ubyte *)mex;

    if (strcasecmp(curmexptr,"mannngo") == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void *mexLoad(char *filename)
{
    void *address;

#ifdef ENDIAN_BIG
	MEXFileHeader *mex;
	MEXChunk *chunk;
	unsigned char i;
#endif

    fileLoadAlloc(filename,&address,NonVolatile);

#ifdef ENDIAN_BIG
	mex          = ( MEXFileHeader *)address;
	mex->version = LittleShort( mex->version );
	mex->nChunks = LittleShort( mex->nChunks );

	chunk = ( MEXChunk *)( ( ( ubyte *)mex ) + sizeof( MEXFileHeader ) );
	for( i=0; i<mex->nChunks; i++ )
	{
		chunk->chunkSize = LittleLong( chunk->chunkSize );

		if(( strcasecmp( chunk->type, "Gun" ) == 0 )
	    || ( strcasecmp( chunk->type, "Eng" ) == 0 )
		|| ( strcasecmp( chunk->type, "Dok" ) == 0 )
		|| ( strcasecmp( chunk->type, "Sal" ) == 0 ) )
		{
			MEXGunChunk *chunk1 = ( MEXGunChunk *)chunk;

			chunk1->position.x = LittleFloat( chunk1->position.x );
			chunk1->position.y = LittleFloat( chunk1->position.y );
			chunk1->position.z = LittleFloat( chunk1->position.z );

			chunk1->normal.x = LittleFloat( chunk1->normal.x );
			chunk1->normal.y = LittleFloat( chunk1->normal.y );
			chunk1->normal.z = LittleFloat( chunk1->normal.z );

			chunk1->coneAngle = LittleFloat( chunk1->coneAngle );
			chunk1->edgeAngle = LittleFloat( chunk1->edgeAngle );
		}

		if( strcasecmp( chunk->type, "Col" ) == 0 )
		{
			MEXCollisionSphereChunk *chunk1 = ( MEXCollisionSphereChunk *)chunk;

			chunk1->level = LittleLong( chunk1->level );

			chunk1->offset.x = LittleFloat( chunk1->offset.x );
			chunk1->offset.y = LittleFloat( chunk1->offset.y );
			chunk1->offset.z = LittleFloat( chunk1->offset.z );

			chunk1->r = LittleFloat( chunk1->r );
		}
		
		if( strcasecmp( chunk->type, "Rct" ) == 0 )
		{
			MEXCollisionRectangleChunk *chunk1 = ( MEXCollisionRectangleChunk *)chunk;

			chunk1->level = LittleLong( chunk1->level );

			chunk1->ox = LittleFloat( chunk1->ox );
			chunk1->oy = LittleFloat( chunk1->oy );
			chunk1->oz = LittleFloat( chunk1->oz );

			chunk1->dx = LittleFloat( chunk1->dx );
			chunk1->dy = LittleFloat( chunk1->dy );
			chunk1->dz = LittleFloat( chunk1->dz );
		}

		if( strcasecmp( chunk->type, "Nav" ) == 0 )
		{
			MEXNAVLightChunk *chunk1 = ( MEXNAVLightChunk *)chunk;

			chunk1->position.x = LittleFloat( chunk1->position.x );
			chunk1->position.y = LittleFloat( chunk1->position.y );
			chunk1->position.z = LittleFloat( chunk1->position.z );
		}

		chunk = ( MEXChunk *)( ( ( ubyte *)chunk ) + sizeof( MEXChunk ) + chunk->chunkSize );
	}
#endif // ENDIAN_BIG

    dbgAssert(address != NULL);
    if (!mexVerify(address))
    {
        dbgFatal(DBG_Loc,"Invalid MEX file");
        return NULL;
    }
    return address;
}

void mexFree(void *mex)
{
    memFree(mex);
}

void *mexGetChunk(void *mex,char *type,char *name)
{
    MEXFileHeader *header = (MEXFileHeader *)mex;
    MEXChunk *curmexchunk = (MEXChunk *) ( ((ubyte *)mex) + sizeof(MEXFileHeader) );
    uword numChunks = header->nChunks;
    uword i;

    for (i=0;i<numChunks;i++)
    {
        if (strcasecmp(curmexchunk->type,type) == 0)
        {
            if (strcasecmp(curmexchunk->name,name) == 0)
            {
                return (void *)curmexchunk;
            }
        }

        curmexchunk = (MEXChunk *)(((ubyte *)curmexchunk) + sizeof(MEXChunk) + curmexchunk->chunkSize);
    }

    return NULL;
}

MEXDockingChunk *mexGetDockChunk(void *mex,char *dockname)
{
    MEXFileHeader *header = (MEXFileHeader *)mex;
    MEXChunk *curmexchunk = (MEXChunk *) ( ((ubyte *)mex) + sizeof(MEXFileHeader) );
    uword numChunks = header->nChunks;
    uword i;

    for (i=0;i<numChunks;i++)
    {
        if (strcasecmp(curmexchunk->type,"Dok") == 0)
        {
            if (strcasecmp(curmexchunk->name,"Dock") == 0)
            {
                if (strcasecmp(((MEXDockingChunk *)curmexchunk)->dockName,dockname) == 0)
                {
                    return (MEXDockingChunk *)curmexchunk;
                }
            }
        }

        curmexchunk = (MEXChunk *)(((ubyte *)curmexchunk) + sizeof(MEXChunk) + curmexchunk->chunkSize);
    }

    return NULL;
}

MEXSalvageChunk *mexGetSalChunk(void *mex,char *salvagename)
{
    MEXFileHeader *header = (MEXFileHeader *)mex;
    MEXChunk *curmexchunk = (MEXChunk *) ( ((ubyte *)mex) + sizeof(MEXFileHeader) );
    uword numChunks = header->nChunks;
    uword i;

    for (i=0;i<numChunks;i++)
    {
        if (strcasecmp(curmexchunk->type,"Sal") == 0)
        {
            if (strcasecmp(curmexchunk->name,"SALVAGE") == 0)
            {
                if (strcasecmp(((MEXSalvageChunk *)curmexchunk)->Name,salvagename) == 0)
                {
                    return (MEXSalvageChunk *)curmexchunk;
                }
            }
        }

        curmexchunk = (MEXChunk *)(((ubyte *)curmexchunk) + sizeof(MEXChunk) + curmexchunk->chunkSize);
    }

    return NULL;
}


MEXNAVLightChunk *mexGetNAVLightChunk(void *mex,char *navLightName)
{
    MEXFileHeader *header = (MEXFileHeader *)mex;
    MEXChunk *curmexchunk = (MEXChunk *) ( ((ubyte *)mex) + sizeof(MEXFileHeader) );
    uword numChunks = header->nChunks;
    uword i;

    for (i=0;i<numChunks;i++)
    {
        if (strcasecmp(curmexchunk->type,"Nav") == 0)
        {
            if (strcasecmp(curmexchunk->name,"Nav") == 0)
            {
                if (strcasecmp(((MEXNAVLightChunk *)curmexchunk)->NAVLightName,navLightName) == 0)
                {
                    return (MEXNAVLightChunk *)curmexchunk;
                }
            }
        }

        curmexchunk = (MEXChunk *)(((ubyte *)curmexchunk) + sizeof(MEXChunk) + curmexchunk->chunkSize);
    }

    return NULL;
}

ResNozzleStatic *mexGetResNozzleStatic(void *mex)
{
    MEXGunChunk *gunchunk;
    ResNozzleStatic *resnozzle = NULL;

    if (mex == NULL)
    {
        dbgMessage("\nWarning: ResNozzle info not retrieved.  Mesh extensions not loaded");
        return NULL;
    }

    gunchunk = mexGetChunk(mex,"Gun","Gun0");       // just use gun chunk for now
    if (gunchunk != NULL)
    {
        resnozzle = memAlloc(sizeof(ResNozzleStatic),"resnozzle",NonVolatile);

        resnozzle->position = gunchunk->position;

        mexVecToVec(&resnozzle->nozzlenormal,&gunchunk->normal);

        return resnozzle;
    }

    return NULL;
}

RepairNozzleStatic *mexGetRepairNozzleStatic(void *mex)
{
    MEXGunChunk *gunchunk;
    RepairNozzleStatic *resnozzle = NULL;

    if (mex == NULL)
    {
        dbgMessage("\nWarning: RepairNozzle info not retrieved.  Mesh extensions not loaded");
        return NULL;
    }

    gunchunk = mexGetChunk(mex,"Gun","Rep0");       // just use gun chunk for now
    if (gunchunk != NULL)
    {
        resnozzle = memAlloc(sizeof(RepairNozzleStatic),"repairnozzle",NonVolatile);

        resnozzle->position = gunchunk->position;

        return resnozzle;
    }

    return NULL;
}

TractorBeamStatic *mexGetTractorBeamStatic(void *mex)
{
    MEXGunChunk *gunchunk;
    TractorBeamStatic *tractoremitter = NULL;

    if (mex == NULL)
    {
        dbgMessage("\nWarning: ResNozzle info not retrieved.  Mesh extensions not loaded");
        return NULL;
    }

    gunchunk = mexGetChunk(mex,"Gun","Beam0");       // just use gun chunk for now
    if (gunchunk != NULL)
    {
        tractoremitter = memAlloc(sizeof(TractorBeamStatic),"tractoremitter",NonVolatile);

        tractoremitter->position = gunchunk->position;

        mexVecToVec(&tractoremitter->nozzlenormal,&gunchunk->normal);

        return tractoremitter;
    }

    return NULL;
}

void mexGetGunStaticInfo(GunStaticInfo *gunstaticinfo,void *mex)
{
    sdword i;
    MEXGunChunk *gunchunk;
    char gunname[8];
    sdword numGuns = gunstaticinfo->numGuns;
    GunStatic *gunstatic;

    if (mex == NULL)
    {
//        dbgFatal(DBG_Loc,"Could not get gun info because mesh extensions were not loaded");
        dbgMessage("\nWarning: Gun info not retrieved.  Mesh extensions not loaded");
        return;
    }

    for (i=0;i<numGuns;i++)
    {
        sprintf(gunname,"Gun%d",i);
        gunchunk = mexGetChunk(mex,"Gun",gunname);
        if (gunchunk != NULL)
        {
            gunstatic = &gunstaticinfo->gunstatics[i];

            gunstatic->position = gunchunk->position;

            mexVecToVec(&gunstatic->gunnormal,&gunchunk->normal);

            if ((gunstatic->cosminAngleFromNorm == 0.0f) && (gunstatic->cosmaxAngleFromNorm == 0.0f))
            {
                gunstatic->cosminAngleFromNorm = (real32)cos(DEG_TO_RAD(gunchunk->edgeAngle));
                gunstatic->cosmaxAngleFromNorm = (real32)cos(DEG_TO_RAD(gunchunk->coneAngle + gunchunk->edgeAngle));
            }
        }
        else
        {
            dbgMessagef("\nWarning: Could not find gun in mex at %x",mex);
        }
    }
}

void mexGetDockStaticInfo(DockStaticInfo *dockstaticinfo,void *mex)
{
    sdword i;
    MEXDockingChunk *dockchunk;
    sdword numDockPoints = dockstaticinfo->numDockPoints;
    DockStaticPoint *dockstaticpoint;

    if (mex == NULL)
    {
        dbgMessage("\nWarning: Dock info not retrieved.  Mesh extensions not loaded");
        return;
    }

    for (i=0;i<numDockPoints;i++)
    {
        dockstaticpoint = &dockstaticinfo->dockstaticpoints[i];
        dockstaticpoint->dockindex = i;
        dockchunk = mexGetDockChunk(mex,dockstaticpoint->name);
        if (dockchunk != NULL)
        {
            dockstaticpoint->position = dockchunk->position;

            mexVecToVec(&dockstaticpoint->conenormal,&dockchunk->normal);

            dockstaticpoint->coneangle = (real32)cos(DEG_TO_RAD(dockchunk->coneAngle + dockchunk->edgeAngle));
        }
        else
        {
            dbgMessagef("\nWarning: Could not find dock point in mex at %x",mex);
        }
    }
}

void mexGetSalvageStaticInfo(SalvageStaticInfo *salvagestaticinfo,void *mex)
{
    sdword i;
    MEXSalvageChunk *salchunk;
    sdword numSalvagePoints = salvagestaticinfo->numSalvagePoints;
    SalvageStaticPoint *salvagestaticpoint;

    if (mex == NULL)
    {
        dbgMessage("\nWarning: Salvage info not retrieved.  Mesh extensions not loaded");
        return;
    }

    for (i=0;i<numSalvagePoints;i++)
    {
        salvagestaticpoint = &salvagestaticinfo->salvageStaticPoints[i];
        salvagestaticpoint->number = i;
        salchunk = mexGetSalChunk(mex,salvagestaticpoint->name);
        if (salchunk != NULL)
        {
            salvagestaticpoint->position = salchunk->position;

            mexVecToVec(&salvagestaticpoint->conenormal,&salchunk->normal);

            salvagestaticpoint->coneangle = (real32)cos(DEG_TO_RAD(salchunk->coneAngle + salchunk->edgeAngle));
        }
        else
        {
            dbgMessagef("\nWarning: Could not find salvage point in mex at %x",mex);
        }
    }
}

void mexGetNAVLightStaticInfo(NAVLightStaticInfo *navlightstaticinfo,void *mex)
{
    sdword i;
    MEXNAVLightChunk *navlightchunk;
    sdword numNAVLights = navlightstaticinfo->numNAVLights;
    NAVLightStatic *navlightstatic;

    if (mex == NULL)
    {
        dbgMessage("\nWarning: NAV Light info not retrieved.  Mesh extensions not loaded");
        return;
    }

    for (i=0;i<numNAVLights;i++)
    {
        navlightstatic = &navlightstaticinfo->navlightstatics[i];
        navlightchunk = mexGetNAVLightChunk(mex,navlightstatic->name);
        if (navlightchunk != NULL)
        {
            navlightstatic->position = navlightchunk->position;
            navlightstatic->color = colRGB(navlightchunk->red,
                                           navlightchunk->green,
                                           navlightchunk->blue);
        }
        else
        {
            dbgMessagef("\nWarning: Could not find NAVLight point in mex at %x",mex);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mexVecToVec
    Description : Convert a mex vector (rotation in x, y, z) to a real vector
    Inputs      : dest - where to store new vector
                  source - rotation of x, y, z
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mexVecToVec(vector *dest, vector *source)
{
    *dest = *source;  //!!! pass straight through !!!
    if (dest->x == 0.0f && dest->y == 0.0f && dest->z == 0.0f)
    {
        dest->x = 1.0f;
    }
    else
    {
        vecNormalize(dest);     //!!! normalize just for now
    }
}

void mexSetCollisionInfo(StaticCollInfo *staticCollInfo,void *mex,real32 forwardScale,real32 upScale,real32 rightScale, real32 xslide, real32 yslide, real32 zslide)
{
    MEXCollisionSphereChunk *spherechunk;
    MEXCollisionRectangleChunk *rectchunk;

    if (mex == NULL)
    {
        return;
    }

    //set uninitialized scale factors to 1.0f;
    if(forwardScale == 0.0f)
        forwardScale = 1.0f;
    if(upScale == 0.0f)
        upScale = 1.0f;
    if(rightScale == 0.0f)
        rightScale = 1.0f;

    spherechunk = mexGetChunk(mex,"Col","Col0");
    if (spherechunk != NULL)
    {
        staticCollInfo->originalcollspheresize = spherechunk->r;
        staticCollInfo->collspheresize = spherechunk->r;
        staticCollInfo->approxcollspheresize = spherechunk->r;
        staticCollInfo->avoidcollspheresize = spherechunk->r;
        staticCollInfo->avoidcollspherepad = spherechunk->r;
        staticCollInfo->collsphereoffset = spherechunk->offset;
        staticCollInfo->collsphereoffset.x += xslide;
        staticCollInfo->collsphereoffset.y += yslide;
        staticCollInfo->collsphereoffset.z += zslide;
        staticCollInfo->collspheresizeSqr = staticCollInfo->collspheresize * staticCollInfo->collspheresize;
    }

    rectchunk = mexGetChunk(mex,"Rct","Rct0");
    if (rectchunk != NULL)
    {
        staticCollInfo->collrectoffset.x = rectchunk->ox+xslide;
        staticCollInfo->collrectoffset.y = rectchunk->oy+yslide;
        staticCollInfo->collrectoffset.z = rectchunk->oz+zslide;

        staticCollInfo->uplength = rectchunk->dx;
        staticCollInfo->rightlength = rectchunk->dy;
        staticCollInfo->forwardlength = rectchunk->dz;

        if (upScale != 1.0f)
        {
            real32 olduplength = staticCollInfo->uplength;
            staticCollInfo->uplength *= upScale;
            staticCollInfo->collrectoffset.x -= (staticCollInfo->uplength - olduplength) * 0.5f;
        }

        if (rightScale != 1.0f)
        {
            real32 oldrightlength = staticCollInfo->rightlength;
            staticCollInfo->rightlength *= rightScale;
            staticCollInfo->collrectoffset.y -= (staticCollInfo->rightlength - oldrightlength) * 0.5f;
        }

        if (forwardScale != 1.0f)
        {
            real32 oldforwardlength = staticCollInfo->forwardlength;
            staticCollInfo->forwardlength *= forwardScale;
            staticCollInfo->collrectoffset.z -= (staticCollInfo->forwardlength - oldforwardlength) * 0.5f;
        }
        // scale the sphere up to guarantee it surrounds the rectangle if necessary
        {
            real32 up = staticCollInfo->uplength * 0.5f;
            real32 right = staticCollInfo->rightlength * 0.5f;
            real32 forward = staticCollInfo->forwardlength * 0.5f;

            real32 minr = sqrt(up*up+right*right+forward*forward) * 1.1f;

            if (minr > staticCollInfo->collspheresize)
            {
                staticCollInfo->originalcollspheresize = minr;
                staticCollInfo->collspheresize = minr;
                staticCollInfo->approxcollspheresize = minr;
                staticCollInfo->avoidcollspheresize = minr;
                staticCollInfo->avoidcollspherepad = minr;
                staticCollInfo->collspheresizeSqr = staticCollInfo->collspheresize * staticCollInfo->collspheresize;
            }
        }

        staticCollInfo->diagonallength = sqrt(staticCollInfo->uplength*staticCollInfo->uplength +
                                              staticCollInfo->forwardlength*staticCollInfo->forwardlength +
                                              staticCollInfo->rightlength*staticCollInfo->rightlength);
    }
    else
    {
        // approximate rectangle using collsphere info
        real32 temp = staticCollInfo->collspheresize * 0.5f;

        staticCollInfo->collrectoffset.x = staticCollInfo->collsphereoffset.x - temp;
        staticCollInfo->collrectoffset.y = staticCollInfo->collsphereoffset.y - temp;
        staticCollInfo->collrectoffset.z = staticCollInfo->collsphereoffset.z - temp;

        staticCollInfo->uplength = staticCollInfo->collspheresize*upScale;
        staticCollInfo->rightlength = staticCollInfo->collspheresize*rightScale;
        staticCollInfo->forwardlength = staticCollInfo->collspheresize*forwardScale;

        staticCollInfo->diagonallength = sqrt(staticCollInfo->uplength*staticCollInfo->uplength +
                                              staticCollInfo->forwardlength*staticCollInfo->forwardlength +
                                              staticCollInfo->rightlength*staticCollInfo->rightlength);
    }
}

void mexSetEngineNozzle(vector* engineNozzleOffset, void* mex, sdword index)
{
    MEXEngineChunk* enginechunk;
    char engine[5];

    if (mex == NULL)
    {
        return;
    }

    strcpy(engine, "Eng0");
    engine[3] = (char)('0' + index);

    enginechunk = mexGetChunk(mex,"Eng",engine);
    if (enginechunk != NULL)
    {
        *engineNozzleOffset = enginechunk->position;
    }
}

