/*=============================================================================
    Name    : particle.c
    Purpose : maintain and render particle systems

    Created 11/13/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define PART_TEST_MORPH 0

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include <float.h>
#include "glinc.h"
#include "Types.h"
#include "Vector.h"
#include "Randy.h"
#include "Debug.h"
#include "color.h"
#include "Matrix.h"
#include "Memory.h"
#include "Particle.h"
#include "SpaceObj.h"
#include "render.h"
#include "Mesh.h"
#include "texreg.h"
#include "FastMath.h"
#include "mainrgn.h"
#include "Task.h"
#include "Universe.h"
#include "AutoLOD.h"
#include "Shader.h"
#include "glcaps.h"
#include "devstats.h"

extern unsigned int gDevcaps;

#ifndef RUB
#define RUB(c) (ubyte)(c * 255.0f)
#endif

#define TEXX(X) ((real32)(X) / (real32)texWidth)
#define TEXY(Y) ((real32)(Y) / (real32)texHeight)

#define MIN2(a,b) ((a) < (b) ? (a) : (b))

/*=============================================================================
    Data:
=============================================================================*/

static sdword alternateIndex = 0;
typedef struct alternate_s
{
    sdword trhandle;    //of the etg texture
    udword glhandle;    //of the alternate
} alternate_t;

static alternate_t alternates[PART_NUM_ALTERNATES];

static GLfloat illumSave[4];
static bool wasStippled;

//function called for each particle created
void (*partCreateCallback)(sdword userValue, ubyte *userData) = NULL;
sdword partCreateUserValue = 0;
ubyte *partCreateUserData = NULL;

sdword partAdvanceMeshMorph(meshSystem* psys, particle* p);
meshdata* partMeshNextMesh(meshSystem* psys, particle* p);

typedef struct
{
    vector  worldVel;
    //misc
    udword  flags;
    real32  drag;
    real32  scale;
    real32  xScale, yScale, zScale;
    real32  scaleDist;
    real32  deltaScale;
    real32  deltaScaleDist;
    //position
    real32  offsetLOF;
    real32  offsetR;
    real32  offsetTheta;
    vector *offsetArray;
    real32  deltaLOF;
    real32  deltaLOFDist;
    real32  deltaR[2];
    real32  deltaRDist[2];
    //velocity
    real32  velLOF;
    real32  velLOFDist;
    real32  velR;
    real32  velRDist;
    real32  deltaVelLOF;
    real32  deltaVelLOFDist;
    real32  deltaVelR;
    real32  deltaVelRDist;
    real32  ang;
    real32  angDist;
    real32  angDelta;
    real32  angDeltaDist;
    //line thingz
    real32  length;
    real32  lengthDist;
    real32  deltaLength;
    real32  deltaLengthDist;
    //appearance
    real32  icolor[4];
    real32  colorDist[4];
    real32  deltaColor[4];
    real32  deltaColorDist[4];
    bool8   lit;
    real32  illum;
    real32  illumDist;
    real32  deltaIllum;
    real32  deltaIllumDist;
    //lifespan
    real32  lifespan;
    real32  lifespanDist;
    real32  waitspan;
    real32  waitspanDist;
    //mesh
    meshdata *mesh;
    real32  exponent;
    real32  exponentDist;
    ubyte   colorScheme;
    sdword  loopCount;
    //texture
    trhandle tex;
    //billboard
    real32  position[3];
    matrix* mat;
    udword  slices;
    udword  uv[2][2];
    void*   tstruct;
    void*   mstruct;
    real32  rate;
    real32  meshRate;
    real32  startMeshFrame;
    bool8   loop;
    uword   startFrame;
    //mesh tumble attribs
    real32  tumble[3];
    real32  tumbleDist[3];
    real32  deltaTumble[3];
    real32  deltaTumbleDist[3];
    //color biasing
    real32  rbias;
    real32  gbias;
    real32  bbias;
    real32  rbiasDist;
    real32  gbiasDist;
    real32  bbiasDist;
    //flags
    bool    specularOnly;
    bool    stippleAlpha;
    bool    noDepthWrite;
    bool    additiveBlends;
    bool    pseudoBillboard;
    bool    trueBillboard;
} particleAttribs;

particleAttribs defaultParticleAttribs =
{
    {0.0f, 0.0f, 0.0f}, //worldVel
    0,                  //flags.
    1.0f,               //drag
    5.0f,               //scale [3]
    1.0f, 1.0f, 1.0f,   //XYZ scale
    0.0f,               //scaleDist
    0.0f,               //deltaScale
    0.0f,               //deltaScaleDist

    0.0f,               //offsetLOF
    0.0f,               //offsetR
    0.0f,               //offsetTheta
    NULL,               //offsetArray
    0.0f,               //deltaLOF
    0.0f,               //deltaLOFDist
    {0.0f, TWOPI},      //deltaR[]
    {0.0f, TWOPI},      //deltaRDist[]
    0.0f,               //velLOF
    0.0f,               //velLOFDist
    0.0f,               //velR
    0.0f,               //velRDist
    0.0f,               //deltaVelLOF
    0.0f,               //deltaVelLOFDist
    0.0f,               //deltaVelR
    0.0f,               //deltaVelRDist
    0.0f,               //ang
    0.0f,               //angDist
    0.0f,               //angDelta
    0.0f,               //angDeltaDist

    5.0f,               //length
    0.0f,               //lengthDist
    0.0f,               //deltaLength
    0.0f,               //deltaLengthDist

    {1.0f, 1.0f, 1.0f, 1.0f},       //icolor
    {0.0f, 0.0f, 0.0f, 0.0f},       //colorDist
    {0.0f, 0.0f, 0.0f, 0.0f},       //deltaColor
    {0.0f, 0.0f, 0.0f, 0.0f},       //deltaColorDist
    FALSE,                          //lit
    0.0f,                           //illum
    0.0f,                           //illumDist
    0.0f,                           //deltaIllum
    0.0f,                           //deltaIllumDist

    2.0f,               //lifespan (s) [2]
    0.0f,               //lifespanDist
    0.0f,               //waitspan
    0.0f,               //waitspanDist

    NULL,               //meshdata
    -1.0f,              //exponent
    0.0f,               //exponentDist
    PART_NOCOLORSCHEME, //color scheme
    -1,                 //loop count
    TR_Invalid,         //trhandle

    {0.0f, 0.0f, 0.0f}, //position
    NULL,
    8,                  //slices
    {{0, 0},            //uv
     {1, 1}},
    NULL,               //tstruct
    NULL,               //mstruct
    10.0f,              //rate (f/s)
    5.0f,               //mesh framerate (f/s)
    0.0f,               //starting mesh frame
    TRUE,               //loopFlag
    0,                  //startFrame

    {0.0f, 0.0f, 0.0f}, //tumble angles (radians)
    {0.0f, 0.0f, 0.0f}, //tumble distribution
    {0.0f, 0.0f, 0.0f}, //deltaTumble (rads/s)
    {0.0f, 0.0f, 0.0f}, //deltaTumble distribution

    //color bias
    0.0f,               //red bias
    0.0f,               //green bias
    0.0f,               //blue bias
    0.0f,               //red bias distribution
    0.0f,               //green bias dist
    0.0f,               //blue bias dist

    //flags
    FALSE,              //specularOnly
    FALSE,              //stippleAlpha
    FALSE,              //noDepthWrite
    TRUE,               //additiveBlends
    FALSE,              //pseudoBillboard
    FALSE               //trueBillboard
};

static particleAttribs pat;

/*-----------------------------------------------------------------------------
    Name        : partStartup
    Description : startup the particle module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partStartup(void)
{
    partShutdown();
}

/*-----------------------------------------------------------------------------
    Name        : partFreeAlternates
    Description : frees "alternates"
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partFreeAlternates(void)
{
    sdword i;

    if (alternateIndex != 0)
    {
#if PART_Verbose > 0
        dbgMessagef("\nfreeing %d particle alternates", alternateIndex);
#endif
        for (i = 0; i < alternateIndex; i++)
        {
            alternates[i].trhandle = TR_InvalidHandle;
            if (alternates[i].glhandle != 0)
            {
                glDeleteTextures(1, &alternates[i].glhandle);
                alternates[i].glhandle = 0;
            }
        }

        alternateIndex = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : partShutdown
    Description : shutdown the particle module.  frees "alternates"
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partShutdown(void)
{
    partFreeAlternates();
}

/*-----------------------------------------------------------------------------
    Name        : partSetDefaults
    Description : reset particle attributes structure to default values
    Inputs      :
    Outputs     : pat is filled with defaultParticleAttribs
    Return      :
----------------------------------------------------------------------------*/
void partSetDefaults()
{
    memcpy(&pat, &defaultParticleAttribs, sizeof(particleAttribs));
}

#define RealClamp(r) \
        if ((r) < 0.0f) r = 0.0f; \
        else if ((r) > 1.0f) r = 1.0f;
#define RealClampNeg(r) \
        if ((r) < -1.0f) r = -1.0f; \
        else if ((r) > 1.0f) r = 1.0f;

static hmatrix particleHMatrix;
static matrix  shiprotmatrix;
static vector  shipposition;
static vector  *partEffectVelocity;              //retained pointer to velocity of effect which spawned said particles

matrix partEffectOwnerSystem;
vector partEffectOwnerPosition;
color  partEffectColor;
real32 partNLips;

//box helper
static void drawBox(GLfloat size, GLenum type)
{
    static GLfloat n[6][3] =
    {
        {-1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f}
    };
    static GLint faces[6][4] =
    {
        {0, 1, 2, 3},
        {3, 2, 6, 7},
        {7, 6, 5, 4},
        {4, 5, 1, 0},
        {5, 6, 2, 1},
        {7, 4, 0, 3}
    };
    GLfloat v[8][3];
    GLint i;

    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

    for (i = 0; i < 6; i++)
    {
        glBegin(type);
        glNormal3fv(&n[i][0]);
        glVertex3fv(&v[faces[i][0]][0]);
        glVertex3fv(&v[faces[i][1]][0]);
        glVertex3fv(&v[faces[i][2]][0]);
        glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}

//like primCircleSolid3, but w/alpha
void partCircleSolid3(vector *centre, real32 radius, sdword nSlices, color c)
{
    sdword index;
    GLfloat v[3];
    double theta;

    glColor4ub(colRed(c), colGreen(c), colBlue(c), colAlpha(c));
    v[0] = centre->x;
    v[1] = centre->y;
    v[2] = centre->z;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(v);
    for (index = 0, theta = 0.0; index < nSlices; index++)
    {
        v[0] = centre->x + (real32)(sin(theta)) * radius;
        v[1] = centre->y + (real32)(cos(theta)) * radius;
        theta += 2.0 * PI / (double)nSlices;
        glVertex3fv(v);
    }
    v[0] = centre->x;
    v[1] = centre->y + radius;
    glVertex3fv(v);
    glEnd();
}

/*-----------------------------------------------------------------------------
    Name        : partBillboardEnable
    Description : sets up sprite billboard mode for worldspace & effectspace systems
    Inputs      : v - position of the sprite
                  isWorldspace - TRUE or FALSE
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partBillboardEnable(vector *v, bool isWorldspace)
{
    if (isWorldspace)
    {
        //enable billboarding
        rndBillboardEnable(v);
    }
    else
    {
        vector vec;

        //object -> world
        vecSubFrom(*v,shipposition);
        matMultiplyMatByVec(&vec, &shiprotmatrix, v);
        vecAddTo(vec,shipposition);
        //enable billboarding
        rndBillboardEnable(&vec);
    }
}

/*-----------------------------------------------------------------------------
    Name        : partBillboardDisable
    Description : disables billboarding
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partBillboardDisable()
{
    //disable billboarding
    rndBillboardDisable();
}

/*-----------------------------------------------------------------------------
    Name        : storeIllum
    Description : saves GL illumination parameters (LIGHT_MODEL_AMBIENT)
    Inputs      :
    Outputs     : global illumSave contains parameters
    Return      :
----------------------------------------------------------------------------*/
void storeIllum()
{
    glGetFloatv(GL_LIGHT_MODEL_AMBIENT, illumSave);
}

/*-----------------------------------------------------------------------------
    Name        : handleIllum
    Description : alters LIGHT_MODEL_AMBIENT according to passed particle's vals
    Inputs      : p - the particle
    Outputs     : the lighting model may be modified
    Return      :
----------------------------------------------------------------------------*/
void handleIllum(particle *p)
{
    GLfloat illum[4];
    if (p->illum != 0.0f)
    {
        illum[0] = p->illum;
        illum[1] = p->illum;
        illum[2] = p->illum;
        illum[3] = 1.0f;
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, illum);
    }
}

/*-----------------------------------------------------------------------------
    Name        : restoreIllum
    Description : restores saved GL illumination parameters
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void restoreIllum()
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, illumSave);
}

/*-----------------------------------------------------------------------------
    Name        : partPerformHacking
    Description : sets up a billboard-friendly MODELVIEW for non-worldspace
                  sprite systems
    Inputs      : partMat - the coordinate system
                  particleTranslation - position in the space
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partPerformHacking(matrix* partMat, vector particleTranslation)
{
    hmatrix mat;
    hvector hvec = {0.0f, 0.0f, 0.0f, 1.0f};

    //build a homogenous matrix
    hmatMakeHMatFromMat(&particleHMatrix, partMat);
    hmatPutVectIntoHMatrixCol4(particleTranslation, particleHMatrix);

    //save position globally
    shipposition = particleTranslation;
    //save rotation globally
    matGetMatFromHMat(&shiprotmatrix, &particleHMatrix);

    //obtain current modelview
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)&mat);
    glPushMatrix();
    //zero translation elements
    hmatPutHVectIntoHMatrixCol4(hvec, mat);
    //load rotation but not translation into modelview
    glLoadMatrixf((GLfloat*)&mat);
}

/*-----------------------------------------------------------------------------
    Name        : partFilter
    Description : enables bilinear filtering for textures without regard to the
                  global filter setting
    Inputs      : on - TRUE or FALSE (yes or no)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partFilter(bool on)
{
    if (on)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

//save GL stipple setting
void stippleSave()
{
    if (RGL)
        wasStippled = (bool)glIsEnabled(GL_POLYGON_STIPPLE);
}

//restore previous GL stipple setting
void stippleRestore()
{
    if (RGL)
    {
        if (wasStippled)
        {
            glEnable(GL_POLYGON_STIPPLE);
        }
        else
        {
            glDisable(GL_POLYGON_STIPPLE);
        }
    }
}

//decide whether to enable stippling or not
void handleStipple(particle* p)
{
    if (!RGL)
        return;

    if (wasStippled)
    {
        //leave stippling alone if global stippling enabled
        return;
    }

    if (bitTest(p->flags, PART_STIPPLE))
    {
        glEnable(GL_POLYGON_STIPPLE);
    }
    else if (!wasStippled)
    {
        glDisable(GL_POLYGON_STIPPLE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : partSaturateAdd
    Description : adds then normalizes input colour values
    Inputs      : out - output array of colours
                  add - input array 0
                  base - input array 1
    Outputs     : out is filled w/ saturated in+base
    Return      :
----------------------------------------------------------------------------*/
void partSaturateAdd(real32* out, real32* add, real32* base)
{
    real32 max, scale;
    real32 in[3];
    sdword i;

    for (i = 0; i < 3; i++)
    {
        in[i] = base[i] + add[i];
    }

    max = in[0];
    if (in[1] > max) max = in[1];
    if (in[2] > max) max = in[2];
    scale = (max == 0.0f) ? 1.0f : (1.0f / max);

    for (i = 0; i < 3; i++)
    {
        out[i] = in[i] * scale;
    }
}

void partBindAlternate(trhandle tex)
{
    sdword i;
    udword alternate;
    ubyte* data;

    texreg* reg = trStructureGet(tex);
    if (!bitTest(reg->flags, TRF_Alpha))
    {
        //do nothing for non-RGBA textures
        return;
    }

    //check devcaps for valid GetTexImage
    if (bitTest(gDevcaps, DEVSTAT_NO_GETTEXIMAGE))
    {
        return;
    }

    //check alternates
    for (i = 0; i < alternateIndex; i++)
    {
        if (alternates[i].trhandle == tex)
        {
            trClearCurrent();
            glBindTexture(GL_TEXTURE_2D, alternates[i].glhandle);
            return;
        }
    }

    //create a new alternate
    glGenTextures(1, &alternate);
    alternates[alternateIndex].trhandle = tex;
    alternates[alternateIndex++].glhandle = alternate;   //store alternate

    if (alternateIndex >= PART_NUM_ALTERNATES)
    {
        //FIXME: this is not an efficient queue
        partFreeAlternates();
    }

    // ... from the data of the old texture (ASSERT: currently bound)
    data = (ubyte*)memAlloc(4*reg->scaledWidth*reg->scaledHeight, "temp part alternate", Pyrophoric);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    for (i = 0; i < 4*reg->scaledWidth*reg->scaledHeight; i += 4)
    {
        data[i+0] = data[i+1] = data[i+2] = 200;
    }

    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, alternate);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 reg->scaledWidth, reg->scaledHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    memFree(data);
}

/*-----------------------------------------------------------------------------
    Name        : partRenderBillSystem
    Description : render a billboard system
    Inputs      : n - number of particles
                  p - pointer to first particle
                  flags - misc flags
                  particleTranslation - position of the system, used for billboarding
                  uv - texture uv coordinates
                  bpart - system header pointer
                  isWorldspace - TRUE or FALSE
    Outputs     :
    Return      : number of live particles; kill the system if 0
----------------------------------------------------------------------------*/
udword partRenderBillSystem(udword n, particle* p, udword flags,
                            vector particleTranslation, udword uv[2][2],
                            billSystem* bpart, bool isWorldspace)
{
    udword i, hits;
    vector pos;
    color col;
    real32 sx;
    real32 sy;
    trhandle currentTex;
    udword texWidth, texHeight;
    static vector origin = {0.0f, 0.0f, 0.0f};
    texreg *reg;
    udword  slices;
    matrix* partMat;
    trhandle tex;
    real32 saturatedBias[3];
    sdword blended;
    bool canTexAdd;

    slices = bpart->slices;
    partMat = bpart->partMat;
    tex = bpart->tex;

    currentTex = tex;

//    rndAdditiveBlends(FALSE);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glShadeModel(GL_SMOOTH);

    if (RGLtype != SWtype)
    {
        rndTextureEnvironment(RTE_Modulate);
    }

    if (currentTex != TR_Invalid)
    {
        reg = trStructureGet(tex);
        texWidth = reg->diskWidth;
        texHeight = reg->diskHeight;
        rndTextureEnable(TRUE);
/*
        trMakeCurrent(tex);
        if (bitTest(reg->flags, TRF_Alpha))
        {
            glEnable(GL_BLEND);
            glDisable(GL_ALPHA_TEST);
            rndAdditiveBlends(FALSE);
        }
*/
    }

    canTexAdd = glCapFeatureExists(RGL_COLOROP_ADD);

    for (i = hits = 0; i < n; i++, p++)
    {
        if (p->lifespan < 0.0f || p->waitspan > 0.0f)
            continue;
        else
            hits++;
        if (p->waitspan > 0.0f)
            continue;
        pos = p->position;
        if (!isWorldspace)
        {
            vecAddTo(pos, particleTranslation);
        }
        partBillboardEnable(&pos, isWorldspace);

        if (p->tstruct == NULL && p->tex != TR_Invalid)
        {
            currentTex = p->tex;
        }

        handleIllum(p);
        //set appropriate stipple mode
        handleStipple(p);

        blended = 0;
        if (canTexAdd)
        {
            glPixelTransferf(GL_RED_BIAS, p->bias[0]);
            glPixelTransferf(GL_GREEN_BIAS, p->bias[1]);
            glPixelTransferf(GL_BLUE_BIAS, p->bias[2]);
        }
        else
        {
            saturatedBias[0] = p->bias[0];
            saturatedBias[1] = p->bias[1];
            saturatedBias[2] = p->bias[2];

            //see if device can handle multiple pass rendering
            if (!bitTest(gDevcaps, DEVSTAT_NO_GETTEXIMAGE))
            {
                if ((p->bias[0] + p->bias[1] + p->bias[2]) > 0.0f)
                {
                    blended = 1;
                }
            }
        }

        if (p->icolor[3] == 1.0f)
        {
            glColor3f(p->icolor[0], p->icolor[1], p->icolor[2]);
            glDisable(GL_BLEND);
        }
        else
        {
            glColor4f(p->icolor[0], p->icolor[1], p->icolor[2], p->icolor[3]);
            glEnable(GL_BLEND);
        }

        if (currentTex == TR_Invalid)
        {
            col = colRGBA(colRealToUbyte(p->icolor[0]), colRealToUbyte(p->icolor[1]),
                          colRealToUbyte(p->icolor[2]), colRealToUbyte(p->icolor[3]));
            partCircleSolid3(&origin, p->scale, slices, col);
        }
        else
        {
            real32 cosTheta, sinTheta;

            rndTextureEnable(TRUE);

            trMakeCurrent(currentTex);
            reg = trStructureGet(tex);
            if (bitTest(reg->flags, TRF_Alpha))
            {
                glEnable(GL_BLEND);
                glDisable(GL_ALPHA_TEST);
                if (bitTest(p->flags, PART_ADDITIVE))
                {
                    rndAdditiveBlends(TRUE);
                }
                else
                {
                    rndAdditiveBlends(FALSE);
                }
            }
            partFilter(TRUE);

            cosTheta = (real32)cos((real64)p->rot);
            sinTheta = (real32)sin((real64)p->rot);

            sx = sy = p->scale * 0.5f;

            if (p->rot == 0.0f)
            {
AGAIN0:
                glBegin(GL_QUADS);
                glTexCoord2f(TEXX(uv[0][0]), TEXY(uv[0][1]));
                glVertex3f(-sx, -sy, 0.0f);
                glTexCoord2f(TEXX(uv[1][0]), TEXY(uv[0][1]));
                glVertex3f(sx, -sy, 0.0f);
                glTexCoord2f(TEXX(uv[1][0]), TEXY(uv[1][1]));
                glVertex3f(sx, sy, 0.0f);
                glTexCoord2f(TEXX(uv[0][0]), TEXY(uv[1][1]));
                glVertex3f(-sx, sy, 0.0f);
                glEnd();
                if (blended)
                {
                    blended--;
                    rndAdditiveBlends(TRUE);
                    glColor4f(saturatedBias[0], saturatedBias[1], saturatedBias[2], p->icolor[3]);
                    //render this guy again, additively blending over the last
                    partBindAlternate(currentTex);
                    partFilter(TRUE);
                    goto AGAIN0;
                }
            }
            else
            {
AGAIN1:
                glBegin(GL_QUADS);
                glTexCoord2f(TEXX(uv[0][0]), TEXY(uv[0][1]));
                glVertex3f((-sx*cosTheta) - (-sy*sinTheta),
                           (-sx*sinTheta) + (-sy*cosTheta),
                           0.0f);
                glTexCoord2f(TEXX(uv[1][0]), TEXY(uv[0][1]));
                glVertex3f((sx*cosTheta) - (-sy*sinTheta),
                           (sx*sinTheta) + (-sy*cosTheta),
                           0.0f);
                glTexCoord2f(TEXX(uv[1][0]), TEXY(uv[1][1]));
                glVertex3f((sx*cosTheta) - (sy*sinTheta),
                           (sx*sinTheta) + (sy*cosTheta),
                           0.0f);
                glTexCoord2f(TEXX(uv[0][0]), TEXY(uv[1][1]));
                glVertex3f((-sx*cosTheta) - (sy*sinTheta),
                           (-sx*sinTheta) + (sy*cosTheta),
                           0.0f);
                glEnd();
                if (blended)
                {
                    blended--;
                    rndAdditiveBlends(TRUE);
                    glColor4f(saturatedBias[0], saturatedBias[1], saturatedBias[2], p->icolor[3]);
                    //render again, additively blending over previous
                    partBindAlternate(currentTex);
                    partFilter(TRUE);
                    goto AGAIN1;
                }
            }
        }

        partBillboardDisable();
    }
    rndTextureEnable(FALSE);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    if (canTexAdd)
    {
        glPixelTransferf(GL_RED_BIAS, 0.0f);
        glPixelTransferf(GL_GREEN_BIAS, 0.0f);
        glPixelTransferf(GL_BLUE_BIAS, 0.0f);
    }

    partFilter(FALSE);
    rndAdditiveBlends(FALSE);
    return hits;
}

/*-----------------------------------------------------------------------------
    Name        : partMeshMaterialPrepare
    Description : Prepare to render a mesh in singl-texture more by setting up
                    material attributes.
    Inputs      : currentTex - handle of texture we're going to render in.
                  p - particle we're going to render.
                  material - first material of the mesh
                  alpha - enable GL_BLEND if alpha > 0 regardless of other settings
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partMeshMaterialPrepare(particle *p, trhandle currentTex, materialentry *material, bool alpha)
{
    texreg *reg;
    GLfloat attribs[4];
    real32 ambientFactor;
#if MESH_SPECULAR
    real32 specularFactor;
#endif
    real32 diffuseRed, otherRed;
    GLenum face = GL_FRONT_AND_BACK;  //!!!

    rndTextureEnable(TRUE);

    trMakeCurrent(currentTex);
    reg = trStructureGet(currentTex);
    if (bitTest(reg->flags, TRF_Alpha) || alpha)
    {
        glEnable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);
        if (bitTest(p->flags, PART_ADDITIVE))
        {
            rndAdditiveBlends(TRUE);
        }
        else
        {
            rndAdditiveBlends(FALSE);
        }
    }
    partFilter(TRUE);
    if (p->lit)
    {                                           //if particle is lit
        //this is an attempt to extract the ambient and specular properties from
        //the materials of the mesh without actually using the ambient and
        //specular colors.  It should work in most cases.  It uses the ratio of
        //ambient/diffuse and ambient/specular reds in the first material of the mesh.
        diffuseRed = colUbyteToReal(colRed(material->diffuse));
        otherRed = colUbyteToReal(colRed(material->ambient));
        ambientFactor = otherRed / diffuseRed;
#if MESH_SPECULAR
        otherRed = colUbyteToReal(colRed(material->specular));
        specularFactor = otherRed / diffuseRed;
#endif

        attribs[0] = p->icolor[0] * ambientFactor;
        attribs[1] = p->icolor[1] * ambientFactor;
        attribs[2] = p->icolor[2] * ambientFactor;
        attribs[3] = 1.0f;
        glMaterialfv(face, GL_AMBIENT, attribs);
#if MESH_SPECULAR
        attribs[0] = p->icolor[0] * specularFactor;
        attribs[1] = p->icolor[1] * specularFactor;
        attribs[2] = p->icolor[2] * specularFactor;
        glMaterialfv(face, GL_SPECULAR, attribs);
#endif
        attribs[0] = p->icolor[0];
        attribs[1] = p->icolor[1];
        attribs[2] = p->icolor[2];
        attribs[3] = p->icolor[3];
        glMaterialfv(face, GL_DIFFUSE, attribs);
        rndLightingEnable(TRUE);
    }
    else
    {
        rndLightingEnable(FALSE);
    }
}

void calcBillboardMatrixWorld(matrix* mat, vector* pos, particle* p)
{
    //nothing here
}

void calcBillboardMatrixNotWorld(matrix* mat, vector* pos)
{
    //nothing here
}

void undoBillboardMatrix()
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : calcPseudoBillboardMatrixWorld
    Description : Calculate a pseudo-billboard matrix for a world-space mesh particle
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void calcPseudoBillboardMatrixWorld(matrix* mat, vector* pos, particle* p, meshSystem* meshPart)
{
    vector velocity, veye, up, right;
    matrix matResult;
    hmatrix hmatResult;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    velocity = p->wVel;
    if (!bitTest(meshPart->flags, PART_WORLDSPACEVELOCITY))
    {
        vecSub(velocity, velocity, *partEffectVelocity);
    }
    if (velocity.x == 0.0f && velocity.y == 0.0f && velocity.z == 0.0f)
    {
        velocity.z = 1.0f;
    }
    else
    {
        vecNormalize(&velocity);
    }

    vecSub(veye, p->position, mrCamera->eyeposition);
    vecNormalize(&veye);

    vecCrossProduct(up, veye, velocity);
    vecCrossProduct(right, velocity, up);
    vecNormalize(&right);
    vecNormalize(&up);

    matPutVectIntoMatrixCol1(up, matResult);
    matPutVectIntoMatrixCol2(right, matResult);
    matPutVectIntoMatrixCol3(velocity, matResult);

    hmatMakeHMatFromMat(&hmatResult, &matResult);

    glMultMatrixf((GLfloat*)&hmatResult);
}

void partInverselyTransform(vector* out, vector* in, matrix* mat, vector* localPos)
{
    vector  spacedIn;

    vecSub(spacedIn, *in, *localPos);
    //transform this way to perform "inverse" operation
    matMultiplyVecByMat(out, &spacedIn, mat);
}

/*-----------------------------------------------------------------------------
    Name        : calcPseudoBillboardMatrixNotWorld
    Description : Calculate a pseudo-billboard matrix for a local-space mesh particle
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void calcPseudoBillboardMatrixNotWorld(matrix* mat, vector* pos, particle* p, meshSystem* meshPart)
{
    vector  velocity, veye, up, right;
    vector  camera_effect, origin_world;
    matrix  matResult;
    hmatrix hmatResult;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    //obtain velocity vector in effectspace
    vecScalarMultiply(velocity, p->rvec, p->velR);
    if (!bitTest(meshPart->flags, PART_WORLDSPACEVELOCITY))
    {
        vecSub(velocity, velocity, *partEffectVelocity);
    }
    if (velocity.x == 0.0f && velocity.y == 0.0f && velocity.z == 0.0f)
    {
        velocity.z = 1.0f;
    }
    else
    {
        vecNormalize(&velocity);
    }

    //obtain particle's worldspace position
    matMultiplyMatByVec(&origin_world, mat, &p->position);
    vecAddTo(origin_world, *pos);

    //obtain camera's position in effectspace
    partInverselyTransform(&camera_effect, &mrCamera->eyeposition, mat, &origin_world);
    //obtain veye
    vecSub(veye, p->position, camera_effect);
    vecNormalize(&veye);

    //obtain up vector from veye & velocity
    vecCrossProduct(up, veye, velocity);
    vecNormalize(&up);
    //obtain right vector from velocity & up
    vecCrossProduct(right, velocity, up);
    vecNormalize(&right);

    //form a coordinate system
    matPutVectIntoMatrixCol1(up, matResult);
    matPutVectIntoMatrixCol2(right, matResult);
    matPutVectIntoMatrixCol3(velocity, matResult);

    hmatMakeHMatFromMat(&hmatResult, &matResult);

    glMultMatrixf((GLfloat*)&hmatResult);
}

void undoPseudoBillboardMatrix()
{
    //ASSERT: MatrixMode == MODELVIEW
    glPopMatrix();
}

/*-----------------------------------------------------------------------------
    Name        : partMeshOrient
    Description : Create a matrix for the mesh
    Inputs      : p - particle to work with
                  bRescaleNormal - TRUE if normals are to be normalized (as in scaled meshes)
                  meshPart - pointer to mesh particle
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void partMeshOrient(particle* p, bool bRescaleNormal, meshSystem* meshPart)
{
    vector velocity;
    hmatrix velHMatrix;
    matrix velMatrix;
    real32 length, scale;

    if (bitTest(p->flags, PART_XYZSCALE))
    {
        hmatMakeHMatFromMat(&velHMatrix, &partEffectOwnerSystem/*meshPart->partMat*/);
        hmatPutVectIntoHMatrixCol4(partEffectOwnerPosition/*p->position*/, velHMatrix);
        glMultMatrixf((GLfloat*)&velHMatrix);
/*
        velocity.x = 0.0f;
        velocity.y = 0.0f;
        velocity.z = -1.0f;

        matCreateCoordSysFromHeading(&velMatrix, &velocity);
        hmatMakeHMatFromMat(&velHMatrix, &velMatrix);
        glMultMatrixf((GLfloat*)&velHMatrix);
*/
        hmatMakeRotAboutX(&velHMatrix, (real32)cos(DEG_TO_RAD(180.0)), (real32)sin(DEG_TO_RAD(180.0)));
        glMultMatrixf((GLfloat*)&velHMatrix);

        glScalef(p->tumble[0], p->tumble[1], p->tumble[2]);
        glEnable(GL_NORMALIZE);

        return;
    }

    //translate the mesh
    glTranslatef(p->position.x, p->position.y, p->position.z);

    //align mesh along particle's velocity vector
    if (!(p->flags & (PART_PSEUDOBILLBOARD | PART_TRUEBILLBOARD)))
    {
        if (bitTest(meshPart->flags, PART_WORLDSPACE))
        {
            velocity = p->wVel;
            if (!bitTest(meshPart->flags, PART_WORLDSPACEVELOCITY))
            {
                vecSub(velocity, velocity, *partEffectVelocity);
            }
            if (velocity.x == 0.0f && velocity.y == 0.0f && velocity.z == 0.0f)
            {
                velocity.z = 1.0f;
            }
            else
            {
                vecNormalize(&velocity);
            }
        }
        else
        {
            vecScalarMultiply(velocity, p->rvec, p->velR);
            if (velocity.x == 0.0f && velocity.y == 0.0f && velocity.z == 0.0f)
            {
                velocity.z = 1.0f;
            }
            else
            {
                vecNormalize(&velocity);
            }
        }
        matCreateCoordSysFromHeading(&velMatrix, &velocity);
        hmatMakeHMatFromMat(&velHMatrix, &velMatrix);
        glMultMatrixf((GLfloat*)&velHMatrix);
    }

    //tumble the mesh
    if (p->tumble[0] != 0.0f)
    {
        glRotatef(RAD_TO_DEG(p->tumble[0]), 1.0f, 0.0f, 0.0f);
    }
    if (p->tumble[1] != 0.0f)
    {
        glRotatef(RAD_TO_DEG(p->tumble[1]), 0.0f, 1.0f, 0.0f);
    }
    if (p->tumble[2] != 0.0f)
    {
        glRotatef(RAD_TO_DEG(p->tumble[2]), 0.0f, 0.0f, 1.0f);
    }

    //scale the mesh, if applicable
    scale = p->scale / partNLips;
    length = scale;
    if (p->length != 5.0f)
    {
        length = p->length / partNLips;
    }
    if (p->scale != 1.0f)
    {
        glScalef(scale, scale, length);
        if (bRescaleNormal && (scale == length))
        {
            glEnable(GL_RESCALE_NORMAL);
        }
        else
        {
            glEnable(GL_NORMALIZE);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : partRenderMeshSystem
    Description : render a mesh system
    Inputs      : n - number of particles
                  p - pointer to first particle
                  flags - misc flags
    Outputs     :
    Return      : number of live particles; kill the system if 0
----------------------------------------------------------------------------*/
udword partRenderMeshSystem(udword n, particle *p, udword flags, trhandle tex, meshSystem* meshPart)
{
    udword i, hits;
    meshdata* mesh;
    meshdata* mesh2;
    polyentry *polyList;
    materialentry *materialList;
    real32 frac;
    trhandle currentTex = tex;
    bool bRescaleNormal, canTexAdd;
    bool hsColor;
    extern bool8 g_SpecHack;

    glColor3ub(200,200,200);

    bRescaleNormal = glCapFeatureExists(GL_RESCALE_NORMAL);

    hsColor = FALSE;
    if (bitTest(p->flags, PART_XYZSCALE))
    {
        //addColor makes hs effect too distinct
        canTexAdd = FALSE;
        if (partEffectColor != colBlack)
        {
            hsColor = TRUE;
        }
    }
    else
    {
        canTexAdd = glCapFeatureExists(RGL_COLOROP_ADD);
    }

    if (!canTexAdd)
    {
        rndTextureEnvironment(RTE_Modulate);
    }

    for (i = hits = 0; i < n; i++, p++)
    {
        if (p->lifespan < 0.0f)
        {
            continue;
        }
        else
        {
            hits++;
        }
        if (p->waitspan > 0.0f)
        {
            continue;
        }

        handleIllum(p);
        handleStipple(p);

        if (bitTest(p->flags, PART_SPECULAR))
        {
            g_SpecHack = TRUE;
            if (usingShader)
                meshSetSpecular(0, 200,200,200,200);
        }
        else
        {
            g_SpecHack = FALSE;
            if (usingShader)
                meshSetSpecular(-1, 0,0,0,0);
        }

        if (bitTest(p->flags, PART_NODEPTHWRITE))
        {
            glDepthMask(GL_FALSE);
        }
        else
        {
            glDepthMask(GL_TRUE);
        }

        glPushMatrix();

        mesh = p->mesh;
        if ((mesh != NULL) && ((sdword)mesh != -1))
        {
            partMeshOrient(p, bRescaleNormal, meshPart);

            if (g_SpecHack)
            {
                if (usingShader)
                    shSetExponent(0, p->exponent);
                else if (RGL)
                    rglSpecExp(0, p->exponent);
            }

            if (bitTest(flags, PART_ALPHA))
            {
                glColor4f(p->icolor[0], p->icolor[1], p->icolor[2], p->icolor[3]);
                glEnable(GL_BLEND);
                if (usingShader && g_SpecHack)
                {
                    meshSetSpecular(0, RUB(p->icolor[0]), RUB(p->icolor[1]), RUB(p->icolor[2]), RUB(p->icolor[3]));
                }
            }
            else
            {
                glColor3f(p->icolor[0], p->icolor[1], p->icolor[2]);
                if (usingShader && g_SpecHack)
                {
                    meshSetSpecular(0, RUB(p->icolor[0]), RUB(p->icolor[1]), RUB(p->icolor[2]), 255);
                }
            }

            if (canTexAdd)
            {
                glPixelTransferf(GL_RED_BIAS, p->bias[0]);
                glPixelTransferf(GL_GREEN_BIAS, p->bias[1]);
                glPixelTransferf(GL_BLUE_BIAS, p->bias[2]);
            }
            else
            {
                //simulate colorop_add in a crude manner

                real32 csat[3];

                if ((p->bias[0] + p->bias[1] + p->bias[2]) > 0.0f)
                {
                    if (hsColor)
                    {
                        p->bias[0] = colReal32(colRed(partEffectColor));
                        p->bias[1] = colReal32(colGreen(partEffectColor));
                        p->bias[2] = colReal32(colBlue(partEffectColor));
                    }
                    partSaturateAdd(csat, p->bias, p->icolor);

                    if (bitTest(flags, PART_ALPHA))
                    {
                        glColor4f(csat[0], csat[1], csat[2], p->icolor[3]);
                    }
                    else
                    {
                        glColor3f(csat[0], csat[1], csat[2]);
                    }
                }
            }

            //logic to determine what texture to use on this mesh, if any
            if (p->tstruct == NULL && p->tex != TR_Invalid)
            {
                currentTex = p->tex;
            }

#if MESH_VERBOSE_LEVEL
            if (mesh->nPolygonObjects > 1)
            {                                               //display warning if there are more than 1 object in the mesh
                dbgMessagef("\nThere are %d polygon objects in mesh ('%s').", mesh->nPolygonObjects,
#if MESH_RETAIN_FILENAMES
                    mesh->fileName);
#else
                    "N/A");
#endif //MESH_RETAIN_FILENAMES
            }
#endif //MESH_VERBOSE_LEVEL

            if (bitTest(p->flags, PART_PSEUDOBILLBOARD))
            {
                if (bitTest(flags, PART_WORLDSPACE))
                {
                    calcPseudoBillboardMatrixWorld(meshPart->partMat, &meshPart->partPos, p, meshPart);
                }
                else
                {
                    calcPseudoBillboardMatrixNotWorld(meshPart->partMat, &meshPart->partPos, p, meshPart);
                }
            }
            else if (bitTest(p->flags, PART_TRUEBILLBOARD))
            {
                if (bitTest(flags, PART_WORLDSPACE))
                {
                    calcBillboardMatrixWorld(meshPart->partMat, &meshPart->partPos, p);
                }
                else
                {
                    calcBillboardMatrixNotWorld(meshPart->partMat, &meshPart->partPos);
                }
            }

            if (p->mstruct == NULL)
            {                                               //if no morph animation
                if (currentTex != TR_Invalid)
                {                                           //if there is a texture
                    partMeshMaterialPrepare(p, currentTex, &mesh->localMaterial[0], bitTest(flags, PART_ALPHA));
                    rndGLStateLog("meshSystem");
                    meshObjectRenderTex(&mesh->object[0], mesh->localMaterial);
                }
                else
                {                                           //no texture, render solid surfaces
                    rndTextureEnable(FALSE);
                    rndGLStateLog("meshSystem");
                    meshObjectRender(&mesh->object[0], mesh->localMaterial, p->colorScheme);
                }
            }
            else
            {                                               //else there is a morph animation
                mesh2 = partMeshNextMesh(meshPart, p);
                frac = p->meshFrame - (real32)((sdword)p->meshFrame);

                if (bRescaleNormal)
                {
                    glDisable(GL_RESCALE_NORMAL);
                }
                glEnable(GL_NORMALIZE);
                polyList = ((meshAnim*)p->mstruct)->mesh->object[0].pPolygonList;
                materialList = ((meshAnim*)p->mstruct)->mesh->localMaterial;
                if (currentTex != TR_Invalid)
                {                                           //if there is a texture
                    partMeshMaterialPrepare(p, currentTex, &materialList[0], bitTest(flags, PART_ALPHA));
                    meshMorphedObjectRenderTex(&mesh->object[0], &mesh2->object[0], polyList, materialList, frac, p->colorScheme);
                }
                else
                {                                           //morphed mesh, mo texture
                    meshMorphedObjectRender(&mesh->object[0], &mesh2->object[0], polyList, materialList, frac, p->colorScheme);
                }
                glDisable(GL_NORMALIZE);
            }

            if (bitTest(p->flags, PART_PSEUDOBILLBOARD))
            {
                undoPseudoBillboardMatrix();
            }
            else if (bitTest(p->flags, PART_TRUEBILLBOARD))
            {
                undoBillboardMatrix();
            }

            if (p->scale != 1.0f)
            {
                if (bRescaleNormal)
                {
                    glDisable(GL_RESCALE_NORMAL);
                }
                else
                {
                    glDisable(GL_NORMALIZE);
                }
            }
        }

        glPopMatrix();
    }

    if (RGL)
    {
        if (usingShader)
        {
            shSetExponent(0, -1.0f);
            meshSetSpecular(-1, 0, 0, 0, 0);
        }
        else
        {
            rglSpecExp(0, -1.0f);
        }
    }
    else
    {
        shSetExponent(0, -1.0f);
        meshSetSpecular(-1, 0, 0, 0, 0);
    }

    if (canTexAdd)
    {
        glPixelTransferf(GL_RED_BIAS, 0.0f);
        glPixelTransferf(GL_GREEN_BIAS, 0.0f);
        glPixelTransferf(GL_BLUE_BIAS, 0.0f);
    }

    g_SpecHack = FALSE;
    glDepthMask(GL_TRUE);

    glDisable(GL_BLEND);

    return hits;
}

/*-----------------------------------------------------------------------------
    Name        : partRenderLineSystem
    Description : render a line system
    Inputs      : n - number of particles
                  p - pointer to first particle
                  flags - misc flags
    Outputs     :
    Return      : number of live particles; kill the system if 0
----------------------------------------------------------------------------*/
udword partRenderLineSystem(udword n, particle *p, udword flags)
{
    udword i, hits;
    vector pos;
    bool texEnabled, lightEnabled;
    bool alpha = FALSE;

    glPushAttrib(GL_LINE_BIT);
    if (bitTest(flags, PART_ALPHA))
    {
        alpha = TRUE;
        rndAdditiveBlends(FALSE);
        glEnable(GL_BLEND);
    }

    texEnabled = rndTextureEnable(FALSE);
    lightEnabled = rndLightingEnable(FALSE);

    for (i = hits = 0; i < n; i++, p++)
    {
        if (p->lifespan < 0.0f)
            continue;
        else
            hits++;
        if (p->waitspan > 0.0f)
            continue;

        handleIllum(p);
        handleStipple(p);

        if (bitTest(p->flags, PART_ADDITIVE))
        {
            rndAdditiveBlends(TRUE);
        }
        else
        {
            rndAdditiveBlends(FALSE);
        }

        glLineWidth(p->scale);
        glBegin(GL_LINES);
        if (alpha)
        {
            glColor4f(p->icolor[0], p->icolor[1], p->icolor[2], p->icolor[3]);
        }
        else
        {
            glColor3f(p->icolor[0], p->icolor[1], p->icolor[2]);
        }
        pos = p->position;
        if (bitTest(flags, PART_WORLDSPACE))
        {
            vector rv = p->wVel;
            vecNormalize(&rv);
            if (isnan((double)rv.x) || isnan((double)rv.y) || isnan((double)rv.z) ||
				(ABS(rv.x) < REALlySmall && ABS(rv.y) <= REALlySmall && ABS(rv.z) <= REALlySmall))
            {
                rv.z = 1.0f;
            }
            if (!isnan((double)p->length))
            {
                vecMultiplyByScalar(rv, p->length);
            }
            vecAddTo(pos, rv);
        }
        else
        {
            vector rv = p->rvec;
            vecMultiplyByScalar(rv, p->velR);
            rv.z += p->velLOF;
            if (rv.x == 0.0f && rv.y == 0.0f && rv.z == 0.0f)
            {
                rv.z = 1.0f;
            }
            vecNormalize(&rv);
            vecMultiplyByScalar(rv, p->length);
            vecAddTo(pos, rv);
        }
        glVertex3f(p->position.x, p->position.y, p->position.z);
        glVertex3f(pos.x, pos.y, pos.z);
        glEnd();
    }

    if (alpha)
    {
        glDisable(GL_BLEND);
    }
    rndAdditiveBlends(FALSE);

    rndTextureEnable(texEnabled);
    rndLightingEnable(lightEnabled);

    glPopAttrib();
    return hits;
}

/*-----------------------------------------------------------------------------
    Name        : partRenderCubeSystem
    Description : render a cube system
    Inputs      : n - number of particles
                  p - pointer to first particle
                  flags - misc flags
    Outputs     :
    Return      : number of live particles; kill the system if 0
----------------------------------------------------------------------------*/
udword partRenderCubeSystem(udword n, particle *p, udword flags)
{
    udword i, hits;
    bool alpha = FALSE;

    if (bitTest(flags, PART_ALPHA))
    {
        alpha = TRUE;
        rndAdditiveBlends(FALSE);
        glEnable(GL_BLEND);
    }

    for (i = hits = 0; i < n; i++, p++)
    {
        if (p->lifespan < 0.0f)
            continue;
        else
            hits++;
        if (p->waitspan > 0.0f)
            continue;
        glPushMatrix();
        glTranslatef(p->position.x, p->position.y, p->position.z);
        if (alpha)
            glColor4f(p->icolor[0], p->icolor[1], p->icolor[2], p->icolor[3]);
        else
            glColor3f(p->icolor[0], p->icolor[1], p->icolor[2]);
        handleIllum(p);
        drawBox(p->scale * 0.5f, GL_QUADS);
        glPopMatrix();
    }

    if (alpha)
        glDisable(GL_BLEND);

    return hits;
}

/*-----------------------------------------------------------------------------
    Name        : partRenderPointSystem
    Description : render a point system
    Inputs      : n - number of particles
                  p - pointer to first particle
                  flags - misc flags
    Outputs     :
    Return      : number of live particles; kill the system if 0
----------------------------------------------------------------------------*/
udword partRenderPointSystem(udword n, particle *p, udword flags)
{
    udword i, hits;
    bool alpha = FALSE;

    bool texEnabled, lightEnabled;

    texEnabled = rndTextureEnable(FALSE);
    lightEnabled = rndLightingEnable(FALSE);

    glPushAttrib(GL_POINT_BIT);
    if (bitTest(flags, PART_ALPHA))
    {
        alpha = TRUE;
        glEnable(GL_BLEND);
        if (bitTest(p->flags, PART_ADDITIVE))
        {
            rndAdditiveBlends(TRUE);
        }
        else
        {
            rndAdditiveBlends(FALSE);
        }
    }

    for (i = hits = 0; i < n; i++, p++)
    {
        if (p->lifespan < 0.0f)
            continue;
        else
            hits++;
        if (p->waitspan > 0.0f)
            continue;

        handleIllum(p);

        glPointSize((p->scale <= 3.0f) ? p->scale : 3.0f);
        if (RGL)
            rglFeature(RGL_EFFECTPOINT);
        glBegin(GL_POINTS);
        if (alpha)
            glColor4f(p->icolor[0], p->icolor[1], p->icolor[2], p->icolor[3]);
        else
            glColor3f(p->icolor[0], p->icolor[1], p->icolor[2]);
        glVertex3f(p->position.x, p->position.y, p->position.z);
        glEnd();
    }

    if (alpha)
    {
        glDisable(GL_BLEND);
    }

    rndTextureEnable(texEnabled);
    rndLightingEnable(lightEnabled);

    glPopAttrib();
    rndAdditiveBlends(FALSE);
    return hits;
}

/*-----------------------------------------------------------------------------
    Name        : partRenderSystem
    Description : render a particle system.  passes control to other more
                  specific functions.  assumes MODELVIEW already set.
    Inputs      : psys - the particle system
    Outputs     : sets psys->isAlive to FALSE if no particle is left alive
    Return      :
----------------------------------------------------------------------------*/
void partRenderSystem(psysPtr psys)
{
    pointSystem *pp;
    particle *p = NULL;
    udword hits;
    sdword wasLit;
    bool isWorldspace;
    vector position;

    billSystem* billPart;

    wasLit = rndLightingEnable(FALSE);

    pp = (pointSystem*)psys;
    storeIllum();
    stippleSave();
    isWorldspace = bitTest(pp->flags, PART_WORLDSPACE) ? TRUE : FALSE;

    p = (particle*)(psys + partHeaderSize(psys));
    if (p->lit)
    {
        rndLightingEnable(TRUE);
    }

    switch (pp->t)
    {
    case PART_BILLBOARD:
        billPart = (billSystem*)pp;
        memcpy(&position, &billPart->position, sizeof(vector));
        if (!isWorldspace)
        {
            //will push matrix
            partPerformHacking(((billSystem*)pp)->partMat, position);
        }
        hits = partRenderBillSystem(pp->n, p, pp->flags, position,
                                    billPart->uv, billPart, isWorldspace);
        if (!isWorldspace)
        {
            //pop the matrix
            glPopMatrix();
        }
        break;

    case PART_MESH:
        alodEnable(FALSE);
        hits = partRenderMeshSystem(pp->n, p, pp->flags, ((meshSystem*)pp)->tex,
                                    (meshSystem*)pp);
        alodEnable(TRUE);
        break;

    case PART_LINES:
        hits = partRenderLineSystem(pp->n, p, pp->flags);
        break;

    case PART_CUBES:
        hits = partRenderCubeSystem(pp->n, p, pp->flags);
        break;

    case PART_POINTS:
        hits = partRenderPointSystem(pp->n, p, pp->flags);
        break;

    default:
#if PART_Verbose > 0
        dbgMessagef("\nunknown particle system type: %d", pp->t);
#endif
        return;
    }

    restoreIllum();
    stippleRestore();

    if (!hits)                      //kill the system
    {
        pp->isAlive = FALSE;
    }
    rndLightingEnable(wasLit);
    rndAdditiveBlends(FALSE);
}

//constant delta
real32 partRealDelta(real32 n, real32 d)
{
    return n + d;
}

//randomized delta
real32 partRealDist(real32 n, real32 d)
{
    real32 r = (real32)((real64)(ranRandom(RAN_ParticleStream) % 1000000) / 1000000.0);
    real32 sign = (ranRandom(RAN_ParticleStream) % 2 == 0) ? -1.0f : 1.0f;
    if (d < 0.0f)
        sign = -1.0f;
    return n + sign*r*d;
}

//constant delta, will handle alpha
void partColorDelta(real32 *c, real32 *d, udword flags, real32 dt)
{
    real32 cr, cg, cb, ca;
    if (d[0] != 0.0f)
    {
        cr = partRealDelta(c[0], d[0] * dt);
        RealClamp(cr);
        c[0] = cr;
    }
    if (d[1] != 0.0f)
    {
        cg = partRealDelta(c[1], d[1] * dt);
        RealClamp(cg);
        c[1] = cg;
    }
    if (d[2] != 0.0f)
    {
        cb = partRealDelta(c[2], d[2] * dt);
        RealClamp(cb);
        c[2] = cb;
    }
    if (bitTest(flags, PART_ALPHA) && d[3] != 0.0f)
    {
        ca = partRealDelta(c[3], d[3] * dt);
        RealClamp(ca);
        c[3] = ca;
    }
}

//randomized color delta
void partColorDist(real32 *out, real32 *c, real32 dr, real32 dg, real32 db)
{
    real32 cr, cg, cb;
    cr = partRealDist(c[0], dr);
    cg = partRealDist(c[1], dg);
    cb = partRealDist(c[2], db);
    RealClampNeg(cr);
    RealClampNeg(cg);
    RealClampNeg(cb);
    out[0] = cr;
    out[1] = cg;
    out[2] = cb;
}

//like partColorDist, but with alpha
void partColorDistA(real32 *out, real32 *c, real32 dr, real32 dg, real32 db, real32 da)
{
    real32 ca;
    partColorDist(out, c, dr, dg, db);
    ca = c[3];
    ca = partRealDist(ca, da);
    RealClampNeg(ca);
    out[3] = ca;
}

real32 fracPart(real32 f)
{
    sdword i = (sdword)f;
    return (real32)fabs((real64)((real32)i - f));
}

#if PART_TEST_MORPH
meshAnim* partSetupTestMeshMorphBlock()
{
    meshAnim* ma;
    meshAnim* a;

    ma = (meshAnim*)memAlloc(5*sizeof(meshAnim), "mesh animation block", 0);

#ifdef _WIN32
#define TMP_MESH_PATH "etg\\meshes\\"
#else
#define TMP_MESH_PATH "etg/meshes/"
#endif

    a = ma;
    a->mesh = meshLoad(TMP_MESH_PATH "head00.geo");
    a->flags = PART_LOOPSTART;

    a++;
    a->mesh = meshLoad(TMP_MESH_PATH "head01.geo");
    a->flags = 0;

    a++;
    a->mesh = meshLoad(TMP_MESH_PATH "head02.geo");
    a->flags = 0;

    a++;
    a->mesh = meshLoad(TMP_MESH_PATH "head03.geo");
    a->flags = PART_LOOP;

    a++;
    a->mesh = NULL;
    a->flags = 0;

#undef TMP_MESH_PATH

    return ma;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : partMeshNextMesh
    Description : mesh animations track the CURRENT frame of animation; this fn
                  returns the NEXT frame of animation for lerping
    Inputs      : psys - particle system header
                  p - the particular particle
    Outputs     :
    Return      : mesh object of the next animation frame
----------------------------------------------------------------------------*/
meshdata* partMeshNextMesh(meshSystem* psys, particle* p)
{
    meshAnim* animblock;
    meshAnim* frame;
    sdword frameNo;

    frameNo = partAdvanceMeshMorph(psys, p);
    animblock = (meshAnim*)p->mstruct;
    frame = animblock + frameNo;
    return frame->mesh;
}

/*-----------------------------------------------------------------------------
    Name        : partAdvanceMeshMorph
    Description : actually updates the frame of a mesh animation.
                  p->meshFrame is ASSUMED VALID
    Inputs      : p - the partice to update
    Outputs     :
    Return      : the frame of animation is returned
----------------------------------------------------------------------------*/
//this fn assumes that the current frame is valid (ie. not an end or overflow)
sdword partAdvanceMeshMorph(meshSystem* psys, particle* p)
{
    meshAnim* animblock;
    meshAnim* prev;
    meshAnim* next;
    sdword    frame;

    animblock = (meshAnim*)p->mstruct;
    prev = animblock + (sdword)p->meshFrame;     //floor for previous

    //-- seek to next position --

    //maybe we're a loopend
    if (prev->flags == PART_LOOP)
    {
        if (p->loopCount > 0)
        {
            p->loopCount--;
        }
        else if (p->loopCount == 0)
        {
            goto NO_LOOP;
        }
        //seek to loopstart
        for (next = animblock, frame = 0;; frame++, next++)
        {
            if (next->flags == PART_LOOPSTART)
            {
                return frame;   //found the start of the loop
            }
            else if (next->mesh == NULL)
            {
                return -1;      //error condition
            }
        }
    }

 NO_LOOP:

    //find the trivial next
    frame = (sdword)p->meshFrame + 1;
    next = animblock + frame;
    if (next->mesh == NULL || next->mesh == 0x7fffffff)
    {
        if (p->loopCount == 0)
        {
            //we've exceeded our loopcount, so don't wrap around
            frame = (sdword)p->meshFrame;
        }
        else
        {
            next = animblock;       //wrapped around
            frame = 0;
        }
    }

    return frame;
}

/*-----------------------------------------------------------------------------
    Name        : partSetMeshFromAnimation
    Description : sets the mesh on a particle from the current frame of animation.
                  p->meshFrame is ASSUMED VALID
    Inputs      : p - the particle
    Outputs     : p->mesh is updated
    Return      :
----------------------------------------------------------------------------*/
void partSetMeshFromAnimation(particle* p)
{
    meshAnim* animblock;
    meshAnim* frame;

    animblock = (meshAnim*)p->mstruct;
    frame = animblock + (sdword)p->meshFrame;
    p->mesh = frame->mesh;
}

/*-----------------------------------------------------------------------------
    Name        : partUpdateMeshAnimation
    Description : updates the frame of a mesh animation
    Inputs      : psys - mesh particle system header
                  part - the particle to update
                  dt - elapsed time
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
//when rendering, use floor(meshFrame) as "from", and the result of
//partAdvanceMeshMorph(..) as "to"
void partUpdateMeshAnimation(meshSystem* psys, particle* part, real32 dt)
{
    real32 prevFrame, nextFrame, frac;
    sdword iFrames, frame, i;

    if (part->meshRate == 0.0f)         //nothing to update
    {
        return;
    }

    prevFrame = part->meshFrame;
    nextFrame = prevFrame + dt * part->meshRate;

    if ((sdword)nextFrame == (sdword)prevFrame)
    {
        //no animation advance, just new lerp parameters
        part->meshFrame = nextFrame;
        partSetMeshFromAnimation(part);
        return;
    }

    iFrames = (sdword)nextFrame - (sdword)prevFrame;
    frac = nextFrame - (real32)floor((real64)nextFrame);
    //seek iFrames into the animation
    for (i = 0; i < iFrames; i++)
    {
        frame = partAdvanceMeshMorph(psys, part);
        part->meshFrame = (real32)frame + frac;
    }

    partSetMeshFromAnimation(part);
}

/*-----------------------------------------------------------------------------
    Name        : partUpdateTexAnim
    Description : returns an incremented frame counter for a texture animation
                  (frame++)
    Inputs      : bsys - billboard particle system header
                  p - the particular particle we're interested in
    Outputs     :
    Return      : incremented frame counter
----------------------------------------------------------------------------*/
sdword partAdvanceTexAnim(billSystem* bsys, particle* p)
{
    texAnim* animblock;
    texAnim* prev;
    texAnim* next;
    sdword   frame;

    animblock = (texAnim*)p->tstruct;
    prev = animblock + (sdword)p->currentFrame;     //floor for previous

    //-- seek to next position --

    //maybe we're a loopend
    if (p->loop && prev->flags == PART_LOOP)
    {
        //seek to loopstart
        for (next = animblock, frame = 0;; frame++, next++)
        {
            if (next->flags == PART_LOOPSTART)
            {
                return frame;   //found the start of the loop
            }
            else if ((sdword)next->tex == -1)
            {
                return -1;      //error condition
            }
        }
    }

    //find the trivial next
    frame = (sdword)p->currentFrame + 1;
    next = animblock + frame;
    if ((sdword)next->tex == -1)
    {
        if (!p->loop)
        {
            //we've exceeded our loopcount, so don't wrap around
            frame = (sdword)p->currentFrame;
        }
        else
        {
            next = animblock;       //wrapped around
            frame = 0;
        }
    }

    return frame;
}

/*-----------------------------------------------------------------------------
    Name        : partSetTexFromAnimation
    Description : sets the texture on a particle from the current frame of animation.
                  part->currentFrame is ASSUMED VALID
    Inputs      : bsys - system header
                  part - individual particle
    Outputs     : system's texture is modified
    Return      :
----------------------------------------------------------------------------*/
void partSetTexFromAnimation(psysPtr bsys, particle* part)
{
    texAnim* animblock;
    texAnim* frame;

    animblock = (texAnim*)part->tstruct;
    frame = animblock + (sdword)part->currentFrame;
    partModifyTexture(bsys, frame->tex,
                      (udword)frame->uv[0][0], (udword)frame->uv[0][1],
                      (udword)frame->uv[1][0], (udword)frame->uv[1][1]);
}

/*-----------------------------------------------------------------------------
    Name        : partUpdateAnimation
    Description : updates the frame of a texture animation
    Inputs      : bsys - system header
                  part - particular particle
                  dt - time elapsed
    Outputs     : system's texture is modified, particle's frame counter is updated
    Return      :
----------------------------------------------------------------------------*/
void partUpdateAnimation(billSystem* bsys, particle* part, real32 dt)
{
    real32 prevFrame, nextFrame, frac;
    sdword iFrames, frame, i;

    if (part->rate == 0.0f)
    {
        return;
    }

    prevFrame = part->currentFrame;
    nextFrame = prevFrame + dt * part->rate;

    if ((sdword)nextFrame == (sdword)prevFrame)
    {
        part->currentFrame = nextFrame;
        partSetTexFromAnimation((psysPtr)bsys, part);
        return;
    }

    iFrames = (sdword)nextFrame - (sdword)prevFrame;
    frac = nextFrame - (real32)floor((real64)nextFrame);
    for (i = 0; i < iFrames; i++)
    {
        frame = partAdvanceTexAnim(bsys, part);
        part->currentFrame = (real32)frame + frac;
    }

    partSetTexFromAnimation((psysPtr)bsys, part);
}

/*-----------------------------------------------------------------------------
    Name        : partUpdateSystem
    Description : ticks a particle system by given elapsed time
    Inputs      : psys - particle system pointer
                  dt - elapsed time since last call
                  velvec - velocity vector to be added to velLOF
                           and velR
    Outputs     :
    Return      : TRUE if system is dead, FALSE otherwise
----------------------------------------------------------------------------*/
bool8 partUpdateSystem(psysPtr psys, real32 dt, vector* velvec)
{
    billSystem *pp;
    particle *p = NULL;
    udword n, i, hits;
    vector rvec;
    bool draggin;

    pp = (billSystem*)psys;      //default assumption
    n = pp->n;
    if (pp->angDelta != 0.0f)
    {
        pp->ang += dt * pp->angDelta;
    }

    p = (particle*)(psys + partHeaderSize(psys));

    if (p == NULL)
    {
        dbgMessage("\n!! invalid particle system !!");
        return TRUE;
    }

    draggin = (pp->drag != 1.0f);

    for (i = hits = 0; i < n; i++, p++)
    {
        //mesh tumble specifics
        if (pp->t == PART_MESH)
        {
            p->tumble[0] += dt * p->deltaTumble[0];
            p->tumble[1] += dt * p->deltaTumble[1];
            p->tumble[2] += dt * p->deltaTumble[2];
        }

        if (p->waitspan > 0.0f)
        {
            p->waitspan -= dt;
            if (p->waitspan < 0.0f)
            {
                p->waitspan = 0.0f;
            }
            hits++;
            continue;       //don't dec lifespan if waiting
        }

        //sprite/mesh system specifics
        if (p->tstruct != NULL)
        {
            partUpdateAnimation(pp, p, dt);
        }
        if (p->mstruct != NULL)
        {
            partUpdateMeshAnimation((meshSystem*)pp, p, dt);
        }
        if (p->deltaRot != 0.0f)
        {
            p->rot += dt * p->deltaRot;
        }

        //lines
        if (p->deltaLength)
        {
            p->length += dt * p->deltaLength;
            if (isnan((double)p->length))
            {
                p->length = 1.0f;
            }
        }

        //kinematics
        if (bitTest(pp->flags, PART_WORLDSPACE))
        {
            vector adjWVel, adjWAccel;

            //don't update position if XYZ scaling (hyperspace effect)
            if (!bitTest(p->flags, PART_XYZSCALE))
            {
                //setup
                adjWVel = p->wVel;
                adjWAccel = p->wAccel;
                vecMultiplyByScalar(adjWVel, dt);
                vecMultiplyByScalar(adjWAccel, dt);
                //velocity
                vecAddTo(adjWVel, adjWAccel);
                //update
                vecAddTo(p->position, adjWVel);
                vecAddTo(p->wVel, adjWAccel);
                //drag
                vecMultiplyByScalar(p->wVel, pp->drag);
                vecMultiplyByScalar(p->wAccel, pp->drag);
            }
        }
        else
        {
            //position
            p->position.z += dt * p->velLOF;
            rvec = p->rvec;
            vecMultiplyByScalar(rvec, dt * p->velR);
            vecAddTo(p->position, rvec);

            //velocity
            p->velLOF += dt * p->deltaVelLOF;
            p->velR += dt * p->deltaVelR;
            if (draggin)
            {
                p->velLOF *= pp->drag;
                p->velR *= pp->drag;
            }
        }

        //appearance
        if (p->deltaScale != 0.0f)
        {                                                   //scale the particle
            p->scale += dt * p->deltaScale;
            if (p->scale < 0.0f)
            {                                               //clamp at zero
                p->scale = 0.0f;
            }
        }

        partColorDelta(p->icolor, p->deltaColor, pp->flags, dt);

        //FIXME: does this make sense?
        if (p->icolor[3] < 1.0f)
        {
            pointSystem* sys = (pointSystem*)psys;
            bitSet(sys->flags, PART_ALPHA);
        }

        if (p->lit)
        {
            p->illum += dt * p->deltaIllum;
            RealClamp(p->illum);
        }

        //lifespan
        p->lifespan -= dt;
        if (p->lifespan > 0.0f)
        {
            hits++;
        }
    }

    return((bool8)((hits == 0) ? TRUE : FALSE));
}

bool g_Billboarded;

real64 randomAngle(udword a)
{
    udword r = ranRandom(RAN_ParticleStream) % a;
    return DEG_TO_RAD((real64)r);
}

//make a spherically distributed particle
vector partMakeSPos(real32 offsetLOF, real32 offsetR, real32 offsetTheta,
                    real32 deltaLOF, real32 deltaLOFDist,
                    real32 deltaR, real32 deltaRDist,
                    real32 deltaTheta, real32 deltaThetaDist,
                    vector *rVec)
{
    vector xyz;
    hvector hvec, rvec;
    real32 sinTheta, cosTheta;
    hmatrix result;

    //deltas
    deltaLOF = partRealDist(deltaLOF, deltaLOFDist);
    deltaR = partRealDist(deltaR, deltaRDist);
    deltaTheta = partRealDist(deltaTheta, deltaThetaDist);

    //offsets
    offsetLOF = partRealDelta(offsetLOF, deltaLOF);
    offsetR = partRealDelta(offsetR, deltaR);
    offsetTheta = partRealDelta(offsetTheta, deltaTheta);

    //cartesian
    {
        real64 a = randomAngle(360);
        sinTheta = (real32)sin(a);
        cosTheta = (real32)cos(a);
    }
    vecZeroVector(xyz);
    xyz.x = 1.0f;
    vecMakeHVecFromVec(hvec, xyz);
    hmatMakeRotAboutZ(&result, sinTheta, cosTheta);
    hmatMultiplyHMatByHVec(&rvec, &result, &hvec);
    {
        real64 a = randomAngle(360);
        sinTheta = (real32)sin(a);
        cosTheta = (real32)cos(a);
    }
    hmatMakeRotAboutX(&result, sinTheta, cosTheta);
    hmatMultiplyHMatByHVec(&hvec, &result, &rvec);
    rvec = hvec;
    hmatMultiplyHMatByHVec(&rvec, &result, &hvec);
    vecHomogenize(&xyz, &rvec);
    *rVec = xyz;
    vecNormalize(rVec);
    vecMultiplyByScalar(xyz, offsetR);
    xyz.z += offsetLOF;

    //done
    return xyz;
}

/*-----------------------------------------------------------------------------
    Name        : partMakePosition
    Description : turns a many-variabled cylindrical coordinate system
                  interpretation into something nice.
    Inputs      : offsetLOF - distance along line of fire
                  offsetR - radius component
                  offsetTheta - theta component
                  deltaLOF - variance in offsetLOF
                  deltaLOFDist - variance in deltaLOF
                  deltaR - variance in offsetR
                  deltaRDist - variance in deltaR
                  deltaTheta - variance in theta
                  deltaThetaDist - variance in deltaTheta
                  rVec - pointer to a vector to recieve radial velocity
    Outputs     : rVec - unitized offsetR rotated by theta
    Return      : a 3d vector (x,y,z)
----------------------------------------------------------------------------*/
vector partMakePosition(real32 offsetLOF, real32 offsetR, real32 offsetTheta,
                        real32 deltaLOF, real32 deltaLOFDist,
                        real32 deltaR, real32 deltaRDist,
                        real32 deltaTheta, real32 deltaThetaDist,
                        vector *rVec)
{
    vector xyz;
    vector hvec;
    real32 sinTheta, cosTheta;
    matrix result;

    //deltas
    deltaLOF = partRealDist(deltaLOF, deltaLOFDist);
    deltaR = partRealDist(deltaR, deltaRDist);
    deltaTheta = partRealDist(deltaTheta, deltaThetaDist);

    //offsets
    offsetLOF = partRealDelta(offsetLOF, deltaLOF);
    offsetR = partRealDelta(offsetR, deltaR);
    offsetTheta = partRealDelta(offsetTheta, deltaTheta);

    //cartesian
    sinTheta = (real32)sin((real64)offsetTheta);
    cosTheta = (real32)cos((real64)offsetTheta);
    vecZeroVector(xyz);
    xyz.x = offsetR;
    hvec = xyz;
    matMakeRotAboutZ(&result, sinTheta, cosTheta);
    matMultiplyMatByVec(&xyz, &result, &hvec);
    hvec.x = 1.0f;
    hvec.y = hvec.z = 0.0f;
    matMultiplyMatByVec(rVec, &result, &hvec);

    xyz.z += offsetLOF;

    //done
    return xyz;
}

/*-----------------------------------------------------------------------------
    Name        : partFillGenericParticles
    Description : fills up the generic portion of a particle structure with the
                  current particle state (particleAttribs)
    Inputs      : n - number of particles
                  psys - the particle system, type-specific header + generic portion
                  headerlength - length of the type-specific portion
                  dist - PART_CYLINDER or PART_SPHERE
    Outputs     : fills up the generic portion of psys appropriately
    Return      :
----------------------------------------------------------------------------*/
void partFillGenericParticles(udword n, psysPtr psys,
                              udword headerlength, udword dist,
                              bool isWorldspace)
{
    udword i, pos, size;
    vector rvec;
    particle *p;
    bool alpha = FALSE;

    if (bitTest(pat.flags, PART_ALPHA))
        alpha = TRUE;

    for (i = 0; i < n; i++)
    {
        //call the particle creation callback, if any
        if (partCreateCallback)
        {
            partCreateCallback(partCreateUserValue, partCreateUserData);
        }
        size = sizeof(particle);
        pos = headerlength + i*size;
        p = (particle*)(psys + pos);

        //flags
        p->flags = 0;
        if (pat.specularOnly)
        {
            bitSet(p->flags, PART_SPECULAR);
        }
        if (pat.stippleAlpha)
        {
            bitSet(p->flags, PART_STIPPLE);
        }
        if (pat.additiveBlends)
        {
            bitSet(p->flags, PART_ADDITIVE);
        }
        if (pat.pseudoBillboard)
        {
            bitSet(p->flags, PART_PSEUDOBILLBOARD);
        }
        if (pat.trueBillboard)
        {
            bitSet(p->flags, PART_TRUEBILLBOARD);
        }
        if (pat.noDepthWrite)
        {
            bitSet(p->flags, PART_NODEPTHWRITE);
        }

        //sprite system specifics
        p->tex = TR_Invalid;
        p->rot = partRealDist(pat.ang, pat.angDist);
        p->deltaRot = partRealDist(pat.angDelta, pat.angDeltaDist);

        p->tstruct = pat.tstruct;
        p->rate = pat.rate;
        p->meshRate = pat.meshRate;
        p->loop = pat.loop;
        p->currentFrame = (real32)pat.startFrame;

        //position
        if (dist == PART_CYLINDER)
        {
            p->position = partMakePosition(pat.offsetLOF, pat.offsetR, pat.offsetTheta,
                                           pat.deltaLOF, pat.deltaLOFDist,
                                           pat.deltaR[0], pat.deltaRDist[0],
                                           pat.deltaR[1], pat.deltaRDist[1],
                                           &rvec);
        }
        else
        {
            p->position = partMakeSPos(pat.offsetLOF, pat.offsetR, pat.offsetTheta,
                                       pat.deltaLOF, pat.deltaLOFDist,
                                       pat.deltaR[0], pat.deltaRDist[0],
                                       pat.deltaR[1], pat.deltaRDist[1],
                                       &rvec);
        }
        if (pat.offsetArray != NULL)
        {
            vecAddTo(p->position, pat.offsetArray[i]);
            //comment out the previous line and uncomment this section to get a mapping from
            //Lightwave ship coords to Homeworld ship coords
            /*
            p->position.x += pat.offsetArray[i].y;
            p->position.y += pat.offsetArray[i].z;
            p->position.z -= pat.offsetArray[i].x;
            */

        }

        p->rvec = rvec;
        vecNormalize(&p->rvec);
        //velocity
        p->deltaVelLOF = partRealDist(pat.deltaVelLOF, pat.deltaVelLOFDist);
        p->deltaVelR = partRealDist(pat.deltaVelR, pat.deltaVelRDist);
        p->velLOF = partRealDist(pat.velLOF, pat.velLOFDist);
        p->velR = partRealDist(pat.velR, pat.velRDist);
        //appearance
        p->scale = partRealDist(pat.scale, pat.scaleDist);
        p->deltaScale = partRealDist(pat.deltaScale, pat.deltaScaleDist);
        if (alpha)
        {
            partColorDistA(p->icolor, pat.icolor, pat.colorDist[0], pat.colorDist[1],
                           pat.colorDist[2], pat.colorDist[3]);
            partColorDistA(p->deltaColor, pat.deltaColor, pat.deltaColorDist[0],
                           pat.deltaColorDist[1], pat.deltaColorDist[2],
                           pat.deltaColorDist[3]);
        }
        else
        {
            partColorDist(p->icolor, pat.icolor, pat.colorDist[0],
                          pat.colorDist[1], pat.colorDist[2]);
            partColorDist(p->deltaColor, pat.deltaColor, pat.deltaColorDist[0],
                          pat.deltaColorDist[1], pat.deltaColorDist[2]);
            p->icolor[3] = 1.0f;
        }
        p->lit = pat.lit;
        p->illum = partRealDist(pat.illum, pat.illumDist);
        p->deltaIllum = partRealDist(pat.deltaIllum, pat.deltaIllumDist);
        p->exponent = partRealDist(pat.exponent, pat.exponentDist);
        p->colorScheme = pat.colorScheme;
        //lifespan
        p->lifespan = partRealDist(pat.lifespan, pat.lifespanDist);
        p->waitspan = partRealDist(pat.waitspan, pat.waitspanDist);
        //line-specifics
        p->length = partRealDist(pat.length, pat.lengthDist);
        p->deltaLength = partRealDist(pat.deltaLength, pat.deltaLengthDist);

        p->bias[0] = partRealDist(pat.rbias, pat.rbiasDist);
        p->bias[1] = partRealDist(pat.gbias, pat.gbiasDist);
        p->bias[2] = partRealDist(pat.bbias, pat.bbiasDist);

        //mesh specifics
        p->mesh = pat.mesh;
        p->tumble[0] = partRealDist(pat.tumble[0], pat.tumbleDist[0]);
        p->tumble[1] = partRealDist(pat.tumble[1], pat.tumbleDist[1]);
        p->tumble[2] = partRealDist(pat.tumble[2], pat.tumbleDist[2]);
        p->deltaTumble[0] = partRealDist(pat.deltaTumble[0], pat.deltaTumbleDist[0]);
        p->deltaTumble[1] = partRealDist(pat.deltaTumble[1], pat.deltaTumbleDist[1]);
        p->deltaTumble[2] = partRealDist(pat.deltaTumble[2], pat.deltaTumbleDist[2]);
        p->loopCount = (sbyte)pat.loopCount;
#if PART_TEST_MORPH
        if (partTypeof(psys) == PART_MESH)
        {
            p->mstruct = partSetupTestMeshMorphBlock();
        }
        else
#endif
        {
            p->mstruct = pat.mstruct;
        }

        p->meshFrame = pat.startMeshFrame;

        if (bitTest(pat.flags, PART_XYZSCALE))
        {
            bitSet(p->flags, PART_XYZSCALE);
            p->tumble[0] = pat.xScale;
            p->tumble[1] = pat.yScale;
            p->tumble[2] = pat.zScale;
        }
        else
        {
            bitClear(p->flags, PART_XYZSCALE);
        }

        if (isWorldspace)
        {
            vector newVector, worldPosition;
            vector velVec, accelVec;
            vector velLOF, velR;
            vector accelLOF, accelR;
            matrix coordSys;
            //initial setup
            memcpy(&coordSys, pat.mat, 9*sizeof(real32));
            memcpy(&worldPosition, pat.position, sizeof(vector));
            //setup velocity
            velLOF.x = velLOF.y = 0.0f;
            velLOF.z = p->velLOF;
            velR = p->rvec;
            vecMultiplyByScalar(velR, p->velR);
            //setup acceleration
            accelLOF.x = accelLOF.y = 0.0f;
            accelLOF.z = p->deltaVelLOF;
            accelR = p->rvec;
            vecMultiplyByScalar(accelR, p->deltaVelR);
            //get velocity
            vecAdd(velVec, velLOF, velR);
            //get acceleration
            vecAdd(accelVec, accelLOF, accelR);
            //transform position
            matMultiplyMatByVec(&newVector, &coordSys, &p->position);
            vecAddTo(newVector, worldPosition);
            p->position = newVector;
            //transform velocity
            matMultiplyMatByVec(&newVector, &coordSys, &velVec);
            vecAddTo(newVector, pat.worldVel);
            p->wVel = newVector;
            //transform acceleration
            matMultiplyMatByVec(&newVector, &coordSys, &accelVec);
            p->wAccel = newVector;
            //transform rvec
#if 0
            p->rvec = velVec;
            if (p->rvec.x == 0.0f
                && p->rvec.y == 0.0f
                && p->rvec.z == 0.0f)
                p->rvec.z = 1.0f;
            vecNormalize(&p->rvec);
#else
            matMultiplyMatByVec(&newVector, &coordSys, &p->rvec);
#endif
            vecNormalize(&p->rvec);
        }
    }
    partCreateCallback = NULL;
}

psysPtr partCreateSystemWithDelta(particleType t, udword n, udword delta)
{
    real32 d = (real32)delta;
    real32 rn = (real32)n;
    rn = partRealDist(rn, d);
    return partCreateSystem(t, (udword)rn);
}

psysPtr partCreateSphericalSystemWithDelta(particleType t, udword n, udword delta)
{
    real32 d = (real32)delta;
    real32 rn = (real32)n;
    rn = partRealDist(rn, d);
    return partCreateSphericalSystem(t, (udword)rn);
}

psysPtr partCreationHelper(particleType t, udword n, udword dist)
{
    psysPtr p;
    pointSystem *point;
    cubeSystem  *cube;
    lineSystem  *line;
    meshSystem  *mesh;
    billSystem  *bill;
    udword len;
    bool isWorldspace;

    isWorldspace = (bool)bitTest(pat.flags, PART_WORLDSPACE);

    g_Billboarded = FALSE;
    switch (t)
    {
    case PART_BILLBOARD:
        len = sizeof(billSystem) + n*sizeof(particle);
        p = memAlloc(len, "ps(partsys)", Pyrophoric);
        bill = (billSystem*)p;
        bill->t = t;
        bill->n = (uword)n;
        bill->lastUpdated = universe.totaltimeelapsed;
        bill->drag = pat.drag;
        bill->isAlive = TRUE;
        bill->flags = pat.flags;
        bill->ang = pat.ang;
        bill->angDelta = pat.angDelta;
        bill->tex = pat.tex;
        bill->slices = (uword)pat.slices;
        bill->partMat = pat.mat;
        memcpy(bill->uv, pat.uv, 4*sizeof(real32));
        memcpy(&bill->position, pat.position, sizeof(vector));
        g_Billboarded = TRUE;
        partFillGenericParticles(n, p, sizeof(billSystem), dist, isWorldspace);
        break;
    case PART_MESH:
        len = sizeof(meshSystem) + n*sizeof(particle);
        p = memAlloc(len, "ps(partsys)", Pyrophoric);
        mesh = (meshSystem*)p;
        mesh->t = t;
        mesh->n = (uword)n;
        mesh->lastUpdated = universe.totaltimeelapsed;
        mesh->drag = pat.drag;
        mesh->isAlive = TRUE;
        mesh->flags = pat.flags;
        mesh->ang = pat.ang;
        mesh->angDelta = pat.angDelta;
        mesh->tex = pat.tex;
        mesh->partMat = pat.mat;
        memcpy(&mesh->partPos, pat.position, sizeof(vector));
        partFillGenericParticles(n, p, sizeof(meshSystem), dist, isWorldspace);
        break;
    case PART_LINES:
        len = sizeof(lineSystem) + n*sizeof(particle);
        p = memAlloc(len, "ps(partsys)", Pyrophoric);
        line = (lineSystem*)p;
        line->t = t;
        line->n = (uword)n;
        line->lastUpdated = universe.totaltimeelapsed;
        line->drag = pat.drag;
        line->isAlive = TRUE;
        line->flags = pat.flags;
        line->ang = pat.ang;
        line->angDelta = pat.angDelta;
        partFillGenericParticles(n, p, sizeof(lineSystem), dist, isWorldspace);
        break;
    case PART_CUBES:
        len = sizeof(cubeSystem) + n*sizeof(particle);
        p = memAlloc(len, "ps(partsys)", Pyrophoric);
        cube = (cubeSystem*)p;
        cube->t = t;
        cube->n = (uword)n;
        cube->lastUpdated = universe.totaltimeelapsed;
        cube->drag = pat.drag;
        cube->isAlive = TRUE;
        cube->flags = pat.flags;
        cube->ang = pat.ang;
        cube->angDelta = pat.angDelta;
        partFillGenericParticles(n, p, sizeof(cubeSystem), dist, isWorldspace);
        break;
    case PART_POINTS:
        len = sizeof(pointSystem) + n*sizeof(particle);
        p = memAlloc(len, "ps(partsys)", Pyrophoric);
        point = (pointSystem*)p;
        point->t = t;
        point->n = (uword)n;
        point->lastUpdated = universe.totaltimeelapsed;
        point->drag = pat.drag;
        point->isAlive = TRUE;
        point->flags = pat.flags;
        point->ang = pat.ang;
        point->angDelta = pat.angDelta;
        partFillGenericParticles(n, p, sizeof(pointSystem), dist, isWorldspace);
        break;
    default:
#if PART_Verbose > 0
        dbgMessagef("\nunknown particle system type %d", (udword)t);
#endif
        p = NULL;
    }
#if khent
#if PART_Verbose > 1
    dbgMessagef("\nsystem required %dK", len >> 10);
#endif
#endif
    return p;
}

/*-----------------------------------------------------------------------------
    Name        : partCreateSphericalSystem
    Description : creates a particle system with the current state,
                  but in the shape of a sphere
    Inputs      : t - type of particles in the system
                  n - number of particles
    Outputs     :
    Return      : returns the psysPtr, or NULL on error
----------------------------------------------------------------------------*/
psysPtr partCreateSphericalSystem(particleType t, udword n)
{
    return partCreationHelper(t, n, PART_SPHERE);
}

/*-----------------------------------------------------------------------------
    Name        : partCreateSystem
    Description : creates a particle system with the current state
    Inputs      : t - type of particles in the system
                  n - number of particles
    Outputs     :
    Return      : returns the psysPtr or NULL on error
----------------------------------------------------------------------------*/
psysPtr partCreateSystem(particleType t, udword n)
{
    return partCreationHelper(t, n, PART_CYLINDER);
}

/*-----------------------------------------------------------------------------
    Name        : partHeaderSize
    Description : returns the size of the header of a system of particles
    Inputs      : psys - the particle system
    Outputs     :
    Return      : returns the header size or 0 if of unrecognized type
----------------------------------------------------------------------------*/
udword partHeaderSize(psysPtr psys)
{
    pointSystem *p = (pointSystem*)psys;
    switch (p->t)
    {
    case PART_POINTS:
        return sizeof(pointSystem);
    case PART_CUBES:
        return sizeof(cubeSystem);
    case PART_LINES:
        return sizeof(lineSystem);
    case PART_MESH:
        return sizeof(meshSystem);
    case PART_BILLBOARD:
        return sizeof(billSystem);
    default:
        return 0;
    }
}

/*============================
  modifiers
 ============================*/
void partSetSystemAlphaFlag(psysPtr psys, real32 a)
{
    pointSystem* sys = (pointSystem*)psys;

    if (a > 0.996f)
    {
        bitClear(sys->flags, PART_ALPHA);
    }
    else
    {
        bitSet(sys->flags, PART_ALPHA);
    }
}

void partModifyTumble(psysPtr psys, vector* t)
{
    if (partTypeof(psys) == PART_MESH)
    {
        udword i;
        particle *p = (particle*)(psys + partHeaderSize(psys));

        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            memcpy(p->tumble, t, sizeof(vector));
        }
    }
    else
    {
        dbgMessage("\nparticle system is not of type PART_MESH");
    }
}

void partModifyDeltaTumble(psysPtr psys, vector* dt)
{
    if (partTypeof(psys) == PART_MESH)
    {
        udword i;
        particle *p = (particle*)(psys + partHeaderSize(psys));

        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            memcpy(p->deltaTumble, dt, sizeof(vector));
        }
    }
    else
    {
        dbgMessage("\nparticle system is not of type PART_MESH");
    }
}

void partModifyExponent(psysPtr psys, real32 exponent)
{
    udword i;
    udword fSpec = (exponent == 0.0f) ? 0 : PART_SPECULAR;

    particle* p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->exponent = exponent;
        p->flags = ((p->flags & (~PART_SPECULAR)) | fSpec);
    }
}

void partModifyDrag(psysPtr psys, real32 d)
{
    pointSystem *p = (pointSystem*)psys;
    if (fabs((real64)d) < 0.0001)
        d = 0.0f;
    if (d < 0.0f || d > 1.0f)
        dbgMessagef("\ndrag coefficient out of range (%f)", d);
    else
        p->drag = d;
}

void partModifyMesh(psysPtr psys, meshdata *mesh)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));

    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->mesh = mesh;
    }
}

void partModifyLighting(psysPtr psys, bool lit)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->lit = (bool8)lit;
}

void partModifyIllum(psysPtr psys, real32 illum)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->illum = illum;
}

void partModifyDeltaIllum(psysPtr psys, real32 deltaIllum)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->deltaIllum = deltaIllum;
}

void partModifyTexture(psysPtr psys, trhandle tex,
                       udword u0, udword v0, udword u1, udword v1)
{
    billSystem *m = (billSystem*)psys;
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    if (m->t == PART_BILLBOARD)
    {
        m->tex = tex;
        m->uv[0][0] = u0;
        m->uv[0][1] = v0;
        m->uv[1][0] = u1;
        m->uv[1][1] = v1;
        for (i = 0; i < partNumberof(psys); i++, p++)
            p->tex = tex;
    }
    else if (m->t == PART_MESH)
    {
        m->tex = tex;
        for (i = 0; i < partNumberof(psys); i++, p++)
            p->tex = tex;
    }
    else
    {
        dbgMessage("\nparticle system is not of type PART_BILLBOARD");
    }
}

void partModifyScale(psysPtr psys, real32 s)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->scale = s;
}

void partModifyDeltaScale(psysPtr psys, real32 d)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->deltaScale = d;
}

void partModifyDeltaLength(psysPtr psys, real32 d)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->deltaLength = d;
}

void partModifyLifespan(psysPtr psys, real32 l)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->lifespan = l;
}

void partModifyLength(psysPtr psys, real32 l)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->length = l;
}

void partModifyVelLOF(psysPtr psys, real32 v)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->velLOF = v;
}

void partModifyDeltaVelLOF(psysPtr psys, real32 d)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->deltaVelLOF = d;
}

void partModifyVelR(psysPtr psys, real32 v)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->velR = v;
}

void partModifyDeltaVelR(psysPtr psys, real32 d)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
        p->deltaVelR = d;
}

void partModifyColor(psysPtr psys, real32 r, real32 g, real32 b, real32 a)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));

    partSetSystemAlphaFlag(psys, a);

    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->icolor[0] = r;
        p->icolor[1] = g;
        p->icolor[2] = b;
        p->icolor[3] = a;
    }
}

void partModifyColorC(psysPtr psys, color c)
{
    udword i;
    real32 r, g, b, a;
    particle *p = (particle*)(psys + partHeaderSize(psys));

    r = colUbyteToReal(colRed(c));
    g = colUbyteToReal(colGreen(c));
    b = colUbyteToReal(colBlue(c));
    a = colUbyteToReal(colAlpha(c));

    partSetSystemAlphaFlag(psys, a);

    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->icolor[0] = r;
        p->icolor[1] = g;
        p->icolor[2] = b;
        p->icolor[3] = a;
    }
}

void partModifyDeltaColorC(psysPtr psys, color c)
{
    udword i;
    real32 r, g, b, a;
    particle *p = (particle*)(psys + partHeaderSize(psys));

    r = colUbyteToReal(colRed(c));
    g = colUbyteToReal(colGreen(c));
    b = colUbyteToReal(colBlue(c));
    a = colUbyteToReal(colAlpha(c));

    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->deltaColor[0] = r;
        p->deltaColor[1] = g;
        p->deltaColor[2] = b;
        p->deltaColor[3] = a;
    }
}

void partModifyDeltaColor(psysPtr psys, real32 dr, real32 dg,
                          real32 db, real32 da)
{
    udword i;
    particle *p = (particle*)(psys + partHeaderSize(psys));

    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->deltaColor[0] = dr;
        p->deltaColor[1] = dg;
        p->deltaColor[2] = db;
        p->deltaColor[3] = da;
    }
}

void partModifyAnimation(psysPtr psys, void* tstruct)
{
    if (partTypeof(psys) == PART_BILLBOARD ||
        partTypeof(psys) == PART_MESH)
    {
        udword i;
        particle* p = (particle*)(psys + partHeaderSize(psys));
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            p->tstruct = tstruct;
        }
    }
    else
    {
        dbgMessage("\npartModifyAnimation needs a PART_BILLBOARD or PART_MESH");
    }
}

void partModifyMorph(psysPtr psys, void* mstruct)
{
    if (partTypeof(psys) == PART_MESH)
    {
        udword i;
        particle* p = (particle*)(psys + partHeaderSize(psys));
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            p->mstruct = mstruct;
        }
    }
    else
    {
        dbgMessage("\npartModifyMorph needs a PART_MESH");
    }
}

void partModifyFramerate(psysPtr psys, real32 rate)
{
    if (partTypeof(psys) == PART_BILLBOARD)
    {
        udword i;
        particle* p = (particle*)(psys + partHeaderSize(psys));
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            p->rate = rate;
        }
    }
    else
    {
        dbgMessage("\npartModifyFramerate needs a PART_BILLBOARD");
    }
}

void partModifyMorphFramerate(psysPtr psys, real32 meshRate)
{
    if (partTypeof(psys) == PART_MESH)
    {
        udword i;
        particle* p = (particle*)(psys + partHeaderSize(psys));
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            p->meshRate = meshRate;
        }
    }
    else
    {
        dbgMessage("\npartModifyMorphFramerate needs a PART_MESH");
    }
}

void partModifyBillPosition(psysPtr psys, vector* pos)
{
    if (partTypeof(psys) == PART_BILLBOARD)
    {
        billSystem* bpart = (billSystem*)psys;
        bpart->position = *pos;
    }
    else
    {
        dbgMessage("\npartModifyBillPosition needs a PART_BILLBOARD");
    }
}

void partModifyColorBias(
    psysPtr psys, real32 rb, real32 gb, real32 bb)
{
    udword i;
    particle* p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->bias[0] = rb;
        p->bias[1] = gb;
        p->bias[2] = bb;
    }
}

void partModifyAddColor(psysPtr psys, color c)
{
    udword i;
    real32 r = colUbyteToReal(colRed(c));
    real32 g = colUbyteToReal(colGreen(c));
    real32 b = colUbyteToReal(colBlue(c));
    particle* p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->bias[0] = r;
        p->bias[1] = g;
        p->bias[2] = b;
    }
}

void partModifyLoopFlag(psysPtr psys, bool8 willLoop)
{
    if (partTypeof(psys) == PART_BILLBOARD)
    {
        udword i;
        particle* p = (particle*)(psys + partHeaderSize(psys));
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            p->loop = willLoop;
        }
    }
    else
    {
        dbgMessage("\npartModifyLoopFlag needs a PART_BILLBOARD");
    }
}

void partModifySpecular(psysPtr psys, bool spec)
{
    udword i;
    particle* p;

    p = (particle*)(psys + partHeaderSize(psys));

    if (spec)
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitSet(p->flags, PART_SPECULAR);
        }
    }
    else
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitClear(p->flags, PART_SPECULAR);
        }
    }
}

void partModifyStipple(psysPtr psys, bool stip)
{
    udword i;
    particle* p;

    p = (particle*)(psys + partHeaderSize(psys));

    if (stip)
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitSet(p->flags, PART_STIPPLE);
        }
    }
    else
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitClear(p->flags, PART_STIPPLE);
        }
    }
}

void partModifyNoDepthWrite(psysPtr psys, bool noWrite)
{
    udword i;
    particle* p;

    p = (particle*)(psys + partHeaderSize(psys));

    if (noWrite)
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitSet(p->flags, PART_NODEPTHWRITE);
        }
    }
    else
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitClear(p->flags, PART_NODEPTHWRITE);
        }
    }
}

void partModifyAdditiveBlends(psysPtr psys, bool add)
{
    udword i;
    particle* p;

    p = (particle*)(psys + partHeaderSize(psys));

    if (add)
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitSet(p->flags, PART_ADDITIVE);
        }
    }
    else
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitClear(p->flags, PART_ADDITIVE);
        }
    }
}

void partModifyPseudoBillboard(psysPtr psys, bool bill)
{
    udword i;
    particle* p;

    p = (particle*)(psys + partHeaderSize(psys));

    if (bill)
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitSet(p->flags, PART_PSEUDOBILLBOARD);
        }
    }
    else
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitClear(p->flags, PART_PSEUDOBILLBOARD);
        }
    }
}

void partModifyTrueBillboard(psysPtr psys, bool bill)
{
    udword i;
    particle* p;

    p = (particle*)(psys + partHeaderSize(psys));

    if (bill)
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitSet(p->flags, PART_TRUEBILLBOARD);
        }
    }
    else
    {
        for (i = 0; i < partNumberof(psys); i++, p++)
        {
            bitClear(p->flags, PART_TRUEBILLBOARD);
        }
    }
}

void partModifyColorScheme(psysPtr psys, sdword colorScheme)
{
    udword i;
    particle* p;

    p = (particle*)(psys + partHeaderSize(psys));
    for (i = 0; i < partNumberof(psys); i++, p++)
    {
        p->colorScheme = colorScheme;
    }
}

//0 - normal, 1 - additive, 2 - stipple
void partModifyAlphaMode(psysPtr psys, udword mode)
{
    switch (mode)
    {
    case 1:
        partModifyAdditiveBlends(psys, TRUE);
        partModifyStipple(psys, FALSE);
        break;
    case 2:
        partModifyAdditiveBlends(psys, FALSE);
        partModifyStipple(psys, TRUE);
        break;
    default:
        partModifyAdditiveBlends(psys, FALSE);
        partModifyStipple(psys, FALSE);
    }
}


/*============================
  callbacks
 ============================*/
void partSetIsWorldspace(bool8 isWorldspace)
{
    if (isWorldspace)
    {
        bitSet(pat.flags, PART_WORLDSPACE);
    }
    else
    {
        bitClear(pat.flags, PART_WORLDSPACE);
    }
}

void partSetVelocityInWorldSpace(bool8 isLocalSpace)
{
    if (isLocalSpace)
    {
        bitSet(pat.flags, PART_WORLDSPACEVELOCITY);
    }
    else
    {
        bitClear(pat.flags, PART_WORLDSPACEVELOCITY);
    }
}

void partSetExponent(real32 exponent)
{
    pat.exponent = exponent;
}

void patSetExponentDist(real32 exponentDist)
{
    pat.exponentDist = exponentDist;
}

void partSetWorldVel(vector* worldVel)
{
    memcpy(&pat.worldVel, worldVel, sizeof(vector));
    partEffectVelocity = worldVel;
}

void partSetAnimation(void* tstruct)
{
    pat.tstruct = tstruct;
}

void partSetMorph(void* mstruct)
{
    pat.mstruct = mstruct;
}

void partSetMorphFramerate(real32 meshRate)
{
    pat.meshRate = meshRate;
}

void partSetMorphLoopCount(sdword loopCount)
{
    pat.loopCount = loopCount;
}

void partSetMeshStartFrame(real32 frame)
{
    pat.startMeshFrame = frame;
}

void partSetFramerate(real32 rate)
{
    pat.rate = rate;
}

void partSetLoopFlag(bool8 willLoop)
{
    pat.loop = willLoop;
}

void partSetStartFrame(udword frame)
{
    pat.startFrame = (uword)frame;
}

void partSetCoordSys(matrix* mat)
{
    pat.mat = mat;
}

void partSetTumble(real32 x, real32 y, real32 z)
{
    pat.tumble[0] = x;
    pat.tumble[1] = y;
    pat.tumble[2] = z;
}

void partSetTumbleDist(real32 dx, real32 dy, real32 dz)
{
    pat.tumbleDist[0] = dx;
    pat.tumbleDist[1] = dy;
    pat.tumbleDist[2] = dz;
}

void partSetDeltaTumble(real32 x, real32 y, real32 z)
{
    pat.deltaTumble[0] = x;
    pat.deltaTumble[1] = y;
    pat.deltaTumble[2] = z;
}

void partSetDeltaTumbleDist(real32 dx, real32 dy, real32 dz)
{
    pat.deltaTumbleDist[0] = dx;
    pat.deltaTumbleDist[1] = dy;
    pat.deltaTumbleDist[2] = dz;
}

void partSetSlices(udword s)
{
    if (s < 2 || s > 64)
    {
        dbgMessage("\ninsane number of slices, using default of 8");
        pat.slices = 8;
    }
    else
        pat.slices = s;
}

void partSetPosition(vector *pos)
{
    memcpy(pat.position, pos, sizeof(vector));
}

void partSetMeshdata(meshdata *m)
{
    pat.mesh = m;
}

void partSetTrHandle(trhandle t, udword u0, udword v0, udword u1, udword v1)
{
    pat.tex = t;
    pat.uv[0][0] = u0;
    pat.uv[0][1] = v0;
    pat.uv[1][0] = u1;
    pat.uv[1][1] = v1;
}

void partSetDrag(real32 n)
{
    pat.drag = n;
}

void partSetXYZScale(real32 x, real32 y, real32 z)
{
    bitSet(pat.flags, PART_XYZSCALE);
    pat.xScale = x;
    pat.yScale = y;
    pat.zScale = z;
}

void partSetScale(real32 n)
{
    pat.scale = n;
}

void partSetScaleDist(real32 d)
{
    pat.scaleDist = d;
    if (d == 0.0f)
        bitSet(pat.flags, PART_MONOSCALE);
    else
        bitClear(pat.flags, PART_MONOSCALE);
}

void partSetDeltaScale(real32 n)
{
    pat.deltaScale = n;
}

void partSetDeltaScaleDist(real32 d)
{
    pat.deltaScaleDist = d;
}

void partSetOffsetLOF(real32 n)
{
    pat.offsetLOF = n;
}

void partSetOffsetR(real32 n)
{
    pat.offsetR = n;
}

void partSetOffsetTheta(real32 n)
{
    pat.offsetTheta = n;
}

void partSetOffsetArray(vector *offsets)
{
    pat.offsetArray = offsets;
}

void partSetDeltaLOF(real32 n)
{
    pat.deltaLOF = n;
}

void partSetDeltaLOFDist(real32 n)
{
    pat.deltaLOFDist = n;
}

void partSetDeltaR(real32 r, real32 t)
{
    pat.deltaR[0] = r;
    pat.deltaR[1] = t;
}

void partSetDeltaRDist(real32 r, real32 t)
{
    pat.deltaRDist[0] = r;
    pat.deltaRDist[1] = (t == 0.0f) ? 6.28f : t;
}

void partSetVelLOF(real32 v)
{
    pat.velLOF = v;
}

void partSetVelLOFDist(real32 d)
{
    pat.velLOFDist = d;
}

void partSetVelR(real32 v)
{
    pat.velR = v;
}

void partSetVelRDist(real32 d)
{
    pat.velRDist = d;
}

void partSetDeltaVelLOF(real32 d)
{
    pat.deltaVelLOF = d;
}

void partSetDeltaVelLOFDist(real32 d)
{
    pat.deltaVelLOFDist = d;
}

void partSetDeltaVelR(real32 v)
{
    pat.deltaVelR = v;
}

void partSetDeltaVelRDist(real32 v)
{
    pat.deltaVelRDist = v;
}

void partSetAng(real32 a)
{
    pat.ang = a;
}

void partSetAngDist(real32 d)
{
    pat.angDist = d;
}

void partSetAngDelta(real32 d)
{
    pat.angDelta = d;
}

void partSetAngDeltaDist(real32 d)
{
    pat.angDeltaDist = d;
}

void partSetColor(real32 r, real32 g, real32 b)
{
    pat.icolor[0] = r;
    pat.icolor[1] = g;
    pat.icolor[2] = b;
    bitClear(pat.flags, PART_ALPHA);
}

void partSetColorDist(real32 r, real32 g, real32 b)
{
    pat.colorDist[0] = r;
    pat.colorDist[1] = g;
    pat.colorDist[2] = b;
    pat.colorDist[3] = 0.0f;
    bitClear(pat.flags, PART_ALPHA);
}

void partSetColorA(real32 r, real32 g, real32 b, real32 a)
{
    pat.icolor[0] = r;
    pat.icolor[1] = g;
    pat.icolor[2] = b;
    pat.icolor[3] = a;
    bitSet(pat.flags, PART_ALPHA);
}

void partSetColorADist(real32 r, real32 g, real32 b, real32 a)
{
    pat.colorDist[0] = r;
    pat.colorDist[1] = g;
    pat.colorDist[2] = b;
    pat.colorDist[3] = a;
    bitSet(pat.flags, PART_ALPHA);
}

void partSetDeltaColor(real32 r, real32 g, real32 b)
{
    pat.deltaColor[0] = r;
    pat.deltaColor[1] = g;
    pat.deltaColor[2] = b;
    pat.deltaColor[3] = 1.0f;
    bitClear(pat.flags, PART_ALPHA);
}

void partSetDeltaColorDist(real32 r, real32 g, real32 b)
{
    pat.deltaColorDist[0] = r;
    pat.deltaColorDist[1] = g;
    pat.deltaColorDist[2] = b;
    pat.deltaColorDist[3] = 0.0f;
    bitClear(pat.flags, PART_ALPHA);
}

void partSetDeltaColorA(real32 r, real32 g, real32 b, real32 a)
{
    pat.deltaColor[0] = r;
    pat.deltaColor[1] = g;
    pat.deltaColor[2] = b;
    pat.deltaColor[3] = a;
    bitSet(pat.flags, PART_ALPHA);
}

void partSetDeltaColorADist(real32 r, real32 g, real32 b, real32 a)
{
    pat.deltaColorDist[0] = r;
    pat.deltaColorDist[1] = g;
    pat.deltaColorDist[2] = b;
    pat.deltaColorDist[3] = a;
    bitSet(pat.flags, PART_ALPHA);
}

void partSetLighting(bool l)
{
    pat.lit = (bool8)l;
}

void partSetIllum(real32 n)
{
    pat.illum = n;
}

void partSetIllumDist(real32 n)
{
    pat.illumDist = n;
}

void partSetDeltaIllum(real32 n)
{
    pat.deltaIllum = n;
}

void partSetDeltaIllumDist(real32 n)
{
    pat.deltaIllumDist = n;
}

void partSetLifespan(real32 t)
{
    pat.lifespan = t;
}

void partSetLifespanDist(real32 d)
{
    pat.lifespanDist = d;
}

void partSetWaitspan(real32 t)
{
    pat.waitspan = t;
}

void partSetWaitspanDist(real32 d)
{
    pat.waitspanDist = d;
}

void partSetLength(real32 l)
{
    pat.length = l;
}

void partSetLengthDist(real32 d)
{
    pat.lengthDist = d;
}

void partSetDeltaLength(real32 d)
{
    pat.deltaLength = d;
}

void partSetDeltaLengthDist(real32 d)
{
    pat.deltaLengthDist = d;
}

void partSetColorBias(real32 rb, real32 gb, real32 bb)
{
    pat.rbias = rb;
    pat.gbias = gb;
    pat.bbias = bb;
}

void partSetColorBiasDist(real32 drb, real32 dgb, real32 dbb)
{
    pat.rbiasDist = drb;
    pat.gbiasDist = dgb;
    pat.bbiasDist = dbb;
}

void partSetSpecular(bool spec)
{
    pat.specularOnly = spec;
}

void partSetStipple(bool stip)
{
    pat.stippleAlpha = stip;
}

void partSetNoDepthWrite(bool noWrite)
{
    pat.noDepthWrite = noWrite;
}

void partSetAdditiveBlends(bool add)
{
    pat.additiveBlends = add;
}

void partSetPseudoBillboard(bool bill)
{
    pat.pseudoBillboard = bill;
}

void partSetTrueBillboard(bool bill)
{
    pat.trueBillboard = bill;
}

void partSetColorScheme(sdword colorScheme)
{
    pat.colorScheme = colorScheme;
}

//0 - normal, 1 - additive, 2 - stipple
void partSetAlphaMode(udword mode)
{
    switch (mode)
    {
    case 1:
        partSetAdditiveBlends(TRUE);
        partSetStipple(FALSE);
        break;
    case 2:
        partSetAdditiveBlends(FALSE);
        partSetStipple(TRUE);
        break;
    default:
        partSetAdditiveBlends(FALSE);
        partSetStipple(FALSE);
    }
}

void partCreateCallbackSet(void (*function)(sdword userValue, ubyte *userData), sdword userValue, ubyte *userData)
{
    partCreateCallback = function;
    partCreateUserValue = userValue;
    partCreateUserData = userData;
}

