/*
** TGA.H : Header file for TGA.C.
*/

#ifndef __TGA_H
#define __TGA_H

extern "C"
{	
	typedef struct tagTGAFileHeader
	{
		unsigned char idLength;
		unsigned char colorMapType;
		unsigned char imageType;
		// Color Map information.
		unsigned short colorMapStartIndex;
		unsigned short colorMapNumEntries;
		unsigned char colorMapBitsPerEntry;
		// Image Map information.
		signed short imageOffsetX;
		signed short imageOffsetY;
		unsigned short imageWidth;
		unsigned short imageHeight;
		unsigned char pixelDepth;
		unsigned char imageDescripor;
	} TGAFileHeader;

	typedef struct tagTGAFile
	{
		char fileName[255];
		unsigned long bitmapSize;
		char *bitmapBits;

		TGAFileHeader	header;
	} TGAFile;

	BOOL LoadTGAFile(char *fileName, TGAFile *pTGAFile);

	void KillTGAFile(TGAFile *pTGAFile);
}

#endif