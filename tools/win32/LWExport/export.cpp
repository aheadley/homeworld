/*
** EXPORT.CPP : Main control code for exporter.
*/

#include "string.h"
#include "stdlib.h"
#include "memory.h"
#include "assert.h"
#include "stdio.h"
#include "io.h"

#include "export.h"

#include "lwsparse.h"
#include "lwoparse.h"
#include "mexparse.h"
#include "geoparse.h"
#include "nisparse.h"
#include "hsfparse.h"
#include "madparse.h"

GlobalData	*globalData;

void InitializeGlobals(void)
{
	globalData = new GlobalData;

	memset(globalData, 0x00, sizeof(GlobalData));

	// Initialize specifics.
}

void DeInitializeGlobals(void)
{
	if(globalData->inputFileName)
		delete [] globalData->inputFileName;

	if(globalData->outputFileName)
		delete [] globalData->outputFileName;

	delete globalData;
}

void Initialize(void)
{
	InitializeGlobals();
	
	InitializeLWSParse();
	InitializeLWOParse();
}

void DeInitialize(void)
{
	DeInitializeLWSParse();
	DeInitializeLWOParse();

	switch(globalData->outputFileType)
	{
	case EF_GEO:
		DeInitializeGEOParse();
		DeInitializeMEXParse();
		break;

	case EF_NIS:
		DeInitializeNISParse();
		break;

	case EF_HSF:
		DeInitializeHSFParse();
		break;
	}
	
	DeInitializeGlobals();
}

void PrintHelp(void)
{
	printf("LWExport\t: Exports data from Lightwave Modeler and Layout.\n");
	printf("Usage   \t: LWExport [options] -+[input file name] -=[output file name]\n");
	printf("        \t: [input file name]  : Full path to input file (.LWO or .LWS)\n");
	printf("        \t: [output file name] : Full path to output file name (.GEO, .NIS, or .HSF)\n");
    printf("Valid options:\n");
    printf("    /S: include surface names in .geo files.\n");
}

// One for the executable name and 2 for the input and output filename.
#define PCL_NUMARGS			(1 + 2)

signed char ParseCommandLine(int argc, char *argv[])
{
	int i;

	if(argc < PCL_NUMARGS)
	{
		PrintHelp();

		return(FAILURE);
	}

	// Start at 1 to skip .EXE name.
	for( i=1 ; i<argc ; i++)
	{
		if(!strncmp(argv[i], "-+", 2))
		{
			// Grab input file name.

			// +1 for NULL, -2 for -+.
			globalData->inputFileName = new char [strlen(argv[i]) + 1 - 2];

			strcpy(globalData->inputFileName, argv[i]+2);
		}

		else if(!strncmp(argv[i], "-=", 2))
		{
			// Grab output file name.

			// +1 for NULL, -2 for -+.
			globalData->outputFileName = new char [strlen(argv[i]) + 1 - 2];

			strcpy(globalData->outputFileName, argv[i]+2);
		}

        else if (!strncmp(argv[i], "/", 1) || !strncmp(argv[i], "-", 1))
        {                                                   //it's a command-line option
            switch (toupper(argv[i][1]))
            {
                case 'S':
                    globalData->bSurfaceNames = (bool)TRUE;
                    break;
                default:
        			printf("LWExport: Unrecognised command line option ('%s')\n", argv[i]);

        			return(FAILURE);
            }
        }

		else
		{
			printf("LWExport: Unrecognised command line parameter (%d)\n", i);

			return(FAILURE);
		}
	}

	char tempInName[1000];
	char tempOutName[1000];

	if(!globalData->inputFileName)
	{
		printf("LWExport: You didn't supply an input file name.\n");

		return(FAILURE);
	}

	if(!globalData->outputFileName)
	{
		printf("LWExport: You didn't supply an output file name.\n");

		return(FAILURE);
	}
	
	MakeNiceFilename(globalData->inputFileName, tempInName);
	MakeNiceFilename(globalData->outputFileName, tempOutName);
	
	printf("LWExport: Converting %s to %s\n", tempInName, tempOutName);

	return(SUCCESS);
}

signed char ChooseFileTypesByExtension(void)
{
	char *pTemp;

	pTemp = globalData->inputFileName + strlen(globalData->inputFileName);

	while((*(--pTemp) != '.') && (pTemp >= globalData->inputFileName));

	if(pTemp < globalData->inputFileName)
	{
		printf("LWExport: Input file must have extension.\n");

		return(FAILURE);
	}

	strupr(pTemp);
	
	if(!strcmp(pTemp, ".LWS"))
		globalData->inputFileType = IF_LWS;

	else
	{
		printf("LWExport: Currently unsupported input format (must be .LWS).\n");

		return(FAILURE);
	}
	
	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////
	
	pTemp = globalData->outputFileName + strlen(globalData->outputFileName);

	while((*(--pTemp) != '.') && (pTemp >= globalData->outputFileName));

	if(pTemp < globalData->outputFileName)
	{
		printf("LWExport: Output file must have extension.\n");

		return(FAILURE);
	}

	strupr(pTemp);
	
	if(!strcmp(pTemp, ".GEO"))
		globalData->outputFileType = EF_GEO;

	else if(!strcmp(pTemp, ".NIS"))
		globalData->outputFileType = EF_NIS;

	else if(!strcmp(pTemp, ".HSF"))
		globalData->outputFileType = EF_HSF;

    else if (!strcmp(pTemp, ".MAD"))
		globalData->outputFileType = EF_MAD;

	else
	{
		printf("LWExport: Currently unsupported output format (must be one of .GEO, .NIS, .HSF or .MAD).\n");

		return(FAILURE);
	}

	return(SUCCESS);
}

signed char FindExplicitFile(char *fileName)
{
	struct _finddata_t fileInfo;

	if(_findfirst(fileName, &fileInfo) != -1)
		return(SUCCESS);

	printf("LWExport: Unable to find file '%s'.\n", fileName);

	return(FAILURE);
}

signed char FindRootFile(char *fileName, char *root)
{
	char *pTempSrc, *pTempDst;;

	assert(root);

	// Fix up the drive.
	if((pTempSrc = strchr(root, ':')) && (pTempDst = strchr(fileName, ':')))
	{
		// Get back to the drive letter.
		pTempSrc --;
		pTempDst --;

		// Make sure there is a drive to fix up.
		pTempDst[0] = pTempSrc[0];
	}
	else
	{
		pTempDst = fileName;
	}

	return(FindExplicitFile(pTempDst));
}

signed char StripExtension(char *fileName)
{
	char *pTemp;

	if(pTemp = strchr(fileName, '.'))
		*pTemp = 0x00;

	return(SUCCESS);
}

signed char MakeNiceFilename(char *pSrc, char *pDst)
{
	char *pTemp = pSrc;

	while(pTemp = strchr(pTemp, '\\'))
	{
		pTemp++;

		if(pTemp)
			strcpy(pDst, pTemp);
	}

	strupr(pDst);

	return(SUCCESS);
}

signed char Job(void)
{
	signed char retVal;
	FILE *fileHandle;
	
	switch(globalData->inputFileType)
	{
	case IF_LWS:
		fileHandle = fopen(globalData->inputFileName, "rt");

		assert(fileHandle);

#ifdef ANAL_DEBUGGING		
		printf("LWExport: Importing %s\n", globalData->inputFileName);
#endif

		retVal = LWSParse_ReadFile(fileHandle);
		break;
	}

	fclose(fileHandle);

	if(retVal != SUCCESS)
		return(retVal);

    fileHandle = fopen(globalData->outputFileName, "wb");

    if(!fileHandle)
    {
        // File is probably read only.
        printf("ERROR %s cannot be opened for writing (not checked out?).\n", globalData->outputFileName);
        return(FAILURE);
    }
	switch(globalData->outputFileType)
	{
    	case EF_GEO:

            fileHandle = fopen(globalData->outputFileName, "wb");

            if(!fileHandle)
            {
                // File is probably read only.
                printf("ERROR %s cannot be opened for writing (not checked out?).\n", globalData->outputFileName);
                return(FAILURE);
            }
    		retVal = GEOParse_WriteFile(fileHandle);

    		if(retVal == SUCCESS)
    		{
    			// Also export the mesh extensions file.
    			char tempFileName[1000];
    			
    			fclose(fileHandle);

    			strcpy(tempFileName, globalData->outputFileName);

    			StripExtension(tempFileName);

    			strcat(tempFileName, ".MEX");

    			fileHandle = fopen(tempFileName, "wb");
    			
    			if(!fileHandle)
    			{
    				// File is probably read only.
    				printf("%s is not checked out.\n", tempFileName);
    				return(FAILURE);
    			}	

    			retVal = MEXParse_WriteFile(fileHandle);
    		}
    		break;

    	case EF_NIS:

            fileHandle = fopen(globalData->outputFileName, "wb");

            if(!fileHandle)
            {
                // File is probably read only.
                printf("ERROR %s cannot be opened for writing (not checked out?).\n", globalData->outputFileName);
                return(FAILURE);
            }
    		printf("LWExport: Exporting %s\n", globalData->outputFileName);
    		retVal = NISParse_WriteFile(fileHandle);
    		break;

    	case EF_HSF:
            fileHandle = fopen(globalData->outputFileName, "wb");

            if(!fileHandle)
            {
                // File is probably read only.
                printf("ERROR %s cannot be opened for writing (not checked out?).\n", globalData->outputFileName);
                return(FAILURE);
            }
    		printf("LWExport: Exporting %s\n", globalData->outputFileName);
    		retVal = HSFParse_WriteFile(fileHandle);
    		break;
        case EF_MAD:
            if (gLWOData->pNULLList->GetNumElements() == 0)
            {                                               //if there are no NULL objects
                printf("No animations for %s.\n", globalData->inputFileName);
                return(0);
            }
            fileHandle = fopen(globalData->outputFileName, "wb");

            if(!fileHandle)
            {
                // File is probably read only.
                printf("ERROR %s cannot be opened for writing (not checked out?).\n", globalData->outputFileName);
                return(FAILURE);
            }
    		printf("LWExport: Exporting %s\n", globalData->outputFileName);
    		retVal = MADParse_WriteFile(fileHandle);
            break;
	}

	fclose(fileHandle);

	return(retVal);
}

int main(int argc, char *argv[])
{	
	signed char retVal;
	
	Initialize();

	retVal = ParseCommandLine(argc, argv);

	if(retVal == SUCCESS)
	{
		retVal = ChooseFileTypesByExtension();
	}
	
	if(retVal == SUCCESS)
	{
		retVal = FindExplicitFile(globalData->inputFileName);
	}
	
	if(retVal == SUCCESS)
	{
		// Initialize the export component module
		switch(globalData->outputFileType)
		{
    		case EF_GEO:
    			InitializeGEOParse();
    			InitializeMEXParse();
    			break;

    		case EF_NIS:
    			InitializeNISParse();
    			break;

    		case EF_HSF:
    			InitializeHSFParse();
    			break;

            case EF_MAD:
                InitializeMADParse();
                break;
		}

		retVal = Job();
	}

	DeInitialize();

	return(retVal);
}