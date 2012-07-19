// Copyright (c) 1998 Relic Entertainment Inc.
// Written by Janik Joire
//
// $History: $

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "lwob.h"

// Integer read/write functions (Motorola byte order) 

short RBMShort(FILE *pStream)
{
	short	nVal;

	nVal=(short)fgetc(pStream);
	nVal=(nVal<<8)|(short)fgetc(pStream);

	return(nVal);
}

long RBMLong(FILE *pStream)
{
	long	nVal;

	nVal=(long)fgetc(pStream);
	nVal=(nVal<<8)|(long)fgetc(pStream);
	nVal=(nVal<<8)|(long)fgetc(pStream);
	nVal=(nVal<<8)|(long)fgetc(pStream);

	return(nVal);
}

float RBMIeee(FILE *pStream)
{
	float fVal;
	long nVal;

	nVal=(long)fgetc(pStream);
	nVal=(nVal<<8)|(long)fgetc(pStream);
	nVal=(nVal<<8)|(long)fgetc(pStream);
	nVal=(nVal<<8)|(long)fgetc(pStream);

	memcpy(&fVal,&nVal,sizeof(float));

	return(fVal);
}

short WBMShort(short nVal,FILE *pStream)
{
	fputc(nVal>>8,pStream);
	fputc(nVal&0xFF,pStream);

	return(nVal);
}

long WBMLong(long nVal,FILE *pStream)
{
	fputc(nVal>>24,pStream);
	fputc((nVal>>16)&0xFF,pStream);
	fputc((nVal>>8)&0xFF,pStream);
	fputc(nVal&0xFF,pStream);

	return(nVal);
}

// IEEE floating-point read/writefunctions

double ReadIeeeExtended(FILE *pStream)
{
	char aBytes[10];

	fread(aBytes,1,10,pStream);

	return(ConvertFromIeeeExtended(aBytes));
}

int WriteIeeeExtended(FILE *pStream,double fNum)
{
	char aBytes[10];

	ConvertToIeeeExtended(aBytes,fNum);

	fwrite(aBytes,1,10,pStream);

	return(OK);
}

/* Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VaVerts
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */

int ConvertToIeeeExtended(char *bytes,double num)
{
    short			sign;
    short			expon;
    double			fMant,fsMant;
    unsigned long	hiMant,loMant;

    if(num < 0)
    {
		sign=(short)0x8000;
		num*=-1;
    }
    else sign = 0;

    if(num == 0)
	{
		expon=0;
		hiMant=0;
		loMant=0;
	}
    else
    {
		fMant=frexp(num,(int *)&expon);

		if((expon > 16384) || !(fMant < 1))	// Infinity or NaN
		{
			expon=sign|0x7FFF;
			hiMant=0;
			loMant=0;	// Infinity
		}
		else	// Finite
		{
			expon+=16382;

			if(expon < 0)	// Denormalized
			{
				fMant=ldexp(fMant,expon);
				expon=0;
			}

			expon|=sign;
			fMant=ldexp(fMant,32);
			fsMant=floor(fMant);
			hiMant=FloatToUnsigned(fsMant);
			fMant=ldexp(fMant-fsMant,32);
			fsMant=floor(fMant);
			loMant=FloatToUnsigned(fsMant);
		}
    }

    bytes[0]=(char)(expon>>8);
    bytes[1]=(char)expon;
    bytes[2]=(char)(hiMant>>24);
    bytes[3]=(char)(hiMant>>16);
    bytes[4]=(char)(hiMant>>8);
    bytes[5]=(char)hiMant;
    bytes[6]=(char)(loMant>>24);
    bytes[7]=(char)(loMant>>16);
    bytes[8]=(char)(loMant>>8);
    bytes[9]=(char)loMant;

    return(OK);
}

double ConvertFromIeeeExtended(char *bytes)
{
    double			f;
    short			expon;
    unsigned long	hiMant,loMant;

    expon=((bytes[0]&0x7F)<<8)|(bytes[1]&0xFF);
    hiMant=((unsigned long)(bytes[2]&0xFF)<<24)
		|((unsigned long)(bytes[3]&0xFF)<<16)
		|((unsigned long)(bytes[4]&0xFF)<<8)
		|((unsigned long)(bytes[5]&0xFF));
    loMant=((unsigned long)(bytes[6]&0xFF)<<24)
		|((unsigned long)(bytes[7]&0xFF)<<16)
		|((unsigned long)(bytes[8]&0xFF)<<8)
		|((unsigned long)(bytes[9]&0xFF));

    if((expon == 0) && (hiMant == 0) && (loMant == 0)) f=0;
    else
    {
		if (expon == 0x7FFF)	// Infinity or NaN
			f=HUGE_VAL;
		else
		{
			expon-=16383;
			f=ldexp(UnsignedToFloat(hiMant),expon-=31);
			f+=ldexp(UnsignedToFloat(loMant),expon-=32);
		}
    }

    if(bytes[0]&0x80) return(-f);
    else return(f);
}

// LWOB functions

// Function GetLwobData() reads the points and polygons from an LWOB file
// Open the file in "rb" mode and call GetLwobData()

int GetLwobData(FILE *pStream,short *nVerts,short *nPolys,
	float **aVerts,short **aPolys)
{
	int		nPntsStat,nSrfsStat,nPolsStat;
	long	nInd,nSize,nPos,nRef,nLen;
	short	n,m,nDet,nSurf,nVert,nNum,nData;
	char 	aData[LWOB_ID];

	*nVerts=0;
	*nPolys=0;

	*aVerts=NULL;
	*aPolys=NULL;

	nPntsStat=FALSE;
	nSrfsStat=FALSE;
	nPolsStat=FALSE;

	rewind(pStream);

	if((fread(aData,LWOB_ID,1,pStream) != 1) || (memcmp(aData,"FORM",LWOB_ID) != 0)) return(LWOB_ERR_INVALID);

	nLen=RBMLong(pStream);

	if(fread(aData,LWOB_ID,1,pStream) != 1) return(LWOB_ERR_INVALID);

	if(nLen == 0)
	{
		if(memcmp(aData,"MMAN",LWOB_ID) != 0) return(LWOB_ERR_INVALID);
	}
	else
	{
		if(memcmp(aData,"LWOB",LWOB_ID) != 0) return(LWOB_ERR_INVALID);
	}

	while((nPntsStat == FALSE) || (nSrfsStat == FALSE) || (nPolsStat == FALSE))
	{
		if(fread(aData,LWOB_ID,1,pStream) != 1)
		{
			if(nPntsStat == FALSE) return(LWOB_ERR_NOPNTS);
			if(nSrfsStat == FALSE) return(LWOB_ERR_NOSRFS);
			if(nPolsStat == FALSE) return(LWOB_ERR_NOPOLS);
		}

		nSize=RBMLong(pStream);
		if(nSize%2 > 0) nSize++;

		nInd=nSize+ftell(pStream);

		if((nPntsStat == FALSE) && (memcmp(aData,"PNTS",LWOB_ID) == 0))
		{
			*nVerts=nSize/LWOB_PNTS;

			*aVerts=(float *)calloc(*nVerts*3,sizeof(float));
			if(*aVerts == NULL) return(LWOB_ERR_ALLOC);

			for(n=0;n<*nVerts;n++)
			{
				(*aVerts)[n*3]=RBMIeee(pStream);
				(*aVerts)[(n*3)+1]=RBMIeee(pStream);
				(*aVerts)[(n*3)+2]=RBMIeee(pStream);
			}

			nPntsStat=TRUE;
		}

		if((nSrfsStat == FALSE) && (memcmp(aData,"SRFS",LWOB_ID) == 0))
		{
			nSrfsStat=TRUE;
		}

		if((nPntsStat == TRUE) && (nPolsStat == FALSE) && (memcmp(aData,"POLS",LWOB_ID) == 0))
		{
			nRef=ftell(pStream);

			nPos=0;

			*nPolys=0;
			
			while(1)
			{
				if(nPos >= nSize) break;
				
				nVert=RBMShort(pStream);
				nPos=nPos+sizeof(short);

				if(nVert < 1) return(LWOB_ERR_EMPTY);
#ifdef LWOB_WARN
				if(nVert > LWOB_SIZE) return(LWOB_ERR_NOTRGL);
#endif
				for(n=0;n<nVert;n++)
				{
					RBMShort(pStream);
					nPos=nPos+sizeof(short);
				}

				(*nPolys)++;

				nSurf=RBMShort(pStream);
				nPos=nPos+sizeof(short);
				if(nSurf < 0)
				{
#ifdef LWOB_WARN
					return(LWOB_ERR_DETAIL);
#else
					nDet=RBMShort(pStream);
					nPos=nPos+sizeof(short);
					
					for(n=0;n<nDet;n++)
					{
						if(nPos >= nSize) break;
				
						nVert=RBMShort(pStream);
						nPos=nPos+sizeof(short);

						for(m=0;m<nVert;m++)
						{
							RBMShort(pStream);
							nPos=nPos+sizeof(short);
						}

						RBMShort(pStream);
						nPos=nPos+sizeof(short);
					}
#endif
				}
			}

			if(*nPolys == 0) return(LWOB_ERR_ALLOC);
			*aPolys=(short *)calloc(*nPolys*LWOB_SIZE,sizeof(short));
			if(*aPolys == NULL) return(LWOB_ERR_ALLOC);

			fseek(pStream,nRef,SEEK_SET);

			nPos=0;

			nData=0;
			
			*nPolys=0;
			
			while(1)
			{
				if(nPos >= nSize) break;
				
				nVert=RBMShort(pStream);
				nPos=nPos+sizeof(short);
				
				if(nVert < LWOB_SIZE) nNum=LWOB_SIZE;
				else nNum=nVert;

				for(n=0;n<nNum;n++)
				{
					if(n < nVert)
					{
						nData=RBMShort(pStream);
						nPos=nPos+sizeof(short);
					}

					if(n < LWOB_SIZE) (*aPolys)[(*nPolys*LWOB_SIZE)+n]=nData;
				}

				(*nPolys)++;

				nSurf=RBMShort(pStream);
				nPos=nPos+sizeof(short);
				if(nSurf < 0)
				{
					nDet=RBMShort(pStream);
					nPos=nPos+sizeof(short);
					
					for(n=0;n<nDet;n++)
					{
						if(nPos >= nSize) break;
				
						nVert=RBMShort(pStream);
						nPos=nPos+sizeof(short);

						for(m=0;m<nVert;m++)
						{
							RBMShort(pStream);
							nPos=nPos+sizeof(short);
						}

						RBMShort(pStream);
						nPos=nPos+sizeof(short);
					}
				}
			}

			nPolsStat=TRUE;
		}

		if(feof(pStream) != 0) return(LWOB_ERR_CORRUPT);

		fseek(pStream,nInd-ftell(pStream),SEEK_CUR);
	}

	return(OK);
}

// Function GetLwobErr() returns an error message for a matching LWOB error code

char *GetLwobErr(int nErr)
{
	switch(nErr)
	{
		// LWOB error codes
		case LWOB_ERR_INVALID:	return("Invalid LWOB/MMAN file");
		case LWOB_ERR_CORRUPT:	return("Corrupt LWOB/MMAN file");
		case LWOB_ERR_NOPNTS:	return("No points chunk in LWOB/MMAN file");
		case LWOB_ERR_NOSRFS:	return("No surfaces chunk in LWOB/MMAN file");
		case LWOB_ERR_NOPOLS:	return("No polygons chunk in LWOB/MMAN file");
		case LWOB_ERR_EMPTY:	return("Empty polygons in LWOB/MMAN file");
		case LWOB_ERR_NOTRGL:	return("No triangles in LWOB/MMAN file");
		case LWOB_ERR_DETAIL:	return("Detail polygons in LWOB/MMAN file");
		case LWOB_ERR_ALLOC:	return("Unable to allocate memory");

		// Default error codes
		case 0:		return("No error");

		default:	return("Undefined error");
	}
}
