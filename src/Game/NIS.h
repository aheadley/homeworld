/*=============================================================================
    Name    : NIS.h
    Purpose : Non-Interactive Sequence code.

    Created 12/2/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___NIS_H
#define ___NIS_H

#ifndef ___TYPES_H
#include "Types.h"
#endif

#ifndef ___B_SPLINE_H
#include "B-Spline.h"
#endif

#include "Camera.h"
#include "SpaceObj.h"
#include "Task.h"
#include "FontReg.h"
#include "StatScript.h"
#include "texreg.h"

/*=============================================================================
    Switches:
=============================================================================*/

#define NIS_NORMALIZE_ANGLES        1           //convert incoming angular domensions to radians an get them in the range of 0..TWOPI
#define NIS_RENDERLIST_DISABLE      0           //stop updating render list when playing an NIS

#ifndef HW_Release

#define NIS_ERROR_CHECKING          1           //general error checking
#define NIS_VERBOSE_LEVEL           3           //control specific output code
#define NIS_DRAW_CAMERA             1           //draw the NIS camer while not looking from it's POV
#define NIS_TIME_CONTROLS           1           //key to pause the NIS's
#define NIS_PRINT_INFO              NUMPAD6     //key to print NIS time info
#define NIS_CAMERA_RELEASE          NUMPAD4     //allow release of the NIS camera to the standard game camera
#define NIS_SHIPS_SELECTABLE        1           //make NIS ships selectable
#define NIS_SEEKABLE                1           //can seek in NIS's
#define NIS_TEST                    1

#else //HW_Debug

#define NIS_ERROR_CHECKING          0           //general error checking
#define NIS_VERBOSE_LEVEL           0           //control specific output code
#define NIS_DRAW_CAMERA             0           //draw the NIS camer while not looking from it's POV
#define NIS_PAUSE_KEY               0           //key to pause the NIS's
#define NIS_PRINT_INFO              0           //key to print NIS time info
#define NIS_CAMERA_RELEASE          0           //allow release of the NIS camera to the standard game camera
#define NIS_SHIPS_SELECTABLE        0           //make NIS ships selectable
#define NIS_SEEKABLE                1           //can seek in NIS's
#define NIS_TEST                    0

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//general
#define NIS_VersionNumber           0x101
#define NIS_Identifier              "Crannnberry"
#define NIS_OneKeyframe             5
#define NIS_FirstKeyFrame           2
#define NIS_FrameRate               30.0f
#define NIS_LetterHeight            ((MAIN_WindowHeight - MAIN_WindowWidth * 20 / 37) / 2)          //1.85:1
#define NIS_HeaderExtra             64          //extra amount of RAM at the end of a header
#define NIS_LookScalar              10000.0f

//additional key controls
#define NIS_Pause                   NUMPAD5
#define NIS_FastForwardRate         4.0f        //play at 4x speed
#define NIS_FastForward             NUMPAD0
#define NIS_NormalSpeed             NUMDOTKEY
#define NIS_Back1Second             NUMPAD2
#define NIS_Back10Second            NUMPAD3

//objectmotion flags
#define OMF_ObjectDied              0x00000001
#define OMF_RemainAtEnd             0x00000002
#define OMF_Firing                  0x00000004
#define OMF_FocusAtEnd              0x00000008
#define OMF_KeepOnMoving            0x00000010
#define OMF_CurvesDeleted           0x00000020

//flags for an NIS header
#define NHF_RacesSwapped            0x00000001
#define NIS_Seeked                  0x00000002  //seeked recently

#define NIS_DefaultFOV              75.0f

#define NIS_NumberTextCards         16
#define NIS_NewLine                 "#n"
#define NIS_NewLineLength           2

//75 deg trigger happy cone
#define NIS_TriggerHappyAngle       0.7071067811865f

#define NIS_NCameraKeys             5
#define NIS_CameraLengthFactor      0.1f
#define NIS_CameraBlendPullBackFactor   2.5f
#define NIS_JumpDistance            300.0f

#define NIS_CentreShip              BIT15       //this bit in the type field indicates the ship is to be the centre of the NIS

//flags for actor selection
#define NIS_ActorMask               0xe0000000
#define NIS_ActorShiftBits          29

#define NIS_PlayedProperlyMargin    0.99f

#define NIS_EnvNameLength           32
#define NIS_NumberEvents            256

//static
#define NIS_NoiseTextureWidth       64
#define NIS_NoiseTextureHeight      8
#define NIS_NumberStatics           3
#define NIS_NumberExtraStatics      1
#define NIS_SMStaticIndex           NIS_NumberStatics

/*=============================================================================
    Type definitions:
=============================================================================*/
//spaceobj motion path
typedef struct spaceobjpath
{
    sdword instance;                            //instance of this type object
    ShipType type;                              //type of ship
    sdword parentIndex;                         //parent ship (NULL if none)
    udword race;                                //0..5 (see racedefs.h).  For determining what exact ship to use.
    sdword nSamples;                            //number of samples, including 2 extra at start and end
    real32 timeOffset;                          //if positive, path will wait this long before starting
    real32 *times;                              //absolute times, from start of NIS (timeOffset will be added in)
    tcb *parameters;                            //tension, continuity, bias
    real32 *curve[6];                           //x,y,z,h,p,b
}
spaceobjpath;

//camera motion path
typedef struct tagcamerapath
{
    real32 oLength;                             //length (seconds) of motion path
    sdword nSamples;                            //number of samples, including 2 extra at start and end
    real32 timeOffset;                          //if positive, path will wait this long before starting
    real32 *times;                              //absolute times, from start of NIS (timeOffset will be added in)
    tcb *parameters;                            //tension, continuity, bias
    real32 *curve[6];                           //same parameters as the spaceobj's
} camerapath;

//also needed later: camera and light motion paths
typedef spaceobjpath lightpath;

typedef enum
{
    NEO_DeathDamage,
    NEO_DeathProjectile,
    NEO_DeathBeam,
    NEO_DeathCollision,
    NEO_Attack,
    NEO_HaltAttack,
    NEO_Fire,
    NEO_Show,
    NEO_Hide,
    NEO_Invincible,
    NEO_Vincible,
    NEO_SoundEvent,
    NEO_SpeechEvent,
    NEO_DisableDefaultSpeech,
    NEO_EnableDefaultSpeech,
    NEO_CustomEffect,
    NEO_CustomGunEffect,
    NEO_GunShoot,
    NEO_RemainAtEnd,
    NEO_FleetSpeechEvent,
    NEO_CameraFOV,
    NEO_CameraCut,
    NEO_BlackScreenStart,
    NEO_BlackScreenEnd,
    NEO_TextCard,
    NEO_TextScroll,
    NEO_ScissorOut,
    NEO_Focus,
    NEO_FocusAtEnd,
    NEO_StartMusic,
    NEO_StopMusic,
    NEO_BlackFadeSet,
    NEO_BlackFadeTo,
    NEO_ColorScheme,
    NEO_MeshAnimationStart,
    NEO_MeshAnimationStop,
    NEO_MeshAnimationPause,
    NEO_MeshAnimationSeek,
    NEO_BlendCameraEndPoint,
    NEO_MusicVolume,
    NEO_SpeechVolume,
    NEO_SFXVolume,
    NEO_TrailMove,
    NEO_TrailZeroLength,
    NEO_UniversePauseToggle,
    NEO_UniverseHideToggle,
    NEO_NewEnvironment,
    NEO_LinkEnvironment,
    NEO_AnimaticSpeechEvent,
    NEO_SMPTEOn,
    NEO_SMPTEOff,
    NEO_StaticOn,
    NEO_StaticOff,
    NEO_HideDerelict,
    NEO_ShowDerelict,
    NEO_StartNewNIS,
    NEO_Attributes,
    NEO_AttackKasSelection,
    NEO_KeepMovingAtEnd,
    NEO_DamageLevel,
    NEO_MinimumLOD,
    NEO_LastNEO
}
nisopcode;

//structure for a NIS time event
typedef struct
{
    real32 time;                                //time of the event
    nisopcode code;                             //what event it is
    sdword shipID;                              //index of the object this event works on, not always a ship
    sdword initialID;                           //index of event in the .script file for correctly time-sorting events
    sdword param[2];                            //parameters, event-specific
}
nisevent;

//parameters for a SMPTE time counter
typedef struct
{
    sdword x, y;                                //where to print the timecode
    bool8 bRightJustified;                      //is it right justified?
    real32 startTime;                           //when it was started (relative to now)
    fonthandle font;                            //font to print it with
    color c;                                    //color to print it in
    char *format;                               //formatting string
}
nisSMPTE;

//patameters for NEO_StaticOn
typedef struct
{
    sdword index;                               //there can be multiple static effects going on at once
    sdword nLines;                              //average number of lines per frame
    sdword nLinesVariation;                     //variation in number of lines
    real32 width, widthVariation;               //horizontal size of static elements
    real32 y, yVariation;                       //vertical position and variation
    real32 hue, hueVariation;                   //colors and variations in HLS space
    real32 lum, lumVariation;
    real32 sat, satVariation;
    real32 alpha, alphaVariation;               //alpha parameters, if applicable
    bool8 bAlpha;                               //does this type of static use alpha?
    bool8 bTextured;
    trhandle texture;                           //optional texture handle if the elements are textured
}
nisstatic;

//for linking in generic objects
typedef struct
{
    DerelictStaticInfo *staticInfo;             //pointer to the data for this object
    char name[1];
}
nisgenericobject;

//structure for a whole NIS
typedef struct
{
    char identifier[12];                        //NIS_Identifier
    udword version;                             //NIS_VersionNumber
    udword flags;                               //general flags (dynamic)
    char *stringBlock;                          //pointer to string block
    sdword stringBlockLength;                   //length of the string block
    real32 length;                              //length (seconds) of NIS
    real32 loop;                                //loop point for NIS (or negative for no loop)
    sdword nObjectPaths;                        //number of motion paths in the file
    spaceobjpath *objectPath;                   //pointer to list of motion paths
    sdword nCameraPaths;                        //number of camera motion paths
    camerapath *cameraPath;
    sdword nLightPaths;                         //number of light motion paths
    lightpath *lightPath;
    sdword nEvents;                             //number of events, NULL for now
    nisevent *events;                           //void * because not initially used
    //after here there are NIS_HeaderExtra bytes of data
    //to be used by loaded data.  It is not stored in the file.
    sdword nGenericObjects;                     //number of different generic objects loaded
    nisgenericobject **genericObject;           //registry of genric objects loaded for this NIS
    sdword iLookyObject;                        //the official "mid point of 2-phase focus zoom look at point" object
    ubyte extra[NIS_HeaderExtra - 12];          //data for future expansion
}
nisheader;

//structures for animating spaceobj's, cameras and lights
typedef struct
{
    SpaceObjRotImp *spaceobj;                   //object controlled by these splines
    sdword parentIndex;
    splinecurve *curve[6];                      //motion paths (x,y,z,h,p,b)
    udword flags;                               //flags
//    bool8  bObjectDied;                         //this object has died a natural death
}
objectmotion;

typedef struct
{
    Camera *cam;                                //actual camera structure
    splinecurve *curve[6];                      //motion paths (x,y,z,h,p,b)
}
cameramotion;

//... structure for camera and light motion too

//structure for an NIS currently playing
typedef struct
{
    nisheader *header;                          //pointer to header which contains motion path data, should it ever be needed
    vector nisPosition;                         //where the nis is playing
    matrix nisMatrix;                           //attitude of the playing NIS
    real32 timeElapsed;                         //time elapsed since NIS started
    sdword nObjects;                            //number of spaceobj's being updated in this NIS
    splinecurve *objectSplines;                 //allocated pool of object paths
    objectmotion *objectsInMotion;              //spaceobj's being updated by this NIS
//    sdword nLights;                           //additional info for lights and cameras
//  ... lightmotion *lightsInMotion;
    sdword nCameras;
    splinecurve *cameraSplines;                 //allocated pool of camera paths
    cameramotion *camerasInMotion;
    sdword iCurrentEvent;
}
nisplaying;

//structure for text cards currently visible on-screen
typedef struct
{
    real32 creationTime;                        //when this text card was created
    real32 duration;                            //how long this text card is supposed to last
    real32 fadeIn;                              //duration of fade in from black
    real32 fadeOut;                             //duration of fade out to black, if any
    color c;                                    //color of text card
    fonthandle font;                            //font to draw in
    sdword x, y;                                //where to draw the text
    sword margin;                               //margin for use with multi-line text
    char *text;                                 //what text to draw
    nisplaying *NIS;                            //NIS this text card belongs to
}
nistextcard;

typedef void (*nisDoneCB)(void);

typedef struct
{
    float theta, phi;
    char name[NIS_EnvNameLength];
}
nisenvironment;

typedef void (*niseventdispatch)(nisplaying* NIS, nisevent* event);

/*=============================================================================
    Data:
=============================================================================*/
extern bool nisIsRunning;
extern bool nisCaptureCamera;
extern Camera *nisCamera;
extern taskhandle nisTaskHandle;
#if NIS_PRINT_INFO
extern char nisInfoString[256];
extern bool nisPrintInfo;
extern bool nisNoLockout;
#endif
#if NIS_TEST
nisplaying *testPlaying;
#endif

extern nisplaying *thisNisPlaying;
extern nisheader *thisNisHeader;
extern nisDoneCB thisNisDoneCB;
extern sdword nisTextCardIndex;

extern bool nisFullyScissored;                  //is scissor fully opaque?
extern real32 nisScissorFade;                   //current fade amount (0..1)
extern real32 nisScissorFadeIn;                 //duration of fade in/out
extern real32 nisScissorFadeOut;                //duration of fade in/out
extern real32 nisScissorFadeTime;               //current fade time
extern real32 nisCameraCutTime;
extern real32 nisBlackFade;                     //the current fade level (0 = normal, 1 = fully faded out)

extern bool8 nisUniversePause;
extern bool8 nisUniverseHidden;

extern niseventdispatch nisEventDispatch[];
extern scriptEntry nisScriptTable[];
extern sdword nisEventIndex;
extern nisheader* nisCurrentHeader;
extern nisevent* nisEventList;
extern int nisEventSortCB(void const* p1, void const* p2);

extern nisSMPTE *nisSMPTECounter;
extern nisstatic *nisStatic[NIS_NumberStatics + NIS_NumberExtraStatics];

/*=============================================================================
    Macros:
=============================================================================*/
#define nisSize(n)  (sizeof(nisplaying) + sizeof(splinecurve *) * ((n) - 1))
#define nisGenericObjectSize(n)     (sizeof(nisgenericobject) + n)

/*=============================================================================
    Functions:
=============================================================================*/
//start/shutdown the module
void nisStartup(void);
void nisShutdown(void);

//load/delete objects
nisheader *nisLoad(char *fileName, char *scriptName);
void nisDelete(nisheader *header);

//play an NIS
nisplaying *nisStart(nisheader *header, vector *position, matrix *coordSystem);
void nisCamPathUpdate(nisplaying *NIS, cameramotion *camPath, real32 timeElapsed);
real32 nisUpdate(nisplaying *NIS, real32 timeElapsed);
void nisObjectEulerToMatrix(matrix *coordsys, vector *rotVector);
void nisShipEulerToMatrix(matrix *coordsys, vector *rotVector);
void nisStop(nisplaying *NIS);
void nisPause(bool bPause);

void nisGoToEnd(nisplaying *NIS);

//utility functions for others to use
bool nisShipStartPosition(vector *destVector, matrix *destMatrix, nisheader *header, ShipRace race, ShipType type, sdword instance);
void nisRotateAboutVector(real32* m, vector* axis, real32 radians);
void nisTextCardListDraw(void);
void nisRemoveMissileReference(Missile *missile);
real32 nisFocusMaxRadius(vector *centre, MaxSelection *selection);
void nisFocusPointCompute(vector *centre, MaxSelection *selection);
void nisSMPTECounterDraw(nisplaying *NIS, nisSMPTE *SMPTE);
void nisStaticDraw(nisstatic *snow);
void nisStaticOnExp(nisstatic *newStatic);
void nisStaticOffExp(sdword index);

void nisTaskPause(void);
void nisTaskResume(void);
void nisTaskPauseOrResume(void);
void nisObjectDied(SpaceObj *obj);

void nisScissorBarsReset(void);

#if NIS_TEST
void nisTest(vector *position, matrix *mat);
void nisTestAnother(sdword skip);
#endif


#endif //___NIS_H

