// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
// Original code for strhash() by Jason Dorie
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MMHash.h"

int main(int argc,char **argv)
{
	unsigned long nObj,nFile,nZero;

	char szConfig[256],szDir[256],szSection[256],szKey[256],szType[256],szFile[256],szSource[256],szDest[256];

	FILE *pStream;

	// Show copyright
	printf("MissionMan hash tool\n");
	printf("Copyright (c) 1999 Relic Entertainment Inc.\n");

	// Check arguments
	if(argc < 3) 
	{
		// Show message
		printf("\nUsage: MMHash <cfg_file> <par_dir>\n");
		return(-1);
	}

	// Get arguments
	if(strchr(argv[1],':') == NULL) sprintf(szConfig,".\\%s",argv[1]);
	else strncpy(szConfig,argv[1],255);
	if(argv[2][strlen(argv[2]-1)] != '\\') sprintf(szDir,"%s\\",argv[2]);
	else strncpy(szDir,argv[2],255);

	// Open config file
	pStream=fopen(szConfig,"rt");
	if(pStream == NULL)
	{
		// Inform user
		printf("\nError: Unable to open config file %s!\n",szConfig);
		return(-1);
	}

	// Close config file
	fclose(pStream);

	// Set zero;
	nZero=0;

	// Initialize index
	nObj=0;

	// Loop thru objects
	while(1)
	{
		// Get type
		sprintf(szSection,"%s%s%ld",MMH_SEC_MULTI,MMH_SEC_OBJ,nObj);
		strcpy(szKey,MMH_KEY_TYPE);
		GetPrivateProfileString(szSection,szKey,"",szType,sizeof(szType)-1,szConfig);

		// Check type
		if(strlen(szType) == 0)
		{
			// Check index
			if(nObj == 0)
			{
				// Inform user
				printf("\nError: Unable to find object %s!\n",szSection);
				return(-1);
			}
			break;
		}

		// Inform user
		printf("\nProcessing object %s (%s)...\n",szSection,szType);
		
		// Initialize index
		nFile=0;

		// Loop thru files
		while(1)
		{
			// Get file
			sprintf(szKey,"%s%ld",MMH_KEY_FILE,nFile);
			GetPrivateProfileString(szSection,szKey,"",szFile,sizeof(szFile)-1,szConfig);

			// Check file
			if(strlen(szFile) == 0) break;
			if(strchr(szFile,':') == NULL)
			{
				// Set source and destination
				sprintf(szSource,"%s%s",szDir,szFile);
				sprintf(szDest,"%s%ld%s",szDir,strhash(szFile),MMH_EXT_OBJ);

				// Inform user
				printf("Processing file %s...\n",szSource);
				printf("  Creating file %s...\n",szDest);

				// Copy file
				CopyFile(szSource,szDest,FALSE);
				SetFileAttributes(szDest,FILE_ATTRIBUTE_NORMAL);

				// Open file
				pStream=fopen(szDest,"r+b");
				if(pStream == NULL) printf("Warning: Unable to open file %s!\n",szDest);
				else
				{
					// Mangle header
					rewind(pStream);
					fwrite("FORM",sizeof(char),4,pStream);
					fwrite(&nZero,sizeof(unsigned long),1,pStream);
					fwrite("MMAN",sizeof(char),4,pStream);
					fclose(pStream);
				}
			}

			// Increment index
			nFile++;
		}

		// Increment index
		nObj++;
	}

	// Initialize index
	nObj=0;

	// Loop thru objects
	while(1)
	{
		// Get type
		sprintf(szSection,"%s%s%ld",MMH_SEC_SING,MMH_SEC_OBJ,nObj);
		GetPrivateProfileString(szSection,MMH_KEY_TYPE,"",szType,sizeof(szType)-1,szConfig);

		// Check type
		if(strlen(szType) == 0)
		{
			// Check index
			if(nObj == 0)
			{
				// Inform user
				printf("\nError: Unable to find object %s!\n",szSection);
				return(-1);
			}
			break;
		}

		// Inform user
		printf("\nProcessing object %s (%s)...\n",szSection,szType);
		
		// Initialize index
		nFile=0;

		// Loop thru files
		while(1)
		{
			// Get file
			sprintf(szKey,"%s%ld",MMH_KEY_FILE,nFile);
			GetPrivateProfileString(szSection,szKey,"",szFile,sizeof(szFile)-1,szConfig);

			// Check file
			if(strlen(szFile) == 0) break;
			if(strchr(szFile,':') == NULL)
			{
				// Set source and destination
				sprintf(szSource,"%s%s",szDir,szFile);
				sprintf(szDest,"%s%ld%s",szDir,strhash(szFile),MMH_EXT_OBJ);

				// Inform user
				printf("Processing file %s...\n",szSource);
				printf("  Creating file %s...\n",szDest);

				// Copy file
				CopyFile(szSource,szDest,FALSE);
				SetFileAttributes(szDest,FILE_ATTRIBUTE_NORMAL);

				// Open file
				pStream=fopen(szDest,"r+b");
				if(pStream == NULL) printf("Warning: Unable to open file %s!\n",szDest);
				else
				{
					// Mangle header
					rewind(pStream);
					fwrite("FORM",sizeof(char),4,pStream);
					fwrite(&nZero,sizeof(unsigned long),1,pStream);
					fwrite("MMAN",sizeof(char),4,pStream);
					fclose(pStream);
				}
			}

			// Increment index
			nFile++;
		}

		// Increment index
		nObj++;
	}

	// Inform user
	printf("\nDone.\n",szConfig);

	return(0);
}

unsigned long strhash(char *str)
{
	unsigned long i,hash;

	hash=0;

	while(*str != 0)
	{
		hash=(hash<<4)+*str++;
		if((i=hash&0xF0000000) != 0) hash=(hash^(i>>24))^i;
	}

	return(hash);
}

