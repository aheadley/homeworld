// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "vol.h"

// Global variables
FILE			*pHeadFile;		// Handle for header file
FILE			*pLookFile;		// Handle for lookup file
FILE			*pFreqFile;		// Handle for frequency file

unsigned short  nNumVolRs;		// Number of volumes/ranges
unsigned short	nNumHeaders;	// Number of headers
unsigned short  nVolRCount;		// Volumes/ranges count
unsigned short	nHeaderCount;	// Headers count

long	*aLookTab;	// Lookup table
float	*aFreqTab;	// Frequency table


// Export functions

// This function opens a lookup table file
__declspec(dllexport) int __stdcall volOpenWLookup(char *szFileName,char *szID,
												  unsigned short nNumVRs,
												  unsigned short nNumHeads)
{
	char *pID;
	
	// Open lookup file
	pLookFile=fopen(szFileName,"wb");
	if(pLookFile == NULL) return(VOL_ERR_LUFILE);

	// Set number of volume/ranges and headers
	nNumVolRs=nNumVRs;
	nNumHeaders=nNumHeads;
	nHeaderCount=0;

	// Pad ID
	while(strlen(szID) < VOL_ID) strcat(szID," ");
	pID=szID;
	
	// Write constants
	fwrite(pID,sizeof(char),VOL_ID,pLookFile);
	fwrite(&nNumVolRs,sizeof(unsigned short),1,pLookFile);
	fwrite(&nNumHeaders,sizeof(unsigned short),1,pLookFile);

	// Allocate memory
	aLookTab=(long *)calloc(nNumVolRs*nNumHeaders,sizeof(long));
	if(aLookTab == NULL) return(VOL_ERR_LUALLOC);
	
	return(OK);
}

// This function writes a patch to the lookup table file
__declspec(dllexport) int __stdcall volWriteLookup(long *aLookup)
{
	unsigned short n;

	// Check header count
	if(nHeaderCount > nNumHeaders) return(VOL_ERR_LUWRITE);

	// Populate lookup table
	for(n=0;n<nNumVolRs;n++) aLookTab[nHeaderCount*nNumVolRs+n]=aLookup[n];

	// Increment header count
	nHeaderCount++;
	
	return(OK);
}

// This function closes a lookup table file
__declspec(dllexport) int __stdcall volCloseWLookup(void)
{
	// Write data
	fwrite(aLookTab,sizeof(long),nNumVolRs*nNumHeaders,pLookFile);

	// Free memory
	free(aLookTab);

	// Close lookup file
	fclose(pLookFile);

	return(OK);
}

// This function opens a lookup table file
__declspec(dllexport) int __stdcall volOpenRLookup(char *szFileName,
												  unsigned short *nNumVRs,
												  unsigned short *nNumHeads)
{
	char szID[VOL_ID];
	
	// Open lookup file
	pLookFile=fopen(szFileName,"rb");
	if(pLookFile == NULL) return(VOL_ERR_LUFILE);

	// Write constants
	fread(szID,sizeof(char),VOL_ID,pLookFile);
	fread(&nNumVolRs,sizeof(unsigned short),1,pLookFile);
	fread(&nNumHeaders,sizeof(unsigned short),1,pLookFile);

	// Set number of volume/ranges and headers
	*nNumVRs=nNumVolRs;
	*nNumHeads=nNumHeaders;
	nHeaderCount=0;

	// Allocate memory
	aLookTab=(long *)calloc(nNumVolRs*nNumHeaders,sizeof(long));
	if(aLookTab == NULL) return(VOL_ERR_LUALLOC);
	
	// Read data
	fread(aLookTab,sizeof(long),nNumVolRs*nNumHeaders,pLookFile);

	return(OK);
}

// This function writes a patch to the lookup table file
__declspec(dllexport) int __stdcall volReadLookup(long *aLookup)
{
	unsigned short n;

	// Check header count
	if(nHeaderCount > nNumHeaders) return(VOL_ERR_LUREAD);

	// Populate lookup table
	for(n=0;n<nNumVolRs;n++) aLookup[n]=aLookTab[nHeaderCount*nNumVolRs+n];

	// Increment header count
	nHeaderCount++;
	
	return(OK);
}

// This function closes a lookup table file
__declspec(dllexport) int __stdcall volCloseRLookup(void)
{
	// Free memory
	free(aLookTab);

	// Close lookup file
	fclose(pLookFile);

	return(OK);
}

// This function opens a frequency file
__declspec(dllexport) int __stdcall volOpenWFreq(char *szFileName,char *szID,
												  unsigned short nNumVRs)
{
	short nNum;

	char *pID;
	
	// Open frequency file
	pFreqFile=fopen(szFileName,"wb");
	if(pFreqFile == NULL) return(VOL_ERR_LUFILE);

	// Set number of volume/ranges
	nNumVolRs=nNumVRs;
	nNum=VOL_FREQ;
	nVolRCount=0;

	// Pad ID
	while(strlen(szID) < VOL_ID) strcat(szID," ");
	pID=szID;
	
	// Write constants
	fwrite(pID,sizeof(char),VOL_ID,pFreqFile);
	fwrite(&nNum,sizeof(unsigned short),1,pFreqFile);
	fwrite(&nNumVolRs,sizeof(unsigned short),1,pFreqFile);

	// Allocate memory
	aFreqTab=(float *)calloc(VOL_FREQ*nNumVolRs,sizeof(float));
	if(aFreqTab == NULL) return(VOL_ERR_LUALLOC);
	
	return(OK);
}

// This function writes a patch to the frequency file
__declspec(dllexport) int __stdcall volWriteFreq(float *aFreq)
{
	unsigned short n;

	// Check header count
	if(nVolRCount > nNumVolRs) return(VOL_ERR_LUWRITE);

	// Populate frequency data
	for(n=0;n<VOL_FREQ;n++) aFreqTab[nVolRCount*VOL_FREQ+n]=aFreq[n];

	// Increment header count
	nVolRCount++;
	
	return(OK);
}

// This function closes a frequency file
__declspec(dllexport) int __stdcall volCloseWFreq(void)
{
	// Write data
	fwrite(aFreqTab,sizeof(float),VOL_FREQ*nNumVolRs,pFreqFile);

	// Free memory
	free(aFreqTab);

	// Close frequency file
	fclose(pFreqFile);

	return(OK);
}

// This function opens a frequency file
__declspec(dllexport) int __stdcall volOpenRFreq(char *szFileName,
												  unsigned short *nNumVRs)
{
	short nNum;

	char szID[VOL_ID];
	
	// Open frequency file
	pFreqFile=fopen(szFileName,"rb");
	if(pFreqFile == NULL) return(VOL_ERR_LUFILE);

	// Write constants
	fread(szID,sizeof(char),VOL_ID,pFreqFile);
	fread(&nNum,sizeof(unsigned short),1,pFreqFile);
	fread(&nNumVolRs,sizeof(unsigned short),1,pFreqFile);

	// Set number of volume/ranges
	*nNumVRs=nNumVolRs;
	nVolRCount=0;

	// Allocate memory
	aFreqTab=(float *)calloc(VOL_FREQ*nNumVolRs,sizeof(float));
	if(aFreqTab == NULL) return(VOL_ERR_LUALLOC);
	
	// Read data
	fread(aFreqTab,sizeof(float),VOL_FREQ*nNumVolRs,pFreqFile);

	return(OK);
}

// This function writes a patch to the frequency file
__declspec(dllexport) int __stdcall volReadFreq(float *aFreq)
{
	unsigned short n;

	// Check header count
	if(nVolRCount > nNumVolRs) return(VOL_ERR_LUREAD);

	// Populate frequency table
	for(n=0;n<VOL_FREQ;n++) aFreq[n]=aFreqTab[nVolRCount*VOL_FREQ+n];

	// Increment header count
	nVolRCount++;
	
	return(OK);
}

// This function closes a frequency file
__declspec(dllexport) int __stdcall volCloseRFreq(void)
{
	// Free memory
	free(aFreqTab);

	// Close frequency file
	fclose(pFreqFile);

	return(OK);
}

// This function gets the labels from a header file
__declspec(dllexport) int __stdcall volGetLabels(char *szFileName,char **szLabels,unsigned short *nCount)
{
	FILE	*hHeadFile;	

	char	*pLabel;
	char	*pLine,szLine[VOL_LEN];

	unsigned short	nSize;

	// Open header file
	hHeadFile=fopen(szFileName,"rt");
	if(hHeadFile == NULL) return(VOL_ERR_LBLFILE);

	// Reset size and count
	nSize=0;
	*nCount=0;
	
	while(1)
	{
		// Read line
		fgets(szLine,VOL_LEN-1,hHeadFile);

		// Check end-of-file and break
		if(feof(hHeadFile) != 0) break;

		// Replace tabs with spaces
		pLine=szLine;
		while(1)
		{
			// Find tabs and break
			pLine=strchr(pLine,9);
			if(pLine == NULL) break;

			// Replace tabs
			pLine[0]=32;
		}

		// Remove trailing line feeds
		while(szLine[strlen(szLine)-1] == 10) szLine[strlen(szLine)-1]=0;

		// Find first token
		pLine=strtok(szLine," ");
		if(pLine != NULL)
		{
			// Check for #define
			if(strcmp(pLine,"#define") == 0)
			{
				// Find label
				pLabel=strtok(NULL," ");
				if(pLabel != NULL)
				{
					// Find index
					pLine=strtok(NULL," ");
					if(pLine != NULL)
					{
						// Check index
						if(atoi(pLine) == *nCount)
						{
							// Adjust size
							nSize=nSize+strlen(pLabel)+1;

							// Increment count
							(*nCount)++;
						}
					}
				}
			}
		}
	}

	// Allocate memory for array of labels
	*szLabels=(char *)calloc(nSize,sizeof(char));
	if(*szLabels == NULL)
	{	
		// Close header file and return
		fclose(hHeadFile);
		return(VOL_ERR_LBLALLOC);
	}
	
	// Rewind header file
	rewind(hHeadFile);

	// Initialize array of labels
	strcpy(*szLabels,"");

	// Reset count
	*nCount=0;
	
	while(1)
	{
		// Read line
		fgets(szLine,VOL_LEN-1,hHeadFile);

		// Check end-of-file and break
		if(feof(hHeadFile) != 0) break;

		// Replace tabs with spaces
		pLine=szLine;
		while(1)
		{
			// Find tabs and break
			pLine=strchr(pLine,9);
			if(pLine == NULL) break;

			// Replace tabs
			pLine[0]=32;
		}

		// Remove trailing line feeds
		while(szLine[strlen(szLine)-1] == 10) szLine[strlen(szLine)-1]=0;

		// Find #define
		pLine=strtok(szLine," ");
		if(pLine != NULL)
		{
			if(strcmp(pLine,"#define") == 0)
			{	
				// Find label
				pLabel=strtok(NULL," ");
				if(pLabel != NULL)
				{
					// Find index
					pLine=strtok(NULL," ");
					if(pLine != NULL)
					{
						// Check index
						if(atoi(pLine) == *nCount)
						{
							if(*nCount > 0) strcat(*szLabels," ");
							strcat(*szLabels,pLabel);

							// Increment count
							(*nCount)++;
						}
					}
				}
			}
		}
	}
	
	// Close header file and return
	fclose(hHeadFile);

	return(OK);
}

// This function returns an error message for a matching VOL error code
__declspec(dllexport) int __stdcall volGetErr(int nErr,char **szText)
{
	char *pText;

	switch(nErr)
	{
		// VOL error codes
		case VOL_ERR_LUFILE:	pText="Unable to open lookup file";
								break;
		case VOL_ERR_LUWRITE:	pText="Unable to write data to lookup table";
								break;
		case VOL_ERR_LUREAD:	pText="Unable to read data from lookup table";
								break;
		case VOL_ERR_LUALLOC:	pText="Unable to allocate memory for lookup table";
								break;
		case VOL_ERR_LBLFILE:	pText="Unable to open labels file";
								break;
		case VOL_ERR_LBLALLOC:	pText="Unable to allocate memory for labels";
								break;
		
		// Default error codes
		case 0:		pText="No error";
					break;

		default:	pText="Undefined error";
	}

	// Allocate memory for text
	*szText=(char *)calloc(strlen(pText)+1,sizeof(char));
	if(*szText == NULL) return(ERR);

	// Copy text
	strcpy(*szText,pText);

	return(OK);
}
