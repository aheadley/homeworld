/*
** BMP.CPP : Code to load Window's .BMP files.
*/

#include "stdafx.h"

#include "bmp.h"

BOOL LoadBMPFile(char *fileName, BMPFile *pBmpFile)
{
	FILE *fileHandle;

	fileHandle = fopen(fileName, "rb");

	ASSERT(fileHandle);

	strcpy(pBmpFile->fileName, fileName);
		
	// Load the BITMAPFILEHEADER structure.
	fread(&pBmpFile->bmfh, sizeof(BITMAPFILEHEADER), 1, fileHandle);

	// Load the BITMAPINFOHEADER structure.
	fread(&pBmpFile->bmih, sizeof(BITMAPINFOHEADER), 1, fileHandle);

	// Make sure we are using the correct number of colors.
	if(pBmpFile->bmih.biClrUsed == 0)
	{
		pBmpFile->bmih.biClrUsed = 1 << pBmpFile->bmih.biBitCount;
		pBmpFile->bmih.biClrImportant = 1 << pBmpFile->bmih.biBitCount;
	}

	// Allocate space for the palette.
	pBmpFile->paletteBits = new RGBQUAD [pBmpFile->bmih.biClrUsed];

	// Read in the palette.
	fread(pBmpFile->paletteBits, sizeof(RGBQUAD), pBmpFile->bmih.biClrUsed, fileHandle);

	// Calculate size of bitmap in bytes.
	pBmpFile->bitmapSize = pBmpFile->bmfh.bfSize - pBmpFile->bmfh.bfOffBits;

	// Allocate space for the bitmap.
	pBmpFile->bitmapBits = new char [pBmpFile->bitmapSize];

	// Read in the bitmap.
	fread(pBmpFile->bitmapBits, 1, pBmpFile->bitmapSize, fileHandle);

	fclose(fileHandle);

	return(1);
}

void KillBMPFile(BMPFile *pBmpFile)
{
	if(pBmpFile->paletteBits)
		delete [] pBmpFile->paletteBits;

	if(pBmpFile->bitmapBits)
		delete [] pBmpFile->bitmapBits;

	memset(pBmpFile, 0x00, sizeof(BMPFile));
}