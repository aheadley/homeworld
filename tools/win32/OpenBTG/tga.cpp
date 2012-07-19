/*
** TGA.CPP : Code to load .TGA files.
*/

#include "stdafx.h"
#include "assert.h"

#include "TGA.h"

BOOL LoadTGAFile(char *fileName, TGAFile *pTGAFile)
{
	FILE *fileHandle;
	unsigned char* bp;
	unsigned char  r, g, b, a;

	fileHandle = fopen(fileName, "rb");

	assert(fileHandle);

	strcpy(pTGAFile->fileName, fileName);

	// Load the TGA header.  Load all items seperately to get around padding issues.
	fread(&pTGAFile->header.idLength, sizeof(unsigned char), 1, fileHandle);
	fread(&pTGAFile->header.colorMapType, sizeof(unsigned char), 1, fileHandle);
	fread(&pTGAFile->header.imageType, sizeof(unsigned char), 1, fileHandle);
	// Color Map information.
	fread(&pTGAFile->header.colorMapStartIndex, sizeof(unsigned short), 1, fileHandle);
	fread(&pTGAFile->header.colorMapNumEntries, sizeof(unsigned short), 1, fileHandle);
	fread(&pTGAFile->header.colorMapBitsPerEntry, sizeof(unsigned char), 1, fileHandle);
	// Image Map information.
	fread(&pTGAFile->header.imageOffsetX, sizeof(unsigned short), 1, fileHandle);
	fread(&pTGAFile->header.imageOffsetY, sizeof(unsigned short), 1, fileHandle);
	fread(&pTGAFile->header.imageWidth, sizeof(unsigned short), 1, fileHandle);
	fread(&pTGAFile->header.imageHeight, sizeof(unsigned short), 1, fileHandle);
	fread(&pTGAFile->header.pixelDepth, sizeof(unsigned char), 1, fileHandle);
	fread(&pTGAFile->header.imageDescripor, sizeof(unsigned char), 1, fileHandle);
	
	// Force 32 bit images only.
	//assert(pTGAFile->header.pixelDepth == 32);
	if(pTGAFile->header.pixelDepth != 32)
	{
		AfxMessageBox("Only 32 bit .TGA images supported.");
		fclose(fileHandle);

		memset(pTGAFile, 0x00, sizeof(TGAFile));

		return(0);
	}
	
	// Skip optional image information.
	fseek(fileHandle, pTGAFile->header.idLength, SEEK_CUR);
	
	// Calculate size of bitmap in bytes.
	pTGAFile->bitmapSize = pTGAFile->header.imageWidth * pTGAFile->header.imageHeight * (pTGAFile->header.pixelDepth / 8);

	// Allocate space for the bitmap.
	pTGAFile->bitmapBits = new char [pTGAFile->bitmapSize];

	// Read in the bitmap.
	fread(pTGAFile->bitmapBits, 1, pTGAFile->bitmapSize, fileHandle);

	// Convert to a GL-friendly format.
	bp = (unsigned char*)pTGAFile->bitmapBits;
	for (int i = 0; i < 4*pTGAFile->header.imageWidth*pTGAFile->header.imageHeight; i += 4, bp += 4)
	{
		r = bp[0];
		g = bp[1];
		b = bp[2];
		a = bp[3];
		bp[0] = b;
		bp[1] = g;
		bp[2] = r;
		bp[3] = 255 - a;
	}

	fclose(fileHandle);

	return(1);
}

void KillTGAFile(TGAFile *pTGAFile)
{
	if(pTGAFile->bitmapBits)
		delete [] pTGAFile->bitmapBits;

	memset(pTGAFile, 0x00, sizeof(TGAFile));
}