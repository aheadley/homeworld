// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fxfont.h"

int main(int argc,char **argv)
{
	int nTokStat,nLevStat,nFontFlag,nNullFlag;
	
	unsigned long nLen,nRow;

	char *pName,szName[256],szFont[256],*pLine,szLine[FX_MARGIN],*pTmp,szTmp[FX_MARGIN],szData[FX_MARGIN];
	char szSepar[256],szFile[256];

	FILE *pIStream,*pFStream,*pTStream,*pOStream;

    CHOOSEFONT cf;
	LOGFONT lfOld,lfNew,lfCur;

	// Set separator
	strcpy(szSepar,FX_SEPAR);
	
	// Show copyright
	printf("Front-end font exchange tool\n");
	printf("Copyright (c) 1999 Relic Entertainment Inc.\n");

	// Check arguments
	if(argc < 2) 
	{
		// Show message
		printf("\nUsage: FXFont <input_file> [<font_file>]\n");
		return(-1);
	}

	// Get argument
	strncpy(szName,argv[1],255);

	// Check arguments
	if(argc > 2)
	{
		// Get argument
		strncpy(szFont,argv[2],255);

		// Open font file
		pFStream=fopen(szFont,"rb");
		if(pFStream == NULL)
		{
			// Reset stream
			pFStream=stderr;

			// Show message
			printf("Error: unable to open font file [%s]!\n",szFont);
			return(-1);
		}

		// Read font data
		fread((LOGFONT *)&lfOld,1,sizeof(LOGFONT),pFStream);
		fread((LOGFONT *)&lfNew,1,sizeof(LOGFONT),pFStream);

		// Close font file
		fclose(pFStream);

		// Show message
		printf("\nFont to find [%s] (S%ld B%ld I%ld U%ld)....\n",
			lfOld.lfFaceName,lfOld.lfHeight,lfOld.lfWeight,lfOld.lfItalic,lfOld.lfUnderline);

		// Show message
		printf("Font to replace with [%s] (S%ld B%ld I%ld U%ld)....\n",
			lfNew.lfFaceName,lfNew.lfHeight,lfNew.lfWeight,lfNew.lfItalic,lfNew.lfUnderline);
	}
	else
	{
		// Show message
		printf("\nPrompting user for font to find...\n");

		// Initialize members of the CHOOSEFONT structure.  
		cf.lStructSize = sizeof(CHOOSEFONT);
		cf.hwndOwner = (HWND)NULL; 
		cf.hDC = (HDC)NULL;
		cf.lpLogFont = &lfOld;
		cf.iPointSize = 0; 
		cf.Flags = CF_SCREENFONTS;
		cf.rgbColors = RGB(0,0,0); 
		cf.lCustData = 0L;
		cf.lpfnHook = (LPCFHOOKPROC)NULL; 
		cf.lpTemplateName = (LPSTR)NULL;
		cf.hInstance = (HINSTANCE) NULL; 
		cf.lpszStyle = (LPSTR)NULL;
		cf.nFontType = SCREEN_FONTTYPE; 
		cf.nSizeMin = 0;
		cf.nSizeMax = 0;  

		// Display the CHOOSEFONT common-dialog box.
		if(ChooseFont(&cf) == 0)
		{
			// Show message
			printf("Error: user cancelled prompt for font to find!\n");
			return(-1);
		}

		// Show message
		printf("Font to find [%s] (S%ld B%ld I%ld U%ld)....\n",
			lfOld.lfFaceName,lfOld.lfHeight,lfOld.lfWeight,lfOld.lfItalic,lfOld.lfUnderline);

		// Show message
		printf("\nPrompting user for font to replace with...\n");

		// Initialize members of the CHOOSEFONT structure.  
		cf.lStructSize = sizeof(CHOOSEFONT);
		cf.hwndOwner = (HWND)NULL; 
		cf.hDC = (HDC)NULL;
		cf.lpLogFont = &lfNew;
		cf.iPointSize = 0; 
		cf.Flags = CF_SCREENFONTS;
		cf.rgbColors = RGB(0,0,0); 
		cf.lCustData = 0L;
		cf.lpfnHook = (LPCFHOOKPROC)NULL; 
		cf.lpTemplateName = (LPSTR)NULL;
		cf.hInstance = (HINSTANCE) NULL; 
		cf.lpszStyle = (LPSTR)NULL;
		cf.nFontType = SCREEN_FONTTYPE; 
		cf.nSizeMin = 0;
		cf.nSizeMax = 0;  

		// Display the CHOOSEFONT common-dialog box.
		if(ChooseFont(&cf) == 0)
		{
			// Show message
			printf("Error: user cancelled prompt for font to replace with!\n");
			return(-1);
		}

		// Show message
		printf("Font to replace with [%s] (S%ld B%ld I%ld U%ld)....\n",
			lfNew.lfFaceName,lfNew.lfHeight,lfNew.lfWeight,lfNew.lfItalic,lfNew.lfUnderline);
	}

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

	// Initialize strings
	strcpy(szFile,"");
	
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

		// Set file
		strncpy(szFile,szLine,255);

		// Set name
		strncpy(szName,szLine,255);
		pName=strchr(szName,'.');
		if(pName != NULL) pName[0]=0;
		strcat(szName,FX_EXT);

		// Copy file
		if(CopyFile(szFile,szName,FALSE) == 0)
			printf("Warning: unable to backup front-end file [%s]!\n",szFile);

		// Show message
		printf("\nProcessing front-end file [%s]...\n",szFile);

		// Create temporary file
		if(MoveFile(szFile,FX_TEMP) == 0)
				printf("Warning: unable to rename front-end file [%s]!\n",szFile);
		else
		{
			// Open temporary file
			pTStream=fopen(FX_TEMP,"rb");
			if(pTStream == NULL)
				printf("Warning: unable to open temporary file!\n");
			else
			{
				// Open output file
				pOStream=fopen(szFile,"wb");
				if(pOStream == NULL)
					printf("Warning: unable to open output file [%s]!\n",szFile);
				else
				{
					// Reset status
					nTokStat=0;
					nLevStat=0;

					// Reset flags
					nFontFlag=0;
					nNullFlag=0;

					// Reset row count
					nRow=0;

					// Row loop
					while(1)
					{
						// Get line
						fgets(szLine,FX_MARGIN-1,pTStream);

						// Parse line
						while(szLine[strlen(szLine)-1] == '\n') szLine[strlen(szLine)-1]=0;

						// Check line
						if(strlen(szLine) > 0)
						{
							// Check for generic font tags
							if(strstr(szLine,FX_GENFONT) != NULL) nFontFlag=1;
							
							// Check for generic label and user info tags
							if((strstr(szLine,FX_GENLABEL) != NULL) || (strstr(szLine,FX_GENINFO) != NULL))
							{
								// Reset length and data
								nLen=0;
								memset(szData,0,FX_MARGIN*sizeof(char));
								
								// Parse line
								strcpy(szTmp,szLine);
								strtok(szTmp,"\t");
								pTmp=strtok(NULL,"\t");

								// Check line
								if(pTmp != NULL)
								{
									// Get length and data
									nLen=atol(pTmp);
									if(nLen > FX_MARGIN-1) nLen=FX_MARGIN-1;
									fread(szData,nLen+1,sizeof(char),pTStream);
								}

								// Set flag
								nNullFlag=1;
							}
						}

						// Write data
						fprintf(pOStream,"%s",szLine);

						// Write line feed
						if(feof(pTStream) == 0) fputc('\n',pOStream);
	
						// Check null flag
						if(nNullFlag == 1)
						{
							// Clear null flag
							nNullFlag=0;

							// Write data
							fwrite(szData,nLen+1,sizeof(char),pOStream);
						}
	
						// Check font flag
						if(nFontFlag == 1)
						{
							// Clear font flag
							nFontFlag=0;

							// Read font data
							fread((LOGFONT *)&lfCur,1,sizeof(LOGFONT),pTStream);

							while(1)
							{
								// Read until line feed
								if(fgetc(pTStream) == '\n') break;
							}
								
							// Show message
							printf("Line %05ld - Located font [%s] (S%ld B%ld I%ld U%ld)....\n",
								nRow+1,lfCur.lfFaceName,lfCur.lfHeight,lfCur.lfWeight,lfCur.lfItalic,lfCur.lfUnderline);

							// Check font
							if((lfCur.lfHeight == lfOld.lfHeight) &&
								(lfCur.lfWeight == lfOld.lfWeight) &&
								(lfCur.lfItalic == lfOld.lfItalic) &&
								(lfCur.lfUnderline == lfOld.lfUnderline) &&
								(strcmp(lfCur.lfFaceName,lfOld.lfFaceName) == 0))
							{
								// Show message
								printf("Line %05ld - Replacing font...\n",nRow+1);

								// Write font data
								fwrite((LOGFONT *)&lfNew,1,sizeof(LOGFONT),pOStream);
							}
							else fwrite((LOGFONT *)&lfCur,1,sizeof(LOGFONT),pOStream);

							// Write line feed
							fputc('\n',pOStream);
						}

						// Check end-of-file
						if(feof(pTStream) != 0) break;

						// Increment row count
						nRow++;
					}

					// Close output file
					fclose(pOStream);
				}

				// Close temporary file
				fclose(pTStream);
			}
		
			// Delete temporary file
			if(DeleteFile(FX_TEMP) == 0)
				printf("Warning: unable to delete temporary file!\n");
		}
	}

	// Close file
	fclose(pIStream);
	printf("\nDone.\n");
	
	return(0);
}
