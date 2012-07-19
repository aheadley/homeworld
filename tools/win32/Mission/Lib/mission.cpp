// Copyright (c) 1998-99 Relic Entertainment Inc.
// Written by Janik Joire
// Original code for misGetHash() by Jason Dorie
//
// $History: $

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mission.h"

char MIS_INI[MIS_LEN];
char MIS_CFG[MIS_LEN];

__declspec(dllexport) int __stdcall misSetFile(char *szFile,int nMode)
{
	// Set file
	if(nMode == MIS_MOD_INI) strncpy(MIS_INI,szFile,MIS_LEN-1);
	else strncpy(MIS_CFG,szFile,MIS_LEN-1);

	return(OK);
}

__declspec(dllexport) int __stdcall misGetVal(char *szSection,char *szKey,char **szVal,int nMode)
{
	char szTmp[256],szTmpEx[256];

	HKEY hKey;
	DWORD dwType,dwIndex,dwLength,dwLengthEx;

	// Initialize value
	strcpy(szTmp,"");

	// Check mode
	if(nMode == MIS_MOD_REG)
	{
		// Open registry key (section)
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,szSection,0,KEY_QUERY_VALUE,&hKey) != ERROR_SUCCESS)
			return(MIS_ERR_REG);
		
		// Set length
		dwLength=sizeof(szTmp)-1;

		// Check key
		if(szKey[0] == '#')
		{
			// Set index
			dwIndex=atol(&szKey[1]);

			// Initialize value
			strcpy(szTmpEx,"");

			// Set length
			dwLengthEx=sizeof(szTmpEx)-1;

			// Get registry value by index (key)
			if(RegEnumValue(hKey,dwIndex,szTmp,&dwLength,NULL,&dwType,(LPBYTE)szTmpEx,&dwLengthEx) != ERROR_SUCCESS)
			{
				// Close registry key
				RegCloseKey(hKey);
				return(MIS_ERR_REG);
			}

			// Check type
			if(dwType != REG_SZ)
			{
				// Reset value
				strcpy(szTmpEx,"");
			}

			// Allocate memory for value
			*szVal=(char *)calloc(strlen(szTmp)+strlen(szTmpEx)+2,sizeof(char));
			if(*szVal == NULL)
			{
				// Close registry key
				RegCloseKey(hKey);
				return(MIS_ERR_ALLOC);
			}
			
			// Copy value
			sprintf(*szVal,"%s=%s",szTmp,szTmpEx);

			// Close registry key
			RegCloseKey(hKey);

			return(OK);
		}

		// Get registry value (key)
		if(RegQueryValueEx(hKey,szKey,NULL,&dwType,(LPBYTE)szTmp,&dwLength) != ERROR_SUCCESS)
		{
			// Close registry key
			RegCloseKey(hKey);
			return(MIS_ERR_REG);
		}
	}
	else
	{
		// Get value
		if(nMode == MIS_MOD_INI) GetPrivateProfileString(szSection,szKey,"",szTmp,sizeof(szTmp)-1,MIS_INI);
		else GetPrivateProfileString(szSection,szKey,"",szTmp,sizeof(szTmp)-1,MIS_CFG);
	}

	// Allocate memory for value
	*szVal=(char *)calloc(strlen(szTmp)+1,sizeof(char));
	if(*szVal == NULL) return(MIS_ERR_ALLOC);
	
	// Copy value
	strcpy(*szVal,szTmp);

	return(OK);
}

__declspec(dllexport) int __stdcall misGetListByKey(char *szSection,char *szPrefix,char **szList,unsigned short *nCount,int nMode)
{
	unsigned short nInd,nSize;

	char szKey[256],szVal[256];

	DWORD dwResult;

	// Check mode
	if(nMode == MIS_MOD_REG) return(MIS_ERR_REG);

	// Reset size, index and count
	nSize=0;
	nInd=0;
	*nCount=0;
	
	while(1)
	{
		// Set key
		_snprintf(szKey,sizeof(szKey)-1,"%s%d",szPrefix,nInd);

		// Initialize value
		strcpy(szVal,"");

		// Get value
		if(nMode == MIS_MOD_INI) dwResult=GetPrivateProfileString(szSection,szKey,"",szVal,sizeof(szVal)-1,MIS_INI);
		else dwResult=GetPrivateProfileString(szSection,szKey,"",szVal,sizeof(szVal)-1,MIS_CFG);
		if(dwResult > 0)
		{
			// Adjust size
			nSize=nSize+strlen(szVal)+1;

			// Increment count
			(*nCount)++;
		}
		else break;

		// Increment index
		nInd++;
	}

	// Check count
	if(*nCount == 0) nSize++;

	// Allocate memory for list
	*szList=(char *)calloc(nSize,sizeof(char));
	if(*szList == NULL) return(MIS_ERR_ALLOC);
	
	// Initialize list
	strcpy(*szList,"");

	// Check count
	if(*nCount == 0) return(OK);

	// Reset index and count
	nInd=0;
	*nCount=0;
	
	while(1)
	{
		// Set key
		_snprintf(szKey,sizeof(szKey)-1,"%s%d",szPrefix,nInd);

		// Get value
		if(nMode == MIS_MOD_INI) dwResult=GetPrivateProfileString(szSection,szKey,"",szVal,sizeof(szVal)-1,MIS_INI);
		else dwResult=GetPrivateProfileString(szSection,szKey,"",szVal,sizeof(szVal)-1,MIS_CFG);
		if(dwResult > 0)
		{
			// Copy value to list
			if(*nCount > 0) strcat(*szList,MIS_SEP);
			strcat(*szList,szVal);

			// Increment count
			(*nCount)++;
		}
		else break;

		// Increment index
		nInd++;
	}

	return(OK);
}

__declspec(dllexport) int __stdcall misGetListBySection(char *szPrefix,char *szKey,char **szTypes,unsigned short *nCount,int nMode)
{
	unsigned short nInd,nSize;

	char szSection[256],szVal[256];

	DWORD dwResult;

	// Check mode
	if(nMode == MIS_MOD_REG) return(MIS_ERR_REG);

	// Reset size, index and count
	nSize=0;
	nInd=0;
	*nCount=0;
	
	while(1)
	{
		// Set section
		_snprintf(szSection,sizeof(szSection)-1,"%s%d",szPrefix,nInd);

		// Initialize value
		strcpy(szVal,"");

		// Get value
		if(nMode == MIS_MOD_INI) GetPrivateProfileString(szSection,szKey,"",szVal,sizeof(szVal)-1,MIS_INI);
		else dwResult=GetPrivateProfileString(szSection,szKey,"",szVal,sizeof(szVal)-1,MIS_CFG);
		if(dwResult > 0)
		{
			// Adjust size
			nSize=nSize+strlen(szVal)+1;

			// Increment count
			(*nCount)++;
		}
		else break;

		// Increment index
		nInd++;
	}

	// Check count
	if(*nCount == 0) nSize++;

	// Allocate memory for types
	*szTypes=(char *)calloc(nSize,sizeof(char));
	if(*szTypes == NULL) return(MIS_ERR_ALLOC);
	
	// Initialize types
	strcpy(*szTypes,"");

	// Check count
	if(*nCount == 0) return(OK);

	// Reset index and count
	nInd=0;
	*nCount=0;
	
	while(1)
	{
		// Set section
		_snprintf(szSection,sizeof(szSection)-1,"%s%d",szPrefix,nInd);

		// Initialize value
		strcpy(szVal,"");

		// Get value
		if(nMode == MIS_MOD_INI) GetPrivateProfileString(szSection,szKey,"",szVal,sizeof(szVal)-1,MIS_INI);
		else dwResult=GetPrivateProfileString(szSection,szKey,"",szVal,sizeof(szVal)-1,MIS_CFG);
		if(dwResult > 0)
		{
			// Copy value to types
			if(*nCount > 0) strcat(*szTypes,MIS_SEP);
			strcat(*szTypes,szVal);

			// Increment count
			(*nCount)++;
		}
		else break;

		// Increment index
		nInd++;
	}

	return(OK);
}

__declspec(dllexport) int __stdcall misPutVal(char *szSection,char *szKey,char *szVal,int nMode)
{
	// Check mode
	if(nMode == MIS_MOD_REG) return(MIS_ERR_REG);

	// Put value
	if(nMode == MIS_MOD_INI) WritePrivateProfileString(szSection,szKey,szVal,MIS_INI);
	else WritePrivateProfileString(szSection,szKey,szVal,MIS_CFG);

	return(OK);
}

__declspec(dllexport) int __stdcall misPutListByKey(char *szSection,char *szPrefix,char *szList,int nMode)
{
	unsigned short nInd;

	char *pVal,*szRList,szKey[256];

	// Check mode
	if(nMode == MIS_MOD_REG) return(MIS_ERR_REG);

	// Allocate memory for list
	szRList=(char *)calloc(strlen(szList)+1,sizeof(char));
	if(szRList == NULL) return(MIS_ERR_ALLOC);
	
	// Copy list
	strcpy(szRList,szList);

	// Find first value in list
	pVal=strtok(szRList,MIS_SEP);
	if(pVal == NULL) pVal=szRList;

	// Reset index
	nInd=0;
	
	while(1)
	{
		// Set key
		_snprintf(szKey,sizeof(szKey)-1,"%s%d",szPrefix,nInd);

		// Put value
		if(nMode == MIS_MOD_INI) WritePrivateProfileString(szSection,szKey,pVal,MIS_INI);
		else WritePrivateProfileString(szSection,szKey,pVal,MIS_CFG);

		// Find next value in list
		pVal=strtok(NULL,MIS_SEP);
		if(pVal == NULL) break;
		
		// Increment index
		nInd++;
	}
	
	// Free memory for list
	free(szRList);
	
	return(OK);
}

__declspec(dllexport) int __stdcall misOpen(FILE **pStream,char *szFile)
{
	// Open file
	*pStream=fopen(szFile,"wt");
	if(*pStream == NULL) return(MIS_ERR_OPEN);

	return(OK);
}

__declspec(dllexport) int __stdcall misWriteAttrib(FILE *pStream,char *szName,char *szVal)
{
	// Write attrib
	fprintf(pStream,"%s    %s\n",szName,szVal);
	
	return(OK);
}

__declspec(dllexport) int __stdcall misWriteInfo(FILE *pStream,char *szInfo)
{
	// Write info
	fprintf(pStream,"; %s\n",szInfo);
	
	return(OK);
}

__declspec(dllexport) int __stdcall misWriteNew(FILE *pStream)
{
	// Write new line
	fprintf(pStream,"\n");
	
	return(OK);
}

__declspec(dllexport) int __stdcall misWriteVal(FILE *pStream,char *szFormat,char *szVal)
{
	// Write value
	fprintf(pStream,szFormat,szVal);
	
	return(OK);
}

__declspec(dllexport) int __stdcall misClose(FILE *pStream)
{
	// Close file
	fclose(pStream);

	return(OK);
}

__declspec(dllexport) int __stdcall misShellExec(char *szFile)
{
	// Open file using shell
	if((int)ShellExecute(NULL,"open",szFile,NULL,NULL,NULL) <= 32) return(ERR);

	return(OK);
}

__declspec(dllexport) int __stdcall misGetVer(char **szVer)
{
	// Allocate memory for version
	*szVer=(char *)calloc(strlen(MIS_VER)+1,sizeof(char));
	if(*szVer == NULL) return(MIS_ERR_ALLOC);
	
	// Copy version
	strcpy(*szVer,MIS_VER);

	return(OK);
}

__declspec(dllexport) unsigned long __stdcall misGetHash(char *szText)
{
	unsigned long i,nHash;

	nHash=0;

	while(*szText != 0)
	{
		nHash=(nHash<<4)+*szText++;
		if((i=nHash&0xF0000000) != 0) nHash=(nHash^(i>>24))^i;
	}

	return(nHash);
}

__declspec(dllexport) int __stdcall misGetErr(int nErr,char **szMsg)
{
	char *pMsg;

	switch(nErr)
	{
		// Error codes
		case MIS_ERR_ALLOC:	pMsg="Unable to allocate memory";
							break;
		case MIS_ERR_OPEN:	pMsg="Unable to open file";
							break;
		case MIS_ERR_REG:	pMsg="Unable to access registry";
							break;
		
		// Default error codes
		case 0:		pMsg="No error";
					break;

		default:	pMsg="Undefined error";
	}

	// Allocate memory for text
	*szMsg=(char *)calloc(strlen(pMsg)+1,sizeof(char));
	if(*szMsg == NULL) return(ERR);

	// Copy text
	strcpy(*szMsg,pMsg);

	return(OK);
}
