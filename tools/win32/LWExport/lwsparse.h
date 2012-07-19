/*
** LWSPARSE.H : Header file for LWSPARSE.CPP.
*/

#ifndef __LWSPARSE_H
#define __LWSPARSE_H

#include "parsefunc.h"
#include "cclist.h"

#define NUM_LWS_PARSE_FUNCS			(13)

	//////////////////////////////////////////////////////////////////////////
	// LWS Structures.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagLWSLight
	{
		char *name;

		unsigned char type;

		float xPos, yPos, zPos;
		float xRot, yRot, zRot;
		float xVec, yVec, zVec;
		
		float coneAngle;
		float edgeAngle;

		unsigned char red, green, blue;
		float intensity;
	} LWSLight;

	typedef struct tagCameraKeyframe
	{
		float xp, yp, zp;
		float h, p, b;
		float xv, yv, zv;
		float xs, ys, zs;
		unsigned long frameNumber;
		float linearValue;
		float tension;
		float continuity;
		float bias;
	} CameraKeyframe;

	typedef struct tagLWSCamera
	{
		ccList *pKeyframeList;
	} LWSCamera;

	//////////////////////////////////////////////////////////////////////////
	// LWS Container classes.
	//////////////////////////////////////////////////////////////////////////
	class LWSLightContainer : public ccNode
	{
	private:
	protected:
	public:
		LWSLight myLight;

		LWSLightContainer(void);
		~LWSLightContainer(void);
	};

	class CameraKeyframeContainer : public ccNode
	{
	private:
	protected:
	public:
		CameraKeyframe myKeyframe;

		CameraKeyframeContainer(void);
		~CameraKeyframeContainer(void);
	};

	//////////////////////////////////////////////////////////////////////////
	// LWS Global Data.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagLWSGlobalData
	{
		FunctionDef *pLWSFuncList;

		ccList *pLightList;
		LWSLight *pCurrentLight;

		LWSCamera *pCamera;

		// Animation data...
		unsigned long firstFrame;
		unsigned long lastFrame;
		unsigned long frameStep;
		float framesPerSecond;

		unsigned char ambientRed, ambientBlue, ambientGreen;
		float ambientIntensity;
	} LWSGlobalData;

	//////////////////////////////////////////////////////////////////////////
	// Prototypes.
	//////////////////////////////////////////////////////////////////////////
	void InitializeLWSParse(void);
	void DeInitializeLWSParse(void);

	signed char CalculateLWObjectNames(void);
	signed char FixupLWOParentIndices(void);

	void ReorientLWSData(void);
	
	signed char LWSParse_ReadFile(FILE *fileHandle);
	signed char LWSParse_LoadObject(char *currentLine, FILE *fileHandle);
	signed char LWSParse_PivotPoint(char *currentLine, FILE *fileHandle);
	signed char LWSParse_AmbientColor(char *currentLine, FILE *fileHandle);
	signed char LWSParse_AmbIntensity(char *currentLine, FILE *fileHandle);

	LWSLight *CreateNewLight(void);
	signed char LWSParse_AddLight(char *currentLine, FILE *fileHandle);
	signed char LWSParse_LightName(char *currentLine, FILE *fileHandle);
	signed char LWSParse_ShadowType(char *currentLine, FILE *fileHandle);
	signed char LWSParse_LightMotion(char *currentLine, FILE *fileHandle);
	signed char LWSParse_ConeAngle(char *currentLine, FILE *fileHandle);
	signed char LWSParse_EdgeAngle(char *currentLine, FILE *fileHandle);
	signed char LWSParse_LightType(char *currentLine, FILE *fileHandle);
	signed char LWSParse_LightColor(char *currentLine, FILE *fileHandle);
	signed char LWSParse_LgtIntensity(char *currentLine, FILE *fileHandle);
	
	signed char LWSParse_ParentObject(char *currentLine, FILE *fileHandle);
	signed char LWSParse_ObjectMotion(char *currentLine, FILE *fileHandle);
	signed char LWSParse_FirstFrame(char *currentLine, FILE *fileHandle);
	signed char LWSParse_LastFrame(char *currentLine, FILE *fileHandle);
	signed char LWSParse_FrameStep(char *currentLine, FILE *fileHandle);
	signed char LWSParse_FramesPerSecond(char *currentLine, FILE *fileHandle);

	signed char LWSParse_CameraMotion(char *currentLine, FILE *fileHandle);

	signed char LWSParse_AddNullObject(char *currentLine, FILE *fileHandle);

	//////////////////////////////////////////////////////////////////////////
	// Externs.
	//////////////////////////////////////////////////////////////////////////
	extern LWSGlobalData	*gLWSData;

#endif