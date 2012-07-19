/*
** EXPORT.H : Header file for EXPORT.CPP.
*/

#ifndef __EXPORT_H
#define __EXPORT_H

#include "stdio.h"

#define SUCCESS					(0)
#define FAILURE					((signed char)(-1))

#define UNIQUE					(-1)

//macros to set/test/clear bits
#define bitSet(dest, bitFlags)      ((dest) |= (bitFlags))
#define bitClear(dest, bitFlags)    ((dest) &= ~(bitFlags))
#define bitTest(dest, bitFlags)     ((dest) & (bitFlags))

	//////////////////////////////////////////////////////////////////////////
	// Global Data Info.
	//////////////////////////////////////////////////////////////////////////

	typedef struct tagGlobalData
	{
		char				*inputFileName;		// From the command line, what is the input file name?
		char				*outputFileName;	// From the command line, what is the output file name?
		unsigned char		inputFileType;		// From the command line, what are we reading?
		unsigned char		outputFileType;		// From the command line, what are we writing?
        bool                bSurfaceNames;      // command-line switch to enable saving surface names
	} GlobalData;

	enum tagImportFormats
	{
		IF_NONE,
		IF_LWO,
		IF_LWS,

		IF_LAST_IF
	};

	enum tagExportFormats
	{
		EF_NONE,
		EF_GEO,
		EF_NIS,
		EF_HSF,
		EF_MAD,

		EF_LAST_EF
	};

	//////////////////////////////////////////////////////////////////////////
	// Prototypes.
	//////////////////////////////////////////////////////////////////////////
	
	void Initialize(void);
	void DeInitialize(void);

	void PrintHelp(void);

	signed char ParseCommandLine(int argc, char *argv[]);

	signed char ChooseFileTypesByExtension(void);

	signed char FindExplicitFile(char *fileName);
	signed char FindRootFile(char *fileName, char *root);
	signed char StripExtension(char *fileName);
	signed char MakeNiceFilename(char *pSrc, char *pDst);

	signed char Job(void);

	//////////////////////////////////////////////////////////////////////////
	// Externals.
	//////////////////////////////////////////////////////////////////////////
	extern GlobalData *globalData;

#endif