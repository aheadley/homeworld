//----------------------------------------------------------------------------
// MD5 clone.
//----------------------------------------------------------------------------
#ifndef __MD52_H__
#define __MD52_H__

typedef unsigned long uint32;

#define MD5_HASH_SIZE 16


struct MD5Context
{
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};


//----------------------------------------------------------------------------
// Raw MD5 calls.
//----------------------------------------------------------------------------
void MD5Init2(struct MD5Context *);
void MD5Update2(struct MD5Context *, unsigned char *buf, unsigned len);
void MD5Final2(unsigned char digest[16], struct MD5Context *);
void Transform2(uint32 buf[4], uint32 in[16]);


//----------------------------------------------------------------------------
// Package Deals (easier for developers to use).
//----------------------------------------------------------------------------
BOOL MD5HashText(LPCSTR sText, BYTE Hash[MD5_HASH_SIZE]);
void MD5HashToStr(BYTE Hash[MD5_HASH_SIZE], LPSTR sText);
void MD5StrToHash(LPCSTR sText, BYTE Hash[MD5_HASH_SIZE]);


#endif
