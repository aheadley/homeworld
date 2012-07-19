/*
** MEXPARSE.H : Header file for MEXPARSE.CPP.
*/

#ifndef __MEXPARSE_H
#define __MEXPARSE_H

#include "stdio.h"

#define MEX_FILE_ID		"mannngo"
#define MEX_VERSION		(0x00000102)

	//////////////////////////////////////////////////////////////////////////
	// MEX Headers.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagMEXFileHeader
	{
		char			identifier[8];					// File identifier.
		unsigned short	version;						// File version.
		unsigned short	nChunks;						// Number of chunks in the file.
	} MEXFileHeader;

	typedef struct tagMEXGunChunk
	{
		// Generic stuff.
		char			type[4];
		char			name[8];
		unsigned long	chunkSize;	

		// Specific stuff...
		float			xPos;
		float			yPos;
		float			zPos;

		float			xRot;
		float			yRot;
		float			zRot;

		float			coneAngle;
		float			edgeAngle;
	} MEXGunChunk;

	typedef struct tagMEXEngineChunk
	{
		// Generic stuff.
		char			type[4];
		char			name[8];
		unsigned long	chunkSize;	

		// Specific stuff...
		float			xPos;
		float			yPos;
		float			zPos;

		float			xRot;
		float			yRot;
		float			zRot;

		float			coneAngle;
		float			edgeAngle;
	} MEXEngineChunk;

	typedef struct tagMEXDamageChunk
	{
		// Generic stuff.
		char			type[4];
		char			name[8];
		unsigned long	chunkSize;

		// Specific stuff...
		float			xPos;
		float			yPos;
		float			zPos;

		float			xRot;
		float			yRot;
		float			zRot;

		float			coneAngle;
		float			edgeAngle;
	} MEXDamageChunk;

	typedef struct tagMEXCollisionSphereChunk
	{
		// Generic stuff.
		char			type[4];
		char			name[8];
		unsigned long	chunkSize;

		// Specific stuff...
		unsigned long	level;

		float			x;
		float			y;
		float			z;

		float			r;
	} MEXCollisionSphereChunk;

	typedef struct tagMEXCollisionRectangleChunk
	{
		// Generic stuff.
		char			type[4];
		char			name[8];
		unsigned long	chunkSize;

		// Specific stuff...
		unsigned long	level;

		float			ox;
		float			oy;
		float			oz;

		float			dx;
		float			dy;
		float			dz;
	} MEXCollisionRectangleChunk;

	typedef struct tagMEXDockingChunk
	{
		// Generic stuff.
		char			type[4];
		char			name[8];			// Always "DOCK"
		unsigned long	chunkSize;

		// Specific stuff...
		float			xPos;
		float			yPos;
		float			zPos;

		float			xRot;
		float			yRot;
		float			zRot;

		float			coneAngle;
		float			edgeAngle;

		char			dockName[20];		// Actual name.
	} MEXDockingChunk;

    typedef struct tagMEXSalvageChunk
    {
        // Generic stuff.
        char			type[4];
        char			name[8];			// Always "DOCK"
        unsigned long	chunkSize;
    
        // Specific stuff...
        float			xPos;
        float			yPos;
        float			zPos;
    
        float			xRot;
        float			yRot;
        float			zRot;
    
        float			coneAngle;
        float			edgeAngle;
    
        char			Name[20];		// Actual name.
    } MEXSalvageChunk;


	typedef struct tagMEXNAVLightChunk
	{
		// Generic stuff.
		char			type[4];
		char			name[8];			// Always "NAV"
		unsigned long	chunkSize;

		// Specific stuff...
		float			xPos;
		float			yPos;
		float			zPos;

		unsigned char	red;
		unsigned char	blue;
		unsigned char	green;
		unsigned char	pad;

		char			NAVLightName[20];		// Actual name. 
	} MEXNAVLightChunk; // 36 bytes...

	//////////////////////////////////////////////////////////////////////////
	// GEO Global Data.
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagMEXGlobalData
	{
		MEXFileHeader			*pMEXFileHeader;

		MEXGunChunk				*pGunChunks;
		unsigned long			nGunChunks;

		MEXEngineChunk			*pEngineChunks;
		unsigned long			nEngineChunks;

		MEXDamageChunk			*pDamageChunks;
		unsigned long			nDamageChunks;

		MEXDockingChunk			*pDockingChunks;
		unsigned long			nDockingChunks;
        
        MEXSalvageChunk			*pSalvageChunks;
		unsigned long			nSalvageChunks;

		MEXNAVLightChunk		*pNAVLightChunks;
		unsigned long			nNAVLightChunks;
		
		MEXCollisionSphereChunk	*pColChunks;
		unsigned long			nColChunks;

		MEXCollisionRectangleChunk *pColRectChunks;
		unsigned long				nColRectChunks;
	} MEXGlobalData;

	//////////////////////////////////////////////////////////////////////////
	// Prototypes.
	//////////////////////////////////////////////////////////////////////////
	void InitializeMEXParse(void);
	void DeInitializeMEXParse(void);

	signed char MEXParse_SetupMEXData(void);
	void MEXParse_SetupGuns(void);
	void MEXParse_SetupEngines(void);
	void MEXParse_SetupDocking(void);
	void MEXParse_SetupSalvage(void);
	void MEXParse_SetupNAVLights(void);
	void MEXParse_SetupCollisionSpheres(void);
	void MEXParse_SetupCollisionRectangles(void);
	
	signed char MEXParse_WriteFile(FILE *fileHandle);
	
	//////////////////////////////////////////////////////////////////////////
	// Externs.
	//////////////////////////////////////////////////////////////////////////
	extern MEXGlobalData	*gMEXData;

#endif