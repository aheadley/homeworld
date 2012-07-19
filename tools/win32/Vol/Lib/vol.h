// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#ifndef VOL_H
#define VOL_H

// General constants
#ifndef OK
#define OK		0
#endif

#ifndef ERR
#define ERR		-1
#endif

#define VOL_ID		4
#define VOL_LEN		256
#define VOL_FREQ	8

#define VOL_ERR_LUFILE		-2 // Unable to open lookup file
#define VOL_ERR_LUWRITE		-3 // Unable to write data to lookup table 
#define VOL_ERR_LUREAD		-4 // Unable to read data from lookup table 
#define VOL_ERR_LUALLOC		-5 // Unable to allocate memory for lookup table 
#define VOL_ERR_LBLFILE		-6 // Unable to open labels file
#define VOL_ERR_LBLALLOC	-7 // Unable to allocate memory for labels

#ifdef __cplusplus
extern "C" {
#endif

// Export functions
__declspec(dllexport) int __stdcall volOpenWLookup(char *szFileName,char *szID,
												  unsigned short nNumVRs,
												  unsigned short nNumHeads);
__declspec(dllexport) int __stdcall volWriteLookup(long *aLookup);
__declspec(dllexport) int __stdcall volCloseWLookup(void);
__declspec(dllexport) int __stdcall volOpenRLookup(char *szFileName,
												  unsigned short *nNumVRs,
												  unsigned short *nNumHeads);
__declspec(dllexport) int __stdcall volReadLookup(long *aLookup);
__declspec(dllexport) int __stdcall volCloseRLookup(void);
__declspec(dllexport) int __stdcall volOpenWFreq(char *szFileName,char *szID,
												  unsigned short nNumVRs);
__declspec(dllexport) int __stdcall volWriteFreq(float *aFreq);
__declspec(dllexport) int __stdcall volCloseWFreq(void);
__declspec(dllexport) int __stdcall volOpenRFreq(char *szFileName,
												  unsigned short *nNumVRs);
__declspec(dllexport) int __stdcall volReadFreq(float *aFreq);
__declspec(dllexport) int __stdcall volCloseRFreq(void);
__declspec(dllexport) int __stdcall volGetLabels(char *szFileName,char **szLabels,
												 unsigned short *nCount);
__declspec(dllexport) int __stdcall volGetErr(int nErr,char **szText);

#ifdef __cplusplus
}		// extern "C"
#endif

#endif  // VOL_H
