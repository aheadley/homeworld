// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "plumber.h"
#include "leakyfaucet.h"

int main(int argc,char **argv)
{
	long n,nSize,nPos,nLen,nArg;
	
	unsigned long nCount,nFlag;

	char *pRoot,*pBuffer,*pLine,szWork[4],szLine[1024];
	char szExe[256],szLog[256],szComment[256],szData[512];
		
	unsigned char aData[MAX_LEAK_STRING_LENGTH];
	
	FILE *pExe,*pLog;

	// Show copyright
	printf("Plumber\n");
	printf("Copyright (c) 1999 Relic Entertainment Inc.\n");

	// Check args
	if(argc < 3) 
	{
		printf("Usage: plumber [-c] -e|<work_path> <comment_string>\n");
		printf("       -c:               Find comment string in log file\n");
		printf("       -e:               Use environment variable for the work path\n");
		printf("       <work_path>:      Work path for executable and log file\n");
		printf("       <comment_string>: Comment string for log file\n");
		return(-1);
	}

	// Set length
	nLen=strlen(leakString);

	// Clear flag
	nFlag=0;
	
	// Check first argument
	nArg=1;
	if((stricmp(argv[nArg],"-c") == 0) || (stricmp(argv[nArg],"/c") == 0))
	{
		// Set flag
		nFlag=1;

		// Increment argument
		nArg++;
	}

	if((stricmp(argv[nArg],"-e") == 0) || (stricmp(argv[nArg],"/e") == 0))
	{
		// Get root name
		pRoot=getenv(ENV_ROOT);
		if(pRoot == NULL)
		{
			printf("Error: Environment variable %s not defined!\n",ENV_ROOT);
			return(-1);
		}

		// Get EXE name
		strcpy(szExe,pRoot);
		if(szExe[strlen(szExe)-1] != '\\') strcat(szExe,"\\");
		strcat(szExe,DEF_PATH);
		strcat(szExe,EXE_NAME);

		// Get log name
		strcpy(szLog,pRoot);
		if(szLog[strlen(szLog)-1] != '\\') strcat(szLog,"\\");
		strcat(szLog,DEF_PATH);
		strcat(szLog,LOG_NAME);
	}
	else
	{
		// Get EXE name
		strcpy(szExe,argv[nArg]);
		if(szExe[strlen(szExe)-1] != '\\') strcat(szExe,"\\");
		strcat(szExe,EXE_NAME);

		// Get log name
		strcpy(szLog,argv[nArg]);
		if(szLog[strlen(szLog)-1] != '\\') strcat(szLog,"\\");
		strcat(szLog,LOG_NAME);
	}

	// Check flag
	if(nFlag == 0)
	{
		// Get comment string
		nArg++;
		strcpy(szComment,"");
		for(n=nArg;n<argc;n++)
		{
			if(n > nArg) strcat(szComment," ");
			strcat(szComment,argv[n]);
		}
	}

	// Open log file to read only
	pLog=fopen(szLog,"r+t");
	if(pLog == NULL)
	{
		printf("Error: Unable to open log file (check attributes) %s\n",szLog);
		return(-1);
	}

	// Open EXE file to read and write (file must exist)
	pExe=fopen(szExe,"r+b");
	if(pExe == NULL)
	{
		fclose(pLog);
		printf("Error: Unable to open executable %s\n",szExe);
		return(-1);
	}

	printf("Checking for leaky faucets in executable: %s\n",szExe);
	printf("Please wait...\n");

	// Get executable size
	fseek(pExe,0,SEEK_SET);
	nSize=ftell(pExe);
	fseek(pExe,0,SEEK_END);
	nSize=ftell(pExe)-nSize;

	// Allocate buffer for executable
	pBuffer=(char *)malloc(nSize*sizeof(char));
	if(pBuffer == NULL)
	{
		fclose(pExe);
		fclose(pLog);
		printf("Error: Unable to allocate buffer for executable!\n");
		return(-1);
	}

	// Read executable into buffer
	fseek(pExe,0,SEEK_SET);
	if(fread(pBuffer,1,nSize,pExe) < (size_t)nSize)
	if(pBuffer == NULL)
	{
		fclose(pExe);
		fclose(pLog);
		printf("Error: Unable to read executable into buffer!\n");
		return(-1);
	}

	// Reset count
	nCount=0;

	// Look for leaky faucet(s)
	for(n=0;n<nSize-MAX_LEAK_STRING_LENGTH;n++)
	{
		if(memcmp(&pBuffer[n],leakString,nLen) == 0)
		{
			// Set position
			if(nCount == 0) nPos=n+nLen;

			// Increment count
			nCount++;
		}
	}

	// Get existing data
	memcpy(aData,&pBuffer[nPos],MAX_LEAK_STRING_LENGTH-nLen);
	
	// Free buffer
	free(pBuffer);

	// Check count
	if(nCount != 1)
	{
		fclose(pExe);
		fclose(pLog);
		printf("Error: %ld leaky faucets detected!\n",nCount);
		return(-1);
	}

	// Check flag
	if(nFlag == 1)
	{
		printf("Checking for comment string in log file: %s\n",szLog);

		// Parse data into HEX format
		strcpy(szData,"");
		for(n=0;n<MAX_LEAK_STRING_LENGTH-nLen;n++)
		{
			sprintf(szWork,"%02X",aData[n]);
			strcat(szData,szWork);
		}
		
		// Reset count
		nCount=0;

		// Row loop
		while(1)
		{
			// Get line
			fgets(szLine,1024-1,pLog);

			// Parse line
			while((szLine[strlen(szLine)-1] == '\n') || (szLine[strlen(szLine)-1] == '\r')) szLine[strlen(szLine)-1]=0;

			// Check end-of-file
			if(feof(pLog)) break;

			// Check line content
			if(strlen(szLine) == 0) continue;

			// Ignore comments
			pLine=strstr(szLine,";");
			if(pLine != NULL) pLine[0]=0;

			// Check line content
			if(strlen(szLine) == 0) continue;

			// Find data
			pLine=strtok(szLine,"\t");
			if(pLine == NULL) continue;

			// Compare data
			if(stricmp(szData,pLine) == 0)
			{
				// Find length
				pLine=strtok(NULL,"\t" );
				if(pLine == NULL) continue;

				// Find comment
				pLine=strtok(NULL,"\t" );
				if(pLine == NULL) continue;
				strcpy(szComment,pLine);

				// Increment count
				nCount++;
				break;
			}
		}

		// Check count
		if(nCount == 0)
		{
			fclose(pExe);
			fclose(pLog);
			printf("Error: No comment strings detected!\n");
			return(-1);
		}

		printf("Comment string detected: %s\n",szComment);
	}
	else
	{
		// Generate random data
		srand((unsigned int)time(NULL));
		for(n=0;n<MAX_LEAK_STRING_LENGTH-nLen;n++) aData[n]=rand()%256;

		printf("Updating log file: %s\n",szLog);

		// Write data and comment into log file
		fseek(pLog,0,SEEK_END);
		for(n=0;n<MAX_LEAK_STRING_LENGTH-nLen;n++) fprintf(pLog,"%02X",aData[n]);
		fprintf(pLog,"\t%ld\t%s\n",MAX_LEAK_STRING_LENGTH-nLen,szComment);
			
		printf("Fixing leaky faucets in executable: %s\n",szExe);

		// Write data into executable
		fseek(pExe,nPos,SEEK_SET);
		fwrite(aData,1,MAX_LEAK_STRING_LENGTH-nLen,pExe);
	}

	// Close EXE file
	fclose(pExe);

	// Close log file
	fclose(pLog);

	printf("Done!\n");

	return(0);
}
