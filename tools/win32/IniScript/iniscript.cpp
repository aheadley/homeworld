// Copyright (c) 1999 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "iniscript.h"

int main(int argc,char **argv)
{
	unsigned long nIndex;

	int n,nDebug,nPrompt;

	char szFile[1024],szArgs[1024];

	// Show copyright
	printf("IniScript\n");
	printf("Copyright (c) 1999 Relic Entertainment Inc.\n");

	// Check args
	if(argc < 2) 
	{
		printf("Usage: iniscript <script_file> [-d] [-p] [<arg>] [<arg>] ... [<arg>]\n");
		printf("       <script_file>: script file (.ini)\n");
		printf("       -d : debug without executing commands (optional)\n");
		printf("       -p : prompt when missing variables (optional)\n");
		printf("       <arg>: script argument (optional)\n");
		return(-1);
	}

	// Parse file
	strcpy(szFile,"");
	if(strchr(argv[1],'\\') == NULL)
	{
		strcat(szFile,".\\");
	}
	strcat(szFile,argv[1]);

	// Parse switches and args
	nDebug=0;
	nPrompt=0;

	strcpy(szArgs,"");

	if(argc > 2)
	{
		for(n=2;n<argc;n++)
		{
			if((argv[n][0] == '-') || (argv[n][0] == '/'))
			{
				// Debug switch
				if((argv[n][1] == 'd') || (argv[n][1] == 'D'))
				{
					nDebug=1;
				}

				// Prompt switch
				if((argv[n][1] == 'p') || (argv[n][1] == 'P'))
				{
					nPrompt=1;
				}
			}
			else
			{
				// Argument
				strcat(szArgs,argv[n]);
				strcat(szArgs," ");
			}
		}
	}

	// Check debug
	if(nDebug == 1) printf("Debugging script %s...\n\n",szFile);
	else printf("Processing script %s...\n\n",szFile);

	// Reset index
	nIndex=0;

	while(1)
	{
		// Process script
		if(isCommand(szFile,szArgs,nIndex,nDebug,nPrompt) == ERR) break;

		// Increment index
		nIndex++;
	}

	printf("\nDone!\n");

	return(0);
}

int isCommand(char *szFile,char *szArgs,unsigned long nIndex,int nDebug,int nPrompt)
{
	char szKey[1024],szVal[1024],szCom[1024];

	// Create key
	sprintf(szKey,"Command_%ld",nIndex);

	// Get value
	isGetFromIni(szFile,"Script",szKey,szVal);
	
	// Check value
	if(strlen(szVal) == 0) return(ERR);

	// Parse value
	printf("\n************\n");
	printf("%03ld: %s\n",nIndex,szVal);
	isParse(szFile,szArgs,szVal,szCom,nPrompt);

	// Exec command
	printf("Cmd: %s\n\n",szCom);
	if(nDebug == 0) system(szCom);

	return(OK);
}

int isParse(char *szFile,char *szArgs,char *szIn,char *szOut,int nPrompt)
{
	char *pIn,*pBeg,*pEnd,*pTok,szWork[1024],szTok[1024],szVar[1024],szAlias[1024];

	unsigned long nVar,nAlias;

	// Init
	strcpy(szOut,"");
	strcpy(szWork,szIn);

	pIn=szWork;
	pBeg=pIn;
	pEnd=pIn;

	nVar=0;
	nAlias=0;

	// Parse string
	while(pIn[0] != 0)
	{
		// Token begin
		if((nVar == 0) && (nAlias == 0))
		{
			// Check for var token
			if(pIn[0] == '#')
			{
				// Check for var
				if(pIn[1] == '(')
				{
					pIn[0]=0;
					pBeg=pIn;

					strcat(szOut,pEnd);

					nVar=1;
				}

				// Check for alias
				if(pIn[1] == '[')
				{
					pIn[0]=0;
					pBeg=pIn;

					strcat(szOut,pEnd);

					nAlias=1;
				}
			}

			// Check for reg token
			if(pIn[0] == '$')
			{
				// Check for var
				if(pIn[1] == '(')
				{
					pIn[0]=0;
					pBeg=pIn;

					strcat(szOut,pEnd);

					nVar=2;
				}

				// Check for alias
				if(pIn[1] == '[')
				{
					pIn[0]=0;
					pBeg=pIn;

					strcat(szOut,pEnd);

					nAlias=2;
				}
			}

			// Check for env token
			if(pIn[0] == '%')
			{
				// Check for var
				if(pIn[1] == '(')
				{
					pIn[0]=0;
					pBeg=pIn;

					strcat(szOut,pEnd);

					nVar=3;
				}

				// Check for alias
				if(pIn[1] == '[')
				{
					pIn[0]=0;
					pBeg=pIn;

					strcat(szOut,pEnd);

					nAlias=3;
				}
			}

			// Check for arg token
			if(pIn[0] == '^')
			{
				// Check for var
				if(pIn[1] == '(')
				{
					pIn[0]=0;
					pBeg=pIn;

					strcat(szOut,pEnd);

					nVar=4;
				}

				// Check for alias
				if(pIn[1] == '[')
				{
					pIn[0]=0;
					pBeg=pIn;

					strcat(szOut,pEnd);

					nAlias=4;
				}
			}
		}

		// Token end
		if((nVar > 0) && (nAlias == 0))
		{
			// Check for var
			if(pIn[0] == ')')
			{
				pIn[0]=0;
				pEnd=pIn+1;

				// Get token and default
				strcpy(szTok,pBeg+2);
				pTok=strchr(szTok,'|');
				if(pTok != NULL)
				{
					pTok[0]=0;
				}

				switch(nVar)
				{
					// Var token
					case 1:
						isGetFromIni(szFile,"Variables",szTok,szVar);

						// Check var and default
						if((strlen(szVar) == 0) && (pTok != NULL))
						{
							strcpy(szVar,pTok+1);
						}

						// Check prompt
						if(nPrompt == 1)
						{
							if(strlen(szVar) == 0)
							{
								isGetFromPrompt(szTok,szVar);
							}
						}

						strcat(szOut,szVar);
						break;

					// Reg token
					case 2:
						isGetFromReg(szTok,szVar);

						// Check var and default
						if((strlen(szVar) == 0) && (pTok != NULL))
						{
							strcpy(szVar,pTok+1);
						}

						strcat(szOut,szVar);
						break;

					// Env token
					case 3:
						isGetFromEnv(szTok,szVar);

						// Check var and default
						if((strlen(szVar) == 0) && (pTok != NULL))
						{
							strcpy(szVar,pTok+1);
						}

						strcat(szOut,szVar);
						break;

					// Arg token
					case 4:
						isGetFromArg(szArgs,szVar,atol(szTok));

						// Check var and default
						if((strlen(szVar) == 0) && (pTok != NULL))
						{
							strcpy(szVar,pTok+1);
						}

						// Check prompt
						if(nPrompt == 1)
						{
							if(strlen(szVar) == 0)
							{
								isGetFromPrompt(szTok,szVar);
							}
						}

						strcat(szOut,szVar);
						break;
				}

				nVar=0;
			}
		}

		// Token end
		if((nVar == 0) && (nAlias > 0))
		{
			// Check for alias
			if(pIn[0] == ']')
			{
				pIn[0]=0;
				pEnd=pIn+1;

				// Get token and default
				strcpy(szTok,pBeg+2);
				pTok=strchr(szTok,'|');
				if(pTok != NULL)
				{
					pTok[0]=0;
				}

				switch(nAlias)
				{
					// Var token
					case 1:
						isGetFromIni(szFile,"Variables",szTok,szVar);

						// Check var and default
						if((strlen(szVar) == 0) && (pTok != NULL))
						{
							strcpy(szVar,pTok+1);
						}

						// Check prompt
						if(nPrompt == 1)
						{
							if(strlen(szVar) == 0)
							{
								isGetFromPrompt(szTok,szVar);
							}
						}

						isGetFromIni(szFile,"Aliases",szVar,szAlias);

						// Check alias
						if(strlen(szAlias) == 0) strcat(szOut,szVar);
						else strcat(szOut,szAlias);
						break;

					// Reg token
					case 2:
						isGetFromReg(szTok,szVar);

						// Check var and default
						if((strlen(szVar) == 0) && (pTok != NULL))
						{
							strcpy(szVar,pTok+1);
						}

						isGetFromIni(szFile,"Aliases",szVar,szAlias);

						// Check alias
						if(strlen(szAlias) == 0) strcat(szOut,szVar);
						else strcat(szOut,szAlias);
						break;

					// Env token
					case 3:
						isGetFromEnv(szTok,szVar);

						// Check var and default
						if((strlen(szVar) == 0) && (pTok != NULL))
						{
							strcpy(szVar,pTok+1);
						}

						isGetFromIni(szFile,"Aliases",szVar,szAlias);

						// Check alias
						if(strlen(szAlias) == 0) strcat(szOut,szVar);
						else strcat(szOut,szAlias);
						break;

					// Arg token
					case 4:
						isGetFromArg(szArgs,szVar,atol(szTok));

						// Check var and default
						if((strlen(szVar) == 0) && (pTok != NULL))
						{
							strcpy(szVar,pTok+1);
						}

						// Check prompt
						if(nPrompt == 1)
						{
							if(strlen(szVar) == 0)
							{
								isGetFromPrompt(szTok,szVar);
							}
						}

						isGetFromIni(szFile,"Aliases",szVar,szAlias);

						// Check alias
						if(strlen(szAlias) == 0) strcat(szOut,szVar);
						else strcat(szOut,szAlias);
						break;
				}

				nAlias=0;
			}
		}

		// Next char
		pIn++;
	}

	// Process leftover string
	strcat(szOut,pEnd);

	return(OK);
}

int isGetFromIni(char *szFile,char *szSection,char *szKey,char *szData)
{
	// Initialize data
	strcpy(szData,"");

	// Get data
	GetPrivateProfileString(szSection,szKey,"",szData,1023,szFile);
	
	return(OK);
}

int isGetFromReg(char *szReg,char *szData)
{
	char *pKey,*pVal,szRoot[1024],szKey[1024],szVal[1024];

	HKEY hKey,hRoot;
	DWORD dwType,dwLength;

	// Initialize data
	strcpy(szData,"");

	// Parse root, key and value
	strcpy(szRoot,szReg);
	pKey=strchr(szRoot,'\\');
	if(pKey == NULL) return(ERR);
	pVal=strrchr(szRoot,'\\');
	if(pVal == NULL) return(ERR);

	pKey[0]=0;
	pVal[0]=0;	

	// Get key and value
	strcpy(szKey,&pKey[1]);
	strcpy(szVal,&pVal[1]);

	// Get handle
	hRoot=0;
	if(stricmp(szRoot,"HKEY_CLASSES_ROOT") == 0) hRoot=HKEY_CLASSES_ROOT;
	if(stricmp(szRoot,"HKEY_CURRENT_CONFIG") == 0) hRoot=HKEY_CURRENT_CONFIG;
	if(stricmp(szRoot,"HKEY_CURRENT_USER") == 0) hRoot=HKEY_CURRENT_USER;
	if(stricmp(szRoot,"HKEY_LOCAL_MACHINE") == 0) hRoot=HKEY_LOCAL_MACHINE;
	if(stricmp(szRoot,"HKEY_USERS") == 0) hRoot=HKEY_USERS;
	if(stricmp(szRoot,"HKEY_PERFORMANCE_DATA") == 0) hRoot=HKEY_PERFORMANCE_DATA;
	if(stricmp(szRoot,"HKEY_DYN_DATA") == 0) hRoot=HKEY_DYN_DATA;

	// Not registry
	if(hRoot == 0)
	{
		// Get data
		GetPrivateProfileString(szKey,szVal,"",szData,1023,szRoot);

		return(OK);
	}

	// Open registry key
	if(RegOpenKeyEx(hRoot,szKey,0,KEY_QUERY_VALUE,&hKey) != ERROR_SUCCESS) return(ERR);
	
	// Set length
	dwLength=1023;

	// Get registry value
	if(RegQueryValueEx(hKey,szVal,NULL,&dwType,(LPBYTE)szData,&dwLength) != ERROR_SUCCESS)
	{
		// Close registry key
		RegCloseKey(hKey);
		return(ERR);
	}
	
	return(OK);
}

int isGetFromEnv(char *szEnv,char *szData)
{
	char *pData;

	// Initialize data
	strcpy(szData,"");

	// Get environment data
	pData=getenv(szEnv);
	if(pData == NULL) return(ERR);

	// Set data
	strncpy(szData,pData,1023);

	return(OK);
}

int isGetFromArg(char *szArgs,char *szData,unsigned long nNum)
{
	char *pArgs,szWork[1024];

	unsigned long n;

	// Check number
	if(nNum == 0)
	{
		strcpy(szData,"");
		return(ERR);
	}

	// Init
	strcpy(szWork,szArgs);

	// Parse args
	for(n=0;n<nNum;n++)
	{
		if(n == 0) pArgs=strtok(szWork," ");
		else pArgs=strtok(NULL," ");

		if(pArgs == NULL)
		{
			strcpy(szData,"");
			return(ERR);
		}
	}
	
	// Set Data
	strncpy(szData,pArgs,1023);

	return(OK);
}

int isGetFromPrompt(char *szVar,char *szData)
{
	// Get data
	printf("Var: %s=",szVar);
	gets(szData);

	return(OK);
}
