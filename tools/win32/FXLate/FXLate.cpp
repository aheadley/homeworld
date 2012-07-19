// Copyright (c) 1998-99 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fxlate.h"

// Language data
char FX_LGDATA[FX_LGNUM][FX_LGSIZE]={"Eng","Fr","De","Es","It","Lg5"};

int main(int argc,char **argv)
{
	int nTokStat,nLevStat;
	int nSkipFlag,nButFlag,nCentFlag,nDimFlag,nFontFlag,nInfoFlag,nLblFlag;
	int nNullFlag,nRectFlag,nScrFlag,nTxtFlag,nEndFlag,nHorzFlag,nVertFlag,nJustFlag;
	
	unsigned long n,nRow,nCol,nIndex,nLen,nPass,nThres;
	long nPosX,nPosY,nDimX,nDimY,nBegX,nBegY,nEndX,nEndY,nCurX,nCurY;
	float fRatio;

	char *pName,szName[256],szLog[256],*pLine,szLine[FX_MARGIN],*pTmp,szTmp[FX_MARGIN],szData[FX_MARGIN];
	char szParam[256],szSepar[256],szFile[256],szScreen[256],szText[FX_LGNUM][256],szToken[256];

	FILE *pIStream,*pLStream,*pTStream,*pOStream;

	LOGFONT lfCur;
	HFONT hfDefault;
	SIZE sCur;
	HDC hdcScreen;

	// Set stream
	pLStream=stderr;

	// Set separator
	strcpy(szSepar,FX_SEPAR);
	
	// Show copyright
	printf("Front-end translation tool\n");
	printf("Copyright (c) 1998-99 Relic Entertainment Inc.\n");

	// Check arguments
	if((argc < 2) || (argc > 6))
	{
		// Show message
		printf("\nUsage: FXLate <input_file> [+H|-H|+V|-V [+H|-H|+V|-V [<log_file> [<lang_thres>]]]]\n");
		fprintf(pLStream,"Line %04ld Pass %01ld - Error: bad argument!\n",0L,0L);
		return(-1);
	}

	// Reset flags
	nHorzFlag=0;
	nVertFlag=0;
	
	// Get argument
	strncpy(szName,argv[1],255);

	// Check arguments
	if(argc > 2)
	{
		// Check prefix
		if(argv[2][0] == '+') nCentFlag=1;
		else nCentFlag=0;

		// Check body
		if(argv[2][1] == 'H') nHorzFlag=nCentFlag;
		if(argv[2][1] == 'h') nHorzFlag=nCentFlag;
		if(argv[2][1] == 'V') nVertFlag=nCentFlag;
		if(argv[2][1] == 'v') nVertFlag=nCentFlag;
	}

	// Check arguments
	if(argc > 3)
	{
		// Check prefix
		if(argv[3][0] == '+') nCentFlag=1;
		else nCentFlag=0;

		// Check body
		if(argv[3][1] == 'H') nHorzFlag=nCentFlag;
		if(argv[3][1] == 'h') nHorzFlag=nCentFlag;
		if(argv[3][1] == 'V') nVertFlag=nCentFlag;
		if(argv[3][1] == 'v') nVertFlag=nCentFlag;
	}

	// Show message
	if(nHorzFlag == 1) printf("\nHorizontally centering all center-justified text (+H)...\n");
	if(nVertFlag == 1) printf("\nVertically centering all text (+V)...\n");

	// Check arguments
	if(argc > 4)
	{
		// Get argument
		strncpy(szLog,argv[4],255);

		// Show message
		printf("\nCreating log file %s...\n",szLog);

		// Open log file
		pLStream=fopen(szLog,"wt");
		if(pLStream == NULL)
		{
			// Reset stream
			pLStream=stderr;

			// Show message
			fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to open log file [%s]!\n",0L,0L,szLog);
		}
	}

	// Reset threshold
	nThres=0;

	// Check arguments
	if(argc > 5)
	{
		// Set threshold
		nThres=atol(argv[5]);
	}

	// Check threshold
	if(nThres == 0) nThres=1;

	// Show message
	printf("\nProcessing input file %s...\n",szName);

	// Open input file
	pIStream=fopen(szName,"rt");
	if(pIStream == NULL)
	{
		// Close log file
		if(pLStream != stderr) fclose(pLStream);

		// Show message
		fprintf(pLStream,"Line %04ld Pass %01ld - Error: unable to open input file [%s]!\n",0L,0L,szName);
		return(-1);
	}

	// Get screen DC
	hdcScreen=GetDC(NULL);
	
	// Initialize strings
	strcpy(szFile,"");
	strcpy(szName,"");
	strcpy(szScreen,"");
	
	// Reset row count
	nRow=0;

	// Row loop
	while(1)
	{
		// Reset skip flag
		nSkipFlag=0;

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
		if(strlen(szLine) == 0)
		{
			nRow++;
			continue;
		}

		// Initialize strings
		strcpy(szText[0],"");
		for(n=1;n<FX_LGNUM;n++) strcpy(szText[n],"");

		// Reset column count 
		nCol=0;
		
		// Set line
		pLine=szLine;

		// Check line
		while(1)
		{
			// Check for NULL-terminator
			if(pLine[0] == 0) break;

			// Check for separator
			if(pLine[0] == szSepar[0]) nCol++;
			else break;

			// Increment pointer
			pLine++;
		}

		// Get first token
		pLine=strtok(szLine,szSepar);

		// Column loop
		while(1)
		{
			// Check line content
			if(pLine == NULL) break;

			// Check column count
			if(nCol >= FX_NUM) break;
			switch(nCol)
			{
				case 0:
					// Check file
					if(strlen(pLine) > 0)
					{
						// Set file
						strncpy(szFile,pLine,255);

						// Set name
						strncpy(szName,pLine,255);
						pName=strchr(szName,'.');
						if(pName != NULL) pName[0]=0;
						strcat(szName,FX_EXT);

						// Copy file
						if(CopyFile(szFile,szName,FALSE) == 0)
							fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to backup front-end file [%s]!\n",nRow+1,0L,szFile);
					}					
					break;

				case 1:
					// Check screen
					if(strlen(pLine) > 0)
					{
						// Get screen
						strncpy(szScreen,pLine,255);

						// Parse screen
						while(szScreen[strlen(szScreen)-1] == ' ') szScreen[strlen(szScreen)-1]=0;

						// Replace spaces with underscores
						for(n=0;n<(int)strlen(szScreen);n++)
						{
							if(szScreen[n] == ' ') szScreen[n]='_';
						}
					}
					break;

				case 2:
					// Check screen
					if(strlen(pLine) > 0)
					{
						// Get skip flag
						if(atol(pLine) > 0) nSkipFlag=1;
					}
					break;
			
				default:
					// Check screen
					if(strlen(pLine) > 0)
					{
						// Get text
						strncpy(szText[nCol-3],pLine,255);

						// Parse text
						while(szText[nCol-3][strlen(szText[nCol-3])-1] == ' ') szText[nCol-3][strlen(szText[nCol-3])-1]=0;
					}
					break;
			}

			// Increment column count
			nCol++;

			// Get next token
			pLine=strtok(NULL,szSepar);
		}						

		// Check file
		if((strlen(szFile) == 0) || (strlen(szScreen) == 0) || (strlen(szText[0]) == 0))
		{
			nRow++;
			continue;
		}

		// Check skip flag
		if(nSkipFlag == 1)
		{
			printf("\nSkipping line %ld...\n",nRow+1);
			nRow++;
			continue;
		}
		
		// Two passes
		for(nPass=0;nPass<2;nPass++)
		{
			// Show message
			printf("\nProcessing line %ld, pass %ld...\n",nRow+1,nPass+1);
			printf("Processing front-end file [%s]...\n",szFile);

			// Create temporary file
			if(MoveFile(szFile,FX_TEMP) == 0)
					fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to rename front-end file [%s]!\n",nRow+1,nPass+1,szFile);
			else
			{
				// Open temporary file
				pTStream=fopen(FX_TEMP,"rb");
				if(pTStream == NULL)
					fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to open temporary file!\n",nRow+1,nPass+1);
				else
				{
					// Open output file
					pOStream=fopen(szFile,"wb");
					if(pOStream == NULL)
						fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to open output file [%s]!\n",nRow+1,nPass+1,szFile);
					else
					{
						// Show message
						printf("Locating screen [%s]...\n",szScreen);

						// Reset status
						nTokStat=0;
						nLevStat=0;

						// Reset flags
						nButFlag=0;
						nCentFlag=0;
						nDimFlag=0;
						nFontFlag=0;
						nInfoFlag=0;
						nLblFlag=0;
						nNullFlag=0;
						nRectFlag=0;
						nScrFlag=0;
						nTxtFlag=0;
						nEndFlag=0;

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
											if((nScrFlag == 0) && (nLevStat == 0)) nLevStat=1;

											// Parse line
											strcpy(szTmp,szLine);
											strtok(szTmp,"\t");
											pTmp=strtok(NULL,"\t");

											// Check line
											if(pTmp != NULL)
											{
												// Check screen
												if(stricmp(pTmp,szScreen) == 0)
												{
													// Set screen flag
													nScrFlag=1;

													// Set status
													nTokStat=3;
													if(nLevStat > 0) nLevStat=0;

													// Show message
													printf("Processing screen [%s]...\n",szScreen);

													// Check pass
													if(nPass == 1) printf("Locating rectangle for native text [%s]...\n",szText[0]);
													else printf("Locating native text [%s]...\n",szText[0]);
												}
											}
										}

										// Check for screen end
										if(strstr(szLine,FX_SCREENEND) != NULL) nTokStat=1;
										break;

									case 3:
										// Check pass
										if(nPass == 1)
										{
											// Check for rectangle tag
											if(strstr(szLine,FX_RECTTAG) != NULL) nTokStat=14;

											// Check for text tag
											if(strstr(szLine,FX_TEXTTAG) != NULL) nTokStat=34;
										}
										else
										{
											// Check for text tag
											if(strstr(szLine,FX_TEXTTAG) != NULL)
											{
												// Set status
												nTokStat=24;
											}
										}

										// Check for screen end
										if(strstr(szLine,FX_SCREENEND) != NULL) nTokStat=1;
										break;

									case 14:
										// Check for object bounds
										if(strstr(szLine,FX_OBJBOUNDS) != NULL)
										{
											// Check center flag
											if(nCentFlag == 1) break;

											// Set status
											if((nButFlag == 0) && (nLevStat == 0) &&
												(strchr(szText[0],FX_HOTKEY) != NULL)) nLevStat=12;

											// Reset positions
											nBegX=0;
											nBegY=0;
											nEndX=0;
											nEndY=0;

											// Parse line
											strcpy(szTmp,szLine);
											strtok(szTmp,"\t");
											pTmp=strtok(NULL,"\t");

											// Check line
											if(pTmp == NULL) break;

											// Parse params
											strncpy(szParam,pTmp,255);
											pTmp=strtok(szParam,",");
											if(pTmp == NULL) break;

											// Get x begin
											nBegX=atol(pTmp);
											pTmp=strtok(NULL,",");
											if(pTmp == NULL) break;

											// Get y begin
											nBegY=atol(pTmp);
											pTmp=strtok(NULL,",");
											if(pTmp == NULL) break;

											// Get x end
											nEndX=atol(pTmp);
											pTmp=strtok(NULL,",");
											if(pTmp == NULL) break;

											// Get y end
											nEndY=atol(pTmp);

											// Check position
											if((nPosX >= nBegX) && (nPosY >= nBegY) &&
												(nPosX+nDimX <= nEndX) && (nPosY+nDimY <= nEndY))
											{
												// Set rectangle flag
												nRectFlag=1;

												// Set status
												if(nLevStat > 0) nLevStat=0;
											}
											else
											{
												// Calculate ratio
												fRatio=(((nPosX+nDimX < nEndX)?nPosX+nDimX:nEndX)-((nPosX > nBegX)?nPosX:nBegX))/(float)nDimX;

												// CHeck position and ratio
												if((nPosY >= nBegY) && (nPosY+nDimY <= nEndY) && (fRatio >= FX_RATIO))
												{
													// Set rectangle flag
													nRectFlag=1;

													// Set status
													if(nLevStat > 0) nLevStat=0;
												}
												else
												{
													// Reset positions
													nBegX=0;
													nBegY=0;
													nEndX=0;
													nEndY=0;
												}
											}
										}

										// Check for object user info
										if(strstr(szLine,FX_OBJINFO) != NULL)
										{
											// Check rectangle flag
											if(nRectFlag == 1)
											{
												// Clear rectangle flag
												nRectFlag=0;
												
												// Set info flag
												nInfoFlag=1;
											}
										}

										// Check for rectangle end
										if(strstr(szLine,FX_RECTEND) != NULL) nTokStat=3;
										break;

									case 24:									
										// Check for object source
										if(strstr(szLine,FX_OBJSRC) != NULL)
										{
											// Check text flag
											if(nTxtFlag == 1) break;

											// Reset positions
											nPosX=0;
											nPosY=0;
		
											// Parse line
											strcpy(szTmp,szLine);
											strtok(szTmp,"\t");
											pTmp=strtok(NULL,"\t");

											// Check line
											if(pTmp == NULL) break;

											// Parse params
											strncpy(szParam,pTmp,255);
											pTmp=strtok(szParam,",");
											if(pTmp == NULL) break;

											// Get x position
											nPosX=atol(pTmp);
											pTmp=strtok(NULL,",");
											if(pTmp == NULL) break;

											// Get y position
											nPosY=atol(pTmp);
										}

										// Check for text label
										if(strstr(szLine,FX_TEXTLABEL) != NULL)
										{									
											// Reset index;
											nIndex=0;

											// Set label flag
											nLblFlag=1;
										}

										// Check for text end
										if(strstr(szLine,FX_TEXTEND) != NULL) nTokStat=3;
										break;

									case 25:
										// Check for text label
										_snprintf(szToken,255,FX_TEXTLABELX,nIndex+1);
										if(strstr(szLine,szToken) != NULL)
										{
											// Increment index
											nIndex++;

											// Check index
											if(nIndex < FX_LGNUM)
											{
												// Set label flag
												nLblFlag=1;
											}
										}

										// Check for text label
										if(strstr(szLine,FX_TEXTJUST) != NULL)
										{									
											// Check end flag
											if(nEndFlag == 1) break;

											// Check text flag
											if(nTxtFlag == 1) nEndFlag=1;

											// Reset justification flag
											nJustFlag=0;

											// Parse line
											strcpy(szTmp,szLine);
											strtok(szTmp,"\t");
											pTmp=strtok(NULL,"\t");

											// Check line
											if(pTmp == NULL) break;

											// Parse params
											strncpy(szParam,pTmp,255);
											pTmp=strtok(szParam,",");
											if(pTmp == NULL) break;

											// Set justification flag
											if(atol(pTmp) == FX_JUSTCENT) nJustFlag=1;
										}

										// Check for text end
										if(strstr(szLine,FX_TEXTEND) != NULL) nTokStat=3;
										break;

									case 34:									
										// Check for object bounds
										if(strstr(szLine,FX_OBJBOUNDS) != NULL)
										{
											// Check center flag
											if(nCentFlag == 1)
											{
												// Reset current x and y
												nCurX=0;
												nCurY=0;

												// Parse line
												strcpy(szTmp,szLine);
												strtok(szTmp,"\t");
												pTmp=strtok(NULL,"\t");

												// Check line
												if(pTmp == NULL) break;

												// Parse params
												strncpy(szParam,pTmp,255);
												pTmp=strtok(szParam,",");
												if(pTmp == NULL) break;

												// Get x position
												nCurX=atol(pTmp);
												pTmp=strtok(NULL,",");
												if(pTmp == NULL) break;

												// Get y position
												nCurY=atol(pTmp);

												// Check positions
												if((nCurX == nPosX) && (nCurY == nPosY))
												{
													// Center string
													nCurX=nBegX+(nEndX-nBegX-nDimX)/2-FX_OFFSET;
													nCurY=nBegY+(nEndY-nBegY-nDimY)/2;

													// Compare result
													if((((nCurX-nPosX)>0?nCurX-nPosX:nPosX-nCurX) < nEndX-nBegX) &&
														(((nCurY-nPosY)>0?nCurY-nPosY:nPosY-nCurY) < nEndY-nBegY))
													{
														// Set line
														if(nHorzFlag == 0) nCurX=nPosX;
														if((nHorzFlag == 1) && (nJustFlag == 0)) nCurX=nPosX;
														if(nVertFlag == 0) nCurY=nPosY;
														_snprintf(szLine,FX_MARGIN-1,"\t\t\t%s\t%ld, %ld, %ld, %ld",FX_OBJBOUNDS,nCurX,nCurY,nCurX+nDimX,nCurY+nDimY);
													}
												}
											}
										}

										// Check for object source
										if(strstr(szLine,FX_OBJSRC) != NULL)
										{
											// Check center flag
											if(nCentFlag == 1)
											{
												// Reset current x and y
												nCurX=0;
												nCurY=0;

												// Parse line
												strcpy(szTmp,szLine);
												strtok(szTmp,"\t");
												pTmp=strtok(NULL,"\t");

												// Check line
												if(pTmp == NULL) break;

												// Parse params
												strncpy(szParam,pTmp,255);
												pTmp=strtok(szParam,",");
												if(pTmp == NULL) break;

												// Get x position
												nCurX=atol(pTmp);
												pTmp=strtok(NULL,",");
												if(pTmp == NULL) break;

												// Get y position
												nCurY=atol(pTmp);

												// Check positions
												if((nCurX == nPosX) && (nCurY == nPosY))
												{
													// Set status
													if(nLevStat == 0) nLevStat=15;

													// Centre string
													nCurX=nBegX+(nEndX-nBegX-nDimX)/2-FX_OFFSET;
													nCurY=nBegY+(nEndY-nBegY-nDimY)/2;

													// Compare result
													if((((nCurX-nPosX)>0?nCurX-nPosX:nPosX-nCurX) < nEndX-nBegX) &&
														(((nCurY-nPosY)>0?nCurY-nPosY:nPosY-nCurY) < nEndY-nBegY))
													{
														// Clear centre flag
														nCentFlag=0;

														// Set status
														if(nLevStat > 0) nLevStat=0;

														// Show message
														if((nHorzFlag == 1) && (nJustFlag == 1)) printf("Horizontally centering text [%s]...\n",szText[0]);
														if(nVertFlag == 1) printf("Vertically centering text [%s]...\n",szText[0]);

														// Set line
														if(nHorzFlag == 0) nCurX=nPosX;
														if((nHorzFlag == 1) && (nJustFlag == 0)) nCurX=nPosX;
														if(nVertFlag == 0) nCurY=nPosY;
														_snprintf(szLine,FX_MARGIN-1,"\t\t\t%s\t%ld, %ld",FX_OBJSRC,nCurX,nCurY);
													}
												}
											}
										}

										// Check for text end
										if(strstr(szLine,FX_TEXTEND) != NULL) nTokStat=3;
										break;
								}
							}

							// Check label flag
							if(nLblFlag == 1)
							{						
								// Clear label flag
								nLblFlag=0;

								// Set status
								if((nTxtFlag == 0) && (nLevStat == 0)) nLevStat=nIndex+2;

								// Parse data
								strcpy(szTmp,szData);
								pTmp=strchr(szTmp,'\n');
								if(pTmp != NULL) pTmp=0;

								// Parse data again
								while((szTmp[strlen(szTmp)-1] == ' ') || (szTmp[strlen(szTmp)-1] == '\r'))
									szTmp[strlen(szTmp)-1]=0;
									
								// Check index
								if(nIndex == 0)
								{
									// Check text
									if(stricmp(szTmp,szText[nIndex]) == 0)
									{
										// Set text and dimension flag
										nTxtFlag=1;
										nDimFlag=1;

										// Set status
										nTokStat=25;
										if(nLevStat > 0) nLevStat=0;

										// Show message
										printf("Processing native text [%s]...\n",szText[nIndex]);
									}
								}
								else
								{
									// Set status
									if(nLevStat > 0) nLevStat=0;

									// Check threshold
									if(nIndex >= nThres)
									{
										// Show message
										printf("Adding foreign (%s) text [%s]...\n",FX_LGDATA[nIndex],szText[nIndex]);

										// Set length
										nLen=strlen(szText[nIndex])+1;
										if(nLen > FX_MARGIN-1) nLen=FX_MARGIN-1;

										// Set line
										_snprintf(szLine,FX_MARGIN-1,"\t\t\t%s\t%ld",szToken,nLen);

										// Set data
										memset(szData,0,FX_MARGIN*sizeof(char));
										strncpy(szData,szText[nIndex],FX_MARGIN-2);
										szData[nLen]='\n';
									}
									else
									{
										// Set text
										strncpy(szText[nIndex],szTmp,255);

										// Show message
										printf("Skipping foreign (%s) text [%s]...\n",FX_LGDATA[nIndex],szText[nIndex]);
									}
								}
							}

							// Check button and info flag
							if((nButFlag == 0) && (nInfoFlag == 1))
							{						
								// Clear info flag
								nInfoFlag=0;

								// Set status
								if((nButFlag == 0) && (nLevStat == 0) &&
									(strchr(szText[0],FX_HOTKEY) != NULL)) nLevStat=13;

								// Parse data
								strcpy(szTmp,szData);

								// Get first token
								pTmp=strtok(szTmp,"\n");

								// Params loop
								while(1)
								{
									// Check params content
									if(pTmp == NULL) break;

									// Parse params
									strncpy(szParam,pTmp,255);
									while((szParam[strlen(szParam)-1] == ' ') || (szParam[strlen(szParam)-1] == '\r'))
										szParam[strlen(szParam)-1]=0;

									// Check params
									if((stricmp(szParam,FX_INFOBUTTON) == 0)
										|| (stricmp(szParam,FX_INFOTOGGLE) == 0)										
										|| (stricmp(szParam,FX_INFODRAG) == 0)										
										|| (stricmp(szParam,FX_INFORADIO) == 0)
										|| (stricmp(szParam,FX_INFOBITMAP) == 0)
										|| (stricmp(szParam,FX_INFOCHECK) == 0))
									{
										// Set button flag
										nButFlag=1;
										break;
									}
									// Get next token
									pTmp=strtok(NULL,"\n");
								}

								// Check button flag
								if(nButFlag == 1)
								{
									// Set centre flag
									if((nHorzFlag == 1) || (nVertFlag == 1)) nCentFlag=1;

									// Set status
									if(nLevStat > 0) nLevStat=0;

									// Show message
									printf("Processing rectangle...\n");

									// Find hotkey in text
									if(strchr(szText[0],FX_HOTKEY) != NULL)
									{
										// Parse data again
										strcpy(szTmp,szData);

										// Reset length and data
										nLen=0;
										memset(szData,0,FX_MARGIN*sizeof(char));
										
										// Get first token
										pTmp=strtok(szTmp,"\n");

										// Params loop
										while(1)
										{
											// Check params content
											if(pTmp == NULL) break;

											// Parse params
											strncpy(szParam,pTmp,255);
											while((szParam[strlen(szParam)-1] == ' ') || (szParam[strlen(szParam)-1] == '\r'))
												szParam[strlen(szParam)-1]=0;

											// Find bracket in params
											strcpy(szToken,szParam);
											pTmp=strchr(szToken,'(');
											if(pTmp != NULL) pTmp[0]=0;
																					
											// Check params
											if(stricmp(szToken,FX_INFOHOTKEY) != 0)
											{
												// Set length and data
												nLen=nLen+strlen(szParam)+2;
												strcat(szData,szParam);
												strcat(szData,"\r\n");
											}
											
											// Get next token
											pTmp=strtok(NULL,"\n");
										}

										// Set status
										if(nLevStat == 0) nLevStat=14;

										for(n=0;n<FX_LGNUM;n++)
										{
											// Find hotkey in text
											pTmp=strchr(szText[n],FX_HOTKEY);
											if(pTmp == NULL) continue;

											// Set status
											if(nLevStat > 0) nLevStat=0;

											// Show message
											if(n == 0) printf("Adding native hotkey [%c]...\n",pTmp[1]);
											else printf("Adding foreign (%s) hotkey [%c]...\n",FX_LGDATA[n],pTmp[1]);

											// Set params
											_snprintf(szParam,255,"%s(%s,%c)",FX_INFOHOTKEY,FX_LGDATA[n],pTmp[1]);

											// Set length and data
											nLen=nLen+strlen(szParam)+2;
											strcat(szData,szParam);
											strcat(szData,"\r\n");
										}

										// Set line
										nLen++;
										_snprintf(szLine,FX_MARGIN-1,"\t\t\t%s\t%ld",FX_OBJINFO,nLen);

										// Terminate data
										szData[nLen]='\n';
									}
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
								
								// Write font data
								fwrite((LOGFONT *)&lfCur,1,sizeof(LOGFONT),pOStream);
								fputc('\n',pOStream);

								// Check dimension flag
								if(nDimFlag == 1)
								{
									// Clear dimension flag
									nDimFlag=0;

									// Select font
									hfDefault=(HFONT)SelectObject(hdcScreen,CreateFontIndirect(&lfCur));

									// Reset dimensions
									nDimX=0;
									nDimY=0;

									// Loop thru text
									for(n=0;n<FX_LGNUM;n++)
									{
										// Check for hotkey
										strncpy(szTmp,szText[n],FX_MARGIN-2);
										pTmp=strchr(szTmp,FX_HOTKEY);
										if(pTmp != NULL)
										{
											// Remove hotkey
											pTmp[0]=0;
											strcat(szTmp,&pTmp[1]);
										}

										// Calculate dimensions
										GetTextExtentPoint32(hdcScreen,szTmp,strlen(szTmp),&sCur);
										if(sCur.cx > nDimX) nDimX=sCur.cx;
										if(sCur.cy > nDimY) nDimY=sCur.cy;
									}

									// Select original font
									SelectObject(hdcScreen,hfDefault);
								}
							}

							// Check end-of-file
							if(feof(pTStream) != 0) break;
						}

						// Close output file
						fclose(pOStream);
					}

					// Close temporary file
					fclose(pTStream);
				}
			
				// Check status
				switch(nLevStat)
				{
					case 0:
						// No problem
						break;
				
					case 1:
						// Show message
						fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to locate screen [%s]!\n",nRow+1,nPass+1,szScreen);
						break;

					case 2:
						// Show message
						fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to locate native text [%s]!\n",nRow+1,nPass+1,szText[0]);
						break;

					case 12:
						// Show message
						fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to locate valid rectangle (native text [%s])!\n",nRow+1,nPass+1,szText[0]);
						break;

					case 13:
						// Show message
						fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to locate user information (native text [%s])!\n",nRow+1,nPass+1,szText[0]);
						break;

					case 14:
						// Show message
						fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to add hotkeys (native text [%s])!\n",nRow+1,nPass+1,szText[0]);
						break;

					case 15:
						// Show message
						fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to center text (native text [%s])!\n",nRow+1,nPass+1,szText[0]);
						break;

					default:
						// Show message
						fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to add foreign text %ld [%s]!\n",nRow+1,nPass+1,nLevStat-1,szText[nLevStat-2]);
						break;
				}

				// Delete temporary file
				if(DeleteFile(FX_TEMP) == 0)
					fprintf(pLStream,"Line %04ld Pass %01ld - Warning: unable to delete temporary file!\n",nRow+1);
			}

			// Check status
			if(nLevStat > 0) break;
		}
			
		// Increment row count
		nRow++;
	}

	// Release screen DC
	ReleaseDC(NULL,hdcScreen);
	
	// Close file
	fclose(pIStream);
	printf("\nDone.\n");
	
	// Close log file
	if(pLStream != stderr) fclose(pLStream);

	return(0);
}
