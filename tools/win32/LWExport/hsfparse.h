/*
** HSFParse.h : Header file for HSFParse.cpp.
*/

#ifndef __HSFPARSE_H
#define __HSFPARSE_H

#define HSF_FILE_ID		"rrasberry"
#define HSF_VERSION		(0x00000100)

	//////////////////////////////////////////////////////////////////////////
	// HSF Headers.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagHSFFileHeader
	{
		char			identifier[12];					// File identifier.
		unsigned long	version;						// File version.
		unsigned long	nLights;						// Number of lights in the scene.
	} HSFFileHeader;

	typedef struct tagHSFLight
	{
		unsigned long type;
		float x, y, z;
		float h, p, b;
		float coneAngle;
		float edgeAngle;
		unsigned char red, green, blue;
		float intensity;
	} HSFLight;

	enum tagHSFLightTypes
	{
		HSFL_DistantLight,
		HSFL_PointLight,
		HSFL_SpotLight,
		HSFL_LinearLight,
		HSFL_AreaLight,
		HSFL_AmbientLight,

		HSFL_Last_HSFL
	};

	//////////////////////////////////////////////////////////////////////////
	// HSF Global Data.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagHSFGlobalData
	{
		HSFFileHeader		*pHSFFileHeader;

		unsigned long nLights;
		HSFLight *pLights;
	} HSFGlobalData;

	//////////////////////////////////////////////////////////////////////////
	// Prototypes.
	//////////////////////////////////////////////////////////////////////////
	void InitializeHSFParse(void);
	void DeInitializeHSFParse(void);

	void HSFParse_SetupLights(void);
	void HSFParse_SetupFileHeader(void);

	signed char HSFParse_WriteFile(FILE *fileHandle);

#endif