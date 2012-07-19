// Copyright (c) 1998-99 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef MISSION_H
#define MISSION_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

#define MIS_INI		szIniFile
#define MIS_CFG		szCfgFile
#define MIS_LEN		256

#define MIS_SEP		"|"

#define MIS_VER		"1.05B"

#define MIS_MOD_INI		0
#define MIS_MOD_CFG		1
#define MIS_MOD_REG		2

#define MIS_ERR_ALLOC		-2 // Unable to allocate memory
#define MIS_ERR_OPEN		-3 // Unable to open file
#define MIS_ERR_REG			-4 // Unable to access registry

#ifdef __cplusplus
extern "C" {
#endif

// Export functions
__declspec(dllexport) int __stdcall misSetFile(char *szFile,int nMode);

__declspec(dllexport) int __stdcall misGetVal(char *szSection,char *szKey,char **szVal,int nMode);
__declspec(dllexport) int __stdcall misGetListByKey(char *szSection,char *szPrefix,char **szList,unsigned short *nCount,int nMode);
__declspec(dllexport) int __stdcall misGetListBySection(char *szPrefix,char *szKey,char **szTypes,unsigned short *nCount,int nMode);

__declspec(dllexport) int __stdcall misPutVal(char *szSection,char *szKey,char *szVal,int nMode);
__declspec(dllexport) int __stdcall misPutListByKey(char *szSection,char *szPrefix,char *szList,int nMode);

__declspec(dllexport) int __stdcall misOpen(FILE **pStream,char *szFile);
__declspec(dllexport) int __stdcall misWriteAttrib(FILE *pStream,char *szName,char *szVal);
__declspec(dllexport) int __stdcall misWriteInfo(FILE *pStream,char *szInfo);
__declspec(dllexport) int __stdcall misWriteNew(FILE *pStream);
__declspec(dllexport) int __stdcall misWriteVal(FILE *pStream,char *szFormat,char *szVal);
__declspec(dllexport) int __stdcall misClose(FILE *pStream);

__declspec(dllexport) int __stdcall misShellExec(char *szFile);

__declspec(dllexport) int __stdcall misGetVer(char **szVer);
__declspec(dllexport) unsigned long __stdcall misGetHash(char *szText);
__declspec(dllexport) int __stdcall misGetErr(int nErr,char **szMsg);

__declspec(dllexport) int __stdcall misRender(HWND hWnd,int x,int y,int nButton);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif
