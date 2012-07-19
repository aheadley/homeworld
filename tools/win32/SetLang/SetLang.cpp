// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "setlang.h"

// Language codes for CFG file
unsigned long nLangCode[NUM_LANG]=
{
	0,
	1,
	2,
	3,
	4
};

// Language names for registry
char *szLangName[]=
{
	"English",
	"French",
	"German",
	"Spanish",
	"Italian"
};

int main(int argc,char **argv)
{
	long nArg;
	
	unsigned long i,nFlag,nNum;

	char *pRoot,*pLine;
	char szCfg[256],szBak[256],szRead[1024],szWrite[1024];
	
	DWORD nDisp;

	FILE *pRead,*pWrite;

    HKEY hKey;

	// Show copyright
	printf("SetLang\n");
	printf("Copyright (c) 1999 Relic Entertainment Inc.\n\n");

	// Check args
	if(argc < 3)
	{
		printf("Usage: setlang -e|<work_path> <lang_num>\n");
		printf("       -e:               Use environment variable for the work path\n");
		printf("       <work_path>:      Work path for config file\n");
		printf("       <lang_num>:       Language number\n");

		// Enumerate languages
		for(i=0;i<NUM_LANG;i++)
		{
			printf("                         - %ld: %s\n",i,szLangName[i]);
		}

		return(-1);
	}

	// Check first argument
	nArg=1;
	if((stricmp(argv[nArg],"-e") == 0) || (stricmp(argv[nArg],"/e") == 0))
	{
		// Get root name
		pRoot=getenv(ENV_ROOT);
		if(pRoot == NULL)
		{
			printf("Error: Environment variable %s not defined!\n",ENV_ROOT);
			return(-1);
		}

		// Get CFG name
		strcpy(szCfg,pRoot);
		if(szCfg[strlen(szCfg)-1] != '\\') strcat(szCfg,"\\");
		strcat(szCfg,DEF_PATH);
		strcat(szCfg,CFG_NAME);

		// Get backup name
		strcpy(szBak,pRoot);
		if(szBak[strlen(szBak)-1] != '\\') strcat(szBak,"\\");
		strcat(szBak,DEF_PATH);
		strcat(szBak,BAK_PFIX);
		strcat(szBak,CFG_NAME);
	}
	else
	{
		// Get CFG name
		strcpy(szCfg,argv[nArg]);
		if(szCfg[strlen(szCfg)-1] != '\\') strcat(szCfg,"\\");
		strcat(szCfg,CFG_NAME);

		// Get backup name
		strcpy(szBak,argv[nArg]);
		if(szBak[strlen(szBak)-1] != '\\') strcat(szBak,"\\");
		strcat(szBak,BAK_PFIX);
		strcat(szBak,CFG_NAME);
	}

	// Get second argument
	nArg=2;
	nNum=atol(argv[nArg]);

	// Check number
	if(nNum >= NUM_LANG)
	{
		printf("Error: Invalid language number %ld!\n",nNum);
		return(-1);
	}
	
	printf("Updating config file: %s\n",szCfg);

	// Copy file
	CopyFile(szCfg,szBak,FALSE);

	// Try to open backup file
	pRead=fopen(szBak,"rt");
	if(pRead != NULL)
	{
		// Create CFG file
		pWrite=fopen(szCfg,"wt");
		if(pWrite == NULL)
		{
			fclose(pRead);
			CopyFile(szBak,szCfg,FALSE);
			printf("Error: Unable to open config file %s!\n",szCfg);
			return(-1);
		}
		
		// Row loop
		while(1)
		{
			// Set flag
			nFlag=1;

			// Read line
			fgets(szRead,1024-1,pRead);

			// Parse line
			while((szRead[strlen(szRead)-1] == '\n') || (szRead[strlen(szRead)-1] == '\r')) szRead[strlen(szRead)-1]=0;

			// Copy line
			strcpy(szWrite,szRead);

			// Replace tabs with spaces
			for(i=0;i<strlen(szRead);i++)
			{
				if(szRead[i] == '\t') szRead[i]=' ';
			}

			// Check for end-of-file
			if(feof(pRead)) break;

			// Check line content
			if(strlen(szRead) != 0)
			{
				// Find data
				pLine=strtok(szRead," ");
				if(pLine != NULL)
				{
					// Compare data
					if(stricmp(pLine,CFG_TOK) == 0)
					{
						// Clear flag
						nFlag=0;
					}
				}
			}

			// Write line
			if(nFlag == 1) fprintf(pWrite,"%s\n",szWrite);
		}

		// Close backup file
		fclose(pWrite);

		// Close CFG file
		fclose(pRead);
	}

	// Create or append CFG file
	pWrite=fopen(szCfg,"a+t");
	if(pWrite == NULL)
	{
		CopyFile(szBak,szCfg,FALSE);
		printf("Error: Unable to open config file %s!\n",szCfg);
		return(-1);
	}

	// Write language to CFG file
	fprintf(pWrite,"%s %ld\n",CFG_TOK,nLangCode[nNum]);

	// Close CFG file
	fclose(pWrite);

	printf("Updating registry...\n");

	// Create or open registry key
    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,REG_KEY,0,"",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&nDisp) != ERROR_SUCCESS)
    {
		printf("Error: Unable to open registry key %s!\n",REG_KEY);
		return(-1);
    }

	// Write registry value
    if(RegSetValueEx(hKey,REG_VAL,0,REG_SZ,(unsigned char *)szLangName[nNum],strlen(szLangName[nNum])+1) != ERROR_SUCCESS)
    {
		printf("Error: Unable to write registry value %s!\n",REG_VAL);
		return(-1);
    }

	// Close registry key
    RegCloseKey(hKey);

	printf("Done!\n");

	return(0);
}
