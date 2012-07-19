// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fxtract.h"

int main(int argc,char **argv)
{
	int nTokStat,nFileFlag,nScrFlag,nSepFlag,nDupFlag;
	
	unsigned long n,nIndex;

	char szName[256],szOut[256],*pLine,szLine[FX_MARGIN],szTmp[FX_MARGIN];
	char szFile[256],szScreen[256],szText[FX_COLUMN-2][256],szToken[255];

	FILE *pIStream,*pTStream,*pOStream;

	// Show copyright
	printf("Front-end extraction tool\n");
	printf("Copyright (c) 1999 Relic Entertainment Inc.\n");

	// Check arguments
	if(argc < 3) 
	{
		// Show message
		printf("\nUsage: FXTract <input_file> <output_file>\n");
		return(-1);
	}

	// Get argument
	strncpy(szName,argv[1],255);
	strncpy(szOut,argv[2],255);

	// Show message
	printf("\nProcessing input file %s...\n",szName);

	// Open input file
	pIStream=fopen(szName,"rt");
	if(pIStream == NULL)
	{
		// Show message
		printf("Error: unable to open input file [%s]!\n",szName);
		return(-1);
	}

	// Show message
	printf("\nCreating ouput file %s...\n",szOut);

	// Open output file
	pOStream=fopen(szOut,"wt");
	if(pOStream == NULL)
	{
		// Close input file
		fclose(pIStream);

		// Show message
		printf("Error: unable to open output file [%s]!\n",szOut);
		return(-1);
	}

	// Initialize strings
	strcpy(szFile,"");
	strcpy(szName,"");
	strcpy(szScreen,"");

	// Reset separator flag
	nSepFlag=0;
	
	// Row loop
	while(1)
	{
		// Get line
		fgets(szLine,FX_MARGIN-1,pIStream);

		// Check end-of-file
		if(feof(pIStream) != 0) break;

		// Parse line
		while((szLine[strlen(szLine)-1] == '\n') || (szLine[strlen(szLine)-1] == '\r')) szLine[strlen(szLine)-1]=0;

		// Ignore comments
		pLine=strstr(szLine,FX_COMMENT);
		if(pLine != NULL) pLine[0]=0;

		// Check line content
		if(strlen(szLine) == 0) continue;

		// Initialize strings
		strcpy(szText[0],"");
		for(n=1;n<FX_COLUMN-2;n++) strcpy(szText[n],"");

		// Set file
		strncpy(szFile,szLine,255);

		// Show message
		printf("\nProcessing front-end file [%s]...\n",szFile);

		// Open temporary file
		pTStream=fopen(szFile,"rb");
		if(pTStream == NULL) printf("Warning: unable to open front-end file %s!\n",szFile);
		else
		{
			// Reset file flag
			nFileFlag=0;
		
			// Reset status
			nTokStat=0;

			// Row loop
			while(1)
			{
				// Get line
				fgets(szLine,FX_MARGIN-1,pTStream);

				// Parse line
				while(szLine[strlen(szLine)-1] == '\n') szLine[strlen(szLine)-1]=0;
				if(szLine[0] != ' ')
				{
					while(szLine[strlen(szLine)-1] == ' ') szLine[strlen(szLine)-1]=0;
				}

				// Check line
				if(strlen(szLine) > 0)
				{
					// Check status
					switch(nTokStat)
					{
						case 0:
							// Check for file tag
							if(strstr(szLine,FX_FILETAG) != NULL) nTokStat=1;
							break;

						case 1:
							// Check for screen tag
							if(strstr(szLine,FX_SCREENTAG) != NULL) nTokStat=2;

							// Check for file end
							if(strstr(szLine,FX_FILEEND) != NULL) nTokStat=0;
							break;

						case 2:
							// Check for screen label
							if(strstr(szLine,FX_SCREENLABEL) != NULL)
							{
								// Set status
								nTokStat=3;

								// Parse line
								strcpy(szTmp,szLine);
								strtok(szTmp,"\t");
								pLine=strtok(NULL,"\t");

								// Check line
								if(pLine != NULL)
								{
									// Reset screen flag
									nScrFlag=0;
		
									// Set file
									strncpy(szScreen,pLine,255);

									// Show message
									printf("Processing screen [%s]...\n",szScreen);
								}
							}

							// Check for screen end
							if(strstr(szLine,FX_SCREENEND) != NULL) nTokStat=1;
							break;

						case 3:
							// Check for text tag
							if(strstr(szLine,FX_TEXTTAG) != NULL) nTokStat=4;

							// Check for screen end
							if(strstr(szLine,FX_SCREENEND) != NULL) nTokStat=1;
							break;

						case 4:									
							// Check for text label
							if(strstr(szLine,FX_TEXTLABEL) != NULL)
							{
								// Set index amd status
								nIndex=0;
								nTokStat=5;
							}

							// Check for text end
							if(strstr(szLine,FX_TEXTEND) != NULL) nTokStat=3;
							break;

						case 5:
							// Check for text end
							if(strstr(szLine,FX_TEXTEND) != NULL) nTokStat=3;
							else
							{
								// Set status
								nTokStat=6;

								// Set file
								strncpy(szText[nIndex],szLine,255);

								// Show message
								printf("Processing native text [%s]...\n",szText[nIndex]);

								// Write line feed
								if(nSepFlag == 1) fprintf(pOStream,"\n");
								if((nSepFlag == 1) && (nScrFlag == 0)) fprintf(pOStream,"\n");

								// Write file
								if(nFileFlag == 0) fprintf(pOStream,"%s",szFile);
								fprintf(pOStream,"%s",FX_SEPAR);

								// Write screen
								if(nScrFlag == 0) fprintf(pOStream,"%s",szScreen);
								fprintf(pOStream,"%s",FX_SEPAR);

								// Write skip flag
								fprintf(pOStream,"%ld",1);
								fprintf(pOStream,"%s",FX_SEPAR);

								// Write native text
								fprintf(pOStream,"%s",szText[nIndex]);

								// Set flags
								nSepFlag=1;
								nFileFlag=1;
								nScrFlag=1;		
							}

							break;

						case 6:
							// Check for text label
							_snprintf(szToken,255,FX_TEXTLABELX,nIndex+1);
							if(strstr(szLine,szToken) != NULL)
							{
								// Set status
								nTokStat=7;

								// Increment index
								nIndex++;
							}

							// Check for text end
							if(strstr(szLine,FX_TEXTEND) != NULL) nTokStat=3;
							break;

						case 7:
							// Check for text end
							if(strstr(szLine,FX_TEXTEND) != NULL) nTokStat=3;
							else
							{
								// Check for text label
								_snprintf(szToken,255,FX_TEXTLABELX,nIndex+1);
								if(strstr(szLine,szToken) != NULL)
								{
									// Set status
									nTokStat=6;
								}
								else
								{
									// Set status
									nTokStat=4;

									// Check index
									if(nIndex < FX_COLUMN-2)
									{
										// Set status
										nTokStat=6;

										// Set file
										strncpy(szText[nIndex],szLine,255);

										// Check index
										if(nIndex < FX_LIMIT-2)
										{
											// Check text for FX_UNDEF and clear
											if(stricmp(szText[nIndex],FX_UNDEF) == 0) strcpy(szText[nIndex],"");

											// Show message
											printf("Processing foreign text %ld [%s]...\n",nIndex,szText[nIndex]);

											// Write text
											fprintf(pOStream,"%s",FX_SEPAR);
											fprintf(pOStream,"%s",szText[nIndex]);
										}
										else
										{
											// Set flag
											nDupFlag=1;

											for(n=0;n<FX_LIMIT-2;n++)
											{
												// Compare text and reset flag
												nDupFlag&=~strcmp(szText[n],szText[nIndex]);
											}
											
											// Check flag
											if(nDupFlag == 1)
											{
												// Show message
												printf("Processing foreign text %ld [%s]...\n",nIndex,szText[nIndex]);

												// Write text
												fprintf(pOStream,"%s",FX_SEPAR);
												fprintf(pOStream,"%s",szText[nIndex]);
											}
										}
									}
								}
							}
							break;
					}
				}

				// Check end-of-file
				if(feof(pTStream) != 0) break;
			}

			// Close temporary file
			fclose(pTStream);
		}	
	}

	// Write line feed
	if(nSepFlag == 1) fprintf(pOStream,"\n");

	// Close output file
	fclose(pOStream);

	// Close input file
	fclose(pIStream);
	printf("\nDone.\n");
	
	return(0);
}
