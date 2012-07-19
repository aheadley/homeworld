/*
** BMP.H : Header file for BMP.C.
*/

#ifndef __BMP_H
#define __BMP_H

extern "C"
{	
	typedef struct tagBMPFile
	{
		char fileName[255];

		BITMAPFILEHEADER bmfh;
		BITMAPINFOHEADER bmih;
		RGBQUAD			 *paletteBits;
		unsigned long	 bitmapSize;
		char			 *bitmapBits;
	} BMPFile;

	BOOL LoadBMPFile(char *fileName, BMPFile *pBmpFile);

	void KillBMPFile(BMPFile *pBmpFile);
}

#endif