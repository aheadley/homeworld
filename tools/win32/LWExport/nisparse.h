/*
** NISPARSE.H : Header file for NISPARSE.CPP.
*/

#ifndef __NISPARSE_H
#define __NISPARSE_H

#include "vector.h"
#include "cclist.h"
#include "lwoparse.h"

#define NIS_FILE_ID		"Crannnberry"
#define NIS_VERSION		(0x00000101)

//special 'races' for different types of objects besides ships
#define NSR_Asteroid        (NUM_RACES + 0)
#define NSR_DustCloud       (NUM_RACES + 1)
#define NSR_GasCloud        (NUM_RACES + 2)
#define NSR_Nebula          (NUM_RACES + 3)
#define NSR_Derelict        (NUM_RACES + 4)
#define NSR_Effect          (NUM_RACES + 5)
#define NSR_Generic         (NUM_RACES + 6)

#define NIS_HeaderExtra     64

typedef float		timeindex;

typedef struct tagtcb
{
	float tension;
	float continuity;
	float bias;
} tcb;

typedef struct tagspaceobjectpath
{
    unsigned long	oLength;      //length (seconds) of motion path
    unsigned long	oName;        //name of ship
	unsigned long	oParentName;  //name of parent object (if any).
    unsigned long	race;         //0..5 (se racedefs.h).  For determining what exact ship to use
    signed long		nSamples;     //number of samples, including 2 extra at start and end
    float			timeOffset;   //if positive, path will wait this long before starting
    unsigned long	oTimes;       //absolute times, from start of NIS (timeOffset will be added in)
    unsigned long   oParameters;  //tension, continuity, bias
    unsigned long	oX;			  //x, y, z	
    unsigned long	oY;
    unsigned long	oZ;
    unsigned long	oH;           //heading, pitch, bank
    unsigned long	oP;
    unsigned long	oB;
} spaceobjpath;

typedef struct tagcamerapath
{
	unsigned long	oLength;      //length (seconds) of motion path
    signed long		nSamples;     //number of samples, including 2 extra at start and end
    float			timeOffset;   //if positive, path will wait this long before starting
    unsigned long	oTimes;       //absolute times, from start of NIS (timeOffset will be added in)
    unsigned long   oParameters;  //tension, continuity, bias
    unsigned long	oX;			  //x, y, z	
    unsigned long	oY;
    unsigned long	oZ;
    unsigned long	oH;           //heading, pitch, bank
    unsigned long	oP;
    unsigned long	oB;
} camerapath;

typedef struct taglightpath
{
	char temp;
} lightpath;

typedef struct tagevent
{
	char temp;
} event;

typedef struct tagNISFileHeader
{
    char identifier[12];                        //NIS_Identifier
    unsigned long version;                      //NIS_VersionNumber
    unsigned long oName;                        //name of NIS (for debugging)
    unsigned long oStringBlock;                 //pointer to string block
	unsigned long sStringBlock;					//size of the string block.
    timeindex tiNISLength;                      //length (seconds) of NIS
    timeindex tiLoopPoint;                      //loop point for NIS (or negative for no loop)
    signed long nObjectPaths;                   //number of motion paths in the file
    unsigned long oObjectPaths;                 //pointer to list of motion paths
    signed long nCameraPaths;                   //number of camera motion paths
    unsigned long oCameraPaths;
    signed long nLightPaths;                    //number of light motion paths
    unsigned long oLightPaths;
    unsigned long nEvents;                      //number of events, NULL for now
    unsigned long oEvents;                      //void * because not initially used
    ubyte extra[NIS_HeaderExtra];               //extra data used at run-time
} NISFileHeader;

typedef struct tagNISGloblaData
{
	NISFileHeader	*pNISFileHeader;
	
	unsigned long	sStringBlock;
	char *			pStringBlock;

	unsigned long	oLengthArrays;
	unsigned long	oTimeArrays;
	unsigned long	oParameterArrays;
	unsigned long	oXArrays;
	unsigned long	oYArrays;
	unsigned long	oZArrays;
	unsigned long	oHArrays;
	unsigned long	oPArrays;
	unsigned long	oBArrays;

	unsigned long	nKeyframes;
	unsigned long	nObjects;
	unsigned long	nCameras;
	unsigned long	nLights;
	unsigned long	nEvents;
	
	spaceobjpath	*pObjectKeyframes;
	camerapath		*pCameraKeyframes;
	lightpath		*pLightKeyframes;
	event			*pEventKeyframes;
	timeindex		*pLengths;
	timeindex		*pTimes;
	tcb				*pTCB;
	float			*pX;
	float			*pY;
	float			*pZ;
	float			*pH;
	float			*pP;
	float			*pB;
} NISGlobalData;

void InitializeNISParse(void);
void DeInitializeNISParse(void);

void NISParse_SetupNISFileHeader(void);

void NISParse_CreateKeyframeList(void);
timeindex CalcTimeIndex(LWOKeyframe *pLWOKeyframe);

void NISParse_CalcStringBlockSize(ccList *stringList);
void NISParse_BuildStringBlock(void);

signed char NISParse_WriteFile(FILE *fileHandle);

#endif