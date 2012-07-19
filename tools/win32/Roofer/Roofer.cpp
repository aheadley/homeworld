// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc,char **argv)
{
	unsigned long n,nSize;

	char szPad[256],szSize[256],*pSize;
		
	FILE *pPad;

	// Show copyright
	printf("Roofer\n");
	printf("Copyright (c) 1999 Relic Entertainment Inc.\n");

	// Check args
	if(argc < 3) 
	{
		printf("Usage: roofer <pad_file> <size>\n");
		printf("       <pad_file>: Pad file, containing random characters\n");
		printf("       <size>:     Size of the pad file (1 = 1 byte, 1K = 1 kilo, 1M = 1 meg)\n");
		return(-1);
	}

	// Get pad file name
	strcpy(szPad,argv[1]);
	
	// Get size
	strcpy(szSize,argv[2]);
	strupr(szSize);

	// Process bytes value
	nSize=atol(szSize);

	// Process KB value
	pSize=strchr(szSize,'K');
	if(pSize != NULL)
	{
		pSize[0]=0;
		nSize=atol(szSize)*1024;
	}

	// Process MB value
	pSize=strchr(szSize,'M');
	if(pSize != NULL)
	{
		pSize[0]=0;
		nSize=atol(szSize)*1024*1024;
	}
	
	// Check size
	if(nSize < sizeof(unsigned long)) nSize=sizeof(unsigned long);
	
	// Open pad file
	pPad=fopen(szPad,"wb");
	if(pPad == NULL)
	{
		printf("Error: Unable to create pad file %s\n",szPad);
		return(-1);
	}

	printf("Creating pad file, %ld bytes...\n",nSize);

	// Write random characters
	srand((unsigned int)time(NULL));
	for(n=0;n<nSize-sizeof(unsigned long);n++) putc(rand()%256,pPad);

	// Write size
	fwrite(&nSize,1,sizeof(unsigned long),pPad);

	// Close Pad file
	fclose(pPad);

	printf("Done!\n");

	return(0);
}
